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

using namespace flvr;

bool RenderSystem::initialized_ = false;

void RenderSystem::Init()
{
	if (initialized_)
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

	initialized_ = true;
}