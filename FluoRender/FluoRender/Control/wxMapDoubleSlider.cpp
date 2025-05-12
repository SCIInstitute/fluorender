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

void wxMapDoubleSlider::SetMapData(const std::vector<unsigned char>& rgbData)
{
	m_rgbData = rgbData;
	GenerateBaseImage();
	Refresh();
}

void wxMapDoubleSlider::renderNormal(wxDC& dc)
{
	int w, h;
	GetSize(&w, &h);

	bool bitmap_valid = m_colorBitmap.IsOk() && m_grayBitmap.IsOk();
	int a, b, c, d, e, f;
	wxColor color, color1, color2;

	//draw slot
	if (horizontal_)
	{
		a = std::round(h / 2.0 - scale_);
		b = std::round(w - margin_ + 1.0 + 1.5 * scale_);
		c = std::round(h / 2.0 - 0.2 * scale_);
		d = std::round(h / 2.0 + scale_ + 0.5 * scale_);
		e = std::round(h / 2.0 + 2.0 * scale_);
		f = std::round(margin_ - 1.5 * scale_);
	}
	else
	{
		a = std::round(w / 2.0 - scale_);
		b = std::round(h - margin_ + 1.0 + 1.5 * scale_);
		c = std::round(w / 2.0 - 0.2 * scale_);
		d = std::round(w / 2.0 + scale_ + 0.5 * scale_);
		e = std::round(w / 2.0 + 2.0 * scale_);
		f = std::round(margin_ - 1.5 * scale_);
	}

	dc.SetPen(*wxGREY_PEN);
	dc.SetBrush(*wxTRANSPARENT_BRUSH);
	if (horizontal_)
	{
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
	//size
	int width, height;
	if (horizontal_)
	{
		a = std::round(h / 2.0 - 4 * scale_);
		b = std::round(h / 2.0 + 4 * scale_);
		width = w - 2 * margin_;
		height = 8 * scale_;
	}
	else
	{
		a = std::round(w / 2.0 - 4 * scale_);
		b = std::round(w / 2.0 + 4 * scale_);
		width = 8 * scale_;
		height = h - 2 * margin_;
	}

	//draw map
	if (bitmap_valid)
	{
		if (enabled_)
		{
			wxImage colorScaled = m_colorBitmap.ConvertToImage().Rescale(width, height, wxIMAGE_QUALITY_BICUBIC);
			wxBitmap colorBitmapScaled(colorScaled);
			if (horizontal_)
				dc.DrawBitmap(colorBitmapScaled.GetSubBitmap(wxRect(posl, 0, posr - posl, height)), margin_ + posl, h / 2.0 - 4 * scale_, false);
			else
				dc.DrawBitmap(colorBitmapScaled.GetSubBitmap(wxRect(0, posl, width, posr - posl)), w / 2.0 - 4 * scale_, margin_ + posl, false);
		}
		else
		{
			wxImage grayScaled = m_grayBitmap.ConvertToImage().Rescale(width, height, wxIMAGE_QUALITY_BICUBIC);
			wxBitmap grayBitmapScaled(grayScaled);
			if (horizontal_)
				dc.DrawBitmap(grayBitmapScaled, margin_, h / 2.0 - 4 * scale_, false);
			else
				dc.DrawBitmap(grayBitmapScaled, w / 2.0 - 4 * scale_, margin_, false);
		}
	}
	else
	{
		//range color
		if (use_range_color_)
		{
			color = enabled_ ? range_color_ : *wxBLACK;
			dc.SetPen(color);
			dc.SetBrush(color);
			if (horizontal_)
			{
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
				wxPoint p2v[] = {
					wxPoint(a, margin_ + posl),
					wxPoint(a, margin_ + posr),
					wxPoint(b, margin_ + posr),
					wxPoint(b, margin_ + posl)
				};
				dc.DrawPolygon(4, p2v, wxODDEVEN_RULE);
			}
		}
	}

	//draw thumb
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
	dc.SetBrush(*wxTRANSPARENT_BRUSH);
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
			a = std::round(h / 2.0 - 4 * scale_);
			b = std::round(h / 2.0 + 4 * scale_);
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
			a = std::round(w / 2.0 - 4 * scale_);
			b = std::round(w / 2.0 + 4 * scale_);
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

void wxMapDoubleSlider::GenerateBaseImage()
{
	int numBins = static_cast<int>(m_rgbData.size() / 3);
	if (numBins == 0) return;

	wxImage colorImage(numBins, 1);
	unsigned char* data = colorImage.GetData();

	for (int i = 0; i < numBins; ++i)
	{
		unsigned char r = m_rgbData[i * 3];
		unsigned char g = m_rgbData[i * 3 + 1];
		unsigned char b = m_rgbData[i * 3 + 2];

		int index = i * 3;
		data[index] = r;
		data[index + 1] = g;
		data[index + 2] = b;
	}

	wxImage grayImage = colorImage.ConvertToGreyscale();

	m_colorBitmap = wxBitmap(colorImage);
	m_grayBitmap = wxBitmap(grayImage);
}
