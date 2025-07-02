/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2025 Scientific Computing and Imaging Institute,
University of Utah.


Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/
#include <TreePanel.h>
#include <Global.h>
#include <Names.h>
#include <BrushDefault.h>
#include <GlobalStates.h>
#include <MainFrame.h>
#include <RenderView.h>
#include <compatibility.h>
#include <DataManager.h>
#include <VolumeSelector.h>
#include <RulerHandler.h>
#include <Colocalize.h>
#include <Ruler.h>
//resources
#include <png_resource.h>
#include <tick.xpm>
#include <cross.xpm>
#include <icons.h>

DataTreeCtrl::DataTreeCtrl(
	wxWindow* parent,
	const wxPoint& pos,
	const wxSize& size,
	long style) :
wxTreeCtrl(parent, wxID_ANY, pos, size, style)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);
	SetDoubleBuffered(true);

	wxImageList *images = new wxImageList(16, 16, true);
	wxIcon icons[2];
	icons[0] = wxIcon(cross_xpm);
	icons[1] = wxIcon(tick_xpm);
	images->Add(icons[0]);
	images->Add(icons[1]);
	AssignImageList(images);
	m_selected.Unset();

	Bind(wxEVT_TREE_SEL_CHANGED, &DataTreeCtrl::OnSelectionChanged, this);
}

DataTreeCtrl::~DataTreeCtrl()
{
}

//icons
void DataTreeCtrl::AppendIcon()
{
	wxImageList *images = GetImageList();
	if (!images)
		return;

	wxIcon icon0 = wxIcon(cross_xpm);
	wxIcon icon1 = wxIcon(tick_xpm);
	images->Add(icon0);
	images->Add(icon1);
}

void DataTreeCtrl::ClearIcons()
{
	wxImageList *images = GetImageList();
	if (!images)
		return;

	images->RemoveAll();
}

int DataTreeCtrl::GetIconNum()
{
	wxImageList* images = GetImageList();
	if (images)
		return images->GetImageCount()/2;
	else
		return 0;
}

void DataTreeCtrl::ChangeIconColor(int i, wxColor c)
{
	ChangeIconColor(i*2, c, 0);
	ChangeIconColor(i*2+1, c, 1);
}

void DataTreeCtrl::ChangeIconColor(int which, wxColor c, int type)
{
	int i;
	int icon_lines = 0;
	int icon_height = 0;

	wxImageList *images = GetImageList();
	if (!images)
		return;

	const char **orgn_data = type?tick_xpm:cross_xpm;
	char cc[8];

	int dummy;
	SSCANF(orgn_data[0], "%d %d %d %d",
		&dummy, &icon_height, &icon_lines, &dummy);
	icon_lines += icon_height+1;
	char **data = new char*[icon_lines];

	sprintf(cc, "#%02X%02X%02X", c.Red(), c.Green(), c.Blue());

	for (i=0; i<icon_lines; i++)
	{
		if (i==icon_change)
		{
			int len = strlen(orgn_data[i]);
			int len_key = strlen(icon_key);
			int len_chng = len+strlen(cc)-len_key+1;
			data[i] = new char[len_chng]();
			char *temp = new char[len_key+1]();
			int index = 0;
			for (int j=0; j<len; j++)
			{
				char val = orgn_data[i][j];
				if (j>=len_key-1)
				{
					for (int k=0; k<len_key; k++)
						temp[k] = orgn_data[i][j-(len_key-k-1)];
					if (!strcmp(temp, icon_key))
					{
						strcpy(data[i]+index-len_key+1, cc);
						index = index-len_key+1+strlen(cc);
						continue;
					}
				}
				data[i][index++] = val;
			}
			delete[] temp;
		}
		else
		{
			int len = strlen(orgn_data[i]);
			data[i] = new char[len+1];
			memcpy(data[i], orgn_data[i], len+1);
		}
	}
	wxIcon icon = wxIcon(data);
	images->Replace(which, icon);
	for (i=0; i<icon_lines; i++)
	{
		delete[] data[i];
	}
	delete[] data;
}

//item operations
//root item
wxTreeItemId DataTreeCtrl::AddRootItem(const wxString &text)
{
	wxTreeItemId item = AddRoot(text);
	LayerInfo* item_data = new LayerInfo;
	item_data->type = 0;//root;
	SetItemData(item, item_data);
	return item;
}

void DataTreeCtrl::ExpandRootItem()
{
	Expand(GetRootItem());
}

//view item
wxTreeItemId DataTreeCtrl::AddViewItem(const wxString &text)
{
	wxTreeItemId item = AppendItem(GetRootItem(),text, 1);
	LayerInfo* item_data = new LayerInfo;
	item_data->type = 1;//view
	SetItemData(item, item_data);
	return item;
}

void DataTreeCtrl::SetViewItemImage(const wxTreeItemId& item, int image)
{
	SetItemImage(item , image);
}

//volume data item
wxTreeItemId DataTreeCtrl::AddVolItem(wxTreeItemId par_item, const wxString &text)
{
	wxTreeItemId item = AppendItem(par_item, text, 1);
	LayerInfo* item_data = new LayerInfo;
	item_data->type = 2;//volume data
	SetItemData(item, item_data);
	return item;
}

void DataTreeCtrl::SetVolItemImage(const wxTreeItemId item, int image)
{
	SetItemImage(item, image);
}

//mesh data item
wxTreeItemId DataTreeCtrl::AddMeshItem(wxTreeItemId par_item, const wxString &text)
{
	wxTreeItemId item = AppendItem(par_item, text, 1);
	LayerInfo* item_data = new LayerInfo;
	item_data->type = 3;//mesh data
	SetItemData(item, item_data);
	return item;
}

void DataTreeCtrl::SetMeshItemImage(const wxTreeItemId item, int image)
{
	SetItemImage(item, image);
}

//annotation item
wxTreeItemId DataTreeCtrl::AddAnnotationItem(wxTreeItemId par_item, const wxString &text)
{
	wxTreeItemId item = AppendItem(par_item, text, 1);
	LayerInfo* item_data = new LayerInfo;
	item_data->type = 4;//annotations
	SetItemData(item, item_data);
	return item;
}

void DataTreeCtrl::SetAnnotationItemImage(const wxTreeItemId item, int image)
{
	SetItemImage(item, image);
}

//group item
wxTreeItemId DataTreeCtrl::AddGroupItem(wxTreeItemId par_item, const wxString &text)
{
	wxTreeItemId item = AppendItem(par_item, text, 1);
	LayerInfo* item_data = new LayerInfo;
	item_data->type = 5;//group
	SetItemData(item, item_data);
	return item;
}

void DataTreeCtrl::SetGroupItemImage(const wxTreeItemId item, int image)
{
	SetItemImage(item, image);
}

//mesh group item
wxTreeItemId DataTreeCtrl::AddMGroupItem(wxTreeItemId par_item, const wxString &text)
{
	wxTreeItemId item = AppendItem(par_item, text, 1);
	LayerInfo* item_data = new LayerInfo;
	item_data->type = 6;//mesh group
	SetItemData(item, item_data);
	return item;
}

void DataTreeCtrl::SetMGroupItemImage(const wxTreeItemId item, int image)
{
	SetItemImage(item, image);
}

