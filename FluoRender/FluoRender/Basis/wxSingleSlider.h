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
#ifndef _WXSINGLESLIDER_H_
#define _WXSINGLESLIDER_H_

#include <wx/wx.h>
#include <wx/slider.h>

class wxSingleSlider : public wxControl
{
public:
	wxSingleSlider(
		wxWindow *parent,
		wxWindowID id,
		int value, int minValue, int maxValue,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxSL_HORIZONTAL,
		const wxValidator& val = wxDefaultValidator,
		const wxString& name = "wxSingleSlider");

	int GetValue();
	bool SetValue(int val);
	int GetMax();
	int GetMin();

	wxSize DoGetBestSize();
	void OnPaint(wxPaintEvent&);
	void OnLeftDown(wxMouseEvent& event);
	void OnMotion(wxMouseEvent& event);
	void OnLeftUp(wxMouseEvent& event);
	void OnWheel(wxMouseEvent& event);
	void OnLeave(wxMouseEvent& event);

	void SetRange(int min_val, int max_val);
	void SetRangeColor(const wxColor& c);
	void DisableRangeColor();
	void SetThumbColor(const wxColor& c);
	void DisableThumbColor();

protected:
	void paintNow();
	void render(wxDC& dc);
	void DrawThumb(wxDC& dc, wxCoord x, wxCoord y, const wxColor& c);
	
private:
	wxWindow* parent_;
	wxWindowID id_;

	bool inverse_;
	bool horizontal_;
	int margin_;
	double scale_;
	int val_;
	int min_val_, max_val_;
	bool sel_;
	//int sel_, last_sel_;
	//int sel_pos_;

	bool use_range_color_;
	wxColor range_color_;
	bool use_thumb_color_;
	wxColor thumb_color_;

	int thumb_state_;//0-normal;1-mouse on;2-moving

private:
	void renderNormal(wxDC& dc);
	void renderInverse(wxDC& dc);
};

#endif//_WXSINGLESLIDER_H_
