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
#include <pvxml_reader.h>
#include <Utils.h>
#include <compatibility.h>
#include <XmlUtils.h>
#include <fstream>
#include <iostream>

PVXMLReader::PVXMLReader() :
	BaseReader()
{
	m_force_stack = false;

	m_resize_type = 0;
	m_resample_type = 0;
	m_alignment = 0;

	m_time_num = 0;
	m_cur_time = -1;
	m_chan_num = 0;
	m_group_num = 0;
	m_slice_num = 0;
	m_x_size = 0;
	m_y_size = 0;

	m_valid_spc = false;
	m_xspc = 0.0;
	m_yspc = 0.0;
	m_zspc = 0.0;

	m_min_value = 0.0;
	m_max_value = 0.0;
	m_scalar_scale = 1.0;

	m_fp_convert = false;
	m_fp_min = 0;
	m_fp_max = 1;

	m_batch = false;
	m_cur_batch = -1;

	//falgs for flipping
	m_user_flip_x = 0;
	m_user_flip_y = 0;
	m_flip_x = false;
	m_flip_y = false;
	m_seq_type = 0;
}

PVXMLReader::~PVXMLReader()
{
}

//void PVXMLReader::SetFile(const std::string& file)
//{
//	if (!file.empty())
//	{
//		if (!m_path_name.empty())
//			m_path_name.clear();
//		m_path_name.assign(file.length(), L' ');
//		copy(file.begin(), file.end(), m_path_name.begin());
//		m_data_name = GET_STEM(m_path_name);
//	}
//	m_id_string = m_path_name;
//}

void PVXMLReader::SetFile(const std::wstring& file)
{
	m_path_name = file;
	m_data_name = GET_STEM(m_path_name);
	m_id_string = m_path_name;
}

