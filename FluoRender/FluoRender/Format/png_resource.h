﻿/*
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

#define wxGetBitmapFromMemory(name) _wxGetBitmapFromMemory(icons::name ## _png, sizeof(icons::name ## _png))

inline wxBitmap _wxGetBitmapFromMemory(const unsigned char *data, int length)
{
	wxLogNull logNo;
	wxMemoryInputStream is(data, length);
	return wxBitmap(wxImage(is, wxBITMAP_TYPE_ANY, -1), -1);
}

#define wxGetBitmap(name, f) _wxGetBitmap(icons::name ## _png, sizeof(icons::name ## _png), f)

inline wxBitmap _wxGetBitmap(const unsigned char* data, int length, double f)
{
	wxLogNull logNo;
	wxMemoryInputStream is(data, length);
	wxImage image(is, wxBITMAP_TYPE_ANY, -1);
	if (f < 0.9 || f > 1.1)
		image.Rescale(image.GetWidth() * f, image.GetHeight() * f, wxIMAGE_QUALITY_HIGH);
	return wxBitmap(image, -1);
}

#endif//PNG_RESOURCE_H
