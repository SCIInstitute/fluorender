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
#include "VRenderView.h"
#include "VRenderFrame.h"
#include <tiffio.h>
#include <wx/utils.h>
#include <wx/valnum.h>
#include <algorithm>
#include <limits>
#include "png_resource.h"
#include "img/icons.h"
#include <wx/stdpaths.h>

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

VRenderView::VRenderView(wxWindow* frame,
	wxWindow* parent,
	wxWindowID id,
	wxGLContext* sharedContext,
	const wxPoint& pos,
	const wxSize& size,
	long style) :
wxPanel(parent, id, pos, size, style),
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

	m_id = m_max_id;
	wxString name = wxString::Format("Render View:%d", m_max_id++);
	this->SetName(name);
	// this list takes care of both pixel and context attributes (no custom edits of wx is preferred)
	//render view/////////////////////////////////////////////////
	int red_bit = 8;
	int green_bit = 8;
	int blue_bit = 8;
	int alpha_bit = 8;
	int depth_bit = 24;
	int samples = 0;
	int gl_major_ver = 4;
	int gl_minor_ver = 4;
	int gl_profile_mask = 1;
	int api_type = 0;
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetSettingDlg())
	{
		api_type = vr_frame->GetSettingDlg()->GetApiType();
		red_bit = vr_frame->GetSettingDlg()->GetRedBit();
		green_bit = vr_frame->GetSettingDlg()->GetGreenBit();
		blue_bit = vr_frame->GetSettingDlg()->GetBlueBit();
		alpha_bit = vr_frame->GetSettingDlg()->GetAlphaBit();
		depth_bit = vr_frame->GetSettingDlg()->GetDepthBit();
		samples = vr_frame->GetSettingDlg()->GetSamples();
		gl_major_ver = vr_frame->GetSettingDlg()->GetGLMajorVer();
		gl_minor_ver = vr_frame->GetSettingDlg()->GetGLMinorVer();
		gl_profile_mask = vr_frame->GetSettingDlg()->GetGLProfileMask();
	}

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
	attriblist.DoubleBuffer();
	attriblist.EndList();
	m_glview = new VRenderGLView(frame, this, wxID_ANY, attriblist, sharedContext);
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
			if (vr_frame && vr_frame->GetSettingDlg())
			{
				//could be that color bits are incorrectly set
				vr_frame->GetSettingDlg()->SetRedBit(8);
				vr_frame->GetSettingDlg()->SetGreenBit(8);
				vr_frame->GetSettingDlg()->SetBlueBit(8);
				vr_frame->GetSettingDlg()->SetAlphaBit(8);
				vr_frame->GetSettingDlg()->SaveSettings();
			}
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
	if (vr_frame && vr_frame->GetSettingDlg())
	{
		vr_frame->GetSettingDlg()->SetGLMajorVer(gl_major_ver);
		vr_frame->GetSettingDlg()->SetGLMinorVer(gl_minor_ver);
	}

	CreateBar();
	if (m_glview) {
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

	//bar top///////////////////////////////////////////////////
	//toolbar 1
	m_options_toolbar = new wxToolBar(this,wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	m_options_toolbar->SetDoubleBuffered(true);
	wxBoxSizer* sizer_h_1 = new wxBoxSizer(wxHORIZONTAL);
	wxSize tbs = m_options_toolbar->GetSize();
#ifndef _DARWIN
	//the spacer
	wxStaticText * stb;
#endif
	//add the options
	wxBitmap bitmap;
	bitmap = wxGetBitmapFromMemory(layers);
#ifdef _DARWIN
	m_options_toolbar->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_options_toolbar->AddRadioTool(
		ID_VolumeSeqRd, "Layered",
		bitmap, wxNullBitmap,
		"Render View as Layers",
		"Render View as Layers");
	bitmap = wxGetBitmapFromMemory(depth);
	m_options_toolbar->AddRadioTool(
		ID_VolumeMultiRd, "Depth",
		bitmap, wxNullBitmap,
		"Render View by Depth",
		"Render View by Depth");
	bitmap = wxGetBitmapFromMemory(composite);
	m_options_toolbar->AddRadioTool(
		ID_VolumeCompRd, "Composite",
		bitmap, wxNullBitmap,
		"Render View as a Composite of Colors",
		"Render View as a Composite of Colors");

	m_options_toolbar->ToggleTool(ID_VolumeSeqRd,false);
	m_options_toolbar->ToggleTool(ID_VolumeMultiRd,false);
	m_options_toolbar->ToggleTool(ID_VolumeCompRd,false);
#ifndef _DARWIN
	stb = new wxStaticText(m_options_toolbar, wxID_ANY, "",
		wxDefaultPosition, wxSize(1,tbs.y-2));
	stb->SetBackgroundColour(wxColour(128,128,128));
	m_options_toolbar->AddControl(stb);
#endif
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
	wxButton * cam = new wxButton(m_options_toolbar, ID_CaptureBtn, "Capture");
	cam->SetBitmap(wxGetBitmapFromMemory(camera));
	m_options_toolbar->AddControl(cam);
#ifndef _DARWIN
	stb = new wxStaticText(m_options_toolbar, wxID_ANY, "",
		wxDefaultPosition, wxSize(1, tbs.y-2));
	stb->SetBackgroundColour(wxColour(128,128,128));
	m_options_toolbar->AddControl(stb);
#endif

	bitmap = wxGetBitmapFromMemory(info);
	m_options_toolbar->AddCheckTool(
		ID_FpsChk, "Info",
		bitmap, wxNullBitmap,
		"Toggle View of FPS and Mouse Position",
		"Toggle View of FPS and Mouse Position");
	m_options_toolbar->ToggleTool(ID_FpsChk,false);

	bitmap = wxGetBitmapFromMemory(axis);
	m_options_toolbar->AddCheckTool(
		ID_CamCtrChk, "Axis",
		bitmap, wxNullBitmap,
		"Toggle View of the Center Axis",
		"Toggle View of the Center Axis");
	m_options_toolbar->ToggleTool(ID_CamCtrChk,false);

	bitmap = wxGetBitmapFromMemory(legend);
	m_options_toolbar->AddCheckTool(
		ID_LegendChk, "Legend",
		bitmap, wxNullBitmap,
		"Toggle View of the Legend",
		"Toggle View of the Legend");
	m_options_toolbar->ToggleTool(ID_LegendChk,false);

	bitmap = wxGetBitmapFromMemory(colormap);
	m_options_toolbar->AddCheckTool(
		ID_ColormapChk, "Color Map",
		bitmap, wxNullBitmap,
		"Toggle View of the Colormap Sample",
		"Toggle View of the Colormap Sample");
	m_options_toolbar->ToggleTool(ID_ColormapChk, false);
#ifndef _DARWIN
	stb = new wxStaticText(m_options_toolbar, wxID_ANY, "",
		wxDefaultPosition, wxSize(1, tbs.y-2));
	stb->SetBackgroundColour(wxColour(128,128,128));
	m_options_toolbar->AddControl(stb);
#endif

	//scale bar
	bitmap = wxGetBitmapFromMemory(scalebar);
	m_options_toolbar->AddTool(
		ID_ScaleBar, "Scale Bar", bitmap,
		"Toggle Scalebar Options (Off, On, On with text)");
	m_scale_text = new wxTextCtrl(m_options_toolbar, ID_ScaleText, "50",
		wxDefaultPosition, wxSize(35, 20), 0, vald_int);
	m_scale_text->Disable();
	m_scale_cmb = new wxComboBox(m_options_toolbar, ID_ScaleCmb, "",
		wxDefaultPosition, wxSize(50, 30), 0, NULL, wxCB_READONLY);
	m_scale_cmb->Append("nm");
	m_scale_cmb->Append(L"\u03BCm");
	m_scale_cmb->Append("mm");
	m_scale_cmb->Select(1);
	m_scale_cmb->Disable();

	m_options_toolbar->AddControl(m_scale_text);
	m_options_toolbar->AddControl(m_scale_cmb);

	//m_options_toolbar->Realize();
#ifndef _DARWIN
	m_options_toolbar->AddStretchableSpace();
#endif

	//background option
	m_bg_color_picker = new wxColourPickerCtrl(m_options_toolbar,
		ID_BgColorPicker);
	wxSize bs = m_bg_color_picker->GetSize();
	m_bg_inv_btn = new wxButton(m_options_toolbar, ID_BgInvBtn, L"\u262f",
		wxDefaultPosition, wxSize(bs.y, bs.y));
#ifdef _WIN32
	wxFont font(34, wxFONTFAMILY_DEFAULT, wxNORMAL, wxNORMAL);
#else
	wxFont font(17, wxFONTFAMILY_DEFAULT, wxNORMAL, wxNORMAL);
#endif
	m_bg_inv_btn->SetFont(font);
	m_options_toolbar->AddControl(m_bg_color_picker);
	m_options_toolbar->AddControl(m_bg_inv_btn);

#ifndef _DARWIN
	stb = new wxStaticText(m_options_toolbar, wxID_ANY, "",
		wxDefaultPosition, wxSize(1, tbs.y-2));
	stb->SetBackgroundColour(wxColour(128, 128, 128));
	m_options_toolbar->AddControl(stb);
#endif

	//angle of view
	st2 = new wxStaticText(m_options_toolbar, wxID_ANY, "Projection:");
	m_aov_sldr = new wxSlider(m_options_toolbar, ID_AovSldr, 45, 10, 100,
		wxDefaultPosition, wxSize(180, 20), wxSL_HORIZONTAL);
	m_aov_sldr->SetValue(GetPersp()?GetAov():10);
	m_aov_sldr->Connect(wxID_ANY, wxEVT_IDLE,
		wxIdleEventHandler(VRenderView::OnAovSldrIdle),
		NULL, this);
	m_aov_text = new wxTextCtrl(m_options_toolbar, ID_AovText, "",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	m_aov_text->ChangeValue(GetPersp()?wxString::Format("%d", 
		int(GetAov())):"Ortho");
	m_options_toolbar->AddControl(st2);
	m_options_toolbar->AddControl(m_aov_sldr);
	m_options_toolbar->AddControl(m_aov_text);

	bitmap = wxGetBitmapFromMemory(freefly);
	m_options_toolbar->AddCheckTool(
		ID_FreeChk, "Free Fly",
		bitmap, wxNullBitmap,
		"Change the camera to a 'Free-Fly' Mode",
		"Change the camera to a 'Free-Fly' Mode");

	if (GetFree())
		m_options_toolbar->ToggleTool(ID_FreeChk,true);
	else
		m_options_toolbar->ToggleTool(ID_FreeChk,false);

#ifndef _DARWIN
	stb = new wxStaticText(m_options_toolbar, wxID_ANY, "",
		wxDefaultPosition, wxSize(1, tbs.y-2));
	stb->SetBackgroundColour(wxColour(128, 128, 128));
	m_options_toolbar->AddControl(stb);
#endif

	//save default
	bitmap = wxGetBitmapFromMemory(save_settings);
	m_options_toolbar->AddTool(
		ID_DefaultBtn, "Save", bitmap,
		"Set Default Render View Settings");

	m_options_toolbar->SetRows(1);
	m_options_toolbar->Realize();

	m_full_screen_btn = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_FLAT|wxTB_NODIVIDER);
	bitmap = wxGetBitmapFromMemory(full_view);
	m_full_screen_btn->AddTool(
		ID_FullScreenBtn, "Full Screen",
		bitmap, "Show full screen");
	m_full_screen_btn->Realize();

	//add the toolbars and other options in order
	sizer_h_1->AddSpacer(40);
	sizer_h_1->Add(m_options_toolbar,1, wxALIGN_CENTER);
	sizer_h_1->Add(m_full_screen_btn, 0, wxALIGN_CENTER);

	//bar left///////////////////////////////////////////////////
	wxBoxSizer* sizer_v_3 = new wxBoxSizer(wxVERTICAL);
	m_left_toolbar = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	bitmap = wxGetBitmapFromMemory(no_depth_atten);
#ifdef _DARWIN
	m_left_toolbar->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_left_toolbar->AddCheckTool(ID_DepthAttenChk, "Depth Interval",
		bitmap, wxNullBitmap,
		"Enable adjustment of the Depth Attenuation Interval",
		"Enable adjustment of the Depth Attenuation Interval");
	m_left_toolbar->ToggleTool(ID_DepthAttenChk, true);
	m_depth_atten_factor_sldr = new wxSlider(this, ID_DepthAttenFactorSldr, 0, 0, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL|wxSL_INVERSE);
	m_depth_atten_factor_sldr->Disable();
	m_depth_atten_reset_btn = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	bitmap = wxGetBitmapFromMemory(reset);
#ifdef _DARWIN
	m_depth_atten_reset_btn->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_depth_atten_reset_btn->AddTool(
		ID_DepthAttenResetBtn, "Reset",
		bitmap, "Reset Depth Attenuation Interval");
	m_depth_atten_reset_btn->Realize();
	m_depth_atten_factor_text = new wxTextCtrl(this, ID_DepthAttenFactorText, "0.0",
		wxDefaultPosition, wxSize(40, 20), 0, vald_fp2);
	m_depth_atten_factor_text->Disable();

	m_left_toolbar->Realize();

	sizer_v_3->AddSpacer(50);
	sizer_v_3->Add(m_left_toolbar, 0, wxALIGN_CENTER);
	sizer_v_3->Add(m_depth_atten_factor_sldr, 1, wxALIGN_CENTER);
	sizer_v_3->Add(m_depth_atten_factor_text, 0, wxALIGN_CENTER);
	sizer_v_3->Add(m_depth_atten_reset_btn, 0, wxALIGN_CENTER);
	sizer_v_3->AddSpacer(50);

	//bar right///////////////////////////////////////////////////
	wxBoxSizer* sizer_v_4 = new wxBoxSizer(wxVERTICAL);
	m_pin_btn = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	bitmap = wxGetBitmapFromMemory(anchor_dark);
#ifdef _DARWIN
	m_pin_btn->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_pin_btn->AddCheckTool(ID_PinBtn, "Pin",
		bitmap, wxNullBitmap,
		"Anchor the rotation center on data",
		"Anchor the rotation center on data");
	m_pin_btn->ToggleTool(ID_PinBtn, false);
	m_pin_btn->Realize();
	m_center_btn = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	bitmap = wxGetBitmapFromMemory(center);
#ifdef _DARWIN
	m_center_btn->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_center_btn->AddTool(ID_CenterBtn, "Center",
		bitmap, "Center the Data on the Render View");
	m_center_btn->Realize();
	m_scale_121_btn = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	bitmap = wxGetBitmapFromMemory(ratio);
#ifdef _DARWIN
	m_scale_121_btn->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_scale_121_btn->AddTool(ID_Scale121Btn, "1 to 1",
		bitmap, "Auto-size the data to a 1:1 ratio");
	m_scale_121_btn->Realize();
	m_scale_factor_sldr = new wxSlider(this, ID_ScaleFactorSldr, 100, 50, 999,
		wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL);
	m_scale_reset_btn = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	bitmap = wxGetBitmapFromMemory(reset);
#ifdef _DARWIN
	m_scale_reset_btn->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_scale_reset_btn->AddTool(ID_ScaleResetBtn, "Reset",
		bitmap, "Reset the Zoom");
	m_scale_reset_btn->Realize();
	m_scale_factor_text = new wxTextCtrl(this, ID_ScaleFactorText, "100",
		wxDefaultPosition, wxSize(40, 20), 0, vald_int);
	m_scale_factor_spin = new wxSpinButton(this, ID_ScaleFactorSpin,
		wxDefaultPosition, wxSize(40, 20));
	m_scale_factor_spin->SetRange(-0x8000, 0x7fff);
	m_scale_mode_btn = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	bitmap = wxGetBitmapFromMemory(zoom_view);
#ifdef _DARWIN
	m_scale_mode_btn->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_scale_mode_btn->AddTool(
		ID_ScaleModeBtn, "Switch zoom ratio mode",
		bitmap, "View-based zoom ratio");
	m_scale_mode_btn->Realize();
	sizer_v_4->AddSpacer(50);
	sizer_v_4->Add(m_pin_btn, 0, wxALIGN_CENTER);
	sizer_v_4->Add(m_center_btn, 0, wxALIGN_CENTER);
	sizer_v_4->Add(m_scale_121_btn, 0, wxALIGN_CENTER);
	sizer_v_4->Add(m_scale_factor_sldr, 1, wxALIGN_CENTER);
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
	m_lower_toolbar = new wxToolBar(this,wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	st1 = new wxStaticText(this, 0, "X:");
	m_x_rot_sldr = new wxScrollBar(this, ID_XRotSldr);
	m_x_rot_sldr->SetScrollbar(180,60,420,15);
	m_x_rot_text = new wxTextCtrl(this, ID_XRotText, "0.0",
		wxDefaultPosition, wxSize(45,20), 0, vald_fp1);
	st2 = new wxStaticText(this, 0, "Y:");
	m_y_rot_sldr = new wxScrollBar(this, ID_YRotSldr);
	m_y_rot_sldr->SetScrollbar(180,60,420,15);
	m_y_rot_text = new wxTextCtrl(this, ID_YRotText, "0.0",
		wxDefaultPosition, wxSize(45,20), 0, vald_fp1);
	st3 = new wxStaticText(this, 0, "Z:");
	m_z_rot_sldr = new wxScrollBar(this, ID_ZRotSldr);
	m_z_rot_sldr->SetScrollbar(180,60,420,15);
	m_z_rot_text = new wxTextCtrl(this, ID_ZRotText, "0.0",
		wxDefaultPosition, wxSize(45,20), 0, vald_fp1);

	//45 lock
	m_rot_lock_btn = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	bitmap = wxGetBitmapFromMemory(gear_dark);
#ifdef _DARWIN
	m_rot_lock_btn->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_rot_lock_btn->AddCheckTool(ID_RotLockChk, "45 Angles",
		bitmap, wxNullBitmap,
		"Confine all angles to 45 Degrees",
		"Confine all angles to 45 Degrees");
	bitmap = wxGetBitmapFromMemory(slider_type_rot);
	m_rot_lock_btn->AddCheckTool(ID_RotSliderType, "Slider Style",
		bitmap, wxNullBitmap,
		"Choose slider style",
		"Choose slider style");
	m_rot_lock_btn->ToggleTool(ID_RotSliderType, m_rot_slider);
	m_rot_lock_btn->Realize();

	//ortho view selector
	m_ortho_view_cmb = new wxComboBox(m_lower_toolbar, ID_OrthoViewCmb, "",
		wxDefaultPosition, wxSize(50, 30), 0, NULL, wxCB_READONLY);
	m_ortho_view_cmb->Append("+X");
	m_ortho_view_cmb->Append("-X");
	m_ortho_view_cmb->Append("+Y");
	m_ortho_view_cmb->Append("-Y");
	m_ortho_view_cmb->Append("+Z");
	m_ortho_view_cmb->Append("-Z");
	m_ortho_view_cmb->Append("NA");
	m_ortho_view_cmb->Select(6);
	m_lower_toolbar->AddControl(m_ortho_view_cmb);
	bitmap = wxGetBitmapFromMemory(zrot);
#ifdef _DARWIN
	m_lower_toolbar->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_lower_toolbar->AddTool(ID_ZeroRotBtn, "Set Zeros",
		bitmap, "Set current angles as zeros");
	bitmap = wxGetBitmapFromMemory(reset);
	m_lower_toolbar->AddTool(ID_RotResetBtn,"Reset",
		bitmap, "Reset Rotations");
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
	sizer_h_2->Add(m_lower_toolbar, 0, wxALIGN_CENTER);
	sizer_h_2->AddSpacer(40);

	sizer_v->Add(sizer_h_1, 0, wxEXPAND);
	sizer_v->Add(sizer_m, 1, wxEXPAND);
	sizer_v->Add(sizer_h_2, 0, wxEXPAND);

	SetSizer(sizer_v);
	Layout();

}

//recalculate view according to object bounds
void VRenderView::InitView(unsigned int type)
{
	if (m_glview)
		m_glview->InitView(type);
	if (m_use_dft_settings)
		UpdateView();
}

void VRenderView::Clear()
{
	if (m_glview)
		m_glview->Clear();
	ClearVolList();
	ClearMeshList();
}

//data management
int VRenderView::GetDispVolumeNum()
{
	if (m_glview)
		return m_glview->GetDispVolumeNum();
	else
		return 0;
}

int VRenderView::GetAny()
{
	if (m_glview)
		return m_glview->GetAny();
	else
		return 0;
}

int VRenderView::GetAllVolumeNum()
{
	if (m_glview)
		return m_glview->GetAllVolumeNum();
	else return 0;
}

int VRenderView::GetMeshNum()
{
	if (m_glview)
		return m_glview->GetMeshNum();
	else
		return 0;
}

int VRenderView::GetGroupNum()
{
	if (m_glview)
		return m_glview->GetGroupNum();
	else
		return 0;
}

int VRenderView::GetLayerNum()
{
	if (m_glview)
		return m_glview->GetLayerNum();
	else
		return 0;
}

VolumeData* VRenderView::GetAllVolumeData(int index)
{
	if (m_glview)
		return m_glview->GetAllVolumeData(index);
	else
		return 0;
}

VolumeData* VRenderView::GetDispVolumeData(int index)
{
	if (m_glview)
		return m_glview->GetDispVolumeData(index);
	else
		return 0;
}

MeshData* VRenderView::GetMeshData(int index)
{
	if (m_glview)
		return m_glview->GetMeshData(index);
	else
		return 0;
}

TreeLayer* VRenderView::GetLayer(int index)
{
	if (m_glview)
		return m_glview->GetLayer(index);
	else
		return 0;
}

MultiVolumeRenderer* VRenderView::GetMultiVolumeData()
{
	if (m_glview)
		return m_glview->GetMultiVolumeData();
	else
		return 0;
}

VolumeData* VRenderView::GetVolumeData(wxString &name)
{
	if (m_glview)
		return m_glview->GetVolumeData(name);
	else
		return 0;
}

MeshData* VRenderView::GetMeshData(wxString &name)
{
	if (m_glview)
		return m_glview->GetMeshData(name);
	else
		return 0;
}

Annotations* VRenderView::GetAnnotations(wxString &name)
{
	if (m_glview)
		return m_glview->GetAnnotations(name);
	else
		return 0;
}

DataGroup* VRenderView::GetGroup(wxString &name)
{
	if (m_glview)
		return m_glview->GetGroup(name);
	else
		return 0;
}

DataGroup* VRenderView::GetGroup(int index)
{
	if (m_glview)
		return m_glview->GetGroup(index);
	else
		return 0;
}

DataGroup* VRenderView::AddVolumeData(VolumeData* vd, wxString group_name)
{
	if (m_glview) {
		double val = 50.;
		m_scale_text->GetValue().ToDouble(&val);
		m_glview->SetScaleBarLen(val);
		return m_glview->AddVolumeData(vd, group_name);
	}
	else
		return 0;
}

void VRenderView::AddMeshData(MeshData* md)
{
	if (m_glview)
		m_glview->AddMeshData(md);
}

void VRenderView::AddAnnotations(Annotations* ann)
{
	if (m_glview)
		m_glview->AddAnnotations(ann);
}

wxString VRenderView::AddGroup(wxString str, wxString prev_group)
{
	if (m_glview)
		return m_glview->AddGroup(str);
	else
		return "";
}

DataGroup* VRenderView::AddOrGetGroup()
{
	if (m_glview)
		return m_glview->AddOrGetGroup();
	else
		return 0;
}

wxString VRenderView::AddMGroup(wxString str)
{
	if (m_glview)
		return m_glview->AddMGroup(str);
	else
		return "";
}

MeshGroup* VRenderView::AddOrGetMGroup()
{
	if (m_glview)
		return m_glview->AddOrGetMGroup();
	else
		return 0;
}

MeshGroup* VRenderView::GetMGroup(wxString &str)
{
	if (m_glview)
		return m_glview->GetMGroup(str);
	else return 0;
}

void VRenderView::RemoveVolumeData(wxString &name)
{
	if (m_glview)
		m_glview->RemoveVolumeData(name);
}

void VRenderView::RemoveVolumeDataDup(wxString &name)
{
	if (m_glview)
		m_glview->RemoveVolumeDataDup(name);
}

void VRenderView::RemoveMeshData(wxString &name)
{
	if (m_glview)
		m_glview->RemoveMeshData(name);
}

void VRenderView::RemoveAnnotations(wxString &name)
{
	if (m_glview)
		m_glview->RemoveAnnotations(name);
}

void VRenderView::RemoveGroup(wxString &name)
{
	if (m_glview)
		m_glview->RemoveGroup(name);
}

void VRenderView::Isolate(int type, wxString name)
{
	if (m_glview)
		m_glview->Isolate(type, name);
}

void VRenderView::ShowAll()
{
	if (m_glview)
		m_glview->ShowAll();
}

//move
void VRenderView::MoveLayerinView(wxString &src_name, wxString &dst_name)
{
	if (m_glview)
		m_glview->MoveLayerinView(src_name, dst_name);
}

//move volume
void VRenderView::MoveLayerinGroup(wxString &group_name, wxString &src_name, wxString &dst_name)
{
	if (m_glview)
		m_glview->MoveLayerinGroup(group_name, src_name, dst_name);
}

void VRenderView::MoveLayertoView(wxString &group_name, wxString &src_name, wxString &dst_name)
{
	if (m_glview)
		m_glview->MoveLayertoView(group_name, src_name, dst_name);
}

void VRenderView::MoveLayertoGroup(wxString &group_name, wxString &src_name, wxString &dst_name)
{
	if (m_glview)
		m_glview->MoveLayertoGroup(group_name, src_name, dst_name);
}

void VRenderView::MoveLayerfromtoGroup(wxString &src_group_name,
	wxString &dst_group_name,
	wxString &src_name, wxString &dst_name)
{
	if (m_glview)
		m_glview->MoveLayerfromtoGroup(src_group_name,
		dst_group_name, src_name, dst_name);
}

//move mesh
void VRenderView::MoveMeshinGroup(wxString &group_name, wxString &src_name, wxString &dst_name)
{
	if (m_glview)
		m_glview->MoveMeshinGroup(group_name, src_name, dst_name);
}

void VRenderView::MoveMeshtoView(wxString &group_name, wxString &src_name, wxString &dst_name)
{
	if (m_glview)
		m_glview->MoveMeshtoView(group_name, src_name, dst_name);
}

void VRenderView::MoveMeshtoGroup(wxString &group_name, wxString &src_name, wxString &dst_name)
{
	if (m_glview)
		m_glview->MoveMeshtoGroup(group_name, src_name, dst_name);
}

void VRenderView::MoveMeshfromtoGroup(wxString &src_group_name,
	wxString &dst_group_name,
	wxString &src_name, wxString &dst_name)
{
	if (m_glview)
		m_glview->MoveMeshfromtoGroup(src_group_name,
		dst_group_name, src_name, dst_name);
}

//reorganize layers in view
void VRenderView::OrganizeLayers()
{
	if (m_glview)
		m_glview->OrganizeLayers();
}

//randomize color
void VRenderView::RandomizeColor()
{
	if (m_glview)
		m_glview->RandomizeColor();
}

//toggle hiding/displaying
void VRenderView::SetDraw(bool draw)
{
	if (m_glview)
		m_glview->SetDraw(draw);
}

void VRenderView::ToggleDraw()
{
	if (m_glview)
		m_glview->ToggleDraw();
}

bool VRenderView::GetDraw()
{
	if (m_glview)
		return m_glview->GetDraw();
	else
		return false;
}

//camera operations
void VRenderView::GetTranslations(double &transx, double &transy, double &transz)
{
	if (m_glview)
		m_glview->GetTranslations(transx, transy, transz);
}

void VRenderView::SetTranslations(double transx, double transy, double transz)
{
	if (m_glview)
		m_glview->SetTranslations(transx, transy, transz);
}

void VRenderView::GetRotations(double &rotx, double &roty, double &rotz)
{
	if (m_glview)
		m_glview->GetRotations(rotx, roty, rotz);
}

void VRenderView::SetRotations(double rotx, double roty, double rotz, bool ui_update)
{
	if (m_glview)
		m_glview->SetRotations(rotx, roty, rotz, ui_update);

}

void VRenderView::GetCenters(double &ctrx, double &ctry, double &ctrz)
{
	if (m_glview)
		m_glview->GetCenters(ctrx, ctry, ctrz);
}

void VRenderView::SetCenters(double ctrx, double ctry, double ctrz)
{
	if (m_glview)
		m_glview->SetCenters(ctrx, ctry, ctrz);
}

double VRenderView::GetCenterEyeDist()
{
	if (m_glview)
		return m_glview->GetCenterEyeDist();
	else
		return 0.0;
}

void VRenderView::SetCenterEyeDist(double dist)
{
	if (m_glview)
		m_glview->SetCenterEyeDist(dist);
}

double VRenderView::GetRadius()
{
	if (m_glview)
		return m_glview->GetRadius();
	else
		return 0.0;
}

void VRenderView::SetRadius(double r)
{
	if (m_glview)
		m_glview->SetRadius(r);
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
		double spcx, spcy, spcz;
		if (vd)
			vd->GetSpacings(spcx, spcy, spcz, vd->GetLevel());
		if (spcx > 0.0)
				scale /= m_glview->Get121ScaleFactor() * spcx;
		}
		break;
	}
	int val = int(scale * 100 + 0.5);
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
					wxGetBitmapFromMemory(pin));
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
					wxGetBitmapFromMemory(anchor_dark));
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
			wxGetBitmapFromMemory(zoom_view));
		m_scale_mode_btn->SetToolShortHelp(ID_ScaleModeBtn,
			"View-based zoom ratio");
		m_scale_mode_btn->SetToolLongHelp(ID_ScaleModeBtn,
			"View-based zoom ratio (View entire data set at 100%)");
		break;
	case 1:
		m_scale_mode_btn->SetToolNormalBitmap(ID_ScaleModeBtn,
			wxGetBitmapFromMemory(zoom_pixel));
		m_scale_mode_btn->SetToolShortHelp(ID_ScaleModeBtn,
			"Pixel-based zoom ratio");
		m_scale_mode_btn->SetToolLongHelp(ID_ScaleModeBtn,
			"Pixel-based zoom ratio (View 1 data pixel to 1 screen pixel at 100%)");
		break;
	case 2:
		m_scale_mode_btn->SetToolNormalBitmap(ID_ScaleModeBtn,
			wxGetBitmapFromMemory(zoom_data));
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

