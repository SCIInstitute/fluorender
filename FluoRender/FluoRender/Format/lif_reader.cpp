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
#include <lif_reader.h>
#include <Global.h>
#include <MainSettings.h>
#include <compatibility.h>
#include <XmlUtils.h>

LIFReader::LIFReader():
	BaseVolReader()
{
	m_tile_scan = false;
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

	m_version = -1;
}

LIFReader::~LIFReader()
{
}

//void LIFReader::SetFile(const std::string &file)
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

void LIFReader::SetFile(const std::wstring &file)
{
	m_path_name = file;
	m_id_string = m_path_name;
}

int LIFReader::Preprocess()
{
	FILE* pfile = 0;
	if (!WFOPEN(&pfile, m_path_name, L"rb"))
		return READER_OPEN_FAIL;

	unsigned long long ioffset = 0;
	unsigned long long ull;
	//read metadata
	ull = ReadMetadata(pfile, ioffset);
	if (!ull)
		return READER_OPEN_FAIL;
	ioffset += ull;
	//read memory blocks
	while (!feof(pfile))
	{
		unsigned long long ull = PreReadMemoryBlock(pfile, ioffset);
		if (!ull)
			break;
		ioffset += ull;
	}

	fclose(pfile);

	FillLifInfo();
	m_cur_time = 0;
	m_data_name = GET_STEM(m_path_name);
	LoadBatch(0);

	return READER_OK;
}

void LIFReader::SetBatch(bool batch)
{
}

int LIFReader::LoadBatch(int index)
{
	int result = -1;
	if (index >= 0 && index < (int)m_lif_info.images.size())
	{
		ImageInfo* imgi = &(m_lif_info.images[index]);
		if (imgi)
			GenImageInfo(imgi);

		result = index;
		m_cur_batch = result;
	}
	else
		result = -1;

	return result;
}

int LIFReader::LoadOffset(int offset)
{
	return LoadBatch(offset);
}

double LIFReader::GetExcitationWavelength(int chan)
{
	for (int i = 0; i < (int)m_excitation_wavelength_list.size(); i++)
	{
		if (m_excitation_wavelength_list[i].chan_num == chan)
			return m_excitation_wavelength_list[i].wavelength;
	}
	return 0.0;
}

