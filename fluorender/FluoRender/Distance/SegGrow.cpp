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
#include <Distance/RulerHandler.h>
#include <FLIVR/VolKernel.h>
#include <algorithm>
#include <unordered_map>
#include <Debug.h>

using namespace flrd;

const char* str_cl_segrow = \
"const sampler_t samp =\n" \
"	CLK_NORMALIZED_COORDS_FALSE|\n" \
"	CLK_ADDRESS_CLAMP_TO_EDGE|\n" \
"	CLK_FILTER_NEAREST;\n" \
"\n" \
"//initialize new mask regions to ids (ordered)\n" \
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
"	unsigned int index = nx*ny*k + nx*j + i;\n" \
"	unsigned int lv = index + 1;\n" \
"	if (value == 0.0f)\n" \
"		lv = 0;\n" \
"	atomic_xchg(label+index, lv);\n" \
"}\n" \
"//initialize but keep old values\n" \
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
"	if (value == 0.0f)\n" \
"		return;\n" \
"	unsigned int index = nx*ny*k + nx*j + i;\n" \
"	if (label[index] > 0)\n" \
"		return;\n" \
"	atomic_xchg(label+index, index + 1);\n" \
"}\n" \
"//grow ids reverse\n" \
"__kernel void kernel_2(\n" \
"	__global unsigned int* label,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nxy,\n" \
"	unsigned int nz,\n" \
"	unsigned int ngx,\n" \
"	unsigned int ngy,\n" \
"	unsigned int ngz)\n" \
"{\n" \
"	int3 gid = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);\n" \
"	int3 ub = (int3)(lb.x + ngx - 1, lb.y + ngy - 1, lb.z + ngz - 1);\n" \
"	int3 ijk = (int3)(0, 0, 0);\n" \
"	unsigned int index;\n" \
"	unsigned int label_v;\n" \
"	unsigned int m, label_m;\n" \
"	for (ijk.z = ub.z; ijk.z >= lb.z; --ijk.z)\n" \
"	for (ijk.y = ub.y; ijk.y >= lb.y; --ijk.y)\n" \
"	for (ijk.x = ub.x; ijk.x >= lb.x; --ijk.x)\n" \
"	{\n" \
"		if (ijk.x >= nx || ijk.y >= ny || ijk.z >= nz)\n" \
"			continue;\n" \
"		index = nxy*ijk.z + nx*ijk.y + ijk.x;\n" \
"		label_v = label[index];\n" \
"		if (label_v == 0 || label_v & 0x80000000)\n" \
"			continue;\n" \
"		label_m = label_v;\n" \
"		//search neighbors\n" \
"		for (int i = -1; i < 2; ++i)\n" \
"		for (int j = -1; j < 2; ++j)\n" \
"		for (int k = -1; k < 2; ++k)\n" \
"		{\n" \
"			if (ijk.x < 1 || ijk.x > nx-2 || ijk.y < 1 || ijk.y > ny-2 || ijk.z < 1 || ijk.z > nz-2)\n" \
"				continue;\n" \
"			m = label[nxy*(ijk.z+i) + nx*(ijk.y+j) + ijk.x + k];\n" \
"			if (m  && !(m & 0x80000000))\n" \
"				label_m = max(label_m, m);\n" \
"		}\n" \
"		if (label_m != label_v)\n" \
"			label[index] = label_m;\n" \
"	}\n" \
"}\n" \
"//grow id ordered\n" \
"__kernel void kernel_3(\n" \
"	__global unsigned int* label,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nxy,\n" \
"	unsigned int nz,\n" \
"	unsigned int ngx,\n" \
"	unsigned int ngy,\n" \
"	unsigned int ngz)\n" \
"{\n" \
"	int3 gid = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);\n" \
"	int3 ub = (int3)(lb.x + ngx, lb.y + ngy, lb.z + ngz);\n" \
"	int3 ijk = (int3)(0, 0, 0);\n" \
"	unsigned int index;\n" \
"	unsigned int label_v;\n" \
"	unsigned int m, label_m;\n" \
"	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)\n" \
"	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)\n" \
"	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)\n" \
"	{\n" \
"		if (ijk.x >= nx || ijk.y >= ny || ijk.z >= nz)\n" \
"			continue;\n" \
"		index = nxy*ijk.z + nx*ijk.y + ijk.x;\n" \
"		label_v = label[index];\n" \
"		if (label_v == 0 || label_v & 0x80000000)\n" \
"			continue;\n" \
"		label_m = label_v;\n" \
"		//search neighbors\n" \
"		for (int i = -1; i < 2; ++i)\n" \
"		for (int j = -1; j < 2; ++j)\n" \
"		for (int k = -1; k < 2; ++k)\n" \
"		{\n" \
"			if (ijk.x < 1 || ijk.x > nx-2 || ijk.y < 1 || ijk.y > ny-2 || ijk.z < 1 || ijk.z > nz-2)\n" \
"				continue;\n" \
"			m = label[nxy*(ijk.z+i) + nx*(ijk.y+j) + ijk.x + k];\n" \
"			if (m  && !(m & 0x80000000))\n" \
"				label_m = max(label_m, m);\n" \
"		}\n" \
"		if (label_m != label_v)\n" \
"			label[index] = label_m;\n" \
"	}\n" \
"}\n" \
"//count newly grown labels\n" \
"__kernel void kernel_4(\n" \
"	__global unsigned int* label,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nxy,\n" \
"	unsigned int nz,\n" \
"	unsigned int ngx,\n" \
"	unsigned int ngy,\n" \
"	unsigned int ngz,\n" \
"	unsigned int gsxy,\n" \
"	unsigned int gsx,\n" \
"	unsigned int maxc,\n" \
"	__global unsigned int* count,\n" \
"	__global unsigned int* ids,\n" \
"	__local unsigned int* lids)\n" \
"{\n" \
"	int3 gid = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);\n" \
"	int3 ub = (int3)(lb.x + ngx, lb.y + ngy, lb.z + ngz);\n" \
"	int3 ijk = (int3)(0, 0, 0);\n" \
"	unsigned int lcount = 0;\n" \
"	unsigned int index;\n" \
"	unsigned int label_v;\n" \
"	bool found;\n" \
"	int c;\n" \
"	for (c = 0; c < maxc; ++c)\n" \
"		lids[c] = 0;\n" \
"	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)\n" \
"	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)\n" \
"	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)\n" \
"	{\n" \
"		if (ijk.x >= nx || ijk.y >= ny || ijk.z >= nz)\n" \
"			continue;\n" \
"		index = nxy*ijk.z + nx*ijk.y + ijk.x;\n" \
"		label_v = label[index];\n" \
"		if (label_v == 0 || label_v & 0x80000000)\n" \
"			continue;\n" \
"		found = false;\n" \
"		for (c = 0; c < lcount; ++c)\n" \
"		if (lids[c] == label_v)\n" \
"		{\n" \
"			found = true;\n" \
"			break;\n" \
"		}\n" \
"		if (!found && lcount < maxc)\n" \
"		{\n" \
"			lids[lcount] = label_v;\n" \
"			lcount++;\n" \
"		}\n" \
"	}\n" \
"	index = gsxy * gid.z + gsx * gid.y + gid.x;\n" \
"	atomic_xchg(count+index, lcount);\n" \
"	for (c = 0; c < lcount; ++c)\n" \
"		atomic_xchg(ids+index*maxc+c, lids[c]);\n" \
"}\n" \
"//find connected parts\n" \
"__kernel void kernel_5(\n" \
"	__global unsigned int* label,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nxy,\n" \
"	unsigned int nz,\n" \
"	unsigned int ngx,\n" \
"	unsigned int ngy,\n" \
"	unsigned int ngz,\n" \
"	unsigned int gsxy,\n" \
"	unsigned int gsx,\n" \
"	unsigned int nid,\n" \
"	__global unsigned int* ids,\n" \
"	__global unsigned int* cids,\n" \
"	__local unsigned int* lcids)\n" \
"{\n" \
"	int3 gid = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);\n" \
"	int3 ub = (int3)(lb.x + ngx, lb.y + ngy, lb.z + ngz);\n" \
"	int3 ijk = (int3)(0, 0, 0);\n" \
"	unsigned int index;\n" \
"	unsigned int label_v;\n" \
"	unsigned int m;\n" \
"	bool found;\n" \
"	int c;\n" \
"	for (c = 0; c < nid * 6; ++c)\n" \
"		lcids[c] = 0;\n" \
"	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)\n" \
"	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)\n" \
"	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)\n" \
"	{\n" \
"		if (ijk.x >= nx || ijk.y >= ny || ijk.z >= nz)\n" \
"			continue;\n" \
"		index = nxy*ijk.z + nx*ijk.y + ijk.x;\n" \
"		label_v = label[index];\n" \
"		if (label_v == 0 || label_v & 0x80000000)\n" \
"			continue;\n" \
"		found = false;\n" \
"		for (c = 0; c < nid; ++c)\n" \
"		if (ids[c] == label_v)\n" \
"		{\n" \
"			found = true;\n" \
"			break;\n" \
"		}\n" \
"		if (!found)\n" \
"			continue;\n" \
"		//-x\n" \
"		if (ijk.x > 0)\n" \
"		{\n" \
"			m = label[index - 1];\n" \
"			if (m != label_v && !(m & 0x80000000))\n" \
"				lcids[c*6] = m;\n" \
"		}\n" \
"		//+x\n" \
"		if (ijk.x < nx-1)\n" \
"		{\n" \
"			m = label[index + 1];\n" \
"			if (m != label_v && !(m & 0x80000000))\n" \
"				lcids[c*6+1] = m;\n" \
"		}\n" \
"		//-y\n" \
"		if (ijk.y > 0)\n" \
"		{\n" \
"			m = label[index - nx];\n" \
"			if (m != label_v && !(m & 0x80000000))\n" \
"				lcids[c*6+2] = m;\n" \
"		}\n" \
"		//+y\n" \
"		if (ijk.y < ny-1)\n" \
"		{\n" \
"			m = label[index + nx];\n" \
"			if (m != label_v && !(m & 0x80000000))\n" \
"				lcids[c*6+3] = m;\n" \
"		}\n" \
"		//-z\n" \
"		if (ijk.z > 0)\n" \
"		{\n" \
"			m = label[index - nxy];\n" \
"			if (m != label_v && !(m & 0x80000000))\n" \
"				lcids[c*6+4] = m;\n" \
"		}\n" \
"		//+z\n" \
"		if (ijk.z < nz-1)\n" \
"		{\n" \
"			m = label[index + nxy];\n" \
"			if (m != label_v && !(m & 0x80000000))\n" \
"				lcids[c*6+5] = m;\n" \
"		}\n" \
"	}\n" \
"	index = gsxy * gid.z + gsx * gid.y + gid.x;\n" \
"	for (c = 0; c < nid*6; ++c)\n" \
"		atomic_xchg(cids+(index*nid)*6+c, lcids[c]);\n" \
"}\n" \
"//merge connected ids\n" \
"__kernel void kernel_6(\n" \
"	__global unsigned int* label,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nxy,\n" \
"	unsigned int nz,\n" \
"	unsigned int ngx,\n" \
"	unsigned int ngy,\n" \
"	unsigned int ngz,\n" \
"	unsigned int nid,\n" \
"	__global unsigned int* mids)\n" \
"{\n" \
"	int3 gid = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);\n" \
"	int3 ub = (int3)(lb.x + ngx, lb.y + ngy, lb.z + ngz);\n" \
"	int3 ijk = (int3)(0, 0, 0);\n" \
"	unsigned int index;\n" \
"	unsigned int label_v;\n" \
"	bool found;\n" \
"	int c;\n" \
"	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)\n" \
"	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)\n" \
"	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)\n" \
"	{\n" \
"		if (ijk.x >= nx || ijk.y >= ny || ijk.z >= nz)\n" \
"			continue;\n" \
"		index = nxy*ijk.z + nx*ijk.y + ijk.x;\n" \
"		label_v = label[index];\n" \
"		if (label_v == 0 || label_v & 0x80000000)\n" \
"			continue;\n" \
"		found = false;\n" \
"		for (c = 0; c < nid; ++c)\n" \
"		if (mids[c*2] == label_v)\n" \
"		{\n" \
"			found = true;\n" \
"			break;\n" \
"		}\n" \
"		if (!found)\n" \
"			continue;\n" \
"		label[index] = mids[c*2+1];\n" \
"	}\n" \
"}\n" \
"//find connectivity/center of new ids\n" \
"__kernel void kernel_7(\n" \
"	__global unsigned int* label,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nxy,\n" \
"	unsigned int nz,\n" \
"	unsigned int ngx,\n" \
"	unsigned int ngy,\n" \
"	unsigned int ngz,\n" \
"	unsigned int gsxy,\n" \
"	unsigned int gsx,\n" \
"	unsigned int nid,\n" \
"	__global unsigned int* ids,\n" \
"	__global unsigned int* cids,\n" \
"	__global unsigned int* sum,\n" \
"	__global float* csum,\n" \
"	__local unsigned int* lcids,\n" \
"	__local unsigned int* lsum,\n" \
"	__local float* lcsum)\n" \
"{\n" \
"	int3 gid = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);\n" \
"	int3 ub = (int3)(lb.x + ngx, lb.y + ngy, lb.z + ngz);\n" \
"	int3 ijk = (int3)(0, 0, 0);\n" \
"	unsigned int index;\n" \
"	unsigned int label_v;\n" \
"	unsigned int m;\n" \
"	bool found;\n" \
"	int c;\n" \
"	for (c = 0; c < nid; ++c)\n" \
"	{\n" \
"		lcids[c*3] = 0;\n" \
"		lcids[c*3+1] = 0;\n" \
"		lcids[c*3+2] = 0;\n" \
"		lsum[c] = 0;\n" \
"		lcsum[c*3] = 0.0f;\n" \
"		lcsum[c*3+1] = 0.0f;\n" \
"		lcsum[c*3+2] = 0.0f;\n" \
"	}\n" \
"	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)\n" \
"	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)\n" \
"	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)\n" \
"	{\n" \
"		if (ijk.x >= nx || ijk.y >= ny || ijk.z >= nz)\n" \
"			continue;\n" \
"		index = nxy*ijk.z + nx*ijk.y + ijk.x;\n" \
"		label_v = label[index];\n" \
"		if (label_v == 0 || label_v & 0x80000000)\n" \
"			continue;\n" \
"		found = false;\n" \
"		for (c = 0; c < nid; ++c)\n" \
"		if (ids[c] == label_v)\n" \
"		{\n" \
"			found = true;\n" \
"			break;\n" \
"		}\n" \
"		if (!found)\n" \
"			continue;\n" \
"		lsum[c]++;\n" \
"		lcsum[c*3] += (float)(ijk.x);\n" \
"		lcsum[c*3+1] += (float)(ijk.y);\n" \
"		lcsum[c*3+2] += (float)(ijk.z);\n" \
"		//search neighbors\n" \
"		for (int i = -1; i < 2; ++i)\n" \
"		for (int j = -1; j < 2; ++j)\n" \
"		for (int k = -1; k < 2; ++k)\n" \
"		{\n" \
"			if (ijk.x < 1 || ijk.x > nx-2 || ijk.y < 1 || ijk.y > ny-2 || ijk.z < 1 || ijk.z > nz-2)\n" \
"				continue;\n" \
"			m = label[nxy*(ijk.z+i) + nx*(ijk.y+j) + ijk.x + k];\n" \
"			if (m != label_v && m & 0x80000000)\n" \
"			{\n" \
"				lcids[c*3] = lcids[c*3]?lcids[c*3]:m;\n" \
"				lcids[c*3+1] = lcids[c*3]&&!lcids[c*3+1]&&lcids[c*3]!=m?m:lcids[c*3+1];\n" \
"				lcids[c*3+2] = lcids[c*3+1]&&!lcids[c*3+2]&&lcids[c*3+1]!=m?m:lcids[c*3+2];\n" \
"			}\n" \
"		}\n" \
"	}\n" \
"	index = gsxy * gid.z + gsx * gid.y + gid.x;\n" \
"	for (c = 0; c < nid; ++c)\n" \
"	{\n" \
"		atomic_xchg(cids+(index*nid+c)*3, lcids[c*3]);\n" \
"		atomic_xchg(cids+(index*nid+c)*3+1, lcids[c*3+1]);\n" \
"		atomic_xchg(cids+(index*nid+c)*3+2, lcids[c*3+2]);\n" \
"		atomic_xchg(sum+index*nid+c, lsum[c]);\n" \
"		atomic_xchg(csum+(index*nid+c)*3, lcsum[c*3]);\n" \
"		atomic_xchg(csum+(index*nid+c)*3+1, lcsum[c*3+1]);\n" \
"		atomic_xchg(csum+(index*nid+c)*3+2, lcsum[c*3+2]);\n" \
"	}\n" \
"}\n" \
"//fix processed ids\n" \
"__kernel void kernel_8(\n" \
"	__global unsigned int* label,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz)\n" \
"{\n" \
"	unsigned int i = (unsigned int)(get_global_id(0));\n" \
"	unsigned int j = (unsigned int)(get_global_id(1));\n" \
"	unsigned int k = (unsigned int)(get_global_id(2));\n" \
"	if (i >= nx || j >= ny || k >= nz)\n" \
"		return;\n" \
"	unsigned int index = nx*ny*k + nx*j + i;\n" \
"	unsigned int label_v = label[index];\n" \
"	if (label_v == 0 || label_v & 0x80000000)\n" \
"		return;\n" \
"	label[index] = label[index] | 0x80000000;\n" \
"}\n" \
;

