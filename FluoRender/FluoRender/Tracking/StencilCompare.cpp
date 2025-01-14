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

#include <Global.h>
#include <StencilCompare.h>
#ifdef _DEBUG
#include <Debug.h>
#endif

using namespace flrd;

const char* str_cl_stencil = \
"//2d filter 8 bit\n" \
"__kernel void kernel_0(\n" \
"	__global unsigned char* img_in,\n" \
"	__global unsigned char* img_out,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz)\n" \
"{\n" \
"	int3 gid = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	int3 lb = gid - (int3)(1, 1, 0);\n" \
"	int3 ub = gid + (int3)(1, 1, 0);\n" \
"	lb = clamp(lb, (int3)(0), (int3)(nx-1, ny-1, nz-1));\n" \
"	ub = clamp(ub, (int3)(0), (int3)(nx-1, ny-1, nz-1));\n" \
"	int3 ijk;\n" \
"	float sum = 0.0f;\n" \
"	int count = 0;\n" \
"	unsigned int index;\n" \
"#pragma unroll\n" \
"	for (ijk.z = lb.z; ijk.z <= ub.z; ++ijk.z)\n" \
"#pragma unroll\n" \
"	for (ijk.y = lb.y; ijk.y <= ub.y; ++ijk.y)\n" \
"#pragma unroll\n" \
"	for (ijk.x = lb.x; ijk.x <= ub.x; ++ijk.x)\n" \
"	{\n" \
"		index = nx*ny*ijk.z + nx*ijk.y + ijk.x;\n" \
"		sum += img_in[index];\n" \
"		count++;\n" \
"	}\n" \
"	sum = count ? sum / count : sum;\n" \
"	index = nx*ny*gid.z + nx*gid.y + gid.x;\n" \
"	img_out[index] = convert_uchar(sum);\n" \
"}\n"
"//2d filter 16 bit\n" \
"__kernel void kernel_1(\n" \
"	__global unsigned short* img_in,\n" \
"	__global unsigned short* img_out,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz)\n" \
"{\n" \
"	int3 gid = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	int3 lb = gid - (int3)(1, 1, 0);\n" \
"	int3 ub = gid + (int3)(1, 1, 0);\n" \
"	lb = clamp(lb, (int3)(0), (int3)(nx-1, ny-1, nz-1));\n" \
"	ub = clamp(ub, (int3)(0), (int3)(nx-1, ny-1, nz-1));\n" \
"	int3 ijk;\n" \
"	float sum = 0.0f;\n" \
"	int count = 0;\n" \
"	unsigned int index;\n" \
"#pragma unroll\n" \
"	for (ijk.z = lb.z; ijk.z <= ub.z; ++ijk.z)\n" \
"#pragma unroll\n" \
"	for (ijk.y = lb.y; ijk.y <= ub.y; ++ijk.y)\n" \
"#pragma unroll\n" \
"	for (ijk.x = lb.x; ijk.x <= ub.x; ++ijk.x)\n" \
"	{\n" \
"		index = nx*ny*ijk.z + nx*ijk.y + ijk.x;\n" \
"		sum += img_in[index];\n" \
"		count++;\n" \
"	}\n" \
"	sum = count ? sum / count : sum;\n" \
"	index = nx*ny*gid.z + nx*gid.y + gid.x;\n" \
"	img_out[index] = convert_ushort(sum);\n" \
"}\n"
"//compare0 8 bit\n" \
"__kernel void kernel_2(\n" \
"	__global unsigned char* img1,\n" \
"	__global unsigned char* img2,\n" \
"	__global float* sumf,\n" \
"	__global uint* sumi,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	unsigned int nxy,\n" \
"	unsigned int ngx,\n" \
"	unsigned int ngy,\n" \
"	unsigned int ngz,\n" \
"	unsigned int gsxy,\n" \
"	unsigned int gsx,\n" \
"	int3 bmin,\n" \
"	float intmax,\n" \
"	float4 mat0,\n" \
"	float4 mat1,\n" \
"	float4 mat2,\n" \
"	float4 mat3)\n" \
"{\n" \
"	int3 gid = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);\n" \
"	int3 ub = (int3)(lb.x + ngx, lb.y + ngy, lb.z + ngz);\n" \
"	lb += bmin;\n" \
"	ub += bmin;\n" \
"	int4 ijk = (int4)(0, 0, 0, 1);\n" \
"	unsigned int index;\n" \
"	float lsumf = 0.0f;\n" \
"	uint lsumi = 0;\n" \
"	float v1, v2;\n" \
"	float4 coord;\n" \
"	int4 coordi;\n" \
"	int flag;\n" \
"#pragma unroll\n" \
"	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)\n" \
"#pragma unroll\n" \
"	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)\n" \
"#pragma unroll\n" \
"	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)\n" \
"	{\n" \
"		flag = 0;\n" \
"		if (ijk.x >= 0 && ijk.x < nx && ijk.y >= 0 && ijk.y < ny && ijk.z >= 0 && ijk.z < nz)\n" \
"		{\n" \
"			index = nxy*ijk.z + nx*ijk.y + ijk.x;\n" \
"			v1 = convert_float(img1[index]) / 255.0f;\n" \
"			flag++;\n" \
"		}\n" \
"		coord = convert_float4(ijk) + (float4)(0.5f, 0.5f, 0.5f, 0.0f);\n" \
"		coord = (float4)(dot(coord, mat0), dot(coord, mat1), dot(coord, mat2), dot(coord, mat3));\n" \
"		coord /= coord.w;\n" \
"		coordi = convert_int4_rtz(coord);\n" \
"		if (coordi.x >= 0 && coordi.x < nx && coordi.y >= 0 && coordi.y < ny && coordi.z >= 0 && coordi.z < nz)\n" \
"		{\n" \
"			index = nxy*coordi.z + nx*coordi.y + coordi.x;\n" \
"			v2 = convert_float(img2[index]) / 255.0f;\n" \
"			flag++;\n" \
"		}\n" \
"		if (flag == 2)\n" \
"		{\n" \
"			v1 = v1 * v2;\n" \
"			//v1 = pown(v1, 2);\n" \
"			lsumf += v1;\n" \
"			lsumi++;\n" \
"		}\n" \
"	}\n" \
"	index = gsxy * gid.z + gsx * gid.y + gid.x;\n" \
"	atomic_xchg(sumf+index, lsumf);\n" \
"	atomic_xchg(sumi+index, lsumi);\n" \
"}\n" \
"//compare0 16 bit\n" \
"__kernel void kernel_3(\n" \
"	__global unsigned short* img1,\n" \
"	__global unsigned short* img2,\n" \
"	__global float* sumf,\n" \
"	__global uint* sumi,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	unsigned int nxy,\n" \
"	unsigned int ngx,\n" \
"	unsigned int ngy,\n" \
"	unsigned int ngz,\n" \
"	unsigned int gsxy,\n" \
"	unsigned int gsx,\n" \
"	int3 bmin,\n" \
"	float intmax,\n" \
"	float4 mat0,\n" \
"	float4 mat1,\n" \
"	float4 mat2,\n" \
"	float4 mat3)\n" \
"{\n" \
"	int3 gid = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);\n" \
"	int3 ub = (int3)(lb.x + ngx, lb.y + ngy, lb.z + ngz);\n" \
"	lb += bmin;\n" \
"	ub += bmin;\n" \
"	int4 ijk = (int4)(0, 0, 0, 1);\n" \
"	unsigned int index;\n" \
"	float lsumf = 0.0f;\n" \
"	uint lsumi = 0;\n" \
"	float v1, v2;\n" \
"	float4 coord;\n" \
"	int4 coordi;\n" \
"	int flag;\n" \
"#pragma unroll\n" \
"	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)\n" \
"#pragma unroll\n" \
"	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)\n" \
"#pragma unroll\n" \
"	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)\n" \
"	{\n" \
"		flag = 0;\n" \
"		if (ijk.x >= 0 && ijk.x < nx && ijk.y >= 0 && ijk.y < ny && ijk.z >= 0 && ijk.z < nz)\n" \
"		{\n" \
"			index = nxy*ijk.z + nx*ijk.y + ijk.x;\n" \
"			v1 = convert_float(img1[index]) / intmax;\n" \
"			flag++;\n" \
"		}\n" \
"		coord = convert_float4(ijk) + (float4)(0.5f, 0.5f, 0.5f, 0.0f);\n" \
"		coord = (float4)(dot(coord, mat0), dot(coord, mat1), dot(coord, mat2), dot(coord, mat3));\n" \
"		coord /= coord.w;\n" \
"		coordi = convert_int4_rtz(coord);\n" \
"		if (coordi.x >= 0 && coordi.x < nx && coordi.y >= 0 && coordi.y < ny && coordi.z >= 0 && coordi.z < nz)\n" \
"		{\n" \
"			index = nxy*coordi.z + nx*coordi.y + coordi.x;\n" \
"			v2 = convert_float(img2[index]) / intmax;\n" \
"			flag++;\n" \
"		}\n" \
"		if (flag == 2)\n" \
"		{\n" \
"			v1 = v1 * v2;\n" \
"			//v1 = pown(v1, 2);\n" \
"			lsumf += v1;\n" \
"			lsumi++;\n" \
"		}\n" \
"	}\n" \
"	index = gsxy * gid.z + gsx * gid.y + gid.x;\n" \
"	atomic_xchg(sumf+index, lsumf);\n" \
"	atomic_xchg(sumi+index, lsumi);\n" \
"}\n" \
"//compare1 8 bit\n" \
"__kernel void kernel_4(\n" \
"	__global unsigned char* img1,\n" \
"	__global unsigned char* img2,\n" \
"	__global float* sumf,\n" \
"	__global uint* sumi,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	unsigned int nxy,\n" \
"	unsigned int ngx,\n" \
"	unsigned int ngy,\n" \
"	unsigned int ngz,\n" \
"	unsigned int gsxy,\n" \
"	unsigned int gsx,\n" \
"	int3 bmin,\n" \
"	float intmax,\n" \
"	float4 mat0,\n" \
"	float4 mat1,\n" \
"	float4 mat2,\n" \
"	float4 mat3)\n" \
"{\n" \
"	int3 gid = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);\n" \
"	int3 ub = (int3)(lb.x + ngx, lb.y + ngy, lb.z + ngz);\n" \
"	lb += bmin;\n" \
"	ub += bmin;\n" \
"	int4 ijk = (int4)(0, 0, 0, 1);\n" \
"	unsigned int index;\n" \
"	float lsumf = 0.0f;\n" \
"	uint lsumi = 0;\n" \
"	float v1, v2, w;\n" \
"	float4 coord;\n" \
"	int4 coordi;\n" \
"	int flag;\n" \
"#pragma unroll\n" \
"	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)\n" \
"#pragma unroll\n" \
"	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)\n" \
"#pragma unroll\n" \
"	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)\n" \
"	{\n" \
"		flag = 0;\n" \
"		if (ijk.x >= 0 && ijk.x < nx && ijk.y >= 0 && ijk.y < ny && ijk.z >= 0 && ijk.z < nz)\n" \
"		{\n" \
"			index = nxy*ijk.z + nx*ijk.y + ijk.x;\n" \
"			v1 = convert_float(img1[index]) / 255.0f;\n" \
"			flag++;\n" \
"		}\n" \
"		coord = convert_float4(ijk) + (float4)(0.5f, 0.5f, 0.5f, 0.0f);\n" \
"		coord = (float4)(dot(coord, mat0), dot(coord, mat1), dot(coord, mat2), dot(coord, mat3));\n" \
"		coord /= coord.w;\n" \
"		coordi = convert_int4_rtz(coord);\n" \
"		if (coordi.x >= 0 && coordi.x < nx && coordi.y >= 0 && coordi.y < ny && coordi.z >= 0 && coordi.z < nz)\n" \
"		{\n" \
"			index = nxy*coordi.z + nx*coordi.y + coordi.x;\n" \
"			v2 = convert_float(img2[index]) / 255.0f;\n" \
"			flag++;\n" \
"		}\n" \
"		if (flag == 2)\n" \
"		{\n" \
"			w = min(v1, v2);\n" \
"			v1 = v1 - v2;\n" \
"			v1 *= v1;\n" \
"			v1 = 1.0f - v1;\n" \
"			lsumf += v1 * w;\n" \
"			lsumi++;\n" \
"		}\n" \
"	}\n" \
"	index = gsxy * gid.z + gsx * gid.y + gid.x;\n" \
"	atomic_xchg(sumf+index, lsumf);\n" \
"	atomic_xchg(sumi+index, lsumi);\n" \
"}\n" \
"//compare1 16 bit\n" \
"__kernel void kernel_5(\n" \
"	__global unsigned short* img1,\n" \
"	__global unsigned short* img2,\n" \
"	__global float* sumf,\n" \
"	__global uint* sumi,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	unsigned int nxy,\n" \
"	unsigned int ngx,\n" \
"	unsigned int ngy,\n" \
"	unsigned int ngz,\n" \
"	unsigned int gsxy,\n" \
"	unsigned int gsx,\n" \
"	int3 bmin,\n" \
"	float intmax,\n" \
"	float4 mat0,\n" \
"	float4 mat1,\n" \
"	float4 mat2,\n" \
"	float4 mat3)\n" \
"{\n" \
"	int3 gid = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);\n" \
"	int3 ub = (int3)(lb.x + ngx, lb.y + ngy, lb.z + ngz);\n" \
"	lb += bmin;\n" \
"	ub += bmin;\n" \
"	int4 ijk = (int4)(0, 0, 0, 1);\n" \
"	unsigned int index;\n" \
"	float lsumf = 0.0f;\n" \
"	uint lsumi = 0;\n" \
"	float v1, v2, w;\n" \
"	float4 coord;\n" \
"	int4 coordi;\n" \
"	int flag;\n" \
"#pragma unroll\n" \
"	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)\n" \
"#pragma unroll\n" \
"	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)\n" \
"#pragma unroll\n" \
"	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)\n" \
"	{\n" \
"		flag = 0;\n" \
"		if (ijk.x >= 0 && ijk.x < nx && ijk.y >= 0 && ijk.y < ny && ijk.z >= 0 && ijk.z < nz)\n" \
"		{\n" \
"			index = nxy*ijk.z + nx*ijk.y + ijk.x;\n" \
"			v1 = convert_float(img1[index]) / intmax;\n" \
"			flag++;\n" \
"		}\n" \
"		coord = convert_float4(ijk) + (float4)(0.5f, 0.5f, 0.5f, 0.0f);\n" \
"		coord = (float4)(dot(coord, mat0), dot(coord, mat1), dot(coord, mat2), dot(coord, mat3));\n" \
"		coord /= coord.w;\n" \
"		coordi = convert_int4_rtz(coord);\n" \
"		if (coordi.x >= 0 && coordi.x < nx && coordi.y >= 0 && coordi.y < ny && coordi.z >= 0 && coordi.z < nz)\n" \
"		{\n" \
"			index = nxy*coordi.z + nx*coordi.y + coordi.x;\n" \
"			v2 = convert_float(img2[index]) / intmax;\n" \
"			flag++;\n" \
"		}\n" \
"		if (flag == 2)\n" \
"		{\n" \
"			w = min(v1, v2);\n" \
"			v1 = v1 - v2;\n" \
"			v1 *= v1;\n" \
"			v1 = 1.0f - v1;\n" \
"			lsumf += v1 * w;\n" \
"			lsumi++;\n" \
"		}\n" \
"	}\n" \
"	index = gsxy * gid.z + gsx * gid.y + gid.x;\n" \
"	atomic_xchg(sumf+index, lsumf);\n" \
"	atomic_xchg(sumi+index, lsumi);\n" \
"}\n" \
;

