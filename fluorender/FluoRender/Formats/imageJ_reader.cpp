/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2014 Scientific Computing and Imaging Institute,
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
#include "imageJ_reader.h"
#include "../compatibility.h"
#include <wx/stdpaths.h>

ImageJReader::ImageJReader()
{
	m_resize_type = 0;
	m_resample_type = 0;
	m_alignment = 0;

	m_slice_seq = false;
	m_time_num = 0;
	m_cur_time = -1;
	m_chan_num = 0;
	m_slice_num = 0;
	m_x_size = 0;
	m_y_size = 0;
	m_eight_bit = true;

	m_valid_spc = false;
	m_xspc = 1.0;
	m_yspc = 1.0;
	m_zspc = 1.0;

	m_max_value = 0.0;
	m_scalar_scale = 1.0;

	m_batch = false;
	m_cur_batch = -1;

	m_time_id = L"_T";

	current_page_ = current_offset_ = 0;
	swap_ = false;
	isBig_ = false;
	isHyperstack_ = false;

    //Geting absolute path to class file.
	wxString exePath = wxStandardPaths::Get().GetExecutablePath();
	exePath = wxPathOnly(exePath);
	string imageJPath = exePath + GETSLASH() + "Java_Code" + GETSLASH() + "ImageJ_Reader";

	//Java code to get the number of depth images.
	m_pJVMInstance = JVMInitializer::getInstance();
	printf("%s", imageJPath.c_str());
	fflush(stdout);

	m_imageJ_cls = m_pJVMInstance->m_pEnv->FindClass("ImageJ_Reader");
	if (m_imageJ_cls == nullptr) {
		cerr << "ERROR: class not found !";
	}
}

ImageJReader::~ImageJReader()
{
	//if (tiff_stream.is_open())
	//	tiff_stream.close();
}

void ImageJReader::SetFile(string &file)
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

void ImageJReader::SetFile(wstring &file)
{
	m_path_name = file;
	m_id_string = m_path_name;	
}

int ImageJReader::Preprocess()
{
	int i;

	m_4d_seq.clear();
	isHyperstack_ = false;
	isHsTimeSeq_ = false;
	imagej_raw_ = false;
	imagej_raw_possible_ = false;
	m_b_page_num = false;
	m_ull_page_num = 0;
	InvalidatePageInfo();

	//separate path and name
	wstring path, name;
	if (!SEP_PATH_NAME(m_path_name, path, name))
		return READER_OPEN_FAIL;
	m_data_name = name;	

	// ImageJ code here..................
	if (m_imageJ_cls == nullptr) {
		cerr << "ERROR: class not found !";
	}
	else {
		// getting the image metadata.
		jmethodID method_handle = m_pJVMInstance->m_pEnv->GetStaticMethodID(m_imageJ_cls, "getMetaData", "([Ljava/lang/String;)[I");
		if (method_handle == nullptr)
			cerr << "ERROR: method void getDepth() not found !" << endl;
		else {
			// This part goes in setFile.
			jobjectArray arr = m_pJVMInstance->m_pEnv->NewObjectArray(2,      // constructs java array of 2
				m_pJVMInstance->m_pEnv->FindClass("java/lang/String"),    // Strings
				m_pJVMInstance->m_pEnv->NewStringUTF("str"));   // each initialized with value "str"
			
			char* cstr = new char[m_path_name.length() + 1];
			sprintf(cstr, "%ws", m_path_name.c_str());

			m_pJVMInstance->m_pEnv->SetObjectArrayElement(arr, 0, m_pJVMInstance->m_pEnv->NewStringUTF(cstr));  // change an element
			m_pJVMInstance->m_pEnv->SetObjectArrayElement(arr, 1, m_pJVMInstance->m_pEnv->NewStringUTF("4D_1ch.lsm"));  // change an element
			//jint depth = (jint)(m_pJVMInstance->m_pEnv->CallStaticIntMethod(imageJ_cls, mid, arr));   // call the method with the arr as argument.
			//m_pJVMInstance->m_pEnv->DeleteLocalRef(arr);     // release the object

			jintArray val = (jintArray)(m_pJVMInstance->m_pEnv->CallStaticObjectMethod(m_imageJ_cls, method_handle, arr));   // call the method with the arr as argument.					
			jsize len = m_pJVMInstance->m_pEnv->GetArrayLength(val);
			jint* body = m_pJVMInstance->m_pEnv->GetIntArrayElements(val, 0);

			//Checking if the right format was loaded.
			if (len == 1) {
				int test = *body;
				if (test == 1)
					return READER_FORMAT_ERROR;
				else if (test == 2)
					return READER_OPEN_FAIL;
				else
					return READER_EMPTY_DATA; //This is for unkown exception on java side.
			}

			for (int i = 0; i < len; i++) {
				int test = *(body + i);
				switch (i)
				{
				case 0:
					break;
				case 1:
					m_x_size = test;
					break;
				case 2:
					m_y_size = test;
					break;
				case 3:
					m_slice_num = test;
					break;
				case 4:
					m_chan_num = test;
					break;
				case 5:
					m_time_num = test;
					break;				
				case 6:
					int m_bits_per_pixel = test;
					if (m_bits_per_pixel == 1)
						m_eight_bit = true;
					else
						m_eight_bit = false;
					break;
				// TODO: What is m_max_value.
				}
			}
			m_pJVMInstance->m_pEnv->DeleteLocalRef(arr);     // release the object
		}
	}
	m_cur_time = 0;	
	
	return READER_OK;
}

