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
#include <DataManager.h>
#include "CompGenerator.h"
#include "cl_code.h"
#include "cl_code_db.h"
#include <Global.h>
#include <Database/EntryHist.h>
#include <Database/EntryParams.h>
#include <algorithm>
#ifdef _DEBUG
#include <fstream>
#include <Debug.h>
#endif

using namespace flrd;

ComponentGenerator::ComponentGenerator()
	: m_vd(0),
	prework(0),
	postwork(0)
{
}

ComponentGenerator::~ComponentGenerator()
{
}

bool ComponentGenerator::CheckBricks()
{
	if (!m_vd || !m_vd->GetTexture())
		return false;
	vector<flvr::TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks();
	if (!bricks || bricks->size() == 0)
		return false;
	return true;
}

void ComponentGenerator::ShuffleID()
{
	if (!CheckBricks())
		return;

	//create program and kernels
	flvr::KernelProgram* kernel_prog = glbin_vol_kernel_factory.kernel(str_cl_shuffle_id_3d);
	if (!kernel_prog)
		return;
	int kernel_index;
	if (m_use_sel)
		kernel_index = kernel_prog->createKernel("kernel_1");
	else
		kernel_index = kernel_prog->createKernel("kernel_0");

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
		if (prework)
			prework("");

		flvr::TextureBrick* b = (*bricks)[i];
		if (m_use_sel)
		{
			if (!b->is_mask_valid())
				continue;
		}
		else
			b->valid_mask();

		int bits = b->nb(0) * 8;
		int nx = b->nx();
		int ny = b->ny();
		int nz = b->nz();
		GLint did = m_vd->GetVR()->load_brick(b);
		GLint mid = 0;
		if (m_use_sel)
			mid = m_vd->GetVR()->load_brick_mask(b);
		GLint lid = m_vd->GetVR()->load_brick_label(b);

		size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };
		size_t local_size[3] = { 1, 1, 1 };
		//bit length
		unsigned int lenx = 0;
		unsigned int r = std::max(nx, ny);
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
		//brick matrix
		fluo::BBox bbx = b->dbox();
		cl_float3 scl = {
			float(bbx.Max().x() - bbx.Min().x()),
			float(bbx.Max().y() - bbx.Min().y()),
			float(bbx.Max().z() - bbx.Min().z()) };
		cl_float3 trl ={
			float(bbx.Min().x()),
			float(bbx.Min().y()),
			float(bbx.Min().z())};
		kernel_prog->setKernelArgBegin(kernel_index);
		kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, did);
		flvr::Argument arg_label =
			kernel_prog->setKernelArgTex3DBuf(CL_MEM_READ_WRITE, lid, sizeof(unsigned int)*nx*ny*nz, region);
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&lenx));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&lenz));
		kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(p));
		kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(p+1));
		kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(p+2));
		kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(p+3));
		kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(p+4));
		kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(p+5));
		kernel_prog->setKernelArgConst(sizeof(cl_float3), (void*)(&scl));
		kernel_prog->setKernelArgConst(sizeof(cl_float3), (void*)(&trl));
		if (m_use_sel)
			kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, mid);
		//execute
		kernel_prog->executeKernel(kernel_index, 3, global_size, local_size);
		//read back
		kernel_prog->copyBufTex3D(arg_label, lid,
			sizeof(unsigned int)*nx*ny*nz, region);

		//release buffer
		kernel_prog->releaseAll();

		if (postwork)
			postwork(__FUNCTION__);
	}
}

void ComponentGenerator::SetIDBit(int psize)
{
	//debug
#ifdef _DEBUG
	unsigned int* val = 0;
	std::ofstream ofs;
#endif

	if (!CheckBricks())
		return;

	//create program and kernels
	flvr::KernelProgram* kernel_prog = glbin_vol_kernel_factory.kernel(str_cl_set_bit_3d);
	if (!kernel_prog)
		return;
	int kernel_index0;
	//int kernel_index1;
	int kernel_index2;
	int kernel_index3;
	if (m_use_sel)
	{
		kernel_index0 = kernel_prog->createKernel("kernel_4");
		//kernel_index1 = kernel_prog->createKernel("kernel_5");
		kernel_index2 = kernel_prog->createKernel("kernel_6");
		kernel_index3 = kernel_prog->createKernel("kernel_7");
	}
	else
	{
		kernel_index0 = kernel_prog->createKernel("kernel_0");
		//kernel_index1 = kernel_prog->createKernel("kernel_1");
		kernel_index2 = kernel_prog->createKernel("kernel_2");
		kernel_index3 = kernel_prog->createKernel("kernel_3");
	}

	size_t brick_num = m_vd->GetTexture()->get_brick_num();
	vector<flvr::TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks();
	for (size_t i = 0; i < brick_num; ++i)
	{
		if (prework)
			prework("");

		flvr::TextureBrick* b = (*bricks)[i];
		if (m_use_sel)
		{
			if (!b->is_mask_valid())
				continue;
		}
		else
			b->valid_mask();

		int bits = b->nb(0) * 8;
		int nx = b->nx();
		int ny = b->ny();
		int nz = b->nz();
		GLint mid = 0;
		if (m_use_sel)
			mid = m_vd->GetVR()->load_brick_mask(b);
		GLint lid = m_vd->GetVR()->load_brick_label(b);

		size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };
		size_t local_size[3] = { 1, 1, 1 };

		//bit length
		unsigned int lenx = 0;
		unsigned int r = std::max(nx, ny);
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

		unsigned long long data_size =
			(unsigned long long)nx *
			(unsigned long long)ny *
			(unsigned long long)nz;
		unsigned long long label_size = data_size * 4;

		//set
		//kernel 0
		size_t region[3] = { (size_t)nx, (size_t)ny, (size_t)nz };
		kernel_prog->setKernelArgBegin(kernel_index0);
		flvr::Argument arg_szbuf =
			kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE, label_size, nullptr);
		flvr::Argument arg_label =
			kernel_prog->setKernelArgTex3DBuf(CL_MEM_READ_WRITE, lid, sizeof(unsigned int)*nx*ny*nz, region);
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&lenx));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&lenz));
		flvr::Argument arg_mask;
		if (m_use_sel)
			arg_mask = kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, mid);
		//kernel 2
		kernel_prog->setKernelArgBegin(kernel_index2);
		kernel_prog->setKernelArgument(arg_szbuf);
		kernel_prog->setKernelArgument(arg_label);
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&lenx));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&lenz));
		if (m_use_sel)
			kernel_prog->setKernelArgument(arg_mask);
		//kernel 3
		kernel_prog->setKernelArgBegin(kernel_index3);
		kernel_prog->setKernelArgument(arg_szbuf);
		kernel_prog->setKernelArgument(arg_label);
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&psize));
		if (m_use_sel)
			kernel_prog->setKernelArgument(arg_mask);

		//execute
		kernel_prog->executeKernel(kernel_index0, 3, global_size, local_size);
		kernel_prog->executeKernel(kernel_index2, 3, global_size, local_size);
		kernel_prog->executeKernel(kernel_index3, 3, global_size, local_size);

		//read back
		kernel_prog->copyBufTex3D(arg_label, lid,
			sizeof(unsigned int)*nx*ny*nz, region);

		//release buffer
		kernel_prog->releaseAll();

		if (postwork)
			postwork(__FUNCTION__);
	}
}

