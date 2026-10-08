/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2026 Scientific Computing and Imaging Institute,
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
#include <TreePanelAgent.h>
#include <Ruler.h>
#include <RenderView.h>
#include <VolumeSelector.h>
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
wxTreeItemId DataTreeCtrl::AddAnnotItem(wxTreeItemId par_item, const wxString &text)
{
	wxTreeItemId item = AppendItem(par_item, text, 1);
	LayerInfo* item_data = new LayerInfo;
	item_data->type = 4;//annotations
	SetItemData(item, item_data);
	return item;
}

void DataTreeCtrl::SetAnnotItemImage(const wxTreeItemId item, int image)
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
TreePanel::TreePanel(
	wxWindow* parent,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name) :
	AgentPanel(parent, pos, size, style, name),
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
	bitmap = wxGetBitmap(locator);
	m_toolbar->AddCheckTool(ID_RulerLocator, "Locator",
		bitmap, wxNullBitmap,
		"Add locators by clicking on data",
		"Add locators by clicking on data");
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
	bitmap = wxGetBitmap(mesh_convert);
	m_toolbar2->AddTool(ID_MeshConvert, "Convert",
		bitmap, "Convert volume to mesh");
	m_toolbar2->SetToolLongHelp(ID_MeshConvert, "Convert volume to mesh");
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

	//organize positions
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);

	sizer_v->Add(m_toolbar, 0, wxEXPAND | wxLEFT, 20);
	sizer_v->Add(m_toolbar2, 0, wxEXPAND | wxLEFT, 20);
	sizer_v->Add(m_datatree, 1, wxEXPAND);

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

