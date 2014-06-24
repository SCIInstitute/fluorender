#include "MManipulator.h"
#include "VRenderFrame.h"

BEGIN_EVENT_TABLE(MManipulator, wxPanel)
	EVT_SPIN_UP(wxID_ANY, MManipulator::OnSpinUp)
	EVT_SPIN_DOWN(wxID_ANY, MManipulator::OnSpinDown)
	EVT_TEXT_ENTER(wxID_ANY, MManipulator::OnValueEnter)
END_EVENT_TABLE()

MManipulator::MManipulator(wxWindow* frame, wxWindow* parent,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name) :
wxPanel(parent, id, pos, size, style, name),
m_frame(frame),
m_md(0)
{
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizer_3 = new wxBoxSizer(wxHORIZONTAL);

	m_trans_st = new wxStaticText(this, 0, "Translation:");

	m_x_trans_st = new wxStaticText(this, 0, "X:");
	m_x_trans_text = new wxTextCtrl(this, ID_XTransText, "",
		wxDefaultPosition, wxSize(60, 20), wxTE_PROCESS_ENTER);
	m_x_trans_spin = new wxSpinButton(this, ID_XTransSpin);
	m_y_trans_st = new wxStaticText(this, 0, "Y:");
	m_y_trans_text = new wxTextCtrl(this, ID_YTransText, "",
		wxDefaultPosition, wxSize(60, 20), wxTE_PROCESS_ENTER);
	m_y_trans_spin = new wxSpinButton(this, ID_YTransSpin);
	m_z_trans_st = new wxStaticText(this, 0, "Z:");
	m_z_trans_text = new wxTextCtrl(this, ID_ZTransText, "",
		wxDefaultPosition, wxSize(60, 20), wxTE_PROCESS_ENTER);
	m_z_trans_spin = new wxSpinButton(this, ID_ZTransSpin);
	sizer_1->Add(m_x_trans_st, 0, wxALIGN_CENTER);
	sizer_1->Add(m_x_trans_text, 0, wxALIGN_CENTER);
	sizer_1->Add(m_x_trans_spin, 0, wxALIGN_CENTER);
	sizer_1->Add(m_y_trans_st, 0, wxALIGN_CENTER);
	sizer_1->Add(m_y_trans_text, 0, wxALIGN_CENTER);
	sizer_1->Add(m_y_trans_spin, 0, wxALIGN_CENTER);
	sizer_1->Add(m_z_trans_st, 0, wxALIGN_CENTER);
	sizer_1->Add(m_z_trans_text, 0, wxALIGN_CENTER);
	sizer_1->Add(m_z_trans_spin, 0, wxALIGN_CENTER);

	m_rot_st = new wxStaticText(this, 0, "Rotation:");

	m_x_rot_st = new wxStaticText(this, 0, "X:");
	m_x_rot_text = new wxTextCtrl(this, ID_XRotText, "",
		wxDefaultPosition, wxSize(60, 20), wxTE_PROCESS_ENTER);
	m_x_rot_spin = new wxSpinButton(this, ID_XRotSpin);
	m_y_rot_st = new wxStaticText(this, 0, "Y:");
	m_y_rot_text = new wxTextCtrl(this, ID_YRotText, "",
		wxDefaultPosition, wxSize(60, 20), wxTE_PROCESS_ENTER);
	m_y_rot_spin = new wxSpinButton(this, ID_YRotSpin);
	m_z_rot_st = new wxStaticText(this, 0, "Z:");
	m_z_rot_text = new wxTextCtrl(this, ID_ZRotText, "",
		wxDefaultPosition, wxSize(60, 20), wxTE_PROCESS_ENTER);
	m_z_rot_spin = new wxSpinButton(this, ID_ZRotSpin);
	sizer_2->Add(m_x_rot_st, 0, wxALIGN_CENTER);
	sizer_2->Add(m_x_rot_text, 0, wxALIGN_CENTER);
	sizer_2->Add(m_x_rot_spin, 0, wxALIGN_CENTER);
	sizer_2->Add(m_y_rot_st, 0, wxALIGN_CENTER);
	sizer_2->Add(m_y_rot_text, 0, wxALIGN_CENTER);
	sizer_2->Add(m_y_rot_spin, 0, wxALIGN_CENTER);
	sizer_2->Add(m_z_rot_st, 0, wxALIGN_CENTER);
	sizer_2->Add(m_z_rot_text, 0, wxALIGN_CENTER);
	sizer_2->Add(m_z_rot_spin, 0, wxALIGN_CENTER);

	m_scl_st = new wxStaticText(this, 0, "Scaling:");

	m_x_scl_st = new wxStaticText(this, 0, "X:");
	m_x_scl_text = new wxTextCtrl(this, ID_XScalText, "",
		wxDefaultPosition, wxSize(60, 20), wxTE_PROCESS_ENTER);
	m_x_scl_spin = new wxSpinButton(this, ID_XScalSpin);
	m_y_scl_st = new wxStaticText(this, 0, "Y:");
	m_y_scl_text = new wxTextCtrl(this, ID_YScalText, "",
		wxDefaultPosition, wxSize(60, 20), wxTE_PROCESS_ENTER);
	m_y_scl_spin = new wxSpinButton(this, ID_YScalSpin);
	m_z_scl_st = new wxStaticText(this, 0, "Z:");
	m_z_scl_text = new wxTextCtrl(this, ID_ZScalText, "",
		wxDefaultPosition, wxSize(60, 20), wxTE_PROCESS_ENTER);
	m_z_scl_spin = new wxSpinButton(this, ID_ZScalSpin);
	sizer_3->Add(m_x_scl_st, 0, wxALIGN_CENTER);
	sizer_3->Add(m_x_scl_text, 0, wxALIGN_CENTER);
	sizer_3->Add(m_x_scl_spin, 0, wxALIGN_CENTER);
	sizer_3->Add(m_y_scl_st, 0, wxALIGN_CENTER);
	sizer_3->Add(m_y_scl_text, 0, wxALIGN_CENTER);
	sizer_3->Add(m_y_scl_spin, 0, wxALIGN_CENTER);
	sizer_3->Add(m_z_scl_st, 0, wxALIGN_CENTER);
	sizer_3->Add(m_z_scl_text, 0, wxALIGN_CENTER);
	sizer_3->Add(m_z_scl_spin, 0, wxALIGN_CENTER);

	sizer_v->Add(m_trans_st, 0, wxALIGN_LEFT);
	sizer_v->Add(sizer_1, 0, wxALIGN_LEFT);
	sizer_v->Add(m_rot_st, 0, wxALIGN_LEFT);
	sizer_v->Add(sizer_2, 0, wxALIGN_LEFT);
	sizer_v->Add(m_scl_st, 0, wxALIGN_LEFT);
	sizer_v->Add(sizer_3, 0, wxALIGN_LEFT);
	SetSizer(sizer_v);
	Layout();
}

