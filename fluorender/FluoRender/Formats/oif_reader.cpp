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
#include "oif_reader.h"
#include "../compatibility.h"
#include <algorithm>

OIFReader::OIFReader()
{
   m_time_num = 0;
   m_cur_time = -1;
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

   m_time_id = L"_T";
   m_type = 0;
   m_oif_t = 0;
}

OIFReader::~OIFReader()
{
}

void OIFReader::SetFile(std::string &file)
{
   if (!file.empty())
   {
      if (!m_path_name.empty())
         m_path_name.clear();
      m_path_name.assign(file.length(), L' ');
      copy(file.begin(), file.end(), m_path_name.begin());

      m_data_name = m_path_name.substr(m_path_name.find_last_of(GETSLASH())+1);
   }
   m_id_string = m_path_name;
}

void OIFReader::SetFile(wstring &file)
{
   m_path_name = file;
   m_data_name = m_path_name.substr(m_path_name.find_last_of(GETSLASH())+1);
   m_id_string = m_path_name;
}

void OIFReader::Preprocess()
{
   m_type = 0;
   m_oif_info.clear();

   //separate path and name
   int64_t pos = m_path_name.find_last_of(GETSLASH());
   if (pos == -1)
      return;
   wstring path = m_path_name.substr(0, pos+1);
   wstring name = m_path_name.substr(pos+1);
   //extract time sequence string
   int64_t begin = name.find(m_time_id);
   size_t end = -1;
   size_t id_len = m_time_id.size();
   if (begin != -1)
   {
      wstring t_num;
      size_t j;
      for (j=begin+id_len; j<name.size(); j++)
      {
         wchar_t c = name[j];
         if (iswdigit(c))
            t_num.push_back(c);
         else break;
      }
      if (t_num.size() > 0)
         end = j;
      else
         begin = -1;
   }

   if (begin == -1)
   {
      ReadSingleOif();
   }
   else
   {
      //search time sequence files
      std::vector<std::wstring> list;
      int tmp = 0;
      FIND_FILES(path,L".oif",list,tmp,name.substr(0,begin+id_len+1));
      for(size_t i = 0; i < list.size(); i++) {
         size_t start_idx = list.at(i).find(m_time_id) + id_len;
         size_t end_idx   = list.at(i).find(L".oif");
         size_t size = end_idx - start_idx;
         std::wstring fileno = list.at(i).substr(start_idx, size);
         TimeDataInfo info;
         info.filenumber = WSTOI(fileno);
         info.filename = list.at(i);
         m_oif_info.push_back(info);
      }

      if (m_oif_info.size() > 0)
      {
         m_type = 1;
         std::sort(m_oif_info.begin(), m_oif_info.end(), OIFReader::oif_sort);
         ReadSequenceOif();
      }
      else
      {
         m_oif_info.clear();
         ReadSingleOif();
      }
   }

   ReadOif();

   m_time_num = int(m_oif_info.size());
   if (m_type == 0)
      m_cur_time = 0;
   m_chan_num = m_time_num>0?int(m_oif_info[0].dataset.size()):0;
   m_slice_num = m_chan_num>0?int(m_oif_info[0].dataset[0].size()):0;
}

bool OIFReader::oif_sort(const TimeDataInfo& info1, const TimeDataInfo& info2)
{
   return info1.filenumber < info2.filenumber;
}

void OIFReader::ReadSingleOif()
{
   m_subdir_name = m_path_name + L".files";
   std::vector<std::wstring> list;
   int tmp;
   FIND_FILES(m_subdir_name,L".tif",list,tmp);
   //read file sequence
   for(size_t f = 0; f < list.size(); f++)
      ReadTifSequence(list.at(f));
}

void OIFReader::ReadSequenceOif()
{
   for (int i=0; i<(int)m_oif_info.size(); i++)
   {
      wstring path_name = m_oif_info[i].filename;
      m_oif_info[i].subdirname = path_name + L".files";

      if (path_name == m_path_name)
         m_cur_time = i;

      m_subdir_name = path_name + L".files";
      std::vector<std::wstring> list;
      FIND_FILES(m_subdir_name,L".tif",list,m_oif_t);
      //read file sequence
      for(size_t f = 0; f < list.size(); f++)
         ReadTifSequence(list.at(f));
   }
}

