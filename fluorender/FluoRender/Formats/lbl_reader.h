#ifndef _LBL_READER_H_
#define _LBL_READER_H_

#include <base_reader.h>

using namespace std;

class LBLReader : public BaseReader
{
public:
	LBLReader();
	~LBLReader();

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
	wstring GetDataName() {return L"";}
	int GetTimeNum() {return 0;}
	int GetCurTime() {return 0;}
	int GetChanNum() {return 0;}
	double GetExcitationWavelength(int chan) {return 0.0;}
	int GetSliceNum() {return 0;}
	int GetXSize() {return 0;}
	int GetYSize() {return 0;}
	bool IsSpcInfoValid() {return false;}
	double GetXSpc() {return 0.0;}
	double GetYSpc() {return 0.0;}
	double GetZSpc() {return 0.0;}
	double GetMaxValue() {return 0.0;}
	double GetScalarScale() {return 0.0;}
	bool GetBatch() {return false;}
	int GetBatchNum() {return 0;}
	int GetCurBatch() {return 0;}
};

#endif//_LBL_READER_H_
