#include "KernelProgram.h"
#include <Windows.h>

namespace FLIVR
{
	bool KernelProgram::init_ = false;
	cl_device_id KernelProgram::device_ = 0;
	cl_context KernelProgram::context_ = 0;

	KernelProgram::KernelProgram(const std::string& source) :
	source_(source), program_(0), kernel_(0), queue_(0)
	{
	}

	KernelProgram::~KernelProgram()
	{
	}

	void KernelProgram::init_kernels_supported()
	{
		if (init_)
			return;

		cl_int err;
		cl_platform_id platform;

		err = clGetPlatformIDs(1, &platform, NULL);
		if (err < 0)
			return;

		err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device_, NULL);
		if (err < 0)
			return;

		cl_context_properties properties[] =
		{
			CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
			CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
			CL_CONTEXT_PLATFORM, (cl_context_properties)platform,
			0
		};
		context_ = clCreateContext(properties, 1, &device_, NULL, NULL, &err);
		if (err < 0)
			return;

		init_ = true;
	}

	bool KernelProgram::init()
	{
		return init_;
	}

	void KernelProgram::clear()
	{
		clReleaseContext(context_);
		init_ = false;
	}

	bool KernelProgram::create()
	{
		cl_int err;
		const char *c_source[1];
		c_source[0] = source_.c_str();
		size_t program_size = source_.size();
		program_ = clCreateProgramWithSource(context_, 1,
			c_source, &program_size, &err);
		if (err < 0)
		{
			return false;
		}

		err = clBuildProgram(program_, 0, NULL, NULL, NULL, NULL);
		if (err < 0)
		{
			char *program_log;
			size_t log_size;
			clGetProgramBuildInfo(program_, device_, CL_PROGRAM_BUILD_LOG,
				0, NULL, &log_size);
			program_log = new char[log_size+1];
			program_log[log_size] = '\0';
			clGetProgramBuildInfo(program_, device_, CL_PROGRAM_BUILD_LOG,
				log_size+1, program_log, NULL);
			delete []program_log;
			return false;
		}

		kernel_ = clCreateKernel(program_, "add_numbers", &err);
		if (err < 0)
		{
			return false;
		}

		queue_ = clCreateCommandQueue(context_, device_, 0, &err);
		if (err < 0)
		{
			return false;
		}

		return true;
	}

	bool KernelProgram::valid()
	{
		return init_ && program_ && kernel_ && queue_;
	}

	void KernelProgram::destroy()
	{
		clReleaseKernel(kernel_);
		for (unsigned int i=0; i<arg_list_.size(); ++i)
			clReleaseMemObject(arg_list_[i].buffer);
		clReleaseCommandQueue(queue_);
		clReleaseProgram(program_);
	}

	void KernelProgram::execute(cl_uint dim, size_t global_size, size_t local_size)
	{
		if (!valid())
			return;

		cl_int err;
		glFinish();
		unsigned int i;
		for (i=0; i<arg_list_.size(); ++i)
		{
			if (arg_list_[i].size == 0)
			{
				err = clEnqueueAcquireGLObjects(queue_, 1, &(arg_list_[i].buffer), 0, NULL, NULL);
				if (err < 0)
					return;
			}
		}
		err = clEnqueueNDRangeKernel(queue_, kernel_, dim, NULL, &global_size,
			&local_size, 0, NULL, NULL);
		if (err < 0)
			return;
		for (i=0; i<arg_list_.size(); ++i)
		{
			if (arg_list_[i].size == 0)
			{
				err = clEnqueueReleaseGLObjects(queue_, 1, &(arg_list_[i].buffer), 0, NULL, NULL);
				if (err < 0)
					return;
			}
		}
		clFinish(queue_);
	}

	void KernelProgram::setKernelArg(int i, cl_mem_flags flag, size_t size, void* data)
	{
		cl_int err;

		if (data)
		{
			cl_mem buffer = clCreateBuffer(context_, flag, size, data, &err);
			if (err < 0)
				return;
			Argument arg;
			arg.index = i;
			arg.size = size;
			arg.buffer = buffer;
			arg_list_.push_back(arg);
			err = clSetKernelArg(kernel_, i, sizeof(cl_mem), &buffer);
			if (err < 0)
				return;
		}
		else
		{
			err = clSetKernelArg(kernel_, i, size, NULL);
		}
	}

	void KernelProgram::setKernelArgTex2D(int i, cl_mem_flags flag, GLenum texture_target, GLuint texture)
	{
		cl_int err;
		cl_mem tex_buffer = clCreateFromGLTexture2D(context_, flag, texture_target, 0, texture, &err);
		if (err < 0)
			return;
		Argument arg;
		arg.index = i;
		arg.size = 0;
		arg.buffer = tex_buffer;
		arg_list_.push_back(arg);
		err = clSetKernelArg(kernel_, i, sizeof(cl_mem), &tex_buffer);
		if (err < 0)
			return;
	}

	void KernelProgram::setKernelArgTex3D(int i, cl_mem_flags flag, GLenum texture_target, GLuint texture)
	{
		cl_int err;
		cl_mem tex_buffer = clCreateFromGLTexture3D(context_, flag, texture_target, 0, texture, &err);
		if (err < 0)
			return;
		Argument arg;
		arg.index = i;
		arg.size = 0;
		arg.buffer = tex_buffer;
		arg_list_.push_back(arg);
		err = clSetKernelArg(kernel_, i, sizeof(cl_mem), &tex_buffer);
		if (err < 0)
			return;
	}

	void KernelProgram::readBuffer(int index, void* data)
	{
		bool found = false;
		unsigned int i;
		for (i=0; i<arg_list_.size(); ++i)
		{
			if (arg_list_[i].index == index)
			{
				found = true;
				break;
			}
		}
		if (found)
		{
			Argument arg = arg_list_[i];
			cl_int err;
			err = clEnqueueReadBuffer(queue_, arg.buffer, CL_TRUE, 0,
				arg.size, data, 0, NULL, NULL);
			if (err < 0)
				return;
		}
	}
}