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
#include <VRenderView.h>
#include <Global/Global.h>
#include <VRenderFrame.h>
#include <ClippingView.h>
#include <tiffio.h>
#include <wxSingleSlider.h>
#include <wx/utils.h>
#include <wx/valnum.h>
#include <algorithm>
#include <limits>
#include "png_resource.h"
#include "img/icons.h"
#include <wx/stdpaths.h>
#include <wx/display.h>

int VRenderView::m_max_id = 1;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(VRenderView, wxPanel)
	//bar top
	EVT_TOOL(ID_VolumeSeqRd, VRenderView::OnVolumeMethodCheck)
	EVT_TOOL(ID_VolumeMultiRd, VRenderView::OnVolumeMethodCheck)
	EVT_TOOL(ID_VolumeCompRd, VRenderView::OnVolumeMethodCheck)
	EVT_BUTTON(ID_CaptureBtn, VRenderView::OnCapture)
	EVT_COLOURPICKER_CHANGED(ID_BgColorPicker, VRenderView::OnBgColorChange)
	EVT_BUTTON(ID_BgInvBtn, VRenderView::OnBgInvBtn)
	EVT_TOOL(ID_CamCtrChk, VRenderView::OnCamCtrCheck)
	EVT_TOOL(ID_FpsChk, VRenderView::OnFpsCheck)
	EVT_TOOL(ID_LegendChk, VRenderView::OnLegendCheck)
	EVT_TOOL(ID_ColormapChk, VRenderView::OnColormapCheck)
	//scale bar
	EVT_TOOL(ID_ScaleBar, VRenderView::OnScaleBar)
	EVT_TEXT(ID_ScaleText, VRenderView::OnScaleTextEditing)
	EVT_COMBOBOX(ID_ScaleCmb, VRenderView::OnScaleTextEditing)
	//other tools
	EVT_COMMAND_SCROLL(ID_AovSldr, VRenderView::OnAovChange)
	EVT_TEXT(ID_AovText, VRenderView::OnAovText)
	EVT_TOOL(ID_FreeChk, VRenderView::OnFreeChk)
	EVT_TOOL(ID_FullScreenBtn, VRenderView::OnFullScreen)
	//bar left
	EVT_TOOL(ID_DepthAttenChk, VRenderView::OnDepthAttenCheck)
	EVT_COMMAND_SCROLL(ID_DepthAttenFactorSldr, VRenderView::OnDepthAttenFactorChange)
	EVT_TEXT(ID_DepthAttenFactorText, VRenderView::OnDepthAttenFactorEdit)
	EVT_TOOL(ID_DepthAttenResetBtn, VRenderView::OnDepthAttenReset)
	//bar right
	EVT_TOOL(ID_PinBtn, VRenderView::OnPin)
	EVT_TOOL(ID_CenterBtn, VRenderView::OnCenter)
	EVT_TOOL(ID_Scale121Btn, VRenderView::OnScale121)
	EVT_COMMAND_SCROLL(ID_ScaleFactorSldr, VRenderView::OnScaleFactorChange)
	EVT_TEXT(ID_ScaleFactorText, VRenderView::OnScaleFactorEdit)
	EVT_TOOL(ID_ScaleModeBtn, VRenderView::OnScaleMode)
	EVT_TOOL(ID_ScaleResetBtn, VRenderView::OnScaleReset)
	EVT_SPIN_UP(ID_ScaleFactorSpin, VRenderView::OnScaleFactorSpinDown)
	EVT_SPIN_DOWN(ID_ScaleFactorSpin, VRenderView::OnScaleFactorSpinUp)
	//bar bottom
	EVT_TOOL(ID_ZeroRotBtn, VRenderView::OnZeroRot)
	EVT_TOOL(ID_RotResetBtn, VRenderView::OnRotReset)
	EVT_TEXT(ID_XRotText, VRenderView::OnValueEdit)
	EVT_TEXT(ID_YRotText, VRenderView::OnValueEdit)
	EVT_TEXT(ID_ZRotText, VRenderView::OnValueEdit)
	EVT_COMMAND_SCROLL(ID_XRotSldr, VRenderView::OnXRotScroll)
	EVT_COMMAND_SCROLL(ID_YRotSldr, VRenderView::OnYRotScroll)
	EVT_COMMAND_SCROLL(ID_ZRotSldr, VRenderView::OnZRotScroll)
	EVT_TOOL(ID_RotLockChk, VRenderView::OnRotLockCheck)
	EVT_TOOL(ID_RotSliderType, VRenderView::OnRotSliderType)
	EVT_COMBOBOX(ID_OrthoViewCmb, VRenderView::OnOrthoViewSelected)
	//timer
	EVT_TIMER(ID_RotateTimer, VRenderView::OnTimer)
	//save default
	EVT_TOOL(ID_DefaultBtn, VRenderView::OnSaveDefault)

	EVT_KEY_DOWN(VRenderView::OnKeyDown)
END_EVENT_TABLE()

VRenderView::VRenderView(VRenderFrame* frame,
	wxGLContext* sharedContext,
	const wxPoint& pos,
	const wxSize& size,
	long style) :
	wxPanel(frame, wxID_ANY, pos, size, style),
	m_default_saved(false),
	m_frame(frame),
	m_timer(this,ID_RotateTimer),
	m_draw_clip(false), 
	m_draw_scalebar(kOff),
	m_pin_scale_thresh(10.0),
	m_rot_slider(true),
	m_use_dft_settings(false),
	m_dft_x_rot(0.0),
	m_dft_y_rot(0.0),
	m_dft_z_rot(0.0),
	m_dft_depth_atten_factor(0.0),
	m_dft_scale_factor(1.0),
	m_dft_scale_factor_mode(0)
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
	wxString name = wxString::Format("Render View:%d", m_max_id++);
	SetName(name);
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
	m_glview = new VRenderGLView(frame, this, attriblist, sharedContext);
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
	if (m_glview)
	{
		m_glview->SetSBText(L"50 \u03BCm");
		m_glview->SetScaleBarLen(1.);
	}
	LoadSettings();
	m_x_rotating = m_y_rotating = m_z_rotating = false;
	m_skip_thumb = false;
}

VRenderView::~VRenderView()
{
	if (m_glview)
		delete m_glview;
	if (m_full_frame)
		delete m_full_frame;
}