int PVXMLReader::Preprocess()
{
	m_pvxml_info.clear();
	m_slice_num = 0;
	m_chan_num = 0;
	m_group_num = 0;
	m_min_value = 0.0;
	m_max_value = 0.0;
	m_seq_boxes.clear();
	m_force_stack = false;

	//separate path and name
	std::wstring path, name;
	if (!SEP_PATH_NAME(m_path_name, path, name))
		return READER_OPEN_FAIL;

	tinyxml2::XMLDocument doc;
	std::string str = ws2s(m_path_name);
	if (doc.LoadFile(str.c_str()) != tinyxml2::XML_SUCCESS)
		return READER_OPEN_FAIL;

	tinyxml2::XMLElement* root = doc.RootElement();

	if (!root || std::string(root->Name()) != "PVScan")
		return READER_FORMAT_ERROR;

	tinyxml2::XMLElement* child = root->FirstChildElement();
	while (child)
	{
		if (std::string(child->Name()) == "SystemConfiguration")
			ReadSystemConfig(child);
		else if (std::string(child->Name()) == "PVStateShard")
		{
			UpdateStateShard(child);
			m_state_shard_stack.push_back(m_current_state);
		}
		else if (std::string(child->Name()) == "Sequence")
			ReadSequence(child);
		child = child->NextSiblingElement();
	}

	m_time_num = int(m_pvxml_info.size());
	m_cur_time = 0;

	if (m_time_num == 0) return READER_EMPTY_DATA;

	size_t i, j, k, l;
	double x_end, y_end, z_end;
	std::wstring filename;
	bool first = true;
	m_sep_seq = false;
	for (i = 0; i < m_pvxml_info.size(); i++)
	{
		TimeDataInfo* time_data_info = &(m_pvxml_info[i]);
		int chan_num = 0;
		for (j = 0; j < time_data_info->size(); j++)
		{
			SequenceInfo* sequence_info = &((*time_data_info)[j]);
			m_sep_seq = m_sep_seq || sequence_info->apart;
			for (k = 0; k < sequence_info->frames.size(); k++)
			{
				FrameInfo* frame_info = &((sequence_info->frames)[k]);
				x_end = frame_info->x_start + frame_info->x_size * m_xspc;
				y_end = frame_info->y_start + frame_info->y_size * m_yspc;
				z_end = frame_info->z_start + m_zspc;

				if (k == 0)
					chan_num += static_cast<int>(frame_info->channels.size());

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
					if (m_sep_seq)
					{
						double x_extend = frame_info->x_start - x_end;
						double y_extend = frame_info->y_start - y_end;
						double z_extend = frame_info->z_start - z_end;
						m_x_max = x_extend > (m_x_max - m_x_min) ? m_x_min + x_extend : m_x_max;
						m_y_max = y_extend > (m_y_max - m_y_min) ? m_y_min + y_extend : m_y_max;
						m_z_max = z_extend > (m_z_max - m_z_min) ? m_z_min + z_extend : m_z_max;
					}
					else
					{
						m_x_min = frame_info->x_start < m_x_min ? frame_info->x_start : m_x_min;
						m_y_min = frame_info->y_start < m_y_min ? frame_info->y_start : m_y_min;
						m_z_min = frame_info->z_start < m_z_min ? frame_info->z_start : m_z_min;
						m_x_max = x_end > m_x_max ? x_end : m_x_max;
						m_y_max = y_end > m_y_max ? y_end : m_y_max;
						m_z_max = z_end > m_z_max ? z_end : m_z_max;
					}
				}

				//append path
				for (l = 0; l < frame_info->channels.size(); l++)
				{
					filename = path + frame_info->channels[l].file_name;
					frame_info->channels[l].file_name = filename;
				}
			}
		}

		//correct chan num
		if (m_sep_seq)
		{
			m_group_num = chan_num > m_group_num ? chan_num : m_group_num;
		}
	}

	//make sure not divide by zero
	m_xspc = m_xspc == 0.0 ? 1.0 : m_xspc;
	m_yspc = m_yspc == 0.0 ? 1.0 : m_yspc;
	m_zspc = m_zspc == 0.0 ? 1.0 : (m_zspc == FLT_MAX ? 1.0 : m_zspc);

	m_x_size = int((m_x_max - m_x_min) / m_xspc + 0.5);
	m_y_size = int((m_y_max - m_y_min) / m_yspc + 0.5);
	if (m_z_max == FLT_MAX) m_z_max = m_seq_slice_num;
	double dt = m_z_max - m_z_min;
	if (std::abs(dt) > fluo::Epsilon(10))
		m_slice_num = int(dt / m_zspc + 0.5);

	if (m_user_flip_y == 1 ||
		m_user_flip_y == 0)
		m_flip_y = false;
	else if (m_user_flip_y == -1)
		m_flip_y = true;
	int y0;
	bool firsty = true;
	bool flipy = false;
	for (i = 0; i < (int)m_pvxml_info.size(); i++)
	{
		TimeDataInfo* time_data_info = &(m_pvxml_info[i]);
		for (j = 0; j < (int)time_data_info->size(); j++)
		{
			SequenceInfo* sequence_info = &((*time_data_info)[j]);
			for (k = 0; k < (int)sequence_info->frames.size(); k++)
			{
				FrameInfo* frame_info = &((sequence_info->frames)[k]);
				if (m_sep_seq)
				{
					frame_info->x = 0;
					frame_info->y = 0;
					frame_info->z = static_cast<int>(k);
				}
				else
				{
					frame_info->x = int((frame_info->x_start - m_x_min) / m_xspc + 0.5);
					frame_info->y = int((frame_info->y_start - m_y_min) / m_yspc + 0.5);
					if (m_force_stack)
						frame_info->z = static_cast<int>(k);
					else
					{
						double dt = frame_info->z_start - m_z_min;
						if (std::abs(dt) > fluo::Epsilon(10))
							frame_info->z = int(dt / m_zspc + 0.5);
						else frame_info->z = static_cast<int>(k);
					}
				}
				if (m_user_flip_y == 0 && !flipy)
				{
					if (firsty)
					{
						y0 = frame_info->y;
						firsty = false;
					}
					else
					{
						if (frame_info->y > y0 + int(frame_info->y_size * 0.1))
						{
							m_flip_y = true;
							flipy = true;
						}
						else if (frame_info->y < y0 - int(frame_info->y_size * 0.1))
							flipy = true;
					}
				}
			}
		}
	}

	return READER_OK;
}

void PVXMLReader::ReadSystemConfig(tinyxml2::XMLElement* systemNode)
{
}

