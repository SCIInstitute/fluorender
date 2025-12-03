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
#include <nd2_reader.h>
#include <Global.h>
#include <MainSettings.h>
#include <compatibility.h>
#include <Nd2ReadSdk.h>
#include <json.hpp>

ND2Reader::ND2Reader():
	BaseVolReader()
{
	m_time_num = 0;
	m_cur_time = -1;
	m_chan_num = 0;

	m_valid_spc = false;
	m_spacing = fluo::Vector(1.0);

	m_min_value = 0.0;
	m_max_value = 0.0;
	m_scalar_scale = 1.0;

	m_fp_convert = false;
	m_fp_min = 0;
	m_fp_max = 1;

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

//void ND2Reader::SetFile(const std::string &file)
//{
//	if (!file.empty())
//	{
//		if (!m_path_name.empty())
//			m_path_name.clear();
//		m_path_name.assign(file.length(), L' ');
//		copy(file.begin(), file.end(), m_path_name.begin());
//	}
//	m_id_string = m_path_name;
//}

void ND2Reader::SetFile(const std::wstring &file)
{
	m_path_name = file;
	m_id_string = m_path_name;
}

int ND2Reader::Preprocess()
{
	LIMCWSTR filename = m_path_name.c_str();
	LIMFILEHANDLE h = Lim_FileOpenForRead(filename);
	if (h == nullptr)
	{
		Lim_FileClose(h);
		return READER_OPEN_FAIL;
	}

	//get attributes
	ReadAttributes(h);
	//get wavelength info
	ReadTextInfo(h);
	//get xyz voxel sizes
	ReadMetadata(h);
	//read sequence info
	ReadSequences(h);

	Lim_FileClose(h);
	return READER_OK;
}

void ND2Reader::SetBatch(bool batch)
{
	if (batch)
	{
		//read the directory info
		FIND_FILES_BATCH(m_path_name, ESCAPE_REGEX(L".nd2"), m_batch_list, m_cur_batch);
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

	LIMCWSTR filename = m_path_name.c_str();
	LIMFILEHANDLE h = Lim_FileOpenForRead(filename);
	if (h == nullptr)
	{
		Lim_FileClose(h);
		return 0;
	}

	if (t >= 0 && t < m_time_num &&
		c >= 0 && c < m_chan_num &&
		!m_size.any_le_zero())
	{
		switch (m_bits)
		{
		case 8:
		{
			unsigned long long mem_size = m_size.get_size_xyz();
			unsigned char *val = new (std::nothrow) unsigned char[mem_size];
			ReadChannel(h, t, c, val);
			//create nrrd
			data = nrrdNew();
			nrrdWrap_va(data, val, nrrdTypeUChar, 3, (size_t)m_size.intx(), (size_t)m_size.inty(), (size_t)m_size.intz());
			nrrdAxisInfoSet_va(data, nrrdAxisInfoSpacing, m_spacing.x(), m_spacing.y(), m_spacing.z());
			auto max_size = m_spacing * m_size;
			nrrdAxisInfoSet_va(data, nrrdAxisInfoMax, max_size.x(), max_size.y(), max_size.z());
			nrrdAxisInfoSet_va(data, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
			nrrdAxisInfoSet_va(data, nrrdAxisInfoSize, (size_t)m_size.intx(), (size_t)m_size.inty(), (size_t)m_size.intz());
		}
		break;
		case 16:
		{
			unsigned long long mem_size = m_size.get_size_xyz();
			unsigned short *val = new (std::nothrow) unsigned short[mem_size];
			ReadChannel(h, t, c, val);
			//create nrrd
			data = nrrdNew();
			nrrdWrap_va(data, val, nrrdTypeUShort, 3, (size_t)m_size.intx(), (size_t)m_size.inty(), (size_t)m_size.intz());
			nrrdAxisInfoSet_va(data, nrrdAxisInfoSpacing, m_spacing.x(), m_spacing.y(), m_spacing.z());
			auto max_size = m_spacing * m_size;
			nrrdAxisInfoSet_va(data, nrrdAxisInfoMax, max_size.x(), max_size.y(), max_size.z());
			nrrdAxisInfoSet_va(data, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
			nrrdAxisInfoSet_va(data, nrrdAxisInfoSize, (size_t)m_size.intx(), (size_t)m_size.inty(), (size_t)m_size.intz());
		}
		break;
		}
	}

	Lim_FileClose(h);
	m_cur_time = t;

	return data;
}

std::wstring ND2Reader::GetCurDataName(int t, int c)
{
	return m_path_name;
}

std::wstring ND2Reader::GetCurMaskName(int t, int c)
{
	std::wostringstream woss;
	woss << m_path_name.substr(0, m_path_name.find_last_of(L'.'));
	if (m_time_num > 1) woss << L"_T" << t;
	if (m_chan_num > 1) woss << L"_C" << c;
	woss << L".msk";
	std::wstring mask_name = woss.str();
	return mask_name;
}

std::wstring ND2Reader::GetCurLabelName(int t, int c)
{
	std::wostringstream woss;
	woss << m_path_name.substr(0, m_path_name.find_last_of(L'.'));
	if (m_time_num > 1) woss << L"_T" << t;
	if (m_chan_num > 1) woss << L"_C" << c;
	woss << L".lbl";
	std::wstring label_name = woss.str();
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

	unsigned long long xysize = (unsigned long long)m_size.get_size_xy();
	unsigned long long pos;
	int bytes = m_bits / 8;
	unsigned char *dst, *src;
	size_t num = cinfo->chann.size();
	unsigned long long mem_size = m_size.get_size_xyz();
	bool show_progress = mem_size > glbin_settings.m_prg_size;

	for (size_t i = 0; i < num; ++i)
	{
		int seq = cinfo->chann[i].seq;
		int z = cinfo->chann[i].slice;
		int x = cinfo->chann[i].x;
		int y = cinfo->chann[i].y;
		LIMPICTURE pic = { 0 };
		Lim_FileGetImageData(h, seq, &pic);
		pos = xysize * z + m_size.intx() * y + x;//consider it a brick

		for (unsigned int j = 0; j < pic.uiHeight; ++j)
		{
			for (unsigned int k = 0; k < pic.uiWidth; ++k)
			{
				dst = (unsigned char*)val;
				dst += (pos + m_size.intx() * j + k) * bytes;
				src = (unsigned char*)(pic.pImageData);
				src += pic.uiWidthBytes * j + k * pic.uiComponents * bytes + c * bytes;
				memcpy(dst, src, bytes);
			}
		}

		Lim_DestroyPicture(&pic);

		if (show_progress && m_time_num == 1)
			SetProgress(static_cast<int>(std::round(100.0 * (i + 1) / num)), "NOT_SET");
	}

	return true;
}

void ND2Reader::ReadAttributes(LIMFILEHANDLE h)
{
	LIMSTR limstr = Lim_FileGetAttributes(h);
	nlohmann::json j = nlohmann::json::parse(limstr);
	Lim_FileFreeString(limstr);
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
			m_size.x(it.value());
		else if (str == "heightPx")
			m_size.y(it.value());
	}
	if (m_bits > 8)
	{
		m_max_value = std::pow(2.0, double(m_bits_used));
		m_scalar_scale = 65535.0 / m_max_value;
	}
}

void ND2Reader::ReadTextInfo(LIMFILEHANDLE h)
{
	LIMSTR limstr = Lim_FileGetTextinfo(h);
	nlohmann::json j = nlohmann::json::parse(limstr);
	Lim_FileFreeString(limstr);
	std::string str, plane, exw;
	for (nlohmann::json::iterator it = j.begin(); it != j.end(); ++it)
	{
		str = it.key();
		if (str == "description")
		{
			str = it.value();
			//search for wavelengths
			size_t pos = 0, pos2 = 0, pt = 0;
			pos = str.find("Plane #", pos);
			if (pos == std::string::npos)
				break;
			pt = str.find(":", pos);
			if (pt == std::string::npos)
				break;
			plane = str.substr(pos + 7, pt - pos - 7);//channel number
			pos2 = str.find("Plane #", pos + 5);
			if (pos2 == std::string::npos)
				pos2 = str.length() - 1;
			int c = 0;
			do
			{
				pos = str.find("{Laser Wavelength}: ", pos);
				if (pos != std::string::npos &&
					pos < pos2)
				{
					pt = str.find("\t", pos);
					exw = str.substr(pos + 20, pt - pos - 20);
					WavelengthInfo winfo;
					winfo.chan_num = c;
					winfo.wavelength = STOD(exw);
					m_excitation_wavelength_list.push_back(winfo);
					c++;
					pos = pt;
				}
				else
					break;
			} while (pos < pos2);
			break;
		}
	}
}

void ND2Reader::ReadMetadata(LIMFILEHANDLE h)
{
	LIMSTR limstr = Lim_FileGetMetadata(h);
	nlohmann::json j = nlohmann::json::parse(limstr);
	Lim_FileFreeString(limstr);
	auto it = j.find("channels");
	std::string str;
	if (it != j.end())
	{
		str = it->dump();
		size_t pos = str.find("axesCalibration");
		if (pos == std::string::npos)
			return;
		pos = str.find("[", pos);
		if (pos == std::string::npos)
			return;
		size_t pos2 = str.find("]", pos);
		if (pos2 == std::string::npos)
			return;
		std::string x, y, z;
		int count = 0;
		bool flag = false;
		for (size_t i = pos; i < pos2; ++i)
		{
			if (isdigit(str[i]) || str[i] == '.')
			{
				flag = true;
				if (!count)
					count++;
				switch (count)
				{
				case 1:
					x += str[i];
					break;
				case 2:
					y += str[i];
					break;
				case 3:
					z += str[i];
					break;
				}
			}
			else
			{
				if (flag)
					count++;
				flag = false;
			}
		}
		if (count >= 3)
		{
			m_spacing = fluo::Vector(
				stod(x),
				stod(y),
				stod(z));
			m_valid_spc = true;
		}
	}
}

void ND2Reader::ReadSequences(LIMFILEHANDLE h)
{
	m_nd2_info.init();

	LIMCHAR buffer[ND2_STR_SIZE];
	LIMSIZE coordsize = Lim_FileGetCoordSize(h);
	LIMUINT fnum = Lim_FileGetSeqCount(h);
	int ti = -1, zi = -1, xyi = -1;//indecis in coords
	int maxz = 0;
	std::vector<LIMUINT> vec(coordsize);
	std::string str;
	if (coordsize > 0)
	{
		for (size_t i = 0; i < coordsize; ++i)
		{
			Lim_FileGetCoordInfo(h, (LIMUINT)i, buffer, ND2_STR_SIZE);
			str = buffer;
			if (str == "TimeLoop" ||
				str == "NETimeLoop")
				ti = static_cast<int>(i);
			else if (str == "XYPosLoop")
				xyi = static_cast<int>(i);
			else if (str == "ZStackLoop")
				zi = static_cast<int>(i);
		}
		for (unsigned int i = 0; i < fnum; ++i)
		{
			FrameInfo frame;
			Lim_FileGetCoordsFromSeqIndex(h, i, vec.data(), vec.size());
			LIMSTR fmd = Lim_FileGetFrameMetadata(h, i);
			GetFramePos(fmd, frame);
			Lim_FileFreeString(fmd);
			frame.chan = 0;
			frame.time = ti >= 0 ? vec[ti] : 0;
			frame.slice = zi >= 0 ? vec[zi] : 0;
			frame.seq = i;
			frame.x = 0;
			frame.y = 0;
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
			frame.x = 0;
			frame.y = 0;
			frame.xsize = m_size.intx();
			frame.ysize = m_size.inty();
			frame.posx = 0.0;
			frame.posy = 0.0;
			frame.posz = 0.0;
			AddFrameInfo(frame);
		}
	}

	m_time_num = static_cast<int>(m_nd2_info.times.size());
	m_size.z(maxz + 1);
	m_cur_time = 0;
	m_data_name = GET_STEM(m_path_name);

	//get tiles
	ChannelInfo* cinfo = GetChaninfo(0, 0);
	if (cinfo && cinfo->chann.size() > m_size.intz() &&
		m_valid_spc)
	{
		for (int t = 0; t < m_time_num; ++t)
		{
			cinfo = GetChaninfo(t, 0);
			if (!cinfo)
				continue;
			for (size_t i = 0; i < cinfo->chann.size(); ++i)
			{
				cinfo->chann[i].x = static_cast<int>((cinfo->chann[i].posx - m_nd2_info.xmin) / m_spacing.x());
				cinfo->chann[i].y = static_cast<int>((cinfo->chann[i].posy - m_nd2_info.ymin) / m_spacing.y());
				m_size.x(std::max(m_size.intx(), cinfo->chann[i].x + cinfo->chann[i].xsize));
				m_size.y(std::max(m_size.inty(), cinfo->chann[i].y + cinfo->chann[i].ysize));
			}
		}
	}
}

void ND2Reader::GetFramePos(LIMSTR fmd, FrameInfo& frame)
{
	std::string str(fmd);
	size_t pos = str.find("stagePositionUm");
	if (pos == std::string::npos)
		return;
	pos = str.find("[", pos);
	if (pos == std::string::npos)
		return;
	size_t pos2 = str.find("]", pos);
	if (pos2 == std::string::npos)
		return;
	std::string x, y, z;
	int count = 0;
	bool flag = false;
	for (size_t i = pos; i < pos2; ++i)
	{
		if (isdigit(str[i]) || str[i] == '.')
		{
			flag = true;
			if (!count)
				count++;
			switch (count)
			{
			case 1:
				x += str[i];
				break;
			case 2:
				y += str[i];
				break;
			case 3:
				z += str[i];
				break;
			}
		}
		else
		{
			if (flag)
				count++;
			flag = false;
		}
	}
	if (count >= 3)
	{
		frame.posx = stod(x);
		frame.posy = stod(y);
		frame.posz = stod(z);
		m_nd2_info.update(frame.posx, frame.posy, frame.posz);
	}
	//get size
	x = y = z = "";
	pos = str.find("voxelCount");
	if (pos == std::string::npos)
		return;
	pos = str.find("[", pos);
	if (pos == std::string::npos)
		return;
	pos2 = str.find("]", pos);
	if (pos2 == std::string::npos)
		return;
	count = 0;
	flag = false;
	for (size_t i = pos; i < pos2; ++i)
	{
		if (isdigit(str[i]))
		{
			flag = true;
			if (!count)
				count++;
			switch (count)
			{
			case 1:
				x += str[i];
				break;
			case 2:
				y += str[i];
				break;
			case 3:
				z += str[i];
				break;
			}
		}
		else
		{
			if (flag)
				count++;
			flag = false;
		}
	}
	if (count >= 3)
	{
		frame.xsize = stoi(x);
		frame.ysize = stoi(y);
	}
}