Nrrd* LIFReader::Convert(int t, int c, bool get_max)
{
	Nrrd *data = 0;
	FILE* pfile = 0;
	if (!WFOPEN(&pfile, m_path_name, L"rb"))
		return 0;

	if (t >= 0 && t < m_time_num &&
		c >= 0 && c < m_chan_num &&
		!m_size.any_le_zero())
	{
		TimeInfo *tinfo = GetTimeInfo(c, t);
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

std::wstring LIFReader::GetCurDataName(int t, int c)
{
	return m_path_name;
}

std::wstring LIFReader::GetCurMaskName(int t, int c)
{
	std::wostringstream woss;
	woss << m_path_name.substr(0, m_path_name.find_last_of(L'.'));
	if (m_time_num > 1) woss << L"_T" << t;
	if (m_chan_num > 1) woss << L"_C" << c;
	woss << L".msk";
	std::wstring mask_name = woss.str();
	return mask_name;
}

std::wstring LIFReader::GetCurLabelName(int t, int c)
{
	std::wostringstream woss;
	woss << m_path_name.substr(0, m_path_name.find_last_of(L'.'));
	if (m_time_num > 1) woss << L"_T" << t;
	if (m_chan_num > 1) woss << L"_C" << c;
	woss << L".lbl";
	std::wstring label_name = woss.str();
	return label_name;
}

unsigned long long LIFReader::ReadMetadata(FILE* pfile, unsigned long long ioffset)
{
	bool result = true;
	if (FSEEK64(pfile, ioffset, SEEK_SET) != 0)
		return 0;

	unsigned int tv0;
	result &= fread(&tv0, sizeof(unsigned int), 1, pfile) == 1;
	if (!result || tv0 != LIFTEST0)
		return 0;
	unsigned int uisize;
	result &= fread(&uisize, sizeof(unsigned int), 1, pfile) == 1;
	if (!result || !uisize)
		return 0;

	//read xml
	unsigned char tv1;
	result &= fread(&tv1, 1, 1, pfile) == 1;
	if (!result || tv1 != LIFTEXT1)
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
	if (!root || std::string(root->Name()) != "LMSDataContainerHeader")
		return 0;
	if (HasAttribute(root, "Version"))
	{
		m_version = STOUL(GetAttributeValue(root, "Version"));
	}
	ReadElement(root);

	return LIFHSIZE + uisize;
}

unsigned long long LIFReader::PreReadMemoryBlock(FILE* pfile, unsigned long long ioffset)
{
	bool result = true;
	if (FSEEK64(pfile, ioffset, SEEK_SET) != 0)
		return 0;

	unsigned int tv0;
	result &= fread(&tv0, sizeof(unsigned int), 1, pfile) == 1;
	if (!result || tv0 != LIFTEST0)
		return 0;
	unsigned int uisize = 0;
	result &= fread(&uisize, sizeof(unsigned int), 1, pfile) == 1;
	if (!result || !uisize)
		return 0;

	//data block
	unsigned char tv1;
	result &= fread(&tv1, 1, 1, pfile) == 1;
	if (!result || tv1 != LIFTEXT1)
		return 0;
	unsigned long long memsize = 0;
	if (m_version == 1)
	{
		unsigned int uimemsize;
		result &= fread(&uimemsize, sizeof(unsigned int), 1, pfile) == 1;
		memsize = uimemsize;
	}
	else if (m_version == 2)
	{
		result &= fread(&memsize, sizeof(unsigned long long), 1, pfile) == 1;
	}
	else
		return 0;//only support ver 1 and 2
	if (!result)
		return 0;
	result &= fread(&tv1, 1, 1, pfile) == 1;
	if (!result || tv1 != LIFTEXT1)
		return 0;
	unsigned int nsize;
	result &= fread(&nsize, sizeof(unsigned int), 1, pfile) == 1;
#ifdef _WIN32
	std::wstring namestr(nsize, 0);
	result &= fread(&namestr[0], sizeof(wchar_t), nsize, pfile) == nsize;
	if (!result)
		return 0;
#else
	std::string temp(nsize * 2 + 2, 0);
	result &= fread(&temp[0], 1, nsize * 2, pfile) == nsize * 2;
	if (!result)
		return 0;
	std::wstring namestr = s2ws(temp);
#endif
	ImageInfo* imgi = FindImageInfoMbid(namestr);
	if (imgi)
	{
		imgi->loc = ioffset;
		imgi->loc += LIFHSIZE + uisize;
		imgi->size = memsize;
	}

	return LIFHSIZE + uisize + memsize;
}

bool LIFReader::ReadMemoryBlock(FILE* pfile, SubBlockInfo* sbi, void* val)
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
			result &= CopyMemoryBlock(pfile, sbi, val);
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
			//result &= fread((unsigned char*)val, 1, size, pfile) == size;
			result &= CopyMemoryBlock(pfile, sbi, val);
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

bool LIFReader::CopyMemoryBlock(FILE* pfile, SubBlockInfo* sbi, void* val)
{
	if (!m_tile_scan)
	{
		unsigned long long blck_size = (unsigned long long)sbi->x_size
			* sbi->y_size * sbi->z_size * m_datatype;
		return (fread((unsigned char*)val, 1, blck_size, pfile) == blck_size);
	}

	unsigned long long line_size = (unsigned long long)sbi->x_size * m_datatype;
	unsigned long long offset = ((unsigned long long)(sbi->z) * m_size.get_size_xy() +
		sbi->y * m_size.intx() + sbi->x) * m_datatype;
	unsigned char* pos = (unsigned char*)val + offset;
	unsigned char* pos2;
	bool result = true;
	for (int i = 0; i < sbi->z_size; ++i)
	{
		pos2 = pos;
		for (int j = 0; j < sbi->y_size; ++j)
		{
			result &= fread(pos2, 1, line_size, pfile) == line_size;
			pos2 += m_size.intx() * m_datatype;
		}
		pos += (unsigned long long)m_size.get_size_xy() * m_datatype;
	}
	return result;
}

void LIFReader::ReadElement(tinyxml2::XMLElement* node)
{
	if (!node)
		return;
	std::string str;
	tinyxml2::XMLElement *child = node->FirstChildElement();
	while (child)
	{
		str = child->Name();
		if (str == "Element")
		{
			str = GetAttributeValue(child, "Visibility");
			if (str != "0")
			{
				str = GetAttributeValue(child, "name");
				std::wstring name = s2ws(str);
				ReadData(child, name);
			}
		}
		ReadElement(child);
		child = child->NextSiblingElement();
	}
}

void LIFReader::ReadData(tinyxml2::XMLElement* node, std::wstring &name)
{
	if (!node)
		return;
	std::string str;
	tinyxml2::XMLElement *child = node->FirstChildElement();
	ImageInfo* imgi = 0;
	unsigned long long sbsize = 0;
	std::wstring sbname;
	while (child)
	{
		str = child->Name();
		if (str == "Data")
			imgi = ReadImage(child, name);
		else if (str == "Memory")
		{
			str = GetAttributeValue(child, "Size");
			sbsize = STOULL(str);
			if (HasAttribute(child, "MemoryBlockID"))
				sbname = s2ws(GetAttributeValue(child, "MemoryBlockID"));
		}
		child = child->NextSiblingElement();
	}
	if (imgi && sbsize && sbname != L"")
	{
		imgi->mbid = sbname;
	}
}

LIFReader::ImageInfo* LIFReader::ReadImage(tinyxml2::XMLElement* node, std::wstring &name)
{
	if (!node)
		return 0;
	std::string str;
	tinyxml2::XMLElement* child = node->FirstChildElement();
	if (!child || std::string(child->Name()) != "Image")
		return 0;
	ImageInfo imgi;
	imgi.name = name;
	ReadSubBlockInfo(child, imgi);
	for (size_t i = 0; i < imgi.channels.size(); ++i)
	{
		str = imgi.channels[i].lut;
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
	m_lif_info.images.push_back(imgi);
	if (m_lif_info.images.size() > 1)
		m_batch = true;
	return &(m_lif_info.images.back());
}

void LIFReader::ReadSubBlockInfo(tinyxml2::XMLElement* node, LIFReader::ImageInfo &imgi)
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
			cinfo.chan = static_cast<int>(imgi.channels.size());
			str = GetAttributeValue(child, "Resolution");
			cinfo.res = STOUL(str);
			str = GetAttributeValue(child, "Min");
			cinfo.minv = STOD(str);
			str = GetAttributeValue(child, "Max");
			cinfo.maxv = STOD(str);
			str = GetAttributeValue(child, "BytesInc");
			cinfo.inc = STOULL(str);
			cinfo.lut = GetAttributeValue(child, "LUTName");
			imgi.channels.push_back(cinfo);
			imgi.minv = std::min(imgi.minv, cinfo.minv);
			imgi.maxv = std::max(imgi.maxv, cinfo.maxv);
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
				AddSubBlockInfo(imgi, did, size, orig, len, inc);
			}
		}
		else if (str == "Attachment")
		{
			ReadTileScanInfo(child, imgi.tile_list);
		}

		ReadSubBlockInfo(child, imgi);
		child = child->NextSiblingElement();
	}
}

