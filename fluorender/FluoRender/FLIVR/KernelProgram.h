#ifndef KernelProgram_h
#define KernelProgram_h

#include <GL/glew.h>
#ifdef _WIN32
#include <CL/cl.h>
#include <CL/cl_gl.h>
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
	class KernelProgram
	{
	public:
		KernelProgram(const std::string& source);
		~KernelProgram();

		bool create(std::string &name);
		bool valid();
		void destroy();

		void execute(cl_uint, size_t*, size_t*);

		typedef struct
		{
			cl_uint index;
			size_t size;
			GLuint texture;
			cl_mem buffer;
		} Argument;
		bool matchArg(Argument*, unsigned int&);
		void setKernelArgConst(int, size_t, void*);
		void setKernelArgBuf(int, cl_mem_flags, size_t, void*);
		void setKernelArgBufWrite(int, cl_mem_flags, size_t, void*);
		void setKernelArgTex2D(int, cl_mem_flags, GLuint);
		void setKernelArgTex3D(int, cl_mem_flags, GLuint);
		void readBuffer(int, void*);
		void writeBuffer(int, void*, size_t, size_t, size_t);

		//initialization
		static void init_kernels_supported();
		static bool init();
		static void clear();
		static void set_device_id(int id);
		static int get_device_id();
		static std::string& get_device_name();

		//info
		std::string &getInfo();

		friend class VolKernel;
#ifdef _DARWIN
        static CGLContextObj gl_context_;
#endif
	protected:
		std::string source_;
		cl_program program_;
		cl_kernel kernel_;
		cl_command_queue queue_;

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