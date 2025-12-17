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

#include <CamCenterRenderer.h>
#include <Global.h>
#include <Framebuffer.h>
#include <VertexArray.h>
#include <ShaderProgram.h>
#include <ImgShader.h>
#include <RenderView.h>
#include <compatibility.h>
#include <Debug.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace flrd;

void CamCenterRenderer::render()
{
	auto base_buffer = glbin_framebuffer_manager.current();
	assert(base_buffer);
	flvr::FramebufferStateGuard fbg(*base_buffer);
	base_buffer->set_blend_enabled_all(false);
	base_buffer->set_depth_test_enabled(false);
	base_buffer->apply_state();

	std::shared_ptr<flvr::VertexArray> va;
	switch (settings_->style)
	{
	case CamCenterStyle::CenterJack:
	case CamCenterStyle::CornerJack:
		va = glbin_vertex_array_manager.vertex_array(flvr::VAType::VA_Cam_Jack);
		break;
	case CamCenterStyle::Crosshair:
		va = glbin_vertex_array_manager.vertex_array(flvr::VAType::VA_Cam_Crosshair);
		break;
	}
	assert(va);

	va->set_param(0, settings_->size);

	auto shader =
		glbin_shader_manager.shader(gstImgShader,
			flvr::ShaderParams::Img(IMG_SHDR_DRAW_GEOMETRY, 0));
	assert(shader);
	shader->bind();

	va->draw_begin();
	switch (settings_->style)
	{
	case CamCenterStyle::CenterJack:
	case CamCenterStyle::CornerJack:
		drawJack(va, shader);
		break;
	case CamCenterStyle::Crosshair:
		drawCrosshair(va, shader);
		break;
	}
	va->draw_end();

	shader->unbind();
	//fbg exits
}

void CamCenterRenderer::drawCrosshair(const std::shared_ptr<flvr::VertexArray>& va, const std::shared_ptr<flvr::ShaderProgram>& shader)
{
	auto view = settings_->view.lock();
	if (!view)
		return;
	int nx, ny;
	view->GetRenderSize(nx, ny);
	glm::mat4 matrix = glm::ortho(-nx / 2.0f, nx / 2.0f, -ny / 2.0f, ny / 2.0f);
	shader->setLocalParamMatrix(0, glm::value_ptr(matrix));
	fluo::Color text_color = view->GetTextColor();
	shader->setLocalParam(0, text_color.r(), text_color.g(), text_color.b(), 1.0);
	va->draw_cam_center();
}

void CamCenterRenderer::drawJack(const std::shared_ptr<flvr::VertexArray>& va, const std::shared_ptr<flvr::ShaderProgram>& shader)
{
	auto view = settings_->view.lock();
	if (!view)
		return;
	glm::mat4 viewRot = glm::mat4(glm::mat3(view->GetModelView()));
	glm::mat4 matrix = view->GetProjection() * viewRot;
	shader->setLocalParamMatrix(0, glm::value_ptr(matrix));
	shader->setLocalParam(0, 1.0, 0.0, 0.0, 1.0);
	va->draw_cam_jack(0);
	shader->setLocalParam(0, 0.0, 1.0, 0.0, 1.0);
	va->draw_cam_jack(1);
	shader->setLocalParam(0, 0.0, 0.0, 1.0, 1.0);
	va->draw_cam_jack(2);
}
