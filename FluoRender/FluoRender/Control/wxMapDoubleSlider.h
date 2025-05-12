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
#ifndef _WXMAPDOUBLESLIDER_H_
#define _WXMAPDOUBLESLIDER_H_

#include <wxDoubleSlider.h>

class wxMapDoubleSlider : public wxDoubleSlider
{
public:
	wxMapDoubleSlider(
		wxWindow* parent,
		wxWindowID id,
		int lowValue, int hiValue, int minValue, int maxValue,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxSL_HORIZONTAL,
		const wxValidator& val = wxDefaultValidator,
		const wxString& name = "wxDoubleSlider");

	void SetMapData(const std::vector<unsigned char>& rgbData);
	void SetColors(const wxColour& lc, const wxColour& hc) { m_low_color = lc; m_high_color = hc; }
	void SetMode(int ival) { m_mode = ival; }

protected:
	int m_mode;	//0:range only; 1:threshold with gray low and high
				//2:minmax with low and high solid colors
				//3:colormap rescale within range, solid colors outside
	wxColour m_low_color;
	wxColour m_high_color;
	std::vector<unsigned char> m_rgbData;
	wxBitmap m_colorBitmap;
	wxBitmap m_grayBitmap;

protected:
	virtual void renderNormal(wxDC& dc);
	virtual void renderInverse(wxDC& dc);

	void GenerateBaseImage();
};

#endif//_WXMAPDOUBLESLIDER_H_