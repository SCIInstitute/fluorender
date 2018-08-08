/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2014 Scientific Computing and Imaging Institute,
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

#include <Scenegraph/Group.h>

using namespace FL;

Group::Group()
{

}

Group::Group(const Group& group, const CopyOp& copyop) :
	Node(group, copyop)
{
	for (auto it = m_children.begin();
		it != m_children.end(); ++it)
	{
		//copy
	}
}

Group::~Group()
{

}

bool Group::addChild(pNode &child)
{
	return insertChild(m_children.size(), child);
}

bool Group::insertChild(size_t index, pNode &child)
{
	if (child == nullptr)
		return false;

	if (index >= m_children.size())
		m_children.push_back(child);
	else
		m_children.insert(m_children.begin() + index, child);

	return true;
}

bool Group::removeChildren(size_t pos, size_t num)
{
	if (pos < m_children.size() && num > 0)
	{
		size_t end = pos + num;
		if (end > m_children.size())
			end = m_children.size();
		m_children.erase(m_children.begin() + pos, m_children.begin() + end);

		return true;
	}
	return false;
}

bool Group::replaceChild(pNode &orig_child, pNode &new_child)
{
	if (orig_child == nullptr ||
		new_child == nullptr)
		return false;

	size_t pos = getChildIndex(orig_child);
	if (pos < m_children.size())
		return setChild(pos, new_child);
	return false;
}

bool Group::setChild(size_t i, pNode &node)
{
	if (i < m_children.size() && node != nullptr)
	{
		m_children[i] = node;
		return true;
	}
	return false;
}