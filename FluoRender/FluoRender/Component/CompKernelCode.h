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
#ifndef COMP_CL_CODE_H
#define COMP_CL_CODE_H

inline constexpr const char* str_cl_shuffle_id_3d = R"CLKER(
const sampler_t samp =
	CLK_NORMALIZED_COORDS_FALSE|
	CLK_ADDRESS_CLAMP_TO_EDGE|
	CLK_FILTER_NEAREST;

unsigned int __attribute((always_inline)) reverse_bit(unsigned int val, unsigned int len)
{
	unsigned int res = val;
	int s = len - 1;
	for (val >>= 1; val; val >>= 1)
	{
		res <<= 1;
		res |= val & 1;
		s--;
	}
	res <<= s;
	res <<= 32-len;
	res >>= 32-len;
	return res;
}

//use clipping planes, no mask
__kernel void kernel_0(
	__read_only image3d_t data,
	__global unsigned int* label,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz,
	unsigned int lenx,
	unsigned int lenz,
	float4 p0,
	float4 p1,
	float4 p2,
	float4 p3,
	float4 p4,
	float4 p5,
	float3 scl,
	float3 trl)
{
	unsigned int res;
	unsigned int x, y, z, ii;
	unsigned int i = (unsigned int)(get_global_id(0));
	unsigned int j = (unsigned int)(get_global_id(1));
	unsigned int k = (unsigned int)(get_global_id(2));
	unsigned int index = nx*ny*k + nx*j + i;
	float3 pt = (float3)((float)(i) / (float)(nx), (float)(j) / (float)(ny), (float)(k) / (float)(nz));
	pt = pt * scl + trl;
	if (dot(pt, p0.xyz)+p0.w > 0.0f ||
		dot(pt, p1.xyz)+p1.w > 0.0f ||
		dot(pt, p2.xyz)+p2.w > 0.0f ||
		dot(pt, p3.xyz)+p3.w > 0.0f ||
		dot(pt, p4.xyz)+p4.w > 0.0f ||
		dot(pt, p5.xyz)+p5.w > 0.0f)
	{
		label[index] = 0;
		return;
	}
	float value = read_imagef(data, samp, (int4)(i, j, k, 1)).x;
	if (value < 1e-4f)
		label[index] = 0;
	else if (i<1 || i>nx-2 ||
			j<1 || j>ny-2)
		label[index] = 0;
	else
	{
		x = reverse_bit(i, lenx);
		y = reverse_bit(j, lenx);
		z = reverse_bit(k, lenz);
		res = 0;
		for (ii=0; ii<lenx; ++ii)
		{
			res |= (1<<ii & x)<<(ii);
			res |= (1<<ii & y)<<(ii+1);
		}
		res |= z<<lenx*2;
		label[index] = res + 1;
	}
}
//use clipping planes, use mask
__kernel void kernel_1(
	__read_only image3d_t data,
	__global unsigned int* label,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz,
	unsigned int lenx,
	unsigned int lenz,
	float4 p0,
	float4 p1,
	float4 p2,
	float4 p3,
	float4 p4,
	float4 p5,
	float3 scl,
	float3 trl,
	__read_only image3d_t mask)
{
	unsigned int res;
	unsigned int x, y, z, ii;
	unsigned int i = (unsigned int)(get_global_id(0));
	unsigned int j = (unsigned int)(get_global_id(1));
	unsigned int k = (unsigned int)(get_global_id(2));
	float mask_value = read_imagef(mask, samp, (int4)(i, j, k, 1)).x;
	if (mask_value < 1e-6f)
		return;
	unsigned int index = nx*ny*k + nx*j + i;
	float3 pt = (float3)((float)(i) / (float)(nx), (float)(j) / (float)(ny), (float)(k) / (float)(nz));
	pt = pt * scl + trl;
	if (dot(pt, p0.xyz)+p0.w > 0.0f ||
		dot(pt, p1.xyz)+p1.w > 0.0f ||
		dot(pt, p2.xyz)+p2.w > 0.0f ||
		dot(pt, p3.xyz)+p3.w > 0.0f ||
		dot(pt, p4.xyz)+p4.w > 0.0f ||
		dot(pt, p5.xyz)+p5.w > 0.0f)
	{
		label[index] = 0;
		return;
	}
	float value = read_imagef(data, samp, (int4)(i, j, k, 1)).x;
	if (value < 1e-4f)
		label[index] = 0;
	else if (i<1 || i>nx-2 ||
			j<1 || j>ny-2)
		label[index] = 0;
	else
	{
		x = reverse_bit(i, lenx);
		y = reverse_bit(j, lenx);
		z = reverse_bit(k, lenz);
		res = 0;
		for (ii=0; ii<lenx; ++ii)
		{
			res |= (1<<ii & x)<<(ii);
			res |= (1<<ii & y)<<(ii+1);
		}
		res |= z<<lenx*2;
		label[index] = res + 1;
	}
}
//not used
__kernel void kernel_null(
	__read_only image3d_t data,
	__global unsigned int* label,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz,
	unsigned int lenx,
	unsigned int lenz)
{
	unsigned int res;
	unsigned int x, y, z, ii;
	unsigned int i = (unsigned int)(get_global_id(0));
	unsigned int j = (unsigned int)(get_global_id(1));
	unsigned int k = (unsigned int)(get_global_id(2));
	unsigned int index = nx*ny*k + nx*j + i;
	float value = read_imagef(data, samp, (int4)(i, j, k, 1)).x;
	if (value < 1e-4f)
		label[index] = 0;
	else if (i<1 || i>nx-2 ||
			j<1 || j>ny-2)
		label[index] = 0;
	else
	{
		x = reverse_bit(i, lenx);
		y = reverse_bit(j, lenx);
		z = reverse_bit(k, lenz);
		res = 0;
		for (ii=0; ii<lenx; ++ii)
		{
			res |= (1<<ii & x)<<(ii);
			res |= (1<<ii & y)<<(ii+1);
		}
		res |= z<<lenx*2;
		label[index] = res + 1;
	}
}
)CLKER";

inline constexpr const char* str_cl_set_bit_3d = R"CLKER(
const sampler_t samp =
	CLK_NORMALIZED_COORDS_FALSE|
	CLK_ADDRESS_CLAMP_TO_EDGE|
	CLK_FILTER_NEAREST;

unsigned int __attribute((always_inline)) reverse_bit(unsigned int val, unsigned int len)
{
	unsigned int res = val;
	int s = len - 1;
	for (val >>= 1; val; val >>= 1)
	{
		res <<= 1;
		res |= val & 1;
		s--;
	}
	res <<= s;
	res <<= 32-len;
	res >>= 32-len;
	return res;
}
//increase count in size buffer
__kernel void kernel_0(
	__global unsigned int* szbuf,
	__global unsigned int* label,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz,
	unsigned int lenx,
	unsigned int lenz)
{
	unsigned int i = (unsigned int)(get_global_id(0));
	unsigned int j = (unsigned int)(get_global_id(1));
	unsigned int k = (unsigned int)(get_global_id(2));
	unsigned int index = nx*ny*k + nx*j + i;
	unsigned int value_l = label[index];
	if (value_l == 0)
		return;
	unsigned int res = value_l - 1;
	unsigned int x = 0;
	unsigned int y = 0;
	unsigned int z = 0;
	unsigned int ii;
	for (ii=0; ii<lenx; ++ii)
	{
		x |= (1<<(2*ii) & res)>>(ii);
		y |= (1<<(2*ii+1) & res)>>(ii+1);
	}
	z = res<<(32-lenx*2-lenz)>>(32-lenz);
	x = reverse_bit(x, lenx);
	y = reverse_bit(y, lenx);
	z = reverse_bit(z, lenz);
	index = nx*ny*z + nx*y + x;
	atomic_inc(szbuf+index);
}
//find max size for each component
__kernel void kernel_1(
	__global unsigned int* szbuf,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz,
	__global unsigned int* maxv)
{
	unsigned int i = (unsigned int)(get_global_id(0));
	unsigned int j = (unsigned int)(get_global_id(1));
	unsigned int k = (unsigned int)(get_global_id(2));
	unsigned int index = nx*ny*k + nx*j + i;
	*maxv = max(szbuf[index], *maxv);
}
//set all size to max
__kernel void kernel_2(
	__global unsigned int* szbuf,
	__global unsigned int* label,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz,
	unsigned int lenx,
	unsigned int lenz)
{
	unsigned int i = (unsigned int)(get_global_id(0));
	unsigned int j = (unsigned int)(get_global_id(1));
	unsigned int k = (unsigned int)(get_global_id(2));
	unsigned int index = nx*ny*k + nx*j + i;
	unsigned int value_l = label[index];
	if (value_l == 0)
		return;
	unsigned int res = value_l - 1;
	unsigned int x = 0;
	unsigned int y = 0;
	unsigned int z = 0;
	unsigned int ii;
	for (ii=0; ii<lenx; ++ii)
	{
		x |= (1<<(2*ii) & res)>>(ii);
		y |= (1<<(2*ii+1) & res)>>(ii+1);
	}
	z = res<<(32-lenx*2-lenz)>>(32-lenz);
	x = reverse_bit(x, lenx);
	y = reverse_bit(y, lenx);
	z = reverse_bit(z, lenz);
	unsigned int index2 = nx*ny*z + nx*y + x;
	if (index != index2)
		szbuf[index] = szbuf[index2];
}
//fix id by size
__kernel void kernel_3(
	__global unsigned int* szbuf,
	__global unsigned int* label,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz,
	unsigned int limit)
{
	unsigned int i = (unsigned int)(get_global_id(0));
	unsigned int j = (unsigned int)(get_global_id(1));
	unsigned int k = (unsigned int)(get_global_id(2));
	unsigned int index = nx*ny*k + nx*j + i;
	//break if too small
	if (label[index]==0 ||
		szbuf[index] < limit)
		return;
	label[index] = label[index] | 0x80000000;
}
//increase count in size buffer
__kernel void kernel_4(
	__global unsigned int* szbuf,
	__global unsigned int* label,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz,
	unsigned int lenx,
	unsigned int lenz,
	__read_only image3d_t mask)
{
	unsigned int i = (unsigned int)(get_global_id(0));
	unsigned int j = (unsigned int)(get_global_id(1));
	unsigned int k = (unsigned int)(get_global_id(2));
	float mask_value = read_imagef(mask, samp, (int4)(i, j, k, 1)).x;
	if (mask_value < 1e-6f)
		return;
	unsigned int index = nx*ny*k + nx*j + i;
	unsigned int value_l = label[index];
	if (value_l == 0)
		return;
	unsigned int res = value_l - 1;
	unsigned int x = 0;
	unsigned int y = 0;
	unsigned int z = 0;
	unsigned int ii;
	for (ii=0; ii<lenx; ++ii)
	{
		x |= (1<<(2*ii) & res)>>(ii);
		y |= (1<<(2*ii+1) & res)>>(ii+1);
	}
	z = res<<(32-lenx*2-lenz)>>(32-lenz);
	x = reverse_bit(x, lenx);
	y = reverse_bit(y, lenx);
	z = reverse_bit(z, lenz);
	index = nx*ny*z + nx*y + x;
	atomic_inc(szbuf+index);
}
//find max size for each component
__kernel void kernel_5(
	__global unsigned int* szbuf,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz,
	__global unsigned int* maxv,
	__read_only image3d_t mask)
{
	unsigned int i = (unsigned int)(get_global_id(0));
	unsigned int j = (unsigned int)(get_global_id(1));
	unsigned int k = (unsigned int)(get_global_id(2));
	float mask_value = read_imagef(mask, samp, (int4)(i, j, k, 1)).x;
	if (mask_value < 1e-6f)
		return;
	unsigned int index = nx*ny*k + nx*j + i;
	*maxv = max(szbuf[index], *maxv);
}
//set all size to max
__kernel void kernel_6(
	__global unsigned int* szbuf,
	__global unsigned int* label,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz,
	unsigned int lenx,
	unsigned int lenz,
	__read_only image3d_t mask)
{
	unsigned int i = (unsigned int)(get_global_id(0));
	unsigned int j = (unsigned int)(get_global_id(1));
	unsigned int k = (unsigned int)(get_global_id(2));
	float mask_value = read_imagef(mask, samp, (int4)(i, j, k, 1)).x;
	if (mask_value < 1e-6f)
		return;
	unsigned int index = nx*ny*k + nx*j + i;
	unsigned int value_l = label[index];
	if (value_l == 0)
		return;
	unsigned int res = value_l - 1;
	unsigned int x = 0;
	unsigned int y = 0;
	unsigned int z = 0;
	unsigned int ii;
	for (ii=0; ii<lenx; ++ii)
	{
		x |= (1<<(2*ii) & res)>>(ii);
		y |= (1<<(2*ii+1) & res)>>(ii+1);
	}
	z = res<<(32-lenx*2-lenz)>>(32-lenz);
	x = reverse_bit(x, lenx);
	y = reverse_bit(y, lenx);
	z = reverse_bit(z, lenz);
	unsigned int index2 = nx*ny*z + nx*y + x;
	if (index != index2)
		szbuf[index] = szbuf[index2];
}
//fix id by size
__kernel void kernel_7(
	__global unsigned int* szbuf,
	__global unsigned int* label,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz,
	unsigned int limit,
	__read_only image3d_t mask)
{
	unsigned int i = (unsigned int)(get_global_id(0));
	unsigned int j = (unsigned int)(get_global_id(1));
	unsigned int k = (unsigned int)(get_global_id(2));
	float mask_value = read_imagef(mask, samp, (int4)(i, j, k, 1)).x;
	if (mask_value < 1e-6f)
		return;
	unsigned int index = nx*ny*k + nx*j + i;
	//break if too small
	if (label[index]==0 ||
		szbuf[index] < limit)
		return;
	label[index] = label[index] | 0x80000000;
}
)CLKER";

inline constexpr const char* str_cl_brainbow_3d = R"CLKER(
const sampler_t samp =
	CLK_NORMALIZED_COORDS_FALSE|
	CLK_ADDRESS_CLAMP_TO_EDGE|
	CLK_FILTER_NEAREST;

float3 vol_grad_func(image3d_t image, int4 pos)
{
	float3 grad;
	grad.x = read_imagef(image, samp, pos+(int4)(1, 0, 0, 0)).x-
		read_imagef(image, samp, pos+(int4)(-1, 0, 0, 0)).x;
	grad.y = read_imagef(image, samp, pos+(int4)(0, 1, 0, 0)).x-
		read_imagef(image, samp, pos+(int4)(0, -1, 0, 0)).x;
	grad.z = read_imagef(image, samp, pos+(int4)(0, 0, 1, 0)).x-
		read_imagef(image, samp, pos+(int4)(0, 0, -1, 0)).x;
	return grad;
}