uint64_t ImageJReader::GetTiffField(const uint64_t in_tag)
{
	if (!m_page_info.b_valid)
		ReadTiffFields();

	if (m_page_info.b_valid)
	{
		switch (in_tag)
		{
		case kSubFileTypeTag:
			if (m_page_info.b_sub_file_type)
				return m_page_info.ul_sub_file_type;
			break;
		case kImageWidthTag:
			if (m_page_info.b_image_width)
				return m_page_info.ul_image_width;
			break;
		case kImageLengthTag:
			if (m_page_info.b_image_length)
				return m_page_info.ul_image_length;
			break;
		case kBitsPerSampleTag:
			if (m_page_info.b_bits_per_sample)
				return m_page_info.us_bits_per_sample;
			break;
		case kCompressionTag:
			if (m_page_info.b_compression)
				return m_page_info.us_compression;
			break;
		case kPredictionTag:
			if (m_page_info.b_prediction)
				return m_page_info.us_prediction;
			break;
		case kPlanarConfigurationTag:
			if (m_page_info.b_planar_config)
				return m_page_info.us_planar_config;
			break;
		case kSamplesPerPixelTag:
			if (m_page_info.b_samples_per_pixel)
				return m_page_info.us_samples_per_pixel;
			break;
		case kRowsPerStripTag:
			if (m_page_info.b_rows_per_strip)
				return m_page_info.ul_rows_per_strip;
			break;
		case kTileWidthTag:
			if (m_page_info.b_tile_width)
				return m_page_info.ul_tile_width;
			break;
		case kTileLengthTag:
			if (m_page_info.b_tile_length)
				return m_page_info.ul_tile_length;
			break;
		}
	}

	return 0;
}

bool ImageJReader::TagInInfo(uint16_t tag)
{
	if (tag == kSubFileTypeTag ||
		tag == kImageWidthTag ||
		tag == kImageLengthTag ||
		tag == kBitsPerSampleTag ||
		tag == kCompressionTag ||
		tag == kPredictionTag ||
		tag == kPlanarConfigurationTag ||
		tag == kImageDescriptionTag ||
		tag == kStripOffsetsTag ||
		tag == kSamplesPerPixelTag ||
		tag == kRowsPerStripTag ||
		tag == kStripBytesCountsTag ||
		tag == kXResolutionTag ||
		tag == kYResolutionTag ||
		tag == kTileWidthTag ||
		tag == kTileLengthTag ||
		tag == kTileOffsetsTag ||
		tag == kTileBytesCountsTag)
		return true;
	else
		return false;
}