void ComponentGenerator::Grow(/*bool diffuse, int iter, float tran, float falloff, float sscale, int fixed*/)
{
	if (!CheckBricks())
		return;

	float scale = m_vd->GetScalarScale();

	//create program and kernels
	flvr::KernelProgram* kernel_prog = glbin_vol_kernel_factory.kernel(str_cl_brainbow_3d);
	if (!kernel_prog)
		return;
	int kernel_index0;
	if (m_use_sel)
		kernel_index0 = kernel_prog->createKernel("kernel_1");
	else
		kernel_index0 = kernel_prog->createKernel("kernel_0");

	size_t brick_num = m_vd->GetTexture()->get_brick_num();
	vector<flvr::TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks();
	for (size_t i = 0; i < brick_num; ++i)
	{
		if (prework)
			prework("");

		flvr::TextureBrick* b = (*bricks)[i];
		if (m_use_sel && !b->is_mask_valid())
			continue;
		int bits = b->nb(0) * 8;
		int nx = b->nx();
		int ny = b->ny();
		int nz = b->nz();
		GLint did = m_vd->GetVR()->load_brick(b);
		GLint mid = 0;
		if (m_use_sel)
			mid = m_vd->GetVR()->load_brick_mask(b);
		GLint lid = m_vd->GetVR()->load_brick_label(b);

		//auto iter
		int biter = m_iter;
		if (biter < 0)
			biter = std::max(std::max(nx, ny), nz);

		unsigned int rcnt = 0;
		unsigned int seed = biter > 10 ? biter : 11;
		size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };
		size_t local_size[3] = { 1, 1, 1 };
		float scl_ff = m_diff ? m_falloff : 0.0f;
		float grad_ff = m_diff ? m_falloff : 0.0f;
		float tran = m_thresh * m_tfactor;

		//set
		size_t region[3] = { (size_t)nx, (size_t)ny, (size_t)nz };
		kernel_prog->setKernelArgBegin(kernel_index0);
		kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, did);
		flvr::Argument arg_label =
			kernel_prog->setKernelArgTex3DBuf(CL_MEM_READ_WRITE, lid, sizeof(unsigned int)*nx*ny*nz, region);
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(unsigned int), (void*)(&rcnt));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&seed));
		kernel_prog->setKernelArgConst(sizeof(float), (void*)(&tran));
		kernel_prog->setKernelArgConst(sizeof(float), (void*)(&scl_ff));
		kernel_prog->setKernelArgConst(sizeof(float), (void*)(&grad_ff));
		kernel_prog->setKernelArgConst(sizeof(float), (void*)(&scale));
		kernel_prog->setKernelArgConst(sizeof(int), (void*)(&fixed));
		if (m_use_sel)
			kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, mid);

		//execute
		for (int j = 0; j < biter; ++j)
			kernel_prog->executeKernel(kernel_index0, 3, global_size, local_size);

		//read back
		kernel_prog->copyBufTex3D(arg_label, lid,
			sizeof(unsigned int)*nx*ny*nz, region);

		//release buffer
		kernel_prog->releaseAll();

		if (postwork)
			postwork(__FUNCTION__);
	}

	if (glbin.get_cg_table_enable())
		AddEntry();
}

void ComponentGenerator::DensityField(/*int dsize, int wsize,
	bool diffuse, int iter, float tran, float falloff,
	float density, float varth, float sscale, int fixed*/)
{
	//debug
#ifdef _DEBUG
	unsigned char* val = 0;
	std::ofstream ofs;
#endif

	if (!CheckBricks())
		return;

	float scale = m_vd->GetScalarScale();

	//create program and kernels
	//prog density
	flvr::KernelProgram* kernel_prog_dens = glbin_vol_kernel_factory.kernel(str_cl_density_field_3d);
	if (!kernel_prog_dens)
		return;
	int kernel_dens_index0 = kernel_prog_dens->createKernel("kernel_0");
	int kernel_dens_index1 = kernel_prog_dens->createKernel("kernel_1");
	int kernel_dens_index2 = kernel_prog_dens->createKernel("kernel_2");

	//prog grow
	flvr::KernelProgram* kernel_prog_grow = glbin_vol_kernel_factory.kernel(str_cl_density_grow_3d);
	if (!kernel_prog_grow)
		return;
	int kernel_grow_index0;
	if (m_use_sel)
		kernel_grow_index0 = kernel_prog_grow->createKernel("kernel_1");
	else
		kernel_grow_index0 = kernel_prog_grow->createKernel("kernel_0");

	//processing by brick
	size_t brick_num = m_vd->GetTexture()->get_brick_num();
	vector<flvr::TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks();
	for (size_t i = 0; i < brick_num; ++i)
	{
		if (prework)
			prework("");

		flvr::TextureBrick* b = (*bricks)[i];
		if (m_use_sel && !b->is_mask_valid())
			continue;
		int bits = b->nb(0) * 8;
		int nx = b->nx();
		int ny = b->ny();
		int nz = b->nz();
		GLint did = m_vd->GetVR()->load_brick(b);
		GLint mid = 0;
		if (m_use_sel)
			mid = m_vd->GetVR()->load_brick_mask(b);
		GLint lid = m_vd->GetVR()->load_brick_label(b);

		//divide
		unsigned int gsx, gsy, gsz;//pixel number in group
		int ngx, ngy, ngz;//number of groups
		int dnx, dny, dnz;//adjusted n size
		int dnxy, ngxy;//precalculate
		gsx = m_density_stats_size >= nx ? nx : m_density_stats_size;
		gsy = m_density_stats_size >= ny ? ny : m_density_stats_size;
		gsz = m_density_stats_size >= nz ? nz : m_density_stats_size;
		ngx = nx / gsx + (nx % gsx ? 1 : 0);
		ngy = ny / gsy + (ny % gsy ? 1 : 0);
		ngz = nz / gsz + (nz % gsz ? 1 : 0);
		dnx = gsx * ngx;
		dny = gsy * ngy;
		dnz = gsz * ngz;
		dnxy = dnx * dny;
		ngxy = ngy * ngx;
		//sizes
		size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };
		size_t global_size2[3] = { size_t(dnx), size_t(dny), size_t(dnz) };
		size_t local_size[3] = { 1, 1, 1 };

		//generate density field arg_densf
		//set
		//kernel 0
		kernel_prog_dens->setKernelArgBegin(kernel_dens_index0);
		flvr::Argument arg_img =
			kernel_prog_dens->setKernelArgTex3D(CL_MEM_READ_ONLY, did);
		flvr::Argument arg_densf =
			kernel_prog_dens->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY, sizeof(unsigned char)*dnx*dny*dnz, NULL);
		kernel_prog_dens->setKernelArgConst(sizeof(unsigned int), (void*)(&dnxy));
		kernel_prog_dens->setKernelArgConst(sizeof(unsigned int), (void*)(&dnx));
		kernel_prog_dens->setKernelArgConst(sizeof(int), (void*)(&m_density_window_size));
		kernel_prog_dens->setKernelArgConst(sizeof(float), (void*)(&scale));
		//kernel 1
		kernel_prog_dens->setKernelArgBegin(kernel_dens_index1);
		kernel_prog_dens->setKernelArgument(arg_densf);
		flvr::Argument arg_gavg =
			kernel_prog_dens->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY, sizeof(unsigned char)*ngx*ngy*ngz, NULL);
		flvr::Argument arg_gvar =
			kernel_prog_dens->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY, sizeof(unsigned char)*ngx*ngy*ngz, NULL);
		kernel_prog_dens->setKernelArgConst(sizeof(unsigned int), (void*)(&gsx));
		kernel_prog_dens->setKernelArgConst(sizeof(unsigned int), (void*)(&gsy));
		kernel_prog_dens->setKernelArgConst(sizeof(unsigned int), (void*)(&gsz));
		kernel_prog_dens->setKernelArgConst(sizeof(unsigned int), (void*)(&ngxy));
		kernel_prog_dens->setKernelArgConst(sizeof(unsigned int), (void*)(&ngx));
		kernel_prog_dens->setKernelArgConst(sizeof(unsigned int), (void*)(&dnxy));
		kernel_prog_dens->setKernelArgConst(sizeof(unsigned int), (void*)(&dnx));
		//kernel 2
		kernel_prog_dens->setKernelArgBegin(kernel_dens_index2, 2);
		kernel_prog_dens->setKernelArgConst(sizeof(unsigned int), (void*)(&gsx));
		kernel_prog_dens->setKernelArgConst(sizeof(unsigned int), (void*)(&gsy));
		kernel_prog_dens->setKernelArgConst(sizeof(unsigned int), (void*)(&gsz));
		kernel_prog_dens->setKernelArgConst(sizeof(unsigned int), (void*)(&ngx));
		kernel_prog_dens->setKernelArgConst(sizeof(unsigned int), (void*)(&ngy));
		kernel_prog_dens->setKernelArgConst(sizeof(unsigned int), (void*)(&ngz));
		kernel_prog_dens->setKernelArgConst(sizeof(unsigned int), (void*)(&dnxy));
		kernel_prog_dens->setKernelArgConst(sizeof(unsigned int), (void*)(&dnx));

		//init
		kernel_prog_dens->executeKernel(kernel_dens_index0, 3, global_size2, local_size);
