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
#ifndef FL_RulerAlign_h
#define FL_RulerAlign_h

#include <Distance/Ruler.h>
#include <FLIVR/Vector.h>

class VRenderGLView;

namespace FL
{
	class RulerAlign
	{
	public:
		RulerAlign():
			m_view(0),
			m_axis_type(0) {};
		~RulerAlign() {};

		void SetView(VRenderGLView* view)
		{
			m_view = view;
		}

		void AddRuler(Ruler* ruler)
		{
			FLIVR::Point p;
			for (size_t i = 0; i < ruler->GetNumPoint(); ++i)
			{
				RulerPoint* point = ruler->GetPoint(i);
				if (!point)
					continue;
				p = point->GetPoint();
				m_point_list.push_back(p);
			}
		}

		void SetRulerList(RulerList* list)
		{
			Clear();
			for (size_t i = 0; i < list->size(); ++i)
			{
				Ruler* ruler = (*list)[i];
				if (!ruler)
					continue;
				AddRuler(ruler);
			}
		}

		void Clear()
		{
			m_point_list.clear();
		}

		void SetRuler(Ruler* ruler)
		{
			Clear();
			AddRuler(ruler);
		}

		void SetCovMat(std::vector<double> &cov)
		{
			m_cov.assign(cov.begin(), cov.end());
		}

		void AlignRuler(int axis_type);//axis_type: 0-x; 1-y; 2-z
		void AlignPca(int axis_type, bool cov=true);//0-xyz; 1-yxz; 2-zxy, 3-xzy; 4-yzx; 5-zyx

	private:
		VRenderGLView *m_view;
		std::vector<FLIVR::Point> m_point_list;
		int m_axis_type;
		FLIVR::Vector m_axis;
		FLIVR::Vector m_axis_x;
		FLIVR::Vector m_axis_y;
		FLIVR::Vector m_axis_z;
		std::vector<double> m_cov;
	};
}

#endif//FL_RulerAlign_h