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
#include <CalculationDlg.h>
#include <CalculationDlgAgent.h>

CalculationDlg::CalculationDlg(wxWindow* parent)
	: PropPanel(parent,
		wxDefaultPosition,
		parent->FromDIP(wxSize(500, 350)),
		0, "CalculationDlg")
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);
	SetDoubleBuffered(true);

	wxStaticText *st = 0;

	//operand A
	wxBoxSizer *sizer1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Operand A:",
		wxDefaultPosition, FromDIP(wxSize(75, 20)));
	m_calc_load_a_btn = new wxButton(this, wxID_ANY, "Load",
		wxDefaultPosition, FromDIP(wxSize(50, 20)));
	m_calc_a_text = new wxTextCtrl(this, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(200, 20)), wxTE_READONLY);
	m_calc_load_a_btn->Bind(wxEVT_BUTTON, &CalculationDlg::OnLoadA, this);
	sizer1->Add(st, 0, wxALIGN_CENTER);
	sizer1->Add(m_calc_load_a_btn, 0, wxALIGN_CENTER);
	sizer1->Add(m_calc_a_text, 1, wxEXPAND);
	//operand B
	wxBoxSizer *sizer2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Operand B:",
		wxDefaultPosition, FromDIP(wxSize(75, 20)));
	m_calc_load_b_btn = new wxButton(this, wxID_ANY, "Load",
		wxDefaultPosition, FromDIP(wxSize(50, 20)));
	m_calc_b_text = new wxTextCtrl(this, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(200, 20)), wxTE_READONLY);
	m_calc_load_b_btn->Bind(wxEVT_BUTTON, &CalculationDlg::OnLoadB, this);
	sizer2->Add(st, 0, wxALIGN_CENTER);
	sizer2->Add(m_calc_load_b_btn, 0, wxALIGN_CENTER);
	sizer2->Add(m_calc_b_text, 1, wxEXPAND);
	//single operators
	wxStaticBoxSizer *sizer3 = new wxStaticBoxSizer(
		wxHORIZONTAL, this, "Single-valued Operators (Require only A)");
	//sizer3
	m_calc_fill_btn = new wxButton(this, wxID_ANY, "Consolidate Voxels",
		wxDefaultPosition, FromDIP(wxSize(50, 25)));
	m_calc_combine_btn = new wxButton(this, wxID_ANY, "Combine Group",
		wxDefaultPosition, FromDIP(wxSize(50, 25)));
	m_calc_fill_btn->Bind(wxEVT_BUTTON, &CalculationDlg::OnCalcFill, this);
	m_calc_combine_btn->Bind(wxEVT_BUTTON, &CalculationDlg::OnCalcCombine, this);
	sizer3->Add(m_calc_fill_btn, 1, wxEXPAND);
	sizer3->Add(m_calc_combine_btn, 1, wxEXPAND);
	//two operators
	wxStaticBoxSizer *sizer4 = new wxStaticBoxSizer(
		wxHORIZONTAL, this, "Two-valued Operators (Require both A and B)");
	m_calc_sub_btn = new wxButton(this, wxID_ANY, "Subtract",
		wxDefaultPosition, FromDIP(wxSize(50, 25)));
	m_calc_add_btn = new wxButton(this, wxID_ANY, "Add",
		wxDefaultPosition, FromDIP(wxSize(50, 25)));
	m_calc_div_btn = new wxButton(this, wxID_ANY, "Divide",
		wxDefaultPosition, FromDIP(wxSize(50, 25)));
	m_calc_isc_btn = new wxButton(this, wxID_ANY, "Colocalize",
		wxDefaultPosition, FromDIP(wxSize(50, 25)));
	m_calc_sub_btn->Bind(wxEVT_BUTTON, &CalculationDlg::OnCalcSub, this);
	m_calc_add_btn->Bind(wxEVT_BUTTON, &CalculationDlg::OnCalcAdd, this);
	m_calc_div_btn->Bind(wxEVT_BUTTON, &CalculationDlg::OnCalcDiv, this);
	m_calc_isc_btn->Bind(wxEVT_BUTTON, &CalculationDlg::OnCalcIsc, this);
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

	SetSizer(sizer_v);
	Layout();
	SetAutoLayout(true);
	SetScrollRate(10, 10);
}

CalculationDlg::~CalculationDlg()
{

}

//update
void CalculationDlg::UpdateVolumeA(const std::wstring& str)
{
	m_calc_a_text->ChangeValue(str);
}

void CalculationDlg::UpdateVolumeB(const std::wstring& str)
{
	m_calc_b_text->ChangeValue(str);
}

//calculations
//operands
void CalculationDlg::OnLoadA(wxCommandEvent& event)
{
	auto agent = m_agent->As<CalculationDlgAgent>();
	if (agent)
	{
		agent->UpdateUIToData({ gstLoadVolumeA });
		agent->UpdateDataToUI({ gstVolumeA });
	}
}

void CalculationDlg::OnLoadB(wxCommandEvent& event)
{
	auto agent = m_agent->As<CalculationDlgAgent>();
	if (agent)
	{
		agent->UpdateUIToData({ gstLoadVolumeB });
		agent->UpdateDataToUI({ gstVolumeB });
	}
}

//operators
void CalculationDlg::OnCalcSub(wxCommandEvent& event)
{
	auto agent = m_agent->As<CalculationDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstCalcSub });
}

void CalculationDlg::OnCalcAdd(wxCommandEvent& event)
{
	auto agent = m_agent->As<CalculationDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstCalcAdd });
}

void CalculationDlg::OnCalcDiv(wxCommandEvent& event)
{
	auto agent = m_agent->As<CalculationDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstCalcDiv });
}

void CalculationDlg::OnCalcIsc(wxCommandEvent& event)
{
	auto agent = m_agent->As<CalculationDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstCalcIsc });
}

//one-operators
void CalculationDlg::OnCalcFill(wxCommandEvent& event)
{
	auto agent = m_agent->As<CalculationDlgAgent>();
	if (agent)
	{
		agent->UpdateUIToData({ gstCalcFill });
		agent->UpdateDataToUI({ gstVolumeB });
	}
}

void CalculationDlg::OnCalcCombine(wxCommandEvent& event)
{
	auto agent = m_agent->As<CalculationDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstCalcCombine });
}