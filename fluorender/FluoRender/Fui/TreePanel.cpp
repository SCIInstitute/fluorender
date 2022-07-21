/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2022 Scientific Computing and Imaging Institute,
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
#include <RenderFrame.h>
#include <DataViewColorRenderer.h>
//resources
#include <png_resource.h>
#include <img/icons.h>
#include <img/tick.xpm>
#include <img/cross.xpm>
#include <boost/algorithm/string.hpp>

BEGIN_EVENT_TABLE(TreePanel, wxPanel)
EVT_DATAVIEW_SELECTION_CHANGED(ID_TreeCtrl, TreePanel::OnSelectionChanged)
EVT_DATAVIEW_ITEM_BEGIN_DRAG(ID_TreeCtrl, TreePanel::OnBeginDrag)
EVT_DATAVIEW_ITEM_DROP_POSSIBLE(ID_TreeCtrl, TreePanel::OnDropPossible)
EVT_DATAVIEW_ITEM_DROP(ID_TreeCtrl, TreePanel::OnDrop)
EVT_DATAVIEW_ITEM_ACTIVATED(ID_TreeCtrl, TreePanel::OnActivated)
EVT_DATAVIEW_COLUMN_SORTED(ID_TreeCtrl, TreePanel::OnSorted)
EVT_DATAVIEW_COLUMN_HEADER_RIGHT_CLICK(ID_TreeCtrl, TreePanel::OnHeaderRightClick)
EVT_TOOL(ID_ToggleView, TreePanel::OnToggleView)
EVT_TOOL(ID_AddGroup, TreePanel::OnAddGroup)
EVT_TOOL(ID_AddMGroup, TreePanel::OnAddMGroup)
EVT_TOOL(ID_RemoveData, TreePanel::OnRemoveData)
//brush commands
EVT_TOOL(ID_BrushAppend, TreePanel::OnBrushAppend)
EVT_TOOL(ID_BrushDesel, TreePanel::OnBrushDesel)
EVT_TOOL(ID_BrushDiffuse, TreePanel::OnBrushDiffuse)
EVT_TOOL(ID_BrushClear, TreePanel::OnBrushClear)
EVT_TOOL(ID_BrushErase, TreePanel::OnBrushErase)
EVT_TOOL(ID_BrushCreate, TreePanel::OnBrushCreate)
END_EVENT_TABLE()

TreePanel::TreePanel(
	wxWindow* parent,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name) :
	wxPanel(parent, wxID_ANY, pos, size, style, name),
	m_frame(parent),
	m_tree_model(nullptr),
	m_brushtool_agent(nullptr)
{
	wxEventBlocker blocker(this);
	SetDoubleBuffered(true);

	//create tool bar
	m_toolbar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxTB_FLAT | wxTB_TOP | wxTB_NODIVIDER);
	wxBitmap bitmap = wxGetBitmapFromMemory(toggle_disp);
#ifdef _DARWIN
	m_toolbar->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_toolbar->AddTool(ID_ToggleView, "Toggle View", bitmap,
		"Toggle the visibility of current selection");
	bitmap = wxGetBitmapFromMemory(add_group);
	m_toolbar->AddTool(ID_AddGroup, "Add Group", bitmap,
		"Add a volume data group to the selected view");
	bitmap = wxGetBitmapFromMemory(add_mgroup);
	m_toolbar->AddTool(ID_AddMGroup, "Add Mesh Group", bitmap,
		"Add a mesh data group to the selected view");
	bitmap = wxGetBitmapFromMemory(delet);
	m_toolbar->AddTool(ID_RemoveData, "Delete", bitmap,
		"Delete current selection");
	m_toolbar->AddSeparator();
	bitmap = wxGetBitmapFromMemory(brush_append);
	m_toolbar->AddCheckTool(ID_BrushAppend, "Highlight",
		bitmap, wxNullBitmap,
		"Highlight structures by painting on the render view (hold Shift)");
	bitmap = wxGetBitmapFromMemory(brush_diffuse);
	m_toolbar->AddCheckTool(ID_BrushDiffuse, "Diffuse",
		bitmap, wxNullBitmap,
		"Diffuse highlighted structures by painting (hold Z)");
	bitmap = wxGetBitmapFromMemory(brush_desel);
	m_toolbar->AddCheckTool(ID_BrushDesel, "Reset",
		bitmap, wxNullBitmap,
		"Reset highlighted structures by painting (hold X)");
	bitmap = wxGetBitmapFromMemory(brush_clear);
	m_toolbar->AddTool(ID_BrushClear, "Reset All",
		bitmap, "Reset all highlighted structures");
	m_toolbar->AddSeparator();
	bitmap = wxGetBitmapFromMemory(brush_erase);
	m_toolbar->AddTool(ID_BrushErase, "Erase",
		bitmap, "Erase highlighted structures");
	bitmap = wxGetBitmapFromMemory(brush_create);
	m_toolbar->AddTool(ID_BrushCreate, "Extract", bitmap,
		"Extract highlighted structures out and create a new volume");
	m_toolbar->Realize();

	//create data tree
	m_tree_ctrl = new wxDataViewCtrl(this, ID_TreeCtrl,
		wxDefaultPosition, wxDefaultSize,
		wxDV_MULTIPLE | wxDV_ROW_LINES);
	m_tree_ctrl->EnableDragSource(wxDF_UNICODETEXT);
	m_tree_ctrl->EnableDropTarget(wxDF_UNICODETEXT);
	m_tree_ctrl->SetDoubleBuffered(true);
	//add columns
	//name
	wxDataViewIconTextRenderer *itr =
		new wxDataViewIconTextRenderer();
	wxDataViewColumn *column0 =
		new wxDataViewColumn("Name", itr, 0, 200, wxALIGN_LEFT,
			wxDATAVIEW_COL_SORTABLE |
			wxDATAVIEW_COL_REORDERABLE |
			wxDATAVIEW_COL_RESIZABLE);
	m_tree_ctrl->AppendColumn(column0);
	//type
	DataViewColorRenderer *cr = new DataViewColorRenderer(wxDATAVIEW_CELL_ACTIVATABLE);
	wxDataViewColumn *column1 =
		new wxDataViewColumn("Color", cr, 1, 200, wxALIGN_LEFT,
			wxDATAVIEW_COL_REORDERABLE |
			wxDATAVIEW_COL_RESIZABLE);
	m_tree_ctrl->AppendColumn(column1);
	//m_tree_ctrl->AllowMultiColumnSort(true);

	//organize positions
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
	sizer_v->Add(m_toolbar, 0, wxEXPAND);
	sizer_v->Add(m_tree_ctrl, 1, wxEXPAND);
	SetSizer(sizer_v);
	Layout();
}

