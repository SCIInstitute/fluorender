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
#include <lof_reader.h>
#include <Global.h>
#include <MainSettings.h>
#include <compatibility.h>
#include <XmlUtils.h>
#include <algorithm>

LOFReader::LOFReader():
	BaseVolReader()
{
	m_time_num = 0;
	m_cur_time = -1;
	m_chan_num = 0;

	m_valid_spc = false;
	m_spacing = fluo::Vector(1.0);

	m_min_value = 0.0;
	m_max_value = 0.0;
	m_scalar_scale = 1.0;

	m_fp_convert = false;
	m_fp_min = 0;
	m_fp_max = 1;

	m_batch = false;
	m_cur_batch = -1;

	m_ver_major = -1;
	m_ver_minor = -1;
}

LOFReader::~LOFReader()
{
}

//void LOFReader::SetFile(const std::string &file)
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

void LOFReader::SetFile(const std::wstring &file)
{
	m_path_name = file;
	m_id_string = m_path_name;
}

int LOFReader::Preprocess()
{
	FILE* pfile = 0;
	if (!WFOPEN(&pfile, m_path_name, L"rb"))
		return READER_OPEN_FAIL;

	unsigned long long ioffset = 0;
	bool result = true;

	//header
	unsigned int tv0;
	result &= fread(&tv0, sizeof(unsigned int), 1, pfile) == 1;
	if (!result || tv0 != LOFTEST0)
		return READER_FORMAT_ERROR;
	unsigned int uisize;
	result &= fread(&uisize, sizeof(unsigned int), 1, pfile) == 1;
	if (!result || !uisize)
		return READER_FORMAT_ERROR;
	//type content
	unsigned char tv1;
	result &= fread(&tv1, 1, 1, pfile) == 1;
	if (!result || tv1 != LOFTEXT1)
		return READER_FORMAT_ERROR;
	unsigned int namesize;
	result &= fread(&namesize, sizeof(unsigned int), 1, pfile) == 1;
	if (!result || !namesize)
		return READER_FORMAT_ERROR;
	std::string type_name(namesize*2 + 2, 0);
	result &= fread(&type_name[0], 1, namesize*2, pfile) == namesize*2;
	if (!result)
		return READER_FORMAT_ERROR;
	//major version
	result &= fread(&tv1, 1, 1, pfile) == 1;
	if (!result || tv1 != LOFTEXT1)
		return READER_FORMAT_ERROR;
	result &= fread(&m_ver_major, sizeof(int), 1, pfile) == 1;
	if (!result)
		return READER_FORMAT_ERROR;
	//minor version
	result &= fread(&tv1, 1, 1, pfile) == 1;
	if (!result || tv1 != LOFTEXT1)
		return READER_FORMAT_ERROR;
	result &= fread(&m_ver_minor, sizeof(int), 1, pfile) == 1;
	if (!result)
		return READER_FORMAT_ERROR;
	//memory size
	result &= fread(&tv1, 1, 1, pfile) == 1;
	if (!result || tv1 != LOFTEXT1)
		return READER_FORMAT_ERROR;
	unsigned long long ullsize;
	result &= fread(&ullsize, sizeof(unsigned long long), 1, pfile) == 1;
	if (!result)
		return READER_FORMAT_ERROR;

	//metadata
	m_mem_loc = LOFHSIZE + uisize;
	m_mem_size = ullsize;
	ioffset = m_mem_loc  + ullsize;
	ReadMetadata(pfile, ioffset);
	FillLofInfo();

	fclose(pfile);

	return READER_OK;
}

void LOFReader::SetBatch(bool batch)
{
	if (batch)
	{
		//read the directory info
		FIND_FILES_BATCH(m_path_name, ESCAPE_REGEX(L".lof"), m_batch_list, m_cur_batch);
		m_batch = true;
	}
	else
		m_batch = false;
}

int LOFReader::LoadBatch(int index)
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

double LOFReader::GetExcitationWavelength(int chan)
{
	for (int i = 0; i < (int)m_excitation_wavelength_list.size(); i++)
	{
		if (m_excitation_wavelength_list[i].chan_num == chan)
			return m_excitation_wavelength_list[i].wavelength;
	}
	return 0.0;
}