void DataTreeCtrl::OnSelectionChanged(wxTreeEvent& event)
{
	if (m_selected.IsOk())
		SetItemBold(m_selected, false);
	m_selected = GetSelection();
	if (m_selected.IsOk())
		SetItemBold(m_selected);
	event.Skip();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
TreePanel::TreePanel(MainFrame* frame,
	wxWindow* parent,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name) :
	PropPanel(frame, parent, pos, size, style, name),
	m_scroll_pos(-1)
{
	wxEventBlocker blocker(this);
	SetDoubleBuffered(true);
	//create data tree
	m_datatree = new DataTreeCtrl(this);

	//create tool bar
	m_toolbar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxTB_FLAT|wxTB_TOP|wxTB_NODIVIDER);
	wxBitmapBundle bitmap = wxGetBitmap(toggle_disp);
	m_toolbar->AddTool(ID_ToggleDisp, "Toggle View", bitmap,
		"Toggle the visibility of current selection");
	m_toolbar->SetToolLongHelp(ID_ToggleDisp, "Toggle the visibility of current selection");
	bitmap = wxGetBitmap(add_group);
	m_toolbar->AddTool(ID_AddVolGroup, "Add Group", bitmap,
		"Add a volume data group to the selected view");
	m_toolbar->SetToolLongHelp(ID_AddVolGroup, "Add a volume data group to the selected view");
	bitmap = wxGetBitmap(add_mgroup);
	m_toolbar->AddTool(ID_AddMeshGroup, "Add Mesh Group", bitmap,
		"Add a mesh data group to the selected view");
	m_toolbar->SetToolLongHelp(ID_AddMeshGroup, "Add a mesh data group to the selected view");
	bitmap = wxGetBitmap(delet);
	m_toolbar->AddTool(ID_RemoveData, "Delete", bitmap,
		"Delete current selection");
	m_toolbar->SetToolLongHelp(ID_RemoveData, "Delete current selection");
	m_toolbar->AddSeparator();
	bitmap = wxGetBitmap(two_point);
	m_toolbar->AddCheckTool(ID_RulerLine, "Line",
		bitmap, wxNullBitmap,
		"Add rulers by clicking twice at end points",
		"Add rulers by clicking twice at end points");
	bitmap = wxGetBitmap(multi_point);
	m_toolbar->AddCheckTool(ID_RulerPolyline, "Polyline",
		bitmap, wxNullBitmap,
		"Add a polyline ruler by clicking at each point",
		"Add a polyline ruler by clicking at each point");
	bitmap = wxGetBitmap(pencil);
	m_toolbar->AddCheckTool(ID_RulerPencil, "Pencil",
		bitmap, wxNullBitmap,
		"Draw ruler with multiple points continuously",
		"Draw ruler with multiple points continuously");
	m_toolbar->AddSeparator();
	bitmap = wxGetBitmap(ruler_edit);
	m_toolbar->AddCheckTool(ID_RulerEdit, "Edit",
		bitmap, wxNullBitmap,
		"Select and move a ruler point",
		"Select and move a ruler point");
	bitmap = wxGetBitmap(ruler_del);
	m_toolbar->AddCheckTool(ID_RulerDeletePoint, "Delete",
		bitmap, wxNullBitmap,
		"Select and delete a ruler point",
		"Select and delete a ruler point");

	m_toolbar->Bind(wxEVT_TOOL, &TreePanel::OnToolbar, this);
	m_toolbar->Realize();

	//create toolbar2
	m_toolbar2 = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxTB_FLAT|wxTB_TOP|wxTB_NODIVIDER);
	bitmap = wxGetBitmap(grow);
	m_toolbar2->AddCheckTool(ID_BrushGrow, "Grow",
		bitmap, wxNullBitmap,
		"Click and hold mouse button to grow selection mask from a point",
		"Click and hold mouse button to grow selection mask from a point");
	bitmap = wxGetBitmap(brush_append);
	m_toolbar2->AddCheckTool(ID_BrushAppend, "Select",
		bitmap, wxNullBitmap,
		"Highlight structures by painting (hold Shift)",
		"Highlight structures by painting (hold Shift)");
	bitmap = wxGetBitmap(brush_diffuse);
	m_toolbar2->AddCheckTool(ID_BrushDiffuse, "Diffuse",
		bitmap, wxNullBitmap,
		"Diffuse highlighted structures by painting (hold Z)",
		"Diffuse highlighted structures by painting (hold Z)");
	bitmap = wxGetBitmap(brush_unsel);
	m_toolbar2->AddCheckTool(ID_BrushUnselect, "Unselect",
		bitmap, wxNullBitmap,
		"Remove the highlights by painting (hold X)",
		"Remove the highlights by painting (hold X)");
	m_toolbar2->AddSeparator();
	bitmap = wxGetBitmap(brush_locator);
	m_toolbar2->AddCheckTool(ID_BrushRuler, "Centroid",
		bitmap, wxNullBitmap,
		"Select structures and create a locator at center",
		"Select structures and create a locator at center");
	bitmap = wxGetBitmap(brush_comp);
	m_toolbar2->AddCheckTool(ID_BrushComp, "Segment",
		bitmap, wxNullBitmap,
		"Select structures and then segment them into components",
		"Select structures and then segment them into components");
	m_toolbar2->AddSeparator();
	bitmap = wxGetBitmap(brush_clear);
	m_toolbar2->AddTool(ID_BrushClear, "Clear",
		bitmap, "Clear all highlights");
	m_toolbar2->SetToolLongHelp(ID_BrushClear, "Clear all highlights");
	bitmap = wxGetBitmap(brush_extract);
	m_toolbar2->AddTool(ID_BrushExtract, "Extract", bitmap,
		"Extract highlighted structures and create a new volume");
	m_toolbar2->SetToolLongHelp(ID_BrushExtract, "Extract highlighted structures and create a new volume");
	bitmap = wxGetBitmap(brush_delete);
	m_toolbar2->AddTool(ID_BrushDelete, "Delete",
		bitmap, "Delete highlighted structures");
	m_toolbar2->SetToolLongHelp(ID_BrushDelete, "Delete highlighted structures");

	m_toolbar2->Bind(wxEVT_TOOL, &TreePanel::OnToolbar, this);
	m_toolbar2->Realize();

	//toolbar 3
	m_toolbar3 = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxTB_FLAT|wxTB_LEFT|wxTB_NODIVIDER);
	bitmap = wxGetBitmap(filter_small);
	m_toolbar3->AddTool(ID_Ocl, "Volume Filter", bitmap,
		"Show Volume Filter Dialog");
	m_toolbar3->SetToolLongHelp(ID_Ocl, "Show Volume Filter Dialog");
	m_toolbar3->AddSeparator();
	bitmap = wxGetBitmap(brush_small);
	m_toolbar3->AddTool(ID_Brush, "Paint Brush", bitmap,
		"Show Paint Brush Dialog");
	m_toolbar3->SetToolLongHelp(ID_Brush, "Show Paint Brush Dialog");
	bitmap = wxGetBitmap(measure_small);
	m_toolbar3->AddTool(ID_Measurement, "Measurement", bitmap,
		"Show Measurement Dialog");
	m_toolbar3->SetToolLongHelp(ID_Measurement, "Show Measurement Dialog");
	bitmap = wxGetBitmap(component_small);
	m_toolbar3->AddTool(ID_Component, "Component Analyzer", bitmap,
		"Show Component Analyzer Dialog");
	m_toolbar3->SetToolLongHelp(ID_Component, "Show Component Analyzer Dialog");
	bitmap = wxGetBitmap(track_small);
	m_toolbar3->AddTool(ID_Track, "Tracking", bitmap,
		"Show Tracking Dialog");
	m_toolbar3->SetToolLongHelp(ID_Track, "Show Tracking Dialog");
	m_toolbar3->AddSeparator();
	bitmap = wxGetBitmap(calculate_small);
	m_toolbar3->AddTool(ID_Calculation, "Calculation", bitmap,
		"Show Calculation Dialog");
	m_toolbar3->SetToolLongHelp(ID_Calculation, "Show Calculation Dialog");
	bitmap = wxGetBitmap(noise_red_small);
	m_toolbar3->AddTool(ID_NoiseReduct, "Noise Reduction", bitmap,
		"Show Noise Reduction Dialog");
	m_toolbar3->SetToolLongHelp(ID_NoiseReduct, "Show Noise Reduction Dialog");
	bitmap = wxGetBitmap(size_small);
	m_toolbar3->AddTool(ID_VolumeSize, "Volume Size", bitmap,
		"Show Volume Size Dialog");
	m_toolbar3->SetToolLongHelp(ID_VolumeSize, "Show Volume Size Dialog");
	bitmap = wxGetBitmap(colocal_small);
	m_toolbar3->AddTool(ID_Colocalization, "Colocalization", bitmap,
		"Show Colocalization Dialog");
	m_toolbar3->SetToolLongHelp(ID_Colocalization, "Show Colocalization Dialog");
	bitmap = wxGetBitmap(convert_small);
	m_toolbar3->AddTool(ID_Convert, "Convert", bitmap,
		"Show Convert Dialog");
	m_toolbar3->SetToolLongHelp(ID_Convert, "Show Convert Dialog");

	m_toolbar3->Bind(wxEVT_TOOL, &TreePanel::OnToolbar, this);
	m_toolbar3->Realize();

	//organize positions
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);

	int tool_width = m_toolbar3->GetSize().GetWidth();
	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	sizer_1->Add(tool_width, tool_width);
	sizer_1->Add(m_toolbar, 0, wxEXPAND);
	wxBoxSizer* sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	sizer_2->Add(tool_width, tool_width);
	sizer_2->Add(m_toolbar2, 0, wxEXPAND);
	wxBoxSizer* sizer_3 = new wxBoxSizer(wxHORIZONTAL);
	sizer_3->Add(m_toolbar3, 0, wxEXPAND);
	sizer_3->Add(m_datatree, 1, wxEXPAND);

	sizer_v->Add(sizer_1, 0, wxEXPAND);
	sizer_v->Add(sizer_2, 0, wxEXPAND);
	sizer_v->Add(sizer_3, 1, wxEXPAND);

	SetSizer(sizer_v);
	Layout();

	//events
	Bind(wxEVT_CONTEXT_MENU, &TreePanel::OnContextMenu, this);
	Bind(wxEVT_MENU, &TreePanel::OnMenu, this);
	Bind(wxEVT_TREE_SEL_CHANGED, &TreePanel::OnSelChanged, this);
	Bind(wxEVT_TREE_DELETE_ITEM, &TreePanel::OnDeleting, this);
	Bind(wxEVT_TREE_ITEM_ACTIVATED, &TreePanel::OnAct, this);
	Bind(wxEVT_TREE_BEGIN_DRAG, &TreePanel::OnBeginDrag, this);
	Bind(wxEVT_TREE_END_DRAG, &TreePanel::OnEndDrag, this);
	Bind(wxEVT_KEY_DOWN, &TreePanel::OnKeyDown, this);
}

