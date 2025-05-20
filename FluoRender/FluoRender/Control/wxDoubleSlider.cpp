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
#include <wxDoubleSlider.h>
#include <HistoryIndicator.h>
#include <Debug.h>

wxDoubleSlider::wxDoubleSlider(
	wxWindow *parent,
		wxWindowID id,
		int lowValue, int hiValue, int minValue, int maxValue,
		const wxPoint& pos,
		const wxSize& size,
		long style,
		const wxValidator& val,
		const wxString& name):
	link_(false),
	wxBasisSlider(parent, id, pos, size, style, val, name)
{
	low_val_ = lowValue; hi_val_ = hiValue;
	min_val_ = minValue; max_val_ = maxValue;
	last_sel_ = 1;
	thumb_state1_ = 0;
	thumb_state2_ = 0;
	Bind(wxEVT_PAINT, &wxDoubleSlider::OnPaint, this);
	Bind(wxEVT_LEFT_DOWN,&wxDoubleSlider::OnLeftDown, this);
	Bind(wxEVT_MOTION,&wxDoubleSlider::OnMotion, this);
	Bind(wxEVT_LEFT_UP,&wxDoubleSlider::OnLeftUp, this);
	Bind(wxEVT_LEAVE_WINDOW, &wxDoubleSlider::OnLeave, this);
	Bind(wxEVT_MOUSEWHEEL, &wxDoubleSlider::OnWheel, this);
	Refresh();
	Update();
}

bool wxDoubleSlider::setLowValue(int val, bool notify)
{
	int old = low_val_;
	if (link_)
	{
		low_val_ = val < min_val_ ? min_val_ :
		(val >= max_val_ ? max_val_ - 1 : val);

		hi_val_ = low_val_ + link_dist_;
		if (hi_val_ > max_val_)
		{
			hi_val_ = max_val_;
			low_val_ = hi_val_ - link_dist_;
		}
	}
	else
		low_val_ = val < min_val_ ? min_val_ :
		(val >= hi_val_ ? hi_val_ - 1 : val);

	bool changed = old != low_val_;

	if (changed || stack_.empty())
	{
		double t;
		if (time_sample(t))
			push(t);
		else
			replace(t);
	}

	if (!changed)
		return changed;

	Refresh();
	Update();

	if (notify)
	{
		wxCommandEvent e(wxEVT_SCROLL_CHANGED, id_);
		e.SetEventObject(this);
		e.SetString("update low");
		ProcessWindowEvent(e);
		wxPostEvent(parent_, e);
	}

	return changed;
}

bool wxDoubleSlider::setHighValue(int val, bool notify)
{
	int old = hi_val_;
	if (link_)
	{
		hi_val_ = val <= min_val_ ? min_val_ + 1 :
		(val > max_val_ ? max_val_ : val);
		low_val_ = hi_val_ - link_dist_;
		if (low_val_ < min_val_)
		{
			low_val_ = min_val_;
			hi_val_ = low_val_ + link_dist_;
		}
	}
	else
		hi_val_ = val <= low_val_ ? low_val_ + 1 :
		(val > max_val_ ? max_val_ : val);

	bool changed = old != hi_val_;

	if (changed || stack_.empty())
	{
		double t;
		if (time_sample(t))
			push(t);
		else
			replace(t);
	}

	if (!changed)
		return changed;

	Refresh();
	Update();

	if (notify)
	{
		wxCommandEvent e(wxEVT_SCROLL_CHANGED, id_);
		e.SetEventObject(this);
		e.SetString("update high");
		ProcessWindowEvent(e);
		wxPostEvent(parent_, e);
	}

	return changed;
}

bool wxDoubleSlider::SetLowValue(int val)
{
	return setLowValue(val);
}

bool wxDoubleSlider::SetHighValue(int val)
{
	return setHighValue(val);
}

bool wxDoubleSlider::ChangeLowValue(int val)
{
	return setLowValue(val, false);
}

