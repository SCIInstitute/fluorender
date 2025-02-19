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
#include <kmeans.h>
#include <algorithm>

using namespace flrd;

ClusterKmeans::ClusterKmeans() :
	m_clnum(2),
	m_eps(1e-3f),
	m_max_iter(100)
{

}

ClusterKmeans::~ClusterKmeans()
{

}

bool ClusterKmeans::Execute()
{
	if (m_data.empty())
		return false;

	Initialize();

	size_t counter = 0;
	do {
		m_means_prv = m_means;
		Assign();
		Update();
		counter++;
		SetProgress(static_cast<int>(100 * counter / (m_max_iter ? m_max_iter : 1)),
			"Computing K-Means.");
	} while (!Converge() &&
		counter < m_max_iter);

	if (counter == m_max_iter)
		return false;
	else
		return true;
}

float ClusterKmeans::GetProb()
{
	return 1.0;
}

void ClusterKmeans::Initialize()
{
	m_means.clear();
	//search for maximum
	pClusterPoint p = nullptr;
	Cluster cluster;
	for (ClusterIter iter = m_data.begin();
		iter != m_data.end(); ++iter)
	{
		if (iter == m_data.begin())
			p = *iter;
		else
		{
			if ((*iter)->intensity > p->intensity)
				p = *iter;
		}
	}
	if (p != nullptr)
	{
		m_means.push_back(p->centerf);
		cluster.push_back(p);
	}
	//search for the rest
	for (size_t i = 1; i < m_clnum; ++i)
	{
		p = nullptr;
		for (ClusterIter iter = m_data.begin();
			iter != m_data.end(); ++iter)
		{
			if (cluster.find(*iter))
				continue;
			if (p == nullptr)
				p = *iter;
			else
			{
				double d1 = boost::qvm::mag(p->centerf - m_means[i - 1]);
				double d2 = boost::qvm::mag((*iter)->centerf - m_means[i - 1]);
				if (d2 > d1)
					p = *iter;
			}
		}
		if (p != nullptr)
		{
			m_means.push_back(p->centerf);
			cluster.push_back(p);
		}
	}
}

void ClusterKmeans::Assign()
{
	m_result.clear();
	m_result.resize(m_clnum);
	for (ClusterIter iter = m_data.begin();
		iter != m_data.end(); ++iter)
	{
		size_t index = 0;
		bool changed = false;
		double mind;
		for (size_t i = 0; i < m_clnum; ++i)
		{
			double d = boost::qvm::mag((*iter)->centerf - m_means[i]);
			if (i == 0)
			{
				index = i;
				mind = d;
				changed = true;
			}
			else
			{
				if (d < mind)
				{
					index = i;
					mind = d;
					changed = true;
				}
			}
		}

		if (changed)
			m_result[index].push_back(*iter);
	}
}

void ClusterKmeans::Update()
{
	for (size_t i = 0; i < m_clnum; ++i)
	{
		if (m_result[i].size() == 0)
			continue;
		EmVec sum = { 0, 0, 0 };
		double count = 0;
		for (ClusterIter iter = m_result[i].begin();
			iter != m_result[i].end(); ++iter)
		{
			sum += (*iter)->centerf * (*iter)->intensity;
			count += (*iter)->intensity;
		}
		m_means[i] = sum / count;
	}
}

bool ClusterKmeans::Converge()
{
	for (size_t i = 0; i < m_clnum; ++i)
	{
		double d = boost::qvm::mag(m_means[i] - m_means_prv[i]);
		if (d > m_eps)
			return false;
	}
	return true;
}