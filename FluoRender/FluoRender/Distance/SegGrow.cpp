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
#include <SegGrow.h>
#include <Global.h>
#include <VolumeData.h>
#include <KernelProgram.h>
#include <Kernel.h>
#include <RulerHandler.h>
#include <Texture.h>
#include <TextureBrick.h>
#include <VolumeRenderer.h>
#include <algorithm>
#include <fstream>
#include <Debug.h>

using namespace flrd;

constexpr const char* str_cl_segrow = R"CLKER(
const sampler_t samp =
	CLK_NORMALIZED_COORDS_FALSE|
	CLK_ADDRESS_CLAMP_TO_EDGE|
	CLK_FILTER_NEAREST;

//initialize new mask regions to ids (ordered)
__kernel void kernel_0(
	__read_only image3d_t mask,
	__global unsigned int* label,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz)
{
	unsigned int i = (unsigned int)(get_global_id(0));
	unsigned int j = (unsigned int)(get_global_id(1));
	unsigned int k = (unsigned int)(get_global_id(2));
	float value = read_imagef(mask, samp, (int4)(i, j, k, 1)).x;
	unsigned int index = nx*ny*k + nx*j + i;
	unsigned int lv = index + 1;
	if (value == 0.0f)
		lv = 0;
	label[index] = lv;
}
//initialize but keep old values
__kernel void kernel_1(
	__read_only image3d_t mask,
	__global unsigned int* label,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz)
{
	unsigned int i = (unsigned int)(get_global_id(0));
	unsigned int j = (unsigned int)(get_global_id(1));
	unsigned int k = (unsigned int)(get_global_id(2));
	float value = read_imagef(mask, samp, (int4)(i, j, k, 1)).x;
	if (value == 0.0f)
		return;
	unsigned int index = nx*ny*k + nx*j + i;
	if (label[index] > 0)
		return;
	label[index] = index + 1;
}
//grow ids reverse
__kernel void kernel_2(
	__global unsigned int* label,
	unsigned int nx,
	unsigned int ny,
	unsigned int nxy,
	unsigned int nz,
	unsigned int ngx,
	unsigned int ngy,
	unsigned int ngz)
{
	int3 gid = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);
	int3 ub = (int3)(lb.x + ngx - 1, lb.y + ngy - 1, lb.z + ngz - 1);
	int3 ijk = (int3)(0, 0, 0);
	unsigned int index;
	unsigned int label_v;
	unsigned int m, label_m;
	for (ijk.z = ub.z; ijk.z >= lb.z; --ijk.z)
	for (ijk.y = ub.y; ijk.y >= lb.y; --ijk.y)
	for (ijk.x = ub.x; ijk.x >= lb.x; --ijk.x)
	{
		if (ijk.x >= nx || ijk.y >= ny || ijk.z >= nz)
			continue;
		index = nxy*ijk.z + nx*ijk.y + ijk.x;
		label_v = label[index];
		if (label_v == 0 || label_v & 0x80000000)
			continue;
		label_m = label_v;
		//search neighbors
		for (int i = -1; i < 2; ++i)
		for (int j = -1; j < 2; ++j)
		for (int k = -1; k < 2; ++k)
		{
			if (ijk.x < 1 || ijk.x > nx-2 || ijk.y < 1 || ijk.y > ny-2 || ijk.z < 1 || ijk.z > nz-2)
				continue;
			m = label[nxy*(ijk.z+i) + nx*(ijk.y+j) + ijk.x + k];
			if (m  && !(m & 0x80000000))
				label_m = max(label_m, m);
		}
		if (label_m != label_v)
			label[index] = label_m;
	}
}
//grow id ordered
__kernel void kernel_3(
	__global unsigned int* label,
	unsigned int nx,
	unsigned int ny,
	unsigned int nxy,
	unsigned int nz,
	unsigned int ngx,
	unsigned int ngy,
	unsigned int ngz)
{
	int3 gid = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);
	int3 ub = (int3)(lb.x + ngx, lb.y + ngy, lb.z + ngz);
	int3 ijk = (int3)(0, 0, 0);
	unsigned int index;
	unsigned int label_v;
	unsigned int m, label_m;
	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)
	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)
	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)
	{
		if (ijk.x >= nx || ijk.y >= ny || ijk.z >= nz)
			continue;
		index = nxy*ijk.z + nx*ijk.y + ijk.x;
		label_v = label[index];
		if (label_v == 0 || label_v & 0x80000000)
			continue;
		label_m = label_v;
		//search neighbors
		for (int i = -1; i < 2; ++i)
		for (int j = -1; j < 2; ++j)
		for (int k = -1; k < 2; ++k)
		{
			if (ijk.x < 1 || ijk.x > nx-2 || ijk.y < 1 || ijk.y > ny-2 || ijk.z < 1 || ijk.z > nz-2)
				continue;
			m = label[nxy*(ijk.z+i) + nx*(ijk.y+j) + ijk.x + k];
			if (m  && !(m & 0x80000000))
				label_m = max(label_m, m);
		}
		if (label_m != label_v)
			label[index] = label_m;
	}
}
//count newly grown labels
__kernel void kernel_4(
	__global unsigned int* label,
	unsigned int nx,
	unsigned int ny,
	unsigned int nxy,
	unsigned int nz,
	unsigned int ngx,
	unsigned int ngy,
	unsigned int ngz,
	unsigned int gsxy,
	unsigned int gsx,
	unsigned int maxc,
	__global unsigned int* count,
	__global unsigned int* ids,
	__local unsigned int* lids)
{
	int3 gid = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);
	int3 ub = (int3)(lb.x + ngx, lb.y + ngy, lb.z + ngz);
	int3 ijk = (int3)(0, 0, 0);
	unsigned int lcount = 0;
	unsigned int index;
	unsigned int label_v;
	bool found;
	int c;
	for (c = 0; c < maxc; ++c)
		lids[c] = 0;
	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)
	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)
	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)
	{
		if (ijk.x >= nx || ijk.y >= ny || ijk.z >= nz)
			continue;
		index = nxy*ijk.z + nx*ijk.y + ijk.x;
		label_v = label[index];
		if (label_v == 0 || label_v & 0x80000000)
			continue;
		found = false;
		for (c = 0; c < lcount; ++c)
		if (lids[c] == label_v)
		{
			found = true;
			break;
		}
		if (!found && lcount < maxc)
		{
			lids[lcount] = label_v;
			lcount++;
		}
	}
	index = gsxy * gid.z + gsx * gid.y + gid.x;
	count[index] = lcount;
	for (c = 0; c < lcount; ++c)
		ids[index*maxc+c] = lids[c];
}
//find connected parts
__kernel void kernel_5(
	__global unsigned int* label,
	unsigned int nx,
	unsigned int ny,
	unsigned int nxy,
	unsigned int nz,
	unsigned int ngx,
	unsigned int ngy,
	unsigned int ngz,
	unsigned int gsxy,
	unsigned int gsx,
	unsigned int nid,
	__global unsigned int* ids,
	__global unsigned int* cids,
	__local unsigned int* lcids)
{
	int3 gid = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);
	int3 ub = (int3)(lb.x + ngx, lb.y + ngy, lb.z + ngz);
	int3 ijk = (int3)(0, 0, 0);
	unsigned int index;
	unsigned int label_v;
	unsigned int m;
	bool found;
	int c;
	for (c = 0; c < nid * 6; ++c)
		lcids[c] = 0;
	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)
	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)
	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)
	{
		if (ijk.x >= nx || ijk.y >= ny || ijk.z >= nz)
			continue;
		index = nxy*ijk.z + nx*ijk.y + ijk.x;
		label_v = label[index];
		if (label_v == 0 || label_v & 0x80000000)
			continue;
		found = false;
		for (c = 0; c < nid; ++c)
		if (ids[c] == label_v)
		{
			found = true;
			break;
		}
		if (!found)
			continue;
		//-x
		if (ijk.x > 0)
		{
			m = label[index - 1];
			if (m != label_v && !(m & 0x80000000))
				lcids[c*6] = m;
		}
		//+x
		if (ijk.x < nx-1)
		{
			m = label[index + 1];
			if (m != label_v && !(m & 0x80000000))
				lcids[c*6+1] = m;
		}
		//-y
		if (ijk.y > 0)
		{
			m = label[index - nx];
			if (m != label_v && !(m & 0x80000000))
				lcids[c*6+2] = m;
		}
		//+y
		if (ijk.y < ny-1)
		{
			m = label[index + nx];
			if (m != label_v && !(m & 0x80000000))
				lcids[c*6+3] = m;
		}
		//-z
		if (ijk.z > 0)
		{
			m = label[index - nxy];
			if (m != label_v && !(m & 0x80000000))
				lcids[c*6+4] = m;
		}
		//+z
		if (ijk.z < nz-1)
		{
			m = label[index + nxy];
			if (m != label_v && !(m & 0x80000000))
				lcids[c*6+5] = m;
		}
	}
	index = gsxy * gid.z + gsx * gid.y + gid.x;
	for (c = 0; c < nid*6; ++c)
		cids[(index*nid)*6+c] = lcids[c];
}
//merge connected ids
__kernel void kernel_6(
	__global unsigned int* label,
	unsigned int nx,
	unsigned int ny,
	unsigned int nxy,
	unsigned int nz,
	unsigned int ngx,
	unsigned int ngy,
	unsigned int ngz,
	unsigned int nid,
	__global unsigned int* mids)
{
	int3 gid = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);
	int3 ub = (int3)(lb.x + ngx, lb.y + ngy, lb.z + ngz);
	int3 ijk = (int3)(0, 0, 0);
	unsigned int index;
	unsigned int label_v;
	bool found;
	int c;
	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)
	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)
	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)
	{
		if (ijk.x >= nx || ijk.y >= ny || ijk.z >= nz)
			continue;
		index = nxy*ijk.z + nx*ijk.y + ijk.x;
		label_v = label[index];
		if (label_v == 0 || label_v & 0x80000000)
			continue;
		found = false;
		for (c = 0; c < nid; ++c)
		if (mids[c*2] == label_v)
		{
			found = true;
			break;
		}
		if (!found)
			continue;
		label[index] = mids[c*2+1];
	}
}
//find connectivity/center of new ids
__kernel void kernel_7(
	__global unsigned int* label,
	unsigned int nx,
	unsigned int ny,
	unsigned int nxy,
	unsigned int nz,
	unsigned int ngx,
	unsigned int ngy,
	unsigned int ngz,
	unsigned int gsxy,
	unsigned int gsx,
	unsigned int nid,
	__global unsigned int* ids,
	__global unsigned int* cids,
	__global unsigned int* sum,
	__global float* csum,
	__local unsigned int* lcids,
	__local unsigned int* lsum,
	__local float* lcsum)
{
	int3 gid = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);
	int3 ub = (int3)(lb.x + ngx, lb.y + ngy, lb.z + ngz);
	int3 ijk = (int3)(0, 0, 0);
	unsigned int index;
	unsigned int label_v;
	unsigned int m;
	bool found;
	int c;
	for (c = 0; c < nid; ++c)
	{
		lcids[c*3] = 0;
		lcids[c*3+1] = 0;
		lcids[c*3+2] = 0;
		lsum[c] = 0;
		lcsum[c*3] = 0.0f;
		lcsum[c*3+1] = 0.0f;
		lcsum[c*3+2] = 0.0f;
	}
	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)
	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)
	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)
	{
		if (ijk.x >= nx || ijk.y >= ny || ijk.z >= nz)
			continue;
		index = nxy*ijk.z + nx*ijk.y + ijk.x;
		label_v = label[index];
		if (label_v == 0 || label_v & 0x80000000)
			continue;
		found = false;
		for (c = 0; c < nid; ++c)
		if (ids[c] == label_v)
		{
			found = true;
			break;
		}
		if (!found)
			continue;
		lsum[c]++;
		lcsum[c*3] += (float)(ijk.x);
		lcsum[c*3+1] += (float)(ijk.y);
		lcsum[c*3+2] += (float)(ijk.z);
		//search neighbors
		for (int i = -1; i < 2; ++i)
		for (int j = -1; j < 2; ++j)
		for (int k = -1; k < 2; ++k)
		{
			if (ijk.x < 1 || ijk.x > nx-2 || ijk.y < 1 || ijk.y > ny-2 || ijk.z < 1 || ijk.z > nz-2)
				continue;
			m = label[nxy*(ijk.z+i) + nx*(ijk.y+j) + ijk.x + k];
			if (m != label_v && m & 0x80000000)
			{
				lcids[c*3] = lcids[c*3]?lcids[c*3]:m;
				lcids[c*3+1] = lcids[c*3]&&!lcids[c*3+1]&&lcids[c*3]!=m?m:lcids[c*3+1];
				lcids[c*3+2] = lcids[c*3+1]&&!lcids[c*3+2]&&lcids[c*3+1]!=m?m:lcids[c*3+2];
			}
		}
	}
	index = gsxy * gid.z + gsx * gid.y + gid.x;
	for (c = 0; c < nid; ++c)
	{
		cids[(index*nid+c)*3] = lcids[c*3];
		cids[(index*nid+c)*3+1] = lcids[c*3+1];
		cids[(index*nid+c)*3+2] = lcids[c*3+2];
		sum[index*nid+c] = lsum[c];
		csum[(index*nid+c)*3] = lcsum[c*3];
		csum[(index*nid+c)*3+1] = lcsum[c*3+1];
		csum[(index*nid+c)*3+2] = lcsum[c*3+2];
	}
}
//fix processed ids
__kernel void kernel_8(
	__global unsigned int* label,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz)
{
	unsigned int i = (unsigned int)(get_global_id(0));
	unsigned int j = (unsigned int)(get_global_id(1));
	unsigned int k = (unsigned int)(get_global_id(2));
	if (i >= nx || j >= ny || k >= nz)
		return;
	unsigned int index = nx*ny*k + nx*j + i;
	unsigned int label_v = label[index];
	if (label_v == 0 || label_v & 0x80000000)
		return;
	label[index] = label[index] | 0x80000000;
}
)CLKER";