bool wxDoubleSlider::ChangeHighValue(int val)
{
	return setHighValue(val, false);
}

bool wxDoubleSlider::ChangeValues(int &low, int &hi)
{
	int olow = low_val_;
	int ohi = hi_val_;

	low_val_ = low < min_val_ ? min_val_ :
		(low >= hi ? hi - 1 : low);
	hi_val_ = hi <= low_val_ ? low_val_ + 1 :
		(hi > max_val_ ? max_val_ : hi);
	link_dist_ = hi_val_ - low_val_;

	bool changed = low_val_ != olow || hi_val_ != ohi;
	low = low_val_;
	hi = hi <= low ? hi_val_ : hi;

	if (changed || stack_.empty())
	{
		double t;
		if (time_sample(t))
			push(t);
		else
			replace(t);
	}

	if (!changed)
		return changed;

	Refresh();
	Update();

	return changed;
}

void wxDoubleSlider::SetLink(bool val)
{
	link_ = val;
	if (link_)
		link_dist_ = hi_val_ - low_val_;
}

bool wxDoubleSlider::GetLink()
{
	return link_;
}

void wxDoubleSlider::SetLinkDist(int val)
{
	link_dist_ = val;
	if (link_dist_ > max_val_ - min_val_)
		link_dist_ = max_val_ - min_val_;
	hi_val_ = low_val_ + link_dist_;
	if (hi_val_ > max_val_)
	{
		hi_val_ = max_val_;
		low_val_ = hi_val_ - link_dist_;
	}
	Refresh();
	Update();
}

int wxDoubleSlider::GetLinkDist()
{
	return link_dist_;
}

int wxDoubleSlider::GetLowValue()
{
	return low_val_;
}

int wxDoubleSlider::GetHighValue()
{
	return hi_val_;
}

void wxDoubleSlider::renderNormal(wxDC& dc)
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
	int posl = std::max(min_val_, low_val_);
	posl = std::round(double(posl - min_val_) * ((horizontal_ ? w : h) - margin_ * 2) / (max_val_ - min_val_));
	//right slider:
	int posr = std::min(max_val_, hi_val_);
	posr = std::round(double(posr - min_val_) * ((horizontal_ ? w : h) - margin_ * 2) / (max_val_ - min_val_));

	wxColor color, color1, color2;
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
		color1 = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
		color2 = color1;
		switch (thumb_state1_)
		{
		case 0:
			if (use_thumb_color_)
				color1 = thumb_color1_;
			break;
		case 1:
			color1 = wxColor(128, 128, 128);
			break;
		case 2:
			if (use_thumb_color_)
				color1 = thumb_color1_;
			else
				color1 = *wxLIGHT_GREY;
			break;
		}
		switch (thumb_state2_)
		{
		case 0:
			if (use_thumb_color_)
				color2 = thumb_color2_;
			break;
		case 1:
			color2 = wxColor(128, 128, 128);
			break;
		case 2:
			if (use_thumb_color_)
				color2 = thumb_color2_;
			else
				color2 = *wxLIGHT_GREY;
			break;
		}
	}
	else
	{
		color1 = *wxLIGHT_GREY;
		color2 = color1;
	}

	if (horizontal_)
	{
		DrawThumb(dc, margin_ + posl, h * 0.5, color1);
		DrawThumb(dc, margin_ + posr, h * 0.5, color2);
	}
	else
	{
		DrawThumb(dc, w * 0.5, margin_ + posl, color1);
		DrawThumb(dc, w * 0.5, margin_ + posr, color2);
	}
}

