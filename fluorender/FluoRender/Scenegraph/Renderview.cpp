/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2022 Scientific Computing and Imaging Institute,
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
#include <Global.hpp>
#include <AgentFactory.hpp>
#include <ClipPlaneAgent.hpp>
#include <ColocalAgent.hpp>
#include <Group.hpp>
#include <Timer.hpp>
#include <Input.hpp>
#include <Root.hpp>
#include <Annotations.hpp>
#include <RenderviewFactory.hpp>
#include <VolumeFactory.hpp>
#include <MeshFactory.hpp>
#include <NodeVisitor.hpp>
#include <SearchVisitor.hpp>
#include <VolumeLoader.h>
#include <compatibility.h>
#include <Debug.h>
#include <Animator/Interpolator.h>
#include <Script/ScriptProc.h>
#include <Selection/VolumeSelector.h>
#include <Selection/VolumePoint.h>
#include <Calculate/VolumeCalculator.h>
#include <Calculate/KernelExecutor.h>
#include <Distance/Ruler.h>
#include <Distance/RulerRenderer.h>
#include <Distance/RulerHandler.h>
#include <Distance/Cov.h>
#include <Distance/SegGrow.h>
#include <Tracking/Tracks.h>
#include <FLIVR/TextureRenderer.h>
#include <FLIVR/VolumeRenderer.h>
#include <FLIVR/MultiVolumeRenderer.h>
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
#include <array>

using namespace fluo;

#ifdef _WIN32
HCTX Renderview::m_hTab = 0;
LOGCONTEXTA Renderview::m_lc;
#endif

Renderview::Renderview()
{
	m_cur_ruler = 0;
	m_trace_group = 0;
	m_mvr = 0;
	//temporary, dynamic data will be managed by global
	m_ruler_list = new flrd::RulerList();
	m_kernel_executor = new flrd::KernelExecutor();
	m_selector = new flrd::VolumeSelector();
	m_calculator = new flrd::VolumeCalculator();
	m_scriptor = new flrd::ScriptProc();
	m_ruler_handler = new flrd::RulerHandler();
	m_ruler_renderer = new flrd::RulerRenderer();
	m_volume_point = new flrd::VolumePoint();
	m_loader = new VolumeLoader();
	m_interpolator = new Interpolator();
	m_text_renderer = new flvr::TextRenderer();
}

Renderview::Renderview(const Renderview& view, const CopyOp& copyop) :
	Group(view, copyop)
{
	m_cur_ruler = 0;
	m_trace_group = 0;
	m_mvr = 0;
	//temporary, dynamic data will be managed by global
	m_ruler_list = new flrd::RulerList();
	m_kernel_executor = new flrd::KernelExecutor();
	m_selector = new flrd::VolumeSelector();
	m_calculator = new flrd::VolumeCalculator();
	m_scriptor = new flrd::ScriptProc();
	m_ruler_handler = new flrd::RulerHandler();
	m_ruler_renderer = new flrd::RulerRenderer();
	m_volume_point = new flrd::VolumePoint();
	m_loader = new VolumeLoader();
	m_interpolator = new Interpolator();
	m_text_renderer = new flvr::TextRenderer();
}

Renderview::~Renderview()
{
#ifdef _WIN32
	//tablet
	if (m_hTab)
	{
		gpWTClose(m_hTab);
		m_hTab = 0;
		UnloadWintab();
	}
	bool enable_vr, enable_openvr;
	getValue(gstVrEnable, enable_vr);
	getValue(gstOpenvrEnable, enable_openvr);
	if (enable_vr && enable_openvr)
	{
		//vr shutdown
		vr::VR_Shutdown();
		//UnloadVR();
	}
	//if (m_controller)
	//	delete m_controller;
#endif
	bool bval;
	getValue(gstBenchmark, bval);
	if (bval)
	{
		unsigned long long ullval = (unsigned long long)(glbin_timer->total_time() * 1000.0);
		setValue(gstBmRuntime, ullval);
		ullval = glbin_timer->count();
		setValue(gstBmFrames, ullval);
		double dval = glbin_timer->total_fps();
		setValue(gstBmFps, dval);
	}
	//glbin_timer->stop();
	//m_selector->SaveBrushSettings();
	m_loader->StopAll();

	//temporary, dynamic data will be managed by global
	delete m_mvr;
	delete m_trace_group;
	for (auto i : *m_ruler_list)
		delete i;
	delete m_ruler_list;
	delete m_kernel_executor;
	delete m_selector;
	delete m_calculator;
	delete m_scriptor;
	delete m_ruler_handler;
	delete m_ruler_renderer;
	delete m_volume_point;
	delete m_loader;
	delete m_interpolator;
	delete m_text_renderer;
}

VolumeGroup* Renderview::addVolumeGroup(const std::string &group_name, const std::string &prv_group_name)
{
	VolumeGroup* group = glbin_volf->buildGroup();
	if (group && group_name != "")
		group->setName(group_name);
	VolumeGroup* prv_group = dynamic_cast<VolumeGroup*>(glbin.get(prv_group_name, glbin_root));
	if (!insertChildAfter(prv_group, group))
		addChild(group);
	return group;
}

VolumeGroup* Renderview::addVolumeData(VolumeData* vd, const std::string &group_name)
{
	Object* obj = glbin.get(group_name, this);
	VolumeGroup* group = dynamic_cast<VolumeGroup*>(obj);
	addVolumeData(vd, group);
	if (group)
		group->setName(group_name);
	return group;
}

VolumeGroup* Renderview::addVolumeData(VolumeData* vd, VolumeGroup* group)
{
	if (!group)
	{
		group = glbin_volf->buildGroup();
		if (!group)
			return 0;
		addChild(group);
	}

	/*for (i=0; i<1; i++)
	{
	VolumeData* vol_data = group->GetVolumeData(i);
	if (vol_data)
	{
	double spcx, spcy, spcz;
	vol_data->GetSpacings(spcx, spcy, spcz);
	vd->SetSpacings(spcx, spcy, spcz);
	}
	}*/

	group->addChild(vd);

	if (group && vd)
	{
		//Color gamma = group->GetGamma();
		//vd->SetGamma(gamma);
		//Color brightness = group->GetBrightness();
		//vd->SetBrightness(brightness);
		//Color hdr = group->GetHdr();
		//vd->SetHdr(hdr);
		//bool sync_r = group->GetSyncR();
		//vd->SetSyncR(sync_r);
		//bool sync_g = group->GetSyncG();
		//vd->SetSyncG(sync_g);
		//bool sync_b = group->GetSyncB();
		//vd->SetSyncB(sync_b);

		//if (m_frame)
		//{
		//	m_frame->GetAdjustView()->SetVolumeData(vd);
		//	m_frame->GetAdjustView()->SetGroupLink(group);
		//}
	}

	updValue(gstVolListDirty, true, Event());
	updValue(gstFullVolListDirty, true, Event());

	//if (m_frame)
	//{
	//	OutAdjustPanel* adjust_view = m_frame->GetAdjustView();
	//	if (adjust_view)
	//	{
	//		adjust_view->SetGroupLink(group);
	//		adjust_view->UpdateSync();
	//	}
	//}

	setValue(gstLoadUpdate, true);

	return group;
}

MeshGroup* Renderview::addMeshGroup(const std::string &group_name)
{
	MeshGroup* group = glbin_mshf->buildGroup();
	if (group && group_name != "")
		group->setName(group_name);
	addChild(group);
	return group;
}

MeshGroup* Renderview::addMeshData(MeshData* md, MeshGroup* group)
{
	if (!group)
	{
		group = glbin_mshf->buildGroup();
		if (!group)
			return nullptr;
		addChild(group);
	}

	group->addChild(md);

	setValue(gstMshListDirty, true);
	setValue(gstFullMshListDirty, true);

	return group;
}