void VRenderView::CreateBar()
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
	//bar top///////////////////////////////////////////////////
	//toolbar 1
	m_options_toolbar = new wxToolBar(this,wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	m_options_toolbar->SetDoubleBuffered(true);
	wxBoxSizer* sizer_h_1 = new wxBoxSizer(wxHORIZONTAL);
	wxSize tbs = m_options_toolbar->GetSize();
	wxSize toolsize = FromDIP(wxSize(tbs.y, tbs.y));
	m_options_toolbar->SetToolBitmapSize(toolsize);

	//the spacer
	//wxStaticText * stb;

	//add the options
	wxBitmap bitmap;
	wxImage image;
	bitmap = wxGetBitmap(layers, m_dpi_sf2);
	m_options_toolbar->AddRadioTool(
		ID_VolumeSeqRd, "Layered",
		bitmap, wxNullBitmap,
		"Render View as Layers",
		"Render View as Layers");
	bitmap = wxGetBitmap(depth, m_dpi_sf2);
	m_options_toolbar->AddRadioTool(
		ID_VolumeMultiRd, "Depth",
		bitmap, wxNullBitmap,
		"Render View by Depth",
		"Render View by Depth");
	bitmap = wxGetBitmap(composite, m_dpi_sf2);
	m_options_toolbar->AddRadioTool(
		ID_VolumeCompRd, "Composite",
		bitmap, wxNullBitmap,
		"Render View as a Composite of Colors",
		"Render View as a Composite of Colors");

	m_options_toolbar->ToggleTool(ID_VolumeSeqRd,false);
	m_options_toolbar->ToggleTool(ID_VolumeMultiRd,false);
	m_options_toolbar->ToggleTool(ID_VolumeCompRd,false);

	//stb = new wxStaticText(m_options_toolbar, wxID_ANY, "",
	//	wxDefaultPosition, FromDIP(wxSize(1,tbs.y-2)));
	//stb->SetBackgroundColour(wxColour(128,128,128));
	//m_options_toolbar->AddControl(stb);
	m_options_toolbar->AddSeparator();

	switch (m_glview->GetVolMethod())
	{
	case VOL_METHOD_SEQ:
		m_options_toolbar->ToggleTool(ID_VolumeSeqRd,true);
		break;
	case VOL_METHOD_MULTI:
		m_options_toolbar->ToggleTool(ID_VolumeMultiRd,true);
		break;
	case VOL_METHOD_COMP:
		m_options_toolbar->ToggleTool(ID_VolumeCompRd,true);
		break;
	}
	//camera
	wxButton* cam = new wxButton(m_options_toolbar, ID_CaptureBtn, "Capture",
		wxDefaultPosition, FromDIP(wxSize(100, 20)));
	cam->SetBitmap(wxGetBitmapFromMemory(camera));
	bitmap = wxGetBitmap(camera, m_dpi_sf2);
	m_options_toolbar->AddControl(cam);

	//stb = new wxStaticText(m_options_toolbar, wxID_ANY, "",
	//	wxDefaultPosition, FromDIP(wxSize(1, tbs.y-2)));
	//stb->SetBackgroundColour(wxColour(128,128,128));
	//m_options_toolbar->AddControl(stb);
	m_options_toolbar->AddSeparator();

	bitmap = wxGetBitmap(info, m_dpi_sf2);
	m_options_toolbar->AddCheckTool(
		ID_FpsChk, "Info",
		bitmap, wxNullBitmap,
		"Toggle View of FPS and Mouse Position",
		"Toggle View of FPS and Mouse Position");
	m_options_toolbar->ToggleTool(ID_FpsChk,false);

	bitmap = wxGetBitmap(axis, m_dpi_sf2);
	m_options_toolbar->AddCheckTool(
		ID_CamCtrChk, "Axis",
		bitmap, wxNullBitmap,
		"Toggle View of the Center Axis",
		"Toggle View of the Center Axis");
	m_options_toolbar->ToggleTool(ID_CamCtrChk,false);

	bitmap = wxGetBitmap(legend, m_dpi_sf2);
	m_options_toolbar->AddCheckTool(
		ID_LegendChk, "Legend",
		bitmap, wxNullBitmap,
		"Toggle View of the Legend",
		"Toggle View of the Legend");
	m_options_toolbar->ToggleTool(ID_LegendChk,false);

	bitmap = wxGetBitmap(colormap, m_dpi_sf2);
	m_options_toolbar->AddCheckTool(
		ID_ColormapChk, "Color Map",
		bitmap, wxNullBitmap,
		"Toggle View of the Colormap Sample",
		"Toggle View of the Colormap Sample");
	m_options_toolbar->ToggleTool(ID_ColormapChk, false);
//#ifndef _DARWIN
//	stb = new wxStaticText(m_options_toolbar, wxID_ANY, "",
//		wxDefaultPosition, FromDIP(wxSize(1, tbs.y-2)));
//	stb->SetBackgroundColour(wxColour(128,128,128));
//	m_options_toolbar->AddControl(stb);
//#endif
	m_options_toolbar->AddSeparator();

	//scale bar
	bitmap = wxGetBitmap(scalebar, m_dpi_sf2);
	m_options_toolbar->AddTool(
		ID_ScaleBar, "Scale Bar", bitmap,
		"Toggle Scalebar Options (Off, On, On with text)");

	sizer_h_1->Add(40, 40);
	sizer_h_1->Add(m_options_toolbar, 1, wxALIGN_CENTER);
	m_options_toolbar->Realize();

	m_scale_text = new wxTextCtrl(this, ID_ScaleText, "50",
		wxDefaultPosition, FromDIP(wxSize(35, 20)), 0, vald_int);
	m_scale_text->Disable();
	m_scale_cmb = new wxComboBox(this, ID_ScaleCmb, "",
		wxDefaultPosition, FromDIP(wxSize(50, 30)), 0, NULL, wxCB_READONLY);
	m_scale_cmb->Append("nm");
	m_scale_cmb->Append(L"\u03BCm");
	m_scale_cmb->Append("mm");
	m_scale_cmb->Select(1);
	m_scale_cmb->Disable();
	sizer_h_1->Add(m_scale_text, 0, wxALIGN_CENTER);
	sizer_h_1->Add(m_scale_cmb, 0, wxALIGN_CENTER);

	//m_options_toolbar->Realize();
//#ifndef _DARWIN
//	m_options_toolbar->AddStretchableSpace();
//#endif
	sizer_h_1->AddStretchSpacer(1);

	//background option
	m_bg_color_picker = new wxColourPickerCtrl(this,
		ID_BgColorPicker, wxColor(), wxDefaultPosition, FromDIP(wxSize(100, 20)));
	wxSize bs = m_bg_color_picker->GetSize();
	m_bg_inv_btn = new wxButton(this, ID_BgInvBtn, L"\u262f",
		wxDefaultPosition, FromDIP(wxSize(20, 20)));
	wxFont font(15 * m_dpi_sf, wxFONTFAMILY_DEFAULT, wxNORMAL, wxNORMAL);
	m_bg_inv_btn->SetFont(font);
	sizer_h_1->Add(m_bg_color_picker, 0, wxALIGN_CENTER);
	sizer_h_1->Add(m_bg_inv_btn, 0, wxALIGN_CENTER);
	//m_options_toolbar->AddControl(m_bg_color_picker);
	//m_options_toolbar->AddControl(m_bg_inv_btn);

//#ifndef _DARWIN
//	stb = new wxStaticText(m_options_toolbar, wxID_ANY, "",
//		wxDefaultPosition, FromDIP(wxSize(1, tbs.y-2)));
//	stb->SetBackgroundColour(wxColour(128, 128, 128));
//	m_options_toolbar->AddControl(stb);
//#endif

	//angle of view
	st2 = new wxStaticText(this, wxID_ANY, "Projection:");
	m_aov_sldr = new wxSingleSlider(this, ID_AovSldr, 45, 10, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_aov_sldr->SetValue(m_glview->GetPersp()? m_glview->GetAov():10);
	m_aov_sldr->Connect(wxID_ANY, wxEVT_IDLE,
		wxIdleEventHandler(VRenderView::OnAovSldrIdle),
		NULL, this);
	m_aov_text = new wxTextCtrl(this, ID_AovText, "",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), 0, vald_int);
	m_aov_text->ChangeValue(m_glview->GetPersp()?wxString::Format("%d",
		std::round(m_glview->GetAov())):wxString("Ortho"));
	sizer_h_1->Add(st2, 0, wxALIGN_CENTER);
	sizer_h_1->Add(m_aov_sldr, 1, wxEXPAND);
	sizer_h_1->Add(m_aov_text, 0, wxALIGN_CENTER);

	
	m_options_toolbar2 = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	m_options_toolbar2->SetToolBitmapSize(toolsize);
	m_options_toolbar2->SetDoubleBuffered(true);
	bitmap = wxGetBitmap(freefly, m_dpi_sf2);
	m_options_toolbar2->AddCheckTool(
		ID_FreeChk, "Free Fly",
		bitmap, wxNullBitmap,
		"Change the camera to a 'Free-Fly' Mode",
		"Change the camera to a 'Free-Fly' Mode");

	if (m_glview->GetFree())
		m_options_toolbar2->ToggleTool(ID_FreeChk,true);
	else
		m_options_toolbar2->ToggleTool(ID_FreeChk,false);

//#ifndef _DARWIN
//	stb = new wxStaticText(m_options_toolbar, wxID_ANY, "",
//		wxDefaultPosition, FromDIP(wxSize(1, tbs.y-2)));
//	stb->SetBackgroundColour(wxColour(128, 128, 128));
//	m_options_toolbar->AddControl(stb);
//#endif

	//save default
	bitmap = wxGetBitmap(save_settings, m_dpi_sf2);
	m_options_toolbar2->AddTool(
		ID_DefaultBtn, "Save", bitmap,
		"Set Default Render View Settings");

	//m_options_toolbar->SetRows(1);
	sizer_h_1->Add(m_options_toolbar2, 0, wxALIGN_CENTER);
	m_options_toolbar2->Realize();

	m_full_screen_btn = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_FLAT|wxTB_NODIVIDER);
	m_full_screen_btn->SetToolBitmapSize(toolsize);
	m_full_screen_btn->SetDoubleBuffered(true);
	bitmap = wxGetBitmap(full_view, m_dpi_sf2);
	m_full_screen_btn->AddTool(
		ID_FullScreenBtn, "Full Screen",
		bitmap, "Show full screen");
	m_full_screen_btn->Realize();

	//add the toolbars and other options in order
	sizer_h_1->Add(m_full_screen_btn, 0, wxALIGN_CENTER);

	//bar left///////////////////////////////////////////////////
	wxBoxSizer* sizer_v_3 = new wxBoxSizer(wxVERTICAL);
	m_left_toolbar = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	m_left_toolbar->SetToolBitmapSize(toolsize);
	m_left_toolbar->SetDoubleBuffered(true);
	bitmap = wxGetBitmap(no_depth_atten, m_dpi_sf2);
	m_left_toolbar->AddCheckTool(ID_DepthAttenChk, "Depth Interval",
		bitmap, wxNullBitmap,
		"Enable adjustment of the Depth Attenuation Interval",
		"Enable adjustment of the Depth Attenuation Interval");
	m_left_toolbar->ToggleTool(ID_DepthAttenChk, true);
	long ls = inverse_slider ? wxSL_VERTICAL : (wxSL_VERTICAL | wxSL_INVERSE);
	m_depth_atten_factor_sldr = new wxSingleSlider(this, ID_DepthAttenFactorSldr, 0, 0, 100,
		wxDefaultPosition, wxDefaultSize, ls);
	m_depth_atten_factor_sldr->Disable();
	m_depth_atten_reset_btn = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	m_depth_atten_reset_btn->SetToolBitmapSize(toolsize);
	m_depth_atten_reset_btn->SetDoubleBuffered(true);
	bitmap = wxGetBitmap(reset, m_dpi_sf2);
	m_depth_atten_reset_btn->AddTool(
		ID_DepthAttenResetBtn, "Reset",
		bitmap, "Reset Depth Attenuation Interval");
	m_depth_atten_reset_btn->Realize();
	m_depth_atten_factor_text = new wxTextCtrl(this, ID_DepthAttenFactorText, "0.0",
		wxDefaultPosition, FromDIP(wxSize(40, 20)), 0, vald_fp2);
	m_depth_atten_factor_text->Disable();

	m_left_toolbar->Realize();

	sizer_v_3->AddSpacer(50);
	sizer_v_3->Add(m_left_toolbar, 0, wxALIGN_CENTER);
	sizer_v_3->Add(m_depth_atten_factor_sldr, 1, wxEXPAND);
	sizer_v_3->Add(m_depth_atten_factor_text, 0, wxALIGN_CENTER);
	sizer_v_3->Add(m_depth_atten_reset_btn, 0, wxALIGN_CENTER);
	sizer_v_3->AddSpacer(50);

	//bar right///////////////////////////////////////////////////
	wxBoxSizer* sizer_v_4 = new wxBoxSizer(wxVERTICAL);
	m_pin_btn = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	m_pin_btn->SetToolBitmapSize(toolsize);
	m_pin_btn->SetDoubleBuffered(true);
	bitmap = wxGetBitmap(anchor_dark, m_dpi_sf2);
	m_pin_btn->AddCheckTool(ID_PinBtn, "Pin",
		bitmap, wxNullBitmap,
		"Anchor the rotation center on data",
		"Anchor the rotation center on data");
	m_pin_btn->ToggleTool(ID_PinBtn, false);
	m_pin_btn->Realize();
	m_center_btn = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	m_center_btn->SetToolBitmapSize(toolsize);
	m_center_btn->SetDoubleBuffered(true);
	bitmap = wxGetBitmap(center, m_dpi_sf2);
	m_center_btn->AddTool(ID_CenterBtn, "Center",
		bitmap, "Center the Data on the Render View");
	m_center_btn->Realize();
	m_scale_121_btn = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	m_scale_121_btn->SetToolBitmapSize(toolsize);
	m_scale_121_btn->SetDoubleBuffered(true);
	bitmap = wxGetBitmap(ratio, m_dpi_sf2);
	m_scale_121_btn->AddTool(ID_Scale121Btn, "1 to 1",
		bitmap, "Auto-size the data to a 1:1 ratio");
	m_scale_121_btn->Realize();
	ls = inverse_slider ? wxSL_VERTICAL : (wxSL_VERTICAL | wxSL_INVERSE);
	m_scale_factor_sldr = new wxSingleSlider(this, ID_ScaleFactorSldr, 100, 50, 999,
		wxDefaultPosition, wxDefaultSize, ls, wxDefaultValidator, "test");
	m_scale_reset_btn = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	m_scale_reset_btn->SetToolBitmapSize(toolsize);
	m_scale_reset_btn->SetDoubleBuffered(true);
	bitmap = wxGetBitmap(reset, m_dpi_sf2);
	m_scale_reset_btn->AddTool(ID_ScaleResetBtn, "Reset",
		bitmap, "Reset the Zoom");
	m_scale_reset_btn->Realize();
	m_scale_factor_text = new wxTextCtrl(this, ID_ScaleFactorText, "100",
		wxDefaultPosition, FromDIP(wxSize(40, 20)), 0, vald_int);
	m_scale_factor_spin = new wxSpinButton(this, ID_ScaleFactorSpin,
		wxDefaultPosition, FromDIP(wxSize(40, 20)));
	m_scale_factor_spin->SetRange(-0x8000, 0x7fff);
	m_scale_mode_btn = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	m_scale_mode_btn->SetToolBitmapSize(toolsize);
	m_scale_mode_btn->SetDoubleBuffered(true);
	bitmap = wxGetBitmap(zoom_view, m_dpi_sf2);
	m_scale_mode_btn->AddTool(
		ID_ScaleModeBtn, "Switch zoom ratio mode",
		bitmap, "View-based zoom ratio");
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
	st1 = new wxStaticText(this, 0, "X:");
	m_x_rot_sldr = new wxScrollBar(this, ID_XRotSldr);
	m_x_rot_sldr->SetScrollbar(180,60,420,15);
	m_x_rot_text = new wxTextCtrl(this, ID_XRotText, "0.0",
		wxDefaultPosition, FromDIP(wxSize(45,20)), 0, vald_fp1);
	st2 = new wxStaticText(this, 0, "Y:");
	m_y_rot_sldr = new wxScrollBar(this, ID_YRotSldr);
	m_y_rot_sldr->SetScrollbar(180,60,420,15);
	m_y_rot_text = new wxTextCtrl(this, ID_YRotText, "0.0",
		wxDefaultPosition, FromDIP(wxSize(45,20)), 0, vald_fp1);
	st3 = new wxStaticText(this, 0, "Z:");
	m_z_rot_sldr = new wxScrollBar(this, ID_ZRotSldr);
	m_z_rot_sldr->SetScrollbar(180,60,420,15);
	m_z_rot_text = new wxTextCtrl(this, ID_ZRotText, "0.0",
		wxDefaultPosition, FromDIP(wxSize(45,20)), 0, vald_fp1);

	//45 lock
	m_rot_lock_btn = new wxToolBar(this, wxID_ANY,
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
		"Choose slider style",
		"Choose slider style");
	m_rot_lock_btn->ToggleTool(ID_RotSliderType, m_rot_slider);
	m_rot_lock_btn->Realize();

	//ortho view selector
	m_ortho_view_cmb = new wxComboBox(this, ID_OrthoViewCmb, "",
		wxDefaultPosition, wxDefaultSize,/*FromDIP(wxSize(50, 30)),*/ 0, NULL, wxCB_READONLY);
	m_ortho_view_cmb->Append("+X");
	m_ortho_view_cmb->Append("-X");
	m_ortho_view_cmb->Append("+Y");
	m_ortho_view_cmb->Append("-Y");
	m_ortho_view_cmb->Append("+Z");
	m_ortho_view_cmb->Append("-Z");
	m_ortho_view_cmb->Append("NA");
	m_ortho_view_cmb->Select(6);
	m_lower_toolbar = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	m_lower_toolbar->SetToolBitmapSize(toolsize);
	m_lower_toolbar->SetDoubleBuffered(true);
	//m_lower_toolbar->AddControl(m_ortho_view_cmb);
	bitmap = wxGetBitmap(zrot, m_dpi_sf2);
	m_lower_toolbar->AddTool(ID_ZeroRotBtn, "Set Zeros",
		bitmap, "Set current angles as zeros");
	bitmap = wxGetBitmap(reset, m_dpi_sf2);
	m_lower_toolbar->AddTool(ID_RotResetBtn,"Reset",
		bitmap, "Reset Rotations");
	//m_lower_toolbar->SetMaxRowsCols(1, 1);
	m_lower_toolbar->Realize();

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
	sizer_h_2->Add(m_lower_toolbar, 0, wxALIGN_CENTER);
	sizer_h_2->AddSpacer(40);

	sizer_v->Add(sizer_h_1, 0, wxEXPAND);
	sizer_v->Add(sizer_m, 1, wxEXPAND);
	sizer_v->Add(sizer_h_2, 0, wxEXPAND);

	SetSizer(sizer_v);
	Layout();

}

