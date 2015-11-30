/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2014 Scientific Computing and Imaging Institute,
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
#include "CalculationDlg.h"
#include "VRenderFrame.h"

BEGIN_EVENT_TABLE(CalculationDlg, wxPanel)
	//calculations
	//operands
	EVT_BUTTON(ID_CalcLoadABtn, CalculationDlg::OnLoadA)
	EVT_BUTTON(ID_CalcLoadBBtn, CalculationDlg::OnLoadB)
	//operators
	EVT_BUTTON(ID_CalcSubBtn, CalculationDlg::OnCalcSub)
	EVT_BUTTON(ID_CalcAddBtn, CalculationDlg::OnCalcAdd)
	EVT_BUTTON(ID_CalcDivBtn, CalculationDlg::OnCalcDiv)
	EVT_BUTTON(ID_CalcIscBtn, CalculationDlg::OnCalcIsc)
	//one-operators
	EVT_BUTTON(ID_CalcFillBtn, CalculationDlg::OnCalcFill)
END_EVENT_TABLE()

CalculationDlg::CalculationDlg(wxWindow *frame, wxWindow *parent)
	: wxPanel(parent, wxID_ANY,
		wxDefaultPosition,
		wxSize(400, 550),
		0, "CalculationDlg")
{
	wxStaticText *st = 0;

	//operand A
	wxBoxSizer *sizer1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Volume A:",
		wxDefaultPosition, wxSize(75, 20));
	m_calc_load_a_btn = new wxButton(this, ID_CalcLoadABtn, "Load",
		wxDefaultPosition, wxSize(50, 20));
	m_calc_a_text = new wxTextCtrl(this, ID_CalcAText, "",
		wxDefaultPosition, wxSize(200, 20));
	sizer1->Add(st, 0, wxALIGN_CENTER);
	sizer1->Add(m_calc_load_a_btn, 0, wxALIGN_CENTER);
	sizer1->Add(m_calc_a_text, 1, wxEXPAND);
	//operand B
	wxBoxSizer *sizer2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Volume B:",
		wxDefaultPosition, wxSize(75, 20));
	m_calc_load_b_btn = new wxButton(this, ID_CalcLoadBBtn, "Load",
		wxDefaultPosition, wxSize(50, 20));
	m_calc_b_text = new wxTextCtrl(this, ID_CalcBText, "",
		wxDefaultPosition, wxSize(200, 20));
	sizer2->Add(st, 0, wxALIGN_CENTER);
	sizer2->Add(m_calc_load_b_btn, 0, wxALIGN_CENTER);
	sizer2->Add(m_calc_b_text, 1, wxEXPAND);
	//single operators
	wxBoxSizer *sizer3 = new wxStaticBoxSizer(
		new wxStaticBox(this, wxID_ANY,
			"Single-valued Operators (Require A)"), wxVERTICAL);
	//sizer3
	m_calc_fill_btn = new wxButton(this, ID_CalcFillBtn, "Consolidate Voxels",
		wxDefaultPosition, wxDefaultSize);
	sizer3->Add(m_calc_fill_btn, 0, wxEXPAND);
	//two operators
	wxBoxSizer *sizer4 = new wxStaticBoxSizer(
		new wxStaticBox(this, wxID_ANY,
			"Two-valued Operators (Require both A and B)"), wxHORIZONTAL);
	m_calc_sub_btn = new wxButton(this, ID_CalcSubBtn, "Subtract",
		wxDefaultPosition, wxSize(50, 25));
	m_calc_add_btn = new wxButton(this, ID_CalcAddBtn, "Add",
		wxDefaultPosition, wxSize(50, 25));
	m_calc_div_btn = new wxButton(this, ID_CalcDivBtn, "Divide",
		wxDefaultPosition, wxSize(50, 25));
	m_calc_isc_btn = new wxButton(this, ID_CalcIscBtn, "Colocalize",
		wxDefaultPosition, wxSize(50, 25));
	sizer4->Add(m_calc_sub_btn, 1, wxEXPAND);
	sizer4->Add(m_calc_add_btn, 1, wxEXPAND);
	sizer4->Add(m_calc_div_btn, 1, wxEXPAND);
	sizer4->Add(m_calc_isc_btn, 1, wxEXPAND);

	//vertical sizer
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
	sizer_v->Add(10, 10);
	sizer_v->Add(sizer1, 0, wxEXPAND);
	sizer_v->Add(10, 10);
	sizer_v->Add(sizer2, 0, wxEXPAND);
	sizer_v->Add(10, 30);
	sizer_v->Add(sizer3, 0, wxEXPAND);
	sizer_v->Add(10, 30);
	sizer_v->Add(sizer4, 0, wxEXPAND);
	sizer_v->Add(10, 10);

	SetSizerAndFit(sizer_v);
	Layout();
}

