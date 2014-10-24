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
#include "msk_writer.h"
#include <sstream>
#include <inttypes.h>

MSKWriter::MSKWriter()
{
	m_data = 0;
	m_spcx = 0.0;
	m_spcy = 0.0;
	m_spcz = 0.0;
	m_use_spacings = false;
	m_time = 0;
	m_channel = 0;
}

MSKWriter::~MSKWriter()
{
}

void MSKWriter::SetData(Nrrd *data)
{
	m_data = data;
}

void MSKWriter::SetSpacings(double spcx, double spcy, double spcz)
{
	m_spcx = spcx;
	m_spcy = spcy;
	m_spcz = spcz;
	m_use_spacings = true;
}

void MSKWriter::SetCompression(bool value)
{
}

void MSKWriter::Save(wstring filename, int mode)
{
	if (!m_data)
		return;

	int64_t pos = filename.find_last_of('.');
	if (pos == -1)
		return;
	wstring str_name = filename.substr(0, pos);
	wostringstream strs;
	if (mode == 0)
		strs << str_name /*<< "_t" << m_time << "_c" << m_channel*/ << ".msk";
	else if (mode == 1)
		strs << str_name /*<< "_t" << m_time << "_c" << m_channel*/ << ".lbl";
	str_name = strs.str();

	if (m_use_spacings &&
		m_data->dim == 3)
	{
		nrrdAxisInfoSet(m_data, nrrdAxisInfoSpacing, m_spcx, m_spcy, m_spcz);
		nrrdAxisInfoSet(m_data, nrrdAxisInfoMax,
			m_spcx*m_data->axis[0].size,
			m_spcy*m_data->axis[1].size,
			m_spcz*m_data->axis[2].size);
	}

	string str;
	str.assign(str_name.length(), 0);
	for (int i=0; i<(int)str_name.length(); i++)
		str[i] = (char)str_name[i];
	nrrdSave(str.c_str(), m_data, NULL);
}

void MSKWriter::SetTC(int t, int c)
{
	m_time = t;
	m_channel = c;
}
