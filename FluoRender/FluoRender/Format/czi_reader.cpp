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
#include <czi_reader.h>
#include <Global.h>
#include <MainSettings.h>
#include <compatibility.h>
#include <XmlUtils.h>
#include <set>
#include <Debug.h>

std::vector<std::string> CZIReader::m_types{
	"ZISRAWFILE",
	"ZISRAWDIRECTORY",
	"ZISRAWSUBBLOCK",
	"ZISRAWMETADATA",
	"ZISRAWATTACH",
	"ZISRAWATTDIR",
	"DELETED" };

CZIReader::CZIReader() :
	BaseVolReader(),
	m_header_read(false),
	m_multi_file(false),
	m_file_part(0),
	m_dir_pos(0),
	m_meta_pos(0),
	m_att_dir(0)
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

	//m_compression = 0;
	//m_predictor = 0;
	//m_version = 0;
	//m_datatype = 0;
	//m_l4gb = false;
}

CZIReader::~CZIReader()
{
}

//void CZIReader::SetFile(const std::string& file)
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

void CZIReader::SetFile(const std::wstring& file)
{
	m_path_name = file;
	m_id_string = m_path_name;
}

int CZIReader::Preprocess()
{
	FILE* pfile = 0;
	if (!WFOPEN(&pfile, m_path_name, L"rb"))
		return READER_OPEN_FAIL;

	unsigned long long ioffset = 0;
	//read header
	while (!feof(pfile))
		if (ReadSegment(pfile, ioffset, SegFile))
			break;
	if (m_header_read)
	{
		if (m_meta_pos)
			ReadSegment(pfile, m_meta_pos, SegMetadata);
		if (m_dir_pos)
			ReadSegment(pfile, m_dir_pos, SegDirectory);
	}

	//if 2d
	if (m_czi_info.zmin == std::numeric_limits<int>::max())
		m_czi_info.zmin = 0;
	if (m_czi_info.zmax == std::numeric_limits<int>::min())
		m_czi_info.zmax = 1;

	fclose(pfile);

	m_cur_time = 0;
	m_data_name = GET_STEM(m_path_name);

	return READER_OK;
}

void CZIReader::SetBatch(bool batch)
{
	if (batch)
	{
		//read the directory info
		FIND_FILES_BATCH(m_path_name, ESCAPE_REGEX(L".czi"), m_batch_list, m_cur_batch);
		m_batch = true;
	}
	else
		m_batch = false;
}

int CZIReader::LoadBatch(int index)
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

double CZIReader::GetExcitationWavelength(int chan)
{
	for (int i = 0; i < (int)m_excitation_wavelength_list.size(); i++)
	{
		if (m_excitation_wavelength_list[i].chan_num == chan)
			return m_excitation_wavelength_list[i].wavelength;
	}
	return 0.0;
}

Nrrd* CZIReader::Convert(int t, int c, bool get_max)
{
	Nrrd* data = 0;
	FILE* pfile = 0;
	if (!WFOPEN(&pfile, m_path_name, L"rb"))
		return 0;

	if (t >= 0 && t < m_time_num &&
		c >= 0 && c < m_chan_num &&
		m_slice_num > 0 &&
		m_x_size > 0 &&
		m_y_size > 0 &&
		t < (int)m_czi_info.times.size() &&
		c < (int)m_czi_info.times[t].channels.size())
	{
		ChannelInfo* cinfo = GetChaninfo(t, c);
		if (!cinfo)
		{
			fclose(pfile);
			return 0;
		}
		//allocate memory for nrrd
		bool show_progress = false;
		size_t blk_num = cinfo->blocks.size();
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
			val = new (std::nothrow) unsigned short[mem_size];
			show_progress = mem_size * 2 > glbin_settings.m_prg_size;
			break;
		}

		for (size_t i = 0; i < blk_num; i++)
		{
			SubBlockInfo* sbi = &(cinfo->blocks[i]);
			ReadSegSubBlock(pfile, sbi, val);
			if (show_progress && m_time_num == 1)
				SetProgress(static_cast<int>(std::round(100.0 * (i + 1) / blk_num)), "NOT_SET");
		}
		//create nrrd
		data = nrrdNew();
		switch (m_datatype)
		{
		case 1:
			nrrdWrap_va(data, val, nrrdTypeUChar, 3, (size_t)m_x_size, (size_t)m_y_size, (size_t)m_slice_num);
			break;
		case 2:
			nrrdWrap_va(data, val, nrrdTypeUShort, 3, (size_t)m_x_size, (size_t)m_y_size, (size_t)m_slice_num);
			break;
		}
		nrrdAxisInfoSet_va(data, nrrdAxisInfoSpacing, m_xspc, m_yspc, m_zspc);
		nrrdAxisInfoSet_va(data, nrrdAxisInfoMax, m_xspc * m_x_size, m_yspc * m_y_size, m_zspc * m_slice_num);
		nrrdAxisInfoSet_va(data, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
		nrrdAxisInfoSet_va(data, nrrdAxisInfoSize, (size_t)m_x_size, (size_t)m_y_size, (size_t)m_slice_num);
	}

	m_scalar_scale = 65535.0 / m_max_value;

	fclose(pfile);
	m_cur_time = t;
	return data;
}