void ImageJReader::SetPageInfo(uint16_t tag, uint64_t answer)
{
	//save info
	switch (tag)
	{
	case kSubFileTypeTag:
		m_page_info.b_sub_file_type = true;
		m_page_info.ul_sub_file_type = static_cast<unsigned long>(answer);
		break;
	case kImageWidthTag:
		m_page_info.b_image_width = true;
		if (answer)
			m_page_info.ul_image_width = static_cast<unsigned long>(answer);
		break;
	case kImageLengthTag:
		m_page_info.b_image_length = true;
		if (answer)
			m_page_info.ul_image_length = static_cast<unsigned long>(answer);
		break;
	case kBitsPerSampleTag:
		m_page_info.b_bits_per_sample = true;
		if (answer)
			m_page_info.us_bits_per_sample = static_cast<unsigned short>(answer);
		break;
	case kCompressionTag:
		m_page_info.b_compression = true;
		m_page_info.us_compression = static_cast<unsigned short>(answer);
		break;
	case kPredictionTag:
		m_page_info.b_prediction = true;
		m_page_info.us_prediction = static_cast<unsigned short>(answer);
		break;
	case kPlanarConfigurationTag:
		m_page_info.b_planar_config = true;
		m_page_info.us_planar_config = static_cast<unsigned short>(answer);
		break;
	case kSamplesPerPixelTag:
		m_page_info.b_samples_per_pixel = true;
		if (answer)
			m_page_info.us_samples_per_pixel = static_cast<unsigned short>(answer);
		break;
	case kRowsPerStripTag:
		m_page_info.b_rows_per_strip = true;
		if (answer)
			m_page_info.ul_rows_per_strip = static_cast<unsigned long>(answer);
		break;
	case kXResolutionTag:
		m_page_info.b_x_resolution = true;
		memcpy(&(m_page_info.d_x_resolution), &answer, sizeof(double));
		break;
	case kYResolutionTag:
		m_page_info.b_y_resolution = true;
		memcpy(&(m_page_info.d_y_resolution), &answer, sizeof(double));
		break;
	case kTileWidthTag:
		m_page_info.b_tile_width = true;
		if (answer)
			m_page_info.ul_tile_width = static_cast<unsigned long>(answer);
		break;
	case kTileLengthTag:
		m_page_info.b_tile_length = true;
		if (answer)
			m_page_info.ul_tile_length = static_cast<unsigned long>(answer);
		break;
	case kStripOffsetsTag:
		m_page_info.b_strip_offsets = true;
		m_page_info.ull_strip_offsets.resize(1);
		m_page_info.ull_strip_offsets[0] = static_cast<unsigned long long>(answer);
		break;
	case kStripBytesCountsTag:
		m_page_info.b_strip_byte_counts = true;
		m_page_info.ull_strip_byte_counts.resize(1);
		m_page_info.ull_strip_byte_counts[0] = static_cast<unsigned long long>(answer);
		break;
	case kTileOffsetsTag:
		m_page_info.b_tile_offsets = true;
		m_page_info.ull_tile_offsets.resize(1);
		m_page_info.ull_tile_offsets[0] = static_cast<unsigned long long>(answer);
		break;
	case kTileBytesCountsTag:
		m_page_info.b_tile_byte_counts = true;
		m_page_info.ull_tile_byte_counts.resize(1);
		m_page_info.ull_tile_byte_counts[0] = static_cast<unsigned long long>(answer);
		break;
	}
}

void ImageJReader::SetPageInfoVector(uint16_t tag, uint16_t type, uint64_t cnt, void* data)
{
	vector<unsigned long long>* v = 0;
	if (tag == kStripOffsetsTag)
	{
		m_page_info.b_strip_offsets = true;
		v = &(m_page_info.ull_strip_offsets);
	}
	else if (tag == kStripBytesCountsTag)
	{
		m_page_info.b_strip_byte_counts = true;
		v = &(m_page_info.ull_strip_byte_counts);
	}
	else if (tag == kTileOffsetsTag)
	{
		m_page_info.b_tile_offsets = true;
		v = &(m_page_info.ull_tile_offsets);
	}
	else if (tag == kTileBytesCountsTag)
	{
		m_page_info.b_tile_byte_counts = true;
		v = &(m_page_info.ull_tile_byte_counts);
	}
	else if (tag == kBitsPerSampleTag)
	{
		if (type == kShort)
		{
			uint16_t *data2 = (uint16*)data;
			m_page_info.b_bits_per_sample = true;
			m_page_info.us_bits_per_sample =
				swap_ ? SwapShort(data2[0]) : data2[0];
		}
		return;
	}

	v->clear();
	v->reserve(cnt);
	unsigned long long value;
	if (type == kShort)
	{
		uint16_t *data2 = (uint16_t*)data;
		for (uint64_t i = 0; i < cnt; ++i)
		{
			value = static_cast<unsigned long long>(
				swap_ ? SwapShort(data2[i]) : data2[i]);
			v->push_back(value);
		}
	}
	else if (type == kLong)
	{
		uint32_t *data2 = (uint32_t*)data;
		for (uint64_t i = 0; i < cnt; ++i)
		{
			value = static_cast<unsigned long long>(
				swap_ ? SwapWord(data2[i]) : data2[i]);
			v->push_back(value);
		}
	}
	else if (type == kLong8)
	{
		uint64_t *data2 = (uint64_t*)data;
		for (uint64_t i = 0; i < cnt; ++i)
		{
			value = static_cast<unsigned long long>(
				swap_ ? SwapLong(data2[i]) : data2[i]);
			v->push_back(value);
		}
	}
}

