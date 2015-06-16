/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2014 Scientific Computing and Imaging Institute,
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
#ifndef _PVXML_READER_H_
#define _PVXML_READER_H_

#include <vector>
#include <base_reader.h>

using namespace std;
class wxXmlNode;
class wxString;

class PVXMLReader : public BaseReader
{
public:
	PVXMLReader();
	~PVXMLReader();

	void SetFile(string &file);
	void SetFile(wstring &file);
	void SetSliceSeq(bool ss);
	bool GetSliceSeq();
	void SetTimeId(wstring &id);
	wstring GetTimeId();
	void Preprocess();
	void SetBatch(bool batch);
	int LoadBatch(int index);
	Nrrd* Convert(int t, int c, bool get_max);
	wstring GetCurName(int t, int c);

	wstring GetPathName() {return m_path_name;}
	wstring GetDataName() {return m_data_name;}
	int GetTimeNum() {return m_time_num;}
	int GetCurTime() {return m_cur_time;}
	int GetChanNum()
	{if (m_sep_seq) return m_group_num; else return m_chan_num;}
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

	//flipping
	void SetFlipX(int flip) {m_user_flip_x = flip;}
	void SetFlipY(int flip) {m_user_flip_y = flip;}
private:
	wstring m_data_name;

	struct ChannelInfo
	{
		wstring file_name;
	};
	struct FrameInfo
	{
		//corner index
		int x;
		int y;
		int z;
		//size
		int x_size;
		int y_size;
		//start position
		double x_start;
		double y_start;
		double z_start;
		vector<ChannelInfo> channels;
	};
	struct SequenceInfo
	{
		int grid_index;
		bool apart;
		vector<FrameInfo> frames;
	};
	typedef vector<SequenceInfo> TimeDataInfo;
	vector<TimeDataInfo> m_pvxml_info;

	//struct for PVStateShard
	struct StateShard
	{
		int grid_index;
		int grid_index_x;
		int grid_index_y;
		double pos_x;
		double pos_y;
		double pos_z;
		//usually won't change
		int z_device;
		int ppl;//pixels per line
		int lpf;//lines per frame
		double mpp_x;//microns per pixel x
		double mpp_y;//microns per pixel y
		int bit_depth;
	};
	StateShard m_current_state;
	vector<StateShard> m_state_shard_stack;

	//struct for sequence bbox
	struct SeqBox
	{
		double x_min;
		double x_max;
		double y_min;
		double y_max;

		double overlaps(const SeqBox& sb) const
		{
			double x_apart = 1.0;
			double y_apart = 1.0;
			double ix_min = sb.x_min>x_min?sb.x_min:x_min;
			double ix_max = sb.x_max<x_max?sb.x_max:x_max;
			if (ix_min >= ix_max)
				x_apart = (ix_max-ix_min)/(x_max-x_min);
			double iy_min = sb.y_min>y_min?sb.y_min:y_min;
			double iy_max = sb.y_max<y_max?sb.y_max:y_max;
			if (iy_min >= iy_max)
				y_apart = (iy_max-iy_min)/(y_max-y_min);
			if (x_apart<0.0 || y_apart<0.0)
				return x_apart<y_apart?x_apart:y_apart;
			else
				return (ix_max-ix_min)*(iy_max-iy_min)/(x_max-x_min)/(y_max-y_min);
		}
		void extend(const SeqBox& sb)
		{
			x_min = sb.x_min<x_min?sb.x_min:x_min;
			x_max = sb.x_max>x_max?sb.x_max:x_max;
			y_min = sb.y_min<y_min?sb.y_min:y_min;
			y_max = sb.y_max>y_max?sb.y_max:y_max;
		}
	};
	vector<SeqBox> m_seq_boxes;
	bool m_new_seq;//starts a new sequence

	double m_x_min, m_y_min, m_z_min;
	double m_x_max, m_y_max, m_z_max;
	int m_seq_slice_num;
	double m_seq_zspc;
	double m_seq_zpos;

	int m_time_num;
	int m_cur_time;
	int m_chan_num;

	int m_group_num;
	bool m_sep_seq;//non-overlapping sequences

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

	//user setting for flipping
	//0:auto; -1:flip; 1:no flip
	int m_user_flip_x;
	int m_user_flip_y;
	//actual flags for flipping
	bool m_flip_x;
	bool m_flip_y;

private:
	bool ConvertS(int c, TimeDataInfo* time_data_info, unsigned short *val);
	bool ConvertN(int c, TimeDataInfo* time_data_info, unsigned short *val);
	void ReadSystemConfig(wxXmlNode *systemNode);
	void UpdateStateShard(wxXmlNode *stateNode);
	void ReadKey(wxXmlNode *keyNode);
	void ReadIndexedKey(wxXmlNode *keyNode, wxString &key);
	void ReadSequence(wxXmlNode *seqNode);
	void ReadFrame(wxXmlNode *frameNode);
	void ReadTiff(char* pbyData, unsigned short *val);
};

#endif//_PVXML_READER_H_
