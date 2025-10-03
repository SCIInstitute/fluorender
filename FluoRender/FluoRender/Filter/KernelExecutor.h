/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2025 Scientific Computing and Imaging Institute,
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

#include <Progress.h>
#include <vector>
#include <memory>

class VolumeData;
namespace flvr
{
	class KernelProgram;
	class TextureBrick;
}
class KernelExecutor : public Progress
{
public:
	KernelExecutor();
	~KernelExecutor();

	void SetCode(const std::string &code);
	void LoadCode(const std::wstring &filename);
	void SetVolume(const std::shared_ptr<VolumeData>& vd);
	void SetDuplicate(bool dup);
	void SetRepeat(int val) { m_repeat = val; }
	std::shared_ptr<VolumeData> GetVolume();
	std::shared_ptr<VolumeData> GetResult(bool pop);
	std::wstring GetInfo();

	std::string GetCode() { return m_code; }
	int GetRepeat() { return m_repeat; }
	std::wstring GetFile() { return m_file; }
	void SetFileIndex(int val) { m_file_index = val; }
	int GetFileIndex() { return m_file_index; }

	bool Execute();

private:
	std::weak_ptr<VolumeData> m_vd;
	std::vector<std::shared_ptr<VolumeData>> m_vd_r;//result
	bool m_duplicate;//whether duplicate the input volume
	int m_repeat;//number of execution on top of the base, no duplication

	std::string m_code;
	std::wstring m_message;

	std::wstring m_file;//current file
	int m_file_index;//index of current file in the kernel list

	bool ExecuteKernel(VolumeData* vd, VolumeData* vd_r);
	bool ExecuteKernelBrick(flvr::KernelProgram* kernel_prog,
		unsigned int data_id, void* result,
		size_t brick_x, size_t brick_y,
		size_t brick_z, int chars);

};

#endif//_KERNELEXECUTOR_H_
