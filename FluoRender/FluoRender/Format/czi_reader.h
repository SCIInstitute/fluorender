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
#ifndef _CZI_READER_H_
#define _CZI_READER_H_

#include <base_reader.h>
#include <vector>
#include <string>
#include <limits>

#define HDRSIZE	32//header size
#define FIXSIZE	256//fixed part size

namespace tinyxml2
{
	class XMLElement;
}

class CZIReader : public BaseReader
{
public:
	CZIReader();
	~CZIReader();

	int GetType() { return READER_CZI_TYPE; }

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

	struct SubBlockInfo
	{
		unsigned int dirpos;
		int chan;//channel number
		int time;//time
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
		//location in file
		unsigned long long loc;
		//store size
		unsigned int size;
		//compression
		unsigned int compress;
		//pixel type
		unsigned int pxtype;

		SubBlockInfo():
			dirpos(0),
			chan(0),
			time(0),
			x(0),
			y(0),
			z(0),
			x_size(1),
			y_size(1),
			z_size(1),
			x_start(0),
			y_start(0),
			z_start(0),
			loc(0),
			size(0),
			compress(0),
			pxtype(0)
		{}
	};
	struct ChannelInfo
	{
		int chan;//channel number
		std::vector<SubBlockInfo> blocks;
	};
	struct TimeInfo
	{
		int time;//time number
		std::vector<ChannelInfo> channels;
	};
	struct CZIInfo
	{
		int xmin, ymin, zmin;
		int xmax, ymax, zmax;
		std::vector<TimeInfo> times;

		void init()
		{
			xmin = ymin = zmin = std::numeric_limits<int>::max();
			xmax = ymax = zmax = std::numeric_limits<int>::min();
			times.clear();
		}
		void xsize(int x0, int x1)
		{
			xmin = std::min(xmin, std::min(x0, x1));
			xmax = std::max(xmax, std::max(x0, x1));
		}
		void ysize(int y0, int y1)
		{
			ymin = std::min(ymin, std::min(y0, y1));
			ymax = std::max(ymax, std::max(y0, y1));
		}
		void zsize(int z0, int z1)
		{
			zmin = std::min(zmin, std::min(z0, z1));
			zmax = std::max(zmax, std::max(z0, z1));
		}
	};
	CZIInfo m_czi_info;

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

	typedef enum
	{
		SegFile = 0,
		SegDirectory,
		SegSubBlock,
		SegMetadata,
		SegAttach,
		SegAttDir,
		SegDeleted,
		SegAll
	} SegType;

	static std::vector<std::string> m_types;
	bool m_header_read;
	bool m_multi_file;
	unsigned int m_file_part;
	unsigned long long m_dir_pos;//directory segment position
	unsigned long long m_meta_pos;//metadata segment position
	unsigned long long m_att_dir;//attachment directory position

private:
	//segment reader
	bool ReadSegment(FILE* pfile, unsigned long long &ioffset, SegType type = SegAll);
	bool ReadFile(FILE* pfile);
	unsigned int ReadDirectoryEntry(FILE* pfile);//return size
	bool ReadDirectory(FILE* pfile, unsigned long long ioffset);
	bool ReadSubBlock(FILE* pfile, unsigned long long ioffset);
	bool ReadMetadata(FILE* pfile, unsigned long long ioffset);
	bool ReadAttach(FILE* pfile);
	bool ReadAttDir(FILE* pfile);
	bool ReadDeleted(FILE* pfile);
	//read info
	TimeInfo* GetTimeinfo(int time)
	{
		for (size_t i = 0; i < m_czi_info.times.size(); ++i)
		{
			if (m_czi_info.times[i].time == time)
				return &(m_czi_info.times[i]);
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
	//read data
	bool ReadSegSubBlock(FILE* pfile, SubBlockInfo* sbi, void* val);
	//get min max
	void GetMinMax16(unsigned short* val, unsigned long long px,
		unsigned short &minv, unsigned short &maxv);
	void GetMinMax16B(unsigned short* val, int nx, int ny, int nz, int sx, int sy,
		unsigned short &minv, unsigned short &maxv);
	//search metadata
	void FindNodeRecursive(tinyxml2::XMLElement* node);
};

#endif//_CZI_READER_H_
