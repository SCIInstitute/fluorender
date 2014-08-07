#include "pvxml_reader.h"
#include <wx/xml/xml.h>
#include "../compatibility.h"
#include <fstream>
#include <iostream>

PVXMLReader::PVXMLReader()
{
   m_resize_type = 0;
   m_resample_type = 0;
   m_alignment = 0;

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

	//falgs for flipping
	m_user_flip_x = 0;
	m_user_flip_y = 0;
	m_flip_x = false;
	m_flip_y = false;
}

PVXMLReader::~PVXMLReader()
{
}

void PVXMLReader::SetFile(string &file)
{
   if (!file.empty())
   {
      if (!m_path_name.empty())
         m_path_name.clear();
      m_path_name.assign(file.length(), L' ');
      copy(file.begin(), file.end(), m_path_name.begin());
#ifdef _WIN32
   wchar_t slash = L'\\';
#else
   wchar_t slash = L'/';
#endif
      m_data_name = m_path_name.substr(m_path_name.find_last_of(slash)+1);
   }
   m_id_string = m_path_name;
}

void PVXMLReader::SetFile(wstring &file)
{
   m_path_name = file;
#ifdef _WIN32
   wchar_t slash = L'\\';
#else
   wchar_t slash = L'/';
#endif
   m_data_name = m_path_name.substr(m_path_name.find_last_of(slash)+1);
   m_id_string = m_path_name;
}

void PVXMLReader::Preprocess()
{
   m_pvxml_info.clear();
   m_slice_num = 0;
   m_chan_num = 0;
   m_max_value = 0.0;

#ifdef _WIN32
   wchar_t slash = L'\\';
#else
   wchar_t slash = L'/';
#endif
   //separate path and name
   size_t pos = m_path_name.find_last_of(slash);
   if (pos == -1)
      return;
   wstring path = m_path_name.substr(0, pos+1);
   wstring name = m_path_name.substr(pos+1);

   wxXmlDocument doc;
   if (!doc.Load(m_path_name))
      return;

   wxXmlNode *root = doc.GetRoot();

   if (!root || root->GetName() != "PVScan")
      return;

   wxXmlNode *child = root->GetChildren();
   while (child)
   {
      if (child->GetName() == "SystemConfiguration")
         ReadSystemConfig(child);
      else if (child->GetName() == "PVStateShard")
      {
         UpdateStateShard(child);
         m_state_shard_stack.push_back(m_current_state);
      }
      else if (child->GetName() == "Sequence")
         ReadSequence(child);
      child = child->GetNext();
   }

   m_time_num = int(m_pvxml_info.size());
   m_cur_time = 0;

   if (m_time_num == 0) return;

   int i, j, k, l;
   double x_end, y_end, z_end;
   wstring filename;
   bool first = true;

   for (i=0; i<(int)m_pvxml_info.size(); i++)
   {
      TimeDataInfo* time_data_info = &(m_pvxml_info[i]);
      for (j=0; j<(int)time_data_info->size(); j++)
      {
         SequenceInfo* sequence_info = &((*time_data_info)[j]);
         for (k=0; k<(int)sequence_info->frames.size(); k++)
         {
            FrameInfo *frame_info = &((sequence_info->frames)[k]);
            x_end = frame_info->x_start + frame_info->x_size * m_xspc;
            y_end = frame_info->y_start + frame_info->y_size * m_yspc;
            z_end = frame_info->z_start + m_zspc;

            if (first)
            {
               m_x_min = frame_info->x_start;
               m_y_min = frame_info->y_start;
               m_z_min = frame_info->z_start;
               m_x_max = x_end;
               m_y_max = y_end;
               m_z_max = z_end;
               first = false;
            }
            else
            {
               m_x_min = frame_info->x_start<m_x_min?frame_info->x_start:m_x_min;
               m_y_min = frame_info->y_start<m_y_min?frame_info->y_start:m_y_min;
               m_z_min = frame_info->z_start<m_z_min?frame_info->z_start:m_z_min;
               m_x_max = x_end>m_x_max?x_end:m_x_max;
               m_y_max = y_end>m_y_max?y_end:m_y_max;
               m_z_max = z_end>m_z_max?z_end:m_z_max;
            }

            //append path
            for (l=0; l<(int)frame_info->channels.size(); l++)
            {
               filename = path + frame_info->channels[l].file_name;
               frame_info->channels[l].file_name = filename;
            }
         }
      }
   }

   //make sure not divide by zero
   m_xspc = m_xspc==0.0?1.0:m_xspc;
   m_yspc = m_yspc==0.0?1.0:m_yspc;
   m_zspc = m_zspc==0.0?1.0:m_zspc;

   m_x_size = int((m_x_max - m_x_min) / m_xspc + 0.5);
   m_y_size = int((m_y_max - m_y_min) / m_yspc + 0.5);
   m_slice_num = int((m_z_max - m_z_min) / m_zspc + 0.5);

	if (m_user_flip_y == 1 ||
		m_user_flip_y == 0)
		m_flip_y = false;
	else if (m_user_flip_y == -1)
		m_flip_y = true;
	int y0;
	bool firsty = true;
	bool flipy = false;
   for (i=0; i<(int)m_pvxml_info.size(); i++)
   {
      TimeDataInfo* time_data_info = &(m_pvxml_info[i]);
      for (j=0; j<(int)time_data_info->size(); j++)
      {
         SequenceInfo* sequence_info = &((*time_data_info)[j]);
         for (k=0; k<(int)sequence_info->frames.size(); k++)
         {
            FrameInfo *frame_info = &((sequence_info->frames)[k]);
            frame_info->x = int((frame_info->x_start - m_x_min) / m_xspc + 0.5);
            frame_info->y = int((frame_info->y_start - m_y_min) / m_yspc + 0.5);
            frame_info->z = int((frame_info->z_start - m_z_min) / m_zspc + 0.5);
				if (m_user_flip_y==0 && !flipy)
				{
					if (firsty)
					{
						y0 = frame_info->y;
						firsty = false;
					}
					else
					{
						if (frame_info->y > y0+int(frame_info->y_size*0.1))
						{
							m_flip_y = true;
							flipy = true;
						}
						else if (frame_info->y < y0-int(frame_info->y_size*0.1))
							flipy = true;
					}
				}
			}
		}
	}
}