Nrrd* LOFReader::Convert(int t, int c, bool get_max)
{
	Nrrd *data = 0;
	FILE* pfile = 0;
	if (!WFOPEN(&pfile, m_path_name, L"rb"))
		return 0;

	if (t >= 0 && t < m_time_num &&
		c >= 0 && c < m_chan_num &&
		!m_size.any_le_zero())
	{
		TimeInfo *tinfo = m_lof_info.GetTimeInfo(c, t);
		if (!tinfo)
		{
			return 0;
		}
		//allocate memory for nrrd
		bool show_progress = false;
		size_t blk_num = tinfo->blocks.size();
		unsigned long long mem_size = m_size.get_size_xyz();
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
			SubBlockInfo* sbi = &(tinfo->blocks[i]);
			ReadMemoryBlock(pfile, sbi, val);
			if (show_progress && m_time_num == 1)
				SetProgress(static_cast<int>(std::round(100.0 * (i + 1) / blk_num)), "NOT_SET");
		}
		//create nrrd
		data = nrrdNew();
		switch (m_datatype)
		{
		case 1:
			nrrdWrap_va(data, val, nrrdTypeUChar, 3, (size_t)m_size.intx(), (size_t)m_size.inty(), (size_t)m_size.intz());
			break;
		case 2:
			nrrdWrap_va(data, val, nrrdTypeUShort, 3, (size_t)m_size.intx(), (size_t)m_size.inty(), (size_t)m_size.intz());
			break;
		}
		nrrdAxisInfoSet_va(data, nrrdAxisInfoSpacing, m_spacing.x(), m_spacing.y(), m_spacing.z());
		auto max_size = m_spacing * m_size;
		nrrdAxisInfoSet_va(data, nrrdAxisInfoMax, max_size.x(), max_size.y(), max_size.z());
		nrrdAxisInfoSet_va(data, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
		nrrdAxisInfoSet_va(data, nrrdAxisInfoSize, (size_t)m_size.intx(), (size_t)m_size.inty(), (size_t)m_size.intz());
	}

	fclose(pfile);
	m_cur_time = t;
	return data;
}

std::wstring LOFReader::GetCurDataName(int t, int c)
{
	return m_path_name;
}

std::wstring LOFReader::GetCurMaskName(int t, int c)
{
	std::wostringstream woss;
	woss << m_path_name.substr(0, m_path_name.find_last_of(L'.'));
	if (m_time_num > 1) woss << L"_T" << t;
	if (m_chan_num > 1) woss << L"_C" << c;
	woss << L".msk";
	std::wstring mask_name = woss.str();
	return mask_name;
}

std::wstring LOFReader::GetCurLabelName(int t, int c)
{
	std::wostringstream woss;
	woss << m_path_name.substr(0, m_path_name.find_last_of(L'.'));
	if (m_time_num > 1) woss << L"_T" << t;
	if (m_chan_num > 1) woss << L"_C" << c;
	woss << L".lbl";
	std::wstring label_name = woss.str();
	return label_name;
}

unsigned long long LOFReader::ReadMetadata(FILE* pfile, unsigned long long ioffset)
{
	bool result = true;
	if (FSEEK64(pfile, ioffset, SEEK_SET) != 0)
		return 0;

	unsigned int tv0;
	result &= fread(&tv0, sizeof(unsigned int), 1, pfile) == 1;
	if (!result || tv0 != LOFTEST0)
		return 0;
	unsigned int uisize;
	result &= fread(&uisize, sizeof(unsigned int), 1, pfile) == 1;
	if (!result || !uisize)
		return 0;

	//read xml
	unsigned char tv1;
	result &= fread(&tv1, 1, 1, pfile) == 1;
	if (!result || tv1 != LOFTEXT1)
		return 0;
	unsigned int xmlsize;
	result &= fread(&xmlsize, sizeof(unsigned int), 1, pfile) == 1;
	if (!result || !xmlsize)
		return 0;
#ifdef _WIN32
	std::wstring xmlstr(xmlsize + 1, 0);
	result &= fread(&xmlstr[0], sizeof(wchar_t), xmlsize, pfile) == xmlsize;
	if (!result)
		return 0;
#else
	std::string temp(xmlsize * 2 + 2, 0);
	result &= fread(&temp[0], 1, xmlsize * 2, pfile) == xmlsize * 2;
	if (!result)
		return 0;
	std::wstring xmlstr = s2ws(temp);
#endif

	//test xml
	//std::ofstream outfile;
	//std::string outname = GET_PATH(m_path_name) + "metadata.xml";
	//outfile.open(outname, std::ofstream::out);
	//outfile << xmlstr;
	//outfile.close();
	//endof test

	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError eResult = doc.Parse(ws2s(xmlstr).c_str());
	if (eResult != tinyxml2::XML_SUCCESS)
		return 0;
	tinyxml2::XMLElement* root = doc.RootElement();
	ReadImage(root);

	return LOFHSIZE + uisize;
}

