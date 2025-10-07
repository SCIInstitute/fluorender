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
#ifndef _BASE_MESH_READER_H_
#define _BASE_MESH_READER_H_

#include <base_reader.h>

typedef struct _GLMmodel GLMmodel;

class BaseMeshReader : public BaseReader
{
public:
	BaseMeshReader();
	virtual ~BaseMeshReader() {};

	virtual void SetFile(const std::wstring& file);
	virtual int Preprocess();

	virtual GLMmodel* Convert();
	virtual GLMmodel* Convert(int t) = 0;

	virtual void SetBatch(bool batch) = 0;
	virtual bool GetBatch() { return m_batch; }
	virtual int LoadBatch(int index);

	virtual std::wstring GetPathName() {return m_path_name;}
	virtual std::wstring GetDataName() {return m_data_name;}
	virtual int GetTimeNum() {return m_time_num;}
	virtual int GetCurTime() {return m_cur_time;}

	virtual int GetBatchNum() {return (int)m_batch_list.size();}
	virtual int GetCurBatch() {return m_cur_batch;}

	virtual std::wstring GetCurDataName(int t);

protected:
	std::wstring m_data_name;

	//4d sequence
	struct TimeDataInfo
	{
		int filenumber;
		std::wstring filename;
	};
	std::vector<TimeDataInfo> m_4d_seq;
	int m_cur_time = -1;

	int m_time_num = 0;
};
#endif//_BASE_MESH_READER_H_