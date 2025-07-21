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

#include <GL/glew.h>
#ifdef _WIN32
#include <GL/wglew.h>
#endif
#include <RenderViewPanel.h>
#include <Global.h>
#include <Names.h>
#include <GlobalStates.h>
#include <MainSettings.h>
#include <MainFrame.h>
#include <RenderCanvas.h>
#include <ClipPlanePanel.h>
#include <ModalDlg.h>
#include <RenderView.h>
#include <Project.h>
#include <VolumeSelector.h>
#include <Ruler.h>
#include <RulerHandler.h>
#include <wxSingleSlider.h>
#include <wxUndoableScrollBar.h>
#include <wxUndoableToolbar.h>
#include <wxUndoableColorPicker.h>
#include <wxBoldText.h>
#include <wx/utils.h>
#include <wx/valnum.h>
#include <algorithm>
#include <limits>
#include <png_resource.h>
#include <icons.h>
#include <wx/display.h>
#include <Debug.h>

int RenderViewPanel::m_max_id = 1;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RenderViewPanel::RenderViewPanel(MainFrame* frame,
	wxGLContext* sharedContext,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name) :
	PropPanel(frame, frame, pos, size, style, name),
	m_default_saved(false),
	m_bg_color_inv(false),
	m_rot_slider(true),
	m_pin_by_user(0),
	m_pin_by_scale(false),
	m_enter_fscreen_trigger(this, 0)
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

	m_id = m_max_id;
	SetName(wxString::Format("Render View:%d", m_max_id++));
	// this list takes care of both pixel and context attributes (no custom edits of wx is preferred)
	//render view/////////////////////////////////////////////////
	int red_bit = glbin_settings.m_red_bit;
	int green_bit = glbin_settings.m_green_bit;
	int blue_bit = glbin_settings.m_blue_bit;
	int alpha_bit = glbin_settings.m_alpha_bit;
	int depth_bit = glbin_settings.m_depth_bit;
	int samples = glbin_settings.m_samples;
	int gl_major_ver = glbin_settings.m_gl_major_ver;
	int gl_minor_ver = glbin_settings.m_gl_minor_ver;
	int gl_profile_mask = glbin_settings.m_gl_profile_mask;
	int api_type = glbin_settings.m_api_type;

	wxGLAttributes attriblist;
#ifdef _WIN32
	if (red_bit >= 16 || green_bit >= 16 || blue_bit >= 16)
	{
		attriblist.AddAttribute(WGL_SUPPORT_OPENGL_ARB);
		attriblist.AddAttribute(GL_TRUE);
		attriblist.AddAttribute(WGL_ACCELERATION_ARB);
		attriblist.AddAttribute(WGL_FULL_ACCELERATION_ARB);
		if (api_type == 2)
		{
			attriblist.AddAttribute(WGL_FLOAT_COMPONENTS_NV);
			attriblist.AddAttribute(GL_TRUE);
		}
		else
		{
			attriblist.AddAttribute(WGL_PIXEL_TYPE_ARB);
			attriblist.AddAttribute(WGL_TYPE_RGBA_FLOAT_ARB);
			//attriblist.AddAttribute(WGL_TYPE_RGBA_ARB);
		}
		attriblist.AddAttribute(WGL_COLOR_BITS_ARB);
		attriblist.AddAttribute(red_bit+green_bit+blue_bit+alpha_bit);
		attriblist.AddAttribute(WGL_RED_BITS_ARB);
		attriblist.AddAttribute(red_bit);
		attriblist.AddAttribute(WGL_GREEN_BITS_ARB);
		attriblist.AddAttribute(green_bit);
		attriblist.AddAttribute(WGL_BLUE_BITS_ARB);
		attriblist.AddAttribute(blue_bit);
		attriblist.AddAttribute(WGL_ALPHA_BITS_ARB);
		attriblist.AddAttribute(alpha_bit);
		attriblist.AddAttribute(WGL_DEPTH_BITS_ARB);
		attriblist.AddAttribute(depth_bit);
		attriblist.AddAttribute(WGL_STENCIL_BITS_ARB);
		attriblist.AddAttribute(8);
	}
	else
	{
		attriblist.PlatformDefaults();
		attriblist.MinRGBA(red_bit, green_bit, blue_bit, alpha_bit);
		attriblist.Depth(depth_bit);
		attriblist.SampleBuffers(1);
		attriblist.Samplers(samples);
	}
#else
	attriblist.PlatformDefaults();
	attriblist.MinRGBA(red_bit, green_bit, blue_bit, alpha_bit);
	attriblist.Depth(depth_bit);
	attriblist.SampleBuffers(1);
	attriblist.Samplers(samples);
#ifdef _DARWIN
	if (gl_major_ver == 3)
	{
		attriblist.AddAttribute(kCGLPFAOpenGLProfile);
		attriblist.AddAttribute(kCGLOGLPVersion_GL3_Core);
	}
	else if (gl_major_ver == 4)
	{
		attriblist.AddAttribute(kCGLPFAOpenGLProfile);
		attriblist.AddAttribute(kCGLOGLPVersion_GL4_Core);
	}
#endif
#endif
	attriblist.DoubleBuffer();
	attriblist.EndList();
	m_canvas = new RenderCanvas(frame, this, attriblist, sharedContext);
	m_render_view = m_canvas->GetRenderView();
	if (!sharedContext)
	{
		wxGLContextAttrs contextAttrs;
		switch (gl_profile_mask)
		{
		case 1:
			contextAttrs.CoreProfile().EndList();
			break;
		case 2:
			contextAttrs.CompatibilityProfile().EndList();
			break;
		}
		contextAttrs.
			MajorVersion(gl_major_ver).
			MinorVersion(gl_minor_ver).
			Robust().
			ResetIsolation().
			EndList();
		sharedContext = new wxGLContext(m_canvas, NULL, &contextAttrs);
		if (!sharedContext->IsOK())
		{
			contextAttrs.Reset();
			contextAttrs.PlatformDefaults().EndList();
			sharedContext = new wxGLContext(m_canvas, NULL, &contextAttrs);
		}
		if (!sharedContext->IsOK())
		{
			wxMessageBox("FluoRender needs an OpenGL 3.3 capable driver.\n" \
				"Please update your graphics card driver or upgrade your graphics card.\n",
				"Graphics card error", wxOK | wxICON_ERROR, this);
			delete sharedContext;
			sharedContext = 0;
			glbin_settings.m_red_bit = 8;
			glbin_settings.m_green_bit = 8;
			glbin_settings.m_blue_bit = 8;
			glbin_settings.m_alpha_bit = 8;
			glbin_settings.Save();
		}
		if (sharedContext)
		{
			//sharedContext->SetCurrent(*m_canvas);
			m_canvas->SetCurrent(*sharedContext);
			m_canvas->m_glRC = sharedContext;
			m_canvas->m_set_gl = true;
		}
	}
	m_canvas->SetCanFocus(false);
	m_view_sizer->Add(m_canvas, 1, wxEXPAND);
