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
#include <Compare.h>
#include <KernelProgram.h>
#include <Global.h>
#include <VolumeRenderer.h>
#include <TextureBrick.h>
#include <Texture.h>
#include <VolKernel.h>
#include <DataManager.h>
#include <algorithm>

using namespace flrd;

constexpr const char* str_cl_chann_threshold = R"CLKER(
const sampler_t samp =
	CLK_NORMALIZED_COORDS_FALSE|
	CLK_ADDRESS_CLAMP_TO_EDGE|
	CLK_FILTER_NEAREST;

__kernel void kernel_0(
	__read_only image3d_t chann1,
	__read_only image3d_t chann2,
	float ss1,
	float ss2,
	unsigned int ngx,
	unsigned int ngy,
	unsigned int ngz,
	unsigned int gsxy,
	unsigned int gsx,
	__global float* sum,
	float4 th)
{
	int3 gid = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);
	int3 ub = (int3)(lb.x + ngx, lb.y + ngy, lb.z + ngz);
	int4 ijk = (int4)(0, 0, 0, 1);
	float lsum = 0.0f;
	float v1, v2;
	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)
	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)
	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)
	{
		v1 = read_imagef(chann1, samp, ijk).x * ss1;
		v2 = read_imagef(chann2, samp, ijk).x * ss2;
		if (v1 > th.x && v1 <= th.y && v2 > th.z && v2 <= th.w)
			lsum += v1;
	}
	unsigned int index = gsxy * gid.z + gsx * gid.y + gid.x;
	atomic_xchg(sum+index, lsum);
}

//with mask
__kernel void kernel_1(
	__read_only image3d_t chann1,
	__read_only image3d_t chann2,
	float ss1,
	float ss2,
	unsigned int ngx,
	unsigned int ngy,
	unsigned int ngz,
	unsigned int gsxy,
	unsigned int gsx,
	__global float* sum,
	float4 th,
	__read_only image3d_t mask1,
	__read_only image3d_t mask2)
{
	int3 gid = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);
	int3 ub = (int3)(lb.x + ngx, lb.y + ngy, lb.z + ngz);
	int4 ijk = (int4)(0, 0, 0, 1);
	float lsum = 0.0f;
	float m, v1, v2;
	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)
	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)
	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)
	{
		m = read_imagef(mask1, samp, ijk).x;
		if (m < 1e-6) continue;
		m = read_imagef(mask2, samp, ijk).x;
		if (m < 1e-6) continue;
		v1 = read_imagef(chann1, samp, ijk).x * ss1;
		v2 = read_imagef(chann2, samp, ijk).x * ss2;
		if (v1 > th.x && v1 <= th.y && v2 > th.z && v2 <= th.w)
			lsum += v1;
	}
	unsigned int index = gsxy * gid.z + gsx * gid.y + gid.x;
	atomic_xchg(sum+index, lsum);
}
__kernel void kernel_2(
	__read_only image3d_t chann1,
	__read_only image3d_t chann2,
	float ss1,
	float ss2,
	unsigned int ngx,
	unsigned int ngy,
	unsigned int ngz,
	unsigned int gsxy,
	unsigned int gsx,
	__global float* sum,
	float4 th)
{
	int3 gid = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);
	int3 ub = (int3)(lb.x + ngx, lb.y + ngy, lb.z + ngz);
	int4 ijk = (int4)(0, 0, 0, 1);
	float lsum = 0.0f;
	float v1, v2;
	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)
	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)
	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)
	{
		v1 = read_imagef(chann1, samp, ijk).x * ss1;
		v2 = read_imagef(chann2, samp, ijk).x * ss2;
		if (v1 > th.x && v1 <= th.y && v2 > th.z && v2 <= th.w)
			lsum += 1.0f;
	}
	unsigned int index = gsxy * gid.z + gsx * gid.y + gid.x;
	atomic_xchg(sum+index, lsum);
}