__kernel void kernel_0(
	__read_only image3d_t data,
	__global unsigned int* label,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz,
	__global unsigned int* rcnt,
	unsigned int seed,
	float value_t,
	float value_f,
	float grad_f,
	float sscale,
	int fixed)
{
	int3 coord = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	unsigned int index = nx*ny*coord.z + nx*coord.y + coord.x;
	unsigned int label_v = label[index];
	if (label_v == 0 || label_v & 0x80000000)
		return;
	atomic_inc(rcnt);
	float value = read_imagef(data, samp, (int4)(coord, 1)).x;
	value *= sscale;
	float grad = length(sscale * vol_grad_func(data, (int4)(coord, 1)));
	//stop function
	float stop =
		(grad_f>0.0f?(grad>sqrt(grad_f)*2.12f?0.0f:exp(-grad*grad/grad_f)):1.0f)*
		(value>value_t?1.0f:(value_f>0.0f?(value<value_t-sqrt(value_f)*2.12f?0.0f:exp(-(value-value_t)*(value-value_t)/value_f)):0.0f));
	
	//max filter
	float random = (float)((*rcnt) % seed)/(float)(seed)+1e-4f;
	if (stop < random)
		return;
	int3 nb_coord;
	unsigned int nb_index;
	unsigned int m;
	for (int i=-1; i<2; ++i)
	for (int j=-1; j<2; ++j)
	for (int k=-1; k<2; ++k)
	{
		nb_coord = (int3)(coord.x+i, coord.y+j, coord.z+k);
		if (nb_coord.x < 0 || nb_coord.x > nx-1 ||
			nb_coord.y < 0 || nb_coord.y > ny-1 ||
			nb_coord.z < 0 || nb_coord.z > nz-1)
			continue;
		nb_index = nx*ny*nb_coord.z + nx*nb_coord.y + nb_coord.x;
		m = label[nb_index];
		if ((m <= label_v) || (!fixed && (m & 0x80000000))) continue;
		label_v = m;
	}
	label[index] = label_v;
}
//grow only within mask
__kernel void kernel_1(
	__read_only image3d_t data,
	__global unsigned int* label,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz,
	__global unsigned int* rcnt,
	unsigned int seed,
	float value_t,
	float value_f,
	float grad_f,
	float sscale,
	int fixed,
	__read_only image3d_t mask)
{
	int3 coord = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	float mask_value = read_imagef(mask, samp, (int4)(coord, 1)).x;
	if (mask_value < 1e-6f)
		return;
	unsigned int index = nx*ny*coord.z + nx*coord.y + coord.x;
	unsigned int label_v = label[index];
	if (label_v == 0 || label_v & 0x80000000)
		return;
	atomic_inc(rcnt);
	float value = read_imagef(data, samp, (int4)(coord, 1)).x;
	value *= sscale;
	float grad = length(sscale * vol_grad_func(data, (int4)(coord, 1)));
	//stop function
	float stop =
		(grad_f>0.0f?(grad>sqrt(grad_f)*2.12f?0.0f:exp(-grad*grad/grad_f)):1.0f)*
		(value>value_t?1.0f:(value_f>0.0f?(value<value_t-sqrt(value_f)*2.12f?0.0f:exp(-(value-value_t)*(value-value_t)/value_f)):0.0f));
	
	//max filter
	float random = (float)((*rcnt) % seed)/(float)(seed)+1e-4f;
	if (stop < random)
		return;
	int3 nb_coord;
	unsigned int nb_index;
	unsigned int m;
	for (int i=-1; i<2; ++i)
	for (int j=-1; j<2; ++j)
	for (int k=-1; k<2; ++k)
	{
		nb_coord = (int3)(coord.x+i, coord.y+j, coord.z+k);
		mask_value = read_imagef(mask, samp, (int4)(nb_coord, 1)).x;
		if (mask_value < 1e-6f)
			continue;
		if (nb_coord.x < 0 || nb_coord.x > nx-1 ||
			nb_coord.y < 0 || nb_coord.y > ny-1 ||
			nb_coord.z < 0 || nb_coord.z > nz-1)
			continue;
		nb_index = nx*ny*nb_coord.z + nx*nb_coord.y + nb_coord.x;
		m = label[nb_index];
		if ((m <= label_v) || (!fixed && (m & 0x80000000))) continue;
		label_v = m;
	}
	label[index] = label_v;
}
)CLKER";

inline constexpr const char* str_cl_density_field_3d = R"CLKER(
const sampler_t samp =
	CLK_NORMALIZED_COORDS_FALSE|
	CLK_ADDRESS_CLAMP_TO_EDGE|
	CLK_FILTER_NEAREST;

float get_2d_density(image3d_t image, int4 pos, int r)
{
	float sum = 0.0f;
	int d = 2*r+1;
	for (int i=-r; i<=r; ++i)
	for (int j=-r; j<=r; ++j)
		sum += read_imagef(image, samp, pos+(int4)(i, j, 0, 0)).x;
	return sum / (float)(d * d);
}

//generate density field
__kernel void kernel_0(
	__read_only image3d_t data,
	__global unsigned char* df,
	unsigned int dnxy,
	unsigned int dnx,
	int dsize,
	float sscale)
{
	int3 ijk = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	unsigned int index = dnxy*ijk.z + dnx*ijk.y + ijk.x;
	float density = get_2d_density(data, (int4)(ijk, 1), dsize);
	df[index] = (unsigned char)(density * sscale * 255.0f);
}

//compute statistics on density field
__kernel void kernel_1(
	__global unsigned char* df,
	__global unsigned char* gavg,
	__global unsigned char* gvar,
	unsigned int gsx,
	unsigned int gsy,
	unsigned int gsz,
	unsigned int ngxy,
	unsigned int ngx,
	unsigned int dnxy, 
	unsigned int dnx)
{
	int3 gid = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	int3 lb = (int3)(gid.x*gsx, gid.y*gsy, gid.z*gsz);
	int3 ub = (int3)(lb.x + gsx, lb.y + gsy, lb.z + gsz);
	int3 ijk = (int3)(0);
	float gnum = (float)(gsx * gsy * gsz);
	float sum = 0.0f;
	float sum2 = 0.0f;
	unsigned int index;
	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)
	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)
	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)
	{
		index = dnxy*ijk.z + dnx*ijk.y + ijk.x;
		sum += df[index];
		sum2 += df[index] * df[index];
	}
	index = ngxy * gid.z + ngx * gid.y + gid.x;
	float avg = sum / gnum;
	gavg[index] = avg;
	float v = clamp(sqrt((sum2 + avg * avg * gnum - 2.0f * avg * sum) / gnum), 0.0f, 255.0f);
	gvar[index] = v;
}

//interpolate statistics on density field
__kernel void kernel_2(
	__global unsigned char* idf,
	__global unsigned char* gd,
	unsigned int gsx,
	unsigned int gsy,
	unsigned int gsz,
	unsigned int ngx,
	unsigned int ngy,
	unsigned int ngz,
	unsigned int dnxy, 
	unsigned int dnx)
{
	int3 ijk = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	int3 gid;
	int3 gijk;
	gijk = ijk % (int3)(gsx, gsy, gsz);
	gid = ijk / (int3)(gsx, gsy, gsz);
	gid += isless((float3)(gijk.x, gijk.y, gijk.z), (float3)(gsx/2.0f, gsy/2.0f, gsz/2.0f));
	int3 gcrd = clamp(gid + (int3)(0, 0, 0), (int3)(0), (int3)(ngx-1, ngy-1, ngz-1));
	uchar c000 = gd[ngx*ngy*gcrd.z + ngx*gcrd.y + gcrd.x];
	gcrd = clamp(gid + (int3)(1, 0, 0), (int3)(0), (int3)(ngx-1, ngy-1, ngz-1));
	uchar c100 = gd[ngx*ngy*gcrd.z + ngx*gcrd.y + gcrd.x];
	gcrd = clamp(gid + (int3)(0, 1, 0), (int3)(0), (int3)(ngx-1, ngy-1, ngz-1));
	uchar c010 = gd[ngx*ngy*gcrd.z + ngx*gcrd.y + gcrd.x];
	gcrd = clamp(gid + (int3)(1, 1, 0), (int3)(0), (int3)(ngx-1, ngy-1, ngz-1));
	uchar c110 = gd[ngx*ngy*gcrd.z + ngx*gcrd.y + gcrd.x];
	gcrd = clamp(gid + (int3)(0, 0, 1), (int3)(0), (int3)(ngx-1, ngy-1, ngz-1));
	uchar c001 = gd[ngx*ngy*gcrd.z + ngx*gcrd.y + gcrd.x];
	gcrd = clamp(gid + (int3)(1, 0, 1), (int3)(0), (int3)(ngx-1, ngy-1, ngz-1));
	uchar c101 = gd[ngx*ngy*gcrd.z + ngx*gcrd.y + gcrd.x];
	gcrd = clamp(gid + (int3)(0, 1, 1), (int3)(0), (int3)(ngx-1, ngy-1, ngz-1));
	uchar c011 = gd[ngx*ngy*gcrd.z + ngx*gcrd.y + gcrd.x];
	gcrd = clamp(gid + (int3)(1, 1, 1), (int3)(0), (int3)(ngx-1, ngy-1, ngz-1));
	uchar c111 = gd[ngx*ngy*gcrd.z + ngx*gcrd.y + gcrd.x];
	float3 d = ((float3)(gijk.x, gijk.y, gijk.z) - (float3)(gsx/2.0f, gsy/2.0f, gsz/2.0f)) / (float3)(gsx, gsy, gsz);
	int3 delta = isless(d, (float3)(0.0f));
	d -= (float3)(delta.x, delta.y, delta.z);
	float c00 = (float)(c000)*(1.0f-d.x) + (float)(c100)*d.x;
	float c01 = (float)(c001)*(1.0f-d.x) + (float)(c101)*d.x;
	float c10 = (float)(c010)*(1.0f-d.x) + (float)(c110)*d.x;
	float c11 = (float)(c011)*(1.0f-d.x) + (float)(c111)*d.x;
	float c0 = c00*(1.0f-d.y) + c10*d.y;
	float c1 = c01*(1.0f-d.y) + c11*d.y;
	idf[dnxy* ijk.z + dnx*ijk.y + ijk.x] = c0*(1.0f-d.z) + c1*d.z;
}
)CLKER";

inline constexpr const char* str_cl_density_grow_3d = R"CLKER(
const sampler_t samp =
	CLK_NORMALIZED_COORDS_FALSE|
	CLK_ADDRESS_CLAMP_TO_EDGE|
	CLK_FILTER_NEAREST;

float3 vol_grad_func(image3d_t image, int4 pos)
{
	float3 grad;
	grad.x = read_imagef(image, samp, pos+(int4)(1, 0, 0, 0)).x-
		read_imagef(image, samp, pos+(int4)(-1, 0, 0, 0)).x;
	grad.y = read_imagef(image, samp, pos+(int4)(0, 1, 0, 0)).x-
		read_imagef(image, samp, pos+(int4)(0, -1, 0, 0)).x;
	grad.z = read_imagef(image, samp, pos+(int4)(0, 0, 1, 0)).x-
		read_imagef(image, samp, pos+(int4)(0, 0, -1, 0)).x;
	return grad;
}

__kernel void kernel_0(
	__read_only image3d_t data,
	__global unsigned int* label,
	__global unsigned char* df,
	__global unsigned char* avg,
	__global unsigned char* var,
	__global unsigned int* rcnt,
	unsigned int seed,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz,
	unsigned int dnxy,
	unsigned int dnx,
	float value_t,
	float value_f,
	float grad_f,
	float density,
	float varth,
	float sscale,
	int fixed)
{
	int3 coord = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	unsigned int index = nx*ny*coord.z + nx*coord.y + coord.x;
	unsigned int label_v = label[index];
	if (label_v == 0 || label_v & 0x80000000)
		return;
	//break if low density
	if (density > 0.0f)
	{
		unsigned int index2 = dnxy*coord.z + dnx*coord.y + coord.x;
		unsigned char vdf = df[index2];
		unsigned char vavg = avg[index2];
		unsigned char vvar = var[index2];
		//break if low variance
		if (vvar < varth * 255)
			return;
		if (vdf < vavg - (1.0f-density)*vvar)
			return;
	}
	float value = read_imagef(data, samp, (int4)(coord, 1)).x;
	value *= sscale;
	float grad = length(sscale * vol_grad_func(data, (int4)(coord, 1)));
	//stop function
	float stop =
		(grad_f>0.0f?(grad>sqrt(grad_f)*2.12f?0.0f:exp(-grad*grad/grad_f)):1.0f)*
		(value>value_t?1.0f:(value_f>0.0f?(value<value_t-sqrt(value_f)*2.12f?0.0f:exp(-(value-value_t)*(value-value_t)/value_f)):0.0f));
	
	//max filter
	atomic_inc(rcnt);
	float random = (float)((*rcnt) % seed)/(float)(seed)+1e-4f;
	if (stop < random)
		return;
	int3 nb_coord;
	unsigned int nb_index;
	unsigned int m;
	for (int i=-1; i<2; ++i)
	for (int j=-1; j<2; ++j)
	for (int k=-1; k<2; ++k)
	{
		nb_coord = (int3)(coord.x+i, coord.y+j, coord.z+k);
		if (nb_coord.x < 0 || nb_coord.x > nx-1 ||
			nb_coord.y < 0 || nb_coord.y > ny-1 ||
			nb_coord.z < 0 || nb_coord.z > nz-1)
			continue;
		nb_index = nx*ny*nb_coord.z + nx*nb_coord.y + nb_coord.x;
		m = label[nb_index];
		if ((m <= label_v) || (!fixed && (m & 0x80000000))) continue;
		label_v = m;
	}
	label[index] = label_v;
}
//grow only within mask
__kernel void kernel_1(
	__read_only image3d_t data,
	__global unsigned int* label,
	__global unsigned char* df,
	__global unsigned char* avg,
	__global unsigned char* var,
	__global unsigned int* rcnt,
	unsigned int seed,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz,
	unsigned int dnxy,
	unsigned int dnx,
	float value_t,
	float value_f,
	float grad_f,
	float density,
	float varth,
	float sscale,
	int fixed,
	__read_only image3d_t mask)
{
	int3 coord = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	float mask_value = read_imagef(mask, samp, (int4)(coord, 1)).x;
	if (mask_value < 1e-6f)
		return;
	unsigned int index = nx*ny*coord.z + nx*coord.y + coord.x;
	unsigned int label_v = label[index];
	if (label_v == 0 || label_v & 0x80000000)
		return;
	//break if low density
	if (density > 0.0f)
	{
		unsigned int index2 = dnxy*coord.z + dnx*coord.y + coord.x;
		unsigned char vdf = df[index2];
		unsigned char vavg = avg[index2];
		unsigned char vvar = var[index2];
		//break if low variance
		if (vvar < varth * 255)
			return;
		if (vdf < vavg - (1.0f-density)*vvar)
			return;
	}
	float value = read_imagef(data, samp, (int4)(coord, 1)).x;
	value *= sscale;
	float grad = length(sscale * vol_grad_func(data, (int4)(coord, 1)));
	//stop function
	float stop =
		(grad_f>0.0f?(grad>sqrt(grad_f)*2.12f?0.0f:exp(-grad*grad/grad_f)):1.0f)*
		(value>value_t?1.0f:(value_f>0.0f?(value<value_t-sqrt(value_f)*2.12f?0.0f:exp(-(value-value_t)*(value-value_t)/value_f)):0.0f));
	
	//max filter
	atomic_inc(rcnt);
	float random = (float)((*rcnt) % seed)/(float)(seed)+1e-4f;
	if (stop < random)
		return;
	int3 nb_coord;
	unsigned int nb_index;
	unsigned int m;
	for (int i=-1; i<2; ++i)
	for (int j=-1; j<2; ++j)
	for (int k=-1; k<2; ++k)
	{
		nb_coord = (int3)(coord.x+i, coord.y+j, coord.z+k);
		mask_value = read_imagef(mask, samp, (int4)(nb_coord, 1)).x;
		if (mask_value < 1e-6f)
			continue;
		if (nb_coord.x < 0 || nb_coord.x > nx-1 ||
			nb_coord.y < 0 || nb_coord.y > ny-1 ||
			nb_coord.z < 0 || nb_coord.z > nz-1)
			continue;
		nb_index = nx*ny*nb_coord.z + nx*nb_coord.y + nb_coord.x;
		m = label[nb_index];
		if ((m <= label_v) || (!fixed && (m & 0x80000000))) continue;
		label_v = m;
	}
	label[index] = label_v;
}
)CLKER";