void Renderview::addAnnotations(Annotations* an)
{
	addChild(an);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
void Renderview::Init()
{
	bool bval;
	getValue(gstInitialized, bval);
	if (!bval)
	{
		m_controller = 0;
#ifdef _WIN32
		//tablet initialization
		if (m_selector->GetBrushUsePres())
		{
			if (!m_hTab && LoadWintab() &&
				gpWTInfoA(0, 0, NULL))
			{
				unsigned long long hwnd, hinst;
				getValue(gstHwnd, hwnd);
				getValue(gstHinstance, hinst);
				m_hTab = TabletInit((HWND)hwnd, (HINSTANCE)hinst);
			}
			else
				m_selector->SetBrushUsePres(false);
		}

		//xbox controller
#ifdef USE_XINPUT
		m_controller = new XboxController(1);
#endif
#endif
		m_selector->LoadBrushSettings();

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
			setValue(gstNearClip, near_clip);
			setValue(gstFarClip, far_clip);
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
		setValue(gstCamTransX, double(0));
		setValue(gstCamTransY, double(0));
		setValue(gstCamTransZ, distance);
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
		VolumeList list_;
	};
	PopVolumeVisitor visitor;
	accept(visitor);
	visitor.getResult(m_vol_list);

	setValue(gstVolListDirty, false);
}

void Renderview::PopFullVolList()
{
	bool bval;
	getValue(gstVolListDirty, bval);
	if (!bval)
		return;

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
				list_.push_back(vd);
			traverse(node);
		}

		virtual void apply(Group& group)
		{
			traverse(group);
		}

		void getResult(std::vector<ref_ptr<VolumeData>> &list)
		{
			for (auto vd : list_)
				list.push_back(vd);
		}

	private:
		VolumeList list_;
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

void Renderview::PopFullMeshList()
{
	bool bval;
	getValue(gstMshListDirty, bval);
	if (!bval)
		return;

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
				list_.push_back(md);
			traverse(node);
		}

		virtual void apply(Group& group)
		{
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

//current data
VolumeData* Renderview::GetCurrentVolume()
{
	Referenced* ref;
	getRvalu(gstCurrentVolume, &ref);
	return dynamic_cast<VolumeData*>(ref);
}

MeshData* Renderview::GetCurrentMesh()
{
	Referenced* ref;
	getRvalu(gstCurrentMesh, &ref);
	return dynamic_cast<MeshData*>(ref);
}

//indexed data
VolumeData* Renderview::GetVolume(size_t i)
{
	if (i >= m_vol_full_list.size())
		return nullptr;
	return m_vol_full_list[i].get();
}

MeshData* Renderview::GetMesh(size_t i)
{
	if (i >= m_msh_full_list.size())
		return nullptr;
	return m_msh_full_list[i].get();
}

VolumeData* Renderview::GetShownVolume(size_t i)
{
	if (i >= m_vol_list.size())
		return nullptr;
	return m_vol_list[i].get();
}

MeshData* Renderview::GetShownMesh(size_t i)
{
	if (i >= m_msh_list.size())
		return nullptr;
	return m_msh_list[i].get();
}

//conversion
VolumeList Renderview::GetVolList()
{
	VolumeList list;
	for (auto vd : m_vol_list)
		list.push_back(vd.get());
	return list;
}

MeshList Renderview::GetMeshList()
{
	MeshList list;
	for (auto md : m_msh_list)
		list.push_back(md.get());
	return list;
}

VolumeList Renderview::GetFullVolList()
{
	VolumeList list;
	for (auto vd : m_vol_full_list)
		list.push_back(vd.get());
	return list;
}

MeshList Renderview::GetFullMeshList()
{
	MeshList list;
	for (auto md : m_msh_full_list)
		list.push_back(md.get());
	return list;
}

//num
size_t Renderview::GetVolListSize()
{
	return m_vol_list.size();
}

size_t Renderview::GetFullVolListSize()
{
	return m_vol_full_list.size();
}

size_t Renderview::GetMeshListSize()
{
	return m_msh_list.size();
}

size_t Renderview::GetFullMeshListSize()
{
	return m_msh_full_list.size();
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
	getValue(gstCamTransX, dx);
	getValue(gstCamTransY, dy);
	getValue(gstCamTransZ, dz);
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
	setValue(gstCamTransX, pos.x());
	setValue(gstCamTransY, pos.y());
	setValue(gstCamTransZ, pos.z());

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
	updValue(gstVolListDirty, true, Event());
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

	//RefreshGL(16);
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
	VolumeData* vd = GetCurrentVolume();

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
	setRvalu(gstCurrentVolume, vd);
	m_selector->SetVolume(vd);
	m_calculator->SetVolumeA(vd);

	//RefreshGL(17);
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
	VolumeData* vd = GetCurrentVolume();

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
	setValue(gstCurrentVolume, vd);
	m_selector->SetVolume(vd);
	m_calculator->SetVolumeA(vd);

	//RefreshGL(18);
}

void Renderview::CalculateCrop()
{
	long w, h;
	getValue(gstSizeX, w);
	getValue(gstSizeY, h);

	VolumeData* vd = GetCurrentVolume();

	if (vd)
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

void Renderview::Grow(long sz)
{
	m_selector->SetInitMask(2);
	Segment();
	m_selector->SetInitMask(3);
	long int_mode;
	getValue(gstInterMode, int_mode);
	if (int_mode == 12)
	{
		flrd::SegGrow sg(GetCurrentVolume());
		sg.SetRulerHandler(m_ruler_handler);
		sg.SetIter(m_selector->GetIter() * 3);
		sg.SetSizeThresh(sz);
		sg.Compute();
	}
}

void Renderview::ForceDraw()
{
	bool bval;
	getValue(gstRefresh, bval);
	if (!bval)
		setValue(gstRetainFb, true);
	else
		setValue(gstRefresh, false);

	Init();

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
	if (bval) DrawCropFrame();
	//draw info
	getValue(gstDrawInfo, bval);
	if (bval & INFO_DISP) DrawInfo();

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
			flipValue(gstSwapBuffers, bval);
			//SwapBuffers();
		}
		else
		{
			setValue(gstVrEyeIdx, long(1));
			//RefreshGL(99);
		}
	}
	else
		flipValue(gstSwapBuffers, bval);
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
	//			RenderCanvas* view = m_frame->GetView(i);
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
		VolumeList list;
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
				VolumeList list;
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
void Renderview::Update(int debug_code, bool start_loop)
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
	//setValue(gstRefresh, true);
	//setValue(gstRefreshErase, erase);
	flupValue(gstRefreshNotify, bval);
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
	if (m_cell_list.empty())
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
	int vert_num = m_cell_list.size();
	verts.reserve(vert_num * 8 * 3 * 2);

	unsigned int num = 0;
	Point p1, p2, p3, p4;
	Color c;
	getValue(gstTextColor, c);
	double sx, sy, sz;
	sx = m_cell_list.sx;
	sy = m_cell_list.sy;
	sz = m_cell_list.sz;
	for (auto it = m_cell_list.begin();
		it != m_cell_list.end(); ++it)
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
	VolumeData* vd = GetCurrentVolume();
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
	VolumeData* vd = GetCurrentVolume();
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
	long lx, ly, lw, lh;
	bool enlarge;
	getValue(gstEnlarge, enlarge);
	bool bval;
	getValue(gstDrawCropFrame, bval);
	if (bval)
	{
		getValue(gstCropX, lx);
		getValue(gstCropY, ly);
		getValue(gstCropW, lw);
		getValue(gstCropH, lh);
		if (enlarge)
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
	Color text_color;
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

void Renderview::DrawLegend()
{
	double font_height =
		flvr::TextRenderer::text_texture_manager_.GetSize() + 3.0;

	long nx, ny;
	GetRenderSize(nx, ny);

	double xoffset = 10.0;
	double yoffset = 10.0;
	bool draw_cropf;
	getValue(gstDrawCropFrame, draw_cropf);
	long lx, ly, lw, lh;
	getValue(gstCropX, lx);
	getValue(gstCropY, ly);
	getValue(gstCropW, lw);
	getValue(gstCropH, lh);
	if (draw_cropf)
	{
		xoffset = 10.0 + lx;
		yoffset = ny - lh - ly + 10.0;
	}

	wstring wstr;
	double length = 0.0;
	double name_len = 0.0;
	double gap_width = font_height * 1.5;
	int lines = 0;
	bool bval;
	//first pass
	for (auto vd : m_vol_list)
	{

		if (!vd) continue;
		vd->getValue(gstLegend, bval);
		if (bval)
		{
			wstr = s2ws(vd->getName());
			name_len = m_text_renderer->RenderTextLen(wstr) + font_height;
			length += name_len;
			if (length < double(draw_cropf ? lw : nx) - gap_width)
			{
				length += gap_width;
			}
			else
			{
				length = name_len + gap_width;
				lines++;
			}
		}
	}
	for (auto md : m_msh_list)
	{
		if (!md) continue;
		md->getValue(gstLegend, bval);
		if (bval)
		{
			wstr = s2ws(md->getName());
			name_len = m_text_renderer->RenderTextLen(wstr) + font_height;
			length += name_len;
			if (length < double(draw_cropf ? lw : nx) - gap_width)
			{
				length += gap_width;
			}
			else
			{
				length = name_len + gap_width;
				lines++;
			}
		}
	}

	//second pass
	int cur_line = 0;
	double xpos;
	length = 0.0;
	long current_sel;
	Referenced* ref = 0;
	getValue(gstCurrentSelect, current_sel);
	switch (current_sel)
	{
	case 2:
		getRvalu(gstCurrentVolume, &ref);
		break;
	case 3:
		getRvalu(gstCurrentMesh, &ref);
		break;
	}
	for (auto vd : m_vol_list)
	{
		if (!vd) continue;
		vd->getValue(gstLegend, bval);
		if (bval)
		{
			wstr = s2ws(vd->getName());
			xpos = length;
			name_len = m_text_renderer->RenderTextLen(wstr) + font_height;
			length += name_len;
			if (length < double(draw_cropf ? lw : nx) - gap_width)
			{
				length += gap_width;
			}
			else
			{
				length = name_len + gap_width;
				xpos = 0.0;
				cur_line++;
			}
			bool highlighted = false;
			if (current_sel == 2 && ref)
			{
				VolumeData* vd = dynamic_cast<VolumeData*>(ref);
				if (vd && s2ws(vd->getName()) == wstr)
					highlighted = true;
			}
			Color color;
			vd->getValue(gstColor, color);
			DrawName(xpos + xoffset, ny - (lines - cur_line + 0.1)*font_height - yoffset,
				nx, ny, wstr, color, font_height, highlighted);
		}
	}
	for (auto md : m_msh_list)
	{
		if (!md) continue;
		md->getValue(gstLegend, bval);
		if (bval)
		{
			wstr = s2ws(md->getName());
			xpos = length;
			name_len = m_text_renderer->RenderTextLen(wstr) + font_height;
			length += name_len;
			if (length < double(draw_cropf ? lw : nx) - gap_width)
			{
				length += gap_width;
			}
			else
			{
				length = name_len + gap_width;
				xpos = 0.0;
				cur_line++;
			}
			Color color;
			md->getValue(gstColor, color);
			bool highlighted = false;
			if (current_sel == 3 && ref)
			{
				MeshData* md = dynamic_cast<MeshData*>(ref);
				if (md && s2ws(md->getName()) == wstr)
					highlighted = true;
			}
			DrawName(xpos + xoffset, ny - (lines - cur_line + 0.1)*font_height - yoffset,
				nx, ny, wstr, color, font_height, highlighted);
		}
	}

	setValue(gstScaleBarHeight, (lines + 1)*font_height);
}

void Renderview::DrawName(
	double x, double y, int nx, int ny,
	const std::wstring &name, Color color,
	double font_height,
	bool highlighted)
{
	flvr::VertexArray* va_legend_squares =
		flvr::TextureRenderer::vertex_array_manager_.vertex_array(flvr::VA_Legend_Squares);
	if (!va_legend_squares)
		return;

	float sx, sy;
	sx = 2.0 / nx;
	sy = 2.0 / ny;
	glm::mat4 proj_mat = glm::ortho(0.0f, float(nx), 0.0f, float(ny));

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	flvr::ShaderProgram* shader =
		flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHDR_DRAW_GEOMETRY);
	if (shader)
	{
		if (!shader->valid())
			shader->create();
		shader->bind();
	}
	shader->setLocalParamMatrix(0, glm::value_ptr(proj_mat));

	std::vector<std::pair<unsigned int, double>> params;
	params.push_back(std::pair<unsigned int, double>(0, x + 0.2*font_height));
	params.push_back(std::pair<unsigned int, double>(1, ny - y + 0.2*font_height));
	params.push_back(std::pair<unsigned int, double>(2, x + 0.8*font_height));
	params.push_back(std::pair<unsigned int, double>(3, ny - y + 0.8*font_height));
	va_legend_squares->set_param(params);
	va_legend_squares->draw_begin();
	Color text_color;
	getValue(gstTextColor, text_color);
	shader->setLocalParam(0, text_color.r(), text_color.g(), text_color.b(), 1.0);
	va_legend_squares->draw_legend_square(0);
	shader->setLocalParam(0, color.r(), color.g(), color.b(), 1.0);
	va_legend_squares->draw_legend_square(1);
	va_legend_squares->draw_end();

	if (shader && shader->valid())
		shader->release();

	float px1 = x + font_height - nx / 2;
	float py1 = ny / 2 - y + 0.25*font_height;
	m_text_renderer->RenderText(
		name, text_color,
		px1*sx, py1*sy, sx, sy);
	if (highlighted)
	{
		px1 -= 0.5;
		py1 += 0.5;
		m_text_renderer->RenderText(
			name, color,
			px1*sx, py1*sy, sx, sy);
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
}

void Renderview::DrawCropFrame()
{
	long nx, ny;
	GetRenderSize(nx, ny);
	glm::mat4 proj_mat = glm::ortho(float(0), float(nx), float(0), float(ny));

	flvr::VertexArray* va_frame =
		flvr::TextureRenderer::vertex_array_manager_.vertex_array(flvr::VA_Crop_Frame);
	if (!va_frame)
		return;

	glDisable(GL_DEPTH_TEST);
	flvr::ShaderProgram* shader =
		flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHDR_DRAW_GEOMETRY);
	if (shader)
	{
		if (!shader->valid())
			shader->create();
		shader->bind();
	}
	shader->setLocalParam(0, 1.0, 1.0, 0.0, 1.0);
	shader->setLocalParamMatrix(0, glm::value_ptr(proj_mat));

	//draw frame
	vector<std::pair<unsigned int, double>> params;
	long lval;
	getValue(gstCropX, lval);
	params.push_back(std::pair<unsigned int, double>(0, lval));
	getValue(gstCropY, lval);
	params.push_back(std::pair<unsigned int, double>(1, lval));
	getValue(gstCropW, lval);
	params.push_back(std::pair<unsigned int, double>(2, lval));
	getValue(gstCropH, lval);
	params.push_back(std::pair<unsigned int, double>(3, lval));
	va_frame->set_param(params);
	va_frame->draw();

	if (shader && shader->valid())
		shader->release();
	glEnable(GL_DEPTH_TEST);
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

	VolumeData* cur_vol = GetCurrentVolume();
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

void Renderview::SetColormapColors(long colormap, Color &c, double inv)
{
	switch (colormap)
	{
	case 0://rainbow
		if (inv > 0.0)
		{
			setValue(gstColor1, Color(0.0, 0.0, 1.0));
			setValue(gstColor2, Color(0.0, 0.0, 1.0));
			setValue(gstColor3, Color(0.0, 1.0, 1.0));
			setValue(gstColor4, Color(0.0, 1.0, 0.0));
			setValue(gstColor5, Color(1.0, 1.0, 0.0));
			setValue(gstColor6, Color(1.0, 0.0, 0.0));
			setValue(gstColor7, Color(1.0, 0.0, 0.0));
		}
		else
		{
			setValue(gstColor1, Color(1.0, 0.0, 0.0));
			setValue(gstColor2, Color(1.0, 0.0, 0.0));
			setValue(gstColor3, Color(1.0, 1.0, 0.0));
			setValue(gstColor4, Color(0.0, 1.0, 0.0));
			setValue(gstColor5, Color(0.0, 1.0, 1.0));
			setValue(gstColor6, Color(0.0, 0.0, 1.0));
			setValue(gstColor7, Color(0.0, 0.0, 1.0));
		}
		break;
	case 1://hot
		if (inv > 0.0)
		{
			setValue(gstColor1, Color(0.0, 0.0, 0.0));
			setValue(gstColor2, Color(0.0, 0.0, 0.0));
			setValue(gstColor3, Color(0.5, 0.0, 0.0));
			setValue(gstColor4, Color(1.0, 0.0, 0.0));
			setValue(gstColor5, Color(1.0, 1.0, 0.0));
			setValue(gstColor6, Color(1.0, 1.0, 1.0));
			setValue(gstColor7, Color(1.0, 1.0, 1.0));
		}
		else
		{
			setValue(gstColor1, Color(1.0, 1.0, 1.0));
			setValue(gstColor2, Color(1.0, 1.0, 1.0));
			setValue(gstColor3, Color(1.0, 1.0, 0.0));
			setValue(gstColor4, Color(1.0, 0.0, 0.0));
			setValue(gstColor5, Color(0.5, 0.0, 0.0));
			setValue(gstColor6, Color(0.0, 0.0, 0.0));
			setValue(gstColor7, Color(0.0, 0.0, 0.0));
		}
		break;
	case 2://cool
		if (inv > 0.0)
		{
			setValue(gstColor1, Color(0.0, 1.0, 1.0));
			setValue(gstColor2, Color(0.0, 1.0, 1.0));
			setValue(gstColor3, Color(0.25, 0.75, 1.0));
			setValue(gstColor4, Color(0.5, 0.5, 1.0));
			setValue(gstColor5, Color(0.75, 0.25, 1.0));
			setValue(gstColor6, Color(1.0, 0.0, 1.0));
			setValue(gstColor7, Color(1.0, 0.0, 1.0));
		}
		else
		{
			setValue(gstColor1, Color(1.0, 0.0, 1.0));
			setValue(gstColor2, Color(1.0, 0.0, 1.0));
			setValue(gstColor3, Color(0.75, 0.25, 1.0));
			setValue(gstColor4, Color(0.5, 0.5, 1.0));
			setValue(gstColor5, Color(0.25, 0.75, 1.0));
			setValue(gstColor6, Color(0.0, 1.0, 1.0));
			setValue(gstColor7, Color(0.0, 1.0, 1.0));
		}
		break;
	case 3://diverging
		if (inv > 0.0)
		{
			setValue(gstColor1, Color(0.25, 0.3, 0.75));
			setValue(gstColor2, Color(0.25, 0.3, 0.75));
			setValue(gstColor3, Color(0.475, 0.5, 0.725));
			setValue(gstColor4, Color(0.7, 0.7, 0.7));
			setValue(gstColor5, Color(0.7, 0.35, 0.425));
			setValue(gstColor6, Color(0.7, 0.0, 0.15));
			setValue(gstColor7, Color(0.7, 0.0, 0.15));
		}
		else
		{
			setValue(gstColor1, Color(0.7, 0.0, 0.15));
			setValue(gstColor2, Color(0.7, 0.0, 0.15));
			setValue(gstColor3, Color(0.7, 0.35, 0.425));
			setValue(gstColor4, Color(0.7, 0.7, 0.7));
			setValue(gstColor5, Color(0.475, 0.5, 0.725));
			setValue(gstColor6, Color(0.25, 0.3, 0.75));
			setValue(gstColor7, Color(0.25, 0.3, 0.75));
		}
		break;
	case 4://monochrome
		if (inv > 0.0)
		{
			setValue(gstColor1, Color(0.0, 0.0, 0.0));
			setValue(gstColor2, Color(0.0, 0.0, 0.0));
			setValue(gstColor3, Color(0.25, 0.25, 0.25));
			setValue(gstColor4, Color(0.5, 0.5, 0.5));
			setValue(gstColor5, Color(0.75, 0.75, 0.75));
			setValue(gstColor6, Color(1.0, 1.0, 1.0));
			setValue(gstColor7, Color(1.0, 1.0, 1.0));
		}
		else
		{
			setValue(gstColor1, Color(1.0, 1.0, 1.0));
			setValue(gstColor2, Color(1.0, 1.0, 1.0));
			setValue(gstColor3, Color(0.75, 0.75, 0.75));
			setValue(gstColor4, Color(0.5, 0.5, 0.5));
			setValue(gstColor5, Color(0.25, 0.25, 0.25));
			setValue(gstColor6, Color(0.0, 0.0, 0.0));
			setValue(gstColor7, Color(0.0, 0.0, 0.0));
		}
		break;
	case 5://high-key
		if (inv > 0.0)
		{
			setValue(gstColor1, Color(1.0, 1.0, 1.0));
			setValue(gstColor2, Color(1.0, 1.0, 1.0));
			setValue(gstColor3, c * 0.25 + Color(1.0, 1.0, 1.0)*0.75);
			setValue(gstColor4, (c + Color(1.0, 1.0, 1.0))*0.5);
			setValue(gstColor5, c * 0.75 + Color(1.0, 1.0, 1.0)*0.25);
			setValue(gstColor6, c);
			setValue(gstColor7, c);
		}
		else
		{
			setValue(gstColor1, c);
			setValue(gstColor2, c);
			setValue(gstColor3, c * 0.75 + Color(1.0, 1.0, 1.0)*0.25);
			setValue(gstColor4, (c + Color(1.0, 1.0, 1.0))*0.5);
			setValue(gstColor5, c * 0.25 + Color(1.0, 1.0, 1.0)*0.75);
			setValue(gstColor6, Color(1.0, 1.0, 1.0));
			setValue(gstColor7, Color(1.0, 1.0, 1.0));
		}
		break;
	case 6://low-key
		if (inv > 0.0)
		{
			setValue(gstColor1, c);
			setValue(gstColor2, c);
			setValue(gstColor3, c * (0.025 + 0.75));
			setValue(gstColor4, c * 0.55);
			setValue(gstColor5, c * (0.075 + 0.25));
			setValue(gstColor6, c * 0.1);
			setValue(gstColor7, c * 0.1);
		}
		else
		{
			setValue(gstColor1, c * 0.1);
			setValue(gstColor2, c * 0.1);
			setValue(gstColor3, c * (0.075 + 0.25));
			setValue(gstColor4, c * 0.55);
			setValue(gstColor5, c * (0.025 + 0.75));
			setValue(gstColor6, c);
			setValue(gstColor7, c);
		}
		break;
	case 7://high transparency
		if (inv > 0.0)
		{
			setValue(gstColor1, Color(0.0, 0.0, 0.0));
			setValue(gstColor2, Color(0.0, 0.0, 0.0));
			setValue(gstColor3, c * 0.25 + Color(0.0, 0.0, 0.0) * 0.75);
			setValue(gstColor4, c * 0.5 + Color(0.0, 0.0, 0.0) * 0.5);
			setValue(gstColor5, c * 0.75 + Color(0.0, 0.0, 0.0) * 0.25);
			setValue(gstColor6, c);
			setValue(gstColor7, c);
		}
		else
		{
			setValue(gstColor1, c);
			setValue(gstColor2, c);
			setValue(gstColor3, c * 0.75 + Color(0.0, 0.0, 0.0) * 0.25);
			setValue(gstColor4, c * 0.5 + Color(0.0, 0.0, 0.0) * 0.5);
			setValue(gstColor5, c * 0.25 + Color(0.0, 0.0, 0.0) * 0.75);
			setValue(gstColor6, Color(0.0, 0.0, 0.0));
			setValue(gstColor7, Color(0.0, 0.0, 0.0));
		}
		break;
	}
}

void Renderview::DrawColormap()
{
	VolumeData* vd = GetCurrentVolume();
	if (!vd) return;

	double dval;
	bool bval;
	double max_val = 255.0;
	bool enable_alpha = false;
	long colormap_mode;
	vd->getValue(gstColormapMode, colormap_mode);
	if (colormap_mode)
	{
		double low, high;
		vd->getValue(gstColormapLow, low);
		vd->getValue(gstColormapHigh, high);
		dval = (low + high) / 2.0;
		setValue(gstColVal2, low);
		setValue(gstColVal3, (low + dval) / 2.0);
		setValue(gstColVal4, dval);
		setValue(gstColVal5, (dval + high) / 2.0);
		setValue(gstColVal6, high);
		vd->getValue(gstMaxInt, max_val);
		vd->getValue(gstAlphaEnable, enable_alpha);
		Color vd_color;
		vd->getValue(gstColor, vd_color);
		bool colormap_inv;
		vd->getValue(gstColormapInv, colormap_inv);
		SetColormapColors(colormap_mode, vd_color, colormap_inv);
	}
	else return;

	double offset = 0.0;
	getValue(gstDrawLegend, bval);
	if (bval)
	{
		getValue(gstScaleBarHeight, dval);
		offset = dval;
	}

	long nx, ny;
	GetRenderSize(nx, ny);
	float sx, sy;
	sx = 2.0 / nx;
	sy = 2.0 / ny;

	glm::mat4 proj_mat = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f);

	vector<float> vertex;
	vertex.reserve(14 * 7);

	float px, py;
	//draw colormap
	Color c1, c2, c3, c4, c5, c6, c7;
	getValue(gstColor1, c1);
	getValue(gstColor2, c2);
	getValue(gstColor3, c3);
	getValue(gstColor4, c4);
	getValue(gstColor5, c5);
	getValue(gstColor6, c6);
	getValue(gstColor7, c7);
	double cv2, cv3, cv4, cv5, cv6;
	getValue(gstColVal2, cv2);
	getValue(gstColVal3, cv3);
	getValue(gstColVal4, cv4);
	getValue(gstColVal5, cv5);
	getValue(gstColVal6, cv6);
	getValue(gstDrawCropFrame, bval);
	if (bval)
	{
		long lx, ly, lw, lh;
		getValue(gstCropX, lx);
		getValue(gstCropY, ly);
		getValue(gstCropW, lw);
		getValue(gstCropH, lh);
		px = (0.01*lw + lx) / nx;
		py = (0.05*lw + lx) / nx;
		vertex.push_back(px); vertex.push_back((0.1*lh + ly + offset) / ny); vertex.push_back(0.0);
		vertex.push_back(c1.r()); vertex.push_back(c1.g()); vertex.push_back(c1.b()); vertex.push_back(enable_alpha ? 0.0 : 1.0);
		vertex.push_back(py); vertex.push_back((0.1*lh + ly + offset) / ny); vertex.push_back(0.0);
		vertex.push_back(c1.r()); vertex.push_back(c1.g()); vertex.push_back(c1.b()); vertex.push_back(enable_alpha ? 0.0 : 1.0);
		vertex.push_back(px); vertex.push_back(((0.1 + 0.4*cv2)*lh + ly + offset) / ny); vertex.push_back(0.0);
		vertex.push_back(c2.r()); vertex.push_back(c2.g()); vertex.push_back(c2.b()); vertex.push_back(enable_alpha ? cv2 : 1.0);
		vertex.push_back(py); vertex.push_back(((0.1 + 0.4*cv2)*lh + ly + offset) / ny); vertex.push_back(0.0);
		vertex.push_back(c2.r()); vertex.push_back(c2.g()); vertex.push_back(c2.b()); vertex.push_back(enable_alpha ? cv2 : 1.0);
		vertex.push_back(px); vertex.push_back(((0.1 + 0.4*cv3)*lh + ly + offset) / ny); vertex.push_back(0.0);
		vertex.push_back(c3.r()); vertex.push_back(c3.g()); vertex.push_back(c3.b()); vertex.push_back(enable_alpha ? cv3 : 1.0);
		vertex.push_back(py); vertex.push_back(((0.1 + 0.4*cv3)*lh + ly + offset) / ny); vertex.push_back(0.0);
		vertex.push_back(c3.r()); vertex.push_back(c3.g()); vertex.push_back(c3.b()); vertex.push_back(enable_alpha ? cv3 : 1.0);
		vertex.push_back(px); vertex.push_back(((0.1 + 0.4*cv4)*lh + ly + offset) / ny); vertex.push_back(0.0);
		vertex.push_back(c4.r()); vertex.push_back(c4.g()); vertex.push_back(c4.b()); vertex.push_back(enable_alpha ? cv4 : 1.0);
		vertex.push_back(py); vertex.push_back(((0.1 + 0.4*cv4)*lh + ly + offset) / ny); vertex.push_back(0.0);
		vertex.push_back(c4.r()); vertex.push_back(c4.g()); vertex.push_back(c4.b()); vertex.push_back(enable_alpha ? cv4 : 1.0);
		vertex.push_back(px); vertex.push_back(((0.1 + 0.4*cv5)*lh + ly + offset) / ny); vertex.push_back(0.0);
		vertex.push_back(c5.r()); vertex.push_back(c5.g()); vertex.push_back(c5.b()); vertex.push_back(enable_alpha ? cv5 : 1.0);
		vertex.push_back(py); vertex.push_back(((0.1 + 0.4*cv5)*lh + ly + offset) / ny); vertex.push_back(0.0);
		vertex.push_back(c5.r()); vertex.push_back(c5.g()); vertex.push_back(c5.b()); vertex.push_back(enable_alpha ? cv5 : 1.0);
		vertex.push_back(px); vertex.push_back(((0.1 + 0.4*cv6)*lh + ly + offset) / ny); vertex.push_back(0.0);
		vertex.push_back(c6.r()); vertex.push_back(c6.g()); vertex.push_back(c6.b()); vertex.push_back(enable_alpha ? cv6 : 1.0);
		vertex.push_back(py); vertex.push_back(((0.1 + 0.4*cv6)*lh + ly + offset) / ny); vertex.push_back(0.0);
		vertex.push_back(c6.r()); vertex.push_back(c6.g()); vertex.push_back(c6.b()); vertex.push_back(enable_alpha ? cv6 : 1.0);
		vertex.push_back(px); vertex.push_back((0.5*lh + ly + offset) / ny); vertex.push_back(0.0);
		vertex.push_back(c7.r()); vertex.push_back(c7.g()); vertex.push_back(c7.b()); vertex.push_back(1.0);
		vertex.push_back(py); vertex.push_back((0.5*lh + ly + offset) / ny); vertex.push_back(0.0);
		vertex.push_back(c7.r()); vertex.push_back(c7.g()); vertex.push_back(c7.b()); vertex.push_back(1.0);

		wstring wstr;

		Color text_color;
		getValue(gstTextColor, text_color);

		//value 1
		px = 0.052*lw + lx - nx / 2.0;
		py = 0.1*lh + ly + offset - ny / 2.0;
		wstr = std::to_wstring(0);
		m_text_renderer->RenderText(
			wstr, text_color,
			px*sx, py*sy, sx, sy);
		//value 2
		px = 0.052*lw + lx - nx / 2.0;
		py = (0.1 + 0.4*cv2)*lh + ly + offset - ny / 2.0;
		wstr = std::to_wstring(int(cv2*max_val));
		m_text_renderer->RenderText(
			wstr, text_color,
			px*sx, py*sy, sx, sy);
		//value 4
		px = 0.052*lw + lx - nx / 2.0;
		py = (0.1 + 0.4*cv4)*lh + ly + offset - ny / 2.0;
		wstr = std::to_wstring(int(cv4*max_val));
		m_text_renderer->RenderText(
			wstr, text_color,
			px*sx, py*sy, sx, sy);
		//value 6
		px = 0.052*lw + lx - nx / 2.0;
		py = (0.1 + 0.4*cv6)*lh + ly + offset - ny / 2.0;
		wstr = std::to_wstring(int(cv6*max_val));
		m_text_renderer->RenderText(
			wstr, text_color,
			px*sx, py*sy, sx, sy);
		//value 7
		px = 0.052*lw + lx - nx / 2.0;
		py = 0.5*lh + ly + offset - ny / 2.0;
		wstr = std::to_wstring(int(max_val));
		m_text_renderer->RenderText(
			wstr, text_color,
			px*sx, py*sy, sx, sy);
	}
	else
	{
		vertex.push_back(0.01); vertex.push_back(0.1 + offset / ny); vertex.push_back(0.0);
		vertex.push_back(c1.r()); vertex.push_back(c1.g()); vertex.push_back(c1.b()); vertex.push_back(enable_alpha ? 0.0 : 1.0);
		vertex.push_back(0.05); vertex.push_back(0.1 + offset / ny); vertex.push_back(0.0);
		vertex.push_back(c1.r()); vertex.push_back(c1.g()); vertex.push_back(c1.b()); vertex.push_back(enable_alpha ? 0.0 : 1.0);
		vertex.push_back(0.01); vertex.push_back(0.1 + 0.4*cv2 + offset / ny); vertex.push_back(0.0);
		vertex.push_back(c2.r()); vertex.push_back(c2.g()); vertex.push_back(c2.b()); vertex.push_back(enable_alpha ? cv2 : 1.0);
		vertex.push_back(0.05); vertex.push_back(0.1 + 0.4*cv2 + offset / ny); vertex.push_back(0.0);
		vertex.push_back(c2.r()); vertex.push_back(c2.g()); vertex.push_back(c2.b()); vertex.push_back(enable_alpha ? cv2 : 1.0);
		vertex.push_back(0.01); vertex.push_back(0.1 + 0.4*cv3 + offset / ny); vertex.push_back(0.0);
		vertex.push_back(c3.r()); vertex.push_back(c3.g()); vertex.push_back(c3.b()); vertex.push_back(enable_alpha ? cv3 : 1.0);
		vertex.push_back(0.05); vertex.push_back(0.1 + 0.4*cv3 + offset / ny); vertex.push_back(0.0);
		vertex.push_back(c3.r()); vertex.push_back(c3.g()); vertex.push_back(c3.b()); vertex.push_back(enable_alpha ? cv3 : 1.0);
		vertex.push_back(0.01); vertex.push_back(0.1 + 0.4*cv4 + offset / ny); vertex.push_back(0.0);
		vertex.push_back(c4.r()); vertex.push_back(c4.g()); vertex.push_back(c4.b()); vertex.push_back(enable_alpha ? cv4 : 1.0);
		vertex.push_back(0.05); vertex.push_back(0.1 + 0.4*cv4 + offset / ny); vertex.push_back(0.0);
		vertex.push_back(c4.r()); vertex.push_back(c4.g()); vertex.push_back(c4.b()); vertex.push_back(enable_alpha ? cv4 : 1.0);
		vertex.push_back(0.01); vertex.push_back(0.1 + 0.4*cv5 + offset / ny); vertex.push_back(0.0);
		vertex.push_back(c5.r()); vertex.push_back(c5.g()); vertex.push_back(c5.b()); vertex.push_back(enable_alpha ? cv5 : 1.0);
		vertex.push_back(0.05); vertex.push_back(0.1 + 0.4*cv5 + offset / ny); vertex.push_back(0.0);
		vertex.push_back(c5.r()); vertex.push_back(c5.g()); vertex.push_back(c5.b()); vertex.push_back(enable_alpha ? cv5 : 1.0);
		vertex.push_back(0.01); vertex.push_back(0.1 + 0.4*cv6 + offset / ny); vertex.push_back(0.0);
		vertex.push_back(c6.r()); vertex.push_back(c6.g()); vertex.push_back(c6.b()); vertex.push_back(enable_alpha ? cv6 : 1.0);
		vertex.push_back(0.05); vertex.push_back(0.1 + 0.4*cv6 + offset / ny); vertex.push_back(0.0);
		vertex.push_back(c6.r()); vertex.push_back(c6.g()); vertex.push_back(c6.b()); vertex.push_back(enable_alpha ? cv6 : 1.0);
		vertex.push_back(0.01); vertex.push_back(0.5 + offset / ny); vertex.push_back(0.0);
		vertex.push_back(c7.r()); vertex.push_back(c7.g()); vertex.push_back(c7.b()); vertex.push_back(1.0);
		vertex.push_back(0.05); vertex.push_back(0.5 + offset / ny); vertex.push_back(0.0);
		vertex.push_back(c7.r()); vertex.push_back(c7.g()); vertex.push_back(c7.b()); vertex.push_back(1.0);

		wstring wstr;

		Color text_color;
		getValue(gstTextColor, text_color);

		//value 1
		px = 0.052*nx - nx / 2.0;
		py = ny / 2.0 - 0.9*ny + offset;
		wstr = std::to_wstring(0);
		m_text_renderer->RenderText(
			wstr, text_color,
			px*sx, py*sy, sx, sy);
		//value 2
		px = 0.052*nx - nx / 2.0;
		py = ny / 2.0 - (0.9 - 0.4*cv2)*ny + offset;
		wstr = std::to_wstring(int(cv2*max_val));
		m_text_renderer->RenderText(
			wstr, text_color,
			px*sx, py*sy, sx, sy);
		//value 4
		px = 0.052*nx - nx / 2.0;
		py = ny / 2.0 - (0.9 - 0.4*cv4)*ny + offset;
		wstr = std::to_wstring(int(cv4*max_val));
		m_text_renderer->RenderText(
			wstr, text_color,
			px*sx, py*sy, sx, sy);
		//value 6
		px = 0.052*nx - nx / 2.0;
		py = ny / 2.0 - (0.9 - 0.4*cv6)*ny + offset;
		wstr = std::to_wstring(int(cv6*max_val));
		m_text_renderer->RenderText(
			wstr, text_color,
			px*sx, py*sy, sx, sy);
		//value 7
		px = 0.052*nx - nx / 2.0;
		py = ny / 2.0 - 0.5*ny + offset;
		wstr = std::to_wstring(int(max_val));
		m_text_renderer->RenderText(
			wstr, text_color,
			px*sx, py*sy, sx, sy);
	}

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	flvr::ShaderProgram* shader =
		flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHDR_DRAW_GEOMETRY_COLOR4);
	if (shader)
	{
		if (!shader->valid())
			shader->create();
		shader->bind();
	}
	shader->setLocalParamMatrix(0, glm::value_ptr(proj_mat));

	flvr::VertexArray* va_colormap =
		flvr::TextureRenderer::vertex_array_manager_.vertex_array(flvr::VA_Color_Map);
	if (va_colormap)
	{
		va_colormap->set_param(vertex);
		va_colormap->draw();
	}

	if (shader && shader->valid())
		shader->release();

	glEnable(GL_DEPTH_TEST);
}

void Renderview::DrawGradBg()
{
	glm::mat4 proj_mat = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	Color cbg, color1, color2;
	getValue(gstBgColor, cbg);
	HSVColor hsv_color1(cbg);
	if (hsv_color1.val() > 0.5)
	{
		if (hsv_color1.sat() > 0.3)
		{
			color1 = Color(HSVColor(hsv_color1.hue(),
				hsv_color1.sat() * 0.1,
				std::min(hsv_color1.val() + 0.3, 1.0)));
			color2 = Color(HSVColor(hsv_color1.hue(),
				hsv_color1.sat() * 0.3,
				std::min(hsv_color1.val() + 0.1, 1.0)));
		}
		else
		{
			color1 = Color(HSVColor(hsv_color1.hue(),
				hsv_color1.sat() * 0.1,
				std::max(hsv_color1.val() - 0.5, 0.0)));
			color2 = Color(HSVColor(hsv_color1.hue(),
				hsv_color1.sat() * 0.3,
				std::max(hsv_color1.val() - 0.3, 0.0)));
		}
	}
	else
	{
		color1 = Color(HSVColor(hsv_color1.hue(),
			hsv_color1.sat() * 0.1,
			std::min(hsv_color1.val() + 0.7, 1.0)));
		color2 = Color(HSVColor(hsv_color1.hue(),
			hsv_color1.sat() * 0.3,
			std::min(hsv_color1.val() + 0.5, 1.0)));
	}

	vector<float> vertex;
	vertex.reserve(16 * 3);
	vertex.push_back(0.0); vertex.push_back(0.0); vertex.push_back(0.0);
	vertex.push_back(cbg.r()); vertex.push_back(cbg.g()); vertex.push_back(cbg.b());
	vertex.push_back(1.0); vertex.push_back(0.0); vertex.push_back(0.0);
	vertex.push_back(cbg.r()); vertex.push_back(cbg.g()); vertex.push_back(cbg.b());
	vertex.push_back(0.0); vertex.push_back(0.3); vertex.push_back(0.0);
	vertex.push_back(color1.r()); vertex.push_back(color1.g()); vertex.push_back(color1.b());
	vertex.push_back(1.0); vertex.push_back(0.3); vertex.push_back(0.0);
	vertex.push_back(color1.r()); vertex.push_back(color1.g()); vertex.push_back(color1.b());
	vertex.push_back(0.0); vertex.push_back(0.5); vertex.push_back(0.0);
	vertex.push_back(color2.r()); vertex.push_back(color2.g()); vertex.push_back(color2.b());
	vertex.push_back(1.0); vertex.push_back(0.5); vertex.push_back(0.0);
	vertex.push_back(color2.r()); vertex.push_back(color2.g()); vertex.push_back(color2.b());
	vertex.push_back(0.0); vertex.push_back(1.0); vertex.push_back(0.0);
	vertex.push_back(cbg.r()); vertex.push_back(cbg.g()); vertex.push_back(cbg.b());
	vertex.push_back(1.0); vertex.push_back(1.0); vertex.push_back(0.0);
	vertex.push_back(cbg.r()); vertex.push_back(cbg.g()); vertex.push_back(cbg.b());

	flvr::ShaderProgram* shader =
		flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHDR_DRAW_GEOMETRY_COLOR3);
	if (shader)
	{
		if (!shader->valid())
			shader->create();
		shader->bind();
	}
	shader->setLocalParamMatrix(0, glm::value_ptr(proj_mat));

	flvr::VertexArray* va_bkg =
		flvr::TextureRenderer::vertex_array_manager_.vertex_array(flvr::VA_Grad_Bkg);
	if (va_bkg)
	{
		va_bkg->set_param(vertex);
		va_bkg->draw();
	}

	if (shader && shader->valid())
		shader->release();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
}

void Renderview::DrawInfo()
{
	long nx, ny;
	GetRenderSize(nx, ny);

	bool bval;
	double dval;
	float sx, sy;
	sx = 2.0 / nx;
	sy = 2.0 / ny;
	float px, py;
	float gapw = flvr::TextRenderer::text_texture_manager_.GetSize();
	float gaph = gapw * 2;

	double fps = 1.0 / glbin_timer->average();
	std::wstring wstr;
	wstr = L"FPS: ";
	wstr += std::to_wstring(fps);
	if (flvr::TextureRenderer::get_mem_swap())
	{
		wstr += L", Int: ";
		getValue(gstInteractive, bval);
		wstr += bval ? L"Yes" : L"No";
		wstr += L", Bricks: ";
		wstr += std::to_wstring(flvr::TextureRenderer::get_finished_bricks());
		wstr += L", Quota: ";
		wstr += std::to_wstring(flvr::TextureRenderer::get_quota_bricks());
		wstr += L", Time: ";
		wstr += std::to_wstring(flvr::TextureRenderer::get_cor_up_time());
		////budget_test
		//if (m_interactive)
		//  tos <<
		//  flvr::TextureRenderer::get_quota_bricks()
		//  << "\t" <<
		//  flvr::TextureRenderer::get_finished_bricks()
		//  << "\t" <<
		//  flvr::TextureRenderer::get_queue_last()
		//  << "\t" <<
		//  int(flvr::TextureRenderer::get_finished_bricks()*
		//    flvr::TextureRenderer::get_up_time()/
		//    flvr::TextureRenderer::get_consumed_time())
		//  << "\n";
	}
	if (m_selector->GetBrushUsePres())
	{
		wstr += L", Pressure: ";
		wstr += std::to_wstring(m_selector->GetPressure());
	}
	Color text_color;
	getValue(gstTextColor, text_color);
	px = gapw - nx / 2;
	py = ny / 2 - gaph / 2;
	m_text_renderer->RenderText(
		wstr, text_color,
		px*sx, py*sy, sx, sy);

	VolumeData* cvol = GetCurrentVolume();
	long lval;
	getValue(gstDrawInfo, lval);
	if ((lval & INFO_T) &&
		(lval & INFO_X) &&
		(lval & INFO_Y) &&
		(lval & INFO_Z))
	{
		Point p;
		long lx, ly;
		getValue(gstMouseClientX, lx);
		getValue(gstMouseClientY, ly);
		m_volume_point->SetVolumeData(cvol);
		if ((m_volume_point->GetPointVolumeBox(lx, ly, true, p) > 0.0) ||
			m_volume_point->GetPointPlane(lx, ly, 0, true, p) > 0.0)
		{
			getValue(gstCurrentFrame, lval);
			wstr = L"T: "; wstr += std::to_wstring(lval);
			wstr += L", X: "; wstr += std::to_wstring(p.x());
			wstr += L", Y: "; wstr += std::to_wstring(p.y());
			wstr += L", Z: "; wstr += std::to_wstring(p.z());
			px = gapw - nx / 2;
			py = ny / 2 - gaph;
			m_text_renderer->RenderText(
				wstr, text_color,
				px*sx, py*sy, sx, sy);
		}
	}
	else if (lval & INFO_Z)
	{
		if (cvol)
		{
			long resx, resy, resz;
			cvol->getValue(gstResX, resx);
			cvol->getValue(gstResY, resy);
			cvol->getValue(gstResZ, resz);
			double spcx, spcy, spcz;
			cvol->getValue(gstSpcX, spcx);
			cvol->getValue(gstSpcY, spcy);
			cvol->getValue(gstSpcZ, spcz);
			vector<Plane*> *planes = cvol->GetRenderer()->get_planes();
			Plane* plane = (*planes)[4];
			double abcd[4];
			plane->get_copy(abcd);
			int val = fabs(abcd[3] * resz) + 0.499;

			wstr = L"Z: "; wstr += std::to_wstring(val*spcz); wstr += L"\u03BCm";
			getValue(gstDrawCropFrame, bval);
			if (bval)
			{
				long lx, ly, lw, lh;
				getValue(gstCropX, lx);
				getValue(gstCropY, ly);
				getValue(gstCropW, lw);
				getValue(gstCropH, lh);
				px = 0.01*lw + lx - nx / 2.0;
				py = 0.04*lh + ly - ny / 2.0;
			}
			else
			{
				px = 0.01*nx - nx / 2.0;
				py = 0.04*ny - ny / 2.0;
			}
			m_text_renderer->RenderText(
				wstr, text_color,
				px*sx, py*sy, sx, sy);
		}
	}

	getValue(gstTestWiref, bval);
	if (bval)
	{
		getValue(gstMixMethod, lval);
		if (lval == MIX_METHOD_MULTI && m_mvr)
		{
			wstr = L"SLICES: "; wstr += std::to_wstring(m_mvr->get_slice_num());
			px = gapw - nx / 2;
			py = ny / 2 - gaph * 1.5;
			m_text_renderer->RenderText(
				wstr, text_color,
				px*sx, py*sy, sx, sy);
		}
		else
		{
			int i = 1;
			for (auto vd : m_vol_list)
			{
				if (vd && vd->GetRenderer())
				{
					wstr = L"SLICES_"; wstr += std::to_wstring(i); wstr += L": ";
					wstr += std::to_wstring(vd->GetRenderer()->get_slice_num());
					px = gapw - nx / 2;
					py = ny / 2 - gaph * (3 + i) / 2;
					m_text_renderer->RenderText(
						wstr, text_color,
						px*sx, py*sy, sx, sy);
				}
				i++;
			}
		}
	}
}

