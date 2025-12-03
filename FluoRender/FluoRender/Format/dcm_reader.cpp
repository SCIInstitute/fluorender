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
#include <jpeglib.h>
#include <openjpeg.h>
#include <zlib.h>
#include <fstream>

namespace {
	class JPEG2000Decoder {
	public:
		static bool Decode(const std::vector<char>& compressed,
			std::vector<uint8_t>& output,
			int& width, int& height,
			int& channels, int& bits);

	private:
		struct MemoryStream {
			const uint8_t* data;
			size_t size;
			size_t offset;
		};

		static OPJ_SIZE_T Read(void* buffer, OPJ_SIZE_T size, void* user_data);
		static OPJ_OFF_T Skip(OPJ_OFF_T n, void* user_data);
		static OPJ_BOOL Seek(OPJ_OFF_T pos, void* user_data);
	};

	bool JPEG2000Decoder::Decode(const std::vector<char>& compressed,
		std::vector<uint8_t>& output,
		int& width, int& height,
		int& channels, int& bits)
	{
		if (compressed.size() < 2) return false;

		// Heuristic: detect JP2 vs J2K
		bool is_jp2 = (compressed.size() >= 12 &&
			static_cast<uint8_t>(compressed[4]) == 0x6A && // 'j'
			static_cast<uint8_t>(compressed[5]) == 0x50);  // 'P'

		opj_codec_t* codec = opj_create_decompress(is_jp2 ? OPJ_CODEC_JP2 : OPJ_CODEC_J2K);
		if (!codec) return false;

		opj_dparameters_t parameters;
		opj_set_default_decoder_parameters(&parameters);

		MemoryStream* mem_stream = new MemoryStream{
			reinterpret_cast<const uint8_t*>(compressed.data()),
			compressed.size(),
			0
		};

		opj_stream_t* stream = opj_stream_create(65536, true);
		if (!stream) {
			delete mem_stream;
			opj_destroy_codec(codec);
			return false;
		}

		opj_stream_set_user_data(stream, mem_stream, [](void* p) { delete static_cast<MemoryStream*>(p); });
		opj_stream_set_read_function(stream, Read);
		opj_stream_set_skip_function(stream, Skip);
		opj_stream_set_seek_function(stream, Seek);
		opj_stream_set_user_data_length(stream, static_cast<OPJ_UINT64>(compressed.size()));

		if (!opj_setup_decoder(codec, &parameters)) {
			opj_stream_destroy(stream);
			opj_destroy_codec(codec);
			return false;
		}

		opj_image_t* image = nullptr;
		if (!opj_read_header(stream, codec, &image)) {
			opj_stream_destroy(stream);
			opj_destroy_codec(codec);
			return false;
		}

		if (!opj_decode(codec, stream, image) || !opj_end_decompress(codec, stream)) {
			opj_stream_destroy(stream);
			opj_destroy_codec(codec);
			opj_image_destroy(image);
			return false;
		}

		opj_stream_destroy(stream);
		opj_destroy_codec(codec);

		if (!image || image->numcomps < 1) {
			opj_image_destroy(image);
			return false;
		}

		width = image->comps[0].w;
		height = image->comps[0].h;
		channels = image->numcomps;
		bits = image->comps[0].prec;

		size_t slice_size = width * height;
		output.resize(slice_size * channels);

		for (int c = 0; c < channels; ++c) {
			const opj_image_comp_t& comp = image->comps[c];
			bool is_signed = comp.sgnd != 0;
			int max_val = (1 << comp.prec) - 1;

			for (size_t i = 0; i < slice_size; ++i) {
				int val = comp.data[i];
				if (is_signed) val = std::max(val, 0);
				val = std::min(val, max_val);
				output[i * channels + c] = static_cast<uint8_t>(val);
			}
		}

		opj_image_destroy(image);
		return true;
	}

