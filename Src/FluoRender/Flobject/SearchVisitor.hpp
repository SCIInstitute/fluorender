/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2022 Scientific Computing and Imaging Institute,
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

#ifndef SEARCHVISITOR_HPP
#define SEARCHVISITOR_HPP

#include <NodeVisitor.hpp>
#include <Group.hpp>
#include <VolumeData.hpp>
#include <VolumeGroup.hpp>
#include <MeshData.hpp>
#include <MeshGroup.hpp>
#include <Annotations.hpp>
#include <Renderview.hpp>

#include <iostream>
#include <string>
#include <vector>

namespace fluo
{
    class SearchVisitor : public NodeVisitor
	{
	public:

		SearchVisitor()
		{
            setTraversalMode(NodeVisitor::TRAVERSE_CHILDREN);
		}

        virtual void apply(Node& node)
		{
			conditions(&node);
			traverse(node);
		}

        virtual void apply(Group& group)
		{
			conditions(&group);
			traverse(group);
		}

		virtual void reset()
		{
			results_.clear();
			names_.clear();
			ids_.clear();
			class_names_.clear();
			values_.clear();
		}

		ObjectList* getResult() { return &results_; }
		size_t getResultNum() { return results_.size(); }

		Node* getNode()
		{
			for (auto i : results_)
			{
				Node* node = dynamic_cast<Node*>(i);
				if (node) return node;
			}
			return nullptr;
		}

		Group* getGroup()
		{
			for (auto i : results_)
			{
				Group* group = dynamic_cast<Group*>(i);
				if (group) return group;
			}
			return nullptr;
		}

		VolumeData* getVolumeData()
		{
			for (auto i : results_)
			{
				VolumeData* vd = dynamic_cast<VolumeData*>(i);
				if (vd) return vd;
			}
			return nullptr;
		}

		VolumeGroup* getVolumeGroup()
		{
			for (auto i : results_)
			{
				VolumeGroup* group = dynamic_cast<VolumeGroup*>(i);
				if (group) return group;
			}
			return nullptr;
		}

		MeshData* getMeshData()
		{
			for (auto i : results_)
			{
				MeshData* md = dynamic_cast<MeshData*>(i);
				if (md) return md;
			}
			return nullptr;
		}

		MeshGroup* getMeshGroup()
		{
			for (auto i : results_)
			{
				MeshGroup* group = dynamic_cast<MeshGroup*>(i);
				if (group) return group;
			}
			return nullptr;
		}

		Annotations* getAnnotations()
		{
			for (auto i : results_)
			{
				Annotations* an = dynamic_cast<Annotations*>(i);
				if (an) return an;
			}
			return nullptr;
		}

		Renderview* getRenderview()
		{
			for (auto i : results_)
			{
				Renderview* view = dynamic_cast<Renderview*>(i);
				if (view) return view;
			}
			return nullptr;
		}

		void matchName(const std::string &name)
		{ names_.push_back(name); }
		void matchNames(const std::vector<std::string> &names)
		{ names_.insert(names_.end(), names.begin(), names.end()); }

		void matchId(const unsigned int id)
		{ ids_.push_back(id); }
		void matchIds(std::vector<unsigned int> &ids)
		{ ids_.insert(ids_.end(), ids.begin(), ids.end()); }

		void matchClassName(const std::string &class_name)
		{ class_names_.push_back(class_name); }
		void matchClassNames(const std::vector<std::string> &class_names)
		{ class_names_.insert(class_names_.end(), class_names.begin(), class_names.end()); }

		void matchValue(const ValueTuple& vt)
		{ values_.push_back(vt); }
		void matchValues(const std::vector<ValueTuple> & vts)
		{ values_.insert(values_.end(), vts.begin(), vts.end()); }

	protected:
		ObjectList results_;
		//and conditions
		std::vector<std::string> names_;
		std::vector<unsigned int> ids_;
		std::vector<std::string> class_names_;
		std::vector<ValueTuple> values_;

		void conditions(Object* obj)
		{
			bool do_names = false;
			bool names_matched = false;
			if (!names_.empty())
			{
				do_names = true;
				std::string name = obj->getName();
				if (std::find(names_.begin(), names_.end(), name) != names_.end())
					names_matched = true;
			}

			bool do_ids = false;
			bool ids_matched = false;
			if (!ids_.empty())
			{
				do_ids = true;
				unsigned int id = obj->getId();
				if (std::find(ids_.begin(), ids_.end(), id) != ids_.end())
					ids_matched = true;
			}

			bool do_class_names = false;
			bool class_names_matched = false;
			if (!class_names_.empty())
			{
				do_class_names = true;
				std::string class_name = obj->className();
				if (std::find(class_names_.begin(), class_names_.end(), class_name) != class_names_.end())
					class_names_matched = true;
			}

			bool do_values = false;
			bool values_matched = false;
			if (!values_.empty())
			{
				do_values = true;
				size_t count = 0;
				for (auto it = values_.begin();
					it != values_.end(); ++it)
				{
					ValueTuple vt = *it;
					if (obj->getValue(vt) && vt == *it)
						count++;
				}
				if (count == values_.size())
					values_matched = true;
			}

			//conditions may change
			if (!do_names) names_matched = true;
			if (!do_ids) ids_matched = true;
			if (!do_class_names) class_names_matched = true;
			if (!do_values) values_matched = true;
			if (names_matched &&
				ids_matched &&
				class_names_matched &&
				values_matched)
				results_.push_back(obj);
		}
	};
}
#endif//_SEARCHVISITOR_H_
