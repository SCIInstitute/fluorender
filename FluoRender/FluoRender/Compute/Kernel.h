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

#ifndef _KERNEL_H_
#define _KERNEL_H_

#include <string>
#include <vector>

namespace flvr
{
#define KERNEL_STRING	0
#define KERNEL_HIST_3D	1
#define KERNEL_TEST		2

	class KernelProgram;

	class Kernel
	{
	public:
		Kernel(int type);
		~Kernel();

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

	class KernelFactory
	{
	public:
		KernelFactory();
		~KernelFactory();
		void clear();

		KernelProgram* kernel(int type);
		KernelProgram* kernel(std::string s, int bits, float max_int);

	protected:
		std::vector<Kernel*> kernels_;
		int prev_kernel_;

	private:
		std::string replace(const std::string& s,
			const std::string& src, const std::string& rep)
		{
			std::string r = s;
			size_t len = src.length();
			size_t pos = r.find(src, 0);
			while (pos != std::string::npos)
			{
				r.replace(pos, len, rep);
				pos = r.find(src, pos+1);
			}
			return r;
		}
	};
}

#endif