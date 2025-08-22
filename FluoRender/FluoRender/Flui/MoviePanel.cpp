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
#include <MoviePanel.h>
#include <Global.h>
#include <GlobalStates.h>
#include <Names.h>
#include <MainSettings.h>
#include <MovieDefault.h>
#include <MainFrame.h>
#include <RenderView.h>
#include <RenderViewPanel.h>
#include <Interpolator.h>
#include <MovieMaker.h>
#include <wxUndoableScrollBar.h>
#include <wxUndoableToolbar.h>
#include <ModalDlg.h>
#include <wx/aboutdlg.h>
#include <wx/valnum.h>
#include <png_resource.h>
#include <icons.h>
#include <key.xpm>
#include <Debug.h>

KeyListCtrl::KeyListCtrl(
	wxWindow* parent,
	MainFrame* frame,
	const wxPoint& pos,
	const wxSize& size,
	long style) :
	wxListCtrl(parent, wxID_ANY, pos, size, style),
	m_frame(frame),
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
	long item = GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	if (item == -1)
		return;
	wxString str = GetItemText(item);
	long id;
	str.ToLong(&id);

	glbin_interpolator.RemoveKey(id);
	Update();
}

void KeyListCtrl::DeleteAll()
{
	glbin_interpolator.Clear();
	Update();
}

void KeyListCtrl::Update()
{
	m_frame_text->Hide();
	m_duration_text->Hide();
	m_interpolation_cmb->Hide();
	m_description_text->Hide();
	m_editing_item = -1;

	DeleteAllItems();
	for (int i = 0; i < glbin_interpolator.GetKeyNum(); i++)
	{
		int id = glbin_interpolator.GetKeyID(i);
		int time = glbin_interpolator.GetKeyTime(i);
		int duration = glbin_interpolator.GetKeyDuration(i);
		int interp = glbin_interpolator.GetKeyType(i);
		std::wstring desc = glbin_interpolator.GetKeyDesc(i);
		Append(id, time, duration, interp, desc);
	}

	for (int i = 0; i < 4; ++i)
		SetColumnWidth(i, wxLIST_AUTOSIZE_USEHEADER);
	SetColumnWidth(4, wxLIST_AUTOSIZE);
}

