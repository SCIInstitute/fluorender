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
"#define IITER 0\n" \
"#define IVALT 1\n" \
"#define IVALF 3\n" \
"#define IGRDF 3\n" \
"#define IDENS 5\n" \
"#define IVRTH 6\n" \
"#define IDENSF 7\n" \
"#define IDENSW 8\n" \
"#define IDMIX 10\n" \
"#define IDISTH 11\n" \
"#define IDISTF 12\n" \
"const sampler_t samp =\n" \
"	CLK_NORMALIZED_COORDS_FALSE|\n" \
"	CLK_ADDRESS_CLAMP_TO_EDGE|\n" \
"	CLK_FILTER_NEAREST;\n" \
"\n" \
"float get_2d_density(image3d_t image, int4 pos, int r)\n" \
"{\n" \
"	float sum = 0.0f;\n" \
"	int d = 2*r+1;\n" \
"	for (int i=-r; i<=r; ++i)\n" \
"	for (int j=-r; j<=r; ++j)\n" \
"		sum += read_imagef(image, samp, pos+(int4)(i, j, 0, 0)).x;\n" \
"	return sum / (float)(d * d);\n" \
"}\n" \
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
"//generate param index by direct hist\n" \
"__kernel void kernel_0(\n" \
"	__read_only image3d_t data,\n" \
"	__global float* rechist,\n" \
"	__global ushort* lut,\n" \
"	__local float* lh,\n" \
"	int3 histxyz,\n" \
"	int3 nxyz,\n" \
"	float minv,\n" \
"	float maxv,\n" \
"	ushort bin,\n" \
"	ushort rec)\n" \
"{\n" \
"	int3 gid = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	int3 lb = gid - histxyz / 2;\n" \
"	lb = max(lb, (int3)(0));\n" \
"	int3 ub = lb + histxyz;\n" \
"	ub = min(ub, nxyz - (int3)(1));\n" \
"	lb = min(lb, ub - histxyz);\n" \
"	uint index;\n" \
"	for (index = 0; index < bin; ++ index)\n" \
"		lh[index] = 0.0f;\n" \
"	int3 ijk;\n" \
"	float val;\n" \
"	float popl = 0.0f;\n" \
"	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)\n" \
"	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)\n" \
"	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)\n" \
"	{\n" \
"		val = read_imagef(data, samp, (int4)(ijk, 1)).x;\n" \
"		index = (val - minv) * (bin - 1) / (maxv - minv);\n" \
"		lh[index] += 1.0f;\n" \
"		popl += 1.0f;\n" \
"	}\n" \
"	for (index = 0; index < bin; ++ index)\n" \
"		lh[index] /= popl;\n" \
"	float f1, f2;\n" \
"	ushort r = 0;\n" \
"	float* fp = rechist;\n" \
"	for (int i = 0; i < rec; ++i)\n" \
"	{\n" \
"		f1 = 0.0f;\n" \
"		for (int j = 0; j < bin; ++j)\n" \
"			f1 += (lh[j] - fp[j]) * (lh[j] - fp[j]);\n" \
"		fp += bin;\n" \
"		f2 = i ? min(f1, f2) : f1;\n" \
"		r = f1 == f2? (ushort)(i) : r;\n" \
"	}\n" \
"	index = nxyz.x*nxyz.y*gid.z + nxyz.x*gid.y + gid.x;\n" \
"	lut[index] = (ushort)(r);\n" \
"}\n" \
"\n" \
"//init dist field by db lookup\n" \
"__kernel void kernel_1(\n" \
"	__read_only image3d_t data,\n" \
"	__global ushort* lut,\n" \
"	__global float* params,\n" \
"	__global uchar* df,\n" \
"	float sscale,\n" \
"	uchar ini,\n" \
"	int3 nxyz,\n" \
"	uint nxy,\n" \
"	uint npar)\n" \
"{\n" \
"	int3 ijk = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	uint index = nxy*ijk.z + nxyz.x*ijk.y + ijk.x;\n" \
"	if (ijk.x == 0 || ijk.x == nxyz.x-1 ||\n" \
"		ijk.y == 0 || ijk.y == nxyz.y-1)\n" \
"	{\n" \
"		df[index] = 0;\n" \
"		return;\n" \
"	}\n" \
"	uint lutr = (uint)(lut[index]) * npar;\n" \
"	int dsize = (int)(params[lutr + IDISTF]);\n" \
"	float dth = params[lutr + IDISTF];\n" \
"	float dval = get_2d_density(data, (int4)(ijk, 1), dsize);\n" \
"	dval *= sscale;\n" \
"	if (dval > dth)\n" \
"		df[index] = ini;\n" \
"	else\n" \
"		df[index] = 0;\n" \
"}\n" \
"\n" \
"//generate dist field\n" \
"__kernel void kernel_2(\n" \
"	__global uchar* df,\n" \
"	uchar ini,\n" \
"	int3 nxyz,\n" \
"	uint nxy,\n" \
"	uchar nn,\n" \
"	uchar re)\n" \
"{\n" \
"	int3 ijk = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	uint index = nxy*ijk.z + nxyz.x*ijk.y + ijk.x;\n" \
"	if (df[index] == ini)\n" \
"	{\n" \
"		short v1 = df[nxy*ijk.z + nxyz.x*ijk.y + ijk.x - 1];\n" \
"		short v2 = df[nxy*ijk.z + nxyz.x*ijk.y + ijk.x + 1];\n" \
"		short v3 = df[nxy*ijk.z + nxyz.x*(ijk.y-1) + ijk.x];\n" \
"		short v4 = df[nxy*ijk.z + nxyz.x*(ijk.y+1) + ijk.x];\n" \
"		short rre = (ijk.x % 13 + ijk.y % 17) % 4;\n" \
"		v1 = rre == 0 ? -1 : v1;\n" \
"		v2 = rre == 3 ? -1 : v2;\n" \
"		v3 = rre == 1 ? -1 : v3;\n" \
"		v4 = rre == 2 ? -1 : v4;\n" \
"		if (v1 == nn || v2 == nn ||\n" \
"			v3 == nn || v4 == nn)\n" \
"			df[index] = re;\n" \
"	}\n" \
"}\n" \
"\n" \
"//generate density field mixed with dist field\n" \
"__kernel void kernel_3(\n" \
"	__read_only image3d_t data,\n" \
"	__global unsigned char* distf,\n" \
"	__global unsigned char* densf,\n" \
"	unsigned int nxy,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	int dsize,\n" \
"	float sscale,\n" \
"	float distscl,\n" \
"	float dist_strength)\n" \
"{\n" \
"	int3 ijk = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	float density = get_2d_density(data, (int4)(ijk, 1), dsize) * sscale;\n" \
"	unsigned int index = nxy*clamp(ijk.z, 0, (int)(nz-1)) +\n" \
"		nx*clamp(ijk.y, 0, (int)(ny-1)) + clamp(ijk.x, 0, (int)(nx-1));\n" \
"	float distv = distscl * distf[index];\n" \
"	density = density * (1.0f - dist_strength) + distv * dist_strength;\n" \
"	index = nxy*ijk.z + nx*ijk.y + ijk.x;\n" \
"	densf[index] = (unsigned char)(density * 255.0f);\n" \
"}\n" \
"\n" \
"//generate statistics on density field\n" \
"__kernel void kernel_4(\n" \
"	__global unsigned char* df,\n" \
"	__global unsigned char* avg,\n" \
"	__global unsigned char* var,\n" \
"	int3 histxyz,\n" \
"	int3 nxyz,\n" \
"	uint nxy)\n" \
"{\n" \
"	int3 gid = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	int3 lb = gid - histxyz / 2;\n" \
"	int3 ub = lb + histxyz;\n" \
"	lb = clamp(lb, (int3)(0), nxyz - (int3)(1));\n" \
"	ub = clamp(ub, (int3)(0), nxyz - (int3)(1));\n" \
"	int3 ijk;\n" \
"	float gnum = 0.0f;\n" \
"	float sum = 0.0f;\n" \
"	float sum2 = 0.0f;\n" \
"	float v;\n" \
"	uint index;\n" \
"	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)\n" \
"	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)\n" \
"	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)\n" \
"	{\n" \
"		index = nxy*ijk.z + nxyz.x*ijk.y + ijk.x;\n" \
"		v = df[index];\n" \
"		sum += v;\n" \
"		sum2 += v * v;\n" \
"		gnum += 1.0f;\n" \
"	}\n" \
"	index = nxy * gid.z + nxyz.x * gid.y + gid.x;\n" \
"	v = sum / gnum;\n" \
"	avg[index] = v;\n" \
"	v = clamp(sqrt((sum2 + v * v * gnum - 2.0f * v * sum) / gnum), 0.0f, 255.0f);\n" \
"	var[index] = v;\n" \
"}\n" \
"\n" \
"//grow by db lookup\n" \
"__kernel void kernel_5(\n" \
"	float iter,\n" \
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
"	uint nxy,\n" \
"	float sscale,\n" \
"	unsigned int npar)\n" \
"{\n" \
"	int3 coord = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	unsigned int index = nxy*coord.z + nxyz.x*coord.y + coord.x;\n" \
"	unsigned int lutr = (uint)(lut[index]) * npar;\n" \
"	if (params[lutr + IITER] < iter) return;\n" \
"	float value_t = params[lutr + IVALT];\n" \
"	float value_f = params[lutr + IVALF];\n" \
"	float grad_f = params[lutr + IGRDF];\n" \
"	float density = params[lutr + IDENS];\n" \
"	float varth = params[lutr + IVRTH];\n" \
"	unsigned int label_v = label[index];\n" \
"	if (label_v == 0)\n" \
"		return;\n" \
"	//break if low density\n" \
"	if (density > 0.0f)\n" \
"	{\n" \
"		unsigned int index2 = nxy*coord.z + nxyz.x*coord.y + coord.x;\n" \
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
"		nb_index = nxy*nb_coord.z + nxyz.x*nb_coord.y + nb_coord.x;\n" \
"		m = label[nb_index];\n" \
"		if (m <= label_v) continue;\n" \
"		label_v = m;\n" \
"	}\n" \
"	atomic_xchg(label+index, label_v);\n" \
"}\n" \
;

const char* str_cl_comp_gen_db_unused = \
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
"//generate param index by interpolation\n" \
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
"	uint index = nxyz.x*nxyz.y*ijk.z + nxyz.x*ijk.y + ijk.x;\n" \
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
"	lut[index] = r;\n" \
"}\n" \
"\n" \
;

#endif//COMP_CL_CODE_DB_H
