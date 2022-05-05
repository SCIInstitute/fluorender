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
#include <CountingDlg.h>
#include <RenderFrame.h>
#include <Global.hpp>
#include <AgentFactory.hpp>
#include <wx/valnum.h>

BEGIN_EVENT_TABLE(CountingDlg, wxPanel)
	//component analyzer
	EVT_CHECKBOX(ID_CAIgnoreMaxChk, CountingDlg::OnCAIgnoreMaxChk)
	EVT_BUTTON(ID_CAAnalyzeBtn, CountingDlg::OnCAAnalyzeBtn)
END_EVENT_TABLE()

CountingDlg::CountingDlg(RenderFrame *frame)
: wxPanel(frame, wxID_ANY,
wxDefaultPosition, wxSize(400, 150),
0, "CountingDlg")
{
	m_agent = glbin_agtf->addCountingAgent(gstCountingAgent, *this);
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);

	SetDoubleBuffered(true);

	//validator: floating point 1
	wxFloatingPointValidator<double> vald_fp1(1);
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;

	wxStaticText *st = 0;

	//component analyzer
	//size of ccl
	wxBoxSizer *sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	m_ca_select_only_chk = new wxCheckBox(this, ID_CASelectOnlyChk, "Selct. Only",
		wxDefaultPosition, wxSize(95, 20));
	sizer_1->Add(m_ca_select_only_chk, 0, wxALIGN_CENTER);
	sizer_1->AddStretchSpacer();
	st = new wxStaticText(this, 0, "Min:",
		wxDefaultPosition, wxSize(35, 15));
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	m_ca_min_text = new wxTextCtrl(this, ID_CAMinText, "0",
		wxDefaultPosition, wxSize(40, 20), 0, vald_int);
	sizer_1->Add(m_ca_min_text, 0, wxALIGN_CENTER);
	st = new wxStaticText(this, 0, "vx",
		wxDefaultPosition, wxSize(15, 15));
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	sizer_1->AddStretchSpacer();
	st = new wxStaticText(this, 0, "Max:",
		wxDefaultPosition, wxSize(35, 15));
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	m_ca_max_text = new wxTextCtrl(this, ID_CAMaxText, "1000",
		wxDefaultPosition, wxSize(40, 20), 0, vald_int);
	sizer_1->Add(m_ca_max_text, 0, wxALIGN_CENTER);
	st = new wxStaticText(this, 0, "vx",
		wxDefaultPosition, wxSize(15, 15));
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	sizer_1->AddStretchSpacer();
	m_ca_ignore_max_chk = new wxCheckBox(this, ID_CAIgnoreMaxChk, "Ignore Max");
	sizer_1->Add(m_ca_ignore_max_chk, 0, wxALIGN_CENTER);
	//text result
	wxBoxSizer *sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Components:  ");
	sizer_2->Add(st, 0, wxALIGN_CENTER);
	m_ca_comps_text = new wxTextCtrl(this, ID_CACompsText, "0",
		wxDefaultPosition, wxSize(60, 20), wxTE_READONLY);
	sizer_2->Add(m_ca_comps_text, 2, wxALIGN_CENTER);
	st = new wxStaticText(this, 0, "Total Volume:  ");
	sizer_2->Add(st, 0, wxALIGN_CENTER);
	m_ca_volume_text = new wxTextCtrl(this, ID_CAVolumeText, "0",
		wxDefaultPosition, wxSize(60, 20), wxTE_READONLY);
	sizer_2->Add(m_ca_volume_text, 2, wxALIGN_CENTER);
	st = new wxStaticText(this, 0, "vx",
		wxDefaultPosition, wxSize(15, 15));
	sizer_2->Add(st, 0, wxALIGN_CENTER);
	sizer_2->Add(5, 5);
	m_ca_vol_unit_text = new wxTextCtrl(this, ID_CAVolUnitText, "0",
		wxDefaultPosition, wxSize(90, 20), wxTE_READONLY);
	sizer_2->Add(m_ca_vol_unit_text, 3, wxALIGN_CENTER);
	//export
	wxBoxSizer *sizer_3 = new wxBoxSizer(wxHORIZONTAL);
	sizer_3->AddStretchSpacer();
	m_ca_analyze_btn = new wxButton(this, ID_CAAnalyzeBtn, "Analyze",
		wxDefaultPosition, wxSize(-1, 23));
	sizer_3->Add(m_ca_analyze_btn, 0, wxALIGN_CENTER);

	//all controls
	wxBoxSizer *sizerV = new wxBoxSizer(wxVERTICAL);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_1, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_2, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_3, 0, wxEXPAND);

	SetSizer(sizerV);
	Layout();
}

CountingDlg::~CountingDlg()
{
}

void CountingDlg::OnCASelectOnlyChk(wxCommandEvent& event)
{
	bool bval = m_ca_select_only_chk->GetValue();
	m_agent->setValue(gstUseSelection, bval);
}

void CountingDlg::OnCAMinChange(wxCommandEvent& event)
{
	long lval;
	wxString str = m_ca_min_text->GetValue();
	if (str.ToLong(&lval))
		m_agent->setValue(gstMinValue, lval);
}

void CountingDlg::OnCAMaxChange(wxCommandEvent& event)
{
	long lval;
	wxString str = m_ca_max_text->GetValue();
	if (str.ToLong(&lval))
		m_agent->setValue(gstMaxValue, lval);
}

void CountingDlg::OnCAIgnoreMaxChk(wxCommandEvent &event)
{
	bool bval = m_ca_ignore_max_chk->GetValue();
	m_agent->setValue(gstUseMax, !bval);
}

//component analyze
void CountingDlg::OnCAAnalyzeBtn(wxCommandEvent &event)
{
	m_agent->Analyze();
}

