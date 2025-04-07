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
	m_fullscreen_trigger(this, ID_ftrigger),
	m_enable_touch(false),
	m_ptr_id1(-1),
	m_ptr_id2(-1),
	m_full_screen(false),
	m_focused_slider(0)
#if defined(_WIN32) && defined(USE_XINPUT)
	,
	m_controller(0)
#endif
{
	m_glRC = sharedContext;
	m_sharedRC = m_glRC ? true : false;

	m_render_view = new RenderView();
	m_render_view->SetSize(size.x, size.y);

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

	//xbox controller
#ifdef USE_XINPUT
	m_controller = new XboxController(1);
	if (m_controller)
	{
		m_control_connected = m_controller->IsConnected();
	}
#endif
#endif

	if (m_frame && m_frame->GetBenchmark())
		m_render_view->SetBenchmark(true);
	else
		m_render_view->SetBenchmark(false);

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
	if (m_render_view->GetBenchmark())
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

	if (m_render_view)
		delete m_render_view;

#ifdef _WIN32
	//tablet
	if (m_hTab)
	{
		gpWTClose(m_hTab);
		m_hTab = 0;
		UnloadWintab();
	}

#ifdef USE_XINPUT
	if (m_controller)
		delete m_controller;
#endif
#endif

	std::string str = GetName().ToStdString() + ":";

	if (m_full_screen)
	{
		m_full_screen = false;
		m_renderview_panel->m_canvas = 0;
		m_renderview_panel->m_full_frame = 0;
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
	if (m_render_view->GetEnlarge())
		pnt = pnt * m_render_view->GetEnlargeScale();
	return pnt;
}

void RenderCanvas::Draw()
{
#ifdef _WIN32
	if (!m_set_gl)
	{
		SetCurrent(*m_glRC);
		m_set_gl = true;
		if (m_frame)
		{
			for (int i = 0; i< m_frame->GetCanvasNum(); i++)
			{
				RenderCanvas* view = m_frame->GetRenderCanvas(i);
				if (view && view != this)
				{
					view->m_set_gl = false;
				}
			}
		}
	}
#endif
#if defined(_DARWIN) || defined(__linux__)
	SetCurrent(*m_glRC);
#endif

	wxPaintDC dc(this);
	if (m_render_view->ForceDraw())
		SwapBuffers();

}
void RenderCanvas::Init()
{
	m_render_view->Init();
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
	m_render_view->SetDpiFactor(dval);

	wxSize size = GetSize();
	m_render_view->SetSize(size.x, size.y);

	wxRect rect = GetClientRect();
	m_render_view->SetClient(rect.x, rect.y, rect.width, rect.height);

	m_render_view->RefreshGL(1);
	m_renderview_panel->FluoUpdate({ gstScaleFactor });
}

void RenderCanvas::OnMove(wxMoveEvent& event)
{
	wxRect rect = GetClientRect();
	m_render_view->SetClient(rect.x, rect.y, rect.width, rect.height);
}

void RenderCanvas::OnIdle(wxIdleEvent& event)
{
	fluo::ValueCollection vc;

	bool refresh = false;
	bool ref_stat = false;
	bool start_loop = true;
	bool set_focus = false;
	bool lg_changed = false;
	m_retain_finalbuffer = false;

	if (glbin_settings.m_hologram_mode == 2)
	{
		//make sure all views are drawn for the quilt
		if (glbin_lg_renderer.GetFinished())
			event.RequestMore(false);
		else
		{
			event.RequestMore(true);
			refresh = true;
			lg_changed = false;
		}
	}

	//check memory swap status
	if (glbin_settings.m_mem_swap &&
		flvr::TextureRenderer::get_start_update_loop() &&
		!flvr::TextureRenderer::get_done_update_loop())
	{
		if (flvr::TextureRenderer::active_view_ == m_renderview_panel->m_id)
		{
			refresh = true;
			start_loop = false;
			lg_changed = true;
		}
	}
	else
	{
		if (glbin_moviemaker.GetRenderCanvas() == this)
		{
			//DBGPRINT(L"lg finished: %d, cur view: %d\n",
			//	glbin_lg_renderer.GetFinished(),
			//	glbin_lg_renderer.GetCurView());
			if (glbin_lg_renderer.GetFinished())
			{
				refresh = glbin_moviemaker.Action();
				event.RequestMore(glbin_moviemaker.IsRunning());
				if (refresh)
				{
					lg_changed = true;
					vc.insert(gstCamRotation);
					if (!glbin_moviemaker.IsRunning())
						vc.insert(gstMovPlay);
				}
			}
		}
	}

	if (m_capture_rotat ||
		m_capture_tsequ ||
		m_capture_param ||
		glbin_settings.m_test_speed)
	{
		refresh = true;
		lg_changed = true;
		if (glbin_settings.m_mem_swap &&
			flvr::TextureRenderer::get_done_update_loop())
			m_pre_draw = true;
	}

	if (m_use_openxr)
	{
		event.RequestMore(true);
		refresh = true;
		lg_changed = true;
		//m_retain_finalbuffer = true;
	}

	if (m_frame && m_frame->GetBenchmark())
	{
		double fps = 1.0 / glbin.getStopWatch(gstStopWatch)->average();
		wxString title = wxString(FLUORENDER_TITLE) +
			" " + wxString(VERSION_MAJOR_TAG) +
			"." + wxString(VERSION_MINOR_TAG) +
			" Benchmarking... FPS = " +
			wxString::Format("%.2f", fps);
		m_frame->SetTitle(title);

		refresh = true;
		lg_changed = true;
		if (glbin_settings.m_mem_swap &&
			flvr::TextureRenderer::get_done_update_loop())
			m_pre_draw = true;
	}

	//update pin rotation center
	if (m_update_rot_ctr && m_cur_vol && !m_free)
	{
		fluo::Point p, ip;
		int nx = GetGLSize().w();
		int ny = GetGLSize().h();
		int mode = 2;
		if (m_cur_vol->GetMode() == 1) mode = 1;
		glbin_volume_point.SetVolumeData(m_cur_vol);
		double dist = glbin_volume_point.GetPointVolume(nx / 2.0, ny / 2.0,
			mode, true, m_pin_pick_thresh, p, ip);
		if (dist <= 0.0)
			dist = glbin_volume_point.GetPointVolumeBox(
				nx / 2.0, ny / 2.0,
				true, p);
		if (dist > 0.0)
		{
			m_pin_ctr = p;
			double obj_transx, obj_transy, obj_transz;
			p = fluo::Point(m_obj_ctrx + m_obj_ctr_offx - p.x(),
				p.y() - m_obj_ctry - m_obj_ctr_offy,
				p.z() - m_obj_ctrz - m_obj_ctr_offz);
			obj_transx = p.x();
			obj_transy = p.y();
			obj_transz = p.z();
			double thresh = 10.0;
			double spcx, spcy, spcz;
			m_cur_vol->GetSpacings(spcx, spcy, spcz);
			thresh *= spcx;
			if (sqrt((m_obj_transx - obj_transx)*(m_obj_transx - obj_transx) +
				(m_obj_transy - obj_transy)*(m_obj_transy - obj_transy) +
				(m_obj_transz - obj_transz)*(m_obj_transz - obj_transz)) > thresh)
			{
				m_obj_transx = obj_transx;
				m_obj_transy = obj_transy;
				m_obj_transz = obj_transz;
			}
		}
		m_update_rot_ctr = false;
		refresh = true;
		lg_changed = true;
	}

	wxPoint mps = wxGetMousePosition();
	bool mouse_over = wxFindWindowAtPoint(mps) == this &&
		!glbin_states.m_modal_shown;

	if (mouse_over)
	{
		//forced refresh
		if (wxGetKeyState(WXK_F5))
		{
			SetFocus();
			m_clear_buffer = true;
			m_updating = true;
			glbin_states.m_status_str = "Forced Refresh";
			m_frame->FluoUpdate({ gstMainStatusbarPush });
			wxSizeEvent e;
			OnResize(e);
			RefreshGL(14, false, true, true);
			m_frame->FluoUpdate({ gstMainStatusbarPop });
			return;
		}

		if (UpdateBrushState())
		{
			set_focus = true;
			refresh = true;
			vc.insert({ gstSelUndo, gstBrushState, gstBrushThreshold, gstBrushSize1, gstBrushSize2 });
			if (glbin_brush_def.m_update_size)
				vc.insert(gstBrushCountResult);
			if (glbin_brush_def.m_update_colocal)
				vc.insert(gstColocalResult);
		}

		//draw_mask
		if (wxGetKeyState(wxKeyCode('V')) &&
			m_draw_mask)
		{
			m_draw_mask = false;
			refresh = true;
			lg_changed = true;
			set_focus = true;
		}
		if (!wxGetKeyState(wxKeyCode('V')) &&
			!m_draw_mask)
		{
			m_draw_mask = true;
			refresh = true;
			lg_changed = true;
		}

		//move view
		//left
		if (!m_move_left &&
			wxGetKeyState(WXK_CONTROL) &&
			wxGetKeyState(WXK_LEFT))
		{
			m_move_left = true;

			m_head = fluo::Vector(-m_transx, -m_transy, -m_transz);
			m_head.normalize();
			fluo::Vector side = fluo::Cross(m_up, m_head);
			fluo::Vector trans = -(side * (std::round(0.8 * (m_ortho_right - m_ortho_left))));
			m_obj_transx += trans.x();
			m_obj_transy += trans.y();
			m_obj_transz += trans.z();
			//if (m_persp) SetSortBricks();
			refresh = true;
			lg_changed = true;
			set_focus = true;
		}
		if (m_move_left &&
			(!wxGetKeyState(WXK_CONTROL) ||
				!wxGetKeyState(WXK_LEFT)))
			m_move_left = false;
		//right
		if (!m_move_right &&
			wxGetKeyState(WXK_CONTROL) &&
			wxGetKeyState(WXK_RIGHT))
		{
			m_move_right = true;

			m_head = fluo::Vector(-m_transx, -m_transy, -m_transz);
			m_head.normalize();
			fluo::Vector side = fluo::Cross(m_up, m_head);
			fluo::Vector trans = side * (std::round(0.8 * (m_ortho_right - m_ortho_left)));
			m_obj_transx += trans.x();
			m_obj_transy += trans.y();
			m_obj_transz += trans.z();
			//if (m_persp) SetSortBricks();
			refresh = true;
			lg_changed = true;
			set_focus = true;
		}
		if (m_move_right &&
			(!wxGetKeyState(WXK_CONTROL) ||
				!wxGetKeyState(WXK_RIGHT)))
			m_move_right = false;
		//up
		if (!m_move_up &&
			wxGetKeyState(WXK_CONTROL) &&
			wxGetKeyState(WXK_UP))
		{
			m_move_up = true;

			m_head = fluo::Vector(-m_transx, -m_transy, -m_transz);
			m_head.normalize();
			fluo::Vector trans = -m_up * (std::round(0.8 * (m_ortho_top - m_ortho_bottom)));
			m_obj_transx += trans.x();
			m_obj_transy += trans.y();
			m_obj_transz += trans.z();
			//if (m_persp) SetSortBricks();
			refresh = true;
			lg_changed = true;
			set_focus = true;
		}
		if (m_move_up &&
			(!wxGetKeyState(WXK_CONTROL) ||
				!wxGetKeyState(WXK_UP)))
			m_move_up = false;
		//down
		if (!m_move_down &&
			wxGetKeyState(WXK_CONTROL) &&
			wxGetKeyState(WXK_DOWN))
		{
			m_move_down = true;

			m_head = fluo::Vector(-m_transx, -m_transy, -m_transz);
			m_head.normalize();
			fluo::Vector trans = m_up * (std::round(0.8 * (m_ortho_top - m_ortho_bottom)));
			m_obj_transx += trans.x();
			m_obj_transy += trans.y();
			m_obj_transz += trans.z();
			//if (m_persp) SetSortBricks();
			refresh = true;
			lg_changed = true;
			set_focus = true;
		}
		if (m_move_down &&
			(!wxGetKeyState(WXK_CONTROL) ||
				!wxGetKeyState(WXK_DOWN)))
			m_move_down = false;

		//move time sequence
		//forward
		if ((!m_tseq_forward &&
			wxGetKeyState(wxKeyCode('d'))) ||
			(!wxGetKeyState(WXK_CONTROL) &&
			wxGetKeyState(WXK_SPACE)))
		{
			m_tseq_forward = true;
			int frame = glbin_moviemaker.GetCurrentFrame();
			frame++;
			glbin_moviemaker.SetCurrentFrame(frame);
			refresh = true;
			lg_changed = true;
			set_focus = true;
			vc.insert({ gstMovProgSlider, gstCurrentFrame, gstMovCurTime, gstMovSeqNum, gstTrackList });
		}
		if (m_tseq_forward &&
			!wxGetKeyState(wxKeyCode('d')))
			m_tseq_forward = false;
		//backforward
		if ((!m_tseq_backward &&
			wxGetKeyState(wxKeyCode('a'))) ||
			(wxGetKeyState(WXK_SPACE) &&
			wxGetKeyState(WXK_CONTROL)))
		{
			m_tseq_backward = true;
			int frame = glbin_moviemaker.GetCurrentFrame();
			frame--;
			glbin_moviemaker.SetCurrentFrame(frame);
			refresh = true;
			lg_changed = true;
			set_focus = true;
			vc.insert({ gstMovProgSlider, gstCurrentFrame, gstMovCurTime, gstMovSeqNum, gstTrackList });
		}
		if (m_tseq_backward &&
			!wxGetKeyState(wxKeyCode('a')))
			m_tseq_backward = false;

		//move clip
		//up
		if (!m_clip_up &&
			wxGetKeyState(wxKeyCode('s')))
		{
			m_clip_up = true;
			if (m_frame && m_frame->GetClipPlanPanel())
				m_frame->GetClipPlanPanel()->MoveLinkedClippingPlanes(-1);
			refresh = true;
			lg_changed = true;
			set_focus = true;
			vc.insert({ gstClipX1, gstClipX2, gstClipY1, gstClipY2, gstClipZ1, gstClipZ2 });
		}
		if (m_clip_up &&
			!wxGetKeyState(wxKeyCode('s')))
			m_clip_up = false;
		//down
		if (!m_clip_down &&
			wxGetKeyState(wxKeyCode('w')))
		{
			m_clip_down = true;
			if (m_frame && m_frame->GetClipPlanPanel())
				m_frame->GetClipPlanPanel()->MoveLinkedClippingPlanes(1);
			refresh = true;
			lg_changed = true;
			set_focus = true;
			vc.insert({ gstClipX1, gstClipX2, gstClipY1, gstClipY2, gstClipZ1, gstClipZ2 });
		}
		if (m_clip_down &&
			!wxGetKeyState(wxKeyCode('w')))
			m_clip_down = false;

		//cell full
		if (!m_cell_full &&
			wxGetKeyState(wxKeyCode('f')))
		{
			m_cell_full = true;
			glbin_comp_selector.SelectFullComp();
			refresh = true;
			lg_changed = true;
			set_focus = true;
			vc.insert({ gstTrackList, gstSelUndoRedo });
		}
		if (m_cell_full &&
			!wxGetKeyState(wxKeyCode('f')))
			m_cell_full = false;
		//cell link
		if (!m_cell_link &&
			wxGetKeyState(wxKeyCode('l')))
		{
			m_cell_link = true;
			glbin_trackmap_proc.LinkCells(false);
			refresh = true;
			lg_changed = true;
			set_focus = true;
			vc.insert({ gstTrackList, gstSelUndoRedo });
		}
		if (m_cell_link &&
			!wxGetKeyState(wxKeyCode('l')))
			m_cell_link = false;
		//new cell id
		if (!m_cell_new_id &&
			wxGetKeyState(wxKeyCode('n')))
		{
			m_cell_new_id = true;
			glbin_comp_editor.NewId(false, true);
			refresh = true;
			lg_changed = true;
			set_focus = true;
			vc.insert({ gstTrackList, gstSelUndoRedo });
		}
		if (m_cell_new_id &&
			!wxGetKeyState(wxKeyCode('n')))
			m_cell_new_id = false;
		//clear
		if (wxGetKeyState(wxKeyCode('c')) &&
			!m_clear_mask)
		{
			glbin_vol_selector.Clear();
			glbin_comp_selector.Clear();
			GetTraces(false);
			m_clear_mask = true;
			refresh = true;
			lg_changed = true;
			set_focus = true;
			vc.insert({ gstTrackList, gstSelUndoRedo });
		}
		if (!wxGetKeyState(wxKeyCode('c')) &&
			m_clear_mask)
			m_clear_mask = false;
		//save all masks
		if (wxGetKeyState(wxKeyCode('m')) &&
			!m_save_mask)
		{
			//if (m_frame && m_frame->GetListPanel())
			//	m_frame->GetListPanel()->SaveAllMasks();
			VolumeData* vd = glbin_current.vol_data;
			if (vd)
			{
				vd->SaveMask(true, vd->GetCurTime(), vd->GetCurChannel());
				vd->SaveLabel(true, vd->GetCurTime(), vd->GetCurChannel());
			}
			m_save_mask = true;
			set_focus = true;
		}
		if (!wxGetKeyState(wxKeyCode('m')) &&
			m_save_mask)
			m_save_mask = false;
		//full screen
		if (wxGetKeyState(WXK_ESCAPE))
		{
			m_fullscreen_trigger.Start(10);
		}
		//brush size
		if (wxGetKeyState(wxKeyCode('[')))
		{
			ChangeBrushSize(-10);
			set_focus = true;
		}
		if (wxGetKeyState(wxKeyCode(']')))
		{
			ChangeBrushSize(10);
			set_focus = true;
		}
		//comp include
		if (wxGetKeyState(WXK_RETURN) &&
			!m_comp_include)
		{
			if (m_frame && m_frame->GetComponentDlg())
				m_frame->GetComponentDlg()->IncludeComps();
			m_comp_include = true;
			refresh = true;
			lg_changed = true;
			set_focus = true;
		}
		if (!wxGetKeyState(WXK_RETURN) &&
			m_comp_include)
			m_comp_include = false;
		//comp exclude
		if (wxGetKeyState(wxKeyCode('\\')) &&
			!m_comp_exclude)
		{
			if (m_frame && m_frame->GetComponentDlg())
				m_frame->GetComponentDlg()->ExcludeComps();
			m_comp_exclude = true;
			refresh = true;
			lg_changed = true;
			set_focus = true;
		}
		if (!wxGetKeyState(wxKeyCode('\\')) &&
			m_comp_exclude)
			m_comp_exclude = false;
		//ruler relax
		if (wxGetKeyState(wxKeyCode('r')) &&
			!m_ruler_relax)
		{
			if (m_frame && m_frame->GetMeasureDlg())
				m_frame->GetMeasureDlg()->Relax();
			m_ruler_relax = true;
			refresh = true;
			lg_changed = true;
			set_focus = true;
		}
		if (!wxGetKeyState(wxKeyCode('r')) &&
			m_ruler_relax)
			m_ruler_relax = false;

		//grow
		if ((m_int_mode == 10 ||
			m_int_mode == 12) &&
			wxGetMouseState().LeftIsDown() &&
			m_grow_on)
		{
			int sz = glbin_settings.m_ruler_size_thresh;
			//event.RequestMore();
			glbin_vol_selector.SetInitMask(2);
			mps = ScreenToClient(mps);
			glbin_vol_selector.Segment(false, true, mps.x, mps.y);
			glbin_vol_selector.SetInitMask(3);
			if (m_int_mode == 12)
			{
				glbin_seg_grow.SetVolumeData(m_cur_vol);
				glbin_seg_grow.SetIter(glbin_vol_selector.GetIter() * 3);
				glbin_seg_grow.SetSizeThresh(sz);
				glbin_seg_grow.Compute();
			}
			refresh = true;
			lg_changed = true;
			start_loop = true;
			//update
			if (glbin_brush_def.m_update_size)
				vc.insert(gstBrushCountResult);
			if (glbin_brush_def.m_update_colocal)
				vc.insert(gstColocalResult);
			if (m_int_mode == 12)
				vc.insert(gstRulerList);
		}
	}

#if defined(_WIN32)
	//vr controller, similar to xinput
	if (m_use_openxr)
	{
		glbin_xr_renderer->GetControllerStates();
		float leftx, lefty, rightx, righty;
		leftx = glbin_xr_renderer->GetControllerLeftThumbstickX();
		lefty = glbin_xr_renderer->GetControllerLeftThumbstickY();

		int nx = GetGLSize().w();
		int ny = GetGLSize().h();
		//horizontal move
		if (leftx != 0.0)
		{
			event.RequestMore(true);
			ControllerMoveHorizontal(leftx, nx, ny);
			m_interactive = true;
			m_update_rot_ctr = true;
			refresh = true;
		}
		//zoom/dolly
		if (lefty != 0.0)
		{
			event.RequestMore(true);
			ControllerZoomDolly(lefty, nx, ny);
			m_interactive = true;
			refresh = true;
			vc.insert(gstScaleFactor);
		}

		if (glbin_xr_renderer->GetGrab())
		{
			event.RequestMore(true);
			glm::mat4 rot_mat = glbin_xr_renderer->GetGrabMatrix();
			GrabRotate(rot_mat);
			m_interactive = true;
			refresh = true;
			vc.insert(gstCamRotation);
		}
		else
		{
			rightx = glbin_xr_renderer->GetControllerRightThumbstickX();
			righty = glbin_xr_renderer->GetControllerRightThumbstickY();

			//rotate
			if (rightx != 0.0 || righty != 0.0)
			{
				event.RequestMore(true);
				ControllerRotate(rightx, righty, nx, ny);
				m_interactive = true;
				refresh = true;
				vc.insert(gstCamRotation);
			}
		}
	}
#endif

#if defined(_WIN32) && defined(USE_XINPUT)
	//xinput controller
	if (m_control_connected)
	{
		//DBGPRINT(L"Idle controller\n");
		XINPUT_STATE xstate = m_controller->GetState();
		double dzone = 0.2;
		double sclr = 20;
		double leftx = double(xstate.Gamepad.sThumbLX) / 32767.0;
		if (leftx > -dzone && leftx < dzone) leftx = 0.0;
		double lefty = double(xstate.Gamepad.sThumbLY) / 32767.0;
		if (lefty > -dzone && lefty < dzone) lefty = 0.0;
		double rghtx = double(xstate.Gamepad.sThumbRX) / 32767.0;
		if (rghtx > -dzone && rghtx < dzone) rghtx = 0.0;
		double rghty = double(xstate.Gamepad.sThumbRY) / 32767.0;
		if (rghty > -dzone && rghty < dzone) rghty = 0.0;
		int px = 0;
		int py = 0;
		int inc = 5;
		if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) py = -inc;
		if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) py = inc;
		if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) px = -inc;
		if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) px = inc;

		int nx = GetGLSize().w();
		int ny = GetGLSize().h();
		//horizontal move
		if (leftx != 0.0)
		{
			event.RequestMore(true);
			ControllerMoveHorizontal(leftx * sclr, nx, ny);
			m_interactive = true;
			m_update_rot_ctr = true;
			refresh = true;
			lg_changed = true;
		}
		//zoom/dolly
		if (lefty != 0.0)
		{
			event.RequestMore(true);
			ControllerZoomDolly(lefty * sclr, nx, ny);
			m_interactive = true;
			refresh = true;
			lg_changed = true;
			vc.insert(gstScaleFactor);
		}
		//rotate
		if (rghtx != 0.0 || rghty != 0.0)
		{
			event.RequestMore(true);
			ControllerRotate(rghtx * sclr, rghty * sclr, nx, ny);
			m_interactive = true;
			refresh = true;
			lg_changed = true;
			vc.insert(gstCamRotation);
		}
		//pan
		if (px != 0 || py != 0)
		{
			event.RequestMore(true);
			ControllerPan(px, py, nx, ny);
			m_interactive = true;
			m_update_rot_ctr = true;
			refresh = true;
			lg_changed = true;
		}
	}
