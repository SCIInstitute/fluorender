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
#include <limits>
#include <algorithm>

using namespace FL;

DistCalculator::DistCalculator() :
	m_init(false),
	m_comp_list(0),
	m_ruler(0)
{
	m_f1 = 1;
	m_f2 = 2;
	m_f3 = 3;
}

DistCalculator::~DistCalculator()
{
}

void DistCalculator::CenterRuler(bool init, int iter)
{
	if (!m_init || init)
	{
		BuildSpring();
		BuildCloud();
		m_rest = GetRestDist();
		m_init = true;
	}

	for (int it = 0; it < iter; ++it)
	for (int i = 0; i < m_spring.size(); ++i)
		UpdateSpringNode(i);

	UpdateRuler();
}

void DistCalculator::Project()
{
	if (!m_comp_list)
		return;
	BuildSpring();
	if (m_spring.empty())
		return;

	double sx = m_comp_list->sx;
	double sy = m_comp_list->sy;
	double sz = m_comp_list->sz;

	Point p0, pp;
	for (auto it = m_comp_list->begin();
		it != m_comp_list->end(); ++it)
	{
		p0 = it->second->GetPos(sx, sy, sz);
		SpringProject(p0, pp);
		it->second->proj = pp;
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
	int n = rn == 1 ? 1 : rn - 1;
	for (int i = 0; i < n; ++i)
	{
		if (i == 0)
		{
			SpringNode node;
			node.p = m_ruler->GetPoint(i)->GetPoint();
			node.prevd = 0.0;
			node.nextd = 0.0;
			node.dist = 0.0;
			m_spring.push_back(node);
		}
		
		if (rn > 1)
		{
			SpringNode &node1 = m_spring.at(i);
			SpringNode node2;
			node2.p = m_ruler->GetPoint(i + 1)->GetPoint();
			dist = (node2.p - node1.p).length();
			node1.nextd = dist;
			node2.prevd = dist;
			node2.nextd = 0.0;
			node2.dist = node1.dist + dist;
			m_spring.push_back(node2);
		}
	}
}

void DistCalculator::BuildCloud()
{
	if (!m_comp_list)
		return;
	if (m_comp_list->empty())
		return;
	if (!m_cloud.empty())
		m_cloud.clear();

	double sx = m_comp_list->sx;
	double sy = m_comp_list->sy;
	double sz = m_comp_list->sz;

	Point p;
	for (auto it = m_comp_list->begin();
		it != m_comp_list->end(); ++it)
	{
		p = it->second->GetPos(sx, sy, sz);
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
	Point p1, p2;
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
	SpringNode& node = m_spring.at(idx);
	int sz = m_spring.size();
	Point pos = node.p;
	Vector force, f1, f2, f3;

	double dist, ang;
	Vector dir, dir2;
	//from cloud
	std::vector<double> lens;
	for (int i = 0; i < m_cloud.size(); ++i)
	{
		dir = m_cloud[i] - pos;
		lens.push_back(dir.length());
	}
	std::sort(lens.begin(), lens.end());
	double scale = (idx == 0 || idx == sz - 1) ? 1.0 : 2.0;
	int loc = int(scale * m_cloud.size() / sz + 1.0);
	loc = std::min(loc, int(m_cloud.size() - 1));
	for (int i = 0; i < m_cloud.size(); ++i)
	{
		dir = m_cloud[i] - pos;
		dist = dir.length();
		if (dist > lens[loc])
			continue;
		dist = std::max(m_rest, dist);
		dir.normalize();
		f1 += dir / dist / dist;
	}
	//from neighbors
	if (idx > 0)
	{
		SpringNode& prev = m_spring.at(idx - 1);
		dir = prev.p - pos;
		dist = dir.length();
		dir.normalize();
		f2 += dir * (dist - node.prevd);
	}
	if (idx < sz - 1)
	{
		SpringNode& next = m_spring.at(idx + 1);
		dir = next.p - pos;
		dist = dir.length();
		dir.normalize();
		f2 += dir * (dist - node.nextd);
	}
	//angular
	if (idx > 0 && idx < sz - 1)
	{
		SpringNode& prev = m_spring.at(idx - 1);
		dir = prev.p - pos;
		SpringNode& next = m_spring.at(idx + 1);
		dir2 = next.p - pos;
		dir.normalize();
		dir2.normalize();
		ang = Dot(dir, dir2)+1.0;
		dir = dir + dir2;
		dir.normalize();
		f3 += dir * ang;
	}

	double norm = (node.prevd + node.nextd) / 100.0;
	norm = std::max(1 / 100.0, norm);
	f1.normalize();
	f1 *= norm;
	f2 = m_f2 * f2 + m_f3 * f3;
	f2.normalize();
	f2 *= norm * 2.0;
	//f3.normalize();
	//f3 *= norm;
	force = m_f1 * f1 + m_f2 * f2;
	node.p = pos + force;
}

void DistCalculator::UpdateRuler()
{
	if (!m_ruler)
		return;

	if (m_ruler->GetNumPoint() != m_spring.size())
		return;

	for (int i = 0; i < m_ruler->GetNumPoint(); ++i)
	{
		m_ruler->GetPoint(i)->SetPoint(
			m_spring[i].p);
	}
}

void DistCalculator::SpringProject(Point &p0, Point &pp)
{
	if (m_spring.empty())
		return;
	int sz = m_spring.size();
	if (sz < 2)
	{
		pp = Point(p0 - m_spring[0].p);
		return;
	}

	//find closest segment
	int minidx = -1;
	double dist;
	double mind = std::numeric_limits<double>::max();
	Point mp;
	for (int i = 0; i < sz - 1; ++i)
	{
		SpringNode &node1 = m_spring.at(i);
		SpringNode &node2 = m_spring.at(i + 1);
		mp = Point((node1.p + node2.p) / 2.0);
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
	Point p1 = m_spring[minidx].p;
	Point p2 = m_spring[minidx + 1].p;
	Vector axis = p2 - p1;
	axis.normalize();
	Vector vp0 = p0 - p1;
	double len = vp0.length();
	double ppx = Dot(vp0, axis);
	double ppy = sqrt(len * len - ppx * ppx);
	pp = Point(ppx + m_spring[minidx].dist, ppy, 0.0);
}