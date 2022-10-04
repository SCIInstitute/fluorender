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
#include "EntryParams.h"
#include <Table.h>
#include <FileIO/File.h>

using namespace flrd;

unsigned int EntryParams::m_size = 18;
std::vector<std::string> EntryParams::m_names = {
	"iter",//uint
	"thresh",//float
	"diff",//bool
	"falloff",//float
	"density",//bool
	"density_thresh",//float
	"varth",//float
	"density_window_size",//uint
	"density_stats_size",//uint
	"use_dist_field",//bool
	"dist_strength",//float
	"dist_thresh",//float
	"dist_filter_size",//uint
	"max_dist",//uint
	"cleanb",//bool
	"clean_iter",//uint
	"clean_size_vl",//uint
	"grow_fixed"//bool
	};
std::unordered_map<std::string, size_t> EntryParams::m_name_index = {
	{"iter", 0},
	{"thresh", 1},
	{"diff", 2},
	{"falloff", 3},
	{"density", 4},
	{"density_thresh", 5},
	{"varth", 6},
	{"density_window_size", 7},
	{"density_stats_size", 8},
	{"use_dist_field", 9},
	{"dist_strength", 10},
	{"dist_thresh", 11},
	{"dist_filter_size", 12},
	{"max_dist", 13},
	{"cleanb", 14},
	{"clean_iter", 15},
	{"clean_size_vl", 16},
	{"grow_fixed", 17}
};
std::vector<EntryParams::ParamTypes> EntryParams::m_types = {
	IPT_UINT,
	IPT_FLOAT,
	IPT_BOOL,
	IPT_FLOAT,
	IPT_BOOL,
	IPT_FLOAT,
	IPT_FLOAT,
	IPT_UINT,
	IPT_UINT,
	IPT_BOOL,
	IPT_FLOAT,
	IPT_FLOAT,
	IPT_UINT,
	IPT_UINT,
	IPT_BOOL,
	IPT_UINT,
	IPT_UINT,
	IPT_BOOL
	};

EntryParams::EntryParams()
{
	//fixed size
	if (m_size)
		m_data.assign(m_size, 0);
	m_valid = false;
}

EntryParams::EntryParams(const EntryParams& ent) :
	Entry(ent)
{
	m_valid = ent.m_valid;
}

EntryParams::~EntryParams()
{
}

size_t EntryParams::getNameIndex(const std::string& name)
{
	size_t result = m_size;
	std::unordered_map<std::string, size_t>::const_iterator i =
		m_name_index.find(name);
	if (i != m_name_index.end())
	{
		result = i->second;
	}
	return result;
}

void EntryParams::open(File& file)
{
	//size
	if (file.check(TAG_ENT_SIZE))
	{
		unsigned int size;
		file.readValue(size);
		if (size != m_size) return;
	}

	//data
	if (file.check(TAG_ENT_DATA))
		file.readVector(m_data);
}

void EntryParams::save(File& file)
{
	//type
	file.writeValue(Table::TAG_TABLE_ENT_PARAMS);

	//size
	file.writeValue(TAG_ENT_SIZE);
	file.writeValue(m_size);

	//data
	file.writeValue(TAG_ENT_DATA);
	file.writeVector(m_data);
}
