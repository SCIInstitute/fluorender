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
#ifndef RefreshScheduler_h
#define RefreshScheduler_h

#include <memory>
#include <string>
#include <unordered_map>
#include <set>

class RenderCanvas;
class RenderView;

struct DrawRequest
{
	DrawRequest(const std::string& r = "Refresh request",
		const std::set<int>& ids = {},
		bool cf = true,
		bool lu = false,
		bool rl = true,
		bool ia = false,
		bool lc = true) :
		reason(r),
		view_ids(ids),
		clearFramebuffer(cf),
		loadUpdate(lu),
		restartLoop(rl),
		interactive(ia),
		lgChanged(lc)
	{}

	std::string reason;
	std::set<int> view_ids;//-1: none; empty: all
	int view_origin_id = 0;//prevent loop
	bool clearFramebuffer = true;//from m_retain_finalbuffer in renderview
	bool loadUpdate = false;//from m_load_update in renderview
	bool restartLoop = true;
	bool interactive = false;
	bool lgChanged = true;

	static DrawRequest LinkedView(
		const std::set<int>& ids,
		int self_id
	)
	{
		auto dr = DrawRequest("Linked view refresh",
			ids);
		dr.view_origin_id = self_id;
		return dr;
	}
};

class RefreshScheduler
{
public:
	RefreshScheduler(RenderCanvas* canvas, std::shared_ptr<RenderView>& view);

	void requestDraw(const DrawRequest& request); // Called by canvas, view
	void performDraw();                          // Called by canvas during paint event

	RenderCanvas* getCanvas() const { return canvas_; }
	std::shared_ptr<RenderView> getView() const { return view_.lock(); }

private:
	RenderCanvas* canvas_;
	std::weak_ptr<RenderView> view_;
	bool draw_pending_;
	DrawRequest last_request_;
};

class RefreshSchedulerManager
{
public:
	void registerScheduler(std::shared_ptr<RefreshScheduler> scheduler);
	std::shared_ptr<RefreshScheduler> getScheduler(int view_id);
	void removeScheduler(int view_id);

	void requestDraw(const DrawRequest& request);

private:
	std::unordered_map<int, std::shared_ptr<RefreshScheduler>> schedulers_;
};

#endif//RefreshScheduler_h
