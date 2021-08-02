/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2021 Scientific Computing and Imaging Institute,
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
#include "exmax1.h"
#include <algorithm>
#include <fstream>
#include <boost/qvm/mat_operations.hpp>
#include <boost/qvm/vec_operations.hpp>
#include <boost/qvm/vec_mat_operations.hpp>
#include <boost/qvm/mat_access.hpp>
#include <boost/qvm/vec_access.hpp>

using namespace flrd;

void ExMax1::AddClusterPoint(const EmVec &p, const float value)
{
	pClusterPoint pp(new ClusterPoint);
	pp->id = 0;
	pp->cid = 0;
	pp->visited = false;
	pp->noise = true;
	using namespace boost::qvm;
	pp->centeri = p;
	pp->centerf = { A0(p)*A0(m_spc), A1(p)*A1(m_spc), A2(p)*A2(m_spc) };
	pp->intensity = value;
	m_data.push_back(pp);
}

bool ExMax1::Execute()
{
	if (m_data.empty())
		return false;

	Initialize();

	size_t counter = 0;
	do
	{
		m_params_prv = m_params;
		m_likelihood_prv = m_likelihood;
		Expectation();
		Maximization();
		counter++;
	}
	while (!Converge() &&
		counter < m_max_iter);

	return true;
}

fluo::Point ExMax1::GetCenter()
{
	return fluo::Point(
		A0(m_params.mean),
		A1(m_params.mean),
		A2(m_params.mean));
}

float ExMax1::GetProb()
{
	if (m_mem_prob.empty())
		return 0.0f;

	double sum = 0;
	size_t size = m_mem_prob.size();
	for (size_t i = 0; i < size; ++i)
	{
		sum += m_mem_prob[i];
	}
	if (size)
		return float(sum / size);
	else
		return 0.0f;
}

void ExMax1::Initialize()
{
	float minint, maxint;
	double count = 0;
	for (ClusterIter iter = m_data.begin();
		iter != m_data.end(); ++iter)
	{
		if (iter == m_data.begin())
		{
			minint = (*iter)->intensity;
			maxint = (*iter)->intensity;
			m_params.mean = (*iter)->centerf;
		}
		else if ((*iter)->intensity < minint)
		{
			minint = (*iter)->intensity;
			m_params.mean = (*iter)->centerf;
		}
		else if ((*iter)->intensity > maxint)
		{
			maxint = (*iter)->intensity;
		}
		count += (*iter)->intensity;
	}

	//normalize
	for (ClusterIter iter = m_data.begin();
		iter != m_data.end(); ++iter)
	{
		(*iter)->intensity = 1.0 - ((*iter)->intensity - minint) / (maxint - minint);
	}

	EmVec trace = { 0, 0, 0 };
	EmVec vec;
	for (ClusterIter iter = m_data.begin();
		iter != m_data.end(); ++iter)
	{
		vec = (*iter)->centerf - m_params.mean;
		EmVec temp = { 0, 0, 0 };
		boost::qvm::A0(temp) = boost::qvm::A0(vec) * boost::qvm::A0(vec);
		boost::qvm::A1(temp) = boost::qvm::A1(vec) * boost::qvm::A1(vec);
		boost::qvm::A2(temp) = boost::qvm::A2(vec) * boost::qvm::A2(vec);
		trace += temp * (*iter)->intensity;
	}
	trace /= count/*double(m_data.size() - 1)*/;
	m_params.covar = boost::qvm::zero_mat<double, 3, 3>();
	boost::qvm::A00(m_params.covar) = boost::qvm::A0(trace);
	boost::qvm::A11(m_params.covar) = boost::qvm::A1(trace);
	boost::qvm::A22(m_params.covar) = boost::qvm::A2(trace);
	m_params.tau = 1;

	m_mem_prob.resize(m_data.size(), 0);

	m_likelihood = 0;
}

void ExMax1::Expectation()
{
	unsigned int i = 0;
	for (ClusterIter iter = m_data.begin();
		iter != m_data.end(); ++iter)
	{
		double sum = m_params_prv.tau *
			Gaussian((*iter)->centerf,
				m_params_prv.mean,
				m_params_prv.covar) * (*iter)->intensity;
		if (sum > 0.0)
			m_mem_prob[i] = m_params_prv.tau *
				Gaussian((*iter)->centerf,
				m_params_prv.mean,
				m_params_prv.covar) * (*iter)->intensity / sum;
		else
			m_mem_prob[i] = 1.0;
		i++;
	}
}

double ExMax1::Gaussian(EmVec &p, EmVec &m, EmMat &s)
{
	using namespace boost::qvm;
	double pi2_3 = 248.050213442399; //(2pi)^3
	double det = determinant(s);
	if (det == 0.0)
	{
		//perturb
		A00(s) = A00(s) < 0.5 ? 0.5 : A00(s);
		A11(s) = A11(s) < 0.5 ? 0.5 : A11(s);
		A22(s) = A22(s) < 0.5 ? 0.5 : A22(s);
		det = determinant(s);
	}
	EmVec d = p - m;
	return exp(-0.5 * dot(d, inverse(s) * d)) / sqrt(pi2_3 * det);
}

void ExMax1::Maximization()
{
	//update params
	double sum_t = 0;
	unsigned int i = 0;
	for (ClusterIter iter = m_data.begin();
		iter != m_data.end(); ++iter)
	{
		sum_t += m_mem_prob[i] * (*iter)->intensity;
		i++;
	}

	//tau
	m_params.tau = sum_t / m_data.size();

	//mean
	i = 0;
	EmVec sum_p = { 0, 0, 0 };
	for (ClusterIter iter = m_data.begin();
		iter != m_data.end(); ++iter)
	{
		sum_p += (*iter)->centerf * (*iter)->intensity * m_mem_prob[i];
		i++;
	}
	m_params.mean = sum_p / sum_t;

	//covar/sigma
	i = 0;
	EmMat sum_s = boost::qvm::zero_mat<double, 3, 3>();
	for (ClusterIter iter = m_data.begin();
		iter != m_data.end(); ++iter)
	{
		EmVec d = (*iter)->centerf - m_params.mean;
		EmMat form;
		using namespace boost::qvm;
		A00(form) = A0(d) * A0(d);
		A01(form) = A0(d) * A1(d);
		A02(form) = A0(d) * A2(d);
		A10(form) = A1(d) * A0(d);
		A11(form) = A1(d) * A1(d);
		A12(form) = A1(d) * A2(d);
		A20(form) = A2(d) * A0(d);
		A21(form) = A2(d) * A1(d);
		A22(form) = A2(d) * A2(d);
		sum_s += form * m_mem_prob[i] * (*iter)->intensity;
		i++;
	}
	m_params.covar = sum_s / sum_t;
}

bool ExMax1::Converge()
{
	//compute likelihood
	double c = 2.75681559961402;
	m_likelihood = 0;
	double l;
	for (ClusterIter iter = m_data.begin();
		iter != m_data.end(); ++iter)
	{
		l = log(m_params_prv.tau);
		l -= 0.5 * log(boost::qvm::determinant(m_params_prv.covar));
		EmVec d = (*iter)->centerf - m_params_prv.mean;
		l -= 0.5 * boost::qvm::dot(d, boost::qvm::inverse(m_params_prv.covar) * d);
		l -= c;
		m_likelihood += l * (*iter)->intensity;
	}

	if (fabs((m_likelihood - m_likelihood_prv) / m_likelihood) > m_eps)
		return false;
	else
		return true;
}