//with mask
__kernel void kernel_3(
	__read_only image3d_t chann1,
	__read_only image3d_t chann2,
	float ss1,
	float ss2,
	unsigned int ngx,
	unsigned int ngy,
	unsigned int ngz,
	unsigned int gsxy,
	unsigned int gsx,
	__global float* sum,
	float4 th,
	__read_only image3d_t mask1,
	__read_only image3d_t mask2)
{
	int3 gid = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);
	int3 ub = (int3)(lb.x + ngx, lb.y + ngy, lb.z + ngz);
	int4 ijk = (int4)(0, 0, 0, 1);
	float lsum = 0.0f;
	float m, v1, v2;
	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)
	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)
	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)
	{
		m = read_imagef(mask1, samp, ijk).x;
		if (m < 1e-6) continue;
		m = read_imagef(mask2, samp, ijk).x;
		if (m < 1e-6) continue;
		v1 = read_imagef(chann1, samp, ijk).x * ss1;
		v2 = read_imagef(chann2, samp, ijk).x * ss2;
		if (v1 > th.x && v1 <= th.y && v2 > th.z && v2 <= th.w)
			lsum += 1.0f;
	}
	unsigned int index = gsxy * gid.z + gsx * gid.y + gid.x;
	atomic_xchg(sum+index, lsum);
}
)CLKER";

constexpr const char* str_cl_chann_dotprod = R"CLKER(
const sampler_t samp =
CLK_NORMALIZED_COORDS_FALSE |
CLK_ADDRESS_CLAMP |
CLK_FILTER_NEAREST;

__kernel void kernel_0(
	__read_only image3d_t chann1,
	__read_only image3d_t chann2,
	float ss1,
	float ss2,
	unsigned int ngx,
	unsigned int ngy,
	unsigned int ngz,
	unsigned int gsxy,
	unsigned int gsx,
	__global float* sum)
{
	int3 gid = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);
	int3 ub = (int3)(lb.x + ngx, lb.y + ngy, lb.z + ngz);
	int4 ijk = (int4)(0, 0, 0, 1);
	float lsum = 0.0f;
	float v1, v2;
	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)
	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)
	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)
	{
		v1 = read_imagef(chann1, samp, ijk).x * ss1;
		v2 = read_imagef(chann2, samp, ijk).x * ss2;
		lsum += v1 * v2;
	}
	unsigned int index = gsxy * gid.z + gsx * gid.y + gid.x;
	atomic_xchg(sum+index, lsum);
}
//product with mask
__kernel void kernel_1(
	__read_only image3d_t chann1,
	__read_only image3d_t chann2,
	float ss1,
	float ss2,
	unsigned int ngx,
	unsigned int ngy,
	unsigned int ngz,
	unsigned int gsxy,
	unsigned int gsx,
	__global float* sum,
	__read_only image3d_t mask1,
	__read_only image3d_t mask2)
{
	int3 gid = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);
	int3 ub = (int3)(lb.x + ngx, lb.y + ngy, lb.z + ngz);
	int4 ijk = (int4)(0, 0, 0, 1);
	float lsum = 0.0f;
	float m, v1, v2;
	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)
	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)
	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)
	{
		m = read_imagef(mask1, samp, ijk).x;
		if (m < 1e-6) continue;
		m = read_imagef(mask2, samp, ijk).x;
		if (m < 1e-6) continue;
		v1 = read_imagef(chann1, samp, ijk).x * ss1;
		v2 = read_imagef(chann2, samp, ijk).x * ss2;
		lsum += v1 * v2;
	}
	unsigned int index = gsxy * gid.z + gsx * gid.y + gid.x;
	atomic_xchg(sum+index, lsum);
}

