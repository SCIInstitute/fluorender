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
#include "VPropView.h"
#include "VRenderFrame.h"
#include <VolumeData.hpp>
#include <VolumeGroup.hpp>
#include <FLIVR/MultiVolumeRenderer.h>
#include <FLIVR/VolumeRenderer.h>
#include <FLIVR/VolShaderCode.h>
#include <Types/Color.h>
#include <Types/BBox.h>
#include <Types/Point.h>
#include "png_resource.h"
#include "img/icons.h"
#include <wx/wfstream.h>
#include <wx/fileconf.h>
#include <wx/aboutdlg.h>
#include <wx/colordlg.h>
#include <wx/valnum.h>
#include <wx/hyperlink.h>
#include <wx/stdpaths.h>
#include <limits>

BEGIN_EVENT_TABLE(VPropView, wxPanel)
	//1
	EVT_COMMAND_SCROLL(ID_GammaSldr, VPropView::OnGammaChange)
	EVT_TEXT(ID_GammaText, VPropView::OnGammaText)
	EVT_COMMAND_SCROLL(ID_BoundarySldr, VPropView::OnBoundaryChange)
	EVT_TEXT(ID_BoundaryText, VPropView::OnBoundaryText)
	//2
	EVT_COMMAND_SCROLL(ID_SaturationSldr, VPropView::OnSaturationChange)
	EVT_TEXT(ID_SaturationText, VPropView::OnSaturationText)
	EVT_COMMAND_SCROLL(ID_LeftThreshSldr, VPropView::OnLeftThreshChange)
	EVT_TEXT(ID_LeftThreshText, VPropView::OnLeftThreshText)
	EVT_COMMAND_SCROLL(ID_RightThreshSldr, VPropView::OnRightThreshChange)
	EVT_TEXT(ID_RightThreshText, VPropView::OnRightThreshText)
	//3
	EVT_COMMAND_SCROLL(ID_LuminanceSldr, VPropView::OnLuminanceChange)
	EVT_TEXT(ID_LuminanceText, VPropView::OnLuminanceText)
	EVT_TOOL(ID_ShadowChk, VPropView::OnShadowEnable)
	EVT_COMMAND_SCROLL(ID_ShadowSldr, VPropView::OnShadowChange)
	EVT_TEXT(ID_ShadowText, VPropView::OnShadowText)
	EVT_COMMAND_SCROLL(ID_HiShadingSldr, VPropView::OnHiShadingChange)
	EVT_TEXT(ID_HiShadingText, VPropView::OnHiShadingText)
	//4
	EVT_TOOL(ID_AlphaChk, VPropView::OnAlphaCheck)
	EVT_COMMAND_SCROLL(ID_AlphaSldr, VPropView::OnAlphaChange)
	EVT_TEXT(ID_Alpha_Text, VPropView::OnAlphaText)
	EVT_COMMAND_SCROLL(ID_SampleSldr, VPropView::OnSampleChange)
	EVT_TEXT(ID_SampleText, VPropView::OnSampleText)
	//5
	EVT_COMMAND_SCROLL(ID_LowShadingSldr, VPropView::OnLowShadingChange)
	EVT_TEXT(ID_LowShadingText, VPropView::OnLowShadingText)
	EVT_TOOL(ID_ShadingEnableChk, VPropView::OnShadingEnable)
	//colormap
	EVT_TOOL(ID_ColormapEnableChk, VPropView::OnEnableColormap)
	EVT_COMMAND_SCROLL(ID_ColormapHighValueSldr, VPropView::OnColormapHighValueChange)
	EVT_TEXT(ID_ColormapHighValueText, VPropView::OnColormapHighValueText)
	EVT_COMMAND_SCROLL(ID_ColormapLowValueSldr, VPropView::OnColormapLowValueChange)
	EVT_TEXT(ID_ColormapLowValueText, VPropView::OnColormapLowValueText)
	EVT_TOGGLEBUTTON(ID_ColormapInvBtn, VPropView::OnColormapInvBtn)
	EVT_COMBOBOX(ID_ColormapCombo, VPropView::OnColormapCombo)
	EVT_COMBOBOX(ID_ColormapCombo2, VPropView::OnColormapCombo2)
	//6
	//color 1
	EVT_TEXT(ID_ColorText, VPropView::OnColorTextChange)
	EVT_COLOURPICKER_CHANGED(ID_ColorBtn, VPropView::OnColorBtn)
	EVT_TEXT(ID_Color2Text, VPropView::OnColor2TextChange)
	EVT_COLOURPICKER_CHANGED(ID_Color2Btn, VPropView::OnColor2Btn)
	//spacings
	EVT_TEXT(ID_SpaceXText, VPropView::OnSpaceText)
	EVT_TEXT(ID_SpaceYText, VPropView::OnSpaceText)
	EVT_TEXT(ID_SpaceZText, VPropView::OnSpaceText)
	//legend
	EVT_TOOL(ID_LegendChk, VPropView::OnLegendCheck)
	//EVT_TOOL
	EVT_TOOL(ID_InterpolateChk, VPropView::OnInterpolateCheck)
	//sync within group
	EVT_TOOL(ID_SyncGroupChk, VPropView::OnSyncGroupCheck)
	//save default
	EVT_TOOL(ID_SaveDefault, VPropView::OnSaveDefault)
	EVT_TOOL(ID_ResetDefault, VPropView::OnResetDefault)
	//inversion
	EVT_TOOL(ID_InvChk, VPropView::OnInvCheck)
	//MIP
	EVT_TOOL(ID_MipChk, VPropView::OnMIPCheck)
	//noise reduction
	EVT_TOOL(ID_NRChk, VPropView::OnNRCheck)
	//depth mode
	EVT_TOOL(ID_DepthChk, VPropView::OnDepthCheck)
	//transp
	EVT_TOOL(ID_TranspChk, VPropView::OnTranspChk)
	//components
	EVT_TOOL(ID_CompChk, VPropView::OnCompChk)
END_EVENT_TABLE()

