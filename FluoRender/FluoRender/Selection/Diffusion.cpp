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
#include <Diffusion.h>
#include <Global.h>
#include <KernelProgram.h>
#include <VolumeData.h>
#include <Texture.h>
#include <TextureBrick.h>
#include <Kernel.h>
#include <VolumeRenderer.h>
#include <Plane.h>
#include <vector>

using namespace flrd;

constexpr const char* str_cl_diffusion = R"CLKER(
const sampler_t samp =
	CLK_NORMALIZED_COORDS_FALSE|
	CLK_ADDRESS_CLAMP_TO_EDGE|
	CLK_FILTER_LINEAR;

__kernel void kernel_0(
	__read_only image3d_t data,
	__global unsigned char* mask,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz,
	float4 p0,
	float4 p1,
	float4 p2,
	float4 p3,
	float4 p4,
	float4 p5,
	float3 scl,
	float3 trl,
	float3 p,
	float thresh,
	unsigned char val)
{
	unsigned int i = (int)(p.x);
	unsigned int j = (int)(p.y);
	unsigned int k = (int)(p.z);
	float v = read_imagef(data, samp, (int4)(i, j, k, 1)).x;
	if (v <= thresh)
		return;
	unsigned int index = nx*ny*k + nx*j + i;
	float3 pt = (float3)((float)(i) / (float)(nx), (float)(j) / (float)(ny), (float)(k) / (float)(nz));
	pt = pt * scl + trl;
	if (dot(pt, p0.xyz)+p0.w < 0.0f ||
		dot(pt, p1.xyz)+p1.w < 0.0f ||
		dot(pt, p2.xyz)+p2.w < 0.0f ||
		dot(pt, p3.xyz)+p3.w < 0.0f ||
		dot(pt, p4.xyz)+p4.w < 0.0f ||
		dot(pt, p5.xyz)+p5.w < 0.0f)
		return;
	mask[index] = val;
}
__kernel void kernel_1(
	__read_only image3d_t data,
	__global unsigned char* mask,
	unsigned int nx,
	unsigned int ny,
	unsigned int nz,
	float4 p0,
	float4 p1,
	float4 p2,
	float4 p3,
	float4 p4,
	float4 p5,
	float3 scl,
	float3 trl,
	float4 loc2,
	float4 loc3,
	float4 loc7)
{
	unsigned int i = (unsigned int)(get_global_id(0));
	unsigned int j = (unsigned int)(get_global_id(1));
	unsigned int k = (unsigned int)(get_global_id(2));
	unsigned int index = nx*ny*k + nx*j + i;
	float3 dir = (float3)(1.0f/(float)(nx), 1.0f/(float)(ny), 1.0f/(float)(nz));
	float3 pt = dir * (float3)((float)(i), (float)(j), (float)(k));
	pt = pt * scl + trl;
	if (dot(pt, p0.xyz)+p0.w < 0.0f ||
		dot(pt, p1.xyz)+p1.w < 0.0f ||
		dot(pt, p2.xyz)+p2.w < 0.0f ||
		dot(pt, p3.xyz)+p3.w < 0.0f ||
		dot(pt, p4.xyz)+p4.w < 0.0f ||
		dot(pt, p5.xyz)+p5.w < 0.0f)
		return;
	//grad compute
	float3 v;
	v.x = read_imagef(data, samp, (int4)(i, j, k, 1)).x;
	float3 n = (float3)(0.0f);
	n.x += read_imagef(data, samp, (int4)(i+1, j, k, 1)).x;
	n.x -= read_imagef(data, samp, (int4)(i-1, j, k, 1)).x;
	n.y += read_imagef(data, samp, (int4)(i, j+1, k, 1)).x;
	n.y -= read_imagef(data, samp, (int4)(i, j-1, k, 1)).x;
	n.z += read_imagef(data, samp, (float4)((float)(i), (float)(j), (float)(k)+min((float)(nz)/(float)(nx), 1.0f), 1.0f)).x;
	n.z -= read_imagef(data, samp, (float4)((float)(i), (float)(j), (float)(k)-min((float)(nz)/(float)(nx), 1.0f), 1.0f)).x;
	v.y = length(n);
	v.y = 0.5f * (loc2.x<0.0f?(1.0f+v.y*loc2.x):v.y*loc2.x);
	//VOL_TRANSFER_FUNCTION_SIN_COLOR_L
	float c;
	v.x = loc2.x < 0.0f ? (1.0f + v.x*loc2.x) : v.x*loc2.x;
	if (v.x < loc2.z - loc3.w || (loc2.w<1.0 && v.x>loc2.w + loc3.w))
		c = 0.0f;
	else
	{
		v.x = (v.x < loc2.z ? (loc3.w - loc2.z + v.x) / loc3.w : (loc2.w<1.0f && v.x>loc2.w ? (loc3.w - v.x + loc2.w) / loc3.w : 1.0f))*v.x;
		v.x = (loc2.y > 0.0f ? clamp(v.y / loc2.y, 0.0f, 1.0f + loc2.y*10.0f) : 1.0f)*v.x;
		c = pow(clamp((v.x-loc3.y)/(loc3.z-loc3.y), loc3.x<1.0f ? -(loc3.x - 1.0f)*0.00001f : 0.0f, 1.0f), loc3.x);
	}
	//SEG_BODY_DB_GROW_STOP_FUNC
	if (c <= 0.0001f)
		return;
	v.x = c > 1.0f ? 1.0f : c;
	float stop =
		(loc7.y >= 1.0f ? 1.0f : (v.y > sqrt(loc7.y)*2.12f ? 0.0f : exp(-v.y*v.y / loc7.y)))*
		(v.x > loc7.w ? 1.0f : (loc7.z > 0.0f ? (v.x < loc7.w - sqrt(loc7.z)*2.12f ? 0.0 : exp(-(v.x - loc7.w)*(v.x - loc7.w) / loc7.z)) : 0.0f));
	if (stop <= 0.0001f)
		return;
	//SEG_BODY_DB_GROW_BLEND_APPEND
	unsigned char cc = mask[index];
	float val = (1.0f - stop) * (float)(cc);
	int3 nb_coord;
	int3 max_nb;
	unsigned int nb_index;
	unsigned char m;
	unsigned char mx;
	for (int ii = -1; ii < 2; ii++)
	for (int jj = -1; jj < 2; jj++)
	for (int kk = -1; kk < 2; kk++)
	{
		nb_coord = (int3)(ii+i, jj+j, kk+k);
		if (nb_coord.x < 0 || nb_coord.x > nx-1 ||
			nb_coord.y < 0 || nb_coord.y > ny-1 ||
			nb_coord.z < 0 || nb_coord.z > nz-1)
			continue;
		nb_index = nx*ny*nb_coord.z + nx*nb_coord.y + nb_coord.x;
		m = mask[nb_index];
		if (m > cc)
		{
			cc = m;
			max_nb = nb_coord;
		}
	}
	if (loc7.y > 0.0f)
	{
		m = (unsigned char)((read_imagef(data, samp, (int4)(max_nb, 1)).x + loc7.y) * 255.0f);
		mx = (unsigned char)(read_imagef(data, samp, (int4)(i, j, k, 1)).x * 255.0f);
		if (m < mx || m - mx > (unsigned char)(510.0f*loc7.y))
			return;
	}
	cc = clamp(cc * (unsigned char)(stop * 255.0f), 0, 255);
	mask[index] = cc;
}
)CLKER";

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
	std::vector<flvr::TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks();
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
		unsigned int nx = b->nx();
		unsigned int ny = b->ny();
		unsigned int nz = b->nz();
		unsigned long long mem_size = (unsigned long long)nx*
			(unsigned long long)ny*(unsigned long long)nz*(unsigned long long)nb;
		unsigned char* temp = new unsigned char[mem_size];
		unsigned char* tempp = temp;
		unsigned char* tp = (unsigned char*)(b->tex_data(c));
		unsigned char* tp2;
		for (size_t k = 0; k < nz; ++k)
		{
			tp2 = tp;
			for (size_t j = 0; j < ny; ++j)
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
	unsigned int nx = b->nx();
	unsigned int ny = b->ny();
	unsigned int nz = b->nz();
	unsigned char* tp = (unsigned char*)(b->tex_data(c));
	unsigned char* tp2;
	for (size_t k = 0; k < nz; ++k)
	{
		tp2 = tp;
		for (size_t j = 0; j < ny; ++j)
		{
			memcpy(tp2, tempp, nx*nb);
			tempp += nx * nb;
			tp2 += b->sx()*nb;
		}
		tp += b->sx()*b->sy()*nb;
	}
	delete[] (unsigned char*)val;
}

