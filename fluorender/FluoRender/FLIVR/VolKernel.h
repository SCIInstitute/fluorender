#ifndef VolKernel_h
#define VolKernel_h

#include <string>
#include <vector>

namespace FLIVR
{
	class KernelProgram;

	class VolKernel
	{
	public:
		VolKernel();
		~VolKernel();

		bool create();

		inline bool match()
		{ return true; }

		inline KernelProgram* program()
		{ return program_; }

	protected:
		bool emit(std::string& s);

		KernelProgram* program_;
	};

	class VolKernelFactory
	{
	public:
		VolKernelFactory();
		~VolKernelFactory();

		KernelProgram* kernel();

	protected:
		std::vector<VolKernel*> kernels_;
		int prev_kernel_;
	};
}

#endif