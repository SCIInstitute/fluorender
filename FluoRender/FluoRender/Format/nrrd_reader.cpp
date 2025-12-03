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
#include <nrrd_reader.h>
#include <compatibility.h>
#include <algorithm>
#include <sstream>

NRRDReader::NRRDReader():
	BaseVolReader()
{
	m_time_num = 0;
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
	m_cur_time = -1;

	m_time_id = L"_T";
}

NRRDReader::~NRRDReader()
{
}

//void NRRDReader::SetFile(const std::string &file)
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

void NRRDReader::SetFile(const std::wstring &file)
{
	m_path_name = file;
	m_id_string = m_path_name;
}

int NRRDReader::Preprocess()
{
	m_4d_seq.clear();

	//separate path and name
	std::wstring path, name;
	if (!SEP_PATH_NAME(m_path_name, path, name))
		return READER_OPEN_FAIL;

	//build 4d sequence
	//search time sequence files
	std::vector< std::wstring> list;
	if (!FIND_FILES_4D(m_path_name, m_time_id, list, m_cur_time))
	{
		TimeDataInfo info;
		info.filenumber = 0;
		info.filename = m_path_name;
		m_4d_seq.push_back(info);
		m_cur_time = 0;
	}
	else
	{
		int64_t begin = m_path_name.find(m_time_id);
		size_t id_len = m_time_id.length();
		for(size_t i = 0; i < list.size(); i++) {
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
			m_4d_seq.push_back(info);
		}
	}
	if (m_4d_seq.size() > 0)
	{
		//std::sort(m_4d_seq.begin(), m_4d_seq.end(), NRRDReader::nrrd_sort);
		for (int t=0; t<(int)m_4d_seq.size(); t++)
		{
			if (m_4d_seq[t].filename == m_path_name)
			{
				m_cur_time = t;
				break;
			}
		}
	}
	else
		m_cur_time = 0;

	//3D nrrd file
	m_chan_num = 1;
	//get time number
	m_time_num = (int)m_4d_seq.size();

	return READER_OK;
}

void NRRDReader::SetBatch(bool batch)
{
	if (batch)
	{
		//read the directory info
		std::wstring search_path = GET_PATH(m_path_name);
		FIND_FILES_BATCH(search_path,ESCAPE_REGEX(L".nrrd"),m_batch_list,m_cur_batch);
		m_batch = true;
	}
	else
		m_batch = false;
}

