/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2014 Scientific Computing and Imaging Institute,
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
#include "CompGenerator.h"
#include "cl_code.h"
#include "DataManager.h"
#include <algorithm>

using namespace FL;

ComponentGenerator::ComponentGenerator(VolumeData* vd, int device_id)
	: m_vd(vd),
	m_init(false),
	m_use_mask(false)
{
	cl_int err;
	cl_platform_id platform;

	err = clGetPlatformIDs(1, &platform, NULL);
	if (err != CL_SUCCESS)
		return;

	cl_device_id devices[4];
	cl_uint dev_num = 0;
	err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 4, devices, &dev_num);
	if (err != CL_SUCCESS)
		return;
	if (device_id >= 0 && device_id < dev_num)
		m_device = devices[device_id];
	else
		m_device = devices[0];
	char buffer[10240];
	clGetDeviceInfo(m_device, CL_DEVICE_NAME, sizeof(buffer), buffer, NULL);

	cl_context_properties properties[] =
	{
		CL_CONTEXT_PLATFORM,
		(cl_context_properties)platform,
		0
	};
	m_context = clCreateContext(properties, 1, &m_device, NULL, NULL, &err);
	if (err != CL_SUCCESS)
		return;

	m_init = true;
}

ComponentGenerator::~ComponentGenerator()
{
	clReleaseContext(m_context);
}

void ComponentGenerator::OrderID_3D()
{
	if (!m_vd)
		return;
	m_vd->AddEmptyLabel(1);
	m_sig_progress();
}

#define GET_VOLDATA \
	if (!m_vd) \
		return; \
	int nx, ny, nz; \
	m_vd->GetResolution(nx, ny, nz); \
	Nrrd* nrrd_data = 0; \
	if (m_use_mask) \
		nrrd_data = m_vd->GetMask(true); \
	if (!nrrd_data) \
		nrrd_data = m_vd->GetVolume(false); \
	if (!nrrd_data) \
		return; \
	Nrrd* nrrd_label = m_vd->GetLabel(false); \
	if (!nrrd_data) \
	return; \
	unsigned char* val8 = 0; \
	unsigned short* val16 = 0; \
	int bits; \
	if (nrrd_data->type == nrrdTypeUChar) \
	{ \
		bits = 8; \
		val8 = (unsigned char*)(nrrd_data->data); \
	} \
	else if (nrrd_data->type == nrrdTypeUShort) \
	{ \
		bits = 16; \
		val16 = (unsigned short*)(nrrd_data->data); \
	} \
	unsigned int* val32 = (unsigned int*)(nrrd_label->data);


void ComponentGenerator::OrderID_2D()
{
	GET_VOLDATA

	cl_int err;
	size_t program_size = strlen(str_cl_shuffle_id_2d);
	cl_program program = clCreateProgramWithSource(m_context, 1,
		&str_cl_shuffle_id_2d, &program_size, &err);
	if (err != CL_SUCCESS) return;
	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (err != CL_SUCCESS)
	{
		char *program_log;
		size_t log_size;
		clGetProgramBuildInfo(program, m_device, CL_PROGRAM_BUILD_LOG,
			0, NULL, &log_size);
		program_log = new char[log_size + 1];
		program_log[log_size] = '\0';
		clGetProgramBuildInfo(program, m_device, CL_PROGRAM_BUILD_LOG,
			log_size + 1, program_log, NULL);
		cout << program_log;
		delete[]program_log;
		return;
	}
	cl_kernel kernel = clCreateKernel(program, "kernel_0", &err);
	cl_command_queue queue = clCreateCommandQueue(m_context, m_device, 0, &err);

	size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };
	size_t local_size[3] = { 1, 1, 1 };

	//data
	cl_image_format image_format;
	image_format.image_channel_order = CL_R;
	if (bits == 8)
		image_format.image_channel_data_type = CL_UNORM_INT8;
	else if (bits == 16)
		image_format.image_channel_data_type = CL_UNORM_INT16;
	cl_image_desc image_desc;
	image_desc.image_type = CL_MEM_OBJECT_IMAGE3D;
	image_desc.image_width = nx;
	image_desc.image_height = ny;
	image_desc.image_depth = nz;
	image_desc.image_array_size = 0;
	image_desc.image_row_pitch = 0;
	image_desc.image_slice_pitch = 0;
	image_desc.num_mip_levels = 0;
	image_desc.num_samples = 0;
	image_desc.buffer = 0;
	cl_mem data_buffer = clCreateImage(m_context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		&image_format, &image_desc, bits == 8 ? (void*)(val8) : (void*)(val16), &err);
	//label
	cl_mem label_buffer = clCreateBuffer(m_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
		sizeof(unsigned int)*nx*ny*nz, val32, &err);
	//set
	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &data_buffer);
	err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &label_buffer);
	err = clSetKernelArg(kernel, 2, sizeof(unsigned int), (void*)(&nx));
	err = clSetKernelArg(kernel, 3, sizeof(unsigned int), (void*)(&ny));
	err = clSetKernelArg(kernel, 4, sizeof(unsigned int), (void*)(&nz));

	err = clEnqueueNDRangeKernel(queue, kernel, 3, NULL, global_size,
		local_size, 0, NULL, NULL);

	clFinish(queue);
	err = clEnqueueReadBuffer(
		queue, label_buffer,
		CL_TRUE, 0, sizeof(unsigned int)*nx*ny*nz,
		val32, 0, NULL, NULL);

	clReleaseMemObject(data_buffer);
	clReleaseMemObject(label_buffer);
	clReleaseKernel(kernel);
	clReleaseCommandQueue(queue);
	clReleaseProgram(program);

	m_sig_progress();
}

