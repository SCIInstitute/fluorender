/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2024 Scientific Computing and Imaging Institute,
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
#ifndef _TIF_READER_H_
#define _TIF_READER_H_

#include <base_reader.h>
#include <cstdio>
#include <vector>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <stdint.h>
#include <string>
#include <deque>
#include <set>

using namespace std;

class TIFReader : public BaseReader
{
public:
	TIFReader();
	~TIFReader();

	int GetType() { return READER_TIF_TYPE; }

	void SetFile(string &file);
	void SetFile(wstring &file);
	void SetSliceSeq(bool ss);
	bool GetSliceSeq();
	void SetChannSeq(bool cs);
	bool GetChannSeq();
	void SetDigitOrder(int order);
	int GetDigitOrder();
	void SetTimeId(wstring &id);
	wstring GetTimeId();
	int Preprocess();
	/**
	 * Finds the tag value given by \@tag of a tiff on the current page.
	 * @param tag The tag to look for in the header.
	 * @warn If the result is not expected to be 8, 16 or 32 bit int, 
	 *       it will be a data assigned to pointer below.
	 *       The caller must allocate necessary memory.
	 * @param pointer The pointer to the longer data (can be null, 
	 *        and it is not used if expected result is an int).
	 * @param size The maximum number of bytes to copy into pointer.
	 *        If pointer is null, this parameter is checked to see if
	 *        we want the value (kValue), count (kCount), or 
	 *        type (kType) of the @tag provided.
	 * @throws An exception if a tiff is not open.
	 * @return The value in the header denoted by tag.
	 */
	uint64_t GetTiffField(const uint64_t tag);
	double GetTiffFieldD(const uint64_t tag);
	//next page
	uint64_t GetTiffNextPageOffset();
	uint64_t TurnToPage(uint64_t page);
	//get description
	inline bool GetImageDescription(string &desc);
	/**
	* Gets the specified offset for the strip offset or count.
	* @param tag The tag for either the offset or the count.
	* @param strip The strip number to get the correct count/offset.
	* @return The count or strip offset determined by @strip.
	*/
	inline uint32_t GetTiffStripNum();
	inline uint64_t GetTiffStripOffset(uint64_t strip);
	inline uint64_t GetTiffStripCount(uint64_t strip);
	//for tiles
	inline bool GetTiffUseTiles();
	inline uint32_t GetTiffTileNum();
	inline uint64_t GetTiffTileOffset(uint64_t tile);
	inline uint64_t GetTiffTileCount(uint64_t tile);
	//resolution
	inline double GetTiffXResolution();
	inline double GetTiffYResolution();
	/**
	 * Reads a strip of data from a tiff file.
	 * @param page The page to read from.
	 * @param strip Which strip to read from the file.
	 * @param data The location of the buffer to store data.
	 * @param strip_size The uncompressed size.
	 * @throws An exception if a tiff is not open.
	 */
	void GetTiffStrip( 
		uint64_t page,
		uint64_t strip,
		void * data,
		uint64_t strip_size);
	//read a tile
	void GetTiffTile(
		uint64_t page,
		uint64_t tile,
		void *data,
		uint64_t tile_size,
		uint64_t tile_height);
	//scan for minmax
	void GetTiffStripMinMax(
		uint64_t page,
		uint64_t strip,
		uint64_t strip_size);
	void GetTiffTileMinMax(
		uint64_t page,
		uint64_t tile,
		uint64_t tile_size,
		uint64_t tile_height);
	/**
	 * Opens the tiff stream.
	 * @param name The filename of the tiff.
	 * @throws An exception when the opening fails.
	 */
	void OpenTiff(std::wstring name);
	/**
	 * Closes the tiff stream if it is open.
	 */
	void CloseTiff();
	/**
	 * Resets the tiff reader to the start of the file.
	 */
	void ResetTiff();
	/**
	 * This method swaps the byte order of a short.
	 * @param num The short to swap byte order.
	 * @return The short with bytes swapped.
	 */
	uint16_t SwapShort(uint16_t num);
	/**
	 * This method swaps the byte order of a word.
	 * @param num The word to swap byte order.
	 * @return The word with bytes swapped.
	 */
	uint32_t SwapWord(uint32_t num);
	/**
	 * This method swaps the byte order of a 8byte number.
	 * @param num The 8byte to swap byte order.
	 * @return The 8byte with bytes swapped.
	 */
	uint64_t SwapLong(uint64_t num);
	/**
	 * Determines the number of pages in a tiff.
	 * @throws An exception if a tiff is not open.
	 * @return The number of pages in the file.
	 */
	uint64_t GetNumTiffPages();
	void SetBatch(bool batch);
	int LoadBatch(int index);
	Nrrd* Convert(int t, int c, bool get_max);
	wstring GetCurDataName(int t, int c);
	wstring GetCurMaskName(int t, int c);
	wstring GetCurLabelName(int t, int c);

