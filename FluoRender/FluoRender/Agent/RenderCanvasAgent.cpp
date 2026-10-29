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
#include <MainFrameAgent.h>
#include <RenderView.h>
#include <Global.h>
#include <Tokens.h>
#include <FramebufferStateTracker.h>
#include <LookingGlassRenderer.h>
#include <Value.hpp>
#include <CurrentObjects.h>
#include <MainSettings.h>
#include <GlobalStates.h>
#include <Root.h>
#include <DataManager.h>
#include <Coordinator.h>
#include <State.h>
#include <MovieMaker.h>

RenderCanvasAgent::RenderCanvasAgent(
	RenderCanvas* canvas,
	const std::shared_ptr<RenderView>& view) :
	Agent(canvas),
	m_view(view)
{
}

bool RenderCanvasAgent::Accept(
	const UpdateRequest& request) const
{
	return
		request.HasValue(gstRotations) ||
		request.HasValue(gstCamera) ||
		request.HasValue(gstRenderView);
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

	dst_view->CopyCamera(src_view);
}

void RenderCanvasAgent::Update(
	const UpdateRequest& request)
{
	auto view = m_view.lock();
	if (!view)
		return;

	auto sender = GetRenderSender(request);

	// linked rotation
	if (glbin_settings.m_linked_rot &&
		request.HasValue(gstRotations))
	{
		SyncRotations(sender);
	}

	// future camera sync
	if (request.HasValue(gstCamera))
	{
		SyncCamera(sender);
	}

	if (request.HasValue(gstInitViewAll))
		InitViewAll();
	else if (request.HasValue(gstInitViewBoundsCenter))
		InitViewBoundsCenter();

	if (request.HasValue(gstResize))
		Resize();

	if (request.HasValue(gstCanvasMove))
		Move();

	if (request.HasValue(gstCanvasIdle))
		Idle();

	RequestDraw();
}

void RenderCanvasAgent::ApplySettings()
{
	if (auto view = GetView())
		view->Init();
	NotifyDataToUI({ gstMaxTextureSize, gstDeviceTree });
}

void RenderCanvasAgent::PreShutdown()
{
	if (glbin_states.m_benchmark)
	{
		//unsigned long long msec =
		//	(unsigned long long)(glbin.getStopWatch(gstStopWatch)->total_time() * 1000.0);
		//unsigned long long count =
		//	glbin.getStopWatch(gstStopWatch)->count();
		//double fps = glbin.getStopWatch(gstStopWatch)->total_fps();
		//wxString string = wxString("FluoRender has finished benchmarking.\n") +
		//	wxString("Results:\n") +
		//	wxString("Render size: ") + wxString::Format("%d X %d\n", GetSize().x, GetSize().y) +
		//	wxString("Time: ") + wxString::Format("%llu msec\n", msec) +
		//	wxString("Frames: ") + wxString::Format("%llu\n", count) +
		//	wxString("FPS: ") + wxString::Format("%.2f", fps);
		//wxMessageDialog* diag = new wxMessageDialog(this, string, "Benchmark Results",
		//	wxOK | wxICON_INFORMATION);
		//diag->ShowModal();
	}

	if (auto view = GetView())
	{
		view->SetRenderViewPanel(0);
		Root* root = glbin_data_manager.GetRoot();
		if (root)
			root->DeleteView(view.get());
	}

}

void RenderCanvasAgent::RequestDraw()
{
	if (draw_pending_)
		return;

	draw_pending_ = true;

	if (auto canvas = GetCanvas())
	{
		canvas->Refresh(false);

		if (glbin_settings.m_linked_rot)
			canvas->Update();
	}
}

void RenderCanvasAgent::PerformDraw()
{
	draw_pending_ = false;

	auto view = GetView();
	if (!view)
		return;
	bool initialized = view->Init();

	glbin_current.render_view_drawing = view;

	glbin_fb_state_tracker.sync();

	//glbin_lg_renderer.SetUpdating(
	//	view->GetLgChanged());

	bool success = view->Draw();

	view->DrawDefault();

	auto canvas = GetCanvas();
	if (canvas && success)
		canvas->SwapBuffers();

	if (initialized)
		NotifyDataToUI({ gstMaxTextureSize, gstDeviceTree });
}

RenderCanvas* RenderCanvasAgent::GetCanvas() const
{
	return static_cast<RenderCanvas*>(GetOwner());
}

void RenderCanvasAgent::InitViewAll()
{
	auto view = m_view.lock();
	if (view)
		view->InitView(-1);
}

void RenderCanvasAgent::InitViewBoundsCenter()
{
	auto view = m_view.lock();
	if (view)
		view->InitView(INIT_BOUNDS | INIT_CENTER);
}

