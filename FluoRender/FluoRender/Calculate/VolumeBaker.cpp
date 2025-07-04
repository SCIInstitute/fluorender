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
#include <VolumeBaker.h>
#include <DataManager.h>
#include <Texture.h>
#include <stdexcept>

using namespace flrd;

VolumeBaker::VolumeBaker() :
	m_raw_input(0),
	m_raw_result(0),
	m_nx(0),
	m_ny(0),
	m_nz(0),
	m_bits(0)
{

}

VolumeBaker::~VolumeBaker()
{
	//if (m_result)
	//	delete m_result;
}

void VolumeBaker::SetInput(const std::shared_ptr<VolumeData>& data)
{
	m_input = data;
}

std::shared_ptr<VolumeData> VolumeBaker::GetInput()
{
	return m_input.lock();
}

std::shared_ptr<VolumeData> VolumeBaker::GetResult()
{
	return m_result;
}

void VolumeBaker::Bake(bool replace)
{
	auto input = m_input.lock();
	if (!input)
		return;
	m_raw_input = GetRaw(input.get());
	Nrrd* input_nrrd = GetNrrd(input.get());
	if (!input_nrrd)
		return;

	//input size
	m_nx = static_cast<int>(input_nrrd->axis[0].size);
	m_ny = static_cast<int>(input_nrrd->axis[1].size);
	m_nz = static_cast<int>(input_nrrd->axis[2].size);
	//bits
	switch (input_nrrd->type)
	{
	case nrrdTypeChar:
	case nrrdTypeUChar:
		m_bits = 8;
		break;
	case nrrdTypeShort:
	case nrrdTypeUShort:
		m_bits = 16;
		break;
	case nrrdTypeInt:
	case nrrdTypeUInt:
		m_bits = 32;
		break;
	}

	//output raw
	unsigned long long total_size = (unsigned long long)m_nx*
		(unsigned long long)m_ny*(unsigned long long)m_nz;
	m_raw_result = (void*)(new unsigned char[total_size * (m_bits / 8)]);
	if (!m_raw_result)
		return;

	//transfer function
	unsigned long long index;
	for (int i = 0; i < m_nx; i++)
	for (int j = 0; j < m_ny; j++)
	for (int k = 0; k < m_nz; k++)
	{
		index = (unsigned long long)m_nx * m_ny * k +
			(unsigned long long)m_nx * j + i;
		fluo::Point p(double(i) / double(m_nx),
			double(j) / double(m_ny),
			double(k) / double(m_nz));
		double new_value = input->GetTransferedValue(i, j, k);
		if (m_bits == 8)
			((unsigned char*)m_raw_result)[index] = uint8_t(new_value*255.0);
		else if (m_bits == 16)
			((unsigned short*)m_raw_result)[index] = uint16_t(new_value*65535.0);
	}

	//write to nrrd
	Nrrd* nrrd_result = nrrdNew();
	if (m_bits == 8)
		nrrdWrap_va(nrrd_result, (uint8_t*)m_raw_result, nrrdTypeUChar,
			3, (size_t)m_nx, (size_t)m_ny, (size_t)m_nz);
	else if (m_bits == 16)
		nrrdWrap_va(nrrd_result, (uint16_t*)m_raw_result, nrrdTypeUShort,
			3, (size_t)m_nx, (size_t)m_ny, (size_t)m_nz);

	//spacing
	double spcx, spcy, spcz;
	input->GetSpacings(spcx, spcy, spcz);
	nrrdAxisInfoSet_va(nrrd_result, nrrdAxisInfoSpacing, spcx, spcy, spcz);
	nrrdAxisInfoSet_va(nrrd_result, nrrdAxisInfoMax, spcx*m_nx,
		spcy*m_ny, spcz*m_nz);
	nrrdAxisInfoSet_va(nrrd_result, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
	nrrdAxisInfoSet_va(nrrd_result, nrrdAxisInfoSize, (size_t)m_nx,
		(size_t)m_ny, (size_t)m_nz);

	if (replace)
	{
		input->Replace(nrrd_result, true);
	}
	else
	{
		if (!m_result)
		{
			m_result = std::make_shared<VolumeData>();
			std::wstring name, path;
			m_result->Load(nrrd_result, name, path);
		}
		else
			m_result->Replace(nrrd_result, false);
		m_result->SetSpacings(spcx, spcy, spcz);
		m_result->SetBaseSpacings(spcx, spcy, spcz);
	}
}

Nrrd* VolumeBaker::GetNrrd(VolumeData* vd)
{
	if (!vd || !vd->GetTexture())
		return 0;
	flvr::Texture* tex = vd->GetTexture();
	return tex->get_nrrd(0);
}

void* VolumeBaker::GetRaw(VolumeData* vd)
{
	Nrrd* nrrd = GetNrrd(vd);
	if (nrrd)
		return nrrd->data;
	return 0;
}