//count voxels
__kernel void kernel_2(
	__read_only image3d_t chann1,
	__read_only image3d_t chann2,
	float ss1,
	float ss2,
	unsigned int ngx,
	unsigned int ngy,
	unsigned int ngz,
	unsigned int gsxy,
	unsigned int gsx,
	__global float* sum)
{
	int3 gid = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);
	int3 ub = (int3)(lb.x + ngx, lb.y + ngy, lb.z + ngz);
	int4 ijk = (int4)(0, 0, 0, 1);
	float lsum = 0.0f;
	float v1, v2;
	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)
	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)
	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)
	{
		v1 = read_imagef(chann1, samp, ijk).x * ss1;
		v2 = read_imagef(chann2, samp, ijk).x * ss2;
		if (v1 * v2 > 0.0f)
			lsum += 1.0f;
	}
	unsigned int index = gsxy * gid.z + gsx * gid.y + gid.x;
	atomic_xchg(sum+index, lsum);
}
//product with mask
__kernel void kernel_3(
	__read_only image3d_t chann1,
	__read_only image3d_t chann2,
	float ss1,
	float ss2,
	unsigned int ngx,
	unsigned int ngy,
	unsigned int ngz,
	unsigned int gsxy,
	unsigned int gsx,
	__global float* sum,
	__read_only image3d_t mask1,
	__read_only image3d_t mask2)
{
	int3 gid = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);
	int3 ub = (int3)(lb.x + ngx, lb.y + ngy, lb.z + ngz);
	int4 ijk = (int4)(0, 0, 0, 1);
	float lsum = 0.0f;
	float m, v1, v2;
	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)
	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)
	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)
	{
		m = read_imagef(mask1, samp, ijk).x;
		if (m < 1e-6) continue;
		m = read_imagef(mask2, samp, ijk).x;
		if (m < 1e-6) continue;
		v1 = read_imagef(chann1, samp, ijk).x * ss1;
		v2 = read_imagef(chann2, samp, ijk).x * ss2;
		if (v1 * v2 > 0.0f)
			lsum += 1.0f;
	}
	unsigned int index = gsxy * gid.z + gsx * gid.y + gid.x;
	atomic_xchg(sum+index, lsum);
}
)CLKER";

constexpr const char* str_cl_chann_minvalue = R"CLKER(
const sampler_t samp =
CLK_NORMALIZED_COORDS_FALSE |
CLK_ADDRESS_CLAMP |
CLK_FILTER_NEAREST;

__kernel void kernel_0(
	__read_only image3d_t chann1,
	__read_only image3d_t chann2,
	float ss1,
	float ss2,
	unsigned int ngx,
	unsigned int ngy,
	unsigned int ngz,
	unsigned int gsxy,
	unsigned int gsx,
	__global float* sum)
{
	int3 gid = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);
	int3 ub = (int3)(lb.x + ngx, lb.y + ngy, lb.z + ngz);
	int4 ijk = (int4)(0, 0, 0, 1);
	float lsum = 0.0f;
	float v1, v2;
	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)
	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)
	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)
	{
		v1 = read_imagef(chann1, samp, ijk).x * ss1;
		v2 = read_imagef(chann2, samp, ijk).x * ss2;
		lsum += min(v1, v2);
	}
	unsigned int index = gsxy * gid.z + gsx * gid.y + gid.x;
	atomic_xchg(sum+index, lsum);
}
//product with mask
__kernel void kernel_1(
	__read_only image3d_t chann1,
	__read_only image3d_t chann2,
	float ss1,
	float ss2,
	unsigned int ngx,
	unsigned int ngy,
	unsigned int ngz,
	unsigned int gsxy,
	unsigned int gsx,
	__global float* sum,
	__read_only image3d_t mask1,
	__read_only image3d_t mask2)
{
	int3 gid = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);
	int3 ub = (int3)(lb.x + ngx, lb.y + ngy, lb.z + ngz);
	int4 ijk = (int4)(0, 0, 0, 1);
	float lsum = 0.0f;
	float m, v1, v2;
	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)
	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)
	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)
	{
		m = read_imagef(mask1, samp, ijk).x;
		if (m < 1e-6) continue;
		m = read_imagef(mask2, samp, ijk).x;
		if (m < 1e-6) continue;
		v1 = read_imagef(chann1, samp, ijk).x * ss1;
		v2 = read_imagef(chann2, samp, ijk).x * ss2;
		lsum += min(v1, v2);
	}
	unsigned int index = gsxy * gid.z + gsx * gid.y + gid.x;
	atomic_xchg(sum+index, lsum);
}

