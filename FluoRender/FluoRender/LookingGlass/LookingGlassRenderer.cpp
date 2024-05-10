/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2024 Scientific Computing and Imaging Institute,
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

#include <LookingGlassRenderer.h>
#include <Global.h>
#include <HoloPlayCore.h>
#include <compatibility.h>
#include <string>
#include <Debug.h>

LookingGlassRenderer::LookingGlassRenderer() :
	m_initialized(false),
	m_dev_index(0),
	m_cur_view(0)
{
	SetPreset(1);
}

LookingGlassRenderer::~LookingGlassRenderer()
{
	Close();
}

bool LookingGlassRenderer::Init()
{
	if (m_initialized)
		return true;
	hpc_client_error errco = hpc_InitializeApp("FluoRender", hpc_LICENSE_NONCOMMERCIAL);
	std::wstring wstr;
	if (errco)
	{
		switch (errco)
		{
		case hpc_CLIERR_NOSERVICE:
			wstr = L"HoloPlay Service not running";
			break;
		case hpc_CLIERR_SERIALIZEERR:
			wstr = L"Client message could not be serialized";
			break;
		case hpc_CLIERR_VERSIONERR:
			wstr = L"Incompatible version of HoloPlay Service";
			break;
		case hpc_CLIERR_PIPEERROR:
			wstr = L"Interprocess pipe broken";
			break;
		case hpc_CLIERR_SENDTIMEOUT:
			wstr = L"Interprocess pipe send timeout";
			break;
		case hpc_CLIERR_RECVTIMEOUT:
			wstr = L"Interprocess pipe receive timeout";
			break;
		default:
			wstr = L"Unknown error";
			break;
		}
		DBGPRINT(L"HoloPlay Service access error (code %d): %s.\n", errco, wstr.c_str());
		return false;
	}
	std::string str;
	char buf[1000];
	hpc_GetHoloPlayCoreVersion(buf, 1000);
	str = buf;
	wstr = s2ws(str);
	DBGPRINT(L"HoloPlay Core version %s.\n", wstr.c_str());
	hpc_GetHoloPlayServiceVersion(buf, 1000);
	str = buf;
	wstr = s2ws(str);
	DBGPRINT(L"HoloPlay Service version %s.\n", wstr.c_str());
	int ival = hpc_GetNumDevices();
	DBGPRINT(L"%d devices connected.\n", ival);
	if (ival < 1)
		return false;
	m_initialized = true;
	return true;
}

void LookingGlassRenderer::Close()
{
	if (m_initialized)
	{
		hpc_CloseApp();
		m_initialized = false;
	}
}

void LookingGlassRenderer::SetPreset(int val)
{
	switch (val)
	{
	case 0: // standard
		m_width = 2048;
		m_height = 2048;
		m_columns = 4;
		m_rows = 8;
		m_totalViews = 32;
		m_preset = 0;
		break;
	default:
	case 1: // hires
		m_width = 4096;
		m_height = 4096;
		m_columns = 5;
		m_rows = 9;
		m_totalViews = 45;
		m_preset = 1;
		break;
	case 2: // 8k
		m_width = 4096 * 2;
		m_height = 4096 * 2;
		m_columns = 5;
		m_rows = 9;
		m_totalViews = 45;
		m_preset = 2;
		break;
	}
	m_viewWidth = int(float(m_width) / float(m_columns));
	m_viewHeight = int(float(m_height) / float(m_rows));
}

