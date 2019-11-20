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
#include "Diffusion.h"
#include "cl_code.h"
#ifdef _DEBUG
#include <fstream>
#endif

using namespace FL;

Diffusion::Diffusion(VolumeData* vd)
	: m_vd(vd)
{
}

Diffusion::~Diffusion()
{
}

bool Diffusion::CheckBricks()
{
	if (!m_vd || !m_vd->GetTexture())
		return false;
	vector<TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks();
	if (!bricks || bricks->size() == 0)
		return false;
	return true;
}

void Diffusion::GetMask(size_t brick_num, TextureBrick* b, void** val)
{
	if (!b)
		return;

	m_vd->AddEmptyMask(0, false);
	Nrrd* nrrd_mask = m_vd->GetMask(true);
	if (brick_num > 1)
	{
		int c = b->nmask();
		int nb = b->nb(c);
		int nx = b->nx();
		int ny = b->ny();
		int nz = b->nz();
		unsigned long long mem_size = (unsigned long long)nx*
			(unsigned long long)ny*(unsigned long long)nz*(unsigned long long)nb;
		unsigned char* temp = new unsigned char[mem_size];
		unsigned char* tempp = temp;
		unsigned char* tp = (unsigned char*)(b->tex_data(c));
		unsigned char* tp2;
		for (unsigned int k = 0; k < nz; ++k)
		{
			tp2 = tp;
			for (unsigned int j = 0; j < ny; ++j)
			{
				memcpy(tempp, tp2, nx*nb);
				tempp += nx * nb;
				tp2 += b->sx()*nb;
			}
				tp += b->sx()*b->sy()*nb;
		}
		*val = (void*)temp;
	}
	else
	{
		if (!nrrd_mask)
			return;
		*val = (void*)(nrrd_mask->data);
	}
}

void Diffusion::ReleaseMask(void* val, size_t brick_num, TextureBrick* b)
{
	if (!val || brick_num <= 1)
		return;

	unsigned char* tempp = (unsigned char*)val;
	int c = b->nmask();
	int nb = b->nb(c);
	int nx = b->nx();
	int ny = b->ny();
	int nz = b->nz();
	unsigned char* tp = (unsigned char*)(b->tex_data(c));
	unsigned char* tp2;
	for (unsigned int k = 0; k < nz; ++k)
	{
		tp2 = tp;
		for (unsigned int j = 0; j < ny; ++j)
		{
			memcpy(tp2, tempp, nx*nb);
			tempp += nx * nb;
			tp2 += b->sx()*nb;
		}
		tp += b->sx()*b->sy()*nb;
	}
	delete[] val;
}

