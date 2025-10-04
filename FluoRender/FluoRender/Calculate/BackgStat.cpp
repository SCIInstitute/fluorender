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
#include <BackgStat.h>
#include <Global.h>
#include <VolumeRenderer.h>
#include <KernelProgram.h>
#include <TextureBrick.h>
#include <Texture.h>
#include <KernelFactory.h>
#include <VolumeData.h>
#include <algorithm>

using namespace flrd;

//8-bit data
constexpr const char* str_cl_backg_stat = R"CLKER(
#define DWL unsigned char
#define VSCL 255
const sampler_t samp =
	CLK_NORMALIZED_COORDS_FALSE|
	CLK_ADDRESS_CLAMP_TO_EDGE|
	CLK_FILTER_NEAREST;

//extract background
__kernel void kernel_0(
	__read_only image3d_t data,
	__global DWL* bkg,
	unsigned int dnxy,
	unsigned int dnx,
	unsigned int kx,
	unsigned int ky,
	unsigned int kxy,
	float varth,
	float gauth)
{
	int4 coord = (int4)(get_global_id(0),
		get_global_id(1), get_global_id(2), 1);
	unsigned int index = dnxy*coord.z + dnx*coord.y + coord.x;
	int4 kc;
	float4 dvalue;
	dvalue = read_imagef(data, samp, coord);
	float cvalue = dvalue.x;
	float sumi = 0.0f;
	float sumi2 = 0.0f;
	int i, j, k;
	for (i = 0; i < kx; ++i)
	for (j = 0; j < ky; ++j)
	{
		kc = (int4)(coord.x + (i - kx / 2),
			coord.y + (j - ky / 2),
			coord.z, 1);
		dvalue = read_imagef(data, samp, kc);
		sumi += dvalue.x;
		sumi2 += dvalue.x * dvalue.x;
	}
	float mean = sumi / kxy;
	float var = sqrt((sumi2 + kxy * mean * mean - 2.0f * mean * sumi) / kxy);
	cvalue = (var < varth) || (cvalue - mean < var * gauth) ? cvalue : 0.0f;
	DWL bv = cvalue * VSCL;
	bkg[index] = bv;
}
//extract background in mask
__kernel void kernel_1(
	__read_only image3d_t data,
	__global DWL* bkg,
	unsigned int dnxy,
	unsigned int dnx,
	unsigned int kx,
	unsigned int ky,
	unsigned int kxy,
	float varth,
	float gauth,
	__read_only image3d_t mask)
{
	int4 coord = (int4)(get_global_id(0),
		get_global_id(1), get_global_id(2), 1);
	unsigned int index = dnxy*coord.z + dnx*coord.y + coord.x;
	float mask_value = read_imagef(mask, samp, coord).x;
	if (mask_value < 1e-6f)
	{
		bkg[index] = 0;
		return;
	}
	int4 kc;
	float4 dvalue;
	dvalue = read_imagef(data, samp, coord);
	float cvalue = dvalue.x;
	float sumi = 0.0f;
	float sumi2 = 0.0f;
	int i, j, k;
	for (i = 0; i < kx; ++i)
	for (j = 0; j < ky; ++j)
	{
		kc = (int4)(coord.x + (i - kx / 2),
			coord.y + (j - ky / 2),
			coord.z, 1);
		dvalue = read_imagef(data, samp, kc);
		sumi += dvalue.x;
		sumi2 += dvalue.x * dvalue.x;
	}
	float mean = sumi / kxy;
	float var = sqrt((sumi2 + kxy * mean * mean - 2.0f * mean * sumi) / kxy);
	cvalue = (var < varth) || (cvalue - mean < var * gauth) ? cvalue : 0.0f;
	DWL bv = cvalue * VSCL;
	bkg[index] = bv;
}
//count in background
__kernel void kernel_2(
	__global DWL* bkg,
	unsigned int ngx,
	unsigned int ngy,
	unsigned int ngz,
	unsigned int gsxy,
	unsigned int gsx,
	unsigned int dnxy, 
	unsigned int dnx,
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
	unsigned int index;
	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)
	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)
	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)
	{
		index = dnxy* ijk.z + dnx*ijk.y + ijk.x;
		val = bkg[index];
		if (val > 0.0f)
		{
			lsum++;
			lwsum += val;
		}
	}
	index = gsxy * gid.z + gsx * gid.y + gid.x;
	count[index] = lsum;
	wcount[index] = lwsum;
}
//minmax in background
__kernel void kernel_3(
	__global DWL* bkg,
	unsigned int ngx,
	unsigned int ngy,
	unsigned int ngz,
	unsigned int gsxy,
	unsigned int gsx,
	unsigned int dnxy, 
	unsigned int dnx,
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
	unsigned int index;
	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)
	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)
	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)
	{
		index = dnxy* ijk.z + dnx*ijk.y + ijk.x;
		val = bkg[index];
		lminv = val ? min(val, lminv) : lminv;
		lmaxv = max(val, lmaxv);
	}
	lminv = lminv == VSCL ? 0 : lminv;
	index = gsxy * gid.z + gsx * gid.y + gid.x;
	minv[index] = (uint)(lminv);
	maxv[index] = (uint)(lmaxv);
}
//histogram in background
__kernel void kernel_4(
	__global DWL* bkg,
	unsigned int dnxy, 
	unsigned int dnx,
	unsigned int minv,
	unsigned int maxv,
	unsigned int bin,
	__global unsigned int* hist)
{
	int4 coord = (int4)(get_global_id(0),
		get_global_id(1), get_global_id(2), 1);
	unsigned int index = dnxy* coord.z + dnx*coord.y + coord.x;
	unsigned int val = bkg[index];
	if (val < minv || val > maxv)
		return;
	index = (val - minv) * (bin - 1) / (maxv - minv);
	atomic_inc(hist+index);
};
)CLKER";

