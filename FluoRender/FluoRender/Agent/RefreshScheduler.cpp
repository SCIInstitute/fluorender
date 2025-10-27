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
#include <Root.h>
#include <DataManager.h>
#include <FramebufferStateTracker.h>
#include <LookingGlassRenderer.h>
#include <Debug.h>

RefreshScheduler::RefreshScheduler(RenderCanvas* canvas, std::shared_ptr<RenderView>& view)
	: canvas_(canvas), view_(view), draw_pending_(false) {
}

void RefreshScheduler::requestDraw(const DrawRequest& request)
{
	if (!draw_pending_)
	{
		draw_pending_ = true;
		last_request_ = request;
		canvas_->Refresh(false); // Schedule paint event
		//auto view = view_.lock();
		//assert(view);
		//DBGPRINT(L"requestDraw: %s, id: %d\n", s2ws(last_request_.reason).c_str(), view->Id());
		if (glbin_linked_rot)
			canvas_->Update();
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
			//DBGPRINT(L"SwapBuffers: %s, id: %d\n", s2ws(last_request_.reason).c_str(), view->Id());
			//DBGPRINT(L"SwapBuffers: %s\n", s2ws(last_request_.reason).c_str());
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
	auto updated_ids = request.view_ids;
	int origin_id = request.view_origin_id;
	if (updated_ids.find(-1) != updated_ids.end())
		return;

	bool update_all = updated_ids.empty();

	//check linked rotation
	if (glbin_linked_rot && origin_id > 0)
	{
		Root* root = glbin_data_manager.GetRoot();
		assert(root);
		auto origin_view = root->GetViewById(origin_id);
		if (origin_view)
		{
			fluo::Vector rot = origin_view->GetRotations();
			for (int i = 0; i < root->GetViewNum(); i++)
			{
				auto viewi = root->GetView(i);
				if (viewi && origin_view != viewi)
				{
					viewi->SetRotations(rot, true);
					updated_ids.insert(viewi->Id());
				}
			}
		}
	}

	for (const auto& [view_id, scheduler] : schedulers_)
	{
		auto view = scheduler->getView();
		if (view)
		{
			if (update_all || updated_ids.find(view_id) != updated_ids.end())
			{
				//DBGPRINT(L"requestDraw: %s, id: %d\n", s2ws(request.reason).c_str(), view_id);
				scheduler->requestDraw(request);
			}
		}
	}
}
