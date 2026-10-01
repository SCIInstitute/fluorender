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

#include <RenderViewPanel.h>
#include <RenderViewPanelAgent.h>
#include <RenderCanvas.h>
#include <Global.h>
#include <Names.h>
#include <MainSettings.h>
#include <RenderView.h>
#include <RenderCanvasAgent.h>
#include <wxSingleSlider.h>
#include <wxUndoableScrollBar.h>
#include <wxUndoableToolbar.h>
#include <wxUndoableColorPicker.h>
#include <wxBoldText.h>
#include <wxNumTextCtrl.h>
#include <wx/utils.h>
#include <wx/valnum.h>
#include <algorithm>
#include <wx/display.h>
#include <png_resource.h>
#include <icons.h>
#include <limits>

CaptureHook::CaptureHook(
	const CaptureOptions& options) :
	m_options(options)
{
}

void CaptureHook::AddCustomControls(
	wxFileDialogCustomize& customizer)
{
	// Compress

	m_compressChk =
		customizer.AddCheckBox(
			"Compress to save space");

	m_compressChk->SetValue(
		m_options.compress);

	// Alpha

	m_alphaChk =
		customizer.AddCheckBox(
			"Save alpha channel");

	m_alphaChk->SetValue(
		m_options.saveAlpha);

	// Float

	m_floatChk =
		customizer.AddCheckBox(
			"Save float channel");

	m_floatChk->SetValue(
		m_options.saveFloat);

	// DPI

	m_dpiTxt =
		customizer.AddTextCtrl(
			wxString::Format("%d",
				m_options.dpi));

	// Enlarge

	m_enlargeChk =
		customizer.AddCheckBox(
			"Enlarge output image");

	m_enlargeChk->SetValue(
		m_options.enlarge);

	m_enlargeTxt =
		customizer.AddTextCtrl(
			wxString::Format("%.1f",
				m_options.enlargeScale));

	// Embed

	if (glbin_settings.m_prj_save)
	{
		m_embedChk =
			customizer.AddCheckBox(
				"Embed all files in the project folder");

		m_embedChk->SetValue(
			m_options.embedFiles);
	}
}

void CaptureHook::TransferDataFromCustomControls()
{
	if (m_compressChk)
		m_options.compress =
		m_compressChk->GetValue();

	if (m_alphaChk)
		m_options.saveAlpha =
		m_alphaChk->GetValue();

	if (m_floatChk)
		m_options.saveFloat =
		m_floatChk->GetValue();

	if (m_dpiTxt)
	{
		long dpi = 72;

		if (m_dpiTxt->GetValue().ToLong(&dpi))
			m_options.dpi =
			static_cast<int>(dpi);
	}

	if (m_enlargeChk)
		m_options.enlarge =
		m_enlargeChk->GetValue();

	if (m_enlargeTxt)
	{
		double scale = 1.0;

		if (m_enlargeTxt->GetValue().ToDouble(&scale))
			m_options.enlargeScale =
			scale;
	}

	if (m_embedChk)
		m_options.embedFiles =
		m_embedChk->GetValue();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RenderViewPanel::RenderViewPanel(wxWindow* parent,
	wxGLContext* sharedContext,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name) :
	AgentPanel(parent, pos, size, style, name),
	m_channel_mix_mode(ChannelMixMode::CompositeAdd)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);
	Freeze();

	wxLogNull logNo;
	//full frame
	m_full_frame = new wxFrame((wxFrame*)NULL, wxID_ANY, "FluoRender");
	m_view_sizer = new wxBoxSizer(wxVERTICAL);

	//m_dpi_sf = GetDPIScaleFactor();
	//m_dpi_sf2 = std::round(m_dpi_sf - 0.1);
	//m_dpi_sf2 = m_dpi_sf2 < m_dpi_sf ? m_dpi_sf : 1;

	//m_canvas = glbin_flui_builder.BuildRenderCanvas(this, sharedContext);
	//m_renderview = m_canvas->GetAgent()->GetView();
	m_view_sizer->Add(m_canvas, 1, wxEXPAND);
	CreateBar();

	//add controls
	glbin.add_undo_control(m_mix_mode_tb);
	glbin.add_undo_control(m_hud_tb);
	glbin.add_undo_control(m_bg_color_picker);
	glbin.add_undo_control(m_aov_sldr);
	glbin.add_undo_control(m_cam_op_tb);
	glbin.add_undo_control(m_depth_atten_btn);
	glbin.add_undo_control(m_depth_atten_factor_sldr);
	glbin.add_undo_control(m_scale_factor_sldr);
	glbin.add_undo_control(m_slider_mode_btn);
	glbin.add_undo_control(m_x_rot_sldr);
	glbin.add_undo_control(m_y_rot_sldr);
	glbin.add_undo_control(m_z_rot_sldr);

	Thaw();
}

RenderViewPanel::~RenderViewPanel()
{
	if (m_full_frame)
		m_full_frame->Destroy();

	//delete controls
	glbin.del_undo_control(m_mix_mode_tb);
	glbin.del_undo_control(m_hud_tb);
	glbin.del_undo_control(m_bg_color_picker);
	glbin.del_undo_control(m_aov_sldr);
	glbin.del_undo_control(m_cam_op_tb);
	glbin.del_undo_control(m_depth_atten_btn);
	glbin.del_undo_control(m_depth_atten_factor_sldr);
	glbin.del_undo_control(m_scale_factor_sldr);
	glbin.del_undo_control(m_slider_mode_btn);
	glbin.del_undo_control(m_x_rot_sldr);
	glbin.del_undo_control(m_y_rot_sldr);
	glbin.del_undo_control(m_z_rot_sldr);
}

