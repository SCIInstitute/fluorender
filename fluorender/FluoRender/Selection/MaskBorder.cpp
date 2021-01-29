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
#include "DataManager.h"
#include "MaskBorder.h"

using namespace fls;

const char* str_cl_check_box_borders = \
"const sampler_t samp =\n" \
"	CLK_NORMALIZED_COORDS_FALSE|\n" \
"	CLK_ADDRESS_CLAMP_TO_EDGE|\n" \
"	CLK_FILTER_NEAREST;\n" \
"\n" \
"//check yz plane\n" \
"__kernel void kernel_0(\n" \
"	__read_only image3d_t mask,\n" \
"	__global unsigned int* hits,\n" \
"	unsigned int x)\n" \
"{\n" \
"	unsigned int i = (unsigned int)(get_global_id(0));\n" \
"	unsigned int j = (unsigned int)(get_global_id(1));\n" \
"	float v = read_imagef(mask, samp, (int4)(x, i, j, 1)).x;\n" \
"	if (v > 0)\n" \
"		atomic_inc(hits);\n" \
"}\n" \
"//check xz plane\n" \
"__kernel void kernel_1(\n" \
"	__read_only image3d_t mask,\n" \
"	__global unsigned int* hits,\n" \
"	unsigned int y)\n" \
"{\n" \
"	unsigned int i = (unsigned int)(get_global_id(0));\n" \
"	unsigned int j = (unsigned int)(get_global_id(1));\n" \
"	float v = read_imagef(mask, samp, (int4)(i, y, j, 1)).x;\n" \
"	if (v > 0)\n" \
"		atomic_inc(hits);\n" \
"}\n" \
"//check xy plane\n" \
"__kernel void kernel_2(\n" \
"	__read_only image3d_t mask,\n" \
"	__global unsigned int* hits,\n" \
"	unsigned int z)\n" \
"{\n" \
"	unsigned int i = (unsigned int)(get_global_id(0));\n" \
"	unsigned int j = (unsigned int)(get_global_id(1));\n" \
"	float v = read_imagef(mask, samp, (int4)(i, j, z, 1)).x;\n" \
"	if (v > 0)\n" \
"		atomic_inc(hits);\n" \
"}\n" \
;

MaskBorder::MaskBorder(VolumeData* vd)
	: m_vd(vd)
{
}

MaskBorder::~MaskBorder()
{
}

bool MaskBorder::CheckBricks()
{
	if (!m_vd || !m_vd->GetTexture())
		return false;
	vector<flvr::TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks();
	if (!bricks || bricks->size() == 0)
		return false;
	return true;
}

void MaskBorder::Compute(int order)
{
	if (!order)
		return;
	if (!CheckBricks())
		return;
	flvr::Texture* tex = m_vd->GetTexture();
	if (!tex)
		return;

	unsigned int idx, bn;
	std::vector<flvr::TextureBrick*> *all_bricks = m_vd->GetTexture()->get_bricks();
	bn = all_bricks->size();
	if (bn < 2)
		return;
	//get bricks with paint mask flag
	std::vector<flvr::TextureBrick*> bricks;
	for (int i = 0; i < bn; ++i)
	{
		if ((*all_bricks)[i]->is_mask_act())
			bricks.push_back((*all_bricks)[i]);
	}
	bn = bricks.size();

	//create program and kernels
	flvr::KernelProgram* kernel_prog = flvr::VolumeRenderer::
		vol_kernel_factory_.kernel(str_cl_check_box_borders);
	if (!kernel_prog)
		return;
	int kernel_index0 = kernel_prog->createKernel("kernel_0");
	int kernel_index1 = kernel_prog->createKernel("kernel_1");
	int kernel_index2 = kernel_prog->createKernel("kernel_2");

	flvr::TextureBrick* nb;//neighbor brick
	unsigned int nid;//neighbor id
	unsigned int bid;
	for (int i = 0; i < bn; ++i)
	{
		flvr::TextureBrick* b = bricks[i];
		bid = b->get_id();
		int nx = b->nx();
		int ny = b->ny();
		int nz = b->nz();
		GLint mid = m_vd->GetVR()->load_brick_mask(b);

		size_t global_size[2] = { 1, 1 };
		size_t local_size[2] = { 1, 1 };

		//yz plane
		flvr::Argument arg_tex =
			kernel_prog->setKernelArgTex3D(kernel_index0, 0,
			CL_MEM_READ_ONLY, mid);
		unsigned int hits_x = 0;
		kernel_prog->setKernelArgBuf(kernel_index0, 1,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(unsigned int), &hits_x);
		idx = order == 1 ? nx - 1 : 0;
		kernel_prog->setKernelArgConst(kernel_index0, 2,
			sizeof(unsigned int), (void*)(&idx));
		//execute
		global_size[0] = ny; global_size[1] = nz;
		kernel_prog->executeKernel(kernel_index0, 2, global_size, local_size);
		//read back
		kernel_prog->readBuffer(sizeof(unsigned int), &hits_x, &hits_x);
		if (hits_x)
		{
			nid = order == 2 ? tex->negxid(bid) : tex->posxid(bid);
			nb = tex->get_brick(nid);
			if (nb)
			{
				nb->valid_mask();
				nb->act_mask();
			}
		}

		//xz plane
		arg_tex.kernel_index = kernel_index1;
		arg_tex.index = 0;
		kernel_prog->setKernelArgument(arg_tex);
		unsigned int hits_y = 0;
		kernel_prog->setKernelArgBuf(kernel_index1, 1,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(unsigned int), &hits_y);
		idx = order == 1 ? ny - 1 : 0;
		kernel_prog->setKernelArgConst(kernel_index1, 2,
			sizeof(unsigned int), (void*)(&idx));
		//execute
		global_size[0] = nx; global_size[1] = nz;
		kernel_prog->executeKernel(kernel_index1, 2, global_size, local_size);
		//read back
		kernel_prog->readBuffer(sizeof(unsigned int), &hits_y, &hits_y);
		if (hits_y)
		{
			nid = order == 2 ? tex->negyid(bid) : tex->posyid(bid);
			nb = tex->get_brick(nid);
			if (nb)
			{
				nb->valid_mask();
				nb->act_mask();
			}
		}

		//xy plane
		arg_tex.kernel_index = kernel_index2;
		arg_tex.index = 0;
		kernel_prog->setKernelArgument(arg_tex);
		unsigned int hits_z = 0;
		kernel_prog->setKernelArgBuf(kernel_index2, 1,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(unsigned int), &hits_z);
		idx = order == 1 ? nz - 1 : 0;
		kernel_prog->setKernelArgConst(kernel_index2, 2,
			sizeof(unsigned int), (void*)(&idx));
		//execute
		global_size[0] = nx; global_size[1] = ny;
		kernel_prog->executeKernel(kernel_index2, 2, global_size, local_size);
		//read back
		kernel_prog->readBuffer(sizeof(unsigned int), &hits_z, &hits_z);
		if (hits_z)
		{
			nid = order == 2 ? tex->negzid(bid) : tex->poszid(bid);
			nb = tex->get_brick(nid);
			if (nb)
			{
				nb->valid_mask();
				nb->act_mask();
			}
		}

		//release buffer
		kernel_prog->releaseAll();
	}
}
