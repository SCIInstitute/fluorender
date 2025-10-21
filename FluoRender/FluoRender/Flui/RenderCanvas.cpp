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

#include <RenderCanvas.h>
#include <Global.h>
#include <Names.h>
#include <MainFrame.h>
#include <RenderViewPanel.h>
#include <VolumeSelector.h>
#include <RenderView.h>
#include <StopWatch.hpp>
#include <State.h>
#include <MovieMaker.h>
#include <GlobalStates.h>
#include <MainSettings.h>
#include <CurrentObjects.h>
#include <Root.h>
#include <DataManager.h>
#include <LookingGlassRenderer.h>
#include <Debug.h>
#ifdef _WIN32
//wacom support
#include <wx/msw/private.h>
#include <MSGPACK.h>
#define PACKETDATA	(/*PK_X | PK_Y | */PK_BUTTONS |\
		PK_NORMAL_PRESSURE | PK_TANGENT_PRESSURE)
#define PACKETMODE	PK_BUTTONS
#include <PKTDEF.h>
#include <WacomUtils.h>
#endif

#ifdef _WIN32
HCTX RenderCanvas::m_hTab = 0;
LOGCONTEXTA RenderCanvas::m_lc;
#endif

RenderCanvas::RenderCanvas(MainFrame* frame,
	RenderViewPanel* parent,
	const wxGLAttributes& attriblist,
	wxGLContext* sharedContext,
	const wxPoint& pos,
	const wxSize& size,
	long style) :
	wxGLCanvas(parent, attriblist, wxID_ANY, pos, size, style),
	//set gl
	m_set_gl(false),
	m_frame(frame),
	m_renderview_panel(parent),
	//previous focus
	m_prev_focus(0),
	//timer for fullscreen
	m_exit_fscreen_trigger(this, 0),
	m_enable_touch(false),
	m_ptr_id1(-1),
	m_ptr_id2(-1),
	m_full_screen(false),
	m_focused_slider(0)
{
	m_glRC = sharedContext;
	m_sharedRC = m_glRC ? true : false;

	auto view = std::make_shared<RenderView>();
	glbin_current.render_view_drawing = view;
	Root* root = glbin_data_manager.GetRoot();
	view->SetRenderViewPanel(m_renderview_panel);
	view->SetRenderCanvas(this);
#ifdef _WIN32
	view->SetHandle((void*)GetHWND());
#endif
	view->SetSize(size.x, size.y);
	root->AddView(view);
	m_render_view = view;

#ifdef _WIN32
	//tablet initialization
	if (glbin_vol_selector.GetBrushUsePres())
	{
		if (!m_hTab && LoadWintab() &&
			gpWTInfoA(0, 0, NULL))
		{
			m_hTab = TabletInit((HWND)GetHWND(), (HINSTANCE)::wxGetInstance());
		}
		else
			glbin_vol_selector.SetBrushUsePres(false);
	}

	//check touch
	HMODULE user32 = LoadLibrary(L"user32");
	GetPI = reinterpret_cast<decltype(GetPointerInfo)*>
		(GetProcAddress(user32, "GetPointerInfo"));
	if (GetPI != NULL)
		m_enable_touch = true;
	else
		m_enable_touch = false;

#endif

	//events
	Bind(wxEVT_PAINT, &RenderCanvas::OnDraw, this);
	Bind(wxEVT_SIZE, &RenderCanvas::OnResize, this);
	Bind(wxEVT_MOVE, &RenderCanvas::OnMove, this);
	Bind(wxEVT_LEFT_DOWN, &RenderCanvas::OnMouse, this);
	Bind(wxEVT_LEFT_UP, &RenderCanvas::OnMouse, this);
	Bind(wxEVT_MIDDLE_DOWN, &RenderCanvas::OnMouse, this);
	Bind(wxEVT_MIDDLE_UP, &RenderCanvas::OnMouse, this);
	Bind(wxEVT_RIGHT_DOWN, &RenderCanvas::OnMouse, this);
	Bind(wxEVT_RIGHT_UP, &RenderCanvas::OnMouse, this);
	Bind(wxEVT_MOTION, &RenderCanvas::OnMouse, this);
	Bind(wxEVT_MOUSEWHEEL, &RenderCanvas::OnMouse, this);
	Bind(wxEVT_IDLE, &RenderCanvas::OnIdle, this);
	Bind(wxEVT_CLOSE_WINDOW, &RenderCanvas::OnClose, this);
	Bind(wxEVT_TIMER, &RenderCanvas::OnQuitFscreen, this);
}

