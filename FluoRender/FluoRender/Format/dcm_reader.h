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
#ifndef _DCM_READER_H_
#define _DCM_READER_H_

#include <base_vol_reader.h>
#include <map>

class DCMReader : public BaseVolReader
{
public:
	DCMReader();
	~DCMReader();

	enum DCMCompression {
		DCM_UNCOMPRESSED = 0,
		DCM_JPEG_BASELINE = 1,
		DCM_JPEG_LOSSLESS = 2,
		DCM_DEFLATE = 3,
		DCM_JPEG2000 = 4,
		DCM_UNSUPPORTED = -1
	};

	int GetType() { return READER_DCM_TYPE; }

	void SetFile(const std::wstring& file);
	int Preprocess();
	void SetBatch(bool batch);
	int LoadBatch(int index);
	Nrrd* Convert(int t, int c, bool get_max);
	std::wstring GetCurDataName(int t, int c);
	std::wstring GetCurMaskName(int t, int c);
	std::wstring GetCurLabelName(int t, int c);

	std::wstring GetPathName() { return m_path_name; }
	std::wstring GetDataName() { return m_data_name; }
	int GetTimeNum() { return m_time_num; }
	int GetCurTime() { return m_cur_time; }
	int GetChanNum() { return m_chan_num; }
	double GetExcitationWavelength(int chan) {return 0.0;}
	int GetSliceNum() { return m_slice_num; }
	int GetXSize() { return m_x_size; }
	int GetYSize() { return m_y_size; }
	bool IsSpcInfoValid() { return m_valid_spc; }
	double GetXSpc() { return m_xspc; }
	double GetYSpc() { return m_yspc; }
	double GetZSpc() { return m_zspc; }
	double GetMinValue() { return m_min_value; }
	double GetMaxValue() { return m_max_value; }
	double GetScalarScale() { return m_scalar_scale; }
	bool GetBatch() { return m_batch; }
	int GetBatchNum() { return (int)m_batch_list.size(); }
	int GetCurBatch() { return m_cur_batch; }

private:
	std::wstring m_data_name;

	struct SliceInfo
	{
		int slicenumber;	//slice number for sorting
		std::wstring slice;		//slice file name
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

	struct SequenceItem
	{
		std::vector<char> data;
		std::vector<SequenceItem> children; // Nested sequence items
		bool is_valid = true;     // True if parsing succeeded
		bool is_delim = false;    // True if this was a Delimitation Item (0xFFFEE0DD)
		bool is_empty = false;    // True if item had zero length
	};

	std::map<uint32_t, std::vector<char>> m_tag_map;
	bool m_valid_info;
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
	double m_min_value;
	double m_max_value;
	double m_scalar_scale;
	int m_bits;
	bool m_big_endian;
	bool m_signed;
	DCMCompression m_compression;

	bool GetFileInfo(const std::wstring& filename);
	Nrrd* ReadDcm(const std::vector<SliceInfo>& filelist, int c, bool get_max);
	bool ReadSingleDcm(void* val, const std::wstring& filename, int c);
	bool CleanPixelData(std::vector<char>& pixel_data);
	bool Decompress(std::vector<char>& pixel_data, std::vector<uint8_t>& decompressed, int c);
	SequenceItem ParseSequenceItem(std::ifstream& file);
	void DetectCompression(const std::string& uid);
};

#endif// _DCM_READER_H_