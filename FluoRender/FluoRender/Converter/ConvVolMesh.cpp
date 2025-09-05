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
#include <DataManager.h>
#include <Texture.h>
#include <TextureBrick.h>
#include <VertexArray.h>
#include <VolumeRenderer.h>
#include <MeshRenderer.h>
#include <KernelProgram.h>
#include <VolKernel.h>
#include <ConvVolMeshCode.h>
#include <MCTable.h>
#include <glm.h>
#include <random>

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

	if (!m_mesh)
	{
		m_mesh = std::make_shared<MeshData>();
		m_mesh->SetName(vd->GetName() + L"_mesh");
		m_mesh->AddEmptyData();
	}

	long bits = vd->GetBits();
	int chars = bits / 8;
	float max_int = static_cast<float>(vd->GetMaxValue());

	//clamp factors
	auto clamp_factor = [](int dim, int factor) -> int
		{
			return (factor < 1) ? 1 : (dim / factor < 1) ? dim : factor;
		};

	//create program and kernels
	flvr::KernelProgram* kernel_prog = glbin_vol_kernel_factory.kernel(str_cl_marching_cubes, bits, max_int);
	if (!kernel_prog)
		return;

	int kernel_idx0 = kernel_prog->createKernel("kernel_0");
	if (kernel_idx0 < 0) return;
	int kernel_idx1 = kernel_prog->createKernel("kernel_1");
	if (kernel_idx1 < 0) return;

	std::vector<flvr::TextureBrick*> *bricks = vd->GetTexture()->get_bricks();
	//estimate the buffer size
	int vsize = 0;
	float iso_value = static_cast<float>(m_iso);
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
		if (m_use_mask)
			mid = vd->GetVR()->load_brick_mask(b);

		//compute workload
		size_t local_size[3] = { 1, 1, 1 };
		size_t global_size[3] = { size_t(ds_nx), size_t(ds_ny), size_t(ds_nz) };

		kernel_prog->setKernelArgBegin(kernel_idx0);
		kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, tid);
		kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(int), (void*)(&vsize));
		kernel_prog->setKernelArgConst(sizeof(float), (void*)(&iso_value));
		kernel_prog->setKernelArgConst(sizeof(int), (void*)(&nx));
		kernel_prog->setKernelArgConst(sizeof(int), (void*)(&ny));
		kernel_prog->setKernelArgConst(sizeof(int), (void*)(&nz));
		kernel_prog->setKernelArgConst(sizeof(int), (void*)(&xy_factor));
		kernel_prog->setKernelArgConst(sizeof(int), (void*)(&z_factor));
		//if (m_use_mask)
		//	kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, mid);

		//execute
		kernel_prog->executeKernel(kernel_idx0, 3, global_size, local_size);
	}
	//read back vsize
	kernel_prog->readBuffer(sizeof(int), &vsize, &vsize);
	if (vsize <= 0)
	{
		kernel_prog->releaseAll();
		return;
	}

	//allocate vertex buffer
	GLuint vbo_id = m_mesh->AddVBO(vsize);
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
		if (m_use_mask)
			mid = vd->GetVR()->load_brick_mask(b);

		//compute workload
		size_t local_size[3] = { 1, 1, 1 };
		size_t global_size[3] = { size_t(ds_nx), size_t(ds_ny), size_t(ds_nz) };

		kernel_prog->setKernelArgBegin(kernel_idx1);
		kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, tid);
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
		//if (m_use_mask)
		//	kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, mid);

		//execute
		kernel_prog->executeKernel(kernel_idx1, 3, global_size, local_size);
	}
	//read back vsize
	kernel_prog->readBuffer(sizeof(int), &vsize2, &vsize2);
	//kernel_prog->readBuffer(vbo_arg, &verts[0]);

	//update triangle num
	m_mesh->SetTriangleNum(vsize2 / 3);
	double spcx, spcy, spcz;
	vd->GetSpacings(spcx, spcy, spcz);
	m_mesh->SetScaling(spcx, spcy, spcz);
	m_mesh->SetGpuDirty();
	//download data
	//m_mesh->ReturnData();
	//va_model->unbind();

	kernel_prog->releaseAll();
}