#ifdef _DEBUG
	//example Pixel format descriptor detailing each part
	//PIXELFORMATDESCRIPTOR pfd = {
	// sizeof(PIXELFORMATDESCRIPTOR),  //  size of this pfd
	// 1,                     // version number
	// PFD_DRAW_TO_WINDOW |   // support window
	// PFD_SUPPORT_OPENGL |   // support OpenGL
	// PFD_DOUBLEBUFFER,      // double buffered
	// PFD_TYPE_RGBA,         // RGBA type
	// 24,                    // 24-bit color depth
	// 0, 0, 0, 0, 0, 0,      // color bits ignored
	// 0,                     // no alpha buffer
	// 0,                     // shift bit ignored
	// 0,                     // no accumulation buffer
	// 0, 0, 0, 0,            // accum bits ignored
	// 32,                    // 32-bit z-buffer
	// 0,                     // no stencil buffer
	// 0,                     // no auxiliary buffer
	// PFD_MAIN_PLANE,        // main layer
	// 0,                     // reserved
	// 0, 0, 0                // layer masks ignored
	// };
	PIXELFORMATDESCRIPTOR  pfd;
	//check ret. this is an error code when the pixel format is invalid.
	int ret = m_canvas->GetPixelFormat(&pfd);
#endif

	//get actual version
	glGetIntegerv(GL_MAJOR_VERSION, &gl_major_ver);
	glGetIntegerv(GL_MINOR_VERSION, &gl_minor_ver);
	glbin_settings.m_gl_major_ver = gl_major_ver;
	glbin_settings.m_gl_minor_ver = gl_minor_ver;

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

	Bind(wxEVT_TIMER, &RenderViewPanel::OnSetFullScreen, this);

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

