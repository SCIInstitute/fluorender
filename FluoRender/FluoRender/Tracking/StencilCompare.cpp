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
"	int3 lb = gid - (int3)(1, 1, 0);\n" \
"	int3 ub = gid + (int3)(1, 1, 0);\n" \
"	lb = clamp(lb, (int3)(0), (int3)(nx-1, ny-1, nz-1));\n" \
"	ub = clamp(ub, (int3)(0), (int3)(nx-1, ny-1, nz-1));\n" \
"	int3 ijk;\n" \
"	float sum = 0.0f;\n" \
"	int count = 0;\n" \
"	unsigned int index;\n" \
"#pragma unroll\n" \
"	for (ijk.z = lb.z; ijk.z <= ub.z; ++ijk.z)\n" \
"#pragma unroll\n" \
"	for (ijk.y = lb.y; ijk.y <= ub.y; ++ijk.y)\n" \
"#pragma unroll\n" \
"	for (ijk.x = lb.x; ijk.x <= ub.x; ++ijk.x)\n" \
"	{\n" \
"		index = nx*ny*ijk.z + nx*ijk.y + ijk.x;\n" \
"		sum += img_in[index];\n" \
"		count++;\n" \
"	}\n" \
"	sum = count ? sum / count : sum;\n" \
"	index = nx*ny*gid.z + nx*gid.y + gid.x;\n" \
"	img_out[index] = convert_uchar(sum);\n" \
"}\n"
"//2d filter 16 bit\n" \
"__kernel void kernel_1(\n" \
"	__global unsigned short* img_in,\n" \
"	__global unsigned short* img_out,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz)\n" \
"{\n" \
"	int3 gid = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	int3 lb = gid - (int3)(1, 1, 0);\n" \
"	int3 ub = gid + (int3)(1, 1, 0);\n" \
"	lb = clamp(lb, (int3)(0), (int3)(nx-1, ny-1, nz-1));\n" \
"	ub = clamp(ub, (int3)(0), (int3)(nx-1, ny-1, nz-1));\n" \
"	int3 ijk;\n" \
"	float sum = 0.0f;\n" \
"	int count = 0;\n" \
"	unsigned int index;\n" \
"#pragma unroll\n" \
"	for (ijk.z = lb.z; ijk.z <= ub.z; ++ijk.z)\n" \
"#pragma unroll\n" \
"	for (ijk.y = lb.y; ijk.y <= ub.y; ++ijk.y)\n" \
"#pragma unroll\n" \
"	for (ijk.x = lb.x; ijk.x <= ub.x; ++ijk.x)\n" \
"	{\n" \
"		index = nx*ny*ijk.z + nx*ijk.y + ijk.x;\n" \
"		sum += img_in[index];\n" \
"		count++;\n" \
"	}\n" \
"	sum = count ? sum / count : sum;\n" \
"	index = nx*ny*gid.z + nx*gid.y + gid.x;\n" \
"	img_out[index] = convert_ushort(sum);\n" \
"}\n"
"//compare0 8 bit\n" \
"__kernel void kernel_2(\n" \
"	__global unsigned char* img1,\n" \
"	__global unsigned char* img2,\n" \
"	__global float* sum,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	unsigned int nxy,\n" \
"	unsigned int ngx,\n" \
"	unsigned int ngy,\n" \
"	unsigned int ngz,\n" \
"	unsigned int gsxy,\n" \
"	unsigned int gsx,\n" \
"	int3 bmin,\n" \
"	float4 mat0,\n" \
"	float4 mat1,\n" \
"	float4 mat2,\n" \
"	float4 mat3)\n" \
"{\n" \
"	int3 gid = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);\n" \
"	int3 ub = (int3)(lb.x + ngx, lb.y + ngy, lb.z + ngz);\n" \
"	lb += bmin;\n" \
"	ub += bmin;\n" \
"	int4 ijk = (int4)(0, 0, 0, 1);\n" \
"	unsigned int index;\n" \
"	float lsum = 0.0f;\n" \
"	float v1, v2;\n" \
"	float4 coord;\n" \
"	int4 coordi;\n" \
"#pragma unroll\n" \
"	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)\n" \
"#pragma unroll\n" \
"	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)\n" \
"#pragma unroll\n" \
"	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)\n" \
"	{\n" \
"		if (ijk.x < 0 || ijk.x >= nx || ijk.y < 0 || ijk.y >= ny || ijk.z < 0 || ijk.z >= nz)\n" \
"			v1 = 0.0f;\n" \
"		else\n" \
"		{\n" \
"			index = nxy*ijk.z + nx*ijk.y + ijk.x;\n" \
"			v1 = convert_float(img1[index]);\n" \
"		}\n" \
"		coord = convert_float4(ijk);\n" \
"		coord = (float4)(dot(coord, mat0), dot(coord, mat1), dot(coord, mat2), dot(coord, mat3));\n" \
"		coord /= coord.w;\n" \
"		coordi = convert_int4(coord);\n" \
"		if (coordi.x < 0 || coordi.x >= nx || coordi.y < 0 || coordi.y >= ny || coordi.z < 0 || coordi.z >= nz)\n" \
"			v2 = 0.0f;\n" \
"		else\n" \
"		{\n" \
"			index = nxy*coordi.z + nx*coordi.y + coordi.x;\n" \
"			v2 = convert_float(img2[index]);\n" \
"		}\n" \
"		v1 = v1 * v2 / 65025.0f;\n" \
"		lsum += v1;\n" \
"	}\n" \
"	index = gsxy * gid.z + gsx * gid.y + gid.x;\n" \
"	atomic_xchg(sum+index, lsum);\n" \
"}\n" \
"//compare0 16 bit\n" \
"__kernel void kernel_3(\n" \
"	__global unsigned short* img1,\n" \
"	__global unsigned short* img2,\n" \
"	__global float* sum,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	unsigned int nxy,\n" \
"	unsigned int ngx,\n" \
"	unsigned int ngy,\n" \
"	unsigned int ngz,\n" \
"	unsigned int gsxy,\n" \
"	unsigned int gsx,\n" \
"	int3 bmin,\n" \
"	float4 mat0,\n" \
"	float4 mat1,\n" \
"	float4 mat2,\n" \
"	float4 mat3)\n" \
"{\n" \
"	int3 gid = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);\n" \
"	int3 ub = (int3)(lb.x + ngx, lb.y + ngy, lb.z + ngz);\n" \
"	lb += bmin;\n" \
"	ub += bmin;\n" \
"	int4 ijk = (int4)(0, 0, 0, 1);\n" \
"	unsigned int index;\n" \
"	float lsum = 0.0f;\n" \
"	float v1, v2;\n" \
"	float4 coord;\n" \
"	int4 coordi;\n" \
"#pragma unroll\n" \
"	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)\n" \
"#pragma unroll\n" \
"	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)\n" \
"#pragma unroll\n" \
"	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)\n" \
"	{\n" \
"		if (ijk.x < 0 || ijk.x >= nx || ijk.y < 0 || ijk.y >= ny || ijk.z < 0 || ijk.z >= nz)\n" \
"			v1 = 0.0f;\n" \
"		else\n" \
"		{\n" \
"			index = nxy*ijk.z + nx*ijk.y + ijk.x;\n" \
"			v1 = convert_float(img1[index]) / 65535.0f;\n" \
"		}\n" \
"		coord = convert_float4(ijk);\n" \
"		coord = (float4)(dot(coord, mat0), dot(coord, mat1), dot(coord, mat2), dot(coord, mat3));\n" \
"		coord /= coord.w;\n" \
"		coordi = convert_int4(coord);\n" \
"		if (coordi.x < 0 || coordi.x >= nx || coordi.y < 0 || coordi.y >= ny || coordi.z < 0 || coordi.z >= nz)\n" \
"			v2 = 0.0f;\n" \
"		else\n" \
"		{\n" \
"			index = nxy*coordi.z + nx*coordi.y + coordi.x;\n" \
"			v2 = convert_float(img2[index]) / 65535.0f;\n" \
"		}\n" \
"		v1 = v1 * v2;\n" \
"		lsum += v1;\n" \
"	}\n" \
"	index = gsxy * gid.z + gsx * gid.y + gid.x;\n" \
"	atomic_xchg(sum+index, lsum);\n" \
"}\n" \
"//compare1 8 bit\n" \
"__kernel void kernel_4(\n" \
"	__global unsigned char* img1,\n" \
"	__global unsigned char* img2,\n" \
"	__global float* sum,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	unsigned int nxy,\n" \
"	unsigned int ngx,\n" \
"	unsigned int ngy,\n" \
"	unsigned int ngz,\n" \
"	unsigned int gsxy,\n" \
"	unsigned int gsx,\n" \
"	int3 bmin,\n" \
"	float4 mat0,\n" \
"	float4 mat1,\n" \
"	float4 mat2,\n" \
"	float4 mat3)\n" \
"{\n" \
"	int3 gid = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);\n" \
"	int3 ub = (int3)(lb.x + ngx, lb.y + ngy, lb.z + ngz);\n" \
"	lb += bmin;\n" \
"	ub += bmin;\n" \
"	int4 ijk = (int4)(0, 0, 0, 1);\n" \
"	unsigned int index;\n" \
"	float lsum = 0.0f;\n" \
"	float v1, v2;\n" \
"	float4 coord;\n" \
"	int4 coordi;\n" \
"	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)\n" \
"	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)\n" \
"	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)\n" \
"	{\n" \
"		if (ijk.x < 0 || ijk.x >= nx || ijk.y < 0 || ijk.y >= ny || ijk.z < 0 || ijk.z >= nz)\n" \
"			v1 = 0.0f;\n" \
"		else\n" \
"		{\n" \
"			index = nxy*ijk.z + nx*ijk.y + ijk.x;\n" \
"			v1 = convert_float(img1[index]);\n" \
"		}\n" \
"		coord = convert_float4(ijk);\n" \
"		coord = (float4)(dot(coord, mat0), dot(coord, mat1), dot(coord, mat2), dot(coord, mat3));\n" \
"		coord /= coord.w;\n" \
"		coordi = convert_int4(coord);\n" \
"		if (coordi.x < 0 || coordi.x >= nx || coordi.y < 0 || coordi.y >= ny || coordi.z < 0 || coordi.z >= nz)\n" \
"			v2 = 0.0f;\n" \
"		else\n" \
"		{\n" \
"			index = nxy*coordi.z + nx*coordi.y + coordi.x;\n" \
"			v2 = convert_float(img2[index]);\n" \
"		}\n" \
"		v1 = (v1 - v2) / 255.0f;\n" \
"		v1 *= v1;\n" \
"		v1 = 1.0f - v1;\n" \
"		lsum += v1;\n" \
"	}\n" \
"	index = gsxy * gid.z + gsx * gid.y + gid.x;\n" \
"	atomic_xchg(sum+index, lsum);\n" \
"}\n" \
"//compare1 16 bit\n" \
"__kernel void kernel_5(\n" \
"	__global unsigned short* img1,\n" \
"	__global unsigned short* img2,\n" \
"	__global float* sum,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	unsigned int nxy,\n" \
"	unsigned int ngx,\n" \
"	unsigned int ngy,\n" \
"	unsigned int ngz,\n" \
"	unsigned int gsxy,\n" \
"	unsigned int gsx,\n" \
"	int3 bmin,\n" \
"	float4 mat0,\n" \
"	float4 mat1,\n" \
"	float4 mat2,\n" \
"	float4 mat3)\n" \
"{\n" \
"	int3 gid = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);\n" \
"	int3 ub = (int3)(lb.x + ngx, lb.y + ngy, lb.z + ngz);\n" \
"	lb += bmin;\n" \
"	ub += bmin;\n" \
"	int4 ijk = (int4)(0, 0, 0, 1);\n" \
"	unsigned int index;\n" \
"	float lsum = 0.0f;\n" \
"	float v1, v2;\n" \
"	float4 coord;\n" \
"	int4 coordi;\n" \
"	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)\n" \
"	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)\n" \
"	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)\n" \
"	{\n" \
"		if (ijk.x < 0 || ijk.x >= nx || ijk.y < 0 || ijk.y >= ny || ijk.z < 0 || ijk.z >= nz)\n" \
"			v1 = 0.0f;\n" \
"		else\n" \
"		{\n" \
"			index = nxy*ijk.z + nx*ijk.y + ijk.x;\n" \
"			v1 = convert_float(img1[index]);\n" \
"		}\n" \
"		coord = convert_float4(ijk);\n" \
"		coord = (float4)(dot(coord, mat0), dot(coord, mat1), dot(coord, mat2), dot(coord, mat3));\n" \
"		coord /= coord.w;\n" \
"		coordi = convert_int4(coord);\n" \
"		if (coordi.x < 0 || coordi.x >= nx || coordi.y < 0 || coordi.y >= ny || coordi.z < 0 || coordi.z >= nz)\n" \
"			v2 = 0.0f;\n" \
"		else\n" \
"		{\n" \
"			index = nxy*coordi.z + nx*coordi.y + coordi.x;\n" \
"			v2 = convert_float(img2[index]);\n" \
"		}\n" \
"		v1 = (v1 - v2) / 65535.0f;\n" \
"		v1 *= v1;\n" \
"		v1 = 1.0f - v1;\n" \
"		lsum += v1;\n" \
"	}\n" \
"	index = gsxy * gid.z + gsx * gid.y + gid.x;\n" \
"	atomic_xchg(sum+index, lsum);\n" \
"}\n" \
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
	//create program
	m_prog = flvr::VolumeRenderer::
		vol_kernel_factory_.kernel(str_cl_stencil);
}

