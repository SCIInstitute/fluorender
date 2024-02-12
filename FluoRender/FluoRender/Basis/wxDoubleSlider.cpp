

#include "wxDoubleSlider.h"

wxDoubleSlider::wxDoubleSlider(wxWindow *parent,
		wxWindowID id,
		//const wxString& label,
		int leftValue, int rightValue, int minValue, int maxValue,
		const wxPoint& pos,
		const wxSize& size,
		long style,
		const wxValidator& val,
		const wxString& name):
	parent_(parent),
	id_(id),
	use_range_color_(false),
	use_thumb_color_(false),
	horizontal_(!(style & wxSL_VERTICAL)),
	inverse_(style & wxSL_INVERSE),
	wxControl(parent, id, pos,
		wxSize(!(style& wxSL_VERTICAL) ? int(std::round(23 * parent->GetDPIScaleFactor())) : 1,
			!(style& wxSL_VERTICAL) ? 1 : int(std::round(23* parent->GetDPIScaleFactor()))), wxBORDER_NONE)
{
	scale_ = parent->GetDPIScaleFactor();
	margin_ = int(std::round(12 * scale_));
	SetBackgroundColour(parent->GetBackgroundColour());
	SetDoubleBuffered(true);
	low_val_ = leftValue; hi_val_ = rightValue;
	min_val_ = minValue; max_val_ = maxValue;
	sel_ = 0;
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

void wxDoubleSlider::SetLowValue(int val)
{
	low_val_ = val < min_val_ ? min_val_ : 
		(val >= hi_val_ ? hi_val_ - 1 : val);
	Refresh();
	Update();
}

void wxDoubleSlider::SetHighValue(int val)
{
	hi_val_ = val <= low_val_ ? low_val_ + 1 :
		(val > max_val_ ? max_val_ : val);
	Refresh();
	Update();
}

int wxDoubleSlider::GetLowValue()
{
	return low_val_;
}

int wxDoubleSlider::GetHighValue()
{
	return hi_val_;
}

wxSize wxDoubleSlider::DoGetBestSize()
{
	return (wxSize(1,1));
}

void wxDoubleSlider::OnPaint(wxPaintEvent&)
{
	wxPaintDC dc(this);
	render(dc);
}

void wxDoubleSlider::paintNow()
{
	wxClientDC dc(this);
	render(dc);
}

void  wxDoubleSlider::DrawThumb(wxDC& dc, wxCoord x, wxCoord y, const wxColor& c)
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
	dc.DrawPolygon(num, horizontal_? ph : pv, wxODDEVEN_RULE);
}

void wxDoubleSlider::render(wxDC& dc)
{
	if (inverse_)
		renderInverse(dc);
	else
		renderNormal(dc);
}