//count voxels
__kernel void kernel_2(
	__read_only image3d_t chann1,
	__read_only image3d_t chann2,
	float ss1,
	float ss2,
	unsigned int ngx,
	unsigned int ngy,
	unsigned int ngz,
	unsigned int gsxy,
	unsigned int gsx,
	__global float* sum)
{
	int3 gid = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);
	int3 ub = (int3)(lb.x + ngx, lb.y + ngy, lb.z + ngz);
	int4 ijk = (int4)(0, 0, 0, 1);
	float lsum = 0.0f;
	float v1, v2;
	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)
	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)
	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)
	{
		v1 = read_imagef(chann1, samp, ijk).x * ss1;
		v2 = read_imagef(chann2, samp, ijk).x * ss2;
		if (min(v1, v2) > 0.0f)
			lsum += 1.0f;
	}
	unsigned int index = gsxy * gid.z + gsx * gid.y + gid.x;
	atomic_xchg(sum+index, lsum);
}
//product with mask
__kernel void kernel_3(
	__read_only image3d_t chann1,
	__read_only image3d_t chann2,
	float ss1,
	float ss2,
	unsigned int ngx,
	unsigned int ngy,
	unsigned int ngz,
	unsigned int gsxy,
	unsigned int gsx,
	__global float* sum,
	__read_only image3d_t mask1,
	__read_only image3d_t mask2)
{
	int3 gid = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);
	int3 ub = (int3)(lb.x + ngx, lb.y + ngy, lb.z + ngz);
	int4 ijk = (int4)(0, 0, 0, 1);
	float lsum = 0.0f;
	float m, v1, v2;
	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)
	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)
	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)
	{
		m = read_imagef(mask1, samp, ijk).x;
		if (m < 1e-6) continue;
		m = read_imagef(mask2, samp, ijk).x;
		if (m < 1e-6) continue;
		v1 = read_imagef(chann1, samp, ijk).x * ss1;
		v2 = read_imagef(chann2, samp, ijk).x * ss2;
		if (min(v1, v2) > 0.0f)
			lsum += 1.0f;
	}
	unsigned int index = gsxy * gid.z + gsx * gid.y + gid.x;
	atomic_xchg(sum+index, lsum);
}
)CLKER";

constexpr const char* str_cl_chann_sum = R"CLKER(
const sampler_t samp =
CLK_NORMALIZED_COORDS_FALSE |
CLK_ADDRESS_CLAMP |
CLK_FILTER_NEAREST;

__kernel void kernel_0(
	__read_only image3d_t chann1,
	__read_only image3d_t chann2,
	float ss1,
	float ss2,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz,
	__global float* sum)
{
	int4 ijk = (int4)(get_global_id(0),
		get_global_id(1), get_global_id(2), 1);
	unsigned int index = nx*ny*ijk.z + nx*ijk.y + ijk.x;
	float value1 = read_imagef(chann1, samp, ijk).x * ss1;
	float value2 = read_imagef(chann2, samp, ijk).x * ss2;
	sum[index] += value1 + value2;
}

__kernel void kernel_1(
	__read_only image3d_t chann1,
	__read_only image3d_t chann2,
	float ss1,
	float ss2,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz,
	__global float* sum,
	float w1,
	float w2)
{
	int4 ijk = (int4)(get_global_id(0),
		get_global_id(1), get_global_id(2), 1);
	unsigned int index = nx*ny*ijk.z + nx*ijk.y + ijk.x;
	float value1 = read_imagef(chann1, samp, ijk).x * ss1;
	float value2 = read_imagef(chann2, samp, ijk).x * ss2;
	sum[index] += value1 * w1 + value2 * w2;
};
)CLKER";

ChannelCompare::ChannelCompare(VolumeData* vd1, VolumeData* vd2)
	: m_vd1(vd1), m_vd2(vd2),
	m_use_mask(false),
	m_int_weighted(false),
	m_init(false),
	prework(nullptr),
	postwork(nullptr)
{
}