void wxDoubleSlider::renderInverse(wxDC& dc)
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
	int posl = std::max(min_val_, low_val_);
	posl = std::round(double(posl - min_val_) * ((horizontal_ ? w : h) - margin_ * 2) / (max_val_ - min_val_));
	//right slider:
	int posr = std::min(max_val_, hi_val_);
	posr = std::round(double(posr - min_val_) * ((horizontal_ ? w : h) - margin_ * 2) / (max_val_ - min_val_));

	wxColor color, color1, color2;
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
		color1 = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
		color2 = color1;
		switch (thumb_state1_)
		{
		case 0:
			if (use_thumb_color_)
				color1 = thumb_color1_;
			break;
		case 1:
			color1 = wxColor(128, 128, 128);
			break;
		case 2:
			if (use_thumb_color_)
				color1 = thumb_color1_;
			else
				color1 = *wxLIGHT_GREY;
			break;
		}
		switch (thumb_state2_)
		{
		case 0:
			if (use_thumb_color_)
				color2 = thumb_color2_;
			break;
		case 1:
			color2 = wxColor(128, 128, 128);
			break;
		case 2:
			if (use_thumb_color_)
				color2 = thumb_color2_;
			else
				color2 = *wxLIGHT_GREY;
			break;
		}
	}
	else
	{
		color1 = *wxLIGHT_GREY;
		color2 = color1;
	}

	if (horizontal_)
	{
		DrawThumb(dc, w - margin_ - posl, h * 0.5, color1);
		DrawThumb(dc, w - margin_ - posr, h * 0.5, color2);
	}
	else
	{
		DrawThumb(dc, w * 0.5, h - margin_ - posl, color1);
		DrawThumb(dc, w * 0.5, h - margin_ - posr, color2);
	}
}

void wxDoubleSlider::OnLeftDown(wxMouseEvent& event)
{
	SetFocus();
	CaptureMouse();

	int w, h;
	wxClientDC dc(this);
	dc.GetSize(&w, &h);
	wxPoint pos = event.GetLogicalPosition(dc);

	//left slider:
	int posl = std::max(min_val_, low_val_);
	posl = std::round(double(posl - min_val_) * ((horizontal_ ? w : h) - margin_ * 2) / (max_val_ - min_val_));
	posl = inverse_ ? (horizontal_ ? w - margin_ - posl : h - margin_ - posl) : (posl + margin_);
	//right slider:
	int posr = std::min(max_val_, hi_val_);
	posr = std::round(double(posr - min_val_) * ((horizontal_ ? w : h) - margin_ * 2) / (max_val_ - min_val_));
	posr = inverse_ ? (horizontal_ ? w - margin_ - posr : h - margin_ - posr) : (posr + margin_);

	if (link_)
	{
		sel_ = 3;//both
		last_sel_ = sel_;
		sel_val_ = horizontal_ ? pos.x : pos.y;
	}
	else
	{
		int pp = horizontal_ ? pos.x : pos.y;
		int distl = std::abs(pp - posl);
		int distr = std::abs(pp - posr);
		if (distl < distr)
			sel_ = 1;
		else if (distl > distr)
			sel_ = 2;
		else
		{
			if (pp < posl)
				sel_ = 1;
			else
				sel_ = 2;
		}
		last_sel_ = sel_;
	}

	int val;

	if (link_)
	{
		bool set_low = inverse_? sel_val_ > posl : sel_val_ < posl;
		bool set_hi = inverse_ ? sel_val_ < posr : sel_val_ > posr;
		
		posl = margin_;
		posr = (horizontal_ ? w : h) - margin_;
		if (inverse_)
			val = std::round(double(min_val_) + double(max_val_ - min_val_) *
				(posr - sel_val_) / (posr - posl));
		else
			val = std::round(double(min_val_) + double(max_val_ - min_val_) *
				(sel_val_ - posl) / (posr - posl));

		if (set_low)
		{
			setLowValue(val);
			sel_val_ = low_val_;
		}
		else if (set_hi)
		{
			setHighValue(val);
			sel_val_ = hi_val_;
		}
		else
		{
			sel_val_ = val;
		}
	}
	else
	{
		posl = margin_;
		posr = (horizontal_ ? w : h) - margin_;
		if (inverse_)
			val = std::round(double(min_val_) + double(max_val_ - min_val_) *
				(posr - (horizontal_ ? pos.x : pos.y)) / (posr - posl));
		else
			val = std::round(double(min_val_) + double(max_val_ - min_val_) *
				((horizontal_ ? pos.x : pos.y) - posl) / (posr - posl));

		if (sel_ == 1)
			setLowValue(val);
		else if (sel_ == 2)
			setHighValue(val);
	}

	event.Skip();
}

