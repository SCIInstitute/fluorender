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
#include <FLIVR/TextureBrick.h>

using namespace std;

namespace FL
{
	class PaintBoxes
	{
	public:
		PaintBoxes() : m_bricks(0), m_paint_tex(0),
			m_ptx(0), m_pty(0), m_persp(false) {}
		~PaintBoxes() {}

		void SetBricks(vector<FLIVR::TextureBrick*> *bricks)
		{
			m_bricks = bricks;
			for (int i = 0; i < bricks->size(); ++i)
				(*bricks)[i]->set_paint_mask(false);
		}

		void EnableBricks()
		{
			for (int i = 0; i < m_bricks->size(); ++i)
				(*m_bricks)[i]->set_paint_mask(true);
		}

		void SetPaintTex(int pt, int ptx, int pty)
		{
			m_paint_tex = pt;
			m_ptx = ptx;
			m_pty = pty;
		}

		void SetPersp(bool val)
		{
			m_persp = val;
		}

		void SetMats(FLIVR::Transform &mv, FLIVR::Transform &pr, FLIVR::Transform &mat)
		{
			m_mv = mv;
			m_pr = pr;
			mv.invert();
			pr.invert();
			m_imv = mv;
			m_ipr = pr;
			//combined
			m_mat = mat;
			mat.invert();
			m_imat = mat;
		}

		void Compute();

	private:
		vector<FLIVR::TextureBrick*> *m_bricks;
		int m_paint_tex;//2d tex of paint strokes
		int m_ptx, m_pty;//tex size
		//for persp
		bool m_persp;
		FLIVR::Transform m_mat;//combined mat
		FLIVR::Transform m_imat;//combined invert
		FLIVR::Transform m_mv;//modelview
		FLIVR::Transform m_pr;//projection
		FLIVR::Transform m_imv;//modelview invert
		FLIVR::Transform m_ipr;//projection invert

		struct BrickBox
		{
			FLIVR::BBox bbox;
			FLIVR::TextureBrick* brick;
		};

	private:
		bool GetBrickBoxes(vector<BrickBox> &bbs);
		bool test_against_view(const FLIVR::BBox &bbox);
	};
}

#endif//FL_PaintBoxes_h