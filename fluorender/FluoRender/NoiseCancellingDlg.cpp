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
#include "NoiseCancellingDlg.h"
#include "VRenderFrame.h"
#include "Components/CompSelector.h"
#include <wx/valnum.h>

BEGIN_EVENT_TABLE(NoiseCancellingDlg, wxPanel)
	EVT_COMMAND_SCROLL(ID_ThresholdSldr, NoiseCancellingDlg::OnThresholdChange)
	EVT_TEXT(ID_ThresholdText, NoiseCancellingDlg::OnThresholdText)
	EVT_COMMAND_SCROLL(ID_VoxelSldr, NoiseCancellingDlg::OnVoxelChange)
	EVT_TEXT(ID_VoxelText, NoiseCancellingDlg::OnVoxelText)
	EVT_BUTTON(ID_PreviewBtn, NoiseCancellingDlg::OnPreviewBtn)
	EVT_BUTTON(ID_EraseBtn, NoiseCancellingDlg::OnEraseBtn)
	EVT_CHECKBOX(ID_EnhanceSelChk, NoiseCancellingDlg::OnEnhanceSelChk)
END_EVENT_TABLE()

NoiseCancellingDlg::NoiseCancellingDlg(wxWindow *frame, wxWindow *parent)
: wxPanel(parent, wxID_ANY,
	wxDefaultPosition, wxSize(400, 150),
	0, "NoiseCancellingDlg"),
	m_frame(parent),
	m_view(0),
	m_max_value(255.0),
	m_dft_thresh(0.5),
	m_dft_size(50),
	m_previewed(false)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);

	//validator: floating point 1
	wxFloatingPointValidator<double> vald_fp1(1);
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;

	wxStaticText *st = 0;

	//group1
	wxBoxSizer *sizer1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Threshold:",
		wxDefaultPosition, wxSize(75, 23));
	m_threshold_sldr = new wxSlider(this, ID_ThresholdSldr, 0, 0, 2550,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_threshold_text = new wxTextCtrl(this, ID_ThresholdText, "0.0",
		wxDefaultPosition, wxSize(40, 20), 0, vald_fp1);
	m_preview_btn = new wxButton(this, ID_PreviewBtn, "Preview",
		wxDefaultPosition, wxSize(70, 23));
	sizer1->Add(st, 0, wxALIGN_CENTER);
	sizer1->Add(m_threshold_sldr, 1, wxEXPAND);
	sizer1->Add(m_threshold_text, 0, wxALIGN_CENTER);
	sizer1->Add(5, 5);
	sizer1->Add(m_preview_btn, 0, wxALIGN_CENTER);

	//group2
	wxBoxSizer *sizer2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Voxel Size:",
		wxDefaultPosition, wxSize(75, 23));
	m_voxel_sldr = new wxSlider(this, ID_VoxelSldr, 1, 1, 500,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_voxel_text = new wxTextCtrl(this, ID_VoxelText, "1",
		wxDefaultPosition, wxSize(40, 20), 0, vald_int);
	m_erase_btn = new wxButton(this, ID_EraseBtn, "Erase",
		wxDefaultPosition, wxSize(70, 23));
	sizer2->Add(st, 0, wxALIGN_CENTER);
	sizer2->Add(m_voxel_sldr, 1, wxEXPAND);
	sizer2->Add(m_voxel_text, 0, wxALIGN_CENTER);
	sizer2->Add(5, 5);
	sizer2->Add(m_erase_btn, 0, wxALIGN_CENTER);

	//group3
	wxBoxSizer *sizer3 = new wxBoxSizer(wxHORIZONTAL);
	m_ca_select_only_chk = new wxCheckBox(this, ID_CASelectOnlyChk, "Selct. Only",
		wxDefaultPosition, wxSize(95, 20));
	sizer3->Add(m_ca_select_only_chk, 0, wxALIGN_CENTER);

	//group4
	wxBoxSizer *sizer4 = new wxBoxSizer(wxHORIZONTAL);
	m_enhance_sel_chk = new wxCheckBox(this, ID_EnhanceSelChk, "Enhance selection",
		wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
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

void NoiseCancellingDlg::GetSettings(VRenderView* vrv)
{
	if (!vrv)
		return;

	VolumeData* sel_vol = 0;
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
		sel_vol = vr_frame->GetCurSelVol();
	else
		return;

	m_view = vrv;

	//threshold range
	if (sel_vol)
	{
		m_max_value = sel_vol->GetMaxValue();
		//threshold
		m_threshold_sldr->SetRange(0, int(m_max_value*10.0));
		m_threshold_sldr->SetValue(int(m_dft_thresh*m_max_value*10.0+0.5));
		m_threshold_text->ChangeValue(wxString::Format("%.1f", m_dft_thresh*m_max_value));
		//voxel
		int nx, ny, nz;
		sel_vol->GetResolution(nx, ny, nz);
		m_voxel_sldr->SetRange(1, nx);
		m_voxel_sldr->SetValue(int(m_dft_size));
		m_voxel_text->ChangeValue(wxString::Format("%d", int(m_dft_size)));
		m_previewed = false;
	}
}

void NoiseCancellingDlg::Preview(bool select, double size, double thresh)
{
	if (!m_view)
		return;
	VolumeData* vd = m_view->m_glview->m_cur_vol;
	if (!vd)
		return;

	FL::ComponentGenerator cg(vd);
	cg.SetUseMask(select);
	vd->AddEmptyMask(1, !cg.GetUseMask());
	vd->AddEmptyLabel(0, !cg.GetUseMask());
	cg.ShuffleID();
	double scale = vd->GetScalarScale();
	cg.Grow(false, -1, thresh, 0.0, scale);

	FL::ComponentAnalyzer ca(vd);
	ca.Analyze(select, true, false);

	FL::ComponentSelector comp_selector(vd);
	//cell size filter
	comp_selector.SetMinNum(false, 0);
	comp_selector.SetMaxNum(true, size);
	comp_selector.SetAnalyzer(&ca);
	comp_selector.CompFull();

	vd->GetVR()->clear_tex_pool();
	m_view->RefreshGL();
}

//threshold
void NoiseCancellingDlg::OnThresholdChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	wxString str = wxString::Format("%.1f", double (ival)/10.0);
	m_threshold_text->SetValue(str);
}

void NoiseCancellingDlg::OnThresholdText(wxCommandEvent &event)
{
	wxString str = m_threshold_text->GetValue();
	double val;
	str.ToDouble(&val);
	m_dft_thresh = val/m_max_value;
	m_threshold_sldr->SetValue(int(val*10.0+0.5));

	//change mask threshold
	VolumeData* sel_vol = 0;
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
		sel_vol = vr_frame->GetCurSelVol();
	if (sel_vol)
		sel_vol->SetMaskThreshold(m_dft_thresh);
	vr_frame->RefreshVRenderViews();
}

//voxel size
void NoiseCancellingDlg::OnVoxelChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	wxString str = wxString::Format("%d", ival);
	m_voxel_text->SetValue(str);
}

