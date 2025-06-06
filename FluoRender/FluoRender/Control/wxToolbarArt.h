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

#include <wx/aui/auibar.h>
#include <wx/settings.h>
#include <wx/dc.h>
#include <wx/graphics.h>

class wxToolbarArt : public wxAuiGenericToolBarArt
{
public:
	wxAuiToolBarArt* Clone() override { return new wxToolbarArt(*this); }

	void DrawBackground(wxDC& dc, wxWindow* wnd, const wxRect& rect) override
	{
		wxPaintDC* paintDC = dynamic_cast<wxPaintDC*>(&dc);
		if (!paintDC)
			return;

		std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(*paintDC));
		if (!gc)
			return;

		wxColour color1 = wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE);
		int r = color1.Red();
		int g = color1.Green();
		int b = color1.Blue();
		int brightness = (r * 299 + g * 587 + b * 114) / 1000;
		wxColour color2;
		int offset = 20; // Adjust this value to change the brightness difference
		if (brightness > 128)
			color2 = wxColour(std::max(0, r - offset), std::max(0, g - offset), std::max(0, b - offset));
		else
			color2 = wxColour(std::min(255, r + offset), std::min(255, g + offset), std::min(255, b + offset));
		
		// Create a gradient brush
		wxGraphicsGradientStops stops(color1, color1);
		stops.Add(color2, 0.0f);
		stops.Add(color1, 0.5f);
		stops.Add(color1, 0.7f);
		stops.Add(color2, 0.97f);
		stops.Add(color2, 1.0f);

		wxGraphicsPath path = gc->CreatePath();
		path.AddRectangle(0, 0, rect.width, rect.height);
		gc->SetBrush(gc->CreateLinearGradientBrush(0, 0, 0, rect.height, stops));
		gc->FillPath(path);
	}
};