void PVXMLReader::UpdateStateShard(tinyxml2::XMLElement* stateNode)
{
	if (m_state_shard_stack.size())
		m_current_state = m_state_shard_stack.back();
	tinyxml2::XMLElement* child = stateNode->FirstChildElement();
	while (child)
	{
		std::string child_name(child->Name());
		if (child_name == "Key" ||
			child_name == "PVStateValue")
			ReadKey(child);
		child = child->NextSiblingElement();
	}
}

void PVXMLReader::ReadKey(tinyxml2::XMLElement* keyNode)
{
	int ival;
	double dval;

	std::string strKey = GetAttributeValue(keyNode, "key");
	std::string strValue = GetAttributeValue(keyNode, "value");

	if (strKey == "xYStageGridIndex")
	{
		ival = STOI(strValue);
		m_current_state.grid_index = ival;
	}
	else if (strKey == "xYStageGridXIndex")
	{
		ival = STOI(strValue);
		m_current_state.grid_index_x = ival;
	}
	else if (strKey == "xYStageGridYIndex")
	{
		ival = STOI(strValue);
		m_current_state.grid_index_y = ival;
	}
	else if (strKey == "positionCurrent_XAxis")
	{
		dval = STOD(strValue);
		m_current_state.pos_x = dval;
	}
	else if (strKey == "positionCurrent_YAxis")
	{
		dval = STOD(strValue);
		m_current_state.pos_y = dval;
	}
	else if (strKey == "positionCurrent_ZAxis")
	{
		size_t pos = strValue.find(',');
		if (pos == std::string::npos)
		{
			dval = STOD(strValue);
			m_current_state.pos_z = dval;
		}
		else
		{
			m_current_state.pos_z = 0.0;
			do
			{
				dval = STOD(strValue.substr(0, pos));
				m_current_state.pos_z += dval;
				strValue = strValue.substr(pos + 1);
				pos = strValue.find(',');
			} while (pos != std::string::npos);
			dval = STOD(strValue);
			if (!strValue.empty() && dval)
				m_current_state.pos_z += dval;
		}
	}
	else if (strKey == "positionCurrent")
	{
		ReadIndexedKey(keyNode, strKey);
	}
	else if (strKey == "zDevice")
	{
		ival = STOI(strValue);
		m_current_state.z_device = ival;
	}
	else if (strKey == "pixelsPerLine")
	{
		ival = STOI(strValue);
		m_current_state.ppl = ival;
	}
	else if (strKey == "linesPerFrame")
	{
		ival = STOI(strValue);
		m_current_state.lpf = ival;
	}
	else if (strKey == "micronsPerPixel_XAxis")
	{
		dval = STOD(strValue);
		m_current_state.mpp_x = dval;
	}
	else if (strKey == "micronsPerPixel_YAxis")
	{
		dval = STOD(strValue);
		m_current_state.mpp_y = dval;
	}
	else if (strKey == "micronsPerPixel")
	{
		ReadIndexedKey(keyNode, strKey);
	}
	else if (strKey == "bitDepth")
	{
		ival = STOI(strValue);
		m_current_state.bit_depth = ival;
	}
	else if (strKey == "seqType")
	{
		ival = STOI(strValue);
		m_seq_type = ival;
	}
	else if (strKey == "laserPower")
	{
		ReadLaser(keyNode);
	}
}

void PVXMLReader::ReadIndexedKey(tinyxml2::XMLElement* keyNode, const std::string& key)
{
	double dval;

	if (key == "positionCurrent")
	{
		tinyxml2::XMLElement* child = keyNode->FirstChildElement();
		while (child)
		{
			std::string child_name(child->Name());
			if (child_name == "SubindexedValues")
			{
				std::string strIndex = GetAttributeValue(child, "index");
				tinyxml2::XMLElement* gchild = child->FirstChildElement();
				while (gchild)
				{
					std::string strSubIndex = GetAttributeValue(gchild, "subindex");
					std::string strValue = GetAttributeValue(gchild, "value");
					if (strSubIndex == "0")
					{
						if (strIndex == "XAxis")
						{
							dval = STOD(strValue);
							m_current_state.pos_x = dval;
						}
						else if (strIndex == "YAxis")
						{
							dval = STOD(strValue);
							m_current_state.pos_y = dval;
						}
						else if (strIndex == "ZAxis")
						{
							dval = STOD(strValue);
							m_current_state.pos_z = dval;
						}
					}
					gchild = gchild->NextSiblingElement();
				}
			}
			child = child->NextSiblingElement();
		}
	}
	else if (key == "micronsPerPixel")
	{
		tinyxml2::XMLElement* child = keyNode->FirstChildElement();
		while (child)
		{
			std::string child_name(child->Name());
			if (child_name == "IndexedValue")
			{
				std::string strIndex = GetAttributeValue(child, "index");
				std::string strValue = GetAttributeValue(child, "value");
				if (strIndex == "XAxis")
				{
					dval = STOD(strValue);
					m_current_state.mpp_x = dval;
				}
				else if (strIndex == "YAxis")
				{
					dval = STOD(strValue);
					m_current_state.mpp_y = dval;
				}
			}
			child = child->NextSiblingElement();
		}
	}
}

