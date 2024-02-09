

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
	wxControl(parent, id, pos,
		wxSize(1, int(std::round(23* parent->GetDPIScaleFactor()))), wxBORDER_NONE)
{
	scale = parent->GetDPIScaleFactor();
	margin = int(std::round(12 * scale));
	SetBackgroundColour(parent->GetBackgroundColour());
	SetDoubleBuffered(true);
	leftval = leftValue; rightval = rightValue; minval = minValue; maxval = maxValue;
	selectedslider = 0;
	floatlabel = false;
	Bind(wxEVT_PAINT, &wxDoubleSlider::OnPaint, this);
	Bind(wxEVT_LEFT_DOWN,&wxDoubleSlider::OnLeftDown, this);
	Bind(wxEVT_MOTION,&wxDoubleSlider::OnMotion, this);
	Bind(wxEVT_LEFT_UP,&wxDoubleSlider::OnLeftUp, this);
	Bind(wxEVT_LEAVE_WINDOW, &wxDoubleSlider::OnLeftUp, this);
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
	wxColor color = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
	dc.SetPen(color);
	dc.SetBrush(wxBrush(color, wxBRUSHSTYLE_SOLID));
	wxPoint p[] = {
		wxPoint(x - 5 * scale, y - 7 * scale),
		wxPoint(x + 5 * scale, y - 7 * scale),
		wxPoint(x + 5 * scale, y + 6 * scale),
		wxPoint(x, y + 11 * scale),
		wxPoint(x - 5 * scale, y + 6 * scale)
	};
	int num = 5;
	dc.DrawPolygon(num, p, wxODDEVEN_RULE);
}

void wxDoubleSlider::render(wxDC& dc)
{
	int w, h;
	GetSize(&w, &h);
	int tw, th;
	
	dc.SetPen(*wxGREY_PEN);
	dc.SetBrush(*wxLIGHT_GREY_BRUSH);
	wxPoint p1[] = {
		wxPoint(margin, h / 2 - scale),
		wxPoint(w-margin+1, h / 2 - scale),
		wxPoint(w - margin + 1, h / 2 + 2 * scale),
		wxPoint(margin, h / 2 + 2 * scale)
	};
	int num = 4;
	dc.DrawPolygon(num, p1, wxODDEVEN_RULE);

	//left slider:
	int posl = std::max(minval, leftval);
	posl = std::round(double(posl - minval) * (w - margin * 2) / (maxval - minval));
	//right slider:
	int posr = std::min(maxval, rightval);
	posr = std::round(double(posr - minval) * (w - margin * 2) / (maxval - minval));

	//range color
	dc.SetPen(range_color_);
	dc.SetBrush(range_color_);
	wxPoint p2[] = {
		wxPoint(margin + posl, h / 2 - scale),
		wxPoint(margin + posr, h / 2 - scale),
		wxPoint(margin + posr, h / 2 + 2 * scale),
		wxPoint(margin + posl, h / 2 + 2 * scale)
	};
	num = 4;
	dc.DrawPolygon(num, p2, wxODDEVEN_RULE);


	DrawThumb(dc,margin + posl, h*0.5);
	DrawThumb(dc, margin + posr, h * 0.5);
}


void wxDoubleSlider::OnLeftDown(wxMouseEvent& event)
{
	int w, h;
	wxClientDC dc(this);
	dc.GetSize(&w, &h);
	wxPoint pos = event.GetLogicalPosition(dc);

	//left slider:
	int posl = std::max(minval, leftval);
	posl = std::round(double(posl - minval) * (w - margin * 2) / (maxval - minval)) + margin;
	//right slider:
	int posr = std::min(maxval, rightval);
	posr = std::round(double(posr - minval) * (w - margin * 2) / (maxval - minval)) + margin;

	int distl = std::abs(pos.x - posl);
	int distr = std::abs(pos.x - posr);
	if (distl <= distr)
		selectedslider = 1;
	else
		selectedslider = 2;

	posl = margin;
	posr = w - margin;
	int val = std::round(double(minval) + double(maxval - minval) * (pos.x - posl) / (posr - posl));

	if (selectedslider == 1)
	{
		leftval = val;
		if (leftval < minval)  leftval = minval;
		if (leftval >= rightval) leftval = rightval - 1;
	}
	else if (selectedslider == 2)
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
	if (selectedslider != 0)
	{
		int w, h;
		wxClientDC dc(this);
		dc.GetSize(&w, &h);
		wxPoint pos = event.GetLogicalPosition(dc);

		int posl = margin;
		int posr = w - margin;
		int val = std::round(double(minval) + double(maxval - minval) * (pos.x - posl) / (posr - posl));

		if (selectedslider == 1)
		{
			leftval = val;
			if (leftval < minval)  leftval = minval;
			if (leftval >= rightval) leftval = rightval-1;
		}
		else if (selectedslider == 2)
		{
			rightval = val;
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
	}
	event.Skip();
}

void wxDoubleSlider::OnLeftUp(wxMouseEvent& event)
{
	event.Skip();
	if (selectedslider != 0)
	{
		selectedslider = 0;
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
	//pos.x = pos.x-margin;
	//pos.y = h-margin-pos.y;
	if (event.GetWheelRotation() > 0)
		m=-1;
	else
		m=1;
	if (pos.y >= h/2) {
		leftval -= m;
		if (leftval < minval)  leftval = minval;
		if (leftval >= rightval) leftval = rightval-1;
	}
	else if (pos.y < h/2) {
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

void wxDoubleSlider::SetRangeColor(const wxColor& c)
{
	range_color_ = c;
	use_range_color_ = true;
}

void wxDoubleSlider::DisableRangeColor()
{
	use_range_color_ = false;
}