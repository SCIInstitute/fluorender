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
#include <MoviePanel.h>
#include <MoviePanelAgent.h>
#include <wxUndoableScrollBar.h>
#include <wxUndoableToolbar.h>
#include <wx/aboutdlg.h>
#include <wx/valnum.h>
#include <png_resource.h>
#include <icons.h>
#include <key.xpm>
#include <Debug.h>

KeyListCtrl::KeyListCtrl(
	wxWindow* parent,
	const wxPoint& pos,
	const wxSize& size,
	long style) :
	wxListCtrl(parent, wxID_ANY, pos, size, style),
	m_editing_item(-1),
	m_dragging_to_item(-1)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);
	//SetDoubleBuffered(true);

	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;

	wxListItem itemCol;
	itemCol.SetText("ID");
	itemCol.SetAlign(wxLIST_FORMAT_CENTER);
	InsertColumn(0, itemCol);
	itemCol.SetText("Frame");
	itemCol.SetAlign(wxLIST_FORMAT_RIGHT);
	InsertColumn(1, itemCol);
	itemCol.SetText("Duration");
	itemCol.SetAlign(wxLIST_FORMAT_RIGHT);
	InsertColumn(2, itemCol);
	itemCol.SetText("Interpolation");
	itemCol.SetAlign(wxLIST_FORMAT_LEFT);
	InsertColumn(3, itemCol);
	itemCol.SetText("Description");
	itemCol.SetAlign(wxLIST_FORMAT_LEFT);
	InsertColumn(4, itemCol);

	m_images = new wxImageList(16, 16, true);
	wxIcon icon = wxIcon(key_xpm);
	m_images->Add(icon);
	AssignImageList(m_images, wxIMAGE_LIST_SMALL);

	//frame edit
	m_frame_text = new wxTextCtrl(this, wxID_ANY, "",
		wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, vald_int);
	m_frame_text->Hide();
	m_frame_text->Bind(wxEVT_TEXT, &KeyListCtrl::OnFrameText, this);
	//duration edit
	m_duration_text = new wxTextCtrl(this, wxID_ANY, "",
		wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, vald_int);
	m_duration_text->Hide();
	m_duration_text->Bind(wxEVT_TEXT, &KeyListCtrl::OnDurationText, this);
	//interpolation combo box
	m_interpolation_cmb = new wxComboBox(this, wxID_ANY, "",
		wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY);
	std::vector<wxString> list = { "Linear", "Smooth" };
	m_interpolation_cmb->Append(list);
	m_interpolation_cmb->Hide();
	m_interpolation_cmb->Bind(wxEVT_COMBOBOX, &KeyListCtrl::OnInterpoCmb, this);
	//description edit
	m_description_text = new wxTextCtrl(this, wxID_ANY, "",
		wxDefaultPosition, wxDefaultSize);
	m_description_text->Hide();
	m_description_text->Bind(wxEVT_TEXT, &KeyListCtrl::OnDescritionText, this);

	//event handling
	Bind(wxEVT_LIST_ITEM_SELECTED, &KeyListCtrl::OnSelection, this);
	Bind(wxEVT_LIST_ITEM_DESELECTED, &KeyListCtrl::OnEndSelection, this);
	Bind(wxEVT_KEY_DOWN, &KeyListCtrl::OnKeyDown, this);
	Bind(wxEVT_KEY_UP, &KeyListCtrl::OnKeyUp, this);
	Bind(wxEVT_LIST_BEGIN_DRAG, &KeyListCtrl::OnBeginDrag, this);
	Bind(wxEVT_SCROLLWIN_THUMBTRACK, &KeyListCtrl::OnScroll, this);
	Bind(wxEVT_MOUSEWHEEL, &KeyListCtrl::OnMouseScroll, this);
}

KeyListCtrl::~KeyListCtrl()
{
}

MoviePanel* KeyListCtrl::GetMoviePanel()
{
	return dynamic_cast<MoviePanel*>(GetParent());
}

void KeyListCtrl::Append(int id, int time, int duration, int interp, const std::wstring& description)
{
	long tmp = InsertItem(GetItemCount(), wxString::Format("%d", id), 0);
	SetItem(tmp, 1, wxString::Format("%d", time));
	SetItem(tmp, 2, wxString::Format("%d", duration));
	SetItem(tmp, 3, interp == 0 ? "Linear" : "Smooth");
	SetItem(tmp, 4, description);
}

void KeyListCtrl::DeleteSel()
{
	if (auto panel = GetMoviePanel())
	{
		panel->DeleteKeyframe(panel->GetKeyframeNum());
	}
}

void KeyListCtrl::DeleteAll()
{
	if (auto panel = GetMoviePanel())
		panel->DeleteAllKeyframes();
}

void KeyListCtrl::SetKeyframes(
	const std::vector<KeyframeInfo>& keys)
{
	m_frame_text->Hide();
	m_duration_text->Hide();
	m_interpolation_cmb->Hide();
	m_description_text->Hide();

	m_editing_item = -1;

	DeleteAllItems();

	for (const auto& key : keys)
	{
		Append(
			key.id,
			key.time,
			key.duration,
			key.interpolation,
			key.description);
	}

	for (int i = 0; i < 4; ++i)
		SetColumnWidth(i, wxLIST_AUTOSIZE_USEHEADER);

	SetColumnWidth(4, wxLIST_AUTOSIZE);
}

wxString KeyListCtrl::GetText(long item, int col)
{
	wxListItem info;
	info.SetId(item);
	info.SetColumn(col);
	info.SetMask(wxLIST_MASK_TEXT);
	GetItem(info);
	return info.GetText();
}

void KeyListCtrl::SetText(long item, int col, wxString& str)
{
	wxListItem info;
	info.SetId(item);
	info.SetColumn(col);
	info.SetMask(wxLIST_MASK_TEXT);
	GetItem(info);
	info.SetText(str);
	SetItem(info);
}

void KeyListCtrl::OnSelection(wxListEvent& event)
{
	if (m_silent_select)
		return;

	long item = GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	m_editing_item = item;
	if (item != -1 && m_dragging_to_item == -1)
	{
		wxRect rect;
		wxString str;
		//add frame text
		GetSubItemRect(item, 1, rect);
		str = GetText(item, 1);
		m_frame_text->SetPosition(rect.GetTopLeft());
		m_frame_text->SetSize(rect.GetSize());
		m_frame_text->ChangeValue(str);
		//m_frame_text->Show();
		//add duration text
		GetSubItemRect(item, 2, rect);
		str = GetText(item, 2);
		m_duration_text->SetPosition(rect.GetTopLeft());
		m_duration_text->SetSize(rect.GetSize());
		m_duration_text->ChangeValue(str);
		m_duration_text->Show();
		//add interpolation combo
		GetSubItemRect(item, 3, rect);
		str = GetText(item, 3);
		m_interpolation_cmb->SetPosition(rect.GetTopLeft() - FromDIP(wxSize(0, 5)));
		m_interpolation_cmb->SetSize(FromDIP(wxSize(rect.GetSize().GetWidth(), -1)));
		int sel = 0;
		if (str == "Linear")
			sel = 0;
		else if (str == "Smooth")
			sel = 1;
		m_interpolation_cmb->Select(sel);
		m_interpolation_cmb->Show();
		//add description text
		GetSubItemRect(item, 4, rect);
		str = GetText(item, 4);
		m_description_text->SetPosition(rect.GetTopLeft());
		m_description_text->SetSize(rect.GetSize());
		m_description_text->ChangeValue(str);
		m_description_text->Show();
	}
}

void KeyListCtrl::EndEdit(bool refresh)
{
	if (m_duration_text->IsShown())
	{
		m_frame_text->Hide();
		m_duration_text->Hide();
		m_interpolation_cmb->Hide();
		m_description_text->Hide();

		m_editing_item = -1;

		if (refresh)
		{
			if (auto panel = GetMoviePanel())
				panel->UpdateParamList();
		}
	}
}

void KeyListCtrl::OnEndSelection(wxListEvent& event)
{
	EndEdit();
}

void KeyListCtrl::OnFrameText(wxCommandEvent& event)
{
	if (m_editing_item == -1)
		return;

	long id;
	GetItemText(m_editing_item).ToLong(&id);

	wxString str = m_frame_text->GetValue();

	double time;
	if (str.ToDouble(&time))
	{
		if (auto panel = GetMoviePanel())
			panel->SetKeyframeTime(int(id), time);

		SetText(m_editing_item, 1, str);
	}
}

void KeyListCtrl::OnDurationText(wxCommandEvent& event)
{
	if (m_editing_item == -1)
		return;

	long id;
	GetItemText(m_editing_item).ToLong(&id);

	wxString str = m_duration_text->GetValue();

	double duration;
	if (str.ToDouble(&duration))
	{
		if (auto panel = GetMoviePanel())
			panel->SetKeyframeDuration(int(id), duration);

		SetText(m_editing_item, 2, str);
	}
}

void KeyListCtrl::OnInterpoCmb(wxCommandEvent& event)
{
	if (m_editing_item == -1)
		return;

	long id;
	GetItemText(m_editing_item).ToLong(&id);

	int sel = m_interpolation_cmb->GetSelection();

	if (auto panel = GetMoviePanel())
		panel->SetKeyframeInterpolation(int(id), sel);

	wxString str =
		sel == 0 ? "Linear" : "Smooth";

	SetText(m_editing_item, 3, str);
}

void KeyListCtrl::OnDescritionText(wxCommandEvent& event)
{
	if (m_editing_item == -1)
		return;

	long id;
	GetItemText(m_editing_item).ToLong(&id);

	wxString str = m_description_text->GetValue();

	if (auto panel = GetMoviePanel())
	{
		panel->SetKeyframeDescription(
			int(id),
			str.ToStdWstring());
	}

	SetText(m_editing_item, 4, str);
}

void KeyListCtrl::OnKeyDown(wxKeyEvent& event)
{
	if (event.GetKeyCode() == WXK_DELETE ||
		event.GetKeyCode() == WXK_BACK)
		DeleteSel();
}

void KeyListCtrl::OnKeyUp(wxKeyEvent& event)
{
}

