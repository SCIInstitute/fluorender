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
#include <obj_reader.h>
#include <glm.h>
#include <compatibility.h>

ObjReader::ObjReader():
	BaseMeshReader()
{
}

GLMmodel* ObjReader::Convert(int t)
{
	if (t < 0 || t >= m_time_num)
		return nullptr;

	std::wstring str_name = m_4d_seq[t].filename;
	m_data_name = GET_STEM(str_name);
	m_cur_time = t;
	std::string str_fn = ws2s(str_name);
	bool no_fail = true;
	return glmReadOBJ(str_fn.c_str(), &no_fail);
}

void ObjReader::SetBatch(bool batch)
{
	if (batch)
	{
		//read the directory info
		std::wstring search_path = GET_PATH(m_path_name);
		FIND_FILES_BATCH(search_path,ESCAPE_REGEX(L".obj"),m_batch_list,m_cur_batch);
		m_batch = true;
	}
	else
		m_batch = false;
}

