#ifndef KernelProgram_h
#define KernelProgram_h

#include <GL/glew.h>
#include <CL/cl.h>
#include <CL/cl_gl.h>
#include <string>
#include <vector>

namespace FLIVR
{
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
			cl_mem buffer;
		} Argument;
		bool matchArg(Argument*, unsigned int&);
		void setKernelArgConst(int, size_t, void*);
		void setKernelArgBuf(int, cl_mem_flags, size_t, void*);
		void setKernelArgTex2D(int, cl_mem_flags, GLuint);
		void setKernelArgTex3D(int, cl_mem_flags, GLuint);
		void readBuffer(int, void*);

		//initialization
		static void init_kernels_supported();
		static bool init();
		static void clear();

	protected:
		std::string source_;
		cl_program program_;
		cl_kernel kernel_;
		cl_command_queue queue_;

		//memory object to release
		std::vector<Argument> arg_list_;

		static bool init_;
		static cl_device_id device_;
		static cl_context context_;
	};
}

#endif