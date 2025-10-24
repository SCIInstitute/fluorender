/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2025 Scientific Computing and Imaging Institute,
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
#include <RefreshScheduler.h>
#include <RenderCanvas.h>
#include <RenderView.h>
#include <Global.h>
#include <CurrentObjects.h>
#include <FramebufferStateTracker.h>
#include <LookingGlassRenderer.h>
#include <Debug.h>

RefreshScheduler::RefreshScheduler(RenderCanvas* canvas, std::shared_ptr<RenderView>& view)
	: canvas_(canvas), view_(view), draw_pending_(false) {
}

void RefreshScheduler::requestDraw(const DrawRequest& request)
{
	if (!draw_pending_) {
		draw_pending_ = true;
		last_request_ = request;
		canvas_->Refresh(false); // Schedule paint event
		DBGPRINT(L"requestDraw: %s\n", s2ws(last_request_.reason).c_str());
		//canvas_->Update();
	}
}

void RefreshScheduler::performDraw()
{
	draw_pending_ = false;

	if (auto view = view_.lock())
	{
		glbin_current.render_view_drawing = view;
		glbin_fb_state_tracker.sync();

		view->SetForceClear(last_request_.clearFramebuffer);
		view->SetInteractive(last_request_.interactive);
		if (last_request_.restartLoop)
			view->StartLoopUpdate();
		view->SetSortBricks();
		glbin_lg_renderer.SetUpdating(last_request_.lgChanged);
		bool bval = view->Draw();
		view->DrawDefault();

		if (canvas_)
		{
			canvas_->SwapBuffers();
			DBGPRINT(L"SwapBuffers: %s\n", s2ws(last_request_.reason).c_str());
		}
	}
}

void RefreshSchedulerManager::registerScheduler(std::shared_ptr<RefreshScheduler> scheduler)
{
	if (!scheduler)
		return;
	auto view = scheduler->getView();
	if (!view)
		return;
	int view_id = view->Id();
	schedulers_[view_id] = scheduler;
}

std::shared_ptr<RefreshScheduler> RefreshSchedulerManager::getScheduler(int view_id)
{
	auto it = schedulers_.find(view_id);
	if (it != schedulers_.end())
		return it->second;
	return nullptr;
}

void RefreshSchedulerManager::removeScheduler(int view_id)
{
	schedulers_.erase(view_id);
}

void RefreshSchedulerManager::requestDraw(const DrawRequest& request)
{
	if (request.view_ids.find(-1) != request.view_ids.end())
		return;

	bool update_all = request.view_ids.empty();

	for (const auto& [view_id, scheduler] : schedulers_)
	{
		auto view = scheduler->getView();
		if (view)
		{
			int id = view->Id();
			if (update_all || request.view_ids.find(id) != request.view_ids.end())
			{
				scheduler->requestDraw(request);
			}
		}
	}
}
