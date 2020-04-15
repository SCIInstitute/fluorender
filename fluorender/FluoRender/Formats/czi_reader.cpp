/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2018 Scientific Computing and Imaging Institute,
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
#include "czi_reader.h"
#include "../compatibility.h"
#include <wx/xml/xml.h>
#include <wx/sstream.h>
#include <stdio.h>
#include <algorithm>

std::vector<std::string> CZIReader::m_types{
	"ZISRAWFILE",
	"ZISRAWDIRECTORY",
	"ZISRAWSUBBLOCK",
	"ZISRAWMETADATA",
	"ZISRAWATTACH",
	"ZISRAWATTDIR",
	"DELETED"};

CZIReader::CZIReader():
	m_header_read(false),
	m_multi_file(false),
	m_file_part(0),
	m_dir_pos(0),
	m_meta_pos(0),
	m_att_dir(0)
{
	m_time_num = 0;
	m_cur_time = -1;
	m_chan_num = 0;
	m_slice_num = 0;
	m_x_size = 0;
	m_y_size = 0;

	m_valid_spc = false;
	m_xspc = 0.0;
	m_yspc = 0.0;
	m_zspc = 0.0;

	m_max_value = 0.0;
	m_scalar_scale = 1.0;

	m_batch = false;
	m_cur_batch = -1;

	//m_compression = 0;
	//m_predictor = 0;
	//m_version = 0;
	//m_datatype = 0;
	//m_l4gb = false;
}

CZIReader::~CZIReader()
{
}

void CZIReader::SetFile(string &file)
{
	if (!file.empty())
	{
		if (!m_path_name.empty())
			m_path_name.clear();
		m_path_name.assign(file.length(), L' ');
		copy(file.begin(), file.end(), m_path_name.begin());
	}
	m_id_string = m_path_name;
}

void CZIReader::SetFile(wstring &file)
{
	m_path_name = file;
	m_id_string = m_path_name;
}

int CZIReader::Preprocess()
{
	FILE* pfile = 0;
	if (!WFOPEN(&pfile, m_path_name.c_str(), L"rb"))
		return READER_OPEN_FAIL;

	unsigned long long ioffset = 0;
	//read header
	while (!feof(pfile))
		if (ReadSegment(pfile, ioffset, SegFile))
			break;
	if (m_header_read)
	{
		if (m_meta_pos)
			ReadSegment(pfile, m_meta_pos, SegMetadata);
		if (m_dir_pos)
			ReadSegment(pfile, m_dir_pos, SegDirectory);
	}

	fclose(pfile);

	m_cur_time = 0;
	m_data_name = GET_NAME(m_path_name);

	return READER_OK;
}

void CZIReader::SetSliceSeq(bool ss)
{
	//do nothing
}

bool CZIReader::GetSliceSeq()
{
	return false;
}

void CZIReader::SetChannSeq(bool cs)
{
	//do nothing
}

bool CZIReader::GetChannSeq()
{
	return false;
}

void CZIReader::SetDigitOrder(int order)
{
	//do nothing
}

int CZIReader::GetDigitOrder()
{
	return 0;
}

void CZIReader::SetTimeId(wstring &id)
{
	//do nothing
}

wstring CZIReader::GetTimeId()
{
	return wstring(L"");
}

void CZIReader::SetBatch(bool batch)
{
	if (batch)
	{
		//read the directory info
		FIND_FILES(m_path_name, L"*.lsm", m_batch_list, m_cur_batch);
		m_batch = true;
	}
	else
		m_batch = false;
}

int CZIReader::LoadBatch(int index)
{
	int result = -1;
	if (index >= 0 && index < (int)m_batch_list.size())
	{
		m_path_name = m_batch_list[index];
		Preprocess();
		result = index;
		m_cur_batch = result;
	}
	else
		result = -1;

	return result;
}

double CZIReader::GetExcitationWavelength(int chan)
{
	for (int i = 0; i < (int)m_excitation_wavelength_list.size(); i++)
	{
		if (m_excitation_wavelength_list[i].chan_num == chan)
			return m_excitation_wavelength_list[i].wavelength;
	}
	return 0.0;
}

