#ifndef VolKernel_h
#define VolKernel_h

#include <string>
#include <vector>


namespace FLIVR
{
#define KERNEL_STRING	0
#define KERNEL_HIST_3D	1
#define KERNEL_TEST		2

	class KernelProgram;

	class VolKernel
	{
	public:
		VolKernel(int type);
		~VolKernel();

		bool create();
		bool create(std::string &s);

		inline int type() {return type_;}

		inline bool match(int type)
		{ return (type == type_); }

		inline bool match(std::string &s);

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

		void clean();
		void remove(KernelProgram* kernel);

		KernelProgram* kernel(int type = 0);
		KernelProgram* kernel(std::string s);

	protected:
		std::vector<VolKernel*> kernels_;
		int prev_kernel_;
	};
}

#endif