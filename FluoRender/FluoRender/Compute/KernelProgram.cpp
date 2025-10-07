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
#include <Global.h>
#include <MainSettings.h>
#include <KernelProgram.h>
#include <Debug.h>
#ifdef _WIN32
#define WINDOWS_LEAN_AND_MEAN
#include <Windows.h>
#endif
#include <algorithm>
#include <cmath>
#include <sstream>
#include <string>

namespace flvr
{
	std::shared_ptr<Argument> Argument::createFromTexture2D(cl_context context, cl_mem_flags flags, GLuint tex_id)
	{
		cl_int err = CL_SUCCESS;
		cl_mem buf = clCreateFromGLTexture(
			context,
			flags,
			GL_TEXTURE_2D,
			0,          // mip level
			tex_id,
			&err
		);

		if (err != CL_SUCCESS || !buf) {
			return nullptr;
		}

		auto arg = std::make_shared<Argument>();
		arg->type_ = ArgType_Tex;
		arg->buffer_ = buf;
		arg->tex_ = tex_id;
		arg->valid_ = true;
		return arg;
	}

	std::shared_ptr<Argument> Argument::createFromTexture3D(cl_context context, cl_mem_flags flags, GLuint tex_id)
	{
		cl_int err = CL_SUCCESS;
		cl_mem buf = clCreateFromGLTexture(
			context,
			flags,
			GL_TEXTURE_3D,
			0,          // mip level
			tex_id,
			&err
		);

		if (err != CL_SUCCESS || !buf) {
			return nullptr;
		}

		auto arg = std::make_shared<Argument>();
		arg->type_ = ArgType_Tex;
		arg->buffer_ = buf;
		arg->tex_ = tex_id;
		arg->valid_ = true;
		return arg;
	}

	bool KernelProgram::init_ = false;
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
				if (!device->vendor.empty() && device->vendor.back() == '\0')
					device->vendor.pop_back();
				//get device name
				err = clGetDeviceInfo(devices[j], CL_DEVICE_NAME, 0, NULL, &info_size);
				device->name.resize(info_size, 0);
				err = clGetDeviceInfo(devices[j], CL_DEVICE_NAME, info_size, &(device->name[0]), NULL);
				if (!device->name.empty() && device->name.back() == '\0')
					device->name.pop_back();
				//get device version
				err = clGetDeviceInfo(devices[j], CL_DEVICE_OPENCL_C_VERSION, 0, NULL, &info_size);
				device->version.resize(info_size, 0);
				err = clGetDeviceInfo(devices[j], CL_DEVICE_OPENCL_C_VERSION, info_size, &(device->version[0]), NULL);
				if (!device->version.empty() && device->version.back() == '\0')
					device->version.pop_back();
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

		//check features
		err = clGetDeviceInfo(device_, CL_DEVICE_EXTENSIONS, 0, NULL, &info_size);
		std::string extensions(info_size, '\0');
		err = clGetDeviceInfo(device_, CL_DEVICE_EXTENSIONS, info_size, &extensions[0], NULL);
		float_atomics_ = extensions.find("cl_khr_global_float_atomics") != std::string::npos;

		//check if needs clear
		need_clear_ = get_need_clear(device);
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