const char* str_cl_stencil_mask = \
"//2d filter 8 bit\n" \
"__kernel void kernel_0(\n" \
"	__global unsigned char* img_in,\n" \
"	__global unsigned char* img_out,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz)\n" \
"{\n" \
"	int3 gid = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	int3 lb = gid - (int3)(1, 1, 0);\n" \
"	int3 ub = gid + (int3)(1, 1, 0);\n" \
"	lb = clamp(lb, (int3)(0), (int3)(nx-1, ny-1, nz-1));\n" \
"	ub = clamp(ub, (int3)(0), (int3)(nx-1, ny-1, nz-1));\n" \
"	int3 ijk;\n" \
"	float sum = 0.0f;\n" \
"	int count = 0;\n" \
"	unsigned int index;\n" \
"#pragma unroll\n" \
"	for (ijk.z = lb.z; ijk.z <= ub.z; ++ijk.z)\n" \
"#pragma unroll\n" \
"	for (ijk.y = lb.y; ijk.y <= ub.y; ++ijk.y)\n" \
"#pragma unroll\n" \
"	for (ijk.x = lb.x; ijk.x <= ub.x; ++ijk.x)\n" \
"	{\n" \
"		index = nx*ny*ijk.z + nx*ijk.y + ijk.x;\n" \
"		sum += img_in[index];\n" \
"		count++;\n" \
"	}\n" \
"	sum = count ? sum / count : sum;\n" \
"	index = nx*ny*gid.z + nx*gid.y + gid.x;\n" \
"	img_out[index] = convert_uchar(sum);\n" \
"}\n"
"//2d filter 16 bit\n" \
"__kernel void kernel_1(\n" \
"	__global unsigned short* img_in,\n" \
"	__global unsigned short* img_out,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz)\n" \
"{\n" \
"	int3 gid = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	int3 lb = gid - (int3)(1, 1, 0);\n" \
"	int3 ub = gid + (int3)(1, 1, 0);\n" \
"	lb = clamp(lb, (int3)(0), (int3)(nx-1, ny-1, nz-1));\n" \
"	ub = clamp(ub, (int3)(0), (int3)(nx-1, ny-1, nz-1));\n" \
"	int3 ijk;\n" \
"	float sum = 0.0f;\n" \
"	int count = 0;\n" \
"	unsigned int index;\n" \
"#pragma unroll\n" \
"	for (ijk.z = lb.z; ijk.z <= ub.z; ++ijk.z)\n" \
"#pragma unroll\n" \
"	for (ijk.y = lb.y; ijk.y <= ub.y; ++ijk.y)\n" \
"#pragma unroll\n" \
"	for (ijk.x = lb.x; ijk.x <= ub.x; ++ijk.x)\n" \
"	{\n" \
"		index = nx*ny*ijk.z + nx*ijk.y + ijk.x;\n" \
"		sum += img_in[index];\n" \
"		count++;\n" \
"	}\n" \
"	sum = count ? sum / count : sum;\n" \
"	index = nx*ny*gid.z + nx*gid.y + gid.x;\n" \
"	img_out[index] = convert_ushort(sum);\n" \
"}\n"
"//compare0 8 bit\n" \
"__kernel void kernel_2(\n" \
"	__global unsigned char* img1,\n" \
"	__global unsigned char* mask,\n" \
"	__global unsigned char* img2,\n" \
"	__global float* sumf,\n" \
"	__global uint* sumi,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	unsigned int nxy,\n" \
"	unsigned int ngx,\n" \
"	unsigned int ngy,\n" \
"	unsigned int ngz,\n" \
"	unsigned int gsxy,\n" \
"	unsigned int gsx,\n" \
"	int3 bmin,\n" \
"	float intmax,\n" \
"	float4 mat0,\n" \
"	float4 mat1,\n" \
"	float4 mat2,\n" \
"	float4 mat3)\n" \
"{\n" \
"	int3 gid = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);\n" \
"	int3 ub = (int3)(lb.x + ngx, lb.y + ngy, lb.z + ngz);\n" \
"	lb += bmin;\n" \
"	ub += bmin;\n" \
"	int4 ijk = (int4)(0, 0, 0, 1);\n" \
"	unsigned int index;\n" \
"	float lsumf = 0.0f;\n" \
"	uint lsumi = 0;\n" \
"	float v1, v2;\n" \
"	float4 coord;\n" \
"	int4 coordi;\n" \
"	int flag;\n" \
"#pragma unroll\n" \
"	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)\n" \
"#pragma unroll\n" \
"	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)\n" \
"#pragma unroll\n" \
"	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)\n" \
"	{\n" \
"		flag = 0;\n" \
"		if (ijk.x >= 0 && ijk.x < nx && ijk.y >= 0 && ijk.y < ny && ijk.z >= 0 && ijk.z < nz)\n" \
"		{\n" \
"			index = nxy*ijk.z + nx*ijk.y + ijk.x;\n" \
"			if (mask[index])\n" \
"			{\n" \
"				v1 = convert_float(img1[index]) / 255.0f;\n" \
"				flag++;\n" \
"			}\n" \
"		}\n" \
"		coord = convert_float4(ijk) + (float4)(0.5f, 0.5f, 0.5f, 0.0f);\n" \
"		coord = (float4)(dot(coord, mat0), dot(coord, mat1), dot(coord, mat2), dot(coord, mat3));\n" \
"		coord /= coord.w;\n" \
"		coordi = convert_int4_rtz(coord);\n" \
"		if (coordi.x >= 0 && coordi.x < nx && coordi.y >= 0 && coordi.y < ny && coordi.z >= 0 && coordi.z < nz)\n" \
"		{\n" \
"			index = nxy*coordi.z + nx*coordi.y + coordi.x;\n" \
"			v2 = convert_float(img2[index]) / 255.0f;\n" \
"			flag++;\n" \
"		}\n" \
"		if (flag == 2)\n" \
"		{\n" \
"			v1 = v1 * v2;\n" \
"			//v1 = pown(v1, 2);\n" \
"			lsumf += v1;\n" \
"			lsumi++;\n" \
"		}\n" \
"	}\n" \
"	index = gsxy * gid.z + gsx * gid.y + gid.x;\n" \
"	atomic_xchg(sumf+index, lsumf);\n" \
"	atomic_xchg(sumi+index, lsumi);\n" \
"}\n" \
"//compare0 16 bit\n" \
"__kernel void kernel_3(\n" \
"	__global unsigned short* img1,\n" \
"	__global unsigned char* mask,\n" \
"	__global unsigned short* img2,\n" \
"	__global float* sumf,\n" \
"	__global uint* sumi,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	unsigned int nxy,\n" \
"	unsigned int ngx,\n" \
"	unsigned int ngy,\n" \
"	unsigned int ngz,\n" \
"	unsigned int gsxy,\n" \
"	unsigned int gsx,\n" \
"	int3 bmin,\n" \
"	float intmax,\n" \
"	float4 mat0,\n" \
"	float4 mat1,\n" \
"	float4 mat2,\n" \
"	float4 mat3)\n" \
"{\n" \
"	int3 gid = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);\n" \
"	int3 ub = (int3)(lb.x + ngx, lb.y + ngy, lb.z + ngz);\n" \
"	lb += bmin;\n" \
"	ub += bmin;\n" \
"	int4 ijk = (int4)(0, 0, 0, 1);\n" \
"	unsigned int index;\n" \
"	float lsumf = 0.0f;\n" \
"	uint lsumi = 0;\n" \
"	float v1, v2;\n" \
"	float4 coord;\n" \
"	int4 coordi;\n" \
"	int flag;\n" \
"#pragma unroll\n" \
"	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)\n" \
"#pragma unroll\n" \
"	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)\n" \
"#pragma unroll\n" \
"	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)\n" \
"	{\n" \
"		flag = 0;\n" \
"		if (ijk.x >= 0 && ijk.x < nx && ijk.y >= 0 && ijk.y < ny && ijk.z >= 0 && ijk.z < nz)\n" \
"		{\n" \
"			index = nxy*ijk.z + nx*ijk.y + ijk.x;\n" \
"			if (mask[index])\n" \
"			{\n" \
"				v1 = convert_float(img1[index]) / intmax;\n" \
"				flag++;\n" \
"			}\n" \
"		}\n" \
"		coord = convert_float4(ijk) + (float4)(0.5f, 0.5f, 0.5f, 0.0f);\n" \
"		coord = (float4)(dot(coord, mat0), dot(coord, mat1), dot(coord, mat2), dot(coord, mat3));\n" \
"		coord /= coord.w;\n" \
"		coordi = convert_int4_rtz(coord);\n" \
"		if (coordi.x >= 0 && coordi.x < nx && coordi.y >= 0 && coordi.y < ny && coordi.z >= 0 && coordi.z < nz)\n" \
"		{\n" \
"			index = nxy*coordi.z + nx*coordi.y + coordi.x;\n" \
"			v2 = convert_float(img2[index]) / intmax;\n" \
"			flag++;\n" \
"		}\n" \
"		if (flag == 2)\n" \
"		{\n" \
"			v1 = v1 * v2;\n" \
"			//v1 = pown(v1, 2);\n" \
"			lsumf += v1;\n" \
"			lsumi++;\n" \
"		}\n" \
"	}\n" \
"	index = gsxy * gid.z + gsx * gid.y + gid.x;\n" \
"	atomic_xchg(sumf+index, lsumf);\n" \
"	atomic_xchg(sumi+index, lsumi);\n" \
"}\n" \
"//compare1 8 bit\n" \
"__kernel void kernel_4(\n" \
"	__global unsigned char* img1,\n" \
"	__global unsigned char* mask,\n" \
"	__global unsigned char* img2,\n" \
"	__global float* sumf,\n" \
"	__global uint* sumi,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	unsigned int nxy,\n" \
"	unsigned int ngx,\n" \
"	unsigned int ngy,\n" \
"	unsigned int ngz,\n" \
"	unsigned int gsxy,\n" \
"	unsigned int gsx,\n" \
"	int3 bmin,\n" \
"	float intmax,\n" \
"	float4 mat0,\n" \
"	float4 mat1,\n" \
"	float4 mat2,\n" \
"	float4 mat3)\n" \
"{\n" \
"	int3 gid = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);\n" \
"	int3 ub = (int3)(lb.x + ngx, lb.y + ngy, lb.z + ngz);\n" \
"	lb += bmin;\n" \
"	ub += bmin;\n" \
"	int4 ijk = (int4)(0, 0, 0, 1);\n" \
"	unsigned int index;\n" \
"	float lsumf = 0.0f;\n" \
"	uint lsumi = 0;\n" \
"	float v1, v2, w;\n" \
"	float4 coord;\n" \
"	int4 coordi;\n" \
"	int flag;\n" \
"#pragma unroll\n" \
"	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)\n" \
"#pragma unroll\n" \
"	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)\n" \
"#pragma unroll\n" \
"	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)\n" \
"	{\n" \
"		flag = 0;\n" \
"		if (ijk.x >= 0 && ijk.x < nx && ijk.y >= 0 && ijk.y < ny && ijk.z >= 0 && ijk.z < nz)\n" \
"		{\n" \
"			index = nxy*ijk.z + nx*ijk.y + ijk.x;\n" \
"			if (mask[index])\n" \
"			{\n" \
"				v1 = convert_float(img1[index]) / 255.0f;\n" \
"				flag++;\n" \
"			}\n" \
"		}\n" \
"		coord = convert_float4(ijk) + (float4)(0.5f, 0.5f, 0.5f, 0.0f);\n" \
"		coord = (float4)(dot(coord, mat0), dot(coord, mat1), dot(coord, mat2), dot(coord, mat3));\n" \
"		coord /= coord.w;\n" \
"		coordi = convert_int4_rtz(coord);\n" \
"		if (coordi.x >= 0 && coordi.x < nx && coordi.y >= 0 && coordi.y < ny && coordi.z >= 0 && coordi.z < nz)\n" \
"		{\n" \
"			index = nxy*coordi.z + nx*coordi.y + coordi.x;\n" \
"			v2 = convert_float(img2[index]) / 255.0f;\n" \
"			flag++;\n" \
"		}\n" \
"		if (flag == 2)\n" \
"		{\n" \
"			w = min(v1, v2);\n" \
"			v1 = v1 - v2;\n" \
"			v1 *= v1;\n" \
"			v1 = 1.0f - v1;\n" \
"			lsumf += v1 * w;\n" \
"			lsumi++;\n" \
"		}\n" \
"	}\n" \
"	index = gsxy * gid.z + gsx * gid.y + gid.x;\n" \
"	atomic_xchg(sumf+index, lsumf);\n" \
"	atomic_xchg(sumi+index, lsumi);\n" \
"}\n" \
"//compare1 16 bit\n" \
"__kernel void kernel_5(\n" \
"	__global unsigned short* img1,\n" \
"	__global unsigned char* mask,\n" \
"	__global unsigned short* img2,\n" \
"	__global float* sumf,\n" \
"	__global uint* sumi,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	unsigned int nxy,\n" \
"	unsigned int ngx,\n" \
"	unsigned int ngy,\n" \
"	unsigned int ngz,\n" \
"	unsigned int gsxy,\n" \
"	unsigned int gsx,\n" \
"	int3 bmin,\n" \
"	float intmax,\n" \
"	float4 mat0,\n" \
"	float4 mat1,\n" \
"	float4 mat2,\n" \
"	float4 mat3)\n" \
"{\n" \
"	int3 gid = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);\n" \
"	int3 ub = (int3)(lb.x + ngx, lb.y + ngy, lb.z + ngz);\n" \
"	lb += bmin;\n" \
"	ub += bmin;\n" \
"	int4 ijk = (int4)(0, 0, 0, 1);\n" \
"	unsigned int index;\n" \
"	float lsumf = 0.0f;\n" \
"	uint lsumi = 0;\n" \
"	float v1, v2, w;\n" \
"	float4 coord;\n" \
"	int4 coordi;\n" \
"	int flag;\n" \
"#pragma unroll\n" \
"	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)\n" \
"#pragma unroll\n" \
"	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)\n" \
"#pragma unroll\n" \
"	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)\n" \
"	{\n" \
"		flag = 0;\n" \
"		if (ijk.x >= 0 && ijk.x < nx && ijk.y >= 0 && ijk.y < ny && ijk.z >= 0 && ijk.z < nz)\n" \
"		{\n" \
"			index = nxy*ijk.z + nx*ijk.y + ijk.x;\n" \
"			if (mask[index])\n" \
"			{\n" \
"				v1 = convert_float(img1[index]) / intmax;\n" \
"				flag++;\n" \
"			}\n" \
"		}\n" \
"		coord = convert_float4(ijk) + (float4)(0.5f, 0.5f, 0.5f, 0.0f);\n" \
"		coord = (float4)(dot(coord, mat0), dot(coord, mat1), dot(coord, mat2), dot(coord, mat3));\n" \
"		coord /= coord.w;\n" \
"		coordi = convert_int4_rtz(coord);\n" \
"		if (coordi.x >= 0 && coordi.x < nx && coordi.y >= 0 && coordi.y < ny && coordi.z >= 0 && coordi.z < nz)\n" \
"		{\n" \
"			index = nxy*coordi.z + nx*coordi.y + coordi.x;\n" \
"			v2 = convert_float(img2[index]) / intmax;\n" \
"			flag++;\n" \
"		}\n" \
"		if (flag == 2)\n" \
"		{\n" \
"			w = min(v1, v2);\n" \
"			v1 = v1 - v2;\n" \
"			v1 *= v1;\n" \
"			v1 = 1.0f - v1;\n" \
"			lsumf += v1 * w;\n" \
"			lsumi++;\n" \
"		}\n" \
"	}\n" \
"	index = gsxy * gid.z + gsx * gid.y + gid.x;\n" \
"	atomic_xchg(sumf+index, lsumf);\n" \
"	atomic_xchg(sumi+index, lsumi);\n" \
"}\n" \
;

