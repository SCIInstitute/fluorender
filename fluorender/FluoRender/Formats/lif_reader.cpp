/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2018 Scientific Computing and Imaging Institute,
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
#include "lif_reader.h"
#include "../compatibility.h"
#include <wx/sstream.h>
//#include <fstream>
#include <stdio.h>

LIFReader::LIFReader()
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

	m_version = -1;
}

LIFReader::~LIFReader()
{
}

void LIFReader::SetFile(string &file)
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

void LIFReader::SetFile(wstring &file)
{
	m_path_name = file;
	m_id_string = m_path_name;
}

int LIFReader::Preprocess()
{
	FILE* pfile = 0;
	if (!WFOPEN(&pfile, m_path_name.c_str(), L"rb"))
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
	m_data_name = GET_NAME(m_path_name);
	LoadBatch(0);

	return READER_OK;
}

void LIFReader::SetSliceSeq(bool ss)
{
	//do nothing
}

bool LIFReader::GetSliceSeq()
{
	return false;
}

void LIFReader::SetChannSeq(bool cs)
{
	//do nothing
}

bool LIFReader::GetChannSeq()
{
	return false;
}

void LIFReader::SetDigitOrder(int order)
{
	//do nothing
}

int LIFReader::GetDigitOrder()
{
	return 0;
}

void LIFReader::SetTimeId(wstring &id)
{
	//do nothing
}

wstring LIFReader::GetTimeId()
{
	return wstring(L"");
}

void LIFReader::SetBatch(bool batch)
{
}

int LIFReader::LoadBatch(int index)
{
	int result = -1;
	if (index >= 0 && index < (int)m_batch_list.size())
	{
		std::wstring name = m_batch_list[index];
		//Preprocess();
		ImageInfo* imgi = FindImageInfo(name);
		if (imgi)
		{
			m_chan_num = imgi->channels.size();
			ChannelInfo* cinfo = imgi->GetChannelInfo(0);
			if (cinfo)
			{
				m_time_num = cinfo->times.size();
				if (cinfo->res == 8)
					m_datatype = 1;
				else if (cinfo->res > 8)
					m_datatype = 2;
			}
			TimeInfo* tinfo = imgi->GetTimeInfo(0, 0);
			if (tinfo && tinfo->blocks.size() > 0)
			{
				m_slice_num = tinfo->blocks[0].z_size;
				m_x_size = tinfo->blocks[0].x_size;
				m_y_size = tinfo->blocks[0].y_size;
			}
		}

		result = index;
		m_cur_batch = result;
	}
	else
		result = -1;

	return result;
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
	if (!WFOPEN(&pfile, m_path_name.c_str(), L"rb"))
		return 0;

/*	if (t >= 0 && t < m_time_num &&
		c >= 0 && c < m_chan_num &&
		m_slice_num > 0 &&
		m_x_size > 0 &&
		m_y_size > 0)
	{
		//allocate memory for nrrd
		switch (m_datatype)
		{
		case 1://8-bit
		{
			unsigned long long mem_size = (unsigned long long)m_x_size*
				(unsigned long long)m_y_size*(unsigned long long)m_slice_num;
			unsigned char *val = new (std::nothrow) unsigned char[mem_size];
			ChannelInfo *cinfo = GetChaninfo(t, c);
			for (int i = 0; i < (int)cinfo->blocks.size(); i++)
			{
				SubBlockInfo* sbi = &(cinfo->blocks[i]);
				ReadMemoryBlock(pfile, sbi, val);
			}
			//create nrrd
			data = nrrdNew();
			nrrdWrap(data, val, nrrdTypeUChar, 3, (size_t)m_x_size, (size_t)m_y_size, (size_t)m_slice_num);
			nrrdAxisInfoSet(data, nrrdAxisInfoSpacing, m_xspc, m_yspc, m_zspc);
			nrrdAxisInfoSet(data, nrrdAxisInfoMax, m_xspc*m_x_size, m_yspc*m_y_size, m_zspc*m_slice_num);
			nrrdAxisInfoSet(data, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
			nrrdAxisInfoSet(data, nrrdAxisInfoSize, (size_t)m_x_size, (size_t)m_y_size, (size_t)m_slice_num);
		}
		break;
		case 2://16-bit
		{
			unsigned long long mem_size = (unsigned long long)m_x_size*
				(unsigned long long)m_y_size*(unsigned long long)m_slice_num;
			unsigned short *val = new (std::nothrow) unsigned short[mem_size];
			ChannelInfo *cinfo = GetChaninfo(t, c);
			for (int i = 0; i < (int)cinfo->blocks.size(); i++)
			{
				SubBlockInfo* sbi = &(cinfo->blocks[i]);
				ReadMemoryBlock(pfile, sbi, val);
			}
			//create nrrd
			data = nrrdNew();
			nrrdWrap(data, val, nrrdTypeUShort, 3, (size_t)m_x_size, (size_t)m_y_size, (size_t)m_slice_num);
			nrrdAxisInfoSet(data, nrrdAxisInfoSpacing, m_xspc, m_yspc, m_zspc);
			nrrdAxisInfoSet(data, nrrdAxisInfoMax, m_xspc*m_x_size, m_yspc*m_y_size, m_zspc*m_slice_num);
			nrrdAxisInfoSet(data, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
			nrrdAxisInfoSet(data, nrrdAxisInfoSize, (size_t)m_x_size, (size_t)m_y_size, (size_t)m_slice_num);
		}
		break;
		}
	}
*/
	fclose(pfile);
	m_cur_time = t;
	return data;
}

