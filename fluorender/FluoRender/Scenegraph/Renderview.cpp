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
#include <RenderviewFactory.hpp>
#include <VolumeData.hpp>
#include <MeshData.hpp>
#include <NodeVisitor.hpp>
#include <Global.hpp>
#include <SearchVisitor.hpp>
#include <VolumeLoader.h>
#include <compatibility.h>
#include <Animator/Interpolator.h>
#include <FLIVR/TextureRenderer.h>
#include <FLIVR/VolumeRenderer.h>
#include <FLIVR/ShaderProgram.h>
#include <FLIVR/KernelProgram.h>
#include <FLIVR/ImgShader.h>
#include <FLIVR/VertexArray.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

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
			Vector diag = bounds.diagonal();
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
		Plane* px1 = (*planes)[0];
		Plane* px2 = (*planes)[1];
		Plane* py1 = (*planes)[2];
		Plane* py2 = (*planes)[3];
		Plane* pz1 = (*planes)[4];
		Plane* pz2 = (*planes)[5];

		//calculate 4 lines
		Vector lv_x1z1, lv_x1z2, lv_x2z1, lv_x2z2;
		Point lp_x1z1, lp_x1z2, lp_x2z1, lp_x2z2;
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
		Point pp[8];
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
		Color color(1.0, 1.0, 1.0);
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
		Transform *tform = vd->GetTexture()->transform();
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
		std::vector<Point> clip_points(pp, pp + 8);
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

void Renderview::ClearVolList()
{
	m_loader->RemoveAllLoadedBrick();
	flvr::TextureRenderer::clear_tex_pool();
	m_vol_list.clear();
}

void Renderview::ClearMeshList()
{
	m_msh_list.clear();
}

void Renderview::HandleProjection(int nx, int ny, bool vr)
{
	bool bval;
	getValue(gstFree, bval);
	double radius, aov, scale_factor, distance;
	getValue(gstRadius, radius);
	getValue(gstAov, aov);
	getValue(gstScaleFactor, scale_factor);
	if (!bval)
	{
		distance = radius / tan(d2r(aov / 2.0)) / scale_factor;
		setValue(gstCamDist, distance);
	}

	double aspect = (double)nx / (double)ny;
	double ortho_left = -radius * aspect / scale_factor;
	double ortho_right = -ortho_left;
	double ortho_bottom = -radius / scale_factor;
	double ortho_top = -ortho_bottom;
	setValue(gstOrthoLeft, ortho_left);
	setValue(gstOrthoRight, ortho_right);
	setValue(gstOrthoBottom, ortho_bottom);
	setValue(gstOrthoTop, ortho_top);

	double nc, fc;
	getValue(gstNearClip, nc);
	getValue(gstFarClip, fc);
	getValue(gstOpenvrEnable, bval);
	if (vr && bval)
	{
#ifdef _WIN32
		//get projection matrix
		long lval;
		getValue(gstVrEyeIdx, lval);
		vr::EVREye eye = lval ? vr::Eye_Right : vr::Eye_Left;
		auto proj_mat = m_vr_system->GetProjectionMatrix(eye, nc, fc);
		static int ti[] = { 0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15 };
		for (int i = 0; i < 16; ++i)
			glm::value_ptr(m_proj_mat)[i] =
			((float*)(proj_mat.m))[ti[i]];
#endif
	}
	else
	{
		getValue(gstPerspective, bval);
		if (bval)
		{
			m_proj_mat = glm::perspective(glm::radians(aov), aspect, nc, fc);
		}
		else
		{
			m_proj_mat = glm::ortho(ortho_left, ortho_right, ortho_bottom, ortho_top,
				-fc / 100.0, fc);
		}
	}
}

