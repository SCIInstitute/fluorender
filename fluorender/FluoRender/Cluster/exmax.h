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
#ifndef FL_Exmax_h
#define FL_Exmax_h

#include "ClusterMethod.h"

namespace FL
{
	class ClusterExmax : public ClusterMethod
	{
	public:
		ClusterExmax();
		~ClusterExmax();

		void SetClnum(unsigned int num)
		{ m_clnum = num; }
		void SetMaxiter(size_t num)
		{ m_max_iter = num; }
		bool Execute();
		float GetProb();
		void SetWeakResult(bool result = true)
		{ m_weak_result = result; }

		//for test
		void GenerateNewColors(void* label,
			size_t nx, size_t ny, size_t nz);
		void GenerateNewColors2(void* label,
			size_t nx, size_t ny, size_t nz);

	private:
		//cluster number
		unsigned int m_clnum;
		//try to match the cluster
		bool m_weak_result;
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
		std::vector<Params> m_params;
		std::vector<Params> m_params_prv;
		//likelihood
		double m_likelihood;
		double m_likelihood_prv;
		//membership probabilities
		std::vector<std::vector<double>> m_mem_prob;//0-idx: comps; 1-idx: points
		std::vector<std::vector<double>> m_mem_prob_prv;//for comparison
		std::vector<double> m_count;//count for changes

		//histogram
		typedef struct
		{
			double value;//starting value of the range
			size_t count;
		} EmBin;
		std::vector<EmBin> m_histogram;
		size_t m_bins;

	private:
		void Initialize();
		void Init1();
		void Init2();
		void Expectation();
		double Gaussian(EmVec &p, EmVec &m, EmMat &s);
		void Maximization();
		bool Converge();
		void GenResult();
		void GenHistogram(size_t bins);
		void GenUncertainty(double delta, bool output = false);
	};

}
#endif//FL_Exmax_h