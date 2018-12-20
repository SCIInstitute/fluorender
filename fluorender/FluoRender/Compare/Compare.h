/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2018 Scientific Computing and Imaging Institute,
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

using namespace std;

namespace FLIVR
{
	class TextureBrick;
}
namespace FL
{
	class VolumeData;
	class ChannelCompare
	{
	public:
		ChannelCompare(VolumeData* vd1, VolumeData* vd2);
		~ChannelCompare();

		void SetUseMask(bool use_mask)
		{ m_use_mask = use_mask; }
		bool GetUseMask()
		{ return m_use_mask; }

		void Compare(float, float);
		double Result()
		{ return m_result; }

	private:
		VolumeData *m_vd1, *m_vd2;
		bool m_use_mask;//use mask instead of data
		bool m_init;
		double m_result;

		bool CheckBricks();
		bool GetInfo(FLIVR::TextureBrick* b1, FLIVR::TextureBrick* b2,
			long &bits, long &bits2,
			long &nx, long &ny, long &nz);
		void* GetVolDataBrick(FLIVR::TextureBrick* b);
		void* GetVolData(VolumeData* vd);
		void ReleaseData(void* val, long bits);
	};

}
#endif//FL_Compare_h
