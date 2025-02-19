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
#ifdef _DEBUG
#include <Debug.h>
#endif
#include <exmax1.h>
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
	pp->centerf = { A0(p), A1(p), A2(p) };
	pp->intensity = value;
	m_data.push_back(pp);
}

bool ExMax1::Execute()
{
	if (m_data.empty())
		return false;
//#ifdef _DEBUG
//	DBMIFLOAT64 mi;
//	mi.nx = 69; mi.ny = 69; mi.nc = 1; mi.nt = mi.nx * mi.nc * 8;
//#endif
	Initialize();
//#ifdef _DEBUG
//	mi.data = m_mem_prob.data();
//#endif

	if (m_max_iter)
	{
		size_t counter = 0;
		do
		{
			if (m_inc_counter > 3)
				break;
			m_params_prv = m_params;
			m_likelihood_prv = m_likelihood;
			Expectation();
			Maximization();
			counter++;
		} while (!Converge() &&
			counter < m_max_iter);
	}

	return true;
}

fluo::Point ExMax1::GetCenter()
{
	if (GetProb() < 0.5)
		return fluo::Point(
			A0(m_params.mean),
			A1(m_params.mean),
			A2(m_params.mean));
	else
		return m_init_mean;
}

double ExMax1::GetProb()
{
	return m_likelihood;
}

void ExMax1::Initialize()
{
	//float minint, maxint;
	double count = 0;
	m_params.mean = { 0, 0, 0 };
	for (ClusterIter iter = m_data.begin();
		iter != m_data.end(); ++iter)
	{
		//if (iter == m_data.begin())
		//{
		//	minint = (*iter)->intensity;
		//	maxint = (*iter)->intensity;
		//	m_params.mean = (*iter)->centerf;
		//}
		//else if ((*iter)->intensity < minint)
		//{
		//	minint = (*iter)->intensity;
		//	m_params.mean = (*iter)->centerf;
		//}
		//else if ((*iter)->intensity > maxint)
		//{
		//	maxint = (*iter)->intensity;
		//}
		m_params.mean += (*iter)->centerf * (*iter)->intensity;
		count += (*iter)->intensity;
	}
	if (count == 0)
		return;
	m_params.mean /= count;

	//normalize
	//float range = maxint - minint;
	//if (range > 0.0f)
	//{
	//	for (ClusterIter iter = m_data.begin();
	//		iter != m_data.end(); ++iter)
	//	{
	//		(*iter)->intensity = 1.0 - ((*iter)->intensity - minint) / range;
	//	}
	//}

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
	//Regulate(m_params.covar);
	m_params.tau = 1;

	m_mem_prob.resize(m_data.size(), 0);

	m_likelihood = 0;

	//limit
	m_eps = m_eps == 0.0 ? m_l : m_eps * m_l;
	m_inc_counter = 0;

	//mean
	m_init_mean = fluo::Point(
		A0(m_params.mean),
		A1(m_params.mean),
		A2(m_params.mean));

	m_likelihood_sum = 0;
}

void ExMax1::Expectation()
{
	unsigned int i = 0;
	for (ClusterIter iter = m_data.begin();
		iter != m_data.end(); ++iter)
	{
		m_mem_prob[i] = /*m_params_prv.tau **/
			Gaussian((*iter)->centerf,
				m_params_prv.mean,
				m_params_prv.covar) * (*iter)->intensity;
		i++;
	}
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
	//m_params.tau = sum_t / m_data.size();

	//mean
	i = 0;
	EmVec sum_p = { 0, 0, 0 };
	for (ClusterIter iter = m_data.begin();
		iter != m_data.end(); ++iter)
	{
		sum_p += (*iter)->centerf * (*iter)->intensity * m_mem_prob[i];
		i++;
	}
	if (sum_t != 0.0)
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
	if (sum_t != 0.0)
		m_params.covar = sum_s / sum_t;
	//Regulate(m_params.covar);
}

bool ExMax1::Converge()
{
	//compute likelihood
	//m_likelihood = GetProb();
	m_likelihood = mag(m_params.mean - m_params_prv.mean);
	m_likelihood_sum += m_likelihood;
	if (m_likelihood >= m_likelihood_prv)
		m_inc_counter++;
	if (fabs(m_likelihood) > m_eps)
		return false;
	else
		return true;
}

double ExMax1::Gaussian(EmVec &p, EmVec &m, EmMat &s)
{
	using namespace boost::qvm;
	double pi2_3 = 248.050213442399; //(2pi)^3
	double det = Det(s);
	if (std::abs(det) < fluo::Epsilon(10))
		return 0.0;
	EmVec d = p - m;
	EmMat inv = Inv(s);
	return exp(-0.5 * dot(d, inv * d)) / sqrt(pi2_3 * det);
}

double ExMax1::Det(EmMat &mat)
{
	double det = determinant(mat);
	if (det == 0.0)
	{
		//get 2d
		boost::qvm::mat<double, 2, 2> m2;
		A00(m2) = A00(mat);
		A01(m2) = A01(mat);
		A10(m2) = A10(mat);
		A11(m2) = A11(mat);
		det = determinant(m2);
	}
	if (det == 0.0)
	{
		det = std::max(A00(mat), A11(mat));
	}
	return det;
}

EmMat ExMax1::Inv(EmMat &mat)
{
	EmMat result = mat;
	double det = determinant(mat);
	if (det == 0.0)
	{
		//get 2d
		boost::qvm::mat<double, 2, 2> m2;
		A00(m2) = A00(mat);
		A01(m2) = A01(mat);
		A10(m2) = A10(mat);
		A11(m2) = A11(mat);
		det = determinant(m2);
		if (det == 0.0)
		{
			if (A00(m2) != 0.0)
				A00(result) = 1.0 / A00(m2);
			if (A11(m2) != 0.0)
				A11(result) = 1.0 / A11(m2);
		}
		else
		{
			m2 = inverse(m2);
			A00(result) = A00(m2);
			A01(result) = A01(m2);
			A10(result) = A10(m2);
			A11(result) = A11(m2);
		}
	}
	else
	{
		result = inverse(mat);
	}
	return result;
}

void ExMax1::Regulate(EmMat &s)
{
	double det = determinant(s);
	if (det == 0.0)
	{
		double dv[3] = { A00(s), A11(s), A22(s) };
		if (m_cov_eps == 0.0)
		{
			for (const double &ev : dv)
			{
				if (ev != 0.0)
				{
					if (m_cov_eps == 0.0)
						m_cov_eps = ev;
					else
						m_cov_eps = std::min(m_cov_eps, ev);
				}
			}
			if (m_cov_eps == 0.0)
				m_cov_eps = 0.5;
			else
				m_cov_eps *= 0.5;
		}
		//perturb
		if (dv[0] == 0.0)
			A00(s) = m_cov_eps;
		if (dv[1] == 0.0)
			A11(s) = m_cov_eps;
		if (dv[2] == 0.0)
			A22(s) = m_cov_eps;
	}
}