void VRenderView::UpdateView(bool ui_update)
{
	double rotx, roty, rotz;
	wxString str_val = m_x_rot_text->GetValue();
	rotx = STOD(str_val.fn_str());
	str_val = m_y_rot_text->GetValue();
	roty = STOD(str_val.fn_str());
	str_val = m_z_rot_text->GetValue();
	rotz = STOD(str_val.fn_str());
	m_glview->SetRotations(rotx, roty, rotz, ui_update);
	RefreshGL(true);
}

void VRenderView::UpdateScaleFactor(bool update_text)
{
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
		VolumeData *vd = 0;
		if (m_glview->m_cur_vol)
			vd = m_glview->m_cur_vol;
		else if (m_glview->m_vd_pop_list.size())
			vd = m_glview->m_vd_pop_list[0];
		if (vd)
		{
			double spcx, spcy, spcz;
				vd->GetSpacings(spcx, spcy, spcz, vd->GetLevel());
			if (spcx > 0.0)
				scale /= m_glview->Get121ScaleFactor() * spcx;
		}
		}
		break;
	}
	int val = std::round(scale * 100);
	m_scale_factor_sldr->SetValue(val);
	wxString str = wxString::Format("%d", val);
	if (update_text)
		m_scale_factor_text->SetValue(str);
	else
		m_scale_factor_text->ChangeValue(str);
	//auto update pin rotation center
	if (m_glview->m_auto_update_rot_center)
	{
		if (scale > m_pin_scale_thresh)
		{
			if (!m_glview->m_pin_rot_center)
			{
				m_pin_btn->ToggleTool(ID_PinBtn, true);
				m_pin_btn->SetToolNormalBitmap(ID_PinBtn,
					wxGetBitmap(pin, m_dpi_sf2));
				m_glview->m_pin_rot_center = true;
				m_glview->m_rot_center_dirty = true;
			}
		}
		else
		{
			if (m_glview->m_pin_rot_center)
			{
				m_pin_btn->ToggleTool(ID_PinBtn, false);
				m_pin_btn->SetToolNormalBitmap(ID_PinBtn,
					wxGetBitmap(anchor_dark, m_dpi_sf2));
				m_glview->m_pin_rot_center = false;
			}
		}
	}
}