void RenderViewPanel::CreateBar()
{
	//validator: floating point 1
	wxFloatingPointValidator<double> vald_fp1(1);
	vald_fp1.SetRange(0.0, 360.0);
	//validator: floating point 2
	wxFloatingPointValidator<double> vald_fp2(2);
	vald_fp2.SetRange(0.0, 1.0);
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;
	vald_int.SetMin(1);

	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* sizer_m = new wxBoxSizer(wxHORIZONTAL);

	bool inverse_slider = glbin_settings.m_inverse_slider;
	wxBitmapBundle bitmap;
	wxImage image;
	//bar top///////////////////////////////////////////////////
	wxBoxSizer* sizer_h_1 = new wxBoxSizer(wxHORIZONTAL);
	//toolbar 1
	m_mix_mode_tb = new wxUndoableToolbar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	m_mix_mode_tb->SetDoubleBuffered(true);
	wxSize tbs = m_mix_mode_tb->GetBestSize();

	//blend mode
	bitmap = wxGetBitmap(layers);
	m_mix_mode_tb->AddCheckTool(
		ID_ChannelMixLayered, "Layer",
		bitmap, wxNullBitmap,
		"Render View as Layers",
		"Render View as Layers");
	bitmap = wxGetBitmap(depth);
	m_mix_mode_tb->AddCheckTool(
		ID_ChannelMixDepth, "Depth",
		bitmap, wxNullBitmap,
		"Render View by Depth",
		"Render View by Depth");
	bitmap = wxGetBitmap(composite);
	m_mix_mode_tb->AddCheckTool(
		ID_ChannelMixCompositeAdd, "Compo",
		bitmap, wxNullBitmap,
		"Render View as a Composite of Colors",
		"Render View as a Composite of Colors");
	m_mix_mode_tb->Bind(wxEVT_TOOL, &RenderViewPanel::OnChannelMixMode, this);
	m_mix_mode_tb->Realize();

	sizer_h_1->AddSpacer(50);
	sizer_h_1->Add(m_mix_mode_tb, 0, wxALIGN_CENTER);

	//hud
	m_hud_tb = new wxUndoableToolbar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);

	//info
	bitmap = wxGetBitmap(info);
	m_hud_tb->AddCheckTool(
		ID_InfoChk, "Info",
		bitmap, wxNullBitmap,
		"Toggle View of FPS and Mouse Position",
		"Toggle View of FPS and Mouse Position");

	//cam center
	bitmap = wxGetBitmap(axis);
	m_hud_tb->AddCheckTool(
		ID_CamCtrChk, "Axis",
		bitmap, wxNullBitmap,
		"Toggle View of the Center Axis",
		"Toggle View of the Center Axis");

	//legend
	bitmap = wxGetBitmap(legend);
	m_hud_tb->AddCheckTool(
		ID_LegendChk, "Legend",
		bitmap, wxNullBitmap,
		"Toggle View of the Legend",
		"Toggle View of the Legend");

	//colormap
	bitmap = wxGetBitmap(colormap_off);
	m_hud_tb->AddToolWithHelp(
		ID_Colormap, "Colormap", bitmap,
		"Toggle Colormap Legend Options (Off, On, On with text)");

	//scale bar
	bitmap = wxGetBitmap(scalebar);
	m_hud_tb->AddToolWithHelp(
		ID_ScaleBar, "Scale", bitmap,
		"Toggle Scalebar Options (Off, On, On with text)");
	m_hud_tb->Bind(wxEVT_TOOL, &RenderViewPanel::OnHud, this);
	m_hud_tb->Realize();

	sizer_h_1->AddSpacer(10);
	sizer_h_1->Add(m_hud_tb, 0, wxALIGN_CENTER);

	m_scale_text = new wxTextCtrl(this, wxID_ANY, "50",
		wxDefaultPosition, FromDIP(wxSize(35, 20)), wxTE_RIGHT, vald_int);
	m_scale_cmb = new wxComboBox(this, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(100, 20)), 0, NULL, wxCB_READONLY);
	std::vector<wxString> scale_list = { "nm", L"\u03BCm", "mm" };
	m_scale_cmb->Append(scale_list);
	m_scale_text->Bind(wxEVT_TEXT, &RenderViewPanel::OnScaleText, this);
	m_scale_cmb->Bind(wxEVT_TEXT, &RenderViewPanel::OnScaleUnit, this);
	sizer_h_1->Add(m_scale_text, 0, wxALIGN_CENTER);
	sizer_h_1->Add(m_scale_cmb, 0, wxALIGN_CENTER);

	//background
	m_bg_color_picker = new wxUndoableColorPicker(this,
		wxID_ANY, *wxBLACK, wxDefaultPosition, FromDIP(wxSize(40, 20)));
	wxSize bs = m_bg_color_picker->GetSize();
	m_bg_inv_btn = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	m_bg_inv_btn->SetDoubleBuffered(true);
	bitmap = wxGetBitmap(invert);
	m_bg_inv_btn->AddCheckTool(
		0, "Invert",
		bitmap, wxNullBitmap,
		"Invert background color",
		"Invert background color");
	m_bg_inv_btn->Realize();
	m_bg_color_picker->Bind(wxEVT_COLOURPICKER_CHANGED, &RenderViewPanel::OnBgColorChange, this);
	m_bg_inv_btn->Bind(wxEVT_TOOL, &RenderViewPanel::OnBgInvBtn, this);
	sizer_h_1->AddSpacer(10);
	sizer_h_1->Add(m_bg_inv_btn, 0, wxALIGN_CENTER);
	sizer_h_1->Add(m_bg_color_picker, 0, wxALIGN_CENTER);

	//capture
	bitmap = wxGetBitmap(snap);
	m_snapshot_btn = new wxButton(this, wxID_ANY, "Snapshot");
	m_snapshot_btn->SetBitmap(bitmap);
	m_snapshot_btn->SetBitmapPosition(wxLEFT);
	m_snapshot_btn->SetToolTip("Take a snapshot of the Render View as an image");
	m_snapshot_btn->Bind(wxEVT_BUTTON, &RenderViewPanel::OnSnapshotBtn, this);
	sizer_h_1->AddSpacer(10);
	sizer_h_1->Add(m_snapshot_btn, 0, wxALIGN_CENTER);

	sizer_h_1->AddStretchSpacer(1);

	//cam
	m_view_manip_btn = new wxUndoableToolbar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	m_view_manip_btn->SetDoubleBuffered(true);
	bitmap = wxGetBitmap(camera);
	m_view_manip_btn->AddToolWithHelp(
		0, "Camera Manipulation", bitmap,
		"Set the mouse interaction mode to camera manipulation");
	m_view_manip_btn->Realize();
	m_view_manip_btn->Bind(wxEVT_TOOL, &RenderViewPanel::OnViewManipBtn, this);
	//angle of view
	m_aov_sldr = new wxSingleSlider(this, wxID_ANY, 45, 10, 100,
		wxDefaultPosition, FromDIP(wxSize(100, 20)), wxSL_HORIZONTAL);
	m_aov_text = new wxNumTextCtrl(this, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(40, 20)), wxTE_RIGHT, vald_int);
	m_aov_sldr->Bind(wxEVT_IDLE, &RenderViewPanel::OnAovSldrIdle, this);
	m_aov_sldr->Bind(wxEVT_SCROLL_CHANGED, &RenderViewPanel::OnAovChange, this);
	m_aov_text->Bind(wxEVT_TEXT, &RenderViewPanel::OnAovText, this);
	sizer_h_1->Add(m_view_manip_btn, 0, wxALIGN_CENTER);
	sizer_h_1->Add(m_aov_sldr, 0, wxALIGN_CENTER);
	sizer_h_1->Add(m_aov_text, 0, wxALIGN_CENTER);

	//free fly
	m_cam_op_tb = new wxUndoableToolbar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	m_cam_op_tb->SetDoubleBuffered(true);
	bitmap = wxGetBitmap(ortho);
	m_cam_op_tb->AddToolWithHelp(
		ID_OrthoPerspBtn, "Camera Projection", bitmap,
		"Change camera projection between orthographic and perspective");
	bitmap = wxGetBitmap(globe);
	m_cam_op_tb->AddToolWithHelp(
		ID_CamModeBtn, "Camera operation mode", bitmap,
		"Change camera operation mode between globe and flight");
	//save default
	bitmap = wxGetBitmap(save_settings);
	m_cam_op_tb->AddToolWithHelp(
		ID_DefaultBtn, "Save", bitmap,
		"Set Default Render View Settings");
	m_cam_op_tb->Bind(wxEVT_TOOL, &RenderViewPanel::OnToolBar2, this);
	m_cam_op_tb->Realize();
	sizer_h_1->Add(m_cam_op_tb, 0, wxALIGN_CENTER);

	sizer_h_1->AddSpacer(10);
	//full screen
	m_full_screen_toolbar = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_FLAT|wxTB_NODIVIDER);
	m_full_screen_toolbar->SetDoubleBuffered(true);
	//vr
	bitmap = wxGetBitmap(vr);
	m_full_screen_toolbar->AddCheckTool(
		ID_VrChk, "Stereography",
		bitmap, wxNullBitmap,
		"Enable stereography",
		"Enable stereography");
	//looking glass
	bitmap = wxGetBitmap(looking_glass);
	m_full_screen_toolbar->AddCheckTool(
		ID_LookingGlassChk, "Holography",
		bitmap, wxNullBitmap,
		"Enable holography",
		"Enable holography");
	//full screen
	bitmap = wxGetBitmap(full_view);
	m_full_screen_toolbar->AddTool(
		ID_FullScreenBtn, "Full Screen",
		bitmap, "Show full screen");
	m_full_screen_toolbar->SetToolLongHelp(ID_FullScreenBtn, "Show full screen");
	m_full_screen_toolbar->Bind(wxEVT_TOOL, &RenderViewPanel::OnFullScreenToolbar, this);
	m_full_screen_toolbar->Realize();
	sizer_h_1->Add(m_full_screen_toolbar, 0, wxALIGN_CENTER);
	sizer_h_1->AddSpacer(50);

	//bar left///////////////////////////////////////////////////
	wxBoxSizer* sizer_v_3 = new wxBoxSizer(wxVERTICAL);
	//depth attenuation
	m_depth_atten_btn = new wxUndoableToolbar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	m_depth_atten_btn->SetDoubleBuffered(true);
	bitmap = wxGetBitmap(no_depth_atten);
	m_depth_atten_btn->AddCheckTool(0, "Depth Interval",
		bitmap, wxNullBitmap,
		"Enable adjustment of the Depth Attenuation Interval",
		"Enable adjustment of the Depth Attenuation Interval");
	m_depth_atten_btn->Bind(wxEVT_TOOL, &RenderViewPanel::OnDepthAttenCheck, this);
	m_depth_atten_btn->Realize();
	//slider
	long ls = inverse_slider ? wxSL_VERTICAL : (wxSL_VERTICAL | wxSL_INVERSE);
	m_depth_atten_factor_sldr = new wxSingleSlider(this, wxID_ANY, 0, 0, 100,
		wxDefaultPosition, wxDefaultSize, ls);
	//text
	m_depth_atten_factor_text = new wxNumTextCtrl(this, wxID_ANY, "0.00",
		wxDefaultPosition, FromDIP(wxSize(40, 20)), wxTE_CENTER, vald_fp2);
	m_depth_atten_factor_sldr->Bind(wxEVT_SCROLL_CHANGED, &RenderViewPanel::OnDepthAttenChange, this);
	m_depth_atten_factor_text->Bind(wxEVT_TEXT, &RenderViewPanel::OnDepthAttenEdit, this);
	//reset
	m_depth_atten_reset_btn = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	m_depth_atten_reset_btn->SetDoubleBuffered(true);
	bitmap = wxGetBitmap(reset);
	m_depth_atten_reset_btn->AddTool(
		0, "Reset",
		bitmap, "Reset Depth Attenuation Interval");
	m_depth_atten_reset_btn->SetToolLongHelp(0, "Reset Depth Attenuation Interval");
	m_depth_atten_reset_btn->Bind(wxEVT_TOOL, &RenderViewPanel::OnDepthAttenReset, this);
	m_depth_atten_reset_btn->Realize();
	sizer_v_3->AddSpacer(50);
	sizer_v_3->Add(m_depth_atten_btn, 0, wxALIGN_CENTER);
	sizer_v_3->Add(m_depth_atten_factor_sldr, 1, wxEXPAND);
	sizer_v_3->Add(m_depth_atten_factor_text, 0, wxALIGN_CENTER);
	sizer_v_3->Add(m_depth_atten_reset_btn, 0, wxALIGN_CENTER);
	sizer_v_3->AddSpacer(50);

	//bar right///////////////////////////////////////////////////
	wxBoxSizer* sizer_v_4 = new wxBoxSizer(wxVERTICAL);
	//pin rotation
	m_pin_btn = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	m_pin_btn->SetDoubleBuffered(true);
	bitmap = wxGetBitmap(pin_off);
	m_pin_btn->AddCheckTool(0, "Pin",
		bitmap, wxNullBitmap,
		"Anchor the rotation center on data",
		"Anchor the rotation center on data");
	m_pin_btn->Bind(wxEVT_TOOL, &RenderViewPanel::OnPin, this);
	m_pin_btn->Realize();

	//centerize
	m_center_btn = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	m_center_btn->SetDoubleBuffered(true);
	bitmap = wxGetBitmap(center);
	m_center_btn->AddTool(0, "Center",
		bitmap, "Center the Data on the Render View");
	m_center_btn->SetToolLongHelp(0, "Center the Data on the Render View");
	m_center_btn->Bind(wxEVT_TOOL, &RenderViewPanel::OnCenter, this);
	m_center_btn->Realize();
	m_center_click_btn = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	m_center_click_btn->SetDoubleBuffered(true);
	bitmap = wxGetBitmap(center_click);
	m_center_click_btn->AddCheckTool(0, "Click Center",
		bitmap, wxNullBitmap,
		"Center the Render View to a clicked point on data",
		"Center the Render View to a clicked point on data");
	m_center_click_btn->Bind(wxEVT_TOOL, &RenderViewPanel::OnCenterClick, this);
	m_center_click_btn->Realize();

	//one to one scale
	m_scale_121_btn = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	m_scale_121_btn->SetDoubleBuffered(true);
	bitmap = wxGetBitmap(ratio);
	m_scale_121_btn->AddTool(0, "1 to 1",
		bitmap, "Auto-size the data to a 1:1 ratio");
	m_scale_121_btn->SetToolLongHelp(0, "Auto-size the data to a 1:1 ratio");
	m_scale_121_btn->Bind(wxEVT_TOOL, &RenderViewPanel::OnScale121, this);
	m_scale_121_btn->Realize();

	//scale slider
	ls = inverse_slider ? wxSL_VERTICAL : (wxSL_VERTICAL | wxSL_INVERSE);
	m_scale_factor_sldr = new wxSingleSlider(this, wxID_ANY, 100, 50, 999,
		wxDefaultPosition, wxDefaultSize, ls, wxDefaultValidator, "test");
	m_scale_factor_text = new wxNumTextCtrl(this, wxID_ANY, "100",
		wxDefaultPosition, FromDIP(wxSize(40, 20)), wxTE_CENTER, vald_int);
	m_scale_factor_spin = new wxSpinButton(this, wxID_ANY,
		wxDefaultPosition, FromDIP(wxSize(40, 20)));
	m_scale_factor_spin->SetRange(-0x8000, 0x7fff);
	m_scale_factor_sldr->Bind(wxEVT_SCROLL_CHANGED, &RenderViewPanel::OnScaleFactorChange, this);
	m_scale_factor_text->Bind(wxEVT_TEXT, &RenderViewPanel::OnScaleFactorEdit, this);
	m_scale_factor_spin->Bind(wxEVT_SPIN_UP, &RenderViewPanel::OnScaleFactorSpinDown, this);
	m_scale_factor_spin->Bind(wxEVT_SPIN_DOWN, &RenderViewPanel::OnScaleFactorSpinUp, this);
	m_scale_reset_btn = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	m_scale_reset_btn->SetDoubleBuffered(true);
	bitmap = wxGetBitmap(reset);
	m_scale_reset_btn->AddTool(0, "Reset",
		bitmap, "Reset the Zoom");
	m_scale_reset_btn->SetToolLongHelp(0, "Reset the Zoom");
	m_scale_reset_btn->Bind(wxEVT_TOOL, &RenderViewPanel::OnScaleReset, this);
	m_scale_reset_btn->Realize();
	m_scale_mode_btn = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	m_scale_mode_btn->SetDoubleBuffered(true);
	bitmap = wxGetBitmap(zoom_view);
	m_scale_mode_btn->AddTool(
		0, "Switch zoom ratio mode",
		bitmap, "View-based zoom ratio");
	m_scale_mode_btn->SetToolLongHelp(0, "View-based zoom ratio");
	m_scale_mode_btn->Bind(wxEVT_TOOL, &RenderViewPanel::OnScaleMode, this);
	m_scale_mode_btn->Realize();
	sizer_v_4->AddSpacer(50);
	sizer_v_4->Add(m_pin_btn, 0, wxALIGN_CENTER);
	sizer_v_4->Add(m_center_click_btn, 0, wxALIGN_CENTER);
	sizer_v_4->Add(m_center_btn, 0, wxALIGN_CENTER);
	sizer_v_4->Add(m_scale_121_btn, 0, wxALIGN_CENTER);
	sizer_v_4->Add(m_scale_factor_sldr, 1, wxEXPAND);
	sizer_v_4->Add(m_scale_factor_spin, 0, wxALIGN_CENTER);
	sizer_v_4->Add(m_scale_factor_text, 0, wxALIGN_CENTER);
	sizer_v_4->Add(m_scale_mode_btn, 0, wxALIGN_CENTER);
	sizer_v_4->Add(m_scale_reset_btn, 0, wxALIGN_CENTER);
	sizer_v_4->AddSpacer(50);

	//middle sizer
	sizer_m->Add(sizer_v_3, 0, wxEXPAND);
	sizer_m->Add(m_view_sizer, 1, wxEXPAND);
	sizer_m->Add(sizer_v_4, 0, wxEXPAND);

	//bar bottom///////////////////////////////////////////////////
	wxBoxSizer* sizer_h_2 = new wxBoxSizer(wxHORIZONTAL);
	//45 lock
	m_slider_mode_btn = new wxUndoableToolbar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	m_slider_mode_btn->SetDoubleBuffered(true);
	bitmap = wxGetBitmap(slider);
	m_slider_mode_btn->AddToolWithHelp(
		0, "Slider Style", bitmap,
		"Choose slider style between jog and normal");
	m_slider_mode_btn->Bind(wxEVT_TOOL, &RenderViewPanel::OnRotSliderMode, this);
	m_slider_mode_btn->Realize();

	m_x_rot_sldr = new wxUndoableScrollBar(this, ID_RotXScroll);
	m_x_rot_sldr->SetScrollbar2(180, 60, 0, 360, 15);
	m_x_rot_text = new wxNumTextCtrl(this, wxID_ANY, "0.0",
		wxDefaultPosition, FromDIP(wxSize(45,20)), wxTE_RIGHT, vald_fp1);
	m_y_rot_sldr = new wxUndoableScrollBar(this, ID_RotYScroll);
	m_y_rot_sldr->SetScrollbar2(180, 60, 0, 360, 15);
	m_y_rot_text = new wxNumTextCtrl(this, wxID_ANY, "0.0",
		wxDefaultPosition, FromDIP(wxSize(45,20)), wxTE_RIGHT, vald_fp1);
	m_z_rot_sldr = new wxUndoableScrollBar(this, ID_RotZScroll);
	m_z_rot_sldr->SetScrollbar2(180, 60, 0, 360, 15);
	m_z_rot_text = new wxNumTextCtrl(this, wxID_ANY, "0.0",
		wxDefaultPosition, FromDIP(wxSize(45,20)), wxTE_RIGHT, vald_fp1);
	m_x_rot_text->Bind(wxEVT_TEXT, &RenderViewPanel::OnRotEdit, this);
	m_y_rot_text->Bind(wxEVT_TEXT, &RenderViewPanel::OnRotEdit, this);
	m_z_rot_text->Bind(wxEVT_TEXT, &RenderViewPanel::OnRotEdit, this);
	m_x_rot_sldr->Bind(wxEVT_SCROLL_CHANGED, &RenderViewPanel::OnRotScroll, this);
	m_y_rot_sldr->Bind(wxEVT_SCROLL_CHANGED, &RenderViewPanel::OnRotScroll, this);
	m_z_rot_sldr->Bind(wxEVT_SCROLL_CHANGED, &RenderViewPanel::OnRotScroll, this);

	//ortho view selector
	m_ortho_view_cmb = new wxComboBox(this, wxID_ANY, "",
		wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY);
	std::vector<wxString> ov_list = { "XY/Front", "XY/Back", "XZ/Top", "XZ/Bottom", "YZ/Left", "YZ/Right", "Free" };
	m_ortho_view_cmb->Append(ov_list);
	m_ortho_view_cmb->Bind(wxEVT_COMBOBOX, &RenderViewPanel::OnOrthoViewSelected, this);

	//set reset
	m_rot_btn = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	m_rot_btn->SetDoubleBuffered(true);
	bitmap = wxGetBitmap(gear_dark);
	m_rot_btn->AddCheckTool(ID_RotLockChk, "45 Angles",
		bitmap, wxNullBitmap,
		"Confine all angles to 45 Degrees",
		"Confine all angles to 45 Degrees");
	bitmap = wxGetBitmap(zrot);
	m_rot_btn->AddTool(ID_ZeroRotBtn, "Set Zeros",
		bitmap, "Set current angles as zeros");
	m_rot_btn->SetToolLongHelp(ID_ZeroRotBtn, "Set current angles as zeros");
	bitmap = wxGetBitmap(reset);
	m_rot_btn->AddTool(ID_RotResetBtn,"Reset",
		bitmap, "Reset Rotations");
	m_rot_btn->SetToolLongHelp(ID_RotResetBtn, "Reset Rotations");
	m_rot_btn->Bind(wxEVT_TOOL, &RenderViewPanel::OnRotSettings, this);
	m_rot_btn->Realize();

	sizer_h_2->AddSpacer(50);
	sizer_h_2->Add(m_slider_mode_btn, 0, wxALIGN_CENTER);
	sizer_h_2->Add(new wxStaticText(this, 0, "X:"), 0, wxALIGN_CENTER);
	sizer_h_2->Add(m_x_rot_sldr, 1, wxALIGN_CENTER);
	sizer_h_2->Add(m_x_rot_text, 0, wxALIGN_CENTER);
	sizer_h_2->Add(5, 5, 0);
	sizer_h_2->Add(new wxStaticText(this, 0, "Y:"), 0, wxALIGN_CENTER);
	sizer_h_2->Add(m_y_rot_sldr, 1, wxALIGN_CENTER);
	sizer_h_2->Add(m_y_rot_text, 0, wxALIGN_CENTER);
	sizer_h_2->Add(5, 5, 0);
	sizer_h_2->Add(new wxStaticText(this, 0, "Z:"), 0, wxALIGN_CENTER);
	sizer_h_2->Add(m_z_rot_sldr, 1, wxALIGN_CENTER);
	sizer_h_2->Add(m_z_rot_text, 0, wxALIGN_CENTER);
	sizer_h_2->Add(5, 5, 0);
	sizer_h_2->Add(m_ortho_view_cmb, 0, wxALIGN_CENTER, 2);
	sizer_h_2->Add(m_rot_btn, 0, wxALIGN_CENTER);
	sizer_h_2->AddSpacer(50);

	sizer_v->Add(sizer_h_1, 0, wxEXPAND);
	sizer_v->Add(sizer_m, 1, wxEXPAND);
	sizer_v->Add(sizer_h_2, 0, wxEXPAND);

	SetSizer(sizer_v);
	Layout();
	SetAutoLayout(true);
	SetScrollRate(10, 10);
}