StencilCompare::~StencilCompare()
{

}

void StencilCompare::Prepare()
{
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
	unsigned int nx = m_s1->nx, ny = m_s1->ny, nz = m_s1->nz;
	size_t local_size[3] = { 1, 1, 1 };
	size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };
	size_t buf_size = m_s1->bits == 8 ?
		sizeof(unsigned char) : sizeof(unsigned short);
	buf_size *= nx * ny * nz;
	//DBMIUINT8 mi(m_s1->nx, m_s1->ny, 1);

	//set up kernel
	m_prog->setKernelArgBegin(kernel_index);
	if (m_s1->fsize < 1)
		m_img1 = m_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, buf_size, (void*)(m_s1->data));
	else
	{
		img[0] = m_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, buf_size, (void*)(m_s1->data));
		img[1] = m_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY, buf_size, NULL);
		m_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nx));
		m_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&ny));
		m_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nz));
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
		}
		m_img1 = img[m_s1->fsize % 2];
		m_prog->releaseMemObject(img[(m_s1->fsize+1)%2]);
	}

	//set up kernel
	m_prog->setKernelArgBegin(kernel_index);
	if (m_s2->fsize < 1)
		m_img2 = m_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, buf_size, (void*)(m_s2->data));
	else
	{
		img[0] = m_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, buf_size, (void*)(m_s2->data));
		img[1] = m_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY, buf_size, NULL);
		m_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nx));
		m_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&ny));
		m_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nz));
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
			m_prog->executeKernel(kernel_index, 3, global_size, local_size);
		}
		m_img2 = img[m_s2->fsize % 2];
		m_prog->releaseMemObject(img[(m_s2->fsize + 1) % 2]);
	}
	//m_prog->readBuffer(m_img1, (void*)(mi.data));
	//m_prog->readBuffer(m_img2, (void*)(mi.data));
}

