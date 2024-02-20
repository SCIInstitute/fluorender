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
#include <DataManager.h>
#include "VRenderFrame.h"
#include <Global/Global.h>
#include <Calculate/Histogram.h>
#include <Database/RecordHistParams.h>
#include <FLIVR/MultiVolumeRenderer.h>
#include <FLIVR/VolumeRenderer.h>
#include <FLIVR/VolShaderCode.h>
#include <Types/Color.h>
#include <Types/BBox.h>
#include <Types/Point.h>
#include <wxDoubleSlider.h>
#include <wxSingleSlider.h>
#include "png_resource.h"
#include <wx/wfstream.h>
#include <wx/fileconf.h>
#include <wx/aboutdlg.h>
#include <wx/colordlg.h>
#include <wx/valnum.h>
#include <wx/hyperlink.h>
#include <wx/stdpaths.h>
#include "img/icons.h"
#include <limits>

BEGIN_EVENT_TABLE(VPropView, wxPanel)
	//1
	EVT_BUTTON(ID_GammaSync, VPropView::OnGammaSync)
	EVT_COMMAND_SCROLL(ID_GammaSldr, VPropView::OnGammaChange)
	EVT_TEXT(ID_GammaText, VPropView::OnGammaText)
	EVT_CHECKBOX(ID_GammaChk, VPropView::OnGammaChk)
	//
	EVT_BUTTON(ID_SaturationSync, VPropView::OnSaturationSync)
	EVT_COMMAND_SCROLL(ID_SaturationSldr, VPropView::OnSaturationChange)
	EVT_TEXT(ID_SaturationText, VPropView::OnSaturationText)
	EVT_CHECKBOX(ID_SaturationChk, VPropView::OnSaturationChk)
	//
	EVT_BUTTON(ID_LuminanceSync, VPropView::OnLuminanceSync)
	EVT_COMMAND_SCROLL(ID_LuminanceSldr, VPropView::OnLuminanceChange)
	EVT_TEXT(ID_LuminanceText, VPropView::OnLuminanceText)
	EVT_CHECKBOX(ID_LuminanceChk, VPropView::OnLuminanceChk)
	//
	EVT_BUTTON(ID_AlphaSync, VPropView::OnAlphaSync)
	EVT_COMMAND_SCROLL(ID_AlphaSldr, VPropView::OnAlphaChange)
	EVT_TEXT(ID_Alpha_Text, VPropView::OnAlphaText)
	EVT_CHECKBOX(ID_AlphaChk, VPropView::OnAlphaCheck)
	//
	EVT_BUTTON(ID_ShadingSync, VPropView::OnShadingSync)
	EVT_COMMAND_SCROLL(ID_HiShadingSldr, VPropView::OnHiShadingChange)
	EVT_TEXT(ID_HiShadingText, VPropView::OnHiShadingText)
	EVT_COMMAND_SCROLL(ID_LowShadingSldr, VPropView::OnLowShadingChange)
	EVT_TEXT(ID_LowShadingText, VPropView::OnLowShadingText)
	EVT_CHECKBOX(ID_ShadingChk, VPropView::OnShadingChk)

	//2
	EVT_BUTTON(ID_BoundarySync, VPropView::OnBoundarySync)
	EVT_COMMAND_SCROLL(ID_BoundarySldr, VPropView::OnBoundaryChange)
	EVT_TEXT(ID_BoundaryText, VPropView::OnBoundaryText)
	EVT_CHECKBOX(ID_BoundaryChk, VPropView::OnBoundaryChk)
	//
	EVT_BUTTON(ID_ThreshSync, VPropView::OnThreshSync)
	EVT_COMMAND_SCROLL(ID_ThreshSldr, VPropView::OnThreshChange)
	EVT_TEXT(ID_ThreshLowText, VPropView::OnThreshLowText)
	EVT_TEXT(ID_TreshHiText, VPropView::OnThreshHiText)
	EVT_TOOL(ID_ThreshLinkTb, VPropView::OnThreshLink)
	EVT_CHECKBOX(ID_ThreshChk, VPropView::OnThreshChk)
	//
	EVT_BUTTON(ID_ShadowSync, VPropView::OnShadowSync)
	EVT_COMMAND_SCROLL(ID_ShadowSldr, VPropView::OnShadowChange)
	EVT_TEXT(ID_ShadowText, VPropView::OnShadowText)
	EVT_CHECKBOX(ID_ShadowChk, VPropView::OnShadowChk)
	//
	EVT_BUTTON(ID_SampleSync, VPropView::OnSampleSync)
	EVT_COMMAND_SCROLL(ID_SampleSldr, VPropView::OnSampleChange)
	EVT_TEXT(ID_SampleText, VPropView::OnSampleText)
	EVT_CHECKBOX(ID_SampleChk, VPropView::OnSampleChk)
	//
	EVT_BUTTON(ID_ColormapSync, VPropView::OnColormapSync)
	EVT_COMMAND_SCROLL(ID_ColormapSldr, VPropView::OnColormapChange)
	EVT_TEXT(ID_ColormapHiText, VPropView::OnColormapHiText)
	EVT_TEXT(ID_ColormapLowText, VPropView::OnColormapLowText)
	EVT_TOOL(ID_ColormapLinkTb, VPropView::OnColormapLink)
	EVT_CHECKBOX(ID_ColormapChk, VPropView::OnColormapChk)

	//3
	EVT_TOGGLEBUTTON(ID_ColormapInvBtn, VPropView::OnColormapInvBtn)
	EVT_COMBOBOX(ID_ColormapCombo, VPropView::OnColormapCombo)
	EVT_COMBOBOX(ID_ColormapCombo2, VPropView::OnColormapCombo2)
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
	//ml
	EVT_TOOL(ID_UseMlChk, VPropView::OnUseMlChk)
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
	SetDoubleBuffered(true);

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

	wxBitmap bitmap;
	//left///////////////////////////////////////////////////
	//gamma
	m_gamma_st = new wxButton(this, ID_GammaSync, ": Gamma",
		wxDefaultPosition, FromDIP(wxSize(100, -1)), wxBU_LEFT);
	m_gamma_sldr = new wxSingleSlider(this, ID_GammaSldr, 100, 10, 400,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL|wxSL_INVERSE);
	m_gamma_sldr->SetRangeStyle(1);
	m_gamma_text = new wxTextCtrl(this, ID_GammaText, "1.00",
		wxDefaultPosition, FromDIP(wxSize(40, 20)), 0, vald_fp2);
	m_gamma_chk = new wxCheckBox(this, ID_GammaChk, "");
	sizer_l1->Add(m_gamma_sldr, 1, wxEXPAND);
	sizer_l1->Add(m_gamma_text, 0, wxALIGN_CENTER);
	sizer_l1->Add(m_gamma_chk, 0, wxALIGN_CENTER);
	sizer_l1->Add(m_gamma_st, 0, wxALIGN_CENTER);
	//saturation point
	m_saturation_st = new wxButton(this, ID_SaturationSync, ": Saturation",
		wxDefaultPosition, FromDIP(wxSize(100, -1)), wxBU_LEFT);
	m_saturation_sldr = new wxSingleSlider(this, ID_SaturationSldr, 255, 0, 255,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_saturation_text = new wxTextCtrl(this, ID_SaturationText, "50",
		wxDefaultPosition, FromDIP(wxSize(40, 20))/*, 0, vald_int*/);
	m_saturation_chk = new wxCheckBox(this, ID_SaturationChk, "");
	sizer_l2->Add(m_saturation_sldr, 1, wxEXPAND);
	sizer_l2->Add(m_saturation_text, 0, wxALIGN_CENTER);
	sizer_l2->Add(m_saturation_chk, 0, wxALIGN_CENTER);
	sizer_l2->Add(m_saturation_st, 0, wxALIGN_CENTER);
	//luminance
	m_luminance_st = new wxButton(this, ID_LuminanceSync, ": Luminance",
		wxDefaultPosition, FromDIP(wxSize(100, -1)), wxBU_LEFT);
	m_luminance_sldr = new wxSingleSlider(this, ID_LuminanceSldr, 128, 0, 255,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_luminance_text = new wxTextCtrl(this, ID_LuminanceText, "128",
		wxDefaultPosition, FromDIP(wxSize(40, 20))/*, 0, vald_int*/);
	m_luminance_chk = new wxCheckBox(this, ID_LuminanceChk, "");
	sizer_l3->Add(m_luminance_sldr, 1, wxEXPAND, 0);
	sizer_l3->Add(m_luminance_text, 0, wxALIGN_CENTER, 0);
	sizer_l3->Add(m_luminance_chk, 0, wxALIGN_CENTER);
	sizer_l3->Add(m_luminance_st, 0, wxALIGN_CENTER, 0);
	//alpha
	m_alpha_st = new wxButton(this, ID_AlphaSync, ": Alpha",
		wxDefaultPosition, FromDIP(wxSize(100, -1)), wxBU_LEFT);
	m_alpha_sldr = new wxSingleSlider(this, ID_AlphaSldr, 127, 0, 255,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_alpha_text = new wxTextCtrl(this, ID_Alpha_Text, "127",
		wxDefaultPosition, FromDIP(wxSize(40, 20)), 0, vald_int);
	m_alpha_chk = new wxCheckBox(this, ID_AlphaChk, "");
	sizer_l4->Add(m_alpha_sldr, 1, wxEXPAND);
	sizer_l4->Add(m_alpha_text, 0, wxALIGN_CENTER);
	sizer_l4->Add(m_alpha_chk, 0, wxALIGN_CENTER);
	sizer_l4->Add(m_alpha_st, 0, wxALIGN_CENTER);
	//highlight
	m_shade_st = new wxButton(this, ID_ShadingSync, ": Shading",
		wxDefaultPosition, FromDIP(wxSize(100, -1)), wxBU_LEFT);
	m_hi_shading_sldr = new wxSingleSlider(this, ID_HiShadingSldr, 0, 0, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_hi_shading_text = new wxTextCtrl(this, ID_HiShadingText, "0.00",
		wxDefaultPosition, FromDIP(wxSize(50, 20)), 0, vald_fp2);
	//shading
	m_low_shading_sldr = new wxSingleSlider(this, ID_LowShadingSldr, 0, 0, 200,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_low_shading_text = new wxTextCtrl(this, ID_LowShadingText, "0.00",
		wxDefaultPosition, FromDIP(wxSize(40, 20)), 0, vald_fp2);
	m_shade_chk = new wxCheckBox(this, ID_ShadingChk, "");
	sizer_l5->Add(m_hi_shading_sldr, 1, wxEXPAND);
	sizer_l5->Add(m_hi_shading_text, 0, wxALIGN_CENTER);
	sizer_l5->Add(m_low_shading_sldr, 1, wxEXPAND);
	sizer_l5->Add(m_low_shading_text, 0, wxALIGN_CENTER);
	sizer_l5->Add(m_shade_chk, 0, wxALIGN_CENTER);
	sizer_l5->Add(m_shade_st, 0, wxALIGN_CENTER);

	//middle///////////////////////////////////////////////////
	//extract boundary
	m_boundary_st = new wxButton(this, ID_BoundarySync, "Boundary :",
		wxDefaultPosition, FromDIP(wxSize(100, -1)), wxBU_RIGHT);
	m_boundary_sldr = new wxSingleSlider(this, ID_BoundarySldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_boundary_text = new wxTextCtrl(this, ID_BoundaryText, "0.0000",
		wxDefaultPosition, FromDIP(wxSize(50, 20)), 0, vald_fp4);
	m_boundary_chk = new wxCheckBox(this, ID_BoundaryChk, "");
	sizer_m1->Add(m_boundary_st, 0, wxALIGN_CENTER);
	sizer_m1->Add(m_boundary_chk, 0, wxALIGN_CENTER);
	sizer_m1->Add(m_boundary_text, 0, wxALIGN_CENTER);
	sizer_m1->Add(m_boundary_sldr, 1, wxEXPAND);
	//thresholds
	m_thresh_st = new wxButton(this, ID_ThreshSync, "Threshold :",
		wxDefaultPosition, FromDIP(wxSize(100, -1)), wxBU_RIGHT);
	m_thresh_sldr = new wxDoubleSlider(this, ID_ThreshSldr, 0, 255, 0, 255,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_left_thresh_text = new wxTextCtrl(this, ID_ThreshLowText, "0",
		wxDefaultPosition, FromDIP(wxSize(50, 20)), 0, vald_int);
	m_right_thresh_text = new wxTextCtrl(this, ID_TreshHiText, "255",
		wxDefaultPosition, FromDIP(wxSize(50, 20)), 0, vald_int);
	m_thresh_link_tb = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	m_thresh_link_tb->AddCheckTool(ID_ThreshLinkTb, "Lock Threshold Range",
		wxGetBitmapFromMemory(unlink), wxNullBitmap, "Lock Threshold Range");
	m_thresh_link_tb->SetToolBitmapSize(wxSize(20, 20));
	m_thresh_chk = new wxCheckBox(this, ID_ThreshChk, "");
	sizer_m2->Add(m_thresh_st, 0, wxALIGN_CENTER);
	sizer_m2->Add(m_thresh_chk, 0, wxALIGN_CENTER);
	sizer_m2->Add(m_left_thresh_text, 0, wxALIGN_CENTER);
	sizer_m2->Add(m_thresh_sldr, 1, wxEXPAND);
	sizer_m2->Add(m_right_thresh_text, 0, wxALIGN_CENTER);
	sizer_m2->Add(m_thresh_link_tb, 0, wxALIGN_CENTER, 0);
	m_thresh_link_tb->Realize();
	//shadow
	m_shadow_st = new wxButton(this, ID_ShadowSync, "Shadow :",
		wxDefaultPosition, FromDIP(wxSize(100, -1)), wxBU_RIGHT);
	m_shadow_sldr = new wxSingleSlider(this, ID_ShadowSldr, 0, 0, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_shadow_text = new wxTextCtrl(this, ID_ShadowText, "0.00",
		wxDefaultPosition, FromDIP(wxSize(50, 20)), 0, vald_fp2);
	m_shadow_chk = new wxCheckBox(this, ID_ShadowChk, "");
	sizer_m3->Add(m_shadow_st, 0, wxALIGN_CENTER);
	sizer_m3->Add(m_shadow_chk, 0, wxALIGN_CENTER);
	sizer_m3->Add(m_shadow_text, 0, wxALIGN_CENTER);
	sizer_m3->Add(m_shadow_sldr, 1, wxEXPAND);
	//sample rate
	m_sample_st = new wxButton(this, ID_SampleSync, "Sample Rate :",
		wxDefaultPosition, FromDIP(wxSize(100, -1)), wxBU_RIGHT);
	m_sample_sldr = new wxSingleSlider(this, ID_SampleSldr, 10, 1, 50,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_sample_text = new wxTextCtrl(this, ID_SampleText, "1.0",
		wxDefaultPosition, FromDIP(wxSize(50, 20)), 0, vald_fp2);
	m_sample_chk = new wxCheckBox(this, ID_SampleChk, "");
	sizer_m4->Add(m_sample_st, 0, wxALIGN_CENTER);
	sizer_m4->Add(m_sample_chk, 0, wxALIGN_CENTER);
	sizer_m4->Add(m_sample_text, 0, wxALIGN_CENTER);
	sizer_m4->Add(m_sample_sldr, 1, wxEXPAND);
	//colormap
	m_colormap_st = new wxButton(this, ID_ColormapSync, "Colormap :",
		wxDefaultPosition, FromDIP(wxSize(100, -1)), wxBU_RIGHT);
	m_colormap_low_text = new wxTextCtrl(this,
		ID_ColormapLowText, "0",
		wxDefaultPosition, FromDIP(wxSize(50, 20)), 0, vald_int);
	m_colormap_sldr = new wxDoubleSlider(this,
		ID_ColormapSldr, 0, 255, 0, 255,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_colormap_hi_text = new wxTextCtrl(this,
		ID_ColormapHiText, "255",
		wxDefaultPosition, FromDIP(wxSize(50, 20)), 0, vald_int);
	m_colormap_link_tb = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	m_colormap_link_tb->AddCheckTool(ID_ColormapLinkTb, "Lock Colormap Range",
		wxGetBitmapFromMemory(unlink), wxNullBitmap, "Lock Colormap Range");
	m_colormap_link_tb->SetToolBitmapSize(wxSize(20, 20));
	m_colormap_chk = new wxCheckBox(this, ID_ColormapChk, "");
	sizer_m5->Add(m_colormap_st, 0, wxALIGN_CENTER);
	sizer_m5->Add(m_colormap_chk, 0, wxALIGN_CENTER);
	sizer_m5->Add(m_colormap_low_text, 0, wxALIGN_CENTER);
	sizer_m5->Add(m_colormap_sldr, 1, wxEXPAND);
	sizer_m5->Add(m_colormap_hi_text, 0, wxALIGN_CENTER);
	sizer_m5->Add(m_colormap_link_tb, 0, wxALIGN_CENTER, 0);
	m_colormap_link_tb->Realize();

	//right ///////////////////////////////////////////////////
	m_options_toolbar = new wxToolBar(this,wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	double dpi_sf = GetDPIScaleFactor();
	double dpi_sf2 = std::round(dpi_sf - 0.1);
	dpi_sf = dpi_sf2 < dpi_sf ? dpi_sf : 1;
	//ml
	bitmap = wxGetBitmap(starknot, dpi_sf);
#ifdef _DARWIN
	m_options_toolbar->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_options_toolbar->AddTool(ID_UseMlChk, "Use Machine Learning",
		bitmap, "Generate properties using machine learning");
	//transparency
	bitmap = wxGetBitmap(transplo, dpi_sf);
	m_options_toolbar->AddCheckTool(ID_TranspChk, "Increase Transpancy",
		bitmap, wxNullBitmap,
		"Enable High Tarnsparency mode",
		"Enable High Tarnsparency mode");
	m_options_toolbar->ToggleTool(ID_TranspChk, false);
	//MIP
	bitmap = wxGetBitmap(mip, dpi_sf);
	m_options_toolbar->AddCheckTool(ID_MipChk, "MIP",
		bitmap, wxNullBitmap,
		"Enable Maximum Intensity Projection (MIP)",
		"Enable Maximum Intensity Projection (MIP)");
	m_options_toolbar->ToggleTool(ID_MipChk,false);
	//inversion
	bitmap = wxGetBitmap(invert_off, dpi_sf);
	m_options_toolbar->AddCheckTool(ID_InvChk, "Inversion",
		bitmap, wxNullBitmap,
		"Invert data intensity values",
		"Invert data intensity values");
	m_options_toolbar->ToggleTool(ID_InvChk,false);
	//component display
	bitmap = wxGetBitmap(comp_off, dpi_sf);
	m_options_toolbar->AddCheckTool(ID_CompChk, "Components",
		bitmap, wxNullBitmap,
		"Show components",
		"Show components");
	m_options_toolbar->ToggleTool(ID_CompChk, false);
	//interpolation
	bitmap = wxGetBitmap(interpolate, dpi_sf);
	m_options_toolbar->AddCheckTool(ID_InterpolateChk, "Interpolate",
		bitmap, wxNullBitmap,
		"Enable spatial interpolation of voxel intensity values",
		"Enable spatial interpolation of voxel intensity values");
	m_options_toolbar->ToggleTool(ID_InterpolateChk,true);
	//noise reduction
	bitmap = wxGetBitmap(smooth_off, dpi_sf);
	m_options_toolbar->AddCheckTool(ID_NRChk, "Smoothing",
		bitmap, wxNullBitmap,
		"Enable rendering result smoothing",
		"Enable rendering result smoothing");
	m_options_toolbar->ToggleTool(ID_NRChk,false);
	//sync group
	bitmap = wxGetBitmap(sync_chan, dpi_sf);
	m_options_toolbar->AddCheckTool(ID_SyncGroupChk,"Group Sync",
		bitmap, wxNullBitmap,
		"Sync current channel with other channels in the group",
		"Sync current channel with other channels in the group");
	m_options_toolbar->ToggleTool(ID_SyncGroupChk,false);
	//depth mode
	bitmap = wxGetBitmap(depth_off, dpi_sf);
	m_options_toolbar->AddCheckTool(ID_DepthChk, "Depth Mode",
		bitmap, wxNullBitmap,
		"Enable Depth Mode within the group",
		"Enable Depth Mode within the group");
	m_options_toolbar->ToggleTool(ID_DepthChk,false);
	//legend
	bitmap = wxGetBitmap(legend, dpi_sf);
	m_options_toolbar->AddCheckTool(ID_LegendChk, "Legend",
		bitmap, wxNullBitmap,
		"Enable name legend display for current channel",
		"Enable name legend display for current channel");
	m_options_toolbar->ToggleTool(ID_LegendChk,true);
	//buttons
	bitmap = wxGetBitmap(reset, dpi_sf);
	m_options_toolbar->AddTool(ID_ResetDefault,"Reset",
		bitmap, "Reset all properties");
	bitmap = wxGetBitmap(save_settings, dpi_sf);
	m_options_toolbar->AddTool(ID_SaveDefault,"Save",
		bitmap, "Set current settings as default");
	sizer_r1->AddStretchSpacer();
	sizer_r1->Add(m_options_toolbar, 0, wxALIGN_CENTER);
	sizer_r1->AddStretchSpacer();
	m_options_toolbar->Realize();
	//spacings
	//x
	st = new wxStaticText(this, 0, "Voxel Size: ",
		wxDefaultPosition, FromDIP(wxSize(70, -1)), wxALIGN_RIGHT);
	m_space_x_text = new wxTextCtrl(this, ID_SpaceXText, "1.000",
		wxDefaultPosition, FromDIP(wxSize(50, -1)), 0, vald_fp3);
	sizer_r2->Add(st, 0, wxALIGN_CENTER);
	//sizer_r2->AddStretchSpacer();
	st = new wxStaticText(this, 0, "X ");
	sizer_r2->Add(st, 0, wxALIGN_CENTER);
	sizer_r2->Add(m_space_x_text, 1, wxALIGN_CENTER);
	//y
	st = new wxStaticText(this, 0, "Y ");
	m_space_y_text = new wxTextCtrl(this, ID_SpaceYText, "1.000",
		wxDefaultPosition, FromDIP(wxSize(50, -1)), 0, vald_fp3);
	sizer_r2->Add(3, 5, 0);
	sizer_r2->Add(st, 0, wxALIGN_CENTER);
	sizer_r2->Add(m_space_y_text, 1, wxALIGN_CENTER);
	//z
	st = new wxStaticText(this, 0, "Z ");
	m_space_z_text = new wxTextCtrl(this, ID_SpaceZText, "1.000",
		wxDefaultPosition, FromDIP(wxSize(50, -1)), 0, vald_fp3);
	sizer_r2->Add(3, 5, 0);
	sizer_r2->Add(st, 0, wxALIGN_CENTER);
	sizer_r2->Add(m_space_z_text, 1, wxALIGN_CENTER);
	//color 1
	st = new wxStaticText(this, 0, "Prime Color: ",
		wxDefaultPosition, FromDIP(wxSize(70, -1)), wxALIGN_RIGHT);
	m_color_text = new wxTextCtrl(this, ID_ColorText, "255 , 255 , 255",
		wxDefaultPosition, FromDIP(wxSize(50, 20)));
	m_color_text->Connect(ID_ColorText, wxEVT_LEFT_DCLICK,
		wxCommandEventHandler(VPropView::OnColorTextFocus),
		NULL, this);
	m_color_btn = new wxColourPickerCtrl(this, ID_ColorBtn, *wxRED,
		wxDefaultPosition, FromDIP(wxSize(50, 25)));
	sizer_r3->Add(st, 0, wxALIGN_CENTER, 0); 
	sizer_r3->Add(5, 5, 0);
	sizer_r3->Add(m_color_text, 1, wxALIGN_CENTER, 0);
	sizer_r3->Add(m_color_btn, 1, wxALIGN_CENTER, 0);
	//color 2
	st = new wxStaticText(this, 0, "Secnd Color: ",
		wxDefaultPosition, FromDIP(wxSize(70, -1)), wxALIGN_RIGHT);
	m_color2_text = new wxTextCtrl(this, ID_Color2Text, "255 , 255 , 255",
		wxDefaultPosition, FromDIP(wxSize(50, 20)));
	m_color2_text->Connect(ID_Color2Text, wxEVT_LEFT_DCLICK,
		wxCommandEventHandler(VPropView::OnColor2TextFocus),
		NULL, this);
	m_color2_btn = new wxColourPickerCtrl(this, ID_Color2Btn, *wxRED,
		wxDefaultPosition, FromDIP(wxSize(50, 25)));
	sizer_r4->Add(st, 0, wxALIGN_CENTER, 0); 
	sizer_r4->Add(5, 5, 0);
	sizer_r4->Add(m_color2_text, 1, wxALIGN_CENTER, 0);
	sizer_r4->Add(m_color2_btn, 1, wxALIGN_CENTER, 0);
	// colormap chooser
	st = new wxStaticText(this, 0, "Effects: ",
		wxDefaultPosition, FromDIP(wxSize(70, -1)), wxALIGN_RIGHT);
	m_colormap_inv_btn = new wxToggleButton(this, ID_ColormapInvBtn,
		L"\u262f", wxDefaultPosition, FromDIP(wxSize(24, 24)));
#ifdef _WIN32
	wxFont font(30, wxFONTFAMILY_DEFAULT, wxNORMAL, wxNORMAL);
#else
	wxFont font(15, wxFONTFAMILY_DEFAULT, wxNORMAL, wxNORMAL);
#endif
	m_colormap_inv_btn->SetFont(font);
	m_colormap_combo = new wxComboBox(this, ID_ColormapCombo, "",
		wxDefaultPosition, FromDIP(wxSize(85, 25)), 0, NULL, wxCB_READONLY);
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
		wxDefaultPosition, FromDIP(wxSize(85, 25)), 0, NULL, wxCB_READONLY);
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
	sizer_left->Add(2, 2, 0);
	sizer_left->Add(sizer_l1, 0, wxEXPAND);
	sizer_left->Add(2, 2, 0);
	sizer_left->Add(sizer_l2, 0, wxEXPAND);
	sizer_left->Add(2, 2, 0);
	sizer_left->Add(sizer_l3, 0, wxEXPAND);
	sizer_left->Add(2, 2, 0);
	sizer_left->Add(sizer_l4, 0, wxEXPAND);
	sizer_left->Add(2, 2, 0);
	sizer_left->Add(sizer_l5, 0, wxEXPAND);
	//middle
	sizer_middle->Add(2, 2, 0);
	sizer_middle->Add(sizer_m1, 0, wxEXPAND);
	sizer_middle->Add(2, 2, 0);
	sizer_middle->Add(sizer_m2, 0, wxEXPAND);
	sizer_middle->Add(2, 2, 0);
	sizer_middle->Add(sizer_m3, 0, wxEXPAND);
	sizer_middle->Add(2, 2, 0);
	sizer_middle->Add(sizer_m4, 0, wxEXPAND);
	sizer_middle->Add(2, 2, 0);
	sizer_middle->Add(sizer_m5, 0, wxEXPAND);
	//right
	sizer_right->Add(2, 2, 0);
	sizer_right->Add(sizer_r1, 0, wxEXPAND);
	sizer_right->Add(2, 2, 0);
	sizer_right->Add(sizer_r2, 0, wxEXPAND);
	sizer_right->Add(2, 2, 0);
	sizer_right->Add(sizer_r3, 0, wxEXPAND);
	sizer_right->Add(2, 2, 0);
	sizer_right->Add(sizer_r4, 0, wxEXPAND);
	sizer_right->Add(2, 2, 0);
	sizer_right->Add(sizer_r5, 0, wxEXPAND);
	//ADD ALL TOGETHER
	sizer_all->Add(sizer_left, 1, wxEXPAND);
	sizer_all->Add(sizer_middle, 1, wxEXPAND);
	sizer_all->Add(sizer_right, 0, wxSHRINK);
	SetSizer(sizer_all);
	Layout();

	//add sliders for undo and redo
	glbin.add_slider(m_gamma_sldr);
	glbin.add_slider(m_saturation_sldr);
	glbin.add_slider(m_luminance_sldr);
	glbin.add_slider(m_alpha_sldr);
	glbin.add_slider(m_hi_shading_sldr);
	glbin.add_slider(m_low_shading_sldr);
	glbin.add_slider(m_boundary_sldr);
	glbin.add_slider(m_thresh_sldr);
	glbin.add_slider(m_shadow_sldr);
	glbin.add_slider(m_sample_sldr);
	glbin.add_slider(m_colormap_sldr);
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
	m_max_val = m_vd->GetMaxValue();
	m_max_val = std::max(255.0, m_max_val);

	//set range
	wxFloatingPointValidator<double>* vald_fp;
	wxIntegerValidator<unsigned int>* vald_i;

	//volume properties
	//transfer function
	//gamma
	if ((vald_fp = (wxFloatingPointValidator<double>*)m_gamma_text->GetValidator()))
		vald_fp->SetRange(0.0, 10.0);
	dval = m_vd->Get3DGamma();
	m_gamma_sldr->SetValue(int(dval*100.0+0.5));
	str = wxString::Format("%.2f", dval);
	m_gamma_text->ChangeValue(str);
	//boundary
	if ((vald_fp = (wxFloatingPointValidator<double>*)m_boundary_text->GetValidator()))
		vald_fp->SetRange(0.0, 1.0);
	dval = m_vd->GetBoundary();
	m_boundary_sldr->SetValue(int(dval*2000.0+0.5));
	str = wxString::Format("%.4f", dval);
	m_boundary_text->ChangeValue(str);
	//contrast
	if ((vald_i = (wxIntegerValidator<unsigned int>*)m_saturation_text->GetValidator()))
		vald_i->SetMin(0);
	dval = m_vd->GetOffset();
	ival = int(dval*m_max_val+0.5);
	m_saturation_sldr->SetRange(0, int(m_max_val));
	str = wxString::Format("%d", ival);
	m_saturation_sldr->SetValue(ival);
	m_saturation_text->ChangeValue(str);
	//left threshold
	if ((vald_i = (wxIntegerValidator<unsigned int>*)m_left_thresh_text->GetValidator()))
		vald_i->SetMin(0);
	dval = m_vd->GetLeftThresh();
	ival = int(dval*m_max_val+0.5);
	m_thresh_sldr->SetRange(0, int(m_max_val));
	str = wxString::Format("%d", ival);
	m_thresh_sldr->SetLowValue(ival);
	m_left_thresh_text->ChangeValue(str);
	//right threshold
	if ((vald_i = (wxIntegerValidator<unsigned int>*)m_right_thresh_text->GetValidator()))
		vald_i->SetMin(0);
	dval = m_vd->GetRightThresh();
	ival = int(dval*m_max_val+0.5);
	//m_right_thresh_sldr->SetRange(0, int(m_max_val));
	str = wxString::Format("%d", ival);
	m_thresh_sldr->SetHighValue(ival);
	//m_right_thresh_sldr->SetValue(ival);
	m_right_thresh_text->ChangeValue(str);
	//luminance
	if ((vald_i = (wxIntegerValidator<unsigned int>*)m_luminance_text->GetValidator()))
		vald_i->SetMin(0);
	dval = m_vd->GetLuminance();
	ival = int(dval*m_max_val+0.5);
	m_luminance_sldr->SetRange(0, int(m_max_val));
	str = wxString::Format("%d", ival);
	m_luminance_sldr->SetValue(ival);
	m_luminance_text->ChangeValue(str);
	//color
	fluo::Color c = m_vd->GetColor();
	wxColor wxc((unsigned char)(c.r()*255+0.5),
		(unsigned char)(c.g()*255+0.5),
		(unsigned char)(c.b()*255+0.5));
	m_color_text->ChangeValue(wxString::Format("%d , %d , %d",
		wxc.Red(), wxc.Green(), wxc.Blue()));
	m_color_btn->SetColour(wxc);
	c = m_vd->GetMaskColor();
	wxc = wxColor((unsigned char)(c.r()*255+0.5),
		(unsigned char)(c.g()*255+0.5),
		(unsigned char)(c.b()*255+0.5));
	m_color2_text->ChangeValue(wxString::Format("%d , %d , %d",
		wxc.Red(), wxc.Green(), wxc.Blue()));
	m_color2_btn->SetColour(wxc);
	//alpha
	if ((vald_i = (wxIntegerValidator<unsigned int>*)m_alpha_text->GetValidator()))
		vald_i->SetMin(0);
	dval = m_vd->GetAlpha();
	ival = int(dval*m_max_val+0.5);
	m_alpha_sldr->SetRange(0, int(m_max_val));
	str = wxString::Format("%d", ival);
	m_alpha_sldr->SetValue(ival);
	m_alpha_text->ChangeValue(str);
	bool alpha = m_vd->GetEnableAlpha();
	m_alpha_chk->SetValue(alpha);

	//shadings
	if ((vald_fp = (wxFloatingPointValidator<double>*)m_low_shading_text->GetValidator()))
		vald_fp->SetRange(0.0, 10.0);
	if ((vald_fp = (wxFloatingPointValidator<double>*)m_hi_shading_text->GetValidator()))
		vald_fp->SetRange(0.0, 100.0);
	double amb, diff, spec, shine;
	m_vd->GetMaterial(amb, diff, spec, shine);
	m_low_shading_sldr->SetValue(amb*100.0);
	str = wxString::Format("%.2f", amb);
	m_low_shading_text->ChangeValue(str);
	m_hi_shading_sldr->SetValue(shine*10.0);
	str = wxString::Format("%.2f", shine);
	m_hi_shading_text->ChangeValue(str);
	bool shade = m_vd->GetVR()->get_shading();
	m_shade_chk->SetValue(shade);

	//shadow
	if ((vald_fp = (wxFloatingPointValidator<double>*)m_shadow_text->GetValidator()))
		vald_fp->SetRange(0.0, 1.0);
	bool shadow = m_vd->GetShadow();
	m_shadow_chk->SetValue(shadow);
	double shadow_int;
	m_vd->GetShadowParams(shadow_int);
	m_shadow_sldr->SetValue(int(shadow_int*100.0+0.5));
	str = wxString::Format("%.2f", shadow_int);
	m_shadow_text->ChangeValue(str);

	//smaple rate
	if ((vald_fp = (wxFloatingPointValidator<double>*)m_sample_text->GetValidator()))
		vald_fp->SetRange(0.0, 100.0);
	double sr = m_vd->GetSampleRate();
	m_sample_sldr->SetValue(sr*10.0);
	str = wxString::Format("%.1f", sr);
	m_sample_text->ChangeValue(str);

	//spacings
	double spcx, spcy, spcz;
	m_vd->GetBaseSpacings(spcx, spcy, spcz);
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
	m_options_toolbar->ToggleTool(ID_LegendChk,m_vd->GetLegend());

	//interpolate
	bool interp = m_vd->GetInterpolate();
	m_options_toolbar->ToggleTool(ID_InterpolateChk, interp);
	if(interp) 
		m_options_toolbar->SetToolNormalBitmap(ID_InterpolateChk, 
		wxGetBitmapFromMemory(interpolate));
	else
		m_options_toolbar->SetToolNormalBitmap(ID_InterpolateChk, 
		wxGetBitmapFromMemory(interpolate_off));

	//sync group
	if (m_group)
		m_sync_group = m_group->GetVolumeSyncProp();
	m_options_toolbar->ToggleTool(ID_SyncGroupChk,m_sync_group);

	//colormap
	double low, high;
	m_vd->GetColormapValues(low, high);
	//low
	if ((vald_i = (wxIntegerValidator<unsigned int>*)m_colormap_low_text->GetValidator()))
		vald_i->SetMin(0);
	ival = int(low*m_max_val+0.5);
	m_colormap_sldr->SetRange(0, int(m_max_val));
	str = wxString::Format("%d", ival);
	m_colormap_sldr->SetLowValue(ival);
	m_colormap_low_text->ChangeValue(str);
	//high
	if ((vald_i = (wxIntegerValidator<unsigned int>*)m_colormap_hi_text->GetValidator()))
		vald_i->SetMin(0);
	ival = int(high*m_max_val+0.5);
	str = wxString::Format("%d", ival);
	m_colormap_sldr->SetHighValue(ival);
	m_colormap_hi_text->ChangeValue(str);
	//colormap
	m_colormap_inv_btn->SetValue(m_vd->GetColormapInv()>0.0?false:true);
	m_colormap_combo->SetSelection(m_vd->GetColormap());
	m_colormap_combo2->SetSelection(m_vd->GetColormapProj());
	//mode
	bool colormap = m_vd->GetColormapMode() == 1;
	m_colormap_chk->SetValue(colormap);

	//inversion
	bool inv = m_vd->GetInvert();
	m_options_toolbar->ToggleTool(ID_InvChk,inv);
	if(inv) 
		m_options_toolbar->SetToolNormalBitmap(ID_InvChk, 
		wxGetBitmapFromMemory(invert));
	else
		m_options_toolbar->SetToolNormalBitmap(ID_InvChk, 
		wxGetBitmapFromMemory(invert_off));

	//MIP
	int mode = m_vd->GetMode();
	if (mode == 1)
	{
		m_options_toolbar->ToggleTool(ID_MipChk,true);
		if (m_thresh_st)
			m_thresh_st->SetLabel("Shade Thresh. :");
	}
	else
	{
		m_options_toolbar->ToggleTool(ID_MipChk,false);
		if (m_thresh_st)
			m_thresh_st->SetLabel("Threshold :");
	}

	//transparency
	double alpha_power = m_vd->GetAlphaPower();
	if (alpha_power > 1.1)
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
	int label_mode = m_vd->GetLabelMode();
	if (label_mode)
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
	bool nr = m_vd->GetNR();
	m_options_toolbar->ToggleTool(ID_NRChk,nr);
	if(nr) 
		m_options_toolbar->SetToolNormalBitmap(ID_NRChk, 
		wxGetBitmapFromMemory(smooth));
	else
		m_options_toolbar->SetToolNormalBitmap(ID_NRChk, 
		wxGetBitmapFromMemory(smooth_off));

	//blend mode
	int blend_mode = m_vd->GetBlendMode();
	if (blend_mode == 2)
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
	if (shade)
		EnableShading();
	else
		DisableShading();
	if (shadow)
		EnableShadow();
	else
		DisableShadow();
	if (colormap)
		EnableColormap();
	else
		DisableColormap();
	if (mode == 1)
		EnableMip();
	else
		DisableMip();

	Layout();
}

void VPropView::SetVolumeData(VolumeData* vd)
{
	if (m_vd != vd)
		ClearUndo();

	m_vd = vd;
	GetSettings();
}

VolumeData* VPropView::GetVolumeData()
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

void VPropView::SetFocusVRenderViews(wxBasisSlider* slider)
{
	if (m_frame)
	{
		for (int i = 0; i < m_frame->GetViewNum(); i++)
		{
			VRenderGLView* view = m_frame->GetView(i);
			if (view)
			{
				view->SetFocusedSlider(slider);
			}
		}
	}
}

void VPropView::SetGroup(DataGroup* group)
{
	m_group = group;
	if (m_group)
	{
		m_sync_group = m_group->GetVolumeSyncProp();
		m_options_toolbar->ToggleTool(ID_SyncGroupChk,m_sync_group);
	}
}

DataGroup* VPropView::GetGroup()
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

void VPropView::ApplyMl()
{
	if (m_sync_group && m_group)
		m_group->ApplyMlVolProp();
	else if (m_vd)
		m_vd->ApplyMlVolProp();
	GetSettings();
	RefreshVRenderViews(false, true);
}

void VPropView::ClearUndo()
{
	m_gamma_sldr->Clear();
	m_saturation_sldr->Clear();
	m_luminance_sldr->Clear();
	m_alpha_sldr->Clear();
	m_hi_shading_sldr->Clear();
	m_low_shading_sldr->Clear();
	m_boundary_sldr->Clear();
	m_thresh_sldr->Clear();
	m_shadow_sldr->Clear();
	m_sample_sldr->Clear();
	m_colormap_sldr->Clear();
}

//1
void VPropView::OnGammaSync(wxCommandEvent& event)
{
	switch (glbin.get_mul_func())
	{
	case 0:
		{
		wxString str = m_gamma_text->GetValue();
		double dVal;
		str.ToDouble(&dVal);
		if (m_group)
			m_group->Set3DGamma(dVal);
		}
		break;
	case 1:
		SetFocusVRenderViews(m_gamma_sldr);
		break;
	case 2:
		break;
	case 3:
		break;
	case 4:
		m_gamma_sldr->Undo();
		break;
	}
	RefreshVRenderViews(false, true);
}

void VPropView::OnGammaChange(wxScrollEvent & event)
{
	double val = m_gamma_sldr->GetValue() / 100.0;
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
		m_group->Set3DGamma(val);
	else if (m_vd)
		m_vd->Set3DGamma(val);

	RefreshVRenderViews(false, true);
}

void VPropView::OnGammaChk(wxCommandEvent& event)
{

}

void VPropView::OnSaturationSync(wxCommandEvent& event)
{
	wxString str = m_saturation_text->GetValue();
	long iVal;
	str.ToLong(&iVal);
	double dVal = double(iVal) / m_max_val;
	if (m_group)
		m_group->SetOffset(dVal);
	RefreshVRenderViews(false, true);
}

void VPropView::OnSaturationChange(wxScrollEvent & event)
{
	int ival = m_saturation_sldr->GetValue();
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
		m_group->SetOffset(val);
	else if (m_vd)
		m_vd->SetOffset(val);

	RefreshVRenderViews(false, true);
}

void VPropView::OnSaturationChk(wxCommandEvent& event)
{

}

void VPropView::OnLuminanceSync(wxCommandEvent& event)
{
	wxString str = m_luminance_text->GetValue();
	long iVal;
	str.ToLong(&iVal);
	double dVal = double(iVal) / m_max_val;
	if (m_group)
		m_group->SetLuminance(dVal);
	RefreshVRenderViews(true, true);
}

void VPropView::OnLuminanceChange(wxScrollEvent& event)
{
	int ival = m_luminance_sldr->GetValue();
	wxString str = wxString::Format("%d", ival);
	if (str != m_luminance_text->GetValue())
		m_luminance_text->SetValue(str);
}

void VPropView::OnLuminanceText(wxCommandEvent& event)
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
		m_group->SetLuminance(val);
	else if (m_vd)
		m_vd->SetLuminance(val);

	if (m_vd)
	{
		fluo::Color color = m_vd->GetColor();
		wxColor wxc((unsigned char)(color.r() * 255),
			(unsigned char)(color.g() * 255),
			(unsigned char)(color.b() * 255));
		m_color_text->ChangeValue(wxString::Format("%d , %d , %d",
			wxc.Red(), wxc.Green(), wxc.Blue()));
		m_color_btn->SetBackgroundColour(wxc);
		m_lumi_change = true;
	}

	RefreshVRenderViews(true, true);
}

void VPropView::OnLuminanceChk(wxCommandEvent& event)
{

}

void VPropView::OnAlphaSync(wxCommandEvent& event)
{
	wxString str = m_alpha_text->GetValue();
	long iVal;
	str.ToLong(&iVal);
	double dVal = double(iVal) / m_max_val;
	if (m_group)
		m_group->SetAlpha(dVal);
	RefreshVRenderViews(false, true);
}

void VPropView::OnAlphaCheck(wxCommandEvent& event)
{
	bool alpha = m_alpha_chk->GetValue();
	if (m_sync_group && m_group)
		m_group->SetEnableAlpha(alpha);
	else if (m_vd)
		m_vd->SetEnableAlpha(alpha);

	if (alpha)
		EnableAlpha();
	else
		DisableAlpha();

	RefreshVRenderViews(false, true);
}

void VPropView::OnAlphaChange(wxScrollEvent& event)
{
	int ival = m_alpha_sldr->GetValue();
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
		m_group->SetAlpha(val);
	else if (m_vd)
		m_vd->SetAlpha(val);

	RefreshVRenderViews(false, true);
}

void VPropView::OnShadingSync(wxCommandEvent& event)
{
	bool bVal = m_shade_chk->GetValue();
	wxString str = m_low_shading_text->GetValue();
	double dVal;
	str.ToDouble(&dVal);
	str = m_hi_shading_text->GetValue();
	double dVal2;
	str.ToDouble(&dVal2);
	if (m_group)
	{
		m_group->SetShading(bVal);
		m_group->SetLowShading(dVal);
		m_group->SetHiShading(dVal2);
	}
	RefreshVRenderViews(false, true);
}

//hi shading
void VPropView::OnHiShadingChange(wxScrollEvent& event)
{
	double val = m_hi_shading_sldr->GetValue() / 10.0;
	wxString str = wxString::Format("%.2f", val);
	if (str != m_hi_shading_text->GetValue())
		m_hi_shading_text->SetValue(str);
}

void VPropView::OnHiShadingText(wxCommandEvent& event)
{
	wxString str = m_hi_shading_text->GetValue();
	double val = 0.0;
	str.ToDouble(&val);
	m_hi_shading_sldr->SetValue(int(val * 10.0 + 0.5));

	//set high shading value
	if (m_sync_group && m_group)
		m_group->SetHiShading(val);
	else if (m_vd)
		m_vd->SetHiShading(val);

	RefreshVRenderViews(false, true);
}

void VPropView::OnLowShadingChange(wxScrollEvent& event)
{
	double val = m_low_shading_sldr->GetValue() / 100.0;
	wxString str = wxString::Format("%.2f", val);
	if (str != m_low_shading_text->GetValue())
		m_low_shading_text->SetValue(str);
}

void VPropView::OnLowShadingText(wxCommandEvent& event)
{
	wxString str = m_low_shading_text->GetValue();
	double val = 0.0;
	str.ToDouble(&val);
	m_low_shading_sldr->SetValue(int(val * 100.0 + 0.5));

	//set low shading value
	if (m_sync_group && m_group)
		m_group->SetLowShading(val);
	else if (m_vd)
		m_vd->SetLowShading(val);

	RefreshVRenderViews(false, true);
}

void VPropView::OnShadingChk(wxCommandEvent& event)
{
	bool shading = m_shade_chk->GetValue();
	if (m_sync_group && m_group)
		m_group->SetShading(shading);
	else if (m_vd)
		m_vd->SetShading(shading);

	if (shading)
		EnableShading();
	else
		DisableShading();

	RefreshVRenderViews(false, true);
}

void VPropView::OnBoundarySync(wxCommandEvent& event)
{
	wxString str = m_boundary_text->GetValue();
	double dVal;
	str.ToDouble(&dVal);
	if (m_group)
		m_group->SetBoundary(dVal);
	RefreshVRenderViews(false, true);
}

void VPropView::OnBoundaryChange(wxScrollEvent& event)
{
	double val = m_boundary_sldr->GetValue() / 2000.0;
	wxString str = wxString::Format("%.4f", val);
	if (str != m_boundary_text->GetValue())
		m_boundary_text->SetValue(str);
}

void VPropView::OnBoundaryText(wxCommandEvent& event)
{
	wxString str = m_boundary_text->GetValue();
	double val = 0.0;
	str.ToDouble(&val);
	int ival = int(val * 2000.0 + 0.5);
	m_boundary_sldr->SetValue(ival);

	//set boundary value
	if (m_sync_group && m_group)
		m_group->SetBoundary(val);
	else if (m_vd)
		m_vd->SetBoundary(val);

	RefreshVRenderViews(false, true);
}

void VPropView::OnBoundaryChk(wxCommandEvent& event)
{

}

void VPropView::OnThreshSync(wxCommandEvent& event)
{
	wxString str = m_left_thresh_text->GetValue();
	long iVal;
	str.ToLong(&iVal);
	double dVal = double(iVal) / m_max_val;
	if (m_group)
		m_group->SetLeftThresh(dVal);
	str = m_right_thresh_text->GetValue();
	str.ToLong(&iVal);
	dVal = double(iVal) / m_max_val;
	if (m_group)
		m_group->SetRightThresh(dVal);
	RefreshVRenderViews(false, true);
}

void VPropView::OnThreshChange(wxScrollEvent &event)
{
	int ival = m_thresh_sldr->GetLowValue();
	wxString str = wxString::Format("%d", ival);
	if (str != m_left_thresh_text->GetValue())
		m_left_thresh_text->SetValue(str);

	ival = m_thresh_sldr->GetHighValue();
	str = wxString::Format("%d", ival);
	if (str != m_right_thresh_text->GetValue())
		m_right_thresh_text->SetValue(str);
}

void VPropView::OnThreshLowText(wxCommandEvent &event)
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
	double right_val = (double)m_thresh_sldr->GetHighValue() / m_max_val;

	if (val > right_val)
	{
		val = right_val;
		ival = int(val*m_max_val+0.5);
		wxString str2 = wxString::Format("%d", ival);
		m_left_thresh_text->ChangeValue(str2);
	}
	m_thresh_sldr->SetLowValue(ival);

	//set left threshold value
	if (m_sync_group && m_group)
		m_group->SetLeftThresh(val);
	else if (m_vd)
		m_vd->SetLeftThresh(val);

	RefreshVRenderViews(false, true);

	//update colocalization
	if (m_frame && m_frame->GetColocalizationDlg() &&
		m_frame->GetColocalizationDlg()->GetThreshUpdate())
		m_frame->GetColocalizationDlg()->Colocalize();
}

void VPropView::OnThreshHiText(wxCommandEvent &event)
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
	double left_val = m_thresh_sldr->GetLowValue() / m_max_val;

	if (val >= left_val)
	{
		m_thresh_sldr->SetHighValue(ival);

		//set right threshold value
		if (m_sync_group && m_group)
			m_group->SetRightThresh(val);
		else if (m_vd)
			m_vd->SetRightThresh(val);

		RefreshVRenderViews(false, true);
	}

	//update colocalization
	if (m_frame && m_frame->GetColocalizationDlg() &&
		m_frame->GetColocalizationDlg()->GetThreshUpdate())
		m_frame->GetColocalizationDlg()->Colocalize();
}

void VPropView::OnThreshLink(wxCommandEvent& event)
{
	bool val = m_thresh_sldr->GetLink();
	val = !val;
	m_thresh_sldr->SetLink(val);
	m_thresh_link_tb->ToggleTool(ID_ThreshLinkTb, val);
	if (val)
		m_thresh_link_tb->SetToolNormalBitmap(ID_ThreshLinkTb,
			wxGetBitmapFromMemory(link));
	else
		m_thresh_link_tb->SetToolNormalBitmap(ID_ThreshLinkTb,
			wxGetBitmapFromMemory(unlink));
}

void VPropView::OnThreshChk(wxCommandEvent& event)
{

}

//shadow
void VPropView::OnShadowSync(wxCommandEvent& event)
{
	bool bVal = m_shadow_chk->GetValue();
	wxString str = m_shadow_text->GetValue();
	double dVal;
	str.ToDouble(&dVal);
	if (m_group)
	{
		m_group->SetShadow(bVal);
		m_group->SetShadowParams(dVal);
	}
	RefreshVRenderViews(false, true);
}

void VPropView::OnShadowChk(wxCommandEvent &event)
{
	bool shadow = m_shadow_chk->GetValue();
	if (m_sync_group && m_group)
		m_group->SetShadow(shadow);
	else if (m_group && m_group->GetBlendMode()==2)
		m_vd->SetShadow(shadow);
	else if (m_vd)
		m_vd->SetShadow(shadow);

	if (shadow)
		EnableShadow();
	else
		DisableShadow();

	RefreshVRenderViews(false, true);
}

void VPropView::OnShadowChange(wxScrollEvent &event)
{
	double val = m_shadow_sldr->GetValue() / 100.0;
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

	//set shadow darkness
	if (m_sync_group && m_group)
		m_group->SetShadowParams(val);
	else if (m_group && m_group->GetBlendMode()==2)
		m_group->SetShadowParams(val);
	else if (m_vd)
		m_vd->SetShadowParams(val);

	RefreshVRenderViews(false, true);
}

void VPropView::OnSampleSync(wxCommandEvent& event)
{
	wxString str = m_sample_text->GetValue();
	double srate = 0.0;
	str.ToDouble(&srate);
	if (m_group)
		m_group->SetSampleRate(srate);
	RefreshVRenderViews(false, true);
}

void VPropView::OnSampleChange(wxScrollEvent & event)
{
	double val = m_sample_sldr->GetValue() / 10.0;
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
			VolumeData* vd = m_view->GetAllVolumeData(i);
			if (vd)
				vd->SetSampleRate(srate);
		}
	}
	else
	{
		if (m_sync_group && m_group)
			m_group->SetSampleRate(srate);
		else if (m_group && m_group->GetBlendMode()==2)
			m_group->SetSampleRate(srate);
		else if (m_vd)
			m_vd->SetSampleRate(srate);
	}

	RefreshVRenderViews(false, true);
}

void VPropView::OnSampleChk(wxCommandEvent& event)
{

}

//colormap controls
void VPropView::OnColormapSync(wxCommandEvent& event)
{
	if (m_group)
	{
		bool bVal = m_colormap_chk->GetValue();
		m_group->SetColormapMode(bVal ? 1 : 0);
		m_group->SetColormapDisp(bVal);
		long iVal;
		double dVal1, dVal2;
		wxString str = m_colormap_low_text->GetValue();
		str.ToLong(&iVal);
		dVal1 = double(iVal) / m_max_val;
		str = m_colormap_low_text->GetValue();
		str.ToLong(&iVal);
		dVal2 = double(iVal) / m_max_val;
		m_group->SetColormapValues(dVal1, dVal2);
		//colormap
		iVal = m_colormap_combo->GetCurrentSelection();
		m_group->SetColormap(iVal);
		//colormap inv
		bVal = m_colormap_inv_btn->GetValue();
		m_group->SetColormapInv(bVal?-1.0:1.0);
		//colormap proj
		iVal = m_colormap_combo2->GetCurrentSelection();
		m_group->SetColormapProj(iVal);
	}
	RefreshVRenderViews(false, true);
}

void VPropView::OnColormapChk(wxCommandEvent &event)
{
	bool colormap = 
		m_colormap_chk->GetValue();

	if (m_sync_group && m_group)
	{
		m_group->SetColormapMode(colormap?1:0);
		m_group->SetColormapDisp(colormap);
	}
	else if (m_vd)
	{
		m_vd->SetColormapMode(colormap?1:0);
		m_vd->SetColormapDisp(colormap);
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

void VPropView::OnColormapHiText(wxCommandEvent &event)
{
	wxString str = m_colormap_hi_text->GetValue();
	long iVal = 0;
	str.ToLong(&iVal);
	if (double(iVal) > m_max_val)
	{
		UpdateMaxVal(iVal);
		str = wxString::Format("%d", iVal);
		m_colormap_hi_text->ChangeValue(str);
	}
	long iVal2 = m_colormap_sldr->GetLowValue();

	if (iVal >= iVal2)
	{
		m_colormap_sldr->SetHighValue(iVal);

		double val = double(iVal)/m_max_val;

		if (m_sync_group && m_group)
			m_group->SetColormapValues(-1, val);
		else if (m_vd)
		{
			double low, high;
			m_vd->GetColormapValues(low, high);
			m_vd->SetColormapValues(low, val);
		}

		RefreshVRenderViews(false, true);
	}
}

void VPropView::OnColormapChange(wxScrollEvent &event)
{
	int iVal = m_colormap_sldr->GetLowValue();
	wxString str = wxString::Format("%d", iVal);
	if (str != m_colormap_low_text->GetValue())
		m_colormap_low_text->SetValue(str);

	iVal = m_colormap_sldr->GetHighValue();
	str = wxString::Format("%d", iVal);
	if (str != m_colormap_hi_text->GetValue())
		m_colormap_hi_text->SetValue(str);
}

void VPropView::OnColormapLowText(wxCommandEvent &event)
{
	wxString str = m_colormap_low_text->GetValue();
	long iVal = 0;
	str.ToLong(&iVal);
	if (double(iVal) > m_max_val)
	{
		UpdateMaxVal(iVal);
		str = wxString::Format("%d", iVal);
		m_colormap_low_text->ChangeValue(str);
	}
	//long iVal2 = m_colormap_high_value_sldr->GetValue();

	//if (iVal > iVal2)
	//{
	//	iVal = iVal2;
	//	str = wxString::Format("%d", iVal);
	//	m_colormap_low_text->ChangeValue(str);
	//}
	m_colormap_sldr->SetLowValue(iVal);

	double val = double(iVal)/m_max_val;

	if (m_sync_group && m_group)
		m_group->SetColormapValues(val, -1);
	else if (m_vd)
	{
		double low, high;
		m_vd->GetColormapValues(low, high);
		m_vd->SetColormapValues(val, high);
	}

	RefreshVRenderViews(false, true);
}

void VPropView::OnColormapLink(wxCommandEvent& event)
{
	bool val = m_colormap_sldr->GetLink();
	val = !val;
	m_colormap_sldr->SetLink(val);
	m_colormap_link_tb->ToggleTool(ID_ColormapLinkTb, val);
	if (val)
		m_colormap_link_tb->SetToolNormalBitmap(ID_ColormapLinkTb,
			wxGetBitmapFromMemory(link));
	else
		m_colormap_link_tb->SetToolNormalBitmap(ID_ColormapLinkTb,
			wxGetBitmapFromMemory(unlink));
}

void VPropView::OnColormapInvBtn(wxCommandEvent &event)
{
	bool val = m_colormap_inv_btn->GetValue();

	m_colormap_chk->SetValue(true);
	OnColormapChk(event);

	if (m_sync_group && m_group)
		m_group->SetColormapInv(val ? -1.0 : 1.0);
	else if (m_vd)
		m_vd->SetColormapInv(val ? -1.0 : 1.0);

	RefreshVRenderViews(false, true);

	//update colocalization
	if (m_frame && m_frame->GetColocalizationDlg() &&
		m_frame->GetColocalizationDlg()->GetColormapUpdate())
		m_frame->GetColocalizationDlg()->Colocalize();
}

void VPropView::OnColormapCombo(wxCommandEvent &event)
{
	int colormap = m_colormap_combo->GetCurrentSelection();

	m_colormap_chk->SetValue(true);
	OnColormapChk(event);
	if (colormap >= 5)
	{
		m_options_toolbar->ToggleTool(ID_TranspChk, true);
		OnTranspChk(event);
	}

	if (m_sync_group && m_group)
		m_group->SetColormap(colormap);
	else if (m_vd)
		m_vd->SetColormap(colormap);

	RefreshVRenderViews(false, true);

	//update colocalization
	if (m_frame && m_frame->GetColocalizationDlg() &&
		m_frame->GetColocalizationDlg()->GetColormapUpdate())
		m_frame->GetColocalizationDlg()->Colocalize();
}

void VPropView::OnColormapCombo2(wxCommandEvent &event)
{
	int colormap_proj = m_colormap_combo2->GetCurrentSelection();

	m_colormap_chk->SetValue(true);
	OnColormapChk(event);

	if (m_sync_group && m_group)
		m_group->SetColormapProj(colormap_proj);
	else if (m_vd)
		m_vd->SetColormapProj(colormap_proj);

	RefreshVRenderViews(false, true);
}

//6
void VPropView::OnColorChange(wxColor c)
{
	fluo::Color color(c.Red()/255.0, c.Green()/255.0, c.Blue()/255.0);
	if (m_vd)
	{
		if (m_lumi_change)
		{
			m_vd->SetColor(color, true);
			m_lumi_change = false;
		}
		else
			m_vd->SetColor(color);

		double lum = m_vd->GetLuminance();
		int ilum = int(lum*m_max_val+0.5);
		m_luminance_sldr->SetValue(ilum);
		wxString str = wxString::Format("%d", ilum);
		m_luminance_text->ChangeValue(str);

		if (!m_vd->GetMaskColorSet())
		{
			color = m_vd->GetMaskColor();
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
		m_vd->SetMaskColor(color);
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
		m_group->SetInvert(inv);
	else if (m_vd)
		m_vd->SetInvert(inv);

	RefreshVRenderViews(false, true);
}

void VPropView::OnMIPCheck(wxCommandEvent &event)
{
	int val = m_options_toolbar->GetToolState(ID_MipChk)?1:0;

	if (m_sync_group && m_group)
		m_group->SetMode(val);
	else if (m_vd)
		m_vd->SetMode(val);

	if (val==1)
	{
		EnableMip();
		if (m_thresh_st)
			m_thresh_st->SetLabel("Shade Thresh. :");
	}
	else
	{
		DisableMip();
		if (m_thresh_st)
			m_thresh_st->SetLabel("Threshold :");
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
			m_group->SetAlphaPower(2.0);
		else if (m_vd)
			m_vd->SetAlphaPower(2.0);
	}
	else
	{
		m_options_toolbar->SetToolNormalBitmap(ID_TranspChk,
			wxGetBitmapFromMemory(transplo));
		if (m_sync_group && m_group)
			m_group->SetAlphaPower(1.0);
		else if (m_vd)
			m_vd->SetAlphaPower(1.0);
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
			m_group->SetLabelMode(1);
		else if (m_vd)
			m_vd->SetLabelMode(1);
	}
	else
	{
		m_options_toolbar->SetToolNormalBitmap(ID_CompChk,
			wxGetBitmapFromMemory(comp_off));
		if (m_sync_group && m_group)
			m_group->SetLabelMode(0);
		else if (m_vd)
			m_vd->SetLabelMode(0);
	}

	RefreshVRenderViews(false, true);
}

//ml
void VPropView::OnUseMlChk(wxCommandEvent& event)
{
	ApplyMl();
	//settings not managed by ml
	if (!m_frame)
		return;
	DataManager* mgr = m_frame->GetDataManager();
	if (!mgr)
		return;
	if (!m_vd)
		return;
	//component display
	int ival = mgr->m_vol_com;
	m_options_toolbar->ToggleTool(ID_CompChk, ival ? true : false);
	if (m_sync_group && m_group)
		m_group->SetLabelMode(ival);
	else
		m_vd->SetLabelMode(ival);
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
			VolumeData* vd = m_view->GetAllVolumeData(i);
			if (vd)
				vd->SetNR(val);
		}
	}
	else
	{
		if (m_sync_group && m_group)
			m_group->SetNR(val);
		else if (m_group && m_group->GetBlendMode()==2)
			m_group->SetNR(val);
		else if (m_vd)
			m_vd->SetNR(val);
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
			m_group->SetBlendMode(2);
			if (m_vd)
			{
				m_group->SetNR(m_vd->GetNR());
				m_group->SetSampleRate(m_vd->GetSampleRate());
				m_group->SetShadow(m_vd->GetShadow());
				double sp;
				m_vd->GetShadowParams(sp);
				m_group->SetShadowParams(sp);
			}
		}
	}
	else
	{
		if (m_group)
			m_group->SetBlendMode(0);
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
		for (int i = 0; i < m_group->GetVolumeNum(); i++)
		{
			m_group->GetVolumeData(i)->SetSpacings(spcx, spcy, spcz);
			m_group->GetVolumeData(i)->SetBaseSpacings(spcx, spcy, spcz);
		}
	}
	else if (m_vd)
	{
		m_vd->SetSpacings(spcx, spcy, spcz);
		m_vd->SetBaseSpacings(spcx, spcy, spcz);
	}
	else return false;

	return true;
}

//enable/disable
void VPropView::EnableGamma()
{

}

void VPropView::DisableGamma()
{

}

void VPropView::EnableSaturation()
{

}

void VPropView::DisableSaturation()
{

}

void VPropView::EnableLuminance()
{

}

void VPropView::DisableLuminance()
{

}

void VPropView::EnableAlpha()
{
	if (m_vd->GetMode() != 1)
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
	if (m_vd->GetMode() == 1)
	{
		m_thresh_sldr->Enable();
		m_left_thresh_text->Enable();
		m_right_thresh_text->Enable();
	}
}

void VPropView::DisableShading()
{
	m_low_shading_sldr->Disable();
	m_low_shading_text->Disable();
	m_hi_shading_sldr->Disable();
	m_hi_shading_text->Disable();
	if (m_vd->GetMode() == 1 &&
		!m_vd->GetShadow())
	{
		m_thresh_sldr->Disable();
		m_left_thresh_text->Disable();
		m_right_thresh_text->Disable();
	}
}

void VPropView::EnableBoundary()
{

}

void VPropView::DisableBoundary()
{

}

void VPropView::EnableThresh()
{

}

void VPropView::DisableThresh()
{

}

void VPropView::EnableShadow()
{
	m_shadow_sldr->Enable();
	m_shadow_text->Enable();
	if (m_vd->GetMode() == 1)
	{
		m_thresh_sldr->Enable();
		m_left_thresh_text->Enable();
		m_right_thresh_text->Enable();
	}
}

void VPropView::DisableShadow()
{
	m_shadow_sldr->Disable();
	m_shadow_text->Disable();
	if (m_vd->GetMode() == 1 &&
		!m_vd->GetShading())
	{
		m_thresh_sldr->Disable();
		m_left_thresh_text->Disable();
		m_right_thresh_text->Disable();
	}
}

void VPropView::EnableSample()
{

}

void VPropView::DisableSample()
{

}

void VPropView::EnableColormap()
{
	m_colormap_low_text->Enable();
	m_colormap_sldr->Enable();
	m_colormap_hi_text->Enable();
}

void VPropView::DisableColormap()
{
	m_colormap_low_text->Disable();
	m_colormap_sldr->Disable();
	m_colormap_hi_text->Disable();
}

void VPropView::EnableMip()
{
	DisableAlpha();
	m_boundary_sldr->Disable();
	m_boundary_text->Disable();
	m_luminance_sldr->Disable();
	m_luminance_text->Disable();
	if (m_vd->GetShading() ||
		m_vd->GetShadow())
		EnableShading();
	else
		DisableShading();
}

void VPropView::DisableMip()
{
	if (m_vd->GetEnableAlpha())
		EnableAlpha();
	else
		DisableAlpha();
	m_boundary_sldr->Enable();
	m_boundary_text->Enable();
	m_luminance_sldr->Enable();
	m_luminance_text->Enable();
	if (m_vd->GetShading() ||
		m_vd->GetShadow())
		EnableShading();
	else
		DisableShading();
	m_thresh_sldr->Enable();
	m_left_thresh_text->Enable();
	m_right_thresh_text->Enable();
}

//update max value
void VPropView::UpdateMaxVal(double value)
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
	m_vd->SetMaxValue(m_max_val);
	m_vd->SetScalarScale(65535.0 / m_max_val);
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
		m_vd->SetLegend(leg);

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
		m_group->SetInterpolate(inv);
	else if (m_vd)
		m_vd->SetInterpolate(inv);
	if (m_view)
		m_view->SetIntp(inv);

	RefreshVRenderViews(false, true);
}

//sync within group
void VPropView::OnSyncGroupCheck(wxCommandEvent& event)
{
	m_sync_group = m_options_toolbar->GetToolState(ID_SyncGroupChk);
	if (m_group)
		m_group->SetVolumeSyncProp(m_sync_group);

	if (m_sync_group && m_group)
	{
		wxString str;
		double dVal;
		long iVal;
		bool bVal;

		//gamma
		str = m_gamma_text->GetValue();
		str.ToDouble(&dVal);
		m_group->Set3DGamma(dVal);
		//boundary
		str = m_boundary_text->GetValue();
		str.ToDouble(&dVal);
		m_group->SetBoundary(dVal);
		//saturation
		str = m_saturation_text->GetValue();
		str.ToLong(&iVal);
		dVal = double(iVal) / m_max_val;
		m_group->SetOffset(dVal);
		//left threshold
		str = m_left_thresh_text->GetValue();
		str.ToLong(&iVal);
		dVal = double(iVal) / m_max_val;
		m_group->SetLeftThresh(dVal);
		//right thresh
		str = m_right_thresh_text->GetValue();
		str.ToLong(&iVal);
		dVal = double(iVal) / m_max_val;
		m_group->SetRightThresh(dVal);
		//luminance
		//str = m_luminance_text->GetValue();
		//str.ToLong(&iVal);
		//dVal = double(iVal)/m_max_val;
		//m_group->SetLuminance(dVal);
		//shadow
		bVal = m_shadow_chk->GetValue();
		m_group->SetShadow(bVal);
		str = m_shadow_text->GetValue();
		str.ToDouble(&dVal);
		m_group->SetShadowParams(dVal);
		//high shading
		str = m_hi_shading_text->GetValue();
		str.ToDouble(&dVal);
		m_group->SetHiShading(dVal);
		//alpha
		str = m_alpha_text->GetValue();
		str.ToLong(&iVal);
		dVal = double(iVal) / m_max_val;
		m_group->SetAlpha(dVal);
		//sample rate
		str = m_sample_text->GetValue();
		str.ToDouble(&dVal);
		m_group->SetSampleRate(dVal);
		//shading
		bVal = m_shade_chk->GetValue();
		m_group->SetShading(bVal);
		str = m_low_shading_text->GetValue();
		str.ToDouble(&dVal);
		m_group->SetLowShading(dVal);
		//inversion
		bVal = m_options_toolbar->GetToolState(ID_InvChk);
		m_group->SetInvert(bVal);
		//interpolation
		bVal = m_options_toolbar->GetToolState(ID_InterpolateChk);
		m_group->SetInterpolate(bVal);
		if (m_view)
			m_view->SetIntp(bVal);
		//MIP
		bVal = m_options_toolbar->GetToolState(ID_MipChk);
		m_group->SetMode(bVal?1:0);
		//transp
		bVal = m_options_toolbar->GetToolState(ID_TranspChk);
		m_group->SetAlphaPower(bVal ? 2.0 : 1.0);
		//noise reduction
		bVal = m_options_toolbar->GetToolState(ID_InvChk);
		m_group->SetNR(bVal);
		//colormap low
		str = m_colormap_low_text->GetValue();
		str.ToLong(&iVal);
		dVal = double(iVal) / m_max_val;
		m_group->SetColormapValues(dVal, -1);
		//colormap high
		str = m_colormap_hi_text->GetValue();
		str.ToLong(&iVal);
		dVal = double(iVal) / m_max_val;
		m_group->SetColormapValues(-1, dVal);
		//colormap mode
		bVal = m_colormap_chk->GetValue();
		m_group->SetColormapMode(bVal?1:0);
		//colormap
		iVal = m_colormap_combo->GetCurrentSelection();
		m_group->SetColormap(iVal);
		//colormap inv
		bVal = m_colormap_inv_btn->GetValue();
		m_group->SetColormapInv(bVal ? -1.0 : 1.0);
		//colormap proj
		iVal = m_colormap_combo2->GetCurrentSelection();
		m_group->SetColormapProj(iVal);
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

	bool use_ml = glbin.get_vp_table_enable() && m_vd;
	flrd::EntryParams* ep = 0;
	if (use_ml)
	{
		ep = new flrd::EntryParams();
		ep->setParams(glbin.get_params("vol_prop"));
	}

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
	if (use_ml)
		ep->setParam("extract_boundary", float(val));
	else
	{
		fconfig.Write("extract_boundary", val);
		mgr->m_vol_exb = val;
	}
	//gamma
	str = m_gamma_text->GetValue();
	str.ToDouble(&val);
	if (use_ml)
		ep->setParam("gamma3d", float(val));
	else
	{
		fconfig.Write("gamma", val);
		mgr->m_vol_gam = val;
	}
	//low offset
	str = m_saturation_text->GetValue();
	str.ToDouble(&val);
	val /= m_max_val;
	if (use_ml)
		ep->setParam("low_offset", float(val));
	else
	{
		fconfig.Write("low_offset", val);
		mgr->m_vol_of1 = val;
	}
	//high offset
	val = 1.0;
	if (use_ml)
		ep->setParam("high_offset", float(val));
	else
	{
		fconfig.Write("high_offset", val);
		mgr->m_vol_of2 = val;
	}
	//low thresholding
	str = m_left_thresh_text->GetValue();
	str.ToDouble(&val);
	val /= m_max_val;
	if (use_ml)
		ep->setParam("low_threshold", float(val));
	else
	{
		fconfig.Write("low_thresholding", val);
		mgr->m_vol_lth = val;
	}
	//high thresholding
	str = m_right_thresh_text->GetValue();
	str.ToDouble(&val);
	val /= m_max_val;
	if (use_ml)
		ep->setParam("high_threshold", float(val));
	else
	{
		fconfig.Write("high_thresholding", val);
		mgr->m_vol_hth = val;
	}
	//low shading
	str = m_low_shading_text->GetValue();
	str.ToDouble(&val);
	if (use_ml)
		ep->setParam("low_shading", float(val));
	else
	{
		fconfig.Write("low_shading", val);
		mgr->m_vol_lsh = val;
	}
	//high shading
	str = m_hi_shading_text->GetValue();
	str.ToDouble(&val);
	if (use_ml)
		ep->setParam("high_shading", float(val));
	else
	{
		fconfig.Write("high_shading", val);
		mgr->m_vol_hsh = val;
	}
	//alpha
	str = m_alpha_text->GetValue();
	str.ToDouble(&val);
	val /= m_max_val;
	if (use_ml)
		ep->setParam("alpha", float(val));
	else
	{
		fconfig.Write("alpha", val);
		mgr->m_vol_alf = val;
	}
	//sample rate
	str = m_sample_text->GetValue();
	str.ToDouble(&val);
	if (use_ml)
		ep->setParam("sample_rate", float(val));
	else
	{
		fconfig.Write("sample_rate", val);
		mgr->m_vol_spr = val;
	}
	//spacings
	if (!use_ml)
	{
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
	}
	//luminance
	str = m_luminance_text->GetValue();
	str.ToDouble(&val);
	val /= m_max_val;
	if (use_ml)
		ep->setParam("luminance", float(val));
	else
	{
		fconfig.Write("luminance", val);
		mgr->m_vol_lum = val;
	}
	//colormap enable
	bool bval = m_colormap_chk->GetValue();
	if (use_ml)
		ep->setParam("colormap_enable", bval);
	else
	{
		fconfig.Write("colormap_mode", bval);
		mgr->m_vol_cmm = bval;
	}
	//colormap inv
	bval = m_colormap_inv_btn->GetValue();
	if (use_ml)
		ep->setParam("colormap_inv", bval);
	else
	{
		fconfig.Write("colormap_inv", bval);
		mgr->m_vol_cmi = bval;
	}
	//colormap type
	ival = m_colormap_combo->GetCurrentSelection();
	if (use_ml)
		ep->setParam("colormap_type", ival);
	else
	{
		fconfig.Write("colormap", ival);
		mgr->m_vol_cmp = ival;
	}
	//colormap projection
	ival = m_colormap_combo2->GetCurrentSelection();
	if (use_ml)
		ep->setParam("colormap_proj", ival);
	else
	{
		fconfig.Write("colormap_proj", ival);
		mgr->m_vol_cmj = ival;
	}
	//colormap low value
	str = m_colormap_low_text->GetValue();
	str.ToDouble(&val);
	val /= m_max_val;
	if (use_ml)
		ep->setParam("colormap_low", float(val));
	else
	{
		fconfig.Write("colormap_low", val);
		mgr->m_vol_lcm = val;
	}
	//colormap high value
	str = m_colormap_hi_text->GetValue();
	str.ToDouble(&val);
	val /= m_max_val;
	if (use_ml)
		ep->setParam("colormap_hi", float(val));
	else
	{
		fconfig.Write("colormap_hi", val);
		mgr->m_vol_hcm = val;
	}
	//alpha
	bool alpha = m_alpha_chk->GetValue();
	if (use_ml)
		ep->setParam("alpha_enable", alpha);
	else
	{
		fconfig.Write("enable_alpha", alpha);
		mgr->m_vol_eap = alpha;
	}
	//enable shading
	bool shading = m_shade_chk->GetValue();
	if (use_ml)
		ep->setParam("shading_enable", shading);
	else
	{
		fconfig.Write("enable_shading", shading);
		mgr->m_vol_esh = shading;
	}
	//interpolation
	bool interp = m_options_toolbar->GetToolState(ID_InterpolateChk);
	if (use_ml)
		ep->setParam("interp_enable", interp);
	else
	{
		fconfig.Write("enable_interp", interp);
		mgr->m_vol_interp = interp;
	}
	//inversion
	bool inv = m_options_toolbar->GetToolState(ID_InvChk);
	if (use_ml)
		ep->setParam("invert_enable", inv);
	else
	{
		fconfig.Write("enable_inv", inv);
		mgr->m_vol_inv = inv;
	}
	//enable mip
	bool mip = m_options_toolbar->GetToolState(ID_MipChk);
	if (use_ml)
		ep->setParam("mip_enable", mip);
	else
	{
		fconfig.Write("enable_mip", mip);
		mgr->m_vol_mip = mip;
	}
	//enable hi transp
	bool trp = m_options_toolbar->GetToolState(ID_TranspChk);
	if (use_ml)
		ep->setParam("transparent_enable", trp);
	else
	{
		fconfig.Write("enable_trp", trp);
		mgr->m_vol_trp = trp;
	}
	//enable component display
	if (!use_ml)
	{
		int comp = m_options_toolbar->GetToolState(ID_CompChk);
		fconfig.Write("enable_comp", comp);
		mgr->m_vol_com = comp;
	}
	//noise reduction
	bool nrd = m_options_toolbar->GetToolState(ID_NRChk);
	if (use_ml)
		ep->setParam("denoise_enable", nrd);
	else
	{
		fconfig.Write("noise_rd", nrd);
		mgr->m_vol_nrd = nrd;
	}
	//shadow
	bool shw = m_shadow_chk->GetValue();
	if (use_ml)
		ep->setParam("shadow_enable", shw);
	else
	{
		fconfig.Write("enable_shadow", shw);
		mgr->m_vol_shw = shw;
	}
	//shadow intensity
	str = m_shadow_text->GetValue();
	str.ToDouble(&val);
	double swi = val;
	if (use_ml)
		ep->setParam("shadow_intensity", float(val));
	else
	{
		fconfig.Write("shadow_intensity", swi);
		mgr->m_vol_swi = swi;
	}

	if (use_ml)
	{
		//histogram
		flrd::Histogram histogram(m_vd);
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
	else
	{
		wxString expath = wxStandardPaths::Get().GetExecutablePath();
		expath = wxPathOnly(expath);
		wxString dft = expath + GETSLASH() + "default_volume_settings.dft";
		SaveConfig(fconfig, dft);
	}
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
	int ival;
	bool bval;

	//gamma
	dval = mgr->m_vol_gam;
	str = wxString::Format("%.2f", dval);
	m_gamma_text->ChangeValue(str);
	ival = int(dval*100.0+0.5);
	m_gamma_sldr->SetValue(ival);
	m_vd->Set3DGamma(dval);
	//extract boundary
	dval = mgr->m_vol_exb;
	str = wxString::Format("%.4f", dval);
	m_boundary_text->ChangeValue(str);
	ival = int(dval*2000.0+0.5);
	m_boundary_sldr->SetValue(ival);
	m_vd->SetBoundary(dval);
	//low offset
	dval = mgr->m_vol_of1;
	ival = int(dval*m_max_val+0.5);
	str = wxString::Format("%d", ival);
	m_saturation_text->ChangeValue(str);
	m_saturation_sldr->SetValue(ival);
	m_vd->SetOffset(dval);
	//low thresholding
	dval = mgr->m_vol_lth;
	ival = int(dval*m_max_val+0.5);
	str = wxString::Format("%d", ival);
	m_left_thresh_text->ChangeValue(str);
	m_thresh_sldr->SetLowValue(ival);
	m_vd->SetLeftThresh(dval);
	//high thresholding
	dval = mgr->m_vol_hth;
	ival = int(dval*m_max_val+0.5);
	str = wxString::Format("%d", ival);
	m_right_thresh_text->ChangeValue(str);
	m_thresh_sldr->SetHighValue(ival);
	m_vd->SetRightThresh(dval);
	//low shading
	dval = mgr->m_vol_lsh;
	str = wxString::Format("%.2f", dval);
	m_low_shading_text->ChangeValue(str);
	ival = int(dval*100.0+0.5);
	m_low_shading_sldr->SetValue(ival);
	double amb, diff, spec, shine;
	m_vd->GetMaterial(amb, diff, spec, shine);
	m_vd->SetMaterial(dval, diff, spec, shine);
	//high shading
	dval = mgr->m_vol_hsh;
	str = wxString::Format("%.2f", dval);
	m_hi_shading_text->ChangeValue(str);
	ival = int(dval*10.0+0.5);
	m_hi_shading_sldr->SetValue(ival);
	m_vd->GetMaterial(amb, diff, spec, shine);
	m_vd->SetMaterial(amb, diff, spec, dval);
	//alpha
	dval = mgr->m_vol_alf;
	ival = int(dval*m_max_val+0.5);
	str = wxString::Format("%d", ival);
	m_alpha_text->ChangeValue(str);
	m_alpha_sldr->SetValue(ival);
	m_vd->SetAlpha(dval);
	//sample rate
	dval = mgr->m_vol_spr;
	str = wxString::Format("%.1f", dval);
	m_sample_text->ChangeValue(str);
	ival = int(dval*10.0+0.5);
	m_sample_sldr->SetValue(ival);
	m_vd->SetSampleRate(dval);
	//luminance
	dval = mgr->m_vol_lum;
	ival = int(dval*m_max_val+0.5);
	str = wxString::Format("%d", ival);
	m_luminance_text->ChangeValue(str);
	m_luminance_sldr->SetValue(ival);
	double h, s, v;
	m_vd->GetHSV(h, s, v);
	fluo::HSVColor hsv(h, s, dval);
	fluo::Color color(hsv);
	m_vd->ResetMaskColorSet();
	m_vd->SetColor(color);
	wxColor wxc((unsigned char)(color.r()*255),
		(unsigned char)(color.g()*255),
		(unsigned char)(color.b()*255));
	m_color_text->ChangeValue(wxString::Format("%d , %d , %d",
		wxc.Red(), wxc.Green(), wxc.Blue()));
	m_color_btn->SetColour(wxc);
	color = m_vd->GetMaskColor();
	wxc = wxColor((unsigned char)(color.r()*255),
		(unsigned char)(color.g()*255),
		(unsigned char)(color.b()*255));
	m_color2_text->ChangeValue(wxString::Format("%d , %d , %d",
		wxc.Red(), wxc.Green(), wxc.Blue()));
	m_color2_btn->SetColour(wxc);
	//colormap mode
	m_vd->SetColormapMode(mgr->m_vol_cmm);
	bool colormap = m_vd->GetColormapMode() == 1;
	m_colormap_chk->SetValue(colormap);
	//colormap inv
	m_colormap_inv_btn->SetValue(mgr->m_vol_cmi);
	m_vd->SetColormapInv(mgr->m_vol_cmi?-1.0:1.0);
	//colormap
	m_colormap_combo->SetSelection(mgr->m_vol_cmp);
	m_vd->SetColormap(mgr->m_vol_cmp);
	//colormap projection
	m_colormap_combo2->SetSelection(mgr->m_vol_cmj);
	m_vd->SetColormapProj(mgr->m_vol_cmj);
	//colormap low value
	dval = mgr->m_vol_lcm;
	ival = int(dval*m_max_val+0.5);
	str = wxString::Format("%d", ival);
	m_colormap_low_text->ChangeValue(str);
	m_colormap_sldr->SetLowValue(ival);
	double lcm = dval;
	dval = mgr->m_vol_hcm;
	ival = int(dval*m_max_val+0.5);
	str = wxString::Format("%d", ival);
	m_colormap_hi_text->ChangeValue(str);
	m_colormap_sldr->SetHighValue(ival);
	m_vd->SetColormapValues(lcm, dval);
	//shadow intensity
	dval = mgr->m_vol_swi;
	str = wxString::Format("%.2f", dval);
	ival = int(dval*100.0+0.5);
	m_shadow_text->ChangeValue(str);
	m_shadow_sldr->SetValue(ival);
	m_vd->SetShadowParams(dval);

	//enable alpha
	bval = mgr->m_vol_eap;
	m_alpha_chk->SetValue(bval);
	if (m_sync_group && m_group)
		m_group->SetEnableAlpha(bval);
	else
		m_vd->SetEnableAlpha(bval);
	//enable shading
	bval = mgr->m_vol_esh;
	m_shade_chk->SetValue(bval);
	if (m_sync_group && m_group)
		m_group->SetShading(bval);
	else
		m_vd->SetShading(bval);
	//inversion
	bval = mgr->m_vol_inv;
	m_options_toolbar->ToggleTool(ID_InvChk,bval);
	if (m_sync_group && m_group)
		m_group->SetInvert(bval);
	else
		m_vd->SetInvert(bval);
	//enable mip
	bval = mgr->m_vol_mip;
	m_options_toolbar->ToggleTool(ID_MipChk,bval);
	if (m_sync_group && m_group)
		m_group->SetMode(bval?1:0);
	else
		m_vd->SetMode(bval?1:0);
	//enable transp
	bval = mgr->m_vol_trp;
	m_options_toolbar->ToggleTool(ID_TranspChk, bval);
	if (m_sync_group && m_group)
		m_group->SetAlphaPower(bval ? 2.0 : 1.0);
	else
		m_vd->SetAlphaPower(bval ? 2.0 : 1.0);
	//component display
	ival = mgr->m_vol_com;
	m_options_toolbar->ToggleTool(ID_CompChk, ival?true:false);
	if (m_sync_group && m_group)
		m_group->SetLabelMode(ival);
	else
		m_vd->SetLabelMode(ival);
	//noise reduction
	bval = mgr->m_vol_nrd;
	m_options_toolbar->ToggleTool(ID_NRChk,bval);
	if (m_sync_group && m_group)
		m_group->SetNR(bval);
	else
		m_vd->SetNR(bval);
	//shadow
	bval = mgr->m_vol_shw;
	m_shadow_chk->SetValue(bval);
	if (m_sync_group && m_group)
		m_group->SetShadow(bval);
	else
		m_vd->SetShadow(bval);
	//colormap
	bval = mgr->m_vol_cmm;
	if (m_sync_group && m_group)
		m_group->SetColormapMode(bval);

	if (m_vd->GetEnableAlpha())
		EnableAlpha();
	else
		DisableAlpha();
	if (m_vd->GetVR()->get_shading())
		EnableShading();
	else
		DisableShading();
	if (m_vd->GetShadow())
		EnableShadow();
	else
		DisableShadow();
	if (m_vd->GetColormapMode() == 1)
		EnableColormap();
	else
		DisableColormap();
	if (m_vd->GetMode() == 1)
		EnableMip();
	else
		DisableMip();

	//apply all
	RefreshVRenderViews(false, true);
}
