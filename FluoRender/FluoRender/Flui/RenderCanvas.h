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

#ifndef _RENDERCANVAS_H_
#define _RENDERCANVAS_H_

#include <AgentOwner.h>
#include <Size.h>
#include <Point.h>
#include <wxBasisSlider.h>
#ifdef _WIN32
#include <wintab.h>
#endif
#include <wx/wx.h>
#include <wx/clrpicker.h>
#include <wx/spinbutt.h>
#include <wx/glcanvas.h>
#include <wx/event.h>
#include <wx/timer.h>

class RenderViewPanel;
class RenderView;
class RenderCanvasAgent;
class RenderCanvas : public AgentOwner, public wxGLCanvas
{
public:
	RenderCanvas(
		wxWindow* parent,
		wxGLContext* sharedContext = 0,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0);
	~RenderCanvas();

	virtual void CreateUI() override;

	//for degugging, this allows inspection of the pixel format actually given.
#ifdef _WIN32
	int GetPixelFormat(PIXELFORMATDESCRIPTOR *pfd);
#endif

	RenderCanvasAgent* GetRenderCanvasAgent() const;

	void SetFocusedSlider(wxBasisSlider* slider);

	void ExitFullscreen();

	//get
	double GetDpiScale();

private:
	wxGLContext* m_glRC;
	bool m_sharedRC;

	//timer for exit full screen
	wxTimer m_exit_fscreen_trigger;

#ifdef _WIN32
	//wacom support
	static HCTX m_hTab;
	static LOGCONTEXTA m_lc;
	decltype(GetPointerInfo)* GetPI;
#endif

	//touch pointer ids
	bool m_enable_touch;
	int m_ptr_id1;
	int m_ptr1_x;
	int m_ptr1_y;
	int m_ptr1_x_save;
	int m_ptr1_y_save;
	int m_ptr_id2;
	int m_ptr2_x;
	int m_ptr2_y;
	int m_ptr2_x_save;
	int m_ptr2_y_save;

	//is full screen
	bool m_full_screen;

	//focused slider
	wxBasisSlider* m_focused_slider;

private:
#ifdef _WIN32
	//wacom tablet
	HCTX TabletInit(HWND hWnd, HINSTANCE hInst);
	WXLRESULT MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam);
#endif

	//system call
	void OnMouse(wxMouseEvent& event);
	void OnDraw(wxPaintEvent& event);
	void OnResize(wxSizeEvent& event);
	void OnMove(wxMoveEvent& event);
	void OnIdle(wxIdleEvent& event);
	void OnQuitFscreen(wxTimerEvent& event);
	void OnClose(wxCloseEvent& event);
};

#endif//_RENDERCANVAS_H_
