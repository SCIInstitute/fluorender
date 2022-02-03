/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2018 Scientific Computing and Imaging Institute,
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

#ifndef _VRENDERGLVIEW_H_
#define _VRENDERGLVIEW_H_

#include <RenderCanvasAgent.hpp>
#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <wx/gdicmn.h>

#define ID_ftrigger	ID_VRENDER_VIEW1

class VRenderFrame;
class VRenderView;
class RenderCanvas : public wxGLCanvas
{
public:
	RenderCanvas(VRenderFrame* frame,
		VRenderView* parent,
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
	//initialization
	void Init();

	//public mouse
	void OnMouse(wxMouseEvent& event);

private:
	fluo::RenderCanvasAgent* m_agent;
	VRenderFrame* m_frame;
	VRenderView* m_vrv;
	//set gl context
	bool m_set_gl;

	//linked rotation
	static bool m_linked_rot;
	static RenderCanvas* m_master_linked_view;

	//initializaion
	bool m_initialized;

	wxSize m_size;
	wxGLContext* m_glRC;
	bool m_sharedRC;

	//previous focus before brush
	wxWindow* m_prev_focus;

	//timer for full screen
	wxTimer m_fullscreen_trigger;

	//is full screen
	bool m_full_screen;

private:
	void ChangeBrushSize(int value);
	void UpdateBrushState();
	void PickMesh();
	void PickVolume();
	void SetCompSelection(fluo::Point& p, int mode);
	//system call
	void OnDraw(wxPaintEvent& event);
	void OnResize(wxSizeEvent& event);
	void OnIdle(wxIdleEvent& event);
	void OnKeyDown(wxKeyEvent& event);
	void OnQuitFscreen(wxTimerEvent& event);
	void OnClose(wxCloseEvent& event);
#ifdef _WIN32
	WXLRESULT MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam);
#endif
	DECLARE_EVENT_TABLE()

	friend class VRenderView;
	friend class fluo::RenderCanvasAgent;
};

#endif//_VRENDERGLVIEW_H_
