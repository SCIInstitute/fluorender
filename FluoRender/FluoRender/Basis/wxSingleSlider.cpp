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
#include "wxSingleSlider.h"
#include "Debug.h"

wxSingleSlider::wxSingleSlider(
	wxWindow *parent,
	wxWindowID id,
	int value, int minValue, int maxValue,
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
	sel_(false),
	use_range_color_(true),
	range_color_(wxColor(75, 154, 255)),
	use_thumb_color_(false),
	horizontal_(!(style & wxSL_VERTICAL)),
	inverse_(style & wxSL_INVERSE),
	range_style_(0),
	wxControl(parent, id, pos,
		wxSize(std::max(size.GetWidth(), int(std::round(24 * parent->GetDPIScaleFactor()))),
			std::max(size.GetHeight(), int(std::round(24 * parent->GetDPIScaleFactor())))),
		wxBORDER_NONE, val, name)
{
	scale_ = parent->GetDPIScaleFactor();
	margin_ = int(std::round(12 * scale_));
	SetBackgroundColour(parent->GetBackgroundColour());
	SetDoubleBuffered(true);
	val_ = value;
	min_val_ = minValue;
	max_val_ = maxValue;
	thumb_state_ = 0;
	Bind(wxEVT_PAINT, &wxSingleSlider::OnPaint, this);
	Bind(wxEVT_LEFT_DOWN,&wxSingleSlider::OnLeftDown, this);
	Bind(wxEVT_MOTION,&wxSingleSlider::OnMotion, this);
	Bind(wxEVT_LEFT_UP,&wxSingleSlider::OnLeftUp, this);
	Bind(wxEVT_LEAVE_WINDOW, &wxSingleSlider::OnLeave, this);
	Bind(wxEVT_MOUSEWHEEL, &wxSingleSlider::OnWheel, this);
	Refresh();
	Update();
}

bool wxSingleSlider::SetValue(int val)
{
	int old = val_;
	val_ = val < min_val_ ? min_val_ :
		(val > max_val_ ? max_val_ : val);
	bool changed = old != val_;
	if (!changed)
		return changed;
	Refresh();
	Update();
	return changed;
}

int wxSingleSlider::GetValue()
{
	return val_;
}

wxSize wxSingleSlider::DoGetBestSize()
{
	if (horizontal_)
		return (parent_->FromDIP(wxSize(200,24)));
	return (parent_->FromDIP(wxSize(24, 200)));
}

void wxSingleSlider::OnPaint(wxPaintEvent&)
{
	wxPaintDC dc(this);
	render(dc);
}

void wxSingleSlider::paintNow()
{
	wxClientDC dc(this);
	render(dc);
}

void  wxSingleSlider::DrawThumb(wxDC& dc, wxCoord x, wxCoord y, const wxColor& c)
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
		dc.DrawCircle(xx, yy, 6 * scale_);
	}
}

void wxSingleSlider::render(wxDC& dc)
{
	if (inverse_)
		renderInverse(dc);
	else
		renderNormal(dc);
}

