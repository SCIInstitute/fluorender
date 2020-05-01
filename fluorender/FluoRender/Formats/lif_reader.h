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
#ifndef _LIF_READER_H_
#define _LIF_READER_H_

#include <base_reader.h>
#include <wx/xml/xml.h>
#include <vector>
#include <string>
#include <unordered_map>

using namespace std;

#define LIFHSIZE	8
#define LIFTEST0	0x70
#define LIFTEXT1	0x2A

class LIFReader : public BaseReader
{
public:
	LIFReader();
	~LIFReader();

	int GetType() { return READER_LIF_TYPE; }

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

	wstring GetPathName() { return m_path_name; }
	wstring GetDataName() { return m_data_name; }
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
	double GetMaxValue() { return m_max_value; }
	double GetScalarScale() { return m_scalar_scale; }
	bool GetBatch() { return m_batch; }
	int GetBatchNum() { return (int)m_batch_list.size(); }
	int GetCurBatch() { return m_cur_batch; }

private:
	wstring m_data_name;

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
	unsigned int m_datatype;//pixel type of data: 0-na; 1-8bit; 2-16bit 4-32bit

	//wavelength info
	struct WavelengthInfo
	{
		int chan_num;
		double wavelength;
	};
	vector<WavelengthInfo> m_excitation_wavelength_list;

	struct SubBlockInfo
	{
		std::wstring name;
		unsigned long long loc;//position in file
		unsigned long long size;//size to read
		int chan;//channel number
		int time;//time number
		//corner
		int x;
		int y;
		int z;
		//size
		int x_size;
		int y_size;
		int z_size;
		//position
		double x_start;
		double y_start;
		double z_start;
		double x_len;
		double y_len;
		double z_len;

		SubBlockInfo():
			loc(0), size(0), chan(0), time(0),
			x(0), y(0), z(0), x_size(0), y_size(0), z_size(0),
			x_start(0), y_start(0), z_start(0), x_len(0), y_len(0), z_len(0)
		{}
	};
	struct TimeInfo
	{
		int time;//time number
		std::vector<SubBlockInfo> blocks;
	};
	struct ChannelInfo
	{
		int chan;//channel number
		//bits
		int res;
		//min max
		double minv;
		double maxv;
		//loc from first channel
		unsigned long long loc;
		std::vector<TimeInfo> times;

		ChannelInfo():
			chan(0), res(0), minv(0), maxv(0), loc(0)
		{}
	};
	struct ImageInfo
	{
		std::wstring name;//image name for batch
		std::wstring mbid;//memory block name
		std::vector<ChannelInfo> channels;
	};
	struct LIFInfo
	{
		std::unordered_map<std::wstring, ImageInfo> images;
	};
	LIFInfo m_lif_info;

	//lif properties
	int m_version;//version 1 and 2 have different max data sizes

private:
	unsigned long long ReadMetadata(FILE* pfile, unsigned long long ioffset);
	unsigned long long PreReadMemoryBlock(FILE* pfile, unsigned long long ioffset);
	bool ReadMemoryBlock(FILE* pfile, SubBlockInfo* sbi, void* val);
	void ReadElement(wxXmlNode* node);
	void ReadData(wxXmlNode* node, std::wstring &name);
	ImageInfo* ReadImage(wxXmlNode* node, std::wstring &name);
	void ReadSubBlockInfo(wxXmlNode* node, ImageInfo &imgi);
	void AddSubBlockInfo(ImageInfo &imgi, unsigned int dim, unsigned int size,
		double orig, double len, unsigned long long inc);
	SubBlockInfo* FindSubBlockInfo(std::wstring &name)
	{
		for (size_t i = 0; i < m_lif_info.times.size(); ++i)
		for (size_t j = 0; j < m_lif_info.times[i].channels.size(); ++j)
		for (size_t k = 0; k < m_lif_info.times[i].channels[j].blocks.size(); ++k)
		{
			if (m_lif_info.times[i].channels[j].blocks[k].name == name)
				return &(m_lif_info.times[i].channels[j].blocks[k]);
		}
		return 0;
	}
	//read info
	TimeInfo* GetTimeinfo(int time)
	{
		for (size_t i = 0; i < m_lif_info.times.size(); ++i)
		{
			if (m_lif_info.times[i].time == time)
				return &(m_lif_info.times[i]);
		}
		return 0;
	}
	ChannelInfo* GetChaninfo(TimeInfo* seqinfo, int chan)
	{
		if (!seqinfo)
			return 0;
		for (size_t i = 0; i < seqinfo->channels.size(); ++i)
		{
			if (seqinfo->channels[i].chan == chan)
				return &(seqinfo->channels[i]);
		}
		return 0;
	}
	ChannelInfo* GetChaninfo(int time, int chan)
	{
		TimeInfo* seqinfo = GetTimeinfo(time);
		return GetChaninfo(seqinfo, chan);
	}
};

#endif//_LIF_READER_H_