inline constexpr const char* str_cl_dist_field_2d = R"CLKER(
const sampler_t samp =
	CLK_NORMALIZED_COORDS_FALSE|
	CLK_ADDRESS_CLAMP_TO_EDGE|
	CLK_FILTER_NEAREST;

float get_2d_density(image3d_t image, int4 pos, int r)
{
	float sum = 0.0f;
	int d = 2*r+1;
	for (int i=-r; i<=r; ++i)
	for (int j=-r; j<=r; ++j)
		sum += read_imagef(image, samp, pos+(int4)(i, j, 0, 0)).x;
	return sum / (float)(d * d);
}
__kernel void kernel_0(
	__read_only image3d_t data,
	__global unsigned char* df,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz,
	int dsize,
	float th,
	float sscale,
	unsigned char ini)
{
	int3 ijk = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	unsigned int index = nx*ny*ijk.z + nx*ijk.y + ijk.x;
	if (ijk.x == 0 || ijk.x == nx-1 ||
		ijk.y == 0 || ijk.y == ny-1)
	{
		df[index] = 0;
		return;
	}
	//float dval = read_imagef(data, samp, (int4)(ijk, 1)).x;
	float dval = get_2d_density(data, (int4)(ijk, 1), dsize);
	dval *= sscale;
	if (dval > th)
		df[index] = ini;
	else
		df[index] = 0;
}

__kernel void kernel_1(
	__global unsigned char* df,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz,
	unsigned char ini,
	unsigned char nn,
	unsigned char re)
{
	int3 ijk = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	unsigned int nxy = nx*ny;
	unsigned int index = nxy*ijk.z + nx*ijk.y + ijk.x;
	if (df[index] == ini)
	{
		unsigned char v1 = df[nxy*ijk.z + nx*ijk.y + ijk.x - 1];
		unsigned char v2 = df[nxy*ijk.z + nx*ijk.y + ijk.x + 1];
		unsigned char v3 = df[nxy*ijk.z + nx*(ijk.y-1) + ijk.x];
		unsigned char v4 = df[nxy*ijk.z + nx*(ijk.y+1) + ijk.x];
		if (v1 == nn || v2 == nn ||
			v3 == nn || v4 == nn)
			df[index] = re;
	}
}
__kernel void kernel_2(
	__global unsigned char* df,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz,
	unsigned char ini,
	unsigned char nn,
	unsigned char re)
{
	int3 ijk = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	unsigned int nxy = nx*ny;
	unsigned int index = nxy*ijk.z + nx*ijk.y + ijk.x;
	if (df[index] == ini)
	{
		short v1 = df[nxy*ijk.z + nx*ijk.y + ijk.x - 1];
		short v2 = df[nxy*ijk.z + nx*ijk.y + ijk.x + 1];
		short v3 = df[nxy*ijk.z + nx*(ijk.y-1) + ijk.x];
		short v4 = df[nxy*ijk.z + nx*(ijk.y+1) + ijk.x];
		short rre = (ijk.x % 13 + ijk.y % 17) % 4;
		v1 = rre == 0 ? -1 : v1;
		v2 = rre == 3 ? -1 : v2;
		v3 = rre == 1 ? -1 : v3;
		v4 = rre == 2 ? -1 : v4;
		if (v1 == nn || v2 == nn ||
			v3 == nn || v4 == nn)
			df[index] = re;
	}
}
//compute dist field in mask
__kernel void kernel_3(
	__read_only image3d_t data,
	__global unsigned char* df,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz,
	int dsize,
	float th,
	float sscale,
	unsigned char ini,
	__read_only image3d_t mask)
{
	int3 ijk = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	unsigned int index = nx*ny*ijk.z + nx*ijk.y + ijk.x;
	float mask_value = read_imagef(mask, samp, (int4)(ijk, 1)).x;
	if (mask_value < 1e-6f)
	{
		df[index] = 0;
		return;
	}
	if (ijk.x == 0 || ijk.x == nx-1 ||
		ijk.y == 0 || ijk.y == ny-1)
	{
		df[index] = 0;
		return;
	}
	//float dval = read_imagef(data, samp, (int4)(ijk, 1)).x;
	float dval = get_2d_density(data, (int4)(ijk, 1), dsize);
	dval *= sscale;
	if (dval > th)
		df[index] = ini;
	else
		df[index] = 0;
}
//compute dist field in mask
__kernel void kernel_4(
	__global unsigned char* df,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz,
	unsigned char ini,
	unsigned char nn,
	unsigned char re,
	__read_only image3d_t mask)
{
	int3 ijk = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	float mask_value = read_imagef(mask, samp, (int4)(ijk, 1)).x;
	if (mask_value < 1e-6f)
		return;
	unsigned int nxy = nx*ny;
	unsigned int index = nxy*ijk.z + nx*ijk.y + ijk.x;
	if (df[index] == ini)
	{
		unsigned char v1 = df[nxy*ijk.z + nx*ijk.y + ijk.x - 1];
		unsigned char v2 = df[nxy*ijk.z + nx*ijk.y + ijk.x + 1];
		unsigned char v3 = df[nxy*ijk.z + nx*(ijk.y-1) + ijk.x];
		unsigned char v4 = df[nxy*ijk.z + nx*(ijk.y+1) + ijk.x];
		if (v1 == nn || v2 == nn ||
			v3 == nn || v4 == nn)
			df[index] = re;
	}
}
//compute dist field in mask
__kernel void kernel_5(
	__global unsigned char* df,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz,
	unsigned char ini,
	unsigned char nn,
	unsigned char re,
	__read_only image3d_t mask)
{
	int3 ijk = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	float mask_value = read_imagef(mask, samp, (int4)(ijk, 1)).x;
	if (mask_value < 1e-6f)
		return;
	unsigned int nxy = nx*ny;
	unsigned int index = nxy*ijk.z + nx*ijk.y + ijk.x;
	if (df[index] == ini)
	{
		short v1 = df[nxy*ijk.z + nx*ijk.y + ijk.x - 1];
		short v2 = df[nxy*ijk.z + nx*ijk.y + ijk.x + 1];
		short v3 = df[nxy*ijk.z + nx*(ijk.y-1) + ijk.x];
		short v4 = df[nxy*ijk.z + nx*(ijk.y+1) + ijk.x];
		short rre = (ijk.x % 13 + ijk.y % 17) % 4;
		v1 = rre == 0 ? -1 : v1;
		v2 = rre == 3 ? -1 : v2;
		v3 = rre == 1 ? -1 : v3;
		v4 = rre == 2 ? -1 : v4;
		if (v1 == nn || v2 == nn ||
			v3 == nn || v4 == nn)
			df[index] = re;
	}
}
)CLKER";

inline constexpr const char* str_cl_dist_field_3d = R"CLKER(
const sampler_t samp =
	CLK_NORMALIZED_COORDS_FALSE|
	CLK_ADDRESS_CLAMP_TO_EDGE|
	CLK_FILTER_NEAREST;

float get_2d_density(image3d_t image, int4 pos, int r)
{
	float sum = 0.0f;
	int d = 2*r+1;
	for (int i=-r; i<=r; ++i)
	for (int j=-r; j<=r; ++j)
		sum += read_imagef(image, samp, pos+(int4)(i, j, 0, 0)).x;
	return sum / (float)(d * d);
}
__kernel void kernel_0(
	__read_only image3d_t data,
	__global unsigned char* df,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz,
	int dsize,
	float th,
	float sscale,
	unsigned char ini)
{
	int3 ijk = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	unsigned int index = nx*ny*ijk.z + nx*ijk.y + ijk.x;
	if (ijk.x == 0 || ijk.x == nx-1 ||
		ijk.y == 0 || ijk.y == ny-1 ||
		ijk.z == 0 || ijk.z == nz-1)
	{
		df[index] = 0;
		return;
	}
	//float dval = read_imagef(data, samp, (int4)(ijk, 1)).x;
	float dval = get_2d_density(data, (int4)(ijk, 1), dsize);
	dval *= sscale;
	if (dval > th)
		df[index] = ini;
	else
		df[index] = 0;
}

__kernel void kernel_1(
	__global unsigned char* df,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz,
	unsigned char ini,
	unsigned char nn,
	unsigned char re)
{
	int3 ijk = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	unsigned int nxy = nx*ny;
	unsigned int index = nxy*ijk.z + nx*ijk.y + ijk.x;
	if (df[index] == ini)
	{
		unsigned char v1 = df[nxy*ijk.z + nx*ijk.y + ijk.x - 1];
		unsigned char v2 = df[nxy*ijk.z + nx*ijk.y + ijk.x + 1];
		unsigned char v3 = df[nxy*ijk.z + nx*(ijk.y-1) + ijk.x];
		unsigned char v4 = df[nxy*ijk.z + nx*(ijk.y+1) + ijk.x];
		unsigned char v5 = df[nxy*(ijk.z-1) + nx*ijk.y + ijk.x];
		unsigned char v6 = df[nxy*(ijk.z+1) + nx*ijk.y + ijk.x];
		if (v1 == nn || v2 == nn ||
			v3 == nn || v4 == nn ||
			v5 == nn || v6 == nn)
			df[index] = re;
	}
}
__kernel void kernel_2(
	__global unsigned char* df,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz,
	unsigned char ini,
	unsigned char nn,
	unsigned char re)
{
	int3 ijk = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	unsigned int nxy = nx*ny;
	unsigned int index = nxy*ijk.z + nx*ijk.y + ijk.x;
	if (df[index] == ini)
	{
		short v1 = df[nxy*ijk.z + nx*ijk.y + ijk.x - 1];
		short v2 = df[nxy*ijk.z + nx*ijk.y + ijk.x + 1];
		short v3 = df[nxy*ijk.z + nx*(ijk.y-1) + ijk.x];
		short v4 = df[nxy*ijk.z + nx*(ijk.y+1) + ijk.x];
		short v5 = df[nxy*(ijk.z-1) + nx*ijk.y + ijk.x];
		short v6 = df[nxy*(ijk.z+1) + nx*ijk.y + ijk.x];
		short rre = (ijk.x % 13 + ijk.y % 17 + ijk.z % 7) % 6;
		v1 = rre == 0 ? -1 : v1;
		v2 = rre == 5 ? -1 : v2;
		v3 = rre == 2 ? -1 : v3;
		v4 = rre == 3 ? -1 : v4;
		v5 = rre == 1 ? -1 : v5;
		v6 = rre == 4 ? -1 : v6;
		if (v1 == nn || v2 == nn ||
			v3 == nn || v4 == nn ||
			v5 == nn || v6 == nn)
			df[index] = re;
	}
}
//compute dist field in mask
__kernel void kernel_3(
	__read_only image3d_t data,
	__global unsigned char* df,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz,
	int dsize,
	float th,
	float sscale,
	unsigned char ini,
	__read_only image3d_t mask)
{
	int3 ijk = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	unsigned int index = nx*ny*ijk.z + nx*ijk.y + ijk.x;
	float mask_value = read_imagef(mask, samp, (int4)(ijk, 1)).x;
	if (mask_value < 1e-6f)
	{
		df[index] = 0;
		return;
	}
	if (ijk.x == 0 || ijk.x == nx-1 ||
		ijk.y == 0 || ijk.y == ny-1 ||
		ijk.z == 0 || ijk.z == nz-1)
	{
		df[index] = 0;
		return;
	}
	//float dval = read_imagef(data, samp, (int4)(ijk, 1)).x;
	float dval = get_2d_density(data, (int4)(ijk, 1), dsize);
	dval *= sscale;
	if (dval > th)
		df[index] = ini;
	else
		df[index] = 0;
}
//compute dist field in mask
__kernel void kernel_4(
	__global unsigned char* df,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz,
	unsigned char ini,
	unsigned char nn,
	unsigned char re,
	__read_only image3d_t mask)
{
	int3 ijk = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	float mask_value = read_imagef(mask, samp, (int4)(ijk, 1)).x;
	if (mask_value < 1e-6f)
		return;
	unsigned int nxy = nx*ny;
	unsigned int index = nxy*ijk.z + nx*ijk.y + ijk.x;
	if (df[index] == ini)
	{
		unsigned char v1 = df[nxy*ijk.z + nx*ijk.y + ijk.x - 1];
		unsigned char v2 = df[nxy*ijk.z + nx*ijk.y + ijk.x + 1];
		unsigned char v3 = df[nxy*ijk.z + nx*(ijk.y-1) + ijk.x];
		unsigned char v4 = df[nxy*ijk.z + nx*(ijk.y+1) + ijk.x];
		unsigned char v5 = df[nxy*(ijk.z-1) + nx*ijk.y + ijk.x];
		unsigned char v6 = df[nxy*(ijk.z+1) + nx*ijk.y + ijk.x];
		if (v1 == nn || v2 == nn ||
			v3 == nn || v4 == nn ||
			v5 == nn || v6 == nn)
			df[index] = re;
	}
}
//compute dist field in mask
__kernel void kernel_5(
	__global unsigned char* df,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz,
	unsigned char ini,
	unsigned char nn,
	unsigned char re,
	__read_only image3d_t mask)
{
	int3 ijk = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	float mask_value = read_imagef(mask, samp, (int4)(ijk, 1)).x;
	if (mask_value < 1e-6f)
		return;
	unsigned int nxy = nx*ny;
	unsigned int index = nxy*ijk.z + nx*ijk.y + ijk.x;
	if (df[index] == ini)
	{
		short v1 = df[nxy*ijk.z + nx*ijk.y + ijk.x - 1];
		short v2 = df[nxy*ijk.z + nx*ijk.y + ijk.x + 1];
		short v3 = df[nxy*ijk.z + nx*(ijk.y-1) + ijk.x];
		short v4 = df[nxy*ijk.z + nx*(ijk.y+1) + ijk.x];
		short v5 = df[nxy*(ijk.z-1) + nx*ijk.y + ijk.x];
		short v6 = df[nxy*(ijk.z+1) + nx*ijk.y + ijk.x];
		short rre = (ijk.x % 13 + ijk.y % 17 + ijk.z % 7) % 6;
		v1 = rre == 0 ? -1 : v1;
		v2 = rre == 5 ? -1 : v2;
		v3 = rre == 2 ? -1 : v3;
		v4 = rre == 3 ? -1 : v4;
		v5 = rre == 1 ? -1 : v5;
		v6 = rre == 4 ? -1 : v6;
		if (v1 == nn || v2 == nn ||
			v3 == nn || v4 == nn ||
			v5 == nn || v6 == nn)
			df[index] = re;
	}
}
)CLKER";

