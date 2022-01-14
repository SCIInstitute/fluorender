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
#include <VolumeGroup.hpp>
#include <MeshData.hpp>
#include <NodeVisitor.hpp>
#include <Global.hpp>
#include <SearchVisitor.hpp>
#include <VolumeLoader.h>
#include <compatibility.h>
#include <Debug.h>
#include <Animator/Interpolator.h>
#include <Script/ScriptProc.h>
#include <Selection/VolumeSelector.h>
#include <Calculate/VolumeCalculator.h>
#include <Distance/Ruler.h>
#include <Distance/RulerRenderer.h>
#include <Tracking/Cell.h>
#include <Tracking/Tracks.h>
#include <FLIVR/TextureRenderer.h>
#include <FLIVR/VolumeRenderer.h>
#include <FLIVR/ShaderProgram.h>
#include <FLIVR/KernelProgram.h>
#include <FLIVR/ImgShader.h>
#include <FLIVR/VertexArray.h>
#include <FLIVR/TextRenderer.h>
#include <FLIVR/Framebuffer.h>
#include <Formats/base_reader.h>
#include <Formats/brkxml_reader.h>
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
		std::vector<Plane*> *planes = vd->GetRenderer()->get_planes();
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

void Renderview::ResetMovieAngle()
{
	long axis;
	getValue(gstMovRotAxis, axis);
	std::string s;
	switch (axis)
	{
	case 0:
		s = gstCamRotX;
		break;
	case 1:
		s = gstCamRotY;
		break;
	case 2:
		s = gstCamRotZ;
		break;
	}
	double dval;
	getValue(gstMovInitAng, dval);
	setValue(s, dval);

	setValue(gstCapture, false);
	setValue(gstCaptureRot, false);

	RefreshGL(16);
}

void Renderview::StopMovie()
{
	setValue(gstCapture, false);
	setValue(gstCaptureRot, false);
	setValue(gstCaptureTime, false);
	setValue(gstCaptureParam, false);
}

void Renderview::Get4DSeqRange(long &start_frame, long &end_frame)
{
	int i = 0;//counter
	for (auto vd : m_vol_list)
	{
		if (vd && vd->GetReader())
		{
			BaseReader* reader = vd->GetReader();

			long vd_start_frame = 0;
			long vd_end_frame = reader->GetTimeNum() - 1;
			long vd_cur_frame = reader->GetCurTime();

			if (i == 0)
			{
				//first dataset
				start_frame = vd_start_frame;
				end_frame = vd_end_frame;
			}
			else
			{
				//datasets after the first one
				if (vd_end_frame > end_frame)
					end_frame = vd_end_frame;
			}
		}
		i++;
	}
}

void Renderview::Set4DSeqFrame(long frame, long start_frame, long end_frame, bool rewind)
{
	long lval;
	//compute frame number
	setValue(gstBeginFrame, start_frame);
	setValue(gstEndFrame, end_frame);
	lval = std::abs(end_frame - start_frame + 1);
	setValue(gstTotalFrames, lval);
	//skip update if frame num unchanged
	getValue(gstCurrentFrame, lval);
	bool update = lval == frame ? false : true;

	//save currently selected volume
	Referenced* ref;
	getRvalu(gstCurrentVolume, &ref);

	//run pre-change script
	bool bval;
	getValue(gstRunScript, bval);
	std::wstring script_file;
	getValue(gstScriptFile, script_file);
	if (update && bval)
		m_scriptor->Run4DScript(flrd::ScriptProc::TM_ALL_PRE, script_file, rewind);

	//change time frame
	setValue(gstPreviousFrame, lval);
	setValue(gstCurrentFrame, frame);

	if (update)
		for (auto i : m_vol_list)
			UpdateVolumeData(frame, i.get());

	//run post-change script
	if (update && bval)
		m_scriptor->Run4DScript(flrd::ScriptProc::TM_ALL_POST, script_file, rewind);

	//restore currently selected volume
	setRvalu(gstCurrentVolume, ref);
	VolumeData* vd = dynamic_cast<VolumeData*>(ref);
	m_selector->SetVolume(vd);
	m_calculator->SetVolumeA(vd);

	RefreshGL(17);
}

void Renderview::UpdateVolumeData(long frame, VolumeData* vd)
{
	if (vd && vd->GetReader())
	{
		BaseReader* reader = vd->GetReader();
		bool clear_pool = false;

		long cur_time;
		vd->getValue(gstTime, cur_time);
		if (cur_time != frame)
		{
			flvr::Texture *tex = vd->GetTexture();
			if (tex && tex->isBrxml())
			{
				BRKXMLReader *br = (BRKXMLReader *)reader;
				br->SetCurTime(frame);
				int curlv = tex->GetCurLevel();
				for (int j = 0; j < br->GetLevelNum(); j++)
				{
					tex->setLevel(j);
					if (vd->GetRenderer()) vd->GetRenderer()->clear_brick_buf();
				}
				tex->setLevel(curlv);
				long chan;
				vd->getValue(gstChannel, chan);
				tex->set_FrameAndChannel(frame, chan);
				vd->setValue(gstTime, long(reader->GetCurTime()));
				//update rulers
				//if (m_frame && m_frame->GetMeasureDlg())
				//	m_frame->GetMeasureDlg()->UpdateList();
			}
			else
			{
				double spcx, spcy, spcz;
				vd->getValue(gstSpcX, spcx);
				vd->getValue(gstSpcY, spcy);
				vd->getValue(gstSpcZ, spcz);

				long chan;
				vd->getValue(gstChannel, chan);
				Nrrd* data = reader->Convert(frame, chan, false);
				if (!vd->ReplaceData(data, false))
					return;

				vd->setValue(gstTime, long(reader->GetCurTime()));
				vd->setValue(gstSpcX, spcx);
				vd->setValue(gstSpcY, spcy);
				vd->setValue(gstSpcZ, spcz);

				//update rulers
				//if (m_frame && m_frame->GetMeasureDlg())
				//	m_frame->GetMeasureDlg()->UpdateList();

				clear_pool = true;
			}
		}

		if (clear_pool && vd->GetRenderer())
			vd->GetRenderer()->clear_tex_pool();
	}
}

void Renderview::ReloadVolumeData(int frame)
{
	int i = 0; int j;
	std::vector<BaseReader*> reader_list;
	std::wstring bat_folder;

	for (auto vd : m_vol_list)
	{
		if (vd && vd->GetReader())
		{
			flvr::Texture *tex = vd->GetTexture();
			BaseReader* reader = vd->GetReader();
			if (tex && tex->isBrxml())
			{
				BRKXMLReader *br = (BRKXMLReader *)reader;
				int curlv = tex->GetCurLevel();
				for (j = 0; j < br->GetLevelNum(); j++)
				{
					tex->setLevel(j);
					if (vd->GetRenderer()) vd->GetRenderer()->clear_brick_buf();
				}
				tex->setLevel(curlv);
				long chan;
				vd->getValue(gstChannel, chan);
				tex->set_FrameAndChannel(0, chan);
				vd->setValue(gstTime, long(reader->GetCurTime()));
				std::wstring data_name = reader->GetDataName();
				if (i > 0)
					bat_folder += L"_";
				bat_folder += data_name;

				int chan_num = 0;
				if (data_name.find(L"_1ch") != std::wstring::npos)
					chan_num = 1;
				else if (data_name.find(L"_2ch") != std::wstring::npos)
					chan_num = 2;
				if (chan_num > 0 && chan >= chan_num)
					vd->setValue(gstDisplay, false);
				else
					vd->setValue(gstDisplay, true);

				if (reader->GetChanNum() > 1)
					data_name += std::to_wstring(chan + 1);
				vd->setName(ws2s(data_name));
			}
			else
			{
				bool found = false;
				for (j = 0; j < (int)reader_list.size(); j++)
				{
					if (reader == reader_list[j])
					{
						found = true;
						break;
					}
				}
				if (!found)
				{
					reader->LoadOffset(frame);
					reader_list.push_back(reader);
				}

				double spcx, spcy, spcz;
				vd->getValue(gstSpcX, spcx);
				vd->getValue(gstSpcY, spcy);
				vd->getValue(gstSpcZ, spcz);
				long chan;
				vd->getValue(gstChannel, chan);
				Nrrd* data = reader->Convert(0, chan, true);
				if (vd->ReplaceData(data, true))
					vd->setValue(gstDisplay, true);
				else
				{
					vd->setValue(gstDisplay, false);
					continue;
				}

				wxString data_name = wxString(reader->GetDataName());
				if (i > 0)
					bat_folder += L"_";
				bat_folder += data_name;

				int chan_num = 0;
				if (data_name.Find("_1ch") != -1)
					chan_num = 1;
				else if (data_name.Find("_2ch") != -1)
					chan_num = 2;
				if (chan_num > 0 && chan >= chan_num)
					vd->setValue(gstDisplay, false);
				else
					vd->setValue(gstDisplay, true);

				if (reader->GetChanNum() > 1)
					data_name += wxString::Format("_%d", chan + 1);
				vd->setName(data_name.ToStdString());
				vd->setValue(gstDataPath, reader->GetPathName());
				vd->setValue(gstTime, long(reader->GetCurTime()));
				if (!reader->IsSpcInfoValid())
				{
					vd->setValue(gstSpcX, spcx);
					vd->setValue(gstSpcY, spcy);
					vd->setValue(gstSpcZ, spcz);
				}
				else
				{
					vd->setValue(gstSpcX, reader->GetXSpc());
					vd->setValue(gstSpcY, reader->GetYSpc());
					vd->setValue(gstSpcZ, reader->GetZSpc());
				}
				if (vd->GetRenderer())
					vd->GetRenderer()->clear_tex_pool();
			}
		}
		i++;
	}
	//set bat folder name
	setValue(gstBatFolder, bat_folder);

	InitView(INIT_BOUNDS | INIT_CENTER);

	//if (m_frame)
	//{
	//	m_frame->UpdateList();
	//	m_frame->UpdateTree(
	//		m_frame->GetCurSelVol() ?
	//		m_frame->GetCurSelVol()->getName() :
	//		"");
	//}
}

