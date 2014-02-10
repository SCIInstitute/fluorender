#include <Formats\tif_reader.h>
#include <algorithm>

TIFReader::TIFReader()
{
	m_resize_type = 0;
	m_resample_type = 0;
	m_alignment = 0;

	m_slice_seq = false;
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

	//TIFFSetWarningHandler(NULL);
	//TIFFSetErrorHandler(NULL);
}

TIFReader::~TIFReader()
{
}

void TIFReader::SetFile(string &file)
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

void TIFReader::SetFile(wstring &file)
{
	m_path_name = file;
	m_id_string = m_path_name;
}

void TIFReader::Preprocess()
{
	int i, j;

	m_4d_seq.clear();

	//separate path and name
	size_t pos = m_path_name.find_last_of(L'\\');
	if (pos == -1)
		return;
	wstring path = m_path_name.substr(0, pos+1);
	wstring name = m_path_name.substr(pos+1);
	//extract time sequence string
	size_t begin = name.find(m_time_id);
	size_t end = -1;
	size_t id_len = m_time_id.size();
	if (begin != -1)
	{
		wstring t_num;
		size_t j;
		for (j=begin+id_len; j<name.size(); j++)
		{
			WCHAR c = name[j];
			if (iswdigit(c))
				t_num.push_back(c);
			else break;
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
		SliceInfo sliceinfo;
		sliceinfo.slice = name;	//temporary name
		sliceinfo.slicenumber = 0;
		info.slices.push_back(sliceinfo);
		info.filenumber = 0;
		m_4d_seq.push_back(info);
		m_cur_time = 0;
	}
	else
	{
		//search time sequence files
		wstring search_str = path + name.substr(0, begin+id_len) +
			L"*" + name.substr(end);
		WIN32_FIND_DATAW FindFileData;
		HANDLE hFind;
		hFind = FindFirstFileW(search_str.c_str(), &FindFileData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			TimeDataInfo info;
			wstring str = FindFileData.cFileName;
			wstring t_num;
			size_t j;
			for (j=begin+id_len; j<str.size(); j++)
			{
				WCHAR c = str[j];
				if (iswdigit(c))
					t_num.push_back(c);
				else break;
			}
			if (t_num.size() > 0)
				info.filenumber = _wtoi(t_num.c_str());
			else
				info.filenumber = 0;
			SliceInfo sliceinfo;
			sliceinfo.slice = str;
			sliceinfo.slicenumber = 0;
			info.slices.push_back(sliceinfo);
			m_4d_seq.push_back(info);

			while (FindNextFileW(hFind, &FindFileData) != 0)
			{
				TimeDataInfo info;
				str = FindFileData.cFileName;
				t_num.clear();
				for (j=begin+id_len; j<str.size(); j++)
				{
					WCHAR c = str[j];
					if (iswdigit(c))
						t_num.push_back(c);
					else break;
				}
				if (t_num.size() > 0)
					info.filenumber = _wtoi(t_num.c_str());
				else
					info.filenumber = 0;
				SliceInfo sliceinfo;
				sliceinfo.slice = str;
				sliceinfo.slicenumber = 0;
				info.slices.push_back(sliceinfo);
				m_4d_seq.push_back(info);
			}
		}
		FindClose(hFind);
	}

	if (m_4d_seq.size() > 0)
		std::sort(m_4d_seq.begin(), m_4d_seq.end(), TIFReader::tif_sort);

	//build 3d slice sequence
	for (int t=0; t<(int)m_4d_seq.size(); t++)
	{
		wstring slice_str = m_4d_seq[t].slices[0].slice;

		if (m_slice_seq)
		{
			//extract common string in name
			size_t pos2 = slice_str.find_last_of(L'.');
			size_t begin2 = 0;
			size_t end2 = -1;
			for (i=int(pos2)-1; i>=0; i--)
			{
				if (iswdigit(slice_str[i]) && end2==-1)
					end2 = i;
				if (!iswdigit(slice_str[i]) && end2!=-1)
				{
					begin2 = i;
					break;
				}
			}
			if (end2!=-1)
			{
				size_t sn_start = begin2+1;
				wstring search_str2 = path + slice_str.substr(0, begin2+1) + L'*' + slice_str.substr(end2+1);
				//search slice sequence
				WIN32_FIND_DATAW FindFileData;
				HANDLE hFind;
				hFind = FindFirstFileW(search_str2.c_str(), &FindFileData);
				if (hFind != INVALID_HANDLE_VALUE)
				{
					m_4d_seq[t].type = 1;
					wstring temp = FindFileData.cFileName;
					wstring slnum_s;
					for (j=(int)sn_start; j<(int)temp.size(); j++)
					{
						if (iswdigit(temp[j]))
							slnum_s.push_back(temp[j]);
						else
							break;
					}
					temp = path + FindFileData.cFileName;
					m_4d_seq[t].slices[0].slice = temp;
					m_4d_seq[t].slices[0].slicenumber = _wtoi(slnum_s.c_str());
					if (temp == m_path_name)
						m_cur_time = t;

					while (FindNextFileW(hFind, &FindFileData) != 0)
					{
						slnum_s.clear();
						temp = FindFileData.cFileName;
						for (j=(int)sn_start; j<(int)temp.size(); j++)
						{
							if (iswdigit(temp[j]))
								slnum_s.push_back(temp[j]);
							else
								break;
						}
						temp = path + FindFileData.cFileName;
						SliceInfo sliceinfo;
						sliceinfo.slice = temp;
						sliceinfo.slicenumber = _wtoi(slnum_s.c_str());
						m_4d_seq[t].slices.push_back(sliceinfo);
						if (temp == m_path_name)
							m_cur_time = t;
					}

					if (m_4d_seq[t].slices.size() > 0)
						std::sort(m_4d_seq[t].slices.begin(),
						m_4d_seq[t].slices.end(),
						TIFReader::tif_slice_sort);
				}
				FindClose(hFind);
			}
		}
		else
		{
			m_4d_seq[t].type = 0;
			m_4d_seq[t].slices[0].slice = path + slice_str;
			if (m_4d_seq[t].slices[0].slice == m_path_name)
				m_cur_time = t;
		}
	}

	//get time number and channel number
	m_time_num = (int)m_4d_seq.size();
	if (m_4d_seq.size()>0 &&
		m_cur_time>=0 &&
		m_cur_time<(int)m_4d_seq.size() &&
		m_4d_seq[m_cur_time].slices.size()>0)
	{
		wstring tiff_name = m_4d_seq[m_cur_time].slices[0].slice;
		if (tiff_name.size()>0)
		{
			m_chan_num = GetTiffField(tiff_name,kSamplesPerPixelTag);
		}
		else m_chan_num = 0;
	}
	else m_chan_num = 0;
}

uint32_t TIFReader::GetTiffField(std::wstring name, const int in_tag) 
{
	std::ifstream is(name, std::ifstream::binary);
	is.seekg(2,is.beg);
	uint16_t tiff_num=0;
	is.read((char*)&tiff_num,sizeof(uint16_t));
	bool swap =  SwapShort(tiff_num) == 42;
	if (tiff_num == 42 || swap) {
		//find the first IFD block/page
		is.seekg(4,is.beg);
		uint32_t ifd_offset=0;
		is.read((char*)&ifd_offset,sizeof(uint32_t));
		//go to the first IFD block/page
		if (swap) ifd_offset = SwapWord(ifd_offset);
		is.seekg(ifd_offset,is.beg);
		uint16_t num_entries=0;
		//how many entries are there?
		is.read((char*)&num_entries,sizeof(uint16_t));
		if (swap) num_entries = SwapShort(num_entries);
		for(size_t i = 0; i < num_entries; i++) {
			is.seekg(ifd_offset+2+12*i,is.beg);
			uint16_t tag=0;
			//get the tag of entry
			is.read((char*)&tag,sizeof(uint16_t));
			if (swap) tag = SwapShort(tag);
			if (tag == in_tag) {
				//find the type
				is.seekg(ifd_offset+2+12*i+2,is.beg);
				uint16_t type=0;
				is.read((char*)&type,sizeof(uint16_t));
				if (swap) type = SwapShort(type);
				// This is the correct tag, grab and go.
				is.seekg(ifd_offset+2+12*i+8,is.beg);
				uint32_t answer=0;
				if (type == kByte) {
					uint8_t value=0;
					is.read((char*)&value,sizeof(uint8_t));
					answer = static_cast<uint32_t>(value);
				} else if (type == kShort) {
					uint16_t value=0;
					is.read((char*)&value,sizeof(uint16_t));
					if (swap) value = SwapShort(value);
					answer = static_cast<uint32_t>(value);
				} else if (type == kLong) {
					uint32_t value=0;
					is.read((char*)&value,sizeof(uint32_t));
					if (swap) value = SwapWord(value);
					answer = value;
				} else if (type == kRational) {
					//TODO
					exit(0);
				} else { exit(0); }
				is.close();
				return answer;
			}
		}
	} 
	is.close();
	return 0;
}

uint16_t TIFReader::SwapShort(uint16_t num) {
	return ((num & 0x00FF) << 8) | ((num & 0xFF00) >> 8);
}

uint32_t TIFReader::SwapWord(uint32_t num) {
	return ((num & 0x000000FF) << 24) | ((num & 0xFF000000) >> 24) |
		   ((num & 0x0000FF00) << 8)  | ((num & 0x00FF0000) >> 8);
}

void TIFReader::SetSliceSeq(bool ss)
{
	//enable searching for slices
	m_slice_seq = ss;
}

bool TIFReader::GetSliceSeq()
{
	return m_slice_seq;
}

void TIFReader::SetTimeId(wstring &id)
{
	m_time_id = id;
}

wstring TIFReader::GetTimeId()
{
	return m_time_id;
}

void TIFReader::SetBatch(bool batch)
{
	if (batch)
	{
		//separate path and name
		size_t pos = m_path_name.find_last_of(L'\\');
		if (pos == -1)
			return;
		wstring path = m_path_name.substr(0, pos+1);
		wstring name = m_path_name.substr(pos+1);

		//read the directory info
		WIN32_FIND_DATAW FindFileData;
		HANDLE hFind;
		wstring search_str = path + L"*.tif*";
		hFind = FindFirstFileW(search_str.c_str(), &FindFileData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			int cnt = 0;
			m_batch_list.clear();
			wstring name_temp = path + FindFileData.cFileName;
			if (IsNewBatchFile(name_temp))
			{
				m_batch_list.push_back(name_temp);
				if (IsBatchFileIdentical(name_temp, m_path_name))
					m_cur_batch = cnt;
				cnt++;
			}

			while (FindNextFileW(hFind, &FindFileData) != 0)
			{
				name_temp = path + FindFileData.cFileName;
				if (IsNewBatchFile(name_temp))
				{
					m_batch_list.push_back(name_temp);
					if (IsBatchFileIdentical(name_temp, m_path_name))
						m_cur_batch = cnt;
					cnt++;
				}
			}
		}
		FindClose(hFind);

		m_batch = true;
	}
	else
		m_batch = false;
}

bool TIFReader::IsNewBatchFile(wstring name)
{
	if (m_batch_list.size() == 0)
		return true;

	for (int i=0; i<(int)m_batch_list.size(); i++)
	{
		if (IsBatchFileIdentical(name, m_batch_list[i]))
			return false;
	}

	return true;
}

bool TIFReader::IsBatchFileIdentical(wstring name1, wstring name2)
{
	if (m_4d_seq.size() > 1)
	{
		size_t pos = name1.find(m_time_id);
		if (pos == -1)
			return false;
		wstring find_str = name1.substr(0, pos+2);
		pos = name2.find(find_str);
		if (pos == -1)
			return false;
		else
			return true;
	}
	else if (m_slice_seq)
	{
		size_t pos = name1.find_last_of(L'.');
		size_t begin = -1;
		size_t end = -1;
		for (int i=int(pos)-1; i>=0; i--)
		{
			if (iswdigit(name1[i]) && end==-1 && begin==-1)
				end = i;
			if (!iswdigit(name1[i]) && end!=-1 && begin==-1)
			{
				begin = i;
				break;
			}
		}
		if (begin == -1)
			return false;
		else
		{
			wstring find_str = name1.substr(0, begin+1);
			pos = name2.find(find_str);
			if (pos == -1)
				return false;
			else
				return true;
		}
	}
	else
	{
		if (name1 == name2)
			return true;
		else
			return false;
	}
}

int TIFReader::LoadBatch(int index)
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

int TIFReader::LoadOffset(int offset)
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

Nrrd* TIFReader::Convert(bool get_max)
{
	return Convert(0, 0, get_max);
}

Nrrd* TIFReader::Convert(int c, bool get_max)
{
	return Convert(0, c, get_max);
}

Nrrd* TIFReader::Convert(int t, int c, bool get_max)
{
	if (t<0 || t>=m_time_num ||
		c<0 || c>=m_chan_num)
		return 0;

	Nrrd* data = 0; 
	TimeDataInfo chan_info = m_4d_seq[t];
	m_data_name = chan_info.slices[0].slice.substr(chan_info.slices[0].slice.find_last_of(L'\\')+1);
	if (chan_info.type == 0)	//single file
		data = ReadSingleTiff(chan_info.slices[0].slice, c, get_max);
	else	//sequence tiff
		data = ReadSequenceTiff(chan_info.slices, c, get_max);

	m_cur_time = t;
	return data;
}

wstring TIFReader::GetCurName(int t, int c)
{
	if (t>=0 && t<m_4d_seq.size())
		return (m_4d_seq[t].slices)[0].slice;
	else
		return L"";
}

bool TIFReader::tif_sort(const TimeDataInfo& info1, const TimeDataInfo& info2)
{
	return info1.filenumber < info2.filenumber;
}

bool TIFReader::tif_slice_sort(const SliceInfo& info1, const SliceInfo& info2)
{
	return info1.slicenumber < info2.slicenumber;
}

uint32_t TIFReader::GetNumTiffPages(std::wstring name) 
{
	uint32_t count = 0;
	std::ifstream is(name, std::ifstream::binary);
	is.seekg(2,is.beg);
	uint16_t tiff_num=0;
	is.read((char*)&tiff_num,sizeof(uint16_t));
	bool swap =  SwapShort(tiff_num) == 42;
	if (tiff_num == 42 || swap) {
		//find the first IFD block/page
		is.seekg(4,is.beg);
		while (true) {
			uint32_t ifd_offset=0;
			is.read((char*)&ifd_offset,sizeof(uint32_t));
			//go to the first IFD block/page
			if (swap) ifd_offset = SwapWord(ifd_offset);
			//stop if there are no more directories/pages
			if (ifd_offset == 0) break;
			is.seekg(ifd_offset,is.beg);
			uint16_t num_entries=0;
			//how many entries are there?
			is.read((char*)&num_entries,sizeof(uint16_t));
			if (swap) num_entries = SwapShort(num_entries);
			bool thumbnail = false;
			for(size_t i = 0; i < num_entries; i++) {
				is.seekg(ifd_offset+2+12*i,is.beg);
				uint16_t tag=0;
				//get the tag of entry
				is.read((char*)&tag,sizeof(uint16_t));
				if (swap) tag = SwapShort(tag);
				if (tag == kSubFileTypeTag) {
					//find the type
					is.seekg(ifd_offset+2+12*i+2,is.beg);
					uint16_t type=0;
					is.read((char*)&type,sizeof(uint16_t));
					if (swap) type = SwapShort(type);
					// This is the correct tag, grab and go.
					is.seekg(ifd_offset+2+12*i+8,is.beg);
					uint32_t value=0;
					is.read((char*)&value,sizeof(uint32_t));
					if (swap) value = SwapWord(value);
					if (value == 1) thumbnail = true;
					break;
				}
			}
			if (!thumbnail) count++;
			is.seekg(ifd_offset+2+12*num_entries,is.beg);
		} 
	}
	is.close();
	return count;
}

Nrrd* TIFReader::ReadSingleTiff(std::wstring filename, int c, bool get_max)
{
	TIFF* infile = TIFFOpenW(filename.c_str(), "r");
	if (!infile)
		return 0;
	int numPages = 0;
	if (get_max)
		numPages = GetNumTiffPages(filename);
	else
		numPages = m_slice_num;

	uint32_t width = GetTiffField(filename,kImageWidthTag);
	uint32_t height = GetTiffField(filename,kImageLengthTag);
	uint16 bits = GetTiffField(filename,kBitsPerSampleTag);
	uint16 samples = GetTiffField(filename,kSamplesPerPixelTag);
	float x_res = 0.0;
	float y_res = 0.0;
	double z_res = 0.0;
	TIFFGetField(infile, TIFFTAG_XRESOLUTION, &x_res);
	TIFFGetField(infile, TIFFTAG_YRESOLUTION, &y_res);
	uint32_t rowsperstrip = GetTiffField(filename,kRowsPerStripTag);
	uint32_t strip_size = rowsperstrip * width * samples * (bits/8);
	char* img_desc = 0;
	TIFFGetField(infile, TIFFTAG_IMAGEDESCRIPTION, &img_desc);
	if (img_desc)
	{
		std::string desc = std::string ((char*)img_desc);
		size_t start = desc.find("spacing=");
		if (start!=-1)
		{
			string spacing = desc.substr(start+8);
			size_t end = spacing.find("\n");
			if (end != -1)
			{
				spacing = spacing.substr(0, end);
				z_res = atof(spacing.c_str());
			}
		}
	}

	if (x_res>0.0 && y_res>0.0 && z_res>0.0)
	{
		m_xspc = 1.0/x_res;
		m_yspc = 1.0/y_res;
		m_zspc = z_res;
		m_valid_spc = true;
	}
	else
	{
		m_xspc = 1.0;
		m_yspc = 1.0;
		m_zspc = 1.0;
		m_valid_spc = false;
	}
	switch (m_resize_type)
	{
	case 0:
		m_slice_num = numPages;
		m_x_size = width;
		m_y_size = height;
		break;
	case 1:
		m_slice_num = numPages;
		if (m_alignment > 1)
		{
			m_x_size = (width/m_alignment+(width%m_alignment?1:0))*m_alignment;
			m_y_size = (height/m_alignment+(height%m_alignment?1:0))*m_alignment;
		}
		break;
	case 2://not implemented
		m_slice_num = numPages;
		m_x_size = width;
		m_y_size = height;
		break;
	}
	int pagepixels = m_x_size*m_y_size;

	Nrrd *nrrdout = nrrdNew();

	//allocate memory
	uint16_t *val16 = 0;
	uint8_t *val8 = 0;

	if (bits == 16)
	{
		long long total_size = (long long)m_x_size*(long long)m_y_size*(long long)numPages;
		val16 = new (std::nothrow) uint16[total_size];
		if (!val16)
			return 0;
	}
	else
	{
		long long total_size = (long long)m_x_size*(long long)m_y_size*(long long)numPages;
		val8 = new (std::nothrow) uint8_t[total_size];
		if (!val8)
			return 0;
	}

	int pageindex = 0;
	int max_value = 0;
	bool use_max_value = false;

	tstrip_t strip;
	tdata_t buf = 0;
	if (samples > 1)
		buf = _TIFFmalloc(strip_size);

	do
	{
		unsigned int subfiletype = 0;
		TIFFGetField(infile, TIFFTAG_SUBFILETYPE, &subfiletype);
		if (subfiletype == 1)	//this is a thumbnail, skip
			continue;

		for (strip=0; strip<TIFFNumberOfStrips(infile); strip++)
		{
			long long valindex;
			int indexinpage;
			if (bits == 16)
			{
				if (samples > 1)
				{
					TIFFReadEncodedStrip(infile, strip, buf, (tsize_t)-1);
					int num_pixels = strip_size/samples/2;
					indexinpage = strip*num_pixels;
					valindex = pageindex*pagepixels + indexinpage;
					for (int i=0; i<num_pixels; i++)
					{
						if (indexinpage++ >= pagepixels)
							break;
						memcpy(val16+valindex, (uint16*)buf+samples*i+c, sizeof(uint16));
						if (get_max && val16[valindex] > max_value)
							max_value = val16[valindex];
					}
					use_max_value = true;
				}
				else
				{
					valindex = pageindex*pagepixels + strip*strip_size/2;
					TIFFReadEncodedStrip(infile, strip, val16+valindex, (tsize_t)-1);
				}
			}
			else if (bits == 8)
			{
				if (samples > 1)
				{
					TIFFReadEncodedStrip(infile, strip, buf, (tsize_t)-1);
					int num_pixels = strip_size/samples;
					indexinpage = strip*num_pixels;
					valindex = pageindex*pagepixels + indexinpage;
					for (int i=0; i<num_pixels; i++)
					{
						if (indexinpage++ >= pagepixels)
							break;
						memcpy(val8+valindex, (uint8_t*)buf+samples*i+c, sizeof(uint8_t));
						valindex++;
					}
				}
				else
				{
					valindex = pageindex*pagepixels + strip*strip_size;
					TIFFReadEncodedStrip(infile, strip, val8+valindex, (tsize_t)-1);
				}
			}
		}
		pageindex++;
	} while (TIFFReadDirectory(infile));
	
	if (samples > 1)
		_TIFFfree(buf);
	TIFFClose(infile);

	//write to nrrd
	if (bits == 8)
		nrrdWrap(nrrdout, val8, nrrdTypeUChar, 3, (size_t)m_x_size, (size_t)m_y_size, (size_t)numPages);
	else if (bits == 16)
		nrrdWrap(nrrdout, val16, nrrdTypeUShort, 3, (size_t)m_x_size, (size_t)m_y_size, (size_t)numPages);
	nrrdAxisInfoSet(nrrdout, nrrdAxisInfoSpacing, m_xspc, m_yspc, m_zspc);
	nrrdAxisInfoSet(nrrdout, nrrdAxisInfoMax, m_xspc*m_x_size, m_yspc*m_y_size, m_zspc*numPages);
	nrrdAxisInfoSet(nrrdout, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
	nrrdAxisInfoSet(nrrdout, nrrdAxisInfoSize, (size_t)m_x_size, (size_t)m_y_size, (size_t)numPages);

	if (bits == 16)
	{
		if (get_max)
		{
			if (use_max_value)
				m_max_value = max_value;
			else
			{
				double value;
				for (int i=0; i<m_slice_num*m_x_size*m_y_size; i++)
				{
					value= ((unsigned short*)nrrdout->data)[i];
					m_max_value = value>m_max_value ? value : m_max_value;
				}
			}
		}
		if (m_max_value > 0.0)
			m_scalar_scale = 65535.0 / m_max_value;
		else
			m_scalar_scale = 1.0;
	}
	else
		m_max_value = 255.0;

	return nrrdout;
}

Nrrd* TIFReader::ReadSequenceTiff(std::vector<SliceInfo> &filelist, int c, bool get_max)
{
	if (filelist.size()<=0)
		return 0;

	int i;
	int numPages = int(filelist.size());
	wstring filename = filelist[0].slice;
	TIFF* infile = TIFFOpenW(filename.c_str(), "r");
	if (!infile)
		return 0;

	uint32_t width;
	uint32_t height;
	TIFFGetField(infile, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(infile, TIFFTAG_IMAGELENGTH, &height);
	uint16 bits;
	TIFFGetField(infile, TIFFTAG_BITSPERSAMPLE, &bits);
	uint16 samples;
	TIFFGetField(infile, TIFFTAG_SAMPLESPERPIXEL, &samples);
	float x_res = 0.0;
	float y_res = 0.0;
	double z_res = 0.0;
	TIFFGetField(infile, TIFFTAG_XRESOLUTION, &x_res);
	TIFFGetField(infile, TIFFTAG_YRESOLUTION, &y_res);
	uint32_t rowsperstrip;
	TIFFGetField(infile, TIFFTAG_ROWSPERSTRIP, &rowsperstrip);
	uint32_t strip_size = rowsperstrip * width * samples * (bits/8);
	char* img_desc = 0;
	TIFFGetField(infile, TIFFTAG_IMAGEDESCRIPTION, &img_desc);
	if (img_desc)
	{
		string desc = string ((char*)img_desc);
		size_t start = desc.find("spacing=");
		if (start!=-1)
		{
			string spacing = desc.substr(start+8);
			size_t end = spacing.find("\n");
			if (end != -1)
			{
				spacing = spacing.substr(0, end);
				z_res = atof(spacing.c_str());
			}
		}
	}

	if (x_res>0.0 && y_res>0.0 && z_res>0.0)
	{
		m_xspc = 1.0/x_res;
		m_yspc = 1.0/y_res;
		m_zspc = z_res;
		m_valid_spc = true;
	}
	else
	{
		m_xspc = 1.0;
		m_yspc = 1.0;
		m_zspc = 1.0;
		m_valid_spc = false;
	}
	switch (m_resize_type)
	{
	case 0:
		m_slice_num = numPages;
		m_x_size = width;
		m_y_size = height;
		break;
	case 1:
		m_slice_num = numPages;
		if (m_alignment > 1)
		{
			m_x_size = (width/m_alignment+(width%m_alignment?1:0))*m_alignment;
			m_y_size = (height/m_alignment+(height%m_alignment?1:0))*m_alignment;
		}
		break;
	case 2://not implemented
		m_slice_num = numPages;
		m_x_size = width;
		m_y_size = height;
		break;
	}
	int pagepixels = m_x_size*m_y_size;

	TIFFClose(infile);

	Nrrd *nrrdout = nrrdNew();

	//allocate memory
	uint16 *val16 = 0;
	uint8_t *val8 = 0;

	if (bits == 16)
	{
		long long total_size = (long long)m_x_size*(long long)m_y_size*(long long)numPages;
		val16 = new (std::nothrow) uint16[total_size];
		if (!val16)
			return 0;
	}
	else
	{
		long long total_size = (long long)m_x_size*(long long)m_y_size*(long long)numPages;
		val8 = new (std::nothrow) uint8_t[total_size];
		if (!val8)
			return 0;
	}

	int pageindex = 0;
	int max_value = 0;
	bool use_max_value = false;

	tstrip_t strip;
	tdata_t buf = 0;
	if (samples > 1)
		buf = _TIFFmalloc(strip_size);

	for (i=0; i<(int)filelist.size(); i++)
	{
		filename = filelist[i].slice;
		infile = TIFFOpenW(filename.c_str(), "r");
		if (!infile)
			continue;

		//get bit info
		uint16 bits_file;
		TIFFGetField(infile, TIFFTAG_BITSPERSAMPLE, &bits_file);

		//read file
		for (strip=0; strip<TIFFNumberOfStrips(infile); strip++)
		{
			long long valindex;
			int indexinpage;
			if (bits == 16)
			{
				if (samples > 1)
				{
					TIFFReadEncodedStrip(infile, strip, buf, (tsize_t)-1);
					int num_pixels = strip_size/samples/2;
					indexinpage = strip*num_pixels;
					valindex = pageindex*pagepixels + indexinpage;
					for (int i=0; i<num_pixels; i++)
					{
						if (indexinpage++ >= pagepixels)
							break;
						memcpy(val16+valindex, (uint16*)buf+samples*i+c, sizeof(uint16));
						if (get_max && val16[valindex] > max_value)
							max_value = val16[valindex];
					}
					use_max_value = true;
				}
				else
				{
					valindex = pageindex*pagepixels + strip*strip_size/2;
					TIFFReadEncodedStrip(infile, strip, val16+valindex, (tsize_t)-1);
				}
			}
			else if (bits == 8)
			{
				if (samples > 1)
				{
					TIFFReadEncodedStrip(infile, strip, buf, (tsize_t)-1);
					int num_pixels = strip_size/samples;
					indexinpage = strip*num_pixels;
					valindex = pageindex*pagepixels + indexinpage;
					for (int i=0; i<num_pixels; i++)
					{
						if (indexinpage++ >= pagepixels)
							break;
						memcpy(val8+valindex, (uint8_t*)buf+samples*i+c, sizeof(uint8_t));
						valindex++;
					}
				}
				else
				{
					valindex = pageindex*pagepixels + strip*strip_size;
					TIFFReadEncodedStrip(infile, strip, val8+valindex, (tsize_t)-1);
				}
			}
		}
		pageindex++;
		TIFFClose(infile);
	}

	if (samples > 1)
		_TIFFfree(buf);

	//write to nrrd
	if (bits == 8)
		nrrdWrap(nrrdout, val8, nrrdTypeUChar, 3, (size_t)m_x_size, (size_t)m_y_size, (size_t)numPages);
	else if (bits == 16)
		nrrdWrap(nrrdout, val16, nrrdTypeUShort, 3, (size_t)m_x_size, (size_t)m_y_size, (size_t)numPages);
	nrrdAxisInfoSet(nrrdout, nrrdAxisInfoSpacing, m_xspc, m_yspc, m_zspc);
	nrrdAxisInfoSet(nrrdout, nrrdAxisInfoMax, m_xspc*m_x_size, m_yspc*m_y_size, m_zspc*numPages);
	nrrdAxisInfoSet(nrrdout, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
	nrrdAxisInfoSet(nrrdout, nrrdAxisInfoSize, (size_t)m_x_size, (size_t)m_y_size, (size_t)numPages);

	if (bits == 16)
	{
		if (get_max)
		{
			if (use_max_value)
				m_max_value = max_value;
			else
			{
				double value;
				for (i=0; i<m_slice_num*m_x_size*m_y_size; i++)
				{
					value= ((unsigned short*)nrrdout->data)[i];
					m_max_value = value>m_max_value ? value : m_max_value;
				}
			}
		}
		if (m_max_value > 0.0)
			m_scalar_scale = 65535.0 / m_max_value;
		else
			m_scalar_scale = 1.0;
	}
	else
		m_max_value = 255.0;

	return nrrdout;
}