	OPJ_SIZE_T JPEG2000Decoder::Read(void* buffer, OPJ_SIZE_T size, void* user_data) {
		MemoryStream* stream = static_cast<MemoryStream*>(user_data);
		size_t remaining = stream->size - stream->offset;
		size_t to_read = std::min<size_t>(size, remaining);
		memcpy(buffer, stream->data + stream->offset, to_read);
		stream->offset += to_read;
		return to_read;
	}

	OPJ_OFF_T JPEG2000Decoder::Skip(OPJ_OFF_T n, void* user_data) {
		MemoryStream* stream = static_cast<MemoryStream*>(user_data);
		stream->offset = std::min(stream->offset + static_cast<size_t>(n), stream->size);
		return n;
	}

	OPJ_BOOL JPEG2000Decoder::Seek(OPJ_OFF_T pos, void* user_data) {
		MemoryStream* stream = static_cast<MemoryStream*>(user_data);
		if (pos < 0 || static_cast<size_t>(pos) > stream->size) return OPJ_FALSE;
		stream->offset = static_cast<size_t>(pos);
		return OPJ_TRUE;
	}
}

DCMReader::DCMReader() :
	BaseVolReader()
{
	m_valid_info = false;
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
	m_cur_time = -1;

	m_time_id = L"_T";

	m_bits = 8; //default bits per channel
	m_big_endian = false;
	m_signed = false;
	m_compression = DCM_UNCOMPRESSED;
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
	m_valid_info = false;

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

		m_size.z(static_cast<int>(m_4d_seq[m_cur_time].slices.size()));
	}
	else m_chan_num = 0;

	if (m_compression == DCM_UNSUPPORTED ||
		m_compression == DCM_JPEG_LOSSLESS ||
		m_size.any_le_zero() ||
		m_time_num == 0)
		return READER_FORMAT_ERROR;
	return READER_OK;
}

