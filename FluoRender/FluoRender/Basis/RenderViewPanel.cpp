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

#include <FLIVR/ShaderProgram.h>
#include <Global/Global.h>
#include <MainFrame.h>
#include <RenderViewPanel.h>
#include <RenderCanvas.h>
#include <ClipPlanePanel.h>
#include <wxSingleSlider.h>
#include <wxUndoableScrollBar.h>
#include <wxUndoableToolbar.h>
#include <wxUndoableColorPicker.h>
#include <wx/utils.h>
#include <wx/valnum.h>
#include <tiffio.h>
#include <algorithm>
#include <limits>
#include <png_resource.h>
#include <img/icons.h>
#include <wx/stdpaths.h>
#include <wx/display.h>

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
	m_draw_scalebar(0),
	m_bg_color_inv(false),
	m_draw_clip(false),
	m_rot_slider(true)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);

	wxLogNull logNo;
	//full frame
	m_full_frame = new wxFrame((wxFrame*)NULL, wxID_ANY, "FluoRender");
	m_view_sizer = new wxBoxSizer(wxVERTICAL);

	m_dpi_sf = GetDPIScaleFactor();
	m_dpi_sf2 = std::round(m_dpi_sf - 0.1);
	m_dpi_sf2 = m_dpi_sf2 < m_dpi_sf ? m_dpi_sf : 1;

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
	m_glview = new RenderCanvas(frame, this, attriblist, sharedContext);
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
		sharedContext = new wxGLContext(m_glview, NULL, &contextAttrs);
		if (!sharedContext->IsOK())
		{
			contextAttrs.Reset();
			contextAttrs.PlatformDefaults().EndList();
			sharedContext = new wxGLContext(m_glview, NULL, &contextAttrs);
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
			//sharedContext->SetCurrent(*m_glview);
			m_glview->SetCurrent(*sharedContext);
			m_glview->m_glRC = sharedContext;
			m_glview->m_set_gl = true;
		}
	}
	m_glview->SetCanFocus(false);
	m_view_sizer->Add(m_glview, 1, wxEXPAND);
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
	int ret = m_glview->GetPixelFormat(&pfd);
#endif
	
	//get actual version
	glGetIntegerv(GL_MAJOR_VERSION, &gl_major_ver);
	glGetIntegerv(GL_MINOR_VERSION, &gl_minor_ver);
	glbin_settings.m_gl_major_ver = gl_major_ver;
	glbin_settings.m_gl_minor_ver = gl_minor_ver;

	CreateBar();

	//add controls
	glbin.add_undo_control(m_options_toolbar);
	glbin.add_undo_control(m_bg_color_picker);
	glbin.add_undo_control(m_aov_sldr);
	glbin.add_undo_control(m_options_toolbar2);
	glbin.add_undo_control(m_depth_atten_btn);
	glbin.add_undo_control(m_depth_atten_factor_sldr);
	glbin.add_undo_control(m_scale_factor_sldr);
	glbin.add_undo_control(m_rot_lock_btn);
	glbin.add_undo_control(m_x_rot_sldr);
	glbin.add_undo_control(m_y_rot_sldr);
	glbin.add_undo_control(m_z_rot_sldr);

}