void wxDoubleSlider::renderNormal(wxDC& dc)
{
	int w, h;
	GetSize(&w, &h);

	dc.SetPen(*wxGREY_PEN);
	dc.SetBrush(*wxLIGHT_GREY_BRUSH);
	wxPoint p1h[] = {
		wxPoint(margin_, h / 2 - scale_),
		wxPoint(w - margin_ + 1, h / 2 - scale_),
		wxPoint(w - margin_ + 1, h / 2 + 2 * scale_),
		wxPoint(margin_, h / 2 + 2 * scale_)
	};
	wxPoint p1v[] = {
		wxPoint(w / 2 - scale_, margin_),
		wxPoint(w / 2 - scale_, h - margin_ + 1),
		wxPoint(w / 2 + 2 * scale_, h - margin_ + 1),
		wxPoint(w / 2 + 2 * scale_, margin_)
	};
	int num = 4;
	dc.DrawPolygon(num, horizontal_ ? p1h : p1v, wxODDEVEN_RULE);

	//left slider:
	int posl = std::max(min_val_, low_val_);
	posl = std::round(double(posl - min_val_) * ((horizontal_ ? w : h) - margin_ * 2) / (max_val_ - min_val_));
	//right slider:
	int posr = std::min(max_val_, hi_val_);
	posr = std::round(double(posr - min_val_) * ((horizontal_ ? w : h) - margin_ * 2) / (max_val_ - min_val_));

	//range color
	if (use_range_color_)
	{
		dc.SetPen(range_color_);
		dc.SetBrush(range_color_);
		wxPoint p2h[] = {
			wxPoint(margin_ + posl, h / 2 - scale_),
			wxPoint(margin_ + posr, h / 2 - scale_),
			wxPoint(margin_ + posr, h / 2 + 2 * scale_),
			wxPoint(margin_ + posl, h / 2 + 2 * scale_)
		};
		wxPoint p2v[] = {
			wxPoint(w / 2 - scale_, margin_ + posl),
			wxPoint(w / 2 - scale_, margin_ + posr),
			wxPoint(w / 2 + 2 * scale_, margin_ + posr),
			wxPoint(w / 2 + 2 * scale_, margin_ + posl)
		};
		num = 4;
		dc.DrawPolygon(num, horizontal_ ? p2h : p2v, wxODDEVEN_RULE);
	}

	wxColor color1 = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
	wxColor color2 = color1;
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
	wxPoint p1h[] = {
		wxPoint(margin_, h / 2 - scale_),
		wxPoint(w - margin_ + 1, h / 2 - scale_),
		wxPoint(w - margin_ + 1, h / 2 + 2 * scale_),
		wxPoint(margin_, h / 2 + 2 * scale_)
	};
	wxPoint p1v[] = {
		wxPoint(w / 2 - scale_, margin_),
		wxPoint(w / 2 - scale_, h - margin_ + 1),
		wxPoint(w / 2 + 2 * scale_, h - margin_ + 1),
		wxPoint(w / 2 + 2 * scale_, margin_)
	};
	int num = 4;
	dc.DrawPolygon(num, horizontal_ ? p1h : p1v, wxODDEVEN_RULE);

	//left slider:
	int posl = std::max(min_val_, low_val_);
	posl = std::round(double(posl - min_val_) * ((horizontal_ ? w : h) - margin_ * 2) / (max_val_ - min_val_));
	//right slider:
	int posr = std::min(max_val_, hi_val_);
	posr = std::round(double(posr - min_val_) * ((horizontal_ ? w : h) - margin_ * 2) / (max_val_ - min_val_));

	//range color
	if (use_range_color_)
	{
		dc.SetPen(range_color_);
		dc.SetBrush(range_color_);
		wxPoint p2h[] = {
			wxPoint(w - margin_ - posl, h / 2 - scale_),
			wxPoint(w - margin_ - posr, h / 2 - scale_),
			wxPoint(w - margin_ - posr, h / 2 + 2 * scale_),
			wxPoint(w - margin_ - posl, h / 2 + 2 * scale_)
		};
		wxPoint p2v[] = {
			wxPoint(w / 2 - scale_, h - margin_ - posl),
			wxPoint(w / 2 - scale_, h - margin_ - posr),
			wxPoint(w / 2 + 2 * scale_, h - margin_ - posr),
			wxPoint(w / 2 + 2 * scale_, h - margin_ - posl)
		};
		num = 4;
		dc.DrawPolygon(num, horizontal_ ? p2h : p2v, wxODDEVEN_RULE);
	}

	wxColor color1 = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
	wxColor color2 = color1;
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

	int distl = std::abs((horizontal_ ? pos.x : pos.y) - posl);
	int distr = std::abs((horizontal_ ? pos.x : pos.y) - posr);
	if (distl <= distr)
		sel_ = 1;
	else
		sel_ = 2;
	last_sel_ = sel_;

	posl = margin_;
	posr = (horizontal_ ? w : h) - margin_;
	int val;
	if (inverse_)
		val = std::round(double(min_val_) + double(max_val_ - min_val_) *
			(posr - (horizontal_ ? pos.x : pos.y)) / (posr - posl));
	else
		val = std::round(double(min_val_) + double(max_val_ - min_val_) *
			((horizontal_ ? pos.x : pos.y) - posl) / (posr - posl));

	if (sel_ == 1)
	{
		low_val_ = val;
		if (low_val_ < min_val_)  low_val_ = min_val_;
		if (low_val_ >= hi_val_) low_val_ = hi_val_ - 1;
	}
	else if (sel_ == 2)
	{
		hi_val_ = val;
		if (hi_val_ > max_val_) hi_val_ = max_val_;
		if (hi_val_ <= low_val_) hi_val_ = low_val_ + 1;
	}

	Refresh();
	Update();

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

		if (sel_ == 1)
		{
			thumb_state1_ = 2;
			low_val_ = val;
			if (low_val_ < min_val_)  low_val_ = min_val_;
			if (low_val_ >= hi_val_) low_val_ = hi_val_ - 1;
		}
		else if (sel_ == 2)
		{
			thumb_state2_ = 2;
			hi_val_ = val;
			if (hi_val_ > max_val_) hi_val_ = max_val_;
			if (hi_val_ <= low_val_) hi_val_ = low_val_ + 1;
		}

		Refresh();
		Update();

		wxCommandEvent e(wxEVT_SCROLL_CHANGED, id_);
		e.SetEventObject(this);
		e.SetString("update");
		ProcessWindowEvent(e);
		wxPostEvent(parent_, e);
	}

	event.Skip();
}