void Renderview::Get3DBatRange(long &start_frame, long &end_frame)
{
	int i = 0;//counter
	std::wstring bat_folder;

	for (auto vd : m_vol_list)
	{
		if (vd && vd->GetReader())
		{
			BaseReader* reader = vd->GetReader();
			reader->SetBatch(true);

			int vd_cur_frame = reader->GetCurBatch();
			int vd_start_frame = -vd_cur_frame;
			int vd_end_frame = reader->GetBatchNum() - 1 - vd_cur_frame;

			if (i > 0)
				bat_folder += L"_";
			bat_folder += reader->GetDataName();

			if (i == 0)
			{
				//first dataset
				start_frame = vd_start_frame;
				end_frame = vd_end_frame;
			}
			else
			{
				//datasets after the first one
				if (vd_start_frame < start_frame)
					start_frame = vd_start_frame;
				if (vd_end_frame > end_frame)
					end_frame = vd_end_frame;
			}
		}
		i++;
	}
	end_frame -= start_frame;
	start_frame = 0;
}

void Renderview::Set3DBatFrame(long frame, long start_frame, long end_frame, bool rewind)
{
	long lval;
	//compute frame number
	setValue(gstBeginFrame, start_frame);
	setValue(gstEndFrame, end_frame);
	lval = std::abs(end_frame - start_frame + 1);
	setValue(gstTotalFrames, lval);
	//skip update if frame num unchanged
	getValue(gstCurrentFrame, lval);
	bool update = lval == frame ? false : true;

	//save currently selected volume
	Referenced* ref;
	getRvalu(gstCurrentVolume, &ref);

	//run pre-change script
	bool bval;
	getValue(gstRunScript, bval);
	std::wstring script_file;
	getValue(gstScriptFile, script_file);
	if (update && bval)
		m_scriptor->Run4DScript(flrd::ScriptProc::TM_ALL_PRE, script_file, rewind);

	//change time frame
	setValue(gstPreviousFrame, lval);
	setValue(gstCurrentFrame, frame);

	if (update)
		ReloadVolumeData(frame);

	//run post-change script
	if (update && bval)
		m_scriptor->Run4DScript(flrd::ScriptProc::TM_ALL_POST, script_file, rewind);

	//restore currently selected volume
	setValue(gstCurrentVolume, ref);
	VolumeData* vd = dynamic_cast<VolumeData*>(ref);
	m_selector->SetVolume(vd);
	m_calculator->SetVolumeA(vd);

	RefreshGL(18);
}

void Renderview::CalculateCrop()
{
	long w, h;
	getValue(gstSizeX, w);
	getValue(gstSizeY, h);

	Referenced* ref;
	getRvalu(gstCurrentVolume, &ref);

	if (ref)
	{
		//projection
		HandleProjection(w, h);
		//Transformation
		HandleCamera();

		glm::mat4 mv_temp = GetDrawMat();
		Transform mv;
		Transform pr;
		mv.set(glm::value_ptr(mv_temp));
		pr.set(glm::value_ptr(m_proj_mat));

		double minx, maxx, miny, maxy;
		minx = 1.0;
		maxx = -1.0;
		miny = 1.0;
		maxy = -1.0;
		std::vector<Point> points;
		BBox bbox;
		VolumeData* vd = dynamic_cast<VolumeData*>(ref);
		vd->getValue(gstBounds, bbox);
		points.push_back(Point(bbox.Min().x(), bbox.Min().y(), bbox.Min().z()));
		points.push_back(Point(bbox.Min().x(), bbox.Min().y(), bbox.Max().z()));
		points.push_back(Point(bbox.Min().x(), bbox.Max().y(), bbox.Min().z()));
		points.push_back(Point(bbox.Min().x(), bbox.Max().y(), bbox.Max().z()));
		points.push_back(Point(bbox.Max().x(), bbox.Min().y(), bbox.Min().z()));
		points.push_back(Point(bbox.Max().x(), bbox.Min().y(), bbox.Max().z()));
		points.push_back(Point(bbox.Max().x(), bbox.Max().y(), bbox.Min().z()));
		points.push_back(Point(bbox.Max().x(), bbox.Max().y(), bbox.Max().z()));

		Point p;
		for (unsigned int i = 0; i < points.size(); ++i)
		{
			p = mv.transform(points[i]);
			p = pr.transform(p);
			minx = p.x() < minx ? p.x() : minx;
			maxx = p.x() > maxx ? p.x() : maxx;
			miny = p.y() < miny ? p.y() : miny;
			maxy = p.y() > maxy ? p.y() : maxy;
		}

		minx = Clamp(minx, -1.0, 1.0);
		maxx = Clamp(maxx, -1.0, 1.0);
		miny = Clamp(miny, -1.0, 1.0);
		maxy = Clamp(maxy, -1.0, 1.0);

		setValue(gstCropX, long((minx + 1.0)*w / 2.0 + 1.0));
		setValue(gstCropY, long((miny + 1.0)*h / 2.0 + 1.0));
		setValue(gstCropW, long((maxx - minx)*w / 2.0 - 1.5));
		setValue(gstCropH, long((maxy - miny)*h / 2.0 - 1.5));

	}
	else
	{
		long size;
		if (w > h)
		{
			size = h;
			setValue(gstCropX, long((w - h) / 2.0));
			setValue(gstCropY, long(0));
		}
		else
		{
			size = w;
			setValue(gstCropX, long(0));
			setValue(gstCropY, long((h - w) / 2.0));
		}
		setValue(gstCropW, size);
		setValue(gstCropH, size);
	}
}

//segment volumes in current view
void Renderview::Segment()
{
	int mode = m_selector->GetMode();
	HandleCamera();
	if (mode == 9)
	{
		long lx, ly;
		getValue(gstMouseClientX, lx);
		getValue(gstMouseClientY, ly);
		m_selector->Segment(lx, ly);
	}
	else
		m_selector->Segment();

	//bool count = false;
	//bool colocal = false;
	//if (mode == 1 ||
	//	mode == 2 ||
	//	mode == 3 ||
	//	mode == 4 ||
	//	mode == 5 ||
	//	mode == 7 ||
	//	mode == 8 ||
	//	mode == 9)
	//{
	//	if (m_paint_count)
	//		count = true;
	//	if (m_paint_colocalize)
	//		colocal = true;
	//}

	////update
	//if (m_frame)
	//{
	//	if (m_frame->GetBrushToolDlg())
	//		m_frame->GetBrushToolDlg()->Update(count ? 0 : 1);
	//	if (colocal && m_frame->GetColocalizationDlg())
	//		m_frame->GetColocalizationDlg()->Colocalize();
	//}
}