std::wstring CZIReader::GetCurDataName(int t, int c)
{
	return m_path_name;
}

std::wstring CZIReader::GetCurMaskName(int t, int c)
{
	std::wostringstream woss;
	woss << m_path_name.substr(0, m_path_name.find_last_of(L'.'));
	if (m_time_num > 1) woss << L"_T" << t;
	if (m_chan_num > 1) woss << L"_C" << c;
	woss << L".msk";
	std::wstring mask_name = woss.str();
	return mask_name;
}

std::wstring CZIReader::GetCurLabelName(int t, int c)
{
	std::wostringstream woss;
	woss << m_path_name.substr(0, m_path_name.find_last_of(L'.'));
	if (m_time_num > 1) woss << L"_T" << t;
	if (m_chan_num > 1) woss << L"_C" << c;
	woss << L".lbl";
	std::wstring label_name = woss.str();
	return label_name;
}

bool CZIReader::ReadSegment(FILE* pfile, unsigned long long& ioffset, SegType readtype)
{
	if (FSEEK64(pfile, ioffset, SEEK_SET) != 0)
		return false;

	char id[16];
	unsigned long long alloc_size;
	unsigned long long used_size;
	bool result = true;
	result &= fread(id, 1, 16, pfile) == 16;
	result &= fread(&alloc_size, sizeof(unsigned long long), 1, pfile) == 1;
	result &= fread(&used_size, sizeof(unsigned long long), 1, pfile) == 1;
	if (!result)
		return false;

	std::string strid(id);
	auto it = std::find(m_types.begin(), m_types.end(), id);
	if (it == m_types.end())
	{
		ioffset += HDRSIZE + alloc_size;
		return false;
	}
	SegType type = (SegType)(std::distance(m_types.begin(), it));
	if (readtype != SegAll && type != readtype)
	{
		ioffset += HDRSIZE + alloc_size;
		return false;
	}
	result = false;
	switch (type)
	{
	case SegFile:
		result = ReadFile(pfile);
		break;
	case SegDirectory:
		result = ReadDirectory(pfile, ioffset + HDRSIZE);
		break;
	case SegSubBlock:
		result = ReadSubBlock(pfile, ioffset + HDRSIZE);
		break;
	case SegMetadata:
		result = ReadMetadata(pfile, ioffset + HDRSIZE);
		break;
	case SegAttach:
		result = ReadAttach(pfile);
		break;
	case SegAttDir:
		result = ReadAttDir(pfile);
		break;
	case SegDeleted:
		result = ReadDeleted(pfile);
		break;
	}

	ioffset += HDRSIZE + alloc_size;
	return result;
}

