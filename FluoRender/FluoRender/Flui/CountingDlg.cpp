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
#include <CountingDlg.h>
#include <Global.h>
#include <Names.h>
#include <MainFrame.h>
#include <RenderView.h>
#include <CurrentObjects.h>
#include <VolumeData.h>
#include <CompGenerator.h>
#include <CompAnalyzer.h>
#include <wx/valnum.h>

CountingDlg::CountingDlg(MainFrame *frame)
: PropPanel(frame, frame,
wxDefaultPosition,
frame->FromDIP(wxSize(400, 150)),
0, "CountingDlg"),
m_max_value(255.0)
{
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
	m_ca_select_only_chk = new wxCheckBox(this, wxID_ANY, "Selct. Only",
		wxDefaultPosition, FromDIP(wxSize(95, 20)));
	m_ca_select_only_chk->Bind(wxEVT_CHECKBOX, &CountingDlg::OnUseSelChk, this);
	sizer_1->Add(m_ca_select_only_chk, 0, wxALIGN_CENTER);
	sizer_1->AddStretchSpacer();
	st = new wxStaticText(this, 0, "Min:",
		wxDefaultPosition, FromDIP(wxSize(35, 15)));
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	m_ca_min_text = new wxTextCtrl(this, wxID_ANY, "0",
		wxDefaultPosition, FromDIP(wxSize(40, 20)), wxTE_RIGHT, vald_int);
	m_ca_min_text->Bind(wxEVT_TEXT, &CountingDlg::OnMinText, this);
	sizer_1->Add(m_ca_min_text, 0, wxALIGN_CENTER);
	st = new wxStaticText(this, 0, "vx",
		wxDefaultPosition, FromDIP(wxSize(15, 15)));
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	sizer_1->AddStretchSpacer();
	st = new wxStaticText(this, 0, "Max:",
		wxDefaultPosition, FromDIP(wxSize(35, 15)));
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	m_ca_max_text = new wxTextCtrl(this, wxID_ANY, "1000",
		wxDefaultPosition, FromDIP(wxSize(40, 20)), wxTE_RIGHT, vald_int);
	m_ca_max_text->Bind(wxEVT_TEXT, &CountingDlg::OnMaxText, this);
	sizer_1->Add(m_ca_max_text, 0, wxALIGN_CENTER);
	st = new wxStaticText(this, 0, "vx",
		wxDefaultPosition, FromDIP(wxSize(15, 15)));
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	sizer_1->AddStretchSpacer();
	m_ca_ignore_max_chk = new wxCheckBox(this, wxID_ANY, "Ignore Max");
	m_ca_ignore_max_chk->Bind(wxEVT_CHECKBOX, &CountingDlg::OnIgnoreMaxChk, this);
	sizer_1->Add(m_ca_ignore_max_chk, 0, wxALIGN_CENTER);
	//text result
	wxBoxSizer *sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Components:  ");
	sizer_2->Add(st, 0, wxALIGN_CENTER);
	m_ca_comps_text = new wxTextCtrl(this, wxID_ANY, "0",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_READONLY | wxTE_RIGHT);
	sizer_2->Add(m_ca_comps_text, 2, wxALIGN_CENTER);
	st = new wxStaticText(this, 0, "Total Volume:  ");
	sizer_2->Add(st, 0, wxALIGN_CENTER);
	m_ca_volume_text = new wxTextCtrl(this, wxID_ANY, "0",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_READONLY | wxTE_RIGHT);
	sizer_2->Add(m_ca_volume_text, 2, wxALIGN_CENTER);
	st = new wxStaticText(this, 0, "vx",
		wxDefaultPosition, FromDIP(wxSize(15, 15)));
	sizer_2->Add(st, 0, wxALIGN_CENTER);
	sizer_2->Add(5, 5);
	m_ca_vol_unit_text = new wxTextCtrl(this, wxID_ANY, "0",
		wxDefaultPosition, FromDIP(wxSize(90, 20)), wxTE_READONLY | wxTE_RIGHT);
	sizer_2->Add(m_ca_vol_unit_text, 3, wxALIGN_CENTER);
	//export
	wxBoxSizer *sizer_3 = new wxBoxSizer(wxHORIZONTAL);
	sizer_3->AddStretchSpacer();
	m_ca_analyze_btn = new wxButton(this, wxID_ANY, "Analyze",
		wxDefaultPosition, FromDIP(wxSize(-1, 23)));
	m_ca_analyze_btn->Bind(wxEVT_BUTTON, &CountingDlg::OnAnalyzeBtn, this);
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

void CountingDlg::FluoUpdate(const fluo::ValueCollection& vc)
{
	//update user interface
	if (FOUND_VALUE(gstNull))
		return;
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;

	bool update_all = vc.empty();
	m_max_value = vd->GetMaxValue();

	bool bval;
	int ival;
	wxString str;

	//selected only
	if (update_all || FOUND_VALUE(gstUseSelection))
	{
		bval = glbin_comp_generator.GetUseSel();
		m_ca_select_only_chk->SetValue(bval);
	}
	//min voxel
	if (update_all || FOUND_VALUE(gstCountMinValue))
	{
		ival = glbin_comp_analyzer.GetMinNum();
		str = wxString::Format("%d", ival);
		m_ca_min_text->ChangeValue(str);
	}
	//max voxel
	if (update_all || FOUND_VALUE(gstCountMaxValue))
	{
		ival = glbin_comp_analyzer.GetMaxNum();
		str = wxString::Format("%d", ival);
		m_ca_max_text->ChangeValue(str);
	}
	//ignore max
	if (update_all || FOUND_VALUE(gstCountUseMax))
	{
		bval = !glbin_comp_analyzer.GetUseMax();
		m_ca_ignore_max_chk->SetValue(bval);
		m_ca_max_text->Enable(bval);
	}
	//result
	if (FOUND_VALUE(gstCountResult))
		OutputSize();
}

void CountingDlg::OutputSize()
{
	size_t count = glbin_comp_analyzer.GetCount();
	size_t vox = glbin_comp_analyzer.GetVox();
	double size = glbin_comp_analyzer.GetSize();

	m_ca_comps_text->ChangeValue(wxString::Format("%llu", count));
	m_ca_volume_text->ChangeValue(wxString::Format("%llu", vox));

	wxString vol_unit_text;
	vol_unit_text = wxString::Format("%.0f", size);
	vol_unit_text += " ";
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;
	switch (view->m_sb_unit)
	{
	case 0:
		vol_unit_text += L"nm\u00B3";
		break;
	case 1:
	default:
		vol_unit_text += L"\u03BCm\u00B3";
		break;
	case 2:
		vol_unit_text += L"mm\u00B3";
		break;
	}

	m_ca_vol_unit_text->ChangeValue(vol_unit_text);
}

void CountingDlg::OnUseSelChk(wxCommandEvent& event)
{
	bool bval = m_ca_select_only_chk->GetValue();
	glbin_comp_generator.SetUseSel(bval);
}

void CountingDlg::OnMinText(wxCommandEvent& event)
{
	long ival;
	wxString str = m_ca_min_text->GetValue();
	if (str.ToLong(&ival))
	{
		glbin_comp_analyzer.SetUseMin(true);
		glbin_comp_analyzer.SetMinNum(ival);
	}
}

void CountingDlg::OnMaxText(wxCommandEvent& event)
{
	long ival;
	wxString str = m_ca_max_text->GetValue();
	if (str.ToLong(&ival))
	{
		glbin_comp_analyzer.SetUseMax(true);
		glbin_comp_analyzer.SetMaxNum(ival);
	}
}

void CountingDlg::OnIgnoreMaxChk(wxCommandEvent& event)
{
	bool val = m_ca_ignore_max_chk->GetValue();
	glbin_comp_analyzer.SetUseMax(!val);
	FluoUpdate({ gstCountUseMax });
}

//component analyze
void CountingDlg::OnAnalyzeBtn(wxCommandEvent& event)
{
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;

	glbin_comp_generator.SetVolumeData(vd);
	glbin_comp_generator.Compute();
	glbin_comp_analyzer.SetVolume(vd);
	glbin_comp_analyzer.Analyze();
	glbin_comp_analyzer.Count();

	FluoRefresh(2, { gstCountResult });
}

