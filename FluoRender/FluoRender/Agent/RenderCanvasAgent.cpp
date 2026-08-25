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
#include <CurrentObjects.h>
#include <FramebufferStateTracker.h>
#include <LookingGlassRenderer.h>
//#include <Debug.h>

RenderCanvasAgent::RenderCanvasAgent(RenderCanvas* canvas, std::shared_ptr<RenderView>& view)
	: canvas_(canvas), view_(view)
{
}

void RenderCanvasAgent::requestDraw(const DrawRequest& request)
{
	if (draw_pending_)
		return;

	draw_pending_ = true;
	last_request_ = request;

	if (canvas_)
	{
		canvas_->Refresh(false);

		if (glbin_linked_rot)
			canvas_->Update();
	}
}

void RenderCanvasAgent::performDraw()
{
	draw_pending_ = false;

	auto view = view_.lock();
	if (!view)
		return;

	glbin_current.render_view_drawing = view;

	glbin_fb_state_tracker.sync();

	view->SetForceClear(
		last_request_.clearFramebuffer);

	view->SetInteractive(
		last_request_.interactive);

	if (last_request_.restartLoop)
		view->StartLoopUpdate();

	view->SetSortBricks();

	glbin_lg_renderer.SetUpdating(
		last_request_.lgChanged);

	bool success = view->Draw();

	view->DrawDefault();

	if (canvas_ && success)
		canvas_->SwapBuffers();
}