RenderCanvas::~RenderCanvas()
{
	if (glbin_states.m_benchmark)
	{
		unsigned long long msec =
			(unsigned long long)(glbin.getStopWatch(gstStopWatch)->total_time() * 1000.0);
		unsigned long long count =
			glbin.getStopWatch(gstStopWatch)->count();
		double fps = glbin.getStopWatch(gstStopWatch)->total_fps();
		wxString string = wxString("FluoRender has finished benchmarking.\n") +
			wxString("Results:\n") +
			wxString("Render size: ") + wxString::Format("%d X %d\n", GetSize().x, GetSize().y) +
			wxString("Time: ") + wxString::Format("%llu msec\n", msec) +
			wxString("Frames: ") + wxString::Format("%llu\n", count) +
			wxString("FPS: ") + wxString::Format("%.2f", fps);
		wxMessageDialog *diag = new wxMessageDialog(this, string, "Benchmark Results",
			wxOK | wxICON_INFORMATION);
		diag->ShowModal();
	}

	if (auto view_ptr = m_render_view.lock())
	{
		view_ptr->SetRenderCanvas(0);
		view_ptr->SetRenderViewPanel(0);
		Root* root = glbin_data_manager.GetRoot();
		if (root)
			root->DeleteView(view_ptr.get());
	}

#ifdef _WIN32
	//tablet
	if (m_hTab)
	{
		gpWTClose(m_hTab);
		m_hTab = 0;
		UnloadWintab();
	}

#endif

	std::string str = GetName().ToStdString() + ":";

	if (m_full_screen)
	{
		m_full_screen = false;
		m_renderview_panel->SetFullFrame(0);
		if (m_frame)
		{
			//m_frame->EraseAllCanvases();
			m_frame->Close();
		}
	}

	if (!m_sharedRC)
		delete m_glRC;

}

#ifdef _WIN32
int RenderCanvas::GetPixelFormat(PIXELFORMATDESCRIPTOR *pfd) {
	int pixelFormat = ::GetPixelFormat(m_hDC);
	if (pixelFormat == 0) return GetLastError();
	pixelFormat = DescribePixelFormat(m_hDC, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), pfd);
	if (pixelFormat == 0) return GetLastError();
	return pixelFormat;
}
#endif

inline fluo::Point RenderCanvas::GetMousePos(wxMouseEvent& e)
{
	double dval = 1;
#ifdef _DARWIN
	dval = GetDPIScaleFactor();
#endif
	fluo::Point pnt(e.GetPosition().x, e.GetPosition().y, 0);
	pnt *= dval;
	if (auto view_ptr = m_render_view.lock())
	{
		if (view_ptr->GetEnlarge())
			pnt = pnt * view_ptr->GetEnlargeScale();
	}
	return pnt;
}

void RenderCanvas::SetFocusedSlider(wxBasisSlider* slider)
{
	if (m_focused_slider)
		m_focused_slider->SetIndicatorFocused(false);
	m_focused_slider = slider;
	if (m_focused_slider)
		m_focused_slider->SetIndicatorFocused(true);
}