void Renderview::ForceDraw()
{
//#ifdef _WIN32
//	if (!m_set_gl)
//	{
//		SetCurrent(*m_glRC);
//		m_set_gl = true;
//		if (m_frame)
//		{
//			for (int i = 0; i < m_frame->GetViewNum(); i++)
//			{
//				VRenderGLView* view = m_frame->GetView(i);
//				if (view && view != this)
//				{
//					view->m_set_gl = false;
//				}
//			}
//		}
//	}
//#endif
//#if defined(_DARWIN) || defined(__linux__)
//	SetCurrent(*m_glRC);
//#endif
	Init();
	//wxPaintDC dc(this);

	bool bval;
	getValue(gstResize, bval);
	if (bval)
		setValue(gstRetainFb, false);

	long nx, ny;
	GetRenderSize(nx, ny);

	PopMeshList();
	long draw_type;
	if (m_msh_list.empty())
		draw_type = 1;
	else
		draw_type = 2;
	setValue(gstDrawType, draw_type);

	setValue(gstDrawing, true);
	PreDraw();

	getValue(gstVrEnable, bval);
	if (bval)
	{
		PrepVRBuffer();
		BindRenderBuffer();
	}

	switch (draw_type)
	{
	case 1:  //draw volumes only
		Draw();
		break;
	case 2:  //draw volumes and meshes with depth peeling
		DrawDP();
		break;
	}

	getValue(gstDrawCamCtr, bval);
	if (bval) DrawCamCtr();
	getValue(gstDrawLegend, bval);
	if (bval) DrawLegend();
	getValue(gstDrawScaleBar, bval);
	if (bval) DrawScaleBar();
	getValue(gstDrawColormap, bval);
	if (bval) DrawColormap();

	PostDraw();

	//draw frame after capture
	getValue(gstDrawCropFrame, bval);
	if (bval) DrawFrame();
	//draw info
	getValue(gstDrawInfo, bval);
	if (bval & INFO_DISP) DrawInfo(nx, ny);

	long int_mode;
	getValue(gstInterMode, int_mode);
	if (int_mode == 2 ||
		int_mode == 7)  //painting mode
	{
		getValue(gstDrawBrush, bval);
		if (bval) DrawBrush();
		getValue(gstPaintEnable, bval);
		if (bval) PaintStroke();//for volume segmentation
		bool bval2;
		getValue(gstPaintDisplay, bval2);
		if (bval && bval2) DisplayStroke();//show the paint strokes
	}
	else if (int_mode == 4)
		setValue(gstInterMode, 2);
	else if (int_mode == 8)
		setValue(gstInterMode, 7);


#ifdef _WIN32
	if (flvr::TextureRenderer::get_invalidate_tex())
	{
//		for (int i = 0; i < m_dp_tex_list.size(); ++i)
//			glInvalidateTexImage(m_dp_tex_list[i], 0);
//		glInvalidateTexImage(m_tex_paint, 0);
//		glInvalidateTexImage(m_tex_final, 0);
//		glInvalidateTexImage(m_tex, 0);
//		glInvalidateTexImage(m_tex_temp, 0);
//		glInvalidateTexImage(m_tex_wt2, 0);
//		glInvalidateTexImage(m_tex_ol1, 0);
//		glInvalidateTexImage(m_tex_ol2, 0);
//		glInvalidateTexImage(m_tex_pick, 0);
//		glInvalidateTexImage(m_tex_pick_depth, 0);
	}
#endif

	//swap
	getValue(gstVrEnable, bval);
	if (bval)
	{
		long lval;
		getValue(gstVrEyeIdx, lval);
		if (lval)
		{
			DrawVRBuffer();
			setValue(gstVrEyeIdx, long(0));
			toggleValue(gstSwapBuffers, bval);
			//SwapBuffers();
		}
		else
		{
			setValue(gstVrEyeIdx, long(1));
			RefreshGL(99);
		}
	}
	else
		toggleValue(gstSwapBuffers, bval);
		//SwapBuffers();

	glbin_timer->sample();
	setValue(gstDrawing, false);

	getValue(gstInteractive, bval);
	DBGPRINT(L"buffer swapped\t%d\n", bval);

	getValue(gstResize, bval);
	if (bval) setValue(gstResize, false);
	getValue(gstEnlarge, bval);
	if (bval)
		ResetEnlarge();

	//if (m_linked_rot)
	//{
	//	if (!m_master_linked_view ||
	//		this != m_master_linked_view)
	//		return;

	//	if (m_frame)
	//	{
	//		for (int i = 0; i < m_frame->GetViewNum(); i++)
	//		{
	//			VRenderGLView* view = m_frame->GetView(i);
	//			if (view && view != this)
	//			{
	//				view->SetRotations(m_rotx, m_roty, m_rotz, true);
	//				view->RefreshGL(39);
	//			}
	//		}
	//	}

	//	m_master_linked_view = 0;
	//}
}

