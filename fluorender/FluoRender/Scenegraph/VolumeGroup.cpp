/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2021 Scientific Computing and Imaging Institute,
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

#include "VolumeGroup.hpp"
#include <VolumeData.hpp>

using namespace fluo;

VolumeGroup::VolumeGroup()
{

}

VolumeGroup::VolumeGroup(const VolumeGroup& group, const CopyOp& copyop) :
	Group(group, copyop)
{

}

VolumeGroup::VolumeGroup(const VolumeData& vd, const CopyOp& copyop) :
	Group()
{
	copyValues(vd, copyop);
}

VolumeGroup::~VolumeGroup()
{

}

bool VolumeGroup::insertChild(size_t index, Node* child)
{
	if (child)
	{
		std::string ss[] = {
			gstGammaR,
			gstGammaG,
			gstGammaB,
			gstBrightnessR,
			gstBrightnessG,
			gstBrightnessB,
			gstEqualizeR,
			gstEqualizeG,
			gstEqualizeB,
			gstSyncR,
			gstSyncG,
			gstSyncB
		};
		ValueCollection names(std::begin(ss), std::end(ss));
		syncValues(names, child);
		child->syncValues(names, this);
	}
	return Group::insertChild(index, child);
}

bool VolumeGroup::removeChildren(size_t pos, size_t num)
{
	if (pos < m_children.size() && num > 0)
	{
		size_t end = pos + num;
		if (end > m_children.size())
			end = m_children.size();

		std::string ss[] = {
			gstGammaR,
			gstGammaG,
			gstGammaB,
			gstBrightnessR,
			gstBrightnessG,
			gstBrightnessB,
			gstEqualizeR,
			gstEqualizeG,
			gstEqualizeB,
			gstSyncR,
			gstSyncG,
			gstSyncB
		};
		ValueCollection names(std::begin(ss), std::end(ss));

		for (unsigned int i = pos; i < end; ++i)
		{
			Node* child = m_children[i].get();
			if (child)
			{
				unsyncValues(names, child);
				child->unsyncValues(names, this);
			}
		}
	}
	return Group::removeChildren(pos, num);
}

bool VolumeGroup::setChild(size_t i, Node* node)
{
	if (i < m_children.size() && node)
	{
		std::string ss[] = {
			gstGammaR,
			gstGammaG,
			gstGammaB,
			gstBrightnessR,
			gstBrightnessG,
			gstBrightnessB,
			gstEqualizeR,
			gstEqualizeG,
			gstEqualizeB,
			gstSyncR,
			gstSyncG,
			gstSyncB
		};
		ValueCollection names(std::begin(ss), std::end(ss));

		Node* orig_node = m_children[i].get();
		if (orig_node)
		{
			unsyncValues(names, orig_node);
			orig_node->unsyncValues(names, this);
		}

		syncValues(names, node);
		node->syncValues(names, this);
	}
	return Group::setChild(i, node);
}

void VolumeGroup::OnRandomizeColor(Event& event)
{
	//maybe there is a better solution?
	Node* node = dynamic_cast<Node*>(event.penultimate());
	if (node && containsNode(node))
		return;

	for (auto it = m_children.begin();
		it != m_children.end(); ++it)
	{
		bool bval;
		(*it)->toggleValue(gstRandomizeColor, bval, event);
	}
}

void VolumeGroup::AddMask(Nrrd* mask, int op)
{
	for (auto it = m_children.begin();
		it != m_children.end(); ++it)
	{
		(*it)->asVolumeData()->AddMask(mask, op);
	}
}
