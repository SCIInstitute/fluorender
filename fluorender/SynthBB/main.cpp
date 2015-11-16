#ifdef _WIN32
#include <CL/cl.h>
#endif
#ifdef _DARWIN
#include <OpenCL/cl.h>
#endif
#include <vector>
#include <algorithm>
#include <boost/unordered_map.hpp>
#include "tif_reader.h"
#include "msk_writer.h"
#include "cl_code.h"
//#include <vld.h>

using namespace std;

cl_device_id device_;
cl_context context_;

double spcx = 1.0;
double spcy = 1.0;
double spcz = 3.0;
int tempo = 0;

bool Init()
{
	cl_int err;
	cl_platform_id platform;

	err = clGetPlatformIDs(1, &platform, NULL);
	if (err != CL_SUCCESS)
		return false;

	cl_device_id devices[2];
	cl_uint dev_num = 0;
	err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 2, devices, &dev_num);
	if (err != CL_SUCCESS)
		return false;
	device_ = devices[0];
	char buffer[10240];
	clGetDeviceInfo(device_, CL_DEVICE_NAME, sizeof(buffer), buffer, NULL);

	cl_context_properties properties[] = 
	{
		CL_CONTEXT_PLATFORM,
		(cl_context_properties)platform,
		0
	};
	context_ = clCreateContext(properties, 1, &device_, NULL, NULL, &err);
	if (err != CL_SUCCESS)
		return false;

	return true;
}

void Uninit()
{
	clReleaseContext(context_);
}

Nrrd* ReadData(string &filename)
{
	TIFReader reader;
	
	reader.SetFile(filename);
	reader.SetSliceSeq(false);
	wstring time_id = L"NA!@";
	reader.SetTimeId(time_id);
	reader.Preprocess();
	return reader.Convert(tempo, 0, true);
}

inline unsigned int reverse_bit(unsigned int val, unsigned int len)
{
	unsigned int res = val;
	int s = len - 1;

	for (val >>= 1; val; val >>= 1)
	{
		res <<= 1;
		res |= val & 1;
		s--;
	}
	res <<= s;
	res <<= 32-len;
	res >>= 32-len;
	return res;
}

inline size_t Max(size_t d1, size_t d2)
{
	return d1>d2?d1:d2;
}

void SetShuffledID(unsigned int* val32,
	unsigned char* val8,
	size_t nx, size_t ny, size_t nz)
{
	unsigned int x, y, z;
	unsigned int res;
	unsigned int len = 0;
	unsigned int r = Max(nx, Max(ny, nz));
	while (r > 0)
	{
		r /= 2;
		len++;
	}
	for (int i=0; i<nx; i++)
	for (int j=0; j<ny; j++)
	for (int k=0; k<nz; k++)
	{
		unsigned int index = nx*ny*k + nx*j + i;
		if (val8[index] == 0)
			val32[index] = 0;
		else if (i<1 || i>nx-2 ||
				j<1 || j>ny-2)
			val32[index] = 0;
		else
		{
			x = reverse_bit(i, len);
			y = reverse_bit(j, len);
			z = reverse_bit(k, len);
			res = 0;
			for (unsigned int ii=0; ii<len; ii++)
			{
				res |= (1<<ii & x)<<(2*ii);
				res |= (1<<ii & y)<<(2*ii+1);
				res |= (1<<ii & z)<<(2*ii+2);
			}
			val32[index] = nx*ny*nz - res;
		}
	}
}

Nrrd* label = 0;
Nrrd* cdata = 0;
size_t nx, ny, nz;

