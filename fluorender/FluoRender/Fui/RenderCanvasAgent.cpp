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

#include <Fui/RenderCanvasAgent.h>
#include <VRenderGLView.h>

using namespace FUI;

RenderCanvasAgent::RenderCanvasAgent(VRenderGLView &gl_view) :
	InterfaceAgent(),
	gl_view_(gl_view)
{

}

void RenderCanvasAgent::objectChanging(int notify_level, void* ptr, void* orig_node, const std::string &exp)
{
	//before change
	if (notify_level & FL::Value::NotifyLevel::NOTIFY_AGENT)
	{
		InterfaceAgent::objectChanging(notify_level, ptr, orig_node, exp);
	}
}

void RenderCanvasAgent::objectChanged(int notify_level, void* ptr, void* orig_node, const std::string &exp)
{
	//set values in ui
	if (notify_level & FL::Value::NotifyLevel::NOTIFY_AGENT)
	{
		InterfaceAgent::objectChanged(notify_level, ptr, orig_node, exp);
		if (exp.find("bounds") != std::string::npos)
			gl_view_.InitView(INIT_BOUNDS | INIT_CENTER);
		gl_view_.RefreshGL(41);
	}
}

void RenderCanvasAgent::setObject(FL::RenderView* view)
{
	InterfaceAgent::setObject(view);
}

FL::RenderView* RenderCanvasAgent::getObject()
{
	return dynamic_cast<FL::RenderView*>(InterfaceAgent::getObject());
}

void RenderCanvasAgent::UpdateAllSettings()
{

}

VRenderGLView &RenderCanvasAgent::getView()
{
	return gl_view_;
}