RenderViewPanel::~RenderViewPanel()
{
	if (m_glview)
		delete m_glview;
	if (m_full_frame)
		delete m_full_frame;

	//delete controls
	glbin.del_undo_control(m_options_toolbar);
	glbin.del_undo_control(m_bg_color_picker);
	glbin.del_undo_control(m_aov_sldr);
	glbin.del_undo_control(m_options_toolbar2);
	glbin.del_undo_control(m_depth_atten_btn);
	glbin.del_undo_control(m_depth_atten_factor_sldr);
	glbin.del_undo_control(m_scale_factor_sldr);
	glbin.del_undo_control(m_rot_lock_btn);
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
	wxStaticText *st1, *st2, *st3;

	bool inverse_slider = glbin_settings.m_inverse_slider;
	wxBitmap bitmap;
	wxImage image;
	//bar top///////////////////////////////////////////////////
	wxBoxSizer* sizer_h_1 = new wxBoxSizer(wxHORIZONTAL);
	//toolbar 1
	m_options_toolbar = new wxUndoableToolbar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	m_options_toolbar->SetDoubleBuffered(true);
	wxSize tbs = m_options_toolbar->GetSize();
	wxSize toolsize = FromDIP(wxSize(tbs.y, tbs.y));
	m_options_toolbar->SetToolBitmapSize(toolsize);

	//blend mode
	bitmap = wxGetBitmap(layers, m_dpi_sf2);
	m_options_toolbar->AddCheckTool(
		ID_VolumeSeqRd, "Layer",
		bitmap, wxNullBitmap,
		"Render View as Layers",
		"Render View as Layers");
	bitmap = wxGetBitmap(depth, m_dpi_sf2);
	m_options_toolbar->AddCheckTool(
		ID_VolumeMultiRd, "Depth",
		bitmap, wxNullBitmap,
		"Render View by Depth",
		"Render View by Depth");
	bitmap = wxGetBitmap(composite, m_dpi_sf2);
	m_options_toolbar->AddCheckTool(
		ID_VolumeCompRd, "Compo",
		bitmap, wxNullBitmap,
		"Render View as a Composite of Colors",
		"Render View as a Composite of Colors");

	//capture
	bitmap = wxGetBitmap(camera, m_dpi_sf2);
	m_options_toolbar->AddTool(
		ID_CaptureBtn, "Snap",
		bitmap,
		"Capture Render View as an image");

	//info
	bitmap = wxGetBitmap(info, m_dpi_sf2);
	m_options_toolbar->AddCheckTool(
		ID_InfoChk, "Info",
		bitmap, wxNullBitmap,
		"Toggle View of FPS and Mouse Position",
		"Toggle View of FPS and Mouse Position");

	//cam center
	bitmap = wxGetBitmap(axis, m_dpi_sf2);
	m_options_toolbar->AddCheckTool(
		ID_CamCtrChk, "Axis",
		bitmap, wxNullBitmap,
		"Toggle View of the Center Axis",
		"Toggle View of the Center Axis");

	//legend
	bitmap = wxGetBitmap(legend, m_dpi_sf2);
	m_options_toolbar->AddCheckTool(
		ID_LegendChk, "Legend",
		bitmap, wxNullBitmap,
		"Toggle View of the Legend",
		"Toggle View of the Legend");

	//colormap
	bitmap = wxGetBitmap(colormap, m_dpi_sf2);
	m_options_toolbar->AddCheckTool(
		ID_ColormapChk, "Colormap",
		bitmap, wxNullBitmap,
		"Toggle View of the Colormap Sample",
		"Toggle View of the Colormap Sample");

	//scale bar
	bitmap = wxGetBitmap(scalebar, m_dpi_sf2);
	m_options_toolbar->AddTool(
		ID_ScaleBar, "Scale", bitmap,
		"Toggle Scalebar Options (Off, On, On with text)");
	m_options_toolbar->Bind(wxEVT_TOOL, &RenderViewPanel::OnToolBar, this);
	m_options_toolbar->Realize();

	sizer_h_1->Add(40, 40);
	sizer_h_1->Add(m_options_toolbar, 0, wxALIGN_CENTER);

	m_scale_text = new wxTextCtrl(this, wxID_ANY, "50",
		wxDefaultPosition, FromDIP(wxSize(35, tbs.y-3)), 0, vald_int);
	m_scale_cmb = new wxComboBox(this, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(50, tbs.y)), 0, NULL, wxCB_READONLY);
	std::vector<wxString> scale_list = { "nm", L"\u03BCm", "mm" };
	m_scale_cmb->Append(scale_list);
	m_scale_text->Bind(wxEVT_TEXT, &RenderViewPanel::OnScaleText, this);
	m_scale_cmb->Bind(wxEVT_TEXT, &RenderViewPanel::OnScaleUnit, this);
	sizer_h_1->Add(m_scale_text, 0, wxALIGN_CENTER);
	sizer_h_1->Add(m_scale_cmb, 0, wxALIGN_CENTER);

	sizer_h_1->AddStretchSpacer(1);

	//background
	m_bg_color_picker = new wxUndoableColorPicker(this,
		wxID_ANY, wxColor(), wxDefaultPosition, FromDIP(wxSize(40, 20)));
	wxSize bs = m_bg_color_picker->GetSize();
	m_bg_inv_btn = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	m_bg_inv_btn->SetDoubleBuffered(true);
	m_bg_inv_btn->SetToolBitmapSize(toolsize);
	bitmap = wxGetBitmap(invert, m_dpi_sf2);
	m_bg_inv_btn->AddCheckTool(
		0, "Invert",
		bitmap, wxNullBitmap,
		"Inver background color",
		"Inver background color");
	m_bg_inv_btn->Realize();
	m_bg_color_picker->Bind(wxEVT_COLOURPICKER_CHANGED, &RenderViewPanel::OnBgColorChange, this);
	m_bg_inv_btn->Bind(wxEVT_TOOL, &RenderViewPanel::OnBgInvBtn, this);
	sizer_h_1->Add(m_bg_color_picker, 0, wxALIGN_CENTER);
	sizer_h_1->Add(m_bg_inv_btn, 0, wxALIGN_CENTER);

	//angle of view
	st2 = new wxStaticText(this, wxID_ANY, "AOV:");
	m_aov_sldr = new wxSingleSlider(this, wxID_ANY, 45, 10, 100,
		wxDefaultPosition, FromDIP(wxSize(100, 20)), wxSL_HORIZONTAL);
	m_aov_text = new wxTextCtrl(this, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(40, 20)), 0, vald_int);
	m_aov_sldr->Bind(wxEVT_IDLE, &RenderViewPanel::OnAovSldrIdle, this);
	m_aov_sldr->Bind(wxEVT_SCROLL_CHANGED, &RenderViewPanel::OnAovChange, this);
	m_aov_text->Bind(wxEVT_TEXT, &RenderViewPanel::OnAovText, this);
	sizer_h_1->Add(st2, 0, wxALIGN_CENTER);
	sizer_h_1->Add(m_aov_sldr, 0, wxALIGN_CENTER);
	sizer_h_1->Add(m_aov_text, 0, wxALIGN_CENTER);

	//free fly
	m_options_toolbar2 = new wxUndoableToolbar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	m_options_toolbar2->SetToolBitmapSize(toolsize);
	m_options_toolbar2->SetDoubleBuffered(true);
	bitmap = wxGetBitmap(freefly, m_dpi_sf2);
	m_options_toolbar2->AddCheckTool(
		ID_FreeChk, "Free Fly",
		bitmap, wxNullBitmap,
		"Change the camera to a 'Free-Fly' Mode",
		"Change the camera to a 'Free-Fly' Mode");

	//save default
	bitmap = wxGetBitmap(save_settings, m_dpi_sf2);
	m_options_toolbar2->AddTool(
		ID_DefaultBtn, "Save", bitmap,
		"Set Default Render View Settings");
	m_options_toolbar2->Bind(wxEVT_TOOL, &RenderViewPanel::OnToolBar2, this);
	m_options_toolbar2->Realize();
	sizer_h_1->Add(m_options_toolbar2, 0, wxALIGN_CENTER);

	//full screen
	m_full_screen_btn = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_FLAT|wxTB_NODIVIDER);
	m_full_screen_btn->SetToolBitmapSize(toolsize);
	m_full_screen_btn->SetDoubleBuffered(true);
	bitmap = wxGetBitmap(full_view, m_dpi_sf2);
	m_full_screen_btn->AddTool(
		0, "Full Screen",
		bitmap, "Show full screen");
	m_full_screen_btn->Bind(wxEVT_TOOL, &RenderViewPanel::OnFullScreen, this);
	m_full_screen_btn->Realize();
	sizer_h_1->Add(m_full_screen_btn, 0, wxALIGN_CENTER);

	//bar left///////////////////////////////////////////////////
	wxBoxSizer* sizer_v_3 = new wxBoxSizer(wxVERTICAL);
	//depth attenuation
	m_depth_atten_btn = new wxUndoableToolbar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	m_depth_atten_btn->SetToolBitmapSize(toolsize);
	m_depth_atten_btn->SetDoubleBuffered(true);
	bitmap = wxGetBitmap(no_depth_atten, m_dpi_sf2);
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
		wxDefaultPosition, FromDIP(wxSize(40, 20)), 0, vald_fp2);
	m_depth_atten_factor_sldr->Bind(wxEVT_SCROLL_CHANGED, &RenderViewPanel::OnDepthAttenChange, this);
	m_depth_atten_factor_text->Bind(wxEVT_TEXT, &RenderViewPanel::OnDepthAttenEdit, this);
	//reset
	m_depth_atten_reset_btn = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	m_depth_atten_reset_btn->SetToolBitmapSize(toolsize);
	m_depth_atten_reset_btn->SetDoubleBuffered(true);
	bitmap = wxGetBitmap(reset, m_dpi_sf2);
	m_depth_atten_reset_btn->AddTool(
		0, "Reset",
		bitmap, "Reset Depth Attenuation Interval");
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
	m_pin_btn->SetToolBitmapSize(toolsize);
	m_pin_btn->SetDoubleBuffered(true);
	bitmap = wxGetBitmap(anchor_dark, m_dpi_sf2);
	m_pin_btn->AddCheckTool(0, "Pin",
		bitmap, wxNullBitmap,
		"Anchor the rotation center on data",
		"Anchor the rotation center on data");
	m_pin_btn->Bind(wxEVT_TOOL, &RenderViewPanel::OnPin, this);
	m_pin_btn->Realize();

	//centerize
	m_center_btn = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	m_center_btn->SetToolBitmapSize(toolsize);
	m_center_btn->SetDoubleBuffered(true);
	bitmap = wxGetBitmap(center, m_dpi_sf2);
	m_center_btn->AddTool(0, "Center",
		bitmap, "Center the Data on the Render View");
	m_center_btn->Bind(wxEVT_TOOL, &RenderViewPanel::OnCenter, this);
	m_center_btn->Realize();

	//one to one scale
	m_scale_121_btn = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	m_scale_121_btn->SetToolBitmapSize(toolsize);
	m_scale_121_btn->SetDoubleBuffered(true);
	bitmap = wxGetBitmap(ratio, m_dpi_sf2);
	m_scale_121_btn->AddTool(0, "1 to 1",
		bitmap, "Auto-size the data to a 1:1 ratio");
	m_scale_121_btn->Bind(wxEVT_TOOL, &RenderViewPanel::OnScale121, this);
	m_scale_121_btn->Realize();

	//scale slider
	ls = inverse_slider ? wxSL_VERTICAL : (wxSL_VERTICAL | wxSL_INVERSE);
	m_scale_factor_sldr = new wxSingleSlider(this, wxID_ANY, 100, 50, 999,
		wxDefaultPosition, wxDefaultSize, ls, wxDefaultValidator, "test");
	m_scale_factor_text = new wxTextCtrl(this, wxID_ANY, "100",
		wxDefaultPosition, FromDIP(wxSize(40, 20)), 0, vald_int);
	m_scale_factor_spin = new wxSpinButton(this, wxID_ANY,
		wxDefaultPosition, FromDIP(wxSize(40, 20)));
	m_scale_factor_spin->SetRange(-0x8000, 0x7fff);
	m_scale_factor_sldr->Bind(wxEVT_SCROLL_CHANGED, &RenderViewPanel::OnScaleFactorChange, this);
	m_scale_factor_text->Bind(wxEVT_TEXT, &RenderViewPanel::OnScaleFactorEdit, this);
	m_scale_factor_spin->Bind(wxEVT_SPIN_UP, &RenderViewPanel::OnScaleFactorSpinDown, this);
	m_scale_factor_spin->Bind(wxEVT_SPIN_DOWN, &RenderViewPanel::OnScaleFactorSpinUp, this);
	m_scale_reset_btn = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	m_scale_reset_btn->SetToolBitmapSize(toolsize);
	m_scale_reset_btn->SetDoubleBuffered(true);
	bitmap = wxGetBitmap(reset, m_dpi_sf2);
	m_scale_reset_btn->AddTool(0, "Reset",
		bitmap, "Reset the Zoom");
	m_scale_reset_btn->Bind(wxEVT_TOOL, &RenderViewPanel::OnScaleReset, this);
	m_scale_reset_btn->Realize();
	m_scale_mode_btn = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	m_scale_mode_btn->SetToolBitmapSize(toolsize);
	m_scale_mode_btn->SetDoubleBuffered(true);
	bitmap = wxGetBitmap(zoom_view, m_dpi_sf2);
	m_scale_mode_btn->AddTool(
		0, "Switch zoom ratio mode",
		bitmap, "View-based zoom ratio");
	m_scale_mode_btn->Bind(wxEVT_TOOL, &RenderViewPanel::OnScaleMode, this);
	m_scale_mode_btn->Realize();
	sizer_v_4->AddSpacer(50);
	sizer_v_4->Add(m_pin_btn, 0, wxALIGN_CENTER);
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
	m_rot_lock_btn = new wxUndoableToolbar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	m_rot_lock_btn->SetToolBitmapSize(toolsize);
	m_rot_lock_btn->SetDoubleBuffered(true);
	bitmap = wxGetBitmap(gear_dark, m_dpi_sf2);
	m_rot_lock_btn->AddCheckTool(ID_RotLockChk, "45 Angles",
		bitmap, wxNullBitmap,
		"Confine all angles to 45 Degrees",
		"Confine all angles to 45 Degrees");
	bitmap = wxGetBitmap(slider_type_rot, m_dpi_sf2);
	m_rot_lock_btn->AddCheckTool(ID_RotSliderType, "Slider Style",
		bitmap, wxNullBitmap,
		"Choose slider style between jog and normal",
		"Choose slider style between jog and normal");
	m_rot_lock_btn->Bind(wxEVT_TOOL, &RenderViewPanel::OnRotLockCheck, this);
	m_rot_lock_btn->Realize();

	st1 = new wxStaticText(this, 0, "X:");
	m_x_rot_sldr = new wxUndoableScrollBar(this, ID_RotXScroll);
	m_x_rot_sldr->SetScrollbar(180,60,420,15);
	m_x_rot_text = new wxTextCtrl(this, wxID_ANY, "0.0",
		wxDefaultPosition, FromDIP(wxSize(45,20)), 0, vald_fp1);
	st2 = new wxStaticText(this, 0, "Y:");
	m_y_rot_sldr = new wxUndoableScrollBar(this, ID_RotYScroll);
	m_y_rot_sldr->SetScrollbar(180,60,420,15);
	m_y_rot_text = new wxTextCtrl(this, wxID_ANY, "0.0",
		wxDefaultPosition, FromDIP(wxSize(45,20)), 0, vald_fp1);
	st3 = new wxStaticText(this, 0, "Z:");
	m_z_rot_sldr = new wxUndoableScrollBar(this, ID_RotZScroll);
	m_z_rot_sldr->SetScrollbar(180,60,420,15);
	m_z_rot_text = new wxTextCtrl(this, wxID_ANY, "0.0",
		wxDefaultPosition, FromDIP(wxSize(45,20)), 0, vald_fp1);
	m_x_rot_text->Bind(wxEVT_TEXT, &RenderViewPanel::OnRotEdit, this);
	m_y_rot_text->Bind(wxEVT_TEXT, &RenderViewPanel::OnRotEdit, this);
	m_z_rot_text->Bind(wxEVT_TEXT, &RenderViewPanel::OnRotEdit, this);
	m_x_rot_sldr->Bind(wxEVT_SCROLL_CHANGED, &RenderViewPanel::OnRotScroll, this);
	m_y_rot_sldr->Bind(wxEVT_SCROLL_CHANGED, &RenderViewPanel::OnRotScroll, this);
	m_z_rot_sldr->Bind(wxEVT_SCROLL_CHANGED, &RenderViewPanel::OnRotScroll, this);

	//ortho view selector
	m_ortho_view_cmb = new wxComboBox(this, wxID_ANY, "",
		wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY);
	std::vector<wxString> ov_list = { "+X", "-X", "+Y", "-Y", "+Z", "-Z", "NA" };
	m_ortho_view_cmb->Append(ov_list);
	m_ortho_view_cmb->Bind(wxEVT_COMBOBOX, &RenderViewPanel::OnOrthoViewSelected, this);

	//set reset
	m_reset_btn = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	m_reset_btn->SetToolBitmapSize(toolsize);
	m_reset_btn->SetDoubleBuffered(true);
	bitmap = wxGetBitmap(zrot, m_dpi_sf2);
	m_reset_btn->AddTool(ID_ZeroRotBtn, "Set Zeros",
		bitmap, "Set current angles as zeros");
	bitmap = wxGetBitmap(reset, m_dpi_sf2);
	m_reset_btn->AddTool(ID_RotResetBtn,"Reset",
		bitmap, "Reset Rotations");
	m_reset_btn->Bind(wxEVT_TOOL, &RenderViewPanel::OnRotReset, this);
	m_reset_btn->Realize();

	sizer_h_2->AddSpacer(40);
	sizer_h_2->Add(m_rot_lock_btn, 0, wxALIGN_CENTER);
	sizer_h_2->Add(st1, 0, wxALIGN_CENTER);
	sizer_h_2->Add(m_x_rot_sldr, 1, wxALIGN_CENTER);
	sizer_h_2->Add(m_x_rot_text, 0, wxALIGN_CENTER);
	sizer_h_2->Add(5, 5, 0);
	sizer_h_2->Add(st2, 0, wxALIGN_CENTER);
	sizer_h_2->Add(m_y_rot_sldr, 1, wxALIGN_CENTER);
	sizer_h_2->Add(m_y_rot_text, 0, wxALIGN_CENTER);
	sizer_h_2->Add(5, 5, 0);
	sizer_h_2->Add(st3, 0, wxALIGN_CENTER);
	sizer_h_2->Add(m_z_rot_sldr, 1, wxALIGN_CENTER);
	sizer_h_2->Add(m_z_rot_text, 0, wxALIGN_CENTER);
	sizer_h_2->Add(5, 5, 0);
	sizer_h_2->Add(m_ortho_view_cmb, 0, wxALIGN_CENTER, 2);
	sizer_h_2->Add(m_reset_btn, 0, wxALIGN_CENTER);
	sizer_h_2->AddSpacer(40);

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
	if (!m_glview)
		return;

	int ival;
	bool bval;
	double dval;

	bool update_all = vc.empty();

	//blend mode
	if (update_all || FOUND_VALUE(gstMixMethod))
	{
		ival = m_glview->GetVolMethod();
		int test = m_options_toolbar->GetToolsCount();
		m_options_toolbar->ToggleTool(ID_VolumeSeqRd, ival == VOL_METHOD_SEQ);
		m_options_toolbar->SetToolNormalBitmap(ID_VolumeSeqRd,
			ival == VOL_METHOD_SEQ ?
			wxGetBitmap(layers, m_dpi_sf2) :
			wxGetBitmap(layers_off, m_dpi_sf2));
		m_options_toolbar->ToggleTool(ID_VolumeMultiRd, ival == VOL_METHOD_MULTI);
		m_options_toolbar->SetToolNormalBitmap(ID_VolumeMultiRd,
			ival == VOL_METHOD_MULTI ?
			wxGetBitmap(depth, m_dpi_sf2) :
			wxGetBitmap(depth_off, m_dpi_sf2));
		m_options_toolbar->ToggleTool(ID_VolumeCompRd, ival == VOL_METHOD_COMP);
		m_options_toolbar->SetToolNormalBitmap(ID_VolumeCompRd,
			ival == VOL_METHOD_COMP ?
			wxGetBitmap(composite, m_dpi_sf2) :
			wxGetBitmap(composite_off, m_dpi_sf2));
	}

	//info
	if (update_all || FOUND_VALUE(gstDrawInfo))
	{
		bval = m_glview->m_draw_info & 1;
		m_options_toolbar->ToggleTool(ID_InfoChk, bval);
	}

	//cam center
	if (update_all || FOUND_VALUE(gstDrawCamCtr))
	{
		bval = m_glview->m_draw_camctr;
		m_options_toolbar->ToggleTool(ID_CamCtrChk, bval);
	}

	//legend
	if (update_all || FOUND_VALUE(gstDrawLegend))
	{
		bval = m_glview->m_draw_legend;
		m_options_toolbar->ToggleTool(ID_LegendChk, bval);
	}

	//colormap
	if (update_all || FOUND_VALUE(gstDrawColormap))
	{
		bval = m_glview->m_draw_colormap;
		m_options_toolbar->ToggleTool(ID_ColormapChk, bval);
	}

	//scale bar
	if (update_all || FOUND_VALUE(gstDrawScaleBar))
	{
		switch (m_draw_scalebar)
		{
		case 0:
			m_options_toolbar->SetToolNormalBitmap(ID_ScaleBar,
				wxGetBitmap(scale_text_off, m_dpi_sf2));
			m_scale_text->Enable();
			m_scale_cmb->Disable();
			break;
		case 1:
			m_options_toolbar->SetToolNormalBitmap(ID_ScaleBar,
				wxGetBitmap(scale_text, m_dpi_sf2));
			m_scale_text->Enable();
			m_scale_cmb->Enable();
			break;
		case 2:
			m_options_toolbar->SetToolNormalBitmap(ID_ScaleBar,
				wxGetBitmap(scalebar, m_dpi_sf2));
			m_scale_text->Disable();
			m_scale_cmb->Disable();
			break;
		}
	}
	if (update_all || FOUND_VALUE(gstScaleBarUnit))
		m_scale_cmb->Select(m_glview->m_sb_unit);

	//background
	if (update_all || FOUND_VALUE(gstBgColor))
	{
		fluo::Color c = m_glview->GetBackgroundColor();
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
				wxGetBitmap(invert, m_dpi_sf2));
		else
			m_bg_inv_btn->SetToolNormalBitmap(0,
				wxGetBitmap(invert_off, m_dpi_sf2));
	}

	//angle of view
	if (update_all || FOUND_VALUE(gstAov))
	{
		dval = m_glview->GetAov();
		bval = m_glview->GetPersp();
		m_aov_sldr->ChangeValue(bval ? std::round(dval) : 10);
		m_aov_text->ChangeValue(bval ? wxString::Format("%d",
			(int)(std::round(dval))) : wxString("Ortho"));
	}

	//free fly
	if (update_all || FOUND_VALUE(gstFree))
		m_options_toolbar2->ToggleTool(ID_FreeChk, m_glview->GetFree());

	//depthe attenuation
	if (update_all || FOUND_VALUE(gstDepthAtten))
	{
		bval = m_glview->m_use_fog;
		m_depth_atten_btn->ToggleTool(0, bval);
		if (bval)
			m_depth_atten_btn->SetToolNormalBitmap(0,
				wxGetBitmap(depth_atten, m_dpi_sf2));
		else
			m_depth_atten_btn->SetToolNormalBitmap(0,
				wxGetBitmap(no_depth_atten, m_dpi_sf2));
		m_depth_atten_factor_sldr->Enable(bval);
		m_depth_atten_factor_text->Enable(bval);
	}
	if (update_all || FOUND_VALUE(gstDaInt))
	{
		dval = m_glview->GetFogIntensity();
		m_depth_atten_factor_sldr->ChangeValue(std::round(dval * 100));
		m_depth_atten_factor_text->ChangeValue(wxString::Format("%.2f", dval));
	}

	//pin rotation center
	if (update_all || FOUND_VALUE(gstPinRotCtr))
	{
		bval = m_glview->m_pin_rot_center;
		m_pin_btn->ToggleTool(0, bval);
		if (bval)
			m_pin_btn->SetToolNormalBitmap(0,
				wxGetBitmap(pin, m_dpi_sf2));
		else
			m_pin_btn->SetToolNormalBitmap(0,
				wxGetBitmap(anchor_dark, m_dpi_sf2));
	}

	//scale factor
	if (update_all || FOUND_VALUE(gstScaleFactor))
	{
		double scale = m_glview->m_scale_factor;
		ival = std::round(scale * 100);
		m_scale_factor_sldr->ChangeValue(ival);
		m_scale_factor_text->ChangeValue(wxString::Format("%d", ival));
	}
	//scale mode
	if (update_all || FOUND_VALUE(gstScaleMode))
	{
		switch (m_glview->m_scale_mode)
		{
		case 0:
			m_scale_mode_btn->SetToolNormalBitmap(0,
				wxGetBitmap(zoom_view, m_dpi_sf2));
			m_scale_mode_btn->SetToolShortHelp(0,
				"View-based zoom ratio");
			m_scale_mode_btn->SetToolLongHelp(0,
				"View-based zoom ratio (View entire data set at 100%)");
			break;
		case 1:
			m_scale_mode_btn->SetToolNormalBitmap(0,
				wxGetBitmap(zoom_pixel, m_dpi_sf2));
			m_scale_mode_btn->SetToolShortHelp(0,
				"Pixel-based zoom ratio");
			m_scale_mode_btn->SetToolLongHelp(0,
				"Pixel-based zoom ratio (View 1 data pixel to 1 screen pixel at 100%)");
			break;
		case 2:
			m_scale_mode_btn->SetToolNormalBitmap(0,
				wxGetBitmap(zoom_data, m_dpi_sf2));
			m_scale_mode_btn->SetToolShortHelp(0,
				"Data-based zoom ratio");
			m_scale_mode_btn->SetToolLongHelp(0,
				"Data-based zoom ratio (View with consistent scale bar sizes)");
			break;
		}
	}

	//lock rot
	if (update_all || FOUND_VALUE(gstGearedEnable))
	{
		m_rot_lock_btn->ToggleTool(ID_RotLockChk, m_glview->m_rot_lock);
		if (m_glview->m_rot_lock)
			m_rot_lock_btn->SetToolNormalBitmap(ID_RotLockChk,
				wxGetBitmap(gear_45, m_dpi_sf2));
		else
			m_rot_lock_btn->SetToolNormalBitmap(ID_RotLockChk,
				wxGetBitmap(gear_dark, m_dpi_sf2));
	}

	//slider type
	if (update_all || FOUND_VALUE(gstRotSliderMode))
	{
		m_rot_lock_btn->ToggleTool(ID_RotSliderType, m_rot_slider);
		if (m_rot_slider)
		{
			m_rot_lock_btn->SetToolNormalBitmap(ID_RotSliderType,
				wxGetBitmap(slider_type_rot, m_dpi_sf2));
			if (m_x_rot_sldr->GetMode() != 1)
			{
				m_x_rot_sldr->SetMode(1);
				m_y_rot_sldr->SetMode(1);
				m_z_rot_sldr->SetMode(1);
			}
		}
		else
		{
			m_rot_lock_btn->SetToolNormalBitmap(ID_RotSliderType,
				wxGetBitmap(slider_type_pos, m_dpi_sf2));
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
		double rotx, roty, rotz;
		m_glview->GetRotations(rotx, roty, rotz);
		m_x_rot_sldr->ChangeValue(std::round(rotx));
		m_y_rot_sldr->ChangeValue(std::round(roty));
		m_z_rot_sldr->ChangeValue(std::round(rotz));
		m_x_rot_text->ChangeValue(wxString::Format("%.1f", rotx));
		m_y_rot_text->ChangeValue(wxString::Format("%.1f", roty));
		m_z_rot_text->ChangeValue(wxString::Format("%.1f", rotz));
		m_ortho_view_cmb->Select(m_glview->GetOrientation());
	}
}