bool CZIReader::ReadFile(FILE* pfile)
{
	bool result = true;
	int ival;
	result &= fread(&ival, sizeof(int), 1, pfile) == 1;//major
	result &= fread(&ival, sizeof(int), 1, pfile) == 1;//minor
	result &= fread(&ival, sizeof(int), 1, pfile) == 1;//reserved
	result &= fread(&ival, sizeof(int), 1, pfile) == 1;//reserved
	unsigned int guid[4];
	result &= fread(guid, sizeof(unsigned int), 4, pfile) == 4;//master guid
	result &= fread(guid, sizeof(unsigned int), 4, pfile) == 4;//unique guid
	result &= fread(&m_file_part, sizeof(int), 1, pfile) == 1;//part number
	result &= fread(&m_dir_pos, sizeof(unsigned long long), 1, pfile) == 1;//directory position
	result &= fread(&m_meta_pos, sizeof(unsigned long long), 1, pfile) == 1;//metadata position
	result &= fread(&ival, sizeof(int), 1, pfile) == 1;//update pending
	result &= fread(&m_att_dir, sizeof(unsigned long long), 1, pfile) == 1;//attachment directory position
	if (m_file_part == 0)
	{
		m_multi_file = false;
	}
	else
	{
		m_multi_file = true;
	}
	m_header_read = result;
	return result;
}

unsigned int CZIReader::ReadDirectoryEntry(FILE* pfile)
{
	SubBlockInfo sbi;
	bool result = true;
	char schema_type[2];
	result &= fread(schema_type, sizeof(char), 2, pfile) == 2;//schema type
	unsigned int pixel_type;
	result &= fread(&pixel_type, sizeof(unsigned int), 1, pfile) == 1;//pixel type
	unsigned long long file_pos;
	result &= fread(&file_pos, sizeof(unsigned long long), 1, pfile) == 1;//file position
	unsigned int file_part;
	result &= fread(&file_part, sizeof(unsigned int), 1, pfile) == 1;//file part
	unsigned int compress;
	result &= fread(&compress, sizeof(unsigned int), 1, pfile) == 1;//compression
	char pyra_type;
	result &= fread(&pyra_type, 1, 1, pfile) == 1;//pyramid type
	char cval[5];
	result &= fread(cval, 1, 5, pfile) == 5;//reserved
	//set info
	sbi.pxtype = pixel_type;
	sbi.loc = file_pos;
	sbi.compress = compress;

	unsigned int dim_count;
	result &= fread(&dim_count, sizeof(unsigned int), 1, pfile) == 1;//dimension count
	for (size_t i = 0; i < dim_count; ++i)
	{
		char dim[4];
		result &= fread(dim, 1, 4, pfile) == 4;//dimension
		int start;
		result &= fread(&start, sizeof(int), 1, pfile) == 1;//start position
		unsigned int size;
		result &= fread(&size, sizeof(unsigned int), 1, pfile) == 1;//size in units of pixels
		float start_coord;
		result &= fread(&start_coord, sizeof(float), 1, pfile) == 1;//start coordinate
		unsigned int store_size;
		result &= fread(&store_size, sizeof(unsigned int), 1, pfile) == 1;//stored size

		//set info
		switch (dim[0])
		{
		case 'X':
			sbi.x = start;
			sbi.x_size = size;
			sbi.x_start = start_coord;
			m_czi_info.xsize(start, start + size);
			break;
		case 'Y':
			sbi.y = start;
			sbi.y_size = size;
			sbi.y_start = start_coord;
			m_czi_info.ysize(start, start + size);
			break;
		case 'Z':
			sbi.z = start;
			sbi.z_size = size;
			sbi.z_start = start_coord;
			m_czi_info.zsize(start, start + size);
			break;
		case 'C':
			sbi.chan = start;
			break;
		case 'T':
			sbi.time = start;
			break;
		}
	}

	unsigned int dirpos = 32 + dim_count * 20;
	sbi.dirpos = dirpos;

	//skip pyramid levels for now
	if (pyra_type > 0)
		return dirpos;
	//add info to list
	TimeInfo* timeinfo = GetTimeinfo(sbi.time);
	if (timeinfo)
	{
		ChannelInfo* chaninfo = GetChaninfo(timeinfo, sbi.chan);
		if (chaninfo)
		{
			chaninfo->blocks.push_back(sbi);
		}
		else
		{
			timeinfo->channels.push_back(ChannelInfo());
			chaninfo = &(timeinfo->channels.back());
			chaninfo->chan = sbi.chan;
			chaninfo->blocks.push_back(sbi);
		}
	}
	else
	{
		m_czi_info.times.push_back(TimeInfo());
		timeinfo = &(m_czi_info.times.back());
		timeinfo->time = sbi.time;
		ChannelInfo chaninfo;
		chaninfo.chan = sbi.chan;
		chaninfo.blocks.push_back(sbi);
		timeinfo->channels.push_back(chaninfo);
	}
	if (result)
		return dirpos;
	else
		return 0;
}

