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
#include <json.hpp>
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

	//get attributes
	LIMSTR attstr = Lim_FileGetAttributes(h);
	nlohmann::json j = nlohmann::json::parse(attstr);
	Lim_FileFreeString(attstr);
	std::string str;
	for (nlohmann::json::iterator it = j.begin(); it != j.end(); ++it)
	{
		str = it.key();
		if (str == "bitsPerComponentInMemory")
			m_bits = it.value();
		else if (str == "bitsPerComponentSignificant")
			m_bits_used = it.value();
		else if (str == "componentCount")
			m_chan_num = it.value();
		else if (str == "widthPx")
			m_x_size = it.value();
		else if (str == "heightPx")
			m_y_size = it.value();
	}

	if (m_bits > 8)
	{
		m_max_value = std::pow(2.0, double(m_bits_used));
		m_scalar_scale = 65535.0 / m_max_value;
	}

	LIMCHAR buffer[ND2_STR_SIZE];
	LIMSIZE coordsize = Lim_FileGetCoordSize(h);
	LIMUINT fnum = Lim_FileGetSeqCount(h);
	int ti = -1, zi = -1, xyi = -1;//indecis in coords
	int maxz = 0;
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
			maxz = frame.slice > maxz ? frame.slice : maxz;
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

	m_time_num = m_nd2_info.times.size();
	m_slice_num = maxz + 1;
	m_cur_time = 0;
	m_data_name = GET_NAME(m_path_name);
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
#ifndef _DEBUG
	LIMCWSTR filename = m_path_name.c_str();
	LIMFILEHANDLE h = Lim_FileOpenForRead(filename);
	if (h == nullptr)
	{
		Lim_FileClose(h);
		return 0;
	}

	if (t >= 0 && t < m_time_num &&
		c >= 0 && c < m_chan_num &&
		m_slice_num > 0 &&
		m_x_size > 0 &&
		m_y_size > 0)
	{
		switch (m_bits)
		{
		case 8:
		{
			unsigned long long mem_size = (unsigned long long)m_x_size*
				(unsigned long long)m_y_size*(unsigned long long)m_slice_num;
			unsigned char *val = new (std::nothrow) unsigned char[mem_size];
			ReadChannel(h, t, c, val);
			//create nrrd
			data = nrrdNew();
			nrrdWrap(data, val, nrrdTypeUChar, 3, (size_t)m_x_size, (size_t)m_y_size, (size_t)m_slice_num);
			nrrdAxisInfoSet(data, nrrdAxisInfoSpacing, m_xspc, m_yspc, m_zspc);
			nrrdAxisInfoSet(data, nrrdAxisInfoMax, m_xspc*m_x_size, m_yspc*m_y_size, m_zspc*m_slice_num);
			nrrdAxisInfoSet(data, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
			nrrdAxisInfoSet(data, nrrdAxisInfoSize, (size_t)m_x_size, (size_t)m_y_size, (size_t)m_slice_num);
		}
		break;
		case 16:
		{
			unsigned long long mem_size = (unsigned long long)m_x_size*
				(unsigned long long)m_y_size*(unsigned long long)m_slice_num;
			unsigned short *val = new (std::nothrow) unsigned short[mem_size];
			ReadChannel(h, t, c, val);
			//create nrrd
			data = nrrdNew();
			nrrdWrap(data, val, nrrdTypeUShort, 3, (size_t)m_x_size, (size_t)m_y_size, (size_t)m_slice_num);
			nrrdAxisInfoSet(data, nrrdAxisInfoSpacing, m_xspc, m_yspc, m_zspc);
			nrrdAxisInfoSet(data, nrrdAxisInfoMax, m_xspc*m_x_size, m_yspc*m_y_size, m_zspc*m_slice_num);
			nrrdAxisInfoSet(data, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
			nrrdAxisInfoSet(data, nrrdAxisInfoSize, (size_t)m_x_size, (size_t)m_y_size, (size_t)m_slice_num);
		}
		break;
		}
	}

	Lim_FileClose(h);
	m_cur_time = t;
#endif
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

bool ND2Reader::ReadChannel(LIMFILEHANDLE h, int t, int c, void* val)
{
	ChannelInfo* cinfo = GetChaninfo(t, 0);
	if (!cinfo)
		return false;

	unsigned long long xysize = (unsigned long long)m_x_size * m_y_size;
	unsigned long long pos;
	int bytes = m_bits / 8;
	unsigned char *dst, *src;
	for (size_t i = 0; i < cinfo->chann.size(); ++i)
	{
		int seq = cinfo->chann[i].seq;
		int z = cinfo->chann[i].slice;
		LIMPICTURE pic = { 0 };
		Lim_FileGetImageData(h, seq, &pic);
		pos = xysize * z;//consider it a brick
		for (unsigned int j = 0; j < pic.uiHeight; ++j)
		for (unsigned int k = 0; k < pic.uiWidth; ++k)
		{
			dst = (unsigned char*)val;
			dst += (pos + m_x_size * j + k) * bytes;
			src = (unsigned char*)(pic.pImageData);
			src += pic.uiWidthBytes * j + k * pic.uiComponents * bytes + c * bytes;
			memcpy(dst, src, bytes);
		}

		Lim_DestroyPicture(&pic);
	}

	return true;
}