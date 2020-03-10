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
#include <FLIVR/VolKernel.h>
#include <algorithm>

using namespace FL;

const char* str_cl_segrow = \
"const sampler_t samp =\n" \
"	CLK_NORMALIZED_COORDS_FALSE|\n" \
"	CLK_ADDRESS_CLAMP_TO_EDGE|\n" \
"	CLK_FILTER_NEAREST;\n" \
"\n" \
"unsigned int __attribute((always_inline)) reverse_bit(unsigned int val, unsigned int len)\n" \
"{\n" \
"	unsigned int res = val;\n" \
"	int s = len - 1;\n" \
"	for (val >>= 1; val; val >>= 1)\n" \
"	{\n" \
"		res <<= 1;\n" \
"		res |= val & 1;\n" \
"		s--;\n" \
"	}\n" \
"	res <<= s;\n" \
"	res <<= 32-len;\n" \
"	res >>= 32-len;\n" \
"	return res;\n" \
"}\n" \
"\n" \
"//initialize masked regions to ids (shuffle)\n" \
"__kernel void kernel_1(\n" \
"	__read_only image3d_t mask,\n" \
"	__global unsigned int* label,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	unsigned int lenx,\n" \
"	unsigned int lenz)\n" \
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
"	unsigned int x, y, z, ii;\n" \
"	x = reverse_bit(i, lenx);\n" \
"	y = reverse_bit(j, lenx);\n" \
"	z = reverse_bit(k, lenz);\n" \
"	unsigned int res = 0;\n" \
"	for (ii=0; ii<lenx; ++ii)\n" \
"	{\n" \
"		res |= (1<<ii & x)<<(ii);\n" \
"		res |= (1<<ii & y)<<(ii+1);\n" \
"	}\n" \
"	res |= z<<lenx*2;\n" \
"	atomic_xchg(label+index, res + 1);\n" \
"}\n" \
"//initialize new mask regions to ids (ordered)\n" \
"__kernel void kernel_null(\n" \
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
"	atomic_xchg(label+index, nx*ny*nz - index);\n" \
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
"	unsigned int maxc,\n" \
"	__global unsigned int* count,\n" \
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
"		atomic_xchg(ids+(*count)-1, label_v);\n" \
"	}\n" \
"}\n" \
"//find connectivity/center of new ids\n" \
"__kernel void kernel_4(\n" \
"	__global unsigned int* label,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	unsigned int count,\n" \
"	__global unsigned int* ids,\n" \
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
"	atomic_inc(sum+c);\n" \
"	atomic_xchg(csum+c*3, csum[c*3] + (float)(coord.x));\n" \
"	atomic_xchg(csum+c*3+1, csum[c*3+1] + (float)(coord.y));\n" \
"	atomic_xchg(csum+c*3+2, csum[c*3+2] + (float)(coord.z));\n" \
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
"__kernel void kernel_5(\n" \
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
	m_vd(vd),
	m_branches(10),
	m_iter(0)
{
}

SegGrow::~SegGrow()
{

}

bool SegGrow::CheckBricks()
{
	if (!m_vd || !m_vd->GetTexture())
		return false;
	vector<TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks();
	if (!bricks || bricks->size() == 0)
		return false;
	return true;
}

