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
#ifdef _DEBUG
#include <Debug.h>
#endif
#include <ExGauss.h>

using namespace flrd;

void ExGauss::Execute()
{
//#ifdef _DEBUG
//	DBMIINT32 mi;
//	mi.nx = nx; mi.ny = ny; mi.nc = 1; mi.nt = mi.nx * mi.nc * 4;
//	mi.data = front;
//#endif
	FindMin();
	for (unsigned int i = 1; ; ++i)
	{
		if (Flood(i))
			break;
	}
	FitGauss();
}

fluo::Point ExGauss::GetCenter(double x, double y, double z)
{
	return fluo::Point(min_init.x() + x, min_init.y() + y, min_init.z() +z);
}

double ExGauss::GetProb()
{
	return 0.0;
}

void ExGauss::FindMin()
{
	float minv = data[0];
	Coord minc = { 0, 0, 0 };
	cl.clear();
	unsigned int i, j, k;
	unsigned long long idx;
	for (k = 0; k < nz; ++k)
	for (j = 0; j < ny; ++j)
	for (i = 0; i < nx; ++i)
	{
		idx = (unsigned long long)k * nx * ny + j * nx + i;
		if (data[idx] < minv)
		{
			minv = data[idx];
			minc.z = k;
			minc.y = j;
			minc.x = i;
		}
	}
	idx = Index(minc);
	front[idx] = 1;
	cl.push_back(minc);
	min_init = fluo::Point(minc.x, minc.y, minc.z);
}

bool ExGauss::Flood(unsigned int i)
{
	std::list<Coord> cl2;
	for (auto it : cl)
	{
		unsigned long long idx;
		Coord c = it;
		float v = data[Index(c)];
		Coord nb;
		float nv;
		//-x
		nb.x = c.x - 1; nb.y = c.y; nb.z = c.z;
		idx = Index(nb);
		if (Valid(nb) && !front[idx])
		{
			nv = data[idx];
			if (nv > v)
			{
				cl2.push_back(nb);
				front[idx] = i + 1;
			}
		}
		//+x
		nb.x = c.x + 1; nb.y = c.y; nb.z = c.z;
		idx = Index(nb);
		if (Valid(nb) && !front[idx])
		{
			nv = data[idx];
			if (nv > v)
			{
				cl2.push_back(nb);
				front[idx] = i + 1;
			}
		}
		//-y
		nb.x = c.x; nb.y = c.y - 1; nb.z = c.z;
		idx = Index(nb);
		if (Valid(nb) && !front[idx])
		{
			nv = data[idx];
			if (nv > v)
			{
				cl2.push_back(nb);
				front[idx] = i + 1;
			}
		}
		//+y
		nb.x = c.x; nb.y = c.y + 1; nb.z = c.z;
		idx = Index(nb);
		if (Valid(nb) && !front[idx])
		{
			nv = data[idx];
			if (nv > v)
			{
				cl2.push_back(nb);
				front[idx] = i + 1;
			}
		}
		//-z
		nb.x = c.x; nb.y = c.y; nb.z = c.z - 1;
		idx = Index(nb);
		if (Valid(nb) && !front[idx])
		{
			nv = data[idx];
			if (nv > v)
			{
				cl2.push_back(nb);
				front[idx] = i + 1;
			}
		}
		//+z
		nb.x = c.x; nb.y = c.y; nb.z = c.z + 1;
		idx = Index(nb);
		if (Valid(nb) && !front[idx])
		{
			nv = data[idx];
			if (nv > v)
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
	double cx = 0, cy = 0, cz = 0, cw = 0;
	double v;
	unsigned int i, j, k;
	unsigned long long idx;

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
	for (k = 0; k < nz; ++k)
	for (j = 0; j < ny; ++j)
	for (i = 0; i < nx; ++i)
	{
		idx = (unsigned long long)k * nx * ny + j * nx + i;
		if (front[idx] > 0)
		{
			v = (data[idx] - minint) / range;
			cx += v * i;
			cy += v * j;
			cz += v * k;
			cw += v;
		}
	}
	if (cw > 0)
		mean = fluo::Point(cx, cy, cz) / cw;
}

