/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2025 Scientific Computing and Imaging Institute,
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

#include <RulerHandler.h>
#include <Global.h>
#include <Ruler.h>
#include <RenderView.h>
#include <CurrentObjects.h>
#include <VolumeData.h>
#include <Texture.h>
#include <Cov.h>
#include <WalkCycle.h>
#include <Count.h>
#include <BackgStat.h>
#include <VolumeRoi.h>
#include <VolumePoint.h>
#include <DistCalculator.h>
#include <MainSettings.h>
#include <CompAnalyzer.h>
#include <VolumeSelector.h>
#include <VolumeRenderer.h>
#include <glm/gtc/type_ptr.hpp>
#include <nrrd.h>
#include <Debug.h>

using namespace flrd;

RulerHandler::RulerHandler() :
	m_group(0),
	m_mag_ruler(0),
	m_mag_branch(0),
	m_mag_branch_point(0),
	m_redist_len(true),
	m_point(0),
	m_pindex(-1),
	m_mouse(fluo::Point(-1)),
	m_fsize(1),
	m_sample_type(1),
	m_step_length(1),
	m_edited(false),
	m_mode(RulerMode::None)
{

}

RulerHandler::~RulerHandler()
{

}

void RulerHandler::NewGroup()
{
	RulerList* list = glbin_current.GetRulerList();
	if (!list)
		return;

	std::vector<unsigned int> groups;
	int num = list->GetGroupNum(groups);
	if (num)
	{
		auto it = std::max_element(groups.begin(), groups.end());
		if (it != groups.end())
			m_group = *it + 1;
	}
}

void RulerHandler::GroupRulers(const std::set<int>& rulers)
{
	RulerList* list = glbin_current.GetRulerList();
	if (!list)
		return;

	bool update_all = rulers.empty();

	size_t c = 0;
	for (auto i : *list)
	{
		if (update_all || rulers.find(static_cast<int>(c)) != rulers.end())
			i->Group(m_group);
		c++;
	}
}

double RulerHandler::GetVolumeBgInt()
{
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return 0;
	return vd->GetBackgroundInt();
}

Ruler* RulerHandler::GetRuler(const std::wstring& name)
{
	RulerList* list = glbin_current.GetRulerList();
	if (!list)
		return 0;
	for (auto ruler : *list)
	{
		if (ruler->GetName() == name)
			return ruler;
	}
	return 0;
}

Ruler* RulerHandler::GetRuler(size_t i)
{
	RulerList* list = glbin_current.GetRulerList();
	if (!list)
		return 0;
	if (i < list->size())
		return (*list)[i];
	return 0;
}

int RulerHandler::GetRulerIndex()
{
	Ruler* ruler = glbin_current.GetRuler();
	RulerList* list = glbin_current.GetRulerList();
	if (!ruler || !list)
		return -1;
	for (int i = 0; i < list->size(); ++i)
		if ((*list)[i] == ruler)
			return i;
	return -1;
}

void RulerHandler::GetRulerList(const std::set<int>& rulers, flrd::RulerList& out_list)
{
	RulerList* list = glbin_current.GetRulerList();
	if (!list)
		return;

	bool update_all = rulers.empty();

	size_t c = 0;
	for (auto i : *list)
	{
		if (update_all || rulers.find(static_cast<int>(c)) != rulers.end())
			out_list.push_back(i);
		c++;
	}
}

void RulerHandler::SetDisplay(bool bval, const std::set<int>& rulers)
{
	RulerList* list = glbin_current.GetRulerList();
	if (!list)
		return;

	for (size_t i = 0; i < list->size(); ++i)
	{
		Ruler* ruler = (*list)[i];
		if (!ruler) continue;
		if (rulers.find(static_cast<int>(i)) != rulers.end())
			ruler->SetDisp(bval);
	}
}

void RulerHandler::ToggleDisplay(const std::set<int>& rulers)
{
	RulerList* list = glbin_current.GetRulerList();
	if (!list)
		return;

	for (size_t i = 0; i < list->size(); ++i)
	{
		Ruler* ruler = (*list)[i];
		if (!ruler) continue;
		if (rulers.find(static_cast<int>(i)) != rulers.end())
			ruler->ToggleDisp();
	}
}

void RulerHandler::ToggleGroupDisp()
{
	RulerList* list = glbin_current.GetRulerList();
	if (!list)
		return;

	bool disp;
	bool first = true;
	for (size_t i = 0; i < list->size(); ++i)
	{
		Ruler* ruler = (*list)[i];
		if (!ruler) continue;
		if (ruler->Group() == m_group)
		{
			if (first)
			{
				first = false;
				disp = !ruler->GetDisp();
			}
			ruler->SetDisp(disp);
		}
	}
}

bool RulerHandler::FindEditingRuler(double mx, double my)
{
	auto view = glbin_current.render_view.lock();
	RulerList* list = glbin_current.GetRulerList();
	if (!view || !list)
		return false;
	size_t rwt = view->m_tseq_cur_num;

	//get view size
	Size2D view_size = view->GetCanvasSize();
	int nx = view_size.w();
	int ny = view_size.h();
	if (nx <= 0 || ny <= 0)
		return false;

	//get aspect, norm xy
	double x, y;
	x = mx * 2.0 / double(nx) - 1.0;
	y = 1.0 - my * 2.0 / double(ny);
	double aspect = (double)nx / (double)ny;

	//get persp
	bool persp = view->GetPersp();

	//get transform
	glm::mat4 mv_temp = view->GetObjectMat();
	glm::mat4 prj_temp = view->GetProjection();
	fluo::Transform mv;
	fluo::Transform prj;
	mv.set(glm::value_ptr(mv_temp));
	prj.set(glm::value_ptr(prj_temp));

	pRulerPoint point;
	int i, j, k;
	fluo::Point ptemp;
	for (i = 0; i < (int)list->size(); i++)
	{
		Ruler* ruler = (*list)[i];
		if (!ruler) continue;
		if (!ruler->GetDisp()) continue;
		int interp = ruler->GetInterp();

		for (j = 0; j < ruler->GetNumBranch(); j++)
		for (k = 0; k < ruler->GetNumBranchPoint(j); ++k)
		{
			point = ruler->GetPRulerPoint(j, k);
			if (!point) continue;
			ptemp = point->GetPoint(rwt, interp);
			ptemp = mv.transform(ptemp);
			ptemp = prj.transform(ptemp);
			if ((persp && (ptemp.z() <= 0.0 || ptemp.z() >= 1.0)) ||
				(!persp && (ptemp.z() >= 0.0 || ptemp.z() <= -1.0)))
				continue;
			if (x<ptemp.x() + 0.02 / aspect &&
				x>ptemp.x() - 0.02 / aspect &&
				y<ptemp.y() + 0.02 &&
				y>ptemp.y() - 0.02)
			{
				view->SetCurRuler(ruler);
				m_point = point;
				m_pindex = k;
				return true;
			}
		}
	}

	return false;
}