VPropView::VPropView(VRenderFrame* frame,
	wxWindow* parent,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name) :
	wxPanel(parent, wxID_ANY, pos, size,style, name),
	m_frame(frame),
	m_vd(0),
	m_lumi_change(false),
	m_sync_group(false),
	m_group(0),
	m_view(0),
	m_max_val(255.0),
	m_space_x_text(0),
	m_space_y_text(0),
	m_space_z_text(0)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);

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

	//validator: floating point 2
	wxFloatingPointValidator<double> vald_fp2(2);
	//validator: floating point 3
	wxFloatingPointValidator<double> vald_fp3(3);
	//validator: floating point 4
	wxFloatingPointValidator<double> vald_fp4(4);
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;

	//left///////////////////////////////////////////////////
	//gamma
	m_gamma_st = new wxStaticText(this, ID_GammaSync, " : Gamma",
		wxDefaultPosition, wxSize(100, -1));
	m_gamma_st->Connect(ID_GammaSync, wxEVT_LEFT_DCLICK,
		wxMouseEventHandler(VPropView::OnGammaSync), NULL, this);
	m_gamma_st->Connect(ID_GammaSync, wxEVT_RIGHT_DCLICK,
		wxMouseEventHandler(VPropView::OnGammaSync), NULL, this);
	m_gamma_sldr = new wxSlider(this, ID_GammaSldr, 100, 10, 400,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL|wxSL_INVERSE);
	m_gamma_text = new wxTextCtrl(this, ID_GammaText, "1.00",
		wxDefaultPosition, wxSize(40, 20), 0, vald_fp2);
	sizer_l1->Add(m_gamma_sldr, 1, wxEXPAND);
	sizer_l1->Add(m_gamma_text, 0, wxALIGN_CENTER);
	sizer_l1->Add(m_gamma_st, 0, wxALIGN_CENTER);
	//saturation point
	m_saturation_st = new wxStaticText(this, ID_SaturationSync, " : Saturation",
		wxDefaultPosition, wxSize(100, -1));
	m_saturation_st->Connect(ID_SaturationSync, wxEVT_LEFT_DCLICK,
		wxMouseEventHandler(VPropView::OnSaturationSync), NULL, this);
	m_saturation_st->Connect(ID_SaturationSync, wxEVT_RIGHT_DCLICK,
		wxMouseEventHandler(VPropView::OnSaturationSync), NULL, this);
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
		wxMouseEventHandler(VPropView::OnLuminanceSync), NULL, this);
	m_luminance_st->Connect(ID_LuminanceSync, wxEVT_RIGHT_DCLICK,
		wxMouseEventHandler(VPropView::OnLuminanceSync), NULL, this);
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
		"Enable Alpha adjustments",
		"Enable Alpha adjustments");
	m_alpha_tool->ToggleTool(ID_AlphaChk,true);
	m_alpha_tool->Realize();
	m_alpha_tool->Connect(ID_AlphaSync, wxEVT_LEFT_DCLICK,
		wxMouseEventHandler(VPropView::OnAlphaSync), NULL, this);
	m_alpha_tool->Connect(ID_AlphaSync, wxEVT_RIGHT_DCLICK,
		wxMouseEventHandler(VPropView::OnAlphaSync), NULL, this);
	m_alpha_sldr = new wxSlider(this, ID_AlphaSldr, 127, 0, 255,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_alpha_text = new wxTextCtrl(this, ID_Alpha_Text, "127",
		wxDefaultPosition, wxSize(40, 20), 0, vald_int);
	sizer_l4->Add(m_alpha_sldr, 1, wxEXPAND);
	sizer_l4->Add(m_alpha_text, 0, wxALIGN_CENTER);
	sizer_l4->Add(new wxStaticText(this, 0 , " : ", 
		wxDefaultPosition,wxSize(13,-1)), 0, wxALIGN_CENTER);
	sizer_l4->Add(m_alpha_tool, 0, wxALIGN_CENTER);
	sizer_l4->Add(30,10,0);
	//highlight
	m_hi_shading_sldr = new wxSlider(this, ID_HiShadingSldr, 0, 0, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_hi_shading_text = new wxTextCtrl(this, ID_HiShadingText, "0.00",
		wxDefaultPosition, wxSize(50, 20), 0, vald_fp2);
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
	m_shade_tool->AddCheckTool(ID_ShadingEnableChk,"Shading",
		bitmap, wxNullBitmap,
		"Enable Shading adjustments",
		"Enable Shading adjustments for high and low values");
	m_shade_tool->ToggleTool(ID_ShadingEnableChk,true);
	m_shade_tool->Realize();
	m_shade_tool->Connect(ID_ShadingSync, wxEVT_LEFT_DCLICK,
		wxMouseEventHandler(VPropView::OnShadingSync), NULL, this);
	m_shade_tool->Connect(ID_ShadingSync, wxEVT_RIGHT_DCLICK,
		wxMouseEventHandler(VPropView::OnShadingSync), NULL, this);
	sizer_l5->Add(m_hi_shading_sldr, 1, wxEXPAND);
	sizer_l5->Add(m_hi_shading_text, 0, wxALIGN_CENTER);
	sizer_l5->Add(m_low_shading_sldr, 1, wxEXPAND);
	sizer_l5->Add(m_low_shading_text, 0, wxALIGN_CENTER);
	sizer_l5->Add(new wxStaticText(this, 0 , " : ", 
		wxDefaultPosition,wxSize(13,-1)), 0, wxALIGN_CENTER);
	sizer_l5->Add(m_shade_tool, 0, wxALIGN_CENTER);
	sizer_l5->Add(30,10,0);
	//middle///////////////////////////////////////////////////
	//extract boundary
	m_boundary_st = new wxStaticText(this, ID_BoundarySync, "Extract Boundary : ",
		wxDefaultPosition, wxSize(127, -1), wxALIGN_RIGHT);
	m_boundary_st->Connect(ID_BoundarySync, wxEVT_LEFT_DCLICK,
		wxMouseEventHandler(VPropView::OnBoundarySync), NULL, this);
	m_boundary_st->Connect(ID_BoundarySync, wxEVT_RIGHT_DCLICK,
		wxMouseEventHandler(VPropView::OnBoundarySync), NULL, this);
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
		wxMouseEventHandler(VPropView::OnThreshSync), NULL, this);
	m_threh_st->Connect(ID_ThreshSync, wxEVT_RIGHT_DCLICK,
		wxMouseEventHandler(VPropView::OnThreshSync), NULL, this);
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
	sizer_m2->Add(m_right_thresh_sldr,1, wxEXPAND);
	//shadow
	m_shadow_tool = new wxToolBar(this, ID_ShadowSync,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	bitmap = wxGetBitmapFromMemory(shadow);
#ifdef _DARWIN
	m_shadow_tool->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_shadow_tool->AddCheckTool(ID_ShadowChk, "Shadow",
		bitmap, wxNullBitmap,
		"Enable Shadow and its adjustments",
		"Enable Shadow and its adjustments");
	m_shadow_tool->ToggleTool(ID_ShadowChk,false);
	m_shadow_tool->Realize();
	m_shadow_tool->Connect(ID_ShadowSync, wxEVT_LEFT_DCLICK,
		wxMouseEventHandler(VPropView::OnShadowSync), NULL, this);
	m_shadow_tool->Connect(ID_ShadowSync, wxEVT_RIGHT_DCLICK,
		wxMouseEventHandler(VPropView::OnShadowSync), NULL, this);
	st = new wxStaticText(this, 0, " : ",
		wxDefaultPosition, wxSize(20, -1), wxALIGN_RIGHT);
	m_shadow_sldr = new wxSlider(this, ID_ShadowSldr, 0, 0, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_shadow_text = new wxTextCtrl(this, ID_ShadowText, "0.00",
		wxDefaultPosition, wxSize(50, 20), 0, vald_fp2);
	sizer_m3->Add(50, -1, 0);
	sizer_m3->Add(m_shadow_tool, 0, wxALIGN_CENTER);
	sizer_m3->Add(st, 0, wxALIGN_CENTER);
	sizer_m3->Add(m_shadow_text, 0, wxALIGN_CENTER);
	sizer_m3->Add(m_shadow_sldr, 1, wxEXPAND);
	//sample rate
	m_sample_st = new wxStaticText(this, ID_SampleSync, "Sample Rate : ",
		wxDefaultPosition, wxSize(127, -1), wxALIGN_RIGHT);
	m_sample_st->Connect(ID_SampleSync, wxEVT_LEFT_DCLICK,
		wxMouseEventHandler(VPropView::OnSampleSync), NULL, this);
	m_sample_st->Connect(ID_SampleSync, wxEVT_RIGHT_DCLICK,
		wxMouseEventHandler(VPropView::OnSampleSync), NULL, this);
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
		"Enable Color Maps",
		"Enable Color Maps");
	m_colormap_tool->ToggleTool(ID_ColormapEnableChk,false);
	m_colormap_tool->Realize();
	m_colormap_tool->Connect(ID_ColormapSync, wxEVT_LEFT_DCLICK,
		wxMouseEventHandler(VPropView::OnColormapSync), NULL, this);
	m_colormap_tool->Connect(ID_ColormapSync, wxEVT_RIGHT_DCLICK,
		wxMouseEventHandler(VPropView::OnColormapSync), NULL, this);
	sizer_m5->Add(50,50,0);
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
		wxDefaultPosition + wxPoint(10,0), wxSize(50, 20), 0, vald_int);
	sizer_m5->Add(m_colormap_high_value_text, 0, wxALIGN_CENTER);
	m_colormap_high_value_sldr = new wxSlider(this, 
		ID_ColormapHighValueSldr, 255, 0, 255,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	sizer_m5->Add(m_colormap_high_value_sldr, 1, wxEXPAND);

	//right ///////////////////////////////////////////////////
	m_options_toolbar = new wxToolBar(this,wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	bitmap = wxGetBitmapFromMemory(transplo);
#ifdef _DARWIN
	m_options_toolbar->SetToolBitmapSize(bitmap.GetSize());
#endif
	//transparency
	m_options_toolbar->AddCheckTool(ID_TranspChk, "Increase Transpancy",
		bitmap, wxNullBitmap,
		"Enable High Tarnsparency mode",
		"Enable High Tarnsparency mode");
	m_options_toolbar->ToggleTool(ID_TranspChk, false);
	//MIP
	bitmap = wxGetBitmapFromMemory(mip);
	m_options_toolbar->AddCheckTool(ID_MipChk, "MIP",
		bitmap, wxNullBitmap,
		"Enable Maximum Intensity Projection (MIP)",
		"Enable Maximum Intensity Projection (MIP)");
	m_options_toolbar->ToggleTool(ID_MipChk,false);
	//inversion
	bitmap = wxGetBitmapFromMemory(invert_off);
	m_options_toolbar->AddCheckTool(ID_InvChk, "Inversion",
		bitmap, wxNullBitmap,
		"Invert data intensity values",
		"Invert data intensity values");
	m_options_toolbar->ToggleTool(ID_InvChk,false);
	//component display
	bitmap = wxGetBitmapFromMemory(comp_off);
	m_options_toolbar->AddCheckTool(ID_CompChk, "Components",
		bitmap, wxNullBitmap,
		"Show components",
		"Show components");
	m_options_toolbar->ToggleTool(ID_CompChk, false);
	//interpolation
	bitmap = wxGetBitmapFromMemory(interpolate);
	m_options_toolbar->AddCheckTool(ID_InterpolateChk, "Interpolate",
		bitmap, wxNullBitmap,
		"Enable spatial interpolation of voxel intensity values",
		"Enable spatial interpolation of voxel intensity values");
	m_options_toolbar->ToggleTool(ID_InterpolateChk,true);
	//noise reduction
	bitmap = wxGetBitmapFromMemory(smooth_off);
	m_options_toolbar->AddCheckTool(ID_NRChk, "Smoothing",
		bitmap, wxNullBitmap,
		"Enable rendering result smoothing",
		"Enable rendering result smoothing");
	m_options_toolbar->ToggleTool(ID_NRChk,false);
	//sync group
	bitmap = wxGetBitmapFromMemory(sync_chan);
	m_options_toolbar->AddCheckTool(ID_SyncGroupChk,"Group Sync",
		bitmap, wxNullBitmap,
		"Sync current channel with other channels in the group",
		"Sync current channel with other channels in the group");
	m_options_toolbar->ToggleTool(ID_SyncGroupChk,false);
	//depth mode
	bitmap = wxGetBitmapFromMemory(depth_off);
	m_options_toolbar->AddCheckTool(ID_DepthChk, "Depth Mode",
		bitmap, wxNullBitmap,
		"Enable Depth Mode within the group",
		"Enable Depth Mode within the group");
	m_options_toolbar->ToggleTool(ID_DepthChk,false);
	//legend
	bitmap = wxGetBitmapFromMemory(legend);
	m_options_toolbar->AddCheckTool(ID_LegendChk, "Legend",
		bitmap, wxNullBitmap,
		"Enable name legend display for current channel",
		"Enable name legend display for current channel");
	m_options_toolbar->ToggleTool(ID_LegendChk,true);
	//buttons
	bitmap = wxGetBitmapFromMemory(reset);
	m_options_toolbar->AddTool(ID_ResetDefault,"Reset",
		bitmap, "Reset all properties");
	bitmap = wxGetBitmapFromMemory(save_settings);
	m_options_toolbar->AddTool(ID_SaveDefault,"Save",
		bitmap, "Set current settings as default");
	sizer_r1->AddStretchSpacer();
	sizer_r1->Add(m_options_toolbar, 0, wxALIGN_CENTER);
	sizer_r1->AddStretchSpacer();
	m_options_toolbar->Realize();
	//spacings
	//x
	st = new wxStaticText(this, 0, "Voxel Size: ",
		wxDefaultPosition, wxSize(70, -1), wxALIGN_RIGHT);
	m_space_x_text = new wxTextCtrl(this, ID_SpaceXText, "1.000",
		wxDefaultPosition, wxSize(50, -1), 0, vald_fp3);
	sizer_r2->Add(st, 0, wxALIGN_CENTER);
	//sizer_r2->AddStretchSpacer();
	st = new wxStaticText(this, 0, "X ");
	sizer_r2->Add(st, 0, wxALIGN_CENTER);
	sizer_r2->Add(m_space_x_text, 1, wxALIGN_CENTER);
	//y
	st = new wxStaticText(this, 0, "Y ");
	m_space_y_text = new wxTextCtrl(this, ID_SpaceYText, "1.000",
		wxDefaultPosition, wxSize(50, -1), 0, vald_fp3);
	sizer_r2->Add(3, 5, 0);
	sizer_r2->Add(st, 0, wxALIGN_CENTER);
	sizer_r2->Add(m_space_y_text, 1, wxALIGN_CENTER);
	//z
	st = new wxStaticText(this, 0, "Z ");
	m_space_z_text = new wxTextCtrl(this, ID_SpaceZText, "1.000",
		wxDefaultPosition, wxSize(50, -1), 0, vald_fp3);
	sizer_r2->Add(3, 5, 0);
	sizer_r2->Add(st, 0, wxALIGN_CENTER);
	sizer_r2->Add(m_space_z_text, 1, wxALIGN_CENTER);
	//color 1
	st = new wxStaticText(this, 0, "Prime Color: ",
		wxDefaultPosition, wxSize(70, -1), wxALIGN_RIGHT);
	m_color_text = new wxTextCtrl(this, ID_ColorText, "255 , 255 , 255",
		wxDefaultPosition, wxSize(50, 20));
	m_color_text->Connect(ID_ColorText, wxEVT_LEFT_DCLICK,
		wxCommandEventHandler(VPropView::OnColorTextFocus),
		NULL, this);
	m_color_btn = new wxColourPickerCtrl(this, ID_ColorBtn, *wxRED,
		wxDefaultPosition, wxSize(50, 25));
	sizer_r3->Add(st, 0, wxALIGN_CENTER, 0); 
	sizer_r3->Add(5, 5, 0);
	sizer_r3->Add(m_color_text, 1, wxALIGN_CENTER, 0);
	sizer_r3->Add(m_color_btn, 1, wxALIGN_CENTER, 0);
	//color 2
	st = new wxStaticText(this, 0, "Secnd Color: ",
		wxDefaultPosition, wxSize(70, -1), wxALIGN_RIGHT);
	m_color2_text = new wxTextCtrl(this, ID_Color2Text, "255 , 255 , 255",
		wxDefaultPosition, wxSize(50, 20));
	m_color2_text->Connect(ID_Color2Text, wxEVT_LEFT_DCLICK,
		wxCommandEventHandler(VPropView::OnColor2TextFocus),
		NULL, this);
	m_color2_btn = new wxColourPickerCtrl(this, ID_Color2Btn, *wxRED,
		wxDefaultPosition, wxSize(50, 25));
	sizer_r4->Add(st, 0, wxALIGN_CENTER, 0); 
	sizer_r4->Add(5, 5, 0);
	sizer_r4->Add(m_color2_text, 1, wxALIGN_CENTER, 0);
	sizer_r4->Add(m_color2_btn, 1, wxALIGN_CENTER, 0);
	// colormap chooser
	st = new wxStaticText(this, 0, "Effects: ",
		wxDefaultPosition, wxSize(70, -1), wxALIGN_RIGHT);
	m_colormap_inv_btn = new wxToggleButton(this, ID_ColormapInvBtn,
		L"\u262f", wxDefaultPosition, wxSize(24, 24));
#ifdef _WIN32
	wxFont font(30, wxFONTFAMILY_DEFAULT, wxNORMAL, wxNORMAL);
#else
	wxFont font(15, wxFONTFAMILY_DEFAULT, wxNORMAL, wxNORMAL);
#endif
	m_colormap_inv_btn->SetFont(font);
	m_colormap_combo = new wxComboBox(this, ID_ColormapCombo, "",
		wxDefaultPosition, wxSize(85, 25), 0, NULL, wxCB_READONLY);
	vector<string>colormap_list;
	colormap_list.push_back("Rainbow");
	colormap_list.push_back("Hot");
	colormap_list.push_back("Cool");
	colormap_list.push_back("Diverging");
	colormap_list.push_back("Monochrome");
	colormap_list.push_back("High-key");
	colormap_list.push_back("Low-key");
	colormap_list.push_back("Hi Transparency");
	for (size_t i=0; i<colormap_list.size(); ++i)
		m_colormap_combo->Append(colormap_list[i]);
	m_colormap_combo2 = new wxComboBox(this, ID_ColormapCombo2, "",
		wxDefaultPosition, wxSize(85, 25), 0, NULL, wxCB_READONLY);
	vector<string>colormap_list2;
	colormap_list2.push_back("Intensity");
	colormap_list2.push_back("Z Value");
	colormap_list2.push_back("Y Value");
	colormap_list2.push_back("X Value");
	colormap_list2.push_back("Gradient");
	colormap_list2.push_back("Differential");
	for (size_t i=0; i<colormap_list2.size(); ++i)
		m_colormap_combo2->Append(colormap_list2[i]);
	sizer_r5->Add(st, 0, wxALIGN_CENTER, 0);
	sizer_r5->Add(5, 5, 0);
	sizer_r5->Add(m_colormap_inv_btn, 0, wxALIGN_CENTER);
	sizer_r5->Add(m_colormap_combo, 1, wxALIGN_CENTER, 0);
	sizer_r5->Add(m_colormap_combo2, 1, wxALIGN_CENTER, 0);

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
	sizer_right->Add(-1, 3);
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

VPropView::~VPropView()
{
}

void VPropView::GetSettings()
{
	if (!m_vd)
		return;

	wxString str;
	double dval = 0.0;
	int ival = 0;

	//maximum value
	m_vd->getValue(gstMaxInt, m_max_val);
	m_max_val = std::max(255.0, m_max_val);

	//set range
	wxFloatingPointValidator<double>* vald_fp;
	wxIntegerValidator<unsigned int>* vald_i;

	//volume properties
	//transfer function
	//gamma
	if ((vald_fp = (wxFloatingPointValidator<double>*)m_gamma_text->GetValidator()))
		vald_fp->SetRange(0.0, 10.0);
	m_vd->getValue(gstGamma3d, dval);
	m_gamma_sldr->SetValue(int(dval*100.0+0.5));
	str = wxString::Format("%.2f", dval);
	m_gamma_text->ChangeValue(str);
	//boundary
	if ((vald_fp = (wxFloatingPointValidator<double>*)m_boundary_text->GetValidator()))
		vald_fp->SetRange(0.0, 1.0);
	m_vd->getValue(gstExtractBoundary, dval);
	m_boundary_sldr->SetValue(int(dval*2000.0+0.5));
	str = wxString::Format("%.4f", dval);
	m_boundary_text->ChangeValue(str);
	//contrast
	if ((vald_i = (wxIntegerValidator<unsigned int>*)m_saturation_text->GetValidator()))
		vald_i->SetMin(0);
	m_vd->getValue(gstSaturation, dval);
	ival = int(dval*m_max_val+0.5);
	m_saturation_sldr->SetRange(0, int(m_max_val));
	str = wxString::Format("%d", ival);
	m_saturation_sldr->SetValue(ival);
	m_saturation_text->ChangeValue(str);
	//left threshold
	if ((vald_i = (wxIntegerValidator<unsigned int>*)m_left_thresh_text->GetValidator()))
		vald_i->SetMin(0);
	m_vd->getValue(gstLowThreshold, dval);
	ival = int(dval*m_max_val+0.5);
	m_left_thresh_sldr->SetRange(0, int(m_max_val));
	str = wxString::Format("%d", ival);
	m_left_thresh_sldr->SetValue(ival);
	m_left_thresh_text->ChangeValue(str);
	//right threshold
	if ((vald_i = (wxIntegerValidator<unsigned int>*)m_right_thresh_text->GetValidator()))
		vald_i->SetMin(0);
	m_vd->getValue(gstHighThreshold, dval);
	ival = int(dval*m_max_val+0.5);
	m_right_thresh_sldr->SetRange(0, int(m_max_val));
	str = wxString::Format("%d", ival);
	m_right_thresh_sldr->SetValue(ival);
	m_right_thresh_text->ChangeValue(str);
	//luminance
	if ((vald_i = (wxIntegerValidator<unsigned int>*)m_luminance_text->GetValidator()))
		vald_i->SetMin(0);
	m_vd->getValue(gstLuminance, dval);
	ival = int(dval*m_max_val+0.5);
	m_luminance_sldr->SetRange(0, int(m_max_val));
	str = wxString::Format("%d", ival);
	m_luminance_sldr->SetValue(ival);
	m_luminance_text->ChangeValue(str);
	//color
	fluo::Color c;
	m_vd->getValue(gstColor, c);
	wxColor wxc((unsigned char)(c.r()*255+0.5),
		(unsigned char)(c.g()*255+0.5),
		(unsigned char)(c.b()*255+0.5));
	m_color_text->ChangeValue(wxString::Format("%d , %d , %d",
		wxc.Red(), wxc.Green(), wxc.Blue()));
	m_color_btn->SetColour(wxc);
	m_vd->getValue(gstSecColor, c);
	wxc = wxColor((unsigned char)(c.r()*255+0.5),
		(unsigned char)(c.g()*255+0.5),
		(unsigned char)(c.b()*255+0.5));
	m_color2_text->ChangeValue(wxString::Format("%d , %d , %d",
		wxc.Red(), wxc.Green(), wxc.Blue()));
	m_color2_btn->SetColour(wxc);
	//alpha
	if ((vald_i = (wxIntegerValidator<unsigned int>*)m_alpha_text->GetValidator()))
		vald_i->SetMin(0);
	m_vd->getValue(gstAlpha, dval);
	ival = int(dval*m_max_val+0.5);
	m_alpha_sldr->SetRange(0, int(m_max_val));
	str = wxString::Format("%d", ival);
	m_alpha_sldr->SetValue(ival);
	m_alpha_text->ChangeValue(str);
	bool alpha;
	m_vd->getValue(gstAlphaEnable, alpha);
	m_alpha_tool->ToggleTool(ID_AlphaChk,alpha);

	//shadings
	if ((vald_fp = (wxFloatingPointValidator<double>*)m_low_shading_text->GetValidator()))
		vald_fp->SetRange(0.0, 10.0);
	if ((vald_fp = (wxFloatingPointValidator<double>*)m_hi_shading_text->GetValidator()))
		vald_fp->SetRange(0.0, 100.0);
	double amb, diff, spec, shine;
	m_vd->getValue(gstMatAmb, amb);
	m_vd->getValue(gstMatDiff, diff);
	m_vd->getValue(gstMatSpec, spec);
	m_vd->getValue(gstMatShine, shine);
	m_low_shading_sldr->SetValue(amb*100.0);
	str = wxString::Format("%.2f", amb);
	m_low_shading_text->ChangeValue(str);
	m_hi_shading_sldr->SetValue(shine*10.0);
	str = wxString::Format("%.2f", shine);
	m_hi_shading_text->ChangeValue(str);
	bool shading = m_vd->GetRenderer()->get_shading();
	m_shade_tool->ToggleTool(ID_ShadingEnableChk,shading);

	//shadow
	if ((vald_fp = (wxFloatingPointValidator<double>*)m_shadow_text->GetValidator()))
		vald_fp->SetRange(0.0, 1.0);
	bool shadow;
	m_vd->getValue(gstShadowEnable, shadow);
	m_shadow_tool->ToggleTool(ID_ShadowChk, shadow);
	double shadow_int;
	m_vd->getValue(gstShadowInt, shadow_int);
	m_shadow_sldr->SetValue(int(shadow_int*100.0+0.5));
	str = wxString::Format("%.2f", shadow_int);
	m_shadow_text->ChangeValue(str);

	//smaple rate
	if ((vald_fp = (wxFloatingPointValidator<double>*)m_sample_text->GetValidator()))
		vald_fp->SetRange(0.0, 100.0);
	double sr;
	m_vd->getValue(gstSampleRate, sr);
	m_sample_sldr->SetValue(sr*10.0);
	str = wxString::Format("%.1f", sr);
	m_sample_text->ChangeValue(str);

	//spacings
	double spcx, spcy, spcz;
	m_vd->getValue(gstSpcX, spcx);
	m_vd->getValue(gstSpcY, spcy);
	m_vd->getValue(gstSpcZ, spcz);
	if ((vald_fp = (wxFloatingPointValidator<double>*)m_space_x_text->GetValidator()))
		vald_fp->SetMin(0.0);
	str = wxString::Format("%.3f", spcx);
	m_space_x_text->ChangeValue(str);
	if ((vald_fp = (wxFloatingPointValidator<double>*)m_space_y_text->GetValidator()))
		vald_fp->SetMin(0.0);
	str = wxString::Format("%.3f", spcy);
	m_space_y_text->ChangeValue(str);
	if ((vald_fp = (wxFloatingPointValidator<double>*)m_space_z_text->GetValidator()))
		vald_fp->SetMin(0.0);
	str = wxString::Format("%.3f", spcz);
	m_space_z_text->ChangeValue(str);

	//legend
	bool bval;
	m_vd->getValue("legend", bval);
	m_options_toolbar->ToggleTool(ID_LegendChk, bval);

	//interpolate
	bool interp;
	m_vd->getValue("interpolate", interp);
	m_options_toolbar->ToggleTool(ID_InterpolateChk, interp);
	if(interp) 
		m_options_toolbar->SetToolNormalBitmap(ID_InterpolateChk, 
		wxGetBitmapFromMemory(interpolate));
	else
		m_options_toolbar->SetToolNormalBitmap(ID_InterpolateChk, 
		wxGetBitmapFromMemory(interpolate_off));

	//sync group
	if (m_group)
		m_group->getValue("sync group", m_sync_group);
	m_options_toolbar->ToggleTool(ID_SyncGroupChk,m_sync_group);

	//colormap
	double low, high;
	m_vd->getValue(gstColormapLow, low);
	m_vd->getValue(gstColormapHigh, high);
	//low
	if ((vald_i = (wxIntegerValidator<unsigned int>*)m_colormap_low_value_text->GetValidator()))
		vald_i->SetMin(0);
	ival = int(low*m_max_val+0.5);
	m_colormap_low_value_sldr->SetRange(0, int(m_max_val));
	str = wxString::Format("%d", ival);
	m_colormap_low_value_sldr->SetValue(ival);
	m_colormap_low_value_text->ChangeValue(str);
	//high
	if ((vald_i = (wxIntegerValidator<unsigned int>*)m_colormap_high_value_text->GetValidator()))
		vald_i->SetMin(0);
	ival = int(high*m_max_val+0.5);
	m_colormap_high_value_sldr->SetRange(0, int(m_max_val));
	str = wxString::Format("%d", ival);
	m_colormap_high_value_sldr->SetValue(ival);
	m_colormap_high_value_text->ChangeValue(str);
	//colormap
	m_vd->getValue(gstColormapInv, dval);
	m_colormap_inv_btn->SetValue(dval>0.0?false:true);
	long lval;
	m_vd->getValue(gstColormapType, lval);
	m_colormap_combo->SetSelection(lval);
	m_vd->getValue(gstColormapProj, lval);
	m_colormap_combo2->SetSelection(lval);
	//mode
	bool colormap_enable;
	m_vd->getValue(gstColormapEnable, colormap_enable);
	m_colormap_tool->ToggleTool(ID_ColormapEnableChk, colormap_enable);

	//inversion
	m_vd->getValue(gstInvert, bval);
	m_options_toolbar->ToggleTool(ID_InvChk, bval);
	if(bval)
		m_options_toolbar->SetToolNormalBitmap(ID_InvChk, 
		wxGetBitmapFromMemory(invert));
	else
		m_options_toolbar->SetToolNormalBitmap(ID_InvChk, 
		wxGetBitmapFromMemory(invert_off));

	//MIP
	long mip_mode;
	m_vd->getValue(gstMipMode, mip_mode);
	if (mip_mode == 1)
	{
		m_options_toolbar->ToggleTool(ID_MipChk,true);
		if (m_threh_st)
			m_threh_st->SetLabel("Shade Threshold : ");
	}
	else
	{
		m_options_toolbar->ToggleTool(ID_MipChk,false);
		if (m_threh_st)
			m_threh_st->SetLabel("Threshold : ");
	}

	//transparency
	m_vd->getValue(gstAlphaPower, dval);
	if (dval > 1.1)
	{
		m_options_toolbar->ToggleTool(ID_TranspChk, true);
		m_options_toolbar->SetToolNormalBitmap(ID_TranspChk,
			wxGetBitmapFromMemory(transphi));
	}
	else
	{
		m_options_toolbar->ToggleTool(ID_TranspChk, false);
		m_options_toolbar->SetToolNormalBitmap(ID_TranspChk,
			wxGetBitmapFromMemory(transplo));
	}

	//component display
	m_vd->getValue("label mode", lval);
	if (lval)
	{
		m_options_toolbar->ToggleTool(ID_CompChk, true);
		m_options_toolbar->SetToolNormalBitmap(ID_CompChk,
			wxGetBitmapFromMemory(comp));
	}
	else
	{
		m_options_toolbar->ToggleTool(ID_CompChk, false);
		m_options_toolbar->SetToolNormalBitmap(ID_CompChk,
			wxGetBitmapFromMemory(comp_off));
	}

	//noise reduction
	m_vd->getValue(gstNoiseRedct, bval);
	m_options_toolbar->ToggleTool(ID_NRChk, bval);
	if(bval)
		m_options_toolbar->SetToolNormalBitmap(ID_NRChk, 
		wxGetBitmapFromMemory(smooth));
	else
		m_options_toolbar->SetToolNormalBitmap(ID_NRChk, 
		wxGetBitmapFromMemory(smooth_off));

	//blend mode
	m_vd->getValue(gstBlendMode, lval);
	if (lval == 2)
	{
		m_options_toolbar->ToggleTool(ID_DepthChk,true);
		m_options_toolbar->SetToolNormalBitmap(ID_DepthChk, wxGetBitmapFromMemory(depth));
	}
	else
	{
		m_options_toolbar->ToggleTool(ID_DepthChk,false);
		m_options_toolbar->SetToolNormalBitmap(ID_DepthChk, wxGetBitmapFromMemory(depth_off));
	}

	if (alpha)
		EnableAlpha();
	else
		DisableAlpha();
	if (shading)
		EnableShading();
	else
		DisableShading();
	if (shadow)
		EnableShadow();
	else
		DisableShadow();
	if (colormap_enable)
		EnableColormap();
	else
		DisableColormap();
	if (mip_mode == 1)
		EnableMip();
	else
		DisableMip();

	Layout();
}

void VPropView::SetVolumeData(fluo::VolumeData* vd)
{
	m_vd = vd;
	GetSettings();
}

fluo::VolumeData* VPropView::GetVolumeData()
{
	return m_vd;
}

void VPropView::RefreshVRenderViews(bool tree, bool interactive)
{
	if (m_frame)
		m_frame->RefreshVRenderViews(tree, interactive);
}

void VPropView::InitVRenderViews(unsigned int type)
{
	if (m_frame)
	{
		for (int i = 0; i < m_frame->GetViewNum(); i++)
		{
			VRenderGLView* view = m_frame->GetView(i);
			if (view)
			{
				view->InitView(type);
				view->m_vrv->UpdateView();
			}
		}
	}
}

void VPropView::SetGroup(fluo::VolumeGroup* group)
{
	m_group = group;
	if (m_group)
	{
		m_group->getValue("sync group", m_sync_group);
		m_options_toolbar->ToggleTool(ID_SyncGroupChk,m_sync_group);
	}
}

fluo::VolumeGroup* VPropView::GetGroup()
{
	return m_group;
}

void VPropView::SetView(VRenderGLView *view)
{
	m_view = view;
}

VRenderGLView* VPropView::GetView()
{
	return m_view;
}

//1
void VPropView::OnGammaSync(wxMouseEvent& event)
{
	wxString str = m_gamma_text->GetValue();
	double dVal;
	str.ToDouble(&dVal);
	if (m_group)
		m_group->setValue(gstGamma3d, dVal);
	RefreshVRenderViews(false, true);
}

void VPropView::OnGammaChange(wxScrollEvent & event)
{
	double val = (double)event.GetPosition() / 100.0;
	wxString str = wxString::Format("%.2f", val);
	if (str != m_gamma_text->GetValue())
		m_gamma_text->SetValue(str);
}

void VPropView::OnGammaText(wxCommandEvent& event)
{
	wxString str = m_gamma_text->GetValue();
	double val = 0.0;
	str.ToDouble(&val);
	int ival = int(val*100.0+0.5);
	m_gamma_sldr->SetValue(ival);

	//set gamma value
	if (m_sync_group && m_group)
		m_group->setValue(gstGamma3d, val);
	else if (m_vd)
		m_vd->setValue(gstGamma3d, val);

	RefreshVRenderViews(false, true);
}

void VPropView::OnBoundarySync(wxMouseEvent& event)
{
	wxString str = m_boundary_text->GetValue();
	double dVal;
	str.ToDouble(&dVal);
	if (m_group)
		m_group->setValue(gstExtractBoundary, dVal);
	RefreshVRenderViews(false, true);
}

void VPropView::OnBoundaryChange(wxScrollEvent & event)
{
	double val = (double)event.GetPosition() / 2000.0;
	wxString str = wxString::Format("%.4f", val);
	if (str != m_boundary_text->GetValue())
		m_boundary_text->SetValue(str);
}

void VPropView::OnBoundaryText(wxCommandEvent& event)
{
	wxString str = m_boundary_text->GetValue();
	double val = 0.0;
	str.ToDouble(&val);
	int ival = int(val*2000.0+0.5);
	m_boundary_sldr->SetValue(ival);

	//set boundary value
	if (m_sync_group && m_group)
		m_group->setValue(gstExtractBoundary, val);
	else if (m_vd)
		m_vd->setValue(gstExtractBoundary, val);

	RefreshVRenderViews(false, true);
}

//2
void VPropView::OnSaturationSync(wxMouseEvent& event)
{
	wxString str = m_saturation_text->GetValue();
	long iVal;
	str.ToLong(&iVal);
	double dVal = double(iVal) / m_max_val;
	if (m_group)
		m_group->setValue(gstSaturation, dVal);
	RefreshVRenderViews(false, true);
}

void VPropView::OnSaturationChange(wxScrollEvent & event)
{
	int ival = event.GetPosition();
	wxString str = wxString::Format("%d", ival);
	if (str != m_saturation_text->GetValue())
		m_saturation_text->SetValue(str);
}

void VPropView::OnSaturationText(wxCommandEvent& event)
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

	//set contrast value
	if (m_sync_group && m_group)
		m_group->setValue(gstSaturation, val);
	else if (m_vd)
		m_vd->setValue(gstSaturation, val);

	RefreshVRenderViews(false, true);
}

void VPropView::OnThreshSync(wxMouseEvent& event)
{
	wxString str = m_left_thresh_text->GetValue();
	long iVal;
	str.ToLong(&iVal);
	double dVal = double(iVal) / m_max_val;
	if (m_group)
		m_group->setValue(gstLowThreshold, dVal);
	str = m_right_thresh_text->GetValue();
	str.ToLong(&iVal);
	dVal = double(iVal) / m_max_val;
	if (m_group)
		m_group->setValue(gstHighThreshold, dVal);
	RefreshVRenderViews(false, true);
}

void VPropView::OnLeftThreshChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	wxString str = wxString::Format("%d", ival);
	if (str != m_left_thresh_text->GetValue())
		m_left_thresh_text->SetValue(str);
}

void VPropView::OnLeftThreshText(wxCommandEvent &event)
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
		ival = int(val*m_max_val+0.5);
		wxString str2 = wxString::Format("%d", ival);
		m_left_thresh_text->ChangeValue(str2);
	}
	m_left_thresh_sldr->SetValue(ival);

	//set left threshold value
	if (m_sync_group && m_group)
		m_group->setValue(gstLowThreshold, val);
	else if (m_vd)
		m_vd->setValue(gstLowThreshold, val);

	RefreshVRenderViews(false, true);

	//update colocalization
	if (m_frame && m_frame->GetColocalizationDlg() &&
		m_frame->GetColocalizationDlg()->GetThreshUpdate())
		m_frame->GetColocalizationDlg()->Colocalize();
}

