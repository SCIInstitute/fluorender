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

#ifndef _RulerRenderer_H_
#define _RulerRenderer_H_

#include <vector>
#include <set>

class RenderView;
namespace fluo
{
	class Color;
	class Point;
	class Transform;
}
namespace flrd
{
	class RulerList;
	class RulerRenderer
	{
	public:
		RulerRenderer();
		~RulerRenderer();

		void SetView(RenderView* view)
		{
			m_view = view;
		}

		void SetRulerList(flrd::RulerList* ruler_list)
		{
			m_ruler_list = ruler_list;
		}

		flrd::RulerList* GetRulerList()
		{
			return m_ruler_list;
		}

		void SetLineSize(double val)
		{
			m_line_size = val;
		}

		void SetSelLineSize(double val)
		{
			m_sel_line_size = val;
		}

		void SetDrawText(bool val)
		{
			m_draw_text = val;
		}

		void SetSelection(const std::set<int>& sel_list)
		{
			m_sel_list = sel_list;
		}

		void Draw();

	private:
		RenderView *m_view;
		RulerList *m_ruler_list;
		double m_line_size;
		double m_sel_line_size;//line size for selected ruler
		bool m_draw_text;
		std::set<int> m_sel_list;//index to selected rulers

	private:
		unsigned int DrawVerts(std::vector<float> &verts, int sel_mode);//sel_mode: 0: all; 1: selected; 2 unselected
		void DrawPoint(std::vector<float> &verts, int type, float px, float py, float w, fluo::Color &c);
		void DrawArc(fluo::Point & ppc, fluo::Point& pp0, fluo::Point& pp1,
			fluo::Color &c, fluo::Transform& mv, fluo::Transform& p,
			std::vector<float> &verts, unsigned int& num);
		void DrawTextAt(int, int);
	};

}
#endif//_RulerRenderer_H_