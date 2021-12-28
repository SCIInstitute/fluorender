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
#include "MManipulator.h"
#include "VRenderFrame.h"
#include <MeshData.hpp>
#include "compatibility.h"

BEGIN_EVENT_TABLE(MManipulator, wxPanel)
	EVT_SPIN_UP(wxID_ANY, MManipulator::OnSpinUp)
	EVT_SPIN_DOWN(wxID_ANY, MManipulator::OnSpinDown)
	EVT_TEXT_ENTER(wxID_ANY, MManipulator::OnValueEnter)
END_EVENT_TABLE()

MManipulator::MManipulator(VRenderFrame* frame,
	wxWindow* parent,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name) :
	wxPanel(parent, wxID_ANY, pos, size, style, name),
	m_frame(frame),
	m_md(0)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);

	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizer_3 = new wxBoxSizer(wxHORIZONTAL);

	m_trans_st = new wxStaticText(this, 0, "Translation:");

	m_x_trans_st = new wxStaticText(this, 0, "X:");
	m_x_trans_text = new wxTextCtrl(this, ID_XTransText, "",
		wxDefaultPosition, wxSize(60, 20), wxTE_PROCESS_ENTER);
	m_x_trans_spin = new wxSpinButton(this, ID_XTransSpin);
	m_x_trans_spin->SetRange(-0x8000, 0x7fff);
	m_y_trans_st = new wxStaticText(this, 0, "Y:");
	m_y_trans_text = new wxTextCtrl(this, ID_YTransText, "",
		wxDefaultPosition, wxSize(60, 20), wxTE_PROCESS_ENTER);
	m_y_trans_spin = new wxSpinButton(this, ID_YTransSpin);
	m_y_trans_spin->SetRange(-0x8000, 0x7fff);
	m_z_trans_st = new wxStaticText(this, 0, "Z:");
	m_z_trans_text = new wxTextCtrl(this, ID_ZTransText, "",
		wxDefaultPosition, wxSize(60, 20), wxTE_PROCESS_ENTER);
	m_z_trans_spin = new wxSpinButton(this, ID_ZTransSpin);
	m_z_trans_spin->SetRange(-0x8000, 0x7fff);
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
	m_x_rot_spin->SetRange(-0x8000, 0x7fff);
	m_y_rot_st = new wxStaticText(this, 0, "Y:");
	m_y_rot_text = new wxTextCtrl(this, ID_YRotText, "",
		wxDefaultPosition, wxSize(60, 20), wxTE_PROCESS_ENTER);
	m_y_rot_spin = new wxSpinButton(this, ID_YRotSpin);
	m_y_rot_spin->SetRange(-0x8000, 0x7fff);
	m_z_rot_st = new wxStaticText(this, 0, "Z:");
	m_z_rot_text = new wxTextCtrl(this, ID_ZRotText, "",
		wxDefaultPosition, wxSize(60, 20), wxTE_PROCESS_ENTER);
	m_z_rot_spin = new wxSpinButton(this, ID_ZRotSpin);
	m_z_rot_spin->SetRange(-0x8000, 0x7fff);
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
	m_x_scl_spin->SetRange(-0x8000, 0x7fff);
	m_y_scl_st = new wxStaticText(this, 0, "Y:");
	m_y_scl_text = new wxTextCtrl(this, ID_YScalText, "",
		wxDefaultPosition, wxSize(60, 20), wxTE_PROCESS_ENTER);
	m_y_scl_spin = new wxSpinButton(this, ID_YScalSpin);
	m_y_scl_spin->SetRange(-0x8000, 0x7fff);
	m_z_scl_st = new wxStaticText(this, 0, "Z:");
	m_z_scl_text = new wxTextCtrl(this, ID_ZScalText, "",
		wxDefaultPosition, wxSize(60, 20), wxTE_PROCESS_ENTER);
	m_z_scl_spin = new wxSpinButton(this, ID_ZScalSpin);
	m_z_scl_spin->SetRange(-0x8000, 0x7fff);
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

void MManipulator::SetMeshData(fluo::MeshData* md)
{
	m_md = md;
}

fluo::MeshData* MManipulator::GetMeshData()
{
	return m_md;
}

void MManipulator::RefreshVRenderViews()
{
	if (m_frame)
		m_frame->RefreshVRenderViews();
}

void MManipulator::GetData()
{
	if (!m_md)
		return;

	wxString str;
	double x, y, z;
	m_md->getValue(gstTransX, x);
	m_md->getValue(gstTransY, y);
	m_md->getValue(gstTransZ, z);
	//sprintf(str, "%.2f", x);
	str = wxString::Format("%.2f", x);
	m_x_trans_text->SetValue(str);
	//sprintf(str, "%.2f", y);
	str = wxString::Format("%.2f", y);
	m_y_trans_text->SetValue(str);
	//sprintf(str, "%.2f", z);
	str = wxString::Format("%.2f", z);
	m_z_trans_text->SetValue(str);
	m_md->getValue(gstRotX, x);
	m_md->getValue(gstRotY, y);
	m_md->getValue(gstRotZ, z);
	//sprintf(str, "%.2f", x);
	str = wxString::Format("%.2f", x);
	m_x_rot_text->SetValue(str);
	//sprintf(str, "%.2f", y);
	str = wxString::Format("%.2f", y);
	m_y_rot_text->SetValue(str);
	//sprintf(str, "%.2f", z);
	str = wxString::Format("%.2f", z);
	m_z_rot_text->SetValue(str);
	m_md->getValue(gstScaleX, x);
	m_md->getValue(gstScaleY, y);
	m_md->getValue(gstScaleZ, z);
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
	m_md->setValue(gstTransX, x);
	m_md->setValue(gstTransY, y);
	m_md->setValue(gstTransZ, z);

	str = m_x_rot_text->GetValue();
	x = STOD(str.fn_str());
	str = m_y_rot_text->GetValue();
	y = STOD(str.fn_str());
	str = m_z_rot_text->GetValue();
	z = STOD(str.fn_str());
	m_md->setValue(gstRotX, x);
	m_md->setValue(gstRotY, y);
	m_md->setValue(gstRotZ, z);

	str = m_x_scl_text->GetValue();
	x = STOD(str.fn_str());
	str = m_y_scl_text->GetValue();
	y = STOD(str.fn_str());
	str = m_z_scl_text->GetValue();
	z = STOD(str.fn_str());
	m_md->setValue(gstScaleX, x);
	m_md->setValue(gstScaleY, y);
	m_md->setValue(gstScaleZ, z);

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

