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

#include <RenderSystem.h>
#include <Global.h>
#include <ShaderProgram.h>
#include <MainSettings.h>
#include <KernelProgram.h>
#include <Coordinator.h>
#include <SettingDlgAgent.h>
#include <BaseXrRenderer.h>
#include <CurrentObjects.h>
#include <RenderView.h>
#include <RenderCanvasAgent.h>
#include <RenderCanvas.h>
#include <LookingGlassRenderer.h>
#if defined(_WIN32) || defined(_WIN64)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

using namespace flvr;

bool RenderSystem::view_renderer_initialized_ = false;
bool RenderSystem::openxr_initialized_ = false;
bool RenderSystem::looking_glass_initialized_ = false;

void RenderSystem::InitViewRenderer()
{
	if (view_renderer_initialized_)
		return;

	flvr::ShaderProgram::init_shaders_supported();
	flvr::ShaderProgram::set_max_texture_size(glbin_settings.m_max_texture_size);
	flvr::ShaderProgram::set_no_tex_upack(glbin_settings.m_no_tex_pack);
	flvr::KernelProgram::set_platform_id(glbin_settings.m_cl_platform_id);
	flvr::KernelProgram::set_device_id(glbin_settings.m_cl_device_id);
	flvr::KernelProgram::init_kernels_supported();
#ifdef _DARWIN
	CGLContextObj ctx = CGLGetCurrentContext();
	if (ctx != flvr::TextureRenderer::gl_context_)
		flvr::TextureRenderer::gl_context_ = ctx;
#endif
	glbin_settings.GetGraphicsInfo();
	flvr::BrickTexture::mask_undo_num_ = (size_t)(glbin_brush_def.m_paint_hist_depth);

	//update ui
	auto setting_agent = glbin_coordinator.FindAgent<SettingDlgAgent>();
	if (setting_agent)
		setting_agent->UpdateDataToUI({ gstMaxTextureSize, gstDeviceTree });

	view_renderer_initialized_ = true;
}

void RenderSystem::InitOpenXR()
{
	if (openxr_initialized_)
		return;
#ifdef _WIN32
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;
	HDC hdc = GetDC((HWND)view->GetHandle());
	HGLRC hglrc = wglGetCurrentContext();
	if (glbin_xr_renderer)
		openxr_initialized_ = glbin_xr_renderer->Init(
			static_cast<void*>(hdc),
			static_cast<void*>(hglrc),
			0);
#elif defined(__APPLE__)
	/*CGLContextObj cglContext = CGLGetCurrentContext();
	// Define the pixel format attributes based on wxGLCanvas attributes
	NSOpenGLPixelFormatAttribute profile_ver = NSOpenGLProfileVersionLegacy;
	if (glbin_settings.m_gl_major_ver == 3)
		profile_ver = NSOpenGLProfileVersion3_2Core;
	else if (glbin_settings.gl_major_ver == 4)
		profile_ver = NSOpenGLProfileVersion4_1Core;
	NSOpenGLPixelFormatAttribute color_size =
		glbin_settings.m_red_bit +
		glbin_settings.m_green_bit +
		glbin_settings.m_blue_bit;
	NSOpenGLPixelFormatAttribute alpha_size = glbin_settings.m_alpha_bit;
	NSOpenGLPixelFormatAttribute depth_size = glbin_settings.m_depth_bit;
	NSOpenGLPixelFormatAttribute attributes[] = {
		NSOpenGLPFAOpenGLProfile, profile_ver,
		NSOpenGLPFAColorSize, color_size,
		NSOpenGLPFAAlphaSize, alpha_size,
		NSOpenGLPFADepthSize, depth_size,
		NSOpenGLPFADoubleBuffer,
		0
	};
	// Create the NSOpenGLPixelFormat object
	NSOpenGLPixelFormat* pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
	if (!pixelFormat)
		return;
	// Retrieve the CGLPixelFormatObj from the NSOpenGLPixelFormat
	CGLPixelFormatObj cglPixelFormat = [pixelFormat CGLPixelFormatObj];*/
	if (glbin_xr_renderer)
		m_use_openxr = glbin_xr_renderer->Init(
			0,
			0,
			0);
#elif defined(__linux__)
	Display* xDisplay = glXGetCurrentDisplay();
	GLXDrawable glxDrawable = glXGetCurrentDrawable();
	GLXContext glxContext = glXGetCurrentContext();
	if (glbin_xr_renderer)
		m_use_openxr = glbin_xr_renderer->Init(
			static_cast<void*>(xDisplay),
			static_cast<void*>(glxContext),
			static_cast<uint64_t>(glxDrawable));
#endif
}

void RenderSystem::InitLookingGlass()
{
	//set canvas size to the display
	//looking glass size
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;
	glbin_lg_renderer.SetRenderViewSize(view->GetCanvasSize());
	Size2D lg_size = glbin_lg_renderer.GetViewSize();
	if (lg_size.isValid())
		view->SetSize(lg_size);
	if (looking_glass_initialized_)
		return;
	bool bval = glbin_lg_renderer.Init();
	auto setting_agent = glbin_coordinator.FindAgent<SettingDlgAgent>();
	if (!bval)
	{
		glbin_settings.m_hologram_mode = 0;
		if (setting_agent)
			setting_agent->UpdateDataToUI({ gstHologramMode });
		return;
	}
	glbin_lg_renderer.SetDevIndex(glbin_settings.m_lg_dev_id);
	glbin_lg_renderer.Setup();
	glbin_lg_renderer.Clear();
	if (glbin_settings.m_lg_offset == 0)
		glbin_settings.m_lg_offset = glbin_lg_renderer.GetHalfCone();
	glbin_settings.m_disp_id = glbin_lg_renderer.GetDisplayId();
	if (setting_agent)
		setting_agent->UpdateDataToUI({ gstHologramMode, gstFullscreenDisplay });
	//if (m_render_view_panel)
	//	m_render_view_panel->SetFullScreen();
	looking_glass_initialized_ = true;
}

