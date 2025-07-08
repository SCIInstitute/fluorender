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
#include <LookingGlassRenderer.h>
#include <Global.h>
#include <MainSettings.h>
#include <Framebuffer.h>
#include <ShaderProgram.h>
#include <ImgShader.h>
#include <LightFieldShader.h>
#include <VertexArray.h>
#include <compatibility.h>
#include <string>
#include <Debug.h>

LookingGlassRenderer::LookingGlassRenderer()
{
	// Create the bridge controller
	m_lg_controller = std::make_unique<Controller>();
}

LookingGlassRenderer::~LookingGlassRenderer()
{
	Close();
}

bool LookingGlassRenderer::Init()
{
	if (m_initialized)
		return m_initialized;

	std::wstring wstr;
#ifdef _WIN32
	if (!m_lg_controller->Initialize(L"FluoRender"))
#else
	if (!controller->Initialize("FluoRender"))
#endif
	{
		m_lg_controller.reset();
		DBGPRINT(L"Failed to initialize bridge. Bridge may be missing, or the version may be too old\n");
		return false;
	}
	// Create BridgeData
	WINDOW_HANDLE wnd = 0;

	if (m_lg_controller)
	{
		// Get display information list
		m_lg_displays = m_lg_controller->GetDisplayInfoList();
		if (m_lg_displays.empty())
		{
			DBGPRINT(L"No Looking Glass displays found. Please connect a display and try again.\n");
			return false;
		}
		if (m_cur_lg_display < 0 || m_cur_lg_display >= m_lg_displays.size())
			m_cur_lg_display = 0;

		// Print all display names
		//for (const auto& displayInfo : displays)
		//{
		//	std::wcout << displayInfo.name << std::endl;
		//}

		// For now we will use the first looking glass display
		if (!m_lg_displays.empty() && m_lg_controller->InstanceOffscreenWindowGL(&wnd, m_lg_displays[m_cur_lg_display].display_id))
		{
			DBGPRINT(L"Successfully created the window handle.\n");
		}
		else
		{
			wnd = 0;
			DBGPRINT(L"Failed to initialize bridge window. do you have any displays connected?\n");
			return false;
		}
	}

	m_lg_data = m_lg_controller ? m_lg_controller->GetWindowData(wnd) : BridgeWindowData();
	m_initialized = (m_lg_data.wnd != 0);
	m_viewCone = m_lg_displays[m_cur_lg_display].viewcone;
	//m_initialized = true;
	return m_initialized;
}

void LookingGlassRenderer::Close()
{
	if (m_initialized && m_lg_controller)
	{
		m_lg_controller->Uninitialize();
		m_initialized = false;
	}
}

int LookingGlassRenderer::GetDisplayId()
{
	if (!m_initialized)
		return 0;
	if (m_lg_displays.empty())
		return 0;
	if (m_cur_lg_display < 0 || m_cur_lg_display >= m_lg_displays.size())
		return 0;
	return m_lg_displays[m_cur_lg_display].display_id;
}

void LookingGlassRenderer::Setup()
{
	if (!m_initialized)
		return;

	//set up framebuffer for quilt
	flvr::Framebuffer* quilt_buffer =
		glbin_framebuffer_manager.framebuffer(
			flvr::FB_Render_RGBA, m_lg_data.quilt_width, m_lg_data.quilt_height, "quilt");
	quilt_buffer->protect();

	flvr::ShaderProgram* shader = 0;
	//set up shader to render quilt
	shader = glbin_light_field_shader_factory.shader(0);
	if (shader)
	{
		if (!shader->valid())
			shader->create();
		shader->bind();
	}

	float pitch = m_lg_displays[m_cur_lg_display].pitch;
	float tilt = m_lg_displays[m_cur_lg_display].tilt;
	float center = m_lg_displays[m_cur_lg_display].center;
	float subp = m_lg_displays[m_cur_lg_display].subp;
	float vp0 = float(m_lg_data.view_width * m_lg_data.vx) / float(m_lg_data.quilt_width);
	float vp1 = float(m_lg_data.view_height * m_lg_data.vy) / float(m_lg_data.quilt_height);
	float aspect = m_lg_displays[m_cur_lg_display].aspect;
	int invView = m_lg_displays[m_cur_lg_display].viewinv;
	int ri = m_lg_displays[m_cur_lg_display].ri;
	int bi = m_lg_displays[m_cur_lg_display].bi;
	int quiltInvert = 1;

	shader->setLocalParam(0, pitch, aspect / tilt, center, subp);
	shader->setLocalParam(1, vp0, vp1, aspect, aspect);
	shader->setLocalParam(2, m_lg_data.vx, m_lg_data.vy, m_lg_data.vx * m_lg_data.vy, 0);
	shader->setLocalParamInt4(0, invView, ri, bi, quiltInvert);

	shader->release();
}

void LookingGlassRenderer::Clear()
{
	if (!m_initialized)
		return;

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
	if (!m_initialized)
		return;

	m_finished = false;
	//draw view tex to quilt
	//bind quilt frame buffer
	glActiveTexture(GL_TEXTURE0);
	flvr::Framebuffer* quilt_buffer =
		glbin_framebuffer_manager.framebuffer("quilt");
	quilt_buffer->bind();
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);//debug
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	//texture lookup shader
	flvr::ShaderProgram* shader = 0;
	shader = glbin_img_shader_factory.shader(IMG_SHADER_TEXTURE_LOOKUP);
	shader->bind();
	//set up view port for place texture
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	// get the x and y origin for this view
	int x = (m_cur_view % m_lg_data.vx) * m_lg_data.view_width;
	int y = int(float(m_cur_view) / float(m_lg_data.vx)) * m_lg_data.view_height;
	glViewport(x, y, m_lg_data.view_width, m_lg_data.view_height);
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

	//move index for next
	advance_views();

	//draw quilt to view
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// reset viewport
	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
	//glDisable(GL_BLEND);
	//glDisable(GL_DEPTH_TEST);
	shader = glbin_light_field_shader_factory.shader(0);
	shader->bind();
	shader->setLocalParamUInt(0, glbin_settings.m_hologram_debug ? 1 : 0);//show quilt
	quilt_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
	quad_va->draw();
	shader->release();
	glBindTexture(GL_TEXTURE_2D, 0);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

void LookingGlassRenderer::SetUpdating(bool val)
{
	if (!m_initialized)
		return;

	m_updating = val;
	if (val)
		m_upd_view = m_cur_view;
}

double LookingGlassRenderer::GetOffset()
{
	if (!m_initialized)
		return 0.0;

	double len = double(m_lg_data.vx * m_lg_data.vy - 1) / 2;
	return (m_cur_view - len) / len;
}

void LookingGlassRenderer::BindRenderBuffer(int nx, int ny)
{
	if (!m_initialized)
		return;

	flvr::Framebuffer* buffer =
		glbin_framebuffer_manager.framebuffer(
			flvr::FB_Render_RGBA, nx, ny, "quilt view");
	if (buffer)
		buffer->bind();
}

void LookingGlassRenderer::advance_views()
{
	if (!m_initialized)
		return;

	m_cur_view++;
	if (m_cur_view == m_lg_data.vx * m_lg_data.vy)
		m_cur_view = 0;
	if (!m_updating && m_cur_view == m_upd_view)
		m_finished = true;
}
