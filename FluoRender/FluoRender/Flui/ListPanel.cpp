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
#include <ListPanel.h>
#include <ListPanelAgent.h>
#include <png_resource.h>
#include <wx/valnum.h>
//resources
#include <icons.h>

DataListCtrl::DataListCtrl(
	wxWindow* parent,
	const wxPoint& pos,
	const wxSize& size,
	long style) :
	wxListCtrl(parent, wxID_ANY, pos, size, style),
	m_selected(-1)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);
	SetDoubleBuffered(true);

	wxListItem itemCol;
	itemCol.SetText("Type");
	this->InsertColumn(0, itemCol);

	itemCol.SetText("Name");
	this->InsertColumn(1, itemCol);

	itemCol.SetText("Path");
	this->InsertColumn(2, itemCol);

	m_rename_text = new wxTextCtrl(this, wxID_ANY, "",
		wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	m_rename_text->Bind(wxEVT_LEFT_DCLICK, &DataListCtrl::OnTextFocus, this);
	m_rename_text->Bind(wxEVT_TEXT, &DataListCtrl::OnNameText, this);
	m_rename_text->Bind(wxEVT_TEXT_ENTER, &DataListCtrl::OnNameEnter, this);
	m_rename_text->Bind(wxEVT_KILL_FOCUS, &DataListCtrl::OnKillFocus, this);
	m_rename_text->Hide();

	Bind(wxEVT_LIST_ITEM_SELECTED, &DataListCtrl::OnSelectionChanged, this);
}

void DataListCtrl::SelectItemSilently(ListItemType type, const wxString& name)
{
	wxString type_str;
	if (type == ListItemType::Volume)
		type_str = "Volume";
	else if (type == ListItemType::Mesh)
		type_str = "Mesh";
	else if (type == ListItemType::Annot)
		type_str = "AnnotData";

	for (int i = 0; i < GetItemCount(); ++i)
	{
		auto stype = GetText(i, 0);
		auto sname = GetText(i, 1);

		if (stype == type_str &&
			sname == name)
		{
			SelectItemSilently(i);
			break;
		}
	}
}

void DataListCtrl::Append(ListItemType type, const wxString& name, const wxString& path)
{
	long tmp = 0;
	if (type == ListItemType::Volume)
		tmp = InsertItem(GetItemCount(), "Volume");
	else if (type == ListItemType::Mesh)
		tmp = InsertItem(GetItemCount(), "Mesh");
	else if (type == ListItemType::Annot)
		tmp = InsertItem(GetItemCount(), "AnnotData");

	SetItem(tmp, 1, name);
	SetItem(tmp, 2, path);
	SetColumnWidth(0, wxLIST_AUTOSIZE);
	SetColumnWidth(1, wxLIST_AUTOSIZE);
	SetColumnWidth(2, wxLIST_AUTOSIZE);
}

wxString DataListCtrl::GetText(long item, int col)
{
	wxListItem info;
	info.SetId(item);
	info.SetColumn(col);
	info.SetMask(wxLIST_MASK_TEXT);
	GetItem(info);
	return info.GetText();
}

void DataListCtrl::SetText(long item, int col, const wxString &str)
{
	wxListItem info;
	info.SetId(item);
	info.SetColumn(col);
	info.SetMask(wxLIST_MASK_TEXT);
	GetItem(info);
	info.SetText(str);
	SetItem(info);
}

void DataListCtrl::StartEdit()
{
	long item = GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	if (item != -1)
	{
		wxRect rect;
		GetSubItemRect(item, 1, rect);
		wxString str = GetText(item, 1);
		m_rename_text->SetPosition(rect.GetTopLeft());
		m_rename_text->SetSize(rect.GetSize());
		m_rename_text->ChangeValue(str);
		m_rename_text->SetFocus();
		m_rename_text->SetSelection(-1, -1);
		m_rename_text->Show();
	}
}

void DataListCtrl::EndEdit()
{
	if (m_rename_text->IsShown())
	{
		m_rename_text->Hide();
	}
}

void DataListCtrl::OnTextFocus(wxMouseEvent& event)
{
	wxTextCtrl* object = dynamic_cast<wxTextCtrl*>(event.GetEventObject());
	if (object)
		object->SetSelection(0, -1);
}

