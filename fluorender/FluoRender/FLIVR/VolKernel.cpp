#include <string>
#include <sstream>
#include <iostream>
#include <FLIVR\VolKernel.h>
#include <FLIVR\KernelProgram.h>

using std::string;
using std::vector;
using std::ostringstream;

namespace FLIVR
{
#define KERNEL_TEST_CODE \
	"__kernel void add_numbers(__global float4* data, \n" \
	"	__local float* local_result, __global float* group_result) {\n" \
	"	float sum;\n" \
	"	float4 input1, input2, sum_vector;\n" \
	"	uint global_addr, local_addr;\n" \
	"	global_addr = get_global_id(0) * 2;\n" \
	"	input1 = data[global_addr];\n" \
	"	input2 = data[global_addr+1];\n" \
	"	sum_vector = input1 + input2;\n" \
	"	local_addr = get_local_id(0);\n" \
	"	local_result[local_addr] = sum_vector.s0 + sum_vector.s1 + \n" \
	"		sum_vector.s2 + sum_vector.s3;\n" \
	"	barrier(CLK_LOCAL_MEM_FENCE);\n" \
	"	if(get_local_id(0) == 0) {\n" \
	"		sum = 0.0f;\n" \
	"		for(int i=0; i<get_local_size(0); i++) {\n" \
	"		sum += local_result[i];\n" \
	"		}\n" \
	"		group_result[get_group_id(0)] = sum;\n" \
	"	}\n" \
	"}\n"


	VolKernel::VolKernel() :
		program_(0)
	{
	}

	VolKernel::~VolKernel()
	{
		delete program_;
	}

	bool VolKernel::create()
	{
		string s;
		if (!emit(s)) return false;
		program_ = new KernelProgram(s);
		return true;
	}

	bool VolKernel::emit(string& s)
	{
		ostringstream z;

		z << KERNEL_TEST_CODE;

		s = z.str();
		return true;
	}

	VolKernelFactory::VolKernelFactory()
		: prev_kernel_(-1)
	{
	}

	VolKernelFactory::~VolKernelFactory()
	{
		for (unsigned int i=0; i<kernels_.size(); ++i)
		{
			delete kernels_[i];
		}
	}

	KernelProgram* VolKernelFactory::kernel()
	{
		if (prev_kernel_ >= 0)
		{
			if (kernels_[prev_kernel_]->match())
			{
				return kernels_[prev_kernel_]->program();
			}
		}

		for (unsigned int i=0; i<kernels_.size(); ++i)
		{
			if (kernels_[i]->match())
			{
				prev_kernel_ = i;
				return kernels_[i]->program();
			}
		}

		VolKernel* k = new VolKernel();
		if (!k->create())
		{
			delete k;
			return 0;
		}
		kernels_.push_back(k);
		prev_kernel_ = int(kernels_.size())-1;
		return k->program();
	}
}