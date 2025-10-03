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

#ifndef KernelProgram_h
#define KernelProgram_h

#if defined(__linux__)
#include <GL/glx.h>
#endif
#if defined(_WIN32) || defined(__linux__)
#include <CL/cl.h>
#include <CL/cl_gl.h>
#endif
#ifdef _DARWIN
#include <OpenCL/cl.h>
#include <OpenCL/cl_gl.h>
#include <OpenCL/opencl.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/CGLCurrent.h>
#endif
#include <string>
#include <vector>
#include <set>
#include <memory>
#include <unordered_map>

#ifndef __glew_h__
typedef unsigned int GLuint;
#endif

namespace flvr
{
	class Kernel;
	//argument
	class Argument
	{
	public:
		bool protect_;
		bool valid_;//buffer valid
		GLuint texture;
		GLuint vbo;
		void* orgn_addr;
		cl_mem buffer;
		size_t size;

		void protect() { protect_ = true; }
		void unprotect() { protect_ = false; }
		void destroy()
		{
			if (valid_)
			{
				cl_int err = clReleaseMemObject(buffer);
				protect_ = false;
				size = 0;
				texture = 0;
				vbo = 0;
				buffer = 0;
				orgn_addr = 0;
			}
		}
		void release()
		{
			if (valid_ && !protect_)
				destroy();
		}

		bool matchesPointer(void* data) const
		{
			return orgn_addr == data;
		}

		bool matchesTexture(GLuint tex_id) const
		{
			return texture == tex_id;
		}

		bool matchesVBO(GLuint vbo_id) const
		{
			return vbo == vbo_id;
		}

		static std::shared_ptr<Argument> createFromPointer(cl_context context, cl_mem_flags flags, size_t size, void* data)
		{
			cl_int err = CL_SUCCESS;
			cl_mem buf = clCreateBuffer(context, flags, size, data, &err);
			if (err != CL_SUCCESS || !buf) {
				return nullptr;
			}

			auto arg = std::make_shared<Argument>();
			arg->buffer = buf;
			arg->size = size;
			arg->orgn_addr = data;
			arg->valid_ = true;
			return arg;
		}

		static std::shared_ptr<Argument> createFromTexture2D(cl_context context, cl_mem_flags flags, GLuint tex_id);
		static std::shared_ptr<Argument> createFromTexture3D(cl_context context, cl_mem_flags flags, GLuint tex_id);

		static std::shared_ptr<Argument> createFromVBO(cl_context context, cl_mem_flags flags, GLuint vbo_id, size_t size)
		{
			cl_int err = CL_SUCCESS;
			cl_mem buf = clCreateFromGLBuffer(context, flags, vbo_id, &err);
			if (err != CL_SUCCESS || !buf) {
				return nullptr;
			}

			auto arg = std::make_shared<Argument>();
			arg->buffer = buf;
			arg->vbo = vbo_id;
			arg->size = size;
			arg->valid_ = true;
			return arg;
		}

		Argument() :
			valid_(false),
			protect_(false),
			size(0),
			texture(0),
			vbo(0),
			buffer(0),
			orgn_addr(0) {}
		~Argument()
		{
			destroy();
		}

	};

	typedef struct
	{
		unsigned int ngx;
		unsigned int ngy;
		unsigned int ngz;
		unsigned int gsx;
		unsigned int gsy;
		unsigned int gsz;
		unsigned int gsxyz;
		unsigned int gsxy;
	} GroupSize;

	struct CLDevice
	{
		cl_device_id id;
		std::string vendor;
		std::string name;
		std::string version;
	};
	struct CLPlatform
	{
		cl_platform_id id;
		std::string vendor;
		std::string name;
		std::vector<CLDevice> devices;
	};

	class KernelProgram
	{
	public:
		KernelProgram(const std::string& source);
		~KernelProgram();

		bool valid();
		void destroy();

