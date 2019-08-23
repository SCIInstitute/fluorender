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

#include "DrawVolumeVisitor.hpp"
#include <Group.hpp>
#include <VolumeData/VolumeData.hpp>
#include <Volume/VolumeGroup.hpp>
#include <cctype>

using namespace fluo;

void DrawVolumeVisitor::apply(Node& node)
{
	VolumeData* vd = dynamic_cast<VolumeData*>(&node);
	if (vd)
	{
		bool disp;
		vd->getValue("display", disp);
		if (disp)
		{
			if (m_blend_mode == 2)//multi
				addVolume(DVG_MULTI, vd);
			else
			{
				long blend_mode;
				vd->getValue("blend mode", blend_mode);
				if (blend_mode == 2)
					addVolume(DVG_MULTI, vd);
				else
					addVolume(DVG_COMP, vd);
			}
			if (m_draw_mask)
				addVolume(DVG_MASK, vd);
		}
	}
	traverse(node);
}

void DrawVolumeVisitor::apply(Group& group)
{
	bool result;
	bool disp;
	result = group.getValue("display", disp);
	if (!result || disp)
	{
		long blend_mode;
		result = group.getValue("blend mode", blend_mode);
		m_draw_list.push_back(DrawVolumeGroupList(DVG_MULTI));
		traverse(group);
	}
}

void DrawVolumeVisitor::addVolume(DrawVolumeGroupType type, VolumeData* vd)
{
	if (!m_quota_list)
		getLastList(type).group.push_back(vd);
	else if (!m_quota_list->empty())
		getLastList(type).group.push_back(vd);
}

bool DrawVolumeVisitor::compare_name_asc(VolumeData* vd1, VolumeData* vd2)
{
	if (!vd1 || !vd2)
		return false;
	std::string a = vd1->getName();
	std::string b = vd2->getName();
	for (size_t c = 0; c < a.size() && c < b.size(); c++)
	{
		if (std::tolower(a[c]) != std::tolower(b[c]))
			return (std::tolower(a[c]) > std::tolower(b[c]));
	}
	return a.size() > b.size();
}

bool DrawVolumeVisitor::compare_name_dsc(VolumeData* vd1, VolumeData* vd2)
{
	if (!vd1 || !vd2)
		return false;
	std::string a = vd1->getName();
	std::string b = vd2->getName();
	for (size_t c = 0; c < a.size() && c < b.size(); c++)
	{
		if (std::tolower(a[c]) != std::tolower(b[c]))
			return (std::tolower(a[c]) < std::tolower(b[c]));
	}
	return a.size() < b.size();
}
