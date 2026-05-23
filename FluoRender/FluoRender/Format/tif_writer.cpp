/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2026 Scientific Computing and Imaging Institute,
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
#include <tif_writer.h>
#include <compatibility.h>
#include <tiffio.h>
#include <sstream>

TIFWriter::TIFWriter()
{
	m_data = 0;
	m_use_spacings = false;
	m_compression = false;
}

TIFWriter::~TIFWriter()
{
}

void TIFWriter::SetData(const std::shared_ptr<fluo::RawData>& data)
{
	m_data = data;
}

void TIFWriter::SetSpacing(const fluo::Vector& spc)
{
	m_spc = spc;
	m_use_spacings = true;
}

void TIFWriter::SetCompression(bool value)
{
	m_compression = value;
}

void TIFWriter::Save(const std::wstring& filename, int mode)
{
	switch (mode)
	{
	case 0://single file
		SaveSingleFile(filename);
		break;
	case 1://file sequence
		SaveSequence(filename);
		break;
	}
}

void TIFWriter::SaveSingleFile(const std::wstring& filename)
{
	if (!m_data || !m_data->IsAllocated())
		return;

	const auto& size = m_data->GetSize();
	if (size[2] == 0)
		return;

	const int bits = GetTiffBits(*m_data);
	if (bits == 0)
		return; // unsupported format for TIFF

	const int numPages = int(size[2]);
	const int width = int(size[0]);
	const int height = int(size[1]);
	const int samples = 1;

	// Resolution
	float x_res, y_res;
	double z_res;
	if (m_use_spacings)
	{
		x_res = float(m_spc.x() > 0.0 ? 1.0 / m_spc.x() : 1.0);
		y_res = float(m_spc.y() > 0.0 ? 1.0 / m_spc.y() : 1.0);
		z_res = m_spc.z();
	}
	else
	{
		x_res = y_res = 1.0f;
		z_res = 1.0;
	}

	// Line buffer
	std::vector<uint8_t> line8;
	std::vector<uint16_t> line16;
	if (bits == 8)
		line8.resize(width * samples);
	else
		line16.resize(width * samples);

	TIFF* tif = TIFFOpenW(filename.c_str(), "wb");
	if (!tif)
		return;

	for (int z = 0; z < numPages; ++z)
	{
		TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width);
		TIFFSetField(tif, TIFFTAG_IMAGELENGTH, height);
		TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bits);
		TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, samples);
		TIFFSetField(tif, TIFFTAG_XRESOLUTION, x_res);
		TIFFSetField(tif, TIFFTAG_YRESOLUTION, y_res);
		TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
		TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
		TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
		TIFFSetField(tif, TIFFTAG_SUBFILETYPE, FILETYPE_PAGE);
		TIFFSetField(tif, TIFFTAG_PAGENUMBER, z, numPages);

		if (m_compression)
			TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_LZW);

		std::ostringstream desc;
		desc << "ImageJ=1.52a\n";
		desc << "spacing=" << z_res << "\n";
		desc << "images=" << numPages << "\n";
		desc << "slices=" << numPages << "\n";
		desc << "loop=false";

		TIFFSetField(tif, TIFFTAG_IMAGEDESCRIPTION, desc.str().c_str());

		for (int y = 0; y < height; ++y)
		{
			const size_t index =
				(size_t(z) * width * height) + size_t(y) * width;

			if (bits == 8)
			{
				auto* src = m_data->DataAs<uint8_t>() + index;
				memcpy(line8.data(), src, width);
				TIFFWriteScanline(tif, line8.data(), y, 0);
			}
			else
			{
				auto* src = m_data->DataAs<uint16_t>() + index;
				memcpy(line16.data(), src, width * sizeof(uint16_t));
				TIFFWriteScanline(tif, line16.data(), y, 0);
			}
		}

		TIFFWriteDirectory(tif);
	}

	TIFFClose(tif);
}

void TIFWriter::SaveSequence(const std::wstring& filename)
{
	if (!m_data || !m_data->IsAllocated())
		return;

	const auto& size = m_data->GetSize();
	if (size[2] == 0)
		return;

	const int bits = GetTiffBits(*m_data);
	if (bits == 0)
		return;

	const int width = int(size[0]);
	const int height = int(size[1]);
	const int slices = int(size[2]);
	const int samples = 1;

	std::wstring base = filename;
	const size_t dot = base.find_last_of(L'.');
	if (dot != std::wstring::npos)
		base = base.substr(0, dot);

	float x_res = 1.0f, y_res = 1.0f;
	double z_res = 1.0;
	if (m_use_spacings)
	{
		x_res = float(m_spc.x() > 0.0 ? 1.0 / m_spc.x() : 1.0);
		y_res = float(m_spc.y() > 0.0 ? 1.0 / m_spc.y() : 1.0);
		z_res = m_spc.z();
	}

	std::vector<uint8_t> line8;
	std::vector<uint16_t> line16;
	if (bits == 8)
		line8.resize(width);
	else
		line16.resize(width);

	const int ndigit = int(std::log10(double(slices))) + 1;
	wchar_t fmt[16];
	swprintf_s(fmt, 32, L"%%0%dd", ndigit);

	for (int z = 0; z < slices; ++z)
	{
		wchar_t index[16];
		swprintf_s(index, 32, fmt, z + 1);
		std::wstring outname = base + index + L".tif";

		TIFF* tif = TIFFOpenW(outname.c_str(), "wb");
		if (!tif)
			continue;

		TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width);
		TIFFSetField(tif, TIFFTAG_IMAGELENGTH, height);
		TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bits);
		TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, samples);
		TIFFSetField(tif, TIFFTAG_XRESOLUTION, x_res);
		TIFFSetField(tif, TIFFTAG_YRESOLUTION, y_res);
		TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
		TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
		TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);

		if (m_compression)
			TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_LZW);

		std::ostringstream desc;
		desc << "ImageJ=1.52a\n";
		desc << "spacing=" << z_res << "\n";
		desc << "images=1\n";
		desc << "slices=1\n";
		desc << "loop=false";

		TIFFSetField(tif, TIFFTAG_IMAGEDESCRIPTION, desc.str().c_str());

		for (int y = 0; y < height; ++y)
		{
			const size_t index =
				(size_t(z) * width * height) + size_t(y) * width;

			if (bits == 8)
			{
				memcpy(line8.data(),
					m_data->DataAs<uint8_t>() + index,
					width);
				TIFFWriteScanline(tif, line8.data(), y, 0);
			}
			else
			{
				memcpy(line16.data(),
					m_data->DataAs<uint16_t>() + index,
					width * sizeof(uint16_t));
				TIFFWriteScanline(tif, line16.data(), y, 0);
			}
		}

		TIFFClose(tif);
	}
}
