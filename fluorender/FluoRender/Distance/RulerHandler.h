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

#ifndef _RulerHandler_H_
#define _RulerHandler_H_

#include <Distance/Ruler.h>

class VRenderGLView;
class wxFileConfig;

namespace FL
{
	class RulerHandler
	{
	public:
		RulerHandler();
		~RulerHandler();

		void SetView(VRenderGLView* view)
		{
			m_view = view;
		}

		void SetRuler(FL::Ruler* ruler)
		{
			m_ruler = ruler;
		}

		FL::Ruler* GetRuler()
		{
			return m_ruler;
		}

		void SetRulerList(FL::RulerList* ruler_list)
		{
			m_ruler_list = ruler_list;
		}

		FL::RulerList* GetRulerList()
		{
			return m_ruler_list;
		}

		void SetType(int type)
		{
			m_type = type;
		}

		int GetType()
		{
			return m_type;
		}

		bool FindEditingRuler(double mx, double my);

		void SetPoint(FL::pRulerPoint point)
		{
			m_point = point;
		}
		RulerPoint* GetPoint()
		{
			return m_point.get();
		}

		RulerPoint* GetEllipsePoint(int index);

		void FinishRuler();
		bool GetRulerFinished();

		void AddRulerPoint(int mx, int my);
		void AddPaintRulerPoint();
		bool MoveRuler(int mx, int my);
		bool EditPoint(int mx, int my, bool alt);

		void Save(wxFileConfig &fconfig, int vi);
		void Read(wxFileConfig &fconfig, int vi);

	private:
		VRenderGLView *m_view;
		FL::Ruler *m_ruler;
		FL::RulerList *m_ruler_list;
		int m_type;	//0: 2 point; 1: multi point; 2:locator; 3: probe;
					//4: protractor; 5: ellipse

		//get point
		FL::pRulerPoint m_point;
		int m_pindex;//index of point in ruler

	private:
	};

}
#endif//_RulerHandler_H_