void KeyListCtrl::OnBeginDrag(wxListEvent& event)
{
	if (m_editing_item == -1)
		return;

	m_dragging_to_item = -1;
	// trigger when user releases left button (drop)
	Connect(wxEVT_MOTION, wxMouseEventHandler(KeyListCtrl::OnDragging), NULL, this);
	Connect(wxEVT_LEFT_UP, wxMouseEventHandler(KeyListCtrl::OnEndDrag), NULL, this);
	Connect(wxEVT_LEAVE_WINDOW, wxMouseEventHandler(KeyListCtrl::OnEndDrag), NULL, this);
	SetCursor(wxCursor(wxCURSOR_WATCH));

	m_frame_text->Hide();
	m_duration_text->Hide();
	m_interpolation_cmb->Hide();
	m_description_text->Hide();
}

void KeyListCtrl::OnDragging(wxMouseEvent& event)
{
	wxPoint pos = event.GetPosition();

	int flags = wxLIST_HITTEST_ONITEM;
	long target = HitTest(pos, flags, NULL);

	if (target < 0 ||
		target == m_editing_item ||
		target == m_dragging_to_item)
	{
		return;
	}

	long sourceId;
	GetItemText(m_editing_item).ToLong(&sourceId);

	long targetId;
	GetItemText(target).ToLong(&targetId);

	bool before = m_editing_item > target;

	if (auto panel = GetMoviePanel())
	{
		panel->MoveKeyframe(
			int(sourceId),
			int(targetId),
			before);
	}

	m_dragging_to_item = target;
}

void KeyListCtrl::OnEndDrag(wxMouseEvent& event)
{
	SetCursor(wxCursor(*wxSTANDARD_CURSOR));
	Disconnect(wxEVT_MOTION, wxMouseEventHandler(KeyListCtrl::OnDragging));
	Disconnect(wxEVT_LEFT_UP, wxMouseEventHandler(KeyListCtrl::OnEndDrag));
	Disconnect(wxEVT_LEAVE_WINDOW, wxMouseEventHandler(KeyListCtrl::OnEndDrag));
	m_dragging_to_item = -1;
}

void KeyListCtrl::OnScroll(wxScrollWinEvent& event)
{
	EndEdit(false);
	event.Skip();
}

void KeyListCtrl::OnMouseScroll(wxMouseEvent& event)
{
	EndEdit(false);
	event.Skip();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
wxWindow* MoviePanel::CreateSimplePage(wxWindow* parent)
{
	wxScrolledWindow* page = new wxScrolledWindow(parent);

	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;

	wxStaticText* st = 0, * st2 = 0;
	//sizers

	//rotations
	wxBoxSizer* sizer1 = new wxBoxSizer(wxHORIZONTAL);
	m_rot_chk = new wxCheckBox(page, wxID_ANY, "Rotation");
	m_rot_chk->Bind(wxEVT_CHECKBOX, &MoviePanel::OnRotateChecked, this);
	sizer1->Add(10, 10);
	sizer1->Add(m_rot_chk, 0, wxALIGN_CENTER);

	//axis
	wxBoxSizer* sizer2 = new wxBoxSizer(wxHORIZONTAL);
	m_x_rd = new wxRadioButton(page, wxID_ANY, "X",
		wxDefaultPosition, FromDIP(wxSize(30, 22)));
	m_y_rd = new wxRadioButton(page, wxID_ANY, "Y",
		wxDefaultPosition, FromDIP(wxSize(30, 22)));
	m_z_rd = new wxRadioButton(page, wxID_ANY, "Z",
		wxDefaultPosition, FromDIP(wxSize(30, 22)));
	m_x_rd->Bind(wxEVT_RADIOBUTTON, &MoviePanel::OnRotAxis, this);
	m_y_rd->Bind(wxEVT_RADIOBUTTON, &MoviePanel::OnRotAxis, this);
	m_z_rd->Bind(wxEVT_RADIOBUTTON, &MoviePanel::OnRotAxis, this);
	sizer2->Add(20, 5);
	sizer2->Add(m_x_rd, 0, wxALIGN_CENTER);
	sizer2->Add(20, 5);
	sizer2->Add(m_y_rd, 0, wxALIGN_CENTER);
	sizer2->Add(20, 5);
	sizer2->Add(m_z_rd, 0, wxALIGN_CENTER);

	//degrees
	wxBoxSizer* sizer3 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, wxID_ANY, "Range:",
		wxDefaultPosition, FromDIP(wxSize(200, -1)));
	m_degree_text = new wxTextCtrl(page, wxID_ANY, "360",
		wxDefaultPosition, FromDIP(wxSize(40, -1)), wxTE_RIGHT);
	m_degree_text->Bind(wxEVT_TEXT, &MoviePanel::OnDegreeText, this);
	st2 = new wxStaticText(page, wxID_ANY, "Deg.");
	sizer3->Add(20, 5, 0);
	sizer3->Add(st, 0, wxALIGN_CENTER);
	sizer3->Add(m_degree_text, 0, wxALIGN_CENTER);
	sizer3->Add(20, 5, 0);
	sizer3->Add(st2, 0, wxALIGN_CENTER);

	//rotation interpolation
	wxBoxSizer* sizer4 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, wxID_ANY, "Interpolation:",
		wxDefaultPosition, FromDIP(wxSize(200, -1)));
	m_rot_int_cmb = new wxComboBox(page, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(65, -1)), 0, NULL, wxCB_READONLY);
	std::vector<wxString> list = { "Linear", "Smooth" };
	m_rot_int_cmb->Append(list);
	m_rot_int_cmb->Select(0);
	m_rot_int_cmb->Bind(wxEVT_COMBOBOX, &MoviePanel::OnRotIntCmb, this);
	sizer4->Add(20, 5, 0);
	sizer4->Add(st, 0, wxALIGN_CENTER);
	sizer4->Add(m_rot_int_cmb, 0, wxALIGN_CENTER);

	//type
	wxBoxSizer* sizer5 = new wxBoxSizer(wxHORIZONTAL);
	m_seq_chk = new wxCheckBox(page, wxID_ANY, "Time Sequence");
	m_seq_chk->Bind(wxEVT_CHECKBOX, &MoviePanel::OnSequenceChecked, this);
	sizer5->Add(10, 10, 0);
	sizer5->Add(m_seq_chk, 0, wxALIGN_CENTER);

	wxBoxSizer* sizer6 = new wxBoxSizer(wxHORIZONTAL);
	m_bat_chk = new wxCheckBox(page, wxID_ANY, "Batch (Files in the same folder)");
	m_bat_chk->Bind(wxEVT_CHECKBOX, &MoviePanel::OnBatchChecked, this);
	sizer6->Add(10, 10, 0);
	sizer6->Add(m_bat_chk, 0, wxALIGN_CENTER);

	//sequence number
	wxBoxSizer* sizer7 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, wxID_ANY, "T: ");
	m_seq_dec_btn = new wxButton(page, wxID_ANY);
	m_seq_dec_btn->SetBitmap(wxGetBitmap(minus));
	wxSize bs = m_seq_dec_btn->GetBestSize();
	int h = bs.GetHeight();
	m_seq_dec_btn->SetMinSize(wxSize(h, h));
	m_seq_dec_btn->SetMaxSize(wxSize(h, h));
	m_seq_dec_btn->Bind(wxEVT_BUTTON, &MoviePanel::OnSeqDecBtn, this);
	m_seq_dec_btn->SetToolTip("Decrease the time point number by 1");
	m_seq_num_text = new wxTextCtrl(page, wxID_ANY, "0",
		wxDefaultPosition, FromDIP(wxSize(40, -1)), wxTE_RIGHT, vald_int);
	m_seq_num_text->Bind(wxEVT_TEXT, &MoviePanel::OnSeqNumText, this);
	m_seq_inc_btn = new wxButton(page, wxID_ANY, "",
		wxDefaultPosition, bs);
	m_seq_inc_btn->SetBitmap(wxGetBitmap(plus));
	m_seq_inc_btn->SetMinSize(wxSize(h, h));
	m_seq_inc_btn->SetMaxSize(wxSize(h, h));
	m_seq_inc_btn->Bind(wxEVT_BUTTON, &MoviePanel::OnSeqIncBtn, this);
	m_seq_inc_btn->SetToolTip("Increase the time point number by 1");
	st2 = new wxStaticText(page, wxID_ANY, "of: ");
	m_seq_total_text = new wxTextCtrl(page, wxID_ANY, "0",
		wxDefaultPosition, FromDIP(wxSize(40, -1)), wxTE_RIGHT | wxTE_READONLY);
	sizer7->Add(20, 20);
	sizer7->Add(st, 0, wxALIGN_CENTER);
	sizer7->Add(10, 10);
	sizer7->Add(m_seq_dec_btn, 0, wxALIGN_CENTER);
	sizer7->Add(m_seq_num_text, 0, wxALIGN_CENTER);
	sizer7->Add(m_seq_inc_btn, 0, wxALIGN_CENTER);
	sizer7->Add(20, 20);
	sizer7->Add(st2, 0, wxALIGN_CENTER);
	sizer7->Add(m_seq_total_text, 0, wxALIGN_CENTER);

	//vertical sizer
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
	sizer_v->Add(5, 5, 0);
	sizer_v->Add(sizer1, 0, wxEXPAND);
	sizer_v->Add(5, 5, 0);
	sizer_v->Add(sizer2, 0, wxEXPAND);
	sizer_v->Add(5, 5, 0);
	sizer_v->Add(sizer3, 0, wxEXPAND);
	sizer_v->Add(5, 5, 0);
	sizer_v->Add(sizer4, 0, wxEXPAND);
	sizer_v->Add(5, 20, 0);
	sizer_v->Add(sizer5, 0, wxEXPAND);
	sizer_v->Add(5, 20, 0);
	sizer_v->Add(sizer6, 0, wxEXPAND);
	sizer_v->Add(5, 20, 0);
	sizer_v->Add(sizer7, 0, wxEXPAND);
	//set the page
	page->SetSizer(sizer_v);
	page->SetAutoLayout(true);
	page->SetScrollRate(10, 10);
	return page;
}

