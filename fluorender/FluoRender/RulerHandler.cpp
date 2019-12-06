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

#include "RulerHandler.h"
#include "VRenderGLView.h"
#include <glm/gtc/type_ptr.hpp>

RulerHandler::RulerHandler() :
	m_view(0),
	m_ruler(0),
	m_ruler_list(0),
	m_point(0),
	m_pindex(-1)
{

}

RulerHandler::~RulerHandler()
{

}

bool RulerHandler::FindEditingRuler(double mx, double my)
{
	if (!m_view || !m_ruler_list)
		return false;

	//get view size
	wxSize view_size = m_view->GetGLSize();
	int nx = view_size.x;
	int ny = view_size.y;
	if (nx <= 0 || ny <= 0)
		return false;

	//get aspect, norm xy
	double x, y;
	x = mx * 2.0 / double(nx) - 1.0;
	y = 1.0 - my * 2.0 / double(ny);
	double aspect = (double)nx / (double)ny;

	//get persp
	bool persp = m_view->GetPersp();

	//get transform
	glm::mat4 mv_temp = m_view->GetObjectMat();
	glm::mat4 prj_temp = m_view->GetProjection();
	Transform mv;
	Transform prj;
	mv.set(glm::value_ptr(mv_temp));
	prj.set(glm::value_ptr(prj_temp));

	Point *point = 0;
	int i, j;
	Point ptemp;
	for (i = 0; i < (int)m_ruler_list->size(); i++)
	{
		Ruler* ruler = (*m_ruler_list)[i];
		if (!ruler) continue;
		if (!ruler->GetDisp()) continue;

		for (j = 0; j < ruler->GetNumPoint(); j++)
		{
			point = ruler->GetPoint(j);
			if (!point) continue;
			ptemp = *point;
			ptemp = mv.transform(ptemp);
			ptemp = prj.transform(ptemp);
			if ((persp && (ptemp.z() <= 0.0 || ptemp.z() >= 1.0)) ||
				(!persp && (ptemp.z() >= 0.0 || ptemp.z() <= -1.0)))
				continue;
			if (x<ptemp.x() + 0.01 / aspect &&
				x>ptemp.x() - 0.01 / aspect &&
				y<ptemp.y() + 0.01 &&
				y>ptemp.y() - 0.01)
			{
				m_ruler = ruler;
				m_point = point;
				m_pindex = j;
				return true;
			}
		}
	}

	return false;
}

Point* RulerHandler::GetEllipsePoint(int index)
{
	if (!m_ruler ||
		m_ruler->GetRulerType() != 5 ||
		m_ruler->GetNumPoint() != 4)
		return 0;

	switch (m_pindex)
	{
	case 0:
		return m_ruler->GetPoint(index);
	case 1:
		{
			int imap[4] = {1, 0, 3, 2};
			return m_ruler->GetPoint(imap[index]);
		}
	case 2:
		{
			int imap[4] = { 2, 3, 1, 0 };
			return m_ruler->GetPoint(imap[index]);
		}
	case 3:
		{
			int imap[4] = { 3, 2, 0, 1 };
			return m_ruler->GetPoint(imap[index]);
		}
	}

	return 0;
}