void wxDoubleSlider::OnMotion(wxMouseEvent& event)
{
	int w, h;
	wxClientDC dc(this);
	dc.GetSize(&w, &h);
	wxPoint pos = event.GetLogicalPosition(dc);

	if (sel_ == 0)
	{
		//left slider:
		int posl = std::max(min_val_, low_val_);
		posl = std::round(double(posl - min_val_) * ((horizontal_ ? w : h) - margin_ * 2) / (max_val_ - min_val_));
		//right slider:
		int posr = std::min(max_val_, hi_val_);
		posr = std::round(double(posr - min_val_) * ((horizontal_ ? w : h) - margin_ * 2) / (max_val_ - min_val_));
		wxRect rect1, rect2;
		if (inverse_)
		{
			if (horizontal_)
			{
				rect1 = wxRect(w - margin_ - posl - 5 * scale_,
					h * 0.5 - 7 * scale_,
					10 * scale_, 13 * scale_);
				rect2 = wxRect(w - margin_ - posr - 5 * scale_,
					h * 0.5 - 7 * scale_,
					10 * scale_, 13 * scale_);
			}
			else
			{
				rect1 = wxRect(w * 0.5 - 7 * scale_,
					h - margin_ - posl - 5 * scale_,
					13 * scale_, 10 * scale_);
				rect2 = wxRect(w * 0.5 - 7 * scale_,
					h - margin_ - posr - 5 * scale_,
					13 * scale_, 10 * scale_);
			}
		}
		else
		{
			if (horizontal_)
			{
				rect1 = wxRect(margin_ + posl - 5 * scale_,
					h * 0.5 - 7 * scale_,
					10 * scale_, 13 * scale_);
				rect2 = wxRect(margin_ + posr - 5 * scale_,
					h * 0.5 - 7 * scale_,
					10 * scale_, 13 * scale_);
			}
			else
			{
				rect1 = wxRect(w * 0.5 - 7 * scale_,
					margin_ + posl - 5 * scale_,
					13 * scale_, 10 * scale_);
				rect2 = wxRect(w * 0.5 - 7 * scale_,
					margin_ + posr - 5 * scale_,
					13 * scale_, 10 * scale_);
			}
		}
		bool refresh = false;
		if (thumb_state1_ == 0 && rect1.Contains(pos))
		{
			thumb_state1_ = 1;
			refresh = true;
		}
		if (thumb_state1_ == 1 && !rect1.Contains(pos))
		{
			thumb_state1_ = 0;
			refresh = true;
		}
		if (thumb_state2_ == 0 && rect2.Contains(pos))
		{
			thumb_state2_ = 1;
			refresh = true;
		}
		if (thumb_state2_ == 1 && !rect2.Contains(pos))
		{
			thumb_state2_ = 0;
			refresh = true;
		}
		if (refresh)
		{
			Refresh();
			Update();
		}
	}
	else if (sel_ == 1 || sel_ == 2)
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

		if (sel_ == 1)
		{
			thumb_state1_ = 2;
			bool changed = setLowValue(val, false);
			//if (!changed && low_value_ < max_)
		}
		else if (sel_ == 2)
		{
			thumb_state2_ = 2;
			setHighValue(val, false);
		}

		wxCommandEvent e(wxEVT_SCROLL_CHANGED, id_);
		e.SetEventObject(this);
		if (sel_ == 1)
			e.SetString("update low");
		else if (sel_ == 2)
			e.SetString("update high");
		ProcessWindowEvent(e);
		wxPostEvent(parent_, e);
	}
	else if (sel_ == 3)
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

		bool changed = setLowValue(low_val_ + val - sel_val_, false);
		if (changed)
		{
			sel_val_ = val;
			wxCommandEvent e(wxEVT_SCROLL_CHANGED, id_);
			e.SetEventObject(this);
			e.SetString("update both");
			ProcessWindowEvent(e);
			wxPostEvent(parent_, e);
		}
	}

	event.Skip();
}

