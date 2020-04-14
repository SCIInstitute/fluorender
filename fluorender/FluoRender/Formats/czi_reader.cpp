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
#include <stdio.h>

CZIReader::CZIReader()
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

	unsigned int header_size = 32;
	unsigned char id[16];
	unsigned long long alloc_size;
	unsigned long long used_size;
	unsigned long long ioffset = 0;

	while (!feof(pfile))
	{
		if (FSEEK64(pfile, ioffset, SEEK_SET) != 0)
			break;
		//read segment
		fread(id, 1, 16, pfile);
		fread(&alloc_size, sizeof(unsigned long long), 1, pfile);
		fread(&used_size, sizeof(unsigned long long), 1, pfile);

		//next segment
		ioffset += 32 + alloc_size;
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