	wstring GetPathName() {return m_path_name;}
	wstring GetDataName() {return m_data_name;}
	int GetCurTime() {return m_cur_time;}
	int GetTimeNum() {return m_time_num;}
	int GetChanNum() {return m_chan_num;}
	double GetExcitationWavelength(int chan) {return 0.0;}
	int GetSliceNum() {return m_slice_num;}
	int GetXSize() {return m_x_size;}
	int GetYSize() {return m_y_size;}
	bool IsSpcInfoValid() {return m_valid_spc;}
	double GetXSpc() {return m_xspc;}
	double GetYSpc() {return m_yspc;}
	double GetZSpc() {return m_zspc;}
	double GetMaxValue() {return m_max_value;}
	double GetScalarScale() {return m_scalar_scale;}
	bool GetBatch() {return m_batch;}
	int GetBatchNum() {return (int)m_batch_list.size();}
	int GetCurBatch() {return m_cur_batch;}

private:
	wstring m_data_name;
	bool isBig_;
	bool isHyperstack_;		//true if it is a hyperstack tiff saved by ImageJ
	bool isHsTimeSeq_;		//true if it is a time sequence of hyperstack files

	struct SliceInfo
	{
		int slicenumber;	//slice number for sorting
		wstring slice;		//slice file name
		int pagenumber;		//used to find the slice if it's in a hyperstack
							//for a multichannel data set, this is the number of the first channel
	};
	struct TimeDataInfo
	{
		int type;	//0-single file;1-sequence
		int filenumber;	//filenumber for sorting
		std::vector<SliceInfo> slices;
	};
	std::vector<TimeDataInfo> m_4d_seq;

	struct NamePattern
	{
		size_t start;
		size_t end;
		size_t len;//0:indefinite
		int type;//0:string; 1:digits
		int use;//0:z sections; 1:channels; 2:time
		wstring str;//content
	};
	std::deque<NamePattern> m_name_patterns;
	std::set<int> m_slice_count;
	std::set<int> m_chann_count;//counting total numbers in preprocessing

	bool m_slice_seq;
	bool m_chann_seq;
	int m_digit_order;
	int m_time_num;
	int m_cur_time;
	int m_chan_num;
	int m_slice_num;
	int m_x_size;
	int m_y_size;
	bool m_valid_spc;
	double m_xspc;
	double m_yspc;
	double m_zspc;
	double m_max_value;
	double m_scalar_scale;

	//pages
	bool m_b_page_num;
	unsigned long long m_ull_page_num;
	//page properties
	struct PageInfo
	{
		//if the page info is valid
		bool b_valid;
		//sub file type
		bool b_sub_file_type;
		unsigned long ul_sub_file_type;
		//width
		bool b_image_width;
		unsigned long ul_image_width;
		//length
		bool b_image_length;
		unsigned long ul_image_length;
		//bits per sample
		bool b_bits_per_sample;
		unsigned short us_bits_per_sample;
		//compression
		bool b_compression;
		unsigned short us_compression;
		//prediction
		bool b_prediction;
		unsigned short us_prediction;
		//planar configuration
		bool b_planar_config;
		unsigned short us_planar_config;
		//image desc
		bool b_image_desc;
		string s_image_desc;
		//strip number
		bool b_strip_num;
		unsigned long ul_strip_num;
		//strip offsets
		bool b_strip_offsets;
		std::vector<unsigned long long> ull_strip_offsets;
		//strip byte counts
		bool b_strip_byte_counts;
		std::vector<unsigned long long> ull_strip_byte_counts;
		//rows per strip
		bool b_rows_per_strip;
		unsigned long ul_rows_per_strip;
		//samples per pixel
		bool b_samples_per_pixel;
		unsigned short us_samples_per_pixel;
		//x resolution
		bool b_x_resolution;
		double d_x_resolution;
		//y resolution
		bool b_y_resolution;
		double d_y_resolution;
		//use tiles (instead of strips)
		bool b_use_tiles;
		//tile width
		bool b_tile_width;
		unsigned long ul_tile_width;
		//tile length
		bool b_tile_length;
		unsigned long ul_tile_length;
		//tile number
		bool b_tile_num;
		unsigned long ul_tile_num;
		//tile offsets
		bool b_tile_offsets;
		std::vector<unsigned long long> ull_tile_offsets;
		//tile byte counts
		bool b_tile_byte_counts;
		std::vector<unsigned long long> ull_tile_byte_counts;
		//next page offset
		bool b_next_page_offset;
		unsigned long long ull_next_page_offset;
		//sample format
		bool b_sample_format;
		unsigned short us_sample_format;
		//min sample value
		bool b_min_sample_value;
		double d_min_sample_value;
		//max sample value
		bool b_max_sample_value;
		double d_max_sample_value;
	};
	PageInfo m_page_info;

