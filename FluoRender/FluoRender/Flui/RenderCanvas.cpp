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

#include <fl_gl.h>
#include <RenderCanvas.h>
#include <GLAttribProvider.h>
#include <Global.h>
#include <Tokens.h>
#include <MainFrame.h>
#include <RenderViewPanel.h>
#include <VolumeSelector.h>
#include <RenderView.h>
#include <StopWatch.hpp>
#include <MovieMaker.h>
#include <GlobalStates.h>
#include <MainSettings.h>
#include <CurrentObjects.h>
#include <Root.h>
#include <DataManager.h>
#include <LookingGlassRenderer.h>
#include <FramebufferStateTracker.h>
#include <RenderCanvasAgent.h>
#include <Coordinator.h>
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

wxDEFINE_EVENT(EVT_RENDER_SCHEDULER_DRAW, wxCommandEvent);

#if defined(__WXMSW__) // Windows
#include <wx/msw/wrapwin.h>

GLADapiproc MyGLGetProcAddress(const char* name) {
	// 1. Try to load modern/extension functions first
	void* p = (void*)wglGetProcAddress(name);

	// 2. If wglGetProcAddress returns an invalid pointer, fallback to the DLL
	if (p == 0 || (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) || (p == (void*)-1)) {
		static HMODULE openGLModule = LoadLibraryA("opengl32.dll");
		p = (void*)GetProcAddress(openGLModule, name);
	}
	return (GLADapiproc)p;
}

#elif defined(__WXGTK__) // Linux
#include <GL/glx.h>

GLADapiproc MyGLGetProcAddress(const char* name)
{
	return (GLADapiproc)glXGetProcAddress((const GLubyte*)name);
}

#elif defined(__WXMAC__) // macOS
//#include <dlfcn.h>
//GLADapiproc MyGLGetProcAddress(const char* name) {
//	return (GLADapiproc)dlsym(RTLD_DEFAULT, name);
//}
#endif

RenderCanvas::RenderCanvas(
	wxWindow* parent,
	wxGLContext* sharedContext,
	const wxPoint& pos,
	const wxSize& size,
	long style) :
	wxGLCanvas(parent, GLAttribProvider::CanvasAttribs(), wxID_ANY, pos, size, style),
	m_glRC(sharedContext),
	//timer for fullscreen
	m_exit_fscreen_trigger(this, 0),
	m_enable_touch(false),
	m_ptr_id1(-1),
	m_ptr_id2(-1),
	m_full_screen(false),
	m_focused_slider(0)
{
}

RenderCanvas::~RenderCanvas()
{
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
		//m_full_screen = false;
		//m_renderview_panel->SetFullFrame(0);
		//if (auto frame_agent = glbin_coordinator.FindAgent<MainFrameAgent>())
		//{
		//	if (auto frame = frame_agent->GetMainFrame())
		//	{
		//		//m_frame->EraseAllCanvases();
		//		frame->Close();
		//	}
		//}
	}

	if (!m_sharedRC)
		delete m_glRC;
}

void RenderCanvas::CreateUI()
{
	m_sharedRC = (m_glRC != nullptr);

	if (!m_sharedRC)
	{
		wxGLContextAttrs ctxAttrs = GLAttribProvider::MakeContextAttrs();
		m_glRC = new wxGLContext(this, nullptr, &ctxAttrs);

		if (!m_glRC->IsOK())
		{
			ctxAttrs.Reset();
			ctxAttrs.PlatformDefaults().EndList();
			m_glRC = new wxGLContext(this, nullptr, &ctxAttrs);
		}

		if (!m_glRC->IsOK())
		{
			wxMessageBox(
				"FluoRender needs an OpenGL 3.3 capable driver.\n"
				"Please update your graphics card driver or upgrade "
				"your graphics card.",
				"Graphics card error",
				wxOK | wxICON_ERROR,
				this);

			delete m_glRC;
			m_glRC = nullptr;
			return;
		}
	}

#ifdef _DEBUG
	PIXELFORMATDESCRIPTOR  pfd;
	//check ret. this is an error code when the pixel format is invalid.
	int ret = GetPixelFormat(&pfd);
#endif

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

#ifdef _WIN32
int RenderCanvas::GetPixelFormat(PIXELFORMATDESCRIPTOR* pfd)
{
	int pixelFormat = ::GetPixelFormat(m_hDC);
	if (pixelFormat == 0) return GetLastError();
	pixelFormat = DescribePixelFormat(m_hDC, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), pfd);
	if (pixelFormat == 0) return GetLastError();
	return pixelFormat;
}
#endif

RenderCanvasAgent* RenderCanvas::GetRenderCanvasAgent() const
{
	return dynamic_cast<RenderCanvasAgent*>(m_agent.get());
}

void RenderCanvas::SetFocusedSlider(wxBasisSlider* slider)
{
	if (m_focused_slider)
		m_focused_slider->SetIndicatorFocused(false);
	m_focused_slider = slider;
	if (m_focused_slider)
		m_focused_slider->SetIndicatorFocused(true);
}

void RenderCanvas::ExitFullscreen()
{
	if (m_full_screen)
		m_exit_fscreen_trigger.Start(10);
}

double RenderCanvas::GetDpiScale()
{
#ifdef _DARWIN
	return GetDPIScaleFactor(); // Logical->physical scale
#else
	return 1.0;
#endif
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
	auto agent = GetRenderCanvasAgent();
	if (agent)
		agent->UpdateUIToData({ gstResize });
	// Always process the event so the framework can continue its layout pass.
	event.Skip();
}

void RenderCanvas::OnMove(wxMoveEvent& event)
{
	auto agent = GetRenderCanvasAgent();
	if (agent)
		agent->UpdateUIToData({ gstCanvasMove });
}

void RenderCanvas::OnIdle(wxIdleEvent& event)
{
	auto agent = GetRenderCanvasAgent();
	if (agent)
	{
		agent->UpdateUIToData({ gstCanvasIdle });
		event.RequestMore(agent->GetRequestMore());
	}
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
		if (auto view = GetRenderView())
			if (auto agent = GetRenderCanvasAgent())
				agent->RequestDraw();
	}
}

void RenderCanvas::OnClose(wxCloseEvent& event)
{
	ExitFullscreen();
	event.Veto();
}

//draw
void RenderCanvas::OnDraw(wxPaintEvent& event)
{
	wxPaintDC dc(this);
	SetCurrent(*m_glRC);

#ifdef __APPLE__

#else
	static bool glad_init = false;
	if (!glad_init)
	{
		int result = gladLoadGL(MyGLGetProcAddress);
		glad_init = result > 0;
	}
#endif

	auto agent = m_agent->As<RenderCanvasAgent>();
	if (agent)
		agent->NotifyViewUpdate({}, UpdateMode::SenderOnly, "RenderCanvas::OnDraw()");
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

	wxWindow* window = wxWindow::FindFocus();
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

	if (auto view = GetRenderView())
	{
		view->SetMousePos(event.GetPosition().x, event.GetPosition().y);

		view->ProcessMouse(state);
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

