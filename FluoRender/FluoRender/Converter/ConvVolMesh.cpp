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
#include <GL/glew.h>
#include <ConvVolMesh.h>
#include <Global.h>
#include <AutomateDefault.h>
#include <VolumeData.h>
#include <MeshData.h>
#include <Texture.h>
#include <TextureBrick.h>
#include <VertexArray.h>
#include <VolumeRenderer.h>
#include <MeshRenderer.h>
#include <KernelProgram.h>
#include <VolKernel.h>
#include <ConvVolMeshCode.h>
#include <MCTable.h>
#include <Plane.h>
#include <glm.h>
#include <random>
#include <sstream>
#include <iostream>
#include <unordered_set>

using namespace flrd;

ConvVolMesh::ConvVolMesh() :
	BaseConvVolMesh()
{
}

ConvVolMesh::~ConvVolMesh()
{
}

bool ConvVolMesh::GetInfo(
	flvr::TextureBrick* b, long &bits,
	long &nx, long &ny, long &nz,
	long &ox, long &oy, long &oz)
{
	bits = b->nb(0) * 8;
	nx = b->nx();
	ny = b->ny();
	nz = b->nz();
	ox = b->ox();
	oy = b->oy();
	oz = b->oz();
	return true;
}

void ConvVolMesh::Convert()
{
	auto vd = m_volume.lock();
	if (!vd)
		return;
	if (!vd->GetTexture())
		return;
	int brick_num = vd->GetTexture()->get_brick_num();
	if (brick_num <= 0)
		return;
	bool valid_mask = vd->IsValidMask();
	m_use_sel = m_use_mask && valid_mask;

	//always create new mesh
	m_mesh = std::make_shared<MeshData>();
	m_mesh->SetName(vd->GetName() + L"_mesh");
	m_mesh->AddEmptyData();
	m_mesh->SetFlatShading(true);

	MarchingCubes(vd.get(), m_mesh.get());
}

void ConvVolMesh::Update(bool create_mesh)
{
	auto vd = m_volume.lock();
	if (!vd)
		return;
	if (!vd->GetTexture())
		return;
	int brick_num = vd->GetTexture()->get_brick_num();
	if (brick_num <= 0)
		return;
	bool valid_mask = vd->IsValidMask();
	m_use_sel = m_use_mask && valid_mask;

	if (!m_mesh)
	{
		if (!create_mesh)
			return;

		m_mesh = std::make_shared<MeshData>();
		m_mesh->SetName(vd->GetName() + L"_mesh");
		m_mesh->AddEmptyData();
		m_mesh->SetFlatShading(true);
	}
	else
		m_mesh->ClearData();

	MarchingCubes(vd.get(), m_mesh.get());
}

bool ConvVolMesh::GetAutoUpdate()
{
	auto vd = m_volume.lock();
	if (!vd)
		return false;
	int auto_update = glbin_automate_def.m_conv_vol_mesh;
	if (auto_update == 0)
		return false;
	else if (auto_update == 1)
		return true;
	else if (auto_update == 2)
	{
		if (vd->GetAllBrickNum() > 1)
			return false;
	}
	return true;
}

std::string ConvVolMesh::GetKernelStrMarchingCubes(
	bool mask,
	bool tf)
{
	std::ostringstream z;
	//if mask is used, tf is disabled
	tf = mask ? false : tf;

	z << str_cl_marching_cubes_head;
	//transfer function defs
	if (tf)
	{
		z << str_cl_marching_cubes_clip;
		z << str_cl_marching_cubes_tf;
	}
	//kernel0: preprocess
	if (mask)
		z << str_cl_marching_cubes_kernel0_mask;
	else if (tf)
		z << str_cl_marching_cubes_kernel0_tf;
	else
		z << str_cl_marching_cubes_kernel0_nomask;
	z << str_cl_marching_cubes_kernel_head;
	z << str_cl_marching_cubes_read_volume;
	if (mask)
		z << str_cl_marching_cubes_read_mask;
	if (tf)
		z << str_cl_marching_cubes_apply_clip_tf;
	z << str_cl_marching_cubes_kernel0_tail;
	//kernel1 func: interpolation
	z << str_cl_marching_cubes_kernel1_func;
	//kernel1: marching cubes
	if (mask)
		z << str_cl_marching_cubes_kernel1_mask;
	else if (tf)
		z << str_cl_marching_cubes_kernel1_tf;
	else
		z << str_cl_marching_cubes_kernel1_nomask;
	z << str_cl_marching_cubes_kernel_head;
	z << str_cl_marching_cubes_read_volume;
	if (mask)
		z << str_cl_marching_cubes_read_mask;
	if (tf)
		z << str_cl_marching_cubes_apply_clip_tf;
	z << str_cl_marching_cubes_kernel1_tail;

	return z.str();
}

