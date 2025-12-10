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

#include <ClippingBoxRenderer.h>
#include <Global.h>
#include <Names.h>
#include <MainSettings.h>
#include <Framebuffer.h>
#include <ImgShader.h>
#include <VertexArray.h>
#include <RenderView.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace flrd;

void ClippingBoxRenderer::render()
{
	ClippingRenderMode mode = settings_->mode;
	FaceWinding winding = settings_->winding;
	bool all_planes = (settings_->plane_mask & 0x3F) == 0x3F;
	switch (mode)
	{
		case ClippingRenderMode::None:
			return;
		case ClippingRenderMode::ColoredFront:
		case ClippingRenderMode::FrameFront:
		case ClippingRenderMode::TransFront:
			if (winding != FaceWinding::Front &&
				all_planes)
				return;
			break;
		case ClippingRenderMode::ColoredBack:
		case ClippingRenderMode::FrameBack:
		case ClippingRenderMode::TransBack:
			if (winding != FaceWinding::Back &&
				all_planes)
				return;
			break;
		case ClippingRenderMode::FrameAll:
			break;
	}

	auto view = settings_->view.lock();
	if (!view)
		return;

	auto planes = box_.GetPlanesWorld();
	//calculating planes
	//get six planes
	fluo::Plane px1 = planes[0];
	fluo::Plane px2 = planes[1];
	fluo::Plane py1 = planes[2];
	fluo::Plane py2 = planes[3];
	fluo::Plane pz1 = planes[4];
	fluo::Plane pz2 = planes[5];

	//calculate 4 lines
	fluo::Vector lv_x1z1, lv_x1z2, lv_x2z1, lv_x2z2;
	fluo::Point lp_x1z1, lp_x1z2, lp_x2z1, lp_x2z2;
	//x1z1
	if (!px1.Intersect(pz1, lp_x1z1, lv_x1z1))
		return;
	//x1z2
	if (!px1.Intersect(pz2, lp_x1z2, lv_x1z2))
		return;
	//x2z1
	if (!px2.Intersect(pz1, lp_x2z1, lv_x2z1))
		return;
	//x2z2
	if (!px2.Intersect(pz2, lp_x2z2, lv_x2z2))
		return;

	//calculate 8 points
	fluo::Point pp[8];
	//p0 = l_x1z1 * py1
	if (!py1.Intersect(lp_x1z1, lv_x1z1, pp[0]))
		return;
	//p1 = l_x1z2 * py1
	if (!py1.Intersect(lp_x1z2, lv_x1z2, pp[1]))
		return;
	//p2 = l_x2z1 *py1
	if (!py1.Intersect(lp_x2z1, lv_x2z1, pp[2]))
		return;
	//p3 = l_x2z2 * py1
	if (!py1.Intersect(lp_x2z2, lv_x2z2, pp[3]))
		return;
	//p4 = l_x1z1 * py2
	if (!py2.Intersect(lp_x1z1, lv_x1z1, pp[4]))
		return;
	//p5 = l_x1z2 * py2
	if (!py2.Intersect(lp_x1z2, lv_x1z2, pp[5]))
		return;
	//p6 = l_x2z1 * py2
	if (!py2.Intersect(lp_x2z1, lv_x2z1, pp[6]))
		return;
	//p7 = l_x2z2 * py2
	if (!py2.Intersect(lp_x2z2, lv_x2z2, pp[7]))
		return;

	double width = glbin_settings.m_line_width;

	auto data_buffer = glbin_framebuffer_manager.current();
	assert(data_buffer);
	flvr::FramebufferStateGuard fbg(*data_buffer);//outer guard

	data_buffer->set_depth_test_enabled(false);
	data_buffer->set_blend_enabled(0, true);
	data_buffer->set_blend_func(0,
		flvr::BlendFactor::SrcAlpha, flvr::BlendFactor::OneMinusSrcAlpha,
		flvr::BlendFactor::SrcAlpha, flvr::BlendFactor::OneMinusSrcAlpha);
	if (settings_->winding == flrd::FaceWinding::Front)
	{
		data_buffer->set_cull_face_enabled(true);
		data_buffer->set_face_winding(flvr::FaceWinding::Front);
		data_buffer->set_cull_face(flvr::CullFace::Front);
	}
	else if (settings_->winding == flrd::FaceWinding::Back)
	{
		data_buffer->set_cull_face_enabled(true);
		data_buffer->set_face_winding(flvr::FaceWinding::Back);
		data_buffer->set_cull_face(flvr::CullFace::Front);
	}
	else if (settings_->winding == flrd::FaceWinding::Off)
	{
		data_buffer->set_cull_face_enabled(false);
	}
	data_buffer->apply_state();

	auto shader1 = glbin_shader_manager.shader(gstImgShader,
		flvr::ShaderParams::Img(IMG_SHDR_DRAW_GEOMETRY, 0));
	assert(shader1);
	shader1->bind();
	auto shader2 = glbin_shader_manager.shader(gstImgShader,
		flvr::ShaderParams::Img(IMG_SHDR_DRAW_CLIPPING_BOX_LINES, 0));

	//transform
	glm::mat4 mv_mat = view->GetModelView();
	glm::mat4 proj_mat = view->GetProjection();
	auto size = view->GetSize();
	glm::mat4 matrix = proj_mat * mv_mat;
	shader1->setLocalParamMatrix(0, glm::value_ptr(matrix));
	if (settings_->draw_border)
	{
		shader2->bind();
		shader2->setLocalParamMatrix(0, glm::value_ptr(mv_mat));
		shader2->setLocalParamMatrix(1, glm::value_ptr(proj_mat));
		double cull = 0.0;
		//if (all_planes)
		{
			if (settings_->winding == flrd::FaceWinding::Front)
				cull = 1.0;
			else if (settings_->winding == flrd::FaceWinding::Back)
				cull = -1.0;
		}
		shader2->setLocalParam(0, size.w(), size.h(), width, cull);
		shader1->bind();
	}

	auto va_clipp = glbin_vertex_array_manager.vertex_array(flvr::VAType::VA_Clip_Planes);
	assert(va_clipp);
	std::vector<fluo::Point> clip_points(pp, pp + 8);
	va_clipp->set_param(clip_points);
	va_clipp->draw_begin();
	// Draw all six planes
	for (int i = 0; i < 6; ++i)
		drawPlane(i, va_clipp, shader1, shader2, data_buffer);
	va_clipp->draw_end();

	shader1->unbind();
	//fbg exits
}

