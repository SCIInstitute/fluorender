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
#ifndef _RENDERCANVASAGENT_H_
#define _RENDERCANVASAGENT_H_

#include <InterfaceAgent.hpp>
#include <Renderview.hpp>

class RenderCanvas;
namespace fluo
{
	class RenderCanvasAgent : public InterfaceAgent
	{
	public:
		RenderCanvasAgent(RenderCanvas &canvas);

		virtual bool isSameKindAs(const Object* obj) const
		{
			return dynamic_cast<const RenderCanvasAgent*>(obj) != NULL;
		}

		virtual const char* className() const { return "RenderCanvasAgent"; }

		virtual void setObject(Renderview* view);
		virtual Renderview* getObject();

		virtual void UpdateFui(const ValueCollection &names = {});

		virtual RenderCanvasAgent* asRenderCanvasAgent() { return this; }
		virtual const RenderCanvasAgent* asRenderCanvasAgent() const { return this; }

		friend class AgentFactory;

	protected:
		RenderCanvas &canvas_;

		virtual void setupInputs();

		void OnBoundsChanged(Event& event);
		void OnSceneChanged(Event& event);
		void OnFocusChanged(Event& event);
		void OnSetGl(Event& event);
	};
}
#endif//_RENDERCANVASAGENT_H_