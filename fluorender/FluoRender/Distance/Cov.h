/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2019 Scientific Computing and Imaging Institute,
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
#ifndef FL_Cov_h
#define FL_Cov_h

#include "DataManager.h"
#include <FLIVR/KernelProgram.h>
#include <FLIVR/VolKernel.h>
#include <FLIVR/Point.h>
#include <vector>

using namespace std;

class VolumeData;
namespace FL
{
	class Cov
	{
	public:
		Cov(VolumeData* vd);
		~Cov();

		void SetUseMask(bool use_mask)
		{
			m_use_mask = use_mask;
		}
		bool GetUseMask()
		{
			return m_use_mask;
		}

		bool Compute(bool type);//type: 0-cov; 1-center only

		std::vector<double> GetCov()
		{
			std::vector<double> cov;
			for (int i = 0; i < 6; ++i)
				cov.push_back(m_cov[i]);
			return cov;
		}

		FLIVR::Point GetCenter()
		{
			FLIVR::Point center;
			center.x(m_center[0]);
			center.y(m_center[1]);
			center.z(m_center[2]);
			return center;
		}

	private:
		VolumeData *m_vd;
		bool m_use_mask;//use mask instead of data
		bool m_use_int;//use intensity values as weights
		//result
		float m_cov[6];//covariance matrix {xx, xy, xz, yy, yz, zz}
		float m_center[3];//center

		bool ComputeCenter();
		bool ComputeCov();
		bool CheckBricks();
		bool GetInfo(FLIVR::TextureBrick* b,
			long &bits, long &nx, long &ny, long &nz);
		void ReleaseData(void* val, long bits);
		long OptimizeGroupSize(long nt, long target);
	};

}
#endif//FL_Cov_h