void ImageJReader::ReadTiffFields()
{
	if (m_page_info.b_valid)
		return;

	if (!tiff_stream.is_open())
		throw std::runtime_error("TIFF File not open for reading.");
	//go to the current IFD block/page
	tiff_stream.seekg(current_offset_, tiff_stream.beg);
	uint64_t num_entries = 0;
	//how many entries are there?
	if (isBig_)
	{
		tiff_stream.read((char*)&num_entries, sizeof(uint64_t));
		if (swap_) num_entries = SwapLong(num_entries);
	}
	else
	{
		uint16_t temp;
		tiff_stream.read((char*)&temp, sizeof(uint16_t));
		if (swap_) temp = SwapShort(temp);
		num_entries = static_cast<uint64_t>(temp);
	}
	char start_off = 2, multiplier = 12;
	if (isBig_) { start_off = 8; multiplier = 20; }

	//go through all of the entries to find the one we want
	for (size_t i = 0; i < num_entries; i++)
	{
		tiff_stream.seekg(current_offset_ + start_off + multiplier*i, tiff_stream.beg);
		uint16_t tag = 0;
		//get the tag of entry
		tiff_stream.read((char*)&tag, sizeof(uint16_t));
		if (swap_) tag = SwapShort(tag);
		// This is the tag we use, grab and go.
		if (TagInInfo(tag))
		{
			//find the type
			tiff_stream.seekg(current_offset_ + start_off + multiplier*i + 2, tiff_stream.beg);
			uint16_t type = 0;
			tiff_stream.read((char*)&type, sizeof(uint16_t));
			if (swap_) type = SwapShort(type);
			//if (!buf && size == kType)
			//{
			//	//number of
			//	//return static_cast<uint64_t>(type);
			//}
			//find the count
			tiff_stream.seekg(current_offset_ + start_off + multiplier*i + 4, tiff_stream.beg);
			uint64_t cnt = 0;
			if (isBig_)
			{
				tiff_stream.read((char*)&cnt, sizeof(uint64_t));
				if (swap_) cnt = SwapLong(cnt);
			}
			else
			{
				uint32_t tmp = 0;
				tiff_stream.read((char*)&tmp, sizeof(uint32_t));
				if (swap_) tmp = SwapWord(tmp);
				cnt = static_cast<uint64_t>(tmp);
			}

			if (tag == kStripOffsetsTag)
			{
				m_page_info.b_strip_num = true;
				m_page_info.ul_strip_num = static_cast<unsigned long>(cnt);
			}
			else if (tag == kTileOffsetsTag)
			{
				m_page_info.b_tile_num = true;
				m_page_info.ul_tile_num = static_cast<unsigned long>(cnt);
				m_page_info.b_use_tiles = true;
			}

			//now get the value (different for different types.)
			char off2 = isBig_ ? 12 : 8;
			uint32_t addr32;
			uint64_t addr64 = current_offset_ + start_off + multiplier*i + off2;
			//if (size == kValueAddress) return addr;
			tiff_stream.seekg(addr64, tiff_stream.beg);

			uint64_t answer = 0;
			char* buf = 0;
			if (type == kByte)
			{
				uint8_t value = 0;
				tiff_stream.read((char*)&value, sizeof(uint8_t));
				answer = static_cast<uint64_t>(value);
			}
			else if (type == kASCII)
			{
				if (cnt > 1)
				{
					if (isBig_)
					{
						tiff_stream.read((char*)&addr64, sizeof(uint64_t));
						addr64 = swap_ ? SwapLong(addr64) : addr64;
					}
					else
					{
						tiff_stream.read((char*)&addr32, sizeof(uint32_t));
						addr64 = static_cast<uint64_t>(swap_ ? SwapWord(addr32) : addr32);
					}
					tiff_stream.seekg(addr64, tiff_stream.beg);
				}
				//read the the string
				buf = new char[cnt];
				tiff_stream.read(buf, cnt);
			}
			else if (type == kShort)
			{
				uint16_t value = 0;
				if (cnt > (isBig_ ? 4 : 2))
				{
					if (isBig_)
					{
						tiff_stream.read((char*)&addr64, sizeof(uint64_t));
						addr64 = swap_ ? SwapLong(addr64) : addr64;
					}
					else
					{
						tiff_stream.read((char*)&addr32, sizeof(uint32_t));
						addr64 = static_cast<uint64_t>(swap_ ? SwapWord(addr32) : addr32);
					}
					tiff_stream.seekg(addr64, tiff_stream.beg);
					buf = (char*)(new uint16_t[cnt]);
					tiff_stream.read((char*)buf, sizeof(uint16_t)*cnt);
				}
				else
				{
					tiff_stream.read((char*)&value, sizeof(uint16_t));
					if (swap_) value = SwapShort(value);
					answer = static_cast<uint64_t>(value);
				}
			}
			else if (type == kLong)
			{
				uint32_t value = 0;
				if (cnt > (isBig_ ? 2 : 1))
				{
					if (isBig_)
					{
						tiff_stream.read((char*)&addr64, sizeof(uint64_t));
						addr64 = swap_ ? SwapLong(addr64) : addr64;
					}
					else
					{
						tiff_stream.read((char*)&addr32, sizeof(uint32_t));
						addr64 = static_cast<uint64_t>(swap_ ? SwapWord(addr32) : addr32);
					}
					tiff_stream.seekg(addr64, tiff_stream.beg);
					buf = (char*)(new uint32_t[cnt]);
					tiff_stream.read((char*)buf, sizeof(uint32_t)*cnt);
				}
				else
				{
					tiff_stream.read((char*)&value, sizeof(uint32_t));
					if (swap_) value = SwapWord(value);
					answer = static_cast<uint64_t>(value);
				}
			}
			else if (type == kLong8)
			{
				if (cnt > 1)
				{
					tiff_stream.read((char*)&addr64, sizeof(uint64_t));
					addr64 = swap_ ? SwapLong(addr64) : addr64;
					tiff_stream.seekg(addr64, tiff_stream.beg);
					buf = (char*)(new uint64_t[cnt]);
					tiff_stream.read((char*)buf, sizeof(uint64_t)*cnt);
				}
				else
				{
					tiff_stream.read((char*)&answer, sizeof(uint64_t));
					if (swap_) answer = SwapLong(answer);
				}
			}
			else if (type == kRational)
			{
				//get the two values in the data to make a float.
				uint32_t value = 0;
				tiff_stream.read((char*)&value, sizeof(uint32_t));
				if (swap_) value = SwapWord(value);
				tiff_stream.seekg(value, tiff_stream.beg);
				uint32_t num = 0, den = 0;
				tiff_stream.read((char*)&num, sizeof(uint32_t));
				if (swap_) num = SwapWord(num);
				tiff_stream.seekg(value + sizeof(uint32_t), tiff_stream.beg);
				tiff_stream.read((char*)&den, sizeof(uint32_t));
				if (swap_) den = SwapWord(den);
				double rat = static_cast<double>(num) /
					static_cast<double>(den);
				memcpy(&answer, &rat, sizeof(double));
			}
			else
			{
				std::cerr << "Unhandled TIFF Tag type" << std::endl;
			}

			if (tag == kImageDescriptionTag && buf)
			{
				m_page_info.b_image_desc = true;
				m_page_info.s_image_desc = buf;
			}
			else if (!answer && buf)
				SetPageInfoVector(tag, type, cnt, (void*)buf);
			else
				SetPageInfo(tag, answer);
			if (buf) delete[] buf;
		}
	}
	
	//get the next page offset
	tiff_stream.seekg(current_offset_ + start_off + multiplier*num_entries, tiff_stream.beg);
	if (isBig_)
	{
		uint64_t next_offset = 0;
		tiff_stream.read((char*)&next_offset, sizeof(uint64_t));
		if (swap_)
			m_page_info.ull_next_page_offset = SwapLong(next_offset);
		else
			m_page_info.ull_next_page_offset = next_offset;
		m_page_info.b_next_page_offset = true;
	}
	else
	{
		uint32_t next_offset = 0;
		tiff_stream.read((char*)&next_offset, sizeof(uint32_t));
		if (swap_)
			m_page_info.ull_next_page_offset = static_cast<uint64_t>(SwapWord(next_offset));
		else
			m_page_info.ull_next_page_offset = static_cast<uint64_t>(next_offset);
		m_page_info.b_next_page_offset = true;
	}

	m_page_info.b_valid = true;
}

