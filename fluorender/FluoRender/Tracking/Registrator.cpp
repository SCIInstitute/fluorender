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

#include "Registrator.h"
#include <DataManager.h>
#include "Stencil.h"
#include "StencilCompare.h"
#ifdef _DEBUG
#include <Debug.h>
#endif

using namespace flrd;

Registrator::Registrator() :
	m_extt(1),
	m_exta(1),
	m_iter(20),
	m_method(1),//0-dot product; 1-diff squared
	m_fsize(2)
{

}

Registrator::~Registrator()
{
	UnregisterCacheQueueFuncs();
}

bool Registrator::Run(size_t f1, size_t f2,
	int mode, size_t start)
{
	//get data
	size_t f0 = mode == 1 ? start : f1;
	m_vol_cache.set_max_size(2);
	VolCache cache = m_vol_cache.get(f0);
	m_vol_cache.protect(f0);
	void* data1 = cache.data;
	if (!data1)
		return false;
	cache = m_vol_cache.get(f2);
	void* data2 = cache.data;
	if (!data2)
		return false;

	int nx, ny, nz;
	m_vd->GetResolution(nx, ny, nz);
	Stencil s1, s2;
	s1.data = data1;
	s2.data = data2;
	s1.nx = s2.nx = nx;
	s1.ny = s2.ny = ny;
	s1.nz = s2.nz = nz;
	s1.bits = s2.bits = m_vd->GetBits();
	s1.scale = s2.scale = m_vd->GetScalarScale();
	s1.fsize = s2.fsize = m_fsize;
	s1.box = s2.box = fluo::BBox(fluo::Point(0), fluo::Point(nx, ny, nz));

	fluo::Vector off1, off2;
	StencilCompare compare(&s1, &s2,
		m_extt, m_exta, off1, off2,
		m_iter, m_method);
	if (compare.Compare())
	{
		//get transformation
		m_center = compare.GetCenter();
		m_euler = compare.GetEuler();
	}

	return true;
}
