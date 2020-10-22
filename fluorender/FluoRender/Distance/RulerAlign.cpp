/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2019 Scientific Computing and Imaging Institute,
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

#include <Distance/RulerAlign.h>
#include <FLIVR/Quaternion.h>
#include <FLIVR/Utils.h>
#include <VRenderGLView.h>
#include <Distance/Pca.h>

using namespace FL;

void RulerAlign::AlignRuler(int axis_type)
{
	if (!m_view)
		return;
	if (m_point_list.size() < 2)
		return;
	m_axis_type = axis_type;
	FLIVR::Vector axis;
	switch (m_axis_type)
	{
	case 0:
		axis = FLIVR::Vector(1.0, 0.0, 0.0);
		break;
	case 1:
		axis = FLIVR::Vector(0.0, 1.0, 0.0);
		break;
	case 2:
		axis = FLIVR::Vector(0.0, 0.0, 1.0);
		break;
	case 3:
		axis = FLIVR::Vector(-1.0, 0.0, 0.0);
		break;
	case 4:
		axis = FLIVR::Vector(0.0, -1.0, 0.0);
		break;
	case 5:
		axis = FLIVR::Vector(0.0, 0.0, -1.0);
		break;
	}
	//ruler vector
	FLIVR::Vector rv = m_point_list.back() -
		m_point_list.front();
	rv.normalize();
	m_axis = rv;
	FLIVR::Vector rotv = Cross(m_axis, axis);
	rotv.normalize();
	double ang = Dot(m_axis, axis);
	ang = r2d(std::acos(ang));
	FLIVR::Quaternion q(ang, rotv);
	double qx, qy, qz;
	m_view->ResetZeroRotations(qx, qy, qz);
	q.ToEuler(qx, qy, qz);
	m_view->SetRotations(qx, -qy, -qz);
	m_view->RefreshGL(50);
}

void RulerAlign::AlignPca(int axis_type, bool cov)
{
	Pca solver;
	if (cov)
		solver.SetPoints(m_point_list);
	else
		solver.SetCovMat(m_cov);
	solver.Compute();

	FLIVR::Vector source0 = solver.GetAxis(0);
	FLIVR::Vector source1 = solver.GetAxis(1);
	FLIVR::Vector source2 = solver.GetAxis(2);
	//store
	m_axis = source0;
	m_axis_x = source0;
	m_axis_y = source1;
	m_axis_z = source2;

	m_axis_type = axis_type;
	FLIVR::Vector target0;
	FLIVR::Vector target1;
	switch (m_axis_type)
	{
	case 0://xyz
		target0 = FLIVR::Vector(1, 0, 0);
		target1 = FLIVR::Vector(0, 1, 0);
		break;
	case 1://yxz
		target0 = FLIVR::Vector(0, 1, 0);
		target1 = FLIVR::Vector(1, 0, 0);
		break;
	case 2://zxy
		target0 = FLIVR::Vector(0, 0, 1);
		target1 = FLIVR::Vector(1, 0, 0);
		break;
	case 3://xzy
		target0 = FLIVR::Vector(-1, 0, 0);
		target1 = FLIVR::Vector(0, 0, 1);
		break;
	case 4://yzx
		target0 = FLIVR::Vector(0, -1, 0);
		target1 = FLIVR::Vector(0, 0, 1);
		break;
	case 5://zyx
		target0 = FLIVR::Vector(0, 0, -1);
		target1 = FLIVR::Vector(0, 1, 0);
		break;
	}

	FLIVR::Vector rotv = Cross(source0, target0);
	rotv.normalize();
	//angle between source0 and target0
	double ang = Dot(source0, target0);
	ang = r2d(std::acos(ang));
	FLIVR::Quaternion q(ang, rotv);
	q.Normalize();
	//rotate source1
	FLIVR::Quaternion s1(source1.x(), source1.y(), source1.z(), 0.0);
	s1 = (q) * s1 * (-q);
	FLIVR::Vector s1v(s1.x, s1.y, s1.z);
	s1v.normalize();
	//angle between s1v and target1
	ang = Dot(s1v, target1);
	ang = r2d(std::acos(ang));
	//decide direction
	FLIVR::Vector dir = Cross(s1v, target1);
	double t0_dir = Dot(target0, dir);
	if (t0_dir < 0.0)
		ang = -ang;
	//rotate
	FLIVR::Quaternion rotq(ang, source0);
	rotq.Normalize();
	FLIVR::Quaternion q2 = q * rotq;
	double qx, qy, qz;
	m_view->ResetZeroRotations(qx, qy, qz);
	q2.ToEuler(qx, qy, qz);
	m_view->SetRotations(qx, -qy, -qz);
	m_view->RefreshGL(50);
}
