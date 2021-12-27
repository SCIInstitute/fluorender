/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2021 Scientific Computing and Imaging Institute,
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
#include "BackgStat.h"
#include <VolumeData.hpp>
#include <FLIVR/VolumeRenderer.h>
#include <FLIVR/KernelProgram.h>
#include <FLIVR/VolKernel.h>
#include <FLIVR/TextureBrick.h>
#include <FLIVR/Texture.h>
#include <algorithm>
#ifdef _DEBUG
#include <fstream>
#endif

using namespace flrd;

//8-bit data
const char* str_cl_backg_stat = \
"#define DWL unsigned char\n" \
"#define VSCL 255\n" \
"const sampler_t samp =\n" \
"	CLK_NORMALIZED_COORDS_FALSE|\n" \
"	CLK_ADDRESS_CLAMP_TO_EDGE|\n" \
"	CLK_FILTER_NEAREST;\n" \
"\n" \
"//extract background\n" \
"__kernel void kernel_0(\n" \
"	__read_only image3d_t data,\n" \
"	__global DWL* bkg,\n" \
"	unsigned int dnxy,\n" \
"	unsigned int dnx,\n" \
"	unsigned int kx,\n" \
"	unsigned int ky,\n" \
"	unsigned int kxy,\n" \
"	float varth,\n" \
"	float gauth)\n" \
"{\n" \
"	int4 coord = (int4)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2), 1);\n" \
"	unsigned int index = dnxy*coord.z + dnx*coord.y + coord.x;\n" \
"	int4 kc;\n" \
"	float4 dvalue;\n" \
"	dvalue = read_imagef(data, samp, coord);\n" \
"	float cvalue = dvalue.x;\n" \
"	float sumi = 0.0;\n" \
"	float sumi2 = 0.0;\n" \
"	int i, j, k;\n" \
"	for (i = 0; i < kx; ++i)\n" \
"	for (j = 0; j < ky; ++j)\n" \
"	{\n" \
"		kc = (int4)(coord.x + (i - kx / 2),\n" \
"			coord.y + (j - ky / 2),\n" \
"			coord.z, 1);\n" \
"		dvalue = read_imagef(data, samp, kc);\n" \
"		sumi += dvalue.x;\n" \
"		sumi2 += dvalue.x * dvalue.x;\n" \
"	}\n" \
"	float mean = sumi / kxy;\n" \
"	float var = sqrt((sumi2 + kxy * mean * mean - 2.0 * mean * sumi) / kxy);\n" \
"	cvalue = (var < varth) || (cvalue - mean < var * gauth) ? cvalue : 0.0;\n" \
"	DWL bv = cvalue * VSCL;\n" \
"	bkg[index] = bv;\n" \
"}\n" \
"//extract background in mask\n" \
"__kernel void kernel_1(\n" \
"	__read_only image3d_t data,\n" \
"	__global DWL* bkg,\n" \
"	unsigned int dnxy,\n" \
"	unsigned int dnx,\n" \
"	unsigned int kx,\n" \
"	unsigned int ky,\n" \
"	unsigned int kxy,\n" \
"	float varth,\n" \
"	float gauth,\n" \
"	__read_only image3d_t mask)\n" \
"{\n" \
"	int4 coord = (int4)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2), 1);\n" \
"	unsigned int index = dnxy*coord.z + dnx*coord.y + coord.x;\n" \
"	float mask_value = read_imagef(mask, samp, coord).x;\n" \
"	if (mask_value < 1e-6)\n" \
"	{\n" \
"		bkg[index] = 0;\n" \
"		return;\n" \
"	}\n" \
"	int4 kc;\n" \
"	float4 dvalue;\n" \
"	dvalue = read_imagef(data, samp, coord);\n" \
"	float cvalue = dvalue.x;\n" \
"	float sumi = 0.0;\n" \
"	float sumi2 = 0.0;\n" \
"	int i, j, k;\n" \
"	for (i = 0; i < kx; ++i)\n" \
"	for (j = 0; j < ky; ++j)\n" \
"	{\n" \
"		kc = (int4)(coord.x + (i - kx / 2),\n" \
"			coord.y + (j - ky / 2),\n" \
"			coord.z, 1);\n" \
"		dvalue = read_imagef(data, samp, kc);\n" \
"		sumi += dvalue.x;\n" \
"		sumi2 += dvalue.x * dvalue.x;\n" \
"	}\n" \
"	float mean = sumi / kxy;\n" \
"	float var = sqrt((sumi2 + kxy * mean * mean - 2.0 * mean * sumi) / kxy);\n" \
"	cvalue = (var < varth) || (cvalue - mean < var * gauth) ? cvalue : 0.0;\n" \
"	DWL bv = cvalue * VSCL;\n" \
"	bkg[index] = bv;\n" \
"}\n" \
"//count in background\n" \
"__kernel void kernel_2(\n" \
"	__global DWL* bkg,\n" \
"	unsigned int ngx,\n" \
"	unsigned int ngy,\n" \
"	unsigned int ngz,\n" \
"	unsigned int gsxy,\n" \
"	unsigned int gsx,\n" \
"	unsigned int dnxy, \n" \
"	unsigned int dnx,\n" \
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
"	float val;\n" \
"	unsigned int index;\n" \
"	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)\n" \
"	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)\n" \
"	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)\n" \
"	{\n" \
"		index = dnxy* ijk.z + dnx*ijk.y + ijk.x;\n" \
"		val = bkg[index];\n" \
"		if (val > 0.0)\n" \
"		{\n" \
"			lsum++;\n" \
"			lwsum += val;\n" \
"		}\n" \
"	}\n" \
"	index = gsxy * gid.z + gsx * gid.y + gid.x;\n" \
"	//atomic_xchg(count+index, lsum);\n" \
"	//atomic_xchg(wcount+index, lwsum);\n" \
"	count[index] = lsum;\n" \
"	wcount[index] = lwsum;\n" \
"}\n" \
"//minmax in background\n" \
"__kernel void kernel_3(\n" \
"	__global DWL* bkg,\n" \
"	unsigned int ngx,\n" \
"	unsigned int ngy,\n" \
"	unsigned int ngz,\n" \
"	unsigned int gsxy,\n" \
"	unsigned int gsx,\n" \
"	unsigned int dnxy, \n" \
"	unsigned int dnx,\n" \
"	__global uint* minv,\n" \
"	__global uint* maxv)\n" \
"{\n" \
"	int3 gid = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);\n" \
"	int3 ub = (int3)(lb.x + ngx, lb.y + ngy, lb.z + ngz);\n" \
"	int4 ijk = (int4)(0, 0, 0, 1);\n" \
"	DWL lminv = VSCL;\n" \
"	DWL lmaxv = 0;\n" \
"	DWL val;\n" \
"	unsigned int index;\n" \
"	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)\n" \
"	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)\n" \
"	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)\n" \
"	{\n" \
"		index = dnxy* ijk.z + dnx*ijk.y + ijk.x;\n" \
"		val = bkg[index];\n" \
"		lminv = val ? min(val, lminv) : lminv;\n" \
"		lmaxv = max(val, lmaxv);\n" \
"	}\n" \
"	lminv = lminv == VSCL ? 0 : lminv;\n" \
"	index = gsxy * gid.z + gsx * gid.y + gid.x;\n" \
"	//atomic_xchg(minv+index, (uint)(lminv));\n" \
"	//atomic_xchg(maxv+index, (uint)(lmaxv));\n" \
"	minv[index] = (uint)(lminv);\n" \
"	maxv[index] = (uint)(lmaxv);\n" \
"}\n" \
"//histogram in background\n" \
"__kernel void kernel_4(\n" \
"	__global DWL* bkg,\n" \
"	unsigned int dnxy, \n" \
"	unsigned int dnx,\n" \
"	unsigned int minv,\n" \
"	unsigned int maxv,\n" \
"	unsigned int bin,\n" \
"	__global unsigned int* hist)\n" \
"{\n" \
"	int4 coord = (int4)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2), 1);\n" \
"	unsigned int index = dnxy* coord.z + dnx*coord.y + coord.x;\n" \
"	unsigned int val = bkg[index];\n" \
"	if (val < minv || val > maxv)\n" \
"		return;\n" \
"	index = (val - minv) * (bin - 1) / (maxv - minv);\n" \
"	atomic_inc(hist+index);\n" \
"}\n";