void Renderview::HandleCamera(bool vr)
{
	double dx, dy, dz;
	getValue(gstTransX, dx);
	getValue(gstTransY, dy);
	getValue(gstTransZ, dz);
	Vector pos(dx, dy, dz);
	pos.normalize();
	bool bval;
	getValue(gstFree, bval);
	if (bval)
		pos *= 0.1;
	else
	{
		double distance;
		getValue(gstCamDist, distance);
		pos *= distance;
	}
	glm::vec3 eye(pos.x(), pos.y(), pos.z());
	glm::vec3 center(0.0);
	Vector vval;
	getValue(gstCamUp, vval);
	glm::vec3 up(vval.x(), vval.y(), vval.z());

	if (bval)
	{
		getValue(gstCamCtrX, dx);
		getValue(gstCamCtrY, dy);
		getValue(gstCamCtrZ, dz);
		center = glm::vec3(dx, dy, dz);
		eye += center;
	}

	getValue(gstVrEnable, bval);
	if (vr && bval)
	{
		long lval;
		getValue(gstVrEyeIdx, lval);
		double dval;
		getValue(gstVrEyeOffset, dval);
		glm::vec3 offset((lval ? 1.0 : -1.0) * dval / 2.0, 0.0, 0.0);
		m_mv_mat = glm::lookAt(
			eye + offset,
			center + offset,
			up);
	}
	else
	{
		m_mv_mat = glm::lookAt(eye, center, up);
	}

	getValue(gstCamLockObjEnable, bval);
	if (bval)
	{
		Point pval;
		getValue(gstCamLockCtr, pval);
		//rotate first
		glm::vec3 v1(0, 0, -1);//initial cam direction
		glm::mat4 mv_mat = GetDrawMat();
		glm::vec4 lock_ctr(pval.x(), pval.y(), pval.z(), 1);
		lock_ctr = mv_mat * lock_ctr;
		glm::vec3 v2(lock_ctr);
		v2 = glm::normalize(v2);
		float c = glm::dot(v1, v2);
		if (std::abs(std::abs(c) - 1) < Epsilon())
			return;
		glm::vec3 v = glm::cross(v1, v2);
		glm::mat3 vx(
			0, -v.z, v.y,
			v.z, 0, -v.x,
			-v.y, v.x, 0);
		glm::mat3 vx2 = vx * vx;
		glm::mat3 rot3(1);
		rot3 += vx + vx2 / (1 + c);
		glm::mat4 rot4(rot3);
		m_mv_mat = rot4 * glm::lookAt(eye, center, up);
	}
}

void Renderview::Set3DRotCapture(
	const std::wstring &cap_file,
	double start_angle,
	double end_angle,
	double step,
	long frames,
	long rot_axis,
	bool rewind)
{
	double rv[3];
	getValue(gstRotX, rv[0]);
	getValue(gstRotY, rv[1]);
	getValue(gstRotZ, rv[2]);

	//remove the chance of the x/y/z angles being outside 360.
	while (rv[0] > 360.)  rv[0] -= 360.;
	while (rv[0] < -360.) rv[0] += 360.;
	while (rv[1] > 360.)  rv[1] -= 360.;
	while (rv[1] < -360.) rv[1] += 360.;
	while (rv[2] > 360.)  rv[2] -= 360.;
	while (rv[2] < -360.) rv[2] += 360.;
	if (360. - std::abs(rv[0]) < 0.001) rv[0] = 0.;
	if (360. - std::abs(rv[1]) < 0.001) rv[1] = 0.;
	if (360. - std::abs(rv[2]) < 0.001) rv[2] = 0.;

	setValue(gstMovStep, step);
	setValue(gstTotalFrames, frames);
	setValue(gstCaptureFile, cap_file);
	setValue(gstMovRewind, rewind);
	setValue(gstMovSeqNum, long(0));
	setValue(gstMovRotAxis, rot_axis);
	if (start_angle == 0.)
	{
		setValue(gstMovInitAng, rv[rot_axis]);
		setValue(gstMovEndAng, rv[rot_axis] + end_angle);
	}
	setValue(gstMovCurAng, rv[rot_axis]);
	setValue(gstMovStartAng, rv[rot_axis]);
	setValue(gstCapture, true);
	setValue(gstCaptureRot, true);
	setValue(gstCaptureRotOver, false);
	setValue(gstMovStage, long(0));
}