inline constexpr const char* str_cl_dist_grow_3d = R"CLKER(
const sampler_t samp =
	CLK_NORMALIZED_COORDS_FALSE|
	CLK_ADDRESS_CLAMP_TO_EDGE|
	CLK_FILTER_NEAREST;

float3 vol_grad_func(image3d_t image, int4 pos)
{
	float3 grad;
	grad.x = read_imagef(image, samp, pos+(int4)(1, 0, 0, 0)).x-
		read_imagef(image, samp, pos+(int4)(-1, 0, 0, 0)).x;
	grad.y = read_imagef(image, samp, pos+(int4)(0, 1, 0, 0)).x-
		read_imagef(image, samp, pos+(int4)(0, -1, 0, 0)).x;
	grad.z = read_imagef(image, samp, pos+(int4)(0, 0, 1, 0)).x-
		read_imagef(image, samp, pos+(int4)(0, 0, -1, 0)).x;
	return grad;
}

__kernel void kernel_0(
	__read_only image3d_t data,
	__global unsigned int* label,
	__global unsigned char* distf,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz,
	__global unsigned int* rcnt,
	unsigned int seed,
	float value_t,
	float value_f,
	float grad_f,
	float sscale,
	float distscl,
	float dist_strength,
	int fixed)
{
	int3 coord = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	unsigned int index = nx*ny*coord.z + nx*coord.y + coord.x;
	unsigned int label_v = label[index];
	if (label_v == 0 || label_v & 0x80000000)
		return;
	atomic_inc(rcnt);
	float value = read_imagef(data, samp, (int4)(coord, 1)).x;
	value *= sscale;
	float distv = distscl * distf[index];
	distv = clamp(distv, 0.0f, 1.0f);
	value = value * (1.0f - dist_strength) + distv * dist_strength;
	float grad = length(sscale * vol_grad_func(data, (int4)(coord, 1)));
	//stop function
	float stop =
		(grad_f>0.0f?(grad>sqrt(grad_f)*2.12f?0.0f:exp(-grad*grad/grad_f)):1.0f)*
		(value>value_t?1.0f:(value_f>0.0f?(value<value_t-sqrt(value_f)*2.12f?0.0f:exp(-(value-value_t)*(value-value_t)/value_f)):0.0f));
	
	//max filter
	float random = (float)((*rcnt) % seed)/(float)(seed)+1e-4f;
	if (stop < random)
		return;
	int3 nb_coord;
	unsigned int nb_index;
	unsigned int m;
	for (int i=-1; i<2; ++i)
	for (int j=-1; j<2; ++j)
	for (int k=-1; k<2; ++k)
	{
		nb_coord = (int3)(coord.x+i, coord.y+j, coord.z+k);
		if (nb_coord.x < 0 || nb_coord.x > nx-1 ||
			nb_coord.y < 0 || nb_coord.y > ny-1 ||
			nb_coord.z < 0 || nb_coord.z > nz-1)
			continue;
		nb_index = nx*ny*nb_coord.z + nx*nb_coord.y + nb_coord.x;
		m = label[nb_index];
		if ((m <= label_v) || (!fixed && (m & 0x80000000))) continue;
		label_v = m;
	}
	label[index] = label_v;
}
//only grow within mask
__kernel void kernel_1(
	__read_only image3d_t data,
	__global unsigned int* label,
	__global unsigned char* distf,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz,
	__global unsigned int* rcnt,
	unsigned int seed,
	float value_t,
	float value_f,
	float grad_f,
	float sscale,
	float distscl,
	float dist_strength,
	int fixed,
	__read_only image3d_t mask)
{
	int3 coord = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	float mask_value = read_imagef(mask, samp, (int4)(coord, 1)).x;
	if (mask_value < 1e-6f)
		return;
	unsigned int index = nx*ny*coord.z + nx*coord.y + coord.x;
	unsigned int label_v = label[index];
	if (label_v == 0 || label_v & 0x80000000)
		return;
	atomic_inc(rcnt);
	float value = read_imagef(data, samp, (int4)(coord, 1)).x;
	value *= sscale;
	float distv = distscl * distf[index];
	distv = clamp(distv, 0.0f, 1.0f);
	value = value * (1.0f - dist_strength) + distv * dist_strength;
	float grad = length(sscale * vol_grad_func(data, (int4)(coord, 1)));
	//stop function
	float stop =
		(grad_f>0.0f?(grad>sqrt(grad_f)*2.12f?0.0f:exp(-grad*grad/grad_f)):1.0f)*
		(value>value_t?1.0f:(value_f>0.0f?(value<value_t-sqrt(value_f)*2.12f?0.0f:exp(-(value-value_t)*(value-value_t)/value_f)):0.0f));
	
	//max filter
	float random = (float)((*rcnt) % seed)/(float)(seed)+1e-4f;
	if (stop < random)
		return;
	int3 nb_coord;
	unsigned int nb_index;
	unsigned int m;
	for (int i=-1; i<2; ++i)
	for (int j=-1; j<2; ++j)
	for (int k=-1; k<2; ++k)
	{
		nb_coord = (int3)(coord.x+i, coord.y+j, coord.z+k);
		mask_value = read_imagef(mask, samp, (int4)(nb_coord, 1)).x;
		if (mask_value < 1e-6f)
			continue;
		if (nb_coord.x < 0 || nb_coord.x > nx-1 ||
			nb_coord.y < 0 || nb_coord.y > ny-1 ||
			nb_coord.z < 0 || nb_coord.z > nz-1)
			continue;
		nb_index = nx*ny*nb_coord.z + nx*nb_coord.y + nb_coord.x;
		m = label[nb_index];
		if ((m <= label_v) || (!fixed && (m & 0x80000000))) continue;
		label_v = m;
	}
	label[index] = label_v;
}
)CLKER";

inline constexpr const char* str_cl_distdens_field_3d = R"CLKER(
const sampler_t samp =
	CLK_NORMALIZED_COORDS_FALSE|
	CLK_ADDRESS_CLAMP_TO_EDGE|
	CLK_FILTER_NEAREST;

float get_2d_density(image3d_t image, int4 pos, int r)
{
	float sum = 0.0f;
	int d = 2*r+1;
	for (int i=-r; i<=r; ++i)
	for (int j=-r; j<=r; ++j)
		sum += read_imagef(image, samp, pos+(int4)(i, j, 0, 0)).x;
	return sum / (float)(d * d);
}

//generate density field
__kernel void kernel_0(
	__read_only image3d_t data,
	__global unsigned char* distf,
	__global unsigned char* densf,
	unsigned int nxy,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz,
	unsigned int dnxy,
	unsigned int dnx,
	int dsize,
	float sscale,
	float distscl,
	float dist_strength)
{
	int3 ijk = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	float density = get_2d_density(data, (int4)(ijk, 1), dsize) * sscale;
	unsigned int index = nxy*clamp(ijk.z, 0, (int)(nz-1)) +
		nx*clamp(ijk.y, 0, (int)(ny-1)) + clamp(ijk.x, 0, (int)(nx-1));
	float distv = distscl * dist_strength * distf[index];
	distv = clamp(distv, 0.0f, 1.0f);
	density = density * (1.0f - dist_strength) + distv * dist_strength;
	index = dnxy*ijk.z + dnx*ijk.y + ijk.x;
	densf[index] = (unsigned char)(density * 255.0f);
}

//compute statistics on density field
__kernel void kernel_1(
	__global unsigned char* df,
	__global unsigned char* gavg,
	__global unsigned char* gvar,
	unsigned int gsx,
	unsigned int gsy,
	unsigned int gsz,
	unsigned int ngxy,
	unsigned int ngx,
	unsigned int dnxy, 
	unsigned int dnx)
{
	int3 gid = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	int3 lb = (int3)(gid.x*gsx, gid.y*gsy, gid.z*gsz);
	int3 ub = (int3)(lb.x + gsx, lb.y + gsy, lb.z + gsz);
	int3 ijk = (int3)(0);
	float gnum = (float)(gsx * gsy * gsz);
	float sum = 0.0f;
	float sum2 = 0.0f;
	unsigned int index;
	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)
	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)
	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)
	{
		index = dnxy*ijk.z + dnx*ijk.y + ijk.x;
		sum += df[index];
		sum2 += df[index] * df[index];
	}
	index = ngxy * gid.z + ngx * gid.y + gid.x;
	float avg = sum / gnum;
	gavg[index] = avg;
	float v = clamp(sqrt((sum2 + avg * avg * gnum - 2.0f * avg * sum) / gnum), 0.0f, 255.0f);
	gvar[index] = v;
}

//interpolate statistics on density field
__kernel void kernel_2(
	__global unsigned char* idf,
	__global unsigned char* gd,
	unsigned int gsx,
	unsigned int gsy,
	unsigned int gsz,
	unsigned int ngx,
	unsigned int ngy,
	unsigned int ngz,
	unsigned int dnxy, 
	unsigned int dnx)
{
	int3 ijk = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	int3 gid;
	int3 gijk;
	gijk = ijk % (int3)(gsx, gsy, gsz);
	gid = ijk / (int3)(gsx, gsy, gsz);
	gid += isless((float3)(gijk.x, gijk.y, gijk.z), (float3)(gsx/2.0f, gsy/2.0f, gsz/2.0f));
	int3 gcrd = clamp(gid + (int3)(0, 0, 0), (int3)(0), (int3)(ngx-1, ngy-1, ngz-1));
	uchar c000 = gd[ngx*ngy*gcrd.z + ngx*gcrd.y + gcrd.x];
	gcrd = clamp(gid + (int3)(1, 0, 0), (int3)(0), (int3)(ngx-1, ngy-1, ngz-1));
	uchar c100 = gd[ngx*ngy*gcrd.z + ngx*gcrd.y + gcrd.x];
	gcrd = clamp(gid + (int3)(0, 1, 0), (int3)(0), (int3)(ngx-1, ngy-1, ngz-1));
	uchar c010 = gd[ngx*ngy*gcrd.z + ngx*gcrd.y + gcrd.x];
	gcrd = clamp(gid + (int3)(1, 1, 0), (int3)(0), (int3)(ngx-1, ngy-1, ngz-1));
	uchar c110 = gd[ngx*ngy*gcrd.z + ngx*gcrd.y + gcrd.x];
	gcrd = clamp(gid + (int3)(0, 0, 1), (int3)(0), (int3)(ngx-1, ngy-1, ngz-1));
	uchar c001 = gd[ngx*ngy*gcrd.z + ngx*gcrd.y + gcrd.x];
	gcrd = clamp(gid + (int3)(1, 0, 1), (int3)(0), (int3)(ngx-1, ngy-1, ngz-1));
	uchar c101 = gd[ngx*ngy*gcrd.z + ngx*gcrd.y + gcrd.x];
	gcrd = clamp(gid + (int3)(0, 1, 1), (int3)(0), (int3)(ngx-1, ngy-1, ngz-1));
	uchar c011 = gd[ngx*ngy*gcrd.z + ngx*gcrd.y + gcrd.x];
	gcrd = clamp(gid + (int3)(1, 1, 1), (int3)(0), (int3)(ngx-1, ngy-1, ngz-1));
	uchar c111 = gd[ngx*ngy*gcrd.z + ngx*gcrd.y + gcrd.x];
	float3 d = ((float3)(gijk.x, gijk.y, gijk.z) - (float3)(gsx/2.0f, gsy/2.0f, gsz/2.0f)) / (float3)(gsx, gsy, gsz);
	int3 delta = isless(d, (float3)(0.0f));
	d -= (float3)(delta.x, delta.y, delta.z);
	float c00 = (float)(c000)*(1.0-d.x) + (float)(c100)*d.x;
	float c01 = (float)(c001)*(1.0-d.x) + (float)(c101)*d.x;
	float c10 = (float)(c010)*(1.0-d.x) + (float)(c110)*d.x;
	float c11 = (float)(c011)*(1.0-d.x) + (float)(c111)*d.x;
	float c0 = c00*(1.0-d.y) + c10*d.y;
	float c1 = c01*(1.0-d.y) + c11*d.y;
	idf[dnxy* ijk.z + dnx*ijk.y + ijk.x] = c0*(1.0f-d.z) + c1*d.z;
}
)CLKER";

inline constexpr const char* str_cl_cleanup_3d = R"CLKER(
const sampler_t samp =
	CLK_NORMALIZED_COORDS_FALSE|
	CLK_ADDRESS_CLAMP_TO_EDGE|
	CLK_FILTER_NEAREST;