void VRenderView::SetScaleFactor(double s, bool update)
{
	switch (m_glview->m_scale_mode)
	{
	case 0:
		m_glview->m_scale_factor = s;
		break;
	case 1:
		m_glview->m_scale_factor = s * m_glview->Get121ScaleFactor();
		break;
	case 2:
		{
		VolumeData *vd = 0;
		if (m_glview->m_cur_vol)
			vd = m_glview->m_cur_vol;
		else if (m_glview->m_vd_pop_list.size())
			vd = m_glview->m_vd_pop_list[0];
		double spcx, spcy, spcz;
		if (vd)
			vd->GetSpacings(spcx, spcy, spcz, vd->GetLevel());
		if (spcx > 0.0)
			m_glview->m_scale_factor = s * m_glview->Get121ScaleFactor() * spcx;
		}
		break;
	}

	if (update)
		UpdateScaleFactor();
}

void VRenderView::SetScaleMode(int mode, bool update)
{
	switch (mode)
	{
	case 0:
		m_scale_mode_btn->SetToolNormalBitmap(ID_ScaleModeBtn,
			wxGetBitmap(zoom_view, m_dpi_sf2));
		m_scale_mode_btn->SetToolShortHelp(ID_ScaleModeBtn,
			"View-based zoom ratio");
		m_scale_mode_btn->SetToolLongHelp(ID_ScaleModeBtn,
			"View-based zoom ratio (View entire data set at 100%)");
		break;
	case 1:
		m_scale_mode_btn->SetToolNormalBitmap(ID_ScaleModeBtn,
			wxGetBitmap(zoom_pixel, m_dpi_sf2));
		m_scale_mode_btn->SetToolShortHelp(ID_ScaleModeBtn,
			"Pixel-based zoom ratio");
		m_scale_mode_btn->SetToolLongHelp(ID_ScaleModeBtn,
			"Pixel-based zoom ratio (View 1 data pixel to 1 screen pixel at 100%)");
		break;
	case 2:
		m_scale_mode_btn->SetToolNormalBitmap(ID_ScaleModeBtn,
			wxGetBitmap(zoom_data, m_dpi_sf2));
		m_scale_mode_btn->SetToolShortHelp(ID_ScaleModeBtn,
			"Data-based zoom ratio");
		m_scale_mode_btn->SetToolLongHelp(ID_ScaleModeBtn,
			"Data-based zoom ratio (View with consistent scale bar sizes)");
		break;
	}
	m_glview->m_scale_mode = mode;
	if (update)
		UpdateScaleFactor();
}

//rot center anchor thresh
void VRenderView::SetPinThreshold(double value)
{
	m_pin_scale_thresh = value;
}

//reset counter
void VRenderView::ResetID()
{
	m_max_id = 1;
}

//get rendering context
wxGLContext* VRenderView::GetContext()
{
	if (m_glview)
		return m_glview->m_glRC/*GetContext()*/;
	else
		return 0;
}

void VRenderView::RefreshGL(bool interactive, bool start_loop)
{
	if (m_glview)
	{
		m_glview->m_force_clear = true;
		m_glview->m_interactive = interactive;
		m_glview->RefreshGL(39, false, start_loop);
	}
}

//bar top
void VRenderView::OnVolumeMethodCheck(wxCommandEvent& event)
{
	//mode switch type
	//0 - didn't change
	//1 - to depth mode
	//2 - from depth mode
	//int mode_switch_type = 0;
	//int old_mode = GetVolMethod();

	int sender_id = event.GetId();
	switch (sender_id)
	{
	case ID_VolumeSeqRd:
		m_glview->SetVolMethod(VOL_METHOD_SEQ);
		break;
	case ID_VolumeMultiRd:
		m_glview->SetVolMethod(VOL_METHOD_MULTI);
		break;
	case ID_VolumeCompRd:
		m_glview->SetVolMethod(VOL_METHOD_COMP);
		break;
	}

	/*   int new_mode = GetVolMethod();

	if (new_mode == VOL_METHOD_MULTI &&
	(old_mode == VOL_METHOD_SEQ || old_mode == VOL_METHOD_COMP))
	mode_switch_type = 1;
	else if ((new_mode == VOL_METHOD_SEQ || new_mode == VOL_METHOD_COMP) &&
	old_mode == VOL_METHOD_MULTI)
	mode_switch_type = 2;

	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
	if (mode_switch_type == 1)
	{
	int cnt = 0;
	bool r = false;
	bool g = false;
	bool b = false;
	Color gamma = m_glview->GetGamma();
	Color brightness = m_glview->GetBrightness();
	Color hdr = m_glview->GetHdr();
	for (int i=0; i<GetLayerNum(); i++)
	{
	TreeLayer* layer = GetLayer(i);
	if (layer && layer->IsA() == 5)
	{
	DataGroup* group = (DataGroup*)layer;

	if (group->GetVolumeNum() == 0)
	continue;

	r = r||group->GetSyncR();
	g = g||group->GetSyncG();
	b = b||group->GetSyncB();

	if (cnt == 0)
	{
	gamma = group->GetGamma();
	brightness = group->GetBrightness();
	hdr = group->GetHdr();
	}
	}
	cnt++;
	}

	if (r && g && b)
	{
	gamma[1] = gamma[2] = gamma[0];
	brightness[1] = brightness[2] = brightness[0];
	hdr[1] = hdr[2] = hdr[0];
	}
	else
	{
	if (r && g)
	{
	gamma[1] = gamma[0];
	brightness[1] = brightness[0];
	hdr[1] = hdr[0];
	}
	else if (r & b)
	{
	gamma[2] = gamma[0];
	brightness[2] = brightness[0];
	hdr[2] = hdr[0];
	}
	else if (g & b)
	{
	gamma[2] = gamma[1];
	brightness[2] = brightness[1];
	hdr[2] = hdr[1];
	}
	}
	m_glview->SetGamma(gamma);
	m_glview->SetBrightness(brightness);
	m_glview->SetHdr(hdr);
	m_glview->SetSyncR(r);
	m_glview->SetSyncG(g);
	m_glview->SetSyncB(b);

	//sync properties of the selcted volume
	VolumeData* svd = vr_frame->GetCurSelVol();
	if (!svd)
	svd = GetAllVolumeData(0);
	if (svd)
	{
	for (int i=0; i<GetAllVolumeNum(); i++)
	{
	VolumeData* vd = GetAllVolumeData(i);
	if (vd)
	{
	vd->SetNR(svd->GetNR());
	vd->SetSampleRate(svd->GetSampleRate());
	vd->SetShadow(svd->GetShadow());
	double sp;
	svd->GetShadowParams(sp);
	vd->SetShadowParams(sp);
	}
	}
	}
	}
	else if (mode_switch_type == 2)
	{
	if (GetGroupNum() == 1)
	{
	for (int i=0; i<GetLayerNum(); i++)
	{
	TreeLayer* layer = GetLayer(i);
	if (layer && layer->IsA() == 5)
	{
	DataGroup* group = (DataGroup*)layer;
	flvr::Color col = m_glview->GetGamma();
	group->SetGammaAll(col);
	col = m_glview->GetBrightness();
	group->SetBrightnessAll(col);
	col = m_glview->GetHdr();
	group->SetHdrAll(col);
	group->SetSyncRAll(m_glview->GetSyncR());
	group->SetSyncGAll(m_glview->GetSyncG());
	group->SetSyncBAll(m_glview->GetSyncB());
	break;
	}
	}
	}
	}

	vr_frame->GetTree()->UpdateSelection();
	}*/

	RefreshGL();
}

//ch1
void VRenderView::OnCh1Check(wxCommandEvent &event)
{
	wxCheckBox* ch1 = (wxCheckBox*)event.GetEventObject();
	glbin_settings.m_save_compress = ch1->GetValue();
}