void OIFReader::SetSliceSeq(bool ss)
{
   //do nothing
}

bool OIFReader::GetSliceSeq()
{
   return false;
}

void OIFReader::SetTimeId(wstring &id)
{
   m_time_id = id;
}

wstring OIFReader::GetTimeId()
{
   return m_time_id;
}

void OIFReader::SetBatch(bool batch)
{
   if (batch)
   {
      //read the directory info
      wstring search_path = m_path_name.substr(0, m_path_name.find_last_of(GETSLASH())) + GETSLASH();
      FIND_FILES(search_path,L".oif",m_batch_list,m_cur_batch);
      m_batch = true;
   }
   else
      m_batch = false;
}

int OIFReader::LoadBatch(int index)
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

void OIFReader::ReadTifSequence(wstring file_name)
{
   size_t line_size = file_name.size();
   if (file_name.substr(line_size-3, 3) == L"tif")
   {
      //interpret file_name
      int64_t pos;
      int64_t pos_ = file_name.find_last_of(L'_');
      if (pos_!=-1)
      {
         size_t j;
         wstring wstr;
         int num_c = -1;
         int num_z = -1;
         int num_t = -1;
         int num_l = -1;

         //read channel number 'C'
         pos = file_name.find(L'C', pos_+1);
         if (pos!=-1)
         {
            for (j=pos+1; j<file_name.size(); j++)
            {
               if (iswdigit(file_name[j]))
                  wstr.push_back(file_name[j]);
               else
                  break;
            }
            num_c = WSTOI(wstr);
            wstr.clear();
         }
         //read z number 'Z'
         pos = file_name.find(L'Z', pos_+1);
         if (pos!=-1)
         {
            for (j=pos+1; j<file_name.size(); j++)
            {
               if (iswdigit(file_name[j]))
                  wstr.push_back(file_name[j]);
               else
                  break;
            }
            num_z = WSTOI(wstr);
            wstr.clear();
         }
         //read time number 'T'
         pos = file_name.find(L'T', pos_+1);
         if (pos!=-1)
         {
            for (j=pos+1; j<file_name.size(); j++)
            {
               if (iswdigit(file_name[j]))
                  wstr.push_back(file_name[j]);
               else
                  break;
            }
            num_t = WSTOI(wstr);
            wstr.clear();
         }
         //read lambda number 'L'
         pos = file_name.find(L'L', pos_+1);
         if (pos!=-1)
         {
            for (j=pos+1; j<file_name.size(); j++)
            {
               if (iswdigit(file_name[j]))
                  wstr.push_back(file_name[j]);
               else
                  break;
            }
            num_l = WSTOI(wstr);
            wstr.clear();
         }

         //add info to the list
         num_c = num_c==-1?0:num_c-1;
         num_t = num_t==-1?0:num_t-1;
         num_z = num_z==-1?1:num_z;
         if (num_z > 0)
         {
            num_z--;
            //allocate
            if (m_type == 0)
            {
               if (int(m_oif_info.size()) < num_t+1)
                  m_oif_info.resize(num_t+1);
               if (int(m_oif_info[num_t].dataset.size()) < num_c+1)
                  m_oif_info[num_t].dataset.resize(num_c+1);
               if (int(m_oif_info[num_t].dataset[num_c].size()) < num_z+1)
                  m_oif_info[num_t].dataset[num_c].resize(num_z+1);
               //add
               m_oif_info[num_t].dataset[num_c][num_z] = file_name;
            }
            else
            {
               if (num_t == 0)
               {
                  if (int(m_oif_info[m_oif_t].dataset.size()) < num_c+1)
                     m_oif_info[m_oif_t].dataset.resize(num_c+1);
                  if (int(m_oif_info[m_oif_t].dataset[num_c].size()) < num_z+1)
                     m_oif_info[m_oif_t].dataset[num_c].resize(num_z+1);
                  //add
                  m_oif_info[m_oif_t].dataset[num_c][num_z] = file_name;
               }
            }
         }
      }
   }
}

