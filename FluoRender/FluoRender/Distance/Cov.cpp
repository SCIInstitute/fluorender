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
#include <Cov.h>
#include <Global.h>
#include <VolumeRenderer.h>
#include <KernelProgram.h>
#include <TextureBrick.h>
#include <Texture.h>
#include <algorithm>

using namespace flrd;

const char* str_cl_cov = \
"const sampler_t samp =\n" \
"	CLK_NORMALIZED_COORDS_FALSE|\n" \
"	CLK_ADDRESS_CLAMP_TO_EDGE|\n" \
"	CLK_FILTER_NEAREST;\n" \
"\n" \
"__kernel void kernel_0(\n" \
"	__read_only image3d_t mask,\n" \
"	unsigned int ngx,\n" \
"	unsigned int ngy,\n" \
"	unsigned int ngz,\n" \
"	unsigned int gsxy,\n" \
"	unsigned int gsx,\n" \
"	__global unsigned int* count,\n" \
"	__global float* csum)\n" \
"{\n" \
"	int3 gid = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);\n" \
"	int3 ub = (int3)(lb.x + ngx, lb.y + ngy, lb.z + ngz);\n" \
"	int4 ijk = (int4)(0, 0, 0, 1);\n" \
"	unsigned int lsum = 0;\n" \
"	float3 lcsum = (float3)(0.0f, 0.0f, 0.0f);\n" \
"	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)\n" \
"	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)\n" \
"	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)\n" \
"	{\n" \
"		float mval = read_imagef(mask, samp, ijk).x;\n" \
"		if (mval > 0.0f)\n" \
"		{\n" \
"			lsum++;\n" \
"			lcsum.x += (float)(ijk.x);\n" \
"			lcsum.y += (float)(ijk.y);\n" \
"			lcsum.z += (float)(ijk.z);\n" \
"		}\n" \
"	}\n" \
"	unsigned int index = gsxy * gid.z + gsx * gid.y + gid.x;\n" \
"	atomic_xchg(count+index, lsum);\n" \
"	atomic_xchg(csum+index*3, lcsum.x);\n" \
"	atomic_xchg(csum+index*3+1, lcsum.y);\n" \
"	atomic_xchg(csum+index*3+2, lcsum.z);\n" \
"}\n" \
"__kernel void kernel_1(\n" \
"	__read_only image3d_t mask,\n" \
"	unsigned int ngx,\n" \
"	unsigned int ngy,\n" \
"	unsigned int ngz,\n" \
"	unsigned int gsxy,\n" \
"	unsigned int gsx,\n" \
"	float3 center,\n" \
"	float3 orig,\n" \
"	__global float* cov)\n" \
"{\n" \
"	int3 gid = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);\n" \
"	int3 ub = (int3)(lb.x + ngx, lb.y + ngy, lb.z + ngz);\n" \
"	int4 ijk = (int4)(0, 0, 0, 1);\n" \
"	float xx = 0.0f, xy = 0.0f, xz = 0.0f, yy = 0.0f, yz = 0.0f, zz = 0.0f;\n" \
"	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)\n" \
"	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)\n" \
"	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)\n" \
"	{\n" \
"		float mval = read_imagef(mask, samp, ijk).x;\n" \
"		if (mval < 0.0001f)\n" \
"			continue;\n" \
"		float3 fijk = (float3)(ijk.x, ijk.y, ijk.z) + orig - center;\n" \
"		xx += fijk.x * fijk.x;\n" \
"		xy += fijk.x * fijk.y;\n" \
"		xz += fijk.x * fijk.z;\n" \
"		yy += fijk.y * fijk.y;\n" \
"		yz += fijk.y * fijk.z;\n" \
"		zz += fijk.z * fijk.z;\n" \
"	}\n" \
"	unsigned int index = gsxy * gid.z + gsx * gid.y + gid.x;\n" \
"	atomic_xchg(cov+index*6, xx);\n" \
"	atomic_xchg(cov+index*6+1, xy);\n" \
"	atomic_xchg(cov+index*6+2, xz);\n" \
"	atomic_xchg(cov+index*6+3, yy);\n" \
"	atomic_xchg(cov+index*6+4, yz);\n" \
"	atomic_xchg(cov+index*6+5, zz);\n" \
"}\n" \
;

Cov::Cov(VolumeData* vd)
	: m_vd(vd),
	m_use_mask(false)
{
	std::memset(m_cov, 0, sizeof(float) * 9);
	std::memset(m_center, 0, sizeof(float) * 3);
}

Cov::~Cov()
{
}

bool Cov::CheckBricks()
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

bool Cov::GetInfo(
	flvr::TextureBrick* b,
	long &bits, long &nx, long &ny, long &nz)
{
	bits = b->nb(0) * 8;
	nx = b->nx();
	ny = b->ny();
	nz = b->nz();
	return true;
}

