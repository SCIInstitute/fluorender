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
