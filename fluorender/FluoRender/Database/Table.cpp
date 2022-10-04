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
#include <filesystem>

using namespace flrd;

Table::Table():
	m_recnum(0),
	m_modified(false)
{
	m_modify_time = m_create_time = std::time(0);
}

Table::Table(const Table& table) :
	m_modified(true),
	m_notes(table.m_notes),
	m_recnum(table.m_recnum)
{
	m_name = table.m_name + "_copy";
	m_modify_time = m_create_time = std::time(0);
	for (auto i : table.m_data)
	{
		Record* rec = 0;
		RecordHistParams* temp = i->asRecordHistParams();
		if (temp)
			rec = new RecordHistParams(*temp);
		if (rec)
			m_data.push_back(rec);
	}
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
	//make sure uniquness
	for (auto i : m_data)
	{
		if (i->compare(rec))
			return;
	}
	m_data.push_back(rec);
	m_modify_time = std::time(0);
	setModified();
}

void Table::readRecord(Record* rec)
{
	if (!rec)
		return;
	m_data.push_back(rec);
	m_modify_time = std::time(0);
}

void Table::delRecord(size_t i)
{
	if (i < m_data.size())
	{
		m_data.erase(m_data.begin() + i);
		setModified();
	}
}

void Table::setCreateTime(const std::time_t& t)
{
	m_create_time = t;
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

	//name
	if (file.check(TAG_TABLE_NAME))
	{
		size_t ns = 0;
		if (file.check(TAG_TABLE_NAME_SIZE))
			file.readValue(ns);
		if (ns)
			m_name = file.readString(ns);
	}
	if (m_name.empty())
	{
		const std::filesystem::path p(filename);
		m_name = p.stem().string();
	}
	//time of creation
	if (file.check(TAG_TABLE_TIME_CREATE))
		file.readValue(m_create_time);
	//time of modification
	if (file.check(TAG_TABLE_TIME_MODIFY))
		file.readValue(m_modify_time);
	//notes
	if (file.check(TAG_TABLE_NOTES))
	{
		size_t ns = 0;
		if (file.check(TAG_TABLE_NOTE_SIZE))
			file.readValue(ns);
		if (ns)
			m_notes = file.readString(ns);
	}

	//rec num
	if (file.check(TAG_TABLE_REC_NUM))
	{
		file.readValue(m_recnum);
	}
	else
	{
		m_recnum = 0;
		file.endRead();
		return;
	}

	//data
	if (!m_data.empty())
		clear();
	TableTags t;
	file.getPos();
	for (size_t i = 0; i < m_recnum; ++i)
	{
		file.readValue(t);
		Record* rec = 0;
		switch (t)
		{
		case TAG_TABLE_REC_HISTPARAM:
			rec = new RecordHistParams();
			break;
		default:
			file.setPos();
			break;
		}
		if (rec)
		{
			rec->open(file);
			readRecord(rec);
		}
	}

	file.endRead();
	m_modified = false;
}

void Table::openinfo(const std::string& filename)
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

	//name
	if (file.check(TAG_TABLE_NAME))
	{
		size_t ns = 0;
		if (file.check(TAG_TABLE_NAME_SIZE))
			file.readValue(ns);
		if (ns)
			m_name = file.readString(ns);
	}
	if (m_name.empty())
	{
		const std::filesystem::path p(filename);
		m_name = p.stem().string();
	}
	//time of creation
	if (file.check(TAG_TABLE_TIME_CREATE))
		file.readValue(m_create_time);
	//time of modification
	if (file.check(TAG_TABLE_TIME_MODIFY))
		file.readValue(m_modify_time);
	//notes
	if (file.check(TAG_TABLE_NOTES))
	{
		size_t ns = 0;
		if (file.check(TAG_TABLE_NOTE_SIZE))
			file.readValue(ns);
		if (ns)
			m_notes = file.readString(ns);
	}

	//rec num
	if (file.check(TAG_TABLE_REC_NUM))
	{
		file.readValue(m_recnum);
	}

	file.endRead();
}

void Table::save(const std::string& filename)
{
	flrd::File file;
	file.beginWrite(filename);

	//header
	file.writeString("FluoRender table");

	//name
	file.writeValue(TAG_TABLE_NAME);
	file.writeValue(TAG_TABLE_NAME_SIZE);
	file.writeValue(m_name.size());
	file.writeString(m_name);
	//time of creation
	file.writeValue(TAG_TABLE_TIME_CREATE);
	file.writeValue(m_create_time);
	//time of modification
	file.writeValue(TAG_TABLE_TIME_MODIFY);
	file.writeValue(m_modify_time);
	//notes
	file.writeValue(TAG_TABLE_NOTES);
	file.writeValue(TAG_TABLE_NOTE_SIZE);
	file.writeValue(m_notes.size());
	file.writeString(m_notes);

	//rec num
	file.writeValue(TAG_TABLE_REC_NUM);
	file.writeValue(m_data.size());

	//data
	for (auto i : m_data)
		i->save(file);

	file.endWrite();
	m_modified = false;
}
