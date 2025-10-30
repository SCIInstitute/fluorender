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

#include <LookingGlassRenderer.h>
#include <Global.h>
#include <Names.h>
#include <MainSettings.h>
#include <Framebuffer.h>
#include <ShaderProgram.h>
#include <ImgShader.h>
#include <LightFieldShader.h>
#include <VertexArray.h>
#include <compatibility.h>
#include <bridge_utils.hpp>
#include <glm/gtc/matrix_transform.hpp>
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
	if (!m_lg_controller->Initialize("FluoRender"))
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
		std::vector<DisplayInfo> displayList = m_lg_controller->GetDisplayInfoList();
		m_lg_displays.clear();
		m_lg_displays.reserve(displayList.size());

		for (const auto& display : displayList) {
			m_lg_displays.push_back(std::make_shared<DisplayInfo>(display));
		}
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
		if (!m_lg_displays.empty() && m_lg_controller->InstanceWindowGL(&wnd, m_lg_displays[m_cur_lg_display]->display_id))
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

	m_lg_data = m_lg_controller
		? std::make_unique<BridgeWindowData>(m_lg_controller->GetWindowData(wnd))
		: std::make_unique<BridgeWindowData>();

	m_initialized = (m_lg_data->wnd != 0);
	m_viewCone = m_lg_displays[m_cur_lg_display]->viewcone;
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
	return m_lg_displays[m_cur_lg_display]->display_id;
}

void LookingGlassRenderer::Setup()
{
	if (!m_initialized)
		return;

	//set up framebuffer for quilt
	auto quilt_buffer =
		glbin_framebuffer_manager.framebuffer(
			flvr::FBRole::RenderFloatDepth, m_lg_data->quilt_width, m_lg_data->quilt_height, gstRBQuilt);
	assert(quilt_buffer);

/*	flvr::ShaderProgram* shader = 0;
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

	shader->release();*/
}

void LookingGlassRenderer::Clear()
{
	if (!m_initialized)
		return;

	auto quilt_buffer =
		glbin_framebuffer_manager.framebuffer(gstRBQuilt);
	assert(quilt_buffer);
	//clear fbo
	auto guard = glbin_framebuffer_manager.bind_scoped(quilt_buffer);
	quilt_buffer->clear(true, true);
}

