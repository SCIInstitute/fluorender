/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2018 Scientific Computing and Imaging Institute,
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

#ifndef _SEARCHVISITOR_H_
#define _SEARCHVISITOR_H_

#include <Scenegraph/NodeVisitor.h>
#include <Scenegraph/Group.h>
#include <Scenegraph/VolumeData.h>
#include <Scenegraph/VolumeGroup.h>
#include <Scenegraph/MeshData.h>
#include <Scenegraph/MeshGroup.h>
#include <Scenegraph/Annotations.h>
#include <iostream>
#include <string>
#include <vector>

namespace flrd
{
	class SearchVisitor : public flrd::NodeVisitor
	{
	public:

		SearchVisitor()
		{
			setTraversalMode(flrd::NodeVisitor::TRAVERSE_ALL_CHILDREN);
		}

		virtual void apply(flrd::Node& node)
		{
			traverse(node);
		}

		virtual void apply(flrd::Group& group)
		{
			traverse(group);
		}

		virtual void reset()
		{
			results_.clear();
			names_.clear();
			ids_.clear();
			class_names_.clear();
		}

		ObjectList* getResult() { return &results_; }
		size_t getResultNum() { return results_.size(); }

		Node* getNode()
		{
			if (!results_.empty())
				return dynamic_cast<Node*>(results_[0]);
			else
				return 0;
		}

		Group* getGroup()
		{
			if (!results_.empty())
				return dynamic_cast<Group*>(results_[0]);
			else
				return 0;
		}

		VolumeData* getVolumeData()
		{
			if (!results_.empty())
				return dynamic_cast<VolumeData*>(results_[0]);
			else
				return 0;
		}

		VolumeGroup* getVolumeGroup()
		{
			if (!results_.empty())
				return dynamic_cast<VolumeGroup*>(results_[0]);
			else
				return 0;
		}

		MeshData* getMeshData()
		{
			if (!results_.empty())
				return dynamic_cast<MeshData*>(results_[0]);
			else
				return 0;
		}

		MeshGroup* getMeshGroup()
		{
			if (!results_.empty())
				return dynamic_cast<MeshGroup*>(results_[0]);
			else
				return 0;
		}

		Annotations* getAnnotations()
		{
			if (!results_.empty())
				return dynamic_cast<Annotations*>(results_[0]);
			else
				return 0;
		}

		void matchName(const std::string &name)
		{ names_.push_back(name); }
		void matchNames(const std::vector<std::string> &names)
		{ names_.insert(names_.end(), names.begin(), names.end()); }

		void matchId(const unsigned int id)
		{ ids_.push_back(id); }
		void matchIds(std::vector<unsigned int> &ids)
		{ ids_.insert(ids_.end(), ids.begin(), ids.end()); }



	protected:
		ObjectList results_;
		std::vector<std::string> names_;
		std::vector<unsigned int> ids_;
		std::vector<std::string> class_names_;
	};
}
#endif//_SEARCHVISITOR_H_