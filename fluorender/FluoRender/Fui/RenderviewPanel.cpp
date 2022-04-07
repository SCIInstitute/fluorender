/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2022 Scientific Computing and Imaging Institute,
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
#include <RenderviewPanel.h>
#include <RenderFrame.h>
#include <RenderCanvas.h>
#include <Global.hpp>
#include <AgentFactory.hpp>
#include <RenderviewFactory.hpp>
#include <VolumeData.hpp>
#include <tiffio.h>
#include <wx/utils.h>
#include <wx/valnum.h>
#include <algorithm>
#include <limits>
#include <png_resource.h>
#include "img/icons.h"
#include <wx/stdpaths.h>

int RenderviewPanel::m_max_id = 1;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(RenderviewPanel, wxPanel)
	//bar top
	EVT_TOOL(ID_VolumeSeqRd, RenderviewPanel::OnVolumeMethodCheck)
	EVT_TOOL(ID_VolumeMultiRd, RenderviewPanel::OnVolumeMethodCheck)
	EVT_TOOL(ID_VolumeCompRd, RenderviewPanel::OnVolumeMethodCheck)
	EVT_BUTTON(ID_CaptureBtn, RenderviewPanel::OnCapture)
	EVT_COLOURPICKER_CHANGED(ID_BgColorPicker, RenderviewPanel::OnBgColorChange)
	EVT_BUTTON(ID_BgInvBtn, RenderviewPanel::OnBgInvBtn)
	EVT_TOOL(ID_CamCtrChk, RenderviewPanel::OnCamCtrCheck)
	EVT_TOOL(ID_FpsChk, RenderviewPanel::OnFpsCheck)
	EVT_TOOL(ID_LegendChk, RenderviewPanel::OnLegendCheck)
	EVT_TOOL(ID_ColormapChk, RenderviewPanel::OnColormapCheck)
	//scale bar
	EVT_TOOL(ID_ScaleBar, RenderviewPanel::OnScaleBar)
	EVT_TEXT(ID_ScaleText, RenderviewPanel::OnScaleTextEditing)
	EVT_COMBOBOX(ID_ScaleCmb, RenderviewPanel::OnScaleTextEditing)
	//other tools
	EVT_COMMAND_SCROLL(ID_AovSldr, RenderviewPanel::OnAovChange)
	EVT_TEXT(ID_AovText, RenderviewPanel::OnAovText)
	EVT_TOOL(ID_FreeChk, RenderviewPanel::OnFreeChk)
	EVT_TOOL(ID_FullScreenBtn, RenderviewPanel::OnFullScreen)
	//bar left
	EVT_TOOL(ID_DepthAttenChk, RenderviewPanel::OnDepthAttenCheck)
	EVT_COMMAND_SCROLL(ID_DepthAttenFactorSldr, RenderviewPanel::OnDepthAttenFactorChange)
	EVT_TEXT(ID_DepthAttenFactorText, RenderviewPanel::OnDepthAttenFactorEdit)
	EVT_TOOL(ID_DepthAttenResetBtn, RenderviewPanel::OnDepthAttenReset)
	//bar right
	EVT_TOOL(ID_PinBtn, RenderviewPanel::OnPin)
	EVT_TOOL(ID_CenterBtn, RenderviewPanel::OnCenter)
	EVT_TOOL(ID_Scale121Btn, RenderviewPanel::OnScale121)
	EVT_COMMAND_SCROLL(ID_ScaleFactorSldr, RenderviewPanel::OnScaleFactorChange)
	EVT_TEXT(ID_ScaleFactorText, RenderviewPanel::OnScaleFactorEdit)
	EVT_TOOL(ID_ScaleModeBtn, RenderviewPanel::OnScaleMode)
	EVT_TOOL(ID_ScaleResetBtn, RenderviewPanel::OnScaleReset)
	EVT_SPIN_UP(ID_ScaleFactorSpin, RenderviewPanel::OnScaleFactorSpinDown)
	EVT_SPIN_DOWN(ID_ScaleFactorSpin, RenderviewPanel::OnScaleFactorSpinUp)
	//bar bottom
	EVT_TOOL(ID_ZeroRotBtn, RenderviewPanel::OnZeroRot)
	EVT_TOOL(ID_RotResetBtn, RenderviewPanel::OnRotReset)
	EVT_TEXT(ID_XRotText, RenderviewPanel::OnValueEdit)
	EVT_TEXT(ID_YRotText, RenderviewPanel::OnValueEdit)
	EVT_TEXT(ID_ZRotText, RenderviewPanel::OnValueEdit)
	EVT_COMMAND_SCROLL(ID_XRotSldr, RenderviewPanel::OnXRotScroll)
	EVT_COMMAND_SCROLL(ID_YRotSldr, RenderviewPanel::OnYRotScroll)
	EVT_COMMAND_SCROLL(ID_ZRotSldr, RenderviewPanel::OnZRotScroll)
	EVT_TOOL(ID_RotLockChk, RenderviewPanel::OnRotLockCheck)
	EVT_TOOL(ID_RotSliderType, RenderviewPanel::OnRotSliderType)
	EVT_COMBOBOX(ID_OrthoViewCmb, RenderviewPanel::OnOrthoViewSelected)
	//timer
	EVT_TIMER(ID_RotateTimer, RenderviewPanel::OnTimer)
	//save default
	EVT_TOOL(ID_DefaultBtn, RenderviewPanel::OnSaveDefault)

	EVT_KEY_DOWN(RenderviewPanel::OnKeyDown)
END_EVENT_TABLE()

