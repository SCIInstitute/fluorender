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

#ifndef GROUP_HPP
#define GROUP_HPP

#include <Node.hpp>

namespace fluo
{
	class Group : public Node
	{
	public:
		Group();
		Group(const Group& group, const CopyOp& copyop=CopyOp::SHALLOW_COPY, bool copy_values = true);

		virtual Object* clone(const CopyOp& copyop) const
		{
			return new Group(*this, copyop);
		}

		virtual bool isSameKindAs(const Object* obj) const
		{
			return dynamic_cast<const Group*>(obj) != NULL;
		}

		virtual const char* className() const { return "Group"; }

		virtual Group* asGroup() { return this; }
		virtual const Group* asGroup() const { return this; }

		virtual void traverse(NodeVisitor& nv, bool reverse=false);

		/* children
		*/
		virtual bool addChild(Node* child);
		virtual bool insertChild(size_t index, Node* child);
		inline bool removeChild(Node* child)
		{
			size_t pos = getChildIndex(child);
			if (pos < m_children.size())
				return removeChildren(pos, 1);
			else
				return false;
		}
		inline bool removeChild(size_t pos, size_t num = 1)
		{
			if (pos < m_children.size())
				return removeChildren(pos, num);
			else
				return false;
		}
		inline bool removeAllChildren() { return removeChildren(0, getNumChildren()); }
		virtual bool removeChildren(size_t pos, size_t num);
		virtual bool replaceChild(Node* orig_child, Node* new_child);
		inline size_t getNumChildren() const { return m_children.size(); }
		virtual bool setChild(size_t i, Node* node);
		inline Node* getChild(size_t i) { return m_children[i].get(); }
		inline const Node* getChild(size_t i) const { return m_children[i].get(); }
		inline Node* getChild(const std::string& child_name)
		{
			std::string name;
			for (auto it = m_children.begin();
				it != m_children.end(); ++it)
			{
				name = (*it)->getName();
				if (name == child_name)
					return it->get();
			}
			return nullptr;
		}
		inline bool containsNode(const Node* node) const
		{
			for (auto it = m_children.begin();
				it != m_children.end(); ++it)
			{
				if (*it == node)
					return true;
			}
			return false;
		}
		inline size_t getChildIndex(const Node* node) const
		{
			for (size_t i = 0; i < m_children.size(); ++i)
			{
				if (m_children[i] == node)
					return i;
			}
			return m_children.size();
		}
		virtual Node* getOrAddNode(const std::string& child_name)
		{
			Node* node = getChild(child_name);
			if (node)
				return node;
			//no found
			node = new Node();
			node->setName(child_name);
			addChild(node);
			return node;
		}
		virtual Group* getOrAddGroup(const std::string& child_name)
		{
			std::string name;
			for (auto it = m_children.begin();
				it != m_children.end(); ++it)
			{
				name = (*it)->getName();
				if (name == child_name)
				{
					Group* group = dynamic_cast<Group*>(it->get());
					if (group)
						return group;
				}
			}
			//no found
			Group* group = new Group();
			group->setName(child_name);
			addChild(group);
			return group;
		}

		virtual void accept(NodeVisitor& nv);

	protected:
		virtual ~Group();
		NodeList m_children;
	};
}

#endif//_GROUP_H_
