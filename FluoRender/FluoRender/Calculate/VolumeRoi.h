/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2023 Scientific Computing and Imaging Institute,
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
#ifndef FL_Volume_Roi_h
#define FL_Volume_Roi_h

#include <DataManager.h>
#include <FLIVR/KernelProgram.h>
#include <FLIVR/VolKernel.h>

class VolumeData;
namespace flrd
{
	class VolumeRoi
	{
	public:
		VolumeRoi(VolumeData* vd);
		~VolumeRoi();

		void SetUseMask(bool use_mask)
		{
			m_use_mask = use_mask;
		}
		bool GetUseMask()
		{
			return m_use_mask;
		}
		void SetTransform(fluo::Transform& tf)
		{
			m_tf = tf;
		}
		fluo::Transform GetTransform()
		{
			return m_tf;
		}
		void SetRoi(Ruler* ruler)
		{
			m_ruler = ruler;
		}
		Ruler* GetRoi()
		{
			return m_ruler;
		}
		void SetAspect(int nx, int ny)
		{
			if (ny > 0)
				m_aspect = double(nx) / double(ny);
			else
				m_aspect = 1;
		}
		double GetAspect()
		{
			return m_aspect;
		}

		void Run();
		double GetResult();
		double GetResultf()
		{
			return m_sum > 0 ? m_wsum / m_sum : 0;
		}

	private:
		VolumeData* m_vd;
		bool m_use_mask;//use mask instead of data
		fluo::Transform m_tf;//view transform
		Ruler* m_ruler;//a closed ruler for roi
		double m_aspect;//nx/ny of render view

		//result
		unsigned int m_sum;
		float m_wsum;

		bool CheckBricks();
		bool GetInfo(flvr::TextureBrick* b,
			long& bits, long& nx, long& ny, long& nz);
	};
}
#endif//FL_Volume_Roi_h