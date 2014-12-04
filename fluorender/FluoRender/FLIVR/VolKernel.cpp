#include <string>
#include <sstream>
#include <iostream>
#include <FLIVR/VolKernel.h>
#include <FLIVR/KernelProgram.h>

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

#define KERNEL_HIST_3D_CODE \
	"//KERNEL_HIST_3D_CODE\n" \
	"const sampler_t samp =\n" \
	"	CLK_NORMALIZED_COORDS_FALSE|\n" \
	"	CLK_ADDRESS_REPEAT|\n" \
	"	CLK_FILTER_NEAREST;\n" \
	"__kernel void hist_3d(image3d_t data,\n" \
	"	image3d_t mask, __global float* hist,\n" \
	"	__const int hist_size)\n" \
	"{\n" \
	"	int4 coord = (int4)(get_global_id(0),\n" \
	"		get_global_id(1), get_global_id(2), 1);\n" \
	"	float4 mask_value = read_imagef(mask, samp, coord);\n" \
	"	if (mask_value.x > 0.0)\n" \
	"	{\n" \
	"		float4 data_value = read_imagef(data, samp, coord);\n" \
	"		int index = (int)(data_value.x*(hist_size-1));\n" \
	"		hist[index] += 1.0;\n" \
	"	}\n" \
	"}\n" \
	"\n"

	VolKernel::VolKernel(int type) :
		type_(type),
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

	bool VolKernel::create(std::string &s)
	{
		program_ = new KernelProgram(s);
		return true;
	}

	inline bool VolKernel::match(std::string &s)
	{
		return (type_ == KERNEL_STRING &&
			s == program_->source_);
	}

	bool VolKernel::emit(string& s)
	{
		ostringstream z;

		switch (type_)
		{
		case KERNEL_TEST:
			z << KERNEL_TEST_CODE;
			break;
		case KERNEL_HIST_3D:
			z << KERNEL_HIST_3D_CODE;
			break;
		}

		s = z.str();
		return true;
	}

	VolKernelFactory::VolKernelFactory()
		: prev_kernel_(-1)
	{
	}

	VolKernelFactory::~VolKernelFactory()
	{
		clean();
	}

	void VolKernelFactory::clean()
	{
		for (unsigned int i=0; i<kernels_.size(); ++i)
		{
			delete kernels_[i];
		}
		kernels_.clear();
		prev_kernel_ = -1;
	}

	KernelProgram* VolKernelFactory::kernel(int type)
	{
		if (prev_kernel_ >= 0)
		{
			if (kernels_[prev_kernel_]->match(type))
			{
				return kernels_[prev_kernel_]->program();
			}
		}

		for (unsigned int i=0; i<kernels_.size(); ++i)
		{
			if (kernels_[i]->match(type))
			{
				prev_kernel_ = i;
				return kernels_[i]->program();
			}
		}

		VolKernel* k = new VolKernel(type);
		if (!k->create())
		{
			delete k;
			return 0;
		}
		kernels_.push_back(k);
		prev_kernel_ = int(kernels_.size())-1;
		return k->program();
	}

	KernelProgram* VolKernelFactory::kernel(std::string s)
	{
		if (prev_kernel_ >= 0)
		{
			if (kernels_[prev_kernel_]->match(s))
			{
				return kernels_[prev_kernel_]->program();
			}
		}

		for (unsigned int i=0; i<kernels_.size(); ++i)
		{
			if (kernels_[i]->match(s))
			{
				prev_kernel_ = i;
				return kernels_[i]->program();
			}
		}

		VolKernel* k = new VolKernel(KERNEL_STRING);
		if (!k->create(s))
		{
			delete k;
			return 0;
		}
		kernels_.push_back(k);
		prev_kernel_ = int(kernels_.size())-1;
		return k->program();
	}
}