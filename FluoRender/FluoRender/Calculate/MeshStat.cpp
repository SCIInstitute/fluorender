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
#include <MeshStat.h>
#include <KernelProgram.h>
#include <Global.h>
#include <MeshData.h>
#include <KernelFactory.h>
#include <numeric>
#include <execution>

using namespace flrd;

constexpr const char* str_cl_mesh_stat = R"CLKER(
__kernel void kernel_0(
	__global const float* vertex_buffer,   // [vertex_count * 3]
	__global const int* index_buffer,      // [idx_count]
	__global float* tri_area,              // [idx_count/3]
	const int idx_count)
{
	//area
	int i = get_global_id(0);
	if (i >= idx_count / 3) return;

	int i0 = index_buffer[i*3 + 0];
	int i1 = index_buffer[i*3 + 1];
	int i2 = index_buffer[i*3 + 2];

	float3 v0 = vload3(i0, vertex_buffer);
	float3 v1 = vload3(i1, vertex_buffer);
	float3 v2 = vload3(i2, vertex_buffer);

	float3 e1 = v1 - v0;
	float3 e2 = v2 - v0;

	float3 crossp = cross(e1, e2);
	float area = 0.5f * length(crossp);

	tri_area[i] = area;
}

__kernel void kernel_1(
	__global const float* vertex_buffer,
	__global const int* index_buffer,
	__global float* tri_volume,            // [idx_count/3]
	const int idx_count)
{
	//volume
	int i = get_global_id(0);
	if (i >= idx_count / 3) return;

	int i0 = index_buffer[i*3 + 0];
	int i1 = index_buffer[i*3 + 1];
	int i2 = index_buffer[i*3 + 2];

	float3 v0 = vload3(i0, vertex_buffer);
	float3 v1 = vload3(i1, vertex_buffer);
	float3 v2 = vload3(i2, vertex_buffer);

	float3 crossp = cross(v1, v2);
	float vol = dot(v0, crossp) / 6.0f;

	tri_volume[i] = vol;
}
)CLKER";

MeshStat::MeshStat(MeshData* md)
	: Progress(),
	m_md(md),
	m_vertex_num(0),
	m_triangle_num(0),
	m_normal_num(0),
	m_area(0.0),
	m_volume(0.0)
{
}

void MeshStat::Run()
{
	if (!m_md)
		return;

	size_t vertex_num = m_md->GetVertexNum();
	if (vertex_num <= 0)
		return;
	m_vertex_num = static_cast<int>(vertex_num);
	size_t tri_num = m_md->GetTriangleNum();
	if (tri_num <= 0)
		return;
	m_triangle_num = static_cast<int>(tri_num);
	size_t idx_num = tri_num * 3;

	//get vbo
	GLuint vbo_id = m_md->GetCoordVBO();
	if (vbo_id == 0)
		return;
	size_t vbo_size = sizeof(float) * vertex_num * 3;
	//get index vbo
	GLuint ibo_id = m_md->GetIndexVBO();
	if (ibo_id == 0)
		return;
	size_t ibo_size = sizeof(unsigned int) * idx_num;
	//get normals
	GLuint normal_id = m_md->GetNormalVBO();
	if (normal_id)
		m_normal_num = m_triangle_num;
	else
		m_normal_num = 0;

	m_busy = true;

	//create program and kernels
	flvr::KernelProgram* kernel_prog = glbin_kernel_factory.program(
		str_cl_mesh_stat, 8, 256.0f);
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

	//compute workload
	size_t local_size[1] = { 1 };
	size_t global_size[1] = { tri_num };

	//compute area
	kernel_prog->beginArgs(kernel_idx0);
	auto arg_vbo = kernel_prog->bindVeretxBuf(CL_MEM_READ_ONLY, vbo_id, vbo_size);
	auto arg_ibo = kernel_prog->bindVeretxBuf(CL_MEM_READ_ONLY, ibo_id, ibo_size);
	auto arg_area = kernel_prog->setBufNew(CL_MEM_READ_WRITE, "arg_area", sizeof(float) * tri_num, nullptr);
	int idx_count = static_cast<int>(idx_num);
	kernel_prog->setConst(sizeof(int), (void*)(&idx_count));
	//execute
	kernel_prog->executeKernel(kernel_idx0, 1, global_size, local_size);

	//read back
	std::vector<float> area(tri_num);
	kernel_prog->readBuffer(arg_area, area.data());

	//compute area
	m_area = std::reduce(
		std::execution::par_unseq,
		area.begin(),
		area.end(),
		0.0f
	);

	//compute volume
	kernel_prog->beginArgs(kernel_idx1);
	kernel_prog->bindArg(arg_vbo);
	kernel_prog->bindArg(arg_ibo);
	auto arg_volume = kernel_prog->setBufNew(CL_MEM_READ_WRITE, "arg_volume", sizeof(float) * tri_num, nullptr);
	kernel_prog->setConst(sizeof(int), (void*)(&idx_count));
	//execute
	kernel_prog->executeKernel(kernel_idx1, 1, global_size, local_size);

	//read back
	std::vector<float> volume(tri_num);
	kernel_prog->readBuffer(arg_volume, volume.data());

	//compute area
	m_volume = std::reduce(
		std::execution::par_unseq,
		volume.begin(),
		volume.end(),
		0.0f
	);

	kernel_prog->releaseAllArgs();
	m_busy = false;
}

