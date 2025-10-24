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

#ifndef _RENDERCANVAS_H_
#define _RENDERCANVAS_H_

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

class MainFrame;
class RenderViewPanel;
class RenderView;
class RenderCanvas : public wxGLCanvas
{
public:
	RenderCanvas(MainFrame* frame,
		RenderViewPanel* parent,
		const wxGLAttributes& attriblist,
		wxGLContext* sharedContext = 0,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0);
	~RenderCanvas();

	//for degugging, this allows inspection of the pixel format actually given.
#ifdef _WIN32
	int GetPixelFormat(PIXELFORMATDESCRIPTOR *pfd);
#endif

	RenderView* GetRenderView()
	{
		if (auto view_ptr = m_renderview.lock())
			return view_ptr.get();
		return 0;
	}

	//get view info for external ops
	//get size, considering enlargement
	inline fluo::Point GetMousePos(wxMouseEvent& e);

	void SetFocusedSlider(wxBasisSlider* slider);

	void Draw();

public:
	RenderViewPanel* m_renderview_panel;

private:
	wxGLContext* m_glRC;
	bool m_sharedRC;
	MainFrame* m_frame;
	//previous focus before brush
	wxWindow* m_prev_focus;
	//render view
	std::weak_ptr<RenderView> m_renderview;

	//timer for exit full screen
	wxTimer m_exit_fscreen_trigger;

	//wacom support
#ifdef _WIN32
	static HCTX m_hTab;
	static LOGCONTEXTA m_lc;
#endif

	//touch pointer ids
	bool m_enable_touch;
#ifdef _WIN32
	decltype(GetPointerInfo)* GetPI;
#endif
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
	void Init();
#ifdef _WIN32
	//wacom tablet
	HCTX TabletInit(HWND hWnd, HINSTANCE hInst);
#endif

	//system call
	void OnMouse(wxMouseEvent& event);
	void OnDraw(wxPaintEvent& event);
	void OnResize(wxSizeEvent& event);
	void OnMove(wxMoveEvent& event);
	void OnIdle(wxIdleEvent& event);
	void OnQuitFscreen(wxTimerEvent& event);
	void OnClose(wxCloseEvent& event);
#ifdef _WIN32
	WXLRESULT MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam);
#endif

	friend class RenderViewPanel;
};

#endif//_RENDERCANVAS_H_