void RenderViewPanel::UpdateMixMethod(ChannelMixMode mode)
{
	m_mix_mode_tb->ToggleTool(ID_ChannelMixLayered, mode == ChannelMixMode::Layered);
	m_mix_mode_tb->SetToolNormalBitmap(ID_ChannelMixLayered,
		mode == ChannelMixMode::Layered ?
		wxGetBitmap(layers) :
		wxGetBitmap(layers_off));
	m_mix_mode_tb->ToggleTool(ID_ChannelMixDepth, mode == ChannelMixMode::Depth);
	m_mix_mode_tb->SetToolNormalBitmap(ID_ChannelMixDepth,
		mode == ChannelMixMode::Depth ?
		wxGetBitmap(depth) :
		wxGetBitmap(depth_off));
	m_mix_mode_tb->ToggleTool(ID_ChannelMixCompositeAdd, mode == ChannelMixMode::CompositeAdd);
	m_mix_mode_tb->SetToolNormalBitmap(ID_ChannelMixCompositeAdd,
		mode == ChannelMixMode::CompositeAdd ?
		wxGetBitmap(composite) :
		wxGetBitmap(composite_off));
}

void RenderViewPanel::UpdateDrawInfo(bool bval)
{
	m_hud_tb->ToggleTool(ID_InfoChk, bval);
}

