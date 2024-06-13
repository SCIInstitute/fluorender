/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2024 Scientific Computing and Imaging Institute,
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
#include <MainFrame.h>
#include <RenderCanvas.h>
#include <wx/valnum.h>
#include <wx/stdpaths.h>

//BEGIN_EVENT_TABLE(CountingDlg, wxPanel)
//	//component analyzer
//	EVT_CHECKBOX(ID_CAIgnoreMaxChk, CountingDlg::OnCAIgnoreMaxChk)
//	EVT_BUTTON(ID_CAAnalyzeBtn, CountingDlg::OnCAAnalyzeBtn)
//END_EVENT_TABLE()

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
	m_ca_select_only_chk = new wxCheckBox(this, ID_CASelectOnlyChk, "Selct. Only",
		wxDefaultPosition, FromDIP(wxSize(95, 20)));
	sizer_1->Add(m_ca_select_only_chk, 0, wxALIGN_CENTER);
	sizer_1->AddStretchSpacer();
	st = new wxStaticText(this, 0, "Min:",
		wxDefaultPosition, FromDIP(wxSize(35, 15)));
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	m_ca_min_text = new wxTextCtrl(this, ID_CAMinText, "0",
		wxDefaultPosition, FromDIP(wxSize(40, 20)), wxTE_RIGHT, vald_int);
	sizer_1->Add(m_ca_min_text, 0, wxALIGN_CENTER);
	st = new wxStaticText(this, 0, "vx",
		wxDefaultPosition, FromDIP(wxSize(15, 15)));
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	sizer_1->AddStretchSpacer();
	st = new wxStaticText(this, 0, "Max:",
		wxDefaultPosition, FromDIP(wxSize(35, 15)));
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	m_ca_max_text = new wxTextCtrl(this, ID_CAMaxText, "1000",
		wxDefaultPosition, FromDIP(wxSize(40, 20)), wxTE_RIGHT, vald_int);
	sizer_1->Add(m_ca_max_text, 0, wxALIGN_CENTER);
	st = new wxStaticText(this, 0, "vx",
		wxDefaultPosition, FromDIP(wxSize(15, 15)));
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	sizer_1->AddStretchSpacer();
	m_ca_ignore_max_chk = new wxCheckBox(this, ID_CAIgnoreMaxChk, "Ignore Max");
	sizer_1->Add(m_ca_ignore_max_chk, 0, wxALIGN_CENTER);
	//text result
	wxBoxSizer *sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Components:  ");
	sizer_2->Add(st, 0, wxALIGN_CENTER);
	m_ca_comps_text = new wxTextCtrl(this, ID_CACompsText, "0",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_READONLY | wxTE_RIGHT);
	sizer_2->Add(m_ca_comps_text, 2, wxALIGN_CENTER);
	st = new wxStaticText(this, 0, "Total Volume:  ");
	sizer_2->Add(st, 0, wxALIGN_CENTER);
	m_ca_volume_text = new wxTextCtrl(this, ID_CAVolumeText, "0",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_READONLY | wxTE_RIGHT);
	sizer_2->Add(m_ca_volume_text, 2, wxALIGN_CENTER);
	st = new wxStaticText(this, 0, "vx",
		wxDefaultPosition, FromDIP(wxSize(15, 15)));
	sizer_2->Add(st, 0, wxALIGN_CENTER);
	sizer_2->Add(5, 5);
	m_ca_vol_unit_text = new wxTextCtrl(this, ID_CAVolUnitText, "0",
		wxDefaultPosition, FromDIP(wxSize(90, 20)), wxTE_READONLY | wxTE_RIGHT);
	sizer_2->Add(m_ca_vol_unit_text, 3, wxALIGN_CENTER);
	//export
	wxBoxSizer *sizer_3 = new wxBoxSizer(wxHORIZONTAL);
	sizer_3->AddStretchSpacer();
	m_ca_analyze_btn = new wxButton(this, ID_CAAnalyzeBtn, "Analyze",
		wxDefaultPosition, FromDIP(wxSize(-1, 23)));
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

	LoadDefault();
}

CountingDlg::~CountingDlg()
{
}

void CountingDlg::FluoUpdate(const fluo::ValueCollection& vc)
{

}

//load default
void CountingDlg::LoadDefault()
{
	bool bval;
	wxString str;
	//component analyzer
	//selected only
	m_ca_select_only_chk->SetValue(glbin_comp_def.m_use_sel);
	//min voxel
	str = wxString::Format("%d", glbin_comp_def.m_min_num);
	m_ca_min_text->ChangeValue(str);
	//max voxel
	str = wxString::Format("%d", glbin_comp_def.m_max_num);
	m_ca_max_text->ChangeValue(str);
	//ignore max
	bval = glbin_comp_def.m_use_min && !glbin_comp_def.m_use_max;
	m_ca_ignore_max_chk->SetValue(bval);
	m_ca_max_text->Enable(bval);
}

void CountingDlg::OnCAIgnoreMaxChk(wxCommandEvent& event)
{
	bool val = m_ca_ignore_max_chk->GetValue();
	m_ca_max_text->Enable(val);
}

//component analyze
void CountingDlg::OnCAAnalyzeBtn(wxCommandEvent& event)
{
	if (!m_view)
		return;
	VolumeData* vd = m_view->m_cur_vol;
	if (!vd)
		return;

	bool select = m_ca_select_only_chk->GetValue();

	glbin_comp_generator.SetVolumeData(vd);
	glbin_comp_generator.SetUseSel(select);
	vd->AddEmptyMask(1, !glbin_comp_generator.GetUseSel());
	vd->AddEmptyLabel(0, !glbin_comp_generator.GetUseSel());
	glbin_comp_generator.ShuffleID();
	double scale = vd->GetScalarScale();
	glbin_comp_generator.Grow();

	glbin_comp_analyzer.SetVolume(vd);
	glbin_comp_analyzer.Analyze(select);
	m_view->RefreshGL(39);

	flrd::CelpList *list = glbin_comp_analyzer.GetCelpList();
	if (!list || list->empty())
		return;

	double min_voxels, max_voxels;
	wxString str = m_ca_min_text->GetValue();
	str.ToDouble(&min_voxels);
	str = m_ca_max_text->GetValue();
	str.ToDouble(&max_voxels);
	bool ignore_max = m_ca_ignore_max_chk->GetValue();

	int count = 0;
	unsigned int vox = 0;
	for (auto it = list->begin();
		it != list->end(); ++it)
	{
		unsigned int sumi = it->second->GetSizeUi();
		if (sumi > min_voxels &&
			(ignore_max ||
			(!ignore_max && sumi < max_voxels)))
		{
			++count;
			vox += sumi;
		}
	}

	if (count > 0)
	{
		m_ca_comps_text->ChangeValue(wxString::Format("%d", count));
		m_ca_volume_text->ChangeValue(wxString::Format("%d", vox));

		double spcx, spcy, spcz;
		vd->GetSpacings(spcx, spcy, spcz);
		double vol_unit = vox * spcx*spcy*spcz;
		wxString vol_unit_text;
		vol_unit_text = wxString::Format("%.0f", vol_unit);
		vol_unit_text += " ";
		switch (m_view->m_sb_unit)
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
}

