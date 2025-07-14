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
#ifndef _MPG_READER_H_
#define _MPG_READER_H_

#include <base_reader.h>
#include <unordered_map>
#include <list>

struct AVFormatContext;
struct AVCodecContext;
struct SwsContext;
struct AVFrame;

class MPGReader : public BaseReader
{
public:
	MPGReader();
	~MPGReader();

	int GetType() { return READER_MPG_TYPE; }

	//void SetFile(const std::string &file);
	void SetFile(const std::wstring &file);
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
	double GetExcitationWavelength(int chan);
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

	struct FrameInfo
	{
		int64_t pts;
		int64_t dts;
	};
	std::vector<FrameInfo> m_mpg_info;

	//ffmpeg structs
	int m_stream_index;
	AVFormatContext* m_av_format_context;
	AVCodecContext* m_av_codec_context;
	SwsContext* m_sws_context;
	//frame cache
	AVFrame* m_frame_yuv;//frame cache, should all be the same size
	AVFrame* m_frame_rgb;//frame cache
	std::unordered_map<int, AVFrame*> m_frame_cache; // Frame cache
	std::list<int> m_cache_order; // Order of access for LRU eviction
	int m_cache_size_limit = -1; // -1: infinity; 0: disable; >0: size
	uint8_t* m_frame_buffer;

private:
	FrameInfo get_frame_info(int64_t dts, int64_t pts);
	Nrrd* get_nrrd(AVFrame* frame, int c);
	void add_cache(int t, AVFrame* frame);
	void invalidate_cache();
	void release_cache();
};

#endif//_MPG_READER_H_