//object operations
void VRenderView::GetObjCenters(double &ctrx, double &ctry, double &ctrz)
{
	if (m_glview)
		m_glview->GetObjCenters(ctrx, ctry, ctrz);
}

void VRenderView::SetObjCenters(double ctrx, double ctry, double ctrz)
{
	if (m_glview)
		m_glview->SetObjCenters(ctrx, ctry, ctrz);
}

void VRenderView::GetObjTrans(double &transx, double &transy, double &transz)
{
	if (m_glview)
		m_glview->GetObjTrans(transx, transy, transz);
}

void VRenderView::SetObjTrans(double transx, double transy, double transz)
{
	if (m_glview)
		m_glview->SetObjTrans(transx, transy, transz);
}

void VRenderView::GetObjRot(double &rotx, double &roty, double &rotz)
{
	if (m_glview)
		m_glview->GetObjRot(rotx, roty, rotz);
}

void VRenderView::SetObjRot(double rotx, double roty, double rotz)
{
	if (m_glview)
		m_glview->SetObjRot(rotx, roty, rotz);
}

//camera properties
double VRenderView::GetAov()
{
	if (m_glview)
		return m_glview->GetAov();
	else
		return 0.0;
}

void VRenderView::SetAov(double aov)
{
	if (m_glview)
		m_glview->SetAov(aov);
}

