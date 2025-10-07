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
#include <unordered_set>
#include <memory>
#include <unordered_map>

#ifndef __glew_h__
typedef unsigned int GLuint;
#endif

namespace flvr
{
	class KernelFactory;
	//argument
	class Argument
	{
		enum ArgType
		{
			ArgType_Unknown = 0,
			ArgType_Pointer,
			ArgType_Tex,
			ArgType_VBO
		};

	public:
		bool protect_;
		bool valid_;//buffer valid
		ArgType type_;
		std::string name_;//id
		GLuint tex_;
		GLuint vbo_;
		void* pointer_;
		cl_mem buffer_;
		size_t size_;

		void protect() { protect_ = true; }
		void unprotect() { protect_ = false; }
		void destroy()
		{
			if (valid_)
			{
				cl_int err = clReleaseMemObject(buffer_);
				type_ = ArgType_Unknown;
				name_ = "";
				protect_ = false;
				size_ = 0;
				tex_ = 0;
				vbo_ = 0;
				buffer_ = 0;
				pointer_ = 0;
			}
		}
		void release()
		{
			if (valid_ && !protect_)
				destroy();
		}

		bool matchesPointer(const std::string& name, size_t size, void* data) const
		{
			if (type_ != ArgType_Pointer)
				return false;
			if (!name_.empty())
				return name_ == name;
			return pointer_ == data &&
				size_ == size;
		}

		bool matchesTexture(GLuint tex_id) const
		{
			return type_ == ArgType_Tex &&
				tex_ == tex_id;
		}

		bool matchesVBO(GLuint vbo_id) const
		{
			return type_ == ArgType_VBO &&
				vbo_ == vbo_id;
		}

		static std::shared_ptr<Argument> createFromPointer(cl_context context, cl_mem_flags flags, const std::string& name, size_t size, void* data)
		{
			cl_int err = CL_SUCCESS;
			cl_mem buf = clCreateBuffer(context, flags, size, data, &err);
			if (err != CL_SUCCESS || !buf) {
				return nullptr;
			}

			auto arg = std::make_shared<Argument>();
			arg->type_ = ArgType_Pointer;
			arg->name_ = name;
			arg->buffer_ = buf;
			arg->size_ = size;
			arg->pointer_ = data;
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
			arg->type_ = ArgType_VBO;
			arg->buffer_ = buf;
			arg->vbo_ = vbo_id;
			arg->size_ = size;
			arg->valid_ = true;
			return arg;
		}

		Argument() :
			valid_(false),
			type_(ArgType_Unknown),
			protect_(false),
			size_(0),
			tex_(0),
			vbo_(0),
			buffer_(0),
			pointer_(0) {}
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

		bool matches(const std::string& s);

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
		void beginArgs(int kernel_idx, int arg_idx = 0)//Sets up kernel + arg index — short and scoped.
		{
			kernel_idx_ = kernel_idx; arg_idx_ = arg_idx;
		}
		bool setConst(size_t, void*);
		bool setLocal(size_t);
		bool bindArg(std::weak_ptr<Argument> arg);//Binds existing Argument object.
		std::weak_ptr<Argument> setBufNew(cl_mem_flags, const std::string&, size_t, void*);//Sets from host pointer, creates buffer only if needed.
		std::weak_ptr<Argument> setBufNewOrUpdate(cl_mem_flags, const std::string&, size_t, void*);//Always updates from host pointer.
		bool updateBuf(std::weak_ptr<Argument> arg, cl_mem_flags flags, size_t new_size, void* data);//Updates existing buffer from host pointer.
		std::weak_ptr<Argument> setTex2D(cl_mem_flags, GLuint);//OpenGL texture binding.
		std::weak_ptr<Argument> setTex3D(cl_mem_flags, GLuint);//OpenGL texture binding.
		std::weak_ptr<Argument> copyTex3DToBuf(cl_mem_flags, GLuint, const std::string&, size_t, size_t*);//copy existing texure to buffer
		bool copyBufToTex3D(const std::weak_ptr<Argument> arg, GLuint, size_t, size_t*);//copy buffer back to texture
		std::weak_ptr<Argument> bindVeretxBuf(cl_mem_flags, GLuint, size_t);//assign existing vertex buffer to buffer

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
		static void release_context();
		static std::vector<CLPlatform>* GetDeviceList();
		//context
		static cl_device_id get_device() { return device_; }
		static cl_context get_context() { return context_; }

		//features
		static bool get_float_atomics() { return float_atomics_; }
		static bool get_need_clear(CLDevice* device);

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

		friend class KernelFactory;
#ifdef _DARWIN
		static CGLContextObj gl_context_;
#endif
	protected:
		std::string source_;
		cl_program program_;

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
		struct WeakPtrHash {
			size_t operator()(const std::weak_ptr<Argument>& wp) const {
				auto sp = wp.lock();
				return std::hash<Argument*>()(sp.get()); // Hash by raw pointer
			}
		};

		struct WeakPtrEqual {
			bool operator()(const std::weak_ptr<Argument>& a, const std::weak_ptr<Argument>& b) const {
				return !a.owner_before(b) && !b.owner_before(a); // Equivalent if neither is before the other
			}
		};
		std::unordered_map<int, std::unordered_set<std::weak_ptr<Argument>, WeakPtrHash, WeakPtrEqual>> arg_map_;

		//global settings
		static bool init_;
		static cl_device_id device_;
		static cl_context context_;
		static cl_command_queue queue_;
		static int platform_id_;
		static int device_id_;
		static std::string device_name_;
		static std::vector<CLPlatform> device_list_;

		//features
		static bool float_atomics_;
		static bool need_clear_;
	};
}

#endif
