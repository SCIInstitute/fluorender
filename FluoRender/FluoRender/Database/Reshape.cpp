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
#include <Reshape.h>

using namespace flrd;

std::unordered_map<std::string, Params> gen_list()
{
	std::unordered_map<std::string, Params> list;
	std::vector<std::string> names;
	std::unordered_map<std::string, size_t> index;
	std::vector<Entry::ParamTypes> types;
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
		Entry::IPT_UINT,
		Entry::IPT_FLOAT,
		Entry::IPT_BOOL,
		Entry::IPT_FLOAT,
		Entry::IPT_BOOL,
		Entry::IPT_FLOAT,
		Entry::IPT_FLOAT,
		Entry::IPT_UINT,
		Entry::IPT_UINT,
		Entry::IPT_BOOL,
		Entry::IPT_FLOAT,
		Entry::IPT_FLOAT,
		Entry::IPT_UINT,
		Entry::IPT_UINT,
		Entry::IPT_BOOL,
		Entry::IPT_UINT,
		Entry::IPT_UINT,
		Entry::IPT_BOOL
	};
	list.insert(
		std::pair<std::string, Params>(
			"comp_gen", Params(names, index, types)));

	//for volume property
	names =
	{
		"extract_boundary",//float
		"gamma3d",//float
		"low_offset",//float
		"high_offset",//float
		"low_threshold",//float
		"high_threshold",//float
		"low_shading",//float
		"high_shading",//float
		"alpha",//float
		"sample_rate",//float
		"luminance",//float
		"colormap_enable",//bool
		"colormap_inv",//bool
		"colormap_type",//uint
		"colormap_proj",//uint
		"colormap_low",//float
		"colormap_hi",//float
		"alpha_enable",//bool
		"shading_enable",//bool
		"interp_enable",//bool
		"invert_enable",//bool
		"mip_enable",//bool
		"transparent_enable",//bool
		"denoise_enable",//bool
		"shadow_enable",//bool
		"shadow_intensity"//float
	};
	index =
	{
		{"extract_boundary", 0},
		{"gamma3d", 1},
		{"low_offset", 2},
		{"high_offset", 3},
		{"low_threshold", 4},
		{"high_threshold", 5},
		{"low_shading", 6},
		{"high_shading", 7},
		{"alpha", 8},
		{"sample_rate", 9},
		{"luminance", 10},
		{"colormap_enable", 11},
		{"colormap_inv", 12},
		{"colormap_type", 13},
		{"colormap_proj", 14},
		{"colormap_low", 15},
		{"colormap_hi", 16},
		{"alpha_enable", 17},
		{"shading_enable", 18},
		{"interp_enable", 19},
		{"invert_enable", 20},
		{"mip_enable", 21},
		{"transparent_enable", 22},
		{"denoise_enable", 23},
		{"shadow_enable", 24},
		{"shadow_intensity", 25}
	};
	types =
	{
		Entry::IPT_FLOAT,
		Entry::IPT_FLOAT,
		Entry::IPT_FLOAT,
		Entry::IPT_FLOAT,
		Entry::IPT_FLOAT,
		Entry::IPT_FLOAT,
		Entry::IPT_FLOAT,
		Entry::IPT_FLOAT,
		Entry::IPT_FLOAT,
		Entry::IPT_FLOAT,
		Entry::IPT_FLOAT,
		Entry::IPT_BOOL,
		Entry::IPT_BOOL,
		Entry::IPT_UINT,
		Entry::IPT_UINT,
		Entry::IPT_FLOAT,
		Entry::IPT_FLOAT,
		Entry::IPT_BOOL,
		Entry::IPT_BOOL,
		Entry::IPT_BOOL,
		Entry::IPT_BOOL,
		Entry::IPT_BOOL,
		Entry::IPT_BOOL,
		Entry::IPT_BOOL,
		Entry::IPT_BOOL,
		Entry::IPT_FLOAT,
	};
	list.insert(
		std::pair<std::string, Params>(
			"vol_prop", Params(names, index, types)));

	return list;
}

std::unordered_map<std::string, Params> Reshape::params_list_ = gen_list();
EntryParams* Reshape::result_ = 0;

Params* Reshape::get_params(const std::string& name)
{
	auto it = params_list_.find(name);
	if (it == params_list_.end())
		return 0;
	return &(it->second);
}

EntryParams* Reshape::get_entry_params(const std::string& name, float* val)
{
	auto it = params_list_.find(name);
	if (it == params_list_.end())
		return 0;
	Params* p = &(it->second);
	result_ = new EntryParams();
	std::string str;

	result_->setParams(p);
	for (size_t i = 0; i < p->size(); ++i)
	{
		str = p->getName(i);
		switch (p->getType(i))
		{
		case Entry::IPT_BOOL:
			result_->setParam(str, bool(val[i] > 0.5));
			break;
		case Entry::IPT_CHAR:
			result_->setParam(str, char(std::round(val[i])));
			break;
		case Entry::IPT_UCHAR:
			result_->setParam(str, (unsigned char)(std::round(val[i])));
			break;
		case Entry::IPT_SHORT:
			result_->setParam(str, short(std::round(val[i])));
			break;
		case Entry::IPT_USHORT:
			result_->setParam(str, (unsigned short)(std::round(val[i])));
			break;
		case Entry::IPT_INT:
			result_->setParam(str, int(std::round(val[i])));
			break;
		case Entry::IPT_UINT:
			result_->setParam(str, (unsigned int)(std::round(val[i])));
			break;
		case Entry::IPT_FLOAT:
			result_->setParam(str, val[i]);
			break;
		case Entry::IPT_DOUBLE:
			result_->setParam(str, double(val[i]));
			break;
		}
	}

	return result_;
}

size_t Reshape::get_param_size(const std::string& name)
{
	auto it = params_list_.find(name);
	if (it == params_list_.end())
		return 0;
	Params* p = &(it->second);
	return p->size();
}

void Reshape::clear()
{
	if (result_)
	{
		delete result_;
		result_ = 0;
	}
}