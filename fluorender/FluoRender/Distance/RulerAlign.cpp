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

using namespace FL;

void RulerAlign::AlignRuler(int axis_type)
{
	if (!m_view)
		return;
	if (m_ruler_list.empty())
		return;
	Ruler* ruler = m_ruler_list[0];
	if (!ruler || ruler->GetNumPoint() < 2)
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
	}
	//ruler vector
	FLIVR::Vector rv = ruler->GetLastPoint()->GetPoint() -
		ruler->GetPoint(0)->GetPoint();
	rv.normalize();
	m_axis = rv;
	FLIVR::Vector rotv = Cross(m_axis, axis);
	rotv.normalize();
	double ang = Dot(m_axis, axis);
	ang = r2d(std::acos(ang));
	FLIVR::Quaternion q(ang, rotv);
	double qx, qy, qz;
	q.ToEuler(qx, qy, qz);
	m_view->SetRotations(qx, -qy, -qz);
	m_view->RefreshGL(50);
}

void RulerAlign::Rotate(double val)
{
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
	}

	FLIVR::Vector rotv = Cross(m_axis, axis);
	rotv.normalize();
	double ang = Dot(m_axis, axis);
	ang = r2d(std::acos(ang));
	FLIVR::Quaternion q(ang, rotv);
	//rotate
	FLIVR::Quaternion rotq(val, m_axis);
	FLIVR::Quaternion q2 = q * rotq;
	double qx, qy, qz;
	q2.ToEuler(qx, qy, qz);
	m_view->SetRotations(qx, -qy, -qz);
	m_view->RefreshGL(50);
}