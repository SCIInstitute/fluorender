#ifndef VolKernel_h
#define VolKernel_h

#include <string>
#include <vector>


namespace FLIVR
{
#define KERNEL_TEST	0
#define KERNEL_HIST_3D	1

	class KernelProgram;

	class VolKernel
	{
	public:
		VolKernel(int type);
		~VolKernel();

		bool create();

		inline int type() {return type_;}

		inline bool match(int type)
		{ return (type == type_); }

		inline KernelProgram* program()
		{ return program_; }

	protected:
		bool emit(std::string& s);

		int type_;

		KernelProgram* program_;
	};

	class VolKernelFactory
	{
	public:
		VolKernelFactory();
		~VolKernelFactory();

		KernelProgram* kernel(int type = 0);

	protected:
		std::vector<VolKernel*> kernels_;
		int prev_kernel_;
	};
}

#endif