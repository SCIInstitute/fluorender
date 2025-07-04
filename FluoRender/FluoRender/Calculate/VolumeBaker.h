﻿/*
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
#ifndef _VOLUMEBAKER_H_
#define _VOLUMEBAKER_H_

#include <nrrd.h>
#include <memory>

class VolumeData;
namespace flrd
{
	class VolumeBaker
	{
	public:
		VolumeBaker();
		~VolumeBaker();

		void SetInput(const std::shared_ptr<VolumeData>& data);
		std::shared_ptr<VolumeData> GetInput();
		std::shared_ptr<VolumeData> GetResult();
		void Bake(bool replace);

	private:
		std::weak_ptr<VolumeData> m_input;	//input
		std::shared_ptr<VolumeData> m_result;	//result
		void* m_raw_input;		//
		void* m_raw_result;		//

		//size
		int m_nx;
		int m_ny;
		int m_nz;
		//input bits
		int m_bits;

	private:
		Nrrd* GetNrrd(VolumeData* vd);
		void* GetRaw(VolumeData* vd);
	};
}
#endif//_VOLUMEBAKER_H_