wxWindow* MoviePanel::CreateKeyframePage(wxWindow* parent)
{
	wxScrolledWindow* page = new wxScrolledWindow(parent);

	wxIntegerValidator<unsigned int> vald_int;
	wxStaticText* st = 0;

	//check
	wxBoxSizer* sizer1 = new wxBoxSizer(wxHORIZONTAL);
	m_keyframe_chk = new wxCheckBox(page, wxID_ANY, "Enable keyframe movie");
	m_keyframe_chk->Bind(wxEVT_CHECKBOX, &MoviePanel::OnKeyframeChk, this);
	sizer1->Add(5, 5);
	sizer1->Add(m_keyframe_chk, 0, wxALIGN_CENTER);

	//default duration
	wxBoxSizer* sizer2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, wxID_ANY, "Duration:");
	m_duration_text = new wxTextCtrl(page, wxID_ANY, "30",
		wxDefaultPosition, FromDIP(wxSize(30, 23)), wxTE_RIGHT, vald_int);
	m_duration_text->Bind(wxEVT_TEXT, &MoviePanel::OnDurationText, this);
	m_duration_text->SetToolTip("Set the default duration between two keyframes");
	sizer2->Add(5, 5);
	sizer2->Add(st, 0, wxALIGN_CENTER);
	sizer2->Add(5, 5);
	sizer2->Add(m_duration_text, 0, wxALIGN_CENTER);
	sizer2->Add(5, 5);
	st = new wxStaticText(page, wxID_ANY, "Interpolation:");
	m_interpolation_cmb = new wxComboBox(page, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(65, -1)), 0, NULL, wxCB_READONLY);
	m_interpolation_cmb->Bind(wxEVT_COMBOBOX, &MoviePanel::OnInterpolation, this);
	std::vector<wxString> list = { "Linear", "Smooth" };
	m_interpolation_cmb->Append(list);
	sizer2->Add(st, 0, wxALIGN_CENTER);
	sizer2->Add(5, 5);
	sizer2->Add(m_interpolation_cmb, 0, wxALIGN_CENTER);

	//list
	m_keylist = new KeyListCtrl(page,
		wxDefaultPosition, FromDIP(wxSize(200, 200)), wxLC_REPORT);
	m_keylist->Bind(wxEVT_LIST_ITEM_ACTIVATED, &MoviePanel::OnAct, this);

	//key buttons
	wxBoxSizer* sizer3 = new wxBoxSizer(wxHORIZONTAL);
	m_set_key_btn = new wxButton(page, wxID_ANY, "Add",
		wxDefaultPosition, FromDIP(wxSize(50, 23)));
	m_set_key_btn->Bind(wxEVT_BUTTON, &MoviePanel::OnInsKey, this);
	m_set_key_btn->SetToolTip("Add or insert a keyframe");
	m_del_key_btn = new wxButton(page, wxID_ANY, "Delete",
		wxDefaultPosition, FromDIP(wxSize(55, 23)));
	m_del_key_btn->Bind(wxEVT_BUTTON, &MoviePanel::OnDelKey, this);
	m_del_key_btn->SetToolTip("Delete the selected keyframe");
	m_del_all_btn = new wxButton(page, wxID_ANY, "Del. All",
		wxDefaultPosition, FromDIP(wxSize(60, 23)));
	m_del_all_btn->Bind(wxEVT_BUTTON, &MoviePanel::OnDelAll, this);
	m_del_all_btn->SetToolTip("Delete all keyframes");
	sizer3->AddStretchSpacer(1);
	sizer3->Add(m_set_key_btn, 0, wxALIGN_CENTER);
	sizer3->Add(5, 5);
	sizer3->Add(m_del_key_btn, 0, wxALIGN_CENTER);
	sizer3->Add(5, 5);
	sizer3->Add(m_del_all_btn, 0, wxALIGN_CENTER);
	sizer3->Add(5, 5);

	//lock cam center object
	wxBoxSizer* sizer4 = new wxBoxSizer(wxHORIZONTAL);
	m_cam_lock_chk = new wxCheckBox(page, wxID_ANY,
		"Lock View Target:");
	m_cam_lock_chk->Bind(wxEVT_CHECKBOX, &MoviePanel::OnCamLockChk, this);
	sizer4->Add(5, 5);
	sizer4->Add(m_cam_lock_chk, 0, wxALIGN_CENTER);
	wxBoxSizer* sizer5 = new wxBoxSizer(wxHORIZONTAL);
	m_cam_lock_cmb = new wxComboBox(page, wxID_ANY, "",
		wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY);
	std::vector<wxString> list2 = { "Image center", "Click view", "Ruler", "Selection" };
	m_cam_lock_cmb->Append(list2);
	m_cam_lock_cmb->Bind(wxEVT_COMBOBOX, &MoviePanel::OnCamLockCmb, this);
	m_cam_lock_btn = new wxButton(page, wxID_ANY, "Apply");
	m_cam_lock_btn->Bind(wxEVT_BUTTON, &MoviePanel::OnCamLockBtn, this);
	m_cam_lock_btn->SetToolTip("Apply camera viewing direction lock to the target");
	sizer5->AddStretchSpacer(1);
	sizer5->Add(m_cam_lock_cmb, 0, wxALIGN_CENTER);
	sizer5->Add(5, 5);
	sizer5->Add(m_cam_lock_btn, 0, wxALIGN_CENTER);
	sizer5->Add(5, 5);


	//vertical sizer
	wxBoxSizer* sizerv = new wxBoxSizer(wxVERTICAL);
	sizerv->Add(5, 5, 0);
	sizerv->Add(sizer1, 0, wxEXPAND);
	sizerv->Add(10, 5);
	sizerv->Add(sizer2, 0, wxEXPAND);
	sizerv->Add(10, 5);
	sizerv->Add(m_keylist, 1, wxEXPAND);
	sizerv->Add(10, 5);
	sizerv->Add(sizer3, 0, wxEXPAND);
	sizerv->Add(5, 5);
	sizerv->Add(sizer4, 0, wxEXPAND);
	sizerv->Add(5, 5);
	sizerv->Add(sizer5, 0, wxEXPAND);
	sizerv->Add(5, 5);
	//set the page
	page->SetSizer(sizerv);
	Layout();
	page->SetAutoLayout(true);
	page->SetScrollRate(10, 10);
	return page;
}