void DataListCtrl::OnNameText(wxCommandEvent& event)
{
	auto name = m_rename_text->GetValue();
	wxWindow* par = GetParent();
	ListPanel* lp = dynamic_cast<ListPanel*>(par);
	if (lp)
		lp->RenameSelection(name);
}

void DataListCtrl::OnNameEnter(wxCommandEvent& event)
{
	EndEdit();
}

void DataListCtrl::OnSelectionChanged(wxListEvent& event)
{
	if (m_silent_select)
		return;

	wxFont font;
	if (m_selected != -1)
	{
		font = GetItemFont(m_selected);
		font.SetWeight(wxFONTWEIGHT_NORMAL);
		SetItemFont(m_selected, font);
	}
	m_selected = GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	if (m_selected != -1)
	{
		font = GetItemFont(m_selected);
		font.SetWeight(wxFONTWEIGHT_BOLD);
		SetItemFont(m_selected, font);
	}
	event.Skip();
}

void DataListCtrl::OnKillFocus(wxFocusEvent& event)
{
	EndEdit();
	event.Skip();
}

SaveVolumeHook::SaveVolumeHook(
	const SaveVolumeOptions& options) :
	m_options(options)
{

}

void SaveVolumeHook::AddCustomControls(
	wxFileDialogCustomize& customizer)
{
	customizer.AddStaticText("Additional Options");

	m_comp_chk =
		customizer.AddCheckBox(
			"Lempel-Ziv-Welch Compression");

	m_comp_chk->SetValue(
		m_options.compress);

	m_crop_chk =
		customizer.AddCheckBox(
			"Use Clipping Planes to Crop");

	m_crop_chk->SetValue(
		m_options.crop);

	m_resize_chk =
		customizer.AddCheckBox(
			"Resize");

	m_resize_chk->SetValue(
		m_options.resize);

	customizer.AddStaticText(
		"Size X");

	m_size_x_txt =
		customizer.AddTextCtrl(
			std::to_string(
				m_options.size_x));

	customizer.AddStaticText(
		"Size Y");

	m_size_y_txt =
		customizer.AddTextCtrl(
			std::to_string(
				m_options.size_y));

	customizer.AddStaticText(
		"Size Z");

	m_size_z_txt =
		customizer.AddTextCtrl(
			std::to_string(
				m_options.size_z));

	static const wxString kFilterChoices[] =
	{
		"Nearest neighbor",
		"Bilinear",
		"Trilinear",
		"Box"
	};

	m_filter_choice =
		customizer.AddChoice(
			WXSIZEOF(kFilterChoices),
			kFilterChoices);

	m_filter_choice->SetSelection(
		m_options.filter);
}

