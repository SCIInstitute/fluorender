﻿/*
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

#include <Registrator.h>
#include <Global.h>
#include <Stencil.h>
#include <StencilCompare.h>
#ifdef _DEBUG
#include <Debug.h>
#endif

using namespace flrd;

Registrator::Registrator() :
	m_extt(1),
	m_exta(1),
	m_iter(50),
	m_conv_num(4),
	m_method(1),//0-dot product; 1-diff squared
	m_fsize(2)
{
	m_tf.load_identity();
}

Registrator::~Registrator()
{
}

bool Registrator::Run(size_t f1, size_t f2,
	int mode, size_t start)
{
	//get data
	size_t f0 = mode == 1 ? start : f1;
	//size_t f0 = start;
	glbin_cache_queue.set_max_size(2);
	VolCache cache = glbin_cache_queue.get(f0);
	if (!cache.data)
		return false;
	glbin_cache_queue.protect(f0);
	void *data1 = 0, *data2 = 0, *mask1 = 0;
	data1 = cache.data;
	if (m_use_mask && cache.mask)
		mask1 = cache.mask;
	cache = glbin_cache_queue.get(f2);
	if (!cache.data)
		return false;
	data2 = cache.data;

	int nx, ny, nz;
	m_vd->GetResolution(nx, ny, nz);
	Stencil s1, s2;
	s1.data = data1;
	s1.mask = mask1;
	s2.data = data2;
	s1.nx = s2.nx = nx;
	s1.ny = s2.ny = ny;
	s1.nz = s2.nz = nz;
	int bits = m_vd->GetBits();
	s1.bits = s2.bits = bits;
	s1.scale = s2.scale = m_vd->GetScalarScale();
	s1.max_int = s2.max_int = m_vd->GetMaxValue();
	s1.fsize = s2.fsize = m_fsize;
	fluo::BBox extent(fluo::Point(0), fluo::Point(nx, ny, nz));
	if (m_use_mask)
	{
		if (mask1)
			extent = GetExtent(mask1, nx, ny, nz);
		if (!extent.valid() || !mask1)
		{
			extent = fluo::BBox(fluo::Point(0), fluo::Point(nx, ny, nz));
			m_use_mask = false;
		}
	}
	s1.box = s2.box = extent;
	if (mode == 0)
	{
		m_offt = fluo::Vector(m_translate);
		m_offa = -fluo::Vector(m_euler);
	}
	else
	{
		m_offt = fluo::Vector();
		m_offa = fluo::Vector();
	}

	StencilCompare compare(&s1, &s2,
		m_extt, m_exta, m_offt, m_offa,
		m_iter, m_conv_num, m_method,
		m_use_mask);
	if (compare.Compare())
	{
		//get transformation
		//if (mode == 1)
		//{
			m_translate = compare.GetTranslate();
			m_center = compare.GetCenter();
			m_euler = compare.GetEuler();
			m_tf = s2.tf;
		//}
		//else
		//{
		//	m_translate += compare.GetTranslate();
		//	m_center = compare.GetCenter();
		//	m_euler += compare.GetEuler();
		//	m_tf.post_trans(s2.tf);
		//}
	}

	glbin_cache_queue.unprotect(f0);
	return true;
}

fluo::Point Registrator::GetTranslateVol()
{
	fluo::Point result = m_translate;
	double dx = 1, dy = 1, dz = 1;
	if (m_vd)
		m_vd->GetSpacings(dx, dy, dz);
	result.x(result.x() * dx);
	result.y(result.y() * dy);
	result.z(result.z() * dz);
	return result;
}

fluo::Point Registrator::GetCenterVol()
{
	fluo::Point result = m_center;
	double dx = 1, dy = 1, dz = 1;
	if (m_vd)
		m_vd->GetSpacings(dx, dy, dz);
	result.x(result.x() * dx);
	result.y(result.y() * dy);
	result.z(result.z() * dz);
	return result;
}

fluo::BBox Registrator::GetExtent(void* mask, int nx, int ny, int nz)
{
	fluo::BBox result;
	unsigned long long index;
	unsigned char* data = (unsigned char*)mask;
	for (int k = 0; k < nz; ++k) for (int j = 0; j < ny; ++j) for (int i = 0; i < nx; ++i)
	{
		index = (unsigned long long)nx * ny * k +
			(unsigned long long)nx * j + (unsigned long long)i;
		if (data[index])
			result.extend(fluo::Point(i, j, k));
	}
	return result;
}