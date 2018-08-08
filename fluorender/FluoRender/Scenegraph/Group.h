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

#ifndef _GROUP_H_
#define _GROUP_H_

#include <Scenegraph/Node.h>

namespace FL
{
	typedef std::vector<pNode> NodeList;

	class Group : public Node
	{
	public:
		Group();
		Group(const Group& group, const CopyOp& copyop=CopyOp::SHALLOW_COPY);
		virtual ~Group();

		virtual bool addChild(pNode &child);
		virtual bool insertChild(size_t index, pNode &child);
		inline bool removeChild(pNode &child)
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
		virtual bool removeChildren(size_t pos, size_t num);
		virtual bool replaceChild(pNode &orig_child, pNode &new_child);
		inline size_t getNumChildren() const { return m_children.size(); }
		virtual bool setChild(size_t i, pNode &node);
		inline pNode getChild(size_t i) { return m_children[i]; }
		inline const pNode getChild(size_t i) const { return m_children[i]; }
		inline bool containsNode(const pNode &node) const
		{
			for (auto it = m_children.begin();
				it != m_children.end(); ++it)
			{
				if (*it == node)
					return true;
			}
			return false;
		}
		inline size_t getChildIndex(const pNode &node) const
		{
			for (size_t i = 0; i < m_children.size(); ++i)
			{
				if (m_children[i] == node)
					return i;
			}
			return m_children.size();
		}

	protected:
		NodeList m_children;
	};
}

#endif//_GROUP_H_