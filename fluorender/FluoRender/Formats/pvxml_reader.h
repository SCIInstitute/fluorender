#ifndef _PVXML_READER_H_
#define _PVXML_READER_H_

#include <vector>
#include <base_reader.h>

using namespace std;
class wxXmlNode;

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
	int LoadOffset(int offset);
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

	//flipping
	void SetFlipX(int flip) {m_user_flip_x = flip;}
	void SetFlipY(int flip) {m_user_flip_y = flip;}
private:
	wstring m_path_name;
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

	double m_x_min, m_y_min, m_z_min;
	double m_x_max, m_y_max, m_z_max;
	int m_seq_slice_num;
	double m_seq_zspc;
	double m_seq_zpos;

	int m_time_num;
	int m_cur_time;
	int m_chan_num;

	//3d batch
	bool m_batch;
	vector<wstring> m_batch_list;
	int m_cur_batch;

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
	void ReadSystemConfig(wxXmlNode *systemNode);
	void UpdateStateShard(wxXmlNode *stateNode);
	void ReadKey(wxXmlNode *keyNode);
	void ReadSequence(wxXmlNode *seqNode);
	void ReadFrame(wxXmlNode *frameNode);
	void ReadTiff(char* pbyData, unsigned short *val);
};

#endif//_PVXML_READER_H_
