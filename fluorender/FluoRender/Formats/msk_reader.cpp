/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2014 Scientific Computing and Imaging Institute,
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
#include "msk_reader.h"
#include "../compatibility.h"
#include <sstream>
#include <inttypes.h>

MSKReader::MSKReader()
{
	m_id_string = L"FluoRender_mask_reader_id";
}

MSKReader::~MSKReader()
{
}

void MSKReader::SetFile(string &file)
{
	if (!file.empty())
	{
		if (!m_path_name.empty())
			m_path_name.clear();
		m_path_name.assign(file.length(), L' ');
		copy(file.begin(), file.end(), m_path_name.begin());
	}
}

void MSKReader::SetFile(wstring &file)
{
	m_path_name = file;
}

void MSKReader::Preprocess()
{
}

void MSKReader::SetSliceSeq(bool ss)
{
}

bool MSKReader::GetSliceSeq()
{
	return false;
}

void MSKReader::SetTimeId(wstring &id)
{
}

wstring MSKReader::GetTimeId()
{
	return wstring(L"");
}

void MSKReader::SetBatch(bool batch)
{
}

int MSKReader::LoadBatch(int index)
{
	return 0;
}

Nrrd* MSKReader::Convert(int t, int c, bool get_max)
{
	int64_t pos = m_path_name.find_last_of('.');
	if (pos == -1)
		return 0;
	wstring str_name = m_path_name.substr(0, pos);
	wostringstream strs;
	strs << str_name /*<< "_t" << t << "_c" << c*/ << ".msk";
	str_name = strs.str();
	FILE* msk_file = 0;
	if (!WFOPEN(&msk_file, str_name.c_str(), L"rb"))
		return 0;

	Nrrd *output = nrrdNew();
	NrrdIoState *nio = nrrdIoStateNew();
	nrrdIoStateSet(nio, nrrdIoStateSkipData, AIR_TRUE);
	if (nrrdRead(output, msk_file, nio))
	{
		fclose(msk_file);
		return 0;
	}
	nio = nrrdIoStateNix(nio);
	rewind(msk_file);
	if (output->dim != 3 ||
		(output->type != nrrdTypeChar &&
		output->type != nrrdTypeUChar))
	{
		delete []output->data;
		nrrdNix(output);
		fclose(msk_file);
		return 0;
	}
	int slice_num = int(output->axis[2].size);
	int x_size = int(output->axis[0].size);
	int y_size = int(output->axis[1].size);
	int data_size = slice_num * x_size * y_size;
	output->data = new unsigned char[data_size];

	if (nrrdRead(output, msk_file, NULL))
	{
		delete []output->data;
		nrrdNix(output);
		fclose(msk_file);
		return 0;
	}

	fclose(msk_file);
	return output;
}

wstring MSKReader::GetCurName(int t, int c)
{
	return wstring(L"");
}
