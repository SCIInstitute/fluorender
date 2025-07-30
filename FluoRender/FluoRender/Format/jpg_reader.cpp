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
#include <jpg_reader.h>
#include <Global.h>
#include <MainSettings.h>
#include <compatibility.h>
#include <jpeglib.h>

JPGReader::JPGReader() : BaseReader()
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
	m_max_value = 255.0;
	m_scalar_scale = 1.0;

	m_fp_convert = false;
	m_fp_min = 0;
	m_fp_max = 1;

	m_batch = false;
	m_cur_batch = -1;
	m_cur_time = -1;

	m_time_id = L"_T";
}

JPGReader::~JPGReader()
{
	// Cleanup if necessary
}

void JPGReader::SetFile(const std::wstring& file)
{
	m_path_name = file;
	m_id_string = file;
}

int JPGReader::Preprocess()
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

void JPGReader::GetFileInfo(const std::wstring& filename)
{
	FILE* infile = 0;
	if (!WFOPEN(&infile, filename, L"rb")) return;

	jpeg_decompress_struct cinfo;
	jpeg_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, infile);
	jpeg_read_header(&cinfo, TRUE);

	m_x_size = cinfo.image_width;
	m_y_size = cinfo.image_height;
	m_chan_num = cinfo.num_components;

	jpeg_destroy_decompress(&cinfo);
	fclose(infile);
}

void JPGReader::SetBatch(bool batch)
{
	if (batch)
	{
		//read the directory info
		std::wstring search_path = GET_PATH(m_path_name);
		FIND_FILES_BATCH(search_path, ESCAPE_REGEX(L".jpg"), m_batch_list, m_cur_batch);
		m_batch = true;
	}
	else
		m_batch = false;
}

int JPGReader::LoadBatch(int index)
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

Nrrd* JPGReader::Convert(int t, int c, bool get_max)
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
	data = ReadJpg(chan_info.slices, c, get_max);
	m_cur_time = t;
	return data;
}

Nrrd* JPGReader::ReadJpg(const std::vector<SliceInfo>& filelist, int c, bool get_max)
{
	if (filelist.empty())
		return nullptr;

	Nrrd* nrrdout = nrrdNew();

	unsigned long long total_size = (unsigned long long)m_x_size *
		(unsigned long long)m_y_size * (unsigned long long)m_slice_num;
	void* val = (void*)(new unsigned char[total_size]);
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
		if (!ReadSingleJpg(static_cast<void*>(val_ptr), slice_info.slice, c))
		{
			delete[] static_cast<unsigned char*>(val);
			return nullptr;
		}
		if (show_progress)
			SetProgress(static_cast<int>(std::round(100.0 * (i + 1) / for_size)), "NOT_SET");
		val_ptr += m_x_size * m_y_size;
	}

	//write to nrrd
	nrrdWrap_va(nrrdout, (uint8_t*)val, nrrdTypeUChar,
		3, (size_t)m_x_size, (size_t)m_y_size, (size_t)m_slice_num);
	nrrdAxisInfoSet_va(nrrdout, nrrdAxisInfoSpacing, m_xspc, m_yspc, m_zspc);
	nrrdAxisInfoSet_va(nrrdout, nrrdAxisInfoMax, m_xspc * m_x_size,
		m_yspc * m_y_size, m_zspc * m_slice_num);
	nrrdAxisInfoSet_va(nrrdout, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
	nrrdAxisInfoSet_va(nrrdout, nrrdAxisInfoSize, (size_t)m_x_size,
		(size_t)m_y_size, (size_t)m_slice_num);

	return nrrdout;
}

bool JPGReader::ReadSingleJpg(void* val, const std::wstring& filename, int c)
{
	FILE* infile = 0;
	if (!WFOPEN(&infile, filename, L"rb")) return false;

	jpeg_decompress_struct cinfo;
	jpeg_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, infile);

	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);

	int width = cinfo.output_width;
	int height = cinfo.output_height;
	int channels = cinfo.output_components;

	if (c < 0 || c >= channels) {
		jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);
		fclose(infile);
		return false; // Invalid channel index
	}

	size_t row_stride = width * channels;
	unsigned char* channel_data = static_cast<unsigned char*>(val);
	std::vector<unsigned char> scanline(row_stride);

	for (size_t row = 0; row < height; ++row)
	{
		unsigned char* buffer_array[1] = { scanline.data() };
		jpeg_read_scanlines(&cinfo, buffer_array, 1);

		for (size_t col = 0; col < width; ++col)
		{
			channel_data[row * width + col] = scanline[col * channels + c];
		}
	}

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	fclose(infile);

	return true;
}

std::wstring JPGReader::GetCurDataName(int t, int c)
{
	if (t >= 0 && t < (int)m_4d_seq.size() &&
		!m_4d_seq[t].slices.empty())
		return m_4d_seq[t].slices[0].slice;
	return m_path_name;
}

std::wstring JPGReader::GetCurMaskName(int t, int c)
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

std::wstring JPGReader::GetCurLabelName(int t, int c)
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