bool DCMReader::GetFileInfo(const std::wstring& filename)
{
	if (m_valid_info)
		return true;

	std::ifstream file(filename, std::ios::binary);
	if (!file.is_open())
		return false;

	file.seekg(132, std::ios::beg); // Skip preamble + "DICM"

	uint16_t group = 0, element = 0;
	char vr[2];
	std::string transfer_syntax_uid;
	m_compression = DCM_UNCOMPRESSED;
	m_tag_map.clear();

	auto ReadU16_LE = [&](std::ifstream& f) -> uint16_t {
		uint16_t val;
		f.read(reinterpret_cast<char*>(&val), 2);
		return val;
		};

	auto ReadU32_LE = [&](std::ifstream& f) -> uint32_t {
		uint32_t val;
		f.read(reinterpret_cast<char*>(&val), 4);
		return val;
		};

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

	auto trim_uid = [](std::string uid) -> std::string {
		uid.erase(std::find_if(uid.rbegin(), uid.rend(), [](unsigned char ch) {
			return !std::isspace(ch) && ch != '\0';
			}).base(), uid.end());
		return uid;
		};

	// Safe metadata extraction
	auto get_u16 = [&](uint32_t tag) -> uint16_t {
		auto it = m_tag_map.find(tag);
		if (it == m_tag_map.end() || it->second.size() < 2) return 0;
		uint16_t val = *reinterpret_cast<uint16_t*>(it->second.data());
		if (m_big_endian)
			val = (val >> 8) | (val << 8);
		return val;
		};

	std::streampos pixel_data_pos;
	while (file.read(reinterpret_cast<char*>(&group), 2) &&
		file.read(reinterpret_cast<char*>(&element), 2))
	{
		uint32_t tag = (group << 16) | element;
		uint32_t length = 0;

		if (tag == 0x7FE00010)
		{
			// Save position just before reading VR/length/value
			pixel_data_pos = file.tellg();
			break; // Exit loop to process later
		}
		else if (tag == 0xFFFEE0DD)
		{
			// Sequence Delimitation Item — no VR, no value
			if (!file.read(reinterpret_cast<char*>(&length), 4)) break;

			if (length != 0)
			{
				// Technically should be zero, but skip anyway
				file.seekg(length, std::ios::cur);
			}

			continue; // Skip tag_map insertion
		}
		else
		{
			if (!file.read(vr, 2)) break;

			bool long_length = (vr[0] == 'O' && vr[1] == 'B') || (vr[0] == 'O' && vr[1] == 'W') ||
				(vr[0] == 'U' && vr[1] == 'N') || (vr[0] == 'S' && vr[1] == 'Q');

			if (long_length) {
				file.ignore(2);
				length = (group == 0x0002) ? ReadU32_LE(file) : ReadU32(file);
			}
			else {
				length = (group == 0x0002) ? ReadU16_LE(file) : ReadU16(file);
			}

			std::vector<char> value;

			if (vr[0] == 'S' && vr[1] == 'Q' && length == 0xFFFFFFFF)
			{
				// Sequence with undefined length
				while (true)
				{
					SequenceItem item = ParseSequenceItem(file);
					if (!item.is_valid)
						break;

					if (item.is_delim)
						break;

					if (item.is_empty)
						continue;

					value.insert(value.end(), item.data.begin(), item.data.end());
				}
			}
			else
			{
				value.resize(length);
				if (!file.read(value.data(), length)) break;
			}

			m_tag_map[tag] = std::move(value);
		}

	}

	auto it_spc = m_tag_map.find(0x00020010);
	if (it_spc != m_tag_map.end() && !it_spc->second.empty()) {
		transfer_syntax_uid = trim_uid(std::string(it_spc->second.data(), it_spc->second.size()));
		DetectCompression(transfer_syntax_uid);
	}

	// Optional: pixel spacing
	it_spc = m_tag_map.find(0x00280030);
	if (it_spc != m_tag_map.end() && !it_spc->second.empty())
	{
		std::string spacing_str(it_spc->second.data(), it_spc->second.size());
		double xspc, yspc, zspc;
		if (sscanf(spacing_str.c_str(), "%lf\\%lf", &xspc, &yspc) == 2)
		{
			zspc = std::max(xspc, yspc);
			m_spacing = fluo::Vector(xspc, yspc, zspc);
			m_valid_spc = true;
		}
	}

	if (pixel_data_pos != std::streampos(-1))
	{
		// Try to get image size from tags
		m_size.y(get_u16(0x00280010));
		m_size.x(get_u16(0x00280011));
		m_bits = get_u16(0x00280100);
		m_chan_num = get_u16(0x00280002);
		m_signed = (get_u16(0x00280103) == 1);

		if (m_size.intx() > 0 && m_size.inty() > 0 && m_bits != 0)
		{
			m_valid_info = true;
			return true;
		}

		// Now process Pixel Data as usual
		file.seekg(pixel_data_pos);
		// Fallback: decompress pixel data to infer size
		bool long_length = (vr[0] == 'O' && vr[1] == 'B') || (vr[0] == 'O' && vr[1] == 'W') ||
			(vr[0] == 'U' && vr[1] == 'N') || (vr[0] == 'S' && vr[1] == 'Q');

		uint32_t length = 0;
		if (long_length) {
			file.ignore(2);
			length = (group == 0x0002) ? ReadU32_LE(file) : ReadU32(file);
		}
		else {
			length = (group == 0x0002) ? ReadU16_LE(file) : ReadU16(file);
		}

		std::vector<char> pixel_data;
		if (length == 0xFFFFFFFF)
		{
			while (true)
			{
				SequenceItem item = ParseSequenceItem(file);
				if (!item.is_valid)
					break;

				if (item.is_delim)
					break;

				if (item.is_empty)
					continue;

				pixel_data.insert(pixel_data.end(), item.data.begin(), item.data.end());
			}
		}
		else
		{
			pixel_data.resize(length);
			if (!file.read(pixel_data.data(), length)) return false;
		}

		std::vector<uint8_t> decompressed;
		if (!Decompress(pixel_data, decompressed, 0)) return false;

		return (m_size.intx() > 0 && m_size.inty() > 0 && m_bits != 0);
	}

	m_valid_info = true;

	return false;
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

	unsigned long long total_size = m_size.get_size_xyz();
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
		val_ptr += m_size.get_size_xy() * (m_bits / 8);
	}

	//write to nrrd
	if (eight_bit)
		nrrdWrap_va(nrrdout, (uint8_t*)val, nrrdTypeUChar,
			3, (size_t)m_size.intx(), (size_t)m_size.inty(), (size_t)m_size.intz());
	else
		nrrdWrap_va(nrrdout, (uint16_t*)val, nrrdTypeUShort,
			3, (size_t)m_size.intx(), (size_t)m_size.inty(), (size_t)m_size.intz());
	nrrdAxisInfoSet_va(nrrdout, nrrdAxisInfoSpacing, m_spacing.x(), m_spacing.y(), m_spacing.z());
	auto max_size = m_size * m_spacing;
	nrrdAxisInfoSet_va(nrrdout, nrrdAxisInfoMax, max_size.x(),
		max_size.y(), max_size.z());
	nrrdAxisInfoSet_va(nrrdout, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
	nrrdAxisInfoSet_va(nrrdout, nrrdAxisInfoSize, (size_t)m_size.intx(),
		(size_t)m_size.inty(), (size_t)m_size.intz());

	if (!eight_bit)
	{
		if (get_max)
		{
			double value;
			unsigned long long totali = m_size.get_size_xyz();
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

	uint16_t group = 0, element = 0;
	char vr[2];

	auto ReadU16_LE = [&](std::ifstream& f) -> uint16_t {
		uint16_t val;
		f.read(reinterpret_cast<char*>(&val), 2);
		return val;
		};

	auto ReadU32_LE = [&](std::ifstream& f) -> uint32_t {
		uint32_t val;
		f.read(reinterpret_cast<char*>(&val), 4);
		return val;
		};

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
		uint32_t length = 0;

		if (tag == 0xFFFEE0DD)
		{
			// Sequence Delimitation Item — no VR, no value
			if (!file.read(reinterpret_cast<char*>(&length), 4)) break;

			if (length != 0)
			{
				// Technically should be zero, but skip anyway
				file.seekg(length, std::ios::cur);
			}

			continue; // Skip tag_map insertion
		}

		if (!file.read(vr, 2)) break;

		bool long_length = (vr[0] == 'O' && vr[1] == 'B') || (vr[0] == 'O' && vr[1] == 'W') ||
			(vr[0] == 'U' && vr[1] == 'N') || (vr[0] == 'S' && vr[1] == 'Q');

		if (long_length) {
			file.ignore(2);
			length = (group == 0x0002) ? ReadU32_LE(file) : ReadU32(file);
		}
		else {
			length = (group == 0x0002) ? ReadU16_LE(file) : ReadU16(file);
		}

		if (tag == 0x7FE00010) {
			std::vector<char> pixel_data;

			if (length == 0xFFFFFFFF) {
				// Encapsulated Pixel Data (fragments)
				while (true) {
					SequenceItem item = ParseSequenceItem(file);
					if (!item.is_valid)
						break;

					if (item.is_delim)
						break;

					if (item.is_empty)
						continue;

					pixel_data.insert(pixel_data.end(), item.data.begin(), item.data.end());
				}
			}
			else {
				// Explicit length pixel data
				pixel_data.resize(length);
				if (!file.read(pixel_data.data(), length)) return false;
			}

			std::vector<uint8_t> decompressed;
			if (!Decompress(pixel_data, decompressed, c)) return false;

			size_t slice_size = static_cast<size_t>(m_size.get_size_xy());
			if (m_bits == 8) {
				const uint8_t* full_data = decompressed.data();
				uint8_t* channel_data = static_cast<uint8_t*>(val);

				for (size_t i = 0; i < slice_size; ++i)
					channel_data[i] = full_data[i * m_chan_num + c];
			}
			else if (m_bits == 16) {
				uint16_t* channel_data = static_cast<uint16_t*>(val);

				if (m_signed) {
					const int16_t* full_data = reinterpret_cast<const int16_t*>(decompressed.data());
					for (size_t i = 0; i < slice_size; ++i) {
						int16_t raw = full_data[i * m_chan_num + c];
						if (m_big_endian)
							raw = (raw >> 8) | (raw << 8);
						channel_data[i] = static_cast<uint16_t>(raw < 0 ? 0 : raw);
					}
				}
				else {
					const uint16_t* full_data = reinterpret_cast<const uint16_t*>(decompressed.data());
					for (size_t i = 0; i < slice_size; ++i) {
						uint16_t raw = full_data[i * m_chan_num + c];
						if (m_big_endian)
							raw = (raw >> 8) | (raw << 8);
						channel_data[i] = raw;
					}
				}
			}
			else {
				return false; // Unsupported bit depth
			}

			break;
		}
		else {
			if (vr[0] == 'S' && vr[1] == 'Q' && length == 0xFFFFFFFF)
			{
				// Sequence with undefined length
				while (true)
				{
					SequenceItem item = ParseSequenceItem(file);
					if (!item.is_valid)
						break;

					if (item.is_delim)
						break;

					if (item.is_empty)
						continue;
				}
			}
			else
			{
				std::vector<char> value(length);
				if (!file.read(value.data(), length)) break;
			}
		}
	}

	return val != nullptr;
}

bool DCMReader::CleanPixelData(std::vector<char>& pixel_data)
{
	if (pixel_data.empty()) return false;

	auto FindMarker = [](const std::vector<char>& data, uint8_t byte1, uint8_t byte2) -> size_t {
		for (size_t i = 0; i + 1 < data.size(); ++i) {
			if ((uint8_t)data[i] == byte1 && (uint8_t)data[i + 1] == byte2)
				return i;
		}
		return std::string::npos;
		};

	size_t start = std::string::npos;
	size_t end = std::string::npos;

	if (m_compression == DCM_JPEG_BASELINE) {
		start = FindMarker(pixel_data, 0xFF, 0xD8); // SOI
		end = FindMarker(pixel_data, 0xFF, 0xD9); // EOI
	}
	else if (m_compression == DCM_JPEG2000) {
		start = FindMarker(pixel_data, 0xFF, 0x4F); // SOC
		end = FindMarker(pixel_data, 0xFF, 0xD9); // EOC
	}

	if (start != std::string::npos && end != std::string::npos && end > start) {
		end += 2; // Include marker
		std::vector<char> cleaned(pixel_data.begin() + start, pixel_data.begin() + end);
		pixel_data.swap(cleaned);
		return true;
	}

	// If no markers found, fallback to original logic for trailing 0x00 after EOI
	size_t size = pixel_data.size();
	if (size >= 3) {
		unsigned char last = static_cast<unsigned char>(pixel_data[size - 1]);
		unsigned char second_last = static_cast<unsigned char>(pixel_data[size - 2]);
		if (second_last == 0xFF && last == 0xD9) {
			// Valid EOI marker
		}
		else if (size >= 4) {
			unsigned char third_last = static_cast<unsigned char>(pixel_data[size - 3]);
			if (third_last == 0xFF && second_last == 0xD9 && last == 0x00) {
				pixel_data.resize(size - 1); // Strip trailing 0x00
			}
		}
	}

	return true;
}

bool DCMReader::Decompress(std::vector<char>& pixel_data, std::vector<uint8_t>& decompressed, int c)
{
	if (pixel_data.empty())
		return false;

	if (m_compression == DCM_JPEG_BASELINE || m_compression == DCM_JPEG2000) {
		if (!CleanPixelData(pixel_data))
			return false;
	}

	int width = m_size.intx();
	int height = m_size.inty();
	int channels = m_chan_num;
	int bits = m_bits;

	if (m_compression == DCM_UNCOMPRESSED) {
		if (width == 0 || height == 0 || channels == 0 || bits == 0)
			return false;

		decompressed.assign(pixel_data.begin(), pixel_data.end());
	}
	else if (m_compression == DCM_DEFLATE) {
		if (width == 0 || height == 0 || channels == 0 || bits == 0)
			return false;

		uLongf out_len = width * height * channels * (bits / 8);
		decompressed.resize(out_len);

		int ret = uncompress(decompressed.data(), &out_len,
			reinterpret_cast<const Bytef*>(pixel_data.data()), pixel_data.size());

		if (ret != Z_OK) return false;
	}
	else if (m_compression == DCM_JPEG_BASELINE) {
		jpeg_decompress_struct cinfo;
		jpeg_error_mgr jerr;
		cinfo.err = jpeg_std_error(&jerr);
		jpeg_create_decompress(&cinfo);

		jpeg_mem_src(&cinfo, reinterpret_cast<unsigned char*>(pixel_data.data()), pixel_data.size());
		jpeg_read_header(&cinfo, TRUE);
		jpeg_start_decompress(&cinfo);

		width = cinfo.output_width;
		height = cinfo.output_height;
		channels = cinfo.output_components;
		//bits = 8;

		size_t row_stride = width * channels;
		decompressed.resize(height * row_stride);

		while (cinfo.output_scanline < height) {
			uint8_t* rowptr = decompressed.data() + cinfo.output_scanline * row_stride;
			jpeg_read_scanlines(&cinfo, &rowptr, 1);
		}

		jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);
	}
	else if (m_compression == DCM_JPEG2000) {
		std::vector<uint8_t> decoded;
		if (!JPEG2000Decoder::Decode(pixel_data, decoded, width, height, channels, bits))
			return false;

		size_t slice_size = width * height;
		size_t expected_bytes_per_sample = (bits > 8) ? 2 : 1;
		size_t expected_total_size = slice_size * channels * expected_bytes_per_sample;

		// If decoder returned only 8-bit data, override bits
		bool decoder_is_8bit = (decoded.size() == slice_size * channels);
		if (decoder_is_8bit && bits > 8) {
			// Promote 8-bit samples to 16-bit
			decompressed.resize(slice_size * 2); // 2 bytes per pixel

			for (size_t i = 0; i < slice_size; ++i) {
				uint16_t promoted = static_cast<uint16_t>(decoded[i * channels + c]);
				decompressed[i * 2] = static_cast<uint8_t>(promoted & 0xFF);
				decompressed[i * 2 + 1] = static_cast<uint8_t>((promoted >> 8) & 0xFF);
			}
		}
		else if (bits == 12 || bits == 16) {
			// Assume decoder returned 2 bytes per sample
			decompressed.resize(slice_size * 2);

			for (size_t i = 0; i < slice_size; ++i) {
				uint16_t raw = static_cast<uint16_t>(
					decoded[(i * channels + c) * 2] |
					(decoded[(i * channels + c) * 2 + 1] << 8)
					);
				uint16_t promoted = (bits == 12) ? (raw & 0x0FFF) : raw;
				decompressed[i * 2] = static_cast<uint8_t>(promoted & 0xFF);
				decompressed[i * 2 + 1] = static_cast<uint8_t>((promoted >> 8) & 0xFF);
			}
		}
		else if (bits == 8) {
			// True 8-bit image
			decompressed.resize(slice_size);
			for (size_t i = 0; i < slice_size; ++i)
				decompressed[i] = decoded[i * channels + c];
		}
		else {
			return false; // Unsupported bit depth
		}
	}
	else {
		return false;
	}

	if (m_size.intx() == 0) m_size.x(width);
	if (m_size.inty() == 0) m_size.y(height);
	if (m_chan_num == 0) m_chan_num = channels;
	if (m_bits == 0) m_bits = bits;

	return true;
}

