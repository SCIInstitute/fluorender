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
#include <KernelProgram.h>
#include <Relax.h>
#include <Global.h>
#include <Ruler.h>
#include <VolumeData.h>
#include <Texture.h>
#include <TextureBrick.h>
#include <VolumeRenderer.h>
#include <VolKernel.h>

using namespace flrd;

constexpr const char* str_cl_relax = R"CLKER(
const sampler_t samp =
	CLK_NORMALIZED_COORDS_FALSE|
	CLK_ADDRESS_CLAMP_TO_EDGE|
	CLK_FILTER_NEAREST;

__kernel void kernel_0(
	__read_only image3d_t data,
	unsigned int ngx,
	unsigned int ngy,
	unsigned int ngz,
	unsigned int gsxy,
	unsigned int gsx,
	unsigned int np,
	float3 org,
	float3 scl,
	float rest,
	float infr,
	__global float* spp,
	__global unsigned int* slk,
	__global float* gdsp,
	__global float* gwsum)
{
	int3 gid = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);
	int3 ub = (int3)(lb.x + ngx, lb.y + ngy, lb.z + ngz);
	int4 ijk = (int4)(0, 0, 0, 1);
	float w, dist;
	float wsum;
	float3 loc, pos, dir;
	float3 dsp;
	int c;
	unsigned int index = gsxy * gid.z + gsx * gid.y + gid.x;
	for (c = 0; c < np; ++c)
	{
		if (slk[c]) continue;
		dsp = (float3)(0.0f, 0.0f, 0.0f);
		wsum = 0.0f;
		pos = (float3)(spp[c*3], spp[c*3+1], spp[c*3+2]);
		for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)
		for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)
		for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)
		{
			w = read_imagef(data, samp, ijk).x;
			if (w == 0.0f) continue;
			loc = (org + (float3)(ijk.x, ijk.y, ijk.z)) * scl;
			dir = loc - pos;
			dist = length(dir);
			if (dist > infr) continue;
			dist = max(rest, dist);
			dsp += dir * w / dist / dist;
			wsum += w;
		}
		gdsp[(index*np+c)*3] = dsp.x;
		gdsp[(index*np+c)*3+1] = dsp.y;
		gdsp[(index*np+c)*3+2] = dsp.z;
		gwsum[index*np+c] = wsum;
	}
}
)CLKER";

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

void Relax::SetVolume(VolumeData* vd)
{
	m_vd = vd;
}

void Relax::SetRuler(Ruler* ruler)
{
	if (ruler != m_ruler)
	{
		m_ruler = ruler;
	}
}

Ruler* Relax::GetRuler()
{
	return m_ruler;
}

void Relax::BuildSpring()
{
	if (!m_ruler)
		return;
	int rn = m_ruler->GetNumPoint();
	if (rn < 1)
		return;
	size_t rwt = m_ruler->GetWorkTime();
	int interp = m_ruler->GetInterp();

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
				p = m_ruler->GetRulerPoint(bi, i);
				if (p)
				{
					pp = p->GetPoint(rwt, interp);
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
				p = m_ruler->GetRulerPoint(bi, i + 1);
				if (p)
				{
					pp = p->GetPoint(rwt, interp);
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
	if (idx < 0 || static_cast<unsigned int>(idx) >= m_snum)
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
	m_vd->GetSpacings(dx, dy, dz);
	cl_float3 scl = { (float)dx, (float)dy, (float)dz };

	m_dsp.assign(m_snum * 3, 0.0);
	m_wsum.assign(m_snum, 0.0);

	long bits = m_vd->GetBits();
	float max_int = static_cast<float>(m_vd->GetMaxValue());

	//create program and kernels
	flvr::KernelProgram* kernel_prog = glbin_vol_kernel_factory.kernel(str_cl_relax, bits, max_int);
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
			tid = m_vd->GetVR()->load_brick_mask(b);
		else
			tid = m_vd->GetVR()->load_brick(b);

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
		for (size_t j = 0; j < m_snum; ++j)
		{
			for (size_t i = 0; i < gsize.gsxyz; ++i)
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