bool RulerHandler::FindClosestRulerPoint(double mx, double my)
{
	auto view = glbin_current.render_view.lock();
	RulerList* list = glbin_current.GetRulerList();
	if (!view || !list)
		return false;
	size_t rwt = view->m_tseq_cur_num;

	//get view size
	Size2D view_size = view->GetCanvasSize();
	int nx = view_size.w();
	int ny = view_size.h();
	if (nx <= 0 || ny <= 0)
		return false;

	//get aspect, norm xy
	double x, y;
	x = mx * 2.0 / double(nx) - 1.0;
	y = 1.0 - my * 2.0 / double(ny);
	double aspect = (double)nx / (double)ny;

	//get persp
	bool persp = view->GetPersp();

	//get transform
	glm::mat4 mv_temp = view->GetObjectMat();
	glm::mat4 prj_temp = view->GetProjection();
	fluo::Transform mv;
	fluo::Transform prj;
	mv.set(glm::value_ptr(mv_temp));
	prj.set(glm::value_ptr(prj_temp));

	pRulerPoint point, ppmin;
	double dmin = std::numeric_limits<double>::max();
	double dist;
	int i, j, k;
	fluo::Point ptemp;
	for (i = 0; i < (int)list->size(); i++)
	{
		Ruler* ruler = (*list)[i];
		if (!ruler) continue;
		if (!ruler->GetDisp()) continue;
		int interp = ruler->GetInterp();

		for (j = 0; j < ruler->GetNumBranch(); j++)
		for (k = 0; k < ruler->GetNumBranchPoint(j); ++k)
		{
			point = ruler->GetPRulerPoint(j, k);
			if (!point) continue;
			ptemp = point->GetPoint(rwt, interp);
			ptemp = mv.transform(ptemp);
			ptemp = prj.transform(ptemp);
			if ((persp && (ptemp.z() <= 0.0 || ptemp.z() >= 1.0)) ||
				(!persp && (ptemp.z() >= 0.0 || ptemp.z() <= -1.0)))
				continue;
			dist = (x - ptemp.x()) * (x - ptemp.x()) +
				(y - ptemp.y()) * (y - ptemp.y());
			if (dist < dmin)
			{
				ppmin = point;
				dmin = dist;
				view->SetCurRuler(ruler);
				m_pindex = k;
			}
		}
	}

	if (ppmin)
	{
		m_point = ppmin;
		return true;
	}
	return false;
}

bool RulerHandler::FindClosestRulerBranch(double mx, double my)
{
	auto view = glbin_current.render_view.lock();
	RulerList* list = glbin_current.GetRulerList();
	if (!view || !list)
		return false;
	size_t rwt = view->m_tseq_cur_num;

	//get view size
	Size2D view_size = view->GetCanvasSize();
	int nx = view_size.w();
	int ny = view_size.h();
	if (nx <= 0 || ny <= 0)
		return false;

	//get aspect, norm xy
	double x, y;
	x = mx * 2.0 / double(nx) - 1.0;
	y = 1.0 - my * 2.0 / double(ny);
	double aspect = (double)nx / (double)ny;

	//get persp
	bool persp = view->GetPersp();

	//get transform
	glm::mat4 mv_temp = view->GetObjectMat();
	glm::mat4 prj_temp = view->GetProjection();
	fluo::Transform mv;
	fluo::Transform prj;
	mv.set(glm::value_ptr(mv_temp));
	prj.set(glm::value_ptr(prj_temp));

	pRulerPoint point, ppmin;
	Ruler* rulermin = 0;
	double dmin = std::numeric_limits<double>::max();
	double dist;
	size_t i, j, rj;
	fluo::Point ptemp;
	for (i = 0; i < list->size(); i++)
	{
		Ruler* ruler = (*list)[i];
		if (!ruler) continue;
		if (!ruler->GetDisp()) continue;
		int interp = ruler->GetInterp();

		for (j = 0; j < ruler->GetNumBranch(); j++)
		{
			//if (ruler->GetNumBranchPoint(j) < 2)
			//	continue;
			point = ruler->GetPRulerPoint(static_cast<int>(j), 0);
			if (!point) continue;
			ptemp = point->GetPoint(rwt, interp);
			ptemp = mv.transform(ptemp);
			ptemp = prj.transform(ptemp);
			if ((persp && (ptemp.z() <= 0.0 || ptemp.z() >= 1.0)) ||
				(!persp && (ptemp.z() >= 0.0 || ptemp.z() <= -1.0)))
				continue;
			dist = (x - ptemp.x()) * (x - ptemp.x()) +
				(y - ptemp.y()) * (y - ptemp.y());
			if (dist < dmin)
			{
				ppmin = point;
				rulermin = ruler;
				rj = j;
				dmin = dist;
				m_pindex = 0;
			}
		}
	}

	if (ppmin && rulermin)
	{
		m_point = ppmin;
		view->SetCurRuler(rulermin);
		m_mag_ruler = rulermin;
		m_mag_branch = rj;
		m_mag_branch_point = 0;
		return true;
	}
	return false;
}

bool RulerHandler::FindClosestRulerBranchPoint(double mx, double my)
{
	auto view = glbin_current.render_view.lock();
	RulerList* list = glbin_current.GetRulerList();
	if (!view || !list)
		return false;
	size_t rwt = view->m_tseq_cur_num;

	//get view size
	Size2D view_size = view->GetCanvasSize();
	int nx = view_size.w();
	int ny = view_size.h();
	if (nx <= 0 || ny <= 0)
		return false;

	//get aspect, norm xy
	double x, y;
	x = mx * 2.0 / double(nx) - 1.0;
	y = 1.0 - my * 2.0 / double(ny);
	double aspect = (double)nx / (double)ny;

	//get persp
	bool persp = view->GetPersp();

	//get transform
	glm::mat4 mv_temp = view->GetObjectMat();
	glm::mat4 prj_temp = view->GetProjection();
	fluo::Transform mv;
	fluo::Transform prj;
	mv.set(glm::value_ptr(mv_temp));
	prj.set(glm::value_ptr(prj_temp));

	pRulerPoint point, ppmin;
	Ruler* rulermin = 0;
	double dmin = std::numeric_limits<double>::max();
	double dist;
	size_t i, j, k, rj, rk;
	fluo::Point ptemp;
	for (i = 0; i < list->size(); i++)
	{
		Ruler* ruler = (*list)[i];
		if (!ruler) continue;
		if (!ruler->GetDisp()) continue;
		int interp = ruler->GetInterp();

		for (j = 0; j < ruler->GetNumBranch(); j++)
		{
			int nbp = ruler->GetNumBranchPoint(static_cast<int>(j));
			//if (nbp < 2)
			//	continue;
			for (k = 0; k < nbp; ++k)
			{
				point = ruler->GetPRulerPoint(static_cast<int>(j), static_cast<int>(k));
				if (!point) continue;
				ptemp = point->GetPoint(rwt, interp);
				ptemp = mv.transform(ptemp);
				ptemp = prj.transform(ptemp);
				if ((persp && (ptemp.z() <= 0.0 || ptemp.z() >= 1.0)) ||
					(!persp && (ptemp.z() >= 0.0 || ptemp.z() <= -1.0)))
					continue;
				dist = (x - ptemp.x()) * (x - ptemp.x()) +
					(y - ptemp.y()) * (y - ptemp.y());
				if (dist < dmin)
				{
					ppmin = point;
					rulermin = ruler;
					rj = j;
					rk = k;
					dmin = dist;
				}
			}
		}
	}

	if (ppmin && rulermin)
	{
		m_point = ppmin;
		m_mag_ruler = rulermin;
		m_mag_branch = rj;
		m_mag_branch_point = rk;
		return true;
	}
	return false;
}

void RulerHandler::DeletePoint()
{
	RulerList* list = glbin_current.GetRulerList();
	Ruler* ruler = glbin_current.GetRuler();
	if (!ruler || !list)
		return;

	ruler->DeletePoint(m_point);
	m_point = nullptr;
	m_pindex = -1;
	if (!ruler->GetNumPoint())
	{
		//delete ruler
		for (int i = 0; i < (int)list->size(); i++)
		{
			Ruler* r = (*list)[i];
			if (r == ruler)
			{
				list->erase(list->begin() + i);
				//m_ruler = 0;
				return;
			}
		}
	}
}

RulerPoint* RulerHandler::GetEllipsePoint(int index)
{
	Ruler* ruler = glbin_current.GetRuler();
	if (!ruler ||
		ruler->GetRulerMode() != RulerMode::Ellipse ||
		ruler->GetNumPoint() != 4)
		return 0;

	switch (m_pindex)
	{
	case 0:
		return ruler->GetRulerPoint(index);
	case 1:
		{
			int imap[4] = {1, 0, 3, 2};
			return ruler->GetRulerPoint(imap[index]);
		}
	case 2:
		{
			int imap[4] = { 2, 3, 1, 0 };
			return ruler->GetRulerPoint(imap[index]);
		}
	case 3:
		{
			int imap[4] = { 3, 2, 0, 1 };
			return ruler->GetRulerPoint(imap[index]);
		}
	}

	return 0;
}