Nrrd* CZIReader::Convert(int t, int c, bool get_max)
{
	Nrrd *data = 0;
	FILE* pfile = 0;
	if (!WFOPEN(&pfile, m_path_name.c_str(), L"rb"))
		return 0;

	return data;
}

wstring CZIReader::GetCurDataName(int t, int c)
{
	return m_path_name;
}

wstring CZIReader::GetCurMaskName(int t, int c)
{
	wostringstream woss;
	woss << m_path_name.substr(0, m_path_name.find_last_of('.'));
	if (m_time_num > 1) woss << "_T" << t;
	if (m_chan_num > 1) woss << "_C" << c;
	woss << ".msk";
	wstring mask_name = woss.str();
	return mask_name;
}

wstring CZIReader::GetCurLabelName(int t, int c)
{
	wostringstream woss;
	woss << m_path_name.substr(0, m_path_name.find_last_of('.'));
	if (m_time_num > 1) woss << "_T" << t;
	if (m_chan_num > 1) woss << "_C" << c;
	woss << ".lbl";
	wstring label_name = woss.str();
	return label_name;
}

bool CZIReader::ReadSegment(FILE* pfile, unsigned long long &ioffset, SegType readtype)
{
	if (FSEEK64(pfile, ioffset, SEEK_SET) != 0)
		return false;

	char id[16];
	unsigned long long alloc_size;
	unsigned long long used_size;
	bool result = true;
	result &= fread(id, 1, 16, pfile) == 16;
	result &= fread(&alloc_size, sizeof(unsigned long long), 1, pfile) == 1;
	result &= fread(&used_size, sizeof(unsigned long long), 1, pfile) == 1;
	if (!result)
		return false;

	std::string strid(id);
	auto it = std::find(m_types.begin(), m_types.end(), id);
	if (it == m_types.end())
	{
		ioffset += HDRSIZE + alloc_size;
		return false;
	}
	SegType type = (SegType)(std::distance(m_types.begin(), it));
	if (readtype != SegAll && type != readtype)
	{
		ioffset += HDRSIZE + alloc_size;
		return false;
	}
	result = false;
	switch (type)
	{
	case SegFile:
		result = ReadFile(pfile);
		break;
	case SegDirectory:
		result = ReadDirectory(pfile, ioffset + HDRSIZE);
		break;
	case SegSubBlock:
		result = ReadSubBlock(pfile, ioffset + HDRSIZE);
		break;
	case SegMetadata:
		result = ReadMetadata(pfile, ioffset + HDRSIZE);
		break;
	case SegAttach:
		result = ReadAttach(pfile);
		break;
	case SegAttDir:
		result = ReadAttDir(pfile);
		break;
	case SegDeleted:
		result = ReadDeleted(pfile);
		break;
	}

	ioffset += HDRSIZE + alloc_size;
	return result;
}

bool CZIReader::ReadFile(FILE* pfile)
{
	bool result = true;
	int ival;
	result &= fread(&ival, sizeof(int), 1, pfile) == 1;//major
	result &= fread(&ival, sizeof(int), 1, pfile) == 1;//minor
	result &= fread(&ival, sizeof(int), 1, pfile) == 1;//reserved
	result &= fread(&ival, sizeof(int), 1, pfile) == 1;//reserved
	unsigned int guid[4];
	result &= fread(guid, sizeof(unsigned int), 4, pfile) == 4;//master guid
	result &= fread(guid, sizeof(unsigned int), 4, pfile) == 4;//unique guid
	result &= fread(&m_file_part, sizeof(int), 1, pfile) == 1;//part number
	result &= fread(&m_dir_pos, sizeof(unsigned long long), 1, pfile) == 1;//directory position
	result &= fread(&m_meta_pos, sizeof(unsigned long long), 1, pfile) == 1;//metadata position
	result &= fread(&ival, sizeof(int), 1, pfile) == 1;//update pending
	result &= fread(&m_att_dir, sizeof(unsigned long long), 1, pfile) == 1;//attachment directory position
	if (m_file_part == 0)
	{
		m_multi_file = false;
	}
	else
	{
		m_multi_file = true;
	}
	m_header_read = result;
	return result;
}