void Diffusion::Init(Point &ip, double ini_thresh)
{
	//debug
#ifdef _DEBUG
	unsigned int* val = 0;
	std::ofstream ofs;
#endif

	if (!CheckBricks())
		return;

	//create program and kernels
	KernelProgram* kernel_prog = VolumeRenderer::
		vol_kernel_factory_.kernel(str_cl_diffusion);
	if (!kernel_prog)
		return;
	int kernel_index = kernel_prog->createKernel("kernel_0");

	//clipping planes
	cl_float4 p[6];
	if (m_vd && m_vd->GetVR())
	{
		vector<Plane*> *planes = m_vd->GetVR()->get_planes();
		double abcd[4];
		for (size_t i = 0; i < 6; ++i)
		{
			(*planes)[i]->get(abcd);
			p[i] = { float(abcd[0]),
				float(abcd[1]),
				float(abcd[2]),
				float(abcd[3]) };
		}
	}

	size_t brick_num = m_vd->GetTexture()->get_brick_num();
	vector<FLIVR::TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks();
	for (size_t i = 0; i < brick_num; ++i)
	{
		TextureBrick* b = (*bricks)[i];
		int bits = b->nb(0) * 8;
		int nx = b->nx();
		int ny = b->ny();
		int nz = b->nz();
		GLint did = m_vd->GetVR()->load_brick(0, 0, bricks, i);
		void* val = 0;
		GetMask(brick_num, b, &val);

		size_t global_size[3] = { 1, 1, 1 };
		size_t local_size[3] = { 1, 1, 1 };

		//set
		kernel_prog->setKernelArgTex3D(kernel_index, 0,
			CL_MEM_READ_ONLY, did);
		kernel_prog->setKernelArgBuf(kernel_index, 1,
			CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			sizeof(unsigned char)*nx*ny*nz, val);
		kernel_prog->setKernelArgConst(kernel_index, 2,
			sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(kernel_index, 3,
			sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(kernel_index, 4,
			sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setKernelArgConst(kernel_index, 5,
			sizeof(cl_float4), (void*)(p));
		kernel_prog->setKernelArgConst(kernel_index, 6,
			sizeof(cl_float4), (void*)(p + 1));
		kernel_prog->setKernelArgConst(kernel_index, 7,
			sizeof(cl_float4), (void*)(p + 2));
		kernel_prog->setKernelArgConst(kernel_index, 8,
			sizeof(cl_float4), (void*)(p + 3));
		kernel_prog->setKernelArgConst(kernel_index, 9,
			sizeof(cl_float4), (void*)(p + 4));
		kernel_prog->setKernelArgConst(kernel_index, 10,
			sizeof(cl_float4), (void*)(p + 5));
		//brick matrix
		BBox bbx = b->dbox();
		cl_float3 scl = {
			float(bbx.max().x() - bbx.min().x()),
			float(bbx.max().y() - bbx.min().y()),
			float(bbx.max().z() - bbx.min().z()) };
		cl_float3 trl = {
			float(bbx.min().x()),
			float(bbx.min().y()),
			float(bbx.min().z()) };
		kernel_prog->setKernelArgConst(kernel_index, 11,
			sizeof(cl_float3), (void*)(&scl));
		kernel_prog->setKernelArgConst(kernel_index, 12,
			sizeof(cl_float3), (void*)(&trl));
		cl_float3 ipp = { float(ip.x()), float(ip.y()), float(ip.z()) };
		kernel_prog->setKernelArgConst(kernel_index, 13,
			sizeof(cl_float3), (void*)(&ipp));
		float thresh = float(ini_thresh);
		kernel_prog->setKernelArgConst(kernel_index, 14,
			sizeof(float), (void*)(&thresh));
		unsigned char init_val = 255;
		kernel_prog->setKernelArgConst(kernel_index, 15,
			sizeof(unsigned char), (void*)(&init_val));
		//execute
		kernel_prog->executeKernel(kernel_index, 3, global_size, local_size);
		//read back
		kernel_prog->readBuffer(sizeof(unsigned char)*nx*ny*nz, val, val);
		////debug
		//ofs.open("E:/DATA/Test/colocal/test.bin", std::ios::out | std::ios::binary);
		//ofs.write((char*)val, nx*ny*nz*sizeof(unsigned char));
		//ofs.close();

		//release buffer
		kernel_prog->releaseAll();
		ReleaseMask(val, brick_num, b);
	}
}

void Diffusion::Grow(int iter, double ini_thresh, double gm_falloff, double scl_falloff, double scl_translate)
{
	//debug
#ifdef _DEBUG
	unsigned int* val = 0;
	std::ofstream ofs;
#endif

	if (!CheckBricks())
		return;

	//create program and kernels
	KernelProgram* kernel_prog = VolumeRenderer::
		vol_kernel_factory_.kernel(str_cl_diffusion);
	if (!kernel_prog)
		return;
	int kernel_index = kernel_prog->createKernel("kernel_1");

	//clipping planes
	cl_float4 p[6];
	bool inv;
	float scalar_scale, lo_thresh, hi_thresh, gamma3d, gm_thresh,
		offset, sw;
	if (m_vd && m_vd->GetVR())
	{
		VolumeRenderer* vr = m_vd->GetVR();
		vector<Plane*> *planes = vr->get_planes();
		double abcd[4];
		for (size_t i = 0; i < 6; ++i)
		{
			(*planes)[i]->get(abcd);
			p[i] = { float(abcd[0]),
				float(abcd[1]),
				float(abcd[2]),
				float(abcd[3]) };
		}
		//params
		inv = m_vd->GetInvert();
		scalar_scale = m_vd->GetScalarScale();
		lo_thresh = m_vd->GetLeftThresh();
		hi_thresh = m_vd->GetRightThresh();
		gamma3d = m_vd->Get3DGamma();
		gm_thresh = m_vd->GetBoundary();
		offset = m_vd->GetOffset();
		sw = m_vd->GetSoftThreshold();
	}


	size_t brick_num = m_vd->GetTexture()->get_brick_num();
	vector<FLIVR::TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks();
	for (size_t i = 0; i < brick_num; ++i)
	{
		TextureBrick* b = (*bricks)[i];
		int bits = b->nb(0) * 8;
		int nx = b->nx();
		int ny = b->ny();
		int nz = b->nz();
		GLint did = m_vd->GetVR()->load_brick(0, 0, bricks, i);
		void* val = 0;
		GetMask(brick_num, b, &val);

		size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };
		size_t local_size[3] = { 1, 1, 1 };

		//set
		kernel_prog->setKernelArgTex3D(kernel_index, 0,
			CL_MEM_READ_ONLY, did);
		kernel_prog->setKernelArgBuf(kernel_index, 1,
			CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			sizeof(unsigned char)*nx*ny*nz, val);
		kernel_prog->setKernelArgConst(kernel_index, 2,
			sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(kernel_index, 3,
			sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(kernel_index, 4,
			sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setKernelArgConst(kernel_index, 5,
			sizeof(cl_float4), (void*)(p));
		kernel_prog->setKernelArgConst(kernel_index, 6,
			sizeof(cl_float4), (void*)(p + 1));
		kernel_prog->setKernelArgConst(kernel_index, 7,
			sizeof(cl_float4), (void*)(p + 2));
		kernel_prog->setKernelArgConst(kernel_index, 8,
			sizeof(cl_float4), (void*)(p + 3));
		kernel_prog->setKernelArgConst(kernel_index, 9,
			sizeof(cl_float4), (void*)(p + 4));
		kernel_prog->setKernelArgConst(kernel_index, 10,
			sizeof(cl_float4), (void*)(p + 5));
		//brick matrix
		BBox bbx = b->dbox();
		cl_float3 scl = {
			float(bbx.max().x() - bbx.min().x()),
			float(bbx.max().y() - bbx.min().y()),
			float(bbx.max().z() - bbx.min().z()) };
		cl_float3 trl = {
			float(bbx.min().x()),
			float(bbx.min().y()),
			float(bbx.min().z()) };
		kernel_prog->setKernelArgConst(kernel_index, 11,
			sizeof(cl_float3), (void*)(&scl));
		kernel_prog->setKernelArgConst(kernel_index, 12,
			sizeof(cl_float3), (void*)(&trl));
		cl_float4 loc2 = { inv ? -scalar_scale : scalar_scale, 1.0, lo_thresh, hi_thresh };
		kernel_prog->setKernelArgConst(kernel_index, 13,
			sizeof(cl_float4), (void*)(&loc2));
		cl_float4 loc3 = { 1.0f / gamma3d, gm_thresh, offset, sw };
		kernel_prog->setKernelArgConst(kernel_index, 14,
			sizeof(cl_float4), (void*)(&loc3));
		cl_float4 loc7 = { float(ini_thresh), float(gm_falloff), float(scl_falloff), float(scl_translate) };
		kernel_prog->setKernelArgConst(kernel_index, 15,
			sizeof(cl_float4), (void*)(&loc7));
		//execute
		for (int i = 0; i<iter; ++i)
			kernel_prog->executeKernel(kernel_index, 3, global_size, local_size);
		//read back
		kernel_prog->readBuffer(sizeof(unsigned char)*nx*ny*nz, val, val);
		////debug
		//ofs.open("E:/DATA/Test/colocal/test.bin", std::ios::out | std::ios::binary);
		//ofs.write((char*)val, nx*ny*nz*sizeof(unsigned char));
		//ofs.close();

		//release buffer
		kernel_prog->releaseAll();
		ReleaseMask(val, brick_num, b);
	}
}