std::string ConvVolMesh::GetKernelStrSmoothNormals()
{
	std::ostringstream z;
	if (flvr::KernelProgram::get_float_atomics())
		z << str_cl_smooth_normals_float_atomic_supported;
	else
		z << str_cl_smooth_normals_float_atomic_unsupported;
	z << str_cl_smooth_normals;

	return z.str();
}

void ConvVolMesh::MarchingCubes(VolumeData* vd, MeshData* md)
{
	m_busy = true;

	int brick_num = vd->GetTexture()->get_brick_num();
	long bits = vd->GetBits();
	int chars = bits / 8;
	float max_int = static_cast<float>(vd->GetMaxValue());

	//clamp factors
	auto clamp_factor = [](int dim, int factor) -> int
		{
			return (factor < 1) ? 1 : (dim / factor < 1) ? dim : factor;
		};

	//create program and kernels
	bool use_tf = m_use_sel ? false : m_use_transfer;
	flvr::KernelProgram* kernel_prog = glbin_vol_kernel_factory.kernel(
		GetKernelStrMarchingCubes(m_use_sel, use_tf), bits, max_int);
	if (!kernel_prog)
	{
		m_busy = false;
		return;
	}

	int kernel_idx0 = kernel_prog->createKernel("kernel_0");
	if (kernel_idx0 < 0)
	{
		m_busy = false;
		return;
	}
	int kernel_idx1 = kernel_prog->createKernel("kernel_1");
	if (kernel_idx1 < 0)
	{
		m_busy = false;
		return;
	}

	std::vector<flvr::TextureBrick*> *bricks = vd->GetTexture()->get_bricks();
	//estimate the buffer size
	int vsize = 0;
	float iso_value = static_cast<float>(m_iso);
	//transfer function parameters
	bool inv;
	float scalar_scale, lo_thresh, hi_thresh, gamma3d,
		gm_scale, gm_low, gm_high, gm_max,
		lo_offset, hi_offset, sw;
	//clipping planes
	cl_float4 p[6];
	if (use_tf)
	{
		flvr::VolumeRenderer* vr = vd->GetVR();
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
		inv = vd->GetInvert();
		scalar_scale = static_cast<float>(vd->GetScalarScale());
		lo_thresh = static_cast<float>(vd->GetLeftThresh());
		hi_thresh = static_cast<float>(vd->GetRightThresh());
		gamma3d = static_cast<float>(vd->GetGamma());
		gm_scale = static_cast<float>(vd->GetGMScale());
		gm_low = static_cast<float>(vd->GetBoundaryLow());
		gm_high = static_cast<float>(vd->GetBoundaryHigh());
		gm_max = static_cast<float>(vd->GetBoundaryMax());
		lo_offset = static_cast<float>(vd->GetLowOffset());
		hi_offset = static_cast<float>(vd->GetHighOffset());
		sw = static_cast<float>(vd->GetSoftThreshold());
	}
	for (size_t i = 0; i < brick_num; ++i)
	{
		flvr::TextureBrick* b = (*bricks)[i];
		long nx, ny, nz, ox, oy, oz;
		if (!GetInfo(b, bits, nx, ny, nz, ox, oy, oz))
			continue;

		int xy_factor = clamp_factor(std::min(nx, ny), m_downsample);
		int z_factor = clamp_factor(nz, m_downsample_z);
		int ds_nx = nx / xy_factor;
		int ds_ny = ny / xy_factor;
		int ds_nz = nz / z_factor;

		//get tex ids
		GLint tid = vd->GetVR()->load_brick(b);
		GLint mid = 0;
		if (m_use_sel)
			mid = vd->GetVR()->load_brick_mask(b);

		//compute workload
		size_t local_size[3] = { 1, 1, 1 };
		size_t global_size[3] = { size_t(ds_nx), size_t(ds_ny), size_t(ds_nz) };

		kernel_prog->setKernelArgBegin(kernel_idx0);
		kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, tid);
		if (m_use_sel)
			kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, mid);
		kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(int), (void*)(&vsize));
		kernel_prog->setKernelArgConst(sizeof(float), (void*)(&iso_value));
		kernel_prog->setKernelArgConst(sizeof(int), (void*)(&nx));
		kernel_prog->setKernelArgConst(sizeof(int), (void*)(&ny));
		kernel_prog->setKernelArgConst(sizeof(int), (void*)(&nz));
		kernel_prog->setKernelArgConst(sizeof(int), (void*)(&xy_factor));
		kernel_prog->setKernelArgConst(sizeof(int), (void*)(&z_factor));
		if (use_tf)
		{
			cl_float4 loc2 = { inv ? -scalar_scale : scalar_scale, gm_scale, lo_thresh, hi_thresh };
			cl_float4 loc3 = { 1.0f / gamma3d, lo_offset, hi_offset, sw };
			cl_float4 loc17 = { gm_low, gm_high, gm_max, 0.0f };
			fluo::BBox bbx = b->dbox();
			cl_float3 scl = {
				float(bbx.Max().x() - bbx.Min().x()),
				float(bbx.Max().y() - bbx.Min().y()),
				float(bbx.Max().z() - bbx.Min().z()) };
			cl_float3 trl = {
				float(bbx.Min().x()),
				float(bbx.Min().y()),
				float(bbx.Min().z()) };
			kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(&loc2));
			kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(&loc3));
			kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(&loc17));
			kernel_prog->setKernelArgConst(sizeof(cl_float3), (void*)(&scl));
			kernel_prog->setKernelArgConst(sizeof(cl_float3), (void*)(&trl));
			kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(p));
			kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(p + 1));
			kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(p + 2));
			kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(p + 3));
			kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(p + 4));
			kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(p + 5));
		}

		//execute
		kernel_prog->executeKernel(kernel_idx0, 3, global_size, local_size);
	}
	//read back vsize
	kernel_prog->readBuffer(sizeof(int), &vsize, &vsize);
	if (vsize <= 0)
	{
		kernel_prog->releaseAll();
		m_busy = false;
		return;
	}

	//allocate vertex buffer
	GLuint vbo_id = m_mesh->AddCoordVBO(vsize);
	if (m_merged)
		m_mesh->DeleteNormalVBO();
	size_t vbo_size = sizeof(float) * vsize * 45;
	int vsize2 = 0;//reset vsize

	//marching cubes
	for (size_t i = 0; i < brick_num; ++i)
	{
		flvr::TextureBrick* b = (*bricks)[i];
		long nx, ny, nz, ox, oy, oz;
		if (!GetInfo(b, bits, nx, ny, nz, ox, oy, oz))
			continue;

		int xy_factor = clamp_factor(std::min(nx, ny), m_downsample);
		int z_factor = clamp_factor(nz, m_downsample_z);
		int ds_nx = nx / xy_factor;
		int ds_ny = ny / xy_factor;
		int ds_nz = nz / z_factor;

		//get tex ids
		GLint tid = vd->GetVR()->load_brick(b);
		GLint mid = 0;
		if (m_use_sel)
			mid = vd->GetVR()->load_brick_mask(b);

		//compute workload
		size_t local_size[3] = { 1, 1, 1 };
		size_t global_size[3] = { size_t(ds_nx), size_t(ds_ny), size_t(ds_nz) };

		kernel_prog->setKernelArgBegin(kernel_idx1);
		kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, tid);
		if (m_use_sel)
			kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, mid);
		kernel_prog->setKernelArgVertexBuf(CL_MEM_WRITE_ONLY, vbo_id, vbo_size);
		kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(int), (void*)(&vsize2));
		kernel_prog->setKernelArgBuf(CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int) * 24, (void*)(cubeTable));
		kernel_prog->setKernelArgBuf(CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int) * 256, (void*)(edgeTable));
		kernel_prog->setKernelArgBuf(CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int) * 256 * 16, (void*)triTable);
		kernel_prog->setKernelArgBuf(CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int) * 12 * 2, (void*)edgePairs);
		kernel_prog->setKernelArgConst(sizeof(float), (void*)(&iso_value));
		kernel_prog->setKernelArgConst(sizeof(int), (void*)(&nx));
		kernel_prog->setKernelArgConst(sizeof(int), (void*)(&ny));
		kernel_prog->setKernelArgConst(sizeof(int), (void*)(&nz));
		kernel_prog->setKernelArgConst(sizeof(int), (void*)(&xy_factor));
		kernel_prog->setKernelArgConst(sizeof(int), (void*)(&z_factor));
		kernel_prog->setKernelArgConst(sizeof(int), (void*)(&ox));
		kernel_prog->setKernelArgConst(sizeof(int), (void*)(&oy));
		kernel_prog->setKernelArgConst(sizeof(int), (void*)(&oz));
		if (use_tf)
		{
			cl_float4 loc2 = { inv ? -scalar_scale : scalar_scale, gm_scale, lo_thresh, hi_thresh };
			cl_float4 loc3 = { 1.0f / gamma3d, lo_offset, hi_offset, sw };
			cl_float4 loc17 = { gm_low, gm_high, gm_max, 0.0f };
			fluo::BBox bbx = b->dbox();
			cl_float3 scl = {
				float(bbx.Max().x() - bbx.Min().x()),
				float(bbx.Max().y() - bbx.Min().y()),
				float(bbx.Max().z() - bbx.Min().z()) };
			cl_float3 trl = {
				float(bbx.Min().x()),
				float(bbx.Min().y()),
				float(bbx.Min().z()) };
			kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(&loc2));
			kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(&loc3));
			kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(&loc17));
			kernel_prog->setKernelArgConst(sizeof(cl_float3), (void*)(&scl));
			kernel_prog->setKernelArgConst(sizeof(cl_float3), (void*)(&trl));
			kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(p));
			kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(p + 1));
			kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(p + 2));
			kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(p + 3));
			kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(p + 4));
			kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(p + 5));
		}

		//execute
		kernel_prog->executeKernel(kernel_idx1, 3, global_size, local_size);
	}
	//read back vsize
	kernel_prog->readBuffer(sizeof(int), &vsize2, &vsize2);
	//kernel_prog->readBuffer(vbo_arg, &verts[0]);

	//update triangle num
	m_mesh->SetVertexNum(vsize2);
	m_mesh->SetTriangleNum(vsize2 / 3);
	double spcx, spcy, spcz;
	vd->GetSpacings(spcx, spcy, spcz);
	m_mesh->SetScaling(spcx, spcy, spcz);
	m_mesh->SetGpuDirty();
	//download data
	//m_mesh->ReturnData();
	//va_model->unbind();

	kernel_prog->releaseAll();

	m_merged = false;
	m_busy = false;
}

