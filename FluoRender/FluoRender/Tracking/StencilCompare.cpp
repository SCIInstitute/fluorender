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

#include <StencilCompare.h>
#include <FLIVR/VolumeRenderer.h>

using namespace flrd;

const char* str_cl_compare = \
"const sampler_t samp =\n" \
"CLK_NORMALIZED_COORDS_FALSE |\n" \
"CLK_ADDRESS_CLAMP |\n" \
"CLK_FILTER_NEAREST;\n" \
"\n" \
"//low pass filter"
"__kernel void kernel_0(\n" \
"	__read_only image3d_t img_in,\n" \
"	__write_only image3d_t img_out,\n" \
"	int3 range\n" \
")\n" \
"{\n" \
"	int3 gid = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	int3 lb = gid - range;\n" \
"	int3 ub = gid + range;\n" \
"	int4 ijk = (int4)(0, 0, 0, 1);\n" \
"	float sum = 0.0f;\n" \
"	int count = 0;\n" \
"#pragma unroll\n" \
"	for (ijk.z = lb.z; ijk.z <= ub.z; ++ijk.z)\n" \
"#pragma unroll\n" \
"	for (ijk.y = lb.y; ijk.y <= ub.y; ++ijk.y)\n" \
"#pragma unroll\n" \
"	for (ijk.x = lb.x; ijk.x <= ub.x; ++ijk.x)\n" \
"	{\n" \
"		sum += read_imagef(img_in, samp, ijk).x;\n" \
"		count++;\n" \
"	}\n" \
"	sum = count ? sum / count : sum;\n" \
"	write_imagef(img_out, int4(gid, 1), sum);\n" \
"}\n"
;

StencilCompare::StencilCompare() :
	m_s1(0), m_s2(0)
{

}

StencilCompare::StencilCompare(Stencil* s1, Stencil* s2,
	const fluo::Vector& ext1, const fluo::Vector& ext2,
	const fluo::Vector& off1, const fluo::Vector& off2,
	const int iter, const int method):
m_s1(s1), m_s2(s2),
m_ext1(ext1), m_ext2(ext2),
m_off1(off1), m_off2(off2),
m_iter(iter), m_method(method)
{

}

StencilCompare::~StencilCompare()
{

}

void StencilCompare::Prepare()
{
	//create program and kernels
	m_prog = flvr::VolumeRenderer::
		vol_kernel_factory_.kernel(str_cl_compare);
	if (!m_prog)
		return;
	int kernel_index = -1;
	std::string name = "kernel_0";
	if (m_prog->valid())
	{
		kernel_index = m_prog->findKernel(name);
		if (kernel_index == -1)
			kernel_index = m_prog->createKernel(name);
	}
	else
		kernel_index = m_prog->createKernel(name);

	size_t local_size[3] = { 1, 1, 1 };
	size_t global_size[3] = { m_s1->nx, m_s1->ny, m_s1->nz };

	cl_image_format format;
	format.image_channel_order = CL_R;
	format.image_channel_data_type = m_s1->bits == 8 ? CL_UNORM_INT8 : CL_UNORM_INT16;
	cl_image_desc desc;
	desc.image_type = CL_MEM_OBJECT_IMAGE3D;
	desc.image_width = m_s1->nx;
	desc.image_height = m_s1->ny;
	desc.image_depth = m_s1->nz;
	desc.image_array_size = 0;
	desc.image_row_pitch = 0;
	desc.image_slice_pitch = 0;
	desc.num_mip_levels = 0;
	desc.mem_object = NULL;

	//set up kernel
	m_prog->setKernelArgBegin(kernel_index);
	if (m_s1->fsize < 1)
		m_img1 = m_prog->setKernelArgImage(CL_MEM_READ_ONLY, format, desc, m_s1->data);
	else
	{
		m_prog->setKernelArgImage(CL_MEM_READ_ONLY, format, desc, m_s1->data);
		m_img1 = m_prog->setKernelArgImage(CL_MEM_WRITE_ONLY, format, desc, NULL);
		cl_int3 range = { 1, 1, m_s1->nz > 1 ? 1 : 0 };
		m_prog->setKernelArgConst(sizeof(cl_int3), (void*)(&range));
		//filter s1
		for (int i = 0; i < m_s1->fsize; ++i)
			m_prog->executeKernel(kernel_index, 3, global_size, 0/*local_size*/);
	}

	//set up kernel
	format.image_channel_data_type = m_s2->bits == 8 ? CL_UNORM_INT8 : CL_UNORM_INT16;
	desc.image_width = m_s2->nx;
	desc.image_height = m_s2->ny;
	desc.image_depth = m_s2->nz;
	m_prog->setKernelArgBegin(kernel_index);
	if (m_s2->fsize < 1)
		m_img2 = m_prog->setKernelArgImage(CL_MEM_READ_ONLY, format, desc, m_s2->data);
	else
	{
		m_prog->setKernelArgImage(CL_MEM_READ_ONLY, format, desc, m_s2->data);
		m_img2 = m_prog->setKernelArgImage(CL_MEM_WRITE_ONLY, format, desc, NULL);
		cl_int3 range = { 1, 1, m_s2->nz > 1 ? 1 : 0 };
		m_prog->setKernelArgConst(sizeof(cl_int3), (void*)(&range));
		//filter s2
		for (int i = 0; i < m_s2->fsize; ++i)
			m_prog->executeKernel(kernel_index, 3, global_size, 0/*local_size*/);
	}
}

