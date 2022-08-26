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
#include "Table.h"
#include "RecordHistParams.h"
#include <FileIO/File.h>
#include <fstream>

using namespace flrd;

Table::Table()
{
}

Table::~Table()
{
	clear();
}

void Table::clear()
{
	for (auto i : m_data)
	{
		delete i;
	}
	m_data.clear();
}

void Table::addRecord(Record* rec)
{
	if (!rec)
		return;
	m_data.push_back(rec);
}

void Table::open(const std::string& filename)
{
	flrd::File file;
	file.beginRead(filename);

	//header
	std::string str;
	str = file.readString(16);
	if (str != "FluoRender table")
	{
		file.endRead();
		return;
	}

	//rec num
	TableTags t;
	file.readValue(t);
	if (t != TAG_TABLE_REC_NUM)
	{
		file.endRead();
		return;
	}
	size_t n;
	file.readValue(n);

	//data
	if (!m_data.empty())
		clear();
	for (size_t i = 0; i < n; ++i)
	{
		file.readValue(t);
		Record* rec = 0;
		switch (t)
		{
		case TAG_TABLE_REC_HISTPARAM:
			rec = new RecordHistParams();
			break;
		default:
			break;
		}
		if (rec)
		{
			rec->open(file);
			addRecord(rec);
		}
	}

	file.endRead();
}

void Table::save(const std::string& filename)
{
	flrd::File file;
	file.beginWrite(filename);

	//header
	file.writeString("FluoRender table");

	//rec num
	file.writeValue(TAG_TABLE_REC_NUM);
	file.writeValue(m_data.size());

	//data
	for (auto i : m_data)
		i->save(file);

	file.endWrite();
}
