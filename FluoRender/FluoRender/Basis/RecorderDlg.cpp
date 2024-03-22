/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2018 Scientific Computing and Imaging Institute,
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
#include <RecorderDlg.h>
#include <Global.h>
#include <MainFrame.h>
#include <RenderCanvas.h>
#include <RenderViewPanel.h>
#include <wx/artprov.h>
#include <wx/valnum.h>
#include "key.xpm"

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
	MainFrame* frame,
	RecorderDlg* parent,
	const wxPoint& pos,
	const wxSize& size,
	long style) :
wxListCtrl(parent, wxID_ANY, pos, size, style),
m_frame(frame),
m_recdlg(parent),
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
	for (int i=0; i<glbin_interpolator.GetKeyNum(); i++)
	{
		int id = glbin_interpolator.GetKeyID(i);
		int time = glbin_interpolator.GetKeyTime(i);
		int duration = glbin_interpolator.GetKeyDuration(i);
		int interp = glbin_interpolator.GetKeyType(i);
		string desc = glbin_interpolator.GetKeyDesc(i);
		Append(id, time, duration, interp, desc);
	}
}

void KeyListCtrl::UpdateText()
{
	wxString str;

	for (int i=0; i<glbin_interpolator.GetKeyNum(); i++)
	{
		int id = glbin_interpolator.GetKeyID(i);
		int time = glbin_interpolator.GetKeyTime(i);
		int duration = glbin_interpolator.GetKeyDuration(i);
		int interp = glbin_interpolator.GetKeyType(i);
		string desc = glbin_interpolator.GetKeyDesc(i);
		
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

	int index = glbin_interpolator.GetKeyIndex(int(id));
	double time = glbin_interpolator.GetKeyTime(index);
	if (!m_recdlg)
		return;
	RenderCanvas* view = m_recdlg->GetView();
	if (view)
	{
		view->SetParams(time);
		view->RefreshGL(39);
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
		m_interpolation_cmb->SetPosition(rect.GetTopLeft()-FromDIP(wxSize(0,5)));
		m_interpolation_cmb->SetSize(FromDIP(wxSize(rect.GetSize().GetWidth(),-1)));
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

	int index = glbin_interpolator.GetKeyIndex(int(id));
	FlKeyGroup* keygroup = glbin_interpolator.GetKeyGroup(index);
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

BEGIN_EVENT_TABLE(RecorderDlg, wxScrolledWindow)
	EVT_BUTTON(ID_AutoKeyBtn, RecorderDlg::OnAutoKey)
	EVT_BUTTON(ID_SetKeyBtn, RecorderDlg::OnInsKey)
	EVT_BUTTON(ID_InsKeyBtn, RecorderDlg::OnInsKey)
	EVT_BUTTON(ID_DelKeyBtn, RecorderDlg::OnDelKey)
	EVT_BUTTON(ID_DelAllBtn, RecorderDlg::OnDelAll)
	EVT_CHECKBOX(ID_CamLockChk, RecorderDlg::OnCamLockChk)
	EVT_COMBOBOX(ID_CamLockCmb, RecorderDlg::OnCamLockCmb)
	EVT_BUTTON(ID_CamLockBtn, RecorderDlg::OnCamLockBtn)
END_EVENT_TABLE()

RecorderDlg::RecorderDlg(MainFrame* frame, wxWindow* parent)
: wxScrolledWindow(parent, wxID_ANY,
wxDefaultPosition,
frame->FromDIP(wxSize(450, 650)),
0, "RecorderDlg"),
m_frame(frame),
m_view(0),
m_cam_lock(false),
m_cam_lock_type(0)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);

	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;
	wxStaticText* st = 0;

	//list
	wxBoxSizer *group2 = new wxBoxSizer(wxVERTICAL);
	st = new wxStaticText(this, wxID_ANY, "Key Frames:");
	m_keylist = new KeyListCtrl(frame, this);
	group2->Add(st, 0, wxEXPAND);
	group2->Add(5, 5);
	group2->Add(m_keylist, 1, wxEXPAND);

	//default duration
	wxBoxSizer *group3 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, wxID_ANY, "Default:",wxDefaultPosition,FromDIP(wxSize(50,-1)));
	m_duration_text = new wxTextCtrl(this, ID_DurationText, "30",
		wxDefaultPosition, FromDIP(wxSize(30, 23)), 0, vald_int);
	m_interpolation_cmb = new wxComboBox(this, ID_InterpolationCmb, "",
		wxDefaultPosition, FromDIP(wxSize(65,-1)), 0, NULL, wxCB_READONLY);
	m_interpolation_cmb->Append("Linear");
	m_interpolation_cmb->Append("Smooth");
	m_interpolation_cmb->Select(0);

	//key buttons
	//wxBoxSizer *group4 = new wxBoxSizer(wxHORIZONTAL);
	m_set_key_btn = new wxButton(this, ID_SetKeyBtn, "Add",
		wxDefaultPosition, FromDIP(wxSize(50, 23)));
	m_del_key_btn = new wxButton(this, ID_DelKeyBtn, "Delete",
		wxDefaultPosition, FromDIP(wxSize(55, 23)));
	m_del_all_btn = new wxButton(this, ID_DelAllBtn, "Del. All",
		wxDefaultPosition, FromDIP(wxSize(60, 23)));

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
	SetAutoLayout(true);
	SetScrollRate(10, 10);
}