ChannelCompare::~ChannelCompare()
{
}

bool ChannelCompare::CheckBricks()
{
	if (!m_vd1)
		return false;
	if (!m_vd2)
		return false;
	if (!m_vd1->GetTexture())
		return false;
	if (!m_vd2->GetTexture())
		return false;
	int brick_num1 = m_vd1->GetTexture()->get_brick_num();
	int brick_num2 = m_vd2->GetTexture()->get_brick_num();
	if (!brick_num1 || !brick_num2 || brick_num1 != brick_num2)
		return false;
	return true;
}

bool ChannelCompare::GetInfo(
	flvr::TextureBrick* b1, flvr::TextureBrick* b2,
	long &bits1, long &bits2,
	long &nx, long &ny, long &nz)
{
	bits1 = b1->nb(0)*8;
	bits2 = b2->nb(0)*8;
	long nx1 = b1->nx();
	long nx2 = b2->nx();
	long ny1 = b1->ny();
	long ny2 = b2->ny();
	long nz1 = b1->nz();
	long nz2 = b2->nz();
	if (nx1 != nx2 || ny1 != ny2 || nz1 != nz2)
		return false;
	nx = nx1; ny = ny1; nz = nz1;
	return true;
}

void* ChannelCompare::GetVolDataBrick(flvr::TextureBrick* b)
{
	if (!b)
		return 0;

	size_t nx, ny, nz;
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
	for (size_t k = 0; k < nz; ++k)
	{
		tp2 = tp;
		for (size_t j = 0; j < ny; ++j)
		{
			memcpy(tempp, tp2, nx*nb);
			tempp += nx * nb;
			tp2 += b->sx()*nb;
		}
		tp += b->sx()*b->sy()*nb;
	}
	return (void*)temp;
}

void* ChannelCompare::GetVolData(VolumeData* vd)
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

void ChannelCompare::Product()
{
	m_result = 0.0;

	if (!CheckBricks())
		return;
	long bits = m_vd1->GetBits();
	float max_int = static_cast<float>(m_vd1->GetMaxValue());

	//create program and kernels
	flvr::KernelProgram* kernel_prog = glbin_vol_kernel_factory.kernel(str_cl_chann_dotprod, bits, max_int);
	if (!kernel_prog)
		return;
	int kernel_index = -1;
	std::string name = "kernel_0";
	if (m_use_mask)
	{
		if (m_int_weighted)
			name = "kernel_1";
		else
			name = "kernel_3";
	}
	else
	{
		if (!m_int_weighted)
			name = "kernel_2";
	}
	if (kernel_prog->valid())
	{
		kernel_index = kernel_prog->findKernel(name);
		if (kernel_index == -1)
			kernel_index = kernel_prog->createKernel(name);
	}
	else
		kernel_index = kernel_prog->createKernel(name);

	size_t brick_num = m_vd1->GetTexture()->get_brick_num();
	std::vector<flvr::TextureBrick*> *bricks1 = m_vd1->GetTexture()->get_bricks();
	std::vector<flvr::TextureBrick*> *bricks2 = m_vd2->GetTexture()->get_bricks();
	float ss1 = (float)(m_vd1->GetScalarScale());
	float ss2 = (float)(m_vd2->GetScalarScale());

	for (size_t i = 0; i < brick_num; ++i)
	{
		flvr::TextureBrick* b1 = (*bricks1)[i];
		flvr::TextureBrick* b2 = (*bricks2)[i];
		if (m_use_mask)
		{
			if (!b1->is_mask_valid() ||
				!b2->is_mask_valid())
				continue;
		}
		long nx, ny, nz, bits1, bits2;
		if (!GetInfo(b1, b2, bits1, bits2, nx, ny, nz))
			continue;
		//get tex ids
		GLint tid1 = m_vd1->GetVR()->load_brick(b1);
		GLint tid2 = m_vd2->GetVR()->load_brick(b2);
		GLint mid1, mid2;
		if (m_use_mask)
		{
			mid1 = m_vd1->GetVR()->load_brick_mask(b1);
			mid2 = m_vd2->GetVR()->load_brick_mask(b2);
			if (mid1 < 0 || mid2 < 0)
				continue;
		}

		//compute workload
		flvr::GroupSize gsize;
		kernel_prog->get_group_size(kernel_index, nx, ny, nz, gsize);

		size_t local_size[3] = { 1, 1, 1 };
		size_t global_size[3] = {
			size_t(gsize.gsx), size_t(gsize.gsy), size_t(gsize.gsz) };

		//set
		//unsigned int count = 0;
		float *sum = new float[gsize.gsxyz];
		kernel_prog->setKernelArgBegin(kernel_index);
		kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, tid1);
		kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, tid2);
		kernel_prog->setKernelArgConst(sizeof(float), (void*)(&ss1));
		kernel_prog->setKernelArgConst(sizeof(float), (void*)(&ss2));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngx));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngy));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngz));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.gsxy));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.gsx));
		kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(float)*(gsize.gsxyz), (void*)(sum));
		if (m_use_mask)
		{
			kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, mid1);
			kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, mid2);
		}

		//execute
		kernel_prog->executeKernel(kernel_index, 3, global_size, 0/*local_size*/);
		//read back
		kernel_prog->readBuffer(sizeof(float)*(gsize.gsxyz), sum, sum);

		//release buffer
		kernel_prog->releaseAll();

		//sum
		for (size_t i=0; i< gsize.gsxyz; ++i)
			m_result += sum[i];
		delete[] sum;
	}
}

