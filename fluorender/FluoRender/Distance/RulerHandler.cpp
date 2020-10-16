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
#include <FLIVR/Texture.h>
#include <DataManager.h>
#include <Components/CompAnalyzer.h>
#include <Selection/VolumePoint.h>
#include <Selection/VolumeSelector.h>
#include <Distance/Cov.h>
#include <Calculate/Count.h>
#include <glm/gtc/type_ptr.hpp>
#include <Nrrd/nrrd.h>
#include <wx/fileconf.h>

using namespace FL;

RulerHandler::RulerHandler() :
	m_group(0),
	m_view(0),
	m_vd(0),
	m_ca(0),
	m_ruler(0),
	m_ruler_list(0),
	m_point(0),
	m_pindex(-1),
	m_mouse(FLIVR::Point(-1))
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

	pRulerPoint point;
	int i, j, k;
	Point ptemp;
	for (i = 0; i < (int)m_ruler_list->size(); i++)
	{
		Ruler* ruler = (*m_ruler_list)[i];
		if (!ruler) continue;
		if (!ruler->GetDisp()) continue;

		for (j = 0; j < ruler->GetNumBranch(); j++)
		for (k = 0; k < ruler->GetNumBranchPoint(j); ++k)
		{
			point = ruler->GetPPoint(j, k);
			if (!point) continue;
			ptemp = point->GetPoint();
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
				m_pindex = k;
				return true;
			}
		}
	}

	return false;
}

void RulerHandler::DeletePoint()
{
	if (!m_ruler || !m_point)
		return;

	m_ruler->DeletePoint(m_point);
	m_point = nullptr;
	m_pindex = -1;
	if (!m_ruler->GetNumPoint())
	{
		//delete ruler
		for (int i = 0; i < (int)m_ruler_list->size(); i++)
		{
			Ruler* ruler = (*m_ruler_list)[i];
			if (ruler == m_ruler)
			{
				m_ruler_list->erase(m_ruler_list->begin() + i);
				m_ruler = 0;
				return;
			}
		}
	}
}

RulerPoint* RulerHandler::GetEllipsePoint(int index)
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
	m_mouse = FLIVR::Point(-1);
}

bool RulerHandler::GetRulerFinished()
{
	if (m_ruler)
		return m_ruler->GetFinished();
	else
		return true;
}

void RulerHandler::AddRulerPoint(FLIVR::Point &p)
{
	if (m_ruler &&
		m_ruler->GetDisp() &&
		!m_ruler->GetFinished())
	{
		m_ruler->AddPoint(p);
	}
	else
	{
		m_ruler = new Ruler();
		m_ruler->Group(m_group);
		m_ruler->SetRulerType(m_type);
		m_ruler->AddPoint(p);
		m_ruler->SetTimeDep(m_view->m_ruler_time_dep);
		m_ruler->SetTime(m_view->m_tseq_cur_num);
		m_ruler_list->push_back(m_ruler);
	}
}

void RulerHandler::AddRulerPointAfterId(FLIVR::Point &p, unsigned int id,
	std::set<unsigned int> &cid, std::set<unsigned int> &bid)
{
	if (m_ruler &&
		m_ruler->GetDisp() &&
		m_ruler->GetRulerType() == 1 &&
		!m_ruler->GetFinished())
		m_ruler->AddPointAfterId(p, id, cid, bid);
	else
	{
		m_ruler = new Ruler();
		m_ruler->Group(m_group);
		m_ruler->SetRulerType(m_type);
		m_ruler->AddPointAfterId(p, id, cid, bid);
		m_ruler->SetTimeDep(m_view->m_ruler_time_dep);
		m_ruler->SetTime(m_view->m_tseq_cur_num);
		m_ruler_list->push_back(m_ruler);
	}
}

bool RulerHandler::GetMouseDist(int mx, int my, double dist)
{
	if (m_mouse.x() < 0 || m_mouse.y() < 0)
		return true;
	FLIVR::Point p(mx, my, 0);
	return (p - m_mouse).length() > dist;
}

