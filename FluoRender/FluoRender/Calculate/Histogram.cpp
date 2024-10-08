/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2024 Scientific Computing and Imaging Institute,
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
#include <Histogram.h>
#include <Global.h>
#include <MainFrame.h>
#include <VolumeRenderer.h>
#include <KernelProgram.h>
#include <TextureBrick.h>
#include <Texture.h>
#include <EntryHist.h>

using namespace flrd;

const char* str_cl_histogram =\
"const sampler_t samp =\n" \
"	CLK_NORMALIZED_COORDS_FALSE|\n" \
"	CLK_ADDRESS_CLAMP_TO_EDGE|\n" \
"	CLK_FILTER_NEAREST;\n" \
"\n" \
"//histogram\n" \
"__kernel void kernel_0(\n" \
"	__read_only image3d_t data,\n" \
"	float minv,\n" \
"	float maxv,\n" \
"	unsigned int bin,\n" \
"	__global unsigned int* hist)\n" \
"{\n" \
"	int4 coord = (int4)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2), 1);\n" \
"	float val = read_imagef(data, samp, coord).x;\n" \
"	if (val < minv || val > maxv)\n" \
"		return;\n" \
"	unsigned int index = (val - minv) * (bin - 1) / (maxv - minv);\n" \
"	atomic_inc(hist+index);\n" \
"	atomic_inc(hist+bin);\n" \
"}\n" \
"//histogram in mask\n" \
"__kernel void kernel_1(\n" \
"	__read_only image3d_t data,\n" \
"	float minv,\n" \
"	float maxv,\n" \
"	unsigned int bin,\n" \
"	__global unsigned int* hist,\n" \
"	__read_only image3d_t mask)\n" \
"{\n" \
"	int4 coord = (int4)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2), 1);\n" \
"	float val = read_imagef(mask, samp, coord).x;\n" \
"	if (val == 0.0f)\n" \
"		return;\n" \
"	val = read_imagef(data, samp, coord).x;\n" \
"	if (val < minv || val > maxv)\n" \
"		return;\n" \
"	unsigned int index = (val - minv) * (bin - 1) / (maxv - minv);\n" \
"	atomic_inc(hist+index);\n" \
"	atomic_inc(hist+bin);\n" \
"}\n" \
;

Histogram::Histogram(VolumeData* vd) :
	m_vd(vd),
	m_use_mask(true),
	m_bins(EntryHist::m_bins),
	Progress()
{
}

Histogram::~Histogram()
{

}

bool Histogram::CheckBricks()
{
	if (!m_vd)
		return false;
	if (m_use_mask && !m_vd->GetMask(false))
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
	long bits = m_vd->GetBits();
	float minv = 0;
	float maxv = 1;
	if (bits > 8) maxv = float(1.0 / m_vd->GetScalarScale());

	//create program and kernels
	flvr::KernelProgram* kernel_prog = glbin_vol_kernel_factory.kernel(str_cl_histogram, bits);
	if (!kernel_prog)
		return hist;
	int kernel_index;
	if (!m_use_mask)
		kernel_index = kernel_prog->createKernel("kernel_0");
	else
		kernel_index = kernel_prog->createKernel("kernel_1");

	size_t brick_num = m_vd->GetTexture()->get_brick_num();
	size_t count = 0;
	std::vector<flvr::TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks();

	//sum histogram
	unsigned int* sh = new unsigned int[m_bins + 1]();
	flvr::Argument arg_sh;

	for (size_t i = 0; i < brick_num; ++i)
	{
		flvr::TextureBrick* b = (*bricks)[i];
		long nx, ny, nz;
		if (!GetInfo(b, bits, nx, ny, nz))
			continue;
		//get tex ids
		GLint tid = m_vd->GetVR()->load_brick(b);
		GLint mid = 0;
		if (m_use_mask)
			mid = m_vd->GetVR()->load_brick_mask(b);

		size_t local_size[3] = { 1, 1, 1 };
		size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };

		unsigned int bin = m_bins;

		kernel_prog->setKernelArgBegin(kernel_index);
		kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, tid);
		kernel_prog->setKernelArgConst(sizeof(float), (void*)(&minv));
		kernel_prog->setKernelArgConst(sizeof(float), (void*)(&maxv));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&bin));
		if (i == 0)
			arg_sh = kernel_prog->setKernelArgBuf(
				CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
				sizeof(unsigned int)*(bin + 1), (void*)(sh));
		else
			kernel_prog->setKernelArgument(arg_sh);
		if (m_use_mask)
			kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, mid);

		//execute
		kernel_prog->executeKernel(kernel_index, 3, global_size, local_size);
		//read back
		kernel_prog->readBuffer(sizeof(unsigned int)*(bin+1), sh, sh);

		SetProgress(100 * count / brick_num,
			"Computing histogram.");
		count++;
	}

	if (sh[m_bins])
	{
		hist = new flrd::EntryHist();
		hist->setRange(minv, maxv);
		hist->setPopulation(sh[m_bins]);
		hist->setData(sh);
	}
	
	kernel_prog->releaseAll();
	delete[] sh;
	SetProgress(0, "");

	return hist;
}