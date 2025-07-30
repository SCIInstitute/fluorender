//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2025 Scientific Computing and Imaging Institute,
//  University of Utah.
//  
//  
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//  

#include <GL/glew.h>
#ifdef _WIN32
#include <GL/wglew.h>
#endif
#include <KernelProgram.h>
#include <Debug.h>
#ifdef _WIN32
#define WINDOWS_LEAN_AND_MEAN
#include <Windows.h>
#endif
#include <algorithm>
#include <cmath>

namespace flvr
{
	bool KernelProgram::init_ = false;
	cl_device_id KernelProgram::device_ = 0;
	cl_context KernelProgram::context_ = 0;
	int KernelProgram::platform_id_ = 0;
	int KernelProgram::device_id_ = 0;
	std::string KernelProgram::device_name_;
	std::vector<CLPlatform> KernelProgram::device_list_;
#ifdef _DARWIN
	CGLContextObj KernelProgram::gl_context_ = 0;
#endif
	KernelProgram::KernelProgram(const std::string& source) :
	source_(source), program_(0), queue_(0),
	kernel_idx_(-1), arg_idx_(-1)
	{
	}

	KernelProgram::~KernelProgram()
	{
		destroy();
	}

	void KernelProgram::init_kernels_supported()
	{
		if (init_)
			return;

		device_list_.clear();
		cl_int err;
		cl_uint platform_num;
		cl_platform_id* platforms;

		//get platform number
		err = clGetPlatformIDs(0, NULL, &platform_num);
		if (err != CL_SUCCESS)
			return;
		if (platform_num == 0)
			return;
		platforms = new cl_platform_id[platform_num];
		err = clGetPlatformIDs(platform_num, platforms, NULL);
		if (err != CL_SUCCESS)
		{
			delete[] platforms;
			return;
		}

		//go through each platform
		size_t info_size;
		for (cl_uint i = 0; i < platform_num; ++i)
		{
			device_list_.push_back(CLPlatform());
			CLPlatform* platform = &(device_list_.back());
			//id
			platform->id = platforms[i];
			//get vendor name
			err = clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, 0, NULL, &info_size);
			platform->vendor.resize(info_size, 0);
			err = clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, info_size, &(platform->vendor[0]), NULL);
			//get platform name
			err = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, 0, NULL, &info_size);
			platform->name.resize(info_size, 0);
			err = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, info_size, &(platform->name[0]), NULL);

			cl_device_id *devices;
			cl_uint device_num;
			//get gpu devices
			err = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, 0, NULL, &device_num);
			if (err != CL_SUCCESS || device_num == 0)
				continue;
			devices = new cl_device_id[device_num];
			err = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, device_num, devices, NULL);
			if (err != CL_SUCCESS)
			{
				delete[] devices;
				continue;
			}

			//go through each device
			for (cl_uint j = 0; j < device_num; ++j)
			{
				platform->devices.push_back(CLDevice());
				CLDevice* device = &(platform->devices.back());
				//id
				device->id = devices[j];
				//get vendor name
				err = clGetDeviceInfo(devices[j], CL_DEVICE_VENDOR, 0, NULL, &info_size);
				device->vendor.resize(info_size, 0);
				err = clGetDeviceInfo(devices[j], CL_DEVICE_VENDOR, info_size, &(device->vendor[0]), NULL);
				//get device name
				err = clGetDeviceInfo(devices[j], CL_DEVICE_NAME, 0, NULL, &info_size);
				device->name.resize(info_size, 0);
				err = clGetDeviceInfo(devices[j], CL_DEVICE_NAME, info_size, &(device->name[0]), NULL);
			}
			delete[] devices;
		}
		delete[] platforms;

		if (device_list_.empty())
			return;
#ifdef _WIN32
		cl_context_properties properties[] =
		{
			CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
			CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
			CL_CONTEXT_PLATFORM, (cl_context_properties)0,
			0
		};
#else
		cl_context_properties properties[] =
		{
			#if defined(_DARWIN) 
			CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE,
			(cl_context_properties)CGLGetShareGroup(CGLGetCurrentContext()),
			#elif defined(__linux__)
			// https://www.codeproject.com/Articles/685281/OpenGL-OpenCL-Interoperability-A-Case-Study-Using
			CL_GL_CONTEXT_KHR , (cl_context_properties)glXGetCurrentContext() ,
			CL_GLX_DISPLAY_KHR , (cl_context_properties)glXGetCurrentDisplay() ,
			#endif
			CL_CONTEXT_PLATFORM, (cl_context_properties)0,
			0
		};
