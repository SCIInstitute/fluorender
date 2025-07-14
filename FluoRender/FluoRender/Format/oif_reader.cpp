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
#include <oif_reader.h>
#include <Global.h>
#include <MainSettings.h>
#include <compatibility.h>
#include <algorithm>

OIFReader::OIFReader():
	BaseReader()
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

	m_min_value = 0.0;
	m_max_value = 0.0;
	m_scalar_scale = 1.0;

	m_fp_convert = false;
	m_fp_min = 0;
	m_fp_max = 1;

	m_batch = false;
	m_cur_batch = -1;

	m_time_id = L"_T";
	m_type = 0;
	m_oif_t = 0;
}

OIFReader::~OIFReader()
{
}

//void OIFReader::SetFile(const std::string &file)
//{
//	if (!file.empty())
//	{
//		if (!m_path_name.empty())
//			m_path_name.clear();
//		m_path_name.assign(file.length(), L' ');
//		copy(file.begin(), file.end(), m_path_name.begin());
//
//		m_data_name = GET_STEM(m_path_name);
//	}
//	m_id_string = m_path_name;
//}

void OIFReader::SetFile(const std::wstring &file)
{
	m_path_name = file;
	m_data_name = GET_STEM(m_path_name);
	m_id_string = m_path_name;
}

int OIFReader::Preprocess()
{
	m_type = 0;
	m_oif_info.clear();

	//separate path and name
	std::wstring path, name;
	if (!SEP_PATH_NAME(m_path_name, path, name))
		return READER_OPEN_FAIL;

	//build 4d sequence
	//search time sequence files
	std::vector< std::wstring> list;
	if (!FIND_FILES_4D(m_path_name, m_time_id, list, m_cur_time))
	{
		ReadSingleOif();
	}
	else
	{
		//search time sequence files
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
			info.filename = list.at(i);
			m_oif_info.push_back(info);
		}

		if (m_oif_info.size() > 0)
		{
			m_type = 1;
			//std::sort(m_oif_info.begin(), m_oif_info.end(), OIFReader::oif_sort);
			ReadSequenceOif();
		}
		else
		{
			m_oif_info.clear();
			ReadSingleOif();
		}
	}

	ReadOif();

	m_time_num = int(m_oif_info.size());
	if (m_type == 0)
		m_cur_time = 0;
	m_chan_num = m_time_num > 0 ? int(m_oif_info[0].dataset.size()) : 0;
	m_slice_num = m_chan_num > 0 ? int(m_oif_info[0].dataset[0].size()) : 0;

	return READER_OK;
}

bool OIFReader::oif_sort(const TimeDataInfo& info1, const TimeDataInfo& info2)
{
	return info1.filenumber < info2.filenumber;
}

void OIFReader::ReadSingleOif()
{
	std::filesystem::path p(m_path_name + L".files");
	p /= "";
	m_subdir_name = p.wstring();
	std::vector<std::wstring> list;
	int tmp;
	FIND_FILES_BATCH(m_subdir_name, ESCAPE_REGEX(L".tif"), list, tmp);
	//read file sequence
	for (size_t f = 0; f < list.size(); f++)
		ReadTifSequence(list.at(f));
}

void OIFReader::ReadSequenceOif()
{
	for (int i = 0; i < (int)m_oif_info.size(); i++)
	{
		std::wstring path_name = m_oif_info[i].filename;
		std::filesystem::path p(path_name + L".files");
		p /= "";
		m_oif_info[i].subdirname = p.wstring();

		if (path_name == m_path_name)
			m_cur_time = i;

		m_subdir_name = p.wstring();
		std::vector<std::wstring> list;
		FIND_FILES_BATCH(m_subdir_name, ESCAPE_REGEX(L".tif"), list, m_oif_t);
		//read file sequence
		for (size_t f = 0; f < list.size(); f++)
			ReadTifSequence(list.at(f), i);
	}
}

void OIFReader::SetSliceSeq(bool ss)
{
	//do nothing
}

bool OIFReader::GetSliceSeq()
{
	return false;
}

void OIFReader::SetChannSeq(bool cs)
{
	//do nothing
}

bool OIFReader::GetChannSeq()
{
	return false;
}

void OIFReader::SetDigitOrder(int order)
{
	//do nothing
}

int OIFReader::GetDigitOrder()
{
	return 0;
}

void OIFReader::SetTimeId(const std::wstring &id)
{
	m_time_id = id;
}