double Renderview::Get121ScaleFactor()
{
	double result = 1.0;

	long nx, ny;
	GetRenderSize(nx, ny);

	double spc_x = 1.0;
	double spc_y = 1.0;
	double spc_z = 1.0;
	VolumeData *vd = GetCurrentVolume();
	if (!vd && !m_vol_list.empty())
		vd = m_vol_list.front().get();
	if (vd)
	{
		vd->getValue(gstSpcX, spc_x);
		vd->getValue(gstSpcY, spc_y);
		vd->getValue(gstSpcZ, spc_z);
		//vd->GetSpacings(spc_x, spc_y, spc_z, vd->GetLevel());
	}
	spc_y = spc_y < Epsilon() ? 1.0 : spc_y;

	double dval;
	getValue(gstRadius, dval);
	result = 2.0*dval / spc_y / ny;

	return result;
}

//depth buffer calculation
double Renderview::CalcZ(double z)
{
	double result = 0.0;
	bool bval;
	getValue(gstPerspective, bval);
	double near_clip, far_clip;
	getValue(gstNearClip, near_clip);
	getValue(gstFarClip, far_clip);
	if (bval)
	{
		if (z != 0.0)
		{
			result = (far_clip + near_clip) / (far_clip - near_clip) / 2.0 +
				(-far_clip * near_clip) / (far_clip - near_clip) / z + 0.5;
		}
	}
	else
		result = (z - near_clip) / (far_clip - near_clip);
	return result;
}

void Renderview::CalcFogRange()
{
	bool bval;
	BBox bbox;
	bool use_box = false;
	VolumeData* cvol = GetCurrentVolume();
	if (cvol)
	{
		cvol->getValue(gstClipBounds, bbox);
		use_box = true;
	}
	else if (!m_vol_list.empty())
	{
		for (auto vd : m_vol_list)
		{
			vd->getValue(gstDisplay, bval);
			if (bval)
			{
				BBox b;
				vd->getValue(gstBounds, b);
				bbox.extend(b);
				use_box = true;
			}
		}
	}

	if (use_box)
	{
		Transform mv;
		mv.set(glm::value_ptr(m_mv_mat));

		double minz, maxz;
		minz = numeric_limits<double>::max();
		maxz = -numeric_limits<double>::max();

		vector<Point> points;
		points.push_back(Point(bbox.Min().x(), bbox.Min().y(), bbox.Min().z()));
		points.push_back(Point(bbox.Min().x(), bbox.Min().y(), bbox.Max().z()));
		points.push_back(Point(bbox.Min().x(), bbox.Max().y(), bbox.Min().z()));
		points.push_back(Point(bbox.Min().x(), bbox.Max().y(), bbox.Max().z()));
		points.push_back(Point(bbox.Max().x(), bbox.Min().y(), bbox.Min().z()));
		points.push_back(Point(bbox.Max().x(), bbox.Min().y(), bbox.Max().z()));
		points.push_back(Point(bbox.Max().x(), bbox.Max().y(), bbox.Min().z()));
		points.push_back(Point(bbox.Max().x(), bbox.Max().y(), bbox.Max().z()));

		Point p;
		for (size_t i = 0; i < points.size(); ++i)
		{
			p = mv.transform(points[i]);
			minz = p.z() < minz ? p.z() : minz;
			maxz = p.z() > maxz ? p.z() : maxz;
		}
		minz = fabs(minz);
		maxz = fabs(maxz);
		double start, end;
		start = minz < maxz ? minz : maxz;
		end = maxz > minz ? maxz : minz;
		getValue(gstPinRotCtr, bval);
		if (bval)
		{
			Point pin_ctr;
			getValue(gstRotCtrPin, pin_ctr);
			p = -mv.transform(pin_ctr);
			if (p.z() > start && p.z() < end)
				start = p.z();
		}
		setValue(gstDaStart, start);
		setValue(gstDaEnd, end);
	}
	else
	{
		double dist, radius, start, end;
		getValue(gstCamDist, dist);
		getValue(gstRadius, radius);
		start = dist - radius / 2;
		start = start < 0 ? 0 : start;
		end = dist + radius / 4;
		setValue(gstDaStart, start);
		setValue(gstDaEnd, end);
	}
}

//draw the volume data only
void Renderview::Draw()
{
	bool bval;
	long nx, ny;
	GetRenderSize(nx, ny);

	// clear color and depth buffers
	glClearDepth(1.0);
	Color cbg;
	getValue(gstBgColor, cbg);
	glClearColor(cbg.r(), cbg.g(), cbg.b(), 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, (GLint)nx, (GLint)ny);

	//gradient background
	getValue(gstGradBg, bval);
	if (bval)
		DrawGradBg();

	//projection
	HandleProjection(nx, ny, true);
	//Transformation
	HandleCamera(true);

	getValue(gstDrawAll, bval);
	if (!bval) return;

	glm::mat4 mv_temp = m_mv_mat;
	m_mv_mat = GetDrawMat();

	getValue(gstDepthAtten, bval);
	if (bval)
		CalcFogRange();
	getValue(gstDrawGrid, bval);
	if (bval)
		DrawGrid();
	getValue(gstDrawClip, bval);
	if (bval)
		DrawClippingPlanes(false, BACK_FACE);
	//setup
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//draw the volumes
	DrawVolumes();
	if (bval)//draw the clipping planes, paied boolean
		DrawClippingPlanes(true, FRONT_FACE);
	getValue(gstDrawBounds, bval);
	if (bval)
		DrawBounds();
	getValue(gstDrawAnnotations, bval);
	if (bval)
		DrawAnnotations();
	getValue(gstDrawRulers, bval);
	if (bval)
		DrawRulers();
	DrawCells();
	//traces
	DrawTraces();

	m_mv_mat = mv_temp;
}

//draw with depth peeling
void Renderview::DrawDP()
{
	bool bval;
	long nx, ny;
	GetRenderSize(nx, ny);
	std::string name;
	flvr::Framebuffer* peel_buffer = 0;

	//clear
	//	glDrawBuffer(GL_BACK);
	glClearDepth(1.0);
	Color cbg;
	getValue(gstBgColor, cbg);
	glClearColor(cbg.r(), cbg.g(), cbg.b(), 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, (GLint)nx, (GLint)ny);

	//gradient background
	getValue(gstGradBg, bval);
	if (bval)
		DrawGradBg();

	//projection
	HandleProjection(nx, ny, true);
	//Transformation
	HandleCamera(true);

	getValue(gstDrawAll, bval);
	if (!bval) return;

	glm::mat4 mv_temp = m_mv_mat;
	m_mv_mat = GetDrawMat();

	bool use_fog_save;
	getValue(gstDepthAtten, bval);
	use_fog_save = bval;
	if (bval)
		CalcFogRange();
	getValue(gstDrawGrid, bval);
	if (bval)
		DrawGrid();
	bool draw_clip;
	getValue(gstDrawClip, draw_clip);
	if (draw_clip)
		DrawClippingPlanes(true, BACK_FACE);

	//setup
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	setValue(gstDepthAtten, false);

	//draw depth values of each layer into the buffers
	long peelnum;
	getValue(gstPeelNum, peelnum);
	for (long i = 0; i < peelnum; i++)
	{
		name = "peel buffer" + std::to_string(i);
		peel_buffer =
			flvr::TextureRenderer::framebuffer_manager_.framebuffer(
				flvr::FB_Depth_Float, nx, ny, name);
		if (peel_buffer)
		{
			peel_buffer->bind();
			peel_buffer->protect();
		}

		glClearDepth(1.0);
		glClear(GL_DEPTH_BUFFER_BIT);

		if (i == 0)
		{
			DrawMeshes(0);
		}
		else
		{
			glActiveTexture(GL_TEXTURE15);
			name = "peel buffer" + std::to_string(i - 1);
			peel_buffer =
				flvr::TextureRenderer::framebuffer_manager_.framebuffer(name);
			if (peel_buffer)
				peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
			glActiveTexture(GL_TEXTURE0);
			DrawMeshes(1);
			glActiveTexture(GL_TEXTURE15);
			glBindTexture(GL_TEXTURE_2D, 0);
			glActiveTexture(GL_TEXTURE0);
		}
	}

	//bind back the framebuffer
	BindRenderBuffer();

	//restore fog
	setValue(gstDepthAtten, use_fog_save);

	//draw depth peeling
	for (long i = peelnum; i >= 0; i--)
	{
		if (i == 0)
		{
			//draw volumes before the depth
			glActiveTexture(GL_TEXTURE15);
			name = "peel buffer" + std::to_string(0);
			peel_buffer =
				flvr::TextureRenderer::framebuffer_manager_.framebuffer(name);
			if (peel_buffer)
				peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
			glActiveTexture(GL_TEXTURE0);
			DrawVolumes(1);
			glActiveTexture(GL_TEXTURE15);
			glBindTexture(GL_TEXTURE_2D, 0);
			glActiveTexture(GL_TEXTURE0);
		}
		else
		{
			if (peelnum == 1)
			{
				//i == m_peeling_layers == 1
				glActiveTexture(GL_TEXTURE15);
				name = "peel buffer" + std::to_string(0);
				peel_buffer =
					flvr::TextureRenderer::framebuffer_manager_.framebuffer(name);
				if (peel_buffer)
					peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
				glActiveTexture(GL_TEXTURE0);
			}
			else if (peelnum == 2)
			{
				glActiveTexture(GL_TEXTURE14);
				name = "peel buffer" + std::to_string(0);
				peel_buffer =
					flvr::TextureRenderer::framebuffer_manager_.framebuffer(name);
				if (peel_buffer)
					peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
				glActiveTexture(GL_TEXTURE15);
				name = "peel buffer" + std::to_string(1);
				peel_buffer =
					flvr::TextureRenderer::framebuffer_manager_.framebuffer(name);
				if (peel_buffer)
					peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
				glActiveTexture(GL_TEXTURE0);
			}
			else if (peelnum > 2)
			{
				if (i == peelnum)
				{
					glActiveTexture(GL_TEXTURE14);
					name = "peel buffer" + std::to_string(i - 2);
					peel_buffer =
						flvr::TextureRenderer::framebuffer_manager_.framebuffer(name);
					if (peel_buffer)
						peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
					glActiveTexture(GL_TEXTURE15);
					name = "peel buffer" + std::to_string(i - 1);
					peel_buffer =
						flvr::TextureRenderer::framebuffer_manager_.framebuffer(name);
					if (peel_buffer)
						peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
					glActiveTexture(GL_TEXTURE0);
				}
				else if (i == 1)
				{
					glActiveTexture(GL_TEXTURE14);
					name = "peel buffer" + std::to_string(0);
					peel_buffer =
						flvr::TextureRenderer::framebuffer_manager_.framebuffer(name);
					if (peel_buffer)
						peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
					glActiveTexture(GL_TEXTURE15);
					name = "peel buffer" + std::to_string(1);
					peel_buffer =
						flvr::TextureRenderer::framebuffer_manager_.framebuffer(name);
					if (peel_buffer)
						peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
					glActiveTexture(GL_TEXTURE0);
				}
				else
				{
					glActiveTexture(GL_TEXTURE13);
					name = "peel buffer" + std::to_string(i - 2);
					peel_buffer =
						flvr::TextureRenderer::framebuffer_manager_.framebuffer(name);
					if (peel_buffer)
						peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
					glActiveTexture(GL_TEXTURE14);
					name = "peel buffer" + std::to_string(i - 1);
					peel_buffer =
						flvr::TextureRenderer::framebuffer_manager_.framebuffer(name);
					if (peel_buffer)
						peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
					glActiveTexture(GL_TEXTURE15);
					name = "peel buffer" + std::to_string(i);
					peel_buffer =
						flvr::TextureRenderer::framebuffer_manager_.framebuffer(name);
					if (peel_buffer)
						peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
					glActiveTexture(GL_TEXTURE0);
				}
			}

			//draw volumes
			if (peelnum == 1)
				//i == m_peeling_layers == 1
				DrawVolumes(5);//draw volume after 15
			else if (peelnum == 2)
			{
				if (i == 2)
					DrawVolumes(2);//draw volume after 15
				else if (i == 1)
					DrawVolumes(4);//draw volume after 14 and before 15
			}
			else if (peelnum > 2)
			{
				if (i == peelnum)
					DrawVolumes(2);//draw volume after 15
				else if (i == 1)
					DrawVolumes(4);//draw volume after 14 and before 15
				else
					DrawVolumes(3);//draw volume after 14 and before 15
			}

			//draw meshes
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LEQUAL);
			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			if (peelnum == 1)
				//i == m_peeling_layers == 1
				DrawMeshes(5);//draw mesh at 15
			else if (peelnum == 2)
			{
				if (i == 2)
					DrawMeshes(2);//draw mesh after 14
				else if (i == 1)
					DrawMeshes(4);//draw mesh before 15
			}
			else if (peelnum > 2)
			{
				if (i == peelnum)
					DrawMeshes(2);//draw mesh after 14
				else if (i == 1)
					DrawMeshes(4);//draw mesh before 15
				else
					DrawMeshes(3);//draw mesh after 13 and before 15
			}

			glActiveTexture(GL_TEXTURE13);
			glBindTexture(GL_TEXTURE_2D, 0);
			glActiveTexture(GL_TEXTURE14);
			glBindTexture(GL_TEXTURE_2D, 0);
			glActiveTexture(GL_TEXTURE15);
			glBindTexture(GL_TEXTURE_2D, 0);
			glActiveTexture(GL_TEXTURE0);

		}
	}

	double darkness;
	if (GetMeshShadow(darkness))
		DrawOLShadowsMesh(darkness);

	if (draw_clip)
		DrawClippingPlanes(false, FRONT_FACE);
	getValue(gstDrawBounds, bval);
	if (bval)
		DrawBounds();
	getValue(gstDrawAnnotations, bval);
	if (bval)
		DrawAnnotations();
	getValue(gstDrawRulers, bval);
	if (bval)
		DrawRulers();
	DrawCells();
	//traces
	DrawTraces();

	m_mv_mat = mv_temp;
}

//draw meshes
//peel==true -- depth peeling
void Renderview::DrawMeshes(long peel)
{
	long nx, ny;
	GetRenderSize(nx, ny);
	GLint vp[4] = { 0, 0, (GLint)nx, (GLint)ny };
	bool use_fog;
	double fog_intensity, fog_start, fog_end;
	getValue(gstDepthAtten, use_fog);
	getValue(gstDaInt, fog_intensity);
	getValue(gstDaStart, fog_start);
	getValue(gstDaEnd, fog_end);

	for (size_t i = 0; i < getNumChildren(); i++)
	{
		if (MeshData* md = dynamic_cast<MeshData*>(getChild(i)))
		{
			bool disp;
			md->getValue(gstDisplay, disp);
			if (disp)
			{
				md->SetMatrices(m_mv_mat, m_proj_mat);
				md->setValue(gstDepthAtten, use_fog);
				md->setValue(gstDaInt, fog_intensity);
				md->setValue(gstDaStart, fog_start);
				md->setValue(gstDaEnd, fog_end);
				md->setValue(gstViewport, Vector4i(vp));
				md->Draw(peel);
			}
		}
		else if (MeshGroup* group = dynamic_cast<MeshGroup*>(getChild(i)))
		{
			bool disp;
			group->getValue(gstDisplay, disp);
			if (disp)
			{
				for (int j = 0; j < (int)group->getNumChildren(); j++)
				{
					MeshData* md = group->getChild(j)->asMeshData();
					if (!md) continue;
					bool disp;
					md->getValue(gstDisplay, disp);
					if (disp)
					{
						md->SetMatrices(m_mv_mat, m_proj_mat);
						md->setValue(gstDepthAtten, use_fog);
						md->setValue(gstDaInt, fog_intensity);
						md->setValue(gstDaStart, fog_start);
						md->setValue(gstDaEnd, fog_end);
						md->setValue(gstViewport, Vector4i(vp));
						md->Draw(peel);
					}
				}
			}
		}
	}
}

//draw volumes
//peel==true -- depth peeling
void Renderview::DrawVolumes(long peel)
{
	bool bval;
	int finished_bricks = 0;
	if (flvr::TextureRenderer::get_mem_swap())
	{
		finished_bricks = flvr::TextureRenderer::get_finished_bricks();
		flvr::TextureRenderer::reset_finished_bricks();
	}

	PrepFinalBuffer();

	long nx, ny;
	GetRenderSize(nx, ny);

	flrd::RulerPoint *p0 = m_ruler_handler->GetPoint();
	bool load_update, retain_finalbuffer, updating, force_clear, interactive, draw_mask;
	long inter_mode;
	getValue(gstLoadUpdate, load_update);
	getValue(gstRetainFb, retain_finalbuffer);
	getValue(gstUpdating, updating);
	getValue(gstForceClear, force_clear);
	getValue(gstInterMode, inter_mode);
	getValue(gstInteractive, interactive);
	getValue(gstDrawMask, draw_mask);
	VolumeData* cvol = GetCurrentVolume();
	//draw
	if (load_update ||
		(!retain_finalbuffer &&
			inter_mode != 2 &&
			inter_mode != 7 &&
			updating) ||
			(!retain_finalbuffer &&
		(inter_mode == 1 ||
			inter_mode == 3 ||
			inter_mode == 4 ||
			inter_mode == 5 ||
			((inter_mode == 6 ||
				inter_mode == 9) &&
				!p0) ||
			inter_mode == 8 ||
			force_clear)))
	{
		setValue(gstUpdating, false);
		setValue(gstForceClear, false);
		setValue(gstLoadUpdate, false);

		long update_order;
		getValue(gstUpdateOrder, update_order);
		if (update_order == 1)
		{
			if (interactive)
				ClearFinalBuffer();
			else
			{
				getValue(gstClearBuffer, bval);
				if (bval)
				{
					ClearFinalBuffer();
					setValue(gstClearBuffer, false);
				}
			}
		}
		else
			ClearFinalBuffer();

		GLboolean bCull = glIsEnabled(GL_CULL_FACE);
		glDisable(GL_CULL_FACE);

		PopVolumeList();

		VolumeList quota_vd_list;
		if (flvr::TextureRenderer::get_mem_swap())
		{
			//set start time for the texture renderer
			flvr::TextureRenderer::set_st_time(glbin_timer->get_ticks());

			flvr::TextureRenderer::set_interactive(interactive);
			//if in interactive mode, do interactive bricking also
			if (interactive)
			{
				//calculate quota
				long total_bricks = flvr::TextureRenderer::get_total_brick_num();
				long quota_bricks = 1;// total_bricks / 2;
				long fin_bricks = finished_bricks;
				long last_bricks = flvr::TextureRenderer::
					get_est_bricks(3);
				long adj_bricks = 0;
				unsigned long up_time = flvr::TextureRenderer::get_cor_up_time();
				unsigned long consumed_time = flvr::TextureRenderer::get_consumed_time();
				if (consumed_time == 0)
					quota_bricks = total_bricks;
				else if (consumed_time / up_time > total_bricks)
					quota_bricks = 1;
				else
				{
					adj_bricks = std::max(long(1), long(double(last_bricks) *
						double(up_time) / double(consumed_time)));
					quota_bricks = flvr::TextureRenderer::
						get_est_bricks(0, adj_bricks);
				}
				quota_bricks = std::min(total_bricks, quota_bricks);
				flvr::TextureRenderer::set_quota_bricks(quota_bricks);
				flvr::TextureRenderer::push_quota_brick(quota_bricks);
				////test
				//std::ofstream ofs("quota.txt", std::ios::out | std::ios::app);
				//std::string str;
				//str += std::to_string(quota_bricks) + "\t";
				//str += std::to_string(total_bricks) + "\t";
				//str += std::to_string(fin_bricks) + "\t";
				//str += std::to_string(adj_bricks) + "\t";
				//str += std::to_string(TextureRenderer::get_up_time()) + "\t";
				//str += std::to_string(up_time) + "\t";
				//str += std::to_string(consumed_time) + "\n";
				//ofs.write(str.c_str(), str.size());

				int quota_bricks_chan = 0;
				if (m_vol_list.size() > 1)
				{
					//priority: 1-selected channel; 2-group contains selected channel; 3-linear distance to above
					//not considering mask for now
					long cur_index, vd_index;
					getValue(gstCurVolIdx, cur_index);
					if (cur_index != -1)
					{
						VolumeData* vd = GetCurrentVolume();
						quota_vd_list.push_back(vd);
						long brick_num, count_bricks;
						vd->getValue(gstBrickNum, brick_num);
						count_bricks = brick_num;
						quota_bricks_chan = std::min(count_bricks, quota_bricks);
						vd->GetRenderer()->set_quota_bricks_chan(quota_bricks_chan);
						int count = 0;
						while (count_bricks < quota_bricks &&
							quota_vd_list.size() < m_vol_list.size())
						{
							if (count % 2 == 0)
								vd_index = cur_index + count / 2 + 1;
							else
								vd_index = cur_index - count / 2 - 1;
							count++;
							if (vd_index < 0 ||
								(size_t)vd_index >= m_vol_list.size())
								continue;
							vd = m_vol_list[vd_index].get();
							quota_vd_list.push_back(vd);
							if (count_bricks + brick_num > quota_bricks)
								quota_bricks_chan = quota_bricks - count_bricks;
							else
								quota_bricks_chan = brick_num;
							vd->GetRenderer()->set_quota_bricks_chan(quota_bricks_chan);
							count_bricks += quota_bricks_chan;
						}
					}
				}
				else if (m_vol_list.size() == 1)
				{
					quota_bricks_chan = quota_bricks;
					VolumeData* vd = m_vol_list[0].get();
					if (vd)
						vd->GetRenderer()->set_quota_bricks_chan(quota_bricks_chan);
				}

				//get and set center point
				VolumeData* vd = GetCurrentVolume();
				if (!vd)
					if (m_vol_list.size())
						vd = m_vol_list[0].get();
				m_volume_point->SetVolumeData(vd);
				Point p;
				if (m_volume_point->GetPointVolumeBox(nx / 2, ny / 2, false, p) > 0.0 ||
					(vd && m_volume_point->GetPointPlane(nx / 2, ny / 2, 0, false, p) > 0.0))
				{
					long resx, resy, resz;
					double sclx, scly, sclz;
					double spcx, spcy, spcz;
					vd->getValue(gstResX, resx);
					vd->getValue(gstResY, resy);
					vd->getValue(gstResZ, resz);
					vd->getValue(gstScaleX, sclx);
					vd->getValue(gstScaleY, scly);
					vd->getValue(gstScaleZ, sclz);
					vd->getValue(gstSpcX, spcx);
					vd->getValue(gstSpcY, spcy);
					vd->getValue(gstSpcZ, spcz);
					p = Point(p.x() / (resx*sclx*spcx),
						p.y() / (resy*scly*spcy),
						p.z() / (resz*sclz*spcz));
					flvr::TextureRenderer::set_qutoa_center(p);
				}
				else
					flvr::TextureRenderer::set_interactive(false);
			}
		}

		//handle intermixing modes
		long mix_method;
		getValue(gstMixMethod, mix_method);
		if (mix_method == MIX_METHOD_MULTI)
		{
			if (flvr::TextureRenderer::get_mem_swap() &&
				flvr::TextureRenderer::get_interactive() &&
				quota_vd_list.size() > 0)
				DrawVolumesMulti(quota_vd_list, peel);
			else
				DrawVolumesMulti(GetVolList(), peel);
			//draw masks
			if (draw_mask)
				DrawVolumesComp(GetVolList(), true, peel);
		}
		else
		{
			int i, j;
			VolumeList list;
			for (i = getNumChildren() - 1; i >= 0; i--)
			{
				if (VolumeData* vd = dynamic_cast<VolumeData*>(getChild(i)))
				{
					bool disp;
					vd->getValue(gstDisplay, disp);
					if (disp)
					{
						if (flvr::TextureRenderer::get_mem_swap() &&
							flvr::TextureRenderer::get_interactive() &&
							quota_vd_list.size() > 0)
						{
							if (std::find(quota_vd_list.begin(),
								quota_vd_list.end(), vd) !=
								quota_vd_list.end())
								list.push_back(vd);
						}
						else
							list.push_back(vd);
					}
				}
				else if (VolumeGroup* group = dynamic_cast<VolumeGroup*>(getChild(i)))
				{
					if (!list.empty())
					{
						DrawVolumesComp(list, false, peel);
						//draw masks
						if (draw_mask)
							DrawVolumesComp(list, true, peel);
						list.clear();
					}

					bool disp;
					group->getValue(gstDisplay, disp);
					if (!disp)
						continue;
					for (j = group->getNumChildren() - 1; j >= 0; j--)
					{
						VolumeData* vd = group->getChild(j)->asVolumeData();
						if (!vd)
							continue;
						bool disp;
						vd->getValue(gstDisplay, disp);
						if (disp)
						{
							if (flvr::TextureRenderer::get_mem_swap() &&
								flvr::TextureRenderer::get_interactive() &&
								quota_vd_list.size() > 0)
							{
								if (std::find(quota_vd_list.begin(),
									quota_vd_list.end(), vd) !=
									quota_vd_list.end())
									list.push_back(vd);
							}
							else
								list.push_back(vd);
						}
					}
					if (!list.empty())
					{
						long blend_mode;
						group->getValue(gstBlendMode, blend_mode);
						if (blend_mode == MIX_METHOD_MULTI)
							DrawVolumesMulti(list, peel);
						else
							DrawVolumesComp(list, false, peel);
						//draw masks
						if (draw_mask)
							DrawVolumesComp(list, true, peel);
						list.clear();
					}
				}
			}
		}

		if (bCull) glEnable(GL_CULL_FACE);
	}

	//final composition
	DrawFinalBuffer();

	if (flvr::TextureRenderer::get_mem_swap())
	{
		flvr::TextureRenderer::set_consumed_time(glbin_timer->get_ticks() - flvr::TextureRenderer::get_st_time());
		if (flvr::TextureRenderer::get_start_update_loop() &&
			flvr::TextureRenderer::get_done_update_loop())
			flvr::TextureRenderer::reset_update_loop();
	}

	if (interactive)
	{
		//wxMouseState ms = wxGetMouseState();
		//if (ms.LeftIsDown() ||
		//	ms.MiddleIsDown() ||
		//	ms.RightIsDown())
		//	return;
		setValue(gstInteractive, false);
		setValue(gstClearBuffer, true);
		//RefreshGL(2);
	}

	//if (TextureRenderer::get_mem_swap())
	//{
	//	if (finished_bricks == 0)
	//	{
	//		if (m_nodraw_count == 100)
	//		{
	//			TextureRenderer::set_done_update_loop();
	//			m_nodraw_count = 0;
	//		}
	//		else
	//			m_nodraw_count++;
	//	}
	//}
}