void ShuffleID_2D()
{
	unsigned char* val8 = (unsigned char*)(cdata->data);
	unsigned int* val32 = (unsigned int*)(label->data);

	cl_int err;
	size_t program_size = strlen(str_cl_shuffle_id_2d);
	cl_program program = clCreateProgramWithSource(context_, 1, 
		&str_cl_shuffle_id_2d, &program_size, &err);
	if (err != CL_SUCCESS) return;
	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (err != CL_SUCCESS)
	{
		char *program_log;
		size_t log_size;
		clGetProgramBuildInfo(program, device_, CL_PROGRAM_BUILD_LOG,
			0, NULL, &log_size);
		program_log = new char[log_size+1];
		program_log[log_size] = '\0';
		clGetProgramBuildInfo(program, device_, CL_PROGRAM_BUILD_LOG,
			log_size+1, program_log, NULL);
		cout << program_log;
		delete []program_log;
		return;
	}
	cl_kernel kernel = clCreateKernel(program, "kernel_0", &err);
	cl_command_queue queue = clCreateCommandQueue(context_, device_, 0, &err);

	size_t global_size[3] = {nx, ny, nz};
	size_t local_size[3] = {1, 1, 1};

	//data
	cl_image_format image_format;
	cl_image_desc image_desc;
	image_format.image_channel_order = CL_R;
	image_format.image_channel_data_type = CL_UNORM_INT8;
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
	cl_mem data_buffer = clCreateImage(context_, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,
		&image_format, &image_desc, val8, &err);
	//label
	cl_mem label_buffer = clCreateBuffer(context_, CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR,
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
}

void ShuffleID_3D()
{
	unsigned char* val8 = (unsigned char*)(cdata->data);
	unsigned int* val32 = (unsigned int*)(label->data);

	cl_int err;
	size_t program_size = strlen(str_cl_shuffle_id_3d);
	cl_program program = clCreateProgramWithSource(context_, 1, 
		&str_cl_shuffle_id_3d, &program_size, &err);
	if (err != CL_SUCCESS) return;
	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (err != CL_SUCCESS)
	{
		char *program_log;
		size_t log_size;
		clGetProgramBuildInfo(program, device_, CL_PROGRAM_BUILD_LOG,
			0, NULL, &log_size);
		program_log = new char[log_size+1];
		program_log[log_size] = '\0';
		clGetProgramBuildInfo(program, device_, CL_PROGRAM_BUILD_LOG,
			log_size+1, program_log, NULL);
		cout << program_log;
		delete []program_log;
		return;
	}
	cl_kernel kernel = clCreateKernel(program, "kernel_0", &err);
	cl_command_queue queue = clCreateCommandQueue(context_, device_, 0, &err);

	size_t global_size[3] = {nx, ny, nz};
	size_t local_size[3] = {1, 1, 1};
	unsigned int len = 0;
	unsigned int r = Max(nx, Max(ny, nz));
	while (r > 0)
	{
		r /= 2;
		len++;
	}

	//data
	cl_image_format image_format;
	cl_image_desc image_desc;
	image_format.image_channel_order = CL_R;
	image_format.image_channel_data_type = CL_UNORM_INT8;
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
	cl_mem data_buffer = clCreateImage(context_, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,
		&image_format, &image_desc, val8, &err);
	//label
	cl_mem label_buffer = clCreateBuffer(context_, CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR,
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
}

void Brainbow()
{
	unsigned char* val8 = (unsigned char*)(cdata->data);
	unsigned int* val32 = (unsigned int*)(label->data);

	cl_int err;
	size_t program_size = strlen(str_cl_slice_brainbow);
	cl_program program = clCreateProgramWithSource(context_, 1, 
		&str_cl_slice_brainbow, &program_size, &err);
	if (err != CL_SUCCESS) return;
	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (err != CL_SUCCESS)
	{
		char *program_log;
		size_t log_size;
		clGetProgramBuildInfo(program, device_, CL_PROGRAM_BUILD_LOG,
			0, NULL, &log_size);
		program_log = new char[log_size+1];
		program_log[log_size] = '\0';
		clGetProgramBuildInfo(program, device_, CL_PROGRAM_BUILD_LOG,
			log_size+1, program_log, NULL);
		cout << program_log;
		delete []program_log;
		return;
	}
	cl_kernel kernel = clCreateKernel(program, "kernel_0", &err);
	cl_command_queue queue = clCreateCommandQueue(context_, device_, 0, &err);

	int iter = 60;
	unsigned char* cur_page_data = val8;
	unsigned int* cur_page_label = val32;
	unsigned int rcnt = 0;
	unsigned int seed = 11;
	float value_t = 1.0f;
	float value_f = 0.15f;
	float grad_f = 0.1f;
	float vv_f = 0.2f;
	float av_f = 0.2f;
	size_t global_size[2] = {nx, ny};
	size_t local_size[2] = {1, 1};

	//data
	cl_image_format image_format;
	cl_image_desc image_desc;
	image_format.image_channel_order = CL_R;
	image_format.image_channel_data_type = CL_UNORM_INT8;
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
	cl_mem data_buffer = clCreateImage(context_, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,
		&image_format, &image_desc, cur_page_data, &err);
	//label
	cl_mem label_buffer = clCreateBuffer(context_, CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR,
		sizeof(unsigned int)*nx*ny, cur_page_label, &err);
	//counter
	cl_mem rcnt_buffer = clCreateBuffer(context_, CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR,
		sizeof(unsigned int), &rcnt, &err);
	//set
	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &data_buffer);
	err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &label_buffer);
	err = clSetKernelArg(kernel, 2, sizeof(unsigned int), (void*)(&nx));
	err = clSetKernelArg(kernel, 3, sizeof(unsigned int), (void*)(&ny));
	err = clSetKernelArg(kernel, 4, sizeof(cl_mem), &rcnt_buffer);
	err = clSetKernelArg(kernel, 5, sizeof(unsigned int), (void*)(&seed));
	err = clSetKernelArg(kernel, 6, sizeof(float), (void*)(&value_t));
	err = clSetKernelArg(kernel, 7, sizeof(float), (void*)(&value_f));
	err = clSetKernelArg(kernel, 8, sizeof(float), (void*)(&grad_f));
	err = clSetKernelArg(kernel, 9, sizeof(float), (void*)(&vv_f));
	err = clSetKernelArg(kernel, 10, sizeof(float), (void*)(&av_f));

	const size_t origin[] = {0, 0, 0};
	const size_t region[] = {nx, ny, 1};
	for (size_t i=0; i<nz; ++i)
	{
		if (i)
		{
			err = clEnqueueWriteImage(
				queue, data_buffer,
				CL_TRUE, origin, region,
				0, 0, cur_page_data, 0, NULL, NULL);
			err = clEnqueueWriteBuffer(
				queue, label_buffer,
				CL_TRUE, 0, sizeof(unsigned int)*nx*ny,
				cur_page_label, 0, NULL, NULL);
		}

		for (int j=0; j<iter; ++j)
			err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global_size,
				local_size, 0, NULL, NULL);

		clFinish(queue);
		err = clEnqueueReadBuffer(
			queue, label_buffer,
			CL_TRUE, 0, sizeof(unsigned int)*nx*ny,
			cur_page_label, 0, NULL, NULL);

		cur_page_data += nx*ny;
		cur_page_label += nx*ny;
	}

	clReleaseMemObject(data_buffer);
	clReleaseMemObject(label_buffer);
	clReleaseMemObject(rcnt_buffer);
	clReleaseKernel(kernel);
	clReleaseCommandQueue(queue);
	clReleaseProgram(program);
}

