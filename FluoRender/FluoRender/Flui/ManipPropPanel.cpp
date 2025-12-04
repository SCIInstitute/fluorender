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
#include <ManipPropPanel.h>
#include <Global.h>
#include <Names.h>
#include <MainFrame.h>
#include <MeshData.h>
#include <compatibility.h>

ManipPropPanel::ManipPropPanel(MainFrame* frame,
	wxWindow* parent,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name) :
	PropPanel(frame, parent, pos, size, style, name),
	m_md(0)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);
	SetDoubleBuffered(true);

	wxBoxSizer* sizer_v = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizer_3 = new wxBoxSizer(wxHORIZONTAL);

	m_trans_st = new wxStaticText(this, 0, "Translation:");

	m_x_trans_st = new wxStaticText(this, 0, "X:");
	m_x_trans_text = new wxTextCtrl(this, ID_XTransText, "",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT);
	m_x_trans_text->Bind(wxEVT_TEXT, &ManipPropPanel::OnValueEnter, this);
	m_x_trans_spin = new wxSpinButton(this, ID_XTransSpin);
	m_x_trans_spin->SetRange(-0x8000, 0x7fff);
	m_x_trans_spin->Bind(wxEVT_SPIN_UP, &ManipPropPanel::OnSpinUp, this);
	m_x_trans_spin->Bind(wxEVT_SPIN_DOWN, &ManipPropPanel::OnSpinDown, this);
	m_y_trans_st = new wxStaticText(this, 0, "Y:");
	m_y_trans_text = new wxTextCtrl(this, ID_YTransText, "",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT);
	m_y_trans_text->Bind(wxEVT_TEXT, &ManipPropPanel::OnValueEnter, this);
	m_y_trans_spin = new wxSpinButton(this, ID_YTransSpin);
	m_y_trans_spin->SetRange(-0x8000, 0x7fff);
	m_y_trans_spin->Bind(wxEVT_SPIN_UP, &ManipPropPanel::OnSpinUp, this);
	m_y_trans_spin->Bind(wxEVT_SPIN_DOWN, &ManipPropPanel::OnSpinDown, this);
	m_z_trans_st = new wxStaticText(this, 0, "Z:");
	m_z_trans_text = new wxTextCtrl(this, ID_ZTransText, "",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT);
	m_z_trans_text->Bind(wxEVT_TEXT, &ManipPropPanel::OnValueEnter, this);
	m_z_trans_spin = new wxSpinButton(this, ID_ZTransSpin);
	m_z_trans_spin->SetRange(-0x8000, 0x7fff);
	m_z_trans_spin->Bind(wxEVT_SPIN_UP, &ManipPropPanel::OnSpinUp, this);
	m_z_trans_spin->Bind(wxEVT_SPIN_DOWN, &ManipPropPanel::OnSpinDown, this);
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
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT);
	m_x_rot_text->Bind(wxEVT_TEXT, &ManipPropPanel::OnValueEnter, this);
	m_x_rot_spin = new wxSpinButton(this, ID_XRotSpin);
	m_x_rot_spin->SetRange(-0x8000, 0x7fff);
	m_x_rot_spin->Bind(wxEVT_SPIN_UP, &ManipPropPanel::OnSpinUp, this);
	m_x_rot_spin->Bind(wxEVT_SPIN_DOWN, &ManipPropPanel::OnSpinDown, this);
	m_y_rot_st = new wxStaticText(this, 0, "Y:");
	m_y_rot_text = new wxTextCtrl(this, ID_YRotText, "",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT);
	m_y_rot_text->Bind(wxEVT_TEXT, &ManipPropPanel::OnValueEnter, this);
	m_y_rot_spin = new wxSpinButton(this, ID_YRotSpin);
	m_y_rot_spin->SetRange(-0x8000, 0x7fff);
	m_y_rot_spin->Bind(wxEVT_SPIN_UP, &ManipPropPanel::OnSpinUp, this);
	m_y_rot_spin->Bind(wxEVT_SPIN_DOWN, &ManipPropPanel::OnSpinDown, this);
	m_z_rot_st = new wxStaticText(this, 0, "Z:");
	m_z_rot_text = new wxTextCtrl(this, ID_ZRotText, "",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT);
	m_z_rot_text->Bind(wxEVT_TEXT, &ManipPropPanel::OnValueEnter, this);
	m_z_rot_spin = new wxSpinButton(this, ID_ZRotSpin);
	m_z_rot_spin->SetRange(-0x8000, 0x7fff);
	m_z_rot_spin->Bind(wxEVT_SPIN_UP, &ManipPropPanel::OnSpinUp, this);
	m_z_rot_spin->Bind(wxEVT_SPIN_DOWN, &ManipPropPanel::OnSpinDown, this);
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
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT);
	m_x_scl_text->Bind(wxEVT_TEXT, &ManipPropPanel::OnValueEnter, this);
	m_x_scl_spin = new wxSpinButton(this, ID_XScalSpin);
	m_x_scl_spin->SetRange(-0x8000, 0x7fff);
	m_x_scl_spin->Bind(wxEVT_SPIN_UP, &ManipPropPanel::OnSpinUp, this);
	m_x_scl_spin->Bind(wxEVT_SPIN_DOWN, &ManipPropPanel::OnSpinDown, this);
	m_y_scl_st = new wxStaticText(this, 0, "Y:");
	m_y_scl_text = new wxTextCtrl(this, ID_YScalText, "",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT);
	m_y_scl_text->Bind(wxEVT_TEXT, &ManipPropPanel::OnValueEnter, this);
	m_y_scl_spin = new wxSpinButton(this, ID_YScalSpin);
	m_y_scl_spin->SetRange(-0x8000, 0x7fff);
	m_y_scl_spin->Bind(wxEVT_SPIN_UP, &ManipPropPanel::OnSpinUp, this);
	m_y_scl_spin->Bind(wxEVT_SPIN_DOWN, &ManipPropPanel::OnSpinDown, this);
	m_z_scl_st = new wxStaticText(this, 0, "Z:");
	m_z_scl_text = new wxTextCtrl(this, ID_ZScalText, "",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT);
	m_z_scl_text->Bind(wxEVT_TEXT, &ManipPropPanel::OnValueEnter, this);
	m_z_scl_spin = new wxSpinButton(this, ID_ZScalSpin);
	m_z_scl_spin->SetRange(-0x8000, 0x7fff);
	m_z_scl_spin->Bind(wxEVT_SPIN_UP, &ManipPropPanel::OnSpinUp, this);
	m_z_scl_spin->Bind(wxEVT_SPIN_DOWN, &ManipPropPanel::OnSpinDown, this);
	sizer_3->Add(m_x_scl_st, 0, wxALIGN_CENTER);
	sizer_3->Add(m_x_scl_text, 0, wxALIGN_CENTER);
	sizer_3->Add(m_x_scl_spin, 0, wxALIGN_CENTER);
	sizer_3->Add(m_y_scl_st, 0, wxALIGN_CENTER);
	sizer_3->Add(m_y_scl_text, 0, wxALIGN_CENTER);
	sizer_3->Add(m_y_scl_spin, 0, wxALIGN_CENTER);
	sizer_3->Add(m_z_scl_st, 0, wxALIGN_CENTER);
	sizer_3->Add(m_z_scl_text, 0, wxALIGN_CENTER);
	sizer_3->Add(m_z_scl_spin, 0, wxALIGN_CENTER);

	sizer_v->Add(10, 10);
	sizer_v->Add(m_trans_st, 0, wxALIGN_CENTER);
	sizer_v->Add(sizer_1, 1, wxEXPAND);
	sizer_v->Add(10, 10);
	sizer_v->Add(m_rot_st, 0, wxALIGN_CENTER);
	sizer_v->Add(sizer_2, 1, wxEXPAND);
	sizer_v->Add(10, 10);
	sizer_v->Add(m_scl_st, 0, wxALIGN_CENTER);
	sizer_v->Add(sizer_3, 1, wxEXPAND);

	SetSizer(sizer_v);
	Layout();
	SetScrollRate(10, 10);
}

