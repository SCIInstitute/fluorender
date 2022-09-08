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
"#define IVALT 1\n" \
"#define IVALF 3\n" \
"#define IGRDF 3\n" \
"#define IDENS 5\n" \
"#define IVRTH 6\n" \
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
"//generate param index\n" \
"__kernel void kernel_1(\n" \
"	__read_only image3d_t data,\n" \
"	__global float* hist,\n" \
"	__global float* rechist,\n" \
"	__global ushort* lut,\n" \
"	__local float* lh,\n" \
"	int3 gsxyz,\n" \
"	int3 ngxyz,\n" \
"	int3 nxyz,\n" \
"	ushort bin,\n" \
"	ushort rec)\n" \
"{\n" \
"	int3 ijk = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
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
"	ushort r = 0;\n" \
"	float* fp = rechist;\n" \
"	for (int i = 0; i < rec; ++i)\n" \
"	{\n" \
"		f1 = 0.0f;\n" \
"		for (int j = 0; j < bin; ++j)\n" \
"			f1 += lh[j] * fp[j];\n" \
"		fp += bin;\n" \
"		f2 = i ? max(f1, f2) : f1;\n" \
"		r = f1 == f2? (ushort)(i) : r;\n" \
"	}\n" \
"	uint index = nxyz.x*nxyz.y*ijk.z + nxyz.x*ijk.y + ijk.x;\n" \
"	lut[index] = r;\n" \
"}\n" \
"\n" \
"//grow by db lookup\n" \
"__kernel void kernel_2(\n" \
"	__read_only image3d_t data,\n" \
"	__global ushort* lut,\n" \
"	__global unsigned int* label,\n" \
"	__global unsigned char* df,\n" \
"	__global unsigned char* avg,\n" \
"	__global unsigned char* var,\n" \
"	__global unsigned int* rcnt,\n" \
"	__global float* params,\n" \
"	unsigned int seed,\n" \
"	int3 nxyz,\n" \
"	unsigned int dnxy,\n" \
"	unsigned int dnx,\n" \
"	float sscale,\n" \
"	unsigned int rec)\n" \
"{\n" \
"	int3 coord = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	unsigned int index = nxyz.x*nxyz.y*coord.z + nxyz.x*coord.y + coord.x;\n" \
"	unsigned int lutr = (uint)(lut[index]) * rec;\n" \
"	//float value_t = params[lutr + IVALT];\n" \
"	//float value_f = params[lutr + IVALF];\n" \
"	//float grad_f = params[lutr + IGRDF];\n" \
"	//float density = params[lutr + IDENS];\n" \
"	//float varth = params[lutr + IVRTH];\n" \
"	float value_t = 0.2f;\n" \
"	float value_f = 0.0f;\n" \
"	float grad_f = 0.0f;\n" \
"	float density = 0.1f;\n" \
"	float varth = 0.0f;\n" \
"	unsigned int label_v = label[index];\n" \
"	if (label_v == 0)\n" \
"		return;\n" \
"	//break if low density\n" \
"	if (density > 0.0f)\n" \
"	{\n" \
"		unsigned int index2 = dnxy*coord.z + dnx*coord.y + coord.x;\n" \
"		unsigned char vdf = df[index2];\n" \
"		unsigned char vavg = avg[index2];\n" \
"		unsigned char vvar = var[index2];\n" \
"		//break if low variance\n" \
"		if (vvar < varth * 255)\n" \
"			return;\n" \
"		if (vdf < vavg - (1.0f-density)*vvar)\n" \
"			return;\n" \
"	}\n" \
"	float value = read_imagef(data, samp, (int4)(coord, 1)).x;\n" \
"	value *= sscale;\n" \
"	float grad = length(sscale * vol_grad_func(data, (int4)(coord, 1)));\n" \
"	//stop function\n" \
"	float stop =\n" \
"		(grad_f>0.0f?(grad>sqrt(grad_f)*2.12f?0.0f:exp(-grad*grad/grad_f)):1.0f)*\n" \
"		(value>value_t?1.0f:(value_f>0.0f?(value<value_t-sqrt(value_f)*2.12f?0.0f:exp(-(value-value_t)*(value-value_t)/value_f)):0.0f));\n" \
"	\n" \
"	//max filter\n" \
"	atomic_inc(rcnt);\n" \
"	float random = (float)((*rcnt) % seed)/(float)(seed)+1e-4f;\n" \
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
"		if (nb_coord.x < 0 || nb_coord.x > nxyz.x-1 ||\n" \
"			nb_coord.y < 0 || nb_coord.y > nxyz.y-1 ||\n" \
"			nb_coord.z < 0 || nb_coord.z > nxyz.z-1)\n" \
"			continue;\n" \
"		nb_index = nxyz.x*nxyz.y*nb_coord.z + nxyz.x*nb_coord.y + nb_coord.x;\n" \
"		m = label[nb_index];\n" \
"		if (m <= label_v) continue;\n" \
"		label_v = m;\n" \
"	}\n" \
"	atomic_xchg(label+index, label_v);\n" \
"}\n" \
;

#endif//COMP_CL_CODE_DB_H
