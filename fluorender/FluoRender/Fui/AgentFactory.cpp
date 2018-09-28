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

#include <Fui/AgentFactory.h>
#include <Fui/ListModel.h>
#include <Fui/TreeModel.h>
#include <Fui/VolumePropAgent.h>
#include <Fui/VolumePropPanel.h>
#include <Fui/TreePanel.h>

using namespace FUI;

AgentFactory::AgentFactory()
{
	m_name = "agent factory";
}

AgentFactory::~AgentFactory()
{

}

InterfaceAgent* AgentFactory::getOrAddAgent(const std::string &name, wxPanel &panel)
{
	InterfaceAgent* result = findFirst(name);
	if (result)
		return result;

	//not found
	if (name == "ListModel")
	{
		ListModel* list_model = new ListModel();
		list_model->setName(name);
		result = list_model;
	}
	else if (name == "TreeModel")
	{
		TreeModel* tree_model =
			new TreeModel(static_cast<TreePanel&>(panel));
		tree_model->setName(name);
		result = tree_model;
	}
	else if (name == "VolumePropAgent")
	{
		VolumePropAgent* volume_prop_agent =
			new VolumePropAgent(static_cast<VolumePropPanel&>(panel));
		volume_prop_agent->setName(name);
		result = volume_prop_agent;
	}

	if (result)
	{
		objects_.push_front(result);
		notifyObserversNodeAdded(this, result);
	}

	return result;
}