void PVXMLReader::ReadSystemConfig(wxXmlNode* systemNode)
{
}

void PVXMLReader::UpdateStateShard(wxXmlNode *stateNode)
{
   if (m_state_shard_stack.size())
      m_current_state = m_state_shard_stack.back();
   wxXmlNode *child = stateNode->GetChildren();
   while (child)
   {
      if (child->GetName() == "Key")
         ReadKey(child);
      child = child->GetNext();
   }
}

void PVXMLReader::ReadKey(wxXmlNode* keyNode)
{
   long ival;
   double dval;
   wxString strKey = keyNode->GetAttribute("key");
   wxString strValue = keyNode->GetAttribute("value");

   if (strKey == "xYStageGridIndex")
   {
      strValue.ToLong(&ival);
      m_current_state.grid_index = ival;
   }
   else if (strKey == "xYStageGridXIndex")
   {
      strValue.ToLong(&ival);
      m_current_state.grid_index_x = ival;
   }
   else if (strKey == "xYStageGridYIndex")
   {
      strValue.ToLong(&ival);
      m_current_state.grid_index_y = ival;
   }
   else if (strKey == "positionCurrent_XAxis")
   {
      strValue.ToDouble(&dval);
      m_current_state.pos_x = dval;
   }
   else if (strKey == "positionCurrent_YAxis")
   {
      strValue.ToDouble(&dval);
      m_current_state.pos_y = dval;
   }
   else if (strKey == "positionCurrent_ZAxis")
   {
      int pos = strValue.Find(',');
      if (pos == wxNOT_FOUND)
      {
         strValue.ToDouble(&dval);
         m_current_state.pos_z = dval;
      }
      else
      {
         m_current_state.pos_z = 0.0;
         do
         {
            strValue.Left(pos).ToDouble(&dval);
            m_current_state.pos_z += dval;
            strValue = strValue.Right(strValue.Length()-pos-1);
            pos = strValue.Find(',');
         } while (pos != wxNOT_FOUND);
         if (strValue.Length() &&
               strValue.ToDouble(&dval))
            m_current_state.pos_z += dval;
      }

   }
   else if (strKey == "zDevice")
   {
      strValue.ToLong(&ival);
      m_current_state.z_device = ival;
   }
   else if (strKey == "pixelsPerLine")
   {
      strValue.ToLong(&ival);
      m_current_state.ppl = ival;
   }
   else if (strKey == "linesPerFrame")
   {
      strValue.ToLong(&ival);
      m_current_state.lpf = ival;
   }
   else if (strKey == "micronsPerPixel_XAxis")
   {
      strValue.ToDouble(&dval);
      m_current_state.mpp_x = dval;
   }
   else if (strKey == "micronsPerPixel_YAxis")
   {
      strValue.ToDouble(&dval);
      m_current_state.mpp_y = dval;
   }
   else if (strKey == "bitDepth")
   {
      strValue.ToLong(&ival);
      m_current_state.bit_depth = ival;
   }
}