//#ifdef _DEBUG
//		//read back
//		DBMIUINT8 densf(dnx, dny, 1);
//		kernel_prog_dens->readBuffer(arg_densf, densf.data);
//#endif
		//group avg and var
		global_size[0] = size_t(ngx); global_size[1] = size_t(ngy); global_size[2] = size_t(ngz);
		kernel_prog_dens->executeKernel(kernel_dens_index1, 3, global_size, local_size);
//#ifdef _DEBUG
//		//read back
//		DBMIUINT8 gvar(ngx, ngy, 1);
//		kernel_prog_dens->readBuffer(arg_gavg, gvar.data);
//		kernel_prog_dens->readBuffer(arg_gvar, gvar.data);
//#endif
		//compute avg
		global_size[0] = size_t(nx); global_size[1] = size_t(ny); global_size[2] = size_t(nz);
		kernel_prog_dens->setKernelArgBegin(kernel_dens_index2);
		flvr::Argument arg_avg =
			kernel_prog_dens->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY, sizeof(unsigned char)*dnx*dny*dnz, NULL);
		kernel_prog_dens->setKernelArgument(arg_gavg);
		kernel_prog_dens->executeKernel(kernel_dens_index2, 3, global_size, local_size);
		//compute var
		kernel_prog_dens->setKernelArgBegin(kernel_dens_index2);
		flvr::Argument arg_var =
			kernel_prog_dens->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY, sizeof(unsigned char)*dnx*dny*dnz, NULL);
		kernel_prog_dens->setKernelArgument(arg_gvar);
		kernel_prog_dens->executeKernel(kernel_dens_index2, 3, global_size, local_size);

		//release buffer
		kernel_prog_dens->releaseMemObject(arg_gavg);
		kernel_prog_dens->releaseMemObject(arg_gvar);

		//density grow
		unsigned int rcnt = 0;
		unsigned int seed = m_iter > 10 ? m_iter : 11;
		float scl_ff = m_diff ? m_falloff : 0.0f;
		float grad_ff = m_diff ? m_falloff : 0.0f;
		float tran = m_thresh * m_tfactor;
		float density = m_density_thresh;
		float varth = m_varth;
		int fixed = m_grow_fixed;

		//set
		size_t region[3] = { (size_t)nx, (size_t)ny, (size_t)nz };
		kernel_prog_grow->setKernelArgBegin(kernel_grow_index0);
		kernel_prog_grow->setKernelArgument(arg_img);
		flvr::Argument arg_label =
			kernel_prog_grow->setKernelArgTex3DBuf(CL_MEM_READ_WRITE, lid, sizeof(unsigned int)*nx*ny*nz, region);
		kernel_prog_grow->setKernelArgument(arg_densf);
		kernel_prog_grow->setKernelArgument(arg_avg);
		kernel_prog_grow->setKernelArgument(arg_var);
		kernel_prog_grow->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(unsigned int), (void*)(&rcnt));
		kernel_prog_grow->setKernelArgConst(sizeof(unsigned int), (void*)(&seed));
		kernel_prog_grow->setKernelArgConst(sizeof(unsigned int), (void*)(&nx));
		kernel_prog_grow->setKernelArgConst(sizeof(unsigned int), (void*)(&ny));
		kernel_prog_grow->setKernelArgConst(sizeof(unsigned int), (void*)(&nz));
		kernel_prog_grow->setKernelArgConst(sizeof(unsigned int), (void*)(&dnxy));
		kernel_prog_grow->setKernelArgConst(sizeof(unsigned int), (void*)(&dnx));
		kernel_prog_grow->setKernelArgConst(sizeof(float), (void*)(&tran));
		kernel_prog_grow->setKernelArgConst(sizeof(float), (void*)(&scl_ff));
		kernel_prog_grow->setKernelArgConst(sizeof(float), (void*)(&grad_ff));
		kernel_prog_grow->setKernelArgConst(sizeof(float), (void*)(&density));
		kernel_prog_grow->setKernelArgConst(sizeof(float), (void*)(&varth));
		kernel_prog_grow->setKernelArgConst(sizeof(float), (void*)(&scale));
		kernel_prog_grow->setKernelArgConst(sizeof(int), (void*)(&fixed));
		if (m_use_sel)
			kernel_prog_grow->setKernelArgTex3D(CL_MEM_READ_ONLY, mid);

		//execute
		for (int j = 0; j < m_iter; ++j)
			kernel_prog_grow->executeKernel(kernel_grow_index0, 3, global_size, local_size);

		//read back
		kernel_prog_grow->copyBufTex3D(arg_label, lid,
			sizeof(unsigned int)*nx*ny*nz, region);

		//release buffer
		kernel_prog_grow->releaseAll();
		kernel_prog_dens->releaseAll(false);

		if (postwork)
			postwork(__FUNCTION__);
	}

	if (glbin.get_cg_table_enable())
		AddEntry();
}

