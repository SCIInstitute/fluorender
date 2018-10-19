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
#include <Fui/VolumePropPanel.h>
#include <Global/Global.h>
#include <wx/valnum.h>
#include <png_resource.h>
#include <img/icons.h>

using namespace FUI;

BEGIN_EVENT_TABLE(VolumePropPanel, wxPanel)
//1
EVT_COMMAND_SCROLL(ID_GammaSldr, VolumePropPanel::OnGammaChange)
EVT_TEXT(ID_GammaText, VolumePropPanel::OnGammaText)
EVT_COMMAND_SCROLL(ID_BoundarySldr, VolumePropPanel::OnBoundaryChange)
EVT_TEXT(ID_BoundaryText, VolumePropPanel::OnBoundaryText)
//2
EVT_COMMAND_SCROLL(ID_SaturationSldr, VolumePropPanel::OnSaturationChange)
EVT_TEXT(ID_SaturationText, VolumePropPanel::OnSaturationText)
EVT_COMMAND_SCROLL(ID_LeftThreshSldr, VolumePropPanel::OnLeftThreshChange)
EVT_TEXT(ID_LeftThreshText, VolumePropPanel::OnLeftThreshText)
EVT_COMMAND_SCROLL(ID_RightThreshSldr, VolumePropPanel::OnRightThreshChange)
EVT_TEXT(ID_RightThreshText, VolumePropPanel::OnRightThreshText)
//3
EVT_COMMAND_SCROLL(ID_LuminanceSldr, VolumePropPanel::OnLuminanceChange)
EVT_TEXT(ID_LuminanceText, VolumePropPanel::OnLuminanceText)
EVT_TOOL(ID_ShadowChk, VolumePropPanel::OnShadowEnable)
EVT_COMMAND_SCROLL(ID_ShadowSldr, VolumePropPanel::OnShadowChange)
EVT_TEXT(ID_ShadowText, VolumePropPanel::OnShadowText)
EVT_COMMAND_SCROLL(ID_HiShadingSldr, VolumePropPanel::OnHiShadingChange)
EVT_TEXT(ID_HiShadingText, VolumePropPanel::OnHiShadingText)
//4
EVT_TOOL(ID_AlphaChk, VolumePropPanel::OnAlphaCheck)
EVT_COMMAND_SCROLL(ID_AlphaSldr, VolumePropPanel::OnAlphaChange)
EVT_TEXT(ID_Alpha_Text, VolumePropPanel::OnAlphaText)
EVT_COMMAND_SCROLL(ID_SampleSldr, VolumePropPanel::OnSampleChange)
EVT_TEXT(ID_SampleText, VolumePropPanel::OnSampleText)
//5
EVT_COMMAND_SCROLL(ID_LowShadingSldr, VolumePropPanel::OnLowShadingChange)
EVT_TEXT(ID_LowShadingText, VolumePropPanel::OnLowShadingText)
EVT_TOOL(ID_ShadingEnableChk, VolumePropPanel::OnShadingEnable)
//colormap
EVT_TOOL(ID_ColormapEnableChk, VolumePropPanel::OnEnableColormap)
EVT_COMMAND_SCROLL(ID_ColormapHighValueSldr, VolumePropPanel::OnColormapHighValueChange)
EVT_TEXT(ID_ColormapHighValueText, VolumePropPanel::OnColormapHighValueText)
EVT_COMMAND_SCROLL(ID_ColormapLowValueSldr, VolumePropPanel::OnColormapLowValueChange)
EVT_TEXT(ID_ColormapLowValueText, VolumePropPanel::OnColormapLowValueText)
EVT_COMBOBOX(ID_ColormapCombo, VolumePropPanel::OnColormapCombo)
EVT_COMBOBOX(ID_ColormapCombo2, VolumePropPanel::OnColormapCombo2)
//6
//color 1
EVT_TEXT(ID_ColorText, VolumePropPanel::OnColorTextChange)
EVT_COLOURPICKER_CHANGED(ID_ColorBtn, VolumePropPanel::OnColorBtn)
EVT_TEXT(ID_Color2Text, VolumePropPanel::OnColor2TextChange)
EVT_COLOURPICKER_CHANGED(ID_Color2Btn, VolumePropPanel::OnColor2Btn)
//spacings
EVT_TEXT(ID_SpaceXText, VolumePropPanel::OnSpaceText)
EVT_TEXT(ID_SpaceYText, VolumePropPanel::OnSpaceText)
EVT_TEXT(ID_SpaceZText, VolumePropPanel::OnSpaceText)
//legend
EVT_TOOL(ID_LegendChk, VolumePropPanel::OnLegendCheck)
//EVT_TOOL
EVT_TOOL(ID_InterpolateChk, VolumePropPanel::OnInterpolateCheck)
//sync within group
EVT_TOOL(ID_SyncGroupChk, VolumePropPanel::OnSyncGroupCheck)
//save default
EVT_TOOL(ID_SaveDefault, VolumePropPanel::OnSaveDefault)
EVT_TOOL(ID_ResetDefault, VolumePropPanel::OnResetDefault)
//inversion
EVT_TOOL(ID_InvChk, VolumePropPanel::OnInvCheck)
//MIP
EVT_TOOL(ID_MipChk, VolumePropPanel::OnMIPCheck)
//noise reduction
EVT_TOOL(ID_NRChk, VolumePropPanel::OnNRCheck)
//depth mode
EVT_TOOL(ID_DepthChk, VolumePropPanel::OnDepthCheck)
END_EVENT_TABLE()

