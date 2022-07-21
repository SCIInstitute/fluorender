/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2022 Scientific Computing and Imaging Institute,
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
#include <RenderviewPanel.h>
#include <RenderFrame.h>

#include <wx/stdpaths.h>
#ifdef _WIN32
#include <wx/msw/private.h>
#endif
#include <png_resource.h>
#include "img/icons.h"

bool RenderCanvas::m_linked_rot = false;
RenderCanvas* RenderCanvas::m_master_linked_view = 0;

BEGIN_EVENT_TABLE(RenderCanvas, wxGLCanvas)
EVT_PAINT(RenderCanvas::OnDraw)
EVT_SIZE(RenderCanvas::OnResize)
EVT_MOUSE_EVENTS(RenderCanvas::OnMouse)
EVT_IDLE(RenderCanvas::OnIdle)
EVT_KEY_DOWN(RenderCanvas::OnKeyDown)
EVT_TIMER(ID_ftrigger, RenderCanvas::OnQuitFscreen)
EVT_CLOSE(RenderCanvas::OnClose)
END_EVENT_TABLE()

RenderCanvas::RenderCanvas(RenderFrame* frame,
	RenderviewPanel* parent,
	const wxGLAttributes& attriblist,
	wxGLContext* sharedContext,
	const wxPoint& pos,
	const wxSize& size,
	long style) :
	wxGLCanvas(parent, attriblist, wxID_ANY, pos, size, style),
	m_frame(frame),
	m_vrv(parent),
	//initializaion
	m_initialized(false),
	//timer for fullscreen
	m_fullscreen_trigger(this, ID_ftrigger),
	//touch
	m_enable_touch(false),
	m_ptr_id1(-1),
	m_ptr_id2(-1),
	m_agent(nullptr)
{
	m_glRC = sharedContext;
	m_sharedRC = m_glRC ? true : false;

	//bool bval = m_frame && m_frame->GetBenchmark();
	//m_agent->setValue(gstBenchmark, bval);

}

RenderCanvas::~RenderCanvas()
{
	//bool bval;
	//m_agent->getValue(gstBenchmark, bval);
	//if (bval)
	//{
	//	unsigned long long msec, frames;
	//	double fps;
	//	m_agent->getValue(gstBmRuntime, msec);
	//	m_agent->getValue(gstBmFrames, frames);
	//	m_agent->getValue(gstBmFps, fps);
	//	long lx, ly;
	//	m_agent->getValue(gstSizeX, lx);
	//	m_agent->getValue(gstSizeY, ly);
	//	wxString string = wxString("FluoRender has finished benchmarking.\n") +
	//		wxString("Results:\n") +
	//		wxString("Render size: ") + wxString::Format("%d X %d\n", lx, ly) +
	//		wxString("Time: ") + wxString::Format("%llu msec\n", msec) +
	//		wxString("Frames: ") + wxString::Format("%llu\n", frames) +
	//		wxString("FPS: ") + wxString::Format("%.2f", fps);
	//	wxMessageDialog *diag = new wxMessageDialog(this, string, "Benchmark Results",
	//		wxOK | wxICON_INFORMATION);
	//	diag->ShowModal();
	//}

	if (m_full_screen)
	{
		m_full_screen = false;
		m_vrv->SetCanvas(0);
		m_vrv->SetFullFrame(0);
		if (m_frame)
		{
			m_frame->ClearVrvList();
			m_frame->Close();
		}
	}

	if (!m_sharedRC)
		delete m_glRC;
}

void RenderCanvas::OnResize(wxSizeEvent& event)
{
	if (!m_agent)
		return;
	m_agent->holdoffObserverNotification();

	double dval = 1;
#ifdef _DARWIN
	dval = GetDPIScaleFactor();
#endif
	wxSize size = GetSize() * dval;
	m_vrv->UpdateScaleFactor(false);
	m_agent->changeValue(gstSizeX, long(size.x));
	m_agent->changeValue(gstSizeY, long(size.y));

	m_agent->resumeObserverNotificationAndUpdate();
	//m_agent->getObject()->RefreshGL(1);
	//RefreshGL(1);
}

void RenderCanvas::Init()
{
	if (!m_initialized)
	{
		m_agent->Init(
			(unsigned long long)GetHWND(),
			(unsigned long long)::wxGetInstance());
#ifdef _WIN32
		//check touch
		HMODULE user32 = LoadLibrary(L"user32");
		GetPI = reinterpret_cast<decltype(GetPointerInfo)*>
			(GetProcAddress(user32, "GetPointerInfo"));
		if (GetPI != NULL)
			m_enable_touch = true;
		else
			m_enable_touch = false;
#endif
		m_initialized = true;

		//glbin_stopwatch->start();
	}
}

void RenderCanvas::PickMesh()
{
	wxPoint mouse_pos = ScreenToClient(wxGetMousePosition());
	m_agent->setValue(gstMouseClientX, long(mouse_pos.x));
	m_agent->setValue(gstMouseClientY, long(mouse_pos.y));
	m_agent->getObject()->PickMesh();
	std::string str;
	m_agent->getValue(gstSelectedMshName, str);
	if (!str.empty())
	{
		//if (m_frame && m_frame->GetTree())
		//{
		//	m_frame->GetTree()->SetFocus();
		//	m_frame->GetTree()->Select(m_vrv->GetName(), str);
		//}
		m_agent->getObject()->Update(27);
	}
	else
	{
		//if (m_frame && m_frame->GetCurSelType() == 3 &&
		//	m_frame->GetTree())
		//	m_frame->GetTree()->Select(m_vrv->GetName(), "");
	}
}

