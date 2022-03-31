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
#include "RecorderDlg.h"
#include "RenderFrame.h"
#include <Global.hpp>
#include <AgentFactory.hpp>
#include <Root.hpp>
#include <Renderview.hpp>
#include <VolumeData.hpp>
#include <Global.hpp>
#include <VolumeFactory.hpp>
#include <base_reader.h>
#include <FLIVR/VolumeRenderer.h>
#include <wx/artprov.h>
#include <wx/valnum.h>
#include <key.xpm>

BEGIN_EVENT_TABLE(KeyListCtrl, wxListCtrl)
	EVT_LIST_ITEM_ACTIVATED(wxID_ANY, KeyListCtrl::OnAct)
	EVT_LIST_ITEM_SELECTED(wxID_ANY, KeyListCtrl::OnSelection)
	EVT_LIST_ITEM_DESELECTED(wxID_ANY, KeyListCtrl::OnEndSelection)
	EVT_TEXT(ID_FrameText, KeyListCtrl::OnFrameText)
	EVT_TEXT(ID_DurationText, KeyListCtrl::OnDurationText)
	EVT_COMBOBOX(ID_InterpoCmb, KeyListCtrl::OnInterpoCmb)
	EVT_TEXT(ID_DescriptionText, KeyListCtrl::OnDescritionText)
	EVT_KEY_DOWN(KeyListCtrl::OnKeyDown)
	EVT_KEY_UP(KeyListCtrl::OnKeyUp)
	EVT_LIST_BEGIN_DRAG(wxID_ANY, KeyListCtrl::OnBeginDrag)
	EVT_SCROLLWIN(KeyListCtrl::OnScroll)
	EVT_MOUSEWHEEL(KeyListCtrl::OnScroll)
END_EVENT_TABLE()

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

	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;

	wxListItem itemCol;
	itemCol.SetText("ID");
	this->InsertColumn(0, itemCol);
    SetColumnWidth(0, 40);
	itemCol.SetText("Frame");
	this->InsertColumn(1, itemCol);
    SetColumnWidth(1, 60);
	itemCol.SetText("Inbetweens");
	this->InsertColumn(2, itemCol);
    SetColumnWidth(2, 80);
	itemCol.SetText("Interpolation");
	this->InsertColumn(3, itemCol);
    SetColumnWidth(3, 80);
	itemCol.SetText("Description");
	this->InsertColumn(4, itemCol);
    SetColumnWidth(4, 80);

	m_images = new wxImageList(16, 16, true);
	wxIcon icon = wxIcon(key_xpm);
	m_images->Add(icon);
	AssignImageList(m_images, wxIMAGE_LIST_SMALL);

	//frame edit
	m_frame_text = new wxTextCtrl(this, ID_FrameText, "",
		wxDefaultPosition, wxDefaultSize, 0, vald_int);
	m_frame_text->Hide();
	//duration edit
	m_duration_text = new wxTextCtrl(this, ID_DurationText, "",
		wxDefaultPosition, wxDefaultSize, 0, vald_int);
	m_duration_text->Hide();
	//interpolation combo box
	m_interpolation_cmb = new wxComboBox(this, ID_InterpoCmb, "",
		wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY);
	m_interpolation_cmb->Append("Linear");
	m_interpolation_cmb->Append("Smooth");
	m_interpolation_cmb->Hide();
	//description edit
	m_description_text = new wxTextCtrl(this, ID_DescriptionText, "",
		wxDefaultPosition, wxDefaultSize);
	m_description_text->Hide();

	//SetDoubleBuffered(true);
}

KeyListCtrl::~KeyListCtrl()
{
}

