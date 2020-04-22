/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2020 Scientific Computing and Imaging Institute,
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
#ifndef _ND2_READER_H_
#define _ND2_READER_H_

#include <Nd2ReadSdk.h>
#include <base_reader.h>
#include <vector>
#include <string>
#include <limits>

using namespace std;

#define ND2_STR_SIZE	1024

class ND2Reader : public BaseReader
{
public:
	ND2Reader();
	~ND2Reader();

	int GetType() { return READER_ND2_TYPE; }

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
	void SetBatch(bool batch);
	int LoadBatch(int index);
	Nrrd* Convert(int t, int c, bool get_max);
	wstring GetCurDataName(int t, int c);
	wstring GetCurMaskName(int t, int c);
	wstring GetCurLabelName(int t, int c);

	wstring GetPathName() {return m_path_name;}
	wstring GetDataName() {return m_data_name;}
	int GetTimeNum() {return m_time_num;}
	int GetCurTime() {return m_cur_time;}
	int GetChanNum() {return m_chan_num;}
	double GetExcitationWavelength(int chan);
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

	struct FrameInfo
	{
		int chan;//channel number
		int time;//time number
		int slice;//z stack slice number
		int seq;//sequence number
	};
	struct ChannelInfo
	{
		int chan;//channel number
		std::vector<FrameInfo> chann;//single channel of z slices
	};
	struct TimeInfo
	{
		int time;//time number
		std::vector<ChannelInfo> channels;//single time point of channels
	};
	struct ND2Info
	{
		std::vector<TimeInfo> times;
	};
	ND2Info m_nd2_info;

	int m_bits;
	int m_bits_used;
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

	//wavelength info
	struct WavelengthInfo
	{
		int chan_num;
		double wavelength;
	};
	vector<WavelengthInfo> m_excitation_wavelength_list;

private:
	void AddFrameInfo(FrameInfo &frame);
	ChannelInfo* GetChaninfo(int t, int c)
	{
		if (t >= 0 && t < m_nd2_info.times.size())
		{
			TimeInfo* tinfo = &(m_nd2_info.times[t]);
			if (tinfo->channels.size() > 0)
				return &(tinfo->channels[0]);
		}
		return 0;
	}
	bool ReadChannel(LIMFILEHANDLE h, int t, int c, void* val);
};

#endif//_ND2_READER_H_
