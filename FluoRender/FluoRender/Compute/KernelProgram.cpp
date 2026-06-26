//
//  For more information, please see: http://software.sci.utah.edu
//
//  The MIT License
//
//  Copyright (c) 2026 Scientific Computing and Imaging Institute,
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

#include <fl_gl.h>
#include <Global.h>
#include <MainSettings.h>
#include <KernelProgram.h>
#include <Debug.h>
#ifdef _WIN32
#define WINDOWS_LEAN_AND_MEAN
#include <Windows.h>
#endif
#ifdef __linux__
#include <EGL/egl.h>
#endif
#include <algorithm>
#include <cmath>
#include <sstream>
#include <string>
#include <cstring>

namespace flvr
{
	void Argument::destroy()
	{
		DBGPRINT(L"Argument::destroy() called: ptr=%p buffer=%p valid=%d\n",
			this, buffer_, valid_);

		if (valid_)
		{
			if (buffer_)
			{
				cl_int err = clReleaseMemObject(buffer_);
				DBGPRINT(L"clReleaseMemObject err=%d\n", err);

				if (err != CL_SUCCESS)
				{
					DBGPRINT(L"WARNING: clReleaseMemObject failed\n");
				}
			}
			else
			{
				DBGPRINT(L"WARNING: buffer_ already null\n");
			}

			type_ = ArgType_Unknown;
			name_.clear();
			protect_ = false;
			size_ = 0;
			tex_ = 0;
			vbo_ = 0;
			buffer_ = 0;
			pointer_ = nullptr;
			valid_ = false;

			DBGPRINT(L"Argument destroyed successfully\n");
		}
		else
		{
			DBGPRINT(L"Argument already invalid\n");
		}
	}

	std::shared_ptr<Argument> Argument::createFromPointer(
		cl_context context,
		cl_mem_flags flags,
		const std::string& name,
		size_t size,
		void* data)
	{
		DBGPRINT(L"createFromPointer(): name=%S size=%zu flags=0x%X data=%p\n",
			name.c_str(), size, (unsigned int)flags, data);

		if (!context)
		{
			DBGPRINT(L"ERROR: context is null\n");
			return nullptr;
		}

		cl_int err = CL_SUCCESS;

		cl_mem buf = clCreateBuffer(context, flags, size, data, &err);

		DBGPRINT(L"clCreateBuffer err=%d buffer=%p\n", err, buf);

		if (err != CL_SUCCESS || !buf)
		{
			DBGPRINT(L"ERROR: Failed to create buffer\n");
			return nullptr;
		}

		auto arg = std::make_shared<Argument>();
		arg->type_ = ArgType_Pointer;
		arg->name_ = name;
		arg->buffer_ = buf;
		arg->size_ = size;
		arg->pointer_ = data;
		arg->valid_ = true;

		DBGPRINT(L"createFromPointer success: arg=%p\n", arg.get());

		return arg;
	}

	std::shared_ptr<Argument> Argument::createFromTexture2D(
		cl_context context,
		cl_mem_flags flags,
		GLuint tex_id)
	{
		DBGPRINT(L"createFromTexture2D(): tex_id=%u flags=0x%X\n",
			tex_id, (unsigned int)flags);

		if (!context)
		{
			DBGPRINT(L"ERROR: context is null\n");
			return nullptr;
		}

		if (!glIsTexture(tex_id))
		{
			DBGPRINT(L"ERROR: invalid GL texture %u\n", tex_id);
			return nullptr;
		}

		cl_int err = CL_SUCCESS;

		cl_mem buf = clCreateFromGLTexture(
			context,
			flags,
			GL_TEXTURE_2D,
			0,
			tex_id,
			&err
		);

		DBGPRINT(L"clCreateFromGLTexture(2D) err=%d buffer=%p\n", err, buf);

		if (err != CL_SUCCESS || !buf)
		{
			DBGPRINT(L"ERROR: Failed to create CL texture\n");
			return nullptr;
		}

		auto arg = std::make_shared<Argument>();
		arg->type_ = ArgType_Tex;
		arg->buffer_ = buf;
		arg->tex_ = tex_id;
		arg->valid_ = true;

		DBGPRINT(L"createFromTexture2D success: arg=%p\n", arg.get());

		return arg;
	}

	std::shared_ptr<Argument> Argument::createFromTexture3D(
		cl_context context,
		cl_mem_flags flags,
		GLuint tex_id)
	{
		DBGPRINT(L"createFromTexture3D(): tex_id=%u flags=0x%X\n",
			tex_id, (unsigned int)flags);

		if (!context)
		{
			DBGPRINT(L"ERROR: context is null\n");
			return nullptr;
		}

		if (!glIsTexture(tex_id))
		{
			DBGPRINT(L"ERROR: invalid GL texture %u\n", tex_id);
			return nullptr;
		}

		cl_int err = CL_SUCCESS;

		cl_mem buf = clCreateFromGLTexture(
			context,
			flags,
			GL_TEXTURE_3D,
			0,
			tex_id,
			&err
		);

		DBGPRINT(L"clCreateFromGLTexture(3D) err=%d buffer=%p\n", err, buf);

		if (err != CL_SUCCESS || !buf)
		{
			DBGPRINT(L"ERROR: Failed to create CL 3D texture\n");
			return nullptr;
		}

		auto arg = std::make_shared<Argument>();
		arg->type_ = ArgType_Tex;
		arg->buffer_ = buf;
		arg->tex_ = tex_id;
		arg->valid_ = true;

		DBGPRINT(L"createFromTexture3D success: arg=%p\n", arg.get());

		return arg;
	}

	std::shared_ptr<Argument> Argument::createFromVBO(
		cl_context context,
		cl_mem_flags flags,
		GLuint vbo_id,
		size_t size)
	{
		DBGPRINT(L"createFromVBO(): vbo_id=%u size=%zu flags=0x%X\n",
			vbo_id, size, (unsigned int)flags);

		if (!context)
		{
			DBGPRINT(L"ERROR: context is null\n");
			return nullptr;
		}

		if (!glIsBuffer(vbo_id))
		{
			DBGPRINT(L"ERROR: invalid GL buffer %u\n", vbo_id);
			return nullptr;
		}

		cl_int err = CL_SUCCESS;

		cl_mem buf = clCreateFromGLBuffer(context, flags, vbo_id, &err);

		DBGPRINT(L"clCreateFromGLBuffer err=%d buffer=%p\n", err, buf);

		if (err != CL_SUCCESS || !buf)
		{
			DBGPRINT(L"ERROR: Failed to create CL buffer from VBO\n");
			return nullptr;
		}

		auto arg = std::make_shared<Argument>();
		arg->type_ = ArgType_VBO;
		arg->buffer_ = buf;
		arg->vbo_ = vbo_id;
		arg->size_ = size;
		arg->valid_ = true;

		DBGPRINT(L"createFromVBO success: arg=%p\n", arg.get());

		return arg;
	}

	bool KernelProgram::init_ = false;
	bool KernelProgram::interop_ = false;
	cl_device_id KernelProgram::device_ = 0;
	cl_context KernelProgram::context_ = 0;
	cl_command_queue KernelProgram::queue_ = 0;
	int KernelProgram::platform_id_ = 0;
	int KernelProgram::device_id_ = 0;
	std::string KernelProgram::device_name_;
	std::vector<CLPlatform> KernelProgram::device_list_;
	bool KernelProgram::float_atomics_ = false;
	bool KernelProgram::need_clear_ = false;
#ifdef _DARWIN
	CGLContextObj KernelProgram::gl_context_ = 0;
#endif
	KernelProgram::KernelProgram(const std::string& source) :
		source_(source), program_(0),
		kernel_idx_(-1), arg_idx_(-1)
	{
	}

	KernelProgram::~KernelProgram()
	{
		destroy();
	}

	void KernelProgram::init_kernels_supported()
	{
		DBGPRINT(L"init_kernels_supported() entered\n");

		if (init_)
		{
			DBGPRINT(L"Already initialized, skipping\n");
			return;
		}

		device_list_.clear();

		cl_int err;
		cl_uint platform_num = 0;

		// --- Get platforms ---
		err = clGetPlatformIDs(0, NULL, &platform_num);
		DBGPRINT(L"clGetPlatformIDs(count) err=%d, platform_num=%u\n", err, platform_num);

		if (err != CL_SUCCESS || platform_num == 0)
		{
			DBGPRINT(L"No platforms found or error occurred\n");
			return;
		}

		std::vector<cl_platform_id> platforms(platform_num);

		err = clGetPlatformIDs(platform_num, platforms.data(), NULL);
		DBGPRINT(L"clGetPlatformIDs(list) err=%d\n", err);

		if (err != CL_SUCCESS)
			return;

		// --- Enumerate platforms/devices ---
		size_t info_size = 0;

		for (cl_uint i = 0; i < platform_num; ++i)
		{
			DBGPRINT(L"Inspecting platform %u\n", i);

			device_list_.push_back(CLPlatform());
			CLPlatform* platform = &(device_list_.back());
			platform->id = platforms[i];

			// platform vendor
			clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, 0, NULL, &info_size);
			platform->vendor.resize(info_size);
			clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, info_size, &platform->vendor[0], NULL);

			DBGPRINT(L"Platform vendor: %S\n", platform->vendor.c_str());