double VRenderView::GetNearClip()
{
	if (m_glview)
		return m_glview->GetNearClip();
	else
		return 0.0;
}

void VRenderView::SetNearClip(double nc)
{
	if (m_glview)
		m_glview->SetNearClip(nc);
}

double VRenderView::GetFarClip()
{
	if (m_glview)
		return m_glview->GetFarClip();
	else
		return 0.0;
}

void VRenderView::SetFarClip(double fc)
{
	if (m_glview)
		m_glview->SetFarClip(fc);
}

//background color
Color VRenderView::GetBackgroundColor()
{
	if (m_glview)
		return m_glview->GetBackgroundColor();
	else
		return Color(0, 0, 0);
}

void VRenderView::SetBackgroundColor(Color &color)
{
	if (m_glview)
		m_glview->SetBackgroundColor(color);
	wxColor c(int(color.r()*255.0), int(color.g()*255.0), int(color.b()*255.0));
	m_bg_color_picker->SetColour(c);
}

void VRenderView::SetGradBg(bool val)
{
	if (m_glview)
		m_glview->SetGradBg(val);
}

//rot center anchor thresh
void VRenderView::SetPinThreshold(double value)
{
	m_pin_scale_thresh = value;
}

//point volume mode
void VRenderView::SetPointVolumeMode(int mode)
{
	if (m_glview)
		m_glview->m_point_volume_mode = mode;
}