void RenderViewPanel::UpdateDrawCamCtr(bool bval)
{
	m_hud_tb->ToggleTool(ID_CamCtrChk, bval);
}

void RenderViewPanel::UpdateDrawLegend(bool bval)
{
	m_hud_tb->ToggleTool(ID_LegendChk, bval);
}

void RenderViewPanel::UpdateDrawColormap(int ival)
{
	wxBitmapBundle colormap_bmp;
	switch (ival)
	{
	case 0:
	default:
		colormap_bmp = wxGetBitmap(colormap_off);
		break;
	case 1:
		colormap_bmp = wxGetBitmap(colormap);
		break;
	case 2:
		colormap_bmp = wxGetBitmap(colormap_text);
		break;
	}
	m_hud_tb->SetToolNormalBitmap(ID_Colormap, colormap_bmp);
}

void RenderViewPanel::UpdateDrawScalebar(int ival)
{
	switch (ival)
	{
	case 0:
	default:
		m_hud_tb->SetToolNormalBitmap(ID_ScaleBar,
			wxGetBitmap(scalebar));
		m_scale_text->Disable();
		m_scale_cmb->Disable();
		break;
	case 1:
		m_hud_tb->SetToolNormalBitmap(ID_ScaleBar,
			wxGetBitmap(scale_text_off));
		m_scale_text->Enable();
		m_scale_cmb->Disable();
		break;
	case 2:
		m_hud_tb->SetToolNormalBitmap(ID_ScaleBar,
			wxGetBitmap(scale_text));
		m_scale_text->Enable();
		m_scale_cmb->Enable();
		break;
	}
}

