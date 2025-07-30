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
#include <png_capture.h>
#include <compatibility.h>
#include <png.h>
#include <fstream>
#include <vector>
#include <cstring>

struct PngCapture::Impl
{
    std::wstring filename;
    int w, h, chann;
    bool open;

    Impl(const std::wstring& fname, int width, int height, int channels)
        : filename(fname), w(width), h(height), chann(channels), open(true) {}

    bool isOpen() const { return open; }

    bool write(const void* image, bool flipVertically)
    {
        FILE* fp = 0;
        if (!WFOPEN(&fp, filename, L"wb")) return false;

        png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        if (!png) return false;

        png_infop info = png_create_info_struct(png);
        if (!info) {
            png_destroy_write_struct(&png, nullptr);
            return false;
        }

        if (setjmp(png_jmpbuf(png))) {
            png_destroy_write_struct(&png, &info);
            fclose(fp);
            return false;
        }

        png_init_io(png, fp);
        png_set_IHDR(png, info, w, h, 8, chann == 4 ? PNG_COLOR_TYPE_RGBA : PNG_COLOR_TYPE_RGB,
                     PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
        png_write_info(png, info);

        std::vector<png_bytep> rows(h);
        for (int y = 0; y < h; ++y) {
            int rowIndex = flipVertically ? (h - 1 - y) : y;
            rows[y] = (png_bytep)image + rowIndex * w * chann;
        }

        png_write_image(png, rows.data());
        png_write_end(png, nullptr);
        png_destroy_write_struct(&png, &info);
        fclose(fp);
        return true;
    }
};

PngCapture::~PngCapture() { delete impl; }

bool PngCapture::Write()
{
	impl = new Impl(m_filename, m_width, m_height, m_channels);
    return impl->write(m_data, m_flipVertically);
}
