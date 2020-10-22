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
#include <Selection/VolumePoint.h>
#include <string>
#include <algorithm>

class VRenderGLView;
class wxFileConfig;
class VolumeData;

namespace FL
{
	class ComponentAnalyzer;
	class RulerHandler
	{
	public:
		RulerHandler();
		~RulerHandler();

		//handle group
		void NewGroup()
		{
			if (m_ruler_list)
			{
				std::vector<unsigned int> groups;
				int num = m_ruler_list->GetGroupNum(groups);
				if (num)
				{
					auto it = std::max_element(groups.begin(), groups.end());
					if (it != groups.end())
						m_group = *it + 1;
				}
			}
		}
		void SetGroup(unsigned int group)
		{
			m_group = group;
		}

		void SetView(VRenderGLView* view)
		{
			m_view = view;
			m_vp.SetView(view);
		}

		void SetVolumeData(VolumeData* vd)
		{
			m_vd = vd;
			m_vp.SetVolumeData(vd);
		}

		void SetCompAnalyzer(ComponentAnalyzer* ca)
		{
			m_ca = ca;
		}

		void SetRuler(FL::Ruler* ruler)
		{
			m_ruler = ruler;
		}

		FL::Ruler* GetRuler()
		{
			return m_ruler;
		}

		int GetRulerIndex()
		{
			if (!m_ruler)
				return -1;
			for (int i = 0; i < m_ruler_list->size(); ++i)
				if ((*m_ruler_list)[i] == m_ruler)
					return i;
			return -1;
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
		void DeletePoint();

		RulerPoint* GetEllipsePoint(int index);

		void FinishRuler();
		bool GetRulerFinished();

		void AddRulerPoint(FLIVR::Point &p);
		void AddRulerPointAfterId(FLIVR::Point &p, unsigned int id, std::set<unsigned int> &cid, std::set<unsigned int> &bid);
		bool GetMouseDist(int mx, int my, double dist);
		void AddRulerPoint(int mx, int my, bool branch);
		void AddPaintRulerPoint();
		bool MoveRuler(int mx, int my);
		bool EditPoint(int mx, int my, bool alt);
		void Prune(int idx, int len);

		void DeleteSelection(std::vector<int> &sel);
		void DeleteAll(bool cur_time);

		void Save(wxFileConfig &fconfig, int vi);
		void Read(wxFileConfig &fconfig, int vi);

		int Profile(int index);
		int Distance(int index, std::string filename);

	private:
		unsigned int m_group;
		VRenderGLView *m_view;
		VolumeData * m_vd;
		ComponentAnalyzer* m_ca;
		VolumePoint m_vp;
		Ruler *m_ruler;
		RulerList *m_ruler_list;
		int m_type;	//0: 2 point; 1: multi point; 2:locator; 3: probe;
					//4: protractor; 5: ellipse

		//find moving distance
		FLIVR::Point m_mouse;//mouse position
		//get point
		FL::pRulerPoint m_point;
		int m_pindex;//index of point in ruler

	private:
	};

}
#endif//_RulerHandler_H_