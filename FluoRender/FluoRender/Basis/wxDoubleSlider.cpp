

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
	wxControl(parent, id, pos,
		wxSize(!(style& wxSL_VERTICAL) ? int(std::round(23 * parent->GetDPIScaleFactor())) : 1,
			!(style& wxSL_VERTICAL) ? 1 : int(std::round(23* parent->GetDPIScaleFactor()))), wxBORDER_NONE)
{
	scale = parent->GetDPIScaleFactor();
	margin = int(std::round(12 * scale));
	SetBackgroundColour(parent->GetBackgroundColour());
	SetDoubleBuffered(true);
	leftval = leftValue; rightval = rightValue; minval = minValue; maxval = maxValue;
	sel_ = 0;
	last_sel_ = 1;
	floatlabel = false;
	Bind(wxEVT_PAINT, &wxDoubleSlider::OnPaint, this);
	Bind(wxEVT_LEFT_DOWN,&wxDoubleSlider::OnLeftDown, this);
	Bind(wxEVT_MOTION,&wxDoubleSlider::OnMotion, this);
	Bind(wxEVT_LEFT_UP,&wxDoubleSlider::OnLeftUp, this);
	//Bind(wxEVT_LEAVE_WINDOW, &wxDoubleSlider::OnLeftUp, this);
	Bind(wxEVT_MOUSEWHEEL, &wxDoubleSlider::OnWheel, this);
	Refresh();
	Update();
}

void wxDoubleSlider::SetFloatLabel(bool f)
{
	floatlabel = f;
}

void wxDoubleSlider::SetLeftValue(int lval)
{
	leftval = lval;
	Refresh();
	Update();
}

void wxDoubleSlider::SetRightValue(int rval)
{
	rightval = rval;
	Refresh();
	Update();
}

int wxDoubleSlider::GetLeftValue()
{
	return leftval;
}