uint64_t ImageJReader::GetTiffNextPageOffset()
{
	uint64_t results = 0;
	if (!tiff_stream.is_open())
		throw std::runtime_error("TIFF File not open for reading.");
	//go to the current IFD block/page
	tiff_stream.seekg(current_offset_, tiff_stream.beg);
	uint64_t num_entries = 0;
	//how many entries are there?
	if (isBig_)
	{
		tiff_stream.read((char*)&num_entries, sizeof(uint64_t));
		if (swap_) num_entries = SwapLong(num_entries);
	}
	else
	{
		uint16_t temp;
		tiff_stream.read((char*)&temp, sizeof(uint16_t));
		if (swap_) temp = SwapShort(temp);
		num_entries = static_cast<uint64_t>(temp);
	}
	char start_off = 2, multiplier = 12;
	if (isBig_) { start_off = 8; multiplier = 20; }
	//get the next page offset
	tiff_stream.seekg(current_offset_ + start_off + multiplier*num_entries, tiff_stream.beg);
	if (isBig_)
	{
		uint64_t next_offset = 0;
		tiff_stream.read((char*)&next_offset, sizeof(uint64_t));
		if (swap_)
			results = SwapLong(next_offset);
		else
			results = next_offset;
	}
	else
	{
		uint32_t next_offset = 0;
		tiff_stream.read((char*)&next_offset, sizeof(uint32_t));
		if (swap_)
			results = static_cast<uint64_t>(SwapWord(next_offset));
		else
			results = static_cast<uint64_t>(next_offset);
	}
	return results;
}
/*
uint64_t ImageJReader::TurnToPage(uint64_t page)
{
	uint64_t page_save = current_page_;
	//make sure we are on the correct page,
	//reset if ahead
	if (current_page_ > page)
		ResetTiff();
	// fast forward if we are behind.
	if (!imagej_raw_)
	{
		while (current_page_ < page)
		{
			tiff_stream.seekg(current_offset_, tiff_stream.beg);
			if (GetTiffField(kSubFileTypeTag) != 1) current_page_++;
			current_offset_ = GetTiffNextPageOffset();
		}
		if (!current_offset_)
			imagej_raw_ = true;
	}
	if (current_page_ != page_save &&
		!imagej_raw_)
		InvalidatePageInfo();
	return current_page_;
}
*/
uint16_t ImageJReader::SwapShort(uint16_t num) {
	return ((num & 0x00FF) << 8) | ((num & 0xFF00) >> 8);
}

