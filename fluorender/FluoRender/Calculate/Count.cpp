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
#include <FLIVR/VolumeRenderer.h>
#include <FLIVR/KernelProgram.h>
#include <FLIVR/VolKernel.h>
#include <FLIVR/TextureBrick.h>
#include <FLIVR/Texture.h>
#include <algorithm>

using namespace FL;

const char* str_cl_count_voxels = \
"const sampler_t samp =\n" \
"	CLK_NORMALIZED_COORDS_FALSE|\n" \
"	CLK_ADDRESS_CLAMP_TO_EDGE|\n" \
"	CLK_FILTER_NEAREST;\n" \
"\n" \
"__kernel void kernel_0(\n" \
"	__read_only image3d_t data,\n" \
"	__read_only image3d_t mask,\n" \
"	unsigned int ngx,\n" \
"	unsigned int ngy,\n" \
"	unsigned int ngz,\n" \
"	unsigned int gsxy,\n" \
"	unsigned int gsx,\n" \
"	__global unsigned int* count,\n" \
"	__global float* wcount)\n" \
"{\n" \
"	int3 gid = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);\n" \
"	int3 ub = (int3)(lb.x + ngx, lb.y + ngy, lb.z + ngz);\n" \
"	int4 ijk = (int4)(0, 0, 0, 1);\n" \
"	unsigned int lsum = 0;\n" \
"	float lwsum = 0.0;\n" \
"	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)\n" \
"	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)\n" \
"	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)\n" \
"	{\n" \
"		float v1 = read_imagef(data, samp, ijk).x;\n" \
"		float v2 = read_imagef(mask, samp, ijk).x;\n" \
"		if (v2 > 0.0)\n" \
"		{\n" \
"			lsum++;\n" \
"			lwsum += v1;\n" \
"		}\n" \
"	}\n" \
"	unsigned int index = gsxy * gid.z + gsx * gid.y + gid.x;\n" \
"	atomic_xchg(count+index, lsum);\n" \
"	atomic_xchg(wcount+index, lwsum);\n" \
"}\n";

CountVoxels::CountVoxels(VolumeData* vd)
	: m_vd(vd),
	m_use_mask(false),
	m_sum(0),
	m_wsum(0.0)
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
	bits = b->nb(0)*8;
	nx = b->nx();
	ny = b->ny();
	nz = b->nz();
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
		vol_kernel_factory_.kernel(str_cl_count_voxels);
	if (!kernel_prog)
		return;
	int kernel_index = -1;
	string name = "kernel_0";
	if (kernel_prog->valid())
		kernel_index = kernel_prog->findKernel(name);
	else
		kernel_index = kernel_prog->createKernel(name);

	size_t brick_num = m_vd->GetTexture()->get_brick_num();
	vector<FLIVR::TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks();

	m_sum = 0; m_wsum = 0.0;
	for (size_t i = 0; i < brick_num; ++i)
	{
		FLIVR::TextureBrick* b = (*bricks)[i];
		long nx, ny, nz, bits;
		if (!GetInfo(b, bits, nx, ny, nz))
			continue;
		//get tex ids
		GLint tid = m_vd->GetVR()->load_brick(0, 0, bricks, i);
		GLint mid = m_vd->GetVR()->load_brick_mask(bricks, i);

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
		unsigned int* sum = new unsigned int[gsxyz];
		float *wsum = new float[gsxyz];
		kernel_prog->setKernelArgTex3D(kernel_index, 0,
			CL_MEM_READ_ONLY, tid);
		kernel_prog->setKernelArgTex3D(kernel_index, 1,
			CL_MEM_READ_ONLY, mid);
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
			sizeof(unsigned int)*(gsxyz), (void*)(sum));
		kernel_prog->setKernelArgBuf(kernel_index, 8,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(float)*(gsxyz), (void*)(wsum));

		//execute
		kernel_prog->executeKernel(kernel_index, 3, global_size, 0/*local_size*/);
		//read back
		kernel_prog->readBuffer(sizeof(unsigned int)*(gsxyz), sum, sum);
		kernel_prog->readBuffer(sizeof(float)*(gsxyz), wsum, wsum);

		//release buffer
		kernel_prog->releaseMemObject(kernel_index, 0, 0, tid);
		kernel_prog->releaseMemObject(kernel_index, 1, 0, mid);
		kernel_prog->releaseMemObject(sizeof(unsigned int)*(gsxyz), sum);
		kernel_prog->releaseMemObject(sizeof(float)*(gsxyz), wsum);

		//sum
		for (int i = 0; i < gsxyz; ++i)
		{
			m_sum += sum[i];
			m_wsum += wsum[i];
		}
		delete[] sum;
		delete[] wsum;
	}
}

