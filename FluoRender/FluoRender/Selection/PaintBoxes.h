/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2024 Scientific Computing and Imaging Institute,
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
#include <Types/BBox.h>
#include <Types/Vector.h>
#include <Types/Transform.h>
#include <FLIVR/TextureBrick.h>

using namespace std;

namespace flrd
{
	class PaintBoxes
	{
	public:
		PaintBoxes() : m_bricks(0), m_paint_tex(0),
			m_ptx(0), m_pty(0), m_persp(false),
			m_view_only(false), m_mouse_pos(false) {}
		~PaintBoxes() {}

		void SetBricks(vector<flvr::TextureBrick*> *bricks)
		{
			m_bricks = bricks;
		}

		void SetPaintTex(int pt, int ptx, int pty)
		{
			m_paint_tex = pt;
			m_ptx = ptx;
			m_pty = pty;
		}

		void SetViewOnly(bool value)
		{
			m_view_only = value;
		}

		void SetMousePos(int mx, int my)
		{
			if (mx > 0 && my > 0)
			{
				m_mouse_pos = true;
				m_mx = mx; m_my = my;
			}
		}

		void SetPersp(bool val)
		{
			m_persp = val;
		}

		void SetMats(fluo::Transform &mv, fluo::Transform &pr, fluo::Transform &mat)
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
		vector<flvr::TextureBrick*> *m_bricks;
		int m_paint_tex;//2d tex of paint strokes
		int m_ptx, m_pty;//tex size
		bool m_view_only;//only test for view intersection
		bool m_mouse_pos;//use mouse position to determine brick
		int m_mx, m_my;//mouse click position
		//for persp
		bool m_persp;
		fluo::Transform m_mat;//combined mat
		fluo::Transform m_imat;//combined invert
		fluo::Transform m_mv;//modelview
		fluo::Transform m_pr;//projection
		fluo::Transform m_imv;//modelview invert
		fluo::Transform m_ipr;//projection invert

		struct BrickBox
		{
			fluo::BBox bbox;
			flvr::TextureBrick* brick;
		};

	private:
		bool GetBrickBoxes(vector<BrickBox> &bbs);
		void BrickViewInt();
		void BrickRayInt();
		bool test_against_view(const fluo::BBox &bbox);
	};
}

#endif//FL_PaintBoxes_h