uint32_t ImageJReader::SwapWord(uint32_t num) {
	return ((num & 0x000000FF) << 24) | ((num & 0xFF000000) >> 24) |
		((num & 0x0000FF00) << 8) | ((num & 0x00FF0000) >> 8);
}

uint64_t ImageJReader::SwapLong(uint64_t num) {
	return
		((num & 0x00000000000000FF) << 56) |
		((num & 0xFF00000000000000) >> 56) |
		((num & 0x000000000000FF00) << 40) |
		((num & 0x00FF000000000000) >> 40) |
		((num & 0x0000000000FF0000) << 24) |
		((num & 0x0000FF0000000000) >> 24) |
		((num & 0x00000000FF000000) << 8) |
		((num & 0x000000FF00000000) >> 8);
}

void ImageJReader::SetSliceSeq(bool ss)
{
	//enable searching for slices
	m_slice_seq = ss;
}

bool ImageJReader::GetSliceSeq()
{
	return m_slice_seq;
}

void ImageJReader::SetTimeId(wstring &id)
{
	m_time_id = id;
}

wstring ImageJReader::GetTimeId()
{
	return m_time_id;
}

void ImageJReader::SetBatch(bool batch)
{
	if (batch)
	{
		//separate path and name
		wstring search_path = GET_PATH(m_path_name);
		wstring suffix = GET_SUFFIX(m_path_name);
		FIND_FILES(search_path, suffix, m_batch_list, m_cur_batch);
		m_batch = true;
	}
	else
		m_batch = false;
}

bool ImageJReader::IsNewBatchFile(wstring name)
{
	if (m_batch_list.size() == 0)
		return true;

	for (int i = 0; i < (int)m_batch_list.size(); i++)
	{
		if (IsBatchFileIdentical(name, m_batch_list[i]))
			return false;
	}

	return true;
}

bool ImageJReader::IsBatchFileIdentical(wstring name1, wstring name2)
{
	if (m_4d_seq.size() > 1)
	{
		int64_t pos = name1.find(m_time_id);
		if (pos == -1)
			return false;
		wstring find_str = name1.substr(0, pos + 2);
		pos = name2.find(find_str);
		if (pos == -1)
			return false;
		else
			return true;
	}
	else if (m_slice_seq)
	{
		int64_t pos = name1.find_last_of(L'.');
		int64_t begin = -1;
		int64_t end = -1;
		for (int i = int(pos) - 1; i >= 0; i--)
		{
			if (iswdigit(name1[i]) && end == -1 && begin == -1)
				end = i;
			if (!iswdigit(name1[i]) && end != -1 && begin == -1)
			{
				begin = i;
				break;
			}
		}
		if (begin == -1)
			return false;
		else
		{
			wstring find_str = name1.substr(0, begin + 1);
			pos = name2.find(find_str);
			if (pos == -1)
				return false;
			else
				return true;
		}
	}
	else
	{
		if (name1 == name2)
			return true;
		else
			return false;
	}
}

int ImageJReader::LoadBatch(int index)
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

Nrrd* ImageJReader::Convert(int t, int c, bool get_max)
{	
	if (t < 0 || t >= m_time_num || c < 0 || c >= m_chan_num)
		return 0;

	//if (isHyperstack_ && m_max_value)
	//	get_max = false;

	Nrrd* data = 0;
	//TODO: Fix the m_data_name.
	//TimeDataInfo chan_info = m_4d_seq[t];
	//if (!isHyperstack_ || isHsTimeSeq_)
		//m_data_name = GET_NAME(chan_info.slices[0].slice);
	
	data = ReadFromImageJ(t, c, get_max);
	m_cur_time = t;
	return data;
}

wstring ImageJReader::GetCurDataName(int t, int c)
{
	if (isHyperstack_ && !isHsTimeSeq_)
		return m_path_name;
	else
	{
		if (t >= 0 && t < (int64_t)m_4d_seq.size())
			return (m_4d_seq[t].slices)[0].slice;
		else
			return L"";
	}
}

