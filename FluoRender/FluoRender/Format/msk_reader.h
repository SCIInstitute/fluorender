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
#ifndef _MSK_READER_H_
#define _MSK_READER_H_

#include <base_vol_reader.h>

class MSKReader : public BaseVolReader
{
public:
	MSKReader();
	~MSKReader();

	int GetType() { return READER_MSK_TYPE; }

	//void SetFile(const std::string &file);
	void SetFile(const std::wstring &file);
	int Preprocess();
	void SetBatch(bool batch);
	int LoadBatch(int index);
	Nrrd* Convert(int t, int c, bool get_max);
	std::wstring GetCurDataName(int t, int c);
	std::wstring GetCurMaskName(int t, int c);
	std::wstring GetCurLabelName(int t, int c);

	std::wstring GetPathName() {return m_path_name;}
	std::wstring GetDataName() {return L"";}
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
	double GetMinValue() { return 0.0; }
	double GetMaxValue() {return 0.0;}
	double GetScalarScale() {return 0.0;}
	bool GetBatch() {return false;}
	int GetBatchNum() {return 0;}
	int GetCurBatch() {return 0;}
};

#endif//_MSK_READER_H_
