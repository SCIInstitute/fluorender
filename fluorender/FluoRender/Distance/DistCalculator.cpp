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
#include "DistCalculator.h"
#include <VolumeData.hpp>
#include <limits>
#include <algorithm>

using namespace flrd;

DistCalculator::DistCalculator() :
	m_type(0),
	m_init(false),
	m_vd(0),
	m_celps(0),
	m_ruler(0)
{
	m_f1 = 1;
	m_f2 = 2;
	m_f3 = 3;
	m_infr = 2.5;
}

DistCalculator::~DistCalculator()
{
}

void DistCalculator::CenterRuler(int type, bool init, int iter)
{
	m_type = type;
	
	if (m_type == 1)
		m_relax.SetUseMask(false);
	else if (m_type == 2)
		m_relax.SetUseMask(true);
	if (m_type != 3)
		iter = std::max(1, iter / 10);

	if (!m_init || init)
	{
		BuildSpring();
		if (m_type == 3)
			BuildCloud();
		m_rest = GetRestDist();
		m_relax.SetRestDist(m_rest);
		m_relax.SetInflRange(m_rest*m_infr*5);
		m_init = true;
	}

	for (int it = 0; it < iter; ++it)
	{
		if (m_type == 1 || m_type == 2)
			m_relax.Compute();
		for (int i = 0; i < m_spring.size(); ++i)
			UpdateSpringNode(i);
	}
}

void DistCalculator::Project()
{
	if (!m_celps)
		return;
	if (!m_init)
		BuildSpring();
	else
		UpdateSpringDist();
	if (m_spring.empty())
		return;

	double sx = m_celps->sx;
	double sy = m_celps->sy;
	double sz = m_celps->sz;

	fluo::Point p0, pp;
	for (auto it = m_celps->begin();
		it != m_celps->end(); ++it)
	{
		p0 = it->second->GetCenter(sx, sy, sz);
		SpringProject(p0, pp);
		it->second->SetProjp(pp);
	}
}

void DistCalculator::BuildSpring()
{
	if (!m_ruler)
		return;
	int rn = m_ruler->GetNumPoint();
	if (rn < 1)
		return;

	if (!m_spring.empty())
		m_spring.clear();

	//build a spring form the ruler
	double dist;
	int bn = m_ruler->GetNumBranch();
	for (int bi = 0; bi < bn; ++bi)
	{
		rn = m_ruler->GetNumBranchPoint(bi);
		int n = rn == 1 ? 1 : rn - 1;
		for (int i = 0; i < n; ++i)
		{
			if (i == 0)
			{
				SpringNode node;
				node.p = m_ruler->GetPoint(bi, i);
				node.prevd = 0.0;
				node.nextd = 0.0;
				node.dist = 0.0;
				m_spring.push_back(node);
			}

			if (rn > 1)
			{
				SpringNode &node1 = m_spring.back();
				SpringNode node2;
				node2.p = m_ruler->GetPoint(bi, i + 1);
				dist = (node2.p->GetPoint() - node1.p->GetPoint()).length();
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
	if (!m_celps)
		return;
	if (m_celps->empty())
		return;
	if (!m_cloud.empty())
		m_cloud.clear();

	double sx = m_celps->sx;
	double sy = m_celps->sy;
	double sz = m_celps->sz;

	fluo::Point p;
	for (auto it = m_celps->begin();
		it != m_celps->end(); ++it)
	{
		p = it->second->GetCenter(sx, sy, sz);
		m_cloud.push_back(p);
	}
}

double DistCalculator::GetRestDist()
{
	int size = m_cloud.size();
	if (size < 2)
		return 1.0;

	int num = size * (size - 1) / 2;
	double len;
	double sumd = 0.0;
	double mind = std::numeric_limits<double>::max();
	fluo::Point p1, p2;
	for (int i = 0; i < size - 1; ++i)
	for (int j = i + 1; j < size - 1; ++j)
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
	int sz = m_spring.size();
	int cz = m_cloud.size();
	if (idx < 0 || idx >= sz)
		return;
	SpringNode& node = m_spring.at(idx);
	if (node.p->GetLocked())
		return;
	fluo::Point pos = node.p->GetPoint();
	fluo::Vector force, f1, f2, f3;

	double dist, ang;
	fluo::Vector dir, dir2;
	if (m_type == 1 || m_type == 2)
	{
		//from relax
		f1 = m_relax.GetDisplacement(idx);
	}
	else if (m_type == 3)
	{
		//from cloud
		std::vector<double> lens;
		for (int i = 0; i < cz; ++i)
		{
			dir = m_cloud[i] - pos;
			lens.push_back(dir.length());
		}
		std::sort(lens.begin(), lens.end());
		double scale = (node.prevd == 0.0 ||
			node.nextd == 0.0) ? 1.0 : m_infr;
		int loc = int(scale * cz / sz + 1.0);
		loc = std::min(loc, cz - 1);
		for (int i = 0; i < cz; ++i)
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
		dir = prev.p->GetPoint() - pos;
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
		dir = next.p->GetPoint() - pos;
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
		dir = prev.p->GetPoint() - pos;
		SpringNode& next = m_spring.at(idx + 1);
		dir2 = next.p->GetPoint() - pos;
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
	node.p->DisplacePoint(force);
}

void DistCalculator::UpdateSpringDist()
{
	if (m_spring.empty())
		return;

	double dist;
	fluo::Vector dir;
	for (int i = 0; i < m_spring.size(); ++i)
	{
		SpringNode& node = m_spring.at(i);
		if (node.prevd > 0.0)
		{
			SpringNode& prev = m_spring.at(i - 1);
			dir = node.p->GetPoint() - prev.p->GetPoint();
			dist = dir.length();
			node.dist = prev.dist + dist;
		}
	}
}

void DistCalculator::SpringProject(fluo::Point &p0, fluo::Point &pp)
{
	if (m_spring.empty())
		return;
	int sz = m_spring.size();
	if (sz < 2)
	{
		pp = fluo::Point(p0 - m_spring[0].p->GetPoint());
		return;
	}

	//find closest segment
	int minidx = -1;
	double dist;
	double mind = std::numeric_limits<double>::max();
	fluo::Point mp;
	for (int i = 0; i < sz - 1; ++i)
	{
		SpringNode &node1 = m_spring.at(i);
		SpringNode &node2 = m_spring.at(i + 1);
		mp = fluo::Point((node1.p->GetPoint() + node2.p->GetPoint()) / 2.0);
		dist = (mp - p0).length2();
		if (dist < mind)
		{
			mind = dist;
			minidx = i;
		}
	}

	if (minidx < 0 || minidx >= sz - 1)
		return;

	//project
	fluo::Point p1 = m_spring[minidx].p->GetPoint();
	fluo::Point p2 = m_spring[minidx + 1].p->GetPoint();
	fluo::Vector axis = p2 - p1;
	axis.normalize();
	fluo::Vector vp0 = p0 - p1;
	double len = vp0.length();
	double ppx = fluo::Dot(vp0, axis);
	double ppy = std::sqrt(len * len - ppx * ppx);
	pp = fluo::Point(ppx + m_spring[minidx].dist, ppy, minidx);
}