void RenderViewPanel::SetVolumeMethod(int val)
{
	m_glview->SetVolMethod(val);

	FluoRefresh(true, 2, { gstMixMethod }, { m_frame->GetView(m_glview) });
}

void RenderViewPanel::Capture()
{
	//reset enlargement
	RenderCanvas::SetEnlarge(false);
	RenderCanvas::SetEnlargeScale(1.0);

	wxFileDialog file_dlg(m_frame, "Save captured image", "", "", "*.tif", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	file_dlg.SetExtraControlCreator(CreateExtraCaptureControl);
	int rval = file_dlg.ShowModal();
	if (rval == wxID_OK)
	{
		m_glview->m_cap_file = file_dlg.GetDirectory() + GETSLASH() + file_dlg.GetFilename();
		m_glview->m_capture = true;
		RefreshGL();

		if (glbin_settings.m_prj_save)
		{
			wxString new_folder;
			new_folder = m_glview->m_cap_file + "_project";
			MkDirW(new_folder.ToStdWstring());
			wxString prop_file = new_folder + GETSLASH() + file_dlg.GetFilename() + "_project.vrp";
			bool inc = wxFileExists(prop_file) &&
				glbin_settings.m_prj_save_inc;
			m_frame->SaveProject(prop_file, inc);
		}
	}
}

void RenderViewPanel::SetInfo(bool val)
{
	if (val)
		m_glview->m_draw_info |= 1;
	else
		m_glview->m_draw_info &= ~1;

	FluoRefresh(true, 2, { gstDrawInfo }, { m_frame->GetView(m_glview) });
}

void RenderViewPanel::SetDrawCamCtr(bool val)
{
	m_glview->m_draw_camctr = val;

	FluoRefresh(true, 2, { gstDrawCamCtr }, { m_frame->GetView(m_glview) });
}

void RenderViewPanel::SetLegend(bool val)
{
	m_glview->m_draw_legend = val;

	FluoRefresh(true, 2, { gstDrawLegend }, { m_frame->GetView(m_glview) });
}

void RenderViewPanel::SetDrawColormap(bool val)
{
	m_glview->m_draw_colormap = val;

	FluoRefresh(true, 2, { gstDrawColormap }, { m_frame->GetView(m_glview) });
}

void RenderViewPanel::SetDrawScalebar(int val)
{
	switch (val)
	{
	case 0:
		m_glview->m_disp_scale_bar = true;
		m_glview->m_disp_scale_bar_text = false;
		break;
	case 1:
		m_glview->m_disp_scale_bar = true;
		m_glview->m_disp_scale_bar_text = true;
		break;
	case 2:
		m_glview->m_disp_scale_bar = false;
		m_glview->m_disp_scale_bar_text = false;
		break;
	}

	FluoRefresh(true, 2, { gstDrawScaleBar }, { m_frame->GetView(m_glview) });
}

void RenderViewPanel::SetScaleText(double val)
{
	wxString str, num_text, unit_text;
	num_text = wxString::Format("%d", (int)val);
	switch (m_glview->m_sb_unit)
	{
	case 0:
		unit_text = "nm";
		break;
	case 1:
	default:
		unit_text = L"\u03BCm";
		break;
	case 2:
		unit_text = "mm";
		break;
	}
	str = num_text + " " + unit_text;
	m_glview->SetSBText(str);
	m_glview->SetScaleBarLen(val);
	m_glview->m_sb_num = num_text;

	FluoRefresh(true, 2, {gstNull}, { m_frame->GetView(m_glview) });
}

void RenderViewPanel::SetScaleUnit(int val)
{
	m_glview->m_sb_unit = val;
	double dval = m_glview->m_sb_length;
	wxString str, num_text, unit_text;
	num_text = wxString::Format("%d", (int)dval);
	switch (val)
	{
	case 0:
		unit_text = "nm";
		break;
	case 1:
	default:
		unit_text = L"\u03BCm";
		break;
	case 2:
		unit_text = "mm";
		break;
	}
	str = num_text + " " + unit_text;
	m_glview->SetSBText(str);
	m_glview->SetScaleBarLen(dval);
	m_glview->m_sb_num = num_text;

	FluoRefresh(true, 2, { gstNull }, { m_frame->GetView(m_glview) });
}

void RenderViewPanel::SetBgColor(fluo::Color val)
{
	m_glview->SetBackgroundColor(val);

	FluoRefresh(true, 2, { gstBgColor }, { m_frame->GetView(m_glview) });
}

void RenderViewPanel::SetBgColorInvert(bool val)
{
	m_bg_color_inv = val;
	fluo::Color c = m_glview->GetBackgroundColor();
	c = fluo::Color(1.0, 1.0, 1.0) - c;
	m_glview->SetBackgroundColor(c);

	FluoRefresh(true, 2, { gstBgColor, gstBgColorInv }, { m_frame->GetView(m_glview) });
}

void RenderViewPanel::SetDrawClipPlanes(bool val)
{
	bool draw_clip = m_glview->m_draw_clip;
	if (val && !draw_clip)
	{
		m_glview->m_draw_clip = true;
		m_glview->m_clip_mask = -1;
		FluoRefresh(true, 2, {gstNull}, { m_frame->GetView(m_glview) });
	}
	else if (!val && draw_clip)
	{
		m_glview->m_draw_clip = false;
		FluoRefresh(true, 2, {gstNull}, { m_frame->GetView(m_glview) });
	}
}

void RenderViewPanel::SetAov(double val, bool notify)
{
	if (val < 11)
	{
		m_glview->SetPersp(false);
		m_glview->SetAov(10);
	}
	else if (val > 100)
	{
		m_glview->SetPersp(true);
		m_glview->SetAov(100);
	}
	else
	{
		m_glview->SetPersp(true);
		m_glview->SetAov(val);
	}

	if (notify)
		FluoRefresh(true, 2, { gstAov }, { m_frame->GetView(m_glview) });
	else
		FluoRefresh(true, 2, { gstNull }, { m_frame->GetView(m_glview) });
}

void RenderViewPanel::SetFree(bool val)
{
	m_glview->SetFree(val);
	if (!val)
	{
		if (m_glview->m_aov > 10)
			m_glview->SetPersp(true);
		else
			m_glview->SetPersp(false);
	}

	FluoRefresh(true, 2, { gstFree }, { m_frame->GetView(m_glview) });
}

void RenderViewPanel::SetFullScreen()
{
	if (m_glview->GetParent() != m_full_frame)
	{
		m_view_sizer->Detach(m_glview);
		m_glview->Reparent(m_full_frame);
		//get display id
		int disp_id = glbin_settings.m_disp_id;
		if (disp_id >= wxDisplay::GetCount())
			disp_id = 0;
		wxDisplay display(disp_id);
		wxRect rect = display.GetGeometry();
		m_full_frame->SetSize(rect.GetSize());
		m_full_frame->SetPosition(rect.GetPosition());
		m_full_frame->ShowFullScreen(true);
		m_glview->SetPosition(wxPoint(0, 0));
		m_glview->SetSize(m_full_frame->GetSize());
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
		m_glview->m_full_screen = true;
		m_glview->SetFocus();
		RefreshGL();
	}
	else
	{
		m_glview->Close();
	}
}

void RenderViewPanel::SetDepthAttenEnable(bool val)
{
	m_glview->SetFog(true);
	FluoRefresh(true, 2, { gstDepthAtten }, { m_frame->GetView(m_glview) });
}

void RenderViewPanel::SetDepthAtten(double val, bool notify)
{
	m_glview->SetFogIntensity(val);
	if (notify)
		FluoRefresh(true, 2, { gstDaInt }, { m_frame->GetView(m_glview) });
	else
		FluoRefresh(true, 2, { gstNull }, { m_frame->GetView(m_glview) });
}

void RenderViewPanel::SetPinRotCenter(bool val)
{
	m_glview->SetPinRotCenter(val);
	double scale = m_glview->m_scale_factor;
	switch (m_glview->m_scale_mode)
	{
		case 0:
			break;
		case 1:
			scale /= m_glview->Get121ScaleFactor();
			break;
		case 2:
		{
			VolumeData* vd = 0;
			if (m_glview->m_cur_vol)
				vd = m_glview->m_cur_vol;
			else if (m_glview->m_vd_pop_list.size())
				vd = m_glview->m_vd_pop_list[0];
			double spcx, spcy, spcz;
			if (vd)
			{
				vd->GetSpacings(spcx, spcy, spcz, vd->GetLevel());
				if (spcx > 0.0)
					scale /= m_glview->Get121ScaleFactor() * spcx;
			}
		}
		break;
	}

	if (val)
	{
		if (scale > glbin_settings.m_pin_threshold)
			m_glview->m_auto_update_rot_center = true;
		else
			m_glview->m_auto_update_rot_center = false;
	}
	else
	{
		if (scale > glbin_settings.m_pin_threshold)
			m_glview->m_auto_update_rot_center = false;
		else
			m_glview->m_auto_update_rot_center = true;
	}
	FluoRefresh(true, 2, { gstPinRotCtr }, { m_frame->GetView(m_glview) });
}

void RenderViewPanel::SetCenter()
{
	m_glview->SetCenter();
	FluoRefresh(true, 2, { gstNull }, { m_frame->GetView(m_glview) });
}

void RenderViewPanel::SetScale121()
{
	m_glview->SetScale121();
	if (m_glview->m_mouse_focus)
		m_glview->SetFocus();
	FluoRefresh(true, 2, { gstNull }, { m_frame->GetView(m_glview) });
}

void RenderViewPanel::SetScaleFactor(double val, bool notify)
{
	switch (m_glview->m_scale_mode)
	{
		case 0:
			m_glview->m_scale_factor = val;
			break;
		case 1:
			m_glview->m_scale_factor = val * m_glview->Get121ScaleFactor();
			break;
		case 2:
		{
			VolumeData* vd = 0;
			if (m_glview->m_cur_vol)
				vd = m_glview->m_cur_vol;
			else if (m_glview->m_vd_pop_list.size())
				vd = m_glview->m_vd_pop_list[0];
			double spcx, spcy, spcz;
			if (vd)
			{
				vd->GetSpacings(spcx, spcy, spcz, vd->GetLevel());
				if (spcx > 0.0)
					m_glview->m_scale_factor = val * m_glview->Get121ScaleFactor() * spcx;
			}
		}
		break;
	}
	if (notify)
		FluoRefresh(true, 2, { gstScaleFactor }, { m_frame->GetView(m_glview) });
	else
		FluoRefresh(true, 2, { gstNull }, { m_frame->GetView(m_glview) });
}

void RenderViewPanel::SetScaleMode(int val)
{
	m_glview->m_scale_mode = val;
	FluoRefresh(true, 2, { gstScaleMode }, { m_frame->GetView(m_glview) });
}

void RenderViewPanel::SetRotLock(bool val)
{
	m_glview->SetRotLock(val);
	if (val)
	{
		double rotx, roty, rotz;
		m_glview->GetRotations(rotx, roty, rotz);
		rotx = (((int)(rotx / 45)) * 45);
		roty = (((int)(roty / 45)) * 45);
		rotz = (((int)(rotz / 45)) * 45);
		SetRotations(rotx, roty, rotz, true);
	}
	FluoRefresh(true, 2, { gstGearedEnable }, { m_frame->GetView(m_glview) });
}

void RenderViewPanel::SetSliderType(bool val)
{
	m_rot_slider = val;
	FluoRefresh(true, 2, { gstRotSliderMode }, { m_frame->GetView(m_glview) });
}

void RenderViewPanel::SetRotations(double rotx, double roty, double rotz, bool notify)
{
	m_glview->SetRotations(rotx, roty, rotz, false);
	if (notify)
		FluoRefresh(true, 2, { gstCamRotation }, { m_frame->GetView(m_glview) });
	else
		FluoRefresh(true, 2, { gstNull }, { m_frame->GetView(m_glview) });
}

void RenderViewPanel::SetZeroRotations()
{
	double rotx, roty, rotz;
	m_glview->GetRotations(rotx, roty, rotz);
	if (rotx == 0.0 &&
		roty == 0.0 &&
		rotz == 0.0)
	{
		//reset
		m_glview->ResetZeroRotations(rotx, roty, rotz);
		m_glview->SetRotations(rotx, roty, rotz, false);
	}
	else
	{
		m_glview->SetZeroRotations();
		m_glview->SetRotations(0.0, 0.0, 0.0, false);
	}
	FluoRefresh(true, 2, { gstCamRotation }, { m_frame->GetView(m_glview) });
}

void RenderViewPanel::OnToolBar(wxCommandEvent& event)
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
	case ID_CaptureBtn:
		Capture();
		break;
	case ID_InfoChk:
		SetInfo(m_options_toolbar->GetToolState(ID_InfoChk));
		break;
	case ID_CamCtrChk:
		SetDrawCamCtr(m_options_toolbar->GetToolState(ID_CamCtrChk));
		break;
	case ID_LegendChk:
		SetLegend(m_options_toolbar->GetToolState(ID_LegendChk));
		break;
	case ID_ColormapChk:
		SetDrawColormap(m_options_toolbar->GetToolState(ID_ColormapChk));
		break;
	case ID_ScaleBar:
		m_draw_scalebar += 1;
		m_draw_scalebar = m_draw_scalebar > 2 ? 0 : m_draw_scalebar;
		SetDrawScalebar(m_draw_scalebar);
		break;
	}

	event.Skip();
}

