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
#include "wxBasisSlider.h"
#include "Debug.h"

wxBasisSlider::wxBasisSlider(
	wxWindow *parent,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxValidator& val,
	const wxString& name):
	parent_(parent),
	id_(id),
#ifdef _WIN32
	thumb_style_(0),
#else
	thumb_style_(1),
#endif
	enabled_(true),
	sel_(0),
	use_range_color_(true),
	range_color_(wxColor(75, 154, 255)),
	use_thumb_color_(false),
	horizontal_(!(style & wxSL_VERTICAL)),
	inverse_(style & wxSL_INVERSE),
	range_style_(0),
#ifdef _WIN32
	wxControl(parent, id, pos,
		wxSize(std::max(size.GetWidth(), int(std::round(24 * parent->GetDPIScaleFactor()))),
			std::max(size.GetHeight(), int(std::round(24 * parent->GetDPIScaleFactor())))),
		wxBORDER_NONE, val, name),
#else
	wxControl(parent, id, pos,
		wxSize(std::max(size.GetWidth(), 24),
			std::max(size.GetHeight(), 24)),
		wxBORDER_NONE, val, name),
#endif
	Undoable()
{
	scale_ = thumb_style_ ? 1 : parent->GetDPIScaleFactor();
	margin_ = std::round(12 * scale_);
	SetBackgroundColour(parent->GetBackgroundColour());
	SetDoubleBuffered(true);
}

wxSize wxBasisSlider::DoGetBestSize()
{
	if (horizontal_)
		return (parent_->FromDIP(wxSize(200,24)));
	return (parent_->FromDIP(wxSize(24, 200)));
}

void wxBasisSlider::OnPaint(wxPaintEvent&)
{
	wxPaintDC dc(this);
	render(dc);
}

void wxBasisSlider::paintNow()
{
	wxClientDC dc(this);
	render(dc);
}

void wxBasisSlider::render(wxDC& dc)
{
	if (inverse_)
		renderInverse(dc);
	else
		renderNormal(dc);
}

void  wxBasisSlider::DrawThumb(wxDC& dc, wxCoord x, wxCoord y, const wxColor& c)
{
	if (thumb_style_ == 0)
	{
		wxPoint ph[] = {
			wxPoint(x - 5 * scale_, y - 7 * scale_),
			wxPoint(x + 5 * scale_, y - 7 * scale_),
			wxPoint(x + 5 * scale_, y + 6 * scale_),
			wxPoint(x, y + 11 * scale_),
			wxPoint(x - 5 * scale_, y + 6 * scale_)
		};
		wxPoint pv[] = {
			wxPoint(x - 7 * scale_, y - 5 * scale_),
			wxPoint(x - 7 * scale_, y + 5 * scale_),
			wxPoint(x + 6 * scale_, y + 5 * scale_),
			wxPoint(x + 11 * scale_, y),
			wxPoint(x + 6 * scale_, y - 5 * scale_)
		};
		int num = 5;

		dc.SetPen(c);
		dc.SetBrush(wxBrush(c, wxBRUSHSTYLE_SOLID));
		dc.DrawPolygon(num, horizontal_ ? ph : pv, wxODDEVEN_RULE);
	}
	else
	{
		dc.SetPen(*wxWHITE);
		//dc.SetBrush(wxBrush(*wxWHITE, wxBRUSHSTYLE_SOLID));
		int xx = horizontal_ ? x : x + scale_;
		int yy = horizontal_ ? y + scale_ : y;
		//dc.DrawCircle(xx, yy, 7 * scale_);
		//dc.SetPen(c);
		dc.SetBrush(wxBrush(c, wxBRUSHSTYLE_SOLID));
		dc.DrawCircle(xx, yy, 9 * scale_);
	}
}

void wxBasisSlider::OnLeftDown(wxMouseEvent& event)
{
	event.Skip();
}

void wxBasisSlider::OnMotion(wxMouseEvent& event)
{
	event.Skip();
}

void wxBasisSlider::OnLeftUp(wxMouseEvent& event)
{
	if (HasCapture())
		ReleaseMouse();
	event.Skip();

	if (sel_)
	{
		sel_ = 0;
		//thumb_state_ = 0;
		Refresh();
		Update();
		wxCommandEvent e(wxEVT_SCROLL_CHANGED, id_);
		e.SetEventObject(this);
		e.SetString("update");
		ProcessWindowEvent(e);
		wxPostEvent(parent_, e);

	}
}

void wxBasisSlider::OnLeave(wxMouseEvent& event)
{
	//thumb_state_ = 0;
	Refresh();
	Update();
	event.Skip();
}

void wxBasisSlider::OnWheel(wxMouseEvent& event)
{
	int m;
	if (event.GetWheelRotation() > 0)
		m = inverse_ == horizontal_ ? -1 : 1;
	else
		m = inverse_ == horizontal_ ? 1 : -1;

	Scroll(m);
	event.Skip();
}

void wxBasisSlider::SetRangeStyle(int val)
{
	range_style_ = val;
}

void wxBasisSlider::SetRange(int min_val, int max_val)
{
	min_val_ = min_val;
	max_val_ = max_val;
	Refresh();
	Update();
}

int wxBasisSlider::GetMax()
{
	return max_val_;
}

int wxBasisSlider::GetMin()
{
	return min_val_;
}

void wxBasisSlider::SetRangeColor(const wxColor& c)
{
	range_color_ = c;
	use_range_color_ = true;
}

void wxBasisSlider::DisableRangeColor()
{
	use_range_color_ = false;
}

void wxBasisSlider::SetThumbColor(const wxColor& c)
{
	//thumb_color_ = c;
	use_thumb_color_ = true;
}

void wxBasisSlider::DisableThumbColor()
{
	use_thumb_color_ = false;
}

bool wxBasisSlider::Disable()
{
	bool val = wxControl::Disable();
	enabled_ = false;
	Refresh();
	Update();
	return val;
}

bool wxBasisSlider::Enable(bool enable)
{
	bool val = wxControl::Enable(enable);
	enabled_ = enable;
	Refresh();
	Update();
	return val;
}

void wxBasisSlider::Scroll(int val)
{
	wxCommandEvent e(wxEVT_SCROLL_CHANGED, id_);
	e.SetEventObject(this);
	e.SetString("update");
	ProcessWindowEvent(e);
	wxPostEvent(parent_, e);
}