void wxDoubleSlider::OnLeftUp(wxMouseEvent& event)
{
	ReleaseMouse();
	event.Skip();

	if (sel_)
	{
		sel_ = 0;
		thumb_state1_ = 0;
		thumb_state2_ = 0;
		Refresh();
		Update();
		wxCommandEvent e(wxEVT_SCROLL_CHANGED, id_);
		e.SetEventObject(this);
		e.SetString("update");
		ProcessWindowEvent(e);
		wxPostEvent(parent_, e);

	}
}

void wxDoubleSlider::OnLeave(wxMouseEvent& event)
{
	thumb_state1_ = 0;
	thumb_state2_ = 0;
	Refresh();
	Update();
	event.Skip();
}

void wxDoubleSlider::OnWheel(wxMouseEvent& event)
{
	int m, w, h;
	wxClientDC dc(this);
	dc.GetSize(&w, &h);
	wxPoint pos = event.GetLogicalPosition(dc);

	if (event.GetWheelRotation() > 0)
		m=-1;
	else
		m=1;
	//bool rotl = true;
	//if (horizontal_)
	//	rotl = pos.y >= h / 2;
	//else
	//	rotl = pos.x >= w / 2;
	if (last_sel_ == 1)
	{
		low_val_ -= m;
		if (low_val_ < min_val_)  low_val_ = min_val_;
		if (low_val_ >= hi_val_) low_val_ = hi_val_-1;
	}
	else
	{
		hi_val_ -= m;
		if (hi_val_ > max_val_) hi_val_ = max_val_;
		if (hi_val_ <= low_val_) hi_val_ = low_val_+1;
	}
	Refresh();
	Update();
	wxCommandEvent e(wxEVT_SCROLL_CHANGED, id_);
	e.SetEventObject(this);
	e.SetString("update");
	ProcessWindowEvent(e);
	wxPostEvent(parent_, e);
	event.Skip();
}

void wxDoubleSlider::SetRange(int min_val, int max_val)
{
	min_val_ = min_val;
	max_val_ = max_val;
}

int wxDoubleSlider::GetMax()
{
	return max_val_;
}

int wxDoubleSlider::GetMin()
{
	return min_val_;
}

void wxDoubleSlider::SetRangeColor(const wxColor& c)
{
	range_color_ = c;
	use_range_color_ = true;
}

void wxDoubleSlider::DisableRangeColor()
{
	use_range_color_ = false;
}

void wxDoubleSlider::SetThumbColor(const wxColor& c1, const wxColor& c2)
{
	thumb_color1_ = c1;
	thumb_color2_ = c2;
	use_thumb_color_ = true;
}

void wxDoubleSlider::DisableThumbColor()
{
	use_thumb_color_ = false;
}
