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
	wxBasisSlider(parent, id, pos, size, style, val, name)
{
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

bool wxSingleSlider::setValue(int val, bool notify)
{
	int old = val_;
	val_ = val < min_val_ ? min_val_ :
		(val > max_val_ ? max_val_ : val);
	bool changed = old != val_;
	if (!changed)
		return changed;
	else
	{
		double t;
		if (time_sample(t))
			push(t);
		else
			replace(t);
	}

	Refresh();
	Update();

	if (notify)
	{
		wxCommandEvent e(wxEVT_SCROLL_CHANGED, id_);
		e.SetEventObject(this);
		e.SetString("update");
		ProcessWindowEvent(e);
		wxPostEvent(parent_, e);
	}

	return changed;
}

bool wxSingleSlider::SetValue(int val)
{
	return setValue(val);
}

bool wxSingleSlider::ChangeValue(int val)
{
	return setValue(val, false);
}

int wxSingleSlider::GetValue()
{
	return val_;
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

	setValue(val);

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
		bool changed = setValue(val);
	}

	event.Skip();
}

void wxSingleSlider::OnLeftUp(wxMouseEvent& event)
{
	thumb_state_ = 0;
	wxBasisSlider::OnLeftUp(event);
}

void wxSingleSlider::OnLeave(wxMouseEvent& event)
{
	thumb_state_ = 0;
	wxBasisSlider::OnLeave(event);
}

void wxSingleSlider::SetThumbColor(const wxColor& c)
{
	thumb_color_ = c;
	wxBasisSlider::SetThumbColor(c);
}

void wxSingleSlider::Scroll(int val)
{
	SetValue(val_ + val);
	wxBasisSlider::Scroll(val);
}

void wxSingleSlider::replace(double t)
{
	if (stack_.empty())
		return;
	stack_[stack_pointer_] = std::pair<double, int>(t, val_);
}

void wxSingleSlider::push(double t)
{
	size_t size = stack_.size();
	int val = size ? std::any_cast<int>(stack_[stack_pointer_].second) : 0;
	if (!size || val_ != val)
	{
		if (!size ||
			stack_pointer_ == size - 1)
			stack_.push_back(std::pair<double, int>(t, val_));
		else
			stack_.insert(stack_.begin() + stack_pointer_, std::pair<double, int>(t, val_));
		stack_pointer_++;
		//DBGPRINT(L"\tsize:%d,pointer:%d,last:(%f, %d)\n",
		//	stack_.size(), stack_pointer_, stack_.back().first,
		//	std::any_cast<int>(stack_.back().second));
	}
}

void wxSingleSlider::update()
{
	int val = std::any_cast<int>(stack_[stack_pointer_].second);
	setValue(val);
}
