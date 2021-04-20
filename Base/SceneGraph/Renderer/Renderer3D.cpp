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
#include "Renderer3D.hpp"
#include <glm/gtx/transform.hpp>
#include <Utils.hpp>

using namespace fluo;

Renderer3D::Renderer3D():
	ProcessorNode()
{
	setupInputs();
	setupOutputs();
	setProcessFunction(
		std::bind(&Renderer3D::render,
			this, std::placeholders::_1));
}

Renderer3D::Renderer3D(const Renderer3D& renderer, const CopyOp& copyop, bool copy_values):
	ProcessorNode(renderer, copyop, false)
{
	if (copy_values)
		copyValues(renderer, copyop);
}

Renderer3D::~Renderer3D()
{
}

void Renderer3D::setupInputs()
{
}

void Renderer3D::setupOutputs()
{

}

void Renderer3D::handleProjection()
{
	bool free;
	getValue("free", free);
	double radius, aov, scale_factor;
	getValue("radius", radius);
	getValue("aov", aov);
	getValue("scale factor", scale_factor);
	double distance;
	getValue("distance", distance);
	if (!free)
	{
		distance = radius / std::tan(d2r(aov / 2.0)) / scale_factor;
		setValue("distance", distance);
	}

	long nx, ny;
	getValue("nx", nx);
	getValue("ny", ny);
	double aspect = (double)nx / (double)ny;
	double ortho_left = -radius * aspect / scale_factor;
	double ortho_right = -ortho_left;
	double ortho_bottom = -radius / scale_factor;
	double ortho_top = -ortho_bottom;

	bool use_vr;
	getValue("use vr", use_vr);
	if (use_vr)
	{/*
#ifdef _WIN32
		//get projection matrix
		vr::EVREye eye = m_vr_eye_idx ? vr::Eye_Right : vr::Eye_Left;
		auto proj_mat = m_vr_system->GetProjectionMatrix(eye, m_near_clip, m_far_clip);
		static int ti[] = { 0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15 };
		for (int i = 0; i < 16; ++i)
			glm::value_ptr(m_proj_mat)[i] =
			((float*)(proj_mat.m))[ti[i]];
#endif*/
	}
	else
	{
		bool persp;
		getValue("persp", persp);
		double near_clip, far_clip;
		getValue("near clip", near_clip);
		getValue("far clip", far_clip);
		glm::mat4 proj_mat;
		if (persp)
		{
			proj_mat =
				glm::perspective(glm::radians(aov), aspect, near_clip, far_clip);
		}
		else
		{
			proj_mat = glm::ortho(ortho_left, ortho_right, ortho_bottom, ortho_top,
				-far_clip / 100.0, far_clip);
		}
		setValue("proj mat", proj_mat);
	}
}

void Renderer3D::handleCamera()
{
	Vector trans;
	getValue("trans", trans);
	//Vector pos(m_transx, m_transy, m_transz);
	trans.normalize();
	bool free;
	getValue("free", free);
	double distance;
	getValue("distance", distance);
	if (free)
		trans *= 0.1;
	else
		trans *= distance;
	setValue("trans", trans);
	//m_transx = pos.x();
	//m_transy = pos.y();
	//m_transz = pos.z();

	glm::vec3 eye(trans.x(), trans.y(), trans.z());
	glm::vec3 gcenter(0.0);
	Vector up;
	getValue("up", up);
	glm::vec3 gup(up.x(), up.y(), up.z());
	Point center;
	getValue("center", center);

	if (free)
	{
		gcenter = glm::vec3(center.x(), center.y(), center.z());
		eye += gcenter;
	}

	bool use_vr;
	getValue("use vr", use_vr);
	glm::mat4 mv_mat;
	if (use_vr)
	{
		long vr_eye_idx;
		double vr_eye_offset;
		getValue("vr eye idx", vr_eye_idx);
		getValue("vr eye offset", vr_eye_offset);
		glm::vec3 offset((vr_eye_idx ? 1.0 : -1.0) * vr_eye_offset / 2.0, 0.0, 0.0);
		mv_mat = glm::lookAt(
			eye + offset,
			gcenter + offset,
			gup);
	}
	else
	{
		mv_mat = glm::lookAt(eye, gcenter, gup);
	}

	bool lock_cam_object;
	getValue("lock cam object", lock_cam_object);
	if (lock_cam_object)
	{
		Point lock_center;
		getValue("lock center", lock_center);
		//rotate first
		glm::vec3 v1(0, 0, -1);//initial cam direction
		glm::mat4 mv_mat = getDrawMat();
		glm::vec4 lock_ctr(lock_center.x(), lock_center.y(), lock_center.z(), 1);
		lock_ctr = mv_mat * lock_ctr;
		glm::vec3 v2(lock_ctr);
		v2 = glm::normalize(v2);
		float c = glm::dot(v1, v2);
		if (std::abs(std::abs(c) - 1) < Epslf())
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
		mv_mat = rot4 * glm::lookAt(eye, gcenter, gup);
	}

	setValue("mv mat", mv_mat);
}