			// platform name
			clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, 0, NULL, &info_size);
			platform->name.resize(info_size);
			clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, info_size, &platform->name[0], NULL);

			DBGPRINT(L"Platform name: %S\n", platform->name.c_str());

			// --- Devices ---
			cl_uint device_num = 0;

			err = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, 0, NULL, &device_num);
			DBGPRINT(L"clGetDeviceIDs(count) err=%d, devices=%u\n", err, device_num);

			if (err != CL_SUCCESS || device_num == 0)
			{
				DBGPRINT(L"No GPU devices found on this platform\n");
				continue;
			}

			std::vector<cl_device_id> devices(device_num);

			err = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, device_num, devices.data(), NULL);
			DBGPRINT(L"clGetDeviceIDs(list) err=%d\n", err);

			if (err != CL_SUCCESS)
				continue;

			for (cl_uint j = 0; j < device_num; ++j)
			{
				DBGPRINT(L"Inspecting device %u\n", j);

				platform->devices.push_back(CLDevice());
				CLDevice* device = &(platform->devices.back());
				device->id = devices[j];

				// vendor
				clGetDeviceInfo(devices[j], CL_DEVICE_VENDOR, 0, NULL, &info_size);
				device->vendor.resize(info_size);
				clGetDeviceInfo(devices[j], CL_DEVICE_VENDOR, info_size, &device->vendor[0], NULL);
				if (!device->vendor.empty() && device->vendor.back() == '\0')
					device->vendor.pop_back();

				DBGPRINT(L"Device vendor: %S\n", device->vendor.c_str());

				// name
				clGetDeviceInfo(devices[j], CL_DEVICE_NAME, 0, NULL, &info_size);
				device->name.resize(info_size);
				clGetDeviceInfo(devices[j], CL_DEVICE_NAME, info_size, &device->name[0], NULL);
				if (!device->name.empty() && device->name.back() == '\0')
					device->name.pop_back();

				DBGPRINT(L"Device name: %S\n", device->name.c_str());

				// version
				clGetDeviceInfo(devices[j], CL_DEVICE_OPENCL_C_VERSION, 0, NULL, &info_size);
				device->version.resize(info_size);
				clGetDeviceInfo(devices[j], CL_DEVICE_OPENCL_C_VERSION, info_size, &device->version[0], NULL);
				if (!device->version.empty() && device->version.back() == '\0')
					device->version.pop_back();

				DBGPRINT(L"Device OpenCL C version: %S\n", device->version.c_str());
			}
		}

		if (device_list_.empty())
		{
			DBGPRINT(L"No usable platforms/devices found\n");
			return;
		}

		// --- Select platform ---
		if (platform_id_ < 0 || platform_id_ >= (int)device_list_.size())
		{
			DBGPRINT(L"Invalid platform_id_=%d, defaulting to 0\n", platform_id_);
			platform_id_ = 0;
		}

		CLPlatform* platform = &(device_list_[platform_id_]);

		if (!platform || platform->devices.empty())
		{
			DBGPRINT(L"Selected platform has no devices\n");
			return;
		}

		// --- Select device ---
		CLDevice* device = nullptr;

		if (device_id_ < 0 || device_id_ >= (int)platform->devices.size())
		{
			DBGPRINT(L"Invalid device_id_=%d, defaulting to 0\n", device_id_);
			device = &(platform->devices[0]);
		}
		else
		{
			device = &(platform->devices[device_id_]);
		}

		if (!device)
		{
			DBGPRINT(L"Device selection failed\n");
			return;
		}

		device_ = device->id;

		DBGPRINT(L"Selected device: %S | %S\n",
			platform->name.c_str(),
			device->name.c_str());

		// --- Device name ---
		device_name_ = platform->name;
		if (!device_name_.empty())
			device_name_.back() = ';';
		device_name_ += " " + device->name;

		DBGPRINT(L"Device name string: %S\n", device_name_.c_str());

		// --- GL-CL interop setup ---
		cl_context_properties properties[7];
		int p = 0;

#ifdef _WIN32

		HGLRC gl_ctx = wglGetCurrentContext();
		HDC   hdc = wglGetCurrentDC();

		DBGPRINT(L"WGL context=%p, DC=%p\n", gl_ctx, hdc);

		if (!gl_ctx || !hdc)
		{
			DBGPRINT(L"No valid GL context\n");
			return;
		}

		properties[p++] = CL_GL_CONTEXT_KHR;
		properties[p++] = (cl_context_properties)gl_ctx;
		properties[p++] = CL_WGL_HDC_KHR;
		properties[p++] = (cl_context_properties)hdc;

		properties[p++] = CL_CONTEXT_PLATFORM;
		properties[p++] = (cl_context_properties)platform->id;

