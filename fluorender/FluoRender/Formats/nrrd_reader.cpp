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
#include "nrrd_reader.h"
#include "../compatibility.h"
#include <algorithm>
#include <sstream>

NRRDReader::NRRDReader()
{
   m_time_num = 0;
   m_chan_num = 0;
   m_slice_num = 0;
   m_x_size = 0;
   m_y_size = 0;

   m_valid_spc = false;
   m_xspc = 0.0;
   m_yspc = 0.0;
   m_zspc = 0.0;

   m_max_value = 0.0;
   m_scalar_scale = 1.0;

   m_batch = false;
   m_cur_batch = -1;
   m_cur_time = -1;

   m_time_id = L"_T";
}

NRRDReader::~NRRDReader()
{
}

void NRRDReader::SetFile(string &file)
{
   if (!file.empty())
   {
      if (!m_path_name.empty())
         m_path_name.clear();
      m_path_name.assign(file.length(), L' ');
      copy(file.begin(), file.end(), m_path_name.begin());
   }
   m_id_string = m_path_name;
}

void NRRDReader::SetFile(wstring &file)
{
   m_path_name = file;
   m_id_string = m_path_name;
}

void NRRDReader::Preprocess()
{
   m_4d_seq.clear();

   //separate path and name
   int64_t pos = m_path_name.find_last_of(GETSLASH());
   if (pos == -1)
      return;
   wstring path = m_path_name.substr(0, pos+1);
   wstring name = m_path_name.substr(pos+1);
   //generate search name for time sequence
   int64_t begin = name.find(m_time_id);
   size_t end = -1;
   size_t id_len = m_time_id.size();
   wstring t_num;
   if (begin != -1)
   {
      size_t j;
      for (j=begin+id_len; j<name.size(); j++)
      {
         wchar_t c = name[j];
         if (iswdigit(c))
            t_num.push_back(c);
         else
            break;
      }
      if (t_num.size() > 0)
         end = j;
      else
         begin = -1;
   }
   //build 4d sequence
   if (begin == -1)
   {
      TimeDataInfo info;
      info.filenumber = 0;
      info.filename = m_path_name;
      m_4d_seq.push_back(info);
      m_cur_time = 0;
   }
   else
   {
      //search time sequence files
      std::vector<std::wstring> list;
      int tmp = 0;
      FIND_FILES(path,L".nrrd",list,tmp,name.substr(0,begin+id_len+1));
      for(size_t i = 0; i < list.size(); i++) {
         size_t start_idx = list.at(i).find(m_time_id) + id_len;
         size_t end_idx   = list.at(i).find(L".nrrd");
         size_t size = end_idx - start_idx;
         std::wstring fileno = list.at(i).substr(start_idx, size);
         TimeDataInfo info;
         info.filenumber = WSTOI(fileno);
         info.filename = list.at(i);
         m_4d_seq.push_back(info);
      }
   }
   if (m_4d_seq.size() > 0)
   {
      std::sort(m_4d_seq.begin(), m_4d_seq.end(), NRRDReader::nrrd_sort);
      for (int t=0; t<(int)m_4d_seq.size(); t++)
      {
         if (m_4d_seq[t].filename == m_path_name)
         {
            m_cur_time = t;
            break;
         }
      }
   }
   else
      m_cur_time = 0;

   //3D nrrd file
   m_chan_num = 1;
   //get time number
   m_time_num = (int)m_4d_seq.size();
}

void NRRDReader::SetSliceSeq(bool ss)
{
   //do nothing
}

bool NRRDReader::GetSliceSeq()
{
   return false;
}

void NRRDReader::SetTimeId(wstring &id)
{
   m_time_id = id;
}

wstring NRRDReader::GetTimeId()
{
   return m_time_id;
}

void NRRDReader::SetBatch(bool batch)
{
   if (batch)
   {
      //read the directory info
      wstring search_path = m_path_name.substr(0, m_path_name.find_last_of(GETSLASH())) + GETSLASH();
      wstring search_str = search_path + L".nrrd";
      FIND_FILES(search_path,L".nrrd",m_batch_list,m_cur_batch,L"");
      m_batch = true;
   }
   else
      m_batch = false;
}

