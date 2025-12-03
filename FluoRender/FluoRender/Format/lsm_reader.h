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
#ifndef _LSM_READER_H_
#define _LSM_READER_H_

#include <base_vol_reader.h>
#include <Vector.h>
#include <string>
#include <vector>

class LSMReader : public BaseVolReader
{
public:
	LSMReader();
	~LSMReader();

	int GetType() { return READER_LSM_TYPE; }

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
	std::wstring GetDataName() {return m_data_name;}
	int GetTimeNum() {return m_time_num;}
	int GetCurTime() {return m_cur_time;}
	int GetChanNum() {return m_chan_num;}
	double GetExcitationWavelength(int chan);
	fluo::Vector GetResolution() { return m_size; }
	bool IsSpcInfoValid() {return m_valid_spc;}
	fluo::Vector GetSpacing() { return m_spacing; }
	double GetMinValue() { return m_min_value; }
	double GetMaxValue() {return m_max_value;}
	double GetScalarScale() {return m_scalar_scale;}
	bool GetBatch() {return m_batch;}
	int GetBatchNum() {return (int)m_batch_list.size();}
	int GetCurBatch() {return m_cur_batch;}

private:
	std::wstring m_data_name;

	struct SliceInfo
	{
		unsigned int offset;	//offset value in lsm file
		unsigned int offset_high;//if it is larger than 4GB, this is the high 32 bits of the 64-bit address
		unsigned int size;		//size in lsm file
	};
	typedef std::vector<SliceInfo> ChannelInfo;		//all slices form a channel
	typedef std::vector<ChannelInfo> DatasetInfo;	//channels form a dataset
	std::vector<DatasetInfo> m_lsm_info;				//datasets of different time points form an lsm file


	int m_time_num;
	int m_cur_time;
	int m_chan_num;
	fluo::Vector m_size;
	bool m_valid_spc;
	fluo::Vector m_spacing;
	double m_min_value;
	double m_max_value;
	double m_scalar_scale;

	//lsm properties
	int m_compression;		//1:no compression; 5:lzw compression
	int m_predictor;		//shoud be 2 if above is 5
	unsigned int m_version;	//lsm version
	int m_datatype;			//0: varying; 1: 8-bit; 2: 12-bit; 5: 32-bit
	bool m_l4gb;			//true: this is a larger-than-4-GB file

	//wavelength info
	struct WavelengthInfo
	{
		int chan_num;
		double wavelength;
	};
	std::vector<WavelengthInfo> m_excitation_wavelength_list;

private:
	void ReadLsmInfo(FILE* pfile, unsigned char* pdata, unsigned int size);

};

#endif//_LSM_READER_H_