wstring ImageJReader::GetCurMaskName(int t, int c)
{
	if (isHyperstack_)
	{
		wostringstream woss;
		woss << m_path_name.substr(0, m_path_name.find_last_of('.'));
		if (m_time_num > 1) woss << "_T" << t;
		if (m_chan_num > 1) woss << "_C" << c;
		woss << ".msk";
		wstring mask_name = woss.str();
		return mask_name;
	}
	else
	{
		if (t >= 0 && t < (int64_t)m_4d_seq.size())
		{
			wstring data_name = (m_4d_seq[t].slices)[0].slice;
			wostringstream woss;
			woss << data_name.substr(0, data_name.find_last_of('.'));
			if (m_chan_num > 1) woss << "_C" << c;
			woss << ".msk";
			wstring mask_name = woss.str();
			return mask_name;
		}
		else
			return L"";
	}
}

wstring ImageJReader::GetCurLabelName(int t, int c)
{
	if (isHyperstack_)
	{
		wostringstream woss;
		woss << m_path_name.substr(0, m_path_name.find_last_of('.'));
		if (m_time_num > 1) woss << "_T" << t;
		if (m_chan_num > 1) woss << "_C" << c;
		woss << ".lbl";
		wstring label_name = woss.str();
		return label_name;
	}
	else
	{
		if (t >= 0 && t < (int64_t)m_4d_seq.size())
		{
			wstring data_name = (m_4d_seq[t].slices)[0].slice;
			wostringstream woss;
			woss << data_name.substr(0, data_name.find_last_of('.'));
			if (m_chan_num > 1) woss << "_C" << c;
			woss << ".lbl";
			wstring label_name = woss.str();
			return label_name;
		}
		else
			return L"";
	}
}

//read a tile
void ImageJReader::GetTiffTile(uint64_t page, uint64_t tile,
	void *data, uint64_t tile_size, uint64_t tile_height)
{
	uint64_t byte_count = GetTiffTileCount(tile);
	tiff_stream.seekg(GetTiffTileOffset(tile), tiff_stream.beg);
	//actually read the data now
	char *temp = new char[byte_count];
	//unsigned long long pos = tiff_stream.tellg();
	tiff_stream.read((char*)temp, byte_count);
	bool eight_bits = 8 == GetTiffField(kBitsPerSampleTag);
	//get compression tag, decompress if necessary
	uint64_t tmp = GetTiffField(kCompressionTag);
	uint64_t prediction = GetTiffField(kPredictionTag);
	uint64_t samples = GetTiffField(kSamplesPerPixelTag);
	samples = samples == 0 ? 1 : samples;
	tsize_t stride = (GetTiffField(kPlanarConfigurationTag) == 2) ? 1 : samples;
	bool isCompressed = tmp == 5;
	if (isCompressed)
	{
		LZWDecode((tidata_t)temp, (tidata_t)data, tile_size);
		if (prediction == 2)
		{
			for (size_t j = 0; j < tile_height; j++)
				if (eight_bits)
					DecodeAcc8((tidata_t)data + j*m_x_size*samples, m_x_size*samples, stride);
				else
					DecodeAcc16((tidata_t)data + j*m_x_size*samples * 2, m_x_size*samples * 2, stride);
		}
	}
	else
		memcpy(data, temp, byte_count);
	delete[] temp;

	//swap after decompression
	if (swap_ && !eight_bits)
	{
		short * data2 = reinterpret_cast<short*>(data);
		for (uint64_t sh = 0; sh < tile_size / 2; sh++)
			data2[sh] = SwapShort(data2[sh]);
	}
}