void ComponentGenerator::DistGrow()
{
	//debug
#ifdef _DEBUG
	unsigned char* val = 0;
	std::ofstream ofs;
#endif

	if (!CheckBricks())
		return;

	float scale = m_vd->GetScalarScale();

	//create program and kernels
	//prog dist
	flvr::KernelProgram* kernel_prog_dist = glbin_vol_kernel_factory.kernel(str_cl_dist_field_2d);
	if (!kernel_prog_dist)
		return;
	int kernel_dist_index0;
	int kernel_dist_index1;
	if (m_use_sel)
	{
		kernel_dist_index0 = kernel_prog_dist->createKernel("kernel_3");
		kernel_dist_index1 = kernel_prog_dist->createKernel("kernel_5");
	}
	else
	{
		kernel_dist_index0 = kernel_prog_dist->createKernel("kernel_0");
		kernel_dist_index1 = kernel_prog_dist->createKernel("kernel_2");
	}

	flvr::KernelProgram* kernel_prog = glbin_vol_kernel_factory.kernel(str_cl_dist_grow_3d);
	if (!kernel_prog)
		return;
	int kernel_index0;
	if (m_use_sel)
		kernel_index0 = kernel_prog->createKernel("kernel_1");
	else
		kernel_index0 = kernel_prog->createKernel("kernel_0");

	size_t brick_num = m_vd->GetTexture()->get_brick_num();
	vector<flvr::TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks();
	for (size_t i = 0; i < brick_num; ++i)
	{
		if (prework)
			prework("");

		flvr::TextureBrick* b = (*bricks)[i];
		if (m_use_sel && !b->is_mask_valid())
			continue;
		int bits = b->nb(0) * 8;
		int nx = b->nx();
		int ny = b->ny();
		int nz = b->nz();
		GLint did = m_vd->GetVR()->load_brick(b);
		GLint mid = 0;
		if (m_use_sel)
			mid = m_vd->GetVR()->load_brick_mask(b);
		GLint lid = m_vd->GetVR()->load_brick_label(b);

		size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };
		size_t local_size[3] = { 1, 1, 1 };

		float dist_thresh = m_dist_thresh;
		//generate distance field arg_distf
		unsigned char ini = 1;
		//set
		//kernel 0
		kernel_prog_dist->setKernelArgBegin(kernel_dist_index0);
		flvr::Argument arg_img =
			kernel_prog_dist->setKernelArgTex3D(CL_MEM_READ_ONLY, did);
		flvr::Argument arg_distf =
			kernel_prog_dist->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY, sizeof(unsigned char)*nx*ny*nz, NULL);
		kernel_prog_dist->setKernelArgConst(sizeof(unsigned int), (void*)(&nx));
		kernel_prog_dist->setKernelArgConst(sizeof(unsigned int), (void*)(&ny));
		kernel_prog_dist->setKernelArgConst(sizeof(unsigned int), (void*)(&nz));
		kernel_prog_dist->setKernelArgConst(sizeof(int), (void*)(&m_dist_filter_size));
		kernel_prog_dist->setKernelArgConst(sizeof(float), (void*)(&dist_thresh));
		kernel_prog_dist->setKernelArgConst(sizeof(float), (void*)(&scale));
		kernel_prog_dist->setKernelArgConst(sizeof(unsigned char), (void*)(&ini));
		flvr::Argument arg_mask;
		if (m_use_sel)
			arg_mask = kernel_prog_dist->setKernelArgTex3D(CL_MEM_READ_ONLY, mid);
		//kernel 1
		kernel_prog_dist->setKernelArgBegin(kernel_dist_index1);
		kernel_prog_dist->setKernelArgument(arg_distf);
		kernel_prog_dist->setKernelArgConst(sizeof(unsigned int), (void*)(&nx));
		kernel_prog_dist->setKernelArgConst(sizeof(unsigned int), (void*)(&ny));
		kernel_prog_dist->setKernelArgConst(sizeof(unsigned int), (void*)(&nz));
		kernel_prog_dist->setKernelArgConst(sizeof(unsigned char), (void*)(&ini));
		if (m_use_sel)
		{
			kernel_prog_dist->setKernelArgBegin(kernel_dist_index1, 7);
			kernel_prog_dist->setKernelArgument(arg_mask);
		}
		//init
		kernel_prog_dist->executeKernel(kernel_dist_index0, 3, global_size, local_size);
		unsigned char nn, re;
		for (int j = 0; j < m_max_dist; ++j)
		{
			nn = j == 0 ? 0 : j + ini;
			re = j + ini + 1;
			kernel_prog_dist->setKernelArgBegin(kernel_dist_index1, 5);
			kernel_prog_dist->setKernelArgConst(sizeof(unsigned char), (void*)(&nn));
			kernel_prog_dist->setKernelArgConst(sizeof(unsigned char), (void*)(&re));
			kernel_prog_dist->executeKernel(kernel_dist_index1, 3, global_size, local_size);
		}

		//grow
		unsigned int rcnt = 0;
		unsigned int seed = m_iter > 10 ? m_iter : 11;
		float scl_ff = m_diff ? m_falloff : 0.0f;
		float grad_ff = m_diff ? m_falloff : 0.0f;
		float distscl = 5.0f / m_max_dist;
		float tran = m_thresh * m_tfactor;
		float dist_strength = m_dist_strength;
		int fixed = m_grow_fixed;
		//set
		size_t region[3] = { (size_t)nx, (size_t)ny, (size_t)nz };
		kernel_prog->setKernelArgBegin(kernel_index0);
		kernel_prog->setKernelArgument(arg_img);
		flvr::Argument arg_label =
			kernel_prog->setKernelArgTex3DBuf(CL_MEM_READ_WRITE, lid, sizeof(unsigned int)*nx*ny*nz, region);
		kernel_prog->setKernelArgument(arg_distf);
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(unsigned int), (void*)(&rcnt));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&seed));
		kernel_prog->setKernelArgConst(sizeof(float), (void*)(&tran));
		kernel_prog->setKernelArgConst(sizeof(float), (void*)(&scl_ff));
		kernel_prog->setKernelArgConst(sizeof(float), (void*)(&grad_ff));
		kernel_prog->setKernelArgConst(sizeof(float), (void*)(&scale));
		kernel_prog->setKernelArgConst(sizeof(float), (void*)(&distscl));
		kernel_prog->setKernelArgConst(sizeof(float), (void*)(&dist_strength));
		kernel_prog->setKernelArgConst(sizeof(int), (void*)(&fixed));
		if (m_use_sel)
			kernel_prog->setKernelArgument(arg_mask);

		//execute
		for (int j = 0; j < m_iter; ++j)
			kernel_prog->executeKernel(kernel_index0, 3, global_size, local_size);

		//read back
		kernel_prog->copyBufTex3D(arg_label, lid,
			sizeof(unsigned int)*nx*ny*nz, region);

		//release buffer
		kernel_prog->releaseAll();
		kernel_prog_dist->releaseAll(false);

		if (postwork)
			postwork(__FUNCTION__);
	}

	if (glbin.get_cg_table_enable())
		AddEntry();
}

void ComponentGenerator::DistDensityField(
	/*bool diffuse, int iter, float tran, float falloff,
	int dsize1, int max_dist, float dist_thresh, float dist_strength,
	int dsize2, int wsize, float density, float varth, float sscale, int fixed*/)
{
	//debug
#ifdef _DEBUG
	unsigned char* val = 0;
	std::ofstream ofs;
#endif

	if (!CheckBricks())
		return;

	float scale = m_vd->GetScalarScale();

	//create program and kernels
	//prog dist
	flvr::KernelProgram* kernel_prog_dist = glbin_vol_kernel_factory.kernel(str_cl_dist_field_2d);
	if (!kernel_prog_dist)
		return;
	int kernel_dist_index0;
	int kernel_dist_index1;
	if (m_use_sel)
	{
		kernel_dist_index0 = kernel_prog_dist->createKernel("kernel_3");
		kernel_dist_index1 = kernel_prog_dist->createKernel("kernel_5");
	}
	else
	{
		kernel_dist_index0 = kernel_prog_dist->createKernel("kernel_0");
		kernel_dist_index1 = kernel_prog_dist->createKernel("kernel_2");
	}
	//prog density
	flvr::KernelProgram* kernel_prog_dens = glbin_vol_kernel_factory.kernel(str_cl_distdens_field_3d);
	if (!kernel_prog_dens)
		return;
	int kernel_dens_index0 = kernel_prog_dens->createKernel("kernel_0");
	int kernel_dens_index1 = kernel_prog_dens->createKernel("kernel_1");
	int kernel_dens_index2 = kernel_prog_dens->createKernel("kernel_2");

	//prog grow
	flvr::KernelProgram* kernel_prog_grow = glbin_vol_kernel_factory.kernel(str_cl_density_grow_3d);
	if (!kernel_prog_grow)
		return;
	int kernel_grow_index0;
	if (m_use_sel)
		kernel_grow_index0 = kernel_prog_grow->createKernel("kernel_1");
	else
		kernel_grow_index0 = kernel_prog_grow->createKernel("kernel_0");

	//processing by brick
	size_t brick_num = m_vd->GetTexture()->get_brick_num();
	vector<flvr::TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks();
	for (size_t i = 0; i < brick_num; ++i)
	{
		if (prework)
			prework("");

		flvr::TextureBrick* b = (*bricks)[i];
		if (m_use_sel && !b->is_mask_valid())
			continue;
		int bits = b->nb(0) * 8;
		int nx = b->nx();
		int ny = b->ny();
		int nz = b->nz();
		GLint did = m_vd->GetVR()->load_brick(b);
		GLint mid = 0;
		if (m_use_sel)
			mid = m_vd->GetVR()->load_brick_mask(b);
		GLint lid = m_vd->GetVR()->load_brick_label(b);

		//divide
		unsigned int gsx, gsy, gsz;//pixel number in group
		int ngx, ngy, ngz;//number of groups
		int dnx, dny, dnz;//adjusted n size
		int nxy, dnxy, ngxy;//precalculate
		gsx = m_density_stats_size >= nx ? nx : m_density_stats_size;
		gsy = m_density_stats_size >= ny ? ny : m_density_stats_size;
		gsz = m_density_stats_size >= nz ? nz : m_density_stats_size;
		ngx = nx / gsx + (nx % gsx ? 1 : 0);
		ngy = ny / gsy + (ny % gsy ? 1 : 0);
		ngz = nz / gsz + (nz % gsz ? 1 : 0);
		dnx = gsx * ngx;
		dny = gsy * ngy;
		dnz = gsz * ngz;
		nxy = nx * ny;
		dnxy = dnx * dny;
		ngxy = ngy * ngx;
		//sizes
		size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };
		size_t global_size2[3] = { size_t(dnx), size_t(dny), size_t(dnz) };
		size_t local_size[3] = { 1, 1, 1 };

		//generate distance field arg_distf
		unsigned char ini = 1;
		float dist_thresh = m_dist_thresh;
		//set
		//kernel 0
		kernel_prog_dist->setKernelArgBegin(kernel_dist_index0);
		flvr::Argument arg_img =
			kernel_prog_dist->setKernelArgTex3D(CL_MEM_READ_ONLY, did);
		flvr::Argument arg_distf = kernel_prog_dist->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY, sizeof(unsigned char)*nx*ny*nz, NULL);
		kernel_prog_dist->setKernelArgConst(sizeof(unsigned int), (void*)(&nx));
		kernel_prog_dist->setKernelArgConst(sizeof(unsigned int), (void*)(&ny));
		kernel_prog_dist->setKernelArgConst(sizeof(unsigned int), (void*)(&nz));
		kernel_prog_dist->setKernelArgConst(sizeof(int), (void*)(&m_dist_filter_size));
		kernel_prog_dist->setKernelArgConst(sizeof(float), (void*)(&dist_thresh));
		kernel_prog_dist->setKernelArgConst(sizeof(float), (void*)(&scale));
		kernel_prog_dist->setKernelArgConst(sizeof(unsigned char), (void*)(&ini));
		flvr::Argument arg_mask;
		if (m_use_sel)
			arg_mask = kernel_prog_dist->setKernelArgTex3D(CL_MEM_READ_ONLY, mid);
		//kernel 1
		kernel_prog_dist->setKernelArgBegin(kernel_dist_index1);
		kernel_prog_dist->setKernelArgument(arg_distf);
		kernel_prog_dist->setKernelArgConst(sizeof(unsigned int), (void*)(&nx));
		kernel_prog_dist->setKernelArgConst(sizeof(unsigned int), (void*)(&ny));
		kernel_prog_dist->setKernelArgConst(sizeof(unsigned int), (void*)(&nz));
		kernel_prog_dist->setKernelArgConst(sizeof(unsigned char), (void*)(&ini));
		if (m_use_sel)
		{
			kernel_prog_dist->setKernelArgBegin(kernel_dist_index1, 7);
			kernel_prog_dist->setKernelArgument(arg_mask);
		}
		//init
		kernel_prog_dist->executeKernel(kernel_dist_index0, 3, global_size, local_size);
		unsigned char nn, re;
		for (int j = 0; j < m_max_dist; ++j)
		{
			nn = j == 0 ? 0 : j + ini;
			re = j + ini + 1;
			kernel_prog_dist->setKernelArgBegin(kernel_dist_index1, 5);
			kernel_prog_dist->setKernelArgConst(sizeof(unsigned char), (void*)(&nn));
			kernel_prog_dist->setKernelArgConst(sizeof(unsigned char), (void*)(&re));
			kernel_prog_dist->executeKernel(kernel_dist_index1, 3, global_size, local_size);
		}
