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
		m_mesh->AddEmptyData();
	}
	flvr::MeshRenderer* mr = m_mesh->GetMR();
	if (!mr)
		return;

	long bits = vd->GetBits();
	int chars = bits / 8;
	float max_int = static_cast<float>(vd->GetMaxValue());

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
		//get tex ids
		GLint tid = vd->GetVR()->load_brick(b);
		GLint mid = 0;
		if (m_use_mask)
			mid = vd->GetVR()->load_brick_mask(b);

		//compute workload
		size_t local_size[3] = { 1, 1, 1 };
		size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };

		kernel_prog->setKernelArgBegin(kernel_idx0);
		kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, tid);
		kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(int), (void*)(&vsize));
		kernel_prog->setKernelArgConst(sizeof(float), (void*)(&iso_value));
		kernel_prog->setKernelArgConst(sizeof(int), (void*)(&nx));
		kernel_prog->setKernelArgConst(sizeof(int), (void*)(&ny));
		kernel_prog->setKernelArgConst(sizeof(int), (void*)(&nz));
		kernel_prog->setKernelArgConst(sizeof(int), (void*)(&m_downsample));
		kernel_prog->setKernelArgConst(sizeof(int), (void*)(&m_downsample_z));
		//if (m_use_mask)
		//	kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, mid);

		//execute
		kernel_prog->executeKernel(kernel_idx0, 3, global_size, local_size);
	}
	//read back vsize
	kernel_prog->readBuffer(sizeof(int), &vsize, &vsize);

	//allocate vertex buffer
	if (vsize <= 0)
	{
		kernel_prog->releaseAll();
		return;
	}
	std::vector<float> verts(vsize * 15);
	size_t vbo_size = sizeof(float) * verts.size();
	//std::mt19937 rng(std::random_device{}());
	//std::uniform_real_distribution<float> dist(0.0f, 10.0f);

	//for (auto &it : verts)
	//	it = dist(rng);

	flvr::VertexArray* va_model = mr->GetOrCreateVertexArray();
	va_model->buffer_data(
		flvr::VABuf_Coord, vbo_size,
		&verts[0], GL_DYNAMIC_DRAW);
	va_model->attrib_pointer(
		0, 3, GL_FLOAT, GL_FALSE, 3, (const GLvoid*)0);
	GLuint vbo_id = static_cast<GLuint>(va_model->id_buffer(flvr::VABuf_Coord));
	int vsize2 = 0;//reset vsize

	//marching cubes
	for (size_t i = 0; i < brick_num; ++i)
	{
		flvr::TextureBrick* b = (*bricks)[i];
		long nx, ny, nz, ox, oy, oz;
		if (!GetInfo(b, bits, nx, ny, nz, ox, oy, oz))
			continue;
		//get tex ids
		GLint tid = vd->GetVR()->load_brick(b);
		GLint mid = 0;
		if (m_use_mask)
			mid = vd->GetVR()->load_brick_mask(b);

		//compute workload
		size_t local_size[3] = { 1, 1, 1 };
		size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };

		kernel_prog->setKernelArgBegin(kernel_idx1);
		kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, tid);
		kernel_prog->setKernelArgVertexBuf(CL_MEM_WRITE_ONLY, vbo_id, vbo_size);
		kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(int), (void*)(&vsize2));
		//kernel_prog->setKernelArgBuf(CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int) * 24, (void*)(cubeTable));
		//kernel_prog->setKernelArgBuf(CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int) * 256, (void*)(edgeTable));
		//kernel_prog->setKernelArgBuf(CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int) * 256 * 16, (void*)triTable);
		//kernel_prog->setKernelArgBuf(CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int) * 12 * 2, (void*)edge_pairs);
		kernel_prog->setKernelArgConst(sizeof(float), (void*)(&iso_value));
		kernel_prog->setKernelArgConst(sizeof(int), (void*)(&nx));
		kernel_prog->setKernelArgConst(sizeof(int), (void*)(&ny));
		kernel_prog->setKernelArgConst(sizeof(int), (void*)(&nz));
		kernel_prog->setKernelArgConst(sizeof(int), (void*)(&m_downsample));
		kernel_prog->setKernelArgConst(sizeof(int), (void*)(&m_downsample_z));
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
	//download data
	m_mesh->ReturnData();
	va_model->unbind();

	kernel_prog->releaseAll();
}