void RenderViewPanel::UpdateScaleBarValue(double dval)
{
	m_scale_text->SetValue(wxString::Format("%.0f", dval));
}

void RenderViewPanel::UpdateScaleBarUnit(int ival)
{
	m_scale_cmb->Select(ival);
}

void RenderViewPanel::UpdateBgColor(const fluo::Color& c)
{
	wxColor wxc((unsigned char)(c.r() * 255 + 0.5),
		(unsigned char)(c.g() * 255 + 0.5),
		(unsigned char)(c.b() * 255 + 0.5));
	m_bg_color_picker->SetColour(wxc);
}

void RenderViewPanel::UpdateBgColorInvert(bool bval)
{
	m_bg_inv_btn->ToggleTool(0, bval);
	if (bval)
		m_bg_inv_btn->SetToolNormalBitmap(0,
			wxGetBitmap(invert));
	else
		m_bg_inv_btn->SetToolNormalBitmap(0,
			wxGetBitmap(invert_off));
}

void RenderViewPanel::UpdateAov(int ival, bool bval)
{
	m_aov_sldr->ChangeValue(bval ? ival : 10);
	m_aov_text->ChangeValue(bval ? std::to_string(ival) : "Ortho");
	if (bval)
		m_cam_op_tb->SetToolNormalBitmap(ID_OrthoPerspBtn,
			wxGetBitmap(persp));
	else
		m_cam_op_tb->SetToolNormalBitmap(ID_OrthoPerspBtn,
			wxGetBitmap(ortho));
}