void RenderCanvas::Draw()
{
#ifdef _WIN32
	if (!m_set_gl)
	{
		SetCurrent(*m_glRC);
		m_set_gl = true;

		Root* root = glbin_data_manager.GetRoot();
		if (root)
		{
			for (int i = 0; i< root->GetViewNum(); i++)
			{
				auto view = root->GetView(i);
				RenderCanvas* canvas = view->GetRenderCanvas();
				if (canvas && canvas != this)
				{
					canvas->m_set_gl = false;
				}
			}
		}
	}
#endif
#if defined(_DARWIN) || defined(__linux__)
	SetCurrent(*m_glRC);
#endif

	wxPaintDC dc(this);
	if (auto view_ptr = m_render_view.lock())
	{
		glbin_current.render_view_drawing = view_ptr;
		bool bval = view_ptr->Draw();
		if (bval)
		{
			view_ptr->DrawDefault();
			SwapBuffers();
			DBGPRINT(L"SwapBuffers();\n");
		}
	}

}
void RenderCanvas::Init()
{
	if (auto view_ptr = m_render_view.lock())
		view_ptr->Init();
	m_renderview_panel->FluoRefresh(0, { gstMaxTextureSize, gstDeviceTree }, { -1 });
}

#ifdef _WIN32
//tablet init
HCTX RenderCanvas::TabletInit(HWND hWnd, HINSTANCE hInst)
{
	HCTX hctx = NULL;
	UINT wDevice = 0;
	UINT wExtX = 0;
	UINT wExtY = 0;
	UINT wWTInfoRetVal = 0;
	AXIS TabletX = { 0 };
	AXIS TabletY = { 0 };
	AXIS TabletNPress = { 0 };
	AXIS TabletTPress = { 0 };

	// Set option to move system cursor before getting default system context.
	m_lc.lcOptions |= CXO_SYSTEM;

	// Open default system context so that we can get tablet data
	// in screen coordinates (not tablet coordinates).
	wWTInfoRetVal = gpWTInfoA(WTI_DEFSYSCTX, 0, &m_lc);
	WACOM_ASSERT(wWTInfoRetVal == sizeof(LOGCONTEXTA));

	WACOM_ASSERT(m_lc.lcOptions & CXO_SYSTEM);

	// modify the digitizing region
	sprintf(m_lc.lcName,
		"FluoRender Digitizing %llx",
		reinterpret_cast<unsigned long long>(hInst));

	// We process WT_PACKET (CXO_MESSAGES) messages.
	m_lc.lcOptions |= CXO_MESSAGES;

	// What data items we want to be included in the tablet packets
	m_lc.lcPktData = PACKETDATA;

	// Which packet items should show change in value since the last
	// packet (referred to as 'relative' data) and which items
	// should be 'absolute'.
	m_lc.lcPktMode = PACKETMODE;

	// This bitfield determines whether or not this context will receive
	// a packet when a value for each packet field changes.  This is not
	// supported by the Intuos Wintab.  Your context will always receive
	// packets, even if there has been no change in the data.
	m_lc.lcMoveMask = PACKETDATA;

	// Which buttons events will be handled by this context.  lcBtnMask
	// is a bitfield with one bit per button.
	m_lc.lcBtnUpMask = m_lc.lcBtnDnMask;

	// Set the entire tablet as active
	wWTInfoRetVal = gpWTInfoA(WTI_DEVICES + 0, DVC_X, &TabletX);
	WACOM_ASSERT(wWTInfoRetVal == sizeof(AXIS));

	wWTInfoRetVal = gpWTInfoA(WTI_DEVICES, DVC_Y, &TabletY);
	WACOM_ASSERT(wWTInfoRetVal == sizeof(AXIS));

	wWTInfoRetVal = gpWTInfoA(WTI_DEVICES, DVC_NPRESSURE, &TabletNPress);
	if (wWTInfoRetVal == sizeof(AXIS))
		glbin_vol_selector.SetBrushPnMax(TabletNPress.axMax);
	else
		glbin_vol_selector.SetBrushPnMax(1.0);

	wWTInfoRetVal = gpWTInfoA(WTI_DEVICES, DVC_TPRESSURE, &TabletTPress);
	if (wWTInfoRetVal == sizeof(AXIS))
		glbin_vol_selector.SetBrushPtMax(TabletTPress.axMax);
	else
		glbin_vol_selector.SetBrushPnMax(1.0);

/*	m_lc.lcInOrgX = 0;
	m_lc.lcInOrgY = 0;
	m_lc.lcInExtX = TabletX.axMax;
	m_lc.lcInExtY = TabletY.axMax;

	// Guarantee the output coordinate space to be in screen coordinates.  
	m_lc.lcOutOrgX = GetSystemMetrics(SM_XVIRTUALSCREEN);
	m_lc.lcOutOrgY = GetSystemMetrics(SM_YVIRTUALSCREEN);
	m_lc.lcOutExtX = GetSystemMetrics(SM_CXVIRTUALSCREEN); //SM_CXSCREEN );

														   // In Wintab, the tablet origin is lower left.  Move origin to upper left
														   // so that it coincides with screen origin.
	m_lc.lcOutExtY = -GetSystemMetrics(SM_CYVIRTUALSCREEN);	//SM_CYSCREEN );

															// Leave the system origin and extents as received:
															// lcSysOrgX, lcSysOrgY, lcSysExtX, lcSysExtY

															// open the region
															// The Wintab spec says we must open the context disabled if we are 
															// using cursor masks.  
*/	hctx = gpWTOpenA(hWnd, &m_lc, TRUE);

	WacomTrace("HCTX: %i\n", hctx);

	return hctx;
}
#endif

