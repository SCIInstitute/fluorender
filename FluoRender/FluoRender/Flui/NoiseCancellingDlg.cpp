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
#include <NoiseCancellingDlg.h>
#include <Global.h>
#include <Names.h>
#include <ComponentDefault.h>
#include <MainFrame.h>
#include <TreePanel.h>
#include <CurrentObjects.h>
#include <VolumeData.h>
#include <CompGenerator.h>
#include <CompAnalyzer.h>
#include <CompSelector.h>
#include <VolumeSelector.h>
#include <wxSingleSlider.h>
#include <wx/valnum.h>

NoiseCancellingDlg::NoiseCancellingDlg(MainFrame *frame)
: PropPanel(frame, frame,
	wxDefaultPosition,
	frame->FromDIP(wxSize(450, 200)),
	0, "NoiseCancellingDlg"),
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

	//group1
	wxBoxSizer *sizer1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Threshold:",
		wxDefaultPosition, FromDIP(wxSize(75, 23)));
	m_threshold_sldr = new wxSingleSlider(this, wxID_ANY, 0, 0, 2550,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_threshold_text = new wxTextCtrl(this, wxID_ANY, "0.0",
		wxDefaultPosition, FromDIP(wxSize(40, 20)), wxTE_RIGHT, vald_fp1);
	m_preview_btn = new wxButton(this, wxID_ANY, "Preview",
		wxDefaultPosition, FromDIP(wxSize(70, 23)));
	m_threshold_sldr->Bind(wxEVT_SCROLL_CHANGED, &NoiseCancellingDlg::OnThresholdChange, this);
	m_threshold_text->Bind(wxEVT_TEXT, &NoiseCancellingDlg::OnThresholdText, this);
	m_preview_btn->Bind(wxEVT_BUTTON, &NoiseCancellingDlg::OnPreviewBtn, this);
	sizer1->Add(st, 0, wxALIGN_CENTER);
	sizer1->Add(m_threshold_sldr, 1, wxEXPAND);
	sizer1->Add(m_threshold_text, 0, wxALIGN_CENTER);
	sizer1->Add(5, 5);
	sizer1->Add(m_preview_btn, 0, wxALIGN_CENTER);

	//group2
	wxBoxSizer *sizer2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Voxel Size:",
		wxDefaultPosition, FromDIP(wxSize(75, 23)));
	m_voxel_sldr = new wxSingleSlider(this, wxID_ANY, 1, 1, 500,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_voxel_text = new wxTextCtrl(this, wxID_ANY, "1",
		wxDefaultPosition, FromDIP(wxSize(40, 20)), wxTE_RIGHT, vald_int);
	m_erase_btn = new wxButton(this, wxID_ANY, "Erase",
		wxDefaultPosition, FromDIP(wxSize(70, 23)));
	m_voxel_sldr->Bind(wxEVT_SCROLL_CHANGED, &NoiseCancellingDlg::OnVoxelChange, this);
	m_voxel_text->Bind(wxEVT_TEXT, &NoiseCancellingDlg::OnVoxelText, this);
	m_erase_btn->Bind(wxEVT_BUTTON, &NoiseCancellingDlg::OnEraseBtn, this);
	sizer2->Add(st, 0, wxALIGN_CENTER);
	sizer2->Add(m_voxel_sldr, 1, wxEXPAND);
	sizer2->Add(m_voxel_text, 0, wxALIGN_CENTER);
	sizer2->Add(5, 5);
	sizer2->Add(m_erase_btn, 0, wxALIGN_CENTER);

	//group3
	wxBoxSizer *sizer3 = new wxBoxSizer(wxHORIZONTAL);
	m_ca_select_only_chk = new wxCheckBox(this, wxID_ANY, "Selct. Only",
		wxDefaultPosition, FromDIP(wxSize(95, 20)));
	m_ca_select_only_chk->Bind(wxEVT_CHECKBOX, &NoiseCancellingDlg::OnSelOnlyChk, this);
	sizer3->Add(m_ca_select_only_chk, 0, wxALIGN_CENTER);

	//group4
	wxBoxSizer *sizer4 = new wxBoxSizer(wxHORIZONTAL);
	m_enhance_sel_chk = new wxCheckBox(this, wxID_ANY, "Enhance selection",
		wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_enhance_sel_chk->Bind(wxEVT_CHECKBOX, &NoiseCancellingDlg::OnEnhanceSelChk, this);
	sizer4->Add(m_enhance_sel_chk, 0, wxALIGN_CENTER);

	//group5
	wxBoxSizer *sizer5 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0,
		"Check this option if selections are too dim. It is equivalent to\n"\
		"adjusting the Equalization values in the Output Adjustment\n panel.",
		wxDefaultPosition, wxDefaultSize);
	sizer5->Add(st, 0, wxALIGN_CENTER);

	//all controls
	wxBoxSizer *sizerV = new wxBoxSizer(wxVERTICAL);
	sizerV->Add(10, 10);
	sizerV->Add(sizer1, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(sizer2, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(sizer3, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(sizer4, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(sizer5, 0, wxEXPAND);

	SetSizer(sizerV);
	Layout();
}

NoiseCancellingDlg::~NoiseCancellingDlg()
{
}

void NoiseCancellingDlg::FluoUpdate(const fluo::ValueCollection& vc)
{
	//update user interface
	if (FOUND_VALUE(gstNull))
		return;
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;

	bool update_all = vc.empty();
	m_max_value = vd->GetMaxValue();

	double dval;
	int ival;

	if (update_all || FOUND_VALUE(gstNrThresh))
	{
		//threshold
		dval = glbin_comp_def.m_nr_thresh;
		m_threshold_sldr->SetRange(0, std::round(m_max_value*10.0));
		m_threshold_sldr->ChangeValue(std::round(dval * m_max_value * 10.0));
		m_threshold_text->ChangeValue(wxString::Format("%.1f", dval * m_max_value));
	}

	if (update_all || FOUND_VALUE(gstNrSize))
	{
		//voxel
		ival = glbin_comp_def.m_nr_size;
		auto res = vd->GetResolution();
		m_voxel_sldr->SetRange(1, res.intx());
		m_voxel_sldr->ChangeValue(std::round(ival));
		m_voxel_text->ChangeValue(wxString::Format("%d", int(std::round(ival))));
	}

	if (update_all || FOUND_VALUE(gstUseSelection))
	{
		m_ca_select_only_chk->SetValue(glbin_comp_generator.GetUseSel());
	}

	if (update_all || FOUND_VALUE(gstNrPreview))
	{
		m_enhance_sel_chk->SetValue(glbin_comp_def.m_nr_preview);
	}
}

void NoiseCancellingDlg::Preview()
{
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;

	bool bval = glbin_comp_generator.GetUseSel();
	glbin_comp_generator.SetThresh(glbin_comp_def.m_nr_thresh);
	glbin_comp_generator.SetVolumeData(vd);
	glbin_comp_generator.Compute();

	bool use_min = glbin_comp_analyzer.GetUseMin();
	bool use_max = glbin_comp_analyzer.GetUseMax();
	int min_num = glbin_comp_analyzer.GetMinNum();
	int max_num = glbin_comp_analyzer.GetMaxNum();

	glbin_comp_analyzer.SetUseMin(false);
	glbin_comp_analyzer.SetUseMax(true);
	glbin_comp_analyzer.SetMinNum(0);
	glbin_comp_analyzer.SetMaxNum(glbin_comp_def.m_nr_size);
	glbin_comp_analyzer.SetVolume(vd);
	glbin_comp_analyzer.Analyze();

	//cell size filter
	glbin_comp_selector.SetUseMin(false);
	glbin_comp_selector.SetUseMax(true);
	glbin_comp_selector.SetMinNum(0);
	glbin_comp_selector.SetMaxNum(glbin_comp_def.m_nr_size);
	glbin_comp_selector.CompFull();

	glbin_comp_def.m_nr_preview = true;

	Enhance();

	//restore settings
	glbin_comp_analyzer.SetUseMin(use_min);
	glbin_comp_analyzer.SetUseMax(use_max);
	glbin_comp_analyzer.SetMinNum(min_num);
	glbin_comp_analyzer.SetMaxNum(max_num);
	glbin_comp_selector.SetUseMin(use_min);
	glbin_comp_selector.SetUseMax(use_max);
	glbin_comp_selector.SetMinNum(min_num);
	glbin_comp_selector.SetMaxNum(max_num);
}

void NoiseCancellingDlg::Enhance()
{
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;

	if (glbin_comp_def.m_nr_preview)
	{
		fluo::Color mask_color = vd->GetMaskColor();
		double hdr_r = 0.0;
		double hdr_g = 0.0;
		double hdr_b = 0.0;
		if (mask_color.r() > 0.0)
			hdr_r = 0.4;
		if (mask_color.g() > 0.0)
			hdr_g = 0.4;
		if (mask_color.b() > 0.0)
			hdr_b = 0.4;
		fluo::Color hdr_color(hdr_r, hdr_g, hdr_b);
		glbin_comp_def.m_nr_hdr_r = vd->GetHdr().r();
		glbin_comp_def.m_nr_hdr_g = vd->GetHdr().g();
		glbin_comp_def.m_nr_hdr_b = vd->GetHdr().b();
		vd->SetHdr(hdr_color);
	}
	else if (!glbin_comp_def.m_nr_preview)
	{
		fluo::Color c(
			glbin_comp_def.m_nr_hdr_r,
			glbin_comp_def.m_nr_hdr_g,
			glbin_comp_def.m_nr_hdr_b);
		vd->SetHdr(c);
	}
	FluoRefresh(3, { gstNull });
}

//threshold
void NoiseCancellingDlg::OnThresholdChange(wxScrollEvent& event)
{
	int ival = m_threshold_sldr->GetValue();
	wxString str = wxString::Format("%.1f", double (ival)/10.0);
	if (str != m_threshold_text->GetValue())
		m_threshold_text->SetValue(str);
}

void NoiseCancellingDlg::OnThresholdText(wxCommandEvent& event)
{
	wxString str = m_threshold_text->GetValue();
	double val;
	str.ToDouble(&val);
	m_threshold_sldr->ChangeValue(std::round(val*10.0));
	glbin_comp_def.m_nr_thresh = val/m_max_value;

	//change mask threshold
	auto vd = glbin_current.vol_data.lock();
	if (vd)
		vd->SetMaskThreshold(glbin_comp_def.m_nr_thresh);
	FluoRefresh(3, { gstNull });
}

//voxel size
void NoiseCancellingDlg::OnVoxelChange(wxScrollEvent& event)
{
	int ival = m_voxel_sldr->GetValue();
	wxString str = wxString::Format("%d", ival);
	if (str != m_voxel_text->GetValue())
		m_voxel_text->SetValue(str);
}

void NoiseCancellingDlg::OnVoxelText(wxCommandEvent& event)
{
	wxString str = m_voxel_text->GetValue();
	long ival;
	str.ToLong(&ival);
	m_voxel_sldr->ChangeValue(ival);
	glbin_comp_def.m_nr_size = ival;
}

void NoiseCancellingDlg::OnSelOnlyChk(wxCommandEvent& event)
{
	bool bval = m_ca_select_only_chk->GetValue();
	glbin_comp_generator.SetUseSel(bval);
	FluoRefresh(1, { gstUseSelection }, { -1 });
}

void NoiseCancellingDlg::OnPreviewBtn(wxCommandEvent& event)
{
	Preview();
	FluoRefresh(1, { gstNrPreview }, { -1 });
}

void NoiseCancellingDlg::OnEraseBtn(wxCommandEvent& event)
{
	glbin_vol_selector.Erase();
	FluoRefresh(3, { gstNull });
}

void NoiseCancellingDlg::OnEnhanceSelChk(wxCommandEvent& event)
{
	glbin_comp_def.m_nr_preview = m_enhance_sel_chk->GetValue();
	FluoRefresh(3, { gstNull });
}