void TreePanel::UpdateFreehandToolState(InteractiveMode int_mode,
	flrd::SelectMode sel_mode, flrd::RulerMode rul_mode)
{
	bool bval;
	m_toolbar->ToggleTool(ID_RulerLocator, rul_mode == flrd::RulerMode::Locator);
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

void TreePanel::UpdateTree(
	const TreeUpdateData& data)
{
	if (!m_datatree)
		return;

	m_suppress_event = true;

	itemMap_.clear();
	itemInfo_.clear();

	m_datatree->DeleteAllItems();
	m_datatree->ClearIcons();

	BuildTreeItem(data.root);

	m_datatree->ExpandAll();
	m_datatree->SetScrollPos(wxVERTICAL, 0);

	m_suppress_event = false;
}

void TreePanel::UpdateTreeIcons(
	const TreeIconUpdateData& data)
{
	for (const auto& item : data.items)
		UpdateItemIcon(item.id, item.visible);

	Refresh(false);
}

void TreePanel::UpdateTreeColors(
	const TreeColorUpdateData& data)
{
	for (const auto& item : data.items)
		UpdateItemColor(item.id, item.color);

	Refresh(false);
}

void TreePanel::UpdateTreeSelection(
	const TreeSelectionData& data)
{
	SelectItem(data.selectedId);
}

namespace
{
	void BuildWxMenu(wxMenu& menu, const MenuData& data)
	{
		for (const auto& item : data)
		{
			switch (item.type)
			{
			case MenuItemData::Type::Separator:
			{
				menu.AppendSeparator();
				break;
			}

			case MenuItemData::Type::Action:
			{
				wxMenuItem* menu_item =
					menu.Append(item.id, item.label);

				menu_item->Enable(item.enabled);

				if (item.checked)
				{
					menu_item->SetItemLabel(item.label);
					menu_item->Check(true);
				}

				break;
			}

			case MenuItemData::Type::SubMenu:
			{
				auto submenu = std::make_unique<wxMenu>();

				BuildWxMenu(*submenu, item.children);

				menu.AppendSubMenu(
					submenu.release(),
					item.label);

				break;
			}
			}
		}
	}
}

void TreePanel::UpdateExpandSelectedItem()
{
	auto sel_item = m_datatree->GetSelection();
	if (!sel_item.IsOk())
		return;
	if (m_datatree->IsExpanded(sel_item))
		m_datatree->Collapse(sel_item);
	else
		m_datatree->Expand(sel_item);
}

void TreePanel::UpdateScrollPos()
{
	m_scroll_pos = GetScrollPos(wxVERTICAL);
	SetScrollPos(wxVERTICAL, m_scroll_pos);
}

void TreePanel::ShowContextMenu(const MenuData& data)
{
	wxMenu menu;

	BuildWxMenu(menu, data);

	PopupMenu(&menu, m_context_pos.x, m_context_pos.y);
}

bool TreePanel::GetTreeExpanded()
{
	wxTreeItemId sel_item = m_datatree->GetSelection();
	if (sel_item.IsOk())
		return m_datatree->IsExpanded(sel_item);
	return false;
}

std::wstring TreePanel::GetSelItemText()
{
	wxTreeItemId sel_item = m_datatree->GetSelection();
	if (sel_item.IsOk())
		return m_datatree->GetItemText(sel_item).ToStdWstring();
	return L"";
}

std::wstring TreePanel::GetSelItemParentText()
{
	wxTreeItemId sel_item = m_datatree->GetSelection();
	if (sel_item.IsOk())
		return m_datatree->GetItemText(m_datatree->GetItemParent(sel_item)).ToStdWstring();
	return L"";
}

LayerInfo* TreePanel::GetSelItemData()
{
	wxTreeItemId sel_item = m_datatree->GetSelection();
	if (sel_item.IsOk())
		return static_cast<LayerInfo*>(m_datatree->GetItemData(sel_item));
	return nullptr;
}

bool TreePanel::GetCtrlDown()
{
	return wxGetKeyState(WXK_CONTROL);
}

wxTreeItemId TreePanel::BuildTreeItem(
	const TreeItemData& node,
	wxTreeItemId parent)
{
	wxTreeItemId item;

	int iconIndex = m_datatree->GetIconNum();
	wxColor wxc(node.color.r, node.color.g, node.color.b);

	switch (node.type)
	{
	case TreeNodeType::Root:
		item = m_datatree->AddRootItem(node.name);
		break;

	case TreeNodeType::View:
		item = m_datatree->AddViewItem(node.name);
		m_datatree->SetViewItemImage(
			item,
			node.visible);
		break;

	case TreeNodeType::Volume:
	{
		m_datatree->ChangeIconColor(iconIndex, wxc);

		item = m_datatree->AddVolItem(
			parent,
			node.name);

		m_datatree->SetVolItemImage(
			item,
			node.visible ?
			2 * iconIndex + 1 :
			2 * iconIndex);
	}
	break;

	case TreeNodeType::Mesh:
	{
		m_datatree->ChangeIconColor(iconIndex, wxc);

		item = m_datatree->AddMeshItem(
			parent,
			node.name);

		m_datatree->SetMeshItemImage(
			item,
			node.visible ?
			2 * iconIndex + 1 :
			2 * iconIndex);
	}
	break;

	case TreeNodeType::Annotation:
	{
		m_datatree->ChangeIconColor(iconIndex, wxc);

		item = m_datatree->AddAnnotItem(
			parent,
			node.name);

		m_datatree->SetAnnotItemImage(
			item,
			node.visible ?
			2 * iconIndex + 1 :
			2 * iconIndex);
	}
	break;

	case TreeNodeType::VolumeGroup:
		item = m_datatree->AddGroupItem(
			parent,
			node.name);

		m_datatree->SetGroupItemImage(
			item,
			int(node.visible));
		break;

	case TreeNodeType::MeshGroup:
		item = m_datatree->AddMGroupItem(
			parent,
			node.name);

		m_datatree->SetMGroupItemImage(
			item,
			int(node.visible));
		break;
	}

	itemMap_[node.id] = item;

	itemInfo_[node.id] =
	{
		node.type,
		iconIndex
	};

	for (const auto& child : node.children)
		BuildTreeItem(child, item);

	return item;
}

void TreePanel::SelectItem(TreeNodeId id)
{
	auto it = itemMap_.find(id);

	if (it == itemMap_.end())
		return;

	m_datatree->SelectItemSilently(
		it->second);
}

void TreePanel::UpdateItemColor(
	TreeNodeId id,
	const TreeColor& color)
{
	auto info_it = itemInfo_.find(id);

	if (info_it == itemInfo_.end())
		return;

	int iconIndex = info_it->second.iconIndex;

	if (iconIndex < 0)
		return;

	wxColor wxc(
		color.r,
		color.g,
		color.b);

	m_datatree->ChangeIconColor(
		iconIndex,
		wxc);
}

void TreePanel::UpdateItemIcon(
	TreeNodeId id,
	bool visible)
{
	auto item_it = itemMap_.find(id);

	if (item_it == itemMap_.end())
		return;

	auto info_it = itemInfo_.find(id);

	if (info_it == itemInfo_.end())
		return;

	auto& info = info_it->second;

	switch (info.type)
	{
	case TreeNodeType::View:
		m_datatree->SetViewItemImage(
			item_it->second,
			visible);
		break;

	case TreeNodeType::Volume:
		m_datatree->SetVolItemImage(
			item_it->second,
			visible ?
			2 * info.iconIndex + 1 :
			2 * info.iconIndex);
		break;

	case TreeNodeType::Mesh:
		m_datatree->SetMeshItemImage(
			item_it->second,
			visible ?
			2 * info.iconIndex + 1 :
			2 * info.iconIndex);
		break;

	case TreeNodeType::Annotation:
		m_datatree->SetAnnotItemImage(
			item_it->second,
			visible ?
			2 * info.iconIndex + 1 :
			2 * info.iconIndex);
		break;

	case TreeNodeType::VolumeGroup:
		m_datatree->SetGroupItemImage(
			item_it->second,
			int(visible));
		break;

	case TreeNodeType::MeshGroup:
		m_datatree->SetMGroupItemImage(
			item_it->second,
			int(visible));
		break;

	default:
		break;
	}
}

void TreePanel::OnContextMenu(wxContextMenuEvent& event)
{
	int flag;
	auto sel_item = m_datatree->HitTest(
		m_datatree->ScreenToClient(event.GetPosition()), flag);

	if (!sel_item.IsOk())
		return;

	m_datatree->SelectItem(sel_item);

	m_context_pos = event.GetPosition();
	// If from keyboard
	if (m_context_pos.x == -1 && m_context_pos.y == -1) {
		wxSize size = GetSize();
		m_context_pos.x = size.x / 2;
		m_context_pos.y = size.y / 2;
	}
	else {
		m_context_pos = ScreenToClient(m_context_pos);
	}

	auto agent = m_agent->As<TreePanelAgent>();
	if (agent)
		agent->UpdateDataToUI({ gstTreeContextMenu });
}

void TreePanel::OnToolbar(wxCommandEvent& event)
{
	fluo::ValueCollection vc;
	int id = event.GetId();

	switch (id)
	{
	case ID_ToggleDisp:
		vc.insert(gstTreeAction);
		break;
	case ID_AddVolGroup:
		vc.insert(gstAddVolumeGroup);
		break;
	case ID_AddMeshGroup:
		vc.insert(gstAddMeshGroup);
		break;
	case ID_RemoveData:
		vc.insert(gstRemoveData);
		break;
	case ID_RulerLocator:
		vc.insert(gstRulerLocator);
		break;
	case ID_RulerLine:
		vc.insert(gstRulerLine);
		break;
	case ID_RulerPolyline:
		vc.insert(gstRulerPolyline);
		break;
	case ID_RulerPencil:
		vc.insert(gstRulerPencil);
		break;
	case ID_RulerEdit:
		vc.insert(gstRulerMovePoint);
		break;
	case ID_RulerDeletePoint:
		vc.insert(gstRulerDeletePoint);
		break;
	case ID_BrushRuler:
		vc.insert(gstBrushLocator);
		break;
	case ID_BrushGrow:
		vc.insert(gstBrushGrow);
		break;
	case ID_BrushAppend:
		vc.insert(gstBrushAppend);
		break;
	case ID_MeshConvert:
		vc.insert(gstMeshConvert);
		break;
	case ID_BrushComp:
		vc.insert(gstBrushComp);
		break;
	case ID_BrushDiffuse:
		vc.insert(gstBrushDiffuse);
		break;
	case ID_BrushUnselect:
		vc.insert(gstBrushUnsel);
		break;
	case ID_BrushClear:
		vc.insert(gstBrushClear);
		break;
	case ID_BrushExtract:
		vc.insert(gstBrushExtract);
		break;
	case ID_BrushDelete:
		vc.insert(gstBrushDelete);
		break;
	case ID_MachineLearning:
		vc.insert(gstMachineLearningDlg);
		break;
	case ID_ManipulateData:
		vc.insert(gstManipPropPanel);
		break;
	}

	auto agent = m_agent->As<TreePanelAgent>();
	if (agent)
		agent->UpdateUIToData(vc);
}

void TreePanel::OnMenu(wxCommandEvent& event)
{
	fluo::ValueCollection vc;
	int id = event.GetId();

	switch (id)
	{
	case ID_ToggleDisp:
		vc.insert(gstTreeAction);
		return;
	case ID_AddVolGroup:
		vc.insert(gstAddVolumeGroup);
		return;
	case ID_AddMeshGroup:
		vc.insert(gstAddMeshGroup);
		return;
	case ID_RemoveData:
		vc.insert(gstRemoveData);
		break;
	case ID_Expand:
		vc.insert(gstTreeExpandSelItem);
		break;
	case ID_RandomizeColor:
		vc.insert(gstRandomizeColor);
		break;
	case ID_CloseView:
		vc.insert(gstCloseView);
		break;
	case ID_Isolate:
		vc.insert(gstIsolate);
		break;
	case ID_ShowAll:
		vc.insert(gstShowAll);
		break;
	case ID_CopyMask:
		vc.insert(gstCopyMask);
		break;
	case ID_PasteMask:
		vc.insert(gstPasteMask);
		break;
	case ID_MergeMask:
		vc.insert(gstMergeMask);
		break;
	case ID_ExcludeMask:
		vc.insert(gstExcludeMask);
		break;
	case ID_IntersectMask:
		vc.insert(gstIntersectMask);
		break;
	case ID_Brush:
		vc.insert(gstBrushToolDlg);
		break;
	case ID_Measurement:
		vc.insert(gstMeasureDlg);
		break;
	case ID_Component:
		vc.insert(gstComponentDlg);
		break;
	case ID_Track:
		vc.insert(gstTrackDlg);
		break;
	case ID_Calculation:
		vc.insert(gstCalculationDlg);
		break;
	case ID_NoiseReduct:
		vc.insert(gstNoiseCancellingDlg);
		break;
	case ID_VolumeSize:
		vc.insert(gstCountingDlg);
		break;
	case ID_Colocalization:
		vc.insert(gstColocalizationDlg);
		break;
	case ID_Convert:
		vc.insert(gstConvertDlg);
		break;
	case ID_Ocl:
		vc.insert(gstOclDlg);
		break;
	case ID_MachineLearning:
		vc.insert(gstMachineLearningDlg);
		break;
	case ID_ManipulateData:
		vc.insert(gstManipPropPanel);
		break;
	}

	auto agent = m_agent->As<TreePanelAgent>();
	if (agent)
		agent->UpdateUIToData(vc);
}

void TreePanel::OnSelChanged(wxTreeEvent& event)
{
	if (m_suppress_event)
		return;
	if (m_datatree && m_datatree->m_silent_select)
		return;

	auto agent = m_agent->As<TreePanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstTreeSelection });

	event.Skip();
}

void TreePanel::OnDeleting(wxTreeEvent& event)
{
	if (m_suppress_event)
		return;
	auto agent = m_agent->As<TreePanelAgent>();
	if (agent)
		agent->NotifyDataToUI({ gstCurrentSelect });
}

void TreePanel::OnAct(wxTreeEvent& event)
{
	auto agent = m_agent->As<TreePanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstTreeAction });
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
	{
		auto agent = m_agent->As<TreePanelAgent>();
		if (agent)
			agent->UpdateUIToData({ gstRemoveData });
	}
}

