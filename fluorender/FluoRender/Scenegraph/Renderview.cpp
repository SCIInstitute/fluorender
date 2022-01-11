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

#include "Renderview.hpp"
#include <Group.hpp>
#include <Timer.h>
#include <VolumeData.hpp>
#include <MeshData.hpp>
#include <NodeVisitor.hpp>
#include <Global.hpp>
#include <VolumeLoader.h>
#include <FLIVR/TextureRenderer.h>
#include <FLIVR/VolumeRenderer.h>
#include <FLIVR/ShaderProgram.h>
#include <FLIVR/KernelProgram.h>
#include <FLIVR/ImgShader.h>
#include <FLIVR/VertexArray.h>
#include <glm/gtc/type_ptr.hpp>

using namespace fluo;

Renderview::Renderview()
{
}

Renderview::Renderview(const Renderview& view, const CopyOp& copyop) :
	Group(view, copyop)
{

}

Renderview::~Renderview()
{

}

void Renderview::Init()
{
	bool bval;
	getValue(gstInitialized, bval);
	if (!bval)
	{
		//glViewport(0, 0, (GLint)(GetSize().x), (GLint)(GetSize().y));
		glEnable(GL_MULTISAMPLE);

		setValue(gstInitialized, true);

		glbin_timer->start();
	}
}

void Renderview::Clear()
{
	m_loader->RemoveAllLoadedBrick();
	flvr::TextureRenderer::clear_tex_pool();

	setRvalu(gstCurrentVolume, (Referenced*)(0));
}

void Renderview::InitView(unsigned int type)
{
	bool bval;

	if (type&INIT_BOUNDS)
	{
		BBox bounds, box;
		getValue(gstBounds, bounds);
		bounds.reset();
		PopVolumeList();
		PopMeshList();

		for (auto i : m_vol_list)
		{
			i->getValue(gstBounds, box);
			bounds.extend(box);
		}
		for (auto i : m_msh_list)
		{
			i->getValue(gstBounds, box);
			bounds.extend(box);
		}

		if (bounds.valid())
		{
			setValue(gstBounds, bounds);
			fluo::Vector diag = bounds.diagonal();
			double radius = sqrt(diag.x()*diag.x() + diag.y()*diag.y()) / 2.0;
			if (radius < 0.1)
				radius = 348.0;
			double near_clip = radius / 1000.0;
			double far_clip = radius * 100.0;
			setValue(gstRadius, radius);
			setValue(gstDaStart, near_clip);
			setValue(gstDaEnd, far_clip);
		}
	}

	if (type&INIT_CENTER)
	{
		BBox bounds;
		getValue(gstBounds, bounds);
		if (bounds.valid())
		{
			double obj_ctrx = (bounds.Min().x() + bounds.Max().x()) / 2.0;
			double obj_ctry = (bounds.Min().y() + bounds.Max().y()) / 2.0;
			double obj_ctrz = (bounds.Min().z() + bounds.Max().z()) / 2.0;
			setValue(gstObjCtrX, obj_ctrx);
			setValue(gstObjCtrY, obj_ctry);
			setValue(gstObjCtrZ, obj_ctrz);
		}
	}

	if (type&INIT_TRANSL)
	{
		double radius, aov;
		getValue(gstRadius, radius);
		getValue(gstAov, aov);
		double distance = radius / tan(d2r(aov / 2.0));
		setValue(gstCamDist, distance);
		setValue(gstCamDistIni, distance);
		setValue(gstTransX, double(0));
		setValue(gstTransY, double(0));
		setValue(gstTransZ, distance);
		getValue(gstUseDefault, bval);
		if (!bval)
			setValue(gstScaleFactor, double(1));
	}

	if (type&INIT_OBJ_TRANSL)
	{
		setValue(gstObjTransX, double(0));
		setValue(gstObjTransY, double(0));
		setValue(gstObjTransZ, double(0));
	}

	getValue(gstInitView, bval);
	if (type&INIT_ROTATE || !bval)
	{
		getValue(gstUseDefault, bval);
		if (!bval)
		{
			Quaternion q(0, 0, 0, 1);
			double dx, dy, dz;
			q.ToEuler(dx, dy, dz);
			setValue(gstCamRotQ, q);
			setValue(gstCamRotX, dx);
			setValue(gstCamRotY, dy);
			setValue(gstCamRotZ, dz);
		}
	}

	setValue(gstInitView, true);
}

void Renderview::DrawBounds()
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	flvr::ShaderProgram* shader =
		flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHDR_DRAW_GEOMETRY);
	if (shader)
	{
		if (!shader->valid())
			shader->create();
		shader->bind();
	}
	shader->setLocalParam(0, 1.0, 1.0, 1.0, 1.0);
	glm::mat4 matrix = m_proj_mat * m_mv_mat;
	shader->setLocalParamMatrix(0, glm::value_ptr(matrix));

	flvr::VertexArray* va_cube =
		flvr::TextureRenderer::vertex_array_manager_.vertex_array(flvr::VA_Bound_Cube);
	if (va_cube)
	{
		BBox bounds;
		getValue(gstBounds, bounds);
		va_cube->set_param(bounds);
		va_cube->draw();
	}

	if (shader && shader->valid())
		shader->release();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
}

