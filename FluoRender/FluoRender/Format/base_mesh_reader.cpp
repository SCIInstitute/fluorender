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
#include <base_mesh_reader.h>
#include <glm.h>
#include <compatibility.h>

BaseMeshReader::BaseMeshReader() :
	BaseReader()
{
}

void BaseMeshReader::SetFile(const std::wstring& file)
{
	m_path_name = file;
	m_id_string = m_path_name;
}

int BaseMeshReader::Preprocess()
{
	m_4d_seq.clear();

	//separate path and name
	std::wstring path, name;
	if (!SEP_PATH_NAME(m_path_name, path, name))
		return READER_OPEN_FAIL;

	//build 4d sequence
	//search time sequence files
	std::vector< std::wstring> list;
	if (!FIND_FILES_4D(m_path_name, m_time_id, list, m_cur_time))
	{
		TimeDataInfo info;
		info.filenumber = 0;
		info.filename = m_path_name;
		m_4d_seq.push_back(info);
		m_cur_time = 0;
	}
	else
	{
		int64_t begin = m_path_name.find(m_time_id);
		size_t id_len = m_time_id.length();
		for(size_t i = 0; i < list.size(); i++) {
			TimeDataInfo info;
			std::wstring str = list.at(i);
			std::wstring t_num;
			for (size_t j = begin + id_len; j < str.size(); j++)
			{
				wchar_t c = str[j];
				if (iswdigit(c))
					t_num.push_back(c);
				else break;
			}
			if (t_num.size() > 0)
				info.filenumber = WSTOI(t_num);
			else
				info.filenumber = 0;
			info.filename = list.at(i);
			m_4d_seq.push_back(info);
		}
	}
	if (m_4d_seq.size() > 0)
	{
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

	//get time number
	m_time_num = (int)m_4d_seq.size();

	return READER_OK;
}

GLMmodel* BaseMeshReader::Convert()
{
	return Convert(0);
}

int BaseMeshReader::LoadBatch(int index)
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

std::wstring BaseMeshReader::GetCurDataName(int t)
{
	if (t >= 0 && t < (int)m_4d_seq.size())
		return m_4d_seq[t].filename;
	return m_path_name;
}