bool RulerHandler::CompleteEllipse(int mode)
{
	Ruler* ruler = glbin_current.GetRuler();
	if (!ruler ||
		ruler->GetRulerMode() != RulerMode::Ellipse)
		return false;
	size_t rwt = ruler->GetWorkTime();
	int interp = ruler->GetInterp();
	flrd::RulerPoint* p0 = m_point.get();
	flrd::RulerPoint* p1 = GetEllipsePoint(1);
	flrd::RulerPoint* p2 = GetEllipsePoint(2);
	flrd::RulerPoint* p3 = GetEllipsePoint(3);
	if (!p1 || !p2 || !p3)
		return false;
	fluo::Point tmp;
	if (mode == 0)
	{
		fluo::Point c((p2->GetPoint(rwt, interp) +
			p3->GetPoint(rwt, interp)) / 2.0);
		fluo::Vector v0 = p0->GetPoint(rwt, interp) - c;
		fluo::Vector v2 = p2->GetPoint(rwt, interp) - c;
		fluo::Vector axis = Cross(v2, v0);
		axis = Cross(axis, v2);
		axis.normalize();
		tmp = fluo::Point(c + axis * v0.length());
		p0->SetPoint(tmp, rwt);
		tmp = c + (c - p0->GetPoint(rwt, interp));
		p1->SetPoint(tmp, rwt);
	}
	else if (mode == 1)
	{
		fluo::Point c((
			p0->GetPoint(rwt, interp) +
			p1->GetPoint(rwt, interp) +
			p2->GetPoint(rwt, interp) +
			p3->GetPoint(rwt, interp)) / 4.0);
		fluo::Vector v0 = p0->GetPoint(rwt, interp) - c;
		fluo::Vector v2 = p2->GetPoint(rwt, interp) - c;
		fluo::Vector axis = Cross(v2, v0);
		fluo::Vector a2 = Cross(v0, axis);
		a2.normalize();
		tmp = fluo::Point(c + a2 * v2.length());
		p2->SetPoint(tmp, rwt);
		tmp = fluo::Point(c - a2 * v2.length());
		p3->SetPoint(tmp, rwt);
		tmp = c + (c - p0->GetPoint(rwt, interp));
		p1->SetPoint(tmp, rwt);
	}

	return true;
}

void RulerHandler::FinishRuler()
{
	Ruler* ruler = glbin_current.GetRuler();
	if (!ruler)
		return;
	if (ruler->GetRulerMode() == RulerMode::Polyline)
		ruler->SetFinished();
	m_mouse = fluo::Point(-1);
}

bool RulerHandler::GetRulerFinished()
{
	Ruler* ruler = glbin_current.GetRuler();
	if (ruler)
		return ruler->GetFinished();
	else
		return true;
}

Ruler* RulerHandler::AddRuler(fluo::Point& p, size_t t)
{
	RulerList* list = glbin_current.GetRulerList();
	if (!list)
		return 0;

	Ruler* r = new Ruler();
	r->SetWorkTime(t);
	r->Group(m_group);
	r->SetRulerMode(RulerMode::Locator);
	r->AddPoint(p);
	list->push_back(r);
	return r;
}

void RulerHandler::AddRulerPoint(fluo::Point &p)
{
	RulerList* list = glbin_current.GetRulerList();
	if (!list)
		return;

	Ruler* ruler = glbin_current.GetRuler();
	if (ruler &&
		ruler->GetDisp() &&
		!ruler->GetFinished())
	{
		ruler->AddPoint(p);
	}
	else
	{
		ruler = new Ruler();
		ruler->Group(m_group);
		ruler->SetRulerMode(m_mode);
		ruler->AddPoint(p);
		list->push_back(ruler);
		auto view = glbin_current.render_view.lock();
		if (view)
			view->SetCurRuler(ruler);
	}

	Profile(ruler);
}

void RulerHandler::AddRulerPointAfterId(fluo::Point &p, unsigned int id,
	std::set<unsigned int> &cid, std::set<unsigned int> &bid)
{
	RulerList* list = glbin_current.GetRulerList();
	if (!list)
		return;

	Ruler* ruler = glbin_current.GetRuler();
	if (ruler &&
		ruler->GetDisp() &&
		ruler->GetRulerMode() == RulerMode::Polyline &&
		!ruler->GetFinished())
		ruler->AddPointAfterId(p, id, cid, bid);
	else
	{
		ruler = new Ruler();
		ruler->Group(m_group);
		ruler->SetRulerMode(m_mode);
		ruler->AddPointAfterId(p, id, cid, bid);
		//ruler->SetTransient(m_view->m_ruler_time_dep);
		//ruler->SetTransTime(m_view->m_tseq_cur_num);
		list->push_back(ruler);
		auto view = glbin_current.render_view.lock();
		if (view)
			view->SetCurRuler(ruler);
	}

	Profile(ruler);
}

bool RulerHandler::GetMouseDist(int mx, int my, double dist)
{
	if (m_mouse.x() < 0 || m_mouse.y() < 0)
		return true;
	fluo::Point p(mx, my, 0);
	return (p - m_mouse).length() > dist;
}

void RulerHandler::AddRulerPoint(int mx, int my, int branch)
{
	auto view = glbin_current.render_view.lock();
	RulerList* list = glbin_current.GetRulerList();
	if (!view || !list)
		return;

	int point_volume_mode = glbin_settings.m_point_volume_mode;
	size_t rwt = view->m_tseq_cur_num;
	Ruler* ruler = glbin_current.GetRuler();
	if (ruler)
	{
		ruler->SetWorkTime(rwt);
		//DBGPRINT(L"Ruler:%s\n", ruler->GetName().c_str());
	}

	if (branch &&
		m_mode == RulerMode::Polyline)
	{
		if (FindEditingRuler(mx, my))
		{
			if (ruler &&
				ruler->GetDisp() &&
				!ruler->GetFinished())
			{
				ruler->AddBranch(m_point);
				m_mouse = fluo::Point(mx, my, 0);
				return;
			}
		}
		else if (branch == 2)
		{
			FinishRuler();
			return;
		}
	}
	if (m_mode == RulerMode::Probe)
	{
		fluo::Point p1, p2;
		Ruler* ruler = new Ruler();
		ruler->SetWorkTime(rwt);
		ruler->Group(m_group);
		ruler->SetRulerMode(m_mode);
		glbin_volume_point.SetVolumeData(glbin_current.vol_data.lock());
		glbin_volume_point.GetPointVolumeBox2(mx, my, p1, p2);
		ruler->AddPoint(p1);
		ruler->AddPoint(p2);
		list->push_back(ruler);
		//store brush size in ruler
		//flrd::VolumeSelector* selector = m_view->GetVolumeSelector();
		if (glbin_vol_selector.GetBrushSizeData())
			ruler->SetBrushSize(glbin_vol_selector.GetBrushSize1());
		else
			ruler->SetBrushSize(glbin_vol_selector.GetBrushSize1()
				/ view->Get121ScaleFactor());
	}
	else
	{
		fluo::Point p, ip, planep;
		fluo::Point* pplanep = 0;
		if (m_mode == RulerMode::Polyline)
		{
			if (ruler)
			{
				RulerPoint* pp = ruler->GetLastRulerPoint();
				if (pp)
				{
					size_t rwt = ruler->GetWorkTime();
					int interp = ruler->GetInterp();
					planep = pp->GetPoint(rwt, interp);
					pplanep = &planep;
				}
			}
			else
				point_volume_mode = point_volume_mode ? point_volume_mode : 2;
		}
		if (point_volume_mode)
		{
			glbin_volume_point.SetVolumeData(glbin_current.vol_data.lock());
			double t = glbin_volume_point.GetPointVolume(mx, my,
				point_volume_mode,
				glbin_settings.m_ruler_use_transf, 0.5,
				p, ip);
			if (t <= 0.0)
			{
				t = glbin_volume_point.GetPointPlane(mx, my, pplanep, true, p);
				if (t <= 0.0)
					return;
			}
		}
		else
		{
			double t = glbin_volume_point.GetPointPlane(mx, my, pplanep, true, p);
			if (t <= 0.0)
				return;
		}

		bool new_ruler = true;
		if (ruler &&
			ruler->GetDisp() &&
			!ruler->GetFinished())
		{
			ruler->AddPoint(p);
			new_ruler = false;
			if (m_mode == RulerMode::Ellipse)
			{
				//finish
				glm::mat4 mv_temp = view->GetDrawMat();
				glm::vec4 axis(0, 0, -1, 0);
				axis = glm::transpose(mv_temp) * axis;
				ruler->FinishEllipse(fluo::Vector(axis[0], axis[1], axis[2]));
			}
		}
		if (new_ruler)
		{
			ruler = new Ruler();
			ruler->SetWorkTime(rwt);
			ruler->Group(m_group);
			ruler->SetRulerMode(m_mode);
			ruler->AddPoint(p);
			list->push_back(ruler);
			view->SetCurRuler(ruler);
		}
	}

	m_mouse = fluo::Point(mx, my, 0);

	Profile(ruler);
}

