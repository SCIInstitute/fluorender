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
#ifndef _COMBINELIST_H_
#define _COMBINELIST_H_

#include <Vector.h>
#include <list>
#include <string>
#include <memory>

class VolumeData;

namespace flrd
{
	class CombineList
	{
	public:
		CombineList() {};
		~CombineList() {};

		void SetName(const std::wstring &name);
		void SetVolumes(const std::list<std::weak_ptr<VolumeData>> &channs);
		std::list<std::shared_ptr<VolumeData>> GetResults();
		int Execute();

	private:
		std::list<std::weak_ptr<VolumeData>> m_channs;
		std::list<std::shared_ptr<VolumeData>> m_results;
		fluo::Vector m_size;
		fluo::Vector m_spacing;
		int m_bits;
		std::wstring m_name;

	private:
		unsigned char Inc(unsigned char base, unsigned char inc);
		unsigned short Inc(unsigned short base, unsigned short inc);
	};
}
#endif//_COMBINELIST_H_