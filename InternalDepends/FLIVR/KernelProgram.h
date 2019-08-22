#ifndef KernelProgram_h
#define KernelProgram_h

#include <GL/glew.h>
#if defined(_WIN32) || defined(__linux__)
#include <CL/cl.h>
#include <CL/cl_gl.h>
#include <CL/cl_gl_ext.h>
#include <GL/glx.h>
#endif
#ifdef _DARWIN
#include <OpenCL/cl.h>
#include <OpenCL/cl_gl.h>
#include <OpenCL/cl_gl_ext.h>
#include <OpenGL/CGLCurrent.h>
#endif
#include <string>
#include <vector>

namespace FLIVR
{
	class VolKernel;
	//argument
	typedef struct
	{
		int kernel_index;
		cl_uint index;
		size_t size;
		GLuint texture;
		cl_mem buffer;
		void* orgn_addr;
	} Argument;
	class KernelProgram
	{
	public:
		KernelProgram(const std::string& source);
		~KernelProgram();

		bool valid();
		void destroy();

		//create a kernel in the program
		//return kernel index; -1 unsuccessful
		int createKernel(std::string &name);
		int findKernel(std::string &name);
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

		bool matchArg(cl_mem, unsigned int&);//find buffer
		bool matchArg(Argument*, unsigned int&);
		bool matchArgTex(Argument*, unsigned int&);//use texture id to match
		bool matchArgAddr(Argument*, unsigned int&);//use data address to match
		//set argument
		int setKernelArgument(Argument*);
		Argument getKernelArgumnet(unsigned int ai);
		void setKernelArgConst(int, int, size_t, void*);
		void setKernelArgConst(std::string &name, int, size_t, void*);
		cl_mem setKernelArgBuf(int, int, cl_mem_flags, size_t, void*);
		cl_mem setKernelArgBuf(std::string &name, int, cl_mem_flags, size_t, void*);
		cl_mem setKernelArgBufWrite(int, int, cl_mem_flags, size_t, void*);
		cl_mem setKernelArgBufWrite(std::string &name, int, cl_mem_flags, size_t, void*);
		cl_mem setKernelArgTex2D(int, int, cl_mem_flags, GLuint);
		cl_mem setKernelArgTex2D(std::string &name, int, cl_mem_flags, GLuint);
		cl_mem setKernelArgTex3D(int, int, cl_mem_flags, GLuint);
		cl_mem setKernelArgTex3D(std::string &name, int, cl_mem_flags, GLuint);
		cl_mem setKernelArgImage(int, int, cl_mem_flags, cl_image_format, cl_image_desc, void*);
		cl_mem setKernelArgImage(std::string &name, int, cl_mem_flags, cl_image_format, cl_image_desc, void*);

		//read/write
		void readBuffer(size_t size,
			void* buf_data, void* data);
		void readBuffer(cl_mem buffer, void* data);
		void writeBuffer(size_t size,
			void* buf_data, void* data);
		void writeBuffer(cl_mem buffer, void* data);
		void writeImage(const size_t* origin, const size_t* region,
			void* img_data, void* data);
		void writeImage(const size_t* origin, const size_t* region,
			cl_mem image, void* data);

		//release mem obj
		void releaseMemObject(cl_mem);
		void releaseMemObject(int, int, size_t, GLuint);
		void releaseMemObject(size_t, void* orgn_addr);

		//initialization
		static void init_kernels_supported();
		static bool init();
		static void clear();
		static void set_device_id(int id);
		static int get_device_id();
		static std::string& get_device_name();
		static void release();

		//info
		std::string &getInfo();

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

		//memory object to release
		std::vector<Argument> arg_list_;

		static bool init_;
		static cl_device_id device_;
		static cl_context context_;
		static int device_id_;
		static std::string device_name_;
	};
}

#endif
