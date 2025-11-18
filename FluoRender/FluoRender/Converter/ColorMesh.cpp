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
#include <ColorMesh.h>
#include <ColorMeshCode.h>
#include <VolumeData.h>
#include <MeshData.h>
#include <KernelProgram.h>
#include <KernelFactory.h>
#include <Global.h>
#include <Texture.h>
#include <TextureBrick.h>
#include <VolumeRenderer.h>
#include <glm.h>

using namespace flrd;

bool ColorMesh::GetInfo(
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

void ColorMesh::Update()
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
	GLuint cbo = md->AddColorVBO(vertex_count);
	fluo::Color color;
	if (m_use_comp)
		color = fluo::Color(1.0);
	else if (m_use_sel)
		color = vd->GetMaskColor();
	else
		color = vd->GetColor();
	md->SetColor(color);
	size_t vbo_size = vertex_count * sizeof(float) * 3;
	size_t cbo_size = vertex_count * sizeof(float) * 4;
	int si = 0;
	if (m_use_comp)
		si = vd->GetShuffle();

	long bits = vd->GetBits();
	float max_int = static_cast<float>(vd->GetMaxValue());
	int mode = m_use_comp ? 2 : (m_use_sel ? 1 : 0);

	//create program and kernels
	flvr::KernelProgram* kernel_prog =
		glbin_kernel_factory.program(GetKernelStrColorMesh(mode), bits, max_int);
	if (!kernel_prog)
		return;
	int kernel_index0 = kernel_prog->createKernel("kernel_0");
	if (kernel_index0 < 0)
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
		GLint tid = 0;
		if (m_use_sel)
			tid = vd->GetVR()->load_brick_mask(b);
		else
			tid = vd->GetVR()->load_brick(b);
		GLint lid = 0;
		if (m_use_comp)
			lid = vd->GetVR()->load_brick_label(b);
		size_t region[3] = { (size_t)nx, (size_t)ny, (size_t)nz };
		cl_int3 voxel_cnt = { cl_int(nx), cl_int(ny), cl_int(nz) };
		cl_int3 vol_org = { cl_int(ox), cl_int(oy), cl_int(oz) };

		kernel_prog->beginArgs(kernel_index0);
		kernel_prog->setTex3D(CL_MEM_READ_ONLY, tid);
		if (m_use_comp)
			kernel_prog->copyTex3DToBuf(CL_MEM_READ_ONLY, lid, "arg_label", sizeof(unsigned int) * nx * ny * nz, region);
		kernel_prog->bindVeretxBuf(CL_MEM_READ_ONLY, vbo, vbo_size);
		kernel_prog->bindVeretxBuf(CL_MEM_WRITE_ONLY, cbo, cbo_size);
		kernel_prog->setConst(sizeof(cl_int3), (void*)(&voxel_cnt));
		kernel_prog->setConst(sizeof(cl_int3), (void*)(&vol_org));
		if (m_use_comp)
			kernel_prog->setConst(sizeof(cl_uint), (void*)(&si));
		kernel_prog->setConst(sizeof(int), (void*)(&vertex_count));

		//execute
		kernel_prog->executeKernel(kernel_index0, 1, global_size, local_size);

		kernel_prog->releaseAllArgs();
	}
	md->SetGpuDirty();

	glbin_kernel_factory.clear(kernel_prog);
}

std::string ColorMesh::GetKernelStrColorMesh(int mode)
{
	bool comp = mode == 2;
	std::ostringstream z;

	if (comp)
		z << str_cl_color_mesh_comp;
	else
		z << str_cl_color_mesh_vol;

	z << str_cl_color_mesh_head;

	if (comp)
		z << str_cl_color_mesh_body_comp;
	else
		z << str_cl_color_mesh_body_vol;

	return z.str();
}