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
		//bits
		int res;
		//min max
		double minv;
		double maxv;

		SubBlockInfo():
			loc(0), size(0), chan(0), time(0),
			x(0), y(0), z(0), x_size(0), y_size(0), z_size(0),
			x_start(0), y_start(0), z_start(0), x_len(0), y_len(0), z_len(0),
			res(0), minv(0), maxv(0)
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
	struct LIFInfo
	{
		int xmin, ymin, zmin;
		int xmax, ymax, zmax;
		std::vector<TimeInfo> times;
	};
	LIFInfo m_lif_info;

	//lif properties
	int m_version;//version 1 and 2 have different max data sizes

private:
	unsigned long long ReadMetadata(FILE* pfile, unsigned long long ioffset);
	unsigned long long PreReadMemoryBlock(FILE* pfile, unsigned long long ioffset);
	bool ReadMemoryBlock(FILE* pfile, SubBlockInfo* sbi, void* val);
	void ReadElement(wxXmlNode* node);
	void ReadData(wxXmlNode* node);
	SubBlockInfo* ReadImage(wxXmlNode* node)
	{
		if (!node)
			return 0;
		wxString str;
		wxXmlNode* child = node->GetChildren();
		if (!child || child->GetName() != "Image")
			return 0;
		SubBlockInfo sbi;
		ReadSubBlockInfo(child, sbi);
		return AddSubBlockInfo(sbi);
	}
	void ReadSubBlockInfo(wxXmlNode* node, SubBlockInfo &sbi)
	{
		if (!node)
			return;
		wxString str;
		unsigned long ulval;
		double dval;
		wxXmlNode *child = node->GetChildren();
		while (child)
		{
			str = child->GetName();
			if (str == "ChannelDescription")
			{
				str = child->GetAttribute("Resolution");
				if (str.ToULong(&ulval))
					sbi.res = ulval;
				str = child->GetAttribute("Min");
				if (str.ToDouble(&dval))
					sbi.minv = dval;
				str = child->GetAttribute("Max");
				if (str.ToDouble(&dval))
					sbi.maxv = dval;
			}
			else if (str == "DimensionDescription")
			{
				unsigned long did;
				str = child->GetAttribute("DimID");
				if (str.ToULong(&did))
				{
					str = child->GetAttribute("Unit");
					double sfactor = 1.0;
					if (str == "m")
						sfactor = 1e6;
					switch (did)
					{
					case 1:
						str = child->GetAttribute("NumberOfElements");
						if (str.ToULong(&ulval))
							sbi.x_size = ulval;
						str = child->GetAttribute("Origin");
						if (str.ToDouble(&dval))
							sbi.x_start = dval * sfactor;
						str = child->GetAttribute("Length");
						if (str.ToDouble(&dval))
							sbi.x_len = dval * sfactor;
						break;
					case 2:
						str = child->GetAttribute("NumberOfElements");
						if (str.ToULong(&ulval))
							sbi.y_size = ulval;
						str = child->GetAttribute("Origin");
						if (str.ToDouble(&dval))
							sbi.y_start = dval * sfactor;
						str = child->GetAttribute("Length");
						if (str.ToDouble(&dval))
							sbi.y_len = dval * sfactor;
						break;
					case 3:
						str = child->GetAttribute("NumberOfElements");
						if (str.ToULong(&ulval))
							sbi.z_size = ulval;
						str = child->GetAttribute("Origin");
						if (str.ToDouble(&dval))
							sbi.z_start = dval * sfactor;
						str = child->GetAttribute("Length");
						if (str.ToDouble(&dval))
							sbi.z_len = dval * sfactor;
						break;
					case 4:
						break;
					}
				}
			}
			ReadSubBlockInfo(child, sbi);
			child = child->GetNext();
		}
	}
	SubBlockInfo* AddSubBlockInfo(SubBlockInfo &sbi)
	{
		if (m_lif_info.times.size() <= sbi.time)
			m_lif_info.times.resize(sbi.time + 1);
		TimeInfo* timeinfo = &(m_lif_info.times[sbi.time]);
		if (!timeinfo)
			return 0;
		if (timeinfo->channels.size() <= sbi.chan)
			timeinfo->channels.resize(sbi.chan + 1);
		ChannelInfo* chaninfo = &(timeinfo->channels[sbi.chan]);
		if (!chaninfo)
			return 0;
		chaninfo->blocks.push_back(sbi);
		return &(chaninfo->blocks.back());
	}
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