unsigned int CZIReader::ReadDirectoryEntry(FILE* pfile)
{
	bool result = true;
	char schema_type[2];
	result &= fread(schema_type, sizeof(char), 2, pfile) == 2;//schema type
	unsigned int pixel_type;
	result &= fread(&pixel_type, sizeof(unsigned int), 1, pfile) == 1;//pixel type
	unsigned long long file_pos;
	result &= fread(&file_pos, sizeof(unsigned long long), 1, pfile) == 1;//file position
	unsigned int file_part;
	result &= fread(&file_part, sizeof(unsigned int), 1, pfile) == 1;//file part
	unsigned int compress;
	result &= fread(&compress, sizeof(unsigned int), 1, pfile) == 1;//compression
	char pyra_type;
	result &= fread(&pyra_type, 1, 1, pfile) == 1;//pyramid type
	char cval[5];
	result &= fread(cval, 1, 5, pfile) == 5;//reserved
	unsigned int dim_count;
	result &= fread(&dim_count, sizeof(unsigned int), 1, pfile) == 1;//dimension count
	for (int i = 0; i < dim_count; ++i)
	{
		char dim[4];
		result &= fread(dim, 1, 4, pfile) == 4;//dimension
		int start;
		result &= fread(&start, sizeof(int), 1, pfile) == 1;//start position
		unsigned int size;
		result &= fread(&size, sizeof(unsigned int), 1, pfile) == 1;//size in units of pixels
		float start_coord;
		result &= fread(&start_coord, sizeof(float), 1, pfile) == 1;//start coordinate
		unsigned int store_size;
		result &= fread(&store_size, sizeof(unsigned int), 1, pfile) == 1;//stored size
	}
	if (result)
		return 32 + dim_count * 20;
	else
		return 0;
}

bool CZIReader::ReadDirectory(FILE* pfile, unsigned long long ioffset)
{
	bool result = true;
	int entry_count;
	result &= fread(&entry_count, sizeof(int), 1, pfile) == 1;//number of entries
	ioffset += 128;
	if (FSEEK64(pfile, ioffset, SEEK_SET) != 0)
		return false;
	//read entries
	for (int i = 0; i < entry_count; ++i)
		result &= ReadDirectoryEntry(pfile) > 0;
	return result;
}

bool CZIReader::ReadSubBlock(FILE* pfile, unsigned long long ioffset)
{
	bool result = true;
	unsigned int meta_size;
	result &= fread(&meta_size, sizeof(unsigned int), 1, pfile) == 1;//metadata size
	unsigned int att_size;
	result &= fread(&att_size, sizeof(unsigned int), 1, pfile) == 1;//attachment size
	unsigned long long data_size;
	result &= fread(&data_size, sizeof(unsigned long long), 1, pfile) == 1;//data size

	//directory entry
	unsigned int dir_pos = ReadDirectoryEntry(pfile);
	dir_pos += 16;
	if (dir_pos > FIXSIZE)
		ioffset += dir_pos;
	else
		ioffset += FIXSIZE;
	if (FSEEK64(pfile, ioffset, SEEK_SET) != 0)
		return false;
	//read metadata
	std::string xmlstr(meta_size, 0);
	result &= fread(&xmlstr[0], 1, meta_size, pfile) == meta_size;//xml info
	//data
	//attachment

	return result;
}

bool CZIReader::ReadMetadata(FILE* pfile, unsigned long long ioffset)
{
	bool result = true;
	unsigned int xmlsize;
	result &= fread(&xmlsize, sizeof(unsigned int), 1, pfile) == 1;//xml data size
	ioffset += FIXSIZE;
	if (FSEEK64(pfile, ioffset, SEEK_SET) != 0)
		return false;
	std::string xmlstr(xmlsize, 0);
	result &= fread(&xmlstr[0], 1, xmlsize, pfile) == xmlsize;//xml info
	if (!result)
		return result;

	wxXmlDocument doc;
	wxStringInputStream wxss(xmlstr);
	result &= doc.Load(wxss);

	return result;
}

bool CZIReader::ReadAttach(FILE* pfile)
{
	return true;
}

bool CZIReader::ReadAttDir(FILE* pfile)
{
	return true;
}

bool CZIReader::ReadDeleted(FILE* pfile)
{
	return true;
}