int VRenderView::GetPointVolumeMode()
{
	if (m_glview)
		return m_glview->m_point_volume_mode;
	else
		return 0;
}

//ruler uses trnasfer function
void VRenderView::SetRulerUseTransf(bool val)
{
	if (m_glview)
		m_glview->m_ruler_use_transf = val;
}

bool VRenderView::GetRulerUseTransf()
{
	if (m_glview)
		return m_glview->m_ruler_use_transf;
	else
		return false;
}

//ruler time dependent
void VRenderView::SetRulerTimeDep(bool val)
{
	if (m_glview)
		m_glview->m_ruler_time_dep = val;
}

bool VRenderView::GetRulerTimeDep()
{
	if (m_glview)
		return m_glview->m_ruler_time_dep;
	else
		return true;
}

//disply modes
int VRenderView::GetDrawType()
{
	if (m_glview)
		return m_glview->GetDrawType();
	else
		return 0;
}

void VRenderView::SetVolMethod(int method)
{
	if (m_glview)
		m_glview->SetVolMethod(method);
}

int VRenderView::GetVolMethod()
{
	if (m_glview)
		return m_glview->GetVolMethod();
	else
		return 0;
}

//other properties
void VRenderView::SetPeelingLayers(int n)
{
	if (m_glview)
		m_glview->SetPeelingLayers(n);
}