void wxSingleSlider::renderNormal(wxDC& dc)
{
	int w, h;
	GetSize(&w, &h);

	dc.SetPen(*wxGREY_PEN);
	dc.SetBrush(*wxLIGHT_GREY_BRUSH);
	if (horizontal_)
	{
		int a, b, c, d, e, f;
		a = std::round(h / 2.0 - scale_);
		b = std::round(w - margin_ + 1.0 + 1.5 * scale_);
		c = std::round(h / 2.0 - 0.2 * scale_);
		d = std::round(h / 2.0 + scale_ + 0.5 * scale_);
		e = std::round(h / 2.0 + 2.0 * scale_);
		f = std::round(margin_ - 1.5 * scale_);
		wxPoint p1h[] = {
			wxPoint(margin_, a),
			wxPoint(w - margin_ + 1, a),
			wxPoint(b, c),
			wxPoint(b, d),
			wxPoint(w - margin_ + 1, e),
			wxPoint(margin_, e),
			wxPoint(f, d),
			wxPoint(f, c)
		};
		dc.DrawPolygon(8, p1h, wxODDEVEN_RULE);
	}
	else
	{
		int a, b, c, d, e, f;
		a = std::round(w / 2.0 - scale_);
		b = std::round(h - margin_ + 1.0 + 1.5 * scale_);
		c = std::round(w / 2.0 - 0.2 * scale_);
		d = std::round(w / 2.0 + scale_ + 0.5 * scale_);
		e = std::round(w / 2.0 + 2.0 * scale_);
		f = std::round(margin_ - 1.5 * scale_);
		wxPoint p1v[] = {
			wxPoint(a, margin_),
			wxPoint(a, h - margin_ + 1),
			wxPoint(c, b),
			wxPoint(d, b),
			wxPoint(e, h - margin_ + 1),
			wxPoint(e, margin_),
			wxPoint(d, f),
			wxPoint(c, f)
		};
		dc.DrawPolygon(8, p1v, wxODDEVEN_RULE);
	}

	//left slider:
	int posl = 0;
	if (range_style_ == 1)
		posl = (horizontal_ ? w : h) - margin_ * 2;
	else if (range_style_ == 2)
		posl = ((horizontal_ ? w : h) - margin_ * 2) / 2;
	//right slider:
	int posr = std::min(max_val_, val_);
	posr = std::round(double(posr - min_val_) * ((horizontal_ ? w : h) - margin_ * 2) / (max_val_ - min_val_));

	wxColor color;
	//range color
	if (use_range_color_)
	{
		color = enabled_ ? range_color_ : *wxBLACK;
		dc.SetPen(color);
		dc.SetBrush(color);
		if (horizontal_)
		{
			int a, b;
			a = std::round(h / 2.0 - scale_);
			b = std::round(h / 2.0 + 2.0 * scale_);
			wxPoint p2h[] = {
				wxPoint(margin_ + posl, a),
				wxPoint(margin_ + posr, a),
				wxPoint(margin_ + posr, b),
				wxPoint(margin_ + posl, b)
			};
			dc.DrawPolygon(4, p2h, wxODDEVEN_RULE);
		}
		else
		{
			int a, b;
			a = std::round(w / 2.0 - scale_);
			b = std::round(w / 2.0 + 2.0 * scale_);
			wxPoint p2v[] = {
				wxPoint(a, margin_ + posl),
				wxPoint(a, margin_ + posr),
				wxPoint(b, margin_ + posr),
				wxPoint(b, margin_ + posl)
			};
			dc.DrawPolygon(4, p2v, wxODDEVEN_RULE);
		}
	}

	if (enabled_)
	{
		color = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
		switch (thumb_state_)
		{
		case 0:
			if (use_thumb_color_)
				color = thumb_color_;
			break;
		case 1:
			color = wxColor(128, 128, 128);
			break;
		case 2:
			if (use_thumb_color_)
				color = thumb_color_;
			else
				color = *wxLIGHT_GREY;
			break;
		}
	}
	else
		color = *wxLIGHT_GREY;

	if (horizontal_)
	{
		DrawThumb(dc, margin_ + posr, h * 0.5, color);
	}
	else
	{
		DrawThumb(dc, w * 0.5, margin_ + posr, color);
	}
}