void RulerHandler::AddPaintRulerPoint()
{
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;
	Ruler* ruler = glbin_current.GetRuler();
	RulerList* list = glbin_current.GetRulerList();
	if (!list)
		return;

	flrd::Cov cover(vd.get());
	cover.Compute(1);
	flrd::CountVoxels counter;
	counter.SetVolumeData(vd);
	counter.Count();

	fluo::Point center = cover.GetCenter();
	double size = counter.GetSum();
	double intensity = counter.GetWeightedSum();

	std::string str;
	bool new_ruler = true;
	if (ruler &&
		ruler->GetDisp() &&
		!ruler->GetFinished())
	{
		ruler->AddPoint(center);
		str = "\tv" + std::to_string(ruler->GetNumPoint() - 1);
		ruler->AddInfoNames(str);
		str = "\t" + std::to_string(static_cast<int>(size));
		ruler->AddInfoValues(str);
		new_ruler = false;
		if (m_mode == RulerMode::Ellipse)
		{
			//finish
			if (auto view = glbin_current.render_view.lock())
			{
				glm::mat4 mv_temp = view->GetDrawMat();
				glm::vec4 axis(0, 0, -1, 0);
				axis = glm::transpose(mv_temp) * axis;
				ruler->FinishEllipse(fluo::Vector(axis[0], axis[1], axis[2]));
			}
		}
	}
	if (new_ruler)
	{
		ruler = new Ruler();
		ruler->Group(m_group);
		ruler->SetRulerMode(m_mode);
		ruler->AddPoint(center);
		str = "v0";
		ruler->AddInfoNames(str);
		str = std::to_string(static_cast<int>(size));
		ruler->AddInfoValues(str);
		list->push_back(ruler);
		auto view = glbin_current.render_view.lock();
		if (view)
			view->SetCurRuler(ruler);
	}

	ruler->SetPaintIntensity(static_cast<int>(size), intensity*vd->GetMaxScale());
	//Profile(ruler);
}

bool RulerHandler::MoveRuler(int mx, int my)
{
	auto view = glbin_current.render_view.lock();
	Ruler* ruler = glbin_current.GetRuler();
	if (!m_point || !view || !ruler)
		return false;
	size_t rwt = ruler->GetWorkTime();
	int interp = ruler->GetInterp();

	fluo::Point point, ip, tmp;
	if (glbin_settings.m_point_volume_mode)
	{
		glbin_volume_point.SetVolumeData(glbin_current.vol_data.lock());
		double t = glbin_volume_point.GetPointVolume(mx, my,
			glbin_settings.m_point_volume_mode,
			glbin_settings.m_ruler_use_transf,
			0.5, point, ip);
		if (t <= 0.0)
		{
			tmp = m_point->GetPoint(rwt, interp);
			t = glbin_volume_point.GetPointPlane(mx, my, &tmp, true, point);
		}
		if (t <= 0.0)
			return false;
	}
	else
	{
		tmp = m_point->GetPoint(rwt, interp);
		double t = glbin_volume_point.GetPointPlane(mx, my, &tmp, true, point);
		if (t <= 0.0)
			return false;
	}

	fluo::Point p0 = m_point->GetPoint(rwt, interp);
	fluo::Vector displace = point - p0;
	for (int i = 0; i < ruler->GetNumPoint(); ++i)
	{
		ruler->GetRulerPoint(i)->DisplacePoint(displace, rwt, interp);
	}

	Profile(ruler);

	return true;
}

bool RulerHandler::EditPoint(int mx, int my, bool alt)
{
	auto view = glbin_current.render_view.lock();
	Ruler* ruler = glbin_current.GetRuler();
	if (!m_point || !view || !ruler)
		return false;
	size_t rwt = ruler->GetWorkTime();
	int interp = ruler->GetInterp();

	fluo::Point point, ip, tmp;
	if (glbin_settings.m_point_volume_mode)
	{
		glbin_volume_point.SetVolumeData(glbin_current.vol_data.lock());
		double t = glbin_volume_point.GetPointVolume(mx, my,
			glbin_settings.m_point_volume_mode,
			glbin_settings.m_ruler_use_transf,
			0.5, point, ip);
		if (t <= 0.0)
		{
			tmp = m_point->GetPoint(rwt, interp);
			t = glbin_volume_point.GetPointPlane(mx, my, &tmp, true, point);
		}
		if (t <= 0.0)
			return false;
	}
	else
	{
		tmp = m_point->GetPoint(rwt, interp);
		double t = glbin_volume_point.GetPointPlane(mx, my, &tmp, true, point);
		if (t <= 0.0)
			return false;
	}

	m_point->SetPoint(point, rwt);

	CompleteEllipse(alt ? 1 : 0);
	Profile(ruler);
	return true;

}

void RulerHandler::Prune(int idx, int len)
{
	RulerList* list = glbin_current.GetRulerList();
	if (!list)
		return;
	if (idx < 0 || idx >= list->size())
		return;
	if (len <= 0)
		return;
	Ruler* ruler = (*list)[idx];
	if (!ruler)
		return;

	ruler->Prune(len);
}

void RulerHandler::Flip(const std::set<int>& rulers)
{
	RulerList* list = glbin_current.GetRulerList();
	if (!list)
		return;

	bool update_all = rulers.empty();

	size_t c = 0;
	for (auto i : *list)
	{
		if (update_all || rulers.find(static_cast<int>(c)) != rulers.end())
			i->Reverse();
		c++;
	}
	m_edited = true;
}

void RulerHandler::AddAverage(const std::set<int>& rulers)
{
	RulerList* list = glbin_current.GetRulerList();
	if (!list)
		return;

	bool update_all = rulers.empty();

	fluo::Point avg;
	size_t c = 0, count = 0;
	for (auto i : *list)
	{
		if (update_all || rulers.find(static_cast<int>(c)) != rulers.end())
		{
			avg += i->GetCenter();
			count++;
		}
		c++;
	}

	if (!count)
		return;

	avg /= double(count);
	flrd::Ruler* ruler = new flrd::Ruler();
	ruler->SetRulerMode(RulerMode::Locator);
	ruler->SetName(L"Average");
	ruler->AddPoint(avg);
	ruler->SetTransient(false);
	ruler->SetTransTime(0);
	list->push_back(ruler);
}

bool RulerHandler::GetAutoRelax()
{
	int relax_ruler = glbin_automate_def.m_relax_ruler;
	if (relax_ruler == 0)
		return false;
	else if (relax_ruler == 1)
		return true;
	else if (relax_ruler == 2)
	{
		auto vd = glbin_current.vol_data.lock();
		if (vd && vd->GetAllBrickNum() == 1)
			return true;
	}
	return false;
}