glm::mat4 Renderer3D::getObjectMat()
{
	glm::mat4 obj_mat;
	getValue("mv mat", obj_mat);
	//translate object
	Vector obj_trans;
	getValue("obj trans", obj_trans);
	obj_mat = glm::translate(obj_mat, glm::vec3(obj_trans.x(), obj_trans.y(), obj_trans.z()));
	//rotate object
	Vector obj_rot;
	getValue("obj rot", obj_rot);
	obj_mat = glm::rotate(obj_mat, float(glm::radians(obj_rot.y())), glm::vec3(0.0, 1.0, 0.0));
	obj_mat = glm::rotate(obj_mat, float(glm::radians(obj_rot.z())), glm::vec3(0.0, 0.0, 1.0));
	obj_mat = glm::rotate(obj_mat, float(glm::radians(obj_rot.x())), glm::vec3(1.0, 0.0, 0.0));
	//center object
	Point obj_center;
	getValue("obj center", obj_center);
	obj_mat = glm::translate(obj_mat, glm::vec3(-obj_center.x(), -obj_center.y(), -obj_center.z()));
	return obj_mat;
}
glm::mat4 Renderer3D::getDrawMat()
{
	glm::mat4 drw_mat;
	getValue("mv mat", drw_mat);
	//translate object
	Vector obj_trans;
	getValue("obj trans", obj_trans);
	drw_mat = glm::translate(drw_mat, glm::vec3(obj_trans.x(), obj_trans.y(), obj_trans.z()));
	//rotate object
	Vector obj_rot;
	getValue("obj rot", obj_rot);
	drw_mat = glm::rotate(drw_mat, float(glm::radians(obj_rot.x())), glm::vec3(1.0, 0.0, 0.0));
	drw_mat = glm::rotate(drw_mat, float(glm::radians(obj_rot.y())), glm::vec3(0.0, 1.0, 0.0));
	drw_mat = glm::rotate(drw_mat, float(glm::radians(obj_rot.z())), glm::vec3(0.0, 0.0, 1.0));
	//center object
	Point obj_center;
	getValue("obj center", obj_center);
	drw_mat = glm::translate(drw_mat, glm::vec3(-obj_center.x(), -obj_center.y(), -obj_center.z()));
	return drw_mat;
}
glm::mat4 Renderer3D::getInvtMat()
{
	glm::mat4 inv_mat;
	getValue("mv mat", inv_mat);
	//translate object
	Vector obj_trans;
	getValue("obj trans", obj_trans);
	inv_mat = glm::translate(inv_mat, glm::vec3(obj_trans.x(), obj_trans.y(), obj_trans.z()));
	//rotate object
	Vector obj_rot;
	getValue("obj rot", obj_rot);
	inv_mat = glm::rotate(inv_mat, float(glm::radians(obj_rot.z())), glm::vec3(0.0, 0.0, 1.0));
	inv_mat = glm::rotate(inv_mat, float(glm::radians(obj_rot.y())), glm::vec3(0.0, 1.0, 0.0));
	inv_mat = glm::rotate(inv_mat, float(glm::radians(obj_rot.x())), glm::vec3(1.0, 0.0, 0.0));
	//center object
	Point obj_center;
	getValue("obj center", obj_center);
	inv_mat = glm::translate(inv_mat, glm::vec3(-obj_center.x(), -obj_center.y(), -obj_center.z()));
	return inv_mat;
}