TreePanel::~TreePanel()
{
}

void TreePanel::FluoUpdate(const fluo::ValueCollection& vc)
{
	if (FOUND_VALUE(gstNull))
		return;

	bool update_all = vc.empty();
	bool bval;

	//update icons only
	if (update_all || FOUND_VALUE(gstTreeCtrl))
		UpdateTree();
	else
	{
		if (update_all || FOUND_VALUE(gstTreeIcons))
			UpdateTreeIcons();
		if (update_all || FOUND_VALUE(gstTreeColors))
			UpdateTreeColors();
	}

	if (update_all || FOUND_VALUE(gstCurrentSelect))
	{
		UpdateTreeSel();
	}

	if (update_all || FOUND_VALUE(gstFreehandToolState))
	{
		auto view = glbin_current.render_view.lock();
		InteractiveMode int_mode = view ? view->GetIntMode() : InteractiveMode::None;
		flrd::SelectMode sel_mode = glbin_vol_selector.GetSelectMode();
		flrd::RulerMode rul_mode = glbin_ruler_handler.GetRulerMode();

		m_toolbar->ToggleTool(ID_RulerLine, rul_mode == flrd::RulerMode::Line);
		bval = rul_mode == flrd::RulerMode::Polyline &&
			(int_mode == InteractiveMode::Ruler ||
				int_mode == InteractiveMode::BrushRuler);
		m_toolbar->ToggleTool(ID_RulerPolyline, bval);
		m_toolbar->ToggleTool(ID_RulerPencil, int_mode == InteractiveMode::Pencil);
		m_toolbar->ToggleTool(ID_RulerEdit, int_mode == InteractiveMode::EditRulerPoint);
		m_toolbar->ToggleTool(ID_RulerDeletePoint, int_mode == InteractiveMode::RulerDelPoint);

		bval = rul_mode == flrd::RulerMode::Locator &&
			sel_mode == flrd::SelectMode::SingleSelect;
		m_toolbar2->ToggleTool(ID_BrushRuler, bval);
		m_toolbar2->ToggleTool(ID_BrushGrow, sel_mode == flrd::SelectMode::Grow);
		m_toolbar2->ToggleTool(ID_BrushAppend, sel_mode == flrd::SelectMode::Append);
		m_toolbar2->ToggleTool(ID_BrushComp, sel_mode == flrd::SelectMode::Segment);
		m_toolbar2->ToggleTool(ID_BrushDiffuse, sel_mode == flrd::SelectMode::Diffuse);
		m_toolbar2->ToggleTool(ID_BrushUnselect, sel_mode == flrd::SelectMode::Eraser);
	}
}

void TreePanel::Select()
{
	if (!m_datatree)
		return;

	wxTreeItemId sel_item = m_datatree->GetSelection();

	if (!sel_item.IsOk())
		return;

	fluo::ValueCollection vc;

	//select data
	std::wstring name = m_datatree->GetItemText(sel_item).ToStdWstring();
	LayerInfo* item_data = (LayerInfo*)m_datatree->GetItemData(sel_item);
	Root* root = glbin_data_manager.GetRoot();

	if (item_data)
	{
		switch (item_data->type)
		{
		case 0://root
			glbin_current.SetRoot();
			break;
		case 1://view
		{
			if (root)
			{
				auto view = root->GetView(name);
				glbin_current.SetRenderView(view);
			}
		}
			break;
		case 2://volume data
		{
			auto vd = glbin_data_manager.GetVolumeData(name);
			glbin_current.SetVolumeData(vd);
			vc.insert(gstVolumePropPanel);
		}
			break;
		case 3://mesh data
		{
			auto md = glbin_data_manager.GetMeshData(name);
			glbin_current.SetMeshData(md);
			vc.insert(gstMeshPropPanel);
		}
			break;
		case 4://annotations
		{
			auto ann = glbin_data_manager.GetAnnotations(name);
			glbin_current.SetAnnotation(ann);
			vc.insert(gstAnnotatPropPanel);
		}
			break;
		case 5://volume group
		{
			std::wstring par_name = m_datatree->GetItemText(m_datatree->GetItemParent(sel_item)).ToStdWstring();
			if (root)
			{
				auto view = root->GetView(par_name);
				if (view)
				{
					auto group = view->GetGroup(name);
					glbin_current.SetVolumeGroup(group);
				}
			}
		}
			break;
		case 6://mesh group
		{
			std::wstring par_name = m_datatree->GetItemText(m_datatree->GetItemParent(sel_item)).ToStdWstring();
			if (root)
			{
				auto view = root->GetView(par_name);
				if (view)
				{
					auto group = view->GetMGroup(name);
					glbin_current.SetMeshGroup(group);
				}
			}
		}
			break;
		}
	}

	vc.insert(gstCurrentSelect);
	FluoRefresh(1, { vc });
}

void TreePanel::Action()
{
	bool bval = wxGetKeyState(WXK_CONTROL);
	fluo::ValueCollection vc;

	if (bval)
	{
		RandomizeColor();
		vc.insert(gstTreeColors);
	}
	else
	{
		ToggleDisplay();
		vc.insert(gstTreeIcons);
	}
	FluoRefresh(2, vc);
}

void TreePanel::AddVolGroup()
{
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;

	std::wstring name = view->AddGroup(L"");
	auto group = view->GetGroup(name);
	glbin_current.SetVolumeGroup(group);

	FluoRefresh(0, { gstNull }, {-1});
}

void TreePanel::AddMeshGroup()
{
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;

	std::wstring name = view->AddMGroup(L"");
	auto group = view->GetMGroup(name);
	glbin_current.SetMeshGroup(group);

	FluoRefresh(0, { gstNull }, {-1});
}

void TreePanel::RemoveData()
{
	DeleteSelection();
	glbin_current.SetRoot();

	FluoRefresh(0, { gstTreeCtrl, gstCurrentSelect });
}

void TreePanel::RulerLine()
{
	glbin_states.ToggleRulerMode(flrd::RulerMode::Line);
	FluoRefresh(0, { gstFreehandToolState }, {-1});
}

void TreePanel::RulerPolyline()
{
	glbin_states.ToggleRulerMode(flrd::RulerMode::Polyline);
	FluoRefresh(0, { gstFreehandToolState }, {-1});
}

void TreePanel::RulerPencil()
{
	glbin_states.ToggleIntMode(InteractiveMode::Pencil);
	FluoRefresh(0, { gstFreehandToolState }, {-1});
}

void TreePanel::RulerEdit()
{
	glbin_states.ToggleIntMode(InteractiveMode::EditRulerPoint);
	FluoRefresh(0, { gstFreehandToolState }, {-1});
}

void TreePanel::RulerDeletePoint()
{
	glbin_states.ToggleIntMode(InteractiveMode::RulerDelPoint);
	FluoRefresh(0, { gstFreehandToolState }, {-1});
}

void TreePanel::BrushRuler()
{
	glbin_states.ToggleBrushMode(flrd::SelectMode::SingleSelect);
	glbin_states.ToggleRulerMode(flrd::RulerMode::Locator);
	FluoRefresh(0, { gstFreehandToolState, gstBrushSize1, gstBrushSize2, gstBrushIter }, {-1});
}

void TreePanel::BrushGrow()
{
	glbin_states.ToggleBrushMode(flrd::SelectMode::Grow);
	FluoRefresh(0, { gstFreehandToolState, gstBrushSize1, gstBrushSize2, gstBrushIter }, {-1});
}

void TreePanel::BrushAppend()
{
	glbin_states.ToggleBrushMode(flrd::SelectMode::Append);
	FluoRefresh(0, { gstFreehandToolState, gstBrushSize1, gstBrushSize2, gstBrushIter }, {-1});
}

void TreePanel::BrushComp()
{
	glbin_states.ToggleBrushMode(flrd::SelectMode::Segment);
	FluoRefresh(0, { gstFreehandToolState, gstBrushSize1, gstBrushSize2, gstBrushIter }, {-1});
}

void TreePanel::BrushDiffuse()
{
	glbin_states.ToggleBrushMode(flrd::SelectMode::Diffuse);
	FluoRefresh(0, { gstFreehandToolState, gstBrushSize1, gstBrushSize2, gstBrushIter }, {-1});
}

void TreePanel::BrushUnselect()
{
	glbin_states.ToggleBrushMode(flrd::SelectMode::Eraser);
	FluoRefresh(0, { gstFreehandToolState, gstBrushSize1, gstBrushSize2, gstBrushIter }, {-1});
}

void TreePanel::BrushClear()
{
	glbin_vol_selector.Clear();
	FluoRefresh(3, { gstNull });
}

void TreePanel::BrushExtract()
{
	glbin_vol_selector.Extract();
	FluoRefresh(3, { gstNull });
}

void TreePanel::BrushDelete()
{
	glbin_vol_selector.Erase();
	FluoRefresh(3, { gstNull });
}

