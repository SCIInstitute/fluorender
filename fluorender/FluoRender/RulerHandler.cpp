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
#include <wx/fileconf.h>

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

	pPoint point;
	int i, j;
	Point ptemp;
	for (i = 0; i < (int)m_ruler_list->size(); i++)
	{
		Ruler* ruler = (*m_ruler_list)[i];
		if (!ruler) continue;
		if (!ruler->GetDisp()) continue;

		for (j = 0; j < ruler->GetNumPoint(); j++)
		{
			point = ruler->GetPPoint(j);
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

void RulerHandler::FinishRuler()
{
	if (!m_ruler)
		return;
	if (m_ruler->GetRulerType() == 1)
		m_ruler->SetFinished();
}

bool RulerHandler::GetRulerFinished()
{
	if (m_ruler)
		return m_ruler->GetFinished();
	else
		return true;
}

void RulerHandler::AddRulerPoint(int mx, int my)
{
	if (!m_view)
		return;

	if (m_type == 1)
	{
		if (FindEditingRuler(mx, my))
		{
			if (m_ruler &&
				m_ruler->GetDisp() &&
				!m_ruler->GetFinished())
			{
				m_ruler->AddBranch(m_point);
			}
			return;
		}
	}
	if (m_type == 3)
	{
		Point p1, p2;
		Ruler* ruler = new Ruler();
		ruler->SetRulerType(m_type);
		m_view->GetPointVolumeBox2(p1, p2, mx, my);
		ruler->AddPoint(p1);
		ruler->AddPoint(p2);
		ruler->SetTimeDep(m_view->m_ruler_time_dep);
		ruler->SetTime(m_view->m_tseq_cur_num);
		m_ruler_list->push_back(ruler);
		//store brush size in ruler
		if (m_view->GetBrushSizeData())
			ruler->SetBrushSize(m_view->GetBrushSize1());
		else
			ruler->SetBrushSize(m_view->GetBrushSize1()
				/ m_view->Get121ScaleFactor());
	}
	else
	{
		Point p, ip;
		if (m_view->m_point_volume_mode)
		{
			double t = m_view->GetPointVolume(p, ip, mx, my, m_view->m_cur_vol,
				m_view->m_point_volume_mode, m_view->m_ruler_use_transf);
			if (t <= 0.0)
			{
				t = m_view->GetPointPlane(p, mx, my);
				if (t <= 0.0)
					return;
			}
		}
		else
		{
			double t = m_view->GetPointPlane(p, mx, my);
			if (t <= 0.0)
				return;
		}

		bool new_ruler = true;
		if (m_ruler &&
			m_ruler->GetDisp() &&
			!m_ruler->GetFinished())
		{
			m_ruler->AddPoint(p);
			new_ruler = false;
			if (m_type == 5)
			{
				//finish
				glm::mat4 mv_temp = m_view->GetDrawMat();
				glm::vec4 axis(0, 0, -1, 0);
				axis = glm::transpose(mv_temp) * axis;
				m_ruler->FinishEllipse(Vector(axis[0], axis[1], axis[2]));
			}
		}
		if (new_ruler)
		{
			m_ruler = new Ruler();
			m_ruler->SetRulerType(m_type);
			m_ruler->AddPoint(p);
			m_ruler->SetTimeDep(m_view->m_ruler_time_dep);
			m_ruler->SetTime(m_view->m_tseq_cur_num);
			m_ruler_list->push_back(m_ruler);
		}
	}
}

void RulerHandler::AddPaintRulerPoint()
{
	if (!m_view)
		return;

	VolumeSelector* selector = m_view->GetVolumeSelector();
	if (selector->ProcessSel(0.01))
	{
		wxString str;
		Point center;
		double size;
		selector->GetCenter(center);
		selector->GetSize(size);

		bool new_ruler = true;
		if (m_ruler &&
			m_ruler->GetDisp() &&
			!m_ruler->GetFinished())
		{
			m_ruler->AddPoint(center);
			str = wxString::Format("\tv%d", m_ruler->GetNumPoint() - 1);
			m_ruler->AddInfoNames(str);
			str = wxString::Format("\t%.0f", size);
			m_ruler->AddInfoValues(str);
			new_ruler = false;
		}
		if (new_ruler)
		{
			m_ruler = new Ruler();
			m_ruler->SetRulerType(m_type);
			m_ruler->AddPoint(center);
			m_ruler->SetTimeDep(m_view->m_ruler_time_dep);
			m_ruler->SetTime(m_view->m_tseq_cur_num);
			str = "v0";
			m_ruler->AddInfoNames(str);
			str = wxString::Format("%.0f", size);
			m_ruler->AddInfoValues(str);
			m_ruler_list->push_back(m_ruler);
		}
	}
}

void RulerHandler::Save(wxFileConfig &fconfig, int vi)
{
	if (m_ruler_list && m_ruler_list->size())
	{
		fconfig.Write("num", static_cast<unsigned int>(m_ruler_list->size()));
		for (size_t ri = 0; ri < m_ruler_list->size(); ++ri)
		{
			Ruler* ruler = (*m_ruler_list)[ri];
			if (!ruler) continue;
			fconfig.SetPath(wxString::Format("/views/%d/rulers/%d", vi, (int)ri));
			fconfig.Write("name", ruler->GetName());
			fconfig.Write("type", ruler->GetRulerType());
			fconfig.Write("display", ruler->GetDisp());
			fconfig.Write("transient", ruler->GetTimeDep());
			fconfig.Write("time", ruler->GetTime());
			fconfig.Write("info_names", ruler->GetInfoNames());
			fconfig.Write("info_values", ruler->GetInfoValues());
			fconfig.Write("use_color", ruler->GetUseColor());
			fconfig.Write("color", wxString::Format("%f %f %f",
				ruler->GetColor().r(), ruler->GetColor().g(), ruler->GetColor().b()));
			fconfig.Write("num", ruler->GetNumBranch());
			for (size_t rbi = 0; rbi < ruler->GetNumBranch(); ++rbi)
			{
				fconfig.SetPath(wxString::Format(
					"/views/%d/rulers/%d/branches/%d", vi, (int)ri, (int)rbi));
				fconfig.Write("num", ruler->GetNumBranchPoint(rbi));

				for (size_t rpi = 0; rpi < ruler->GetNumBranchPoint(rbi); ++rpi)
				{
					Point* rp = ruler->GetPoint(rbi, rpi);
					if (!rp) continue;
					fconfig.Write(wxString::Format("point%d", (int)rpi),
						wxString::Format("%f %f %f", rp->x(), rp->y(), rp->z()));
				}
			}
		}
	}
}

void RulerHandler::Read(wxFileConfig &fconfig)
{

}