wxWindow* MoviePanel::CreateTemplatePage(wxWindow* parent)
{
	wxScrolledWindow* page = new wxScrolledWindow(parent);

	wxStaticText* st = new wxStaticText(page, 0, "Double-click a preset to generate keyframes");

	//list of options
	m_auto_key_list = new wxListCtrl(page, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
	wxListItem itemCol;
	itemCol.SetText("No.");
	m_auto_key_list->InsertColumn(0, itemCol);
	itemCol.SetText("Keyframe Preset");
	m_auto_key_list->InsertColumn(1, itemCol);
	m_auto_key_list->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
	m_auto_key_list->SetColumnWidth(1, wxLIST_AUTOSIZE);
	m_auto_key_list->Bind(wxEVT_LIST_ITEM_ACTIVATED, &MoviePanel::OnGenKey, this);

	//button
	m_gen_keys_btn = new wxButton(page, wxID_ANY, "Generate keyframes");
	m_gen_keys_btn->Bind(wxEVT_BUTTON, &MoviePanel::OnGenKey, this);

	//vertical sizer
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
	sizer_v->Add(10, 10, 0);
	sizer_v->Add(st, 0, wxALIGN_LEFT);
	sizer_v->Add(10, 10, 0);
	sizer_v->Add(m_auto_key_list, 1, wxEXPAND);
	sizer_v->Add(10, 10, 0);
	sizer_v->Add(m_gen_keys_btn, 0, wxALIGN_RIGHT);
	sizer_v->Add(10, 10, 0);

	page->SetSizer(sizer_v);
	page->SetAutoLayout(true);
	page->SetScrollRate(10, 10);
	return page;

}

wxWindow* MoviePanel::CreateCropPage(wxWindow* parent)
{
	wxScrolledWindow* page = new wxScrolledWindow(parent);

	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;
	wxStaticText* st = 0;

	//check
	wxBoxSizer* sizer1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Enable Cropping:",
		wxDefaultPosition, wxDefaultSize);
	m_crop_chk = new wxCheckBox(page, wxID_ANY, "");
	m_crop_chk->Bind(wxEVT_CHECKBOX, &MoviePanel::OnCropCheck, this);
	sizer1->Add(5, 5, 0);
	sizer1->Add(st, 0, wxALIGN_CENTER);
	sizer1->Add(10, 10, 0);
	sizer1->Add(m_crop_chk, 0, wxALIGN_CENTER);
	//corner coords
	wxBoxSizer* sizer2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "X:",
		wxDefaultPosition, FromDIP(wxSize(20, 20)));
	m_crop_x_text = new wxTextCtrl(page, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_int);
	m_crop_x_text->Bind(wxEVT_TEXT, &MoviePanel::OnEditCrop, this);
	m_crop_x_text->SetToolTip("Also drag the yellow frame in render view");
	m_crop_x_spin = new wxSpinButton(page, wxID_ANY,
		wxDefaultPosition, FromDIP(wxSize(20, 20)));
	m_crop_x_spin->SetRange(-0x8000, 0x7fff);
	m_crop_x_spin->Bind(wxEVT_SPIN_UP, &MoviePanel::OnCropSpinUp, this);
	m_crop_x_spin->Bind(wxEVT_SPIN_DOWN, &MoviePanel::OnCropSpinDown, this);
	sizer2->Add(20, 20, 0);
	sizer2->Add(st, 0, wxALIGN_CENTER);
	sizer2->Add(m_crop_x_text, 0, wxALIGN_CENTER);
	sizer2->Add(m_crop_x_spin, 0, wxALIGN_CENTER);
	st = new wxStaticText(page, 0, "Y:",
		wxDefaultPosition, FromDIP(wxSize(20, 20)));
	m_crop_y_text = new wxTextCtrl(page, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_int);
	m_crop_y_text->Bind(wxEVT_TEXT, &MoviePanel::OnEditCrop, this);
	m_crop_y_text->SetToolTip("Also drag the yellow frame in render view");
	m_crop_y_spin = new wxSpinButton(page, wxID_ANY,
		wxDefaultPosition, FromDIP(wxSize(20, 20)));
	m_crop_y_spin->SetRange(-0x8000, 0x7fff);
	m_crop_y_spin->Bind(wxEVT_SPIN_UP, &MoviePanel::OnCropSpinUp, this);
	m_crop_y_spin->Bind(wxEVT_SPIN_DOWN, &MoviePanel::OnCropSpinDown, this);
	sizer2->Add(20, 20, 0);
	sizer2->Add(st, 0, wxALIGN_CENTER);
	sizer2->Add(m_crop_y_text, 0, wxALIGN_CENTER);
	sizer2->Add(m_crop_y_spin, 0, wxALIGN_CENTER);
	sizer2->Add(20, 20, 0);
	//size
	wxBoxSizer* sizer3 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "W:",
		wxDefaultPosition, FromDIP(wxSize(20, 20)));
	m_crop_w_text = new wxTextCtrl(page, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_int);
	m_crop_w_text->Bind(wxEVT_TEXT, &MoviePanel::OnEditCrop, this);
	m_crop_w_text->SetToolTip("Also drag the cropping frame in render view");
	m_crop_w_spin = new wxSpinButton(page, wxID_ANY,
		wxDefaultPosition, FromDIP(wxSize(20, 20)));
	m_crop_w_spin->SetRange(-0x8000, 0x7fff);
	m_crop_w_spin->Bind(wxEVT_SPIN_UP, &MoviePanel::OnCropSpinUp, this);
	m_crop_w_spin->Bind(wxEVT_SPIN_DOWN, &MoviePanel::OnCropSpinDown, this);
	sizer3->Add(20, 20, 0);
	sizer3->Add(st, 0, wxALIGN_CENTER);
	sizer3->Add(m_crop_w_text, 0, wxALIGN_CENTER);
	sizer3->Add(m_crop_w_spin, 0, wxALIGN_CENTER);
	st = new wxStaticText(page, 0, "H:",
		wxDefaultPosition, FromDIP(wxSize(20, 20)));
	m_crop_h_text = new wxTextCtrl(page, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_int);
	m_crop_h_text->Bind(wxEVT_TEXT, &MoviePanel::OnEditCrop, this);
	m_crop_h_text->SetToolTip("Also drag the cropping frame in render view");
	m_crop_h_spin = new wxSpinButton(page, wxID_ANY,
		wxDefaultPosition, FromDIP(wxSize(20, 20)));
	m_crop_h_spin->SetRange(-0x8000, 0x7fff);
	m_crop_h_spin->Bind(wxEVT_SPIN_UP, &MoviePanel::OnCropSpinUp, this);
	m_crop_h_spin->Bind(wxEVT_SPIN_DOWN, &MoviePanel::OnCropSpinDown, this);
	sizer3->Add(20, 20, 0);
	sizer3->Add(st, 0, wxALIGN_CENTER);
	sizer3->Add(m_crop_h_text, 0, wxALIGN_CENTER);
	sizer3->Add(m_crop_h_spin, 0, wxALIGN_CENTER);
	sizer3->Add(20, 20, 0);
	//scalebar
	wxBoxSizer* sizer4 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Scalebar Position",
		wxDefaultPosition, wxDefaultSize);
	sizer4->Add(5, 5, 0);
	sizer4->Add(st, 0, wxALIGN_CENTER);
	//radiobuttons
	wxBoxSizer* sizer5 = new wxBoxSizer(wxHORIZONTAL);
	m_sb_tl_rb = new wxRadioButton(page, wxID_ANY, "Top-Left",
		wxDefaultPosition, FromDIP(wxSize(100, 20)), wxRB_GROUP);
	m_sb_tr_rb = new wxRadioButton(page, wxID_ANY, "Top-Right",
		wxDefaultPosition, FromDIP(wxSize(100, 20)));
	m_sb_tl_rb->Bind(wxEVT_RADIOBUTTON, &MoviePanel::OnSbRadio, this);
	m_sb_tr_rb->Bind(wxEVT_RADIOBUTTON, &MoviePanel::OnSbRadio, this);
	sizer5->Add(20, 20, 0);
	sizer5->Add(m_sb_tl_rb, 0, wxALIGN_CENTER);
	sizer5->Add(20, 20, 0);
	sizer5->Add(m_sb_tr_rb, 0, wxALIGN_CENTER);
	wxBoxSizer* sizer6 = new wxBoxSizer(wxHORIZONTAL);
	m_sb_bl_rb = new wxRadioButton(page, wxID_ANY, "Bottom-Left",
		wxDefaultPosition, FromDIP(wxSize(100, 20)));
	m_sb_br_rb = new wxRadioButton(page, wxID_ANY, "Bottom-Right",
		wxDefaultPosition, FromDIP(wxSize(100, 20)));
	m_sb_bl_rb->Bind(wxEVT_RADIOBUTTON, &MoviePanel::OnSbRadio, this);
	m_sb_br_rb->Bind(wxEVT_RADIOBUTTON, &MoviePanel::OnSbRadio, this);
	sizer6->Add(20, 20, 0);
	sizer6->Add(m_sb_bl_rb, 0, wxALIGN_CENTER);
	sizer6->Add(20, 20, 0);
	sizer6->Add(m_sb_br_rb, 0, wxALIGN_CENTER);
	//space
	wxBoxSizer* sizer7 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "X:",
		wxDefaultPosition, FromDIP(wxSize(20, 20)));
	m_sb_dx_text = new wxTextCtrl(page, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_int);
	m_sb_dx_text->Bind(wxEVT_TEXT, &MoviePanel::OnSbEdit, this);
	m_sb_dx_spin = new wxSpinButton(page, wxID_ANY,
		wxDefaultPosition, FromDIP(wxSize(20, 20)));
	m_sb_dx_spin->SetRange(-0x8000, 0x7fff);
	m_sb_dx_spin->Bind(wxEVT_SPIN_UP, &MoviePanel::OnSbSpinUp, this);
	m_sb_dx_spin->Bind(wxEVT_SPIN_DOWN, &MoviePanel::OnSbSpinDown, this);
	sizer7->Add(20, 20, 0);
	sizer7->Add(st, 0, wxALIGN_CENTER);
	sizer7->Add(m_sb_dx_text, 0, wxALIGN_CENTER);
	sizer7->Add(m_sb_dx_spin, 0, wxALIGN_CENTER);
	st = new wxStaticText(page, 0, "Y:",
		wxDefaultPosition, FromDIP(wxSize(20, 20)));
	m_sb_dy_text = new wxTextCtrl(page, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_int);
	m_sb_dy_text->Bind(wxEVT_TEXT, &MoviePanel::OnSbEdit, this);
	m_sb_dy_spin = new wxSpinButton(page, wxID_ANY,
		wxDefaultPosition, FromDIP(wxSize(20, 20)));
	m_sb_dy_spin->SetRange(-0x8000, 0x7fff);
	m_sb_dy_spin->Bind(wxEVT_SPIN_UP, &MoviePanel::OnSbSpinUp, this);
	m_sb_dy_spin->Bind(wxEVT_SPIN_DOWN, &MoviePanel::OnSbSpinDown, this);
	sizer7->Add(20, 20, 0);
	sizer7->Add(st, 0, wxALIGN_CENTER);
	sizer7->Add(m_sb_dy_text, 0, wxALIGN_CENTER);
	sizer7->Add(m_sb_dy_spin, 0, wxALIGN_CENTER);
	sizer7->Add(20, 20, 0);
	//reset
	wxBoxSizer* sizer8 = new wxBoxSizer(wxHORIZONTAL);
	m_reset_btn = new wxButton(page, wxID_ANY, "Reset",
		wxDefaultPosition, wxDefaultSize);
	m_reset_btn->Bind(wxEVT_BUTTON, &MoviePanel::OnResetCrop, this);
	m_reset_btn->SetBitmap(wxGetBitmap(reset));
	m_reset_btn->SetToolTip("Also drag the yellow frame in render view");
	sizer8->Add(20, 20, 0);
	sizer8->Add(m_reset_btn, 0, wxALIGN_CENTER);

	//vertical sizer
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
	sizer_v->Add(10, 10, 0);
	sizer_v->Add(sizer1, 0, wxEXPAND);
	sizer_v->Add(10, 10, 0);
	sizer_v->Add(sizer2, 0, wxEXPAND);
	sizer_v->Add(10, 10, 0);
	sizer_v->Add(sizer3, 0, wxEXPAND);
	sizer_v->Add(10, 10, 0);
	sizer_v->Add(sizer4, 0, wxEXPAND);
	sizer_v->Add(10, 10, 0);
	sizer_v->Add(sizer5, 0, wxEXPAND);
	sizer_v->Add(10, 10, 0);
	sizer_v->Add(sizer6, 0, wxEXPAND);
	sizer_v->Add(10, 10, 0);
	sizer_v->Add(sizer7, 0, wxEXPAND);
	sizer_v->Add(10, 10, 0);
	sizer_v->Add(sizer8, 0, wxEXPAND);

	page->SetSizer(sizer_v);
	page->SetAutoLayout(true);
	page->SetScrollRate(10, 10);
	return page;

}

wxWindow* MoviePanel::CreateScriptPage(wxWindow* parent)
{
	wxScrolledWindow* page = new wxScrolledWindow(parent);
	//script
	//vertical sizer
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
	m_run_script_chk = new wxCheckBox(page, wxID_ANY,
		"Enable execution of a script during playback.");
	m_run_script_chk->Bind(wxEVT_CHECKBOX, &MoviePanel::OnRunScriptChk, this);
	wxStaticText* st;
	sizer_v->Add(10, 10);
	sizer_v->Add(m_run_script_chk);
	sizer_v->Add(5, 5);

	//browse button
	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Script File:",
		wxDefaultPosition, FromDIP(wxSize(80, -1)));
	m_script_file_btn = new wxButton(page, wxID_ANY, "Browse...",
		wxDefaultPosition, FromDIP(wxSize(80, -1)));
	m_script_file_btn->Bind(wxEVT_BUTTON, &MoviePanel::OnScriptFileBtn, this);
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	sizer_1->AddStretchSpacer(1);
	sizer_1->Add(m_script_file_btn, 0, wxALIGN_CENTER);

	//file name
	wxBoxSizer* sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	m_script_file_text = new wxTextCtrl(page, wxID_ANY, "",
		wxDefaultPosition, wxDefaultSize);
	m_script_clear_btn = new wxButton(page, wxID_ANY, "X",
		wxDefaultPosition, FromDIP(wxSize(25, -1)));
	m_script_file_text->Bind(wxEVT_TEXT, &MoviePanel::OnScriptFileEdit, this);
	m_script_clear_btn->Bind(wxEVT_BUTTON, &MoviePanel::OnScriptClearBtn, this);
	sizer_2->Add(m_script_file_text, 1, wxEXPAND);
	sizer_2->Add(m_script_clear_btn, 0, wxALIGN_CENTER);

	//script list
	m_script_list = new wxListCtrl(page, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
	wxListItem itemCol;
	itemCol.SetText("No.");
	m_script_list->InsertColumn(0, itemCol);
	itemCol.SetText("Built-in Script Files");
	m_script_list->InsertColumn(1, itemCol);
	m_script_list->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
	m_script_list->SetColumnWidth(1, wxLIST_AUTOSIZE);
	m_script_list->Bind(wxEVT_LIST_ITEM_SELECTED, &MoviePanel::OnScriptListSelected, this);
	m_script_list->Bind(wxEVT_LIST_ITEM_ACTIVATED, &MoviePanel::OnScriptListSelected, this);

	sizer_v->Add(5, 5);
	sizer_v->Add(sizer_1, 0, wxEXPAND);
	sizer_v->Add(5, 5);
	sizer_v->Add(sizer_2, 0, wxEXPAND);
	sizer_v->Add(5, 5);
	sizer_v->Add(m_script_list, 1, wxEXPAND);

	//set the page
	page->SetSizer(sizer_v);
	page->SetAutoLayout(true);
	page->SetScrollRate(10, 10);
	return page;
}