wstring LIFReader::GetCurDataName(int t, int c)
{
	return m_path_name;
}

wstring LIFReader::GetCurMaskName(int t, int c)
{
	wostringstream woss;
	woss << m_path_name.substr(0, m_path_name.find_last_of('.'));
	if (m_time_num > 1) woss << "_T" << t;
	if (m_chan_num > 1) woss << "_C" << c;
	woss << ".msk";
	wstring mask_name = woss.str();
	return mask_name;
}

wstring LIFReader::GetCurLabelName(int t, int c)
{
	wostringstream woss;
	woss << m_path_name.substr(0, m_path_name.find_last_of('.'));
	if (m_time_num > 1) woss << "_T" << t;
	if (m_chan_num > 1) woss << "_C" << c;
	woss << ".lbl";
	wstring label_name = woss.str();
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
	std::wstring xmlstr(xmlsize, 0);
	result &= fread(&xmlstr[0], sizeof(wchar_t), xmlsize, pfile) == xmlsize;
	if (!result)
		return 0;

	//test xml
	//std::ofstream outfile;
	//std::string outname = GET_PATH(m_path_name) + "metadata.xml";
	//outfile.open(outname, std::ofstream::out);
	//outfile << xmlstr;
	//outfile.close();
	//endof test

	wxXmlDocument doc;
	wxStringInputStream wxss(xmlstr);
	result &= doc.Load(wxss);
	wxXmlNode *root = doc.GetRoot();
	if (!root || root->GetName() != "LMSDataContainerHeader")
		return 0;
	wxString wstr = root->GetAttribute("Version");
	unsigned long ival;
	if (wstr.ToULong(&ival))
		m_version = ival;
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
	std::wstring namestr(nsize, 0);
	result &= fread(&namestr[0], sizeof(wchar_t), nsize, pfile) == nsize;
	if (!result)
		return 0;
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
	if (FSEEK64(pfile, ioffset, SEEK_SET) != 0)
		return false;
	bool result = true;
	//result &= fread((unsigned char*)val, 1, sbi->size, pfile) == sbi->size;
	return result;
}

void LIFReader::ReadElement(wxXmlNode* node)
{
	if (!node)
		return;
	wxString str;
	wxXmlNode *child = node->GetChildren();
	while (child)
	{
		str = child->GetName();
		if (str == "Element")
		{
			str = child->GetAttribute("Visibility");
			if (str != "0")
			{
				std::wstring name = child->GetAttribute("Name");
				ReadData(child, name);
			}
		}
		ReadElement(child);
		child = child->GetNext();
	}
}

void LIFReader::ReadData(wxXmlNode* node, std::wstring &name)
{
	if (!node)
		return;
	wxString str;
	wxXmlNode *child = node->GetChildren();
	ImageInfo* imgi = 0;
	unsigned long long sbsize = 0;
	std::wstring sbname;
	while (child)
	{
		str = child->GetName();
		if (str == "Data")
			imgi = ReadImage(child, name);
		else if (str == "Memory")
		{
			str = child->GetAttribute("Size");
			str.ToULongLong(&sbsize);
			sbname = child->GetAttribute("MemoryBlockID");
		}
		child = child->GetNext();
	}
	if (imgi && sbsize && sbname != "")
	{
		imgi->mbid = sbname;
	}
}

