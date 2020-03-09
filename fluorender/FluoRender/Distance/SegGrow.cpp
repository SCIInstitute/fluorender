/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2020 Scientific Computing and Imaging Institute,
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
#include "SegGrow.h"

using namespace FL;

const char* str_cl_segrow = \
"const sampler_t samp =\n" \
"	CLK_NORMALIZED_COORDS_FALSE|\n" \
"	CLK_ADDRESS_CLAMP_TO_EDGE|\n" \
"	CLK_FILTER_NEAREST;\n" \
"\n" \
"//initialize masked regions to ids (ordered)\n" \
"__kernel void kernel_0(\n" \
"	__read_only image3d_t mask,\n" \
"	__global unsigned int* label,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz)\n" \
"{\n" \
"	unsigned int i = (unsigned int)(get_global_id(0));\n" \
"	unsigned int j = (unsigned int)(get_global_id(1));\n" \
"	unsigned int k = (unsigned int)(get_global_id(2));\n" \
"	float value = read_imagef(mask, samp, (int4)(i, j, k, 1)).x;\n" \
"	if (value < 0.01)\n" \
"		return;\n" \
"	unsigned int index = nx*ny*k + nx*j + i;\n" \
"	atomic_xchg(label+index, index + 1);\n" \
"}\n" \
"//initialize new mask regions to ids (ordered)\n" \
"__kernel void kernel_1(\n" \
"	__read_only image3d_t mask,\n" \
"	__global unsigned int* label,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz)\n" \
"{\n" \
"	unsigned int i = (unsigned int)(get_global_id(0));\n" \
"	unsigned int j = (unsigned int)(get_global_id(1));\n" \
"	unsigned int k = (unsigned int)(get_global_id(2));\n" \
"	float value = read_imagef(mask, samp, (int4)(i, j, k, 1)).x;\n" \
"	if (value < 0.01)\n" \
"		return;\n" \
"	unsigned int index = nx*ny*k + nx*j + i;\n" \
"	if (label[index] > 0)\n" \
"		return;\n" \
"	atomic_xchg(label+index, index + 1);\n" \
"}\n" \
"//grow ids\n" \
"__kernel void kernel_2(\n" \
"	__global unsigned int* label,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz)\n" \
"{\n" \
"	int3 coord = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	unsigned int index = nx*ny*coord.z + nx*coord.y + coord.x;\n" \
"	unsigned int label_v = label[index];\n" \
"	if (label_v == 0 || label_v & 0x80000000)\n" \
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
"		if (m & 0x80000000)\n" \
"			continue;\n" \
"		if (m > label_v)\n" \
"			label_v = m;\n" \
"	}\n" \
"	atomic_xchg(label+index, label_v);\n" \
"}\n" \
"//count newly grown labels\n" \
"__kernel void kernel_3(\n" \
"	__global unsigned int* label,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	__global unsigned int* count,\n" \
"	unsigned int maxc,\n" \
"	__global unsigned int* ids)\n" \
"{\n" \
"	unsigned int i = (unsigned int)(get_global_id(0));\n" \
"	unsigned int j = (unsigned int)(get_global_id(1));\n" \
"	unsigned int k = (unsigned int)(get_global_id(2));\n" \
"	unsigned int index = nx*ny*k + nx*j + i;\n" \
"	unsigned int label_v = label[index];\n" \
"	if (label_v == 0 || label_v & 0x80000000)\n" \
"		return;\n" \
"	bool found = false;\n" \
"	for (int c = 0; c < *count; ++c)\n" \
"		if (ids[c] == label_v)\n" \
"		{\n" \
"			found = true;\n" \
"			break;\n" \
"		}\n" \
"	if (!found && *count < maxc)\n" \
"	{\n" \
"		atomic_inc(count);\n" \
"		atomic_xchg(ids+count-1, label_v);\n" \
"	}\n" \
"}\n" \
"//find connectivity/center of new ids\n" \
"__kernel void kernel_4(\n" \
"	__global unsigned int* label,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	unsigned int count,\n" \
"	unsigned int* ids,\n" \
"	__global unsigned int* cids,\n" \
"	__global unsigned int* sum,\n" \
"	__global float* csum)\n" \
"{\n" \
"	int3 coord = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	unsigned int index = nx*ny*coord.z + nx*coord.y + coord.x;\n" \
"	unsigned int label_v = label[index];\n" \
"	if (label_v == 0 || label_v & 0x80000000)\n" \
"		return;\n" \
"	int c;\n" \
"	bool found = false;\n" \
"	for (int c = 0; c < count; ++c)\n" \
"		if (ids[c] == label_v)\n" \
"		{\n" \
"			found = true;\n" \
"			break;\n" \
"		}\n" \
"	if (!found)\n" \
"		return;\n" \
"	sum[c]++;\n" \
"	csum[c*3] += (float)(coord.x);\n" \
"	csum[c*3+1] += (float)(coord.y);\n" \
"	csum[c*3+2] += (float)(coord.z);\n" \
"	unsigned int m;\n" \
"	//-x\n" \
"	if (coord.x > 0)\n" \
"	{\n" \
"		m = label[index - 1];\n" \
"		if (m != label_v)\n" \
"		{\n" \
"			cids[c] = m;\n" \
"			return;\n" \
"		}\n" \
"	}\n" \
"	//+x\n" \
"	if (coord.x < nx-1)\n" \
"	{\n" \
"		m = label[index + 1];\n" \
"		if (m != label_v)\n" \
"		{\n" \
"			cids[c] = m;\n" \
"			return;\n" \
"		}\n" \
"	}\n" \
"	//-y\n" \
"	if (coord.y > 0)\n" \
"	{\n" \
"		m = label[index - nx];\n" \
"		if (m != label_v)\n" \
"		{\n" \
"			cids[c] = m;\n" \
"			return;\n" \
"		}\n" \
"	}\n" \
"	//+y\n" \
"	if (coord.y < ny-1)\n" \
"	{\n" \
"		m = label[index + nx];\n" \
"		if (m != label_v)\n" \
"		{\n" \
"			cids[c] = m;\n" \
"			return;\n" \
"		}\n" \
"	}\n" \
"	//-z\n" \
"	if (coord.z > 0)\n" \
"	{\n" \
"		m = label[index - nx*ny];\n" \
"		if (m != label_v)\n" \
"		{\n" \
"			cids[c] = m;\n" \
"			return;\n" \
"		}\n" \
"	}\n" \
"	//+z\n" \
"	if (coord.z < nz-1)\n" \
"	{\n" \
"		m = label[index + nx*ny];\n" \
"		if (m != label_v)\n" \
"		{\n" \
"			cids[c] = m;\n" \
"			return;\n" \
"		}\n" \
"	}\n" \
"}\n" \
"//fix processed ids\n" \
"__kernel void kernel_6(\n" \
"	__global unsigned int* label,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz)\n" \
"{\n" \
"	unsigned int i = (unsigned int)(get_global_id(0));\n" \
"	unsigned int j = (unsigned int)(get_global_id(1));\n" \
"	unsigned int k = (unsigned int)(get_global_id(2));\n" \
"	unsigned int index = nx*ny*k + nx*j + i;\n" \
"	unsigned int label_v = label[index];\n" \
"	if (label_v == 0 || label_v & 0x80000000)\n" \
"		return;\n" \
"	label[index] = label[index] | 0x80000000;\n" \
"}\n" \
;


SegGrow::SegGrow(VolumeData* vd):
	m_vd(vd)
{

}

SegGrow::~SegGrow()
{

}

void SegGrow::Init()
{

}

void SegGrow::Compute()
{

}