void Renderview::Set4DSeqCapture(
	const std::wstring &cap_file,
	long begin_frame, long end_frame, bool rewind)
{
	setValue(gstCaptureFile, cap_file);
	setValue(gstCurrentFrame, begin_frame);
	setValue(gstPreviousFrame, begin_frame);
	setValue(gstBeginFrame, begin_frame);
	setValue(gstEndFrame, end_frame);
	setValue(gstCaptureTime, true);
	setValue(gstCapture, true);
	setValue(gstMovSeqNum, begin_frame);
	setValue(gstMovRewind4d, rewind);
}

void Renderview::Set3DBatCapture(
	const std::wstring &cap_file,
	long begin_frame, long end_frame)
{
	setValue(gstCaptureFile, cap_file);
	setValue(gstBeginFrame, begin_frame);
	setValue(gstEndFrame, end_frame);
	setValue(gstCaptureBat, true);
	setValue(gstCapture, true);

	long lval;
	getValue(gstTotalFrames, lval);
	std::wstring wsval;
	getValue(gstBatFolder, wsval);
	if (!cap_file.empty() && lval > 1)
	{
		std::wstring new_folder = GET_PATH(cap_file)
			+ GETSLASH() + wsval + L"_folder";
		MkDirW(new_folder);
	}
}

void Renderview::SetParamCapture(
	const std::wstring &cap_file,
	long begin_frame, long end_frame, bool rewind)
{
	setValue(gstCaptureFile, cap_file);
	setValue(gstParamFrame, begin_frame);
	setValue(gstBeginFrame, begin_frame);
	setValue(gstEndFrame, end_frame);
	setValue(gstCaptureParam, true);
	setValue(gstCapture, true);
	setValue(gstMovSeqNum, begin_frame);
	setValue(gstMovRewind4d, rewind);
}