void PVXMLReader::ReadSequence(tinyxml2::XMLElement* seqNode)
{
	//get type
	std::string type = GetAttributeValue(seqNode, "type");
	if (type == "TSeries Timed Element" &&
		!m_seq_type)
		m_seq_type = 2;

	if (m_seq_type == 1)
	{
		if (!m_force_stack)
		{
			m_force_stack = true;
			m_new_seq = false;
			m_seq_slice_num = 0;
		}
	}
	else
	{
		m_new_seq = true;
		m_seq_slice_num = 0;
	}
	m_seq_zspc = FLT_MAX;
	m_seq_zpos = 0.0;
	tinyxml2::XMLElement* child = seqNode->FirstChildElement();
	int stack_push_count = 0;
	while (child)
	{
		std::string strName(child->Name());
		if (strName == "PVStateShard")
		{
			UpdateStateShard(child);
			m_state_shard_stack.push_back(m_current_state);
			stack_push_count++;
		}
		else if (strName == "Frame")
		{
			if (m_seq_type == 2)
			{
				m_new_seq = true;
				ReadFrame(child);
			}
			else
			{
				ReadFrame(child);
				m_seq_slice_num++;
			}
		}
		child = child->NextSiblingElement();
	}
	//pop all stacked states
	for (int i = 0; i < stack_push_count; i++)
		m_state_shard_stack.pop_back();

	if (m_slice_num)
	{
		m_slice_num = m_seq_slice_num > m_slice_num ? m_seq_slice_num : m_slice_num;
		if (m_seq_zspc > 0.0)
		{
			if (m_zspc == 0.0)
				m_zspc = m_seq_zspc;
			else
				m_zspc = m_seq_zspc < m_zspc ? m_seq_zspc : m_zspc;
		}
	}
	else
	{
		if (m_seq_type == 2)
			m_slice_num = 1;
		else
			m_slice_num = m_seq_slice_num;
		m_zspc = m_seq_zspc;
	}
	m_pvxml_info.back().back().grid_index = m_current_state.grid_index;
}

