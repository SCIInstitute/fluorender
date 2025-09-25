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

#ifndef __glew_h__
typedef unsigned int GLuint;
#endif

namespace flvr
{
	class VolKernel;
	//argument
	struct Argument
	{
		bool protect_;
		//int kernel_index;
		//cl_uint index;
		std::set<cl_uint> kidx;//kernels that use it
		size_t size;
		GLuint texture;
		GLuint vbo;
		cl_mem buffer;
		void* orgn_addr;

		Argument() :
			protect_(false),
			size(0),
			texture(0),
			vbo(0),
			buffer(0),
			orgn_addr(0) {}

		void protect() { protect_ = true; }
		void unprotect() { protect_ = false; }
		void kernel(cl_uint idx) { kidx.insert(idx); }
		bool find_kernel(cl_uint idx) { return kidx.find(idx) != kidx.end(); }
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
		//add a kernel from another program
		//for sharing buffers...
		int addKernel(KernelProgram*, int);
		void removeExternalKernels();
		//execute kernel
		bool executeKernel(int, cl_uint, size_t*, size_t*);
		bool executeKernel(std::string &name,
			cl_uint, size_t*, size_t*);

		//get info
		bool getWorkGroupSize(int idex, size_t*);

		bool matchArg(Argument&, unsigned int&);
		bool matchArgBuf(Argument&, unsigned int&);//find buffer
		bool matchArgTex(Argument&, unsigned int&);//use texture id to match
		bool matchArgVBO(Argument&, unsigned int&);//use vbo id to match
		bool matchArgAddr(Argument&, unsigned int&);//use data address to match
		//set argument
		void setKernelArgBegin(int kernel_idx, int arg_idx = 0)
		{
			kernel_idx_ = kernel_idx; arg_idx_ = arg_idx;
		}
		int setKernelArgument(Argument&);
		void setKernelArgConst(size_t, void*);
		Argument setKernelArgBuf(cl_mem_flags, size_t, void*);
		Argument setKernelArgBufWrite(cl_mem_flags, size_t, void*);
		Argument setKernelArgTex2D(cl_mem_flags, GLuint);
		Argument setKernelArgTex3D(cl_mem_flags, GLuint);
		Argument setKernelArgTex3DBuf(cl_mem_flags, GLuint, size_t, size_t*);//copy existing texure to buffer
		Argument setKernelArgVertexBuf(cl_mem_flags, GLuint, size_t);//assign existing vertex buffer to buffer
		Argument setKernelArgImage(cl_mem_flags, cl_image_format, cl_image_desc, void*);
		void setKernelArgLocal(size_t);

		//read/write
		void readBuffer(size_t size,
			void* buf_data, void* data);
		void readBuffer(Argument& arg, void* data);
		void copyBufTex3D(Argument& arg, GLuint, size_t, size_t*);//copy buffer back to texture
		void writeBuffer(size_t size,
			void* buf_data, void* data);
		void writeBuffer(Argument& arg, void* data);
		void writeImage(const size_t* origin, const size_t* region,
			void* img_data, void* data);
		void writeImage(const size_t* origin, const size_t* region,
			Argument& arg, void* data);

		//release mem obj
		void releaseAll(bool del_mem = true);
		void releaseMemObject(Argument&);
		void releaseMemObject(int, int, size_t, GLuint, GLuint);
		void releaseMemObject(size_t, void* orgn_addr);

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

		friend class VolKernel;
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

		//memory object to release
		std::vector<Argument> arg_list_;

		static bool init_;
		static cl_device_id device_;
		static cl_context context_;
		static int platform_id_;
		static int device_id_;
		static std::string device_name_;
		static std::vector<CLPlatform> device_list_;
	};
}

#endif