#endif

		properties[p++] = 0;

		// --- Create context ---
		context_ = clCreateContext(properties, 1, &device_, NULL, NULL, &err);

		DBGPRINT(L"clCreateContext err=%d, context=%p\n", err, context_);

		if (err != CL_SUCCESS)
		{
			DBGPRINT(L"Context creation failed, disabling interop\n");
			interop_ = false;
			return;
		}
		else
		{
			DBGPRINT(L"Context creation succeeded (interop enabled)\n");
			interop_ = true;
		}

		init_ = true;

		// --- Extensions ---
		clGetDeviceInfo(device_, CL_DEVICE_EXTENSIONS, 0, NULL, &info_size);
		std::string extensions(info_size, '\0');
		clGetDeviceInfo(device_, CL_DEVICE_EXTENSIONS, info_size, &extensions[0], NULL);

		DBGPRINT(L"Extensions: %S\n", extensions.c_str());

		float_atomics_ =
			(extensions.find("cl_khr_global_float_atomics") != std::string::npos);

		DBGPRINT(L"float_atomics_=%d\n", float_atomics_);

		// --- Other flags ---
		need_clear_ = get_need_clear(device);

		DBGPRINT(L"need_clear_=%d\n", need_clear_);

		DBGPRINT(L"init_kernels_supported() completed successfully\n");
	}

	bool KernelProgram::init()
	{
		return init_;
	}

	void KernelProgram::clear()
	{
		DBGPRINT(L"clear() called\n");

		if (!context_)
		{
			DBGPRINT(L"Context already null, nothing to release\n");
		}
		else
		{
			cl_int err = clReleaseContext(context_);
			DBGPRINT(L"clReleaseContext err=%d, context=%p\n", err, context_);

			if (err != CL_SUCCESS)
			{
				DBGPRINT(L"WARNING: clReleaseContext failed\n");
			}

			context_ = nullptr;
		}

		init_ = false;

		DBGPRINT(L"clear() completed, init_=false\n");
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

	void KernelProgram::release_context()
	{
		DBGPRINT(L"release_context() called\n");

		// --- Release queue ---
		if (queue_)
		{
			cl_int err = clReleaseCommandQueue(queue_);
			DBGPRINT(L"clReleaseCommandQueue err=%d, queue=%p\n", err, queue_);

			if (err != CL_SUCCESS)
			{
				DBGPRINT(L"WARNING: clReleaseCommandQueue failed\n");
			}

			queue_ = nullptr;
		}
		else
		{
			DBGPRINT(L"No command queue to release\n");
		}

		// --- Release context ---
		if (context_)
		{
			cl_int err = clReleaseContext(context_);
			DBGPRINT(L"clReleaseContext err=%d, context=%p\n", err, context_);

			if (err != CL_SUCCESS)
			{
				DBGPRINT(L"WARNING: clReleaseContext failed\n");
			}

			context_ = nullptr;
		}
		else
		{
			DBGPRINT(L"No context to release\n");
		}

		DBGPRINT(L"release_context() completed\n");
	}

	std::vector<CLPlatform>* KernelProgram::GetDeviceList()
	{
		return &device_list_;
	}

	bool KernelProgram::get_need_clear(CLDevice* device)
	{
		if (!device)
			return false;
		std::istringstream stream(glbin_settings.m_device_need_clear);
		std::string entry;

		while (std::getline(stream, entry, ';')) {
			size_t sep = entry.find('|');
			if (sep == std::string::npos) continue;

			std::string vendor_sub = entry.substr(0, sep);
			std::string device_sub = entry.substr(sep + 1);

			if (strstr(device->vendor.c_str(), vendor_sub.c_str()) &&
				strstr(device->name.c_str(), device_sub.c_str())) {
				return true;
			}
		}
		return false;
	}

	//finish
	void KernelProgram::finish()
	{
		DBGPRINT(L"finish() called\n");

		if (!queue_)
		{
			DBGPRINT(L"WARNING: queue_ is null, cannot flush/finish\n");
			return;
		}

		// --- Flush ---
		cl_int err = clFlush(queue_);
		DBGPRINT(L"clFlush err=%d, queue=%p\n", err, queue_);

		if (err != CL_SUCCESS)
		{
			DBGPRINT(L"WARNING: clFlush failed\n");
		}

		// --- Finish ---
		err = clFinish(queue_);
		DBGPRINT(L"clFinish err=%d, queue=%p\n", err, queue_);

		if (err != CL_SUCCESS)
		{
			DBGPRINT(L"WARNING: clFinish failed\n");
		}

		DBGPRINT(L"finish() completed\n");
	}

	//create a kernel in the program
	//return kernel index; -1 unsuccessful
	int KernelProgram::createKernel(const std::string& name)
	{
		DBGPRINT(L"createKernel() called, name=%S\n", name.c_str());

		cl_int err;

		// --- Build program ---
		if (!program_)
		{
			DBGPRINT(L"Creating OpenCL program from source\n");

			const char* c_source[1];
			c_source[0] = source_.c_str();
			size_t program_size = source_.size();

			program_ = clCreateProgramWithSource(
				context_,
				1,
				c_source,
				&program_size,
				&err);

			DBGPRINT(L"clCreateProgramWithSource err=%d, program=%p\n", err, program_);

			if (err != CL_SUCCESS)
			{
				DBGPRINT(L"ERROR: Failed to create program\n");
				return -1;
			}

			// --- Build program ---
			DBGPRINT(L"Building OpenCL program...\n");

			err = clBuildProgram(program_, 0, NULL, NULL, NULL, NULL);

			DBGPRINT(L"clBuildProgram err=%d\n", err);

			info_.clear();

			if (err != CL_SUCCESS)
			{
				DBGPRINT(L"ERROR: clBuildProgram failed\n");

				size_t log_size = 0;

				clGetProgramBuildInfo(
					program_,
					device_,
					CL_PROGRAM_BUILD_LOG,
					0,
					NULL,
					&log_size);

				DBGPRINT(L"Build log size=%zu\n", log_size);

				if (log_size > 1)
				{
					char* program_log = new char[log_size + 1];
					program_log[log_size] = '\0';

					clGetProgramBuildInfo(
						program_,
						device_,
						CL_PROGRAM_BUILD_LOG,
						log_size + 1,
						program_log,
						NULL);

					info_ = program_log;

					DBGPRINT(L"==== OpenCL Build Log Start ====\n");
					DBGPRINT(L"%S\n", program_log);
					DBGPRINT(L"==== OpenCL Build Log End ====\n");

					delete[] program_log;
				}
				else
				{
					DBGPRINT(L"No build log available\n");
				}

				return -1;
			}

			DBGPRINT(L"Program build succeeded\n");
		}
		else
		{
			DBGPRINT(L"Program already exists, skipping build\n");
		}

		// --- Create command queue ---
		if (!queue_)
		{
			DBGPRINT(L"Creating command queue\n");

#if defined(__APPLE__)
			queue_ = clCreateCommandQueue(context_, device_, 0, &err);
#else
			queue_ = clCreateCommandQueueWithProperties(context_, device_, nullptr, &err);
#endif

			DBGPRINT(L"Command queue created, err=%d, queue=%p\n", err, queue_);

			if (err != CL_SUCCESS)
			{
				DBGPRINT(L"ERROR: Failed to create command queue\n");
				return -1;
			}
		}
		else
		{
			DBGPRINT(L"Command queue already exists\n");
		}

		// --- Check existing kernel ---
		int result = findKernel(name);

		if (result != -1)
		{
			DBGPRINT(L"Kernel '%S' already exists, index=%d\n", name.c_str(), result);
			return result;
		}

		DBGPRINT(L"Creating new kernel '%S'\n", name.c_str());

		// --- Create kernel ---
		cl_kernel kernel = clCreateKernel(program_, name.c_str(), &err);

		DBGPRINT(L"clCreateKernel err=%d, kernel=%p\n", err, kernel);

		if (err != CL_SUCCESS)
		{
			DBGPRINT(L"ERROR: Failed to create kernel '%S'\n", name.c_str());
			return -1;
		}

		// --- Store kernel ---
		Kernel s_kernel;
		s_kernel.kernel = kernel;
		s_kernel.name = name;
		s_kernel.external = false;

		kernels_.push_back(s_kernel);

		int index = static_cast<int>(kernels_.size() - 1);

		DBGPRINT(L"Kernel '%S' created successfully, index=%d\n", name.c_str(), index);

		return index;
	}

	int KernelProgram::findKernel(const std::string& name)
	{
		for (size_t i = 0; i < kernels_.size(); ++i)
		{
			if (kernels_[i].name == name &&
				!kernels_[i].external)
				return static_cast<int>(i);
		}
		return -1;
	}

	bool KernelProgram::valid()
	{
		return init_ && program_ && queue_ && !kernels_.empty();
	}

	void KernelProgram::destroy()
	{
		DBGPRINT(L"destroy() called\n");

		// --- Release kernels ---
		DBGPRINT(L"Releasing %zu kernels\n", kernels_.size());

		for (size_t i = 0; i < kernels_.size(); ++i)
		{
			if (!kernels_[i].external)
			{
				DBGPRINT(L"Releasing kernel[%zu]: name=%S, handle=%p\n",
					i,
					kernels_[i].name.c_str(),
					kernels_[i].kernel);

				if (kernels_[i].kernel)
				{
					cl_int err = clReleaseKernel(kernels_[i].kernel);

					DBGPRINT(L"clReleaseKernel err=%d\n", err);

					if (err != CL_SUCCESS)
					{
						DBGPRINT(L"WARNING: clReleaseKernel failed for '%S'\n",
							kernels_[i].name.c_str());
					}

					kernels_[i].kernel = nullptr;
				}
				else
				{
					DBGPRINT(L"Kernel handle already null\n");
				}
			}
			else
			{
				DBGPRINT(L"Skipping external kernel[%zu]: name=%S\n",
					i,
					kernels_[i].name.c_str());
			}
		}

		// --- Release program ---
		if (program_)
		{
			DBGPRINT(L"Releasing program: %p\n", program_);

			cl_int err = clReleaseProgram(program_);

			DBGPRINT(L"clReleaseProgram err=%d\n", err);

			if (err != CL_SUCCESS)
			{
				DBGPRINT(L"WARNING: clReleaseProgram failed\n");
			}

			program_ = nullptr;
		}
		else
		{
			DBGPRINT(L"No program to release\n");
		}

		// Optional but very helpful
		kernels_.clear();

		DBGPRINT(L"destroy() completed\n");
	}

	bool KernelProgram::matches(const std::string& s)
	{
		return source_ == s;
	}

	bool KernelProgram::executeKernel(int index,
		cl_uint dim,
		size_t* global_size,
		size_t* local_size)
	{
		DBGPRINT(L"executeKernel() called: index=%d, dim=%u\n", index, dim);

		if (!valid())
		{
			DBGPRINT(L"ERROR: KernelProgram not valid\n");
			return false;
		}

		if (index < 0 || index >= (int)kernels_.size())
		{
			DBGPRINT(L"ERROR: Invalid kernel index=%d (size=%zu)\n",
				index, kernels_.size());
			return false;
		}

		if (!queue_)
		{
			DBGPRINT(L"ERROR: queue_ is null\n");
			return false;
		}

		DBGPRINT(L"Executing kernel: %S\n", kernels_[index].name.c_str());

		// --- Log sizes ---
		if (global_size)
		{
			DBGPRINT(L"Global size: ");
			for (cl_uint i = 0; i < dim; i++)
				DBGPRINT(L"%zu ", global_size[i]);
			DBGPRINT(L"\n");
		}

		if (local_size)
		{
			DBGPRINT(L"Local size: ");
			for (cl_uint i = 0; i < dim; i++)
				DBGPRINT(L"%zu ", local_size[i]);
			DBGPRINT(L"\n");

			// Optional sanity check
			if (global_size)
			{
				for (cl_uint i = 0; i < dim; i++)
				{
					if (local_size[i] > 0 && global_size[i] % local_size[i] != 0)
					{
						DBGPRINT(L"WARNING: global_size[%u]=%zu not divisible by local_size[%u]=%zu\n",
							i, global_size[i], i, local_size[i]);
					}
				}
			}
		}
		else
		{
			DBGPRINT(L"Local size: NULL (auto)\n");
		}

		bool result = true;
		cl_int err = CL_SUCCESS;

		// --- GL sync ---
		DBGPRINT(L"Calling glFinish()\n");
		glFinish();

		auto& args = arg_map_[index];

		DBGPRINT(L"Acquiring GL objects (arg count=%zu)\n", args.size());

		// ---------- Acquire ----------
		size_t arg_index = 0;

		for (const auto& weak_arg : args)
		{
			auto arg = weak_arg.lock();

			if (!arg)
			{
				DBGPRINT(L"Arg[%zu]: expired weak_ptr\n", arg_index++);
				continue;
			}

			if (!arg->valid_ || !arg->buffer_)
			{
				DBGPRINT(L"Arg[%zu]: invalid or missing buffer\n", arg_index++);
				continue;
			}

			DBGPRINT(L"Arg[%zu]: ptr=%p tex=%u vbo=%u buf=%p\n",
				arg_index,
				arg.get(),
				arg->tex_,
				arg->vbo_,
				arg->buffer_);

			if (arg->tex_ && glIsTexture(arg->tex_) && arg->size_ == 0)
			{
				err = clEnqueueAcquireGLObjects(queue_, 1, &arg->buffer_, 0, nullptr, nullptr);

				DBGPRINT(L"Arg[%zu]: acquire texture err=%d\n", arg_index, err);

				if (err != CL_SUCCESS) result = false;
			}
			else if (arg->vbo_ && glIsBuffer(arg->vbo_))
			{
				err = clEnqueueAcquireGLObjects(queue_, 1, &arg->buffer_, 0, nullptr, nullptr);

				DBGPRINT(L"Arg[%zu]: acquire VBO err=%d\n", arg_index, err);

				if (err != CL_SUCCESS) result = false;
			}

			arg_index++;
		}

		// ---------- Execute kernel ----------
		DBGPRINT(L"Enqueue kernel execution\n");

		err = clEnqueueNDRangeKernel(
			queue_,
			kernels_[index].kernel,
			dim,
			nullptr,
			global_size,
			local_size,
			0,
			nullptr,
			nullptr
		);

		DBGPRINT(L"clEnqueueNDRangeKernel err=%d, kernel=%p\n",
			err, kernels_[index].kernel);

		if (err != CL_SUCCESS)
		{
			DBGPRINT(L"ERROR: Kernel execution failed\n");
			result = false;
		}

		// ---------- Release ----------
		DBGPRINT(L"Releasing GL objects\n");

		arg_index = 0;

		for (const auto& weak_arg : args)
		{
			auto arg = weak_arg.lock();

			if (!arg || !arg->valid_ || !arg->buffer_)
			{
				arg_index++;
				continue;
			}

			if (arg->tex_ && glIsTexture(arg->tex_) && arg->size_ == 0)
			{
				err = clEnqueueReleaseGLObjects(queue_, 1, &arg->buffer_, 0, nullptr, nullptr);

				DBGPRINT(L"Arg[%zu]: release texture err=%d\n", arg_index, err);

				if (err != CL_SUCCESS) result = false;
			}
			else if (arg->vbo_ && glIsBuffer(arg->vbo_))
			{
				err = clEnqueueReleaseGLObjects(queue_, 1, &arg->buffer_, 0, nullptr, nullptr);

				DBGPRINT(L"Arg[%zu]: release VBO err=%d\n", arg_index, err);

				if (err != CL_SUCCESS) result = false;
			}

			arg_index++;
		}

		// ---------- Final sync ----------
		DBGPRINT(L"Flushing + finishing queue\n");

		err = clFlush(queue_);
		DBGPRINT(L"clFlush err=%d\n", err);
		if (err != CL_SUCCESS) result = false;

		err = clFinish(queue_);
		DBGPRINT(L"clFinish err=%d\n", err);
		if (err != CL_SUCCESS) result = false;

		DBGPRINT(L"executeKernel() completed, result=%d\n", result);

		return result;
	}

	bool KernelProgram::executeKernel(std::string& name, cl_uint dim, size_t* global_size, size_t* local_size)
	{
		int index = findKernel(name);
		return executeKernel(index, dim, global_size, local_size);
	}

	bool KernelProgram::getWorkGroupSize(int index, size_t* wgsize)
	{
		DBGPRINT(L"getWorkGroupSize() called: index=%d\n", index);

		if (!valid())
		{
			DBGPRINT(L"ERROR: KernelProgram not valid\n");
			return false;
		}

		if (index < 0 || index >= (int)kernels_.size())
		{
			DBGPRINT(L"ERROR: Invalid kernel index=%d (size=%zu)\n",
				index, kernels_.size());
			return false;
		}

		if (!wgsize)
		{
			DBGPRINT(L"ERROR: wgsize pointer is null\n");
			return false;
		}

		cl_int err = clGetKernelWorkGroupInfo(
			kernels_[index].kernel,
			device_,
			CL_KERNEL_WORK_GROUP_SIZE,
			sizeof(size_t),
			wgsize,
			NULL
		);

		DBGPRINT(L"clGetKernelWorkGroupInfo err=%d, kernel=%p\n",
			err, kernels_[index].kernel);

		if (err != CL_SUCCESS)
		{
			DBGPRINT(L"ERROR: Failed to query work group size\n");
			return false;
		}

		DBGPRINT(L"Max work group size = %zu\n", *wgsize);

		return true;
	}

	bool KernelProgram::setConst(size_t size, void* data)
	{
		DBGPRINT(L"setConst() called: kernel_idx_=%d, arg_idx_=%d, size=%zu\n",
			kernel_idx_, arg_idx_, size);

		if (!data)
		{
			DBGPRINT(L"ERROR: data pointer is null\n");
			return false;
		}

		if (kernel_idx_ < 0 || kernel_idx_ >= (int)kernels_.size())
		{
			DBGPRINT(L"ERROR: Invalid kernel index=%d (size=%zu)\n",
				kernel_idx_, kernels_.size());
			return false;
		}

		DBGPRINT(L"Setting arg for kernel: %S\n",
			kernels_[kernel_idx_].name.c_str());

		// Optional: show raw pointer value (helps debugging invalid host memory)
		DBGPRINT(L"Arg ptr=%p\n", data);

		cl_int err = clSetKernelArg(
			kernels_[kernel_idx_].kernel,
			arg_idx_,
			size,
			data
		);

		DBGPRINT(L"clSetKernelArg(index=%d) err=%d\n", arg_idx_, err);

		if (err != CL_SUCCESS)
		{
			DBGPRINT(L"ERROR: clSetKernelArg failed (index=%d, size=%zu)\n",
				arg_idx_, size);
			return false;
		}

		// Increment only AFTER success (important fix!)
		arg_idx_++;

		return true;
	}

	bool KernelProgram::setLocal(size_t size)
	{
		DBGPRINT(L"setLocal() called: kernel_idx_=%d, arg_idx_=%d, size=%zu\n",
			kernel_idx_, arg_idx_, size);

		if (kernel_idx_ < 0 || kernel_idx_ >= (int)kernels_.size())
		{
			DBGPRINT(L"ERROR: Invalid kernel index=%d (size=%zu)\n",
				kernel_idx_, kernels_.size());
			return false;
		}

		if (size == 0)
		{
			DBGPRINT(L"WARNING: Local memory size is 0\n");
			// Not necessarily fatal — allow it, but log it
		}

		DBGPRINT(L"Setting local arg for kernel: %S\n",
			kernels_[kernel_idx_].name.c_str());

		cl_int err = clSetKernelArg(
			kernels_[kernel_idx_].kernel,
			arg_idx_,
			size,
			NULL   // local memory
		);

		DBGPRINT(L"clSetKernelArg (local) index=%d, size=%zu, err=%d\n",
			arg_idx_, size, err);

		if (err != CL_SUCCESS)
		{
			DBGPRINT(L"ERROR: clSetKernelArg (local) failed\n");
			return false;
		}

		// Increment ONLY on success (same fix as setConst)
		arg_idx_++;

		return true;
	}

	bool KernelProgram::bindArg(std::weak_ptr<Argument> arg)
	{
		DBGPRINT(L"bindArg() called: kernel_idx_=%d, arg_idx_=%d\n",
			kernel_idx_, arg_idx_);

		// --- Validate kernel index ---
		if (kernel_idx_ < 0 || kernel_idx_ >= (int)kernels_.size())
		{
			DBGPRINT(L"ERROR: Invalid kernel index=%d (size=%zu)\n",
				kernel_idx_, kernels_.size());
			return false;
		}

		// --- Lock weak_ptr ---
		auto shared_arg = arg.lock();

		if (!shared_arg)
		{
			DBGPRINT(L"ERROR: Argument weak_ptr expired\n");
			return false;
		}

		// --- Validate argument ---
		if (!shared_arg->valid_)
		{
			DBGPRINT(L"ERROR: Argument marked invalid\n");
			return false;
		}

		if (!shared_arg->buffer_)
		{
			DBGPRINT(L"ERROR: Argument buffer is null\n");
			return false;
		}

		DBGPRINT(L"Binding arg ptr=%p, buffer=%p, tex=%u, vbo=%u\n",
			shared_arg.get(),
			shared_arg->buffer_,
			shared_arg->tex_,
			shared_arg->vbo_);

		DBGPRINT(L"Kernel: %S\n",
			kernels_[kernel_idx_].name.c_str());

		// --- Track argument in list ---
		auto it = arg_list_.find(shared_arg);

		if (it == arg_list_.end())
		{
			DBGPRINT(L"Adding argument to arg_list_\n");
			arg_list_.insert(shared_arg);
		}
		else
		{
			DBGPRINT(L"Argument already in arg_list_\n");
		}

		// --- Map argument to kernel ---
		arg_map_[kernel_idx_].insert(shared_arg);

		DBGPRINT(L"Mapped argument to kernel index=%d\n", kernel_idx_);

		// --- Bind OpenCL buffer ---
		cl_int err = clSetKernelArg(
			kernels_[kernel_idx_].kernel,
			arg_idx_,
			sizeof(cl_mem),
			&(shared_arg->buffer_)
		);

		DBGPRINT(L"clSetKernelArg(buffer) index=%d err=%d\n",
			arg_idx_, err);

		if (err != CL_SUCCESS)
		{
			DBGPRINT(L"ERROR: Failed to bind buffer argument\n");
			return false;
		}

		// increment ONLY on success
		++arg_idx_;

		DBGPRINT(L"bindArg() success, next arg_idx_=%d\n", arg_idx_);

		return true;
	}

	std::weak_ptr<Argument> KernelProgram::setBufNew(
		cl_mem_flags flags,
		const std::string& name,
		size_t size,
		void* data)
	{
		DBGPRINT(L"setBufNew() called: kernel_idx_=%d, arg_idx_=%d\n",
			kernel_idx_, arg_idx_);

		DBGPRINT(L"Buffer request: name=%S, size=%zu, flags=0x%X, data=%p\n",
			name.c_str(), size, (unsigned int)flags, data);

		// --- Validate kernel index ---
		if (kernel_idx_ < 0 || kernel_idx_ >= (int)kernels_.size())
		{
			DBGPRINT(L"ERROR: Invalid kernel index=%d (size=%zu)\n",
				kernel_idx_, kernels_.size());
			return std::weak_ptr<Argument>();
		}

		if (!context_)
		{
			DBGPRINT(L"ERROR: context_ is null\n");
			return std::weak_ptr<Argument>();
		}

		// --- Step 1: Try to reuse existing argument ---
		std::shared_ptr<Argument> existing_arg;

		for (const auto& arg : arg_list_)
		{
			if (arg->matchesPointer(name, size, data))
			{
				existing_arg = arg;
				DBGPRINT(L"Reusing existing buffer: ptr=%p, buffer=%p\n",
					arg.get(), arg->buffer_);
				break;
			}
		}

		// --- Step 2: Create new if needed ---
		if (!existing_arg)
		{
			DBGPRINT(L"Creating new buffer\n");

			existing_arg = Argument::createFromPointer(
				context_, flags, name, size, data);

			if (!existing_arg)
			{
				DBGPRINT(L"ERROR: Argument::createFromPointer returned null\n");
				return std::weak_ptr<Argument>();
			}

			if (!existing_arg->valid_)
			{
				DBGPRINT(L"ERROR: Created argument is invalid\n");
				return std::weak_ptr<Argument>();
			}

			DBGPRINT(L"New buffer created: ptr=%p, buffer=%p\n",
				existing_arg.get(), existing_arg->buffer_);

			// Track in program
			arg_list_.insert(existing_arg);
		}

		// --- Validate buffer ---
		if (!existing_arg->buffer_)
		{
			DBGPRINT(L"ERROR: buffer_ is null\n");
			return std::weak_ptr<Argument>();
		}

		DBGPRINT(L"Binding buffer to kernel: %S\n",
			kernels_[kernel_idx_].name.c_str());

		// --- Step 3: Bind to kernel ---
		cl_int err = clSetKernelArg(
			kernels_[kernel_idx_].kernel,
			arg_idx_,
			sizeof(cl_mem),
			&(existing_arg->buffer_)
		);

		DBGPRINT(L"clSetKernelArg(buffer) index=%d err=%d\n",
			arg_idx_, err);

		if (err != CL_SUCCESS)
		{
			DBGPRINT(L"ERROR: Failed to bind buffer argument\n");
			return std::weak_ptr<Argument>();
		}

		// --- Step 4: Track per-kernel usage ---
		arg_map_[kernel_idx_].insert(existing_arg);

		DBGPRINT(L"Buffer mapped to kernel index=%d\n", kernel_idx_);

		// increment only after success
		++arg_idx_;

		DBGPRINT(L"setBufNew() success, next arg_idx_=%d\n", arg_idx_);

		// --- Step 5: return weak_ptr ---
		return std::weak_ptr<Argument>(existing_arg);
	}

	std::weak_ptr<Argument> KernelProgram::setBufNewOrUpdate(
		cl_mem_flags flags,
		const std::string& name,
		size_t size,
		void* data)
	{
		DBGPRINT(L"setBufNewOrUpdate() called: kernel_idx_=%d, arg_idx_=%d\n",
			kernel_idx_, arg_idx_);

		DBGPRINT(L"Buffer request: name=%S, size=%zu, flags=0x%X, data=%p\n",
			name.c_str(), size, (unsigned int)flags, data);

		// --- Validate kernel ---
		if (kernel_idx_ < 0 || kernel_idx_ >= (int)kernels_.size())
		{
			DBGPRINT(L"ERROR: Invalid kernel index=%d (size=%zu)\n",
				kernel_idx_, kernels_.size());
			return std::weak_ptr<Argument>();
		}

		if (!context_)
		{
			DBGPRINT(L"ERROR: context_ is null\n");
			return std::weak_ptr<Argument>();
		}

		std::shared_ptr<Argument> arg;

		// ---------- Try to find existing ----------
		for (const auto& existing : arg_list_)
		{
			if (existing->matchesPointer(name, size, data))
			{
				arg = existing;
				DBGPRINT(L"Found existing buffer: ptr=%p, buffer=%p\n",
					existing.get(), existing->buffer_);
				break;
			}
		}

		// ---------- Create new ----------
		if (!arg)
		{
			DBGPRINT(L"No existing buffer found, creating new one\n");

			arg = Argument::createFromPointer(context_, flags, name, size, data);

			if (!arg)
			{
				DBGPRINT(L"ERROR: Argument::createFromPointer returned null\n");
				return std::weak_ptr<Argument>();
			}

			if (!arg->valid_)
			{
				DBGPRINT(L"ERROR: New argument invalid\n");
				return std::weak_ptr<Argument>();
			}

			DBGPRINT(L"New buffer created: ptr=%p, buffer=%p\n",
				arg.get(), arg->buffer_);

			arg_list_.insert(arg);
		}
		else
		{
			// ---------- Update existing ----------
			DBGPRINT(L"Updating existing buffer\n");

			if (!updateBuf(arg, flags, size, data))
			{
				DBGPRINT(L"ERROR: updateBuf failed\n");
				return std::weak_ptr<Argument>();
			}

			DBGPRINT(L"Buffer updated successfully\n");
		}

		// ---------- Validate buffer ----------
		if (!arg->buffer_)
		{
			DBGPRINT(L"ERROR: buffer_ is null after create/update\n");
			return std::weak_ptr<Argument>();
		}

		// ---------- Bind to kernel ----------
		DBGPRINT(L"Binding buffer to kernel: %S\n",
			kernels_[kernel_idx_].name.c_str());

		cl_int err = clSetKernelArg(
			kernels_[kernel_idx_].kernel,
			arg_idx_,
			sizeof(cl_mem),
			&(arg->buffer_)
		);

		DBGPRINT(L"clSetKernelArg(buffer) index=%d err=%d\n",
			arg_idx_, err);

		if (err != CL_SUCCESS)
		{
			DBGPRINT(L"ERROR: Failed to bind buffer argument\n");
			return std::weak_ptr<Argument>();
		}

		// ---------- Track usage ----------
		arg_map_[kernel_idx_].insert(arg);

		DBGPRINT(L"Buffer mapped to kernel index=%d\n", kernel_idx_);

		// Increment ONLY on success
		++arg_idx_;

		DBGPRINT(L"setBufNewOrUpdate() success, next arg_idx_=%d\n", arg_idx_);

		return std::weak_ptr<Argument>(arg);
	}

	bool KernelProgram::updateBuf(std::weak_ptr<Argument> arg,
		cl_mem_flags flags,
		size_t new_size,
		void* data)
	{
		DBGPRINT(L"updateBuf() called: new_size=%zu, flags=0x%X, data=%p\n",
			new_size, (unsigned int)flags, data);

		// --- Lock argument ---
		auto shared_arg = arg.lock();

		if (!shared_arg)
		{
			DBGPRINT(L"ERROR: weak_ptr expired\n");
			return false;
		}

		if (!shared_arg->valid_)
		{
			DBGPRINT(L"ERROR: Argument invalid\n");
			return false;
		}

		if (!shared_arg->buffer_)
		{
			DBGPRINT(L"ERROR: buffer_ is null\n");
			return false;
		}

		if (!context_)
		{
			DBGPRINT(L"ERROR: context_ is null\n");
			return false;
		}

		DBGPRINT(L"Existing buffer: ptr=%p, buffer=%p, size=%zu\n",
			shared_arg.get(),
			shared_arg->buffer_,
			shared_arg->size_);

		// ---------- Case 1: Update in-place ----------
		if (new_size <= shared_arg->size_)
		{
			DBGPRINT(L"Updating buffer in-place (write only)\n");

			if (!queue_)
			{
				DBGPRINT(L"ERROR: queue_ is null\n");
				return false;
			}

			if (!data)
			{
				DBGPRINT(L"WARNING: data is null, writing nothing\n");
				return true; // allowed — nothing to write
			}

			cl_int err = clEnqueueWriteBuffer(
				queue_,
				shared_arg->buffer_,
				CL_TRUE,
				0,
				new_size,
				data,
				0, nullptr, nullptr
			);

			DBGPRINT(L"clEnqueueWriteBuffer size=%zu err=%d\n", new_size, err);

			if (err != CL_SUCCESS)
			{
				DBGPRINT(L"ERROR: clEnqueueWriteBuffer failed\n");
				return false;
			}

			return true;
		}

		// ---------- Case 2: Reallocate ----------
		DBGPRINT(L"Reallocating buffer (old size=%zu, new size=%zu)\n",
			shared_arg->size_, new_size);

		// Release old buffer
		shared_arg->release();

		cl_int err = CL_SUCCESS;

		cl_mem new_buf = clCreateBuffer(
			context_,
			flags,
			new_size,
			data,
			&err
		);

		DBGPRINT(L"clCreateBuffer err=%d, new_buf=%p\n", err, new_buf);

		if (err != CL_SUCCESS || !new_buf)
		{
			DBGPRINT(L"ERROR: Failed to create new buffer\n");
			return false;
		}

		// Update Argument in-place
		shared_arg->buffer_ = new_buf;
		shared_arg->size_ = new_size;
		shared_arg->pointer_ = data;
		shared_arg->valid_ = true;

		DBGPRINT(L"Buffer reallocated successfully: buffer=%p size=%zu\n",
			new_buf, new_size);

		return true;
	}

	std::weak_ptr<Argument> KernelProgram::setTex2D(
		cl_mem_flags flags,
		GLuint tex_id)
	{
		DBGPRINT(L"setTex2D() called: kernel_idx_=%d, arg_idx_=%d, tex_id=%u\n",
			kernel_idx_, arg_idx_, tex_id);

		// --- Validate kernel ---
		if (kernel_idx_ < 0 || kernel_idx_ >= (int)kernels_.size())
		{
			DBGPRINT(L"ERROR: Invalid kernel index=%d (size=%zu)\n",
				kernel_idx_, kernels_.size());
			return std::weak_ptr<Argument>();
		}

		if (!context_)
		{
			DBGPRINT(L"ERROR: context_ is null\n");
			return std::weak_ptr<Argument>();
		}

		if (!tex_id)
		{
			DBGPRINT(L"ERROR: tex_id is 0\n");
			return std::weak_ptr<Argument>();
		}

		if (!glIsTexture(tex_id))
		{
			DBGPRINT(L"ERROR: tex_id=%u is not a valid GL texture\n", tex_id);
			return std::weak_ptr<Argument>();
		}

		DBGPRINT(L"GL texture validated: %u\n", tex_id);

		std::shared_ptr<Argument> existing_arg;

		// ---------- Step 1: reuse existing ----------
		for (const auto& arg : arg_list_)
		{
			if (arg->matchesTexture(tex_id))
			{
				existing_arg = arg;
				DBGPRINT(L"Reusing existing texture arg: ptr=%p, buffer=%p\n",
					arg.get(), arg->buffer_);
				break;
			}
		}

		// ---------- Step 2: create new ----------
		if (!existing_arg)
		{
			DBGPRINT(L"Creating new texture argument\n");

			existing_arg = Argument::createFromTexture2D(context_, flags, tex_id);

			if (!existing_arg)
			{
				DBGPRINT(L"ERROR: createFromTexture2D returned null\n");
				return std::weak_ptr<Argument>();
			}

			if (!existing_arg->valid_)
			{
				DBGPRINT(L"ERROR: created texture argument invalid\n");
				return std::weak_ptr<Argument>();
			}

			if (!existing_arg->buffer_)
			{
				DBGPRINT(L"ERROR: created buffer_ is null\n");
				return std::weak_ptr<Argument>();
			}

			DBGPRINT(L"New texture buffer created: ptr=%p, buffer=%p\n",
				existing_arg.get(), existing_arg->buffer_);

			arg_list_.insert(existing_arg);
		}

		// ---------- Step 3: bind to kernel ----------
		DBGPRINT(L"Binding texture to kernel: %S\n",
			kernels_[kernel_idx_].name.c_str());

		cl_int err = clSetKernelArg(
			kernels_[kernel_idx_].kernel,
			arg_idx_,
			sizeof(cl_mem),
			&(existing_arg->buffer_)
		);

		DBGPRINT(L"clSetKernelArg(texture) index=%d err=%d\n",
			arg_idx_, err);

		if (err != CL_SUCCESS)
		{
			DBGPRINT(L"ERROR: Failed to bind texture argument\n");
			return std::weak_ptr<Argument>();
		}

		// ---------- Step 4: track usage ----------
		arg_map_[kernel_idx_].insert(existing_arg);

		DBGPRINT(L"Texture mapped to kernel index=%d\n", kernel_idx_);

		// increment only after success
		++arg_idx_;

		DBGPRINT(L"setTex2D() success, next arg_idx_=%d\n", arg_idx_);

		return std::weak_ptr<Argument>(existing_arg);
	}

	std::weak_ptr<Argument> KernelProgram::setTex3D(
		cl_mem_flags flags,
		GLuint tex_id)
	{
		DBGPRINT(L"setTex3D() called: kernel_idx_=%d, arg_idx_=%d, tex_id=%u\n",
			kernel_idx_, arg_idx_, tex_id);

		// --- Validate kernel ---
		if (kernel_idx_ < 0 || kernel_idx_ >= (int)kernels_.size())
		{
			DBGPRINT(L"ERROR: Invalid kernel index=%d (size=%zu)\n",
				kernel_idx_, kernels_.size());
			return std::weak_ptr<Argument>();
		}

		if (!context_)
		{
			DBGPRINT(L"ERROR: context_ is null\n");
			return std::weak_ptr<Argument>();
		}

		if (!tex_id)
		{
			DBGPRINT(L"ERROR: tex_id is 0\n");
			return std::weak_ptr<Argument>();
		}

		if (!glIsTexture(tex_id))
		{
			DBGPRINT(L"ERROR: tex_id=%u is not a valid GL texture\n", tex_id);
			return std::weak_ptr<Argument>();
		}

		DBGPRINT(L"GL 3D texture validated: %u\n", tex_id);

		std::shared_ptr<Argument> existing_arg;

		// ---------- Step 1: reuse existing ----------
		for (const auto& arg : arg_list_)
		{
			if (arg->matchesTexture(tex_id))
			{
				existing_arg = arg;
				DBGPRINT(L"Reusing existing 3D texture arg: ptr=%p, buffer=%p\n",
					arg.get(), arg->buffer_);
				break;
			}
		}

		// ---------- Step 2: create new ----------
		if (!existing_arg)
		{
			DBGPRINT(L"Creating new 3D texture argument\n");

			existing_arg = Argument::createFromTexture3D(context_, flags, tex_id);

			if (!existing_arg)
			{
				DBGPRINT(L"ERROR: createFromTexture3D returned null\n");
				return std::weak_ptr<Argument>();
			}

			if (!existing_arg->valid_)
			{
				DBGPRINT(L"ERROR: created 3D texture argument invalid\n");
				return std::weak_ptr<Argument>();
			}

			if (!existing_arg->buffer_)
			{
				DBGPRINT(L"ERROR: created buffer_ is null\n");
				return std::weak_ptr<Argument>();
			}

			DBGPRINT(L"New 3D texture buffer created: ptr=%p, buffer=%p\n",
				existing_arg.get(), existing_arg->buffer_);

			arg_list_.insert(existing_arg);
		}

		// ---------- Optional: log texture dimensions ----------
#ifdef GL_TEXTURE_3D
		GLint width = 0, height = 0, depth = 0;
		glBindTexture(GL_TEXTURE_3D, tex_id);
		glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_WIDTH, &width);
		glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_HEIGHT, &height);
		glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_DEPTH, &depth);

		DBGPRINT(L"3D Texture size: %d x %d x %d\n", width, height, depth);