void PVXMLReader::ReadFrame(tinyxml2::XMLElement* frameNode)
{
	FrameInfo frame_info;

	tinyxml2::XMLElement* child = frameNode->FirstChildElement();
	while (child)
	{
		std::string strName(child->Name());
		if (strName == "File")
		{
			std::string channel = GetAttributeValue(child, "channel");
			int chn = STOI(channel);
			std::string filename = GetAttributeValue(child, "filename");

			ChannelInfo channel_info;
			channel_info.file_name = s2ws(filename);
			channel_info.chan = chn;
			frame_info.channels.push_back(channel_info);
			int size = static_cast<int>(frame_info.channels.size());
			m_chan_num = size > m_chan_num ? size : m_chan_num;
		}
		else if (strName == "PVStateShard")
			UpdateStateShard(child);

		child = child->NextSiblingElement();
	}

	frame_info.x_size = m_current_state.ppl;
	frame_info.y_size = m_current_state.lpf;
	frame_info.x_start = m_current_state.pos_x;
	frame_info.y_start = m_current_state.pos_y;
	frame_info.z_start = m_current_state.pos_z;

	if (m_seq_zpos != 0.0)
	{
		double spc = fabs(frame_info.z_start - m_seq_zpos);
		m_seq_zspc = spc < m_seq_zspc ? spc : m_seq_zspc;
	}
	m_seq_zpos = frame_info.z_start;
	if (m_xspc == 0.0)
	{
		m_xspc = m_current_state.mpp_x;
		m_yspc = m_current_state.mpp_y;
	}
	else
	{
		m_xspc = m_current_state.mpp_x < m_xspc ? m_current_state.mpp_x : m_xspc;
		m_yspc = m_current_state.mpp_y < m_yspc ? m_current_state.mpp_y : m_yspc;
	}

	bool apart = false;
	if (m_new_seq)
	{
		SeqBox sb;
		sb.x_min = frame_info.x_start;
		sb.x_max = sb.x_min + frame_info.x_size * m_current_state.mpp_x;
		sb.y_min = frame_info.y_start;
		sb.y_max = sb.y_min + frame_info.y_size * m_current_state.mpp_y;
		bool overlap = false;
		double ol_value;
		apart = m_seq_boxes.size() > 0 ? true : false;
		for (unsigned int i = 0; i < m_seq_boxes.size(); ++i)
		{
			ol_value = sb.overlaps(m_seq_boxes[i]);
			if (ol_value >= 0.0)
				apart = false;
			if (ol_value >= 0.9)
			{
				overlap = true;
				m_seq_boxes.clear();
				break;
			}
		}
		m_seq_boxes.push_back(sb);

		if (!m_pvxml_info.size() || overlap)
		{
			TimeDataInfo info_new;
			m_pvxml_info.push_back(info_new);
		}
		m_new_seq = false;
	}

	if (m_force_stack && m_pvxml_info.empty())
	{
		TimeDataInfo info_new;
		m_pvxml_info.push_back(info_new);
	}

	TimeDataInfo* time_data_info = &(m_pvxml_info.back());
	if (!m_seq_slice_num)
	{
		SequenceInfo info_new;
		info_new.grid_index = 0;
		info_new.apart = false;
		time_data_info->push_back(info_new);
	}
	SequenceInfo* sequence_info = &(time_data_info->back());
	if (sequence_info)
	{
		sequence_info->apart = sequence_info->apart || apart;
		sequence_info->frames.push_back(frame_info);
	}
	//if (m_force_stack)
	//	m_seq_slice_num++;
}

void PVXMLReader::SetBatch(bool batch)
{
	if (batch)
	{
		//read the directory info
		FIND_FILES_BATCH(m_path_name, ESCAPE_REGEX(L".xml"), m_batch_list, m_cur_batch);
		m_batch = true;
	}
	else
		m_batch = false;
}