void SaveVolumeHook::TransferDataFromCustomControls()
{
	m_options.compress =
		m_comp_chk->GetValue();

	m_options.crop =
		m_crop_chk->GetValue();

	m_options.resize =
		m_resize_chk->GetValue();

	m_options.size_x =
		wxAtoi(m_size_x_txt->GetValue());

	m_options.size_y =
		wxAtoi(m_size_y_txt->GetValue());

	m_options.size_z =
		wxAtoi(m_size_z_txt->GetValue());

	m_options.filter =
		m_filter_choice->GetSelection();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
ListPanel::ListPanel(
	wxWindow* parent,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name) :
	PropPanel(parent, pos, size, style, name)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);

	//create data list
	m_datalist = new DataListCtrl(this);

	//create tool bar
	m_toolbar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxTB_FLAT | wxTB_TOP | wxTB_NODIVIDER);
	wxBitmapBundle bitmap = wxGetBitmap(view);
	m_toolbar->AddTool(ID_AddToView, "Add to View",
		bitmap, "Add selected data set to render view");
	m_toolbar->SetToolLongHelp(ID_AddToView,
		"Add selected data set to render view");
	bitmap = wxGetBitmap(rename);
	m_toolbar->AddTool(ID_Rename, "Rename",
		bitmap, "Rename selected data set");
	m_toolbar->SetToolLongHelp(ID_Rename,
		"Rename selected data set");
	bitmap = wxGetBitmap(save);
	m_toolbar->AddTool(ID_Save, "Save As",
		bitmap, "Save selected volume data set");
	m_toolbar->SetToolLongHelp(ID_Save,
		"Save selected volume data set");
	bitmap = wxGetBitmap(bake);
	m_toolbar->AddTool(ID_Bake, "Bake",
		bitmap, "Apply the volume properties and save");
	m_toolbar->SetToolLongHelp(ID_Bake,
		"Apply the volume properties and save");
	bitmap = wxGetBitmap(save_mask);
	m_toolbar->AddTool(ID_SaveMask, "Save Mask",
		bitmap, "Save its mask to a file");
	m_toolbar->SetToolLongHelp(ID_SaveMask,
		"Save its mask to a file");
	bitmap = wxGetBitmap(delet);
	m_toolbar->AddTool(ID_Delete, "Delete",
		bitmap, "Delete selected data set");
	m_toolbar->SetToolLongHelp(ID_Delete,
		"Delete selected data set");
	bitmap = wxGetBitmap(del_all);
	m_toolbar->AddTool(ID_DeleteAll, "Delete All",
		bitmap, "Delete all data sets");
	m_toolbar->SetToolLongHelp(ID_DeleteAll,
		"Delete all data sets");
	m_toolbar->Bind(wxEVT_TOOL, &ListPanel::OnToolbar, this);
	m_toolbar->Realize();

	//organize positions
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);

	sizer_v->Add(m_toolbar, 0, wxEXPAND | wxLEFT, 20);
	sizer_v->Add(m_datalist, 1, wxEXPAND);

	SetSizer(sizer_v);
	Layout();

	//events
	m_datalist->Bind(wxEVT_LIST_ITEM_DESELECTED, &ListPanel::OnEndEditName, this);
	m_datalist->Bind(wxEVT_KILL_FOCUS, &ListPanel::OnKillFocus, this);
	Bind(wxEVT_CONTEXT_MENU, &ListPanel::OnContextMenu, this);
	Bind(wxEVT_MENU, &ListPanel::OnMenu, this);
	Bind(wxEVT_LIST_ITEM_SELECTED, &ListPanel::OnSelect, this);
	Bind(wxEVT_LIST_ITEM_ACTIVATED, &ListPanel::OnAct, this);
	Bind(wxEVT_KEY_DOWN, &ListPanel::OnKeyDown, this);
	Bind(wxEVT_KEY_UP, &ListPanel::OnKeyUp, this);
	Bind(wxEVT_SCROLLWIN_THUMBTRACK, &ListPanel::OnScrollWin, this);
	Bind(wxEVT_LEFT_DOWN, &ListPanel::OnMouse, this);
	Bind(wxEVT_MOUSEWHEEL, &ListPanel::OnScroll, this);
}

ListPanel::~ListPanel()
{
}

void ListPanel::DeleteAllListItems()
{
	m_datalist->DeleteAllItems();
}

void ListPanel::AppendListItem(ListItemType type, const std::wstring& name, const std::wstring& path)
{
	m_datalist->Append(type, name, path);
}

void ListPanel::RenameSelection(const wxString& name)
{
	auto agent = m_agent->As<ListPanelAgent>();
	if (agent)
		agent->SetSelName(name.ToStdWstring());
}

void ListPanel::SelectListItem(ListItemType type, const std::wstring& name)
{
	m_datalist->SelectItemSilently(type, name);
}

void ListPanel::OnContextMenu(wxContextMenuEvent& event)
{
	auto agent = m_agent->As<ListPanelAgent>();
	if (!agent)
		return;

	int seln = m_datalist->GetSelectedItemCount();
	if (seln == 0)
		return;

	wxPoint point = event.GetPosition();
	// If from keyboard
	if (point.x == -1 && point.y == -1)
	{
		wxSize size = GetSize();
		point.x = size.x / 2;
		point.y = size.y / 2;
	}
	else
	{
		point = ScreenToClient(point);
	}

	wxMenu menu;
	wxMenu* add_to_menu = new wxMenu;
	auto view_list = agent->GetViewNames();
	for (size_t i = 0; i < view_list.size(); ++i)
	{
		add_to_menu->Append(ID_ViewID + i, view_list[i]);
	}

	menu.Append(ID_AddToView, "Add to", add_to_menu);
	if (seln > 1)
	{
		PopupMenu(&menu, point.x, point.y);
		return;
	}

	menu.Append(ID_Delete, "Delete");
	menu.Append(ID_Rename, "Rename");
	//save/save as
	auto info = agent->GetListContextInfo();
	if (info.path_valid)
		menu.Append(ID_Save, "Save As...");
	else
		menu.Append(ID_Save, "Save...");
	if (info.type == ListItemType::Volume)
	{
		menu.Append(ID_Bake, "Bake...");
		menu.Append(ID_SaveMask, "Save Mask");
	}

	PopupMenu(&menu, point.x, point.y);
}