void Renderview::SetParams(double t)
{
	FlKeyCode keycode;
	keycode.l0 = 1;
	keycode.l0_name = getName();

	//get all volume data under view
	SearchVisitor visitor;
	visitor.matchClassName("VolumeData");
	accept(visitor);
	ObjectList* list = visitor.getResult();
	for (auto i : *list)
	{
		VolumeData* vd = dynamic_cast<VolumeData*>(i);
		if (!vd) continue;

		keycode.l1 = 2;
		keycode.l1_name = vd->getName();

		//display
		keycode.l2 = 0;
		keycode.l2_name = "display";
		bool bval;
		if (m_interpolator->GetBoolean(keycode, t, bval))
			vd->setValue(gstDisplay, bval);

		//clipping planes
		vector<Plane*> *planes = vd->GetRenderer()->get_planes();
		if (!planes) continue;
		if (planes->size() != 6) continue;
		Plane *plane = 0;
		double val = 0;
		//x1
		plane = (*planes)[0];
		keycode.l2 = 0;
		keycode.l2_name = "x1_val";
		if (m_interpolator->GetDouble(keycode, t, val))
			plane->ChangePlane(Point(abs(val), 0.0, 0.0),
				Vector(1.0, 0.0, 0.0));
		//x2
		plane = (*planes)[1];
		keycode.l2 = 0;
		keycode.l2_name = "x2_val";
		if (m_interpolator->GetDouble(keycode, t, val))
			plane->ChangePlane(Point(abs(val), 0.0, 0.0),
				Vector(-1.0, 0.0, 0.0));
		//y1
		plane = (*planes)[2];
		keycode.l2 = 0;
		keycode.l2_name = "y1_val";
		if (m_interpolator->GetDouble(keycode, t, val))
			plane->ChangePlane(Point(0.0, abs(val), 0.0),
				Vector(0.0, 1.0, 0.0));
		//y2
		plane = (*planes)[3];
		keycode.l2 = 0;
		keycode.l2_name = "y2_val";
		if (m_interpolator->GetDouble(keycode, t, val))
			plane->ChangePlane(Point(0.0, abs(val), 0.0),
				Vector(0.0, -1.0, 0.0));
		//z1
		plane = (*planes)[4];
		keycode.l2 = 0;
		keycode.l2_name = "z1_val";
		if (m_interpolator->GetDouble(keycode, t, val))
			plane->ChangePlane(Point(0.0, 0.0, abs(val)),
				Vector(0.0, 0.0, 1.0));
		//z2
		plane = (*planes)[5];
		keycode.l2 = 0;
		keycode.l2_name = "z2_val";
		if (m_interpolator->GetDouble(keycode, t, val))
			plane->ChangePlane(Point(0.0, 0.0, abs(val)),
				Vector(0.0, 0.0, -1.0));
		//t
		double frame;
		keycode.l2 = 0;
		keycode.l2_name = "frame";
		if (m_interpolator->GetDouble(keycode, t, frame))
			UpdateVolumeData(int(frame + 0.5), vd);
	}

	bool bx, by, bz;
	//for the view
	keycode.l1 = 1;
	keycode.l1_name = getName();
	//translation
	double tx, ty, tz;
	keycode.l2 = 0;
	keycode.l2_name = "translation_x";
	bx = m_interpolator->GetDouble(keycode, t, tx);
	keycode.l2_name = "translation_y";
	by = m_interpolator->GetDouble(keycode, t, ty);
	keycode.l2_name = "translation_z";
	bz = m_interpolator->GetDouble(keycode, t, tz);
	if (bx && by && bz)
	{
		setValue(gstCamTransX, tx);
		setValue(gstCamTransY, ty);
		setValue(gstCamTransZ, tz);
		double d = std::sqrt(tx*tx + ty * ty + tz * tz);
	}
	//centers
	keycode.l2_name = "center_x";
	bx = m_interpolator->GetDouble(keycode, t, tx);
	keycode.l2_name = "center_y";
	by = m_interpolator->GetDouble(keycode, t, ty);
	keycode.l2_name = "center_z";
	bz = m_interpolator->GetDouble(keycode, t, tz);
	if (bx && by && bz)
	{
		setValue(gstCamCtrX, tx);
		setValue(gstCamCtrY, ty);
		setValue(gstCamCtrZ, tz);
	}
	//obj translation
	keycode.l2_name = "obj_trans_x";
	bx = m_interpolator->GetDouble(keycode, t, tx);
	keycode.l2_name = "obj_trans_y";
	by = m_interpolator->GetDouble(keycode, t, ty);
	keycode.l2_name = "obj_trans_z";
	bz = m_interpolator->GetDouble(keycode, t, tz);
	if (bx && by && bz)
	{
		setValue(gstObjTransX, tx);
		setValue(gstObjTransY, ty);
		setValue(gstObjTransZ, tz);
	}
	//scale
	double scale;
	keycode.l2_name = "scale";
	if (m_interpolator->GetDouble(keycode, t, scale))
	{
		setValue(gstScaleFactor, scale);
		//m_vrv->UpdateScaleFactor(false);
	}
	//rotation
	keycode.l2 = 0;
	keycode.l2_name = "rotation";
	Quaternion q;
	if (m_interpolator->GetQuaternion(keycode, t, q))
	{
		setValue(gstCamRotQ, q);
		Quaternion zq;
		getValue(gstCamRotZeroQ, zq);
		q *= -zq;
		double rotx, roty, rotz;
		q.ToEuler(rotx, roty, rotz);
		setValue(gstCamRotX, rotx);
		setValue(gstCamRotY, roty);
		setValue(gstCamRotZ, rotz);
		//SetRotations(rotx, roty, rotz, true);
	}
	//intermixing mode
	keycode.l2_name = "volmethod";
	int ival;
	if (m_interpolator->GetInt(keycode, t, ival))
		setValue(gstMixMethod, long(ival));
	//perspective angle
	keycode.l2_name = "aov";
	double aov;
	if (m_interpolator->GetDouble(keycode, t, aov))
	{
		if (aov <= 10)
		{
			setValue(gstPerspective, false);
			//m_vrv->m_aov_text->ChangeValue("Ortho");
			//m_vrv->m_aov_sldr->SetValue(10);
		}
		else
		{
			setValue(gstPerspective, true);
			setValue(gstAov, aov);
		}
	}

	//if (m_frame && clip_view)
	//	clip_view->SetVolumeData(m_frame->GetCurSelVol());
	//if (m_frame)
	//{
	//	m_frame->UpdateTree(m_cur_vol ? m_cur_vol->getName() : "");
	//	int index = interpolator->GetKeyIndexFromTime(t);
	//	m_frame->GetRecorderDlg()->SetSelection(index);
	//}
	setValue(gstVolListDirty, true);
}