CalculationDlg::~CalculationDlg()
{

}

//calculations
//operands
void CalculationDlg::OnLoadA(wxCommandEvent &event)
{
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
		m_vol1 = vr_frame->GetCurSelVol();
		if (m_vol1)
			m_calc_a_text->SetValue(m_vol1->GetName());
	}
}

void CalculationDlg::OnLoadB(wxCommandEvent &event)
{
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
		m_vol2 = vr_frame->GetCurSelVol();
		if (m_vol2)
			m_calc_b_text->SetValue(m_vol2->GetName());
	}
}

//operators
void CalculationDlg::OnCalcSub(wxCommandEvent &event)
{
	if (!m_vol1 || !m_vol2)
		return;

	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
		m_cur_view = 0;
		for (int i = 0; i<vr_frame->GetViewNum(); i++)
		{
			VRenderView* vrv = vr_frame->GetView(i);
			wxString str = m_vol1->GetName();
			if (vrv && vrv->GetVolumeData(str))
			{
				m_cur_view = vrv;
				break;
			}
		}

		if (!m_cur_view)
			m_cur_view = vr_frame->GetView(0);

		if (m_cur_view)
		{
			m_cur_view->SetVolumeA(m_vol1);
			m_cur_view->SetVolumeB(m_vol2);
			m_cur_view->Calculate(1);
		}
	}
}

void CalculationDlg::OnCalcAdd(wxCommandEvent &event)
{
	if (!m_vol1 || !m_vol2)
		return;

	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
		m_cur_view = 0;
		for (int i = 0; i<vr_frame->GetViewNum(); i++)
		{
			VRenderView* vrv = vr_frame->GetView(i);
			wxString str = m_vol1->GetName();
			if (vrv && vrv->GetVolumeData(str))
			{
				m_cur_view = vrv;
				break;
			}
		}

		if (!m_cur_view)
			m_cur_view = vr_frame->GetView(0);

		if (m_cur_view)
		{
			m_cur_view->SetVolumeA(m_vol1);
			m_cur_view->SetVolumeB(m_vol2);
			m_cur_view->Calculate(2);
		}
	}
}

void CalculationDlg::OnCalcDiv(wxCommandEvent &event)
{
	if (!m_vol1 || !m_vol2)
		return;

	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
		m_cur_view = 0;
		for (int i = 0; i<vr_frame->GetViewNum(); i++)
		{
			VRenderView* vrv = vr_frame->GetView(i);
			wxString str = m_vol1->GetName();
			if (vrv && vrv->GetVolumeData(str))
			{
				m_cur_view = vrv;
				break;
			}
		}

		if (!m_cur_view)
			m_cur_view = vr_frame->GetView(0);

		if (m_cur_view)
		{
			m_cur_view->SetVolumeA(m_vol1);
			m_cur_view->SetVolumeB(m_vol2);
			m_cur_view->Calculate(3);
		}
	}
}

void CalculationDlg::OnCalcIsc(wxCommandEvent &event)
{
	if (!m_vol1 || !m_vol2)
		return;

	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
		m_cur_view = 0;
		for (int i = 0; i<vr_frame->GetViewNum(); i++)
		{
			VRenderView* vrv = vr_frame->GetView(i);
			wxString str = m_vol1->GetName();
			if (vrv && vrv->GetVolumeData(str))
			{
				m_cur_view = vrv;
				break;
			}
		}

		if (!m_cur_view)
			m_cur_view = vr_frame->GetView(0);

		if (m_cur_view)
		{
			m_cur_view->SetVolumeA(m_vol1);
			m_cur_view->SetVolumeB(m_vol2);
			m_cur_view->Calculate(4);
		}
	}
}

//one-operators
void CalculationDlg::OnCalcFill(wxCommandEvent &event)
{
	if (!m_vol1)
		return;

	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
		m_cur_view = 0;
		for (int i = 0; i<vr_frame->GetViewNum(); i++)
		{
			VRenderView* vrv = vr_frame->GetView(i);
			wxString str = m_vol1->GetName();
			if (vrv && vrv->GetVolumeData(str))
			{
				m_cur_view = vrv;
				break;
			}
		}

		if (!m_cur_view)
			m_cur_view = vr_frame->GetView(0);

		if (m_cur_view)
		{
			m_cur_view->SetVolumeA(m_vol1);
			m_vol2 = 0;
			m_cur_view->SetVolumeB(0);
			m_calc_b_text->Clear();
			m_cur_view->Calculate(9);
		}
	}
}