DCMReader::SequenceItem DCMReader::ParseSequenceItem(std::ifstream& file)
{
	SequenceItem result;
	uint16_t group = 0, element = 0;
	std::streampos tag_pos = file.tellg();

	if (!file.read(reinterpret_cast<char*>(&group), 2) ||
		!file.read(reinterpret_cast<char*>(&element), 2))
	{
		result.is_valid = false;
		return result;
	}

	uint32_t item_tag = (group << 16) | element;

	if (item_tag == 0xFFFEE0DD)
	{
		uint32_t delim_len = 0;
		if (!file.read(reinterpret_cast<char*>(&delim_len), 4))
			result.is_valid = false;
		result.is_delim = true;
		return result;
	}

	if (item_tag != 0xFFFEE000)
	{
		file.seekg(tag_pos);
		result.is_valid = false;
		return result;
	}

	uint32_t item_length = 0;
	if (!file.read(reinterpret_cast<char*>(&item_length), 4))
	{
		result.is_valid = false;
		return result;
	}

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

	auto ParseElement = [&](std::ifstream& f, SequenceItem& parent) -> bool {
		while (f.read(reinterpret_cast<char*>(&group), 2) &&
			f.read(reinterpret_cast<char*>(&element), 2))
		{
			uint32_t tag = (group << 16) | element;

			if (tag == 0xFFFEE00D) {
				uint32_t delim_len = 0;
				if (!f.read(reinterpret_cast<char*>(&delim_len), 4))
					return false;
				return true;
			}

			char vr[2];
			if (!f.read(vr, 2)) return false;

			bool long_length = (vr[0] == 'O' && vr[1] == 'B') ||
				(vr[0] == 'O' && vr[1] == 'W') ||
				(vr[0] == 'U' && vr[1] == 'N') ||
				(vr[0] == 'S' && vr[1] == 'Q');

			uint32_t value_length = 0;
			if (long_length) {
				f.ignore(2);
				value_length = ReadU32(f);
			}
			else {
				value_length = ReadU16(f);
			}

			if (vr[0] == 'S' && vr[1] == 'Q') {
				// Nested sequence
				std::streampos seq_start = f.tellg();
				if (value_length == 0xFFFFFFFF) {
					while (true) {
						auto nested = ParseSequenceItem(f);
						if (!nested.is_valid || nested.is_delim)
							break;
						parent.children.push_back(nested);
					}
				}
				else {
					std::streampos seq_end = seq_start + static_cast<std::streamoff>(value_length);
					while (f.tellg() < seq_end) {
						auto nested = ParseSequenceItem(f);
						if (!nested.is_valid)
							break;
						parent.children.push_back(nested);
					}
				}
			}
			else {
				std::vector<char> value(value_length);
				if (!f.read(value.data(), value_length))
					return false;
				parent.data.insert(parent.data.end(), value.begin(), value.end());
			}
		}
		return true;
		};

	if (item_length == 0)
	{
		result.is_empty = true;
		return result;
	}
	else if (item_length == 0xFFFFFFFF)
	{
		if (!ParseElement(file, result))
			result.is_valid = false;
	}
	else
	{
		result.data.resize(item_length);
		if (!file.read(result.data.data(), item_length))
			result.is_valid = false;
	}

	result.is_valid = true;
	return result;
}

void DCMReader::DetectCompression(const std::string& uid)
{
	if (uid.find("1.2.840.10008.1.2.2") != std::string::npos)
		m_big_endian = true;

	if (uid.find("1.2.840.10008.1.2.4.50") != std::string::npos)
		m_compression = DCM_JPEG_BASELINE;
	else if (uid.find("1.2.840.10008.1.2.4.70") != std::string::npos)
		m_compression = DCM_JPEG_LOSSLESS;
	else if (uid.find("1.2.840.10008.1.2.1.99") != std::string::npos)
		m_compression = DCM_DEFLATE;
	else if (uid.find("1.2.840.10008.1.2.4.91") != std::string::npos)
		m_compression = DCM_JPEG2000;
	else if (uid == "1.2.840.10008.1.2" ||
		uid == "1.2.840.10008.1.2.1" ||
		uid == "1.2.840.10008.1.2.2")
		m_compression = DCM_UNCOMPRESSED;
	else
		m_compression = DCM_UNSUPPORTED;
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
