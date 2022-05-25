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
#ifdef _DEBUG
#include <Debug.h>
#endif

using namespace flrd;

const char* str_cl_stencil = \
"//2d filter 8 bit\n" \
"__kernel void kernel_0(\n" \
"	__global unsigned char* img_in,\n" \
"	__global unsigned char* img_out,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz)\n" \
"{\n" \
"	int3 gid = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	unsigned int index;\n" \
"	index = nx*ny*gid.z + nx*gid.y + gid.x;\n" \
"	img_in[index] = (unsigned char)(255);\n" \
"	img_out[index] = (unsigned char)(255);\n" \
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
		vol_kernel_factory_.kernel(str_cl_stencil);
	if (!m_prog)
		return;
	int kernel_index = -1;
	std::string name = "kernel_0";
	if (m_s1->bits > 8) name = "kernel_1";
	if (m_prog->valid())
	{
		kernel_index = m_prog->findKernel(name);
		if (kernel_index == -1)
			kernel_index = m_prog->createKernel(name);
	}
	else
		kernel_index = m_prog->createKernel(name);

	flvr::Argument img[2];
	size_t local_size[3] = { 1, 1, 1 };
	size_t global_size[3] = { size_t(m_s1->nx), size_t(m_s1->ny), size_t(m_s1->nz) };
	size_t buf_size = m_s1->bits == 8 ?
		sizeof(unsigned char) : sizeof(unsigned short);
	buf_size *= m_s1->nx * m_s1->ny * m_s1->nz;
	DBMIUINT8 mi(m_s1->nx, m_s1->ny, 1);

	//set up kernel
	m_prog->setKernelArgBegin(kernel_index);
	if (m_s1->fsize < 1)
		m_img1 = m_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, buf_size, (void*)(m_s1->data));
	else
	{
		img[0] = m_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, buf_size, (void*)(m_s1->data));
		img[1] = m_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY, buf_size, NULL);
		m_prog->setKernelArgConst(sizeof(size_t), (void*)(&(m_s1->nx)));
		m_prog->setKernelArgConst(sizeof(size_t), (void*)(&(m_s1->ny)));
		m_prog->setKernelArgConst(sizeof(size_t), (void*)(&(m_s1->nz)));
		//filter s1
		for (int i = 0; i < m_s1->fsize; ++i)
		{
			if (i)
			{
				//swap images
				m_prog->setKernelArgBegin(kernel_index);
				m_prog->setKernelArgument(img[i%2]);
				m_prog->setKernelArgument(img[(i+1)%2]);
			}
			m_prog->executeKernel(kernel_index, 3, global_size, local_size);
			m_prog->finish();
			m_prog->readBuffer(img[0], (void*)(mi.data));
			m_prog->readBuffer(img[1], (void*)(mi.data));
		}
		m_img1 = img[m_s1->fsize % 2];
	}

	//set up kernel
	m_prog->setKernelArgBegin(kernel_index);
	if (m_s2->fsize < 1)
		m_img2 = m_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, buf_size, (void*)(m_s2->data));
	else
	{
		img[0] = m_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, buf_size, (void*)(m_s2->data));
		img[1] = m_prog->setKernelArgBuf(CL_MEM_READ_WRITE, buf_size, NULL);
		m_prog->setKernelArgConst(sizeof(size_t), (void*)(&(m_s2->nx)));
		m_prog->setKernelArgConst(sizeof(size_t), (void*)(&(m_s2->ny)));
		m_prog->setKernelArgConst(sizeof(size_t), (void*)(&(m_s2->nz)));
		//filter s2
		for (int i = 0; i < m_s2->fsize; ++i)
		{
			if (i)
			{
				//swap images
				m_prog->setKernelArgBegin(kernel_index);
				m_prog->setKernelArgument(img[i % 2]);
				m_prog->setKernelArgument(img[(i + 1) % 2]);
			}
			m_prog->executeKernel(kernel_index, 3, global_size, 0/*local_size*/);
		}
		m_img2 = img[m_s2->fsize % 2];
	}
}

void StencilCompare::Clean()
{
	m_prog->releaseAll();
}

bool StencilCompare::Compare()
{
	Prepare();

	std::string name;
	switch (m_method)
	{
	case 0:
		name = "kernel_1";
		break;
	case 1:
		name = "kernel_2";
		break;
	}

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
				p = Compare(name);
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

	Clean();

	return true;
}

float StencilCompare::Compare(const std::string& name)
{
	float result = 0.0f;

	if (!m_prog)
		return result;
	int kernel_index = -1;
	if (m_prog->valid())
	{
		kernel_index = m_prog->findKernel(name);
		if (kernel_index == -1)
			kernel_index = m_prog->createKernel(name);
	}
	else
		kernel_index = m_prog->createKernel(name);

	size_t local_size[3] = { 1, 1, 1 };
	fluo::Vector diag = m_s1->box.diagonal();
	size_t global_size[3] = { size_t(diag.intx()), size_t(diag.inty()), size_t(diag.intz()) };

	//set up kernel
	m_prog->setKernelArgBegin(kernel_index);
	m_prog->setKernelArgument(m_img1);
	m_prog->setKernelArgument(m_img2);
	cl_int3 bmin{ m_s1->box.Min().intx(), m_s1->box.Min().inty(), m_s1->box.Min().intz() };
	m_prog->setKernelArgConst(sizeof(cl_int3), (void*)(&bmin));
	cl_float4 tf0 = { float(m_s2->tf.get_mat_val(0, 0)),
		float(m_s2->tf.get_mat_val(1, 0)),
		float(m_s2->tf.get_mat_val(2, 0)),
		float(m_s2->tf.get_mat_val(3, 0)) };
	cl_float4 tf1 = { float(m_s2->tf.get_mat_val(0, 1)),
		float(m_s2->tf.get_mat_val(1, 1)),
		float(m_s2->tf.get_mat_val(2, 1)),
		float(m_s2->tf.get_mat_val(3, 1)) };
	cl_float4 tf2 = { float(m_s2->tf.get_mat_val(0, 2)),
		float(m_s2->tf.get_mat_val(1, 2)),
		float(m_s2->tf.get_mat_val(2, 2)),
		float(m_s2->tf.get_mat_val(3, 2)) };
	cl_float4 tf3 = { float(m_s2->tf.get_mat_val(0, 3)),
		float(m_s2->tf.get_mat_val(1, 3)),
		float(m_s2->tf.get_mat_val(2, 3)),
		float(m_s2->tf.get_mat_val(3, 3)) };
	m_prog->setKernelArgConst(sizeof(cl_float4), (void*)(&tf0));
	m_prog->setKernelArgConst(sizeof(cl_float4), (void*)(&tf1));
	m_prog->setKernelArgConst(sizeof(cl_float4), (void*)(&tf2));
	m_prog->setKernelArgConst(sizeof(cl_float4), (void*)(&tf3));
	m_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(float), (void*)(&result));
	
	//execute
	m_prog->executeKernel(kernel_index, 3, global_size, 0/*local_size*/);
	//read back
	m_prog->readBuffer(sizeof(float), &result, &result);

	return result;
}
