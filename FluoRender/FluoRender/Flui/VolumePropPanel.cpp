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
#include <VolumePropPanel.h>
#include <Global.h>
#include <Names.h>
#include <MainSettings.h>
#include <MainFrame.h>
#include <RenderView.h>
#include <Root.h>
#include <VolumeData.h>
#include <VolumeGroup.h>
#include <CurrentObjects.h>
#include <DataManager.h>
#include <Histogram.h>
#include <RecordHistParams.h>
#include <TableHistParams.h>
#include <Reshape.h>
#include <MultiVolumeRenderer.h>
#include <VolumeRenderer.h>
#include <VolumeSelector.h>
#include <Colocalize.h>
#include <ShaderProgram.h>
#include <Color.h>
#include <BBox.h>
#include <Point.h>
#include <wxFadeButton.h>
#include <wxMapDoubleSlider.h>
#include <wxDoubleSlider.h>
#include <wxSingleSlider.h>
#include <wxUndoableCheckBox.h>
#include <wxUndoableColorPicker.h>
#include <wxUndoableComboBox.h>
#include <wxUndoableToolbar.h>
#include <wxUndoableTextCtrl.h>
#include <wxBoldText.h>
#include <Helper.h>
#include <png_resource.h>
#include <wx/colordlg.h>
#include <wx/valnum.h>
#include <icons.h>
#include <limits>
#include <Debug.h>