void ConvVolMesh::Test()
{
	cl_context clContext = flvr::KernelProgram::get_context();
	cl_device_id device = flvr::KernelProgram::get_device();
	const int volumeSize = 10;
	const int maxVertices = 10000;
	auto vd = m_volume.lock();
	std::vector<flvr::TextureBrick*> *bricks = vd->GetTexture()->get_bricks();
	flvr::TextureBrick* b = (*bricks)[0];
	GLint texID = vd->GetVR()->load_brick(b);

	// 1. Create GL VBO
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, maxVertices * sizeof(cl_float3), nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// 2. Create CL buffer from GL VBO
	cl_int err;
	cl_mem clVBO = clCreateFromGLBuffer(clContext, CL_MEM_WRITE_ONLY, vbo, &err);
	assert(err == CL_SUCCESS);

	// 3. Share existing GL 3D texture with CL
	cl_mem clVolume = clCreateFromGLTexture(clContext, CL_MEM_READ_ONLY, GL_TEXTURE_3D, 0, texID, &err);
	assert(err == CL_SUCCESS);

	// 4. Create vertex counter buffer
	cl_int initialCount = 0;
	cl_mem vertexCounter = clCreateBuffer(clContext, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
		sizeof(cl_int), &initialCount, &err);
	assert(err == CL_SUCCESS);

	// 5. Create command queue
	cl_command_queue queue = clCreateCommandQueue(clContext, device, 0, &err);
	assert(err == CL_SUCCESS);

	// 6. Create and build program
	const char* kernelSource = R"CLC(
        __kernel void kernel_1(
            __read_only image3d_t volume,
            __global float3* vertex_buffer,
            __global int* vertex_counter,
            float isovalue,
            int volume_width,
            int volume_height,
            int volume_depth,
            int xy_factor,
            int z_factor,
            int x_offset,
            int y_offset,
            int z_offset
        ) {
            int gx = get_global_id(0);
            int gy = get_global_id(1);
            int gz = get_global_id(2);

            int x = gx * xy_factor;
            int y = gy * xy_factor;
            int z = gz * z_factor;

            if (x >= volume_width - xy_factor || y >= volume_height - xy_factor || z >= volume_depth - z_factor)
                return;

            sampler_t samp = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

            float cube[8];
            cube[0] = read_imagef(volume, samp, (int4)(x, y, z, 0)).x;
            cube[1] = read_imagef(volume, samp, (int4)(x+xy_factor, y, z, 0)).x;
            cube[2] = read_imagef(volume, samp, (int4)(x+xy_factor, y+xy_factor, z, 0)).x;
            cube[3] = read_imagef(volume, samp, (int4)(x, y+xy_factor, z, 0)).x;
            cube[4] = read_imagef(volume, samp, (int4)(x, y, z+z_factor, 0)).x;
            cube[5] = read_imagef(volume, samp, (int4)(x+xy_factor, y, z+z_factor, 0)).x;
            cube[6] = read_imagef(volume, samp, (int4)(x+xy_factor, y+xy_factor, z+z_factor, 0)).x;
            cube[7] = read_imagef(volume, samp, (int4)(x, y+xy_factor, z+z_factor, 0)).x;

            int cube_index = 0;
            for (int i = 0; i < 8; ++i)
                if (cube[i] < isovalue)
                    cube_index |= (1 << i);

            if (cube_index == 0 || cube_index == 255)
                return;

            int idx = atomic_add(vertex_counter, 3);
            float3 v0 = (float3)(x + x_offset,     y + y_offset,     z + z_offset);
            float3 v1 = (float3)(x + x_offset + 1, y + y_offset,     z + z_offset);
            float3 v2 = (float3)(x + x_offset,     y + y_offset + 1, z + z_offset);
            vertex_buffer[idx + 0] = v0;
            vertex_buffer[idx + 1] = v1;
            vertex_buffer[idx + 2] = v2;
        }
    )CLC";

	cl_program program = clCreateProgramWithSource(clContext, 1, &kernelSource, nullptr, &err);
	assert(err == CL_SUCCESS);
	err = clBuildProgram(program, 1, &device, nullptr, nullptr, nullptr);
	assert(err == CL_SUCCESS);

	// 7. Acquire GL objects
	cl_mem glObjects[] = { clVBO, clVolume };
	err = clEnqueueAcquireGLObjects(queue, 2, glObjects, 0, nullptr, nullptr);
	assert(err == CL_SUCCESS);

	// 8. Setup kernel
	cl_kernel kernel = clCreateKernel(program, "kernel_1", &err);
	assert(err == CL_SUCCESS);

	err |= clSetKernelArg(kernel, 0, sizeof(cl_mem), &clVolume);
	err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &clVBO);
	err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &vertexCounter);
	err |= clSetKernelArg(kernel, 3, sizeof(cl_float), new cl_float(0.5f));
	err |= clSetKernelArg(kernel, 4, sizeof(cl_int), new cl_int(volumeSize));
	err |= clSetKernelArg(kernel, 5, sizeof(cl_int), new cl_int(volumeSize));
	err |= clSetKernelArg(kernel, 6, sizeof(cl_int), new cl_int(volumeSize));
	err |= clSetKernelArg(kernel, 7, sizeof(cl_int), new cl_int(1));
	err |= clSetKernelArg(kernel, 8, sizeof(cl_int), new cl_int(1));
	err |= clSetKernelArg(kernel, 9, sizeof(cl_int), new cl_int(0));
	err |= clSetKernelArg(kernel, 10, sizeof(cl_int), new cl_int(0));
	err |= clSetKernelArg(kernel, 11, sizeof(cl_int), new cl_int(0));
	assert(err == CL_SUCCESS);

	// 9. Launch kernel
	size_t globalSize[3] = { volumeSize, volumeSize, volumeSize };
	err = clEnqueueNDRangeKernel(queue, kernel, 3, nullptr, globalSize, nullptr, 0, nullptr, nullptr);
	assert(err == CL_SUCCESS);

	// 10. Release GL objects
	err = clEnqueueReleaseGLObjects(queue, 2, glObjects, 0, nullptr, nullptr);
	assert(err == CL_SUCCESS);
	clFinish(queue);

	// 11. Read back vertex data
	/*glBindBuffer(GL_ARRAY_BUFFER, vbo);
	cl_float3* vertices = (cl_float3*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
	if (vertices) {
		for (int i = 0; i < 10; ++i)
			printf("Vertex %d: (%f, %f, %f)\n", i, vertices[i].x, vertices[i].y, vertices[i].z);
		glUnmapBuffer(GL_ARRAY_BUFFER);
	}
	else {
		printf("Failed to map buffer\n");
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);*/

	// 11. Read back vertex data using OpenCL
	std::vector<cl_float3> vertexData(maxVertices);
	err = clEnqueueAcquireGLObjects(queue, 1, &clVBO, 0, nullptr, nullptr);
	assert(err == CL_SUCCESS);

	err = clEnqueueReadBuffer(queue, clVBO, CL_TRUE, 0,
		maxVertices * sizeof(cl_float3),
		vertexData.data(), 0, nullptr, nullptr);
	assert(err == CL_SUCCESS);

	err = clEnqueueReleaseGLObjects(queue, 1, &clVBO, 0, nullptr, nullptr);
	assert(err == CL_SUCCESS);
	clFinish(queue);

	// Print first few vertices
	for (int i = 0; i < 10; ++i) {
		cl_float3 v = vertexData[i];
		printf("Vertex %d: (%f, %f, %f)\n", i, v.x, v.y, v.z);
	}
}