void Renderview::DrawAnnotations()
{
	long nx, ny;
	GetRenderSize(nx, ny);
	Transform mv;
	Transform p;
	mv.set(glm::value_ptr(m_mv_mat));
	p.set(glm::value_ptr(m_proj_mat));
	Color text_color;
	getValue(gstTextColor, text_color);
	bool persp;
	getValue(gstPerspective, persp);

	for (size_t i = 0; i < getNumChildren(); i++)
	{
		if (Annotations* ann = dynamic_cast<Annotations*>(getChild(i)))
		{
			bool disp;
			ann->getValue(gstDisplay, disp);
			if (disp)
			{
				ann->setValue(gstColor, text_color);
				ann->Draw(nx, ny, mv, p, persp);
			}
		}
	}
}

//vr buffers
void Renderview::GetRenderSize(long &nx, long &ny)
{
	bool bval;
	getValue(gstOpenvrEnable, bval);
	if (bval)
	{
		getValue(gstVrSizeX, nx);
		getValue(gstVrSizeY, ny);
	}
	else
	{
		getValue(gstSizeX, nx);
		getValue(gstSizeY, ny);
		getValue(gstVrEnable, bval);
		if (bval)
			nx /= 2;
	}
}

//draw out the framebuffer after composition
void Renderview::PrepFinalBuffer()
{
	long nx, ny;
	GetRenderSize(nx, ny);

	//generate textures & buffer objects
	glActiveTexture(GL_TEXTURE0);
	//glEnable(GL_TEXTURE_2D);
	//frame buffer for final
	flvr::Framebuffer* final_buffer =
		flvr::TextureRenderer::framebuffer_manager_.framebuffer(
			flvr::FB_Render_RGBA, nx, ny, "final");
	if (final_buffer)
		final_buffer->protect();
}

void Renderview::ClearFinalBuffer()
{
	flvr::Framebuffer* final_buffer =
		flvr::TextureRenderer::framebuffer_manager_.framebuffer(
			"final");
	if (final_buffer)
		final_buffer->bind();
	//clear color buffer to black for compositing
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
}

void Renderview::DrawFinalBuffer()
{
	bool bval;
	getValue(gstEnlarge, bval);
	if (bval)
		return;

	//bind back the window frame buffer
	BindRenderBuffer();

	//draw the final buffer to the windows buffer
	glActiveTexture(GL_TEXTURE0);
	flvr::Framebuffer* final_buffer =
		flvr::TextureRenderer::framebuffer_manager_.framebuffer("final");
	if (final_buffer)
		final_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	//glBlendFunc(GL_ONE, GL_ONE);
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
	double r, g, b;
	getValue(gstGammaR, r); getValue(gstGammaG, g); getValue(gstGammaB, b);
	img_shader->setLocalParam(0, r, g, b, 1.0);
	getValue(gstBrightnessR, r); getValue(gstBrightnessG, g); getValue(gstBrightnessB, b);
	img_shader->setLocalParam(1, r, g, b, 1.0);
	getValue(gstEqualizeR, r); getValue(gstEqualizeG, g); getValue(gstEqualizeB, b);
	img_shader->setLocalParam(2, r, g, b, 0.0);
	//2d adjustment

	DrawViewQuad();

	if (img_shader && img_shader->valid())
		img_shader->release();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	//get pixel value
	bool pin_rc, rc_dirty, free;
	getValue(gstPinRotCtr, pin_rc);
	getValue(gstRotCtrDirty, rc_dirty);
	getValue(gstFree, free);
	if (pin_rc && rc_dirty && !free)
	{
		unsigned char pixel[4];
		long nx, ny;
		GetRenderSize(nx, ny);
		glReadPixels(nx / 2, ny / 2, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
		setValue(gstPinThresh, 0.8 * double(pixel[3]) / 255.0);
	}
}

void Renderview::PrepVRBuffer()
{
	bool bval;
	getValue(gstOpenvrEnable, bval);
	if (bval)
	{
#ifdef _WIN32
		std::array<vr::TrackedDevicePose_t, vr::k_unMaxTrackedDeviceCount> tracked_device_poses;
		vr::VRCompositor()->WaitGetPoses(tracked_device_poses.data(), tracked_device_poses.size(), NULL, 0);
#endif
	}

	long nx, ny;
	GetRenderSize(nx, ny);

	//generate textures & buffer objects
	glActiveTexture(GL_TEXTURE0);
	//glEnable(GL_TEXTURE_2D);
	//frame buffer for one eye
	std::string vr_buf_name;
	long lval;
	getValue(gstVrEyeIdx, lval);
	if (lval)
		vr_buf_name = "vr right";
	else
		vr_buf_name = "vr left";

	flvr::Framebuffer* vr_buffer =
		flvr::TextureRenderer::framebuffer_manager_.framebuffer(
			flvr::FB_UChar_RGBA, nx, ny, vr_buf_name);
	if (vr_buffer)
		vr_buffer->protect();
}

void Renderview::BindRenderBuffer()
{
	bool bval;
	getValue(gstVrEnable, bval);
	if (bval)
	{
		std::string vr_buf_name;
		long lval;
		getValue(gstVrEyeIdx, lval);
		if (lval)
			vr_buf_name = "vr right";
		else
			vr_buf_name = "vr left";
		flvr::Framebuffer* vr_buffer =
			flvr::TextureRenderer::framebuffer_manager_.framebuffer(
				vr_buf_name);
		if (vr_buffer)
			vr_buffer->bind();
	}
	else
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderview::ClearVRBuffer()
{
	BindRenderBuffer();
	//clear color buffer to black for compositing
	glClearDepth(1.0);
	Color c;
	getValue(gstBgColor, c);
	glClearColor(c.r(), c.g(), c.b(), 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderview::DrawVRBuffer()
{
	long vr_x, vr_y, gl_x, gl_y;
	GetRenderSize(vr_x, vr_y);
	getValue(gstSizeX, gl_x);
	getValue(gstSizeY, gl_y);
	int vp_y = int((double)gl_x * vr_y / vr_x / 2.0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, gl_x, vp_y);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	flvr::ShaderProgram* img_shader =
		flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHADER_TEXTURE_LOOKUP);
	if (img_shader)
	{
		if (!img_shader->valid())
			img_shader->create();
		img_shader->bind();
	}
	//left eye
	flvr::Framebuffer* buffer =
		flvr::TextureRenderer::framebuffer_manager_.framebuffer(
			"vr left");
	if (buffer)
		buffer->bind_texture(GL_COLOR_ATTACHMENT0);
	flvr::VertexArray* quad_va =
		flvr::TextureRenderer::vertex_array_manager_.vertex_array(flvr::VA_Left_Square);
	if (quad_va)
		quad_va->draw();
	//openvr left eye
	bool bval;
	getValue(gstOpenvrEnable, bval);
	if (bval)
	{
#ifdef _WIN32
		vr::Texture_t left_eye = {};
		left_eye.handle = reinterpret_cast<void*>(buffer->tex_id(GL_COLOR_ATTACHMENT0));
		left_eye.eType = vr::TextureType_OpenGL;
		left_eye.eColorSpace = vr::ColorSpace_Gamma;
		vr::VRCompositor()->Submit(vr::Eye_Left, &left_eye, nullptr);
#endif
	}
	//right eye
	buffer =
		flvr::TextureRenderer::framebuffer_manager_.framebuffer(
			"vr right");
	if (buffer)
		buffer->bind_texture(GL_COLOR_ATTACHMENT0);
	quad_va =
		flvr::TextureRenderer::vertex_array_manager_.vertex_array(flvr::VA_Right_Square);
	if (quad_va)
		quad_va->draw();
	//openvr left eye
	if (bval)
	{
#ifdef _WIN32
		vr::Texture_t right_eye = {};
		right_eye.handle = reinterpret_cast<void*>(buffer->tex_id(GL_COLOR_ATTACHMENT0));
		right_eye.eType = vr::TextureType_OpenGL;
		right_eye.eColorSpace = vr::ColorSpace_Gamma;
		vr::VRCompositor()->Submit(vr::Eye_Right, &right_eye, nullptr);
#endif
	}

	if (img_shader && img_shader->valid())
		img_shader->release();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

//draw multi volumes with depth consideration
//peel==true -- depth peeling
void Renderview::DrawVolumesMulti(VolumeList &list, long peel)
{
	if (list.empty())
		return;

	if (!m_mvr)
		m_mvr = new flvr::MultiVolumeRenderer();
	if (!m_mvr)
		return;

	long nx, ny;
	GetRenderSize(nx, ny);
	GLint vp[4] = { 0, 0, (GLint)nx, (GLint)ny };
	GLfloat clear_color[4] = { 0, 0, 0, 0 };
	double dval;
	getValue(gstScaleFactor, dval);
	GLfloat zoom = dval;
	GLfloat sf121 = Get121ScaleFactor();
	bool bval;
	getValue(gstMicroBlendEnable, bval);
	m_mvr->set_blend_slices(bval);
	bool use_fog;
	double fog_intensity, fog_start, fog_end;
	getValue(gstDepthAtten, use_fog);
	getValue(gstDaInt, fog_intensity);
	getValue(gstDaStart, fog_start);
	getValue(gstDaEnd, fog_end);

	int i;
	m_mvr->clear_vr();
	for (i = 0; i < (int)list.size(); i++)
	{
		VolumeData* vd = list[i];
		if (!vd)
			continue;
		bool disp;
		vd->getValue(gstDisplay, disp);
		if (disp)
		{
			flvr::VolumeRenderer* vr = vd->GetRenderer();
			if (vr)
			{
				//drawlabel
				long label_mode;
				vd->getValue(gstLabelMode, label_mode);
				if (label_mode &&
					vd->GetMask(false) &&
					vd->GetLabel(false))
					vd->setValue(gstMaskMode, long(4));
				vd->SetMatrices(m_mv_mat, m_proj_mat, m_tex_mat);
				vd->setValue(gstDepthAtten, use_fog);
				vd->setValue(gstDaInt, fog_intensity);
				vd->setValue(gstDaStart, fog_start);
				vd->setValue(gstDaEnd, fog_end);
				vr->set_zoom(zoom, sf121);
				m_mvr->add_vr(vr);
				m_mvr->set_sampling_rate(vr->get_sampling_rate());
				m_mvr->SetNoiseRed(vr->GetNoiseRed());
			}
		}
	}

	if (m_mvr->get_vr_num() <= 0)
		return;
	m_mvr->set_depth_peel(peel);

	// Set up transform
	Transform *tform = list[0]->GetTexture()->transform();
	float mvmat[16];
	tform->get_trans(mvmat);
	glm::mat4 mv_mat2 = glm::mat4(
		mvmat[0], mvmat[4], mvmat[8], mvmat[12],
		mvmat[1], mvmat[5], mvmat[9], mvmat[13],
		mvmat[2], mvmat[6], mvmat[10], mvmat[14],
		mvmat[3], mvmat[7], mvmat[11], mvmat[15]);
	mv_mat2 = list[0]->GetRenderer()->m_mv_mat * mv_mat2;
	m_mvr->set_matrices(mv_mat2,
		list[0]->GetRenderer()->m_proj_mat,
		list[0]->GetRenderer()->m_tex_mat);

	//generate textures & buffer objects
	//frame buffer for each volume
	flvr::Framebuffer* chann_buffer =
		flvr::TextureRenderer::framebuffer_manager_.framebuffer(
			flvr::FB_Render_RGBA, nx, ny, "channel");
	//bind the fbo
	unsigned long ulval;
	if (chann_buffer)
	{
		chann_buffer->protect();
		chann_buffer->bind();
		ulval = (unsigned long)(chann_buffer->id());
		setValue(gstCurFramebuffer, ulval);
	}
	if (!flvr::TextureRenderer::get_mem_swap() ||
		(flvr::TextureRenderer::get_mem_swap() &&
			flvr::TextureRenderer::get_clear_chan_buffer()))
	{
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT);
		flvr::
			TextureRenderer::reset_clear_chan_buffer();
	}

	//draw multiple volumes at the same time
	m_mvr->set_viewport(vp);
	m_mvr->set_clear_color(clear_color);
	getValue(gstCurFramebuffer, ulval);
	m_mvr->set_cur_framebuffer(ulval);
	bool test_wiref, adaptive, interactive, persp, interpolate;
	getValue(gstTestWiref, test_wiref);
	getValue(gstAdaptive, adaptive);
	getValue(gstInteractive, interactive);
	getValue(gstPerspective, persp);
	getValue(gstInterpolate, interpolate);
	m_mvr->draw(test_wiref, adaptive, interactive, !persp, interpolate);

	//draw shadows
	DrawOLShadows(list);

	//bind fbo for final composition
	flvr::Framebuffer* final_buffer =
		flvr::TextureRenderer::framebuffer_manager_.framebuffer(
			"final");
	if (final_buffer)
		final_buffer->bind();
	glActiveTexture(GL_TEXTURE0);
	if (chann_buffer)
		chann_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
	//build mipmap
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glEnable(GL_BLEND);
	long lval;
	getValue(gstMixMethod, lval);
	if (lval == MIX_METHOD_COMP)
		glBlendFunc(GL_ONE, GL_ONE);
	else
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	//2d adjustment
	flvr::ShaderProgram* img_shader =
		flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHDR_BRIGHTNESS_CONTRAST_HDR);
	if (img_shader)
	{
		if (!img_shader->valid())
		{
			img_shader->create();
		}
		img_shader->bind();
	}
	VolumeData* vd = list[0];
	double r, g, b;
	vd->getValue(gstGammaR, r);
	vd->getValue(gstGammaG, g);
	vd->getValue(gstGammaB, b);
	Color gamma(r, g, b);
	vd->getValue(gstBrightnessR, r);
	vd->getValue(gstBrightnessG, g);
	vd->getValue(gstBrightnessB, b);
	Color brightness(r, g, b);
	vd->getValue(gstEqualizeR, r);
	vd->getValue(gstEqualizeG, g);
	vd->getValue(gstEqualizeB, b);
	Color hdr(r, g, b);
	img_shader->setLocalParam(0, gamma.r(), gamma.g(), gamma.b(), 1.0);
	img_shader->setLocalParam(1, brightness.r(), brightness.g(), brightness.b(), 1.0);
	img_shader->setLocalParam(2, hdr.r(), hdr.g(), hdr.b(), 0.0);
	//2d adjustment

	DrawViewQuad();

	if (img_shader && img_shader->valid())
		img_shader->release();
}

//Draw the volmues with compositing
//peel==true -- depth peeling
void Renderview::DrawVolumesComp(VolumeList &list, bool mask, long peel)
{
	if (list.empty()
		)
		return;

	int i;

	//count volumes with mask
	int cnt_mask = 0;
	for (i = 0; i < (int)list.size(); i++)
	{
		VolumeData* vd = list[i];
		if (!vd)
			continue;
		bool disp;
		vd->getValue(gstDisplay, disp);
		if (!disp)
			continue;
		if (vd->GetTexture() && vd->GetTexture()->nmask() != -1)
			cnt_mask++;
	}

	if (mask && cnt_mask == 0)
		return;

	long nx, ny;
	GetRenderSize(nx, ny);

	//generate textures & buffer objects
	//frame buffer for each volume
	flvr::Framebuffer* chann_buffer =
		flvr::TextureRenderer::framebuffer_manager_.framebuffer(
			flvr::FB_Render_RGBA, nx, ny, "channel");
	if (chann_buffer)
		chann_buffer->protect();

	//draw each volume to fbo
	for (i = 0; i < (int)list.size(); i++)
	{
		VolumeData* vd = list[i];
		if (!vd)
			continue;
		bool disp;
		vd->getValue(gstDisplay, disp);
		if (!disp)
			continue;
		if (mask)
		{
			//drawlabel
			long label_mode;
			vd->getValue(gstLabelMode, label_mode);
			if (label_mode &&
				vd->GetMask(false) &&
				vd->GetLabel(false))
				continue;

			if (vd->GetTexture() && vd->GetTexture()->nmask() != -1)
			{
				vd->setValue(gstMaskMode, long(1));
				long mix_method;
				getValue(gstMixMethod, mix_method);
				setValue(gstMixMethod, MIX_METHOD_COMP);
				long mip_mode;
				vd->getValue(gstMipMode, mip_mode);
				if (mip_mode == 1)
					DrawMIP(vd, peel);
				else
					DrawOVER(vd, mask, peel);
				vd->setValue(gstMaskMode, long(0));
				setValue(gstMixMethod, mix_method);
			}
		}
		else
		{
			long blend_mode;
			vd->getValue(gstBlendMode, blend_mode);
			if (blend_mode != 2)
			{
				//drawlabel
				long label_mode;
				vd->getValue(gstLabelMode, label_mode);
				if (label_mode &&
					vd->GetMask(false) &&
					vd->GetLabel(false))
					vd->setValue(gstMaskMode, long(4));

				long mip_mode;
				vd->getValue(gstMipMode, mip_mode);
				if (mip_mode == 1)
					DrawMIP(vd, peel);
				else
					DrawOVER(vd, mask, peel);
			}
		}
	}
}

void Renderview::DrawMIP(VolumeData* vd, long peel)
{
	long nx, ny;
	GetRenderSize(nx, ny);
	GLint vp[4] = { 0, 0, (GLint)nx, (GLint)ny };
	GLfloat clear_color[4] = { 0, 0, 0, 0 };

	bool do_mip = true;
	if (flvr::TextureRenderer::get_mem_swap() &&
		flvr::TextureRenderer::get_start_update_loop() &&
		!flvr::TextureRenderer::get_done_update_loop())
	{
		unsigned int rn_time = glbin_timer->get_ticks();
		if (rn_time - flvr::TextureRenderer::get_st_time() >
			flvr::TextureRenderer::get_up_time())
			return;
		if (vd->GetRenderer()->get_done_loop(1))
			do_mip = false;
	}

	bool shading;
	vd->getValue(gstShadingEnable, shading);
	bool shadow;
	vd->getValue(gstShadowEnable, shadow);
	long color_mode;
	vd->getValue(gstColormapMode, color_mode);
	bool enable_alpha;
	vd->getValue(gstAlphaEnable, enable_alpha);
	long saved_mip_mode;
	vd->getValue(gstMipMode, saved_mip_mode);

	flvr::ShaderProgram* img_shader = 0;
	flvr::Framebuffer* chann_buffer =
		flvr::TextureRenderer::framebuffer_manager_.framebuffer("channel");
	flvr::Framebuffer* overlay_buffer = 0;
	if (do_mip)
	{
		//before rendering this channel, save final buffer to temp buffer
		if (flvr::TextureRenderer::get_mem_swap() &&
			flvr::TextureRenderer::get_start_update_loop() &&
			flvr::TextureRenderer::get_save_final_buffer())
		{
			flvr::TextureRenderer::reset_save_final_buffer();

			//bind temporary framebuffer for comp in stream mode
			flvr::Framebuffer* temp_buffer =
				flvr::TextureRenderer::framebuffer_manager_.framebuffer(
					flvr::FB_Render_RGBA, nx, ny, "temporary");
			if (temp_buffer)
			{
				temp_buffer->bind();
				temp_buffer->protect();
			}
			glClearColor(0.0, 0.0, 0.0, 0.0);
			glClear(GL_COLOR_BUFFER_BIT);
			glActiveTexture(GL_TEXTURE0);
			flvr::Framebuffer* final_buffer =
				flvr::TextureRenderer::framebuffer_manager_.framebuffer("final");
			if (final_buffer)
				final_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
			glDisable(GL_BLEND);
			glDisable(GL_DEPTH_TEST);

			img_shader =
				flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHADER_TEXTURE_LOOKUP);
			if (img_shader)
			{
				if (!img_shader->valid())
					img_shader->create();
				img_shader->bind();
			}
			DrawViewQuad();
			if (img_shader && img_shader->valid())
				img_shader->release();

			glBindTexture(GL_TEXTURE_2D, 0);
		}

		//bind the fbo
		overlay_buffer =
			flvr::TextureRenderer::framebuffer_manager_.framebuffer(
				flvr::FB_Render_RGBA, nx, ny);
		if (overlay_buffer)
		{
			overlay_buffer->bind();
			overlay_buffer->protect();
			setValue(gstCurFramebuffer, (unsigned long)(overlay_buffer->id()));
		}

		if (!flvr::TextureRenderer::get_mem_swap() ||
			(flvr::TextureRenderer::get_mem_swap() &&
				flvr::TextureRenderer::get_clear_chan_buffer()))
		{
			glClearColor(0.0, 0.0, 0.0, 0.0);
			glClear(GL_COLOR_BUFFER_BIT);
			flvr::TextureRenderer::reset_clear_chan_buffer();
		}

		if (vd->GetRenderer())
			vd->GetRenderer()->set_depth_peel(peel);
		vd->GetRenderer()->set_shading(false);
		//turn off colormap proj
		long saved_colormap_proj;
		vd->getValue(gstColormapProj, saved_colormap_proj);
		if (color_mode == 0)
			vd->setValue(gstColormapProj, long(0));
		if (color_mode == 1)
		{
			vd->setValue(gstMipMode, long(3));
			vd->setValue(gstDepthAtten, false);
		}
		else
		{
			vd->setValue(gstMipMode, long(1));
			ValueCollection fog = { gstDepthAtten, gstDaInt, gstDaStart, gstDaEnd };
			propValues(fog, vd);
		}
		//turn off alpha
		if (color_mode == 1)
			vd->setValue(gstAlphaEnable, false);
		//draw
		vd->setValue(gstStreamMode, long(1));
		vd->SetMatrices(m_mv_mat, m_proj_mat, m_tex_mat);
		vd->setValue(gstViewport, Vector4i(vp));
		vd->setValue(gstClearColor, Vector4f(clear_color));
		propValue(gstCurFramebuffer, vd);
		bool adaptive, interactive, persp;
		double scale_factor;
		getValue(gstAdaptive, adaptive);
		getValue(gstInteractive, interactive);
		getValue(gstPerspective, persp);
		getValue(gstScaleFactor, scale_factor);
		vd->Draw(!persp, adaptive, interactive, scale_factor, Get121ScaleFactor());
		//restore
		if (color_mode == 0)
			vd->setValue(gstColormapProj, saved_colormap_proj);
		if (color_mode == 1)
		{
			vd->setValue(gstMipMode, saved_mip_mode);
			//restore alpha
			vd->setValue(gstAlphaEnable, enable_alpha);
		}

		//bind channel fbo for final composition
		if (chann_buffer)
			chann_buffer->bind();
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
		if (overlay_buffer)
		{
			//ok to unprotect
			overlay_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
			overlay_buffer->unprotect();
		}
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

		if (color_mode == 1)
		{
			long cproj;
			vd->getValue(gstColormapProj, cproj);
			long ctype;
			vd->getValue(gstColormapType, ctype);
			//2d adjustment
			if (cproj)
				img_shader = flvr::TextureRenderer::img_shader_factory_.shader(
					IMG_SHDR_GRADIENT_PROJ_MAP, ctype);
			else
				img_shader = flvr::TextureRenderer::img_shader_factory_.shader(
					IMG_SHDR_GRADIENT_MAP, ctype);
			if (img_shader)
			{
				if (!img_shader->valid())
				{
					img_shader->create();
				}
				img_shader->bind();
			}
			double lo, hi;
			vd->getValue(gstColormapLow, lo);
			vd->getValue(gstColormapHigh, hi);
			img_shader->setLocalParam(
				0, lo, hi, hi - lo, enable_alpha ? 0.0 : 1.0);
			Color c;
			vd->getValue(gstColor, c);
			double cinv;
			vd->getValue(gstColormapInv, cinv);
			img_shader->setLocalParam(
				6, c.r(), c.g(), c.b(), cinv);
			img_shader->setLocalParam(
				9, c.r(), c.g(), c.b(), 0.0);
			//2d adjustment
		}
		else
		{
			img_shader =
				flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHADER_TEXTURE_LOOKUP);
		}

		if (img_shader)
		{
			if (!img_shader->valid())
				img_shader->create();
			img_shader->bind();
		}
		DrawViewQuad();
		if (img_shader && img_shader->valid())
			img_shader->release();

		if (color_mode == 1 &&
			img_shader &&
			img_shader->valid())
		{
			img_shader->release();
		}
	}

	if (shading)
	{
		DrawOLShading(vd);
	}

	if (shadow)
	{
		VolumeList list;
		list.push_back(vd);
		DrawOLShadows(list);
	}

	//bind fbo for final composition
	flvr::Framebuffer* final_buffer =
		flvr::TextureRenderer::framebuffer_manager_.framebuffer(
			"final");
	if (final_buffer)
		final_buffer->bind();

	if (flvr::TextureRenderer::get_mem_swap())
	{
		//restore temp buffer to final buffer
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
		flvr::Framebuffer* temp_buffer =
			flvr::TextureRenderer::framebuffer_manager_.framebuffer(
				"temporary");
		if (temp_buffer)
		{
			//bind tex from temp buffer
			//it becomes unprotected afterwards
			temp_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
			temp_buffer->unprotect();
		}
		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);

		img_shader =
			flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHADER_TEXTURE_LOOKUP);
		if (img_shader)
		{
			if (!img_shader->valid())
				img_shader->create();
			img_shader->bind();
		}
		DrawViewQuad();
		if (img_shader && img_shader->valid())
			img_shader->release();

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glActiveTexture(GL_TEXTURE0);
	if (chann_buffer)
		chann_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
	//build mipmap
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glEnable(GL_BLEND);
	long lval;
	getValue(gstMixMethod, lval);
	if (lval == MIX_METHOD_COMP)
		glBlendFunc(GL_ONE, GL_ONE);
	else
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	//2d adjustment
	img_shader =
		flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHDR_BRIGHTNESS_CONTRAST_HDR);
	if (img_shader)
	{
		if (!img_shader->valid())
			img_shader->create();
		img_shader->bind();
	}
	double r, g, b;
	vd->getValue(gstGammaR, r);
	vd->getValue(gstGammaG, g);
	vd->getValue(gstGammaB, b);
	Color gamma(r, g, b);
	vd->getValue(gstBrightnessR, r);
	vd->getValue(gstBrightnessG, g);
	vd->getValue(gstBrightnessB, b);
	Color brightness(r, g, b);
	vd->getValue(gstEqualizeR, r);
	vd->getValue(gstEqualizeG, g);
	vd->getValue(gstEqualizeB, b);
	Color hdr(r, g, b);
	img_shader->setLocalParam(0, gamma.r(), gamma.g(), gamma.b(), 1.0);
	img_shader->setLocalParam(1, brightness.r(), brightness.g(), brightness.b(), 1.0);
	img_shader->setLocalParam(2, hdr.r(), hdr.g(), hdr.b(), 0.0);
	//2d adjustment

	DrawViewQuad();

	if (img_shader && img_shader->valid())
		img_shader->release();

	vd->setValue(gstShadingEnable, shading);
	vd->setValue(gstColormapMode, color_mode);

	//if vd is duplicated
	if (flvr::TextureRenderer::get_mem_swap() &&
		flvr::TextureRenderer::get_done_current_chan())
	{
		vector<flvr::TextureBrick*> *bricks =
			vd->GetTexture()->get_bricks();
		for (int i = 0; i < bricks->size(); i++)
			(*bricks)[i]->set_drawn(false);
	}
}

