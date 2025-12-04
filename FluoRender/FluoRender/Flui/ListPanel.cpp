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
#include <ListPanel.h>
#include <Global.h>
#include <Names.h>
#include <MainSettings.h>
#include <MainFrame.h>
#include <ModalDlg.h>
#include <RenderView.h>
#include <VolumeData.h>
#include <MeshData.h>
#include <AnnotData.h>
#include <Root.h>
#include <CurrentObjects.h>
#include <DataManager.h>
#include <VolumeRenderer.h>
#include <png_resource.h>
#include <compatibility.h>
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
	m_rename_text->Hide();

	Bind(wxEVT_LIST_ITEM_SELECTED, &DataListCtrl::OnSelectionChanged, this);
}

void DataListCtrl::Append(int type, const wxString& name, const wxString& path)
{
	long tmp = 0;
	if (type == DATA_VOLUME)
		tmp = InsertItem(GetItemCount(), "Volume");
	else if (type == DATA_MESH)
		tmp = InsertItem(GetItemCount(), "Mesh");
	else if (type == DATA_ANNOT)
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

wxString DataListCtrl::EndEdit()
{
	wxString str;
	if (!m_rename_text->IsShown())
		return str;

	str = m_rename_text->GetValue();
	m_rename_text->Hide();
	return str;
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////
ListPanel::ListPanel(MainFrame* frame,
	wxWindow* parent,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name) :
	PropPanel(frame, parent, pos, size, style, name)
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

void ListPanel::FluoUpdate(const fluo::ValueCollection& vc)
{
	if (FOUND_VALUE(gstNull))
		return;

	bool update_all = vc.empty();

	if (update_all || FOUND_VALUE(gstListCtrl))
		UpdateList();

	if (update_all || FOUND_VALUE(gstCurrentSelect))
		UpdateSelection();
}

void ListPanel::UpdateList()
{
	m_suppress_event = true;

	m_datalist->DeleteAllItems();

	for (int i = 0; i < glbin_data_manager.GetVolumeNum(); i++)
	{
		auto vd = glbin_data_manager.GetVolumeData(i);
		if (vd)
		{
			std::wstring name = vd->GetName();
			std::wstring path = vd->GetPath();
			m_datalist->Append(DATA_VOLUME, name, path);
		}
	}

	for (int i = 0; i < glbin_data_manager.GetMeshNum(); i++)
	{
		auto md = glbin_data_manager.GetMeshData(i);
		if (md)
		{
			std::wstring name = md->GetName();
			std::wstring path = md->GetPath();
			m_datalist->Append(DATA_MESH, name, path);
		}
	}

	for (int i = 0; i < glbin_data_manager.GetAnnotNum(); i++)
	{
		auto ann = glbin_data_manager.GetAnnotData(i);
		if (ann)
		{
			std::wstring name = ann->GetName();
			std::wstring path = ann->GetPath();
			m_datalist->Append(DATA_ANNOT, name, path);
		}
	}

	m_suppress_event = false;
}

void ListPanel::UpdateSelection()
{
	int type = glbin_current.GetType();
	std::wstring name, item_type;
	switch (type)
	{
	case 2://volume
	{
		auto vd = glbin_current.vol_data.lock();
		if (vd)
			name = vd->GetName();
		item_type = L"Volume";
	}
	break;
	case 3://mesh
	{
		auto md = glbin_current.mesh_data.lock();
		if (md)
			name = md->GetName();
		item_type = L"Mesh";
	}
	break;
	case 4://annotations
	{
		auto ann = glbin_current.ann_data.lock();
		if (ann)
			name = ann->GetName();
		item_type = L"AnnotData";
	}
	break;
	}

	for (int i = 0; i < m_datalist->GetItemCount(); ++i)
	{
		std::wstring stype = m_datalist->GetText(i, 0).ToStdWstring();
		std::wstring sname = m_datalist->GetText(i, 1).ToStdWstring();

		if (stype == item_type &&
			sname == name)
		{
			m_datalist->SelectItemSilently(i);
			break;
		}
	}
}

void ListPanel::AddSelectionToView(int vid)
{
	Root* root = glbin_data_manager.GetRoot();
	if (!root)
		return;
	auto view = root->GetView(vid);
	if (!view)
		return;

	fluo::ValueCollection vc;
	bool view_empty = true;
	int type = glbin_current.GetType();

	switch (type)
	{
	case 2://volume
	{
		auto vd = glbin_current.vol_data.lock();
		if (!vd)
			break;

		std::wstring name = vd->GetName();
		auto vd_add = vd;

		for (int i = 0; i < root->GetViewNum(); ++i)
		{
			auto v = root->GetView(i);
			if (v && v->GetVolumeData(name))
			{
				vd_add = glbin_data_manager.DuplicateVolumeData(vd);
				break;
			}
		}

		int chan_num = view->GetAny();
		view_empty = chan_num > 0 ? false : view_empty;
		fluo::Color color(1.0, 1.0, 1.0);
		if (chan_num == 0)
			color = fluo::Color(1.0, 0.0, 0.0);
		else if (chan_num == 1)
			color = fluo::Color(0.0, 1.0, 0.0);
		else if (chan_num == 2)
			color = fluo::Color(0.0, 0.0, 1.0);

		if (chan_num >= 0 && chan_num < 3)
			vd_add->SetColor(color);

		auto group = view->AddVolumeData(vd_add);
		glbin_current.SetVolumeData(vd_add);
		if (view->GetChannelMixMode() == ChannelMixMode::Depth)
			vc.insert(gstUpdateSync);
		vc.insert(gstVolumePropPanel);
	}
		break;
	case 3://mesh
	{
		auto md = glbin_current.mesh_data.lock();
		if (!md)
			break;
		int chan_num = view->GetAny();
		view_empty = chan_num > 0 ? false : view_empty;
		view->AddMeshData(md);
		vc.insert(gstMeshPropPanel);
	}
		break;
	case 4://annotations
	{
		auto ann = glbin_current.ann_data.lock();
		if (!ann)
			break;
		int chan_num = view->GetAny();
		view_empty = chan_num > 0 ? false : view_empty;
		view->AddAnnotData(ann);
		vc.insert(gstAnnotatPropPanel);
	}
		break;
	}

	//update
	if (vid)
	{
		if (view_empty)
			view->InitView(INIT_BOUNDS | INIT_CENTER | INIT_TRANSL | INIT_ROTATE);
		else
			view->InitView(INIT_BOUNDS | INIT_CENTER);
	}
	vc.insert({ gstListCtrl, gstTreeCtrl, gstCurrentSelect });
	FluoRefresh(0, vc, { vid });
}

void ListPanel::AddSelToCurView()
{
	AddSelectionToView(glbin_current.GetViewId());
}

void ListPanel::RenameSelection(const std::wstring& name)
{
	std::wstring new_name = name;
	for (int i = 1; glbin_data_manager.CheckNames(new_name); i++)
		new_name = new_name + L"_" + std::to_wstring(i);
	int type = glbin_current.GetType();

	switch (type)
	{
	case 2://volume
	{
		auto vd = glbin_current.vol_data.lock();
		if (vd)
			vd->SetName(new_name);
	}
		break;
	case 3://mesh
	{
		auto md = glbin_current.mesh_data.lock();
		if (md)
			md->SetName(new_name);
	}
		break;
	case 4://annotations
	{
		auto ann = glbin_current.ann_data.lock();
		if (ann)
			ann->SetName(new_name);
	}
		break;
	}
	FluoRefresh(2, { gstTreeCtrl });
}

void ListPanel::SaveSelection()
{
	int type = glbin_current.GetType();
	long item = m_datalist->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);

	switch (type)
	{
	case 2://volume
	{
		auto vd = glbin_current.vol_data.lock();
		if (!vd)
			break;
		fluo::Quaternion q = vd->GetClippingBox().GetRotation();
		vd->SetResample(false);

		ModalDlg fopendlg(
			m_frame, "Save Volume Data", "", "",
			"Muti-page Tiff file (*.tif, *.tiff)|*.tif;*.tiff|"\
			"Single-page Tiff sequence (*.tif)|*.tif;*.tiff|"\
			"Utah Nrrd file (*.nrrd)|*.nrrd",
			wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
		fopendlg.SetExtraControlCreator(CreateExtraControl);

		int rval = fopendlg.ShowModal();

		if (rval == wxID_OK)
		{
			std::wstring filename = fopendlg.GetPath().ToStdWstring();
			vd->Save(filename, fopendlg.GetFilterIndex(), 3, false,
				glbin_settings.m_save_crop, glbin_settings.m_save_filter,
				false, glbin_settings.m_save_compress,
				fluo::Point(), q, fluo::Vector(), false);
			std::wstring str = vd->GetPath();
			m_datalist->SetText(item, 2, str);
		}
	}
	break;
	case 3://mesh
	{
		auto md = glbin_current.mesh_data.lock();
		if (!md)
			break;
		ModalDlg fopendlg(
			m_frame, "Save Mesh Data", "", "",
			"OBJ file (*.obj)|*.obj",
			wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

		int rval = fopendlg.ShowModal();

		if (rval == wxID_OK)
		{
			std::wstring filename = fopendlg.GetPath().ToStdWstring();

			md->Save(filename);
			std::wstring str = md->GetPath();
			m_datalist->SetText(item, 2, str);
		}
	}
	break;
	case 4://annotations
	{
		auto ann = glbin_current.ann_data.lock();
		if (!ann)
			break;
		ModalDlg fopendlg(
			m_frame, "Save AnnotData", "", "",
			"Text file (*.txt)|*.txt",
			wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

		int rval = fopendlg.ShowModal();

		if (rval == wxID_OK)
		{
			std::wstring filename = fopendlg.GetPath().ToStdWstring();

			ann->Save(filename);
			std::wstring str = ann->GetPath();
			m_datalist->SetText(item, 2, str);
		}
	}
	break;
	}
}

void ListPanel::BakeSelection()
{
	int type = glbin_current.GetType();
	long item = m_datalist->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);

	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;

	ModalDlg fopendlg(
		m_frame, "Bake Volume Data", "", "",
		"Muti-page Tiff file (*.tif, *.tiff)|*.tif;*.tiff|"\
		"Single-page Tiff sequence (*.tif)|*.tif;*.tiff|"\
		"Utah Nrrd file (*.nrrd)|*.nrrd",
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	fopendlg.SetExtraControlCreator(CreateExtraControl);

	int rval = fopendlg.ShowModal();

	if (rval == wxID_OK)
	{
		std::wstring filename = fopendlg.GetPath().ToStdWstring();

		fluo::Quaternion q = vd->GetClippingBox().GetRotation();
		vd->Save(filename, fopendlg.GetFilterIndex(), 3, false,
			glbin_settings.m_save_crop, glbin_settings.m_save_filter,
			true, glbin_settings.m_save_compress,
			fluo::Point(), q, fluo::Vector(), false);
		std::wstring str = vd->GetPath();
		m_datalist->SetText(item, 2, str);
	}
}

void ListPanel::SaveSelMask()
{
	auto vd = glbin_current.vol_data.lock();
	if (vd)
	{
		vd->SaveMask(true, vd->GetCurTime(), vd->GetCurChannel());
		vd->SaveLabel(true, vd->GetCurTime(), vd->GetCurChannel());
	}
}

void ListPanel::DeleteSelection()
{
	Root* root = glbin_data_manager.GetRoot();
	if (!root)
		return;
	int type = glbin_current.GetType();

	switch (type)
	{
	case 2://volume
	{
		auto vd = glbin_current.vol_data.lock();
		if (!vd)
			break;
		std::wstring name = vd->GetName();
		//from view
		for (int i = 0; i < root->GetViewNum(); i++)
		{
			auto view = root->GetView(i);
			if (view)
			{
				view->RemoveVolumeData(name);
			}
		}
		//from datamanager
		int index = glbin_data_manager.GetVolumeIndex(name);
		if (index != -1)
		{
			glbin_data_manager.RemoveVolumeData(index);
		}
	}
	break;
	case 3://mesh
	{
		auto md = glbin_current.mesh_data.lock();
		if (!md)
			break;
		std::wstring name = md->GetName();
		//from view
		for (int i = 0; i < root->GetViewNum(); i++)
		{
			auto view = root->GetView(i);
			if (view)
			{
				view->RemoveMeshData(name);
			}
		}
		//from datamanager
		int index = glbin_data_manager.GetMeshIndex(name);
		if (index != -1)
		{
			glbin_data_manager.RemoveMeshData(index);
		}
	}
	break;
	case 4://annotations
	{
		auto ann = glbin_current.ann_data.lock();
		if (!ann)
			break;
		std::wstring name = ann->GetName();
		//from view
		for (int i = 0; i < root->GetViewNum(); i++)
		{
			auto view = root->GetView(i);
			if (view)
				view->RemoveAnnotData(name);
		}
		//from datamanager
		int index = glbin_data_manager.GetAnnotIndex(name);
		if (index != -1)
			glbin_data_manager.RemoveAnnotData(index);
	}
	break;
	}

	glbin_current.SetRoot();
	FluoRefresh(0, { gstTreeCtrl, gstListCtrl });
}

void ListPanel::DeleteAll()
{
	Root* root = glbin_data_manager.GetRoot();
	if (!root)
		return;
	for (int i = 0; i < root->GetViewNum(); ++i)
	{
		auto view = root->GetView(i);
		if (view)
			view->ClearAll();
	}
	glbin_data_manager.ClearAll();
	glbin_current.SetRoot();
	FluoRefresh(0, { gstTreeCtrl, gstListCtrl });
}

void ListPanel::OnContextMenu(wxContextMenuEvent& event)
{
	Root* root = glbin_data_manager.GetRoot();
	if (!root)
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
	for (int i = 0; i < root->GetViewNum(); ++i)
	{
		auto view = root->GetView(i);
		add_to_menu->Append(ID_ViewID + i,
			view->GetName());
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
	switch (glbin_current.GetType())
	{
	case 2://volume
	{
		auto vd = glbin_current.vol_data.lock();
		if (vd)
		{
			if (vd->GetPath() == L"")
				menu.Append(ID_Save, "Save...");
			else
				menu.Append(ID_Save, "Save As...");
			menu.Append(ID_Bake, "Bake...");
			menu.Append(ID_SaveMask, "Save Mask");
		}
	}
		break;
	case 3://mesh
	{
		auto md = glbin_current.mesh_data.lock();
		if (md)
		{
			if (md->GetPath() == L"")
				menu.Append(ID_Save, "Save...");
			else
				menu.Append(ID_Save, "Save As...");
		}
	}
		break;
	case 4://annotations
	{
		auto ann = glbin_current.ann_data.lock();
		if (ann)
		{
			if (ann->GetPath() == L"")
				menu.Append(ID_Save, "Save...");
			else
				menu.Append(ID_Save, "Save As...");
		}
	}
		break;
	}

	PopupMenu(&menu, point.x, point.y);
}

void ListPanel::OnToolbar(wxCommandEvent& event)
{
	int id = event.GetId();

	switch (id)
	{
	case ID_AddToView:
		AddSelToCurView();
		break;
	case ID_Rename:
		m_datalist->StartEdit();
		break;
	case ID_Save:
		SaveSelection();
		break;
	case ID_Bake:
		BakeSelection();
		break;
	case ID_SaveMask:
		SaveSelMask();
		break;
	case ID_Delete:
		DeleteSelection();
		break;
	case ID_DeleteAll:
		DeleteAll();
		break;
	}
}

void ListPanel::OnMenu(wxCommandEvent& event)
{
	int id = event.GetId();

	if (id < ID_ViewID)
	{
		switch (id)
		{
		case ID_AddToView:
			AddSelToCurView();
			break;
		case ID_Rename:
			m_datalist->StartEdit();
			break;
		case ID_Save:
			SaveSelection();
			break;
		case ID_Bake:
			BakeSelection();
			break;
		case ID_SaveMask:
			SaveSelMask();
			break;
		case ID_Delete:
			DeleteSelection();
			break;
		case ID_DeleteAll:
			DeleteAll();
			break;
		}
	}
	else
	{
		int ival = id - ID_ViewID;
		AddSelectionToView(ival);
	}
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

void ListPanel::OnEndEditName(wxCommandEvent& event)
{
	std::wstring str = m_datalist->EndEdit().ToStdWstring();
	RenameSelection(str);
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

