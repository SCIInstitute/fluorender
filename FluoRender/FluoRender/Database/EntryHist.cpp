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
#include <EntryHist.h>
#include <Table.h>
#include <File.h>

using namespace flrd;

unsigned int EntryHist::m_bins = 15;//fixed size

EntryHist::EntryHist() :
	m_min(0),
	m_max(0),
	m_population(0)
{
	m_data.assign(m_bins, 0);
}

EntryHist::EntryHist(const EntryHist& ent) :
	Entry(ent),
	m_min(ent.m_min),
	m_max(ent.m_max),
	m_population(ent.m_population)
{

}

EntryHist::~EntryHist()
{
}

void EntryHist::open(File& file)
{
	//bins
	if (file.check(TAG_ENT_SIZE))
	{
		unsigned int bins;
		file.readValue(bins);
		if (bins != m_bins) return;
	}

	//min/max
	if (file.check(TAG_ENT_MIN))
		file.readValue(m_min);
	if (file.check(TAG_ENT_MAX))
		file.readValue(m_max);

	//population
	if (file.check(TAG_ENT_POPL))
		file.readValue(m_population);

	//data
	if (file.check(TAG_ENT_DATA))
		file.readVector(m_data);
}

void EntryHist::save(File& file)
{
	//type
	file.writeValue(Table::TAG_TABLE_ENT_HIST);

	//bins
	file.writeValue(TAG_ENT_SIZE);
	file.writeValue(m_bins);

	//min/max
	file.writeValue(TAG_ENT_MIN);
	file.writeValue(m_min);
	file.writeValue(TAG_ENT_MAX);
	file.writeValue(m_max);

	//population
	file.writeValue(TAG_ENT_POPL);
	file.writeValue(m_population);

	//data
	file.writeValue(TAG_ENT_DATA);
	file.writeVector(m_data);
}