void KeyListCtrl::Append(int id, int time, int duration, int interp, string &description)
{
	long tmp = InsertItem(GetItemCount(), wxString::Format("%d", id), 0);
	SetItem(tmp, 1, wxString::Format("%d", time));
	SetItem(tmp, 2, wxString::Format("%d", duration));
	SetItem(tmp, 3, interp==0?"Linear":"Smooth");
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

	if (!m_frame)
		return;
	Interpolator* interpolator = m_frame->GetInterpolator();
	if (!interpolator)
		return;
	interpolator->RemoveKey(id);
	Update();
}

void KeyListCtrl::DeleteAll()
{
	if (!m_frame)
		return;
	Interpolator* interpolator = m_frame->GetInterpolator();
	if (!interpolator)
		return;
	interpolator->Clear();
	Update();
}

void KeyListCtrl::Update()
{
	m_frame_text->Hide();
	m_duration_text->Hide();
	m_interpolation_cmb->Hide();
	m_description_text->Hide();
	m_editing_item = -1;

	if (!m_frame)
		return;
	Interpolator* interpolator = m_frame->GetInterpolator();
	if (!interpolator)
		return;

	DeleteAllItems();
	for (int i=0; i<interpolator->GetKeyNum(); i++)
	{
		int id = interpolator->GetKeyID(i);
		int time = interpolator->GetKeyTime(i);
		int duration = interpolator->GetKeyDuration(i);
		int interp = interpolator->GetKeyType(i);
		string desc = interpolator->GetKeyDesc(i);
		Append(id, time, duration, interp, desc);
	}
}

void KeyListCtrl::UpdateText()
{
	if (!m_frame)
		return;
	Interpolator* interpolator = m_frame->GetInterpolator();
	if (!interpolator)
		return;

	wxString str;

	for (int i=0; i<interpolator->GetKeyNum(); i++)
	{
		int id = interpolator->GetKeyID(i);
		int time = interpolator->GetKeyTime(i);
		int duration = interpolator->GetKeyDuration(i);
		int interp = interpolator->GetKeyType(i);
		string desc = interpolator->GetKeyDesc(i);
		
        wxString wx_id = wxString::Format("%d", id);
        wxString wx_time = wxString::Format("%d", time);
        wxString wx_duration = wxString::Format("%d", duration);
		SetText(i, 0, wx_id);
		SetText(i, 1, wx_time);
		SetText(i, 2, wx_duration);
		str = interp==0?"Linear":"Smooth";
		SetText(i, 3, str);
		str = desc;
		SetText(i, 4, str);
	}
}