//start loop update
void Renderview::StartLoopUpdate()
{
	////this is for debug_ds, comment when done
	//if (TextureRenderer::get_mem_swap() &&
	//  TextureRenderer::get_start_update_loop() &&
	//  !TextureRenderer::get_done_update_loop())
	//  return;

	if (!flvr::TextureRenderer::get_mem_swap()) return;
	if (flvr::TextureRenderer::active_view_ > 0 &&
		flvr::TextureRenderer::active_view_ != getId()) return;
	else flvr::TextureRenderer::active_view_ = getId();

	long nx, ny;
	GetRenderSize(nx, ny);
	//projection
	HandleProjection(nx, ny, true);
	//Transformation
	HandleCamera(true);
	glm::mat4 mv_temp = m_mv_mat;
	m_mv_mat = GetDrawMat();

	PopVolumeList();
	int total_num = 0;
	int num_chan;
	//reset drawn status for all bricks
	int i, j, k;
	bool persp;
	getValue(gstPerspective, persp);
	for (auto vd : m_vol_list)
	{
		if (!vd) continue;

		switchLevel(vd.get());
		vd->SetMatrices(m_mv_mat, m_proj_mat, m_tex_mat);

		num_chan = 0;
		flvr::Texture* tex = vd->GetTexture();
		if (tex)
		{
			Transform *tform = tex->transform();
			double mvmat[16];
			tform->get_trans(mvmat);
			vd->GetRenderer()->m_mv_mat2 = glm::mat4(
				mvmat[0], mvmat[4], mvmat[8], mvmat[12],
				mvmat[1], mvmat[5], mvmat[9], mvmat[13],
				mvmat[2], mvmat[6], mvmat[10], mvmat[14],
				mvmat[3], mvmat[7], mvmat[11], mvmat[15]);
			vd->GetRenderer()->m_mv_mat2 = vd->GetRenderer()->m_mv_mat * vd->GetRenderer()->m_mv_mat2;

			Ray view_ray = vd->GetRenderer()->compute_view();
			std::vector<flvr::TextureBrick*> *bricks = 0;
			bricks = tex->get_sorted_bricks(view_ray, !persp);
			if (!bricks || bricks->size() == 0)
				continue;
			for (j = 0; j < bricks->size(); j++)
			{
				(*bricks)[j]->set_drawn(false);
				if ((*bricks)[j]->get_priority() > 0 ||
					!vd->GetRenderer()->test_against_view((*bricks)[j]->bbox(), persp))
				{
					(*bricks)[j]->set_disp(false);
					continue;
				}
				else
					(*bricks)[j]->set_disp(true);
				total_num++;
				num_chan++;
				long mip_mode;
				vd->getValue(gstMipMode, mip_mode);
				bool shading;
				vd->getValue(gstShadingEnable, shading);
				if (mip_mode == 1 && shading)
					total_num++;
				bool shadow;
				vd->getValue(gstShadowEnable, shadow);
				if (shadow)
					total_num++;
				//mask
				long label_mode;
				vd->getValue(gstLabelMode, label_mode);
				if (vd->GetTexture() &&
					vd->GetTexture()->nmask() != -1 &&
					(!label_mode ||
					(label_mode &&
						vd->GetTexture()->nlabel() == -1)))
					total_num++;
			}
		}
		vd->setValue(gstBrickNum, long(num_chan));
		if (vd->GetRenderer())
			vd->GetRenderer()->set_done_loop(false);
	}

	std::vector<VolumeLoaderData> queues;
	long mix_method;
	getValue(gstMixMethod, mix_method);
	if (mix_method == MIX_METHOD_MULTI)
	{
		std::vector<VolumeData*> list;
		for (auto vd : m_vol_list)
		{
			if (!vd) continue;
			bool disp, multires;
			vd->getValue(gstDisplay, disp);
			vd->getValue(gstMultires, multires);
			if (!disp || !multires)
				continue;
			flvr::Texture* tex = vd->GetTexture();
			if (!tex)
				continue;
			std::vector<flvr::TextureBrick*> *bricks = tex->get_bricks();
			if (!bricks || bricks->size() == 0)
				continue;
			list.push_back(vd.get());
		}

		std::vector<VolumeLoaderData> tmp_shade;
		std::vector<VolumeLoaderData> tmp_shadow;
		for (auto vd : list)
		{
			if (!vd) continue;
			flvr::Texture* tex = vd->GetTexture();
			Ray view_ray = vd->GetRenderer()->compute_view();
			std::vector<flvr::TextureBrick*> *bricks = tex->get_sorted_bricks(view_ray, !persp);
			long mip_mode;
			vd->getValue(gstMipMode, mip_mode);
			int mode = mip_mode == 1 ? 1 : 0;
			bool shading;
			vd->getValue(gstShadingEnable, shading);
			bool shade = (mode == 1 && shading);
			bool shadow;
			vd->getValue(gstShadowEnable, shadow);
			for (j = 0; j < bricks->size(); j++)
			{
				VolumeLoaderData d;
				flvr::TextureBrick* b = (*bricks)[j];
				if (b->get_disp())
				{
					d.brick = b;
					d.finfo = tex->GetFileName(b->getID());
					d.vd = vd;
					if (!b->drawn(mode))
					{
						d.mode = mode;
						queues.push_back(d);
					}
					if (shade && !b->drawn(2))
					{
						d.mode = 2;
						tmp_shade.push_back(d);
					}
					if (shadow && !b->drawn(3))
					{
						d.mode = 3;
						tmp_shadow.push_back(d);
					}
				}
			}
		}
		if (flvr::TextureRenderer::get_update_order() == 1)
			std::sort(queues.begin(), queues.end(),
				[](const VolumeLoaderData b1, const VolumeLoaderData b2)
		{ return b2.brick->get_d() > b1.brick->get_d(); });
		else if (flvr::TextureRenderer::get_update_order() == 0)
			std::sort(queues.begin(), queues.end(),
				[](const VolumeLoaderData b1, const VolumeLoaderData b2)
		{ return b2.brick->get_d() < b1.brick->get_d(); });

		if (!tmp_shade.empty())
		{
			if (flvr::TextureRenderer::get_update_order() == 1)
				std::sort(tmp_shade.begin(), tmp_shade.end(),
					[](const VolumeLoaderData b1, const VolumeLoaderData b2)
			{ return b2.brick->get_d() > b1.brick->get_d(); });
			else if (flvr::TextureRenderer::get_update_order() == 0)
				std::sort(tmp_shade.begin(), tmp_shade.end(),
					[](const VolumeLoaderData b1, const VolumeLoaderData b2)
			{ return b2.brick->get_d() < b1.brick->get_d(); });
			queues.insert(queues.end(), tmp_shade.begin(), tmp_shade.end());
		}
		if (!tmp_shadow.empty())
		{
			if (flvr::TextureRenderer::get_update_order() == 1)
			{
				int order = flvr::TextureRenderer::get_update_order();
				flvr::TextureRenderer::set_update_order(0);
				for (i = 0; i < list.size(); i++)
				{
					Ray view_ray = list[i]->GetRenderer()->compute_view();
					list[i]->GetTexture()->set_sort_bricks();
					list[i]->GetTexture()->get_sorted_bricks(view_ray, !persp); //recalculate brick.d_
					list[i]->GetTexture()->set_sort_bricks();
				}
				flvr::TextureRenderer::set_update_order(order);
				std::sort(tmp_shadow.begin(), tmp_shadow.end(),
					[](const VolumeLoaderData b1, const VolumeLoaderData b2)
				{ return b2.brick->get_d() > b1.brick->get_d(); });
			}
			else if (flvr::TextureRenderer::get_update_order() == 0)
				std::sort(tmp_shadow.begin(), tmp_shadow.end(),
					[](const VolumeLoaderData b1, const VolumeLoaderData b2)
			{ return b2.brick->get_d() > b1.brick->get_d(); });
			queues.insert(queues.end(), tmp_shadow.begin(), tmp_shadow.end());
		}
	}
	else if (getNumChildren() > 0)
	{
		for (i = getNumChildren() - 1; i >= 0; i--)
		{
			if (VolumeData* vd = getChild(i)->asVolumeData())
			{
				std::vector<VolumeLoaderData> tmp_shade;
				std::vector<VolumeLoaderData> tmp_shadow;
				bool disp, multires;
				vd->getValue(gstDisplay, disp);
				vd->getValue(gstMultires, multires);
				if (disp && multires)
				{
					flvr::Texture* tex = vd->GetTexture();
					if (!tex)
						continue;
					Ray view_ray = vd->GetRenderer()->compute_view();
					std::vector<flvr::TextureBrick*> *bricks = tex->get_sorted_bricks(view_ray, !persp);
					if (!bricks || bricks->size() == 0)
						continue;
					long mip_mode;
					vd->getValue(gstMipMode, mip_mode);
					int mode = mip_mode == 1 ? 1 : 0;
					bool shading;
					vd->getValue(gstShadingEnable, shading);
					bool shade = (mode == 1 && shading);
					bool shadow;
					vd->getValue(gstShadowEnable, shadow);
					for (j = 0; j < bricks->size(); j++)
					{
						VolumeLoaderData d;
						flvr::TextureBrick* b = (*bricks)[j];
						if (b->get_disp())
						{
							d.brick = b;
							d.finfo = tex->GetFileName(b->getID());
							d.vd = vd;
							if (!b->drawn(mode))
							{
								d.mode = mode;
								queues.push_back(d);
							}
							if (shade && !b->drawn(2))
							{
								d.mode = 2;
								tmp_shade.push_back(d);
							}
							if (shadow && !b->drawn(3))
							{
								d.mode = 3;
								tmp_shadow.push_back(d);
							}
						}
					}
					if (!tmp_shade.empty()) queues.insert(queues.end(), tmp_shade.begin(), tmp_shade.end());
					if (!tmp_shadow.empty())
					{
						if (flvr::TextureRenderer::get_update_order() == 1)
						{
							int order = flvr::TextureRenderer::get_update_order();
							flvr::TextureRenderer::set_update_order(0);
							Ray view_ray = vd->GetRenderer()->compute_view();
							tex->set_sort_bricks();
							tex->get_sorted_bricks(view_ray, !persp); //recalculate brick.d_
							tex->set_sort_bricks();
							flvr::TextureRenderer::set_update_order(order);
							std::sort(tmp_shadow.begin(), tmp_shadow.end(),
								[](const VolumeLoaderData b1, const VolumeLoaderData b2)
							{ return b2.brick->get_d() > b1.brick->get_d(); });
						}
						queues.insert(queues.end(), tmp_shadow.begin(), tmp_shadow.end());
					}
				}
			}
			else if (VolumeGroup* group = getChild(i)->asVolumeGroup())//group
			{
				std::vector<VolumeData*> list;
				bool disp;
				group->getValue(gstDisplay, disp);
				if (!disp)
					continue;
				for (j = group->getNumChildren() - 1; j >= 0; j--)
				{
					VolumeData* vd = group->getChild(j)->asVolumeData();
					bool disp, multires;
					vd->getValue(gstDisplay, disp);
					vd->getValue(gstMultires, multires);
					if (!vd || !disp || !multires)
						continue;
					flvr::Texture* tex = vd->GetTexture();
					if (!tex)
						continue;
					Ray view_ray = vd->GetRenderer()->compute_view();
					std::vector<flvr::TextureBrick*> *bricks = tex->get_sorted_bricks(view_ray, !persp);
					if (!bricks || bricks->size() == 0)
						continue;
					list.push_back(vd);
				}
				if (list.empty())
					continue;

				std::vector<VolumeLoaderData> tmp_q;
				std::vector<VolumeLoaderData> tmp_shade;
				std::vector<VolumeLoaderData> tmp_shadow;
				long blend_mode;
				group->getValue(gstBlendMode, blend_mode);
				if (blend_mode == MIX_METHOD_MULTI)
				{
					for (k = 0; k < list.size(); k++)
					{
						VolumeData* vd = list[k];
						flvr::Texture* tex = vd->GetTexture();
						Ray view_ray = vd->GetRenderer()->compute_view();
						std::vector<flvr::TextureBrick*> *bricks = tex->get_sorted_bricks(view_ray, !persp);
						long mip_mode;
						vd->getValue(gstMipMode, mip_mode);
						int mode = mip_mode == 1 ? 1 : 0;
						bool shading;
						vd->getValue(gstShadingEnable, shading);
						bool shade = (mode == 1 && shading);
						bool shadow;
						vd->getValue(gstShadowEnable, shadow);
						for (j = 0; j < bricks->size(); j++)
						{
							VolumeLoaderData d;
							flvr::TextureBrick* b = (*bricks)[j];
							if (b->get_disp())
							{
								d.brick = b;
								d.finfo = tex->GetFileName(b->getID());
								d.vd = vd;
								if (!b->drawn(mode))
								{
									d.mode = mode;
									tmp_q.push_back(d);
								}
								if (shade && !b->drawn(2))
								{
									d.mode = 2;
									tmp_shade.push_back(d);
								}
								if (shadow && !b->drawn(3))
								{
									d.mode = 3;
									tmp_shadow.push_back(d);
								}
							}
						}
					}
					if (!tmp_q.empty())
					{
						if (flvr::TextureRenderer::get_update_order() == 1)
							std::sort(tmp_q.begin(), tmp_q.end(),
								[](const VolumeLoaderData b1, const VolumeLoaderData b2)
						{ return b2.brick->get_d() > b1.brick->get_d(); });
						else if (flvr::TextureRenderer::get_update_order() == 0)
							std::sort(tmp_q.begin(), tmp_q.end(),
								[](const VolumeLoaderData b1, const VolumeLoaderData b2)
						{ return b2.brick->get_d() < b1.brick->get_d(); });
						queues.insert(queues.end(), tmp_q.begin(), tmp_q.end());
					}
					if (!tmp_shade.empty())
					{
						if (flvr::TextureRenderer::get_update_order() == 1)
							std::sort(tmp_shade.begin(), tmp_shade.end(),
								[](const VolumeLoaderData b1, const VolumeLoaderData b2)
						{ return b2.brick->get_d() > b1.brick->get_d(); });
						else if (flvr::TextureRenderer::get_update_order() == 0)
							std::sort(tmp_shade.begin(), tmp_shade.end(),
								[](const VolumeLoaderData b1, const VolumeLoaderData b2)
						{ return b2.brick->get_d() < b1.brick->get_d(); });
						queues.insert(queues.end(), tmp_shade.begin(), tmp_shade.end());
					}
					if (!tmp_shadow.empty())
					{
						if (flvr::TextureRenderer::get_update_order() == 1)
						{
							int order = flvr::TextureRenderer::get_update_order();
							flvr::TextureRenderer::set_update_order(0);
							for (k = 0; k < list.size(); k++)
							{
								Ray view_ray = list[k]->GetRenderer()->compute_view();
								list[i]->GetTexture()->set_sort_bricks();
								list[i]->GetTexture()->get_sorted_bricks(view_ray, !persp); //recalculate brick.d_
								list[i]->GetTexture()->set_sort_bricks();
							}
							flvr::TextureRenderer::set_update_order(order);
							std::sort(tmp_shadow.begin(), tmp_shadow.end(),
								[](const VolumeLoaderData b1, const VolumeLoaderData b2)
							{ return b2.brick->get_d() > b1.brick->get_d(); });
						}
						else if (flvr::TextureRenderer::get_update_order() == 0)
							std::sort(tmp_shadow.begin(), tmp_shadow.end(),
								[](const VolumeLoaderData b1, const VolumeLoaderData b2)
						{ return b2.brick->get_d() > b1.brick->get_d(); });
						queues.insert(queues.end(), tmp_shadow.begin(), tmp_shadow.end());
					}
				}
				else
				{
					for (j = 0; j < list.size(); j++)
					{
						VolumeData* vd = list[j];
						flvr::Texture* tex = vd->GetTexture();
						Ray view_ray = vd->GetRenderer()->compute_view();
						std::vector<flvr::TextureBrick*> *bricks = tex->get_sorted_bricks(view_ray, !persp);
						long mip_mode;
						vd->getValue(gstMipMode, mip_mode);
						int mode = mip_mode == 1 ? 1 : 0;
						bool shading;
						vd->getValue(gstShadingEnable, shading);
						bool shade = (mode == 1 && shading);
						bool shadow;
						vd->getValue(gstShadowEnable, shadow);
						for (k = 0; k < bricks->size(); k++)
						{
							VolumeLoaderData d;
							flvr::TextureBrick* b = (*bricks)[k];
							if (b->get_disp())
							{
								d.brick = b;
								d.finfo = tex->GetFileName(b->getID());
								d.vd = vd;
								if (!b->drawn(mode))
								{
									d.mode = mode;
									queues.push_back(d);
								}
								if (shade && !b->drawn(2))
								{
									d.mode = 2;
									tmp_shade.push_back(d);
								}
								if (shadow && !b->drawn(3))
								{
									d.mode = 3;
									tmp_shadow.push_back(d);
								}
							}
						}
						if (!tmp_shade.empty()) queues.insert(queues.end(), tmp_shade.begin(), tmp_shade.end());
						if (!tmp_shadow.empty())
						{
							if (flvr::TextureRenderer::get_update_order() == 1)
							{
								int order = flvr::TextureRenderer::get_update_order();
								flvr::TextureRenderer::set_update_order(0);
								Ray view_ray = vd->GetRenderer()->compute_view();
								tex->set_sort_bricks();
								tex->get_sorted_bricks(view_ray, !persp); //recalculate brick.d_
								tex->set_sort_bricks();
								flvr::TextureRenderer::set_update_order(order);
								std::sort(tmp_shadow.begin(), tmp_shadow.end(),
									[](const VolumeLoaderData b1, const VolumeLoaderData b2)
								{ return b2.brick->get_d() > b1.brick->get_d(); });
							}
							queues.insert(queues.end(), tmp_shadow.begin(), tmp_shadow.end());
						}
					}
				}
			}
		}
	}

	if (queues.size() > 0)
	//&& !m_interactive)
	{
		m_loader->Set(queues);
		m_loader->SetMemoryLimitByte((long long)flvr::TextureRenderer::mainmem_buf_size_ * 1024LL * 1024LL);
		flvr::TextureRenderer::set_load_on_main_thread(false);
		m_loader->Run();
	}

	long draw_type;
	getValue(gstDrawType, draw_type);
	if (total_num > 0)
	{
		flvr::TextureRenderer::set_update_loop();
		if (draw_type == 1)
			flvr::TextureRenderer::set_total_brick_num(total_num);
		else if (draw_type == 2)
		{
			long lval;
			getValue(gstPeelNum, lval);
			flvr::TextureRenderer::set_total_brick_num(total_num*(lval + 1));
		}
		flvr::TextureRenderer::reset_done_current_chan();
	}
}