void StencilCompare::Clean()
{
	m_prog->releaseAll();
}

bool StencilCompare::Compare()
{
	Prepare();

	//set up initial neighborhoods
	fluo::Vector s1cp(m_s1->box.center());
	fluo::Vector range = m_s1->box.diagonal();
	range = fluo::Min(range, m_ext1);
	fluo::Neighbor nb_trans(fluo::Point(), range);
	range = fluo::Min(fluo::Vector(range.z(), range.z(), 180), m_ext2);
	fluo::Neighbor nb_rot(fluo::Point(), range);
	//constant neighborhoods
	fluo::Neighbor nb_null(fluo::Point(), fluo::Vector(0));
	fluo::Neighbor nbt_1(fluo::Point(), fluo::Min(fluo::Vector(1), nb_trans.n()));
	fluo::Neighbor nbr_1(fluo::Point(), fluo::Min(fluo::Vector(1), nb_rot.n()));

	//main loop
	float p, maxp;
	maxp = 0;
	fluo::Point center(m_off1), euler(m_off2);//for outer loop
	fluo::Point c, e;//for inner loops
	int counter = 0;
	bool rot = false;//loop mode
	int trans_conv = 0; int rot_conv = 0;//convergence conditions

	fluo::Neighbor nbt;
	fluo::Neighbor nbr;
	//set up neighborhood
	if (rot)
	{
		nbt = nb_null;
		nbr = nb_rot;
	}
	else
	{
		nbt = nb_trans;
		nbr = nb_null;
	}
	while (true)
	{
		bool foundp = false;

		//trans loop
		for (fluo::Point i = nbt.begin(); i != nbt.end(); i = ++nbt)
		{
			//rot loop
			for (fluo::Point j = nbr.begin(); j != nbr.end(); j = ++nbr)
			{
				//compute transform
				m_s2->load_identity();
				m_s2->rotate(euler + j, s1cp + center + i);
				m_s2->translate(center + i);

				//compare images
				switch (m_method)
				{
				case 0:
					p = Compare0();
					break;
				case 1:
					p = Compare1();
					break;
				}
				p = Similar();
				if (p > maxp)
				{
					maxp = p;
					c = i;
					e = j;
					foundp = true;
				}
			}
		}

		if (foundp)
		{
			if (rot)
			{
				euler += e;
				//update neighborhood
				nbr.c(e);
			}
			else
			{
				center += c;
				//update neighborhood
				nbt.c(c);
			}
		}
		else
		{
			//convergence conditions
			if (rot)
			{
				//rot converged
				rot_conv++;
				if (trans_conv > 1)
					break;
				rot = false;
				//update neighborhood
				nbr = nb_null;
				if (trans_conv > 0)
					nbt = nbt_1;
				else
					nbt = nb_trans;
			}
			else
			{
				//trans converged
				trans_conv++;
				if (rot_conv > 1)
					break;
				rot = true;
				//update neighborhood
				if (rot_conv > 0)
					nbr = nbr_1;
				else
					nbr = nb_rot;
				nbt = nb_null;
			}
		}

		counter++;
		if (counter > m_iter)
			break;
	}

	m_s2->load_identity();
	m_s2->rotate(fluo::Vector(euler), fluo::Vector(center) + s1cp);
	m_s2->translate(fluo::Vector(center));
	m_s2->id = m_s1->id;

	return true;
}

float StencilCompare::Compare0()
{
	float result = 0.0f;

	return result;
}

float StencilCompare::Compare1()
{
	float result = 0.0f;
	return result;
}