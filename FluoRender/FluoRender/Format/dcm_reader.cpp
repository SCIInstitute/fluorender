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
#include <dcm_reader.h>
#include <Global.h>
#include <MainSettings.h>
#include <compatibility.h>
#include <fstream>
#include <map>

DCMReader::DCMReader() :
	BaseReader()
{
	m_time_num = 0;
	m_cur_time = -1;
	m_chan_num = 0;
	m_slice_num = 0;
	m_x_size = 0;
	m_y_size = 0;

	m_valid_spc = false;
	m_xspc = 1.0;
	m_yspc = 1.0;
	m_zspc = 1.0;

	m_min_value = 0.0;
	m_max_value = 0.0;
	m_scalar_scale = 1.0;

	m_fp_convert = false;
	m_fp_min = 0;
	m_fp_max = 1;

	m_batch = false;
	m_cur_batch = -1;
	m_cur_time = -1;

	m_time_id = L"_T";

	m_bits = 8; //default bits per channel
	m_big_endian = false;
}

DCMReader::~DCMReader()
{
}

void DCMReader::SetFile(const std::wstring& file)
{
	m_path_name = file;
	m_id_string = m_path_name;
}

int DCMReader::Preprocess()
{
	m_4d_seq.clear();

	std::wstring path, name;
	if (!SEP_PATH_NAME(m_path_name, path, name))
		return READER_OPEN_FAIL;

	AnalyzeNamePattern(m_path_name);
	std::vector<std::wstring> list;
	if (!FIND_FILES_4D(m_path_name, m_time_id, list, m_cur_time))
	{
		TimeDataInfo info;
		SliceInfo sliceinfo;
		sliceinfo.slice = path + name;  //temporary name
		sliceinfo.slicenumber = 0;
		info.slices.push_back(sliceinfo);
		info.filenumber = 0;
		m_4d_seq.push_back(info);
		m_cur_time = 0;
	}
	else
	{
		int64_t begin = m_path_name.find(m_time_id);
		size_t id_len = m_time_id.length();
		for (size_t i = 0; i < list.size(); i++) {
			TimeDataInfo info;
			std::wstring str = list.at(i);
			std::wstring t_num;
			for (size_t j = begin + id_len; j < str.size(); j++)
			{
				wchar_t c = str[j];
				if (iswdigit(c))
					t_num.push_back(c);
				else break;
			}
			if (t_num.size() > 0)
				info.filenumber = WSTOI(t_num);
			else
				info.filenumber = 0;
			SliceInfo sliceinfo;
			sliceinfo.slice = str;
			sliceinfo.slicenumber = 0;
			info.slices.push_back(sliceinfo);
			m_4d_seq.push_back(info);
		}
	}

	m_slice_count.clear();
	m_chann_count.clear();
	//build 3d slice sequence
	for (int t = 0; t < (int)m_4d_seq.size(); t++)
	{
		std::wstring slice_str = m_4d_seq[t].slices[0].slice;
		int tv = GetPatternNumber(slice_str, 2);

		if (m_slice_seq || m_chann_seq)
		{
			//search slice sequence
			std::vector<std::wstring> list;
			std::wstring search_mask;
			if (m_slice_seq && !m_chann_seq)
				search_mask = GetSearchString(0, tv);
			else if (m_chann_seq && !m_slice_seq)
				search_mask = GetSearchString(1, tv);
			else if (m_chann_seq && m_slice_seq)
				search_mask = GetSearchString(-1, tv);
			FIND_FILES(path, search_mask, list, m_cur_time);
			m_4d_seq[t].type = 1;
			m_4d_seq[t].slices.clear();
			for (size_t f = 0; f < list.size(); f++)
			{
				SliceInfo slice;
				slice.slice = list.at(f);
				if (m_slice_seq && !m_chann_seq)
					slice.slicenumber = GetPatternNumber(list.at(f), 0, true);
				else if (m_chann_seq && !m_slice_seq)
					slice.slicenumber = GetPatternNumber(list.at(f), 1, true);
				else if (m_chann_seq && m_slice_seq)
				{
					short sn = short(GetPatternNumber(list.at(f), 0, true));//slice number
					short cn = short(GetPatternNumber(list.at(f), 1, true));//channel number
					slice.slicenumber = (int(cn) << 16) | int(sn);
				}
				m_4d_seq[t].slices.push_back(slice);
			}
			if (m_4d_seq[t].slices.size() > 0)
				std::sort(m_4d_seq[t].slices.begin(),
					m_4d_seq[t].slices.end(),
					[](const SliceInfo& a, const SliceInfo& b)
					{ return a.slicenumber < b.slicenumber; });
		}
		else
		{
			m_4d_seq[t].type = 0;
			m_4d_seq[t].slices[0].slice = slice_str;
			if (m_4d_seq[t].slices[0].slice == m_path_name)
				m_cur_time = t;
		}
	}

	//get time number and channel number
	m_time_num = (int)m_4d_seq.size();
	if (m_4d_seq.size() > 0 &&
		m_cur_time >= 0 &&
		m_cur_time < (int)m_4d_seq.size() &&
		m_4d_seq[m_cur_time].slices.size()>0)
	{
		std::wstring file_name = m_4d_seq[m_cur_time].slices[0].slice;
		if (file_name.size() > 0)
		{
			GetFileInfo(file_name);
			if (m_chan_num <= 0)
			{
				m_chan_num = 1;
			}
			//channels could be stored in slices
			if (m_chann_seq)
			{
				m_chan_num *= static_cast<int>(m_chann_count.size());
			}
		}
		else m_chan_num = 0;

		m_slice_num = static_cast<int>(m_4d_seq[m_cur_time].slices.size());
	}
	else m_chan_num = 0;

	return READER_OK;
}

