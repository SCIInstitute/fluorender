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
#include <RenderCanvasAgent.h>
#include <RenderCanvas.h>
#include <RenderView.h>
#include <Global.h>
#include <Names.h>
#include <FramebufferStateTracker.h>
#include <LookingGlassRenderer.h>
#include <Value.hpp>

RenderCanvasAgent::RenderCanvasAgent(
	RenderCanvas* canvas,
	const std::shared_ptr<RenderView>& view) :
	Agent(canvas),
	view_(view)
{
}

bool RenderCanvasAgent::Accept(
	const UpdateRequest& request) const
{
	return
		FOUND_VALUE(gstRotations) ||
		FOUND_VALUE(gstCamera) ||
		FOUND_VALUE(gstRenderView) ||
		FOUND_VALUE(gstInteractive) ||
		FOUND_VALUE(gstBrushSize) ||
		FOUND_VALUE(gstVolumeProps);
}

RenderCanvasAgent*
RenderCanvasAgent::GetRenderSender(
	const UpdateRequest& request) const
{
	return dynamic_cast<RenderCanvasAgent*>(
		request.sender);
}

void RenderCanvasAgent::SyncRotations(
	RenderCanvasAgent* sender)
{
	if (!sender)
		return;

	auto src_view = sender->GetView();
	auto dst_view = GetView();

	if (!src_view || !dst_view)
		return;

	if (src_view == dst_view)
		return;

	dst_view->SetRotations(
		src_view->GetRotations(),
		true);
}

void RenderCanvasAgent::SyncCamera(
	RenderCanvasAgent* sender)
{
	if (!sender)
		return;

	auto src_view = sender->GetView();
	auto dst_view = GetView();

	if (!src_view || !dst_view)
		return;

	if (src_view == dst_view)
		return;

	dst_view->CopyCamera(*src_view);
}

void RenderCanvasAgent::Update(
	const UpdateRequest& request)
{
	auto view = view_.lock();
	if (!view)
		return;

	auto sender = GetRenderSender(request);

	// linked rotation
	if (glbin_linked_rot &&
		FOUND_VALUE(gstRotations))
	{
		SyncRotations(sender);
	}

	// future camera sync
	if (FOUND_VALUE(gstCamera))
	{
		SyncCamera(sender);
	}

	RequestDraw();
}

void RenderCanvasAgent::RequestDraw()
{
	if (draw_pending_)
		return;

	draw_pending_ = true;

	if (auto canvas = GetCanvas())
	{
		canvas->Refresh(false);

		if (glbin_linked_rot)
			canvas->Update();
	}
}

void RenderCanvasAgent::PerformDraw()
{
	draw_pending_ = false;

	auto view = view_.lock();
	if (!view)
		return;

	glbin_current.render_view_drawing = view;

	glbin_fb_state_tracker.sync();

	glbin_lg_renderer.SetUpdating(
		view->GetLgChanged());

	bool success = view->Draw();

	view->DrawDefault();

	if (canvas_ && success)
		canvas_->SwapBuffers();
}