void TreePanel::UpdateTree()
{
	if (!m_datatree)
		return;
	m_suppress_event = true;

	glbin_vol_selector.SetCopyMaskVolume(0);
	
	m_datatree->DeleteAllItems();
	m_datatree->ClearIcons();

	std::string root_str = "Scene Graph";
	wxTreeItemId root_item = m_datatree->AddRootItem(root_str);
	//append non-color icons for views
	m_datatree->AppendIcon();
	m_datatree->Expand(root_item);
	m_datatree->ChangeIconColor(0, wxColor(255, 255, 255));

	wxTreeItemId sel_item;
	int sel_type = glbin_current.GetType();

	if (sel_type == 0)
		sel_item = root_item;

	Root* root = glbin_data_manager.GetRoot();
	if (root)
	{
		for (int i = 0; i < root->GetViewNum(); i++)
		{
			auto view = root->GetView(i);
			if (!view)
				continue;
			int j, k;

			std::wstring view_name = view->GetName();
			view->OrganizeLayers();
			wxTreeItemId vrv_item = m_datatree->AddViewItem(view_name);
			m_datatree->SetViewItemImage(vrv_item, view->GetDraw());
			if (sel_type == 1 && glbin_current.render_view.lock() == view)
				sel_item = vrv_item;

			for (j = 0; j < view->GetLayerNum(); j++)
			{
				auto layer = view->GetLayer(j);
				switch (layer->IsA())
				{
				case 0://root
					break;
				case 1://view
					break;
				case 2://volume data
				{
					auto vd = std::dynamic_pointer_cast<VolumeData>(layer);
					if (!vd)
						break;
					//append icon for volume
					m_datatree->AppendIcon();
					fluo::Color c = vd->GetColor();
					wxColor wxc(
						(unsigned char)(c.r() * 255),
						(unsigned char)(c.g() * 255),
						(unsigned char)(c.b() * 255));
					int ii = m_datatree->GetIconNum() - 1;
					m_datatree->ChangeIconColor(ii, wxc);
					wxTreeItemId item = m_datatree->AddVolItem(vrv_item, vd->GetName());
					m_datatree->SetVolItemImage(item, vd->GetDisp() ? 2 * ii + 1 : 2 * ii);
					if (sel_type == 2 && glbin_current.vol_data.lock() == vd)
						sel_item = item;
				}
				break;
				case 3://mesh data
				{
					auto md = std::dynamic_pointer_cast<MeshData>(layer);
					if (!md)
						break;
					//append icon for mesh
					m_datatree->AppendIcon();
					fluo::Color amb, diff, spec;
					double shine, alpha;
					md->GetMaterial(amb, diff, spec, shine, alpha);
					wxColor wxc(
						(unsigned char)(diff.r() * 255),
						(unsigned char)(diff.g() * 255),
						(unsigned char)(diff.b() * 255));
					int ii = m_datatree->GetIconNum() - 1;
					m_datatree->ChangeIconColor(ii, wxc);
					wxTreeItemId item = m_datatree->AddMeshItem(vrv_item, md->GetName());
					m_datatree->SetMeshItemImage(item, md->GetDisp() ? 2 * ii + 1 : 2 * ii);
					if (sel_type == 3 && glbin_current.mesh_data.lock() == md)
						sel_item = item;
				}
				break;
				case 4://annotations
				{
					auto ann = std::dynamic_pointer_cast<Annotations>(layer);
					if (!ann)
						break;
					//append icon for annotations
					m_datatree->AppendIcon();
					wxColor wxc(255, 255, 255);
					int ii = m_datatree->GetIconNum() - 1;
					m_datatree->ChangeIconColor(ii, wxc);
					wxTreeItemId item = m_datatree->AddAnnotationItem(vrv_item, ann->GetName());
					m_datatree->SetAnnotationItemImage(item, ann->GetDisp() ? 2 * ii + 1 : 2 * ii);
					if (sel_type == 4 && glbin_current.ann_data.lock() == ann)
						sel_item = item;
				}
				break;
				case 5://group
				{
					auto group = std::dynamic_pointer_cast<DataGroup>(layer);
					if (!group)
						break;
					//append group item to tree
					wxTreeItemId group_item = m_datatree->AddGroupItem(vrv_item, group->GetName());
					m_datatree->SetGroupItemImage(group_item, int(group->GetDisp()));
					//append volume data to group
					for (k = 0; k < group->GetVolumeNum(); k++)
					{
						auto vd = group->GetVolumeData(k);
						if (!vd)
							continue;
						//add icon
						m_datatree->AppendIcon();
						fluo::Color c = vd->GetColor();
						wxColor wxc(
							(unsigned char)(c.r() * 255),
							(unsigned char)(c.g() * 255),
							(unsigned char)(c.b() * 255));
						int ii = m_datatree->GetIconNum() - 1;
						m_datatree->ChangeIconColor(ii, wxc);
						wxTreeItemId item = m_datatree->AddVolItem(group_item, vd->GetName());
						m_datatree->SetVolItemImage(item, vd->GetDisp() ? 2 * ii + 1 : 2 * ii);
						if (sel_type == 2 && glbin_current.vol_data.lock() == vd)
							sel_item = item;
					}
					if (sel_type == 5 && glbin_current.vol_group.lock() == group)
						sel_item = group_item;
				}
				break;
				case 6://mesh group
				{
					auto group = std::dynamic_pointer_cast<MeshGroup>(layer);
					if (!group)
						break;
					//append group item to tree
					wxTreeItemId group_item = m_datatree->AddMGroupItem(vrv_item, group->GetName());
					m_datatree->SetMGroupItemImage(group_item, int(group->GetDisp()));
					//append mesh data to group
					for (k = 0; k < group->GetMeshNum(); k++)
					{
						auto md = group->GetMeshData(k);
						if (!md)
							continue;
						//add icon
						m_datatree->AppendIcon();
						fluo::Color amb, diff, spec;
						double shine, alpha;
						md->GetMaterial(amb, diff, spec, shine, alpha);
						wxColor wxc(
							(unsigned char)(diff.r() * 255),
							(unsigned char)(diff.g() * 255),
							(unsigned char)(diff.b() * 255));
						int ii = m_datatree->GetIconNum() - 1;
						m_datatree->ChangeIconColor(ii, wxc);
						wxTreeItemId item = m_datatree->AddMeshItem(group_item, md->GetName());
						m_datatree->SetMeshItemImage(item, md->GetDisp() ? 2 * ii + 1 : 2 * ii);
						if (sel_type == 3 && glbin_current.mesh_data.lock() == md)
							sel_item = item;
					}
					if (sel_type == 6 && glbin_current.mesh_group.lock() == group)
						sel_item = group_item;
				}
				break;
				}
			}
		}
	}

	m_datatree->ExpandAll();
	m_datatree->SetScrollPos(wxVERTICAL, 0);

	if (sel_item.IsOk())
		m_datatree->SelectItemSilently(sel_item);

	m_suppress_event = false;
}

void TreePanel::UpdateTreeIcons()
{
	int i, j, k;

	wxTreeItemId root_item = m_datatree->GetRootItem();
	wxTreeItemIdValue ck_view;
	int counter = 0;

	Root* root = glbin_data_manager.GetRoot();
	if (root)
	{
		for (i = 0; i < root->GetViewNum(); i++)
		{
			auto view = root->GetView(i);
			wxTreeItemId vrv_item;
			if (i == 0)
				vrv_item = m_datatree->GetFirstChild(root_item, ck_view);
			else
				vrv_item = m_datatree->GetNextChild(root_item, ck_view);

			if (!vrv_item.IsOk())
				continue;

			m_datatree->SetViewItemImage(vrv_item, view->GetDraw());

			wxTreeItemIdValue ck_layer;
			for (j = 0; j < view->GetLayerNum(); j++)
			{
				auto layer = view->GetLayer(j);
				wxTreeItemId layer_item;
				if (j == 0)
					layer_item = m_datatree->GetFirstChild(vrv_item, ck_layer);
				else
					layer_item = m_datatree->GetNextChild(vrv_item, ck_layer);

				if (!layer_item.IsOk())
					continue;

				switch (layer->IsA())
				{
				case 2://volume
				{
					auto vd = std::dynamic_pointer_cast<VolumeData>(layer);
					if (!vd)
						break;
					counter++;
					m_datatree->SetVolItemImage(layer_item, vd->GetDisp() ? 2 * counter + 1 : 2 * counter);
				}
				break;
				case 3://mesh
				{
					auto md = std::dynamic_pointer_cast<MeshData>(layer);
					if (!md)
						break;
					counter++;
					m_datatree->SetMeshItemImage(layer_item, md->GetDisp() ? 2 * counter + 1 : 2 * counter);
				}
				break;
				case 4://annotations
				{
					auto ann = std::dynamic_pointer_cast<Annotations>(layer);
					if (!ann)
						break;
					counter++;
					m_datatree->SetAnnotationItemImage(layer_item, ann->GetDisp() ? 2 * counter + 1 : 2 * counter);
				}
				break;
				case 5://volume group
				{
					auto group = std::dynamic_pointer_cast<DataGroup>(layer);
					if (!group)
						break;
					m_datatree->SetGroupItemImage(layer_item, int(group->GetDisp()));
					wxTreeItemIdValue ck_volume;
					for (k = 0; k < group->GetVolumeNum(); k++)
					{
						auto vd = group->GetVolumeData(k);
						if (!vd)
							continue;
						wxTreeItemId volume_item;
						if (k == 0)
							volume_item = m_datatree->GetFirstChild(layer_item, ck_volume);
						else
							volume_item = m_datatree->GetNextChild(layer_item, ck_volume);
						if (!volume_item.IsOk())
							continue;
						counter++;
						m_datatree->SetVolItemImage(volume_item, vd->GetDisp() ? 2 * counter + 1 : 2 * counter);
					}
				}
				break;
				case 6://mesh group
				{
					auto group = std::dynamic_pointer_cast<MeshGroup>(layer);
					if (!group)
						break;
					m_datatree->SetMGroupItemImage(layer_item, int(group->GetDisp()));
					wxTreeItemIdValue ck_mesh;
					for (k = 0; k < group->GetMeshNum(); k++)
					{
						auto md = group->GetMeshData(k);
						if (!md)
							continue;
						wxTreeItemId mesh_item;
						if (k == 0)
							mesh_item = m_datatree->GetFirstChild(layer_item, ck_mesh);
						else
							mesh_item = m_datatree->GetNextChild(layer_item, ck_mesh);
						if (!mesh_item.IsOk())
							continue;
						counter++;
						m_datatree->SetMeshItemImage(mesh_item, md->GetDisp() ? 2 * counter + 1 : 2 * counter);
					}
				}
				break;
				}
			}
		}
	}
	Refresh(false);
}