bool CZIReader::ReadDirectory(FILE* pfile, unsigned long long ioffset)
{
	bool result = true;
	int entry_count;
	result &= fread(&entry_count, sizeof(int), 1, pfile) == 1;//number of entries
	ioffset += 128;
	if (FSEEK64(pfile, ioffset, SEEK_SET) != 0)
		return false;

	//init info
	m_czi_info.init();
	m_time_num = 0;
	m_chan_num = 0;
	m_slice_num = 0;
	m_x_size = 0;
	m_y_size = 0;

	//read entries
	for (int i = 0; i < entry_count; ++i)
		result &= ReadDirectoryEntry(pfile) > 0;

	//get info
	std::set<unsigned int> pixtypes;
	std::set<int> channums;
	std::set<int> timenums;
	for (size_t i = 0; i < m_czi_info.times.size(); ++i)
		for (size_t j = 0; j < m_czi_info.times[i].channels.size(); ++j)
			for (size_t k = 0; k < m_czi_info.times[i].channels[j].blocks.size(); ++k)
			{
				channums.insert(m_czi_info.times[i].channels[j].blocks[k].chan);
				timenums.insert(m_czi_info.times[i].channels[j].blocks[k].time);
				pixtypes.insert(m_czi_info.times[i].channels[j].blocks[k].pxtype);
			}
	m_time_num = static_cast<int>(timenums.size());
	m_chan_num = static_cast<int>(channums.size());
	m_slice_num = m_czi_info.zmax - m_czi_info.zmin;
	m_x_size = m_czi_info.xmax - m_czi_info.xmin;
	m_y_size = m_czi_info.ymax - m_czi_info.ymin;
	if (!pixtypes.empty())
	{
		m_datatype = *--pixtypes.end();
		switch (m_datatype)
		{
		case 0:
			m_datatype = 1;
			break;
		case 1:
			m_datatype = 2;
			break;
		default:
			result = false;
		}
	}

	return result;
}

bool CZIReader::ReadSubBlock(FILE* pfile, unsigned long long ioffset)
{
	bool result = true;
	unsigned int meta_size;
	result &= fread(&meta_size, sizeof(unsigned int), 1, pfile) == 1;//metadata size
	unsigned int att_size;
	result &= fread(&att_size, sizeof(unsigned int), 1, pfile) == 1;//attachment size
	unsigned long long data_size;
	result &= fread(&data_size, sizeof(unsigned long long), 1, pfile) == 1;//data size

	//directory entry
	unsigned int dir_pos = ReadDirectoryEntry(pfile);
	dir_pos += 16;
	if (dir_pos > FIXSIZE)
		ioffset += dir_pos;
	else
		ioffset += FIXSIZE;
	if (FSEEK64(pfile, ioffset, SEEK_SET) != 0)
		return false;
	//read metadata
	std::string xmlstr(meta_size + 1, 0);
	result &= fread(&xmlstr[0], 1, meta_size, pfile) == meta_size;//xml info
	//data
	//attachment

	return result;
}

bool CZIReader::ReadMetadata(FILE* pfile, unsigned long long ioffset)
{
	bool result = true;
	unsigned int xmlsize;
	result &= fread(&xmlsize, sizeof(unsigned int), 1, pfile) == 1;//xml data size
	ioffset += FIXSIZE;
	if (FSEEK64(pfile, ioffset, SEEK_SET) != 0)
		return false;
	std::string xmlstr(xmlsize + 1, 0);
	result &= fread(&xmlstr[0], 1, xmlsize, pfile) == xmlsize;//xml info
	if (!result)
		return result;

	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError eResult = doc.Parse(xmlstr.c_str());
	if (eResult != tinyxml2::XML_SUCCESS)
		return false;
	tinyxml2::XMLElement* root = doc.RootElement();
	if (!root || std::string(root->Name()) != "ImageDocument")
		return false;

	//get values
	FindNodeRecursive(root);

	if (m_xspc > 0.0 &&
		m_yspc > 0.0 &&
		m_zspc > 0.0)
	{
		m_valid_spc = true;
	}
	else
	{
		m_valid_spc = false;
		m_xspc = 1.0;
		m_yspc = 1.0;
		m_zspc = 1.0;
	}
	return result;
}