//#ifdef _DEBUG
//		//read back
//		DBMIUINT8 distf(nx, ny, 1);
//		kernel_prog_dist->readBuffer(arg_distf, distf.data);
//#endif

		//generate density field arg_densf
		//set
		//kernel 0
		float distscl = 5.0f / m_max_dist;
		float dist_strength = m_dist_strength;
		kernel_prog_dens->setKernelArgBegin(kernel_dens_index0);
		kernel_prog_dens->setKernelArgument(arg_img);
		kernel_prog_dens->setKernelArgument(arg_distf);
		flvr::Argument arg_densf =
			kernel_prog_dens->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY, sizeof(unsigned char)*dnx*dny*dnz, NULL);
		kernel_prog_dens->setKernelArgConst(sizeof(unsigned int), (void*)(&nxy));
		kernel_prog_dens->setKernelArgConst(sizeof(unsigned int), (void*)(&nx));
		kernel_prog_dens->setKernelArgConst(sizeof(unsigned int), (void*)(&ny));
		kernel_prog_dens->setKernelArgConst(sizeof(unsigned int), (void*)(&nz));
		kernel_prog_dens->setKernelArgConst(sizeof(unsigned int), (void*)(&dnxy));
		kernel_prog_dens->setKernelArgConst(sizeof(unsigned int), (void*)(&dnx));
		kernel_prog_dens->setKernelArgConst(sizeof(int), (void*)(&m_density_window_size));
		kernel_prog_dens->setKernelArgConst(sizeof(float), (void*)(&scale));
		kernel_prog_dens->setKernelArgConst(sizeof(float), (void*)(&distscl));
		kernel_prog_dens->setKernelArgConst(sizeof(float), (void*)(&dist_strength));
		//kernel 1
		kernel_prog_dens->setKernelArgBegin(kernel_dens_index1);
		kernel_prog_dens->setKernelArgument(arg_densf);
		flvr::Argument arg_gavg =
			kernel_prog_dens->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY, sizeof(unsigned char)*ngx*ngy*ngz, NULL);
		flvr::Argument arg_gvar =
			kernel_prog_dens->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY, sizeof(unsigned char)*ngx*ngy*ngz, NULL);
		kernel_prog_dens->setKernelArgConst(sizeof(unsigned int), (void*)(&gsx));
		kernel_prog_dens->setKernelArgConst(sizeof(unsigned int), (void*)(&gsy));
		kernel_prog_dens->setKernelArgConst(sizeof(unsigned int), (void*)(&gsz));
		kernel_prog_dens->setKernelArgConst(sizeof(unsigned int), (void*)(&ngxy));
		kernel_prog_dens->setKernelArgConst(sizeof(unsigned int), (void*)(&ngx));
		kernel_prog_dens->setKernelArgConst(sizeof(unsigned int), (void*)(&dnxy));
		kernel_prog_dens->setKernelArgConst(sizeof(unsigned int), (void*)(&dnx));
		//kernel 2
		kernel_prog_dens->setKernelArgBegin(kernel_dens_index2, 2);
		kernel_prog_dens->setKernelArgConst(sizeof(unsigned int), (void*)(&gsx));
		kernel_prog_dens->setKernelArgConst(sizeof(unsigned int), (void*)(&gsy));
		kernel_prog_dens->setKernelArgConst(sizeof(unsigned int), (void*)(&gsz));
		kernel_prog_dens->setKernelArgConst(sizeof(unsigned int), (void*)(&ngx));
		kernel_prog_dens->setKernelArgConst(sizeof(unsigned int), (void*)(&ngy));
		kernel_prog_dens->setKernelArgConst(sizeof(unsigned int), (void*)(&ngz));
		kernel_prog_dens->setKernelArgConst(sizeof(unsigned int), (void*)(&dnxy));
		kernel_prog_dens->setKernelArgConst(sizeof(unsigned int), (void*)(&dnx));

		//init
		kernel_prog_dens->executeKernel(kernel_dens_index0, 3, global_size2, local_size);