//halt loop update
void Renderview::HaltLoopUpdate()
{
	if (flvr::TextureRenderer::get_mem_swap())
		flvr::TextureRenderer::reset_update_loop();
}

//new function to refresh
void Renderview::RefreshGL(int debug_code,
	bool erase,
	bool start_loop)
{
	//m_force_clear = force_clear;
	//m_interactive = interactive;

	//for debugging refresh events
	bool bval;
	getValue(gstInteractive, bval);
	DBGPRINT(L"%d\trefresh\t%d\t%d\n", getId(), debug_code, bval);
	setValue(gstUpdating, true);
	if (start_loop)
		StartLoopUpdate();
	SetSortBricks();
	setValue(gstRefresh, true);
	setValue(gstRefreshErase, erase);
	toggleValue(gstRefreshNotify, bval);
	//Refresh(erase);
}

void Renderview::DrawRulers()
{
	if (m_ruler_list->empty())
		return;
	double width;
	getValue(gstLineWidth, width);
	m_ruler_renderer->SetLineSize(width);
	m_ruler_renderer->Draw();
}

flrd::Ruler* Renderview::GetRuler(unsigned int id)
{
	m_cur_ruler = 0;
	for (auto i : *m_ruler_list)
	{
		if (i && i->Id() == id)
		{
			m_cur_ruler = i;
			break;
		}
	}
	return m_cur_ruler;
}

