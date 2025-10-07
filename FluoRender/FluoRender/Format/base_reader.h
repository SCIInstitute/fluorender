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
#ifndef _BASE_READER_H_
#define _BASE_READER_H_

#include <Progress.h>
#include <string>
#include <vector>

//error codes
//return to notify caller if fail
#define READER_OK	0
#define READER_OPEN_FAIL	1
#define READER_FORMAT_ERROR	2
#define READER_EMPTY_DATA	3
#define READER_FP64_DATA	4
#define READER_JAVA_ARRAY_SIZE_ERROR 5

//define reader types
#define READER_MSK_TYPE	0
#define READER_LBL_TYPE	0
#define READER_NRRD_TYPE	1
#define READER_TIF_TYPE	2
#define READER_OIB_TYPE	3
#define READER_OIF_TYPE	4
#define READER_LSM_TYPE	5
#define READER_PVXML_TYPE	6
#define READER_BRKXML_TYPE	7
#define READER_CZI_TYPE	8
#define READER_IMAGEJ_TYPE	9
#define READER_ND2_TYPE	10
#define READER_LIF_TYPE	11
#define READER_LOF_TYPE	11
#define READER_MPG_TYPE	12
#define READER_JPG_TYPE	13
#define READER_PNG_TYPE	14
#define READER_DCM_TYPE	15
//mesh types
#define READER_OBJ_TYPE	50

class BaseReader : public Progress
{
public:
	BaseReader();
	virtual ~BaseReader() {};

	//get the reader type
	//see the header of each reader implementation for the actual value
	virtual int GetType() = 0;	//get reader type

	//set the file name and path to open and read
	//virtual void SetFile(const std::string &file) = 0;
	//set the file name and path to open and read (in wide string for foreign languages)
	virtual void SetFile(const std::wstring &file) = 0;

	//set time identifier
	//time identifier is a string to identify each file in a sequence as a time point
	//in UI, it is set in the open file dialog as option "time sequence identifier"
	//default value is "_T", which means any digits after the string in a file name is used as its time value
	virtual void SetTimeId(const std::wstring& id) { m_time_id = id; }
	//get current time identifier
	virtual std::wstring GetTimeId() { return m_time_id; }

	//preprocess the file
	//get the structure of the data without reading actual volume
	virtual int Preprocess() = 0;

	//a sequence of similar files can be put in one folder
	//once batch mode is turn on, those files can be "played back" as if they were in a time sequence
	//a time sequence assumes all files are of the same size, therefore no memory releasing (simple replacing) when time changes
	//a batch sequence can have files with different sizes. memory is released every time when time changes
	//batch mode is set in the movie export panel when time sequence is checked for a non-time-sequence
	virtual void SetBatch(bool batch) = 0;
	//get batch mode
	virtual bool GetBatch() = 0;
	//batch sequence is determined when SetBatch(true) is called
	//a stl vector containing file names is created
	//LoadBatch() loads a new file in the vector to replace the current one
	//usually called when the time value is changed from the movie export panel
	virtual int LoadBatch(int index) = 0;
	//similar to LoadBatch(). instead of using an absolute index value
	//it uses an offset value, telling the reader to load a file n indices after/before the current one
	virtual int LoadOffset(int offset);
	//get the current offest index value
	virtual int GetOffset();

	//get a string for only the path of the file
	virtual std::wstring GetPathName() = 0;
	//get a string for only the file name without its path
	virtual std::wstring GetDataName() = 0;
	//get the total number of time points
	virtual int GetTimeNum() = 0;
	//get the time point value of last/current loaded file
	virtual int GetCurTime() = 0;

	//get the total number of files in a batch when batch is turned on with SetBatch(true)
	virtual int GetBatchNum() = 0;
	//get the index of the current file when the batch mode is turned on
	virtual int GetCurBatch() = 0;

	//check if two readers are the same
	bool operator==(BaseReader& reader)
	{
		return m_id_string == reader.m_id_string;
	}
	//another way to check if two readers are the same
	bool Match(const std::wstring &id_string)
	{
		return m_id_string == id_string;
	}

	static std::string GetError(int code);

protected:
	std::wstring m_id_string;	//the path and file name used to read files

	//3d batch
	bool m_batch;
	std::vector<std::wstring> m_batch_list;
	int m_cur_batch;
	
	std::wstring m_path_name;

	std::wstring m_info;

	//time sequence id
	std::wstring m_time_id = L"_T";
};

#endif//_BASE_READER_H_