int VRenderView::GetPeelingLayers()
{
	if (m_glview)
		return m_glview->GetPeelingLayers();
	else
		return 0;
}

void VRenderView::SetBlendSlices(bool val)
{
	if (m_glview)
		m_glview->SetBlendSlices(val);
}

bool VRenderView::GetBlendSlices()
{
	if (m_glview)
		return m_glview->GetBlendSlices();
	else
		return false;
}

void VRenderView::SetAdaptive(bool val)
{
	if (m_glview)
		m_glview->SetAdaptive(val);
}

bool VRenderView::GetAdaptive()
{
	if (m_glview)
		return m_glview->GetAdaptive();
	else
		return false;
}

void VRenderView::SetFog(bool b)
{
	if (m_glview)
		m_glview->SetFog(b);
	if (m_left_toolbar)
		m_left_toolbar->ToggleTool(ID_DepthAttenChk, b);
}

bool VRenderView::GetFog()
{
	if (m_glview)
		return m_glview->GetFog();
	else
		return false;
}

void VRenderView::SetFogIntensity(double i)
{
	if (m_glview)
		m_glview->SetFogIntensity(i);
}

double VRenderView::GetFogIntensity()
{
	if (m_glview)
		return m_glview->GetFogIntensity();
	else
		return 0.0;
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
		SetVolMethod(VOL_METHOD_SEQ);
		break;
	case ID_VolumeMultiRd:
		SetVolMethod(VOL_METHOD_MULTI);
		break;
	case ID_VolumeCompRd:
		SetVolMethod(VOL_METHOD_COMP);
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
	FLIVR::Color col = m_glview->GetGamma();
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
	if (ch1)
		VRenderFrame::SetCompression(ch1->GetValue());
}

//save alpha
void VRenderView::OnChAlphaCheck(wxCommandEvent &event)
{
	wxCheckBox* ch_alpha = (wxCheckBox*)event.GetEventObject();
	if (ch_alpha)
		VRenderFrame::SetSaveAlpha(ch_alpha->GetValue());
}

//save float
void VRenderView::OnChFloatCheck(wxCommandEvent &event)
{
	wxCheckBox* ch_float = (wxCheckBox*)event.GetEventObject();
	if (ch_float)
		VRenderFrame::SetSaveFloat(ch_float->GetValue());
}

//embde project
void VRenderView::OnChEmbedCheck(wxCommandEvent &event)
{
	wxCheckBox* ch_embed = (wxCheckBox*)event.GetEventObject();
	if (ch_embed)
		VRenderFrame::SetEmbedProject(ch_embed->GetValue());
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
	int ival = int(dval * 10 + 0.5);
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
	wxPanel* panel = new wxPanel(parent, 0, wxDefaultPosition, wxSize(600, 150));
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
		ch1->SetValue(VRenderFrame::GetCompression());

	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	//save alpha
	wxCheckBox* ch_alpha = new wxCheckBox(panel, ID_SAVE_ALPHA,
		"Save alpha channel");
	ch_alpha->Connect(ch_alpha->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(VRenderView::OnChAlphaCheck), NULL, panel);
	if (ch_alpha)
		ch_alpha->SetValue(VRenderFrame::GetSaveAlpha());
	//save float
	wxCheckBox* ch_float = new wxCheckBox(panel, ID_SAVE_FLOAT,
		"Save float channel");
	ch_float->Connect(ch_float->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(VRenderView::OnChFloatCheck), NULL, panel);
	if (ch_float)
		ch_float->SetValue(VRenderFrame::GetSaveFloat());
	sizer_1->Add(ch_alpha, 0, wxALIGN_CENTER);
	sizer_1->Add(10, 10);
	sizer_1->Add(ch_float, 0, wxALIGN_CENTER);

	//enlarge
	wxBoxSizer* sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	wxCheckBox* ch_enlarge = new wxCheckBox(panel, ID_ENLARGE_CHK,
		"Enlarge output image");
	ch_enlarge->Connect(ch_enlarge->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(VRenderView::OnChEnlargeCheck), NULL, panel);
	wxSlider* sl_enlarge = new wxSlider(panel, ID_ENLARGE_SLDR,
		10, 10, 100);
	sl_enlarge->Connect(sl_enlarge->GetId(), wxEVT_COMMAND_SLIDER_UPDATED,
		wxScrollEventHandler(VRenderView::OnSlEnlargeScroll), NULL, panel);
	sl_enlarge->Disable();
	wxFloatingPointValidator<double> vald_fp(1);
	wxTextCtrl* tx_enlarge = new wxTextCtrl(panel, ID_ENLARGE_TEXT,
		"1.0", wxDefaultPosition, wxSize(60, 23), 0, vald_fp);
	tx_enlarge->Connect(tx_enlarge->GetId(), wxEVT_COMMAND_TEXT_UPDATED,
		wxCommandEventHandler(VRenderView::OnTxEnlargeText), NULL, panel);
	tx_enlarge->Disable();
	sizer_2->Add(ch_enlarge, 0, wxALIGN_CENTER);
	sizer_2->Add(10, 10);
	sizer_2->Add(sl_enlarge, 1, wxEXPAND);
	sizer_2->Add(10, 10);
	sizer_2->Add(tx_enlarge, 0, wxALIGN_CENTER);

	//copy all files check box
	wxCheckBox* ch_embed = 0;
	if (VRenderFrame::GetSaveProject())
	{
		ch_embed = new wxCheckBox(panel, ID_EMBED_FILES,
			"Embed all files in the project folder");
		ch_embed->Connect(ch_embed->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
			wxCommandEventHandler(VRenderView::OnChEmbedCheck), NULL, panel);
		ch_embed->SetValue(VRenderFrame::GetEmbedProject());
	}

	//group
	group1->Add(10, 10);
	group1->Add(ch1);
	group1->Add(10, 10);
	group1->Add(sizer_1);
	group1->Add(10, 10);
	group1->Add(sizer_2);
	group1->Add(10, 10);
	if (VRenderFrame::GetSaveProject() &&
		ch_embed)
	{
		group1->Add(ch_embed);
		group1->Add(10, 10);
	}

	panel->SetSizer(group1);
	panel->Layout();

	return panel;
}

