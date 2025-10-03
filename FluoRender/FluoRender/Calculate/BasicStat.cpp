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
#include <BasicStat.h>
#include <KernelProgram.h>
#include <Global.h>
#include <VolumeRenderer.h>
#include <VolumeData.h>
#include <TextureBrick.h>
#include <Texture.h>
#include <Kernel.h>
#include <algorithm>

using namespace flrd;

//8-bit data
constexpr const char* str_cl_basic_stat = R"CLKER(
#define DWL unsigned char
#define VSCL 255
const sampler_t samp =
	CLK_NORMALIZED_COORDS_FALSE|
	CLK_ADDRESS_CLAMP_TO_EDGE|
	CLK_FILTER_NEAREST;

//count
__kernel void kernel_0(
	__read_only image3d_t data,
	unsigned int ngx,
	unsigned int ngy,
	unsigned int ngz,
	unsigned int gsxy,
	unsigned int gsx,
	__global unsigned int* count,
	__global float* wcount)
{
	int3 gid = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);
	int3 ub = (int3)(lb.x + ngx, lb.y + ngy, lb.z + ngz);
	int4 ijk = (int4)(0, 0, 0, 1);
	unsigned int lsum = 0;
	float lwsum = 0.0f;
	float val;
	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)
	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)
	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)
	{
		val = read_imagef(data, samp, ijk).x;
		if (val > 0.0f)
		{
			lsum++;
			lwsum += val;
		}
	}
	unsigned int index = gsxy * gid.z + gsx * gid.y + gid.x;
	count[index] = lsum;
	wcount[index] = lwsum;
}
//minmax
__kernel void kernel_1(
	__read_only image3d_t data,
	unsigned int ngx,
	unsigned int ngy,
	unsigned int ngz,
	unsigned int gsxy,
	unsigned int gsx,
	__global uint* minv,
	__global uint* maxv)
{
	int3 gid = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);
	int3 ub = (int3)(lb.x + ngx, lb.y + ngy, lb.z + ngz);
	int4 ijk = (int4)(0, 0, 0, 1);
	DWL lminv = VSCL;
	DWL lmaxv = 0;
	DWL val;
	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)
	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)
	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)
	{
		val = read_imagef(data, samp, ijk).x * VSCL;
		lminv = val ? min(val, lminv) : lminv;
		lmaxv = max(val, lmaxv);
	}
	lminv = lminv == VSCL ? 0 : lminv;
	unsigned int index = gsxy * gid.z + gsx * gid.y + gid.x;
	minv[index] = lminv;
	maxv[index] = lmaxv;
}
//histogram
__kernel void kernel_2(
	__read_only image3d_t data,
	unsigned int minv,
	unsigned int maxv,
	unsigned int bin,
	__global unsigned int* hist)
{
	int4 coord = (int4)(get_global_id(0),
		get_global_id(1), get_global_id(2), 1);
	unsigned int val = read_imagef(data, samp, coord).x * VSCL;
	if (val < minv || val > maxv)
		return;
	unsigned int index = (val - minv) * (bin - 1) / (maxv - minv);
	atomic_inc(hist+index);
}//count in mask
__kernel void kernel_3(
	__read_only image3d_t data,
	unsigned int ngx,
	unsigned int ngy,
	unsigned int ngz,
	unsigned int gsxy,
	unsigned int gsx,
	__global unsigned int* count,
	__global float* wcount,
	__read_only image3d_t mask)
{
	int3 gid = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);
	int3 ub = (int3)(lb.x + ngx, lb.y + ngy, lb.z + ngz);
	int4 ijk = (int4)(0, 0, 0, 1);
	unsigned int lsum = 0;
	float lwsum = 0.0f;
	float val, val2;
	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)
	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)
	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)
	{
		val = read_imagef(data, samp, ijk).x;
		val2 = read_imagef(mask, samp, ijk).x;
		if (val > 0.0f && val2 > 0.0f)
		{
			lsum++;
			lwsum += val;
		}
	}
	unsigned int index = gsxy * gid.z + gsx * gid.y + gid.x;
	count[index] = lsum;
	wcount[index] = lwsum;
}
//minmax in mask
__kernel void kernel_4(
	__read_only image3d_t data,
	unsigned int ngx,
	unsigned int ngy,
	unsigned int ngz,
	unsigned int gsxy,
	unsigned int gsx,
	__global uint* minv,
	__global uint* maxv,
	__read_only image3d_t mask)
{
	int3 gid = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);
	int3 ub = (int3)(lb.x + ngx, lb.y + ngy, lb.z + ngz);
	int4 ijk = (int4)(0, 0, 0, 1);
	DWL lminv = VSCL;
	DWL lmaxv = 0;
	DWL val;
	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)
	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)
	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)
	{
		val = read_imagef(data, samp, ijk).x * VSCL;
		float val2 = read_imagef(mask, samp, ijk).x;
		if (val2 > 0.0f)
		{
			lminv = val ? min(val, lminv) : lminv;
			lmaxv = max(val, lmaxv);
		}
	}
	lminv = lminv == VSCL ? 0 : lminv;
	unsigned int index = gsxy * gid.z + gsx * gid.y + gid.x;
	minv[index] = lminv;
	maxv[index] = lmaxv;
}
//histogram in mask
__kernel void kernel_5(
	__read_only image3d_t data,
	unsigned int minv,
	unsigned int maxv,
	unsigned int bin,
	__global unsigned int* hist,
	__read_only image3d_t mask)
{
	int4 coord = (int4)(get_global_id(0),
		get_global_id(1), get_global_id(2), 1);
	float val2 = read_imagef(mask, samp, coord).x;
	if (val2 < 1e-6)
		return;
	unsigned int val = read_imagef(data, samp, coord).x * VSCL;
	if (val < minv || val > maxv)
		return;
	unsigned int index = (val - minv) * (bin - 1) / (maxv - minv);
	atomic_inc(hist+index);
};
)CLKER";