void RulerHandler::AddRulerPoint(int mx, int my, bool branch)
{
	if (!m_view)
		return;

	int point_volume_mode = m_view->m_point_volume_mode;

	if (m_type == 1 && branch)
	{
		if (FindEditingRuler(mx, my))
		{
			if (m_ruler &&
				m_ruler->GetDisp() &&
				!m_ruler->GetFinished())
			{
				m_ruler->AddBranch(m_point);
				m_mouse = FLIVR::Point(mx, my, 0);
				return;
			}
		}
	}
	if (m_type == 3)
	{
		Point p1, p2;
		Ruler* ruler = new Ruler();
		ruler->Group(m_group);
		ruler->SetRulerType(m_type);
		m_vp.SetVolumeData(m_view->m_cur_vol);
		m_vp.GetPointVolumeBox2(mx, my, p1, p2);
		ruler->AddPoint(p1);
		ruler->AddPoint(p2);
		ruler->SetTimeDep(m_view->m_ruler_time_dep);
		ruler->SetTime(m_view->m_tseq_cur_num);
		m_ruler_list->push_back(ruler);
		//store brush size in ruler
		FL::VolumeSelector* selector = m_view->GetVolumeSelector();
		if (selector)
		{
			if (selector->GetBrushSizeData())
				ruler->SetBrushSize(selector->GetBrushSize1());
			else
				ruler->SetBrushSize(selector->GetBrushSize1()
					/ m_view->Get121ScaleFactor());
		}
	}
	else
	{
		Point p, ip, planep;
		Point* pplanep = 0;
		if (m_type == 1)
		{
			if (m_ruler)
			{
				RulerPoint* pp = m_ruler->GetLastPoint();
				if (pp)
				{
					planep = pp->GetPoint();
					pplanep = &planep;
				}
			}
			else
				point_volume_mode = point_volume_mode ? point_volume_mode : 2;
		}
		if (point_volume_mode)
		{
			m_vp.SetVolumeData(m_view->m_cur_vol);
			double t = m_vp.GetPointVolume(mx, my,
				point_volume_mode, m_view->m_ruler_use_transf, 0.5,
				p, ip);
			if (t <= 0.0)
			{
				t = m_vp.GetPointPlane(mx, my, pplanep, true, p);
				if (t <= 0.0)
					return;
			}
		}
		else
		{
			double t = m_vp.GetPointPlane(mx, my, pplanep, true, p);
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
			m_ruler->Group(m_group);
			m_ruler->SetRulerType(m_type);
			m_ruler->AddPoint(p);
			m_ruler->SetTimeDep(m_view->m_ruler_time_dep);
			m_ruler->SetTime(m_view->m_tseq_cur_num);
			m_ruler_list->push_back(m_ruler);
		}
	}

	m_mouse = FLIVR::Point(mx, my, 0);
}

