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
const char* str_cl_chann_compare = \
"const sampler_t samp =\n" \
"	CLK_NORMALIZED_COORDS_FALSE|\n" \
"	CLK_ADDRESS_CLAMP_TO_EDGE|\n" \
"	CLK_FILTER_NEAREST;\n" \
"\n" \
"\n" \
"__kernel void kernel_0(\n" \
"	__read_only image3d_t chann1,\n" \
"	__read_only image3d_t chann2,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	float th1,\n" \
"	float th2,\n" \
"	__global unsigned int* count)\n" \
"{\n" \
"	int3 coord = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	//unsigned int index = nx*ny*coord.z + nx*coord.y + coord.x;\n" \
"	float value1 = read_imagef(chann1, samp, (int4)(coord, 1)).x;\n" \
"	float value2 = read_imagef(chann2, samp, (int4)(coord, 1)).x;\n" \
"	if (value1 > th1 && value2 > th2)\n" \
"		atomic_inc(count);\n" \
"}\n";

const char* str_cl_chann_dotprod = \
"const sampler_t samp =\n" \
"CLK_NORMALIZED_COORDS_FALSE |\n" \
"CLK_ADDRESS_CLAMP |\n" \
"CLK_FILTER_NEAREST;\n" \
"\n" \
"__kernel void kernel_0(\n" \
"	__read_only image3d_t chann1,\n" \
"	__read_only image3d_t chann2,\n" \
"	unsigned int ngx,\n" \
"	unsigned int ngy,\n" \
"	unsigned int ngz,\n" \
"	unsigned int gsxy,\n" \
"	unsigned int gsx,\n" \
"	__global float* sum)\n" \
"{\n" \
"	int3 gid = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);\n" \
"	int3 ub = (int3)(lb.x + ngx, lb.y + ngy, lb.z + ngz);\n" \
"	int4 ijk = (int4)(0, 0, 0, 1);\n" \
"	float lsum = 0.0;\n" \
"	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)\n" \
"		for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)\n" \
"			for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)\n" \
"			{\n" \
"				float v1 = read_imagef(chann1, samp, ijk).x;\n" \
"				float v2 = read_imagef(chann2, samp, ijk).x;\n" \
"				lsum += v1 * v2;\n" \
"			}\n" \
"	unsigned int index = gsxy * gid.z + gsx * gid.y + gid.x;\n" \
"	sum[index] = lsum;\n" \
"}\n";