void RenderViewPanel::OnScaleText(wxCommandEvent& event)
{
	wxString str = m_scale_text->GetValue();
	double len;
	str.ToDouble(&len);
	SetScaleText(len);
	event.Skip();
}

void RenderViewPanel::OnScaleUnit(wxCommandEvent& event)
{
	SetScaleUnit(m_scale_cmb->GetSelection());
	event.Skip();
}

void RenderViewPanel::OnBgColorChange(wxColourPickerEvent& event)
{
	wxColor c = m_bg_color_picker->GetColour();
	fluo::Color color(c.Red() / 255.0, c.Green() / 255.0, c.Blue() / 255.0);
	SetBgColor(color);
	event.Skip();
}

void RenderViewPanel::OnBgInvBtn(wxCommandEvent& event)
{
	SetBgColorInvert(!m_bg_color_inv);
	event.Skip();
}

void RenderViewPanel::OnAovSldrIdle(wxIdleEvent& event)
{
	if (m_frame->GetClippingView()->GetHoldPlanes())
		return;
	if (m_glview->m_capture)
		return;

	wxPoint pos = wxGetMousePosition();
	wxRect reg = m_aov_sldr->GetScreenRect();
	wxWindow* window = wxWindow::FindFocus();
	if (window && reg.Contains(pos))
	{
		if (!m_draw_clip)
		{
			SetDrawClipPlanes(true);
			m_draw_clip = true;
		}
	}
	else
	{
		if (m_draw_clip)
		{
			SetDrawClipPlanes(false);
			m_draw_clip = false;
		}
	}
	event.Skip();
}

