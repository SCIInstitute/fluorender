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

#include <Node.hpp>
#include <NodeVisitor.hpp>

#include <algorithm>

using namespace fluo;

Node::Node() :
	Object(),
	m_node_mask(0xffffffff)
{
}

Node::Node(const Node& node, const CopyOp& copyop, bool copy_values) :
	Object(node, copyop, false),
	m_parents(),
	m_node_mask(node.m_node_mask)
{
	//copy
	if (copy_values)
		copyValues(node, copyop);
}

Node::~Node()
{
}

void Node::addParent(Node* node)
{
	m_parents.push_back(node);
}

void Node::removeParent(Node* node)
{
	auto pitr = std::find(m_parents.begin(), m_parents.end(), node);
	if (pitr != m_parents.end())
		m_parents.erase(pitr);
}

void Node::accept(NodeVisitor& nv)
{
	if (nv.validNodeMask(*this))
	{
		nv.pushOntoNodePath(this);
		nv.apply(*this);
		nv.popFromNodePath();
	}
}

void Node::ascend(NodeVisitor& nv)
{
	std::for_each(m_parents.begin(), m_parents.end(), NodeAcceptOp(nv));
}

//as observer
void Node::processNotification(Event& event)
{
	Object::handleEvent(event);
	//notify only if event is from child
	if (event.getNotifyFlags() & Event::NOTIFY_PARENT &&
		event.type != Event::EVENT_VALUE_CHANGING)
		notifyObservers(event);
}