//draw highlighted comps
void Renderview::DrawCells()
{
	if (m_cell_list->empty())
		return;
	double width = 1.0;
	getValue(gstLineWidth, width);
	//if (m_frame && m_frame->GetSettingDlg())
	//	width = m_frame->GetSettingDlg()->GetLineWidth();
	long lx, ly;
	getValue(gstSizeX, lx);
	getValue(gstSizeY, ly);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	flvr::ShaderProgram* shader =
		flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHDR_DRAW_THICK_LINES);
	if (shader)
	{
		if (!shader->valid())
			shader->create();
		shader->bind();
	}
	glm::mat4 matrix = glm::ortho(float(0), float(lx), float(0), float(ly));
	shader->setLocalParamMatrix(0, glm::value_ptr(matrix));
	shader->setLocalParam(0, lx, ly, width, 0.0);

	flvr::VertexArray* va_rulers =
		flvr::TextureRenderer::vertex_array_manager_.vertex_array(flvr::VA_Rulers);
	if (va_rulers)
	{
		std::vector<float> verts;
		unsigned int num = DrawCellVerts(verts);
		if (num)
		{
			va_rulers->buffer_data(flvr::VABuf_Coord,
				sizeof(float)*verts.size(),
				&verts[0], GL_STREAM_DRAW);
			va_rulers->set_param(0, num);
			va_rulers->draw();
		}
	}

	if (shader && shader->valid())
		shader->release();
}

unsigned int Renderview::DrawCellVerts(std::vector<float>& verts)
{
	float w = flvr::TextRenderer::text_texture_manager_.GetSize() / 4.0f;
	float px, py;

	Transform mv;
	Transform p;
	mv.set(glm::value_ptr(m_mv_mat));
	p.set(glm::value_ptr(m_proj_mat));

	//estimate
	int vert_num = m_cell_list->size();
	verts.reserve(vert_num * 8 * 3 * 2);

	unsigned int num = 0;
	Point p1, p2, p3, p4;
	Color c;
	getValue(gstTextColor, c);
	double sx, sy, sz;
	sx = m_cell_list->sx;
	sy = m_cell_list->sy;
	sz = m_cell_list->sz;
	for (auto it = m_cell_list->begin();
		it != m_cell_list->end(); ++it)
	{
		BBox box = it->second->GetBox(sx, sy, sz);
		GetCellPoints(box, p1, p2, p3, p4, mv, p);

		verts.push_back(p1.x()); verts.push_back(p1.y()); verts.push_back(0.0);
		verts.push_back(c.r()); verts.push_back(c.g()); verts.push_back(c.b());
		verts.push_back(p2.x()); verts.push_back(p2.y()); verts.push_back(0.0);
		verts.push_back(c.r()); verts.push_back(c.g()); verts.push_back(c.b());
		verts.push_back(p2.x()); verts.push_back(p2.y()); verts.push_back(0.0);
		verts.push_back(c.r()); verts.push_back(c.g()); verts.push_back(c.b());
		verts.push_back(p3.x()); verts.push_back(p3.y()); verts.push_back(0.0);
		verts.push_back(c.r()); verts.push_back(c.g()); verts.push_back(c.b());
		verts.push_back(p3.x()); verts.push_back(p3.y()); verts.push_back(0.0);
		verts.push_back(c.r()); verts.push_back(c.g()); verts.push_back(c.b());
		verts.push_back(p4.x()); verts.push_back(p4.y()); verts.push_back(0.0);
		verts.push_back(c.r()); verts.push_back(c.g()); verts.push_back(c.b());
		verts.push_back(p4.x()); verts.push_back(p4.y()); verts.push_back(0.0);
		verts.push_back(c.r()); verts.push_back(c.g()); verts.push_back(c.b());
		verts.push_back(p1.x()); verts.push_back(p1.y()); verts.push_back(0.0);
		verts.push_back(c.r()); verts.push_back(c.g()); verts.push_back(c.b());
		num += 8;
	}

	return num;
}

void Renderview::GetCellPoints(BBox& box,
	Point& p1, Point& p2, Point& p3, Point& p4,
	Transform& mv, Transform& p)
{
	//get 6 points of the jack of bbox
	Point pp[6];
	Point c = box.center();
	pp[0] = Point(box.Min().x(), c.y(), c.z());
	pp[1] = Point(box.Max().x(), c.y(), c.z());
	pp[2] = Point(c.x(), box.Min().y(), c.z());
	pp[3] = Point(c.x(), box.Max().y(), c.z());
	pp[4] = Point(c.x(), c.y(), box.Min().z());
	pp[5] = Point(c.x(), c.y(), box.Max().z());

	double minx = std::numeric_limits<double>::max();
	double maxx = -std::numeric_limits<double>::max();
	double miny = std::numeric_limits<double>::max();
	double maxy = -std::numeric_limits<double>::max();

	for (int i = 0; i < 6; ++i)
	{
		pp[i] = mv.transform(pp[i]);
		pp[i] = p.transform(pp[i]);
		minx = std::min(minx, pp[i].x());
		maxx = std::max(maxx, pp[i].x());
		miny = std::min(miny, pp[i].y());
		maxy = std::max(maxy, pp[i].y());
	}

	long lx, ly;
	getValue(gstSizeX, lx);
	getValue(gstSizeY, ly);

	p1 = Point((minx + 1)*lx / 2, (miny + 1)*ly / 2, 0.0);
	p2 = Point((maxx + 1)*lx / 2, (miny + 1)*ly / 2, 0.0);
	p3 = Point((maxx + 1)*lx / 2, (maxy + 1)*ly / 2, 0.0);
	p4 = Point((minx + 1)*lx / 2, (maxy + 1)*ly / 2, 0.0);
}

void Renderview::CreateTraceGroup()
{
	if (m_trace_group)
		delete m_trace_group;

	m_trace_group = new flrd::Tracks;
}

int Renderview::LoadTraceGroup(const std::wstring &filename)
{
	if (m_trace_group)
		delete m_trace_group;

	m_trace_group = new flrd::Tracks;
	return m_trace_group->LoadData(filename);
}

int Renderview::SaveTraceGroup(const std::wstring &filename)
{
	if (m_trace_group)
		return m_trace_group->SaveData(filename);
	else
		return 0;
}