//save alpha
void VRenderView::OnChAlphaCheck(wxCommandEvent &event)
{
	wxCheckBox* ch_alpha = (wxCheckBox*)event.GetEventObject();
	glbin_settings.m_save_alpha = ch_alpha->GetValue();
}

//save float
void VRenderView::OnChFloatCheck(wxCommandEvent &event)
{
	wxCheckBox* ch_float = (wxCheckBox*)event.GetEventObject();
	glbin_settings.m_save_float = ch_float->GetValue();
}

//dpi
void VRenderView::OnDpiText(wxCommandEvent& event)
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
	VRenderGLView::SetEnlarge(enlarge);
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
void VRenderView::OnChEmbedCheck(wxCommandEvent &event)
{
	wxCheckBox* ch_embed = (wxCheckBox*)event.GetEventObject();
	glbin_settings.m_vrp_embed = ch_embed->GetValue();
}

//enlarge output image
void VRenderView::OnChEnlargeCheck(wxCommandEvent &event)
{
	wxCheckBox* ch_enlarge = (wxCheckBox*)event.GetEventObject();
	if (ch_enlarge)
	{
		bool enlarge = ch_enlarge->GetValue();
		VRenderGLView::SetEnlarge(enlarge);
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

void VRenderView::OnSlEnlargeScroll(wxScrollEvent &event)
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

void VRenderView::OnTxEnlargeText(wxCommandEvent &event)
{
	wxString str = event.GetString();
	double dval;
	str.ToDouble(&dval);
	VRenderGLView::SetEnlargeScale(dval);
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


wxWindow* VRenderView::CreateExtraCaptureControl(wxWindow* parent)
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
		wxCommandEventHandler(VRenderView::OnCh1Check), NULL, panel);
	if (ch1)
		ch1->SetValue(glbin_settings.m_save_compress);

	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	//save alpha
	wxCheckBox* ch_alpha = new wxCheckBox(panel, ID_SAVE_ALPHA,
		"Save alpha channel");
	ch_alpha->Connect(ch_alpha->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(VRenderView::OnChAlphaCheck), NULL, panel);
	if (ch_alpha)
		ch_alpha->SetValue(glbin_settings.m_save_alpha);
	//save float
	wxCheckBox* ch_float = new wxCheckBox(panel, ID_SAVE_FLOAT,
		"Save float channel");
	ch_float->Connect(ch_float->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(VRenderView::OnChFloatCheck), NULL, panel);
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
		wxCommandEventHandler(VRenderView::OnDpiText), NULL, panel);
	float dpi = glbin_settings.m_dpi;
	tx_dpi->SetValue(wxString::Format("%.0f", dpi));
	//enlarge
	wxBoxSizer* sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	wxCheckBox* ch_enlarge = new wxCheckBox(panel, ID_ENLARGE_CHK,
		"Enlarge output image");
	ch_enlarge->Connect(ch_enlarge->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(VRenderView::OnChEnlargeCheck), NULL, panel);
	bool enlarge = dpi > 72;
	double enlarge_scale = dpi / 72.0;
	ch_enlarge->SetValue(enlarge);
	wxSlider* sl_enlarge = new wxSlider(panel, ID_ENLARGE_SLDR,
		10, 10, 100);
	sl_enlarge->Connect(sl_enlarge->GetId(), wxEVT_COMMAND_SLIDER_UPDATED,
		wxScrollEventHandler(VRenderView::OnSlEnlargeScroll), NULL, panel);
	sl_enlarge->Enable(enlarge);
	sl_enlarge->SetValue(std::round(enlarge_scale * 10));
	wxFloatingPointValidator<double> vald_fp(1);
	wxTextCtrl* tx_enlarge = new wxTextCtrl(panel, ID_ENLARGE_TEXT,
		"1.0", wxDefaultPosition, parent->FromDIP(wxSize(60, 23)), 0, vald_fp);
	tx_enlarge->Connect(tx_enlarge->GetId(), wxEVT_COMMAND_TEXT_UPDATED,
		wxCommandEventHandler(VRenderView::OnTxEnlargeText), NULL, panel);
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
			wxCommandEventHandler(VRenderView::OnChEmbedCheck), NULL, panel);
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

void VRenderView::OnCapture(wxCommandEvent& event)
{
	//reset enlargement
	VRenderGLView::SetEnlarge(false);
	VRenderGLView::SetEnlargeScale(1.0);

	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;

	wxFileDialog file_dlg(m_frame, "Save captured image", "", "", "*.tif", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
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
			vr_frame->SaveProject(prop_file, inc);
		}
	}
}


//bar left
void VRenderView::OnDepthAttenCheck(wxCommandEvent& event)
{
	if (m_left_toolbar->GetToolState(ID_DepthAttenChk))
	{
		m_glview->SetFog(true);
		m_depth_atten_factor_sldr->Enable();
		m_depth_atten_factor_text->Enable();
		m_left_toolbar->SetToolNormalBitmap (ID_DepthAttenChk,
			wxGetBitmap(depth_atten, m_dpi_sf2));
	}
	else
	{
		m_glview->SetFog(false);
		m_depth_atten_factor_sldr->Disable();
		m_depth_atten_factor_text->Disable();
		m_left_toolbar->SetToolNormalBitmap (ID_DepthAttenChk,
			wxGetBitmap(no_depth_atten, m_dpi_sf2));
	}

	RefreshGL();
}

void VRenderView::OnDepthAttenFactorChange(wxScrollEvent& event)
{
	double atten_factor = m_depth_atten_factor_sldr->GetValue()/100.0;
	wxString str = wxString::Format("%.2f", atten_factor);
	if (str != m_depth_atten_factor_text->GetValue())
		m_depth_atten_factor_text->SetValue(str);
}

void VRenderView::OnDepthAttenFactorEdit(wxCommandEvent& event)
{
	wxString str = m_depth_atten_factor_text->GetValue();
	double val;
	str.ToDouble(&val);
	m_glview->SetFogIntensity(val);
	m_depth_atten_factor_sldr->SetValue(std::round(val*100.0));
	RefreshGL(true);
}

void VRenderView::OnDepthAttenReset(wxCommandEvent &event)
{
	if (m_use_dft_settings)
	{
		wxString str = wxString::Format("%.2f", m_dft_depth_atten_factor);
		m_depth_atten_factor_text->SetValue(str);
	}
	else
	{
		wxString str = wxString::Format("%.2f", 0.0);
		m_depth_atten_factor_text->SetValue(str);
	}
}

//bar right
void VRenderView::OnPin(wxCommandEvent &event)
{
	bool pin = m_pin_btn->GetToolState(ID_PinBtn);
	m_glview->SetPinRotCenter(pin);
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
		VolumeData *vd = 0;
		if (m_glview->m_cur_vol)
			vd = m_glview->m_cur_vol;
		else if (m_glview->m_vd_pop_list.size())
			vd = m_glview->m_vd_pop_list[0];
		double spcx, spcy, spcz;
		if (vd)
			vd->GetSpacings(spcx, spcy, spcz, vd->GetLevel());
		if (spcx > 0.0)
			scale /= m_glview->Get121ScaleFactor() * spcx;
		}
		break;
	}
	if (pin)
	{
		m_pin_btn->SetToolNormalBitmap(ID_PinBtn,
			wxGetBitmap(pin, m_dpi_sf2));
		if (scale > m_pin_scale_thresh)
			m_glview->m_auto_update_rot_center = true;
		else
			m_glview->m_auto_update_rot_center = false;
	}
	else
	{
		m_pin_btn->SetToolNormalBitmap(ID_PinBtn,
			wxGetBitmap(anchor_dark, m_dpi_sf2));
		if (scale > m_pin_scale_thresh)
			m_glview->m_auto_update_rot_center = false;
		else
			m_glview->m_auto_update_rot_center = true;
	}
}

void VRenderView::OnCenter(wxCommandEvent &event)
{
	if (m_glview)
	{
		m_glview->SetCenter();
		RefreshGL();
	}
}

void VRenderView::OnScale121(wxCommandEvent &event)
{
	if (m_glview)
	{
		m_glview->SetScale121();
		RefreshGL();
		if (m_glview->m_mouse_focus)
			m_glview->SetFocus();
	}
}

void VRenderView::OnScaleFactorChange(wxScrollEvent& event)
{
	int scale_factor = m_scale_factor_sldr->GetValue();
	//m_glview->m_scale_factor = scale_factor/100.0;
	wxString str = wxString::Format("%d", scale_factor);
	if (str != m_scale_factor_text->GetValue())
		m_scale_factor_text->SetValue(str);
}

void VRenderView::OnScaleFactorEdit(wxCommandEvent& event)
{
	wxString str = m_scale_factor_text->GetValue();
	long val;
	str.ToLong(&val);
	if (val>0)
	{
		m_scale_factor_sldr->SetValue(val);
		SetScaleFactor(val/100.0, false);
		//m_glview->SetSortBricks();
		RefreshGL(true);
	}
}