ManipPropPanel::~ManipPropPanel()
{
}

void ManipPropPanel::FluoUpdate(const fluo::ValueCollection& vc)
{
	if (!m_md)
		return;

	//update user interface
	if (FOUND_VALUE(gstNull))
		return;
	bool update_all = vc.empty();

	wxString str;
	fluo::Vector vval;

	if (update_all || FOUND_VALUE(gstMeshTranslation))
	{
		vval = m_md->GetTranslation();
		str = wxString::Format("%.2f", vval.x());
		m_x_trans_text->ChangeValue(str);
		str = wxString::Format("%.2f", vval.y());
		m_y_trans_text->ChangeValue(str);
		str = wxString::Format("%.2f", vval.z());
		m_z_trans_text->ChangeValue(str);
	}
	if (update_all || FOUND_VALUE(gstMeshRotation))
	{
		vval = m_md->GetRotation();
		str = wxString::Format("%.2f", vval.x());
		m_x_rot_text->ChangeValue(str);
		str = wxString::Format("%.2f", vval.y());
		m_y_rot_text->ChangeValue(str);
		str = wxString::Format("%.2f", vval.z());
		m_z_rot_text->ChangeValue(str);
	}
	if (update_all || FOUND_VALUE(gstMeshScale))
	{
		vval = m_md->GetScaling();
		str = wxString::Format("%.2f", vval.x());
		m_x_scl_text->ChangeValue(str);
		str = wxString::Format("%.2f", vval.y());
		m_y_scl_text->ChangeValue(str);
		str = wxString::Format("%.2f", vval.z());
		m_z_scl_text->ChangeValue(str);
	}
}