void KeyListCtrl::OnAct(wxListEvent &event)
{
	long item = GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	if (item == -1)
		return;
	wxString str = GetItemText(item);
	long id;
	str.ToLong(&id);

	if (!m_frame || !m_recdlg)
		return;
	Interpolator* interpolator = m_frame->GetInterpolator();
	if (!interpolator)
		return;

	int index = interpolator->GetKeyIndex(int(id));
	double time = interpolator->GetKeyTime(index);
	fluo::Renderview* view = m_recdlg->GetView();
	if (view)
	{
		view->SetParams(time);
		//view->Update(39);
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

void KeyListCtrl::SetText(long item, int col, wxString &str)
{
	wxListItem info;
	info.SetId(item);
	info.SetColumn(col);
	info.SetMask(wxLIST_MASK_TEXT);
	GetItem(info);
	info.SetText(str);
	SetItem(info);
}

void KeyListCtrl::OnSelection(wxListEvent &event)
{
	long item = GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	m_editing_item = item;
	if (item != -1 && m_dragging_to_item==-1)
	{
		wxRect rect;
		wxString str;
		//add frame text
		GetSubItemRect(item, 1, rect);
		str = GetText(item, 1);
		m_frame_text->SetPosition(rect.GetTopLeft());
		m_frame_text->SetSize(rect.GetSize());
		m_frame_text->SetValue(str);
		//m_frame_text->Show();
		//add duration text
		GetSubItemRect(item, 2, rect);
		str = GetText(item, 2);
		m_duration_text->SetPosition(rect.GetTopLeft());
		m_duration_text->SetSize(rect.GetSize());
		m_duration_text->SetValue(str);
		m_duration_text->Show();
		//add interpolation combo
		GetSubItemRect(item, 3, rect);
		str = GetText(item, 3);
		m_interpolation_cmb->SetPosition(rect.GetTopLeft()-wxSize(0,5));
		m_interpolation_cmb->SetSize(wxSize(rect.GetSize().GetWidth(),-1));
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
		m_description_text->SetValue(str);
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

void KeyListCtrl::OnEndSelection(wxListEvent &event)
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

	if (!m_frame)
		return;
	Interpolator* interpolator = m_frame->GetInterpolator();
	if (!interpolator)
		return;

	int index = interpolator->GetKeyIndex(int(id));
	str = m_frame_text->GetValue();
	double time;
	if (str.ToDouble(&time))
	{
		interpolator->ChangeTime(index, time);
	}
}

void KeyListCtrl::OnDurationText(wxCommandEvent& event)
{
	if (m_editing_item == -1)
		return;

	wxString str = GetItemText(m_editing_item);
	long id;
	str.ToLong(&id);

	if (!m_frame)
		return;
	Interpolator* interpolator = m_frame->GetInterpolator();
	if (!interpolator)
		return;

	int index = interpolator->GetKeyIndex(int(id));
	str = m_duration_text->GetValue();
	double duration;
	if (str.ToDouble(&duration))
	{
		interpolator->ChangeDuration(index, duration);
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

	if (!m_frame)
		return;
	Interpolator* interpolator = m_frame->GetInterpolator();
	if (!interpolator)
		return;

	int index = interpolator->GetKeyIndex(int(id));
	FlKeyGroup* keygroup = interpolator->GetKeyGroup(index);
	if (keygroup)
	{
		int sel = m_interpolation_cmb->GetSelection();
		keygroup->type = sel;
		str = sel==0?"Linear":"Smooth";
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

	if (!m_frame)
		return;
	Interpolator* interpolator = m_frame->GetInterpolator();
	if (!interpolator)
		return;

	int index = interpolator->GetKeyIndex(int(id));
	FlKeyGroup* keygroup = interpolator->GetKeyGroup(index);
	if (keygroup)
	{
		str = m_description_text->GetValue();
		keygroup->desc = str.ToStdString();
		SetText(m_editing_item, 4, str);
	}
}

void KeyListCtrl::OnKeyDown(wxKeyEvent& event)
{
	if ( event.GetKeyCode() == WXK_DELETE ||
		event.GetKeyCode() == WXK_BACK)
		DeleteSel();
	event.Skip();
}

void KeyListCtrl::OnKeyUp(wxKeyEvent& event)
{
	event.Skip();
}

void KeyListCtrl::OnBeginDrag(wxListEvent& event)
{
	if (m_editing_item == -1)
		return;

	m_dragging_to_item = -1;
	// trigger when user releases left button (drop)
	Connect(wxEVT_MOTION, wxMouseEventHandler(KeyListCtrl::OnDragging), NULL, this);
	Connect(wxEVT_LEFT_UP, wxMouseEventHandler(KeyListCtrl::OnEndDrag), NULL, this);
	Connect(wxEVT_LEAVE_WINDOW, wxMouseEventHandler(KeyListCtrl::OnEndDrag), NULL,this);
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
	if (index >=0 && index != m_editing_item && index != m_dragging_to_item)
	{
		if (!m_frame)
			return;
		Interpolator* interpolator = m_frame->GetInterpolator();
		if (!interpolator)
			return;

		m_dragging_to_item = index;

		//change the content in the interpolator
		if (m_editing_item > m_dragging_to_item)
			interpolator->MoveKeyBefore(m_editing_item, m_dragging_to_item);
		else
			interpolator->MoveKeyAfter(m_editing_item, m_dragging_to_item);

		DeleteItem(m_editing_item);
		InsertItem(m_dragging_to_item, "", 0);
		UpdateText();

		m_editing_item = m_dragging_to_item;
		SetItemState(m_editing_item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
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
	event.Skip(true);
}

void KeyListCtrl::OnScroll(wxMouseEvent& event)
{
	EndEdit(false);
	event.Skip(true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(RecorderDlg, wxPanel)
	EVT_BUTTON(ID_AutoKeyBtn, RecorderDlg::OnAutoKey)
	EVT_BUTTON(ID_SetKeyBtn, RecorderDlg::OnInsKey)
	EVT_BUTTON(ID_InsKeyBtn, RecorderDlg::OnInsKey)
	EVT_BUTTON(ID_DelKeyBtn, RecorderDlg::OnDelKey)
	EVT_BUTTON(ID_DelAllBtn, RecorderDlg::OnDelAll)
	EVT_CHECKBOX(ID_CamLockChk, RecorderDlg::OnCamLockChk)
	EVT_COMBOBOX(ID_CamLockCmb, RecorderDlg::OnCamLockCmb)
	EVT_BUTTON(ID_CamLockBtn, RecorderDlg::OnCamLockBtn)
END_EVENT_TABLE()

RecorderDlg::RecorderDlg(RenderFrame* frame, wxWindow* parent)
: wxPanel(parent, wxID_ANY,
wxPoint(500, 150), wxSize(450, 650),
0, "RecorderDlg")
{
	m_agent = glbin_agtf->addRecorderAgent(gstRecorderAgent, *this);

	// temporarily block events during constructor:
	wxEventBlocker blocker(this);

	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;
	wxStaticText* st = 0;

	//list
	wxBoxSizer *group2 = new wxBoxSizer(wxVERTICAL);
	st = new wxStaticText(this, wxID_ANY, "Key Frames:");
	m_keylist = new KeyListCtrl(this);
	m_keylist->m_agent = m_agent;
	group2->Add(st, 0, wxEXPAND);
	group2->Add(5, 5);
	group2->Add(m_keylist, 1, wxEXPAND);

	//default duration
	wxBoxSizer *group3 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, wxID_ANY, "Default:",wxDefaultPosition,wxSize(50,-1));
	m_duration_text = new wxTextCtrl(this, ID_DurationText, "30",
		wxDefaultPosition, wxSize(30, 23), 0, vald_int);
	m_interpolation_cmb = new wxComboBox(this, ID_InterpolationCmb, "",
		wxDefaultPosition, wxSize(65,-1), 0, NULL, wxCB_READONLY);
	m_interpolation_cmb->Append("Linear");
	m_interpolation_cmb->Append("Smooth");
	m_interpolation_cmb->Select(0);

	//key buttons
	//wxBoxSizer *group4 = new wxBoxSizer(wxHORIZONTAL);
	m_set_key_btn = new wxButton(this, ID_SetKeyBtn, "Add",
		wxDefaultPosition, wxSize(50, 23));
	m_del_key_btn = new wxButton(this, ID_DelKeyBtn, "Delete",
		wxDefaultPosition, wxSize(55, 23));
	m_del_all_btn = new wxButton(this, ID_DelAllBtn, "Del. All",
		wxDefaultPosition, wxSize(60, 23));

	group3->Add(st, 0, wxALIGN_CENTER);
	group3->Add(5, 5);
	group3->Add(m_duration_text, 0, wxALIGN_CENTER);
	group3->Add(5, 5);
	group3->Add(m_interpolation_cmb, 0, wxALIGN_CENTER);
	group3->AddStretchSpacer(1);
	group3->Add(m_set_key_btn, 0, wxALIGN_CENTER);
	group3->Add(5, 5);
	group3->Add(m_del_key_btn, 0, wxALIGN_CENTER);
	group3->Add(5, 5);
	group3->Add(m_del_all_btn, 0, wxALIGN_CENTER);

	//lock cam center object
	wxBoxSizer *group4 = new wxBoxSizer(wxHORIZONTAL);
	m_cam_lock_chk = new wxCheckBox(this, ID_CamLockChk,
		"Lock View Target:");
	m_cam_lock_cmb = new wxComboBox(this, ID_CamLockCmb, "",
		wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY);
	m_cam_lock_cmb->Append("Image center");
	m_cam_lock_cmb->Append("Click view");
	m_cam_lock_cmb->Append("Ruler");
	m_cam_lock_cmb->Append("Selection");
	m_cam_lock_cmb->Select(0);
	m_cam_lock_btn = new wxButton(this, ID_CamLockBtn, "Apply");
	group4->Add(5, 5);
	group4->Add(m_cam_lock_chk, 0, wxALIGN_CENTER);
	group4->AddStretchSpacer(1);
	group4->Add(m_cam_lock_cmb, 0, wxALIGN_CENTER);
	group4->Add(5, 5);
	group4->Add(m_cam_lock_btn, 0, wxALIGN_CENTER);
	group4->Add(5, 5);

	//all controls
	wxBoxSizer *sizerV = new wxBoxSizer(wxVERTICAL);
	//sizerV->Add(10, 5);
	//sizerV->Add(group1, 0, wxEXPAND);
	sizerV->Add(10, 5);
	sizerV->Add(group2, 1, wxEXPAND);
	sizerV->Add(10, 5);
	sizerV->Add(group3, 0, wxEXPAND);
	sizerV->Add(5, 5);
	sizerV->Add(group4, 0, wxEXPAND);
	sizerV->Add(5, 5);

	SetSizer(sizerV);
	Layout();
}

RecorderDlg::~RecorderDlg()
{
}

void RecorderDlg::OnAutoKey(wxCommandEvent &event)
{
	int sel = m_auto_key_cmb->GetSelection();
	m_agent->AutoKeyChanComb(sel + 1);
}

void RecorderDlg::OnInsKey(wxCommandEvent &event)
{
	m_agent->AddKey();
}

void RecorderDlg::OnDelKey(wxCommandEvent &event)
{
	m_agent->DeleteSel();
}

void RecorderDlg::OnDelAll(wxCommandEvent &event)
{
	m_agent->DeleteAll();
}

//ch1
void RecorderDlg::OnCh1Check(wxCommandEvent &event)
{
	wxCheckBox* ch1 = (wxCheckBox*)event.GetEventObject();
	if (ch1)
		RenderFrame::SetCompression(ch1->GetValue());
}

wxWindow* RecorderDlg::CreateExtraCaptureControl(wxWindow* parent)
{
	wxPanel* panel = new wxPanel(parent);
#ifdef _DARWIN
	panel->SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#endif
	wxBoxSizer *group1 = new wxStaticBoxSizer(
		new wxStaticBox(panel, wxID_ANY, "Additional Options"), wxVERTICAL);

	//compressed
	wxCheckBox* ch1 = new wxCheckBox(panel, ID_LZW_COMP,
		"Lempel-Ziv-Welch Compression");
	ch1->Connect(ch1->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(RecorderDlg::OnCh1Check), NULL, panel);
	if (ch1)
		ch1->SetValue(RenderFrame::GetCompression());

	//group
	group1->Add(10, 10);
	group1->Add(ch1);
	group1->Add(10, 20);

	panel->SetSizerAndFit(group1);
	panel->Layout();

	return panel;
}

void RecorderDlg::OnCamLockChk(wxCommandEvent &event)
{
	bool bval = m_cam_lock_chk->GetValue();
	m_agent->setValue(gstCamLockObjEnable, bval);
}

void RecorderDlg::OnCamLockCmb(wxCommandEvent &event)
{
	long lval = m_cam_lock_cmb->GetSelection() + 1;
	m_agent->setValue(gstCamLockType, lval);
}

void RecorderDlg::OnCamLockBtn(wxCommandEvent &event)
{
	m_agent->getObject()->SetLockCenter();
}
