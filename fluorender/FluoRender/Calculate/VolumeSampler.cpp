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

using namespace FL;

VolumeSampler::VolumeSampler() :
	m_vd_r(0),
	m_vd(0),
	m_nx(0),
	m_ny(0),
	m_nz(0),
	m_type(0)
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

void VolumeSampler::Sample()
{
	if (!m_vd)
		return;
	if (m_nx <= 0 || m_ny <= 0 || m_nz <= 0)
		return;

	//create m_vd_r
	if (m_vd_r && m_vd_r->data)
	{
		delete[] m_vd_r->data;
		nrrdNix(m_vd_r);
	}

	int nx_in, ny_in, nz_in;
	nx_in = m_vd->axis[0].size;
	ny_in = m_vd->axis[1].size;
	nz_in = m_vd->axis[2].size;
	if (m_nx == nx_in && m_ny == ny_in && m_nz == nz_in)
	{
		nrrdCopy(m_vd_r, m_vd);
		return;
	}
	else
		m_vd_r = nrrdNew();

	int bits_in, bits_out;
	switch (m_vd->type)
	{
	case nrrdTypeChar:
	case nrrdTypeUChar:
		bits_in = 8;
		break;
	case nrrdTypeShort:
	case nrrdTypeUShort:
		bits_in = 16;
		break;
	}
}