MoviePanel::MoviePanel(wxWindow* parent,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name) :
	TabbedPanel(parent, pos, size, style, name)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);
	Freeze();
	//SetDoubleBuffered(true);
	wxIntegerValidator<unsigned int> vald_int;

	//Root* root = glbin_data_manager.GetRoot();
	//if (root)
	//	m_view = root->GetView(glbin_mov_def.m_view_idx).get();

	//notebook
	m_notebook = new wxAuiNotebook(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize,
		wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE |
		wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_TAB_EXTERNAL_MOVE |
		wxAUI_NB_WINDOWLIST_BUTTON | wxNO_BORDER);
	m_notebook->AddPage(CreateSimplePage(m_notebook), UITEXT_NBPG0, true);
	m_notebook->AddPage(CreateKeyframePage(m_notebook), UITEXT_NBPG1);
	m_notebook->AddPage(CreateTemplatePage(m_notebook), UITEXT_NBPG2);
	m_notebook->AddPage(CreateCropPage(m_notebook), UITEXT_NBPG3);
	m_notebook->AddPage(CreateScriptPage(m_notebook), UITEXT_NBPG4_0);
	m_notebook->Bind(wxEVT_AUINOTEBOOK_PAGE_CHANGED, &MoviePanel::OnNotebookPage, this);

	wxStaticText* st = 0, * st2 = 0;
	//common settings
	wxBoxSizer* sizer1 = new wxBoxSizer(wxHORIZONTAL);
	//FPS
	st = new wxStaticText(this, wxID_ANY, "FPS: ");
	m_fps_text = new wxTextCtrl(this, wxID_ANY, "30",
		wxDefaultPosition, FromDIP(wxSize(30, -1)), wxTE_RIGHT);
	m_fps_text->Bind(wxEVT_TEXT, &MoviePanel::OnFpsEdit, this);
	sizer1->Add(5, 5);
	sizer1->Add(st, 0, wxALIGN_CENTER);
	sizer1->Add(m_fps_text, 0, wxALIGN_CENTER);
	//movie length
	st = new wxStaticText(this, wxID_ANY, "Length: ");
	st2 = new wxStaticText(this, wxID_ANY, "Sec.");
	m_movie_len_text = new wxTextCtrl(this, wxID_ANY, "0",
		wxDefaultPosition, FromDIP(wxSize(50, -1)), wxTE_RIGHT);
	m_movie_len_text->Bind(wxEVT_TEXT, &MoviePanel::OnMovieLenText, this);
	sizer1->Add(5, 5);
	sizer1->Add(st, 0, wxALIGN_CENTER);
	sizer1->Add(5, 5);
	sizer1->Add(m_movie_len_text, 0, wxALIGN_CENTER);
	sizer1->Add(st2, 0, wxALIGN_CENTER);
	//view
	st = new wxStaticText(this, wxID_ANY, "For: ");
	m_views_cmb = new wxComboBox(this, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(100, -1)), 0, NULL, wxCB_READONLY);
	m_views_cmb->Bind(wxEVT_COMBOBOX, &MoviePanel::OnViewSelected, this);
	sizer1->AddStretchSpacer();
	sizer1->Add(st, 0, wxALIGN_CENTER);
	sizer1->Add(m_views_cmb, 0, wxALIGN_CENTER);
	sizer1->Add(5, 5);

	//slider
	wxBoxSizer* sizer2 = new wxBoxSizer(wxHORIZONTAL);
	m_slider_btn = new wxUndoableToolbar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	m_slider_btn->SetDoubleBuffered(true);
	m_slider_btn->AddToolWithHelp(
		0, "Slider style", wxGetBitmap(slider),
		"Choose slider style between jog and normal");
	m_slider_btn->Bind(wxEVT_TOOL, &MoviePanel::OnSliderStyle, this);
	m_slider_btn->Realize();
	m_progress_sldr = new wxUndoableScrollBar(this, wxID_ANY,
		wxDefaultPosition, FromDIP(wxSize(-1, 20)));
	m_progress_sldr->Bind(wxEVT_SCROLL_CHANGED, &MoviePanel::OnProgressScroll, this);
	sizer2->Add(5, 5);
	sizer2->Add(m_slider_btn, 0, wxALIGN_CENTER);
	sizer2->Add(5, 5);
	sizer2->Add(m_progress_sldr, 1, wxALIGN_CENTER);
	sizer2->Add(5, 5);

	wxSize ts = FromDIP(wxSize(50, -1));
	wxFont f;

	// =====================
	// Create controls
	// =====================

	// --- buttons (create first so we can size them consistently)
	m_start_btn = new wxButton(this, wxID_ANY);
	m_start_btn->SetBitmap(wxGetBitmap(start));

	wxSize best = m_start_btn->GetBestSize();
	int h = best.GetHeight();

	// helper lambda to enforce square buttons
	auto MakeSquare = [&](wxWindow* w)
		{
			w->SetMinSize(wxSize(h, h));
			w->SetMaxSize(wxSize(h, h));
		};

	MakeSquare(m_start_btn);
	m_start_btn->Bind(wxEVT_BUTTON, &MoviePanel::OnStartFrameBtn, this);

	// --- text controls
	m_start_frame_text = new wxTextCtrl(this, wxID_ANY, "1",
		wxDefaultPosition, FromDIP(wxSize(45, -1)), wxTE_RIGHT, vald_int);

	f = m_start_frame_text->GetFont();
	f.MakeLarger();
	m_start_frame_text->SetFont(f);

	m_end_frame_text = new wxTextCtrl(this, wxID_ANY, "10",
		wxDefaultPosition, FromDIP(wxSize(45, -1)), wxTE_RIGHT, vald_int);
	m_end_frame_text->SetFont(f);

	m_cur_frame_text = new wxTextCtrl(this, wxID_ANY, "0",
		wxDefaultPosition, ts, wxTE_RIGHT, vald_int);
	m_cur_frame_text->SetFont(f);

	m_full_frame_text = new wxTextCtrl(this, wxID_ANY, "0",
		wxDefaultPosition, ts, wxTE_RIGHT, vald_int);
	m_full_frame_text->SetFont(f);

	// --- remaining buttons
	m_end_btn = new wxButton(this, wxID_ANY);
	m_end_btn->SetBitmap(wxGetBitmap(end));
	MakeSquare(m_end_btn);
	m_end_btn->Bind(wxEVT_BUTTON, &MoviePanel::OnEndFrameBtn, this);

	m_dec_time_btn = new wxButton(this, wxID_ANY);
	m_dec_time_btn->SetBitmap(wxGetBitmap(step_back));
	MakeSquare(m_dec_time_btn);
	m_dec_time_btn->Bind(wxEVT_BUTTON, &MoviePanel::OnDecFrame, this);

	m_inc_time_btn = new wxButton(this, wxID_ANY);
	m_inc_time_btn->SetBitmap(wxGetBitmap(step_forward));
	MakeSquare(m_inc_time_btn);
	m_inc_time_btn->Bind(wxEVT_BUTTON, &MoviePanel::OnIncFrame, this);

	// --- playback controls
	m_rewind_btn = new wxButton(this, wxID_ANY);
	m_rewind_btn->SetBitmap(wxGetBitmap(rewind));
	MakeSquare(m_rewind_btn);
	m_rewind_btn->Bind(wxEVT_BUTTON, &MoviePanel::OnRewind, this);

	m_play_inv_btn = new wxToggleButton(this, wxID_ANY, "");
	m_play_inv_btn->SetBitmap(wxGetBitmap(play_inv));
	MakeSquare(m_play_inv_btn);
	m_play_inv_btn->Bind(wxEVT_TOGGLEBUTTON, &MoviePanel::OnPlayInv, this);

	m_play_btn = new wxToggleButton(this, wxID_ANY, "");
	m_play_btn->SetBitmap(wxGetBitmap(play));
	MakeSquare(m_play_btn);
	m_play_btn->Bind(wxEVT_TOGGLEBUTTON, &MoviePanel::OnPlay, this);

	m_forward_btn = new wxButton(this, wxID_ANY);
	m_forward_btn->SetBitmap(wxGetBitmap(forward));
	MakeSquare(m_forward_btn);
	m_forward_btn->Bind(wxEVT_BUTTON, &MoviePanel::OnForward, this);

	m_loop_btn = new wxToggleButton(this, wxID_ANY, "");
	m_loop_btn->SetBitmap(wxGetBitmap(loop));
	MakeSquare(m_loop_btn);
	m_loop_btn->Bind(wxEVT_TOGGLEBUTTON, &MoviePanel::OnLoop, this);

	// --- right-side controls
	m_progress_text = new wxTextCtrl(this, wxID_ANY, "0.00",
		wxDefaultPosition, ts, wxTE_RIGHT);
	m_progress_text->SetFont(f);

	st = new wxStaticText(this, wxID_ANY, "Sec.",
		wxDefaultPosition);

	m_save_btn = new wxButton(this, wxID_ANY);
	m_save_btn->SetBitmap(wxGetBitmap(save));
	MakeSquare(m_save_btn);
	m_save_btn->Bind(wxEVT_BUTTON, &MoviePanel::OnSave, this);

	// =====================
	// Sizer flags (consistent spacing)
	// =====================
	wxSizerFlags center = wxSizerFlags(0)
		.Align(wxALIGN_CENTER_VERTICAL)
		.Border(wxRIGHT, FromDIP(3));

	// =====================
	// Row 1 (time range)
	// =====================
	wxBoxSizer* group1 = new wxBoxSizer(wxHORIZONTAL);
	group1->Add(m_start_btn, center);
	group1->Add(m_start_frame_text, center);
	group1->Add(m_end_frame_text, center);
	group1->Add(m_end_btn, wxSizerFlags(0).Align(wxALIGN_CENTER_VERTICAL));

	wxBoxSizer* group2 = new wxBoxSizer(wxHORIZONTAL);
	group2->Add(m_dec_time_btn, center);
	group2->Add(m_cur_frame_text, center);
	group2->Add(m_inc_time_btn, wxSizerFlags(0).Align(wxALIGN_CENTER_VERTICAL));

	wxBoxSizer* group3 = new wxBoxSizer(wxHORIZONTAL);
	group3->Add(m_full_frame_text, wxSizerFlags(0).Align(wxALIGN_CENTER_VERTICAL));

	wxBoxSizer* sizer3 = new wxBoxSizer(wxHORIZONTAL);
	sizer3->AddStretchSpacer(2);
	sizer3->Add(group1, 0, wxALIGN_CENTER_VERTICAL);
	sizer3->AddStretchSpacer(1);
	sizer3->Add(group2, 0, wxALIGN_CENTER_VERTICAL);
	sizer3->AddStretchSpacer(2);
	sizer3->Add(group3, 0, wxALIGN_CENTER_VERTICAL);
	sizer3->AddSpacer(FromDIP(5));

	// =====================
	// Row 2 (playback)
	// =====================
	wxBoxSizer* playback = new wxBoxSizer(wxHORIZONTAL);
	playback->Add(m_rewind_btn, center);
	playback->Add(m_play_inv_btn, center);
	playback->Add(m_play_btn, center);
	playback->Add(m_forward_btn, center);
	playback->Add(m_loop_btn, wxSizerFlags(0).Align(wxALIGN_CENTER_VERTICAL));

	wxBoxSizer* timegroup = new wxBoxSizer(wxHORIZONTAL);
	timegroup->Add(m_progress_text, center);
	timegroup->Add(st, wxSizerFlags(0).Align(wxALIGN_CENTER_VERTICAL));

	wxBoxSizer* savegroup = new wxBoxSizer(wxHORIZONTAL);
	savegroup->Add(m_save_btn, wxSizerFlags(0).Align(wxALIGN_CENTER_VERTICAL));

	wxBoxSizer* sizer4 = new wxBoxSizer(wxHORIZONTAL);
	sizer4->AddStretchSpacer(2);
	sizer4->Add(playback, 0, wxALIGN_CENTER_VERTICAL);
	sizer4->AddStretchSpacer(1);
	sizer4->Add(timegroup, 0, wxALIGN_CENTER_VERTICAL);
	sizer4->AddStretchSpacer(2);
	sizer4->Add(savegroup, 0, wxALIGN_CENTER_VERTICAL);
	sizer4->AddSpacer(FromDIP(5));

	//sizer
	wxBoxSizer* sizerv = new wxBoxSizer(wxVERTICAL);
	sizerv->Add(m_notebook, 1, wxEXPAND);
	sizerv->Add(sizer1, 0, wxEXPAND);
	sizerv->Add(5, 5);
	sizerv->Add(sizer2, 0, wxEXPAND);
	sizerv->Add(5, 5);
	sizerv->Add(sizer3, 0, wxEXPAND);
	sizerv->Add(sizer4, 0, wxEXPAND);
	SetSizer(sizerv);
	Layout();
	SetAutoLayout(true);
	SetScrollRate(10, 10);
	Thaw();
}

