﻿/*
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
#ifndef _WXDOUBLESLIDER_H_
#define _WXDOUBLESLIDER_H_

#include <wxBasisSlider.h>

class wxDoubleSlider : public wxBasisSlider
{
public:
	wxDoubleSlider(
		wxWindow *parent,
		wxWindowID id,
		int lowValue, int hiValue, int minValue, int maxValue,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxSL_HORIZONTAL,
		const wxValidator& val = wxDefaultValidator,
		const wxString& name = "wxDoubleSlider");

	void SetLink(bool val);
	bool GetLink();
	void SetLinkDist(int val);
	int GetLinkDist();

	int GetLowValue();
	int GetHighValue();
	bool SetLowValue(int val);
	bool SetHighValue(int val);
	bool ChangeLowValue(int val);
	bool ChangeHighValue(int val);
	bool ChangeValues(int &low, int &hi);

	virtual void OnLeftDown(wxMouseEvent& event);
	virtual void OnMotion(wxMouseEvent& event);
	virtual void OnLeftUp(wxMouseEvent& event);
	virtual void OnLeave(wxMouseEvent& event);

	virtual void SetRange(int min_val, int max_val);
	virtual void SetThumbColor(const wxColor& c1, const wxColor& c2);

	virtual void Scroll(int val);

protected:
	bool link_;
	int low_val_, hi_val_;
	int link_dist_;
	int last_sel_;
	int sel_val_;

	wxColor thumb_color1_;
	wxColor thumb_color2_;

	int thumb_state1_;//0-normal;1-mouse on;2-moving
	int thumb_state2_;//0-normal;1-mouse on;2-moving

	//struct stored in stack
	struct ValueDSL
	{
		ValueDSL(int v1, int v2) { lv = v1; hv = v2; }
		ValueDSL(ValueDSL const& v) { lv = v.lv; hv = v.hv; }
		ValueDSL& operator=(ValueDSL const& v) { lv = v.lv; hv = v.hv; return *this; }
		bool Equals(const ValueDSL& v) const { return lv == v.lv && hv == v.hv; }
		int lv;
		int hv;
	};

protected:
	virtual void renderNormal(wxDC& dc);
	virtual void renderInverse(wxDC& dc);

	bool setLowValue(int val, bool notify = true);
	bool setHighValue(int val, bool notify = true);

	virtual void replace(double t);
	virtual void push(double t);
	virtual void update();
};

#endif//_WXDOUBLESLIDER_H_