void VPropView::OnRightThreshChange(wxScrollEvent & event)
{
	int ival = event.GetPosition();
	int ival2 = m_left_thresh_sldr->GetValue();

	if (ival < ival2)
	{
		ival = ival2;
		m_right_thresh_sldr->SetValue(ival);
	}
	wxString str = wxString::Format("%d", ival);
	if (str != m_right_thresh_text->GetValue())
		m_right_thresh_text->SetValue(str);
}

void VPropView::OnRightThreshText(wxCommandEvent &event)
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

		//set right threshold value
		if (m_sync_group && m_group)
			m_group->setValue(gstHighThreshold, val);
		else if (m_vd)
			m_vd->setValue(gstHighThreshold, val);

		RefreshVRenderViews(false, true);
	}

	//update colocalization
	if (m_frame && m_frame->GetColocalizationDlg() &&
		m_frame->GetColocalizationDlg()->GetThreshUpdate())
		m_frame->GetColocalizationDlg()->Colocalize();

}

//3
void VPropView::OnLuminanceSync(wxMouseEvent& event)
{
	wxString str = m_luminance_text->GetValue();
	long iVal;
	str.ToLong(&iVal);
	double dVal = double(iVal) / m_max_val;
	if (m_group)
		m_group->setValue(gstLuminance, dVal);
	RefreshVRenderViews(true, true);
}

