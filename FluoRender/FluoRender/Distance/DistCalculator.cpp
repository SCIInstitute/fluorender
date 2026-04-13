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
#include <DistCalculator.h>
#include <VolumeData.h>
#include <Cell.h>
#include <Ruler.h>
#include <Relax.h>
#include <limits>
#include <algorithm>

using namespace flrd;

DistCalculator::DistCalculator() :
	m_type(0),
	m_init(false)
{
	m_f1 = 1;
	m_f2 = 2;
	m_f3 = 3;
	m_infr = 2.5;

	m_relax = std::make_unique<Relax>();
}

DistCalculator::~DistCalculator()
{
}

void DistCalculator::SetRuler(const std::shared_ptr<Ruler>& ruler)
{
	if (ruler && ruler != m_ruler.lock())
	{
		m_ruler = ruler;
		m_init = false;
	}
	m_relax->SetRuler(ruler);
}

std::shared_ptr<Ruler> DistCalculator::GetRuler()
{
	return m_ruler.lock();
}

void DistCalculator::SetCelpList(const std::shared_ptr<CelpList>& list)
{
	if (list && list != m_celps.lock())
	{
		m_celps = list;
		m_init = false;
	}
}

std::shared_ptr<CelpList> DistCalculator::GetCelpList()
{
	return m_celps.lock();
}

void DistCalculator::SetVolume(const std::shared_ptr<VolumeData>& vd)
{
	if (vd && vd != m_vd.lock())
	{
		m_vd = vd;
		m_init = false;
	}
	m_relax->SetVolume(vd);
}

void DistCalculator::CenterRuler(int type, bool init, int iter)
{
	m_type = type;
	
	if (m_type == 1)
		m_relax->SetUseMask(false);
	else if (m_type == 2)
		m_relax->SetUseMask(true);
	if (m_type != 3)
		iter = std::max(1, iter / 10);

	if (!m_init || init)
	{
		BuildSpring();
		if (m_type == 3)
			BuildCloud();
		m_rest = GetRestDist();
		m_relax->SetRestDist(static_cast<float>(m_rest));
		m_relax->SetInflRange(static_cast<float>(m_rest*m_infr*5));
		m_init = true;
	}

	for (int it = 0; it < iter; ++it)
	{
		if (m_type == 1 || m_type == 2)
			m_relax->Compute();
		for (int i = 0; i < m_spring.size(); ++i)
			UpdateSpringNode(i);
	}
}

void DistCalculator::Project()
{
	auto celp_list = m_celps.lock();
	if (!celp_list)
		return;
	if (!m_init)
		BuildSpring();
	else
		UpdateSpringDist();
	if (m_spring.empty())
		return;

	fluo::Vector scale = celp_list->scale;

	fluo::Point p0, pp;
	for (auto it = celp_list->begin();
		it != celp_list->end(); ++it)
	{
		p0 = it->second->GetCenter(scale);
		SpringProject(p0, pp);
		it->second->SetProjp(pp);
	}
}

void DistCalculator::BuildSpring()
{
	auto ruler = m_ruler.lock();
	if (!ruler)
		return;
	size_t rwt = ruler->GetWorkTime();
	int interp = ruler->GetInterp();
	int rn = ruler->GetNumPoint();
	if (rn < 1)
		return;

	if (!m_spring.empty())
		m_spring.clear();

	//build a spring form the ruler
	double dist;
	int bn = ruler->GetNumBranch();
	for (int bi = 0; bi < bn; ++bi)
	{
		rn = ruler->GetNumBranchPoint(bi);
		int n = rn == 1 ? 1 : rn - 1;
		for (int i = 0; i < n; ++i)
		{
			if (i == 0)
			{
				SpringNode node;
				node.p = ruler->GetRulerPoint(bi, i);
				node.prevd = 0.0;
				node.nextd = 0.0;
				node.dist = 0.0;
				m_spring.push_back(node);
			}

			if (rn > 1)
			{
				SpringNode &node1 = m_spring.back();
				SpringNode node2;
				node2.p = ruler->GetRulerPoint(bi, i + 1);
				auto n1p = node1.p.lock();
				auto n2p = node2.p.lock();
				if (!n1p || !n2p)
					continue;
				dist = (n2p->GetPoint(rwt, interp) -
					n1p->GetPoint(rwt, interp)).length();
				node1.nextd = dist;
				node2.prevd = dist;
				node2.nextd = 0.0;
				node2.dist = node1.dist + dist;
				m_spring.push_back(node2);
			}
		}
	}
}