void RenderCanvas::OnResize(wxSizeEvent& event)
{
	double dval = 1;
#ifdef _DARWIN
	dval = GetDPIScaleFactor();
#endif

	if (auto view_ptr = m_render_view.lock())
	{
		view_ptr->SetDpiFactor(dval);

		wxSize size = GetSize();
		view_ptr->SetSize(size.x, size.y);

		wxRect rect = GetClientRect();
		view_ptr->SetClient(rect.x, rect.y, rect.width, rect.height);

		view_ptr->RefreshGL(1);
	}

	m_renderview_panel->FluoUpdate({ gstScaleFactor });
}

void RenderCanvas::OnMove(wxMoveEvent& event)
{
	wxRect rect = GetClientRect();
	if (auto view_ptr = m_render_view.lock())
		view_ptr->SetClient(rect.x, rect.y, rect.width, rect.height);
}

void RenderCanvas::OnIdle(wxIdleEvent& event)
{
	IdleState state;
	auto view_ptr = m_render_view.lock();

	state.m_movie_maker_render_canvas = view_ptr && (glbin_moviemaker.GetView() == view_ptr.get());
	//mouse state
	wxPoint mps = wxGetMousePosition();
	state.m_mouse_over = wxFindWindowAtPoint(mps) == this &&
		!glbin_states.m_modal_shown;
	state.m_mouse_left = wxGetMouseState().LeftIsDown();
	//key state
	state.m_key_paint = wxGetKeyState(WXK_SHIFT);
	state.m_key_erase = wxGetKeyState(wxKeyCode('x'));
	state.m_key_diff = wxGetKeyState(wxKeyCode('z'));
	state.m_key_ctrl = wxGetKeyState(WXK_CONTROL);
	state.m_key_refresh = wxGetKeyState(WXK_F5);
	state.m_key_mask = wxGetKeyState(wxKeyCode('v'));
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

	m_prev_focus = FindFocus();

	if (view_ptr)
		view_ptr->ProcessIdle(state);

	event.RequestMore(state.m_request_more);

	//full screen
	if (state.m_fullscreen)
		m_renderview_panel->SetFullScreen();
	if (state.m_exit_fullscreen)
		m_exit_fscreen_trigger.Start(10);

	if (state.m_refresh)
		m_renderview_panel->FluoRefresh(0, state.m_value_collection, {-1});
	if (state.m_set_focus)
		SetFocus();
	if (glbin_states.m_benchmark)
	{
		wxString title = wxString(FLUORENDER_TITLE) +
			" " + wxString(VERSION_MAJOR_TAG) +
			"." + wxString(VERSION_MINOR_TAG) +
			" Benchmarking... FPS = " +
			wxString::Format("%.2f", state.m_benchmark_fps);
		m_frame->SetTitle(title);
	}

	if (state.m_set_cur_focus)
		SetFocus();

	if (state.m_set_previous_focus && m_prev_focus)
		m_prev_focus->SetFocus();
}