void LookingGlassRenderer::Draw()
{
	if (!m_initialized)
		return;

	m_finished = false;

	auto cur_buffer = glbin_framebuffer_manager.current();

	//set up view port for place texture
	// get the x and y origin for this view
	int corrected_view = m_lg_data->vx * m_lg_data->vy - 1 - m_cur_view;
	int x = (corrected_view % m_lg_data->vx) * m_lg_data->view_width;
	int y = (m_lg_data->vy - 1 - (corrected_view / m_lg_data->vx)) * m_lg_data->view_height;
	//draw view tex to quilt
	//bind quilt frame buffer
	auto quilt_buffer =
		glbin_framebuffer_manager.framebuffer(gstRBQuilt);
	assert(quilt_buffer);
	quilt_buffer->set_blend_enabled(false);
	quilt_buffer->set_depth_test_enabled(false);
	//set viewport size
	quilt_buffer->set_viewport({ x, y, m_lg_data->view_width, m_lg_data->view_height });
	glbin_framebuffer_manager.bind(quilt_buffer);

	//texture lookup shader
	auto shader = glbin_shader_manager.shader(gstImgShader,
		flvr::ShaderParams::Img(IMG_SHDR_TEXTURE_FLIP, 0));
	assert(shader);
	shader->bind();
	//bind texture
	auto view_buffer =
		glbin_framebuffer_manager.framebuffer(gstRBQuiltView);
	assert(view_buffer);
	view_buffer->bind_texture(flvr::AttachmentPoint::Color(0), 0);
	auto quad_va =
		glbin_vertex_array_manager.vertex_array(flvr::VAType::VA_Norm_Square);
	assert(quad_va);
	quad_va->draw();
	shader->unbind();
	view_buffer->unbind_texture(flvr::AttachmentPoint::Color(0));

	//move index for next
	advance_views();

	quilt_buffer->bind_texture(flvr::AttachmentPoint::Color(0), 0);
	m_lg_controller->DrawInteropQuiltTextureGL(m_lg_data->wnd,
		quilt_buffer->tex_id(flvr::AttachmentPoint::Color(0)), PixelFormats::RGBA,
		m_lg_data->quilt_width, m_lg_data->quilt_height,
		m_lg_data->vx, m_lg_data->vy,
		m_lg_data->displayaspect, 1.0f);

	//shader = glbin_light_field_shader_factory.shader(0);
	//shader->bind();
	//shader->setLocalParamUInt(0, glbin_settings.m_hologram_debug ? 1 : 0);//show quilt
	//quilt_buffer->bind_texture(flvr::AttachmentPoint::Color(0));
	//quad_va->draw();
	//shader->release();

	//draw quilt to view
	//only draw the middle view
	// reset viewport
	assert(cur_buffer);
	//set viewport size and clear
	cur_buffer->set_viewport({ 0, 0, m_render_view_size.w(), m_render_view_size.h() });
	glbin_framebuffer_manager.bind(cur_buffer);
	cur_buffer->clear(true, true);

	shader = glbin_shader_manager.shader(gstImgShader,
		flvr::ShaderParams::Img(IMG_SHADER_TEXTURE_LOOKUP, 0));
	assert(shader);
	shader->bind();

	auto rect_va =
		glbin_vertex_array_manager.vertex_array(flvr::VAType::VA_Rectangle);
	assert(rect_va);
	int mode = glbin_settings.m_hologram_debug;
	if (mode == 3)
	{
		rect_va->set_param(0, 1);
		rect_va->set_param(1, 1);
		rect_va->set_param(2, 0);
		rect_va->set_param(3, 1);
		rect_va->set_param(4, 1);
		rect_va->set_param(5, 0);
	}
	else
	{
		rect_va->set_param(0, (float)m_lg_data->view_width / (float)m_lg_data->view_height);
		rect_va->set_param(1, (float)m_render_view_size.w() / (float)m_render_view_size.h());
		int view_index = 0;
		switch (mode)
		{
			case 0:
			default:
				view_index = (m_lg_data->vx * m_lg_data->vy) / 2;
				break;
			case 1:
				view_index = 0;
				break;
			case 2:
				view_index = m_lg_data->vx * m_lg_data->vy - 1;
				break;
		}
		int cx = view_index % m_lg_data->vx;
		int cy = m_lg_data->vy - 1 - (view_index / m_lg_data->vx); // Flip Y if origin is bottom-left
		float u0 = (float)(cx * m_lg_data->view_width) / m_lg_data->quilt_width;
		float u1 = (float)((cx + 1) * m_lg_data->view_width) / m_lg_data->quilt_width;
		float v0 = (float)((cy + 1) * m_lg_data->view_height) / m_lg_data->quilt_height;
		float v1 = (float)(cy * m_lg_data->view_height) / m_lg_data->quilt_height;
		rect_va->set_param(2, u0);
		rect_va->set_param(3, u1);
		rect_va->set_param(4, v0);
		rect_va->set_param(5, v1);
	}
	rect_va->draw();
	shader->unbind();

	quilt_buffer->unbind_texture(flvr::AttachmentPoint::Color(0));
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

	double len = double(m_lg_data->vx * m_lg_data->vy - 1) / 2;
	return (m_cur_view - len) / len;
}

void LookingGlassRenderer::BindViewBaseFramebuffer(int nx, int ny)
{
	if (!m_initialized)
		return;

	auto buffer =
		glbin_framebuffer_manager.framebuffer(
			flvr::FBRole::RenderFloat, nx, ny, gstRBQuiltView);
	assert(buffer);
	glbin_framebuffer_manager.bind(buffer);
}

