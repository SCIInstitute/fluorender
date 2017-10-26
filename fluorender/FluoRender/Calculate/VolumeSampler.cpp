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
#include "VolumeSampler.h"
#include <stdexcept>

using namespace FL;

VolumeSampler::VolumeSampler() :
	m_vd_r(0),
	m_vd(0),
	m_nx(0),
	m_ny(0),
	m_nz(0),
	m_nx_in(0),
	m_ny_in(0),
	m_nz_in(0),
	m_bits(0),
	m_bits_in(0),
	m_spcx_in(1.0),
	m_spcy_in(1.0),
	m_spcz_in(1.0),
	m_type(0),
	m_fx(0),
	m_fy(0),
	m_fz(0),
	m_border(0)
{
}

VolumeSampler::~VolumeSampler()
{
}

void VolumeSampler::SetVolume(Nrrd *data)
{
	m_vd = data;
}

Nrrd* VolumeSampler::GetVolume()
{
	return m_vd;
}

Nrrd* VolumeSampler::GetResult()
{
	return m_vd_r;
}

void VolumeSampler::SetSize(int nx, int ny, int nz)
{
	m_nx = nx;
	m_ny = ny;
	m_nz = nz;
}

void VolumeSampler::SetSpacings(double spcx, double spcy, double spcz)
{
	m_spcx_in = spcx;
	m_spcy_in = spcy;
	m_spcz_in = spcz;
}

void VolumeSampler::SetType(int type)
{
	m_type = type;
}

void VolumeSampler::SetFilterSize(int fx, int fy, int fz)
{
	m_fx = fx;
	m_fy = fy;
	m_fz = fz;
}