void Renderview::DrawOVER(VolumeData* vd, bool mask, int peel)
{
	long nx, ny;
	GetRenderSize(nx, ny);
	GLint vp[4] = { 0, 0, (GLint)nx, (GLint)ny };
	GLfloat clear_color[4] = { 0, 0, 0, 0 };

	flvr::ShaderProgram* img_shader = 0;

	bool do_over = true;
	if (flvr::TextureRenderer::get_mem_swap() &&
		flvr::TextureRenderer::get_start_update_loop() &&
		!flvr::TextureRenderer::get_done_update_loop())
	{
		unsigned int rn_time = glbin_timer->get_ticks();
		if (rn_time - flvr::TextureRenderer::get_st_time() >
			flvr::TextureRenderer::get_up_time())
			return;
		if (mask)
		{
			if (vd->GetRenderer()->get_done_loop(4))
				do_over = false;
		}
		else
		{
			if (vd->GetRenderer()->get_done_loop(0))
				do_over = false;
		}
	}

	flvr::Framebuffer* chann_buffer =
		flvr::TextureRenderer::framebuffer_manager_.framebuffer("channel");
	if (do_over)
	{
		//before rendering this channel, save final buffer to temp buffer
		if (flvr::TextureRenderer::get_mem_swap() &&
			flvr::TextureRenderer::get_start_update_loop() &&
			flvr::TextureRenderer::get_save_final_buffer())
		{
			flvr::TextureRenderer::reset_save_final_buffer();

			//bind temporary framebuffer for comp in stream mode
			flvr::Framebuffer* temp_buffer =
				flvr::TextureRenderer::framebuffer_manager_.framebuffer(
					flvr::FB_Render_RGBA, nx, ny, "temporary");
			if (temp_buffer)
			{
				temp_buffer->bind();
				temp_buffer->protect();
			}
			glClearColor(0.0, 0.0, 0.0, 0.0);
			glClear(GL_COLOR_BUFFER_BIT);
			glActiveTexture(GL_TEXTURE0);
			flvr::Framebuffer* final_buffer =
				flvr::TextureRenderer::framebuffer_manager_.framebuffer("final");
			if (final_buffer)
				final_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
			glDisable(GL_BLEND);
			glDisable(GL_DEPTH_TEST);

			img_shader =
				flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHADER_TEXTURE_LOOKUP);
			if (img_shader)
			{
				if (!img_shader->valid())
					img_shader->create();
				img_shader->bind();
			}
			DrawViewQuad();
			if (img_shader && img_shader->valid())
				img_shader->release();

			glBindTexture(GL_TEXTURE_2D, 0);
		}
		//bind the fbo
		if (chann_buffer)
		{
			chann_buffer->bind();
			setValue(gstCurFramebuffer, (unsigned long)(chann_buffer->id()));
		}

		if (!flvr::TextureRenderer::get_mem_swap() ||
			(flvr::TextureRenderer::get_mem_swap() &&
				flvr::TextureRenderer::get_clear_chan_buffer()))
		{
			glClearColor(0.0, 0.0, 0.0, 0.0);
			glClear(GL_COLOR_BUFFER_BIT);
			flvr::TextureRenderer::reset_clear_chan_buffer();
		}

		if (vd->GetRenderer())
			vd->GetRenderer()->set_depth_peel(peel);
		if (mask)
			vd->setValue(gstStreamMode, long(4));
		else
			vd->setValue(gstStreamMode, long(0));
		vd->SetMatrices(m_mv_mat, m_proj_mat, m_tex_mat);
		ValueCollection fog = { gstDepthAtten, gstDaInt, gstDaStart, gstDaEnd };
		propValues(fog, vd);
		vd->setValue(gstViewport, Vector4i(vp));
		vd->setValue(gstClearColor, Vector4f(clear_color));
		propValue(gstCurFramebuffer, vd);
		bool adaptive, interactive, persp;
		double scale_factor;
		getValue(gstAdaptive, adaptive);
		getValue(gstInteractive, interactive);
		getValue(gstPerspective, persp);
		getValue(gstScaleFactor, scale_factor);
		vd->Draw(!persp, adaptive, interactive, scale_factor, Get121ScaleFactor());
	}

	bool shadow;
	vd->getValue(gstShadowEnable, shadow);
	if (shadow)
	{
		VolumeList list;
		list.push_back(vd);
		DrawOLShadows(list);
	}

	//bind fbo for final composition
	flvr::Framebuffer* final_buffer =
		flvr::TextureRenderer::framebuffer_manager_.framebuffer(
			"final");
	if (final_buffer)
		final_buffer->bind();

	if (flvr::TextureRenderer::get_mem_swap())
	{
		//restore temp buffer to final buffer
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
		flvr::Framebuffer* temp_buffer =
			flvr::TextureRenderer::framebuffer_manager_.framebuffer(
				"temporary");
		if (temp_buffer)
		{
			//temp buffer becomes unused after texture is bound
			//ok to unprotect
			temp_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
			temp_buffer->unprotect();
		}
		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);

		img_shader =
			flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHADER_TEXTURE_LOOKUP);
		if (img_shader)
		{
			if (!img_shader->valid())
				img_shader->create();
			img_shader->bind();
		}
		DrawViewQuad();
		if (img_shader && img_shader->valid())
			img_shader->release();

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glActiveTexture(GL_TEXTURE0);
	if (chann_buffer)
		chann_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
	//build mipmap
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glEnable(GL_BLEND);
	long lval;
	getValue(gstMixMethod, lval);
	if (lval == MIX_METHOD_COMP)
		glBlendFunc(GL_ONE, GL_ONE);
	else
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	//2d adjustment
	img_shader =
		flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHDR_BRIGHTNESS_CONTRAST_HDR);
	if (img_shader)
	{
		if (!img_shader->valid())
			img_shader->create();
		img_shader->bind();
	}
	double r, g, b;
	vd->getValue(gstGammaR, r);
	vd->getValue(gstGammaG, g);
	vd->getValue(gstGammaB, b);
	Color gamma(r, g, b);
	vd->getValue(gstBrightnessR, r);
	vd->getValue(gstBrightnessG, g);
	vd->getValue(gstBrightnessB, b);
	Color brightness(r, g, b);
	vd->getValue(gstEqualizeR, r);
	vd->getValue(gstEqualizeG, g);
	vd->getValue(gstEqualizeB, b);
	Color hdr(r, g, b);
	img_shader->setLocalParam(0, gamma.r(), gamma.g(), gamma.b(), 1.0);
	img_shader->setLocalParam(1, brightness.r(), brightness.g(), brightness.b(), 1.0);
	img_shader->setLocalParam(2, hdr.r(), hdr.g(), hdr.b(), 0.0);
	//2d adjustment

	DrawViewQuad();

	if (img_shader && img_shader->valid())
		img_shader->release();

	//if vd is duplicated
	if (flvr::TextureRenderer::get_mem_swap() &&
		flvr::TextureRenderer::get_done_current_chan())
	{
		vector<flvr::TextureBrick*> *bricks =
			vd->GetTexture()->get_bricks();
		for (int i = 0; i < bricks->size(); i++)
			(*bricks)[i]->set_drawn(false);
	}
}

void Renderview::DrawOLShading(VolumeData* vd)
{
	long nx, ny;
	GetRenderSize(nx, ny);

	if (flvr::TextureRenderer::get_mem_swap() &&
		flvr::TextureRenderer::get_start_update_loop() &&
		!flvr::TextureRenderer::get_done_update_loop())
	{
		unsigned int rn_time = glbin_timer->get_ticks();
		if (rn_time - flvr::TextureRenderer::get_st_time() >
			flvr::TextureRenderer::get_up_time())
			return;
		if (vd->GetRenderer()->get_done_loop(2))
			return;
	}

	//shading pass
	flvr::Framebuffer* overlay_buffer =
		flvr::TextureRenderer::framebuffer_manager_.framebuffer(
			flvr::FB_Render_RGBA, nx, ny);
	if (overlay_buffer)
	{
		overlay_buffer->bind();
		overlay_buffer->protect();
	}
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	vd->setValue(gstShadingEnable, true);
	bool alpha;
	vd->getValue(gstAlphaEnable, alpha);
	vd->setValue(gstAlphaEnable, true);
	long mip_mode;
	vd->getValue(gstMipMode, mip_mode);
	vd->setValue(gstMipMode, long(2));
	long colormode;
	vd->getValue(gstColormapMode, colormode);
	vd->setValue(gstStreamMode, long(2));
	vd->SetMatrices(m_mv_mat, m_proj_mat, m_tex_mat);
	ValueCollection fog = { gstDepthAtten, gstDaInt, gstDaStart, gstDaEnd };
	propValues(fog, vd);
	bool adaptive, interactive, persp;
	double scale_factor;
	getValue(gstAdaptive, adaptive);
	getValue(gstInteractive, interactive);
	getValue(gstPerspective, persp);
	getValue(gstScaleFactor, scale_factor);
	vd->Draw(!persp, adaptive, interactive, scale_factor, Get121ScaleFactor());
	vd->setValue(gstMipMode, mip_mode);
	vd->setValue(gstColormapMode, colormode);
	vd->setValue(gstAlphaEnable, alpha);

	//bind fbo for final composition
	flvr::Framebuffer* chann_buffer =
		flvr::TextureRenderer::framebuffer_manager_.framebuffer("channel");
	if (chann_buffer)
		chann_buffer->bind();
	glActiveTexture(GL_TEXTURE0);
	if (overlay_buffer)
	{
		//ok to unprotect
		overlay_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
		overlay_buffer->unprotect();
	}
	glEnable(GL_BLEND);
	glBlendFunc(GL_ZERO, GL_SRC_COLOR);
	//glBlendEquation(GL_MIN);
	glDisable(GL_DEPTH_TEST);

	flvr::ShaderProgram* img_shader =
		flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHADER_TEXTURE_LOOKUP);
	if (img_shader)
	{
		if (!img_shader->valid())
			img_shader->create();
		img_shader->bind();
	}
	DrawViewQuad();
	if (img_shader && img_shader->valid())
		img_shader->release();

	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}

void Renderview::DrawOLShadows(VolumeList &vlist)
{
	if (vlist.empty())
		return;

	long nx, ny;
	GetRenderSize(nx, ny);
	GLint vp[4] = { 0, 0, (GLint)nx, (GLint)ny };
	GLfloat clear_color[4] = { 1, 1, 1, 1 };

	size_t i;
	bool has_shadow = false;
	vector<long> colormodes;
	vector<bool> shadings;
	vector<long> mip_modes;
	VolumeList list;
	//generate list
	for (i = 0; i < vlist.size(); i++)
	{
		VolumeData* vd = vlist[i];
		if (!vd)
			continue;
		bool shadow;
		vd->getValue(gstShadowEnable, shadow);
		if (shadow)
		{
			long colormode;
			vd->getValue(gstColormapMode, colormode);
			colormodes.push_back(colormode);
			bool shading;
			vd->getValue(gstShadingEnable, shading);
			shadings.push_back(shading);
			long mip_mode;
			vd->getValue(gstMipMode, mip_mode);
			mip_modes.push_back(mip_mode);
			list.push_back(vd);
			has_shadow = true;
		}
	}

	if (!has_shadow)
		return;

	if (flvr::TextureRenderer::get_mem_swap() &&
		flvr::TextureRenderer::get_start_update_loop() &&
		!flvr::TextureRenderer::get_done_update_loop())
	{
		unsigned int rn_time = glbin_timer->get_ticks();
		if (rn_time - flvr::TextureRenderer::get_st_time() >
			flvr::TextureRenderer::get_up_time())
			return;
		if (list.size() == 1)
		{
			bool shadow;
			list[0]->getValue(gstShadowEnable, shadow);
			if (shadow && list[0]->GetRenderer()->get_done_loop(3))
				return;
		}
	}

	flvr::Framebuffer* overlay_buffer =
		flvr::TextureRenderer::framebuffer_manager_.framebuffer(
			flvr::FB_Render_RGBA, nx, ny);
	if (overlay_buffer)
	{
		overlay_buffer->bind();
		overlay_buffer->protect();
		setValue(gstCurFramebuffer, (unsigned long)(overlay_buffer->id()));
	}

	if (!flvr::TextureRenderer::get_mem_swap() ||
		(flvr::TextureRenderer::get_mem_swap() &&
			flvr::TextureRenderer::get_clear_chan_buffer()))
	{
		glClearColor(1.0, 1.0, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		flvr::TextureRenderer::reset_clear_chan_buffer();
	}
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	double shadow_darkness = 0.0;

	if (list.empty())
		;
	else if (list.size() == 1)
	{
		VolumeData* vd = list[0];
		//save
		long colormode;
		vd->getValue(gstColormapMode, colormode);
		bool shading;
		vd->getValue(gstShadingEnable, shading);
		long mip_mode;
		vd->getValue(gstMipMode, mip_mode);
		//set to draw depth
		vd->setValue(gstShadingEnable, false);
		vd->setValue(gstMipMode, long(0));
		vd->setValue(gstColormapMode, long(2));
		if (overlay_buffer)
			vd->setValue(gst2dDmapId,
			(unsigned long)(overlay_buffer->tex_id(GL_COLOR_ATTACHMENT0)));
		long msk_mode;
		vd->getValue(gstMaskMode, msk_mode);
		vd->setValue(gstMaskMode, long(0));
		//draw
		vd->setValue(gstStreamMode, long(3));
		vd->SetMatrices(m_mv_mat, m_proj_mat, m_tex_mat);
		ValueCollection fog = { gstDepthAtten, gstDaInt, gstDaStart, gstDaEnd };
		propValues(fog, vd);
		vd->setValue(gstViewport, Vector4i(vp));
		vd->setValue(gstClearColor, Vector4f(clear_color));
		propValue(gstCurFramebuffer, vd);
		bool adaptive, interactive, persp;
		double scale_factor;
		getValue(gstAdaptive, adaptive);
		getValue(gstInteractive, interactive);
		getValue(gstPerspective, persp);
		getValue(gstScaleFactor, scale_factor);
		vd->Draw(!persp, adaptive, interactive, scale_factor, Get121ScaleFactor());
		//restore
		vd->setValue(gstMipMode, mip_mode);
		vd->setValue(gstMaskMode, msk_mode);
		vd->setValue(gstColormapMode, colormode);
		vd->setValue(gstShadingEnable, shading);
		vd->setValue(gstShadowInt, shadow_darkness);
	}
	else
	{
		m_mvr->clear_vr();
		for (i = 0; i < list.size(); i++)
		{
			VolumeData* vd = list[i];
			vd->setValue(gstShadingEnable, false);
			vd->setValue(gstMipMode, long(0));
			vd->setValue(gstColormapMode, long(2));
			if (overlay_buffer)
				vd->setValue(gst2dDmapId,
				(unsigned long)(overlay_buffer->tex_id(GL_COLOR_ATTACHMENT0)));
			flvr::VolumeRenderer* vr = list[i]->GetRenderer();
			if (vr)
			{
				list[i]->SetMatrices(m_mv_mat, m_proj_mat, m_tex_mat);
				ValueCollection fog = { gstDepthAtten, gstDaInt, gstDaStart, gstDaEnd };
				propValues(fog, list[i]);
				m_mvr->add_vr(vr);
				m_mvr->set_sampling_rate(vr->get_sampling_rate());
				m_mvr->SetNoiseRed(vr->GetNoiseRed());
			}
		}
		//draw
		m_mvr->set_viewport(vp);
		m_mvr->set_clear_color(clear_color);
		unsigned long ulval;
		getValue(gstCurFramebuffer, ulval);
		m_mvr->set_cur_framebuffer(ulval);
		bool test_wiref, adaptive, interactive, persp, interpolate;
		getValue(gstTestWiref, test_wiref);
		getValue(gstAdaptive, adaptive);
		getValue(gstInteractive, interactive);
		getValue(gstPerspective, persp);
		getValue(gstInterpolate, interpolate);
		m_mvr->draw(test_wiref, adaptive, interactive, !persp, interpolate);

		for (i = 0; i < list.size(); i++)
		{
			VolumeData* vd = list[i];
			vd->setValue(gstMipMode, long(mip_modes[i]));
			vd->setValue(gstColormapMode, long(colormodes[i]));
			vd->setValue(gstShadingEnable, bool(shadings[i]));
		}
		list[0]->getValue(gstShadowInt, shadow_darkness);
	}

	//
	if (!flvr::TextureRenderer::get_mem_swap() ||
		(flvr::TextureRenderer::get_mem_swap() &&
			flvr::TextureRenderer::get_clear_chan_buffer()))
	{
		//shadow pass
		//bind the fbo
		flvr::Framebuffer* temp_buffer =
			flvr::TextureRenderer::framebuffer_manager_.framebuffer(
				flvr::FB_Render_RGBA, nx, ny);
		if (temp_buffer)
		{
			temp_buffer->bind();
			temp_buffer->protect();
		}
		glClearColor(1.0, 1.0, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
		if (overlay_buffer)
		{
			//ok to unprotect
			overlay_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
			overlay_buffer->unprotect();
		}
		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);

		//2d adjustment
		flvr::ShaderProgram* img_shader =
			flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHDR_DEPTH_TO_GRADIENT);
		if (img_shader)
		{
			if (!img_shader->valid())
				img_shader->create();
			img_shader->bind();
		}
		bool bval;
		getValue(gstPerspective, bval);
		img_shader->setLocalParam(0, 1.0 / nx, 1.0 / ny, bval ? 2e10 : 1e6, 0.0);
		double dir_x = 0.0, dir_y = 0.0;
		getValue(gstShadowDirX, dir_x);
		getValue(gstShadowDirY, dir_y);
		img_shader->setLocalParam(1, dir_x, dir_y, 0.0, 0.0);
		//2d adjustment

		DrawViewQuad();

		if (img_shader && img_shader->valid())
			img_shader->release();

		//bind fbo for final composition
		flvr::Framebuffer* chann_buffer =
			flvr::TextureRenderer::framebuffer_manager_.framebuffer("channel");
		if (chann_buffer)
			chann_buffer->bind();
		glActiveTexture(GL_TEXTURE0);
		if (temp_buffer)
		{
			temp_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
			temp_buffer->unprotect();
		}
		glEnable(GL_BLEND);
		glBlendFunc(GL_ZERO, GL_SRC_COLOR);
		glDisable(GL_DEPTH_TEST);

		//2d adjustment
		img_shader =
			flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHDR_GRADIENT_TO_SHADOW);
		if (img_shader)
		{
			if (!img_shader->valid())
				img_shader->create();
			img_shader->bind();
		}
		double dval;
		getValue(gstScaleFactor, dval);
		img_shader->setLocalParam(0, 1.0 / nx, 1.0 / ny, std::max(dval, 1.0), 0.0);
		img_shader->setLocalParam(1, shadow_darkness, 0.0, 0.0, 0.0);
		glActiveTexture(GL_TEXTURE1);
		if (chann_buffer)
			chann_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
		//2d adjustment

		DrawViewQuad();

		if (img_shader && img_shader->valid())
			img_shader->release();
	}

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);

	glBlendEquation(GL_FUNC_ADD);
}

void Renderview::DrawOLShadowsMesh(double darkness)
{
	long nx, ny;
	GetRenderSize(nx, ny);

	//shadow pass
	//bind the fbo
	flvr::Framebuffer* overlay_buffer =
		flvr::TextureRenderer::framebuffer_manager_.framebuffer(
			flvr::FB_Render_RGBA, nx, ny);
	if (overlay_buffer)
	{
		overlay_buffer->bind();
		overlay_buffer->protect();
	}
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	string name = "peel buffer" + std::to_string(0);
	flvr::Framebuffer* peel_buffer =
		flvr::TextureRenderer::framebuffer_manager_.framebuffer(name);
	if (peel_buffer)
		peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	//2d adjustment
	flvr::ShaderProgram* img_shader =
		flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHDR_DEPTH_TO_GRADIENT);
	if (img_shader)
	{
		if (!img_shader->valid())
		{
			img_shader->create();
		}
		img_shader->bind();
	}
	bool bval;
	getValue(gstPerspective, bval);
	img_shader->setLocalParam(0, 1.0 / nx, 1.0 / ny, bval ? 2e10 : 1e6, 0.0);
	double dir_x = 0.0, dir_y = 0.0;
	getValue(gstShadowDirX, dir_x);
	getValue(gstShadowDirY, dir_y);
	img_shader->setLocalParam(1, dir_x, dir_y, 0.0, 0.0);
	//2d adjustment

	DrawViewQuad();

	if (img_shader && img_shader->valid())
		img_shader->release();

	//
	//bind fbo for final composition
	BindRenderBuffer();
	glActiveTexture(GL_TEXTURE0);
	if (overlay_buffer)
	{
		overlay_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
		overlay_buffer->unprotect();
	}
	glEnable(GL_BLEND);
	glBlendFunc(GL_ZERO, GL_SRC_COLOR);
	glDisable(GL_DEPTH_TEST);

	//2d adjustment
	img_shader =
		flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHDR_GRADIENT_TO_SHADOW_MESH);
	if (img_shader)
	{
		if (!img_shader->valid())
			img_shader->create();
		img_shader->bind();
	}
	double dval;
	getValue(gstScaleFactor, dval);
	img_shader->setLocalParam(0, 1.0 / nx, 1.0 / ny, std::max(dval, 1.0), 0.0);
	img_shader->setLocalParam(1, darkness, 0.0, 0.0, 0.0);
	glActiveTexture(GL_TEXTURE1);
	if (peel_buffer)
		peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
	glActiveTexture(GL_TEXTURE2);
	flvr::Framebuffer* final_buffer =
		flvr::TextureRenderer::framebuffer_manager_.framebuffer("final");
	if (final_buffer)
		final_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
	//2d adjustment

	DrawViewQuad();

	if (img_shader && img_shader->valid())
	{
		img_shader->release();
	}
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
}

void Renderview::DrawViewQuad()
{
	flvr::VertexArray* quad_va =
		flvr::TextureRenderer::vertex_array_manager_.vertex_array(flvr::VA_Norm_Square);
	if (quad_va)
		quad_va->draw();
}

