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
#include "lbl_reader.h"
#include "../compatibility.h"
#include <sstream>
#include <inttypes.h>

LBLReader::LBLReader()
{
	m_id_string = L"FluoRender_mask_reader_id";
}

LBLReader::~LBLReader()
{
}

void LBLReader::SetFile(string &file)
{
	if (!file.empty())
	{
		if (!m_path_name.empty())
			m_path_name.clear();
		m_path_name.assign(file.length(), L' ');
		copy(file.begin(), file.end(), m_path_name.begin());
	}
}

void LBLReader::SetFile(wstring &file)
{
	m_path_name = file;
}

void LBLReader::Preprocess()
{
}

void LBLReader::SetSliceSeq(bool ss)
{
}

bool LBLReader::GetSliceSeq()
{
	return false;
}

void LBLReader::SetTimeId(wstring &id)
{
}

wstring LBLReader::GetTimeId()
{
	return wstring(L"");
}

void LBLReader::SetBatch(bool batch)
{
}

int LBLReader::LoadBatch(int index)
{
	return 0;
}

Nrrd* LBLReader::Convert(int t, int c, bool get_max)
{
	int64_t pos = m_path_name.find_last_of('.');
	if (pos == -1)
		return 0;
	wstring str_name = m_path_name.substr(0, pos);
	wostringstream strs;
	strs << str_name /*<< "_t" << t << "_c" << c*/ << ".lbl";
	str_name = strs.str();
	FILE* lbl_file = 0;
	if (!WFOPEN(&lbl_file, str_name.c_str(), L"rb"))
		return 0;

	Nrrd *output = nrrdNew();
	NrrdIoState *nio = nrrdIoStateNew();
	nrrdIoStateSet(nio, nrrdIoStateSkipData, AIR_TRUE);
	if (nrrdRead(output, lbl_file, nio))
	{
		fclose(lbl_file);
		return 0;
	}
	nio = nrrdIoStateNix(nio);
	rewind(lbl_file);
	if (output->dim != 3 ||
		(output->type != nrrdTypeInt &&
		output->type != nrrdTypeUInt))
	{
		delete []output->data;
		nrrdNix(output);
		fclose(lbl_file);
		return 0;
	}
	int slice_num = int(output->axis[2].size);
	int x_size = int(output->axis[0].size);
	int y_size = int(output->axis[1].size);
	int data_size = slice_num * x_size * y_size;
	output->data = new unsigned int[data_size];

	if (nrrdRead(output, lbl_file, NULL))
	{
		delete []output->data;
		nrrdNix(output);
		fclose(lbl_file);
		return 0;
	}

	fclose(lbl_file);
	return output;
}

wstring LBLReader::GetCurName(int t, int c)
{
	return wstring(L"");
}