void RulerHandler::Relax()
{
	Ruler* ruler = glbin_current.GetRuler();
	if (!ruler)
		return;

	flrd::CelpList* list = glbin_comp_analyzer.GetCelpList();
	if (list && list->empty())
		list = 0;
	double infr = glbin_settings.m_ruler_infr;
	int type = glbin_settings.m_ruler_relax_type;
	int iter = glbin_settings.m_ruler_relax_iter;
	double f1 = glbin_settings.m_ruler_relax_f1;
	glbin_dist_calculator.SetF1(f1);
	glbin_dist_calculator.SetInfr(infr);
	glbin_dist_calculator.SetCelpList(list);
	glbin_dist_calculator.SetVolume(glbin_current.vol_data.lock().get());
	glbin_dist_calculator.SetRuler(ruler);
	glbin_dist_calculator.CenterRuler(type, m_edited, iter);
	m_edited = false;
}

void RulerHandler::Relax(const std::set<int>& rulers)
{
	RulerList* list = glbin_current.GetRulerList();
	if (!list)
		return;

	bool update_all = rulers.empty();

	flrd::CelpList* cplist = glbin_comp_analyzer.GetCelpList();
	if (cplist && cplist->empty())
		cplist = 0;
	double infr = glbin_settings.m_ruler_infr;
	int type = glbin_settings.m_ruler_relax_type;
	int iter = glbin_settings.m_ruler_relax_iter;
	double f1 = glbin_settings.m_ruler_relax_f1;
	glbin_dist_calculator.SetF1(f1);
	glbin_dist_calculator.SetInfr(infr);
	glbin_dist_calculator.SetCelpList(cplist);
	glbin_dist_calculator.SetVolume(glbin_current.vol_data.lock().get());

	size_t c = 0;
	for (auto i : *list)
	{
		if (update_all || rulers.find(static_cast<int>(c)) != rulers.end())
		{
			glbin_dist_calculator.SetRuler(i);
			glbin_dist_calculator.CenterRuler(type, m_edited, iter);
		}
		c++;
	}
	m_edited = false;
}

void RulerHandler::Prune(const std::set<int>& rulers)
{
	RulerList* list = glbin_current.GetRulerList();
	if (!list)
		return;

	bool update_all = rulers.empty();

	size_t c = 0;
	for (auto i : *list)
	{
		if (update_all || rulers.find(static_cast<int>(c)) != rulers.end())
			Prune(static_cast<int>(c), 1);
		c++;
	}
}

//stroke for magnet
void RulerHandler::ApplyMagPoint()
{
	if (m_mag_stroke.size() > 1)
		return;
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;
	size_t rwt = view->m_tseq_cur_num;
	fluo::Point p = m_mag_stroke.front();
	if (!FindClosestRulerPoint(m_magx, m_magy))
		return;
	if (!m_point)
		return;
	m_point->SetPoint(p, rwt);

	CompleteEllipse(1);
	Profile(glbin_current.GetRuler());
}

void RulerHandler::ApplyMagStroke()
{
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;
	size_t rwt = view->m_tseq_cur_num;
	if (m_mag_stroke.size() < 2)
		return;
	if (!m_mag_ruler)
	{
		if (m_redist_len)
			FindClosestRulerBranch(m_magx, m_magy);
		else
		{
			FindClosestRulerBranchPoint(m_magx, m_magy);
			InitMagRulerLength();
		}
	}
	if (!m_mag_ruler || m_mag_branch >= m_mag_ruler->GetNumBranch())
		return;
	int num = m_mag_ruler->GetNumBranchPoint(static_cast<int>(m_mag_branch));
	if (num == 1)
	{
		//move point
		pRulerPoint temp = m_mag_ruler->GetPRulerPoint(static_cast<int>(m_mag_branch), 0);
		if (!temp)
			return;
		fluo::Point p = m_mag_stroke.back();
		temp->SetPoint(p, rwt);
		return;
	}

	if (m_redist_len && m_mag_branch_point >= num)
		return;

	InitMagStrokeLength(num);

	for (size_t i = m_mag_branch_point; i < num; ++i)
	{
		pRulerPoint temp = m_mag_ruler->GetPRulerPoint(static_cast<int>(m_mag_branch), static_cast<int>(i));
		if (!temp)
			continue;
		if (m_redist_len)
		{
			//make lengths equal
			fluo::Point p = GetPointOnMagStroke(int(i));
			temp->SetPoint(p, rwt);
		}
		else
		{
			//snap each point
			size_t si = i - m_mag_branch_point;
			//if (si < m_mag_stroke.size())
			//	temp->SetPoint(m_mag_stroke[si], rwt);
			double d = 0;
			if (si < m_mag_ruler_len.size())
				d = m_mag_ruler_len[si];
			if (m_mag_stroke_len.empty() ||
				d > m_mag_stroke_len.back())
				break;
			fluo::Point p = GetPointOnMagStroke(d);
			temp->SetPoint(p, rwt);
		}
	}

	CompleteEllipse(1);
	Profile(glbin_current.GetRuler());
}

void RulerHandler::InitMagStrokeLength(int n)
{
	size_t stroke_num = m_mag_stroke.size();
	m_mag_stroke_len.resize(stroke_num, 0);

	//compute total length
	for (size_t i = 0; i < stroke_num - 1; ++i)
	{
		fluo::Point p1 = m_mag_stroke[i];
		fluo::Point p2 = m_mag_stroke[i+1];
		m_mag_stroke_len[i+1] = m_mag_stroke_len[i] + (p2 - p1).length();
	}
	m_mag_stroke_int = m_mag_stroke_len.back() / (n - 1);
}

void RulerHandler::InitMagRulerLength()
{
	m_mag_ruler_len.clear();
	if (!m_mag_ruler)
		return;
	if (m_mag_branch >= m_mag_ruler->GetNumBranch())
		return;
	size_t bp_num = m_mag_ruler->GetNumBranchPoint(static_cast<int>(m_mag_branch));
	if (m_mag_branch_point >= bp_num)
		return;

	size_t ruler_num = bp_num - m_mag_branch_point;
	m_mag_ruler_len.resize(ruler_num, 0);

	//compute total length
	for (size_t i = m_mag_branch_point; i < bp_num - 1; ++i)
	{
		fluo::Point p1 = m_mag_ruler->GetPoint(static_cast<int>(m_mag_branch), static_cast<int>(i));
		fluo::Point p2 = m_mag_ruler->GetPoint(static_cast<int>(m_mag_branch), static_cast<int>(i + 1));
		size_t si = i - m_mag_branch_point;
		m_mag_ruler_len[si + 1] = m_mag_ruler_len[si] + (p2 - p1).length();
	}
}

fluo::Point RulerHandler::GetPointOnMagStroke(int i)
{
	double len = m_mag_stroke_int * i;
	double t;
	fluo::Point p1, p2;
	bool found = false;
	size_t stroke_num = m_mag_stroke_len.size();
	for (size_t i = 0; i < stroke_num - 1; ++i)
	{
		if (m_mag_stroke_len[i] <= len &&
			m_mag_stroke_len[i + 1] >= len)
		{
			p1 = m_mag_stroke[i];
			p2 = m_mag_stroke[i + 1];
			t = (len - m_mag_stroke_len[i]) / 
				(m_mag_stroke_len[i + 1] - m_mag_stroke_len[i]);
			found = true;
			break;
		}
	}
	if (found)
		return fluo::Point(p1 * (1 - t) + p2 * t);
	if (i == 0)
		return m_mag_stroke.front();
	return m_mag_stroke.back();
}

fluo::Point RulerHandler::GetPointOnMagStroke(double d)
{
	double t;
	fluo::Point p1, p2;
	bool found = false;
	size_t stroke_num = m_mag_stroke_len.size();
	for (size_t i = 0; i < stroke_num - 1; ++i)
	{
		if (m_mag_stroke_len[i] <= d &&
			m_mag_stroke_len[i + 1] >= d)
		{
			p1 = m_mag_stroke[i];
			p2 = m_mag_stroke[i + 1];
			t = (d - m_mag_stroke_len[i]) /
				(m_mag_stroke_len[i + 1] - m_mag_stroke_len[i]);
			found = true;
			break;
		}
	}
	if (found)
		return fluo::Point(p1 * (1 - t) + p2 * t);
	if (d == 0.0)
		return m_mag_stroke.front();
	return m_mag_stroke.back();
}