MManipulator::~MManipulator()
{
}

void MManipulator::SetMeshData(MeshData* md)
{
	m_md = md;
}

MeshData* MManipulator::GetMeshData()
{
	return m_md;
}

void MManipulator::RefreshVRenderViews()
{
	VRenderFrame* vrender_frame = (VRenderFrame*)m_frame;
	vrender_frame->RefreshVRenderViews();
}

void MManipulator::GetData()
{
	if (!m_md)
		return;

	double x, y, z;
	wxString str;
	m_md->GetTranslation(x, y, z);
	//sprintf(str, "%.2f", x);
	str = wxString::Format("%.2f", x);
	m_x_trans_text->SetValue(str);
	//sprintf(str, "%.2f", y);
	str = wxString::Format("%.2f", y);
	m_y_trans_text->SetValue(str);
	//sprintf(str, "%.2f", z);
	str = wxString::Format("%.2f", z);
	m_z_trans_text->SetValue(str);
	m_md->GetRotation(x, y, z);
	//sprintf(str, "%.2f", x);
	str = wxString::Format("%.2f", x);
	m_x_rot_text->SetValue(str);
	//sprintf(str, "%.2f", y);
	str = wxString::Format("%.2f", y);
	m_y_rot_text->SetValue(str);
	//sprintf(str, "%.2f", z);
	str = wxString::Format("%.2f", z);
	m_z_rot_text->SetValue(str);
	m_md->GetScaling(x, y, z);
	//sprintf(str, "%.2f", x);
	str = wxString::Format("%.2f", x);
	m_x_scl_text->SetValue(str);
	//sprintf(str, "%.2f", y);
	str = wxString::Format("%.2f", y);
	m_y_scl_text->SetValue(str);
	//sprintf(str, "%.2f", z);
	str = wxString::Format("%.2f", z);
	m_z_scl_text->SetValue(str);
}