StencilCompare::StencilCompare(Stencil* s1, Stencil* s2,
	const fluo::Vector& ext1, const fluo::Vector& ext2,
	const fluo::Vector& off1, const fluo::Vector& off2,
	const int iter, const int conv_num, const int method,
	const bool use_mask):
m_s1(s1), m_s2(s2),
m_ext1(ext1), m_ext2(ext2),
m_off1(off1), m_off2(off2),
m_iter(iter), m_conv_num(conv_num),
m_method(method),
m_use_mask(use_mask)
{
	//create program
	m_prog = glbin_vol_kernel_factory.kernel(
		m_use_mask ? str_cl_stencil_mask : str_cl_stencil);
}

StencilCompare::~StencilCompare()
{

}

void StencilCompare::Prepare(const std::string& cmp_name)
{
	if (!m_prog)
		return;
	int kernel_index = -1;
	std::string name = "kernel_0";
	if (m_s1->bits > 8) name = "kernel_1";
	if (m_prog->valid())
	{
		kernel_index = m_prog->findKernel(name);
		if (kernel_index == -1)
			kernel_index = m_prog->createKernel(name);
	}
	else
		kernel_index = m_prog->createKernel(name);

	flvr::Argument img[2];
	unsigned int nx = m_s1->nx, ny = m_s1->ny, nz = m_s1->nz;
	size_t local_size[3] = { 1, 1, 1 };
	size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };
	size_t buf_size = m_s1->bits == 8 ?
		sizeof(unsigned char) : sizeof(unsigned short);
	buf_size *= nx * ny * nz;
	//DBMIUINT8 mi(m_s1->nx, m_s1->ny, 1);

	//set up kernel
	m_prog->setKernelArgBegin(kernel_index);
	if (m_s1->fsize < 1)
		m_img1 = m_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, buf_size, (void*)(m_s1->data));
	else
	{
		img[0] = m_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, buf_size, (void*)(m_s1->data));
		img[1] = m_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY, buf_size, NULL);
		m_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nx));
		m_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&ny));
		m_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nz));
		//filter s1
		for (int i = 0; i < m_s1->fsize; ++i)
		{
			if (i)
			{
				//swap images
				m_prog->setKernelArgBegin(kernel_index);
				m_prog->setKernelArgument(img[i%2]);
				m_prog->setKernelArgument(img[(i+1)%2]);
			}
			m_prog->executeKernel(kernel_index, 3, global_size, local_size);
		}
		m_img1 = img[m_s1->fsize % 2];
		m_prog->releaseMemObject(img[(m_s1->fsize+1)%2]);
	}

	//set up kernel
	m_prog->setKernelArgBegin(kernel_index);
	if (m_s2->fsize < 1)
		m_img2 = m_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, buf_size, (void*)(m_s2->data));
	else
	{
		img[0] = m_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, buf_size, (void*)(m_s2->data));
		img[1] = m_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY, buf_size, NULL);
		m_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nx));
		m_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&ny));
		m_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nz));
		//filter s2
		for (int i = 0; i < m_s2->fsize; ++i)
		{
			if (i)
			{
				//swap images
				m_prog->setKernelArgBegin(kernel_index);
				m_prog->setKernelArgument(img[i % 2]);
				m_prog->setKernelArgument(img[(i + 1) % 2]);
			}
			m_prog->executeKernel(kernel_index, 3, global_size, local_size);
		}
		m_img2 = img[m_s2->fsize % 2];
		m_prog->releaseMemObject(img[(m_s2->fsize + 1) % 2]);
	}
	//m_prog->readBuffer(m_img1, (void*)(mi.data));
	//m_prog->readBuffer(m_img2, (void*)(mi.data));

	//set up kernel
	if (m_use_mask)
	{
		if (m_prog->valid())
		{
			kernel_index = m_prog->findKernel(cmp_name);
			if (kernel_index == -1)
				kernel_index = m_prog->createKernel(cmp_name);
		}
		else
			kernel_index = m_prog->createKernel(cmp_name);
		buf_size = sizeof(unsigned char) * nx * ny * nz;
		m_prog->setKernelArgBegin(kernel_index, 1);
		m_mask1 = m_prog->setKernelArgBuf(
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			buf_size, m_s1->mask);
	}
}