BackgStat::BackgStat(fluo::VolumeData* vd)
	: m_vd(vd),
	m_use_mask(false),
	m_type(0),
	m_kx(40),
	m_ky(40),
	m_kz(10),
	m_varth(0.0001),
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
	//debug
#ifdef _DEBUG
	unsigned char* val = 0;
	std::ofstream ofs;
#endif
	if (!CheckBricks())
		return;
	long bits;
	m_vd->getValue(gstBits, bits);
	int chars = bits / 8;

	//create program and kernels
	flvr::KernelProgram* kernel_prog = flvr::VolumeRenderer::
		vol_kernel_factory_.kernel(str_cl_backg_stat, bits);
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
	vector<flvr::TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks();

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
		GLint tid = m_vd->GetRenderer()->load_brick(b);
		GLint mid = 0;
		if (m_use_mask)
			mid = m_vd->GetRenderer()->load_brick_mask(b);

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
		flvr::Argument arg_bkg =
			kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY, chars*nx*ny*nz, NULL);
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

		//debug
		//val = new unsigned char[nx*ny*nz*chars];
		//kernel_prog->readBuffer(arg_bkg, val);
		//ofs.open("E:/DATA/Test/bkg/avg.bin", std::ios::out | std::ios::binary);
		//ofs.write((char*)val, nx*ny*nz*chars);
		//ofs.close();
		//delete[] val;

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
			kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(unsigned int)*(gsize.gsxyz), (void*)(sum));
			kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(float)*(gsize.gsxyz), (void*)(wsum));

			//execute
			kernel_prog->executeKernel(kernel_index1, 3, global_size1, local_size);
			//read back
			kernel_prog->readBuffer(sizeof(unsigned int)*(gsize.gsxyz), sum, sum);
			kernel_prog->readBuffer(sizeof(float)*(gsize.gsxyz), wsum, wsum);

			//sum
			for (int ii = 0; ii < gsize.gsxyz; ++ii)
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
			kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(unsigned int)*(gsize.gsxyz), (void*)(minv));
			kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(unsigned int)*(gsize.gsxyz), (void*)(maxv));

			//execute
			kernel_prog->executeKernel(kernel_index1, 3, global_size1, local_size);
			//read back
			kernel_prog->readBuffer(sizeof(unsigned int)*(gsize.gsxyz), minv, minv);
			kernel_prog->readBuffer(sizeof(unsigned int)*(gsize.gsxyz), maxv, maxv);

			//collect
			for (int ii = 0; ii < gsize.gsxyz; ++ii)
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
			unsigned int* hist = new unsigned int[bin];
			memset(hist, 0, sizeof(unsigned int)*bin);
			kernel_prog->setKernelArgBegin(kernel_index2);
			kernel_prog->setKernelArgument(arg_bkg);
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&dnxy));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&dnx));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&bminv));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&bmaxv));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&bin));
			kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(unsigned int)*(bin), (void*)(hist));

			//execute
			kernel_prog->executeKernel(kernel_index2, 3, global_size, local_size);
			//read back
			kernel_prog->readBuffer(sizeof(unsigned int)*(bin), hist, hist);

			//debug
			//ofs.open("E:/DATA/Test/bkg/hist.bin", std::ios::out | std::ios::binary);
			//ofs.write((char*)hist, bin*sizeof(unsigned int));
			//ofs.close();

			//collect
			for (int ii = 0; ii < bin; ++ii)
			{
				unsigned int key = ii + bminv;
				auto it = m_hist.find(key);
				if (it == m_hist.end())
					m_hist.insert(std::pair<unsigned int, unsigned int>(key, hist[ii]));
				else
					it->second += hist[ii];
			}
			delete[] hist;
		}

		kernel_prog->releaseAll();
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