void ConvVolMesh::MergeVertices(bool avg_normals)
{
	if (!m_mesh)
		return;
	if (m_merged)
		return;

	size_t idx_num = m_mesh->GetVertexNum();
	if (idx_num <= 0)
		return;

	m_busy = true;

	//create program kernels
	flvr::KernelProgram* kernel_prog = glbin_vol_kernel_factory.kernel(
		str_cl_merge_vertices, 8, 256.0f);
	if (!kernel_prog)
	{
		m_busy = false;
		return;
	}

	int kernel_idx0 = kernel_prog->createKernel("kernel_0");
	if (kernel_idx0 < 0)
	{
		m_busy = false;
		return;
	}
	int kernel_idx1 = kernel_prog->createKernel("kernel_1");
	if (kernel_idx1 < 0)
	{
		m_busy = false;
		return;
	}
	//int kernel_idx2 = kernel_prog->createKernel("kernel_2");
	//if (kernel_idx2 < 0)
	//{
	//	m_busy = false;
	//	return;
	//}
	int kernel_idx3 = kernel_prog->createKernel("kernel_3");
	if (kernel_idx3 < 0)
	{
		m_busy = false;
		return;
	}
	int kernel_idx4 = kernel_prog->createKernel("kernel_4");
	if (kernel_idx4 < 0)
	{
		m_busy = false;
		return;
	}
	int kernel_idx5 = kernel_prog->createKernel("kernel_5");
	if (kernel_idx5 < 0)
	{
		m_busy = false;
		return;
	}

	//compute workload
	size_t ng;
	kernel_prog->getWorkGroupSize(kernel_idx0, &ng);
	size_t local_size[1] = { 1 };
	//size_t local_size2[1] = { ng };
	size_t global_size[1] = { idx_num };
	size_t global_size2[1] = { idx_num };

	//get vbo
	GLuint vbo_id = m_mesh->GetVBO();
	size_t vbo_size = sizeof(float) * idx_num * 3;
	//add index vbo
	GLuint ibo_id = m_mesh->ConvertIndexed(idx_num);
	size_t ibo_size = sizeof(unsigned int) * idx_num;
	//get epsilon
	double spcx, spcy, spcz;
	if (auto vd = m_volume.lock())
		vd->GetSpacings(spcx, spcy, spcz);
	else
		spcx = spcy = spcz = 1.0;
	float epsilon = static_cast<float>(0.1 * fluo::Min(spcx, spcy, spcz));
	//vertex count
	int vertex_count = static_cast<int>(idx_num);
	//windows size
	int window_size = static_cast<int>(ng) * 4;

	//windowed dedup pass
	kernel_prog->setKernelArgBegin(kernel_idx0);
	flvr::Argument arg_vbo = kernel_prog->setKernelArgVertexBuf(CL_MEM_READ_ONLY, vbo_id, vbo_size);
	flvr::Argument arg_ibo = kernel_prog->setKernelArgVertexBuf(CL_MEM_READ_WRITE, ibo_id, ibo_size);
	kernel_prog->setKernelArgConst(sizeof(int), (void*)(&vertex_count));
	kernel_prog->setKernelArgConst(sizeof(float), (void*)(&epsilon));
	kernel_prog->setKernelArgConst(sizeof(int), (void*)(&window_size));
	//execute
	kernel_prog->executeKernel(kernel_idx0, 1, global_size, local_size);

	//debug read back remap_table
	//std::vector<float> verts(idx_num * 3, 0.0f);
	//kernel_prog->readBuffer(arg_vbo, verts.data());
	//std::vector<int> remap_table_temp(idx_num, 0);
	//kernel_prog->readBuffer(arg_ibo, remap_table_temp.data());

	//count unique vertices
	std::vector<int> unique_flags(idx_num, 0);
	kernel_prog->setKernelArgBegin(kernel_idx1);
	kernel_prog->setKernelArgument(arg_ibo);
	flvr::Argument arg_uflags = kernel_prog->setKernelArgBuf(CL_MEM_WRITE_ONLY, sizeof(int) * idx_num, nullptr);
	kernel_prog->setKernelArgConst(sizeof(int), (void*)(&vertex_count));
	//execute
	kernel_prog->executeKernel(kernel_idx1, 1, global_size, local_size);

	//get count
	kernel_prog->readBuffer(arg_uflags, &unique_flags[0]);
	int unique_count = 0;
	for (size_t i = 0; i < idx_num; ++i)
		if (unique_flags[i])
			++unique_count;
	std::vector<int> remap_table(idx_num, 0);
	kernel_prog->readBuffer(arg_ibo, remap_table.data());

	//compute prefix sum
	std::vector<int> prefix_sum(idx_num, 0);
	std::vector<int> remap_to_compact(idx_num, 0);
	PrefixSum(unique_flags, remap_table, prefix_sum, remap_to_compact, false);
	//kernel_prog->setKernelArgBegin(kernel_idx2);
	//kernel_prog->setKernelArgument(arg_uflags);
	//flvr::Argument arg_psum = kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(int) * idx_num, (void*)(&prefix_sum[0]));
	//kernel_prog->setKernelArgLocal(sizeof(int) * ng * 2);
	//kernel_prog->setKernelArgConst(sizeof(int), (void*)(&vertex_count));
	////execute
	//kernel_prog->executeKernel(kernel_idx2, 1, global_size, local_size2);

	////debug read back prefix sum
	//kernel_prog->readBuffer(sizeof(int) * idx_num, &prefix_sum[0], &prefix_sum[0]);

	//compact and remap
	//allocate new vbo
	kernel_prog->setKernelArgBegin(kernel_idx3);
	kernel_prog->setKernelArgument(arg_vbo);
	kernel_prog->setKernelArgument(arg_ibo);
	kernel_prog->setKernelArgument(arg_uflags);
	flvr::Argument arg_psum = kernel_prog->setKernelArgBuf(CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int) * idx_num, (void*)(&prefix_sum[0]));
	flvr::Argument arg_rtcmp = kernel_prog->setKernelArgBuf(CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int) * idx_num, (void*)(&remap_to_compact[0]));
	flvr::Argument arg_cvbo = kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE, sizeof(float) * unique_count * 3, nullptr);
	flvr::Argument arg_cibo = kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE, sizeof(int) * idx_num, nullptr);
	kernel_prog->setKernelArgConst(sizeof(int), (void*)(&vertex_count));
	//execute
	kernel_prog->executeKernel(kernel_idx3, 1, global_size, local_size);

	//release unused buffers
	kernel_prog->releaseMemObject(arg_vbo);
	kernel_prog->releaseMemObject(arg_ibo);
	kernel_prog->releaseMemObject(arg_uflags);
	kernel_prog->releaseMemObject(arg_psum);
	kernel_prog->releaseMemObject(arg_rtcmp);

	if (window_size < vertex_count)
	{
		//global pass
		vertex_count = unique_count;
		global_size2[0] = vertex_count;
		//find dupes
		//new remap_table
		remap_table.resize(vertex_count, 0);
		kernel_prog->setKernelArgBegin(kernel_idx4);
		kernel_prog->setKernelArgument(arg_cvbo);
		flvr::Argument arg_rtable = kernel_prog->setKernelArgBuf(CL_MEM_WRITE_ONLY, sizeof(int) * vertex_count, nullptr);
		kernel_prog->setKernelArgConst(sizeof(int), (void*)(&vertex_count));
		kernel_prog->setKernelArgConst(sizeof(float), (void*)(&epsilon));
		//loop to process all vertices in batches
		float t = fminf(1.0f, (float)1e5 / vertex_count);
		int batch_size = (int)(window_size + t * (65535 - window_size));
		int batch_num = vertex_count / batch_size + 1;
		int batch_count = 0;
		SetProgress(batch_count, "Global comparisons for vertex merge.");
		for (int lower = 0; lower < vertex_count; lower += batch_size)
		{
			int upper = std::min(lower + batch_size, vertex_count);
			size_t global_size3[1] = { static_cast<size_t>(upper - lower) };
			kernel_prog->setKernelArgBegin(kernel_idx4, 4);
			kernel_prog->setKernelArgConst(sizeof(int), (void*)(&lower));
			kernel_prog->setKernelArgConst(sizeof(int), (void*)(&upper));
			//execute
			kernel_prog->executeKernel(kernel_idx4, 1, global_size3, local_size);
			SetProgress(static_cast<int>(100 * batch_count / batch_num), "Global comparisons for vertex merge.");
			batch_count++;
		}
		SetProgress(0, "");

		//debug read back remap_table
		//kernel_prog->readBuffer(arg_rtable, remap_table.data());

		//count unique vertices
		unique_flags.resize(vertex_count, 0);
		kernel_prog->setKernelArgBegin(kernel_idx1);
		kernel_prog->setKernelArgument(arg_rtable);
		flvr::Argument arg_guflags = kernel_prog->setKernelArgBuf(CL_MEM_WRITE_ONLY, sizeof(int) * vertex_count, nullptr);
		kernel_prog->setKernelArgConst(sizeof(int), (void*)(&vertex_count));
		//execute
		kernel_prog->executeKernel(kernel_idx1, 1, global_size2, local_size);

		//get count
		kernel_prog->readBuffer(arg_guflags, &unique_flags[0]);
		unique_count = 0;
		for (size_t i = 0; i < vertex_count; ++i)
			if (unique_flags[i])
				++unique_count;
		kernel_prog->readBuffer(arg_rtable, remap_table.data());

		//compute prefix sum
		prefix_sum.resize(vertex_count, 0);
		remap_to_compact.resize(vertex_count, 0);
		PrefixSum(unique_flags, remap_table, prefix_sum, remap_to_compact, true);
		int idx_count = static_cast<int>(idx_num);

		//compact and remap
		//allocate new vbo
		kernel_prog->setKernelArgBegin(kernel_idx5);
		kernel_prog->setKernelArgument(arg_cvbo);
		kernel_prog->setKernelArgument(arg_cibo);
		kernel_prog->setKernelArgument(arg_rtable);
		kernel_prog->setKernelArgument(arg_guflags);
		kernel_prog->setKernelArgBuf(CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int) * vertex_count, (void*)(&prefix_sum[0]));
		kernel_prog->setKernelArgBuf(CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int) * vertex_count, (void*)(&remap_to_compact[0]));
		flvr::Argument arg_gcvbo = kernel_prog->setKernelArgBuf(CL_MEM_WRITE_ONLY, sizeof(float) * unique_count * 3, nullptr);
		flvr::Argument arg_gcibo = kernel_prog->setKernelArgBuf(CL_MEM_WRITE_ONLY, sizeof(int) * idx_num, nullptr);
		kernel_prog->setKernelArgConst(sizeof(int), (void*)(&vertex_count));
		kernel_prog->setKernelArgConst(sizeof(int), (void*)(&idx_count));
		//execute
		kernel_prog->executeKernel(kernel_idx5, 1, global_size, local_size);

		//debug read back
		//std::vector<float> old_vbo(vertex_count * 3, 0.0f);
		//std::vector<int> old_ibo(idx_num, 0);
		//kernel_prog->readBuffer(arg_cvbo, old_vbo.data());
		//kernel_prog->readBuffer(arg_cibo, old_ibo.data());

		//global read back
		std::vector<float> compacted_vbo(unique_count * 3, 0.0f);
		std::vector<int> compacted_ibo(idx_num, 0);
		kernel_prog->readBuffer(arg_gcvbo, &compacted_vbo[0]);
		kernel_prog->readBuffer(arg_gcibo, &compacted_ibo[0]);
		//update mesh
		m_mesh->UpdateCoordVBO(compacted_vbo, compacted_ibo);
		if (avg_normals)
		{
			AverageNormals(&arg_gcvbo, &arg_gcibo, unique_count, idx_count);
		}
	}
	else
	{
		//read back
		std::vector<float> compacted_vbo(unique_count * 3, 0.0f);
		std::vector<int> compacted_ibo(idx_num, 0);
		kernel_prog->readBuffer(arg_cvbo, &compacted_vbo[0]);
		kernel_prog->readBuffer(arg_cibo, &compacted_ibo[0]);
		//update mesh
		m_mesh->UpdateCoordVBO(compacted_vbo, compacted_ibo);
		if (avg_normals)
		{
			AverageNormals(&arg_cvbo, &arg_cibo, unique_count, static_cast<int>(idx_num));
		}
	}

	m_mesh->SetVertexNum(unique_count);
	m_mesh->SetTriangleNum(idx_num / 3);//should stay the same
	m_mesh->SetGpuDirty();

	kernel_prog->releaseAll();

	m_merged = true;
	m_busy = false;
}