TreePanel::~TreePanel()
{
}

void TreePanel::SetScenegraph(void* root)
{
	if (!m_tree_model)
		return;
	m_tree_ctrl->AssociateModel(m_tree_model);
	m_tree_model->ItemAdded(
		wxDataViewItem(0),
		wxDataViewItem((void*)m_tree_model->getObject()));
	m_tree_ctrl->Expand(wxDataViewItem(root));
}

//seelction
void TreePanel::UpdateSelection()
{

}

wxString TreePanel::GetCurrentSel()
{
	return "";
}

void TreePanel::Select(wxString view, wxString name)
{

}

void TreePanel::SelectBrush(int id)
{
	m_toolbar->ToggleTool(ID_BrushAppend, false);
	m_toolbar->ToggleTool(ID_BrushDiffuse, false);
	m_toolbar->ToggleTool(ID_BrushDesel, false);
	//m_datatree->m_fixed = false;

	switch (id)
	{
	case ID_BrushAppend:
		m_toolbar->ToggleTool(ID_BrushAppend, true);
		//m_datatree->m_fixed = true;
		break;
	case ID_BrushDiffuse:
		m_toolbar->ToggleTool(ID_BrushDiffuse, true);
		//m_datatree->m_fixed = true;
		break;
	case ID_BrushDesel:
		m_toolbar->ToggleTool(ID_BrushDesel, true);
		//m_datatree->m_fixed = true;
		break;
	}
}

int TreePanel::GetBrushSelected()
{
	if (m_toolbar->GetToolState(ID_BrushAppend))
		return ID_BrushAppend;
	else if (m_toolbar->GetToolState(ID_BrushDiffuse))
		return ID_BrushDiffuse;
	else if (m_toolbar->GetToolState(ID_BrushDesel))
		return ID_BrushDesel;
	else
		return 0;
}

void TreePanel::OnSelectionChanged(wxDataViewEvent &event)
{
	wxDataViewItem evt_item = m_tree_ctrl->GetCurrentItem();//event item, could be selection or deselection
	wxDataViewItem cur_item = m_tree_ctrl->GetSelection();//selected item, could be 0
	wxDataViewItemArray items;//all selected items
	int num = m_tree_ctrl->GetSelections(items);
	if (!cur_item.GetID() && evt_item.GetID())
		cur_item = evt_item;
	fluo::NodeSet nodes;
	for (auto it = items.begin(); it != items.end(); ++it)
	{
		nodes.insert(static_cast<fluo::Node*>(it->GetID()));
	}
	m_tree_model->UpdateSelections(nodes);

	RenderFrame* vr_frame = (RenderFrame*)m_frame;

	if (!vr_frame)
		return;

	fluo::Node* node = static_cast<fluo::Node*>(cur_item.GetID());
	std::string name(node->getName());
	vr_frame->OnSelection(name);

}

void TreePanel::OnBeginDrag(wxDataViewEvent &event)
{
	wxDataViewItem item(event.GetItem());
	fluo::Referenced* refd = static_cast<fluo::Referenced*>(item.GetID());
	if (!refd)
	{
		event.Veto();
		return;
	}
	fluo::Node* node = dynamic_cast<fluo::Node*>(refd);
	if (!node)
	{
		event.Veto();
		return;
	}
	//multiple selections
	wxDataViewItemArray sel;
	m_tree_ctrl->GetSelections(sel);
	wxString names;
	for (auto it = sel.begin();
		it != sel.end(); ++it)
	{
		fluo::Node* sel_node = (fluo::Node*)it->GetID();
		if (sel_node)
		{
			names += sel_node->getName();
			if (it != std::prev(sel.end()))
				names += '\n';
		}
	}

	wxTextDataObject *wxobj = new wxTextDataObject;
	wxobj->SetText(names);
	event.SetDataObject(wxobj);
	event.SetDragFlags(wxDrag_AllowMove); // allows both copy and move
}