void MManipulator::OnSpinUp(wxSpinEvent& event)
{
	int sender_id = event.GetId();
	wxTextCtrl* text_ctrl = 0;
	switch (sender_id)
	{
	case ID_XTransSpin:
		text_ctrl = m_x_trans_text;
		break;
	case ID_YTransSpin:
		text_ctrl = m_y_trans_text;
		break;
	case ID_ZTransSpin:
		text_ctrl = m_z_trans_text;
		break;
	case ID_XRotSpin:
		text_ctrl = m_x_rot_text;
		break;
	case ID_YRotSpin:
		text_ctrl = m_y_rot_text;
		break;
	case ID_ZRotSpin:
		text_ctrl = m_z_rot_text;
		break;
	case ID_XScalSpin:
		text_ctrl = m_x_scl_text;
		break;
	case ID_YScalSpin:
		text_ctrl = m_y_scl_text;
		break;
	case ID_ZScalSpin:
		text_ctrl = m_z_scl_text;
		break;
	}
	if (text_ctrl)
	{
		wxString str_val = text_ctrl->GetValue();
		wxString str = wxString::Format("%.3f", STOD(str_val.fn_str())+1.0);
		text_ctrl->SetValue(str);
		UpdateData();
	}
}

void MManipulator::OnSpinDown(wxSpinEvent& event)
{
	int sender_id = event.GetId();
	wxTextCtrl* text_ctrl = 0;
	switch (sender_id)
	{
	case ID_XTransSpin:
		text_ctrl = m_x_trans_text;
		break;
	case ID_YTransSpin:
		text_ctrl = m_y_trans_text;
		break;
	case ID_ZTransSpin:
		text_ctrl = m_z_trans_text;
		break;
	case ID_XRotSpin:
		text_ctrl = m_x_rot_text;
		break;
	case ID_YRotSpin:
		text_ctrl = m_y_rot_text;
		break;
	case ID_ZRotSpin:
		text_ctrl = m_z_rot_text;
		break;
	case ID_XScalSpin:
		text_ctrl = m_x_scl_text;
		break;
	case ID_YScalSpin:
		text_ctrl = m_y_scl_text;
		break;
	case ID_ZScalSpin:
		text_ctrl = m_z_scl_text;
		break;
	}
	if (text_ctrl)
	{
		wxString str_val = text_ctrl->GetValue();
		wxString str = wxString::Format("%.3f", STOD(str_val.fn_str())-1.0);
		text_ctrl->SetValue(str);
		UpdateData();
	}
}

void MManipulator::UpdateData()
{
	if (!m_md)
		return;

	double x, y, z;
	wxString str = m_x_trans_text->GetValue();
	x = STOD(str.fn_str());
	str = m_y_trans_text->GetValue();
	y = STOD(str.fn_str());
	str = m_z_trans_text->GetValue();
	z = STOD(str.fn_str());
	m_md->SetTranslation(x, y, z);

	str = m_x_rot_text->GetValue();
	x = STOD(str.fn_str());
	str = m_y_rot_text->GetValue();
	y = STOD(str.fn_str());
	str = m_z_rot_text->GetValue();
	z = STOD(str.fn_str());
	m_md->SetRotation(x, y, z);

	str = m_x_scl_text->GetValue();
	x = STOD(str.fn_str());
	str = m_y_scl_text->GetValue();
	y = STOD(str.fn_str());
	str = m_z_scl_text->GetValue();
	z = STOD(str.fn_str());
	m_md->SetScaling(x, y, z);

	RefreshVRenderViews();
}

void MManipulator::OnValueEnter(wxCommandEvent& event)
{
	int sender_id = event.GetId();
	wxTextCtrl* text_ctrl = 0;
	switch (sender_id)
	{
	case ID_XTransText:
		text_ctrl = m_x_trans_text;
		break;
	case ID_YTransText:
		text_ctrl = m_y_trans_text;
		break;
	case ID_ZTransText:
		text_ctrl = m_z_trans_text;
		break;
	case ID_XRotText:
		text_ctrl = m_x_rot_text;
		break;
	case ID_YRotText:
		text_ctrl = m_y_rot_text;
		break;
	case ID_ZRotText:
		text_ctrl = m_z_rot_text;
		break;
	case ID_XScalText:
		text_ctrl = m_x_scl_text;
		break;
	case ID_YScalText:
		text_ctrl = m_y_scl_text;
		break;
	case ID_ZScalText:
		text_ctrl = m_z_scl_text;
		break;
	}

	if (text_ctrl)
	{
		wxString str_val = text_ctrl->GetValue();
		wxString str = wxString::Format("%.3f", STOD(str_val.fn_str()));
		text_ctrl->SetValue(str);
		UpdateData();
	}
}

