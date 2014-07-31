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

using namespace std;

class TIFReader : public BaseReader
{
public:
	TIFReader();
	~TIFReader();

	void SetFile(string &file);
	void SetFile(wstring &file);
	void SetSliceSeq(bool ss);
	bool GetSliceSeq();
	void SetTimeId(wstring &id);
	wstring GetTimeId();
	void Preprocess();
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
	uint64_t GetTiffField(
		const uint64_t tag,
		void * pointer, uint64_t size);
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
	/**
	 * Gets the specified offset for the strip offset or count.
	 * @param tag The tag for either the offset or the count.
	 * @param strip The strip number to get the correct count/offset.
	 * @return The count or strip offset determined by @strip.
	 */
	uint64_t GetTiffStripOffsetOrCount(uint64_t tag, uint64_t strip);
	void SetBatch(bool batch);
	int LoadBatch(int index);
	int LoadOffset(int offset);
	Nrrd* Convert(int t, int c, bool get_max);
	wstring GetCurName(int t, int c);

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
	wstring m_path_name;
	wstring m_data_name;
	bool isBig_;

	struct SliceInfo
	{
		int slicenumber;	//slice number for sorting
		wstring slice;		//slice file name
	};
	struct TimeDataInfo
	{
		int type;	//0-single file;1-sequence
		int filenumber;	//filenumber for sorting
		vector<SliceInfo> slices;
	};
	vector<TimeDataInfo> m_4d_seq;

	//3d batch
	bool m_batch;
	vector<wstring> m_batch_list;
	int m_cur_batch;

	bool m_slice_seq;
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

	//time sequence id
	wstring m_time_id;
	/** The input stream for reading the tiff */
	std::ifstream tiff_stream;
	/** This keeps track of what page we are on in the tiff */
	uint64_t current_page_;
	/** This is the offset of the page we are currently on */
	uint64_t current_offset_;
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
	/** The tiff tag for strip bytes count */
	static const uint64_t kStripBytesCountTag = 279;
	/** The tiff tag for x resolution */
	static const uint64_t kXResolutionTag = 282;
	/** The tiff tag for y resolution */
	static const uint64_t kYResolutionTag = 283;
	/** The tiff tag number of entries on current page */
	static const uint64_t kNextPageOffsetTag = 500;
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
	Nrrd* ReadTiff(vector<SliceInfo> &filelist, int c, bool get_max);
};

#endif//_TIF_READER_H_
