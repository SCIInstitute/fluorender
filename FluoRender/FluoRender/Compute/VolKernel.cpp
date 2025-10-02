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

#include <string>
#include <sstream>
#include <iostream>
#include <VolKernel.h>
#include <KernelProgram.h>
#include <VolKernelCode.h>

using std::string;
using std::vector;
using std::ostringstream;

namespace flvr
{
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
		for (unsigned int i = 0; i<kernels_.size(); ++i)
			delete kernels_[i];
	}

	void VolKernelFactory::clear()
	{
		for (unsigned int i=0; i<kernels_.size(); ++i)
			delete kernels_[i];
		kernels_.clear();
		prev_kernel_ = -1;
	}

	KernelProgram* VolKernelFactory::kernel(int type)
	{
		if (prev_kernel_ >= 0)
		{
			if (kernels_[prev_kernel_]->match(type))
				return kernels_[prev_kernel_]->program();
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

	KernelProgram* VolKernelFactory::kernel(std::string s, int bits, float max_int)
	{
		//change string according to bits
		if (bits > 8 && bits <= 16)
		{
			s = replace(s, "#define DWL ",
				"#define DWL unsigned short//");
			s = replace(s, "#define VSCL ",
				"#define VSCL 65535//");
			s = replace(s, "#define MAX_INT ",
				"#define MAX_INT " + std::to_string(max_int) + "f//");
		}
		else if (bits > 16 && bits <= 32)
		{ }

		if (prev_kernel_ >= 0)
		{
			if (kernels_[prev_kernel_]->match(s))
				return kernels_[prev_kernel_]->program();
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