bool Renderview::GetMeshShadow(double &val)
{
	for (int i = 0; i < getNumChildren(); i++)
	{
		if (MeshData* md = dynamic_cast<MeshData*>(getChild(i)))
		{
			bool disp;
			md->getValue(gstDisplay, disp);
			if (disp)
			{
				md->getValue(gstShadowInt, val);
				bool shadow;
				md->getValue(gstShadowEnable, shadow);
				return shadow;
			}
		}
		else if (MeshGroup* group = dynamic_cast<MeshGroup*>(getChild(i)))
		{
			bool disp;
			group->getValue(gstDisplay, disp);
			if (disp)
			{
				for (int j = 0; j < (int)group->getNumChildren(); j++)
				{
					MeshData* md = group->getChild(j)->asMeshData();
					if (!md)
						continue;
					md->getValue(gstDisplay, disp);
					if (disp)
					{
						md->getValue(gstShadowInt, val);
						bool shadow;
						md->getValue(gstShadowEnable, shadow);
						return shadow;
					}
				}
			}
		}
	}
	val = 0.0;
	return false;
}

void Renderview::DrawCircles(double cx, double cy,
	double r1, double r2, Color &color, glm::mat4 &matrix)
{
	flvr::ShaderProgram* shader =
		flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHDR_DRAW_GEOMETRY);
	if (shader)
	{
		if (!shader->valid())
			shader->create();
		shader->bind();
	}

	shader->setLocalParam(0, color.r(), color.g(), color.b(), 1.0);
	//apply translate first
	glm::mat4 mat0 = matrix * glm::translate(
		glm::mat4(), glm::vec3(cx, cy, 0.0));
	shader->setLocalParamMatrix(0, glm::value_ptr(mat0));

	flvr::VertexArray* va_circles =
		flvr::TextureRenderer::vertex_array_manager_.vertex_array(flvr::VA_Brush_Circles);
	if (va_circles)
	{
		//set parameters
		std::vector<std::pair<unsigned int, double>> params;
		params.push_back(std::pair<unsigned int, double>(0, r1));
		params.push_back(std::pair<unsigned int, double>(1, r2));
		params.push_back(std::pair<unsigned int, double>(2, 60.0));
		va_circles->set_param(params);
		va_circles->draw();
	}

	if (shader && shader->valid())
		shader->release();
}

//draw the brush shape
void Renderview::DrawBrush()
{
	double pressure = m_selector->GetNormPress();

	bool bval;
	getValue(gstMouseIn, bval);
	if (bval)
	{
		long nx, ny;
		GetRenderSize(nx, ny);
		float sx, sy;
		sx = 2.0 / nx;
		sy = 2.0 / ny;
		long lx, ly;
		getValue(gstMouseX, lx);
		getValue(gstMouseY, ly);

		//draw the circles
		//set up the matrices
		glm::mat4 proj_mat;
		double cx, cy;
		if (m_selector->GetBrushSizeData())
		{
			double dl, dr, dt, db;
			getValue(gstOrthoLeft, dl);
			getValue(gstOrthoRight, dr);
			getValue(gstOrthoBottom, db);
			getValue(gstOrthoTop, dt);
			proj_mat = glm::ortho(float(dl), float(dr), float(dt), float(db));
			cx = dl + lx * (dr - dl) / nx;
			cy = db + ly * (dt - db) / ny;
		}
		else
		{
			proj_mat = glm::ortho(float(0), float(nx), float(0), float(ny));
			cx = lx;
			cy = ny - ly;
		}

		//attributes
		glDisable(GL_DEPTH_TEST);

		int mode = m_selector->GetMode();

		Color text_color;
		getValue(gstTextColor, text_color);

		double br1 = m_selector->GetBrushSize1();
		double br2 = m_selector->GetBrushSize2();

		if (mode == 1 ||
			mode == 2)
			DrawCircles(cx, cy, br1*pressure,
				br2*pressure, text_color, proj_mat);
		else if (mode == 8)
			DrawCircles(cx, cy, br1*pressure,
				-1.0, text_color, proj_mat);
		else if (mode == 3 ||
			mode == 4)
			DrawCircles(cx, cy, -1.0,
				br2*pressure, text_color, proj_mat);

		float cx2 = lx;
		float cy2 = ny - ly;
		float px, py;
		px = cx2 - 7 - nx / 2.0;
		py = cy2 - 3 - ny / 2.0;
		wstring wstr;
		switch (mode)
		{
		case 1:
			wstr = L"S";
			break;
		case 2:
			wstr = L"+";
			break;
		case 3:
			wstr = L"-";
			break;
		case 4:
			wstr = L"*";
			break;
		}
		m_text_renderer->RenderText(wstr, text_color, px*sx, py*sy, sx, sy);

		glEnable(GL_DEPTH_TEST);

	}
}

//paint strokes on the paint fbo
void Renderview::PaintStroke()
{
	long nx, ny;
	GetRenderSize(nx, ny);

	double pressure = m_selector->GetNormPress();

	//generate texture and buffer objects
	//painting fbo
	flvr::Framebuffer* paint_buffer =
		flvr::TextureRenderer::framebuffer_manager_.framebuffer(
			flvr::FB_Render_RGBA, nx, ny, "paint brush");
	if (!paint_buffer)
		return;
	paint_buffer->bind();
	paint_buffer->protect();
	//clear if asked so
	bool bval;
	getValue(gstClearPaint, bval);
	if (bval)
	{
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT);
		setValue(gstClearPaint, false);
	}
	else
	{
		//paint shader
		flvr::ShaderProgram* paint_shader =
			flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHDR_PAINT);
		if (paint_shader)
		{
			if (!paint_shader->valid())
				paint_shader->create();
			paint_shader->bind();
		}

		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendEquation(GL_MAX);

		double radius1 = m_selector->GetBrushSize1();
		double radius2 = m_selector->GetBrushSize2();
		double bspc = m_selector->GetBrushSpacing();
		bool bs_data = m_selector->GetBrushSizeData();
		long lx, ly, lxp, lyp;
		getValue(gstMouseX, lx); getValue(gstMouseY, ly);
		getValue(gstMousePrvX, lxp); getValue(gstMousePrvY, lyp);
		double px = double(lx - lxp);
		double py = double(ly - lyp);
		double dist = sqrt(px*px + py * py);
		double step = radius1 * pressure * bspc;
		int repeat = int(dist / step + 0.5);
		double spx = (double)lxp;
		double spy = (double)lyp;
		if (repeat > 0)
		{
			px /= repeat;
			py /= repeat;
		}

		double dl, dr, dt, db;
		getValue(gstOrthoLeft, dl);
		getValue(gstOrthoRight, dr);
		getValue(gstOrthoBottom, db);
		getValue(gstOrthoTop, dt);
		//set the width and height
		if (bs_data)
		{
			paint_shader->setLocalParam(1, dr - dl, dt - db, 0.0f, 0.0f);
		}
		else
			paint_shader->setLocalParam(1, nx, ny, 0.0f, 0.0f);

		double x, y;
		double cx, cy;
		for (int i = 0; i <= repeat; i++)
		{
			x = spx + i * px;
			y = spy + i * py;
			if (bs_data)
			{
				cx = x * (dr - dl) / nx;
				cy = (ny - y) * (dt - db) / ny;
			}
			else
			{
				cx = x;
				cy = double(ny) - y;
			}
			switch (m_selector->GetMode())
			{
			case 3:
				radius1 = radius2;
				break;
			case 4:
				radius1 = 0.0;
				break;
			case 8:
				radius2 = radius1;
				break;
			default:
				break;
			}
			//send uniforms to paint shader
			paint_shader->setLocalParam(0, cx, cy,
				radius1*pressure,
				radius2*pressure);
			//draw a square
			DrawViewQuad();
		}

		//release paint shader
		if (paint_shader && paint_shader->valid())
			paint_shader->release();
	}

	//bind back the window frame buffer
	BindRenderBuffer();
	glBlendEquation(GL_FUNC_ADD);
	//RefreshGL(3);
}

//show the stroke buffer
void Renderview::DisplayStroke()
{
	//painting texture
	flvr::Framebuffer* paint_buffer =
		flvr::TextureRenderer::framebuffer_manager_.framebuffer("paint brush");
	if (!paint_buffer)
		return;

	//draw the final buffer to the windows buffer
	glActiveTexture(GL_TEXTURE0);
	paint_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	flvr::ShaderProgram* img_shader =
		flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHADER_TEXTURE_LOOKUP);
	if (img_shader)
	{
		if (!img_shader->valid())
			img_shader->create();
		img_shader->bind();
	}
	DrawViewQuad();
	if (img_shader && img_shader->valid())
		img_shader->release();

	glBindTexture(GL_TEXTURE_2D, 0);
	glEnable(GL_DEPTH_TEST);
}

Quaternion Renderview::Trackball(double dx, double dy)
{
	Quaternion q;
	Vector a; /* Axis of rotation */
	double phi;  /* how much to rotate about axis */

	if (dx == 0.0 && dy == 0.0)
	{
		/* Zero rotation */
		return q;
	}

	a = Vector(-dy, dx, 0.0);
	phi = a.length() / 3.0;
	a.normalize();
	
	bool bval;
	getValue(gstGearedEnable, bval);
	if (bval && phi < 45.0)
	{
		/* Zero rotation */
		return q;
	}

	//rotate back to local
	Quaternion aq(a);
	Quaternion vq;
	getValue(gstCamRotQ, vq);
	aq = (-vq) * aq * vq;
	if (bval)
	{
		//rotate back to basis
		Quaternion zq;
		getValue(gstCamRotZeroQ, zq);
		aq = (zq)* aq * (-zq);
		a = Vector(aq.x, aq.y, aq.z);
		a.normalize();
		//snap to closest basis component
		double maxv = std::max(std::fabs(a.x()),
			std::max(std::fabs(a.y()), std::fabs(a.z())));
		if (std::fabs(maxv - std::fabs(a.x())) < Epsilon())
			a = Vector(a.x() < 0 ? -1 : 1, 0, 0);
		else if (std::fabs(maxv - std::fabs(a.y())) < Epsilon())
			a = Vector(0, a.y() < 0 ? -1 : 1, 0);
		else if (std::fabs(maxv - std::fabs(a.z())) < Epsilon())
			a = Vector(0, 0, a.z() < 0 ? -1 : 1);
		aq = Quaternion(a);
		//rotate again to restore
		aq = (-zq) * aq * zq;
		a = Vector(aq.x, aq.y, aq.z);
		a.normalize();
		//snap to 45 deg
		phi = int(phi / 45.0) * 45.0;
	}
	else
	{
		a = Vector(aq.x, aq.y, aq.z);
		a.normalize();
	}

	q = Quaternion(phi, a);
	q.Normalize();

	return q;
}

Quaternion Renderview::TrackballClip(long p1x, long p1y, long p2x, long p2y)
{
	Quaternion q;
	Vector a; /* Axis of rotation */
	double phi;  /* how much to rotate about axis */

	if (p1x == p2x && p1y == p2y)
	{
		/* Zero rotation */
		return q;
	}

	a = Vector(p2y - p1y, p2x - p1x, 0.0);
	phi = a.length() / 3.0;
	a.normalize();
	Quaternion q_a(a);
	//rotate back to local
	Quaternion q2;
	double dx, dy, dz;
	getValue(gstCamRotX, dx);
	getValue(gstCamRotY, dy);
	getValue(gstCamRotZ, dz);
	q2.FromEuler(-dx, dy, dz);
	q_a = (q2)* q_a * (-q2);
	Quaternion q_cl;
	getValue(gstClipRotQ, q_cl);
	q_a = (q_cl)* q_a * (-q_cl);
	a = Vector(q_a.x, q_a.y, q_a.z);
	a.normalize();

	q = Quaternion(phi, a);
	q.Normalize();

	return q;
}

void Renderview::Q2A()
{
	//view changed, re-sort bricks
	//SetSortBricks();
	Quaternion vq, zq;
	getValue(gstCamRotQ, vq);
	getValue(gstCamRotZeroQ, zq);
	Quaternion q = vq * (-zq);
	double dx, dy, dz;
	q.ToEuler(dx, dy, dz);

	setValue(gstCamRotX, dx);
	setValue(gstCamRotX, dx);
	setValue(gstCamRotX, dx);

	long lval;
	getValue(gstClipMode, lval);
	if (lval)
		UpdateClips();
}

void Renderview::A2Q()
{
	//view changed, re-sort bricks
	//SetSortBricks();
	double dx, dy, dz;
	getValue(gstCamRotX, dx);
	getValue(gstCamRotY, dy);
	getValue(gstCamRotZ, dz);
	Quaternion q, zq;
	getValue(gstCamRotZeroQ, zq);
	q.FromEuler(dx, dy, dz);
	q = q * zq;
	setValue(gstCamRotQ, q);

	long lval;
	getValue(gstClipMode, lval);
	if (lval)
		UpdateClips();
}

//sort bricks after view changes
void Renderview::SetSortBricks()
{
	PopVolumeList();

	for (auto vd : m_vol_list)
	{
		if (vd && vd->GetTexture())
			vd->GetTexture()->set_sort_bricks();
	}
}

//pre-draw processings
void Renderview::PreDraw()
{
	//skip if not done with loop
	if (flvr::TextureRenderer::get_mem_swap())
	{
		bool bval;
		getValue(gstPreDraw, bval);
		if (bval)
			setValue(gstPreDraw, false);
		else
			return;
	}
}

void Renderview::PostDraw()
{
	//skip if not done with loop
	if (flvr::TextureRenderer::get_mem_swap() &&
		flvr::TextureRenderer::get_start_update_loop() &&
		!flvr::TextureRenderer::get_done_update_loop())
		return;

	//output animations
	bool bval;
	std::wstring cap_file;
	getValue(gstCapture, bval);
	getValue(gstCaptureFile, cap_file);
	if (!bval || cap_file.empty()) return;

	//capture
	int chann;
	bool fp32, compress;
	getValue(gstCaptureAlpha, bval);
	chann = bval ? 4 : 3;
	getValue(gstCaptureFloat, fp32);
	getValue(gstCaptureCompress, compress);
	long x, y, w, h;
	void* image = 0;
	ReadPixels(chann, fp32, x, y, w, h, &image);

	TIFF *out = TIFFOpenW(cap_file, "wb");
	if (!out)
		return;
	TIFFSetField(out, TIFFTAG_IMAGEWIDTH, w);
	TIFFSetField(out, TIFFTAG_IMAGELENGTH, h);
	TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, chann);
	if (fp32)
	{
		TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 32);
		TIFFSetField(out, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
	}
	else
	{
		TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 8);
		//TIFFSetField(out, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
	}
	TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
	TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
	if (compress)
		TIFFSetField(out, TIFFTAG_COMPRESSION, COMPRESSION_LZW);

	tsize_t linebytes = chann * w * (fp32 ? 4 : 1);
	void *buf = NULL;
	buf = _TIFFmalloc(linebytes);
	TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(out, 0));
	for (uint32 row = 0; row < (uint32)h; row++)
	{
		if (fp32)
		{
			float* line = ((float*)image) + (h - row - 1) * chann * w;
			memcpy(buf, line, linebytes);
		}
		else
		{// check the index here, and figure out why not using h*linebytes
			unsigned char* line = ((unsigned char*)image) + (h - row - 1) * chann * w;
			memcpy(buf, line, linebytes);
		}
		if (TIFFWriteScanline(out, buf, row, 0) < 0)
			break;
	}
	TIFFClose(out);
	if (buf)
		_TIFFfree(buf);
	if (image)
		delete[]image;

	setValue(gstCapture, false);
}

void Renderview::ResetEnlarge()
{
	//skip if not done with loop
	if (flvr::TextureRenderer::get_mem_swap() &&
		flvr::TextureRenderer::get_start_update_loop() &&
		!flvr::TextureRenderer::get_done_update_loop())
		return;
	bool bval;
	getValue(gstKeepEnlarge, bval);
	if (bval)
		return;
	setValue(gstEnlarge, false);
	double text_size;
	getValue(gstTextSize, text_size);
	flvr::TextRenderer::text_texture_manager_.SetSize(text_size);
	//RefreshGL(19);
}

void Renderview::SetBrush(long mode)
{
	//m_prev_focus = FindFocus();
	//SetFocus();

	int ruler_type = m_ruler_handler->GetType();
	long int_mode;
	getValue(gstInterMode, int_mode);

	if (int_mode == 5 ||
		int_mode == 7)
	{
		setValue(gstInterMode, 7);
		if (ruler_type == 3)
			m_selector->SetMode(8);
		else
			m_selector->SetMode(1);
	}
	else if (int_mode == 8)
	{
		if (ruler_type == 3)
			m_selector->SetMode(8);
		else
			m_selector->SetMode(1);
	}
	else if (int_mode == 10)
	{
		m_selector->SetMode(9);
	}
	else
	{
		setValue(gstInterMode, long(2));
		m_selector->SetMode(mode);
	}
	setValue(gstPaintDisplay, true);
	setValue(gstDrawBrush, true);
	m_selector->ChangeBrushSetsIndex();
}

//selection
void Renderview::Pick()
{
	bool bval;
	getValue(gstDrawAll, bval);
	if (bval)
	{
		PickVolume();
		PickMesh();
	}
}

void Renderview::PickMesh()
{
	long nx, ny;
	GetRenderSize(nx, ny);
	if (nx <= 0 || ny <= 0)
		return;
	long lx, ly;
	getValue(gstMouseClientX, lx);
	getValue(gstMouseClientY, ly);
	if (lx<0 || lx >= nx ||
		ly <= 0 || ly>ny)
		return;

	//projection
	HandleProjection(nx, ny);
	//Transformation
	HandleCamera();
	//obj
	glm::mat4 mv_temp = m_mv_mat;
	m_mv_mat = GetDrawMat();

	//set up fbo
	setValue(gstCurFramebuffer, (unsigned long)(0));
	//bind
	flvr::Framebuffer* pick_buffer =
		flvr::TextureRenderer::framebuffer_manager_.framebuffer(
			flvr::FB_Pick_Int32_Float, nx, ny);
	if (pick_buffer)
		pick_buffer->bind();

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glScissor(lx, ny - ly, 1, 1);
	glEnable(GL_SCISSOR_TEST);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	unsigned int i = 0;
	for (auto md : m_msh_list)
	{
		if (!md) continue;
		md->SetMatrices(m_mv_mat, m_proj_mat);
		md->DrawInt(i + 1);
		i++;
	}
	glDisable(GL_SCISSOR_TEST);

	unsigned int choose = 0;
	if (pick_buffer)
		choose = pick_buffer->read_value(lx, ny - ly);
	unsigned long ulval;
	getValue(gstCurFramebuffer, ulval);
	glBindFramebuffer(GL_FRAMEBUFFER, ulval);

	std::string str;
	if (choose > 0 && choose <= m_msh_list.size())
	{
		MeshData* md = m_msh_list[choose - 1].get();
		if (md)
			str = md->getName();
	}
	setValue(gstSelectedMshName, str);
	m_mv_mat = mv_temp;
}

void Renderview::PickVolume()
{
	//int kmode = 0;// wxGetKeyState(WXK_CONTROL) ? 1 : 0;
	double dist = 0.0;
	double min_dist = -1.0;
	Point p, ip, pp;
	VolumeData* picked_vd = 0;
	long lx, ly;
	getValue(gstMouseX, lx);
	getValue(gstMouseY, ly);
	bool persp;
	getValue(gstPerspective, persp);
	for (auto vd : m_vol_list)
	{
		if (!vd) continue;
		long mode = 2;
		long mip_mode;
		vd->getValue(gstMipMode, mip_mode);
		if (mip_mode == 1) mode = 1;
		m_volume_point->SetVolumeData(vd.get());

		dist = m_volume_point->GetPointVolume(lx, ly,
			mode, true, 0.5, p, ip);
		if (dist > 0.0)
		{
			if (min_dist < 0.0)
			{
				min_dist = dist;
				picked_vd = vd.get();
				pp = p;
			}
			else
			{
				if (persp)
				{
					if (dist < min_dist)
					{
						min_dist = dist;
						picked_vd = vd.get();
						pp = p;
					}
				}
				else
				{
					if (dist > min_dist)
					{
						min_dist = dist;
						picked_vd = vd.get();
						pp = p;
					}
				}
			}
		}
	}

	std::string str;
	if (picked_vd)
	{
		str = picked_vd->getName();
		setValue(gstSelPointVolume, ip);
		bool bval;
		getValue(gstCamLockPick, bval);
		if (bval)
			setValue(gstCamLockCtr, pp);
	}
	setValue(gstSelectedVolName, str);
}

bool Renderview::PinRotCtr()
{
	//pin rotation center
	bool pin_rot_ctr, rot_ctr_dirty, free;
	VolumeData* cur_vol = GetCurrentVolume();
	getValue(gstPinRotCtr, pin_rot_ctr);
	getValue(gstRotCtrDirty, rot_ctr_dirty);
	getValue(gstFree, free);
	if (pin_rot_ctr && rot_ctr_dirty &&
		cur_vol && !free)
	{
		Point p, ip;
		long nx, ny;
		getValue(gstSizeX, nx);
		getValue(gstSizeY, ny);
		long mode = 2;
		long mip_mode;
		cur_vol->getValue(gstMipMode, mip_mode);
		if (mip_mode == 1) mode = 1;
		m_volume_point->SetVolumeData(cur_vol);
		double dval;
		getValue(gstPinThresh, dval);
		double dist = m_volume_point->GetPointVolume(nx / 2.0, ny / 2.0,
			mode, true, dval, p, ip);
		if (dist <= 0.0)
			dist = m_volume_point->GetPointVolumeBox(
				nx / 2.0, ny / 2.0,
				true, p);
		if (dist > 0.0)
		{
			setValue(gstRotCtrPin, p);
			double dx, dy, dz;
			getValue(gstObjCtrX, dx);
			getValue(gstObjCtrY, dy);
			getValue(gstObjCtrZ, dz);
			p = Point(dx - p.x(), p.y() - dy, p.z() - dz);
			dx = p.x();
			dy = p.y();
			dz = p.z();
			double thresh = 10.0;
			double spcx, spcy, spcz;
			cur_vol->getValue(gstSpcX, spcx);
			cur_vol->getValue(gstSpcY, spcy);
			cur_vol->getValue(gstSpcZ, spcz);
			thresh *= spcx;
			double dx2, dy2, dz2;
			getValue(gstObjTransX, dx2);
			getValue(gstObjTransY, dy2);
			getValue(gstObjTransZ, dz2);
			if (sqrt((dx2 - dx)*(dx2 - dx) +
				(dy2 - dy)*(dy2 - dy) +
				(dz2 - dz)*(dz2 - dz)) > thresh)
			{
				setValue(gstObjTransX, dx);
				setValue(gstObjTransY, dy);
				setValue(gstObjTransZ, dz);
			}
		}
		setValue(gstRotCtrDirty, false);
		return true;
	}
	return false;
}

void Renderview::SetCenter()
{
	InitView(INIT_BOUNDS | INIT_CENTER | INIT_OBJ_TRANSL);

	VolumeData *vd = GetCurrentVolume();

	if (vd)
	{
		BBox bbox;
		vd->getValue(gstBounds, bbox);
		flvr::VolumeRenderer *vr = vd->GetRenderer();
		if (!vr) return;
		vector<Plane*> *planes = vr->get_planes();
		if (planes->size() != 6) return;
		double x1, x2, y1, y2, z1, z2;
		double abcd[4];
		(*planes)[0]->get_copy(abcd);
		x1 = fabs(abcd[3])*bbox.Max().x();
		(*planes)[1]->get_copy(abcd);
		x2 = fabs(abcd[3])*bbox.Max().x();
		(*planes)[2]->get_copy(abcd);
		y1 = fabs(abcd[3])*bbox.Max().y();
		(*planes)[3]->get_copy(abcd);
		y2 = fabs(abcd[3])*bbox.Max().y();
		(*planes)[4]->get_copy(abcd);
		z1 = fabs(abcd[3])*bbox.Max().z();
		(*planes)[5]->get_copy(abcd);
		z2 = fabs(abcd[3])*bbox.Max().z();

		double dx, dy, dz;
		dx = (x1 + x2) / 2.0;
		dy = (y1 + y2) / 2.0;
		dz = (z1 + z2) / 2.0;
		setValue(gstObjCtrX, dx);
		setValue(gstObjCtrY, dy);
		setValue(gstObjCtrZ, dz);

		//SetSortBricks();

		//RefreshGL(20);
	}
}

void Renderview::SetLockCenter(int type)
{
	switch (type)
	{
	case 1:
	default:
		SetLockCenterVol();
		break;
	case 2:
		setValue(gstCamLockPick, true);
		break;
	case 3:
		SetLockCenterRuler();
		break;
	case 4:
		SetLockCenterSel();
		break;
	}
}

void Renderview::SetLockCenterVol()
{
	VolumeData* vd = GetCurrentVolume();
	if (!vd)
		return;
	BBox box;
	vd->getValue(gstClipBounds, box);
	setValue(gstCamLockCtr, box.center());
}

void Renderview::SetLockCenterRuler()
{
	if (!m_cur_ruler)
		return;
	setValue(gstCamLockCtr, m_cur_ruler->GetCenter());
}

void Renderview::SetLockCenterSel()
{
	VolumeData* vd = GetCurrentVolume();
	if (!vd)
		return;
	flrd::Cov cover(vd);
	cover.Compute(1);
	setValue(gstCamLockCtr, cover.GetCenter());
}

void Renderview::switchLevel(VolumeData *vd)
{
	if (!vd) return;

	long nx, ny;
	GetRenderSize(nx, ny);
	bool bval;
	getValue(gstEnlarge, bval);
	if (bval)
	{
		double dval;
		getValue(gstEnlargeScale, dval);
		nx = long(nx * dval);
		ny = long(ny * dval);
	}

	flvr::Texture *vtex = vd->GetTexture();
	if (vtex && vtex->isBrxml())
	{
		long prev_lv;
		vd->getValue(gstLevelNum, prev_lv);
		int new_lv = 0;

		long lval;
		getValue(gstResMode, lval);
		if (lval > 0)
		{
			double res_scale = 1.0;
			switch (lval)
			{
			case 1:
				res_scale = 1;
				break;
			case 2:
				res_scale = 1.5;
				break;
			case 3:
				res_scale = 2.0;
				break;
			case 4:
				res_scale = 3.0;
				break;
			default:
				res_scale = 1.0;
			}
			std::vector<double> sfs;
			std::vector<double> spx, spy, spz;
			int lvnum = vtex->GetLevelNum();
			double dval;
			getValue(gstRadius, dval);
			for (int i = 0; i < lvnum; i++)
			{
				double aspect = (double)nx / (double)ny;

				double spc_x;
				double spc_y;
				double spc_z;
				vtex->get_spacings(spc_x, spc_y, spc_z, i);
				spc_x = spc_x < Epsilon() ? 1.0 : spc_x;
				spc_y = spc_y < Epsilon() ? 1.0 : spc_y;

				spx.push_back(spc_x);
				spy.push_back(spc_y);
				spz.push_back(spc_z);

				double sf = 2.0*dval*res_scale / spc_y / double(ny);
				sfs.push_back(sf);
			}

			int lv = lvnum - 1;
			getValue(gstScaleFactor, dval);
			//if (!m_manip)
			{
				for (int i = lvnum - 1; i >= 0; i--)
				{
					if (dval / 5 > (/*m_interactive ? sfs[i] * 16.0 :*/ sfs[i])) lv = i - 1;
				}
			}
			//apply offset
			getValue(gstLevelOffset, lval);
			lv += lval;
			if (lv < 0) lv = 0;
			//if (m_interactive) lv += 1;
			if (lv >= lvnum) lv = lvnum - 1;
			new_lv = lv;
		}
		if (prev_lv != new_lv)
		{
			vector<flvr::TextureBrick*> *bricks = vtex->get_bricks();
			if (bricks)
			{
				for (int i = 0; i < bricks->size(); i++)
					(*bricks)[i]->set_disp(false);
			}
			vd->setValue(gstLevel, long(new_lv));
			vtex->set_sort_bricks();
		}
	}
}