		//create a kernel in the program
		//return kernel index; -1 unsuccessful
		int createKernel(const std::string &name);
		int findKernel(const std::string &name);
		//execute kernel
		bool executeKernel(int, cl_uint, size_t*, size_t*);
		bool executeKernel(std::string &name,
			cl_uint, size_t*, size_t*);

		//get info
		bool getWorkGroupSize(int idex, size_t*);

		//set argument
		void setKernelArgBegin(int kernel_idx, int arg_idx = 0)
		{
			kernel_idx_ = kernel_idx; arg_idx_ = arg_idx;
		}
		bool setKernelArgConst(size_t, void*);
		bool setKernelArgLocal(size_t);
		bool setKernelArgument(std::weak_ptr<Argument> arg);
		std::weak_ptr<Argument> setKernelArgBuf(cl_mem_flags, size_t, void*);
		bool updateKernelArgBuf(std::weak_ptr<Argument> arg, cl_mem_flags flags, size_t new_size, void* data);
		std::weak_ptr<Argument> setKernelArgTex2D(cl_mem_flags, GLuint);
		std::weak_ptr<Argument> setKernelArgTex3D(cl_mem_flags, GLuint);
		std::weak_ptr<Argument> copyTex3DToArgBuf(cl_mem_flags, GLuint, size_t, size_t*);//copy existing texure to buffer
		bool copyArgBufToTex3D(const std::weak_ptr<Argument> arg, GLuint, size_t, size_t*);//copy buffer back to texture
		std::weak_ptr<Argument> setKernelArgVertexBuf(cl_mem_flags, GLuint, size_t);//assign existing vertex buffer to buffer

		//read back
		bool readBuffer(const std::weak_ptr<Argument> arg, void* data);

		//release mem obj
		void releaseAllArgs();
		void releaseKernelArgs(int kernel_idx);
		void releaseArg(const std::weak_ptr<Argument>&);

		//initialization
		static void init_kernels_supported();
		static bool init();
		static void clear();
		static void set_platform_id(int id);
		static int get_platform_id();
		static void set_device_id(int id);
		static int get_device_id();
		static std::string& get_device_name();
		static void release();
		static std::vector<CLPlatform>* GetDeviceList();
		//context
		static cl_device_id get_device() { return device_; }
		static cl_context get_context() { return context_; }

		//features
		static bool get_float_atomics() { return float_atomics_; }

		//finish
		void finish();

		//info
		std::string &getInfo();

		//group division
		bool get_group_size(int index,
			unsigned int nx, unsigned int ny, unsigned int nz,
			GroupSize &ksize);
		bool get_group_size2(int index,
			unsigned int nx, unsigned int ny, unsigned int nz,
			GroupSize &ksize);
		unsigned int optimize_group_size_xy(unsigned int nt, unsigned int target);
		unsigned int optimize_group_size_z(unsigned int nt, unsigned int target);

		friend class Kernel;
#ifdef _DARWIN
		static CGLContextObj gl_context_;
#endif
	protected:
		std::string source_;
		cl_program program_;
		cl_command_queue queue_;

		//there can be multiple kernels in one program
		typedef struct
		{
			cl_kernel kernel;
			std::string name;
			bool external;
		} Kernel;
		std::vector<Kernel> kernels_;

		std::string info_;

		int kernel_idx_;
		int arg_idx_;

		//a list of arguments to keep track of cl mem objs
		std::set<std::shared_ptr<Argument>> arg_list_;
		//references which arguments are used by each kernel, without owning them
		std::unordered_map<int, std::vector<std::weak_ptr<Argument>>> arg_map_;//maps kernel index to a list of arguments

		//global settings
		static bool init_;
		static cl_device_id device_;
		static cl_context context_;
		static int platform_id_;
		static int device_id_;
		static std::string device_name_;
		static std::vector<CLPlatform> device_list_;

		//features
		static bool float_atomics_;
	};
}

#endif