void DistCalculator::BuildCloud()
{
	auto celps = m_celps.lock();
	if (!celps)
		return;
	if (celps->empty())
		return;
	if (!m_cloud.empty())
		m_cloud.clear();

	auto scale(celps->scale);

	fluo::Point p;
	for (auto it = celps->begin();
		it != celps->end(); ++it)
	{
		p = it->second->GetCenter(scale);
		m_cloud.push_back(p);
	}
}

double DistCalculator::GetRestDist()
{
	size_t size = m_cloud.size();
	if (size < 2)
		return 1.0;

	size_t num = size * (size - 1) / 2;
	double len;
	double sumd = 0.0;
	double mind = std::numeric_limits<double>::max();
	fluo::Point p1, p2;
	for (size_t i = 0; i < size - 1; ++i)
	for (size_t j = i + 1; j < size - 1; ++j)
	{
		p1 = m_cloud[i];
		p2 = m_cloud[j];
		len = (p1 - p2).length();
		sumd += len;
		mind = std::min(mind, len);
	}

	//return sumd / num / 2.0;
	//return mind * 2.0;
	return (mind + sumd / num) / 4.0;
}

void DistCalculator::UpdateSpringNode(int idx)
{
	auto ruler = m_ruler.lock();
	if (!ruler)
		return;
	size_t rwt = ruler->GetWorkTime();
	int interp = ruler->GetInterp();
	size_t sz = m_spring.size();
	size_t cz = m_cloud.size();
	if (idx < 0 || idx >= static_cast<int>(sz))
		return;
	SpringNode& node = m_spring.at(idx);
	auto node_p = node.p.lock();
	if (!node_p || node_p->GetLocked())
		return;
	auto pos = node_p->GetPoint(rwt, interp);
	fluo::Vector force, f1, f2, f3;

	double dist, ang;
	fluo::Vector dir, dir2;
	if (m_type == 1 || m_type == 2)
	{
		//from relax
		f1 = m_relax->GetDisplacement(idx);
	}
	else if (m_type == 3)
	{
		//from cloud
		std::vector<double> lens;
		for (size_t i = 0; i < cz; ++i)
		{
			dir = m_cloud[i] - pos;
			lens.push_back(dir.length());
		}
		std::sort(lens.begin(), lens.end());
		double scale = (node.prevd == 0.0 ||
			node.nextd == 0.0) ? 1.0 : m_infr;
		size_t loc = size_t(scale * cz / sz + 1.0);
		loc = std::min(loc, cz - 1);
		for (size_t i = 0; i < cz; ++i)
		{
			dir = m_cloud[i] - pos;
			dist = dir.length();
			if (dist > lens[loc])
				continue;
			dist = std::max(m_rest, dist);
			dir.normalize();
			f1 += dir / dist / dist;
		}
	}
	//from neighbors
	if (idx > 0 && node.prevd > 0.0)
	{
		SpringNode& prev = m_spring.at(idx - 1);
		auto prev_p = prev.p.lock();
		if (!prev_p)
			return;
		dir = prev_p->GetPoint(rwt, interp) - pos;
		dist = dir.length();
		dir.normalize();
		if (node.nextd == 0.0)
			f2 += dir * (dist - node.prevd) * 0.2;
		else
			f2 += dir * (dist - node.prevd);
	}
	if (idx < sz - 1 && node.nextd > 0.0)
	{
		SpringNode& next = m_spring.at(idx + 1);
		auto next_p = next.p.lock();
		if (!next_p)
			return;
		dir = next_p->GetPoint(rwt, interp) - pos;
		dist = dir.length();
		dir.normalize();
		if (node.prevd == 0.0)
			f2 += dir * (dist - node.nextd) * 0.2;
		else
			f2 += dir * (dist - node.nextd);
	}
	//angular
	if (idx > 0 && idx < sz - 1 &&
		node.prevd > 0.0 && node.nextd > 0.0)
	{
		SpringNode& prev = m_spring.at(idx - 1);
		auto prev_p = prev.p.lock();
		if (!prev_p)
			return;
		dir = prev_p->GetPoint(rwt, interp) - pos;
		SpringNode& next = m_spring.at(idx + 1);
		auto next_p = next.p.lock();
		if (!next_p)
			return;
		dir2 = next_p->GetPoint(rwt, interp) - pos;
		dir.normalize();
		dir2.normalize();
		ang = Dot(dir, dir2)+1.0;
		dir = dir + dir2;
		dir.normalize();
		f3 += dir * ang;
	}

	double ff1, ff2, ff3;
	if (m_type != 3)
	{
		ff1 = m_f1;
		ff2 = m_f2;
		ff3 = m_f3;
	}
	else
	{
		ff1 = m_f1;
		ff2 = m_f2;
		ff3 = m_f3;
	}
	double norm = (node.prevd + node.nextd) / 100.0;
	norm = std::max(1 / 100.0, norm);
	f1.normalize();
	f1 *= norm;
	f2 = ff2 * f2 + ff3 * f3;
	f2.normalize();
	f2 *= norm * 2.0;
	//f3.normalize();
	//f3 *= norm;
	force = ff1 * f1 + ff2 * f2;
	node_p->DisplacePoint(force, rwt, interp);
}

