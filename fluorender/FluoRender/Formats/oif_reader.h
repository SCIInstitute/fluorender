#ifndef _OIF_READER_H_
#define _OIF_READER_H_

#include <base_reader.h>
#include <stdio.h>
//#include <windows.h>
#include <vector>
#include <fstream>
#include <iostream>

using namespace std;

class OIFReader : public BaseReader
{
public:
	OIFReader();
	~OIFReader();

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
	wstring m_subdir_name;

	int m_type;	//0-time data in a single file; 1-time data in a file sequence
	typedef vector<wstring> ChannelInfo;	//slices form a channel
	typedef vector<ChannelInfo> DatasetInfo;//channels form dataset
	struct TimeDataInfo
	{
		int filenumber;		//if type is 1, file number for time data
		wstring filename;	//if type is 1, file name for current time data
		wstring subdirname;	//subdirectory name
		DatasetInfo dataset;//a list of the channels
	};
	vector<TimeDataInfo> m_oif_info;		//time data form the complete oif dataset
	int m_oif_t;	//current time point in oib info for reading

	int m_time_num;
	int m_cur_time;
	int m_chan_num;
	struct WavelengthInfo
	{
		int chan_num;
		double wavelength;
	};
	vector<WavelengthInfo> m_excitation_wavelength_list;
	int m_slice_num;
	int m_x_size;
	int m_y_size;
	bool m_valid_spc;
	double m_xspc;
	double m_yspc;
	double m_zspc;
	double m_max_value;
	double m_scalar_scale;

private:
	static bool oif_sort(const TimeDataInfo& info1, const TimeDataInfo& info2);
	void ReadSingleOif();
	void ReadSequenceOif();
	void ReadTifSequence(wstring file_name);
	void ReadOif();
	void ReadOifLine(wstring oneline);
	void ReadTiff(char* pbyData, unsigned short *val, int z);

	//axis count
	int axis_num;
	int cur_axis;
	//channel count
	int chan_num;
	int cur_chan;
	//axis info
	wstring axis_code;
	wstring pix_unit;
	wstring max_size;
	wstring start_pos;
	wstring end_pos;

	//time sequence id
	wstring m_time_id;
};

#endif//_OIF_READER_H_
