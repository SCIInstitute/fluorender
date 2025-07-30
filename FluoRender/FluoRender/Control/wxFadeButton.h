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
#ifndef _FADEBUTTON_H_
#define _FADEBUTTON_H_

#include <wx/wx.h>
#include <wx/control.h>
#include <HistoryIndicator.h>

class wxFadeButton : public wxControl, public HistoryIndicator
{
public:
	wxFadeButton(wxWindow* parent, wxWindowID id, const wxString& label,
		const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
		long style = 0, const wxValidator& validator = wxDefaultValidator,
		const wxString& name = wxButtonNameStr);

	void SetMode(int mode) { m_mode = mode; }
	void SetTintColor(const wxColour& c) { m_tint = c; m_mode = 1; }
	void SetFadeLimit(int val) { m_fade_limit = val; }
	void SetFontBold(bool val = true);

	virtual void UpdateHistory() override;

	virtual bool Disable();
	virtual bool Enable(bool enable = true);

protected:
	virtual void OnPaint(wxPaintEvent& event);
	virtual void OnLeftDown(wxMouseEvent& event);
	virtual void OnLeftUp(wxMouseEvent& event);
	virtual void OnLeave(wxMouseEvent& event);
	virtual void OnEnter(wxMouseEvent& event);

private:
	int m_mode;//0: normal; 1: color gradient; 2: rainbow gradient
	int m_fade_limit;
	wxColour m_tint;
	wxString m_label;
	bool m_pressed;
	bool m_hovered;
	bool m_enabled;

private:
	void DrawNormal(wxPaintDC& dc);
	void DrawTint(wxPaintDC& dc, double f);
	void DrawRainbow(wxPaintDC& dc, double f);
	void DrawHover(wxPaintDC& dc, const wxRect& rect);

	wxColour ToGrayscale(const wxColour& color) {
		// Calculate the luminance using the weighted sum of the RGB components
		unsigned char gray = static_cast<unsigned char>(0.299 * color.Red() +
			0.587 * color.Green() +
			0.114 * color.Blue());
		// Create and return the grayscale color
		return wxColour(gray, gray, gray);
	}

};

#endif//_FADEBUTTON_H_