void ComponentGenerator::ShuffleID_3D()
{
	GET_VOLDATA

	cl_int err;
	size_t program_size = strlen(str_cl_shuffle_id_3d);
	cl_program program = clCreateProgramWithSource(m_context, 1,
		&str_cl_shuffle_id_3d, &program_size, &err);
	if (err != CL_SUCCESS) return;
	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (err != CL_SUCCESS)
	{
		char *program_log;
		size_t log_size;
		clGetProgramBuildInfo(program, m_device, CL_PROGRAM_BUILD_LOG,
			0, NULL, &log_size);
		program_log = new char[log_size + 1];
		program_log[log_size] = '\0';
		clGetProgramBuildInfo(program, m_device, CL_PROGRAM_BUILD_LOG,
			log_size + 1, program_log, NULL);
		cout << program_log;
		delete[]program_log;
		return;
	}
	cl_kernel kernel = clCreateKernel(program, "kernel_0", &err);
	cl_command_queue queue = clCreateCommandQueue(m_context, m_device, 0, &err);

	size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };
	size_t local_size[3] = { 1, 1, 1 };
	unsigned int len = 0;
	unsigned int r = Max(nx, Max(ny, nz));
	while (r > 0)
	{
		r /= 2;
		len++;
	}

	//data
	cl_image_format image_format;
	image_format.image_channel_order = CL_R;
	if (bits == 8)
		image_format.image_channel_data_type = CL_UNORM_INT8;
	else if (bits == 16)
		image_format.image_channel_data_type = CL_UNORM_INT16;
	cl_image_desc image_desc;
	image_desc.image_type = CL_MEM_OBJECT_IMAGE3D;
	image_desc.image_width = size_t(nx);
	image_desc.image_height = size_t(ny);
	image_desc.image_depth = size_t(nz);
	image_desc.image_array_size = 0;
	image_desc.image_row_pitch = 0;
	image_desc.image_slice_pitch = 0;
	image_desc.num_mip_levels = 0;
	image_desc.num_samples = 0;
	image_desc.buffer = 0;
	cl_mem data_buffer = clCreateImage(m_context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		&image_format, &image_desc, bits==8? (void*)(val8): (void*)(val16), &err);
	//label
	cl_mem label_buffer = clCreateBuffer(m_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
		sizeof(unsigned int)*nx*ny*nz, val32, &err);
	//set
	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &data_buffer);
	err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &label_buffer);
	err = clSetKernelArg(kernel, 2, sizeof(unsigned int), (void*)(&nx));
	err = clSetKernelArg(kernel, 3, sizeof(unsigned int), (void*)(&ny));
	err = clSetKernelArg(kernel, 4, sizeof(unsigned int), (void*)(&nz));
	err = clSetKernelArg(kernel, 5, sizeof(unsigned int), (void*)(&len));

	err = clEnqueueNDRangeKernel(queue, kernel, 3, NULL, global_size,
		local_size, 0, NULL, NULL);

	clFinish(queue);
	err = clEnqueueReadBuffer(
		queue, label_buffer,
		CL_TRUE, 0, sizeof(unsigned int)*nx*ny*nz,
		val32, 0, NULL, NULL);

	clReleaseMemObject(data_buffer);
	clReleaseMemObject(label_buffer);
	clReleaseKernel(kernel);
	clReleaseCommandQueue(queue);
	clReleaseProgram(program);

	m_sig_progress();
}

void ComponentGenerator::ShuffleID_2D()
{
	if (!m_vd)
		return;
	m_vd->AddEmptyLabel(2);

	m_sig_progress();
}

