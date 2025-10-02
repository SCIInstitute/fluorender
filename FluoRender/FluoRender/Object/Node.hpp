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

#ifndef NODE_HPP
#define NODE_HPP

#include <CopyOp.hpp>
#include <Object.hpp>


#include <vector>
#include <set>


namespace fluo
{
	class Group;
	class Layer;
	class VolumeData;
	class MeshData;
	class VolumeGroup;
	class MeshGroup;
	class AnnotData;
	class Node;
	class NodeVisitor;
	class View;
	class Root;

	typedef std::vector<Node*> ParentList;
    typedef std::vector<ref_ptr<Node>> NodeList;
	typedef std::vector<Node*> NodePath;
	typedef std::vector<NodePath> NodePathList;
	typedef std::set<Node*> NodeSet;
	typedef std::set<Node*>::iterator NodeSetIter;

    class Node : public Object
	{
	public:
		Node();
        Node(const Node&, const CopyOp& copyop = CopyOp::SHALLOW_COPY, bool copy_values = true);

        virtual Object* clone(const CopyOp& copyop) const
		{ return new Node(*this, copyop); }

        virtual bool isSameKindAs(const Object* obj) const
		{ return dynamic_cast<const Node*>(obj) != NULL; }

		virtual const char* className() const { return "Node"; }

		virtual unsigned int getPriority() const { return 100; }

		virtual Group* asGroup() { return 0; }
		virtual const Group* asGroup() const { return 0; }
		virtual Layer* asLayer() { return 0; }
		virtual const Layer* asLayer() const { return 0; }
		virtual VolumeData* asVolumeData() { return 0; }
		virtual const VolumeData* asVolumeData() const { return 0; }
		virtual MeshData* asMeshData() { return 0; }
		virtual const MeshData* asMeshData() const { return 0; }
		virtual VolumeGroup* asVolumeGroup() { return 0; }
		virtual const VolumeGroup* asVolumeGroup() const { return 0; }
        virtual MeshGroup* asMeshGroup() { return 0; }
		virtual const MeshGroup* asMeshGroup() const { return 0; }
		virtual AnnotData* asAnnotData() { return 0; }
		virtual const AnnotData* asAnnotData() const { return 0; }
		virtual View* asView() { return 0; }
		virtual const View* asView() const { return 0; }
		virtual Root* asRoot() { return 0; }
		virtual const Root* asRoot() const { return 0; }

		virtual void accept(NodeVisitor& nv);
		virtual void ascend(NodeVisitor& nv);
		virtual void traverse(NodeVisitor& nv, bool reverse=false) {}

		//as observer
        virtual void processNotification(Event& event);

		/* parents
		*/
		inline const ParentList& getParents() const { return m_parents; }

		inline ParentList getParents() { return m_parents; }

		inline const Node* getParent(unsigned int i) const { if (getNumParents()) return m_parents[i]; else return 0; }

		inline Node* getParent(unsigned int i) { if (getNumParents()) return m_parents[i]; else return 0; }

		inline unsigned int getNumParents() const { return static_cast<unsigned int>(m_parents.size()); }

		typedef unsigned int NodeMask;
		inline void setNodeMask(NodeMask nm) { m_node_mask = nm; }
		inline NodeMask getNodeMask() const { return m_node_mask; }

	protected:
		virtual ~Node();
		void addParent(Node* node);
		void removeParent(Node* node);

		ParentList m_parents;
		friend class Group;

		NodeMask m_node_mask;
	};
}

#endif//_NODE_H_