void RulerHandler::ClearMagStroke()
{
	m_mag_stroke.clear();
	m_mouse = fluo::Point(-1);
	m_mag_ruler = 0;
	m_mag_branch = 0;
}

void RulerHandler::AddMagStrokePoint(int mx, int my)
{
	int point_volume_mode = glbin_settings.m_point_volume_mode;
	fluo::Point p, ip, planep;
	fluo::Point* pplanep = 0;

	if (point_volume_mode)
	{
		glbin_volume_point.SetVolumeData(glbin_current.vol_data.lock());
		double t = glbin_volume_point.GetPointVolume(mx, my,
			point_volume_mode,
			glbin_settings.m_ruler_use_transf,
			0.5, p, ip);
		if (t <= 0.0)
		{
			t = glbin_volume_point.GetPointPlane(mx, my, pplanep, true, p);
			if (t <= 0.0)
				return;
		}
	}
	else
	{
		double t = glbin_volume_point.GetPointPlane(mx, my, pplanep, true, p);
		if (t <= 0.0)
			return;
	}

	if (m_mag_stroke.empty())
	{
		m_magx = mx;//only the first point
		m_magy = my;
	}
	m_mag_stroke.push_back(p);
	m_mouse = fluo::Point(mx, my, 0);
}

void RulerHandler::DeleteSelection(const std::set<int> &sel)
{
	RulerList* list = glbin_current.GetRulerList();
	if (!list)
		return;

	for (auto it = list->rbegin();
		it != list->rend();)
	{
		auto it2 = std::next(it).base();
		int idx = static_cast<int>(it2 - list->begin());
		if (sel.find(idx) != sel.end())
		{
			if (*it2)
				delete *it2;
			it2 = list->erase(it2);
			it = std::reverse_iterator<flrd::RulerList::iterator>(it2);
		}
		else
			++it;
	}

	auto view = glbin_current.render_view.lock();
	if (view)
		view->SetCurRuler(0);
	m_point = nullptr;
	m_pindex = -1;
}

void RulerHandler::DeleteAll(bool cur_time)
{
	RulerList* list = glbin_current.GetRulerList();
	auto view = glbin_current.render_view.lock();
	if (!list || !view)
		return;

	if (cur_time)
	{
		int tseq = view->m_tseq_cur_num;
		for (size_t i = list->size(); i > 0; i--)
		{
			flrd::Ruler* ruler = (*list)[i-1];
			if (ruler &&
				((ruler->GetTransient() &&
					ruler->GetTransTime() == tseq) ||
					!ruler->GetTransient()))
			{
				list->erase(list->begin() + i - 1);
				delete ruler;
			}
		}
	}
	else
	{
		for (size_t i = list->size(); i > 0; i--)
		{
			flrd::Ruler* ruler = (*list)[i-1];
			if (ruler)
				delete ruler;
		}
		list->clear();
	}

	view->SetCurRuler(0);
	m_point = nullptr;
	m_pindex = -1;
}

std::wstring RulerHandler::PrintRulers(bool h)
{
	std::wstring s;
	RulerList* list = glbin_current.GetRulerList();
	if (!list)
		return s;

	for (size_t ri = 0; ri < list->size(); ++ri)
	{
		Ruler* ruler = (*list)[ri];
		if (!ruler) continue;
		std::wstring rname = ruler->GetName();
		std::wstring line;
		int rpn = ruler->GetNumPoint();
		for (size_t rpi = 0; rpi < rpn; ++rpi)
		{
			if (h)
			{
				if (rpi == 0)
					line = L"- - ";
				else
					line = L"  - ";
			}
			else
				line = L"- ";
			line += rname;
			if (rpn > 1)
				line += L"_" + std::to_wstring(rpi + 1);
			line += L"\n";
			s += line;
		}
	}
	return s;
}

void RulerHandler::Profile(const std::set<int>& rulers)
{
	RulerList* list = glbin_current.GetRulerList();
	if (!list)
		return;

	bool update_all = rulers.empty();

	size_t c = 0;
	for (auto i : *list)
	{
		if (update_all || rulers.find(static_cast<int>(c)) != rulers.end())
			Profile(i);
		c++;
	}
}

int RulerHandler::Profile(Ruler* ruler)
{
	auto view = glbin_current.render_view.lock();
	auto vd = glbin_current.vol_data.lock();
	if (!view || !vd || !ruler)
		return 0;
	if (ruler->GetNumPoint() < 1)
		return 0;
	size_t rwt = ruler->GetWorkTime();

	//set ruler transform
	fluo::Transform tf = view->GetInvOffsetMat();
	ruler->SetTransform(tf);

	double spcx, spcy, spcz;
	vd->GetSpacings(spcx, spcy, spcz);
	int nx, ny, nz;
	vd->GetResolution(nx, ny, nz);
	if (spcx <= 0.0 || spcy <= 0.0 || spcz <= 0.0 ||
		nx <= 0 || ny <= 0 || nz <= 0)
		return 0;
	//get data
	vd->GetVR()->return_mask();
	flvr::Texture* tex = vd->GetTexture();
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
	double scale = vd->GetScalarScale();
	//set up sampler
	m_data = data;
	m_nx = nx; m_ny = ny; m_nz = nz;
	m_bits = vd->GetBits();
	m_scale = vd->GetScalarScale();
	ruler->SetScalarScale(m_bits == 8 ? 255: 65535);
	if (!valid()) return 0;

	if (ruler->GetRulerMode() == RulerMode::Probe &&
		mask)
	{
		if (ruler->GetNumPoint() < 1)
			return 0;
		fluo::Point p1, p2;
		p1 = ruler->GetPoint(0);
		p2 = ruler->GetPoint(1);
		//object space
		p1 = fluo::Point(p1.x() / spcx, p1.y() / spcy, p1.z() / spcz);
		p2 = fluo::Point(p2.x() / spcx, p2.y() / spcy, p2.z() / spcz);
		fluo::Vector dir = p2 - p1;
		double dist = dir.length();
		if (dist < EPS)
			return 0;
		dir.normalize();

		//bin number
		int bins = int(dist / 1 + 0.5);
		if (bins <= 0) return 0;
		double bin_dist = dist / bins;
		std::vector<flrd::ProfileBin>* profile = ruler->GetProfile();
		if (!profile) return 0;
		profile->clear();
		profile->reserve(size_t(bins));
		for (unsigned int b = 0; b < static_cast<unsigned int>(bins); ++b)
			profile->push_back(flrd::ProfileBin());

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
						fluo::Point p(i, j, k);
						fluo::Vector pdir = p - p1;
						double proj = fluo::Dot(pdir, dir);
						int bin_num = int(proj / bin_dist);
						if (bin_num < 0 || bin_num >= bins)
							continue;
						//make sure it's within the brush radius
						fluo::Point p_ruler = p1 + proj * dir;
						if ((p_ruler - p).length() > brush_radius)
							continue;

						double intensity = get_data(i, j, k);

						(*profile)[bin_num].m_pixels++;
						(*profile)[bin_num].m_accum += intensity;
					}
				}
	}
	else
	{
		//calculate length in object space
		double total_length = ruler->GetLengthObject(spcx, spcy, spcz);
		int bins = int(total_length / m_step_length);
		std::vector<flrd::ProfileBin>* profile = ruler->GetProfile();
		if (!profile) return 0;
		profile->clear();

		//sample data through ruler
		fluo::Point p;
		double intensity;
		if (bins == 0)
		{
			//allocate
			profile->reserve(size_t(1));
			profile->push_back(flrd::ProfileBin());

			p = ruler->GetPointTransformed(0);
			//object space
			p = fluo::Point(p.x() / spcx, p.y() / spcy, p.z() / spcz);
			intensity = get_filtered_data(p.x(), p.y(), p.z());
			(*profile)[0].m_pixels++;
			(*profile)[0].m_accum += intensity;
			(*profile)[0].dist = 0;
		}
		else
		{
			//allocate
			profile->reserve(size_t(bins));
			for (unsigned int b = 0; b < static_cast<unsigned int>(bins); ++b)
				profile->push_back(flrd::ProfileBin());

			fluo::Point p1, p2;
			fluo::Vector dir, dir2;
			double dist;
			double real_dist = 0, real_step = 0;
			int total_dist = 0;
			for (int pn = 0; pn < ruler->GetNumPoint() - 1; ++pn)
			{
				p1 = ruler->GetPointTransformed(pn);
				p2 = ruler->GetPointTransformed(pn + 1);
				//object space
				p1 = fluo::Point(p1.x() / spcx, p1.y() / spcy, p1.z() / spcz);
				p2 = fluo::Point(p2.x() / spcx, p2.y() / spcy, p2.z() / spcz);
				dir = p2 - p1;
				dist = dir.length();
				dir.normalize();
				dir2 = dir * m_step_length;
				dir2 *= fluo::Vector(spcx, spcy, spcz);
				real_step = dir2.length();

				for (double dn = 0; dn < dist; dn += m_step_length)
				{
					p = p1 + dir * dn;
					intensity = get_filtered_data(p.x(), p.y(), p.z());
					if (total_dist >= bins) break;
					(*profile)[total_dist].m_pixels++;
					(*profile)[total_dist].m_accum += intensity;
					(*profile)[total_dist].dist = real_dist;
					real_dist += real_step;
					total_dist++;
				}
			}
			if (total_dist < bins)
				profile->erase(profile->begin() + total_dist, profile->begin() + bins - 1);
		}
	}
	std::string str = "Profile of volume ";
	str = str + ws2s(vd->GetName());
	ruler->SetInfoProfile(str);

	return 1;
}