void StencilCompare::Clean()
{
	m_prog->releaseAll();
}

bool StencilCompare::Compare()
{
	Prepare();

	size_t bits = m_s1->bits;
	std::string name;
	switch (m_method)
	{
	case 0:
		if (bits == 8)
			name = "kernel_2";
		else
			name = "kernel_3";
		break;
	case 1:
		if (bits == 8)
			name = "kernel_4";
		else
			name = "kernel_5";
		break;
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
				p = Similar(name);
				//p = Similar();
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

float StencilCompare::Similar(const std::string& name)
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
	diag = fluo::Max(diag, fluo::Vector(1));
	flvr::GroupSize gsize;
	m_prog->get_group_size(kernel_index, diag.intx(), diag.inty(), diag.intz(), gsize);
	size_t global_size[3] = { size_t(gsize.gsx), size_t(gsize.gsy), size_t(gsize.gsz) };

	//set up kernel
	unsigned int nx = m_s1->nx, ny = m_s1->ny, nz = m_s1->nz;
	unsigned int nxy = nx * ny;
	cl_int3 bmin{
		m_s1->box.Min().intx(),
		m_s1->box.Min().inty(),
		m_s1->box.Min().intz() };
	cl_float4 tf0 = {
		float(m_s2->tf.get_mat_val(0, 0)),
		float(m_s2->tf.get_mat_val(0, 1)),
		float(m_s2->tf.get_mat_val(0, 2)),
		float(m_s2->tf.get_mat_val(0, 3)) };
	cl_float4 tf1 = {
		float(m_s2->tf.get_mat_val(1, 0)),
		float(m_s2->tf.get_mat_val(1, 1)),
		float(m_s2->tf.get_mat_val(1, 2)),
		float(m_s2->tf.get_mat_val(1, 3)) };
	cl_float4 tf2 = {
		float(m_s2->tf.get_mat_val(2, 0)),
		float(m_s2->tf.get_mat_val(2, 1)),
		float(m_s2->tf.get_mat_val(2, 2)),
		float(m_s2->tf.get_mat_val(2, 3)) };
	cl_float4 tf3 = {
		float(m_s2->tf.get_mat_val(3, 0)),
		float(m_s2->tf.get_mat_val(3, 1)),
		float(m_s2->tf.get_mat_val(3, 2)),
		float(m_s2->tf.get_mat_val(3, 3)) };
	float *sum = new float[gsize.gsxyz];
	//
	m_prog->setKernelArgBegin(kernel_index);
	m_prog->setKernelArgument(m_img1);
	m_prog->setKernelArgument(m_img2);
	flvr::Argument arg_sum = m_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(float)*(gsize.gsxyz), (void*)(sum));
	m_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nx));
	m_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&ny));
	m_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nz));
	m_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nxy));
	m_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngx));
	m_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngy));
	m_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngz));
	m_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.gsxy));
	m_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.gsx));
	m_prog->setKernelArgConst(sizeof(cl_int3), (void*)(&bmin));
	m_prog->setKernelArgConst(sizeof(cl_float4), (void*)(&tf0));
	m_prog->setKernelArgConst(sizeof(cl_float4), (void*)(&tf1));
	m_prog->setKernelArgConst(sizeof(cl_float4), (void*)(&tf2));
	m_prog->setKernelArgConst(sizeof(cl_float4), (void*)(&tf3));

	glHint(GL_LINE_SMOOTH, GL_DONT_CARE);
	//execute
	m_prog->executeKernel(kernel_index, 3, global_size, 0/*local_size*/);
	//read back
	m_prog->readBuffer(sizeof(float)*(gsize.gsxyz), sum, sum);
	m_prog->releaseMemObject(arg_sum);

	//sum
	for (int i = 0; i < gsize.gsxyz; ++i)
		result += sum[i];
	delete[] sum;

	return result;
}