void RenderViewPanel::OnAovChange(wxScrollEvent& event)
{
	int val = m_aov_sldr->GetValue();
	bool bval = m_glview->m_persp;
	m_aov_text->ChangeValue(bval ? wxString::Format("%d", val) : wxString("Ortho"));

	SetAov(val, false);

	event.Skip();
}

void RenderViewPanel::OnAovText(wxCommandEvent& event)
{
	wxString str = m_aov_text->GetValue();
	int ival = 10;
	long val;
	if (str.ToLong(&val))
		ival = val;
	m_aov_sldr->ChangeValue(ival);
	SetAov(ival, false);
	event.Skip();
}

void RenderViewPanel::OnToolBar2(wxCommandEvent& event)
{
	int id = event.GetId();

	switch (id)
	{
	case ID_FreeChk:
		SetFree(m_options_toolbar2->GetToolState(ID_FreeChk));
		break;
	case ID_DefaultBtn:
		SaveDefault();
		break;
	}
	event.Skip();
}

void RenderViewPanel::OnFullScreen(wxCommandEvent& event)
{
	SetFullScreen();
	event.Skip();
}

void RenderViewPanel::OnDepthAttenCheck(wxCommandEvent& event)
{
	SetDepthAttenEnable(m_depth_atten_btn->GetToolState(0));
	event.Skip();
}