void VolumeSampler::Resize()
{
	if (!m_vd)
		return;
	if (m_nx <= 0 || m_ny <= 0 || m_nz <= 0)
		return;

	//create m_vd_r
	if (m_vd_r && m_vd_r->data)
		nrrdNuke(m_vd_r);

	
	m_nx_in = m_vd->axis[0].size;
	m_ny_in = m_vd->axis[1].size;
	m_nz_in = m_vd->axis[2].size;
	if (m_nx == m_nx_in && m_ny == m_ny_in && m_nz == m_nz_in)
	{
		nrrdCopy(m_vd_r, m_vd);
		return;
	}
	else
		m_vd_r = nrrdNew();

	switch (m_vd->type)
	{
	case nrrdTypeChar:
	case nrrdTypeUChar:
		m_bits_in = 8;
		break;
	case nrrdTypeShort:
	case nrrdTypeUShort:
		m_bits_in = 16;
		break;
	case nrrdTypeInt:
	case nrrdTypeUInt:
		m_bits_in = 32;
		break;
	}
	m_bits = m_bits_in;

	void *data = 0;
	unsigned long long total_size = (unsigned long long)m_nx*
		(unsigned long long)m_ny*(unsigned long long)m_nz;
	data = (void*)(new unsigned char[total_size * (m_bits /8)]);
	if (!data)
		throw std::runtime_error("Unable to allocate memory.");

	unsigned long long index;
	int i, j, k;
	double x, y, z;
	double value;
	for (k = 0; k < m_nz; ++k)
	for (j = 0; j < m_ny; ++j)
	for (i = 0; i < m_nx; ++i)
	{
		index = (unsigned long long)m_nx*(unsigned long long)m_ny*
			(unsigned long long)k + (unsigned long long)m_nx*
			(unsigned long long)j + (unsigned long long)i;
		x = (double(i) + 0.5) / double(m_nx);
		y = (double(j) + 0.5) / double(m_ny);
		z = (double(k) + 0.5) / double(m_nz);
		if (m_bits == 32)
			((unsigned int*)data)[index] = SampleInt(x, y, z);
		else
		{
			value = Sample(x, y, z);
			if (m_bits == 8)
				((unsigned char*)data)[index] = (unsigned char)(value * 255);
			else if (m_bits == 16)
				((unsigned short*)data)[index] = (unsigned short)(value * 65535);
		}
	}

	//write to nrrd
	if (m_bits == 8)
		nrrdWrap(m_vd_r, (uint8_t*)data, nrrdTypeUChar,
			3, (size_t)m_nx, (size_t)m_ny, (size_t)m_nz);
	else if (m_bits == 16)
		nrrdWrap(m_vd_r, (uint16_t*)data, nrrdTypeUShort,
			3, (size_t)m_nx, (size_t)m_ny, (size_t)m_nz);
	else if (m_bits == 32)
		nrrdWrap(m_vd_r, (uint32_t*)data, nrrdTypeUInt,
			3, (size_t)m_nx, (size_t)m_ny, (size_t)m_nz);

	double spcx, spcy, spcz;
	spcx = m_spcx_in * double(m_nx_in) / double(m_nx);
	spcy = m_spcy_in * double(m_ny_in) / double(m_ny);
	spcz = m_spcz_in * double(m_nz_in) / double(m_nz);
	nrrdAxisInfoSet(m_vd_r, nrrdAxisInfoSpacing, spcx, spcy, spcz);
	nrrdAxisInfoSet(m_vd_r, nrrdAxisInfoMax, spcx*m_nx,
		spcy*m_ny, spcz*m_nz);
	nrrdAxisInfoSet(m_vd_r, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
	nrrdAxisInfoSet(m_vd_r, nrrdAxisInfoSize, (size_t)m_nx,
		(size_t)m_ny, (size_t)m_nz);
}

double VolumeSampler::Sample(double x, double y, double z)
{
	switch (m_type)
	{
	case 0:
		return SampleNearestNeighbor(x, y, z);
	case 1:
		return SampleLinear(x, y, z);
	case 2:
		return SampleBox(x, y, z);
	}
	return 0.0;
}

unsigned int VolumeSampler::SampleInt(double x, double y, double z)
{
	int i, j, k;
	xyz2ijk(x, y, z, i, j, k);
	if (!ijk(i, j, k))
		return 0.0;
	unsigned long long index = (unsigned long long)m_nx_in*(unsigned long long)m_ny_in*
		(unsigned long long)k + (unsigned long long)m_nx_in*
		(unsigned long long)j + (unsigned long long)i;
	return ((unsigned int*)(m_vd->data))[index];
}

bool VolumeSampler::ijk(int &i, int &j, int &k)
{
	if (i < 0)
	{
		switch (m_border)
		{
		case 0:
			return false;
		case 1:
			i = 0;
			break;
		case 2:
			i = -1 - i;
			break;
		}
	}
	if (i >= m_nx_in)
	{
		switch (m_border)
		{
		case 0:
			return false;
		case 1:
			i = m_nx_in - 1;
			break;
		case 2:
			i = m_nx_in * 2 - i - 1;
		}
	}
	if (j < 0)
	{
		switch (m_border)
		{
		case 0:
			return false;
		case 1:
			j = 0;
			break;
		case 2:
			j = -1 - j;
			break;
		}
	}
	if (j >= m_ny_in)
	{
		switch (m_border)
		{
		case 0:
			return false;
		case 1:
			j = m_ny_in - 1;
			break;
		case 2:
			j = m_ny_in * 2 - j - 1;
		}
	}
	if (k < 0)
	{
		switch (m_border)
		{
		case 0:
			return false;
		case 1:
			k = 0;
			break;
		case 2:
			k = -1 - k;
			break;
		}
	}
	if (k >= m_nz_in)
	{
		switch (m_border)
		{
		case 0:
			return false;
		case 1:
			k = m_nz_in - 1;
			break;
		case 2:
			k = m_nz_in * 2 - k - 1;
		}
	}
	return true;
}

void VolumeSampler::xyz2ijk(double x, double y, double z,
	int &i, int &j, int &k)
{
	i = int(x*m_nx_in);
	j = int(y*m_ny_in);
	k = int(z*m_nz_in);
}

double VolumeSampler::SampleNearestNeighbor(double x, double y, double z)
{
	int i, j, k;
	xyz2ijk(x, y, z, i, j, k);
	if (!ijk(i, j, k))
		return 0.0;
	unsigned long long index = (unsigned long long)m_nx_in*(unsigned long long)m_ny_in*
		(unsigned long long)k + (unsigned long long)m_nx_in*
		(unsigned long long)j + (unsigned long long)i;
	if (m_bits_in == 8)
		return double(((unsigned char*)(m_vd->data))[index]) / 255.0;
	else if (m_bits_in == 16)
		return double(((unsigned short*)(m_vd->data))[index]) / 65535.0;
	return 0.0;
}

double VolumeSampler::SampleLinear(double x, double y, double z)
{
	return 0;
}

double VolumeSampler::SampleBox(double x, double y, double z)
{
	int i, j, k;
	xyz2ijk(x, y, z, i, j, k);
	double sum = 0.0;
	int count = 0;
	unsigned long long index;
	for (int kk=k-m_fz; kk<=k+m_fz; ++kk)
	for (int jj=j-m_fy; jj<=j+m_fy; ++jj)
	for (int ii=i-m_fx; ii<=i+m_fx; ++ii)
	{
		if (ijk(ii, jj, kk))
		{
			index = (unsigned long long)m_nx_in*(unsigned long long)m_ny_in*
				(unsigned long long)kk + (unsigned long long)m_nx_in*
				(unsigned long long)jj + (unsigned long long)ii;
			if (m_bits_in == 8)
				sum += double(((unsigned char*)(m_vd->data))[index]) / 255.0;
			else if (m_bits_in == 16)
				sum += double(((unsigned short*)(m_vd->data))[index]) / 65535.0;
		}
		count++;
	}
	if (count)
		sum /= count;
	index = (unsigned long long)m_nx_in*(unsigned long long)m_ny_in*
		(unsigned long long)k + (unsigned long long)m_nx_in*
		(unsigned long long)j + (unsigned long long)i;
	double test = double(((unsigned char*)(m_vd->data))[index]) / 255.0;
	return sum;
}
