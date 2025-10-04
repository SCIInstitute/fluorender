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
#include <KernelFactory.h>
#include <KernelProgram.h>
#include <TestKernelCode.h>

using std::string;
using std::vector;
using std::ostringstream;

namespace flvr
{
	KernelFactory::KernelFactory()
	{
	}

	KernelFactory::~KernelFactory()
	{
	}

	void KernelFactory::clear()
	{
		programs_.clear();
	}

	void KernelFactory::clear(KernelProgram* prog)
	{
		if (!KernelProgram::need_clear_)
			return;

		auto it = std::remove_if(programs_.begin(), programs_.end(),
			[prog](const std::shared_ptr<KernelProgram>& p) {
				return p.get() == prog;
			});

		if (it != programs_.end()) {
			programs_.erase(it, programs_.end());
		}
	}

	KernelProgram* KernelFactory::program(std::string s, int bits, float max_int)
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
		{
			return nullptr;
		}

		for (size_t i = 0; i < programs_.size(); ++i)
		{
			if (programs_[i]->matches(s))
			{
				return programs_[i].get();
			}
		}

		auto prog = std::make_shared<KernelProgram>(s);
		programs_.push_back(prog);
		return prog.get();
	}
}