void ChannelCompare::MinValue()
{
	m_result = 0.0;

	if (!CheckBricks())
		return;
	long bits = m_vd1->GetBits();
	float max_int = static_cast<float>(m_vd1->GetMaxValue());

	//create program and kernels
	flvr::KernelProgram* kernel_prog = glbin_vol_kernel_factory.kernel(str_cl_chann_minvalue, bits, max_int);
	if (!kernel_prog)
		return;
	int kernel_index = -1;
	std::string name = "kernel_0";
	if (m_use_mask)
	{
		if (m_int_weighted)
			name = "kernel_1";
		else
			name = "kernel_3";
	}
	else
	{
		if (!m_int_weighted)
			name = "kernel_2";
	}
	if (kernel_prog->valid())
	{
		kernel_index = kernel_prog->findKernel(name);
		if (kernel_index == -1)
			kernel_index = kernel_prog->createKernel(name);
	}
	else
		kernel_index = kernel_prog->createKernel(name);

	size_t brick_num = m_vd1->GetTexture()->get_brick_num();
	std::vector<flvr::TextureBrick*> *bricks1 = m_vd1->GetTexture()->get_bricks();
	std::vector<flvr::TextureBrick*> *bricks2 = m_vd2->GetTexture()->get_bricks();
	float ss1 = (float)(m_vd1->GetScalarScale());
	float ss2 = (float)(m_vd2->GetScalarScale());

	for (size_t i = 0; i < brick_num; ++i)
	{
		flvr::TextureBrick* b1 = (*bricks1)[i];
		flvr::TextureBrick* b2 = (*bricks2)[i];
		if (m_use_mask)
		{
			if (!b1->is_mask_valid() ||
				!b2->is_mask_valid())
				continue;
		}
		long nx, ny, nz, bits1, bits2;
		if (!GetInfo(b1, b2, bits1, bits2, nx, ny, nz))
			continue;
		//get tex ids
		GLint tid1 = m_vd1->GetVR()->load_brick(b1);
		GLint tid2 = m_vd2->GetVR()->load_brick(b2);
		GLint mid1, mid2;
		if (m_use_mask)
		{
			mid1 = m_vd1->GetVR()->load_brick_mask(b1);
			mid2 = m_vd2->GetVR()->load_brick_mask(b2);
			if (mid1 < 0 || mid2 < 0)
				continue;
		}

		//compute workload
		flvr::GroupSize gsize;
		kernel_prog->get_group_size(kernel_index, nx, ny, nz, gsize);

		size_t local_size[3] = { 1, 1, 1 };
		size_t global_size[3] = {
			size_t(gsize.gsx), size_t(gsize.gsy), size_t(gsize.gsz) };

		//set
		//unsigned int count = 0;
		float *sum = new float[gsize.gsxyz];
		kernel_prog->setKernelArgBegin(kernel_index);
		kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, tid1);
		kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, tid2);
		kernel_prog->setKernelArgConst(sizeof(float), (void*)(&ss1));
		kernel_prog->setKernelArgConst(sizeof(float), (void*)(&ss2));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngx));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngy));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngz));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.gsxy));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.gsx));
		kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(float)*(gsize.gsxyz), (void*)(sum));
		if (m_use_mask)
		{
			kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, mid1);
			kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, mid2);
		}

		//execute
		kernel_prog->executeKernel(kernel_index, 3, global_size, 0/*local_size*/);
		//read back
		kernel_prog->readBuffer(sizeof(float)*(gsize.gsxyz), sum, sum);

		//release buffer
		kernel_prog->releaseAll();

		//sum
		for (size_t i = 0; i < gsize.gsxyz; ++i)
			m_result += sum[i];
		delete[] sum;
	}
}

