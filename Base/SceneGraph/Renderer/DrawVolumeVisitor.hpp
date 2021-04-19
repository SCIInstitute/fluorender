/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2018 Scientific Computing and Imaging Institute,
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

//the draw volume visitor simply copies code from the old renderglview
//a new render pipeline should replace this
#ifndef DRAWVOLUMEVISITOR_HPP
#define DRAWVOLUMEVISITOR_HPP

#include <NodeVisitor.hpp>

namespace fluo
{
	class DrawVolumeVisitor : public NodeVisitor
	{
	public:
		enum DrawVolumeGroupType
		{
			DVG_COMP = 0,
			DVG_MASK,
			DVG_MULTI
		};
		struct DrawVolumeGroupList
		{
			std::vector<VolumeData*> group;
			DrawVolumeGroupType type;

			DrawVolumeGroupList(DrawVolumeGroupType type) :type(type) {}
		};
		DrawVolumeVisitor() : NodeVisitor(), m_quota_list(0), m_sort_method(0)
		{
            setTraversalMode(NodeVisitor::TRAVERSE_ALL_CHILDREN);
		}

		virtual void reset()
		{
			m_quota_list = 0;
			m_draw_list.clear();
			m_sort_method = 0;
		}

        virtual void apply(Node& node);

        virtual void apply(Group& group);

		void setBlendMode(long mode)
		{
			m_blend_mode = mode;
			if (m_blend_mode == 2)//multi
			{
				m_draw_list.push_back(DrawVolumeGroupList(DVG_MASK));
				m_draw_list.push_back(DrawVolumeGroupList(DVG_MULTI));
			}
		}
		void setDrawMask(bool draw_mask) { m_draw_mask = draw_mask; }
		void setQuotaList(std::vector<VolumeData*>* list) { m_quota_list = list; }
		void setSortValue(const std::string &sort_value) { m_sort_value = sort_value; }
		void setSortMethod(long sort_method) { m_sort_method = sort_method; }

		std::vector<DrawVolumeGroupList>& getResult()
		{
			if (m_sort_method > 0)
			{
				for (auto it = m_draw_list.begin();
					it != m_draw_list.end(); ++it)
				{
					if (it->type == DVG_MASK ||
						it->type == DVG_MULTI)
						continue;
					if (m_sort_value == "name")
					{
						if (m_sort_method == 1)
							std::sort(it->group.begin(), it->group.end(), compare_name_asc);
						else if (m_sort_method == 2)
							std::sort(it->group.begin(), it->group.end(), compare_name_dsc);
					}
				}
			}

			return m_draw_list;
		}

	private:
		long m_blend_mode;
		bool m_draw_mask;
		std::vector<DrawVolumeGroupList> m_draw_list;
		std::vector<VolumeData*> *m_quota_list;
		std::string m_sort_value;
		long m_sort_method;

		DrawVolumeGroupList& getLastList(DrawVolumeGroupType type)
		{
			if (m_draw_list.empty())
			{
				m_draw_list.push_back(DrawVolumeGroupList(type));
				return m_draw_list.back();
			}
			if (type == m_draw_list.back().type)
				return m_draw_list.back();
			else
			{
				if (type == DVG_MULTI)
				{
					m_draw_list.push_back(DrawVolumeGroupList(type));
					return m_draw_list.back();
				}
				else
				{
					for (auto it = m_draw_list.rbegin();
						it != m_draw_list.rend(); ++it)
					{
						if ((*it).type == type)
							return *it;
					}
				}
			}
			m_draw_list.push_back(DrawVolumeGroupList(type));
			return m_draw_list.back();
		}

		void addVolume(DrawVolumeGroupType type, VolumeData* vd);

		static bool compare_name_asc(VolumeData* vd1, VolumeData* vd2);
		static bool compare_name_dsc(VolumeData* vd1, VolumeData* vd2);
	};
}
#endif//_DRAWVOLUMEVISITOR_H_
