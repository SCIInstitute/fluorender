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
#include <jpg_capture.h>
#include <compatibility.h>
#include <jpeglib.h>
#include <cstdio>
#include <vector>
#include <cstring>

struct JpgCapture::Impl
{
	std::wstring filename;
	int w, h, chann, quality;
	bool open;

	Impl(const std::wstring& fname, int width, int height, int channels, int q)
		: filename(fname), w(width), h(height), chann(channels), quality(q), open(true) {
	}

	bool isOpen() const { return open; }

	bool write(const void* image, bool flipVertically)
	{
        FILE* fp = 0;
 		if (!WFOPEN(&fp, filename, L"wb")) return false;

		jpeg_compress_struct cinfo;
		jpeg_error_mgr jerr;
		cinfo.err = jpeg_std_error(&jerr);
		jpeg_create_compress(&cinfo);
		jpeg_stdio_dest(&cinfo, fp);

		cinfo.image_width = w;
		cinfo.image_height = h;
		cinfo.input_components = 3;  // JPEG only supports RGB
		cinfo.in_color_space = JCS_RGB;

		jpeg_set_defaults(&cinfo);
		jpeg_set_quality(&cinfo, quality, TRUE);
		jpeg_start_compress(&cinfo, TRUE);

		std::vector<uint8_t> rgbRow(w * 3);
		for (int y = 0; y < h; ++y) {
			int rowIndex = flipVertically ? (h - 1 - y) : y;
			const uint8_t* src = static_cast<const uint8_t*>(image) + rowIndex * w * chann;

			// Strip alpha if present
			for (int x = 0; x < w; ++x) {
				rgbRow[x * 3 + 0] = src[x * chann + 0];
				rgbRow[x * 3 + 1] = src[x * chann + 1];
				rgbRow[x * 3 + 2] = src[x * chann + 2];
			}

			JSAMPROW row_pointer = rgbRow.data();
			jpeg_write_scanlines(&cinfo, &row_pointer, 1);
		}

		jpeg_finish_compress(&cinfo);
		jpeg_destroy_compress(&cinfo);
		fclose(fp);
		return true;
	}
};

JpgCapture::~JpgCapture() { delete impl; }

bool JpgCapture::Write()
{
	impl = new Impl(m_filename, m_width, m_height, m_channels, m_quality);
	return impl->write(m_data, m_flipVertically);
}
