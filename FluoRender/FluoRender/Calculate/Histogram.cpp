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
#include <Histogram.h>
#include <Global.h>
#include <MainFrame.h>
#include <VolumeRenderer.h>
#include <KernelProgram.h>
#include <TextureBrick.h>
#include <Texture.h>
#include <KernelFactory.h>
#include <EntryHist.h>
#include <VolumeData.h>

using namespace flrd;

constexpr const char* str_cl_histogram = R"CLKER(
const sampler_t samp =
	CLK_NORMALIZED_COORDS_FALSE|
	CLK_ADDRESS_CLAMP_TO_EDGE|
	CLK_FILTER_NEAREST;

//histogram
__kernel void kernel_0(
	__read_only image3d_t data,
	float minv,
	float maxv,
	unsigned int bin,
	__global unsigned int* hist)
{
	int4 coord = (int4)(get_global_id(0),
		get_global_id(1), get_global_id(2), 1);
	float val = read_imagef(data, samp, coord).x;
	if (val < minv || val > maxv)
		return;
	unsigned int index = (val - minv) * (bin - 1) / (maxv - minv);
	atomic_inc(hist+index);
	atomic_inc(hist+bin);
}
//histogram in mask
__kernel void kernel_1(
	__read_only image3d_t data,
	float minv,
	float maxv,
	unsigned int bin,
	__global unsigned int* hist,
	__read_only image3d_t mask)
{
	int4 coord = (int4)(get_global_id(0),
		get_global_id(1), get_global_id(2), 1);
	float val = read_imagef(mask, samp, coord).x;
	if (val == 0.0f)
		return;
	val = read_imagef(data, samp, coord).x;
	if (val < minv || val > maxv)
		return;
	unsigned int index = (val - minv) * (bin - 1) / (maxv - minv);
	atomic_inc(hist+index);
	atomic_inc(hist+bin);
}
)CLKER";

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

void Histogram::Compute()
{
	if (!CheckBricks())
		return;
	long bits = m_vd->GetBits();
	float minv = 0;
	float maxv = 1;
	if (bits > 8) maxv = float(1.0 / m_vd->GetScalarScale());
	float max_int = m_vd->GetMaxValue();

	//create program and kernels
	flvr::KernelProgram* kernel_prog = glbin_kernel_factory.program(str_cl_histogram, bits, max_int);
	if (!kernel_prog)
		return;
	int kernel_index;
	if (!m_use_mask)
		kernel_index = kernel_prog->createKernel("kernel_0");
	else
		kernel_index = kernel_prog->createKernel("kernel_1");

	size_t brick_num = m_vd->GetTexture()->get_brick_num();
	size_t count = 0;
	std::vector<flvr::TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks();

	//sum histogram
	m_histogram.resize(m_bins + 1, 0);
	std::weak_ptr<flvr::Argument> arg_sh;

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
				CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, "arg_sh",
				sizeof(unsigned int)*(bin + 1), (void*)(m_histogram.data()));
		else
			kernel_prog->setKernelArgument(arg_sh);
		if (m_use_mask)
			kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, mid);

		//execute
		kernel_prog->executeKernel(kernel_index, 3, global_size, local_size);
		//read back
		kernel_prog->readBuffer(arg_sh, (void*)(m_histogram.data()));

		kernel_prog->releaseAllArgs();

		SetProgress(100 * count / brick_num,
			"Computing histogram.");
		count++;
	}

	SetProgress(0, "");

	glbin_kernel_factory.clear(kernel_prog);
}

EntryHist* Histogram::GetEntryHist()
{
	EntryHist* hist = 0;
	Compute();
	if (m_histogram.size() != m_bins + 1)
		return hist;
	if (m_histogram[m_bins])
	{
		hist = new flrd::EntryHist();
		hist->setRange(0, 1);
		hist->setPopulation(m_histogram[m_bins]);
		hist->setData(m_histogram.data());
	}

	return hist;
}