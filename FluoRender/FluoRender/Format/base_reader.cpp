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

#include <base_reader.h>

BaseReader::BaseReader() :
	Progress()
{
}

int BaseReader::LoadOffset(int offset)
{
	if (m_batch_list.size() <= 1) return -1;
	int result = offset;
	if (offset < 0)
		result = 0;
	if (offset >= (int)m_batch_list.size())
		result = (int)m_batch_list.size() - 1;
	m_path_name = m_batch_list[result];
	Preprocess();
	m_cur_batch = result;
	return result;
}

int BaseReader::GetOffset()
{
	return m_cur_batch;
}

std::string BaseReader::GetError(int code)
{
	std::string err_str;
	switch (code)
	{
	case READER_OK:
		err_str = "No Error.";
		break;
	case READER_OPEN_FAIL:
		err_str = "Cannot open file.";
		break;
	case READER_FORMAT_ERROR:
		err_str = "Cannot understand file format.";
		break;
	case READER_EMPTY_DATA:
		err_str = "File is empty.";
		break;
	case READER_FP64_DATA:
		err_str = "Sample format is unsupported.";
		break;
	case READER_JAVA_ARRAY_SIZE_ERROR:
		err_str = "File contains too many voxels for Java.\n" \
			"Max is 2^31-1 (2,147,483,647) voxels.";
		break;
	}
	return err_str;
}
