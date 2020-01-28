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
#ifndef FL_PaintBoxes_h
#define FL_PaintBoxes_h

#include <FLIVR/KernelProgram.h>
#include <FLIVR/VolKernel.h>
#include <FLIVR/BBox.h>
#include <FLIVR/Vector.h>
#include <FLIVR/Transform.h>

using namespace std;

namespace FL
{
	class PaintBoxes
	{
	public:
		PaintBoxes() : m_type(0), m_paint_tex(0) {}
		~PaintBoxes() {}

		void SetBoxes(vector<FLIVR::BBox> &boxes)
		{
			m_boxes = boxes;
		}

		void SetPaintTex(int pt)
		{
			m_paint_tex = pt;
		}

		void SetOrtho(FLIVR::Vector & dir)
		{
			m_type = 1;
			m_dir = dir;
		}

		void SetPersp(FLIVR::Transform &mv, FLIVR::Transform &pr)
		{
			m_type = 2;
			m_mv = mv;
			m_pr = pr;
		}

		void Compute()
		{
			switch (m_type)
			{
			case 1:
				ComputeOrtho();
				break;
			case 2:
				ComputePersp();
				break;
			}
		}

	private:
		int m_type;//1-ortho; 2-persp
		vector<FLIVR::BBox> m_boxes;
		int m_paint_tex;//2d tex of paint strokes
		//for ortho
		FLIVR::Vector m_dir;
		//for persp
		FLIVR::Transform m_mv;//modelview
		FLIVR::Transform m_pr;//projection

	private:
		void ComputeOrtho();
		void ComputePersp();
	};
}

#endif//FL_PaintBoxes_h