void VPropView::OnLuminanceChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	wxString str = wxString::Format("%d", ival);
	if (str != m_luminance_text->GetValue())
		m_luminance_text->SetValue(str);
}

void VPropView::OnLuminanceText(wxCommandEvent &event)
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

	if (m_sync_group && m_group)
		m_group->setValue(gstLuminance, val);
	else if (m_vd)
		m_vd->setValue(gstLuminance, val);

	if (m_vd)
	{
		fluo::Color color;
		m_vd->getValue(gstColor, color);
		wxColor wxc((unsigned char)(color.r()*255),
			(unsigned char)(color.g()*255),
			(unsigned char)(color.b()*255));
		m_color_text->ChangeValue(wxString::Format("%d , %d , %d",
			wxc.Red(), wxc.Green(), wxc.Blue()));
		m_color_btn->SetBackgroundColour(wxc);
		m_lumi_change = true;
	}

	RefreshVRenderViews(true, true);
}

//shadow
void VPropView::OnShadowSync(wxMouseEvent& event)
{
	bool bVal = m_shadow_tool->GetToolState(ID_ShadowChk);
	wxString str = m_shadow_text->GetValue();
	double dVal;
	str.ToDouble(&dVal);
	if (m_group)
	{
		m_group->setValue(gstShadowEnable, bVal);
		m_group->setValue(gstShadowInt, dVal);
	}
	RefreshVRenderViews(false, true);
}

