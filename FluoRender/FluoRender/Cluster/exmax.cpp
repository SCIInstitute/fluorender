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
#include <exmax.h>
#include <algorithm>
#include <fstream>
#include <boost/qvm/mat_operations.hpp>
#include <boost/qvm/vec_operations.hpp>
#include <boost/qvm/vec_mat_operations.hpp>
#include <boost/qvm/mat_access.hpp>
#include <boost/qvm/vec_access.hpp>

using namespace flrd;

ClusterExmax::ClusterExmax() :
	m_clnum(2),
	m_weak_result(false),
	m_eps(1e-6f),
	m_max_iter(300),
	m_tol(0.9f)
{

}

ClusterExmax::~ClusterExmax()
{

}

bool ClusterExmax::Execute()
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
		GenUncertainty(0.05);
		counter++;
		SetProgress(static_cast<int>(100 * counter / m_max_iter),
			"Computing ExMax.");
	} while (!Converge() &&
		counter < m_max_iter);

	if (counter == m_max_iter)
	{
		if (m_weak_result)
		{
			GenResult();
			if (GetProb() > m_tol - 0.2f)
				return true;
		}
		return false;
	}
	else
	{
		GenResult();
		if (GetProb() > (m_weak_result? m_tol - 0.2f : m_tol))
			return true;
		else
			return false;
	}
}

float ClusterExmax::GetProb()
{
	if (m_mem_prob.empty())
		return 0.0f;

	double sum = 0;
	double max;
	size_t size = m_mem_prob[0].size();
	for (size_t i = 0; i < size; ++i)
	{
		max = 0;
		for (size_t j = 0; j < m_clnum; ++j)
		{
			if (m_mem_prob[j][i] > max)
				max = m_mem_prob[j][i];
		}
		sum += max;
	}
	if (size)
		return float(sum / size);
	else
		return 0.0f;
}

void ClusterExmax::Initialize()
{
	if (m_use_init_cluster)
		Init2();
	else
		Init1();

	//allocate membership probabilities
	m_mem_prob.resize(m_clnum);
	for (size_t i = 0; i < m_clnum; ++i)
		m_mem_prob[i].resize(m_data.size(), 0);
	m_mem_prob_prv.clear();
	m_count.resize(m_data.size(), 0);
}

void ClusterExmax::Init1()
{
	//use same tau and covar
	EmVec trace = { 0, 0, 0 };
	EmVec mean = { 0, 0, 0 };
	EmVec vec;
	double count = 0;
	for (ClusterIter iter = m_data.begin();
		iter != m_data.end(); ++iter)
	{
		mean += (*iter)->centerf * (*iter)->intensity;
		count += (*iter)->intensity;
	}
	mean /= count;

	for (ClusterIter iter = m_data.begin();
		iter != m_data.end(); ++iter)
	{
		vec = (*iter)->centerf - mean;
		EmVec temp = { 0, 0, 0 };
		boost::qvm::A0(temp) = boost::qvm::A0(vec) * boost::qvm::A0(vec);
		boost::qvm::A1(temp) = boost::qvm::A1(vec) * boost::qvm::A1(vec);
		boost::qvm::A2(temp) = boost::qvm::A2(vec) * boost::qvm::A2(vec);
		trace += temp * (*iter)->intensity;
	}
	trace /= count/*double(m_data.size() - 1)*/;
	EmMat covar = boost::qvm::zero_mat<double, 3, 3>();
	boost::qvm::A00(covar) = boost::qvm::A0(trace);
	boost::qvm::A11(covar) = boost::qvm::A1(trace);
	boost::qvm::A22(covar) = boost::qvm::A2(trace);
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
		params.mean = p->centerf;
		params.covar = covar;
		m_params.push_back(params);
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
				EmVec pi_1 = m_params[i - 1].mean;
				double d1 = boost::qvm::mag(p->centerf - pi_1);
				double d2 = boost::qvm::mag((*iter)->centerf - pi_1);
				if (d2 > d1)
					p = *iter;
			}
		}
		if (p != nullptr)
		{
			Params params;
			params.tau = tau;
			params.mean = p->centerf;
			params.covar = covar;
			m_params.push_back(params);
			cluster.push_back(p);
		}
	}
}

