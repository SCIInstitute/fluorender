/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2024 Scientific Computing and Imaging Institute,
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
#ifndef _WXUNDOABLESCROLLBAR_H_
#define _WXUNDOABLESCROLLBAR_H_

#include <Undoable.h>
#include <wx/scrolbar.h>
#include <wx/timer.h>

class wxUndoableScrollBar : public wxScrollBar, public Undoable
{
public:
	wxUndoableScrollBar(
		wxWindow *parent,
		wxWindowID id,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxSB_HORIZONTAL,
		const wxValidator& val = wxDefaultValidator,
		const wxString& name = "wxUndoableScrollBar");

	virtual void SetMode(int val);
	virtual int GetMode() { return mode_; }
	virtual void SetScrollbar2(int position, int thumbSize, int low, int high, int pageSize);
	virtual void SetThumbPosition2(int val);
	virtual void SetValue(int val);
	virtual void ChangeValue(int val);
	virtual int GetValue();

private:
	int value_;
	int low_;
	int high_;
	int mode_;//0: normal slider; 1: jog
	wxTimer timer_;
	bool track_;

private:
	void OnTimer(wxTimerEvent& event);
	void OnTrack(wxScrollEvent& event);
	void OnLeftDown(wxMouseEvent& event);
	void OnLeftUp(wxMouseEvent& event);
	void OnRelease(wxScrollEvent& event);
	void OnLineDown(wxScrollEvent& event);
	void OnLineUp(wxScrollEvent& event);
	void OnScrollPageUp(wxScrollEvent& event);
	void OnScrollPageDown(wxScrollEvent& event);

private:
	virtual void replace(double t);
	virtual void push(double t);
	virtual void update();
};

#endif//_WXUNDOABLESCROLLBAR_H_