void Diffusion::Init(fluo::Point &ip, double ini_thresh)
{
	if (!CheckBricks())
		return;

	//add empty mask if there is no mask
	//then, push the mask for undos
	m_vd->AddEmptyMask(0, false);
	if (flvr::Texture::mask_undo_num_ > 0 &&
		m_vd->GetTexture())
		m_vd->GetTexture()->push_mask();
	long bits = m_vd->GetBits();
	float max_int = static_cast<float>(m_vd->GetMaxValue());

	//create program and kernels
	flvr::KernelProgram* kernel_prog = glbin_kernel_factory.kernel(str_cl_diffusion, bits, max_int);
	if (!kernel_prog)
		return;
	int kernel_index = kernel_prog->createKernel("kernel_0");

	//clipping planes
	cl_float4 p[6];
	if (m_vd && m_vd->GetVR())
	{
		std::vector<fluo::Plane*> *planes = m_vd->GetVR()->get_planes();
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
	std::vector<flvr::TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks();
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
		cl_float3 ipp = { float(ip.x()), float(ip.y()), float(ip.z()) };
		float thresh = float(ini_thresh);
		unsigned char init_val = 255;
		kernel_prog->setKernelArgBegin(kernel_index);
		kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, did);
		auto arg_val =
			kernel_prog->setKernelArgBuf(CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, "arg_val", sizeof(unsigned char) * nx * ny * nz, val);
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(p));
		kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(p + 1));
		kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(p + 2));
		kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(p + 3));
		kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(p + 4));
		kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(p + 5));
		kernel_prog->setKernelArgConst(sizeof(cl_float3), (void*)(&scl));
		kernel_prog->setKernelArgConst(sizeof(cl_float3), (void*)(&trl));
		kernel_prog->setKernelArgConst(sizeof(cl_float3), (void*)(&ipp));
		kernel_prog->setKernelArgConst(sizeof(float), (void*)(&thresh));
		kernel_prog->setKernelArgConst(sizeof(unsigned char), (void*)(&init_val));
		//execute
		kernel_prog->executeKernel(kernel_index, 3, global_size, local_size);
		//read back
		kernel_prog->readBuffer(arg_val, val);

		//release buffer
		kernel_prog->releaseAllArgs();
		ReleaseMask(val, brick_num, b);
	}
}