constexpr const char* str_cl_sg_check_borders = R"CLKER(
const sampler_t samp =
	CLK_NORMALIZED_COORDS_FALSE|
	CLK_ADDRESS_CLAMP_TO_EDGE|
	CLK_FILTER_NEAREST;

//check yz plane (+-x)
__kernel void kernel_0(
	__read_only image3d_t l0,
	__read_only image3d_t l1,
	unsigned int d0,
	unsigned int d1,
	unsigned int nid,
	__global unsigned int* ids,
	__global unsigned int* cids)
{
	unsigned int i = (unsigned int)(get_global_id(0));
	unsigned int j = (unsigned int)(get_global_id(1));
	unsigned int v0 = read_imageui(l0, samp, (int4)(d0, i, j, 1)).x;
	if (v0 == 0 || v0 & 0x80000000)
		return;
	unsigned int v1 = read_imageui(l1, samp, (int4)(d1, i, j, 1)).x;
	if (v1 == 0 || v1 & 0x80000000)
		return;
	bool found = false;
	int c;
	for (c = 0; c < nid; ++c)
	if (ids[c] == v0)
	{
		found = true;
		break;
	}
	if (!found)
		return;
	cids[c*3] = cids[c*3]?cids[c*3]:v1;
	cids[c*3+1] = cids[c*3]&&!cids[c*3+1]&&cids[c*3]!=v1?v1:cids[c*3+1];
	cids[c*3+2] = cids[c*3+1]&&!cids[c*3+2]&&cids[c*3+1]!=v1?v1:cids[c*3+2];
}
//check xz plane (+-y)
__kernel void kernel_1(
	__read_only image3d_t l0,
	__read_only image3d_t l1,
	unsigned int d0,
	unsigned int d1,
	unsigned int nid,
	__global unsigned int* ids,
	__global unsigned int* cids)
{
	unsigned int i = (unsigned int)(get_global_id(0));
	unsigned int j = (unsigned int)(get_global_id(1));
	unsigned int v0 = read_imageui(l0, samp, (int4)(i, d0, j, 1)).x;
	if (v0 == 0 || v0 & 0x80000000)
		return;
	unsigned int v1 = read_imageui(l1, samp, (int4)(i, d1, j, 1)).x;
	if (v1 == 0 || v1 & 0x80000000)
		return;
	bool found = false;
	int c;
	for (c = 0; c < nid; ++c)
	if (ids[c] == v0)
	{
		found = true;
		break;
	}
	if (!found)
		return;
	cids[c*3] = cids[c*3]?cids[c*3]:v1;
	cids[c*3+1] = cids[c*3]&&!cids[c*3+1]&&cids[c*3]!=v1?v1:cids[c*3+1];
	cids[c*3+2] = cids[c*3+1]&&!cids[c*3+2]&&cids[c*3+1]!=v1?v1:cids[c*3+2];
}
//check xy plane (+-z)
__kernel void kernel_2(
	__read_only image3d_t l0,
	__read_only image3d_t l1,
	unsigned int d0,
	unsigned int d1,
	unsigned int nid,
	__global unsigned int* ids,
	__global unsigned int* cids)
{
	unsigned int i = (unsigned int)(get_global_id(0));
	unsigned int j = (unsigned int)(get_global_id(1));
	unsigned int v0 = read_imageui(l0, samp, (int4)(i, j, d0, 1)).x;
	if (v0 == 0 || v0 & 0x80000000)
		return;
	unsigned int v1 = read_imageui(l1, samp, (int4)(i, j, d1, 1)).x;
	if (v1 == 0 || v1 & 0x80000000)
		return;
	bool found = false;
	int c;
	for (c = 0; c < nid; ++c)
	if (ids[c] == v0)
	{
		found = true;
		break;
	}
	if (!found)
		return;
	cids[c*3] = cids[c*3]?cids[c*3]:v1;
	cids[c*3+1] = cids[c*3]&&!cids[c*3+1]&&cids[c*3]!=v1?v1:cids[c*3+1];
	cids[c*3+2] = cids[c*3+1]&&!cids[c*3+2]&&cids[c*3+1]!=v1?v1:cids[c*3+2];
}
)CLKER";