void SegGrow::Compute()
{
	if (!CheckBricks())
		return;

	//create program and kernels
	FLIVR::KernelProgram* kernel_prog = FLIVR::VolumeRenderer::
		vol_kernel_factory_.kernel(str_cl_segrow);
	if (!kernel_prog)
		return;
	int kernel_1 = kernel_prog->createKernel("kernel_1");//init
	int kernel_2 = kernel_prog->createKernel("kernel_2");//grow
	int kernel_3 = kernel_prog->createKernel("kernel_3");//count
	int kernel_4 = kernel_prog->createKernel("kernel_4");//get shape
	int kernel_5 = kernel_prog->createKernel("kernel_5");//finalize

	std::vector<unsigned int> ids(m_branches);
	unsigned int* pids = ids.data();
	std::vector<unsigned int> cids(m_branches);
	unsigned int* pcids = cids.data();

	size_t brick_num = m_vd->GetTexture()->get_brick_num();
	vector<FLIVR::TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks();
	for (size_t bi = 0; bi < brick_num; ++bi)
	{
		TextureBrick* b = (*bricks)[bi];
		if (!b->get_paint_mask())
			continue;
		int nx = b->nx();
		int ny = b->ny();
		int nz = b->nz();
		GLint mid = m_vd->GetVR()->load_brick_mask(b);
		GLint lid = m_vd->GetVR()->load_brick_label(b);

		size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };
		size_t local_size[3] = { 1, 1, 1 };
		//bit length
		unsigned int lenx = 0;
		unsigned int r = Max(nx, ny);
		while (r > 0)
		{
			r /= 2;
			lenx++;
		}
		unsigned int lenz = 0;
		r = nz;
		while (r > 0)
		{
			r /= 2;
			lenz++;
		}

		//set
		size_t region[3] = { (size_t)nx, (size_t)ny, (size_t)nz };
		unsigned int count, sum;
		//kernel1
		kernel_prog->setKernelArgTex3D(kernel_1, 0,
			CL_MEM_READ_ONLY, mid);
		Argument arg_label =
			kernel_prog->setKernelArgTex3DBuf(kernel_1, 1,
			CL_MEM_READ_WRITE, lid, sizeof(unsigned int)*nx*ny*nz, region);
		kernel_prog->setKernelArgConst(kernel_1, 2,
			sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(kernel_1, 3,
			sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(kernel_1, 4,
			sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setKernelArgConst(kernel_1, 5,
			sizeof(unsigned int), (void*)(&lenx));
		kernel_prog->setKernelArgConst(kernel_1, 6,
			sizeof(unsigned int), (void*)(&lenz));
		//kernel2
		arg_label.kernel_index = kernel_2;
		arg_label.index = 0;
		kernel_prog->setKernelArgument(arg_label);
		kernel_prog->setKernelArgConst(kernel_2, 1,
			sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(kernel_2, 2,
			sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(kernel_2, 3,
			sizeof(unsigned int), (void*)(&nz));
		//kernel3
		arg_label.kernel_index = kernel_3;
		arg_label.index = 0;
		kernel_prog->setKernelArgument(arg_label);
		kernel_prog->setKernelArgConst(kernel_3, 1,
			sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(kernel_3, 2,
			sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(kernel_3, 3,
			sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setKernelArgConst(kernel_3, 4,
			sizeof(unsigned int), (void*)(&m_branches));
		count = 0;
		kernel_prog->setKernelArgBuf(kernel_3, 5,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(unsigned int), (void*)(&count));
		kernel_prog->setKernelArgBuf(kernel_3, 6,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(unsigned int)*m_branches, (void*)(pids));
		
		//first pass
		kernel_prog->executeKernel(kernel_1, 3, global_size, local_size);
		for (int i = 0; i < m_iter; ++i)
			kernel_prog->executeKernel(kernel_2, 3, global_size, local_size);
		kernel_prog->executeKernel(kernel_3, 3, global_size, local_size);

		//read back
		kernel_prog->readBuffer(sizeof(unsigned int), &count, &count);
		kernel_prog->readBuffer(sizeof(unsigned int)*m_branches, pids, pids);

		if (count > m_branches)
			count = m_branches;
		//check
		std::set<unsigned int> uniqids;
		for (int i = 0; i < count; ++i)
		{
			if (ids[i])
				uniqids.insert(ids[i]);
		}
		count = uniqids.size();
		if (!count)
			continue;
		std::fill(ids.begin(), ids.end(), 0);
		int ii = 0;
		for (auto it = uniqids.begin();
			it != uniqids.end(); ++it)
		{
			ids[ii] = *it;
			++ii;
		}

		//set
		//kernel4
		arg_label.kernel_index = kernel_4;
		arg_label.index = 0;
		kernel_prog->setKernelArgument(arg_label);
		kernel_prog->setKernelArgConst(kernel_4, 1,
			sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(kernel_4, 2,
			sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(kernel_4, 3,
			sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setKernelArgConst(kernel_4, 4,
			sizeof(unsigned int), (void*)(&count));
		kernel_prog->setKernelArgBuf(kernel_4, 5,
			CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			sizeof(unsigned int)*count, (void*)(pids));
		kernel_prog->setKernelArgBuf(kernel_4, 6,
			CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
			sizeof(unsigned int)*count, (void*)(pcids));
		sum = 0;
		kernel_prog->setKernelArgBuf(kernel_4, 7,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(unsigned int), (void*)(&sum));
		std::vector<float> csum(count * 3, 0.0f);
		float* pcsum = csum.data();
		kernel_prog->setKernelArgBuf(kernel_4, 8,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(float)*count*3, (void*)(pcsum));

		//execute
		kernel_prog->executeKernel(kernel_4, 3, global_size, local_size);

		//read back
		kernel_prog->readBuffer(sizeof(unsigned int)*count, pcids, pcids);
		kernel_prog->readBuffer(sizeof(unsigned int), &sum, &sum);
		kernel_prog->readBuffer(sizeof(unsigned int)*count*3, pcsum, pcsum);

		//finalize
		//kernel5
		arg_label.kernel_index = kernel_5;
		arg_label.index = 0;
		kernel_prog->setKernelArgument(arg_label);
		kernel_prog->setKernelArgConst(kernel_5, 1,
			sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(kernel_5, 2,
			sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(kernel_5, 3,
			sizeof(unsigned int), (void*)(&nz));

		//execute
		kernel_prog->executeKernel(kernel_5, 3, global_size, local_size);

		//read back
		kernel_prog->copyBufTex3D(arg_label, lid,
			sizeof(unsigned int)*nx*ny*nz, region);

		//release buffer
		kernel_prog->releaseAll();
	}
}