//event functions
void Renderview::OnCamRotChanged(Event& event)
{
	double dval;
	getValue(gstCamRotX, dval);
	dval = RoundDeg(dval);
	setValue(gstCamRotX, dval);
	getValue(gstCamRotY, dval);
	dval = RoundDeg(dval);
	setValue(gstCamRotY, dval);
	getValue(gstCamRotZ, dval);
	dval = RoundDeg(dval);
	setValue(gstCamRotZ, dval);
	
	A2Q();

	getValue(gstCamDist, dval);
	Quaternion q;
	getValue(gstCamRotQ, q);
	Quaternion cam_pos(0.0, 0.0, dval, 0.0);
	Quaternion cam_pos2 = (-q) * cam_pos * q;
	setValue(gstCamTransX, cam_pos2.x);
	setValue(gstCamTransY, cam_pos2.y);
	setValue(gstCamTransZ, cam_pos2.z);

	Quaternion up(0.0, 1.0, 0.0, 0.0);
	Quaternion up2 = (-q) * up * q;
	Vector vval(up2.x, up2.y, up2.z);
	setValue(gstCamUp, vval);
}

void Renderview::OnPerspectiveChanged(Event& event)
{
	bool free, persp;
	getValue(gstFree, free);
	getValue(gstPerspective, persp);
	if (free && !persp)
	{
		setValue(gstFree, false);

		//restore camera translation
		double dval;
		getValue(gstCamTransSavedX, dval); setValue(gstCamTransX, dval);
		getValue(gstCamTransSavedY, dval); setValue(gstCamTransY, dval);
		getValue(gstCamTransSavedZ, dval); setValue(gstCamTransZ, dval);
		getValue(gstCamCtrSavedX, dval); setValue(gstCamCtrX, dval);
		getValue(gstCamCtrSavedY, dval); setValue(gstCamCtrY, dval);
		getValue(gstCamCtrSavedZ, dval); setValue(gstCamCtrZ, dval);
		//restore object translation
		getValue(gstObjTransSavedX, dval); setValue(gstObjTransX, dval);
		getValue(gstObjTransSavedY, dval); setValue(gstObjTransY, dval);
		getValue(gstObjTransSavedZ, dval); setValue(gstObjTransZ, dval);
		//restore scale factor
		getValue(gstScaleFactorSaved, dval); setValue(gstScaleFactor, dval);
		//restore camera rotation
		getValue(gstCamRotSavedX, dval); setValue(gstCamRotX, dval);
		getValue(gstCamRotSavedY, dval); setValue(gstCamRotY, dval);
		getValue(gstCamRotSavedZ, dval); setValue(gstCamRotZ, dval);
		//m_vrv->UpdateScaleFactor(false);
		//wxString str = wxString::Format("%.0f", m_scale_factor*100.0);
		//m_vrv->m_scale_factor_sldr->SetValue(m_scale_factor*100);
		//m_vrv->m_scale_factor_text->ChangeValue(str);
		//m_vrv->m_options_toolbar->ToggleTool(VRenderView::ID_FreeChk, false);

		//SetRotations(m_rotx, m_roty, m_rotz);
	}
}

void Renderview::OnVolListDirtyChanged(Event& event)
{
	PopVolumeList();
}

void Renderview::OnMshListDirtyChanged(Event& event)
{
	PopMeshList();
}