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
#include "DataManager.h"
#include "SkeletonGenerator.h"
#include <algorithm>
#ifdef _DEBUG
#include <fstream>
#endif

using namespace FL;

const char* str_cl_order_id = \
"const sampler_t samp =\n" \
"	CLK_NORMALIZED_COORDS_FALSE|\n" \
"	CLK_ADDRESS_CLAMP_TO_EDGE|\n" \
"	CLK_FILTER_NEAREST;\n" \
"\n" \
"//use clipping planes, no mask, ascending\n" \
"__kernel void kernel_0(\n" \
"	__read_only image3d_t data,\n" \
"	__global unsigned int* label,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	float4 p0,\n" \
"	float4 p1,\n" \
"	float4 p2,\n" \
"	float4 p3,\n" \
"	float4 p4,\n" \
"	float4 p5,\n" \
"	float3 scl,\n" \
"	float3 trl)\n" \
"{\n" \
"	unsigned int res;\n" \
"	unsigned int x, y, z;\n" \
"	unsigned int i = (unsigned int)(get_global_id(0));\n" \
"	unsigned int j = (unsigned int)(get_global_id(1));\n" \
"	unsigned int k = (unsigned int)(get_global_id(2));\n" \
"	unsigned int index = nx*ny*k + nx*j + i;\n" \
"	float3 pt = (float3)((float)(i) / (float)(nx), (float)(j) / (float)(ny), (float)(k) / (float)(nz));\n" \
"	pt = pt * scl + trl;\n" \
"	if (dot(pt, p0.xyz)+p0.w < 0.0 ||\n" \
"		dot(pt, p1.xyz)+p1.w < 0.0 ||\n" \
"		dot(pt, p2.xyz)+p2.w < 0.0 ||\n" \
"		dot(pt, p3.xyz)+p3.w < 0.0 ||\n" \
"		dot(pt, p4.xyz)+p4.w < 0.0 ||\n" \
"		dot(pt, p5.xyz)+p5.w < 0.0)\n" \
"	{\n" \
"		atomic_xchg(label+index, 0);\n" \
"		return;\n" \
"	}\n" \
"	float value = read_imagef(data, samp, (int4)(i, j, k, 1)).x;\n" \
"	if (value < 0.001)\n" \
"		atomic_xchg(label+index, 0);\n" \
"	else if (i<1 || i>nx-2 ||\n" \
"			j<1 || j>ny-2)\n" \
"		atomic_xchg(label+index, 0);\n" \
"	else\n" \
"	{\n" \
"		res = k * nx * ny + j * nx + i;\n" \
"		atomic_xchg(label+index, res + 1);\n" \
"	}\n" \
"}\n" \
"//use clipping planes, use mask, ascending\n" \
"__kernel void kernel_1(\n" \
"	__read_only image3d_t data,\n" \
"	__global unsigned int* label,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	float4 p0,\n" \
"	float4 p1,\n" \
"	float4 p2,\n" \
"	float4 p3,\n" \
"	float4 p4,\n" \
"	float4 p5,\n" \
"	float3 scl,\n" \
"	float3 trl,\n" \
"	__read_only image3d_t mask)\n" \
"{\n" \
"	unsigned int res;\n" \
"	unsigned int x, y, z;\n" \
"	unsigned int i = (unsigned int)(get_global_id(0));\n" \
"	unsigned int j = (unsigned int)(get_global_id(1));\n" \
"	unsigned int k = (unsigned int)(get_global_id(2));\n" \
"	float mask_value = read_imagef(mask, samp, (int4)(i, j, k, 1)).x;\n" \
"	if (mask_value < 1e-6)\n" \
"		return;\n" \
"	unsigned int index = nx*ny*k + nx*j + i;\n" \
"	float3 pt = (float3)((float)(i) / (float)(nx), (float)(j) / (float)(ny), (float)(k) / (float)(nz));\n" \
"	pt = pt * scl + trl;\n" \
"	if (dot(pt, p0.xyz)+p0.w < 0.0 ||\n" \
"		dot(pt, p1.xyz)+p1.w < 0.0 ||\n" \
"		dot(pt, p2.xyz)+p2.w < 0.0 ||\n" \
"		dot(pt, p3.xyz)+p3.w < 0.0 ||\n" \
"		dot(pt, p4.xyz)+p4.w < 0.0 ||\n" \
"		dot(pt, p5.xyz)+p5.w < 0.0)\n" \
"	{\n" \
"		atomic_xchg(label+index, 0);\n" \
"		return;\n" \
"	}\n" \
"	float value = read_imagef(data, samp, (int4)(i, j, k, 1)).x;\n" \
"	if (value < 0.001)\n" \
"		atomic_xchg(label+index, 0);\n" \
"	else if (i<1 || i>nx-2 ||\n" \
"			j<1 || j>ny-2)\n" \
"		atomic_xchg(label+index, 0);\n" \
"	else\n" \
"	{\n" \
"		res = k * nx * ny + j * nx + i;\n" \
"		atomic_xchg(label+index, res + 1);\n" \
"	}\n" \
"}\n" \
"\n" \
"//use clipping planes, no mask, descending\n" \
"__kernel void kernel_2(\n" \
"	__read_only image3d_t data,\n" \
"	__global unsigned int* label,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	float4 p0,\n" \
"	float4 p1,\n" \
"	float4 p2,\n" \
"	float4 p3,\n" \
"	float4 p4,\n" \
"	float4 p5,\n" \
"	float3 scl,\n" \
"	float3 trl)\n" \
"{\n" \
"	unsigned int res;\n" \
"	unsigned int x, y, z;\n" \
"	unsigned int i = nx - (unsigned int)(get_global_id(0)) - 1;\n" \
"	unsigned int j = ny - (unsigned int)(get_global_id(1)) - 1;\n" \
"	unsigned int k = nz - (unsigned int)(get_global_id(2)) - 1;\n" \
"	unsigned int index = nx*ny*k + nx*j + i;\n" \
"	float3 pt = (float3)((float)(i) / (float)(nx), (float)(j) / (float)(ny), (float)(k) / (float)(nz));\n" \
"	pt = pt * scl + trl;\n" \
"	if (dot(pt, p0.xyz)+p0.w < 0.0 ||\n" \
"		dot(pt, p1.xyz)+p1.w < 0.0 ||\n" \
"		dot(pt, p2.xyz)+p2.w < 0.0 ||\n" \
"		dot(pt, p3.xyz)+p3.w < 0.0 ||\n" \
"		dot(pt, p4.xyz)+p4.w < 0.0 ||\n" \
"		dot(pt, p5.xyz)+p5.w < 0.0)\n" \
"	{\n" \
"		atomic_xchg(label+index, 0);\n" \
"		return;\n" \
"	}\n" \
"	float value = read_imagef(data, samp, (int4)(i, j, k, 1)).x;\n" \
"	if (value < 0.001)\n" \
"		atomic_xchg(label+index, 0);\n" \
"	else if (i<1 || i>nx-2 ||\n" \
"			j<1 || j>ny-2)\n" \
"		atomic_xchg(label+index, 0);\n" \
"	else\n" \
"	{\n" \
"		res = k * nx * ny + j * nx + i;\n" \
"		atomic_xchg(label+index, res + 1);\n" \
"	}\n" \
"}\n" \
"//use clipping planes, use mask, descending\n" \
"__kernel void kernel_3(\n" \
"	__read_only image3d_t data,\n" \
"	__global unsigned int* label,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	float4 p0,\n" \
"	float4 p1,\n" \
"	float4 p2,\n" \
"	float4 p3,\n" \
"	float4 p4,\n" \
"	float4 p5,\n" \
"	float3 scl,\n" \
"	float3 trl,\n" \
"	__read_only image3d_t mask)\n" \
"{\n" \
"	unsigned int res;\n" \
"	unsigned int x, y, z;\n" \
"	unsigned int i = nx - (unsigned int)(get_global_id(0)) - 1;\n" \
"	unsigned int j = ny - (unsigned int)(get_global_id(1)) - 1;\n" \
"	unsigned int k = nz - (unsigned int)(get_global_id(2)) - 1;\n" \
"	float mask_value = read_imagef(mask, samp, (int4)(i, j, k, 1)).x;\n" \
"	if (mask_value < 1e-6)\n" \
"		return;\n" \
"	unsigned int index = nx*ny*k + nx*j + i;\n" \
"	float3 pt = (float3)((float)(i) / (float)(nx), (float)(j) / (float)(ny), (float)(k) / (float)(nz));\n" \
"	pt = pt * scl + trl;\n" \
"	if (dot(pt, p0.xyz)+p0.w < 0.0 ||\n" \
"		dot(pt, p1.xyz)+p1.w < 0.0 ||\n" \
"		dot(pt, p2.xyz)+p2.w < 0.0 ||\n" \
"		dot(pt, p3.xyz)+p3.w < 0.0 ||\n" \
"		dot(pt, p4.xyz)+p4.w < 0.0 ||\n" \
"		dot(pt, p5.xyz)+p5.w < 0.0)\n" \
"	{\n" \
"		atomic_xchg(label+index, 0);\n" \
"		return;\n" \
"	}\n" \
"	float value = read_imagef(data, samp, (int4)(i, j, k, 1)).x;\n" \
"	if (value < 0.001)\n" \
"		atomic_xchg(label+index, 0);\n" \
"	else if (i<1 || i>nx-2 ||\n" \
"			j<1 || j>ny-2)\n" \
"		atomic_xchg(label+index, 0);\n" \
"	else\n" \
"	{\n" \
"		res = k * nx * ny + j * nx + i;\n" \
"		atomic_xchg(label+index, res + 1);\n" \
"	}\n" \
"}\n" \
;