void ManipPropPanel::SetMeshData(MeshData* md)
{
	m_md = md;
}

MeshData* ManipPropPanel::GetMeshData()
{
	return m_md;
}

void ManipPropPanel::OnSpinUp(wxSpinEvent& event)
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
		double dval;
		str_val.ToDouble(&dval);
		dval += 1.0;
		wxString str = wxString::Format("%.3f", dval);
		text_ctrl->ChangeValue(str);
		UpdateMeshData();
	}
}

void ManipPropPanel::OnSpinDown(wxSpinEvent& event)
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
		double dval;
		str_val.ToDouble(&dval);
		dval -= 1.0;
		wxString str = wxString::Format("%.3f", dval);
		text_ctrl->ChangeValue(str);
		UpdateMeshData();
	}
}

void ManipPropPanel::UpdateMeshData()
{
	if (!m_md)
		return;

	double x, y, z;
	wxString str = m_x_trans_text->GetValue();
	str.ToDouble(&x);
	str = m_y_trans_text->GetValue();
	str.ToDouble(&y);
	str = m_z_trans_text->GetValue();
	str.ToDouble(&z);
	m_md->SetTranslation(fluo::Vector(x, y, z));

	str = m_x_rot_text->GetValue();
	str.ToDouble(&x);
	str = m_y_rot_text->GetValue();
	str.ToDouble(&y);
	str = m_z_rot_text->GetValue();
	str.ToDouble(&z);
	m_md->SetRotation(fluo::Vector(x, y, z));

	str = m_x_scl_text->GetValue();
	str.ToDouble(&x);
	str = m_y_scl_text->GetValue();
	str.ToDouble(&y);
	str = m_z_scl_text->GetValue();
	str.ToDouble(&z);
	m_md->SetScaling(fluo::Vector(x, y, z));

	FluoRefresh(1, { gstNull });
}

void ManipPropPanel::OnValueEnter(wxCommandEvent& event)
{
	UpdateMeshData();
}