#endif
		if (platform_id_ < 0 || platform_id_ >= device_list_.size())
			platform_id_ = 0;
		CLPlatform* platform = &(device_list_[platform_id_]);
		if (!platform)
			return;
		if (platform->devices.empty())
			return;
#ifdef _WIN32
		properties[5] = (cl_context_properties)(platform->id);
#else
		properties[3] = (cl_context_properties)(platform->id);
#endif
		CLDevice* device = 0;
		if (device_id_ < 0 || device_id_ >= platform->devices.size())
			device = &(platform->devices[0]);
		else
			device = &(platform->devices[device_id_]);
		if (!device)
			return;
		device_ = device->id;
		device_name_ = platform->name;
		device_name_.back() = ';';
		device_name_ += " " + device->name;

		context_ = clCreateContext(properties, 1, &device_, NULL, NULL, &err);
		if (err == CL_SUCCESS)
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

	void KernelProgram::set_platform_id(int id)
	{
		platform_id_ = id;
	}

	int KernelProgram::get_platform_id()
	{
		return platform_id_;
	}

	void KernelProgram::set_device_id(int id)
	{
		device_id_ = id;
	}

	int KernelProgram::get_device_id()
	{
		return device_id_;
	}

	std::string& KernelProgram::get_device_name()
	{
		return device_name_;
	}

	void KernelProgram::release()
	{
		clReleaseContext(context_);
	}

	std::vector<CLPlatform>* KernelProgram::GetDeviceList()
	{
		return &device_list_;
	}

	//finish
	void KernelProgram::finish()
	{
		clFlush(queue_);
		clFinish(queue_);
	}

	//create a kernel in the program
	//return kernel index; -1 unsuccessful
	int KernelProgram::createKernel(const std::string &name)
	{
		cl_int err;

		//build program
		if (!program_)
		{
			const char *c_source[1];
			c_source[0] = source_.c_str();
			size_t program_size = source_.size();
			program_ = clCreateProgramWithSource(context_, 1,
				c_source, &program_size, &err);
			if (err != CL_SUCCESS)
				return -1;

			err = clBuildProgram(program_, 0, NULL, NULL, NULL, NULL);
			info_.clear();
			if (err != CL_SUCCESS)
			{
				char *program_log;
				size_t log_size;
				clGetProgramBuildInfo(program_, device_, CL_PROGRAM_BUILD_LOG,
					0, NULL, &log_size);
				program_log = new char[log_size+1];
				program_log[log_size] = '\0';
				clGetProgramBuildInfo(program_, device_, CL_PROGRAM_BUILD_LOG,
					log_size+1, program_log, NULL);
				info_ = program_log;
				//DBGPRINT(L"clBuildProgram error:\t%d\n", err);
				delete[] program_log;
				return -1;
			}
		}

		if (!queue_)
		{
#if defined(__APPLE__) || defined(CL_VERSION_1_2)
            queue_ = clCreateCommandQueue(context_, device_, 0, &err);
#else
			queue_ = clCreateCommandQueueWithProperties(context_, device_, nullptr, &err);
#endif
			if (err != CL_SUCCESS)
				return -1;
		}

		int result = findKernel(name);
		if (result == -1)
		{
			cl_kernel kernel = clCreateKernel(program_, name.c_str(), &err);
			if (err != CL_SUCCESS)
				return -1;
			else
			{
				Kernel s_kernel;
				s_kernel.kernel = kernel;
				s_kernel.name = name;
				s_kernel.external = false;
				kernels_.push_back(s_kernel);
				return static_cast<int>(kernels_.size() - 1);
			}
		}

		return result;
	}

	int KernelProgram::findKernel(const std::string &name)
	{
		for (size_t i = 0; i < kernels_.size(); ++i)
		{
			if (kernels_[i].name == name &&
				!kernels_[i].external)
				return static_cast<int>(i);
		}
		return -1;
	}

	int KernelProgram::addKernel(KernelProgram* kernel_prog, int kernel_index)
	{
		if (kernel_index < 0 ||
			kernel_index >= kernel_prog->kernels_.size())
			return -1;

		Kernel s_kernel;
		s_kernel.kernel = kernel_prog->kernels_[kernel_index].kernel;
		s_kernel.name = kernel_prog->kernels_[kernel_index].name;
		s_kernel.external = true;
		kernels_.push_back(s_kernel);
		return static_cast<int>(kernels_.size() - 1);
	}

	void KernelProgram::removeExternalKernels()
	{
		auto it = kernels_.begin();
		while (it != kernels_.end())
		{
			if (it->external)
				it = kernels_.erase(it);
			else
				++it;
		}
	}

	bool KernelProgram::valid()
	{
		return init_ && program_ && queue_ && !kernels_.empty();
	}

	void KernelProgram::destroy()
	{
		for (size_t i = 0; i < kernels_.size(); ++i)
			if (!kernels_[i].external)
				clReleaseKernel(kernels_[i].kernel);
		//difficult to handle mem release here
		//some may be still used after kernal prog
		//for (unsigned int i = 0; i < arg_list_.size(); ++i)
		//{
		//	if (!arg_list_[i].protect_)
		//		clReleaseMemObject(arg_list_[i].buffer);
		//}
		clReleaseCommandQueue(queue_);
		clReleaseProgram(program_);
	}

	bool KernelProgram::executeKernel(int index, cl_uint dim, size_t *global_size, size_t *local_size)
	{
		bool result = false;
		if (!valid())
			return false;
		if (index < 0 || index >= kernels_.size())
			return false;

		cl_int err;
		glFinish();
		unsigned int i;
		for (i=0; i<arg_list_.size(); ++i)
		{
			if (arg_list_[i].find_kernel(index) &&
				arg_list_[i].size == 0 &&
				arg_list_[i].texture &&
				glIsTexture(arg_list_[i].texture))
			{
				err = clEnqueueAcquireGLObjects(queue_, 1, &(arg_list_[i].buffer), 0, NULL, NULL);
				if (err != CL_SUCCESS)
					result = false;
			}
		}
		err = clEnqueueNDRangeKernel(queue_, kernels_[index].kernel, dim, NULL, global_size,
			local_size, 0, NULL, NULL);
		if (err != CL_SUCCESS)
		{
			//DBGPRINT(L"clEnqueueNDRangeKernel error:\t%d\n", err);
			result = false;
		}
		for (i=0; i<arg_list_.size(); ++i)
		{
			if (arg_list_[i].find_kernel(index) &&
				arg_list_[i].size == 0 &&
				arg_list_[i].texture &&
				glIsTexture(arg_list_[i].texture))
			{
				err = clEnqueueReleaseGLObjects(queue_, 1, &(arg_list_[i].buffer), 0, NULL, NULL);
				if (err != CL_SUCCESS)
					result = false;
			}
		}
		clFlush(queue_);
		clFinish(queue_);
		return result;
	}

	bool KernelProgram::executeKernel(std::string &name, cl_uint dim, size_t *global_size, size_t *local_size)
	{
		int index = findKernel(name);
		return executeKernel(index, dim, global_size, local_size);
	}

	bool KernelProgram::getWorkGroupSize(int index, size_t* wgsize)
	{
		if (!valid())
			return false;
		if (index < 0 || index >= kernels_.size())
			return false;

		cl_int err;
		err = clGetKernelWorkGroupInfo(kernels_[index].kernel,
			device_, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t),
			wgsize, NULL);
		if (err != CL_SUCCESS)
			return false;
		return true;
	}

	bool KernelProgram::matchArgBuf(Argument& arg, unsigned int& arg_index)
	{
		for (unsigned int i = 0; i < arg_list_.size(); ++i)
		{
			if (arg_list_[i].buffer == arg.buffer)
			{
				arg_index = i;
				return true;
			}
		}
		return false;
	}

	bool KernelProgram::matchArg(Argument& arg, unsigned int& arg_index)
	{
		for (unsigned int i=0; i<arg_list_.size(); ++i)
		{
			if (arg_list_[i].size == arg.size &&
				arg_list_[i].texture == arg.texture)
			{
				arg_index = i;
				return true;
			}
		}
		return false;
	}

	bool KernelProgram::matchArgTex(Argument& arg, unsigned int& arg_index)
	{
		for (unsigned int i = 0; i<arg_list_.size(); ++i)
		{
			if (arg_list_[i].texture == arg.texture)
			{
				arg_index = i;
				return true;
			}
		}
		return false;
	}

	bool KernelProgram::matchArgAddr(Argument& arg, unsigned int& arg_index)
	{
		for (unsigned int i = 0; i<arg_list_.size(); ++i)
		{
			if (arg_list_[i].orgn_addr == arg.orgn_addr &&
				arg_list_[i].size == arg.size)
			{
				arg_index = i;
				return true;
			}
		}
		return false;
	}

	int KernelProgram::setKernelArgument(Argument& arg)
	{
		arg.kernel(kernel_idx_);

		unsigned int ai = -1;
		if (!matchArgBuf(arg, ai))
		{
			arg_list_.push_back(arg);
			ai = static_cast<int>(arg_list_.size() - 1);
		}
		else
			arg_list_[ai].kernel(kernel_idx_);

		cl_int err = clSetKernelArg(kernels_[kernel_idx_].kernel,
			arg_idx_++, sizeof(cl_mem), &(arg.buffer));
		if (err != CL_SUCCESS)
			return -1;
		return ai;
	}

	void KernelProgram::setKernelArgConst(size_t size, void* data)
	{
		cl_int err;

		if (!data)
			return;
		if (kernel_idx_ < 0 || kernel_idx_ >= kernels_.size())
			return;

		err = clSetKernelArg(kernels_[kernel_idx_].kernel, arg_idx_++, size, data);
		if (err != CL_SUCCESS)
			return;
	}

	Argument KernelProgram::setKernelArgBuf(cl_mem_flags flag, size_t size, void* data)
	{
		Argument arg;
		cl_int err;
		if (kernel_idx_ < 0 || kernel_idx_ >= kernels_.size())
			return arg;

		arg.kernel(kernel_idx_);
		arg.size = size;
		arg.texture = 0;
		arg.orgn_addr = data;

		unsigned int ai;
		bool found = false;
		if (data)
		{
			found = matchArgAddr(arg, ai);
			if (found)
			{
				arg.buffer = arg_list_[ai].buffer;
				arg_list_[ai].kernel(kernel_idx_);
			}
		}

		if (!found)
		{
			arg.buffer = clCreateBuffer(context_, flag, size, data, &err);
			if (err != CL_SUCCESS)
				return Argument();
			arg_list_.push_back(arg);
		}

		err = clSetKernelArg(
			kernels_[kernel_idx_].kernel,
			arg_idx_++, sizeof(cl_mem), &(arg.buffer));
		if (err != CL_SUCCESS)
			return Argument();

		return arg;
	}

	Argument KernelProgram::setKernelArgBufWrite(cl_mem_flags flag, size_t size, void* data)
	{
		Argument arg;
		cl_int err;
		cl_mem buffer = 0;
		if (kernel_idx_ < 0 || kernel_idx_ >= kernels_.size())
			return arg;

		if (data)
		{
			arg.kernel(kernel_idx_);
			arg.size = size;
			arg.texture = 0;
			arg.orgn_addr = data;

			unsigned int ai;
			if (matchArgAddr(arg, ai))
			{
				arg.buffer = arg_list_[ai].buffer;
				clReleaseMemObject(arg_list_[ai].buffer);
				arg.buffer = clCreateBuffer(context_, flag, size, data, &err);
				buffer = arg.buffer;
				arg_list_[ai].kernel(kernel_idx_);
				if (err != CL_SUCCESS)
					return Argument();
			}
			else
			{
				buffer = clCreateBuffer(context_, flag, size, data, &err);
				if (err != CL_SUCCESS)
					return Argument();
				arg.buffer = buffer;
				arg_list_.push_back(arg);
			}

			err = clSetKernelArg(
				kernels_[kernel_idx_].kernel,
				arg_idx_++, sizeof(cl_mem), &(arg.buffer));
			if (err != CL_SUCCESS)
				return Argument();
		}
		else
		{
			err = clSetKernelArg(
				kernels_[kernel_idx_].kernel,
				arg_idx_++, size, NULL);
		}
		return arg;
	}

	Argument KernelProgram::setKernelArgTex2D(cl_mem_flags flag, GLuint texture)
	{
		Argument arg;
		cl_int err;
		cl_mem buffer = 0;
		if (kernel_idx_ < 0 || kernel_idx_ >= kernels_.size())
			return arg;

		arg.kernel(kernel_idx_);
		//arg.index = arg_idx_++;
		arg.size = 0;
		arg.texture = texture;
		arg.orgn_addr = 0;

		unsigned int ai;
		if (matchArgTex(arg, ai))
		{
			arg.buffer = arg_list_[ai].buffer;
			buffer = arg.buffer;
			arg_list_[ai].kernel(kernel_idx_);
		}
		else
		{
			buffer = clCreateFromGLTexture(context_, flag, GL_TEXTURE_2D, 0, texture, &err);
			if (err != CL_SUCCESS)
				return Argument();
			arg.buffer = buffer;
			arg_list_.push_back(arg);
		}
		err = clSetKernelArg(
			kernels_[kernel_idx_].kernel,
			arg_idx_++, sizeof(cl_mem), &(arg.buffer));
		if (err != CL_SUCCESS)
			return Argument();
		return arg;
	}

	Argument KernelProgram::setKernelArgTex3D(cl_mem_flags flag, GLuint texture)
	{
		Argument arg;
		cl_int err;
		cl_mem buffer = 0;
		if (kernel_idx_ < 0 || kernel_idx_ >= kernels_.size())
			return arg;

		arg.kernel(kernel_idx_);
		arg.size = 0;
		arg.texture = texture;
		arg.orgn_addr = 0;

		unsigned int ai;
		if (matchArgTex(arg, ai))
		{
			arg.buffer = arg_list_[ai].buffer;
			buffer = arg.buffer;
			arg_list_[ai].kernel(kernel_idx_);
		}
		else
		{
			buffer = clCreateFromGLTexture(context_, flag, GL_TEXTURE_3D, 0, texture, &err);
			if (err != CL_SUCCESS)
				return Argument();
			arg.buffer = buffer;
			arg_list_.push_back(arg);
		}
		err = clSetKernelArg(
			kernels_[kernel_idx_].kernel,
			arg_idx_++, sizeof(cl_mem), &(arg.buffer));
		if (err != CL_SUCCESS)
			return Argument();
		return arg;
	}

	//copy existing texure to buffer
	Argument KernelProgram::setKernelArgTex3DBuf(
		cl_mem_flags flag, GLuint texture,
		size_t size, size_t *region)
	{
		Argument arg, argt;
		cl_int err;
		cl_mem buft = 0, bufb = 0;
		if (kernel_idx_ < 0 || kernel_idx_ >= kernels_.size())
			return arg;

		arg.kernel(kernel_idx_);
		//arg.index = arg_idx_++;
		arg.size = size;
		arg.texture = 0;
		arg.orgn_addr = 0;

		unsigned int ai;
		//create texture buffer
		argt.texture = texture;
		if (matchArgTex(argt, ai))
		{
			argt.buffer = arg_list_[ai].buffer;
			buft = argt.buffer;
			arg_list_[ai].kernel(kernel_idx_);
		}
		else
		{
			buft = clCreateFromGLTexture(context_, CL_MEM_READ_WRITE, GL_TEXTURE_3D, 0, texture, &err);
			if (err != CL_SUCCESS)
				return Argument();
			argt.buffer = buft;
			arg_list_.push_back(argt);
		}
		//find data buffer
		if (matchArgBuf(arg, ai))
		{
			bufb = arg.buffer;
			arg_list_[ai].kernel(kernel_idx_);
		}
		else
		{
			bufb = clCreateBuffer(context_, flag, size, 0, &err);
			if (err != CL_SUCCESS)
				return Argument();
			arg.buffer = bufb;
			arg_list_.push_back(arg);
		}

		err = clEnqueueAcquireGLObjects(
			queue_, 1, &(buft), 0, NULL, NULL);
		if (err != CL_SUCCESS)
			return Argument();
		size_t sorigin[3] = { 0, 0, 0 };
		err = clEnqueueCopyImageToBuffer(
			queue_, buft, arg.buffer,
			sorigin, region, 0, 0, NULL, NULL);
		if (err != CL_SUCCESS)
			return Argument();
		err = clEnqueueReleaseGLObjects(
			queue_, 1, &(buft), 0, NULL, NULL);
		if (err != CL_SUCCESS)
			return Argument();

		err = clSetKernelArg(
			kernels_[kernel_idx_].kernel,
			arg_idx_++, sizeof(cl_mem), &(arg.buffer));
		if (err != CL_SUCCESS)
			return Argument();

		clFlush(queue_);
		clFinish(queue_);
		return arg;
	}

	Argument KernelProgram::setKernelArgImage(cl_mem_flags flag, cl_image_format format, cl_image_desc desc, void* data)
	{
		Argument arg;
		cl_int err;
		cl_mem buffer = 0;
		if (kernel_idx_ < 0 || kernel_idx_ >= kernels_.size())
			return arg;

		arg.kernel(kernel_idx_);
		//arg.index = arg_idx_++;
		arg.size = 0;
		arg.texture = 0;
		arg.orgn_addr = data;

		unsigned int ai;
		if (matchArgAddr(arg, ai))
		{
			arg.buffer = arg_list_[ai].buffer;
			buffer = arg.buffer;
			arg_list_[ai].kernel(kernel_idx_);
		}
		else
		{
			buffer = clCreateImage(context_, flag,
				&format, &desc, data, &err);
			if (err != CL_SUCCESS)
				return Argument();
			arg.buffer = buffer;
			arg_list_.push_back(arg);
		}
		err = clSetKernelArg(
			kernels_[kernel_idx_].kernel,
			arg_idx_++, sizeof(cl_mem), &(arg.buffer));
		if (err != CL_SUCCESS)
			return Argument();
		return arg;
	}

	void KernelProgram::setKernelArgLocal(size_t size)
	{
		cl_int err = clSetKernelArg(
			kernels_[kernel_idx_].kernel,
			arg_idx_++, size, NULL);
	}

	void KernelProgram::readBuffer(size_t size,
		void* buf_data, void* data)
	{
		cl_int err;

		if (buf_data)
		{
			Argument arg;
			arg.size = size;
			arg.texture = 0;
			arg.orgn_addr = buf_data;

			unsigned int ai;
			if (matchArgAddr(arg, ai))
			{
				arg.buffer = arg_list_[ai].buffer;
				err = clEnqueueReadBuffer(
					queue_, arg.buffer,
					CL_TRUE, 0, arg.size,
					data, 0, NULL, NULL);
				if (err != CL_SUCCESS)
					return;
				clFlush(queue_);
				clFinish(queue_);
			}
		}
	}

	void KernelProgram::readBuffer(Argument& arg, void* data)
	{
		cl_int err;
		unsigned int ai;
		if (matchArgBuf(arg, ai))
		{
			err = clEnqueueReadBuffer(
				queue_, arg.buffer,
				CL_TRUE, 0, arg_list_[ai].size,
				data, 0, NULL, NULL);
			if (err != CL_SUCCESS)
				return;
			clFlush(queue_);
			clFinish(queue_);
		}
	}

	//copy buffer back to texture
	void KernelProgram::copyBufTex3D(
		Argument& arg, GLuint texture,
		size_t size, size_t* region)
	{
		cl_int err;
		Argument argt;
		argt.texture = texture;
		cl_mem buft = 0;
		unsigned int ai1, ai2;
		if (matchArgBuf(arg, ai1) &&
			matchArgTex(argt, ai2))
		{
			argt.buffer = arg_list_[ai2].buffer;
			buft = argt.buffer;
			err = clEnqueueAcquireGLObjects(
				queue_, 1, &(buft), 0, NULL, NULL);
			if (err != CL_SUCCESS)
				return;
			size_t sorigin[3] = { 0, 0, 0 };
			err = clEnqueueCopyBufferToImage(
				queue_, arg.buffer, buft, 0,
				sorigin, region, 0, NULL, NULL);
			if (err != CL_SUCCESS)
				return;
			err = clEnqueueReleaseGLObjects(
				queue_, 1, &(buft), 0, NULL, NULL);
			if (err != CL_SUCCESS)
				return;
			clFlush(queue_);
			clFinish(queue_);
			//clReleaseMemObject(buft);
		}
	}

	void KernelProgram::writeBuffer(size_t size,
		void* buf_data, void* data)
	{
		cl_int err;

		if (buf_data)
		{
			Argument arg;
			arg.size = size;
			arg.texture = 0;
			arg.orgn_addr = buf_data;

			unsigned int ai;
			if (matchArgAddr(arg, ai))
			{
				arg.buffer = arg_list_[ai].buffer;
				err = clEnqueueWriteBuffer(
					queue_, arg.buffer,
					CL_TRUE, 0, size,
					data, 0, NULL, NULL);
				if (err != CL_SUCCESS)
					return;
				clFlush(queue_);
				clFinish(queue_);
			}
		}
	}

	void KernelProgram::writeBuffer(Argument& arg, void* data)
	{
		cl_int err;
		unsigned int ai;
		if (matchArgBuf(arg, ai))
		{
			err = clEnqueueWriteBuffer(
				queue_, arg.buffer,
				CL_TRUE, 0, arg_list_[ai].size,
				data, 0, NULL, NULL);
			if (err != CL_SUCCESS)
				return;
			clFlush(queue_);
			clFinish(queue_);
		}
	}

	void KernelProgram::writeImage(
		const size_t* origin, const size_t* region,
		void* img_data, void* data)
	{
		cl_int err;

		if (img_data)
		{
			Argument arg;
			arg.size = 0;
			arg.texture = 0;
			arg.orgn_addr = img_data;

			unsigned int ai;
			if (matchArgAddr(arg, ai))
			{
				arg.buffer = arg_list_[ai].buffer;
				err = clEnqueueWriteImage(
					queue_, arg.buffer,
					CL_TRUE, origin, region,
					0, 0, data, 0, NULL, NULL);
				if (err != CL_SUCCESS)
					return;
				clFlush(queue_);
				clFinish(queue_);
			}
		}
	}

	void KernelProgram::writeImage(
		const size_t* origin, const size_t* region,
		Argument& arg, void* data)
	{
		cl_int err;
		unsigned int ai;
		if (matchArgBuf(arg, ai))
		{
			err = clEnqueueWriteImage(
				queue_, arg.buffer,
				CL_TRUE, origin, region,
				0, 0, data, 0, NULL, NULL);
			if (err != CL_SUCCESS)
				return;
			clFlush(queue_);
			clFinish(queue_);
		}
	}

	//release mem obj
	void KernelProgram::releaseAll(bool del_mem)
	{
		cl_int err;
		if (del_mem)
		{
			for (auto it = arg_list_.begin();
				it != arg_list_.end(); ++it)
			{
				err = clReleaseMemObject(it->buffer);
			}
		}
		arg_list_.clear();
	}

	void KernelProgram::releaseMemObject(Argument& arg)
	{
		unsigned int ai;
		if (matchArgBuf(arg, ai))
		{
			clReleaseMemObject(arg.buffer);
			arg_list_.erase(arg_list_.begin() + ai);
		}
	}

	void KernelProgram::releaseMemObject(int kernel_index,
		int index, size_t size, GLuint texture)
	{
		Argument arg;
		arg.size = size;
		arg.texture = texture;
		unsigned int ai;

		if (texture)
		{
			if (matchArgTex(arg, ai))
			{
				clReleaseMemObject(arg_list_[ai].buffer);
				arg_list_.erase(arg_list_.begin() + ai);
			}
		}
		else
		{
			if (matchArg(arg, ai))
			{
				clReleaseMemObject(arg_list_[ai].buffer);
				arg_list_.erase(arg_list_.begin() + ai);
			}
		}
	}

	void KernelProgram::releaseMemObject(size_t size, void* orgn_addr)
	{
		Argument arg;
		arg.size = size;
		arg.texture = 0;
		arg.orgn_addr = orgn_addr;
		unsigned int ai;

		if (matchArgAddr(arg, ai))
		{
			clReleaseMemObject(arg_list_[ai].buffer);
			arg_list_.erase(arg_list_.begin() + ai);
		}
	}

	std::string& KernelProgram::getInfo()
	{
		return info_;
	}

	bool KernelProgram::get_group_size(int index,
		unsigned int nx, unsigned int ny, unsigned int nz,
		GroupSize &ksize)
	{
		size_t ng;
		if (!getWorkGroupSize(index, &ng))
			return false;

		//try to make gsxyz equal to ng
		//ngx*ngy*ngz = nx*ny*nz/ng
		//z
		unsigned int targetz = (unsigned int)(std::ceil(double(nz) /
			std::pow(double(ng), 1 / 3.0)));
		//optimize
		ksize.ngz = optimize_group_size_z(nz, targetz);
		//xy
		unsigned int targetx;
		unsigned int targety;
		if (ksize.ngz == 1)
		{
			targetx = (unsigned int)(std::ceil(double(nx) /
				std::sqrt(double(ng))));
			targety = (unsigned int)(std::ceil(double(ny) /
				std::sqrt(double(ng))));
		}
		else
		{
			targetx = (unsigned int)(std::ceil(double(nx) *
				targetz / nz));
			targety = (unsigned int)(std::ceil(double(ny) *
				targetz / nz));
		}
		//optimize
		ksize.ngx = optimize_group_size_xy(nx, targetx);
		ksize.ngy = optimize_group_size_xy(ny, targety);

		ksize.gsx = nx / ksize.ngx + (nx%ksize.ngx ? 1 : 0);
		ksize.gsy = ny / ksize.ngy + (ny%ksize.ngy ? 1 : 0);
		ksize.gsz = nz / ksize.ngz + (nz%ksize.ngz ? 1 : 0);
		ksize.gsxyz = ksize.gsx * ksize.gsy * ksize.gsz;
		ksize.gsxy = ksize.gsx * ksize.gsy;

		return true;
	}

	bool KernelProgram::get_group_size2(int index,
		unsigned int nx, unsigned int ny, unsigned int nz,
		GroupSize &ksize)
	{
		size_t ng;
		if (!getWorkGroupSize(index, &ng))
			return false;

		//z
		ksize.ngz = nz;
		//xy
		unsigned int targetx;
		unsigned int targety;
		targetx = (unsigned int)(std::ceil(double(nx) /
			std::sqrt(double(ng))));
		targety = (unsigned int)(std::ceil(double(ny) /
			std::sqrt(double(ng))));
		//optimize
		ksize.ngx = optimize_group_size_xy(nx, targetx);
		ksize.ngy = optimize_group_size_xy(ny, targety);

		ksize.gsx = nx / ksize.ngx + (nx%ksize.ngx ? 1 : 0);
		ksize.gsy = ny / ksize.ngy + (ny%ksize.ngy ? 1 : 0);
		ksize.gsz = nz / ksize.ngz + (nz%ksize.ngz ? 1 : 0);
		ksize.gsxyz = ksize.gsx * ksize.gsy * ksize.gsz;
		ksize.gsxy = ksize.gsx * ksize.gsy;

		return true;
	}

	unsigned int KernelProgram::optimize_group_size_xy(unsigned int nt, unsigned int target)
	{
		unsigned int loj, hij, res, maxj;
		if (nt > target)
		{
			loj = std::max((unsigned int)(1), (target + 1) / 2);
			hij = std::min(nt, target * 2);
			res = 0; maxj = 0;
			for (size_t j = loj; j < hij; ++j)
			{
				unsigned int rm = nt % j;
				if (rm)
				{
					if (rm > res)
					{
						res = rm;
						maxj = static_cast<unsigned int>(j);
					}
				}
				else
				{
					return static_cast<unsigned int>(j);
				}
			}
			if (maxj)
				return maxj;
		}

		return target;
	}

	unsigned int KernelProgram::optimize_group_size_z(unsigned int nt, unsigned int target)
	{
		unsigned int loj, hij, res, maxj;
		if (nt > target)
		{
			loj = target;
			hij = std::max(nt, target * 2);
			res = 0; maxj = 0;
			for (size_t j = loj; j < hij; ++j)
			{
				unsigned int rm = nt % j;
				if (rm)
				{
					if (rm > res)
					{
						res = rm;
						maxj = static_cast<unsigned int>(j);
					}
				}
				else
				{
					return static_cast<unsigned int>(j);
				}
			}
			if (maxj)
				return maxj;
		}

		return target;
	}
}