unsigned int __attribute((always_inline)) reverse_bit(unsigned int val, unsigned int len)
{
	unsigned int res = val;
	int s = len - 1;
	for (val >>= 1; val; val >>= 1)
	{
		res <<= 1;
		res |= val & 1;
		s--;
	}
	res <<= s;
	res <<= 32-len;
	res >>= 32-len;
	return res;
}
__kernel void kernel_0(
	__global unsigned int* szbuf,
	__global unsigned int* label,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz,
	unsigned int lenx,
	unsigned int lenz)
{
	unsigned int i = (unsigned int)(get_global_id(0));
	unsigned int j = (unsigned int)(get_global_id(1));
	unsigned int k = (unsigned int)(get_global_id(2));
	unsigned int index = nx*ny*k + nx*j + i;
	unsigned int value_l = label[index];
	if (value_l == 0)
		return;
	unsigned int res = value_l - 1;
	unsigned int x = 0;
	unsigned int y = 0;
	unsigned int z = 0;
	unsigned int ii;
	for (ii=0; ii<lenx; ++ii)
	{
		x |= (1<<(2*ii) & res)>>(ii);
		y |= (1<<(2*ii+1) & res)>>(ii+1);
	}
	z = res<<(32-lenx*2-lenz)>>(32-lenz);
	x = reverse_bit(x, lenx);
	y = reverse_bit(y, lenx);
	z = reverse_bit(z, lenz);
	index = nx*ny*z + nx*y + x;
	atomic_inc(szbuf+index);
}
__kernel void kernel_1(
	__global unsigned int* szbuf,
	__global unsigned int* label,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz,
	unsigned int lenx,
	unsigned int lenz)
{
	unsigned int i = (unsigned int)(get_global_id(0));
	unsigned int j = (unsigned int)(get_global_id(1));
	unsigned int k = (unsigned int)(get_global_id(2));
	unsigned int index = nx*ny*k + nx*j + i;
	unsigned int value_l = label[index];
	if (value_l == 0)
		return;
	unsigned int res = value_l - 1;
	unsigned int x = 0;
	unsigned int y = 0;
	unsigned int z = 0;
	unsigned int ii;
	for (ii=0; ii<lenx; ++ii)
	{
		x |= (1<<(2*ii) & res)>>(ii);
		y |= (1<<(2*ii+1) & res)>>(ii+1);
	}
	z = res<<(32-lenx*2-lenz)>>(32-lenz);
	x = reverse_bit(x, lenx);
	y = reverse_bit(y, lenx);
	z = reverse_bit(z, lenz);
	unsigned int index2 = nx*ny*z + nx*y + x;
	if (index != index2)
		atomic_xchg(szbuf+index, szbuf[index2]);
}
__kernel void kernel_2(
	__read_only image3d_t data,
	__global unsigned int* szbuf,
	__global unsigned int* label,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz,
	unsigned int thresh)
{
	unsigned int i = (unsigned int)(get_global_id(0));
	unsigned int j = (unsigned int)(get_global_id(1));
	unsigned int k = (unsigned int)(get_global_id(2));
	unsigned int index = nx*ny*k + nx*j + i;
	//break if large enough
	if (label[index]==0 ||
		szbuf[index] > thresh)
		return;
	float value = read_imagef(data, samp, (int4)(i, j, k, 1)).x;
	unsigned int nb_index;
	unsigned int min_dist = 10;
	unsigned int dist;
	unsigned int max_nb_index;
	float nb_value;
	for (int ni=-1; ni<2; ++ni)
	for (int nj=-1; nj<2; ++nj)
	for (int nk=-1; nk<2; ++nk)
	{
		if ((i==0 && ni==-1) ||
			(i==nx-1 && ni==1) ||
			(j==0 && nj==-1) ||
			(j==ny-1 && nj==1) ||
			(k==0 && nk==-1) ||
			(k==nz-1 && nk==1))
			continue;
		nb_index = nx*ny*(k+nk) + nx*(j+nj) + i+ni;
		dist = abs(nk) + abs(nj) + abs(ni);
		if (szbuf[nb_index]>thresh &&
			dist < min_dist)
		{
			nb_value = read_imagef(data, samp, (int4)(i+ni, j+nj, k+nk, 1)).x;
			if (nb_value < value)
				continue;
			min_dist = dist;
			max_nb_index = nb_index;
		}
	}
	if (min_dist < 10)
		atomic_xchg(label+index, label[max_nb_index]);
}
__kernel void kernel_3(
	__global unsigned int* szbuf,
	__global unsigned int* label,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz,
	unsigned int lenx,
	unsigned int lenz,
	__read_only image3d_t mask)
{
	unsigned int i = (unsigned int)(get_global_id(0));
	unsigned int j = (unsigned int)(get_global_id(1));
	unsigned int k = (unsigned int)(get_global_id(2));
	float mask_value = read_imagef(mask, samp, (int4)(i, j, k, 1)).x;
	if (mask_value < 1e-6f)
		return;
	unsigned int index = nx*ny*k + nx*j + i;
	unsigned int value_l = label[index];
	if (value_l == 0)
		return;
	unsigned int res = value_l - 1;
	unsigned int x = 0;
	unsigned int y = 0;
	unsigned int z = 0;
	unsigned int ii;
	for (ii=0; ii<lenx; ++ii)
	{
		x |= (1<<(2*ii) & res)>>(ii);
		y |= (1<<(2*ii+1) & res)>>(ii+1);
	}
	z = res<<(32-lenx*2-lenz)>>(32-lenz);
	x = reverse_bit(x, lenx);
	y = reverse_bit(y, lenx);
	z = reverse_bit(z, lenz);
	index = nx*ny*z + nx*y + x;
	atomic_inc(szbuf+index);
}
__kernel void kernel_4(
	__global unsigned int* szbuf,
	__global unsigned int* label,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz,
	unsigned int lenx,
	unsigned int lenz,
	__read_only image3d_t mask)
{
	unsigned int i = (unsigned int)(get_global_id(0));
	unsigned int j = (unsigned int)(get_global_id(1));
	unsigned int k = (unsigned int)(get_global_id(2));
	float mask_value = read_imagef(mask, samp, (int4)(i, j, k, 1)).x;
	if (mask_value < 1e-6f)
		return;
	unsigned int index = nx*ny*k + nx*j + i;
	unsigned int value_l = label[index];
	if (value_l == 0)
		return;
	unsigned int res = value_l - 1;
	unsigned int x = 0;
	unsigned int y = 0;
	unsigned int z = 0;
	unsigned int ii;
	for (ii=0; ii<lenx; ++ii)
	{
		x |= (1<<(2*ii) & res)>>(ii);
		y |= (1<<(2*ii+1) & res)>>(ii+1);
	}
	z = res<<(32-lenx*2-lenz)>>(32-lenz);
	x = reverse_bit(x, lenx);
	y = reverse_bit(y, lenx);
	z = reverse_bit(z, lenz);
	unsigned int index2 = nx*ny*z + nx*y + x;
	if (index != index2)
		atomic_xchg(szbuf+index, szbuf[index2]);
}
__kernel void kernel_5(
	__read_only image3d_t data,
	__global unsigned int* szbuf,
	__global unsigned int* label,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz,
	unsigned int thresh,
	__read_only image3d_t mask)
{
	unsigned int i = (unsigned int)(get_global_id(0));
	unsigned int j = (unsigned int)(get_global_id(1));
	unsigned int k = (unsigned int)(get_global_id(2));
	float mask_value = read_imagef(mask, samp, (int4)(i, j, k, 1)).x;
	if (mask_value < 1e-6f)
		return;
	unsigned int index = nx*ny*k + nx*j + i;
	//break if large enough
	if (label[index]==0 ||
		szbuf[index] > thresh)
		return;
	float value = read_imagef(data, samp, (int4)(i, j, k, 1)).x;
	unsigned int nb_index;
	unsigned int min_dist = 10;
	unsigned int dist;
	unsigned int max_nb_index;
	float nb_value;
	for (int ni=-1; ni<2; ++ni)
	for (int nj=-1; nj<2; ++nj)
	for (int nk=-1; nk<2; ++nk)
	{
		if ((i==0 && ni==-1) ||
			(i==nx-1 && ni==1) ||
			(j==0 && nj==-1) ||
			(j==ny-1 && nj==1) ||
			(k==0 && nk==-1) ||
			(k==nz-1 && nk==1))
			continue;
		mask_value = read_imagef(mask, samp, (int4)(i+ni, j+nj, k+nk, 1)).x;
		if (mask_value < 1e-6f)
			continue;
		nb_index = nx*ny*(k+nk) + nx*(j+nj) + i+ni;
		dist = abs(nk) + abs(nj) + abs(ni);
		if (szbuf[nb_index]>thresh &&
			dist < min_dist)
		{
			nb_value = read_imagef(data, samp, (int4)(i+ni, j+nj, k+nk, 1)).x;
			if (nb_value < value)
				continue;
			min_dist = dist;
			max_nb_index = nb_index;
		}
	}
	if (min_dist < 10)
		atomic_xchg(label+index, label[max_nb_index]);
}
)CLKER";

inline constexpr const char* str_cl_clear_borders_3d = R"CLKER(
const sampler_t samp =
	CLK_NORMALIZED_COORDS_FALSE|
	CLK_ADDRESS_CLAMP_TO_EDGE|
	CLK_FILTER_NEAREST;


__kernel void kernel_0(
	__global unsigned int* label,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz)
{
	unsigned int x, y, z;
	unsigned int i = (unsigned int)(get_global_id(0));
	unsigned int j = (unsigned int)(get_global_id(1));
	unsigned int k = (unsigned int)(get_global_id(2));
	unsigned int index = nx*ny*k + nx*j + i;
	if (i == 0 || i == nx-1 ||
		j == 0 || j == ny-1 ||
		k == 0 || k == nz-1)
		label[index] = 0;
}
__kernel void kernel_1(
	__global unsigned int* label,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz,
	__read_only image3d_t mask)
{
	unsigned int x, y, z;
	unsigned int i = (unsigned int)(get_global_id(0));
	unsigned int j = (unsigned int)(get_global_id(1));
	unsigned int k = (unsigned int)(get_global_id(2));
	float mask_value = read_imagef(mask, samp, (int4)(i, j, k, 1)).x;
	if (mask_value < 1e-6f)
		return;
	unsigned int index = nx*ny*k + nx*j + i;
	if (i == 0 || i == nx-1 ||
		j == 0 || j == ny-1 ||
		k == 0 || k == nz-1)
		label[index] = 0;
}
)CLKER";

inline constexpr const char* str_cl_fill_borders_3d = R"CLKER(
const sampler_t samp =
	CLK_NORMALIZED_COORDS_FALSE|
	CLK_ADDRESS_CLAMP_TO_EDGE|
	CLK_FILTER_NEAREST;

__kernel void kernel_0(
	__read_only image3d_t data,
	__global unsigned int* label,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz,
	float tol)
{
	unsigned int i = (unsigned int)(get_global_id(0));
	unsigned int j = (unsigned int)(get_global_id(1));
	unsigned int k = (unsigned int)(get_global_id(2));
	if (nx > 1 && i == 0)
	{
		float value = read_imagef(data, samp, (int4)(i, j, k, 1)).x;
		float nb_value = read_imagef(data, samp, (int4)(i+1, j, k, 1)).x;
		if (fabs(value - nb_value) < tol)
		{
			unsigned int index = nx*ny*k + nx*j + i;
			unsigned int nb_index = index + 1;
			atomic_xchg(label+index, label[nb_index]);
		}
	}
	if (ny > 1 && j == 0)
	{
		float value = read_imagef(data, samp, (int4)(i, j, k, 1)).x;
		float nb_value = read_imagef(data, samp, (int4)(i, j+1, k, 1)).x;
		if (fabs(value - nb_value) < tol)
		{
			unsigned int index = nx*ny*k + nx*j + i;
			unsigned int nb_index = index + nx;
			atomic_xchg(label+index, label[nb_index]);
		}
	}
	if (nz > 1 && k == 0)
	{
		float value = read_imagef(data, samp, (int4)(i, j, k, 1)).x;
		float nb_value = read_imagef(data, samp, (int4)(i, j, k+1, 1)).x;
		if (fabs(value - nb_value) < tol)
		{
			unsigned int index = nx*ny*k + nx*j + i;
			unsigned int nb_index = index + nx*ny;
			atomic_xchg(label+index, label[nb_index]);
		}
	}
}
__kernel void kernel_1(
	__read_only image3d_t data,
	__global unsigned int* label,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz,
	float tol,
	__read_only image3d_t mask)
{
	unsigned int i = (unsigned int)(get_global_id(0));
	unsigned int j = (unsigned int)(get_global_id(1));
	unsigned int k = (unsigned int)(get_global_id(2));
	float mask_value = read_imagef(mask, samp, (int4)(i, j, k, 1)).x;
	if (mask_value < 1e-6f)
		return;
	if (nx > 1 && i == 0)
	{
		mask_value = read_imagef(mask, samp, (int4)(i+1, j, k, 1)).x;
		if (mask_value > 1e-6f)
		{
			float value = read_imagef(data, samp, (int4)(i, j, k, 1)).x;
			float nb_value = read_imagef(data, samp, (int4)(i+1, j, k, 1)).x;
			if (fabs(value - nb_value) < tol)
			{
				unsigned int index = nx*ny*k + nx*j + i;
				unsigned int nb_index = index + 1;
				atomic_xchg(label+index, label[nb_index]);
			}
		}
	}
	if (ny > 1 && j == 0)
	{
		mask_value = read_imagef(mask, samp, (int4)(i, j+1, k, 1)).x;
		if (mask_value > 1e-6f)
		{
			float value = read_imagef(data, samp, (int4)(i, j, k, 1)).x;
			float nb_value = read_imagef(data, samp, (int4)(i, j+1, k, 1)).x;
			if (fabs(value - nb_value) < tol)
			{
				unsigned int index = nx*ny*k + nx*j + i;
				unsigned int nb_index = index + nx;
				atomic_xchg(label+index, label[nb_index]);
			}
		}
	}
	if (nz > 1 && k == 0)
	{
		mask_value = read_imagef(mask, samp, (int4)(i, j, k+1, 1)).x;
		if (mask_value > 1e-6f)
		{
			float value = read_imagef(data, samp, (int4)(i, j, k, 1)).x;
			float nb_value = read_imagef(data, samp, (int4)(i, j, k+1, 1)).x;
			if (fabs(value - nb_value) < tol)
			{
				unsigned int index = nx*ny*k + nx*j + i;
				unsigned int nb_index = index + nx*ny;
				atomic_xchg(label+index, label[nb_index]);
			}
		}
	}
}
)CLKER";

#ifdef _DEBUG
inline constexpr const char* str_cl_slice_brainbow = R"CLKER(
const sampler_t samp =
	CLK_NORMALIZED_COORDS_FALSE|
	CLK_ADDRESS_CLAMP_TO_EDGE|
	CLK_FILTER_NEAREST;

float2 vol_grad_func(image2d_t image, int2 pos)
{
	float2 grad1;
	float2 grad2;
	grad1.x = read_imagef(image, samp, pos+(int2)(1, 0)).x-
		read_imagef(image, samp, pos+(int2)(-1, 0)).x;
	grad1.y = read_imagef(image, samp, pos+(int2)(0, 1)).x-
		read_imagef(image, samp, pos+(int2)(0, -1)).x;
	grad2.x = read_imagef(image, samp, pos+(int2)(1, 1)).x-
		read_imagef(image, samp, pos+(int2)(-1, -1)).x;
	grad2.y = read_imagef(image, samp, pos+(int2)(1, -1)).x-
		read_imagef(image, samp, pos+(int2)(-1, 1)).x;
	//rotate
	float2 grad2r;
	grad2r.x = dot(grad2, (float2)(-0.707f, 0.707f));
	grad2r.y = dot(grad2, (float2)(-0.707f, -0.707f));
	return 0.586f*grad1 + 0.414f*grad2r;
}

