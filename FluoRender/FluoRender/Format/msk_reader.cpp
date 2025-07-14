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
#include <msk_reader.h>
#include <compatibility.h>
#include <sstream>
#include <inttypes.h>

MSKReader::MSKReader():
	BaseReader()
{
	m_fp_convert = false;
	m_fp_min = 0;
	m_fp_max = 1;

	m_id_string = L"FluoRender_mask_reader_id";
}

MSKReader::~MSKReader()
{
}

//void MSKReader::SetFile(const std::string &file)
//{
//	if (!file.empty())
//	{
//		if (!m_path_name.empty())
//			m_path_name.clear();
//		m_path_name.assign(file.length(), L' ');
//		copy(file.begin(), file.end(), m_path_name.begin());
//	}
//}

void MSKReader::SetFile(const std::wstring &file)
{
	m_path_name = file;
}

int MSKReader::Preprocess()
{
	return READER_OK;
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
	FILE* msk_file = 0;
	if (!WFOPEN(&msk_file, m_path_name, L"rb"))
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
		nrrdNuke(output);
		fclose(msk_file);
		return 0;
	}
	int slice_num = int(output->axis[2].size);
	int x_size = int(output->axis[0].size);
	int y_size = int(output->axis[1].size);
	unsigned long long data_size = (unsigned long long)slice_num * x_size * y_size;
	output->data = new unsigned char[data_size];
	if (!output->data)
	{
		nrrdNuke(output);
		fclose(msk_file);
		return 0;
	}

	if (nrrdRead(output, msk_file, NULL))
	{
		nrrdNuke(output);
		fclose(msk_file);
		return 0;
	}

	fclose(msk_file);
	return output;
}

std::wstring MSKReader::GetCurDataName(int t, int c)
{
	return L"";
}

std::wstring MSKReader::GetCurMaskName(int t, int c)
{
	return L"";
}

std::wstring MSKReader::GetCurLabelName(int t, int c)
{
	return L"";
}