void ConvVolMesh::AverageNormals(flvr::Argument* pvbo, flvr::Argument* pibo,
	int vertex_count, int idx_count)
{
	if (!pvbo || !pibo)
		return;

	//create program kernels
	flvr::KernelProgram* kernel_prog = glbin_vol_kernel_factory.kernel(
		GetKernelStrSmoothNormals(), 8, 256.0f);
	if (!kernel_prog)
		return;

	int kernel_idx0 = kernel_prog->createKernel("kernel_0");
	if (kernel_idx0 < 0)
		return;
	int kernel_idx1 = kernel_prog->createKernel("kernel_1");
	if (kernel_idx1 < 0)
		return;

	size_t local_size[1] = { 1 };
	size_t global_size[1] = { static_cast<size_t>(idx_count / 3) };
	size_t global_size2[1] = { static_cast<size_t>(vertex_count) };

	std::vector<float> normals(vertex_count * 3, 0.0f);
	//accumulation
	kernel_prog->setKernelArgBegin(kernel_idx0);
	kernel_prog->setKernelArgument(*pvbo);
	kernel_prog->setKernelArgument(*pibo);
	flvr::Argument arg_norm = kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(float) * vertex_count * 3, (void*)(&normals[0]));
	kernel_prog->setKernelArgConst(sizeof(int), (void*)(&idx_count));
	//execute
	kernel_prog->executeKernel(kernel_idx0, 1, global_size, local_size);

	//debug read back
	//kernel_prog->readBuffer(arg_norm, normals.data());

	//normalization
	kernel_prog->setKernelArgBegin(kernel_idx1);
	kernel_prog->setKernelArgument(arg_norm);
	kernel_prog->setKernelArgConst(sizeof(int), (void*)(&vertex_count));
	//execute
	kernel_prog->executeKernel(kernel_idx1, 1, global_size2, local_size);

	//read back normals;
	kernel_prog->readBuffer(arg_norm, normals.data());

	//update mesh...
	m_mesh->UpdateNormalVBO(normals);
}

