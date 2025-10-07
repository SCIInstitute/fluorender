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
#include <lsm_reader.h>
#include <Global.h>
#include <MainSettings.h>
#include <compatibility.h>

LSMReader::LSMReader():
	BaseVolReader()
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

	m_min_value = 0.0;
	m_max_value = 0.0;
	m_scalar_scale = 1.0;

	m_fp_convert = false;
	m_fp_min = 0;
	m_fp_max = 1;

	m_batch = false;
	m_cur_batch = -1;

	m_compression = 0;
	m_predictor = 0;
	m_version = 0;
	m_datatype = 0;
	m_l4gb = false;
}

LSMReader::~LSMReader()
{
}

//void LSMReader::SetFile(const std::string &file)
//{
//	if (!file.empty())
//	{
//		if (!m_path_name.empty())
//			m_path_name.clear();
//		m_path_name.assign(file.length(), L' ');
//		copy(file.begin(), file.end(), m_path_name.begin());
//	}
//	m_id_string = m_path_name;
//}

void LSMReader::SetFile(const std::wstring &file)
{
	m_path_name = file;
	m_id_string = m_path_name;
}

int LSMReader::Preprocess()
{
	FILE* pfile = 0;
	if (!WFOPEN(&pfile, m_path_name, L"rb"))
		return READER_OPEN_FAIL;

	m_lsm_info.clear();
	m_l4gb = false;

	int i, j;

	unsigned int ioffset = 0;
	fread(&ioffset, sizeof(unsigned int), 1, pfile);
	if (ioffset != 0x002A4949)
	{
		fclose(pfile);
		return READER_FORMAT_ERROR;
	}

	int cnt_image = 0;
	bool full_image = false;

	//first offset
	if (fread(&ioffset, sizeof(unsigned int), 1, pfile) < 1)
	{
		fclose(pfile);
		return READER_FORMAT_ERROR;
	}

	unsigned int prev_offset = 0;
	unsigned int offset_high = 0;

	//images
	while (FSEEK64(pfile, ioffset, SEEK_SET) == 0)
	{
		unsigned short entry_num;
		//entry number
		if (fread(&entry_num, sizeof(unsigned short), 1, pfile) < 1)
		{
			fclose(pfile);
			return READER_FORMAT_ERROR;
		}

		std::vector<unsigned int> offsets;
		std::vector<unsigned int> offset_highs;
		std::vector<unsigned int> sizes;

		for (i = 0; i < entry_num; i++)
		{
			unsigned short tag;
			if (fread(&tag, sizeof(unsigned short), 1, pfile) < 1)
			{
				fclose(pfile);
				return READER_FORMAT_ERROR;
			}
			unsigned short type;
			if (fread(&type, sizeof(unsigned short), 1, pfile) < 1)
			{
				fclose(pfile);
				return READER_FORMAT_ERROR;
			}
			unsigned int length;
			if (fread(&length, sizeof(unsigned int), 1, pfile) < 1)
			{
				fclose(pfile);
				return READER_FORMAT_ERROR;
			}
			unsigned int value;
			if (fread(&value, sizeof(unsigned int), 1, pfile) < 1)
			{
				fclose(pfile);
				return READER_FORMAT_ERROR;
			}

			//remember current position
			long cur_pos = ftell(pfile);

			switch (tag)
			{
			case 0x00FE://254, new subfile type
				if (value == 0)
				{
					full_image = true;
					cnt_image++;
				}
				else
					full_image = false;
				break;
			case 0x0100://256, image width
				if (full_image && cnt_image == 1)
					m_x_size = value;
				break;
			case 0x0101://257, image length
				if (full_image && cnt_image == 1)
					m_y_size = value;
				break;
			case 0x0102://258, bits per sample
				break;
			case 0x0103://259, compression
				if (full_image && cnt_image == 1)
					m_compression = value << 16 >> 16;
				break;
			case 0x0106://262, photometric interpretation
				break;
			case 0x0111://273, strip offsets
				if (full_image)
				{
					if (length == 1)
					{
						offsets.push_back(value);
						if (value < prev_offset)
						{
							m_l4gb = true;
							offset_high++;
						}
						prev_offset = value;
						offset_highs.push_back(offset_high);
					}
					else
					{
						if (FSEEK64(pfile, value, SEEK_SET) == 0)
						{
							unsigned int vtemp;
							for (j = 0; j < (int)length; j++)
							{
								if (fread(&vtemp, sizeof(unsigned int), 1, pfile) == 1)
								{
									offsets.push_back(vtemp);
									if (vtemp < prev_offset)
									{
										m_l4gb = true;
										offset_high++;
									}
									prev_offset = vtemp;
									offset_highs.push_back(offset_high);
								}
							}
						}
					}
				}
				break;
			case 0x0115://277, samples per pixel
				if (full_image && cnt_image == 1)
					m_chan_num = value << 16 >> 16;
				break;
			case 0x0117://279, strip byte counts
				if (full_image)
				{
					if (length == 1)
					{
						sizes.push_back(value);
					}
					else
					{
						if (FSEEK64(pfile, value, SEEK_SET) == 0)
						{
							unsigned int vtemp;
							for (j = 0; j < (int)length; j++)
							{
								if (fread(&vtemp, sizeof(unsigned int), 1, pfile) == 1)
									sizes.push_back(vtemp);
							}
						}
					}
				}
				break;
			case 0x011C://284, planar configuration
				break;
			case 0x013D://317, predictor
				if (full_image && cnt_image == 1)
					m_predictor = value << 16 >> 16;
				break;
			case 0x0140://320, colormap
				break;
			case 0x866C://34412, zeiss lsm info
				if (type == 1)
				{
					if (FSEEK64(pfile, value, SEEK_SET) != 0)
					{
						fclose(pfile);
						return READER_FORMAT_ERROR;
					}
					unsigned char* pdata = new unsigned char[length];
					if (fread(pdata, sizeof(unsigned char), length, pfile) != length)
					{
						fclose(pfile);
						return READER_FORMAT_ERROR;
					}
					//read lsm info
					ReadLsmInfo(pfile, pdata, length);
					//delete
					delete[]pdata;
				}
				break;
			}

			//reset position
			if (FSEEK64(pfile, cur_pos, SEEK_SET) != 0)
			{
				fclose(pfile);
				return READER_FORMAT_ERROR;
			}
		}

		//build lsm info, which contains all offset values and sizes
		if (full_image)
		{
			int time = (cnt_image - 1) / m_slice_num;
			if (time + 1 > (int)m_lsm_info.size())
			{
				DatasetInfo dinfo;
				for (i = 0; i < m_chan_num; i++)
				{
					ChannelInfo cinfo;
					dinfo.push_back(cinfo);
				}
				m_lsm_info.push_back(dinfo);
			}
			//int slice = (cnt_image-1) % m_slice_num;
			for (i = 0; i < m_chan_num; i++)
			{
				SliceInfo sinfo;
				sinfo.offset = offsets[i];
				sinfo.offset_high = offset_highs[i];
				sinfo.size = sizes[i];
				//add slice info to lsm info
				m_lsm_info[time][i].push_back(sinfo);
			}
		}

		//next image
		if (fread(&ioffset, sizeof(unsigned int), 1, pfile) < 1)
		{
			fclose(pfile);
			return READER_FORMAT_ERROR;
		}
		if (!ioffset)
			break;
	}

	fclose(pfile);

	m_cur_time = 0;
	m_data_name = GET_STEM(m_path_name);

	return READER_OK;
}