	//time sequence id
	wstring m_time_id;
	/** The input stream for reading the tiff */
	std::ifstream tiff_stream;
	/** This keeps track of what page we are on in the tiff */
	uint64_t current_page_;
	/** This is the offset of the page we are currently on */
	uint64_t current_offset_;
	bool imagej_raw_possible_;
	bool imagej_raw_;
	/** Tells us if the data is little endian */
	bool swap_;
	/** The tiff tag for subfile type */
	static const uint64_t kSubFileTypeTag = 254;
	/** The tiff tag for image width */
	static const uint64_t kImageWidthTag = 256;
	/** The tiff tag for image length */
	static const uint64_t kImageLengthTag = 257;
	/** The tiff tag for bits per sample */
	static const uint64_t kBitsPerSampleTag = 258;
	/** The tiff tag for compression */
	static const uint64_t kCompressionTag = 259;
	/** The tiff tag for decode prediction */
	static const uint64_t kPredictionTag = 317;
	/** The tiff tag for planar configuration */
	static const uint64_t kPlanarConfigurationTag = 284;
	/** The tiff tag for image description */
	static const uint64_t kImageDescriptionTag = 270;
	/** The tiff tag for strip offsets */
	static const uint64_t kStripOffsetsTag = 273;
	/** The tiff tag for Samples per pixel */
	static const uint64_t kSamplesPerPixelTag = 277;
	/** The tiff tag for rows per strip */
	static const uint64_t kRowsPerStripTag = 278;
	/** The tiff tag for strip bytes counts */
	static const uint64_t kStripBytesCountsTag = 279;
	/** The tiff tag for x resolution */
	static const uint64_t kXResolutionTag = 282;
	/** The tiff tag for y resolution */
	static const uint64_t kYResolutionTag = 283;
	/** The tiff tag for tile width */
	static const uint64_t kTileWidthTag = 322;
	/** The tiff tag for tile length */
	static const uint64_t kTileLengthTag = 323;
	/** The tiff tag for tile offsets */
	static const uint64_t kTileOffsetsTag = 324;
	/** The tiff tag for tile bytes counts */
	static const uint64_t kTileBytesCountsTag = 325;
	/** The tiff tag number of entries on current page */
	static const uint64_t kNextPageOffsetTag = 500;
	/** how to interpret each data sample in a pixel*/
	static const uint64_t kSampleFormat = 339;
	/** The minimum sample value*/
	static const uint64_t kMinSampleValue = 340;
	/** The maximum sample value*/
	static const uint64_t kMaxSampleValue = 341;
	/** The BYTE type */
	static const uint8_t kByte = 1;
	/** The ASCII type */
	static const uint8_t kASCII = 2;
	/** The SHORT type */
	static const uint8_t kShort = 3;
	/** The LONG type */
	static const uint8_t kLong = 4;
	/** The RATIONAL type */
	static const uint8_t kRational = 5;
	/** The Float type 32-bit*/
	static const uint8_t kFloat = 11;
	/** The Float type 64-bit*/
	static const uint8_t kDouble = 12;
	/** The TIFF_LONG8 type (unsigned 8-byte int)*/
	static const uint8_t kLong8 = 16;
	/** The TIFF_SLONG8 type (signed 8-byte int)*/
	static const uint8_t kSLong8 = 17;
	/** The TIFF_IFD8 type (unsigned 8-byte IFD offset)*/
	static const uint8_t kIFD8 = 18;
	/** Return the offset/value of the tag */
	static const uint8_t kOffset = 4;
	/** Return the value of the tag */
	static const uint8_t kValueAddress = 3;
	/** Return the type of the tag */
	static const uint8_t kType = 1;
	/** Return the count of the tag */
	static const uint8_t kCount = 2;
	/** This is a regular TIF */
	static const uint8_t kRegularTiff = 42;
	/** This is a Big TIF */
	static const uint8_t kBigTiff = 43;

private:
	bool IsNewBatchFile(wstring name);
	bool IsBatchFileIdentical(wstring name1, wstring name2);