void VRenderView::OnScaleMode(wxCommandEvent& event)
{
	int mode = m_glview->m_scale_mode;
	mode += 1;
	mode = mode > 2 ? 0 : mode;
	switch (mode)
	{
	case 0:
		m_scale_mode_btn->SetToolNormalBitmap(ID_ScaleModeBtn,
			wxGetBitmap(zoom_view, m_dpi_sf2));
		m_scale_mode_btn->SetToolShortHelp(ID_ScaleModeBtn,
			"View-based zoom ratio");
		m_scale_mode_btn->SetToolLongHelp(ID_ScaleModeBtn,
			"View-based zoom ratio (View entire data set at 100%)");
		break;
	case 1:
		m_scale_mode_btn->SetToolNormalBitmap(ID_ScaleModeBtn,
			wxGetBitmap(zoom_pixel, m_dpi_sf2));
		m_scale_mode_btn->SetToolShortHelp(ID_ScaleModeBtn,
			"Pixel-based zoom ratio");
		m_scale_mode_btn->SetToolLongHelp(ID_ScaleModeBtn,
			"Pixel-based zoom ratio (View 1 data pixel to 1 screen pixel at 100%)");
		break;
	case 2:
		m_scale_mode_btn->SetToolNormalBitmap(ID_ScaleModeBtn,
			wxGetBitmap(zoom_data, m_dpi_sf2));
		m_scale_mode_btn->SetToolShortHelp(ID_ScaleModeBtn,
			"Data-based zoom ratio");
		m_scale_mode_btn->SetToolLongHelp(ID_ScaleModeBtn,
			"Data-based zoom ratio (View with consistent scale bar sizes)");
		break;
	}
	m_glview->m_scale_mode = mode;
	UpdateScaleFactor(false);
}

void VRenderView::OnScaleFactorSpinUp(wxSpinEvent& event)
{
	wxString str_val = m_scale_factor_text->GetValue();
	long val;
	str_val.ToLong(&val);
	if (glbin_settings.m_inverse_slider)
		val++;
	else
		val--;
	str_val = wxString::Format("%d", val);
	m_scale_factor_text->SetValue(str_val);
}

void VRenderView::OnScaleFactorSpinDown(wxSpinEvent& event)
{
	wxString str_val = m_scale_factor_text->GetValue();
	long val;
	str_val.ToLong(&val);
	if (glbin_settings.m_inverse_slider)
		val--;
	else
		val++;
	str_val = wxString::Format("%d", val);
	m_scale_factor_text->SetValue(str_val);
}

void VRenderView::OnScaleReset(wxCommandEvent &event)
{
	if (m_use_dft_settings)
		m_glview->m_scale_factor = m_dft_scale_factor;
	else
		m_glview->m_scale_factor = 1.0;
	UpdateScaleFactor();
	if (m_glview && m_glview->m_mouse_focus)
		m_glview->SetFocus();
}

//bar bottom
void VRenderView::OnValueEdit(wxCommandEvent& event)
{
	UpdateView(false);
	if (!m_rot_slider)
	{
		double rotx, roty, rotz;
		m_glview->GetRotations(rotx, roty, rotz);
		m_x_rot_sldr->SetThumbPosition(std::round(rotx));
		m_y_rot_sldr->SetThumbPosition(std::round(roty));
		m_z_rot_sldr->SetThumbPosition(std::round(rotz));
	}
}

/*void VRenderView::OnRotLink(bool b)
{
	m_glview->m_linked_rot = true;
	double rotx, roty, rotz;
	m_glview->GetRotations(rotx, roty, rotz);
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
		for (int i=0; i<vr_frame->GetViewNum(); i++)
		{
			VRenderView* view = vr_frame->GetView(i);
			if (view)
			{
				view->m_glview->m_linked_rot = b;
				view->m_glview->SetRotations(rotx, roty, rotz);
				view->m_glview->RefreshGL();
			}
		}
	}
}*/

void VRenderView::OnZeroRot(wxCommandEvent& event)
{
	if (m_glview)
	{
		double rotx, roty, rotz;
		m_glview->GetRotations(rotx, roty, rotz);
		if (rotx == 0.0 &&
			roty == 0.0 &&
			rotz == 0.0)
		{
			//reset
			m_glview->ResetZeroRotations(rotx, roty, rotz);
			m_glview->SetRotations(rotx, roty, rotz, true);
		}
		else
		{
			m_glview->SetZeroRotations();
			OnRotReset(event);
		}
	}
}

void VRenderView::OnRotReset(wxCommandEvent &event)
{
	//if (m_rot_slider)
	//{
	//	m_x_rot_sldr->SetThumbPosition(180);
	//	m_y_rot_sldr->SetThumbPosition(180);
	//	m_z_rot_sldr->SetThumbPosition(180);
	//}
	m_x_rot_text->ChangeValue("0.0");
	m_y_rot_text->ChangeValue("0.0");
	m_z_rot_text->ChangeValue("0.0");
	m_glview->SetRotations(0.0, 0.0, 0.0);
	RefreshGL(true);
	if (m_glview->m_mouse_focus)
		m_glview->SetFocus();
}

//timer used for rotation scrollbars
void VRenderView::OnTimer(wxTimerEvent& event) {
	wxString str;
	bool lock = m_rot_lock_btn->GetToolState(ID_RotLockChk);
	double rotx, roty, rotz;
	m_glview->GetRotations(rotx, roty, rotz);
	//the X bar positions
	int pos = m_x_rot_sldr->GetThumbPosition();
	int mid = 180;
	double dist = pos - mid;
	rotx = lock?(((int)((rotx + dist)/45))*45):
		(rotx + dist * dist * dist / 324000.);
	if (rotx < 0.) rotx +=360.;
	if (rotx > 360.) rotx -=360.;
	str = wxString::Format("%.1f", rotx);
	if (std::abs(dist) > 1) {
		if (!m_x_rotating)
			m_x_rot_sldr->SetThumbPosition((mid + pos) / 2. + 0.5);
		if (std::abs(dist) > 10)
			m_x_rot_text->SetValue(str);
	}
	//the Y bar positions
	pos = m_y_rot_sldr->GetThumbPosition();
	dist = pos - mid;
	roty = lock?(((int)((roty + dist)/45))*45):
		(roty + dist * dist * dist / 324000.);
	if (roty < 0.) roty +=360.;
	if (roty > 360.) roty -=360.;
	str = wxString::Format("%.1f", roty);
	if (std::abs(dist) > 1) {
		if (!m_y_rotating)
			m_y_rot_sldr->SetThumbPosition((mid + pos) / 2. + 0.5);
		if (std::abs(dist) > 10)
			m_y_rot_text->SetValue(str);
	}
	//the Z bar positions
	pos = m_z_rot_sldr->GetThumbPosition();
	dist = pos - mid;
	rotz = lock?(((int)((rotz + dist)/45))*45):
		(rotz + dist * dist * dist / 324000.);
	if (rotz < 0.) rotz +=360.;
	if (rotz > 360.) rotz -=360.;
	str = wxString::Format("%.1f", rotz);
	if (std::abs(dist) > 1) {
		if (!m_z_rotating)
			m_z_rot_sldr->SetThumbPosition((mid + pos) / 2. + 0.5);
		if (std::abs(dist) > 10)
			m_z_rot_text->SetValue(str);
	}
}

void VRenderView::OnXRotScroll(wxScrollEvent& event)
{
	if (m_skip_thumb)
	{
		m_skip_thumb = false;
		m_x_rot_sldr->SetThumbPosition(180);
		event.Skip();
		return;
	}

	bool lock = m_rot_lock_btn->GetToolState(ID_RotLockChk);
	wxString str;
	if (m_rot_slider)
	{
		double rotx, roty, rotz, deg;
		m_glview->GetRotations(rotx, roty, rotz);
		if (event.GetEventType() == wxEVT_SCROLL_THUMBTRACK )
			m_x_rotating = true;
		else if (event.GetEventType() == wxEVT_SCROLL_THUMBRELEASE  )
			m_x_rotating = false;
		else if (event.GetEventType() == wxEVT_SCROLL_LINEDOWN)
		{
			deg = lock ? (rotx + 45) : (rotx + 1);
			deg = deg > 360.0 ? deg - 360.0 : deg;
			str = wxString::Format("%.1f", deg);
			if (str != m_x_rot_text->GetValue())
			{
				m_x_rot_text->SetValue(str);
				m_skip_thumb = true;
			}
		} else if (event.GetEventType() == wxEVT_SCROLL_LINEUP)
		{
			deg = lock ? (rotx - 45) : (rotx - 1);
			deg = deg < 0.0 ? deg + 360.0 : deg;
			str = wxString::Format("%.1f", deg);
			if (str != m_x_rot_text->GetValue())
			{
				m_x_rot_text->SetValue(str);
				m_skip_thumb = true;
			}
		}
	}
	else
	{
		int pos = m_x_rot_sldr->GetThumbPosition();
		if (lock)
			pos = std::round(double(pos) / 45.0) * 45;
		str = wxString::Format("%.1f", double(pos));
		if (str != m_x_rot_text->GetValue())
			m_x_rot_text->SetValue(str);
	}
}

