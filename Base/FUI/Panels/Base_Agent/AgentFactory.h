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
#ifndef _AGENTFACTORY_H_
#define _AGENTFACTORY_H_

#include <Flobject/ObjectFactory.h>
#include <Fui/InterfaceAgent.h>
#include <wx/window.h>

namespace FUI
{
	class ListModel;
	class TreeModel;
	class VolumePropAgent;
	class RenderCanvasAgent;
	class OutAdjustAgent;
	class ClipPlaneAgent;
	class MeshPropAgent;
	class ColocalAgent;
	class AgentFactory : public FL::ObjectFactory
	{
	public:
		AgentFactory();

		virtual bool isSameKindAs(const Object* obj) const
		{
			return dynamic_cast<const AgentFactory*>(obj) != NULL;
		}

		virtual const char* className() const { return "AgentFactory"; }

		virtual void createDefault() {}//no default agent

		virtual InterfaceAgent* getDefault() { return 0; }//no default agent

		virtual InterfaceAgent* build(InterfaceAgent* agent = 0) { return 0; }

		virtual InterfaceAgent* clone(InterfaceAgent*) { return 0; }

		virtual InterfaceAgent* clone(const unsigned int) { return 0; }

		inline virtual InterfaceAgent* get(size_t i)
		{
			return dynamic_cast<InterfaceAgent*>(ObjectFactory::get(i));
		}

		inline virtual const InterfaceAgent* get(size_t i) const
		{
			return dynamic_cast<InterfaceAgent*>(const_cast<Object*>(ObjectFactory::get(i)));
		}

		inline virtual InterfaceAgent* find(const unsigned int id)
		{
			return dynamic_cast<InterfaceAgent*>(ObjectFactory::find(id));
		}

		inline virtual InterfaceAgent* findFirst(const std::string &name)
		{
			return dynamic_cast<InterfaceAgent*>(ObjectFactory::findFirst(name));
		}

		inline virtual InterfaceAgent* findLast(const std::string &name)
		{
			return dynamic_cast<InterfaceAgent*>(ObjectFactory::findLast(name));
		}

		//each agent type has a function
		ListModel* getOrAddListModel(const std::string &name, wxWindow &window);
		TreeModel* getOrAddTreeModel(const std::string &name, wxWindow &window);
		VolumePropAgent* getOrAddVolumePropAgent(const std::string &name, wxWindow &window);
		RenderCanvasAgent* getOrAddRenderCanvasAgent(const std::string &name, wxWindow &window);
		OutAdjustAgent* getOrAddOutAdjustAgent(const std::string &name, wxWindow &window);
		ClipPlaneAgent* getOrAddClipPlaneAgent(const std::string &name, wxWindow &window);
		MeshPropAgent* getOrAddMeshPropAgent(const std::string &name, wxWindow &window);
		ColocalAgent* getOrAddColocalAgent(const std::string &name, wxWindow &window);

	protected:
		virtual ~AgentFactory();
	};
}

#endif//_AGENTFACTORY_H_