BackgStat::BackgStat(VolumeData* vd)
	: m_vd(vd),
	m_use_mask(false),
	m_type(0),
	m_kx(40),
	m_ky(40),
	m_kz(10),
	m_varth(0.0001f),
	m_gauth(2),
	m_sum(0),
	m_wsum(0),
	m_minv(0),
	m_maxv(0),
	m_medv(0),
	m_modv(0)
{
}

BackgStat::~BackgStat()
{
}

bool BackgStat::CheckBricks()
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

bool BackgStat::GetInfo(
	flvr::TextureBrick* b,
	long &bits, long &nx, long &ny, long &nz)
{
	bits = b->nb(0) * 8;
	nx = b->nx();
	ny = b->ny();
	nz = b->nz();
	return true;
}

void BackgStat::Run()
{
	if (!CheckBricks())
		return;
	long bits = m_vd->GetBits();
	int chars = bits / 8;
	float max_int = static_cast<float>(m_vd->GetMaxValue());

	//create program and kernels
	flvr::KernelProgram* kernel_prog = glbin_kernel_factory.program(str_cl_backg_stat, bits, max_int);
	if (!kernel_prog)
		return;
	int kernel_index0;
	if (m_use_mask)
		kernel_index0 = kernel_prog->createKernel("kernel_1");
	else
		kernel_index0 = kernel_prog->createKernel("kernel_0");
	int kernel_index1, kernel_index2;
	switch (m_type)
	{
	case 0:
		//mean
		kernel_index1 = kernel_prog->createKernel("kernel_2");
		break;
	case 1:
		//minmax
		kernel_index1 = kernel_prog->createKernel("kernel_3");
		break;
	case 2:
		//median
		kernel_index1 = kernel_prog->createKernel("kernel_3");
		kernel_index2 = kernel_prog->createKernel("kernel_4");
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

		//set
		unsigned int dnxy = nx * ny;
		unsigned int dnx = nx;
		unsigned int kxy = m_kx * m_ky;
		kernel_prog->setKernelArgBegin(kernel_index0);
		kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, tid);
		auto arg_bkg =
			kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY, "arg_bkg", chars * nx * ny * nz, NULL);
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&dnxy));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&dnx));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&m_kx));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&m_ky));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&kxy));
		kernel_prog->setKernelArgConst(sizeof(float), (void*)(&m_varth));
		kernel_prog->setKernelArgConst(sizeof(float), (void*)(&m_gauth));
		if (m_use_mask)
			kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, mid);

		//execute
		kernel_prog->executeKernel(kernel_index0, 3, global_size, local_size);

		//brick min and max
		unsigned int bminv = std::numeric_limits<unsigned int>::max();
		unsigned int bmaxv = 0;

		if (m_type == 0)
		{
			//mean
			unsigned int* sum = new unsigned int[gsize.gsxyz];
			float *wsum = new float[gsize.gsxyz];
			kernel_prog->setKernelArgBegin(kernel_index1);
			kernel_prog->setKernelArgument(arg_bkg);
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngx));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngy));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngz));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.gsxy));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.gsx));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&dnxy));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&dnx));
			auto arg_sum =
				kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, "arg_sum", sizeof(unsigned int) * (gsize.gsxyz), (void*)(sum));
			auto arg_wsum =
				kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, "arg_wsum", sizeof(float) * (gsize.gsxyz), (void*)(wsum));

			//execute
			kernel_prog->executeKernel(kernel_index1, 3, global_size1, local_size);
			//read back
			kernel_prog->readBuffer(arg_sum, sum);
			kernel_prog->readBuffer(arg_wsum, wsum);

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
			kernel_prog->setKernelArgBegin(kernel_index1);
			kernel_prog->setKernelArgument(arg_bkg);
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngx));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngy));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngz));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.gsxy));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.gsx));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&dnxy));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&dnx));
			auto arg_minv =
				kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, "arg_minv", sizeof(unsigned int) * (gsize.gsxyz), (void*)(minv));
			auto arg_maxv =
				kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, "arg_maxv", sizeof(unsigned int) * (gsize.gsxyz), (void*)(maxv));

			//execute
			kernel_prog->executeKernel(kernel_index1, 3, global_size1, local_size);
			//read back
			kernel_prog->readBuffer(arg_minv, minv);
			kernel_prog->readBuffer(arg_maxv, maxv);

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
			kernel_prog->setKernelArgBegin(kernel_index2);
			kernel_prog->setKernelArgument(arg_bkg);
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&dnxy));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&dnx));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&bminv));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&bmaxv));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&bin));
			auto arg_hist =
				kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, "arg_hist", sizeof(unsigned int) * (bin), (void*)(hist));

			//execute
			kernel_prog->executeKernel(kernel_index2, 3, global_size, local_size);
			//read back
			kernel_prog->readBuffer(arg_hist, hist);

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

