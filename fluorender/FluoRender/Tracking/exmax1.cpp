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
	do {
		m_params_prv = m_params;
		m_likelihood_prv = m_likelihood;
		Expectation();
		Maximization();
		//histograom test
		//GenUncertainty(0.05);
		counter++;
	} while (!Converge() &&
		counter < m_max_iter);

	//if (counter == m_max_iter)
	//{
	//	if (m_weak_result)
	//	{
	//		GenResult();
	//		if (GetProb() > m_tol - 0.2f)
	//			return true;
	//	}
	//	return false;
	//}
	//else
	//{
	//	GenResult();
	//	if (GetProb() > (m_weak_result ? m_tol - 0.2f : m_tol))
	//		return true;
	//	else
	//		return false;
	//}
	return true;
}

void ExMax1::Initialize()
{
}

void ExMax1::Expectation()
{
	unsigned int i = 0;
	for (ClusterIter iter = m_data.begin();
		iter != m_data.end(); ++iter)
	{
		double sum = 0;
		for (unsigned int j = 0; j < m_clnum; ++j)
			sum += m_params_prv[j].tau *
			Gaussian((*iter)->centerf,
				m_params_prv[j].mean,
				m_params_prv[j].covar) * (*iter)->intensity;
		for (unsigned int j = 0; j < m_clnum; ++j)
		{
			if (sum > 0.0)
				m_mem_prob[j][i] = m_params_prv[j].tau *
				Gaussian((*iter)->centerf,
					m_params_prv[j].mean,
					m_params_prv[j].covar) * (*iter)->intensity / sum;
			else
				m_mem_prob[j][i] = 1.0;
		}
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
	for (unsigned int j = 0; j < m_clnum; ++j)
	{
		double sum_t = 0;
		unsigned int i = 0;
		for (ClusterIter iter = m_data.begin();
			iter != m_data.end(); ++iter)
		{
			sum_t += m_mem_prob[j][i] * (*iter)->intensity;
			i++;
		}

		//tau
		m_params[j].tau = sum_t / m_data.size();

		//mean
		i = 0;
		EmVec sum_p = { 0, 0, 0 };
		for (ClusterIter iter = m_data.begin();
			iter != m_data.end(); ++iter)
		{
			sum_p += (*iter)->centerf * (*iter)->intensity * m_mem_prob[j][i];
			i++;
		}
		m_params[j].mean = sum_p / sum_t;

		//covar/sigma
		i = 0;
		EmMat sum_s = boost::qvm::zero_mat<double, 3, 3>();
		for (ClusterIter iter = m_data.begin();
			iter != m_data.end(); ++iter)
		{
			EmVec d = (*iter)->centerf - m_params[j].mean;
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
			sum_s += form * m_mem_prob[j][i] * (*iter)->intensity;
			i++;
		}
		m_params[j].covar = sum_s / sum_t;
	}
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
		for (unsigned int j = 0; j < m_clnum; ++j)
		{
			l = log(m_params_prv[j].tau);
			l -= 0.5 * log(boost::qvm::determinant(m_params_prv[j].covar));
			EmVec d = (*iter)->centerf - m_params_prv[j].mean;
			l -= 0.5 * boost::qvm::dot(d, boost::qvm::inverse(m_params_prv[j].covar) * d);
			l -= c;
			m_likelihood += l * (*iter)->intensity;
		}
	}

	if (fabs((m_likelihood - m_likelihood_prv) / m_likelihood) > m_eps)
		return false;
	else
		return true;
}