void VRenderView::OnYRotScroll(wxScrollEvent& event)
{
	if (m_skip_thumb)
	{
		m_skip_thumb = false;
		m_y_rot_sldr->SetThumbPosition(180);
		event.Skip();
		return;
	}

	bool lock = m_rot_lock_btn->GetToolState(ID_RotLockChk);
	wxString str;
	if (m_rot_slider)
	{
		double rotx, roty, rotz, deg;
		m_glview->GetRotations(rotx, roty, rotz);
		if (event.GetEventType() == wxEVT_SCROLL_THUMBTRACK)
			m_y_rotating = true;
		else if (event.GetEventType() == wxEVT_SCROLL_THUMBRELEASE)
			m_y_rotating = false;
		else if (event.GetEventType() == wxEVT_SCROLL_LINEDOWN)
		{
			deg = lock ? (roty + 45) : (roty + 1);
			deg = deg > 360.0 ? deg - 360.0 : deg;
			str = wxString::Format("%.1f", deg);
			if (str != m_y_rot_text->GetValue())
			{
				m_y_rot_text->SetValue(str);
				m_skip_thumb = true;
			}
		}
		else if (event.GetEventType() == wxEVT_SCROLL_LINEUP)
		{
			deg = lock ? (roty - 45) : (roty - 1);
			deg = deg < 0.0 ? deg + 360.0 : deg;
			str = wxString::Format("%.1f", deg);
			if (str != m_y_rot_text->GetValue())
			{
				m_y_rot_text->SetValue(str);
				m_skip_thumb = true;
			}
		}
	}
	else
	{
		int pos = m_y_rot_sldr->GetThumbPosition();
		if (lock)
			pos = std::round(double(pos) / 45.0) * 45;
		str = wxString::Format("%.1f", double(pos));
		if (str != m_y_rot_text->GetValue())
			m_y_rot_text->SetValue(str);
	}
}

void VRenderView::OnZRotScroll(wxScrollEvent& event)
{
	if (m_skip_thumb)
	{
		m_skip_thumb = false;
		m_z_rot_sldr->SetThumbPosition(180);
		event.Skip();
		return;
	}

	bool lock = m_rot_lock_btn->GetToolState(ID_RotLockChk);
	wxString str;
	if (m_rot_slider)
	{
		double rotx, roty, rotz, deg;
		m_glview->GetRotations(rotx, roty, rotz);
		if (event.GetEventType() == wxEVT_SCROLL_THUMBTRACK)
			m_z_rotating = true;
		else if (event.GetEventType() == wxEVT_SCROLL_THUMBRELEASE)
			m_z_rotating = false;
		else if (event.GetEventType() == wxEVT_SCROLL_LINEDOWN)
		{
			deg = lock ? (rotz + 45) : (rotz + 1);
			deg = deg > 360.0 ? deg - 360.0 : deg;
			str = wxString::Format("%.1f", deg);
			if (str != m_z_rot_text->GetValue())
			{
				m_z_rot_text->SetValue(str);
				m_skip_thumb = true;
			}
		}
		else if (event.GetEventType() == wxEVT_SCROLL_LINEUP)
		{
			deg = lock ? (rotz - 45) : (rotz - 1);
			deg = deg < 0.0 ? deg + 360.0 : deg;
			str = wxString::Format("%.1f", deg);
			if (str != m_z_rot_text->GetValue())
			{
				m_z_rot_text->SetValue(str);
				m_skip_thumb = true;
			}
		}
	}
	else
	{
		int pos = m_z_rot_sldr->GetThumbPosition();
		if (lock)
			pos = std::round(double(pos) / 45.0) * 45;
		str = wxString::Format("%.1f", double(pos));
		if (str != m_z_rot_text->GetValue())
			m_z_rot_text->SetValue(str);
	}
}

void VRenderView::OnRotLockCheck(wxCommandEvent& event)
{
	bool lock = m_rot_lock_btn->GetToolState(ID_RotLockChk);
	double rotx, roty, rotz;
	m_glview->GetRotations(rotx, roty, rotz);
	if (lock)
	{
		m_rot_lock_btn->SetToolNormalBitmap(ID_RotLockChk,
			wxGetBitmap(gear_45, m_dpi_sf2));
		rotx = (((int)(rotx/45))*45);
		roty = (((int)(roty/45))*45);
		rotz = (((int)(rotz/45))*45);
	}
	else
	{
		m_rot_lock_btn->SetToolNormalBitmap(ID_RotLockChk,
			wxGetBitmap(gear_dark, m_dpi_sf2));
	}
	m_glview->SetRotLock(lock);
	wxString str = wxString::Format("%.1f", rotx);
	m_x_rot_text->SetValue(str);
	str = wxString::Format("%.1f", roty);
	m_y_rot_text->SetValue(str);
	str = wxString::Format("%.1f", rotz);
	m_z_rot_text->SetValue(str);
}

void VRenderView::OnRotSliderType(wxCommandEvent& event)
{
	m_rot_slider = m_rot_lock_btn->GetToolState(ID_RotSliderType);
	if (m_rot_slider)
	{
		m_timer.Start(50);
		m_rot_lock_btn->SetToolNormalBitmap(ID_RotSliderType,
			wxGetBitmap(slider_type_rot, m_dpi_sf2));
		m_x_rot_sldr->SetThumbPosition(180);
		m_y_rot_sldr->SetThumbPosition(180);
		m_z_rot_sldr->SetThumbPosition(180);
	}
	else
	{
		m_timer.Stop();
		m_rot_lock_btn->SetToolNormalBitmap(ID_RotSliderType,
			wxGetBitmap(slider_type_pos, m_dpi_sf2));
		double rotx, roty, rotz;
		m_glview->GetRotations(rotx, roty, rotz);
		m_x_rot_sldr->SetThumbPosition(std::round(rotx));
		m_y_rot_sldr->SetThumbPosition(std::round(roty));
		m_z_rot_sldr->SetThumbPosition(std::round(rotz));
	}
}

void VRenderView::OnOrthoViewSelected(wxCommandEvent& event)
{
	int sel = 6;
	if (m_ortho_view_cmb)
		sel = m_ortho_view_cmb->GetSelection();
	switch (sel)
	{
	case 0://+X
		m_glview->SetRotations(0.0, 90.0, 0.0, true);
		break;
	case 1://-X
		m_glview->SetRotations(0.0, 270.0, 0.0, true);
		break;
	case 2://+Y
		m_glview->SetRotations(90.0, 0.0, 0.0, true);
		break;
	case 3://-Y
		m_glview->SetRotations(270.0, 0.0, 0.0, true);
		break;
	case 4://+Z
		m_glview->SetRotations(0.0, 0.0, 0.0, true);
		break;
	case 5:
		m_glview->SetRotations(0.0, 180.0, 0.0, true);
		break;
	}
	if (sel < 6)
	{
		m_rot_lock_btn->ToggleTool(ID_RotLockChk, true);
		m_rot_lock_btn->SetToolNormalBitmap(ID_RotLockChk,
			wxGetBitmap(gear_45, m_dpi_sf2));
		if (m_glview) m_glview->SetRotLock(true);
	}
	else
	{
		m_rot_lock_btn->ToggleTool(ID_RotLockChk, false);
		m_rot_lock_btn->SetToolNormalBitmap(ID_RotLockChk,
			wxGetBitmap(gear_dark, m_dpi_sf2));
		if (m_glview) m_glview->SetRotLock(false);
	}
	RefreshGL();
}

//top
void VRenderView::OnBgColorChange(wxColourPickerEvent& event)
{
	wxColor c = event.GetColour();
	fluo::Color color(c.Red()/255.0, c.Green()/255.0, c.Blue()/255.0);
	m_glview->SetBackgroundColor(color);
	RefreshGL();
}

void VRenderView::OnBgInvBtn(wxCommandEvent& event)
{
	fluo::Color c = m_glview->GetBackgroundColor();
	c = fluo::Color(1.0, 1.0, 1.0) - c;
	m_glview->SetBackgroundColor(c);
	RefreshGL();
}

void VRenderView::OnCamCtrCheck(wxCommandEvent& event)
{
	m_glview->m_draw_camctr = 
		m_options_toolbar->GetToolState(ID_CamCtrChk);
	RefreshGL();
}

void VRenderView::OnFpsCheck(wxCommandEvent& event)
{
	if (m_options_toolbar->GetToolState(ID_FpsChk))
		m_glview->m_draw_info |= 1;
	else
		m_glview->m_draw_info &= ~1;
	RefreshGL();
}

