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
#include <Plane.hpp>
#include <FLIVR/ShaderProgram.h>
#include <FLIVR/TextureRenderer.h>
#include <FLIVR/ImgShader.h>
#include <FLIVR/VertexArray.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "ClipPlaneRenderer.hpp"

using namespace FLR;

ClipPlaneRenderer::ClipPlaneRenderer():
	Renderer3D()
{
	setupInputs();
	setupOutputs();
}

ClipPlaneRenderer::ClipPlaneRenderer(const ClipPlaneRenderer& renderer, const fluo::CopyOp& copyop, bool copy_values):
	Renderer3D(renderer, copyop, false)
{
	if (copy_values)
		copyValues(renderer, copyop);
}

ClipPlaneRenderer::~ClipPlaneRenderer()
{
}

void ClipPlaneRenderer::setupInputs()
{
	//direct inputs
	addValue("border", bool(true));
	addValue("face winding", long(1));//CULL_OFF:0; FRONT_FACE:1; BACK_FACE:2
	inputs_.insert("border");
	inputs_.insert("face winding");
	//inputs from object
	inputs_.insert("clip display");
	inputs_.insert("clip hold");
	inputs_.insert("clip render mode");
	inputs_.insert("clip mask");
	inputs_.insert("clip planes");
	inputs_.insert("color");
	inputs_.insert("scale x");
	inputs_.insert("scale y");
	inputs_.insert("scale z");
	inputs_.insert("tex transform");
}

void ClipPlaneRenderer::setupOutputs()
{

}