LIFReader::ImageInfo* LIFReader::ReadImage(wxXmlNode* node, std::wstring &name)
{
	if (!node)
		return 0;
	wxString str;
	wxXmlNode* child = node->GetChildren();
	if (!child || child->GetName() != "Image")
		return 0;
	ImageInfo imgi;
	imgi.name = name;
	ReadSubBlockInfo(child, imgi);
	auto result = m_lif_info.images.insert(std::pair<std::wstring,
		LIFReader::ImageInfo>(imgi.name, imgi));
	if (result.second)
	{
		m_batch_list.push_back(imgi.name);
		if (m_lif_info.images.size() > 1)
			m_batch = true;
		return &(result.first->second);
	}
	else
		return 0;
}

void LIFReader::ReadSubBlockInfo(wxXmlNode* node, LIFReader::ImageInfo &imgi)
{
	if (!node)
		return;
	wxString str;
	unsigned long ulv;
	double dval;
	unsigned long long ull;
	wxXmlNode *child = node->GetChildren();
	while (child)
	{
		str = child->GetName();
		if (str == "ChannelDescription")
		{
			ChannelInfo cinfo;
			cinfo.chan = imgi.channels.size();
			str = child->GetAttribute("Resolution");
			if (str.ToULong(&ulv))
				cinfo.res = ulv;
			str = child->GetAttribute("Min");
			if (str.ToDouble(&dval))
				cinfo.minv = dval;
			str = child->GetAttribute("Max");
			if (str.ToDouble(&dval))
				cinfo.maxv = dval;
			str = child->GetAttribute("BytesInc");
			if (str.ToULongLong(&ull))
				cinfo.inc = ull;
			imgi.channels.push_back(cinfo);
		}
		else if (str == "DimensionDescription")
		{
			unsigned long did = 0, size = 0;
			double orig = 0, len = 0, sfactor = 1;
			unsigned long long inc = 0;
			str = child->GetAttribute("DimID");
			if (str.ToULong(&did))
			{
				str = child->GetAttribute("Unit");
				if (str == "m")
					sfactor = 1e6;
				else if (str == "mm")
					sfactor = 1e3;
				str = child->GetAttribute("NumberOfElements");
				if (str.ToULong(&ulv))
					size = ulv;
				str = child->GetAttribute("Origin");
				if (str.ToDouble(&dval))
					orig = dval * sfactor;
				str = child->GetAttribute("Length");
				if (str.ToDouble(&dval))
					len = dval * sfactor;
				str = child->GetAttribute("BytesInc");
				if (str.ToULongLong(&ull))
					inc = ull;
				AddSubBlockInfo(imgi, did, size, orig, len, inc);
			}
		}
		ReadSubBlockInfo(child, imgi);
		child = child->GetNext();
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
	else
		cinfo->times.resize(1);
	TimeInfo* tinfo = imgi.GetTimeInfo(chan, time);
	if (!tinfo)
		return;
	SubBlockInfo *sbi = 0;
	if (tinfo->blocks.empty())
		tinfo->blocks.resize(1);
	sbi = &(tinfo->blocks[0]);
	sbi->chan = chan;
	sbi->time = time;
	switch (dim)
	{
	case 1:
		sbi->x_size = size;
		sbi->x_start = orig;
		sbi->x_len = len;
		sbi->x_inc = inc;
		break;
	case 2:
		sbi->y_size = size;
		sbi->y_start = orig;
		sbi->y_len = len;
		sbi->y_inc = inc;
		break;
	case 3:
		sbi->z_size = size;
		sbi->z_start = orig;
		sbi->z_len = len;
		sbi->z_inc = inc;
		break;
	case 4:
		tinfo->inc = inc;
		break;
	}
}

void LIFReader::FillLifInfo()
{
	//if (sbi.x_size && sbi.y_size && !sbi.z_size)
	//	sbi.z_size = 1;
	for (auto it = m_lif_info.images.begin();
		it != m_lif_info.images.end(); ++it)
		it->second.FillInfo();
}