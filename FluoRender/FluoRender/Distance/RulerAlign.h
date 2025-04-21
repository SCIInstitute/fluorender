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
#ifndef FL_RulerAlign_h
#define FL_RulerAlign_h

#include <Vector.h>

class RenderView;
class VolumeData;
namespace flrd
{
	class Ruler;
	class RulerList;
	class RulerAlign
	{
	public:
		RulerAlign():
			m_view(0),
			m_vd(0),
			m_axis_type(0),
			m_align_center(false) {};
		~RulerAlign() {};

		void SetView(RenderView* view)
		{
			m_view = view;
		}

		void SetVolumeData(VolumeData* vd)
		{
			m_vd = vd;
		}

		void AddRuler(Ruler* ruler);

		void SetRulerList(RulerList* list);

		void Clear()
		{
			m_point_list.clear();
		}

		void SetRuler(Ruler* ruler)
		{
			Clear();
			AddRuler(ruler);
		}

		//void SetCovMat(std::vector<double> &cov)
		//{
		//	m_cov.assign(cov.begin(), cov.end());
		//}

		void SetAxisType(int val)
		{
			m_axis_type = val;
		}

		void SetAlignCenter(bool val)
		{
			m_align_center = val;
		}
		bool GetAlignCenter()
		{
			return m_align_center;
		}

		void AlignRuler();//axis_type: 0-x; 1-y; 2-z
		void AlignPca(bool);//0-xyz; 1-yxz; 2-zxy, 3-xzy; 4-yzx; 5-zyx

	private:
		RenderView *m_view;
		VolumeData* m_vd;
		std::vector<fluo::Point> m_point_list;
		int m_axis_type;
		fluo::Vector m_axis;
		fluo::Vector m_axis_x;
		fluo::Vector m_axis_y;
		fluo::Vector m_axis_z;
		//std::vector<double> m_cov;
		bool m_align_center;
	};
}

#endif//FL_RulerAlign_h