void VRenderView::OnCapture(wxCommandEvent& event)
{
	//reset enlargement
	VRenderGLView::SetEnlarge(false);
	VRenderGLView::SetEnlargeScale(1.0);

	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;

	if (vr_frame && vr_frame->GetSettingDlg())
	{
		VRenderFrame::SetSaveProject(vr_frame->GetSettingDlg()->GetProjSave());
		VRenderFrame::SetSaveAlpha(vr_frame->GetSettingDlg()->GetSaveAlpha());
		VRenderFrame::SetSaveFloat(vr_frame->GetSettingDlg()->GetSaveFloat());
	}

	wxFileDialog file_dlg(m_frame, "Save captured image", "", "", "*.tif", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
	file_dlg.SetExtraControlCreator(CreateExtraCaptureControl);
	int rval = file_dlg.ShowModal();
	if (rval == wxID_OK)
	{
		m_glview->m_cap_file = file_dlg.GetDirectory() + GETSLASH() + file_dlg.GetFilename();
		m_glview->m_capture = true;
		RefreshGL();

		if (vr_frame && vr_frame->GetSettingDlg())
		{
			if (vr_frame->GetSettingDlg()->GetProjSave())
			{
				wxString new_folder;
				new_folder = m_glview->m_cap_file + "_project";
				wxFileName::Mkdir(new_folder, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
				wxString prop_file = new_folder + GETSLASH() + file_dlg.GetFilename() + "_project.vrp";
				vr_frame->SaveProject(prop_file);
			}
			vr_frame->GetSettingDlg()->SetSaveAlpha(VRenderFrame::GetSaveAlpha());
			vr_frame->GetSettingDlg()->SetSaveFloat(VRenderFrame::GetSaveFloat());
		}
	}
}


//bar left
void VRenderView::OnDepthAttenCheck(wxCommandEvent& event)
{
	if (m_left_toolbar->GetToolState(ID_DepthAttenChk))
	{
		SetFog(true);
		m_depth_atten_factor_sldr->Enable();
		m_depth_atten_factor_text->Enable();
		m_left_toolbar->SetToolNormalBitmap (ID_DepthAttenChk,
			wxGetBitmapFromMemory(depth_atten));
	}
	else
	{
		SetFog(false);
		m_depth_atten_factor_sldr->Disable();
		m_depth_atten_factor_text->Disable();
		m_left_toolbar->SetToolNormalBitmap (ID_DepthAttenChk,
			wxGetBitmapFromMemory(no_depth_atten));
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
	SetFogIntensity(val);
	m_depth_atten_factor_sldr->SetValue(int(val*100.0));
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
			wxGetBitmapFromMemory(pin));
		if (scale > m_pin_scale_thresh)
			m_glview->m_auto_update_rot_center = true;
		else
			m_glview->m_auto_update_rot_center = false;
	}
	else
	{
		m_pin_btn->SetToolNormalBitmap(ID_PinBtn,
			wxGetBitmapFromMemory(anchor_dark));
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
			wxGetBitmapFromMemory(zoom_view));
		m_scale_mode_btn->SetToolShortHelp(ID_ScaleModeBtn,
			"View-based zoom ratio");
		m_scale_mode_btn->SetToolLongHelp(ID_ScaleModeBtn,
			"View-based zoom ratio (View entire data set at 100%)");
		break;
	case 1:
		m_scale_mode_btn->SetToolNormalBitmap(ID_ScaleModeBtn,
			wxGetBitmapFromMemory(zoom_pixel));
		m_scale_mode_btn->SetToolShortHelp(ID_ScaleModeBtn,
			"Pixel-based zoom ratio");
		m_scale_mode_btn->SetToolLongHelp(ID_ScaleModeBtn,
			"Pixel-based zoom ratio (View 1 data pixel to 1 screen pixel at 100%)");
		break;
	case 2:
		m_scale_mode_btn->SetToolNormalBitmap(ID_ScaleModeBtn,
			wxGetBitmapFromMemory(zoom_data));
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
	val++;
	str_val = wxString::Format("%d", val);
	m_scale_factor_text->SetValue(str_val);
}

void VRenderView::OnScaleFactorSpinDown(wxSpinEvent& event)
{
	wxString str_val = m_scale_factor_text->GetValue();
	long val;
	str_val.ToLong(&val);
	val--;
	str_val = wxString::Format("%d", val);
	m_scale_factor_text->SetValue(str_val);
}

void VRenderView::OnScaleReset(wxCommandEvent &event)
{
	if (m_use_dft_settings)
		m_glview->m_scale_factor = m_dft_scale_factor;
	//wxString str = wxString::Format("%d", int(m_dft_scale_factor));
	//m_scale_factor_text->SetValue(str);
	else
		m_glview->m_scale_factor = 1.0;
	UpdateScaleFactor();
	if (m_glview && m_glview->m_mouse_focus)
		m_glview->SetFocus();
}

//bar bottom
void VRenderView::UpdateView(bool ui_update)
{
	double rotx, roty, rotz;
	wxString str_val = m_x_rot_text->GetValue();
	rotx = STOD(str_val.fn_str());
	str_val = m_y_rot_text->GetValue();
	roty = STOD(str_val.fn_str());
	str_val = m_z_rot_text->GetValue();
	rotz = STOD(str_val.fn_str());
	SetRotations(rotx, roty, rotz, ui_update);
	RefreshGL(true);
}

void VRenderView::OnValueEdit(wxCommandEvent& event)
{
	UpdateView(false);
	if (!m_rot_slider)
	{
		double rotx, roty, rotz;
		m_glview->GetRotations(rotx, roty, rotz);
		m_x_rot_sldr->SetThumbPosition(int(rotx));
		m_y_rot_sldr->SetThumbPosition(int(roty));
		m_z_rot_sldr->SetThumbPosition(int(rotz));
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
	SetRotations(0.0, 0.0, 0.0);
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
			pos = int(double(pos) / 45.0) * 45;
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
			pos = int(double(pos) / 45.0) * 45;
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
			pos = int(double(pos) / 45.0) * 45;
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
			wxGetBitmapFromMemory(gear_45));
		rotx = (((int)(rotx/45))*45);
		roty = (((int)(roty/45))*45);
		rotz = (((int)(rotz/45))*45);
	}
	else
	{
		m_rot_lock_btn->SetToolNormalBitmap(ID_RotLockChk,
			wxGetBitmapFromMemory(gear_dark));
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
			wxGetBitmapFromMemory(slider_type_rot));
		m_x_rot_sldr->SetThumbPosition(180);
		m_y_rot_sldr->SetThumbPosition(180);
		m_z_rot_sldr->SetThumbPosition(180);
	}
	else
	{
		m_timer.Stop();
		m_rot_lock_btn->SetToolNormalBitmap(ID_RotSliderType,
			wxGetBitmapFromMemory(slider_type_pos));
		double rotx, roty, rotz;
		m_glview->GetRotations(rotx, roty, rotz);
		m_x_rot_sldr->SetThumbPosition(int(rotx));
		m_y_rot_sldr->SetThumbPosition(int(roty));
		m_z_rot_sldr->SetThumbPosition(int(rotz));
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
		SetRotations(0.0, 90.0, 0.0, true);
		break;
	case 1://-X
		SetRotations(0.0, 270.0, 0.0, true);
		break;
	case 2://+Y
		SetRotations(90.0, 0.0, 0.0, true);
		break;
	case 3://-Y
		SetRotations(270.0, 0.0, 0.0, true);
		break;
	case 4://+Z
		SetRotations(0.0, 0.0, 0.0, true);
		break;
	case 5:
		SetRotations(0.0, 180.0, 0.0, true);
		break;
	}
	if (sel < 6)
	{
		m_rot_lock_btn->ToggleTool(ID_RotLockChk, true);
		m_rot_lock_btn->SetToolNormalBitmap(ID_RotLockChk,
			wxGetBitmapFromMemory(gear_45));
		if (m_glview) m_glview->SetRotLock(true);
	}
	else
	{
		m_rot_lock_btn->ToggleTool(ID_RotLockChk, false);
		m_rot_lock_btn->SetToolNormalBitmap(ID_RotLockChk,
			wxGetBitmapFromMemory(gear_dark));
		if (m_glview) m_glview->SetRotLock(false);
	}
	RefreshGL();
}