void DistCalculator::UpdateSpringDist()
{
	if (m_spring.empty())
		return;
	auto ruler = m_ruler.lock();
	if (!ruler)
		return;
	size_t rwt = ruler->GetWorkTime();
	int interp = ruler->GetInterp();

	double dist;
	fluo::Vector dir;
	for (int i = 0; i < m_spring.size(); ++i)
	{
		SpringNode& node = m_spring.at(i);
		if (node.prevd > 0.0)
		{
			SpringNode& prev = m_spring.at(i - 1);
			auto node_p = node.p.lock();
			auto prev_p = prev.p.lock();
			if (!node_p || !prev_p)
				continue;
			dir = node_p->GetPoint(rwt, interp) -
				prev_p->GetPoint(rwt, interp);
			dist = dir.length();
			node.dist = prev.dist + dist;
		}
	}
}

void DistCalculator::SpringProject(fluo::Point &p0, fluo::Point &pp)
{
	if (m_spring.empty())
		return;
	auto ruler = m_ruler.lock();
	if (!ruler)
		return;
	size_t rwt = ruler->GetWorkTime();
	int interp = ruler->GetInterp();
	size_t sz = m_spring.size();
	if (sz < 2)
	{
		auto p = m_spring[0].p.lock();
		if (!p)
			return;
		pp = fluo::Point(p0 -
			p->GetPoint(rwt, interp));
		return;
	}

	//find closest segment
	int minidx = -1;
	double dist;
	double mind = std::numeric_limits<double>::max();
	fluo::Point mp;
	for (size_t i = 0; i < sz - 1; ++i)
	{
		SpringNode &node1 = m_spring.at(i);
		SpringNode &node2 = m_spring.at(i + 1);
		auto p1 = node1.p.lock();
		auto p2 = node2.p.lock();
		if (!p1 || !p2)
			continue;
		mp = fluo::Point((p1->GetPoint(rwt, interp) +
			p2->GetPoint(rwt, interp)) / 2.0);
		dist = (mp - p0).length2();
		if (dist < mind)
		{
			mind = dist;
			minidx = static_cast<int>(i);
		}
	}

	if (minidx < 0 || minidx >= sz - 1)
		return;

	//project
	auto sp1 = m_spring[minidx].p.lock();
	auto sp2 = m_spring[minidx + 1].p.lock();
	if (!sp1 || !sp2)
		return;
	fluo::Point p1 = sp1->GetPoint(rwt, interp);
	fluo::Point p2 = sp2->GetPoint(rwt, interp);
	fluo::Vector axis = p2 - p1;
	axis.normalize();
	fluo::Vector vp0 = p0 - p1;
	double len = vp0.length();
	double ppx = fluo::Dot(vp0, axis);
	double ppy = std::sqrt(len * len - ppx * ppx);
	pp = fluo::Point(ppx + m_spring[minidx].dist, ppy, minidx);
}