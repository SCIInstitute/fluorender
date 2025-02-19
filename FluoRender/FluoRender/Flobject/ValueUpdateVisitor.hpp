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

#ifndef VALUEUPDATEVISITOR_HPP
#define VALUEUPDATEVISITOR_HPP

#include <NodeVisitor.hpp>
#include <Group.hpp>

namespace fluo
{
	class ValueUpdateVisitor : public NodeVisitor
	{
	public:
		enum ValueUpdateVisitType
		{
			SYNC_VALUE = 0,
			SYNC_VALUES,
			SYNC_ALL_VALUES,
			UNSYNC_VALUE,
			UNSYNC_VALUES,
			UNSYNC_ALL_VALUES,
			PROP_VALUE,
			PROP_VALUES,
			PROP_ALL_VALUES,
		};

		ValueUpdateVisitor() : type_(SYNC_VALUE), object_(0)
        { setTraversalMode(NodeVisitor::TRAVERSE_CHILDREN); }
		ValueUpdateVisitor(ValueUpdateVisitType type) : type_(type), object_(0)
        { setTraversalMode(NodeVisitor::TRAVERSE_CHILDREN); }
		ValueUpdateVisitor(Object* obj) : type_(SYNC_VALUE), object_(obj)
        { setTraversalMode(NodeVisitor::TRAVERSE_CHILDREN); }
		ValueUpdateVisitor(ValueUpdateVisitType type, Object* obj) : type_(type), object_(obj)
        { setTraversalMode(NodeVisitor::TRAVERSE_CHILDREN); }

		virtual void reset()
		{
			type_ = SYNC_VALUE;
			object_ = 0;
            setTraversalMode(NodeVisitor::TRAVERSE_CHILDREN);
		}

		void setType(const ValueUpdateVisitType type)
		{ type_ = type; }
		void setObject(Object* obj)
		{ object_ = obj; }
		void setValueName(const std::string &name)
		{
			value_names_.clear();
			value_names_.insert(name);
		}
		void setValueNames(const ValueCollection &names)
		{
			value_names_ = names;
		}
		void addValueName(const std::string &name)
		{
			value_names_.insert(name);
		}
		void addValueNames(const ValueCollection &names)
		{
			value_names_.insert(names.begin(), names.end());
		}

        virtual void apply(Node& node)
		{
			switch (type_)
			{
			case SYNC_VALUE:
			case SYNC_VALUES:
			case SYNC_ALL_VALUES:
			case UNSYNC_VALUE:
			case UNSYNC_VALUES:
			case UNSYNC_ALL_VALUES:
				break;
			case PROP_VALUE:
			case PROP_VALUES:
				if (object_)
					object_->propValues(value_names_, &node);
				break;
			case PROP_ALL_VALUES:
				if (object_)
					object_->propAllValues(&node);
				break;
				break;
			}
			traverse(node);
		}

        virtual void apply(Group& group)
		{
			switch (type_)
			{
			case SYNC_VALUE:
			case SYNC_VALUES:
				syncValues(group);
				break;
			case SYNC_ALL_VALUES:
				syncAllValues(group);
				break;
			case UNSYNC_VALUE:
			case UNSYNC_VALUES:
				unsyncValues(group);
				break;
			case UNSYNC_ALL_VALUES:
				unsyncAllValues(group);
				break;
			case PROP_VALUE:
			case PROP_VALUES:
				if (object_)
					object_->propValues(value_names_, &group);
				break;
			case PROP_ALL_VALUES:
				if (object_)
					object_->propAllValues(&group);
				break;
				break;
			}
			traverse(group);
		}

	private:
		ValueUpdateVisitType type_;
		ValueCollection value_names_;
		Object* object_;

        void syncValues(Group& group)
		{
			for (size_t i = 0; i < group.getNumChildren(); ++i)
			{
				Node* child = group.getChild(i);
				if (child)
				{
					child->syncValues(value_names_, &group);
					group.syncValues(value_names_, child);
				}
			}
		}

        void syncAllValues(Group& group)
		{
			for (size_t i = 0; i < group.getNumChildren(); ++i)
			{
				Node* child = group.getChild(i);
				if (child)
				{
					child->syncAllValues(&group);
					group.syncAllValues(child);
				}
			}
		}

        void unsyncValues(Group& group)
		{
			for (size_t i = 0; i < group.getNumChildren(); ++i)
			{
				Node* child = group.getChild(i);
				if (child)
				{
					child->unsyncValues(value_names_, &group);
					group.unsyncValues(value_names_, child);
				}
			}
		}

        void unsyncAllValues(Group& group)
		{
			for (size_t i = 0; i < group.getNumChildren(); ++i)
			{
				Node* child = group.getChild(i);
				if (child)
				{
					child->unsyncAllValues(&group);
					group.unsyncAllValues(child);
				}
			}
		}
	};
}
#endif//_VALUEUPDATEVISITOR_H_