//#ifdef _DEBUG
//		//read back
//		DBMIUINT8 densf(dnx, dny, 1);
//		kernel_prog_dens->readBuffer(arg_densf, densf.data);
//#endif
		//group avg and var
		global_size[0] = size_t(ngx); global_size[1] = size_t(ngy); global_size[2] = size_t(ngz);
		kernel_prog_dens->executeKernel(kernel_dens_index1, 3, global_size, local_size);
		//compute avg
		global_size[0] = size_t(nx); global_size[1] = size_t(ny); global_size[2] = size_t(nz);
		kernel_prog_dens->setKernelArgBegin(kernel_dens_index2);
		flvr::Argument arg_avg =
			kernel_prog_dens->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY, sizeof(unsigned char)*dnx*dny*dnz, NULL);
		kernel_prog_dens->setKernelArgument(arg_gavg);
		kernel_prog_dens->executeKernel(kernel_dens_index2, 3, global_size, local_size);
		//compute var
		kernel_prog_dens->setKernelArgBegin(kernel_dens_index2);
		flvr::Argument arg_var =
			kernel_prog_dens->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY, sizeof(unsigned char)*dnx*dny*dnz, NULL);
		kernel_prog_dens->setKernelArgument(arg_gvar);
		kernel_prog_dens->executeKernel(kernel_dens_index2, 3, global_size, local_size);

		//release buffer
		kernel_prog_dens->releaseMemObject(arg_gavg);
		kernel_prog_dens->releaseMemObject(arg_gvar);
		kernel_prog_dens->releaseMemObject(arg_distf);

		//distance + density grow
		unsigned int rcnt = 0;
		unsigned int seed = m_iter > 10 ? m_iter : 11;
		float scl_ff = m_diff ? m_falloff : 0.0f;
		float grad_ff = m_diff ? m_falloff : 0.0f;
		float tran = m_thresh * m_tfactor;
		int fixed = m_grow_fixed;
		float density = m_density_thresh;
		float varth = m_varth;

		//set
		size_t region[3] = { (size_t)nx, (size_t)ny, (size_t)nz };
		kernel_prog_grow->setKernelArgBegin(kernel_grow_index0);
		kernel_prog_grow->setKernelArgument(arg_img);
		flvr::Argument arg_label =
			kernel_prog_grow->setKernelArgTex3DBuf(CL_MEM_READ_WRITE, lid, sizeof(unsigned int)*nx*ny*nz, region);
		kernel_prog_grow->setKernelArgument(arg_densf);
		kernel_prog_grow->setKernelArgument(arg_avg);
		kernel_prog_grow->setKernelArgument(arg_var);
		kernel_prog_grow->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(unsigned int), (void*)(&rcnt));
		kernel_prog_grow->setKernelArgConst(sizeof(unsigned int), (void*)(&seed));
		kernel_prog_grow->setKernelArgConst(sizeof(unsigned int), (void*)(&nx));
		kernel_prog_grow->setKernelArgConst(sizeof(unsigned int), (void*)(&ny));
		kernel_prog_grow->setKernelArgConst(sizeof(unsigned int), (void*)(&nz));
		kernel_prog_grow->setKernelArgConst(sizeof(unsigned int), (void*)(&dnxy));
		kernel_prog_grow->setKernelArgConst(sizeof(unsigned int), (void*)(&dnx));
		kernel_prog_grow->setKernelArgConst(sizeof(float), (void*)(&tran));
		kernel_prog_grow->setKernelArgConst(sizeof(float), (void*)(&scl_ff));
		kernel_prog_grow->setKernelArgConst(sizeof(float), (void*)(&grad_ff));
		kernel_prog_grow->setKernelArgConst(sizeof(float), (void*)(&density));
		kernel_prog_grow->setKernelArgConst(sizeof(float), (void*)(&varth));
		kernel_prog_grow->setKernelArgConst(sizeof(float), (void*)(&scale));
		kernel_prog_grow->setKernelArgConst(sizeof(int), (void*)(&fixed));
		if (m_use_sel)
			kernel_prog_grow->setKernelArgTex3D(CL_MEM_READ_ONLY, mid);

		//execute
		for (int j = 0; j < m_iter; ++j)
			kernel_prog_grow->executeKernel(kernel_grow_index0, 3, global_size, local_size);

		//read back
		kernel_prog_grow->copyBufTex3D(arg_label, lid,
			sizeof(unsigned int)*nx*ny*nz, region);

		//release buffer
		kernel_prog_grow->releaseAll();
		kernel_prog_dist->releaseAll(false);
		kernel_prog_dens->releaseAll(false);

		if (postwork)
			postwork(__FUNCTION__);
	}

	if (glbin.get_cg_table_enable())
		AddEntry();
}

void ComponentGenerator::Cleanup(/*int iter, unsigned int size_lm*/)
{
	if (m_clean_iter <= 0)
		return;
	if (!CheckBricks())
		return;

	//create program and kernels
	flvr::KernelProgram* kernel_prog = glbin_vol_kernel_factory.kernel(str_cl_cleanup_3d);
	if (!kernel_prog)
		return;
	int kernel_index0;
	int kernel_index1;
	int kernel_index2;
	if (m_use_sel)
	{
		kernel_index0 = kernel_prog->createKernel("kernel_3");
		kernel_index1 = kernel_prog->createKernel("kernel_4");
		kernel_index2 = kernel_prog->createKernel("kernel_5");
	}
	else
	{
		kernel_index0 = kernel_prog->createKernel("kernel_0");
		kernel_index1 = kernel_prog->createKernel("kernel_1");
		kernel_index2 = kernel_prog->createKernel("kernel_2");
	}

	size_t brick_num = m_vd->GetTexture()->get_brick_num();
	vector<flvr::TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks();
	for (size_t i = 0; i < brick_num; ++i)
	{
		if (prework)
			prework("");

		flvr::TextureBrick* b = (*bricks)[i];
		if (m_use_sel && !b->is_mask_valid())
			continue;
		int bits = b->nb(0) * 8;
		int nx = b->nx();
		int ny = b->ny();
		int nz = b->nz();
		GLint did = m_vd->GetVR()->load_brick(b);
		GLint mid = 0;
		if (m_use_sel)
			mid = m_vd->GetVR()->load_brick_mask(b);
		GLint lid = m_vd->GetVR()->load_brick_label(b);

		size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };
		size_t local_size[3] = { 1, 1, 1 };

		//bit length
		unsigned int lenx = 0;
		unsigned int r = std::max(nx, ny);
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

		unsigned long long data_size =
			(unsigned long long)nx *
			(unsigned long long)ny *
			(unsigned long long)nz;
		unsigned long long label_size = data_size * 4;

		//set
		//kernel 0
		size_t region[3] = { (size_t)nx, (size_t)ny, (size_t)nz };
		kernel_prog->setKernelArgBegin(kernel_index0);
		flvr::Argument arg_szbuf =
			kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE, label_size, nullptr);
		flvr::Argument arg_label =
			kernel_prog->setKernelArgTex3DBuf(CL_MEM_READ_WRITE, lid, sizeof(unsigned int)*nx*ny*nz, region);
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&lenx));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&lenz));
		flvr::Argument arg_mask;
		if (m_use_sel)
			arg_mask = kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, mid);
		//kernel 1
		kernel_prog->setKernelArgBegin(kernel_index1);
		kernel_prog->setKernelArgument(arg_szbuf);
		kernel_prog->setKernelArgument(arg_label);
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&lenx));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&lenz));
		if (m_use_sel)
			kernel_prog->setKernelArgument(arg_mask);
		//kernel 2
		unsigned int size_lm = m_clean_size_vl;
		kernel_prog->setKernelArgBegin(kernel_index2);
		kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, did);
		kernel_prog->setKernelArgument(arg_szbuf);
		kernel_prog->setKernelArgument(arg_label);
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&size_lm));
		if (m_use_sel)
			kernel_prog->setKernelArgument(arg_mask);

		//execute
		for (int j = 0; j < m_iter; ++j)
		{
			kernel_prog->executeKernel(kernel_index0, 3, global_size, local_size);
			kernel_prog->executeKernel(kernel_index1, 3, global_size, local_size);
			kernel_prog->executeKernel(kernel_index2, 3, global_size, local_size);
		}

		//read back
		kernel_prog->copyBufTex3D(arg_label, lid,
			sizeof(unsigned int)*nx*ny*nz, region);

		//release buffer
		kernel_prog->releaseAll();

		if (postwork)
			postwork(__FUNCTION__);
	}

	if (glbin.get_cg_table_enable())
		AddCleanEntry();
}

void ComponentGenerator::ClearBorders()
{
	if (!CheckBricks())
		return;

	//create program and kernels
	flvr::KernelProgram* kernel_prog = glbin_vol_kernel_factory.kernel(str_cl_clear_borders_3d);
	if (!kernel_prog)
		return;
	int kernel_index;
	if (m_use_sel)
		kernel_index = kernel_prog->createKernel("kernel_1");
	else
		kernel_index = kernel_prog->createKernel("kernel_0");

	size_t brick_num = m_vd->GetTexture()->get_brick_num();
	vector<flvr::TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks();
	for (size_t i = 0; i < brick_num; ++i)
	{
		if (prework)
			prework("");

		flvr::TextureBrick* b = (*bricks)[i];
		if (m_use_sel && !b->is_mask_valid())
			continue;
		int bits = b->nb(0) * 8;
		int nx = b->nx();
		int ny = b->ny();
		int nz = b->nz();
		GLint mid = 0;
		if (m_use_sel)
			mid = m_vd->GetVR()->load_brick_mask(b);
		GLint lid = m_vd->GetVR()->load_brick_label(b);

		size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };
		size_t local_size[3] = { 1, 1, 1 };

		size_t region[3] = { (size_t)nx, (size_t)ny, (size_t)nz };
		kernel_prog->setKernelArgBegin(kernel_index);
		flvr::Argument arg_label =
			kernel_prog->setKernelArgTex3DBuf(CL_MEM_READ_WRITE, lid, sizeof(unsigned int)*nx*ny*nz, region);
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nz));
		if (m_use_sel)
			kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, mid);

		//execute
		kernel_prog->executeKernel(kernel_index, 3, global_size, local_size);

		//read back
		kernel_prog->copyBufTex3D(arg_label, lid,
			sizeof(unsigned int)*nx*ny*nz, region);

		//release buffer
		kernel_prog->releaseAll();

		if (postwork)
			postwork(__FUNCTION__);
	}
}