void TreePanel::UpdateTreeColors()
{
	int i, j, k;
	int counter = 0;
	Root* root = glbin_data_manager.GetRoot();
	if (root)
	{
		for (i = 0; i < root->GetViewNum(); i++)
		{
			auto view = root->GetView(i);

			for (j = 0; j < view->GetLayerNum(); j++)
			{
				auto layer = view->GetLayer(j);
				switch (layer->IsA())
				{
				case 0://root
					break;
				case 1://view
					break;
				case 2://volume
				{
					auto vd = std::dynamic_pointer_cast<VolumeData>(layer);
					if (!vd)
						break;
					fluo::Color c = vd->GetColor();
					wxColor wxc(
						(unsigned char)(c.r() * 255),
						(unsigned char)(c.g() * 255),
						(unsigned char)(c.b() * 255));
					m_datatree->ChangeIconColor(counter + 1, wxc);
					counter++;
				}
				break;
				case 3://mesh
				{
					auto md = std::dynamic_pointer_cast<MeshData>(layer);
					if (!md)
						break;
					fluo::Color amb, diff, spec;
					double shine, alpha;
					md->GetMaterial(amb, diff, spec, shine, alpha);
					wxColor wxc(
						(unsigned char)(diff.r() * 255),
						(unsigned char)(diff.g() * 255),
						(unsigned char)(diff.b() * 255));
					m_datatree->ChangeIconColor(counter + 1, wxc);
					counter++;
				}
				break;
				case 4://annotations
				{
					auto ann = std::dynamic_pointer_cast<Annotations>(layer);
					if (!ann)
						break;
					wxColor wxc(255, 255, 255);
					m_datatree->ChangeIconColor(counter + 1, wxc);
					counter++;
				}
				break;
				case 5://group
				{
					auto group = std::dynamic_pointer_cast<DataGroup>(layer);
					if (!group)
						break;
					for (k = 0; k < group->GetVolumeNum(); k++)
					{
						auto vd = group->GetVolumeData(k);
						if (!vd)
							break;
						fluo::Color c = vd->GetColor();
						wxColor wxc(
							(unsigned char)(c.r() * 255),
							(unsigned char)(c.g() * 255),
							(unsigned char)(c.b() * 255));
						m_datatree->ChangeIconColor(counter + 1, wxc);
						counter++;
					}
				}
				break;
				case 6://mesh group
				{
					auto group = std::dynamic_pointer_cast<MeshGroup>(layer);
					if (!group)
						break;
					for (k = 0; k < group->GetMeshNum(); k++)
					{
						auto md = group->GetMeshData(k);
						if (!md)
							break;
						fluo::Color amb, diff, spec;
						double shine, alpha;
						md->GetMaterial(amb, diff, spec, shine, alpha);
						wxColor wxc(
							(unsigned char)(diff.r() * 255),
							(unsigned char)(diff.g() * 255),
							(unsigned char)(diff.b() * 255));
						m_datatree->ChangeIconColor(counter + 1, wxc);
						counter++;
					}
				}
				break;
				}
			}
		}
	}
	Refresh(false);
}

void TreePanel::UpdateTreeSel()
{
	wxTreeItemId root = m_datatree->GetRootItem();
	if (root.IsOk())
		traversalSel(root);
}

void TreePanel::traversalSel(wxTreeItemId item)
{
	LayerInfo* item_data = (LayerInfo*)m_datatree->GetItemData(item);
	int type = item_data->type;
	bool sel = false;
	switch (type)
	{
	case 0://root
		if (glbin_current.GetType() == 0)
			sel = true;
		break;
	case 1://view
		if (glbin_current.GetType() == 1)
		{
			std::wstring str1 = m_datatree->GetItemText(item).ToStdWstring();
			std::wstring str2;
			if (auto cur_view = glbin_current.render_view.lock())
				str2 = cur_view->GetName();
			if (str1 == str2)
				sel = true;
		}
		break;
	case 2://volume
		if (glbin_current.GetType() == 2)
		{
			std::wstring str1 = m_datatree->GetItemText(item).ToStdWstring();
			std::wstring str2;
			if (auto cur_vd = glbin_current.vol_data.lock())
				str2 = cur_vd->GetName();
			if (str1 == str2)
				sel = true;
		}
		break;
	case 3://mesh
		if (glbin_current.GetType() == 3)
		{
			std::wstring str1 = m_datatree->GetItemText(item).ToStdWstring();
			std::wstring str2;
			if (auto cur_md = glbin_current.mesh_data.lock())
				str2 = cur_md->GetName();
			if (str1 == str2)
				sel = true;
		}
		break;
	case 4://annotations
		if (glbin_current.GetType() == 4)
		{
			std::wstring str1 = m_datatree->GetItemText(item).ToStdWstring();
			std::wstring str2;
			if (auto cur_ann = glbin_current.ann_data.lock())
				str2 = cur_ann->GetName();
			if (str1 == str2)
				sel = true;
		}
		break;
	case 5://volume group
		if (glbin_current.GetType() == 5)
		{
			std::wstring str1 = m_datatree->GetItemText(item).ToStdWstring();
			std::wstring str2;
			if (auto cur_group = glbin_current.vol_group.lock())
				str2 = cur_group->GetName();
			if (str1 == str2)
				sel = true;
		}
		break;
	case 6://mesh group
		if (glbin_current.GetType() == 6)
		{
			std::wstring str1 = m_datatree->GetItemText(item).ToStdWstring();
			std::wstring str2;
			if (auto cur_group = glbin_current.mesh_group.lock())
				str2 = cur_group->GetName();
			if (str1 == str2)
				sel = true;
		}
		break;
	}

	if (sel)
	{
		m_datatree->SelectItemSilently(item);
		return;
	}

	wxTreeItemIdValue cookie;
	wxTreeItemId child_item = m_datatree->GetFirstChild(item, cookie);
	while (child_item.IsOk())
	{
		traversalSel(child_item);
		child_item = m_datatree->GetNextChild(item, cookie);
	}
}

void TreePanel::DeleteSelection()
{
	int type = glbin_current.GetType();
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;

	switch (type)
	{
	case 1://view
	{
		std::wstring name = view->GetName();
		m_frame->DeleteRenderViewPanel(name);
		Root* root = glbin_data_manager.GetRoot();
		if (root)
			root->DeleteView(name);
	}
		break;
	case 2://volume
	{
		auto vd = glbin_current.vol_data.lock();
		if (vd)
		{
			vd->SetDisp(true);
			std::wstring name = vd->GetName();
			view->RemoveVolumeData(name);
		}
	}
	break;
	case 3://mesh
	{
		auto md = glbin_current.mesh_data.lock();
		if (md)
		{
			md->SetDisp(true);
			std::wstring name = md->GetName();
			view->RemoveMeshData(name);
		}
	}
	break;
	case 4://annotations
	{
		auto ann = glbin_current.ann_data.lock();
		if (ann)
		{
			ann->SetDisp(true);
			std::wstring name = ann->GetName();
			view->RemoveAnnotations(name);
		}
	}
	break;
	case 5://volume group
	{
		auto group = glbin_current.vol_group.lock();
		if (group)
		{
			std::wstring name = group->GetName();
			view->RemoveGroup(name);
		}
	}
	break;
	case 6://mesh group
	{
		auto group = glbin_current.mesh_group.lock();
		if (group)
		{
			std::wstring name = group->GetName();
			view->RemoveGroup(name);
		}
	}
	}

	glbin_current.SetRoot();

	FluoRefresh(0, { gstTreeCtrl, gstListCtrl, gstMovViewList, gstMovViewIndex });
}