bool CZIReader::ReadAttach(FILE* pfile)
{
	return true;
}

bool CZIReader::ReadAttDir(FILE* pfile)
{
	return true;
}

bool CZIReader::ReadDeleted(FILE* pfile)
{
	return true;
}

bool CZIReader::ReadSegSubBlock(FILE* pfile, SubBlockInfo* sbi, void* val)
{
	unsigned long long ioffset = sbi->loc;
	if (FSEEK64(pfile, ioffset, SEEK_SET) != 0)
		return false;

	char id[16];
	unsigned long long alloc_size;
	unsigned long long used_size;
	bool result = true;
	result &= fread(id, 1, 16, pfile) == 16;
	result &= fread(&alloc_size, sizeof(unsigned long long), 1, pfile) == 1;
	result &= fread(&used_size, sizeof(unsigned long long), 1, pfile) == 1;
	if (!result)
		return false;
	std::string strid(id);
	auto it = std::find(m_types.begin(), m_types.end(), id);
	if (it == m_types.end())
		return false;
	SegType type = (SegType)(std::distance(m_types.begin(), it));
	if (type != SegSubBlock)
		return false;

	ioffset += HDRSIZE;
	unsigned int meta_size;
	result &= fread(&meta_size, sizeof(unsigned int), 1, pfile) == 1;//metadata size
	unsigned int att_size;
	result &= fread(&att_size, sizeof(unsigned int), 1, pfile) == 1;//attachment size
	unsigned long long data_size;
	result &= fread(&data_size, sizeof(unsigned long long), 1, pfile) == 1;//data size
	if (!result)
		return false;

	//directory entry
	unsigned int dir_pos = sbi->dirpos;
	dir_pos += 16;
	if (dir_pos > FIXSIZE)
		ioffset += dir_pos;
	else
		ioffset += FIXSIZE;
	if (FSEEK64(pfile, ioffset, SEEK_SET) != 0)
		return false;

	//read metadata
	std::string xmlstr(meta_size + 1, 0);
	result &= fread(&xmlstr[0], 1, meta_size, pfile) == meta_size;//xml info

	//data
	bool bricks = true;
	if (sbi->x_size == m_x_size &&
		sbi->y_size == m_y_size)
		bricks = false;
	bool compress = sbi->compress == 2;//only supports lzw for now
	unsigned long long pxcount = (unsigned long long)sbi->x_size *
		sbi->y_size * sbi->z_size;
	unsigned short minv = std::numeric_limits<unsigned short>::max();
	unsigned short maxv = 0;
	unsigned long long xysize = (unsigned long long)m_x_size * m_y_size;
	unsigned long long pos = 0;
	if (bricks)
		pos = xysize * sbi->z +
		(unsigned long long)m_x_size * sbi->y +
		(unsigned long long)sbi->x;//consider it a brick
	else
		pos = xysize * sbi->z;

	if (bricks || compress)
	{
		unsigned char* block = new (std::nothrow) unsigned char[data_size];
		result &= fread(block, 1, data_size, pfile) == data_size;
		switch (m_datatype)
		{
		case 1:
		{
			if (compress)
			{
				LZWDecode(block, (unsigned char*)val + pos, static_cast<tsize_t>(data_size));
				for (int i = 0; i < sbi->z_size; ++i)
					for (int j = 0; j < sbi->y_size; ++j)
						DecodeAcc8((unsigned char*)val + pos + xysize * i + m_x_size * j,
							sbi->x_size, 1);
			}
			else
			{
				for (int i = 0; i < sbi->z_size; ++i)
					for (int j = 0; j < sbi->y_size; ++j)
						memcpy((unsigned char*)val + pos + xysize * i + m_x_size * j,
							block + i * sbi->x_size * sbi->y_size + j * sbi->x_size, sbi->x_size);
			}
		}
		break;
		case 2:
		{
			if (compress)
			{
				LZWDecode(block, (unsigned char*)((unsigned short*)val + pos), static_cast<tsize_t>(data_size));
				for (int i = 0; i < sbi->z_size; ++i)
					for (int j = 0; j < sbi->y_size; ++j)
						DecodeAcc16((unsigned char*)((unsigned short*)val + pos + xysize * i + m_x_size * j),
							sbi->x_size, 1);
			}
			else
			{
				for (int i = 0; i < sbi->z_size; ++i)
					for (int j = 0; j < sbi->y_size; ++j)
						memcpy((unsigned short*)val + pos + xysize * i + m_x_size * j,
							(unsigned short*)block + i * sbi->x_size * sbi->y_size + j * sbi->x_size,
							2 * sbi->x_size);
			}
			//get min max
			GetMinMax16B((unsigned short*)val + pos,
				sbi->x_size, sbi->y_size, sbi->z_size,
				m_x_size, m_y_size,
				minv, maxv);
			m_min_value = m_min_value == 0.0 ? minv : std::min(m_min_value, static_cast<double>(minv));
			m_max_value = maxv > m_max_value ? maxv : m_max_value;
		}
		break;
		}
		delete[] block;
	}
	else
	{
		switch (m_datatype)
		{
		case 1:
			result &= fread((unsigned char*)val + pos, 1, data_size, pfile) == data_size;
			break;
		case 2:
			result &= fread((unsigned short*)val + pos, 1, data_size, pfile) == data_size;
			if (result)
			{
				GetMinMax16((unsigned short*)val + pos, pxcount, minv, maxv);
				m_min_value = m_min_value == 0.0 ? minv : std::min(m_min_value, static_cast<double>(minv));
				m_max_value = maxv > m_max_value ? maxv : m_max_value;
			}
			break;
		}
	}

	return result;
}