void GrowSize()
{
	unsigned char* val8 = (unsigned char*)(cdata->data);
	unsigned int* val32 = (unsigned int*)(label->data);
	unsigned int* mask32 = new unsigned int[nx*ny];
	memset(mask32, 0, sizeof(unsigned int)*nx*ny);

	cl_int err;
	size_t program_size = strlen(str_cl_grow_size);
	cl_program program = clCreateProgramWithSource(context_, 1, 
		&str_cl_grow_size, &program_size, &err);
	if (err != CL_SUCCESS) return;
	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (err != CL_SUCCESS)
	{
		char *program_log;
		size_t log_size;
		clGetProgramBuildInfo(program, device_, CL_PROGRAM_BUILD_LOG,
			0, NULL, &log_size);
		program_log = new char[log_size+1];
		program_log[log_size] = '\0';
		clGetProgramBuildInfo(program, device_, CL_PROGRAM_BUILD_LOG,
			log_size+1, program_log, NULL);
		cout << program_log;
		delete []program_log;
		return;
	}
	cl_command_queue queue = clCreateCommandQueue(context_, device_, 0, &err);
	cl_kernel kernel_0 = clCreateKernel(program, "kernel_0", &err);
	cl_kernel kernel_1 = clCreateKernel(program, "kernel_1", &err);
	cl_kernel kernel_2 = clCreateKernel(program, "kernel_2", &err);

	int iter = 40;
	unsigned char* cur_page_data = val8;
	unsigned int* cur_page_label = val32;
	unsigned int rcnt = 0;
	unsigned int seed = 11;
	float value_t = 0.5f;
	float value_f = 0.25f;
	float grad_f = 0.25f;
	float vv_f = 0.35f;
	float av_f = 0.35f;
	unsigned int thresh = 20;
	size_t global_size[2] = {nx, ny};
	size_t local_size[2] = {1, 1};

	//data
	cl_image_format image_format;
	cl_image_desc image_desc;
	image_format.image_channel_order = CL_R;
	image_format.image_channel_data_type = CL_UNORM_INT8;
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
	cl_mem data_buffer = clCreateImage(context_, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,
		&image_format, &image_desc, cur_page_data, &err);
	//mask
	cl_mem mask_buffer = clCreateBuffer(context_, CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR,
		sizeof(unsigned int)*nx*ny, mask32, &err);
	//label
	cl_mem label_buffer = clCreateBuffer(context_, CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR,
		sizeof(unsigned int)*nx*ny, cur_page_label, &err);
	//counter
	cl_mem rcnt_buffer = clCreateBuffer(context_, CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR,
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
	err = clSetKernelArg(kernel_2, 7, sizeof(float), (void*)(&value_t));
	err = clSetKernelArg(kernel_2, 8, sizeof(float), (void*)(&value_f));
	err = clSetKernelArg(kernel_2, 9, sizeof(float), (void*)(&grad_f));
	err = clSetKernelArg(kernel_2, 10, sizeof(float), (void*)(&vv_f));
	err = clSetKernelArg(kernel_2, 11, sizeof(float), (void*)(&av_f));
	err = clSetKernelArg(kernel_2, 12, sizeof(unsigned int), (void*)(&thresh));

	const size_t origin[] = {0, 0, 0};
	const size_t region[] = {nx, ny, 1};
	for (size_t i=0; i<nz; ++i)
	{
		if (i)
		{
			err = clEnqueueWriteImage(
				queue, data_buffer,
				CL_TRUE, origin, region,
				0, 0, cur_page_data, 0, NULL, NULL);
			err = clEnqueueWriteBuffer(
				queue, label_buffer,
				CL_TRUE, 0, sizeof(unsigned int)*nx*ny,
				cur_page_label, 0, NULL, NULL);
		}

		for (int j=0; j<iter; ++j)
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
		}

		clFinish(queue);
		err = clEnqueueReadBuffer(
			queue, label_buffer,
			CL_TRUE, 0, sizeof(unsigned int)*nx*ny,
			cur_page_label, 0, NULL, NULL);

		cur_page_data += nx*ny;
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
	delete []mask32;
}

void CleanUp()
{
	unsigned int* val32 = (unsigned int*)(label->data);
	unsigned int* mask32 = new unsigned int[nx*ny];
	memset(mask32, 0, sizeof(unsigned int)*nx*ny);

	cl_int err;
	cl_command_queue queue = clCreateCommandQueue(context_, device_, 0, &err);

	size_t program_size = strlen(str_cl_grow_size);
	cl_program program = clCreateProgramWithSource(context_, 1, 
		&str_cl_grow_size, &program_size, &err);
	if (err != CL_SUCCESS) return;
	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (err != CL_SUCCESS)
	{
		char *program_log;
		size_t log_size;
		clGetProgramBuildInfo(program, device_, CL_PROGRAM_BUILD_LOG,
			0, NULL, &log_size);
		program_log = new char[log_size+1];
		program_log[log_size] = '\0';
		clGetProgramBuildInfo(program, device_, CL_PROGRAM_BUILD_LOG,
			log_size+1, program_log, NULL);
		cout << program_log;
		delete []program_log;
		return;
	}
	cl_kernel kernel_0 = clCreateKernel(program, "kernel_0", &err);
	cl_kernel kernel_1 = clCreateKernel(program, "kernel_1", &err);
	clReleaseProgram(program);

	program_size = strlen(str_cl_clean_up);
	program = clCreateProgramWithSource(context_, 1, 
		&str_cl_clean_up, &program_size, &err);
	if (err != CL_SUCCESS) return;
	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (err != CL_SUCCESS)
	{
		char *program_log;
		size_t log_size;
		clGetProgramBuildInfo(program, device_, CL_PROGRAM_BUILD_LOG,
			0, NULL, &log_size);
		program_log = new char[log_size+1];
		program_log[log_size] = '\0';
		clGetProgramBuildInfo(program, device_, CL_PROGRAM_BUILD_LOG,
			log_size+1, program_log, NULL);
		cout << program_log;
		delete []program_log;
		return;
	}
	cl_kernel kernel_2 = clCreateKernel(program, "kernel_0", &err);

	int iter = 40;
	unsigned int* cur_page_label = val32;
	unsigned int thresh = 5;
	size_t global_size[2] = {nx, ny};
	size_t local_size[2] = {1, 1};

	//mask
	cl_mem mask_buffer = clCreateBuffer(context_, CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR,
		sizeof(unsigned int)*nx*ny, mask32, &err);
	//label
	cl_mem label_buffer = clCreateBuffer(context_, CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR,
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
	err = clSetKernelArg(kernel_2, 4, sizeof(unsigned int), (void*)(&thresh));

	for (size_t i=0; i<nz; ++i)
	{
		if (i)
			err = clEnqueueWriteBuffer(
				queue, label_buffer,
				CL_TRUE, 0, sizeof(unsigned int)*nx*ny,
				cur_page_label, 0, NULL, NULL);

		for (int j=0; j<iter; ++j)
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
	delete []mask32;
}

void MatchSlices()
{
	unsigned int* val32 = (unsigned int*)(label->data);
	unsigned int* mask1 = new unsigned int[nx*ny];
	memset(mask1, 0, sizeof(unsigned int)*nx*ny);
	unsigned int* mask2 = new unsigned int[nx*ny];
	memset(mask2, 0, sizeof(unsigned int)*nx*ny);
	unsigned int* mask_and = new unsigned int[nx*ny];
	memset(mask_and, 0, sizeof(unsigned int)*nx*ny);
	bool* flag = new bool[nx*ny];
	memset(flag, 0, sizeof(bool)*nx*ny);

	cl_int err;
	cl_command_queue queue = clCreateCommandQueue(context_, device_, 0, &err);

	size_t program_size = strlen(str_cl_grow_size);
	cl_program program = clCreateProgramWithSource(context_, 1, 
		&str_cl_grow_size, &program_size, &err);
	if (err != CL_SUCCESS) return;
	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (err != CL_SUCCESS)
	{
		char *program_log;
		size_t log_size;
		clGetProgramBuildInfo(program, device_, CL_PROGRAM_BUILD_LOG,
			0, NULL, &log_size);
		program_log = new char[log_size+1];
		program_log[log_size] = '\0';
		clGetProgramBuildInfo(program, device_, CL_PROGRAM_BUILD_LOG,
			log_size+1, program_log, NULL);
		cout << program_log;
		delete []program_log;
		return;
	}
	cl_kernel kernel_0 = clCreateKernel(program, "kernel_0", &err);
	cl_kernel kernel_1 = clCreateKernel(program, "kernel_1", &err);
	clReleaseProgram(program);

	program_size = strlen(str_cl_match_slices);
	program = clCreateProgramWithSource(context_, 1, 
		&str_cl_match_slices, &program_size, &err);
	if (err != CL_SUCCESS) return;
	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (err != CL_SUCCESS)
	{
		char *program_log;
		size_t log_size;
		clGetProgramBuildInfo(program, device_, CL_PROGRAM_BUILD_LOG,
			0, NULL, &log_size);
		program_log = new char[log_size+1];
		program_log[log_size] = '\0';
		clGetProgramBuildInfo(program, device_, CL_PROGRAM_BUILD_LOG,
			log_size+1, program_log, NULL);
		cout << program_log;
		delete []program_log;
		return;
	}
	cl_kernel kernel_2 = clCreateKernel(program, "kernel_0", &err);
	cl_kernel kernel_3 = clCreateKernel(program, "kernel_1", &err);

	unsigned int* cur_page_label = val32;
	unsigned int thresh = 5;
	size_t global_size[2] = {nx, ny};
	size_t local_size[2] = {1, 1};

	//masks
	cl_mem mask1_buffer = clCreateBuffer(context_, CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR,
		sizeof(unsigned int)*nx*ny, mask1, &err);
	cl_mem mask2_buffer = clCreateBuffer(context_, CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR,
		sizeof(unsigned int)*nx*ny, mask2, &err);
	cl_mem mask_and_buffer = clCreateBuffer(context_, CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR,
		sizeof(unsigned int)*nx*ny, mask_and, &err);
	//label
	cl_mem label1_buffer = clCreateBuffer(context_, CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR,
		sizeof(unsigned int)*nx*ny, cur_page_label, &err);
	cl_mem label2_buffer = clCreateBuffer(context_, CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR,
		sizeof(unsigned int)*nx*ny, cur_page_label+nx*ny, &err);
	//flag
	cl_mem flag_buffer = clCreateBuffer(context_, CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR,
		sizeof(bool)*nx*ny, flag, &err);

	//common settings
	//kernel_0
	err = clSetKernelArg(kernel_0, 2, sizeof(unsigned int), (void*)(&nx));
	err = clSetKernelArg(kernel_0, 3, sizeof(unsigned int), (void*)(&ny));
	//kernel_1
	err = clSetKernelArg(kernel_1, 2, sizeof(unsigned int), (void*)(&nx));
	err = clSetKernelArg(kernel_1, 3, sizeof(unsigned int), (void*)(&ny));
	//kernel_2
	err = clSetKernelArg(kernel_2, 0, sizeof(cl_mem), &mask_and_buffer);
	err = clSetKernelArg(kernel_2, 1, sizeof(cl_mem), &label1_buffer);
	err = clSetKernelArg(kernel_2, 2, sizeof(cl_mem), &label2_buffer);
	err = clSetKernelArg(kernel_2, 3, sizeof(cl_mem), &flag_buffer);
	err = clSetKernelArg(kernel_2, 4, sizeof(unsigned int), (void*)(&nx));
	err = clSetKernelArg(kernel_2, 5, sizeof(unsigned int), (void*)(&ny));
	//kernel_3
	err = clSetKernelArg(kernel_3, 0, sizeof(cl_mem), &mask1_buffer);
	err = clSetKernelArg(kernel_3, 1, sizeof(cl_mem), &mask2_buffer);
	err = clSetKernelArg(kernel_3, 2, sizeof(cl_mem), &mask_and_buffer);
	err = clSetKernelArg(kernel_3, 3, sizeof(cl_mem), &label1_buffer);
	err = clSetKernelArg(kernel_3, 4, sizeof(cl_mem), &label2_buffer);
	err = clSetKernelArg(kernel_3, 5, sizeof(unsigned int), (void*)(&nx));
	err = clSetKernelArg(kernel_3, 6, sizeof(unsigned int), (void*)(&ny));
	err = clSetKernelArg(kernel_3, 7, sizeof(unsigned int), (void*)(&thresh));

	for (size_t i=0; i<nz-1; ++i)
	{
		if (i)
		{
			err = clEnqueueWriteBuffer(
				queue, mask1_buffer,
				CL_TRUE, 0, sizeof(unsigned int)*nx*ny,
				mask1, 0, NULL, NULL);
			err = clEnqueueWriteBuffer(
				queue, label1_buffer,
				CL_TRUE, 0, sizeof(unsigned int)*nx*ny,
				cur_page_label, 0, NULL, NULL);
			err = clEnqueueWriteBuffer(
				queue, mask2_buffer,
				CL_TRUE, 0, sizeof(unsigned int)*nx*ny,
				mask2, 0, NULL, NULL);
			err = clEnqueueWriteBuffer(
				queue, label2_buffer,
				CL_TRUE, 0, sizeof(unsigned int)*nx*ny,
				cur_page_label+nx*ny, 0, NULL, NULL);
			err = clEnqueueWriteBuffer(
				queue, mask_and_buffer,
				CL_TRUE, 0, sizeof(unsigned int)*nx*ny,
				mask_and, 0, NULL, NULL);
			err = clEnqueueWriteBuffer(
				queue, flag_buffer,
				CL_TRUE, 0, sizeof(bool)*nx*ny,
				flag, 0, NULL, NULL);
		}

		//first slice
		//kernel 0
		err = clSetKernelArg(kernel_0, 0, sizeof(cl_mem), &mask1_buffer);
		err = clSetKernelArg(kernel_0, 1, sizeof(cl_mem), &label1_buffer);
		//kernel 1
		err = clSetKernelArg(kernel_1, 0, sizeof(cl_mem), &mask1_buffer);
		err = clSetKernelArg(kernel_1, 1, sizeof(cl_mem), &label1_buffer);
		err = clEnqueueNDRangeKernel(queue, kernel_0, 2, NULL, global_size,
			local_size, 0, NULL, NULL);
		err = clEnqueueNDRangeKernel(queue, kernel_1, 2, NULL, global_size,
			local_size, 0, NULL, NULL);

		//second slice
		//kernel 0
		err = clSetKernelArg(kernel_0, 0, sizeof(cl_mem), &mask2_buffer);
		err = clSetKernelArg(kernel_0, 1, sizeof(cl_mem), &label2_buffer);
		//kernel 1
		err = clSetKernelArg(kernel_1, 0, sizeof(cl_mem), &mask2_buffer);
		err = clSetKernelArg(kernel_1, 1, sizeof(cl_mem), &label2_buffer);
		err = clEnqueueNDRangeKernel(queue, kernel_0, 2, NULL, global_size,
			local_size, 0, NULL, NULL);
		err = clEnqueueNDRangeKernel(queue, kernel_1, 2, NULL, global_size,
			local_size, 0, NULL, NULL);

		//calc intersection sizes
		//kernel2
		err = clEnqueueNDRangeKernel(queue, kernel_2, 2, NULL, global_size,
			local_size, 0, NULL, NULL);

		//assign IDs
		//kernel3
		err = clEnqueueNDRangeKernel(queue, kernel_3, 2, NULL, global_size,
			local_size, 0, NULL, NULL);

		clFinish(queue);
		cur_page_label += nx*ny;
		err = clEnqueueReadBuffer(
			queue, label2_buffer,
			CL_TRUE, 0, sizeof(unsigned int)*nx*ny,
			cur_page_label, 0, NULL, NULL);
	}

	clReleaseMemObject(mask1_buffer);
	clReleaseMemObject(mask2_buffer);
	clReleaseMemObject(mask_and_buffer);
	clReleaseMemObject(label1_buffer);
	clReleaseMemObject(label2_buffer);
	clReleaseMemObject(flag_buffer);
	clReleaseKernel(kernel_0);
	clReleaseKernel(kernel_1);
	clReleaseKernel(kernel_2);
	clReleaseKernel(kernel_3);
	clReleaseProgram(program);
	clReleaseCommandQueue(queue);
	delete []mask1;
	delete []mask2;
	delete []mask_and;
	delete []flag;
}

struct Cell;
struct Edge
{
	Cell* cell1;
	Cell* cell2;
	unsigned int sizei;//voxel count
	float size;//overlapping size
	float x;//x of overlapping center
	float y;//y of overlapping center
};
struct Cell
{
	~Cell()
	{
		for (size_t i=0; i<edges.size(); ++i)
			delete edges[i];
	}
	unsigned int id;
	unsigned int sizei;//voxel count
	float size;
	float x;//x of cell center
	float y;//y of cell center
	vector<Edge*> edges;
};
typedef boost::unordered_map<unsigned int, Cell*> CellList;
typedef boost::unordered_map<unsigned int, Cell*>::iterator CellListIter;

typedef boost::unordered_map<unsigned int, Edge*> ReplaceList;
typedef boost::unordered_map<unsigned int, Edge*>::iterator ReplaceListIter;

void InitializeCellList(unsigned int* page, unsigned char* page_data, CellList* cell_list)
{
	unsigned int index;
	unsigned int label_value;
	float data_value;
	CellListIter iter;

	for (size_t i=0; i<nx; ++i)
	for (size_t j=0; j<ny; ++j)
	{
		index = nx*j + i;
		label_value = page[index];
		data_value = page_data[index]/255.0f;
		if (label_value)
		{
			iter = cell_list->find(label_value);
			if (iter != cell_list->end())
			{
				iter->second->x = (iter->second->x *
					iter->second->sizei + i) /
					(iter->second->sizei + 1);
				iter->second->y = (iter->second->y *
					iter->second->sizei + j) /
					(iter->second->sizei + 1);
				iter->second->sizei++;
				iter->second->size += data_value;
			}
			else
			{
				Cell* cell = new Cell();
				cell->id = label_value;
				cell->sizei = 1;
				cell->size = data_value;
				cell->x = i;
				cell->y = j;
				cell_list->insert(pair<unsigned int, Cell*>(label_value, cell));
			}
		}
	}
}

void ClearCellList(CellList* cell_list)
{
	CellListIter iter;
	for (iter=cell_list->begin(); iter!=cell_list->end(); ++iter)
		delete iter->second;
	cell_list->clear();
}

bool sort_ol(const Edge* e1, const Edge* e2)
{
	float e1_size1 = e1->cell1->size;
	float e1_size2 = e1->cell2->size;
	float e1_size_ol = e1->size;
	float e2_size1 = e2->cell1->size;
	float e2_size2 = e2->cell2->size;
	float e2_size_ol = e2->size;
	return e1_size_ol/e1_size1+e1_size_ol/e1_size2 >
		e2_size_ol/e2_size1+e2_size_ol/e2_size2;
}

void MatchSlicesCPU()
{
	unsigned int* val32 = (unsigned int*)(label->data);
	unsigned char* val8 = (unsigned char*)(cdata->data);
	CellList cell_list1, cell_list2;
	unsigned int* page1 = val32;
	unsigned int* page2 = val32+nx*ny;
	unsigned char* page1_data = val8;
	unsigned char* page2_data = val8+nx*ny;
	unsigned int index;
	unsigned int label_value1, label_value2;
	float data_value1, data_value2;
	CellListIter iter;
	Cell *cell1, *cell2;
	ReplaceList rep_list;
	ReplaceListIter rep_iter;
	unsigned int size_thresh = 25;
	float size_ratio = 0.6f;
	float dist_thresh = 2.5f;
	float angle_thresh = 0.707f;
	float size1, size2, size_ol;
	float vx1, vy1, vx2, vy2, d1, d2;
	//
	for (size_t i=0; i<nz-1; ++i)
	{
		InitializeCellList(page1, page1_data, &cell_list1);
		InitializeCellList(page2, page2_data, &cell_list2);
		//calculate overlap
		for (size_t ii=0; ii<nx; ++ii)
		for (size_t jj=0; jj<ny; ++jj)
		{
			index = nx*jj + ii;
			label_value1 = page1[index];
			label_value2 = page2[index];
			data_value1 = page1_data[index]/255.0f;
			data_value2 = page2_data[index]/255.0f;
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
					for (size_t kk=0; kk<cell1->edges.size(); ++kk)
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
		for (iter=cell_list1.begin(); iter!=cell_list1.end(); ++iter)
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
			if (size_ol/size1 < size_ratio &&
				size_ol/size2 < size_ratio)
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
		for (size_t ii=0; ii<nx; ++ii)
		for (size_t jj=0; jj<ny; ++jj)
		{
			index = nx*jj + ii;
			label_value2 = page2[index];
			rep_iter = rep_list.find(label_value2);
			if (rep_iter != rep_list.end())
				page2[index] = rep_iter->second->cell1->id;
		}
		ClearCellList(&cell_list1);
		ClearCellList(&cell_list2);
		page1 += nx*ny;
		page2 += nx*ny;
		page1_data += nx*ny;
		page2_data += nx*ny;
	}
}

void ProcessData()
{
	if (!cdata) return;

	nx = cdata->axis[0].size;
	ny = cdata->axis[1].size;
	nz = cdata->axis[2].size;

	label = nrrdNew();
	unsigned int *val32 = new unsigned int[nx*ny*nz];

	nrrdWrap(label, val32, nrrdTypeUInt, 3, nx, ny, nz);
	nrrdAxisInfoSet(label, nrrdAxisInfoSpacing, spcx, spcy, spcz);
	nrrdAxisInfoSet(label, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
	nrrdAxisInfoSet(label, nrrdAxisInfoMax, spcx*nx, spcy*ny, spcz*nz);
	nrrdAxisInfoSet(label, nrrdAxisInfoSize, nx, ny, nz);

	//ShuffleID_3D();
	ShuffleID_2D();
	Brainbow();
	GrowSize();
	CleanUp();
	if (nz > 1)
		MatchSlicesCPU();
}

bool SaveLabel(Nrrd* label, const string &filename)
{
	MSKWriter msk_writer;
	msk_writer.SetData(label);
	msk_writer.SetSpacings(spcx, spcy, spcz);
	wstring wsTmp(filename.begin(), filename.end());
	msk_writer.Save(wsTmp, 1);
	return true;
}

int main(int argc, char* argv[])
{
	if (argc < 2)
		return 1;

	if (!Init())
		return 2;

	string filename(argv[1]);
	cdata = ReadData(filename);
	if (!cdata)
		return 3;

	ProcessData();
	if (!label)
		return 4;

	if (!SaveLabel(label, filename))
		return 5;

	nrrdNuke(cdata);
	nrrdNuke(label);

	Uninit();
	return 0;
}