void StencilCompare::Clean()
{
	m_prog->releaseAll();
}

bool StencilCompare::Compare()
{
	size_t bits = m_s1->bits;
	std::string name;
	switch (m_method)
	{
	case 0:
		if (bits == 8)
			name = "kernel_2";
		else
			name = "kernel_3";
		break;
	case 1:
		if (bits == 8)
			name = "kernel_4";
		else
			name = "kernel_5";
		break;
		break;
	}

	Prepare(name);

	//set up initial neighborhoods
	fluo::Vector s1cp(m_s1->box.center());
	fluo::Vector range = m_s1->box.diagonal();
	range = fluo::Min(range, fluo::Vector(m_s1->nx-1, m_s1->ny-1, m_s1->nz-1));
	range = fluo::Min(range, m_ext1);
	fluo::Neighbor nb_trans(fluo::Point(), range);
	range = fluo::Min(fluo::Vector(range.z(), range.z(), 180), m_ext2);
	fluo::Neighbor nb_rot(fluo::Point(), range);
	//constant neighborhoods
	fluo::Neighbor nb_null(fluo::Point(), fluo::Vector(0));
	fluo::Neighbor nbt_1(nb_trans);
	fluo::Neighbor nbr_1(nb_rot);
	//fluo::Vector ht(1), hr(1);//step length

	//main loop
	float p, maxp;
	m_s2->load_identity();
	maxp = Similar(name);//start with origin
	fluo::Point center(m_off1), euler(m_off2);//for outer loop
	fluo::Point c, e;//for inner loops
	int counter = 0;
	bool rot = false;//loop mode
	bool conv = false;//
	int conv_count = 0, trans_count = 0, rot_count = 0;//conv in a sequence

	fluo::Neighbor nbt;
	fluo::Neighbor nbr;
	//set up neighborhood
	if (rot)
	{
		nbt = nb_null;
		nbr = nbr_1;
	}
	else
	{
		nbt = nbt_1;
		nbr = nb_null;
	}
	while (true)
	{
		bool foundp = false;

		//trans loop
		for (fluo::Point i = nbt.begin(); i != nbt.end(); i = ++nbt)
		{
			//rot loop
			for (fluo::Point j = nbr.begin(); j != nbr.end(); j = ++nbr)
			{
				//compute transform
				m_s2->load_identity();
				m_s2->rotate(euler + j, s1cp + center + i);
				m_s2->translate(center + i);

				//compare images
				p = Similar(name);
				if (p > maxp)
				{
					maxp = p;
					c = i;
					e = j;
					foundp = true;
				}
			}
		}

		if (foundp)
		{
			if (rot)
			{
				euler += e;
				//update neighborhood
				nbr.c(e);
			}
			else
			{
				center += c;
				//update neighborhood
				nbt.c(c);
			}
			conv = false;
		}
		else
		{
			//convergence conditions
			if (conv_count > m_conv_num)
				break;
			if (rot)
			{
				//rot converged
				rot_count++;
				rot = false;
				//update trans neighborhood
				if (trans_count > 0)
					nbt_1.halfn();
				if (conv_count)
					nbt_1.halfh();
				nbt = nbt_1;
				nbr = nb_null;
			}
			else
			{
				//trans converged
				trans_count++;
				rot = true;
				//update rot neighborhood
				if (rot_count > 0)
					nbr_1.halfn();
				if (conv_count)
					nbr_1.halfh();
				nbr = nbr_1;
				nbt = nb_null;
			}
			if (conv)
				conv_count++;
			conv = true;
		}

		if (counter > m_iter)
			break;
		counter++;
	}

	//for read externally
	m_translate = center;
	m_euler = -euler;
	m_center = fluo::Point(s1cp);
	//for internal use
	m_s2->load_identity();
	m_s2->rotate(fluo::Vector(euler), fluo::Vector(center) + s1cp);
	m_s2->translate(fluo::Vector(center));
	m_s2->id = m_s1->id;

	Clean();

	return true;
}