std::wstring OIFReader::GetTimeId()
{
	return m_time_id;
}

void OIFReader::SetBatch(bool batch)
{
	if (batch)
	{
		//read the directory info
		std::wstring search_path = GET_PATH(m_path_name);
		FIND_FILES_BATCH(search_path, ESCAPE_REGEX(L".oif"), m_batch_list, m_cur_batch);
		m_batch = true;
	}
	else
		m_batch = false;
}

int OIFReader::LoadBatch(int index)
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

void OIFReader::ReadTifSequence(const std::wstring& file_name, int t)
{
	size_t line_size = file_name.size();
	if (file_name.substr(line_size - 3, 3) == L"tif")
	{
		//interpret file_name
		int64_t pos;
		int64_t pos_ = file_name.find_last_of(L'_');
		if (pos_ != -1)
		{
			size_t j;
			std::wstring wstr;
			int num_c = -1;
			int num_z = -1;
			int num_t = -1;
			int num_l = -1;

			//read channel number 'C'
			pos = file_name.find(L'C', pos_ + 1);
			if (pos != -1)
			{
				for (j = pos + 1; j < file_name.size(); j++)
				{
					if (iswdigit(file_name[j]))
						wstr.push_back(file_name[j]);
					else
						break;
				}
				num_c = WSTOI(wstr);
				wstr.clear();
			}
			//read z number 'Z'
			pos = file_name.find(L'Z', pos_ + 1);
			if (pos != -1)
			{
				for (j = pos + 1; j < file_name.size(); j++)
				{
					if (iswdigit(file_name[j]))
						wstr.push_back(file_name[j]);
					else
						break;
				}
				num_z = WSTOI(wstr);
				wstr.clear();
			}
			//read time number 'T'
			pos = file_name.find(L'T', pos_ + 1);
			if (pos != -1)
			{
				for (j = pos + 1; j < file_name.size(); j++)
				{
					if (iswdigit(file_name[j]))
						wstr.push_back(file_name[j]);
					else
						break;
				}
				num_t = WSTOI(wstr);
				wstr.clear();
			}
			//read lambda number 'L'
			pos = file_name.find(L'L', pos_ + 1);
			if (pos != -1)
			{
				for (j = pos + 1; j < file_name.size(); j++)
				{
					if (iswdigit(file_name[j]))
						wstr.push_back(file_name[j]);
					else
						break;
				}
				num_l = WSTOI(wstr);
				wstr.clear();
			}

			//add info to the list
			num_c = num_c == -1 ? 0 : num_c - 1;
			num_t = num_t == -1 ? t : num_t - 1;
			num_z = num_z == -1 ? 1 : num_z;
			if (num_z > 0)
			{
				num_z--;
				//allocate
				if (m_type == 0)
				{
					if (int(m_oif_info.size()) < num_t + 1)
						m_oif_info.resize(num_t + 1);
					if (int(m_oif_info[num_t].dataset.size()) < num_c + 1)
						m_oif_info[num_t].dataset.resize(num_c + 1);
					if (int(m_oif_info[num_t].dataset[num_c].size()) < num_z + 1)
						m_oif_info[num_t].dataset[num_c].resize(num_z + 1);
					//add
					m_oif_info[num_t].dataset[num_c][num_z] = file_name;
				}
				else
				{
					//if (num_t == 0)
					{
						if (int(m_oif_info[num_t].dataset.size()) < num_c + 1)
							m_oif_info[num_t].dataset.resize(num_c + 1);
						if (int(m_oif_info[num_t].dataset[num_c].size()) < num_z + 1)
							m_oif_info[num_t].dataset[num_c].resize(num_z + 1);
						//add
						m_oif_info[num_t].dataset[num_c][num_z] = file_name;
					}
				}
			}
		}
	}
}