const char* str_cl_sg_check_borders = \
"const sampler_t samp =\n" \
"	CLK_NORMALIZED_COORDS_FALSE|\n" \
"	CLK_ADDRESS_CLAMP_TO_EDGE|\n" \
"	CLK_FILTER_NEAREST;\n" \
"\n" \
"//check yz plane (+-x)\n" \
"__kernel void kernel_0(\n" \
"	__read_only image3d_t l0,\n" \
"	__read_only image3d_t l1,\n" \
"	unsigned int d0,\n" \
"	unsigned int d1,\n" \
"	unsigned int nid,\n" \
"	__global unsigned int* ids,\n" \
"	__global unsigned int* cids)\n" \
"{\n" \
"	unsigned int i = (unsigned int)(get_global_id(0));\n" \
"	unsigned int j = (unsigned int)(get_global_id(1));\n" \
"	unsigned int v0 = read_imageui(l0, samp, (int4)(d0, i, j, 1)).x;\n" \
"	if (v0 == 0 || v0 & 0x80000000)\n" \
"		return;\n" \
"	unsigned int v1 = read_imageui(l1, samp, (int4)(d1, i, j, 1)).x;\n" \
"	if (v1 == 0 || v1 & 0x80000000)\n" \
"		return;\n" \
"	bool found = false;\n" \
"	int c;\n" \
"	for (c = 0; c < nid; ++c)\n" \
"	if (ids[c] == v0)\n" \
"	{\n" \
"		found = true;\n" \
"		break;\n" \
"	}\n" \
"	if (!found)\n" \
"		return;\n" \
"	cids[c*3] = cids[c*3]?cids[c*3]:v1;\n" \
"	cids[c*3+1] = cids[c*3]&&!cids[c*3+1]&&cids[c*3]!=v1?v1:cids[c*3+1];\n" \
"	cids[c*3+2] = cids[c*3+1]&&!cids[c*3+2]&&cids[c*3+1]!=v1?v1:cids[c*3+2];\n" \
"}\n" \
"//check xz plane (+-y)\n" \
"__kernel void kernel_1(\n" \
"	__read_only image3d_t l0,\n" \
"	__read_only image3d_t l1,\n" \
"	unsigned int d0,\n" \
"	unsigned int d1,\n" \
"	unsigned int nid,\n" \
"	__global unsigned int* ids,\n" \
"	__global unsigned int* cids)\n" \
"{\n" \
"	unsigned int i = (unsigned int)(get_global_id(0));\n" \
"	unsigned int j = (unsigned int)(get_global_id(1));\n" \
"	unsigned int v0 = read_imageui(l0, samp, (int4)(i, d0, j, 1)).x;\n" \
"	if (v0 == 0 || v0 & 0x80000000)\n" \
"		return;\n" \
"	unsigned int v1 = read_imageui(l1, samp, (int4)(i, d1, j, 1)).x;\n" \
"	if (v1 == 0 || v1 & 0x80000000)\n" \
"		return;\n" \
"	bool found = false;\n" \
"	int c;\n" \
"	for (c = 0; c < nid; ++c)\n" \
"	if (ids[c] == v0)\n" \
"	{\n" \
"		found = true;\n" \
"		break;\n" \
"	}\n" \
"	if (!found)\n" \
"		return;\n" \
"	cids[c*3] = cids[c*3]?cids[c*3]:v1;\n" \
"	cids[c*3+1] = cids[c*3]&&!cids[c*3+1]&&cids[c*3]!=v1?v1:cids[c*3+1];\n" \
"	cids[c*3+2] = cids[c*3+1]&&!cids[c*3+2]&&cids[c*3+1]!=v1?v1:cids[c*3+2];\n" \
"}\n" \
"//check xy plane (+-z)\n" \
"__kernel void kernel_2(\n" \
"	__read_only image3d_t l0,\n" \
"	__read_only image3d_t l1,\n" \
"	unsigned int d0,\n" \
"	unsigned int d1,\n" \
"	unsigned int nid,\n" \
"	__global unsigned int* ids,\n" \
"	__global unsigned int* cids)\n" \
"{\n" \
"	unsigned int i = (unsigned int)(get_global_id(0));\n" \
"	unsigned int j = (unsigned int)(get_global_id(1));\n" \
"	unsigned int v0 = read_imageui(l0, samp, (int4)(i, j, d0, 1)).x;\n" \
"	if (v0 == 0 || v0 & 0x80000000)\n" \
"		return;\n" \
"	unsigned int v1 = read_imageui(l1, samp, (int4)(i, j, d1, 1)).x;\n" \
"	if (v1 == 0 || v1 & 0x80000000)\n" \
"		return;\n" \
"	bool found = false;\n" \
"	int c;\n" \
"	for (c = 0; c < nid; ++c)\n" \
"	if (ids[c] == v0)\n" \
"	{\n" \
"		found = true;\n" \
"		break;\n" \
"	}\n" \
"	if (!found)\n" \
"		return;\n" \
"	cids[c*3] = cids[c*3]?cids[c*3]:v1;\n" \
"	cids[c*3+1] = cids[c*3]&&!cids[c*3+1]&&cids[c*3]!=v1?v1:cids[c*3+1];\n" \
"	cids[c*3+2] = cids[c*3+1]&&!cids[c*3+2]&&cids[c*3+1]!=v1?v1:cids[c*3+2];\n" \
"}\n" \
;