void RenderViewPanel::UpdateCamMode(int ival)
{
	switch (ival)
	{
	case 0:
		m_cam_op_tb->SetToolNormalBitmap(ID_CamModeBtn,
			wxGetBitmap(globe));
		break;
	case 1:
		m_cam_op_tb->SetToolNormalBitmap(ID_CamModeBtn,
			wxGetBitmap(flight));
		break;
	}
}

void RenderViewPanel::UpdateHologramMode(int ival)
{
	m_full_screen_toolbar->ToggleTool(ID_VrChk, ival == 1);
	m_full_screen_toolbar->ToggleTool(ID_LookingGlassChk, ival == 2);
}

void RenderViewPanel::UpdateFreehandToolState(bool bval)
{
	m_center_click_btn->ToggleTool(0, bval);
}

void RenderViewPanel::UpdateDepthAtten(bool bval)
{
	m_depth_atten_btn->ToggleTool(0, bval);
	if (bval)
		m_depth_atten_btn->SetToolNormalBitmap(0,
			wxGetBitmap(depth_atten));
	else
		m_depth_atten_btn->SetToolNormalBitmap(0,
			wxGetBitmap(no_depth_atten));
	m_depth_atten_factor_sldr->Enable(bval);
	m_depth_atten_factor_text->Enable(bval);
}

void RenderViewPanel::UpdateDepthAttenFactor(double dval)
{
	m_depth_atten_factor_sldr->ChangeValue(std::round(dval * 100));
	m_depth_atten_factor_text->ChangeValue(wxString::Format("%.2f", dval));
}

void RenderViewPanel::UpdateScaleFactor(int ival)
{
	m_scale_factor_sldr->ChangeValue(ival);
	m_scale_factor_text->ChangeValue(wxString::Format("%d", ival));
	m_scale_factor_text->Update();
}

void RenderViewPanel::UpdateScaleMode(int ival)
{
	switch (ival)
	{
	case 0:
		m_scale_mode_btn->SetToolNormalBitmap(0,
			wxGetBitmap(zoom_view));
		m_scale_mode_btn->SetToolShortHelp(0,
			"View-based zoom ratio");
		m_scale_mode_btn->SetToolLongHelp(0,
			"View-based zoom ratio (View entire data set at 100%)");
		break;
	case 1:
		m_scale_mode_btn->SetToolNormalBitmap(0,
			wxGetBitmap(zoom_pixel));
		m_scale_mode_btn->SetToolShortHelp(0,
			"Pixel-based zoom ratio");
		m_scale_mode_btn->SetToolLongHelp(0,
			"Pixel-based zoom ratio (View 1 data pixel to 1 screen pixel at 100%)");
		break;
	case 2:
		m_scale_mode_btn->SetToolNormalBitmap(0,
			wxGetBitmap(zoom_data));
		m_scale_mode_btn->SetToolShortHelp(0,
			"Data-based zoom ratio");
		m_scale_mode_btn->SetToolLongHelp(0,
			"Data-based zoom ratio (View with consistent scale bar sizes)");
		break;
	}
}

void RenderViewPanel::UpdatePinRotCenter(bool bval)
{
	m_pin_btn->ToggleTool(0, bval);
	if (bval)
		m_pin_btn->SetToolNormalBitmap(0,
			wxGetBitmap(pin));
	else
		m_pin_btn->SetToolNormalBitmap(0,
			wxGetBitmap(pin_off));
}

void RenderViewPanel::UpdateGearedEnable(bool bval)
{
	m_rot_btn->ToggleTool(ID_RotLockChk, bval);
	if (bval)
		m_rot_btn->SetToolNormalBitmap(ID_RotLockChk,
			wxGetBitmap(gear_45));
	else
		m_rot_btn->SetToolNormalBitmap(ID_RotLockChk,
			wxGetBitmap(gear_dark));
}

void RenderViewPanel::UpdateRotSliderMode(bool bval)
{
	m_slider_mode_btn->ToggleTool(0, bval);
	if (bval)
	{
		m_slider_mode_btn->SetToolNormalBitmap(0,
			wxGetBitmap(jog));
		if (m_x_rot_sldr->GetMode() != 1)
		{
			m_x_rot_sldr->SetMode(1);
			m_y_rot_sldr->SetMode(1);
			m_z_rot_sldr->SetMode(1);
		}
	}
	else
	{
		m_slider_mode_btn->SetToolNormalBitmap(0,
			wxGetBitmap(slider));
		if (m_x_rot_sldr->GetMode() != 0)
		{
			m_x_rot_sldr->SetMode(0);
			m_y_rot_sldr->SetMode(0);
			m_z_rot_sldr->SetMode(0);
		}
	}
}

void RenderViewPanel::UpdateCamRotation(const fluo::Vector& val, int ival)
{
	m_x_rot_sldr->ChangeValue(static_cast<int>(std::round(val.x())));
	m_y_rot_sldr->ChangeValue(static_cast<int>(std::round(val.y())));
	m_z_rot_sldr->ChangeValue(static_cast<int>(std::round(val.z())));
	m_x_rot_text->ChangeValue(wxString::Format("%.1f", val.x()));
	m_y_rot_text->ChangeValue(wxString::Format("%.1f", val.y()));
	m_z_rot_text->ChangeValue(wxString::Format("%.1f", val.z()));
	m_x_rot_text->Update();
	m_y_rot_text->Update();
	m_z_rot_text->Update();
	m_ortho_view_cmb->Select(ival);
}

