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
#include <dbscan.h>
#include <Cell.h>
#include <algorithm>

using namespace flrd;

ClusterDbscan::ClusterDbscan():
	ClusterMethod(),
	m_size(60),
	m_eps(2.5f),
	m_intw(8.0f)
{

}

ClusterDbscan::~ClusterDbscan()
{

}

bool ClusterDbscan::Execute()
{
	if (m_data.empty())
		return false;

	m_result.clear();
	Dbscan();
	//while (m_result.size() == 1 && m_size)
	//{
	//	ResetData();
	//	m_size += 1;
	//	//m_eps += 0.1;
	//	Dbscan();
	//}

	if (m_result.size() > 0)
	{
		RemoveNoise();
		return true;
	}
	else
		return false;
}

float ClusterDbscan::GetProb()
{
	return 1.0;
}

void ClusterDbscan::ResetData()
{
	for (ClusterIter iter = m_data.begin();
		iter != m_data.end(); ++iter)
	{
		(*iter)->visited = false;
		(*iter)->noise = true;
	}
	m_result.clear();
}

void ClusterDbscan::Dbscan()
{
	size_t ticks = m_data.size() / 100;
	ticks = ticks ? ticks : 1;
	size_t count = 0;
	//an implementation of the DBSCAN algorithm
	//see wikipedia.org
	for (ClusterIter iter = m_data.begin();
		iter != m_data.end(); ++iter)
	{
		pClusterPoint p = *iter;
		if (p->visited)
			continue;
		p->visited = true;
		Cluster neighbors = GetNeighbors(p, m_eps, m_intw);
		if (neighbors.size() >= m_size)
		{
			Cluster cluster;
			ExpandCluster(p, neighbors, cluster);
			m_result.push_back(cluster);
		}
		if (count % 100 == 0)
		{
			SetProgress(static_cast<int>(count / ticks),
				"Computing DBSCAN.");
			count++;
		}
	}
}

void ClusterDbscan::ExpandCluster(pClusterPoint& p, Cluster& neighbors, Cluster& cluster)
{
	cluster.push_back(p);
	p->noise = false;
	for (ClusterIter iter = neighbors.begin();
		iter != neighbors.end(); ++iter)
	{
		pClusterPoint p2 = *iter;
		if (!p2->visited)
		{
			p2->visited = true;
			Cluster neighbors2 = GetNeighbors(p2, m_eps, m_intw);
			if (neighbors2.size() >= m_size)
				neighbors.join(neighbors2);
		}
		if (!m_result.find(p2))
		{
			cluster.push_back(p2);
			p2->noise = false;
		}
	}
}

Cluster ClusterDbscan::GetNeighbors(pClusterPoint &p, float eps, float intw)
{
	Cluster neighbors;

	for (ClusterIter iter = m_data.begin();
		iter != m_data.end(); ++iter)
	{
		if (Dist(*p, **iter, intw) < eps)
			neighbors.push_back(*iter);
	}

	return neighbors;
}

void ClusterDbscan::RemoveNoise()
{
	//remove noise
	unsigned int noise_num;
	do
	{
		noise_num = 0;
		for (ClusterIter iter = m_data.begin();
			iter != m_data.end(); ++iter)
		{
			//find a point that is not clustered
			if (!(*iter)->noise)
				continue;
			Cluster neighbors = GetNeighbors(*iter, 1.1f, 0.0f);
			if (neighbors.size() &&
				ClusterNoise(*iter, neighbors))
				noise_num++;
		}
	} while (noise_num);
}

bool ClusterDbscan::ClusterNoise(pClusterPoint& p, Cluster& neighbors)
{
	if (m_result.size() > 1)
	{
		std::vector<unsigned int> cluster_count;
		cluster_count.resize(m_result.size(), 0);

		for (ClusterIter iter = neighbors.begin();
			iter != neighbors.end(); ++iter)
		{
			if ((*iter)->noise)
				continue;
			int index = m_result.find_(*iter);
			if (index == -1)
				continue;
			cluster_count[index]++;
		}

		//find max
		auto result = std::max_element(
			cluster_count.begin(), cluster_count.end());
		if (*result)
		{
			size_t index = std::distance(cluster_count.begin(), result);
			m_result[index].push_back(p);
			p->noise = false;
			return true;
		}
		else return false;
	}
	else
	{
		Cluster cluster;
		cluster.push_back(p);
		p->noise = false;
		m_result.push_back(cluster);
		return true;
	}
}