SegGrow::SegGrow():
	m_vd(0),
	m_branches(10),
	m_iter(0),
	m_sz_thresh(10)
{
}

SegGrow::~SegGrow()
{

}

void SegGrow::SetVolumeData(VolumeData* vd)
{
	m_vd = vd;
}

void SegGrow::SetRulerHandler(RulerHandler* handler)
{
	m_handler = handler;
}

bool SegGrow::CheckBricks()
{
	if (!m_vd || !m_vd->GetTexture())
		return false;
	std::vector<flvr::TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks();
	if (!bricks || bricks->size() == 0)
		return false;
	return true;
}

void SegGrow::Compute()
{
	//debug
#ifdef _DEBUG
	unsigned int* val = 0;
	std::ofstream ofs;
#endif

	if (!m_handler)
		return;
	if (!CheckBricks())
		return;

	m_list.clear();
	bool clear_label = m_vd->GetMaskClear();
	m_vd->SetMaskClear(false);
	long bits = m_vd->GetBits();
	float max_int = static_cast<float>(m_vd->GetMaxValue());

	//create program and kernels
	flvr::KernelProgram* kernel_prog = glbin_kernel_factory.kernel(str_cl_segrow, bits, max_int);
	if (!kernel_prog)
		return;
	int kernel_0 = kernel_prog->createKernel(
		clear_label?"kernel_0":"kernel_1");//init ordered
	//int kernel_0 = kernel_prog->createKernel("kernel_1");
	int kernel_1 = kernel_prog->createKernel("kernel_2");//grow reverse
	int kernel_2 = kernel_prog->createKernel("kernel_3");//grow ordered
	int kernel_3 = kernel_prog->createKernel("kernel_4");//count
	int kernel_4 = kernel_prog->createKernel("kernel_5");//find connected parts
	int kernel_5 = kernel_prog->createKernel("kernel_6");//merge connected parts
	int kernel_6 = kernel_prog->createKernel("kernel_7");//get shape

	int bnum = 0;
	size_t brick_num = m_vd->GetTexture()->get_brick_num();
	std::vector<flvr::TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks();
	for (size_t bi = 0; bi < brick_num; ++bi)
	{
		flvr::TextureBrick* b = (*bricks)[bi];
		if (!b->is_mask_act())
			continue;
		//clear new grown flag
		b->set_new_grown(false);
		int nx = b->nx();
		int ny = b->ny();
		int nz = b->nz();
		GLint mid = m_vd->GetVR()->load_brick_mask(b);
		GLint lid = m_vd->GetVR()->load_brick_label(b);

		//compute workload
		flvr::GroupSize gsize;
		kernel_prog->get_group_size2(kernel_3, nx, ny, nz, gsize);
		unsigned int nxy = nx * ny;
		size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };
		size_t global_size2[3] = {
			size_t(gsize.gsx), size_t(gsize.gsy), size_t(gsize.gsz) };
		size_t local_size[3] = { 1, 1, 1 };

		//set
		size_t region[3] = { (size_t)nx, (size_t)ny, (size_t)nz };
		//kernel0: init ordered
		kernel_prog->setKernelArgBegin(kernel_0);
		kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, mid);
		auto arg_label =
			kernel_prog->copyTex3DToArgBuf(CL_MEM_READ_WRITE, lid, "arg_label", sizeof(unsigned int) * nx * ny * nz, region);
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nz));
		//kernel1: grow reverse
		kernel_prog->setKernelArgBegin(kernel_1);
		kernel_prog->setKernelArgument(arg_label);
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nxy));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngx));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngy));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngz));
		//kernel2: grow ordered
		kernel_prog->setKernelArgBegin(kernel_2);
		kernel_prog->setKernelArgument(arg_label);
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nxy));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngx));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngy));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngz));
		//kernel3: count ids
		std::vector<unsigned int> count(gsize.gsxyz, 0);
		unsigned int* pcount = count.data();
		std::vector<unsigned int> ids(m_branches*gsize.gsxyz, 0);
		unsigned int* pids = ids.data();
		kernel_prog->setKernelArgBegin(kernel_3);
		kernel_prog->setKernelArgument(arg_label);
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nxy));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngx));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngy));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngz));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.gsxy));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.gsx));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&m_branches));
		auto arg_pcount =
			kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, "arg_pcount", sizeof(unsigned int) * gsize.gsxyz, (void*)(pcount));
		auto arg_pids =
			kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, "arg_pids", sizeof(unsigned int) * m_branches * gsize.gsxyz, (void*)(pids));
		kernel_prog->setKernelArgLocal(sizeof(unsigned int)*m_branches);
		
		//debug
		//val = new unsigned int[nx*ny*nz];
		//kernel_prog->readBuffer(arg_label, val);
		//ofs.open("E:/DATA/Test/grow/labelbuf.bin", std::ios::out | std::ios::binary);
		//ofs.write((char*)val, nx*ny*nz*sizeof(unsigned int));
		//delete[] val;
		//ofs.close();
		//first pass
		kernel_prog->executeKernel(kernel_0, 3, global_size, local_size);
		for (int i = 0; i < 2; ++i)
		{
			kernel_prog->executeKernel(kernel_1, 3, global_size2, local_size);
			kernel_prog->executeKernel(kernel_2, 3, global_size2, local_size);
		}
		kernel_prog->executeKernel(kernel_3, 3, global_size2, local_size);

		//read back
		kernel_prog->readBuffer(arg_pcount, pcount);
		kernel_prog->readBuffer(arg_pids, pids);

		//get count and ids
		std::set<unsigned int> uniqids;
		for (size_t i = 0; i < gsize.gsxyz; ++i)
		{
			if (count[i])
				for (size_t j = 0; j < count[i]; ++j)
					uniqids.insert(ids[i*m_branches+j]);
		}
		size_t total = uniqids.size();
		if (total)
		{
			b->set_new_grown(true);//new ids exist in brick, will be considered for merging
			ids.clear();
			for (auto it = uniqids.begin();
				it != uniqids.end(); ++it)
				ids.push_back(*it);
			pids = ids.data();

			//kernel4: connect ids
			std::vector<unsigned int> dids(total*gsize.gsxyz*6, 0);
			unsigned int *pdids = dids.data();
			kernel_prog->setKernelArgBegin(kernel_4);
			kernel_prog->setKernelArgument(arg_label);
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nx));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&ny));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nxy));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nz));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngx));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngy));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngz));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.gsxy));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.gsx));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&total));
			kernel_prog->setKernelArgBuf(CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, "arg_pids", sizeof(unsigned int) * total, (void*)(pids));
			auto arg_pdids =
				kernel_prog->setKernelArgBuf(CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR, "arg_pdids", sizeof(unsigned int) * total * gsize.gsxyz * 6, (void*)(pdids));
			kernel_prog->setKernelArgLocal(sizeof(unsigned int)*total*6);

			//execute
			kernel_prog->executeKernel(kernel_4, 3, global_size2, local_size);

			//read back
			kernel_prog->readBuffer(arg_pdids, pdids);

			//merge ids
			std::vector<std::set<unsigned int>> id_set;
			for (size_t i = 0; i < total; ++i)
			{
				unsigned int id = ids[i];
				bool found = false;
				std::vector<std::set<unsigned int>>::iterator it;
				for (it = id_set.begin();
					it != id_set.end(); ++it)
				{
					auto it2 = it->find(id);
					if (it2 != it->end())
					{
						found = true;
						break;
					}
				}
				if (!found)
				{
					id_set.push_back(std::set<unsigned int>{id});
					it = id_set.end() - 1;
				}

				for (size_t j = 0; j < gsize.gsxyz; ++j)
				{
					for (int k = 0; k < 6; ++k)
					{
						id = pdids[(j * total + i)*6 + k];
						if (!id)
							continue;
						it->insert(id);
					}
				}
			}

			//kernel5: merge connected ids
			total = id_set.size();
			ids.clear();
			std::vector<unsigned int> merge_id;
			for (auto it = id_set.begin();
				it != id_set.end(); ++it)
			{
				if (it->size() < 2)
				{
					if (!it->empty())
						ids.push_back(*it->begin());
					continue;
				}
				unsigned int id = *(--it->end());
				for (auto it2 = it->begin();
					it2 != it->end(); ++it2)
				{
					if (*it2 != id)
					{
						merge_id.push_back(*it2);
						merge_id.push_back(id);
					}
				}
				ids.push_back(id);
			}
			pids = ids.data();
			if (!merge_id.empty())
			{
				unsigned int nmid = static_cast<unsigned int>(merge_id.size() / 2);
				kernel_prog->setKernelArgBegin(kernel_5);
				kernel_prog->setKernelArgument(arg_label);
				kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nx));
				kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&ny));
				kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nxy));
				kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nz));
				kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngx));
				kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngy));
				kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngz));
				kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nmid));
				kernel_prog->setKernelArgBuf(CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, "arg_merge_id", sizeof(unsigned int) * nmid * 2, (void*)(merge_id.data()));

				//execute
				kernel_prog->executeKernel(kernel_5, 3, global_size2, local_size);
			}

			//set
			//kernel6: get shape
			std::vector<unsigned int> cids(total*gsize.gsxyz*3, 0);
			unsigned int *pcids = cids.data();
			std::vector<unsigned int> sum(total*gsize.gsxyz, 0);
			unsigned int *psum = sum.data();
			std::vector<float> csum(total*gsize.gsxyz * 3, 0.0f);
			float* pcsum = csum.data();
			kernel_prog->setKernelArgBegin(kernel_6);
			kernel_prog->setKernelArgument(arg_label);
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nx));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&ny));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nxy));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nz));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngx));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngy));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngz));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.gsxy));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.gsx));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&total));
			kernel_prog->setKernelArgBuf(CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, "arg_pids", sizeof(unsigned int) * total, (void*)(pids));
			auto arg_pcids =
				kernel_prog->setKernelArgBuf(CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR, "arg_pcids", sizeof(unsigned int) * total * gsize.gsxyz * 3, (void*)(pcids));
			auto arg_psum =
				kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, "arg_psum", sizeof(unsigned int) * total * gsize.gsxyz, (void*)(psum));
			auto arg_pcsum =
				kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, "arg_pcsum", sizeof(float) * total * gsize.gsxyz * 3, (void*)(pcsum));
			kernel_prog->setKernelArgLocal(sizeof(unsigned int)*total*3);
			kernel_prog->setKernelArgLocal(sizeof(unsigned int)*total);
			kernel_prog->setKernelArgLocal(sizeof(float)*total * 3);

			//execute
			kernel_prog->executeKernel(kernel_6, 3, global_size2, local_size);

			//read back
			kernel_prog->readBuffer(arg_pcids, pcids);
			kernel_prog->readBuffer(arg_psum, psum);
			kernel_prog->readBuffer(arg_pcsum, pcsum);

			int ox, oy, oz, nc;
			ox = b->ox(); oy = b->oy(); oz = b->oz();
			//compute centers and connection
			unsigned int cid0, cid1, cid2;
			for (size_t j = 0; j < total; ++j)
			{
				auto it = m_list.find(ids[j]);
				if (it == m_list.end())
				{
					BranchPoint bp;
					bp.id = ids[j];
					bp.sum = 0;
					auto ret = m_list.insert(
						std::pair<unsigned int,
						BranchPoint>(bp.id, bp));
					it = ret.first;
				}
				for (size_t i = 0; i < gsize.gsxyz; ++i)
				{
					nc = sum[i*total + j];
					it->second.sum += nc;
					it->second.ctr += fluo::Point(
						csum[(total * i + j) * 3] + ox * nc,
						csum[(total * i + j) * 3 + 1] + oy * nc,
						csum[(total * i + j) * 3 + 2] + oz * nc);
					cid0 = cids[(i*total + j) * 3];
					cid1 = cids[(i*total + j) * 3 + 1];
					cid2 = cids[(i*total + j) * 3 + 2];
					if (cid0)
						it->second.cid.insert(cid0 & 0x7FFFFFFF);
					if (cid1)
						it->second.cid.insert(cid1 & 0x7FFFFFFF);
					if (cid2)
						it->second.cid.insert(cid2 & 0x7FFFFFFF);
				}
			}
		}

		//read back
		kernel_prog->copyArgBufToTex3D(arg_label, lid,
			sizeof(unsigned int)*nx*ny*nz, region);

		//release buffer
		kernel_prog->releaseAllArgs();

		bnum++;//brick number of processed
	}

	//connect bricks
	size_t idnum = m_list.size();
	std::vector<std::set<unsigned int>> merge_list;//ids in different bricks to be merged
	std::vector<std::set<unsigned int>> brick_pairs;//pairs processed don't need to process again
	while (bnum > 1 && idnum > 1)
	{
		flvr::Texture* tex = m_vd->GetTexture();
		if (!tex)
			break;
		kernel_prog = glbin_kernel_factory.kernel(str_cl_sg_check_borders, bits, max_int);
		if (!kernel_prog)
			break;
		kernel_0 = kernel_prog->createKernel("kernel_0");//x
		kernel_1 = kernel_prog->createKernel("kernel_1");//x
		kernel_2 = kernel_prog->createKernel("kernel_2");//x

		std::vector<unsigned int> ids;
		for (auto it = m_list.begin();
			it != m_list.end(); ++it)
			ids.push_back(it->second.id);

		for (size_t bi = 0; bi < brick_num; ++bi)
		{
			flvr::TextureBrick* b = (*bricks)[bi];
			if (!b->get_new_grown())
				continue;
			int nx = b->nx();
			int ny = b->ny();
			int nz = b->nz();
			GLint lid = m_vd->GetVR()->load_brick_label(b);
			unsigned bid;
			bid = b->get_id();
			kernel_prog->setKernelArgBegin(kernel_0);
			auto arg_tex =
				kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, lid);

			flvr::TextureBrick* nb;
			unsigned int nid;
			//+x
			nid = tex->posxid(bid);
			if (nid != bid)
				nb = tex->get_brick(nid);
			else
				nb = 0;
			if (nb &&
				nb->get_new_grown() &&
				!CheckBrickPair(bid, nid, brick_pairs))
				CheckBorders(nx - 1, 0, ny, nz, ids, nb,
				kernel_prog, kernel_0, arg_tex,
				brick_pairs, merge_list);
			//-x
			nid = tex->negxid(bid);
			if (nid != bid)
				nb = tex->get_brick(nid);
			else
				nb = 0;
			if (nb &&
				nb->get_new_grown() &&
				!CheckBrickPair(bid, nid, brick_pairs))
				CheckBorders(0, nb->nx()-1, ny, nz, ids, nb,
					kernel_prog, kernel_0, arg_tex,
					brick_pairs, merge_list);
			//+y
			nid = tex->posyid(bid);
			if (nid != bid)
				nb = tex->get_brick(nid);
			else
				nb = 0;
			if (nb &&
				nb->get_new_grown() &&
				!CheckBrickPair(bid, nid, brick_pairs))
				CheckBorders(ny - 1, 0, nx, nz, ids, nb,
					kernel_prog, kernel_1, arg_tex,
					brick_pairs, merge_list);
			//-y
			nid = tex->negyid(bid);
			if (nid != bid)
				nb = tex->get_brick(nid);
			else
				nb = 0;
			if (nb &&
				nb->get_new_grown() &&
				!CheckBrickPair(bid, nid, brick_pairs))
				CheckBorders(0, nb->ny() - 1, nx, nz, ids, nb,
					kernel_prog, kernel_1, arg_tex,
					brick_pairs, merge_list);
			//+z
			nid = tex->poszid(bid);
			if (nid != bid)
				nb = tex->get_brick(nid);
			else
				nb = 0;
			if (nb &&
				nb->get_new_grown() &&
				!CheckBrickPair(bid, nid, brick_pairs))
				CheckBorders(nz - 1, 0, nx, ny, ids, nb,
					kernel_prog, kernel_2, arg_tex,
					brick_pairs, merge_list);
			//-z
			nid = tex->negzid(bid);
			if (nid != bid)
				nb = tex->get_brick(nid);
			else
				nb = 0;
			if (nb &&
				nb->get_new_grown() &&
				!CheckBrickPair(bid, nid, brick_pairs))
				CheckBorders(0, nb->nz() - 1, nx, ny, ids, nb,
					kernel_prog, kernel_2, arg_tex,
					brick_pairs, merge_list);
		}

		//release buffer
		kernel_prog->releaseAllArgs();
		break;
	}

	//finalize bricks
	kernel_prog = glbin_kernel_factory.kernel(str_cl_segrow, bits, max_int);
	if (!kernel_prog)
		return;
	int kernel_7 = kernel_prog->createKernel("kernel_8");//finalize
	for (size_t bi = 0; bi < brick_num; ++bi)
	{
		flvr::TextureBrick* b = (*bricks)[bi];
		if (!b->is_mask_act())
			continue;
		int nx = b->nx();
		int ny = b->ny();
		int nz = b->nz();
		GLint lid = m_vd->GetVR()->load_brick_label(b);
		//compute workload
		size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };
		size_t local_size[3] = { 1, 1, 1 };
		size_t region[3] = { (size_t)nx, (size_t)ny, (size_t)nz };

		//finalize
		kernel_prog->setKernelArgBegin(kernel_7);
		auto arg_label =
			kernel_prog->copyTex3DToArgBuf(CL_MEM_READ_WRITE, lid, "arg_label", sizeof(unsigned int) * nx * ny * nz, region);
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nz));

		//execute
		kernel_prog->executeKernel(kernel_7, 3, global_size, local_size);

		//read back
		kernel_prog->copyArgBufToTex3D(arg_label, lid,
			sizeof(unsigned int)*nx*ny*nz, region);

		//release buffer
		kernel_prog->releaseAllArgs();
	}

	MergeIds(merge_list);

	//add ruler points
	double spcx, spcy, spcz;
	m_vd->GetSpacings(spcx, spcy, spcz);
	for (auto it = m_list.begin();
		it != m_list.end(); ++it)
	{
		if (static_cast<long long>(it->second.sum) <
			static_cast<long long>(m_sz_thresh))
			continue;
		it->second.ctr = it->second.ctr / it->second.sum;
		it->second.ctr.scale(spcx, spcy, spcz);
		m_handler->AddRulerPointAfterId(
			it->second.ctr,
			it->second.id,
			it->second.cid,
			it->second.bid);
	}
}