void wxSingleSlider::renderInverse(wxDC& dc)
{
	int w, h;
	GetSize(&w, &h);

	dc.SetPen(*wxGREY_PEN);
	dc.SetBrush(*wxLIGHT_GREY_BRUSH);
	if (horizontal_)
	{
		int a, b, c, d, e, f;
		a = std::round(h / 2.0 - scale_);
		b = std::round(w - margin_ + 1.0 + 1.5 * scale_);
		c = std::round(h / 2.0 - 0.2 * scale_);
		d = std::round(h / 2.0 + scale_ + 0.5 * scale_);
		e = std::round(h / 2.0 + 2.0 * scale_);
		f = std::round(margin_ - 1.5 * scale_);
		wxPoint p1h[] = {
			wxPoint(margin_, a),
			wxPoint(w - margin_ + 1, a),
			wxPoint(b, c),
			wxPoint(b, d),
			wxPoint(w - margin_ + 1, e),
			wxPoint(margin_, e),
			wxPoint(f, d),
			wxPoint(f, c)
		};
		dc.DrawPolygon(8, p1h, wxODDEVEN_RULE);
	}
	else
	{
		int a, b, c, d, e, f;
		a = std::round(w / 2.0 - scale_);
		b = std::round(h - margin_ + 1.0 + 1.5 * scale_);
		c = std::round(w / 2.0 - 0.2 * scale_);
		d = std::round(w / 2.0 + scale_ + 0.5 * scale_);
		e = std::round(w / 2.0 + 2.0 * scale_);
		f = std::round(margin_ - 1.5 * scale_);
		wxPoint p1v[] = {
			wxPoint(a, margin_),
			wxPoint(a, h - margin_ + 1),
			wxPoint(c, b),
			wxPoint(d, b),
			wxPoint(e, h - margin_ + 1),
			wxPoint(e, margin_),
			wxPoint(d, f),
			wxPoint(c, f)
		};
		dc.DrawPolygon(8, p1v, wxODDEVEN_RULE);
	}

	//left slider:
	int posl = 0;
	if (range_style_ == 1)
		posl = (horizontal_ ? w : h) - margin_ * 2;
	else if (range_style_ == 2)
		posl = ((horizontal_ ? w : h) - margin_ * 2) / 2;
	//right slider:
	int posr = std::min(max_val_, val_);
	posr = std::round(double(posr - min_val_) * ((horizontal_ ? w : h) - margin_ * 2) / (max_val_ - min_val_));

	wxColor color;
	//range color
	if (use_range_color_)
	{
		color = enabled_ ? range_color_ : *wxBLACK;
		dc.SetPen(color);
		dc.SetBrush(color);
		if (horizontal_)
		{
			int a, b;
			a = std::round(h / 2.0 - scale_);
			b = std::round(h / 2.0 + 2.0 * scale_);
			wxPoint p2h[] = {
			wxPoint(w - margin_ - posl, a),
			wxPoint(w - margin_ - posr, a),
			wxPoint(w - margin_ - posr, b),
			wxPoint(w - margin_ - posl, b)
			};
			dc.DrawPolygon(4, p2h, wxODDEVEN_RULE);
		}
		else
		{
			int a, b;
			a = std::round(w / 2.0 - scale_);
			b = std::round(w / 2.0 + 2.0 * scale_);
			wxPoint p2v[] = {
			wxPoint(a, h - margin_ - posl),
			wxPoint(a, h - margin_ - posr),
			wxPoint(b, h - margin_ - posr),
			wxPoint(b, h - margin_ - posl)
			};
			dc.DrawPolygon(4, p2v, wxODDEVEN_RULE);
		}
	}

	if (enabled_)
	{
		color = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
		switch (thumb_state_)
		{
		case 0:
			if (use_thumb_color_)
				color = thumb_color_;
			break;
		case 1:
			color = wxColor(128, 128, 128);
			break;
		case 2:
			if (use_thumb_color_)
				color = thumb_color_;
			else
				color = *wxLIGHT_GREY;
			break;
		}
	}
	else
		color = *wxLIGHT_GREY;

	if (horizontal_)
	{
		DrawThumb(dc, w - margin_ - posr, h * 0.5, color);
	}
	else
	{
		DrawThumb(dc, w * 0.5, h - margin_ - posr, color);
	}
}

void wxSingleSlider::OnLeftDown(wxMouseEvent& event)
{
	SetFocus();
	CaptureMouse();

	int w, h;
	wxClientDC dc(this);
	dc.GetSize(&w, &h);
	wxPoint pos = event.GetLogicalPosition(dc);

	sel_ = true;

	int val;
	int posl = margin_;
	int posr = (horizontal_ ? w : h) - margin_;
	if (inverse_)
		val = std::round(double(min_val_) + double(max_val_ - min_val_) *
			(posr - (horizontal_ ? pos.x : pos.y)) / (posr - posl));
	else
		val = std::round(double(min_val_) + double(max_val_ - min_val_) *
			((horizontal_ ? pos.x : pos.y) - posl) / (posr - posl));

	SetValue(val);

	event.Skip();
}

