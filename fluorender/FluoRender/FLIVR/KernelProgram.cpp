#include "KernelProgram.h"
#ifdef _WIN32
#include <Windows.h>
#endif

namespace FLIVR
{
	bool KernelProgram::init_ = false;
	cl_device_id KernelProgram::device_ = 0;
	cl_context KernelProgram::context_ = 0;
	int KernelProgram::platform_id_ = 0;
	int KernelProgram::device_id_ = 0;
	std::string KernelProgram::device_name_;
#ifdef _DARWIN
	CGLContextObj KernelProgram::gl_context_ = 0;
#endif
	KernelProgram::KernelProgram(const std::string& source) :
	source_(source), program_(0), queue_(0)
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

#ifdef _WIN32
		cl_context_properties properties[] =
		{
			CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
			CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
			CL_CONTEXT_PLATFORM, (cl_context_properties)0,
			0
		};

		typedef CL_API_ENTRY cl_int(CL_API_CALL *P1)(
			const cl_context_properties *properties,
			cl_gl_context_info param_name,
			size_t param_value_size,
			void *param_value,
			size_t *param_value_size_ret);
		CL_API_ENTRY cl_int(CL_API_CALL *myclGetGLContextInfoKHR)(
			const cl_context_properties *properties,
			cl_gl_context_info param_name,
			size_t param_value_size,
			void *param_value,
			size_t *param_value_size_ret) = NULL;
		myclGetGLContextInfoKHR = (P1)clGetExtensionFunctionAddress("clGetGLContextInfoKHR");
#endif
#if defined(_DARWIN) || defined(__linux__)
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