void wxDoubleSlider::OnLeftUp(wxMouseEvent& event)
{
	thumb_state1_ = 0;
	thumb_state2_ = 0;
	wxBasisSlider::OnLeftUp(event);
}

void wxDoubleSlider::OnLeave(wxMouseEvent& event)
{
	thumb_state1_ = 0;
	thumb_state2_ = 0;
	wxBasisSlider::OnLeave(event);
}

void wxDoubleSlider::SetRange(int min_val, int max_val)
{
	double old_range = max_val_ - min_val_;
	double new_range = max_val - min_val;
	low_val_ = static_cast<int>(std::round((double(low_val_) - min_val_)*new_range/old_range + min_val));
	hi_val_ = static_cast<int>(std::round((double(hi_val_) - min_val_)*new_range/old_range + min_val));
	wxBasisSlider::SetRange(min_val, max_val);
}

void wxDoubleSlider::SetThumbColor(const wxColor& c1, const wxColor& c2)
{
	thumb_color1_ = c1;
	thumb_color2_ = c2;
	wxBasisSlider::SetThumbColor(c1);
}

void wxDoubleSlider::Scroll(int val)
{
	if (last_sel_ == 1)
		SetLowValue(low_val_ + val);
	else
		SetHighValue(hi_val_ + val);
	//wxBasisSlider::Scroll(val);
}

void wxDoubleSlider::replace(double t)
{
	if (stack_.empty())
		return;
	stack_[stack_pointer_] = std::pair<double, ValueDSL>(t, ValueDSL(low_val_, hi_val_));
	//DBGPRINT(L"\tsize:%d,pointer:%d,last:((%f, %d), (%f, %d))\n",
	//	stack_size_, stack_pointer_,
	//	stack1_.back().first, stack1_.back().second,
	//	stack2_.back().first, stack2_.back().second);
}

void wxDoubleSlider::push(double t)
{
	size_t size = stack_.size();
	ValueDSL val_(low_val_, hi_val_);
	ValueDSL val = size ? std::any_cast<ValueDSL>(stack_[stack_pointer_].second) : val_;
	if (!size || !val_.Equals(val))
	{
		if (!size ||
			stack_pointer_ == size - 1)
			stack_.push_back(std::pair<double, ValueDSL>(t, val_));
		else
			stack_.insert(stack_.begin() + stack_pointer_,
				std::pair<double, ValueDSL>(t, val_));
		stack_pointer_++;
		//DBGPRINT(L"\tsize:%d,pointer:%d,last:((%f, %d), (%f, %d))\n",
		//	stack_size_, stack_pointer_,
		//	stack1_.back().first, stack1_.back().second,
		//	stack2_.back().first, stack2_.back().second);

		if (indicator_)
		{
			indicator_->SetLength(stack_.size());
			indicator_->SetPosition(stack_pointer_);
			indicator_->UpdateHistory();
		}
	}
}

void wxDoubleSlider::update()
{
	ValueDSL val = std::any_cast<ValueDSL>(stack_[stack_pointer_].second);
	if (val.hv - val.lv != link_dist_ && link_)
		link_ = false;
	low_val_ = val.lv;
	hi_val_ = val.hv;

	Refresh();
	Update();

	wxCommandEvent e(wxEVT_SCROLL_CHANGED, id_);
	e.SetEventObject(this);
	e.SetString("undo/redo");
	ProcessWindowEvent(e);
	wxPostEvent(parent_, e);
}