void ComponentGenerator::FillBorders()
{
	if (!CheckBricks())
		return;

	//create program and kernels
	flvr::KernelProgram* kernel_prog = glbin_vol_kernel_factory.kernel(str_cl_fill_borders_3d);
	if (!kernel_prog)
		return;
	int kernel_index;
	if (m_use_sel)
		kernel_index = kernel_prog->createKernel("kernel_1");
	else
		kernel_index = kernel_prog->createKernel("kernel_0");

	size_t brick_num = m_vd->GetTexture()->get_brick_num();
	vector<flvr::TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks_id();
	for (size_t i = 0; i < brick_num; ++i)
	{
		if (prework)
			prework("");

		flvr::TextureBrick* b = (*bricks)[i];
		if (m_use_sel && !b->is_mask_valid())
			continue;
		int bits = b->nb(0) * 8;
		int nx = b->nx();
		int ny = b->ny();
		int nz = b->nz();
		GLint did = m_vd->GetVR()->load_brick(b);
		GLint mid = 0;
		if (m_use_sel)
			mid = m_vd->GetVR()->load_brick_mask(b);
		GLint lid = m_vd->GetVR()->load_brick_label(b);

		size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };
		size_t local_size[3] = { 1, 1, 1 };

		float tol = m_fill_border;
		//set
		size_t region[3] = { (size_t)nx, (size_t)ny, (size_t)nz };
		kernel_prog->setKernelArgBegin(kernel_index);
		kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, did);
		flvr::Argument arg_label =
			kernel_prog->setKernelArgTex3DBuf(CL_MEM_READ_WRITE, lid, sizeof(unsigned int)*nx*ny*nz, region);
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setKernelArgConst(sizeof(float), (void*)(&tol));
		if (m_use_sel)
			kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, mid);

		//execute
		kernel_prog->executeKernel(kernel_index, 3, global_size, local_size);

		//read back
		kernel_prog->copyBufTex3D(arg_label, lid,
			sizeof(unsigned int)*nx*ny*nz, region);

		//release buffer
		kernel_prog->releaseAll();

		if (postwork)
			postwork(__FUNCTION__);
	}
}

void ComponentGenerator::AddEntry()
{
	//parameters
	flrd::EntryParams& ep = glbin.get_cg_entry();
	ep.setParam("iter", m_iter);
	ep.setParam("thresh", m_thresh);
	ep.setParam("use_dist_field", m_use_dist_field);
	ep.setParam("dist_strength", m_dist_strength);
	ep.setParam("dist_filter_size", m_dist_filter_size);
	ep.setParam("max_dist", m_max_dist);
	ep.setParam("dist_thresh", m_dist_thresh);
	ep.setParam("diff", m_diff);
	ep.setParam("falloff", m_falloff);
	ep.setParam("density", m_density);
	ep.setParam("density_thresh", m_density_thresh);
	ep.setParam("varth", m_varth);
	ep.setParam("density_window_size", m_density_window_size);
	ep.setParam("density_stats_size", m_density_stats_size);
	ep.setParam("grow_fixed", m_grow_fixed);
}

void ComponentGenerator::AddCleanEntry()
{
	flrd::EntryParams& ep = glbin.get_cg_entry();
	ep.setParam("cleanb", 1.0f);
	ep.setParam("clean_iter", m_clean_iter);
	ep.setParam("clean_size_vl", m_clean_size_vl);
}

void ComponentGenerator::GenerateDB()
{
	if (!CheckBricks())
		return;

	flrd::TableHistParams& table = glbin.get_cg_table();
	//check table size
	unsigned int rec = table.getRecSize();
	unsigned int bin = EntryHist::m_bins;
	unsigned int par = glbin.get_params("comp_gen")->size();
	if (!(rec && bin && par))
		return;

	//iterration maximum from db
	int iter = table.getParamIter();
	int max_dist = table.getParamMxdist();//max iteration for distance field
	bool cleanb = table.getParamCleanb() > 0.0f;
	int clean_iter = table.getParamCleanIter();

	//histogram window size
	int whistxy = 20;//histogram size
	int whistz = 3;
	float hsize = table.getHistPopl();
	int nx, ny, nz;
	m_vd->GetResolution(nx, ny, nz);
	float w;
	if (nz < 5)
	{
		w = std::sqrt(hsize);
		whistxy = (int)std::ceil(w);
		whistz = 1;
	}
	else
	{
		w = std::pow((double)hsize / (nx / nz) / (ny / nz), double(1) / 3);
		whistz = (int)std::ceil(w);
		whistxy = (int)std::ceil(w * nx / nz);
	}
	//intensity scale
	float sscale = float(m_vd->GetScalarScale());

	//prog
	flvr::KernelProgram* kernel_prog = glbin_vol_kernel_factory.kernel(str_cl_comp_gen_db);
	if (!kernel_prog)
		return;
	int kernel_index0 = kernel_prog->createKernel("kernel_0");//generate lut
	int kernel_index1 = kernel_prog->createKernel("kernel_1");//init dist field
	int kernel_index2 = kernel_prog->createKernel("kernel_2");//generate dist field
	int kernel_index3 = kernel_prog->createKernel("kernel_3");//mix den and dist fields
	int kernel_index4 = kernel_prog->createKernel("kernel_4");//generate statistics
	int kernel_index5 = kernel_prog->createKernel("kernel_5");//grow
	int kernel_index6 = kernel_prog->createKernel("kernel_6");//count and store size
	int kernel_index7 = kernel_prog->createKernel("kernel_7");//set size to same comp
	int kernel_index8 = kernel_prog->createKernel("kernel_8");//size based grow

	//processing by brick
	size_t brick_num = m_vd->GetTexture()->get_brick_num();
	vector<flvr::TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks();
	for (size_t i = 0; i < brick_num; ++i)
	{
		if (prework)
			prework("");

		flvr::TextureBrick* b = (*bricks)[i];
		int bits = b->nb(0) * 8;
		nx = b->nx();
		ny = b->ny();
		nz = b->nz();
		GLint did = m_vd->GetVR()->load_brick(b);
		GLint lid = m_vd->GetVR()->load_brick_label(b);

		//precalculate
		unsigned int nxy, nxyz;
		nxy = nx * ny;
		nxyz = nxy * nz;
		//sizes
		size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };
		size_t local_size[3] = { 1, 1, 1 };

		//compute local histogram and generate lut
		kernel_prog->setKernelArgBegin(kernel_index0);
		flvr::Argument arg_img =
			kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, did);
		//rechist
		size_t fsize = bin * rec;
		float* rechist = new float[fsize]();
		table.getRecInput(rechist);
		//debug
//#ifdef _DEBUG
//		DBMIFLOAT32 histmi2;
//		histmi2.nx = bin; histmi2.ny = rec; histmi2.nc = 1; histmi2.nt = bin * 4; histmi2.data = rechist;
//#endif
		flvr::Argument arg_rechist =
			kernel_prog->setKernelArgBuf(CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float)*fsize, (void*)(rechist));
		fsize = nx * ny * nz;
		cl_uchar* lut = new cl_uchar[fsize]();
		flvr::Argument arg_lut =
			kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_uchar)*fsize, (void*)(lut));
		//local histogram
		kernel_prog->setKernelArgLocal(sizeof(float)*bin);
		cl_int3 cl_histxyz = { cl_int(whistxy), cl_int(whistxy), cl_int(whistz) };
		kernel_prog->setKernelArgConst(sizeof(cl_int3), (void*)(&cl_histxyz));
		cl_int3 cl_nxyz = { cl_int(nx), cl_int(ny), cl_int(nz) };
		kernel_prog->setKernelArgConst(sizeof(cl_int3), (void*)(&cl_nxyz));
		float minv = 0;
		float maxv = 1;
		if (bits > 8) maxv = float(1.0 / m_vd->GetScalarScale());
		kernel_prog->setKernelArgConst(sizeof(float), (void*)(&minv));
		kernel_prog->setKernelArgConst(sizeof(float), (void*)(&maxv));
		cl_uchar cl_bin = (cl_uchar)(bin);
		kernel_prog->setKernelArgConst(sizeof(cl_uchar), (void*)(&cl_bin));
		cl_uchar cl_rec = (cl_uchar)(rec);
		kernel_prog->setKernelArgConst(sizeof(cl_uchar), (void*)(&cl_rec));

		//execute
		kernel_prog->executeKernel(kernel_index0, 3, global_size, local_size);

