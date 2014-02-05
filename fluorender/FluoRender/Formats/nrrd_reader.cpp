#include <Formats\nrrd_reader.h>
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
	size_t pos = m_path_name.find_last_of(L'\\');
	if (pos == -1)
		return;
	wstring path = m_path_name.substr(0, pos+1);
	wstring name = m_path_name.substr(pos+1);
	//generate search name for time sequence
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
		wstring search_str = path + name.substr(0, begin+id_len) +
			L"*" + name.substr(end);
		//search time sequence files
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
			info.filename = path + str;
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
				info.filename = path + str;
				m_4d_seq.push_back(info);
			}
		}
		FindClose(hFind);
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
		wstring search_path = m_path_name.substr(0, m_path_name.find_last_of(L'\\')) + L'\\';
		wstring search_str = search_path + L"*.nrrd";
		WIN32_FIND_DATAW FindFileData;
		HANDLE hFind;
		hFind = FindFirstFileW(search_str.c_str(), &FindFileData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			int cnt = 0;
			m_batch_list.clear();
			wstring name = search_path + FindFileData.cFileName;
			m_batch_list.push_back(name);
			if (name == m_path_name)
				m_cur_batch = cnt;
			cnt++;

			while (FindNextFileW(hFind, &FindFileData) != 0)
			{
				name = search_path + FindFileData.cFileName;
				m_batch_list.push_back(name);
				if (name == m_path_name)
					m_cur_batch = cnt;
				cnt++;
			}
		}
		FindClose(hFind);

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

int NRRDReader::LoadOffset(int offset)
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

Nrrd* NRRDReader::Convert(bool get_max)
{
	return Convert(0, 0, get_max);
}

Nrrd* NRRDReader::Convert(int c, bool get_max)
{
	return Convert(0, c, get_max);
}

Nrrd* NRRDReader::Convert(int t, int c, bool get_max)
{
	if (t<0 || t>=m_time_num)
		return 0;

	int i;

	m_data_name = m_4d_seq[t].filename.substr(m_4d_seq[t].filename.find_last_of(L'\\')+1);

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
	if (output->type == nrrdTypeUShort)
		data_size *= 2;
	output->data = new unsigned char[data_size];

	if (nrrdLoad(output, str.c_str(), NULL))
	{
		delete []output->data;
		nrrdNix(output);
		return 0;
	}

	m_max_value = 0.0;
	//find max value
	if (output->type == nrrdTypeUChar)
	{
		//8 bit
		m_max_value = 255.0;
		m_scalar_scale = 1.0;
	}
	else if (output->type == nrrdTypeUShort)
	{
		//16 bit
		if (get_max)
		{
			double value;
			for (i=0; i<m_slice_num*m_x_size*m_y_size; i++)
			{
				value= ((unsigned short*)output->data)[i];
				m_max_value = value>m_max_value ? value : m_max_value;
			}
		}

		if (m_max_value > 0.0)
			m_scalar_scale = 65535.0 / m_max_value;
		else
			m_scalar_scale = 1.0;
	}
	else
	{
		delete []output->data;
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