void LSMReader::ReadLsmInfo(FILE* pfile, unsigned char* pdata, unsigned int size)
{
	m_excitation_wavelength_list.clear();

	int value;

	if (!pdata)
		return;
	unsigned int offset = 0;
	if (offset < size)
		m_version = *((unsigned int*)pdata);
	offset += 8;  //dimension x
	if (offset < size) value = *((int*)(pdata + offset));
	offset += 4;  //dimension y
	if (offset < size) value = *((int*)(pdata + offset));
	offset += 4;  //dimension z
	if (offset < size) m_slice_num = *((int*)(pdata + offset));
	offset += 4;  //channels
	if (offset < size) value = *((int*)(pdata + offset));
	offset += 4;  //time
	if (offset < size) m_time_num = *((int*)(pdata + offset));
	offset += 4;  //data type
	if (offset < size)
	{
		m_datatype = *((int*)(pdata + offset));
		switch (m_datatype)
		{
		case 0:  //multiple types
			break;
		case 1:  //8-bit unsigned integer
			m_max_value = 255.0;
			m_scalar_scale = 1.0;
			break;
		case 2:  //12-bit unsigned integer
			m_max_value = 4095.0;
			m_scalar_scale = 65535.0 / m_max_value;
			break;
		default://hopefully not
			m_max_value = 255.0;
			m_scalar_scale = 1.0;
			break;
		}
	}
	offset += 12;
	double spcx = 0.0, spcy = 0.0, spcz = 0.0;
	if (offset < size) spcx = *((double*)(pdata + offset));
	offset += 8;
	if (offset < size) spcy = *((double*)(pdata + offset));
	offset += 8;
	if (offset < size) spcz = *((double*)(pdata + offset));
	if (spcx > 0.0 && spcx<1e-4 &&
		spcy>0.0 && spcy<1e-4 &&
		spcz>0.0 && spcz < 1e-4)
	{
		m_valid_spc = true;
		m_xspc = spcx * 1e6;
		m_yspc = spcy * 1e6;
		m_zspc = spcz * 1e6;
		if (m_zspc < 1e-3) m_zspc = m_xspc;
	}
	else
	{
		m_valid_spc = false;
		m_xspc = 1.0;
		m_yspc = 1.0;
		m_zspc = 1.0;
	}
	//read data type if m_datatype is 0
	if (m_datatype == 0)
	{
		offset += 64;
		unsigned int type_offset = 0;
		if (offset < size) type_offset = *((unsigned int*)(pdata + offset));
		long cur_pos = ftell(pfile);
		if (FSEEK64(pfile, type_offset, SEEK_SET) == 0)
		{
			unsigned int data_type;
			if (fread(&data_type, sizeof(unsigned int), 1, pfile) == 1)
			{
				m_datatype = data_type;
				switch (data_type)
				{
				case 1:  //8-bit unsigned integer
					m_max_value = 255.0;
					m_scalar_scale = 1.0;
					break;
				case 2:  //12-bit unsigned integer
					m_max_value = 4095.0;
					m_scalar_scale = 65535.0 / m_max_value;
					break;
				case 3:  //16-bit unsigned integer
					m_max_value = 65535.0;
					m_scalar_scale = 1.0;
					break;
				default://hopefully not
					m_max_value = 255.0;
					m_scalar_scale = 1.0;
					break;
				}
			}
		}
		FSEEK64(pfile, cur_pos, SEEK_SET);
		offset -= 64;
	}
	//read wavelength
	offset += 68;
	unsigned int wl_offset = 0;
	if (offset < size) wl_offset = *((unsigned int*)(pdata + offset));
	if (wl_offset)
	{
		if (FSEEK64(pfile, wl_offset, SEEK_SET) == 0)
		{
			unsigned int uentry, utype, usize;

			WavelengthInfo info;
			int sub_cnt = 0;
			int track_cnt = 0;
			int ill_chan_cnt = 0;
			int last_sub = 0;//1:tracks; 2:track; 3:ill_chans; 4:ill_chan;
			std::vector<unsigned int> entry_list;
			bool sub_tracks = false, sub_track = false, sub_ill_chans = false, sub_ill_chan = false;

			while (fread(&uentry, sizeof(unsigned int), 1, pfile) == 1)
			{
				if (uentry == 0x010000000 ||  //SUBBLOCK_RECORDING
					uentry == 0x030000000 ||  //SUBBLOCK_LASERS
					uentry == 0x050000000 ||  //SUBBLOCK_LASER
					uentry == 0x020000000 ||  //SUBBLOCK_TRACKS
					uentry == 0x040000000 ||  //SUBBLOCK_TRACK
					uentry == 0x060000000 ||  //SUBBLOCK_DETECTION_CHANNELS
					uentry == 0x070000000 ||  //SUBBLOCK_DETECTION_CHANNEL
					uentry == 0x080000000 ||  //SUBBLOCK_ILLUMINATION_CHANNELS
					uentry == 0x090000000 ||  //SUBBLOCK_ILLUMINATION_CHANNEL
					uentry == 0x0A0000000 ||  //SUBBLOCK_BEAM_SPLITTERS
					uentry == 0x0B0000000 ||  //SUBBLOCK_BEAM_SPLITTER
					uentry == 0x0C0000000 ||  //SUBBLOCK_DATA_CHANNELS
					uentry == 0x0D0000000 ||  //SUBBLOCK_DATA_CHANNEL
					uentry == 0x011000000 ||  //SUBBLOCK_TIMERS
					uentry == 0x012000000 ||  //SUBBLOCK_TIMER
					uentry == 0x013000000 ||  //SUBBLOCK_MARKERS
					uentry == 0x014000000)    //SUBBLOCK_MARKER
				{
					entry_list.push_back(uentry);

					if (fread(&utype, sizeof(unsigned int), 1, pfile) != 1)
						break;
					if (fread(&usize, sizeof(unsigned int), 1, pfile) != 1)
						break;
					if (FSEEK64(pfile, usize, SEEK_CUR) != 0)
						break;

					sub_cnt++;
					if (uentry == 0x020000000)  //SUBBLOCK_TRACKS
					{
						last_sub = 1;
						sub_tracks = true;
					}
					else if (uentry == 0x040000000)//SUBBLOCK_TRACK
					{
						if (last_sub == 1)
							last_sub = 2;
						sub_track = true;
						track_cnt++;
						ill_chan_cnt = 0;
					}
					else if (uentry == 0x080000000)//SUBBLOCK_ILLUMINATION_CHANNELS
					{
						if (last_sub == 2)
							last_sub = 3;
						sub_ill_chans = true;
					}
					else if (uentry == 0x090000000)//SUBBLOCK_ILLUMINATION_CHANNEL
					{
						if (last_sub == 3)
							last_sub = 4;
						sub_ill_chan = true;
						ill_chan_cnt++;
					}
				}
				else if (uentry == 0x0FFFFFFFF)  //SUBBLOCK_END
				{
					if (fread(&utype, sizeof(unsigned int), 1, pfile) != 1)
						break;
					if (fread(&usize, sizeof(unsigned int), 1, pfile) != 1)
						break;
					if (FSEEK64(pfile, usize, SEEK_CUR) != 0)
						break;

					sub_cnt--;
					if (entry_list.empty())
						break;
					unsigned int last_entry = entry_list[entry_list.size() - 1];
					entry_list.pop_back();
					if (last_entry == 0x090000000)
					{
						sub_ill_chan = false;
						last_sub = 3;
					}
					else if (last_entry == 0x080000000)
					{
						sub_ill_chans = false;
						last_sub = 2;
					}
					else if (last_entry == 0x040000000)
					{
						sub_track = false;
						last_sub = 1;
					}
					else if (last_entry == 0x020000000)
					{
						sub_tracks = false;
						last_sub = 0;
					}
					if (sub_cnt == 0)
						break;
				}
				else if (uentry == 0x090000003)
				{
					double wavelength;
					if (fread(&utype, sizeof(unsigned int), 1, pfile) != 1)
						break;
					if (fread(&usize, sizeof(unsigned int), 1, pfile) != 1)
						break;
					if (fread(&wavelength, sizeof(double), 1, pfile) != 1)
						break;
					if (last_sub == 4 &&
						sub_tracks &&
						sub_track &&
						sub_ill_chans &&
						sub_ill_chan)
					{
						info.wavelength = wavelength;
					}
				}
				else if (uentry == 0x090000004)
				{
					int aquire;
					if (fread(&utype, sizeof(unsigned int), 1, pfile) != 1)
						break;
					if (fread(&usize, sizeof(unsigned int), 1, pfile) != 1)
						break;
					if (fread(&aquire, sizeof(int), 1, pfile) != 1)
						break;
					if (last_sub == 4 &&
						sub_tracks &&
						sub_track &&
						sub_ill_chans &&
						sub_ill_chan &&
						aquire)
					{
						m_excitation_wavelength_list.push_back(info);
						if ((int)m_excitation_wavelength_list.size() == m_chan_num)
							break;
					}
				}
				else if (uentry == 0x040000002)
				{
					int order;
					if (fread(&utype, sizeof(unsigned int), 1, pfile) != 1)
						break;
					if (fread(&usize, sizeof(unsigned int), 1, pfile) != 1)
						break;
					if (fread(&order, sizeof(int), 1, pfile) != 1)
						break;
					if (last_sub == 2 &&
						sub_tracks &&
						sub_track)
					{
						info.chan_num = order - 1;
					}
				}
				else
				{
					if (fread(&utype, sizeof(unsigned int), 1, pfile) != 1)
						break;
					if (fread(&usize, sizeof(unsigned int), 1, pfile) != 1)
						break;
					if (FSEEK64(pfile, usize, SEEK_CUR) != 0)
						break;
				}
			}
		}
	}
}