void Renderview::DrawTraces()
{
	Referenced* ref;
	getRvalu(gstCurrentVolume, &ref);
	VolumeData* vd = dynamic_cast<VolumeData*>(ref);
	if (!vd || !m_trace_group) return;

	double width = 1.0;
	getValue(gstLineWidth, width);
	//if (m_frame && m_frame->GetSettingDlg())
	//	width = m_frame->GetSettingDlg()->GetLineWidth();
	long lx, ly;
	getValue(gstSizeX, lx);
	getValue(gstSizeY, ly);

	//glEnable(GL_LINE_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	double spcx, spcy, spcz;
	vd->getValue(gstSpcX, spcx);
	vd->getValue(gstSpcY, spcy);
	vd->getValue(gstSpcZ, spcz);
	glm::mat4 matrix = glm::scale(m_mv_mat,
		glm::vec3(float(spcx), float(spcy), float(spcz)));
	matrix = m_proj_mat * matrix;

	flvr::ShaderProgram* shader =
		flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHDR_DRAW_THICK_LINES);
	if (shader)
	{
		if (!shader->valid())
			shader->create();
		shader->bind();
	}
	shader->setLocalParamMatrix(0, glm::value_ptr(matrix));
	shader->setLocalParam(0, lx, ly, width, 0.0);

	flvr::VertexArray* va_traces =
		flvr::TextureRenderer::vertex_array_manager_.vertex_array(flvr::VA_Traces);
	if (va_traces)
	{
		if (va_traces->get_dirty())
		{
			std::vector<float> verts;
			unsigned int num = m_trace_group->Draw(verts, vd->GetShuffle());
			if (num)
			{
				va_traces->buffer_data(flvr::VABuf_Coord,
					sizeof(float)*verts.size(),
					&verts[0], GL_STREAM_DRAW);
				va_traces->set_param(0, num);
				va_traces->draw();
			}
		}
		else
			va_traces->draw();
	}

	if (shader && shader->valid())
		shader->release();
	//glDisable(GL_LINE_SMOOTH);

}

void Renderview::GetTraces(bool update)
{
	Referenced* ref;
	getRvalu(gstCurrentVolume, &ref);
	VolumeData* vd = dynamic_cast<VolumeData*>(ref);
	if (!vd || !m_trace_group) return;

	int ii, jj, kk;
	long nx, ny, nz;
	//return current mask (into system memory)
	vd->GetRenderer()->return_mask();
	vd->getValue(gstResX, nx);
	vd->getValue(gstResY, ny);
	vd->getValue(gstResZ, nz);
	//find labels in the old that are selected by the current mask
	Nrrd* mask_nrrd = vd->GetMask(true);
	if (!mask_nrrd) return;
	Nrrd* label_nrrd = vd->GetLabel(true);
	if (!label_nrrd) return;
	unsigned char* mask_data = (unsigned char*)(mask_nrrd->data);
	if (!mask_data) return;
	unsigned int* label_data = (unsigned int*)(label_nrrd->data);
	if (!label_data) return;
	flrd::CelpList sel_labels;
	flrd::CelpListIter label_iter;
	for (ii = 0; ii < nx; ii++)
	for (jj = 0; jj < ny; jj++)
	for (kk = 0; kk < nz; kk++)
	{
		int index = nx * ny*kk + nx * jj + ii;
		unsigned int label_value = label_data[index];
		if (mask_data[index] && label_value)
		{
			label_iter = sel_labels.find(label_value);
			if (label_iter == sel_labels.end())
			{
				flrd::Celp cell(new flrd::Cell(label_value));
				cell->Inc(ii, jj, kk, 1.0f);
				sel_labels.insert(std::pair<unsigned int, flrd::Celp>
					(label_value, cell));
			}
			else
			{
				label_iter->second->Inc(ii, jj, kk, 1.0f);
			}
		}
	}

	//create id list
	long lval;
	getValue(gstCurrentFrame, lval);
	m_trace_group->SetCurTime(lval);
	m_trace_group->SetPrvTime(lval);
	m_trace_group->UpdateCellList(sel_labels);
	//m_trace_group->SetPrvTime(m_tseq_prv_num);

	//add traces to trace dialog
	//if (update)
	//{
	//	if (m_vrv && m_frame && m_frame->GetTraceDlg())
	//		m_frame->GetTraceDlg()->GetSettings(m_vrv->m_glview);
	//}
}

//read pixels
void Renderview::ReadPixels(long chann, bool fp32,
	long &x, long &y, long &w, long &h, void** image)
{
	bool bval;
	bool enlarge;
	long lx, ly, lw, lh;
	getValue(gstDrawCropFrame, enlarge);
	if (enlarge)
	{
		getValue(gstCropX, lx);
		getValue(gstCropY, ly);
		getValue(gstCropW, lw);
		getValue(gstCropH, lh);
		getValue(gstEnlarge, bval);
		if (bval)
		{
			double dval;
			getValue(gstEnlargeScale, dval);
			x = long(lx * dval + 0.5);
			y = long(ly * dval + 0.5);
			w = long(lw * dval + 0.5);
			h = long(lh * dval + 0.5);
		}
		else
		{
			x = lx;
			y = ly;
			w = lw;
			h = lh;
		}
	}
	else
	{
		getValue(gstSizeX, lw);
		getValue(gstSizeY, lh);
		x = 0;
		y = 0;
		w = lw;
		h = lh;
	}

	if (enlarge || fp32)
	{
		glActiveTexture(GL_TEXTURE0);
		flvr::Framebuffer* final_buffer =
			flvr::TextureRenderer::framebuffer_manager_.framebuffer(
				"final");
		if (final_buffer)
		{
			//draw the final buffer to itself
			final_buffer->bind();
			final_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
		}
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_DEPTH_TEST);

		//2d adjustment
		flvr::ShaderProgram* img_shader =
			flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHDR_BLEND_BRIGHT_BACKGROUND_HDR);
		if (img_shader)
		{
			if (!img_shader->valid())
				img_shader->create();
			img_shader->bind();
		}
		double dr, dg, db;
		getValue(gstGammaR, dr); getValue(gstGammaG, dg); getValue(gstGammaB, db);
		img_shader->setLocalParam(0, dr, dg, db, 1.0);
		getValue(gstBrightnessR, dr); getValue(gstBrightnessG, dg); getValue(gstBrightnessB, db);
		img_shader->setLocalParam(1, dr, dg, db, 1.0);
		getValue(gstEqualizeR, dr); getValue(gstEqualizeG, dg); getValue(gstEqualizeB, db);
		img_shader->setLocalParam(2, dr, dg, db, 0.0);
		//2d adjustment

		DrawViewQuad();

		if (img_shader && img_shader->valid())
			img_shader->release();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glPixelStorei(GL_PACK_ROW_LENGTH, w);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	if (fp32)
		*image = new float[w*h*chann];
	else
		*image = new unsigned char[w*h*chann];
	glReadBuffer(GL_BACK);
	glReadPixels(x, y, w, h,
		chann == 3 ? GL_RGB : GL_RGBA,
		fp32 ? GL_FLOAT : GL_UNSIGNED_BYTE, *image);
	glPixelStorei(GL_PACK_ROW_LENGTH, 0);

	if (enlarge || fp32)
		BindRenderBuffer();
}

