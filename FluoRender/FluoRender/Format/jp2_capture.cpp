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
#include <jp2_capture.h>
#include <compatibility.h>
#include <openjpeg.h>
#include <cstdio>
#include <vector>
#include <cstring>

struct Jp2Capture::Impl
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
		// Convert filename to UTF-8
		std::string utf8_filename = ws2s(filename);

		const int numcomps = 3; // RGB only
		opj_image_cmptparm_t cmptparm[3] = {};
		for (int i = 0; i < numcomps; ++i) {
			cmptparm[i].dx = 1;
			cmptparm[i].dy = 1;
			cmptparm[i].w = w;
			cmptparm[i].h = h;
			cmptparm[i].x0 = 0;
			cmptparm[i].y0 = 0;
			cmptparm[i].prec = 8;
			cmptparm[i].bpp = 8;
			cmptparm[i].sgnd = 0;
		}

		opj_image_t* jp2_image = opj_image_create(numcomps, cmptparm, OPJ_CLRSPC_SRGB);
		if (!jp2_image) return false;

		jp2_image->x0 = 0;
		jp2_image->y0 = 0;
		jp2_image->x1 = w;
		jp2_image->y1 = h;

		// Fill image data
		const uint8_t* src = static_cast<const uint8_t*>(image);
		for (int y = 0; y < h; ++y) {
			int rowIndex = flipVertically ? (h - 1 - y) : y;
			const uint8_t* row = src + rowIndex * w * chann;

			for (int x = 0; x < w; ++x) {
				for (int c = 0; c < numcomps; ++c) {
					jp2_image->comps[c].data[y * w + x] = row[x * chann + c];
				}
			}
		}

		// Set encoder parameters
		opj_cparameters_t parameters;
		opj_set_default_encoder_parameters(&parameters);

		parameters.tcp_numlayers = 1;
		parameters.cp_disto_alloc = 1;
		parameters.numresolution = 6;
		parameters.cp_comment = const_cast<char*>("Captured by FluoRender");

		if (quality >= 100) {
			parameters.tcp_rates[0] = 0; // Lossless
		}
		else {
			float rate = 100.0f / std::max(1, quality); // e.g., quality=50 → rate=2.0
			parameters.tcp_rates[0] = rate;
		}

		// Create encoder
		opj_codec_t* codec = opj_create_compress(OPJ_CODEC_JP2);
		if (!codec) {
			opj_image_destroy(jp2_image);
			return false;
		}

		opj_setup_encoder(codec, &parameters, jp2_image);

		// Create output stream
		opj_stream_t* stream = opj_stream_create_default_file_stream(utf8_filename.c_str(), false);
		if (!stream) {
			opj_destroy_codec(codec);
			opj_image_destroy(jp2_image);
			return false;
		}

		// Encode and write
		bool success = opj_start_compress(codec, jp2_image, stream)
			&& opj_encode(codec, stream)
			&& opj_end_compress(codec, stream);

		opj_stream_destroy(stream);
		opj_destroy_codec(codec);
		opj_image_destroy(jp2_image);

		return success;
	}
};

Jp2Capture::~Jp2Capture() { delete impl; }

bool Jp2Capture::Write()
{
	impl = new Impl(m_filename, m_width, m_height, m_channels, m_quality);
	return impl->write(m_data, m_flipVertically);
}
