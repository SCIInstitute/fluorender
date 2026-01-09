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

#ifndef FL_VolumePoint_h_
#define FL_VolumePoint_h_

#include <memory>

class VolumeData;
namespace fluo
{
	class Point;
}
namespace flrd
{
	class VolumePoint
	{
	public:
		VolumePoint() {}
		~VolumePoint() {}

		void SetVolumeData(const std::shared_ptr<VolumeData>& vd)
		{
			m_vd = vd;
		}

		//mode: 0-maximum with original value; 1-maximum with transfered value; 2-accumulated with original value; 3-accumulated with transfered value
		double GetPointVolume(
			double mx, double my,//mouse coord on screen
			int mode, bool use_transf, double thresh,//params
			fluo::Point &mp, fluo::Point &ip);
		double GetPointVolumeBoxOnePoint(
			double mx, double my,//mouse coord on screen
			fluo::Point &mp);
		double GetPointVolumeBoxTwoPoint(
			double mx, double my,//mouse coord on screen
			fluo::Point &p1, fluo::Point &p2);
		double GetPointPlane(
			double mx, double my,//mouse coord on screen
			fluo::Point* planep,
			fluo::Point &mp);

	private:
		std::weak_ptr<VolumeData> m_vd;
	};
}

#endif//FL_VolumePoint_h_