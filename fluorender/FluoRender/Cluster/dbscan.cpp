/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2014 Scientific Computing and Imaging Institute,
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
#include "dbscan.h"

using namespace FL;

ClusterDbscan::ClusterDbscan():
	m_size(50),
	m_eps(2.0f),
	m_intw(100.0f),
	m_id_counter(1)
{

}

ClusterDbscan::~ClusterDbscan()
{

}

void ClusterDbscan::AddClusterPoint(const FLIVR::Point &p, const float value)
{
	pClusterPoint pp(new ClusterPoint);
	pp->id = m_id_counter++;
	pp->visited = false;
	pp->center = p;
	pp->intensity = value;
	m_data.insert(std::pair<unsigned int, pClusterPoint>
		(pp->id, pp));
}

bool ClusterDbscan::Execute()
{
	for (ClusterIter iter = m_data.begin();
		iter != m_data.end(); ++iter)
	{
		pClusterPoint p = iter->second;
		if (p->visited)
			continue;
		p->visited = true;
		Cluster neighbors = GetNeighbors(p);
		if (neighbors.size() >= m_size)
		{
			Cluster cluster;
			ExpandCluster(p, neighbors, cluster);
			m_result.push_back(cluster);
		}
	}
	return true;
}

void ClusterDbscan::ExpandCluster(pClusterPoint& p, Cluster& neighbors, Cluster& cluster)
{
	cluster.insert(std::pair<unsigned int, pClusterPoint>
		(p->id, p));
	for (ClusterIter iter = neighbors.begin();
		iter != neighbors.end(); ++iter)
	{
		pClusterPoint p2 = iter->second;
		if (!p2->visited)
		{
			p2->visited = true;
			Cluster neighbors2 = GetNeighbors(p2);
			if (neighbors2.size() >= m_size)
				neighbors.insert(neighbors2.begin(), neighbors2.end());
		}
		if (!m_result.find(p2))
			cluster.insert(std::pair<unsigned int, pClusterPoint>
			(p2->id, p2));
	}
}

Cluster ClusterDbscan::GetNeighbors(pClusterPoint &p)
{
	Cluster neighbors;

	for (ClusterIter iter = m_data.begin();
		iter != m_data.end(); ++iter)
	{
		if (Dist(*p, *(iter->second), m_intw) < m_eps)
			neighbors.insert(std::pair<unsigned int, pClusterPoint>
			(iter->second->id, iter->second));
	}

	return neighbors;
}