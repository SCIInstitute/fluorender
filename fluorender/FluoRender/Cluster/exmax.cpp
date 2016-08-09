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
#include "exmax.h"
#include <algorithm>

using namespace FL;

ClusterExmax::ClusterExmax() :
	m_clnum(2),
	m_eps(1e-3),
	m_max_iter(100)
{

}

ClusterExmax::~ClusterExmax()
{

}

bool ClusterExmax::Execute()
{
	Initialize();

	size_t counter = 0;
	do {
		m_params_prv = m_params;
		m_likelihood_prv = m_likelihood;
		Expectation();
		Maximization();
		counter++;
	} while (!Converge() &&
		counter < m_max_iter);

	if (counter == m_max_iter)
		return false;
	else
	{
		GenResult();
		return true;
	}
}

void ClusterExmax::Initialize()
{
	//use same tau and covar
	FLIVR::Vector trace;
	FLIVR::Vector mean;
	FLIVR::Vector vec;
	for (ClusterIter iter = m_data.begin();
		iter != m_data.end(); ++iter)
		mean += FLIVR::Vector((*iter)->center);
	mean /= m_data.size();
	for (ClusterIter iter = m_data.begin();
		iter != m_data.end(); ++iter)
	{
		vec = FLIVR::Vector((*iter)->center) - mean;
		trace += vec * vec;
	}
	trace /= m_data.size() - 1;
	FLIVR::Mat3 covar(trace);
	double tau = 1.0 / m_clnum;

	//use similar method as k-means for means
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
		Params params;
		params.tau = tau;
		params.mean = p->center;
		params.covar = covar;
		m_params.push_back(params);
		cluster.push_back(p);
	}
	//search for the rest
	for (int i = 1; i < m_clnum; ++i)
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
				FLIVR::Point pi_1 = m_params[i - 1].mean;
				double d1 = (p->center - pi_1).length();
				double d2 = ((*iter)->center - pi_1).length();
				if (d2 > d1)
					p = *iter;
			}
		}
		if (p != nullptr)
		{
			Params params;
			params.tau = tau;
			params.mean = p->center;
			params.covar = covar;
			m_params.push_back(params);
			cluster.push_back(p);
		}
	}

	//allocate membership probabilities
	m_mem_prob.resize(m_clnum);
	for (size_t i = 0; i < m_clnum; ++i)
		m_mem_prob[i].resize(m_data.size(), 0);
}

void ClusterExmax::Expectation()
{
	unsigned int i = 0;
	for (ClusterIter iter = m_data.begin();
		iter != m_data.end(); ++iter)
	{
		double sum = 0;
		for (unsigned int j = 0; j < m_clnum; ++j)
			sum += m_params_prv[j].tau *
			Gaussian((*iter)->center,
				m_params_prv[j].mean,
				m_params_prv[j].covar);
		for (unsigned int j = 0; j < m_clnum; ++j)
		{
			m_mem_prob[j][i] = m_params_prv[j].tau *
				Gaussian((*iter)->center, m_params_prv[j].mean,
					m_params_prv[j].covar) / sum;
		}
		i++;
	}
}

double ClusterExmax::Gaussian(FLIVR::Point &p, FLIVR::Point &m, FLIVR::Mat3 &s)
{
	double pi2_3 = 248.050213442399; //(2pi)^3
	FLIVR::Vector d = p - m;
	return exp(-0.5 * FLIVR::Dot(d, s.inv() * d)) / sqrt(pi2_3 * s.det());
}

void ClusterExmax::Maximization()
{
	//update params
	for (unsigned int j = 0; j < m_clnum; ++j)
	{
		unsigned int i;
		double sum_t = 0;
		for (i = 0; i < m_data.size(); ++i)
			sum_t += m_mem_prob[j][i];

		//tau
		m_params[j].tau = sum_t / m_data.size();

		//mean
		i = 0;
		FLIVR::Point sum_p;
		for (ClusterIter iter = m_data.begin();
			iter != m_data.end(); ++iter)
		{
			sum_p += (*iter)->center * m_mem_prob[j][i];
			i++;
		}
		m_params[j].mean = sum_p / sum_t;

		//covar/sigma
		i = 0;
		FLIVR::Mat3 sum_s(0);
		for (ClusterIter iter = m_data.begin();
			iter != m_data.end(); ++iter)
		{
			FLIVR::Vector d = (*iter)->center - m_params[j].mean;
			sum_s += form(d, d) * m_mem_prob[j][i];
			i++;
		}
		m_params[j].covar = sum_s / sum_t;
	}
}

bool ClusterExmax::Converge()
{
	//compute likelihood
	double c = 2.75681559961402;
	m_likelihood = 0;
	double l;
	for (ClusterIter iter = m_data.begin();
		iter != m_data.end(); ++iter)
	{
		for (unsigned int j = 0; j < m_clnum; ++j)
		{
			l = log(m_params_prv[j].tau);
			l -= 0.5 * log(m_params_prv[j].covar.det());
			FLIVR::Vector d = (*iter)->center - m_params_prv[j].mean;
			l -= 0.5 * FLIVR::Dot(d, m_params_prv[j].covar.inv() * d);
			l -= c;
			m_likelihood += l;
		}
	}

	if (fabs(m_likelihood - m_likelihood_prv) > m_eps)
		return false;
	else
		return true;
}

void ClusterExmax::GenResult()
{
	m_result.clear();
	m_result.resize(m_clnum);
	unsigned int i = 0;
	for (ClusterIter iter = m_data.begin();
		iter != m_data.end(); ++iter)
	{
		int index = -1;
		double max_mem_prob;
		for (int j = 0; j < m_clnum; ++j)
		{
			if (j == 0)
			{
				index = j;
				max_mem_prob = m_mem_prob[j][i];
			}
			else
			{
				if (m_mem_prob[j][i] > max_mem_prob)
				{
					index = j;
					max_mem_prob = m_mem_prob[j][i];
				}
			}
		}
		if (index > -1)
			m_result[index].push_back(*iter);
		i++;
	}
}