void KeyListCtrl::UpdateText()
{
	wxString str;

	for (int i = 0; i < glbin_interpolator.GetKeyNum(); i++)
	{
		int id = glbin_interpolator.GetKeyID(i);
		int time = glbin_interpolator.GetKeyTime(i);
		int duration = glbin_interpolator.GetKeyDuration(i);
		int interp = glbin_interpolator.GetKeyType(i);
		std::wstring desc = glbin_interpolator.GetKeyDesc(i);

		wxString wx_id = wxString::Format("%d", id);
		wxString wx_time = wxString::Format("%d", time);
		wxString wx_duration = wxString::Format("%d", duration);
		SetText(i, 0, wx_id);
		SetText(i, 1, wx_time);
		SetText(i, 2, wx_duration);
		str = interp == 0 ? "Linear" : "Smooth";
		SetText(i, 3, str);
		str = desc;
		SetText(i, 4, str);
	}
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

void KeyListCtrl::EndEdit(bool update)
{
	if (m_duration_text->IsShown())
	{
		m_frame_text->Hide();
		m_duration_text->Hide();
		m_interpolation_cmb->Hide();
		m_description_text->Hide();
		m_editing_item = -1;
		if (update) UpdateText();
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

	wxString str = GetItemText(m_editing_item);
	long id;
	str.ToLong(&id);

	int index = glbin_interpolator.GetKeyIndex(int(id));
	str = m_frame_text->GetValue();
	double time;
	if (str.ToDouble(&time))
	{
		glbin_interpolator.ChangeTime(index, time);
	}
}

void KeyListCtrl::OnDurationText(wxCommandEvent& event)
{
	if (m_editing_item == -1)
		return;

	wxString str = GetItemText(m_editing_item);
	long id;
	str.ToLong(&id);

	int index = glbin_interpolator.GetKeyIndex(int(id));
	str = m_duration_text->GetValue();
	double duration;
	if (str.ToDouble(&duration))
	{
		glbin_interpolator.ChangeDuration(index, duration);
		SetText(m_editing_item, 2, str);
	}
}

void KeyListCtrl::OnInterpoCmb(wxCommandEvent& event)
{
	if (m_editing_item == -1)
		return;

	wxString str = GetItemText(m_editing_item);
	long id;
	str.ToLong(&id);

	int index = glbin_interpolator.GetKeyIndex(int(id));
	FlKeyGroup* keygroup = glbin_interpolator.GetKeyGroup(index);
	if (keygroup)
	{
		int sel = m_interpolation_cmb->GetSelection();
		keygroup->type = sel;
		str = sel == 0 ? "Linear" : "Smooth";
		SetText(m_editing_item, 3, str);
	}
}

void KeyListCtrl::OnDescritionText(wxCommandEvent& event)
{
	if (m_editing_item == -1)
		return;

	wxString str = GetItemText(m_editing_item);
	long id;
	str.ToLong(&id);

	int index = glbin_interpolator.GetKeyIndex(int(id));
	FlKeyGroup* keygroup = glbin_interpolator.GetKeyGroup(index);
	if (keygroup)
	{
		str = m_description_text->GetValue();
		keygroup->desc = str.ToStdWstring();
		SetText(m_editing_item, 4, str);
	}
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
	long index = HitTest(pos, flags, NULL); // got to use it at last
	if (index >= 0 && index != m_editing_item && index != m_dragging_to_item)
	{
		m_dragging_to_item = index;

		//change the content in the interpolator
		if (m_editing_item > m_dragging_to_item)
			glbin_interpolator.MoveKeyBefore(m_editing_item, m_dragging_to_item);
		else
			glbin_interpolator.MoveKeyAfter(m_editing_item, m_dragging_to_item);

		DeleteItem(m_editing_item);
		InsertItem(m_dragging_to_item, "", 0);
		UpdateText();

		m_editing_item = m_dragging_to_item;
		SelectItemSilently(m_editing_item);
	}
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
wxWindow* MoviePanel::CreateSimplePage(wxWindow *parent)
{
	wxScrolledWindow *page = new wxScrolledWindow(parent);

	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;

	wxStaticText *st = 0, *st2 = 0;
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
	wxSize bs = FromDIP(wxSize(26, 26));
	wxBoxSizer* sizer7 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, wxID_ANY, "T: ");
	m_seq_dec_btn = new wxButton(page, wxID_ANY, "",
		wxDefaultPosition, bs);
	m_seq_dec_btn->SetBitmap(wxGetBitmap(minus));
	m_seq_dec_btn->Bind(wxEVT_BUTTON, &MoviePanel::OnSeqDecBtn, this);
	m_seq_dec_btn->SetToolTip("Decrease the time point number by 1");
	m_seq_num_text = new wxTextCtrl(page, wxID_ANY, "0",
		wxDefaultPosition, FromDIP(wxSize(40, -1)), wxTE_RIGHT, vald_int);
	m_seq_num_text->Bind(wxEVT_TEXT, &MoviePanel::OnSeqNumText, this);
	m_seq_inc_btn = new wxButton(page, wxID_ANY, "",
		wxDefaultPosition, bs);
	m_seq_inc_btn->SetBitmap(wxGetBitmap(plus));
	m_seq_inc_btn->Bind(wxEVT_BUTTON, &MoviePanel::OnSeqIncBtn, this);
	m_seq_inc_btn->SetToolTip("Increase the time point number by 1");
	st2 = new wxStaticText(page, wxID_ANY, "of: ");
	m_seq_total_text = new wxTextCtrl(page, wxID_ANY, "0",
		wxDefaultPosition, FromDIP(wxSize(40, -1)), wxTE_RIGHT|wxTE_READONLY);
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

wxWindow* MoviePanel::CreateKeyframePage(wxWindow *parent)
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
	m_keylist = new KeyListCtrl(page, m_frame,
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

wxWindow* MoviePanel::CreateTemplatePage(wxWindow *parent)
{
	wxScrolledWindow* page = new wxScrolledWindow(parent);

	wxStaticText * st = new wxStaticText(page, 0, "Double-click a preset to generate keyframes");

	//list of options
	m_auto_key_list = new wxListCtrl(page, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
	wxListItem itemCol;
	itemCol.SetText("No.");
	m_auto_key_list->InsertColumn(0, itemCol);
	itemCol.SetText("Keyframe Preset");
	m_auto_key_list->InsertColumn(1, itemCol);
	//options
	//channel comb 1
	long tmp;
	std::vector<std::string> str_list = glbin_moviemaker.GetAutoKeyTypes();
	int i = 0;
	wxString str;
	for (auto& it : str_list)
	{
		i++;
		str = wxString::Format("%d", i);
		tmp = m_auto_key_list->InsertItem(i-1, str, 0);
		m_auto_key_list->SetItem(tmp, 1, it);
	}
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

wxWindow* MoviePanel::CreateCropPage(wxWindow *parent)
{
	wxScrolledWindow* page = new wxScrolledWindow(parent);

	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;
	wxStaticText *st = 0;

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

wxWindow* MoviePanel::CreateScriptPage(wxWindow *parent)
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
	wxBoxSizer *sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Script File:",
		wxDefaultPosition, FromDIP(wxSize(80, -1)));
	m_script_file_btn = new wxButton(page, wxID_ANY, "Browse...",
		wxDefaultPosition, FromDIP(wxSize(80, -1)));
	m_script_file_btn->Bind(wxEVT_BUTTON, &MoviePanel::OnScriptFileBtn, this);
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	sizer_1->AddStretchSpacer(1);
	sizer_1->Add(m_script_file_btn, 0, wxALIGN_CENTER);

	//file name
	wxBoxSizer *sizer_2 = new wxBoxSizer(wxHORIZONTAL);
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

MoviePanel::MoviePanel(MainFrame* frame,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name) :
	TabbedPanel(frame, frame, pos, size, style, name)
{
	m_running = true;
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);
	Freeze();
	//SetDoubleBuffered(true);
	wxIntegerValidator<unsigned int> vald_int;

	Root* root = glbin_data_manager.GetRoot();
	if (root)
		m_view = root->GetView(glbin_mov_def.m_view_idx).get();

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

	wxStaticText* st = 0, *st2 = 0;
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
	m_progress_sldr->SetScrollbar2(
		glbin_moviemaker.GetCurrentFrame(),
		glbin_moviemaker.GetScrollThumbSize(),
		glbin_moviemaker.GetClipStartFrame(),
		glbin_moviemaker.GetClipEndFrame(), 1);
	m_progress_sldr->Bind(wxEVT_SCROLL_CHANGED, &MoviePanel::OnProgressScroll, this);
	sizer2->Add(5, 5);
	sizer2->Add(m_slider_btn, 0, wxALIGN_CENTER);
	sizer2->Add(5, 5);
	sizer2->Add(m_progress_sldr, 1, wxALIGN_CENTER);
	sizer2->Add(5, 5);

	//controls
	wxSize bs = FromDIP(wxSize(26, 26));
	wxSize ts = FromDIP(wxSize(50, -1));
	wxFont f;
	wxBoxSizer* sizer3 = new wxBoxSizer(wxHORIZONTAL);
	m_start_btn = new wxButton(this, wxID_ANY, "",
		wxDefaultPosition, bs);
	m_start_btn->SetBitmap(wxGetBitmap(start));
	m_start_btn->Bind(wxEVT_BUTTON, &MoviePanel::OnStartFrameBtn, this);
	m_start_btn->SetToolTip("Set the start frame number of a clip to current frame");
	m_start_frame_text = new wxTextCtrl(this, wxID_ANY, "1",
		wxDefaultPosition, FromDIP(wxSize(39, -1)), wxTE_RIGHT, vald_int);
	f = m_start_frame_text->GetFont();
	f.MakeLarger();
	m_start_frame_text->SetFont(f);
	m_start_frame_text->Bind(wxEVT_TEXT, &MoviePanel::OnStartFrameText, this);
	m_start_frame_text->SetToolTip("Set the start frame number of a clip");
	m_end_frame_text = new wxTextCtrl(this, wxID_ANY, "10",
		wxDefaultPosition, FromDIP(wxSize(39, -1)), wxTE_RIGHT, vald_int);
	m_end_frame_text->SetFont(f);
	m_end_frame_text->Bind(wxEVT_TEXT, &MoviePanel::OnEndFrameText, this);
	m_end_frame_text->SetToolTip("Set the end frame number of a clip");
	m_end_btn = new wxButton(this, wxID_ANY, "",
		wxDefaultPosition, bs);
	m_end_btn->SetBitmap(wxGetBitmap(end));
	m_end_btn->Bind(wxEVT_BUTTON, &MoviePanel::OnEndFrameBtn, this);
	m_end_btn->SetToolTip("Set the end frame number of a clip to current frame");
	m_dec_time_btn = new wxButton(this, wxID_ANY, "",
		wxDefaultPosition, bs);
	m_dec_time_btn->SetBitmap(wxGetBitmap(step_back));
	m_dec_time_btn->Bind(wxEVT_BUTTON, &MoviePanel::OnDecFrame, this);
	m_dec_time_btn->SetToolTip("Step back one frame");
	m_cur_frame_text = new wxTextCtrl(this, wxID_ANY, "0",
		wxDefaultPosition, ts, wxTE_RIGHT, vald_int);
	m_cur_frame_text->SetFont(f);
	m_cur_frame_text->Bind(wxEVT_TEXT, &MoviePanel::OnCurFrameText, this);
	m_cur_frame_text->SetToolTip("Set current frame number");
	m_inc_time_btn = new wxButton(this, wxID_ANY, "",
		wxDefaultPosition, bs);
	m_inc_time_btn->SetBitmap(wxGetBitmap(step_forward));
	m_inc_time_btn->Bind(wxEVT_BUTTON, &MoviePanel::OnIncFrame, this);
	m_inc_time_btn->SetToolTip("Step forward one frame");
	m_full_frame_text = new wxTextCtrl(this, wxID_ANY, "0",
		wxDefaultPosition, ts, wxTE_RIGHT, vald_int);
	m_full_frame_text->SetFont(f);
	m_full_frame_text->Bind(wxEVT_TEXT, &MoviePanel::OnFullFrameText, this);
	m_full_frame_text->SetToolTip("Set the end frame number of the entire movie");
	sizer3->AddStretchSpacer(2);
	sizer3->Add(m_start_btn, 0, wxALIGN_CENTER);
	sizer3->Add(m_start_frame_text, 0, wxALIGN_CENTER);
	sizer3->Add(m_end_frame_text, 0, wxALIGN_CENTER);
	sizer3->Add(m_end_btn, 0, wxALIGN_CENTER);
	sizer3->AddStretchSpacer(1);
	sizer3->Add(m_dec_time_btn, 0, wxALIGN_CENTER);
	sizer3->Add(m_cur_frame_text, 0, wxALIGN_CENTER);
	sizer3->Add(m_inc_time_btn, 0, wxALIGN_CENTER);
	sizer3->AddStretchSpacer(2);
	sizer3->Add(m_full_frame_text, 0, wxALIGN_CENTER);
	sizer3->Add(5, 5);

	wxBoxSizer* sizer4 = new wxBoxSizer(wxHORIZONTAL);
	m_rewind_btn = new wxButton(this, wxID_ANY, "",
		wxDefaultPosition, bs);
	m_rewind_btn->SetBitmap(wxGetBitmap(rewind));
	m_rewind_btn->Bind(wxEVT_BUTTON, &MoviePanel::OnRewind, this);
	m_rewind_btn->SetToolTip("Rewind to the start frame of a clip");
	m_play_inv_btn = new wxToggleButton(this, wxID_ANY, "",
		wxDefaultPosition, bs);
	m_play_inv_btn->SetBitmap(wxGetBitmap(play_inv));
	m_play_inv_btn->Bind(wxEVT_TOGGLEBUTTON, &MoviePanel::OnPlayInv, this);
	m_play_inv_btn->SetToolTip("Play a clip backward");
	m_play_btn = new wxToggleButton(this, wxID_ANY, "",
		wxDefaultPosition, bs);
	m_play_btn->SetBitmap(wxGetBitmap(play));
	m_play_btn->Bind(wxEVT_TOGGLEBUTTON, &MoviePanel::OnPlay, this);
	m_play_btn->SetToolTip("Play a clip");
	m_forward_btn = new wxButton(this, wxID_ANY, "",
		wxDefaultPosition, bs);
	m_forward_btn->SetBitmap(wxGetBitmap(forward));
	m_forward_btn->Bind(wxEVT_BUTTON, &MoviePanel::OnForward, this);
	m_forward_btn->SetToolTip("Proceed forward to the end of a clip");
	m_loop_btn = new wxToggleButton(this, wxID_ANY, "",
		wxDefaultPosition, bs);
	m_loop_btn->SetBitmap(wxGetBitmap(loop));
	m_loop_btn->Bind(wxEVT_TOGGLEBUTTON, &MoviePanel::OnLoop, this);
	m_loop_btn->SetToolTip("Enable clip playback in a loop");
	m_progress_text = new wxTextCtrl(this, wxID_ANY, "0.00",
		wxDefaultPosition, ts, wxTE_RIGHT);
	m_progress_text->SetFont(f);
	m_progress_text->Bind(wxEVT_TEXT, &MoviePanel::OnCurTimeText, this);
	m_progress_text->SetToolTip("Set current time (in seconds) within a clip");
	st = new wxStaticText(this, wxID_ANY, "Sec.",
		wxDefaultPosition, FromDIP(wxSize(26, -1)));
	m_save_btn = new wxButton(this, wxID_ANY, "",
		wxDefaultPosition, bs);
	m_save_btn->SetBitmap(wxGetBitmap(save));
	m_save_btn->Bind(wxEVT_BUTTON, &MoviePanel::OnSave, this);
	m_save_btn->SetToolTip("Export a movie clip");
	sizer4->AddStretchSpacer(2);
	sizer4->Add(m_rewind_btn, 0, wxALIGN_CENTER);
	sizer4->Add(m_play_inv_btn, 0, wxALIGN_CENTER);
	sizer4->Add(m_play_btn, 0, wxALIGN_CENTER);
	sizer4->Add(m_forward_btn, 0, wxALIGN_CENTER);
	sizer4->Add(m_loop_btn, 0, wxALIGN_CENTER);
	sizer4->AddStretchSpacer(1);
	sizer4->Add(bs.x, bs.y);
	sizer4->Add(m_progress_text, 0, wxALIGN_CENTER);
	sizer4->Add(st, 0, wxALIGN_CENTER);
	sizer4->AddStretchSpacer(2);
	sizer4->Add(ts.x - bs.x, ts.y);
	sizer4->Add(m_save_btn, 0, wxALIGN_CENTER);
	sizer4->Add(5, 5);

	//sizer
	wxBoxSizer *sizerv = new wxBoxSizer(wxVERTICAL);
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

void MoviePanel::FluoUpdate(const fluo::ValueCollection& vc)
{
	if (FOUND_VALUE(gstNull))
		return;

	bool update_all = vc.empty() || FOUND_VALUE(gstMovieAgent);
	bool bval;
	int ival;

	//modes
	if (update_all || FOUND_VALUE(gstMovFps))
		m_fps_text->ChangeValue(wxString::Format("%.0f", glbin_moviemaker.GetFps()));

	if (update_all || FOUND_VALUE(gstMovLength))
		m_movie_len_text->ChangeValue(wxString::Format("%.2f", glbin_moviemaker.GetMovieLength()));

	if (update_all || FOUND_VALUE(gstMovViewList))
	{
		m_views_cmb->Clear();
		Root* root = glbin_data_manager.GetRoot();
		if (root)
		{
			for (int i = 0; i < root->GetViewNum(); i++)
			{
				auto view = root->GetView(i);
				if (view && m_views_cmb)
					m_views_cmb->Append(view->GetName());
			}
		}
	}
	if (update_all || FOUND_VALUE(gstMovViewIndex))
		m_views_cmb->Select(glbin_mov_def.m_view_idx);

	if (update_all || FOUND_VALUE(gstMovSliderStyle))
	{
		bval = glbin_mov_def.m_slider_style;
		m_progress_sldr->SetMode(bval ? 1 : 0);
		if (bval)
			m_slider_btn->SetToolNormalBitmap(0, wxGetBitmap(jog));
		else
			m_slider_btn->SetToolNormalBitmap(0, wxGetBitmap(slider));
	}

	if (update_all || FOUND_VALUE(gstMovProgSlider))
	{
		m_progress_sldr->SetScrollbar2(
			glbin_moviemaker.GetCurrentFrame(),
			glbin_moviemaker.GetScrollThumbSize(),
			glbin_moviemaker.GetClipStartFrame(),
			glbin_moviemaker.GetClipEndFrame(), 1);
		m_progress_sldr->ChangeValue(glbin_moviemaker.GetCurrentFrame());
	}

	if (update_all || FOUND_VALUE(gstBeginFrame))
		m_start_frame_text->ChangeValue(wxString::Format("%d",
			glbin_moviemaker.GetClipStartFrame()));

	if (update_all || FOUND_VALUE(gstEndFrame))
		m_end_frame_text->ChangeValue(wxString::Format("%d",
			glbin_moviemaker.GetClipEndFrame()));

	if (update_all || FOUND_VALUE(gstCurrentFrame))
	{
		m_cur_frame_text->ChangeValue(wxString::Format("%d",
			glbin_moviemaker.GetCurrentFrame()));
		m_cur_frame_text->Update();
	}

	if (update_all || FOUND_VALUE(gstTotalFrames))
		m_full_frame_text->ChangeValue(wxString::Format("%d",
			glbin_moviemaker.GetFullFrameNum()));

	if (update_all || FOUND_VALUE(gstMovCurTime))
	{
		m_progress_text->ChangeValue(wxString::Format("%.2f",
			glbin_moviemaker.GetCurrentTime()));
		m_progress_text->Update();
	}

	if (update_all || FOUND_VALUE(gstMovPlay))
	{
		if (glbin_moviemaker.IsRunning())
		{
			if (!m_running)
			{
				if (glbin_moviemaker.IsReverse())
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
				m_running = true;
			}
		}
		else
		{
			if (glbin_settings.m_run_script)
			{
				m_play_btn->SetBitmap(wxGetBitmap(play_script));
				m_play_btn->SetValue(false);
				m_play_inv_btn->SetBitmap(wxGetBitmap(play_inv_script));
				m_play_inv_btn->SetValue(false);
			}
			else
			{
				m_play_btn->SetBitmap(wxGetBitmap(play));
				m_play_btn->SetValue(false);
				m_play_inv_btn->SetBitmap(wxGetBitmap(play_inv));
				m_play_inv_btn->SetValue(false);
			}
			m_running = false;
		}
	}

	if (update_all || FOUND_VALUE(gstMovLoop))
		m_loop_btn->SetValue(glbin_moviemaker.IsLoop());

	if (update_all || FOUND_VALUE(gstMovRotEnable))
	{
		bval = glbin_moviemaker.GetRotateEnable();
		m_rot_chk->SetValue(bval);
		m_x_rd->Enable(bval);
		m_y_rd->Enable(bval);
		m_z_rd->Enable(bval);
		m_degree_text->Enable(bval);
		m_rot_int_cmb->Enable(bval);
	}

	if (update_all || FOUND_VALUE(gstMovRotAxis))
	{
		ival = glbin_moviemaker.GetRotateAxis();
		if (ival == 0)
			m_x_rd->SetValue(true);
		else if (ival == 1)
			m_y_rd->SetValue(true);
		else if (ival == 2)
			m_z_rd->SetValue(true);
	}

	if (update_all || FOUND_VALUE(gstMovRotAng))
		m_degree_text->ChangeValue(wxString::Format("%d", glbin_moviemaker.GetRotateDeg()));

	if (update_all || FOUND_VALUE(gstMovIntrpMode))
	{
		m_rot_int_cmb->SetSelection(glbin_moviemaker.GetInterpolation());
		m_interpolation_cmb->SetSelection(glbin_moviemaker.GetInterpolation());
	}

	if (update_all || FOUND_VALUE(gstMovSeqMode))
	{
		ival = glbin_moviemaker.GetSeqMode();
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
		bval = ival > 0;
		m_seq_dec_btn->Enable(bval);
		m_seq_inc_btn->Enable(bval);
		m_seq_num_text->Enable(bval);
	}

	if (update_all || FOUND_VALUE(gstMovSeqNum))
	{
		m_seq_num_text->ChangeValue(wxString::Format("%d", glbin_moviemaker.GetSeqCurNum()));
		m_seq_total_text->ChangeValue(wxString::Format("%d", glbin_moviemaker.GetSeqAllNum()));
	}

	if (update_all || FOUND_VALUE(gstCaptureParam))
		m_keyframe_chk->SetValue(glbin_moviemaker.GetKeyframeEnable());

	if (update_all || FOUND_VALUE(gstParamKeyDuration))
		m_duration_text->ChangeValue(wxString::Format("%.0f", glbin_moviemaker.GetKeyDuration()));

	if (update_all || FOUND_VALUE(gstParamList))
		m_keylist->Update();

	if (update_all || FOUND_VALUE(gstParamListSelect))
	{
		double t = glbin_moviemaker.GetCurProg();
		int index = glbin_interpolator.GetKeyIndexFromTime(t);
		long item = m_keylist->GetNextItem(-1,
			wxLIST_NEXT_ALL,
			wxLIST_STATE_SELECTED);
		if (index != item && index != -1)
			m_keylist->SelectItemSilently(index);
	}

	if (update_all || FOUND_VALUE(gstCamLockObjEnable))
		m_cam_lock_chk->SetValue(glbin_moviemaker.GetCamLock());

	if (update_all || FOUND_VALUE(gstCamLockType))
		m_cam_lock_cmb->SetSelection(glbin_moviemaker.GetCamLockType() - 1);

	if (update_all || FOUND_VALUE(gstCropEnable))
	{
		bval = glbin_moviemaker.GetCropEnable();
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

	if (update_all || FOUND_VALUE(gstCropValues))
	{
		m_crop_x_text->ChangeValue(wxString::Format("%d", glbin_moviemaker.GetCropX()));
		m_crop_y_text->ChangeValue(wxString::Format("%d", glbin_moviemaker.GetCropY()));
		m_crop_w_text->ChangeValue(wxString::Format("%d", glbin_moviemaker.GetCropW()));
		m_crop_h_text->ChangeValue(wxString::Format("%d", glbin_moviemaker.GetCropH()));
	}

	if (update_all || FOUND_VALUE(gstScalebarPos))
	{
		ival = glbin_moviemaker.GetScalebarPos();
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
		m_sb_dx_text->ChangeValue(wxString::Format("%d", glbin_moviemaker.GetScalebarX()));
		m_sb_dy_text->ChangeValue(wxString::Format("%d", glbin_moviemaker.GetScalebarY()));
	}

	if (update_all || FOUND_VALUE(gstRunScript))
	{
		bval = glbin_settings.m_run_script;
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

	if (update_all || FOUND_VALUE(gstScriptFile))
	{
		m_script_file_text->ChangeValue(glbin_settings.m_script_file);
	}

	if (update_all || FOUND_VALUE(gstScriptList))
	{
		m_script_list->DeleteAllItems();
		std::vector<std::wstring> list;
		std::wstring filename;
		long tmp;
		if (GetScriptFiles(list))
		{
			for (size_t i = 0; i < list.size(); ++i)
			{
				std::filesystem::path p(list[i]);
				filename = p.stem().wstring();
				tmp = m_script_list->InsertItem(i, std::to_wstring(i + 1), 0);
				m_script_list->SetItem(tmp, 1, filename);
			}
			m_script_list->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
			m_script_list->SetColumnWidth(1, wxLIST_AUTOSIZE);
		}
	}

	if (update_all || FOUND_VALUE(gstScriptSelect))
	{
		std::vector<std::wstring> list;
		if (GetScriptFiles(list))
		{
			int idx = -1;
			for (size_t i = 0; i < list.size(); ++i)
			{
				if (glbin_settings.m_script_file == list[i])
				{
					idx = i;
					break;
				}
			}
			if (idx >= 0 && idx < m_script_list->GetItemCount())
			{
				m_script_list->SetItemState(idx,
					wxLIST_STATE_SELECTED,
					wxLIST_STATE_SELECTED);
				//wxSize ss = m_script_list->GetItemSpacing();
				//m_script_list->ScrollList(0, ss.y*idx);
			}
		}
	}
}

void MoviePanel::SetFps(double val)
{
	glbin_moviemaker.SetFps(val);

	FluoUpdate({ gstMovFps, gstMovLength });
}

void MoviePanel::SetMovieLength(double val)
{
	glbin_moviemaker.SetMovieLength(val);

	FluoUpdate({ gstMovFps, gstMovLength });
}

void MoviePanel::SetView(int index)
{
	glbin_mov_def.m_view_idx = index;

	FluoUpdate({ gstMovViewIndex });
}

void MoviePanel::SetSliderStyle()
{
	glbin_mov_def.m_slider_style = !glbin_mov_def.m_slider_style;

	FluoUpdate({ gstMovSliderStyle });
}

void MoviePanel::SetFullFrame(int val)
{
	glbin_moviemaker.SetFullFrameNum(val);

	FluoUpdate({ gstBeginFrame, gstEndFrame, gstCurrentFrame, gstMovFps, gstMovLength, gstMovProgSlider, gstMovSeqNum });
}

void MoviePanel::SetStartFrame(int val)
{
	if (glbin_moviemaker.IsRunning())
		return;
	glbin_moviemaker.SetClipStartFrame(val);

	FluoUpdate({ gstBeginFrame, gstEndFrame, gstCurrentFrame, gstMovFps, gstMovLength, gstMovProgSlider, gstMovSeqNum });
}

void MoviePanel::SetEndFrame(int val)
{
	if (glbin_moviemaker.IsRunning())
		return;
	glbin_moviemaker.SetClipEndFrame(val);

	FluoUpdate({ gstBeginFrame, gstEndFrame, gstCurrentFrame, gstMovFps, gstMovLength, gstMovProgSlider, gstMovSeqNum });
}

void MoviePanel::SetScrollFrame(int val, bool notify)
{
	if (glbin_moviemaker.GetCurrentFrame() == val)
		return;
	glbin_moviemaker.SetCurrentFrame(val);

	fluo::ValueCollection vc = { gstMovCurTime, gstCurrentFrame, gstMovSeqNum };
	if (notify)
		vc.insert(gstMovProgSlider);
	FluoRefresh(2, vc, { glbin_mov_def.m_view_idx });
}

void MoviePanel::SetCurrentFrame(int val, bool notify)
{
	//if (glbin_moviemaker.IsRunning())
	//	return;
	glbin_moviemaker.SetCurrentFrame(val);

	fluo::ValueCollection vc = { gstMovCurTime, gstMovProgSlider, gstMovSeqNum };
	if (notify)
		vc.insert(gstCurrentFrame);
	FluoRefresh(2, vc, { glbin_mov_def.m_view_idx });
}

void MoviePanel::SetCurrentTime(double val, bool notify)
{
	if (glbin_moviemaker.IsRunning())
		return;

	glbin_moviemaker.SetCurrentTime(val);

	fluo::ValueCollection vc = { gstCurrentFrame, gstMovProgSlider, gstMovSeqNum };
	if (notify)
		vc.insert(gstMovCurTime);
	FluoRefresh(2, vc, { glbin_mov_def.m_view_idx });
}

void MoviePanel::Play()
{
	glbin_moviemaker.Play(false);

	fluo::ValueCollection vc = { gstMovPlay };
	FluoRefresh(2, vc, { glbin_mov_def.m_view_idx });
}

void MoviePanel::PlayInv()
{
	glbin_moviemaker.Play(true);

	fluo::ValueCollection vc = { gstMovPlay };
	FluoRefresh(2, vc, { glbin_mov_def.m_view_idx });
}

void MoviePanel::Rewind()
{
	glbin_moviemaker.Rewind();

	fluo::ValueCollection vc = { gstCurrentFrame, gstMovCurTime, gstMovProgSlider, gstMovPlay, gstMovSeqNum };
	FluoRefresh(2, vc, { glbin_mov_def.m_view_idx });
}

void MoviePanel::Forward()
{
	glbin_moviemaker.Forward();

	fluo::ValueCollection vc = { gstCurrentFrame, gstMovCurTime, gstMovProgSlider, gstMovPlay, gstMovSeqNum };
	FluoRefresh(2, vc, { glbin_mov_def.m_view_idx });
}

void MoviePanel::Loop(bool val)
{
	glbin_moviemaker.SetLoop(val);
}

void MoviePanel::IncFrame()
{
	if (glbin_moviemaker.IsRunning())
		return;
	int frame = glbin_moviemaker.GetCurrentFrame();
	frame++;
	glbin_moviemaker.SetCurrentFrame(frame);
	fluo::ValueCollection vc = { gstMovCurTime, gstMovProgSlider, gstCurrentFrame, gstMovSeqNum };
	FluoRefresh(2, vc, { glbin_mov_def.m_view_idx });
}

void MoviePanel::DecFrame()
{
	if (glbin_moviemaker.IsRunning())
		return;
	int frame = glbin_moviemaker.GetCurrentFrame();
	frame--;
	glbin_moviemaker.SetCurrentFrame(frame);
	fluo::ValueCollection vc = { gstMovCurTime, gstMovProgSlider, gstCurrentFrame, gstMovSeqNum };
	FluoRefresh(2, vc, { glbin_mov_def.m_view_idx });
}

void MoviePanel::Save(const std::wstring& filename)
{
	if (glbin_moviemaker.IsRunning())
		return;

	glbin_states.m_capture = true;
	glbin_moviemaker.SetFileName(filename);
	glbin_moviemaker.PlaySave();

	fluo::ValueCollection vc = { gstMovPlay };
	FluoRefresh(2, vc, { glbin_mov_def.m_view_idx });
}

void MoviePanel::SetKeyframeMovie(bool val)
{
	glbin_moviemaker.SetKeyframeEnable(val, true);

	FluoUpdate({ gstCaptureParam, gstMovLength, gstMovProgSlider, gstBeginFrame, gstEndFrame, gstCurrentFrame, gstMovCurTime, gstMovSeqNum });
}

void MoviePanel::SetCropEnable(bool val)
{
	glbin_moviemaker.SetCropEnable(val);
	FluoRefresh(2, { gstCropEnable, gstCropValues }, { glbin_mov_def.m_view_idx });
}

void MoviePanel::SetCropValues(int x, int y, int w, int h)
{
	glbin_moviemaker.SetCropValues(x, y, w, h);
	FluoRefresh(2, { gstCropValues }, { glbin_mov_def.m_view_idx });
}

void MoviePanel::SetScalebarPos(int pos)
{
	glbin_moviemaker.SetScalebarPos(pos);
	FluoRefresh(2, { gstNull }, { glbin_mov_def.m_view_idx });
}

void MoviePanel::SetScalebarValues(int x, int y)
{
	glbin_moviemaker.SetScalebarDist(x, y);
	FluoRefresh(2, { gstScalebarPos }, { glbin_mov_def.m_view_idx });
}

void MoviePanel::OnNotebookPage(wxAuiNotebookEvent& event)
{
	event.Skip();
}

void MoviePanel::OnFpsEdit(wxCommandEvent& event)
{
	wxString str = m_fps_text->GetValue();
	double val;
	if (str.ToDouble(&val))
		SetFps(val);
}

void MoviePanel::OnMovieLenText(wxCommandEvent& event)
{
	wxString str = m_movie_len_text->GetValue();
	double val;
	if (str.ToDouble(&val))
		SetMovieLength(val);
}

void MoviePanel::OnViewSelected(wxCommandEvent& event)
{
	int val = m_views_cmb->GetCurrentSelection();
	SetView(val);
}

void MoviePanel::OnSliderStyle(wxCommandEvent& event)
{
	SetSliderStyle();
}

void MoviePanel::OnProgressScroll(wxScrollEvent& event)
{
	int val = m_progress_sldr->GetValue();
	SetScrollFrame(val, false);
}

void MoviePanel::OnStartFrameText(wxCommandEvent& event)
{
	wxString str = m_start_frame_text->GetValue();
	long lval;
	if (str.ToLong(&lval))
		SetStartFrame(lval);
}

void MoviePanel::OnEndFrameText(wxCommandEvent& event)
{
	wxString str = m_end_frame_text->GetValue();
	long lval;
	if (str.ToLong(&lval))
		SetEndFrame(lval);
}

void MoviePanel::OnCurFrameText(wxCommandEvent& event)
{
	wxString str = m_cur_frame_text->GetValue();
	long lval;
	if (str.ToLong(&lval))
		SetCurrentFrame(lval, false);
}

void MoviePanel::OnCurTimeText(wxCommandEvent& event)
{
	wxString str = m_progress_text->GetValue();
	double val;
	if (str.ToDouble(&val))
		SetCurrentTime(val, false);
}

void MoviePanel::OnPlay(wxCommandEvent& event)
{
	Play();
}

void MoviePanel::OnPlayInv(wxCommandEvent& event)
{
	PlayInv();
}

void MoviePanel::OnRewind(wxCommandEvent& event)
{
	Rewind();
}

void MoviePanel::OnForward(wxCommandEvent& event)
{
	Forward();
}

void MoviePanel::OnLoop(wxCommandEvent& event)
{
	Loop(m_loop_btn->GetValue());
}

void MoviePanel::OnStartFrameBtn(wxCommandEvent& event)
{
	int frame = glbin_moviemaker.GetCurrentFrame();
	SetStartFrame(frame);
}

void MoviePanel::OnEndFrameBtn(wxCommandEvent& event)
{
	int frame = glbin_moviemaker.GetCurrentFrame();
	SetEndFrame(frame);
}

void MoviePanel::OnIncFrame(wxCommandEvent& event)
{
	IncFrame();
}

void MoviePanel::OnDecFrame(wxCommandEvent& event)
{
	DecFrame();
}

void MoviePanel::OnFullFrameText(wxCommandEvent& event)
{
	wxString str = m_full_frame_text->GetValue();
	long val;
	if (str.ToLong(&val))
		SetFullFrame(val);
}

void MoviePanel::OnSave(wxCommandEvent& event)
{
	ModalDlg fopendlg(
		m_frame, "Save Movie Sequence",
		"", "output",
		"MP4 file (*.mp4)|*.mp4|"\
		"Tiff files(*.tif)|*.tif|"\
		"Tiff files(*.tiff)|*.tiff|"\
		"Png files(*.png)|*.png|"\
		"Jpeg files(*.jpg)|*.jpg|"\
		"Jpeg files(*.jpeg)|*.jpeg",
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	fopendlg.SetExtraControlCreator(CreateExtraCaptureControl);
	fopendlg.CenterOnParent();
	int rval = fopendlg.ShowModal();
	if (rval == wxID_OK)
	{
		Save(fopendlg.GetPath().ToStdWstring());
	}
}

void MoviePanel::OnRotateChecked(wxCommandEvent& event)
{
	bool val = m_rot_chk->GetValue();
	glbin_moviemaker.SetRotateEnable(val);
	FluoUpdate({ gstCaptureParam, gstMovRotEnable, gstMovSeqMode, gstBeginFrame, gstEndFrame, gstCurrentFrame, gstMovLength, gstMovProgSlider, gstMovSeqNum });
}

void MoviePanel::OnRotAxis(wxCommandEvent& event)
{
	int val = 0;
	if (m_x_rd->GetValue())
		val = 0;
	else if (m_y_rd->GetValue())
		val = 1;
	else if (m_z_rd->GetValue())
		val = 2;
	glbin_moviemaker.SetRotateAxis(val);
}

void MoviePanel::OnDegreeText(wxCommandEvent& event)
{
	wxString str = m_degree_text->GetValue();
	long ival = 0;
	str.ToLong(&ival);
	glbin_moviemaker.SetRotateDeg(ival);
	FluoUpdate({ gstBeginFrame, gstEndFrame, gstCurrentFrame, gstMovLength, gstMovProgSlider, gstMovSeqNum });
}

void MoviePanel::OnRotIntCmb(wxCommandEvent& event)
{
	int val = m_rot_int_cmb->GetCurrentSelection();
	glbin_moviemaker.SetInterpolation(val);
	FluoUpdate({ gstMovIntrpMode });
}

void MoviePanel::OnSequenceChecked(wxCommandEvent& event)
{
	bool val = m_seq_chk->GetValue();
	if (val)
		glbin_moviemaker.SetSeqMode(1);
	else
		glbin_moviemaker.SetSeqMode(0);
	FluoUpdate({ gstCaptureParam, gstMovRotEnable, gstMovSeqMode, gstBeginFrame, gstEndFrame, gstCurrentFrame, gstMovLength, gstMovProgSlider, gstMovSeqNum });
}

void MoviePanel::OnBatchChecked(wxCommandEvent& event)
{
	int val = m_bat_chk->GetValue();
	if (val)
		glbin_moviemaker.SetSeqMode(2);
	else
		glbin_moviemaker.SetSeqMode(0);
	FluoUpdate({ gstCaptureParam, gstMovRotEnable, gstMovSeqMode, gstBeginFrame, gstEndFrame, gstCurrentFrame, gstMovLength, gstMovProgSlider, gstMovSeqNum });
}

void MoviePanel::OnSeqDecBtn(wxCommandEvent& event)
{
	int val = glbin_moviemaker.GetSeqCurNum();
	glbin_moviemaker.SetSeqCurNum(val - 1);
	FluoRefresh(2,
		{ gstCurrentFrame, gstMovProgSlider, gstMovSeqNum },
		{ glbin_mov_def.m_view_idx });
}

void MoviePanel::OnSeqNumText(wxCommandEvent& event)
{
	wxString str = m_seq_num_text->GetValue();
	long val;
	if (str.ToLong(&val))
		glbin_moviemaker.SetSeqCurNum(val);
	FluoRefresh(2,
		{ gstCurrentFrame, gstMovProgSlider, gstMovSeqNum },
		{ glbin_mov_def.m_view_idx });
}

void MoviePanel::OnSeqIncBtn(wxCommandEvent& event)
{
	int val = glbin_moviemaker.GetSeqCurNum();
	glbin_moviemaker.SetSeqCurNum(val + 1);
	FluoRefresh(2,
		{ gstCurrentFrame, gstMovProgSlider, gstMovSeqNum },
		{ glbin_mov_def.m_view_idx });
}

void MoviePanel::OnAct(wxListEvent& event)
{
	long item = m_keylist->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	if (item == -1)
		return;
	wxString str = m_keylist->GetItemText(item);
	long id;
	str.ToLong(&id);

	int index = glbin_interpolator.GetKeyIndex(int(id));
	double time = glbin_interpolator.GetKeyTime(index);
	glbin_moviemaker.SetCurrentFrame(time);

	RenderView* view = glbin_moviemaker.GetView();
	if (view)
		view->SetParams(time);
	FluoRefresh(2, { gstCurrentFrame, gstMovProgSlider, gstMovSeqNum, gstParamListSelect},
		{ glbin_mov_def.m_view_idx });
}

void MoviePanel::OnKeyframeChk(wxCommandEvent& event)
{
	bool val = m_keyframe_chk->GetValue();
	SetKeyframeMovie(val);
}

void MoviePanel::OnDurationText(wxCommandEvent& event)
{
	wxString str = m_duration_text->GetValue();
	double val;
	str.ToDouble(&val);
	glbin_moviemaker.SetKeyDuration(val);
}

void MoviePanel::OnInterpolation(wxCommandEvent& event)
{
	int val = m_interpolation_cmb->GetSelection();
	glbin_moviemaker.SetInterpolation(val);
	FluoUpdate({ gstMovIntrpMode });
}

void MoviePanel::OnInsKey(wxCommandEvent& event)
{
	if (!glbin_moviemaker.GetKeyframeEnable())
		glbin_moviemaker.SetKeyframeEnable(true, true);

	wxString str;
	long item = m_keylist->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	int index = -1;
	if (item != -1)
	{
		str = m_keylist->GetItemText(item);
		long id;
		str.ToLong(&id);
		index = glbin_interpolator.GetKeyIndex(id);
	}
	glbin_moviemaker.InsertKey(index);

	FluoUpdate({ gstCaptureParam, gstMovLength, gstMovProgSlider, gstBeginFrame, gstEndFrame, gstCurrentFrame, gstTotalFrames, gstMovCurTime, gstMovSeqNum, gstParamList, gstParamListSelect });
}

void MoviePanel::OnDelKey(wxCommandEvent& event)
{
	m_keylist->DeleteSel();
	glbin_moviemaker.SetFullFrameNum(std::round(glbin_interpolator.GetLastT()));
	FluoUpdate({ gstMovLength, gstMovProgSlider, gstBeginFrame, gstEndFrame, gstCurrentFrame, gstTotalFrames, gstParamList, gstParamListSelect });
}

void MoviePanel::OnDelAll(wxCommandEvent& event)
{
	m_keylist->DeleteAll();
	glbin_moviemaker.SetFullFrameNum(std::round(glbin_interpolator.GetLastT()));
	FluoUpdate({ gstMovLength, gstMovProgSlider, gstBeginFrame, gstEndFrame, gstCurrentFrame, gstTotalFrames, gstParamList, gstParamListSelect });
}

void MoviePanel::OnCamLockChk(wxCommandEvent& event)
{
	glbin_moviemaker.SetCamLock(m_cam_lock_chk->GetValue());
}

void MoviePanel::OnCamLockCmb(wxCommandEvent& event)
{
	glbin_moviemaker.SetCamLockType(m_cam_lock_cmb->GetSelection() + 1);
}

void MoviePanel::OnCamLockBtn(wxCommandEvent& event)
{
	m_view->SetLockCenter();
}

//auto key
void MoviePanel::OnGenKey(wxCommandEvent& event)
{
	long item = m_auto_key_list->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);

	if (item != -1)
	{
		glbin_moviemaker.MakeKeys(item);
		if (!glbin_moviemaker.GetKeyframeEnable())
			glbin_moviemaker.SetKeyframeEnable(true, true);

		m_notebook->SetSelection(1);
		FluoUpdate({ gstCaptureParam, gstMovLength, gstMovProgSlider, gstBeginFrame, gstEndFrame, gstCurrentFrame, gstTotalFrames, gstMovCurTime, gstMovSeqNum, gstParamList, gstParamListSelect });
	}
}

void MoviePanel::OnCropCheck(wxCommandEvent& event)
{
	SetCropEnable(m_crop_chk->GetValue());
}

void MoviePanel::OnResetCrop(wxCommandEvent& event)
{
	SetCropEnable(true);
}

void MoviePanel::OnEditCrop(wxCommandEvent& event)
{
	wxString temp;
	long x, y, w, h;
	temp = m_crop_x_text->GetValue();
	temp.ToLong(&x);
	temp = m_crop_y_text->GetValue();
	temp.ToLong(&y);
	temp = m_crop_w_text->GetValue();
	temp.ToLong(&w);
	temp = m_crop_h_text->GetValue();
	temp.ToLong(&h);

	SetCropValues(x, y, w, h);
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
	int pos = 3;
	if (m_sb_tl_rb->GetValue())
		pos = 0;
	else if (m_sb_tr_rb->GetValue())
		pos = 1;
	else if (m_sb_bl_rb->GetValue())
		pos = 2;
	else if (m_sb_br_rb->GetValue())
		pos = 3;
	SetScalebarPos(pos);
}

void MoviePanel::OnSbEdit(wxCommandEvent& event)
{
	wxString temp;
	long x, y;
	temp = m_sb_dx_text->GetValue();
	temp.ToLong(&x);
	temp = m_sb_dy_text->GetValue();
	temp.ToLong(&y);
	SetScalebarValues(x, y);
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
	bool val = m_run_script_chk->GetValue();
	std::wstring str = m_script_file_text->GetValue().ToStdWstring();
	EnableScript(val, str);
}

void MoviePanel::OnScriptFileEdit(wxCommandEvent& event)
{
	wxString str = m_script_file_text->GetValue();
	glbin_settings.m_script_file = str;
}

void MoviePanel::OnScriptClearBtn(wxCommandEvent& event)
{
	m_script_file_text->Clear();
	EnableScript(false);
}

void MoviePanel::OnScriptFileBtn(wxCommandEvent& event)
{
	ModalDlg fopendlg(
		m_frame, "Choose a 4D script file", "", "",
		"4D script file (*.txt)|*.txt",
		wxFD_OPEN);

	int rval = fopendlg.ShowModal();
	if (rval == wxID_OK)
	{
		std::wstring file = fopendlg.GetPath().ToStdWstring();
		glbin_settings.m_script_file = file;
		m_script_file_text->ChangeValue(file);

		EnableScript(true, file);
	}
}

void MoviePanel::OnScriptListSelected(wxListEvent& event)
{
	long item = m_script_list->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);

	if (item != -1)
	{
		wxString file = m_script_list->GetItemText(item, 1);
		std::filesystem::path p = std::filesystem::current_path();
		p = p / "Scripts" / (file.ToStdString() + ".txt");
		std::wstring filename = p.wstring();
		m_script_file_text->ChangeValue(filename);
		EnableScript(true, filename);
	}
}

size_t MoviePanel::GetScriptFiles(std::vector<std::wstring>& list)
{
	std::filesystem::path p = std::filesystem::current_path();
	p /=  "Scripts";
	// Iterate over the files in the "Scripts" directory
	for (const auto& entry : std::filesystem::directory_iterator(p))
	{
		if (entry.is_regular_file() && entry.path().extension() == ".txt")
		{
			list.push_back(entry.path().wstring());
		}
	}

	// Sort the list of files
	std::sort(list.begin(), list.end());
	return list.size();
}

void MoviePanel::EnableScript(bool val, const std::wstring& filename)
{
	glbin_settings.m_run_script = val;
	glbin_settings.m_script_file = filename;
	FluoRefresh(2, { gstMovPlay, gstRunScript }, { glbin_mov_def.m_view_idx });
}

//ch1
void MoviePanel::OnCh1Check(wxCommandEvent& event)
{
	wxCheckBox* ch1 = (wxCheckBox*)event.GetEventObject();
	if (ch1)
		glbin_settings.m_save_compress = ch1->GetValue();
}

//ch2
void MoviePanel::OnCh2Check(wxCommandEvent& event)
{
	wxCheckBox* ch2 = (wxCheckBox*)event.GetEventObject();
	if (ch2)
		glbin_settings.m_save_alpha = ch2->GetValue();
}

//ch3
void MoviePanel::OnCh3Check(wxCommandEvent& event)
{
	wxCheckBox* ch3 = (wxCheckBox*)event.GetEventObject();
	if (ch3)
		glbin_settings.m_save_float = ch3->GetValue();
}

void MoviePanel::OnDpiText(wxCommandEvent& event)
{
	wxTextCtrl* tx_dpi = (wxTextCtrl*)event.GetEventObject();
	wxString str = event.GetString();
	long lval;
	str.ToLong(&lval);
	glbin_settings.m_dpi = lval;
	if (!tx_dpi)
		return;
	wxCheckBox* ch_enlarge = (wxCheckBox*)tx_dpi->GetParent()->FindWindow(ID_ENLARGE_CHK);
	wxSlider* sl_enlarge = (wxSlider*)tx_dpi->GetParent()->FindWindow(ID_ENLARGE_SLDR);
	wxTextCtrl* tx_enlarge = (wxTextCtrl*)tx_dpi->GetParent()->FindWindow(ID_ENLARGE_TEXT);
	bool enlarge = lval > 72;
	if (ch_enlarge)
		ch_enlarge->SetValue(enlarge);
	double enlarge_scale = (double)lval / 72.0;
	auto view = glbin_current.render_view.lock();
	if (view)
	{
		view->SetEnlarge(enlarge);
		view->SetEnlargeScale(enlarge_scale);
	}
	if (sl_enlarge)
	{
		sl_enlarge->Enable(enlarge);
		sl_enlarge->SetValue(std::round(enlarge_scale * 10));
	}
	if (tx_enlarge)
	{
		tx_enlarge->Enable(enlarge);
		tx_enlarge->ChangeValue(wxString::Format("%.1f", enlarge_scale));
	}
}

//enlarge output image
void MoviePanel::OnChEnlargeCheck(wxCommandEvent& event)
{
	wxCheckBox* ch_enlarge = (wxCheckBox*)event.GetEventObject();
	if (ch_enlarge)
	{
		bool enlarge = ch_enlarge->GetValue();
		auto view = glbin_current.render_view.lock();
		if (view)
			view->SetEnlarge(enlarge);
		if (ch_enlarge->GetParent())
		{
			wxSlider* sl_enlarge = (wxSlider*)
				ch_enlarge->GetParent()->FindWindow(
					ID_ENLARGE_SLDR);
			wxTextCtrl* tx_enlarge = (wxTextCtrl*)
				ch_enlarge->GetParent()->FindWindow(
					ID_ENLARGE_TEXT);
			if (sl_enlarge && tx_enlarge)
			{
				if (enlarge)
				{
					sl_enlarge->Enable();
					tx_enlarge->Enable();
				}
				else
				{
					sl_enlarge->Disable();
					tx_enlarge->Disable();
				}
			}
		}
	}
}

void MoviePanel::OnSlEnlargeScroll(wxScrollEvent& event)
{
	int ival = event.GetPosition();
	wxSlider* sl_enlarge = (wxSlider*)event.GetEventObject();
	if (sl_enlarge && sl_enlarge->GetParent())
	{
		wxTextCtrl* tx_enlarge = (wxTextCtrl*)
			sl_enlarge->GetParent()->FindWindow(
				ID_ENLARGE_TEXT);
		if (tx_enlarge)
		{
			wxString str = wxString::Format("%.1f", ival / 10.0);
			tx_enlarge->ChangeValue(str);
		}
	}
}

void MoviePanel::OnTxEnlargeText(wxCommandEvent& event)
{
	wxString str = event.GetString();
	double dval;
	str.ToDouble(&dval);
	auto view = glbin_current.render_view.lock();
	if (view)
		view->SetEnlargeScale(dval);
	int ival = std::round(dval * 10);
	wxTextCtrl* tx_enlarge = (wxTextCtrl*)event.GetEventObject();
	if (tx_enlarge && tx_enlarge->GetParent())
	{
		wxSlider* sl_enlarge = (wxSlider*)
			tx_enlarge->GetParent()->FindWindow(
				ID_ENLARGE_SLDR);
		if (sl_enlarge)
			sl_enlarge->SetValue(ival);
	}
}

//movie quality
void MoviePanel::OnMovieQuality(wxCommandEvent& event)
{
	wxString str = event.GetString();
	double dval;
	str.ToDouble(&dval);
	glbin_settings.m_mov_bitrate = dval;
	wxTextCtrl* tx_qual = (wxTextCtrl*)event.GetEventObject();
	if (tx_qual && tx_qual->GetParent())
	{
		wxTextCtrl* tx_estimate = (wxTextCtrl*)
			tx_qual->GetParent()->FindWindow(ID_MOV_ESTIMATE_TEXT);
		if (tx_estimate)
		{
			double size = glbin_settings.m_mov_bitrate *
				glbin_moviemaker.GetMovieLength() / 8.;
			tx_estimate->ChangeValue(wxString::Format("%.2f", size));
		}
	}

}
//embed project
void MoviePanel::OnChEmbedCheck(wxCommandEvent& event)
{
	wxCheckBox* ch_embed = (wxCheckBox*)event.GetEventObject();
	if (ch_embed)
		glbin_settings.m_vrp_embed = ch_embed->GetValue();
}

wxWindow* MoviePanel::CreateExtraCaptureControl(wxWindow* parent)
{
	wxPanel* panel = new wxPanel(parent);
#ifdef _DARWIN
	panel->SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#elifdef __linux__
    panel->SetWindowVariant(wxWINDOW_VARIANT_MINI);
#endif
	wxStaticBoxSizer *group1 = new wxStaticBoxSizer(
		wxVERTICAL, panel, "Additional Options");

	wxStaticText* mov_note = new wxStaticText(panel, wxID_ANY,
		"TIFF sequence is preferrable for very short movies.\n",
		wxDefaultPosition, parent->FromDIP(wxSize(-1, 20)));

	//copy all files check box
	wxBoxSizer* line0 = 0;
	if (glbin_settings.m_prj_save)
	{
		line0 = new wxBoxSizer(wxHORIZONTAL);
		wxCheckBox* ch_embed = 0;
		ch_embed = new wxCheckBox(panel, wxID_ANY,
			"Embed all files in the project folder");
		ch_embed->Connect(ch_embed->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
			wxCommandEventHandler(MoviePanel::OnChEmbedCheck), NULL, panel);
		ch_embed->SetValue(glbin_settings.m_vrp_embed);
		line0->Add(ch_embed, 0, wxALIGN_CENTER);
	}

	//dpi
	wxBoxSizer *line1 = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText* st = new wxStaticText(panel, wxID_ANY, "DPI: ",
		wxDefaultPosition, wxDefaultSize);
	wxIntegerValidator<unsigned int> vald_int;
	wxTextCtrl* tx_dpi = new wxTextCtrl(panel, wxID_ANY,
		"", wxDefaultPosition, parent->FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_int);
	tx_dpi->Connect(tx_dpi->GetId(), wxEVT_COMMAND_TEXT_UPDATED,
		wxCommandEventHandler(MoviePanel::OnDpiText), NULL, panel);
	float dpi = glbin_settings.m_dpi;
	tx_dpi->ChangeValue(wxString::Format("%.0f", dpi));
	//enlarge
	wxCheckBox* ch_enlarge = new wxCheckBox(panel, ID_ENLARGE_CHK,
		"Enlarge output image");
	ch_enlarge->Connect(ch_enlarge->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(MoviePanel::OnChEnlargeCheck), NULL, panel);
	bool enlarge = dpi > 72;
	double enlarge_scale = dpi / 72.0;
	ch_enlarge->SetValue(enlarge);
	wxSlider* sl_enlarge = new wxSlider(panel, ID_ENLARGE_SLDR,
		10, 10, 100);
	sl_enlarge->Connect(sl_enlarge->GetId(), wxEVT_COMMAND_SLIDER_UPDATED,
		wxScrollEventHandler(MoviePanel::OnSlEnlargeScroll), NULL, panel);
	sl_enlarge->Enable(enlarge);
	sl_enlarge->SetValue(std::round(enlarge_scale * 10));
	wxFloatingPointValidator<double> vald_fp(1);
	wxTextCtrl* tx_enlarge = new wxTextCtrl(panel, ID_ENLARGE_TEXT,
		"1.0", wxDefaultPosition, parent->FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_fp);
	tx_enlarge->Connect(tx_enlarge->GetId(), wxEVT_COMMAND_TEXT_UPDATED,
		wxCommandEventHandler(MoviePanel::OnTxEnlargeText), NULL, panel);
	tx_enlarge->Enable(enlarge);
	tx_enlarge->ChangeValue(wxString::Format("%.1f", enlarge_scale));
	line1->Add(st, 0, wxALIGN_CENTER);
	line1->Add(tx_dpi, 0, wxALIGN_CENTER);
	line1->Add(5, 5);
	line1->Add(ch_enlarge, 0, wxALIGN_CENTER);
	line1->Add(5, 5);
	line1->Add(sl_enlarge, 1, wxEXPAND);
	line1->Add(5, 5);
	line1->Add(tx_enlarge, 0, wxALIGN_CENTER);

	//compressed TIFF
	wxBoxSizer* line2 = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText *tiffopts = new wxStaticText(panel, wxID_ANY, "Image Sequence Options:",
		wxDefaultPosition, wxDefaultSize);
	wxCheckBox *ch1 = new wxCheckBox(panel, wxID_ANY,
		"Compress to save space");
	ch1->Connect(ch1->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(MoviePanel::OnCh1Check), NULL, panel);
	if (ch1)
		ch1->SetValue(glbin_settings.m_save_compress);
	wxCheckBox *ch2 = new wxCheckBox(panel, wxID_ANY,
		"Save alpha");
	ch2->Connect(ch2->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(MoviePanel::OnCh2Check), NULL, panel);
	if (ch2)
		ch2->SetValue(glbin_settings.m_save_alpha);
	wxCheckBox *ch3 = new wxCheckBox(panel, wxID_ANY,
		"Save float channel");
	ch3->Connect(ch3->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(MoviePanel::OnCh3Check), NULL, panel);
	if (ch3)
		ch3->SetValue(glbin_settings.m_save_float);
	line2->Add(tiffopts, 0, wxALIGN_CENTER);
	line2->Add(5, 5);
	line2->Add(ch1, 0, wxALIGN_CENTER);
	line2->Add(5, 5);
	line2->Add(ch2, 0, wxALIGN_CENTER);
	line2->Add(5, 5);
	line2->Add(ch3, 0, wxALIGN_CENTER);

	// movie quality
	wxBoxSizer* line3 = new wxBoxSizer(wxHORIZONTAL);
	//bitrate
	wxStaticText *MOVopts = new wxStaticText(panel, wxID_ANY, "Movie Options:",
		wxDefaultPosition, wxDefaultSize);
	wxTextCtrl *bitrate_text = new wxTextCtrl(panel, wxID_ANY, "20.0",
		wxDefaultPosition, parent->FromDIP(wxSize(60, 20)), wxTE_RIGHT);
	bitrate_text->Connect(bitrate_text->GetId(), wxEVT_TEXT,
		wxCommandEventHandler(MoviePanel::OnMovieQuality), NULL, panel);
	st = new wxStaticText(panel, wxID_ANY, "Bitrate:",
		wxDefaultPosition, wxDefaultSize);
	wxStaticText *st2 = new wxStaticText(panel, wxID_ANY, "Mbps",
		wxDefaultPosition, wxDefaultSize);
	wxStaticText *st3 = new wxStaticText(panel, wxID_ANY, "Estimated size:",
		wxDefaultPosition, wxDefaultSize);
	wxTextCtrl* tx_estimate = new wxTextCtrl(panel, ID_MOV_ESTIMATE_TEXT, "2.5",
		wxDefaultPosition, parent->FromDIP(wxSize(60, 20)), wxTE_RIGHT);
	tx_estimate->Disable();
	double dval;
	wxString str = bitrate_text->GetValue();
	str.ToDouble(&dval);
	glbin_settings.m_mov_bitrate = dval;
	double size = glbin_settings.m_mov_bitrate *
		glbin_moviemaker.GetMovieLength() / 8.;
	tx_estimate->ChangeValue(wxString::Format("%.2f", size));

	line3->Add(MOVopts, 0, wxALIGN_CENTER);
	line3->Add(st, 0, wxALIGN_CENTER);
	line3->Add(5, 5);
	line3->Add(bitrate_text, 0, wxALIGN_CENTER);
	line3->Add(5, 5);
	line3->Add(st2, 0, wxALIGN_CENTER);
	line3->Add(5, 5);
	line3->Add(st3, 0, wxALIGN_CENTER);
	line3->Add(tx_estimate, 0, wxALIGN_CENTER);
	st2 = new wxStaticText(panel, wxID_ANY, "MB",
		wxDefaultPosition, wxDefaultSize);
	line3->Add(5, 5);
	line3->Add(st2, 0, wxALIGN_CENTER);
	//group
	group1->Add(5, 5);
	group1->Add(mov_note);
	group1->Add(5, 5);
	if (glbin_settings.m_prj_save)
	{
		group1->Add(line0);
		group1->Add(5, 5);
	}
	group1->Add(line1);
	group1->Add(5, 5);
	group1->Add(line2);
	group1->Add(5, 5);
	group1->Add(line3);
	group1->Add(5, 5);
	panel->SetSizerAndFit(group1);
	panel->Layout();

	return panel;
}

