/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2022 Scientific Computing and Imaging Institute,
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
#include "TableHistParams.h"
#include "RecordHistParams.h"

using namespace flrd;

TableHistParams::TableHistParams() :
	Table()
{
	m_hist_size = 0;
}

TableHistParams::~TableHistParams()
{

}

void TableHistParams::addRecord(Record* rec)
{
	Table::addRecord(rec);
	computeHistSize();
}

void TableHistParams::open(const std::string& filename)
{
	Table::open(filename);
	computeHistSize();
}

void TableHistParams::computeHistSize()
{
	double sum = 0;
	if (m_data.empty())
	{
		m_hist_size = 0;
		return;
	}

	for (auto i : m_data)
	{
		RecordHistParams* r = dynamic_cast<RecordHistParams*>(i);
		if (r)
			sum += r->getHistSize();
	}

	m_hist_size = (float)(sum / m_data.size());
}