void LSMReader::SetBatch(bool batch)
{
	if (batch)
	{
		//read the directory info
		FIND_FILES_BATCH(m_path_name, ESCAPE_REGEX(L".lsm"), m_batch_list, m_cur_batch);
		m_batch = true;
	}
	else
		m_batch = false;
}

int LSMReader::LoadBatch(int index)
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

double LSMReader::GetExcitationWavelength(int chan)
{
	for (int i = 0; i < (int)m_excitation_wavelength_list.size(); i++)
	{
		if (m_excitation_wavelength_list[i].chan_num == chan)
			return m_excitation_wavelength_list[i].wavelength;
	}
	return 0.0;
}

Nrrd* LSMReader::Convert(int t, int c, bool get_max)
{
	Nrrd *data = 0;
	FILE* pfile = 0;
	if (!WFOPEN(&pfile, m_path_name, L"rb"))
		return 0;

	if (t >= 0 && t < m_time_num &&
		c >= 0 && c < m_chan_num &&
		m_slice_num > 0 &&
		m_x_size > 0 &&
		m_y_size > 0 &&
		t < (int)m_lsm_info.size() &&
		c < (int)m_lsm_info[t].size())
	{
		//allocate memory for nrrd
		bool show_progress = false;
		ChannelInfo *cinfo = &m_lsm_info[t][c];
		size_t blk_num = cinfo->size();
		unsigned long long mem_size = (unsigned long long)m_x_size *
			(unsigned long long)m_y_size * (unsigned long long)m_slice_num;
		void* val = 0;
		switch (m_datatype)
		{
		case 1://8-bit
			val = new (std::nothrow) unsigned char[mem_size];
			show_progress = mem_size > glbin_settings.m_prg_size;
			break;
		case 2://16-bit
		case 3:
			val = new (std::nothrow) unsigned short[mem_size];
			show_progress = mem_size * 2 > glbin_settings.m_prg_size;
			break;
		}

		for (size_t i = 0; i < blk_num; i++)
		{
			if (m_l4gb ?
				FSEEK64(pfile, ((uint64_t((*cinfo)[i].offset_high)) << 32) + (*cinfo)[i].offset, SEEK_SET) == 0 :
				FSEEK64(pfile, (*cinfo)[i].offset, SEEK_SET) == 0)
			{
				size_t val_pos = 0;
				switch (m_datatype)
				{
				case 1:
					val_pos = m_x_size * m_y_size * i;
					break;
				case 2:
				case 3:
					val_pos = m_x_size * m_y_size * i * 2;
					break;
				}

				if (m_compression == 1)
				{
					fread(tidata_t(val) + val_pos, sizeof(uint8_t), (*cinfo)[i].size, pfile);
				}
				else if (m_compression == 5)
				{
					unsigned char* tif = new (std::nothrow) unsigned char[(*cinfo)[i].size];
					fread(tif, sizeof(unsigned char), (*cinfo)[i].size, pfile);
					LZWDecode(tif, tidata_t(val) + val_pos, (*cinfo)[i].size);
					for (size_t j = 0; j < m_y_size; j++)
					{
						switch (m_datatype)
						{
						case 1:
							DecodeAcc8(tidata_t(val) + val_pos + j * m_x_size, m_x_size, 1);
							break;
						case 2:
						case 3:
							DecodeAcc16(tidata_t(val) + val_pos + j * m_x_size * 2, m_x_size, 1);
							break;
						}
					}
					delete[]tif;
				}

				if (show_progress && m_time_num == 1)
					SetProgress(static_cast<int>(std::round(100.0 * (i + 1) / blk_num)), "NOT_SET");
			}
		}
		//create nrrd
		data = nrrdNew();
		switch (m_datatype)
		{
		case 1:
			nrrdWrap_va(data, val, nrrdTypeUChar, 3, (size_t)m_x_size, (size_t)m_y_size, (size_t)m_slice_num);
			break;
		case 2:
		case 3:
			nrrdWrap_va(data, val, nrrdTypeUShort, 3, (size_t)m_x_size, (size_t)m_y_size, (size_t)m_slice_num);
			break;
		}
		nrrdAxisInfoSet_va(data, nrrdAxisInfoSpacing, m_xspc, m_yspc, m_zspc);
		nrrdAxisInfoSet_va(data, nrrdAxisInfoMax, m_xspc*m_x_size, m_yspc*m_y_size, m_zspc*m_slice_num);
		nrrdAxisInfoSet_va(data, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
		nrrdAxisInfoSet_va(data, nrrdAxisInfoSize, (size_t)m_x_size, (size_t)m_y_size, (size_t)m_slice_num);
	}

	fclose(pfile);
	m_cur_time = t;
	return data;
}

std::wstring LSMReader::GetCurDataName(int t, int c)
{
	return m_path_name;
}

std::wstring LSMReader::GetCurMaskName(int t, int c)
{
	std::wostringstream woss;
	woss << m_path_name.substr(0, m_path_name.find_last_of(L'.'));
	if (m_time_num > 1) woss << L"_T" << t;
	if (m_chan_num > 1) woss << L"_C" << c;
	woss << L".msk";
	std::wstring mask_name = woss.str();
	return mask_name;
}

std::wstring LSMReader::GetCurLabelName(int t, int c)
{
	std::wostringstream woss;
	woss << m_path_name.substr(0, m_path_name.find_last_of(L'.'));
	if (m_time_num > 1) woss << L"_T" << t;
	if (m_chan_num > 1) woss << L"_C" << c;
	woss << L".lbl";
	std::wstring label_name = woss.str();
	return label_name;
}
