#ifndef _NRRD_READER_H_
#define _NRRD_READER_H_

#include <base_reader.h>
#include <stdio.h>
//#include <windows.h>
#include <vector>

using namespace std;

class NRRDReader : public BaseReader
{
public:
	NRRDReader();
	~NRRDReader();

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
	double GetExcitationWavelength(int chan) {return 0.0;}
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

	//4d sequence
	struct TimeDataInfo
	{
		int filenumber;
		wstring filename;
	};
	vector<TimeDataInfo> m_4d_seq;
	int m_cur_time;

	int m_time_num;
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

	//time sequence id
	wstring m_time_id;

private:
	static bool nrrd_sort(const TimeDataInfo& info1, const TimeDataInfo& info2);
};

#endif//_NRRD_READER_H_
