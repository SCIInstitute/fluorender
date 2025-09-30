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
FITNESS FOR A PARTICULAR PURPOSE AND NONINRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/
#include <ColorCompMesh.h>
#include <ColorCompMeshCode.h>
#include <VolumeData.h>
#include <MeshData.h>
#include <KernelProgram.h>
#include <VolKernel.h>
#include <Global.h>
#include <Texture.h>
#include <TextureBrick.h>
#include <VolumeRenderer.h>
#include <glm.h>

using namespace flrd;

bool ColorCompMesh::GetInfo(
	flvr::TextureBrick* b, long& bits,
	long& nx, long& ny, long& nz,
	long& ox, long& oy, long& oz)
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

void ColorCompMesh::Update()
{
	auto vd = m_volume.lock();
	auto md = m_mesh.lock();
	if (!vd || !md)
		return;
	
	GLuint vbo = md->GetCoordVBO();
	if (!vbo)
		return;
	GLMmodel* mesh = md->GetMesh();
	if (!mesh)
		return;
	int vertex_count = static_cast<int>(mesh->numvertices);
	if (vertex_count <= 0)
		return;
	GLuint cbo = md->AddColorVBO();
	size_t vbo_size = vertex_count * sizeof(float) * 3;
	size_t cbo_size = vertex_count * sizeof(float) * 4;
	int si = vd->GetShuffle();
	double spcx, spcy, spcz;
	vd->GetSpacings(spcx, spcy, spcz);
	cl_float3 voxel_size = { cl_float(spcx), cl_float(spcy), cl_float(spcz) };

	long bits = vd->GetBits();
	float max_int = static_cast<float>(vd->GetMaxValue());

	//create program and kernels
	flvr::KernelProgram* kernel_prog = glbin_vol_kernel_factory.kernel(str_cl_color_comp_mesh, bits, max_int);
	if (!kernel_prog)
		return;
	int kernel_index0 = kernel_prog->createKernel("kernel_0");
	if (!kernel_index0)
		return;

	int brick_num = vd->GetTexture()->get_brick_num();
	std::vector<flvr::TextureBrick*>* bricks = vd->GetTexture()->get_bricks();

	//compute workload
	size_t local_size[1] = { 1 };
	size_t global_size[1] = { size_t(vertex_count) };

	for (size_t i = 0; i < brick_num; ++i)
	{
		flvr::TextureBrick* b = (*bricks)[i];
		long nx, ny, nz, ox, oy, oz;
		if (!GetInfo(b, bits, nx, ny, nz, ox, oy, oz))
			continue;

		//get tex ids
		GLint tid = vd->GetVR()->load_brick(b);
		GLint mid = 0;
		if (m_use_sel)
			mid = vd->GetVR()->load_brick_mask(b);
		GLint lid = vd->GetVR()->load_brick_label(b);
		size_t region[3] = { (size_t)nx, (size_t)ny, (size_t)nz };
		cl_int3 voxel_cnt = { cl_int(nx), cl_int(ny), cl_int(nz) };
		cl_int3 vol_org = { cl_int(ox), cl_int(oy), cl_int(oz) };

		kernel_prog->setKernelArgBegin(kernel_index0);
		kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, tid);
		if (m_use_sel)
			kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, mid);
		flvr::Argument arg_label =
			kernel_prog->setKernelArgTex3DBuf(CL_MEM_READ_ONLY, lid, sizeof(unsigned int) * nx * ny * nz, region);
		kernel_prog->setKernelArgVertexBuf(CL_MEM_READ_ONLY, vbo, vbo_size);
		kernel_prog->setKernelArgVertexBuf(CL_MEM_WRITE_ONLY, cbo, cbo_size);
		kernel_prog->setKernelArgConst(sizeof(cl_int3), (void*)(&voxel_cnt));
		kernel_prog->setKernelArgConst(sizeof(cl_int3), (void*)(&vol_org));
		kernel_prog->setKernelArgConst(sizeof(cl_float3), (void*)(&voxel_size));
		kernel_prog->setKernelArgConst(sizeof(cl_uint), (void*)(&si));
		kernel_prog->setKernelArgConst(sizeof(int), (void*)(&vertex_count));

		//execute
		kernel_prog->executeKernel(kernel_index0, 1, global_size, local_size);
	}
}

