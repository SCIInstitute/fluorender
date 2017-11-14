#ifndef _BRKXML_READER_H_
#define _BRKXML_READER_H_

#include <vector>
#include <base_reader.h>
#include <FLIVR/TextureBrick.h>
#include <tinyxml2.h>

using namespace std;

class BRKXMLReader : public BaseReader
{
public:
	BRKXMLReader();
	~BRKXMLReader();

	void SetFile(string &file);
	void SetFile(wstring &file);
	void SetDir(string &dir);
	void SetDir(wstring &dir);
	void SetSliceSeq(bool ss);
	bool GetSliceSeq();
	void SetTimeSeq(bool ss);
	bool GetTimeSeq();
	void SetTimeId(wstring &id);
	void SetCurTime(int t);
	void SetCurChan(int c);
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
	int GetCurChan() {return m_cur_chan;}
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
	bool GetBatch() {return m_batch;}//not base_reader
	int GetBatchNum() {return (int)m_batch_list.size();}
	int GetCurBatch() {return m_cur_batch;}
	bool isURL() {return m_isURL;}
	wstring GetExMetadataURL() {return m_ex_metadata_url;}
	void SetInfo();

	FLIVR::FileLocInfo* GetBrickFilePath(int fr, int ch, int id, int lv = -1);
	wstring GetBrickFileName(int fr, int ch, int id, int lv = -1);
	int GetFileType(int lv = -1);

	int GetLevelNum() {return m_level_num;}
	void SetLevel(int lv);
	int GetCopyableLevel() {return m_copy_lv;}

	void build_bricks(vector<FLIVR::TextureBrick*> &tbrks, int lv = -1);
	void build_pyramid(vector<FLIVR::Pyramid_Level> &pyramid, vector<vector<vector<vector<FLIVR::FileLocInfo *>>>> &filenames, int t, int c);
	void OutputInfo();

	void GetLandmark(int index, wstring &name, double &x, double &y, double &z, double &spcx, double &spcy, double &spcz);
	int GetLandmarkNum() {return m_landmarks.size();}
	wstring GetROITree() {return m_roi_tree;}
	void GetMetadataID(wstring &mid) {mid = m_metadata_id;}

	bool loadMetadata(const wstring &file = wstring());
	void LoadROITree(tinyxml2::XMLElement *lvNode);
	void LoadROITree_r(tinyxml2::XMLElement *lvNode, wstring& tree, const wstring& parent, int& gid);

	tinyxml2::XMLDocument *GetVVDXMLDoc() {return &m_doc;}
	tinyxml2::XMLDocument *GetMetadataXMLDoc() {return &m_md_doc;}

private:
	wstring m_path_name;
	wstring m_data_name;
	wstring m_dir_name;
	wstring m_url_dir;
	wstring m_ex_metadata_path;
	wstring m_ex_metadata_url;
	wstring m_roi_tree;

	struct BrickInfo
	{
		//index
		int id;
		//size
		int x_size;
		int y_size;
		int z_size;
		//start position
		int x_start;
		int y_start;
		int z_start;
		//offset to brick
		long long offset;
		long long fsize;
		//tbox
		double tx0, ty0, tz0, tx1, ty1, tz1;
		//bbox
		double bx0, by0, bz0, bx1, by1, bz1;
	};
	struct LevelInfo
	{
		int imageW;
		int imageH;
		int imageD;
		double xspc;
		double yspc;
		double zspc;
		int brick_baseW;
		int brick_baseH;
		int brick_baseD;
		int bit_depth;
		int file_type;
		vector<vector<vector<FLIVR::FileLocInfo *>>> filename;//Frame->Channel->BrickID->Filename
		vector<BrickInfo *> bricks;
	};
	vector<LevelInfo> m_pyramid;

	
	struct ImageInfo
	{
		int nChannel;
		int nFrame;
		int nLevel;
		int copyableLv;
	};
	ImageInfo m_imageinfo;

	int m_file_type;

	int m_level_num;
	int m_cur_level;
	int m_copy_lv;
	
	int m_time_num;
	int m_cur_time;
	int m_chan_num;
	int m_cur_chan;

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

	bool m_isURL;

	//time sequence id
	wstring m_time_id;

	tinyxml2::XMLDocument m_doc;
	tinyxml2::XMLDocument m_md_doc;

	struct Landmark
	{
		wstring name;
		double x, y, z;
		double spcx, spcy, spcz;
	};
	vector<Landmark> m_landmarks;
	wstring m_metadata_id;

private:
	ImageInfo ReadImageInfo(tinyxml2::XMLElement *seqNode);
	void ReadBrick(tinyxml2::XMLElement *brickNode, BrickInfo &binfo);
	void ReadLevel(tinyxml2::XMLElement* lvNode, LevelInfo &lvinfo);
	void ReadFilenames(tinyxml2::XMLElement* fileRootNode, vector<vector<vector<FLIVR::FileLocInfo *>>> &filename);
	void ReadPackedBricks(tinyxml2::XMLElement* packNode, vector<BrickInfo *> &brks);
	void Readbox(tinyxml2::XMLElement *boxNode, double &x0, double &y0, double &z0, double &x1, double &y1, double &z1);
	void ReadPyramid(tinyxml2::XMLElement *lvRootNode, vector<LevelInfo> &pylamid);

	void Clear();
};

#endif//_BRKXML_READER_H_