void ClippingBoxRenderer::drawPlane(int index,
	const std::shared_ptr<flvr::VertexArray>& va_clipp,
	const std::shared_ptr<flvr::ShaderProgram>& shader1,
	const std::shared_ptr<flvr::ShaderProgram>& shader2,
	const std::shared_ptr<flvr::Framebuffer>& data_buffer)
{
	// mask check: each plane corresponds to a bit
	if (!(settings_->plane_mask & (1 << index)))
		return;

	bool draw_plane = settings_->draw_face;
	if (settings_->mode == ClippingRenderMode::FrameAll ||
		settings_->mode == ClippingRenderMode::FrameFront ||
		settings_->mode == ClippingRenderMode::FrameBack)
		draw_plane = false;
	bool draw_border = settings_->draw_border;
	auto plane_mode = settings_->mode;
	auto color = settings_->color;
	double alpha = settings_->alpha;

	if (draw_plane)
	{
		// Select color depending on mode
		if (plane_mode == ClippingRenderMode::ColoredFront ||
			plane_mode == ClippingRenderMode::ColoredBack)
			color = getPlaneColor(index);
		else if (plane_mode == ClippingRenderMode::TransFront ||
			plane_mode == ClippingRenderMode::TransBack)
			color = fluo::Color(1.0);
		if (plane_mode == ClippingRenderMode::TransFront ||
			plane_mode == ClippingRenderMode::TransBack)
			alpha /= 3.0;

		shader1->setLocalParam(0, color.r(), color.g(), color.b(), alpha);

		// Each plane has a slot in the VA
		va_clipp->draw_clip_plane(index, false);
	}

	if (draw_border)
	{
		flvr::FramebufferStateGuard fbg2(*data_buffer); // inner guard
		data_buffer->set_cull_face_enabled(false);
		data_buffer->apply_state();

		shader2->bind();
		shader2->setLocalParam(1, color.r(), color.g(), color.b(), 1.0);
		auto normal = getPlaneNormal(index);
		double persp = -1.0;
		auto view = settings_->view.lock();
		if (view)
			persp = view->GetPersp() ? 1.0 : -1.0;
		shader2->setLocalParam(2, normal.x(), normal.y(), normal.z(), persp);

		// Border planes are offset by +16 in VA indexing
		va_clipp->draw_clip_plane(index, true);

		shader1->bind();
		// fbg2 exits automatically
	}
}

fluo::Color ClippingBoxRenderer::getPlaneColor(int index)
{
	switch (index)
	{
	case 0:
		return fluo::Color(1.0, 0.5, 0.5);
	case 1:
		return fluo::Color(1.0, 0.5, 1.0);
	case 2:
		return fluo::Color(0.5, 1.0, 0.5);
	case 3:
		return fluo::Color(1.0, 1.0, 0.5);
	case 4:
		return fluo::Color(0.5, 0.5, 1.0);
	case 5:
		return fluo::Color(0.5, 1.0, 1.0);
	}
	return fluo::Color(1.0);
}

fluo::Vector ClippingBoxRenderer::getPlaneNormal(int idx)
{
	if (idx == 0) return fluo::Vector(-1.0, 0.0, 0.0); // -X
	if (idx == 1) return fluo::Vector(1.0, 0.0, 0.0); // +X
	if (idx == 2) return fluo::Vector(0.0, -1.0, 0.0); // -Y
	if (idx == 3) return fluo::Vector(0.0, 1.0, 0.0); // +Y
	if (idx == 4) return fluo::Vector(0.0, 0.0, -1.0); // -Z
	if (idx == 5) return fluo::Vector(0.0, 0.0, 1.0); // +Z
	return fluo::Vector(0.0, 0.0, 0.0); // fallback
}