void LOFReader::ReadImage(tinyxml2::XMLElement* node)
{
	if (!node)
		return;
	std::string str;
	tinyxml2::XMLElement* child = node->FirstChildElement();
	if (!child || std::string(child->Name()) != "Image")
		return;
	ReadSubBlockInfo(child);
	for (size_t i = 0; i < m_lof_info.channels.size(); ++i)
	{
		str = m_lof_info.channels[i].lut;
		if (!str.empty())
		{
			WavelengthInfo winfo;
			winfo.chan_num = static_cast<int>(i);
			if (str == "Red")
				winfo.wavelength = 550.0;
			else if (str == "Green")
				winfo.wavelength = 450.0;
			else if (str == "Blue")
				winfo.wavelength = 400.0;
			else if (str == "Cyan")
				winfo.wavelength = 650.0;
			else
				winfo.wavelength = 800.0;
			m_excitation_wavelength_list.push_back(winfo);
		}
	}
}

void LOFReader::ReadSubBlockInfo(tinyxml2::XMLElement* node)
{
	if (!node)
		return;
	std::string str;
	tinyxml2::XMLElement *child = node->FirstChildElement();
	while (child)
	{
		str = child->Name();
		if (str == "ChannelDescription")
		{
			ChannelInfo cinfo;
			cinfo.chan = static_cast<int>(m_lof_info.channels.size());
			str = GetAttributeValue(child, "Resolution");
			cinfo.res = STOUL(str);
			str = GetAttributeValue(child, "Min");
			cinfo.minv = STOD(str);
			str = GetAttributeValue(child, "Max");
			cinfo.maxv = STOD(str);
			str = GetAttributeValue(child, "BytesInc");
			cinfo.inc = STOULL(str);
			cinfo.lut = GetAttributeValue(child, "LUTName");
			m_lof_info.channels.push_back(cinfo);
			m_lof_info.minv = std::min(m_lof_info.minv, cinfo.minv);
			m_lof_info.maxv = std::max(m_lof_info.maxv, cinfo.maxv);
		}
		else if (str == "DimensionDescription")
		{
			unsigned long did = 0, size = 0;
			double orig = 0, len = 0, sfactor = 1;
			unsigned long long inc = 0;
			str = GetAttributeValue(child, "DimID");
			bool flag = true;
			try
			{
				did = std::stoul(str);
			}
			catch (...)
			{
				flag = false;
			}
			if (flag)
			{
				str = GetAttributeValue(child, "Unit");
				if (str == "m")
					sfactor = 1e6;
				else if (str == "mm")
					sfactor = 1e3;
				str = GetAttributeValue(child, "NumberOfElements");
				size = STOUL(str);
				str = GetAttributeValue(child, "Origin");
				orig = STOD(str) * sfactor;
				str = GetAttributeValue(child, "Length");
				len = STOD(str) * sfactor;
				str = GetAttributeValue(child, "BytesInc");
				inc = STOULL(str);
				AddSubBlockInfo(did, size, orig, len, inc);
			}
		}
		ReadSubBlockInfo(child);
		child = child->NextSiblingElement();
	}
}

void LOFReader::AddSubBlockInfo(unsigned int dim, unsigned int size,
	double orig, double len, unsigned long long inc)
{
	int time = 0;
	int chan = 0;
	ChannelInfo* cinfo = m_lof_info.GetChannelInfo(chan);
	if (dim == 4 && size > 1)
		cinfo->times.resize(size);
	else if (cinfo->times.empty())
		cinfo->times.resize(1);
	TimeInfo* tinfo = m_lof_info.GetTimeInfo(chan, time);
	if (!tinfo)
		return;
	SubBlockInfo *sbi = 0;
	if (tinfo->blocks.empty())
		tinfo->blocks.resize(1);
	sbi = &(tinfo->blocks[0]);
	sbi->chan = chan;
	sbi->time = time;
	unsigned int empty_dim = GetEmptyDim(cinfo, sbi);
	tinfo->SetSubBlockInfo(dim, empty_dim,
		size, orig, len, inc);
}

