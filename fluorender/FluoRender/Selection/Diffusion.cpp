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
#ifdef _DEBUG
#include <fstream>
#endif

using namespace fls;

const char* str_cl_diffusion = \
"const sampler_t samp =\n" \
"	CLK_NORMALIZED_COORDS_FALSE|\n" \
"	CLK_ADDRESS_CLAMP_TO_EDGE|\n" \
"	CLK_FILTER_LINEAR;\n" \
"\n" \
"__kernel void kernel_0(\n" \
"	__read_only image3d_t data,\n" \
"	__global unsigned char* mask,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	float4 p0,\n" \
"	float4 p1,\n" \
"	float4 p2,\n" \
"	float4 p3,\n" \
"	float4 p4,\n" \
"	float4 p5,\n" \
"	float3 scl,\n" \
"	float3 trl,\n" \
"	float3 p,\n" \
"	float thresh,\n" \
"	unsigned char val)\n" \
"{\n" \
"	unsigned int i = (int)(p.x);\n" \
"	unsigned int j = (int)(p.y);\n" \
"	unsigned int k = (int)(p.z);\n" \
"	float v = read_imagef(data, samp, (int4)(i, j, k, 1)).x;\n" \
"	if (v <= thresh)\n" \
"		return;\n" \
"	unsigned int index = nx*ny*k + nx*j + i;\n" \
"	float3 pt = (float3)((float)(i) / (float)(nx), (float)(j) / (float)(ny), (float)(k) / (float)(nz));\n" \
"	pt = pt * scl + trl;\n" \
"	if (dot(pt, p0.xyz)+p0.w < 0.0 ||\n" \
"		dot(pt, p1.xyz)+p1.w < 0.0 ||\n" \
"		dot(pt, p2.xyz)+p2.w < 0.0 ||\n" \
"		dot(pt, p3.xyz)+p3.w < 0.0 ||\n" \
"		dot(pt, p4.xyz)+p4.w < 0.0 ||\n" \
"		dot(pt, p5.xyz)+p5.w < 0.0)\n" \
"		return;\n" \
"	mask[index] = val;\n" \
"}\n" \
"__kernel void kernel_1(\n" \
"	__read_only image3d_t data,\n" \
"	__global unsigned char* mask,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	float4 p0,\n" \
"	float4 p1,\n" \
"	float4 p2,\n" \
"	float4 p3,\n" \
"	float4 p4,\n" \
"	float4 p5,\n" \
"	float3 scl,\n" \
"	float3 trl,\n" \
"	float4 loc2,\n" \
"	float4 loc3,\n" \
"	float4 loc7)\n" \
"{\n" \
"	unsigned int i = (unsigned int)(get_global_id(0));\n" \
"	unsigned int j = (unsigned int)(get_global_id(1));\n" \
"	unsigned int k = (unsigned int)(get_global_id(2));\n" \
"	unsigned int index = nx*ny*k + nx*j + i;\n" \
"	float3 dir = (float3)(1.0/(float)(nx), 1.0/(float)(ny), 1.0/(float)(nz));\n" \
"	float3 pt = dir * (float3)((float)(i), (float)(j), (float)(k));\n" \
"	pt = pt * scl + trl;\n" \
"	if (dot(pt, p0.xyz)+p0.w < 0.0 ||\n" \
"		dot(pt, p1.xyz)+p1.w < 0.0 ||\n" \
"		dot(pt, p2.xyz)+p2.w < 0.0 ||\n" \
"		dot(pt, p3.xyz)+p3.w < 0.0 ||\n" \
"		dot(pt, p4.xyz)+p4.w < 0.0 ||\n" \
"		dot(pt, p5.xyz)+p5.w < 0.0)\n" \
"		return;\n" \
"	//grad compute\n" \
"	float3 v;\n" \
"	v.x = read_imagef(data, samp, (int4)(i, j, k, 1)).x;\n" \
"	float3 n = (float3)(0.0);\n" \
"	n.x += read_imagef(data, samp, (int4)(i+1, j, k, 1)).x;\n" \
"	n.x -= read_imagef(data, samp, (int4)(i-1, j, k, 1)).x;\n" \
"	n.y += read_imagef(data, samp, (int4)(i, j+1, k, 1)).x;\n" \
"	n.y -= read_imagef(data, samp, (int4)(i, j-1, k, 1)).x;\n" \
"	n.z += read_imagef(data, samp, (float4)((float)(i), (float)(j), (float)(k)+min((float)(nz)/(float)(nx), 1.0f), 1.0)).x;\n" \
"	n.z -= read_imagef(data, samp, (float4)((float)(i), (float)(j), (float)(k)-min((float)(nz)/(float)(nx), 1.0f), 1.0)).x;\n" \
"	v.y = length(n);\n" \
"	v.y = 0.5 * (loc2.x<0.0?(1.0+v.y*loc2.x):v.y*loc2.x);\n" \
"	//VOL_TRANSFER_FUNCTION_SIN_COLOR_L\n" \
"	float c;\n" \
"	v.x = loc2.x < 0.0 ? (1.0 + v.x*loc2.x) : v.x*loc2.x;\n" \
"	if (v.x < loc2.z - loc3.w || (loc2.w<1.0 && v.x>loc2.w + loc3.w))\n" \
"		c = 0.0;\n" \
"	else\n" \
"	{\n" \
"		v.x = (v.x < loc2.z ? (loc3.w - loc2.z + v.x) / loc3.w : (loc2.w<1.0 && v.x>loc2.w ? (loc3.w - v.x + loc2.w) / loc3.w : 1.0))*v.x;\n" \
"		v.x = (loc3.y > 0.0 ? clamp(v.y / loc3.y, 0.0f, 1.0f + loc3.y*10.0f) : 1.0)*v.x;\n" \
"		c = pow(clamp(v.x / loc3.z, loc3.x<1.0 ? -(loc3.x - 1.0f)*0.00001f : 0.0f, loc3.x>1.0 ? 0.9999f : 1.0f), loc3.x);\n" \
"	}\n" \
"	//SEG_BODY_DB_GROW_STOP_FUNC\n" \
"	if (c <= 0.0001)\n" \
"		return;\n" \
"	v.x = c > 1.0 ? 1.0 : c;\n" \
"	float stop =\n" \
"		(loc7.y >= 1.0 ? 1.0 : (v.y > sqrt(loc7.y)*2.12 ? 0.0 : exp(-v.y*v.y / loc7.y)))*\n" \
"		(v.x > loc7.w ? 1.0 : (loc7.z > 0.0 ? (v.x < loc7.w - sqrt(loc7.z)*2.12 ? 0.0 : exp(-(v.x - loc7.w)*(v.x - loc7.w) / loc7.z)) : 0.0));\n" \
"	if (stop <= 0.0001)\n" \
"		return;\n" \
"	//SEG_BODY_DB_GROW_BLEND_APPEND\n" \
"	unsigned char cc = mask[index];\n" \
"	float val = (1.0 - stop) * (float)(cc);\n" \
"	int3 nb_coord;\n" \
"	int3 max_nb;\n" \
"	unsigned int nb_index;\n" \
"	unsigned char m;\n" \
"	unsigned char mx;\n" \
"	for (int ii = -1; ii < 2; ii++)\n" \
"	for (int jj = -1; jj < 2; jj++)\n" \
"	for (int kk = -1; kk < 2; kk++)\n" \
"	{\n" \
"		nb_coord = (int3)(ii+i, jj+j, kk+k);\n" \
"		if (nb_coord.x < 0 || nb_coord.x > nx-1 ||\n" \
"			nb_coord.y < 0 || nb_coord.y > ny-1 ||\n" \
"			nb_coord.z < 0 || nb_coord.z > nz-1)\n" \
"			continue;\n" \
"		nb_index = nx*ny*nb_coord.z + nx*nb_coord.y + nb_coord.x;\n" \
"		m = mask[nb_index];\n" \
"		if (m > cc)\n" \
"		{\n" \
"			cc = m;\n" \
"			max_nb = nb_coord;\n" \
"		}\n" \
"	}\n" \
"	if (loc7.y > 0.0)\n" \
"	{\n" \
"		m = (unsigned char)((read_imagef(data, samp, (int4)(max_nb, 1)).x + loc7.y) * 255.0);\n" \
"		mx = (unsigned char)(read_imagef(data, samp, (int4)(i, j, k, 1)).x * 255.0);\n" \
"		if (m < mx || m - mx > (unsigned char)(510.0*loc7.y))\n" \
"			return;\n" \
"	}\n" \
"	cc = clamp(cc * (unsigned char)(stop * 255.0), 0, 255);\n" \
"	mask[index] = cc;\n" \
"}\n" \
;

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
	vector<flvr::TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks();
	if (!bricks || bricks->size() == 0)
		return false;
	return true;
}