//delete
void TreePanel::DeleteAll()
{
	//delete all views other than the first one
	Root* root = glbin_data_manager.GetRoot();
	if (root)
	{
		for (int i = root->GetViewNum(); i > 1; --i)
		{
			m_frame->DeleteRenderViewPanel(i - 1);
			root->DeleteView(i - 1);
		}
	}

	root->GetView(0)->ClearAll();

	glbin_current.SetRoot();

	FluoRefresh(0, { gstTreeCtrl, gstListCtrl, gstMovViewList, gstMovViewIndex });
}

void TreePanel::Expand()
{
	wxTreeItemId sel_item = m_datatree->GetSelection();
	if (m_datatree->IsExpanded(sel_item))
		m_datatree->Collapse(sel_item);
	else
		m_datatree->Expand(sel_item);
}

void TreePanel::ToggleDisplay()
{
	int type = glbin_current.GetType();

	switch (type)
	{
	case 1://view
	{
		//view
		auto view = glbin_current.render_view.lock();
		if (view)
			view->ToggleDraw();
	}
	break;
	case 2://volume data
	{
		//volume
		auto view = glbin_current.render_view.lock();
		auto vd = glbin_current.vol_data.lock();
		if (view && vd)
		{
			vd->ToggleDisp();
			view->SetVolPopDirty();
		}
	}
	break;
	case 3://mesh data
	{
		//mesh
		auto view = glbin_current.render_view.lock();
		auto md = glbin_current.mesh_data.lock();
		if (view && md)
		{
			md->ToggleDisp();
			view->SetMeshPopDirty();
		}
	}
	break;
	case 4://annotations
	{
		auto ann = glbin_current.ann_data.lock();
		if (ann)
			ann->ToggleDisp();
	}
	break;
	case 5://volume group
	{
		//volume group
		auto view = glbin_current.render_view.lock();
		auto group = glbin_current.vol_group.lock();
		if (view && group)
		{
			group->ToggleDisp();
			view->SetVolPopDirty();
		}
	}
	break;
	case 6://mesh group
	{
		//mesh group
		auto view = glbin_current.render_view.lock();
		auto group = glbin_current.mesh_group.lock();
		if (view && group)
		{
			group->ToggleDisp();
			view->SetVolPopDirty();
		}
	}
	}

	m_scroll_pos = GetScrollPos(wxVERTICAL);
	SetScrollPos(wxVERTICAL, m_scroll_pos);

	FluoRefresh(0, { gstTreeIcons });
}

void TreePanel::RandomizeColor()
{
	int type = glbin_current.GetType();

	switch (type)
	{
	case 1://view
	{
		//view
		auto view = glbin_current.render_view.lock();
		if (view)
			view->RandomizeColor();
	}
	break;
	case 2://volume data
	{
		//volume
		auto vd = glbin_current.vol_data.lock();
		if (vd)
			vd->RandomizeColor();
	}
	break;
	case 3://mesh data
	{
		//mesh
		auto md = glbin_current.mesh_data.lock();
		if (md)
			md->RandomizeColor();
	}
	break;
	case 5://volume group
	{
		//volume group
		auto group = glbin_current.vol_group.lock();
		if (group)
			group->RandomizeColor();
	}
	break;
	case 6://mesh group
	{
		//mesh group
		auto group = glbin_current.mesh_group.lock();
		if (group)
			group->RandomizeColor();
	}
	}

	FluoRefresh(0, { gstTreeIcons });
}

void TreePanel::CloseView()
{
	auto view = glbin_current.render_view.lock();
	if (view)
	{
		std::wstring name = view->GetName();
		m_frame->DeleteRenderViewPanel(name);
		Root* root = glbin_data_manager.GetRoot();
		if (root)
			root->DeleteView(name);
	}

	glbin_current.SetRoot();
	FluoRefresh(0, { gstTreeCtrl, gstListCtrl, gstMovViewList, gstMovViewIndex });
}

void TreePanel::Isolate()
{
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;

	std::wstring name;
	int type = glbin_current.GetType();
	switch (type)
	{
	case 1://view
		break;
	case 2://volume
		if (auto cur_vd = glbin_current.vol_data.lock())
			name = cur_vd->GetName();
		break;
	case 3://mesh
		if (auto cur_md = glbin_current.mesh_data.lock())
			name = cur_md->GetName();
		break;
	case 4://annotations
		if (auto cur_ann = glbin_current.ann_data.lock())
			name = cur_ann->GetName();
		break;
	case 5://volume group
		if (auto cur_group = glbin_current.vol_group.lock())
			name = cur_group->GetName();
		break;
	case 6://mesh group
		if (auto cur_group = glbin_current.mesh_group.lock())
			name = cur_group->GetName();
		break;
	}

	view->Isolate(type, name);
	FluoRefresh(2, { gstTreeIcons });
}

void TreePanel::ShowAll()
{
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;

	view->ShowAll();
	FluoRefresh(2, { gstTreeIcons });
}

void TreePanel::ManipulateData()
{
	m_frame->UpdateProps({ gstManipPropPanel });
}

void TreePanel::OnContextMenu(wxContextMenuEvent& event)
{
	int flag;
	wxTreeItemId sel_item = m_datatree->HitTest(
		m_datatree->ScreenToClient(event.GetPosition()), flag);

	if (!sel_item.IsOk())
		return;

	m_datatree->SelectItem(sel_item);

	wxPoint point = event.GetPosition();
	// If from keyboard
	if (point.x == -1 && point.y == -1) {
		wxSize size = GetSize();
		point.x = size.x / 2;
		point.y = size.y / 2;
	}
	else {
		point = ScreenToClient(point);
	}

	wxMenu menu;
	int type = glbin_current.GetType();
	switch (type)
	{
	case 0:  //root
		if (m_datatree->IsExpanded(sel_item))
			menu.Append(TreePanel::ID_Expand, "Collapse");
		else
			menu.Append(TreePanel::ID_Expand, "Expand");
		break;
	case 1:  //view
	{
		menu.Append(TreePanel::ID_ToggleDisp, "Toggle Visibility");
		if (m_datatree->IsExpanded(sel_item))
			menu.Append(TreePanel::ID_Expand, "Collapse");
		else
			menu.Append(TreePanel::ID_Expand, "Expand");
		menu.AppendSeparator();
		menu.Append(TreePanel::ID_RandomizeColor, "Randomize Colors");
		menu.Append(TreePanel::ID_AddVolGroup, "Add Volume Group");
		menu.Append(TreePanel::ID_AddMeshGroup, "Add Mesh Group");
		Root* root = glbin_data_manager.GetRoot();
		wxString view_name;
		if (root)
			view_name = root->GetView(0)->GetName();
		if (m_datatree->GetItemText(sel_item) != view_name)
			menu.Append(TreePanel::ID_CloseView, "Close");
	}
	break;
	case 2:  //volume data
		menu.Append(TreePanel::ID_ToggleDisp, "Toggle Visibility");
		menu.Append(TreePanel::ID_Isolate, "Isolate");
		menu.Append(TreePanel::ID_ShowAll, "Show All");
		menu.AppendSeparator();
		menu.Append(TreePanel::ID_RandomizeColor, "Randomize Colors");
		menu.Append(TreePanel::ID_AddVolGroup, "Add Volume Group");
		menu.Append(TreePanel::ID_RemoveData, "Delete");
		menu.AppendSeparator();
		menu.Append(TreePanel::ID_CopyMask, "Copy Mask");
		if (glbin_vol_selector.GetCopyMaskVolume())
		{
			menu.Append(TreePanel::ID_PasteMask, "Paste Mask");
			menu.Append(TreePanel::ID_MergeMask, "Merge Mask");
			menu.Append(TreePanel::ID_ExcludeMask, "Exclude Mask");
			menu.Append(TreePanel::ID_IntersectMask, "Intersect Mask");
		}
		menu.AppendSeparator();
		menu.Append(TreePanel::ID_Ocl, "Volume Filter...");
		menu.Append(TreePanel::ID_Brush, "Paint Brush...");
		menu.Append(TreePanel::ID_Measurement, "Measurement...");
		menu.Append(TreePanel::ID_Component, "Component Analyzer...");
		menu.Append(TreePanel::ID_Track, "Tracking...");
		menu.Append(TreePanel::ID_Calculation, "Calculations...");
		menu.Append(TreePanel::ID_NoiseReduct, "Noise Reduction...");
		menu.Append(TreePanel::ID_VolumeSize, "Volume Size...");
		menu.Append(TreePanel::ID_Colocalization, "Colocalization...");
		menu.Append(TreePanel::ID_Convert, "Convert...");
		menu.Append(TreePanel::ID_MachineLearning, "Machine Learning Manager...");
		break;
	case 3:  //mesh data
		menu.Append(TreePanel::ID_ToggleDisp, "Toggle Visibility");
		menu.Append(TreePanel::ID_Isolate, "Isolate");
		menu.Append(TreePanel::ID_ShowAll, "Show All");
		menu.AppendSeparator();
		menu.Append(TreePanel::ID_RandomizeColor, "Randomize Colors");
		menu.Append(TreePanel::ID_AddMeshGroup, "Add Mesh Group");
		menu.Append(TreePanel::ID_RemoveData, "Delete");
		menu.AppendSeparator();
		menu.Append(TreePanel::ID_ManipulateData, "Manipulate");
		break;
	case 4:  //annotations
		break;
	case 5:  //data group
		menu.Append(TreePanel::ID_ToggleDisp, "Toggle Visibility");
		menu.Append(TreePanel::ID_Isolate, "Isolate");
		menu.Append(TreePanel::ID_ShowAll, "Show All");
		menu.AppendSeparator();
		if (m_datatree->IsExpanded(sel_item))
			menu.Append(TreePanel::ID_Expand, "Collapse");
		else
			menu.Append(TreePanel::ID_Expand, "Expand");
		menu.AppendSeparator();
		menu.Append(TreePanel::ID_RandomizeColor, "Randomize Colors");
		menu.Append(TreePanel::ID_AddVolGroup, "Add Volume Group");
		menu.Append(TreePanel::ID_RemoveData, "Delete");
		break;
	case 6:  //mesh group
		menu.Append(TreePanel::ID_ToggleDisp, "Toggle Visibility");
		menu.Append(TreePanel::ID_Isolate, "Isolate");
		menu.Append(TreePanel::ID_ShowAll, "Show All");
		menu.AppendSeparator();
		if (m_datatree->IsExpanded(sel_item))
			menu.Append(TreePanel::ID_Expand, "Collapse");
		else
			menu.Append(TreePanel::ID_Expand, "Expand");
		menu.AppendSeparator();
		menu.Append(TreePanel::ID_RandomizeColor, "Randomize Colors");
		menu.Append(TreePanel::ID_AddMeshGroup, "Add Mesh Group");
		menu.Append(TreePanel::ID_RemoveData, "Delete");
		break;
	}
	PopupMenu(&menu, point.x, point.y);
}

