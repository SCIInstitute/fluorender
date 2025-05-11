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
#include <wxMapDoubleSlider.h>

wxMapDoubleSlider::wxMapDoubleSlider(
	wxWindow* parent,
	wxWindowID id,
	int lowValue, int hiValue, int minValue, int maxValue,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxValidator& val,
	const wxString& name) :
	wxDoubleSlider(parent, id, lowValue, hiValue, minValue, maxValue, pos, size, style, val, name)
{
}

void wxMapDoubleSlider::renderNormal(wxDC& dc)
{
	int w, h;
	GetSize(&w, &h);

	dc.SetPen(*wxGREY_PEN);
	dc.SetBrush(*wxLIGHT_GREY_BRUSH);
	if (horizontal_)
	{
		int a, b, c, d, e, f;
		a = std::round(h - scale_);
		b = std::round(w - margin_ + 1.0 + 1.5 * scale_);
		c = std::round(h - 0.2 * scale_);
		d = std::round(h + scale_ + 0.5 * scale_);
		e = std::round(h + 2.0 * scale_);
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
		a = std::round(w - scale_);
		b = std::round(h - margin_ + 1.0 + 1.5 * scale_);
		c = std::round(w - 0.2 * scale_);
		d = std::round(w + scale_ + 0.5 * scale_);
		e = std::round(w + 2.0 * scale_);
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
			a = std::round(h - scale_);
			b = std::round(h + 2.0 * scale_);
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
			a = std::round(w - scale_);
			b = std::round(w + 2.0 * scale_);
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

void wxMapDoubleSlider::renderInverse(wxDC& dc)
{
	int w, h;
	GetSize(&w, &h);

	dc.SetPen(*wxGREY_PEN);
	dc.SetBrush(*wxLIGHT_GREY_BRUSH);
	if (horizontal_)
	{
		int a, b, c, d, e, f;
		a = std::round(h - scale_);
		b = std::round(w - margin_ + 1.0 + 1.5 * scale_);
		c = std::round(h - 0.2 * scale_);
		d = std::round(h + scale_ + 0.5 * scale_);
		e = std::round(h + 2.0 * scale_);
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
		a = std::round(w - scale_);
		b = std::round(h - margin_ + 1.0 + 1.5 * scale_);
		c = std::round(w - 0.2 * scale_);
		d = std::round(w + scale_ + 0.5 * scale_);
		e = std::round(w + 2.0 * scale_);
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
			a = std::round(h - scale_);
			b = std::round(h + 2.0 * scale_);
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
			a = std::round(w - scale_);
			b = std::round(w + 2.0 * scale_);
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