void RenderCanvas::PickVolume()
{
	int kmode = wxGetKeyState(WXK_CONTROL) ? 1 : 0;
	std::string str;
	m_agent->getValue(gstSelectedVolName, str);
	fluo::Point ip;
	m_agent->getValue(gstSelPointVolume, ip);
	if (!str.empty())
	{
		//if (m_frame && m_frame->GetTree())
		//{
		//	m_frame->GetTree()->SetFocus();
		//	m_frame->GetTree()->Select(m_vrv->GetName(), str);
		//}
		//update label selection
		SetCompSelection(ip, kmode);
	}
}

void RenderCanvas::SetCompSelection(fluo::Point& p, int mode)
{
	//update selection
	m_agent->CompSelection(p, mode);
}

void RenderCanvas::OnIdle(wxIdleEvent& event)
{
	if (!m_agent)
		return;
	//key states
	wxPoint mouse_pos = wxGetMousePosition();
	wxRect view_reg = GetScreenRect();
	wxWindow *window = wxWindow::FindFocus();
	bool mouse_in = window && view_reg.Contains(mouse_pos);
	m_agent->setValue(gstMouseIn, mouse_in);
	m_agent->setValue(gstRenderviewPanelId, long(m_vrv->GetID()));
	m_agent->UpdateInput();
	m_agent->getObject()->HandleIdle();

	//full screen
	if (mouse_in && wxGetKeyState(WXK_ESCAPE))
	{
		m_fullscreen_trigger.Start(10);
	}
}

void RenderCanvas::OnKeyDown(wxKeyEvent& event)
{
	event.Skip();
}

void RenderCanvas::OnQuitFscreen(wxTimerEvent& event)
{
	m_fullscreen_trigger.Stop();
	if (!m_frame || !m_vrv)
		return;

	m_full_screen = false;
	bool bval;
	m_agent->getValue(gstBenchmark, bval);
	if (bval)
	{
		m_vrv->HideFullFrame();
		if (m_frame)
			m_frame->Close();
	}
	else if (GetParent() == m_vrv->GetFullFrame())
	{
		Reparent(m_vrv);
		m_vrv->AddCanvas(this);
		m_vrv->Layout();
		m_vrv->HideFullFrame();
		if (m_frame)
		{
#ifdef _WIN32
			//if (m_frame->GetSettingDlg() &&
			//	!m_frame->GetSettingDlg()->GetShowCursor())
			bool bval;
			glbin_root->getValue(gstShowCursor, bval);
			ShowCursor(bval);
#endif
			m_frame->Iconize(false);
			m_frame->SetFocus();
			m_frame->Raise();
			m_frame->Show();
		}
		m_agent->getObject()->Update(40);
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
	if (!m_agent)
		return;

	if (!m_glRC) return;
#ifdef _WIN32
	bool set_gl;
	m_agent->getValue(gstSetGl, set_gl);
	if (!set_gl)
	{
		SetCurrent(*m_glRC);
		m_agent->setValue(gstSetGl, true);
	}
#endif
#if defined(_DARWIN) || defined(__linux__)
	SetCurrent(*m_glRC);
#endif
	wxPaintDC dc(this);
	Init();
	m_agent->getObject()->ForceDraw();
	SwapBuffers();
	//if (!m_refresh)
	//	m_retain_finalbuffer = true;
	//else
	//	m_refresh = false;
	//ForceDraw();
}

#ifdef _WIN32
WXLRESULT RenderCanvas::MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam)
{
	//handle wacom messages
	if (message == WT_PACKET)
	{
		PACKET pkt;
		if (gpWTPacket((HCTX)lParam, wParam, &pkt))
			m_agent->getObject()->SetPressure(pkt.pkNormalPressure, pkt.pkTangentPressure);
	}

	//map touch messages to mouse events
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
	if (!m_agent)
		return;
	bool interactive, geared_rot;
	m_agent->getValue(gstInteractive, interactive);
	m_agent->getValue(gstGearedEnable, geared_rot);
	if (interactive && !geared_rot)
		return;

	//if (m_drawing) return;
	wxWindow *window = wxWindow::FindFocus();
	if (window &&
		(window->GetClassInfo()->
			IsKindOf(CLASSINFO(wxTextCtrl)) ||
			window->GetClassInfo()->
			IsKindOf(CLASSINFO(wxComboBox))) &&
			(event.LeftDown() ||
				event.RightDown() ||
				event.MiddleDown() ||
				event.LeftUp() ||
				event.MiddleUp() ||
				event.RightUp() ||
				event.Dragging() ||
				event.GetWheelRotation()))
		SetFocus();

	//set properties
	double dval = 1;
	long mx, my;
#ifdef _DARWIN
	dval = GetDPIScaleFactor();
#endif
	mx = long(event.GetX() * dval + 0.5);
	my = long(event.GetY() * dval + 0.5);
	m_agent->setValue(gstMouseX, mx);
	m_agent->setValue(gstMouseY, my);
	m_agent->setValue(gstMouseDrag, event.Dragging());
	m_agent->setValue(gstMouseWheel, long(event.GetWheelRotation()));
	m_agent->UpdateInput();
	m_agent->getObject()->HandleMouse();
}

#ifdef _WIN32
int RenderCanvas::GetPixelFormat(PIXELFORMATDESCRIPTOR *pfd)
{
	int pixelFormat = ::GetPixelFormat(m_hDC);
	if (pixelFormat == 0) return GetLastError();
	pixelFormat = DescribePixelFormat(m_hDC, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), pfd);
	if (pixelFormat == 0) return GetLastError();
	return pixelFormat;
}
#endif