void wxSingleSlider::OnMotion(wxMouseEvent& event)
{
	int w, h;
	wxClientDC dc(this);
	dc.GetSize(&w, &h);
	wxPoint pos = event.GetLogicalPosition(dc);

	if (!sel_)
	{
		//left slider:
		int posl = 0;
		//right slider:
		int posr = std::min(max_val_, val_);
		posr = std::round(double(posr - min_val_) * ((horizontal_ ? w : h) - margin_ * 2) / (max_val_ - min_val_));
		wxRect rect;
		if (inverse_)
		{
			if (horizontal_)
			{
				rect = wxRect(w - margin_ - posr - 5 * scale_,
					h * 0.5 - 7 * scale_,
					10 * scale_, 13 * scale_);
			}
			else
			{
				rect = wxRect(w * 0.5 - 7 * scale_,
					h - margin_ - posr - 5 * scale_,
					13 * scale_, 10 * scale_);
			}
		}
		else
		{
			if (horizontal_)
			{
				rect = wxRect(margin_ + posr - 5 * scale_,
					h * 0.5 - 7 * scale_,
					10 * scale_, 13 * scale_);
			}
			else
			{
				rect = wxRect(w * 0.5 - 7 * scale_,
					margin_ + posr - 5 * scale_,
					13 * scale_, 10 * scale_);
			}
		}
		bool refresh = false;
		if (thumb_state_ == 0 && rect.Contains(pos))
		{
			thumb_state_ = 1;
			refresh = true;
		}
		if (thumb_state_ == 1 && !rect.Contains(pos))
		{
			thumb_state_ = 0;
			refresh = true;
		}
		if (refresh)
		{
			Refresh();
			Update();
		}
	}
	else
	{
		int posl = margin_;
		int posr = (horizontal_ ? w : h) - margin_;
		int val;
		if (inverse_)
			val = std::round(double(min_val_) + double(max_val_ - min_val_) *
				(posr - (horizontal_ ? pos.x : pos.y)) / (posr - posl));
		else
			val = std::round(double(min_val_) + double(max_val_ - min_val_) *
				((horizontal_ ? pos.x : pos.y) - posl) / (posr - posl));

		thumb_state_ = 2;
		bool changed = SetValue(val);
		//if (!changed && low_value_ < max_)

		wxCommandEvent e(wxEVT_SCROLL_CHANGED, id_);
		e.SetEventObject(this);
		e.SetString("update");
		ProcessWindowEvent(e);
		wxPostEvent(parent_, e);
	}

	event.Skip();
}

void wxSingleSlider::OnLeftUp(wxMouseEvent& event)
{
	if (HasCapture())
		ReleaseMouse();
	event.Skip();

	if (sel_)
	{
		sel_ = false;
		thumb_state_ = 0;
		Refresh();
		Update();
		wxCommandEvent e(wxEVT_SCROLL_CHANGED, id_);
		e.SetEventObject(this);
		e.SetString("update");
		ProcessWindowEvent(e);
		wxPostEvent(parent_, e);

	}
}

void wxSingleSlider::OnLeave(wxMouseEvent& event)
{
	thumb_state_ = 0;
	Refresh();
	Update();
	event.Skip();
}

void wxSingleSlider::OnWheel(wxMouseEvent& event)
{
	int m, w, h;
	wxClientDC dc(this);
	dc.GetSize(&w, &h);
	wxPoint pos = event.GetLogicalPosition(dc);

	if (event.GetWheelRotation() > 0)
		m = inverse_ == horizontal_ ? -1 : 1;
	else
		m = inverse_ == horizontal_ ? 1 : -1;

	SetValue(val_ + m);

	wxCommandEvent e(wxEVT_SCROLL_CHANGED, id_);
	e.SetEventObject(this);
	e.SetString("update");
	ProcessWindowEvent(e);
	wxPostEvent(parent_, e);
	event.Skip();
}

void wxSingleSlider::SetRangeStyle(int val)
{
	range_style_ = val;
}

void wxSingleSlider::SetRange(int min_val, int max_val)
{
	min_val_ = min_val;
	max_val_ = max_val;
	Refresh();
	Update();
}

int wxSingleSlider::GetMax()
{
	return max_val_;
}

int wxSingleSlider::GetMin()
{
	return min_val_;
}

void wxSingleSlider::SetRangeColor(const wxColor& c)
{
	range_color_ = c;
	use_range_color_ = true;
}

void wxSingleSlider::DisableRangeColor()
{
	use_range_color_ = false;
}

void wxSingleSlider::SetThumbColor(const wxColor& c)
{
	thumb_color_ = c;
	use_thumb_color_ = true;
}

void wxSingleSlider::DisableThumbColor()
{
	use_thumb_color_ = false;
}

bool wxSingleSlider::Disable()
{
	bool val = wxControl::Disable();
	enabled_ = false;
	Refresh();
	Update();
	return val;
}

bool wxSingleSlider::Enable(bool enable)
{
	bool val = wxControl::Enable(enable);
	enabled_ = enable;
	Refresh();
	Update();
	return val;
}
