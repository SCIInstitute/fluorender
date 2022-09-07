/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2022 Scientific Computing and Imaging Institute,
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
#ifndef COMP_CL_CODE_DB_H
#define COMP_CL_CODE_DB_H

const char* str_cl_comp_gen_db = \
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
"//compute histogram in blocks\n" \
"__kernel void kernel_0(\n" \
"	__read_only image3d_t data,\n" \
"	unsigned int gsx,\n" \
"	unsigned int gsy,\n" \
"	unsigned int gsz,\n" \
"	unsigned int ngxy,\n" \
"	unsigned int ngx,\n" \
"	float minv,\n" \
"	float maxv,\n" \
"	unsigned int bin,\n" \
"	__global unsigned int* hist)\n" \
"{\n" \
"	int3 gid = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	int3 lb = (int3)(gid.x*gsx, gid.y*gsy, gid.z*gsz);\n" \
"	int3 ub = (int3)(lb.x + gsx, lb.y + gsy, lb.z + gsz);\n" \
"	int3 ijk = (int3)(0);\n" \
"	uint2 index;//x:hist;y:block\n" \
"	index.y = ngxy * gid.z + ngx * gid.y + gid.x;\n" \
"	index.y *= bin + 1;\n" \
"	float val;\n" \
"	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)\n" \
"	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)\n" \
"	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)\n" \
"	{\n" \
"		val = read_imagef(data, samp, (int4)(ijk, 1)).x;\n" \
"		index.x = (val - minv) * (bin - 1) / (maxv - minv);\n" \
"		atomic_inc(hist+index.y+index.x);\n" \
"		atomic_inc(hist+index.y+bin);\n" \
"	}\n" \
"}\n" \
"\n" \
"//get record index\n" \
"unsigned int get_rec(\n" \
"	__global float* hist,\n" \
"	__global float* rechist,\n" \
"	__local float* lh,\n" \
"	int3 ijk,\n" \
"	int3 gsxyz,\n" \
"	int3 ngxyz,\n" \
"	unsigned int bin,\n" \
"	unsigned int rec)\n" \
"{\n" \
"	ijk -= (int3)(gsxyz.xy / 2, 0);\n" \
"	int3 hc = ijk / gsxyz;\n" \
"	int3 hd = ijk % gsxyz;\n" \
"	int4 hi;\n" \
"	int3 hc2 = hc;\n" \
"	hc2 = clamp(hc2, (int3)(0), ngxyz - (int3)(1));\n" \
"	hi.x = hc2.z * ngxyz.x * ngxyz.y + hc2.y * ngxyz.x + hc2.x;\n" \
"	hc2 = hc + (int3)(1, 0, 0);\n" \
"	hc2 = clamp(hc2, (int3)(0), ngxyz - (int3)(1));\n" \
"	hi.y = hc2.z * ngxyz.x * ngxyz.y + hc2.y * ngxyz.x + hc2.x;\n" \
"	hc2 = hc + (int3)(0, 1, 0);\n" \
"	hc2 = clamp(hc2, (int3)(0), ngxyz - (int3)(1));\n" \
"	hi.z = hc2.z * ngxyz.x * ngxyz.y + hc2.y * ngxyz.x + hc2.x;\n" \
"	hc2 = hc + (int3)(0, 1, 0);\n" \
"	hc2 = clamp(hc2, (int3)(0), ngxyz - (int3)(1));\n" \
"	hi.w = hc2.z * ngxyz.x * ngxyz.y + hc2.y * ngxyz.x + hc2.x;\n" \
"	hi *= (int)(bin);\n" \
"	float4 ft;\n" \
"	ft.x = (float)(hd.x) / (float)(gsxyz.x);\n" \
"	ft.y = 1.0f - ft.x;\n" \
"	ft.z = (float)(hd.y) / (float)(gsxyz.y);\n" \
"	ft.w = 1.0f - ft.z;\n" \
"	float f1, f2;\n" \
"	for (int i = 0; i < bin; ++i)\n" \
"	{\n" \
"		f1 = hist[hi.x + i] * ft.y + hist[hi.y + i] * ft.x;\n" \
"		f2 = hist[hi.z + i] * ft.y + hist[hi.w + i] * ft.x;\n" \
"		lh[i] = f1 * ft.w + f2 * ft.z;\n" \
"	}\n" \
"	unsigned int r = 0;\n" \
"	float* fp = rechist;\n" \
"	for (int i = 0; i < rec; ++i)\n" \
"	{\n" \
"		f1 = 0.0f;\n" \
"		for (int j = 0; j < bin; ++j)\n" \
"			f1 += lh[j] * fp[j];\n" \
"		fp += bin;\n" \
"		f2 = i ? max(f1, f2) : f1;\n" \
"		r = f1 == f2? i : r;\n" \
"	}\n" \
"	return r;\n" \
"}\n" \
"\n" \
"//grow by db lookup\n" \
"__kernel void kernel_1(\n" \
"	__read_only image3d_t data,\n" \
"	__global unsigned int* label,\n" \
"	__global unsigned char* df,\n" \
"	__global unsigned char* avg,\n" \
"	__global unsigned char* var,\n" \
"	__global unsigned int* rcnt,\n" \
"	__global float* hist,\n" \
"	__global float* rechist,\n" \
"	__global float* params,\n" \
"	__local float* lh,\n" \
"	unsigned int seed,\n" \
"	int3 gsxyz,\n" \
"	int3 ngxyz,\n" \
"	int3 nxyz,\n" \
"	unsigned int nxy,\n" \
"	unsigned int bin,\n" \
"	unsigned int rec)\n" \
"{\n" \
"	int3 coord = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	//unsigned int index = get_rec(hist, rechist, lh, coord, gsxyz, ngxyz, bin, rec);\n" \
"}\n" \
;

#endif//COMP_CL_CODE_DB_H
