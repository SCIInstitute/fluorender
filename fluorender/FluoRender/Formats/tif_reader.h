#ifndef _TIF_READER_H_
#define _TIF_READER_H_

#include <Formats\base_reader.h>
#include <cstdio>
#include <windows.h>
#include <vector>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <tiffio.h>

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
	 * Finds the tag value given by \@tag of a tiff.
	 * @param name The name of the tiff file to read;
	 * @param tag The tag to look for in the header.
	 * @warn If the result is not 16 or 32 bit int, it is
	 *       cast into a 32 bit int. Caller must cast it
	 *       back to correct type, such as float for XResolution.
	 * @return The value in the header denoted by tag.
	 */
	uint32_t GetTiffField(std::wstring name, const int tag);
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
	 * Determines the number of pages in a tiff.
	 * @param name The name of the tiff file.
	 * @return The number of pages in the file.
	 */
	uint32_t GetNumTiffPages(std::wstring name);
	void SetBatch(bool batch);
	int LoadBatch(int index);
	int LoadOffset(int offset);
	Nrrd* Convert(bool get_max);
	Nrrd* Convert(int c, bool get_max);
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
	/** The tiff tag for subfile type */
	static const int kSubFileTypeTag = 254;
	/** The tiff tag for image width */
	static const int kImageWidthTag = 256;
	/** The tiff tag for image length */
	static const int kImageLengthTag = 257;
	/** The tiff tag for bits per sample */
	static const int kBitsPerSampleTag = 258;
	/** The tiff tag for image description */
	static const int kImageDescriptionTag = 270;
	/** The tiff tag for strip offsets */
	static const int kStripOffsetsTag = 273;
	/** The tiff tag for Samples per pixel */
	static const int kSamplesPerPixelTag = 277;
	/** The tiff tag for rows per strip */
	static const int kRowsPerStripTag = 278;
	/** The tiff tag for strip bytes count */
	static const int kStripBytesCountTag = 279;
	/** The tiff tag for x resolution */
	static const int kXResolutionTag = 282;
	/** The tiff tag for y resolution */
	static const int kYResolutionTag = 283;
	/** The BYTE type */
	static const char kByte = 1;
	/** The SHORT type */
	static const char kShort = 3;
	/** The LONG type */
	static const char kLong = 4;
	/** The RATIONAL type */
	static const char kRational = 5;

private:
	bool IsNewBatchFile(wstring name);
	bool IsBatchFileIdentical(wstring name1, wstring name2);

	static bool tif_sort(const TimeDataInfo& info1, const TimeDataInfo& info2);
	static bool tif_slice_sort(const SliceInfo& info1, const SliceInfo& info2);
	//read single tiff file
	Nrrd* ReadSingleTiff(wstring filename, int c, bool get_max);
	//read tiff sequence
	Nrrd* ReadSequenceTiff(vector<SliceInfo> &filelist, int c, bool get_max);
};

#endif//_TIF_READER_H_