void RulerHandler::AddPaintRulerPoint()
{
	if (!m_view)
		return;
	FL::VolumeSelector* selector = m_view->GetVolumeSelector();
	if (!selector)
		return;
	VolumeData* vd = selector->GetVolume();
	if (!vd)
		return;

	FL::Cov cover(vd);
	cover.Compute(1);
	FL::CountVoxels counter(vd);
	counter.Count();

	Point center = cover.GetCenter();
	double size = counter.GetSum();

	wxString str;
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
		m_ruler->Group(m_group);
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

bool RulerHandler::MoveRuler(int mx, int my)
{
	if (!m_point || !m_view || !m_ruler)
		return false;

	Point point, ip, tmp;
	if (m_view->m_point_volume_mode)
	{
		m_vp.SetVolumeData(m_view->m_cur_vol);
		double t = m_vp.GetPointVolume(mx, my,
			m_view->m_point_volume_mode, m_view->m_ruler_use_transf, 0.5,
			point, ip);
		if (t <= 0.0)
		{
			tmp = m_point->GetPoint();
			t = m_vp.GetPointPlane(mx, my, &tmp, true, point);
		}
		if (t <= 0.0)
			return false;
	}
	else
	{
		tmp = m_point->GetPoint();
		double t = m_vp.GetPointPlane(mx, my, &tmp, true, point);
		if (t <= 0.0)
			return false;
	}

	Point p0 = m_point->GetPoint();
	Vector displace = point - p0;
	for (int i = 0; i < m_ruler->GetNumPoint(); ++i)
	{
		m_ruler->GetPoint(i)->DisplacePoint(displace);
	}

	return true;
}

bool RulerHandler::EditPoint(int mx, int my, bool alt)
{
	if (!m_point || !m_view || !m_ruler)
		return false;

	Point point, ip, tmp;
	if (m_view->m_point_volume_mode)
	{
		m_vp.SetVolumeData(m_view->m_cur_vol);
		double t = m_vp.GetPointVolume(mx, my,
			m_view->m_point_volume_mode, m_view->m_ruler_use_transf, 0.5,
			point, ip);
		if (t <= 0.0)
		{
			tmp = m_point->GetPoint();
			t = m_vp.GetPointPlane(mx, my, &tmp, true, point);
		}
		if (t <= 0.0)
			return false;
	}
	else
	{
		tmp = m_point->GetPoint();
		double t = m_vp.GetPointPlane(mx, my, &tmp, true, point);
		if (t <= 0.0)
			return false;
	}

	m_point->SetPoint(point);

	FL::RulerPoint *p0 = m_point.get();
	FL::RulerPoint *p1 = GetEllipsePoint(1);
	FL::RulerPoint *p2 = GetEllipsePoint(2);
	FL::RulerPoint *p3 = GetEllipsePoint(3);
	if (!p1 || !p2 || !p3)
		return true;

	if (alt)
	{
		Point c = Point((p2->GetPoint() + p3->GetPoint()) / 2.0);
		Vector v0 = p0->GetPoint() - c;
		Vector v2 = p2->GetPoint() - c;
		Vector axis = Cross(v2, v0);
		axis = Cross(axis, v2);
		axis.normalize();
		tmp = Point(c + axis * v0.length());
		p0->SetPoint(tmp);
		tmp = c + (c - p0->GetPoint());
		p1->SetPoint(tmp);
	}
	else
	{
		Point c = Point((p0->GetPoint() +
			p1->GetPoint() + p2->GetPoint() +
			p3->GetPoint()) / 4.0);
		Vector v0 = p0->GetPoint() - c;
		Vector v2 = p2->GetPoint() - c;
		Vector axis = Cross(v2, v0);
		Vector a2 = Cross(v0, axis);
		a2.normalize();
		tmp = Point(c + a2 * v2.length());
		p2->SetPoint(tmp);
		tmp = Point(c - a2 * v2.length());
		p3->SetPoint(tmp);
		tmp = c + (c - p0->GetPoint());
		p1->SetPoint(tmp);
	}

	return true;
}

void RulerHandler::Prune(int idx, int len)
{
	if (!m_ruler_list)
		return;
	if (idx < 0 || idx >= m_ruler_list->size())
		return;
	if (len <= 0)
		return;
	Ruler* ruler = (*m_ruler_list)[idx];
	if (!ruler)
		return;

	ruler->Prune(len);
}

void RulerHandler::DeleteSelection(std::vector<int> &sel)
{
	if (!m_ruler_list)
		return;

	for (auto it = m_ruler_list->rbegin();
		it != m_ruler_list->rend();)
	{
		auto it2 = std::next(it).base();
		int idx = it2 - m_ruler_list->begin();
		if (std::find(sel.begin(),
			sel.end(), idx) != sel.end())
		{
			if (*it2)
				delete *it2;
			it2 = m_ruler_list->erase(it2);
			it = std::reverse_iterator<FL::RulerList::iterator>(it2);
		}
		else
			++it;
	}

	m_ruler = 0;
	m_point = nullptr;
	m_pindex = -1;
}

void RulerHandler::DeleteAll(bool cur_time)
{
	if (!m_ruler_list)
		return;

	if (cur_time)
	{
		int tseq = m_view->m_tseq_cur_num;
		for (int i = m_ruler_list->size() - 1; i >= 0; i--)
		{
			FL::Ruler* ruler = (*m_ruler_list)[i];
			if (ruler &&
				((ruler->GetTimeDep() &&
					ruler->GetTime() == tseq) ||
					!ruler->GetTimeDep()))
			{
				m_ruler_list->erase(m_ruler_list->begin() + i);
				delete ruler;
			}
		}
	}
	else
	{
		for (int i = m_ruler_list->size() - 1; i >= 0; i--)
		{
			FL::Ruler* ruler = (*m_ruler_list)[i];
			if (ruler)
				delete ruler;
		}
		m_ruler_list->clear();
	}

	m_ruler = 0;
	m_point = nullptr;
	m_pindex = -1;
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
			fconfig.Write("group", ruler->Group());
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
					RulerPoint* rp = ruler->GetPoint(rbi, rpi);
					if (!rp) continue;
					fconfig.Write(wxString::Format("point%d", (int)rpi),
						wxString::Format("%f %f %f %d",
						rp->GetPoint().x(),
						rp->GetPoint().y(),
						rp->GetPoint().z(),
						rp->GetLocked()));
				}
			}
		}
	}
}

