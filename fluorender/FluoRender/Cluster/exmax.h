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
#ifndef FL_Exmax_h
#define FL_Exmax_h

#include "ClusterMethod.h"
#include <FLIVR/Matrix.h>

namespace FL
{
	class ClusterExmax : public ClusterMethod
	{
	public:
		ClusterExmax();
		~ClusterExmax();

		void SetClnum(unsigned int num)
		{ m_clnum = num; }
		bool Execute();

	private:
		//cluster number
		unsigned int m_clnum;
		//measure for convergence
		float m_eps;
		//maximum iteration number
		size_t m_max_iter;

		//parameters to estimate
		typedef struct
		{
			double tau;
			FLIVR::Point mean;
			FLIVR::Mat3 covar;
		} Params;
		//all paramters to estimate
		std::vector<Params> m_params;
		std::vector<Params> m_params_prv;
		//likelihood
		double m_likelihood;
		double m_likelihood_prv;
		//membership probabilities
		std::vector<std::vector<double>> m_mem_prob;//0-idx: comps; 1-idx: points

	private:
		void Initialize();
		void Expectation();
		double Gaussian(FLIVR::Point &p, FLIVR::Point &m, FLIVR::Mat3 &s);
		void Maximization();
		bool Converge();
		void GenResult();
	};

}
#endif//FL_Exmax_h