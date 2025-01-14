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
#ifndef EXGAUSS_H
#define EXGAUSS_H

#include <Point.h>
#include <list>

namespace flrd
{
	class ExGauss
	{
	public:
		ExGauss(unsigned int x, unsigned int y, unsigned int z) :
			nx(x), ny(y), nz(z),
			m_l(1e-3),
			m_max_iter(0),
			prob(0)
		{
			data = new float[(unsigned long long)nx * ny * nz]();
			front = new unsigned int[(unsigned long long)nx * ny * nz]();
		};
		~ExGauss()
		{
			delete[] data;
			delete[] front;
		};

		void SetIter(size_t iter, float eps)
		{
			m_max_iter = iter;
			m_l = eps;
		}
		void SetData(unsigned int x, unsigned int y, unsigned int z, float v)
		{
			unsigned long long idx = (unsigned long long)z * nx * ny + y * nx + x;
			data[idx] = v;
		}

		void Execute();
		fluo::Point GetCenter();
		double GetProb();

	private:
		unsigned int nx;
		unsigned int ny;
		unsigned int nz;
		float* data;
		unsigned int* front;
		struct Coord
		{
			int x;
			int y;
			int z;
		};
		std::list<Coord> cl;
		fluo::Point exetr;
		float exetrval;
		fluo::Point mean;
		double prob;
		//maximum iteration number
		size_t m_max_iter;
		float m_l;

	private:
		void FindExetr();
		bool Flood(unsigned int i);
		void FitGauss();
		unsigned long long Index(Coord &c)
		{
			return (unsigned long long)c.z * nx * ny + c.y * nx + c.x;
		}
		bool Valid(Coord &c)
		{
			return c.x >= 0 && c.x < nx &&
				c.y >= 0 && c.y < ny &&
				c.z >= 0 && c.z < nz;
		}
	};
}
#endif//EXGAUSS_H
