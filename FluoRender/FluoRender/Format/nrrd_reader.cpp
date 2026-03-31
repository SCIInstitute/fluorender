/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2026 Scientific Computing and Imaging Institute,
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
#include <nrrd_utility.h>
#include <algorithm>
#include <sstream>

NRRDReader::NRRDReader() :
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

void NRRDReader::SetFile(const std::wstring& file)
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
			m_4d_seq.push_back(info);
		}
	}
	if (m_4d_seq.size() > 0)
	{
		//std::sort(m_4d_seq.begin(), m_4d_seq.end(), NRRDReader::nrrd_sort);
		for (int t = 0; t < (int)m_4d_seq.size(); t++)
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
		FIND_FILES_BATCH(search_path, ESCAPE_REGEX(L".nrrd"), m_batch_list, m_cur_batch);
		m_batch = true;
	}
	else
		m_batch = false;
}

int NRRDReader::LoadBatch(int index)
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

std::shared_ptr<fluo::RawData>
NRRDReader::Convert(int t, int c, bool get_max)
{
	// ---- Clamp indices ---------------------------------------------------
	if (t < 0) t = 0;
	if (c < 0) c = 0;
	if (t >= m_time_num) t = m_time_num > 0 ? m_time_num - 1 : 0;
	if (c >= m_chan_num) c = m_chan_num > 0 ? m_chan_num - 1 : 0;

	const std::wstring filename = m_4d_seq[t].filename;
	m_data_name = GET_STEM(filename);

	FILE* nrrd_file = nullptr;
	if (!WFOPEN(&nrrd_file, filename, L"rb"))
		return nullptr;

	// ---- Read header only -------------------------------------------------
	Nrrd* nrrd = nrrdNew();
	NrrdIoState* nio = nrrdIoStateNew();
	nrrdIoStateSet(nio, nrrdIoStateSkipData, AIR_TRUE);

	if (nrrdRead(nrrd, nrrd_file, nio))
	{
		nrrdNuke(nrrd);
		fclose(nrrd_file);
		return nullptr;
	}

	nrrdIoStateNix(nio);
	rewind(nrrd_file);

	if (nrrd->dim != 3)
	{
		nrrdNuke(nrrd);
		fclose(nrrd_file);
		return nullptr;
	}

	// ---- Size & spacing ---------------------------------------------------
	const size_t nx = nrrd->axis[0].size;
	const size_t ny = nrrd->axis[1].size;
	const size_t nz = nrrd->axis[2].size;
	const size_t voxel_count = nx * ny * nz;

	m_size = fluo::Vector(nx, ny, nz);
	m_spacing = fluo::Vector(
		nrrd->axis[0].spacing,
		nrrd->axis[1].spacing,
		nrrd->axis[2].spacing
	);

	if (!m_spacing.any_le_zero())
		m_valid_spc = true;
	else
	{
		m_valid_spc = false;
		m_spacing = fluo::Vector(1.0);
	}

	// ---- Allocate raw buffer ---------------------------------------------
	const size_t bytes_per_elem = nrrdElementSize(nrrd);
	const size_t total_bytes = voxel_count * bytes_per_elem;

	auto* buffer = new unsigned char[total_bytes];
	nrrd->data = buffer;

	if (nrrdRead(nrrd, nrrd_file, nullptr))
	{
		delete[] buffer;
		nrrdNuke(nrrd);
		fclose(nrrd_file);
		return nullptr;
	}

	fclose(nrrd_file);

	// ---- Normalize / convert signed & large types -------------------------
	unsigned short min_value = 0;
	m_min_value = 0.0;
	m_max_value = 0.0;

	const int nrrd_type = nrrd->type;

	if (nrrd_type == nrrdTypeChar)
	{
		for (size_t i = 0; i < voxel_count; ++i)
			buffer[i] = static_cast<unsigned char>(
				static_cast<int8_t*>(nrrd->data)[i] + 128);

		nrrd->type = nrrdTypeUChar;
	}
	else if (nrrd_type == nrrdTypeShort)
	{
		min_value = 65535;
		auto* src = reinterpret_cast<int16_t*>(buffer);
		auto* dst = reinterpret_cast<uint16_t*>(buffer);

		for (size_t i = 0; i < voxel_count; ++i)
		{
			uint16_t v = static_cast<uint16_t>(src[i] + 32768);
			dst[i] = v;
			min_value = std::min(min_value, v);
			if (get_max)
			{
				m_min_value = (m_min_value == 0.0) ? v : std::min(m_min_value, (double)v);
				m_max_value = std::max(m_max_value, (double)v);
			}
		}

		nrrd->type = nrrdTypeUShort;
	}
	else if (nrrd_type == nrrdTypeInt || nrrd_type == nrrdTypeUInt)
	{
		auto* src = reinterpret_cast<uint32_t*>(buffer);
		auto* dst = reinterpret_cast<uint16_t*>(buffer);

		for (size_t i = 0; i < voxel_count; ++i)
		{
			uint16_t v = static_cast<uint16_t>(src[i] >> 8);
			dst[i] = v;
			if (get_max)
			{
				m_min_value = (m_min_value == 0.0) ? v : std::min(m_min_value, (double)v);
				m_max_value = std::max(m_max_value, (double)v);
			}
		}

		nrrd->type = nrrdTypeUShort;
	}

	// ---- Final scaling ----------------------------------------------------
	if (nrrd->type == nrrdTypeUChar)
	{
		m_max_value = 255.0;
		m_scalar_scale = 1.0;
	}
	else if (nrrd->type == nrrdTypeUShort)
	{
		m_max_value -= min_value;
		auto* d = reinterpret_cast<uint16_t*>(buffer);
		for (size_t i = 0; i < voxel_count; ++i)
			d[i] -= min_value;

		m_scalar_scale = (m_max_value > 0.0) ? 65535.0 / m_max_value : 1.0;
	}

	// ---- Create RawData (adopt buffer) ------------------------------------
	auto raw = std::make_shared<fluo::RawData>(
		fluo::RawData::Size3{ nx, ny, nz },
		FromNrrdScalar(nrrd->type),
		1, 1, 0, 0,
		reinterpret_cast<fluo::Byte*>(buffer),
		[](fluo::Byte* p)
		{
			delete[] reinterpret_cast<unsigned char*>(p);
		}
	);

	nrrd->data = nullptr;
	nrrdNuke(nrrd);

	m_cur_time = t;
	return raw;
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