MoviePanel::~MoviePanel()
{
}

void MoviePanel::UpdateMovFps(double dval)
{
	auto str = wxString::Format("%.0f", dval);
	m_fps_text->ChangeValue(str);
}

void MoviePanel::UpdateMovLength(double dval)
{
	auto str = wxString::Format("%.2f", dval);
	m_movie_len_text->ChangeValue(str);
}

void MoviePanel::UpdateMovViewList(const MovViewListInfo& info)
{
	m_views_cmb->Clear();
	for (auto& view : info.views)
	{
		m_views_cmb->Append(view);
	}
}

void MoviePanel::UpdateMovViewIndex(int ival)
{
	m_views_cmb->Select(ival);
}

void MoviePanel::UpdateMovSliderStyle(bool bval)
{
	m_progress_sldr->SetMode(bval ? 1 : 0);
	if (bval)
		m_slider_btn->SetToolNormalBitmap(0, wxGetBitmap(jog));
	else
		m_slider_btn->SetToolNormalBitmap(0, wxGetBitmap(slider));
}

void MoviePanel::UpdateMovProgSlider(int cf, int ts, int sf, int ef)
{
	m_progress_sldr->SetScrollbar2(cf, ts, sf, ef, 1);
	m_progress_sldr->ChangeValue(cf);
}

void MoviePanel::UpdateBeginFrame(int ival)
{
	auto str = wxString::Format("%d", ival);
	m_start_frame_text->ChangeValue(str);
}

void MoviePanel::UpdateEndFrame(int ival)
{
	auto str = wxString::Format("%d", ival);
	m_end_frame_text->ChangeValue(str);
}

void MoviePanel::UpdateCurrentFrame(int ival)
{
	auto str = wxString::Format("%d", ival);
	m_cur_frame_text->ChangeValue(str);
	m_cur_frame_text->Update();
}

void MoviePanel::UpdateTotalFrames(int ival)
{
	auto str = wxString::Format("%d", ival);
	m_full_frame_text->ChangeValue(str);
}

void MoviePanel::UpdateMovCurTime(double dval)
{
	auto str = wxString::Format("%.2f", dval);
	m_progress_text->ChangeValue(str);
	m_progress_text->Update();
}

void MoviePanel::UpdateMovPlay(bool running, bool reverse, bool script)
{
	if (running)
	{
		if (reverse)
		{
			m_play_btn->SetBitmap(wxGetBitmap(play));
			m_play_btn->SetValue(false);

			m_play_inv_btn->SetBitmap(wxGetBitmap(pause));
			m_play_inv_btn->SetValue(true);
		}
		else
		{
			m_play_btn->SetBitmap(wxGetBitmap(pause));
			m_play_btn->SetValue(true);

			m_play_inv_btn->SetBitmap(wxGetBitmap(play_inv));
			m_play_inv_btn->SetValue(false);
		}
	}
	else
	{
		if (script)
		{
			m_play_btn->SetBitmap(wxGetBitmap(play_script));
			m_play_inv_btn->SetBitmap(wxGetBitmap(play_inv_script));
		}
		else
		{
			m_play_btn->SetBitmap(wxGetBitmap(play));
			m_play_inv_btn->SetBitmap(wxGetBitmap(play_inv));
		}
		m_play_btn->SetValue(false);

		m_play_inv_btn->SetValue(false);
	}
}

void MoviePanel::UpdateMovLoop(bool bval)
{
	m_loop_btn->SetValue(bval);
}

void MoviePanel::UpdateMovRotEnable(bool bval)
{
	m_rot_chk->SetValue(bval);
	m_x_rd->Enable(bval);
	m_y_rd->Enable(bval);
	m_z_rd->Enable(bval);
	m_degree_text->Enable(bval);
	m_rot_int_cmb->Enable(bval);
}

void MoviePanel::UpdateMovRotAxis(int ival)
{
	m_x_rd->SetValue(ival == 0);
	m_y_rd->SetValue(ival == 1);
	m_z_rd->SetValue(ival == 2);
}

void MoviePanel::UpdateMovRotAng(int ival)
{
	auto str = wxString::Format("%d", ival);
	m_degree_text->ChangeValue(str);
}

void MoviePanel::UpdateMovIntrpMode(int ival)
{
	m_rot_int_cmb->SetSelection(ival);
	m_interpolation_cmb->SetSelection(ival);
}

void MoviePanel::UpdateMovSeqMode(int ival)
{
	switch (ival)
	{
	case 0:
		m_seq_chk->SetValue(false);
		m_bat_chk->SetValue(false);
		break;
	case 1:
		m_seq_chk->SetValue(true);
		m_bat_chk->SetValue(false);
		break;
	case 2:
		m_seq_chk->SetValue(false);
		m_bat_chk->SetValue(true);
		break;
	}
	m_seq_dec_btn->Enable(ival > 0);
	m_seq_inc_btn->Enable(ival > 0);
	m_seq_num_text->Enable(ival > 0);
}

void MoviePanel::UpdateMovSeqNum(int scn, int san)
{
	auto str = wxString::Format("%d", scn);
	m_seq_num_text->ChangeValue(str);
	str = wxString::Format("%d", san);
	m_seq_total_text->ChangeValue(str);
}

void MoviePanel::UpdateCaptureParam(bool bval)
{
	m_keyframe_chk->SetValue(bval);
}

void MoviePanel::UpdateParamKeyDuration(double dval)
{
	auto str = wxString::Format("%.0f", dval);
	m_duration_text->ChangeValue(str);
}

void MoviePanel::UpdateParamList()
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateDataToUI({ gstParamList });
}

void MoviePanel::UpdateParamList(const std::vector<KeyframeInfo>& list)
{
	m_keylist->SetKeyframes(list);
}

void MoviePanel::UpdateParamListSelect(int ival)
{
	long item = m_keylist->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	if (ival != item && ival != -1)
		m_keylist->SelectItemSilently(ival);
}

void MoviePanel::UpdateCamLockObjEnable(bool bval)
{
	m_cam_lock_chk->SetValue(bval);
}

void MoviePanel::UpdateCamLockType(int ival)
{
	m_cam_lock_cmb->SetSelection(ival);
}