bool SegGrow::CheckBrickPair(unsigned int id1, unsigned int id2,
	std::vector<std::set<unsigned int>> &pairs)
{
	std::set<unsigned int> pair{ id1, id2 };
	if (std::find(pairs.begin(), pairs.end(), pair) ==
		pairs.end())
	{
		//add pair
		pairs.push_back(pair);
		return false;
	}
	else
		return true;
}

void SegGrow::CollectIds(std::vector<unsigned int> &ids,
	std::vector<unsigned int> &cids,
	std::vector<std::set<unsigned int>> &merge_list)
{
	for (size_t i = 0; i < cids.size(); ++i)
	{
		if (!cids[i])
			continue;
		unsigned int id0 = ids[i / 3];
		unsigned int id1 = cids[i];
		bool found = false;
		for (size_t j = 0; j < merge_list.size(); ++j)
		{
			if (merge_list[j].find(id0) != merge_list[j].end() ||
				merge_list[j].find(id1) != merge_list[j].end())
			{
				found = true;
				merge_list[j].insert(id0);
				merge_list[j].insert(id1);
				break;
			}
		}
		if (!found)
		{
			std::set<unsigned int> ids{ id0, id1 };
			merge_list.push_back(ids);
		}
	}
}

void SegGrow::MergeIds(std::vector<std::set<unsigned int>> &merge_list)
{
	if (merge_list.empty())
		return;

	std::unordered_map<unsigned int, BranchPoint> list = m_list;
	m_list.clear();
	for (size_t i = 0; i < merge_list.size(); ++i)
	{
		if (merge_list[i].size() < 2)
			continue;
		BranchPoint bp;
		bool first = true;
		for (auto it = merge_list[i].begin();
			it != merge_list[i].end(); ++it)
		{
			auto ibp = list.find(*it);
			if (ibp != list.end())
			{
				if (first)
				{
					bp.id = ibp->second.id;
					bp.sum = ibp->second.sum;
					bp.ctr = ibp->second.ctr;
					bp.cid = ibp->second.cid;
					bp.bid.insert(bp.id);
				}
				else
				{
					bp.sum += ibp->second.sum;
					bp.ctr += ibp->second.ctr;
					for (auto sit = ibp->second.cid.begin();
						sit != ibp->second.cid.end(); ++sit)
						bp.cid.insert(*sit);
					bp.bid.insert(ibp->second.id);
				}
				//remove from list
				list.erase(ibp);
			}
			first = false;
		}
		m_list.insert(
			std::pair<unsigned int,
			BranchPoint>(bp.id, bp));
	}
	for (auto it = list.begin();
		it != list.end(); ++it)
		m_list.insert(
			std::pair<unsigned int,
			BranchPoint>(it->second.id, it->second));
}