RecorderDlg::~RecorderDlg()
{
}

void RecorderDlg::GetSettings(RenderCanvas* view)
{
	m_view = view;
}

void RecorderDlg::SetSelection(int index)
{
	if (m_keylist)
	{
		long item = m_keylist->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
		if (index != item && item != -1)
			m_keylist->SetItemState(index,
			wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
	}
}

void RecorderDlg::OnAutoKey(wxCommandEvent &event)
{
	int sel = m_auto_key_cmb->GetSelection();

	switch (sel)
	{
	case 0://isolations
		AutoKeyChanComb(1);
		break;
	case 1://combination of two
		AutoKeyChanComb(2);
		break;
	case 2://combination of three
		AutoKeyChanComb(3);
		break;
	case 3://combination of all
		break;
	}

}

void RecorderDlg::OnSetKey(wxCommandEvent &event)
{
	wxString str = m_duration_text->GetValue();
	double duration;
	str.ToDouble(&duration);
	int interpolation = m_interpolation_cmb->GetSelection();
	InsertKey(-1, duration, interpolation);

	m_keylist->Update();
}

void RecorderDlg::OnInsKey(wxCommandEvent &event)
{
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
	//check if 4D
	bool is_4d = false;
	VolumeData* vd = 0;
	for (int i = 0; i < glbin_data_manager.GetVolumeNum(); i++)
	{
		vd = glbin_data_manager.GetVolumeData(i);
		if (vd->GetReader() &&
			vd->GetReader()->GetTimeNum() > 1)
		{
			is_4d = true;
			break;
		}
	}
	double duration = 0.0;
	if (is_4d)
	{
		//Interpolator *interpolator = m_frame->GetInterpolator();
		//if (interpolator && m_view)
		//{
		//	double ct = vd->GetCurTime();
		//	FlKeyCode keycode;
		//	keycode.l0 = 1;
		//	keycode.l0_name = m_view->m_vrv->GetName();
		//	keycode.l1 = 2;
		//	keycode.l1_name = vd->GetName();
		//	keycode.l2 = 0;
		//	keycode.l2_name = "frame";
		//	double frame;
		//	if (glbin_interpolator.GetDouble(keycode, 
		//		glbin_interpolator.GetLastIndex(), frame))
		//		duration = fabs(ct - frame);
		//}
	}
	str = m_duration_text->GetValue();
	str.ToDouble(&duration);
	int interpolation = m_interpolation_cmb->GetSelection();
	InsertKey(index, duration, interpolation);

	m_keylist->Update();
	m_keylist->SetItemState(item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
}

void RecorderDlg::InsertKey(int index, double duration, int interpolation)
{
	if (!m_frame)
		return;
	if (!m_view)
	{
		if (m_frame->GetView(0))
			m_view = m_frame->GetView(0);
		else
			return;
	}

	FlKeyCode keycode;
	FlKeyDouble* flkey = 0;
	FlKeyQuaternion* flkeyQ = 0;
	FlKeyBoolean* flkeyB = 0;
	FlKeyInt* flkeyI = 0;
	FlKeyColor* flkeyC = 0;

	double t = glbin_interpolator.GetLastT();
	t = t<0.0?0.0:t+duration;

	glbin_interpolator.Begin(t, duration);

	//for all volumes
	for (int i=0; i< glbin_data_manager.GetVolumeNum() ; i++)
	{
		VolumeData* vd = glbin_data_manager.GetVolumeData(i);
		keycode.l0 = 1;
		keycode.l0_name = m_view->m_vrv->GetName();
		keycode.l1 = 2;
		keycode.l1_name = vd->GetName();
		//display
		keycode.l2 = 0;
		keycode.l2_name = "display";
		flkeyB = new FlKeyBoolean(keycode, vd->GetDisp());
		glbin_interpolator.AddKey(flkeyB);
		//clipping planes
		vector<fluo::Plane*> * planes = vd->GetVR()->get_planes();
		if (!planes)
			continue;
		if (planes->size() != 6)
			continue;
		fluo::Plane* plane = 0;
		double abcd[4];
		//x1
		plane = (*planes)[0];
		plane->get_copy(abcd);
		keycode.l2 = 0;
		keycode.l2_name = "x1_val";
		flkey = new FlKeyDouble(keycode, abs(abcd[3]));
		glbin_interpolator.AddKey(flkey);
		//x2
		plane = (*planes)[1];
		plane->get_copy(abcd);
		keycode.l2 = 0;
		keycode.l2_name = "x2_val";
		flkey = new FlKeyDouble(keycode, abs(abcd[3]));
		glbin_interpolator.AddKey(flkey);
		//y1
		plane = (*planes)[2];
		plane->get_copy(abcd);
		keycode.l2 = 0;
		keycode.l2_name = "y1_val";
		flkey = new FlKeyDouble(keycode, abs(abcd[3]));
		glbin_interpolator.AddKey(flkey);
		//y2
		plane = (*planes)[3];
		plane->get_copy(abcd);
		keycode.l2 = 0;
		keycode.l2_name = "y2_val";
		flkey = new FlKeyDouble(keycode, abs(abcd[3]));
		glbin_interpolator.AddKey(flkey);
		//z1
		plane = (*planes)[4];
		plane->get_copy(abcd);
		keycode.l2 = 0;
		keycode.l2_name = "z1_val";
		flkey = new FlKeyDouble(keycode, abs(abcd[3]));
		glbin_interpolator.AddKey(flkey);
		//z2
		plane = (*planes)[5];
		plane->get_copy(abcd);
		keycode.l2 = 0;
		keycode.l2_name = "z2_val";
		flkey = new FlKeyDouble(keycode, abs(abcd[3]));
		glbin_interpolator.AddKey(flkey);
		//t
		int frame = vd->GetCurTime();
		keycode.l2 = 0;
		keycode.l2_name = "frame";
		flkey = new FlKeyDouble(keycode, frame);
		glbin_interpolator.AddKey(flkey);
		//primary color
		fluo::Color pc = vd->GetColor();
		keycode.l2 = 0;
		keycode.l2_name = "color";
		flkeyC = new FlKeyColor(keycode, pc);
		glbin_interpolator.AddKey(flkeyC);
	}
	//for the view
	keycode.l0 = 1;
	keycode.l0_name = m_view->m_vrv->GetName();
	keycode.l1 = 1;
	keycode.l1_name = m_view->m_vrv->GetName();
	//rotation
	keycode.l2 = 0;
	keycode.l2_name = "rotation";
	fluo::Quaternion q = m_view->GetRotations();
	flkeyQ = new FlKeyQuaternion(keycode, q);
	glbin_interpolator.AddKey(flkeyQ);
	//translation
	double tx, ty, tz;
	m_view->GetTranslations(tx, ty, tz);
	//x
	keycode.l2_name = "translation_x";
	flkey = new FlKeyDouble(keycode, tx);
	glbin_interpolator.AddKey(flkey);
	//y
	keycode.l2_name = "translation_y";
	flkey = new FlKeyDouble(keycode, ty);
	glbin_interpolator.AddKey(flkey);
	//z
	keycode.l2_name = "translation_z";
	flkey = new FlKeyDouble(keycode, tz);
	glbin_interpolator.AddKey(flkey);
	//centers
	m_view->GetCenters(tx, ty, tz);
	//x
	keycode.l2_name = "center_x";
	flkey = new FlKeyDouble(keycode, tx);
	glbin_interpolator.AddKey(flkey);
	//y
	keycode.l2_name = "center_y";
	flkey = new FlKeyDouble(keycode, ty);
	glbin_interpolator.AddKey(flkey);
	//z
	keycode.l2_name = "center_z";
	flkey = new FlKeyDouble(keycode, tz);
	glbin_interpolator.AddKey(flkey);
	//obj traslation
	m_view->GetObjTrans(tx, ty, tz);
	//x
	keycode.l2_name = "obj_trans_x";
	flkey = new FlKeyDouble(keycode, tx);
	glbin_interpolator.AddKey(flkey);
	//y
	keycode.l2_name = "obj_trans_y";
	flkey = new FlKeyDouble(keycode, ty);
	glbin_interpolator.AddKey(flkey);
	//z
	keycode.l2_name = "obj_trans_z";
	flkey = new FlKeyDouble(keycode, tz);
	glbin_interpolator.AddKey(flkey);
	//scale
	double scale = m_view->m_scale_factor;
	keycode.l2_name = "scale";
	flkey = new FlKeyDouble(keycode, scale);
	glbin_interpolator.AddKey(flkey);
	//intermixing mode
	int ival = m_view->GetVolMethod();
	keycode.l2_name = "volmethod";
	flkeyI = new FlKeyInt(keycode, ival);
	glbin_interpolator.AddKey(flkeyI);
	//perspective angle
	bool persp = m_view->GetPersp();
	double aov = m_view->GetAov();
	if (!persp)
		aov = 9.9;
	keycode.l2_name = "aov";
	flkey = new FlKeyDouble(keycode, aov);
	glbin_interpolator.AddKey(flkey);

	glbin_interpolator.End();

	FlKeyGroup* group = glbin_interpolator.GetKeyGroup(glbin_interpolator.GetLastIndex());
	if (group)
		group->type = interpolation;
}

bool RecorderDlg::MoveOne(vector<bool>& chan_mask, int lv)
{
	int i;
	int cur_lv = 0;
	int lv_pos = -1;
	for (i=(int)chan_mask.size()-1; i>=0; i--)
	{
		if (chan_mask[i])
		{
			cur_lv++;
			if (cur_lv == lv)
			{
				lv_pos = i;
				break;
			}
		}
	}
	if (lv_pos >= 0)
	{
		if (lv_pos == (int)chan_mask.size()-lv)
			return MoveOne(chan_mask, ++lv);
		else
		{
			if (!chan_mask[lv_pos+1])
			{
				for (i=lv_pos; i<(int)chan_mask.size(); i++)
				{
					if (i==lv_pos)
						chan_mask[i] = false;
					else if (i<=lv_pos+lv)
						chan_mask[i] = true;
					else
						chan_mask[i] = false;
				}
				return true;
			}
			else return false;//no space anymore
		}
	}
	else return false;
}

bool RecorderDlg::GetMask(vector<bool>& chan_mask)
{
	return MoveOne(chan_mask, 1);
}

void RecorderDlg::AutoKeyChanComb(int comb)
{
	if (!m_frame)
		return;
	if (!m_view)
	{
		if (m_frame->GetView(0))
			m_view = m_frame->GetView(0);
		else
			return;
	}

	wxString str = m_duration_text->GetValue();
	double duration;
	str.ToDouble(&duration);

	FlKeyCode keycode;
	FlKeyBoolean* flkeyB = 0;

	double t = glbin_interpolator.GetLastT();
	t = t<0.0?0.0:t;
	if (t>0.0) t += duration;

	int i;
	int numChan = m_view->GetAllVolumeNum();
	vector<bool> chan_mask;
	//initiate mask
	for (i=0; i<numChan; i++)
	{
		if (i < comb)
			chan_mask.push_back(true);
		else
			chan_mask.push_back(false);
	}

	do
	{
		glbin_interpolator.Begin(t, duration);

		//for all volumes
		for (i=0; i<m_view->GetAllVolumeNum(); i++)
		{
			VolumeData* vd = m_view->GetAllVolumeData(i);
			keycode.l0 = 1;
			keycode.l0_name = m_view->m_vrv->GetName();
			keycode.l1 = 2;
			keycode.l1_name = vd->GetName();
			//display only
			keycode.l2 = 0;
			keycode.l2_name = "display";
			flkeyB = new FlKeyBoolean(keycode, chan_mask[i]);
			glbin_interpolator.AddKey(flkeyB);
		}

		glbin_interpolator.End();
		t += duration;
	} while (GetMask(chan_mask));

	m_keylist->Update();
}

void RecorderDlg::OnDelKey(wxCommandEvent &event)
{
	m_keylist->DeleteSel();
}

void RecorderDlg::OnDelAll(wxCommandEvent &event)
{
	m_keylist->DeleteAll();
}

//ch1
void RecorderDlg::OnCh1Check(wxCommandEvent &event)
{
	wxCheckBox* ch1 = (wxCheckBox*)event.GetEventObject();
	if (ch1)
		glbin_settings.m_save_compress = ch1->GetValue();
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
	wxCheckBox* ch1 = new wxCheckBox(panel, wxID_ANY,
		"Lempel-Ziv-Welch Compression");
	ch1->Connect(ch1->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(RecorderDlg::OnCh1Check), NULL, panel);
	if (ch1)
		ch1->SetValue(glbin_settings.m_save_compress);

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
	m_cam_lock = m_cam_lock_chk->GetValue();
}

void RecorderDlg::OnCamLockCmb(wxCommandEvent &event)
{
	m_cam_lock_type = m_cam_lock_cmb->GetSelection() + 1;
}

void RecorderDlg::OnCamLockBtn(wxCommandEvent &event)
{
	m_view->SetLockCenter(m_cam_lock_type);
}
