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
#ifndef FL_BackgStat_h
#define FL_BackgStat_h

#include <vector>
#include <string>
#include <map>

class VolumeData;
namespace flvr
{
	class TextureBrick;
}
namespace flrd
{
	class BackgStat
	{
	public:
		BackgStat(VolumeData* vd);
		~BackgStat();

		void SetUseMask(bool use_mask)
		{
			m_use_mask = use_mask;
		}
		bool GetUseMask()
		{
			return m_use_mask;
		}
		void SetType(int type)
		{
			m_type = type;
		}
		void SetFeatureSize2D(int kx, int ky)
		{
			m_kx = kx;
			m_ky = ky;
		}
		void SetFeatureSize3D(int kx, int ky, int kz)
		{
			m_kx = kx;
			m_ky = ky;
			m_kz = kz;
		}
		void SetThreshold(float varth, float gauth)
		{
			m_varth = varth;
			m_gauth = gauth;
		}

		void Run();
		std::string GetTypeName(int index = 0)
		{
			std::string result = "n/a";
			switch (m_type)
			{
			case 0:
				result = "bkg_mean";
				break;
			case 1:
				switch (index)
				{
				case 0:
					result = "bkg_min";
					break;
				case 1:
					result = "bkg_max";
					break;
				}
				break;
			case 2:
				switch (index)
				{
				case 0:
					result = "bkg_median";
					break;
				case 1:
					result = "bkg_mode";
					break;
				}
				break;
			}
			return result;
		}
		float GetResultf(int index = 0)
		{
			switch (m_type)
			{
			case 0:
				return m_sum > 0 ? m_wsum / m_sum : 0;
			case 1:
				switch (index)
				{
				case 0:
					//min
					return float(m_minv);
				case 1:
					//max
					return float(m_maxv);
				}
				break;
			case 2:
				switch (index)
				{
				case 0:
					//median
					return float(m_medv);
				case 1:
					//mode
					return float(m_modv);
				}
				break;
			}
			return 0;
		}
		unsigned int GetResulti(int index = 0)
		{
			switch (m_type)
			{
			case 0:
				return m_sum > 0 ? (unsigned int)(m_wsum / m_sum + 0.5) : 0;
			case 1:
				switch (index)
				{
				case 0:
					//min
					return m_minv;
				case 1:
					//max
					return m_maxv;
				}
				break;
			case 2:
				switch (index)
				{
				case 0:
					//median
					return m_medv;
				case 1:
					//mode
					return m_modv;
				}
				break;
			}
			return 0;
		}

	private:
		VolumeData *m_vd;
		bool m_use_mask;//use mask instead of data
		int m_type;//0-mean; 1-minmax; 2-median
		int m_kx, m_ky, m_kz;
		float m_varth, m_gauth;
		//result
		unsigned int m_sum;
		float m_wsum;
		unsigned int m_minv;
		unsigned int m_maxv;
		unsigned int m_medv;
		unsigned int m_modv;
		std::map<unsigned int, unsigned int> m_hist;
		std::vector<unsigned int> m_hist_acc;

		bool CheckBricks();
		bool GetInfo(flvr::TextureBrick* b,
			long &bits, long &nx, long &ny, long &nz);
	};

}
#endif//FL_BackgStat_h