//bar left
void RenderViewPanel::OnDepthAttenChange(wxScrollEvent& event)
{
	double val = m_depth_atten_factor_sldr->GetValue() / 100.0;
	m_depth_atten_factor_text->ChangeValue(wxString::Format("%.2f", val));
	SetDepthAtten(val, false);
	event.Skip();
}

void RenderViewPanel::OnDepthAttenEdit(wxCommandEvent& event)
{
	wxString str = m_depth_atten_factor_text->GetValue();
	double val = 0;
	str.ToDouble(&val);
	m_depth_atten_factor_sldr->ChangeValue(std::round(val * 100));
	SetDepthAtten(val, false);
	event.Skip();
}

void RenderViewPanel::OnDepthAttenReset(wxCommandEvent& event)
{
	SetDepthAttenEnable(glbin_view_def.m_use_fog);
	SetDepthAtten(glbin_view_def.m_fog_intensity, true);
	event.Skip();
}

void RenderViewPanel::OnPin(wxCommandEvent& event)
{
	bool val = m_pin_btn->GetToolState(0);
	SetPinRotCenter(val);
	event.Skip();
}

void RenderViewPanel::OnCenter(wxCommandEvent& event)
{
	SetCenter();
	event.Skip();
}

void RenderViewPanel::OnScale121(wxCommandEvent& event)
{
	SetScale121();
	event.Skip();
}

