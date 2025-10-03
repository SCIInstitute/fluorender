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
#ifndef FL_Compare_h
#define FL_Compare_h

#include <string>
#include <functional>
#include <memory>

class VolumeData;
namespace flvr
{
	class TextureBrick;
	class Argument;
}
namespace flrd
{
	typedef std::function<void(const std::string&)> ChannelCompareFunc;

	class ChannelCompare
	{
	public:
		ChannelCompare(VolumeData* vd1, VolumeData* vd2);
		~ChannelCompare();

		void SetUseMask(bool use_mask)
		{ m_use_mask = use_mask; }
		bool GetUseMask()
		{ return m_use_mask; }
		void SetIntWeighted(bool bval)
		{ m_int_weighted = bval; }
		bool GetCountVoxel()
		{ return m_int_weighted;}

		void Product();
		void MinValue();
		void Threshold(float th1, float th2, float th3, float th4);
		void Average(float weight, std::weak_ptr<flvr::Argument> avg);
		double Result()
		{ return m_result; }

		//update progress
		ChannelCompareFunc prework;
		ChannelCompareFunc postwork;

	private:
		VolumeData *m_vd1, *m_vd2;
		bool m_use_mask;//use mask instead of data
		bool m_int_weighted;//sum of intensity instead of voxel count
		bool m_init;
		double m_result;

		bool CheckBricks();
		bool GetInfo(flvr::TextureBrick* b1, flvr::TextureBrick* b2,
			long &bits, long &bits2,
			long &nx, long &ny, long &nz);
		void* GetVolDataBrick(flvr::TextureBrick* b);
		void* GetVolData(VolumeData* vd);
	};

}
#endif//FL_Compare_h