void VPropView::OnShadowEnable(wxCommandEvent &event)
{
	bool shadow = m_shadow_tool->GetToolState(ID_ShadowChk);
	long lval;
	m_group->getValue(gstBlendMode, lval);
	if (m_sync_group && m_group)
		m_group->setValue(gstShadowEnable, shadow);
	else if (m_group && lval ==2)
		m_vd->setValue(gstShadowEnable, shadow);
	else if (m_vd)
		m_vd->setValue(gstShadowEnable, shadow);

	if (shadow)
		EnableShadow();
	else
		DisableShadow();

	RefreshVRenderViews(false, true);
}

void VPropView::OnShadowChange(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 100.0;
	wxString str = wxString::Format("%.2f", val);
	if (str != m_shadow_text->GetValue())
		m_shadow_text->SetValue(str);
}

void VPropView::OnShadowText(wxCommandEvent &event)
{
	wxString str = m_shadow_text->GetValue();
	double val = 0.0;
	str.ToDouble(&val);
	m_shadow_sldr->SetValue(int(val*100.0+0.5));
	long lval;
	m_group->getValue(gstBlendMode, lval);

	//set shadow darkness
	if (m_sync_group && m_group)
		m_group->setValue(gstShadowInt, val);
	else if (m_group && lval ==2)
		m_group->setValue(gstShadowInt, val);
	else if (m_vd)
		m_vd->setValue(gstShadowInt, val);

	RefreshVRenderViews(false, true);
}

//4
void VPropView::OnAlphaSync(wxMouseEvent& event)
{
	wxString str = m_alpha_text->GetValue();
	long iVal;
	str.ToLong(&iVal);
	double dVal = double(iVal) / m_max_val;
	if (m_group)
		m_group->setValue(gstAlpha, dVal);
	RefreshVRenderViews(false, true);
}

void VPropView::OnAlphaCheck(wxCommandEvent &event)
{
	bool alpha = m_alpha_tool->GetToolState(ID_AlphaChk);
	if (m_sync_group && m_group)
		m_group->setValue(gstAlphaEnable, alpha);
	else if (m_vd)
		m_vd->setValue(gstAlphaEnable, alpha);

	if (alpha)
		EnableAlpha();
	else
		DisableAlpha();

	RefreshVRenderViews(false, true);
}

void VPropView::OnAlphaChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	wxString str = wxString::Format("%d", ival);
	if (str != m_alpha_text->GetValue())
		m_alpha_text->SetValue(str);
}

void VPropView::OnAlphaText(wxCommandEvent& event)
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

	//set alpha value
	if (m_sync_group && m_group)
		m_group->setValue(gstAlpha, val);
	else if (m_vd)
		m_vd->setValue(gstAlpha, val);

	RefreshVRenderViews(false, true);
}

void VPropView::OnSampleSync(wxMouseEvent& event)
{
	wxString str = m_sample_text->GetValue();
	double srate = 0.0;
	str.ToDouble(&srate);
	if (m_group)
		m_group->setValue(gstSampleRate, srate);
	RefreshVRenderViews(false, true);
}

void VPropView::OnSampleChange(wxScrollEvent & event)
{
	double val = event.GetPosition() / 10.0;
	wxString str = wxString::Format("%.1f", val);
	if (str != m_sample_text->GetValue())
		m_sample_text->SetValue(str);
}

void VPropView::OnSampleText(wxCommandEvent& event)
{
	wxString str = m_sample_text->GetValue();
	double srate = 0.0;
	str.ToDouble(&srate);
	double val = srate*10.0;
	m_sample_sldr->SetValue(int(val));

	//set sample rate value
	if (m_view && m_view->GetVolMethod()==VOL_METHOD_MULTI)
	{
		for (int i=0; i< m_view->GetAllVolumeNum(); i++)
		{
			fluo::VolumeData* vd = m_view->GetAllVolumeData(i);
			if (vd)
				vd->setValue(gstSampleRate, srate);
		}
	}
	else
	{
		long lval;
		m_group->getValue(gstBlendMode, lval);
		if (m_sync_group && m_group)
			m_group->setValue(gstSampleRate, srate);
		else if (m_group && lval ==2)
			m_group->setValue(gstSampleRate, srate);
		else if (m_vd)
			m_vd->setValue(gstSampleRate, srate);
	}

	RefreshVRenderViews(false, true);
}

//5
void VPropView::OnShadingSync(wxMouseEvent& event)
{
	bool bVal = m_shade_tool->GetToolState(ID_ShadingEnableChk);
	wxString str = m_low_shading_text->GetValue();
	double dVal;
	str.ToDouble(&dVal);
	str = m_hi_shading_text->GetValue();
	double dVal2;
	str.ToDouble(&dVal2);
	if (m_group)
	{
		m_group->setValue(gstShadingEnable, bVal);
		m_group->setValue(gstLowShading, dVal);
		m_group->setValue(gstHighShading, dVal2);
	}
	RefreshVRenderViews(false, true);
}

//hi shading
void VPropView::OnHiShadingChange(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 10.0;
	wxString str = wxString::Format("%.2f", val);
	if (str != m_hi_shading_text->GetValue())
		m_hi_shading_text->SetValue(str);
}

void VPropView::OnHiShadingText(wxCommandEvent &event)
{
	wxString str = m_hi_shading_text->GetValue();
	double val = 0.0;
	str.ToDouble(&val);
	m_hi_shading_sldr->SetValue(int(val*10.0 + 0.5));

	//set high shading value
	if (m_sync_group && m_group)
		m_group->setValue(gstHighShading, val);
	else if (m_vd)
		m_vd->setValue(gstHighShading, val);

	RefreshVRenderViews(false, true);
}

void VPropView::OnLowShadingChange(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 100.0;
	wxString str = wxString::Format("%.2f", val);
	if (str != m_low_shading_text->GetValue())
		m_low_shading_text->SetValue(str);
}

void VPropView::OnLowShadingText(wxCommandEvent &event)
{
	wxString str = m_low_shading_text->GetValue();
	double val = 0.0;
	str.ToDouble(&val);
	m_low_shading_sldr->SetValue(int(val*100.0+0.5));

	//set low shading value
	if (m_sync_group && m_group)
		m_group->setValue(gstLowShading, val);
	else if (m_vd)
		m_vd->setValue(gstLowShading, val);

	RefreshVRenderViews(false, true);
}

void VPropView::OnShadingEnable(wxCommandEvent &event)
{
	bool shading = m_shade_tool->GetToolState(ID_ShadingEnableChk);
	if (m_sync_group && m_group)
		m_group->setValue(gstShadingEnable, shading);
	else if (m_vd)
		m_vd->setValue(gstShadingEnable, shading);

	if (shading)
		EnableShading();
	else
		DisableShading();

	RefreshVRenderViews(false, true);
}

//colormap controls
void VPropView::OnColormapSync(wxMouseEvent& event)
{
	if (m_group)
	{
		bool bVal = m_colormap_tool->GetToolState(ID_ColormapEnableChk);
		long lval = bVal ? 1 : 0;
		m_group->setValue(gstColormapMode, lval);
		m_group->setValue(gstColormapEnable, bVal);
		long iVal;
		double dVal1, dVal2;
		wxString str = m_colormap_low_value_text->GetValue();
		str.ToLong(&iVal);
		dVal1 = double(iVal) / m_max_val;
		str = m_colormap_high_value_text->GetValue();
		str.ToLong(&iVal);
		dVal2 = double(iVal) / m_max_val;
		m_group->setValue(gstColormapLow, dVal1);
		m_group->setValue(gstColormapHigh, dVal2);
		//colormap
		iVal = m_colormap_combo->GetCurrentSelection();
		m_group->setValue(gstColormapType, iVal);
		//colormap inv
		bVal = m_colormap_inv_btn->GetValue();
		m_group->setValue(gstColormapInv, bVal?-1.0:1.0);
		//colormap proj
		iVal = m_colormap_combo2->GetCurrentSelection();
		m_group->setValue(gstColormapProj, iVal);
	}
	RefreshVRenderViews(false, true);
}

void VPropView::OnEnableColormap(wxCommandEvent &event)
{
	bool colormap = 
		m_colormap_tool->GetToolState(ID_ColormapEnableChk);

	long lval = colormap ? 1 : 0;
	if (m_sync_group && m_group)
	{
		m_group->setValue(gstColormapMode, lval);
		m_group->setValue(gstColormapEnable, colormap);
	}
	else if (m_vd)
	{
		m_vd->setValue(gstColormapMode, lval);
		m_vd->setValue(gstColormapEnable, colormap);
	}

	if (m_frame)
	{
		AdjustView *adjust_view = m_frame->GetAdjustView();
		if (adjust_view)
			adjust_view->UpdateSync();
	}

	if (colormap)
		EnableColormap();
	else
		DisableColormap();

	RefreshVRenderViews(false, true);
}

void VPropView::OnColormapHighValueChange(wxScrollEvent &event)
{
	int iVal = m_colormap_high_value_sldr->GetValue();
	int iVal2 = m_colormap_low_value_sldr->GetValue();

	if (iVal < iVal2)
	{
		iVal = iVal2;
		m_colormap_high_value_sldr->SetValue(iVal);
	}
	wxString str = wxString::Format("%d", iVal);
	if (str != m_colormap_high_value_text->GetValue())
		m_colormap_high_value_text->SetValue(str);
}

void VPropView::OnColormapHighValueText(wxCommandEvent &event)
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

		double val = double(iVal)/m_max_val;

		if (m_sync_group && m_group)
			m_group->setValue(gstColormapHigh, val);
		else if (m_vd)
			m_vd->setValue(gstColormapHigh, val);

		RefreshVRenderViews(false, true);
	}
}

void VPropView::OnColormapLowValueChange(wxScrollEvent &event)
{
	int iVal = m_colormap_low_value_sldr->GetValue();
	wxString str = wxString::Format("%d", iVal);
	if (str != m_colormap_low_value_text->GetValue())
		m_colormap_low_value_text->SetValue(str);
}

void VPropView::OnColormapLowValueText(wxCommandEvent &event)
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

	double val = double(iVal)/m_max_val;

	if (m_sync_group && m_group)
		m_group->setValue(gstColormapLow, val);
	else if (m_vd)
		m_vd->setValue(gstColormapLow, val);

	RefreshVRenderViews(false, true);
}

void VPropView::OnColormapInvBtn(wxCommandEvent &event)
{
	bool val = m_colormap_inv_btn->GetValue();

	m_colormap_tool->ToggleTool(ID_ColormapEnableChk, true);
	OnEnableColormap(event);

	double dval = val ? -1.0 : 1.0;
	if (m_sync_group && m_group)
		m_group->setValue(gstColormapInv, dval);
	else if (m_vd)
		m_vd->setValue(gstColormapInv, dval);

	RefreshVRenderViews(false, true);

	//update colocalization
	if (m_frame && m_frame->GetColocalizationDlg() &&
		m_frame->GetColocalizationDlg()->GetColormapUpdate())
		m_frame->GetColocalizationDlg()->Colocalize();
}

void VPropView::OnColormapCombo(wxCommandEvent &event)
{
	long colormap = m_colormap_combo->GetCurrentSelection();

	m_colormap_tool->ToggleTool(ID_ColormapEnableChk, true);
	OnEnableColormap(event);
	if (colormap >= 5)
	{
		m_options_toolbar->ToggleTool(ID_TranspChk, true);
		OnTranspChk(event);
	}

	if (m_sync_group && m_group)
		m_group->setValue(gstColormapType, colormap);
	else if (m_vd)
		m_vd->setValue(gstColormapType, colormap);

	RefreshVRenderViews(false, true);

	//update colocalization
	if (m_frame && m_frame->GetColocalizationDlg() &&
		m_frame->GetColocalizationDlg()->GetColormapUpdate())
		m_frame->GetColocalizationDlg()->Colocalize();
}