void RenderViewPanel::OnScaleFactorChange(wxScrollEvent& event)
{
	int ival = m_scale_factor_sldr->GetValue();
	double dval = ival / 100.0;
	m_scale_factor_text->ChangeValue(wxString::Format("%d", ival));
	SetScaleFactor(dval, false);
	event.Skip();
}

void RenderViewPanel::OnScaleFactorEdit(wxCommandEvent& event)
{
	wxString str = m_scale_factor_text->GetValue();
	long val = 0;
	str.ToLong(&val);
	if (val > 0)
	{
		double dval = val / 100.0;
		m_scale_factor_sldr->ChangeValue(val);
		SetScaleFactor(dval, false);
	}
	event.Skip();
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
		SetScaleFactor(val / 100.0, true);
	event.Skip();
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
		SetScaleFactor(val / 100.0, true);
	event.Skip();
}

void RenderViewPanel::OnScaleReset(wxCommandEvent& event)
{
	SetScaleFactor(glbin_view_def.m_scale_factor, true);
	event.Skip();
}

void RenderViewPanel::OnScaleMode(wxCommandEvent& event)
{
	int mode = m_glview->m_scale_mode;
	mode += 1;
	mode = mode > 2 ? 0 : mode;
	SetScaleMode(mode);
	event.Skip();
}

void RenderViewPanel::OnRotLockCheck(wxCommandEvent& event)
{
	int id = event.GetId();

	switch (id)
	{
	case ID_RotLockChk:
		SetRotLock(m_rot_lock_btn->GetToolState(ID_RotLockChk));
		break;
	case ID_RotSliderType:
		SetSliderType(m_rot_lock_btn->GetToolState(ID_RotSliderType));
		break;
	}
	event.Skip();
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
	SetRotations(rotx, roty, rotz, false);
	event.Skip();
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
	SetRotations(rotx, roty, rotz, false);
	event.Skip();
}

void RenderViewPanel::OnOrthoViewSelected(wxCommandEvent& event)
{
	int sel = 6;
	if (m_ortho_view_cmb)
		sel = m_ortho_view_cmb->GetSelection();
	switch (sel)
	{
	case 0://+X
		m_glview->SetRotations(0.0, 90.0, 0.0, false);
		break;
	case 1://-X
		m_glview->SetRotations(0.0, 270.0, 0.0, false);
		break;
	case 2://+Y
		m_glview->SetRotations(90.0, 0.0, 0.0, false);
		break;
	case 3://-Y
		m_glview->SetRotations(270.0, 0.0, 0.0, false);
		break;
	case 4://+Z
		m_glview->SetRotations(0.0, 0.0, 0.0, false);
		break;
	case 5:
		m_glview->SetRotations(0.0, 180.0, 0.0, false);
		break;
	}
	if (sel < 6)
		SetRotLock(true);
	else
		SetRotLock(false);
	event.Skip();
}

void RenderViewPanel::OnRotReset(wxCommandEvent& event)
{
	int id = event.GetId();

	switch (id)
	{
	case ID_ZeroRotBtn:
		SetZeroRotations();
		break;
	case ID_RotResetBtn:
		SetRotations(0.0, 0.0, 0.0, true);
		break;
	}
	event.Skip();
}

//reset counter
void RenderViewPanel::ResetID()
{
	m_max_id = 1;
}

//get rendering context
wxGLContext* RenderViewPanel::GetContext()
{
	if (m_glview)
		return m_glview->m_glRC/*GetContext()*/;
	else
		return 0;
}