void OIFReader::ReadOif()
{
	//read oif file
#ifdef _WIN32
	std::ifstream is(m_path_name.c_str());
#else
	std::ifstream is(ws2s(m_path_name).c_str());
#endif
	std::wstring oneline;
	if (is.is_open())
	{
		//reset
		m_excitation_wavelength_list.clear();
		m_x_size = 0;
		m_y_size = 0;
		m_xspc = 0.0;
		m_yspc = 0.0;
		m_zspc = 0.0;
		//axis count
		axis_num = -1;
		cur_axis = -1;
		//channel count
		chan_num = -1;
		cur_chan = -1;
		//axis info
		axis_code.clear();
		pix_unit.clear();
		max_size.clear();
		start_pos.clear();
		end_pos.clear();

		while (!is.eof())
		{
			wchar_t c;
			is.read(((char*)(&c)), 1);
			if (!is.eof())
				is.read(((char*)(&c)) + 1, 1);
			if (c != L'\x0D' && c != L'\n')
				oneline.push_back(c);
			else
			{
				if (!oneline.empty())
					ReadOifLine(oneline);
				oneline.clear();
			}
		}
		is.close();
	}

	if (m_xspc > 0.0 && m_xspc<100.0 &&
		m_yspc>0.0 && m_yspc<100.0)
	{
		m_valid_spc = true;
		if (m_zspc <= 0.0 || m_zspc>100.0)
			m_zspc = std::max(m_xspc, m_yspc);
	}
	else
	{
		m_valid_spc = false;
		m_xspc = 1.0;
		m_yspc = 1.0;
		m_zspc = 1.0;
	}
}

void OIFReader::ReadOifLine(const std::wstring oneline)
{
	//process
	if (oneline.substr(0, 6) == L"[Axis ")
	{
		axis_num++;
	}
	else
	{
		if (axis_num > -1)
		{
			size_t pos = oneline.find(L'=');
			std::wstring str1 = oneline.substr(0, oneline.find_last_not_of(L' ', pos));
			std::wstring str2 = oneline.substr(oneline.find_first_not_of(L' ', pos + 1));

			if (str1 == L"AxisCode")
			{
				if (cur_axis != axis_num)
				{
					cur_axis = axis_num;
					axis_code.clear();
					pix_unit.clear();
					max_size.clear();
					start_pos.clear();
					end_pos.clear();
				}
				axis_code = str2;
			}
			else if (str1 == L"PixUnit")
			{
				if (cur_axis != axis_num)
				{
					cur_axis = axis_num;
					axis_code.clear();
					pix_unit.clear();
					max_size.clear();
					start_pos.clear();
					end_pos.clear();
				}
				pix_unit = str2;
			}
			else if (str1 == L"MaxSize")
			{
				if (cur_axis != axis_num)
				{
					cur_axis = axis_num;
					axis_code.clear();
					pix_unit.clear();
					max_size.clear();
					start_pos.clear();
					end_pos.clear();
				}
				max_size = str2;
			}
			else if (str1 == L"StartPosition")
			{
				if (cur_axis != axis_num)
				{
					cur_axis = axis_num;
					axis_code.clear();
					pix_unit.clear();
					max_size.clear();
					start_pos.clear();
					end_pos.clear();
				}
				start_pos = str2;
			}
			else if (str1 == L"EndPosition")
			{
				if (cur_axis != axis_num)
				{
					cur_axis = axis_num;
					axis_code.clear();
					pix_unit.clear();
					max_size.clear();
					start_pos.clear();
					end_pos.clear();
				}
				end_pos = str2;
			}
		}
	}
	if (oneline.substr(0, 9) == L"[Channel ")
	{
		light_type.clear();
		chan_num++;
	}
	else
	{
		if (chan_num > -1)
		{
			size_t pos = oneline.find(L'=');
			std::wstring str1 = oneline.substr(0, oneline.find_last_not_of(L' ', pos));
			std::wstring str2 = oneline.substr(oneline.find_first_not_of(L' ', pos + 1));
			std::wstring str3 = L"Transmitted Light";
			if (str1 == L"LightType") {
				light_type = str2;
				if (light_type.find(str3) != std::wstring::npos) {
					for (size_t i = m_excitation_wavelength_list.size(); i > 0; --i) {
						if (m_excitation_wavelength_list.at(i-1).chan_num == cur_chan) {
							m_excitation_wavelength_list.at(i-1).wavelength = -1;
							break;
						}
					}
				}
			}
			else if (str1 == L"ExcitationWavelength")
			{
				if (cur_chan != chan_num)
				{
					cur_chan = chan_num;
					WavelengthInfo info;
					info.chan_num = cur_chan;
					info.wavelength = WSTOD(str2);
					if (light_type == L"Transmitted Light")
						info.wavelength = -1;
					m_excitation_wavelength_list.push_back(info);
				}
			}
		}
	}

	//axis
	if (!axis_code.empty() &&
		!pix_unit.empty() &&
		!max_size.empty() &&
		!start_pos.empty() &&
		!end_pos.empty())
	{
		//calculate
		double spc = 0.0;
		double dmax = WSTOD(max_size);
		if (dmax > 0.0)
			spc = fabs((WSTOD(end_pos) -
				WSTOD(start_pos))) /
			dmax;
		if ((int64_t)pix_unit.find(L"nm") != -1)
			spc /= 1000.0;
		if ((int64_t)axis_code.find(L"X") != -1)
		{
			m_x_size = WSTOI(max_size);
			m_xspc = spc;
		}
		else if ((int64_t)axis_code.find(L"Y") != -1)
		{
			m_y_size = WSTOI(max_size);
			m_yspc = spc;
		}
		else if ((int64_t)axis_code.find(L"Z") != -1)
			m_zspc = spc;

		axis_code.clear();
		pix_unit.clear();
		max_size.clear();
		start_pos.clear();
		end_pos.clear();
	}
}