void ConvVolMesh::PrefixSum(
	const std::vector<int>& unique_flags,
	std::vector<int>& remap_table,
	std::vector<int>& prefix_sum,
	std::vector<int>& remap_to_compact,
	bool flatten_remap_table)
{
	size_t vertex_count = unique_flags.size();
	prefix_sum.resize(vertex_count);
	remap_to_compact.resize(vertex_count, -1);

	int count = 0;
	for (size_t i = 0; i < vertex_count; ++i) {
		prefix_sum[i] = count;
		if (unique_flags[i]) {
			remap_to_compact[i] = count;
			count++;
		}
	}

	// Step 1: Flatten remap_table with cycle protection
	if (flatten_remap_table) {
		for (size_t i = 0; i < vertex_count; ++i) {
			std::unordered_set<int> visited;
			int root = i;
			while (remap_table[root] != root) {
				if (visited.count(root)) {
					// Cycle detected — break it
					remap_table[root] = root;
					break;
				}
				visited.insert(root);
				root = remap_table[root];
			}
			// Path compression
			int j = i;
			while (remap_table[j] != root) {
				int parent = remap_table[j];
				remap_table[j] = root;
				j = parent;
			}
		}
	}

	// Step 2: Resolve canonical safely
	auto resolve_canonical_safe = [&](int i) {
		std::unordered_set<int> visited;
		while (remap_table[i] != i) {
			if (visited.count(i)) {
				// Cycle detected — break it
				return i;
			}
			visited.insert(i);
			i = remap_table[i];
		}
		return i;
		};

	for (size_t i = 0; i < vertex_count; ++i) {
		int canonical = resolve_canonical_safe(remap_table[i]);
		remap_to_compact[i] = remap_to_compact[canonical];
	}
}