void TreePanel::OnDropPossible(wxDataViewEvent &event)
{
	event.Allow();
}

void TreePanel::OnDrop(wxDataViewEvent &event)
{
	wxDataViewItem item(event.GetItem());
	fluo::Node* target_node = (fluo::Node*)item.GetID();

	wxTextDataObject wxobj;
	wxobj.SetData(wxDF_UNICODETEXT, event.GetDataSize(), event.GetDataBuffer());
	wxString source_names = wxobj.GetText();
	std::vector<std::string> source_name_list;
	boost::split(source_name_list, source_names.ToStdString(), [](char c) {return c == '\n'; });
	for (auto it = source_name_list.begin();
		it != source_name_list.end(); ++it)
		m_tree_model->MoveNode(*it, target_node);
}

void TreePanel::OnActivated(wxDataViewEvent &event)
{
	wxDataViewItem item(event.GetItem());
	int col = event.GetColumn();
	switch (col)
	{
	case 0:
	{
		fluo::Node* node = (fluo::Node*)item.GetID();
		if (node)
		{
			bool display;
			node->flipUpdateValue(gstDisplay, display);
			//for showing/hiding the property panel
			//this should be improved when frame has its agent
			RenderFrame* vr_frame = (RenderFrame*)m_frame;
			if (vr_frame)
				vr_frame->OnSelection(node->getName());
		}
	}
	break;
	case 1:
	{
		fluo::Node* node = (fluo::Node*)item.GetID();
		if (node)
		{
			bool rc;
			node->flipUpdateValue(gstRandomizeColor, rc);
		}
	}
	break;
	}
}

void TreePanel::OnSorted(wxDataViewEvent &event)
{
	wxDataViewColumn* col = event.GetDataViewColumn();
	if (!col)
		return;
	unsigned int pos = col->GetModelColumn();
	bool is_sorted = col->IsSortable();
	bool is_asc = col->IsSortOrderAscending();
	std::string sort_value;
	switch (pos)
	{
	case 0:
		sort_value = "name";
		break;
	}
	long sort_method;
	if (is_sorted)
		sort_method = is_asc ? 1 : 2;
	else
		sort_method = 0;
	m_tree_model->updateValue(gstSortValue, sort_value);
	m_tree_model->updateValue(gstSortMethod, sort_method);
}

void TreePanel::OnHeaderRightClick(wxDataViewEvent &event)
{
	wxDataViewColumn* col = event.GetDataViewColumn();
	if (!col)
		return;
	col->UnsetAsSortKey();
	m_tree_model->updateValue(gstSortMethod, long(0));
	event.Skip();
}

void TreePanel::OnToggleView(wxCommandEvent &event)
{
}

void TreePanel::OnAddGroup(wxCommandEvent &event)
{
}

void TreePanel::OnAddMGroup(wxCommandEvent &event)
{
}

void TreePanel::OnRemoveData(wxCommandEvent &event)
{
}

void TreePanel::OnBrushAppend(wxCommandEvent &event)
{
	bool bval = m_toolbar->GetToolState(ID_BrushAppend);
	if (bval)
	{
		m_brushtool_agent->setValue(gstInterMode, long(2));
		m_brushtool_agent->setValue(gstPaintMode, long(2));
	}
	else
	{
		m_brushtool_agent->setValue(gstInterMode, long(1));
	}
}

void TreePanel::OnBrushDiffuse(wxCommandEvent &event)
{
	bool bval = m_toolbar->GetToolState(ID_BrushDiffuse);
	if (bval)
	{
		m_brushtool_agent->setValue(gstInterMode, long(2));
		m_brushtool_agent->setValue(gstPaintMode, long(4));
	}
	else
	{
		m_brushtool_agent->setValue(gstInterMode, long(1));
	}
}

void TreePanel::OnBrushDesel(wxCommandEvent &event)
{
	bool bval = m_toolbar->GetToolState(ID_BrushDesel);
	if (bval)
	{
		m_brushtool_agent->setValue(gstInterMode, long(2));
		m_brushtool_agent->setValue(gstPaintMode, long(3));
	}
	else
	{
		m_brushtool_agent->setValue(gstInterMode, long(1));
	}
}

void TreePanel::OnBrushClear(wxCommandEvent &event)
{
	m_brushtool_agent->BrushClear();
}

void TreePanel::OnBrushErase(wxCommandEvent &event)
{
	m_brushtool_agent->BrushErase();
}

void TreePanel::OnBrushCreate(wxCommandEvent &event)
{
	m_brushtool_agent->BrushCreate();
}
