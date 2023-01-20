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
#include <algorithm>

using namespace flrd;

TableHistParams::TableHistParams() :
	Table(),
	m_hist_popl(0),
	m_param_iter(0),
	m_param_mxdist(0),
	m_param_cleanb(0),
	m_param_clean_iter(0)
{
}

TableHistParams::TableHistParams(const TableHistParams& table) :
	Table(table),
	m_hist_popl(table.m_hist_popl),
	m_param_iter(table.m_param_iter),
	m_param_mxdist(table.m_param_mxdist),
	m_param_cleanb(table.m_param_cleanb),
	m_param_clean_iter(table.m_param_clean_iter)
{

}

TableHistParams::~TableHistParams()
{

}

EntryParams* TableHistParams::findNearestOutput(EntryHist* input)
{
	Record* result = 0;
	float vmin = std::numeric_limits<float>::max();
	for (auto i : m_data)
	{
		float v = i->compare(input);
		if (v < vmin)
		{
			result = i;
			vmin = v;
		}
	}
	if (result)
		return dynamic_cast<EntryParams*>(result->getOutput());
	return 0;
}

void TableHistParams::addRecord(Record* rec)
{
	Table::addRecord(rec);
	compute(rec);
}

void TableHistParams::open(const std::string& filename)
{
	Table::open(filename);
	compute();
}

void TableHistParams::compute(Record* rec)
{
	computeHistSize(rec);
	computeParamIter(rec);
}

void TableHistParams::computeHistSize(Record* rec)
{
	double sum = 0;
	if (m_data.empty())
	{
		m_hist_popl = 0;
		return;
	}

	if (rec)
	{
		RecordHistParams* r = dynamic_cast<RecordHistParams*>(rec);
		if (r)
		{
			m_hist_popl *= m_data.size() - 1;
			m_hist_popl += r->getHistPopl();
			m_hist_popl /= m_data.size();
		}
		return;
	}

	for (auto i : m_data)
	{
		RecordHistParams* r = dynamic_cast<RecordHistParams*>(i);
		if (r)
			sum += r->getHistPopl();
	}

	m_hist_popl = (float)(sum / m_data.size());
}

void TableHistParams::getParams(Record* rec)
{
	RecordHistParams* r = dynamic_cast<RecordHistParams*>(rec);
	if (r)
	{
		m_param_iter = std::max(m_param_iter, r->getParamIter());
		m_param_mxdist = std::max(m_param_mxdist, r->getParamMxdist());
		m_param_cleanb = std::max(m_param_cleanb, r->getParamCleanb());
		m_param_clean_iter = std::max(m_param_clean_iter, r->getParamCleanIter());
	}
}

void TableHistParams::computeParamIter(Record* rec)
{
	if (m_data.empty())
	{
		m_param_iter = 0;
		return;
	}

	if (rec)
	{
		getParams(rec);
		return;
	}

	for (auto i : m_data)
	{
		getParams(i);
	}
}