void TreePanel::OnToolbar(wxCommandEvent& event)
{
	int id = event.GetId();

	switch (id)
	{
	case ID_ToggleDisp:
		Action();
		break;
	case ID_AddVolGroup:
		AddVolGroup();
		break;
	case ID_AddMeshGroup:
		AddMeshGroup();
		break;
	case ID_RemoveData:
		RemoveData();
		break;
	case ID_RulerLine:
		RulerLine();
		break;
	case ID_RulerPolyline:
		RulerPolyline();
		break;
	case ID_RulerPencil:
		RulerPencil();
		break;
	case ID_RulerEdit:
		RulerEdit();
		break;
	case ID_RulerDeletePoint:
		RulerDeletePoint();
		break;
	case ID_BrushRuler:
		BrushRuler();
		break;
	case ID_BrushGrow:
		BrushGrow();
		break;
	case ID_BrushAppend:
		BrushAppend();
		break;
	case ID_BrushComp:
		BrushComp();
		break;
	case ID_BrushDiffuse:
		BrushDiffuse();
		break;
	case ID_BrushUnselect:
		BrushUnselect();
		break;
	case ID_BrushClear:
		BrushClear();
		break;
	case ID_BrushExtract:
		BrushExtract();
		break;
	case ID_BrushDelete:
		BrushDelete();
		break;
	case ID_Brush:
		m_frame->ShowBrushDlg();
		break;
	case ID_Measurement:
		m_frame->ShowMeasureDlg();
		break;
	case ID_Component:
		m_frame->ShowComponentDlg();
		break;
	case ID_Track:
		m_frame->ShowTrackDlg();
		break;
	case ID_Calculation:
		m_frame->ShowCalculationDlg();
		break;
	case ID_NoiseReduct:
		m_frame->ShowNoiseCancellingDlg();
		break;
	case ID_VolumeSize:
		m_frame->ShowCountingDlg();
		break;
	case ID_Colocalization:
		m_frame->ShowColocalizationDlg();
		break;
	case ID_Convert:
		m_frame->ShowConvertDlg();
		break;
	case ID_Ocl:
		m_frame->ShowOclDlg();
		break;
	case ID_MachineLearning:
		m_frame->ShowMachineLearningDlg();
		break;
	case ID_ManipulateData:
		ManipulateData();
		break;
	}
}

void TreePanel::OnMenu(wxCommandEvent& event)
{
	int id = event.GetId();
	int excl_self = 0;
	fluo::ValueCollection vc;
	std::set<int> views;

	switch (id)
	{
	case ID_ToggleDisp:
		Action();
		return;
	case ID_AddVolGroup:
		AddVolGroup();
		return;
	case ID_AddMeshGroup:
		AddMeshGroup();
		return;
	case ID_RemoveData:
		DeleteSelection();
		glbin_current.SetRoot();
		vc.insert(gstTreeCtrl);
		break;
	case ID_Expand:
		Expand();
		vc.insert(gstNull);
		views.insert(-1);
		break;
	case ID_RandomizeColor:
		RandomizeColor();
		vc.insert(gstTreeColors);
		break;
	case ID_CloseView:
		CloseView();
		break;
	case ID_Isolate:
	case ID_ShowAll:
		switch (id)
		{
		case ID_Isolate:
			Isolate();
			break;
		case ID_ShowAll:
			ShowAll();
			break;
		}
		excl_self = 2;
		vc.insert(gstTreeIcons);
		break;
	case ID_CopyMask:
		glbin_vol_selector.CopyMask(false);
		vc.insert(gstNull);
		views.insert(-1);
		break;
	case ID_PasteMask:
	case ID_MergeMask:
	case ID_ExcludeMask:
	case ID_IntersectMask:
	{
		int ival = 0;
		switch (id)
		{
		case ID_CopyMask:
			ival = 0;
			break;
		case ID_MergeMask:
			ival = 1;
			break;
		case ID_ExcludeMask:
			ival = 2;
			break;
		case ID_IntersectMask:
			ival = 3;
			break;
		}
		glbin_vol_selector.PasteMask(ival);
		if (glbin_vol_selector.GetAutoPaintSize())
			vc.insert(gstBrushCountResult);
		if (glbin_colocalizer.GetAutoColocalize())
			vc.insert(gstColocalResult);
	}
		break;
	case ID_Brush:
	case ID_Measurement:
	case ID_Component:
	case ID_Track:
	case ID_Calculation:
	case ID_NoiseReduct:
	case ID_VolumeSize:
	case ID_Colocalization:
	case ID_Convert:
	case ID_Ocl:
	case ID_MachineLearning:
	case ID_ManipulateData:
		switch (id)
		{
			case ID_Brush:
				m_frame->ShowBrushDlg();
				break;
			case ID_Measurement:
				m_frame->ShowMeasureDlg();
				break;
			case ID_Component:
				m_frame->ShowComponentDlg();
				break;
			case ID_Track:
				m_frame->ShowTrackDlg();
				break;
			case ID_Calculation:
				m_frame->ShowCalculationDlg();
				break;
			case ID_NoiseReduct:
				m_frame->ShowNoiseCancellingDlg();
				break;
			case ID_VolumeSize:
				m_frame->ShowCountingDlg();
				break;
			case ID_Colocalization:
				m_frame->ShowColocalizationDlg();
				break;
			case ID_Convert:
				m_frame->ShowConvertDlg();
				break;
			case ID_Ocl:
				m_frame->ShowOclDlg();
				break;
			case ID_MachineLearning:
				m_frame->ShowMachineLearningDlg();
				break;
			case ID_ManipulateData:
				ManipulateData();
				break;
		}
		excl_self = 3;
		vc.insert(gstNull);
		views.insert(-1);
		break;
	}

	FluoRefresh(excl_self, vc, views);
}

void TreePanel::OnSelChanged(wxTreeEvent& event)
{
	if (m_suppress_event)
		return;
	if (m_datatree && m_datatree->m_silent_select)
		return;

	Select();
	event.Skip();
}

void TreePanel::OnDeleting(wxTreeEvent& event)
{
	if (m_suppress_event)
		return;
	FluoUpdate({ gstCurrentSelect });
}

void TreePanel::OnAct(wxTreeEvent& event)
{
	Action();
}

void TreePanel::OnBeginDrag(wxTreeEvent& event)
{
	//remember pos
	m_scroll_pos = GetScrollPos(wxVERTICAL);

	m_drag_item = event.GetItem();
	if (m_drag_item.IsOk())
	{
		LayerInfo* item_data = (LayerInfo*)m_datatree->GetItemData(m_drag_item);
		if (item_data)
		{
			switch (item_data->type)
			{
			case 0://root
				break;
			case 1://view
				break;
			case 2://volume data
				event.Allow();
				break;
			case 3://mesh data
				event.Allow();
				break;
			case 4://annotations
				event.Allow();
				break;
			case 5://group
				event.Allow();
				break;
			case 6://mesh group
				event.Allow();
				break;
			}
		}
	}
}