void DCMReader::GetFileInfo(const std::wstring& filename)
{
	std::ifstream file(filename, std::ios::binary);
	if (!file.is_open())
		return;

	file.seekg(132, std::ios::beg); // Skip preamble + "DICM"

	uint16_t group, element;
	char vr[3] = { 0 };
	uint32_t length;
	std::map<uint32_t, std::vector<char>> tag_map;

	auto ReadU16 = [&](std::ifstream& f) -> uint16_t {
		uint16_t val;
		f.read(reinterpret_cast<char*>(&val), 2);
		if (m_big_endian)
			val = (val >> 8) | (val << 8);
		return val;
		};

	auto ReadU32 = [&](std::ifstream& f) -> uint32_t {
		uint32_t val;
		f.read(reinterpret_cast<char*>(&val), 4);
		if (m_big_endian)
			val = ((val >> 24) & 0x000000FF) |
			((val >> 8) & 0x0000FF00) |
			((val << 8) & 0x00FF0000) |
			((val << 24) & 0xFF000000);
		return val;
		};

	while (file.read(reinterpret_cast<char*>(&group), 2) &&
		file.read(reinterpret_cast<char*>(&element), 2))
	{
		uint32_t tag = (group << 16) | element;

		if (!file.read(vr, 2)) break;

		bool long_length = (vr[0] == 'O' && vr[1] == 'B') || (vr[0] == 'O' && vr[1] == 'W') ||
			(vr[0] == 'U' && vr[1] == 'N') || (vr[0] == 'S' && vr[1] == 'Q');

		if (long_length)
			file.ignore(2); // Reserved bytes

		uint32_t length = long_length ? ReadU32(file) : ReadU16(file);

		if (length > 100000000) break; // Sanity check

		std::vector<char> value(length);
		if (!file.read(value.data(), length)) break;

		tag_map[tag] = std::move(value);

		// Detect Transfer Syntax UID
		if (tag == 0x00020010) {
			std::string tsuid(value.data(), value.size());
			if (tsuid.find("1.2.840.10008.1.2.2") != std::string::npos)
				m_big_endian = true;
		}

		if (tag == 0x7FE00010) break; // Pixel Data
	}

	// Safe metadata extraction
	auto get_u16 = [&](uint32_t tag) -> uint16_t {
		auto it = tag_map.find(tag);
		if (it == tag_map.end() || it->second.size() < 2) return 0;
		uint16_t val = *reinterpret_cast<uint16_t*>(it->second.data());
		if (m_big_endian)
			val = (val >> 8) | (val << 8);
		return val;
		};

	m_y_size = get_u16(0x00280010); // Rows
	m_x_size = get_u16(0x00280011); // Columns
	m_bits = get_u16(0x00280100); // Bits Allocated
	m_chan_num = get_u16(0x00280002); // Samples per Pixel

	// Optional: pixel spacing
	auto it_spc = tag_map.find(0x00280030);
	if (it_spc != tag_map.end() && !it_spc->second.empty()) {
		std::string spacing_str(it_spc->second.data(), it_spc->second.size());
		if (sscanf(spacing_str.c_str(), "%lf\\%lf", &m_xspc, &m_yspc) == 2) {
			m_zspc = std::max(m_xspc, m_yspc);
			m_valid_spc = true;
		}
	}
}