int NRRDReader::LoadBatch(int index)
{
   int result = -1;
   if (index>=0 && index<(int)m_batch_list.size())
   {
      m_path_name = m_batch_list[index];
      Preprocess();
      result = index;
      m_cur_batch = result;
   }
   else
      result = -1;

   return result;
}

Nrrd* NRRDReader::Convert(int t, int c, bool get_max)
{
   if (t<0 || t>=m_time_num)
      return 0;

   int i;

   m_data_name = m_4d_seq[t].filename.substr(m_4d_seq[t].filename.find_last_of(GETSLASH())+1);

   Nrrd *output = nrrdNew();
   NrrdIoState *nio = nrrdIoStateNew();
   nrrdIoStateSet(nio, nrrdIoStateSkipData, AIR_TRUE);
   string str;
   str.assign(m_4d_seq[t].filename.length(), 0);
   for (i=0; i<(int)m_4d_seq[t].filename.length(); i++)
      str[i] = (char)m_4d_seq[t].filename[i];
   if (nrrdLoad(output, str.c_str(), nio))
      return 0;
   nio = nrrdIoStateNix(nio);
   if (output->dim != 3)
   {
      nrrdNix(output);
      return 0;
   }
   m_slice_num = int(output->axis[2].size);
   m_x_size = int(output->axis[0].size);
   m_y_size = int(output->axis[1].size);
   m_xspc = output->axis[0].spacing;
   m_yspc = output->axis[1].spacing;
   m_zspc = output->axis[2].spacing;
   if (m_xspc>0.0 && m_xspc<100.0 &&
         m_yspc>0.0 && m_yspc<100.0 &&
         m_zspc>0.0 && m_zspc<100.0)
      m_valid_spc = true;
   else
   {
      m_valid_spc = false;
      m_xspc = 1.0;
      m_yspc = 1.0;
      m_zspc = 1.0;
   }
   int data_size = m_slice_num * m_x_size * m_y_size;
   if (output->type == nrrdTypeUShort || output->type == nrrdTypeShort)
      data_size *= 2;
    output->data = new unsigned char[data_size];

   if (nrrdLoad(output, str.c_str(), NULL))
   {
      //delete []output->data;
      nrrdNix(output);
      return 0;
   }
    // turn signed into unsigned
    if (output->type == nrrdTypeChar) {
        for (i=0; i<m_slice_num*m_x_size*m_y_size; i++) {
            char val = ((char*)output->data)[i];
            unsigned char n = val + 128;
            ((unsigned char*)output->data)[i] = n;
        }
        output->type = nrrdTypeUChar;
    }
    m_max_value = 0.0;
    // turn signed into unsigned
    unsigned short min_value = 32768, n;
    if (output->type == nrrdTypeShort || output->type == nrrdTypeUShort) {
        for (i=0; i<m_slice_num*m_x_size*m_y_size; i++) {
            if (output->type == nrrdTypeShort) {
                short val = ((short*)output->data)[i];
                n = val + 32768;
                ((unsigned short*)output->data)[i] = n;
                min_value = (n < min_value)?n:min_value;
            } else {
                n =  ((unsigned short*)output->data)[i];
            }
            if (get_max)
                m_max_value = (n > m_max_value)?n:m_max_value;
        }
        output->type = nrrdTypeUShort;
    }
   //find max value
   if (output->type == nrrdTypeUChar)
   {
      //8 bit
      m_max_value = 255.0;
       m_scalar_scale = 1.0;
   }
   else if (output->type == nrrdTypeUShort)
   {
       m_max_value -= min_value;
       //16 bit
       for (i=0; i<m_slice_num*m_x_size*m_y_size; i++) {
           ((unsigned short*)output->data)[i] =
           ((unsigned short*)output->data)[i] - min_value;
       }
      if (m_max_value > 0.0)
         m_scalar_scale = 65535.0 / m_max_value;
      else
         m_scalar_scale = 1.0;
   }
   else
   {
      //delete []output->data;
      nrrdNix(output);
      return 0;
   }

   m_cur_time = t;
   return output;
}

bool NRRDReader::nrrd_sort(const TimeDataInfo& info1, const TimeDataInfo& info2)
{
   return info1.filenumber < info2.filenumber;
}

wstring NRRDReader::GetCurName(int t, int c)
{
   return m_4d_seq[t].filename;
}