Size2D LookingGlassRenderer::GetViewSize() const
{
	if (!m_initialized)
		return Size2D(-1, -1);
	return Size2D(m_lg_data->view_width, m_lg_data->view_height);
}

Size2D LookingGlassRenderer::GetQuiltSize()
{
	if (!m_initialized)
		return Size2D(-1, -1);
	return Size2D(m_lg_data->quilt_width, m_lg_data->quilt_height);
}

Size2D LookingGlassRenderer::GetQuiltLayout()
{
	if (!m_initialized)
		return Size2D(-1, -1);
	return Size2D(m_lg_data->vx, m_lg_data->vy);
}

float LookingGlassRenderer::GetAspect()
{
	if (!m_initialized)
		return 1.0;
	return m_lg_data->displayaspect;
}

void LookingGlassRenderer::advance_views()
{
	if (!m_initialized)
		return;

	m_cur_view++;
	if (m_cur_view == m_lg_data->vx * m_lg_data->vy)
		m_cur_view = 0;
	if (!m_updating && m_cur_view == m_upd_view)
		m_finished = true;
}

void LookingGlassRenderer::HandleCamera(bool persp)
{
	switch (glbin_settings.m_hologram_camera_mode)
	{
		case 0: // turntable
		case 1: // shifting
		default:
			HandleCameraShifting(persp);
			break;
		case 2: // shifting skew
			HandleCameraTurntable();
			break;
	}
}

void LookingGlassRenderer::HandleProjection(bool persp)
{
	if (persp)
	{
		m_proj_mat = glm::perspective(m_aov, m_aspect, m_near, m_far);
	}
	else
	{
		m_proj_mat = glm::ortho(m_left, m_right, m_bottom, m_top,
			m_near, m_far);
	}

	if (glbin_settings.m_hologram_camera_mode == 0)
	{
		double d = glbin_settings.m_lg_offset;
		d = m_distance * sin(glm::radians(d));
		double shift = GetOffset() * d;
		// Apply horizontal skew
		if (persp)
		{
			float offset = float(shift * 2.0 / (m_right - m_left));
			m_proj_mat[2][0] += offset; // GLM uses column-major layout
		}
		else
		{
			float offset = float(shift * (m_far - m_near) / ((m_right - m_left) * m_distance));
			glm::mat4 shear = glm::mat4(1.0f);
			shear[2][0] = -offset;
			m_proj_mat = shear * m_proj_mat;
		}
	}
}

glm::mat4 LookingGlassRenderer::GetProjectionMatrix() const
{
	return m_proj_mat;
}

void LookingGlassRenderer::HandleCameraTurntable()
{
	//turntable
	double ang = GetOffset() * glbin_settings.m_lg_offset;//half angle
	glm::mat4 rot(1);
	rot = glm::rotate(rot, float(glm::radians(-ang)), m_up);
	glm::vec4 vv = glm::vec4(m_eye - m_center, 1);
	vv = rot * vv;
	m_eye = m_center + glm::vec3(vv);
	glm::vec3 new_view = glm::vec3(vv);
	new_view = glm::normalize(new_view);
	m_up = glm::cross(new_view, m_up);
	m_up = glm::cross(m_up, new_view);
}

void LookingGlassRenderer::HandleCameraShifting(bool persp)
{
	//linear shift
	double d = glbin_settings.m_lg_offset;
	d = m_distance * sin(glm::radians(d));
	double shift = GetOffset() * d;
	glm::vec3 side = m_side * float(shift);

	if (glbin_settings.m_hologram_camera_mode == 0)
	{
		if (persp)
		{
			m_eye += side;
			m_center += side;
		}
		else
		{
			double shear = shift * (m_far + m_near) / (2.0 * m_distance);
			shear = shift - shear;
			side = m_side * float(shear);
			m_eye += side;
			m_center += side;
		}
	}
	else
	{
		m_eye += side;
	}
}
