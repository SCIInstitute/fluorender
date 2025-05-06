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
#include <wxFadeButton.h>
#include <wx/graphics.h>

wxFadeButton::wxFadeButton(wxWindow* parent, wxWindowID id, const wxString& label,
	const wxPoint& pos, const wxSize& size,
	long style, const wxValidator& validator,
	const wxString& name) :
	wxControl(parent, id, pos, size, style, validator, name),
	m_mode(0),
	m_fade_limit(20),
	m_label(label),
	m_pressed(false),
	m_hovered(false),
	HistoryIndicator()
{
	SetBackgroundStyle(wxBG_STYLE_PAINT);
	Bind(wxEVT_PAINT, &wxFadeButton::OnPaint, this);
	Bind(wxEVT_LEFT_DOWN, &wxFadeButton::OnLeftDown, this);
	Bind(wxEVT_LEFT_UP, &wxFadeButton::OnLeftUp, this);
	Bind(wxEVT_LEAVE_WINDOW, &wxFadeButton::OnLeave, this);
	Bind(wxEVT_ENTER_WINDOW, &wxFadeButton::OnEnter, this);
}

void wxFadeButton::SetFontBold(bool val)
{
	wxFont font = GetFont();
	font.SetWeight(val ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL);
	SetFont(font);
	Refresh();
}

void wxFadeButton::UpdateHistory()
{
	Refresh();
}

void wxFadeButton::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc(this);
	wxSize size = GetClientSize();
	if (size.GetWidth() <= 0 || size.GetHeight() <= 0)
		return;

	// Calculate the fade factor based on history size
	int fadeFactor = std::min(static_cast<int>(m_length), m_fade_limit);
	double interpolationFactor = m_fade_limit > 0 ? static_cast<double>(fadeFactor) / m_fade_limit : 0;

	switch (m_mode)
	{
	case 0://normal
		DrawNormal(dc);
		break;
	case 1://tint
		DrawTint(dc, interpolationFactor);
		break;
	case 2://rainbow
		DrawRainbow(dc, interpolationFactor);
		break;
	}

	// Draw the button text
	wxRect rect = GetClientRect();
	dc.SetTextForeground(GetForegroundColour());
	wxFont font = GetFont();
	dc.SetFont(font);
	//dc.DrawText(m_label, 0, 0);
	dc.DrawLabel(m_label, rect, wxALIGN_CENTER | wxALIGN_CENTER_VERTICAL);

	// Draw the button border if pressed or hovered
	if (m_hovered || m_pressed)
	{
		DrawHover(dc, rect);
	}
}

void wxFadeButton::OnLeftDown(wxMouseEvent& event) {
	m_pressed = true;
	Refresh();
	event.Skip();
}

void wxFadeButton::OnLeftUp(wxMouseEvent& event) {
	m_pressed = false;
	Refresh();
	event.Skip();
}

void wxFadeButton::OnLeave(wxMouseEvent& event) {
	m_hovered = false;
	Refresh();
	event.Skip();
}

void wxFadeButton::OnEnter(wxMouseEvent& event) {
	m_hovered = true;
	Refresh();
	event.Skip();
}

void wxFadeButton::DrawNormal(wxPaintDC& dc)
{
	// Get the default background color
	wxColour defaultBgColor = GetBackgroundColour();
	
	dc.SetBrush(wxBrush(defaultBgColor));
	dc.SetPen(*wxTRANSPARENT_PEN); // No border
	dc.DrawRectangle(GetClientRect()); // Fill the client area
}

void wxFadeButton::DrawTint(wxPaintDC& dc, double f)
{
	wxSize size = GetClientSize();
	int width = size.GetWidth();
	int height = size.GetHeight();

	// Get the default background color
	wxColour defaultBgColor = GetBackgroundColour();

	// Calculate color1 by interpolating between m_tint and defaultBgColor
	wxColour color1(
		static_cast<unsigned char>(m_tint.Red() + f * (defaultBgColor.Red() - m_tint.Red())),
		static_cast<unsigned char>(m_tint.Green() + f * (defaultBgColor.Green() - m_tint.Green())),
		static_cast<unsigned char>(m_tint.Blue() + f * (defaultBgColor.Blue() - m_tint.Blue()))
	);

	// Create a gradient brush
	wxGraphicsContext* gc = wxGraphicsContext::Create(dc);
	if (gc) {
		wxGraphicsGradientStops stops(color1, defaultBgColor);
		stops.Add(color1, 0.0f);
		stops.Add(defaultBgColor, 0.45f);
		stops.Add(defaultBgColor, 0.55f);
		stops.Add(color1, 1.0f);

		wxGraphicsPath path = gc->CreatePath();
		path.AddRectangle(0, 0, width, height);
		gc->SetBrush(gc->CreateLinearGradientBrush(0, 0, width, 0, stops));
		gc->FillPath(path);

		delete gc;
	}
}

void wxFadeButton::DrawRainbow(wxPaintDC& dc, double f)
{
	wxSize size = GetClientSize();
	int width = size.GetWidth();
	int height = size.GetHeight();

	// Get the default background color
	wxColour defaultBgColor = GetBackgroundColour();

	wxColour red(255, 200, 200);
	wxColour green(200, 255, 200);
	wxColour blue(200, 200, 255);

	wxColour attenuatedRed(
		static_cast<unsigned char>(red.Red() + f * (defaultBgColor.Red() - red.Red())),
		static_cast<unsigned char>(red.Green() + f * (defaultBgColor.Green() - red.Green())),
		static_cast<unsigned char>(red.Blue() + f * (defaultBgColor.Blue() - red.Blue()))
	);

	wxColour attenuatedGreen(
		static_cast<unsigned char>(green.Red() + f * (defaultBgColor.Red() - green.Red())),
		static_cast<unsigned char>(green.Green() + f * (defaultBgColor.Green() - green.Green())),
		static_cast<unsigned char>(green.Blue() + f * (defaultBgColor.Blue() - green.Blue()))
	);

	wxColour attenuatedBlue(
		static_cast<unsigned char>(blue.Red() + f * (defaultBgColor.Red() - blue.Red())),
		static_cast<unsigned char>(blue.Green() + f * (defaultBgColor.Green() - blue.Green())),
		static_cast<unsigned char>(blue.Blue() + f * (defaultBgColor.Blue() - blue.Blue()))
	);

	wxGraphicsContext* gc = wxGraphicsContext::Create(dc);
	if (gc) {
		wxGraphicsGradientStops stops(attenuatedRed, attenuatedBlue);
		stops.Add(attenuatedRed, 0.0);
		stops.Add(attenuatedGreen, 0.5);
		stops.Add(attenuatedBlue, 1.0);

		wxGraphicsPath path = gc->CreatePath();
		path.AddRectangle(0, 0, width, height);
		gc->SetBrush(gc->CreateLinearGradientBrush(0, 0, width, 0, stops));
		gc->FillPath(path);

		delete gc;
	}
}

void wxFadeButton::DrawHover(wxPaintDC& dc, const wxRect& rect)
{
	wxGraphicsContext* gc = wxGraphicsContext::Create(dc);

	if (gc)
	{
		wxColour fillColour = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);

		gc->SetPen(wxPen(fillColour)); // Set the pen for the outline
		int alpha = m_pressed ? 100 : 64;
		wxBrush semiTransparentBrush(wxColour(fillColour.Red(), fillColour.Green(), fillColour.Blue(), alpha));
		gc->SetBrush(semiTransparentBrush);

		// Draw the rectangle
		gc->DrawRectangle(rect.x, rect.y, rect.width-1, rect.height-1);

		delete gc; // Clean up the graphics context
	}
}