int NRRDReader::LoadBatch(int index)
{
	int result = -1;
	if (index>=0 && index<(int)m_batch_list.size())
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

Nrrd* NRRDReader::Convert(int t, int c, bool get_max)
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

	std::wstring str_name = m_4d_seq[t].filename;
	m_data_name = GET_STEM(str_name);
	FILE* nrrd_file = 0;
	if (!WFOPEN(&nrrd_file, str_name, L"rb"))
		return 0;

	Nrrd *output = nrrdNew();
	NrrdIoState *nio = nrrdIoStateNew();
	nrrdIoStateSet(nio, nrrdIoStateSkipData, AIR_TRUE);
	if (nrrdRead(output, nrrd_file, nio))
	{
		fclose(nrrd_file);
		return 0;
	}
	nio = nrrdIoStateNix(nio);
	rewind(nrrd_file);
	if (output->dim != 3)
	{
		nrrdNuke(output);
		fclose(nrrd_file);
		return 0;
	}
	m_size = fluo::Vector(
		output->axis[0].size,
		output->axis[1].size,
		output->axis[2].size);
	m_spacing = fluo::Vector(
		output->axis[0].spacing,
		output->axis[1].spacing,
		output->axis[2].spacing);
	if (!m_spacing.any_le_zero())
		m_valid_spc = true;
	else
	{
		m_valid_spc = false;
		m_spacing = fluo::Vector(1.0);
	}

	unsigned long long data_size = (unsigned long long)m_size.get_size_xyz();
	unsigned long long nsize = data_size;
	if (output->type == nrrdTypeUShort || output->type == nrrdTypeShort)
		data_size *= 2;
	else if (output->type == nrrdTypeInt ||
		output->type == nrrdTypeUInt)
		data_size *= 4;
	output->data = new unsigned char[data_size];

	if (nrrdRead(output, nrrd_file, NULL))
	{
		nrrdNuke(output);
		fclose(nrrd_file);
		return 0;
	}

	m_max_value = 0.0;
	// turn signed into unsigned
	if (output->type == nrrdTypeChar)
	{
		for (unsigned long long idx=0; idx < nsize; ++idx)
		{
			char val = ((char*)output->data)[idx];
			unsigned char n = val + 128;
			((unsigned char*)output->data)[idx] = n;
		}
		output->type = nrrdTypeUChar;
	}
	// turn signed into unsigned
	unsigned short min_value, n;
	if (output->type == nrrdTypeShort)
	{
		min_value = 32768;
		for (unsigned long long idx = 0; idx < nsize; ++idx)
		{
			short val = ((short*)output->data)[idx];
			n = val + 32768;
			((unsigned short*)output->data)[idx] = n;
			min_value = (n < min_value) ? n : min_value;
			if (get_max)
			{
				m_min_value = m_min_value == 0.0 ? n : (n < m_min_value ? n : m_min_value);
				m_max_value = (n > m_max_value) ? n : m_max_value;
			}
		}
		output->type = nrrdTypeUShort;
	}
	else if (output->type == nrrdTypeUShort)
	{
		min_value = 0;
		for (unsigned long long idx = 0; idx < nsize; ++idx)
		{
			n =  ((unsigned short*)output->data)[idx];
			if (get_max)
			{
				m_min_value = m_min_value == 0.0 ? n : (n < m_min_value ? n : m_min_value);
				m_max_value = (n > m_max_value) ? n : m_max_value;
			}
		}
	}
	//compress int
	if (output->type == nrrdTypeInt)
	{
		min_value = 32768;
		for (unsigned long long idx = 0; idx < nsize; ++idx)
		{
			int val = ((int*)output->data)[idx];
			val += 0x80000000;
			n = (unsigned short)(val >> 8);
			((unsigned short*)output->data)[idx] = n;
			min_value = (n < min_value) ? n : min_value;
			if (get_max)
			{
				m_min_value = m_min_value == 0.0 ? n : (n < m_min_value ? n : m_min_value);
				m_max_value = (n > m_max_value) ? n : m_max_value;
			}
		}
		output->type = nrrdTypeUShort;
	}
	else if (output->type == nrrdTypeUInt)
	{
		min_value = 0;
		for (unsigned long long idx = 0; idx < nsize; ++idx)
		{
			int val = ((unsigned int*)output->data)[idx];
			n = (unsigned short)(val >> 8);
			((unsigned short*)output->data)[idx] = n;
			if (get_max)
			{
				m_min_value = m_min_value == 0.0 ? n : (n < m_min_value ? n : m_min_value);
				m_max_value = (n > m_max_value) ? n : m_max_value;
			}
		}
		output->type = nrrdTypeUShort;
	}
	//find max value
	if (output->type == nrrdTypeUChar)
	{
		//8 bit
		m_max_value = 255.0;
		m_scalar_scale = 1.0;
	}
	else if (output->type == nrrdTypeUShort)
	{
		//16 bit
		m_max_value -= min_value;
		for (unsigned long long idx=0; idx < nsize; ++idx) {
			((unsigned short*)output->data)[idx] =
				((unsigned short*)output->data)[idx] - min_value;
		}
		if (m_max_value > 0.0)
			m_scalar_scale = 65535.0 / m_max_value;
		else
			m_scalar_scale = 1.0;
	}
	else
	{
		nrrdNuke(output);
		fclose(nrrd_file);
		return 0;
	}

	m_cur_time = t;
	fclose(nrrd_file);
	return output;
}

bool NRRDReader::nrrd_sort(const TimeDataInfo& info1, const TimeDataInfo& info2)
{
	return info1.filenumber < info2.filenumber;
}

std::wstring NRRDReader::GetCurDataName(int t, int c)
{
	if (t >= 0 && t < (int)m_4d_seq.size())
		return m_4d_seq[t].filename;
	return m_path_name;
}

std::wstring NRRDReader::GetCurMaskName(int t, int c)
{
	std::wstring mask_name;
	if (t >= 0 && t < (int)m_4d_seq.size())
	{
		std::wstring data_name = m_4d_seq[t].filename;
		mask_name = data_name.substr(0, data_name.find_last_of('.')) + L".msk";
		return mask_name;
	}
	mask_name = m_path_name.substr(0, m_path_name.find_last_of(L'.'));
	if (m_time_num > 1) mask_name += L"_T" + std::to_wstring(t);
	if (m_chan_num > 1) mask_name += L"_C" + std::to_wstring(c);
	mask_name += L".msk";
	return mask_name;
}

std::wstring NRRDReader::GetCurLabelName(int t, int c)
{
	std::wstring label_name;
	if (t >= 0 && t < (int)m_4d_seq.size())
	{
		std::wstring data_name = m_4d_seq[t].filename;
		std::wstring label_name = data_name.substr(0, data_name.find_last_of('.')) + L".lbl";
		return label_name;
	}
	label_name = m_path_name.substr(0, m_path_name.find_last_of(L'.'));
	if (m_time_num > 1) label_name += L"_T" + std::to_wstring(t);
	if (m_chan_num > 1) label_name += L"_C" + std::to_wstring(c);
	label_name += L".lbl";
	return label_name;
}