#ifdef _WIN32
//tablet init
HCTX Renderview::TabletInit(HWND hWnd, HINSTANCE hInst)
{
	HCTX hctx = NULL;
	UINT wDevice = 0;
	UINT wExtX = 0;
	UINT wExtY = 0;
	UINT wWTInfoRetVal = 0;
	AXIS TabletX = { 0 };
	AXIS TabletY = { 0 };
	AXIS TabletNPress = { 0 };
	AXIS TabletTPress = { 0 };

	// Set option to move system cursor before getting default system context.
	m_lc.lcOptions |= CXO_SYSTEM;

	// Open default system context so that we can get tablet data
	// in screen coordinates (not tablet coordinates).
	wWTInfoRetVal = gpWTInfoA(WTI_DEFSYSCTX, 0, &m_lc);
	WACOM_ASSERT(wWTInfoRetVal == sizeof(LOGCONTEXTA));

	WACOM_ASSERT(m_lc.lcOptions & CXO_SYSTEM);

	// modify the digitizing region
	sprintf(m_lc.lcName,
		"FluoRender Digitizing %llx",
		reinterpret_cast<unsigned long long>(hInst));

	// We process WT_PACKET (CXO_MESSAGES) messages.
	m_lc.lcOptions |= CXO_MESSAGES;

	// What data items we want to be included in the tablet packets
	m_lc.lcPktData = PACKETDATA;

	// Which packet items should show change in value since the last
	// packet (referred to as 'relative' data) and which items
	// should be 'absolute'.
	m_lc.lcPktMode = PACKETMODE;

	// This bitfield determines whether or not this context will receive
	// a packet when a value for each packet field changes.  This is not
	// supported by the Intuos Wintab.  Your context will always receive
	// packets, even if there has been no change in the data.
	m_lc.lcMoveMask = PACKETDATA;

	// Which buttons events will be handled by this context.  lcBtnMask
	// is a bitfield with one bit per button.
	m_lc.lcBtnUpMask = m_lc.lcBtnDnMask;

	// Set the entire tablet as active
	wWTInfoRetVal = gpWTInfoA(WTI_DEVICES + 0, DVC_X, &TabletX);
	WACOM_ASSERT(wWTInfoRetVal == sizeof(AXIS));

	wWTInfoRetVal = gpWTInfoA(WTI_DEVICES, DVC_Y, &TabletY);
	WACOM_ASSERT(wWTInfoRetVal == sizeof(AXIS));

	wWTInfoRetVal = gpWTInfoA(WTI_DEVICES, DVC_NPRESSURE, &TabletNPress);
	if (wWTInfoRetVal == sizeof(AXIS))
		m_selector->SetBrushPnMax(TabletNPress.axMax);
	else
		m_selector->SetBrushPnMax(1.0);

	wWTInfoRetVal = gpWTInfoA(WTI_DEVICES, DVC_TPRESSURE, &TabletTPress);
	if (wWTInfoRetVal == sizeof(AXIS))
		m_selector->SetBrushPtMax(TabletTPress.axMax);
	else
		m_selector->SetBrushPnMax(1.0);

	//m_lc.lcInOrgX = 0;
	//m_lc.lcInOrgY = 0;
	//m_lc.lcInExtX = TabletX.axMax;
	//m_lc.lcInExtY = TabletY.axMax;

	//// Guarantee the output coordinate space to be in screen coordinates.
	//m_lc.lcOutOrgX = GetSystemMetrics(SM_XVIRTUALSCREEN);
	//m_lc.lcOutOrgY = GetSystemMetrics(SM_YVIRTUALSCREEN);
	//m_lc.lcOutExtX = GetSystemMetrics(SM_CXVIRTUALSCREEN); //SM_CXSCREEN );

	//														// In Wintab, the tablet origin is lower left.  Move origin to upper left
	//														// so that it coincides with screen origin.
	//m_lc.lcOutExtY = -GetSystemMetrics(SM_CYVIRTUALSCREEN);	//SM_CYSCREEN );

	//														// Leave the system origin and extents as received:
	//														// lcSysOrgX, lcSysOrgY, lcSysExtX, lcSysExtY

	//														// open the region
	//														// The Wintab spec says we must open the context disabled if we are
	//														// using cursor masks.
	hctx = gpWTOpenA(hWnd, &m_lc, TRUE);

	WacomTrace("HCTX: %i\n", hctx);

	return hctx;
}

void Renderview::InitOpenVR()
{
	//openvr initilization
	vr::EVRInitError vr_error;
	m_vr_system = vr::VR_Init(&vr_error, vr::VRApplication_Scene, 0);
	if (vr_error == vr::VRInitError_None &&
		vr::VRCompositor())
	{
		setValue(gstOpenvrEnable, true);
		//get render size
		uint32_t uix, uiy;
		m_vr_system->GetRecommendedRenderTargetSize(&uix, &uiy);
		setValue(gstVrSizeX, (unsigned long)(uix));
		setValue(gstVrSizeY, (unsigned long)(uiy));
		//get eye offset
		//vr::HmdMatrix34_t eye_mat;
		//eye_mat = m_vr_system->GetEyeToHeadTransform(vr::Eye_Left);
		//double eye_x = eye_mat.m[0][3];
		//double eye_y = eye_mat.m[1][3];
		//double eye_z = eye_mat.m[2][3];
		//m_vr_eye_offset = std::sqrt(eye_x*eye_x+eye_y*eye_y+eye_z*eye_z)*100.0;
	}//otherwise use default settings
}

bool Renderview::UpdateController()
{
	bool refresh = false;
	if (m_controller->IsConnected())
	{
		XINPUT_STATE xstate = m_controller->GetState();
		double dzone = 0.2;
		double sclr = 15.0;
		double leftx = double(xstate.Gamepad.sThumbLX) / 32767.0;
		if (leftx > -dzone && leftx < dzone) leftx = 0.0;
		double lefty = double(xstate.Gamepad.sThumbLY) / 32767.0;
		if (lefty > -dzone && lefty < dzone) lefty = 0.0;
		double rghtx = double(xstate.Gamepad.sThumbRX) / 32767.0;
		if (rghtx > -dzone && rghtx < dzone) rghtx = 0.0;
		double rghty = double(xstate.Gamepad.sThumbRY) / 32767.0;
		if (rghty > -dzone && rghty < dzone) rghty = 0.0;
		int px = 0;
		int py = 0;
		int inc = 5;
		if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) py = -inc;
		if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) py = inc;
		if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) px = -inc;
		if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) px = inc;

		long nx, ny;
		getValue(gstSizeX, nx);
		getValue(gstSizeY, ny);
		//horizontal move
		if (leftx != 0.0)
		{
			double dx, dy, dz;
			getValue(gstCamTransX, dx);
			getValue(gstCamTransY, dy);
			getValue(gstCamTransZ, dz);
			Vector head(-dx, -dy, -dz);
			head.normalize();
			setValue(gstCamHead, head);
			Vector up;
			getValue(gstCamUp, up);
			Vector side = Cross(up, head);
			double dr, dl;
			getValue(gstOrthoLeft, dl);
			getValue(gstOrthoRight, dr);
			Vector trans = side * (leftx*sclr*(dr - dl) / double(nx));
			getValue(gstObjTransX, dx);
			getValue(gstObjTransY, dy);
			getValue(gstObjTransZ, dz);
			dx += trans.x();
			dy += trans.y();
			dz += trans.z();
			setValue(gstObjTransX, dx);
			setValue(gstObjTransY, dy);
			setValue(gstObjTransZ, dz);
			setValue(gstInteractive, true);
			setValue(gstRotCtrDirty, true);
			refresh = true;
		}
		//zoom/dolly
		if (lefty != 0.0)
		{
			double delta = lefty * sclr / (double)ny;
			double scale_factor;
			getValue(gstScaleFactor, scale_factor);
			scale_factor += scale_factor * delta;
			//m_vrv->UpdateScaleFactor(false);
			bool bval;
			getValue(gstFree, bval);
			if (bval)
			{
				double dx, dy, dz;
				getValue(gstCamTransX, dx);
				getValue(gstCamTransY, dy);
				getValue(gstCamTransZ, dz);
				Vector pos(dx, dy, dz);
				pos.normalize();
				getValue(gstCamCtrX, dx);
				getValue(gstCamCtrY, dy);
				getValue(gstCamCtrZ, dz);
				Vector ctr(dx, dy, dz);
				ctr -= delta * pos * 1000;
				setValue(gstCamCtrX, ctr.x());
				setValue(gstCamCtrY, ctr.y());
				setValue(gstCamCtrZ, ctr.z());
			}
			setValue(gstInteractive, true);
			refresh = true;
		}
		//rotate
		if (rghtx != 0.0 || rghty != 0.0)
		{
			Quaternion q_delta = Trackball(rghtx*sclr, rghty*sclr);
			Quaternion q;
			getValue(gstCamRotQ, q);
			q *= q_delta;
			q.Normalize();
			setValue(gstCamRotQ, q);
			double dist;
			getValue(gstCamDist, dist);
			Quaternion cam_pos(0.0, 0.0, dist, 0.0);
			Quaternion cam_pos2 = (-q) * cam_pos * q;
			setValue(gstCamTransX, cam_pos2.x);
			setValue(gstCamTransY, cam_pos2.y);
			setValue(gstCamTransZ, cam_pos2.z);
			Quaternion up2(0.0, 1.0, 0.0, 0.0);
			up2 = (-q) * up2 * q;
			Vector up(up2.x, up2.y, up2.z);
			setValue(gstCamUp, up);
			double dx, dy, dz;
			q.ToEuler(dx, dy, dz);
			setValue(gstCamRotX, dx);
			setValue(gstCamRotY, dy);
			setValue(gstCamRotZ, dz);
			//wxString str = wxString::Format("%.1f", m_rotx);
			//m_vrv->m_x_rot_text->ChangeValue(str);
			//str = wxString::Format("%.1f", m_roty);
			//m_vrv->m_y_rot_text->ChangeValue(str);
			//str = wxString::Format("%.1f", m_rotz);
			//m_vrv->m_z_rot_text->ChangeValue(str);
			//if (!m_vrv->m_rot_slider)
			//{
			//	m_vrv->m_x_rot_sldr->SetThumbPosition(int(m_rotx));
			//	m_vrv->m_y_rot_sldr->SetThumbPosition(int(m_roty));
			//	m_vrv->m_z_rot_sldr->SetThumbPosition(int(m_rotz));
			//}
			setValue(gstInteractive, true);
			refresh = true;
		}
		//pan
		if (px != 0 || py != 0)
		{
			double dx, dy, dz;
			getValue(gstCamTransX, dx);
			getValue(gstCamTransY, dy);
			getValue(gstCamTransZ, dz);
			Vector head(-dx, -dy, -dz);
			head.normalize();
			setValue(gstCamHead, head);
			Vector up;
			getValue(gstCamUp, up);
			Vector side = Cross(up, head);
			double dr, dl, dt, db;
			getValue(gstOrthoLeft, dl);
			getValue(gstOrthoRight, dr);
			getValue(gstOrthoTop, dt);
			getValue(gstOrthoBottom, db);
			Vector trans =
				side * (double(px)*(dr - dl) / double(nx)) +
				up * (double(py)*(dt - db) / double(ny));
			getValue(gstObjTransX, dx);
			getValue(gstObjTransY, dy);
			getValue(gstObjTransZ, dz);
			dx += trans.x();
			dy += trans.y();
			dz += trans.z();
			setValue(gstObjTransX, dx);
			setValue(gstObjTransY, dy);
			setValue(gstObjTransZ, dz);
			setValue(gstInteractive, true);
			setValue(gstRotCtrDirty, true);
			refresh = true;
		}
	}
	return refresh;
}
#endif

void Renderview::SetPressure(int np, int tp)
{
	if (m_selector->GetBrushUsePres())
		m_selector->SetPressure(np, tp);
}

//handle mouse interactions
void Renderview::HandleMouse()
{
	bool refresh = false;

	//properties need update in the end
	bool interactive = false;
	bool paint_enable = false;
	bool clear_paint = false;
	bool retain_fb = false;
	bool pick = false;
	bool pick_lock_center = false;
	bool force_clear = false;
	bool grow_on = false;
	bool rot_center_dirty = false;

	//properties from os
	long nx, ny;
	getValue(gstSizeX, nx);
	getValue(gstSizeY, ny);
	long lx, ly;
	getValue(gstMouseX, lx);
	getValue(gstMouseY, ly);
	bool left_hold, middle_hold, right_hold;
	getValue(gstMouseLeftHold, left_hold);
	getValue(gstMouseMiddleHold, middle_hold);
	getValue(gstMouseRightHold, right_hold);
	bool alt_down;
	getValue(gstKbAltDown, alt_down);
	bool ctrl_down;
	getValue(gstKbCtrlDown, ctrl_down);

	//other properties for reading and updating
	long int_mode;
	getValue(gstInterMode, int_mode);
	long lpx, lpy;
	getValue(gstMousePrvX, lpx);
	getValue(gstMousePrvY, lpy);
	//other properties without updating
	bool geared_rot;
	getValue(gstGearedEnable, geared_rot);

	//mouse button down operations
	bool bval;
	getValue(gstMouseLeftDown, bval);
	if (bval)
	{
		bool found_rp = false;

		if (int_mode == 6 ||
			int_mode == 9 ||
			int_mode == 11 ||
			int_mode == 14)
		{
			found_rp = m_ruler_handler->FindEditingRuler(lx, ly);
		}
		if (found_rp)
		{
			if (int_mode == 11)
			{
				flrd::RulerPoint *p = m_ruler_handler->GetPoint();
				if (p) p->ToggleLocked();
			}
			if (int_mode == 14)
				m_ruler_handler->DeletePoint();
			//if (m_frame && m_frame->GetMeasureDlg())
			//	m_frame->GetMeasureDlg()->GetSettings(this);
			refresh = true;
			//RefreshGL(41);
		}

		if (int_mode == 1 ||
			(int_mode == 5 && alt_down) ||
			((int_mode == 6 ||
			int_mode == 9 ||
			int_mode == 11 ||
			int_mode == 14) &&
			!found_rp))
		{
			pick = true;
		}
		else if (int_mode == 2 || int_mode == 7)
		{
			//old_mouse_X = event.GetX();
			//old_mouse_Y = event.GetY();
			lpx = lx;
			lpy = ly;
			//prv_mouse_X = old_mouse_X;
			//prv_mouse_Y = old_mouse_Y;
			paint_enable = true;
			clear_paint = true;
			m_selector->SetBrushPressPeak(0.0);
			refresh = true;
			//RefreshGL(26);
		}

		if (int_mode == 10 ||
			int_mode == 12)
		{
			m_selector->ResetMousePos();
			m_selector->SetInitMask(1);
			Segment();
			m_selector->SetInitMask(3);
			if (int_mode == 12)
				GetCurrentVolume()->AddEmptyLabel(0, false);
			force_clear = true;
			grow_on = true;
		}
	}
	getValue(gstMouseRightDown, bval);
	if (bval)
	{
		//nothing so far
	}
	getValue(gstMouseMiddleDown, bval);
	if (bval)
	{
		//nothing so far
	}

	//mouse button up operations
	getValue(gstMouseLeftUp, bval);
	if (bval)
	{
		if (int_mode == 1)
		{
			//pick stuff
			getValue(gstPick, bval);
			if (bval)
			{
				Pick();
				pick_lock_center = false;
			}
		}
		else if (int_mode == 2)
		{
			//segment volumes
			paint_enable = true;
			Segment();
			int_mode = 4;
			force_clear = true;
			refresh = true;
			//RefreshGL(27);
			//return;
		}
		else if (int_mode == 5 && !alt_down)
		{
			//add one point to a ruler
			m_ruler_handler->AddRulerPoint(lx, ly, true);
			//if (m_frame && m_frame->GetMeasureDlg())
			//	m_frame->GetMeasureDlg()->GetSettings(this);
			refresh = true;
			//RefreshGL(27);
			//return;
		}
		else if (int_mode == 6 ||
			int_mode == 9 ||
			int_mode == 11)
		{
			m_ruler_handler->SetPoint(0);
		}
		else if (int_mode == 7)
		{
			//segment volume, calculate center, add ruler point
			paint_enable = true;
			Segment();
			if (m_ruler_handler->GetType() == 3)
				m_ruler_handler->AddRulerPoint(lx, ly, true);
			else
				m_ruler_handler->AddPaintRulerPoint();
			int_mode = 8;
			force_clear = true;
			refresh = true;
			//RefreshGL(27);
			//if (m_frame && m_frame->GetMeasureDlg())
			//	m_frame->GetMeasureDlg()->GetSettings(this);
			//return;
		}
		else if (int_mode == 10 ||
			int_mode == 12)
		{
			grow_on = false;
			//return;
		}
		else if (int_mode == 13)
		{
			//relax ruler function needs to be added
			refresh = true;
			//if (m_frame && m_frame->GetMeasureDlg())
			//{
			//	if (m_ruler_autorelax)
			//	{
			//		m_frame->GetMeasureDlg()->SetEdit();
			//		m_frame->GetMeasureDlg()->Relax(
			//			m_ruler_handler.GetRulerIndex());
			//	}
			//	m_frame->GetMeasureDlg()->GetSettings(this);
			//}
			//RefreshGL(29);
			//return;
		}
	}
	getValue(gstMouseMiddleUp, bval);
	if (bval)
	{
		//SetSortBricks();
		//RefreshGL(28);
		//return;
	}
	getValue(gstMouseRightUp, bval);
	if (bval)
	{
		if (int_mode == 1)
		{
			//RefreshGL(27);
			//return;
		}
		if (int_mode == 5 && !alt_down)
		{
			if (m_ruler_handler->GetRulerFinished())
			{
				int_mode = 1;
				//SetIntMode(1);
			}
			else
			{
				m_ruler_handler->AddRulerPoint(lx, ly, true);
				m_ruler_handler->FinishRuler();
			}
			//ruler relax function needs to be added
			//if (m_frame && m_frame->GetMeasureDlg())
			//{
			//	if (m_ruler_autorelax)
			//	{
			//		m_frame->GetMeasureDlg()->SetEdit();
			//		m_frame->GetMeasureDlg()->Relax(
			//			m_ruler_handler.GetRulerIndex());
			//	}
			//	m_frame->GetMeasureDlg()->GetSettings(this);
			//}
			refresh = true;
			//RefreshGL(29);
			//return;
		}
		//SetSortBricks();
	}

	//mouse dragging
	getValue(gstMouseDrag, bval);
	if (bval)
	{
		flvr::TextureRenderer::set_cor_up_time(
			int(sqrt(double(lpx - lx)*
				double(lpx - lx) +
				double(lpy - ly)*
				double(lpy - ly))));

		flrd::RulerPoint *p0 = m_ruler_handler->GetPoint();
		bool hold_old = false;
		if (int_mode == 1 ||
			(int_mode == 5 && alt_down) ||
			((int_mode == 6 ||
			int_mode == 9 ||
			int_mode == 10 ||
			int_mode == 11 ||
			int_mode == 12 ||
			int_mode == 14) &&
			!p0))
		{
			//disable picking
			pick = false;

			if (lpx != -1 && lpx != -1 &&
				abs(lpx - lx) + abs(lpy - ly) < 200)
			{
				if (left_hold &&
					!ctrl_down &&
					int_mode != 10 &&
					int_mode != 12)
				{
					Quaternion q_delta = Trackball(lx - lpx, lpy - ly);
					if (geared_rot && q_delta.IsIdentity())
						hold_old = true;
					Quaternion q;
					getValue(gstCamRotQ, q);
					q *= q_delta;
					q.Normalize();
					setValue(gstCamRotQ, q);
					double dist;
					getValue(gstCamDist, dist);
					Quaternion cam_pos(0.0, 0.0, dist, 0.0);
					Quaternion cam_pos2 = (-q) * cam_pos * q;
					setValue(gstCamTransX, cam_pos2.x);
					setValue(gstCamTransY, cam_pos2.y);
					setValue(gstCamTransZ, cam_pos2.z);

					Quaternion up(0.0, 1.0, 0.0, 0.0);
					up = (-q) * up * q;
					setValue(gstCamUp, Vector(up.x, up.y, up.z));

					Q2A();

					//wxString str = wxString::Format("%.1f", m_rotx);
					//m_vrv->m_x_rot_text->ChangeValue(str);
					//str = wxString::Format("%.1f", m_roty);
					//m_vrv->m_y_rot_text->ChangeValue(str);
					//str = wxString::Format("%.1f", m_rotz);
					//m_vrv->m_z_rot_text->ChangeValue(str);
					//if (!m_vrv->m_rot_slider)
					//{
					//	m_vrv->m_x_rot_sldr->SetThumbPosition(int(m_rotx));
					//	m_vrv->m_y_rot_sldr->SetThumbPosition(int(m_roty));
					//	m_vrv->m_z_rot_sldr->SetThumbPosition(int(m_rotz));
					//}

					interactive = true;

					//if (m_linked_rot)
					//	m_master_linked_view = this;

					if (!hold_old)
						refresh = true;
					//RefreshGL(30);
				}
				if (middle_hold || (ctrl_down && left_hold))
				{
					long distx = lx - lpx;
					long disty = ly - lpy;

					double dx, dy, dz;
					getValue(gstCamTransX, dx);
					getValue(gstCamTransY, dy);
					getValue(gstCamTransZ, dz);
					Vector head(-dx, -dy, -dz);
					head.normalize();
					setValue(gstCamHead, head);
					Vector up;
					getValue(gstCamUp, up);
					Vector side = Cross(up, head);
					double dl, dr, dt, db;
					getValue(gstOrthoLeft, dl);
					getValue(gstOrthoRight, dr);
					getValue(gstOrthoTop, dt);
					getValue(gstOrthoBottom, db);
					Vector trans = -(side*(distx*(dr - dl) / nx) +
						up * (disty*(dt - db) / ny));
					getValue(gstObjTransX, dx);
					getValue(gstObjTransY, dy);
					getValue(gstObjTransZ, dz);
					setValue(gstObjTransX, dx + trans.x());
					setValue(gstObjTransY, dy + trans.y());
					setValue(gstObjTransZ, dz + trans.z());

					interactive = true;
					rot_center_dirty = true;
					refresh = true;

					//SetSortBricks();
					//RefreshGL(31);
				}
				if (right_hold)
				{
					long distx = lx - lpx;
					long disty = ly - lpy;

					double delta = abs(distx) > abs(disty) ?
						(double)distx / (double)nx :
						(double)-disty / (double)ny;
					double scale_factor;
					getValue(gstScaleFactor, scale_factor);
					scale_factor += scale_factor * delta;
					setValue(gstScaleFactor, scale_factor);
					//m_vrv->UpdateScaleFactor(false);
					//wxString str = wxString::Format("%.0f", m_scale_factor*100.0);
					//m_vrv->m_scale_factor_sldr->SetValue(m_scale_factor*100);
					//m_vrv->m_scale_factor_text->ChangeValue(str);

					getValue(gstFree, bval);
					if (bval)
					{
						double dx, dy, dz;
						getValue(gstCamTransX, dx);
						getValue(gstCamTransY, dy);
						getValue(gstCamTransZ, dz);
						Vector pos(dx, dy, dz);
						pos.normalize();
						getValue(gstCamCtrX, dx);
						getValue(gstCamCtrY, dy);
						getValue(gstCamCtrZ, dz);
						Vector ctr(dx, dy, dz);
						ctr -= delta * pos * 1000;
						setValue(gstCamCtrX, ctr.x());
						setValue(gstCamCtrY, ctr.y());
						setValue(gstCamCtrZ, ctr.z());
					}

					interactive = true;
					refresh = true;

					//SetSortBricks();
					//RefreshGL(32);
				}
			}
		}
		else if (int_mode == 2 || int_mode == 7)
		{
			paint_enable = true;
			refresh = true;
			//RefreshGL(33);
		}
		else if (int_mode == 3)
		{
			if (lpx != -1 && lpy != -1 &&
				abs(lpx - lx) + abs(lpy - ly) < 200)
			{
				if (left_hold)
				{
					Quaternion q_delta = TrackballClip(lpx, ly, lx, lpy);
					Quaternion q_cl;
					getValue(gstClipRotQ, q_cl);
					q_cl = q_delta * q_cl;
					q_cl.Normalize();
					setValue(gstClipRotQ, q_cl);
					refresh = true;
					//SetRotations(m_rotx, m_roty, m_rotz);
					//RefreshGL(34);
				}
			}
		}
		else if (int_mode == 6 || int_mode == 9)
		{
			bool rval = false;
			if (int_mode == 6)
				rval = m_ruler_handler->EditPoint(lx, ly, alt_down);
			else if (int_mode == 9)
				rval = m_ruler_handler->MoveRuler(lx, ly);
			if (rval)
			{
				refresh = true;
				//RefreshGL(35);
				//if (m_frame && m_frame->GetMeasureDlg())
				//{
				//	m_frame->GetMeasureDlg()->GetSettings(this);
				//	m_frame->GetMeasureDlg()->SetEdit();
				//}
			}
		}
		else if (int_mode == 13)
		{
			if (m_ruler_handler->GetMouseDist(lx, ly, 35))
			{
				//add one point to a ruler
				m_ruler_handler->AddRulerPoint(lx, ly, true);
				refresh = true;
				//if (m_frame && m_frame->GetMeasureDlg())
				//	m_frame->GetMeasureDlg()->GetSettings(this);
				//RefreshGL(27);
			}
		}

		//else
		//{
		//	old_mouse_X = event.GetX();
		//	old_mouse_Y = event.GetY();
		//	prv_mouse_X = old_mouse_X;
		//	prv_mouse_Y = old_mouse_Y;
		//}
		//return;
	}

	//wheel operations
	long wheel;
	getValue(gstMouseWheel, wheel);
	if (wheel)  //if rotation
	{
		if (int_mode == 2 || int_mode == 7)
		{
			//ChangeBrushSize(wheel);
		}
		else
		{
			interactive = true;
			rot_center_dirty = true;
			double scale_factor;
			getValue(gstScaleFactor, scale_factor);
			double value = wheel * scale_factor / 1000.0;
			if (scale_factor + value > 0.01)
				scale_factor += value;
			setValue(gstScaleFactor, scale_factor);
			//if (m_scale_factor < 0.01)
			//	m_scale_factor = 0.01;
			//m_vrv->UpdateScaleFactor(false);
			//wxString str = wxString::Format("%.0f", m_scale_factor*100.0);
			//m_vrv->m_scale_factor_sldr->SetValue(m_scale_factor*100);
			//m_vrv->m_scale_factor_text->ChangeValue(str);
		}
		refresh = true;
		//RefreshGL(36);
		//return;
	}

	// draw the strokes into a framebuffer texture
	//not actually for displaying it
	//getValue(gstDrawBrush, bval);
	//if (bval)
	//{
	//	//old_mouse_X = event.GetX();
	//	//old_mouse_Y = event.GetY();
	//	//RefreshGL(37);
	//	//return;
	//}

	long draw_info;
	getValue(gstDrawInfo, draw_info);
	if (draw_info & INFO_DISP)
	{
		bool movie_running, vr_enable;
		getValue(gstMovRunning, movie_running);
		getValue(gstVrEnable, vr_enable);
		if (!movie_running && !vr_enable)
		{
			retain_fb = true;
			refresh = true;
		}
		//if (m_frame && m_frame->GetMovieView() &&
		//	m_frame->GetMovieView()->GetRunning())
		//	return;
		//if (m_enable_vr)
		//	return;

		//retain_fb = true;
		//return;
	}

	//update properties
	setValue(gstInteractive, interactive);
	setValue(gstPaintEnable, paint_enable);
	setValue(gstClearPaint, clear_paint);
	setValue(gstRetainFb, retain_fb);
	setValue(gstPick, pick);
	setValue(gstCamLockPick, pick_lock_center);
	setValue(gstForceClear, force_clear);
	setValue(gstGrowEnable, grow_on);
	setValue(gstRotCtrDirty, rot_center_dirty);
	//actually draw everything
	if (refresh)
	{//todo: refresh only when necessary
#ifdef _WIN32
		Update(38, false);
#else
		Update(38, true);
#endif
	}
	//update mouse position
	//if (lpx >= 0 && lpy >= 0)
	{
		setValue(gstMousePrvX, lx);
		setValue(gstMousePrvY, ly);
		//if (!hold_old)
		//{
		//	old_mouse_X = event.GetX();
		//	old_mouse_Y = event.GetY();
		//}
	}
}