void RenderCanvas::OnQuitFscreen(wxTimerEvent& event)
{
	m_exit_fscreen_trigger.Stop();
	if (!m_frame || !m_renderview_panel)
		return;

	m_full_screen = false;
	wxFrame* full_frame = m_renderview_panel->GetFullFrame();
	if (glbin_states.m_benchmark)
	{
		if (full_frame)
			full_frame->Hide();
		if (m_frame)
			m_frame->Close();
	}
	else if (GetParent() == full_frame)
	{
		Reparent(m_renderview_panel);
		wxBoxSizer* view_sizer = m_renderview_panel->GetViewSizer();
		if (view_sizer)
		{
			view_sizer->Clear();
			view_sizer->Add(this, 1, wxEXPAND);
		}
		m_renderview_panel->Layout();
		full_frame->Hide();
		if (m_frame)
		{
#ifdef _WIN32
			if (glbin_settings.m_show_cursor)
				ShowCursor(true);
#endif
			m_frame->Iconize(false);
			m_frame->SetFocus();
			m_frame->Raise();
			m_frame->Show();
		}
		if (auto view_ptr = m_render_view.lock())
			view_ptr->RefreshGL(2);
	}
}

void RenderCanvas::OnClose(wxCloseEvent &event)
{
	if (m_full_screen)
	{
		m_exit_fscreen_trigger.Start(10);
		event.Veto();
	}
}

//draw
void RenderCanvas::OnDraw(wxPaintEvent& event)
{
	Draw();
}

#ifdef _WIN32
WXLRESULT RenderCanvas::MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam)
{
	if (glbin_vol_selector.GetBrushUsePres())
	{
		if (message == WT_PACKET)
		{
			PACKET pkt = {};
			if (gpWTPacket((HCTX)lParam, wParam, &pkt))
				glbin_vol_selector.SetPressure(pkt.pkNormalPressure, pkt.pkTangentPressure);
		}
	}

	if (m_enable_touch)
	{
		if (message == WM_POINTERDOWN)
		{
			POINTER_INFO pointerinfo = {};
			// get pointer info
			if (GetPI(GET_POINTERID_WPARAM(
				wParam), &pointerinfo))
			{
				int id = pointerinfo.pointerId;
				if (m_ptr_id1 < 0)
				{
					m_ptr_id1 = id;
					m_ptr1_x = pointerinfo.ptPixelLocation.x;
					m_ptr1_y = pointerinfo.ptPixelLocation.y;
					m_ptr1_x_save = m_ptr1_x;
					m_ptr1_y_save = m_ptr1_y;
				}
				else if (m_ptr_id2 < 0 &&
					m_ptr_id1 != id)
				{
					m_ptr_id2 = id;
					m_ptr2_x = pointerinfo.ptPixelLocation.x;
					m_ptr2_y = pointerinfo.ptPixelLocation.y;
					m_ptr2_x_save = m_ptr2_x;
					m_ptr2_y_save = m_ptr2_y;
				}
				if (m_ptr_id1 >= 0 &&
					m_ptr_id2 >= 0)
				{
					//simulate middle button
					wxMouseEvent me(wxEVT_MIDDLE_DOWN);
					int ptx = (m_ptr1_x + m_ptr2_x) / 2;
					int pty = (m_ptr1_y + m_ptr2_y) / 2;
					me.SetX(ptx);
					me.SetY(pty);
					ProcessWindowEvent(me);
				}
			}
		}
		else if (message == WM_POINTERUP)
		{
			POINTER_INFO pointerinfo = {};
			// get pointer info
			if (GetPI(GET_POINTERID_WPARAM(
				wParam), &pointerinfo))
			{
				int id = pointerinfo.pointerId;
				if (m_ptr_id1 == id)
					m_ptr_id1 = -1;
				else if (m_ptr_id2 == id)
					m_ptr_id2 = -1;
				if (m_ptr_id1 < 0 &&
					m_ptr_id2 < 0)
				{
					//simulate middle button
					wxMouseEvent me(wxEVT_MIDDLE_UP);
					ProcessWindowEvent(me);
				}
			}
		}
		else if (message == WM_POINTERUPDATE)
		{
			POINTER_INFO pointerinfo = {};
			// get pointer info
			if (GetPI(GET_POINTERID_WPARAM(
				wParam), &pointerinfo))
			{
				int id = pointerinfo.pointerId;
				if (m_ptr_id1 == id)
				{
					m_ptr1_x = pointerinfo.ptPixelLocation.x;
					m_ptr1_y = pointerinfo.ptPixelLocation.y;
				}
				else if (m_ptr_id2 == id)
				{
					m_ptr2_x = pointerinfo.ptPixelLocation.x;
					m_ptr2_y = pointerinfo.ptPixelLocation.y;
				}
			}

			if (m_ptr_id1 > 0 &&
				m_ptr_id2 > 0)
			{
				int dist1 = abs(m_ptr1_x_save - m_ptr2_x_save) +
					abs(m_ptr1_y_save - m_ptr2_y_save);
				int dist2 = abs(m_ptr1_x - m_ptr2_x) +
					abs(m_ptr1_y - m_ptr2_y);
				if (abs(dist1 - dist2) < 20)
				{
					wxMouseEvent me(wxEVT_MOTION);
					me.SetMiddleDown(true);
					int ptx = (m_ptr1_x + m_ptr2_x) / 2;
					int pty = (m_ptr1_y + m_ptr2_y) / 2;
					me.SetX(ptx);
					me.SetY(pty);
					ProcessWindowEvent(me);

				}
			}
		}
	}

	return wxWindow::MSWWindowProc(message, wParam, lParam);
}
#endif