void VPropView::OnColormapCombo2(wxCommandEvent &event)
{
	long colormap_proj = m_colormap_combo2->GetCurrentSelection();

	m_colormap_tool->ToggleTool(ID_ColormapEnableChk, true);
	OnEnableColormap(event);

	if (m_sync_group && m_group)
		m_group->setValue(gstColormapProj, colormap_proj);
	else if (m_vd)
		m_vd->setValue(gstColormapProj, colormap_proj);

	RefreshVRenderViews(false, true);
}

//6
void VPropView::OnColorChange(wxColor c)
{
	fluo::Color color(c.Red()/255.0, c.Green()/255.0, c.Blue()/255.0);
	if (m_vd)
	{
		m_vd->setValue(gstColor, color);

		double lum;
		m_vd->getValue(gstLuminance, lum);
		int ilum = int(lum*m_max_val+0.5);
		m_luminance_sldr->SetValue(ilum);
		wxString str = wxString::Format("%d", ilum);
		m_luminance_text->ChangeValue(str);

		bool bval;
		m_vd->getValue(gstSecColorSet, bval);
		if (!bval)
		{
			m_vd->getValue(gstSecColor, color);
			wxColor wxc((unsigned char)(color.r()*255),
				(unsigned char)(color.g()*255),
				(unsigned char)(color.b()*255));
			m_color2_text->ChangeValue(wxString::Format("%d , %d , %d",
				wxc.Red(), wxc.Green(), wxc.Blue()));
			m_color2_btn->SetColour(wxc);
		}

		if (m_frame)
		{
			AdjustView *adjust_view = m_frame->GetAdjustView();
			if (adjust_view)
				adjust_view->UpdateSync();
		}

		RefreshVRenderViews(true, true);
	}
}

void VPropView::OnColor2Change(wxColor c)
{
	fluo::Color color(c.Red()/255.0, c.Green()/255.0, c.Blue()/255.0);
	if (m_vd)
	{
		m_vd->setValue(gstSecColor, color);
		RefreshVRenderViews(true, true);
	}
}

int VPropView::GetColorString(wxString& str, wxColor& wxc)
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
		for (unsigned int i=0; i<str.length(); i++)
		{
			wxChar c = str[i];
			if (isdigit(c) || c=='.')
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

				if (i == str.length()-1)  //last one
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
		wxc = wxColor(fluo::Clamp(r,0,255), fluo::Clamp(g,0,255), fluo::Clamp(b,0,255));
	}
	return filled;
}

void VPropView::OnColorTextChange(wxCommandEvent& event)
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

void VPropView::OnColor2TextChange(wxCommandEvent& event)
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

void VPropView::OnColorBtn(wxColourPickerEvent& event)
{
	wxColor wxc = event.GetColour();

	m_color_text->ChangeValue(wxString::Format("%d , %d , %d",
		wxc.Red(), wxc.Green(), wxc.Blue()));

	OnColorChange(wxc);
}

void VPropView::OnColor2Btn(wxColourPickerEvent& event)
{
	wxColor wxc = event.GetColour();

	m_color2_text->ChangeValue(wxString::Format("%d , %d , %d",
		wxc.Red(), wxc.Green(), wxc.Blue()));

	OnColor2Change(wxc);
}

void VPropView::OnColorTextFocus(wxCommandEvent& event)
{
	m_color_text->SetSelection(0, -1);
}

void VPropView::OnColor2TextFocus(wxCommandEvent& event)
{
	m_color2_text->SetSelection(0, -1);
}

void VPropView::OnInvCheck(wxCommandEvent &event)
{
	bool inv = m_options_toolbar->GetToolState(ID_InvChk);
	if(inv) 
		m_options_toolbar->SetToolNormalBitmap(ID_InvChk, 
		wxGetBitmapFromMemory(invert));
	else
		m_options_toolbar->SetToolNormalBitmap(ID_InvChk, 
		wxGetBitmapFromMemory(invert_off));

	if (m_sync_group && m_group)
		m_group->setValue(gstInvert, inv);
	else if (m_vd)
		m_vd->setValue(gstInvert, inv);

	RefreshVRenderViews(false, true);
}

void VPropView::OnMIPCheck(wxCommandEvent &event)
{
	long val = m_options_toolbar->GetToolState(ID_MipChk)?1:0;

	if (m_sync_group && m_group)
		m_group->setValue(gstMipMode, val);
	else if (m_vd)
		m_vd->setValue(gstMipMode, val);

	if (val==1)
	{
		EnableMip();
		if (m_threh_st)
			m_threh_st->SetLabel("Shade Threshold : ");
	}
	else
	{
		DisableMip();
		if (m_threh_st)
			m_threh_st->SetLabel("Threshold : ");
	}

	Layout();

	RefreshVRenderViews(false, true);
}

void VPropView::OnTranspChk(wxCommandEvent &event)
{
	bool bval = m_options_toolbar->GetToolState(ID_TranspChk);
	if (bval)
	{
		m_options_toolbar->SetToolNormalBitmap(ID_TranspChk,
			wxGetBitmapFromMemory(transphi));
		if (m_sync_group && m_group)
			m_group->setValue(gstAlphaPower, 2.0);
		else if (m_vd)
			m_vd->setValue(gstAlphaPower, 2.0);
	}
	else
	{
		m_options_toolbar->SetToolNormalBitmap(ID_TranspChk,
			wxGetBitmapFromMemory(transplo));
		if (m_sync_group && m_group)
			m_group->setValue(gstAlphaPower, 1.0);
		else if (m_vd)
			m_vd->setValue(gstAlphaPower, 1.0);
	}

	RefreshVRenderViews(false, true);
}

void VPropView::OnCompChk(wxCommandEvent &event)
{
	bool bval = m_options_toolbar->GetToolState(ID_CompChk);
	if (bval)
	{
		m_options_toolbar->SetToolNormalBitmap(ID_CompChk,
			wxGetBitmapFromMemory(comp));
		if (m_sync_group && m_group)
			m_group->setValue("label mode", long(1));
		else if (m_vd)
			m_vd->setValue("label mode", long(1));
	}
	else
	{
		m_options_toolbar->SetToolNormalBitmap(ID_CompChk,
			wxGetBitmapFromMemory(comp_off));
		if (m_sync_group && m_group)
			m_group->setValue("label mode", long(0));
		else if (m_vd)
			m_vd->setValue("label mode", long(0));
	}

	RefreshVRenderViews(false, true);
}

//noise reduction
void VPropView::OnNRCheck(wxCommandEvent &event)
{
	bool val = m_options_toolbar->GetToolState(ID_NRChk);
	if(val) 
		m_options_toolbar->SetToolNormalBitmap(ID_NRChk, 
		wxGetBitmapFromMemory(smooth));
	else
		m_options_toolbar->SetToolNormalBitmap(ID_NRChk, 
		wxGetBitmapFromMemory(smooth_off));

	if (m_view && m_view->GetVolMethod()==VOL_METHOD_MULTI)
	{
		for (int i=0; i< m_view->GetAllVolumeNum(); i++)
		{
			fluo::VolumeData* vd = m_view->GetAllVolumeData(i);
			if (vd)
				vd->setValue(gstNoiseRedct, val);
		}
	}
	else
	{
		long lval;
		m_group->getValue(gstBlendMode, lval);
		if (m_sync_group && m_group)
			m_group->setValue(gstNoiseRedct, val);
		else if (m_group && lval==2)
			m_group->setValue(gstNoiseRedct, val);
		else if (m_vd)
			m_vd->setValue(gstNoiseRedct, val);
	}

	RefreshVRenderViews(false, true);
}

void VPropView::OnFluoRender(wxCommandEvent &event)
{
	if (!m_frame) return;
	m_frame->OnInfo(event);
}

//depth mode
void VPropView::OnDepthCheck(wxCommandEvent &event)
{
	bool val = m_options_toolbar->GetToolState(ID_DepthChk);
	if(val) 
		m_options_toolbar->SetToolNormalBitmap(ID_DepthChk, wxGetBitmapFromMemory(depth));
	else
		m_options_toolbar->SetToolNormalBitmap(ID_DepthChk, wxGetBitmapFromMemory(depth_off));

	if (val)
	{
		if (m_group)
		{
			m_group->setValue(gstBlendMode, long(2));
			if (m_vd)
			{
				fluo::ValueCollection names{
					gstNoiseRedct,
					gstSampleRate,
					gstShadowEnable,
					gstShadowInt
				};
				m_group->propValues(names, m_vd);
			}
		}
	}
	else
	{
		if (m_group)
			m_group->setValue(gstBlendMode, long(0));
	}

	RefreshVRenderViews(false, true);
}

bool VPropView::SetSpacings()
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
	bool override_vox = true;
	if (m_frame && m_frame->GetSettingDlg())
		override_vox = m_frame->GetSettingDlg()->GetOverrideVox();

	if ((m_sync_group || override_vox) && m_group)
	{
		for (int i = 0; i < m_group->getNumChildren(); i++)
		{
			fluo::Object* obj = m_group->getChild(i);
			obj->setValue(gstSpcX, spcx);
			obj->setValue(gstSpcY, spcy);
			obj->setValue(gstSpcZ, spcz);
			obj->setValue(gstBaseSpcX, spcx);
			obj->setValue(gstBaseSpcY, spcy);
			obj->setValue(gstBaseSpcZ, spcz);
		}
	}
	else if (m_vd)
	{
		m_vd->setValue(gstSpcX, spcx);
		m_vd->setValue(gstSpcY, spcy);
		m_vd->setValue(gstSpcZ, spcz);
		m_vd->setValue(gstBaseSpcX, spcx);
		m_vd->setValue(gstBaseSpcY, spcy);
		m_vd->setValue(gstBaseSpcZ, spcz);
	}
	else return false;

	return true;
}

//enable/disable
void VPropView::EnableAlpha()
{
	long lval;
	m_vd->getValue(gstMipMode, lval);
	if (lval != 1)
	{
		m_alpha_sldr->Enable();
		m_alpha_text->Enable();
	}
}

void VPropView::DisableAlpha()
{
	m_alpha_sldr->Disable();
	m_alpha_text->Disable();
}

void VPropView::EnableShading()
{
	m_low_shading_sldr->Enable();
	m_low_shading_text->Enable();
	m_hi_shading_sldr->Enable();
	m_hi_shading_text->Enable();
	long lval;
	m_vd->getValue(gstMipMode, lval);
	if (lval == 1)
	{
		m_left_thresh_sldr->Enable();
		m_left_thresh_text->Enable();
		m_right_thresh_sldr->Enable();
		m_right_thresh_text->Enable();
	}
}

void VPropView::DisableShading()
{
	m_low_shading_sldr->Disable();
	m_low_shading_text->Disable();
	m_hi_shading_sldr->Disable();
	m_hi_shading_text->Disable();
	long lval;
	m_vd->getValue(gstMipMode, lval);
	bool bval;
	m_vd->getValue(gstShadowEnable, bval);
	if (lval == 1 && !bval)
	{
		m_left_thresh_sldr->Disable();
		m_left_thresh_text->Disable();
		m_right_thresh_sldr->Disable();
		m_right_thresh_text->Disable();
	}
}

void VPropView::EnableShadow()
{
	m_shadow_sldr->Enable();
	m_shadow_text->Enable();
	long lval;
	m_vd->getValue(gstMipMode, lval);
	if (lval == 1)
	{
		m_left_thresh_sldr->Enable();
		m_left_thresh_text->Enable();
		m_right_thresh_sldr->Enable();
		m_right_thresh_text->Enable();
	}
}