void MoviePanel::UpdatePresetList(const std::vector<PresetInfo>& list)
{
	m_auto_key_list->DeleteAllItems();

	long row = 0;
	for (const auto& preset : list)
	{
		long item = m_auto_key_list->InsertItem(
			row,
			preset.id);

		m_auto_key_list->SetItem(
			item,
			1,
			preset.name);

		++row;
	}
	m_auto_key_list->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
	m_auto_key_list->SetColumnWidth(1, wxLIST_AUTOSIZE);
}

void MoviePanel::UpdateCropEnable(bool bval)
{
	m_crop_chk->SetValue(bval);
	m_crop_x_text->Enable(bval);
	m_crop_y_text->Enable(bval);
	m_crop_w_text->Enable(bval);
	m_crop_h_text->Enable(bval);

	m_sb_tl_rb->Enable(bval);
	m_sb_tr_rb->Enable(bval);
	m_sb_bl_rb->Enable(bval);
	m_sb_br_rb->Enable(bval);

	m_sb_dx_text->Enable(bval);
	m_sb_dx_spin->Enable(bval);
	m_sb_dy_text->Enable(bval);
	m_sb_dy_spin->Enable(bval);
}

void MoviePanel::UpdateCropValues(int x, int y, int w, int h)
{
	m_crop_x_text->ChangeValue(wxString::Format("%d", x));
	m_crop_y_text->ChangeValue(wxString::Format("%d", y));
	m_crop_w_text->ChangeValue(wxString::Format("%d", w));
	m_crop_h_text->ChangeValue(wxString::Format("%d", h));
}

void MoviePanel::UpdateScalebarPos(int ival, int x, int y)
{
	switch (ival)
	{
	case 0:
		m_sb_tl_rb->SetValue(true);
		break;
	case 1:
		m_sb_tr_rb->SetValue(true);
		break;
	case 2:
		m_sb_bl_rb->SetValue(true);
		break;
	case 3:
	default:
		m_sb_br_rb->SetValue(true);
		break;
	}
	m_sb_dx_text->ChangeValue(wxString::Format("%d", x));
	m_sb_dy_text->ChangeValue(wxString::Format("%d", y));
}

void MoviePanel::UpdateRunScript(bool bval)
{
	m_run_script_chk->SetValue(bval);
	size_t idx = 4;
	for (size_t i = 0; i < m_notebook->GetPageCount(); ++i)
	{
		wxString str = m_notebook->GetPageText(i);
		if (str.Contains(UITEXT_NBPG4_0))
		{
			idx = i;
			break;
		}
	}
	if (bval)
		m_notebook->SetPageText(idx, UITEXT_NBPG4_1);
	else
		m_notebook->SetPageText(idx, UITEXT_NBPG4_0);
}

void MoviePanel::UpdateScriptFile(const std::wstring& filename)
{
	m_script_file_text->ChangeValue(filename);
}

void MoviePanel::UpdateScriptList(const std::vector<ScriptInfo>& list)
{
	m_script_list->DeleteAllItems();

	long row = 0;
	for (const auto& script : list)
	{
		long item = m_script_list->InsertItem(
			row,
			script.id);

		m_script_list->SetItem(
			item,
			1,
			script.filename);

		++row;
	}
	m_script_list->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
	m_script_list->SetColumnWidth(1, wxLIST_AUTOSIZE);
}

void MoviePanel::UpdateScriptListSelect(int ival)
{
	if (ival >= 0 && ival < m_script_list->GetItemCount())
	{
		m_script_list->SetItemState(ival,
			wxLIST_STATE_SELECTED,
			wxLIST_STATE_SELECTED);
		//wxSize ss = m_script_list->GetItemSpacing();
		//m_script_list->ScrollList(0, ss.y*ival);
	}
}

void MoviePanel::SelectKeyframe(int id)
{

}

void MoviePanel::DeleteKeyframe(int id)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->DeleteKeyframe(id);
}

void MoviePanel::DeleteAllKeyframes()
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->DeleteAllKeyframes();
}

void MoviePanel::SetKeyframeTime(int id, double time)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->SetKeyframeTime(id, time);
}

void MoviePanel::SetKeyframeDuration(int id, double duration)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->SetKeyframeDuration(id, duration);
}

void MoviePanel::SetKeyframeInterpolation(int id, int type)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->SetKeyframeInterpolation(id, type);
}

void MoviePanel::SetKeyframeDescription(int id, const std::wstring& description)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->SetKeyframeDescription(id, description);
}

void MoviePanel::MoveKeyframe(int sourceId, int targetId, bool before)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->MoveKeyframe(sourceId, targetId, before);
}

double MoviePanel::GetFps()
{
	wxString str = m_fps_text->GetValue();
	double val;
	if (str.ToDouble(&val))
		return val;
	return 1.0;
}

double MoviePanel::GetMovieLength()
{
	wxString str = m_movie_len_text->GetValue();
	double val;
	if (str.ToDouble(&val))
		return val;
	return 1.0;
}

int MoviePanel::GetViewIndex()
{
	return m_views_cmb->GetCurrentSelection();
}

int MoviePanel::GetProgressScroll()
{
	return m_progress_sldr->GetValue();
}

int MoviePanel::GetStartFrame()
{
	wxString str = m_start_frame_text->GetValue();
	long lval;
	if (str.ToLong(&lval))
		return lval;
	return 0;
}

int MoviePanel::GetEndFrame()
{
	wxString str = m_end_frame_text->GetValue();
	long lval;
	if (str.ToLong(&lval))
		return lval;
	return 0;
}

int MoviePanel::GetCurrentFrame()
{
	wxString str = m_cur_frame_text->GetValue();
	long lval;
	if (str.ToLong(&lval))
		return lval;
	return 0;
}

double MoviePanel::GetCurTime()
{
	wxString str = m_progress_text->GetValue();
	double dval;
	if (str.ToDouble(&dval))
		return dval;
	return 0.0;
}

int MoviePanel::GetFullFrame()
{
	wxString str = m_full_frame_text->GetValue();
	long lval;
	if (str.ToLong(&lval))
		return lval;
	return 1;
}

bool MoviePanel::GetLoop()
{
	return m_loop_btn->GetValue();
}

bool MoviePanel::GetRotateEnable()
{
	return m_rot_chk->GetValue();
}

int MoviePanel::GetRotateAxis()
{
	int val = 0;
	if (m_x_rd->GetValue())
		val = 0;
	else if (m_y_rd->GetValue())
		val = 1;
	else if (m_z_rd->GetValue())
		val = 2;
	return val;
}

int MoviePanel::GetRotateDeg()
{
	wxString str = m_degree_text->GetValue();
	long ival = 0;
	if (str.ToLong(&ival))
		return ival;
	return 0;
}

int MoviePanel::GetRotateInterp()
{
	return m_rot_int_cmb->GetCurrentSelection();
}

int MoviePanel::GetSeqMode()
{
	if (m_bat_chk->GetValue())
		return 2;
	if (m_seq_chk->GetValue())
		return 1;
	return 0;
}

int MoviePanel::GetSeqNum()
{
	wxString str = m_seq_num_text->GetValue();
	long val;
	if (str.ToLong(&val))
		return val;
	return 0;
}

int MoviePanel::GetKeyframeNum()
{
	long item = m_keylist->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	if (item == -1)
		return 0;
	wxString str = m_keylist->GetItemText(item);
	long id;
	if (str.ToLong(&id))
		return id;
	return 0;
}

bool MoviePanel::GetKeyframeEnable()
{
	return m_keyframe_chk->GetValue();
}

double MoviePanel::GetKeyDuration()
{
	wxString str = m_duration_text->GetValue();
	double val;
	if (str.ToDouble(&val))
		return val;
	return 1.0;
}

int MoviePanel::GetKeyInterpolation()
{
	return m_interpolation_cmb->GetSelection();
}

bool MoviePanel::GetCameraLock()
{
	return m_cam_lock_chk->GetValue();
}

int MoviePanel::GetCameraLockType()
{
	return m_cam_lock_cmb->GetSelection() + 1;
}

int MoviePanel::GetPresetNum()
{
	long item = m_auto_key_list->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);

	if (item != -1)
		return item;
	return 0;
}

bool MoviePanel::GetCropEnable()
{
	return m_crop_chk->GetValue();
}

CropInfo MoviePanel::GetCropValues()
{
	wxString temp;
	long x = 0, y = 0, w = 0, h = 0;
	temp = m_crop_x_text->GetValue();
	temp.ToLong(&x);
	temp = m_crop_y_text->GetValue();
	temp.ToLong(&y);
	temp = m_crop_w_text->GetValue();
	temp.ToLong(&w);
	temp = m_crop_h_text->GetValue();
	temp.ToLong(&h);

	return CropInfo(x, y, w, h);
}

int MoviePanel::GetScalebarPos()
{
	int pos = 3;
	if (m_sb_tl_rb->GetValue())
		pos = 0;
	else if (m_sb_tr_rb->GetValue())
		pos = 1;
	else if (m_sb_bl_rb->GetValue())
		pos = 2;
	else if (m_sb_br_rb->GetValue())
		pos = 3;
	return pos;
}

ScalebarOffset MoviePanel::GetScalebarOffset()
{
	wxString temp;
	long x = 0, y = 0;
	temp = m_sb_dx_text->GetValue();
	temp.ToLong(&x);
	temp = m_sb_dy_text->GetValue();
	temp.ToLong(&y);

	return ScalebarOffset(x, y);
}

bool MoviePanel::GetScriptEnable()
{
	return m_run_script_chk->GetValue();
}

std::wstring MoviePanel::GetScriptFileName()
{
	return m_script_file_text->GetValue().ToStdWstring();
}

std::string MoviePanel::GetSelScriptName()
{
	long item = m_script_list->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);

	std::string str;
	if (item != -1)
	{
		str = m_script_list->GetItemText(item, 1).ToStdString();
	}
	return str;
}

void MoviePanel::OnNotebookPage(wxAuiNotebookEvent& event)
{
	event.Skip();
}

void MoviePanel::OnFpsEdit(wxCommandEvent& event)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstMovFps });
}

void MoviePanel::OnMovieLenText(wxCommandEvent& event)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstMovLength });
}

void MoviePanel::OnViewSelected(wxCommandEvent& event)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstMovViewIndex });
}

void MoviePanel::OnSliderStyle(wxCommandEvent& event)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstMovSliderStyle });
}

void MoviePanel::OnProgressScroll(wxScrollEvent& event)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstMovProgSlider });
}