int RulerHandler::Profile(int index)
{
	RulerList* list = glbin_current.GetRulerList();
	if (!list)
		return 0;
	if (index < 0 ||
		index >= list->size())
		return 0;
	flrd::Ruler* ruler = (*list)[index];
	return Profile(ruler);
}

int RulerHandler::ProfileAll()
{
	RulerList* list = glbin_current.GetRulerList();
	if (!list)
		return 0;
	int c = 0;
	for (size_t i = 0; i < list->size(); ++i)
		c += Profile(static_cast<int>(i));
	return c;
}

int RulerHandler::Roi(Ruler* ruler)
{
	auto view = glbin_current.render_view.lock();
	auto vd = glbin_current.vol_data.lock();
	if (!view || !vd || !ruler)
		return 0;
	if (ruler->GetRulerMode() != RulerMode::Ellipse ||
		ruler->GetNumPoint() != 4)
		return 0;
	//size_t rwt = ruler->GetWorkTime();

	//set ruler transform
	fluo::Transform tf;// = m_view->GetInvOffsetMat();
	//ruler->SetTransform(tf);

	double spcx, spcy, spcz;
	vd->GetSpacings(spcx, spcy, spcz);
	int nx, ny, nz;
	vd->GetResolution(nx, ny, nz);
	if (spcx <= 0.0 || spcy <= 0.0 || spcz <= 0.0 ||
		nx <= 0 || ny <= 0 || nz <= 0)
		return 0;

	//get data
	vd->GetVR()->return_mask();
	flvr::Texture* tex = vd->GetTexture();
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
	double scale = vd->GetScalarScale();
	//set up sampler
	m_data = data;
	m_nx = nx; m_ny = ny; m_nz = nz;
	m_bits = vd->GetBits();
	m_scale = vd->GetScalarScale();
	ruler->SetScalarScale(m_bits == 8 ? 255 : 65535);
	if (!valid()) return 0;

	//get transform
	glm::mat4 mv = view->GetObjectMat();
	glm::mat4 prj = view->GetProjection();
	glm::mat4 mvprj = prj * mv;
	tf.set(glm::value_ptr(mvprj));
	//get volume roi
	VolumeRoi vr(vd.get());
	vr.SetTransform(tf);
	vr.SetRoi(ruler);
	int vx, vy;
	view->GetRenderSize(vx, vy);
	vr.SetAspect(vx, vy);
	vr.Run();
	ruler->SetMeanInt(vr.GetResult());

	return 1;
}

int RulerHandler::Roi(int index)
{
	RulerList* list = glbin_current.GetRulerList();
	if (!list)
		return 0;
	if (index < 0 ||
		index >= list->size())
		return 0;
	flrd::Ruler* ruler = (*list)[index];
	return Roi(ruler);
}

int RulerHandler::RoiAll()
{
	RulerList* list = glbin_current.GetRulerList();
	if (!list)
		return 0;
	int c = 0;
	for (size_t i = 0; i < list->size(); ++i)
		c += Roi(static_cast<int>(i));
	return c;
}

void RulerHandler::Distance(const std::set<int>& rulers, const std::wstring& filename)
{
	RulerList* list = glbin_current.GetRulerList();
	if (!list)
	{
		return;
	}
	flrd::CelpList* cplist = glbin_comp_analyzer.GetCelpList();
	if (cplist->empty())
	{
		return;
	}

	bool update_all = rulers.empty();

	std::wstring str = filename;
	str = str.substr(0, str.find_last_of(L'.'));
	std::wstring fi;

	double sx = cplist->sx;
	double sy = cplist->sy;
	double sz = cplist->sz;

	size_t c = 0;
	for (auto i : *list)
	{
		if (update_all || rulers.find(static_cast<int>(c)) != rulers.end())
		{
			flrd::Ruler* ruler = i;
			if (!ruler)
				continue;
			if (ruler->GetNumPoint() < 1)
				continue;

			fluo::Point p = ruler->GetCenter();
			for (auto it = cplist->begin();
				it != cplist->end(); ++it)
			{
				double dist = (p - it->second->GetCenter(sx, sy, sz)).length();
				it->second->SetDistp(dist);
			}

			fi = str + std::to_wstring(c) + L".txt";
			glbin_comp_analyzer.OutputCompListFile(fi, 1);
		}
		c++;
	}
}

void RulerHandler::Project(const std::set<int>& rulers, const std::wstring& filename)
{
	RulerList* list = glbin_current.GetRulerList();
	if (!list)
	{
		return;
	}
	flrd::CelpList* cplist = glbin_comp_analyzer.GetCelpList();
	if (cplist->empty())
	{
		return;
	}
	glbin_dist_calculator.SetCelpList(cplist);

	bool update_all = rulers.empty();

	std::wstring str = filename;
	std::ofstream ofs;
	ofs.open(str, std::ofstream::out);

	size_t c = 0;
	for (auto i : *list)
	{
		if (update_all || rulers.find(static_cast<int>(c)) != rulers.end())
		{
			glbin_dist_calculator.SetRuler(i);
			glbin_dist_calculator.Project();

			std::vector<flrd::Celp> comps;
			for (auto it = cplist->begin();
				it != cplist->end(); ++it)
				comps.push_back(it->second);
			std::sort(comps.begin(), comps.end(),
				[](const flrd::Celp& a, const flrd::Celp& b) -> bool
				{
					fluo::Point pa = a->GetProjp();
					fluo::Point pb = b->GetProjp();
					if (pa.z() != pb.z()) return pa.z() < pb.z();
					else return pa.x() < pb.x();
				});

			for (auto it = comps.begin();
				it != comps.end(); ++it)
			{
				ofs << (*it)->Id() << "\t";
				fluo::Point p = (*it)->GetProjp();
				ofs << p.x() << "\t";
				ofs << p.y() << "\t";
				ofs << p.z() << "\n";
			}
		}
		c++;
	}

	ofs.close();
}