void VPropView::DisableShadow()
{
	m_shadow_sldr->Disable();
	m_shadow_text->Disable();
	long lval;
	m_vd->getValue(gstMipMode, lval);
	bool bval;
	m_vd->getValue(gstShadingEnable, bval);
	if (lval == 1 && !bval)
	{
		m_left_thresh_sldr->Disable();
		m_left_thresh_text->Disable();
		m_right_thresh_sldr->Disable();
		m_right_thresh_text->Disable();
	}
}

void VPropView::EnableColormap()
{
	m_colormap_high_value_sldr->Enable();
	m_colormap_high_value_text->Enable();
	m_colormap_low_value_sldr->Enable();
	m_colormap_low_value_text->Enable();
}

void VPropView::DisableColormap()
{
	m_colormap_high_value_sldr->Disable();
	m_colormap_high_value_text->Disable();
	m_colormap_low_value_sldr->Disable();
	m_colormap_low_value_text->Disable();
}

void VPropView::EnableMip()
{
	DisableAlpha();
	m_boundary_sldr->Disable();
	m_boundary_text->Disable();
	m_luminance_sldr->Disable();
	m_luminance_text->Disable();
	bool bval1, bval2;
	m_vd->getValue(gstShadingEnable, bval1);
	m_vd->getValue(gstShadowEnable, bval2);
	if (bval1 || bval2)
		EnableShading();
	else
		DisableShading();
}

void VPropView::DisableMip()
{
	bool bval1, bval2;
	m_vd->getValue(gstAlphaEnable, bval1);
	if (bval1)
		EnableAlpha();
	else
		DisableAlpha();
	m_boundary_sldr->Enable();
	m_boundary_text->Enable();
	m_luminance_sldr->Enable();
	m_luminance_text->Enable();
	m_vd->getValue(gstShadingEnable, bval1);
	m_vd->getValue(gstShadowEnable, bval2);
	if (bval1 || bval2)
		EnableShading();
	else
		DisableShading();
	m_left_thresh_sldr->Enable();
	m_left_thresh_text->Enable();
	m_right_thresh_sldr->Enable();
	m_right_thresh_text->Enable();
}

//update max value
void VPropView::UpdateMaxVal(double value)
{
	if (!m_vd) return;
	long bits;
	m_vd->getValue(gstBits, bits);
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
	m_vd->setValue(gstMaxInt, m_max_val);
	m_vd->setValue(gstIntScale, 65535.0 / m_max_val);
	GetSettings();
}

void VPropView::OnSpaceText(wxCommandEvent& event)
{
	if (SetSpacings())
		InitVRenderViews(INIT_BOUNDS|INIT_CENTER);
}

//legend
void VPropView::OnLegendCheck(wxCommandEvent& event)
{
	bool leg = m_options_toolbar->GetToolState(ID_LegendChk);
	if (m_vd)
		m_vd->setValue("legend", leg);

	RefreshVRenderViews(false, true);
}

//interpolation
void VPropView::OnInterpolateCheck(wxCommandEvent& event)
{
	bool inv = m_options_toolbar->GetToolState(ID_InterpolateChk);
	if(inv) 
		m_options_toolbar->SetToolNormalBitmap(ID_InterpolateChk, 
		wxGetBitmapFromMemory(interpolate));
	else
		m_options_toolbar->SetToolNormalBitmap(ID_InterpolateChk, 
		wxGetBitmapFromMemory(interpolate_off));
	if (m_sync_group && m_group)
		m_group->setValue("interpolate", inv);
	else if (m_vd)
		m_vd->setValue("interpolate", inv);
	if (m_view)
		m_view->SetIntp(inv);

	RefreshVRenderViews(false, true);
}

//sync within group
void VPropView::OnSyncGroupCheck(wxCommandEvent& event)
{
	m_sync_group = m_options_toolbar->GetToolState(ID_SyncGroupChk);
	if (m_group)
		m_group->setValue("sync group", m_sync_group);

	if (m_sync_group && m_group)
	{
		wxString str;
		double dVal;
		long iVal;
		bool bVal;

		//gamma
		str = m_gamma_text->GetValue();
		str.ToDouble(&dVal);
		m_group->setValue(gstGamma3d, dVal);
		//boundary
		str = m_boundary_text->GetValue();
		str.ToDouble(&dVal);
		m_group->setValue(gstExtractBoundary, dVal);
		//saturation
		str = m_saturation_text->GetValue();
		str.ToLong(&iVal);
		dVal = double(iVal) / m_max_val;
		m_group->setValue(gstSaturation, dVal);
		//left threshold
		str = m_left_thresh_text->GetValue();
		str.ToLong(&iVal);
		dVal = double(iVal) / m_max_val;
		m_group->setValue(gstLowThreshold, dVal);
		//right thresh
		str = m_right_thresh_text->GetValue();
		str.ToLong(&iVal);
		dVal = double(iVal) / m_max_val;
		m_group->setValue(gstHighThreshold, dVal);
		//luminance
		//str = m_luminance_text->GetValue();
		//str.ToLong(&iVal);
		//dVal = double(iVal)/m_max_val;
		//m_group->SetLuminance(dVal);
		//shadow
		bVal = m_shadow_tool->GetToolState(ID_ShadowChk);
		m_group->setValue(gstShadowEnable, bVal);
		str = m_shadow_text->GetValue();
		str.ToDouble(&dVal);
		m_group->setValue(gstShadowInt, dVal);
		//high shading
		str = m_hi_shading_text->GetValue();
		str.ToDouble(&dVal);
		m_group->setValue(gstHighShading, dVal);
		//alpha
		str = m_alpha_text->GetValue();
		str.ToLong(&iVal);
		dVal = double(iVal) / m_max_val;
		m_group->setValue(gstAlpha, dVal);
		//sample rate
		str = m_sample_text->GetValue();
		str.ToDouble(&dVal);
		m_group->setValue(gstSampleRate, dVal);
		//shading
		bVal = m_shade_tool->GetToolState(ID_ShadingEnableChk);
		m_group->setValue(gstShadingEnable, bVal);
		str = m_low_shading_text->GetValue();
		str.ToDouble(&dVal);
		m_group->setValue(gstLowShading, dVal);
		//inversion
		bVal = m_options_toolbar->GetToolState(ID_InvChk);
		m_group->setValue(gstInvert, bVal);
		//interpolation
		bVal = m_options_toolbar->GetToolState(ID_InterpolateChk);
		m_group->setValue("interpolate", bVal);
		if (m_view)
			m_view->SetIntp(bVal);
		//MIP
		bVal = m_options_toolbar->GetToolState(ID_MipChk);
		m_group->setValue(gstMipMode, long(bVal?1:0));
		//transp
		bVal = m_options_toolbar->GetToolState(ID_TranspChk);
		m_group->setValue(gstAlphaPower, bVal ? 2.0 : 1.0);
		//noise reduction
		bVal = m_options_toolbar->GetToolState(ID_InvChk);
		m_group->setValue(gstNoiseRedct, bVal);
		//colormap low
		str = m_colormap_low_value_text->GetValue();
		str.ToLong(&iVal);
		dVal = double(iVal) / m_max_val;
		m_group->setValue(gstColormapLow, dVal);
		//colormap high
		str = m_colormap_high_value_text->GetValue();
		str.ToLong(&iVal);
		dVal = double(iVal) / m_max_val;
		m_group->setValue(gstColormapHigh, dVal);
		//colormap mode
		bVal = m_colormap_tool->GetToolState(ID_ColormapEnableChk);
		m_group->setValue(gstColormapMode, long(bVal?1:0));
		//colormap
		iVal = m_colormap_combo->GetCurrentSelection();
		m_group->setValue(gstColormapType, iVal);
		//colormap inv
		bVal = m_colormap_inv_btn->GetValue();
		m_group->setValue(gstColormapInv, bVal ? -1.0 : 1.0);
		//colormap proj
		iVal = m_colormap_combo2->GetCurrentSelection();
		m_group->setValue(gstColormapProj, iVal);
	}

	RefreshVRenderViews(false, true);
}

void VPropView::OnSaveDefault(wxCommandEvent& event)
{
	if (!m_frame)
		return;
	DataManager *mgr = m_frame->GetDataManager();
	if (!mgr)
		return;

	wxString app_name = "FluoRender " +
		wxString::Format("%d.%.1f", VERSION_MAJOR, float(VERSION_MINOR));
	wxString vendor_name = "FluoRender";
	wxString local_name = "default_volume_settings.dft";
	wxFileConfig fconfig(app_name, vendor_name, local_name, "",
		wxCONFIG_USE_LOCAL_FILE);
	wxString str;
	double val;
	int ival;
	//extract boundary
	str = m_boundary_text->GetValue();
	str.ToDouble(&val);
	fconfig.Write("extract_boundary", val);
	mgr->m_vol_exb = val;
	//gamma
	str = m_gamma_text->GetValue();
	str.ToDouble(&val);
	fconfig.Write("gamma", val);
	mgr->m_vol_gam = val;
	//low offset
	str = m_saturation_text->GetValue();
	str.ToDouble(&val);
	val /= m_max_val;
	fconfig.Write("low_offset", val);
	mgr->m_vol_of1 = val;
	//high offset
	val = 1.0;
	fconfig.Write("high_offset", val);
	mgr->m_vol_of2 = val;
	//low thresholding
	str = m_left_thresh_text->GetValue();
	str.ToDouble(&val);
	val /= m_max_val;
	fconfig.Write("low_thresholding", val);
	mgr->m_vol_lth = val;
	//high thresholding
	str = m_right_thresh_text->GetValue();
	str.ToDouble(&val);
	val /= m_max_val;
	fconfig.Write("high_thresholding", val);
	mgr->m_vol_hth = val;
	//low shading
	str = m_low_shading_text->GetValue();
	str.ToDouble(&val);
	fconfig.Write("low_shading", val);
	mgr->m_vol_lsh = val;
	//high shading
	str = m_hi_shading_text->GetValue();
	str.ToDouble(&val);
	fconfig.Write("high_shading", val);
	mgr->m_vol_hsh = val;
	//alpha
	str = m_alpha_text->GetValue();
	str.ToDouble(&val);
	val /= m_max_val;
	fconfig.Write("alpha", val);
	mgr->m_vol_alf = val;
	//sample rate
	str = m_sample_text->GetValue();
	str.ToDouble(&val);
	fconfig.Write("sample_rate", val);
	mgr->m_vol_spr = val;
	//x spacing
	str = m_space_x_text->GetValue();
	str.ToDouble(&val);
	fconfig.Write("x_spacing", val);
	mgr->m_vol_xsp = val;
	//y spacing
	str = m_space_y_text->GetValue();
	str.ToDouble(&val);
	fconfig.Write("y_spacing", val);
	mgr->m_vol_ysp = val;
	//z spacing
	str = m_space_z_text->GetValue();
	str.ToDouble(&val);
	fconfig.Write("z_spacing", val);
	mgr->m_vol_zsp = val;
	//luminance
	str = m_luminance_text->GetValue();
	str.ToDouble(&val);
	val /= m_max_val;
	fconfig.Write("luminance", val);
	mgr->m_vol_lum = val;
	//colormap enable
	bool bval = m_colormap_tool->GetToolState(ID_ColormapEnableChk);
	fconfig.Write("colormap_mode", bval);
	mgr->m_vol_cmm = bval;
	//colormap inv
	bval = m_colormap_inv_btn->GetValue();
	fconfig.Write("colormap_inv", bval);
	mgr->m_vol_cmi = bval;
	//colormap type
	ival = m_colormap_combo->GetCurrentSelection();
	fconfig.Write("colormap", ival);
	mgr->m_vol_cmp = ival;
	//colormap projection
	ival = m_colormap_combo2->GetCurrentSelection();
	fconfig.Write("colormap_proj", ival);
	mgr->m_vol_cmj = ival;
	//colormap low value
	str = m_colormap_low_value_text->GetValue();
	str.ToDouble(&val);
	val /= m_max_val;
	fconfig.Write("colormap_low", val);
	mgr->m_vol_lcm = val;
	//colormap high value
	str = m_colormap_high_value_text->GetValue();
	str.ToDouble(&val);
	val /= m_max_val;
	fconfig.Write("colormap_hi", val);
	mgr->m_vol_hcm = val;
	//alpha
	bool alpha = m_alpha_tool->GetToolState(ID_AlphaChk);
	fconfig.Write("enable_alpha", alpha);
	mgr->m_vol_eap = alpha;
	//enable shading
	bool shading = m_shade_tool->GetToolState(ID_ShadingEnableChk);
	fconfig.Write("enable_shading", shading);
	mgr->m_vol_esh = shading;
	//inversion
	bool interp = m_options_toolbar->GetToolState(ID_InterpolateChk);
	fconfig.Write("enable_interp", interp);
	mgr->m_vol_interp = interp;
	//inversion
	bool inv = m_options_toolbar->GetToolState(ID_InvChk);
	fconfig.Write("enable_inv", inv);
	mgr->m_vol_inv = inv;
	//enable mip
	bool mip = m_options_toolbar->GetToolState(ID_MipChk);
	fconfig.Write("enable_mip", mip);
	mgr->m_vol_mip = mip;
	//enable hi transp
	bool trp = m_options_toolbar->GetToolState(ID_TranspChk);
	fconfig.Write("enable_trp", trp);
	mgr->m_vol_trp = trp;
	//enable component display
	int comp = m_options_toolbar->GetToolState(ID_CompChk);
	fconfig.Write("enable_comp", comp);
	mgr->m_vol_com = comp;
	//noise reduction
	bool nrd = m_options_toolbar->GetToolState(ID_NRChk);
	fconfig.Write("noise_rd", nrd);
	mgr->m_vol_nrd = nrd;
	//shadow
	bool shw = m_shadow_tool->GetToolState(ID_ShadowChk);
	fconfig.Write("enable_shadow", shw);
	mgr->m_vol_shw = shw;
	//shadow intensity
	str = m_shadow_text->GetValue();
	str.ToDouble(&val);
	double swi = val;
	fconfig.Write("shadow_intensity", swi);
	mgr->m_vol_swi = swi;
	wxString expath = wxStandardPaths::Get().GetExecutablePath();
	expath = wxPathOnly(expath);
	wxString dft = expath + GETSLASH() + "default_volume_settings.dft";
	SaveConfig(fconfig, dft);
}