void ChannelCompare::Threshold(float th1, float th2, float th3, float th4)
{
	m_result = 0.0;

	if (!CheckBricks())
		return;
	long bits = m_vd1->GetBits();
	float max_int = static_cast<float>(m_vd1->GetMaxValue());

	//create program and kernels
	flvr::KernelProgram* kernel_prog = glbin_vol_kernel_factory.kernel(str_cl_chann_threshold, bits, max_int);
	if (!kernel_prog)
		return;
	int kernel_index = -1;
	std::string name = "kernel_0";
	if (m_use_mask)
	{
		if (m_int_weighted)
			name = "kernel_1";
		else
			name = "kernel_3";
	}
	else
	{
		if (!m_int_weighted)
			name = "kernel_2";
	}
	if (kernel_prog->valid())
	{
		kernel_index = kernel_prog->findKernel(name);
		if (kernel_index == -1)
			kernel_index = kernel_prog->createKernel(name);
	}
	else
		kernel_index = kernel_prog->createKernel(name);

	size_t brick_num = m_vd1->GetTexture()->get_brick_num();
	std::vector<flvr::TextureBrick*> *bricks1 = m_vd1->GetTexture()->get_bricks();
	std::vector<flvr::TextureBrick*> *bricks2 = m_vd2->GetTexture()->get_bricks();
	float ss1 = (float)(m_vd1->GetScalarScale());
	float ss2 = (float)(m_vd2->GetScalarScale());

	for (size_t i = 0; i < brick_num; ++i)
	{
		if (prework) prework("");

		flvr::TextureBrick* b1 = (*bricks1)[i];
		flvr::TextureBrick* b2 = (*bricks2)[i];
		if (m_use_mask)
		{
			if (!b1->is_mask_valid() ||
				!b2->is_mask_valid())
				continue;
		}
		long nx, ny, nz, bits1, bits2;
		if (!GetInfo(b1, b2, bits1, bits2, nx, ny, nz))
			continue;
		//get tex ids
		GLint tid1 = m_vd1->GetVR()->load_brick(b1);
		GLint tid2 = m_vd2->GetVR()->load_brick(b2);
		GLint mid1, mid2;
		if (m_use_mask)
		{
			mid1 = m_vd1->GetVR()->load_brick_mask(b1);
			mid2 = m_vd2->GetVR()->load_brick_mask(b2);
			if (mid1 < 0 || mid2 < 0)
				continue;
		}

		//compute workload
		flvr::GroupSize gsize;
		kernel_prog->get_group_size(kernel_index, nx, ny, nz, gsize);

		size_t local_size[3] = { 1, 1, 1 };
		size_t global_size[3] = {
			size_t(gsize.gsx), size_t(gsize.gsy), size_t(gsize.gsz) };

		//set
		//unsigned int count = 0;
		float *sum = new float[gsize.gsxyz];
		cl_float4 th = {th1, th2, th3, th4};
		kernel_prog->setKernelArgBegin(kernel_index);
		kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, tid1);
		kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, tid2);
		kernel_prog->setKernelArgConst(sizeof(float), (void*)(&ss1));
		kernel_prog->setKernelArgConst(sizeof(float), (void*)(&ss2));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngx));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngy));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngz));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.gsxy));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.gsx));
		kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(float)*(gsize.gsxyz), (void*)(sum));
		kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(&th));
		if (m_use_mask)
		{
			kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, mid1);
			kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, mid2);
		}

		//execute
		kernel_prog->executeKernel(kernel_index, 3, global_size, 0/*local_size*/);
		//read back
		kernel_prog->readBuffer(sizeof(float)*(gsize.gsxyz), sum, sum);

		//release buffer
		kernel_prog->releaseAll();

		//sum
		for (size_t i = 0; i < gsize.gsxyz; ++i)
			m_result += sum[i];
		delete[] sum;

		if (postwork) postwork(__FUNCTION__);
	}
}