VolumePropPanel::VolumePropPanel(MainFrame* frame,
	wxWindow* parent,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name) :
	PropPanel(frame, parent, pos, size, style, name),
	m_vd(0),
	m_sync_group(false),
	m_group(0),
	m_view(0),
	m_max_val(255.0)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);
	Freeze();
	SetDoubleBuffered(true);

	wxBoxSizer* sizer_l1 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizer_l2 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizer_l3 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizer_l4 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizer_l5 = new wxBoxSizer(wxHORIZONTAL);

	wxBoxSizer* sizer_m1 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizer_m2 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizer_m3 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizer_m4 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizer_m5 = new wxBoxSizer(wxHORIZONTAL);

	wxBoxSizer* sizer_r1 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizer_r2 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizer_r3 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizer_r4 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizer_r5 = new wxBoxSizer(wxHORIZONTAL);

	wxStaticText* st = 0;

	//validator: floating point 2
	wxFloatingPointValidator<double> vald_fp2(2);
	//validator: floating point 3
	wxFloatingPointValidator<double> vald_fp3(3);
	//validator: floating point 4
	wxFloatingPointValidator<double> vald_fp4(4);
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;

	wxBitmapBundle bitmap;
	wxSize bts(FromDIP(wxSize(80, -1)));
	wxSize tts1(FromDIP(wxSize(40, -1)));
	wxSize tts2(FromDIP(wxSize(50, -1)));
	wxSize tts3(FromDIP(wxSize(60, -1)));
	wxSize tts4(FromDIP(wxSize(30, -1)));
	//left///////////////////////////////////////////////////
	//saturation point
	m_minmax_st = new wxFadeButton(this, wxID_ANY, "Min-Max",
		wxDefaultPosition, bts);
	m_minmax_sldr = new wxMapDoubleSlider(this, wxID_ANY, 0, 255, 0, 255,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_low_offset_text = new wxTextCtrl(this, wxID_ANY, "0",
		wxDefaultPosition, tts1, wxTE_RIGHT, vald_int);
	m_high_offset_text = new wxTextCtrl(this, wxID_ANY, "255",
		wxDefaultPosition, tts1, wxTE_RIGHT, vald_int);
	m_minmax_link_tb = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	bitmap = wxGetBitmap(unlink);
	m_minmax_link_tb->AddCheckTool(0, "",
		bitmap, wxNullBitmap,
		"Link min and max values",
		"Link min and max values");
	m_minmax_chk = new wxUndoableCheckBox(this, wxID_ANY, "");
	m_minmax_st->SetFontBold();
	m_minmax_st->SetTintColor(wxColor(255, 150, 200));
	m_minmax_sldr->SetMode(2);
	m_minmax_sldr->SetToolTip("Contrast-enhanced intensity distribution");
	m_minmax_sldr->SetHistoryIndicator(m_minmax_st);
	//bind events
	m_minmax_st->Bind(wxEVT_BUTTON, &VolumePropPanel::OnMinMaxMF, this);
	m_minmax_sldr->Bind(wxEVT_SCROLL_CHANGED, &VolumePropPanel::OnMinMaxChange, this);
	m_low_offset_text->Bind(wxEVT_TEXT, &VolumePropPanel::OnMinMaxText, this);
	m_high_offset_text->Bind(wxEVT_TEXT, &VolumePropPanel::OnMinMaxText, this);
	m_minmax_link_tb->Bind(wxEVT_TOOL, &VolumePropPanel::OnMinMaxLink, this);
	m_minmax_chk->Bind(wxEVT_CHECKBOX, &VolumePropPanel::OnMinMaxChk, this);
	//add to sizer
	sizer_l1->Add(m_minmax_link_tb, 0, wxALIGN_CENTER, 0);
	sizer_l1->Add(m_low_offset_text, 0, wxALIGN_CENTER);
	sizer_l1->Add(m_minmax_sldr, 1, wxEXPAND);
	sizer_l1->Add(m_high_offset_text, 0, wxALIGN_CENTER);
	sizer_l1->Add(5, 5);
	sizer_l1->Add(m_minmax_chk, 0, wxALIGN_CENTER);
	sizer_l1->Add(m_minmax_st, 0, wxALIGN_CENTER);
	m_minmax_link_tb->Realize();
	//gamma
	m_gamma_st = new wxFadeButton(this, wxID_ANY, "Gamma",
		wxDefaultPosition, bts);
	m_gamma_sldr = new wxSingleSlider(this, wxID_ANY, 100, 10, 400,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	//m_gamma_sldr->SetRangeStyle(1);
	m_gamma_text = new wxTextCtrl(this, wxID_ANY, "1.00",
		wxDefaultPosition, tts1, wxTE_RIGHT, vald_fp2);
	m_gamma_chk = new wxUndoableCheckBox(this, wxID_ANY, "");
	m_gamma_st->SetFontBold();
	m_gamma_st->SetTintColor(wxColor(255, 150, 150));
	m_gamma_sldr->SetHistoryIndicator(m_gamma_st);
	//bind events
	m_gamma_st->Bind(wxEVT_BUTTON, &VolumePropPanel::OnGammaMF, this);
	m_gamma_sldr->Bind(wxEVT_SCROLL_CHANGED, &VolumePropPanel::OnGammaChange, this);
	m_gamma_text->Bind(wxEVT_TEXT, &VolumePropPanel::OnGammaText, this);
	m_gamma_chk->Bind(wxEVT_CHECKBOX, &VolumePropPanel::OnGammaChk, this);
	//add to sizer
	sizer_l2->Add(m_gamma_sldr, 1, wxEXPAND);
	sizer_l2->Add(m_gamma_text, 0, wxALIGN_CENTER);
	sizer_l2->Add(5, 5);
	sizer_l2->Add(m_gamma_chk, 0, wxALIGN_CENTER);
	sizer_l2->Add(m_gamma_st, 0, wxALIGN_CENTER);
	//alpha
	m_alpha_st = new wxFadeButton(this, wxID_ANY, "Alpha",
		wxDefaultPosition, bts);
	m_alpha_sldr = new wxSingleSlider(this, wxID_ANY, 127, 0, 255,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_alpha_text = new wxTextCtrl(this, wxID_ANY, "127",
		wxDefaultPosition, tts1, wxTE_RIGHT, vald_int);
	m_alpha_chk = new wxUndoableCheckBox(this, wxID_ANY, "");
	m_alpha_st->SetFontBold();
	m_alpha_st->SetTintColor(wxColor(255, 200, 150));
	m_alpha_sldr->SetHistoryIndicator(m_alpha_st);
	//bind events
	m_alpha_st->Bind(wxEVT_BUTTON, &VolumePropPanel::OnAlphaMF, this);
	m_alpha_sldr->Bind(wxEVT_SCROLL_CHANGED, &VolumePropPanel::OnAlphaChange, this);
	m_alpha_text->Bind(wxEVT_TEXT, &VolumePropPanel::OnAlphaText, this);
	m_alpha_chk->Bind(wxEVT_CHECKBOX, &VolumePropPanel::OnAlphaCheck, this);
	//add to sizer
	sizer_l3->Add(m_alpha_sldr, 1, wxEXPAND);
	sizer_l3->Add(m_alpha_text, 0, wxALIGN_CENTER);
	sizer_l3->Add(5, 5);
	sizer_l3->Add(m_alpha_chk, 0, wxALIGN_CENTER);
	sizer_l3->Add(m_alpha_st, 0, wxALIGN_CENTER);
	//luminance
	m_luminance_st = new wxFadeButton(this, wxID_ANY, "Luminance",
		wxDefaultPosition, bts);
	m_luminance_sldr = new wxSingleSlider(this, wxID_ANY, 128, 0, 255,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_luminance_text = new wxTextCtrl(this, wxID_ANY, "128",
		wxDefaultPosition, tts1, wxTE_RIGHT/*, vald_int*/);
	m_luminance_chk = new wxUndoableCheckBox(this, wxID_ANY, "");
	m_luminance_st->SetFontBold();
	m_luminance_st->SetTintColor(wxColor(255, 255, 150));
	m_luminance_sldr->SetHistoryIndicator(m_luminance_st);
	//bind events
	m_luminance_st->Bind(wxEVT_BUTTON, &VolumePropPanel::OnLuminanceMF, this);
	m_luminance_sldr->Bind(wxEVT_SCROLL_CHANGED, &VolumePropPanel::OnLuminanceChange, this);
	m_luminance_text->Bind(wxEVT_TEXT, &VolumePropPanel::OnLuminanceText, this);
	m_luminance_chk->Bind(wxEVT_CHECKBOX, &VolumePropPanel::OnLuminanceChk, this);
	//add to sizer
	sizer_l4->Add(m_luminance_sldr, 1, wxEXPAND, 0);
	sizer_l4->Add(m_luminance_text, 0, wxALIGN_CENTER, 0);
	sizer_l4->Add(5, 5);
	sizer_l4->Add(m_luminance_chk, 0, wxALIGN_CENTER);
	sizer_l4->Add(m_luminance_st, 0, wxALIGN_CENTER, 0);
	//sample rate
	m_sample_st = new wxFadeButton(this, wxID_ANY, "Samp. Rate",
		wxDefaultPosition, bts);
	m_sample_sldr = new wxSingleSlider(this, wxID_ANY, 10, 1, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_sample_text = new wxTextCtrl(this, wxID_ANY, "1.0",
		wxDefaultPosition, tts1, wxTE_RIGHT, vald_fp2);
	m_sample_chk = new wxUndoableCheckBox(this, wxID_ANY, "");
	m_sample_st->SetFontBold();
	m_sample_st->SetTintColor(wxColor(180, 120, 180));
	m_sample_sldr->SetHistoryIndicator(m_sample_st);
	//bind events
	m_sample_st->Bind(wxEVT_BUTTON, &VolumePropPanel::OnSampleMF, this);
	m_sample_sldr->Bind(wxEVT_SCROLL_CHANGED, &VolumePropPanel::OnSampleChange, this);
	m_sample_text->Bind(wxEVT_TEXT, &VolumePropPanel::OnSampleText, this);
	m_sample_chk->Bind(wxEVT_CHECKBOX, &VolumePropPanel::OnSampleChk, this);
	//add to sizer
	sizer_l5->Add(m_sample_sldr, 1, wxEXPAND);
	sizer_l5->Add(m_sample_text, 0, wxALIGN_CENTER);
	sizer_l5->Add(5, 5);
	sizer_l5->Add(m_sample_chk, 0, wxALIGN_CENTER);
	sizer_l5->Add(m_sample_st, 0, wxALIGN_CENTER);

	//middle///////////////////////////////////////////////////
	//thresholds
	m_thresh_st = new wxFadeButton(this, wxID_ANY, "Threshold",
		wxDefaultPosition, bts);
	m_thresh_sldr = new wxMapDoubleSlider(this, wxID_ANY, 0, 255, 0, 255,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_left_thresh_text = new wxTextCtrl(this, wxID_ANY, "0",
		wxDefaultPosition, tts2, wxTE_RIGHT, vald_int);
	m_right_thresh_text = new wxTextCtrl(this, wxID_ANY, "255",
		wxDefaultPosition, tts1, wxTE_RIGHT, vald_int);
	m_thresh_link_tb = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	bitmap = wxGetBitmap(unlink);
	m_thresh_link_tb->AddCheckTool(0, "",
		bitmap, wxNullBitmap,
		"Link low and high threshold values",
		"Link low and high threshold values");
	m_thresh_chk = new wxUndoableCheckBox(this, wxID_ANY, "");
	m_thresh_st->SetFontBold();
	m_thresh_st->SetTintColor(wxColor(180, 255, 150));
	m_thresh_sldr->SetMode(1);
	m_thresh_sldr->SetToolTip("Contrast-enhanced intensity distribution");
	m_thresh_sldr->SetHistoryIndicator(m_thresh_st);
	//bind events
	m_thresh_st->Bind(wxEVT_BUTTON, &VolumePropPanel::OnThreshMF, this);
	m_thresh_sldr->Bind(wxEVT_SCROLL_CHANGED, &VolumePropPanel::OnThreshChange, this);
	m_left_thresh_text->Bind(wxEVT_TEXT, &VolumePropPanel::OnThreshText, this);
	m_right_thresh_text->Bind(wxEVT_TEXT, &VolumePropPanel::OnThreshText, this);
	m_thresh_link_tb->Bind(wxEVT_TOOL, &VolumePropPanel::OnThreshLink, this);
	m_thresh_chk->Bind(wxEVT_CHECKBOX, &VolumePropPanel::OnThreshChk, this);
	//add to sizer
	sizer_m1->Add(m_thresh_st, 0, wxALIGN_CENTER);
	sizer_m1->Add(5, 5);
	sizer_m1->Add(m_thresh_chk, 0, wxALIGN_CENTER);
	sizer_m1->Add(m_left_thresh_text, 0, wxALIGN_CENTER);
	sizer_m1->Add(m_thresh_sldr, 1, wxEXPAND);
	sizer_m1->Add(m_right_thresh_text, 0, wxALIGN_CENTER);
	sizer_m1->Add(m_thresh_link_tb, 0, wxALIGN_CENTER, 0);
	m_thresh_link_tb->Realize();
	//extract boundary
	m_boundary_st = new wxFadeButton(this, wxID_ANY, "Boundary",
		wxDefaultPosition, bts);
	m_boundary_sldr = new wxDoubleSlider(this, wxID_ANY, 0, 1000, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_boundary_low_text = new wxTextCtrl(this, wxID_ANY, "0.0000",
		wxDefaultPosition, tts2, wxTE_RIGHT, vald_fp4);
	m_boundary_high_text = new wxTextCtrl(this, wxID_ANY, "1.0000",
		wxDefaultPosition, tts1, wxTE_RIGHT, vald_fp4);
	m_boundary_link_tb = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	bitmap = wxGetBitmap(unlink);
	m_boundary_link_tb->AddCheckTool(0, "",
		bitmap, wxNullBitmap,
		"Link low and high boundary values",
		"Link low and high boundary values");
	m_boundary_chk = new wxUndoableCheckBox(this, wxID_ANY, "");
	m_boundary_st->SetFontBold();
	m_boundary_st->SetTintColor(wxColor(150, 255, 180));
	m_boundary_sldr->SetHistoryIndicator(m_boundary_st);
	//bind events
	m_boundary_st->Bind(wxEVT_BUTTON, &VolumePropPanel::OnBoundaryMF, this);
	m_boundary_sldr->Bind(wxEVT_SCROLL_CHANGED, &VolumePropPanel::OnBoundaryChange, this);
	m_boundary_low_text->Bind(wxEVT_TEXT, &VolumePropPanel::OnBoundaryText, this);
	m_boundary_high_text->Bind(wxEVT_TEXT, &VolumePropPanel::OnBoundaryText, this);
	m_boundary_link_tb->Bind(wxEVT_TOOL, &VolumePropPanel::OnBoundaryLink, this);
	m_boundary_chk->Bind(wxEVT_CHECKBOX, &VolumePropPanel::OnBoundaryChk, this);
	//add to sizer
	sizer_m2->Add(m_boundary_st, 0, wxALIGN_CENTER);
	sizer_m2->Add(5, 5);
	sizer_m2->Add(m_boundary_chk, 0, wxALIGN_CENTER);
	sizer_m2->Add(m_boundary_low_text, 0, wxALIGN_CENTER);
	sizer_m2->Add(m_boundary_sldr, 1, wxEXPAND);
	sizer_m2->Add(m_boundary_high_text, 0, wxALIGN_CENTER);
	sizer_m2->Add(m_boundary_link_tb, 0, wxALIGN_CENTER, 0);
	m_boundary_link_tb->Realize();
	//shading
	m_shading_strength_sldr = new wxSingleSlider(this, wxID_ANY, 0, 0, 200,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_shading_strength_text = new wxTextCtrl(this, wxID_ANY, "0.00",
		wxDefaultPosition, tts2, wxTE_RIGHT, vald_fp2);
	//highlight
	m_shade_st = new wxFadeButton(this, wxID_ANY, "Shading",
		wxDefaultPosition, bts);
	m_shading_shine_sldr = new wxSingleSlider(this, wxID_ANY, 0, 0, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_shading_shine_text = new wxTextCtrl(this, wxID_ANY, "0.00",
		wxDefaultPosition, tts3, wxTE_RIGHT, vald_fp2);
	m_shade_chk = new wxUndoableCheckBox(this, wxID_ANY, "");
	m_shade_st->SetFontBold();
	m_shade_st->SetTintColor(wxColor(150, 180, 255));
	m_shading_strength_sldr->SetHistoryIndicator(m_shade_st);
	//bind events
	m_shade_st->Bind(wxEVT_BUTTON, &VolumePropPanel::OnShadingMF, this);
	m_shading_strength_sldr->Bind(wxEVT_SCROLL_CHANGED, &VolumePropPanel::OnShadingStrengthChange, this);
	m_shading_strength_text->Bind(wxEVT_TEXT, &VolumePropPanel::OnShadingStrengthText, this);
	m_shading_shine_sldr->Bind(wxEVT_SCROLL_CHANGED, &VolumePropPanel::OnShadingShineChange, this);
	m_shading_shine_text->Bind(wxEVT_TEXT, &VolumePropPanel::OnShadingShineText, this);
	m_shade_chk->Bind(wxEVT_CHECKBOX, &VolumePropPanel::OnShadingChk, this);
	//add to sizer
	sizer_m3->Add(m_shade_st, 0, wxALIGN_CENTER);
	sizer_m3->Add(5, 5);
	sizer_m3->Add(m_shade_chk, 0, wxALIGN_CENTER);
	sizer_m3->Add(m_shading_strength_text, 0, wxALIGN_CENTER);
	sizer_m3->Add(m_shading_strength_sldr, 1, wxEXPAND);
	sizer_m3->Add(m_shading_shine_text, 0, wxALIGN_CENTER);
	sizer_m3->Add(m_shading_shine_sldr, 1, wxEXPAND);
	//shadow
	m_shadow_st = new wxFadeButton(this, wxID_ANY, "Shadow",
		wxDefaultPosition, bts);
	m_shadow_sldr = new wxSingleSlider(this, wxID_ANY, 0, 0, 200,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_shadow_text = new wxTextCtrl(this, wxID_ANY, "0.00",
		wxDefaultPosition, tts2, wxTE_RIGHT, vald_fp2);
	m_shadow_chk = new wxUndoableCheckBox(this, wxID_ANY, "");
	m_shadow_dir_chk = new wxUndoableCheckBox(this, wxID_ANY, "D:",
		wxDefaultPosition, tts4);
	m_shadow_dir_sldr = new wxSingleSlider(this, wxID_ANY, 0, -180, 180,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_shadow_dir_sldr->SetRangeStyle(2);
	m_shadow_dir_text = new wxTextCtrl(this, wxID_ANY, "0",
		wxDefaultPosition, tts4, wxTE_RIGHT, vald_int);
	m_shadow_st->SetFontBold();
	m_shadow_st->SetTintColor(wxColor(160, 150, 255));
	m_shadow_sldr->SetHistoryIndicator(m_shadow_st);
	//bind events
	m_shadow_st->Bind(wxEVT_BUTTON, &VolumePropPanel::OnShadowMF, this);
	m_shadow_sldr->Bind(wxEVT_SCROLL_CHANGED, &VolumePropPanel::OnShadowChange, this);
	m_shadow_text->Bind(wxEVT_TEXT, &VolumePropPanel::OnShadowText, this);
	m_shadow_chk->Bind(wxEVT_CHECKBOX, &VolumePropPanel::OnShadowChk, this);
	m_shadow_dir_chk->Bind(wxEVT_CHECKBOX, &VolumePropPanel::OnShadowDirCheck, this);
	m_shadow_dir_sldr->Bind(wxEVT_SCROLL_CHANGED, &VolumePropPanel::OnShadowDirChange, this);
	m_shadow_dir_text->Bind(wxEVT_TEXT, &VolumePropPanel::OnShadowDirEdit, this);
	//add to sizer
	sizer_m4->Add(m_shadow_st, 0, wxALIGN_CENTER);
	sizer_m4->Add(5, 5);
	sizer_m4->Add(m_shadow_chk, 0, wxALIGN_CENTER);
	sizer_m4->Add(m_shadow_text, 0, wxALIGN_CENTER);
	sizer_m4->Add(m_shadow_sldr, 1, wxEXPAND);
	sizer_m4->Add(m_shadow_dir_chk, 0, wxALIGN_CENTER);
	sizer_m4->Add(m_shadow_dir_text, 0, wxALIGN_CENTER);
	sizer_m4->Add(m_shadow_dir_sldr, 1, wxEXPAND);
	//colormap
	m_colormap_st = new wxFadeButton(this, wxID_ANY, "Colormap",
		wxDefaultPosition, bts);
	m_colormap_sldr = new wxMapDoubleSlider(this,
		wxID_ANY, 0, 255, 0, 255,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_colormap_low_text = new wxTextCtrl(this,
		wxID_ANY, "0",
		wxDefaultPosition, tts2, wxTE_RIGHT, vald_int);
	m_colormap_hi_text = new wxTextCtrl(this,
		wxID_ANY, "255",
		wxDefaultPosition, tts1, wxTE_RIGHT, vald_int);
	m_colormap_link_tb = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	bitmap = wxGetBitmap(unlink);
	m_colormap_link_tb->AddCheckTool(0, "",
		bitmap, wxNullBitmap,
		"Link low and high colormap values",
		"Link low and high colormap values");
	m_colormap_chk = new wxUndoableCheckBox(this, wxID_ANY, "");
	m_colormap_st->SetFontBold();
	m_colormap_st->SetMode(2);
	m_colormap_sldr->SetMode(3);
	m_colormap_sldr->SetHistoryIndicator(m_colormap_st);
	//bind events
	m_colormap_st->Bind(wxEVT_BUTTON, &VolumePropPanel::OnColormapMF, this);
	m_colormap_sldr->Bind(wxEVT_SCROLL_CHANGED, &VolumePropPanel::OnColormapChange, this);
	m_colormap_low_text->Bind(wxEVT_TEXT, &VolumePropPanel::OnColormapText, this);
	m_colormap_hi_text->Bind(wxEVT_TEXT, &VolumePropPanel::OnColormapText, this);
	m_colormap_link_tb->Bind(wxEVT_TOOL, &VolumePropPanel::OnColormapLink, this);
	m_colormap_chk->Bind(wxEVT_CHECKBOX, &VolumePropPanel::OnColormapChk, this);
	//add to sizer
	sizer_m5->Add(m_colormap_st, 0, wxALIGN_CENTER);
	sizer_m5->Add(5, 5);
	sizer_m5->Add(m_colormap_chk, 0, wxALIGN_CENTER);
	sizer_m5->Add(m_colormap_low_text, 0, wxALIGN_CENTER);
	sizer_m5->Add(m_colormap_sldr, 1, wxEXPAND);
	sizer_m5->Add(m_colormap_hi_text, 0, wxALIGN_CENTER);
	sizer_m5->Add(m_colormap_link_tb, 0, wxALIGN_CENTER, 0);
	m_colormap_link_tb->Realize();

	//right ///////////////////////////////////////////////////
	m_options_toolbar = new wxUndoableToolbar(this,wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	//ml
	bitmap = wxGetBitmap(starknot);
	m_options_toolbar->AddToolWithHelp(ID_UseMlChk, "Use Machine Learning",
		bitmap, "Generate properties using machine learning");
	//transparency
	bitmap = wxGetBitmap(transplo);
	m_options_toolbar->AddCheckTool(ID_TranspChk, "Increase Transpancy",
		bitmap, wxNullBitmap,
		"Enable High-Tarnsparency mode",
		"Enable High-Tarnsparency mode");
	//MIP
	bitmap = wxGetBitmap(mip);
	m_options_toolbar->AddCheckTool(ID_MipChk, "MIP",
		bitmap, wxNullBitmap,
		"Enable Maximum Intensity Projection (MIP) mode",
		"Enable Maximum Intensity Projection (MIP) mode");
	//inversion
	bitmap = wxGetBitmap(invert_off);
	m_options_toolbar->AddCheckTool(ID_InvChk, "Invert",
		bitmap, wxNullBitmap,
		"Invert data intensity values",
		"Invert data intensity values");
	//outline
	bitmap = wxGetBitmap(outline);
	m_options_toolbar->AddCheckTool(ID_OutlineChk, "Outline",
		bitmap, wxNullBitmap,
		"Show outlines",
		"Show outlines");
	//interpolation
	bitmap = wxGetBitmap(interpolate);
	m_options_toolbar->AddCheckTool(ID_InterpolateChk, "Trilinear",
		bitmap, wxNullBitmap,
		"Enable trilinear interpolation of voxel intensity values",
		"Enable trilinear interpolation of voxel intensity values");
	//noise reduction
	bitmap = wxGetBitmap(filter);
	m_options_toolbar->AddCheckTool(ID_NoiseReductChk, "Lanczos-Bicubic",
		bitmap, wxNullBitmap,
		"Enable Lanczos-Bicubic filtering to reduce artifacts",
		"Enable Lanczos-Bicubic filtering to reduce artifacts");
	//sync group
	bitmap = wxGetBitmap(sync_chan);
	m_options_toolbar->AddCheckTool(ID_SyncGroupChk,"Group Sync",
		bitmap, wxNullBitmap,
		"Sync current channel with other channels in the group",
		"Sync current channel with other channels in the group");
	//depth mode
	bitmap = wxGetBitmap(depth_off);
	m_options_toolbar->AddCheckTool(ID_ChannelMixDepthChk, "Depth Mode",
		bitmap, wxNullBitmap,
		"Enable Depth Mode within a group",
		"Enable Depth Mode within a group");
	//legend
	bitmap = wxGetBitmap(legend);
	m_options_toolbar->AddCheckTool(ID_LegendChk, "Legend",
		bitmap, wxNullBitmap,
		"Enable name legend display for current channel",
		"Enable name legend display for current channel");
	//buttons
	bitmap = wxGetBitmap(reset);
	m_options_toolbar->AddToolWithHelp(ID_ResetDefault,"Reset",
		bitmap, "Reset all properties");
	bitmap = wxGetBitmap(save_settings);
	m_options_toolbar->AddToolWithHelp(ID_SaveDefault,"Save",
		bitmap, "Set current settings as default");
	m_options_toolbar->Bind(wxEVT_TOOL, &VolumePropPanel::OnOptions, this);
	sizer_r1->AddStretchSpacer();
	sizer_r1->Add(m_options_toolbar, 0, wxALIGN_CENTER);
	sizer_r1->AddStretchSpacer();
	m_options_toolbar->Realize();
	//spacings
	//x
	st = new wxBoldText(this, 0, "Voxel Size",
		wxDefaultPosition, bts, wxALIGN_CENTER);
	m_space_x_text = new wxUndoableTextCtrl(this, wxID_ANY, "1.000",
		wxDefaultPosition, FromDIP(wxSize(50, -1)), wxTE_RIGHT, vald_fp3);
	m_space_x_text->Bind(wxEVT_TEXT, &VolumePropPanel::OnSpaceText, this);
	sizer_r2->Add(st, 0, wxALIGN_CENTER);
	//sizer_r2->AddStretchSpacer();
	st = new wxStaticText(this, 0, "X ");
	sizer_r2->Add(st, 0, wxALIGN_CENTER);
	sizer_r2->Add(m_space_x_text, 1, wxALIGN_CENTER);
	//y
	st = new wxStaticText(this, 0, "Y ");
	m_space_y_text = new wxUndoableTextCtrl(this, wxID_ANY, "1.000",
		wxDefaultPosition, FromDIP(wxSize(50, -1)), wxTE_RIGHT, vald_fp3);
	m_space_y_text->Bind(wxEVT_TEXT, &VolumePropPanel::OnSpaceText, this);
	sizer_r2->Add(3, 5, 0);
	sizer_r2->Add(st, 0, wxALIGN_CENTER);
	sizer_r2->Add(m_space_y_text, 1, wxALIGN_CENTER);
	//z
	st = new wxStaticText(this, 0, "Z ");
	m_space_z_text = new wxUndoableTextCtrl(this, wxID_ANY, "1.000",
		wxDefaultPosition, FromDIP(wxSize(50, -1)), wxTE_RIGHT, vald_fp3);
	m_space_z_text->Bind(wxEVT_TEXT, &VolumePropPanel::OnSpaceText, this);
	sizer_r2->Add(3, 5, 0);
	sizer_r2->Add(st, 0, wxALIGN_CENTER);
	sizer_r2->Add(m_space_z_text, 1, wxALIGN_CENTER);
	//color 1
	st = new wxBoldText(this, 0, "Main/Unsel.",
		wxDefaultPosition, bts, wxALIGN_CENTER);
	m_main_color_mode_tb = new wxUndoableToolbar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	bitmap = wxGetBitmap(comp_off);
	m_main_color_mode_tb->AddToolWithHelp(0, "Display Mode",
		bitmap, "Set display mode of main/unselected data");
	m_main_color_mode_tb->Bind(wxEVT_TOOL, &VolumePropPanel::OnMainColorMode, this);
	m_main_color_mode_tb->Realize();
	m_main_color_text = new wxTextCtrl(this, wxID_ANY, "255 , 255 , 255",
		wxDefaultPosition, tts2, wxTE_CENTER);
	m_main_color_text->Bind(wxEVT_TEXT, &VolumePropPanel::OnMainColorTextChange, this);
	m_main_color_text->Bind(wxEVT_LEFT_DCLICK, &VolumePropPanel::OnMainColorTextFocus, this);
	m_main_color_btn = new wxUndoableColorPicker(this, wxID_ANY, *wxRED,
		wxDefaultPosition, FromDIP(wxSize(50, 25)));
	m_main_color_btn->Bind(wxEVT_COLOURPICKER_CHANGED, &VolumePropPanel::OnMainColorBtn, this);
	sizer_r3->Add(st, 0, wxALIGN_CENTER, 0); 
	sizer_r3->Add(m_main_color_mode_tb, 0, wxALIGN_CENTER, 0);
	sizer_r3->Add(m_main_color_text, 1, wxALIGN_CENTER, 0);
	sizer_r3->Add(m_main_color_btn, 1, wxALIGN_CENTER, 0);
	//color 2
	st = new wxBoldText(this, 0, "Alt./Mask",
		wxDefaultPosition, bts, wxALIGN_CENTER);
	m_alt_color_mode_tb = new wxUndoableToolbar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	bitmap = wxGetBitmap(comp_off);
	m_alt_color_mode_tb->AddToolWithHelp(0, "Display Mode",
		bitmap, "Set display mode of alt/selected data");
	m_alt_color_mode_tb->Bind(wxEVT_TOOL, &VolumePropPanel::OnAltColorMode, this);
	m_alt_color_mode_tb->Realize();
	m_alt_color_text = new wxTextCtrl(this, wxID_ANY, "255 , 255 , 255",
		wxDefaultPosition, tts2, wxTE_CENTER);
	m_alt_color_text->Bind(wxEVT_TEXT, &VolumePropPanel::OnAltColorTextChange, this);
	m_alt_color_text->Bind(wxEVT_LEFT_DCLICK, &VolumePropPanel::OnAltColorTextFocus, this);
	m_alt_color_btn = new wxUndoableColorPicker(this, wxID_ANY, *wxRED,
		wxDefaultPosition, FromDIP(wxSize(50, 25)));
	m_alt_color_btn->Bind(wxEVT_COLOURPICKER_CHANGED, &VolumePropPanel::OnAltColorBtn, this);
	sizer_r4->Add(st, 0, wxALIGN_CENTER, 0);
	sizer_r4->Add(m_alt_color_mode_tb, 0, wxALIGN_CENTER, 0);
	sizer_r4->Add(m_alt_color_text, 1, wxALIGN_CENTER, 0);
	sizer_r4->Add(m_alt_color_btn, 1, wxALIGN_CENTER, 0);
	// colormap chooser
	m_colormap_inv_btn = new wxUndoableToolbar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	bitmap = wxGetBitmap(invert_off);
	m_colormap_inv_btn->AddCheckTool(0, "Invert",
		bitmap, wxNullBitmap,
		"Invert colormap",
		"Invert colormap");
	m_colormap_inv_btn->Bind(wxEVT_TOOL, &VolumePropPanel::OnColormapInvBtn, this);
	m_colormap_inv_btn->Realize();
	wxSize st_size = wxSize(bts.GetWidth() - m_colormap_inv_btn->GetSize().GetWidth(),
		bts.GetHeight());
	st = new wxBoldText(this, 0, "Effects",
		wxDefaultPosition, st_size, wxALIGN_CENTER);
	m_colormap_combo = new wxUndoableComboBox(this, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(85, 25)), 0, NULL, wxCB_READONLY);
	std::vector<wxString> colormap_list = { "Rainbow", "Main-Alt", "Hot", "Cool", "Diverging", "Monochrome", "High-key", "Low-key", "Hi Transparency" };
	m_colormap_combo->Append(colormap_list);
	m_colormap_combo->Bind(wxEVT_COMBOBOX, &VolumePropPanel::OnColormapCombo, this);
	m_colormap_combo2 = new wxUndoableComboBox(this, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(85, 25)), 0, NULL, wxCB_READONLY);
	std::vector<wxString> colormap_list2 =
		{ "Intensity", "Z Value", "Y Value", "X Value", "T Value (4D)",
		"Gradient", "Normals", "Intensity Delta (4D)", "Speed (4D)"};
	m_colormap_combo2->Append(colormap_list2);
	m_colormap_combo2->Bind(wxEVT_COMBOBOX, &VolumePropPanel::OnColormapCombo2, this);
	sizer_r5->Add(st, 0, wxALIGN_CENTER, 0);
	sizer_r5->Add(m_colormap_inv_btn, 0, wxALIGN_CENTER);
	sizer_r5->Add(m_colormap_combo, 1, wxALIGN_CENTER, 0);
	sizer_r5->Add(m_colormap_combo2, 1, wxALIGN_CENTER, 0);

	//ADD COLUMNS//////////////////////////////////////
	wxFlexGridSizer* sizer_all = new wxFlexGridSizer(3, 2, 2);
	wxBoxSizer* sizer_left = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* sizer_middle = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* sizer_right = new wxBoxSizer(wxVERTICAL);
	//left
	sizer_left->Add(sizer_l1, 1, wxEXPAND);
	sizer_left->Add(sizer_l2, 1, wxEXPAND);
	sizer_left->Add(sizer_l3, 1, wxEXPAND);
	sizer_left->Add(sizer_l4, 1, wxEXPAND);
	sizer_left->Add(sizer_l5, 1, wxEXPAND);
	//middle
	sizer_middle->Add(sizer_m1, 1, wxEXPAND);
	sizer_middle->Add(sizer_m2, 1, wxEXPAND);
	sizer_middle->Add(sizer_m3, 1, wxEXPAND);
	sizer_middle->Add(sizer_m4, 1, wxEXPAND);
	sizer_middle->Add(sizer_m5, 1, wxEXPAND);
	//right
	sizer_right->Add(sizer_r1, 1, wxEXPAND);
	sizer_right->Add(sizer_r2, 1, wxEXPAND);
	sizer_right->Add(sizer_r3, 1, wxEXPAND);
	sizer_right->Add(sizer_r4, 1, wxEXPAND);
	sizer_right->Add(sizer_r5, 1, wxEXPAND);
	//ADD ALL TOGETHER
	sizer_all->AddGrowableCol(0);
	sizer_all->AddGrowableCol(1);
	sizer_all->AddGrowableRow(0);
	sizer_all->Add(sizer_left, 1, wxEXPAND);
	sizer_all->Add(sizer_middle, 1, wxEXPAND);
	sizer_all->Add(sizer_right, 1, wxEXPAND);
	SetSizer(sizer_all);
	Layout();
	SetAutoLayout(true);
	SetScrollRate(10, 10);

	//add sliders for undo and redo
	glbin.add_undo_control(m_minmax_sldr);
	glbin.add_undo_control(m_gamma_sldr);
	glbin.add_undo_control(m_alpha_sldr);
	glbin.add_undo_control(m_luminance_sldr);
	glbin.add_undo_control(m_sample_sldr);
	glbin.add_undo_control(m_thresh_sldr);
	glbin.add_undo_control(m_boundary_sldr);
	glbin.add_undo_control(m_shading_shine_sldr);
	glbin.add_undo_control(m_shading_strength_sldr);
	glbin.add_undo_control(m_shadow_sldr);
	glbin.add_undo_control(m_shadow_dir_sldr);
	glbin.add_undo_control(m_colormap_sldr);
	//add checkboxes
	glbin.add_undo_control(m_minmax_chk);
	glbin.add_undo_control(m_gamma_chk);
	glbin.add_undo_control(m_alpha_chk);
	glbin.add_undo_control(m_luminance_chk);
	glbin.add_undo_control(m_sample_chk);
	glbin.add_undo_control(m_thresh_chk);
	glbin.add_undo_control(m_boundary_chk);
	glbin.add_undo_control(m_shade_chk);
	glbin.add_undo_control(m_shadow_chk);
	glbin.add_undo_control(m_shadow_dir_chk);
	glbin.add_undo_control(m_colormap_chk);
	//add others
	glbin.add_undo_control(m_main_color_btn);
	glbin.add_undo_control(m_alt_color_btn);
	glbin.add_undo_control(m_space_x_text);
	glbin.add_undo_control(m_space_y_text);
	glbin.add_undo_control(m_space_z_text);
	glbin.add_undo_control(m_colormap_inv_btn);
	glbin.add_undo_control(m_colormap_combo);
	glbin.add_undo_control(m_colormap_combo2);
	glbin.add_undo_control(m_options_toolbar);

	Thaw();
}

VolumePropPanel::~VolumePropPanel()
{
	//delete sliders for undo and redo
	glbin.del_undo_control(m_minmax_sldr);
	glbin.del_undo_control(m_gamma_sldr);
	glbin.del_undo_control(m_alpha_sldr);
	glbin.del_undo_control(m_luminance_sldr);
	glbin.del_undo_control(m_sample_sldr);
	glbin.del_undo_control(m_thresh_sldr);
	glbin.del_undo_control(m_boundary_sldr);
	glbin.del_undo_control(m_shading_shine_sldr);
	glbin.del_undo_control(m_shading_strength_sldr);
	glbin.del_undo_control(m_shadow_sldr);
	glbin.del_undo_control(m_shadow_dir_sldr);
	glbin.del_undo_control(m_colormap_sldr);
	//delete checkboxes
	glbin.del_undo_control(m_minmax_chk);
	glbin.del_undo_control(m_gamma_chk);
	glbin.del_undo_control(m_alpha_chk);
	glbin.del_undo_control(m_luminance_chk);
	glbin.del_undo_control(m_sample_chk);
	glbin.del_undo_control(m_thresh_chk);
	glbin.del_undo_control(m_boundary_chk);
	glbin.del_undo_control(m_shade_chk);
	glbin.del_undo_control(m_shadow_chk);
	glbin.del_undo_control(m_shadow_dir_chk);
	glbin.del_undo_control(m_colormap_chk);
	//delete others
	glbin.del_undo_control(m_main_color_btn);
	glbin.del_undo_control(m_alt_color_btn);
	glbin.del_undo_control(m_space_x_text);
	glbin.del_undo_control(m_space_y_text);
	glbin.del_undo_control(m_space_z_text);
	glbin.del_undo_control(m_colormap_inv_btn);
	glbin.del_undo_control(m_colormap_combo);
	glbin.del_undo_control(m_colormap_combo2);
	glbin.del_undo_control(m_options_toolbar);

	m_frame->SetFocusVRenderViews(0);
}

void VolumePropPanel::FluoUpdate(const fluo::ValueCollection& vc)
{
	if (FOUND_VALUE(gstNull))
		return;
	if (!m_vd)
		return;

	//std::chrono::time_point t = std::chrono::high_resolution_clock::now();

	wxString str;
	double dval = 0.0;
	int ival = 0;
	bool bval;
	fluo::Color cval;

	//maximum value
	m_max_val = m_vd->GetMaxValue();
	m_max_val = std::max(255.0, m_max_val);

	//set range
	wxFloatingPointValidator<double>* vald_fp;
	wxIntegerValidator<unsigned int>* vald_i;

	bool update_all = vc.empty() || FOUND_VALUE(gstVolumeProps);
	bool update_tips = update_all || FOUND_VALUE(gstMultiFuncTips);
	bool update_gamma = update_all || FOUND_VALUE(gstGamma3d);
	bool update_boundary = update_all || FOUND_VALUE(gstBoundary);
	bool update_minmax = update_all || FOUND_VALUE(gstMinMax);
	bool update_threshold = update_all || FOUND_VALUE(gstThreshold);
	bool update_color = update_all || FOUND_VALUE(gstColor);
	bool update_alpha = update_all || FOUND_VALUE(gstAlpha);
	bool update_luminance = update_all || FOUND_VALUE(gstLuminance);
	bool update_shading = update_all || FOUND_VALUE(gstShading);
	bool update_shadow = update_all || FOUND_VALUE(gstShadow);
	bool update_sample = update_all || FOUND_VALUE(gstSampleRate);
	bool update_colormap = update_all || FOUND_VALUE(gstColormap);
	bool update_histogram = update_all || FOUND_VALUE(gstUpdateHistogram);
	bool mf_enable = glbin_settings.m_mulfunc == 5;

	//DBGPRINT(L"update vol props, update_all=%d, vc_size=%d\n", update_all, vc.size());
	//mf button tips
	if (update_tips)
	{
		switch (glbin_settings.m_mulfunc)
		{
		case 0:
			m_gamma_st->SetToolTip("Synchronize the gamma values of all channels in the group");
			m_minmax_st->SetToolTip("Synchronize the saturation values of all channels in the group");
			m_luminance_st->SetToolTip("Synchronize the luminance values of all channels in the group");
			m_alpha_st->SetToolTip("Synchronize the alpha values of all channels in the group");
			m_shade_st->SetToolTip("Synchronize the shading values of all channels in the group");
			m_boundary_st->SetToolTip("Synchronize the boundary values of all channels in the group");
			m_thresh_st->SetToolTip("Synchronize the threshold values of all channels in the group");
			m_shadow_st->SetToolTip("Synchronize the shadow values of all channels in the group");
			m_sample_st->SetToolTip("Synchronize the sampling rate values of all channels in the group");
			m_colormap_st->SetToolTip("Synchronize the colormap values of all channels in the group");
			break;
		case 1:
			m_gamma_st->SetToolTip("Move the mouse cursor in render view and change the gamma value using the mouse wheel");
			m_minmax_st->SetToolTip("Move the mouse cursor in render view and change the saturation value using the mouse wheel");
			m_luminance_st->SetToolTip("Move the mouse cursor in render view and change the luminance value using the mouse wheel");
			m_alpha_st->SetToolTip("Move the mouse cursor in render view and change the alpha value using the mouse wheel");
			m_shade_st->SetToolTip("Move the mouse cursor in render view and change the shading value using the mouse wheel");
			m_boundary_st->SetToolTip("Move the mouse cursor in render view and change the boundary value using the mouse wheel");
			m_thresh_st->SetToolTip("Move the mouse cursor in render view and change the threshold value using the mouse wheel");
			m_shadow_st->SetToolTip("Move the mouse cursor in render view and change the shadow value using the mouse wheel");
			m_sample_st->SetToolTip("Move the mouse cursor in render view and change the sampling rate value using the mouse wheel");
			m_colormap_st->SetToolTip("Move the mouse cursor in render view and change the colormap value using the mouse wheel");
			break;
		case 2:
			m_gamma_st->SetToolTip("Reset the gamma value");
			m_minmax_st->SetToolTip("Reset the saturation value");
			m_luminance_st->SetToolTip("Reset the luminance value");
			m_alpha_st->SetToolTip("Reset the alpha value");
			m_shade_st->SetToolTip("Reset the shading value");
			m_boundary_st->SetToolTip("Reset the boundary value");
			m_thresh_st->SetToolTip("Reset the threshold value");
			m_shadow_st->SetToolTip("Reset the shadow value");
			m_sample_st->SetToolTip("Reset the sampling rate value");
			m_colormap_st->SetToolTip("Reset the colormap value");
			break;
		case 3:
			m_gamma_st->SetToolTip("Set the gamma value from machine learning");
			m_minmax_st->SetToolTip("Set the saturation value from machine learning");
			m_luminance_st->SetToolTip("Set the luminance value from machine learning");
			m_alpha_st->SetToolTip("Set the alpha value from machine learning");
			m_shade_st->SetToolTip("Set the shading value from machine learning");
			m_boundary_st->SetToolTip("Set the boundary value from machine learning");
			m_thresh_st->SetToolTip("Set the threshold value from machine learning");
			m_shadow_st->SetToolTip("Set the shadow value from machine learning");
			m_sample_st->SetToolTip("Set the sampling rate value from machine learning");
			m_colormap_st->SetToolTip("Set the colormap value from machine learning");
			break;
		case 4:
			m_gamma_st->SetToolTip("Undo the gamma value changes");
			m_minmax_st->SetToolTip("Undo the saturation value changes");
			m_luminance_st->SetToolTip("Undo the luminance value changes");
			m_alpha_st->SetToolTip("Undo the alpha value changes");
			m_shade_st->SetToolTip("Undo the shading value changes");
			m_boundary_st->SetToolTip("Undo the boundary value changes");
			m_thresh_st->SetToolTip("Undo the thresh value changes");
			m_shadow_st->SetToolTip("Undo the shadow value changes");
			m_sample_st->SetToolTip("Undo the sampling rate value changes");
			m_colormap_st->SetToolTip("Undo the colormap value changes");
			break;
		case 5:
			m_gamma_st->SetToolTip("Enable/Disable the gamma value");
			m_minmax_st->SetToolTip("Enable/Disable the saturation value");
			m_luminance_st->SetToolTip("Enable/Disable the luminance value");
			m_alpha_st->SetToolTip("Enable/Disable the alpha value");
			m_shade_st->SetToolTip("Enable/Disable the shading value");
			m_boundary_st->SetToolTip("Enable/Disable the boundary value");
			m_thresh_st->SetToolTip("Enable/Disable the thresh value");
			m_shadow_st->SetToolTip("Enable/Disable the shadow value");
			m_sample_st->SetToolTip("Enable/Disable the sampling rate value");
			m_colormap_st->SetToolTip("Enable/Disable the colormap value");
			break;
		}
	}

	//volume properties
	//histogram
	if (update_histogram)
	{
		std::vector<unsigned char> hist_data;
		if (m_vd->GetHistogram(hist_data))
		{
			wxColour lc = wxColour(0, 0, 0);
			cval = m_vd->GetColor();
			wxColour hc = wxColor((unsigned char)(cval.r() *255 + 0.5),
				(unsigned char)(cval.g() * 255 + 0.5),
				(unsigned char)(cval.b() * 255 + 0.5));
			m_minmax_sldr->SetColors(lc, hc);
			m_minmax_sldr->SetMapData(hist_data);
			m_thresh_sldr->SetMapData(hist_data);
		}
	}
	//transfer function
	//gamma
	if (update_gamma)
	{
		if ((vald_fp = (wxFloatingPointValidator<double>*)m_gamma_text->GetValidator()))
			vald_fp->SetRange(0.0, 10.0);
		dval = m_vd->GetGamma();
		bval = m_vd->GetGammaEnable();
		str = wxString::Format("%.2f", dval);
		m_gamma_sldr->ChangeValue(std::round(dval * 100.0));
		m_gamma_text->ChangeValue(str);
		m_gamma_chk->SetValue(bval);
		if (m_gamma_sldr->IsEnabled() != bval)
		{
			m_gamma_sldr->Enable(bval);
			m_gamma_text->Enable(bval);
		}
	}
	if (update_gamma || update_tips)
	{
		bval = m_vd->GetGammaEnable() || mf_enable;
		if (m_gamma_st->IsEnabled() != bval)
			m_gamma_st->Enable(bval);
	}
	//boundary
	if (update_boundary)
	{
		double gmf = 1000 / m_vd->GetBoundaryMax();
		//low
		if ((vald_fp = (wxFloatingPointValidator<double>*)m_boundary_low_text->GetValidator()))
			vald_fp->SetMin(0.0);
		dval = m_vd->GetBoundaryLow();
		m_boundary_sldr->ChangeLowValue(std::round(dval * gmf));
		str = wxString::Format("%.4f", dval);
		m_boundary_low_text->ChangeValue(str);
		//high
		if ((vald_fp = (wxFloatingPointValidator<double>*)m_boundary_high_text->GetValidator()))
			vald_fp->SetMin(0.0);
		dval = m_vd->GetBoundaryHigh();
		m_boundary_sldr->ChangeHighValue(std::round(dval * gmf));
		str = wxString::Format("%.4f", dval);
		m_boundary_high_text->ChangeValue(str);
		//link
		bval = m_boundary_sldr->GetLink();
		if (bval != m_boundary_link_tb->GetToolState(0))
		{
			m_boundary_link_tb->ToggleTool(0, bval);
			wxBitmapBundle bitmap;
			if (bval)
				bitmap = wxGetBitmap(link);
			else
				bitmap = wxGetBitmap(unlink);
			m_boundary_link_tb->SetToolNormalBitmap(0, bitmap);
		}
		//enable
		bval = m_vd->GetBoundaryEnable();
		m_boundary_chk->SetValue(bval);
		if (m_boundary_sldr->IsEnabled() != bval)
		{
			m_boundary_sldr->Enable(bval);
			m_boundary_low_text->Enable(bval);
			m_boundary_high_text->Enable(bval);
			m_boundary_link_tb->Enable(bval);
		}
	}
	if (update_boundary || update_tips)
	{
		bval = m_vd->GetBoundaryEnable() || mf_enable;
		if (m_boundary_st->IsEnabled() != bval)
			m_boundary_st->Enable(bval);
	}
	//minmax
	if (update_minmax || FOUND_VALUE(gstTransparent))
	{
		if ((vald_i = (wxIntegerValidator<unsigned int>*)m_low_offset_text->GetValidator()))
			vald_i->SetMin(0);
		dval = m_vd->GetLowOffset();
		ival = std::round(dval * m_max_val);
		if (m_vd->GetAlphaPower() > 1.1)
			m_minmax_sldr->SetRange(0, std::round(m_max_val * 2));
		else
			m_minmax_sldr->SetRange(0, std::round(m_max_val));
		str = wxString::Format("%d", ival);
		m_minmax_sldr->ChangeLowValue(ival);
		m_low_offset_text->ChangeValue(str);
		//high offset
		if ((vald_i = (wxIntegerValidator<unsigned int>*)m_high_offset_text->GetValidator()))
			vald_i->SetMin(0);
		dval = m_vd->GetHighOffset();
		ival = std::round(dval * m_max_val);
		str = wxString::Format("%d", ival);
		m_minmax_sldr->ChangeHighValue(ival);
		m_high_offset_text->ChangeValue(str);
		bval = m_minmax_sldr->GetLink();
		if (bval != m_minmax_link_tb->GetToolState(0))
		{
			m_minmax_link_tb->ToggleTool(0, bval);
			wxBitmapBundle bitmap;
			if (bval)
				bitmap = wxGetBitmap(link);
			else
				bitmap = wxGetBitmap(unlink);
			m_minmax_link_tb->SetToolNormalBitmap(0, bitmap);
		}
		bval = m_vd->GetMinMaxEnable();
		m_minmax_chk->SetValue(bval);
		if (m_minmax_sldr->IsEnabled() != bval)
		{
			m_minmax_sldr->Enable(bval);
			m_low_offset_text->Enable(bval);
			m_high_offset_text->Enable(bval);
			m_minmax_link_tb->Enable(bval);
		}
	}
	if (update_minmax || update_tips)
	{
		bval = m_vd->GetMinMaxEnable() || mf_enable;
		if (m_minmax_st->IsEnabled() != bval)
			m_minmax_st->Enable(bval);
	}
	//threshold
	if (update_threshold)
	{
		if ((vald_i = (wxIntegerValidator<unsigned int>*)m_left_thresh_text->GetValidator()))
			vald_i->SetMin(0);
		dval = m_vd->GetLeftThresh();
		ival = std::round(dval * m_max_val);
		m_thresh_sldr->SetRange(0, std::round(m_max_val));
		str = wxString::Format("%d", ival);
		m_thresh_sldr->ChangeLowValue(ival);
		m_left_thresh_text->ChangeValue(str);
		//right threshold
		if ((vald_i = (wxIntegerValidator<unsigned int>*)m_right_thresh_text->GetValidator()))
			vald_i->SetMin(0);
		dval = m_vd->GetRightThresh();
		ival = std::round(dval * m_max_val);
		str = wxString::Format("%d", ival);
		m_thresh_sldr->ChangeHighValue(ival);
		m_right_thresh_text->ChangeValue(str);
		bval = m_thresh_sldr->GetLink();
		if (bval != m_thresh_link_tb->GetToolState(0))
		{
			m_thresh_link_tb->ToggleTool(0, bval);
			wxBitmapBundle bitmap;
			if (bval)
				bitmap = wxGetBitmap(link);
			else
				bitmap = wxGetBitmap(unlink);
			m_thresh_link_tb->SetToolNormalBitmap(0, bitmap);
		}
		bval = m_vd->GetThreshEnable();
		m_thresh_chk->SetValue(bval);
		if (m_thresh_sldr->IsEnabled() != bval)
		{
			m_thresh_sldr->Enable(bval);
			m_left_thresh_text->Enable(bval);
			m_right_thresh_text->Enable(bval);
			m_thresh_link_tb->Enable(bval);
		}
	}
	if (update_threshold || update_tips)
	{
		bval = m_vd->GetThreshEnable() || mf_enable;
		if (m_thresh_st->IsEnabled() != bval)
			m_thresh_st->Enable(bval);
	}
	//alpha
	if (update_alpha)
	{
		if ((vald_i = (wxIntegerValidator<unsigned int>*)m_alpha_text->GetValidator()))
			vald_i->SetMin(0);
		dval = m_vd->GetAlpha();
		ival = std::round(dval * m_max_val);
		m_alpha_sldr->SetRange(0, std::round(m_max_val));
		str = wxString::Format("%d", ival);
		m_alpha_sldr->ChangeValue(ival);
		m_alpha_text->ChangeValue(str);
		bval = m_vd->GetAlphaEnable();
		m_alpha_chk->SetValue(bval);
		if (m_alpha_sldr->IsEnabled() != bval)
		{
			m_alpha_sldr->Enable(bval);
			m_alpha_text->Enable(bval);
		}
	}
	if (update_alpha || update_tips)
	{
		bval = m_vd->GetAlphaEnable() || mf_enable;
		if (m_alpha_st->IsEnabled() != bval)
			m_alpha_st->Enable(bval);
	}
	//luminance
	if (update_luminance)
	{
		if ((vald_i = (wxIntegerValidator<unsigned int>*)m_luminance_text->GetValidator()))
			vald_i->SetMin(0);
		dval = m_vd->GetLuminance();
		bval = m_vd->GetLuminanceEnable();
		ival = std::round(dval * m_max_val);
		m_luminance_sldr->SetRange(0, std::round(m_max_val*2));
		str = wxString::Format("%d", ival);
		m_luminance_sldr->ChangeValue(ival);
		m_luminance_text->ChangeValue(str);
		m_luminance_chk->SetValue(bval);
		if (m_luminance_sldr->IsEnabled() != bval)
		{
			m_luminance_sldr->Enable(bval);
			m_luminance_text->Enable(bval);
		}
	}
	if (update_luminance || update_tips)
	{
		bval = m_vd->GetLuminanceEnable() || mf_enable;
		if (m_luminance_st->IsEnabled() != bval)
			m_luminance_st->Enable(bval);
	}
	//shadings
	if (update_shading)
	{
		if ((vald_fp = (wxFloatingPointValidator<double>*)m_shading_strength_text->GetValidator()))
			vald_fp->SetRange(0.0, 10.0);
		if ((vald_fp = (wxFloatingPointValidator<double>*)m_shading_shine_text->GetValidator()))
			vald_fp->SetRange(0.0, 100.0);
		dval = m_vd->GetShadingStrength();
		str = wxString::Format("%.2f", dval);
		m_shading_strength_sldr->ChangeValue(dval * 100.0);
		m_shading_strength_text->ChangeValue(str);
		dval = m_vd->GetShadingShine();
		str = wxString::Format("%.2f", dval);
		m_shading_shine_sldr->ChangeValue(dval * 100.0);
		m_shading_shine_text->ChangeValue(str);
		bval = m_vd->GetShadingEnable();
		m_shade_chk->SetValue(bval);
		if (m_shading_strength_sldr->IsEnabled() != bval)
		{
			m_shading_strength_sldr->Enable(bval);
			m_shading_strength_text->Enable(bval);
			m_shading_shine_sldr->Enable(bval);
			m_shading_shine_text->Enable(bval);
		}
	}
	if (update_shading || update_tips)
	{
		bval = m_vd->GetShadingEnable() || mf_enable;
		if (m_shade_st->IsEnabled() != bval)
			m_shade_st->Enable(bval);
	}
	//shadow
	if (update_shadow)
	{
		//if ((vald_fp = (wxFloatingPointValidator<double>*)m_shadow_text->GetValidator()))
		//	vald_fp->SetRange(0.0, 1.0);
		bval = m_vd->GetShadowEnable();
		dval = m_vd->GetShadowIntensity();
		str = wxString::Format("%.2f", dval);
		m_shadow_sldr->ChangeValue(std::round(dval * 100.0));
		m_shadow_text->ChangeValue(str);
		m_shadow_chk->SetValue(bval);
		if (m_shadow_sldr->IsEnabled() != bval)
		{
			m_shadow_sldr->Enable(bval);
			m_shadow_text->Enable(bval);
		}
	}
	if (update_all || FOUND_VALUE(gstShadowDir))
	{
		bool bval = glbin_settings.m_shadow_dir;
		m_shadow_dir_chk->SetValue(bval);
		m_shadow_dir_sldr->Enable(bval);
		m_shadow_dir_text->Enable(bval);
		double dirx = glbin_settings.m_shadow_dir_x;
		double diry = glbin_settings.m_shadow_dir_y;
		if (dirx == 0.0 && diry == 0.0)
			dval = 0.0;
		else
			dval = r2d(atan2(glbin_settings.m_shadow_dir_y, glbin_settings.m_shadow_dir_x)) + 45.0;
		m_shadow_dir_sldr->ChangeValue(std::round(dval));
		m_shadow_dir_text->ChangeValue(wxString::Format("%.0f", dval));
	}
	if (update_shadow || update_tips)
	{
		bval = m_vd->GetShadowEnable() || mf_enable;
		if (m_shadow_st->IsEnabled() != bval)
			m_shadow_st->Enable(bval);
	}
	//smaple rate
	if (update_sample)
	{
		if ((vald_fp = (wxFloatingPointValidator<double>*)m_sample_text->GetValidator()))
			vald_fp->SetRange(0.0, 100.0);
		bval = m_vd->GetSampleRateEnable();
		dval = m_vd->GetSampleRate();
		str = wxString::Format("%.1f", dval);
		m_sample_sldr->ChangeValue(dval * 10.0);
		m_sample_text->ChangeValue(str);
		m_sample_chk->SetValue(bval);
		if (m_sample_sldr->IsEnabled() != bval)
		{
			m_sample_sldr->Enable(bval);
			m_sample_text->Enable(bval);
		}
	}
	if (update_sample || update_tips)
	{
		bval = m_vd->GetSampleRateEnable() || mf_enable;
		if (m_sample_st->IsEnabled() != bval)
			m_sample_st->Enable(bval);
	}

	//spacings
	if (update_all || FOUND_VALUE(gstSpacing))
	{
		auto spc = m_vd->GetBaseSpacing();
		if ((vald_fp = (wxFloatingPointValidator<double>*)m_space_x_text->GetValidator()))
			vald_fp->SetMin(0.0);
		str = wxString::Format("%.3f", spc.x());
		m_space_x_text->ChangeValue(str);
		if ((vald_fp = (wxFloatingPointValidator<double>*)m_space_y_text->GetValidator()))
			vald_fp->SetMin(0.0);
		str = wxString::Format("%.3f", spc.y());
		m_space_y_text->ChangeValue(str);
		if ((vald_fp = (wxFloatingPointValidator<double>*)m_space_z_text->GetValidator()))
			vald_fp->SetMin(0.0);
		str = wxString::Format("%.3f", spc.z());
		m_space_z_text->ChangeValue(str);
	}

	//colormap
	if (update_colormap)
	{
		//slider
		m_colormap_sldr->SetRange(0, std::round(m_max_val));
		double low, high;
		m_vd->GetColormapValues(low, high);
		//low
		ival = std::round(low * m_max_val);
		m_colormap_sldr->ChangeLowValue(ival);
		//high
		ival = std::round(high * m_max_val);
		m_colormap_sldr->ChangeHighValue(ival);

		//text
		m_vd->GetColormapDispValues(low, high);
		double minv, maxv;
		m_vd->GetColormapRange(minv, maxv);
		bool int_validator = (maxv - minv) > 10.0;
		if (int_validator)
		{
			m_colormap_low_text->SetValidator(wxIntegerValidator<int>());
			str = wxString::Format("%.0f", low);
			m_colormap_low_text->ChangeValue(str);
			m_colormap_hi_text->SetValidator(wxIntegerValidator<int>());
			str = wxString::Format("%.0f", high);
			m_colormap_hi_text->ChangeValue(str);
		}
		else
		{
			m_colormap_low_text->SetValidator(wxFloatingPointValidator<double>());
			str = wxString::Format("%.3f", low);
			m_colormap_low_text->ChangeValue(str);
			m_colormap_hi_text->SetValidator(wxFloatingPointValidator<double>());
			str = wxString::Format("%.3f", high);
			m_colormap_hi_text->ChangeValue(str);
		}

		bval = m_colormap_sldr->GetLink();
		if (bval != m_colormap_link_tb->GetToolState(0))
		{
			m_colormap_link_tb->ToggleTool(0, bval);
			wxBitmapBundle bitmap;
			if (bval)
				bitmap = wxGetBitmap(link);
			else
				bitmap = wxGetBitmap(unlink);
			m_colormap_link_tb->SetToolNormalBitmap(0, bitmap);
		}
		//mode
		bval = m_vd->GetColorMode() == flvr::ColorMode::Colormap;
		m_colormap_chk->SetValue(bval);
		if (m_colormap_sldr->IsEnabled() != bval)
		{
			m_colormap_sldr->Enable(bval);
			m_colormap_low_text->Enable(bval);
			m_colormap_hi_text->Enable(bval);
			m_colormap_link_tb->Enable(bval);
		}
		//colormap
		bval = m_vd->GetColormapInv() > 0.0 ? false : true;
		if (bval != m_colormap_inv_btn->GetToolState(0))
		{
			m_colormap_inv_btn->ToggleTool(0, bval);
			if (bval)
				m_colormap_inv_btn->SetToolNormalBitmap(0,
					wxGetBitmap(invert));
			else
				m_colormap_inv_btn->SetToolNormalBitmap(0,
					wxGetBitmap(invert_off));
		}
		m_colormap_combo->SetSelection(m_vd->GetColormap());
		flvr::ColormapProj colormap_proj = m_vd->GetColormapProj();
		ival = 0;
		if (flvr::ShaderParams::ValidColormapProj(colormap_proj))
			ival = static_cast<int>(colormap_proj) - 1;
		m_colormap_combo2->SetSelection(ival);
		//show colormap on slider
		std::vector<unsigned char> colormap_data;
		if (m_vd->GetColormapData(colormap_data))
		{
			wxColor lc, hc;
			cval = m_vd->GetColorFromColormap(0, true);
			lc = wxColor((unsigned char)(cval.r() *255 + 0.5),
				(unsigned char)(cval.g() * 255 + 0.5),
				(unsigned char)(cval.b() * 255 + 0.5));
			cval = m_vd->GetColorFromColormap(1, true);
			hc = wxColor((unsigned char)(cval.r() *255 + 0.5),
				(unsigned char)(cval.g() * 255 + 0.5),
				(unsigned char)(cval.b() * 255 + 0.5));
			m_colormap_sldr->SetColors(lc, hc);
			m_colormap_sldr->SetMapData(colormap_data);
		}
	}
	if (update_colormap || update_tips)
	{
		bval = m_vd->GetColorMode() ==
			flvr::ColorMode::Colormap || mf_enable;
		if (m_colormap_st->IsEnabled() != bval)
			m_colormap_st->Enable(bval);
	}

	//color
	if (update_color)
	{
		cval = m_vd->GetColor();
		wxColor wxc((unsigned char)(cval.r() * 255 + 0.5),
			(unsigned char)(cval.g() * 255 + 0.5),
			(unsigned char)(cval.b() * 255 + 0.5));
		m_main_color_text->ChangeValue(wxString::Format("%d , %d , %d",
			wxc.Red(), wxc.Green(), wxc.Blue()));
		m_main_color_btn->SetValue(wxc);
		cval = m_vd->GetMaskColor();
		wxc = wxColor((unsigned char)(cval.r() * 255 + 0.5),
			(unsigned char)(cval.g() * 255 + 0.5),
			(unsigned char)(cval.b() * 255 + 0.5));
		m_alt_color_text->ChangeValue(wxString::Format("%d , %d , %d",
			wxc.Red(), wxc.Green(), wxc.Blue()));
		m_alt_color_btn->SetValue(wxc);
	}

	//mask mode
	if (update_all || FOUND_VALUE(gstMainMode))
	{
		auto main_mode = m_vd->GetMainMaskMode();
		switch (main_mode)
		{
		case flvr::MaskMode::None:
			m_main_color_mode_tb->SetToolNormalBitmap(0,
				wxGetBitmap(clip_none));
			break;
		case flvr::MaskMode::SingleColor:
			m_main_color_mode_tb->SetToolNormalBitmap(0,
				wxGetBitmap(palette));
			break;
		case flvr::MaskMode::Colormap:
			m_main_color_mode_tb->SetToolNormalBitmap(0,
				wxGetBitmap(colormap));
			break;
		case flvr::MaskMode::Component:
			m_main_color_mode_tb->SetToolNormalBitmap(0,
				wxGetBitmap(comp));
			break;
		}
	}

	if (update_all || FOUND_VALUE(gstMaskMode))
	{
		auto mask_mode = m_vd->GetMaskMode();
		switch (mask_mode)
		{
		case flvr::MaskMode::None:
			m_alt_color_mode_tb->SetToolNormalBitmap(0,
				wxGetBitmap(clip_none));
			break;
		case flvr::MaskMode::SingleColor:
			m_alt_color_mode_tb->SetToolNormalBitmap(0,
				wxGetBitmap(palette));
			break;
		case flvr::MaskMode::Colormap:
			m_alt_color_mode_tb->SetToolNormalBitmap(0,
				wxGetBitmap(colormap));
			break;
		case flvr::MaskMode::Component:
			m_alt_color_mode_tb->SetToolNormalBitmap(0,
				wxGetBitmap(comp));
			break;
		}
	}

	//inversion
	if (update_all || FOUND_VALUE(gstInvert))
	{
		bool inv = m_vd->GetInvert();
		m_options_toolbar->ToggleTool(ID_InvChk, inv);
		if (inv)
			m_options_toolbar->SetToolNormalBitmap(ID_InvChk,
				wxGetBitmap(invert));
		else
			m_options_toolbar->SetToolNormalBitmap(ID_InvChk,
				wxGetBitmap(invert_off));
	}

	//MIP
	if (update_all || FOUND_VALUE(gstRenderMode))
	{
		bool mip = m_vd->GetRenderMode() == flvr::RenderMode::Mip;
		m_options_toolbar->ToggleTool(ID_MipChk, mip);
	}

	//transparency
	if (update_all || FOUND_VALUE(gstTransparent))
	{
		double alpha_power = m_vd->GetAlphaPower();
		if (alpha_power > 1.1)
		{
			m_options_toolbar->ToggleTool(ID_TranspChk, true);
			m_options_toolbar->SetToolNormalBitmap(ID_TranspChk,
				wxGetBitmap(transphi));
		}
		else
		{
			m_options_toolbar->ToggleTool(ID_TranspChk, false);
			m_options_toolbar->SetToolNormalBitmap(ID_TranspChk,
				wxGetBitmap(transplo));
		}
	}

	//legend
	if (update_all || FOUND_VALUE(gstLegend))
		m_options_toolbar->ToggleTool(ID_LegendChk, m_vd->GetLegend());

	//outline
	if (update_all || FOUND_VALUE(gstOutline))
	{
		bval = m_vd->GetOutline();
		m_options_toolbar->ToggleTool(ID_OutlineChk, bval);
		if (bval)
			m_options_toolbar->SetToolNormalBitmap(ID_OutlineChk,
				wxGetBitmap(outline));
		else
			m_options_toolbar->SetToolNormalBitmap(ID_OutlineChk,
				wxGetBitmap(outline_off));
	}

	//interpolate
	if (update_all || FOUND_VALUE(gstInterpolate))
	{
		bool interp = m_vd->GetInterpolate();
		m_options_toolbar->ToggleTool(ID_InterpolateChk, interp);
		if (interp)
			m_options_toolbar->SetToolNormalBitmap(ID_InterpolateChk,
				wxGetBitmap(interpolate));
		else
			m_options_toolbar->SetToolNormalBitmap(ID_InterpolateChk,
				wxGetBitmap(interpolate_off));
	}

	//sync group
	if (update_all || FOUND_VALUE(gstSyncGroup))
	{
		if (m_group)
			m_sync_group = m_group->GetVolumeSyncProp();
		m_options_toolbar->ToggleTool(ID_SyncGroupChk, m_sync_group);
	}

	//noise reduction
	if (update_all || FOUND_VALUE(gstNoiseRedct))
	{
		bool nr = m_vd->GetNR();
		m_options_toolbar->ToggleTool(ID_NoiseReductChk, nr);
		if (nr)
			m_options_toolbar->SetToolNormalBitmap(ID_NoiseReductChk,
				wxGetBitmap(filter));
		else
			m_options_toolbar->SetToolNormalBitmap(ID_NoiseReductChk,
				wxGetBitmap(filter_off));
	}

	//blend mode
	if (update_all || FOUND_VALUE(gstChannelMixMode))
	{
		auto channel_mix_mode = m_vd->GetChannelMixMode();
		if (channel_mix_mode == ChannelMixMode::Depth)
		{
			m_options_toolbar->ToggleTool(ID_ChannelMixDepthChk, true);
			m_options_toolbar->SetToolNormalBitmap(ID_ChannelMixDepthChk, wxGetBitmap(depth));
		}
		else
		{
			m_options_toolbar->ToggleTool(ID_ChannelMixDepthChk, false);
			m_options_toolbar->SetToolNormalBitmap(ID_ChannelMixDepthChk, wxGetBitmap(depth_off));
		}
	}

	//std::chrono::duration<double> ts = std::chrono::duration_cast<std::chrono::duration<double>>(
	//	std::chrono::high_resolution_clock::now() - t);
	//DBGPRINT(L"update settings, time: %f\n", ts);
	//return;
}

void VolumePropPanel::SetVolumeData(VolumeData* vd)
{
	if (m_vd != vd)
		ClearUndo();

	m_vd = vd;
	FluoUpdate();
}

VolumeData* VolumePropPanel::GetVolumeData()
{
	return m_vd;
}

void VolumePropPanel::InitViews(unsigned int type)
{
	Root* root = glbin_data_manager.GetRoot();
	if (root)
	{
		for (int i = 0; i < root->GetViewNum(); i++)
		{
			auto view = root->GetView(i);
			if (view)
			{
				view->InitView(type);
			}
		}
	}
}

void VolumePropPanel::SetGroup(VolumeGroup* group)
{
	m_group = group;
	if (m_group)
	{
		m_sync_group = m_group->GetVolumeSyncProp();
		m_options_toolbar->ToggleTool(ID_SyncGroupChk,m_sync_group);
	}
}

VolumeGroup* VolumePropPanel::GetGroup()
{
	return m_group;
}

void VolumePropPanel::SetView(RenderView *view)
{
	m_view = view;
}

RenderView* VolumePropPanel::GetView()
{
	return m_view;
}

void VolumePropPanel::ApplyMl()
{
	if (m_sync_group && m_group)
		m_group->ApplyMlVolProp();
	else if (m_vd)
		m_vd->ApplyMlVolProp();
	FluoRefresh(0, { gstVolumeProps, gstConvVolMeshUpdateTransf }, { glbin_current.GetViewId() });
}

void VolumePropPanel::SaveMl()
{
	std::vector<float> val;
	val.push_back(float(m_vd->GetBoundaryLow()));
	val.push_back(float(m_vd->GetGamma()));
	val.push_back(float(m_vd->GetLowOffset()));
	val.push_back(float(m_vd->GetHighOffset()));
	val.push_back(float(m_vd->GetLeftThresh()));
	val.push_back(float(m_vd->GetRightThresh()));
	val.push_back(float(m_vd->GetShadingStrength()));
	val.push_back(float(m_vd->GetShadingShine()));
	val.push_back(float(m_vd->GetAlpha()));
	val.push_back(float(m_vd->GetSampleRate()));
	val.push_back(float(m_vd->GetLuminance()));
	val.push_back(float(m_vd->GetColorMode() == flvr::ColorMode::Colormap));
	val.push_back(float(m_vd->GetColormapInv()));
	val.push_back(float(m_vd->GetColormap()));
	val.push_back(float(m_vd->GetColormapProj()));
	val.push_back(float(m_vd->GetColormapLow()));
	val.push_back(float(m_vd->GetColormapHigh()));
	val.push_back(float(m_vd->GetAlphaEnable()));
	val.push_back(float(m_vd->GetShadingEnable()));
	val.push_back(float(m_vd->GetInterpolate()));
	val.push_back(float(m_vd->GetInvert()));
	val.push_back(float(m_vd->GetRenderMode() == flvr::RenderMode::Mip));
	val.push_back(float(m_vd->GetTransparent()));
	val.push_back(float(m_vd->GetNR()));
	val.push_back(float(m_vd->GetShadowEnable()));
	val.push_back(float(m_vd->GetShadowIntensity()));
	flrd::EntryParams* ep = flrd::Reshape::get_entry_params("vol_prop", &val[0]);

	//histogram
	flrd::Histogram histogram(m_vd);
	histogram.SetProgressFunc(glbin_data_manager.GetProgressFunc());
	histogram.SetUseMask(false);
	flrd::EntryHist* eh = histogram.GetEntryHist();

	if (eh)
	{
		//record
		flrd::RecordHistParams* rec = new flrd::RecordHistParams();
		rec->setInput(eh);
		rec->setOutput(ep);
		//table
		glbin.get_vp_table().addRecord(rec);
	}
}

void VolumePropPanel::ClearUndo()
{
	m_gamma_sldr->Clear();
	m_minmax_sldr->Clear();
	m_luminance_sldr->Clear();
	m_alpha_sldr->Clear();
	m_shading_shine_sldr->Clear();
	m_shading_strength_sldr->Clear();
	m_boundary_sldr->Clear();
	m_thresh_sldr->Clear();
	m_shadow_sldr->Clear();
	m_shadow_dir_sldr->Clear();
	m_sample_sldr->Clear();
	m_colormap_sldr->Clear();
}

//enable/disable
void VolumePropPanel::EnableGamma(bool bval)
{
	if (m_sync_group && m_group)
		m_group->SetGammaEnable(bval);
	else if (m_vd)
		m_vd->SetGammaEnable(bval);

	FluoRefresh(0, { gstGamma3d }, { glbin_current.GetViewId() });
}

void VolumePropPanel::EnableMinMax(bool bval)
{
	if (m_sync_group && m_group)
		m_group->SetMinMaxEnable(bval);
	else if (m_vd)
		m_vd->SetMinMaxEnable(bval);

	FluoRefresh(0, { gstMinMax }, { glbin_current.GetViewId() });
}

void VolumePropPanel::EnableLuminance(bool bval)
{
	if (m_sync_group && m_group)
		m_group->SetLuminanceEnable(bval);
	else if (m_vd)
		m_vd->SetLuminanceEnable(bval);

	FluoRefresh(0, { gstColor }, { glbin_current.GetViewId() });
}

void VolumePropPanel::EnableAlpha(bool bval)
{
	assert(m_group);
	assert(m_vd);
	if (m_sync_group)
		m_group->SetAlphaEnable(bval);
	else
		m_vd->SetAlphaEnable(bval);

	FluoRefresh(0, { gstAlpha }, { glbin_current.GetViewId() });
}

void VolumePropPanel::EnableShading(bool bval)
{
	if (m_sync_group && m_group)
		m_group->SetShadingEnable(bval);
	else if (m_vd)
		m_vd->SetShadingEnable(bval);

	FluoRefresh(0, { gstShading }, { glbin_current.GetViewId() });
}

void VolumePropPanel::EnableBoundary(bool bval)
{
	if (m_sync_group && m_group)
		m_group->SetBoundaryEnable(bval);
	else if (m_vd)
		m_vd->SetBoundaryEnable(bval);

	FluoRefresh(0, { gstBoundary }, { glbin_current.GetViewId() });
}

void VolumePropPanel::EnableThresh(bool bval)
{
	if (m_sync_group && m_group)
		m_group->SetThreshEnable(bval);
	else if (m_vd)
		m_vd->SetThreshEnable(bval);

	FluoRefresh(0, { gstThreshold }, { glbin_current.GetViewId() });
}

void VolumePropPanel::EnableShadow(bool bval)
{
	if (m_sync_group && m_group)
		m_group->SetShadowEnable(bval);
	else if (m_group && m_group->GetChannelMixMode() == ChannelMixMode::Depth)
		m_group->SetShadowEnable(bval);
	else if (m_vd)
		m_vd->SetShadowEnable(bval);

	FluoRefresh(0, { gstShadow }, { glbin_current.GetViewId() });
}

void VolumePropPanel::EnableShadowDir(bool bval)
{
	if (glbin_settings.m_shadow_dir == bval)
		return;

	glbin_settings.m_shadow_dir = bval;
	if (bval)
	{
		wxString str;
		str = m_shadow_dir_text->GetValue();
		double deg;
		str.ToDouble(&deg);
		deg -= 45.0;
		glbin_settings.m_shadow_dir_x = cos(d2r(deg));
		glbin_settings.m_shadow_dir_y = sin(d2r(deg));
	}
	else
	{
		glbin_settings.m_shadow_dir_x = 0.0;
		glbin_settings.m_shadow_dir_y = 0.0;
	}
	FluoRefresh(0, { gstShadowDir }, { glbin_current.GetViewId() });
}

void VolumePropPanel::EnableSample(bool bval)
{
	if (m_sync_group && m_group)
		m_group->SetSampleRateEnable(bval);
	else if (m_vd)
		m_vd->SetSampleRateEnable(bval);

	FluoRefresh(0, { gstSampleRate }, { glbin_current.GetViewId() });
}

void VolumePropPanel::EnableColormap(bool bval)
{
	assert(m_view);
	assert(m_group);
	assert(m_vd);
	bool mip_enable = m_vd->GetRenderMode() == flvr::RenderMode::Mip;
	bool sync_view_depth_mip = mip_enable && m_view->GetChannelMixMode() == ChannelMixMode::Depth;
	bool sync_group_depth_mip = mip_enable && m_vd->GetChannelMixMode() == ChannelMixMode::Depth;
	if (sync_view_depth_mip)
	{
		m_view->SetColorMode(bval ? flvr::ColorMode::Colormap : flvr::ColorMode::SingleColor);
		m_view->SetColormapDisp(bval);
		//m_view->SetLabelMode(bval ? 0 : 1);
	}
	else if (m_sync_group || sync_group_depth_mip)
	{
		m_group->SetColorMode(bval ? flvr::ColorMode::Colormap : flvr::ColorMode::SingleColor);
		m_group->SetColormapDisp(bval);
		//m_group->SetLabelMode(bval ? 0 : 1);
	}
	else
	{
		m_vd->SetColorMode(bval ? flvr::ColorMode::Colormap : flvr::ColorMode::SingleColor);
		m_vd->SetColormapDisp(bval);
		//m_vd->SetLabelMode(bval ? 0 : 1);
	}

	FluoRefresh(0, { gstColormap, gstMainMode, gstMaskMode, gstUpdateSync }, { glbin_current.GetViewId() });
}

void VolumePropPanel::EnableMip(bool bval)
{
	assert(m_view);
	assert(m_group);
	assert(m_vd);
	bool depth_view = m_view->GetChannelMixMode() == ChannelMixMode::Depth;
	bool depth_group = m_vd->GetChannelMixMode() == ChannelMixMode::Depth;
	if (depth_view)
		m_view->SetRenderMode(bval ? flvr::RenderMode::Mip : flvr::RenderMode::Standard);
	else if (m_sync_group || depth_group)
		m_group->SetRenderMode(bval ? flvr::RenderMode::Mip : flvr::RenderMode::Standard);
	else
		m_vd->SetRenderMode(bval ? flvr::RenderMode::Mip : flvr::RenderMode::Standard);

	FluoRefresh(0, { gstRenderMode }, { glbin_current.GetViewId() });
}

void VolumePropPanel::EnableTransparent(bool bval)
{
	if (m_sync_group && m_group)
		m_group->SetTransparent(bval);
	else if (m_vd)
		m_vd->SetTransparent(bval);

	FluoRefresh(0, { gstTransparent }, { glbin_current.GetViewId() });
}

//set values
void VolumePropPanel::SetGamma(double val, bool notify)
{
	if (!m_vd)
		return;
	if (m_vd->GetGamma() == val)
		return;

	m_vd->SetGamma(val);
	if (notify)
		FluoRefresh(0, { gstGamma3d, gstConvVolMeshUpdateTransf }, { glbin_current.GetViewId() });
	else
		FluoRefresh(0, { gstConvVolMeshUpdateTransf }, { glbin_current.GetViewId() });
}

void VolumePropPanel::SetMinMax(double val1, double val2, bool notify)
{
	if (!m_vd)
		return;
	if (m_vd->GetLowOffset() == val1 &&
		m_vd->GetHighOffset() == val2)
		return;

	m_vd->SetLowOffset(val1);
	m_vd->SetHighOffset(val2);

	if (notify)
		FluoRefresh(0, { gstMinMax, gstConvVolMeshUpdateTransf }, { glbin_current.GetViewId() });
	else
		FluoRefresh(0, { gstConvVolMeshUpdateTransf }, { glbin_current.GetViewId() });
}

void VolumePropPanel::SetLuminance(double val, bool notify)
{
	if (!m_vd)
		return;
	if (m_vd->GetLuminance() == val)
		return;

	m_vd->SetLuminance(val);

	if (notify)
		FluoRefresh(0, { gstLuminance }, { glbin_current.GetViewId() });
	else
		FluoRefresh(0, { gstNull }, { glbin_current.GetViewId() });
}

void VolumePropPanel::SetAlpha(double val, bool notify)
{
	if (!m_vd)
		return;
	if (m_vd->GetAlpha() == val)
		return;

	m_vd->SetAlpha(val);
	if (notify)
		FluoRefresh(0, { gstAlpha }, { glbin_current.GetViewId() });
	else
		FluoRefresh(0, { gstNull }, { glbin_current.GetViewId() });
}

void VolumePropPanel::SetShadingStrength(double val, bool notify)
{
	if (!m_vd)
		return;
	if (m_vd->GetShadingStrength() == val)
		return;

	m_vd->SetShadingStrength(val);
	if (notify)
		FluoRefresh(0, { gstShading }, { glbin_current.GetViewId() });
	else
		FluoRefresh(0, { gstNull }, { glbin_current.GetViewId() });
}

void VolumePropPanel::SetShadingShine(double val, bool notify)
{
	if (!m_vd)
		return;
	if (m_vd->GetShadingShine() == val)
		return;

	m_vd->SetShadingShine(val);
	if (notify)
		FluoRefresh(0, { gstShading }, { glbin_current.GetViewId() });
	else
		FluoRefresh(0, { gstNull }, { glbin_current.GetViewId() });
}

void VolumePropPanel::SetBoundary(double val1, double val2, bool notify)
{
	if (!m_vd)
		return;
	if (m_vd->GetBoundaryLow() == val1 &&
		m_vd->GetBoundaryHigh() == val2)
		return;

	m_vd->SetBoundaryLow(val1);
	m_vd->SetBoundaryHigh(val2);

	if (notify)
		FluoRefresh(0, { gstBoundary, gstConvVolMeshUpdateTransf }, { glbin_current.GetViewId() });
	else
		FluoRefresh(0, { gstConvVolMeshUpdateTransf }, { glbin_current.GetViewId() });
}

void VolumePropPanel::SetThresh(double val1, double val2, bool notify)
{
	if (!m_vd)
		return;
	if (m_vd->GetLeftThresh() == val1 &&
		m_vd->GetRightThresh() == val2)
		return;

	m_vd->SetLeftThresh(val1);
	m_vd->SetRightThresh(val2);

	fluo::ValueCollection vc = { gstBrushCountAutoUpdate, gstColocalAutoUpdate, gstConvVolMeshUpdateTransf };
	if (notify)
		vc.insert(gstThreshold);

	FluoRefresh(0, vc, { glbin_current.GetViewId() });
}

void VolumePropPanel::SetShadowInt(double val, bool notify)
{
	if (!m_vd)
		return;
	if (m_vd->GetShadowIntensity() == val)
		return;

	m_vd->SetShadowIntensity(val);
	if (notify)
		FluoRefresh(0, { gstShadow }, { glbin_current.GetViewId() });
	else
		FluoRefresh(0, { gstNull }, { glbin_current.GetViewId() });
}

void VolumePropPanel::SetShadowDir(double dval, bool notify)
{
	dval -= 45.0;
	double dir_x = cos(d2r(dval));
	double dir_y = sin(d2r(dval));
	if (glbin_settings.m_shadow_dir_x == dir_x &&
		glbin_settings.m_shadow_dir_y == dir_y)
		return;

	glbin_settings.m_shadow_dir_x = dir_x;
	glbin_settings.m_shadow_dir_y = dir_y;
	if (notify)
		FluoRefresh(0, { gstShadowDir }, { glbin_current.GetViewId() });
	else
		FluoRefresh(0, { gstNull }, { glbin_current.GetViewId() });
}

void VolumePropPanel::SetSampleRate(double val, bool notify)
{
	if (!m_vd)
		return;
	if (m_vd->GetSampleRate() == val)
		return;

	m_vd->SetSampleRate(val);
	if (notify)
		FluoRefresh(0, { gstSampleRate }, { glbin_current.GetViewId() });
	else
		FluoRefresh(0, { gstNull }, { glbin_current.GetViewId() });
}

void VolumePropPanel::SetColormapVal(double val1, double val2, bool notify)
{
	if (!m_vd)
		return;
	if (m_vd->GetColormapLow() == val1 &&
		m_vd->GetColormapHigh() == val2)
		return;

	m_vd->SetColormapValues(val1, val2);
	if (notify)
		FluoRefresh(0, { gstColormap }, { glbin_current.GetViewId() });
	else
		FluoRefresh(0, { gstNull }, { glbin_current.GetViewId() });
}


//sync values
void VolumePropPanel::SyncGamma(double val)
{
	if (!m_group)
		return;
	if (m_group->GetGamma() == val)
		return;

	m_group->SetGamma(val);
	FluoRefresh(1, { gstGamma3d }, { glbin_current.GetViewId() });
}

void VolumePropPanel::SyncMinMax(double val1, double val2)
{
	if (!m_group)
		return;
	if (m_group->GetLowOffset() == val1 &&
		m_group->GetHighOffset() == val2)
		return;

	m_group->SetLowOffset(val1);
	m_group->SetHighOffset(val2);

	FluoRefresh(1, { gstMinMax }, { glbin_current.GetViewId() });
}

void VolumePropPanel::SyncLuminance(double val)
{
	if (!m_group)
		return;
	if (m_group->GetLuminance() == val)
		return;

	m_group->SetLuminance(val);
	FluoRefresh(1, { gstColor, gstTreeColors, gstClipPlaneRangeColor }, { glbin_current.GetViewId() });
}

void VolumePropPanel::SyncAlpha(double val)
{
	if (!m_group)
		return;
	if (m_group->GetAlpha() == val)
		return;

	m_group->SetAlpha(val);
	FluoRefresh(1, { gstAlpha }, { glbin_current.GetViewId() });
}

void VolumePropPanel::SyncShadingStrength(double val)
{
	if (!m_group)
		return;
	if (m_group->GetShadingStrength() == val)
		return;

	m_group->SetShadingStrength(val);
	FluoRefresh(1, { gstShading }, { glbin_current.GetViewId() });
}

void VolumePropPanel::SyncShadingShine(double val)
{
	if (!m_group)
		return;
	if (m_group->GetShadingShine() == val)
		return;

	m_group->SetShadingShine(val);
	FluoRefresh(1, { gstShading }, { glbin_current.GetViewId() });
}

void VolumePropPanel::SyncBoundary(double val1, double val2)
{
	if (!m_group)
		return;
	if (m_group->GetBoundaryLow() == val1 &&
		m_group->GetBoundaryHigh() == val2)
		return;

	m_group->SetBoundaryLow(val1);
	m_group->SetBoundaryHigh(val2);

	FluoRefresh(1, { gstBoundary }, { glbin_current.GetViewId() });
}

void VolumePropPanel::SyncThresh(double val1, double val2)
{
	if (!m_group)
		return;
	if (m_group->GetLeftThresh() == val1 &&
		m_group->GetRightThresh() == val2)
		return;

	m_group->SetLeftThresh(val1);
	m_group->SetRightThresh(val2);

	FluoRefresh(1, { gstThreshold, gstBrushCountAutoUpdate, gstColocalAutoUpdate }, {glbin_current.GetViewId()});
}

void VolumePropPanel::SyncShadowInt(double val)
{
	if (!m_group)
		return;
	if (m_group->GetShadowIntensity() == val)
		return;

	m_group->SetShadowIntensity(val);
	FluoRefresh(1, { gstShadow }, { glbin_current.GetViewId() });
}

void VolumePropPanel::SyncSampleRate(double val)
{
	assert(m_view);
	assert(m_group);

	bool depth_view = m_view->GetChannelMixMode() == ChannelMixMode::Depth;
	if (depth_view)
		m_view->SetSampleRate(val);
	else
		m_group->SetSampleRate(val);

	FluoRefresh(1, { gstSampleRate }, { glbin_current.GetViewId() });
}

void VolumePropPanel::SyncColormapVal(double val1, double val2)
{
	if (!m_group)
		return;
	if (m_group->GetColormapLow() == val1 &&
		m_group->GetColormapHigh() == val2)
		return;

	m_group->SetColormapValues(val1, val2);
	FluoRefresh(1, { gstColormap }, { glbin_current.GetViewId() });
}


//1
void VolumePropPanel::OnGammaMF(wxCommandEvent& event)
{
	if (!m_vd)
		return;

	switch (glbin_settings.m_mulfunc)
	{
	case 0:
		SyncGamma(m_vd->GetGamma());
		break;
	case 1:
		m_frame->SetFocusVRenderViews(m_gamma_sldr);
		break;
	case 2:
		SetGamma(glbin_vol_def.m_gamma, true);
		break;
	case 3:
		SetGamma(m_vd->GetMlGamma(), true);
		break;
	case 4:
		m_gamma_sldr->Undo();
		break;
	case 5:
		EnableGamma(!m_vd->GetGammaEnable());
		break;
	}
}

void VolumePropPanel::OnGammaChange(wxScrollEvent & event)
{
	double val = m_gamma_sldr->GetValue() / 100.0;
	wxString str = wxString::Format("%.2f", val);
	if (str != m_gamma_text->GetValue())
		m_gamma_text->ChangeValue(str);

	//set gamma value
	if (m_sync_group)
		SyncGamma(val);
	else
		SetGamma(val, false);
}

void VolumePropPanel::OnGammaText(wxCommandEvent& event)
{
	wxString str = m_gamma_text->GetValue();
	double val = 0.0;
	str.ToDouble(&val);
	m_gamma_sldr->ChangeValue(std::round(val * 100));

	//set gamma value
	if (m_sync_group)
		SyncGamma(val);
	else
		SetGamma(val, false);
}

void VolumePropPanel::OnGammaChk(wxCommandEvent& event)
{
	bool val = m_gamma_chk->GetValue();
	EnableGamma(val);
}

void VolumePropPanel::OnMinMaxMF(wxCommandEvent& event)
{
	if (!m_vd)
		return;

	switch (glbin_settings.m_mulfunc)
	{
	case 0:
		SyncMinMax(m_vd->GetLowOffset(), m_vd->GetHighOffset());
		break;
	case 1:
		m_frame->SetFocusVRenderViews(m_minmax_sldr);
		break;
	case 2:
		SetMinMax(glbin_vol_def.m_lo_offset, glbin_vol_def.m_hi_offset, true);
		break;
	case 3:
		SetMinMax(m_vd->GetMlLowOffset(), m_vd->GetMlHighOffset(), true);
		break;
	case 4:
		m_minmax_sldr->Undo();
		break;
	case 5:
		EnableMinMax(!m_vd->GetMinMaxEnable());
		break;
	}
}

void VolumePropPanel::OnMinMaxChange(wxScrollEvent & event)
{
	int ival1 = m_minmax_sldr->GetLowValue();
	int ival2 = m_minmax_sldr->GetHighValue();
	double val1 = ival1 / m_max_val;
	double val2 = ival2 / m_max_val;
	m_low_offset_text->ChangeValue(wxString::Format("%d", ival1));
	m_high_offset_text->ChangeValue(wxString::Format("%d", ival2));

	//set saturation value
	if (m_sync_group)
		SyncMinMax(val1, val2);
	else
		SetMinMax(val1, val2, false);
}

void VolumePropPanel::OnMinMaxText(wxCommandEvent& event)
{
	wxObject* t = event.GetEventObject();
	long ival1 = 0, ival2 = 0;
	wxString str = m_low_offset_text->GetValue();
	str.ToLong(&ival1);
	str = m_high_offset_text->GetValue();
	str.ToLong(&ival2);
	int low = ival1;
	int hi = ival2;
	if (double(hi) > m_max_val)
		UpdateMaxVal(hi);
	m_minmax_sldr->ChangeValues(low, hi);
	if (low != ival1 && t != m_low_offset_text)
		m_low_offset_text->ChangeValue(std::to_string(low));
	if (hi != ival2 && t != m_high_offset_text)
		m_high_offset_text->ChangeValue(std::to_string(hi));
	double val1 = double(low) / m_max_val;
	double val2 = double(hi) / m_max_val;

	//set minmax value
	if (m_sync_group)
		SyncMinMax(val1, val2);
	else
		SetMinMax(val1, val2, false);

}

void VolumePropPanel::OnMinMaxLink(wxCommandEvent& event)
{
	bool val = m_minmax_sldr->GetLink();
	val = !val;
	m_minmax_sldr->SetLink(val);
	m_minmax_link_tb->ToggleTool(0, val);
	wxBitmapBundle bitmap;
	if (val)
		bitmap = wxGetBitmap(link);
	else
		bitmap = wxGetBitmap(unlink);
	m_minmax_link_tb->SetToolNormalBitmap(0, bitmap);
}

void VolumePropPanel::OnMinMaxChk(wxCommandEvent& event)
{
	bool val = m_minmax_chk->GetValue();
	EnableMinMax(val);
}

void VolumePropPanel::OnLuminanceMF(wxCommandEvent& event)
{
	if (!m_vd)
		return;

	switch (glbin_settings.m_mulfunc)
	{
	case 0:
		SyncLuminance(m_vd->GetLuminance());
		break;
	case 1:
		m_frame->SetFocusVRenderViews(m_luminance_sldr);
		break;
	case 2:
		SetLuminance(glbin_vol_def.m_luminance, true);
		break;
	case 3:
		SetLuminance(m_vd->GetMlLuminance(), true);
		break;
	case 4:
		m_luminance_sldr->Undo();
		break;
	case 5:
		EnableLuminance(!m_vd->GetLuminanceEnable());
		break;
	}
}

void VolumePropPanel::OnLuminanceChange(wxScrollEvent& event)
{
	int ival = m_luminance_sldr->GetValue();
	wxString str = wxString::Format("%d", ival);
	if (str != m_luminance_text->GetValue())
		m_luminance_text->ChangeValue(str);
	double val = ival / m_max_val;

	//set saturation value
	if (m_sync_group)
		SyncLuminance(val);
	else
		SetLuminance(val, false);
}

void VolumePropPanel::OnLuminanceText(wxCommandEvent& event)
{
	wxString str = m_luminance_text->GetValue();
	long ival = 0;
	str.ToLong(&ival);
	if (double(ival) > m_max_val)
		UpdateMaxVal(ival);
	m_luminance_sldr->ChangeValue(ival);
	double val = ival / m_max_val;

	//set saturation value
	if (m_sync_group)
		SyncLuminance(val);
	else
		SetLuminance(val, false);
}

void VolumePropPanel::OnLuminanceChk(wxCommandEvent& event)
{
	bool val = m_luminance_chk->GetValue();
	EnableLuminance(val);
}

void VolumePropPanel::OnAlphaMF(wxCommandEvent& event)
{
	if (!m_vd)
		return;

	switch (glbin_settings.m_mulfunc)
	{
	case 0:
		SyncAlpha(m_vd->GetAlpha());
		break;
	case 1:
		m_frame->SetFocusVRenderViews(m_alpha_sldr);
		break;
	case 2:
		SetAlpha(glbin_vol_def.m_alpha, true);
		break;
	case 3:
		SetAlpha(m_vd->GetMlAlpha(), true);
		break;
	case 4:
		m_alpha_sldr->Undo();
		break;
	case 5:
		EnableAlpha(!m_vd->GetAlphaEnable());
		break;
	}
}

void VolumePropPanel::OnAlphaCheck(wxCommandEvent& event)
{
	bool val = m_alpha_chk->GetValue();
	EnableAlpha(val);
}

void VolumePropPanel::OnAlphaChange(wxScrollEvent& event)
{
	int ival = m_alpha_sldr->GetValue();
	wxString str = wxString::Format("%d", ival);
	if (str != m_alpha_text->GetValue())
		m_alpha_text->ChangeValue(str);
	double val = double(ival) / m_max_val;

	//set alpha value
	if (m_sync_group)
		SyncAlpha(val);
	else
		SetAlpha(val, false);
}

void VolumePropPanel::OnAlphaText(wxCommandEvent& event)
{
	wxString str = m_alpha_text->GetValue();
	long ival = 0;
	str.ToLong(&ival);
	if (double(ival) > m_max_val)
		UpdateMaxVal(ival);
	m_alpha_sldr->ChangeValue(ival);
	double val = double(ival) / m_max_val;

	//set alpha value
	if (m_sync_group)
		SyncAlpha(val);
	else
		SetAlpha(val, false);
}

void VolumePropPanel::OnShadingMF(wxCommandEvent& event)
{
	if (!m_vd)
		return;

	switch (glbin_settings.m_mulfunc)
	{
	case 0:
		{
			SyncShadingStrength(m_vd->GetShadingStrength());
			SyncShadingShine(m_vd->GetShadingShine());
		}
		break;
	case 1:
		m_frame->SetFocusVRenderViews(m_shading_strength_sldr);
		break;
	case 2:
		SetShadingStrength(glbin_vol_def.m_shading_strength, true);
		SetShadingShine(glbin_vol_def.m_shading_shine, true);
		break;
	case 3:
		SetShadingStrength(m_vd->GetMlShadingStrength(), true);
		SetShadingShine(m_vd->GetMlShadingShine(), true);
		break;
	case 4:
		m_shading_strength_sldr->Undo();
		break;
	case 5:
		EnableShading(!m_vd->GetShadingEnable());
		break;
	}
}

void VolumePropPanel::OnShadingStrengthChange(wxScrollEvent& event)
{
	double val = m_shading_strength_sldr->GetValue() / 100.0;
	wxString str = wxString::Format("%.2f", val);
	if (str != m_shading_strength_text->GetValue())
		m_shading_strength_text->ChangeValue(str);

	//set low shading value
	if (m_sync_group)
		SyncShadingStrength(val);
	else
		SetShadingStrength(val, false);
}

void VolumePropPanel::OnShadingStrengthText(wxCommandEvent& event)
{
	wxString str = m_shading_strength_text->GetValue();
	double val = 0.0;
	str.ToDouble(&val);
	m_shading_strength_sldr->ChangeValue(std::round(val * 100));

	//set low shading value
	if (m_sync_group)
		SyncShadingStrength(val);
	else
		SetShadingStrength(val, false);
}

void VolumePropPanel::OnShadingShineChange(wxScrollEvent& event)
{
	double val = m_shading_shine_sldr->GetValue() / 100.0;
	wxString str = wxString::Format("%.2f", val);
	if (str != m_shading_shine_text->GetValue())
		m_shading_shine_text->ChangeValue(str);

	//set high shading value
	if (m_sync_group)
		SyncShadingShine(val);
	else
		SetShadingShine(val, false);
}

void VolumePropPanel::OnShadingShineText(wxCommandEvent& event)
{
	wxString str = m_shading_shine_text->GetValue();
	double val = 0.0;
	str.ToDouble(&val);
	m_shading_shine_sldr->ChangeValue(std::round(val * 100));

	//set high shading value
	if (m_sync_group)
		SyncShadingShine(val);
	else
		SetShadingShine(val, false);
}

void VolumePropPanel::OnShadingChk(wxCommandEvent& event)
{
	bool val = m_shade_chk->GetValue();
	EnableShading(val);
}

void VolumePropPanel::OnBoundaryMF(wxCommandEvent& event)
{
	if (!m_vd)
		return;

	switch (glbin_settings.m_mulfunc)
	{
	case 0:
		SyncBoundary(m_vd->GetBoundaryLow(), m_vd->GetBoundaryHigh());
		break;
	case 1:
		m_frame->SetFocusVRenderViews(m_boundary_sldr);
		break;
	case 2:
		SetBoundary(glbin_vol_def.m_boundary_low, glbin_vol_def.m_boundary_high, true);
		break;
	case 3:
		SetBoundary(m_vd->GetMlBoundaryLow(), m_vd->GetMlBoundaryHigh(), true);
		break;
	case 4:
		m_boundary_sldr->Undo();
		break;
	case 5:
		EnableBoundary(!m_vd->GetBoundaryEnable());
		break;
	}
}

void VolumePropPanel::OnBoundaryChange(wxScrollEvent& event)
{
	double gmf = 1000 / m_vd->GetBoundaryMax();
	double val1 = m_boundary_sldr->GetLowValue() / gmf;
	double val2 = m_boundary_sldr->GetHighValue() / gmf;
	m_boundary_low_text->ChangeValue(wxString::Format("%.4f", val1));
	m_boundary_high_text->ChangeValue(wxString::Format("%.4f", val2));

	//set boundary value
	if (m_sync_group)
		SyncBoundary(val1, val2);
	else
		SetBoundary(val1, val2, false);
}

void VolumePropPanel::OnBoundaryText(wxCommandEvent& event)
{
	double gmf = 1000 / m_vd->GetBoundaryMax();
	wxObject* t = event.GetEventObject();
	double val1 = 0.0, val2 = 0.0;
	wxString str = m_boundary_low_text->GetValue();
	str.ToDouble(&val1);
	str = m_boundary_high_text->GetValue();
	str.ToDouble(&val2);
	int low = std::round(val1 * gmf);
	int high = std::round(val2 * gmf);
	int low_save = low;
	int high_save = high;
	m_boundary_sldr->ChangeValues(low, high);
	if (low != low_save && t != m_boundary_low_text)
	{
		val1 = low / gmf;
		m_boundary_low_text->ChangeValue(wxString::Format("%.4f", val1));
	}
	if (high != high_save && t != m_boundary_high_text)
	{
		val2 = high / gmf;
		m_boundary_high_text->ChangeValue(wxString::Format("%.4f", val2));
	}

	//set boundary value
	if (m_sync_group)
		SyncBoundary(val1, val2);
	else
		SetBoundary(val1, val2, false);
}

void VolumePropPanel::OnBoundaryLink(wxCommandEvent& event)
{
	bool val = m_boundary_sldr->GetLink();
	val = !val;
	m_boundary_sldr->SetLink(val);
	m_boundary_link_tb->ToggleTool(0, val);
	wxBitmapBundle bitmap;
	if (val)
		bitmap = wxGetBitmap(link);
	else
		bitmap = wxGetBitmap(unlink);
	m_boundary_link_tb->SetToolNormalBitmap(0, bitmap);
}

void VolumePropPanel::OnBoundaryChk(wxCommandEvent& event)
{
	bool val = m_boundary_chk->GetValue();
	EnableBoundary(val);
}

void VolumePropPanel::OnThreshMF(wxCommandEvent& event)
{
	if (!m_vd)
		return;

	switch (glbin_settings.m_mulfunc)
	{
	case 0:
		SyncThresh(m_vd->GetLeftThresh(), m_vd->GetRightThresh());
		break;
	case 1:
		m_frame->SetFocusVRenderViews(m_thresh_sldr);
		break;
	case 2:
		SetThresh(glbin_vol_def.m_lo_thresh, glbin_vol_def.m_hi_thresh, true);
		break;
	case 3:
		SetThresh(m_vd->GetMlLeftThresh(), m_vd->GetMlRightThresh(), true);
		break;
	case 4:
		m_thresh_sldr->Undo();
		break;
	case 5:
		EnableThresh(!m_vd->GetThreshEnable());
		break;
	}
}

void VolumePropPanel::OnThreshChange(wxScrollEvent& event)
{
	int ival1 = m_thresh_sldr->GetLowValue();
	int ival2 = m_thresh_sldr->GetHighValue();
	double val1 = double(ival1) / m_max_val;
	double val2 = double(ival2) / m_max_val;
	m_left_thresh_text->ChangeValue(wxString::Format("%d", ival1));
	m_right_thresh_text->ChangeValue(wxString::Format("%d", ival2));

	//set left threshold value
	if (m_sync_group)
		SyncThresh(val1, val2);
	else
		SetThresh(val1, val2, false);
}

void VolumePropPanel::OnThreshText(wxCommandEvent& event)
{
	wxObject* t = event.GetEventObject();
	long ival1 = 0, ival2 = 0;
	wxString str = m_left_thresh_text->GetValue();
	str.ToLong(&ival1);
	str = m_right_thresh_text->GetValue();
	str.ToLong(&ival2);
	int low = ival1;
	int hi = ival2;
	if (double(hi) > m_max_val)
		UpdateMaxVal(hi);
	m_thresh_sldr->ChangeValues(low, hi);
	if (low != ival1 && t != m_left_thresh_text)
		m_left_thresh_text->ChangeValue(std::to_string(low));
	if (hi != ival2 && t != m_right_thresh_text)
		m_right_thresh_text->ChangeValue(std::to_string(hi));
	double val1 = double(low) / m_max_val;
	double val2 = double(hi) / m_max_val;

	//set left threshold value
	if (m_sync_group)
		SyncThresh(val1, val2);
	else
		SetThresh(val1, val2, false);
}

void VolumePropPanel::OnThreshLink(wxCommandEvent& event)
{
	bool val = m_thresh_sldr->GetLink();
	val = !val;
	m_thresh_sldr->SetLink(val);
	m_thresh_link_tb->ToggleTool(0, val);
	wxBitmapBundle bitmap;
	if (val)
		bitmap = wxGetBitmap(link);
	else
		bitmap = wxGetBitmap(unlink);
	m_thresh_link_tb->SetToolNormalBitmap(0, bitmap);
}

void VolumePropPanel::OnThreshChk(wxCommandEvent& event)
{
	bool val = m_thresh_chk->GetValue();
	EnableThresh(val);
}

//shadow
void VolumePropPanel::OnShadowMF(wxCommandEvent& event)
{
	if (!m_vd)
		return;

	switch (glbin_settings.m_mulfunc)
	{
	case 0:
		SyncShadowInt(m_vd->GetShadowIntensity());
		break;
	case 1:
		m_frame->SetFocusVRenderViews(m_shadow_sldr);
		break;
	case 2:
		SetShadowInt(glbin_vol_def.m_shadow_intensity, true);
		break;
	case 3:
		SetShadowInt(m_vd->GetMlShadowIntensity(), true);
		break;
	case 4:
		m_shadow_sldr->Undo();
		break;
	case 5:
		EnableShadow(!m_vd->GetShadowEnable());
		break;
	}
}

void VolumePropPanel::OnShadowChk(wxCommandEvent& event)
{
	bool val = m_shadow_chk->GetValue();
	EnableShadow(val);
}

void VolumePropPanel::OnShadowChange(wxScrollEvent& event)
{
	double val = m_shadow_sldr->GetValue() / 100.0;
	wxString str = wxString::Format("%.2f", val);
	if (str != m_shadow_text->GetValue())
		m_shadow_text->ChangeValue(str);

	//set shadow darkness
	if (m_sync_group)
		SyncShadowInt(val);
	else
		SetShadowInt(val, false);
}

void VolumePropPanel::OnShadowText(wxCommandEvent& event)
{
	wxString str = m_shadow_text->GetValue();
	double val = 0.0;
	str.ToDouble(&val);
	m_shadow_sldr->ChangeValue(std::round(val*100.0));

	//set shadow darkness
	if (m_sync_group)
		SyncShadowInt(val);
	else
		SetShadowInt(val, false);
}

//shadow direction
void VolumePropPanel::OnShadowDirCheck(wxCommandEvent& event)
{
	bool bval = m_shadow_dir_chk->GetValue();
	EnableShadowDir(bval);
}

void VolumePropPanel::OnShadowDirChange(wxScrollEvent& event)
{
	double deg = m_shadow_dir_sldr->GetValue();
	wxString str = wxString::Format("%.0f", deg);
	if (str != m_shadow_dir_text->GetValue())
		m_shadow_dir_text->SetValue(str);
	SetShadowDir(deg, false);
}

void VolumePropPanel::OnShadowDirEdit(wxCommandEvent& event)
{
	wxString str = m_shadow_dir_text->GetValue();
	double deg;
	str.ToDouble(&deg);
	m_shadow_dir_sldr->ChangeValue(std::round(deg));
	SetShadowDir(deg, false);
}

void VolumePropPanel::OnSampleMF(wxCommandEvent& event)
{
	if (!m_vd)
		return;

	switch (glbin_settings.m_mulfunc)
	{
	case 0:
		SyncSampleRate(m_vd->GetSampleRate());
		break;
	case 1:
		m_frame->SetFocusVRenderViews(m_sample_sldr);
		break;
	case 2:
		SetSampleRate(glbin_vol_def.m_sample_rate, true);
		break;
	case 3:
		SetSampleRate(m_vd->GetMlSampleRate(), true);
		break;
	case 4:
		m_sample_sldr->Undo();
		break;
	case 5:
		EnableSample(!m_vd->GetSampleRateEnable());
		break;
	}
}

void VolumePropPanel::OnSampleChange(wxScrollEvent & event)
{
	double val = m_sample_sldr->GetValue() / 10.0;
	wxString str = wxString::Format("%.1f", val);
	if (str != m_sample_text->GetValue())
		m_sample_text->ChangeValue(str);

	//set sample rate value
	if (m_sync_group)
		SyncSampleRate(val);
	else
		SetSampleRate(val, false);
}

void VolumePropPanel::OnSampleText(wxCommandEvent& event)
{
	wxString str = m_sample_text->GetValue();
	double val = 0.0;
	str.ToDouble(&val);
	m_sample_sldr->ChangeValue(std::round(val * 10));

	//set sample rate value
	if (m_sync_group)
		SyncSampleRate(val);
	else
		SetSampleRate(val, false);
}

void VolumePropPanel::OnSampleChk(wxCommandEvent& event)
{
	bool val = m_sample_chk->GetValue();
	EnableSample(val);
}

//colormap controls
void VolumePropPanel::OnColormapMF(wxCommandEvent& event)
{
	if (!m_vd)
		return;

	switch (glbin_settings.m_mulfunc)
	{
	case 0:
		{
			double val1, val2;
			m_vd->GetColormapValues(val1, val2);
			SyncColormapVal(val1, val2);
		}
		break;
	case 1:
		m_frame->SetFocusVRenderViews(m_colormap_sldr);
		break;
	case 2:
		SetColormapVal(glbin_vol_def.m_colormap_low_value, glbin_vol_def.m_colormap_hi_value, true);
		break;
	case 3:
		SetColormapVal(m_vd->GetMlColormapLow(), m_vd->GetMlColormapHigh(), true);
		break;
	case 4:
		m_colormap_sldr->Undo();
		break;
	case 5:
		EnableColormap(m_vd->GetColorMode() == flvr::ColorMode::SingleColor);
		break;
	}
}

void VolumePropPanel::OnColormapChk(wxCommandEvent& event)
{
	bool val = m_colormap_chk->GetValue();
	EnableColormap(val);
}

void VolumePropPanel::OnColormapChange(wxScrollEvent& event)
{
	int ival1 = m_colormap_sldr->GetLowValue();
	int ival2 = m_colormap_sldr->GetHighValue();
	double val1 = double(ival1) / m_max_val;
	double val2 = double(ival2) / m_max_val;

	//set left threshold value
	if (m_sync_group)
		SyncColormapVal(val1, val2);
	else
		SetColormapVal(val1, val2, true);
}

void VolumePropPanel::OnColormapText(wxCommandEvent& event)
{
	wxObject* t = event.GetEventObject();
	wxString str = m_colormap_low_text->GetValue();
	double dval1 = 0, dval2 = 0;
	str.ToDouble(&dval1);
	str = m_colormap_hi_text->GetValue();
	str.ToDouble(&dval2);

	if (m_vd)
	{
		double minv, maxv;
		m_vd->GetColormapRange(minv, maxv);
		dval1 = (dval1 - minv) / (maxv - minv);
		dval2 = (dval2 - minv) / (maxv - minv);
	}

	//set left threshold value
	if (m_sync_group)
		SyncColormapVal(dval1, dval2);
	else
		SetColormapVal(dval1, dval2, true);
}

void VolumePropPanel::OnColormapLink(wxCommandEvent& event)
{
	bool val = m_colormap_sldr->GetLink();
	val = !val;
	m_colormap_sldr->SetLink(val);
	m_colormap_link_tb->ToggleTool(0, val);
	wxBitmapBundle bitmap;
	if (val)
		bitmap = wxGetBitmap(link);
	else
		bitmap = wxGetBitmap(unlink);
	m_colormap_link_tb->SetToolNormalBitmap(0, bitmap);
}

void VolumePropPanel::OnColormapInvBtn(wxCommandEvent& event)
{
	bool val = m_colormap_inv_btn->GetToolState(0);
	if (val)
		m_colormap_inv_btn->SetToolNormalBitmap(0,
			wxGetBitmap(invert));
	else
		m_colormap_inv_btn->SetToolNormalBitmap(0,
			wxGetBitmap(invert_off));
	assert(m_view);
	assert(m_group);
	assert(m_vd);
	bool mip_enable = m_vd->GetRenderMode() == flvr::RenderMode::Mip;
	bool sync_view_depth_mip = mip_enable && m_view->GetChannelMixMode() == ChannelMixMode::Depth;
	bool sync_group_depth_mip = mip_enable && m_vd->GetChannelMixMode() == ChannelMixMode::Depth;

	if (sync_view_depth_mip)
		m_view->SetColormapInv(val ? -1.0 : 1.0);
	else if (m_sync_group || sync_group_depth_mip)
		m_group->SetColormapInv(val ? -1.0 : 1.0);
	else if (m_vd)
		m_vd->SetColormapInv(val ? -1.0 : 1.0);

	EnableColormap(true);

	FluoRefresh(1, { gstColormap }, { glbin_current.GetViewId() });
}

void VolumePropPanel::OnColormapCombo(wxCommandEvent& event)
{
	int colormap = m_colormap_combo->GetCurrentSelection();
	assert(m_view);
	assert(m_group);
	assert(m_vd);
	bool mip_enable = m_vd->GetRenderMode() == flvr::RenderMode::Mip;
	bool sync_view_depth_mip = mip_enable && m_view->GetChannelMixMode() == ChannelMixMode::Depth;
	bool sync_group_depth_mip = mip_enable && m_vd->GetChannelMixMode() == ChannelMixMode::Depth;

	if (sync_view_depth_mip)
		m_view->SetColormap(colormap);
	else if (m_sync_group || sync_group_depth_mip)
		m_group->SetColormap(colormap);
	else if (m_vd)
		m_vd->SetColormap(colormap);

	EnableColormap(true);
	if (colormap >= 5)
	{
		m_options_toolbar->ToggleTool(ID_TranspChk, true);
		EnableTransparent(true);
	}

	FluoRefresh(1, { gstColormap }, { glbin_current.GetViewId() });
}

void VolumePropPanel::OnColormapCombo2(wxCommandEvent& event)
{
	int ival = m_colormap_combo2->GetCurrentSelection() + 1;
	flvr::ColormapProj colormap_proj =
		static_cast<flvr::ColormapProj>(ival);

	if (m_sync_group && m_group)
		m_group->SetColormapProj(colormap_proj);
	else if (m_vd)
		m_vd->SetColormapProj(colormap_proj);

	EnableColormap(true);

	FluoRefresh(1, { gstColormap }, { glbin_current.GetViewId() });
}

//6
void VolumePropPanel::OnMainColorMode(wxCommandEvent& event)
{
	if (!m_vd)
		return;

	auto mode = m_vd->GetMainMaskMode();
	int ival = static_cast<int>(mode);
	ival++;
	ival = ival > static_cast<int>(flvr::MaskMode::Component) ?
		static_cast<int>(flvr::MaskMode::None) : ival;
	mode = static_cast<flvr::MaskMode>(ival);
	if (m_sync_group && m_group)
		m_group->SetMainMaskMode(mode);
	else
		m_vd->SetMainMaskMode(mode);

	FluoRefresh(0, { gstMainMode, gstMaskMode }, { glbin_current.GetViewId() });
}

void VolumePropPanel::OnMainColorChange(wxColor c)
{
	fluo::Color color(c.Red()/255.0, c.Green()/255.0, c.Blue()/255.0);
	if (m_vd)
	{
		m_vd->SetColor(color);

		if (!m_vd->GetMaskColorSet())
		{
			color = m_vd->GetMaskColor();
			wxColor wxc((unsigned char)(color.r()*255),
				(unsigned char)(color.g()*255),
				(unsigned char)(color.b()*255));
			m_alt_color_text->ChangeValue(wxString::Format("%d , %d , %d",
				wxc.Red(), wxc.Green(), wxc.Blue()));
			m_alt_color_btn->SetValue(wxc);
		}

		FluoRefresh(0, { gstColor, gstSecColor, gstTreeColors, gstClipPlaneRangeColor, gstUpdateSync, gstColormap, gstUpdateHistogram }, { glbin_current.GetViewId() });
	}
}

void VolumePropPanel::OnAltColorMode(wxCommandEvent& event)
{
	if (!m_vd)
		return;

	auto mode = m_vd->GetMaskMode();
	int ival = static_cast<int>(mode);
	ival++;
	ival = ival > static_cast<int>(flvr::MaskMode::Component) ?
		static_cast<int>(flvr::MaskMode::None) : ival;
	mode = static_cast<flvr::MaskMode>(ival);
	if (m_sync_group && m_group)
		m_group->SetMaskMode(mode);
	else
		m_vd->SetMaskMode(mode);

	FluoRefresh(0, { gstMainMode, gstMaskMode }, { glbin_current.GetViewId() });
}

void VolumePropPanel::OnAltColorChange(wxColor c)
{
	fluo::Color color(c.Red()/255.0, c.Green()/255.0, c.Blue()/255.0);
	if (m_vd)
	{
		m_vd->SetMaskColor(color);
		FluoRefresh(0, { gstSecColor, gstColormap }, { glbin_current.GetViewId() });
	}
}

void VolumePropPanel::OnMainColorTextChange(wxCommandEvent& event)
{
	wxString str = m_main_color_text->GetValue();
	wxColor wxc;
	if (GetColorString(str, wxc) == 3)
	{
		wxString new_str = wxString::Format("%d , %d , %d",
			wxc.Red(), wxc.Green(), wxc.Blue());
		if (str != new_str)
			m_main_color_text->ChangeValue(new_str);
		m_main_color_btn->SetValue(wxc);

		OnMainColorChange(wxc);
	}
}

void VolumePropPanel::OnAltColorTextChange(wxCommandEvent& event)
{
	wxString str = m_alt_color_text->GetValue();
	wxColor wxc;
	if (GetColorString(str, wxc) == 3)
	{
		wxString new_str = wxString::Format("%d , %d , %d",
			wxc.Red(), wxc.Green(), wxc.Blue());
		if (str != new_str)
			m_alt_color_text->ChangeValue(new_str);
		m_alt_color_btn->SetValue(wxc);

		OnAltColorChange(wxc);
	}
}

void VolumePropPanel::OnMainColorBtn(wxColourPickerEvent& event)
{
	wxColor wxc = event.GetColour();

	m_main_color_text->ChangeValue(wxString::Format("%d , %d , %d",
		wxc.Red(), wxc.Green(), wxc.Blue()));

	OnMainColorChange(wxc);
}

void VolumePropPanel::OnAltColorBtn(wxColourPickerEvent& event)
{
	wxColor wxc = event.GetColour();

	m_alt_color_text->ChangeValue(wxString::Format("%d , %d , %d",
		wxc.Red(), wxc.Green(), wxc.Blue()));

	OnAltColorChange(wxc);
}

void VolumePropPanel::OnMainColorTextFocus(wxMouseEvent& event)
{
	m_main_color_text->SetSelection(0, -1);
}

void VolumePropPanel::OnAltColorTextFocus(wxMouseEvent& event)
{
	m_alt_color_text->SetSelection(0, -1);
}

void VolumePropPanel::OnOptions(wxCommandEvent& event)
{
	int id = event.GetId();

	switch (id)
	{
	case ID_UseMlChk:
		SetMachineLearning();
		break;
	case ID_TranspChk:
		SetTransparent();
		break;
	case ID_MipChk:
		SetMIP();
		break;
	case ID_InvChk:
		SetInvert();
		break;
	case ID_OutlineChk:
		SetOutline();
		break;
	case ID_InterpolateChk:
		SetInterpolate();
		break;
	case ID_NoiseReductChk:
		SetNoiseReduction();
		break;
	case ID_SyncGroupChk:
		SetSyncGroup();
		break;
	case ID_ChannelMixDepthChk:
		SetChannelMixDepth();
		break;
	case ID_LegendChk:
		SetLegend();
		break;
	case ID_ResetDefault:
		ResetDefault();
		break;
	case ID_SaveDefault:
		SaveDefault();
		break;
	}
}

//ml
void VolumePropPanel::SetMachineLearning()
{
	ApplyMl();
	//settings not managed by ml
	//component display
	//int ival = glbin_vol_def.m_label_mode;
	//m_options_toolbar->ToggleTool(ID_CompChk, ival ? true : false);
	//if (m_sync_group && m_group)
	//	m_group->SetLabelMode(ival);
	//else
	//	m_vd->SetLabelMode(ival);
}

void VolumePropPanel::SetTransparent()
{
	bool bval = m_options_toolbar->GetToolState(ID_TranspChk);
	EnableTransparent(bval);
}

void VolumePropPanel::SetMIP()
{
	int val = m_options_toolbar->GetToolState(ID_MipChk)?1:0;
	EnableMip(val);
}

void VolumePropPanel::SetInvert()
{
	bool inv = m_options_toolbar->GetToolState(ID_InvChk);

	if (m_sync_group && m_group)
		m_group->SetInvert(inv);
	else if (m_vd)
		m_vd->SetInvert(inv);

	FluoRefresh(0, { gstInvert, gstConvVolMeshUpdateTransf }, { glbin_current.GetViewId() });
}

void VolumePropPanel::SetOutline()
{
	bool bval = m_options_toolbar->GetToolState(ID_OutlineChk);

	if (m_sync_group && m_group)
		m_group->SetOutline(bval);
	else if (m_vd)
		m_vd->SetOutline(bval);

	FluoRefresh(0, { gstOutline }, { glbin_current.GetViewId() });
}

//interpolation
void VolumePropPanel::SetInterpolate()
{
	bool inv = m_options_toolbar->GetToolState(ID_InterpolateChk);

	if (m_sync_group && m_group)
		m_group->SetInterpolate(inv);
	else if (m_vd)
		m_vd->SetInterpolate(inv);
	if (m_view)
		m_view->SetIntp(inv);

	FluoRefresh(0, { gstInterpolate }, { glbin_current.GetViewId() });
}

//noise reduction
void VolumePropPanel::SetNoiseReduction()
{
	bool val = m_options_toolbar->GetToolState(ID_NoiseReductChk);

	bool depth_view = m_view && m_view->GetChannelMixMode() == ChannelMixMode::Depth;
	bool depth_group = m_vd->GetChannelMixMode() == ChannelMixMode::Depth;
	if (depth_view)
		m_view->SetNR(val);
	else if (m_sync_group || depth_group)
		m_group->SetNR(val);
	else
		m_vd->SetNR(val);

	FluoRefresh(0, { gstNoiseRedct }, { glbin_current.GetViewId() });
}

//sync within group
void VolumePropPanel::SetSyncGroup()
{
	m_sync_group = m_options_toolbar->GetToolState(ID_SyncGroupChk);
	if (m_group)
		m_group->SetVolumeSyncProp(m_sync_group);

	if (m_sync_group && m_group && m_vd && m_view)
	{
		//gamma
		m_group->SetGammaEnable(m_vd->GetGammaEnable());
		m_group->SetGamma(m_vd->GetGamma());
		//minmax
		m_group->SetMinMaxEnable(m_vd->GetMinMaxEnable());
		m_group->SetLowOffset(m_vd->GetLowOffset());
		m_group->SetHighOffset(m_vd->GetHighOffset());
		//alpha
		m_group->SetAlphaEnable(m_vd->GetAlphaEnable());
		m_group->SetAlpha(m_vd->GetAlpha());
		//shading
		m_group->SetShadingEnable(m_vd->GetShadingEnable());
		m_group->SetShadingStrength(m_vd->GetShadingStrength());
		//high shading
		m_group->SetShadingShine(m_vd->GetShadingShine());
		//boundary
		m_group->SetBoundaryEnable(m_vd->GetBoundaryEnable());
		m_group->SetBoundaryLow(m_vd->GetBoundaryLow());
		m_group->SetBoundaryHigh(m_vd->GetBoundaryHigh());
		//left threshold
		m_group->SetThreshEnable(m_vd->GetThreshEnable());
		m_group->SetLeftThresh(m_vd->GetLeftThresh());
		//right thresh
		m_group->SetRightThresh(m_vd->GetRightThresh());
		//shadow
		m_group->SetShadowEnable(m_vd->GetShadowEnable());
		m_group->SetShadowIntensity(m_vd->GetShadowIntensity());
		//sample rate
		m_group->SetSampleRateEnable(m_vd->GetSampleRateEnable());
		m_group->SetSampleRate(m_vd->GetSampleRate());
		//inversion
		m_group->SetInvert(m_vd->GetInvert());
		//interpolation
		m_group->SetInterpolate(m_vd->GetInterpolate());
		m_view->SetIntp(m_vd->GetInterpolate());
		//MIP
		m_group->SetRenderMode(m_vd->GetRenderMode());
		//transp
		m_group->SetAlphaPower(m_vd->GetAlphaPower());
		//noise reduction
		m_group->SetNR(m_vd->GetNR());
		//colormap mode
		m_group->SetColorMode(m_vd->GetColorMode());
		//colormap values
		m_group->SetColormapValues(m_vd->GetColormapLow(), m_vd->GetColormapHigh());
		//colormap type
		m_group->SetColormap(m_vd->GetColormap());
		//colormap inv
		m_group->SetColormapInv(m_vd->GetColormapInv());
		//colormap proj
		m_group->SetColormapProj(m_vd->GetColormapProj());
	}

	FluoRefresh(1, { gstVolumeProps }, { glbin_current.GetViewId() });
}

//depth mode
void VolumePropPanel::SetChannelMixDepth()
{
	bool val = m_options_toolbar->GetToolState(ID_ChannelMixDepthChk);

	if (val)
	{
		if (m_group)
		{
			m_group->SetChannelMixMode(ChannelMixMode::Depth);
			if (m_vd)
			{
				m_group->SetNR(m_vd->GetNR());
				m_group->SetSampleRate(m_vd->GetSampleRate());
				m_group->SetShadowEnable(m_vd->GetShadowEnable());
				m_group->SetShadowIntensity(m_vd->GetShadowIntensity());
			}
		}
	}
	else
	{
		if (m_group)
			m_group->SetChannelMixMode(ChannelMixMode::CompositeAdd);
	}

	FluoRefresh(0, { gstChannelMixMode }, { glbin_current.GetViewId() });
}

//legend
void VolumePropPanel::SetLegend()
{
	bool leg = m_options_toolbar->GetToolState(ID_LegendChk);
	if (m_vd)
		m_vd->SetLegend(leg);

	FluoRefresh(1, { gstLegend }, { glbin_current.GetViewId() });
}

void VolumePropPanel::SaveDefault()
{
	bool use_ml = glbin.get_vp_table_enable() && m_vd;
	if (use_ml)
		SaveMl();
	else
		glbin_vol_def.Set(m_vd);
}

void VolumePropPanel::ResetDefault()
{
	if (!m_vd)
		return;

	m_thresh_sldr->SetLink(false);
	m_colormap_sldr->SetLink(false);

	if (m_sync_group && m_group)
		glbin_vol_def.Apply(m_group);
	else
		glbin_vol_def.Apply(m_vd);

	FluoRefresh(0, { gstVolumeProps, gstConvVolMeshUpdateTransf }, { glbin_current.GetViewId() });
}

bool VolumePropPanel::SetSpacing()
{
	if (!m_space_x_text || !m_space_y_text || !m_space_z_text)
		return false;

	wxString str, str_new;
	double spcx = 0.0;
	double spcy = 0.0;
	double spcz = 0.0;

	str = m_space_x_text->GetValue();
	str.ToDouble(&spcx);
	if (spcx<=0.0)
		return false;

	str = m_space_y_text->GetValue();
	str.ToDouble(&spcy);
	if (spcy<=0.0)
		return false;

	str = m_space_z_text->GetValue();
	str.ToDouble(&spcz);
	if (spcz<=0.0)
		return false;

	fluo::Vector spc(spcx, spcy, spcz);
	if ((m_sync_group || glbin_settings.m_override_vox) && m_group)
	{
		for (int i = 0; i < m_group->GetVolumeNum(); i++)
		{
			m_group->GetVolumeData(i)->SetSpacing(spc);
			m_group->GetVolumeData(i)->SetBaseSpacing(spc);
		}
	}
	else if (m_vd)
	{
		m_vd->SetSpacing(spc);
		m_vd->SetBaseSpacing(spc);
	}
	else return false;

	return true;
}

//update max value
void VolumePropPanel::UpdateMaxVal(double value)
{
	if (!m_vd) return;
	int bits = m_vd->GetBits();
	if (bits == 8)
		return;
	else if (bits > 8)
	{
		if (value < 255.0)
			value = 255.0;
		if (value > 65535.0)
			value = 65535.0;
	}
	m_max_val = value;
	m_vd->SetMinMaxValue(m_vd->GetMinValue(), m_max_val);
	m_vd->SetScalarScale(65535.0 / m_max_val);

	m_minmax_sldr->SetRange(0, std::round(m_max_val));
	m_thresh_sldr->SetRange(0, std::round(m_max_val));
	m_luminance_sldr->SetRange(0, std::round(m_max_val));
	m_alpha_sldr->SetRange(0, std::round(m_max_val));
	m_colormap_sldr->SetRange(0, std::round(m_max_val));
}

void VolumePropPanel::OnSpaceText(wxCommandEvent& event)
{
	if (SetSpacing())
	{
		InitViews(INIT_BOUNDS|INIT_CENTER);
		FluoRefresh(3, { gstNull });
	}
}