#endif


	if (refresh)
	{
		m_clear_buffer = true;
		m_updating = true;
		RefreshGL(15, ref_stat, start_loop, lg_changed);
		if (vc.empty())
			vc.insert(gstNull);
		m_renderview_panel->FluoRefresh(0, vc, {-1});
	}
	else if (glbin_settings.m_inf_loop)
	{
		RefreshGL(0, false, true, true);
		event.RequestMore(true);
		return;
	}
	if (set_focus)
		SetFocus();
}

void RenderCanvas::OnQuitFscreen(wxTimerEvent& event)
{
	m_fullscreen_trigger.Stop();
	if (!m_frame || !m_renderview_panel)
		return;

	m_full_screen = false;
	if (m_benchmark)
	{
		if (m_renderview_panel->m_full_frame)
			m_renderview_panel->m_full_frame->Hide();
		if (m_frame)
			m_frame->Close();
	}
	else if (GetParent() == m_renderview_panel->m_full_frame)
	{
		Reparent(m_renderview_panel);
		m_renderview_panel->m_view_sizer->Clear();
		m_renderview_panel->m_view_sizer->Add(this, 1, wxEXPAND);
		m_renderview_panel->Layout();
		m_renderview_panel->m_full_frame->Hide();
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
		RefreshGL(40);
	}
}

void RenderCanvas::OnClose(wxCloseEvent &event)
{
	if (m_full_screen)
	{
		m_fullscreen_trigger.Start(10);
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
	m_render_view->SetMousePos(event.GetPosition().x, event.GetPosition().y);

	//if (m_drawing) return;
	wxWindow *window = wxWindow::FindFocus();
	//if (window &&
	//	(window->GetClassInfo()->
	//		IsKindOf(CLASSINFO(wxTextCtrl)) ||
	//		window->GetClassInfo()->
	//		IsKindOf(CLASSINFO(wxComboBox))) &&
	//		(event.LeftDown() ||
	//			event.RightDown() ||
	//			event.MiddleDown() ||
	//			event.LeftUp() ||
	//			event.MiddleUp() ||
	//			event.RightUp() ||
	//			event.Dragging() ||
	//			event.GetWheelRotation()))
	if (window &&
		(event.LeftDown() ||
		event.RightDown() ||
		event.MiddleDown() ||
		event.LeftUp() ||
		event.MiddleUp() ||
		event.RightUp() ||
		event.Dragging() ||
		event.GetWheelRotation()))
		SetFocus();

	m_render_view->ProcessMouse();
}