bool ClipPlaneRenderer::render(fluo::Event& event)
{
	bool result = Renderer3D::render(event);

	bool display = false;
	getValue("clip display", display);
	bool hold = false;
	getValue("clip hold", hold);
	if (!display && !hold)
		return false;

	bool border;
	getValue("border", border);
	long face_winding;
	getValue("face winding", face_winding);
	long render_mode = FLTYPE::PRMNone;
	getValue("clip render mode", render_mode);
	if (render_mode == FLTYPE::PRMNone)
		return false;

	long clip_mask;
	getValue("clip mask", clip_mask);

	bool draw_plane = render_mode != FLTYPE::PRMFrame;
	if ((render_mode == FLTYPE::PRMLowTransBack ||
		render_mode == FLTYPE::PRMNormalBack) &&
		clip_mask == -1)
	{
		glCullFace(GL_FRONT);
		if (face_winding == FLTYPE::CPWBackFace)
			face_winding = FLTYPE::CPWFrontFace;
		else
			draw_plane = false;
	}
	else
		glCullFace(GL_BACK);

	if (!border && render_mode == FLTYPE::PRMFrame)
		return false;

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (face_winding == FLTYPE::CPWFrontFace)
	{
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);
	}
	else if (face_winding == FLTYPE::CPWBackFace)
	{
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CW);
	}
	else if (face_winding == FLTYPE::CPWOff)
		glDisable(GL_CULL_FACE);

	FLIVR::ShaderProgram* shader =
		FLIVR::TextureRenderer::img_shader_factory_.shader(IMG_SHDR_DRAW_GEOMETRY);
	if (shader)
	{
		if (!shader->valid())
			shader->create();
		shader->bind();
	}

	//vector<FLTYPE::Plane*> *planes = vr->get_planes();
	FLTYPE::PlaneSet planes;
	getValue("clip planes", planes);
	if (planes.GetSize() != 6)
		return false;

	//calculating planes
	//get six planes
	FLTYPE::Plane* px1 = &(planes[0]);
	FLTYPE::Plane* px2 = &(planes[1]);
	FLTYPE::Plane* py1 = &(planes[2]);
	FLTYPE::Plane* py2 = &(planes[3]);
	FLTYPE::Plane* pz1 = &(planes[4]);
	FLTYPE::Plane* pz2 = &(planes[5]);

	//calculate 4 lines
	FLTYPE::Vector lv_x1z1, lv_x1z2, lv_x2z1, lv_x2z2;
	FLTYPE::Point lp_x1z1, lp_x1z2, lp_x2z1, lp_x2z2;
	//x1z1
	if (!px1->Intersect(*pz1, lp_x1z1, lv_x1z1))
		return false;
	//x1z2
	if (!px1->Intersect(*pz2, lp_x1z2, lv_x1z2))
		return false;
	//x2z1
	if (!px2->Intersect(*pz1, lp_x2z1, lv_x2z1))
		return false;
	//x2z2
	if (!px2->Intersect(*pz2, lp_x2z2, lv_x2z2))
		return false;

	//calculate 8 points
	FLTYPE::Point pp[8];
	//p0 = l_x1z1 * py1
	if (!py1->Intersect(lp_x1z1, lv_x1z1, pp[0]))
		return false;
	//p1 = l_x1z2 * py1
	if (!py1->Intersect(lp_x1z2, lv_x1z2, pp[1]))
		return false;
	//p2 = l_x2z1 *py1
	if (!py1->Intersect(lp_x2z1, lv_x2z1, pp[2]))
		return false;
	//p3 = l_x2z2 * py1
	if (!py1->Intersect(lp_x2z2, lv_x2z2, pp[3]))
		return false;
	//p4 = l_x1z1 * py2
	if (!py2->Intersect(lp_x1z1, lv_x1z1, pp[4]))
		return false;
	//p5 = l_x1z2 * py2
	if (!py2->Intersect(lp_x1z2, lv_x1z2, pp[5]))
		return false;
	//p6 = l_x2z1 * py2
	if (!py2->Intersect(lp_x2z1, lv_x2z1, pp[6]))
		return false;
	//p7 = l_x2z2 * py2
	if (!py2->Intersect(lp_x2z2, lv_x2z2, pp[7]))
		return false;

	//draw the six planes out of the eight points
	//get color
	FLTYPE::Color color(1.0, 1.0, 1.0);
	if (render_mode == FLTYPE::PRMNormal ||
		render_mode == FLTYPE::PRMNormalBack)
		getValue("color", color);
	double plane_trans = 0.0;
	if (face_winding == FLTYPE::CPWBackFace &&
		(clip_mask == 3 ||
		clip_mask == 12 ||
		clip_mask == 48 ||
		clip_mask == 1 ||
		clip_mask == 2 ||
		clip_mask == 4 ||
		clip_mask == 8 ||
		clip_mask == 16 ||
		clip_mask == 32 ||
		clip_mask == 64)
		)
		plane_trans = render_mode == FLTYPE::PRMLowTrans ||
			render_mode == FLTYPE::PRMLowTransBack ? 0.1 : 0.3;

	if (face_winding == FLTYPE::CPWFrontFace)
		plane_trans = render_mode == FLTYPE::PRMLowTrans ||
		render_mode == FLTYPE::PRMLowTransBack ? 0.1 : 0.3;

	float matrix[16];
	FLTYPE::Transform tform;
	//model view
	getValue("model view matrix", tform);
	tform.get(matrix);
	glm::mat4 mv_mat = glm::make_mat4(matrix);
	//vd->GetScalings(sclx, scly, sclz);
	double sclx, scly, sclz;
	getValue("scale x", sclx);
	getValue("scale y", scly);
	getValue("scale z", sclz);
	mv_mat = glm::scale(mv_mat,
		glm::vec3(float(sclx), float(scly), float(sclz)));
	//transform
	getValue("tex transform", tform);
	tform.get(matrix);
	glm::mat4 mv_mat2 = glm::make_mat4(matrix);
	mv_mat = mv_mat * mv_mat2;
	//proj
	getValue("projection matrix", tform);
	tform.get(matrix);
	glm::mat4 proj_mat = glm::make_mat4(matrix);
	glm::mat4 mat = proj_mat * mv_mat;
	shader->setLocalParamMatrix(0, glm::value_ptr(mat));

	FLIVR::VertexArray* va_clipp =
		FLIVR::TextureRenderer::vertex_array_manager_.vertex_array(FLIVR::VA_Clip_Planes);
	if (!va_clipp)
		return false;
	std::vector<FLTYPE::Point> clip_points(pp, pp+8);
	va_clipp->set_param(clip_points);
	va_clipp->draw_begin();
	//draw
	//x1 = (p4, p0, p1, p5)
	if (clip_mask & 1)
	{
		if (draw_plane)
		{
			if (render_mode == FLTYPE::PRMNormal ||
				render_mode == FLTYPE::PRMNormalBack)
				shader->setLocalParam(0, 1.0, 0.5, 0.5, plane_trans);
			else
				shader->setLocalParam(0, color.r(), color.g(), color.b(), plane_trans);
			va_clipp->draw_clip_plane(0, false);
		}
		if (border)
		{
			shader->setLocalParam(0, color.r(), color.g(), color.b(), plane_trans);
			va_clipp->draw_clip_plane(16, true);
		}
	}
	//x2 = (p7, p3, p2, p6)
	if (clip_mask & 2)
	{
		if (draw_plane)
		{
			if (render_mode == FLTYPE::PRMNormal ||
				render_mode == FLTYPE::PRMNormalBack)
				shader->setLocalParam(0, 1.0, 0.5, 1.0, plane_trans);
			else
				shader->setLocalParam(0, color.r(), color.g(), color.b(), plane_trans);
			va_clipp->draw_clip_plane(32, false);
		}
		if (border)
		{
			shader->setLocalParam(0, color.r(), color.g(), color.b(), plane_trans);
			va_clipp->draw_clip_plane(48, true);
		}
	}
	//y1 = (p1, p0, p2, p3)
	if (clip_mask & 4)
	{
		if (draw_plane)
		{
			if (render_mode == FLTYPE::PRMNormal ||
				render_mode == FLTYPE::PRMNormalBack)
				shader->setLocalParam(0, 0.5, 1.0, 0.5, plane_trans);
			else
				shader->setLocalParam(0, color.r(), color.g(), color.b(), plane_trans);
			va_clipp->draw_clip_plane(64, false);
		}
		if (border)
		{
			shader->setLocalParam(0, color.r(), color.g(), color.b(), plane_trans);
			va_clipp->draw_clip_plane(80, true);
		}
	}
	//y2 = (p4, p5, p7, p6)
	if (clip_mask & 8)
	{
		if (draw_plane)
		{
			if (render_mode == FLTYPE::PRMNormal ||
				render_mode == FLTYPE::PRMNormalBack)
				shader->setLocalParam(0, 1.0, 1.0, 0.5, plane_trans);
			else
				shader->setLocalParam(0, color.r(), color.g(), color.b(), plane_trans);
			va_clipp->draw_clip_plane(96, false);
		}
		if (border)
		{
			shader->setLocalParam(0, color.r(), color.g(), color.b(), plane_trans);
			va_clipp->draw_clip_plane(112, true);
		}
	}
	//z1 = (p0, p4, p6, p2)
	if (clip_mask & 16)
	{
		if (draw_plane)
		{
			if (render_mode == FLTYPE::PRMNormal ||
				render_mode == FLTYPE::PRMNormalBack)
				shader->setLocalParam(0, 0.5, 0.5, 1.0, plane_trans);
			else
				shader->setLocalParam(0, color.r(), color.g(), color.b(), plane_trans);
			va_clipp->draw_clip_plane(128, false);
		}
		if (border)
		{
			shader->setLocalParam(0, color.r(), color.g(), color.b(), plane_trans);
			va_clipp->draw_clip_plane(144, true);
		}
	}
	//z2 = (p5, p1, p3, p7)
	if (clip_mask & 32)
	{
		if (draw_plane)
		{
			if (render_mode == FLTYPE::PRMNormal ||
				render_mode == FLTYPE::PRMNormalBack)
				shader->setLocalParam(0, 0.5, 1.0, 1.0, plane_trans);
			else
				shader->setLocalParam(0, color.r(), color.g(), color.b(), plane_trans);
			va_clipp->draw_clip_plane(160, false);
		}
		if (border)
		{
			shader->setLocalParam(0, color.r(), color.g(), color.b(), plane_trans);
			va_clipp->draw_clip_plane(176, true);
		}
	}
	va_clipp->draw_end();

	if (shader && shader->valid())
		shader->release();

	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);

	return result;
}
