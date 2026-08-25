/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2026 Scientific Computing and Imaging Institute,
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

#include <FluiBuilder.h>
#include <Global.h>
#include <CurrentObjects.h>
#include <DataManager.h>
#include <MainFrame.h>
#include <RenderCanvas.h>
#include <RenderCanvasAgent.h>
#include <RenderView.h>
#include <Root.h>
#include <Coordinator.h>

#include <AnnotatPropPanel.h>
#include <AnnotatPropPanelAgent.h>

#include <wx/glcanvas.h>

RenderCanvas* 
FluiBuilder::BuildRenderCanvas(
	MainFrame* frame,
	RenderViewPanel* parent,
	wxGLContext* sharedContext
)
{
	auto canvas =
		new RenderCanvas(frame, parent, sharedContext);
	auto view = std::make_shared<RenderView>();
	glbin_current.render_view_drawing = view;
	Root* root = glbin_data_manager.GetRoot();
	view->SetRenderViewPanel(parent);
#ifdef _WIN32
	view->SetHandle((void*)canvas->GetHWND());
#endif
	auto size = canvas->GetSize();
	view->SetSize(size.x, size.y);
	root->AddView(view);
	auto agent = std::make_unique<RenderCanvasAgent>(canvas, view);
	canvas->SetAgent(std::move(agent));
	glbin_coordinator.Register(agent.get());

	return canvas;
}

AnnotatPropPanel*
FluiBuilder::BuildAnnotatPropPanel(
	wxWindow* parent,
	const std::shared_ptr<AnnotData>& ann)
{
	auto panel =
		new AnnotatPropPanel(parent);

	auto agent =
		std::make_unique<AnnotatPropPanelAgent>(
			panel,
			ann);

	panel->SetAgent(
		std::move(agent));
	glbin_coordinator.Register(agent.get());

	return panel;
}

VolumePropPanel*
FluiBuilder::BuildVolumePanel(
	wxWindow* parent,
	const std::shared_ptr<VolumeData>& vd)
{
	auto panel =
		new VolumePropPanel(parent);

	auto agent =
		std::make_unique<VolumePropAgent>(
			panel,
			vd);

	panel->SetAgent(
		std::move(agent));

	return panel;
}

