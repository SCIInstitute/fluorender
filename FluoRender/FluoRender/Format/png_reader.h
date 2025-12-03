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
#ifndef _PNG_READER_H_
#define _PNG_READER_H_

#include <base_vol_reader.h>
#include <Vector.h>

class PNGReader : public BaseVolReader
{
public:
	PNGReader();
	~PNGReader();

	int GetType() { return READER_PNG_TYPE; }

	// Override base methods
	virtual void SetFile(const std::wstring& file);
	virtual int Preprocess();
	virtual void SetBatch(bool batch);
	virtual int LoadBatch(int index);
	virtual Nrrd* Convert(int t, int c, bool get_max);
	virtual std::wstring GetCurDataName(int t, int c);
	virtual std::wstring GetCurMaskName(int t, int c);
	virtual std::wstring GetCurLabelName(int t, int c);

	virtual std::wstring GetPathName() {return m_path_name;}
	virtual std::wstring GetDataName() {return m_data_name;}
	virtual int GetTimeNum() {return m_time_num;}
	virtual int GetCurTime() {return m_cur_time;}
	virtual int GetChanNum() {return m_chan_num;}
	virtual double GetExcitationWavelength(int chan) {return 0.0;}
	virtual fluo::Vector GetResolution() { return m_size; }
	virtual bool IsSpcInfoValid() {return m_valid_spc;}
	virtual fluo::Vector GetSpacing() { return m_spacing; }
	virtual double GetMinValue() { return m_min_value; }
	virtual double GetMaxValue() {return m_max_value;}
	virtual double GetScalarScale() {return m_scalar_scale;}
	virtual bool GetBatch() {return m_batch;}
	virtual int GetBatchNum() {return (int)m_batch_list.size();}
	virtual int GetCurBatch() {return m_cur_batch;}

private:
	std::wstring m_data_name;

	struct SliceInfo
	{
		int slicenumber;	//slice number for sorting
		std::wstring slice;		//slice file name
		int pagenumber;		//used to find the slice if it's in a hyperstack
							//for a multichannel data set, this is the number of the first channel
	};
	struct TimeDataInfo
	{
		int type;	//0-single file;1-sequence
		int filenumber;	//filenumber for sorting
		std::vector<SliceInfo> slices;
	};
	std::vector<TimeDataInfo> m_4d_seq;
	int m_cur_time;

	int m_time_num;
	int m_chan_num;
	fluo::Vector m_size;
	bool m_valid_spc;
	fluo::Vector m_spacing;
	double m_min_value;
	double m_max_value;
	double m_scalar_scale;
	int m_bits;

	void GetFileInfo(const std::wstring& filename);
	Nrrd* ReadPng(const std::vector<SliceInfo>& filelist, int c, bool get_max);
	bool ReadSinglePng(void* val, const std::wstring& filename, int c);
};

#endif // _PNG_READER_H_
