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
#ifndef _OIB_READER_H_
#define _OIB_READER_H_

#include <stdio.h>
#include "../compatibility.h"
#include <vector>
#include "pole/pole.h"
#include "base_reader.h"

using namespace std;

class OIBReader : public BaseReader
{
   public:
      OIBReader();
      ~OIBReader();

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

   private:
      wstring m_data_name;
      wstring m_oif_name;
      wstring m_substg_name;
	wstring light_type;

      int m_type;  //0-time data in a single file; 1-time data in a file sequence
      struct SliceInfo
      {
         wstring stream_name;
         wstring file_name;
      };
      typedef vector<SliceInfo> ChannelInfo;    //slices form a channel
      typedef vector<ChannelInfo> DatasetInfo;  //channels form a dataset
      struct TimeDataInfo              //
      {
         int filenumber;    //if type is 1, file number for current time data
         wstring filename;  //if type is 1, file name for current time data
         wstring substgname;  //substorage name
         DatasetInfo dataset;  //a list of the channels
      };
      vector<TimeDataInfo> m_oib_info;      // time data form the complete oib dataset
      int m_oib_t;  //current time point in oib info for reading

      int m_time_num;
      int m_cur_time;
      int m_chan_num;
      struct WavelengthInfo
      {
         int chan_num;
         double wavelength;
      };
      vector<WavelengthInfo> m_excitation_wavelength_list;
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
      static bool oib_sort(const TimeDataInfo& info1, const TimeDataInfo& info2);
      void ReadSingleOib();
      void ReadSequenceOib();
	void ReadStream(POLE::Storage &pStg, wstring &stream_name);
	void ReadOibInfo(unsigned char* pbyData, size_t size);
	void ReadOif(unsigned char* pbyData, size_t size);
	void ReadTiff(unsigned char* pbyData, unsigned short *val, int z);
};

#endif//_OIB_READER_H_