int RenderViewPanel::GetViewId()
{
	return glbin_current.GetViewId(m_render_view);
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
	wxSize tbs = m_mix_mode_tb->GetSize();

	//blend mode
	bitmap = wxGetBitmap(layers);
	m_mix_mode_tb->AddCheckTool(
		ID_VolumeSeqRd, "Layer",
		bitmap, wxNullBitmap,
		"Render View as Layers",
		"Render View as Layers");
	bitmap = wxGetBitmap(depth);
	m_mix_mode_tb->AddCheckTool(
		ID_VolumeMultiRd, "Depth",
		bitmap, wxNullBitmap,
		"Render View by Depth",
		"Render View by Depth");
	bitmap = wxGetBitmap(composite);
	m_mix_mode_tb->AddCheckTool(
		ID_VolumeCompRd, "Compo",
		bitmap, wxNullBitmap,
		"Render View as a Composite of Colors",
		"Render View as a Composite of Colors");
	m_mix_mode_tb->Bind(wxEVT_TOOL, &RenderViewPanel::OnMixMode, this);
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
		wxDefaultPosition, FromDIP(wxSize(35, tbs.y-3)), wxTE_RIGHT, vald_int);
	m_scale_cmb = new wxComboBox(this, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(50, tbs.y)), 0, NULL, wxCB_READONLY);
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
	m_aov_text = new wxTextCtrl(this, wxID_ANY, "",
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
	m_depth_atten_factor_text = new wxTextCtrl(this, wxID_ANY, "0.00",
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
	m_scale_factor_text = new wxTextCtrl(this, wxID_ANY, "100",
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
	m_x_rot_text = new wxTextCtrl(this, wxID_ANY, "0.0",
		wxDefaultPosition, FromDIP(wxSize(45,20)), wxTE_RIGHT, vald_fp1);
	m_y_rot_sldr = new wxUndoableScrollBar(this, ID_RotYScroll);
	m_y_rot_sldr->SetScrollbar2(180, 60, 0, 360, 15);
	m_y_rot_text = new wxTextCtrl(this, wxID_ANY, "0.0",
		wxDefaultPosition, FromDIP(wxSize(45,20)), wxTE_RIGHT, vald_fp1);
	m_z_rot_sldr = new wxUndoableScrollBar(this, ID_RotZScroll);
	m_z_rot_sldr->SetScrollbar2(180, 60, 0, 360, 15);
	m_z_rot_text = new wxTextCtrl(this, wxID_ANY, "0.0",
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

void RenderViewPanel::FluoUpdate(const fluo::ValueCollection& vc)
{
	if (FOUND_VALUE(gstNull))
		return;
	if (!m_render_view)
		return;

	int ival;
	bool bval;
	double dval;

	bool update_all = vc.empty();
	bool update_pin_rot_ctr = FOUND_VALUE(gstPinRotCtr);

	//blend mode
	if (update_all || FOUND_VALUE(gstMixMethod))
	{
		ival = m_render_view->GetVolMethod();
		int test = m_mix_mode_tb->GetToolsCount();
		m_mix_mode_tb->ToggleTool(ID_VolumeSeqRd, ival == VOL_METHOD_SEQ);
		m_mix_mode_tb->SetToolNormalBitmap(ID_VolumeSeqRd,
			ival == VOL_METHOD_SEQ ?
			wxGetBitmap(layers) :
			wxGetBitmap(layers_off));
		m_mix_mode_tb->ToggleTool(ID_VolumeMultiRd, ival == VOL_METHOD_MULTI);
		m_mix_mode_tb->SetToolNormalBitmap(ID_VolumeMultiRd,
			ival == VOL_METHOD_MULTI ?
			wxGetBitmap(depth) :
			wxGetBitmap(depth_off));
		m_mix_mode_tb->ToggleTool(ID_VolumeCompRd, ival == VOL_METHOD_COMP);
		m_mix_mode_tb->SetToolNormalBitmap(ID_VolumeCompRd,
			ival == VOL_METHOD_COMP ?
			wxGetBitmap(composite) :
			wxGetBitmap(composite_off));
	}

	//info
	if (update_all || FOUND_VALUE(gstDrawInfo))
	{
		bval = m_render_view->m_draw_info & 1;
		m_hud_tb->ToggleTool(ID_InfoChk, bval);
	}

	//cam center
	if (update_all || FOUND_VALUE(gstDrawCamCtr))
	{
		bval = m_render_view->m_draw_camctr;
		m_hud_tb->ToggleTool(ID_CamCtrChk, bval);
	}

	//legend
	if (update_all || FOUND_VALUE(gstDrawLegend))
	{
		bval = m_render_view->m_draw_legend;
		m_hud_tb->ToggleTool(ID_LegendChk, bval);
	}

	//colormap
	if (update_all || FOUND_VALUE(gstDrawColormap))
	{
		ival = m_render_view->m_colormap_disp;
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

	//scale bar
	if (update_all || FOUND_VALUE(gstDrawScaleBar))
	{
		ival = m_render_view->m_scalebar_disp;
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
	if (update_all || FOUND_VALUE(gstScaleBarUnit))
		m_scale_cmb->Select(m_render_view->m_sb_unit);

	//background
	if (update_all || FOUND_VALUE(gstBgColor))
	{
		fluo::Color c = m_render_view->GetBackgroundColor();
		wxColor wxc((unsigned char)(c.r() * 255 + 0.5),
			(unsigned char)(c.g() * 255 + 0.5),
			(unsigned char)(c.b() * 255 + 0.5));
		m_bg_color_picker->SetColour(wxc);
	}
	if (update_all || FOUND_VALUE(gstBgColorInv))
	{
		m_bg_inv_btn->ToggleTool(0, m_bg_color_inv);
		if (m_bg_color_inv)
			m_bg_inv_btn->SetToolNormalBitmap(0,
				wxGetBitmap(invert));
		else
			m_bg_inv_btn->SetToolNormalBitmap(0,
				wxGetBitmap(invert_off));
	}

	//angle of view
	if (update_all || FOUND_VALUE(gstAov))
	{
		ival = static_cast<int>(std::round(m_render_view->GetAov()));
		bval = m_render_view->GetPersp();
		m_aov_sldr->ChangeValue(bval ? ival : 10);
		m_aov_text->ChangeValue(bval ? std::to_string(ival) : "Ortho");
		if (bval)
			m_cam_op_tb->SetToolNormalBitmap(ID_OrthoPerspBtn,
				wxGetBitmap(persp));
		else
			m_cam_op_tb->SetToolNormalBitmap(ID_OrthoPerspBtn,
				wxGetBitmap(ortho));
	}

	//free fly
	if (update_all || FOUND_VALUE(gstCamMode))
	{
		ival = m_render_view->GetCamMode();
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

	//stereo & holography
	if (update_all || FOUND_VALUE(gstHologramMode))
	{
		ival = glbin_settings.m_hologram_mode;
		m_full_screen_toolbar->ToggleTool(ID_VrChk, ival == 1);
		m_full_screen_toolbar->ToggleTool(ID_LookingGlassChk, ival == 2);
		if (ival != 2)
			m_render_view->ResetSize();
	}

	//depthe attenuation
	if (update_all || FOUND_VALUE(gstDepthAtten))
	{
		bval = m_render_view->GetFog();
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
	if (update_all || FOUND_VALUE(gstDaInt))
	{
		dval = m_render_view->GetFogIntensity();
		m_depth_atten_factor_sldr->ChangeValue(std::round(dval * 100));
		m_depth_atten_factor_text->ChangeValue(wxString::Format("%.2f", dval));
	}

	//center click
	if (update_all || FOUND_VALUE(gstFreehandToolState))
	{
		m_center_click_btn->ToggleTool(0, m_render_view->GetIntMode() == InteractiveMode::CenterClick);
	}

	//scale factor
	if (update_all || FOUND_VALUE(gstScaleFactor))
	{
		double scale = m_render_view->m_scale_factor;
		switch (m_render_view->m_scale_mode)
		{
		case 0:
			break;
		case 1:
			scale /= m_render_view->Get121ScaleFactor();
			break;
		case 2:
		{
			auto vd = m_render_view->m_cur_vol.lock();
			if (!vd && !m_render_view->GetVolPopListEmpty())
				vd = m_render_view->GetVolPopList(0);
			if (!vd)
				break;
			double spcx, spcy, spcz;
			vd->GetSpacings(spcx, spcy, spcz, vd->GetLevel());
			if (spcx > 0.0)
				scale /= m_render_view->Get121ScaleFactor() * spcx;
		}
		break;
		}

		ival = std::round(scale * 100);
		m_scale_factor_sldr->ChangeValue(ival);
		m_scale_factor_text->ChangeValue(wxString::Format("%d", ival));
		m_scale_factor_text->Update();

		//check if need update pin rot center
		m_pin_by_scale = scale > glbin_settings.m_pin_threshold;
		if (m_pin_by_user == 0)
		{
			bool pin_by_canvas = m_render_view->m_pin_rot_ctr;
			m_render_view->SetPinRotCenter(m_pin_by_scale, false);
			update_pin_rot_ctr = m_pin_by_scale != pin_by_canvas;
		}
	}
	//scale mode
	if (update_all || FOUND_VALUE(gstScaleMode))
	{
		switch (m_render_view->m_scale_mode)
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
	//pin rotation center
	if (update_all || update_pin_rot_ctr)
	{
		bval = m_render_view->m_pin_rot_ctr;
		m_pin_btn->ToggleTool(0, bval);
		if (bval)
			m_pin_btn->SetToolNormalBitmap(0,
				wxGetBitmap(pin));
		else
			m_pin_btn->SetToolNormalBitmap(0,
				wxGetBitmap(pin_off));
	}

	//lock rot
	if (update_all || FOUND_VALUE(gstGearedEnable))
	{
		bval = m_render_view->GetRotLock();
		m_rot_btn->ToggleTool(ID_RotLockChk, bval);
		if (bval)
			m_rot_btn->SetToolNormalBitmap(ID_RotLockChk,
				wxGetBitmap(gear_45));
		else
			m_rot_btn->SetToolNormalBitmap(ID_RotLockChk,
				wxGetBitmap(gear_dark));
	}

	//slider type
	if (update_all || FOUND_VALUE(gstRotSliderMode))
	{
		m_slider_mode_btn->ToggleTool(0, m_rot_slider);
		if (m_rot_slider)
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

	//roatation
	if (update_all || FOUND_VALUE(gstCamRotation))
	{
		fluo::Vector rot = m_render_view->GetRotations();
		m_x_rot_sldr->ChangeValue(static_cast<int>(std::round(rot.x())));
		m_y_rot_sldr->ChangeValue(static_cast<int>(std::round(rot.y())));
		m_z_rot_sldr->ChangeValue(static_cast<int>(std::round(rot.z())));
		m_x_rot_text->ChangeValue(wxString::Format("%.1f", rot.x()));
		m_y_rot_text->ChangeValue(wxString::Format("%.1f", rot.y()));
		m_z_rot_text->ChangeValue(wxString::Format("%.1f", rot.z()));
		m_x_rot_text->Update();
		m_y_rot_text->Update();
		m_z_rot_text->Update();
		m_ortho_view_cmb->Select(m_render_view->GetOrientation());
	}
}

void RenderViewPanel::SetVolumeMethod(int val)
{
	m_render_view->SetVolMethod(val);

	FluoRefresh(2, { gstMixMethod }, { GetViewId() });
}

void RenderViewPanel::Capture()
{
	//reset enlargement
	m_render_view->SetEnlarge(false);
	m_render_view->SetEnlargeScale(1.0);

	ModalDlg file_dlg(m_frame,
		"Save Captured Image", "", "output",
		"Tiff File (*.tif)|*.tif|"\
		"Tiff File (*.tiff)|*.tiff|"\
		"Png File (*.png)|*.png|"\
		"Jpeg File (*.jpg)|*.jpg|"\
		"Jpeg File (*.jpeg)|*.jpeg",
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	file_dlg.SetExtraControlCreator(CreateExtraCaptureControl);
	int rval = file_dlg.ShowModal();
	if (rval == wxID_OK)
	{
		m_render_view->m_cap_file = file_dlg.GetPath();
		m_render_view->m_capture = true;
		glbin_states.m_capture = true;
		RefreshGL();

		if (glbin_settings.m_prj_save)
		{
			std::wstring new_folder = m_render_view->m_cap_file + L"_project";
			MkDirW(new_folder);
			std::filesystem::path p = new_folder;
			p /= file_dlg.GetFilename().ToStdString() + "_project.vrp";
			std::wstring prop_file = p.wstring();
			bool inc = std::filesystem::exists(prop_file) &&
				glbin_settings.m_prj_save_inc;
			glbin_project.Save(prop_file, inc);
		}
	}
}

void RenderViewPanel::SetInfo(bool val)
{
	if (val)
		m_render_view->m_draw_info |= 1;
	else
		m_render_view->m_draw_info &= ~1;

	FluoRefresh(2, { gstDrawInfo }, { GetViewId() });
}

void RenderViewPanel::SetDrawCamCtr(bool val)
{
	m_render_view->m_draw_camctr = val;

	FluoRefresh(2, { gstDrawCamCtr }, { GetViewId() });
}

void RenderViewPanel::SetLegend(bool val)
{
	m_render_view->m_draw_legend = val;

	FluoRefresh(2, { gstDrawLegend }, { GetViewId() });
}

void RenderViewPanel::SetDrawColormap()
{
	int val = m_render_view->m_colormap_disp;
	val++;
	val = val % 3; // cycle through 0, 1, 2
	m_render_view->m_colormap_disp = val;

	FluoRefresh(2, { gstDrawColormap }, { GetViewId() });
}

void RenderViewPanel::SetDrawScalebar()
{
	int val = m_render_view->m_scalebar_disp;
	val++;
	val = val % 3; // cycle through 0, 1, 2
	m_render_view->m_scalebar_disp = val;

	FluoRefresh(2, { gstDrawScaleBar }, { GetViewId() });
}

void RenderViewPanel::SetScaleText(double val)
{
	std::wstring str, num_text, unit_text;
	num_text = std::to_wstring((int)val);
	switch (m_render_view->m_sb_unit)
	{
	case 0:
		unit_text = L"nm";
		break;
	case 1:
	default:
		unit_text = L"\u03BCm";
		break;
	case 2:
		unit_text = L"mm";
		break;
	}
	str = num_text + L" " + unit_text;
	m_render_view->SetSBText(str);
	m_render_view->SetScaleBarLen(val);
	m_render_view->m_sb_num = num_text;

	FluoRefresh(2, {gstNull}, { GetViewId() });
}

void RenderViewPanel::SetScaleUnit(int val)
{
	m_render_view->m_sb_unit = val;
	double dval = m_render_view->m_sb_length;
	std::wstring str, num_text, unit_text;
	num_text = std::to_wstring((int)dval);
	switch (val)
	{
	case 0:
		unit_text = L"nm";
		break;
	case 1:
	default:
		unit_text = L"\u03BCm";
		break;
	case 2:
		unit_text = L"mm";
		break;
	}
	str = num_text + L" " + unit_text;
	m_render_view->SetSBText(str);
	m_render_view->SetScaleBarLen(dval);
	m_render_view->m_sb_num = num_text;

	FluoRefresh(2, { gstNull }, { GetViewId() });
}

void RenderViewPanel::SetBgColor(fluo::Color val)
{
	m_render_view->SetBackgroundColor(val);

	FluoRefresh(2, { gstBgColor }, { GetViewId() });
}

void RenderViewPanel::SetBgColorInvert(bool val)
{
	m_bg_color_inv = val;
	fluo::Color c = m_render_view->GetBackgroundColor();
	c = fluo::Color(1.0, 1.0, 1.0) - c;
	m_render_view->SetBackgroundColor(c);

	FluoRefresh(2, { gstBgColor, gstBgColorInv }, { GetViewId() });
}

void RenderViewPanel::SetAov(double val, bool notify)
{
	if (val < 11)
	{
		m_render_view->SetPersp(false);
		if (m_render_view->GetAov() == 10)
			return;
		m_render_view->SetAov(10);
	}
	else if (val > 100)
	{
		m_render_view->SetPersp(true);
		if (m_render_view->GetAov() == 100)
			return;
		m_render_view->SetAov(100);
	}
	else
	{
		m_render_view->SetPersp(true);
		if (m_render_view->GetAov() == val)
			return;
		m_render_view->SetAov(val);
	}

	if (notify)
		FluoRefresh(2, { gstAov }, { GetViewId() });
	else
		FluoRefresh(2, { gstNull }, { GetViewId() });
}

void RenderViewPanel::SetProjection()
{
	bool bval = m_render_view->GetPersp();
	if (bval)
	{
		SetAov(10, true);
	}
	else
	{
		SetAov(45, true);
	}
}

void RenderViewPanel::SetCamMode()
{
	int ival = m_render_view->GetCamMode();
	ival = (ival + 1) % 2; // cycle through 0 and 1
	m_render_view->SetCamMode(ival);

	FluoRefresh(2, { gstCamMode }, { GetViewId() });
}

void RenderViewPanel::SetStereography()
{
	int ival = glbin_settings.m_hologram_mode;
	glbin_settings.m_hologram_mode = ival == 1 ? 0 : 1;
	FluoRefresh(0, { gstHologramMode });
}

void RenderViewPanel::SetHolography()
{
	int ival = glbin_settings.m_hologram_mode;
	glbin_settings.m_hologram_mode = ival == 2 ? 0 : 2;
	FluoRefresh(0, { gstHologramMode });
}

void RenderViewPanel::SetFullScreen()
{
	m_enter_fscreen_trigger.Start(10);
}

void RenderViewPanel::CloseFullScreen()
{
	if (m_canvas->GetParent() == m_full_frame)
		m_canvas->Close();
}

void RenderViewPanel::SetDepthAttenEnable(bool val)
{
	m_render_view->SetFog(val);
	FluoRefresh(2, { gstDepthAtten }, { GetViewId() });
}

void RenderViewPanel::SetDepthAtten(double val, bool notify)
{
	if (m_render_view->GetFogIntensity() == val)
		return;
	m_render_view->SetFogIntensity(val);
	if (notify)
		FluoRefresh(2, { gstDaInt }, { GetViewId() });
	else
		FluoRefresh(2, { gstNull }, { GetViewId() });
}

void RenderViewPanel::SetCenter()
{
	m_render_view->SetCenter();
	FluoRefresh(2, { gstNull }, { GetViewId() });
}

void RenderViewPanel::SetScale121()
{
	m_render_view->SetScale121();
	if (m_render_view->m_mouse_focus)
		m_canvas->SetFocus();
	FluoRefresh(2, { gstNull }, { GetViewId() });
}

void RenderViewPanel::SetScaleFactor(double val)
{
	double factor = val;
	switch (m_render_view->m_scale_mode)
	{
		case 0:
			break;
		case 1:
			factor = val * m_render_view->Get121ScaleFactor();
			break;
		case 2:
		{
			auto vd = m_render_view->m_cur_vol.lock();
			if (!vd && !m_render_view->GetVolPopListEmpty())
				vd = m_render_view->GetVolPopList(0);
			double spcx, spcy, spcz;
			if (vd)
			{
				vd->GetSpacings(spcx, spcy, spcz, vd->GetLevel());
				if (spcx > 0.0)
					factor = val * m_render_view->Get121ScaleFactor() * spcx;
			}
		}
		break;
	}
	if (m_render_view->m_scale_factor == factor)
		return;
	m_render_view->m_scale_factor = factor;
	FluoRefresh(2, { gstScaleFactor, gstPinRotCtr }, { GetViewId() });
}

void RenderViewPanel::SetScaleMode(int val)
{
	m_render_view->m_scale_mode = val;
	FluoRefresh(2, { gstScaleMode, gstScaleFactor }, { GetViewId() });
}

void RenderViewPanel::SetRotLock(bool val)
{
	m_render_view->SetRotLock(val);
	if (val)
	{
		fluo::Vector rot = m_render_view->GetRotations();
		rot = fluo::Vector(static_cast<int>(rot.x() / 45) * 45,
				static_cast<int>(rot.y() / 45) * 45,
				static_cast<int>(rot.z() / 45) * 45);
		SetRotations(rot, true);
	}
	FluoRefresh(2, { gstGearedEnable }, { GetViewId() });
}

void RenderViewPanel::SetSliderType()
{
	m_rot_slider = !m_rot_slider;
	FluoRefresh(2, { gstRotSliderMode }, { GetViewId() });
}

void RenderViewPanel::SetRotations(const fluo::Vector& val, bool notify)
{
	if (m_render_view->GetRotations() == val)
		return;
	m_render_view->SetRotations(val, false);
	if (notify)
		FluoRefresh(2, { gstCamRotation }, { GetViewId() });
	else
		FluoRefresh(2, { gstNull }, { GetViewId() });
}

void RenderViewPanel::SetZeroRotations()
{
	fluo::Vector rot = m_render_view->GetRotations();
	if (rot.x() == 0.0 &&
		rot.y() == 0.0 &&
		rot.z() == 0.0)
	{
		//reset
		rot = m_render_view->ResetZeroRotations();
		m_render_view->SetRotations(rot, false);
	}
	else
	{
		m_render_view->SetZeroRotations();
		m_render_view->SetRotations(fluo::Vector(0), false);
	}
	FluoRefresh(2, { gstCamRotation }, { GetViewId() });
}

void RenderViewPanel::OnMixMode(wxCommandEvent& event)
{
	int id = event.GetId();

	switch (id)
	{
	case ID_VolumeSeqRd:
		SetVolumeMethod(VOL_METHOD_SEQ);
		break;
	case ID_VolumeMultiRd:
		SetVolumeMethod(VOL_METHOD_MULTI);
		break;
	case ID_VolumeCompRd:
		SetVolumeMethod(VOL_METHOD_COMP);
		break;
	}
}

void RenderViewPanel::OnHud(wxCommandEvent& event)
{
	int id = event.GetId();

	switch (id)
	{
	case ID_InfoChk:
		SetInfo(m_hud_tb->GetToolState(ID_InfoChk));
		break;
	case ID_CamCtrChk:
		SetDrawCamCtr(m_hud_tb->GetToolState(ID_CamCtrChk));
		break;
	case ID_LegendChk:
		SetLegend(m_hud_tb->GetToolState(ID_LegendChk));
		break;
	case ID_Colormap:
		SetDrawColormap();
		break;
	case ID_ScaleBar:
		SetDrawScalebar();
		break;
	}
}

void RenderViewPanel::OnSnapshotBtn(wxCommandEvent& event)
{
	Capture();
}

void RenderViewPanel::OnViewManipBtn(wxCommandEvent& event)
{
	if (m_render_view)
		m_render_view->SetIntMode(InteractiveMode::Viewport);
	glbin_vol_selector.SetSelectMode(flrd::SelectMode::None);
	glbin_ruler_handler.SetRulerMode(flrd::RulerMode::None);
	FluoRefresh(0, { gstFreehandToolState }, { -1 });
}

void RenderViewPanel::OnScaleText(wxCommandEvent& event)
{
	wxString str = m_scale_text->GetValue();
	double len;
	str.ToDouble(&len);
	SetScaleText(len);
}

void RenderViewPanel::OnScaleUnit(wxCommandEvent& event)
{
	SetScaleUnit(m_scale_cmb->GetSelection());
}

void RenderViewPanel::OnBgColorChange(wxColourPickerEvent& event)
{
	wxColor c = m_bg_color_picker->GetColour();
	fluo::Color color(c.Red() / 255.0, c.Green() / 255.0, c.Blue() / 255.0);
	SetBgColor(color);
}

void RenderViewPanel::OnBgInvBtn(wxCommandEvent& event)
{
	SetBgColorInvert(!m_bg_color_inv);
}

void RenderViewPanel::OnAovSldrIdle(wxIdleEvent& event)
{
	if (m_render_view->m_capture)
		return;

	wxPoint pos = wxGetMousePosition();
	wxRect reg = m_aov_sldr->GetScreenRect();
	wxWindow* window = wxWindow::FindFocus();
	bool bval = window && reg.Contains(pos);
	glbin_states.m_mouse_in_aov_slider = bval;
	if (glbin_states.ClipDisplayChanged())
		FluoRefresh(3, { gstNull },
			{ GetViewId() });
}

void RenderViewPanel::OnAovChange(wxScrollEvent& event)
{
	int ival = m_aov_sldr->GetValue();
	bool bval = m_render_view->GetPersp();

	SetAov(ival, true);
}

void RenderViewPanel::OnAovText(wxCommandEvent& event)
{
	wxString str = m_aov_text->GetValue();
	int ival = 10;
	long val;
	if (str.ToLong(&val))
		ival = val;
	SetAov(ival, true);
}

void RenderViewPanel::OnToolBar2(wxCommandEvent& event)
{
	int id = event.GetId();

	switch (id)
	{
	case ID_OrthoPerspBtn:
		//toggle between ortho and perspective
		SetProjection();
		break;
	case ID_CamModeBtn:
		SetCamMode();
		break;
	case ID_DefaultBtn:
		SaveDefault();
		break;
	}
}

void RenderViewPanel::OnFullScreenToolbar(wxCommandEvent& event)
{
	int id = event.GetId();

	switch (id)
	{
	case ID_VrChk:
		SetStereography();
		break;
	case ID_LookingGlassChk:
		SetHolography();
		break;
	case ID_FullScreenBtn:
		SetFullScreen();
		break;
	}
}

void RenderViewPanel::OnSetFullScreen(wxTimerEvent& event)
{
	m_enter_fscreen_trigger.Stop();
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
		RefreshGL();
	}
	else
	{
		m_canvas->Close();
	}
}

void RenderViewPanel::OnDepthAttenCheck(wxCommandEvent& event)
{
	SetDepthAttenEnable(m_depth_atten_btn->GetToolState(0));
}

//bar left
void RenderViewPanel::OnDepthAttenChange(wxScrollEvent& event)
{
	double val = m_depth_atten_factor_sldr->GetValue() / 100.0;
	m_depth_atten_factor_text->ChangeValue(wxString::Format("%.2f", val));
	m_depth_atten_factor_text->Update();
	SetDepthAtten(val, false);
}

void RenderViewPanel::OnDepthAttenEdit(wxCommandEvent& event)
{
	wxString str = m_depth_atten_factor_text->GetValue();
	double val = 0;
	str.ToDouble(&val);
	m_depth_atten_factor_sldr->ChangeValue(std::round(val * 100));
	SetDepthAtten(val, false);
}

void RenderViewPanel::OnDepthAttenReset(wxCommandEvent& event)
{
	SetDepthAttenEnable(glbin_view_def.m_use_fog);
	SetDepthAtten(glbin_view_def.m_fog_intensity, true);
}

void RenderViewPanel::OnPin(wxCommandEvent& event)
{
	bool val = m_pin_btn->GetToolState(0);
	if (m_pin_by_scale == val)
		m_pin_by_user = 0;
	else
		m_pin_by_user = val ? 2 : 1;
	m_render_view->SetPinRotCenter(val, true);
	FluoRefresh(2, { gstPinRotCtr }, { GetViewId() });
}

void RenderViewPanel::OnCenter(wxCommandEvent& event)
{
	SetCenter();
}

void RenderViewPanel::OnCenterClick(wxCommandEvent& event)
{
	glbin_states.ToggleIntMode(InteractiveMode::CenterClick);
	m_frame->UpdateProps({ gstFreehandToolState });
}

void RenderViewPanel::OnScale121(wxCommandEvent& event)
{
	SetScale121();
}

void RenderViewPanel::OnScaleFactorChange(wxScrollEvent& event)
{
	int ival = m_scale_factor_sldr->GetValue();
	double dval = ival / 100.0;
	SetScaleFactor(dval);
}

void RenderViewPanel::OnScaleFactorEdit(wxCommandEvent& event)
{
	wxString str = m_scale_factor_text->GetValue();
	long val = 0;
	str.ToLong(&val);
	if (val > 0)
	{
		double dval = val / 100.0;
		SetScaleFactor(dval);
	}
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
		SetScaleFactor(val / 100.0);
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
		SetScaleFactor(val / 100.0);
}

void RenderViewPanel::OnScaleReset(wxCommandEvent& event)
{
	SetScaleFactor(glbin_view_def.m_scale_factor);
}

void RenderViewPanel::OnScaleMode(wxCommandEvent& event)
{
	int mode = m_render_view->m_scale_mode;
	mode += 1;
	mode = mode > 2 ? 0 : mode;
	SetScaleMode(mode);
}

void RenderViewPanel::OnRotSliderMode(wxCommandEvent& event)
{
	SetSliderType();
}

void RenderViewPanel::OnRotEdit(wxCommandEvent& event)
{
	wxString str;
	double rotx, roty, rotz;
	str = m_x_rot_text->GetValue();
	str.ToDouble(&rotx);
	str = m_y_rot_text->GetValue();
	str.ToDouble(&roty);
	str = m_z_rot_text->GetValue();
	str.ToDouble(&rotz);
	m_x_rot_sldr->ChangeValue(std::round(rotx));
	m_y_rot_sldr->ChangeValue(std::round(roty));
	m_z_rot_sldr->ChangeValue(std::round(rotz));
	SetRotations(fluo::Vector(rotx, roty, rotz), false);
}

void RenderViewPanel::OnRotScroll(wxScrollEvent& event)
{
	double rotx, roty, rotz;
	rotx = m_x_rot_sldr->GetValue();
	roty = m_y_rot_sldr->GetValue();
	rotz = m_z_rot_sldr->GetValue();
	m_x_rot_text->ChangeValue(wxString::Format("%.1f", rotx));
	m_y_rot_text->ChangeValue(wxString::Format("%.1f", roty));
	m_z_rot_text->ChangeValue(wxString::Format("%.1f", rotz));
	m_x_rot_text->Update();
	m_y_rot_text->Update();
	m_z_rot_text->Update();
	SetRotations(fluo::Vector(rotx, roty, rotz), false);
}

void RenderViewPanel::OnOrthoViewSelected(wxCommandEvent& event)
{
	int sel = 6;
	if (m_ortho_view_cmb)
		sel = m_ortho_view_cmb->GetSelection();
	switch (sel)
	{
	case 0://+Z
		m_render_view->SetRotations(fluo::Vector(0.0, 0.0, 0.0), false);
		break;
	case 1://-Z
		m_render_view->SetRotations(fluo::Vector(0.0, 180.0, 0.0), false);
		break;
	case 2://+Y
		m_render_view->SetRotations(fluo::Vector(90.0, 0.0, 0.0), false);
		break;
	case 3://-Y
		m_render_view->SetRotations(fluo::Vector(270.0, 0.0, 0.0), false);
		break;
	case 4://+X
		m_render_view->SetRotations(fluo::Vector(0.0, 90.0, 0.0), false);
		break;
	case 5://-X
		m_render_view->SetRotations(fluo::Vector(0.0, 270.0, 0.0), false);
		break;
	}
	if (sel < 6)
		SetRotLock(true);
	else
		SetRotLock(false);
}

void RenderViewPanel::OnRotSettings(wxCommandEvent& event)
{
	int id = event.GetId();

	switch (id)
	{
	case ID_RotLockChk:
	{
		bool bval = m_render_view->GetRotLock();
		SetRotLock(!bval);
	}
		break;
	case ID_ZeroRotBtn:
		SetZeroRotations();
		break;
	case ID_RotResetBtn:
		SetRotations(fluo::Vector(0), true);
		break;
	}
}

//reset counter
void RenderViewPanel::ResetID()
{
	m_max_id = 1;
}

void RenderViewPanel::SetGL(bool bval)
{
	if (m_canvas)
		m_canvas->m_set_gl = bval;
}

//get rendering context
wxGLContext* RenderViewPanel::GetContext()
{
	if (m_canvas)
		return m_canvas->m_glRC/*GetContext()*/;
	else
		return 0;
}

void RenderViewPanel::RefreshGL(bool start_loop)
{
	if (m_render_view)
	{
		m_render_view->SetForceClear(true);
		m_render_view->SetInteractive(false);
		m_render_view->RefreshGL(0, false, start_loop);
	}
}

//bar top
//ch1
void RenderViewPanel::OnCh1Check(wxCommandEvent& event)
{
	wxCheckBox* ch1 = (wxCheckBox*)event.GetEventObject();
	glbin_settings.m_save_compress = ch1->GetValue();
}

//save alpha
void RenderViewPanel::OnChAlphaCheck(wxCommandEvent& event)
{
	wxCheckBox* ch_alpha = (wxCheckBox*)event.GetEventObject();
	glbin_settings.m_save_alpha = ch_alpha->GetValue();
}

//save float
void RenderViewPanel::OnChFloatCheck(wxCommandEvent& event)
{
	wxCheckBox* ch_float = (wxCheckBox*)event.GetEventObject();
	glbin_settings.m_save_float = ch_float->GetValue();
}

//dpi
void RenderViewPanel::OnDpiText(wxCommandEvent& event)
{
	wxTextCtrl* tx_dpi = (wxTextCtrl*)event.GetEventObject();
	wxString str = event.GetString();
	long lval;
	str.ToLong(&lval);
	glbin_settings.m_dpi = lval;
	if (!tx_dpi)
		return;
	wxCheckBox* ch_enlarge = (wxCheckBox*)tx_dpi->GetParent()->FindWindow(ID_ENLARGE_CHK);
	wxSlider* sl_enlarge = (wxSlider*)tx_dpi->GetParent()->FindWindow(ID_ENLARGE_SLDR);
	wxTextCtrl* tx_enlarge = (wxTextCtrl*)tx_dpi->GetParent()->FindWindow(ID_ENLARGE_TEXT);
	bool enlarge = lval > 72;
	if (ch_enlarge)
		ch_enlarge->SetValue(enlarge);
	double enlarge_scale = (double)lval / 72.0;
	auto view = glbin_current.render_view.lock();
	if (view)
	{
		view->SetEnlarge(enlarge);
		view->SetEnlargeScale(enlarge_scale);
	}
	if (sl_enlarge)
	{
		sl_enlarge->Enable(enlarge);
		sl_enlarge->SetValue(std::round(enlarge_scale * 10));
	}
	if (tx_enlarge)
	{
		tx_enlarge->Enable(enlarge);
		tx_enlarge->ChangeValue(wxString::Format("%.1f", enlarge_scale));
	}
}

//embde project
void RenderViewPanel::OnChEmbedCheck(wxCommandEvent& event)
{
	wxCheckBox* ch_embed = (wxCheckBox*)event.GetEventObject();
	glbin_settings.m_vrp_embed = ch_embed->GetValue();
}

//enlarge output image
void RenderViewPanel::OnChEnlargeCheck(wxCommandEvent& event)
{
	wxCheckBox* ch_enlarge = (wxCheckBox*)event.GetEventObject();
	if (ch_enlarge)
	{
		bool enlarge = ch_enlarge->GetValue();
		auto view = glbin_current.render_view.lock();
		if (view)
			view->SetEnlarge(enlarge);
		if (ch_enlarge->GetParent())
		{
			wxSlider* sl_enlarge = (wxSlider*)
				ch_enlarge->GetParent()->FindWindow(
					ID_ENLARGE_SLDR);
			wxTextCtrl* tx_enlarge = (wxTextCtrl*)
				ch_enlarge->GetParent()->FindWindow(
					ID_ENLARGE_TEXT);
			if (sl_enlarge && tx_enlarge)
			{
				if (enlarge)
				{
					sl_enlarge->Enable();
					tx_enlarge->Enable();
				}
				else
				{
					sl_enlarge->Disable();
					tx_enlarge->Disable();
				}
			}
		}
	}
}

void RenderViewPanel::OnSlEnlargeScroll(wxScrollEvent& event)
{
	int ival = event.GetPosition();
	wxSlider* sl_enlarge = (wxSlider*)event.GetEventObject();
	if (sl_enlarge && sl_enlarge->GetParent())
	{
		wxTextCtrl* tx_enlarge = (wxTextCtrl*)
			sl_enlarge->GetParent()->FindWindow(
				ID_ENLARGE_TEXT);
		if (tx_enlarge)
		{
			wxString str = wxString::Format("%.1f", ival / 10.0);
			tx_enlarge->SetValue(str);
		}
	}
}

void RenderViewPanel::OnTxEnlargeText(wxCommandEvent& event)
{
	wxString str = event.GetString();
	double dval;
	str.ToDouble(&dval);
	auto view = glbin_current.render_view.lock();
	if (view)
		view->SetEnlargeScale(dval);
	int ival = std::round(dval * 10);
	wxTextCtrl* tx_enlarge = (wxTextCtrl*)event.GetEventObject();
	if (tx_enlarge && tx_enlarge->GetParent())
	{
		wxSlider* sl_enlarge = (wxSlider*)
			tx_enlarge->GetParent()->FindWindow(
				ID_ENLARGE_SLDR);
		if (sl_enlarge)
			sl_enlarge->SetValue(ival);
	}
}


wxWindow* RenderViewPanel::CreateExtraCaptureControl(wxWindow* parent)
{
	wxPanel* panel = new wxPanel(parent);
#ifdef _DARWIN
	panel->SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#elifdef __linux__
    panel->SetWindowVariant(wxWINDOW_VARIANT_MINI);
#endif
	wxStaticBoxSizer *group1 = new wxStaticBoxSizer(
		wxVERTICAL, panel, "Additional Options");

	//compressed
	wxCheckBox* ch1 = new wxCheckBox(panel, ID_LZW_COMP,
		"Compress to save space");
	ch1->Connect(ch1->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(RenderViewPanel::OnCh1Check), NULL, panel);
	if (ch1)
		ch1->SetValue(glbin_settings.m_save_compress);

	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	//save alpha
	wxCheckBox* ch_alpha = new wxCheckBox(panel, ID_SAVE_ALPHA,
		"Save alpha channel");
	ch_alpha->Connect(ch_alpha->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(RenderViewPanel::OnChAlphaCheck), NULL, panel);
	if (ch_alpha)
		ch_alpha->SetValue(glbin_settings.m_save_alpha);
	//save float
	wxCheckBox* ch_float = new wxCheckBox(panel, ID_SAVE_FLOAT,
		"Save float channel");
	ch_float->Connect(ch_float->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(RenderViewPanel::OnChFloatCheck), NULL, panel);
	if (ch_float)
		ch_float->SetValue(glbin_settings.m_save_float);
	sizer_1->Add(ch_alpha, 0, wxALIGN_CENTER);
	sizer_1->Add(10, 10);
	sizer_1->Add(ch_float, 0, wxALIGN_CENTER);

	//dpi
	wxStaticText* st = new wxStaticText(panel, wxID_ANY, "DPI: ",
		wxDefaultPosition, wxDefaultSize);
	wxIntegerValidator<unsigned int> vald_int;
	wxTextCtrl* tx_dpi = new wxTextCtrl(panel, ID_DPI,
		"", wxDefaultPosition, parent->FromDIP(wxSize(60, 23)), wxTE_RIGHT, vald_int);
	tx_dpi->Connect(tx_dpi->GetId(), wxEVT_COMMAND_TEXT_UPDATED,
		wxCommandEventHandler(RenderViewPanel::OnDpiText), NULL, panel);
	float dpi = glbin_settings.m_dpi;
	tx_dpi->ChangeValue(wxString::Format("%.0f", dpi));
	//enlarge
	wxBoxSizer* sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	wxCheckBox* ch_enlarge = new wxCheckBox(panel, ID_ENLARGE_CHK,
		"Enlarge output image");
	ch_enlarge->Connect(ch_enlarge->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(RenderViewPanel::OnChEnlargeCheck), NULL, panel);
	bool enlarge = dpi > 72;
	double enlarge_scale = dpi / 72.0;
	ch_enlarge->SetValue(enlarge);
	wxSlider* sl_enlarge = new wxSlider(panel, ID_ENLARGE_SLDR,
		10, 10, 100);
	sl_enlarge->Connect(sl_enlarge->GetId(), wxEVT_COMMAND_SLIDER_UPDATED,
		wxScrollEventHandler(RenderViewPanel::OnSlEnlargeScroll), NULL, panel);
	sl_enlarge->Enable(enlarge);
	auto view = glbin_current.render_view.lock();
	if (view)
	{
		view->SetEnlarge(enlarge);
		view->SetEnlargeScale(enlarge_scale);
	}
	sl_enlarge->SetValue(std::round(enlarge_scale * 10));
	wxFloatingPointValidator<double> vald_fp(1);
	wxTextCtrl* tx_enlarge = new wxTextCtrl(panel, ID_ENLARGE_TEXT,
		"1.0", wxDefaultPosition, parent->FromDIP(wxSize(60, 23)), wxTE_RIGHT, vald_fp);
	tx_enlarge->Connect(tx_enlarge->GetId(), wxEVT_COMMAND_TEXT_UPDATED,
		wxCommandEventHandler(RenderViewPanel::OnTxEnlargeText), NULL, panel);
	tx_enlarge->Enable(enlarge);
	tx_enlarge->ChangeValue(wxString::Format("%.1f", enlarge_scale));
	sizer_2->Add(st, 0, wxALIGN_CENTER);
	sizer_2->Add(tx_dpi, 0, wxALIGN_CENTER);
	sizer_2->Add(10, 10);
	sizer_2->Add(ch_enlarge, 0, wxALIGN_CENTER);
	sizer_2->Add(10, 10);
	sizer_2->Add(sl_enlarge, 1, wxEXPAND);
	sizer_2->Add(10, 10);
	sizer_2->Add(tx_enlarge, 0, wxALIGN_CENTER);

	//copy all files check box
	wxCheckBox* ch_embed = 0;
	if (glbin_settings.m_prj_save)
	{
		ch_embed = new wxCheckBox(panel, ID_EMBED_FILES,
			"Embed all files in the project folder");
		ch_embed->Connect(ch_embed->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
			wxCommandEventHandler(RenderViewPanel::OnChEmbedCheck), NULL, panel);
		ch_embed->SetValue(glbin_settings.m_vrp_embed);
	}

	//group
	group1->Add(10, 10);
	group1->Add(ch1);
	group1->Add(10, 10);
	group1->Add(sizer_1);
	group1->Add(10, 10);
	group1->Add(sizer_2);
	if (glbin_settings.m_prj_save &&
		ch_embed)
	{
		group1->Add(10, 10);
		group1->Add(ch_embed);
	}
	group1->Add(10, 20);

	panel->SetSizerAndFit(group1);
	panel->Layout();

	return panel;
}

void RenderViewPanel::SaveDefault(unsigned int mask)
{
	wxString str;
	wxColor cVal;

	//render modes
	if (mask & 0x1)
		glbin_view_def.m_vol_method = m_render_view->GetVolMethod();
	//background color
	if (mask & 0x2)
		glbin_view_def.m_bg_color = m_render_view->GetBackgroundColor();
	//camera center
	if (mask & 0x4)
		glbin_view_def.m_draw_camctr = m_render_view->m_draw_camctr;
	//camctr size
	if (mask & 0x8)
		glbin_view_def.m_camctr_size = m_render_view->m_camctr_size;
	//fps
	if (mask & 0x10)
		glbin_view_def.m_draw_info = m_render_view->m_draw_info;
	//selection
	if (mask & 0x20)
		glbin_view_def.m_draw_legend = m_render_view->m_draw_legend;
	//mouse focus
	if (mask & 0x40)
		glbin_view_def.m_mouse_focus = m_render_view->m_mouse_focus;
	//ortho/persp
	if (mask & 0x80)
	{
		glbin_view_def.m_persp = m_render_view->GetPersp();
		glbin_view_def.m_aov = m_render_view->GetAov();
		glbin_view_def.m_cam_mode = m_render_view->GetCamMode();
	}
	//rotations
	if (mask & 0x100)
	{
		glbin_view_def.m_rot = m_render_view->GetRotations();
		glbin_view_def.m_rot_lock = m_render_view->GetRotLock();
		glbin_view_def.m_rot_slider = m_rot_slider;
	}
	//depth atten
	if (mask & 0x200)
	{
		glbin_view_def.m_use_fog = m_render_view->GetFog();
		glbin_view_def.m_fog_intensity = m_render_view->GetFogIntensity();
	}
	//scale factor
	if (mask & 0x400)
	{
		glbin_view_def.m_pin_rot_center = m_render_view->m_pin_rot_ctr;
		glbin_view_def.m_scale_factor = m_render_view->m_scale_factor;
		glbin_view_def.m_scale_mode = m_render_view->m_scale_mode;
	}
	//camera center
	if (mask & 0x800)
		glbin_view_def.m_center = m_render_view->GetCenters();
	//colormap
	if (mask & 0x1000)
		glbin_view_def.m_colormap_disp = m_render_view->m_colormap_disp;
}

void RenderViewPanel::LoadSettings()
{
	glbin_view_def.Apply(m_render_view);
	m_rot_slider = glbin_view_def.m_rot_slider;

	FluoRefresh(2, {}, { GetViewId() });
}