void ListPanel::OnToolbar(wxCommandEvent& event)
{
	auto agent = m_agent->As<ListPanelAgent>();
	if (!agent)
		return;

	fluo::ValueCollection vc;
	int id = event.GetId();

	switch (id)
	{
	case ID_AddToView:
		vc.insert(gstAddListSelToView);
		break;
	case ID_Rename:
		m_datalist->StartEdit();
		break;
	case ID_Save:
		vc.insert(gstListSaveSelection);
		break;
	case ID_Bake:
		vc.insert(gstListBakeSelection);
		break;
	case ID_SaveMask:
		vc.insert(gstListSaveSelMask);
		break;
	case ID_Delete:
		vc.insert(gstListDeleteSelection);
		break;
	case ID_DeleteAll:
		vc.insert(gstListDeleteAll);
		break;
	}

	agent->UpdateUIToData(vc);
}

void ListPanel::OnMenu(wxCommandEvent& event)
{
	auto agent = m_agent->As<ListPanelAgent>();
	if (!agent)
		return;

	fluo::ValueCollection vc;
	int id = event.GetId();

	switch (id)
	{
	case ID_AddToView:
		vc.insert(gstAddListSelToView);
		break;
	case ID_Rename:
		m_datalist->StartEdit();
		break;
	case ID_Save:
		vc.insert(gstListSaveSelection);
		break;
	case ID_Bake:
		vc.insert(gstListBakeSelection);
		break;
	case ID_SaveMask:
		vc.insert(gstListSaveSelMask);
		break;
	case ID_Delete:
		vc.insert(gstListDeleteSelection);
		break;
	case ID_DeleteAll:
		vc.insert(gstListDeleteAll);
		break;
	}

	agent->UpdateUIToData(vc);
}

void ListPanel::OnSelect(wxListEvent& event)
{
	long item = m_datalist->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);

	if (item == -1)
		return;

	std::wstring stype = m_datalist->GetText(item, 0).ToStdWstring();
	std::wstring name = m_datalist->GetText(item, 1).ToStdWstring();
	ListItemType type = ListItemType::Invalid;

	if (stype == L"Volume")
	{
		type = ListItemType::Volume;
	}
	else if (stype == L"Mesh")
	{
		type = ListItemType::Mesh;
	}
	else if (stype == L"AnnotData")
	{
		type = ListItemType::Annot;
	}

	auto agent = m_agent->As<ListPanelAgent>();
	if (agent)
		agent->SetCurrentSelection(type, name);

	event.Skip();
}

void ListPanel::OnAct(wxListEvent& event)
{
	auto agent = m_agent->As<ListPanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstAddListSelToView });
}

void ListPanel::OnKeyDown(wxKeyEvent& event)
{
	if (event.GetKeyCode() == WXK_DELETE ||
		event.GetKeyCode() == WXK_BACK)
	{
		auto agent = m_agent->As<ListPanelAgent>();
		if (agent)
			agent->UpdateUIToData({ gstListDeleteSelection });
	}
}

void ListPanel::OnKeyUp(wxKeyEvent& event)
{
}

void ListPanel::OnMouse(wxMouseEvent& event)
{
	if (event.Button(wxMOUSE_BTN_ANY))
		m_datalist->EndEdit();
}

void ListPanel::OnEndEditName(wxListEvent& event)
{
	m_datalist->EndEdit();
}

void ListPanel::OnScrollWin(wxScrollWinEvent& event)
{
	m_datalist->EndEdit();
	event.Skip();
}

void ListPanel::OnScroll(wxMouseEvent& event)
{
	m_datalist->EndEdit();
	event.Skip();
}

void ListPanel::OnKillFocus(wxFocusEvent& event)
{
	m_datalist->EndEdit();
	event.Skip();
}

