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
#include <RenderScheduler.h>
#include <RenderCanvas.h>
#include <RenderView.h>
#include <Global.h>
#include <CurrentObjects.h>
#include <FramebufferStateTracker.h>
#include <Debug.h>

RenderScheduler::RenderScheduler(RenderCanvas* canvas, std::shared_ptr<RenderView>& view)
	: canvas_(canvas), view_(view), draw_pending_(false) {
}

void RenderScheduler::requestDraw(const std::string& reason)
{
	if (!draw_pending_) {
		draw_pending_ = true;
		last_reason_ = reason;
		canvas_->Refresh(false); // Schedule paint event
	}
}

void RenderScheduler::performDraw()
{
	draw_pending_ = false;

	if (auto view = view_.lock())
	{
		glbin_current.render_view_drawing = view;
		glbin_fb_state_tracker.sync();

		bool bval = view->Draw();
		view->DrawDefault();

		if (canvas_)
		{
			canvas_->SwapBuffers();
			DBGPRINT(L"SwapBuffers();\n");
		}
	}
}

void RenderSchedulerManager::registerScheduler(std::shared_ptr<RenderScheduler> scheduler)
{
	schedulers_.push_back(scheduler);
}

std::shared_ptr<RenderScheduler> RenderSchedulerManager::getScheduler(RenderCanvas* canvas)
{
	for (auto& sched : schedulers_) {
		if (sched->getCanvas() == canvas)
			return sched;
	}
	return nullptr;
}

void RenderSchedulerManager::requestGlobalDraw(const std::string& reason)
{
	for (auto& sched : schedulers_) {
		sched->requestDraw(reason);
	}
}

void RenderSchedulerManager::removeScheduler(RenderCanvas* canvas)
{
	schedulars_.erase(
		std::remove_if(schedulars_.begin(), schedulars_.end(),
			[canvas](const std::shared_ptr<RenderSchedular>& sched) {
				return sched->getCanvas() == canvas;
			}),
		schedulars_.end()
	);
}