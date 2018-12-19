/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2018 Scientific Computing and Imaging Institute,
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
#include "Compare.h"
#include <Scenegraph/VolumeData.h>
#include <FLIVR/VolumeRenderer.h>
#include <FLIVR/KernelProgram.h>
#include <FLIVR/VolKernel.h>
#include <FLIVR/TextureBrick.h>
#include <FLIVR/Texture.h>
#include "cl_code.h"
#include <algorithm>

using namespace FL;

ChannelCompare::ChannelCompare(VolumeData* vd1, VolumeData* vd2)
	: m_vd1(vd1), m_vd2(vd2),
	m_use_mask(false),
	m_init(false)
{
}

ChannelCompare::~ChannelCompare()
{
}

bool ChannelCompare::CheckBricks()
{
	if (!m_vd1)
		return false;
	if (!m_vd2)
		return false;
	if (!m_vd1->GetTexture())
		return false;
	if (!m_vd2->GetTexture())
		return false;
	int brick_num1 = m_vd1->GetTexture()->get_brick_num();
	int brick_num2 = m_vd2->GetTexture()->get_brick_num();
	if (!brick_num1 || !brick_num2 || brick_num1 != brick_num2)
		return false;
	return true;
}

bool ChannelCompare::GetInfo(
	FLIVR::TextureBrick* b1, FLIVR::TextureBrick* b2,
	long &bits1, long &bits2,
	long &nx, long &ny, long &nz)
{
	bits1 = b1->nb(m_use_mask ? b1->nmask() : 0)*8;
	bits2 = b2->nb(m_use_mask ? b2->nmask() : 0)*8;
	long nx1 = b1->nx();
	long nx2 = b2->nx();
	long ny1 = b1->ny();
	long ny2 = b2->ny();
	long nz1 = b1->nz();
	long nz2 = b2->nz();
	if (nx1 != nx2 || ny1 != ny2 || nz1 != nz2)
		return false;
	nx = nx1; ny = ny1; nz = nz1;
	return true;
}

void* ChannelCompare::GetVolDataBrick(FLIVR::TextureBrick* b)
{
	if (!b)
		return 0;

	long nx, ny, nz;
	int bits = 8;
	int c = 0;
	int nb = 1;

	c = m_use_mask ? b->nmask() : 0;
	nb = b->nb(c);
	nx = b->nx();
	ny = b->ny();
	nz = b->nz();
	bits = nb * 8;
	unsigned long long mem_size = (unsigned long long)nx*
		(unsigned long long)ny*(unsigned long long)nz*(unsigned long long)nb;
	unsigned char* temp = new unsigned char[mem_size];
	unsigned char* tempp = temp;
	unsigned char* tp = (unsigned char*)(b->tex_data(c));
	unsigned char* tp2;
	for (unsigned int k = 0; k < nz; ++k)
	{
		tp2 = tp;
		for (unsigned int j = 0; j < ny; ++j)
		{
			memcpy(tempp, tp2, nx*nb);
			tempp += nx * nb;
			tp2 += b->sx()*nb;
		}
		tp += b->sx()*b->sy()*nb;
	}
	return (void*)temp;
}

void* ChannelCompare::GetVolData(VolumeData* vd)
{
	long nx, ny, nz;
	vd->getValue("res x", nx);
	vd->getValue("res y", ny);
	vd->getValue("res z", nz);
	Nrrd* nrrd_data = 0;
	if (m_use_mask)
		nrrd_data = vd->GetMask(false);
	if (!nrrd_data)
		nrrd_data = vd->GetData(false);
	if (!nrrd_data)
		return 0;
	return nrrd_data->data;
}

void ChannelCompare::ReleaseData(void* val, long bits)
{
	if (bits == 8)
	{
		unsigned char* temp = (unsigned char*)val;
		delete[] temp;
	}
	else if (bits == 16)
	{
		unsigned short* temp = (unsigned short*)val;
		delete[] temp;
	}
}

void ChannelCompare::Compare(float threshold)
{
	m_result = 0.0;

	if (!CheckBricks())
		return;

	//create program and kernels
	FLIVR::KernelProgram* kernel_prog = FLIVR::VolumeRenderer::
		vol_kernel_factory_.kernel(str_cl_chann_compare);
	if (!kernel_prog)
		return;
	int kernel_index = -1;
	string name = "kernel_0";
	if (kernel_prog->valid())
		kernel_index = kernel_prog->findKernel(name);
	else
		kernel_index = kernel_prog->createKernel(name);

	size_t brick_num = m_vd1->GetTexture()->get_brick_num();
	vector<FLIVR::TextureBrick*> *bricks1 = m_vd1->GetTexture()->get_bricks();
	vector<FLIVR::TextureBrick*> *bricks2 = m_vd2->GetTexture()->get_bricks();

	for (size_t i = 0; i < brick_num; ++i)
	{
		FLIVR::TextureBrick* b1 = (*bricks1)[i];
		FLIVR::TextureBrick* b2 = (*bricks2)[i];
		long nx, ny, nz, bits1, bits2;
		if (!GetInfo(b1, b2, bits1, bits2, nx, ny, nz))
			continue;

		//get data
		void* val1 = 0;
		void* val2 = 0;
		if (brick_num > 1)
		{
			val1 = GetVolDataBrick(b1);
			val2 = GetVolDataBrick(b2);
		}
		else
		{
			val1 = GetVolData(m_vd1);
			val2 = GetVolData(m_vd2);
		}

		size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };
		size_t local_size[3] = { 1, 1, 1 };

		//set
		unsigned int count = 0;
		cl_image_format image_format;
		cl_image_desc image_desc;
		//channel1
		image_format.image_channel_order = CL_R;
		if (bits1 == 8)
			image_format.image_channel_data_type = CL_UNORM_INT8;
		else if (bits1 == 16)
			image_format.image_channel_data_type = CL_UNORM_INT16;
		image_desc.image_type = CL_MEM_OBJECT_IMAGE3D;
		image_desc.image_width = nx;
		image_desc.image_height = ny;
		image_desc.image_depth = nz;
		image_desc.image_array_size = 0;
		image_desc.image_row_pitch = 0;
		image_desc.image_slice_pitch = 0;
		image_desc.num_mip_levels = 0;
		image_desc.num_samples = 0;
		image_desc.buffer = 0;
		kernel_prog->setKernelArgImage(kernel_index, 0,
			CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			image_format, image_desc, val1);
		//channel2
		if (bits2 == 8)
			image_format.image_channel_data_type = CL_UNORM_INT8;
		else if (bits2 == 16)
			image_format.image_channel_data_type = CL_UNORM_INT16;
		kernel_prog->setKernelArgImage(kernel_index, 1,
			CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			image_format, image_desc, val2);
		kernel_prog->setKernelArgConst(kernel_index, 2,
			sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(kernel_index, 3,
			sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(kernel_index, 4,
			sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setKernelArgConst(kernel_index, 5,
			sizeof(float), (void*)(&threshold));
		kernel_prog->setKernelArgBuf(kernel_index, 6,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(unsigned int), (void*)(&count));

		//execute
		kernel_prog->executeKernel(kernel_index, 3, global_size, local_size);
		//read back
		kernel_prog->readBuffer(sizeof(unsigned int), &count, &count);

		//release buffer
		kernel_prog->releaseMemObject(0, val1);
		kernel_prog->releaseMemObject(0, val2);
		kernel_prog->releaseMemObject(sizeof(unsigned int), &count);
		m_result += count;

		if (brick_num > 1)
		{
			ReleaseData(val1, bits1);
			ReleaseData(val2, bits2);
		}
	}
}