const char* str_cl_grow_local = \
"const sampler_t samp =\n" \
"	CLK_NORMALIZED_COORDS_FALSE|\n" \
"	CLK_ADDRESS_CLAMP_TO_EDGE|\n" \
"	CLK_FILTER_NEAREST;\n" \
"\n" \
"float3 vol_grad_func(image3d_t image, int4 pos)\n" \
"{\n" \
"	float3 grad;\n" \
"	grad.x = read_imagef(image, samp, pos+(int4)(1, 0, 0, 0)).x-\n" \
"		read_imagef(image, samp, pos+(int4)(-1, 0, 0, 0)).x;\n" \
"	grad.y = read_imagef(image, samp, pos+(int4)(0, 1, 0, 0)).x-\n" \
"		read_imagef(image, samp, pos+(int4)(0, -1, 0, 0)).x;\n" \
"	grad.z = read_imagef(image, samp, pos+(int4)(0, 0, 1, 0)).x-\n" \
"		read_imagef(image, samp, pos+(int4)(0, 0, -1, 0)).x;\n" \
"	return grad;\n" \
"}\n" \
"\n" \
"__kernel void kernel_0(\n" \
"	__read_only image3d_t data,\n" \
"	__global unsigned int* label,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	__global unsigned int* rcnt,\n" \
"	unsigned int seed,\n" \
"	float value_t,\n" \
"	float value_f,\n" \
"	float grad_f,\n" \
"	float sscale)\n" \
"{\n" \
"	int3 coord = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	unsigned int index = nx*ny*coord.z + nx*coord.y + coord.x;\n" \
"	unsigned int label_v = label[index];\n" \
"	if (label_v == 0 || label_v & 0x80000000)\n" \
"		return;\n" \
"	atomic_inc(rcnt);\n" \
"	float value = read_imagef(data, samp, (int4)(coord, 1)).x;\n" \
"	value *= sscale;\n" \
"	float grad = length(sscale * vol_grad_func(data, (int4)(coord, 1)));\n" \
"	//stop function\n" \
"	float stop =\n" \
"		(grad_f>0.0f?(grad>sqrt(grad_f)*2.12f?0.0f:exp(-grad*grad/grad_f)):1.0f)*\n" \
"		(value>value_t?1.0f:(value_f>0.0f?(value<value_t-sqrt(value_f)*2.12f?0.0f:exp(-(value-value_t)*(value-value_t)/value_f)):0.0f));\n" \
"	\n" \
"	//max filter\n" \
"	float random = (float)((*rcnt) % seed)/(float)(seed)+0.001f;\n" \
"	if (stop < random)\n" \
"		return;\n" \
"	int3 nb_coord;\n" \
"	unsigned int nb_index;\n" \
"	unsigned int m;\n" \
"	for (int i=-1; i<2; ++i)\n" \
"	for (int j=-1; j<2; ++j)\n" \
"	for (int k=-1; k<2; ++k)\n" \
"	{\n" \
"		nb_coord = (int3)(coord.x+i, coord.y+j, coord.z+k);\n" \
"		if (nb_coord.x < 0 || nb_coord.x > nx-1 ||\n" \
"			nb_coord.y < 0 || nb_coord.y > ny-1 ||\n" \
"			nb_coord.z < 0 || nb_coord.z > nz-1)\n" \
"			continue;\n" \
"		nb_index = nx*ny*nb_coord.z + nx*nb_coord.y + nb_coord.x;\n" \
"		m = label[nb_index];\n" \
"		if (m > label_v)\n" \
"			label_v = m;\n" \
"	}\n" \
"	atomic_xchg(label+index, label_v);\n" \
"}\n" \
"//grow only within mask\n" \
"__kernel void kernel_1(\n" \
"	__read_only image3d_t data,\n" \
"	__global unsigned int* label,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	__global unsigned int* rcnt,\n" \
"	unsigned int seed,\n" \
"	float value_t,\n" \
"	float value_f,\n" \
"	float grad_f,\n" \
"	float sscale,\n" \
"	__read_only image3d_t mask)\n" \
"{\n" \
"	int3 coord = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	float mask_value = read_imagef(mask, samp, (int4)(coord, 1)).x;\n" \
"	if (mask_value < 1e-6)\n" \
"		return;\n" \
"	unsigned int index = nx*ny*coord.z + nx*coord.y + coord.x;\n" \
"	unsigned int label_v = label[index];\n" \
"	if (label_v == 0 || label_v & 0x80000000)\n" \
"		return;\n" \
"	atomic_inc(rcnt);\n" \
"	float value = read_imagef(data, samp, (int4)(coord, 1)).x;\n" \
"	value *= sscale;\n" \
"	float grad = length(sscale * vol_grad_func(data, (int4)(coord, 1)));\n" \
"	//stop function\n" \
"	float stop =\n" \
"		(grad_f>0.0f?(grad>sqrt(grad_f)*2.12f?0.0f:exp(-grad*grad/grad_f)):1.0f)*\n" \
"		(value>value_t?1.0f:(value_f>0.0f?(value<value_t-sqrt(value_f)*2.12f?0.0f:exp(-(value-value_t)*(value-value_t)/value_f)):0.0f));\n" \
"	\n" \
"	//max filter\n" \
"	float random = (float)((*rcnt) % seed)/(float)(seed)+0.001f;\n" \
"	if (stop < random)\n" \
"		return;\n" \
"	int3 nb_coord;\n" \
"	unsigned int nb_index;\n" \
"	unsigned int m;\n" \
"	for (int i=-1; i<2; ++i)\n" \
"	for (int j=-1; j<2; ++j)\n" \
"	for (int k=-1; k<2; ++k)\n" \
"	{\n" \
"		nb_coord = (int3)(coord.x+i, coord.y+j, coord.z+k);\n" \
"		mask_value = read_imagef(mask, samp, (int4)(nb_coord, 1)).x;\n" \
"		if (mask_value < 1e-6)\n" \
"			continue;\n" \
"		if (nb_coord.x < 0 || nb_coord.x > nx-1 ||\n" \
"			nb_coord.y < 0 || nb_coord.y > ny-1 ||\n" \
"			nb_coord.z < 0 || nb_coord.z > nz-1)\n" \
"			continue;\n" \
"		nb_index = nx*ny*nb_coord.z + nx*nb_coord.y + nb_coord.x;\n" \
"		m = label[nb_index];\n" \
"		if (m > label_v)\n" \
"			label_v = m;\n" \
"	}\n" \
"	atomic_xchg(label+index, label_v);\n" \
"}\n" \
;

SkeletonGenerator::SkeletonGenerator(VolumeData* vd)
	: m_vd(vd),
	m_use_mask(false)
{
}

SkeletonGenerator::~SkeletonGenerator()
{
}