void RenderCanvasAgent::Resize()
{
	auto view = GetView();
	if (!view)
		return;
	auto canvas = GetCanvas();
	if (!canvas)
		return;

	double dpi_scale = canvas->GetDpiScale();

	// Tell the renderer about DPI so it can set GL viewport in physical pixels
	view->SetDpiFactor(dpi_scale);

	// Use the client area (logical units) for layout
	int cw, ch;
	canvas->GetClientSize(&cw, &ch);
	// Avoid SetSize() here—let sizers manage this control
	view->SetSize(cw, ch);           // logical width/height for UI logic
	view->SetClient(0, 0, cw, ch);   // origin is typically 0,0 for client

	RequestDraw();
	NotifyDataToUI({ gstScaleFactor });
}

void RenderCanvasAgent::Move()
{
	auto view = GetView();
	if (!view)
		return;
	auto canvas = GetCanvas();
	if (!canvas)
		return;

	wxRect rect = canvas->GetClientRect();
	view->SetClient(rect.x, rect.y, rect.width, rect.height);
}

void RenderCanvasAgent::Idle()
{
	auto view = GetView();
	if (!view)
		return;
	auto canvas = GetCanvas();
	if (!canvas)
		return;

	IdleState state;
	state.m_movie_maker_render_canvas =
		(glbin_moviemaker.GetView() == view);
	//mouse state
	wxPoint mps = wxGetMousePosition();
	state.m_mouse_over = wxFindWindowAtPoint(mps) == canvas &&
		!glbin_states.m_modal_shown;
	state.m_mouse_left = wxGetMouseState().LeftIsDown();
	//key state
	state.m_key_paint = wxGetKeyState(WXK_SHIFT);
	state.m_key_erase = wxGetKeyState(wxKeyCode('x'));
	state.m_key_diff = wxGetKeyState(wxKeyCode('z'));
	state.m_key_ctrl = wxGetKeyState(WXK_CONTROL);
	state.m_key_refresh = wxGetKeyState(WXK_F5);
	state.m_key_main_mode = wxGetKeyState(wxKeyCode('v'));
	state.m_key_mask_mode = wxGetKeyState(wxKeyCode('b'));
	state.m_key_left = wxGetKeyState(WXK_LEFT);
	state.m_key_right = wxGetKeyState(WXK_RIGHT);
	state.m_key_up = wxGetKeyState(WXK_UP);
	state.m_key_down = wxGetKeyState(WXK_DOWN);
	state.m_key_mov_forward = wxGetKeyState(wxKeyCode('d'));
	state.m_key_mov_backward = wxGetKeyState(wxKeyCode('a'));
	state.m_key_mov_play = wxGetKeyState(WXK_SPACE);
	state.m_key_clip_up = wxGetKeyState(wxKeyCode('s'));
	state.m_key_clip_down = wxGetKeyState(wxKeyCode('w'));
	state.m_key_cell_full = wxGetKeyState(wxKeyCode('f'));
	state.m_key_cell_link = wxGetKeyState(wxKeyCode('l'));
	state.m_key_cell_new_id = wxGetKeyState(wxKeyCode('n'));
	state.m_key_cell_clear = wxGetKeyState(wxKeyCode('c'));
	state.m_key_cell_include = wxGetKeyState(WXK_RETURN);
	state.m_key_cell_exclude = wxGetKeyState(wxKeyCode('\\'));
	state.m_key_save_mask = wxGetKeyState(wxKeyCode('m'));
	state.m_key_exit_fullscreen = wxGetKeyState(WXK_ESCAPE);
	state.m_key_fullscreen = wxGetKeyState(WXK_F11);
	state.m_key_brush_size_down = wxGetKeyState(wxKeyCode('['));
	state.m_key_brush_size_up = wxGetKeyState(wxKeyCode(']'));
	state.m_key_ruler_relax = wxGetKeyState(wxKeyCode('r'));

	m_prev_focus = canvas->FindFocus();

	view->ProcessIdle(state);

	request_more_ = state.m_request_more;

	//full screen
	if (state.m_fullscreen)
	{
		auto view_panel_agent = glbin_coordinator.FindRenderViewPanelAgent(view);
		if (view_panel_agent)
			view_panel_agent->UpdateUIToData({ gstFullScreen });
	}
	if (state.m_exit_fullscreen)
		canvas->ExitFullscreen();

	if (state.m_refresh)
	{
		UpdateUIToData(state.m_value_collection);
	}
	if (state.m_set_focus)
		canvas->SetFocus();
	if (glbin_states.m_benchmark)
	{
		std::string title = std::string(fluo::AppTitle) +
			" " + std::string(fluo::VersionString) +
			" Benchmarking... FPS = " +
			std::to_string(state.m_benchmark_fps);
		auto frame_agent = glbin_coordinator.FindAgent<MainFrameAgent>();
		if (frame_agent)
		{
			frame_agent->SetBenchmarkString(title);
			frame_agent->UpdateDataToUI({ gstBenchmarkTitle });
		}
	}

	if (state.m_set_cur_focus)
		canvas->SetFocus();

	if (state.m_set_previous_focus && m_prev_focus)
		m_prev_focus->SetFocus();
}