__kernel void kernel_0(
	__read_only image2d_t data,
	__global unsigned int* label,
	unsigned int nx,
	unsigned int ny,
	__global unsigned int* rcnt,
	unsigned int seed,
	float value_t,
	float value_f,
	float grad_f,
	float vv_f,
	float av_f)
{
	int2 coord = (int2)(get_global_id(0),
		get_global_id(1));
	unsigned int index = nx*coord.y + coord.x;
	unsigned int label_v = label[index];
	if (label_v == 0)
		return;
	float value = read_imagef(data, samp, coord).x;
	float grad = length(vol_grad_func(data, coord));
	//measures
	int2 nb_coord;
	float nb_value;
	float avg_value = 0.0f;
	float2 nb_grad;
	float2 avg_grad;
	for (int i=-1; i<2; ++i)
	for (int j=-1; j<2; ++j)
	{
		nb_coord = (int2)(coord.x+i, coord.y+j);
		nb_value = read_imagef(data, samp, nb_coord).x;
		avg_value += nb_value;
		nb_grad = vol_grad_func(data, nb_coord);
		avg_grad += nb_grad;
	}
	avg_value /= 9.0f;
	avg_grad = normalize(avg_grad);
	float value_var = 0.0f;
	float angle_var = 0.0f;
	for (int i=-2; i<3; ++i)
	for (int j=-2; j<3; ++j)
	{
		nb_coord = (int2)(coord.x+i, coord.y+j);
		nb_value = read_imagef(data, samp, nb_coord).x;
		nb_grad = vol_grad_func(data, nb_coord);
		value_var += fabs(nb_value - avg_value);
		angle_var += length(nb_grad)*(1.0f-dot(avg_grad,
			normalize(nb_grad))/2.0f);
	}
	value_var /= 25.0f;
	angle_var /= 25.0f;
	
	//stop function
	float stop =
		(grad_f>0.0f?(grad>sqrt(grad_f)*2.12f?0.0f:exp(-grad*grad/grad_f)):1.0f)*
		(value>value_t?1.0f:(value_f>0.0f?(value<value_t-sqrt(value_f)*2.12f?0.0f:exp(-(value-value_t)*(value-value_t)/value_f)):0.0f))*
		(vv_f>0.0f?(value_var>sqrt(vv_f)*2.12f?0.0f:exp(-value_var*value_var/vv_f)):1.0f)*
		(vv_f>0.0f?(avg_value>sqrt(vv_f)*2.12f?0.0f:exp(-avg_value*avg_value/vv_f)):1.0f)*
		(av_f>0.0f?(angle_var>sqrt(av_f)*2.12f?0.0f:exp(-angle_var*angle_var/av_f)):1.0f);
	
	//max filter
	atomic_inc(rcnt);
	float random = (float)((*rcnt) % seed)/(float)(seed)+1e-4f;
	if (stop < random)
		return;
	int2 max_nb_coord = coord;
	unsigned int nb_index;
	unsigned int m;
	for (int i=-1; i<2; ++i)
	for (int j=-1; j<2; ++j)
	{
		nb_coord = (int2)(coord.x+i, coord.y+j);
		nb_index = nx*nb_coord.y + nb_coord.x;
		m = label[nb_index];
		if (m > label_v)
		{
			label_v = m;
			max_nb_coord = nb_coord;
		}
	}
	if (grad_f > 0.0f)
	{
		float xc = value;
		float xn = read_imagef(data, samp, max_nb_coord).x + grad_f;
		if (xn < xc || xn - xc > 2.0f*grad_f)
			return;
	}

	label[index] = label_v;
}
)CLKER";

inline constexpr const char* str_cl_brainbow_3d_sized = R"CLKER(
const sampler_t samp =
	CLK_NORMALIZED_COORDS_FALSE|
	CLK_ADDRESS_CLAMP_TO_EDGE|
	CLK_FILTER_NEAREST;

float3 vol_grad_func(image3d_t image, int4 pos)
{
	float3 grad;
	grad.x = read_imagef(image, samp, pos+(int4)(1, 0, 0, 0)).x-
		read_imagef(image, samp, pos+(int4)(-1, 0, 0, 0)).x;
	grad.y = read_imagef(image, samp, pos+(int4)(0, 1, 0, 0)).x-
		read_imagef(image, samp, pos+(int4)(0, -1, 0, 0)).x;
	grad.z = read_imagef(image, samp, pos+(int4)(0, 0, 1, 0)).x-
		read_imagef(image, samp, pos+(int4)(0, 0, -1, 0)).x;
	return grad;
}

unsigned int __attribute((always_inline)) reverse_bit(unsigned int val, unsigned int len)
{
	unsigned int res = val;
	int s = len - 1;
	for (val >>= 1; val; val >>= 1)
	{
		res <<= 1;
		res |= val & 1;
		s--;
	}
	res <<= s;
	res <<= 32-len;
	res >>= 32-len;
	return res;
}
float get_2d_density(image3d_t image, int4 pos, int r)
{
	float sum = 0.0f;
	int d = 2*r+1;
	for (int i=-r; i<=r; ++i)
	for (int j=-r; j<=r; ++j)
		sum += read_imagef(image, samp, pos+(int4)(i, j, 0, 0)).x;
	return sum / (float)(d * d);
}
__kernel void kernel_0(
	__global unsigned int* mask,
	__global unsigned int* label,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz,
	unsigned int lenx,
	unsigned int lenz)
{
	unsigned int i = (unsigned int)(get_global_id(0));
	unsigned int j = (unsigned int)(get_global_id(1));
	unsigned int k = (unsigned int)(get_global_id(2));
	unsigned int index = nx*ny*k + nx*j + i;
	unsigned int value_l = label[index];
	if (value_l == 0)
		return;
	unsigned int res = value_l - 1;
	unsigned int x = 0;
	unsigned int y = 0;
	unsigned int z = 0;
	unsigned int ii;
	for (ii=0; ii<lenx; ++ii)
	{
		x |= (1<<(2*ii) & res)>>(ii);
		y |= (1<<(2*ii+1) & res)>>(ii+1);
	}
	z = res<<(32-lenx*2-lenz)>>(32-lenz);
	x = reverse_bit(x, lenx);
	y = reverse_bit(y, lenx);
	z = reverse_bit(z, lenz);
	index = nx*ny*z + nx*y + x;
	atomic_inc(mask+index);
}
__kernel void kernel_1(
	__global unsigned int* mask,
	__global unsigned int* label,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz,
	unsigned int lenx,
	unsigned int lenz)
{
	unsigned int i = (unsigned int)(get_global_id(0));
	unsigned int j = (unsigned int)(get_global_id(1));
	unsigned int k = (unsigned int)(get_global_id(2));
	unsigned int index = nx*ny*k + nx*j + i;
	unsigned int value_l = label[index];
	if (value_l == 0)
		return;
	unsigned int res = value_l - 1;
	unsigned int x = 0;
	unsigned int y = 0;
	unsigned int z = 0;
	unsigned int ii;
	for (ii=0; ii<lenx; ++ii)
	{
		x |= (1<<(2*ii) & res)>>(ii);
		y |= (1<<(2*ii+1) & res)>>(ii+1);
	}
	z = res<<(32-lenx*2-lenz)>>(32-lenz);
	x = reverse_bit(x, lenx);
	y = reverse_bit(y, lenx);
	z = reverse_bit(z, lenz);
	unsigned int index2 = nx*ny*z + nx*y + x;
	if (index != index2)
		atomic_xchg(mask+index, mask[index2]);
}
__kernel void kernel_2(
	__read_only image3d_t data,
	__global unsigned int* mask,
	__global unsigned int* label,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz,
	__global unsigned int* rcnt,
	unsigned int seed,
	float value_t,
	float value_f,
	float grad_f,
	unsigned int thresh,
	float density,
	int dsize)
{
	int3 coord = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	unsigned int index = nx*ny*coord.z + nx*coord.y + coord.x;
	//break if large enough
	if (mask[index] > thresh)
		return;
	//break if low density
	if (density > 0.0f && dsize > 0 &&
		get_2d_density(data, (int4)(coord, 1), dsize) < density)
		return;
	unsigned int label_v = label[index];
	if (label_v == 0)
		return;
	float value = read_imagef(data, samp, (int4)(coord, 1)).x;
	float grad = length(vol_grad_func(data, (int4)(coord, 1)));
	//stop function
	float stop =
		(grad_f>0.0f?(grad>sqrt(grad_f)*2.12f?0.0f:exp(-grad*grad/grad_f)):1.0f)*
		(value>value_t?1.0f:(value_f>0.0f?(value<value_t-sqrt(value_f)*2.12f?0.0f:exp(-(value-value_t)*(value-value_t)/value_f)):0.0f));
	
	//max filter
	atomic_inc(rcnt);
	float random = (float)((*rcnt) % seed)/(float)(seed)+1e-4f;
	if (stop < random)
		return;
	int3 nb_coord;
	unsigned int nb_index;
	unsigned int m;
	for (int i=-1; i<2; ++i)
	for (int j=-1; j<2; ++j)
	for (int k=-1; k<2; ++k)
	{
		nb_coord = (int3)(coord.x+i, coord.y+j, coord.z+k);
		if (nb_coord.x < 0 || nb_coord.x > nx-1 ||
			nb_coord.y < 0 || nb_coord.y > ny-1 ||
			nb_coord.z < 0 || nb_coord.z > nz-1)
			continue;
		nb_index = nx*ny*nb_coord.z + nx*nb_coord.y + nb_coord.x;
		m = label[nb_index];
		if (m > label_v)
			label_v = m;
	}
	label[index] = label_v;
}
)CLKER";

inline constexpr const char* str_cl_clear_borders_2d = R"CLKER(
const sampler_t samp =
	CLK_NORMALIZED_COORDS_FALSE|
	CLK_ADDRESS_CLAMP_TO_EDGE|
	CLK_FILTER_NEAREST;


__kernel void kernel_0(
	__global unsigned int* label,
	unsigned int nx,
	unsigned int ny)
{
	unsigned int x, y;
	unsigned int i = (unsigned int)(get_global_id(0));
	unsigned int j = (unsigned int)(get_global_id(1));
	unsigned int index = nx*j + i;
	if (i == 0 || i == nx-1 ||
		j == 0 || j == ny-1)
		label[index] = 0;
}
)CLKER";

inline constexpr const char* str_cl_fill_borders_2d = R"CLKER(
const sampler_t samp =
	CLK_NORMALIZED_COORDS_FALSE|
	CLK_ADDRESS_CLAMP_TO_EDGE|
	CLK_FILTER_NEAREST;

__kernel void kernel_0(
	__read_only image2d_t data,
	__global unsigned int* label,
	unsigned int nx,
	unsigned int ny,
	float tol)
{
	unsigned int i = (unsigned int)(get_global_id(0));
	unsigned int j = (unsigned int)(get_global_id(1));
	if (i == 0)
	{
		float value = read_imagef(data, samp, (int2)(i, j)).x;
		float nb_value = read_imagef(data, samp, (int2)(i+1, j)).x;
		if (fabs(value - nb_value) < tol)
		{
			unsigned int index = nx*j + i;
			unsigned int nb_index = index + 1;
			atomic_xchg(label+index, label[nb_index]);
		}
	}
	if (j == 0)
	{
		float value = read_imagef(data, samp, (int2)(i, j)).x;
		float nb_value = read_imagef(data, samp, (int2)(i, j+1)).x;
		if (fabs(value - nb_value) < tol)
		{
			unsigned int index = nx*j + i;
			unsigned int nb_index = index + nx;
			atomic_xchg(label+index, label[nb_index]);
		}
	}
}
)CLKER";

inline constexpr const char* str_cl_order_id_2d = R"CLKER(
const sampler_t samp =
	CLK_NORMALIZED_COORDS_FALSE|
	CLK_ADDRESS_CLAMP_TO_EDGE|
	CLK_FILTER_NEAREST;


__kernel void kernel_0(
	__read_only image3d_t data,
	__global unsigned int* label,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz)
{
	unsigned int i = (unsigned int)(get_global_id(0));
	unsigned int j = (unsigned int)(get_global_id(1));
	unsigned int k = (unsigned int)(get_global_id(2));
	unsigned int index = nx*ny*k + nx*j + i;
	float value = read_imagef(data, samp, (int4)(i, j, k, 1)).x;
	if (value < 1e-4f)
		label[index] = 0;
	else if (i<1 || i>nx-2 ||
			j<1 || j>ny-2)
		label[index] = 0;
	else
		label[index] = index + 1;
}
)CLKER";

inline constexpr const char* str_cl_shuffle_id_2d = R"CLKER(
const sampler_t samp =
	CLK_NORMALIZED_COORDS_FALSE|
	CLK_ADDRESS_CLAMP_TO_EDGE|
	CLK_FILTER_NEAREST;

unsigned int __attribute((always_inline)) reverse_bit(unsigned int val, unsigned int len)
{
	unsigned int res = val;
	int s = len - 1;
	for (val >>= 1; val; val >>= 1)
	{
		res <<= 1;
		res |= val & 1;
		s--;
	}
	res <<= s;
	res <<= 32-len;
	res >>= 32-len;
	return res;
}

__kernel void kernel_0(
	__read_only image3d_t data,
	__global unsigned int* label,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz,
	unsigned int len)
{
	unsigned int x, y, res, ii;
	unsigned int i = (unsigned int)(get_global_id(0));
	unsigned int j = (unsigned int)(get_global_id(1));
	unsigned int k = (unsigned int)(get_global_id(2));
	unsigned int index = nx*ny*k + nx*j + i;
	float value = read_imagef(data, samp, (int4)(i, j, k, 1)).x;
	if (value < 1e-4f)
		label[index] = 0;
	else if (i<1 || i>nx-2 ||
			j<1 || j>ny-2)
		label[index] = 0;
	else
	{
		x = reverse_bit(i, len);
		y = reverse_bit(j, len);
		res = 0;
		res |= k<<(2*len);
		for (ii=0; ii<len; ++ii)
		{
			res |= (1<<ii & x)<<(ii);
			res |= (1<<ii & y)<<(ii+1);
		}
		label[index] = nx * ny -res;
	}
}
)CLKER";

inline constexpr const char* str_cl_grow_size = R"CLKER(
const sampler_t samp =
	CLK_NORMALIZED_COORDS_FALSE|
	CLK_ADDRESS_CLAMP_TO_EDGE|
	CLK_FILTER_NEAREST;

float2 vol_grad_func(image2d_t image, int2 pos)
{
	float2 grad1;
	float2 grad2;
	grad1.x = read_imagef(image, samp, pos+(int2)(1, 0)).x-
		read_imagef(image, samp, pos+(int2)(-1, 0)).x;
	grad1.y = read_imagef(image, samp, pos+(int2)(0, 1)).x-
		read_imagef(image, samp, pos+(int2)(0, -1)).x;
	grad2.x = read_imagef(image, samp, pos+(int2)(1, 1)).x-
		read_imagef(image, samp, pos+(int2)(-1, -1)).x;
	grad2.y = read_imagef(image, samp, pos+(int2)(1, -1)).x-
		read_imagef(image, samp, pos+(int2)(-1, 1)).x;
	//rotate
	float2 grad2r;
	grad2r.x = dot(grad2, (float2)(-0.707f, 0.707f));
	grad2r.y = dot(grad2, (float2)(-0.707f, -0.707f));
	return 0.586f*grad1 + 0.414f*grad2r;
}