void ChannelCompare::Average(float weight, flvr::Argument& avg)
{
	m_result = 0.0;

	if (!CheckBricks())
		return;
	long bits = m_vd1->GetBits();
	float max_int = static_cast<float>(m_vd1->GetMaxValue());

	//create program and kernels
	flvr::KernelProgram* kernel_prog = glbin_vol_kernel_factory.kernel(str_cl_chann_sum, bits, max_int);
	if (!kernel_prog)
		return;
	int kernel_index = -1;
	std::string name = "kernel_0";
	if (weight > 0.0)
		name = "kernel_1";
	if (kernel_prog->valid())
		kernel_index = kernel_prog->findKernel(name);
	else
		kernel_index = kernel_prog->createKernel(name);

	size_t brick_num = m_vd1->GetTexture()->get_brick_num();
	std::vector<flvr::TextureBrick*> *bricks1 = m_vd1->GetTexture()->get_bricks();
	std::vector<flvr::TextureBrick*> *bricks2 = m_vd2->GetTexture()->get_bricks();
	float ss1 = (float)(m_vd1->GetScalarScale());
	float ss2 = (float)(m_vd2->GetScalarScale());

	for (size_t i = 0; i < brick_num; ++i)
	{
		if (prework) prework("");

		flvr::TextureBrick* b1 = (*bricks1)[i];
		flvr::TextureBrick* b2 = (*bricks2)[i];
		if (m_use_mask)
		{
			if (!b1->is_mask_valid() ||
				!b2->is_mask_valid())
				continue;
		}
		long nx, ny, nz, bits1, bits2;
		if (!GetInfo(b1, b2, bits1, bits2, nx, ny, nz))
			continue;
		//get tex ids
		GLint tid1 = m_vd1->GetVR()->load_brick(b1);
		GLint tid2 = m_vd2->GetVR()->load_brick(b2);

		size_t local_size[3] = { 1, 1, 1 };
		size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };

		//set
		//unsigned int count = 0;
		float* sum = 0;
		unsigned int nxyz = nx * ny * nz;
		kernel_prog->setKernelArgBegin(kernel_index);
		kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, tid1);
		kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, tid2);
		kernel_prog->setKernelArgConst(sizeof(float), (void*)(&ss1));
		kernel_prog->setKernelArgConst(sizeof(float), (void*)(&ss2));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nz));
		if (!avg.buffer)
		{
			sum = new float[nxyz]();
			kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(float)*(nxyz), (void*)(sum));
		}
		else
		{
			avg.kernel(kernel_index);
			kernel_prog->setKernelArgument(avg);
		}

		//execute
		kernel_prog->executeKernel(kernel_index, 3, global_size, 0/*local_size*/);

		//release buffer
		kernel_prog->releaseMemObject(kernel_index, 0, 0, tid1);
		kernel_prog->releaseMemObject(kernel_index, 1, 0, tid2);

		if (postwork) postwork(__FUNCTION__);
	}
}
