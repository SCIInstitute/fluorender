

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
	wxPoint p[] = {
		wxPoint(margin, h / 2 - scale),
		wxPoint(w-margin+1, h / 2 - scale),
		wxPoint(w - margin + 1, h / 2 + 2 * scale),
		wxPoint(margin, h / 2 + 2 * scale)
	};
	int num = 4;
	dc.DrawPolygon(num, p, wxODDEVEN_RULE);

	//left slider:
	int posl = std::max(minval, leftval);
	posl = std::round(double(posl - leftval) * (w - margin * 2) / (maxval - minval));
	DrawThumb(dc,margin + posl, h*0.5);
	//right slider:
	int posr = std::min(maxval, rightval);
	posr = std::round(double(posr - leftval) * (w - margin * 2) / (maxval - minval));
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
	posl = std::round(double(posl - leftval) * (w - margin * 2) / (maxval - minval)) + margin;
	//right slider:
	int posr = std::min(maxval, rightval);
	posr = std::round(double(posr - leftval) * (w - margin * 2) / (maxval - minval)) + margin;

	int distl = std::abs(pos.x - posl);
	int distr = std::abs(pos.x - posr);
	if (distl <= distr)
		selectedslider = 1;
	else
		selectedslider = 2;

	pos.x -= margin;
	pos.y = h-margin-pos.y;
	//if (pos.y < h/2) selectedslider = 1;
	//if (pos.y > h/2) selectedslider = 2;
	prevx = pos.x;
	prevy = pos.y;
	event.Skip();
}

void wxDoubleSlider::OnMotion(wxMouseEvent& event)
{
	int w, h;
	if (selectedslider != 0)
	{
		wxClientDC dc(this);
		dc.GetSize(&w, &h);
		wxPoint pos = event.GetLogicalPosition(dc);
		pos.x = pos.x-margin;
		pos.y = h-margin-pos.y;
		int m = prevx - pos.x;
		if (selectedslider == 1)
		{
			leftval -= m;
			if (leftval < minval)  leftval = minval;
			if (leftval >= rightval) leftval = rightval-1;
		}
		else if (selectedslider == 2)
		{
			rightval -= m;
			if (rightval > maxval) rightval = maxval;
			if (rightval <= leftval) rightval = leftval+1;
		}
		prevx = pos.x;
		prevy = pos.y;
		Refresh();
		Update();
	}
	event.Skip();
}

void wxDoubleSlider::OnLeftUp(wxMouseEvent& event)
{
	event.Skip();
	if (selectedslider != 0)
	{
		selectedslider = 0;
		wxCommandEvent e(wxEVT_SCROLL_CHANGED);
		e.SetEventObject(this);
		e.SetString("update");
		ProcessWindowEvent(e);
	}
}

void wxDoubleSlider::OnWheel(wxMouseEvent& event)
{
	int m, w, h;
	wxClientDC dc(this);
	dc.GetSize(&w, &h);
	wxPoint pos = event.GetLogicalPosition(dc);
	pos.x = pos.x-margin;
	pos.y = h-margin-pos.y;
	if (event.GetWheelRotation() > 0)
		m=1;
	else
		m=-1;
	if (pos.y < h/2) {
		leftval -= m;
		if (leftval < minval)  leftval = minval;
		if (leftval >= rightval) leftval = rightval-1;
	}
	else if (pos.y > h/2) {
		rightval -= m;
		if (rightval > maxval) rightval = maxval;
		if (rightval <= leftval) rightval = leftval+1;
	}
	Refresh();
	Update();
	wxCommandEvent e(wxEVT_SCROLL_CHANGED);
	e.SetEventObject(this);
	e.SetString("update");
	ProcessWindowEvent(e);
	event.Skip();
}