void PVXMLReader::ReadSequence(wxXmlNode* seqNode)
{
   m_seq_slice_num = 0;
   m_seq_zspc = FLT_MAX;
   m_seq_zpos = 0.0;
   wxXmlNode *child = seqNode->GetChildren();
   int stack_push_count = 0;
   while (child)
   {
      if (child->GetName() == "PVStateShard")
      {
         UpdateStateShard(child);
         m_state_shard_stack.push_back(m_current_state);
         stack_push_count++;
      }
      else if (child->GetName() == "Frame")
      {
         ReadFrame(child);
         m_seq_slice_num++;
      }
      child = child->GetNext();
   }
   //pop all stacked states
   for (int i=0; i<stack_push_count; i++)
      m_state_shard_stack.pop_back();

   if (m_slice_num)
   {
      m_slice_num = m_seq_slice_num>m_slice_num?m_seq_slice_num:m_slice_num;
      if (m_seq_zspc > 0.0)
      {
         if (m_zspc == 0.0)
            m_zspc = m_seq_zspc;
         else
            m_zspc = m_seq_zspc<m_zspc?m_seq_zspc:m_zspc;
      }
   }
   else
   {
      m_slice_num = m_seq_slice_num;
      m_zspc = m_seq_zspc;
   }
   m_pvxml_info.back().back().grid_index = m_current_state.grid_index;
}

void PVXMLReader::ReadFrame(wxXmlNode* frameNode)
{
   wxString str;
   long ival;
   FrameInfo frame_info;

   wxXmlNode *child = frameNode->GetChildren();
   while (child)
   {
      if (child->GetName() == "File")
      {
         wxString filename = child->GetAttribute(
               "filename");
         ChannelInfo channel_info;
         channel_info.file_name = filename.ToStdWstring();
         frame_info.channels.push_back(channel_info);
         int size = frame_info.channels.size();
         m_chan_num = size>m_chan_num?size:m_chan_num;
      }
      else if (child->GetName() == "PVStateShard")
         UpdateStateShard(child);

      child = child->GetNext();
   }

   frame_info.x_size = m_current_state.ppl;
   frame_info.y_size = m_current_state.lpf;
   frame_info.x_start = m_current_state.pos_x;
   frame_info.y_start = m_current_state.pos_y;
   frame_info.z_start = m_current_state.pos_z;
   if (m_seq_zpos != 0.0)
   {
      double spc = fabs(frame_info.z_start - m_seq_zpos);
      m_seq_zspc = spc<m_seq_zspc?spc:m_seq_zspc;
   }
   m_seq_zpos = frame_info.z_start;
   ival = 2<<(m_current_state.bit_depth-1);
   if (m_max_value == 0.0)
   {
      m_xspc = m_current_state.mpp_x;
      m_yspc = m_current_state.mpp_y;
      m_max_value = ival;
   }
   else
   {
      m_xspc = m_current_state.mpp_x<m_xspc?m_current_state.mpp_x:m_xspc;
      m_yspc = m_current_state.mpp_y<m_yspc?m_current_state.mpp_y:m_yspc;
      m_max_value = ival>m_max_value?ival:m_max_value;
   }

   if (!m_pvxml_info.size() ||
         (m_pvxml_info.back().size() &&
          m_current_state.grid_index<=
          m_pvxml_info.back().back().grid_index))
   {
      TimeDataInfo info_new;
      m_pvxml_info.push_back(info_new);
   }
   TimeDataInfo* time_data_info = &(m_pvxml_info.back());
   if (time_data_info)
   {
      if (!m_seq_slice_num)
      {
         SequenceInfo info_new;
         info_new.grid_index = -1;
         time_data_info->push_back(info_new);
      }
      SequenceInfo* sequence_info = &(time_data_info->back());
      if (sequence_info)
         sequence_info->frames.push_back(frame_info);
   }
}

void PVXMLReader::SetSliceSeq(bool ss)
{
   //do nothing
}

bool PVXMLReader::GetSliceSeq()
{
   return false;
}

void PVXMLReader::SetTimeId(wstring &id)
{
   m_time_id = id;
}

wstring PVXMLReader::GetTimeId()
{
   return m_time_id;
}

void PVXMLReader::SetBatch(bool batch)
{
#ifdef _WIN32
   wchar_t slash = L'\\';
#else
   wchar_t slash = L'/';
#endif
   if (batch)
   {
      //read the directory info
      wstring search_path = m_path_name.substr(0, m_path_name.find_last_of(slash)) + slash;
      FIND_FILES(search_path,L".oib",m_batch_list,m_cur_batch);
      m_batch = true;
   }
   else
      m_batch = false;
}

