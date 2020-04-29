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
#include "lif_reader.h"
#include "../compatibility.h"
#include <wx/sstream.h>
#include <stdio.h>

LIFReader::LIFReader()
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
}

LIFReader::~LIFReader()
{
}

void LIFReader::SetFile(string &file)
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

void LIFReader::SetFile(wstring &file)
{
	m_path_name = file;
	m_id_string = m_path_name;
}

int LIFReader::Preprocess()
{
	return READER_OK;
}

void LIFReader::SetSliceSeq(bool ss)
{
	//do nothing
}

bool LIFReader::GetSliceSeq()
{
	return false;
}

void LIFReader::SetChannSeq(bool cs)
{
	//do nothing
}

bool LIFReader::GetChannSeq()
{
	return false;
}

void LIFReader::SetDigitOrder(int order)
{
	//do nothing
}

int LIFReader::GetDigitOrder()
{
	return 0;
}

void LIFReader::SetTimeId(wstring &id)
{
	//do nothing
}

wstring LIFReader::GetTimeId()
{
	return wstring(L"");
}

void LIFReader::SetBatch(bool batch)
{
	if (batch)
	{
		//read the directory info
		FIND_FILES(m_path_name, L"*.lif", m_batch_list, m_cur_batch);
		m_batch = true;
	}
	else
		m_batch = false;
}

int LIFReader::LoadBatch(int index)
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

double LIFReader::GetExcitationWavelength(int chan)
{
	for (int i = 0; i < (int)m_excitation_wavelength_list.size(); i++)
	{
		if (m_excitation_wavelength_list[i].chan_num == chan)
			return m_excitation_wavelength_list[i].wavelength;
	}
	return 0.0;
}

Nrrd* LIFReader::Convert(int t, int c, bool get_max)
{
	Nrrd *data = 0;
	return data;
}

wstring LIFReader::GetCurDataName(int t, int c)
{
	return m_path_name;
}

wstring LIFReader::GetCurMaskName(int t, int c)
{
	wostringstream woss;
	woss << m_path_name.substr(0, m_path_name.find_last_of('.'));
	if (m_time_num > 1) woss << "_T" << t;
	if (m_chan_num > 1) woss << "_C" << c;
	woss << ".msk";
	wstring mask_name = woss.str();
	return mask_name;
}

wstring LIFReader::GetCurLabelName(int t, int c)
{
	wostringstream woss;
	woss << m_path_name.substr(0, m_path_name.find_last_of('.'));
	if (m_time_num > 1) woss << "_T" << t;
	if (m_chan_num > 1) woss << "_C" << c;
	woss << ".lbl";
	wstring label_name = woss.str();
	return label_name;
}

