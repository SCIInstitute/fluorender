/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2024 Scientific Computing and Imaging Institute,
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
#include <EntryParams.h>
#include <Table.h>
#include <File.h>

using namespace flrd;

EntryParams::EntryParams() :
	m_params(nullptr),
	m_valid(false)
{}

EntryParams::EntryParams(const EntryParams& ent) :
	Entry(ent)
{
	m_valid = ent.m_valid;
	m_params = ent.m_params;
}

EntryParams::~EntryParams()
{
}

void EntryParams::setParams(Params* params)
{
	m_valid = false;
	if (!params)
		return;
	m_params = params;
	size_t size = params->size();
	//fixed size
	if (size)
		m_data.assign(size, 0);
}

void EntryParams::open(File& file)
{
	if (!m_params)
		return;
	//size
	if (file.check(TAG_ENT_SIZE))
	{
		unsigned int size;
		file.readValue(size);
		if (size != m_params->size())
			return;
	}

	//data
	if (file.check(TAG_ENT_DATA))
		file.readVector(m_data);

	m_valid = true;
}

void EntryParams::save(File& file)
{
	if (!m_params)
		return;
	//type
	file.writeValue(Table::TAG_TABLE_ENT_PARAMS);

	//size
	file.writeValue(TAG_ENT_SIZE);
	unsigned int size = m_params->size();
	file.writeValue(size);

	//data
	file.writeValue(TAG_ENT_DATA);
	file.writeVector(m_data);
}