unsigned int __attribute((always_inline)) reverse_bit(unsigned int val, unsigned int len)
{
	unsigned int res = val;
	int s = len - 1;
	for (val >>= 1; val; val >>= 1)
	{
		res <<= 1;
		res |= val & 1;
		s--;
	}
	res <<= s;
	res <<= 32-len;
	res >>= 32-len;
	return res;
}
__kernel void kernel_0(
	__global unsigned int* mask,
	__global unsigned int* label,
	unsigned int nx,
	unsigned int ny,
	unsigned int len)
{
	unsigned int i = (unsigned int)(get_global_id(0));
	unsigned int j = (unsigned int)(get_global_id(1));
	unsigned int index = nx*j + i;
	unsigned int value_l = label[index];
	if (value_l == 0)
		return;
	unsigned int res = nx*ny - value_l;
	unsigned int x = 0;
	unsigned int y = 0;
	unsigned int ii;
	for (ii=0; ii<len; ++ii)
	{
		x |= (1<<(2*ii) & res)>>(ii);
		y |= (1<<(2*ii+1) & res)>>(ii+1);
	}
	x = reverse_bit(x, len);
	y = reverse_bit(y, len);
	index = nx*y + x;
	atomic_inc(mask+index);
}
__kernel void kernel_1(
	__global unsigned int* mask,
	__global unsigned int* label,
	unsigned int nx,
	unsigned int ny,
	unsigned int len)
{
	unsigned int i = (unsigned int)(get_global_id(0));
	unsigned int j = (unsigned int)(get_global_id(1));
	unsigned int index = nx*j + i;
	unsigned int value_l = label[index];
	if (value_l == 0)
		return;
	unsigned int res = nx*ny - value_l;
	unsigned int x = 0;
	unsigned int y = 0;
	unsigned int ii;
	for (ii=0; ii<len; ++ii)
	{
		x |= (1<<(2*ii) & res)>>(ii);
		y |= (1<<(2*ii+1) & res)>>(ii+1);
	}
	x = reverse_bit(x, len);
	y = reverse_bit(y, len);
	unsigned int index2 = nx*y + x;
	if (index != index2)
		atomic_xchg(mask+index, mask[index2]);
}
__kernel void kernel_2(
	__read_only image2d_t data,
	__global unsigned int* mask,
	__global unsigned int* label,
	unsigned int nx,
	unsigned int ny,
	__global unsigned int* rcnt,
	unsigned int seed,
	float value_t,
	float value_f,
	float grad_f,
	float vv_f,
	float av_f,
	unsigned int thresh)
{
	int2 coord = (int2)(get_global_id(0),
		get_global_id(1));
	unsigned int index = nx*coord.y + coord.x;
	unsigned int label_v = label[index];
	if (label_v == 0)
		return;
	//break if large enough
	if (mask[index] > thresh)
		return;
	atomic_inc(rcnt);
	float value = read_imagef(data, samp, coord).x;
	float grad = length(vol_grad_func(data, coord));
	//measures
	int2 nb_coord;
	float nb_value;
	float avg_value = 0.0f;
	float2 nb_grad;
	float2 avg_grad;
	for (int i=-1; i<2; ++i)
	for (int j=-1; j<2; ++j)
	{
		nb_coord = (int2)(coord.x+i, coord.y+j);
		nb_value = read_imagef(data, samp, nb_coord).x;
		avg_value += nb_value;
		nb_grad = vol_grad_func(data, nb_coord);
		avg_grad += nb_grad;
	}
	avg_value /= 9.0f;
	avg_grad = normalize(avg_grad);
	float value_var = 0.0f;
	float angle_var = 0.0f;
	for (int i=-2; i<3; ++i)
	for (int j=-2; j<3; ++j)
	{
		nb_coord = (int2)(coord.x+i, coord.y+j);
		nb_value = read_imagef(data, samp, nb_coord).x;
		nb_grad = vol_grad_func(data, nb_coord);
		value_var += fabs(nb_value - avg_value);
		angle_var += length(nb_grad)*(1.0f-dot(avg_grad,
			normalize(nb_grad))/2.0f);
	}
	value_var /= 25.0f;
	angle_var /= 25.0f;
	
	//stop function
	float stop =
		(grad_f>0.0f?(grad>sqrt(grad_f)*2.12f?0.0f:exp(-grad*grad/grad_f)):1.0f)*
		(value>value_t?1.0f:(value_f>0.0f?(value<value_t-sqrt(value_f)*2.12f?0.0f:exp(-(value-value_t)*(value-value_t)/value_f)):0.0f))*
		(vv_f>0.0f?(value_var>sqrt(vv_f)*2.12f?0.0f:exp(-value_var*value_var/vv_f)):1.0f)*
		(vv_f>0.0f?(avg_value>sqrt(vv_f)*2.12f?0.0f:exp(-avg_value*avg_value/vv_f)):1.0f)*
		(av_f>0.0f?(angle_var>sqrt(av_f)*2.12f?0.0f:exp(-angle_var*angle_var/av_f)):1.0f);
	
	//max filter
	float random = (float)((*rcnt) % seed)/(float)(seed)+1e-4f;
	if (stop < random)
		return;
	int2 max_nb_coord = coord;
	unsigned int nb_index;
	unsigned int m;
	for (int i=-1; i<2; ++i)
	for (int j=-1; j<2; ++j)
	{
		nb_coord = (int2)(coord.x+i, coord.y+j);
		nb_index = nx*nb_coord.y + nb_coord.x;
		m = label[nb_index];
		if (m > label_v)
		{
			label_v = m;
			max_nb_coord = nb_coord;
		}
	}
	if (grad_f > 0.0f)
	{
		float xc = value;
		float xn = read_imagef(data, samp, max_nb_coord).x + grad_f;
		if (xn < xc || xn - xc > 2.0f*grad_f)
			return;
	}

	label[index] = label_v;
}
)CLKER";

inline constexpr const char* str_cl_clean_up = R"CLKER(
__kernel void kernel_0(
	__global unsigned int* mask,
	__global unsigned int* label,
	unsigned int nx,
	unsigned int ny,
	unsigned int thresh)
{
	unsigned int i = (unsigned int)(get_global_id(0));
	unsigned int j = (unsigned int)(get_global_id(1));
	unsigned int index = nx*j + i;
	//break is large enough
	if (label[index]==0 ||
		mask[index] > thresh)
		return;
	unsigned int nb_index;
	unsigned int max_size = 0;
	unsigned int max_nb_index;
	for (int ni=-1; ni<2; ++ni)
	for (int nj=-1; nj<2; ++nj)
	{
		nb_index = nx*(j+nj) + i+ni;
		if (mask[nb_index]>thresh &&
			mask[nb_index]>max_size)
		{
			max_size = mask[nb_index];
			max_nb_index = nb_index;
		}
	}
	if (max_size > 0)
		atomic_xchg(label+index, label[max_nb_index]);
}
)CLKER";

inline constexpr const char* str_cl_match_slices = R"CLKER(
__kernel void kernel_0(
	__global unsigned int* mask,
	__global unsigned int* label1,
	__global unsigned int* label2,
	__global int* flag,
	unsigned int nx,
	unsigned int ny)
{
	unsigned int i = (unsigned int)(get_global_id(0));
	unsigned int j = (unsigned int)(get_global_id(1));
	unsigned int index = nx*j + i;
	if (flag[index])
		return;
	unsigned int value_l1 = label1[index];
	unsigned int value_l2 = label2[index];
	if (!value_l1 || !value_l2)
		return;
	unsigned int svl1, svl2, sidx;
	unsigned int size = 0;
	for (unsigned int ii=0; ii<nx; ++ii)
	for (unsigned int jj=0; jj<ny; ++jj)
	{
		sidx = nx*jj + ii;
		svl1 = label1[sidx];
		svl2 = label2[sidx];
		if (svl1!=value_l1 || svl2!=value_l2)
			continue;
		if (flag[sidx])
			return;
		flag[sidx] = true;
		size++;
	}
	for (unsigned int ii=0; ii<nx; ++ii)
	for (unsigned int jj=0; jj<ny; ++jj)
	{
		sidx = nx*jj + ii;
		svl1 = label1[sidx];
		svl2 = label2[sidx];
		if (svl1!=value_l1 || svl2!=value_l2)
			continue;
		mask[sidx] = size;
	}
}

__kernel void kernel_1(
	__global unsigned int* mask1,
	__global unsigned int* mask2,
	__global unsigned int* mask_and,
	__global unsigned int* label1,
	__global unsigned int* label2,
	unsigned int nx,
	unsigned int ny,
	unsigned int thresh)
{
	unsigned int i = (unsigned int)(get_global_id(0));
	unsigned int j = (unsigned int)(get_global_id(1));
	unsigned int index = nx*j +i;
	unsigned int value_l1 = label1[index];
	unsigned int value_l2 = label2[index];
	if (!value_l1 || !value_l2 || value_l1==value_l2)
		return;
	unsigned int size1 = mask1[index];
	unsigned int size2 = mask2[index];
	if (size1<=thresh || size2<=thresh)
		return;
	unsigned int size_and = mask_and[index];
	if ((float)size_and/(float)size1 +
		(float)size_and/(float)size2 <= 1.0f)
		return;
	unsigned int sidx;
	for (unsigned int ii=0; ii<nx; ++ii)
	for (unsigned int jj=0; jj<ny; ++jj)
	{
		sidx = nx*jj + ii;
		if (label2[sidx] == value_l2)
			label2[sidx] = value_l1;
	}
}
)CLKER";

#endif

inline constexpr const char* str_cl_comp_gen_db = R"CLKER(
#define IITER 0
#define IVALT 1
#define IBDIF 2
#define IVALF 3
#define IGRDF 3
#define IBDEN 4
#define IDENS 5
#define IVRTH 6
#define IDENSF 7
#define IDENSW 8
#define IBDIST 9
#define IDMIX 10
#define IDISTH 11
#define IDISTF 12
#define IMAXDT 13
#define ICLNSZ 16
const sampler_t samp =
	CLK_NORMALIZED_COORDS_FALSE|
	CLK_ADDRESS_CLAMP_TO_EDGE|
	CLK_FILTER_NEAREST;

float get_2d_density(image3d_t image, int4 pos, int r)
{
	float sum = 0.0f;
	int d = 2*r+1;
#pragma unroll
	for (int i=-r; i<=r; ++i)
#pragma unroll
	for (int j=-r; j<=r; ++j)
		sum += read_imagef(image, samp, pos+(int4)(i, j, 0, 0)).x;
	return sum / (float)(d * d);
}
float3 vol_grad_func(image3d_t image, int4 pos)
{
	float3 grad;
	grad.x = read_imagef(image, samp, pos+(int4)(1, 0, 0, 0)).x-
		read_imagef(image, samp, pos+(int4)(-1, 0, 0, 0)).x;
	grad.y = read_imagef(image, samp, pos+(int4)(0, 1, 0, 0)).x-
		read_imagef(image, samp, pos+(int4)(0, -1, 0, 0)).x;
	grad.z = read_imagef(image, samp, pos+(int4)(0, 0, 1, 0)).x-
		read_imagef(image, samp, pos+(int4)(0, 0, -1, 0)).x;
	return grad;
}

//generate param index by direct hist
__kernel void kernel_0(
	__read_only image3d_t data,
	__global float* rechist,
	__global uchar* lut,
	__local float* lh,
	int3 histxyz,
	int3 nxyz,
	float minv,
	float maxv,
	uchar bin,
	uchar rec)
{
	int3 gid = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	int3 lb = gid - histxyz / 2;
	lb = max(lb, (int3)(0));
	int3 ub = lb + histxyz;
	ub = min(ub, nxyz - (int3)(1));
	lb = min(lb, ub - histxyz);
	uint index;
#pragma unroll
	for (index = 0; index < bin; ++ index)
		lh[index] = 0.0f;
	int3 ijk;
	float val;
	float popl = 0.0f;
#pragma unroll
	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)
#pragma unroll
	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)
#pragma unroll
	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)
	{
		val = read_imagef(data, samp, (int4)(ijk, 1)).x;
		index = (val - minv) * (bin - 1) / (maxv - minv);
		lh[index] += 1.0f;
		popl += 1.0f;
	}
#pragma unroll
	for (index = 0; index < bin; ++ index)
		lh[index] /= popl;
	float f1, f2, fmin;
	uchar r = 0;
	int rhi = 0;
#pragma unroll
	for (int i = 0; i < rec; ++i)
	{
		f1 = 0.0f;
#pragma unroll
		for (int j = 0; j < bin; ++j)
		{
			f2 = lh[j] - rechist[rhi+j];
			f1 += f2 * f2;
		}
		rhi += bin;
		fmin = i ? min(f1, fmin) : f1;
		r = f1 == fmin? (uchar)(i) : r;
	}
	index = nxyz.x*nxyz.y*gid.z + nxyz.x*gid.y + gid.x;
	lut[index] = r;
}

//init dist field by db lookup
__kernel void kernel_1(
	__read_only image3d_t data,
	__global uchar* lut,
	__global float* params,
	__global uchar* df,
	float sscale,
	uchar ini,
	int3 nxyz,
	uint nxy,
	uint npar)
{
	int3 ijk = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	uint index = nxy*ijk.z + nxyz.x*ijk.y + ijk.x;
	if (ijk.x == 0 || ijk.x == nxyz.x-1 ||
		ijk.y == 0 || ijk.y == nxyz.y-1)
	{
		df[index] = 0;
		return;
	}
	uint lutr = (uint)(lut[index]) * npar;
	int dsize = (int)(params[lutr + IDISTF]);
	float dth = params[lutr + IDISTH];
	float dval = get_2d_density(data, (int4)(ijk, 1), dsize);
	dval *= sscale;
	if (dval > dth)
		df[index] = ini;
	else
		df[index] = 0;
}

//generate dist field
__kernel void kernel_2(
	__global uchar* df,
	uchar ini,
	int3 nxyz,
	uint nxy,
	uchar nn,
	uchar re)
{
	int3 ijk = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	uint index = nxy*ijk.z + nxyz.x*ijk.y + ijk.x;
	if (df[index] == ini)
	{
		short v1 = df[nxy*ijk.z + nxyz.x*ijk.y + ijk.x - 1];
		short v2 = df[nxy*ijk.z + nxyz.x*ijk.y + ijk.x + 1];
		short v3 = df[nxy*ijk.z + nxyz.x*(ijk.y-1) + ijk.x];
		short v4 = df[nxy*ijk.z + nxyz.x*(ijk.y+1) + ijk.x];
		short rre = (ijk.x % 13 + ijk.y % 17) % 4;
		v1 = rre == 0 ? -1 : v1;
		v2 = rre == 3 ? -1 : v2;
		v3 = rre == 1 ? -1 : v3;
		v4 = rre == 2 ? -1 : v4;
		if (v1 == nn || v2 == nn ||
			v3 == nn || v4 == nn)
			df[index] = re;
	}
}

