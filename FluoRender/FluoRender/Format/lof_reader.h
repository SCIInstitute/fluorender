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
#ifndef _LOF_READER_H_
#define _LOF_READER_H_

#include <base_vol_reader.h>
#include <vector>
#include <string>
#include <limits>

#define LOFHSIZE	8
#define LOFTEST0	0x70
#define LOFTEXT1	0x2A
#define LOFSKIPSIZE	19

namespace tinyxml2
{
	class XMLElement;
}

class LOFReader : public BaseVolReader
{
public:
	LOFReader();
	~LOFReader();

	int GetType() { return READER_LOF_TYPE; }

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
	unsigned int m_datatype;//pixel type of data: 0-na; 1-8bit; 2-16bit 4-32bit

	//wavelength info
	struct WavelengthInfo
	{
		int chan_num;
		double wavelength;
	};
	std::vector<WavelengthInfo> m_excitation_wavelength_list;

	struct SubBlockInfo
	{
		int chan;//channel number
		int time;//time number
		unsigned long long loc;//position in file
		//inc for xyz
		unsigned long long x_inc;
		unsigned long long y_inc;
		unsigned long long z_inc;
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

		SubBlockInfo() :
			chan(0), time(0), loc(0),
			x_inc(0), y_inc(0), z_inc(0),
			x(0), y(0), z(0),
			x_size(0), y_size(0), z_size(0),
			x_start(0), y_start(0), z_start(0),
			x_len(0), y_len(0), z_len(0)
		{}
	};
	struct TimeInfo
	{
		int chan;//chan number
		int time;//time number
		unsigned long long inc;
		unsigned long long loc;
		std::vector<SubBlockInfo> blocks;

		TimeInfo() :
			chan(0), time(0), inc(0), loc(0)
		{}
		void SetSubBlockInfo(unsigned int dim, unsigned int empty_dim,
			unsigned int size, double orig, double len,
			unsigned long long tinc)
		{
			SubBlockInfo *sbi = 0;
			if (blocks.empty())
				blocks.resize(1);
			sbi = &(blocks[0]);

			switch (dim)
			{
			case 1:
				sbi->x_size = size;
				sbi->x_start = orig;
				sbi->x_len = len;
				sbi->x_inc = tinc;
				break;
			case 2:
				sbi->y_size = size;
				sbi->y_start = orig;
				sbi->y_len = len;
				sbi->y_inc = tinc;
				break;
			case 3:
				sbi->z_size = size;
				sbi->z_start = orig;
				sbi->z_len = len;
				sbi->z_inc = tinc;
				break;
			case 4:
				inc = tinc;
				break;
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
				if (!empty_dim)
					break;
				SetSubBlockInfo(empty_dim, 0,
					size, orig, len, tinc);
				break;
			}
		}
		void FillInfo()
		{
			for (size_t i = 0; i < blocks.size(); ++i)
			{
				blocks[i].chan = chan;
				blocks[i].time = time;
				blocks[i].loc = loc + blocks[i].z_inc * i;
				if (blocks[i].x_size == 0)
					blocks[i].x_size = 1;
				if (blocks[i].y_size == 0)
					blocks[i].y_size = 1;
				if (blocks[i].z_size == 0)
					blocks[i].z_size = 1;
			}
		}
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
		unsigned long long inc;
		unsigned long long loc;
		std::vector<TimeInfo> times;
		//lut name
		std::string lut;

		ChannelInfo() :
			chan(0), res(0), minv(0), maxv(0), loc(0)
		{}
		void FillInfo(unsigned long long pos)
		{
			loc = pos + inc;
			//populate blocks
			for (size_t i = 0; i < times.size(); ++i)
			{
				times[i].chan = chan;
				times[i].time = static_cast<int>(i);
				times[i].loc = loc + times[0].inc * i;
				times[i].blocks = times[0].blocks;
			}
			//correct block values
			for (size_t i = 0; i < times.size(); ++i)
				times[i].FillInfo();
		}
	};
	struct ImageInfo
	{
		unsigned long long loc;//location in file
		unsigned long long size;//read size
		double minv;//min value
		double maxv;//max value
		std::vector<ChannelInfo> channels;

		ImageInfo() :
			loc(0), size(0),
			minv(std::numeric_limits<double>::max()),
			maxv(0.0)
		{}
		ChannelInfo* GetChannelInfo(int chan)
		{
			if (chan >= 0 && chan < channels.size())
				return &(channels[chan]);
			return 0;
		}
		TimeInfo* GetTimeInfo(int chan, int time)
		{
			ChannelInfo* cinfo = GetChannelInfo(chan);
			if (cinfo && time >= 0 && time < cinfo->times.size())
				return &(cinfo->times[time]);
			return 0;
		}
		void FillInfo()
		{
			unsigned long long pos = loc;
			for (size_t i = 0; i < channels.size(); ++i)
			{
				if (i > 0)
					channels[i].times = channels[0].times;
				channels[i].FillInfo(pos);
			}
		}
	};
	ImageInfo m_lof_info;

	//lif properties
	int m_ver_major;//version 1 and 2 have different max data sizes
	int m_ver_minor;
	unsigned long long m_mem_loc;
	unsigned long long m_mem_size;

private:
	unsigned long long ReadMetadata(FILE* pfile, unsigned long long ioffset);
	bool ReadMemoryBlock(FILE* pfile, SubBlockInfo* sbi, void* val);
	void ReadImage(tinyxml2::XMLElement* node);
	void ReadSubBlockInfo(tinyxml2::XMLElement* node);
	void AddSubBlockInfo(unsigned int dim, unsigned int size,
		double orig, double len, unsigned long long inc);
	void FillLofInfo();
	unsigned int GetEmptyDim(ChannelInfo* cinfo, SubBlockInfo* sbi)
	{
		if (sbi->x_size == 0)
			return 1;//x empty
		else if (sbi->y_size == 0)
			return 2;//y empty
		else if (sbi->z_size == 0)
			return 3;//z empty
		else if (cinfo->times.size() == 1)
			return 4;//t empty
		return 0;//all full
	}
};

#endif//_LOF_READER_H_