RenderviewPanel::RenderviewPanel(RenderFrame* frame,
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
	m_rot_slider(true),
	m_mouse_focus(false),
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

	m_id = m_max_id;
	m_max_id++;
	wxString name = wxString::Format("Render View:%d", m_max_id);
	SetName(name);
	std::string str_name = gstRenderviewAgent + std::to_string(m_max_id);
	m_agent = glbin_agtf->addRenderviewAgent(str_name, *this);

	wxLogNull logNo;
	//full frame
	m_full_frame = new wxFrame((wxFrame*)NULL, wxID_ANY, "FluoRender");
	m_view_sizer = new wxBoxSizer(wxVERTICAL);

	// this list takes care of both pixel and context attributes (no custom edits of wx is preferred)
	//render view/////////////////////////////////////////////////
	long red_bit = 8;
	long green_bit = 8;
	long blue_bit = 8;
	long alpha_bit = 8;
	long depth_bit = 24;
	long samples = 0;
	long gl_major_ver = 4;
	long gl_minor_ver = 4;
	long gl_profile_mask = 1;
	long api_type = 0;
	glbin_root->getValue(gstApiType, api_type);
	glbin_root->getValue(gstOutputBitR, red_bit);
	glbin_root->getValue(gstOutputBitG, green_bit);
	glbin_root->getValue(gstOutputBitB, blue_bit);
	glbin_root->getValue(gstOutputBitA, alpha_bit);
	glbin_root->getValue(gstOutputBitD, depth_bit);
	glbin_root->getValue(gstOutputSamples, samples);
	glbin_root->getValue(gstGlVersionMajor, gl_major_ver);
	glbin_root->getValue(gstGlVersionMinor, gl_minor_ver);
	glbin_root->getValue(gstGlProfileMask, gl_profile_mask);

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
	m_canvas = new RenderCanvas(frame, this, attriblist, sharedContext);
	m_agent->setObject(glbin_revf->findFirst(name.ToStdString()));
	//m_agent = m_canvas->m_agent;
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
			//if (vr_frame && vr_frame->GetSettingDlg())
			//{
			//	//could be that color bits are incorrectly set
			//	vr_frame->GetSettingDlg()->SetRedBit(8);
			//	vr_frame->GetSettingDlg()->SetGreenBit(8);
			//	vr_frame->GetSettingDlg()->SetBlueBit(8);
			//	vr_frame->GetSettingDlg()->SetAlphaBit(8);
			//	vr_frame->GetSettingDlg()->SaveSettings();
			//}
		}
		if (sharedContext)
		{
			//sharedContext->SetCurrent(*m_canvas);
			m_canvas->SetCurrent(*sharedContext);
			m_canvas->m_glRC = sharedContext;
			glbin_agtf->getRenderCanvasAgent(GetName().ToStdString())->chgValue(gstSetGl, true);
			//m_canvas->m_set_gl = true;
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
	//glGetIntegerv(GL_MAJOR_VERSION, &gl_major_ver);
	//glGetIntegerv(GL_MINOR_VERSION, &gl_minor_ver);
	//if (vr_frame && vr_frame->GetSettingDlg())
	//{
	//	vr_frame->GetSettingDlg()->SetGLMajorVer(gl_major_ver);
	//	vr_frame->GetSettingDlg()->SetGLMinorVer(gl_minor_ver);
	//}

	CreateBar();
	//if (m_canvas)
	//{
	//	m_canvas->SetSBText(L"50 \u03BCm");
	//	m_canvas->SetScaleBarLen(1.);
	//}
	LoadSettings();
	m_x_rotating = m_y_rotating = m_z_rotating = false;
	m_skip_thumb = false;

	m_agent->getObject()->InitView(
		fluo::Renderview::INIT_BOUNDS |
		fluo::Renderview::INIT_CENTER |
		fluo::Renderview::INIT_TRANSL |
		fluo::Renderview::INIT_ROTATE);
	UpdateView();
}

RenderviewPanel::~RenderviewPanel()
{
	if (m_full_frame)
		delete m_full_frame;
}