VolumePropPanel::VolumePropPanel(wxWindow* frame,
	wxWindow* parent,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name) :
	wxPanel(parent, id, pos, size, style, name),
	m_frame(frame),
	m_max_val(255.0)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);

	m_agent = 
		FL::Global::instance().getAgentFactory().
		getOrAddVolumePropAgent("VolumePropPanel", *this);

	wxBoxSizer* sizer_all = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizer_left = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* sizer_middle = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* sizer_right = new wxBoxSizer(wxVERTICAL);

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

	//validator: floating point 1
	wxFloatingPointValidator<double> vald_fp1(1);
	//validator: floating point 2
	wxFloatingPointValidator<double> vald_fp2(2);
	//validator: floating point 3
	wxFloatingPointValidator<double> vald_fp3(3);
	//validator: floating point 4
	wxFloatingPointValidator<double> vald_fp4(4);
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;

	SetDoubleBuffered(true);

	//left///////////////////////////////////////////////////
	//gamma
	m_gamma_st = new wxStaticText(this, ID_GammaSync, " : Gamma",
		wxDefaultPosition, wxSize(100, -1));
	m_gamma_st->Connect(ID_GammaSync, wxEVT_LEFT_DCLICK,
		wxMouseEventHandler(VolumePropPanel::OnGammaSync), NULL, this);
	m_gamma_st->Connect(ID_GammaSync, wxEVT_RIGHT_DCLICK,
		wxMouseEventHandler(VolumePropPanel::OnGammaSync), NULL, this);
	m_gamma_sldr = new wxSlider(this, ID_GammaSldr, 100, 10, 400,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_INVERSE);
	m_gamma_text = new wxTextCtrl(this, ID_GammaText, "1.00",
		wxDefaultPosition, wxSize(40, 20), 0, vald_fp2);
	sizer_l1->Add(m_gamma_sldr, 1, wxEXPAND);
	sizer_l1->Add(m_gamma_text, 0, wxALIGN_CENTER);
	sizer_l1->Add(m_gamma_st, 0, wxALIGN_CENTER);
	//saturation point
	m_saturation_st = new wxStaticText(this, ID_SaturationSync, " : Saturation",
		wxDefaultPosition, wxSize(100, -1));
	m_saturation_st->Connect(ID_SaturationSync, wxEVT_LEFT_DCLICK,
		wxMouseEventHandler(VolumePropPanel::OnSaturationSync), NULL, this);
	m_saturation_st->Connect(ID_SaturationSync, wxEVT_RIGHT_DCLICK,
		wxMouseEventHandler(VolumePropPanel::OnSaturationSync), NULL, this);
	m_saturation_sldr = new wxSlider(this, ID_SaturationSldr, 255, 0, 255,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_saturation_text = new wxTextCtrl(this, ID_SaturationText, "50",
		wxDefaultPosition, wxSize(40, 20)/*, 0, vald_int*/);
	sizer_l2->Add(m_saturation_sldr, 1, wxEXPAND);
	sizer_l2->Add(m_saturation_text, 0, wxALIGN_CENTER);
	sizer_l2->Add(m_saturation_st, 0, wxALIGN_CENTER);
	//luminance
	m_luminance_st = new wxStaticText(this, ID_LuminanceSync, " : Luminance",
		wxDefaultPosition, wxSize(100, -1));
	m_luminance_st->Connect(ID_LuminanceSync, wxEVT_LEFT_DCLICK,
		wxMouseEventHandler(VolumePropPanel::OnLuminanceSync), NULL, this);
	m_luminance_st->Connect(ID_LuminanceSync, wxEVT_RIGHT_DCLICK,
		wxMouseEventHandler(VolumePropPanel::OnLuminanceSync), NULL, this);
	m_luminance_sldr = new wxSlider(this, ID_LuminanceSldr, 128, 0, 255,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_luminance_text = new wxTextCtrl(this, ID_LuminanceText, "128",
		wxDefaultPosition, wxSize(40, 20)/*, 0, vald_int*/);
	sizer_l3->Add(m_luminance_sldr, 1, wxEXPAND, 0);
	sizer_l3->Add(m_luminance_text, 0, wxALIGN_CENTER, 0);
	sizer_l3->Add(m_luminance_st, 0, wxALIGN_CENTER, 0);
	//alpha
	m_alpha_tool = new wxToolBar(this, ID_AlphaSync,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	wxBitmap bitmap = wxGetBitmapFromMemory(alpha);
#ifdef _DARWIN
	m_alpha_tool->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_alpha_tool->AddCheckTool(ID_AlphaChk, "Alpha",
		bitmap, wxNullBitmap,
		"Enables Alpha Editing.",
		"Enables Alpha Editing.");
	m_alpha_tool->ToggleTool(ID_AlphaChk, true);
	m_alpha_tool->Realize();
	m_alpha_tool->Connect(ID_AlphaSync, wxEVT_LEFT_DCLICK,
		wxMouseEventHandler(VolumePropPanel::OnAlphaSync), NULL, this);
	m_alpha_tool->Connect(ID_AlphaSync, wxEVT_RIGHT_DCLICK,
		wxMouseEventHandler(VolumePropPanel::OnAlphaSync), NULL, this);
	m_alpha_sldr = new wxSlider(this, ID_AlphaSldr, 127, 0, 255,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_alpha_text = new wxTextCtrl(this, ID_Alpha_Text, "127",
		wxDefaultPosition, wxSize(40, 20), 0, vald_int);
	sizer_l4->Add(m_alpha_sldr, 1, wxEXPAND);
	sizer_l4->Add(m_alpha_text, 0, wxALIGN_CENTER);
	sizer_l4->Add(new wxStaticText(this, 0, " : ",
		wxDefaultPosition, wxSize(13, -1)), 0, wxALIGN_CENTER);
	sizer_l4->Add(m_alpha_tool, 0, wxALIGN_CENTER);
	sizer_l4->Add(30, 10, 0);
	//shading
	m_low_shading_sldr = new wxSlider(this, ID_LowShadingSldr, 0, 0, 200,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_low_shading_text = new wxTextCtrl(this, ID_LowShadingText, "0.00",
		wxDefaultPosition, wxSize(40, 20), 0, vald_fp2);
	m_shade_tool = new wxToolBar(this, ID_ShadingSync,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	bitmap = wxGetBitmapFromMemory(shade);
#ifdef _DARWIN
	m_shade_tool->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_shade_tool->AddCheckTool(ID_ShadingEnableChk, "Shading",
		bitmap, wxNullBitmap,
		"Enables Shading Editing.",
		"Enables Shading Editing.");
	m_shade_tool->ToggleTool(ID_ShadingEnableChk, true);
	m_shade_tool->Realize();
	m_shade_tool->Connect(ID_ShadingSync, wxEVT_LEFT_DCLICK,
		wxMouseEventHandler(VolumePropPanel::OnShadingSync), NULL, this);
	m_shade_tool->Connect(ID_ShadingSync, wxEVT_RIGHT_DCLICK,
		wxMouseEventHandler(VolumePropPanel::OnShadingSync), NULL, this);
	sizer_l5->Add(m_low_shading_sldr, 1, wxEXPAND);
	sizer_l5->Add(m_low_shading_text, 0, wxALIGN_CENTER);
	sizer_l5->Add(new wxStaticText(this, 0, " : ",
		wxDefaultPosition, wxSize(13, -1)), 0, wxALIGN_CENTER);
	sizer_l5->Add(m_shade_tool, 0, wxALIGN_CENTER);
	sizer_l5->Add(30, 10, 0);
	//middle///////////////////////////////////////////////////
	//extract boundary
	m_boundary_st = new wxStaticText(this, ID_BoundarySync, "Extract Boundary : ",
		wxDefaultPosition, wxSize(127, -1), wxALIGN_RIGHT);
	m_boundary_st->Connect(ID_BoundarySync, wxEVT_LEFT_DCLICK,
		wxMouseEventHandler(VolumePropPanel::OnBoundarySync), NULL, this);
	m_boundary_st->Connect(ID_BoundarySync, wxEVT_RIGHT_DCLICK,
		wxMouseEventHandler(VolumePropPanel::OnBoundarySync), NULL, this);
	m_boundary_sldr = new wxSlider(this, ID_BoundarySldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_boundary_text = new wxTextCtrl(this, ID_BoundaryText, "0.0000",
		wxDefaultPosition, wxSize(50, 20), 0, vald_fp4);
	sizer_m1->Add(m_boundary_st, 0, wxALIGN_CENTER);
	sizer_m1->Add(m_boundary_text, 0, wxALIGN_CENTER);
	sizer_m1->Add(m_boundary_sldr, 1, wxEXPAND);
	//thresholds
	m_threh_st = new wxStaticText(this, ID_ThreshSync, "Threshold : ",
		wxDefaultPosition, wxSize(127, -1), wxALIGN_RIGHT);
	m_threh_st->Connect(ID_ThreshSync, wxEVT_LEFT_DCLICK,
		wxMouseEventHandler(VolumePropPanel::OnThreshSync), NULL, this);
	m_threh_st->Connect(ID_ThreshSync, wxEVT_RIGHT_DCLICK,
		wxMouseEventHandler(VolumePropPanel::OnThreshSync), NULL, this);
	m_left_thresh_sldr = new wxSlider(this, ID_LeftThreshSldr, 5, 0, 255,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_left_thresh_text = new wxTextCtrl(this, ID_LeftThreshText, "5",
		wxDefaultPosition, wxSize(50, 20), 0, vald_int);
	m_right_thresh_sldr = new wxSlider(this, ID_RightThreshSldr, 230, 0, 255,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_right_thresh_text = new wxTextCtrl(this, ID_RightThreshText, "230",
		wxDefaultPosition, wxSize(50, 20), 0, vald_int);
	sizer_m2->Add(m_threh_st, 0, wxALIGN_CENTER);
	sizer_m2->Add(m_left_thresh_text, 0, wxALIGN_CENTER);
	sizer_m2->Add(m_left_thresh_sldr, 1, wxEXPAND);
	sizer_m2->Add(m_right_thresh_text, 0, wxALIGN_CENTER);
	sizer_m2->Add(m_right_thresh_sldr, 1, wxEXPAND);
	//light/shadow
	//shadow
	m_hi_shading_st = new wxStaticText(this, ID_HiShadingSync, "Light /",
		wxDefaultPosition, wxSize(50, -1), wxALIGN_RIGHT);
	m_hi_shading_st->Connect(ID_HiShadingSync, wxEVT_LEFT_DCLICK,
		wxMouseEventHandler(VolumePropPanel::OnHiShadingSync), NULL, this);
	m_hi_shading_st->Connect(ID_HiShadingSync, wxEVT_RIGHT_DCLICK,
		wxMouseEventHandler(VolumePropPanel::OnHiShadingSync), NULL, this);
	m_shadow_tool = new wxToolBar(this, ID_ShadowSync,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	bitmap = wxGetBitmapFromMemory(shadow);
#ifdef _DARWIN
	m_shadow_tool->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_shadow_tool->AddCheckTool(ID_ShadowChk, "Shadow",
		bitmap, wxNullBitmap,
		"Enables Shadow Editing.",
		"Enables Shadow Editing.");
	m_shadow_tool->ToggleTool(ID_ShadowChk, false);
	m_shadow_tool->Realize();
	m_shadow_tool->Connect(ID_ShadowSync, wxEVT_LEFT_DCLICK,
		wxMouseEventHandler(VolumePropPanel::OnShadowSync), NULL, this);
	m_shadow_tool->Connect(ID_ShadowSync, wxEVT_RIGHT_DCLICK,
		wxMouseEventHandler(VolumePropPanel::OnShadowSync), NULL, this);
	st = new wxStaticText(this, 0, " : ",
		wxDefaultPosition, wxSize(20, -1), wxALIGN_RIGHT);
	//highlight
	m_hi_shading_sldr = new wxSlider(this, ID_HiShadingSldr, 0, 0, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_hi_shading_text = new wxTextCtrl(this, ID_HiShadingText, "0.00",
		wxDefaultPosition, wxSize(50, 20), 0, vald_fp2);
	m_shadow_sldr = new wxSlider(this, ID_ShadowSldr, 0, 0, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_shadow_text = new wxTextCtrl(this, ID_ShadowText, "0.00",
		wxDefaultPosition, wxSize(50, 20), 0, vald_fp2);
	sizer_m3->Add(m_hi_shading_st, 0, wxALIGN_CENTER);
	sizer_m3->Add(m_shadow_tool, 0, wxALIGN_CENTER);
	sizer_m3->Add(st, 0, wxALIGN_CENTER);
	sizer_m3->Add(m_hi_shading_text, 0, wxALIGN_CENTER);
	sizer_m3->Add(m_hi_shading_sldr, 1, wxEXPAND);
	sizer_m3->Add(m_shadow_text, 0, wxALIGN_CENTER);
	sizer_m3->Add(m_shadow_sldr, 1, wxEXPAND);
	//sample rate
	m_sample_st = new wxStaticText(this, ID_SampleSync, "Sample Rate : ",
		wxDefaultPosition, wxSize(127, -1), wxALIGN_RIGHT);
	m_sample_st->Connect(ID_SampleSync, wxEVT_LEFT_DCLICK,
		wxMouseEventHandler(VolumePropPanel::OnSampleSync), NULL, this);
	m_sample_st->Connect(ID_SampleSync, wxEVT_RIGHT_DCLICK,
		wxMouseEventHandler(VolumePropPanel::OnSampleSync), NULL, this);
	m_sample_sldr = new wxSlider(this, ID_SampleSldr, 10, 1, 50,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_sample_text = new wxTextCtrl(this, ID_SampleText, "1.0",
		wxDefaultPosition, wxSize(50, 20), 0, vald_fp2);
	sizer_m4->Add(m_sample_st, 0, wxALIGN_CENTER);
	sizer_m4->Add(m_sample_text, 0, wxALIGN_CENTER);
	sizer_m4->Add(m_sample_sldr, 1, wxEXPAND);
	//colormap
	m_colormap_tool = new wxToolBar(this, ID_ColormapSync,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	bitmap = wxGetBitmapFromMemory(palette);
#ifdef _DARWIN
	m_colormap_tool->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_colormap_tool->AddCheckTool(ID_ColormapEnableChk, "Color Map",
		bitmap, wxNullBitmap,
		"Enables Color Map Editing.",
		"Enables Color Map Editing.");
	m_colormap_tool->ToggleTool(ID_ColormapEnableChk, false);
	m_colormap_tool->Realize();
	m_colormap_tool->Connect(ID_ColormapSync, wxEVT_LEFT_DCLICK,
		wxMouseEventHandler(VolumePropPanel::OnColormapSync), NULL, this);
	m_colormap_tool->Connect(ID_ColormapSync, wxEVT_RIGHT_DCLICK,
		wxMouseEventHandler(VolumePropPanel::OnColormapSync), NULL, this);
	sizer_m5->Add(50, 50, 0);
	sizer_m5->Add(m_colormap_tool, 0, wxALIGN_CENTER);
	st = new wxStaticText(this, 0, " : ",
		wxDefaultPosition, wxSize(20, -1), wxALIGN_RIGHT);
	sizer_m5->Add(st, 0, wxALIGN_CENTER);
	m_colormap_low_value_text = new wxTextCtrl(this,
		ID_ColormapLowValueText, "0",
		wxDefaultPosition, wxSize(50, 20), 0, vald_int);
	sizer_m5->Add(m_colormap_low_value_text, 0, wxALIGN_CENTER);
	m_colormap_low_value_sldr = new wxSlider(this,
		ID_ColormapLowValueSldr, 0, 0, 255,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	sizer_m5->Add(m_colormap_low_value_sldr, 1, wxEXPAND);
	m_colormap_high_value_text = new wxTextCtrl(this,
		ID_ColormapHighValueText, "255",
		wxDefaultPosition + wxPoint(10, 0), wxSize(50, 20), 0, vald_int);
	sizer_m5->Add(m_colormap_high_value_text, 0, wxALIGN_CENTER);
	m_colormap_high_value_sldr = new wxSlider(this,
		ID_ColormapHighValueSldr, 255, 0, 255,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	sizer_m5->Add(m_colormap_high_value_sldr, 1, wxEXPAND);

	//right ///////////////////////////////////////////////////
	m_options_toolbar = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	//MIP
	bitmap = wxGetBitmapFromMemory(mip);
#ifdef _DARWIN
	m_options_toolbar->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_options_toolbar->AddCheckTool(ID_MipChk, "MIP",
		bitmap, wxNullBitmap,
		"Enable Maximum Intensity Projection.",
		"Enable Maximum Intensity Projection.");
	m_options_toolbar->ToggleTool(ID_MipChk, false);
	//inversion
	bitmap = wxGetBitmapFromMemory(invert_off);
	m_options_toolbar->AddCheckTool(ID_InvChk, "Inversion",
		bitmap, wxNullBitmap,
		"Inverts data values when checked.",
		"Inverts data values when checked.");
	m_options_toolbar->ToggleTool(ID_InvChk, false);
	//interpolation
	bitmap = wxGetBitmapFromMemory(interpolate);
	m_options_toolbar->AddCheckTool(ID_InterpolateChk, "Interpolate",
		bitmap, wxNullBitmap,
		"Interpolates between data when checked.",
		"Interpolates between data when checked.");
	m_options_toolbar->ToggleTool(ID_InterpolateChk, true);
	//noise reduction
	bitmap = wxGetBitmapFromMemory(smooth_off);
	m_options_toolbar->AddCheckTool(ID_NRChk, "Smoothing",
		bitmap, wxNullBitmap,
		"Enable Data Smoothing.",
		"Enable Data Smoothing.");
	m_options_toolbar->ToggleTool(ID_NRChk, false);
	//sync group
	bitmap = wxGetBitmapFromMemory(sync_chan);
	m_options_toolbar->AddCheckTool(ID_SyncGroupChk, "Group Sync",
		bitmap, wxNullBitmap,
		"Sync this channel with others in its group.",
		"Sync this channel with others in its group.");
	m_options_toolbar->ToggleTool(ID_SyncGroupChk, false);
	//depth mode
	bitmap = wxGetBitmapFromMemory(depth_off);
	m_options_toolbar->AddCheckTool(ID_DepthChk, "Depth Mode",
		bitmap, wxNullBitmap,
		"Enable Depth Mode.",
		"Enable Depth Mode.");
	m_options_toolbar->ToggleTool(ID_DepthChk, false);
	//legend
	bitmap = wxGetBitmapFromMemory(legend);
	m_options_toolbar->AddCheckTool(ID_LegendChk, "Legend",
		bitmap, wxNullBitmap,
		"Enable Legend for this channel.",
		"Enable Legend for this channel.");
	m_options_toolbar->ToggleTool(ID_LegendChk, true);
	//buttons
	bitmap = wxGetBitmapFromMemory(reset);
	m_options_toolbar->AddTool(ID_ResetDefault, "Reset",
		bitmap, "Reset Properties");
	bitmap = wxGetBitmapFromMemory(save_settings);
	m_options_toolbar->AddTool(ID_SaveDefault, "Save",
		bitmap, "Set as default settings");
	sizer_r1->AddStretchSpacer();
	sizer_r1->Add(m_options_toolbar, 0, wxALIGN_CENTER);
	sizer_r1->AddStretchSpacer();
	m_options_toolbar->Realize();
	//spacings
	//x
	m_space_st = new wxStaticText(this, ID_SpaceSync, "Spacing",
		wxDefaultPosition, wxSize(70, -1), wxALIGN_CENTER);
	m_space_st->Connect(ID_SpaceSync, wxEVT_LEFT_DCLICK,
		wxMouseEventHandler(VolumePropPanel::OnSpaceSync), NULL, this);
	m_space_st->Connect(ID_SpaceSync, wxEVT_RIGHT_DCLICK,
		wxMouseEventHandler(VolumePropPanel::OnSpaceSync), NULL, this);
	m_space_x_text = new wxTextCtrl(this, ID_SpaceXText, "1.000",
		wxDefaultPosition, wxSize(45, -1), 0, vald_fp3);
	sizer_r2->Add(m_space_st, 0, wxALIGN_CENTER);
	sizer_r2->AddStretchSpacer();
	st = new wxStaticText(this, 0, "X:");
	sizer_r2->Add(st, 0, wxALIGN_CENTER);
	sizer_r2->Add(m_space_x_text, 0, wxALIGN_CENTER);
	//y
	st = new wxStaticText(this, 0, "Y:");
	m_space_y_text = new wxTextCtrl(this, ID_SpaceYText, "1.000",
		wxDefaultPosition, wxSize(45, -1), 0, vald_fp3);
	sizer_r2->Add(3, 5, 0);
	sizer_r2->Add(st, 0, wxALIGN_CENTER);
	sizer_r2->Add(m_space_y_text, 0, wxALIGN_CENTER);
	//z
	st = new wxStaticText(this, 0, "Z:");
	m_space_z_text = new wxTextCtrl(this, ID_SpaceZText, "1.000",
		wxDefaultPosition, wxSize(45, -1), 0, vald_fp3);
	sizer_r2->Add(3, 5, 0);
	sizer_r2->Add(st, 0, wxALIGN_CENTER);
	sizer_r2->Add(m_space_z_text, 0, wxALIGN_CENTER);
	//color 1
	st = new wxStaticText(this, 0, "Pri Color:",
		wxDefaultPosition, wxSize(70, -1), wxALIGN_CENTER);
	m_color_text = new wxTextCtrl(this, ID_ColorText, "255 , 255 , 255",
		wxDefaultPosition, wxSize(85, 20));
	m_color_text->Connect(ID_ColorText, wxEVT_LEFT_DCLICK,
		wxCommandEventHandler(VolumePropPanel::OnColorTextFocus),
		NULL, this);
	m_color_btn = new wxColourPickerCtrl(this, ID_ColorBtn, *wxRED,
		wxDefaultPosition, wxSize(50, 25));
	sizer_r3->Add(st, 0, wxALIGN_CENTER, 0);
	sizer_r3->Add(5, 5, 0);
	sizer_r3->Add(m_color_text, 0, wxALIGN_CENTER, 0);
	sizer_r3->Add(m_color_btn, 1, wxALIGN_CENTER, 0);
	//color 2
	st = new wxStaticText(this, 0, "Sec Color:",
		wxDefaultPosition, wxSize(70, -1), wxALIGN_CENTER);
	m_color2_text = new wxTextCtrl(this, ID_Color2Text, "255 , 255 , 255",
		wxDefaultPosition, wxSize(85, 20));
	m_color2_text->Connect(ID_Color2Text, wxEVT_LEFT_DCLICK,
		wxCommandEventHandler(VolumePropPanel::OnColor2TextFocus),
		NULL, this);
	m_color2_btn = new wxColourPickerCtrl(this, ID_Color2Btn, *wxRED,
		wxDefaultPosition, wxSize(50, 25));
	sizer_r4->Add(st, 0, wxALIGN_CENTER, 0);
	sizer_r4->Add(5, 5, 0);
	sizer_r4->Add(m_color2_text, 0, wxALIGN_CENTER, 0);
	sizer_r4->Add(m_color2_btn, 1, wxALIGN_CENTER, 0);
	// colormap chooser
	st = new wxStaticText(this, 0, "Color Map:",
		wxDefaultPosition, wxSize(70, -1), wxALIGN_CENTER);
	m_colormap_combo = new wxComboBox(this, ID_ColormapCombo, "",
		wxDefaultPosition, wxSize(85, 25), 0, NULL, wxCB_READONLY);
	std::vector<std::string>colormap_list;
	colormap_list.push_back("Rainbow");
	colormap_list.push_back("Reverse Rainbow");
	colormap_list.push_back("Hot");
	colormap_list.push_back("Cool");
	colormap_list.push_back("Diverging");
	colormap_list.push_back("Monochrome");
	colormap_list.push_back("Reverse Mono");
	for (size_t i = 0; i<colormap_list.size(); ++i)
		m_colormap_combo->Append(colormap_list[i]);
	m_colormap_combo2 = new wxComboBox(this, ID_ColormapCombo2, "",
		wxDefaultPosition, wxSize(85, 25), 0, NULL, wxCB_READONLY);
	std::vector<std::string>colormap_list2;
	colormap_list2.push_back("Intensity");
	colormap_list2.push_back("Z Value");
	colormap_list2.push_back("Y Value");
	colormap_list2.push_back("X Value");
	colormap_list2.push_back("Gradient");
	colormap_list2.push_back("Differential");
	for (size_t i = 0; i<colormap_list2.size(); ++i)
		m_colormap_combo2->Append(colormap_list2[i]);
	sizer_r5->Add(st, 0, wxALIGN_CENTER, 0);
	sizer_r5->Add(5, 5, 0);
	sizer_r5->Add(m_colormap_combo, 0, wxALIGN_CENTER, 0);
	sizer_r5->Add(m_colormap_combo2, 0, wxALIGN_CENTER, 0);

	//ADD COLUMNS//////////////////////////////////////
	//left
	sizer_left->Add(sizer_l1, 0, wxEXPAND);
	sizer_left->Add(sizer_l2, 0, wxEXPAND);
	sizer_left->Add(sizer_l3, 0, wxEXPAND);
	sizer_left->Add(sizer_l4, 0, wxEXPAND);
	sizer_left->Add(sizer_l5, 0, wxEXPAND);
	//middle
	sizer_middle->Add(sizer_m1, 0, wxEXPAND);
	sizer_middle->Add(sizer_m2, 0, wxEXPAND);
	sizer_middle->Add(sizer_m3, 0, wxEXPAND);
	sizer_middle->Add(sizer_m4, 0, wxEXPAND);
	sizer_middle->Add(sizer_m5, 0, wxEXPAND);
	//right
	sizer_right->Add(sizer_r1, 0, wxEXPAND);
	sizer_right->Add(sizer_r2, 0, wxEXPAND);
	sizer_right->Add(sizer_r3, 0, wxEXPAND);
	sizer_right->Add(sizer_r4, 0, wxEXPAND);
	sizer_right->Add(sizer_r5, 0, wxEXPAND);
	//ADD ALL TOGETHER
	sizer_all->Add(sizer_left, 1, wxEXPAND);
	sizer_all->Add(sizer_middle, 1, wxEXPAND);
	sizer_all->Add(sizer_right, 0, wxSHRINK);
	SetSizer(sizer_all);
	Layout();
}

VolumePropPanel::~VolumePropPanel()
{
}

void VolumePropPanel::AssociateVolumeData(FL::VolumeData* vd)
{
	m_agent->setObject(vd);
}

//1
void VolumePropPanel::OnGammaSync(wxMouseEvent& event)
{
	m_agent->propParentValue("gamma 3d");
}

void VolumePropPanel::OnGammaChange(wxScrollEvent & event)
{
	double val = (double)event.GetPosition() / 100.0;
	wxString str = wxString::Format("%.2f", val);
	m_gamma_text->SetValue(str);
}

void VolumePropPanel::OnGammaText(wxCommandEvent& event)
{
	wxString str = m_gamma_text->GetValue();
	double val = 0.0;
	str.ToDouble(&val);
	int ival = int(val*100.0 + 0.5);
	m_gamma_sldr->SetValue(ival);

	m_agent->setValue("gamma 3d", val);
}

void VolumePropPanel::OnBoundarySync(wxMouseEvent& event)
{
	m_agent->propParentValue("extract boundary");
}

void VolumePropPanel::OnBoundaryChange(wxScrollEvent & event)
{
	double val = (double)event.GetPosition() / 2000.0;
	wxString str = wxString::Format("%.4f", val);
	m_boundary_text->SetValue(str);
}

void VolumePropPanel::OnBoundaryText(wxCommandEvent& event)
{
	wxString str = m_boundary_text->GetValue();
	double val = 0.0;
	str.ToDouble(&val);
	int ival = int(val*2000.0 + 0.5);
	m_boundary_sldr->SetValue(ival);

	m_agent->setValue("extract boundary", val);
}

//2
void VolumePropPanel::OnSaturationSync(wxMouseEvent& event)
{
	m_agent->propParentValue("saturation");
}

void VolumePropPanel::OnSaturationChange(wxScrollEvent & event)
{
	int ival = event.GetPosition();
	wxString str = wxString::Format("%d", ival);
	m_saturation_text->SetValue(str);
}

void VolumePropPanel::OnSaturationText(wxCommandEvent& event)
{
	wxString str = m_saturation_text->GetValue();
	long ival = 0;
	str.ToLong(&ival);
	if (double(ival) > m_max_val)
	{
		UpdateMaxVal(ival);
		str = wxString::Format("%d", ival);
		m_saturation_text->ChangeValue(str);
	}
	m_saturation_sldr->SetValue(ival);
	double val = double(ival) / m_max_val;

	m_agent->setValue("saturation", val);
}

void VolumePropPanel::OnThreshSync(wxMouseEvent& event)
{
	m_agent->propParentValue("low threshold");
	m_agent->propParentValue("high threshold");
}

void VolumePropPanel::OnLeftThreshChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	wxString str = wxString::Format("%d", ival);
	m_left_thresh_text->SetValue(str);
}

void VolumePropPanel::OnLeftThreshText(wxCommandEvent &event)
{
	wxString str = m_left_thresh_text->GetValue();
	long ival = 0;
	str.ToLong(&ival);
	if (double(ival) > m_max_val)
	{
		UpdateMaxVal(ival);
		str = wxString::Format("%d", ival);
		m_left_thresh_text->ChangeValue(str);
	}
	double val = double(ival) / m_max_val;
	double right_val = (double)m_right_thresh_sldr->GetValue() / m_max_val;

	if (val > right_val)
	{
		val = right_val;
		ival = int(val*m_max_val + 0.5);
		wxString str2 = wxString::Format("%d", ival);
		m_left_thresh_text->ChangeValue(str2);
	}
	m_left_thresh_sldr->SetValue(ival);

	m_agent->setValue("low threshold", val);
}

void VolumePropPanel::OnRightThreshChange(wxScrollEvent & event)
{
	int ival = event.GetPosition();
	int ival2 = m_left_thresh_sldr->GetValue();

	if (ival < ival2)
	{
		ival = ival2;
		m_right_thresh_sldr->SetValue(ival);
	}
	wxString str = wxString::Format("%d", ival);
	m_right_thresh_text->SetValue(str);
}

void VolumePropPanel::OnRightThreshText(wxCommandEvent &event)
{
	wxString str = m_right_thresh_text->GetValue();
	long ival = 0;
	str.ToLong(&ival);
	if (double(ival) > m_max_val)
	{
		UpdateMaxVal(ival);
		str = wxString::Format("%d", ival);
		m_right_thresh_text->ChangeValue(str);
	}
	double val = double(ival) / m_max_val;
	double left_val = (double)m_left_thresh_sldr->GetValue() / m_max_val;

	if (val >= left_val)
	{
		m_right_thresh_sldr->SetValue(ival);

		m_agent->setValue("high threshold", val);
	}
}

//3
void VolumePropPanel::OnLuminanceSync(wxMouseEvent& event)
{
	m_agent->propParentValue("luminance");
}

void VolumePropPanel::OnLuminanceChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	wxString str = wxString::Format("%d", ival);
	m_luminance_text->SetValue(str);
}

void VolumePropPanel::OnLuminanceText(wxCommandEvent &event)
{
	wxString str = m_luminance_text->GetValue();
	long ival = 0;
	str.ToLong(&ival);
	if (double(ival) > m_max_val)
	{
		UpdateMaxVal(ival);
		str = wxString::Format("%d", ival);
		m_luminance_text->ChangeValue(str);
	}
	double val = double(ival) / m_max_val;
	m_luminance_sldr->SetValue(ival);

	m_agent->setValue("luminance", val);
}

//hi shading
void VolumePropPanel::OnHiShadingSync(wxMouseEvent& event)
{
	m_agent->propParentValue("high shading");
}

void VolumePropPanel::OnHiShadingChange(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 10.0;
	wxString str = wxString::Format("%.2f", val);
	m_hi_shading_text->SetValue(str);
}

void VolumePropPanel::OnHiShadingText(wxCommandEvent &event)
{
	wxString str = m_hi_shading_text->GetValue();
	double val = 0.0;
	str.ToDouble(&val);
	m_hi_shading_sldr->SetValue(int(val*10.0 + 0.5));

	m_agent->setValue("high shading", val);
}

//shadow
void VolumePropPanel::OnShadowSync(wxMouseEvent& event)
{
	m_agent->propParentValue("shadow enable");
	m_agent->propParentValue("shadow int");
}

void VolumePropPanel::OnShadowEnable(wxCommandEvent &event)
{
	bool shadow = m_shadow_tool->GetToolState(ID_ShadowChk);
	m_agent->setValue("shadow enable", shadow);

	if (shadow)
		EnableShadow();
	else
		DisableShadow();
}

void VolumePropPanel::OnShadowChange(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 100.0;
	wxString str = wxString::Format("%.2f", val);
	m_shadow_text->SetValue(str);
}

void VolumePropPanel::OnShadowText(wxCommandEvent &event)
{
	wxString str = m_shadow_text->GetValue();
	double val = 0.0;
	str.ToDouble(&val);
	m_shadow_sldr->SetValue(int(val*100.0 + 0.5));

	m_agent->setValue("shadow int", val);
}

//4
void VolumePropPanel::OnAlphaSync(wxMouseEvent& event)
{
	m_agent->propParentValue("alpha enable");
	m_agent->propParentValue("alpha");
}

void VolumePropPanel::OnAlphaCheck(wxCommandEvent &event)
{
	bool alpha = m_alpha_tool->GetToolState(ID_AlphaChk);
	m_agent->setValue("alpha enable", alpha);

	if (alpha)
		EnableAlpha();
	else
		DisableAlpha();
}

void VolumePropPanel::OnAlphaChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	wxString str = wxString::Format("%d", ival);
	m_alpha_text->SetValue(str);
}

void VolumePropPanel::OnAlphaText(wxCommandEvent& event)
{
	wxString str = m_alpha_text->GetValue();
	long ival = 0;
	str.ToLong(&ival);
	if (double(ival) > m_max_val)
	{
		UpdateMaxVal(ival);
		str = wxString::Format("%d", ival);
		m_alpha_text->ChangeValue(str);
	}
	double val = double(ival) / m_max_val;
	m_alpha_sldr->SetValue(ival);

	m_agent->setValue("alpha", val);
}

void VolumePropPanel::OnSampleSync(wxMouseEvent& event)
{
	m_agent->propParentValue("sample rate");
}

void VolumePropPanel::OnSampleChange(wxScrollEvent & event)
{
	double val = event.GetPosition() / 10.0;
	wxString str = wxString::Format("%.1f", val);
	m_sample_text->SetValue(str);
}

void VolumePropPanel::OnSampleText(wxCommandEvent& event)
{
	wxString str = m_sample_text->GetValue();
	double srate = 0.0;
	str.ToDouble(&srate);
	double val = srate*10.0;
	m_sample_sldr->SetValue(int(val));

	//set sample rate value
	m_agent->setValue("sample rate", srate);
	//if (m_vrv && m_vrv->GetVolMethod() == VOL_METHOD_MULTI)
	//{
	//	//for (int i=0; i<m_vrv->GetAllVolumeNum(); i++)
	//	//{
	//	//	VolumeData* vd = m_vrv->GetAllVolumeData(i);
	//	//	if (vd)
	//	//		vd->SetSampleRate(srate);
	//	//}
	//}
	//else
	//{
	//	//if (m_sync_group && m_group)
	//	//	m_group->SetSampleRate(srate);
	//	//else if (m_group && m_group->GetBlendMode()==2)
	//	//	m_group->SetSampleRate(srate);
	//	//else if (m_vd)
	//	//	m_vd->SetSampleRate(srate);
	//	m_vd->setValue("sample rate", srate);
	//}
}

//5
void VolumePropPanel::OnShadingSync(wxMouseEvent& event)
{
	m_agent->propParentValue("shading enable");
	m_agent->propParentValue("low shading");
}

void VolumePropPanel::OnLowShadingChange(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 100.0;
	wxString str = wxString::Format("%.2f", val);
	m_low_shading_text->SetValue(str);
}

void VolumePropPanel::OnLowShadingText(wxCommandEvent &event)
{
	wxString str = m_low_shading_text->GetValue();
	double val = 0.0;
	str.ToDouble(&val);
	m_low_shading_sldr->SetValue(int(val*100.0 + 0.5));

	m_agent->setValue("low shading", val);
}

void VolumePropPanel::OnShadingEnable(wxCommandEvent &event)
{
	bool shading = m_shade_tool->GetToolState(ID_ShadingEnableChk);
	m_agent->setValue("shading enable", shading);

	if (shading)
		EnableShading();
	else
		DisableShading();
}

//colormap controls
void VolumePropPanel::OnColormapSync(wxMouseEvent& event)
{
	m_agent->propParentValue("colormap enable");
	m_agent->propParentValue("colormap mode");
	m_agent->propParentValue("colormap low");
	m_agent->propParentValue("colormap high");
	m_agent->propParentValue("colormap type");
	m_agent->propParentValue("colormap proj");
}

void VolumePropPanel::OnEnableColormap(wxCommandEvent &event)
{
	bool colormap =
		m_colormap_tool->GetToolState(ID_ColormapEnableChk);

	m_agent->setValue("colormap enable", colormap);
	m_agent->setValue("colormap mode", long(colormap ? 1 : 0));

	if (colormap)
		EnableColormap();
	else
		DisableColormap();
}

void VolumePropPanel::OnColormapHighValueChange(wxScrollEvent &event)
{
	int iVal = m_colormap_high_value_sldr->GetValue();
	int iVal2 = m_colormap_low_value_sldr->GetValue();

	if (iVal < iVal2)
	{
		iVal = iVal2;
		m_colormap_high_value_sldr->SetValue(iVal);
	}
	wxString str = wxString::Format("%d", iVal);
	m_colormap_high_value_text->SetValue(str);
}

void VolumePropPanel::OnColormapHighValueText(wxCommandEvent &event)
{
	wxString str = m_colormap_high_value_text->GetValue();
	long iVal = 0;
	str.ToLong(&iVal);
	if (double(iVal) > m_max_val)
	{
		UpdateMaxVal(iVal);
		str = wxString::Format("%d", iVal);
		m_colormap_high_value_text->ChangeValue(str);
	}
	long iVal2 = m_colormap_low_value_sldr->GetValue();

	if (iVal >= iVal2)
	{
		m_colormap_high_value_sldr->SetValue(iVal);

		double val = double(iVal) / m_max_val;

		m_agent->setValue("colormap high", val);
	}
}

void VolumePropPanel::OnColormapLowValueChange(wxScrollEvent &event)
{
	int iVal = m_colormap_low_value_sldr->GetValue();
	wxString str = wxString::Format("%d", iVal);
	m_colormap_low_value_text->SetValue(str);
}

void VolumePropPanel::OnColormapLowValueText(wxCommandEvent &event)
{
	wxString str = m_colormap_low_value_text->GetValue();
	long iVal = 0;
	str.ToLong(&iVal);
	if (double(iVal) > m_max_val)
	{
		UpdateMaxVal(iVal);
		str = wxString::Format("%d", iVal);
		m_colormap_low_value_text->ChangeValue(str);
	}
	long iVal2 = m_colormap_high_value_sldr->GetValue();

	if (iVal > iVal2)
	{
		iVal = iVal2;
		str = wxString::Format("%d", iVal);
		m_colormap_low_value_text->ChangeValue(str);
	}
	m_colormap_low_value_sldr->SetValue(iVal);

	double val = double(iVal) / m_max_val;

	m_agent->setValue("colormap low", val);
}

void VolumePropPanel::OnColormapCombo(wxCommandEvent &event)
{
	long colormap = m_colormap_combo->GetCurrentSelection();

	m_agent->setValue("colormap type", colormap);
}

void VolumePropPanel::OnColormapCombo2(wxCommandEvent &event)
{
	long colormap_proj = m_colormap_combo2->GetCurrentSelection();

	m_agent->setValue("colormap proj", colormap_proj);
}

//6
void VolumePropPanel::OnColorChange(wxColor c)
{
	FLTYPE::Color color(c.Red() / 255.0, c.Green() / 255.0, c.Blue() / 255.0);
	m_agent->setValue("color", color);
}

void VolumePropPanel::OnColor2Change(wxColor c)
{
	FLTYPE::Color color(c.Red() / 255.0, c.Green() / 255.0, c.Blue() / 255.0);
	m_agent->setValue("sec color", color);
}

int VolumePropPanel::GetColorString(wxString& str, wxColor& wxc)
{
	int filled = 3;
	if (str == "a" || str == "A")
		wxc = wxColor(0, 127, 255);
	else if (str == "b" || str == "B")
		wxc = wxColor(0, 0, 255);
	else if (str == "c" || str == "C")
		wxc = wxColor(0, 255, 255);
	else if (str == "d" || str == "D")
		wxc = wxColor(193, 154, 107);
	else if (str == "e" || str == "E")
		wxc = wxColor(80, 200, 120);
	else if (str == "f" || str == "F")
		wxc = wxColor(226, 88, 34);
	else if (str == "g" || str == "G")
		wxc = wxColor(0, 255, 0);
	else if (str == "h" || str == "H")
		wxc = wxColor(70, 255, 0);
	else if (str == "i" || str == "I")
		wxc = wxColor(75, 0, 130);
	else if (str == "j" || str == "J")
		wxc = wxColor(0, 168, 107);
	else if (str == "k" || str == "K")
		wxc = wxColor(0, 0, 0);
	else if (str == "l" || str == "L")
		wxc = wxColor(181, 126, 220);
	else if (str == "m" || str == "M")
		wxc = wxColor(255, 0, 255);
	else if (str == "n" || str == "N")
		wxc = wxColor(0, 0, 128);
	else if (str == "o" || str == "O")
		wxc = wxColor(0, 119, 190);
	else if (str == "p" || str == "P")
		wxc = wxColor(254, 40, 162);
	else if (str == "q" || str == "Q")
		wxc = wxColor(232, 204, 215);
	else if (str == "r" || str == "R")
		wxc = wxColor(255, 0, 0);
	else if (str == "s" || str == "S")
		wxc = wxColor(236, 213, 64);
	else if (str == "t" || str == "T")
		wxc = wxColor(255, 99, 71);
	else if (str == "u" || str == "U")
		wxc = wxColor(211, 0, 63);
	else if (str == "v" || str == "V")
		wxc = wxColor(143, 0, 255);
	else if (str == "w" || str == "W")
		wxc = wxColor(255, 255, 255);
	else if (str == "x" || str == "X")
		wxc = wxColor(115, 134, 120);
	else if (str == "y" || str == "Y")
		wxc = wxColor(255, 255, 0);
	else if (str == "z" || str == "Z")
		wxc = wxColor(57, 167, 142);
	else
	{
		int index = 0;//1-red; 2-green; 3-blue;
		int state = 0;//0-idle; 1-reading digit; 3-finished
		wxString sColor;
		long r = 255;
		long g = 255;
		long b = 255;
		for (unsigned int i = 0; i<str.length(); i++)
		{
			wxChar c = str[i];
			if (isdigit(c) || c == '.')
			{
				if (state == 0 || state == 3)
				{
					sColor += c;
					index++;
					state = 1;
				}
				else if (state == 1)
				{
					sColor += c;
				}

				if (i == str.length() - 1)  //last one
				{
					switch (index)
					{
					case 1:
						sColor.ToLong(&r);
						filled = 1;
						break;
					case 2:
						sColor.ToLong(&g);
						filled = 2;
						break;
					case 3:
						sColor.ToLong(&b);
						filled = 3;
						break;
					}
				}
			}
			else
			{
				if (state == 1)
				{
					switch (index)
					{
					case 1:
						sColor.ToLong(&r);
						filled = 1;
						break;
					case 2:
						sColor.ToLong(&g);
						filled = 2;
						break;
					case 3:
						sColor.ToLong(&b);
						filled = 3;
						break;
					}
					state = 3;
					sColor = "";
				}
			}
		}
		wxc = wxColor(FLTYPE::Clamp(r, 0, 255),
			FLTYPE::Clamp(g, 0, 255),
			FLTYPE::Clamp(b, 0, 255));
	}
	return filled;
}

void VolumePropPanel::OnColorTextChange(wxCommandEvent& event)
{
	wxString str = m_color_text->GetValue();
	wxColor wxc;
	if (GetColorString(str, wxc) == 3)
	{
		wxString new_str = wxString::Format("%d , %d , %d",
			wxc.Red(), wxc.Green(), wxc.Blue());
		if (str != new_str)
			m_color_text->ChangeValue(new_str);
		m_color_btn->SetColour(wxc);

		OnColorChange(wxc);
	}
}

void VolumePropPanel::OnColor2TextChange(wxCommandEvent& event)
{
	wxString str = m_color2_text->GetValue();
	wxColor wxc;
	if (GetColorString(str, wxc) == 3)
	{
		wxString new_str = wxString::Format("%d , %d , %d",
			wxc.Red(), wxc.Green(), wxc.Blue());
		if (str != new_str)
			m_color2_text->ChangeValue(new_str);
		m_color2_btn->SetColour(wxc);

		OnColor2Change(wxc);
	}
}

void VolumePropPanel::OnColorBtn(wxColourPickerEvent& event)
{
	wxColor wxc = event.GetColour();

	m_color_text->ChangeValue(wxString::Format("%d , %d , %d",
		wxc.Red(), wxc.Green(), wxc.Blue()));

	OnColorChange(wxc);
}

void VolumePropPanel::OnColor2Btn(wxColourPickerEvent& event)
{
	wxColor wxc = event.GetColour();

	m_color2_text->ChangeValue(wxString::Format("%d , %d , %d",
		wxc.Red(), wxc.Green(), wxc.Blue()));

	OnColor2Change(wxc);
}

void VolumePropPanel::OnColorTextFocus(wxCommandEvent& event)
{
	m_color_text->SetSelection(0, -1);
}

void VolumePropPanel::OnColor2TextFocus(wxCommandEvent& event)
{
	m_color2_text->SetSelection(0, -1);
}

void VolumePropPanel::OnInvCheck(wxCommandEvent &event)
{
	bool inv = m_options_toolbar->GetToolState(ID_InvChk);
	if (inv)
		m_options_toolbar->SetToolNormalBitmap(ID_InvChk,
			wxGetBitmapFromMemory(invert));
	else
		m_options_toolbar->SetToolNormalBitmap(ID_InvChk,
			wxGetBitmapFromMemory(invert_off));

	m_agent->setValue("invert", inv);
}

void VolumePropPanel::OnMIPCheck(wxCommandEvent &event)
{
	long val = m_options_toolbar->GetToolState(ID_MipChk) ? 1 : 0;

	//if (m_sync_group && m_group)
	//	m_group->SetMode(val);
	//else if (m_vd)
	//	m_vd->SetMode(val);
	m_agent->setValue("mip mode", val);

	//if (val == 1)
	//{
	//	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	//	if (vr_frame)
	//	{
	//		for (int i = 0; i<(int)vr_frame->GetViewList()->size(); i++)
	//		{
	//			VRenderView *vrv = (*vr_frame->GetViewList())[i];
	//			if (vrv && vrv->GetVolMethod() == VOL_METHOD_MULTI)
	//			{
	//				::wxMessageBox("MIP is not supported in Depth mode.");
	//				m_options_toolbar->ToggleTool(ID_MipChk, false);
	//				return;
	//			}
	//		}
	//	}
	//	EnableMip();
	//	if (m_threh_st)
	//		m_threh_st->SetLabel("Shade Threshold : ");
	//}
	//else
	//{
	//	DisableMip();
	//	if (m_threh_st)
	//		m_threh_st->SetLabel("Threshold : ");
	//}

	//Layout();
}

//noise reduction
void VolumePropPanel::OnNRCheck(wxCommandEvent &event)
{
	bool val = m_options_toolbar->GetToolState(ID_NRChk);
	if (val)
		m_options_toolbar->SetToolNormalBitmap(ID_NRChk,
			wxGetBitmapFromMemory(smooth));
	else
		m_options_toolbar->SetToolNormalBitmap(ID_NRChk,
			wxGetBitmapFromMemory(smooth_off));

	m_agent->setValue("noise redct", val);
	//if (m_vrv && m_vrv->GetVolMethod() == VOL_METHOD_MULTI)
	//{
	//	//for (int i=0; i<m_vrv->GetAllVolumeNum(); i++)
	//	//{
	//	//	VolumeData* vd = m_vrv->GetAllVolumeData(i);
	//	//	if (vd)
	//	//		vd->SetNR(val);
	//	//}
	//}
	//else
	//{
	//	//if (m_sync_group && m_group)
	//	//	m_group->SetNR(val);
	//	//else if (m_group && m_group->GetBlendMode()==2)
	//	//	m_group->SetNR(val);
	//	//else if (m_vd)
	//	//	m_vd->SetNR(val);
	//	m_vd->setValue("noise redct", val);
	//}

	//RefreshVRenderViews(false, true);
}

//depth mode
void VolumePropPanel::OnDepthCheck(wxCommandEvent &event)
{
	bool val = m_options_toolbar->GetToolState(ID_DepthChk);
	if (val)
		m_options_toolbar->SetToolNormalBitmap(ID_DepthChk, wxGetBitmapFromMemory(depth));
	else
		m_options_toolbar->SetToolNormalBitmap(ID_DepthChk, wxGetBitmapFromMemory(depth_off));

	//if (val)
	//{
	//	if (m_group)
	//	{
	//		m_group->SetBlendMode(2);
	//		if (m_vd)
	//		{
	//			m_group->SetNR(m_vd->GetNR());
	//			m_group->SetSampleRate(m_vd->GetSampleRate());
	//			m_group->SetShadow(m_vd->GetShadow());
	//			double sp;
	//			m_vd->GetShadowParams(sp);
	//			m_group->SetShadowParams(sp);
	//		}
	//	}
	//}
	//else
	//{
	//	if (m_group)
	//		m_group->SetBlendMode(0);
	//}

	//RefreshVRenderViews(false, true);
}

//bool VolumePropPanel::SetSpacings()
//{
//	if (!m_space_x_text || !m_space_y_text || !m_space_z_text)
//		return false;
//
//	wxString str, str_new;
//	double spcx = 0.0;
//	double spcy = 0.0;
//	double spcz = 0.0;
//
//	str = m_space_x_text->GetValue();
//	str.ToDouble(&spcx);
//	if (spcx <= 0.0)
//		return false;
//
//	str = m_space_y_text->GetValue();
//	str.ToDouble(&spcy);
//	if (spcy <= 0.0)
//		return false;
//
//	str = m_space_z_text->GetValue();
//	str.ToDouble(&spcz);
//	if (spcz <= 0.0)
//		return false;
//	bool override_vox = true;
//	//VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
//	//if (vr_frame && vr_frame->GetSettingDlg())
//	//	override_vox = vr_frame->GetSettingDlg()->GetOverrideVox();
//
//	//if ((m_sync_group || override_vox) && m_group)
//	//{
//	//	for (int i = 0; i < m_group->GetVolumeNum(); i++)
//	//	{
//	//		m_group->GetVolumeData(i)->SetSpacings(spcx, spcy, spcz);
//	//		m_group->GetVolumeData(i)->SetBaseSpacings(spcx, spcy, spcz);
//	//	}
//	//}
//	//else if (m_vd)
//	//{
//	//	m_vd->SetSpacings(spcx, spcy, spcz);
//	//	m_vd->SetBaseSpacings(spcx, spcy, spcz);
//	//}
//	//else return false;
//	m_agent->setValue("spc x", spcx);
//	m_agent->setValue("spc y", spcy);
//	m_agent->setValue("spc z", spcz);
//	m_agent->setValue("base spc x", spcx);
//	m_agent->setValue("base spc y", spcy);
//	m_agent->setValue("base spc z", spcz);
//
//	return true;
//}

//enable/disable
void VolumePropPanel::EnableAlpha()
{
	m_alpha_sldr->Enable();
	m_alpha_text->Enable();
}

void VolumePropPanel::DisableAlpha()
{
	m_alpha_sldr->Disable();
	m_alpha_text->Disable();
}

void VolumePropPanel::EnableShading()
{
	m_low_shading_sldr->Enable();
	m_low_shading_text->Enable();
	m_hi_shading_sldr->Enable();
	m_hi_shading_text->Enable();
}

void VolumePropPanel::DisableShading()
{
	m_low_shading_sldr->Disable();
	m_low_shading_text->Disable();
	m_hi_shading_sldr->Disable();
	m_hi_shading_text->Disable();
}

void VolumePropPanel::EnableShadow()
{
	m_shadow_sldr->Enable();
	m_shadow_text->Enable();
}

void VolumePropPanel::DisableShadow()
{
	m_shadow_sldr->Disable();
	m_shadow_text->Disable();
}

void VolumePropPanel::EnableColormap()
{
	m_colormap_high_value_sldr->Enable();
	m_colormap_high_value_text->Enable();
	m_colormap_low_value_sldr->Enable();
	m_colormap_low_value_text->Enable();
}

void VolumePropPanel::DisableColormap()
{
	m_colormap_high_value_sldr->Disable();
	m_colormap_high_value_text->Disable();
	m_colormap_low_value_sldr->Disable();
	m_colormap_low_value_text->Disable();
}

void VolumePropPanel::EnableMip()
{
	//DisableAlpha();
	//m_boundary_sldr->Disable();
	//m_boundary_text->Disable();
	//m_luminance_sldr->Disable();
	//m_luminance_text->Disable();
	//bool shading_enable;
	//m_agent->getValue("shading enable", shading_enable);
	//bool shadow_enable;
	//m_agent->getValue("shadow enable", shadow_enable);
	//if (shading_enable ||
	//	shadow_enable)
	//	EnableShading();
	//else
	//	DisableShading();
}

void VolumePropPanel::DisableMip()
{
	//bool alpha_enable;
	//m_agent->getValue("alpha enable", alpha_enable);
	//if (alpha_enable)
	//	EnableAlpha();
	//else
	//	DisableAlpha();
	//m_boundary_sldr->Enable();
	//m_boundary_text->Enable();
	//m_luminance_sldr->Enable();
	//m_luminance_text->Enable();
	//bool shading_enable;
	//m_agent->getValue("shading enable", shading_enable);
	//bool shadow_enable;
	//m_agent->getValue("shadow enable", shadow_enable);
	//if (shading_enable ||
	//	shadow_enable)
	//	EnableShading();
	//else
	//	DisableShading();
	//m_left_thresh_sldr->Enable();
	//m_left_thresh_text->Enable();
	//m_right_thresh_sldr->Enable();
	//m_right_thresh_text->Enable();
}

//update max value
void VolumePropPanel::UpdateMaxVal(double value)
{
	long bits;
	m_agent->getValue("bits", bits);
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
	m_agent->setValue("max int", m_max_val);
	m_agent->setValue("int scale", 65535.0 / m_max_val);
}

void VolumePropPanel::OnSpaceSync(wxMouseEvent& event)
{
	m_agent->propParentValue("spc x");
	m_agent->propParentValue("spc y");
	m_agent->propParentValue("spc z");
	m_agent->propParentValue("base spc x");
	m_agent->propParentValue("base spc y");
	m_agent->propParentValue("base spc z");
}

void VolumePropPanel::OnSpaceText(wxCommandEvent& event)
{
	wxString str;
	double dval;
	//x
	str = m_space_x_text->GetValue();
	str.ToDouble(&dval);
	if (dval > 0.0)
	{
		m_agent->setValue("spc x", dval);
		m_agent->setValue("base spc x", dval);
	}
	//y
	str = m_space_y_text->GetValue();
	str.ToDouble(&dval);
	if (dval > 0.0)
	{
		m_agent->setValue("spc y", dval);
		m_agent->setValue("base spc y", dval);
	}
	//z
	str = m_space_z_text->GetValue();
	str.ToDouble(&dval);
	if (dval > 0.0)
	{
		m_agent->setValue("spc z", dval);
		m_agent->setValue("base spc z", dval);
	}
}

//legend
void VolumePropPanel::OnLegendCheck(wxCommandEvent& event)
{
	bool leg = m_options_toolbar->GetToolState(ID_LegendChk);
	m_agent->setValue("legend", leg);
}

//interpolation
void VolumePropPanel::OnInterpolateCheck(wxCommandEvent& event)
{
	bool inv = m_options_toolbar->GetToolState(ID_InterpolateChk);
	if (inv)
		m_options_toolbar->SetToolNormalBitmap(ID_InterpolateChk,
			wxGetBitmapFromMemory(interpolate));
	else
		m_options_toolbar->SetToolNormalBitmap(ID_InterpolateChk,
			wxGetBitmapFromMemory(interpolate_off));
	m_agent->setValue("interpolate", inv);
	//if (m_vrv)
	//	m_vrv->m_glview->SetIntp(inv);
}

//sync within group
void VolumePropPanel::OnSyncGroupCheck(wxCommandEvent& event)
{
	bool sync = m_options_toolbar->GetToolState(ID_SyncGroupChk);
	std::string ss[] = {
		"gamma 3d",
		"extract boundary",
		"saturation",
		"low threshold",
		"high threshold",
		"shadow enable",
		"shadow int",
		"alpha",
		"alpha enable",
		"sample rate",
		"shading enable",
		"low shading",
		"high shading",
		"colormap enable",
		"colormap mode",
		"colormap type",
		"colormap low",
		"colormap high",
		"colormap proj",
		"invert",
		"interpolate",
		"mip mode",
		"noise redct"
	};
	std::vector<std::string> names(std::begin(ss), std::end(ss));//values to sync
	if (sync)
	{
		m_agent->propParentValues(names);
		m_agent->syncParentValues(names);
	}
	else
		m_agent->unsyncParentValues(names);
}

void VolumePropPanel::OnSaveDefault(wxCommandEvent& event)
{
	std::string ss[] = {
		"gamma 3d",
		"extract boundary",
		"saturation",
		"low threshold",
		"high threshold",
		"shadow enable",
		"shadow int",
		"alpha",
		"alpha enable",
		"sample rate",
		"shading enable",
		"low shading",
		"high shading",
		"colormap enable",
		"colormap mode",
		"colormap type",
		"colormap low",
		"colormap high",
		"colormap proj",
		"invert",
		"interpolate",
		"mip mode",
		"noise redct",
		"spc x",
		"spc y",
		"spc z"
	};
	std::vector<std::string> names_v(std::begin(ss), std::end(ss));//values to save
	FL::Global::instance().getVolumeFactory().propValuesToDefault(m_agent, names_v);
	std::set<std::string> names_s(std::begin(ss), std::end(ss));//values to save
	FL::Global::instance().getVolumeFactory().writeDefault(names_s);
}

void VolumePropPanel::OnResetDefault(wxCommandEvent &event)
{
	std::string ss[] = {
		"gamma 3d",
		"extract boundary",
		"saturation",
		"low threshold",
		"high threshold",
		"shadow enable",
		"shadow int",
		"alpha",
		"alpha enable",
		"sample rate",
		"shading enable",
		"low shading",
		"high shading",
		"colormap enable",
		"colormap mode",
		"colormap type",
		"colormap low",
		"colormap high",
		"colormap proj",
		"invert",
		"interpolate",
		"mip mode",
		"noise redct"
	};
	std::vector<std::string> names_v(std::begin(ss), std::end(ss));//values to save
	FL::Global::instance().getVolumeFactory().propValuesFromDefault(m_agent, names_v);
}