void MoviePanel::OnStartFrameText(wxCommandEvent& event)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstBeginFrame });
}

void MoviePanel::OnEndFrameText(wxCommandEvent& event)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstEndFrame });
}

void MoviePanel::OnCurFrameText(wxCommandEvent& event)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstCurrentFrame });
}

void MoviePanel::OnCurTimeText(wxCommandEvent& event)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstMovCurTime });
}

void MoviePanel::OnFullFrameText(wxCommandEvent& event)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstTotalFrames });
}

void MoviePanel::OnPlay(wxCommandEvent& event)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstMovPlay });
}

void MoviePanel::OnPlayInv(wxCommandEvent& event)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstMovPlayInv });
}

void MoviePanel::OnRewind(wxCommandEvent& event)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstMovRewind });
}

void MoviePanel::OnForward(wxCommandEvent& event)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstMovForward });
}

void MoviePanel::OnLoop(wxCommandEvent& event)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstMovLoop });
}

void MoviePanel::OnStartFrameBtn(wxCommandEvent& event)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateDataToUI({ gstBeginFrame });
}

void MoviePanel::OnEndFrameBtn(wxCommandEvent& event)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateDataToUI({ gstEndFrame });
}

void MoviePanel::OnIncFrame(wxCommandEvent& event)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstMovIncFrame });
}

void MoviePanel::OnDecFrame(wxCommandEvent& event)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstMovDecFrame });
}

void MoviePanel::OnSave(wxCommandEvent& event)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstMovSave });
}

void MoviePanel::OnRotateChecked(wxCommandEvent& event)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstMovRotEnable });
}

void MoviePanel::OnRotAxis(wxCommandEvent& event)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstMovRotAxis });
}

void MoviePanel::OnDegreeText(wxCommandEvent& event)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstMovRotAng });
}

void MoviePanel::OnRotIntCmb(wxCommandEvent& event)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstMovIntrpMode });
}

void MoviePanel::OnSequenceChecked(wxCommandEvent& event)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstMovSeqMode });
}

void MoviePanel::OnBatchChecked(wxCommandEvent& event)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstMovSeqMode });
}

void MoviePanel::OnSeqDecBtn(wxCommandEvent& event)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstMovSeqDec });
}

void MoviePanel::OnSeqIncBtn(wxCommandEvent& event)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstMovSeqInc });
}

void MoviePanel::OnSeqNumText(wxCommandEvent& event)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstMovSeqNum });
}

void MoviePanel::OnAct(wxListEvent& event)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstMovKeyframeNum });
}

void MoviePanel::OnKeyframeChk(wxCommandEvent& event)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstCaptureParam });
}

void MoviePanel::OnDurationText(wxCommandEvent& event)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstKeyDuration });
}

void MoviePanel::OnInterpolation(wxCommandEvent& event)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstKeyInterpolation });
}

void MoviePanel::OnInsKey(wxCommandEvent& event)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstMovInsertKey });
}

void MoviePanel::OnDelKey(wxCommandEvent& event)
{
	DeleteKeyframe(GetKeyframeNum());
}

void MoviePanel::OnDelAll(wxCommandEvent& event)
{
	DeleteAllKeyframes();
}

void MoviePanel::OnCamLockChk(wxCommandEvent& event)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstCamLockObjEnable });
}

void MoviePanel::OnCamLockCmb(wxCommandEvent& event)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstCamLockType });
}

void MoviePanel::OnCamLockBtn(wxCommandEvent& event)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstCamLockCtr });
}

//auto key
void MoviePanel::OnGenKey(wxCommandEvent& event)
{
	m_notebook->SetSelection(1);
}

void MoviePanel::OnCropCheck(wxCommandEvent& event)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstCropEnable });
}

void MoviePanel::OnResetCrop(wxCommandEvent& event)
{
	m_crop_chk->SetValue(true);
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstCropEnable });
}

void MoviePanel::OnEditCrop(wxCommandEvent& event)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstCropValues });
}

void MoviePanel::OnCropSpinUp(wxSpinEvent& event)
{
	wxObject* obj = event.GetEventObject();
	wxTextCtrl* text_ctrl = 0;
	if (obj == m_crop_x_spin)
		text_ctrl = m_crop_x_text;
	if (obj == m_crop_y_spin)
		text_ctrl = m_crop_y_text;
	if (obj == m_crop_w_spin)
		text_ctrl = m_crop_w_text;
	if (obj == m_crop_h_spin)
		text_ctrl = m_crop_h_text;

	if (text_ctrl)
	{
		wxString str = text_ctrl->GetValue();
		long ival;
		if (str.ToLong(&ival))
			text_ctrl->SetValue(wxString::Format(
				"%d", ival + 1));
	}
}

void MoviePanel::OnCropSpinDown(wxSpinEvent& event)
{
	wxObject* obj = event.GetEventObject();
	wxTextCtrl* text_ctrl = 0;
	if (obj == m_crop_x_spin)
		text_ctrl = m_crop_x_text;
	if (obj == m_crop_y_spin)
		text_ctrl = m_crop_y_text;
	if (obj == m_crop_w_spin)
		text_ctrl = m_crop_w_text;
	if (obj == m_crop_h_spin)
		text_ctrl = m_crop_h_text;

	if (text_ctrl)
	{
		wxString str = text_ctrl->GetValue();
		long ival;
		if (str.ToLong(&ival))
			text_ctrl->SetValue(wxString::Format(
				"%d", ival - 1));
	}
}

void MoviePanel::OnSbRadio(wxCommandEvent& event)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstScalebarPos });
}

void MoviePanel::OnSbEdit(wxCommandEvent& event)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstScalbarOffset });
}

void MoviePanel::OnSbSpinUp(wxSpinEvent& event)
{
	wxObject* obj = event.GetEventObject();
	wxTextCtrl* text_ctrl = 0;
	if (obj == m_sb_dx_spin)
		text_ctrl = m_sb_dx_text;
	if (obj == m_sb_dy_spin)
		text_ctrl = m_sb_dy_text;

	if (text_ctrl)
	{
		wxString str = text_ctrl->GetValue();
		long ival;
		if (str.ToLong(&ival))
			text_ctrl->SetValue(wxString::Format(
				"%d", ival + 1));
	}
}

void MoviePanel::OnSbSpinDown(wxSpinEvent& event)
{
	wxObject* obj = event.GetEventObject();
	wxTextCtrl* text_ctrl = 0;
	if (obj == m_sb_dx_spin)
		text_ctrl = m_sb_dx_text;
	if (obj == m_sb_dy_spin)
		text_ctrl = m_sb_dy_text;

	if (text_ctrl)
	{
		wxString str = text_ctrl->GetValue();
		long ival;
		if (str.ToLong(&ival))
			text_ctrl->SetValue(wxString::Format(
				"%d", ival - 1));
	}
}

//script
void MoviePanel::OnRunScriptChk(wxCommandEvent& event)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstEnableScript });
}

void MoviePanel::OnScriptFileEdit(wxCommandEvent& event)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstScriptFile });
}

void MoviePanel::OnScriptClearBtn(wxCommandEvent& event)
{
	m_script_file_text->Clear();
	m_run_script_chk->SetValue(false);
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstEnableScript });
}

void MoviePanel::OnScriptFileBtn(wxCommandEvent& event)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstLoadScriptFile });
}

void MoviePanel::OnScriptListSelected(wxListEvent& event)
{
	auto agent = m_agent->As<MoviePanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstSelectScriptFile });
}

SaveMovieHook::SaveMovieHook(const SaveMovieOptions& options) :
	m_options(options)
{

}

void SaveMovieHook::AddCustomControls(
	wxFileDialogCustomize& customizer)
{
	customizer.AddStaticText(
		"Additional Options");

	customizer.AddStaticText(
		"TIFF sequence is preferable for very short movies.");

	if (m_options.project_save)
	{
		m_embed_chk =
			customizer.AddCheckBox(
				"Embed all files in the project folder");

		m_embed_chk->SetValue(
			m_options.embed_project_files);
	}

	customizer.AddStaticText("Output Resolution");

	m_dpi_txt =
		customizer.AddTextCtrl(
			wxString::Format("%d", m_options.dpi));

	m_enlarge_chk =
		customizer.AddCheckBox(
			"Enlarge output image");

	m_enlarge_chk->SetValue(
		m_options.enlarge_output);

	m_enlarge_txt =
		customizer.AddTextCtrl(
			wxString::Format("%.1f",
				m_options.enlarge_scale));

	customizer.AddStaticText(
		"Image Sequence Options");

	m_compress_chk =
		customizer.AddCheckBox(
			"Compress to save space");
	m_compress_chk->SetValue(
		m_options.compress);

	m_alpha_chk =
		customizer.AddCheckBox(
			"Save alpha");
	m_alpha_chk->SetValue(
		m_options.save_alpha);

	m_float_chk =
		customizer.AddCheckBox(
			"Save float channel");
	m_float_chk->SetValue(
		m_options.save_float);

	customizer.AddStaticText(
		"Movie Options");

	m_bitrate_txt =
		customizer.AddTextCtrl(
			wxString::Format("%.1f",
				m_options.bitrate));
}

void SaveMovieHook::TransferDataFromCustomControls()
{
	if (m_embed_chk)
	{
		m_options.embed_project_files =
			m_embed_chk->GetValue();
	}

	if (m_dpi_txt)
	{
		long value = 72;

		if (m_dpi_txt->GetValue().ToLong(&value))
			m_options.dpi = static_cast<int>(value);
	}

	if (m_enlarge_chk)
	{
		m_options.enlarge_output =
			m_enlarge_chk->GetValue();
	}

	if (m_enlarge_txt)
	{
		double value = 1.0;

		if (m_enlarge_txt->GetValue().ToDouble(&value))
			m_options.enlarge_scale = value;
	}

	if (m_compress_chk)
		m_options.compress =
		m_compress_chk->GetValue();

	if (m_alpha_chk)
		m_options.save_alpha =
		m_alpha_chk->GetValue();

	if (m_float_chk)
		m_options.save_float =
		m_float_chk->GetValue();

	if (m_bitrate_txt)
	{
		double value = 20.0;

		if (m_bitrate_txt->GetValue().ToDouble(&value))
			m_options.bitrate = value;
	}

	m_options.estimated_size_mb =
		m_options.bitrate *
		m_options.movie_length_sec /
		8.0;
}

