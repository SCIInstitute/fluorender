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
#include <Scenegraph/VolumeData.h>
#include <fstream>

using namespace FUI;

ColocalAgent::ColocalAgent(ColocalDlg &dlg) :
	InterfaceAgent(),
	dlg_(dlg)
{

}

void ColocalAgent::setObject(FL::Root* root)
{
	InterfaceAgent::setObject(root);
	addValue("output file", std::wstring(L"colocalization_result.txt"));
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

	size_t num = nodes.size();
	if (num < 2)
		return;

	//result
	std::vector<std::vector<double>> rm;//result matrix
	rm.reserve(num);
	for (size_t i = 0; i < num; ++i)
	{
		rm.push_back(std::vector<double>());
		rm[i].reserve(num);
		for (size_t j = 0; j < num; ++j)
			rm[i].push_back(0);
	}

	size_t x = 0, y = 0;
	for (auto it1 = nodes.begin();
		it1 != nodes.end(); ++it1)
	{
		y = x + 1;
		for (auto it2 = std::next(it1);
			it2 != nodes.end(); ++it2)
		{
			FL::VolumeData* vd1 = (*it1)->asVolumeData();
			FL::VolumeData* vd2 = (*it2)->asVolumeData();
			if (!vd1 || !vd2)
				continue;

			FL::ChannelCompare compare(vd1, vd2);
			//get threshold values
			double th1, th2;
			vd1->getValue("low threshold", th1);
			vd2->getValue("low threshold", th2);
			compare.Compare(th1, th2);
			rm[x][y] = compare.Result();
			rm[y][x] = compare.Result();
			y++;
		}
		x++;
	}

	//print
	std::wstring filename;
	getValue("output file", filename);
	std::ofstream outfile;
	outfile.open(filename, std::ofstream::out);
	for (size_t i = 0; i < num; ++i)
	{
		for (size_t j = 0; j < num; ++j)
		{
			outfile << rm[i][j];
			if (j < num - 1)
				outfile << "\t";
		}
		outfile << "\n";
	}
	outfile.close();
}