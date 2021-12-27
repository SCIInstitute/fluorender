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
#include "Relax.h"
#include <VolumeData.hpp>
#include <FLIVR/KernelProgram.h>
#include <FLIVR/VolKernel.h>
#include <FLIVR/VolumeRenderer.h>

using namespace flrd;

const char* str_cl_relax = \
"const sampler_t samp =\n" \
"	CLK_NORMALIZED_COORDS_FALSE|\n" \
"	CLK_ADDRESS_CLAMP_TO_EDGE|\n" \
"	CLK_FILTER_NEAREST;\n" \
"\n" \
"__kernel void kernel_0(\n" \
"	__read_only image3d_t data,\n" \
"	unsigned int ngx,\n" \
"	unsigned int ngy,\n" \
"	unsigned int ngz,\n" \
"	unsigned int gsxy,\n" \
"	unsigned int gsx,\n" \
"	unsigned int np,\n" \
"	float3 org,\n" \
"	float3 scl,\n" \
"	float rest,\n" \
"	float infr,\n" \
"	__global float* spp,\n" \
"	__global unsigned int* slk,\n" \
"	__global float* gdsp,\n" \
"	__global float* gwsum)\n" \
"{\n" \
"	int3 gid = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);\n" \
"	int3 ub = (int3)(lb.x + ngx, lb.y + ngy, lb.z + ngz);\n" \
"	int4 ijk = (int4)(0, 0, 0, 1);\n" \
"	float w, dist;\n" \
"	float wsum;\n" \
"	float3 loc, pos, dir;\n" \
"	float3 dsp;\n" \
"	int c;\n" \
"	unsigned int index = gsxy * gid.z + gsx * gid.y + gid.x;\n" \
"	for (c = 0; c < np; ++c)\n" \
"	{\n" \
"		if (slk[c]) continue;\n" \
"		dsp = (float3)(0.0, 0.0, 0.0);\n" \
"		wsum = 0.0;\n" \
"		pos = (float3)(spp[c*3], spp[c*3+1], spp[c*3+2]);\n" \
"		for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)\n" \
"		for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)\n" \
"		for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)\n" \
"		{\n" \
"			w = read_imagef(data, samp, ijk).x;\n" \
"			if (w == 0.0) continue;\n" \
"			loc = (org + (float3)(ijk.x, ijk.y, ijk.z)) * scl;\n" \
"			dir = loc - pos;\n" \
"			dist = length(dir);\n" \
"			if (dist > infr) continue;\n" \
"			dist = max(rest, dist);\n" \
"			dsp += dir * w / dist / dist;\n" \
"			wsum += w;\n" \
"		}\n" \
"		atomic_xchg(gdsp+(index*np+c)*3, dsp.x);\n" \
"		atomic_xchg(gdsp+(index*np+c)*3+1, dsp.y);\n" \
"		atomic_xchg(gdsp+(index*np+c)*3+2, dsp.z);\n" \
"		atomic_xchg(gwsum+index*np+c, wsum);\n" \
"	}\n" \
"}\n" \
;

Relax::Relax() :
	m_ruler(0),
	m_vd(0),
	m_snum(0),
	m_use_mask(true),
	m_rest(0.0f),
	m_infr(0.0f)
{
}

Relax::~Relax()
{
}

void Relax::BuildSpring()
{
	if (!m_ruler)
		return;
	int rn = m_ruler->GetNumPoint();
	if (rn < 1)
		return;

	if (!m_spoints.empty())
		m_spoints.clear();
	if (!m_slock.empty())
		m_slock.clear();
	m_snum = 0;

	//build a spring form the ruler
	double dist;
	RulerPoint *p;
	fluo::Point pp, pp1;
	bool locked;
	int bn = m_ruler->GetNumBranch();
	for (int bi = 0; bi < bn; ++bi)
	{
		rn = m_ruler->GetNumBranchPoint(bi);
		int n = rn == 1 ? 1 : rn - 1;
		for (int i = 0; i < n; ++i)
		{
			if (i == 0)
			{
				p = m_ruler->GetPoint(bi, i);
				if (p)
				{
					pp = p->GetPoint();
					locked = p->GetLocked();
					m_spoints.push_back(float(pp.x()));
					m_spoints.push_back(float(pp.y()));
					m_spoints.push_back(float(pp.z()));
					m_slock.push_back(locked);
					m_snum++;
				}
			}

			if (rn > 1)
			{
				p = m_ruler->GetPoint(bi, i + 1);
				if (p)
				{
					pp = p->GetPoint();
					locked = p->GetLocked();
					pp1 = fluo::Point(
						m_spoints.end()[-3],
						m_spoints.end()[-2],
						m_spoints.end()[-1]);
					dist = (pp - pp1).length();
					m_spoints.push_back(float(pp.x()));
					m_spoints.push_back(float(pp.y()));
					m_spoints.push_back(float(pp.z()));
					m_slock.push_back(locked);
					m_snum++;
				}
			}
		}
	}
}