	static bool tif_sort(const TimeDataInfo& info1, const TimeDataInfo& info2);
	static bool tif_slice_sort(const SliceInfo& info1, const SliceInfo& info2);
	//read tiff
	Nrrd* ReadTiff(std::vector<SliceInfo> &filelist, int c, bool get_max);

	//name pattern
	void AnalyzeNamePattern(std::wstring &path_name);
	void AddPatternR(wchar_t c, size_t pos);//add backwards
	std::wstring GetSearchString(int mode, int t);
	int GetPatternNumber(std::wstring &path_name, int mode, bool count=false);

	//invalidate page info
	bool TagInInfo(uint16_t tag);
	void SetPageInfo(uint16_t tag, uint64_t answer);
	void SetPageInfoVector(uint16_t tag, uint16_t type, uint64_t cnt, void* data);
	void ReadTiffFields();
	inline void InvalidatePageInfo();

	//get min max for conversion
	bool GetTagMinMax();
	bool GetFloatMinMax();
};

void TIFReader::InvalidatePageInfo()
{
	m_page_info.b_valid = false;
	m_page_info.b_sub_file_type = false;
	m_page_info.b_image_width = false;
	m_page_info.b_image_length = false;
	m_page_info.b_bits_per_sample = false;
	m_page_info.b_compression = false;
	m_page_info.b_prediction = false;
	m_page_info.b_planar_config = false;
	m_page_info.b_image_desc = false;
	m_page_info.b_strip_num = false;
	m_page_info.b_strip_offsets = false;
	m_page_info.b_strip_byte_counts = false;
	m_page_info.b_rows_per_strip = false;
	m_page_info.b_samples_per_pixel = false;
	m_page_info.b_x_resolution = false;
	m_page_info.b_y_resolution = false;
	m_page_info.b_use_tiles = false;
	m_page_info.b_tile_width = false;
	m_page_info.b_tile_length = false;
	m_page_info.b_tile_num = false;
	m_page_info.b_tile_offsets = false;
	m_page_info.b_tile_byte_counts = false;
	m_page_info.b_next_page_offset = false;
	m_page_info.s_image_desc.clear();
	m_page_info.ull_strip_offsets.clear();
	m_page_info.ull_strip_byte_counts.clear();
	m_page_info.ull_tile_offsets.clear();
	m_page_info.ull_tile_byte_counts.clear();
	m_page_info.b_min_sample_value = false;
	m_page_info.b_max_sample_value = false;
}

//get description
bool TIFReader::GetImageDescription(string &desc)
{
	if (m_page_info.b_valid && m_page_info.b_image_desc)
	{
		desc = m_page_info.s_image_desc;
		return true;
	}
	else
		return false;
}

//strips
uint32_t TIFReader::GetTiffStripNum()
{
	if (m_page_info.b_valid && m_page_info.b_strip_num)
		return m_page_info.ul_strip_num;
	else
		return 0;
}

uint64_t TIFReader::GetTiffStripOffset(uint64_t strip)
{
	if (m_page_info.b_valid && m_page_info.b_strip_offsets)
		return m_page_info.ull_strip_offsets[strip];
	else
		return 0;
}

uint64_t TIFReader::GetTiffStripCount(uint64_t strip)
{
	if (m_page_info.b_valid && m_page_info.b_strip_byte_counts)
		return m_page_info.ull_strip_byte_counts[strip];
	else
		return 0;
}

//for tiles
bool TIFReader::GetTiffUseTiles()
{
	if (m_page_info.b_valid)
		return m_page_info.b_use_tiles;
	else
		return false;
}

uint64_t TIFReader::GetTiffTileOffset(uint64_t tile)
{
	if (m_page_info.b_valid && m_page_info.b_tile_offsets)
		return m_page_info.ull_tile_offsets[tile];
	else
		return 0;
}

uint32_t TIFReader::GetTiffTileNum()
{
	if (m_page_info.b_valid && m_page_info.b_tile_num)
		return m_page_info.ul_tile_num;
	else
		return 0;
}

uint64_t TIFReader::GetTiffTileCount(uint64_t tile)
{
	if (m_page_info.b_valid && m_page_info.b_tile_byte_counts)
		return m_page_info.ull_tile_byte_counts[tile];
	else
		return 0;
}

//resolution
inline double TIFReader::GetTiffXResolution()
{
	if (m_page_info.b_valid && m_page_info.b_x_resolution)
		return m_page_info.d_x_resolution;
	else
		return 0.0;
}

inline double TIFReader::GetTiffYResolution()
{
	if (m_page_info.b_valid && m_page_info.b_y_resolution)
		return m_page_info.d_y_resolution;
	else
		return 0.0;
}

#endif//_TIF_READER_H_
