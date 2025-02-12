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
#ifndef FL_Count_h
#define FL_Count_h

#include <DataManager.h>
#include <KernelProgram.h>
#include <VolKernel.h>

class VolumeData;
namespace flrd
{
	class CountVoxels
	{
	public:
		CountVoxels(VolumeData* vd);
		~CountVoxels();

		void SetUseMask(bool use_mask)
		{ m_use_mask = use_mask; }
		bool GetUseMask()
		{ return m_use_mask; }

		void Count();
		unsigned int GetSum()
		{ return m_sum; }
		float GetWeightedSum()
		{ return m_wsum; }

	private:
		VolumeData *m_vd;
		bool m_use_mask;//use mask instead of data
		//result
		unsigned int m_sum;
		float m_wsum;

		bool CheckBricks();
		bool GetInfo(flvr::TextureBrick* b,
			long &bits, long &nx, long &ny, long &nz);
		void* GetVolDataBrick(flvr::TextureBrick* b);
		void* GetVolData(VolumeData* vd);
	};

}
#endif//FL_Count_h