fluo::Vector Relax::GetDisplacement(int idx)
{
	fluo::Vector v;
	if (idx < 0 || idx >= m_snum)
		return v;

	double w = m_wsum[idx];
	if (w > 0.0)
	{
		v = fluo::Vector(
			m_dsp[idx * 3],
			m_dsp[idx * 3 + 1],
			m_dsp[idx * 3 + 2]);
		v /= w;
	}
	return v;
}

bool Relax::Compute()
{
	if (!m_vd || !m_ruler)
		return false;

	BuildSpring();

	double dx, dy, dz;
	m_vd->getValue(gstSpcX, dx);
	m_vd->getValue(gstSpcY, dy);
	m_vd->getValue(gstSpcZ, dz);
	cl_float3 scl = { (float)dx, (float)dy, (float)dz };

	m_dsp.assign(m_snum * 3, 0.0);
	m_wsum.assign(m_snum, 0.0);

	//create program and kernels
	flvr::KernelProgram* kernel_prog = flvr::VolumeRenderer::
		vol_kernel_factory_.kernel(str_cl_relax);
	if (!kernel_prog)
		return false;
	int kernel_0 = kernel_prog->createKernel("kernel_0");//init ordered

	size_t brick_num = m_vd->GetTexture()->get_brick_num();
	std::vector<flvr::TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks();
	for (size_t bi = 0; bi < brick_num; ++bi)
	{
		flvr::TextureBrick* b = (*bricks)[bi];
		int nx = b->nx();
		int ny = b->ny();
		int nz = b->nz();
		int ox = b->ox();
		int oy = b->oy();
		int oz = b->oz();
		cl_float3 org = { (float)ox, (float)oy, (float)oz };
		GLint tid;
		if (m_use_mask)
			tid = m_vd->GetRenderer()->load_brick_mask(b);
		else
			tid = m_vd->GetRenderer()->load_brick(b);

		//compute workload
		flvr::GroupSize gsize;
		kernel_prog->get_group_size(kernel_0, nx, ny, nz, gsize);
		size_t global_size[3] = {
			size_t(gsize.gsx), size_t(gsize.gsy), size_t(gsize.gsz) };
		size_t local_size[3] = { 1, 1, 1 };

		//set
		std::vector<float> dsp(gsize.gsxyz * m_snum * 3, 0.0);
		float* pdsp = dsp.data();
		std::vector<float> wsum(gsize.gsxyz * m_snum, 0.0);
		float* pwsum = wsum.data();
		kernel_prog->setKernelArgBegin(kernel_0);
		kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, tid);
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngx));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngy));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngz));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.gsxy));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.gsx));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&m_snum));
		kernel_prog->setKernelArgConst(sizeof(cl_float3), (void*)(&org));
		kernel_prog->setKernelArgConst(sizeof(cl_float3), (void*)(&scl));
		kernel_prog->setKernelArgConst(sizeof(float), (void*)(&m_rest));
		kernel_prog->setKernelArgConst(sizeof(float), (void*)(&m_infr));
		kernel_prog->setKernelArgBuf(CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float)*m_snum * 3, (void*)(m_spoints.data()));
		kernel_prog->setKernelArgBuf(CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(unsigned int)*m_snum, (void*)(m_slock.data()));
		kernel_prog->setKernelArgBuf(CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float)*gsize.gsxyz * m_snum * 3, (void*)(pdsp));
		kernel_prog->setKernelArgBuf(CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float)*gsize.gsxyz * m_snum, (void*)(pwsum));

		//execute
		kernel_prog->executeKernel(kernel_0, 3, global_size, local_size);

		//read back
		kernel_prog->readBuffer(sizeof(float)*gsize.gsxyz * m_snum * 3, pdsp, pdsp);
		kernel_prog->readBuffer(sizeof(float)*gsize.gsxyz * m_snum, pwsum, pwsum);

		//release buffer
		kernel_prog->releaseAll();

		//collect data
		for (int j = 0; j < m_snum; ++j)
		{
			for (int i = 0; i < gsize.gsxyz; ++i)
			{
				m_dsp[j * 3] += dsp[(m_snum * i + j) * 3];
				m_dsp[j * 3 + 1] += dsp[(m_snum * i + j) * 3 + 1];
				m_dsp[j * 3 + 2] += dsp[(m_snum * i + j) * 3 + 2];
				m_wsum[j] += wsum[i * m_snum + j];
			}
		}
	}
	
	return true;
}