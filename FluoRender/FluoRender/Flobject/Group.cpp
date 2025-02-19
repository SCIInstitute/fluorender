/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2025 Scientific Computing and Imaging Institute,
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

#include <Group.hpp>
#include <NodeVisitor.hpp>

using namespace fluo;

Group::Group()
{

}

Group::Group(const Group& group, const CopyOp& copyop, bool copy_values) :
	Node(group, copyop, false)
{
	for (auto it = group.m_children.begin();
		it != group.m_children.end(); ++it)
	{
		//copy
		Node* child = copyop(it->get());
		if (child) addChild(child);
	}
	if (copy_values)
		copyValues(group, copyop);
}

Group::~Group()
{
	for (auto it = m_children.begin();
		it != m_children.end(); ++it)
	{
		(*it)->removeParent(this);
	}
}

void Group::traverse(NodeVisitor& nv, bool reverse)
{
	if (reverse)
	{
		for (auto it = m_children.rbegin();
			it != m_children.rend(); ++it)
			(*it)->accept(nv);
	}
	else
	{
		for (auto it = m_children.begin();
			it != m_children.end(); ++it)
			(*it)->accept(nv);
	}
}

bool Group::addChild(Node* child)
{
	return insertChild(m_children.size(), child);
}

bool Group::insertChild(size_t index, Node* child)
{
	if (child == nullptr)
		return false;

	if (index >= m_children.size())
		m_children.push_back(child);
	else
		m_children.insert(m_children.begin() + index, child);

	child->addParent(this);

	//parent observes children
	child->addObserver(this);
	//notify observers of change
	Event event;
	event.init(Event::EVENT_NODE_ADDED,
		this, child);
	notifyObservers(event);

	return true;
}

bool Group::removeChildren(size_t pos, size_t num)
{
	if (pos < m_children.size() && num > 0)
	{
		size_t end = pos + num;
		if (end > m_children.size())
			end = m_children.size();

		for (size_t i = pos; i < end; ++i)
		{
			m_children[i]->removeParent(this);

			//parent observes children
			m_children[i]->removeObserver(this);
			//notify observers of change
			Event event;
			event.init(Event::EVENT_NODE_REMOVED,
				this, m_children[i].get());
			notifyObservers(event);
		}
		m_children.erase(m_children.begin() + pos, m_children.begin() + end);

		return true;
	}
	return false;
}

bool Group::replaceChild(Node* orig_child, Node* new_child)
{
	if (orig_child == nullptr ||
		new_child == nullptr)
		return false;

	size_t pos = getChildIndex(orig_child);
	if (pos < m_children.size())
		return setChild(pos, new_child);
	return false;
}

bool Group::setChild(size_t i, Node* node)
{
	if (i < m_children.size() && node != nullptr)
	{
		Node* origNode = m_children[i].get();
		origNode->removeParent(this);
		//parent observes children
		origNode->removeObserver(this);
		//notify observers of change
		Event event;
		event.init(Event::EVENT_NODE_REMOVED,
			this, origNode);
		notifyObservers(event);

		m_children[i] = node;
		node->addParent(this);
		//parent observes children
		node->addObserver(this);
		//notify observers of change
		event.init(Event::EVENT_NODE_ADDED,
			this, node);
		notifyObservers(event);

		return true;
	}
	return false;
}

void Group::accept(NodeVisitor& nv)
{
	if (nv.validNodeMask(*this))
	{
		nv.pushOntoNodePath(this);
		nv.apply(*this);
		nv.popFromNodePath();
	}
}

