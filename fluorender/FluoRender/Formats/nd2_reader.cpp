/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2020 Scientific Computing and Imaging Institute,
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
#include "nd2_reader.h"
#include "../compatibility.h"
#include <stdio.h>

ND2Reader::ND2Reader()
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

/*	m_compression = 0;
	m_predictor = 0;
	m_version = 0;
	m_datatype = 0;
	m_l4gb = false;*/
}

ND2Reader::~ND2Reader()
{
}

void ND2Reader::SetFile(string &file)
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

void ND2Reader::SetFile(wstring &file)
{
	m_path_name = file;
	m_id_string = m_path_name;
}

int ND2Reader::Preprocess()
{
#ifndef _DEBUG
	LIMCWSTR filename = m_path_name.c_str();
	LIMFILEHANDLE h = Lim_FileOpenForRead(filename);
	if (h == nullptr)
	{
		Lim_FileClose(h);
		return READER_OPEN_FAIL;
	}

	LIMCHAR buffer[ND2_STR_SIZE];
	std::string str;
	m_nd2_info.init();
	LIMSIZE coordsize = Lim_FileGetCoordSize(h);
	LIMUINT fnum = Lim_FileGetSeqCount(h);
	int ti = -1, zi = -1, xyi = -1;//indecis in coords
	std::vector<LIMUINT> vec(coordsize);
	if (coordsize > 0)
	{
		for (size_t i = 0; i < coordsize; ++i)
		{
			Lim_FileGetCoordInfo(h, (LIMUINT)i, buffer, ND2_STR_SIZE);
			str = buffer;
			if (str == "TimeLoop")
				ti = i;
			else if (str == "XYPosLoop")
				xyi = i;
			else if (str == "ZStackLoop")
				zi = i;
		}
		for (unsigned int i = 0; i < fnum; ++i)
		{
			Lim_FileGetCoordsFromSeqIndex(h, i, vec.data(), vec.size());
			FrameInfo frame;
			frame.chan = 0;
			frame.time = ti >= 0 ? vec[ti] : 0;
			frame.slice = zi >= 0 ? vec[zi] : 0;
			frame.seq = i;
			AddFrameInfo(frame);
		}
	}
	else
	{
		if (fnum)
		{
			//single frame
			FrameInfo frame;
			frame.chan = 0;
			frame.time = 0;
			frame.slice = 0;
			frame.seq = 0;
			AddFrameInfo(frame);
		}
	}
	Lim_FileClose(h);
#endif
	return READER_OK;
}

void ND2Reader::SetSliceSeq(bool ss)
{
	//do nothing
}

bool ND2Reader::GetSliceSeq()
{
	return false;
}

void ND2Reader::SetChannSeq(bool cs)
{
	//do nothing
}

bool ND2Reader::GetChannSeq()
{
	return false;
}

void ND2Reader::SetDigitOrder(int order)
{
	//do nothing
}

int ND2Reader::GetDigitOrder()
{
	return 0;
}

void ND2Reader::SetTimeId(wstring &id)
{
	//do nothing
}

wstring ND2Reader::GetTimeId()
{
	return wstring(L"");
}

void ND2Reader::SetBatch(bool batch)
{
	if (batch)
	{
		//read the directory info
		FIND_FILES(m_path_name, L"*.nd2", m_batch_list, m_cur_batch);
		m_batch = true;
	}
	else
		m_batch = false;
}

int ND2Reader::LoadBatch(int index)
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

double ND2Reader::GetExcitationWavelength(int chan)
{
	for (int i = 0; i < (int)m_excitation_wavelength_list.size(); i++)
	{
		if (m_excitation_wavelength_list[i].chan_num == chan)
			return m_excitation_wavelength_list[i].wavelength;
	}
	return 0.0;
}

Nrrd* ND2Reader::Convert(int t, int c, bool get_max)
{
	Nrrd *data = 0;
	return data;
}

wstring ND2Reader::GetCurDataName(int t, int c)
{
	return m_path_name;
}

wstring ND2Reader::GetCurMaskName(int t, int c)
{
	wostringstream woss;
	woss << m_path_name.substr(0, m_path_name.find_last_of('.'));
	if (m_time_num > 1) woss << "_T" << t;
	if (m_chan_num > 1) woss << "_C" << c;
	woss << ".msk";
	wstring mask_name = woss.str();
	return mask_name;
}

wstring ND2Reader::GetCurLabelName(int t, int c)
{
	wostringstream woss;
	woss << m_path_name.substr(0, m_path_name.find_last_of('.'));
	if (m_time_num > 1) woss << "_T" << t;
	if (m_chan_num > 1) woss << "_C" << c;
	woss << ".lbl";
	wstring label_name = woss.str();
	return label_name;
}

void ND2Reader::AddFrameInfo(FrameInfo &frame)
{
	int chan = frame.chan;
	int time = frame.time;
	int slice = frame.slice;

	TimeInfo *timeinfo;
	if (m_nd2_info.times.size() <= time)
	{
		m_nd2_info.times.resize(time + 1);
		timeinfo = &(m_nd2_info.times.back());
		timeinfo->time = time;
	}
	else
		timeinfo = &(m_nd2_info.times[time]);

	ChannelInfo *chaninfo;
	if (timeinfo->channels.size() <= chan)
	{
		timeinfo->channels.resize(chan + 1);
		chaninfo = &(timeinfo->channels.back());
		chaninfo->chan = chan;
	}
	else
		chaninfo = &(timeinfo->channels[chan]);

	chaninfo->chann.push_back(frame);
}