void ClusterExmax::Init2()
{
	//mixture coefficient
	double tau = 1.0 / m_clnum;
	for (size_t i = 0; i < m_clnum; ++i)
	{
		EmVec trace = { 0, 0, 0 };
		EmVec mean = { 0, 0, 0 };
		EmVec vec;
		double count = 0;

		//center
		for (ClusterIter iter = m_data.begin();
			iter != m_data.end(); ++iter)
		{
			if ((*iter)->cid != i)
				continue;

			mean += (*iter)->centerf * (*iter)->intensity;
			count += (*iter)->intensity;
		}
		mean /= count;

		//covar
		for (ClusterIter iter = m_data.begin();
			iter != m_data.end(); ++iter)
		{
			if ((*iter)->cid != i)
				continue;

			vec = (*iter)->centerf - mean;
			EmVec temp = { 0, 0, 0 };
			boost::qvm::A0(temp) = boost::qvm::A0(vec) * boost::qvm::A0(vec);
			boost::qvm::A1(temp) = boost::qvm::A1(vec) * boost::qvm::A1(vec);
			boost::qvm::A2(temp) = boost::qvm::A2(vec) * boost::qvm::A2(vec);
			trace += temp * (*iter)->intensity;
		}
		trace /= count/* - 1*/;
		EmMat covar = boost::qvm::zero_mat<double, 3, 3>();
		boost::qvm::A00(covar) = boost::qvm::A0(trace);
		boost::qvm::A11(covar) = boost::qvm::A1(trace);
		boost::qvm::A22(covar) = boost::qvm::A2(trace);

		//add to init values
		Params params;
		params.tau = tau;
		params.mean = mean;
		params.covar = covar;
		m_params.push_back(params);
	}
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

double ClusterExmax::Gaussian(EmVec &p, EmVec &m, EmMat &s)
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

void ClusterExmax::Maximization()
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
			l -= 0.5 * log(boost::qvm::determinant(m_params_prv[j].covar));
			EmVec d = (*iter)->centerf - m_params_prv[j].mean;
			l -= 0.5 * boost::qvm::dot(d, boost::qvm::inverse(m_params_prv[j].covar) * d);
			l -= c;
			m_likelihood += l * (*iter)->intensity;
		}
	}

	if (fabs((m_likelihood - m_likelihood_prv)/ m_likelihood) > m_eps)
		return false;
	else
		return true;
}

void ClusterExmax::GenResult()
{
	m_result.clear();
	m_result.resize(m_clnum);
	size_t i = 0;
	for (ClusterIter iter = m_data.begin();
		iter != m_data.end(); ++iter)
	{
		size_t index = 0;
		bool changed = false;
		double max_mem_prob;
		for (size_t j = 0; j < m_clnum; ++j)
		{
			if (j == 0)
			{
				index = j;
				max_mem_prob = m_mem_prob[j][i];
				changed = true;
			}
			else
			{
				if (m_mem_prob[j][i] > max_mem_prob)
				{
					index = j;
					max_mem_prob = m_mem_prob[j][i];
					changed = true;
				}
			}
		}
		if (changed)
			m_result[index].push_back(*iter);
		i++;
	}
}

void ClusterExmax::GenHistogram(size_t bins)
{
	if (!m_clnum)
		return;

	m_bins = bins;
	//allocate histogram space
	m_histogram.clear();
	double value = 1.0 / m_clnum;
	double inc = (1.0 - value) / m_bins;
	for (size_t i = 0; i < m_bins; ++i)
	{
		EmBin bin;
		bin.value = value;
		value += inc;
		bin.count = 0;
		m_histogram.push_back(bin);
	}
	//fill in histogram
	size_t i = 0;
	for (ClusterIter iter = m_data.begin();
		iter != m_data.end(); ++iter)
	{
		double max_mem_prob = 0;
		for (size_t j = 0; j < m_clnum; ++j)
		{
			if (j == 0)
				max_mem_prob = m_mem_prob[j][i];
			else if (m_mem_prob[j][i] > max_mem_prob)
				max_mem_prob = m_mem_prob[j][i];
		}
		i++;
		//
		size_t index = size_t((max_mem_prob - 
			1.0 / m_clnum) / inc);
		if (index < m_bins)
			m_histogram[index].count++;
	}
	//output histogram
	std::ofstream outfile;
	outfile.open("hist.txt", std::ofstream::out |
		std::ofstream::app);
	for (size_t i = 0; i < m_histogram.size(); ++i)
		if (i < m_histogram.size() - 1)
			outfile << m_histogram[i].count << "\t";
		else
			outfile << m_histogram[i].count << "\n";
	outfile.close();
}

