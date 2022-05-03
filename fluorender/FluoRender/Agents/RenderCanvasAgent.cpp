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

#include <RenderCanvasAgent.hpp>
#include <RenderCanvas.h>
#include <Global.hpp>
#include <AgentFactory.hpp>

using namespace fluo;

RenderCanvasAgent::RenderCanvasAgent(RenderCanvas &canvas) :
	InterfaceAgent(),
	canvas_(canvas)
{

}

void RenderCanvasAgent::setObject(Renderview* view)
{
	InterfaceAgent::setObject(view);
}

Renderview* RenderCanvasAgent::getObject()
{
	return dynamic_cast<Renderview*>(InterfaceAgent::getObject());
}

void RenderCanvasAgent::UpdateFui(const ValueCollection &names)
{
	holdoffObserverNotification();

	setValue(gstForceClear, true);
	setValue(gstInteractive, false);
	setValue(gstRefresh, true);
	Renderview* view = getObject();
	if (!view) return;
	view->Update(41);
	bool bval = false;
	getValue(gstRefreshErase, bval);
	canvas_.Refresh(bval);
	setValue(gstRefreshErase, false);

	resumeObserverNotification();
}

void RenderCanvasAgent::OnBoundsChanged(Event& event)
{
	Renderview* view = getObject();
	if (!view) return;
	view->InitView(Renderview::INIT_BOUNDS | Renderview::INIT_CENTER);
}

void RenderCanvasAgent::OnSceneChanged(Event& event)
{
	Renderview* view = getObject();
	if (!view) return;
	view->PopMeshList();
	view->PopVolumeList();
}

void RenderCanvasAgent::OnFocusChanged(Event& event)
{
	bool focus;
	getValue(gstFocus, focus);
	if (focus)
	{
		canvas_.SetFocus();
		setValue(gstFocus, false);
	}
}

void RenderCanvasAgent::OnSetGl(Event& event)
{
	for (size_t i = 0; i < glbin_agtf->getNum(); ++i)
	{
		InterfaceAgent* agent = glbin_agtf->get(i);
		if (agent->asRenderCanvasAgent() && agent != this)
			agent->changeValue(gstSetGl, false);
	}
}