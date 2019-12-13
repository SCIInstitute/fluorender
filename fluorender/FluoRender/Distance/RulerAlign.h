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
		RulerAlign() {};
		~RulerAlign() {};

		void SetView(VRenderGLView* view)
		{
			m_view = view;
		}

		void AddRuler(Ruler* ruler)
		{
			m_ruler_list.push_back(ruler);
		}

		void SetRulerList(RulerList* list)
		{
			m_ruler_list.assign(list->begin(), list->end());
		}

		void Clear()
		{
			m_ruler_list.clear();
		}

		void SetRuler(Ruler* ruler)
		{
			Clear();
			AddRuler(ruler);
		}

		void AlignRuler(int axis_type);//axis_type: 0-x; 1-y; 2-z

	private:
		VRenderGLView *m_view;
		RulerList m_ruler_list;
		FLIVR::Vector m_axis_x;
		FLIVR::Vector m_axis_y;
		FLIVR::Vector m_axis_z;
	};
}

#endif//FL_RulerAlign_h