void Renderview::UpdateClips()
{
	long lval;
	getValue(gstClipMode, lval);
	Quaternion q;
	getValue(gstClipRotQ, q);
	if (lval == 1)
	{
		double dx, dy, dz;
		getValue(gstCamRotX, dx); getValue(gstCamRotY, dy); getValue(gstCamRotZ, dz);
		q.FromEuler(dx, -dy, -dz);
		setValue(gstClipRotQ, q);
	}

	std::vector<Plane*> *planes = 0;
	for (auto vd : m_vol_list)
	{
		if (!vd)
			continue;

		double spcx, spcy, spcz;
		long resx, resy, resz;
		vd->getValue(gstSpcX, spcx);
		vd->getValue(gstSpcY, spcy);
		vd->getValue(gstSpcZ, spcz);
		vd->getValue(gstResX, resx);
		vd->getValue(gstResY, resy);
		vd->getValue(gstResZ, resz);
		Vector scale;
		if (spcx > 0.0 && spcy > 0.0 && spcz > 0.0)
		{
			scale = Vector(1.0 / resx / spcx, 1.0 / resy / spcy, 1.0 / resz / spcz);
			scale.safe_normalize();
		}
		else
			scale = Vector(1.0, 1.0, 1.0);

		if (vd->GetRenderer())
			planes = vd->GetRenderer()->get_planes();
		if (planes && planes->size() == 6)
		{
			double x1, x2, y1, y2, z1, z2;
			double abcd[4];

			(*planes)[0]->get_copy(abcd);
			x1 = fabs(abcd[3]);
			(*planes)[1]->get_copy(abcd);
			x2 = fabs(abcd[3]);
			(*planes)[2]->get_copy(abcd);
			y1 = fabs(abcd[3]);
			(*planes)[3]->get_copy(abcd);
			y2 = fabs(abcd[3]);
			(*planes)[4]->get_copy(abcd);
			z1 = fabs(abcd[3]);
			(*planes)[5]->get_copy(abcd);
			z2 = fabs(abcd[3]);

			Vector trans1(-0.5, -0.5, -0.5);
			Vector trans2(0.5, 0.5, 0.5);

			(*planes)[0]->Restore();
			(*planes)[0]->Translate(trans1);
			(*planes)[0]->Rotate(q);
			(*planes)[0]->Scale(scale);
			(*planes)[0]->Translate(trans2);

			(*planes)[1]->Restore();
			(*planes)[1]->Translate(trans1);
			(*planes)[1]->Rotate(q);
			(*planes)[1]->Scale(scale);
			(*planes)[1]->Translate(trans2);

			(*planes)[2]->Restore();
			(*planes)[2]->Translate(trans1);
			(*planes)[2]->Rotate(q);
			(*planes)[2]->Scale(scale);
			(*planes)[2]->Translate(trans2);

			(*planes)[3]->Restore();
			(*planes)[3]->Translate(trans1);
			(*planes)[3]->Rotate(q);
			(*planes)[3]->Scale(scale);
			(*planes)[3]->Translate(trans2);

			(*planes)[4]->Restore();
			(*planes)[4]->Translate(trans1);
			(*planes)[4]->Rotate(q);
			(*planes)[4]->Scale(scale);
			(*planes)[4]->Translate(trans2);

			(*planes)[5]->Restore();
			(*planes)[5]->Translate(trans1);
			(*planes)[5]->Rotate(q);
			(*planes)[5]->Scale(scale);
			(*planes)[5]->Translate(trans2);
		}
	}
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

void Renderview::DrawGrid()
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
	Color text_color;
	getValue(gstTextColor, text_color);
	shader->setLocalParam(0, text_color.r(), text_color.g(), text_color.b(), 1.0);
	glm::mat4 matrix = m_proj_mat * m_mv_mat;
	shader->setLocalParamMatrix(0, glm::value_ptr(matrix));

	flvr::VertexArray* va_grid =
		flvr::TextureRenderer::vertex_array_manager_.vertex_array(flvr::VA_Grid);
	if (va_grid)
	{
		//set parameters
		std::vector<std::pair<unsigned int, double>> params;
		params.push_back(std::pair<unsigned int, double>(0, 5.0));
		double dist;
		getValue(gstCamDist, dist);
		params.push_back(std::pair<unsigned int, double>(1, dist));
		va_grid->set_param(params);
		va_grid->draw();
	}

	if (shader && shader->valid())
		shader->release();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
}

void Renderview::DrawCamCtr()
{
	flvr::VertexArray* va_jack =
		flvr::TextureRenderer::vertex_array_manager_.vertex_array(flvr::VA_Cam_Jack);
	if (!va_jack)
		return;
	double len, camctr_size, dist, aov;
	getValue(gstCamCtrSize, camctr_size);
	getValue(gstCamDist, dist);
	getValue(gstAov, aov);
	if (camctr_size > 0.0)
		len = dist * std::tan(d2r(aov / 2.0))*camctr_size / 10.0;
	else
		len = std::abs(camctr_size);
	bool bval;
	getValue(gstPinRotCtr, bval);
	if (bval)
		len /= 10.0;
	va_jack->set_param(0, len);

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
	glm::mat4 matrix = m_proj_mat * m_mv_mat;
	shader->setLocalParamMatrix(0, glm::value_ptr(matrix));

	va_jack->draw_begin();
	shader->setLocalParam(0, 1.0, 0.0, 0.0, 1.0);
	va_jack->draw_cam_jack(0);
	shader->setLocalParam(0, 0.0, 1.0, 0.0, 1.0);
	va_jack->draw_cam_jack(1);
	shader->setLocalParam(0, 0.0, 0.0, 1.0, 1.0);
	va_jack->draw_cam_jack(2);
	va_jack->draw_end();

	if (shader && shader->valid())
		shader->release();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
}

void Renderview::DrawScaleBar()
{
	flvr::VertexArray* va_scale_bar =
		flvr::TextureRenderer::vertex_array_manager_.vertex_array(flvr::VA_Scale_Bar);
	if (!va_scale_bar)
		return;

	//if (m_draw_legend)
	//	offset = m_sb_height;
	long nx, ny;
	GetRenderSize(nx, ny);
	float sx, sy;
	sx = 2.0 / nx;
	sy = 2.0 / ny;
	float px, py, ph;
	glm::mat4 proj_mat = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f);
	double sb_length, ortho_right, ortho_left;
	getValue(gstScaleBarLen, sb_length);
	getValue(gstOrthoLeft, ortho_left);
	getValue(gstOrthoRight, ortho_right);
	double len = sb_length / (ortho_right - ortho_left);
	std::wstring wsb_text;
	getValue(gstScaleBarText, wsb_text);
	double textlen =
		m_text_renderer->RenderTextLen(wsb_text);
	fluo::Color text_color;
	getValue(gstTextColor, text_color);
	double font_height =
		flvr::TextRenderer::text_texture_manager_.GetSize() + 3.0;

	std::vector<std::pair<unsigned int, double>> params;
	double dx, dy;
	getValue(gstScaleBarPosX, dx);
	getValue(gstScaleBarPosY, dy);
	bool bval;
	getValue(gstDrawCropFrame, bval);
	if (bval)
	{
		long framew, frameh, framex, framey;
		getValue(gstCropX, framex);
		getValue(gstCropY, framey);
		getValue(gstCropW, framew);
		getValue(gstCropH, frameh);
		getValue(gstEnlarge, bval);
		double enlarge_scale = 1;
		if (bval)
		{
			getValue(gstEnlargeScale, enlarge_scale);
			framew *= enlarge_scale;
			frameh *= enlarge_scale;
			framex *= enlarge_scale;
			framey *= enlarge_scale;
		}
		px = (framex + framew - font_height + dx) / nx;
		py = (1.1 * font_height + framey + dy) / ny;
		ph = 5.0 / ny;
		if (bval)
			ph *= enlarge_scale;
		params.push_back(std::pair<unsigned int, double>(0, px));
		params.push_back(std::pair<unsigned int, double>(1, py));
		params.push_back(std::pair<unsigned int, double>(2, len));
		params.push_back(std::pair<unsigned int, double>(3, ph));

		getValue(gstDrawScaleBarText, bval);
		if (bval)
		{
			px = px * nx - 0.5 * (len * nx + textlen + nx) + dx;
			py = py * ny + 0.5 * font_height - ny / 2.0 + dy;
			m_text_renderer->RenderText(
				wsb_text, text_color,
				px*sx, py*sy, sx, sy);
		}
	}
	else
	{
		px = (nx - font_height + dx) / nx;
		py = (1.1 * font_height + dy) / ny;
		ph = 5.0 / ny;
		getValue(gstEnlarge, bval);
		if (bval)
		{
			double dval;
			getValue(gstEnlargeScale, dval);
			ph *= dval;
		}
		params.push_back(std::pair<unsigned int, double>(0, px));
		params.push_back(std::pair<unsigned int, double>(1, py));
		params.push_back(std::pair<unsigned int, double>(2, len));
		params.push_back(std::pair<unsigned int, double>(3, ph));

		getValue(gstDrawScaleBarText, bval);
		if (bval)
		{
			px = px * nx - 0.5 * (len * nx + textlen + nx) + dx;
			py = (py - 0.5) * ny + 0.5 * font_height + dy;
			m_text_renderer->RenderText(
				wsb_text, text_color,
				px*sx, py*sy, sx, sy);
		}
	}

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
	shader->setLocalParamMatrix(0, glm::value_ptr(proj_mat));
	shader->setLocalParam(0, text_color.r(), text_color.g(), text_color.b(), 1.0);
	va_scale_bar->set_param(params);
	va_scale_bar->draw();

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


//event functions/////////////////////////////////////////////////////////////////////////////////
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