void LookingGlassRenderer::Setup()
{
	//set up framebuffer for quilt
	flvr::Framebuffer* quilt_buffer =
		glbin_framebuffer_manager.framebuffer(
			flvr::FB_Render_RGBA, m_width, m_height, "quilt");
	quilt_buffer->protect();

	flvr::ShaderProgram* shader = 0;
	//set up shader to render texture
	shader = glbin_img_shader_factory.shader(IMG_SHADER_TEXTURE_LOOKUP);
	if (shader)
	{
		if (!shader->valid())
			shader->create();
	}
	//set up shader to render quilt
	shader = glbin_light_field_shader_factory.shader(0);
	if (shader)
	{
		if (!shader->valid())
			shader->create();
		shader->bind();
	}

	float pitch = hpc_GetDevicePropertyPitch(m_dev_index);
	float tilt = hpc_GetDevicePropertyTilt(m_dev_index);
	float center = hpc_GetDevicePropertyCenter(m_dev_index);
	float subp = hpc_GetDevicePropertySubp(m_dev_index);
	shader->setLocalParam(0, pitch, tilt, center, subp);
	float vp0 = float(m_viewWidth * m_columns) / float(m_width);
	float vp1 = float(m_viewHeight * m_rows) / float(m_height);
	float displayAspect = hpc_GetDevicePropertyDisplayAspect(m_dev_index);
	float quiltAspect = hpc_GetDevicePropertyDisplayAspect(m_dev_index);
	shader->setLocalParam(1, vp0, vp1, displayAspect, quiltAspect);
	shader->setLocalParam(2, m_columns, m_rows, m_totalViews, 0);
	int invView = hpc_GetDevicePropertyInvView(m_dev_index);
	int ri = hpc_GetDevicePropertyRi(m_dev_index);
	int bi = hpc_GetDevicePropertyBi(m_dev_index);
	int quiltInvert = 0;
	shader->setLocalParamInt4(0, invView, ri, bi, quiltInvert);
	shader->setLocalParamUInt(0, 0);

	shader->release();
}

void LookingGlassRenderer::Clear()
{
	flvr::Framebuffer* quilt_buffer =
		glbin_framebuffer_manager.framebuffer("quilt");
	quilt_buffer->bind();
	glClearDepth(1);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void LookingGlassRenderer::Draw()
{
	//draw view tex to quilt
	//bind quilt frame buffer
	glActiveTexture(GL_TEXTURE0);
	flvr::Framebuffer* quilt_buffer =
		glbin_framebuffer_manager.framebuffer("quilt");
	quilt_buffer->bind();
	//texture lookup shader
	flvr::ShaderProgram* shader = 0;
	shader = glbin_img_shader_factory.shader(IMG_SHADER_TEXTURE_LOOKUP);
	shader->bind();
	//set up view port for place texture
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	// get the x and y origin for this view
	int x = (m_cur_view % m_columns) * m_viewWidth;
	int y = int(std::round(float(m_cur_view) / float(m_columns))) * m_viewHeight;
	glViewport(x, y, m_viewWidth, m_viewHeight);
	//bind texture
	flvr::Framebuffer* view_buffer =
		glbin_framebuffer_manager.framebuffer("quilt view");
	if (view_buffer)
		view_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
	flvr::VertexArray* quad_va =
		glbin_vertex_array_manager.vertex_array(flvr::VA_Norm_Square);
	quad_va->draw();
	shader->release();
	glBindTexture(GL_TEXTURE_2D, 0);

	GLHINT;
	//move index for next
	m_cur_view++;
	if (m_cur_view == m_totalViews)
		m_cur_view = 0;

	//draw quilt to view
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// reset viewport
	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	shader = glbin_light_field_shader_factory.shader(0);
	shader->bind();
	quilt_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
	quad_va->draw();
	shader->release();
	glBindTexture(GL_TEXTURE_2D, 0);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

int LookingGlassRenderer::GetViewNum()
{
	return m_totalViews;
}

int LookingGlassRenderer::GetCurView()
{
	return m_cur_view;
}

double LookingGlassRenderer::GetOffset()
{
	double len = double(m_totalViews) / 2;
	return (m_cur_view - len) / len;
}

void LookingGlassRenderer::BindRenderBuffer(int nx, int ny)
{
	flvr::Framebuffer* buffer =
		glbin_framebuffer_manager.framebuffer(
			flvr::FB_Render_RGBA, nx, ny, "quilt view");
	if (buffer)
		buffer->bind();
}