void RulerHandler::SetTransient(bool bval, const std::set<int>& rulers)
{
	RulerList* list = glbin_current.GetRulerList();
	if (!list)
		return;

	bool update_all = rulers.empty();

	size_t t = 0;
	if (auto cur_view = glbin_current.render_view.lock())
		t = cur_view->m_tseq_cur_num;

	size_t c = 0;
	for (auto i : *list)
	{
		if (update_all || rulers.find(static_cast<int>(c)) != rulers.end())
		{
			if (!i)
				continue;
			i->SetTransient(bval);
			if (bval)
				i->SetTransTime(t);
		}
		c++;
	}
}

void RulerHandler::SetDisplay(bool bval, const std::set<int>& rulers, int type)
{
	RulerList* list = glbin_current.GetRulerList();
	if (!list)
		return;

	bool update_all = rulers.empty();

	size_t c = 0;
	for (auto i : *list)
	{
		if (update_all || rulers.find(static_cast<int>(c)) != rulers.end())
		{
			if (!i)
				continue;
			i->SetDisplay(type, bval);
		}
		c++;
	}
}

void RulerHandler::SetInterp(int ival, const std::set<int>& rulers)
{
	RulerList* list = glbin_current.GetRulerList();
	if (!list)
		return;

	bool update_all = rulers.empty();

	size_t c = 0;
	for (auto i : *list)
	{
		if (update_all || rulers.find(static_cast<int>(c)) != rulers.end())
		{
			if (!i)
				continue;
			i->SetInterp(ival);
		}
		c++;
	}
}

void RulerHandler::DeleteKey(const std::set<int>& rulers)
{
	RulerList* list = glbin_current.GetRulerList();
	if (!list)
		return;

	bool update_all = rulers.empty();

	size_t t = 0;
	if (auto cur_view = glbin_current.render_view.lock())
		t = cur_view->m_tseq_cur_num;

	size_t c = 0;
	for (auto i : *list)
	{
		if (update_all || rulers.find(static_cast<int>(c)) != rulers.end())
		{
			if (!i)
				continue;
			i->SetWorkTime(t);
			i->DeleteKey();
		}
		c++;
	}
}

void RulerHandler::DeleteAllKeys(const std::set<int>& rulers)
{
	RulerList* list = glbin_current.GetRulerList();
	if (!list)
		return;

	bool update_all = rulers.empty();

	size_t t = 0;
	if (auto cur_view = glbin_current.render_view.lock())
		t = cur_view->m_tseq_cur_num;

	size_t c = 0;
	for (auto i : *list)
	{
		if (update_all || rulers.find(static_cast<int>(c)) != rulers.end())
		{
			if (!i)
				continue;
			i->SetWorkTime(t);
			i->DeleteAllKey();
		}
		c++;
	}
}

//get time points where keys exist
bool RulerHandler::GetKeyFrames(std::set<size_t>& kf)
{
	auto view = glbin_current.render_view.lock();
	RulerList* list = glbin_current.GetRulerList();
	if (!view || !list)
		return false;
	size_t startf, endf;
	startf = view->m_begin_frame;
	endf = view->m_end_frame;

	if (!list)
		return false;
	if (list->empty())
		return false;
	kf.clear();

	for (auto i : *list)
	{
		if (!i)
			continue;
		for (size_t j = 0; j < i->GetNumPoint(); ++j)
		{
			RulerPoint* p = i->GetRulerPoint(static_cast<int>(j));
			if (!p)
				continue;
			size_t tn = p->GetTimeNum();
			for (size_t k = 0; k < tn; ++k)
			{
				size_t t;
				fluo::Point pp;
				if (p->GetTimeAndPoint(k, t, pp) &&
					t > startf && t <= endf)
					kf.insert(t);
			}
		}
	}
	return !kf.empty();
}

size_t RulerHandler::GetRulerPointNum()
{
	RulerList* list = glbin_current.GetRulerList();
	if (!list || list->empty())
		return 0;

	size_t sum = 0;
	for (auto i : *list)
	{
		if (!i)
			continue;
		sum += i->GetNumPoint();
	}
	return sum;
}

bool RulerHandler::GetRulerPointNames(std::vector<std::wstring>& names)
{
	RulerList* list = glbin_current.GetRulerList();
	if (!list || list->empty())
		return false;

	names.clear();
	for (auto i : *list)
	{
		if (!i)
			continue;
		int rpn = i->GetNumPoint();
		std::wstring name = i->GetName();
		std::wstring str;
		if (rpn == 1)
		{
			names.push_back(name);
		}
		else if (rpn > 1)
		{
			for (size_t j = 0; j < rpn; ++j)
			{
				str = name + L"_" + std::to_wstring(j + 1);
				names.push_back(str);
			}
		}
	}
	return true;
}

bool RulerHandler::GetRulerPointNames(std::vector<std::string>& names)
{
	RulerList* list = glbin_current.GetRulerList();
	if (!list || list->empty())
		return false;

	names.clear();
	for (auto i : *list)
	{
		if (!i)
			continue;
		int rpn = i->GetNumPoint();
		std::wstring name = i->GetName();
		std::wstring str;
		if (rpn == 1)
		{
			names.push_back(ws2s(name));
		}
		else if (rpn > 1)
		{
			for (size_t j = 0; j < rpn; ++j)
			{
				str = name + L"_" + std::to_wstring(j + 1);
				names.push_back(ws2s(str));
			}
		}
	}
	return true;
}

bool RulerHandler::GetRulerPointCoords(std::vector<double>& coords)
{
	RulerList* list = glbin_current.GetRulerList();
	if (!list || list->empty())
		return false;

	std::set<size_t> kf;
	GetKeyFrames(kf);
	if (kf.empty())
		return false;

	coords.clear();
	for (auto t : kf)
	{
		for (auto i : *list)
		{
			if (!i)
				continue;
			i->SetWorkTime(t);
			for (int k = 0; k < i->GetNumPoint(); ++k)
			{
				fluo::Point p = i->GetPoint(k);
				coords.push_back(p.x());
				coords.push_back(p.y());
			}
		}
	}
	return true;
}

RulerPoint* RulerHandler::get_closest_point(fluo::Point& p)
{
	auto view = glbin_current.render_view.lock();
	RulerList* list = glbin_current.GetRulerList();
	if (!view || !list)
		return nullptr;

	size_t rwt = view->m_tseq_cur_num;
	double dmin = std::numeric_limits<double>::max();
	RulerPoint* result = 0;

	size_t ri, rj;
	for (auto r : *list)
	{
		r->SetWorkTime(rwt);
		int interp = r->GetInterp();
		pRulerPoint temp = r->FindNearestPRulerPoint(p, ri, rj);
		if (!temp)
			continue;
		double d = (temp->GetPoint(rwt, interp) - p).length();
		if (d < dmin)
		{
			result = temp.get();
			dmin = d;
		}
	}

	if (result)
	{
		//check range

	}

	return result;
}

void RulerHandler::GenerateWalk(size_t nl, double dir, WalkCycle& cycle)
{
	auto view = glbin_current.render_view.lock();
	RulerList* list = glbin_current.GetRulerList();
	if (!view || !list || list->empty())
		return;

	std::vector<WcName> names;
	cycle.GetNames(names);
	if (names.empty())
		return;
	WalkData& data = cycle.GetData();
	size_t period = data.length();
	size_t length = nl * period;

	for (size_t i = 0; i < length; ++i)
	{
		size_t cnt = 0;
		size_t t = i + 1;
		size_t f = i % period;//phase
		for (auto& name : names)
		{
			Ruler* r = list->GetRuler(name.s);
			if (!r)
				continue;
			int dim = static_cast<int>(name.d);
			//compute integral for each point
			for (size_t j = 0; j < r->GetNumPoint(); ++j)
			{
				r->SetWorkTime(i);
				fluo::Point p = r->GetPoint(static_cast<int>(j));
				double x, y, z;
				x = dim > 0 ? data.get(cnt++, f) : 0;
				y = dim > 1 ? data.get(cnt++, f) : 0;
				z = dim > 2 ? data.get(cnt++, f) : 0;
				fluo::Vector v(x, y, z);
				p += v * dir;
				r->SetWorkTime(t);
				r->SetPoint(static_cast<int>(j), p);
			}
		}
	}
}