void LIFReader::AddSubBlockInfo(ImageInfo &imgi, unsigned int dim, unsigned int size,
	double orig, double len, unsigned long long inc)
{
	int time = 0;
	int chan = 0;
	ChannelInfo* cinfo = imgi.GetChannelInfo(chan);
	if (dim == 4 && size > 1)
		cinfo->times.resize(size);
	else if (cinfo->times.empty())
		cinfo->times.resize(1);
	TimeInfo* tinfo = imgi.GetTimeInfo(chan, time);
	if (!tinfo)
		return;
	SubBlockInfo *sbi = 0;
	if (dim == 10 && size > 1)
		tinfo->blocks.resize(size);
	else if (tinfo->blocks.empty())
		tinfo->blocks.resize(1);
	sbi = &(tinfo->blocks[0]);
	sbi->chan = chan;
	sbi->time = time;
	unsigned int empty_dim = GetEmptyDim(cinfo, sbi);
	tinfo->SetSubBlockInfo(dim, empty_dim,
		size, orig, len, inc);
}

bool LIFReader::ReadTileScanInfo(tinyxml2::XMLElement* node, TileList& list)
{
	if (!node)
		return false;
	std::string str;
	str = node->Name();
	if (str != "Attachment")
		return false;
	str = GetAttributeValue(node, "Name");
	if (str != "TileScanInfo")
		return false;
	list.clear();
	tinyxml2::XMLElement* child = node->FirstChildElement();
	while (child)
	{
		str = child->Name();
		if (str == "Tile")
		{
			TileScanInfo info;
			str = GetAttributeValue(child, "FieldX");
			info.fieldx = STOI(str);
			str = GetAttributeValue(child, "FieldY");
			info.fieldy = STOI(str);
			str = GetAttributeValue(child, "FieldZ");
			info.fieldz = STOI(str);
			str = GetAttributeValue(child, "PosX");
			info.posx = STOD(str);
			str = GetAttributeValue(child, "PosY");
			info.posy = STOD(str);
			str = GetAttributeValue(child, "PosZ");
			info.posz = STOD(str);
			list.push_back(info);
		}
		child = child->NextSiblingElement();
	}
	if (list.empty())
		return false;
	return true;
}