double OIFReader::GetExcitationWavelength(int chan)
{
	for (int i = 0; i < (int)m_excitation_wavelength_list.size(); i++)
	{
		if (m_excitation_wavelength_list[i].chan_num == chan)
			return m_excitation_wavelength_list[i].wavelength;
	}
	return 0.0;
}

Nrrd* OIFReader::Convert(int t, int c, bool get_max)
{
	Nrrd *data = 0;
	int sl_num = 0;

	if (t >= 0 && t < m_time_num &&
		c >= 0 && c < m_chan_num &&
		m_slice_num>0 &&
		m_x_size>0 &&
		m_y_size>0)
	{
		//allocate memory for nrrd
		unsigned long long mem_size = (unsigned long long)m_x_size*
			(unsigned long long)m_y_size*(unsigned long long)m_slice_num;
		unsigned short *val = new (std::nothrow) unsigned short[mem_size];
		bool show_progress = mem_size > glbin_settings.m_prg_size;

		//read the channel
		ChannelInfo *cinfo = &m_oif_info[t].dataset[c];
		size_t num = cinfo->size();
		for (size_t i = 0; i < num; i++)
		{
			char *pbyData = 0;
			std::wstring file_name = (*cinfo)[i];

			//open file
			std::ifstream is;
#ifdef _WIN32
			is.open(file_name.c_str(), std::ios::binary);
#else
			is.open(ws2s(file_name).c_str(), std::ios::binary);
#endif
			if (is.is_open())
			{
				is.seekg(0, std::ios::end);
				size_t size = is.tellg();
				pbyData = new char[size];
				is.seekg(0, std::ios::beg);
				is.read(pbyData, size);
				is.close();

				//read
				ReadTiff(pbyData, val, static_cast<int>(i));

				//increase
				sl_num++;
			}

			if (pbyData)
				delete[]pbyData;

			if (show_progress && m_time_num == 1)
				SetProgress(static_cast<int>(std::round(100.0 * (i + 1) / num)), "NOT_SET");
		}

		//create nrrd
		if (val && sl_num == m_slice_num)
		{
			//ok
			data = nrrdNew();
			nrrdWrap_va(data, val, nrrdTypeUShort, 3, (size_t)m_x_size, (size_t)m_y_size, (size_t)m_slice_num);
			nrrdAxisInfoSet_va(data, nrrdAxisInfoSpacing, m_xspc, m_yspc, m_zspc);
			nrrdAxisInfoSet_va(data, nrrdAxisInfoMax, m_xspc*m_x_size, m_yspc*m_y_size, m_zspc*m_slice_num);
			nrrdAxisInfoSet_va(data, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
			nrrdAxisInfoSet_va(data, nrrdAxisInfoSize, (size_t)m_x_size, (size_t)m_y_size, (size_t)m_slice_num);
		}
		else
		{
			//something is wrong
			if (val)
				delete[]val;
		}
	}

	if (m_max_value > 0.0)
		m_scalar_scale = 65535.0 / m_max_value;

	m_cur_time = t;
	return data;
}

std::wstring OIFReader::GetCurDataName(int t, int c)
{
	return m_oif_info[t].dataset[c][0];
}

std::wstring OIFReader::GetCurMaskName(int t, int c)
{
	std::wstring data_name = m_oif_info[t].dataset[c][0];
	std::wstring mask_name = data_name.substr(0, data_name.find_last_of(L'.')) + L".msk";
	return mask_name;
}

std::wstring OIFReader::GetCurLabelName(int t, int c)
{
	std::wstring data_name = m_oif_info[t].dataset[c][0];
	std::wstring label_name = data_name.substr(0, data_name.find_last_of(L'.')) + L".lbl";
	return label_name;
}

void OIFReader::ReadTiff(char *pbyData, unsigned short *val, int z)
{
	if (*((unsigned int*)pbyData) != 0x002A4949)
		return;

	int compression = 0;
	unsigned int offset = 0;
	//directory offset
	offset = *((unsigned int*)(pbyData + 4));
	//the directory
	//entry number
	int entry_num = *((unsigned short*)(pbyData + offset));
	//strip info
	int strips = 0;
	int rows = 0;
	std::vector <unsigned int> strip_offsets;
	std::vector <unsigned int> strip_bytes;
	//get strip info
	unsigned int s_num1 = 0;
	unsigned int s_num2 = 0;
	for (int i = 0; i < entry_num; i++)
	{
		//read each entry (12 bytes)
		unsigned short tag = *((unsigned short*)(pbyData + offset + 2 + 12 * i));
		switch (tag)
		{
		case 0x0103:  //259, compression
		{
			unsigned short value;
			value = *((unsigned short*)(pbyData + offset + 2 + 12 * i + 8));
			compression = value << 16 >> 16;
		}
		break;
		case 0x0111:  //strip offsets
		{
			unsigned short type = *((unsigned short*)(pbyData + offset + 2 + 12 * i + 2));
			//number of values
			s_num1 = *((unsigned int*)(pbyData + offset + 2 + 12 * i + 4));
			unsigned int entry_offset = 0;
			entry_offset = *((unsigned int*)(pbyData + offset + 2 + 12 * i + 8));
			for (int j = 0; j<int(s_num1); j++)
			{
				if (type == 3)
				{
					//unsigned short
					unsigned short value;
					value = *((unsigned short*)(pbyData + entry_offset + 2 * j));
					strip_offsets.push_back((unsigned int)value);
				}
				else if (type == 4)
				{
					//unsigned int
					unsigned int value;
					value = *((unsigned int*)(pbyData + entry_offset + 4 * j));
					strip_offsets.push_back(value);
				}
			}
		}
		break;
		case 0x0116:  //rows per strip
		{
			unsigned short type = *((unsigned short*)(pbyData + offset + 2 + 12 * i + 2));
			if (type == 3)
			{
				//unsigned short
				unsigned short value;
				value = *((unsigned short*)(pbyData + offset + 2 + 12 * i + 8));
				rows = value;
			}
			else if (type == 4)
			{
				//unsigned int
				unsigned int value;
				value = *((unsigned int*)(pbyData + offset + 2 + 12 * i + 8));
				rows = value;
			}
		}
		break;
		case 0x0117:  //strip byte counts
		{
			unsigned short type = *((unsigned short*)(pbyData + offset + 2 + 12 * i + 2));
			//number of values
			s_num2 = *((unsigned int*)(pbyData + offset + 2 + 12 * i + 4));
			unsigned int entry_offset = 0;
			entry_offset = *((unsigned int*)(pbyData + offset + 2 + 12 * i + 8));
			for (int j = 0; j<int(s_num2); j++)
			{
				if (type == 3)
				{
					//unsigned short
					unsigned short value;
					value = *((unsigned short*)(pbyData + entry_offset + 2 * j));
					strip_bytes.push_back((unsigned int)value);
				}
				else if (type == 4)
				{
					//unsigned int
					unsigned int value;
					value = *((unsigned int*)(pbyData + entry_offset + 4 * j));
					strip_bytes.push_back(value);
				}
			}
		}
		break;
		case 0x0119:  //max sample value
		{
			unsigned short value;
			value = *((unsigned short*)(pbyData + offset + 2 + 12 * i + 8));
			if ((double)value > m_max_value)
				m_max_value = (double)value;
		}
		break;
		}
	}
	//read strips
	if (s_num1 == s_num2 &&
		strip_offsets.size() == s_num1 &&
		strip_bytes.size() == s_num2)
	{
		strips = s_num1;

		unsigned int val_pos = z*m_x_size*m_y_size;
		for (int i = 0; i < strips; i++)
		{
			unsigned int data_pos = strip_offsets[i];
			unsigned int data_size = strip_bytes[i];
			if (compression == 1)//no copmression
				memcpy((void*)(val + val_pos), (void*)(pbyData + data_pos), data_size);
			else if (compression == 5)
				LZWDecode((tidata_t)(pbyData + data_pos), (tidata_t)(val + val_pos), m_x_size*rows * 2);
			val_pos += rows*m_x_size;
		}
	}
}