void ComponentGenerator::Grow3D(bool diffuse, int iter, float tran, float falloff)
{
	GET_VOLDATA

	cl_int err;
	size_t program_size = strlen(str_cl_brainbow_3d);
	cl_program program = clCreateProgramWithSource(m_context, 1,
		&str_cl_brainbow_3d, &program_size, &err);
	if (err != CL_SUCCESS) return;
	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (err != CL_SUCCESS)
	{
		char *program_log;
		size_t log_size;
		clGetProgramBuildInfo(program, m_device, CL_PROGRAM_BUILD_LOG,
			0, NULL, &log_size);
		program_log = new char[log_size + 1];
		program_log[log_size] = '\0';
		clGetProgramBuildInfo(program, m_device, CL_PROGRAM_BUILD_LOG,
			log_size + 1, program_log, NULL);
		cout << program_log;
		delete[]program_log;
		return;
	}
	cl_kernel kernel = clCreateKernel(program, "kernel_0", &err);
	cl_command_queue queue = clCreateCommandQueue(m_context, m_device, 0, &err);

	unsigned int rcnt = 0;
	unsigned int seed = iter > 10 ? iter : 11;
	size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };
	size_t local_size[3] = { 1, 1, 1 };
	float scl_ff = diffuse?falloff:0.0f;
	float grad_ff = diffuse?falloff:0.0f;

	//data
	cl_image_format image_format;
	image_format.image_channel_order = CL_R;
	if (bits == 8)
		image_format.image_channel_data_type = CL_UNORM_INT8;
	else if (bits == 16)
		image_format.image_channel_data_type = CL_UNORM_INT16;
	cl_image_desc image_desc;
	image_desc.image_type = CL_MEM_OBJECT_IMAGE3D;
	image_desc.image_width = nx;
	image_desc.image_height = ny;
	image_desc.image_depth = nz;
	image_desc.image_array_size = 0;
	image_desc.image_row_pitch = 0;
	image_desc.image_slice_pitch = 0;
	image_desc.num_mip_levels = 0;
	image_desc.num_samples = 0;
	image_desc.buffer = 0;
	cl_mem data_buffer = clCreateImage(m_context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		&image_format, &image_desc, bits == 8 ? (void*)(val8) : (void*)(val16), &err);
	//label
	cl_mem label_buffer = clCreateBuffer(m_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
		sizeof(unsigned int)*nx*ny*nz, val32, &err);
	//counter
	cl_mem rcnt_buffer = clCreateBuffer(m_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
		sizeof(unsigned int), &rcnt, &err);
	//set
	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &data_buffer);
	err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &label_buffer);
	err = clSetKernelArg(kernel, 2, sizeof(unsigned int), (void*)(&nx));
	err = clSetKernelArg(kernel, 3, sizeof(unsigned int), (void*)(&ny));
	err = clSetKernelArg(kernel, 4, sizeof(unsigned int), (void*)(&nz));
	err = clSetKernelArg(kernel, 5, sizeof(cl_mem), &rcnt_buffer);
	err = clSetKernelArg(kernel, 6, sizeof(unsigned int), (void*)(&seed));
	err = clSetKernelArg(kernel, 7, sizeof(float), (void*)(&tran));
	err = clSetKernelArg(kernel, 8, sizeof(float), (void*)(&scl_ff));
	err = clSetKernelArg(kernel, 9, sizeof(float), (void*)(&grad_ff));

	for (int j = 0; j < iter; ++j)
	{
		err = clEnqueueNDRangeKernel(queue, kernel, 3, NULL, global_size,
			local_size, 0, NULL, NULL);

		clFinish(queue);
		m_sig_progress();
	}

	err = clEnqueueReadBuffer(
		queue, label_buffer,
		CL_TRUE, 0, sizeof(unsigned int)*nx*ny*nz,
		val32, 0, NULL, NULL);

	clReleaseMemObject(data_buffer);
	clReleaseMemObject(label_buffer);
	clReleaseMemObject(rcnt_buffer);
	clReleaseKernel(kernel);
	clReleaseCommandQueue(queue);
	clReleaseProgram(program);
}

