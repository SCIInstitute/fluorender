/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2026 Scientific Computing and Imaging Institute,
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
#include <RulerList.h>
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

void RulerHandler::SetEditingRuler(int index)
{
	auto ruler = GetRuler(index);
	if (ruler)
	{
		m_editing_ruler_index = index;
		auto view = glbin_current.render_view.lock();
		if (view)
			view->SetCurRuler(ruler);
	}
}

void RulerHandler::NewGroup()
{
	auto list = glbin_current.GetRulerList();
	if (list->get().IsEmpty())
		return;

	auto groups = list->get().Groups();
	if (!groups.empty())
	{
		auto it = std::max_element(groups.begin(), groups.end());
		if (it != groups.end())
			m_group = *it + 1;
	}
}

void RulerHandler::GroupRulers(const std::set<int>& rulers)
{
	auto list = glbin_current.GetRulerList();
	if (list->get().IsEmpty())
		return;

	bool update_all = rulers.empty();

	size_t c = 0;
	auto ruler_list = list->get().All();
	for (auto ruler : ruler_list)
	{
		if (update_all || rulers.find(static_cast<int>(c)) != rulers.end())
			ruler->Group(m_group);
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

std::shared_ptr<Ruler> RulerHandler::GetRuler(const std::wstring& name)
{
	auto list = glbin_current.GetRulerList();
	if (list->get().IsEmpty())
		return nullptr;
	return list->get().FindByName(name);
}

std::shared_ptr<Ruler> RulerHandler::GetRuler(size_t i)
{
	auto list = glbin_current.GetRulerList();
	if (list->get().IsEmpty())
		return nullptr;
	auto rulers = list->get().All();
	if (i >= rulers.size())
		return nullptr;
	return rulers[i];
}

int RulerHandler::GetRulerIndex()
{
	auto ruler = glbin_current.GetRuler();
	auto list = glbin_current.GetRulerList();
	if (!ruler || list->get().IsEmpty())
		return -1;
	auto rulers = list->get().All();
	for (int i = 0; i < rulers.size(); ++i)
		if (rulers[i] == ruler)
			return i;
	return -1;
}

void RulerHandler::GetRulerList(const std::set<int>& rulers, flrd::RulerList& out_list)
{
	auto list = glbin_current.GetRulerList();
	if (list->get().IsEmpty())
		return;

	bool update_all = rulers.empty();

	size_t c = 0;
	auto ruler_list = list->get().All();
	for (auto i : ruler_list)
	{
		if (update_all || rulers.find(static_cast<int>(c)) != rulers.end())
			out_list.Add(i);
		c++;
	}
}

void RulerHandler::SetDisplay(bool bval, const std::set<int>& rulers)
{
	auto list = glbin_current.GetRulerList();
	if (!list)
		return;

	auto ruler_list = list->get().All();
	for (auto i : ruler_list)
	{
		if (!i) continue;
		if (rulers.find(i->Id()) != rulers.end())
			i->SetDisp(bval);
	}
}

void RulerHandler::ToggleDisplay(const std::set<int>& rulers)
{
	auto list = glbin_current.GetRulerList();
	if (!list)
		return;

	auto ruler_list = list->get().All();
	for (auto i : ruler_list)
	{
		if (!i) continue;
		if (rulers.find(static_cast<int>(i->Id())) != rulers.end())
			i->ToggleDisp();
	}
}

void RulerHandler::ToggleGroupDisp()
{
	auto list = glbin_current.GetRulerList();
	if (!list)
		return;

	bool disp;
	bool first = true;
	auto ruler_list = list->get().All();
	for (auto i : ruler_list)
	{
		if (!i) continue;
		if (i->Group() == m_group)
		{
			if (first)
			{
				first = false;
				disp = !i->GetDisp();
			}
			i->SetDisp(disp);
		}
	}
}

bool RulerHandler::FindEditingRuler(double mx, double my)
{
	auto view = glbin_current.render_view.lock();
	auto list = glbin_current.GetRulerList();
	if (!view || list->get().IsEmpty())
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
	glm::mat4 mv_temp = view->GetModelView();
	glm::mat4 prj_temp = view->GetProjection();
	fluo::Transform mv;
	fluo::Transform prj;
	mv.set(glm::value_ptr(mv_temp));
	prj.set(glm::value_ptr(prj_temp));

	std::shared_ptr<RulerPoint> point;
	fluo::Point ptemp;
	auto rulers = list->get().All();
	for (auto ruler : rulers)
	{
		if (!ruler) continue;
		if (!ruler->GetDisp()) continue;
		int interp = ruler->GetInterp();

		for (int j = 0; j < ruler->GetNumBranch(); j++)
		for (int k = 0; k < ruler->GetNumBranchPoint(j); ++k)
		{
			point = ruler->GetRulerPoint(j, k);
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
	auto list = glbin_current.GetRulerList();
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

	std::shared_ptr<RulerPoint> point, ppmin;
	double dmin = std::numeric_limits<double>::max();
	double dist;
	fluo::Point ptemp;
	auto ruler_list = list->get().All();
	for (auto ruler : ruler_list)
	{
		if (!ruler) continue;
		if (!ruler->GetDisp()) continue;
		int interp = ruler->GetInterp();

		for (int j = 0; j < ruler->GetNumBranch(); j++)
		for (int k = 0; k < ruler->GetNumBranchPoint(j); ++k)
		{
			point = ruler->GetRulerPoint(j, k);
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
	auto list = glbin_current.GetRulerList();
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

	std::shared_ptr<RulerPoint> point, ppmin;
	std::shared_ptr<Ruler> rulermin;
	double dmin = std::numeric_limits<double>::max();
	double dist;
	fluo::Point ptemp;
	int rj = 0;
	auto ruler_list = list->get().All();
	for (auto ruler : ruler_list)
	{
		if (!ruler) continue;
		if (!ruler->GetDisp()) continue;
		int interp = ruler->GetInterp();

		for (int j = 0; j < ruler->GetNumBranch(); j++)
		{
			//if (ruler->GetNumBranchPoint(j) < 2)
			//	continue;
			point = ruler->GetRulerPoint(static_cast<int>(j), 0);
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
	auto list = glbin_current.GetRulerList();
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

	std::shared_ptr<RulerPoint> point, ppmin;
	std::shared_ptr<Ruler> rulermin;
	double dmin = std::numeric_limits<double>::max();
	double dist;
	size_t rj, rk;
	fluo::Point ptemp;
	auto ruler_list = list->get().All();
	for (auto ruler : ruler_list)
	{
		if (!ruler) continue;
		if (!ruler->GetDisp()) continue;
		int interp = ruler->GetInterp();

		for (int j = 0; j < ruler->GetNumBranch(); j++)
		{
			int nbp = ruler->GetNumBranchPoint(static_cast<int>(j));
			//if (nbp < 2)
			//	continue;
			for (int k = 0; k < nbp; ++k)
			{
				point = ruler->GetRulerPoint(static_cast<int>(j), static_cast<int>(k));
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
	auto list = glbin_current.GetRulerList();
	auto ruler = glbin_current.GetRuler();
	if (!ruler || !list)
		return;

	ruler->DeletePoint(m_point);
	m_point = nullptr;
	m_pindex = -1;
	if (!ruler->GetNumPoint())
	{
		//delete ruler
		auto ruler_list = list->get().All();
		for (auto r : ruler_list)
		{
			if (r == ruler)
			{
				list->get().RemoveByName(r->GetName());
				//m_ruler = 0;
				return;
			}
		}
	}
}

std::shared_ptr<RulerPoint> RulerHandler::GetEllipsePoint(int index)
{
	auto ruler = glbin_current.GetRuler();
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
	auto ruler = glbin_current.GetRuler();
	if (!ruler ||
		ruler->GetRulerMode() != RulerMode::Ellipse)
		return false;
	size_t rwt = ruler->GetWorkTime();
	int interp = ruler->GetInterp();
	auto p0 = m_point;
	auto p1 = GetEllipsePoint(1);
	auto p2 = GetEllipsePoint(2);
	auto p3 = GetEllipsePoint(3);
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
	auto ruler = glbin_current.GetRuler();
	if (!ruler)
		return;
	if (ruler->GetRulerMode() == RulerMode::Polyline)
		ruler->SetFinished();
	m_mouse = fluo::Point(-1);
}

bool RulerHandler::GetRulerFinished()
{
	auto ruler = glbin_current.GetRuler();
	if (ruler)
		return ruler->GetFinished();
	else
		return true;
}

std::shared_ptr<Ruler> RulerHandler::AddRuler(fluo::Point& p, size_t t)
{
	auto list = glbin_current.GetRulerList();
	if (!list)
		return 0;

	auto r = std::make_shared<Ruler>();
	r->SetWorkTime(t);
	r->Group(m_group);
	r->SetRulerMode(RulerMode::Locator);
	r->AddPoint(p);
	list->get().Add(r);
	return r;
}

void RulerHandler::AddRulerPoint(fluo::Point &p)
{
	auto list = glbin_current.GetRulerList();
	if (!list)
		return;

	auto ruler = glbin_current.GetRuler();
	if (ruler &&
		ruler->GetDisp() &&
		!ruler->GetFinished())
	{
		ruler->AddPoint(p);
	}
	else
	{
		ruler = std::make_shared<Ruler>();
		ruler->Group(m_group);
		ruler->SetRulerMode(m_mode);
		ruler->AddPoint(p);
		list->get().Add(ruler);
		auto view = glbin_current.render_view.lock();
		if (view)
			view->SetCurRuler(ruler);
	}

	Profile(ruler);
}

void RulerHandler::AddRulerPointAfterId(fluo::Point &p, unsigned int id,
	std::set<unsigned int> &cid, std::set<unsigned int> &bid)
{
	auto list = glbin_current.GetRulerList();
	if (!list)
		return;

	auto ruler = glbin_current.GetRuler();
	if (ruler &&
		ruler->GetDisp() &&
		ruler->GetRulerMode() == RulerMode::Polyline &&
		!ruler->GetFinished())
		ruler->AddPointAfterId(p, id, cid, bid);
	else
	{
		ruler = std::make_shared<Ruler>();
		ruler->Group(m_group);
		ruler->SetRulerMode(m_mode);
		ruler->AddPointAfterId(p, id, cid, bid);
		//ruler->SetTransient(m_view->m_ruler_time_dep);
		//ruler->SetTransTime(m_view->m_tseq_cur_num);
		list->get().Add(ruler);
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
	auto list = glbin_current.GetRulerList();
	if (!view || !list)
		return;

	int point_volume_mode = glbin_settings.m_point_volume_mode;
	size_t rwt = view->m_tseq_cur_num;
	auto ruler = glbin_current.GetRuler();
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
		auto ruler = std::make_shared<Ruler>();
		ruler->SetWorkTime(rwt);
		ruler->Group(m_group);
		ruler->SetRulerMode(m_mode);
		glbin_volume_point.SetVolumeData(glbin_current.vol_data.lock());
		glbin_volume_point.GetPointVolumeBoxTwoPoint(mx, my, p1, p2);
		ruler->AddPoint(p1);
		ruler->AddPoint(p2);
		list->get().Add(ruler);
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
				auto pp = ruler->GetLastRulerPoint();
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
				t = glbin_volume_point.GetPointPlane(mx, my, pplanep, p);
				if (t <= 0.0)
					return;
			}
		}
		else
		{
			double t = glbin_volume_point.GetPointPlane(mx, my, pplanep, p);
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
			ruler = std::make_shared<Ruler>();
			ruler->SetWorkTime(rwt);
			ruler->Group(m_group);
			ruler->SetRulerMode(m_mode);
			ruler->AddPoint(p);
			list->get().Add(ruler);
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
	auto ruler = glbin_current.GetRuler();
	auto list = glbin_current.GetRulerList();
	if (!list)
		return;

	flrd::Cov cover(vd);
	cover.SetUseMask(true);
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
		ruler = std::make_shared<Ruler>();
		ruler->Group(m_group);
		ruler->SetRulerMode(m_mode);
		ruler->AddPoint(center);
		str = "v0";
		ruler->AddInfoNames(str);
		str = std::to_string(static_cast<int>(size));
		ruler->AddInfoValues(str);
		list->get().Add(ruler);
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
	auto ruler = glbin_current.GetRuler();
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
			t = glbin_volume_point.GetPointPlane(mx, my, &tmp, point);
		}
		if (t <= 0.0)
			return false;
	}
	else
	{
		tmp = m_point->GetPoint(rwt, interp);
		double t = glbin_volume_point.GetPointPlane(mx, my, &tmp, point);
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
	auto ruler = glbin_current.GetRuler();
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
			t = glbin_volume_point.GetPointPlane(mx, my, &tmp, point);
		}
		if (t <= 0.0)
			return false;
	}
	else
	{
		tmp = m_point->GetPoint(rwt, interp);
		double t = glbin_volume_point.GetPointPlane(mx, my, &tmp, point);
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
	auto list = glbin_current.GetRulerList();
	if (!list)
		return;
	if (idx < 0 || idx >= list->get().size())
		return;
	if (len <= 0)
		return;
	auto ruler = list->get().GetRuler(idx);
	if (!ruler)
		return;

	ruler->Prune(len);
}

void RulerHandler::Flip(const std::set<int>& rulers)
{
	auto list = glbin_current.GetRulerList();
	if (!list)
		return;

	bool update_all = rulers.empty();

	size_t c = 0;
	auto ruler_list = list->get().All();
	for (auto i : ruler_list)
	{
		if (update_all || rulers.find(static_cast<int>(c)) != rulers.end())
			i->Reverse();
		c++;
	}
	m_edited = true;
}

void RulerHandler::AddAverage(const std::set<int>& rulers)
{
	auto list = glbin_current.GetRulerList();
	if (!list)
		return;

	bool update_all = rulers.empty();

	fluo::Point avg;
	size_t c = 0, count = 0;
	auto ruler_list = list->get().All();
	for (auto i : ruler_list)
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
	auto ruler = std::make_shared<Ruler>();
	ruler->SetRulerMode(RulerMode::Locator);
	ruler->SetName(L"Average");
	ruler->AddPoint(avg);
	ruler->SetTransient(false);
	ruler->SetTransTime(0);
	list->get().Add(ruler);
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
	auto ruler = glbin_current.GetRuler();
	if (!ruler)
		return;

	auto list = glbin_comp_analyzer.GetCelpList();
	double infr = glbin_settings.m_ruler_infr;
	int type = glbin_settings.m_ruler_relax_type;
	int iter = glbin_settings.m_ruler_relax_iter;
	double f1 = glbin_settings.m_ruler_relax_f1;
	glbin_dist_calculator.SetF1(f1);
	glbin_dist_calculator.SetInfr(infr);
	glbin_dist_calculator.SetCelpList(list);
	glbin_dist_calculator.SetVolume(glbin_current.vol_data.lock());
	glbin_dist_calculator.SetRuler(ruler);
	glbin_dist_calculator.CenterRuler(type, m_edited, iter);
	m_edited = false;
}

void RulerHandler::Relax(const std::set<int>& rulers)
{
	auto list = glbin_current.GetRulerList();
	if (!list)
		return;

	bool update_all = rulers.empty();

	auto cplist = glbin_comp_analyzer.GetCelpList();
	double infr = glbin_settings.m_ruler_infr;
	int type = glbin_settings.m_ruler_relax_type;
	int iter = glbin_settings.m_ruler_relax_iter;
	double f1 = glbin_settings.m_ruler_relax_f1;
	glbin_dist_calculator.SetF1(f1);
	glbin_dist_calculator.SetInfr(infr);
	glbin_dist_calculator.SetCelpList(cplist);
	glbin_dist_calculator.SetVolume(glbin_current.vol_data.lock());

	size_t c = 0;
	auto ruler_list = list->get().All();
	for (auto i : ruler_list)
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
	auto list = glbin_current.GetRulerList();
	if (!list)
		return;

	bool update_all = rulers.empty();

	size_t c = 0;
	auto ruler_list = list->get().All();
	for (auto i : ruler_list)
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
		auto temp = m_mag_ruler->GetRulerPoint(static_cast<int>(m_mag_branch), 0);
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
		auto temp = m_mag_ruler->GetRulerPoint(static_cast<int>(m_mag_branch), static_cast<int>(i));
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
			t = glbin_volume_point.GetPointPlane(mx, my, pplanep, p);
			if (t <= 0.0)
				return;
		}
	}
	else
	{
		double t = glbin_volume_point.GetPointPlane(mx, my, pplanep, p);
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
	auto list = glbin_current.GetRulerList();
	if (!list)
		return;

	auto ruler_list = list->get().All();
	for (auto it = ruler_list.rbegin();
		it != ruler_list.rend();)
	{
		auto it2 = std::next(it).base();
		int idx = static_cast<int>(it2 - ruler_list.begin());
		if (sel.find(idx) != sel.end())
		{
			it2 = ruler_list.erase(it2);
			it = std::reverse_iterator<decltype(ruler_list.begin())>(it2);
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
	auto list = glbin_current.GetRulerList();
	auto view = glbin_current.render_view.lock();
	if (!list || !view)
		return;

	auto ruler_list = list->get().All();
	if (cur_time)
	{
		int tseq = view->m_tseq_cur_num;
		for (size_t i = ruler_list.size(); i > 0; i--)
		{
			auto ruler = ruler_list[i-1];
			if (ruler &&
				((ruler->GetTransient() &&
					ruler->GetTransTime() == tseq) ||
					!ruler->GetTransient()))
			{
				ruler_list.erase(ruler_list.begin() + i - 1);
			}
		}
	}
	else
	{
		ruler_list.clear();
	}

	view->SetCurRuler(nullptr);
	m_point = nullptr;
	m_pindex = -1;
}

std::wstring RulerHandler::PrintRulers(bool h)
{
	std::wstring s;
	auto list = glbin_current.GetRulerList();
	if (!list)
		return s;

	auto ruler_list = list->get().All();
	for (auto ruler : ruler_list)
	{
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
	auto list = glbin_current.GetRulerList();
	if (!list)
		return;

	bool update_all = rulers.empty();

	size_t c = 0;
	auto ruler_list = list->get().All();
	for (auto i : ruler_list)
	{
		if (update_all || rulers.find(static_cast<int>(c)) != rulers.end())
			Profile(i);
		c++;
	}
}

int RulerHandler::Profile(const std::shared_ptr<Ruler>& ruler)
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

	auto spc = vd->GetSpacing();
	auto res = vd->GetResolution();
	if (spc.any_le_zero() ||
		res.any_le_zero())
		return 0;
	double scale = vd->GetScalarScale();
	//get data
	auto raw_data = vd->GetVolume(false);
	if (!raw_data) return 0;
	m_data = raw_data;
	//mask
	auto raw_mask = vd->GetMask(true);
	//set up sampler
	m_nx = res.intx(); m_ny = res.inty(); m_nz = res.intz();
	m_bits = vd->GetBits();
	m_scale = vd->GetScalarScale();
	ruler->SetScalarScale(m_bits == 8 ? 255: 65535);
	if (!valid()) return 0;

	if (ruler->GetRulerMode() == RulerMode::Probe &&
		raw_mask)
	{
		if (ruler->GetNumPoint() < 1)
			return 0;
		fluo::Point p1, p2;
		p1 = ruler->GetPoint(0);
		p2 = ruler->GetPoint(1);
		//object space
		p1 = fluo::Point(fluo::Vector(p1) / spc);
		p2 = fluo::Point(fluo::Vector(p2) / spc);
		fluo::Vector dir = p2 - p1;
		double dist = dir.length();
		if (dist < EPS)
			return 0;
		dir.normalize();

		//bin number
		int bins = int(dist / 1 + 0.5);
		if (bins <= 0) return 0;
		double bin_dist = dist / bins;
		auto& profile = ruler->GetProfile();
		profile.clear();
		profile.reserve(size_t(bins));
		for (unsigned int b = 0; b < static_cast<unsigned int>(bins); ++b)
			profile.push_back(flrd::ProfileBin());

		double brush_radius = ruler->GetBrushSize() + 1.0;
		const int nx = res.intx();
		const int ny = res.inty();
		const int nz = res.intz();
		const int64_t size_xy = static_cast<int64_t>(nx) * ny;

		raw_mask->ForEachElementIndexed(
			[&](auto mask_value, int64_t index)
			{
				// Skip masked-out voxels
				if (mask_value == 0)
					return;

				// Recover (i, j, k) from linear index
				int k = static_cast<int>(index / size_xy);
				int rem = static_cast<int>(index % size_xy);
				int j = rem / nx;
				int i = rem % nx;

				// Project voxel onto ruler direction
				fluo::Point p(i, j, k);
				fluo::Vector pdir = p - p1;
				double proj = fluo::Dot(pdir, dir);

				int bin_num = static_cast<int>(proj / bin_dist);
				if (bin_num < 0 || bin_num >= bins)
					return;

				// Check brush radius
				fluo::Point p_ruler = p1 + proj * dir;
				if ((p_ruler - p).length() > brush_radius)
					return;

				// Sample intensity
				double intensity = get_data(i, j, k);

				// Accumulate
				profile[bin_num].m_pixels++;
				profile[bin_num].m_accum += intensity;
			}
		);
	}
	else
	{
		//calculate length in object space
		double total_length = ruler->GetLengthObject(spc);
		int bins = int(total_length / m_step_length);
		auto& profile = ruler->GetProfile();
		profile.clear();

		//sample data through ruler
		fluo::Point p;
		double intensity;
		if (bins == 0)
		{
			//allocate
			profile.reserve(size_t(1));
			profile.push_back(flrd::ProfileBin());

			p = ruler->GetPointTransformed(0);
			//object space
			p = fluo::Point(fluo::Vector(p) / spc);
			intensity = get_filtered_data(p.x(), p.y(), p.z());
			profile[0].m_pixels++;
			profile[0].m_accum += intensity;
			profile[0].dist = 0;
		}
		else
		{
			//allocate
			profile.reserve(size_t(bins));
			for (unsigned int b = 0; b < static_cast<unsigned int>(bins); ++b)
				profile.push_back(flrd::ProfileBin());

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
				p1 = fluo::Point(fluo::Vector(p1) / spc);
				p2 = fluo::Point(fluo::Vector(p2) / spc);
				dir = p2 - p1;
				dist = dir.length();
				dir.normalize();
				dir2 = dir * m_step_length;
				dir2 *= spc;
				real_step = dir2.length();

				for (double dn = 0; dn < dist; dn += m_step_length)
				{
					p = p1 + dir * dn;
					intensity = get_filtered_data(p.x(), p.y(), p.z());
					if (total_dist >= bins) break;
					profile[total_dist].m_pixels++;
					profile[total_dist].m_accum += intensity;
					profile[total_dist].dist = real_dist;
					real_dist += real_step;
					total_dist++;
				}
			}
			if (total_dist < bins)
				profile.erase(profile.begin() + total_dist, profile.begin() + bins - 1);
		}
	}
	std::string str = "Profile of volume ";
	str = str + ws2s(vd->GetName());
	ruler->SetInfoProfile(str);

	return 1;
}

int RulerHandler::Profile(int index)
{
	auto list = glbin_current.GetRulerList();
	if (!list)
		return 0;
	if (index < 0 ||
		index >= list->get().size())
		return 0;
	auto ruler = list->get().GetRuler(index);
	return Profile(ruler);
}

int RulerHandler::ProfileAll()
{
	auto list = glbin_current.GetRulerList();
	if (!list)
		return 0;
	int c = 0;
	auto ruler_list = list->get().All();
	for (auto ruler : ruler_list)
		c += Profile(ruler);
	return c;
}

int RulerHandler::Roi(const std::shared_ptr<Ruler>& ruler)
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

	auto spc = vd->GetSpacing();
	auto res = vd->GetResolution();
	if (spc.any_le_zero() ||
		res.any_le_zero())
		return 0;

	double scale = vd->GetScalarScale();
	//get data
	auto tex = vd->GetTexture();
	if (!tex) return 0;
	auto raw_data = vd->GetVolume(false);
	m_data = raw_data;
	//mask
	auto raw_mask = vd->GetMask(true);
	//set up sampler
	m_nx = res.intx(); m_ny = res.inty(); m_nz = res.intz();
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
	VolumeRoi vr(vd);
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
	auto list = glbin_current.GetRulerList();
	if (!list)
		return 0;
	if (index < 0 ||
		index >= list->get().size())
		return 0;
	auto ruler = list->get().GetRuler(index);
	return Roi(ruler);
}

int RulerHandler::RoiAll()
{
	auto list = glbin_current.GetRulerList();
	if (!list)
		return 0;
	int c = 0;
	auto ruler_list = list->get().All();
	for (auto ruler : ruler_list)
		c += Roi(ruler);
	return c;
}

void RulerHandler::Distance(const std::set<int>& rulers, const std::wstring& filename)
{
	auto list = glbin_current.GetRulerList();
	if (!list)
	{
		return;
	}
	auto cplist = glbin_comp_analyzer.GetCelpList();
	if (cplist->get().empty())
	{
		return;
	}

	bool update_all = rulers.empty();

	std::wstring str = filename;
	str = str.substr(0, str.find_last_of(L'.'));
	std::wstring fi;

	fluo::Vector scale = cplist->get().scale;

	size_t c = 0;
	auto ruler_list = list->get().All();
	for (auto ruler : ruler_list)
	{
		if (update_all || rulers.find(static_cast<int>(c)) != rulers.end())
		{
			if (!ruler)
				continue;
			if (ruler->GetNumPoint() < 1)
				continue;

			fluo::Point p = ruler->GetCenter();
			for (auto it = cplist->get().begin();
				it != cplist->get().end(); ++it)
			{
				double dist = (p - it->second->GetCenter(scale)).length();
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
	auto list = glbin_current.GetRulerList();
	if (!list)
	{
		return;
	}
	auto cplist = glbin_comp_analyzer.GetCelpList();
	if (cplist->get().empty())
	{
		return;
	}
	glbin_dist_calculator.SetCelpList(cplist);

	bool update_all = rulers.empty();

	std::wstring str = filename;
	std::ofstream ofs;
	ofs.open(str, std::ofstream::out);

	size_t c = 0;
	auto ruler_list = list->get().All();
	for (auto ruler : ruler_list)
	{
		if (update_all || rulers.find(static_cast<int>(c)) != rulers.end())
		{
			glbin_dist_calculator.SetRuler(ruler);
			glbin_dist_calculator.Project();

			std::vector<flrd::Celp> comps;
			for (auto it = cplist->get().begin();
				it != cplist->get().end(); ++it)
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
	auto list = glbin_current.GetRulerList();
	if (!list)
		return;

	bool update_all = rulers.empty();

	size_t t = 0;
	if (auto cur_view = glbin_current.render_view.lock())
		t = cur_view->m_tseq_cur_num;

	size_t c = 0;
	auto ruler_list = list->get().All();
	for (auto ruler : ruler_list)
	{
		if (update_all || rulers.find(static_cast<int>(c)) != rulers.end())
		{
			if (!ruler)
				continue;
			ruler->SetTransient(bval);
			if (bval)
				ruler->SetTransTime(t);
		}
		c++;
	}
}

void RulerHandler::SetDisplay(bool bval, const std::set<int>& rulers, int type)
{
	auto list = glbin_current.GetRulerList();
	if (!list)
		return;

	bool update_all = rulers.empty();

	size_t c = 0;
	auto ruler_list = list->get().All();
	for (auto ruler : ruler_list)
	{
		if (update_all || rulers.find(static_cast<int>(c)) != rulers.end())
		{
			if (!ruler)
				continue;
			ruler->SetDisplay(type, bval);
		}
		c++;
	}
}

void RulerHandler::SetInterp(int ival, const std::set<int>& rulers)
{
	auto list = glbin_current.GetRulerList();
	if (!list)
		return;

	bool update_all = rulers.empty();

	size_t c = 0;
	auto ruler_list = list->get().All();
	for (auto ruler : ruler_list)
	{
		if (update_all || rulers.find(static_cast<int>(c)) != rulers.end())
		{
			if (!ruler)
				continue;
			ruler->SetInterp(ival);
		}
		c++;
	}
}

void RulerHandler::DeleteKey(const std::set<int>& rulers)
{
	auto list = glbin_current.GetRulerList();
	if (!list)
		return;

	bool update_all = rulers.empty();

	size_t t = 0;
	if (auto cur_view = glbin_current.render_view.lock())
		t = cur_view->m_tseq_cur_num;

	size_t c = 0;
	auto ruler_list = list->get().All();
	for (auto ruler : ruler_list)
	{
		if (update_all || rulers.find(static_cast<int>(c)) != rulers.end())
		{
			if (!ruler)
				continue;
			ruler->SetWorkTime(t);
			ruler->DeleteKey();
		}
		c++;
	}
}

void RulerHandler::DeleteAllKeys(const std::set<int>& rulers)
{
	auto list = glbin_current.GetRulerList();
	if (!list)
		return;

	bool update_all = rulers.empty();

	size_t t = 0;
	if (auto cur_view = glbin_current.render_view.lock())
		t = cur_view->m_tseq_cur_num;

	size_t c = 0;
	auto ruler_list = list->get().All();
	for (auto ruler : ruler_list)
	{
		if (update_all || rulers.find(static_cast<int>(c)) != rulers.end())
		{
			if (!ruler)
				continue;
			ruler->SetWorkTime(t);
			ruler->DeleteAllKey();
		}
		c++;
	}
}

//get time points where keys exist
bool RulerHandler::GetKeyFrames(std::set<size_t>& kf)
{
	auto view = glbin_current.render_view.lock();
	auto list = glbin_current.GetRulerList();
	if (!view || !list)
		return false;
	size_t startf, endf;
	startf = view->m_begin_frame;
	endf = view->m_end_frame;

	if (!list)
		return false;
	auto ruler_list = list->get().All();
	if (ruler_list.empty())
		return false;
	kf.clear();

	for (auto ruler : ruler_list)
	{
		if (!ruler)
			continue;
		for (size_t j = 0; j < ruler->GetNumPoint(); ++j)
		{
			auto p = ruler->GetRulerPoint(static_cast<int>(j));
			if (!p)
				continue;
			size_t tn = p->GetTimeNum();
			for (size_t k = 0; k < tn; ++k)
			{
				auto [t, pp] = p->GetTimeAndPoint(k);
				if (t > startf && t <= endf)
					kf.insert(t);
			}
		}
	}
	return !kf.empty();
}

size_t RulerHandler::GetRulerPointNum()
{
	auto list = glbin_current.GetRulerList();
	if (!list)
		return 0;

	auto ruler_list = list->get().All();
	if (ruler_list.empty())
		return 0;

	size_t sum = 0;
	for (auto ruler : ruler_list)
	{
		if (!ruler)
			continue;
		sum += ruler->GetNumPoint();
	}
	return sum;
}

bool RulerHandler::GetRulerPointNames(std::vector<std::wstring>& names)
{
	auto list = glbin_current.GetRulerList();
	if (!list)
		return 0;

	auto ruler_list = list->get().All();
	if (ruler_list.empty())
		return 0;

	names.clear();
	for (auto ruler : ruler_list)
	{
		if (!ruler)
			continue;
		int rpn = ruler->GetNumPoint();
		std::wstring name = ruler->GetName();
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
	auto list = glbin_current.GetRulerList();
	if (!list)
		return 0;

	auto ruler_list = list->get().All();
	if (ruler_list.empty())
		return 0;

	names.clear();
	for (auto ruler : ruler_list)
	{
		if (!ruler)
			continue;
		int rpn = ruler->GetNumPoint();
		std::wstring name = ruler->GetName();
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
	auto list = glbin_current.GetRulerList();
	if (!list)
		return 0;

	auto ruler_list = list->get().All();
	if (ruler_list.empty())
		return 0;

	std::set<size_t> kf;
	GetKeyFrames(kf);
	if (kf.empty())
		return false;

	coords.clear();
	for (auto t : kf)
	{
		for (auto ruler : ruler_list)
		{
			if (!ruler)
				continue;
			ruler->SetWorkTime(t);
			for (int k = 0; k < ruler->GetNumPoint(); ++k)
			{
				fluo::Point p = ruler->GetPoint(k);
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
	auto list = glbin_current.GetRulerList();
	if (!view || !list)
		return nullptr;

	size_t rwt = view->m_tseq_cur_num;
	double dmin = std::numeric_limits<double>::max();
	RulerPoint* result = 0;

	size_t ri, rj;
	auto ruler_list = list->get().All();
	for (auto ruler : ruler_list)
	{
		ruler->SetWorkTime(rwt);
		int interp = ruler->GetInterp();
		auto temp = ruler->FindNearestRulerPoint(p, ri, rj);
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
	auto list = glbin_current.GetRulerList();
	if (!view || !list)
		return;

	auto ruler_list = list->get().All();
	if (ruler_list.empty())
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
			auto ruler = list->get().FindByName(name.s);
			if (!ruler)
				continue;
			int dim = static_cast<int>(name.d);
			//compute integral for each point
			for (size_t j = 0; j < ruler->GetNumPoint(); ++j)
			{
				ruler->SetWorkTime(i);
				fluo::Point p = ruler->GetPoint(static_cast<int>(j));
				double x, y, z;
				x = dim > 0 ? data.get(cnt++, f) : 0;
				y = dim > 1 ? data.get(cnt++, f) : 0;
				z = dim > 2 ? data.get(cnt++, f) : 0;
				fluo::Vector v(x, y, z);
				p += v * dir;
				ruler->SetWorkTime(t);
				ruler->SetPoint(static_cast<int>(j), p);
			}
		}
	}
}

//get values for gradient
std::pair<bool, fluo::Point> RulerHandler::GetCenter()
{
	fluo::Point p;
	auto ruler = glbin_current.GetRuler();
	if (ruler)
	{
		p = ruler->GetCenter();
		return { true, p };
	}
	return { false, p };
}

std::pair<bool, double> RulerHandler::GetRadius()
{
	double r = 0;
	auto ruler = glbin_current.GetRuler();
	if (ruler)
	{
		auto box = ruler->GetBounds();
		if (box.valid())
			r = box.diagonal().length() / 2.0;
		if (r > 0.0)
			return { true, r };
	}
	return { false, r };
}

std::pair<bool, std::pair<fluo::Plane, fluo::Plane>> RulerHandler::GetLinearPlanes()
{
	fluo::Plane p0, p1;
	auto ruler = glbin_current.GetRuler();
	if (ruler)
	{
		int num = ruler->GetNumPoint();
		if (num == 2)
		{
			//use points
			fluo::Point pp0, pp1;
			pp0 = ruler->GetPoint(0);
			pp1 = ruler->GetPoint(1);
			fluo::Vector d = pp1 - pp0;
			d.normalize();
			p0 = fluo::Plane(pp0, d);
			p1 = fluo::Plane(pp1, d);
			return { true, { p0, p1 } };
		}
		else if (num > 2)
		{
			//use pca
			Pca solver;
			std::vector<fluo::Point> list;
			for (size_t i = 0; i < ruler->GetNumPoint(); ++i)
			{
				fluo::Point p;
				if (ruler->GetPoint(static_cast<int>(i), p))
					list.push_back(p);
			}
			solver.SetPoints(list);
			solver.Compute();
			auto axis = solver.GetAxis(0);
			axis.normalize();
			double l = solver.GetLengths().x();
			auto center = ruler->GetCenter();
			fluo::Point pp0, pp1;
			pp0 = center - axis * l / 2.0;
			pp1 = pp0 + axis * l;
			fluo::Vector d = pp1 - pp0;
			d.normalize();
			p0 = fluo::Plane(pp0, d);
			p1 = fluo::Plane(pp1, d);
			return { true, { p0, p1 } };
		}
	}
	return { false, { p0, p1 } };
}