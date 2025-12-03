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

void TIFWriter::SetData(Nrrd *data)
{
	m_data = data;
}

void TIFWriter::SetSpacings(const fluo::Vector& spc)
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
	if (!m_data || !m_data->data || m_data->dim!=3)
		return;

	int numPages = int(m_data->axis[2].size);
	int width = int(m_data->axis[0].size);
	int height = int(m_data->axis[1].size);
	int samples = 1;
	int bits = m_data->type==nrrdTypeUShort?16:8;
	float x_res;
	float y_res;
	double z_res;
	if (m_use_spacings)
	{
		x_res = float(m_spc.x()>0.0 ? 1.0 / m_spc.x() : 1.0);
		y_res = float(m_spc.y()>0.0 ? 1.0 / m_spc.y() : 1.0);
		z_res = m_spc.z();
	}
	else
	{
		x_res = float(m_data->axis[0].spacing>0.0?1.0/m_data->axis[0].spacing:1.0);
		y_res = float(m_data->axis[1].spacing>0.0?1.0/m_data->axis[1].spacing:1.0);
		z_res = m_data->axis[2].spacing;
	}

	//buffers
	uint16_t *buf16 = 0;
	uint8_t *buf8 = 0;
	if (bits == 16)
	{
		buf16 = (uint16_t*)_TIFFmalloc(width*sizeof(uint16_t)*samples);
	}
	else
	{
		buf8 = (uint8_t*)_TIFFmalloc(width*sizeof(uint8_t)*samples);
	}

	TIFF* outfile = TIFFOpenW(filename, "wb");
	for (int i=0; i<numPages; i++)
	{
		TIFFSetField(outfile, TIFFTAG_IMAGEWIDTH, width);
		TIFFSetField(outfile, TIFFTAG_IMAGELENGTH, height);
		TIFFSetField(outfile, TIFFTAG_BITSPERSAMPLE, bits);
		TIFFSetField(outfile, TIFFTAG_SAMPLESPERPIXEL, samples);
		TIFFSetField(outfile, TIFFTAG_XRESOLUTION, x_res);
		TIFFSetField(outfile, TIFFTAG_YRESOLUTION, y_res);
		TIFFSetField(outfile, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
		TIFFSetField(outfile, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
		TIFFSetField(outfile, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
		TIFFSetField(outfile, TIFFTAG_SUBFILETYPE, FILETYPE_PAGE);
		TIFFSetField(outfile, TIFFTAG_PAGENUMBER, i);
		//TIFFSetField(outfile, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(outfile, 0));
		if (m_compression)
			TIFFSetField(outfile, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
		std::ostringstream strs;
		strs << "ImageJ=1.52a\n";
		strs << "spacing=" << z_res << "\n";
		strs << "images=" << numPages << "\n";
		strs << "slices=" << numPages << "\n";
		strs << "loop=false";
		std::string desc = strs.str();
		TIFFSetField(outfile, TIFFTAG_IMAGEDESCRIPTION, desc.c_str());

		for (int j=0; j<height; j++)
		{
			int lineindex = (width*height*i + width*j)*samples;
			if (bits == 16)
			{
				memcpy(buf16, ((uint16_t*)m_data->data)+lineindex, width*sizeof(uint16_t)*samples);
				TIFFWriteScanline(outfile, buf16, j, 0);
			}
			else
			{
				memcpy(buf8, ((uint8_t*)m_data->data)+lineindex, width*sizeof(uint8_t)*samples);
				TIFFWriteScanline(outfile, buf8, j, 0);
			}
		}

		TIFFWriteDirectory(outfile);
	}
	TIFFClose(outfile);
	if (buf16)
		_TIFFfree(buf16);
	if (buf8)
		_TIFFfree(buf8);
}

void TIFWriter::SaveSequence(const std::wstring& filename)
{
	if (!m_data || !m_data->data || m_data->dim!=3)
		return;

	std::wstring str_fn = filename;
	size_t pos = str_fn.find_last_of(L'.');
	if (pos != std::wstring::npos)
		str_fn = str_fn.substr(0, pos);

	int numPages = int(m_data->axis[2].size);
	int width = int(m_data->axis[0].size);
	int height = int(m_data->axis[1].size);
	int samples = 1;
	int bits = m_data->type==nrrdTypeUShort?16:8;
	float x_res;
	float y_res;
	double z_res;
	if (m_use_spacings)
	{
		x_res = float(m_spc.x()>0.0?1.0/m_spc.x():1.0);
		y_res = float(m_spc.y()>0.0?1.0/m_spc.y():1.0);
		z_res = m_spc.z();
	}
	else
	{
		x_res = float(m_data->axis[0].spacing>0.0?1.0/m_data->axis[0].spacing:1.0);
		y_res = float(m_data->axis[1].spacing>0.0?1.0/m_data->axis[1].spacing:1.0);
		z_res = m_data->axis[2].spacing;
	}

	//buffers
	uint16_t *buf16 = 0;
	uint8_t *buf8 = 0;
	if (bits == 16)
	{
		buf16 = (uint16_t*)_TIFFmalloc(width*sizeof(uint16_t)*samples);
	}
	else
	{
		buf8 = (uint8_t*)_TIFFmalloc(width*sizeof(uint8_t)*samples);
	}

	for (int i=0; i<numPages; i++)
	{
		wchar_t fileindex[32];
		wchar_t format[32];
		int ndigit = int(log10(double(numPages))) + 1;
		swprintf_s(format, 32, L"%%0%dd", ndigit);
		swprintf_s(fileindex, 32, format, i+1);
		std::wstring pagefilename = str_fn + fileindex + L".tif";
		TIFF* outfile = TIFFOpenW(pagefilename, "wb");

		TIFFSetField(outfile, TIFFTAG_IMAGEWIDTH, width);
		TIFFSetField(outfile, TIFFTAG_IMAGELENGTH, height);
		TIFFSetField(outfile, TIFFTAG_BITSPERSAMPLE, bits);
		TIFFSetField(outfile, TIFFTAG_SAMPLESPERPIXEL, samples);
		TIFFSetField(outfile, TIFFTAG_XRESOLUTION, x_res);
		TIFFSetField(outfile, TIFFTAG_YRESOLUTION, y_res);
		TIFFSetField(outfile, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
		TIFFSetField(outfile, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
		TIFFSetField(outfile, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
		TIFFSetField(outfile, TIFFTAG_SUBFILETYPE, FILETYPE_PAGE);
		TIFFSetField(outfile, TIFFTAG_PAGENUMBER, 0);
		//TIFFSetField(outfile, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(outfile, 0));
		std::ostringstream strs;
		strs << "ImageJ=1.52a\n";
		strs << "spacing=" << z_res << "\n";
		strs << "images=" << 1 << "\n";
		strs << "slices=" << 1 << "\n";
		strs << "loop=false";
		std::string desc = strs.str();
		TIFFSetField(outfile, TIFFTAG_IMAGEDESCRIPTION, desc.c_str());
		if (m_compression)
			TIFFSetField(outfile, TIFFTAG_COMPRESSION, COMPRESSION_LZW);

		for (int j=0; j<height; j++)
		{
			int lineindex = (width*height*i + width*j)*samples;
			if (bits == 16)
			{
				memcpy(buf16, ((uint16_t*)m_data->data)+lineindex, width*sizeof(uint16_t)*samples);
				TIFFWriteScanline(outfile, buf16, j, 0);
			}
			else
			{
				memcpy(buf8, ((uint8_t*)m_data->data)+lineindex, width*sizeof(uint8_t)*samples);
				TIFFWriteScanline(outfile, buf8, j, 0);
			}
		}
		TIFFClose(outfile);
	}
	if (buf16)
		_TIFFfree(buf16);
	if (buf8)
		_TIFFfree(buf8);
}
