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
#ifndef FL_Exmax1_h
#define FL_Exmax1_h

#include <Cluster/ClusterMethod.h>
#include <boost/qvm/vec.hpp>
#include <boost/qvm/mat.hpp>
#include <boost/qvm/vec_operations.hpp>
#include <boost/shared_ptr.hpp>
#include <list>
#include <vector>

namespace flrd
{
	class ExMax1
	{
	public:
		ExMax1():
			m_eps(1e-6f),
			m_max_iter(100),
			m_spc({1, 1, 1})
		{};
		~ExMax1() {};

		void SetData(Cluster &data)
		{
			m_data = data;
		}
		Cluster &GetData()
		{
			return m_data;
		}
		void SetSpacings(double spcx, double spcy, double spcz)
		{
			m_spc = { spcx, spcy, spcz };
		}
		void AddClusterPoint(const EmVec &p, const float value);
		bool Execute();
		fluo::Point GetCenter();
		float GetProb();

	private:
		Cluster m_data;
		//ClusterSet m_result;
		EmVec m_spc;//spacings
		//measure for convergence
		float m_eps;
		//maximum iteration number
		size_t m_max_iter;

		//parameters to estimate
		struct Params
		{
			double tau;
			EmVec mean;
			EmMat covar;
		};
		//all paramters to estimate
		Params m_params;
		Params m_params_prv;
		//likelihood
		double m_likelihood;
		double m_likelihood_prv;
		//membership probabilities
		std::vector<double> m_mem_prob;//0-idx: comps; 1-idx: points

		void Initialize();
		void Expectation();
		double Gaussian(EmVec &p, EmVec &m, EmMat &s);
		void Maximization();
		bool Converge();
	};
}
#endif//FL_Exmax1_h
