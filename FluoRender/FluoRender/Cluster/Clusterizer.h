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
#ifndef _CLUSTERIZER_H_
#define _CLUSTERIZER_H_

#include <Progress.h>
#include <Cell.h>

namespace flrd
{
	class Clusterizer : public Progress
	{
	public:
		Clusterizer();
		~Clusterizer();

		void Compute();

		void SetMethod(int val) { m_method = val; }
		int GetMethod() { return m_method; }
		void SetNum(int val) { m_num = val; }
		int GetNum() { return m_num; }
		void SetMaxIter(int val) { m_maxiter = val; }
		int GetMaxIter() { return m_maxiter; }
		void SetTol(float val) { m_tol = val; }
		float GetTol() { return m_tol; }
		void SetSize(int val) { m_size = val; }
		int GetSize() { return m_size; }
		void SetEps(double val) { m_eps = val; }
		double GetEps() { return m_eps; }
		//in and out cell lists
		flrd::CelpList& GetInCells()
		{
			return m_in_cells;
		}
		flrd::CelpList& GetOutCells()
		{
			return m_out_cells;
		}

	protected:
		//settings
		int m_method;//0:exmax; 1:dbscan; 2:kmeans
		//parameters
		int m_num;
		int m_maxiter;
		float m_tol;
		int m_size;
		double m_eps;
		//in and out cell lists for tracking
		flrd::CelpList m_in_cells;
		flrd::CelpList m_out_cells;
	};
}
#endif//_CLUSTERIZER_H_