#endif

		// ---------- Step 3: bind to kernel ----------
		DBGPRINT(L"Binding 3D texture to kernel: %S\n",
			kernels_[kernel_idx_].name.c_str());

		cl_int err = clSetKernelArg(
			kernels_[kernel_idx_].kernel,
			arg_idx_,
			sizeof(cl_mem),
			&(existing_arg->buffer_)
		);

		DBGPRINT(L"clSetKernelArg(3D texture) index=%d err=%d\n",
			arg_idx_, err);

		if (err != CL_SUCCESS)
		{
			DBGPRINT(L"ERROR: Failed to bind 3D texture argument\n");
			return std::weak_ptr<Argument>();
		}

		// ---------- Step 4: track usage ----------
		arg_map_[kernel_idx_].insert(existing_arg);

		DBGPRINT(L"3D texture mapped to kernel index=%d\n", kernel_idx_);

		// increment only after success
		++arg_idx_;

		DBGPRINT(L"setTex3D() success, next arg_idx_=%d\n", arg_idx_);

		return std::weak_ptr<Argument>(existing_arg);
	}

	std::weak_ptr<Argument> KernelProgram::copyTex3DToBuf(
		cl_mem_flags flags, GLuint texture_id,
		const std::string& name,
		size_t buffer_size, size_t* region)
	{
		DBGPRINT(L"copyTex3DToBuf() called: kernel_idx_=%d, arg_idx_=%d\n",
			kernel_idx_, arg_idx_);

		DBGPRINT(L"Texture=%u, buffer_name=%S, buffer_size=%zu\n",
			texture_id, name.c_str(), buffer_size);

		if (kernel_idx_ < 0 || kernel_idx_ >= (int)kernels_.size())
		{
			DBGPRINT(L"ERROR: Invalid kernel index=%d (size=%zu)\n",
				kernel_idx_, kernels_.size());
			return {};
		}

		if (!context_ || !queue_)
		{
			DBGPRINT(L"ERROR: context_ or queue_ is null\n");
			return {};
		}

		if (!region)
		{
			DBGPRINT(L"ERROR: region is null\n");
			return {};
		}

		DBGPRINT(L"Copy region: %zu x %zu x %zu\n",
			region[0], region[1], region[2]);

		// ---------- Step 1: Texture wrapper ----------
		std::shared_ptr<Argument> tex_arg;

		for (const auto& arg : arg_list_)
		{
			if (arg->matchesTexture(texture_id))
			{
				tex_arg = arg;
				DBGPRINT(L"Reusing texture arg: ptr=%p buffer=%p\n",
					arg.get(), arg->buffer_);
				break;
			}
		}

		if (!tex_arg)
		{
			DBGPRINT(L"Creating texture wrapper\n");

			tex_arg = Argument::createFromTexture3D(
				context_, CL_MEM_READ_WRITE, texture_id);

			if (!tex_arg || !tex_arg->valid_)
			{
				DBGPRINT(L"ERROR: Failed to create texture argument\n");
				return {};
			}

			arg_list_.insert(tex_arg);

			DBGPRINT(L"Texture wrapper created: buffer=%p\n", tex_arg->buffer_);
		}

		// ---------- Step 2: Buffer ----------
		std::shared_ptr<Argument> buf_arg;

		for (const auto& arg : arg_list_)
		{
			if (arg->matchesPointer(name, buffer_size, nullptr))
			{
				buf_arg = arg;
				DBGPRINT(L"Reusing buffer arg: ptr=%p buffer=%p\n",
					arg.get(), arg->buffer_);
				break;
			}
		}

		if (!buf_arg)
		{
			DBGPRINT(L"Creating buffer for texture copy\n");

			buf_arg = Argument::createFromPointer(
				context_, flags, name, buffer_size, nullptr);

			if (!buf_arg || !buf_arg->valid_)
			{
				DBGPRINT(L"ERROR: Failed to create buffer argument\n");
				return {};
			}

			arg_list_.insert(buf_arg);

			DBGPRINT(L"Buffer created: buffer=%p size=%zu\n",
				buf_arg->buffer_, buffer_size);
		}

		if (!tex_arg->buffer_ || !buf_arg->buffer_)
		{
			DBGPRINT(L"ERROR: Invalid CL buffers\n");
			return {};
		}

		// ---------- Step 3: Acquire GL ----------
		DBGPRINT(L"Acquiring GL texture\n");

		cl_int err = clEnqueueAcquireGLObjects(
			queue_, 1, &tex_arg->buffer_, 0, nullptr, nullptr);

		DBGPRINT(L"clEnqueueAcquireGLObjects err=%d\n", err);

		if (err != CL_SUCCESS)
		{
			DBGPRINT(L"ERROR: Failed to acquire GL object\n");
			return {};
		}

		// ---------- Step 4: Copy ----------
		size_t origin[3] = { 0, 0, 0 };

		DBGPRINT(L"Copying image to buffer\n");

		err = clEnqueueCopyImageToBuffer(
			queue_,
			tex_arg->buffer_,
			buf_arg->buffer_,
			origin,
			region,
			0,
			0, nullptr, nullptr
		);

		DBGPRINT(L"clEnqueueCopyImageToBuffer err=%d\n", err);

		if (err != CL_SUCCESS)
		{
			DBGPRINT(L"ERROR: Copy failed, releasing GL object\n");

			clEnqueueReleaseGLObjects(
				queue_, 1, &tex_arg->buffer_, 0, nullptr, nullptr);

			return {};
		}

		// ---------- Step 5: Release GL ----------
		DBGPRINT(L"Releasing GL texture\n");

		err = clEnqueueReleaseGLObjects(
			queue_, 1, &tex_arg->buffer_, 0, nullptr, nullptr);

		DBGPRINT(L"clEnqueueReleaseGLObjects err=%d\n", err);

		if (err != CL_SUCCESS)
		{
			DBGPRINT(L"ERROR: Failed to release GL object\n");
			return {};
		}

		// ---------- Step 6: Bind buffer ----------
		DBGPRINT(L"Binding output buffer to kernel: %S\n",
			kernels_[kernel_idx_].name.c_str());

		err = clSetKernelArg(
			kernels_[kernel_idx_].kernel,
			arg_idx_,
			sizeof(cl_mem),
			&buf_arg->buffer_
		);

		DBGPRINT(L"clSetKernelArg(buffer copy) index=%d err=%d\n",
			arg_idx_, err);

		if (err != CL_SUCCESS)
		{
			DBGPRINT(L"ERROR: Failed to bind buffer argument\n");
			return {};
		}

		// ---------- Step 7: Track ----------
		arg_map_[kernel_idx_].insert(buf_arg);
		++arg_idx_;

		DBGPRINT(L"Buffer mapped to kernel, new arg_idx_=%d\n", arg_idx_);

		// ---------- Step 8: Sync ----------
		DBGPRINT(L"Flushing + finishing queue\n");

		err = clFlush(queue_);
		DBGPRINT(L"clFlush err=%d\n", err);

		err = clFinish(queue_);
		DBGPRINT(L"clFinish err=%d\n", err);

		DBGPRINT(L"copyTex3DToBuf() completed successfully\n");

		return std::weak_ptr<Argument>(buf_arg);
	}

	bool KernelProgram::copyBufToTex3D(
		std::weak_ptr<Argument> buf_arg,
		GLuint texture_id,
		size_t size,
		size_t* region)
	{
		DBGPRINT(L"copyBufToTex3D() called: texture=%u, size=%zu\n",
			texture_id, size);

		if (!queue_ || !context_)
		{
			DBGPRINT(L"ERROR: queue_ or context_ is null\n");
			return false;
		}

		if (!region)
		{
			DBGPRINT(L"ERROR: region is null\n");
			return false;
		}

		DBGPRINT(L"Region: %zu x %zu x %zu\n",
			region[0], region[1], region[2]);

		// ---------- Lock buffer ----------
		auto buffer = buf_arg.lock();

		if (!buffer)
		{
			DBGPRINT(L"ERROR: buffer weak_ptr expired\n");
			return false;
		}

		if (!buffer->valid_ || !buffer->buffer_)
		{
			DBGPRINT(L"ERROR: buffer invalid or null\n");
			return false;
		}

		DBGPRINT(L"Source buffer: ptr=%p buffer=%p size=%zu\n",
			buffer.get(),
			buffer->buffer_,
			buffer->size_);

		if (size > buffer->size_)
		{
			DBGPRINT(L"WARNING: requested copy size (%zu) > buffer size (%zu)\n",
				size, buffer->size_);
		}

		// ---------- Find texture ----------
		std::shared_ptr<Argument> texture;

		for (const auto& arg : arg_list_)
		{
			if (arg->matchesTexture(texture_id))
			{
				texture = arg;
				DBGPRINT(L"Found texture arg: ptr=%p buffer=%p\n",
					arg.get(), arg->buffer_);
				break;
			}
		}

		if (!texture)
		{
			DBGPRINT(L"ERROR: texture not found in arg_list_\n");
			return false;
		}

		if (!texture->valid_ || !texture->buffer_)
		{
			DBGPRINT(L"ERROR: texture invalid or buffer null\n");
			return false;
		}

		// ---------- Acquire GL ----------
		DBGPRINT(L"Acquiring GL texture\n");

		cl_int err = clEnqueueAcquireGLObjects(
			queue_, 1, &texture->buffer_, 0, nullptr, nullptr);

		DBGPRINT(L"clEnqueueAcquireGLObjects err=%d\n", err);

		if (err != CL_SUCCESS)
		{
			DBGPRINT(L"ERROR: Failed to acquire GL object\n");
			return false;
		}

		// ---------- Copy ----------
		size_t origin[3] = { 0, 0, 0 };

		DBGPRINT(L"Copying buffer to 3D texture\n");

		err = clEnqueueCopyBufferToImage(
			queue_,
			buffer->buffer_,
			texture->buffer_,
			0,
			origin,
			region,
			0, nullptr, nullptr
		);

		DBGPRINT(L"clEnqueueCopyBufferToImage err=%d\n", err);

		if (err != CL_SUCCESS)
		{
			DBGPRINT(L"ERROR: Copy failed, releasing GL object\n");

			clEnqueueReleaseGLObjects(
				queue_, 1, &texture->buffer_, 0, nullptr, nullptr);

			return false;
		}

		// ---------- Release GL ----------
		DBGPRINT(L"Releasing GL texture\n");

		err = clEnqueueReleaseGLObjects(
			queue_, 1, &texture->buffer_, 0, nullptr, nullptr);

		DBGPRINT(L"clEnqueueReleaseGLObjects err=%d\n", err);

		if (err != CL_SUCCESS)
		{
			DBGPRINT(L"ERROR: Failed to release GL object\n");
			return false;
		}

		// ---------- Sync ----------
		DBGPRINT(L"Flushing + finishing queue\n");

		err = clFlush(queue_);
		DBGPRINT(L"clFlush err=%d\n", err);

		err = clFinish(queue_);
		DBGPRINT(L"clFinish err=%d\n", err);

		DBGPRINT(L"copyBufToTex3D() completed successfully\n");

		return true;
	}

	std::weak_ptr<Argument> KernelProgram::bindVeretxBuf(
		cl_mem_flags flags,
		GLuint vbo_id,
		size_t size)
	{
		DBGPRINT(L"bindVeretxBuf() called: kernel_idx_=%d, arg_idx_=%d, vbo_id=%u, size=%zu\n",
			kernel_idx_, arg_idx_, vbo_id, size);

		// --- Validate kernel ---
		if (kernel_idx_ < 0 || kernel_idx_ >= (int)kernels_.size())
		{
			DBGPRINT(L"ERROR: Invalid kernel index=%d (size=%zu)\n",
				kernel_idx_, kernels_.size());
			return {};
		}

		if (!context_)
		{
			DBGPRINT(L"ERROR: context_ is null\n");
			return {};
		}

		if (!vbo_id)
		{
			DBGPRINT(L"ERROR: vbo_id is 0\n");
			return {};
		}

		if (!glIsBuffer(vbo_id))
		{
			DBGPRINT(L"ERROR: vbo_id=%u is not a valid GL buffer\n", vbo_id);
			return {};
		}

		DBGPRINT(L"GL VBO validated: %u\n", vbo_id);

		std::shared_ptr<Argument> existing_arg;

		// ---------- Step 1: reuse ----------
		for (const auto& arg : arg_list_)
		{
			if (arg->matchesVBO(vbo_id))
			{
				existing_arg = arg;
				DBGPRINT(L"Reusing existing VBO arg: ptr=%p, buffer=%p\n",
					arg.get(), arg->buffer_);
				break;
			}
		}

		// ---------- Step 2: create ----------
		if (!existing_arg)
		{
			DBGPRINT(L"Creating new VBO argument\n");

			existing_arg = Argument::createFromVBO(
				context_, flags, vbo_id, size);

			if (!existing_arg)
			{
				DBGPRINT(L"ERROR: createFromVBO returned null\n");
				return {};
			}

			if (!existing_arg->valid_)
			{
				DBGPRINT(L"ERROR: created VBO argument invalid\n");
				return {};
			}

			if (!existing_arg->buffer_)
			{
				DBGPRINT(L"ERROR: created buffer_ is null\n");
				return {};
			}

			DBGPRINT(L"New VBO buffer created: ptr=%p, buffer=%p\n",
				existing_arg.get(), existing_arg->buffer_);

			arg_list_.insert(existing_arg);
		}

		// ---------- Validate buffer ----------
		if (!existing_arg->buffer_)
		{
			DBGPRINT(L"ERROR: buffer_ is null\n");
			return {};
		}

		// ---------- Step 3: bind ----------
		DBGPRINT(L"Binding VBO to kernel: %S\n",
			kernels_[kernel_idx_].name.c_str());

		cl_int err = clSetKernelArg(
			kernels_[kernel_idx_].kernel,
			arg_idx_,
			sizeof(cl_mem),
			&(existing_arg->buffer_)
		);

		DBGPRINT(L"clSetKernelArg(VBO) index=%d err=%d\n",
			arg_idx_, err);

		if (err != CL_SUCCESS)
		{
			DBGPRINT(L"ERROR: Failed to bind VBO argument\n");
			return {};
		}

		// ---------- Step 4: track ----------
		arg_map_[kernel_idx_].insert(existing_arg);

		DBGPRINT(L"VBO mapped to kernel index=%d\n", kernel_idx_);

		// increment only after success
		++arg_idx_;

		DBGPRINT(L"bindVeretxBuf() success, next arg_idx_=%d\n", arg_idx_);

		return std::weak_ptr<Argument>(existing_arg);
	}

	bool KernelProgram::readBuffer(const std::weak_ptr<Argument> arg, void* data)
	{
		DBGPRINT(L"readBuffer() called\n");

		if (!queue_)
		{
			DBGPRINT(L"ERROR: queue_ is null\n");
			return false;
		}

		if (!data)
		{
			DBGPRINT(L"ERROR: output data pointer is null\n");
			return false;
		}

		// ---------- Lock argument ----------
		auto shared_arg = arg.lock();

		if (!shared_arg)
		{
			DBGPRINT(L"ERROR: Argument weak_ptr expired\n");
			return false;
		}

		if (!shared_arg->valid_ || !shared_arg->buffer_)
		{
			DBGPRINT(L"ERROR: Argument invalid or buffer null\n");
			return false;
		}

		DBGPRINT(L"Reading buffer: ptr=%p buffer=%p size=%zu\n",
			shared_arg.get(),
			shared_arg->buffer_,
			shared_arg->size_);

		cl_int err = CL_SUCCESS;
		bool needs_acquire = false;

		// ---------- Determine interop ----------
		if (shared_arg->tex_ && glIsTexture(shared_arg->tex_) && shared_arg->size_ == 0)
		{
			DBGPRINT(L"Argument is GL texture, acquiring\n");
			needs_acquire = true;
		}
		else if (shared_arg->vbo_ && glIsBuffer(shared_arg->vbo_))
		{
			DBGPRINT(L"Argument is GL VBO, acquiring\n");
			needs_acquire = true;
		}

		// ---------- Acquire ----------
		if (needs_acquire)
		{
			err = clEnqueueAcquireGLObjects(
				queue_, 1, &shared_arg->buffer_, 0, nullptr, nullptr);

			DBGPRINT(L"clEnqueueAcquireGLObjects err=%d\n", err);

			if (err != CL_SUCCESS)
			{
				DBGPRINT(L"ERROR: Failed to acquire GL object\n");
				return false;
			}
		}

		// ---------- Read ----------
		DBGPRINT(L"Reading buffer data\n");

		err = clEnqueueReadBuffer(
			queue_,
			shared_arg->buffer_,
			CL_TRUE,
			0,
			shared_arg->size_,
			data,
			0, nullptr, nullptr
		);

		DBGPRINT(L"clEnqueueReadBuffer size=%zu err=%d\n",
			shared_arg->size_, err);

		if (err != CL_SUCCESS)
		{
			DBGPRINT(L"ERROR: clEnqueueReadBuffer failed\n");

			if (needs_acquire)
			{
				DBGPRINT(L"Releasing GL object after read failure\n");
				clEnqueueReleaseGLObjects(
					queue_, 1, &shared_arg->buffer_, 0, nullptr, nullptr);
			}

			return false;
		}

		// ---------- Release ----------
		if (needs_acquire)
		{
			DBGPRINT(L"Releasing GL object\n");

			err = clEnqueueReleaseGLObjects(
				queue_, 1, &shared_arg->buffer_, 0, nullptr, nullptr);

			DBGPRINT(L"clEnqueueReleaseGLObjects err=%d\n", err);

			if (err != CL_SUCCESS)
			{
				DBGPRINT(L"ERROR: Failed to release GL object\n");
				return false;
			}
		}

		// ---------- Sync ----------
		DBGPRINT(L"Flushing + finishing queue\n");

		err = clFlush(queue_);
		DBGPRINT(L"clFlush err=%d\n", err);

		err = clFinish(queue_);
		DBGPRINT(L"clFinish err=%d\n", err);

		DBGPRINT(L"readBuffer() completed successfully\n");

		return true;
	}

	//release mem obj
	void KernelProgram::releaseAllArgs()
	{
		arg_list_.clear();      // shared_ptrs dropped
		arg_map_.clear();       // weak_ptrs dropped
	}

	void KernelProgram::releaseKernelArgs(int kernel_idx)
	{
		if (kernel_idx < 0 || kernel_idx >= kernels_.size()) return;

		auto& used_args = arg_map_[kernel_idx];

		for (const auto& weak_arg : used_args) {
			auto shared_arg = weak_arg.lock();
			if (!shared_arg) continue;

			// Remove from global list only if no other kernel uses it
			bool used_elsewhere = false;
			for (const auto& [other_idx, other_args] : arg_map_) {
				if (other_idx == kernel_idx) continue;
				for (const auto& other_weak : other_args) {
					if (other_weak.lock() == shared_arg) {
						used_elsewhere = true;
						break;
					}
				}
				if (used_elsewhere) break;
			}

			if (!used_elsewhere) {
				arg_list_.erase(shared_arg); // shared_ptr dropped
			}
		}

		arg_map_.erase(kernel_idx); // weak_ptrs dropped
	}

	void KernelProgram::releaseArg(const std::weak_ptr<Argument>& arg)
	{
		auto shared_arg = arg.lock();
		if (!shared_arg) return;

		// Remove from global argument list
		arg_list_.erase(shared_arg);

		// Remove from each kernel's argument set
		for (auto& [kidx, args] : arg_map_) {
			for (auto it = args.begin(); it != args.end(); ) {
				if (it->lock() == shared_arg) {
					it = args.erase(it); // erase returns next valid iterator
				}
				else {
					++it;
				}
			}
		}
	}

	std::string& KernelProgram::getInfo()
	{
		return info_;
	}

	bool KernelProgram::get_group_size(int index,
		unsigned int nx, unsigned int ny, unsigned int nz,
		GroupSize& ksize)
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

		ksize.gsx = nx / ksize.ngx + (nx % ksize.ngx ? 1 : 0);
		ksize.gsy = ny / ksize.ngy + (ny % ksize.ngy ? 1 : 0);
		ksize.gsz = nz / ksize.ngz + (nz % ksize.ngz ? 1 : 0);
		ksize.gsxyz = ksize.gsx * ksize.gsy * ksize.gsz;
		ksize.gsxy = ksize.gsx * ksize.gsy;

		return true;
	}

	bool KernelProgram::get_group_size2(int index,
		unsigned int nx, unsigned int ny, unsigned int nz,
		GroupSize& ksize)
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

		ksize.gsx = nx / ksize.ngx + (nx % ksize.ngx ? 1 : 0);
		ksize.gsy = ny / ksize.ngy + (ny % ksize.ngy ? 1 : 0);
		ksize.gsz = nz / ksize.ngz + (nz % ksize.ngz ? 1 : 0);
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
