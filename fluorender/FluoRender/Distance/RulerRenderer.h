/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2020 Scientific Computing and Imaging Institute,
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

#include <Distance/Ruler.h>
#include <FLIVR/Color.h>
#include <FLIVR/Point.h>
#include <FLIVR/Transform.h>
#include <vector>

class VRenderGLView;

namespace FL
{
	class RulerRenderer
	{
	public:
		RulerRenderer();
		~RulerRenderer();

		void SetView(VRenderGLView* view)
		{
			m_view = view;
		}

		void SetRulerList(FL::RulerList* ruler_list)
		{
			m_ruler_list = ruler_list;
		}

		FL::RulerList* GetRulerList()
		{
			return m_ruler_list;
		}

		void SetLineSize(double val)
		{
			m_line_size = val;
		}

		void SetDrawText(bool val)
		{
			m_draw_text = val;
		}

		void Draw();

	private:
		VRenderGLView *m_view;
		RulerList *m_ruler_list;
		double m_line_size;
		bool m_draw_text;

	private:
		unsigned int DrawVerts(std::vector<float> &verts);
		void DrawPoint(std::vector<float> &verts, int type, float px, float py, float w, FLIVR::Color &c);
		void DrawArc(FLIVR::Point & ppc, FLIVR::Point& pp0, FLIVR::Point& pp1,
			FLIVR::Color &c, FLIVR::Transform& mv, FLIVR::Transform& p,
			std::vector<float> &verts, unsigned int& num);
		void DrawText(int, int, int);
	};

}
#endif//_RulerRenderer_H_