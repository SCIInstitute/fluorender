/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2022 Scientific Computing and Imaging Institute,
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
#ifndef FL_Kmeans_h
#define FL_Kmeans_h

#include "ClusterMethod.h"

namespace flrd
{
	class ClusterKmeans : public ClusterMethod
	{
	public:
		ClusterKmeans();
		~ClusterKmeans();

		void SetClnum(unsigned int num)
		{ m_clnum = num; }
		void SetMaxiter(size_t num)
		{ m_max_iter = num; }
		bool Execute();
		float GetProb();

	private:
		//cluster number
		unsigned int m_clnum;
		//distance measure for convergence
		float m_eps;
		//maximum iteration number
		size_t m_max_iter;
		std::vector<EmVec> m_means;
		std::vector<EmVec> m_means_prv;

	private:
		void Initialize();
		void Assign();
		void Update();
		bool Converge();
	};

}
#endif//FL_Dbscan_h