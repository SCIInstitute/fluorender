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

#include <Global.h>

using namespace fluo;

Global Global::instance_;

Global::Global() :
	comp_gen_table_enable_(false)
{
	gen_params_list();
	comp_gen_entry_.setParams(get_params("comp_gen"));
}

void Global::gen_params_list()
{
	std::vector<std::string> names;
	std::unordered_map<std::string, size_t> index;
	std::vector<flrd::Entry::ParamTypes> types;
	//for comp_gen
	names =
	{
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
	index =
	{
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
	types =
	{
		flrd::Entry::IPT_UINT,
		flrd::Entry::IPT_FLOAT,
		flrd::Entry::IPT_BOOL,
		flrd::Entry::IPT_FLOAT,
		flrd::Entry::IPT_BOOL,
		flrd::Entry::IPT_FLOAT,
		flrd::Entry::IPT_FLOAT,
		flrd::Entry::IPT_UINT,
		flrd::Entry::IPT_UINT,
		flrd::Entry::IPT_BOOL,
		flrd::Entry::IPT_FLOAT,
		flrd::Entry::IPT_FLOAT,
		flrd::Entry::IPT_UINT,
		flrd::Entry::IPT_UINT,
		flrd::Entry::IPT_BOOL,
		flrd::Entry::IPT_UINT,
		flrd::Entry::IPT_UINT,
		flrd::Entry::IPT_BOOL
	};
	params_list_.insert(
		std::pair<std::string, flrd::Params>(
		"comp_gen", flrd::Params(names, index, types)));
}