void DCMReader::SetBatch(bool batch)
{
	if (batch)
	{
		//read the directory info
		std::wstring suffix = GET_SUFFIX(m_path_name);
		FIND_FILES_BATCH(m_path_name, ESCAPE_REGEX(suffix), m_batch_list, m_cur_batch);
		m_batch = true;
	}
	else
		m_batch = false;
}

int DCMReader::LoadBatch(int index)
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

Nrrd* DCMReader::Convert(int t, int c, bool get_max)
{
	if (t < 0 || c < 0)
	{
		t = 0;
		c = 0;
	}
	else if (t >= m_time_num || c >= m_chan_num)
	{
		if (m_time_num > 0)
			t = m_time_num - 1;
		else
			return 0;
		if (m_chan_num > 0)
			c = m_chan_num - 1;
		else
			return 0;
	}

	Nrrd* data = nullptr;
	TimeDataInfo chan_info = m_4d_seq[t];
	m_data_name = GET_STEM(chan_info.slices[0].slice);
	data = ReadDcm(chan_info.slices, c, get_max);
	m_cur_time = t;
	return data;
}

Nrrd* DCMReader::ReadDcm(const std::vector<SliceInfo>& filelist, int c, bool get_max)
{
	if (filelist.empty())
		return nullptr;

	Nrrd *nrrdout = nrrdNew();
	bool eight_bit = m_bits == 8;

	unsigned long long total_size = (unsigned long long)m_x_size*
		(unsigned long long)m_y_size*(unsigned long long)m_slice_num;
	void* val = eight_bit ? (void*)(new unsigned char[total_size]) :
		(void*)(new unsigned short[total_size]);
	if (!val)
		return nullptr;

	bool show_progress = total_size > glbin_settings.m_prg_size;

	uint8_t* val_ptr = static_cast<uint8_t*>(val);
	size_t for_size = filelist.size();
	for (size_t i = 0; i < for_size; i++)
	{
		const SliceInfo& slice_info = filelist[i];
		if (slice_info.slice.empty())
			continue;
		if (!ReadSingleDcm(static_cast<void*>(val_ptr), slice_info.slice, c))
		{
			delete[] static_cast<unsigned char*>(val);
			return nullptr;
		}
		if (show_progress)
			SetProgress(static_cast<int>(std::round(100.0 * (i + 1) / for_size)), "NOT_SET");
		val_ptr += m_x_size * m_y_size * (m_bits / 8);
	}

	//write to nrrd
	if (eight_bit)
		nrrdWrap_va(nrrdout, (uint8_t*)val, nrrdTypeUChar,
			3, (size_t)m_x_size, (size_t)m_y_size, (size_t)m_slice_num);
	else
		nrrdWrap_va(nrrdout, (uint16_t*)val, nrrdTypeUShort,
			3, (size_t)m_x_size, (size_t)m_y_size, (size_t)m_slice_num);
	nrrdAxisInfoSet_va(nrrdout, nrrdAxisInfoSpacing, m_xspc, m_yspc, m_zspc);
	nrrdAxisInfoSet_va(nrrdout, nrrdAxisInfoMax, m_xspc*m_x_size,
		m_yspc*m_y_size, m_zspc*m_slice_num);
	nrrdAxisInfoSet_va(nrrdout, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
	nrrdAxisInfoSet_va(nrrdout, nrrdAxisInfoSize, (size_t)m_x_size,
		(size_t)m_y_size, (size_t)m_slice_num);

	if (!eight_bit)
	{
		if (get_max)
		{
			double value;
			unsigned long long totali = (unsigned long long)m_slice_num*
				(unsigned long long)m_x_size*(unsigned long long)m_y_size;
			for (unsigned long long i = 0; i < totali; ++i)
			{
				value = ((unsigned short*)nrrdout->data)[i];
				m_min_value = m_min_value == 0.0 ? value : (value < m_min_value ? value : m_min_value);
				m_max_value = value > m_max_value ? value : m_max_value;
			}
		}
		if (m_max_value > 0.0) m_scalar_scale = 65535.0 / m_max_value;
		else m_scalar_scale = 1.0;
	}
	else m_max_value = 255.0;

	return nrrdout;
}

bool DCMReader::ReadSingleDcm(void* val, const std::wstring& filename, int c)
{
	if (c < 0 || c >= m_chan_num)
		return false; // Invalid channel index

	std::ifstream file(filename, std::ios::binary);
	if (!file.is_open())
		return false;

	file.seekg(132, std::ios::beg); // Skip DICOM preamble

	uint16_t group, element;
	char vr[3] = { 0 };
	uint32_t length;

	while (file.read(reinterpret_cast<char*>(&group), 2) &&
		file.read(reinterpret_cast<char*>(&element), 2))
	{
		file.read(vr, 2);

		bool long_length = (vr[0] == 'O' && vr[1] == 'B') ||
			(vr[0] == 'O' && vr[1] == 'W') ||
			(vr[0] == 'S' && vr[1] == 'Q') ||
			(vr[0] == 'U' && vr[1] == 'N');

		if (long_length)
			file.ignore(2); // Reserved bytes

		if (long_length)
			file.read(reinterpret_cast<char*>(&length), 4);
		else {
			uint16_t short_len;
			file.read(reinterpret_cast<char*>(&short_len), 2);
			length = short_len;
		}

		if ((group << 16 | element) == 0x7FE00010) {
			std::vector<char> pixel_data(length);
			file.read(pixel_data.data(), length);

			size_t slice_size = m_y_size * m_x_size;
			size_t total_size = slice_size * m_chan_num;

			if (m_bits == 8)
			{
				const uint8_t* full_data = reinterpret_cast<const uint8_t*>(pixel_data.data());
				uint8_t* channel_data = static_cast<uint8_t*>(val);
				//uint8_t* channel_data = new (std::nothrow) uint8_t[slice_size];
				//if (!channel_data) return false;

				for (size_t i = 0; i < slice_size; ++i)
					channel_data[i] = full_data[i * m_chan_num + c];
			}
			else if (m_bits == 16)
			{
				const uint16_t* full_data = reinterpret_cast<const uint16_t*>(pixel_data.data());
				uint16_t* channel_data = static_cast<uint16_t*>(val);
				//uint16_t* channel_data = new (std::nothrow) uint16_t[slice_size];
				//if (!channel_data) return false;

				for (size_t i = 0; i < slice_size; ++i)
				{
					uint16_t raw = full_data[i * m_chan_num + c];
					if (m_big_endian)
						raw = (raw >> 8) | (raw << 8); // Byte swap
					channel_data[i] = raw;
				}
			}
			else {
				// Unsupported bit depth
				return false;
			}

			break;
		}
		else {
			file.seekg(length, std::ios::cur); // Skip non-pixel data
		}
	}

	return val != nullptr;
}

std::wstring DCMReader::GetCurDataName(int t, int c)
{
	if (t >= 0 && t < (int)m_4d_seq.size() &&
		!m_4d_seq[t].slices.empty())
		return m_4d_seq[t].slices[0].slice;
	return m_path_name;
}

std::wstring DCMReader::GetCurMaskName(int t, int c)
{
	std::wstring mask_name;
	if (t >= 0 && t < (int)m_4d_seq.size() &&
		!m_4d_seq[t].slices.empty())
	{
		std::wstring data_name = m_4d_seq[t].slices[0].slice;
		mask_name = data_name.substr(0, data_name.find_last_of('.')) + L".msk";
		return mask_name;
	}
	mask_name = m_path_name.substr(0, m_path_name.find_last_of(L'.'));
	if (m_time_num > 1) mask_name += L"_T" + std::to_wstring(t);
	if (m_chan_num > 1) mask_name += L"_C" + std::to_wstring(c);
	mask_name += L".msk";
	return mask_name;
}

std::wstring DCMReader::GetCurLabelName(int t, int c)
{
	std::wstring label_name;
	if (t >= 0 && t < (int)m_4d_seq.size() &&
		!m_4d_seq[t].slices.empty())
	{
		std::wstring data_name = m_4d_seq[t].slices[0].slice;
		std::wstring label_name = data_name.substr(0, data_name.find_last_of('.')) + L".lbl";
		return label_name;
	}
	label_name = m_path_name.substr(0, m_path_name.find_last_of(L'.'));
	if (m_time_num > 1) label_name += L"_T" + std::to_wstring(t);
	if (m_chan_num > 1) label_name += L"_C" + std::to_wstring(c);
	label_name += L".lbl";
	return label_name;
}