bool Cov::ComputeCenter()
{
	if (!CheckBricks())
		return false;
	if (!m_vd->GetMask(false))
		return false;

	//create program and kernels
	flvr::KernelProgram* kernel_prog = glbin_vol_kernel_factory.kernel(str_cl_cov);
	if (!kernel_prog)
		return false;
	string name = "kernel_0";
	int kernel_index = kernel_prog->createKernel(name);

	size_t brick_num = m_vd->GetTexture()->get_brick_num();
	std::vector<flvr::TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks();

	//get center
	std::memset(m_center, 0, sizeof(float) * 3);
	unsigned long long sum = 0;
	for (size_t i = 0; i < brick_num; ++i)
	{
		flvr::TextureBrick* b = (*bricks)[i];
		long nx, ny, nz, bits;
		if (!GetInfo(b, bits, nx, ny, nz))
			continue;
		//get tex ids
		GLint mid = m_vd->GetVR()->load_brick_mask(b);

		//compute workload
		flvr::GroupSize gsize;
		kernel_prog->get_group_size(kernel_index, nx, ny, nz, gsize);

		size_t local_size[3] = { 1, 1, 1 };
		size_t global_size[3] = {
			size_t(gsize.gsx), size_t(gsize.gsy), size_t(gsize.gsz) };

		//set
		unsigned int* count = new unsigned int[gsize.gsxyz];
		float *csum = new float[gsize.gsxyz * 3];
		kernel_prog->setKernelArgBegin(kernel_index);
		kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, mid);
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngx));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngy));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngz));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.gsxy));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.gsx));
		kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(unsigned int)*(gsize.gsxyz), (void*)(count));
		kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(float)*(gsize.gsxyz * 3), (void*)(csum));

		//execute
		kernel_prog->executeKernel(kernel_index, 3, global_size, 0);
		//read back
		kernel_prog->readBuffer(sizeof(unsigned int)*(gsize.gsxyz), count, count);
		kernel_prog->readBuffer(sizeof(float)*(gsize.gsxyz * 3), csum, csum);

		int ox, oy, oz, nc;
		ox = b->ox(); oy = b->oy(); oz = b->oz();
		//compute center
		for (int i = 0; i < gsize.gsxyz; ++i)
		{
			nc = count[i];
			sum += nc;
			m_center[0] += csum[i * 3] + ox * nc;
			m_center[1] += csum[i * 3 + 1] + oy * nc;
			m_center[2] += csum[i * 3 + 2] + oz * nc;
		}

		//release buffer
		kernel_prog->releaseAll();
	}

	if (!sum)
		return false;
	m_center[0] /= sum;
	m_center[1] /= sum;
	m_center[2] /= sum;

	return true;
}

bool Cov::ComputeCov()
{
	if (!CheckBricks())
		return false;
	if (!m_vd->GetMask(false))
		return false;

	//create program and kernels
	flvr::KernelProgram* kernel_prog = glbin_vol_kernel_factory.kernel(str_cl_cov);
	if (!kernel_prog)
		return false;
	string name = "kernel_1";
	int kernel_index = kernel_prog->createKernel(name);

	size_t brick_num = m_vd->GetTexture()->get_brick_num();
	std::vector<flvr::TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks();

	//get cov
	for (size_t i = 0; i < brick_num; ++i)
	{
		flvr::TextureBrick* b = (*bricks)[i];
		long nx, ny, nz, bits;
		if (!GetInfo(b, bits, nx, ny, nz))
			continue;
		//get tex ids
		GLint mid = m_vd->GetVR()->load_brick_mask(b);

		//compute workload
		flvr::GroupSize gsize;
		kernel_prog->get_group_size(kernel_index, nx, ny, nz, gsize);

		size_t local_size[3] = { 1, 1, 1 };
		size_t global_size[3] = {
			size_t(gsize.gsx), size_t(gsize.gsy), size_t(gsize.gsz) };

		//set
		unsigned int* count = new unsigned int[gsize.gsxyz];
		float *cov = new float[gsize.gsxyz * 6];
		float orig[3] = { float(b->ox()), float(b->oy()), float(b->oz()) };
		kernel_prog->setKernelArgBegin(kernel_index);
		kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, mid);
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngx));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngy));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngz));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.gsxy));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.gsx));
		kernel_prog->setKernelArgConst(sizeof(cl_float3), (void*)(m_center));
		kernel_prog->setKernelArgConst(sizeof(cl_float3), (void*)(orig));
		kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(float)*(gsize.gsxyz * 6), (void*)(cov));

		//execute
		kernel_prog->executeKernel(kernel_index, 3, global_size, 0);
		//read back
		kernel_prog->readBuffer(sizeof(float)*(gsize.gsxyz * 6), cov, cov);

		//compute center
		for (int i = 0; i < gsize.gsxyz; ++i)
		{
			m_cov[0] += cov[i * 6];
			m_cov[1] += cov[i * 6 + 1];
			m_cov[2] += cov[i * 6 + 2];
			m_cov[3] += cov[i * 6 + 3];
			m_cov[4] += cov[i * 6 + 4];
			m_cov[5] += cov[i * 6 + 5];
		}

		//release buffer
		kernel_prog->releaseAll();
	}

	return true;
}

bool Cov::Compute(int type)
{
	bool result = true;
	result = result && ComputeCenter();
	if (type == 0)
		result = result && ComputeCov();
	return result;
}

