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
#include <tif_capture.h>
#include <compatibility.h>
#include <tiffio.h>
#include <cstring>

struct TifCapture::Impl
{
    TIFF* out = nullptr;
    void* buf = nullptr;
    int w, h, chann;
    bool fp32;
    float dpi;
    bool compress;

    Impl(const std::wstring& filename, int width, int height, int channels, bool isFloat, float dpiVal, bool useCompress)
        : w(width), h(height), chann(channels), fp32(isFloat), dpi(dpiVal), compress(useCompress)
    {
        out = TIFFOpenW(filename.c_str(), "wb");
    }

    ~Impl() {
        if (out) TIFFClose(out);
        if (buf) _TIFFfree(buf);
    }

    bool isOpen() const { return out != nullptr; }

    bool write(const void* image, bool flipVertically)
    {
        if (!out) return false;

        TIFFSetField(out, TIFFTAG_IMAGEWIDTH, w);
        TIFFSetField(out, TIFFTAG_IMAGELENGTH, h);
        TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, chann);
        TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, fp32 ? 32 : 8);
        TIFFSetField(out, TIFFTAG_SAMPLEFORMAT, fp32 ? SAMPLEFORMAT_IEEEFP : SAMPLEFORMAT_UINT);
        TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
        TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
        TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
        TIFFSetField(out, TIFFTAG_XRESOLUTION, dpi);
        TIFFSetField(out, TIFFTAG_YRESOLUTION, dpi);
        TIFFSetField(out, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);
        if (compress)
            TIFFSetField(out, TIFFTAG_COMPRESSION, COMPRESSION_LZW);

        tsize_t linebytes = chann * w * (fp32 ? 4 : 1);
        if (!buf) buf = _TIFFmalloc(linebytes);
        if (!buf) return false;

        for (uint32_t row = 0; row < (uint32_t)h; ++row) {
            uint32_t srcRow = flipVertically ? (h - row - 1) : row;
            const void* srcLine = static_cast<const uint8_t*>(image) + srcRow * linebytes;
            std::memcpy(buf, srcLine, linebytes);
            if (TIFFWriteScanline(out, buf, row, 0) < 0)
                return false;
        }

        return true;
    }
};

TifCapture::~TifCapture() { delete impl; }

bool TifCapture::Write()
{
	impl = new Impl(m_filename, m_width, m_height, m_channels, m_is_float, m_dpi_val, m_use_compression);
    return impl->write(m_data, m_flipVertically);
}