void ComponentGenerator::InitialGrow(bool param_tr, int iter,
	float tran, float tran2, float scl_ff, float scl_ff2,
	float grad_ff, float grad_ff2, float var_ff, float var_ff2,
	float ang_ff, float ang_ff2)
{
	GET_VOLDATA

	cl_int err;
	size_t program_size = strlen(str_cl_slice_brainbow);
	cl_program program = clCreateProgramWithSource(m_context, 1,
		&str_cl_slice_brainbow, &program_size, &err);
	if (err != CL_SUCCESS) return;
	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (err != CL_SUCCESS)
	{
		char *program_log;
		size_t log_size;
		clGetProgramBuildInfo(program, m_device, CL_PROGRAM_BUILD_LOG,
			0, NULL, &log_size);
		program_log = new char[log_size + 1];
		program_log[log_size] = '\0';
		clGetProgramBuildInfo(program, m_device, CL_PROGRAM_BUILD_LOG,
			log_size + 1, program_log, NULL);
		cout << program_log;
		delete[]program_log;
		return;
	}
	cl_kernel kernel = clCreateKernel(program, "kernel_0", &err);
	cl_command_queue queue = clCreateCommandQueue(m_context, m_device, 0, &err);

	unsigned char* cur_page_data8 = bits == 8 ? val8 : 0;
	unsigned short* cur_page_data16 = bits == 16 ? val16 : 0;
	unsigned int* cur_page_label = val32;
	unsigned int rcnt = 0;
	unsigned int seed = iter>10 ? iter : 11;
	size_t global_size[2] = { size_t(nx), size_t(ny) };
	size_t local_size[2] = { 1, 1 };
	//trnsition params
	float cur_tran, cur_scl_ff, cur_grad_ff, cur_var_ff, cur_ang_ff;

	//data
	cl_image_format image_format;
	image_format.image_channel_order = CL_R;
	if (bits == 8)
		image_format.image_channel_data_type = CL_UNORM_INT8;
	else if (bits == 16)
		image_format.image_channel_data_type = CL_UNORM_INT16;
	cl_image_desc image_desc;
	image_desc.image_type = CL_MEM_OBJECT_IMAGE2D;
	image_desc.image_width = nx;
	image_desc.image_height = ny;
	image_desc.image_depth = 0;
	image_desc.image_array_size = 0;
	image_desc.image_row_pitch = 0;
	image_desc.image_slice_pitch = 0;
	image_desc.num_mip_levels = 0;
	image_desc.num_samples = 0;
	image_desc.buffer = 0;
	cl_mem data_buffer = clCreateImage(m_context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		&image_format, &image_desc, bits==8?(void*)(cur_page_data8):(void*)(cur_page_data16), &err);
	//label
	cl_mem label_buffer = clCreateBuffer(m_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
		sizeof(unsigned int)*nx*ny, cur_page_label, &err);
	//counter
	cl_mem rcnt_buffer = clCreateBuffer(m_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
		sizeof(unsigned int), &rcnt, &err);
	//set
	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &data_buffer);
	err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &label_buffer);
	err = clSetKernelArg(kernel, 2, sizeof(unsigned int), (void*)(&nx));
	err = clSetKernelArg(kernel, 3, sizeof(unsigned int), (void*)(&ny));
	err = clSetKernelArg(kernel, 4, sizeof(cl_mem), &rcnt_buffer);
	err = clSetKernelArg(kernel, 5, sizeof(unsigned int), (void*)(&seed));
	if (!param_tr)
	{
		err = clSetKernelArg(kernel, 6, sizeof(float), (void*)(&tran));
		err = clSetKernelArg(kernel, 7, sizeof(float), (void*)(&scl_ff));
		err = clSetKernelArg(kernel, 8, sizeof(float), (void*)(&grad_ff));
		err = clSetKernelArg(kernel, 9, sizeof(float), (void*)(&var_ff));
		err = clSetKernelArg(kernel, 10, sizeof(float), (void*)(&ang_ff));
	}

	const size_t origin[] = { 0, 0, 0 };
	const size_t region[] = { size_t(nx), size_t(ny), 1 };
	size_t count = 0;
	for (size_t i = 0; i<nz; ++i)
	{
		if (i)
		{
			err = clEnqueueWriteImage(
				queue, data_buffer,
				CL_TRUE, origin, region,
				0, 0, bits==8?(void*)(cur_page_data8):(void*)(cur_page_data16), 0, NULL, NULL);
			err = clEnqueueWriteBuffer(
				queue, label_buffer,
				CL_TRUE, 0, sizeof(unsigned int)*nx*ny,
				cur_page_label, 0, NULL, NULL);
		}

		for (int j = 0; j < iter; ++j)
		{
			if (param_tr)
			{
				cur_tran = tran + (float)j*(tran2 - tran) / (float)(iter - 1);
				cur_scl_ff = scl_ff + (float)j*(scl_ff2 - scl_ff) / (float)(iter - 1);
				cur_grad_ff = grad_ff + (float)j*(grad_ff2 - grad_ff) / (float)(iter - 1);
				cur_var_ff = var_ff + (float)j*(var_ff2 - var_ff) / (float)(iter - 1);
				cur_ang_ff = ang_ff + (float)j*(ang_ff2 - ang_ff) / (float)(iter - 1);
				err = clSetKernelArg(kernel, 6, sizeof(float), (void*)(&cur_tran));
				err = clSetKernelArg(kernel, 7, sizeof(float), (void*)(&cur_scl_ff));
				err = clSetKernelArg(kernel, 8, sizeof(float), (void*)(&cur_grad_ff));
				err = clSetKernelArg(kernel, 9, sizeof(float), (void*)(&cur_var_ff));
				err = clSetKernelArg(kernel, 10, sizeof(float), (void*)(&cur_ang_ff));
			}

			err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global_size,
				local_size, 0, NULL, NULL);

			count++;
			if (count == nz - 1)
			{
				count = 0;
				m_sig_progress();
			}
		}

		clFinish(queue);
		err = clEnqueueReadBuffer(
			queue, label_buffer,
			CL_TRUE, 0, sizeof(unsigned int)*nx*ny,
			cur_page_label, 0, NULL, NULL);

		if (bits == 8)
			cur_page_data8 += nx*ny;
		else if (bits == 16)
			cur_page_data16 += nx*ny;
		cur_page_label += nx*ny;
	}

	clReleaseMemObject(data_buffer);
	clReleaseMemObject(label_buffer);
	clReleaseMemObject(rcnt_buffer);
	clReleaseKernel(kernel);
	clReleaseCommandQueue(queue);
	clReleaseProgram(program);
}