int PVXMLReader::LoadBatch(int index)
{
	int result = -1;
	if (index >= 0 && index < (int)m_batch_list.size())
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

double PVXMLReader::GetExcitationWavelength(int chan)
{
	if (chan < 0)
		return 0.0;
	//get channel from info
	if (!m_pvxml_info.empty())
	{
		TimeDataInfo* ti = &(m_pvxml_info[0]);
		if (ti && !ti->empty())
		{
			if (chan < (*ti)[0].frames[0].channels.size())
				chan = (*ti)[0].frames[0].channels[chan].chan;
		}
	}
	for (int i = 0; i < (int)m_excitation_wavelength_list.size(); i++)
	{
		if (m_excitation_wavelength_list[i].chan_num == chan)
			return m_excitation_wavelength_list[i].wavelength;
	}
	return 0.0;
}

bool PVXMLReader::ConvertN(int c, TimeDataInfo* time_data_info, unsigned short* val)
{
	int i, j, k;
	for (i = 0; i < (int)time_data_info->size(); i++)
	{
		SequenceInfo* sequence_info = &((*time_data_info)[i]);
		for (j = 0; j < (int)sequence_info->frames.size(); j++)
		{
			FrameInfo* frame_info = &((sequence_info->frames)[j]);

			if ((size_t)c >= frame_info->channels.size())
				continue;

			unsigned long long frame_size = (unsigned long long)(frame_info->x_size) *
				(unsigned long long)(frame_info->y_size);
			unsigned short* frame_val = new (std::nothrow) unsigned short[frame_size];
			if (!val) return 0;

			char* pbyData = 0;
			std::wstring file_name = frame_info->channels[c].file_name;

			//open file
			std::ifstream is;
#ifdef _WIN32
			is.open(file_name.c_str(), std::ios::binary);
#else
			is.open(ws2s(file_name).c_str(), std::ios::binary);
#endif
			if (is.is_open())
			{
				is.seekg(0, std::ios::end);
				size_t size = is.tellg();
				pbyData = new char[size];
				is.seekg(0, std::ios::beg);
				is.read(pbyData, size);
				is.close();

				//read
				ReadTiff(pbyData, frame_val);

				if (pbyData)
					delete[] pbyData;

				//copy frame val to val
				unsigned long long index = (unsigned long long)m_x_size * m_y_size * frame_info->z + m_x_size * (m_y_size - frame_info->y - frame_info->y_size) + frame_info->x;
				long frame_index = 0;
				if (m_flip_y)
					frame_index = frame_info->x_size * (frame_info->y_size - 1);
				for (k = 0; k < frame_info->y_size; k++)
				{
					memcpy((void*)(val + index), (void*)(frame_val + frame_index), frame_info->x_size * sizeof(unsigned short));
					index += m_x_size;
					if (m_flip_y)
						frame_index -= frame_info->x_size;
					else
						frame_index += frame_info->x_size;
				}
			}

			if (frame_val)
				delete[] frame_val;
		}
	}

	return true;
}

bool PVXMLReader::ConvertS(int c, TimeDataInfo* time_data_info, unsigned short* val)
{
	int cur_chan = 0;
	size_t i, j, k;
	for (i = 0; i < time_data_info->size(); ++i)
	{
		if (c >= cur_chan && c < cur_chan + m_chan_num)
		{
			int index = c - cur_chan;
			SequenceInfo* sequence_info = &((*time_data_info)[i]);

			for (j = 0; j < (int)sequence_info->frames.size(); j++)
			{
				FrameInfo* frame_info = &((sequence_info->frames)[j]);
				if ((size_t)index >= frame_info->channels.size())
					continue;

				unsigned long long frame_size = (unsigned long long)(frame_info->x_size) *
					(unsigned long long)(frame_info->y_size);
				unsigned short* frame_val = new (std::nothrow) unsigned short[frame_size];
				if (!val) return 0;

				char* pbyData = 0;
				std::wstring file_name = frame_info->channels[index].file_name;

				//open file
				std::ifstream is;
#ifdef _WIN32
				is.open(file_name.c_str(), std::ios::binary);
#else
				is.open(ws2s(file_name).c_str(), std::ios::binary);
#endif
				if (is.is_open())
				{
					is.seekg(0, std::ios::end);
					size_t size = is.tellg();
					pbyData = new char[size];
					is.seekg(0, std::ios::beg);
					is.read(pbyData, size);
					is.close();

					//read
					ReadTiff(pbyData, frame_val);

					if (pbyData)
						delete[] pbyData;

					//copy frame val to val
					unsigned long long index = (unsigned long long)m_x_size * m_y_size * frame_info->z + m_x_size * (m_y_size - frame_info->y - frame_info->y_size) + frame_info->x;
					long frame_index = 0;
					if (m_flip_y)
						frame_index = frame_info->x_size * (frame_info->y_size - 1);
					for (k = 0; k < frame_info->y_size; k++)
					{
						memcpy((void*)(val + index), (void*)(frame_val + frame_index), frame_info->x_size * sizeof(unsigned short));
						index += m_x_size;
						if (m_flip_y)
							frame_index -= frame_info->x_size;
						else
							frame_index += frame_info->x_size;
					}
				}

				if (frame_val)
					delete[] frame_val;
			}

			break;
		}
		cur_chan += m_chan_num;
	}
	return true;
}

Nrrd* PVXMLReader::Convert(int t, int c, bool get_max)
{
	Nrrd* data = 0;

	int chan_num = m_sep_seq ? m_group_num : m_chan_num;
	if (t >= 0 && t < m_time_num &&
		c >= 0 && c < chan_num &&
		m_slice_num>0 &&
		m_x_size>0 &&
		m_y_size > 0)
	{
		//allocate memory for nrrd
		unsigned long long mem_size = (unsigned long long)m_x_size *
			(unsigned long long)m_y_size * (unsigned long long)m_slice_num;
		unsigned short* val = new (std::nothrow) unsigned short[mem_size]();
		if (!val) return 0;

		TimeDataInfo* time_data_info = &(m_pvxml_info[t]);

		if (m_sep_seq)
			ConvertS(c, time_data_info, val);
		else
			ConvertN(c, time_data_info, val);

		if (val)
		{
			//ok
			data = nrrdNew();
			nrrdWrap_va(data, val, nrrdTypeUShort, 3, (size_t)m_x_size, (size_t)m_y_size, (size_t)m_slice_num);
			nrrdAxisInfoSet_va(data, nrrdAxisInfoSpacing, m_xspc, m_yspc, m_zspc);
			nrrdAxisInfoSet_va(data, nrrdAxisInfoMax, m_xspc * m_x_size, m_yspc * m_y_size, m_zspc * m_slice_num);
			nrrdAxisInfoSet_va(data, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
			nrrdAxisInfoSet_va(data, nrrdAxisInfoSize, (size_t)m_x_size, (size_t)m_y_size, (size_t)m_slice_num);

			if (get_max)
			{
				double value;
				unsigned long long totali = (unsigned long long)m_slice_num *
					m_x_size * m_y_size;
				for (unsigned long long i = 0; i < totali; ++i)
				{
					value = val[i];
					m_min_value = i == 0 ? value : (value < m_min_value ? value : m_min_value);
					m_max_value = value > m_max_value ? value : m_max_value;
				}
			}
		}
	}

	m_cur_time = t;
	if (m_max_value > 0.0)
		m_scalar_scale = 65535.0 / m_max_value;

	if (m_xspc > 0.0 && m_xspc < 100.0 &&
		m_yspc>0.0 && m_yspc < 100.0)
	{
		m_valid_spc = true;
		if (m_zspc <= 0.0 || m_zspc > 100.0)
			m_zspc = std::max(m_xspc, m_yspc);
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

void PVXMLReader::ReadTiff(char* pbyData, unsigned short* val)
{
	if (*((unsigned int*)pbyData) != 0x002A4949)
		return;

	int compression = 0;
	unsigned int offset = 0;
	//directory offset
	offset = *((unsigned int*)(pbyData + 4));
	//the directory
	//entry number
	int entry_num = *((unsigned short*)(pbyData + offset));
	//strip info
	int strips = 0;
	int rows = 0;
	int width = 0;
	std::vector <unsigned int> strip_offsets;
	std::vector <unsigned int> strip_bytes;
	//get strip info
	unsigned int s_num1 = 0;
	unsigned int s_num2 = 0;
	for (int i = 0; i < entry_num; i++)
	{
		//read each entry (12 bytes)
		unsigned short tag = *((unsigned short*)(pbyData + offset + 2 + 12 * i));
		switch (tag)
		{
		case 0x0100:  //256, image width
		{
			unsigned short type = *((unsigned short*)(pbyData + offset + 2 + 12 * i + 2));
			if (type == 3)
			{
				//unsigned short
				unsigned short value;
				value = *((unsigned short*)(pbyData + offset + 2 + 12 * i + 8));
				width = value;
			}
			else if (type == 4)
			{
				//unsigned int
				unsigned int value;
				value = *((unsigned int*)(pbyData + offset + 2 + 12 * i + 8));
				width = value;
			}
		}
		break;
		case 0x0103:  //259, compression
		{
			unsigned short value;
			value = *((unsigned short*)(pbyData + offset + 2 + 12 * i + 8));
			compression = value << 16 >> 16;
		}
		break;
		case 0x0111:  //strip offsets
		{
			unsigned short type = *((unsigned short*)(pbyData + offset + 2 + 12 * i + 2));
			//number of values
			s_num1 = *((unsigned int*)(pbyData + offset + 2 + 12 * i + 4));
			unsigned int entry_offset = 0;
			entry_offset = *((unsigned int*)(pbyData + offset + 2 + 12 * i + 8));
			if (s_num1 == 1)
			{
				strip_offsets.push_back(entry_offset);
			}
			else
			{
				for (int j = 0; j<int(s_num1); j++)
				{
					if (type == 3)
					{
						//unsigned short
						unsigned short value;
						value = *((unsigned short*)(pbyData + entry_offset + 2 * j));
						strip_offsets.push_back((unsigned int)value);
					}
					else if (type == 4)
					{
						//unsigned int
						unsigned int value;
						value = *((unsigned int*)(pbyData + entry_offset + 4 * j));
						strip_offsets.push_back(value);
					}
				}
			}
		}
		break;
		case 0x0116:  //rows per strip
		{
			unsigned short type = *((unsigned short*)(pbyData + offset + 2 + 12 * i + 2));
			if (type == 3)
			{
				//unsigned short
				unsigned short value;
				value = *((unsigned short*)(pbyData + offset + 2 + 12 * i + 8));
				rows = value;
			}
			else if (type == 4)
			{
				//unsigned int
				unsigned int value;
				value = *((unsigned int*)(pbyData + offset + 2 + 12 * i + 8));
				rows = value;
			}
		}
		break;
		case 0x0117:  //strip byte counts
		{
			unsigned short type = *((unsigned short*)(pbyData + offset + 2 + 12 * i + 2));
			//number of values
			s_num2 = *((unsigned int*)(pbyData + offset + 2 + 12 * i + 4));
			unsigned int entry_offset = 0;
			entry_offset = *((unsigned int*)(pbyData + offset + 2 + 12 * i + 8));
			if (s_num2 == 1)
			{
				strip_bytes.push_back(entry_offset);
			}
			else
			{
				for (int j = 0; j<int(s_num2); j++)
				{
					if (type == 3)
					{
						//unsigned short
						unsigned short value;
						value = *((unsigned short*)(pbyData + entry_offset + 2 * j));
						strip_bytes.push_back((unsigned int)value);
					}
					else if (type == 4)
					{
						//unsigned int
						unsigned int value;
						value = *((unsigned int*)(pbyData + entry_offset + 4 * j));
						strip_bytes.push_back(value);
					}
				}
			}
		}
		break;
		case 0x0119:  //max sample value
		{
			unsigned short value;
			value = *((unsigned short*)(pbyData + offset + 2 + 12 * i + 8));
			//if ((double)value > m_max_value)
			m_min_value = m_min_value == 0.0 ? value : (value < m_min_value ? value : m_min_value);
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
		for (int i = 0; i < strips; i++)
		{
			unsigned int data_pos = strip_offsets[i];
			unsigned int data_size = strip_bytes[i];
			if (compression == 1)//no copmression
				memcpy((void*)(val + val_pos), (void*)(pbyData + data_pos), data_size);
			else if (compression == 5)
				LZWDecode((tidata_t)(pbyData + data_pos), (tidata_t)(val + val_pos), m_x_size * rows * 2);
			val_pos += rows * width;
		}
	}
}

void PVXMLReader::ReadLaser(tinyxml2::XMLElement* node)
{
	tinyxml2::XMLElement* child = node->FirstChildElement();
	while (child)
	{
		std::string child_name(child->Name());
		if (child_name == "IndexedValue")
		{
			std::string strIndex = GetAttributeValue(child, "index");
			std::string strValue = GetAttributeValue(child, "value");
			std::string strDesc = GetAttributeValue(child, "description");
			int ch = STOI(strIndex);
			int wl = 0;
			std::string strWl;
			if (strDesc.find("LED") != std::string::npos ||
				strDesc.find("Pockels") != std::string::npos)
				wl = -1;
			else
			{
				for (int i = 0; i < strDesc.size(); ++i)
				{
					if (std::isdigit(strDesc[i]))
						strWl += strDesc[i];
					else
						break;
				}
				wl = STOI(strWl);
			}
			WavelengthInfo winfo;
			winfo.chan_num = ch;
			winfo.wavelength = wl;
			m_excitation_wavelength_list.push_back(winfo);
		}
		child = child->NextSiblingElement();
	}
}

std::wstring PVXMLReader::GetCurDataName(int t, int c)
{
	return m_path_name;
}

std::wstring PVXMLReader::GetCurMaskName(int t, int c)
{
	std::wostringstream woss;
	woss << m_path_name.substr(0, m_path_name.find_last_of(L'.'));
	if (m_time_num > 1) woss << L"_T" << t;
	if (m_chan_num > 1) woss << L"_C" << c;
	woss << L".msk";
	std::wstring mask_name = woss.str();
	return mask_name;
}

std::wstring PVXMLReader::GetCurLabelName(int t, int c)
{
	std::wostringstream woss;
	woss << m_path_name.substr(0, m_path_name.find_last_of(L'.'));
	if (m_time_num > 1) woss << L"_T" << t;
	if (m_chan_num > 1) woss << L"_C" << c;
	woss << L".lbl";
	std::wstring label_name = woss.str();
	return label_name;
}
