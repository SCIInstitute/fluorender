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
	m_rename = m_rename_text->GetValue();
	wxWindow* par = GetParent();
	ListPanel* lp = dynamic_cast<ListPanel*>(par);
	if (lp)
		lp->RenameSelection(m_rename.ToStdWstring());
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

	if (stype == L"Volume")
	{
		glbin_current.SetVolumeData(glbin_data_manager.GetVolumeData(name));
	}
	else if (stype == L"Mesh")
	{
		glbin_current.SetMeshData(glbin_data_manager.GetMeshData(name));
	}
	else if (stype == L"AnnotData")
	{
		glbin_current.SetAnnotData(glbin_data_manager.GetAnnotData(name));
	}

	FluoRefresh(1, { gstCurrentSelect });

	event.Skip();
}

void ListPanel::OnAct(wxListEvent& event)
{
	AddSelToCurView();
}

void ListPanel::OnKeyDown(wxKeyEvent& event)
{
	if (event.GetKeyCode() == WXK_DELETE ||
		event.GetKeyCode() == WXK_BACK)
		DeleteSelection();
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

//crop
void ListPanel::OnCropCheck(wxCommandEvent& event)
{
	wxCheckBox* ch1 = (wxCheckBox*)event.GetEventObject();
	if (ch1)
		glbin_settings.m_save_crop = ch1->GetValue();
}

//compress
void ListPanel::OnCompCheck(wxCommandEvent& event)
{
	wxCheckBox* ch1 = (wxCheckBox*)event.GetEventObject();
	if (ch1)
		glbin_settings.m_save_compress = ch1->GetValue();
}

void ListPanel::OnResizeCheck(wxCommandEvent& event)
{
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;
	wxCheckBox* comp_chk = (wxCheckBox*)event.GetEventObject();
	if (!comp_chk)
		return;
	bool resize = comp_chk->GetValue();
	wxWindow* panel = comp_chk->GetParent();
	if (!panel)
		return;
	wxTextCtrl* size_x_txt = (wxTextCtrl*)panel->FindWindow(ID_RESIZE_X_TXT);
	wxTextCtrl* size_y_txt = (wxTextCtrl*)panel->FindWindow(ID_RESIZE_Y_TXT);
	wxTextCtrl* size_z_txt = (wxTextCtrl*)panel->FindWindow(ID_RESIZE_Z_TXT);
	//set size values
	if (size_x_txt && size_y_txt && size_z_txt)
	{
		if (resize)
		{
			auto res = vd->GetResolution();
			size_x_txt->ChangeValue(std::to_string(res.intx()));
			size_y_txt->ChangeValue(std::to_string(res.inty()));
			size_z_txt->ChangeValue(std::to_string(res.intz()));
		}
		else
		{
			size_x_txt->ChangeValue("");
			size_y_txt->ChangeValue("");
			size_z_txt->ChangeValue("");
		}
	}
	vd->SetResample(resize);
}

void ListPanel::OnSizeXText(wxCommandEvent& event)
{
	wxTextCtrl* size_x_txt = (wxTextCtrl*)event.GetEventObject();
	auto vd = glbin_current.vol_data.lock();
	if (size_x_txt && vd)
	{
		auto size = vd->GetResampledSize();
		size.x(STOI(size_x_txt->GetValue().ToStdString()));
		vd->SetResampledSize(size);
	}
}

void ListPanel::OnSizeYText(wxCommandEvent& event)
{
	wxTextCtrl* size_y_txt = (wxTextCtrl*)event.GetEventObject();
	auto vd = glbin_current.vol_data.lock();
	if (size_y_txt && vd)
	{
		auto size = vd->GetResampledSize();
		size.y(STOI(size_y_txt->GetValue().ToStdString()));
		vd->SetResampledSize(size);
	}
}

void ListPanel::OnSizeZText(wxCommandEvent& event)
{
	wxTextCtrl* size_z_txt = (wxTextCtrl*)event.GetEventObject();
	auto vd = glbin_current.vol_data.lock();
	if (size_z_txt && vd)
	{
		auto size = vd->GetResampledSize();
		size.z(STOI(size_z_txt->GetValue().ToStdString()));
		vd->SetResampledSize(size);
	}
}

void ListPanel::OnFilterChange(wxCommandEvent& event)
{
	wxComboBox* combo = (wxComboBox*)event.GetEventObject();
	if (combo)
		glbin_settings.m_save_filter = combo->GetSelection();
}

wxWindow* ListPanel::CreateExtraControl(wxWindow* parent)
{
	wxIntegerValidator<unsigned int> vald_int;

	wxPanel* panel = new wxPanel(parent);
#ifdef _DARWIN
	panel->SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#elifdef __linux__
	panel->SetWindowVariant(wxWINDOW_VARIANT_MINI);
#endif
	wxStaticBoxSizer* group1 = new wxStaticBoxSizer(
		wxVERTICAL, panel, "Additional Options");

	//compressed
	wxBoxSizer* sizer1 = new wxBoxSizer(wxHORIZONTAL);
	wxCheckBox* comp_chk = new wxCheckBox(panel, ID_LZW_COMP,
		"Lempel-Ziv-Welch Compression");
	comp_chk->Connect(comp_chk->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(ListPanel::OnCompCheck), NULL, panel);
	comp_chk->SetValue(glbin_settings.m_save_compress);
	sizer1->Add(10, 10);
	sizer1->Add(comp_chk);
	//crop
	wxBoxSizer* sizer2 = new wxBoxSizer(wxHORIZONTAL);
	wxCheckBox* crop_chk = new wxCheckBox(panel, ID_CROP,
		"Use Clipping Planes to Crop");
	crop_chk->Connect(crop_chk->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(ListPanel::OnCropCheck), NULL, panel);
	crop_chk->SetValue(glbin_settings.m_save_crop);
	sizer2->Add(10, 10);
	sizer2->Add(crop_chk);
	//resize
	wxBoxSizer* sizer3 = new wxBoxSizer(wxHORIZONTAL);
	wxCheckBox* resize_chk = new wxCheckBox(panel, ID_RESIZE_CHK,
		"Resize");
	resize_chk->Connect(resize_chk->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(ListPanel::OnResizeCheck), NULL, panel);
	wxTextCtrl* size_x_txt = new wxTextCtrl(panel, ID_RESIZE_X_TXT, "",
		wxDefaultPosition, parent->FromDIP(wxSize(40, 20)), wxTE_RIGHT, vald_int);
	size_x_txt->Connect(size_x_txt->GetId(), wxEVT_TEXT,
		wxCommandEventHandler(ListPanel::OnSizeXText), NULL, panel);
	wxTextCtrl* size_y_txt = new wxTextCtrl(panel, ID_RESIZE_Y_TXT, "",
		wxDefaultPosition, parent->FromDIP(wxSize(40, 20)), wxTE_RIGHT, vald_int);
	size_y_txt->Connect(size_y_txt->GetId(), wxEVT_TEXT,
		wxCommandEventHandler(ListPanel::OnSizeYText), NULL, panel);
	wxTextCtrl* size_z_txt = new wxTextCtrl(panel, ID_RESIZE_Z_TXT, "",
		wxDefaultPosition, parent->FromDIP(wxSize(40, 20)), wxTE_RIGHT, vald_int);
	size_z_txt->Connect(size_z_txt->GetId(), wxEVT_TEXT,
		wxCommandEventHandler(ListPanel::OnSizeZText), NULL, panel);
	wxComboBox* combo = new wxComboBox(panel, ID_FILTER,
		"Filter", wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY);
	combo->Connect(combo->GetId(), wxEVT_COMMAND_COMBOBOX_SELECTED,
		wxCommandEventHandler(ListPanel::OnFilterChange), NULL, panel);
	std::vector<std::string> combo_list;
	combo_list.push_back("Nearest neighbor");
	combo_list.push_back("Bilinear");
	combo_list.push_back("Trilinear");
	combo_list.push_back("Box");
	for (size_t i = 0; i < combo_list.size(); ++i)
		combo->Append(combo_list[i]);
	combo->SetSelection(glbin_settings.m_save_filter);

	if (auto vd = glbin_current.vol_data.lock())
	{
		bool resize = vd->GetResample();
		auto size = vd->GetResampledSize();
		resize_chk->SetValue(resize);
		if (resize)
		{
			size_x_txt->ChangeValue(std::to_string(size.intx()));
			size_y_txt->ChangeValue(std::to_string(size.inty()));
			size_z_txt->ChangeValue(std::to_string(size.intz()));
		}
	}
	sizer3->Add(10, 10);
	sizer3->Add(resize_chk, 0, wxALIGN_CENTER);
	sizer3->Add(10, 10);
	sizer3->Add(size_x_txt, 0, wxALIGN_CENTER);
	sizer3->Add(10, 10);
	sizer3->Add(size_y_txt, 0, wxALIGN_CENTER);
	sizer3->Add(10, 10);
	sizer3->Add(size_z_txt, 0, wxALIGN_CENTER);
	sizer3->Add(10, 10);
	sizer3->Add(combo, 0, wxALIGN_CENTER);

	//group
	group1->Add(10, 10);
	group1->Add(sizer1);
	group1->Add(10, 10);
	group1->Add(sizer2);
	group1->Add(10, 10);
	group1->Add(sizer3);
	group1->Add(10, 20);

	panel->SetSizerAndFit(group1);
	panel->Layout();

	return panel;
}

