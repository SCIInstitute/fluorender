/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2022 Scientific Computing and Imaging Institute,
University of Utah.


Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/
#ifndef _KERNELEXECUTOR_H_
#define _KERNELEXECUTOR_H_

#include <vector>
#include <string>

namespace fluo
{
	class VolumeData;
}
namespace flvr
{
	class KernelProgram;
}
namespace flrd
{
	class KernelExecutor
	{
	public:
		KernelExecutor();
		~KernelExecutor();

		void SetCode(const std::string &code);
		void LoadCode(const std::string &filename);
		void SetVolume(fluo::VolumeData *vd);
		void SetDuplicate(bool dup);
		fluo::VolumeData* GetVolume();
		fluo::VolumeData* GetResult(bool pop);
		bool GetMessage(std::string &msg);

		bool Execute();

	private:
		fluo::VolumeData *m_vd;
		std::vector<fluo::VolumeData*> m_vd_r;//result
		bool m_duplicate;//whether duplicate the input volume

		std::string m_code;
		std::string m_message;

		bool ExecuteKernel(flvr::KernelProgram* kernel,
			unsigned int data_id, void* result,
			size_t brick_x, size_t brick_y,
			size_t brick_z, int chars);

	};
}

#endif//_KERNELEXECUTOR_H_
