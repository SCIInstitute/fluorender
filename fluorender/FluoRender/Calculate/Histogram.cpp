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
#include "Histogram.h"
#include <DataManager.h>
#include <FLIVR/VolumeRenderer.h>
#include <FLIVR/KernelProgram.h>
#include <FLIVR/VolKernel.h>
#include <FLIVR/TextureBrick.h>
#include <FLIVR/Texture.h>
#include <Database/EntryHist.h>

using namespace flrd;

const char* str_cl_histogram =\
"#define DWL unsigned char\n" \
"#define VSCL 255\n" \
"const sampler_t samp =\n" \
"	CLK_NORMALIZED_COORDS_FALSE|\n" \
"	CLK_ADDRESS_CLAMP_TO_EDGE|\n" \
"	CLK_FILTER_NEAREST;\n" \
"\n" \
"//histogram\n" \
"__kernel void kernel_0(\n" \
"	__read_only image3d_t data,\n" \
"	unsigned int dnxy, \n" \
"	unsigned int dnx,\n" \
"	unsigned int minv,\n" \
"	unsigned int maxv,\n" \
"	unsigned int bin,\n" \
"	__global unsigned int* hist)\n" \
"{\n" \
"	int4 coord = (int4)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2), 1);\n" \
"	unsigned int index = dnxy* coord.z + dnx*coord.y + coord.x;\n" \
"	unsigned int val = bkg[index];\n" \
"	if (val < minv || val > maxv)\n" \
"		return;\n" \
"	index = (val - minv) * (bin - 1) / (maxv - minv);\n" \
"	atomic_inc(hist+index);\n" \
"}\n" \
"//histogram in mask\n" \
"__kernel void kernel_1(\n" \
"	__read_only image3d_t data,\n" \
"	unsigned int dnxy, \n" \
"	unsigned int dnx,\n" \
"	unsigned int minv,\n" \
"	unsigned int maxv,\n" \
"	unsigned int bin,\n" \
"	__global unsigned int* hist,\n" \
"	__read_only image3d_t mask)\n" \
"{\n" \
"	int4 coord = (int4)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2), 1);\n" \
"	unsigned int index = dnxy* coord.z + dnx*coord.y + coord.x;\n" \
"	unsigned int val = bkg[index];\n" \
"	if (val < minv || val > maxv)\n" \
"		return;\n" \
"	index = (val - minv) * (bin - 1) / (maxv - minv);\n" \
"	atomic_inc(hist+index);\n" \
"}\n" \
;

Histogram::Histogram(VolumeData* vd) :
	m_vd(vd),
	m_use_mask(true),
	m_bins(256)
{

}

Histogram::~Histogram()
{

}

bool Histogram::CheckBricks()
{
	if (!m_vd)
		return false;
	if (!m_vd->GetTexture())
		return false;
	int brick_num = m_vd->GetTexture()->get_brick_num();
	if (!brick_num)
		return false;
	return true;
}

bool Histogram::GetInfo(
	flvr::TextureBrick* b,
	long &bits, long &nx, long &ny, long &nz)
{
	bits = b->nb(0) * 8;
	nx = b->nx();
	ny = b->ny();
	nz = b->nz();
	return true;
}

EntryHist* Histogram::GetEntryHist()
{
	EntryHist* hist = 0;

	if (!CheckBricks())
		return hist;

	return hist;
}