void OIFReader::ReadOif()
{
   //read oif file
   ifstream is(ws2s(m_path_name).c_str());
   wstring oneline;
   if (is.is_open())
   {
      //reset
      m_excitation_wavelength_list.clear();
      m_x_size = 0;
      m_y_size = 0;
      m_xspc = 0.0;
      m_yspc = 0.0;
      m_zspc = 0.0;
      //axis count
      axis_num = -1;
      cur_axis = -1;
      //channel count
      chan_num = -1;
      cur_chan = -1;
      //axis info
      axis_code.clear();
      pix_unit.clear();
      max_size.clear();
      start_pos.clear();
      end_pos.clear();

      while (!is.eof())
      {
         wchar_t c;
         is.read(((char*)(&c)), 1);
         if (!is.eof())
            is.read(((char*)(&c))+1, 1);
         if (c != L'\x0D' && c != L'\n')
            oneline.push_back(c);
         else
         {
            if (!oneline.empty())
               ReadOifLine(oneline);
            oneline.clear();
         }
      }
      is.close();
   }

   if (m_xspc>0.0 && m_xspc<100.0 &&
         m_yspc>0.0 && m_yspc<100.0)
   {
      m_valid_spc = true;
      if (m_zspc<=0.0 || m_zspc>100.0)
         m_zspc = max(m_xspc, m_yspc);
   }
   else
   {
      m_valid_spc = false;
      m_xspc = 1.0;
      m_yspc = 1.0;
      m_zspc = 1.0;
   }
}

void OIFReader::ReadOifLine(wstring oneline)
{
   //process
   if (oneline.substr(0, 6) == L"[Axis ")
   {
      axis_num++;
   }
   else
   {
      if (axis_num > -1)
      {
         size_t pos = oneline.find(L'=');
         wstring str1 = oneline.substr(0, oneline.find_last_not_of(L' ', pos));
         wstring str2 = oneline.substr(oneline.find_first_not_of(L' ', pos+1));

         if (str1 == L"AxisCode")
         {
            if (cur_axis != axis_num)
            {
               cur_axis = axis_num;
               axis_code.clear();
               pix_unit.clear();
               max_size.clear();
               start_pos.clear();
               end_pos.clear();
            }
            axis_code = str2;
         }
         else if (str1 == L"PixUnit")
         {
            if (cur_axis != axis_num)
            {
               cur_axis = axis_num;
               axis_code.clear();
               pix_unit.clear();
               max_size.clear();
               start_pos.clear();
               end_pos.clear();
            }
            pix_unit = str2;
         }
         else if (str1 == L"MaxSize")
         {
            if (cur_axis != axis_num)
            {
               cur_axis = axis_num;
               axis_code.clear();
               pix_unit.clear();
               max_size.clear();
               start_pos.clear();
               end_pos.clear();
            }
            max_size = str2;
         }
         else if (str1 == L"StartPosition")
         {
            if (cur_axis != axis_num)
            {
               cur_axis = axis_num;
               axis_code.clear();
               pix_unit.clear();
               max_size.clear();
               start_pos.clear();
               end_pos.clear();
            }
            start_pos = str2;
         }
         else if (str1 == L"EndPosition")
         {
            if (cur_axis != axis_num)
            {
               cur_axis = axis_num;
               axis_code.clear();
               pix_unit.clear();
               max_size.clear();
               start_pos.clear();
               end_pos.clear();
            }
            end_pos = str2;
         }
      }
   }
   if (oneline.substr(0, 9) == L"[Channel ")
   {
	  light_type.clear();
      chan_num++;
   }
   else
   {
      if (chan_num > -1)
      {
         size_t pos = oneline.find(L'=');
         wstring str1 = oneline.substr(0, oneline.find_last_not_of(L' ', pos));
         wstring str2 = oneline.substr(oneline.find_first_not_of(L' ', pos+1));
		 wstring str3 = L"Transmitted Light";
		 if (str1 == L"LightType") {
			 light_type = str2;
			 if (light_type.find(str3) != wstring::npos) {
				 for (int i = m_excitation_wavelength_list.size() -1; i >=0; i--) {
					 if (m_excitation_wavelength_list.at(i).chan_num == cur_chan) {
						 m_excitation_wavelength_list.at(i).wavelength = -1;
						 break;
					 }
				 }
			 }
		  }
         else if (str1 == L"ExcitationWavelength")
         {
            if (cur_chan != chan_num)
            {
               cur_chan = chan_num;
               WavelengthInfo info;
               info.chan_num = cur_chan;
               info.wavelength = WSTOD(str2);
				if (light_type == L"Transmitted Light") 
					info.wavelength = -1;
               m_excitation_wavelength_list.push_back(info);
            }
         }
      }
   }

   //axis
   if (!axis_code.empty() &&
         !pix_unit.empty() &&
         !max_size.empty() &&
         !start_pos.empty() &&
         !end_pos.empty())
   {
      //calculate
      double spc = 0.0;
      double dmax = WSTOD(max_size);
      if (dmax > 0.0)
         spc= fabs((WSTOD(end_pos)-
                  WSTOD(start_pos)))/
            dmax;
      if ((int64_t)pix_unit.find(L"nm")!=-1)
         spc /= 1000.0;
      if ((int64_t)axis_code.find(L"X")!=-1)
      {
         m_x_size = WSTOI(max_size);
         m_xspc = spc;
      }
      else if ((int64_t)axis_code.find(L"Y")!=-1)
      {
         m_y_size = WSTOI(max_size);
         m_yspc = spc;
      }
      else if ((int64_t)axis_code.find(L"Z")!=-1)
         m_zspc = spc;

      axis_code.clear();
      pix_unit.clear();
      max_size.clear();
      start_pos.clear();
      end_pos.clear();
   }
}