void ComponentGenerator::SizedGrow(bool param_tr, int iter,
	unsigned int size_lm, unsigned int size_lm2, float tran, float tran2,
	float scl_ff, float scl_ff2, float grad_ff, float grad_ff2,
	float var_ff, float var_ff2, float ang_ff, float ang_ff2)
{
	GET_VOLDATA
		
	unsigned int* mask32 = new unsigned int[nx*ny];
	memset(mask32, 0, sizeof(unsigned int)*nx*ny);

	cl_int err;
	size_t program_size = strlen(str_cl_grow_size);
	cl_program program = clCreateProgramWithSource(m_context, 1,
		&str_cl_grow_size, &program_size, &err);
	if (err != CL_SUCCESS) return;
	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (err != CL_SUCCESS)
	{
		char *program_log;
		size_t log_size;
		clGetProgramBuildInfo(program, m_device, CL_PROGRAM_BUILD_LOG,
			0, NULL, &log_size);
		program_log = new char[log_size + 1];
		program_log[log_size] = '\0';
		clGetProgramBuildInfo(program, m_device, CL_PROGRAM_BUILD_LOG,
			log_size + 1, program_log, NULL);
		cout << program_log;
		delete[]program_log;
		return;
	}
	cl_command_queue queue = clCreateCommandQueue(m_context, m_device, 0, &err);
	cl_kernel kernel_0 = clCreateKernel(program, "kernel_0", &err);
	cl_kernel kernel_1 = clCreateKernel(program, "kernel_1", &err);
	cl_kernel kernel_2 = clCreateKernel(program, "kernel_2", &err);

	unsigned char* cur_page_data8 = bits == 8 ? val8 : 0;
	unsigned short* cur_page_data16 = bits == 16 ? val16 : 0;
	unsigned int* cur_page_label = val32;
	unsigned int rcnt = 0;
	unsigned int seed = iter>10 ? iter : 11;
	size_t global_size[2] = { size_t(nx), size_t(ny) };
	size_t local_size[2] = { 1, 1 };
	//trnsition params
	unsigned int cur_size_lm;
	float cur_tran, cur_scl_ff, cur_grad_ff, cur_var_ff, cur_ang_ff;

	//data
	cl_image_format image_format;
	image_format.image_channel_order = CL_R;
	if (bits == 8)
		image_format.image_channel_data_type = CL_UNORM_INT8;
	else if (bits == 16)
		image_format.image_channel_data_type = CL_UNORM_INT16;
	cl_image_desc image_desc;
	image_desc.image_type = CL_MEM_OBJECT_IMAGE2D;
	image_desc.image_width = nx;
	image_desc.image_height = ny;
	image_desc.image_depth = 0;
	image_desc.image_array_size = 0;
	image_desc.image_row_pitch = 0;
	image_desc.image_slice_pitch = 0;
	image_desc.num_mip_levels = 0;
	image_desc.num_samples = 0;
	image_desc.buffer = 0;
	cl_mem data_buffer = clCreateImage(m_context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		&image_format, &image_desc, bits == 8 ? (void*)(cur_page_data8) : (void*)(cur_page_data16), &err);
	//mask
	cl_mem mask_buffer = clCreateBuffer(m_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
		sizeof(unsigned int)*nx*ny, mask32, &err);
	//label
	cl_mem label_buffer = clCreateBuffer(m_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
		sizeof(unsigned int)*nx*ny, cur_page_label, &err);
	//counter
	cl_mem rcnt_buffer = clCreateBuffer(m_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
		sizeof(unsigned int), &rcnt, &err);
	//set
	//kernel 0
	err = clSetKernelArg(kernel_0, 0, sizeof(cl_mem), &mask_buffer);
	err = clSetKernelArg(kernel_0, 1, sizeof(cl_mem), &label_buffer);
	err = clSetKernelArg(kernel_0, 2, sizeof(unsigned int), (void*)(&nx));
	err = clSetKernelArg(kernel_0, 3, sizeof(unsigned int), (void*)(&ny));
	//kernel 1
	err = clSetKernelArg(kernel_1, 0, sizeof(cl_mem), &mask_buffer);
	err = clSetKernelArg(kernel_1, 1, sizeof(cl_mem), &label_buffer);
	err = clSetKernelArg(kernel_1, 2, sizeof(unsigned int), (void*)(&nx));
	err = clSetKernelArg(kernel_1, 3, sizeof(unsigned int), (void*)(&ny));
	//kernel 2
	err = clSetKernelArg(kernel_2, 0, sizeof(cl_mem), &data_buffer);
	err = clSetKernelArg(kernel_2, 1, sizeof(cl_mem), &mask_buffer);
	err = clSetKernelArg(kernel_2, 2, sizeof(cl_mem), &label_buffer);
	err = clSetKernelArg(kernel_2, 3, sizeof(unsigned int), (void*)(&nx));
	err = clSetKernelArg(kernel_2, 4, sizeof(unsigned int), (void*)(&ny));
	err = clSetKernelArg(kernel_2, 5, sizeof(cl_mem), &rcnt_buffer);
	err = clSetKernelArg(kernel_2, 6, sizeof(unsigned int), (void*)(&seed));
	if (!param_tr)
	{
		err = clSetKernelArg(kernel_2, 7, sizeof(float), (void*)(&tran));
		err = clSetKernelArg(kernel_2, 8, sizeof(float), (void*)(&scl_ff));
		err = clSetKernelArg(kernel_2, 9, sizeof(float), (void*)(&grad_ff));
		err = clSetKernelArg(kernel_2, 10, sizeof(float), (void*)(&var_ff));
		err = clSetKernelArg(kernel_2, 11, sizeof(float), (void*)(&ang_ff));
		err = clSetKernelArg(kernel_2, 12, sizeof(unsigned int), (void*)(&size_lm));
	}

	const size_t origin[] = { 0, 0, 0 };
	const size_t region[] = { size_t(nx), size_t(ny), 1 };
	size_t count = 0;
	for (size_t i = 0; i<nz; ++i)
	{
		if (i)
		{
			err = clEnqueueWriteImage(
				queue, data_buffer,
				CL_TRUE, origin, region,
				0, 0, bits == 8 ? (void*)(cur_page_data8) : (void*)(cur_page_data16), 0, NULL, NULL);
			err = clEnqueueWriteBuffer(
				queue, label_buffer,
				CL_TRUE, 0, sizeof(unsigned int)*nx*ny,
				cur_page_label, 0, NULL, NULL);
		}

		for (int j = 0; j<iter; ++j)
		{
			err = clEnqueueWriteBuffer(
				queue, mask_buffer,
				CL_TRUE, 0, sizeof(unsigned int)*nx*ny,
				mask32, 0, NULL, NULL);
			err = clEnqueueNDRangeKernel(queue, kernel_0, 2, NULL, global_size,
				local_size, 0, NULL, NULL);
			err = clEnqueueNDRangeKernel(queue, kernel_1, 2, NULL, global_size,
				local_size, 0, NULL, NULL);

			if (param_tr)
			{
				cur_size_lm = size_lm + (unsigned int)((float)j*(size_lm2 - size_lm) / (float)(iter - 1) + 0.5f);
				cur_tran = tran + (float)j*(tran2 - tran) / (float)(iter - 1);
				cur_scl_ff = scl_ff + (float)j*(scl_ff2 - scl_ff) / (float)(iter - 1);
				cur_grad_ff = grad_ff + (float)j*(grad_ff2 - grad_ff) / (float)(iter - 1);
				cur_var_ff = var_ff + (float)j*(var_ff2 - var_ff) / (float)(iter - 1);
				cur_ang_ff = ang_ff + (float)j*(ang_ff2 - ang_ff) / (float)(iter - 1);
				err = clSetKernelArg(kernel_2, 7, sizeof(float), (void*)(&cur_tran));
				err = clSetKernelArg(kernel_2, 8, sizeof(float), (void*)(&cur_scl_ff));
				err = clSetKernelArg(kernel_2, 9, sizeof(float), (void*)(&cur_grad_ff));
				err = clSetKernelArg(kernel_2, 10, sizeof(float), (void*)(&cur_var_ff));
				err = clSetKernelArg(kernel_2, 11, sizeof(float), (void*)(&cur_ang_ff));
				err = clSetKernelArg(kernel_2, 12, sizeof(unsigned int), (void*)(&cur_size_lm));
			}
			err = clEnqueueNDRangeKernel(queue, kernel_2, 2, NULL, global_size,
				local_size, 0, NULL, NULL);

			count++;
			if (count == nz - 1)
			{
				count = 0;
				m_sig_progress();
			}
		}

		clFinish(queue);
		err = clEnqueueReadBuffer(
			queue, label_buffer,
			CL_TRUE, 0, sizeof(unsigned int)*nx*ny,
			cur_page_label, 0, NULL, NULL);

		if (bits == 8)
			cur_page_data8 += nx*ny;
		else if (bits == 16)
			cur_page_data16 += nx*ny;
		cur_page_label += nx*ny;
	}

	clReleaseMemObject(data_buffer);
	clReleaseMemObject(label_buffer);
	clReleaseMemObject(mask_buffer);
	clReleaseMemObject(rcnt_buffer);
	clReleaseKernel(kernel_0);
	clReleaseKernel(kernel_1);
	clReleaseKernel(kernel_2);
	clReleaseCommandQueue(queue);
	clReleaseProgram(program);
	delete[]mask32;
}