void ClusterExmax::GenUncertainty(double delta, bool output)
{
	if (!m_clnum)
		return;
	if (!m_mem_prob_prv.size())
	{
		m_mem_prob_prv = m_mem_prob;
		return;
	}

	//compute count
	for (size_t i = 0; i < m_mem_prob[0].size(); ++i)
	{
		double var = 0;
		for (size_t j = 0; j < m_mem_prob.size(); ++j)
			var += fabs(m_mem_prob[j][i] -
				m_mem_prob_prv[j][i]);
		//size_t count = size_t(var / delta);
		//m_count[i] += count;
		m_count[i] += var;
	}
	//allocate histogram space
	m_histogram.clear();
	//fill in histogram
	for (size_t i = 0; i < m_count.size(); ++i)
	{
		//if (m_histogram.size() <= m_count[i])
		//	m_histogram.resize(m_count[i] + 1);
		//m_histogram[m_count[i]].count++;
		size_t index = static_cast<size_t>(m_count[i] / delta);
		if (m_histogram.size() <= index)
		{
			m_histogram.resize(index + 1);
			m_histogram[index].value = index * delta;
		}
		m_histogram[index].count++;
	}
	m_mem_prob_prv = m_mem_prob;

	if (output)
	{
		//output histogram
		std::ofstream outfile;
		outfile.open("hist.txt", std::ofstream::out |
			std::ofstream::app);
		for (size_t i = 0; i < m_histogram.size(); ++i)
			if (i < m_histogram.size() - 1)
				outfile << m_histogram[i].count << "\t";
			else
				outfile << m_histogram[i].count << "\n";
		outfile.close();
	}
}

void ClusterExmax::GenerateNewColors(void* label,
	size_t nx, size_t ny, size_t nz)
{
	//m_id_list.clear();

	unsigned int id;
	unsigned long long index;
	int i, j, k;

	double scale = m_histogram.back().value;
	scale = 350 / scale;

	unsigned int ii = 0;
	for (ClusterIter iter = m_data.begin();
		iter != m_data.end(); ++iter)
	{
		//id = m_count[ii] * 10 + 1;
		id = static_cast<unsigned int>(m_count[ii] * scale + 1);
		ii++;
		i = int(boost::qvm::A0((*iter)->centeri) + 0.5);
		if (i < 0 || i >= nx)
			continue;
		j = int(boost::qvm::A1((*iter)->centeri) + 0.5);
		if (j < 0 || j >= ny)
			continue;
		k = int(boost::qvm::A2((*iter)->centeri) + 0.5);
		if (k < 0 || k >= nz)
			continue;
		index = nx*ny*k + nx*j + i;
		((unsigned int*)label)[index] = id;
	}

}

void ClusterExmax::GenerateNewColors2(void* label,
	size_t nx, size_t ny, size_t nz)
{
	//m_id_list.clear();

	unsigned int id;
	int i, j, k;

	unsigned int ii = 0;
	for (ClusterIter iter = m_data.begin();
		iter != m_data.end(); ++iter)
	{
		size_t index = 0;
		bool changed = false;
		double max_mem_prob;
		id = 0;
		for (size_t j = 0; j < m_clnum; ++j)
		{
			if (j == 0)
			{
				index = j;
				max_mem_prob = m_mem_prob[j][ii];
				changed = true;
			}
			else
			{
				if (m_mem_prob[j][ii] > max_mem_prob)
				{
					index = j;
					max_mem_prob = m_mem_prob[j][ii];
					changed = true;
				}
			}
		}
		if (changed)
			id = static_cast<unsigned int>((1.0 - max_mem_prob) * 700 + 1);
		ii++;
		i = int(boost::qvm::A0((*iter)->centeri) + 0.5);
		if (i < 0 || i >= nx)
			continue;
		j = int(boost::qvm::A1((*iter)->centeri) + 0.5);
		if (j < 0 || j >= ny)
			continue;
		k = int(boost::qvm::A2((*iter)->centeri) + 0.5);
		if (k < 0 || k >= nz)
			continue;
		index = nx*ny*k + nx*j + i;
		((unsigned int*)label)[index] = id;
	}

}