float StencilCompare::Similar(const std::string& name)
{
	float result = 0.0f;

	if (!m_prog)
		return result;
	int kernel_index = -1;
	if (m_prog->valid())
	{
		kernel_index = m_prog->findKernel(name);
		if (kernel_index == -1)
			kernel_index = m_prog->createKernel(name);
	}
	else
		kernel_index = m_prog->createKernel(name);

	size_t local_size[3] = { 1, 1, 1 };
	fluo::Vector diag = m_s1->box.diagonal();
	diag = fluo::Max(diag, fluo::Vector(1));
	flvr::GroupSize gsize;
	m_prog->get_group_size(kernel_index, diag.intx(), diag.inty(), diag.intz(), gsize);
	size_t global_size[3] = { size_t(gsize.gsx), size_t(gsize.gsy), size_t(gsize.gsz) };

	//set up kernel
	unsigned int nx = m_s1->nx, ny = m_s1->ny, nz = m_s1->nz;
	unsigned int nxy = nx * ny;
	cl_float intmax = m_s1->max_int;
	cl_int3 bmin{
		m_s1->box.Min().intx(),
		m_s1->box.Min().inty(),
		m_s1->box.Min().intz() };
	cl_float4 tf0 = {
		float(m_s2->tf.get_mat_val(0, 0)),
		float(m_s2->tf.get_mat_val(0, 1)),
		float(m_s2->tf.get_mat_val(0, 2)),
		float(m_s2->tf.get_mat_val(0, 3)) };
	cl_float4 tf1 = {
		float(m_s2->tf.get_mat_val(1, 0)),
		float(m_s2->tf.get_mat_val(1, 1)),
		float(m_s2->tf.get_mat_val(1, 2)),
		float(m_s2->tf.get_mat_val(1, 3)) };
	cl_float4 tf2 = {
		float(m_s2->tf.get_mat_val(2, 0)),
		float(m_s2->tf.get_mat_val(2, 1)),
		float(m_s2->tf.get_mat_val(2, 2)),
		float(m_s2->tf.get_mat_val(2, 3)) };
	cl_float4 tf3 = {
		float(m_s2->tf.get_mat_val(3, 0)),
		float(m_s2->tf.get_mat_val(3, 1)),
		float(m_s2->tf.get_mat_val(3, 2)),
		float(m_s2->tf.get_mat_val(3, 3)) };
	cl_float* sumf = new cl_float[gsize.gsxyz];
	cl_uint* sumi = new cl_uint[gsize.gsxyz];
	//
	m_prog->setKernelArgBegin(kernel_index);
	m_prog->setKernelArgument(m_img1);
	if (m_use_mask)
		m_prog->setKernelArgument(m_mask1);
	m_prog->setKernelArgument(m_img2);
	flvr::Argument arg_sumf = m_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_float)*(gsize.gsxyz), (void*)(sumf));
	flvr::Argument arg_sumi = m_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_uint) * (gsize.gsxyz), (void*)(sumi));
	m_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nx));
	m_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&ny));
	m_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nz));
	m_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nxy));
	m_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngx));
	m_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngy));
	m_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngz));
	m_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.gsxy));
	m_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.gsx));
	m_prog->setKernelArgConst(sizeof(cl_int3), (void*)(&bmin));
	m_prog->setKernelArgConst(sizeof(cl_float), (void*)(&intmax));
	m_prog->setKernelArgConst(sizeof(cl_float4), (void*)(&tf0));
	m_prog->setKernelArgConst(sizeof(cl_float4), (void*)(&tf1));
	m_prog->setKernelArgConst(sizeof(cl_float4), (void*)(&tf2));
	m_prog->setKernelArgConst(sizeof(cl_float4), (void*)(&tf3));

	//execute
	m_prog->executeKernel(kernel_index, 3, global_size, 0/*local_size*/);
	//read back
	m_prog->readBuffer(sizeof(cl_float)*(gsize.gsxyz), sumf, sumf);
	m_prog->readBuffer(sizeof(cl_uint) * (gsize.gsxyz), sumi, sumi);
	m_prog->releaseMemObject(arg_sumf);
	m_prog->releaseMemObject(arg_sumi);

	//sum
	for (int i = 0; i < gsize.gsxyz; ++i)
		result += sumf[i];
	unsigned int num = 0;
	for (int i = 0; i < gsize.gsxyz; ++i)
		num += sumi[i];
	delete[] sumf;
	delete[] sumi;

	if (num)
		return result / num;
	return result;
}