int PVXMLReader::LoadBatch(int index)
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

int PVXMLReader::LoadOffset(int offset)
{
   int result = m_cur_batch + offset;

   if (offset > 0)
   {
      if (result<(int)m_batch_list.size())
      {
         m_path_name = m_batch_list[result];
         Preprocess();
         m_cur_batch = result;
      }
      else if (m_cur_batch<(int)m_batch_list.size()-1)
      {
         result = (int)m_batch_list.size()-1;
         m_path_name = m_batch_list[result];
         Preprocess();
         m_cur_batch = result;
      }
      else
         result = -1;
   }
   else if (offset < 0)
   {
      if (result >= 0)
      {
         m_path_name = m_batch_list[result];
         Preprocess();
         m_cur_batch = result;
      }
      else if (m_cur_batch > 0)
      {
         result = 0;
         m_path_name = m_batch_list[result];
         Preprocess();
         m_cur_batch = result;
      }
      else
         result = -1;
   }
   else
      result = -1;

   return result;
}

double PVXMLReader::GetExcitationWavelength(int chan)
{
   return 0.0;
}

Nrrd *PVXMLReader::Convert(int t, int c, bool get_max)
{
   Nrrd *data = 0;

   if (t>=0 && t<m_time_num &&
         c>=0 && c<m_chan_num &&
         m_slice_num>0 &&
         m_x_size>0 &&
         m_y_size>0)
   {
      //allocate memory for nrrd
      long long total_size = (long long)m_x_size*(long long)m_y_size*(long long)m_slice_num;
      unsigned short *val = new (std::nothrow) unsigned short[total_size];
      if (!val) return 0;

      TimeDataInfo* time_data_info = &(m_pvxml_info[t]);
      int i, j, k;
      for (i=0; i<(int)time_data_info->size(); i++)
      {
         SequenceInfo* sequence_info = &((*time_data_info)[i]);
         for (j=0; j<(int)sequence_info->frames.size(); j++)
         {
            FrameInfo *frame_info = &((sequence_info->frames)[j]);

            if (c >= frame_info->channels.size())
               continue;

            long frame_size = frame_info->x_size * frame_info->y_size;
            unsigned short *frame_val = new (std::nothrow) unsigned short[frame_size];
            if (!val) return 0;

            char *pbyData = 0;
            wstring file_name = frame_info->channels[c].file_name;

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
               ReadTiff(pbyData, frame_val);

               if (pbyData)
                  delete []pbyData;

               //copy frame val to val
               long long index = m_x_size*m_y_size*frame_info->z + m_x_size*(m_y_size-frame_info->y-frame_info->y_size) + frame_info->x;
               long frame_index = 0;
					if (m_flip_y)
						frame_index = frame_info->x_size * (frame_info->y_size-1);
					for (k=0; k<frame_info->y_size; k++)
					{
						memcpy((void*)(val+index), (void*)(frame_val+frame_index), frame_info->x_size*sizeof(unsigned short));
						index += m_x_size;
						if (m_flip_y)
							frame_index -= frame_info->x_size;
						else
							frame_index += frame_info->x_size;
               }
            }

            if (frame_val)
               delete []frame_val;
         }
      }

      if (val)
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

   m_cur_time = t;
   if (m_max_value > 0.0)
      m_scalar_scale = 65535.0 / m_max_value;

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
   return data;
}

void PVXMLReader::ReadTiff(char *pbyData, unsigned short *val)
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
   int width = 0;
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
      case 0x0100:  //256, image width
         {
            unsigned short type = *((unsigned short*)(pbyData+offset+2+12*i+2));
            if (type == 3)
            {
               //unsigned short
               unsigned short value;
               value = *((unsigned short*)(pbyData+offset+2+12*i+8));
               width = value;
            }
            else if (type == 4)
            {
               //unsigned int
               unsigned int value;
               value = *((unsigned int*)(pbyData+offset+2+12*i+8));
               width = value;
            }
         }
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

      unsigned int val_pos = 0;
      for (int i=0; i<strips; i++)
      {
         unsigned int data_pos = strip_offsets[i];
         unsigned int data_size = strip_bytes[i];
         if (compression == 1)//no copmression
            memcpy((void*)(val+val_pos), (void*)(pbyData+data_pos), data_size);
         else if (compression == 5)
            LZWDecode((tidata_t)(pbyData+data_pos), (tidata_t)(val+val_pos), m_x_size*rows*2);
         val_pos += rows*width;
      }
   }
}

wstring PVXMLReader::GetCurName(int t, int c)
{
   return wstring(L"");
}