void Renderview::DrawClippingPlanes(bool border, int face_winding)
{
	int i;
	bool link = false;
	long plane_mode = kNormal;
	long clip_mask;
	getValue(gstClipLinkChan, link);
	getValue(gstClipPlaneMode, plane_mode);
	getValue(gstClipMask, clip_mask);

	if (plane_mode == kNone)
		return;

	bool draw_plane = plane_mode != kFrame;
	if ((plane_mode == kLowTransBack ||
		plane_mode == kNormalBack) &&
		clip_mask == -1)
	{
		glCullFace(GL_FRONT);
		if (face_winding == BACK_FACE)
			face_winding = FRONT_FACE;
		else
			draw_plane = false;
	}
	else
		glCullFace(GL_BACK);

	if (!border && plane_mode == kFrame)
		return;

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (face_winding == FRONT_FACE)
	{
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);
	}
	else if (face_winding == BACK_FACE)
	{
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CW);
	}
	else if (face_winding == CULL_OFF)
		glDisable(GL_CULL_FACE);

	flvr::ShaderProgram* shader =
		flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHDR_DRAW_GEOMETRY);
	if (shader)
	{
		if (!shader->valid())
			shader->create();
		shader->bind();
	}

	Referenced* ref;
	getRvalu(gstCurrentVolume, &ref);
	VolumeData* cur_vol = dynamic_cast<VolumeData*>(ref);
	for (auto vd : m_vol_list)
	{
		if (!vd)
			continue;

		if (vd != cur_vol)
			continue;

		flvr::VolumeRenderer *vr = vd->GetRenderer();
		if (!vr)
			continue;

		std::vector<Plane*> *planes = vr->get_planes();
		if (planes->size() != 6)
			continue;

		//calculating planes
		//get six planes
		fluo::Plane* px1 = (*planes)[0];
		fluo::Plane* px2 = (*planes)[1];
		fluo::Plane* py1 = (*planes)[2];
		fluo::Plane* py2 = (*planes)[3];
		fluo::Plane* pz1 = (*planes)[4];
		fluo::Plane* pz2 = (*planes)[5];

		//calculate 4 lines
		fluo::Vector lv_x1z1, lv_x1z2, lv_x2z1, lv_x2z2;
		fluo::Point lp_x1z1, lp_x1z2, lp_x2z1, lp_x2z2;
		//x1z1
		if (!px1->Intersect(*pz1, lp_x1z1, lv_x1z1))
			continue;
		//x1z2
		if (!px1->Intersect(*pz2, lp_x1z2, lv_x1z2))
			continue;
		//x2z1
		if (!px2->Intersect(*pz1, lp_x2z1, lv_x2z1))
			continue;
		//x2z2
		if (!px2->Intersect(*pz2, lp_x2z2, lv_x2z2))
			continue;

		//calculate 8 points
		fluo::Point pp[8];
		//p0 = l_x1z1 * py1
		if (!py1->Intersect(lp_x1z1, lv_x1z1, pp[0]))
			continue;
		//p1 = l_x1z2 * py1
		if (!py1->Intersect(lp_x1z2, lv_x1z2, pp[1]))
			continue;
		//p2 = l_x2z1 *py1
		if (!py1->Intersect(lp_x2z1, lv_x2z1, pp[2]))
			continue;
		//p3 = l_x2z2 * py1
		if (!py1->Intersect(lp_x2z2, lv_x2z2, pp[3]))
			continue;
		//p4 = l_x1z1 * py2
		if (!py2->Intersect(lp_x1z1, lv_x1z1, pp[4]))
			continue;
		//p5 = l_x1z2 * py2
		if (!py2->Intersect(lp_x1z2, lv_x1z2, pp[5]))
			continue;
		//p6 = l_x2z1 * py2
		if (!py2->Intersect(lp_x2z1, lv_x2z1, pp[6]))
			continue;
		//p7 = l_x2z2 * py2
		if (!py2->Intersect(lp_x2z2, lv_x2z2, pp[7]))
			continue;

		//draw the six planes out of the eight points
		//get color
		fluo::Color color(1.0, 1.0, 1.0);
		double plane_trans = 0.0;
		if (face_winding == BACK_FACE &&
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
			plane_trans = plane_mode == kLowTrans ||
			plane_mode == kLowTransBack ? 0.1 : 0.3;

		if (face_winding == FRONT_FACE)
		{
			plane_trans = plane_mode == kLowTrans ||
				plane_mode == kLowTransBack ? 0.1 : 0.3;
		}

		if (plane_mode == kNormal ||
			plane_mode == kNormalBack)
		{
			if (!link)
				vd->getValue(gstColor, color);
		}
		else
			getValue(gstTextColor, color);

		//transform
		if (!vd->GetTexture())
			continue;
		fluo::Transform *tform = vd->GetTexture()->transform();
		if (!tform)
			continue;
		double mvmat[16];
		tform->get_trans(mvmat);
		double sclx, scly, sclz;
		vd->getValue(gstScaleX, sclx);
		vd->getValue(gstScaleY, scly);
		vd->getValue(gstScaleZ, sclz);
		glm::mat4 mv_mat = glm::scale(m_mv_mat,
			glm::vec3(float(sclx), float(scly), float(sclz)));
		glm::mat4 mv_mat2 = glm::mat4(
			mvmat[0], mvmat[4], mvmat[8], mvmat[12],
			mvmat[1], mvmat[5], mvmat[9], mvmat[13],
			mvmat[2], mvmat[6], mvmat[10], mvmat[14],
			mvmat[3], mvmat[7], mvmat[11], mvmat[15]);
		mv_mat = mv_mat * mv_mat2;
		glm::mat4 matrix = m_proj_mat * mv_mat;
		shader->setLocalParamMatrix(0, glm::value_ptr(matrix));

		flvr::VertexArray* va_clipp =
			flvr::TextureRenderer::vertex_array_manager_.vertex_array(flvr::VA_Clip_Planes);
		if (!va_clipp)
			return;
		std::vector<fluo::Point> clip_points(pp, pp + 8);
		va_clipp->set_param(clip_points);
		va_clipp->draw_begin();
		//draw
		//x1 = (p4, p0, p1, p5)
		if (clip_mask & 1)
		{
			if (draw_plane)
			{
				if (plane_mode == kNormal ||
					plane_mode == kNormalBack)
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
				if (plane_mode == kNormal ||
					plane_mode == kNormalBack)
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
				if (plane_mode == kNormal ||
					plane_mode == kNormalBack)
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
				if (plane_mode == kNormal ||
					plane_mode == kNormalBack)
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
				if (plane_mode == kNormal ||
					plane_mode == kNormalBack)
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
				if (plane_mode == kNormal ||
					plane_mode == kNormalBack)
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
	}

	if (shader && shader->valid())
		shader->release();

	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
}

void Renderview::PopVolumeList()
{
	bool bval;
	getValue(gstVolListDirty, bval);
	if (!bval)
		return;

	int i, j;
	m_vol_list.clear();

	class PopVolumeVisitor : public NodeVisitor
	{
	public:
		PopVolumeVisitor() : NodeVisitor()
		{
			setTraversalMode(NodeVisitor::TRAVERSE_CHILDREN);
		}

		virtual void reset()
		{
			list_.clear();
		}

		virtual void apply(Node& node)
		{
			VolumeData* vd = node.asVolumeData();
			if (vd)
			{
				bool disp;
				vd->getValue(gstDisplay, disp);
				if (disp)
					list_.push_back(vd);
			}
			traverse(node);
		}

		virtual void apply(Group& group)
		{
			bool disp;
			bool result = group.getValue(gstDisplay, disp);
			if (!result || disp)
				traverse(group);
		}

		void getResult(std::vector<ref_ptr<VolumeData>> &list)
		{
			for (auto vd : list_)
				list.push_back(vd);
		}

	private:
		std::vector<VolumeData*> list_;
	};
	PopVolumeVisitor visitor;
	accept(visitor);
	visitor.getResult(m_vol_list);

	setValue(gstVolListDirty, false);
}

void Renderview::PopMeshList()
{
	bool bval;
	getValue(gstMshListDirty, bval);
	if (!bval)
		return;

	int i, j;
	m_msh_list.clear();

	class PopMeshVisitor : public NodeVisitor
	{
	public:
		PopMeshVisitor() : NodeVisitor()
		{
			setTraversalMode(NodeVisitor::TRAVERSE_CHILDREN);
		}

		virtual void reset()
		{
			list_.clear();
		}

		virtual void apply(Node& node)
		{
			MeshData* md = node.asMeshData();
			if (md)
			{
				bool disp;
				md->getValue(gstDisplay, disp);
				if (disp)
					list_.push_back(md);
			}
			traverse(node);
		}

		virtual void apply(Group& group)
		{
			bool disp;
			bool result = group.getValue(gstDisplay, disp);
			if (!result || disp)
				traverse(group);
		}

		void getResult(std::vector<ref_ptr<MeshData>> &list)
		{
			for (auto md : list_)
				list.push_back(md);
		}

	private:
		std::vector<MeshData*> list_;
	};
	PopMeshVisitor visitor;
	accept(visitor);
	visitor.getResult(m_msh_list);

	setValue(gstMshListDirty, false);
}