void ComponentGenerator::Cleanup(int iter, unsigned int size_lm)
{
	if (!m_vd)
		return;
	int nx, ny, nz;
	m_vd->GetResolution(nx, ny, nz);
	Nrrd* nrrd_label = m_vd->GetLabel(false);
	if (!nrrd_label)
		return;
	unsigned int* val32 = (unsigned int*)(nrrd_label->data);
	unsigned int* mask32 = new unsigned int[nx*ny];
	memset(mask32, 0, sizeof(unsigned int)*nx*ny);

	cl_int err;
	cl_command_queue queue = clCreateCommandQueue(m_context, m_device, 0, &err);

	size_t program_size = strlen(str_cl_grow_size);
	cl_program program = clCreateProgramWithSource(m_context, 1,
		&str_cl_grow_size, &program_size, &err);
	if (err != CL_SUCCESS) return;
	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (err != CL_SUCCESS)
	{
		char *program_log;
		size_t log_size;
		clGetProgramBuildInfo(program, m_device, CL_PROGRAM_BUILD_LOG,
			0, NULL, &log_size);
		program_log = new char[log_size + 1];
		program_log[log_size] = '\0';
		clGetProgramBuildInfo(program, m_device, CL_PROGRAM_BUILD_LOG,
			log_size + 1, program_log, NULL);
		cout << program_log;
		delete[]program_log;
		return;
	}
	cl_kernel kernel_0 = clCreateKernel(program, "kernel_0", &err);
	cl_kernel kernel_1 = clCreateKernel(program, "kernel_1", &err);
	clReleaseProgram(program);

	program_size = strlen(str_cl_clean_up);
	program = clCreateProgramWithSource(m_context, 1,
		&str_cl_clean_up, &program_size, &err);
	if (err != CL_SUCCESS) return;
	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (err != CL_SUCCESS)
	{
		char *program_log;
		size_t log_size;
		clGetProgramBuildInfo(program, m_device, CL_PROGRAM_BUILD_LOG,
			0, NULL, &log_size);
		program_log = new char[log_size + 1];
		program_log[log_size] = '\0';
		clGetProgramBuildInfo(program, m_device, CL_PROGRAM_BUILD_LOG,
			log_size + 1, program_log, NULL);
		cout << program_log;
		delete[]program_log;
		return;
	}
	cl_kernel kernel_2 = clCreateKernel(program, "kernel_0", &err);

	unsigned int* cur_page_label = val32;
	unsigned int thresh = 5;
	size_t global_size[2] = { size_t(nx), size_t(ny) };
	size_t local_size[2] = { 1, 1 };

	//mask
	cl_mem mask_buffer = clCreateBuffer(m_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
		sizeof(unsigned int)*nx*ny, mask32, &err);
	//label
	cl_mem label_buffer = clCreateBuffer(m_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
		sizeof(unsigned int)*nx*ny, cur_page_label, &err);
	//set
	//kernel 0
	err = clSetKernelArg(kernel_0, 0, sizeof(cl_mem), &mask_buffer);
	err = clSetKernelArg(kernel_0, 1, sizeof(cl_mem), &label_buffer);
	err = clSetKernelArg(kernel_0, 2, sizeof(unsigned int), (void*)(&nx));
	err = clSetKernelArg(kernel_0, 3, sizeof(unsigned int), (void*)(&ny));
	//kernel 1
	err = clSetKernelArg(kernel_1, 0, sizeof(cl_mem), &mask_buffer);
	err = clSetKernelArg(kernel_1, 1, sizeof(cl_mem), &label_buffer);
	err = clSetKernelArg(kernel_1, 2, sizeof(unsigned int), (void*)(&nx));
	err = clSetKernelArg(kernel_1, 3, sizeof(unsigned int), (void*)(&ny));
	//kernel 2
	err = clSetKernelArg(kernel_2, 0, sizeof(cl_mem), &mask_buffer);
	err = clSetKernelArg(kernel_2, 1, sizeof(cl_mem), &label_buffer);
	err = clSetKernelArg(kernel_2, 2, sizeof(unsigned int), (void*)(&nx));
	err = clSetKernelArg(kernel_2, 3, sizeof(unsigned int), (void*)(&ny));
	err = clSetKernelArg(kernel_2, 4, sizeof(unsigned int), (void*)(&size_lm));

	size_t count = 0;
	for (size_t i = 0; i<nz; ++i)
	{
		if (i)
			err = clEnqueueWriteBuffer(
				queue, label_buffer,
				CL_TRUE, 0, sizeof(unsigned int)*nx*ny,
				cur_page_label, 0, NULL, NULL);

		for (int j = 0; j<iter; ++j)
		{
			err = clEnqueueWriteBuffer(
				queue, mask_buffer,
				CL_TRUE, 0, sizeof(unsigned int)*nx*ny,
				mask32, 0, NULL, NULL);
			err = clEnqueueNDRangeKernel(queue, kernel_0, 2, NULL, global_size,
				local_size, 0, NULL, NULL);
			err = clEnqueueNDRangeKernel(queue, kernel_1, 2, NULL, global_size,
				local_size, 0, NULL, NULL);
			err = clEnqueueNDRangeKernel(queue, kernel_2, 2, NULL, global_size,
				local_size, 0, NULL, NULL);

			count++;
			if (count == nz - 1)
			{
				count = 0;
				m_sig_progress();
			}
		}

		clFinish(queue);
		err = clEnqueueReadBuffer(
			queue, label_buffer,
			CL_TRUE, 0, sizeof(unsigned int)*nx*ny,
			cur_page_label, 0, NULL, NULL);
		cur_page_label += nx*ny;
	}

	clReleaseMemObject(label_buffer);
	clReleaseMemObject(mask_buffer);
	clReleaseKernel(kernel_0);
	clReleaseKernel(kernel_1);
	clReleaseKernel(kernel_2);
	clReleaseProgram(program);
	clReleaseCommandQueue(queue);
	delete[]mask32;
}