SegGrow::SegGrow(VolumeData* vd):
	m_vd(vd),
	m_branches(10),
	m_iter(0),
	m_sz_thresh(10)
{
}

SegGrow::~SegGrow()
{

}

void SegGrow::SetRulerHandler(RulerHandler* handler)
{
	m_handler = handler;
}

bool SegGrow::CheckBricks()
{
	if (!m_vd || !m_vd->GetTexture())
		return false;
	vector<flvr::TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks();
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

	//create program and kernels
	flvr::KernelProgram* kernel_prog = flvr::VolumeRenderer::
		vol_kernel_factory_.kernel(str_cl_segrow);
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
	vector<flvr::TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks();
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
		flvr::Argument arg_label =
			kernel_prog->setKernelArgTex3DBuf(CL_MEM_READ_WRITE, lid, sizeof(unsigned int)*nx*ny*nz, region);
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
		kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(unsigned int)*gsize.gsxyz, (void*)(pcount));
		kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(unsigned int)*m_branches*gsize.gsxyz, (void*)(pids));
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
		kernel_prog->readBuffer(sizeof(unsigned int)*gsize.gsxyz, pcount, pcount);
		kernel_prog->readBuffer(sizeof(unsigned int)*m_branches*gsize.gsxyz, pids, pids);

		//get count and ids
		std::set<unsigned int> uniqids;
		for (int i = 0; i < gsize.gsxyz; ++i)
		{
			if (count[i])
				for (int j = 0; j < count[i]; ++j)
					uniqids.insert(ids[i*m_branches+j]);
		}
		unsigned int total = uniqids.size();
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
			kernel_prog->setKernelArgBuf(CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(unsigned int)*total, (void*)(pids));
			kernel_prog->setKernelArgBuf(CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(unsigned int)*total*gsize.gsxyz*6, (void*)(pdids));
			kernel_prog->setKernelArgLocal(sizeof(unsigned int)*total*6);

			//execute
			kernel_prog->executeKernel(kernel_4, 3, global_size2, local_size);

			//read back
			kernel_prog->readBuffer(sizeof(unsigned int)*total*gsize.gsxyz*6, pdids, pdids);

			//merge ids
			std::vector<std::set<unsigned int>> id_set;
			for (int i = 0; i < total; ++i)
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

				for (int j = 0; j < gsize.gsxyz; ++j)
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
				unsigned int nmid = merge_id.size() / 2;
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
				kernel_prog->setKernelArgBuf(CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(unsigned int)*nmid*2, (void*)(merge_id.data()));

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
			kernel_prog->setKernelArgBuf(CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(unsigned int)*total, (void*)(pids));
			kernel_prog->setKernelArgBuf(CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(unsigned int)*total*gsize.gsxyz*3, (void*)(pcids));
			kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(unsigned int)*total*gsize.gsxyz, (void*)(psum));
			kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(float)*total*gsize.gsxyz * 3, (void*)(pcsum));
			kernel_prog->setKernelArgLocal(sizeof(unsigned int)*total*3);
			kernel_prog->setKernelArgLocal(sizeof(unsigned int)*total);
			kernel_prog->setKernelArgLocal(sizeof(float)*total * 3);

			//execute
			kernel_prog->executeKernel(kernel_6, 3, global_size2, local_size);

			//read back
			kernel_prog->readBuffer(sizeof(unsigned int)*total*gsize.gsxyz*3, pcids, pcids);
			kernel_prog->readBuffer(sizeof(unsigned int)*total*gsize.gsxyz, psum, psum);
			kernel_prog->readBuffer(sizeof(float)*total*gsize.gsxyz * 3, pcsum, pcsum);

			int ox, oy, oz, nc;
			ox = b->ox(); oy = b->oy(); oz = b->oz();
			//compute centers and connection
			unsigned int cid0, cid1, cid2;
			for (int j = 0; j < total; ++j)
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
				for (int i = 0; i < gsize.gsxyz; ++i)
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
		kernel_prog->copyBufTex3D(arg_label, lid,
			sizeof(unsigned int)*nx*ny*nz, region);

		//release buffer
		kernel_prog->releaseAll();

		bnum++;//brick number of processed
	}

	//connect bricks
	unsigned int idnum = m_list.size();
	std::vector<std::set<unsigned int>> merge_list;//ids in different bricks to be merged
	std::vector<std::set<unsigned int>> brick_pairs;//pairs processed don't need to process again
	while (bnum > 1 && idnum > 1)
	{
		flvr::Texture* tex = m_vd->GetTexture();
		if (!tex)
			break;
		kernel_prog = flvr::VolumeRenderer::
			vol_kernel_factory_.kernel(str_cl_sg_check_borders);
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
			flvr::Argument arg_tex =
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
		kernel_prog->releaseAll();
		break;
	}

	//finalize bricks
	kernel_prog = flvr::VolumeRenderer::
		vol_kernel_factory_.kernel(str_cl_segrow);
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
		flvr::Argument arg_label =
			kernel_prog->setKernelArgTex3DBuf(CL_MEM_READ_WRITE, lid, sizeof(unsigned int)*nx*ny*nz, region);
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nz));

		//execute
		kernel_prog->executeKernel(kernel_7, 3, global_size, local_size);

		//read back
		kernel_prog->copyBufTex3D(arg_label, lid,
			sizeof(unsigned int)*nx*ny*nz, region);

		//release buffer
		kernel_prog->releaseAll();
	}

	MergeIds(merge_list);

	//add ruler points
	double spcx, spcy, spcz;
	m_vd->GetSpacings(spcx, spcy, spcz);
	for (auto it = m_list.begin();
		it != m_list.end(); ++it)
	{
		if (it->second.sum < m_sz_thresh)
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
	flvr::KernelProgram *kernel_prog, int kernel, flvr::Argument &arg_tex,
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
	unsigned int idnum = ids.size();
	std::vector<unsigned int> cids(idnum * 3, 0);
	unsigned int* pcids = cids.data();
	kernel_prog->setKernelArgBegin(kernel);
	kernel_prog->setKernelArgument(arg_tex);
	kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, nlid);
	kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&d0));
	kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&d1));
	kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&idnum));
	kernel_prog->setKernelArgBuf(CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(unsigned int)*idnum, (void*)(ids.data()));
	kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(unsigned int)*idnum * 3, (void*)(pcids));

	//execute
	global_size[0] = n0; global_size[1] = n1;
	kernel_prog->executeKernel(kernel, 2, global_size, local_size);
	//read back
	kernel_prog->readBuffer(sizeof(unsigned int)*idnum * 3, pcids, pcids);

	CollectIds(ids, cids, merge_list);
}