void VPropView::OnResetDefault(wxCommandEvent &event)
{
	if (!m_frame)
		return;
	DataManager *mgr = m_frame->GetDataManager();
	if (!mgr)
		return;
	if (!m_vd)
		return;

	wxString str;
	double dval;
	long ival;
	bool bval;

	//gamma
	dval = mgr->m_vol_gam;
	str = wxString::Format("%.2f", dval);
	m_gamma_text->ChangeValue(str);
	ival = int(dval*100.0+0.5);
	m_gamma_sldr->SetValue(ival);
	m_vd->setValue(gstGamma3d, dval);
	//extract boundary
	dval = mgr->m_vol_exb;
	str = wxString::Format("%.4f", dval);
	m_boundary_text->ChangeValue(str);
	ival = int(dval*2000.0+0.5);
	m_boundary_sldr->SetValue(ival);
	m_vd->setValue(gstExtractBoundary, dval);
	//low offset
	dval = mgr->m_vol_of1;
	ival = int(dval*m_max_val+0.5);
	str = wxString::Format("%d", ival);
	m_saturation_text->ChangeValue(str);
	m_saturation_sldr->SetValue(ival);
	m_vd->setValue(gstSaturation, dval);
	//low thresholding
	dval = mgr->m_vol_lth;
	ival = int(dval*m_max_val+0.5);
	str = wxString::Format("%d", ival);
	m_left_thresh_text->ChangeValue(str);
	m_left_thresh_sldr->SetValue(ival);
	m_vd->setValue(gstLowThreshold, dval);
	//high thresholding
	dval = mgr->m_vol_hth;
	ival = int(dval*m_max_val+0.5);
	str = wxString::Format("%d", ival);
	m_right_thresh_text->ChangeValue(str);
	m_right_thresh_sldr->SetValue(ival);
	m_vd->setValue(gstHighThreshold, dval);
	//low shading
	dval = mgr->m_vol_lsh;
	str = wxString::Format("%.2f", dval);
	m_low_shading_text->ChangeValue(str);
	ival = int(dval*100.0+0.5);
	m_low_shading_sldr->SetValue(ival);
	m_vd->setValue(gstMatAmb, dval);
	//high shading
	dval = mgr->m_vol_hsh;
	str = wxString::Format("%.2f", dval);
	m_hi_shading_text->ChangeValue(str);
	ival = int(dval*10.0+0.5);
	m_hi_shading_sldr->SetValue(ival);
	m_vd->setValue(gstMatShine, dval);
	//alpha
	dval = mgr->m_vol_alf;
	ival = int(dval*m_max_val+0.5);
	str = wxString::Format("%d", ival);
	m_alpha_text->ChangeValue(str);
	m_alpha_sldr->SetValue(ival);
	m_vd->setValue(gstAlpha, dval);
	//sample rate
	dval = mgr->m_vol_spr;
	str = wxString::Format("%.1f", dval);
	m_sample_text->ChangeValue(str);
	ival = int(dval*10.0+0.5);
	m_sample_sldr->SetValue(ival);
	m_vd->setValue(gstSampleRate, dval);
	//luminance
	dval = mgr->m_vol_lum;
	ival = int(dval*m_max_val+0.5);
	str = wxString::Format("%d", ival);
	m_luminance_text->ChangeValue(str);
	m_luminance_sldr->SetValue(ival);
	fluo::HSVColor hsv;
	m_vd->getValue(gstHsv, hsv);
	fluo::Color color(hsv);
	m_vd->setValue(gstSecColorSet, false);
	m_vd->setValue(gstColor, color);
	wxColor wxc((unsigned char)(color.r()*255),
		(unsigned char)(color.g()*255),
		(unsigned char)(color.b()*255));
	m_color_text->ChangeValue(wxString::Format("%d , %d , %d",
		wxc.Red(), wxc.Green(), wxc.Blue()));
	m_color_btn->SetColour(wxc);
	m_vd->getValue(gstSecColor, color);
	wxc = wxColor((unsigned char)(color.r()*255),
		(unsigned char)(color.g()*255),
		(unsigned char)(color.b()*255));
	m_color2_text->ChangeValue(wxString::Format("%d , %d , %d",
		wxc.Red(), wxc.Green(), wxc.Blue()));
	m_color2_btn->SetColour(wxc);
	//colormap mode
	m_vd->setValue(gstColormapMode, long(mgr->m_vol_cmm));
	bool colormap = mgr->m_vol_cmm == 1;
	m_colormap_tool->ToggleTool(ID_ColormapEnableChk, colormap);
	//colormap inv
	m_colormap_inv_btn->SetValue(mgr->m_vol_cmi);
	m_vd->setValue(gstColormapInv, mgr->m_vol_cmi?-1.0:1.0);
	//colormap
	m_colormap_combo->SetSelection(mgr->m_vol_cmp);
	m_vd->setValue(gstColormapType, long(mgr->m_vol_cmp));
	//colormap projection
	m_colormap_combo2->SetSelection(mgr->m_vol_cmj);
	m_vd->setValue(gstColormapProj, long(mgr->m_vol_cmj));
	//colormap low value
	dval = mgr->m_vol_lcm;
	ival = int(dval*m_max_val+0.5);
	str = wxString::Format("%d", ival);
	m_colormap_low_value_text->ChangeValue(str);
	m_colormap_low_value_sldr->SetValue(ival);
	double lcm = dval;
	dval = mgr->m_vol_hcm;
	ival = int(dval*m_max_val+0.5);
	str = wxString::Format("%d", ival);
	m_colormap_high_value_text->ChangeValue(str);
	m_colormap_high_value_sldr->SetValue(ival);
	m_vd->setValue(gstColormapLow, lcm);
	m_vd->setValue(gstColormapHigh, dval);
	//shadow intensity
	dval = mgr->m_vol_swi;
	str = wxString::Format("%.2f", dval);
	ival = int(dval*100.0+0.5);
	m_shadow_text->ChangeValue(str);
	m_shadow_sldr->SetValue(ival);
	m_vd->setValue(gstShadowInt, dval);

	//enable alpha
	bval = mgr->m_vol_eap;
	m_alpha_tool->ToggleTool(ID_AlphaChk,bval);
	if (m_sync_group && m_group)
		m_group->setValue(gstAlphaEnable, bval);
	else
		m_vd->setValue(gstAlphaEnable, bval);
	//enable shading
	bval = mgr->m_vol_esh;
	m_shade_tool->ToggleTool(ID_ShadingEnableChk,bval);
	if (m_sync_group && m_group)
		m_group->setValue(gstShadingEnable, bval);
	else
		m_vd->setValue(gstShadingEnable, bval);
	//inversion
	bval = mgr->m_vol_inv;
	m_options_toolbar->ToggleTool(ID_InvChk,bval);
	if (m_sync_group && m_group)
		m_group->setValue(gstInvert, bval);
	else
		m_vd->setValue(gstInvert, bval);
	//enable mip
	bval = mgr->m_vol_mip;
	m_options_toolbar->ToggleTool(ID_MipChk,bval);
	if (m_sync_group && m_group)
		m_group->setValue(gstMipMode, long(bval?1:0));
	else
		m_vd->setValue(gstMipMode, long(bval ? 1 : 0));
	//enable transp
	bval = mgr->m_vol_trp;
	m_options_toolbar->ToggleTool(ID_TranspChk, bval);
	if (m_sync_group && m_group)
		m_group->setValue(gstAlphaPower, bval ? 2.0 : 1.0);
	else
		m_vd->setValue(gstAlphaPower, bval ? 2.0 : 1.0);
	//component display
	ival = mgr->m_vol_com;
	m_options_toolbar->ToggleTool(ID_CompChk, ival?true:false);
	if (m_sync_group && m_group)
		m_group->setValue("label mode", ival);
	else
		m_vd->setValue("label mode", ival);
	//noise reduction
	bval = mgr->m_vol_nrd;
	m_options_toolbar->ToggleTool(ID_NRChk,bval);
	if (m_sync_group && m_group)
		m_group->setValue(gstNoiseRedct, bval);
	else
		m_vd->setValue(gstNoiseRedct, bval);
	//shadow
	bval = mgr->m_vol_shw;
	m_shadow_tool->ToggleTool(ID_ShadowChk,bval);
	if (m_sync_group && m_group)
		m_group->setValue(gstShadowEnable, bval);
	else
		m_vd->setValue(gstShadowEnable, bval);
	//colormap
	if (m_sync_group && m_group)
		m_group->setValue(gstColormapMode, long(mgr->m_vol_cmm));

	m_vd->getValue(gstAlphaEnable, bval);
	if (bval)
		EnableAlpha();
	else
		DisableAlpha();
	m_vd->getValue(gstShadingEnable, bval);
	if (bval)
		EnableShading();
	else
		DisableShading();
	m_vd->getValue(gstShadowEnable, bval);
	if (bval)
		EnableShadow();
	else
		DisableShadow();
	m_vd->getValue(gstColormapMode, ival);
	if (ival == 1)
		EnableColormap();
	else
		DisableColormap();
	m_vd->getValue(gstMipMode, ival);
	if (ival == 1)
		EnableMip();
	else
		DisableMip();

	//apply all
	RefreshVRenderViews(false, true);
}