//generate density field mixed with dist field
__kernel void kernel_3(
	__read_only image3d_t data,
	__global uchar* distf,
	__global uchar* densf,
	__global uchar* lut,
	__global float* params,
	float sscale,
	int3 nxyz,
	uint nxy,
	uint npar)
{
	int3 ijk = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	uint index = nxy*ijk.z + nxyz.x*ijk.y + ijk.x;
	uint lutr = (uint)(lut[index]) * npar;
	int dsize = (int)(params[lutr + IDENSF]);
	float distv = params[lutr + IMAXDT];
	distv = 5.0f / distv;
	float bdis = params[lutr + IBDIST];
	float diststr = params[lutr + IDMIX];
	diststr = bdis > 0.1f ? diststr : 0.0f;
	float density = get_2d_density(data, (int4)(ijk, 1), dsize) * sscale;
	distv = distv * diststr * distf[index];
	distv = clamp(distv, 0.0f, 1.0f);
	density = density * (1.0f - diststr) + distv * diststr;
	densf[index] = (uchar)(density * 255.0f);
}

//generate statistics on density field
__kernel void kernel_4(
	__global uchar* df,
	__global uchar* avg,
	__global uchar* var,
	__global uchar* lut,
	__global float* params,
	int3 nxyz,
	uint nxy,
	uint npar)
{
	int3 gid = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	uint index = nxy*gid.z + nxyz.x*gid.y + gid.x;
	uint lutr = (uint)(lut[index]) * npar;
	int3 histxyz = (int3)(params[lutr + IDENSW]);
	int3 lb = gid - histxyz / 2;
	int3 ub = lb + histxyz;
	lb = clamp(lb, (int3)(0), nxyz - (int3)(1));
	ub = clamp(ub, (int3)(0), nxyz);
	int3 ijk = (int3)(0.0f);
	float gnum = 0.0f;
	float sum = 0.0f;
	float sum2 = 0.0f;
	float v;
#pragma unroll
	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)
#pragma unroll
	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)
#pragma unroll
	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)
	{
		index = nxy*ijk.z + nxyz.x*ijk.y + ijk.x;
		v = df[index];
		sum += v;
		sum2 += v * v;
		gnum += 1.0f;
	}
	index = nxy * gid.z + nxyz.x * gid.y + gid.x;
	v = sum / gnum;
	avg[index] = (uchar)(v);
	v = clamp(sqrt((sum2 + v * v * gnum - 2.0f * v * sum) / gnum), 0.0f, 255.0f);
	var[index] = (uchar)(v);
}

//grow by db lookup
__kernel void kernel_5(
	float iter,
	__read_only image3d_t data,
	__global uint* label,
	__global uchar* df,
	__global uchar* avg,
	__global uchar* var,
	__global uint* rcnt,
	__global uchar* lut,
	__global float* params,
	uint seed,
	float sscale,
	int3 nxyz,
	uint nxy,
	uint npar)
{
	int3 coord = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	uint index = nxy*coord.z + nxyz.x*coord.y + coord.x;
	uint lutr = (uint)(lut[index]) * npar;
	if (params[lutr + IITER] < iter) return;
	float bdif = params[lutr + IBDIF];
	float bden = params[lutr + IBDEN];
	float bdis = params[lutr + IBDIST];
	float value_t = params[lutr + IVALT];
	float value_f = params[lutr + IVALF];
	value_f = bdif > 0.1f ? value_f : 0.0f;
	float grad_f = value_f;
	float density = params[lutr + IDENS];
	float varth = params[lutr + IVRTH];
	uint label_v = label[index];
	if (label_v == 0)
		return;
	//use density
	if (density > 0.0f && bden > 0.1f)
	{
		uint index2 = nxy*coord.z + nxyz.x*coord.y + coord.x;
		uchar vdf = df[index2];
		uchar vavg = avg[index2];
		uchar vvar = var[index2];
		//break if low variance
		if (vvar < varth * 255)
			return;
		if (vdf < vavg - (1.0f-density)*vvar)
			return;
	}
	float value = read_imagef(data, samp, (int4)(coord, 1)).x;
	value *= sscale;
	float grad = length(sscale * vol_grad_func(data, (int4)(coord, 1)));
	//stop function
	float stop =
		(grad_f>0.0f?(grad>sqrt(grad_f)*2.12f?0.0f:exp(-grad*grad/grad_f)):1.0f)*
		(value>value_t?1.0f:(value_f>0.0f?(value<value_t-sqrt(value_f)*2.12f?0.0f:exp(-(value-value_t)*(value-value_t)/value_f)):0.0f));
	
	//max filter
	atomic_inc(rcnt);
	float random = (float)((*rcnt) % seed)/(float)(seed)+1e-4f;
	if (stop < random)
		return;
	int3 nb_coord;
	uint nb_index;
	uint m;
#pragma unroll
	for (int i=-1; i<2; ++i)
#pragma unroll
	for (int j=-1; j<2; ++j)
#pragma unroll
	for (int k=-1; k<2; ++k)
	{
		nb_coord = (int3)(coord.x+i, coord.y+j, coord.z+k);
		if (nb_coord.x < 0 || nb_coord.x > nxyz.x-1 ||
			nb_coord.y < 0 || nb_coord.y > nxyz.y-1 ||
			nb_coord.z < 0 || nb_coord.z > nxyz.z-1)
			continue;
		nb_index = nxy*nb_coord.z + nxyz.x*nb_coord.y + nb_coord.x;
		m = label[nb_index];
		if (m <= label_v) continue;
		label_v = m;
	}
	label[index] = label_v;
}
//code for clean up
uint __attribute((always_inline)) reverse_bit(uint val, uint len)
{
	uint res = val;
	int s = len - 1;
	for (val >>= 1; val; val >>= 1)
	{
		res <<= 1;
		res |= val & 1;
		s--;
	}
	res <<= s;
	res <<= 32-len;
	res >>= 32-len;
	return res;
}
//count and store size in szbuf
__kernel void kernel_6(
	__global uint* szbuf,
	__global uint* label,
	int3 nxyz,
	uint nxy,
	int3 lenxyz)
{
	uint3 ijk = (uint3)(get_global_id(0), get_global_id(1), get_global_id(2));
	uint index = nxy*ijk.z + nxyz.x*ijk.y + ijk.x;
	uint value_l = label[index];
	if (value_l == 0)
		return;
	uint res = value_l - 1;
	uint3 xyz = (uint3)(0);
	uint ii;
#pragma unroll
	for (ii=0; ii<lenxyz.x; ++ii)
	{
		xyz.x |= (1<<(2*ii) & res)>>(ii);
		xyz.y |= (1<<(2*ii+1) & res)>>(ii+1);
	}
	xyz.z = res<<(32-lenxyz.x*2-lenxyz.z)>>(32-lenxyz.z);
	xyz.x = reverse_bit(xyz.x, lenxyz.x);
	xyz.y = reverse_bit(xyz.y, lenxyz.x);
	xyz.z = reverse_bit(xyz.z, lenxyz.z);
	index = nxy*xyz.z + nxyz.x*xyz.y + xyz.x;
	atomic_inc(szbuf+index);
}
//set size value to all
__kernel void kernel_7(
	__global uint* szbuf,
	__global uint* label,
	int3 nxyz,
	uint nxy,
	int3 lenxyz)
{
	uint3 ijk = (uint3)(get_global_id(0), get_global_id(1), get_global_id(2));
	uint index = nxy*ijk.z + nxyz.x*ijk.y + ijk.x;
	uint value_l = label[index];
	if (value_l == 0)
		return;
	uint res = value_l - 1;
	uint3 xyz = (uint3)(0);
	uint ii;
#pragma unroll
	for (ii=0; ii<lenxyz.x; ++ii)
	{
		xyz.x |= (1<<(2*ii) & res)>>(ii);
		xyz.y |= (1<<(2*ii+1) & res)>>(ii+1);
	}
	xyz.z = res<<(32-lenxyz.x*2-lenxyz.z)>>(32-lenxyz.z);
	xyz.x = reverse_bit(xyz.x, lenxyz.x);
	xyz.y = reverse_bit(xyz.y, lenxyz.x);
	xyz.z = reverse_bit(xyz.z, lenxyz.z);
	uint index2 = nxy*xyz.z + nxyz.x*xyz.y + xyz.x;
	if (index != index2)
		atomic_xchg(szbuf+index, szbuf[index2]);
}
//size based grow
__kernel void kernel_8(
	__read_only image3d_t data,
	__global uint* szbuf,
	__global uint* label,
	__global uchar* lut,
	__global float* params,
	int3 nxyz,
	uint nxy,
	uint npar)
{
	uint3 ijk = (uint3)(get_global_id(0), get_global_id(1), get_global_id(2));
	uint index = nxy*ijk.z + nxyz.x*ijk.y + ijk.x;
	uint lutr = (uint)(lut[index]) * npar;
	uint thresh = (uint)(params[lutr + ICLNSZ]);
	//break if large enough
	if (label[index]==0 ||
		szbuf[index] > thresh)
		return;
	float value = read_imagef(data, samp, (int4)(convert_int3(ijk), 1)).x;
	uint nb_index;
	float min_dist = 10.0f;
	float dist;
	uint max_nb_index;
	float nb_value;
	int3 nijk;
#pragma unroll
	for (nijk.z=-1; nijk.z<2; ++nijk.z)
#pragma unroll
	for (nijk.y=-1; nijk.y<2; ++nijk.y)
#pragma unroll
	for (nijk.x=-1; nijk.x<2; ++nijk.x)
	{
		if ((ijk.x==0 && nijk.x==-1) ||
			(ijk.x==nxyz.x-1 && nijk.x==1) ||
			(ijk.y==0 && nijk.y==-1) ||
			(ijk.y==nxyz.y-1 && nijk.y==1) ||
			(ijk.z==0 && nijk.z==-1) ||
			(ijk.z==nxyz.z-1 && nijk.z==1))
			continue;
		nb_index = nxy*(ijk.z+nijk.z) + nxyz.x*(ijk.y+nijk.y) + ijk.x+nijk.x;
		dist = abs(nijk.x) + abs(nijk.y) + abs(nijk.z);
		if (szbuf[nb_index]>thresh &&
			dist < min_dist)
		{
			nb_value = read_imagef(data, samp, (int4)(convert_int3(ijk) + nijk, 1)).x;
			if (nb_value < value)
				continue;
			min_dist = dist;
			max_nb_index = nb_index;
		}
	}
	if (min_dist < 10.f)
		atomic_xchg(label+index, label[max_nb_index]);
}
)CLKER";

inline constexpr const char* str_cl_comp_gen_db_unused = R"CLKER(
//compute histogram in blocks
__kernel void kernel_0(
	__read_only image3d_t data,
	unsigned int gsx,
	unsigned int gsy,
	unsigned int gsz,
	unsigned int ngxy,
	unsigned int ngx,
	float minv,
	float maxv,
	unsigned int bin,
	__global unsigned int* hist)
{
	int3 gid = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	int3 lb = (int3)(gid.x*gsx, gid.y*gsy, gid.z*gsz);
	int3 ub = (int3)(lb.x + gsx, lb.y + gsy, lb.z + gsz);
	int3 ijk = (int3)(0);
	uint2 index;//x:hist;y:block
	index.y = ngxy * gid.z + ngx * gid.y + gid.x;
	index.y *= bin + 1;
	float val;
	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)
	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)
	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)
	{
		val = read_imagef(data, samp, (int4)(ijk, 1)).x;
		index.x = (val - minv) * (bin - 1) / (maxv - minv);
		atomic_inc(hist+index.y+index.x);
		atomic_inc(hist+index.y+bin);
	}
}

//generate param index by interpolation
__kernel void kernel_1(
	__read_only image3d_t data,
	__global float* hist,
	__global float* rechist,
	__global uchar* lut,
	__local float* lh,
	int3 gsxyz,
	int3 ngxyz,
	int3 nxyz,
	uchar bin,
	uchar rec)
{
	int3 ijk = (int3)(get_global_id(0),
		get_global_id(1), get_global_id(2));
	uint index = nxyz.x*nxyz.y*ijk.z + nxyz.x*ijk.y + ijk.x;
	ijk -= (int3)(gsxyz.xy / 2, 0);
	int3 hc = ijk / gsxyz;
	int3 hd = ijk % gsxyz;
	int4 hi;
	int3 hc2 = hc;
	hc2 = clamp(hc2, (int3)(0), ngxyz - (int3)(1));
	hi.x = hc2.z * ngxyz.x * ngxyz.y + hc2.y * ngxyz.x + hc2.x;
	hc2 = hc + (int3)(1, 0, 0);
	hc2 = clamp(hc2, (int3)(0), ngxyz - (int3)(1));
	hi.y = hc2.z * ngxyz.x * ngxyz.y + hc2.y * ngxyz.x + hc2.x;
	hc2 = hc + (int3)(0, 1, 0);
	hc2 = clamp(hc2, (int3)(0), ngxyz - (int3)(1));
	hi.z = hc2.z * ngxyz.x * ngxyz.y + hc2.y * ngxyz.x + hc2.x;
	hc2 = hc + (int3)(0, 1, 0);
	hc2 = clamp(hc2, (int3)(0), ngxyz - (int3)(1));
	hi.w = hc2.z * ngxyz.x * ngxyz.y + hc2.y * ngxyz.x + hc2.x;
	hi *= (int)(bin);
	float4 ft;
	ft.x = (float)(hd.x) / (float)(gsxyz.x);
	ft.y = 1.0f - ft.x;
	ft.z = (float)(hd.y) / (float)(gsxyz.y);
	ft.w = 1.0f - ft.z;
	float f1, f2;
	for (int i = 0; i < bin; ++i)
	{
		f1 = hist[hi.x + i] * ft.y + hist[hi.y + i] * ft.x;
		f2 = hist[hi.z + i] * ft.y + hist[hi.w + i] * ft.x;
		lh[i] = f1 * ft.w + f2 * ft.z;
	}
	uchar r = 0;
	float* fp = rechist;
	for (int i = 0; i < rec; ++i)
	{
		f1 = 0.0f;
		for (int j = 0; j < bin; ++j)
			f1 += lh[j] * fp[j];
		fp += bin;
		f2 = i ? max(f1, f2) : f1;
		r = f1 == f2? (uchar)(i) : r;
	}
	lut[index] = r;
}
)CLKER";

#endif//COMP_CL_CODE_H