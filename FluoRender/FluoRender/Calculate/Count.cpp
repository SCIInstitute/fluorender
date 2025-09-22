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
#include <Count.h>
#include <KernelProgram.h>
#include <Global.h>
#include <VolumeRenderer.h>
#include <TextureBrick.h>
#include <Texture.h>
#include <VolKernel.h>
#include <VolumeData.h>
#include <algorithm>

using namespace flrd;

constexpr const char* str_cl_count_voxels = R"CLKER(
__kernel void kernel_0(
	__read_only image3d_t data,
	__read_only image3d_t mask,
	unsigned int ngx,
	unsigned int ngy,
	unsigned int ngz,
	unsigned int gsxy,
	unsigned int gsx,
	unsigned int width,
	unsigned int height,
	unsigned int depth,
	__global unsigned int* count,
	__global float* wcount)
{
	const sampler_t samp = CLK_NORMALIZED_COORDS_FALSE |
						   CLK_ADDRESS_CLAMP_TO_EDGE |
						   CLK_FILTER_NEAREST;

	int3 gid = (int3)(get_global_id(0), get_global_id(1), get_global_id(2));
	int3 lb = (int3)(gid.x * ngx, gid.y * ngy, gid.z * ngz);
	int3 ub = (int3)(min(lb.x + ngx, width),
					 min(lb.y + ngy, height),
					 min(lb.z + ngz, depth));

	int4 ijk = (int4)(0, 0, 0, 1);
	unsigned int lsum = 0;
	float lwsum = 0.0f;

	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)
	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)
	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)
	{
		float v2 = read_imagef(mask, samp, ijk).x;
		if (v2 > 0.0f)
		{
			float v1 = read_imagef(data, samp, ijk).x;
			lsum++;
			lwsum += v1;
		}
	}

	unsigned int index = gsxy * gid.z + gsx * gid.y + gid.x;
	count[index] = lsum;
	wcount[index] = lwsum;
}
)CLKER";

CountVoxels::CountVoxels() :
	m_sum(0),
	m_wsum(0.0)
{
}

CountVoxels::~CountVoxels()
{
}

bool CountVoxels::CheckBricks()
{
	auto vd = m_vd.lock();
	if (!vd)
		return false;
	if (!vd->GetTexture())
		return false;
	int brick_num = vd->GetTexture()->get_brick_num();
	if (!brick_num)
		return false;
	return true;
}

bool CountVoxels::GetInfo(
	flvr::TextureBrick* b,
	unsigned int &bits, unsigned int &nx, unsigned int &ny, unsigned int &nz)
{
	bits = static_cast<unsigned int>(b->nb(0)*8);
	nx = static_cast<unsigned int>(b->nx());
	ny = static_cast<unsigned int>(b->ny());
	nz = static_cast<unsigned int>(b->nz());
	return true;
}

void CountVoxels::Count()
{
	m_sum = 0;
	m_wsum = 0;

	if (!CheckBricks())
		return;

	auto vd = m_vd.lock();
	if (!vd)
		return;
	if (!vd->GetMask(false))
		return;
	long bits = vd->GetBits();
	float max_int = static_cast<float>(vd->GetMaxValue());

	//create program and kernels
	flvr::KernelProgram* kernel_prog = glbin_vol_kernel_factory.kernel(str_cl_count_voxels, bits, max_int);
	if (!kernel_prog)
		return;
	int kernel_index = -1;
	std::string name = "kernel_0";
	if (kernel_prog->valid())
		kernel_index = kernel_prog->findKernel(name);
	else
		kernel_index = kernel_prog->createKernel(name);

	size_t brick_num = vd->GetTexture()->get_brick_num();
	std::vector<flvr::TextureBrick*> *bricks = vd->GetTexture()->get_bricks();

	m_sum = 0; m_wsum = 0.0;
	for (size_t i = 0; i < brick_num; ++i)
	{
		flvr::TextureBrick* b = (*bricks)[i];
		unsigned int nx, ny, nz, bits;
		if (!GetInfo(b, bits, nx, ny, nz))
			continue;
		//get tex ids
		GLint tid = vd->GetVR()->load_brick(b);
		GLint mid = vd->GetVR()->load_brick_mask(b);

		//compute workload
		flvr::GroupSize gsize;
		kernel_prog->get_group_size(kernel_index, nx, ny, nz, gsize);

		size_t local_size[3] = { 1, 1, 1 };
		size_t global_size[3] = {
			size_t(gsize.gsx), size_t(gsize.gsy), size_t(gsize.gsz) };

		//set
		unsigned int* sum = new unsigned int[gsize.gsxyz];
		float *wsum = new float[gsize.gsxyz];
		kernel_prog->setKernelArgBegin(kernel_index);
		kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, tid);
		kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, mid);
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngx));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngy));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngz));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.gsxy));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.gsx));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(unsigned int)*(gsize.gsxyz), (void*)(sum));
		kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(float)*(gsize.gsxyz), (void*)(wsum));

		//execute
		kernel_prog->executeKernel(kernel_index, 3, global_size, 0/*local_size*/);
		//read back
		kernel_prog->readBuffer(sizeof(unsigned int)*(gsize.gsxyz), sum, sum);
		kernel_prog->readBuffer(sizeof(float)*(gsize.gsxyz), wsum, wsum);

		//release buffer
		kernel_prog->releaseMemObject(kernel_index, 0, 0, tid, 0);
		kernel_prog->releaseMemObject(kernel_index, 1, 0, mid, 0);
		kernel_prog->releaseMemObject(sizeof(unsigned int)*(gsize.gsxyz), sum);
		kernel_prog->releaseMemObject(sizeof(float)*(gsize.gsxyz), wsum);

		//sum
		for (size_t i = 0; i < gsize.gsxyz; ++i)
		{
			m_sum += sum[i];
			m_wsum += wsum[i];
		}
		delete[] sum;
		delete[] wsum;
	}

	//update volume
	vd->SetMaskCount(m_sum, m_wsum);
}