void LIFReader::GenImageInfo(ImageInfo* imgi)
{
	m_chan_num = static_cast<int>(imgi->channels.size());
	if (m_chan_num < 1)
		return;

	ChannelInfo* cinfo = imgi->GetChannelInfo(0);
	if (!cinfo)
		return;
	m_time_num = static_cast<int>(cinfo->times.size());
	if (m_time_num < 1)
		return;
	if (cinfo->res == 8)
		m_datatype = 1;
	else if (cinfo->res > 8)
		m_datatype = 2;

	TimeInfo* tinfo = imgi->GetTimeInfo(0, 0);
	if (!tinfo)
		return;
	int block_num = static_cast<int>(tinfo->blocks.size());
	if (block_num < 1)
		return;
	else if (block_num > 1 &&
		block_num == imgi->tile_list.size())
	{
		//tiled scan
		SubBlockInfo &block0 = tinfo->blocks[0];
		//spacings
		if (block0.x_size)
			m_spacing.x(block0.x_len / block0.x_size);
		if (block0.y_size)
			m_spacing.y(block0.y_len / block0.y_size);
		if (block0.z_size)
			m_spacing.z(block0.z_len / block0.z_size);

		//determine if seq or pos is used
		bool pos_valid = false;
		TileScanInfo &tile0 = imgi->tile_list[0];
		TileScanInfo &tile1 = imgi->tile_list[1];
		if (tile0.posx != tile1.posx ||
			tile0.posy != tile1.posy ||
			tile0.posz != tile1.posz)
			pos_valid = true;

		//find extent
		if (pos_valid)
		{
			double minx, maxx, miny, maxy, minz, maxz;
			for (size_t i = 0; i < block_num; ++i)
			{
				SubBlockInfo &block = tinfo->blocks[i];
				TileScanInfo &tile = imgi->tile_list[i];
				//convert from m to um
				block.x_start = tile.posx * 10e5;
				block.y_start = tile.posy * 10e5;
				block.z_start = tile.posz * 10e5;
				if (i == 0)
				{
					minx = block.x_start;
					maxx = minx + block.x_len;
					miny = block.y_start;
					maxy = miny + block.y_len;
					minz = block.z_start;
					maxz = minz + block.z_len;
				}
				else
				{
					minx = std::min(minx, block.x_start);
					maxx = std::max(maxx, block.x_start + block.x_len);
					miny = std::min(miny, block.y_start);
					maxy = std::max(maxy, block.y_start + block.y_len);
					minz = std::min(minz, block.z_start);
					maxz = std::max(maxz, block.z_start + block.z_len);
				}
			}
			m_size = fluo::Vector(
				maxx - minx,
				maxy - miny,
				maxz - minz);
			m_size /= m_spacing;

			//assign corner coords
			for (size_t i = 0; i < block_num; ++i)
			{
				SubBlockInfo &block = tinfo->blocks[i];
				block.x = int(std::round(block.x_start - minx) / m_spacing.x());
				block.y = int(std::round(block.y_start - miny) / m_spacing.y());
				block.z = int(std::round(block.z_start - minz) / m_spacing.z());
			}
		}
		else
		{
			int minx, maxx, miny, maxy, minz, maxz;
			for (size_t i = 0; i < block_num; ++i)
			{
				SubBlockInfo &block = tinfo->blocks[i];
				TileScanInfo &tile = imgi->tile_list[i];
				if (i == 0)
				{
					maxx = minx = tile.fieldx;
					maxy = miny = tile.fieldy;
					maxz = minz = tile.fieldz;
				}
				else
				{
					minx = std::min(minx, tile.fieldx);
					maxx = std::max(maxx, tile.fieldx);
					miny = std::min(miny, tile.fieldy);
					maxy = std::max(maxy, tile.fieldy);
					minz = std::min(minz, tile.fieldz);
					maxz = std::max(maxz, tile.fieldz);
				}
			}
			m_size = fluo::Vector(
				(maxx - minx + 1) * block0.x_size,
				(maxy - miny + 1) * block0.y_size,
				(maxz - minz + 1) * block0.z_size);

			//assign corner coords
			for (size_t i = 0; i < block_num; ++i)
			{
				SubBlockInfo &block = tinfo->blocks[i];
				TileScanInfo &tile = imgi->tile_list[i];
				block.x = (tile.fieldx - minx) * block0.x_size;
				block.y = (tile.fieldy - miny) * block0.y_size;
				block.z = (tile.fieldz - minz) * block0.z_size;
				block.x_start = block.x * m_spacing.x();
				block.y_start = block.y * m_spacing.y();
				block.z_start = block.z * m_spacing.z();
			}
		}

		m_tile_scan = true;
	}
	else
	{
		SubBlockInfo &block0 = tinfo->blocks[0];
		//pixel size
		m_size = fluo::Vector(
			block0.x_size,
			block0.y_size,
			block0.z_size);
		//spacings
		if (m_size.x() > 0)
			m_spacing.x(block0.x_len / m_size.x());
		if (m_size.y() > 0)
			m_spacing.y(block0.y_len / m_size.y());
		if (m_size.z() > 0)
			m_spacing.z(block0.z_len / m_size.z());
	}

	if (!m_spacing.any_le_zero())
	{
		m_valid_spc = true;
	}
	else
	{
		m_valid_spc = false;
		m_spacing = fluo::Vector(1.0);
	}

	if (m_datatype > 1)
	{
		m_min_value = imgi->minv;
		m_max_value = imgi->maxv;
		m_scalar_scale = 65535.0 / m_max_value;
	}
	else
	{
		m_max_value = 255.0;
		m_scalar_scale = 1.0;
	}
}

void LIFReader::FillLifInfo()
{
	//if (sbi.x_size && sbi.y_size && !sbi.z_size)
	//	sbi.z_size = 1;
	for (auto it = m_lif_info.images.begin();
		it != m_lif_info.images.end(); ++it)
		it->FillInfo();
}
