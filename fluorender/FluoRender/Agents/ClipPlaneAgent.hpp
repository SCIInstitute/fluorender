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
#ifndef _CLIPPLANEAGENT_H_
#define _CLIPPLANEAGENT_H_

#include <InterfaceAgent.hpp>
#include <Node.hpp>

class ClipPlanePanel;
namespace fluo
{
	class AgentFactory;
	class ClipPlaneAgent : public InterfaceAgent
	{
	public:
		ClipPlaneAgent(ClipPlanePanel &panel);

		virtual bool isSameKindAs(const Object* obj) const
		{
			return dynamic_cast<const ClipPlaneAgent*>(obj) != NULL;
		}

		virtual const char* className() const { return "ClipPlaneAgent"; }

		virtual void setObject(Node* vd);
		virtual Node* getObject();

		virtual void UpdateFui(const ValueCollection &names = {});

		virtual ClipPlaneAgent* asClipPlaneAgent() { return this; }
		virtual const ClipPlaneAgent* asClipPlaneAgent() const { return this; }

		void alignRenderViewRot();

		friend class AgentFactory;

	protected:
		ClipPlanePanel &panel_;

		//update functions
		void OnClipXChanged(Event& event);
		void OnClipYChanged(Event& event);
		void OnClipZChanged(Event& event);
		void OnClipDistXChanged(Event& event);
		void OnClipDistYChanged(Event& event);
		void OnClipDistZChanged(Event& event);
		void OnClipLinkXChanged(Event& event);
		void OnClipLinkYChanged(Event& event);
		void OnClipLinkZChanged(Event& event);
		void OnClipRotXChanged(Event& event);
		void OnClipRotYChanged(Event& event);
		void OnClipRotZChanged(Event& event);
	};
}

#endif//_CLIPPLANEAGENT_H_