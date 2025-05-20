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
#ifndef PNG_RESOURCE_H
#define PNG_RESOURCE_H

#include <wx/wx.h>
#include <wx/mstream.h>
#include <compatibility.h>

#define wxGetBitmap(name) _wxGetBitmapBundleFromMemory(icons::name ## _png, sizeof(icons::name ## _png))

inline wxBitmapBundle _wxGetBitmapBundleFromMemory(const unsigned char* data, int length)
{
	wxLogNull logNo;
	wxMemoryInputStream is(data, length);
	wxImage baseImage(is, wxBITMAP_TYPE_ANY, -1);

	if (!baseImage.IsOk())
		return wxBitmapBundle(); // Return empty bundle if image loading fails

	const wxSize baseSize = baseImage.GetSize();
	wxVector<wxBitmap> bitmaps;

	// Always include the original
	bitmaps.push_back(wxBitmap(baseImage));

	// Common Windows DPI scale factors
	const double scales[] = { 1.8, 2.0, 2.5, 3.0 };

	for (double scale : scales)
	{
		wxImage scaled = baseImage.Copy();
		scaled.Rescale(baseSize.GetWidth() * scale, baseSize.GetHeight() * scale, wxIMAGE_QUALITY_HIGH);
		bitmaps.push_back(wxBitmap(scaled));
	}

	return wxBitmapBundle::FromBitmaps(bitmaps);
}

inline wxBitmap _wxGetBitmapFromMemory(const unsigned char *data, int length)
{
	wxLogNull logNo;
	wxMemoryInputStream is(data, length);
	return wxBitmap(wxImage(is, wxBITMAP_TYPE_ANY, -1), -1);
}

inline wxBitmap _wxGetBitmap(const unsigned char* data, int length, double f)
{
	wxLogNull logNo;
	wxMemoryInputStream is(data, length);
	wxImage image(is, wxBITMAP_TYPE_ANY, -1);
	if (f < 0.9 || f > 1.1)
		image.Rescale(image.GetWidth() * f, image.GetHeight() * f, wxIMAGE_QUALITY_BICUBIC);
	return wxBitmap(image, -1);
}

inline wxBitmapBundle _wxGetBitmapBundle(const unsigned char* data, int length, double f)
{
	wxLogNull logNo;
	wxMemoryInputStream is(data, length);
	wxImage image(is, wxBITMAP_TYPE_ANY, -1);
	if (f < 0.9 || f > 1.1)
		image.Rescale(image.GetWidth() * f, image.GetHeight() * f, wxIMAGE_QUALITY_BICUBIC);
	wxBitmap bitmap(image, -1);
	return wxBitmapBundle::FromBitmap(bitmap);
}

#endif//PNG_RESOURCE_H