void ComponentGenerator::MatchSlices_CPU(bool backwards, unsigned int size_thresh,
	float size_ratio, float dist_thresh, float angle_thresh)
{
	GET_VOLDATA

	if (nz < 2) return;
	float sscale = m_vd->GetScalarScale();

	CellList cell_list1, cell_list2;
	unsigned int* page1 = val32;
	if (backwards) page1 = val32 + nx*ny*(nz - 1);
	unsigned int* page2 = val32 + nx*ny;
	if (backwards) page2 = val32 + nx*ny*(nz - 2);
	void* page1_data = bits==8?((void*)val8):((void*)val16);
	if (backwards) page1_data = bits == 8 ? ((void*)(val8 + nx*ny*(nz - 1))) : ((void*)(val16 + nx*ny*(nz - 1)));
	void* page2_data = bits==8?((void*)(val8 + nx*ny)):((void*)(val16 + nx*ny));
	if (backwards) page2_data = bits == 8 ? ((void*)(val8 + nx*ny*(nz - 2))) : ((void*)(val16 + nx*ny*(nz - 2)));
	unsigned int index;
	unsigned int label_value1, label_value2;
	float data_value1, data_value2;
	CellListIter iter;
	Cell *cell1, *cell2;
	ReplaceList rep_list;
	ReplaceListIter rep_iter;
	float size1, size2, size_ol;
	float vx1, vy1, vx2, vy2, d1, d2;
	//
	for (size_t i = backwards?nz:0;
		backwards?(i>1):(i<nz - 1);
		backwards?(--i):(++i))
	{
		InitializeCellList(page1, page1_data, bits, sscale, nx, ny, &cell_list1);
		InitializeCellList(page2, page2_data, bits, sscale, nx, ny, &cell_list2);
		//calculate overlap
		for (size_t ii = 0; ii<nx; ++ii)
		for (size_t jj = 0; jj<ny; ++jj)
		{
			index = nx*jj + ii;
			label_value1 = page1[index];
			label_value2 = page2[index];
			if (bits == 8)
			{
				data_value1 = ((unsigned char*)page1_data)[index] / 255.0f;
				data_value2 = ((unsigned char*)page2_data)[index] / 255.0f;
			}
			else if (bits == 16)
			{
				data_value1 = ((unsigned short*)page1_data)[index] / 65535.0f * sscale;
				data_value2 = ((unsigned short*)page2_data)[index] / 65535.0f * sscale;
			}
			if (label_value1 && label_value2)
			{
				iter = cell_list1.find(label_value1);
				if (iter != cell_list1.end())
				{
					cell1 = iter->second;
					iter = cell_list2.find(label_value2);
					if (iter == cell_list2.end())
						continue;
					cell2 = iter->second;
					bool found_edge = false;
					for (size_t kk = 0; kk<cell1->edges.size(); ++kk)
					{
						if (cell1->edges[kk]->cell2 == cell2)
						{
							cell1->edges[kk]->x = (cell1->edges[kk]->x *
								cell1->edges[kk]->sizei + ii) /
								(cell1->edges[kk]->sizei + 1);
							cell1->edges[kk]->y = (cell1->edges[kk]->y *
								cell1->edges[kk]->sizei + jj) /
								(cell1->edges[kk]->sizei + 1);
							cell1->edges[kk]->sizei++;
							cell1->edges[kk]->size +=
								min(data_value1, data_value2);
							found_edge = true;
						}
					}
					if (!found_edge)
					{
						Edge *edge = new Edge();
						edge->cell1 = cell1;
						edge->cell2 = cell2;
						edge->x = ii;
						edge->y = jj;
						edge->sizei = 1;
						edge->size = min(data_value1, data_value2);
						cell1->edges.push_back(edge);
					}
				}
			}
		}
		//build replacing list
		for (iter = cell_list1.begin(); iter != cell_list1.end(); ++iter)
		{
			cell1 = iter->second;
			if (cell1->edges.empty())
				continue;
			if (cell1->size <= size_thresh)
				continue;
			//sort
			if (cell1->edges.size() > 1)
				std::sort(cell1->edges.begin(),
					cell1->edges.end(), sort_ol);
			cell2 = cell1->edges[0]->cell2;
			if (cell2->size <= size_thresh)
				continue;
			size1 = cell1->size;
			size2 = cell2->size;
			size_ol = cell1->edges[0]->size;
			if (size_ol / size1 < size_ratio &&
				size_ol / size2 < size_ratio)
				continue;
			vx1 = cell1->x - cell1->edges[0]->x;
			vy1 = cell1->y - cell1->edges[0]->y;
			vx2 = cell2->x - cell1->edges[0]->x;
			vy2 = cell2->y - cell1->edges[0]->y;
			d1 = sqrt(vx1 * vx1 + vy1 * vy1);
			vx1 /= d1; vy1 /= d1;
			d2 = sqrt(vx2 * vx2 + vy2 * vy2);
			vx2 /= d2; vy2 /= d2;
			if (d1 > dist_thresh && d2 > dist_thresh &&
				fabs(vx1 * vx2 + vy1 * vy2) < angle_thresh)
				continue;
			rep_iter = rep_list.find(cell2->id);
			if (rep_iter != rep_list.end())
			{
				if (size_ol > rep_iter->second->size)
					rep_list.erase(rep_iter);
			}
			rep_list.insert(pair<unsigned int, Edge*>
				(cell2->id, cell1->edges[0]));
		}
		//replace
		for (size_t ii = 0; ii<nx; ++ii)
			for (size_t jj = 0; jj<ny; ++jj)
			{
				index = nx*jj + ii;
				label_value2 = page2[index];
				rep_iter = rep_list.find(label_value2);
				if (rep_iter != rep_list.end())
					page2[index] = rep_iter->second->cell1->id;
			}
		ClearCellList(&cell_list1);
		ClearCellList(&cell_list2);
		rep_list.clear();
		if (backwards)
		{
			page1 -= nx*ny;
			page2 -= nx*ny;
			if (bits == 8)
			{
				page1_data = (void*)(((unsigned char*)page1_data) - nx*ny);
				page2_data = (void*)(((unsigned char*)page2_data) - nx*ny);
			}
			else
			{
				page1_data = (void*)(((unsigned short*)page1_data) - nx*ny);
				page2_data = (void*)(((unsigned short*)page2_data) - nx*ny);
			}
		}
		else
		{
			page1 += nx*ny;
			page2 += nx*ny;
			if (bits == 8)
			{
				page1_data = (void*)(((unsigned char*)page1_data) + nx*ny);
				page2_data = (void*)(((unsigned char*)page2_data) + nx*ny);
			}
			else
			{
				page1_data = (void*)(((unsigned short*)page1_data) + nx*ny);
				page2_data = (void*)(((unsigned short*)page2_data) + nx*ny);
			}
		}
	}

	m_sig_progress();
}

