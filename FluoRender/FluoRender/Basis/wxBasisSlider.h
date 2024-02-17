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
#ifndef _WXBASISSLIDER_H_
#define _WXBASISSLIDER_H_

#include <wx/wx.h>
#include <wx/slider.h>
#include <chrono>

class wxBasisSlider : public wxControl
{
public:
	wxBasisSlider(
		wxWindow *parent,
		wxWindowID id,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxSL_HORIZONTAL,
		const wxValidator& val = wxDefaultValidator,
		const wxString& name = "wxBasisSlider");

	virtual int GetMax();
	virtual int GetMin();

	virtual wxSize DoGetBestSize();
	virtual void OnPaint(wxPaintEvent&);
	virtual void OnLeftDown(wxMouseEvent& event);
	virtual void OnMotion(wxMouseEvent& event);
	virtual void OnLeftUp(wxMouseEvent& event);
	virtual void OnWheel(wxMouseEvent& event);
	virtual void OnLeave(wxMouseEvent& event);

	virtual void SetRangeStyle(int val);
	virtual void SetRange(int min_val, int max_val);
	virtual void SetRangeColor(const wxColor& c);
	virtual void DisableRangeColor();
	virtual void SetThumbColor(const wxColor& c);
	virtual void DisableThumbColor();

	virtual bool Disable();
	virtual bool Enable(bool enable = true);

	virtual void Scroll(int val);

	virtual void Undo();
	virtual void Redo();

protected:
	wxWindow* parent_;
	wxWindowID id_;

	bool enabled_;
	bool inverse_;
	bool horizontal_;
	int margin_;
	double scale_;
	int min_val_, max_val_;
	int sel_;

	int range_style_;//0-low value;1-high value;2-center
	bool use_range_color_;
	wxColor range_color_;
	bool use_thumb_color_;

	int thumb_style_;//0-windows;1-others

	//value stack
	size_t stack_pointer_;
	size_t stack_size_;

	//timer
	std::chrono::high_resolution_clock::time_point time_;
	static double time_span_;

protected:
	virtual void paintNow();
	virtual void render(wxDC& dc);
	virtual void DrawThumb(wxDC& dc, wxCoord x, wxCoord y, const wxColor& c);
	virtual void renderNormal(wxDC& dc) = 0;
	virtual void renderInverse(wxDC& dc) = 0;

	virtual void replace() = 0;
	virtual void push() = 0;
	virtual void pop() = 0;
	virtual void backward() = 0;
	virtual void forward() = 0;

	bool time_sample();
};

#endif//_WXBASISSLIDER_H_