void Renderview::HandleIdle()
{
	bool bval;
	long lval;
	bool refresh = false;
	bool start_loop = true;
	bool set_focus = false;
	bool retain_fb = false;
	bool pre_draw = false;

	//check memory swap status
	if (flvr::TextureRenderer::get_mem_swap() &&
		flvr::TextureRenderer::get_start_update_loop() &&
		!flvr::TextureRenderer::get_done_update_loop())
	{
		getValue(gstRenderviewPanelId, lval);
		if (flvr::TextureRenderer::active_view_ == lval)
		{
			refresh = true;
			start_loop = false;
		}
	}
	bool capture_rotat, capture_tsequ, capture_param, test_speed;
	getValue(gstCaptureRot, capture_rotat);
	getValue(gstCaptureTime, capture_tsequ);
	getValue(gstCaptureParam, capture_param);
	getValue(gstTestSpeed, test_speed);
	if (capture_rotat ||
		capture_tsequ ||
		capture_param ||
		test_speed)
	{
		refresh = true;
		if (flvr::TextureRenderer::get_mem_swap() &&
			flvr::TextureRenderer::get_done_update_loop())
			pre_draw = true;
	}

	getValue(gstOpenvrEnable, bval);
	if (bval)
	{
		//event.RequestMore();
		refresh = true;
		//m_retain_finalbuffer = true;
	}

	getValue(gstBenchmark, bval);
	if (bval)
	{
		double fps = 1.0 / glbin_timer->average();
		//wxString title = wxString(FLUORENDER_TITLE) +
		//	" " + wxString(VERSION_MAJOR_TAG) +
		//	"." + wxString(VERSION_MINOR_TAG) +
		//	" Benchmarking... FPS = " +
		//	wxString::Format("%.2f", fps);
		//m_frame->SetTitle(title);

		refresh = true;
		if (flvr::TextureRenderer::get_mem_swap() &&
			flvr::TextureRenderer::get_done_update_loop())
			pre_draw = true;
	}

	//pin rotation center
	refresh = refresh || PinRotCtr();

	getValue(gstMouseIn, bval);
	if (bval)
	{
		bool ctrl_down;
		glbin_input->getValue(gstKbCtrlHold, ctrl_down);
		bool spc_down;
		glbin_input->getValue(gstKbSpaceHold, spc_down);

		refresh = refresh || UpdateBrushState();

		//draw_mask
		glbin_input->getValue(gstKbVHold, bval);
		if (bval)
		{
			setValue(gstDrawMask, false);
			refresh = true;
			set_focus = true;
		}
		else
		{
			setValue(gstDrawMask, true);
			refresh = true;
		}

		//move view
		//left
		glbin_input->getValue(gstKbLeftDown, bval);
		if (bval && ctrl_down)
		{
			double dx, dy, dz;
			getValue(gstCamTransX, dx);
			getValue(gstCamTransY, dy);
			getValue(gstCamTransZ, dz);
			Vector head(-dx, -dy, -dz);
			head.normalize();
			setValue(gstCamHead, head);
			Vector up;
			getValue(gstCamUp, up);
			Vector side = Cross(up, head);
			double dr, dl;
			getValue(gstOrthoLeft, dl);
			getValue(gstOrthoRight, dr);
			Vector trans = -(side*(int(0.8*(dr - dl))));
			getValue(gstObjTransX, dx);
			getValue(gstObjTransY, dy);
			getValue(gstObjTransZ, dz);
			dx += trans.x();
			dy += trans.y();
			dz += trans.z();
			setValue(gstObjTransX, dx);
			setValue(gstObjTransY, dy);
			setValue(gstObjTransZ, dz);
			//if (m_persp) SetSortBricks();
			refresh = true;
			set_focus = true;
		}
		//right
		glbin_input->getValue(gstKbRightDown, bval);
		if (bval && ctrl_down)
		{
			double dx, dy, dz;
			getValue(gstCamTransX, dx);
			getValue(gstCamTransY, dy);
			getValue(gstCamTransZ, dz);
			Vector head(-dx, -dy, -dz);
			head.normalize();
			setValue(gstCamHead, head);
			Vector up;
			getValue(gstCamUp, up);
			Vector side = Cross(up, head);
			double dr, dl;
			getValue(gstOrthoLeft, dl);
			getValue(gstOrthoRight, dr);
			Vector trans = side * (int(0.8*(dr - dl)));
			getValue(gstObjTransX, dx);
			getValue(gstObjTransY, dy);
			getValue(gstObjTransZ, dz);
			dx += trans.x();
			dy += trans.y();
			dz += trans.z();
			setValue(gstObjTransX, dx);
			setValue(gstObjTransY, dy);
			setValue(gstObjTransZ, dz);
			//if (m_persp) SetSortBricks();
			refresh = true;
			set_focus = true;
		}
		//up
		glbin_input->getValue(gstKbUpDown, bval);
		if (bval && ctrl_down)
		{
			double dx, dy, dz;
			getValue(gstCamTransX, dx);
			getValue(gstCamTransY, dy);
			getValue(gstCamTransZ, dz);
			Vector head(-dx, -dy, -dz);
			head.normalize();
			setValue(gstCamHead, head);
			Vector up;
			getValue(gstCamUp, up);
			double dt, db;
			getValue(gstOrthoTop, dt);
			getValue(gstOrthoBottom, db);
			Vector trans = -up * (int(0.8*(dt - db)));
			getValue(gstObjTransX, dx);
			getValue(gstObjTransY, dy);
			getValue(gstObjTransZ, dz);
			dx += trans.x();
			dy += trans.y();
			dz += trans.z();
			setValue(gstObjTransX, dx);
			setValue(gstObjTransY, dy);
			setValue(gstObjTransZ, dz);
			//if (m_persp) SetSortBricks();
			refresh = true;
			set_focus = true;
		}
		//down
		glbin_input->getValue(gstKbDownDown, bval);
		if (bval && ctrl_down)
		{
			double dx, dy, dz;
			getValue(gstCamTransX, dx);
			getValue(gstCamTransY, dy);
			getValue(gstCamTransZ, dz);
			Vector head(-dx, -dy, -dz);
			head.normalize();
			setValue(gstCamHead, head);
			Vector up;
			getValue(gstCamUp, up);
			double dt, db;
			getValue(gstOrthoTop, dt);
			getValue(gstOrthoBottom, db);
			Vector trans = up * (int(0.8*(dt - db)));
			getValue(gstObjTransX, dx);
			getValue(gstObjTransY, dy);
			getValue(gstObjTransZ, dz);
			dx += trans.x();
			dy += trans.y();
			dz += trans.z();
			setValue(gstObjTransX, dx);
			setValue(gstObjTransY, dy);
			setValue(gstObjTransZ, dz);
			//if (m_persp) SetSortBricks();
			refresh = true;
			set_focus = true;
		}

		//move time sequence
		//forward
		glbin_input->getValue(gstKbDDown, bval);
		if (bval || spc_down)
		{
			//if (m_frame && m_frame->GetMovieView())
			//	m_frame->GetMovieView()->UpFrame();
			refresh = true;
			set_focus = true;
		}
		//backforward
		glbin_input->getValue(gstKbADown, bval);
		if (bval)
		{
			//if (m_frame && m_frame->GetMovieView())
			//	m_frame->GetMovieView()->DownFrame();
			refresh = true;
			set_focus = true;
		}

		//move clip planes
		//up
		glbin_input->getValue(gstKbSDown, bval);
		if (bval)
		{
			ClipPlaneAgent* agent = glbin_agtf->findFirst(gstClipPlaneAgent)->asClipPlaneAgent();
			if (agent)
			{
				double z, res;
				agent->getValue(gstClipZ1, z);
				agent->getValue(gstResZ, res);
				agent->setValue(gstClipZ1, z + 1 / res);
			}
			refresh = true;
			set_focus = true;
		}
		//down
		glbin_input->getValue(gstKbWDown, bval);
		if (bval)
		{
			ClipPlaneAgent* agent = glbin_agtf->findFirst(gstClipPlaneAgent)->asClipPlaneAgent();
			if (agent)
			{
				double z, res;
				agent->getValue(gstClipZ1, z);
				agent->getValue(gstResZ, res);
				agent->setValue(gstClipZ1, z - 1 / res);
			}
			refresh = true;
			set_focus = true;
		}

		//cell full
		glbin_input->getValue(gstKbFDown, bval);
		if (bval)
		{
			//if (m_frame && m_frame->GetComponentDlg())
			//	m_frame->GetComponentDlg()->SelectFullComp();
			//if (m_frame && m_frame->GetTraceDlg())
			//	m_frame->GetTraceDlg()->CellUpdate();
			refresh = true;
			set_focus = true;
		}
		//cell link
		glbin_input->getValue(gstKbLDown, bval);
		if (bval)
		{
			//if (m_frame && m_frame->GetTraceDlg())
			//	m_frame->GetTraceDlg()->CellLink(false);
			refresh = true;
			set_focus = true;
		}
		//new cell id
		glbin_input->getValue(gstKbNDown, bval);
		if (bval)
		{
			//if (m_frame && m_frame->GetTraceDlg())
			//	m_frame->GetTraceDlg()->CellNewID(false);
			refresh = true;
			set_focus = true;
		}
		//clear
		glbin_input->getValue(gstKbCDown, bval);
		if (bval)
		{
			//if (m_frame && m_frame->GetTree())
			//	m_frame->GetTree()->BrushClear();
			//if (m_frame && m_frame->GetTraceDlg())
			//	m_frame->GetTraceDlg()->CompClear();
			refresh = true;
			set_focus = true;
		}
		//save all masks
		glbin_input->getValue(gstKbMDown, bval);
		if (bval)
		{
			glbin.saveAllMasks();
			set_focus = true;
		}
		//brush size
		glbin_input->getValue(gstKbLbrktHold, bval);
		if (bval)
		{
			ChangeBrushSize(-10);
			set_focus = true;
		}
		glbin_input->getValue(gstKbRbrktHold, bval);
		if (bval)
		{
			ChangeBrushSize(10);
			set_focus = true;
		}
		//comp include
		glbin_input->getValue(gstKbReturnDown, bval);
		if (bval)
		{
			//if (m_frame && m_frame->GetComponentDlg())
			//	m_frame->GetComponentDlg()->IncludeComps();
			refresh = true;
			set_focus = true;
		}
		//comp exclude
		glbin_input->getValue(gstKbBslshDown, bval);
		if (bval)
		{
			//if (m_frame && m_frame->GetComponentDlg())
			//	m_frame->GetComponentDlg()->ExcludeComps();
			refresh = true;
			set_focus = true;
		}
		//ruler relax
		glbin_input->getValue(gstKbRDown, bval);
		if (bval)
		{
			//if (m_frame && m_frame->GetMeasureDlg())
			//	m_frame->GetMeasureDlg()->Relax();
			refresh = true;
			set_focus = true;
		}

		long int_mode;
		getValue(gstInterMode, int_mode);
		bool grow_on;
		getValue(gstGrowEnable, grow_on);
		getValue(gstMouseLeftHold, bval);
		//grow
		if ((int_mode == 10 ||
			int_mode == 12) &&
			bval && grow_on)
		{
			int sz = 5;
			//if (m_frame && m_frame->GetSettingDlg())
			//	sz = m_frame->GetSettingDlg()->GetRulerSizeThresh();
			//event.RequestMore();
			Grow(sz);
			refresh = true;
			start_loop = true;
			//update
			bool bval;
			getValue(gstPaintCount, bval);
			//if (bval && m_frame->GetBrushToolDlg())
			//	m_frame->GetBrushToolDlg()->Update(0);
			getValue(gstPaintColocalize, bval);
			if (bval)
			{
				ColocalAgent* agent = glbin_agtf->findFirst(gstColocalAgent)->asColocalAgent();
				if (agent) agent->Run();
			}
			//if (int_mode == 12 && m_frame->GetMeasureDlg())
			//	m_frame->GetMeasureDlg()->GetSettings(getObject());
		}

		//forced refresh
		glbin_input->getValue(gstKbF5Down, bval);
		if (bval)
		{
			//SetFocus();
			//if (m_frame && m_frame->GetStatusBar())
			//	m_frame->GetStatusBar()->PushStatusText("Forced Refresh");
			//wxSizeEvent e;
			//OnResize(e);

			//setValue(gstClearBuffer, true);
			//setValue(gstUpdating, true);
			//setValue(gstPreDraw, pre_draw);
			//setValue(gstRetainFb, retain_fb);
			//getObject()->Update(14);

			//if (m_frame && m_frame->GetStatusBar())
			//	m_frame->GetStatusBar()->PopStatusText();
			//return;
			set_focus = true;
			refresh = true;
		}
	}

#ifdef _WIN32
	//update ortho rotation
	Quaternion q;
	getValue(gstCamRotQ, q);
	int cmb_sel = 6;
	if (q.AlmostEqual(Quaternion(0, sqrt(2.0) / 2.0, 0, sqrt(2.0) / 2.0)))
		cmb_sel = 0;
	else if (q.AlmostEqual(Quaternion(0, -sqrt(2.0) / 2.0, 0, sqrt(2.0) / 2.0)))
		cmb_sel = 1;
	else if (q.AlmostEqual(Quaternion(sqrt(2.0) / 2.0, 0, 0, sqrt(2.0) / 2.0)))
		cmb_sel = 2;
	else if (q.AlmostEqual(Quaternion(-sqrt(2.0) / 2.0, 0, 0, sqrt(2.0) / 2.0)))
		cmb_sel = 3;
	else if (q.AlmostEqual(Quaternion(0, 0, 0, 1)))
		cmb_sel = 4;
	else if (q.AlmostEqual(Quaternion(0, -1, 0, 0)))
		cmb_sel = 5;
	else
		cmb_sel = 6;
	//m_vrv->UpdateOrientCmb(cmb_sel);
#endif

#if defined(_WIN32) && defined(USE_XINPUT)
	//xinput controller
	refresh = refresh || UpdateController();
#endif

	//if (set_focus)
	//	SetFocus();
	setValue(gstFocus, set_focus);
	if (refresh)
	{
		setValue(gstClearBuffer, true);
		setValue(gstUpdating, true);
		setValue(gstPreDraw, pre_draw);
		setValue(gstRetainFb, retain_fb);
		//getObject()->RefreshGL(15, start_loop);
		Update(15, start_loop);
	}
}

//change brush display
void Renderview::ChangeBrushSize(int value)
{
	bool bval;
	glbin_input->getValue(gstKbCtrlHold, bval);
	GetVolumeSelector()->ChangeBrushSize(value, bval);
	//if (m_frame && m_frame->GetBrushToolDlg())
	//	m_frame->GetBrushToolDlg()->GetSettings(getObject());
}

bool Renderview::UpdateBrushState()
{
	//TreePanel* tree_panel = 0;
	//BrushToolDlg* brush_dlg = 0;
	//if (m_frame)
	//{
	//	tree_panel = m_frame->GetTree();
	//	brush_dlg = m_frame->GetBrushToolDlg();
	//}

	bool refresh = false;
	bool bval;
	long int_mode;
	getValue(gstInterMode, int_mode);
	if (int_mode != 2 && int_mode != 7)
	{
		glbin_input->getValue(gstKbShiftHold, bval);
		if (bval)
		{
			//if (tree_panel)
			//	tree_panel->SelectBrush(TreePanel::ID_BrushAppend);
			//if (brush_dlg)
			//	brush_dlg->SelectBrush(BrushToolDlg::ID_BrushAppend);
			SetBrush(2);
			refresh = true;
			//Update(6);
		}
		glbin_input->getValue(gstKbZHold, bval);
		if (!refresh && bval)
		{
			//if (tree_panel)
			//	tree_panel->SelectBrush(TreePanel::ID_BrushDiffuse);
			//if (brush_dlg)
			//	brush_dlg->SelectBrush(BrushToolDlg::ID_BrushDiffuse);
			SetBrush(4);
			refresh = true;
			//Update(7);
		}
		glbin_input->getValue(gstKbXHold, bval);
		if (!refresh && bval)
		{
			//if (tree_panel)
			//	tree_panel->SelectBrush(TreePanel::ID_BrushDesel);
			//if (brush_dlg)
			//	brush_dlg->SelectBrush(BrushToolDlg::ID_BrushDesel);
			SetBrush(3);
			refresh = true;
			//view->Update(8);
		}
	}
	else
	{
		long lval;
		getValue(gstBrushState, lval);
		if (lval)
		{
			glbin_input->getValue(gstKbShiftHold, bval);
			if (bval)
			{
				setValue(gstBrushState, long(0));
				//if (tree_panel)
				//	tree_panel->SelectBrush(TreePanel::ID_BrushAppend);
				//if (brush_dlg)
				//	brush_dlg->SelectBrush(BrushToolDlg::ID_BrushAppend);
				SetBrush(2);
				refresh = true;
				//view->Update(9);
			}
			glbin_input->getValue(gstKbZHold, bval);
			if (!refresh && bval)
			{
				setValue(gstBrushState, long(0));
				//if (tree_panel)
				//	tree_panel->SelectBrush(TreePanel::ID_BrushDiffuse);
				//if (brush_dlg)
				//	brush_dlg->SelectBrush(BrushToolDlg::ID_BrushDiffuse);
				SetBrush(4);
				refresh = true;
				//view->Update(10);
			}
			glbin_input->getValue(gstKbXHold, bval);
			if (!refresh && bval)
			{
				setValue(gstBrushState, long(0));
				//if (tree_panel)
				//	tree_panel->SelectBrush(TreePanel::ID_BrushDesel);
				//if (brush_dlg)
				//	brush_dlg->SelectBrush(BrushToolDlg::ID_BrushDesel);
				SetBrush(3);
				refresh = true;
				//view->Update(11);
			}
			if (!refresh)
			{
				SetBrush(lval);
				refresh = true;
				//view->Update(12);
			}
		}
		else
		{
			bool shift_down, z_down, x_down;
			glbin_input->getValue(gstKbShiftHold, shift_down);
			glbin_input->getValue(gstKbZHold, z_down);
			glbin_input->getValue(gstKbXHold, x_down);
			if (!shift_down &&
				!z_down &&
				!x_down)
			{
				getValue(gstMouseLeftHold, bval);
				if (bval)
					Segment();
				long lval;
				getValue(gstInterMode, lval);
				if (lval == 7)
					setValue(gstInterMode, 5);
				else
					setValue(gstInterMode, 1);
				setValue(gstPaintDisplay, false);
				setValue(gstDrawBrush, false);
				//if (tree_panel)
				//	tree_panel->SelectBrush(0);
				//if (brush_dlg)
				//	brush_dlg->SelectBrush(0);
				refresh = true;
				//view->Update(13);

				//if (m_prev_focus)
				//{
				//	m_prev_focus->SetFocus();
				//	m_prev_focus = 0;
				//}
			}
		}
	}
	return refresh;
}

//event functions/////////////////////////////////////////////////////////////////////////////////
void Renderview::OnSizeXChanged(Event& event)
{
	double dval;
	getValue(gstEnlargeScale, dval);
	if (dval == 1) return;
	long lval;
	getValue(gstSizeX, lval);
	lval = long(lval * dval + 0.5);
	setValue(gstSizeX, lval);
}

void Renderview::OnSizeYChanged(Event& event)
{
	double dval;
	getValue(gstEnlargeScale, dval);
	if (dval == 1) return;
	long lval;
	getValue(gstSizeY, lval);
	lval = long(lval * dval + 0.5);
	setValue(gstSizeY, lval);
}

void Renderview::OnEnlargeScaleChanged(Event& event)
{
	double dval;
	getValue(gstEnlargeScale, dval);
	if (dval == 1) return;
	long lx, ly;
	getValue(gstSizeX, lx);
	getValue(gstSizeY, ly);
	lx = long(lx * dval + 0.5);
	ly = long(ly * dval + 0.5);
	setValue(gstSizeX, lx);
	setValue(gstSizeY, ly);
	//text scale
	bool bval;
	getValue(gstEnlarge, bval);
	if (bval)
	{
		unsigned int tsize = flvr::TextRenderer::text_texture_manager_.GetSize();
		flvr::TextRenderer::text_texture_manager_.SetSize((unsigned int)(tsize * dval + 0.5));
	}
}

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
		//m_vrv->m_options_toolbar->ToggleTool(RenderviewPanel::ID_FreeChk, false);

		//SetRotations(m_rotx, m_roty, m_rotz);
	}
}

void Renderview::OnVolListDirtyChanged(Event& event)
{
	PopVolumeList();
	OnCurrentVolumeChanged(event);
}

void Renderview::OnMshListDirtyChanged(Event& event)
{
	PopMeshList();
	OnCurrentMeshChanged(event);
}

void Renderview::OnFullVolListDirtyChanged(Event& event)
{
	PopFullVolList();
	OnCurrentVolumeChanged(event);
}

void Renderview::OnFullMshListDirtyChanged(Event& event)
{
	PopFullMeshList();
	OnCurrentMeshChanged(event);
}

void Renderview::OnCurrentVolumeChanged(Event& event)
{
	Referenced* ref;
	getRvalu(gstCurrentVolume, &ref);
	VolumeData* cvol = dynamic_cast<VolumeData*>(ref);
	if (!cvol) return;
	long idx = -1, i = 0;
	for (auto vd : m_vol_list)
	{
		if (vd.get() == cvol)
		{
			idx = i;
			break;
		}
		i++;
	}
	setValue(gstCurVolIdx, idx);
}

void Renderview::OnCurrentMeshChanged(Event& event)
{
	Referenced* ref;
	getRvalu(gstCurrentMesh, &ref);
	MeshData* cmsh = dynamic_cast<MeshData*>(ref);
	if (!cmsh) return;
	long idx = -1, i = 0;
	for (auto md : m_msh_list)
	{
		if (md.get() == cmsh)
		{
			idx = i;
			break;
		}
		i++;
	}
	setValue(gstCurMshIdx, idx);
}

void Renderview::OnTextColorModeChanged(Event& event)
{
	Color color;
	long lval;
	getValue(gstTextColorMode, lval);
	switch (lval)
	{
	case 0://background inverted
		getValue(gstBgColorInv, color);
		break;
	case 1://background
		getValue(gstBgColor, color);
		break;
	case 2://secondary color of current volume
		{
			VolumeData* vd = GetCurrentVolume();
			if (vd)
				vd->getValue(gstSecColor, color);
			else
				getValue(gstBgColorInv, color);
		}
		break;
	}
	setValue(gstTextColor, color);
}

void Renderview::OnInterModeChanged(Event& event)
{
	long lval;
	getValue(gstInterMode, lval);
	if (lval == 1)
	{
		setValue(gstBrushState, long(0));
		setValue(gstDrawBrush, false);
	}
	else if (lval == 10 || lval == 12)
	{
		setValue(gstPaintMode, long(9));
	}
}

void Renderview::OnPaintModeChanged(Event& event)
{
	long lval;
	getValue(gstPaintMode, lval);
	m_selector->SetMode(lval);
	setValue(gstBrushState, lval);
	m_selector->ChangeBrushSetsIndex();
}