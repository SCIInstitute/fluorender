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
#include <image_capture_factory.h>
#include <tif_capture.h>
#include <png_capture.h>
#include <jpg_capture.h>
#include <algorithm>

std::unique_ptr<ImageCapture> CreateImageCapture(const std::wstring& filename)
{
	auto ext_pos = filename.find_last_of(L'.');
	if (ext_pos == std::string::npos) return nullptr;
	std::wstring ext = filename.substr(ext_pos + 1);
	std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

	if (ext == L"tiff" || ext == L"tif") return std::make_unique<TifCapture>();
	if (ext == L"png") return std::make_unique<PngCapture>();
	if (ext == L"jpg" || ext == L"jpeg") return std::make_unique<JpgCapture>();
	return nullptr;
}