//get rendering context
wxGLContext* RenderViewPanel::GetContext()
{
	if (m_canvas)
		return m_canvas->m_glRC/*GetContext()*/;
	else
		return 0;
}

bool RenderViewPanel::SetFullScreen()
{
	if (m_canvas->GetParent() != m_full_frame)
	{
		m_view_sizer->Detach(m_canvas);
		m_view_sizer->AddStretchSpacer();
		m_canvas->Reparent(m_full_frame);
		//get display id
		unsigned int disp_id = glbin_settings.m_disp_id;
		if (disp_id >= wxDisplay::GetCount())
			disp_id = 0;
		wxDisplay display(disp_id);
		wxRect rect = display.GetGeometry();
		m_full_frame->SetSize(rect.GetSize());
		wxPoint pos = rect.GetPosition();
#ifdef _DARWIN
		pos -= wxPoint(0, 10);
#endif
		m_full_frame->SetPosition(pos);
#ifdef _WIN32
		m_full_frame->ShowFullScreen(true);
#endif
		m_canvas->SetPosition(wxPoint(0, 0));
		m_canvas->SetSize(m_full_frame->GetSize());
		if (glbin_settings.m_stay_top)
			m_full_frame->SetWindowStyle(wxBORDER_NONE | wxSTAY_ON_TOP);
		else
			m_full_frame->SetWindowStyle(wxBORDER_NONE);
#ifdef _WIN32
		if (!glbin_settings.m_show_cursor)
			ShowCursor(false);
#endif
		m_full_frame->Iconize(false);
		m_full_frame->Raise();
		m_full_frame->Show();
		m_canvas->m_full_screen = true;
		m_canvas->SetFocus();
		return true;
	}

	m_canvas->Close();
	return false;
}

void RenderViewPanel::CloseFullScreen()
{
	if (m_canvas->GetParent() == m_full_frame)
		m_canvas->Close();
}

void RenderViewPanel::FocusCanvas()
{
	m_canvas->SetFocus();
}

ChannelMixMode RenderViewPanel::GetChannelMixMethod()
{
	return m_channel_mix_mode;
}

bool RenderViewPanel::GetInfo()
{
	return m_hud_tb->GetToolState(ID_InfoChk);
}

bool RenderViewPanel::GetCamCtr()
{
	return m_hud_tb->GetToolState(ID_CamCtrChk);
}

bool RenderViewPanel::GetLegend()
{
	return m_hud_tb->GetToolState(ID_LegendChk);
}

double RenderViewPanel::GetScalebarValue()
{
	wxString str = m_scale_text->GetValue();
	double dval;
	if (str.ToDouble(&dval))
		return dval;
	return 0.0;
}

int RenderViewPanel::GetScalebarUnit()
{
	return m_scale_cmb->GetSelection();
}

fluo::Color RenderViewPanel::GetBgColor()
{
	wxColor c = m_bg_color_picker->GetColour();
	return fluo::Color(c.Red() / 255.0, c.Green() / 255.0, c.Blue() / 255.0);
}

bool RenderViewPanel::GetMouseInAovSldr()
{
	wxPoint pos = wxGetMousePosition();
	wxRect reg = m_aov_sldr->GetScreenRect();
	wxWindow* window = wxWindow::FindFocus();
	return window && reg.Contains(pos);
}

int RenderViewPanel::GetAov()
{
	wxString str = m_aov_text->GetValue();
	int ival = 10;
	long val;
	if (str.ToLong(&val))
		ival = val;
	return ival;
}

bool RenderViewPanel::GetDepthAttenEnable()
{
	return m_depth_atten_btn->GetToolState(0);
}

double RenderViewPanel::GetDepthAttenValue()
{
	wxString str = m_depth_atten_factor_text->GetValue();
	double val = 0.0;
	if (str.ToDouble(&val))
		return val;
	return 0.0;
}

bool RenderViewPanel::GetPin()
{
	return m_pin_btn->GetToolState(0);
}

double RenderViewPanel::GetScaleFactor()
{
	wxString str = m_scale_factor_text->GetValue();
	long val = 0;
	if (str.ToLong(&val) && val > 0)
		return val / 100.0;
	return 1.0;
}

fluo::Vector RenderViewPanel::GetRotations()
{
	wxString str;
	double rotx = 0, roty = 0, rotz = 0;
	str = m_x_rot_text->GetValue();
	str.ToDouble(&rotx);
	str = m_y_rot_text->GetValue();
	str.ToDouble(&roty);
	str = m_z_rot_text->GetValue();
	str.ToDouble(&rotz);
	return fluo::Vector(rotx, roty, rotz);
}

fluo::Vector RenderViewPanel::GetRotationsScroll()
{
	double rotx, roty, rotz;
	rotx = m_x_rot_sldr->GetValue();
	roty = m_y_rot_sldr->GetValue();
	rotz = m_z_rot_sldr->GetValue();
	return fluo::Vector(rotx, roty, rotz);
}

int RenderViewPanel::GetOrthoView()
{
	return m_ortho_view_cmb->GetSelection();
}

void RenderViewPanel::OnChannelMixMode(wxCommandEvent& event)
{
	int id = event.GetId();

	switch (id)
	{
	case ID_ChannelMixLayered:
		m_channel_mix_mode = ChannelMixMode::Layered;
		break;
	case ID_ChannelMixDepth:
		m_channel_mix_mode = ChannelMixMode::Depth;
		break;
	case ID_ChannelMixCompositeAdd:
		m_channel_mix_mode = ChannelMixMode::CompositeAdd;
		break;
	}

	auto agent = m_agent->As<RenderViewPanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstMixMethod });
}

void RenderViewPanel::OnHud(wxCommandEvent& event)
{
	int id = event.GetId();

	fluo::ValueCollection vc;

	switch (id)
	{
	case ID_InfoChk:
		vc.insert(gstDrawInfo);
		break;
	case ID_CamCtrChk:
		vc.insert(gstDrawCamCtr);
		break;
	case ID_LegendChk:
		vc.insert(gstDrawLegend);
		break;
	case ID_Colormap:
		vc.insert(gstDrawColormap);
		break;
	case ID_ScaleBar:
		vc.insert(gstDrawScaleBar);
		break;
	}

	auto agent = m_agent->As<RenderViewPanelAgent>();
	if (agent)
		agent->UpdateUIToData(vc);
}

void RenderViewPanel::OnScaleText(wxCommandEvent& event)
{
	auto agent = m_agent->As<RenderViewPanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstScaleBarLen });
}