void Diffusion::GetMask(size_t brick_num, flvr::TextureBrick* b, void** val)
{
	if (!b)
		return;

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

void Diffusion::ReleaseMask(void* val, size_t brick_num, flvr::TextureBrick* b)
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

void Diffusion::Init(fluo::Point &ip, double ini_thresh)
{
	//debug
#ifdef _DEBUG
	unsigned int* val = 0;
	std::ofstream ofs;
#endif

	if (!CheckBricks())
		return;

	//add empty mask if there is no mask
	//then, push the mask for undos
	m_vd->AddEmptyMask(0, false);
	if (flvr::Texture::mask_undo_num_ > 0 &&
		m_vd->GetTexture())
		m_vd->GetTexture()->push_mask();

	//create program and kernels
	flvr::KernelProgram* kernel_prog = flvr::VolumeRenderer::
		vol_kernel_factory_.kernel(str_cl_diffusion);
	if (!kernel_prog)
		return;
	int kernel_index = kernel_prog->createKernel("kernel_0");

	//clipping planes
	cl_float4 p[6];
	if (m_vd && m_vd->GetVR())
	{
		vector<fluo::Plane*> *planes = m_vd->GetVR()->get_planes();
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
	vector<flvr::TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks();
	for (size_t i = 0; i < brick_num; ++i)
	{
		flvr::TextureBrick* b = (*bricks)[i];
		int bits = b->nb(0) * 8;
		int nx = b->nx();
		int ny = b->ny();
		int nz = b->nz();
		GLint did = m_vd->GetVR()->load_brick(b);
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
		fluo::BBox bbx = b->dbox();
		cl_float3 scl = {
			float(bbx.Max().x() - bbx.Min().x()),
			float(bbx.Max().y() - bbx.Min().y()),
			float(bbx.Max().z() - bbx.Min().z()) };
		cl_float3 trl = {
			float(bbx.Min().x()),
			float(bbx.Min().y()),
			float(bbx.Min().z()) };
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
	flvr::KernelProgram* kernel_prog = flvr::VolumeRenderer::
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
		flvr::VolumeRenderer* vr = m_vd->GetVR();
		vector<fluo::Plane*> *planes = vr->get_planes();
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
	vector<flvr::TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks();
	for (size_t i = 0; i < brick_num; ++i)
	{
		flvr::TextureBrick* b = (*bricks)[i];
		int bits = b->nb(0) * 8;
		int nx = b->nx();
		int ny = b->ny();
		int nz = b->nz();
		GLint did = m_vd->GetVR()->load_brick(b);
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
		fluo::BBox bbx = b->dbox();
		cl_float3 scl = {
			float(bbx.Max().x() - bbx.Min().x()),
			float(bbx.Max().y() - bbx.Min().y()),
			float(bbx.Max().z() - bbx.Min().z()) };
		cl_float3 trl = {
			float(bbx.Min().x()),
			float(bbx.Min().y()),
			float(bbx.Min().z()) };
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