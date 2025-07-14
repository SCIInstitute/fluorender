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
#include <png_reader.h>
#include <Global.h>
#include <MainSettings.h>
#include <compatibility.h>
#include <png.h>

PNGReader::PNGReader() : BaseReader()
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
}

PNGReader::~PNGReader()
{
	// Cleanup if necessary
}

void PNGReader::SetFile(const std::wstring& file)
{
	m_path_name = file;
	m_id_string = file;
}

int PNGReader::Preprocess()
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

void PNGReader::GetFileInfo(const std::wstring& filename)
{
	std::ifstream png_stream;
#ifdef _WIN32
	png_stream.open(filename.c_str(), std::ifstream::binary);
#else
	png_stream.open(ws2s(filename).c_str(), std::ifstream::binary);
#endif

	if (!png_stream.is_open())
		return;

	// Verify PNG signature
	unsigned char signature[8];
	png_stream.read(reinterpret_cast<char*>(signature), 8);
	if (png_sig_cmp(signature, 0, 8))
		return;

	// Read IHDR chunk
	uint32_t length = 0;
	png_stream.read(reinterpret_cast<char*>(&length), 4);
	length = SwapWord(length); // PNG uses big-endian

	char chunk_type[5] = { 0 };
	png_stream.read(chunk_type, 4);
	if (std::string(chunk_type) != "IHDR")
		return;

	// Read width and height (big-endian)
	uint8_t dim_bytes[8];
	png_stream.read(reinterpret_cast<char*>(dim_bytes), 8);

	m_x_size = (dim_bytes[0] << 24) | (dim_bytes[1] << 16) | (dim_bytes[2] << 8) | dim_bytes[3];
	m_y_size = (dim_bytes[4] << 24) | (dim_bytes[5] << 16) | (dim_bytes[6] << 8) | dim_bytes[7];

	uint8_t bit_depth = 0, color_type = 0;
	png_stream.read(reinterpret_cast<char*>(&bit_depth), 1);
	png_stream.read(reinterpret_cast<char*>(&color_type), 1);

	// Store bit depth
	m_bits = static_cast<int>(bit_depth);

	// Map color type to channel count
	switch (color_type)
	{
	case PNG_COLOR_TYPE_GRAY:
		m_chan_num = 1;
		break;
	case PNG_COLOR_TYPE_GRAY_ALPHA:
		m_chan_num = 1;
		break;
	case PNG_COLOR_TYPE_RGB:
		m_chan_num = 3;
		break;
	case PNG_COLOR_TYPE_RGB_ALPHA:
		m_chan_num = 3;
		break;
	default:
		m_chan_num = 0; // Unknown or unsupported
		break;
	}
}

void PNGReader::SetBatch(bool batch)
{
	if (batch)
	{
		//read the directory info
		std::wstring search_path = GET_PATH(m_path_name);
		FIND_FILES_BATCH(search_path, ESCAPE_REGEX(L".png"), m_batch_list, m_cur_batch);
		m_batch = true;
	}
	else
		m_batch = false;
}

int PNGReader::LoadBatch(int index)
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

Nrrd* PNGReader::Convert(int t, int c, bool get_max)
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
	data = ReadPng(chan_info.slices, c, get_max);
	m_cur_time = t;
	return data;
}

Nrrd* PNGReader::ReadPng(const std::vector<SliceInfo>& filelist, int c, bool get_max)
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

	void* val_ptr = val;
	size_t for_size = filelist.size();
	for (size_t i = 0; i < for_size; i++)
	{
		const SliceInfo& slice_info = filelist[i];
		if (slice_info.slice.empty())
			continue;
		if (!ReadSinglePng(val, slice_info.slice, c))
		{
			delete[] static_cast<unsigned char*>(val);
			return nullptr;
		}
		if (show_progress)
			SetProgress(static_cast<int>(std::round(100.0 * (i + 1) / for_size)), "NOT_SET");
		val_ptr = static_cast<unsigned char*>(val) + (i * m_x_size * m_y_size * (m_bits / 8));
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

bool PNGReader::ReadSinglePng(void* val, const std::wstring& filename, int c)
{
	// Open file stream
	FILE* fp = _wfopen(filename.c_str(), L"rb");
	if (!fp)
		return false;

	// Create libpng read structures
	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	png_infop info = png_create_info_struct(png);
	if (!png || !info)
	{
		fclose(fp);
		return false;
	}

	if (setjmp(png_jmpbuf(png)))
	{
		png_destroy_read_struct(&png, &info, nullptr);
		fclose(fp);
		return false;
	}

	png_init_io(png, fp);
	png_read_info(png, info);

	int width = png_get_image_width(png, info);
	int height = png_get_image_height(png, info);
	png_byte color_type = png_get_color_type(png, info);
	png_byte bit_depth = png_get_bit_depth(png, info);

	// Determine channel count
	int channels = 0;
	switch (color_type)
	{
	case PNG_COLOR_TYPE_GRAY:        channels = 1; break;
	case PNG_COLOR_TYPE_GRAY_ALPHA:  channels = 2; break;
	case PNG_COLOR_TYPE_RGB:         channels = 3; break;
	case PNG_COLOR_TYPE_RGB_ALPHA:   channels = 4; break;
	default:
		png_destroy_read_struct(&png, &info, nullptr);
		fclose(fp);
		return false;
	}

	if (c < 0 || c >= channels)
	{
		png_destroy_read_struct(&png, &info, nullptr);
		fclose(fp);
		return false;
	}

	png_read_update_info(png, info);

	// Allocate row pointers
	png_bytep* row_pointers = new png_bytep[height];
	for (int y = 0; y < height; y++)
		row_pointers[y] = new png_byte[png_get_rowbytes(png, info)];

	png_read_image(png, row_pointers);
	fclose(fp);
	png_destroy_read_struct(&png, &info, nullptr);

	// Allocate output buffer based on bit depth
	if (bit_depth == 8)
	{
		unsigned char* data = static_cast<unsigned char*>(val);
		for (int y = 0; y < height; y++)
		{
			png_bytep row = row_pointers[y];
			for (int x = 0; x < width; x++)
				data[y * width + x] = row[x * channels + c];
			delete[] row_pointers[y];
		}
	}
	else if (bit_depth == 16)
	{
		unsigned short* data = static_cast<unsigned short*>(val);
		for (int y = 0; y < height; y++)
		{
			png_bytep row = row_pointers[y];
			for (int x = 0; x < width; x++)
			{
				int offset = x * channels * 2 + c * 2;
				data[y * width + x] = (row[offset] << 8) | row[offset + 1];
			}
			delete[] row_pointers[y];
		}
	}
	else
	{
		// Unsupported bit depth
		for (int y = 0; y < height; y++)
			delete[] row_pointers[y];
		delete[] row_pointers;
		return false;
	}

	delete[] row_pointers;
	return true;
}

std::wstring PNGReader::GetCurDataName(int t, int c)
{
	if (t >= 0 && t < (int)m_4d_seq.size() &&
		!m_4d_seq[t].slices.empty())
		return m_4d_seq[t].slices[0].slice;
	return m_path_name;
}

std::wstring PNGReader::GetCurMaskName(int t, int c)
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

std::wstring PNGReader::GetCurLabelName(int t, int c)
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
