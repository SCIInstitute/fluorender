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
#ifndef _LIF_READER_H_
#define _LIF_READER_H_

#include <base_vol_reader.h>
#include <Vector.h>
#include <vector>
#include <string>
#include <limits>

#define LIFHSIZE	8
#define LIFTEST0	0x70
#define LIFTEXT1	0x2A

namespace tinyxml2
{
	class XMLElement;
}

class LIFReader : public BaseVolReader
{
public:
	LIFReader();
	~LIFReader();

	int GetType() { return READER_LIF_TYPE; }

	//void SetFile(const std::string &file);
	void SetFile(const std::wstring &file);
	int Preprocess();
	void SetBatch(bool batch);
	int LoadBatch(int index);
	int LoadOffset(int offset);
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
	fluo::Vector GetResolution() { return m_size; }
	bool IsSpcInfoValid() { return m_valid_spc; }
	fluo::Vector GetSpacing() { return m_spacing; }
	double GetMinValue() { return m_min_value; }
	double GetMaxValue() { return m_max_value; }
	double GetScalarScale() { return m_scalar_scale; }
	bool GetBatch() { return m_batch; }
	int GetBatchNum() { return (int)m_lif_info.images.size(); }
	int GetCurBatch() { return m_cur_batch; }

private:
	std::wstring m_data_name;

	int m_time_num;
	int m_cur_time;
	int m_chan_num;
	fluo::Vector m_size;
	bool m_valid_spc;
	fluo::Vector m_spacing;
	double m_min_value;
	double m_max_value;
	double m_scalar_scale;
	unsigned int m_datatype;//pixel type of data: 0-na; 1-8bit; 2-16bit 4-32bit
	bool m_tile_scan;

	//wavelength info
	struct WavelengthInfo
	{
		int chan_num;
		double wavelength;
	};
	std::vector<WavelengthInfo> m_excitation_wavelength_list;

	struct TileScanInfo
	{
		int fieldx;
		int fieldy;
		int fieldz;
		double posx;
		double posy;
		double posz;

		TileScanInfo():
			fieldx(0), fieldy(0), fieldz(0),
			posx(0), posy(0), posz(0)
		{}
	};
	typedef std::vector<TileScanInfo> TileList;
	struct SubBlockInfo
	{
		int chan;//channel number
		int time;//time number
		unsigned long long inc;//inc for block
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

		SubBlockInfo():
			chan(0), time(0),
			inc(0), loc(0),
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
			case 0:
				break;
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
			case 10:
				sbi->inc = tinc;
				break;
			default:
				if (!empty_dim)
					break;
				SetSubBlockInfo(empty_dim, 0,
					size, orig, 0.0, tinc);
				break;
			}
		}
		void FillInfo()
		{
			for (size_t i = 0; i < blocks.size(); ++i)
			{
				blocks[i].chan = chan;
				blocks[i].time = time;
				if (blocks[i].z_inc)
				{
					blocks[i].loc = loc + blocks[i].z_inc * i;
					if (blocks[i].x_size == 0)
						blocks[i].x_size = 1;
					if (blocks[i].y_size == 0)
						blocks[i].y_size = 1;
					if (blocks[i].z_size == 0)
						blocks[i].z_size = 1;
				}
				else if (blocks[0].inc)
				{
					//tiled scan
					blocks[i] = blocks[0];
					blocks[i].loc = loc + blocks[0].inc * i;
				}
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

		ChannelInfo():
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
		std::wstring name;//image name for batch
		std::wstring mbid;//memory block name
		unsigned long long loc;//location in file
		unsigned long long size;//read size
		double minv;//min value
		double maxv;//max value
		std::vector<ChannelInfo> channels;
		TileList tile_list;

		ImageInfo():
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
	struct LIFInfo
	{
		std::vector<ImageInfo> images;
	};
	LIFInfo m_lif_info;

	//lif properties
	int m_version;//version 1 and 2 have different max data sizes

private:
	unsigned long long ReadMetadata(FILE* pfile, unsigned long long ioffset);
	unsigned long long PreReadMemoryBlock(FILE* pfile, unsigned long long ioffset);
	bool ReadMemoryBlock(FILE* pfile, SubBlockInfo* sbi, void* val);
	bool CopyMemoryBlock(FILE* pfile, SubBlockInfo* sbi, void* val);
	void ReadElement(tinyxml2::XMLElement* node);
	void ReadData(tinyxml2::XMLElement* node, std::wstring &name);
	ImageInfo* ReadImage(tinyxml2::XMLElement* node, std::wstring &name);
	void ReadSubBlockInfo(tinyxml2::XMLElement* node, ImageInfo &imgi);
	void AddSubBlockInfo(ImageInfo &imgi, unsigned int dim, unsigned int size,
		double orig, double len, unsigned long long inc);
	bool ReadTileScanInfo(tinyxml2::XMLElement* node, TileList& list);
	void GenImageInfo(ImageInfo* imgi);

	ImageInfo* FindImageInfoMbid(std::wstring &mbid)
	{
		for (auto it = m_lif_info.images.begin();
			it != m_lif_info.images.end(); ++it)
		{
			if ((*it).mbid.compare(mbid) == 0)
				return &(*it);
		}
		return 0;
	}
	void FillLifInfo();
	TimeInfo* GetTimeInfo(int c, int t)
	{
		if (m_cur_batch < 0 || m_cur_batch >= m_lif_info.images.size())
			return 0;
		return m_lif_info.images[m_cur_batch].GetTimeInfo(c, t);
	}
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

#endif//_LIF_READER_H_