void NoiseCancellingDlg::OnVoxelText(wxCommandEvent &event)
{
	wxString str = m_voxel_text->GetValue();
	long ival;
	str.ToLong(&ival);
	m_dft_size = ival;
	m_voxel_sldr->SetValue(ival);
}

void NoiseCancellingDlg::OnPreviewBtn(wxCommandEvent &event)
{
	bool select = m_ca_select_only_chk->GetValue();
	Preview(select, m_dft_size, m_dft_thresh);
	m_previewed = true;
	OnEnhanceSelChk(event);
}

void NoiseCancellingDlg::OnEraseBtn(wxCommandEvent &event)
{
	VRenderFrame* frame = (VRenderFrame*)m_frame;
	if (frame && frame->GetTree())
		frame->GetTree()->BrushErase();
}

void NoiseCancellingDlg::OnEnhanceSelChk(wxCommandEvent &event)
{
	if (!m_previewed)
		return;

	bool enhance = m_enhance_sel_chk->GetValue();

	VolumeData* sel_vol = 0;
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
		sel_vol = vr_frame->GetCurSelVol();
	if (enhance && sel_vol)
	{
		Color mask_color = sel_vol->GetMaskColor();
		double hdr_r = 0.0;
		double hdr_g = 0.0;
		double hdr_b = 0.0;
		if (mask_color.r() > 0.0)
			hdr_r = 0.4;
		if (mask_color.g() > 0.0)
			hdr_g = 0.4;
		if (mask_color.b() > 0.0)
			hdr_b = 0.4;
		Color hdr_color = Color(hdr_r, hdr_g, hdr_b);
		m_hdr = sel_vol->GetHdr();
		sel_vol->SetHdr(hdr_color);
	}
	else if (!enhance && sel_vol)
	{
		sel_vol->SetHdr(m_hdr);
	}
	if (vr_frame)
		vr_frame->RefreshVRenderViews();
}