	void KernelProgram::release_context()
	{
		if (queue_)
			clReleaseCommandQueue(queue_);
		if (context_)
			clReleaseContext(context_);
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
#if defined(__APPLE__)
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

	bool KernelProgram::valid()
	{
		return init_ && program_ && queue_ && !kernels_.empty();
	}

	void KernelProgram::destroy()
	{
		for (size_t i = 0; i < kernels_.size(); ++i)
			if (!kernels_[i].external)
				clReleaseKernel(kernels_[i].kernel);
		clReleaseProgram(program_);
	}

	bool KernelProgram::matches(const std::string& s)
	{
		return source_ == s;
	}

	bool KernelProgram::executeKernel(int index, cl_uint dim, size_t* global_size, size_t* local_size)
	{
		if (!valid() || index < 0 || index >= kernels_.size()) {
			return false;
		}

		bool result = true;
		cl_int err = CL_SUCCESS;

		glFinish(); // Ensure GL state is settled before acquiring

		// Step 1: Acquire GL objects used by this kernel
		auto& args = arg_map_[index];
		for (const auto& weak_arg : args) {
			auto arg = weak_arg.lock();
			if (!arg || !arg->valid_ || !arg->buffer_) continue;

			if (arg->tex_ && glIsTexture(arg->tex_) && arg->size_ == 0) {
				err = clEnqueueAcquireGLObjects(queue_, 1, &arg->buffer_, 0, nullptr, nullptr);
				if (err != CL_SUCCESS) result = false;
			}
			else if (arg->vbo_ && glIsBuffer(arg->vbo_)) {
				err = clEnqueueAcquireGLObjects(queue_, 1, &arg->buffer_, 0, nullptr, nullptr);
				if (err != CL_SUCCESS) result = false;
			}
		}

		// Step 2: Execute kernel
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
		if (err != CL_SUCCESS) {
			result = false;
		}

		// Step 3: Release GL objects
		for (const auto& weak_arg : args) {
			auto arg = weak_arg.lock();
			if (!arg || !arg->valid_ || !arg->buffer_) continue;

			if (arg->tex_ && glIsTexture(arg->tex_) && arg->size_ == 0) {
				err = clEnqueueReleaseGLObjects(queue_, 1, &arg->buffer_, 0, nullptr, nullptr);
				if (err != CL_SUCCESS) result = false;
			}
			else if (arg->vbo_ && glIsBuffer(arg->vbo_)) {
				err = clEnqueueReleaseGLObjects(queue_, 1, &arg->buffer_, 0, nullptr, nullptr);
				if (err != CL_SUCCESS) result = false;
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

	bool KernelProgram::setConst(size_t size, void* data)
	{
		cl_int err;

		if (!data)
			return false;
		if (kernel_idx_ < 0 || kernel_idx_ >= kernels_.size())
			return false;

		err = clSetKernelArg(kernels_[kernel_idx_].kernel, arg_idx_++, size, data);
		if (err != CL_SUCCESS)
			return false;
		return true;
	}

	bool KernelProgram::setLocal(size_t size)
	{
		cl_int err = clSetKernelArg(
			kernels_[kernel_idx_].kernel,
			arg_idx_++, size, NULL);
		if (err != CL_SUCCESS)
			return false;
		return true;
	}

	bool KernelProgram::bindArg(std::weak_ptr<Argument> arg)
	{
		// Attempt to lock the weak_ptr
		auto shared_arg = arg.lock();
		if (!shared_arg) {
			// Argument has expired — return empty weak_ptr
			return false;
		}

		// Check if the argument is already in the program's list
		auto it = arg_list_.find(shared_arg);
		if (it == arg_list_.end()) {
			// Not found — insert into the list
			arg_list_.insert(shared_arg);
		}

		// Update the kernel-to-argument map
		arg_map_[kernel_idx_].insert(shared_arg);

		// Call clSetKernelArg
		cl_int err = clSetKernelArg(
			kernels_[kernel_idx_].kernel,
			arg_idx_,
			sizeof(cl_mem),
			&(shared_arg->buffer_)
		);

		if (err != CL_SUCCESS) {
			// Optionally log or handle error
			return false; // return empty on failure
		}

		// Increment internal argument index
		++arg_idx_;

		// Return the original weak_ptr
		return true;
	}

	std::weak_ptr<Argument> KernelProgram::setBufNew(cl_mem_flags flags, const std::string& name, size_t size, void* data)
	{
		// Step 1: Check if an Argument already exists for this raw pointer
		std::shared_ptr<Argument> existing_arg;
		for (const auto& arg : arg_list_) {
			if (arg->matchesPointer(name, size, data)) {
				existing_arg = arg;
				break;
			}
		}

		// Step 2: If not found, create a new Argument
		if (!existing_arg) {
			existing_arg = Argument::createFromPointer(context_, flags, name, size, data);
			if (!existing_arg || !existing_arg->valid_) {
				// Failed to create buffer — return empty weak_ptr
				return std::weak_ptr<Argument>();
			}

			// Add to program's argument list
			arg_list_.insert(existing_arg);
		}

		// Step 3: Set as kernel argument
		cl_int err = clSetKernelArg(
			kernels_[kernel_idx_].kernel,
			arg_idx_,
			sizeof(cl_mem),
			&existing_arg->buffer_
		);

		if (err != CL_SUCCESS) {
			// Optionally log error
			return std::weak_ptr<Argument>();
		}

		// Step 4: Track usage for this kernel
		arg_map_[kernel_idx_].insert(existing_arg);

		// Step 5: Increment internal argument index
		++arg_idx_;

		// Step 6: Return weak_ptr for external use
		return std::weak_ptr<Argument>(existing_arg);
	}

	std::weak_ptr<Argument> KernelProgram::setBufNewOrUpdate(cl_mem_flags flags, const std::string& name, size_t size, void* data)
	{
		std::shared_ptr<Argument> arg;
		for (const auto& existing : arg_list_) {
			if (existing->matchesPointer(name, size, data)) {
				arg = existing;
				break;
			}
		}

		if (!arg) {
			arg = Argument::createFromPointer(context_, flags, name, size, data);
			if (!arg || !arg->valid_) return std::weak_ptr<Argument>();
			arg_list_.insert(arg);
		}
		else {
			if (!updateBuf(arg, flags, size, data)) {
				return std::weak_ptr<Argument>();
			}
		}

		cl_int err = clSetKernelArg(kernels_[kernel_idx_].kernel, arg_idx_, sizeof(cl_mem), &arg->buffer_);
		if (err != CL_SUCCESS) return std::weak_ptr<Argument>();

		arg_map_[kernel_idx_].insert(arg);
		++arg_idx_;
		return std::weak_ptr<Argument>(arg);
	}

	bool KernelProgram::updateBuf(std::weak_ptr<Argument> arg, cl_mem_flags flags, size_t new_size, void* data)
	{
		auto shared_arg = arg.lock();
		if (!shared_arg || !shared_arg->valid_ || !shared_arg->buffer_) {
			return false;
		}

		// Case 1: Size fits — just write new data
		if (new_size <= shared_arg->size_) {
			cl_int err = clEnqueueWriteBuffer(
				queue_,
				shared_arg->buffer_,
				CL_TRUE,
				0,
				new_size,
				data,
				0, nullptr, nullptr
			);
			return err == CL_SUCCESS;
		}

		// Case 2: Size too large — destroy and reallocate
		shared_arg->release(); // safely releases cl_mem if not protected

		cl_int err = CL_SUCCESS;
		cl_mem new_buf = clCreateBuffer(context_, flags, new_size, data, &err);
		if (err != CL_SUCCESS || !new_buf) {
			return false;
		}

		// Update shared Argument in-place
		shared_arg->buffer_ = new_buf;
		shared_arg->size_ = new_size;
		shared_arg->pointer_ = data;
		shared_arg->valid_ = true;

		return true;
	}

	std::weak_ptr<Argument> KernelProgram::setTex2D(cl_mem_flags flags, GLuint tex_id)
	{
		// Step 1: Check if texture already wrapped
		std::shared_ptr<Argument> existing_arg;
		for (const auto& arg : arg_list_) {
			if (arg->matchesTexture(tex_id)) {
				existing_arg = arg;
				break;
			}
		}

		// Step 2: Create new Argument if not found
		if (!existing_arg) {
			existing_arg = Argument::createFromTexture2D(context_, flags, tex_id);
			if (!existing_arg || !existing_arg->valid_) {
				return std::weak_ptr<Argument>();
			}
			arg_list_.insert(existing_arg);
		}

		// Step 3: Set kernel argument
		cl_int err = clSetKernelArg(
			kernels_[kernel_idx_].kernel,
			arg_idx_,
			sizeof(cl_mem),
			&existing_arg->buffer_
		);

		if (err != CL_SUCCESS) {
			return std::weak_ptr<Argument>();
		}

		// Step 4: Track usage
		arg_map_[kernel_idx_].insert(existing_arg);
		++arg_idx_;

		return std::weak_ptr<Argument>(existing_arg);
	}

	std::weak_ptr<Argument> KernelProgram::setTex3D(cl_mem_flags flags, GLuint tex_id)
	{
		// Step 1: Check if texture already wrapped
		std::shared_ptr<Argument> existing_arg;
		for (const auto& arg : arg_list_) {
			if (arg->matchesTexture(tex_id)) {
				existing_arg = arg;
				break;
			}
		}

		// Step 2: Create new Argument if not found
		if (!existing_arg) {
			existing_arg = Argument::createFromTexture3D(context_, flags, tex_id);
			if (!existing_arg || !existing_arg->valid_) {
				return std::weak_ptr<Argument>();
			}
			arg_list_.insert(existing_arg);
		}

		// Step 3: Set kernel argument
		cl_int err = clSetKernelArg(
			kernels_[kernel_idx_].kernel,
			arg_idx_,
			sizeof(cl_mem),
			&existing_arg->buffer_
		);

		if (err != CL_SUCCESS) {
			return std::weak_ptr<Argument>();
		}

		// Step 4: Track usage
		arg_map_[kernel_idx_].insert(existing_arg);
		++arg_idx_;

		return std::weak_ptr<Argument>(existing_arg);
	}

	std::weak_ptr<Argument> KernelProgram::copyTex3DToBuf(
		cl_mem_flags flags, GLuint texture_id,
		const std::string& name,
		size_t buffer_size, size_t* region)
	{
		if (kernel_idx_ < 0 || kernel_idx_ >= kernels_.size()) {
			return std::weak_ptr<Argument>();
		}

		// Step 1: Find or create the texture wrapper
		std::shared_ptr<Argument> tex_arg;
		for (const auto& arg : arg_list_) {
			if (arg->matchesTexture(texture_id)) {
				tex_arg = arg;
				break;
			}
		}

		if (!tex_arg) {
			tex_arg = Argument::createFromTexture3D(context_, CL_MEM_READ_WRITE, texture_id);
			if (!tex_arg || !tex_arg->valid_) {
				return std::weak_ptr<Argument>();
			}
			arg_list_.insert(tex_arg);
		}

		// Step 2: Find or create the buffer to hold copied texture data
		std::shared_ptr<Argument> buf_arg;
		for (const auto& arg : arg_list_) {
			if (arg->matchesPointer(name, buffer_size, nullptr)) {
				buf_arg = arg;
				break;
			}
		}

		if (!buf_arg) {
			buf_arg = Argument::createFromPointer(context_, flags, name, buffer_size, nullptr);
			if (!buf_arg || !buf_arg->valid_) {
				return std::weak_ptr<Argument>();
			}
			arg_list_.insert(buf_arg);
		}

		// Step 3: Acquire GL texture for OpenCL access
		cl_int err = clEnqueueAcquireGLObjects(queue_, 1, &tex_arg->buffer_, 0, nullptr, nullptr);
		if (err != CL_SUCCESS) {
			return std::weak_ptr<Argument>();
		}

		// Step 4: Copy texture to buffer
		size_t origin[3] = { 0, 0, 0 };
		err = clEnqueueCopyImageToBuffer(
			queue_,
			tex_arg->buffer_,
			buf_arg->buffer_,
			origin,
			region,
			0,
			0, nullptr, nullptr
		);
		if (err != CL_SUCCESS) {
			clEnqueueReleaseGLObjects(queue_, 1, &tex_arg->buffer_, 0, nullptr, nullptr);
			return std::weak_ptr<Argument>();
		}

		// Step 5: Release GL texture
		err = clEnqueueReleaseGLObjects(queue_, 1, &tex_arg->buffer_, 0, nullptr, nullptr);
		if (err != CL_SUCCESS) {
			return std::weak_ptr<Argument>();
		}

		// Step 6: Set buffer_ as kernel argument
		err = clSetKernelArg(
			kernels_[kernel_idx_].kernel,
			arg_idx_,
			sizeof(cl_mem),
			&buf_arg->buffer_
		);
		if (err != CL_SUCCESS) {
			return std::weak_ptr<Argument>();
		}

		// Step 7: Track usage
		arg_map_[kernel_idx_].insert(buf_arg);
		++arg_idx_;

		// Step 8: Flush and finish
		clFlush(queue_);
		clFinish(queue_);

		return std::weak_ptr<Argument>(buf_arg);
	}

	bool KernelProgram::copyBufToTex3D(std::weak_ptr<Argument> buf_arg, GLuint texture_id, size_t size, size_t* region)
	{
		auto buffer = buf_arg.lock();
		if (!buffer || !buffer->valid_ || !buffer->buffer_) {
			return false;
		}

		// Step 1: Find the texture Argument
		std::shared_ptr<Argument> texture;
		for (const auto& arg : arg_list_) {
			if (arg->matchesTexture(texture_id)) {
				texture = arg;
				break;
			}
		}

		if (!texture || !texture->valid_ || !texture->buffer_) {
			return false;
		}

		// Step 2: Acquire GL texture
		cl_int err = clEnqueueAcquireGLObjects(queue_, 1, &texture->buffer_, 0, nullptr, nullptr);
		if (err != CL_SUCCESS) {
			return false;
		}

		// Step 3: Copy buffer to texture
		size_t origin[3] = { 0, 0, 0 };
		err = clEnqueueCopyBufferToImage(
			queue_,
			buffer->buffer_,
			texture->buffer_,
			0,
			origin,
			region,
			0, nullptr, nullptr
		);
		if (err != CL_SUCCESS) {
			clEnqueueReleaseGLObjects(queue_, 1, &texture->buffer_, 0, nullptr, nullptr);
			return false;
		}

		// Step 4: Release GL texture
		err = clEnqueueReleaseGLObjects(queue_, 1, &texture->buffer_, 0, nullptr, nullptr);
		if (err != CL_SUCCESS) {
			return false;
		}

		// Step 5: Finalize
		clFlush(queue_);
		clFinish(queue_);

		return true;
	}

	std::weak_ptr<Argument> KernelProgram::bindVeretxBuf(cl_mem_flags flags, GLuint vbo_id, size_t size)
	{
		if (kernel_idx_ < 0 || kernel_idx_ >= kernels_.size()) {
			return std::weak_ptr<Argument>();
		}

		// Step 1: Check if VBO is already wrapped
		std::shared_ptr<Argument> existing_arg;
		for (const auto& arg : arg_list_) {
			if (arg->matchesVBO(vbo_id)) {
				existing_arg = arg;
				break;
			}
		}

		// Step 2: Create new Argument if not found
		if (!existing_arg) {
			existing_arg = Argument::createFromVBO(context_, flags, vbo_id, size);
			if (!existing_arg || !existing_arg->valid_) {
				return std::weak_ptr<Argument>();
			}
			arg_list_.insert(existing_arg);
		}

		// Step 3: Set kernel argument
		cl_int err = clSetKernelArg(
			kernels_[kernel_idx_].kernel,
			arg_idx_,
			sizeof(cl_mem),
			&existing_arg->buffer_
		);

		if (err != CL_SUCCESS) {
			return std::weak_ptr<Argument>();
		}

		// Step 4: Track usage
		arg_map_[kernel_idx_].insert(existing_arg);
		++arg_idx_;

		return std::weak_ptr<Argument>(existing_arg);
	}

	bool KernelProgram::readBuffer(const std::weak_ptr<Argument> arg, void* data)
	{
		auto shared_arg = arg.lock();
		if (!shared_arg || !shared_arg->valid_ || !shared_arg->buffer_) {
			return false;
		}

		cl_int err = CL_SUCCESS;

		// Acquire GL texture if applicable
		if (shared_arg->tex_ && glIsTexture(shared_arg->tex_) && shared_arg->size_ == 0) {
			err = clEnqueueAcquireGLObjects(queue_, 1, &shared_arg->buffer_, 0, nullptr, nullptr);
			if (err != CL_SUCCESS) return false;
		}
		// Acquire VBO if applicable
		else if (shared_arg->vbo_ && glIsBuffer(shared_arg->vbo_)) {
			err = clEnqueueAcquireGLObjects(queue_, 1, &shared_arg->buffer_, 0, nullptr, nullptr);
			if (err != CL_SUCCESS) return false;
		}

		// Read buffer contents
		err = clEnqueueReadBuffer(
			queue_,
			shared_arg->buffer_,
			CL_TRUE,
			0,
			shared_arg->size_,
			data,
			0, nullptr, nullptr
		);
		if (err != CL_SUCCESS) return false;

		// Release GL texture if applicable
		if (shared_arg->tex_ && glIsTexture(shared_arg->tex_) && shared_arg->size_ == 0) {
			err = clEnqueueReleaseGLObjects(queue_, 1, &shared_arg->buffer_, 0, nullptr, nullptr);
			if (err != CL_SUCCESS) return false;
		}
		// Release VBO if applicable
		else if (shared_arg->vbo_ && glIsBuffer(shared_arg->vbo_)) {
			err = clEnqueueReleaseGLObjects(queue_, 1, &shared_arg->buffer_, 0, nullptr, nullptr);
			if (err != CL_SUCCESS) return false;
		}

		clFlush(queue_);
		clFinish(queue_);
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
