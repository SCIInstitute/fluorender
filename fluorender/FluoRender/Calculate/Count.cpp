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
#include "Count.h"
#include "cl_code.h"
#include <FLIVR/VolumeRenderer.h>
#include <FLIVR/KernelProgram.h>
#include <FLIVR/VolKernel.h>
#include <FLIVR/TextureBrick.h>
#include <FLIVR/Texture.h>
#include <algorithm>

using namespace FL;

CountVoxels::CountVoxels(VolumeData* vd)
	: m_vd(vd),
	m_use_mask(false)
{
}

CountVoxels::~CountVoxels()
{
}

bool CountVoxels::CheckBricks()
{
	if (!m_vd)
		return false;
	if (!m_vd->GetTexture())
		return false;
	int brick_num = m_vd->GetTexture()->get_brick_num();
	if (!brick_num)
		return false;
	return true;
}

bool CountVoxels::GetInfo(
	FLIVR::TextureBrick* b,
	long &bits, long &nx, long &ny, long &nz)
{
	bits = b->nb(m_use_mask ? b->nmask() : 0)*8;
	long nx = b->nx();
	long ny = b->ny();
	long nz = b->nz();
	return true;
}

void* CountVoxels::GetVolDataBrick(FLIVR::TextureBrick* b)
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

void* CountVoxels::GetVolData(VolumeData* vd)
{
	int nx, ny, nz;
	vd->GetResolution(nx, ny, nz);
	Nrrd* nrrd_data = 0;
	if (m_use_mask)
		nrrd_data = vd->GetMask(false);
	if (!nrrd_data)
		nrrd_data = vd->GetVolume(false);
	if (!nrrd_data)
		return 0;
	return nrrd_data->data;
}

void CountVoxels::ReleaseData(void* val, long bits)
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

long CountVoxels::OptimizeGroupSize(long nt, long target)
{
	long loj, hij, res, maxj;
	//z
	if (nt > target)
	{
		loj = std::max(long(1), (target+1) / 2);
		hij = std::min(nt, target * 2);
		res = 0; maxj = 0;
		for (long j = loj; j < hij; ++j)
		{
			long rm = nt % j;
			if (rm)
			{
				if (rm > res)
				{
					res = rm;
					maxj = j;
				}
			}
			else
			{
				return j;
			}
		}
		if (maxj)
			return maxj;
	}

	return target;
}

void CountVoxels::Count()
{
	if (!CheckBricks())
		return;

	//create program and kernels
	FLIVR::KernelProgram* kernel_prog = FLIVR::VolumeRenderer::
		vol_kernel_factory_.kernel(str_cl_chann_dotprod);
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
		//get tex ids
		GLint tid1 = m_vd1->GetVR()->load_brick(0, 0, bricks1, i);
		GLint tid2 = m_vd2->GetVR()->load_brick(0, 0, bricks2, i);

		//compute workload
		size_t ng;
		kernel_prog->getWorkGroupSize(kernel_index, &ng);
		//try to make gsxyz equal to ng
		//ngx*ngy*ngz = nx*ny*nz/ng
		//z
		long targetz = std::ceil(double(nz) / std::pow(double(ng), 1/3.0));
		//optimize
		long ngz = OptimizeGroupSize(nz, targetz);
		//xy
		long targetx;
		long targety;
		if (ngz == 1)
		{
			targetx = std::ceil(double(nx) / std::sqrt(double(ng)));
			targety = std::ceil(double(ny) / std::sqrt(double(ng)));
		}
		else
		{
			targetx = std::ceil(double(nx) * targetz / nz);
			targety = std::ceil(double(ny) * targetz / nz);
		}
		//optimize
		long ngx = OptimizeGroupSize(nx, targetx);
		long ngy = OptimizeGroupSize(ny, targety);

		long gsx = nx / ngx + (nx%ngx ? 1 : 0);
		long gsy = ny / ngy + (ny%ngy ? 1 : 0);
		long gsz = nz / ngz + (nz%ngz ? 1 : 0);
		long gsxyz = gsx * gsy * gsz;
		long gsxy = gsx * gsy;

		size_t local_size[3] = { 1, 1, 1 };
		size_t global_size[3] = { size_t(gsx), size_t(gsy), size_t(gsz) };

		//set
		//unsigned int count = 0;
		float *sum = new float[gsxyz];
		kernel_prog->setKernelArgTex3D(kernel_index, 0,
			CL_MEM_READ_ONLY, tid1);
		kernel_prog->setKernelArgTex3D(kernel_index, 1,
			CL_MEM_READ_ONLY, tid2);
		kernel_prog->setKernelArgConst(kernel_index, 2,
			sizeof(unsigned int), (void*)(&ngx));
		kernel_prog->setKernelArgConst(kernel_index, 3,
			sizeof(unsigned int), (void*)(&ngy));
		kernel_prog->setKernelArgConst(kernel_index, 4,
			sizeof(unsigned int), (void*)(&ngz));
		kernel_prog->setKernelArgConst(kernel_index, 5,
			sizeof(unsigned int), (void*)(&gsxy));
		kernel_prog->setKernelArgConst(kernel_index, 6,
			sizeof(unsigned int), (void*)(&gsx));
		kernel_prog->setKernelArgBuf(kernel_index, 7,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(float)*(gsxyz), (void*)(sum));

		//execute
		kernel_prog->executeKernel(kernel_index, 3, global_size, 0/*local_size*/);
		//read back
		kernel_prog->readBuffer(sizeof(float)*(gsxyz), sum, sum);

		//release buffer
		//kernel_prog->releaseMemObject(0, val1);
		//kernel_prog->releaseMemObject(0, val2);
		kernel_prog->releaseMemObject(kernel_index, 0, 0, tid1);
		kernel_prog->releaseMemObject(kernel_index, 1, 0, tid2);
		kernel_prog->releaseMemObject(sizeof(float)*(gsxyz), sum);

		//sum
		for (int i=0; i< gsxyz; ++i)
			m_result += sum[i];
		delete[] sum;
	}
}