//#ifdef _DEBUG
//		//read back
//		kernel_prog->readBuffer(arg_lut, lut);
//		DBMIUINT8 mi;
//		mi.nx = nx; mi.ny = ny; mi.nc = 1; mi.nt = nx; mi.data = lut;
//#endif
		//release
		kernel_prog->releaseMemObject(arg_rechist);
		delete[] rechist;
		delete[] lut;

		//generate distance field arg_distf
		unsigned char ini = 1;
		//set
		//kernel 1
		kernel_prog->setKernelArgBegin(kernel_index1);
		kernel_prog->setKernelArgument(arg_img);
		kernel_prog->setKernelArgument(arg_lut);
		//params
		fsize = par * rec;
		float* params = new float[fsize]();
		table.getRecOutput(params);
		flvr::Argument arg_params =
			kernel_prog->setKernelArgBuf(CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			sizeof(float)*fsize, (void*)(params));
		flvr::Argument arg_distf =
			kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY,
				sizeof(unsigned char)*nxyz, NULL);
		kernel_prog->setKernelArgConst(sizeof(float), (void*)(&sscale));
		kernel_prog->setKernelArgConst(sizeof(unsigned char), (void*)(&ini));
		kernel_prog->setKernelArgConst(sizeof(cl_int3), (void*)(&cl_nxyz));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nxy));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&par));
		//kernel 2
		kernel_prog->setKernelArgBegin(kernel_index2);
		kernel_prog->setKernelArgument(arg_distf);
		kernel_prog->setKernelArgConst(sizeof(unsigned char), (void*)(&ini));
		kernel_prog->setKernelArgConst(sizeof(cl_int3), (void*)(&cl_nxyz));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nxy));
		//init
		kernel_prog->executeKernel(kernel_index1, 3, global_size, local_size);
		unsigned char nn, re;
		for (int j = 0; j < max_dist; ++j)
		{
			nn = j == 0 ? 0 : j + ini;
			re = j + ini + 1;
			kernel_prog->setKernelArgBegin(kernel_index2, 4);
			kernel_prog->setKernelArgConst(sizeof(unsigned char), (void*)(&nn));
			kernel_prog->setKernelArgConst(sizeof(unsigned char), (void*)(&re));
			kernel_prog->executeKernel(kernel_index2, 3, global_size, local_size);
		}

		//generate density field arg_densf
		//set
		//kernel 3
		kernel_prog->setKernelArgBegin(kernel_index3);
		kernel_prog->setKernelArgument(arg_img);
		kernel_prog->setKernelArgument(arg_distf);
		flvr::Argument arg_densf =
			kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY,
				sizeof(unsigned char)*nxyz, NULL);
		kernel_prog->setKernelArgument(arg_lut);
		kernel_prog->setKernelArgument(arg_params);
		kernel_prog->setKernelArgConst(sizeof(float), (void*)(&sscale));
		kernel_prog->setKernelArgConst(sizeof(cl_int3), (void*)(&cl_nxyz));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nxy));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&par));
		//kernel 4
		kernel_prog->setKernelArgBegin(kernel_index4);
		kernel_prog->setKernelArgument(arg_densf);
		flvr::Argument arg_avg =
			kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY,
				sizeof(unsigned char)*nxyz, NULL);
		flvr::Argument arg_var =
			kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY,
				sizeof(unsigned char)*nxyz, NULL);
		kernel_prog->setKernelArgument(arg_lut);
		kernel_prog->setKernelArgument(arg_params);
		kernel_prog->setKernelArgConst(sizeof(cl_int3), (void*)(&cl_nxyz));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nxy));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&par));

		//mix fields
		kernel_prog->executeKernel(kernel_index3, 3, global_size, local_size);
		//gen avg and var
		kernel_prog->executeKernel(kernel_index4, 3, global_size, local_size);
//#ifdef _DEBUG
//		//read back
//		DBMIUINT8 densf(nx, ny, 1);
//		kernel_prog->readBuffer(arg_densf, densf.data);
//		kernel_prog->readBuffer(arg_avg, densf.data);
//		kernel_prog->readBuffer(arg_var, densf.data);
//		//kernel_prog->readBuffer(arg_distf, densf.data);
//#endif

		//release buffer
		kernel_prog->releaseMemObject(arg_distf);

		//grow
		unsigned int rcnt = 0;
		unsigned int seed = iter > 10 ? iter : 11;
		//set
		size_t region[3] = { (size_t)nx, (size_t)ny, (size_t)nz };
		kernel_prog->setKernelArgBegin(kernel_index5);
		float iterf = 0;
		kernel_prog->setKernelArgConst(sizeof(float), (void*)(&iterf));
		kernel_prog->setKernelArgument(arg_img);
		flvr::Argument arg_label =
			kernel_prog->setKernelArgTex3DBuf(CL_MEM_READ_WRITE, lid, sizeof(unsigned int)*nx*ny*nz, region);
		kernel_prog->setKernelArgument(arg_densf);
		kernel_prog->setKernelArgument(arg_avg);
		kernel_prog->setKernelArgument(arg_var);
		kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(unsigned int), (void*)(&rcnt));
		kernel_prog->setKernelArgument(arg_lut);
		kernel_prog->setKernelArgument(arg_params);
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&seed));
		kernel_prog->setKernelArgConst(sizeof(float), (void*)(&sscale));
		kernel_prog->setKernelArgConst(sizeof(cl_int3), (void*)(&cl_nxyz));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nxy));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&par));

		//execute
		for (int j = 0; j < iter; ++j)
		{
			if (j)
			{
				iterf = j;
				kernel_prog->setKernelArgBegin(kernel_index5);
				kernel_prog->setKernelArgConst(sizeof(float), (void*)(&iterf));
			}
			kernel_prog->executeKernel(kernel_index5, 3, global_size, local_size);
		}

		//clean up
		if (cleanb)
		{
			//temp
			unsigned int size_lm = 5;

			//bit length
			unsigned int lenx = 0;
			unsigned int r = std::max(nx, ny);
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
			cl_int3 cl_lenxyz = { cl_int(lenx), cl_int(lenx), cl_int(lenz) };

			unsigned long long data_size =
				(unsigned long long)nx *
				(unsigned long long)ny *
				(unsigned long long)nz;
			unsigned long long label_size = data_size * 4;

			//set
			//kernel 6
			kernel_prog->setKernelArgBegin(kernel_index6);
			flvr::Argument arg_szbuf = kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE, label_size, nullptr);
			kernel_prog->setKernelArgument(arg_label);
			kernel_prog->setKernelArgConst(sizeof(cl_int3), (void*)(&cl_nxyz));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nxy));
			kernel_prog->setKernelArgConst(sizeof(cl_int3), (void*)(&cl_lenxyz));
			//kernel 7
			kernel_prog->setKernelArgBegin(kernel_index7);
			kernel_prog->setKernelArgument(arg_szbuf);
			kernel_prog->setKernelArgument(arg_label);
			kernel_prog->setKernelArgConst(sizeof(cl_int3), (void*)(&cl_nxyz));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nxy));
			kernel_prog->setKernelArgConst(sizeof(cl_int3), (void*)(&cl_lenxyz));
			//kernel 8
			kernel_prog->setKernelArgBegin(kernel_index8);
			kernel_prog->setKernelArgument(arg_img);
			kernel_prog->setKernelArgument(arg_szbuf);
			kernel_prog->setKernelArgument(arg_label);
			kernel_prog->setKernelArgument(arg_lut);
			kernel_prog->setKernelArgument(arg_params);
			kernel_prog->setKernelArgConst(sizeof(cl_int3), (void*)(&cl_nxyz));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&nxy));
			kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&par));

			//execute
			for (int j = 0; j < clean_iter; ++j)
			{
				kernel_prog->executeKernel(kernel_index6, 3, global_size, local_size);
				kernel_prog->executeKernel(kernel_index7, 3, global_size, local_size);
				kernel_prog->executeKernel(kernel_index8, 3, global_size, local_size);
			}
		}

		//read back
		kernel_prog->copyBufTex3D(arg_label, lid,
			sizeof(unsigned int)*nx*ny*nz, region);

		//release buffer
		kernel_prog->releaseAll();
		delete[] params;

		if (postwork)
			postwork(__FUNCTION__);
	}
}