void SegGrow::CheckBorders(int d0, int d1, int n0, int n1,
	std::vector<unsigned int> &ids,
	flvr::TextureBrick* nb,
	flvr::KernelProgram *kernel_prog, int kernel,
	std::weak_ptr<flvr::Argument> arg_tex,
	std::vector<std::set<unsigned int>> &brick_pairs,
	std::vector<std::set<unsigned int>> &merge_list)
{
	GLint nlid;
	size_t global_size[2] = { 1, 1 };
	size_t local_size[2] = { 1, 1 };

	nlid = m_vd->GetVR()->load_brick_label(nb);
	//set
	//unsigned int d0 = nx - 1;
	//unsigned int d1 = 0;
	size_t idnum = ids.size();
	std::vector<unsigned int> cids(idnum * 3, 0);
	unsigned int* pcids = cids.data();
	kernel_prog->setKernelArgBegin(kernel);
	kernel_prog->setKernelArgument(arg_tex);
	kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, nlid);
	kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&d0));
	kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&d1));
	kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&idnum));
	kernel_prog->setKernelArgBuf(CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, "arg_ids", sizeof(unsigned int) * idnum, (void*)(ids.data()));
	auto arg_pcids =
		kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, "arg_pcids", sizeof(unsigned int) * idnum * 3, (void*)(pcids));

	//execute
	global_size[0] = n0; global_size[1] = n1;
	kernel_prog->executeKernel(kernel, 2, global_size, local_size);
	//read back
	kernel_prog->readBuffer(arg_pcids, pcids);

	CollectIds(ids, cids, merge_list);
}