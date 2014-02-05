#ifndef _TIF_READER_H_
#define _TIF_READER_H_

#include <Formats\base_reader.h>
#include <stdio.h>
#include <windows.h>
#include <vector>
#include <tiffio.h>

using namespace std;

class TIFReader : public BaseReader
{
public:
	TIFReader();
	~TIFReader();

	void SetFile(string &file);
	void SetFile(wstring &file);
	void SetSliceSeq(bool ss);
	bool GetSliceSeq();
	void SetTimeId(wstring &id);
	wstring GetTimeId();
	void Preprocess();
	void SetBatch(bool batch);
	int LoadBatch(int index);
	int LoadOffset(int offset);
	Nrrd* Convert(bool get_max);
	Nrrd* Convert(int c, bool get_max);
	Nrrd* Convert(int t, int c, bool get_max);
	wstring GetCurName(int t, int c);

	wstring GetPathName() {return m_path_name;}
	wstring GetDataName() {return m_data_name;}
	int GetCurTime() {return m_cur_time;}
	int GetTimeNum() {return m_time_num;}
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
	wstring m_path_name;
	wstring m_data_name;

	struct SliceInfo
	{
		int slicenumber;	//slice number for sorting
		wstring slice;		//slice file name
	};
	struct TimeDataInfo
	{
		int type;	//0-single file;1-sequence
		int filenumber;	//filenumber for sorting
		vector<SliceInfo> slices;
	};
	vector<TimeDataInfo> m_4d_seq;

	//3d batch
	bool m_batch;
	vector<wstring> m_batch_list;
	int m_cur_batch;

	bool m_slice_seq;
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

	//time sequence id
	wstring m_time_id;

private:
	bool IsNewBatchFile(wstring name);
	bool IsBatchFileIdentical(wstring name1, wstring name2);

	static bool tif_sort(const TimeDataInfo& info1, const TimeDataInfo& info2);
	static bool tif_slice_sort(const SliceInfo& info1, const SliceInfo& info2);
	//read single tiff file
	Nrrd* ReadSingleTiff(wstring filename, int c, bool get_max);
	//read tiff sequence
	Nrrd* ReadSequenceTiff(vector<SliceInfo> &filelist, int c, bool get_max);
};

#endif//_TIF_READER_H_