void RenderCanvas::OnMouse(wxMouseEvent& event)
{
	MouseState state;

	state.m_valid_focus_slider = m_focused_slider != nullptr;
	//mouse
	state.m_mouse_left_down = event.LeftDown();
	state.m_mouse_right_down = event.RightDown();
	state.m_mouse_middle_down = event.MiddleDown();
	state.m_mouse_left_up = event.LeftUp();
	state.m_mouse_right_up = event.RightUp();
	state.m_mouse_middle_up = event.MiddleUp();
	state.m_mouse_left_is_down = event.LeftIsDown();
	state.m_mouse_right_is_down = event.RightIsDown();
	state.m_mouse_middle_is_down = event.MiddleIsDown();
	state.m_mouse_drag = event.Dragging();
	state.m_mouse_wheel_rotate = event.GetWheelRotation();
	state.m_mouse_wheel_delta = event.GetWheelDelta();
	//key
	state.m_key_alt = event.AltDown();
	state.m_key_ctrl = event.ControlDown();

	wxWindow *window = wxWindow::FindFocus();
	if (window &&
		(state.m_mouse_left_down ||
		state.m_mouse_right_down ||
		state.m_mouse_middle_down ||
		state.m_mouse_left_up ||
		state.m_mouse_right_up ||
		state.m_mouse_middle_up ||
		state.m_mouse_drag ||
		state.m_mouse_wheel_rotate != 0))
		SetFocus();

	if (auto view_ptr = m_render_view.lock())
	{
		view_ptr->SetMousePos(event.GetPosition().x, event.GetPosition().y);

		view_ptr->ProcessMouse(state);
	}

	if (state.m_reset_focus_slider)
		SetFocusedSlider(nullptr);
	if (state.m_scroll_focus_slider &&
		m_focused_slider &&
		m_focused_slider->IsEnabled())
	{
		int value = state.m_mouse_wheel_rotate / state.m_mouse_wheel_delta;
		m_focused_slider->Scroll(value);
	}
}

