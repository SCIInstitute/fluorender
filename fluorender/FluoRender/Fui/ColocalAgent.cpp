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

#include <Fui/ColocalAgent.h>
#include <Fui/ColocalDlg.h>
#include <Compare/Compare.h>

using namespace FUI;

ColocalAgent::ColocalAgent(ColocalDlg &dlg) :
	InterfaceAgent(),
	dlg_(dlg)
{

}

void ColocalAgent::setObject(FL::Root* root)
{
	InterfaceAgent::setObject(root);
}

FL::Root* ColocalAgent::getObject()
{
	return dynamic_cast<FL::Root*>(InterfaceAgent::getObject());
}

void ColocalAgent::UpdateAllSettings()
{

}

void ColocalAgent::Run()
{
	FL::Root* root = getObject();
	if (!root)
		return;
	//get selections
	class SelRetriever : public FL::NodeVisitor
	{
	public:
		SelRetriever() : NodeVisitor()
		{
			setTraversalMode(FL::NodeVisitor::TRAVERSE_ALL_CHILDREN);
		}

		virtual void reset()
		{
			nodes_.clear();
		}

		virtual void apply(FL::Node& node)
		{
			if (std::string("VolumeData") == node.className())
			{
				bool selected = false;
				node.getValue("selected", selected);
				if (selected)
					nodes_.insert(&node);
			}
			traverse(node);
		}

		virtual void apply(FL::Group& group)
		{
			traverse(group);
		}

		FL::NodeSet &getResult()
		{
			return nodes_;
		}

	private:
		FL::NodeSet nodes_;
	};

	SelRetriever retriever;
	root->accept(retriever);
	FL::NodeSet nodes = retriever.getResult();

	if (nodes.size() < 2)
		return;

	for (auto it1 = nodes.begin();
		it1 != nodes.end(); ++it1)
	{
		for (auto it2 = std::next(it1);
			it2 != nodes.end(); ++it2)
		{
			FL::ChannelCompare compare(
				(*it1)->asVolumeData(),
				(*it2)->asVolumeData());
			compare.Compare(0.5);
		}
	}
}