		for (cl_uint i = 0; i<platform_num; ++i)
		{
			if (i != platform_id_)
				continue;
#ifdef _WIN32
			cl_device_id device = 0;
#endif
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
#ifdef _WIN32
			//get GL device
			properties[5] = (cl_context_properties)(platforms[i]);
			/*if (myclGetGLContextInfoKHR)
			{
				bool found = false;
				err = myclGetGLContextInfoKHR(properties, CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR,
					sizeof(cl_device_id), &device, NULL);
				if (err != CL_SUCCESS || !device)
				{
					delete[] devices;
					continue;
				}
				else
				{
					for (cl_uint j=0; j<device_num; ++j)
					{
						if (device == devices[j])
						{
							device_ = device;
							found = true;
							break;
						}
					}
				}
				delete[] devices;
				if (!found)
					continue;
			}
			else*/
			{
				if (device_id_ >= 0 && device_id_ < device_num)
					device_ = devices[device_id_];
				else
					device_ = devices[0];
				delete[] devices;
			}
#endif
#if defined(_DARWIN) || defined(__linux__)
			properties[3] = (cl_context_properties)(platforms[i]);
			if (device_id_ >= 0 && device_id_ < device_num)
				device_ = devices[device_id_];
			else
				device_ = devices[0];
			delete[] devices;
#endif

			char buffer[10240];
			clGetDeviceInfo(device_, CL_DEVICE_NAME, sizeof(buffer), buffer, NULL);
			device_name_ = std::string(buffer);

			context_ = clCreateContext(properties, 1, &device_, NULL, NULL, &err);
			if (err == CL_SUCCESS)
				init_ = true;

			delete[] platforms;
			return;
		}
		delete[] platforms;
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
				delete []program_log;
				return -1;
			}
		}

		if (!queue_)
		{
			queue_ = clCreateCommandQueue(context_, device_, 0, &err);
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
				return kernels_.size() - 1;
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
				return i;
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
		return kernels_.size() - 1;
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
		if (!valid())
			return false;
		if (index < 0 || index >= kernels_.size())
			return false;

		cl_int err;
		glFinish();
		unsigned int i;
		for (i=0; i<arg_list_.size(); ++i)
		{
			if (arg_list_[i].kernel_index == index &&
				arg_list_[i].size == 0 &&
				arg_list_[i].texture &&
				glIsTexture(arg_list_[i].texture))
			{
				err = clEnqueueAcquireGLObjects(queue_, 1, &(arg_list_[i].buffer), 0, NULL, NULL);
				if (err != CL_SUCCESS)
					return false;
			}
		}
		err = clEnqueueNDRangeKernel(queue_, kernels_[index].kernel, dim, NULL, global_size,
			local_size, 0, NULL, NULL);
		if (err != CL_SUCCESS)
			return false;
		for (i=0; i<arg_list_.size(); ++i)
		{
			if (arg_list_[i].kernel_index == index &&
				arg_list_[i].size == 0 &&
				arg_list_[i].texture &&
				glIsTexture(arg_list_[i].texture))
			{
				err = clEnqueueReleaseGLObjects(queue_, 1, &(arg_list_[i].buffer), 0, NULL, NULL);
				if (err != CL_SUCCESS)
					return false;
			}
		}
		clFlush(queue_);
		clFinish(queue_);
		return true;
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
			if (arg_list_[i].kernel_index == arg.kernel_index &&
				arg_list_[i].index == arg.index &&
				arg_list_[i].size == arg.size &&
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
		unsigned int ai = -1;
		if (!matchArgBuf(arg, ai))
		{
			arg_list_.push_back(arg);
			ai = arg_list_.size() - 1;
		}
		cl_int err = clSetKernelArg(kernels_[arg.kernel_index].kernel,
			arg.index, sizeof(cl_mem), &(arg.buffer));
		return ai;
	}

	void KernelProgram::setKernelArgConst(int index, int i, size_t size, void* data)
	{
		cl_int err;

		if (!data)
			return;
		if (index < 0 || index >= kernels_.size())
			return;

		err = clSetKernelArg(kernels_[index].kernel, i, size, data);
		if (err != CL_SUCCESS)
			return;
	}

	void KernelProgram::setKernelArgConst(std::string &name, int i, size_t size, void* data)
	{
		int index = findKernel(name);
		return setKernelArgConst(index, i, size, data);
	}

	Argument KernelProgram::setKernelArgBuf(int index, int i, cl_mem_flags flag, size_t size, void* data)
	{
		Argument arg;
		cl_int err;
		if (index < 0 || index >= kernels_.size())
			return arg;

		arg.kernel_index = index;
		arg.index = i;
		arg.size = size;
		arg.texture = 0;
		arg.orgn_addr = data;

		unsigned int ai;
		bool found = false;
		if (data)
		{
			found = matchArgAddr(arg, ai);
			if (found)
				arg.buffer = arg_list_[ai].buffer;
		}

		if (!found)
		{
			arg.buffer = clCreateBuffer(context_, flag, size, data, &err);
			if (err != CL_SUCCESS)
				return Argument();
			arg_list_.push_back(arg);
		}

		err = clSetKernelArg(kernels_[index].kernel, i, sizeof(cl_mem), &(arg.buffer));
		if (err != CL_SUCCESS)
			return Argument();

		return arg;
	}

	Argument KernelProgram::setKernelArgBuf(std::string &name, int i, cl_mem_flags flag, size_t size, void* data)
	{
		int index = findKernel(name);
		return setKernelArgBuf(index, i, flag, size, data);
	}

	Argument KernelProgram::setKernelArgBufWrite(int index, int i, cl_mem_flags flag, size_t size, void* data)
	{
		Argument arg;
		cl_int err;
		cl_mem buffer = 0;
		if (index < 0 || index >= kernels_.size())
			return arg;

		if (data)
		{
			arg.kernel_index = index;
			arg.index = i;
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

			err = clSetKernelArg(kernels_[index].kernel, i, sizeof(cl_mem), &(arg.buffer));
			if (err != CL_SUCCESS)
				return Argument();
		}
		else
		{
			err = clSetKernelArg(kernels_[index].kernel, i, size, NULL);
		}
		return arg;
	}

	Argument KernelProgram::setKernelArgBufWrite(std::string &name, int i, cl_mem_flags flag, size_t size, void* data)
	{
		int index = findKernel(name);
		return setKernelArgBufWrite(index, i, flag, size, data);
	}

	Argument KernelProgram::setKernelArgTex2D(int index, int i, cl_mem_flags flag, GLuint texture)
	{
		Argument arg;
		cl_int err;
		cl_mem buffer = 0;
		if (index < 0 || index >= kernels_.size())
			return arg;

		arg.kernel_index = index;
		arg.index = i;
		arg.size = 0;
		arg.texture = texture;
		arg.orgn_addr = 0;

		unsigned int ai;
		if (matchArgTex(arg, ai))
		{
			arg.buffer = arg_list_[ai].buffer;
			buffer = arg.buffer;
		}
		else
		{
			buffer = clCreateFromGLTexture(context_, flag, GL_TEXTURE_2D, 0, texture, &err);
			if (err != CL_SUCCESS)
				return Argument();
			arg.buffer = buffer;
			arg_list_.push_back(arg);
		}
		err = clSetKernelArg(kernels_[index].kernel, i, sizeof(cl_mem), &(arg.buffer));
		if (err != CL_SUCCESS)
			return Argument();
		return arg;
	}

	Argument KernelProgram::setKernelArgTex2D(std::string &name, int i, cl_mem_flags flag, GLuint texture)
	{
		int index = findKernel(name);
		return setKernelArgTex2D(index, i, flag, texture);
	}

	Argument KernelProgram::setKernelArgTex3D(int index, int i, cl_mem_flags flag, GLuint texture)
	{
		Argument arg;
		cl_int err;
		cl_mem buffer = 0;
		if (index < 0 || index >= kernels_.size())
			return arg;

		arg.kernel_index = index;
		arg.index = i;
		arg.size = 0;
		arg.texture = texture;
		arg.orgn_addr = 0;

		unsigned int ai;
		if (matchArgTex(arg, ai))
		{
			arg.buffer = arg_list_[ai].buffer;
			buffer = arg.buffer;
		}
		else
		{
			buffer = clCreateFromGLTexture(context_, flag, GL_TEXTURE_3D, 0, texture, &err);
			if (err != CL_SUCCESS)
				return Argument();
			arg.buffer = buffer;
			arg_list_.push_back(arg);
		}
		err = clSetKernelArg(kernels_[index].kernel, i, sizeof(cl_mem), &(arg.buffer));
		if (err != CL_SUCCESS)
			return Argument();
		return arg;
	}

	Argument KernelProgram::setKernelArgTex3D(std::string &name, int i, cl_mem_flags flag, GLuint texture)
	{
		int index = findKernel(name);
		return setKernelArgTex3D(index, i, flag, texture);
	}

	//copy existing texure to buffer
	Argument KernelProgram::setKernelArgTex3DBuf(
		int index, int i, cl_mem_flags flag, GLuint texture,
		size_t size, size_t *region)
	{
		Argument arg, argt;
		cl_int err;
		cl_mem buft = 0, bufb = 0;
		if (index < 0 || index >= kernels_.size())
			return arg;

		arg.kernel_index = index;
		arg.index = i;
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
		}
		else
		{
			buft = clCreateFromGLTexture(context_, CL_MEM_READ_ONLY, GL_TEXTURE_3D, 0, texture, &err);
			if (err != CL_SUCCESS)
				return Argument();
			argt.buffer = buft;
			arg_list_.push_back(argt);
		}
		//find data buffer
		if (matchArgBuf(arg, ai))
		{
			bufb = arg.buffer;
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
		clFlush(queue_);
		clFinish(queue_);
		//clReleaseMemObject(buft);
		err = clSetKernelArg(kernels_[index].kernel, i, sizeof(cl_mem), &(arg.buffer));
		if (err != CL_SUCCESS)
			return Argument();
		return arg;
	}

	Argument KernelProgram::setKernelArgTex3DBuf(
		std::string &name, int i, cl_mem_flags flag, GLuint texture,
		size_t size, size_t *region)
	{
		int index = findKernel(name);
		return setKernelArgTex3DBuf(index, i, flag, texture, size, region);
	}

	Argument KernelProgram::setKernelArgImage(int index, int i, cl_mem_flags flag, cl_image_format format, cl_image_desc desc, void* data)
	{
		Argument arg;
		cl_int err;
		cl_mem buffer = 0;
		if (index < 0 || index >= kernels_.size())
			return arg;

		arg.kernel_index = index;
		arg.index = i;
		arg.size = 0;
		arg.texture = 0;
		arg.orgn_addr = data;

		unsigned int ai;
		if (matchArgAddr(arg, ai))
		{
			arg.buffer = arg_list_[ai].buffer;
			buffer = arg.buffer;
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
		err = clSetKernelArg(kernels_[index].kernel, i, sizeof(cl_mem), &(arg.buffer));
		if (err != CL_SUCCESS)
			return Argument();
		return arg;
	}

	Argument KernelProgram::setKernelArgImage(std::string &name, int i, cl_mem_flags flag, cl_image_format format, cl_image_desc desc, void* data)
	{
		int index = findKernel(name);
		return setKernelArgImage(index, i, flag, format, desc, data);
	}

	void KernelProgram::setKernelArgLocal(int index, int i, size_t size)
	{
		cl_int err = clSetKernelArg(kernels_[index].kernel, i, size, NULL);
	}

	void KernelProgram::readBuffer(size_t size,
		void* buf_data, void* data)
	{
		cl_int err;

		if (buf_data)
		{
			Argument arg;
			arg.kernel_index = 0;
			arg.index = 0;
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
			arg.kernel_index = 0;
			arg.index = 0;
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
			arg.kernel_index = 0;
			arg.index = 0;
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
		for (auto it = arg_list_.begin();
			it != arg_list_.end();)
		{
			if (del_mem)
				clReleaseMemObject(it->buffer);
			it = arg_list_.erase(it);
		}
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
		arg.kernel_index = kernel_index;
		arg.index = index;
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
		arg.kernel_index = 0;
		arg.index = 0;
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
}
