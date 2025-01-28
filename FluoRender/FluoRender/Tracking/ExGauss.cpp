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
#include <ExGauss.h>
#include <exmax1.h>

using namespace flrd;

void ExGauss::Execute()
{
#ifdef _DEBUG
	DBMIINT32 mi;
	mi.nx = nx; mi.ny = ny; mi.nc = 1; mi.nt = mi.nx * mi.nc * 4;
	mi.data = front;
	DBMIFLOAT32 mi2;
	mi2.nx = nx; mi2.ny = ny; mi2.nc = 1; mi2.nt = mi2.nx * mi2.nc * 4;
	mi2.data = data;
#endif
	FindExetr();
	if (m_max_iter > 0)
	{
		for (unsigned int i = 1; ; ++i)
		{
			if (Flood(i))
				break;
		}
		FitGauss();
	}
}

fluo::Point ExGauss::GetCenter()
{
	if (m_max_iter)
	{
		if (prob > 0.5)
			return mean;
		else
			return fluo::Point(
				nx > 1 ? double(nx) / 2 : 0,
				ny > 1 ? double(ny) / 2 : 0,
				nz > 1 ? double(nz) / 2 : 0);
	}
	return exetr;
}

double ExGauss::GetProb()
{
	return prob;
}

void ExGauss::FindExetr()
{
	float v = data[0];//max
	float v2 = data[0];//min
	Coord c = { 0, 0, 0 };
	cl.clear();
	unsigned int i, j, k;
	unsigned long long idx;
	for (k = 0; k < nz; ++k)
	for (j = 0; j < ny; ++j)
	for (i = 0; i < nx; ++i)
	{
		idx = (unsigned long long)k * nx * ny + j * nx + i;
		if (data[idx] > v)
		{
			v = data[idx];
			c.z = k;
			c.y = j;
			c.x = i;
		}
		if (data[idx] < v2)
			v2 = data[idx];
	}
	idx = Index(c);
	front[idx] = 1;
	cl.push_back(c);
	exetr = fluo::Point(c.x, c.y, c.z);
	exetrval = v - v2;
}

bool ExGauss::Flood(unsigned int i)
{
	float eps = -1e-2 * exetrval;
	std::list<Coord> cl2;
	for (auto it : cl)
	{
		unsigned long long idx;
		Coord c = it;
		float v = data[Index(c)];
		Coord nb;
		float nv;
		for (int ii = -1; ii < 2; ++ii)
		for (int j = -1; j < 2; ++j)
		for (int k = -1; k < 2; ++k)
		{
			if (ii == 0 && j == 0 && k == 0)
				continue;
			nb.x = c.x + ii;
			nb.y = c.y + j;
			nb.z = c.z + k;
			if (!Valid(nb))
				continue;
			idx = Index(nb);
			if (front[idx])
				continue;
			nv = data[idx];
			if (v - nv > eps)
			{
				cl2.push_back(nb);
				front[idx] = i + 1;
			}
		}
	}

	cl = cl2;
	return cl.empty();
}

void ExGauss::FitGauss()
{
	float v;
	unsigned long long idx;

	//find range
	float minint, maxint, range;
	bool setmm = false;
	unsigned long long data_size = (unsigned long long)nx * ny * nz;
	for (idx = 0; idx < data_size; ++idx)
	{
		if (front[idx] > 0)
		{
			if (!setmm)
			{
				minint = data[idx];
				maxint = data[idx];
				setmm = true;
				continue;
			}
			minint = data[idx] < minint ? data[idx] : minint;
			maxint = data[idx] > maxint ? data[idx] : maxint;
		}
	}
	range = maxint - minint;
	if (range == 0) return;

	//normalize
	for (idx = 0; idx < data_size; ++idx)
		data[idx] = (data[idx] - minint) / range;

	//get gauss
	ExMax1 em;
	for (unsigned int k = 0; k < nz; ++k)
	for (unsigned int j = 0; j < ny; ++j)
	for (unsigned int i = 0; i < nx; ++i)
	{
		idx = (unsigned long long)k * nx * ny + j * nx + i;
		if (front[idx] == 0) continue;
		v = data[idx];
		EmVec pnt = {
			static_cast<double>(i),
			static_cast<double>(j),
			static_cast<double>(k) };
		em.AddClusterPoint(pnt, v);
	}
	em.SetIter(m_max_iter, m_l);
	em.Execute();
	mean = em.GetCenter();
	prob = em.GetProb();
}