void RulerHandler::Read(wxFileConfig &fconfig, int vi)
{
	wxString str;
	int ival;
	bool bval;
	int rbi, rpi;
	float x, y, z;
	int l = 0;
	if (m_ruler_list)
	{
		m_ruler_list->clear();
		int rnum = fconfig.Read("num", 0l);
		for (int ri = 0; ri < rnum; ++ri)
		{
			if (fconfig.Exists(wxString::Format("/views/%d/rulers/%d", vi, ri)))
			{
				fconfig.SetPath(wxString::Format("/views/%d/rulers/%d", vi, ri));
				Ruler* ruler = new Ruler();
				if (fconfig.Read("name", &str))
					ruler->SetName(str);
				if (fconfig.Read("group", &ival))
					ruler->Group(ival);
				if (fconfig.Read("type", &ival))
					ruler->SetRulerType(ival);
				if (fconfig.Read("display", &bval))
					ruler->SetDisp(bval);
				if (fconfig.Read("transient", &bval))
					ruler->SetTimeDep(bval);
				if (fconfig.Read("time", &ival))
					ruler->SetTime(ival);
				if (fconfig.Read("info_names", &str))
					ruler->SetInfoNames(str);
				if (fconfig.Read("info_values", &str))
					ruler->SetInfoValues(str);
				if (fconfig.Read("use_color", &bval))
				{
					if (bval)
					{
						if (fconfig.Read("color", &str))
						{
							float r, g, b;
							if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b))
							{
								FLIVR::Color col(r, g, b);
								ruler->SetColor(col);
							}
						}
					}
				}
				int num = fconfig.Read("num", 0l);
				//num could be points or branch
				for (int ii = 0; ii < num; ++ii)
				{
					//if points
					rbi = rpi = ii;
					if (fconfig.Exists(wxString::Format(
						"/views/%d/rulers/%d/points/%d", vi, ri, rpi)))
					{
						fconfig.SetPath(wxString::Format(
							"/views/%d/rulers/%d/points/%d", vi, ri, rpi));
						if (fconfig.Read("point", &str))
						{
							if (SSCANF(str.c_str(), "%f%f%f", &x, &y, &z))
							{
								Point point(x, y, z);
								ruler->AddPoint(point);
							}
						}
					}
					//if branch
					else if (fconfig.Exists(wxString::Format(
						"/views/%d/rulers/%d/branches/%d", vi, ri, rbi)))
					{
						fconfig.SetPath(wxString::Format(
							"/views/%d/rulers/%d/branches/%d", vi, ri, rbi));
						if (fconfig.Read("num", &ival))
						{
							for (rpi = 0; rpi < ival; ++rpi)
							{
								if (fconfig.Read(wxString::Format("point%d", rpi), &str))
								{
									if (SSCANF(str.c_str(), "%f%f%f%d", &x, &y, &z, &l))
									{
										Point point(x, y, z);
										if (rbi > 0 && rpi == 0)
										{
											pRulerPoint pp = ruler->FindPoint(point);
											pp->SetLocked(l);
											ruler->AddBranch(pp);
										}
										else
										{
											ruler->AddPoint(point);
											pRulerPoint pp = ruler->FindPoint(point);
											pp->SetLocked(l);
										}
									}
								}
							}
						}
					}
				}
				ruler->SetFinished();
				m_ruler_list->push_back(ruler);
			}
		}
	}
}