void LOFReader::FillLofInfo()
{
	m_lof_info.loc = m_mem_loc;
	m_lof_info.size = m_mem_size;
	m_lof_info.FillInfo();

	m_cur_time = 0;
	m_data_name = GET_STEM(m_path_name);
	m_chan_num = static_cast<int>(m_lof_info.channels.size());
	ChannelInfo* cinfo = m_lof_info.GetChannelInfo(0);
	if (cinfo)
	{
		m_time_num = static_cast<int>(cinfo->times.size());
		if (cinfo->res == 8)
			m_datatype = 1;
		else if (cinfo->res > 8)
			m_datatype = 2;
	}
	TimeInfo* tinfo = m_lof_info.GetTimeInfo(0, 0);
	if (tinfo && tinfo->blocks.size() > 0)
	{
		//pixel size
		m_size = fluo::Vector(
			tinfo->blocks[0].x_size,
			tinfo->blocks[0].y_size,
			tinfo->blocks[0].z_size);
		//spacings
		if (m_size.x() > 0)
			m_spacing.x(tinfo->blocks[0].x_len / m_size.x());
		if (m_size.y() > 0)
			m_spacing.y(tinfo->blocks[0].y_len / m_size.y());
		if (m_size.z() > 0)
			m_spacing.z(tinfo->blocks[0].z_len / m_size.z());
		if (!m_spacing.any_le_zero())
		{
			m_valid_spc = true;
		}
		else
		{
			m_valid_spc = false;
			m_spacing = fluo::Vector(1.0);
		}
	}
	if (m_datatype > 1)
	{
		m_min_value = m_lof_info.minv;
		m_max_value = m_lof_info.maxv;
		m_scalar_scale = 65535.0 / m_max_value;
	}
	else
	{
		m_max_value = 255.0;
		m_scalar_scale = 1.0;
	}
}

bool LOFReader::ReadMemoryBlock(FILE* pfile, SubBlockInfo* sbi, void* val)
{
	unsigned long long ioffset = sbi->loc;
	bool result = true;
	unsigned long long size = (unsigned long long)sbi->x_size
		* sbi->y_size * sbi->z_size * m_datatype;
	unsigned long long slice_size = (unsigned long long)sbi->x_size
		* sbi->y_size * m_datatype;
	unsigned long long line_size = (unsigned long long)sbi->x_size * m_datatype;
	if (sbi->z_size == 1)
	{
		if (sbi->y_inc == line_size)
		{
			//read slice
			if (FSEEK64(pfile, ioffset, SEEK_SET) != 0)
				return false;
			result &= fread((unsigned char*)val, 1, size, pfile) == size;
		}
		else
		{
			//read line by line
			//read y dir first
			unsigned char* pos = (unsigned char*)val;
			for (int i = 0; i < sbi->y_size; ++i)
			{
				if (FSEEK64(pfile, ioffset, SEEK_SET) != 0)
					return false;
				result &= fread(pos, 1, line_size, pfile) == line_size;
				if (!result)
					return false;
				ioffset += sbi->y_inc;
				pos += line_size;
			}
		}
	}
	else
	{
		if (sbi->z_inc == slice_size)
		{
			//read volume
			if (FSEEK64(pfile, ioffset, SEEK_SET) != 0)
				return false;
			result &= fread((unsigned char*)val, 1, size, pfile) == size;
		}
		else
		{
			unsigned char* pos = (unsigned char*)val;
			if (sbi->y_inc == line_size)
			{
				//read slice by slice
				for (int i = 0; i < sbi->z_size; ++i)
				{
					if (FSEEK64(pfile, ioffset, SEEK_SET) != 0)
						return false;
					result &= fread(pos, 1, slice_size, pfile) == slice_size;
					if (!result)
						return false;
					ioffset += sbi->z_inc;
					pos += slice_size;
				}
			}
			else
			{
				if (sbi->y_inc < sbi->z_inc ||
					sbi->z_inc == 0)
				{
					//read line by line
					//read y dir first
					for (int i = 0; i < sbi->z_size; ++i)
					{
						unsigned long long iof2 = ioffset;
						unsigned char* pos2 = pos;
						for (int j = 0; j < sbi->y_size; ++j)
						{
							if (FSEEK64(pfile, iof2, SEEK_SET) != 0)
								return false;
							result &= fread(pos2, 1, line_size, pfile) == line_size;
							if (!result)
								return false;
							iof2 += sbi->y_inc;
							pos2 += line_size;
						}
						ioffset += sbi->z_inc;
						pos += slice_size;
					}
				}
				else
				{
					//read pixel by pixel
					//read z dir first
					for (int i = 0; i < sbi->y_size; ++i)
					{
						unsigned long long iof2 = ioffset;
						unsigned char* pos2 = pos;
						for (int j = 0; j < sbi->x_size; ++j)
						{
							if (FSEEK64(pfile, iof2, SEEK_SET) != 0)
								return false;
							unsigned char* pos3 = pos2;
							for (int k = 0; k < sbi->z_size; ++k)
							{
								result &= fread(pos3, 1, m_datatype, pfile) == m_datatype;
								if (!result)
									return false;
								pos3 += slice_size;
							}
							iof2 += sbi->z_inc;
							pos2 += m_datatype;
						}
						ioffset += sbi->y_inc;
						pos += line_size;
					}
				}
			}
		}
	}
	return result;
}