//top
void VRenderView::OnBgColorChange(wxColourPickerEvent& event)
{
	wxColor c = event.GetColour();
	Color color(c.Red()/255.0, c.Green()/255.0, c.Blue()/255.0);
	SetBackgroundColor(color);
	RefreshGL();
}

void VRenderView::OnBgInvBtn(wxCommandEvent& event)
{
	Color c = GetBackgroundColor();
	c = Color(1.0, 1.0, 1.0) - c;
	SetBackgroundColor(c);
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

void VRenderView::OnScaleTextEditing(wxCommandEvent& event) {
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
	if (m_glview) {
		//m_glview->SetScaleBarLen(len);
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
	switch (m_draw_scalebar) {
	case kOff:
		m_draw_scalebar = kOn;
		m_glview->m_disp_scale_bar = true;
		m_glview->m_disp_scale_bar_text = false;
		m_options_toolbar->SetToolNormalBitmap(ID_ScaleBar,
			wxGetBitmapFromMemory(scale_text_off));
		m_scale_text->Enable();
		m_scale_cmb->Disable();
		if (m_glview) m_glview->DisableSBText();
		if (m_glview) m_glview->EnableScaleBar();
		break;
	case kOn:
		m_draw_scalebar = kText;
		m_glview->m_disp_scale_bar = true;
		m_glview->m_disp_scale_bar_text = true;
		m_options_toolbar->SetToolNormalBitmap(ID_ScaleBar,
			wxGetBitmapFromMemory(scale_text));
		m_scale_text->Enable();
		m_scale_cmb->Enable();
		if (m_glview) m_glview->EnableSBText();
		if (m_glview) m_glview->EnableScaleBar();
		break;
	case kText:
		m_draw_scalebar = kOff;
		m_glview->m_disp_scale_bar = false;
		m_glview->m_disp_scale_bar_text = false;
		m_options_toolbar->SetToolNormalBitmap(ID_ScaleBar,
			wxGetBitmapFromMemory(scalebar));
		m_scale_text->Disable();
		m_scale_cmb->Disable();
		if (m_glview) m_glview->DisableScaleBar();
		if (m_glview) m_glview->DisableSBText();
		break;
	default:
		break;
	}
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
		SetPersp(false);
		m_aov_sldr->SetValue(10);
		RefreshGL(true);
		return;
	}
	long val;
	if (!str.ToLong(&val))
		return;
	if (val ==0 || val == 10)
	{
		SetPersp(false);
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
		SetPersp(true);
		SetAov(val);
		m_aov_sldr->SetValue(val);
	}
	RefreshGL(true);
}

void VRenderView::OnFreeChk(wxCommandEvent& event)
{
	if (m_options_toolbar->GetToolState(ID_FreeChk))
		SetFree(true); 
	else
	{
		SetFree(false);
		int val = m_aov_sldr->GetValue();
		if (val == 10)
			SetPersp(false);
		else
			SetPersp(true);
	}
	RefreshGL();
}

void VRenderView::SetFullScreen()
{
	if (m_glview->GetParent() != m_full_frame)
	{
		m_view_sizer->Detach(m_glview);
		m_glview->Reparent(m_full_frame);
		m_full_frame->ShowFullScreen(true);
		m_glview->SetPosition(wxPoint(0, 0));
		m_glview->SetSize(m_full_frame->GetSize());
		VRenderFrame* frame = (VRenderFrame*)m_frame;
		if (frame && frame->GetSettingDlg())
		{
			if (frame->GetSettingDlg()->GetStayTop())
				m_full_frame->SetWindowStyle(wxBORDER_NONE|wxSTAY_ON_TOP);
			else
				m_full_frame->SetWindowStyle(wxBORDER_NONE);
#ifdef _WIN32
			if (!frame->GetSettingDlg()->GetShowCursor())
				ShowCursor(false);
#endif
		}
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
	wxString app_name = "FluoRender " +
		wxString::Format("%d.%.1f", VERSION_MAJOR, float(VERSION_MINOR));
	wxString vendor_name = "FluoRender";
	wxString local_name = "default_view_settings.dft";
	wxFileConfig fconfig(app_name, vendor_name, local_name, "",
		wxCONFIG_USE_LOCAL_FILE);
	wxString str;
	bool bVal;
	wxColor cVal;
	double x, y, z;

	//render modes
	if (mask & 0x1)
	{
		bVal = m_options_toolbar->GetToolState(ID_VolumeSeqRd);
		fconfig.Write("volume_seq_rd", bVal);
		bVal = m_options_toolbar->GetToolState(ID_VolumeMultiRd);
		fconfig.Write("volume_multi_rd", bVal);
		bVal = m_options_toolbar->GetToolState(ID_VolumeCompRd);
		fconfig.Write("volume_comp_rd", bVal);
	}
	//background color
	if (mask & 0x2)
	{
		cVal = m_bg_color_picker->GetColour();
		str = wxString::Format("%d %d %d", cVal.Red(), cVal.Green(), cVal.Blue());
		fconfig.Write("bg_color_picker", str);
	}
	//camera center
	if (mask & 0x4)
	{
		bVal = m_options_toolbar->GetToolState(ID_CamCtrChk);
		fconfig.Write("cam_ctr_chk", bVal);
	}
	//camctr size
	if (mask & 0x8)
	{
		fconfig.Write("camctr_size", m_glview->m_camctr_size);
	}
	//fps
	if (mask & 0x10)
	{
		fconfig.Write("info_chk", m_glview->m_draw_info);
	}
	//selection
	if (mask & 0x20)
	{
		bVal = m_glview->m_draw_legend;
		fconfig.Write("legend_chk", bVal);
	}
	//mouse focus
	if (mask & 0x40)
	{
		bVal = m_glview->m_mouse_focus;
		fconfig.Write("mouse_focus", bVal);
	}
	//ortho/persp
	if (mask & 0x80)
	{
		fconfig.Write("persp", m_glview->m_persp);
		fconfig.Write("aov", m_glview->m_aov);
		bVal = m_options_toolbar->GetToolState(ID_FreeChk);
		fconfig.Write("free_rd", bVal);
	}
	//rotations
	if (mask & 0x100)
	{
		str = m_x_rot_text->GetValue();
		fconfig.Write("x_rot", str);
		str = m_y_rot_text->GetValue();
		fconfig.Write("y_rot", str);
		str = m_z_rot_text->GetValue();
		fconfig.Write("z_rot", str);
		fconfig.Write("rot_lock", m_glview->GetRotLock());
		fconfig.Write("rot_slider", m_rot_slider);
	}
	else
	{
		fconfig.Write("x_rot", m_dft_x_rot);
		fconfig.Write("y_rot", m_dft_y_rot);
		fconfig.Write("z_rot", m_dft_z_rot);
		fconfig.Write("rot_lock", m_glview->GetRotLock());
		fconfig.Write("rot_slider", m_rot_slider);
	}
	//depth atten
	if (mask & 0x200)
	{
		bVal = m_left_toolbar->GetToolState(ID_DepthAttenChk);
		fconfig.Write("depth_atten_chk", bVal);
		str = m_depth_atten_factor_text->GetValue();
		fconfig.Write("depth_atten_factor_text", str);
		str.ToDouble(&m_dft_depth_atten_factor);
	}
	//scale factor
	if (mask & 0x400)
	{
		fconfig.Write("pin_rot_center", m_glview->m_pin_rot_center);
		//str = m_scale_factor_text->GetValue();
		//fconfig.Write("scale_factor_text", str);
		//str.ToDouble(&m_dft_scale_factor);
		m_dft_scale_factor = m_glview->m_scale_factor;
		fconfig.Write("scale_factor", m_dft_scale_factor);
		m_dft_scale_factor_mode = m_glview->m_scale_mode;
		fconfig.Write("scale_factor_mode", m_dft_scale_factor_mode);
	}
	else
	{
		fconfig.Write("scale_factor", m_dft_scale_factor);
		fconfig.Write("scale_factor_mode", m_dft_scale_factor_mode);
	}
	//camera center
	if (mask & 0x800)
	{
		GetCenters(x, y, z);
		str = wxString::Format("%f %f %f", x, y, z);
		fconfig.Write("center", str);
	}
	//colormap
	if (mask & 0x1000)
	{
		bVal = m_glview->m_draw_colormap;
		fconfig.Write("colormap_chk", bVal);
	}
	wxString expath = wxStandardPaths::Get().GetExecutablePath();
	expath = wxPathOnly(expath);
	wxString dft = expath + "/default_view_settings.dft";
	wxFileOutputStream os(dft);
	fconfig.Save(os);

	m_default_saved = true;
}

void VRenderView::OnSaveDefault(wxCommandEvent &event)
{
	SaveDefault();
}

void VRenderView::LoadSettings()
{
	wxString expath = wxStandardPaths::Get().GetExecutablePath();
    expath = wxPathOnly(expath);
    wxString dft = expath + "/default_view_settings.dft";

	wxFileInputStream is(dft);

	if (!is.IsOk()) {
		wxCommandEvent e;
		OnRotSliderType(e);
		UpdateView();
		return;
	}

	wxFileConfig fconfig(is);

	bool bVal;
	double dVal;
	int iVal;
	if (fconfig.Read("volume_seq_rd", &bVal) && bVal)
		SetVolMethod(VOL_METHOD_SEQ);
	if (fconfig.Read("volume_multi_rd", &bVal) && bVal)
		SetVolMethod(VOL_METHOD_MULTI);
	if (fconfig.Read("volume_comp_rd", &bVal) && bVal)
		SetVolMethod(VOL_METHOD_COMP);

	wxString str;
	if (fconfig.Read("bg_color_picker", &str))
	{
		int r, g, b;
		SSCANF(str.c_str(), "%d%d%d", &r, &g, &b);
		wxColor cVal(r, g, b);
		m_bg_color_picker->SetColour(cVal);
		Color c(r/255.0, g/255.0, b/255.0);
		SetBackgroundColor(c);
	}
	if (fconfig.Read("cam_ctr_chk", &bVal))
	{
		m_options_toolbar->ToggleTool(ID_CamCtrChk,bVal);
		m_glview->m_draw_camctr = bVal;
	}
	if (fconfig.Read("camctr_size", &dVal))
	{
		m_glview->m_camctr_size = dVal;
	}
	if (fconfig.Read("info_chk", &iVal))
	{
		m_options_toolbar->ToggleTool(ID_FpsChk, iVal&INFO_DISP);
		m_glview->m_draw_info = iVal;
	}
	if (fconfig.Read("legend_chk", &bVal))
	{
		m_options_toolbar->ToggleTool(ID_LegendChk,bVal);
		m_glview->m_draw_legend = bVal;
	}
	if (fconfig.Read("colormap_chk", &bVal))
	{
		m_options_toolbar->ToggleTool(ID_ColormapChk, bVal);
		m_glview->m_draw_colormap = bVal;
	}
	if (fconfig.Read("mouse_focus", &bVal))
	{
		m_glview->m_mouse_focus = bVal;
	}
	if (fconfig.Read("persp", &bVal))
	{
		if (bVal)
			SetPersp(true);
		else
			SetPersp(false);
	}
	if (fconfig.Read("aov", &dVal))
	{
		SetAov(dVal);
	}
	if (fconfig.Read("free_rd", &bVal))
	{
		m_options_toolbar->ToggleTool(ID_FreeChk,bVal);
		if (bVal)
			SetFree(true);
	}
	if (fconfig.Read("x_rot", &str))
	{
		m_x_rot_text->ChangeValue(str);
		str.ToDouble(&m_dft_x_rot);
	}
	if (fconfig.Read("y_rot", &str))
	{
		m_y_rot_text->ChangeValue(str);
		str.ToDouble(&m_dft_y_rot);
	}
	if (fconfig.Read("z_rot", &str))
	{
		m_z_rot_text->ChangeValue(str);
		str.ToDouble(&m_dft_z_rot);
	}
	if (fconfig.Read("rot_lock", &bVal))
	{
		m_rot_lock_btn->ToggleTool(ID_RotLockChk,bVal);
		if (bVal)
			m_rot_lock_btn->SetToolNormalBitmap(ID_RotLockChk,
				wxGetBitmapFromMemory(gear_45));
		else
			m_rot_lock_btn->SetToolNormalBitmap(ID_RotLockChk,
				wxGetBitmapFromMemory(gear_dark));

		m_glview->SetRotLock(bVal);
	}
	if (fconfig.Read("rot_slider", &m_rot_slider))
	{
		m_rot_lock_btn->ToggleTool(ID_RotSliderType, m_rot_slider);
	}
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
	if (fconfig.Read("pin_rot_center", &bVal))
	{
		m_glview->m_pin_rot_center = bVal;
		if (bVal)
			m_glview->m_rot_center_dirty = true;
		m_pin_btn->ToggleTool(ID_PinBtn, bVal);
		if (bVal)
			m_pin_btn->SetToolNormalBitmap(ID_PinBtn,
				wxGetBitmapFromMemory(pin));
		else
			m_pin_btn->SetToolNormalBitmap(ID_PinBtn,
				wxGetBitmapFromMemory(anchor_dark));
	}
	if (fconfig.Read("scale_factor_mode", &iVal))
	{
		m_dft_scale_factor_mode = iVal;
		SetScaleMode(iVal, false);
	}
	if (fconfig.Read("scale_factor", &dVal))
	{
		m_dft_scale_factor = dVal;
		m_glview->m_scale_factor = m_dft_scale_factor;
		UpdateScaleFactor(false);
	}
	if (fconfig.Read("depth_atten_chk", &bVal))
	{
		//m_left_toolbar->ToggleTool(ID_DepthAttenChk,bVal);
		SetFog(bVal);
		if (bVal)
		{
			m_depth_atten_factor_sldr->Enable();
			m_depth_atten_factor_text->Enable();
		}
		else
		{
			m_depth_atten_factor_sldr->Disable();
			m_depth_atten_factor_text->Disable();
		}
	}
	if (fconfig.Read("depth_atten_factor_text", &str))
	{
		m_depth_atten_factor_text->ChangeValue(str);
		str.ToDouble(&dVal);
		m_depth_atten_factor_sldr->SetValue(int(dVal*100));
		SetFogIntensity(dVal);
		m_dft_depth_atten_factor = dVal;
	}
	if (fconfig.Read("center", &str))
	{
		float x, y, z;
		SSCANF(str.c_str(), "%f%f%f", &x, &y, &z);
		SetCenters(x, y, z);
	}

	m_use_dft_settings = true;
	RefreshGL();
}

void VRenderView::OnKeyDown(wxKeyEvent& event)
{
	event.Skip();
}