double OIFReader::GetExcitationWavelength(int chan)
{
   for (int i=0; i<(int)m_excitation_wavelength_list.size(); i++)
   {
      if (m_excitation_wavelength_list[i].chan_num == chan)
         return m_excitation_wavelength_list[i].wavelength;
   }
   return 0.0;
}

Nrrd* OIFReader::Convert(int t, int c, bool get_max)
{
   Nrrd *data = 0;
   int sl_num = 0;

   if (t>=0 && t<m_time_num &&
         c>=0 && c<m_chan_num &&
         m_slice_num>0 &&
         m_x_size>0 &&
         m_y_size>0)
   {
      //allocate memory for nrrd
      unsigned short *val = new (std::nothrow) unsigned short[m_x_size*m_y_size*m_slice_num];

      //read the channel
      ChannelInfo *cinfo = &m_oif_info[t].dataset[c];
      int i;
      for (i=0; i<int(cinfo->size()); i++)
      {
         char *pbyData = 0;
         wstring file_name = (*cinfo)[i];

         //open file
         ifstream is;
         is.open(ws2s(file_name).c_str(), ios::binary);
         if (is.is_open())
         {
            is.seekg(0, ios::end);
            size_t size = is.tellg();
            pbyData = new char[size];
            is.seekg(0, ios::beg);
            is.read(pbyData, size);
            is.close();

            //read
            ReadTiff(pbyData, val, i);

            //increase
            sl_num++;
         }

         if (pbyData)
            delete []pbyData;
      }

      //create nrrd
      if (val && sl_num == m_slice_num)
      {
         //ok
         data = nrrdNew();
         nrrdWrap(data, val, nrrdTypeUShort, 3, (size_t)m_x_size, (size_t)m_y_size, (size_t)m_slice_num);
         nrrdAxisInfoSet(data, nrrdAxisInfoSpacing, m_xspc, m_yspc, m_zspc);
         nrrdAxisInfoSet(data, nrrdAxisInfoMax, m_xspc*m_x_size, m_yspc*m_y_size, m_zspc*m_slice_num);
         nrrdAxisInfoSet(data, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
         nrrdAxisInfoSet(data, nrrdAxisInfoSize, (size_t)m_x_size, (size_t)m_y_size, (size_t)m_slice_num);
      }
      else
      {
         //something is wrong
         if (val)
            delete []val;
      }
   }

   if (m_max_value > 0.0)
      m_scalar_scale = 65535.0 / m_max_value;

   m_cur_time = t;
   return data;
}

wstring OIFReader::GetCurName(int t, int c)
{
   return m_oif_info[t].dataset[c][0];
}

void OIFReader::ReadTiff(char *pbyData, unsigned short *val, int z)
{
   if (*((unsigned int*)pbyData) != 0x002A4949)
      return;

   int compression = 0;
   unsigned int offset = 0;
   //directory offset
   offset = *((unsigned int*)(pbyData+4));
   //the directory
   //entry number
   int entry_num = *((unsigned short*)(pbyData+offset));
   //strip info
   int strips = 0;
   int rows = 0;
   vector <unsigned int> strip_offsets;
   vector <unsigned int> strip_bytes;
   //get strip info
   unsigned int s_num1 = 0;
   unsigned int s_num2 = 0;
   for (int i=0; i<entry_num; i++)
   {
      //read each entry (12 bytes)
      unsigned short tag = *((unsigned short*)(pbyData+offset+2+12*i));
      switch (tag)
      {
      case 0x0103:  //259, compression
         {
            unsigned short value;
            value = *((unsigned short*)(pbyData+offset+2+12*i+8));
            compression = value<<16>>16;
         }
         break;
      case 0x0111:  //strip offsets
         {
            unsigned short type = *((unsigned short*)(pbyData+offset+2+12*i+2));
            //number of values
            s_num1 = *((unsigned int*)(pbyData+offset+2+12*i+4));
            unsigned int entry_offset = 0;
            entry_offset = *((unsigned int*)(pbyData+offset+2+12*i+8));
            for (int j=0; j<int(s_num1); j++)
            {
               if (type == 3)
               {
                  //unsigned short
                  unsigned short value;
                  value = *((unsigned short*)(pbyData+entry_offset+2*j));
                  strip_offsets.push_back((unsigned int)value);
               }
               else if (type == 4)
               {
                  //unsigned int
                  unsigned int value;
                  value = *((unsigned int*)(pbyData+entry_offset+4*j));
                  strip_offsets.push_back(value);
               }
            }
         }
         break;
      case 0x0116:  //rows per strip
         {
            unsigned short type = *((unsigned short*)(pbyData+offset+2+12*i+2));
            if (type == 3)
            {
               //unsigned short
               unsigned short value;
               value = *((unsigned short*)(pbyData+offset+2+12*i+8));
               rows = value;
            }
            else if (type == 4)
            {
               //unsigned int
               unsigned int value;
               value = *((unsigned int*)(pbyData+offset+2+12*i+8));
               rows = value;
            }
         }
         break;
      case 0x0117:  //strip byte counts
         {
            unsigned short type = *((unsigned short*)(pbyData+offset+2+12*i+2));
            //number of values
            s_num2 = *((unsigned int*)(pbyData+offset+2+12*i+4));
            unsigned int entry_offset = 0;
            entry_offset = *((unsigned int*)(pbyData+offset+2+12*i+8));
            for (int j=0; j<int(s_num2); j++)
            {
               if (type == 3)
               {
                  //unsigned short
                  unsigned short value;
                  value = *((unsigned short*)(pbyData+entry_offset+2*j));
                  strip_bytes.push_back((unsigned int)value);
               }
               else if (type == 4)
               {
                  //unsigned int
                  unsigned int value;
                  value = *((unsigned int*)(pbyData+entry_offset+4*j));
                  strip_bytes.push_back(value);
               }
            }
         }
         break;
      case 0x0119:  //max sample value
         {
            unsigned short value;
            value = *((unsigned short*)(pbyData+offset+2+12*i+8));
            if ((double)value > m_max_value)
               m_max_value = (double)value;
         }
         break;
      }
   }
   //read strips
   if (s_num1 == s_num2 &&
         strip_offsets.size() == s_num1 &&
         strip_bytes.size() == s_num2)
   {
      strips = s_num1;

      unsigned int val_pos = z*m_x_size*m_y_size;
      for (int i=0; i<strips; i++)
      {
         unsigned int data_pos = strip_offsets[i];
         unsigned int data_size = strip_bytes[i];
         if (compression == 1)//no copmression
            memcpy((void*)(val+val_pos), (void*)(pbyData+data_pos), data_size);
         else if (compression == 5)
            LZWDecode((tidata_t)(pbyData+data_pos), (tidata_t)(val+val_pos), m_x_size*rows*2);
         val_pos += rows*m_x_size;
      }
   }
}