BasicStat::BasicStat(VolumeData* vd)
	: m_vd(vd),
	m_use_mask(false),
	m_type(0),
	m_sum(0),
	m_wsum(0),
	m_minv(0),
	m_maxv(0),
	m_medv(0),
	m_modv(0)
{
}

BasicStat::~BasicStat()
{
}

bool BasicStat::CheckBricks()
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

bool BasicStat::GetInfo(
	flvr::TextureBrick* b,
	long &bits, long &nx, long &ny, long &nz)
{
	bits = b->nb(0) * 8;
	nx = b->nx();
	ny = b->ny();
	nz = b->nz();
	return true;
}

void BasicStat::Run()
{
	if (!CheckBricks())
		return;
	long bits = m_vd->GetBits();
	int chars = bits / 8;
	float max_int = static_cast<float>(m_vd->GetMaxValue());

	//create program and kernels
	flvr::KernelProgram* kernel_prog = glbin_kernel_factory.kernel(str_cl_basic_stat, bits, max_int);
	if (!kernel_prog)
		return;
	int kernel_index0, kernel_index1;
	switch (m_type)
	{
	case 0:
		//mean
		if (!m_use_mask)
			kernel_index0 = kernel_prog->createKernel("kernel_0");
		else
			kernel_index0 = kernel_prog->createKernel("kernel_3");
		break;
	case 1:
		//minmax
		if (!m_use_mask)
			kernel_index0 = kernel_prog->createKernel("kernel_1");
		else
			kernel_index0 = kernel_prog->createKernel("kernel_4");
		break;
	case 2:
		//median
		if (!m_use_mask)
		{
			kernel_index0 = kernel_prog->createKernel("kernel_1");
			kernel_index1 = kernel_prog->createKernel("kernel_2");
		}
		else
		{
			kernel_index0 = kernel_prog->createKernel("kernel_4");
			kernel_index1 = kernel_prog->createKernel("kernel_5");
		}
		break;
	}

	size_t brick_num = m_vd->GetTexture()->get_brick_num();
	std::vector<flvr::TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks();

	m_sum = 0;
	m_wsum = 0.0;
	m_minv = std::numeric_limits<unsigned int>::max();
	m_maxv = 0;
	m_medv = m_modv = 0;
	m_hist.clear();
	m_hist_acc.clear();

	for (size_t i = 0; i < brick_num; ++i)
	{
		flvr::TextureBrick* b = (*bricks)[i];
		long nx, ny, nz;
		if (!GetInfo(b, bits, nx, ny, nz))
			continue;
		//get tex ids
		GLint tid = m_vd->GetVR()->load_brick(b);
		GLint mid = 0;
		if (m_use_mask)
			mid = m_vd->GetVR()->load_brick_mask(b);

		//compute workload
		flvr::GroupSize gsize;
		kernel_prog->get_group_size(kernel_index1, nx, ny, nz, gsize);

		size_t local_size[3] = { 1, 1, 1 };
		size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };
		size_t global_size1[3] = {
			size_t(gsize.gsx), size_t(gsize.gsy), size_t(gsize.gsz) };

		//brick min and max
		unsigned int bminv = std::numeric_limits<unsigned int>::max();
		unsigned int bmaxv = 0;

		if (m_type == 0)
		{
			//mean
			unsigned int* sum = new unsigned int[gsize.gsxyz];
			float *wsum = new float[gsize.gsxyz];
			kernel_prog->setKernelArgBegin(kernel_index0);
			kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, tid);
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngx));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngy));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngz));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.gsxy));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.gsx));
			kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(unsigned int)*(gsize.gsxyz), (void*)(sum));
			kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(float)*(gsize.gsxyz), (void*)(wsum));
			if (m_use_mask)
				kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, mid);

			//execute
			kernel_prog->executeKernel(kernel_index0, 3, global_size1, local_size);
			//read back
			kernel_prog->readBuffer(sizeof(unsigned int)*(gsize.gsxyz), sum, sum);
			kernel_prog->readBuffer(sizeof(float)*(gsize.gsxyz), wsum, wsum);

			//sum
			for (size_t ii = 0; ii < gsize.gsxyz; ++ii)
			{
				m_sum += sum[ii];
				m_wsum += wsum[ii];
			}
			delete[] sum;
			delete[] wsum;
		}
		else if (m_type == 1 || m_type == 2)
		{
			//minmax
			unsigned int* minv = new unsigned int[gsize.gsxyz];
			unsigned int* maxv = new unsigned int[gsize.gsxyz];
			kernel_prog->setKernelArgBegin(kernel_index0);
			kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, tid);
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngx));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngy));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngz));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.gsxy));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.gsx));
			kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(unsigned int)*(gsize.gsxyz), (void*)(minv));
			kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(unsigned int)*(gsize.gsxyz), (void*)(maxv));
			if (m_use_mask)
				kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, mid);

			//execute
			kernel_prog->executeKernel(kernel_index0, 3, global_size1, local_size);
			//read back
			kernel_prog->readBuffer(sizeof(unsigned int)*(gsize.gsxyz), minv, minv);
			kernel_prog->readBuffer(sizeof(unsigned int)*(gsize.gsxyz), maxv, maxv);

			//collect
			for (size_t ii = 0; ii < gsize.gsxyz; ++ii)
			{
				bminv = minv[ii] ? std::min(bminv, minv[ii]) : bminv;
				bmaxv = std::max(bmaxv, maxv[ii]);
			}
			if (bminv == std::numeric_limits<unsigned int>::max())
				bminv = bmaxv;
			m_minv = bminv ? std::min(m_minv, bminv) : m_minv;
			m_maxv = std::max(m_maxv, bmaxv);
			if (m_minv == std::numeric_limits<unsigned int>::max())
				m_minv = m_maxv;

			delete[] minv;
			delete[] maxv;
		}

		if (m_type == 2)
		{
			//meidan mode with known minmax
			unsigned int bin = bmaxv - bminv + 1;
			unsigned int* hist = new unsigned int[bin]();
			kernel_prog->setKernelArgBegin(kernel_index1);
			kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, tid);
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&bminv));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&bmaxv));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&bin));
			kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(unsigned int)*(bin), (void*)(hist));
			if (m_use_mask)
				kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, mid);

			//execute
			kernel_prog->executeKernel(kernel_index1, 3, global_size, local_size);
			//read back
			kernel_prog->readBuffer(sizeof(unsigned int)*(bin), hist, hist);

			//collect
			for (size_t ii = 0; ii < bin; ++ii)
			{
				unsigned int key = static_cast<unsigned int>(ii + bminv);
				auto it = m_hist.find(key);
				if (it == m_hist.end())
					m_hist.insert(std::pair<unsigned int, unsigned int>(key, hist[ii]));
				else
					it->second += hist[ii];
			}
			delete[] hist;
		}

		kernel_prog->releaseAllArgs();
	}

	//median and mode from histogram
	if (m_hist.empty())
		return;

	unsigned int hm = 0;
	unsigned int ii = 0;
	for (auto it = m_hist.begin(); it != m_hist.end(); ++it)
	{
		if (it->second > hm)
		{
			hm = it->second;
			m_modv = it->first;
		}
		if (it == m_hist.begin())
			m_hist_acc.push_back(it->second);
		else
			m_hist_acc.push_back(it->second + m_hist_acc[ii-1]);
		ii++;
	}
	unsigned int half = m_hist_acc.back() / 2;
	ii = 0;
	for (auto it = m_hist_acc.begin(); it != m_hist_acc.end(); ++it)
	{
		if (*it >= half)
		{
			m_medv = ii;
			break;
		}
		ii++;
	}
	m_medv += m_minv;
}

