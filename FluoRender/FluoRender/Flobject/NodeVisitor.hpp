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

#ifndef NODEVISITOR_HPP
#define NODEVISITOR_HPP

#include <Node.hpp>

#include <algorithm>

namespace fluo
{
	const unsigned int UNINITIALIZED_FRAME_NUMBER = 0xffffffff;

	class NodeVisitor : public virtual Referenced
	{
	public:

		enum TraversalMode
		{
			TRAVERSE_NONE,
			TRAVERSE_PARENTS,
			TRAVERSE_CHILDREN,
			TRAVERSE_CHILDREN_REV,
		};

		enum VisitorType
		{
			NODE_VISITOR = 0,
			UPDATE_VISITOR,
		};

		NodeVisitor(TraversalMode tm = TRAVERSE_NONE);
		NodeVisitor(VisitorType type, TraversalMode tm = TRAVERSE_NONE);

		virtual ~NodeVisitor();

		virtual const char* className() const { return "NodeVisitor"; }

		virtual void reset() {}

		inline void setVisitorType(VisitorType type) { m_visitor_type = type; }
		inline VisitorType getVisitorType() const { return m_visitor_type; }

		inline void setTraversalNumber(unsigned int fn) { m_traversal_number = fn; }
		inline unsigned int getTraversalNumber() const { return m_traversal_number; }

		inline void setTraversalMask(Node::NodeMask mask) { m_traversal_mask = mask; }
		inline Node::NodeMask getTraversalMask() const { return m_traversal_mask; }
		inline void setNodeMaskOverride(Node::NodeMask mask) { m_node_mask_override = mask; }
		inline Node::NodeMask getNodeMaskOverride() const { return m_node_mask_override; }
		inline bool validNodeMask(const Node& node) const
		{ return (getTraversalMask() & (getNodeMaskOverride() | node.getNodeMask())) !=0; }

		inline void setTraversalMode(TraversalMode mode) { m_traversal_mode = mode; }
		inline TraversalMode getTraversalMode() const { return m_traversal_mode; }

		inline void setUserData(Referenced* obj) { m_user_data = obj; }
		inline Referenced* getUserData() { return m_user_data.get(); }
		inline const Referenced* getUserData() const { return m_user_data.get(); }

		inline void traverse(Node& node)
		{
			//break loop if present
			NodeSet node_set;
			std::pair<NodeSetIter, bool> result;
			for (auto it = m_node_path.begin();
				it != m_node_path.end(); ++it)
			{
				result = node_set.insert(*it);
				if (!result.second)
					return;
			}
			if (m_traversal_mode == TRAVERSE_PARENTS) node.ascend(*this);
			else if (m_traversal_mode != TRAVERSE_NONE)
			{
				if (m_traversal_mode == TRAVERSE_CHILDREN)
					node.traverse(*this);
				else if (m_traversal_mode == TRAVERSE_CHILDREN_REV)
					node.traverse(*this, true);
			}
		}
		inline void pushOntoNodePath(Node* node)
		{
			if (m_traversal_mode != TRAVERSE_PARENTS)
				m_node_path.push_back(node);
			else
				m_node_path.insert(m_node_path.begin(), node);
		}
		inline void popFromNodePath()
		{
			if (m_traversal_mode != TRAVERSE_PARENTS)
				m_node_path.pop_back();
			else
				m_node_path.erase(m_node_path.begin());
		}
		NodePath& getNodePath() { return m_node_path; }
		const NodePath& getNodePath() const { return m_node_path; }

		virtual void apply(Node& node);
		virtual void apply(Group& node);

	protected:

		VisitorType m_visitor_type;
		unsigned int m_traversal_number;
		TraversalMode m_traversal_mode;

		Node::NodeMask m_traversal_mask;
		Node::NodeMask m_node_mask_override;

		NodePath m_node_path;

		ref_ptr<Referenced> m_user_data;
	};

	class NodeAcceptOp
	{
	public:
		NodeAcceptOp(NodeVisitor& nv) : m_nv(nv) {}
		NodeAcceptOp(const NodeAcceptOp& naop) : m_nv(naop.m_nv) {}

		void operator () (Node* node) { node->accept(m_nv); }
		void operator () (ref_ptr<Node> node) { node->accept(m_nv); }

	protected:
		NodeAcceptOp& operator = (const NodeAcceptOp&) { return *this; }

		NodeVisitor& m_nv;
	};
}

#endif//_NODEVISITOR_H_
