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

wxFadeButton::wxFadeButton(wxWindow* parent, wxWindowID id, const wxString& label,
	const wxPoint& pos, const wxSize& size,
	long style = 0, const wxValidator& validator,
	const wxString& name) :
	wxButton(parent, id, label, pos, size, style, validator, name),
	m_mode(0),
	m_fade_limit(20),
	HistoryIndicator()
{

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
	wxWindow::Update();
}

void wxFadeButton::OnPaint(wxPaintEvent& event)
{
	UpdateButtonColor();
	event.skip();
}

void wxFadeButton::UpdateButtonColor()
{
	wxPaintDC dc(this);
	wxSize size = GetSize();
	int width = size.GetWidth();
	int height = size.GetHeight();

	// Calculate the fade factor based on history size
	int fadeFactor = std::min(static_cast<int>(m_historyLength), m_fade_limit);
	double interpolationFactor = static_cast<double>(fadeFactor) / m_fade_limit;

	// Get the default background color
	wxColour defaultBgColor = GetBackgroundColour();

	// Calculate color1 by interpolating between m_tint and defaultBgColor
	wxColour color1(
		static_cast<unsigned char>(m_tint.Red() + interpolationFactor * (defaultBgColor.Red() - m_tint.Red())),
		static_cast<unsigned char>(m_tint.Green() + interpolationFactor * (defaultBgColor.Green() - m_tint.Green())),
		static_cast<unsigned char>(m_tint.Blue() + interpolationFactor * (defaultBgColor.Blue() - m_tint.Blue()))
	);

	// Create a gradient brush
	wxGraphicsContext* gc = wxGraphicsContext::Create(dc);
	if (gc) {
		wxGraphicsGradientStops stops(color1, defaultBgColor);
		stops.Add(color1, 0.0);
		stops.Add(defaultBgColor, 0.5);
		stops.Add(color1, 1.0);

		wxGraphicsPath path = gc->CreatePath();
		path.AddRectangle(0, 0, width, height);
		gc->SetBrush(gc->CreateLinearGradientBrush(0, 0, width, 0, stops));
		gc->FillPath(path);

		delete gc;
	}
}