void RenderViewPanel::RefreshGL(bool interactive, bool start_loop)
{
	if (m_glview)
	{
		m_glview->m_force_clear = true;
		m_glview->m_interactive = interactive;
		m_glview->RefreshGL(39, false, start_loop);
	}
}

//bar top
//ch1
void RenderViewPanel::OnCh1Check(wxCommandEvent &event)
{
	wxCheckBox* ch1 = (wxCheckBox*)event.GetEventObject();
	glbin_settings.m_save_compress = ch1->GetValue();
}

//save alpha
void RenderViewPanel::OnChAlphaCheck(wxCommandEvent &event)
{
	wxCheckBox* ch_alpha = (wxCheckBox*)event.GetEventObject();
	glbin_settings.m_save_alpha = ch_alpha->GetValue();
}

//save float
void RenderViewPanel::OnChFloatCheck(wxCommandEvent &event)
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
	RenderCanvas::SetEnlarge(enlarge);
	if (ch_enlarge)
		ch_enlarge->SetValue(enlarge);
	double enlarge_scale = (double)lval / 72.0;
	if (sl_enlarge)
	{
		sl_enlarge->Enable(enlarge);
		sl_enlarge->SetValue(std::round(enlarge_scale * 10));
	}
	if (tx_enlarge)
	{
		tx_enlarge->Enable(enlarge);
		tx_enlarge->SetValue(wxString::Format("%.1f", enlarge_scale));
	}
}

//embde project
void RenderViewPanel::OnChEmbedCheck(wxCommandEvent &event)
{
	wxCheckBox* ch_embed = (wxCheckBox*)event.GetEventObject();
	glbin_settings.m_vrp_embed = ch_embed->GetValue();
}

//enlarge output image
void RenderViewPanel::OnChEnlargeCheck(wxCommandEvent &event)
{
	wxCheckBox* ch_enlarge = (wxCheckBox*)event.GetEventObject();
	if (ch_enlarge)
	{
		bool enlarge = ch_enlarge->GetValue();
		RenderCanvas::SetEnlarge(enlarge);
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

void RenderViewPanel::OnSlEnlargeScroll(wxScrollEvent &event)
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

void RenderViewPanel::OnTxEnlargeText(wxCommandEvent &event)
{
	wxString str = event.GetString();
	double dval;
	str.ToDouble(&dval);
	RenderCanvas::SetEnlargeScale(dval);
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
#endif
	wxBoxSizer *group1 = new wxStaticBoxSizer(
		new wxStaticBox(panel, wxID_ANY, "Additional Options"), wxVERTICAL);

	//compressed
	wxCheckBox* ch1 = new wxCheckBox(panel, ID_LZW_COMP,
		"Lempel-Ziv-Welch Compression");
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
		"", wxDefaultPosition, parent->FromDIP(wxSize(60, 23)), 0, vald_int);
	tx_dpi->Connect(tx_dpi->GetId(), wxEVT_COMMAND_TEXT_UPDATED,
		wxCommandEventHandler(RenderViewPanel::OnDpiText), NULL, panel);
	float dpi = glbin_settings.m_dpi;
	tx_dpi->SetValue(wxString::Format("%.0f", dpi));
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
	sl_enlarge->SetValue(std::round(enlarge_scale * 10));
	wxFloatingPointValidator<double> vald_fp(1);
	wxTextCtrl* tx_enlarge = new wxTextCtrl(panel, ID_ENLARGE_TEXT,
		"1.0", wxDefaultPosition, parent->FromDIP(wxSize(60, 23)), 0, vald_fp);
	tx_enlarge->Connect(tx_enlarge->GetId(), wxEVT_COMMAND_TEXT_UPDATED,
		wxCommandEventHandler(RenderViewPanel::OnTxEnlargeText), NULL, panel);
	tx_enlarge->Enable(enlarge);
	tx_enlarge->SetValue(wxString::Format("%.1f", enlarge_scale));
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

void RenderViewPanel::InitOpenVR()
{
#ifdef _WIN32
	if (m_glview) m_glview->InitOpenVR();
#endif
}

void RenderViewPanel::SaveDefault(unsigned int mask)
{
	wxString str;
	bool bVal;
	wxColor cVal;
	double x, y, z;

	//render modes
	if (mask & 0x1)
		glbin_view_def.m_vol_method = m_glview->m_vol_method;
	//background color
	if (mask & 0x2)
		glbin_view_def.m_bg_color = m_glview->m_bg_color;
	//camera center
	if (mask & 0x4)
		glbin_view_def.m_draw_camctr = m_glview->m_draw_camctr;
	//camctr size
	if (mask & 0x8)
		glbin_view_def.m_camctr_size = m_glview->m_camctr_size;
	//fps
	if (mask & 0x10)
		glbin_view_def.m_draw_info = m_glview->m_draw_info;
	//selection
	if (mask & 0x20)
		glbin_view_def.m_draw_legend = m_glview->m_draw_legend;
	//mouse focus
	if (mask & 0x40)
		glbin_view_def.m_mouse_focus = m_glview->m_mouse_focus;
	//ortho/persp
	if (mask & 0x80)
	{
		glbin_view_def.m_persp = m_glview->m_persp;
		glbin_view_def.m_aov = m_glview->m_aov;
		glbin_view_def.m_free = m_glview->m_free;
	}
	//rotations
	if (mask & 0x100)
	{
		glbin_view_def.m_rot = fluo::Vector(
			m_glview->m_rotx,
			m_glview->m_roty,
			m_glview->m_rotz
		);
		glbin_view_def.m_rot_lock = m_glview->m_rot_lock;
		glbin_view_def.m_rot_slider = m_rot_slider;
	}
	//depth atten
	if (mask & 0x200)
	{
		glbin_view_def.m_use_fog = m_glview->m_use_fog;
		glbin_view_def.m_fog_intensity = m_glview->m_fog_intensity;
	}
	//scale factor
	if (mask & 0x400)
	{
		glbin_view_def.m_pin_rot_center = m_glview->m_pin_rot_center;
		glbin_view_def.m_scale_factor = m_glview->m_scale_factor;
		glbin_view_def.m_scale_mode = m_glview->m_scale_mode;
	}
	//camera center
	if (mask & 0x800)
		glbin_view_def.m_center = fluo::Point(
			m_glview->m_ctrx,
			m_glview->m_ctry,
			m_glview->m_ctrz
		);
	//colormap
	if (mask & 0x1000)
		glbin_view_def.m_draw_colormap = m_glview->m_draw_colormap;
}

void RenderViewPanel::LoadSettings()
{
	glbin_view_def.Apply(m_glview);
	m_rot_slider = glbin_view_def.m_rot_slider;
	m_glview->m_test_speed = glbin_settings.m_test_speed;
	m_glview->m_test_wiref = glbin_settings.m_test_wiref;
	m_glview->m_draw_bounds = glbin_settings.m_test_wiref;
	m_glview->m_draw_grid = glbin_settings.m_test_wiref;
	m_glview->SetPeelingLayers(glbin_settings.m_peeling_layers);
	m_glview->SetBlendSlices(glbin_settings.m_micro_blend);
	m_glview->SetAdaptive(glbin_settings.m_mouse_int);
	m_glview->SetGradBg(glbin_settings.m_grad_bg);
	m_glview->SetPointVolumeMode(glbin_settings.m_point_volume_mode);
	m_glview->SetRulerUseTransf(glbin_settings.m_ruler_use_transf);
	m_glview->SetStereo(glbin_settings.m_stereo);
	m_glview->SetSBS(glbin_settings.m_sbs);
	m_glview->SetEyeDist(glbin_settings.m_eye_dist);

	FluoRefresh(true, 2, {}, { m_frame->GetView(m_glview) });
}