void RenderviewPanel::CreateBar()
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
	long lval;
	m_agent->getValue(gstMixMethod, lval);
	switch (lval)
	{
	case fluo::Renderview::MIX_METHOD_SEQ:
		m_options_toolbar->ToggleTool(ID_VolumeSeqRd,true);
		break;
	case fluo::Renderview::MIX_METHOD_MULTI:
		m_options_toolbar->ToggleTool(ID_VolumeMultiRd,true);
		break;
	case fluo::Renderview::MIX_METHOD_COMP:
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
	bool persp;
	m_agent->getValue(gstPerspective, persp);
	double aov;
	m_agent->getValue(gstAov, aov);
	m_aov_sldr->SetValue(persp ? aov : 10);
	m_aov_sldr->Connect(wxID_ANY, wxEVT_IDLE,
		wxIdleEventHandler(RenderviewPanel::OnAovSldrIdle),
		NULL, this);
	m_aov_text = new wxTextCtrl(m_options_toolbar, ID_AovText, "",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	m_aov_text->ChangeValue(persp ? wxString::Format("%d", int(aov)) : "Ortho");
	m_options_toolbar->AddControl(st2);
	m_options_toolbar->AddControl(m_aov_sldr);
	m_options_toolbar->AddControl(m_aov_text);

	bitmap = wxGetBitmapFromMemory(freefly);
	m_options_toolbar->AddCheckTool(
		ID_FreeChk, "Free Fly",
		bitmap, wxNullBitmap,
		"Change the camera to a 'Free-Fly' Mode",
		"Change the camera to a 'Free-Fly' Mode");

	bool bval;
	m_agent->getValue(gstFree, bval);
	if (bval)
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

void RenderviewPanel::UpdateView(bool ui_update)
{
	double rotx, roty, rotz;
	wxString str_val = m_x_rot_text->GetValue();
	rotx = STOD(str_val.fn_str());
	str_val = m_y_rot_text->GetValue();
	roty = STOD(str_val.fn_str());
	str_val = m_z_rot_text->GetValue();
	rotz = STOD(str_val.fn_str());
	m_agent->updValue(gstCamRotX, rotx);
	m_agent->updValue(gstCamRotY, roty);
	m_agent->updValue(gstCamRotZ, rotz);
	//RefreshGL(true);
}

void RenderviewPanel::UpdateScaleFactor(bool update_text)
{
	double dval;
	double scale;
	m_agent->getValue(gstScaleFactor, scale);
	long scale_mode;
	m_agent->getValue(gstScaleMode, scale_mode);
	switch (scale_mode)
	{
	case 0:
		break;
	case 1:
		m_agent->getValue(gstScaleFactor121, dval);
		scale /= dval;
		break;
	case 2:
		{
		fluo::VolumeData *vd = m_agent->getObject()->GetCurrentVolume();
		if (vd)
		{
			double spcx;
			vd->getValue(gstSpcX, spcx);
			m_agent->getValue(gstScaleFactor121, dval);
			if (spcx > 0.0)
				scale /= dval * spcx;
		}
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
	bool auto_pin, pin_rc;
	m_agent->getValue(gstAutoPinRotCtr, auto_pin);
	m_agent->getValue(gstPinRotCtr, pin_rc);
	if (auto_pin)
	{
		m_agent->updValue(gstPinThresh, dval);
		if (scale > dval)
		{
			if (!pin_rc)
			{
				m_pin_btn->ToggleTool(ID_PinBtn, true);
				m_pin_btn->SetToolNormalBitmap(ID_PinBtn,
					wxGetBitmapFromMemory(pin));
				m_agent->updValue(gstPinRotCtr, true);
				m_agent->updValue(gstRotCtrDirty, true);
			}
		}
		else
		{
			if (pin_rc)
			{
				m_pin_btn->ToggleTool(ID_PinBtn, false);
				m_pin_btn->SetToolNormalBitmap(ID_PinBtn,
					wxGetBitmapFromMemory(anchor_dark));
				m_agent->updValue(gstPinRotCtr, false);
			}
		}
	}
}

void RenderviewPanel::SetScaleFactor(double s, bool update)
{
	double dval;
	long lval;
	m_agent->getValue(gstScaleMode, lval);
	switch (lval)
	{
	case 0:
		m_agent->updValue(gstScaleFactor, s);
		break;
	case 1:
		m_agent->getValue(gstScaleFactor121, dval);
		m_agent->updValue(gstScaleFactor, s * dval);
		break;
	case 2:
		{
		fluo::VolumeData *vd = m_agent->getObject()->GetCurrentVolume();
		if (vd)
		{
			double spcx;
			vd->getValue(gstSpcX, spcx);
			if (spcx > 0.0)
			{
				m_agent->getValue(gstScaleFactor121, dval);
				m_agent->updValue(gstScaleFactor, s * dval * spcx);
			}
		}
		}
		break;
	}

	if (update)
		UpdateScaleFactor();
}

void RenderviewPanel::SetScaleMode(int mode, bool update)
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
	m_agent->updValue(gstScaleMode, long(mode));
	if (update)
		UpdateScaleFactor();
}

//rot center anchor thresh
void RenderviewPanel::SetPinThreshold(double value)
{
	m_agent->updValue(gstPinThresh, value);
}

void RenderviewPanel::UpdateOrientCmb(int index)
{
	if (!m_ortho_view_cmb->HasFocus())
		m_ortho_view_cmb->Select(index);
}

//reset counter
void RenderviewPanel::ResetID()
{
	m_max_id = 1;
}

//get rendering context
wxGLContext* RenderviewPanel::GetContext()
{
	if (m_canvas)
		return m_canvas->m_glRC/*GetContext()*/;
	else
		return 0;
}

//bar top
void RenderviewPanel::OnVolumeMethodCheck(wxCommandEvent& event)
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
		m_agent->updValue(gstMixMethod, long(fluo::Renderview::MIX_METHOD_SEQ));
		break;
	case ID_VolumeMultiRd:
		m_agent->updValue(gstMixMethod, long(fluo::Renderview::MIX_METHOD_MULTI));
		break;
	case ID_VolumeCompRd:
		m_agent->updValue(gstMixMethod, long(fluo::Renderview::MIX_METHOD_COMP));
		break;
	}

	/*   int new_mode = GetVolMethod();

	if (new_mode == VOL_METHOD_MULTI &&
	(old_mode == VOL_METHOD_SEQ || old_mode == VOL_METHOD_COMP))
	mode_switch_type = 1;
	else if ((new_mode == VOL_METHOD_SEQ || new_mode == VOL_METHOD_COMP) &&
	old_mode == VOL_METHOD_MULTI)
	mode_switch_type = 2;

	RenderFrame* vr_frame = (RenderFrame*)m_frame;
	if (vr_frame)
	{
	if (mode_switch_type == 1)
	{
	int cnt = 0;
	bool r = false;
	bool g = false;
	bool b = false;
	Color gamma = m_canvas->GetGamma();
	Color brightness = m_canvas->GetBrightness();
	Color hdr = m_canvas->GetHdr();
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
	m_canvas->SetGamma(gamma);
	m_canvas->SetBrightness(brightness);
	m_canvas->SetHdr(hdr);
	m_canvas->SetSyncR(r);
	m_canvas->SetSyncG(g);
	m_canvas->SetSyncB(b);

	//sync properties of the selcted volume
	fluo::VolumeData* svd = vr_frame->GetCurSelVol();
	if (!svd)
	svd = GetAllVolumeData(0);
	if (svd)
	{
	for (int i=0; i<GetAllVolumeNum(); i++)
	{
	fluo::VolumeData* vd = GetAllVolumeData(i);
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
	flvr::Color col = m_canvas->GetGamma();
	group->SetGammaAll(col);
	col = m_canvas->GetBrightness();
	group->SetBrightnessAll(col);
	col = m_canvas->GetHdr();
	group->SetHdrAll(col);
	group->SetSyncRAll(m_canvas->GetSyncR());
	group->SetSyncGAll(m_canvas->GetSyncG());
	group->SetSyncBAll(m_canvas->GetSyncB());
	break;
	}
	}
	}
	}

	vr_frame->GetTree()->UpdateSelection();
	}*/

	//RefreshGL();
}

//ch1
void RenderviewPanel::OnCh1Check(wxCommandEvent &event)
{
	wxCheckBox* ch1 = (wxCheckBox*)event.GetEventObject();
	if (ch1)
	{
		bool bval = ch1->GetValue();
		glbin_root->setValue(gstCaptureCompress, bval);
	}
}

//save alpha
void RenderviewPanel::OnChAlphaCheck(wxCommandEvent &event)
{
	wxCheckBox* ch_alpha = (wxCheckBox*)event.GetEventObject();
	if (ch_alpha)
	{
		bool bval = ch_alpha->GetValue();
		glbin_root->setValue(gstCaptureAlpha, bval);
	}
}

//save float
void RenderviewPanel::OnChFloatCheck(wxCommandEvent &event)
{
	wxCheckBox* ch_float = (wxCheckBox*)event.GetEventObject();
	if (ch_float)
	{
		bool bval = ch_float->GetValue();
		glbin_root->setValue(gstCaptureFloat, bval);
	}
}

//embde project
void RenderviewPanel::OnChEmbedCheck(wxCommandEvent &event)
{
	wxCheckBox* ch_embed = (wxCheckBox*)event.GetEventObject();
	if (ch_embed)
	{
		bool bval = ch_embed->GetValue();
		glbin_root->setValue(gstEmbedDataInProject, bval);
	}
}

//enlarge output image
void RenderviewPanel::OnChEnlargeCheck(wxCommandEvent &event)
{
	wxCheckBox* ch_enlarge = (wxCheckBox*)event.GetEventObject();
	if (ch_enlarge)
	{
		bool enlarge = ch_enlarge->GetValue();
		m_agent->updValue(gstEnlarge, enlarge);
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

void RenderviewPanel::OnSlEnlargeScroll(wxScrollEvent &event)
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

void RenderviewPanel::OnTxEnlargeText(wxCommandEvent &event)
{
	wxString str = event.GetString();
	double dval;
	str.ToDouble(&dval);
	m_agent->updValue(gstEnlargeScale, dval);
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


wxWindow* RenderviewPanel::CreateExtraCaptureControl(wxWindow* parent)
{
	bool bval;
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
		wxCommandEventHandler(RenderviewPanel::OnCh1Check), NULL, panel);
	glbin_root->getValue(gstCaptureCompress, bval);
	ch1->SetValue(bval);

	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	//save alpha
	wxCheckBox* ch_alpha = new wxCheckBox(panel, ID_SAVE_ALPHA,
		"Save alpha channel");
	ch_alpha->Connect(ch_alpha->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(RenderviewPanel::OnChAlphaCheck), NULL, panel);
	glbin_root->getValue(gstCaptureAlpha, bval);
	ch_alpha->SetValue(bval);
	//save float
	wxCheckBox* ch_float = new wxCheckBox(panel, ID_SAVE_FLOAT,
		"Save float channel");
	ch_float->Connect(ch_float->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(RenderviewPanel::OnChFloatCheck), NULL, panel);
	glbin_root->getValue(gstCaptureFloat, bval);
	ch_float->SetValue(bval);
	sizer_1->Add(ch_alpha, 0, wxALIGN_CENTER);
	sizer_1->Add(10, 10);
	sizer_1->Add(ch_float, 0, wxALIGN_CENTER);

	//enlarge
	wxBoxSizer* sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	wxCheckBox* ch_enlarge = new wxCheckBox(panel, ID_ENLARGE_CHK,
		"Enlarge output image");
	ch_enlarge->Connect(ch_enlarge->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(RenderviewPanel::OnChEnlargeCheck), NULL, panel);
	wxSlider* sl_enlarge = new wxSlider(panel, ID_ENLARGE_SLDR,
		10, 10, 100);
	sl_enlarge->Connect(sl_enlarge->GetId(), wxEVT_COMMAND_SLIDER_UPDATED,
		wxScrollEventHandler(RenderviewPanel::OnSlEnlargeScroll), NULL, panel);
	sl_enlarge->Disable();
	wxFloatingPointValidator<double> vald_fp(1);
	wxTextCtrl* tx_enlarge = new wxTextCtrl(panel, ID_ENLARGE_TEXT,
		"1.0", wxDefaultPosition, wxSize(60, 23), 0, vald_fp);
	tx_enlarge->Connect(tx_enlarge->GetId(), wxEVT_COMMAND_TEXT_UPDATED,
		wxCommandEventHandler(RenderviewPanel::OnTxEnlargeText), NULL, panel);
	tx_enlarge->Disable();
	sizer_2->Add(ch_enlarge, 0, wxALIGN_CENTER);
	sizer_2->Add(10, 10);
	sizer_2->Add(sl_enlarge, 1, wxEXPAND);
	sizer_2->Add(10, 10);
	sizer_2->Add(tx_enlarge, 0, wxALIGN_CENTER);

	//copy all files check box
	wxCheckBox* ch_embed = 0;
	bool save_project;
	glbin_root->getValue(gstSaveProjectEnable, save_project);
	if (save_project)
	{
		ch_embed = new wxCheckBox(panel, ID_EMBED_FILES,
			"Embed all files in the project folder");
		ch_embed->Connect(ch_embed->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
			wxCommandEventHandler(RenderviewPanel::OnChEmbedCheck), NULL, panel);
		glbin_root->getValue(gstEmbedDataInProject, bval);
		ch_embed->SetValue(bval);
	}

	//group
	group1->Add(10, 10);
	group1->Add(ch1);
	group1->Add(10, 10);
	group1->Add(sizer_1);
	group1->Add(10, 10);
	group1->Add(sizer_2);
	if (save_project && ch_embed)
	{
		group1->Add(10, 10);
		group1->Add(ch_embed);
	}
	group1->Add(10, 20);

	panel->SetSizerAndFit(group1);
	panel->Layout();

	return panel;
}

void RenderviewPanel::OnCapture(wxCommandEvent& event)
{
	//reset enlargement
	m_agent->updValue(gstEnlarge, false);
	m_agent->updValue(gstEnlargeScale, 1.0);

	//RenderFrame* vr_frame = (RenderFrame*)m_frame;
	//if (vr_frame && vr_frame->GetSettingDlg())
	//{
	//	RenderFrame::SetSaveProject(vr_frame->GetSettingDlg()->GetProjSave());
	//	RenderFrame::SetSaveAlpha(vr_frame->GetSettingDlg()->GetSaveAlpha());
	//	RenderFrame::SetSaveFloat(vr_frame->GetSettingDlg()->GetSaveFloat());
	//}

	wxFileDialog file_dlg(m_frame, "Save captured image", "", "", "*.tif", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
	file_dlg.SetExtraControlCreator(CreateExtraCaptureControl);
	int rval = file_dlg.ShowModal();
	if (rval == wxID_OK)
	{
		wxString cap_file = file_dlg.GetDirectory() + GETSLASH() + file_dlg.GetFilename();
		m_agent->updValue(gstCaptureFile, cap_file.ToStdWstring());
		m_agent->updValue(gstCapture, true);
		//RefreshGL();

		bool bval;
		glbin_root->getValue(gstSaveProjectEnable, bval);
		if (bval)
		{
			wxString new_folder;
			new_folder = cap_file + "_project";
			MkDirW(new_folder.ToStdWstring());
			wxString prop_file = new_folder + GETSLASH() + file_dlg.GetFilename() + "_project.vrp";
			glbin_agtf->getRenderFrameAgent()->SaveProject(prop_file.ToStdWstring());
		}
			//vr_frame->GetSettingDlg()->SetSaveAlpha(RenderFrame::GetSaveAlpha());
			//vr_frame->GetSettingDlg()->SetSaveFloat(RenderFrame::GetSaveFloat());
	}
}


//bar left
void RenderviewPanel::OnDepthAttenCheck(wxCommandEvent& event)
{
	if (m_left_toolbar->GetToolState(ID_DepthAttenChk))
	{
		m_agent->updValue(gstDepthAtten, true);
		m_depth_atten_factor_sldr->Enable();
		m_depth_atten_factor_text->Enable();
		m_left_toolbar->SetToolNormalBitmap (ID_DepthAttenChk,
			wxGetBitmapFromMemory(depth_atten));
	}
	else
	{
		m_agent->updValue(gstDepthAtten, false);
		m_depth_atten_factor_sldr->Disable();
		m_depth_atten_factor_text->Disable();
		m_left_toolbar->SetToolNormalBitmap (ID_DepthAttenChk,
			wxGetBitmapFromMemory(no_depth_atten));
	}

	//RefreshGL();
}

void RenderviewPanel::OnDepthAttenFactorChange(wxScrollEvent& event)
{
	double atten_factor = m_depth_atten_factor_sldr->GetValue()/100.0;
	wxString str = wxString::Format("%.2f", atten_factor);
	if (str != m_depth_atten_factor_text->GetValue())
		m_depth_atten_factor_text->SetValue(str);
}

void RenderviewPanel::OnDepthAttenFactorEdit(wxCommandEvent& event)
{
	wxString str = m_depth_atten_factor_text->GetValue();
	double val;
	str.ToDouble(&val);
	m_agent->updValue(gstDaInt, val);
	m_depth_atten_factor_sldr->SetValue(int(val*100.0));
	//RefreshGL(true);
}

void RenderviewPanel::OnDepthAttenReset(wxCommandEvent &event)
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
void RenderviewPanel::OnPin(wxCommandEvent &event)
{
	double dval;
	bool pin = m_pin_btn->GetToolState(ID_PinBtn);
	m_agent->updValue(gstPinRotCtr, pin);
	double scale;
	m_agent->getValue(gstScaleFactor, scale);
	long lval;
	m_agent->getValue(gstScaleMode, lval);
	switch (lval)
	{
	case 0:
		break;
	case 1:
		m_agent->getValue(gstScaleFactor121, dval);
		scale /= dval;
		break;
	case 2:
		{
			fluo::VolumeData *vd = m_agent->getObject()->GetCurrentVolume();
			if (vd)
			{
				double spcx;
				vd->getValue(gstSpcX, spcx);
				m_agent->getValue(gstScaleFactor121, dval);
				if (spcx > 0.0)
					scale /= dval * spcx;
			}
		}
		break;
	}
	m_agent->getValue(gstPinThresh, dval);
	if (pin)
	{
		m_pin_btn->SetToolNormalBitmap(ID_PinBtn,
			wxGetBitmapFromMemory(pin));
		if (scale > dval)
			m_agent->updValue(gstAutoPinRotCtr, true);
		else
			m_agent->updValue(gstAutoPinRotCtr, false);
	}
	else
	{
		m_pin_btn->SetToolNormalBitmap(ID_PinBtn,
			wxGetBitmapFromMemory(anchor_dark));
		if (scale > dval)
			m_agent->updValue(gstAutoPinRotCtr, false);
		else
			m_agent->updValue(gstAutoPinRotCtr, true);
	}
}

void RenderviewPanel::OnCenter(wxCommandEvent &event)
{
	m_agent->getObject()->SetCenter();
	//RefreshGL();
}

void RenderviewPanel::OnScale121(wxCommandEvent &event)
{
	double dval;
	m_agent->getValue(gstScaleFactor121, dval);
	wxString str = wxString::Format("%.0f", dval*100.0);
	m_scale_factor_sldr->SetValue(dval * 100);
	m_scale_factor_text->ChangeValue(str);
	//RefreshGL();
	if (m_mouse_focus)
		m_canvas->SetFocus();
}

void RenderviewPanel::OnScaleFactorChange(wxScrollEvent& event)
{
	int scale_factor = m_scale_factor_sldr->GetValue();
	wxString str = wxString::Format("%d", scale_factor);
	if (str != m_scale_factor_text->GetValue())
		m_scale_factor_text->SetValue(str);
}

void RenderviewPanel::OnScaleFactorEdit(wxCommandEvent& event)
{
	wxString str = m_scale_factor_text->GetValue();
	long val;
	str.ToLong(&val);
	if (val>0)
	{
		m_scale_factor_sldr->SetValue(val);
		SetScaleFactor(val/100.0, false);
		//m_canvas->SetSortBricks();
		//RefreshGL(true);
	}
}

void RenderviewPanel::OnScaleMode(wxCommandEvent& event)
{
	long mode;
	m_agent->getValue(gstScaleMode, mode);
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
	m_agent->updValue(gstScaleMode, mode);
	UpdateScaleFactor(false);
}

void RenderviewPanel::OnScaleFactorSpinUp(wxSpinEvent& event)
{
	wxString str_val = m_scale_factor_text->GetValue();
	long val;
	str_val.ToLong(&val);
	val++;
	str_val = wxString::Format("%d", val);
	m_scale_factor_text->SetValue(str_val);
}

void RenderviewPanel::OnScaleFactorSpinDown(wxSpinEvent& event)
{
	wxString str_val = m_scale_factor_text->GetValue();
	long val;
	str_val.ToLong(&val);
	val--;
	str_val = wxString::Format("%d", val);
	m_scale_factor_text->SetValue(str_val);
}

void RenderviewPanel::OnScaleReset(wxCommandEvent &event)
{
	m_agent->resetValue(gstScaleFactor);
	UpdateScaleFactor();
	if (m_canvas && m_mouse_focus)
		m_canvas->SetFocus();
}

//bar bottom
void RenderviewPanel::OnValueEdit(wxCommandEvent& event)
{
	UpdateView(false);
	if (!m_rot_slider)
	{
		double rotx, roty, rotz;
		m_agent->getValue(gstCamRotX, rotx);
		m_agent->getValue(gstCamRotY, roty);
		m_agent->getValue(gstCamRotZ, rotz);
		m_x_rot_sldr->SetThumbPosition(int(rotx + 0.5));
		m_y_rot_sldr->SetThumbPosition(int(roty + 0.5));
		m_z_rot_sldr->SetThumbPosition(int(rotz + 0.5));
	}
}

/*void RenderviewPanel::OnRotLink(bool b)
{
	m_canvas->m_linked_rot = true;
	double rotx, roty, rotz;
	m_canvas->GetRotations(rotx, roty, rotz);
	RenderFrame* vr_frame = (RenderFrame*)m_frame;
	if (vr_frame)
	{
		for (int i=0; i<vr_frame->GetViewNum(); i++)
		{
			RenderviewPanel* view = vr_frame->GetView(i);
			if (view)
			{
				view->m_canvas->m_linked_rot = b;
				view->m_canvas->SetRotations(rotx, roty, rotz);
				view->m_canvas->RefreshGL();
			}
		}
	}
}*/

void RenderviewPanel::OnZeroRot(wxCommandEvent& event)
{
	double rotx, roty, rotz;
	m_agent->getValue(gstCamRotX, rotx);
	m_agent->getValue(gstCamRotY, roty);
	m_agent->getValue(gstCamRotZ, rotz);
	if (rotx == 0.0 &&
		roty == 0.0 &&
		rotz == 0.0)
	{
		//reset
		m_agent->updValue(gstCamRotZeroQ, fluo::Quaternion());
		fluo::Quaternion q;
		m_agent->getValue(gstCamRotQ, q);
		q.ToEuler(rotx, roty, rotz);
		m_agent->updValue(gstCamRotX, rotx);
		m_agent->updValue(gstCamRotY, roty);
		m_agent->updValue(gstCamRotZ, rotz);
	}
	else
	{
		fluo::Quaternion q;
		m_agent->getValue(gstCamRotQ, q);
		m_agent->updValue(gstCamRotZeroQ, q);
		OnRotReset(event);
	}
}

void RenderviewPanel::OnRotReset(wxCommandEvent &event)
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
	m_agent->updValue(gstCamRotX, double(0));
	m_agent->updValue(gstCamRotY, double(0));
	m_agent->updValue(gstCamRotZ, double(0));
	//RefreshGL(true);
	if (m_mouse_focus)
		m_canvas->SetFocus();
}

//timer used for rotation scrollbars
void RenderviewPanel::OnTimer(wxTimerEvent& event)
{
	wxString str;
	bool lock = m_rot_lock_btn->GetToolState(ID_RotLockChk);
	double rotx, roty, rotz;
	m_agent->getValue(gstCamRotX, rotx);
	m_agent->getValue(gstCamRotY, roty);
	m_agent->getValue(gstCamRotZ, rotz);
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

void RenderviewPanel::OnXRotScroll(wxScrollEvent& event)
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
		m_agent->getValue(gstCamRotX, rotx);
		m_agent->getValue(gstCamRotY, roty);
		m_agent->getValue(gstCamRotZ, rotz);
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

void RenderviewPanel::OnYRotScroll(wxScrollEvent& event)
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
		m_agent->getValue(gstCamRotX, rotx);
		m_agent->getValue(gstCamRotY, roty);
		m_agent->getValue(gstCamRotZ, rotz);
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

void RenderviewPanel::OnZRotScroll(wxScrollEvent& event)
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
		m_agent->getValue(gstCamRotX, rotx);
		m_agent->getValue(gstCamRotY, roty);
		m_agent->getValue(gstCamRotZ, rotz);
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

void RenderviewPanel::OnRotLockCheck(wxCommandEvent& event)
{
	bool lock = m_rot_lock_btn->GetToolState(ID_RotLockChk);
	double rotx, roty, rotz;
	m_agent->getValue(gstCamRotX, rotx);
	m_agent->getValue(gstCamRotY, roty);
	m_agent->getValue(gstCamRotZ, rotz);
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
	m_agent->updValue(gstGearedEnable, lock);
	wxString str = wxString::Format("%.1f", rotx);
	m_x_rot_text->SetValue(str);
	str = wxString::Format("%.1f", roty);
	m_y_rot_text->SetValue(str);
	str = wxString::Format("%.1f", rotz);
	m_z_rot_text->SetValue(str);
}

void RenderviewPanel::OnRotSliderType(wxCommandEvent& event)
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
		m_agent->getValue(gstCamRotX, rotx);
		m_agent->getValue(gstCamRotY, roty);
		m_agent->getValue(gstCamRotZ, rotz);
		m_x_rot_sldr->SetThumbPosition(int(rotx));
		m_y_rot_sldr->SetThumbPosition(int(roty));
		m_z_rot_sldr->SetThumbPosition(int(rotz));
	}
}

void RenderviewPanel::OnOrthoViewSelected(wxCommandEvent& event)
{
	int sel = 6;
	if (m_ortho_view_cmb)
		sel = m_ortho_view_cmb->GetSelection();
	switch (sel)
	{
	case 0://+X
		m_agent->updValue(gstCamRotX, double(0));
		m_agent->updValue(gstCamRotY, double(90));
		m_agent->updValue(gstCamRotZ, double(0));
		break;
	case 1://-X
		m_agent->updValue(gstCamRotX, double(0));
		m_agent->updValue(gstCamRotY, double(270));
		m_agent->updValue(gstCamRotZ, double(0));
		break;
	case 2://+Y
		m_agent->updValue(gstCamRotX, double(90));
		m_agent->updValue(gstCamRotY, double(0));
		m_agent->updValue(gstCamRotZ, double(0));
		break;
	case 3://-Y
		m_agent->updValue(gstCamRotX, double(270));
		m_agent->updValue(gstCamRotY, double(0));
		m_agent->updValue(gstCamRotZ, double(0));
		break;
	case 4://+Z
		m_agent->updValue(gstCamRotX, double(0));
		m_agent->updValue(gstCamRotY, double(0));
		m_agent->updValue(gstCamRotZ, double(0));
		break;
	case 5:
		m_agent->updValue(gstCamRotX, double(0));
		m_agent->updValue(gstCamRotY, double(180));
		m_agent->updValue(gstCamRotZ, double(0));
		break;
	}
	if (sel < 6)
	{
		m_rot_lock_btn->ToggleTool(ID_RotLockChk, true);
		m_rot_lock_btn->SetToolNormalBitmap(ID_RotLockChk,
			wxGetBitmapFromMemory(gear_45));
		m_agent->updValue(gstGearedEnable, true);
	}
	else
	{
		m_rot_lock_btn->ToggleTool(ID_RotLockChk, false);
		m_rot_lock_btn->SetToolNormalBitmap(ID_RotLockChk,
			wxGetBitmapFromMemory(gear_dark));
		m_agent->updValue(gstGearedEnable, false);
	}
	//RefreshGL();
}

//top
void RenderviewPanel::OnBgColorChange(wxColourPickerEvent& event)
{
	wxColor c = event.GetColour();
	fluo::Color color(c.Red()/255.0, c.Green()/255.0, c.Blue()/255.0);
	m_agent->updValue(gstBgColor, color);
	//RefreshGL();
}

void RenderviewPanel::OnBgInvBtn(wxCommandEvent& event)
{
	fluo::Color c;
	m_agent->getValue(gstBgColor, c);
	c = fluo::Color(1.0, 1.0, 1.0) - c;
	m_agent->updValue(gstBgColor, c);
	//RefreshGL();
}

void RenderviewPanel::OnCamCtrCheck(wxCommandEvent& event)
{
	bool bval = m_options_toolbar->GetToolState(ID_CamCtrChk);
	m_agent->updValue(gstDrawCamCtr, bval);
	//RefreshGL();
}

void RenderviewPanel::OnFpsCheck(wxCommandEvent& event)
{
	long lval;
	m_agent->getValue(gstDrawInfo, lval);
	if (m_options_toolbar->GetToolState(ID_FpsChk))
		lval |= 1;
	else
		lval &= ~1;
	m_agent->updValue(gstDrawInfo, lval);
	//RefreshGL();
}

void RenderviewPanel::OnLegendCheck(wxCommandEvent& event)
{
	bool bval = m_options_toolbar->GetToolState(ID_LegendChk);
	m_agent->updValue(gstDrawLegend, bval);
	//RefreshGL();
}

void RenderviewPanel::OnColormapCheck(wxCommandEvent& event)
{
	bool bval = m_options_toolbar->GetToolState(ID_ColormapChk);
	m_agent->updValue(gstDrawColormap, bval);
	//RefreshGL();
}

void RenderviewPanel::OnScaleTextEditing(wxCommandEvent& event)
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
	long lval = m_scale_cmb->GetSelection();
	m_agent->updValue(gstScaleBarText, str.ToStdWstring());
	m_agent->updValue(gstScaleBarLen, len);
	m_agent->updValue(gstScaleBarNum, num_text.ToStdWstring());
	m_agent->updValue(gstScaleBarUnit, lval);
	//RefreshGL();
}

void RenderviewPanel::OnScaleUnitSelected(wxCommandEvent& event) {
}

void RenderviewPanel::OnScaleBar(wxCommandEvent& event)
{
	switch (m_draw_scalebar)
	{
	case kOff:
		m_draw_scalebar = kOn;
		m_agent->updValue(gstDrawScaleBar, true);
		m_agent->updValue(gstDrawScaleBarText, false);
		m_options_toolbar->SetToolNormalBitmap(ID_ScaleBar,
			wxGetBitmapFromMemory(scale_text_off));
		m_scale_text->Enable();
		m_scale_cmb->Disable();
		break;
	case kOn:
		m_draw_scalebar = kText;
		m_agent->updValue(gstDrawScaleBar, true);
		m_agent->updValue(gstDrawScaleBarText, true);
		m_options_toolbar->SetToolNormalBitmap(ID_ScaleBar,
			wxGetBitmapFromMemory(scale_text));
		m_scale_text->Enable();
		m_scale_cmb->Enable();
		break;
	case kText:
		m_draw_scalebar = kOff;
		m_agent->updValue(gstDrawScaleBar, false);
		m_agent->updValue(gstDrawScaleBarText, false);
		m_options_toolbar->SetToolNormalBitmap(ID_ScaleBar,
			wxGetBitmapFromMemory(scalebar));
		m_scale_text->Disable();
		m_scale_cmb->Disable();
		break;
	default:
		break;
	}
	OnScaleTextEditing(event);
	//RefreshGL();
}

void RenderviewPanel::OnAovSldrIdle(wxIdleEvent& event)
{
	//RenderFrame* vr_frame = (RenderFrame*)m_frame;
	//if (vr_frame && vr_frame->GetClippingView())
	//{
	//	if (vr_frame->GetClippingView()->GetHoldPlanes())
	//		return;
	//}
	bool bval;
	m_agent->getValue(gstCapture, bval);
	if (bval) return;
	fluo::ClipPlaneAgent* agent = glbin_agtf->getClipPlaneAgent();
	if (agent)
	{
		agent->getValue(gstClipHold, bval);
		if (bval) return;
	}

	wxPoint pos = wxGetMousePosition();
	wxRect reg = m_aov_sldr->GetScreenRect();
	wxWindow *window = wxWindow::FindFocus();
	if (window && reg.Contains(pos))
	{
		if (!m_draw_clip)
		{
			m_agent->updValue(gstDrawClip, true);
			m_agent->updValue(gstClipMask, long(-1));
			//RefreshGL(true);
			m_draw_clip = true;
		}
	}
	else
	{
		if (m_draw_clip)
		{
			m_agent->updValue(gstDrawClip, false);
			//RefreshGL(true);
			m_draw_clip = false;
		}
	}
	event.Skip();
}

void RenderviewPanel::OnAovChange(wxScrollEvent& event)
{
	int val = m_aov_sldr->GetValue();
	wxString str = wxString::Format("%d", val);
	if (str != m_aov_text->GetValue())
		m_aov_text->SetValue(str);
}

void RenderviewPanel::OnAovText(wxCommandEvent& event)
{
	wxString str = m_aov_text->GetValue();
	if (str == "Ortho")
	{
		m_agent->updValue(gstPerspective, false);
		m_aov_sldr->SetValue(10);
		//RefreshGL(true);
		return;
	}
	long val;
	if (!str.ToLong(&val))
		return;
	if (val ==0 || val == 10)
	{
		m_agent->updValue(gstPerspective, false);
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
		m_agent->updValue(gstPerspective, true);
		m_agent->updValue(gstAov, double(val));
		m_aov_sldr->SetValue(val);
	}
	//RefreshGL(true);
}

void RenderviewPanel::OnFreeChk(wxCommandEvent& event)
{
	if (m_options_toolbar->GetToolState(ID_FreeChk))
		m_agent->updValue(gstFree, true);
	else
	{
		m_agent->updValue(gstFree, false);
		int val = m_aov_sldr->GetValue();
		if (val == 10)
			m_agent->updValue(gstPerspective, false);
		else
			m_agent->updValue(gstPerspective, true);
	}
	//RefreshGL();
}

void RenderviewPanel::SetFullScreen()
{
	if (m_canvas->GetParent() != m_full_frame)
	{
		m_view_sizer->Detach(m_canvas);
		m_canvas->Reparent(m_full_frame);
		m_full_frame->ShowFullScreen(true);
		m_canvas->SetPosition(wxPoint(0, 0));
		m_canvas->SetSize(m_full_frame->GetSize());
		bool bval;
		glbin_root->getValue(gstStayOnTop, bval);
		if (bval)
			m_full_frame->SetWindowStyle(wxBORDER_NONE|wxSTAY_ON_TOP);
		else
			m_full_frame->SetWindowStyle(wxBORDER_NONE);
#ifdef _WIN32
		glbin_root->getValue(gstShowCursor, bval);
		if (!bval)
			ShowCursor(false);
#endif
		m_full_frame->Iconize(false);
		m_full_frame->Raise();
		m_full_frame->Show();
		m_canvas->m_full_screen = true;
		m_canvas->SetFocus();
		//RefreshGL();
	}
	else
	{
		m_canvas->Close();
	}
}

void RenderviewPanel::AddCanvas(RenderCanvas* canvas)
{
	m_view_sizer->Add(canvas, 1, wxEXPAND);
}

void RenderviewPanel::OnFullScreen(wxCommandEvent& event)
{
	SetFullScreen();
}

void RenderviewPanel::SaveDefault(unsigned int mask)
{
	wxString app_name = "FluoRender " +
		wxString::Format("%d.%.1f", VERSION_MAJOR, float(VERSION_MINOR));
	wxString vendor_name = "FluoRender";
	wxString local_name = "default_view_settings.dft";
	wxFileConfig fconfig(app_name, vendor_name, local_name, "",
		wxCONFIG_USE_LOCAL_FILE);
	wxString str;
	bool bval;
	double dval;
	long lval;
	wxColor cval;
	double x, y, z;

	//render modes
	if (mask & 0x1)
	{
		bval = m_options_toolbar->GetToolState(ID_VolumeSeqRd);
		fconfig.Write("volume_seq_rd", bval);
		bval = m_options_toolbar->GetToolState(ID_VolumeMultiRd);
		fconfig.Write("volume_multi_rd", bval);
		bval = m_options_toolbar->GetToolState(ID_VolumeCompRd);
		fconfig.Write("volume_comp_rd", bval);
	}
	//background color
	if (mask & 0x2)
	{
		cval = m_bg_color_picker->GetColour();
		str = wxString::Format("%d %d %d", cval.Red(), cval.Green(), cval.Blue());
		fconfig.Write("bg_color_picker", str);
	}
	//camera center
	if (mask & 0x4)
	{
		bval = m_options_toolbar->GetToolState(ID_CamCtrChk);
		fconfig.Write("cam_ctr_chk", bval);
	}
	//camctr size
	if (mask & 0x8)
	{
		m_agent->getValue(gstCamCtrSize, dval);
		fconfig.Write("camctr_size", dval);
	}
	//fps
	if (mask & 0x10)
	{
		m_agent->getValue(gstDrawInfo, lval);
		fconfig.Write("info_chk", lval);
	}
	//selection
	if (mask & 0x20)
	{
		m_agent->getValue(gstDrawLegend, bval);
		fconfig.Write("legend_chk", bval);
	}
	//mouse focus
	if (mask & 0x40)
	{
		fconfig.Write("mouse_focus", m_mouse_focus);
	}
	//ortho/persp
	if (mask & 0x80)
	{
		m_agent->getValue(gstPerspective, bval);
		fconfig.Write("persp", bval);
		m_agent->getValue(gstAov, dval);
		fconfig.Write("aov", dval);
		bval = m_options_toolbar->GetToolState(ID_FreeChk);
		fconfig.Write("free_rd", bval);
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
		m_agent->getValue(gstGearedEnable, bval);
		fconfig.Write("rot_lock", bval);
		fconfig.Write("rot_slider", m_rot_slider);
	}
	else
	{
		fconfig.Write("x_rot", m_dft_x_rot);
		fconfig.Write("y_rot", m_dft_y_rot);
		fconfig.Write("z_rot", m_dft_z_rot);
		m_agent->getValue(gstGearedEnable, bval);
		fconfig.Write("rot_lock", bval);
		fconfig.Write("rot_slider", m_rot_slider);
	}
	//depth atten
	if (mask & 0x200)
	{
		bval = m_left_toolbar->GetToolState(ID_DepthAttenChk);
		fconfig.Write("depth_atten_chk", bval);
		str = m_depth_atten_factor_text->GetValue();
		fconfig.Write("depth_atten_factor_text", str);
		str.ToDouble(&m_dft_depth_atten_factor);
	}
	//scale factor
	if (mask & 0x400)
	{
		m_agent->getValue(gstPinRotCtr, bval);
		fconfig.Write("pin_rot_center", bval);
		//str = m_scale_factor_text->GetValue();
		//fconfig.Write("scale_factor_text", str);
		//str.ToDouble(&m_dft_scale_factor);
		m_agent->getValue(gstScaleFactor, dval);
		m_dft_scale_factor = dval;
		fconfig.Write("scale_factor", m_dft_scale_factor);
		m_agent->getValue(gstScaleMode, lval);
		m_dft_scale_factor_mode = lval;
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
		m_agent->getValue(gstCamCtrX, x);
		m_agent->getValue(gstCamCtrY, y);
		m_agent->getValue(gstCamCtrZ, z);
		str = wxString::Format("%f %f %f", x, y, z);
		fconfig.Write("center", str);
	}
	//colormap
	if (mask & 0x1000)
	{
		m_agent->getValue(gstDrawColormap, bval);
		fconfig.Write("colormap_chk", bval);
	}
	wxString expath = wxStandardPaths::Get().GetExecutablePath();
	expath = wxPathOnly(expath);
	wxString dft = expath + GETSLASH() + "default_view_settings.dft";
	SaveConfig(fconfig, dft);

	m_default_saved = true;
}

void RenderviewPanel::OnSaveDefault(wxCommandEvent &event)
{
	SaveDefault();
}

void RenderviewPanel::LoadSettings()
{
	wxString expath = wxStandardPaths::Get().GetExecutablePath();
    expath = wxPathOnly(expath);
    wxString dft = expath + GETSLASH() + "default_view_settings.dft";

	wxFileInputStream is(dft);

	if (!is.IsOk()) {
		wxCommandEvent e;
		OnRotSliderType(e);
		UpdateView();
		return;
	}

	wxFileConfig fconfig(is);

	bool bval;
	double dval;
	long lval;
	if (fconfig.Read("volume_seq_rd", &bval) && bval)
		m_agent->updValue(gstMixMethod, long(fluo::Renderview::MIX_METHOD_SEQ));
	if (fconfig.Read("volume_multi_rd", &bval) && bval)
		m_agent->updValue(gstMixMethod, long(fluo::Renderview::MIX_METHOD_MULTI));
	if (fconfig.Read("volume_comp_rd", &bval) && bval)
		m_agent->updValue(gstMixMethod, long(fluo::Renderview::MIX_METHOD_COMP));

	wxString str;
	if (fconfig.Read("bg_color_picker", &str))
	{
		int r, g, b;
		SSCANF(str.c_str(), "%d%d%d", &r, &g, &b);
		wxColor cVal(r, g, b);
		m_bg_color_picker->SetColour(cVal);
		fluo::Color c(r/255.0, g/255.0, b/255.0);
		m_agent->updValue(gstBgColor, c);
	}
	if (fconfig.Read("cam_ctr_chk", &bval))
	{
		m_options_toolbar->ToggleTool(ID_CamCtrChk,bval);
		m_agent->updValue(gstDrawCamCtr, bval);
	}
	if (fconfig.Read("camctr_size", &dval))
	{
		m_agent->updValue(gstCamCtrSize, dval);
	}
	if (fconfig.Read("info_chk", &lval))
	{
		m_options_toolbar->ToggleTool(ID_FpsChk, lval&fluo::Renderview::INFO_DISP);
		m_agent->updValue(gstDrawInfo, lval);
	}
	if (fconfig.Read("legend_chk", &bval))
	{
		m_options_toolbar->ToggleTool(ID_LegendChk,bval);
		m_agent->updValue(gstDrawLegend, bval);
	}
	if (fconfig.Read("colormap_chk", &bval))
	{
		m_options_toolbar->ToggleTool(ID_ColormapChk, bval);
		m_agent->updValue(gstDrawColormap, bval);
	}
	if (fconfig.Read("mouse_focus", &bval))
	{
		m_mouse_focus = bval;
	}
	if (fconfig.Read("persp", &bval))
	{
		m_agent->updValue(gstPerspective, bval);
	}
	if (fconfig.Read("aov", &dval))
	{
		m_agent->updValue(gstAov, dval);
	}
	if (fconfig.Read("free_rd", &bval))
	{
		m_options_toolbar->ToggleTool(ID_FreeChk,bval);
		m_agent->updValue(gstFree, bval);
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
	if (fconfig.Read("rot_lock", &bval))
	{
		m_rot_lock_btn->ToggleTool(ID_RotLockChk,bval);
		if (bval)
			m_rot_lock_btn->SetToolNormalBitmap(ID_RotLockChk,
				wxGetBitmapFromMemory(gear_45));
		else
			m_rot_lock_btn->SetToolNormalBitmap(ID_RotLockChk,
				wxGetBitmapFromMemory(gear_dark));
		m_agent->updValue(gstGearedEnable, bval);
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
	//	m_canvas->m_scale_factor = dVal/100.0;
	//	m_dft_scale_factor = dVal;
	//}
	if (fconfig.Read("pin_rot_center", &bval))
	{
		m_agent->updValue(gstPinRotCtr, bval);
		if (bval)
			m_agent->updValue(gstRotCtrDirty, true);
		m_pin_btn->ToggleTool(ID_PinBtn, bval);
		if (bval)
			m_pin_btn->SetToolNormalBitmap(ID_PinBtn,
				wxGetBitmapFromMemory(pin));
		else
			m_pin_btn->SetToolNormalBitmap(ID_PinBtn,
				wxGetBitmapFromMemory(anchor_dark));
	}
	if (fconfig.Read("scale_factor_mode", &lval))
	{
		m_dft_scale_factor_mode = lval;
		SetScaleMode(lval, false);
	}
	if (fconfig.Read("scale_factor", &dval))
	{
		m_dft_scale_factor = dval;
		m_agent->updValue(gstScaleFactor, dval);
		UpdateScaleFactor(false);
	}
	if (fconfig.Read("depth_atten_chk", &bval))
	{
		//m_left_toolbar->ToggleTool(ID_DepthAttenChk,bVal);
		m_agent->updValue(gstDepthAtten, bval);
		if (bval)
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
		str.ToDouble(&dval);
		m_depth_atten_factor_sldr->SetValue(int(dval*100));
		m_agent->updValue(gstDaInt, dval);
		m_dft_depth_atten_factor = dval;
	}
	if (fconfig.Read("center", &str))
	{
		float x, y, z;
		SSCANF(str.c_str(), "%f%f%f", &x, &y, &z);
		m_agent->updValue(gstCamCtrX, double(x));
		m_agent->updValue(gstCamCtrY, double(y));
		m_agent->updValue(gstCamCtrZ, double(z));
	}

	m_use_dft_settings = true;
	//RefreshGL();
}

void RenderviewPanel::OnKeyDown(wxKeyEvent& event)
{
	event.Skip();
}