void Diffusion::Grow(int iter, double ini_thresh, double gm_falloff, double scl_falloff, double scl_translate)
{
	if (!CheckBricks())
		return;

	long bits = m_vd->GetBits();
	float max_int = static_cast<float>(m_vd->GetMaxValue());

	//create program and kernels
	flvr::KernelProgram* kernel_prog = glbin_kernel_factory.kernel(str_cl_diffusion, bits, max_int);
	if (!kernel_prog)
		return;
	int kernel_index = kernel_prog->createKernel("kernel_1");

	//clipping planes
	cl_float4 p[6];
	bool inv;
	float scalar_scale, lo_thresh, hi_thresh, gamma3d,
		gm_scale, gm_low, gm_high, gm_max,
		lo_offset, hi_offset, sw;
	if (m_vd && m_vd->GetVR())
	{
		flvr::VolumeRenderer* vr = m_vd->GetVR();
		std::vector<fluo::Plane*> *planes = vr->get_planes();
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
		scalar_scale = static_cast<float>(m_vd->GetScalarScale());
		lo_thresh = static_cast<float>(m_vd->GetLeftThresh());
		hi_thresh = static_cast<float>(m_vd->GetRightThresh());
		gamma3d = static_cast<float>(m_vd->GetGamma());
		gm_scale = static_cast<float>(m_vd->GetGMScale());
		gm_low = static_cast<float>(m_vd->GetBoundaryLow());
		gm_high = static_cast<float>(m_vd->GetBoundaryHigh());
		gm_max = static_cast<float>(m_vd->GetBoundaryMax());
		lo_offset = static_cast<float>(m_vd->GetLowOffset());
		hi_offset = static_cast<float>(m_vd->GetHighOffset());
		sw = static_cast<float>(m_vd->GetSoftThreshold());
	}

	size_t brick_num = m_vd->GetTexture()->get_brick_num();
	std::vector<flvr::TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks();
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
		cl_float4 loc2 = { inv ? -scalar_scale : scalar_scale, gm_scale, lo_thresh, hi_thresh };
		cl_float4 loc3 = { 1.0f / gamma3d, lo_offset, hi_offset, sw };
		cl_float4 loc7 = { float(ini_thresh), float(gm_falloff), float(scl_falloff), float(scl_translate) };
		kernel_prog->setKernelArgBegin(kernel_index);
		kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, did);
		auto arg_val =
			kernel_prog->setKernelArgBuf(CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, "arg_val", sizeof(unsigned char) * nx * ny * nz, val);
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(p));
		kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(p + 1));
		kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(p + 2));
		kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(p + 3));
		kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(p + 4));
		kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(p + 5));
		kernel_prog->setKernelArgConst(sizeof(cl_float3), (void*)(&scl));
		kernel_prog->setKernelArgConst(sizeof(cl_float3), (void*)(&trl));
		kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(&loc2));
		kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(&loc3));
		kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(&loc7));
		//execute
		for (int i = 0; i<iter; ++i)
			kernel_prog->executeKernel(kernel_index, 3, global_size, local_size);
		//read back
		kernel_prog->readBuffer(arg_val, val);

		//release buffer
		kernel_prog->releaseAllArgs();
		ReleaseMask(val, brick_num, b);
	}
}