int wxDoubleSlider::GetRightValue()
{
	return rightval;
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

void  wxDoubleSlider::DrawThumb(wxDC& dc, wxCoord x, wxCoord y)
{
	wxPoint ph[] = {
		wxPoint(x - 5 * scale, y - 7 * scale),
		wxPoint(x + 5 * scale, y - 7 * scale),
		wxPoint(x + 5 * scale, y + 6 * scale),
		wxPoint(x, y + 11 * scale),
		wxPoint(x - 5 * scale, y + 6 * scale)
	};
	wxPoint pv[] = {
		wxPoint(x - 7 * scale, y - 5 * scale),
		wxPoint(x - 7 * scale, y + 5 * scale),
		wxPoint(x + 6 * scale, y + 5 * scale),
		wxPoint(x + 11 * scale, y),
		wxPoint(x + 6 * scale, y - 5 * scale)
	};
	int num = 5;
	dc.DrawPolygon(num, horizontal_? ph : pv, wxODDEVEN_RULE);
}

void wxDoubleSlider::render(wxDC& dc)
{
	int w, h;
	GetSize(&w, &h);
	
	dc.SetPen(*wxGREY_PEN);
	dc.SetBrush(*wxLIGHT_GREY_BRUSH);
	wxPoint p1h[] = {
		wxPoint(margin, h / 2 - scale),
		wxPoint(w-margin+1, h / 2 - scale),
		wxPoint(w - margin + 1, h / 2 + 2 * scale),
		wxPoint(margin, h / 2 + 2 * scale)
	};
	wxPoint p1v[] = {
		wxPoint(w / 2 - scale, margin),
		wxPoint(w / 2 - scale, h - margin + 1),
		wxPoint(w / 2 + 2 * scale, h - margin + 1),
		wxPoint(w / 2 + 2 * scale, margin)
	};
	int num = 4;
	dc.DrawPolygon(num, horizontal_ ? p1h : p1v, wxODDEVEN_RULE);

	//left slider:
	int posl = std::max(minval, leftval);
	posl = std::round(double(posl - minval) * ((horizontal_?w:h) - margin * 2) / (maxval - minval));
	//right slider:
	int posr = std::min(maxval, rightval);
	posr = std::round(double(posr - minval) * ((horizontal_ ? w : h) - margin * 2) / (maxval - minval));

	//range color
	if (use_range_color_)
	{
		dc.SetPen(range_color_);
		dc.SetBrush(range_color_);
		wxPoint p2h[] = {
			wxPoint(margin + posl, h / 2 - scale),
			wxPoint(margin + posr, h / 2 - scale),
			wxPoint(margin + posr, h / 2 + 2 * scale),
			wxPoint(margin + posl, h / 2 + 2 * scale)
		};
		wxPoint p2v[] = {
			wxPoint(w / 2 - scale, margin + posl),
			wxPoint(w / 2 - scale, margin + posr),
			wxPoint(w / 2 + 2 * scale, margin + posr),
			wxPoint(w / 2 + 2 * scale, margin + posl)
		};
		num = 4;
		dc.DrawPolygon(num, horizontal_ ? p2h : p2v, wxODDEVEN_RULE);
	}

	if (horizontal_)
	{
		if (!use_thumb_color_)
		{
			wxColor color = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
			dc.SetPen(color);
			dc.SetBrush(wxBrush(color, wxBRUSHSTYLE_SOLID));
		}
		if (use_thumb_color_)
		{
			dc.SetPen(thumb_color1_);
			dc.SetBrush(wxBrush(thumb_color1_, wxBRUSHSTYLE_SOLID));
		}
		DrawThumb(dc, margin + posl, h * 0.5);
		if (use_thumb_color_)
		{
			dc.SetPen(thumb_color2_);
			dc.SetBrush(wxBrush(thumb_color2_, wxBRUSHSTYLE_SOLID));
		}
		DrawThumb(dc, margin + posr, h * 0.5);
	}
	else
	{
		if (!use_thumb_color_)
		{
			wxColor color = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
			dc.SetPen(color);
			dc.SetBrush(wxBrush(color, wxBRUSHSTYLE_SOLID));
		}
		if (use_thumb_color_)
		{
			dc.SetPen(thumb_color1_);
			dc.SetBrush(wxBrush(thumb_color1_, wxBRUSHSTYLE_SOLID));
		}
		DrawThumb(dc, w * 0.5, margin + posl);
		if (use_thumb_color_)
		{
			dc.SetPen(thumb_color2_);
			dc.SetBrush(wxBrush(thumb_color2_, wxBRUSHSTYLE_SOLID));
		}
		DrawThumb(dc, w * 0.5, margin + posr);
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
	int posl = std::max(minval, leftval);
	posl = std::round(double(posl - minval) * ((horizontal_ ? w : h) - margin * 2) / (maxval - minval)) + margin;
	//right slider:
	int posr = std::min(maxval, rightval);
	posr = std::round(double(posr - minval) * ((horizontal_ ? w : h) - margin * 2) / (maxval - minval)) + margin;

	int distl = std::abs((horizontal_?pos.x:pos.y) - posl);
	int distr = std::abs((horizontal_ ? pos.x : pos.y) - posr);
	if (distl <= distr)
		sel_ = 1;
	else
		sel_ = 2;
	last_sel_ = sel_;

	posl = margin;
	posr = (horizontal_ ? w : h) - margin;
	int val = std::round(double(minval) + double(maxval - minval) *
		((horizontal_ ? pos.x : pos.y) - posl) / (posr - posl));

	if (sel_ == 1)
	{
		leftval = val;
		if (leftval < minval)  leftval = minval;
		if (leftval >= rightval) leftval = rightval - 1;
	}
	else if (sel_ == 2)
	{
		rightval = val;
		if (rightval > maxval) rightval = maxval;
		if (rightval <= leftval) rightval = leftval + 1;
	}

	Refresh();
	Update();

	event.Skip();
}

void wxDoubleSlider::OnMotion(wxMouseEvent& event)
{
	if (sel_ != 0)
	{
		int w, h;
		wxClientDC dc(this);
		dc.GetSize(&w, &h);
		wxPoint pos = event.GetLogicalPosition(dc);

		int posl = margin;
		int posr = (horizontal_ ? w : h) - margin;
		int val = std::round(double(minval) + double(maxval - minval) *
			((horizontal_ ? pos.x : pos.y) - posl) / (posr - posl));

		if (sel_ == 1)
		{
			leftval = val;
			if (leftval < minval)  leftval = minval;
			if (leftval >= rightval) leftval = rightval - 1;
		}
		else if (sel_ == 2)
		{
			rightval = val;
			if (rightval > maxval) rightval = maxval;
			if (rightval <= leftval) rightval = leftval + 1;
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
		wxCommandEvent e(wxEVT_SCROLL_CHANGED, id_);
		e.SetEventObject(this);
		e.SetString("update");
		ProcessWindowEvent(e);
		wxPostEvent(parent_, e);

	}
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
		leftval -= m;
		if (leftval < minval)  leftval = minval;
		if (leftval >= rightval) leftval = rightval-1;
	}
	else
	{
		rightval -= m;
		if (rightval > maxval) rightval = maxval;
		if (rightval <= leftval) rightval = leftval+1;
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
	minval = min_val;
	maxval = max_val;
}

int wxDoubleSlider::GetMax()
{
	return maxval;
}

int wxDoubleSlider::GetMin()
{
	return minval;
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