int RulerHandler::Profile(int index)
{
	if (!m_view || !m_vd || !m_ruler_list)
		return 0;
	if (index < 0 ||
		index >= m_ruler_list->size())
		return 0;

	FL::Ruler* ruler = (*m_ruler_list)[index];
	if (ruler->GetNumPoint() < 1)
		return 0;

	double spcx, spcy, spcz;
	m_vd->GetSpacings(spcx, spcy, spcz);
	int nx, ny, nz;
	m_vd->GetResolution(nx, ny, nz);
	if (spcx <= 0.0 || spcy <= 0.0 || spcz <= 0.0 ||
		nx <= 0 || ny <= 0 || nz <= 0)
		return 0;
	//get data
	m_vd->GetVR()->return_mask();
	FLIVR::Texture* tex = m_vd->GetTexture();
	if (!tex) return 0;
	Nrrd* nrrd_data = tex->get_nrrd(0);
	if (!nrrd_data) return 0;
	void* data = nrrd_data->data;
	if (!data) return 0;
	//mask
	Nrrd* nrrd_mask = tex->get_nrrd(tex->nmask());
	void* mask = 0;
	if (nrrd_mask)
		mask = nrrd_mask->data;
	double scale = m_vd->GetScalarScale();

	if (ruler->GetRulerType() == 3 && mask)
	{
		if (ruler->GetNumPoint() < 1)
			return 0;
		FLIVR::Point p1, p2;
		p1 = ruler->GetPoint(0)->GetPoint();
		p2 = ruler->GetPoint(1)->GetPoint();
		//object space
		p1 = FLIVR::Point(p1.x() / spcx, p1.y() / spcy, p1.z() / spcz);
		p2 = FLIVR::Point(p2.x() / spcx, p2.y() / spcy, p2.z() / spcz);
		FLIVR::Vector dir = p2 - p1;
		double dist = dir.length();
		if (dist < EPS)
			return 0;
		dir.normalize();

		//bin number
		int bins = int(dist / 1 + 0.5);
		if (bins <= 0) return 0;
		double bin_dist = dist / bins;
		std::vector<FL::ProfileBin>* profile = ruler->GetProfile();
		if (!profile) return 0;
		profile->clear();
		profile->reserve(size_t(bins));
		for (unsigned int b = 0; b < bins; ++b)
			profile->push_back(FL::ProfileBin());

		double brush_radius = ruler->GetBrushSize() + 1.0;

		int i, j, k;
		long long vol_index;
		//go through data
		for (i = 0; i < nx; ++i)
		for (j = 0; j < ny; ++j)
		for (k = 0; k < nz; ++k)
		{
			vol_index = (long long)nx*ny*k + nx * j + i;
			unsigned char mask_value = ((unsigned char*)mask)[vol_index];
			if (mask_value)
			{
				//find bin
				FLIVR::Point p(i, j, k);
				FLIVR::Vector pdir = p - p1;
				double proj = Dot(pdir, dir);
				int bin_num = int(proj / bin_dist);
				if (bin_num < 0 || bin_num >= bins)
					continue;
				//make sure it's within the brush radius
				FLIVR::Point p_ruler = p1 + proj * dir;
				if ((p_ruler - p).length() > brush_radius)
					continue;

				double intensity = 0.0;
				if (nrrd_data->type == nrrdTypeUChar)
					intensity = double(((unsigned char*)data)[vol_index]) / 255.0;
				else if (nrrd_data->type == nrrdTypeUShort)
					intensity = double(((unsigned short*)data)[vol_index]) * scale / 65535.0;

				(*profile)[bin_num].m_pixels++;
				(*profile)[bin_num].m_accum += intensity;
			}
		}
	}
	else
	{
		//calculate length in object space
		double total_length = ruler->GetLengthObject(spcx, spcy, spcz);
		int bins = int(total_length);
		std::vector<FL::ProfileBin>* profile = ruler->GetProfile();
		if (!profile) return 0;
		profile->clear();

		//sample data through ruler
		int i, j, k;
		long long vol_index;
		FLIVR::Point p;
		double intensity;
		if (bins == 0)
		{
			//allocate
			profile->reserve(size_t(1));
			profile->push_back(FL::ProfileBin());

			p = ruler->GetPoint(0)->GetPoint();
			//object space
			p = FLIVR::Point(p.x() / spcx, p.y() / spcy, p.z() / spcz);
			intensity = 0.0;
			i = int(p.x() + 0.5);
			j = int(p.y() + 0.5);
			k = int(p.z() + 0.5);
			if (i >= 0 && i <= nx && j >= 0 && j <= ny && k >= 0 && k <= nz)
			{
				if (i == nx) i = nx - 1;
				if (j == ny) j = ny - 1;
				if (k == nz) k = nz - 1;
				vol_index = (long long)nx*ny*k + nx * j + i;
				if (nrrd_data->type == nrrdTypeUChar)
					intensity = double(((unsigned char*)data)[vol_index]) / 255.0;
				else if (nrrd_data->type == nrrdTypeUShort)
					intensity = double(((unsigned short*)data)[vol_index]) * scale / 65535.0;
			}
			(*profile)[0].m_pixels++;
			(*profile)[0].m_accum += intensity;
		}
		else
		{
			//allocate
			profile->reserve(size_t(bins));
			for (unsigned int b = 0; b < bins; ++b)
				profile->push_back(FL::ProfileBin());

			FLIVR::Point p1, p2;
			FLIVR::Vector dir;
			double dist;
			int total_dist = 0;
			for (unsigned int pn = 0; pn < ruler->GetNumPoint() - 1; ++pn)
			{
				p1 = ruler->GetPoint(pn)->GetPoint();
				p2 = ruler->GetPoint(pn + 1)->GetPoint();
				//object space
				p1 = FLIVR::Point(p1.x() / spcx, p1.y() / spcy, p1.z() / spcz);
				p2 = FLIVR::Point(p2.x() / spcx, p2.y() / spcy, p2.z() / spcz);
				dir = p2 - p1;
				dist = dir.length();
				dir.normalize();

				for (unsigned int dn = 0; dn < (unsigned int)(dist + 0.5); ++dn)
				{
					p = p1 + dir * double(dn);
					intensity = 0.0;
					i = int(p.x() + 0.5);
					j = int(p.y() + 0.5);
					k = int(p.z() + 0.5);
					if (i >= 0 && i <= nx && j >= 0 && j <= ny && k >= 0 && k <= nz)
					{
						if (i == nx) i = nx - 1;
						if (j == ny) j = ny - 1;
						if (k == nz) k = nz - 1;
						vol_index = (long long)nx*ny*k + nx * j + i;
						if (nrrd_data->type == nrrdTypeUChar)
							intensity = double(((unsigned char*)data)[vol_index]) / 255.0;
						else if (nrrd_data->type == nrrdTypeUShort)
							intensity = double(((unsigned short*)data)[vol_index]) * scale / 65535.0;
					}
					if (total_dist >= bins) break;
					(*profile)[total_dist].m_pixels++;
					(*profile)[total_dist].m_accum += intensity;
					total_dist++;
				}
			}
			if (total_dist < bins)
				profile->erase(profile->begin() + total_dist, profile->begin() + bins - 1);
		}
	}
	wxString str("Profile of volume ");
	str = str + m_vd->GetName();
	ruler->SetInfoProfile(str);
	return 1;
}

int RulerHandler::Distance(int index, std::string filename)
{
	if (!m_view || !m_ruler_list || !m_ca)
		return 0;
	if (index < 0 ||
		index >= m_ruler_list->size())
		return 0;

	FL::Ruler* ruler = (*m_ruler_list)[index];
	if (ruler->GetNumPoint() < 1)
		return 0;

	Point p = ruler->GetCenter();

	FL::CompList* list = m_ca->GetCompList();
	if (list->empty())
		return 0;

	double sx = list->sx;
	double sy = list->sy;
	double sz = list->sz;
	for (auto it = list->begin();
		it != list->end(); ++it)
	{
		double dist = (p - it->second->GetPos(sx, sy, sz)).length();
		it->second->dist = dist;
	}

	m_ca->OutputCompListFile(filename, 1);

	return 1;
}