void TreePanel::OnEndDrag(wxTreeEvent& event)
{
	wxTreeItemId src_item = m_drag_item,
		dst_item = event.GetItem(),
		src_par_item = src_item.IsOk() ? m_datatree->GetItemParent(src_item) : 0,
		dst_par_item = dst_item.IsOk() ? m_datatree->GetItemParent(dst_item) : 0;
	m_drag_item = (wxTreeItemId)0l;
	bool refresh = false;
	std::wstring src_name, src_par_name, dst_name, dst_par_name;
	Root* root = glbin_data_manager.GetRoot();

	if (src_item.IsOk() && dst_item.IsOk() &&
		src_par_item.IsOk() &&
		dst_par_item.IsOk() && m_frame)
	{
		int src_type = ((LayerInfo*)m_datatree->GetItemData(src_item))->type;
		int src_par_type = ((LayerInfo*)m_datatree->GetItemData(src_par_item))->type;
		int dst_type = ((LayerInfo*)m_datatree->GetItemData(dst_item))->type;
		int dst_par_type = ((LayerInfo*)m_datatree->GetItemData(dst_par_item))->type;

		src_name = m_datatree->GetItemText(src_item).ToStdWstring();
		src_par_name = m_datatree->GetItemText(src_par_item).ToStdWstring();
		dst_name = m_datatree->GetItemText(dst_item).ToStdWstring();
		dst_par_name = m_datatree->GetItemText(dst_par_item).ToStdWstring();

		if (src_par_type == 1 &&
			dst_par_type == 1 &&
			src_par_name == dst_par_name &&
			src_name != dst_name)
		{
			if (root)
			{
				auto view = root->GetView(src_par_name);
				//move within the same view
				if (view)
				{
					if (src_type == 2 && dst_type == 5)
					{
						//move volume to the group in the same view
						view->MoveLayertoGroup(dst_name, src_name, L"");
					}
					else if (src_type == 3 && dst_type == 6)
					{
						//move mesh into a group
						view->MoveMeshtoGroup(dst_name, src_name, L"");
					}
					else
					{
						view->MoveLayerinView(src_name, dst_name);
					}
				}
			}
		}
		else if (src_par_type == 5 &&
			dst_par_type == 5 &&
			src_par_name == dst_par_name &&
			src_name != dst_name)
		{
			//move volume within the same group
			std::wstring view_name = m_datatree->GetItemText(m_datatree->GetItemParent(src_par_item)).ToStdWstring();
			if (root)
			{
				auto view = root->GetView(view_name);
				if (view)
					view->MoveLayerinGroup(src_par_name, src_name, dst_name);
			}
		}
		else if (src_par_type == 5 && //par is group
			src_type == 2 && //src is volume
			dst_par_type == 1 && //dst's par is view
			dst_par_name == m_datatree->GetItemText(m_datatree->GetItemParent(src_par_item))) //in same view
		{
			if (root)
			{
				auto view = root->GetView(dst_par_name);
				//move volume outside of the group
				if (view)
				{
					if (dst_type == 5) //dst is group
					{
						view->MoveLayerfromtoGroup(src_par_name, dst_name, src_name, L"");
					}
					else
					{
						view->MoveLayertoView(src_par_name, src_name, dst_name);
					}
				}
			}
		}
		else if (src_par_type == 1 && //src's par is view
			src_type == 2 && //src is volume
			dst_par_type == 5 && //dst's par is group
			src_par_name == m_datatree->GetItemText(m_datatree->GetItemParent(dst_par_item))) //in the same view
		{
			//move volume into group
			if (root)
			{
				auto view = root->GetView(src_par_name);
				if (view)
					view->MoveLayertoGroup(dst_par_name, src_name, dst_name);
			}
		}
		else if (src_par_type == 5 && //src's par is group
			src_type == 2 && // src is volume
			dst_par_type == 5 && //dst's par is group
			dst_type == 2 && //dst is volume
			m_datatree->GetItemText(m_datatree->GetItemParent(src_par_item)) == m_datatree->GetItemText(m_datatree->GetItemParent(dst_par_item)) && // in the same view
			m_datatree->GetItemText(src_par_item) != m_datatree->GetItemText(dst_par_item))// par groups are different
		{
			//move volume from one group to another
			std::wstring view_name = m_datatree->GetItemText(m_datatree->GetItemParent(src_par_item)).ToStdWstring();
			if (root)
			{
				auto view = root->GetView(view_name);
				if (view)
					view->MoveLayerfromtoGroup(src_par_name, dst_par_name, src_name, dst_name);
			}
		}
		else if (src_type == 2 && //src is volume
			src_par_type == 5 && //src's par is group
			dst_type == 1 && //dst is view
			m_datatree->GetItemText(m_datatree->GetItemParent(src_par_item)) == dst_name) //in the same view
		{
			//move volume outside of the group
			if (root)
			{
				auto view = root->GetView(dst_name);
				if (view)
				{
					view->MoveLayertoView(src_par_name, src_name, L"");
				}
			}
		}
		else if (src_par_type == 6 &&
			dst_par_type == 6 &&
			src_par_name == dst_par_name &&
			src_name != dst_name)
		{
			//move mesh within the same group
			std::wstring view_name = m_datatree->GetItemText(m_datatree->GetItemParent(src_par_item)).ToStdWstring();
			if (root)
			{
				auto view = root->GetView(view_name);
				if (view)
					view->MoveMeshinGroup(src_par_name, src_name, dst_name);
			}
		}
		else if (src_par_type == 6 && //par is group
			src_type == 3 && //src is mesh
			dst_par_type == 1 && //dst's par is view
			dst_par_name == m_datatree->GetItemText(m_datatree->GetItemParent(src_par_item))) //in same view
		{
			//move mesh outside of the group
			if (dst_type == 6) //dst is group
			{
				if (root)
				{
					auto view = root->GetView(dst_par_name);
					if (view)
					{
						view->MoveMeshfromtoGroup(src_par_name, dst_name, src_name, L"");
					}
				}
			}
			else
			{
				if (root)
				{
					auto view = root->GetView(dst_par_name);
					if (view)
						view->MoveMeshtoView(src_par_name, src_name, dst_name);
				}
			}
		}
		else if (src_par_type == 1 && //src's par is view
			src_type == 3 && //src is mesh
			dst_par_type == 6 && //dst's par is group
			src_par_name == m_datatree->GetItemText(m_datatree->GetItemParent(dst_par_item))) //in the same view
		{
			//move mesh into group
			if (root)
			{
				auto view = root->GetView(src_par_name);
				if (view)
					view->MoveMeshtoGroup(dst_par_name, src_name, dst_name);
			}
		}
		else if (src_par_type == 6 && //src's par is group
			src_type == 3 && // src is mesh
			dst_par_type == 6 && //dst's par is group
			dst_type == 3 && //dst is mesh
			m_datatree->GetItemText(m_datatree->GetItemParent(src_par_item)) == m_datatree->GetItemText(m_datatree->GetItemParent(dst_par_item)) && // in the same view
			m_datatree->GetItemText(src_par_item) != m_datatree->GetItemText(dst_par_item))// par groups are different
		{
			//move mesh from one group to another
			std::wstring view_name = m_datatree->GetItemText(m_datatree->GetItemParent(src_par_item)).ToStdWstring();
			if (root)
			{
				auto view = root->GetView(view_name);
				if (view)
					view->MoveMeshfromtoGroup(src_par_name, dst_par_name, src_name, dst_name);
			}
		}
		else if (src_type == 3 && //src is mesh
			src_par_type == 6 && //src's par is group
			dst_type == 1 && //dst is view
			m_datatree->GetItemText(m_datatree->GetItemParent(src_par_item)) == dst_name) //in the same view
		{
			//move mesh outside of the group
			if (root)
			{
				auto view = root->GetView(dst_name);
				if (view)
				{
					view->MoveMeshtoView(src_par_name, src_name, L"");
				}
			}
		}

		//glbin.set_tree_selection(src_name.ToStdString());
		refresh = true;
	}
	else if (src_item.IsOk() && src_par_item.IsOk() &&
		!dst_item.IsOk() && m_frame)
	{
		//move volume out of the group
		int src_type = ((LayerInfo*)m_datatree->GetItemData(src_item))->type;
		int src_par_type = ((LayerInfo*)m_datatree->GetItemData(src_par_item))->type;

		src_name = m_datatree->GetItemText(src_item);
		src_par_name = m_datatree->GetItemText(src_par_item);

		if (src_type == 2 && src_par_type == 5)
		{
			std::wstring view_name = m_datatree->GetItemText(m_datatree->GetItemParent(src_par_item)).ToStdWstring();
			if (root)
			{
				auto view = root->GetView(view_name);
				if (view)
				{
					view->MoveLayertoView(src_par_name, src_name, L"");

					refresh = true;
				}
			}
		}
	}

	if (refresh)
	{
		FluoUpdate({ gstTreeCtrl });
		glbin_current.SetSel(src_name);
		FluoRefresh(0, { gstCurrentSelect, gstUpdateSync });
	}

	SetScrollPos(wxVERTICAL, m_scroll_pos);
}

void TreePanel::OnKeyDown(wxKeyEvent& event)
{
	if (event.GetKeyCode() == WXK_DELETE ||
		event.GetKeyCode() == WXK_BACK)
		DeleteSelection();
	glbin_current.SetRoot();
	FluoRefresh(0, { gstTreeCtrl });
}