Nrrd* ImageJReader::ReadFromImageJ(int t, int c, bool get_max) {	
	// ImageJ code to read the data.	
	char* path_cstr = new char[m_path_name.length() + 1];
	sprintf(path_cstr, "%ws", m_path_name.c_str());

	jmethodID method_id = NULL;
	if (m_eight_bit == true){
		method_id = m_pJVMInstance->m_pEnv->GetStaticMethodID(m_imageJ_cls, "getByteData", "([Ljava/lang/String;II)[B");
	}
	else {
		method_id = m_pJVMInstance->m_pEnv->GetStaticMethodID(m_imageJ_cls, "getIntDataB", "([Ljava/lang/String;II)[S");
	}
	
	void* t_data = NULL;
	if (method_id == nullptr) {
		cerr << "ERROR: method void mymain() not found !" << endl;
		return NULL;
	}
	else if (m_eight_bit == true){
		jobjectArray arr = m_pJVMInstance->m_pEnv->NewObjectArray(2,      // constructs java array of 3
			m_pJVMInstance->m_pEnv->FindClass("java/lang/String"),    // Strings
			m_pJVMInstance->m_pEnv->NewStringUTF("str"));   // each initialized with value "str"
		m_pJVMInstance->m_pEnv->SetObjectArrayElement(arr, 0, m_pJVMInstance->m_pEnv->NewStringUTF(path_cstr));  // change an element
		m_pJVMInstance->m_pEnv->SetObjectArrayElement(arr, 1, m_pJVMInstance->m_pEnv->NewStringUTF("4D_1ch.lsm"));  // change an element		

		jbyteArray val = (jbyteArray)(m_pJVMInstance->m_pEnv->CallStaticObjectMethod(m_imageJ_cls, method_id, arr, (jint)t, (jint)c));   // call the method with the arr as argument.
		jboolean flag = m_pJVMInstance->m_pEnv->ExceptionCheck();
		if (flag) {
			m_pJVMInstance->m_pEnv->ExceptionClear();
			//TODO: code to handle exception.
		}

		jsize len = m_pJVMInstance->m_pEnv->GetArrayLength(val);
		jbyte* body = m_pJVMInstance->m_pEnv->GetByteArrayElements(val, 0);	
		unsigned char* dummy = reinterpret_cast<unsigned char*>(body);
		//t_data = dummy;
		//TODO: There is a better way to do this with ReleaseShortArrayElements.
		//Link: https://stackoverflow.com/questions/50613889/how-to-free-memory-allocated-with-jshortarray-jbytearray-from-jni-java-and-c 
		t_data = new unsigned char[len];
		for (int i = 0; i < len; i++) {
			int test = *(body + i);
			*((unsigned char*)t_data + i) = test;			
		}		
		m_pJVMInstance->m_pEnv->DeleteLocalRef(arr);
	}
	else if (m_eight_bit == false)
	{
		//m_pJVMInstance->m_pEnv->PushLocalFrame();
		jobjectArray arr = m_pJVMInstance->m_pEnv->NewObjectArray(2,      // constructs java array of 3
			m_pJVMInstance->m_pEnv->FindClass("java/lang/String"),    // Strings
			m_pJVMInstance->m_pEnv->NewStringUTF("str"));   // each initialized with value "str"
		m_pJVMInstance->m_pEnv->SetObjectArrayElement(arr, 0, m_pJVMInstance->m_pEnv->NewStringUTF(path_cstr));  // change an element
		m_pJVMInstance->m_pEnv->SetObjectArrayElement(arr, 1, m_pJVMInstance->m_pEnv->NewStringUTF("4D_1ch.lsm"));  // change an element
																													
		jshortArray val = (jshortArray)(m_pJVMInstance->m_pEnv->CallStaticObjectMethod(m_imageJ_cls, method_id, arr, (jint)t, (jint)c));   // call the method with the arr as argument.
		jsize len = m_pJVMInstance->m_pEnv->GetArrayLength(val);
		jshort* body = m_pJVMInstance->m_pEnv->GetShortArrayElements(val, 0);		
		unsigned short int* dummy = reinterpret_cast<unsigned short int*>(body);
		//t_data = dummy;
		t_data = new unsigned short int[len];
		for (int i = 0; i < len; i++) {
			int test = *(body + i);
			*((unsigned short int*)t_data + i) = test;
		}
		m_pJVMInstance->m_pEnv->DeleteLocalRef(arr);
	}

	// Creating Nrrd out of the data.
	Nrrd *nrrdout = nrrdNew();	
	
	int numPages = m_slice_num;	
	unsigned long long total_size = (unsigned long long)m_x_size*(unsigned long long)m_y_size*(unsigned long long)numPages;	
	if (!t_data)
		throw std::runtime_error("No data received from imageJ.");
	
	if (m_eight_bit)
		nrrdWrap(nrrdout, (uint8_t*)t_data, nrrdTypeUChar, 3, (size_t)m_x_size, (size_t)m_y_size, (size_t)numPages);
	else
		nrrdWrap(nrrdout, (uint16_t*)t_data, nrrdTypeUShort, 3, (size_t)m_x_size, (size_t)m_y_size, (size_t)numPages);
	nrrdAxisInfoSet(nrrdout, nrrdAxisInfoSpacing, m_xspc, m_yspc, m_zspc);
	nrrdAxisInfoSet(nrrdout, nrrdAxisInfoMax, m_xspc*m_x_size, m_yspc*m_y_size, m_zspc*numPages);
	nrrdAxisInfoSet(nrrdout, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
	nrrdAxisInfoSet(nrrdout, nrrdAxisInfoSize, (size_t)m_x_size, (size_t)m_y_size, (size_t)numPages);

	if (!m_eight_bit) {
		if (get_max) {
			double value;
			unsigned long long totali = (unsigned long long)m_slice_num*
				(unsigned long long)m_x_size*(unsigned long long)m_y_size;
			for (unsigned long long i = 0; i < totali; ++i)
			{
				value = ((unsigned short*)nrrdout->data)[i];
				m_max_value = value > m_max_value ? value : m_max_value;
			}			
		}
		if (m_max_value > 0.0) 
			m_scalar_scale = 65535.0 / m_max_value;
		else 
			m_scalar_scale = 1.0;		
	}
	else 
		m_max_value = 255.0;

	return nrrdout;
}