void RenderViewPanel::OnScaleUnit(wxCommandEvent& event)
{
	auto agent = m_agent->As<RenderViewPanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstScaleBarUnit });
}

void RenderViewPanel::OnBgColorChange(wxColourPickerEvent& event)
{
	auto agent = m_agent->As<RenderViewPanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstBgColor });
}

void RenderViewPanel::OnBgInvBtn(wxCommandEvent& event)
{
	auto agent = m_agent->As<RenderViewPanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstBgColorInv });
}

void RenderViewPanel::OnSnapshotBtn(wxCommandEvent& event)
{
	auto agent = m_agent->As<RenderViewPanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstCapture });
}

void RenderViewPanel::OnViewManipBtn(wxCommandEvent& event)
{
	auto agent = m_agent->As<RenderViewPanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstViewManip });
}

void RenderViewPanel::OnAovSldrIdle(wxIdleEvent& event)
{
	auto agent = m_agent->As<RenderViewPanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstMouseInAovSldr });
}

void RenderViewPanel::OnAovChange(wxScrollEvent& event)
{
	int ival = m_aov_sldr->GetValue();
	m_aov_text->SetValue(wxString::Format("%d", ival));
}

void RenderViewPanel::OnAovText(wxCommandEvent& event)
{
	auto agent = m_agent->As<RenderViewPanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstAov });
}

void RenderViewPanel::OnToolBar2(wxCommandEvent& event)
{
	fluo::ValueCollection vc;
	int id = event.GetId();
	switch (id)
	{
	case ID_OrthoPerspBtn:
		//toggle between ortho and perspective
		vc.insert(gstProjection);
		break;
	case ID_CamModeBtn:
		vc.insert(gstCamMode);
		break;
	case ID_DefaultBtn:
		vc.insert(gstSaveViewDefault);
		break;
	}
	auto agent = m_agent->As<RenderViewPanelAgent>();
	if (agent)
		agent->UpdateUIToData(vc);
}

void RenderViewPanel::OnFullScreenToolbar(wxCommandEvent& event)
{
	fluo::ValueCollection vc;
	int id = event.GetId();
	switch (id)
	{
	case ID_VrChk:
		vc.insert(gstStereography);
		break;
	case ID_LookingGlassChk:
		vc.insert(gstHolography);
		break;
	case ID_FullScreenBtn:
		vc.insert(gstFullScreen);
		break;
	}
	auto agent = m_agent->As<RenderViewPanelAgent>();
	if (agent)
		agent->UpdateUIToData(vc);
}

void RenderViewPanel::OnDepthAttenCheck(wxCommandEvent& event)
{
	auto agent = m_agent->As<RenderViewPanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstDepthAtten });
}

//bar left
void RenderViewPanel::OnDepthAttenChange(wxScrollEvent& event)
{
	double val = m_depth_atten_factor_sldr->GetValue() / 100.0;
	m_depth_atten_factor_text->SetValue(wxString::Format("%.2f", val));
	m_depth_atten_factor_text->Update();
}

void RenderViewPanel::OnDepthAttenEdit(wxCommandEvent& event)
{
	auto agent = m_agent->As<RenderViewPanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstDaInt });
}

void RenderViewPanel::OnDepthAttenReset(wxCommandEvent& event)
{
	auto agent = m_agent->As<RenderViewPanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstDepthAttenReset });
}

void RenderViewPanel::OnPin(wxCommandEvent& event)
{
	auto agent = m_agent->As<RenderViewPanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstViewPin });
}

void RenderViewPanel::OnCenter(wxCommandEvent& event)
{
	auto agent = m_agent->As<RenderViewPanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstCenter });
}

void RenderViewPanel::OnCenterClick(wxCommandEvent& event)
{
	auto agent = m_agent->As<RenderViewPanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstClickCenter });
}

void RenderViewPanel::OnScale121(wxCommandEvent& event)
{
	auto agent = m_agent->As<RenderViewPanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstScaleFactor121 });
}

void RenderViewPanel::OnScaleFactorChange(wxScrollEvent& event)
{
	int ival = m_scale_factor_sldr->GetValue();
	m_scale_factor_text->SetValue(wxString::Format("%d", ival));
}

void RenderViewPanel::OnScaleFactorEdit(wxCommandEvent& event)
{
	auto agent = m_agent->As<RenderViewPanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstScaleFactor });
}

void RenderViewPanel::OnScaleFactorSpinUp(wxSpinEvent& event)
{
	wxString str_val = m_scale_factor_text->GetValue();
	long val;
	str_val.ToLong(&val);
	if (glbin_settings.m_inverse_slider)
		val++;
	else
		val--;
	if (val > 0)
		m_scale_factor_text->SetValue(wxString::Format("%d", val));
}

void RenderViewPanel::OnScaleFactorSpinDown(wxSpinEvent& event)
{
	wxString str_val = m_scale_factor_text->GetValue();
	long val;
	str_val.ToLong(&val);
	if (glbin_settings.m_inverse_slider)
		val--;
	else
		val++;
	if (val > 0)
		m_scale_factor_text->SetValue(wxString::Format("%d", val));
}

void RenderViewPanel::OnScaleReset(wxCommandEvent& event)
{
	auto agent = m_agent->As<RenderViewPanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstScaleFactorReset });
}

void RenderViewPanel::OnScaleMode(wxCommandEvent& event)
{
	auto agent = m_agent->As<RenderViewPanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstScaleMode });
}

void RenderViewPanel::OnRotSliderMode(wxCommandEvent& event)
{
	auto agent = m_agent->As<RenderViewPanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstRotSliderMode });
}

void RenderViewPanel::OnRotEdit(wxCommandEvent& event)
{
	auto agent = m_agent->As<RenderViewPanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstViewRotations });
}

void RenderViewPanel::OnRotScroll(wxScrollEvent& event)
{
	auto agent = m_agent->As<RenderViewPanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstViewRotationsScroll });
}

void RenderViewPanel::OnOrthoViewSelected(wxCommandEvent& event)
{
	auto agent = m_agent->As<RenderViewPanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstOrthoView });
}

void RenderViewPanel::OnRotSettings(wxCommandEvent& event)
{
	fluo::ValueCollection vc;
	int id = event.GetId();
	switch (id)
	{
	case ID_RotLockChk:
		vc.insert(gstGearedEnable);
		break;
	case ID_ZeroRotBtn:
		vc.insert(gstZeroRotations);
		break;
	case ID_RotResetBtn:
		vc.insert(gstRotationsReset);
		break;
	}
	auto agent = m_agent->As<RenderViewPanelAgent>();
	if (agent)
		agent->UpdateUIToData(vc);
}