void CZIReader::GetMinMax16(unsigned short* val, unsigned long long px,
	unsigned short& minv, unsigned short& maxv)
{
	for (unsigned long long i = 0; i < px; ++i)
	{
		minv = std::min(val[i], minv);
		maxv = std::max(val[i], maxv);
	}
}

void CZIReader::GetMinMax16B(unsigned short* val, int nx, int ny, int nz, int sx, int sy,
	unsigned short& minv, unsigned short& maxv)
{
	unsigned long long pos;
	unsigned short* pv;
	for (int i = 0; i < nz; ++i)
		for (int j = 0; j < ny; ++j)
			for (int k = 0; k < nx; ++k)
			{
				pos = (unsigned long long)sx * sy * i + sx * j + k;
				pv = val + pos;
				minv = std::min(*pv, minv);
				maxv = std::max(*pv, maxv);
			}
}

void CZIReader::FindNodeRecursive(tinyxml2::XMLElement* node)
{
	if (!node)
		return;
	std::string str;
	double dval;
	tinyxml2::XMLElement* child = node->FirstChildElement();
	while (child)
	{
		std::string name(child->Name());
		if (name == "ScalingX")
		{
			str = child->GetText();
			dval = STOD(str);
			m_xspc = dval * 1e6;
		}
		else if (name == "ScalingY")
		{
			str = child->GetText();
			dval = STOD(str);
			m_yspc = dval * 1e6;
		}
		else if (name == "ScalingZ")
		{
			str = child->GetText();
			dval = STOD(str);
				m_zspc = dval * 1e6;
		}
		else if (name == "ExcitationWavelength")
		{
			str = child->GetText();
			dval = STOD(str);
			WavelengthInfo winfo;
			winfo.chan_num = static_cast<int>(m_excitation_wavelength_list.size());
			winfo.wavelength = dval;
			m_excitation_wavelength_list.push_back(winfo);
		}
		else
		{
			FindNodeRecursive(child);
		}
		child = child->NextSiblingElement();
	}
	return;
}
