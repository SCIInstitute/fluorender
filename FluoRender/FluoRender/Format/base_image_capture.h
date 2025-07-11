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
#ifndef ImageCapture_h
#define ImageCapture_h

#include <string>
#include <vector>

class ImageCapture {
public:
	virtual ~ImageCapture() = default;
	virtual bool Write() = 0;
	void SetFilename(const std::wstring& filename) {
		m_filename = filename;
	}
	void SetData(const void* data, int width, int height, int channels) {
		m_data = data;
		m_width = width;
		m_height = height;
		m_channels = channels;
	}
	void SetFlipVertically(bool flip) {
		m_flipVertically = flip;
	}

	void SetIsFloat(bool is_float) {
		m_is_float = is_float;
	}

	void SetDpi(float dpi) {
		m_dpi_val = dpi;
	}

	void SetUseCompression(bool use_compression) {
		m_use_compression = use_compression;
	}

	void SetQuality(int quality) {
		m_quality = quality; // JPEG quality setting (0-100)
	}

protected:
	std::wstring m_filename;
	const void* m_data = nullptr;
	int m_width = 0;
	int m_height = 0;
	int m_channels = 0;
	bool m_flipVertically = false;

	//tiff
	bool m_is_float = false;
	float m_dpi_val = 72.0f;
	bool m_use_compression = false;

	//jpg
	int m_quality; // JPEG quality setting (0-100)
};

#endif// ImageCapture_h