void VRenderView::OnLegendCheck(wxCommandEvent& event)
{
	m_glview->m_draw_legend = 
		m_options_toolbar->GetToolState(ID_LegendChk);
	RefreshGL();
}

void VRenderView::OnColormapCheck(wxCommandEvent& event)
{
	m_glview->m_draw_colormap =
		m_options_toolbar->GetToolState(ID_ColormapChk);
	RefreshGL();
}

void VRenderView::OnScaleTextEditing(wxCommandEvent& event)
{
	wxString str, num_text, unit_text;
	num_text = m_scale_text->GetValue();
	double len;
	num_text.ToDouble(&len);
	str = num_text + " ";
	switch (m_scale_cmb->GetSelection())
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
	str += unit_text;
	if (m_glview)
	{
		m_glview->SetSBText(str);
		m_glview->SetScaleBarLen(len);
		m_glview->m_sb_num = num_text;
		m_glview->m_sb_unit = m_scale_cmb->GetSelection();
	}
	RefreshGL();
}

void VRenderView::OnScaleUnitSelected(wxCommandEvent& event) {
}

void VRenderView::OnScaleBar(wxCommandEvent& event)
{
	if (!m_glview)
		return;

	switch (m_draw_scalebar)
	{
	case kOff:
		m_draw_scalebar = kOn;
		m_glview->m_disp_scale_bar = true;
		m_glview->m_disp_scale_bar_text = false;
		m_options_toolbar->SetToolNormalBitmap(ID_ScaleBar,
			wxGetBitmap(scale_text_off, m_dpi_sf2));
		m_scale_text->Enable();
		m_scale_cmb->Disable();
		break;
	case kOn:
		m_draw_scalebar = kText;
		m_glview->m_disp_scale_bar = true;
		m_glview->m_disp_scale_bar_text = true;
		m_options_toolbar->SetToolNormalBitmap(ID_ScaleBar,
			wxGetBitmap(scale_text, m_dpi_sf2));
		m_scale_text->Enable();
		m_scale_cmb->Enable();
		break;
	case kText:
		m_draw_scalebar = kOff;
		m_glview->m_disp_scale_bar = false;
		m_glview->m_disp_scale_bar_text = false;
		m_options_toolbar->SetToolNormalBitmap(ID_ScaleBar,
			wxGetBitmap(scalebar, m_dpi_sf2));
		m_scale_text->Disable();
		m_scale_cmb->Disable();
		break;
	default:
		break;
	}
	OnScaleTextEditing(event);
	RefreshGL();
}

void VRenderView::OnAovSldrIdle(wxIdleEvent& event)
{
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetClippingView())
	{
		if (vr_frame->GetClippingView()->GetHoldPlanes())
			return;
	}
	if (m_glview->m_capture)
		return;

	wxPoint pos = wxGetMousePosition();
	wxRect reg = m_aov_sldr->GetScreenRect();
	wxWindow *window = wxWindow::FindFocus();
	if (window && reg.Contains(pos))
	{
		if (!m_draw_clip)
		{
			m_glview->m_draw_clip = true;
			m_glview->m_clip_mask = -1;
			RefreshGL(true);
			m_draw_clip = true;
		}
	}
	else
	{
		if (m_draw_clip)
		{
			m_glview->m_draw_clip = false;
			RefreshGL(true);
			m_draw_clip = false;
		}
	}
	event.Skip();
}

void VRenderView::OnAovChange(wxScrollEvent& event)
{
	int val = m_aov_sldr->GetValue();
	wxString str = wxString::Format("%d", val);
	if (str != m_aov_text->GetValue())
		m_aov_text->SetValue(str);
}

void VRenderView::OnAovText(wxCommandEvent& event)
{
	wxString str = m_aov_text->GetValue();
	if (str == "Ortho")
	{
		m_glview->SetPersp(false);
		m_aov_sldr->SetValue(10);
		RefreshGL(true);
		return;
	}
	long val;
	if (!str.ToLong(&val))
		return;
	if (val ==0 || val == 10)
	{
		m_glview->SetPersp(false);
		m_aov_text->ChangeValue("Ortho");
		m_aov_sldr->SetValue(10);
	}
	else if (val < 10)
	{
		return;
	}
	else
	{
		if (val > 100)
		{
			val = 100;
			m_aov_text->ChangeValue("100");
			m_aov_sldr->SetValue(100);
		}
		m_glview->SetPersp(true);
		m_glview->SetAov(val);
		m_aov_sldr->SetValue(val);
	}
	RefreshGL(true);
}

void VRenderView::OnFreeChk(wxCommandEvent& event)
{
	if (m_options_toolbar2->GetToolState(ID_FreeChk))
		m_glview->SetFree(true);
	else
	{
		m_glview->SetFree(false);
		int val = m_aov_sldr->GetValue();
		if (val == 10)
			m_glview->SetPersp(false);
		else
			m_glview->SetPersp(true);
	}
	RefreshGL();
}

void VRenderView::SetFullScreen()
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
			m_full_frame->SetWindowStyle(wxBORDER_NONE|wxSTAY_ON_TOP);
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

void VRenderView::OnFullScreen(wxCommandEvent& event)
{
	SetFullScreen();
}

void VRenderView::SaveDefault(unsigned int mask)
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
		glbin_view_def.m_rot_lock = m_glview->GetRotLock();
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

	m_default_saved = true;
}

void VRenderView::OnSaveDefault(wxCommandEvent &event)
{
	SaveDefault();
}

void VRenderView::LoadSettings()
{
	glbin_view_def.Apply(m_glview);

	m_options_toolbar->ToggleTool(ID_CamCtrChk,glbin_view_def.m_draw_camctr);
	m_options_toolbar->ToggleTool(ID_FpsChk, glbin_view_def.m_draw_info & INFO_DISP);
	m_options_toolbar->ToggleTool(ID_LegendChk, glbin_view_def.m_draw_legend);
	m_options_toolbar->ToggleTool(ID_ColormapChk, glbin_view_def.m_draw_colormap);
	m_options_toolbar2->ToggleTool(ID_FreeChk, glbin_view_def.m_free);
	m_x_rot_text->ChangeValue(wxString::Format("%.0f", glbin_view_def.m_rot.x()));
	m_y_rot_text->ChangeValue(wxString::Format("%.0f", glbin_view_def.m_rot.y()));
	m_z_rot_text->ChangeValue(wxString::Format("%.0f", glbin_view_def.m_rot.z()));
	m_rot_lock_btn->ToggleTool(ID_RotLockChk,glbin_view_def.m_rot_lock);
	if (glbin_view_def.m_rot_lock)
		m_rot_lock_btn->SetToolNormalBitmap(ID_RotLockChk,
			wxGetBitmap(gear_45, m_dpi_sf2));
	else
		m_rot_lock_btn->SetToolNormalBitmap(ID_RotLockChk,
			wxGetBitmap(gear_dark, m_dpi_sf2));
	m_rot_lock_btn->ToggleTool(ID_RotSliderType, glbin_view_def.m_rot_slider);
	wxCommandEvent e;
	OnRotSliderType(e);
	UpdateView();  //for rotations
	//if (fconfig.Read("scale_factor_text", &str))
	//{
	//	m_scale_factor_text->ChangeValue(str);
	//	str.ToDouble(&dVal);
	//	if (dVal <= 1.0)
	//		dVal = 100.0;
	//	m_scale_factor_sldr->SetValue(dVal);
	//	m_glview->m_scale_factor = dVal/100.0;
	//	m_dft_scale_factor = dVal;
	//}
	m_pin_btn->ToggleTool(ID_PinBtn, glbin_view_def.m_pin_rot_center);
	if (glbin_view_def.m_pin_rot_center)
		m_pin_btn->SetToolNormalBitmap(ID_PinBtn,
			wxGetBitmap(pin, m_dpi_sf2));
	else
		m_pin_btn->SetToolNormalBitmap(ID_PinBtn,
			wxGetBitmap(anchor_dark, m_dpi_sf2));
	SetScaleMode(glbin_view_def.m_scale_mode, false);
	UpdateScaleFactor(false);
	m_glview->SetFog(glbin_view_def.m_use_fog);
	if (glbin_view_def.m_use_fog)
	{
		m_depth_atten_factor_sldr->Enable();
		m_depth_atten_factor_text->Enable();
	}
	else
	{
		m_depth_atten_factor_sldr->Disable();
		m_depth_atten_factor_text->Disable();
	}
	m_depth_atten_factor_text->ChangeValue(wxString::Format("%.0f", glbin_view_def.m_fog_intensity));
	m_depth_atten_factor_sldr->ChangeValue(std::round(glbin_view_def.m_fog_intensity*100));
	m_glview->SetFogIntensity(glbin_view_def.m_fog_intensity);

	m_use_dft_settings = true;
	RefreshGL();
}

void VRenderView::OnKeyDown(wxKeyEvent& event)
{
	event.Skip();
}

