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

#include <Global.h>
#include <OpenXrRenderer.h>
#include <OpenVrRenderer.h>
#include <HololensRenderer.h>
#include <AsyncTimerFactory.hpp>
#include <StopWatchFactory.hpp>
#include <SearchVisitor.hpp>
#include <Reshape.h>
#include <memory>

using namespace fluo;

std::unordered_map<std::string, flrd::Params> gen_list()
{
	std::unordered_map<std::string, flrd::Params> list;
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
	list.insert(
		std::pair<std::string, flrd::Params>(
			"comp_gen", flrd::Params(names, index, types)));

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
		flrd::Entry::IPT_FLOAT,
		flrd::Entry::IPT_FLOAT,
		flrd::Entry::IPT_FLOAT,
		flrd::Entry::IPT_FLOAT,
		flrd::Entry::IPT_FLOAT,
		flrd::Entry::IPT_FLOAT,
		flrd::Entry::IPT_FLOAT,
		flrd::Entry::IPT_FLOAT,
		flrd::Entry::IPT_FLOAT,
		flrd::Entry::IPT_FLOAT,
		flrd::Entry::IPT_FLOAT,
		flrd::Entry::IPT_BOOL,
		flrd::Entry::IPT_BOOL,
		flrd::Entry::IPT_UINT,
		flrd::Entry::IPT_UINT,
		flrd::Entry::IPT_FLOAT,
		flrd::Entry::IPT_FLOAT,
		flrd::Entry::IPT_BOOL,
		flrd::Entry::IPT_BOOL,
		flrd::Entry::IPT_BOOL,
		flrd::Entry::IPT_BOOL,
		flrd::Entry::IPT_BOOL,
		flrd::Entry::IPT_BOOL,
		flrd::Entry::IPT_BOOL,
		flrd::Entry::IPT_BOOL,
		flrd::Entry::IPT_FLOAT,
	};
	list.insert(
		std::pair<std::string, flrd::Params>(
			"vol_prop", flrd::Params(names, index, types)));

	return list;
}

std::unordered_map<std::string, flrd::Params> flrd::Reshape::params_list_ = gen_list();
flrd::EntryParams* flrd::Reshape::result_ = 0;

Global Global::instance_;

Global::Global() :
	comp_gen_table_enable_(false)
{
	//comp gen
	comp_gen_entry_.setParams(flrd::Reshape::get_params("comp_gen"));
	comp_gen_table_.setParams(flrd::Reshape::get_params("comp_gen"));
	//vol prop
	vol_prop_table_.setParams(flrd::Reshape::get_params("vol_prop"));

	origin_ = ref_ptr<Group>(new Group());
	origin_->setName(gstOrigin);
	BuildFactories();

	help_url_ = "https://github.com/SCIInstitute/fluorender";

	InitLocale();
}

//python
flrd::PyBase* Global::get_add_pybase(const std::string& name)
{
	auto it = python_list_.find(name);
	if (it == python_list_.end())
	{
		flrd::PyBase* pybase = new flrd::PyBase();
		python_list_.insert(std::pair<std::string, flrd::PyBase*>(
			name, pybase));
		return pybase;
	}
	else
		return it->second;
}

flrd::PyDlc* Global::get_add_pydlc(const std::string& name)
{
	auto it = python_list_.find(name);
	if (it == python_list_.end())
	{
		flrd::PyDlc* pydlc = new flrd::PyDlc();
		python_list_.insert(std::pair<std::string, flrd::PyBase*>(
			name, pydlc));
		return pydlc;
	}
	else
		return dynamic_cast<flrd::PyDlc*>(it->second);
}

void Global::clear_python()
{
	for (auto i : python_list_)
	{
		i.second->Exit();
		delete i.second;
	}
	flrd::PyBase::Free();
}

void Global::undo()
{
	//find lastest slider
	double t = 0;
	Undoable* c = 0;
	for (auto i : undo_ctrls_)
	{
		double time = i->GetTimeUndo();
		if (time > t)
		{
			t = time;
			c = i;
		}
	}

	if (!c)
		return;

	double time_span = main_settings_.m_time_span;
	for (auto i : undo_ctrls_)
	{
		double time = i->GetTimeUndo();
		if (std::fabs(time - t) < time_span)
			i->Undo();
	}
}

void Global::redo()
{
	//find earliest slider
	double t = std::numeric_limits<double>::max();
	Undoable* c = 0;
	for (auto i : undo_ctrls_)
	{
		double time = i->GetTimeRedo();
		if (time < t)
		{
			t = time;
			c = i;
		}
	}

	if (!c)
		return;

	double time_span = main_settings_.m_time_span;
	for (auto i : undo_ctrls_)
	{
		double time = i->GetTimeRedo();
		if (std::fabs(time - t) < time_span)
			i->Redo();
	}
}

void Global::apply_processor_settings()
{
	m_ruler_handler.SetBgParams(
		glbin_settings.m_bg_type,
		glbin_settings.m_kx,
		glbin_settings.m_ky,
		glbin_settings.m_varth,
		glbin_settings.m_gauth);
	glbin_script_proc.SetBreak(glbin_settings.m_script_break);
	//glbin_brush_def.Apply(&m_vol_selector);
	glbin_seg_grow.SetRulerHandler(&m_ruler_handler);
}

MainSettings& Global::get_settings()
{
	return main_settings_;
}

BrushDefault& Global::get_brush_def()
{
	return main_settings_.m_brush_def;
}

ComponentDefault& Global::get_comp_def()
{
	return main_settings_.m_comp_def;
}

OutAdjDefault& Global::get_outadj_def()
{
	return main_settings_.m_outadj_def;
}

ViewDefault& Global::get_view_def()
{
	return main_settings_.m_view_def;
}

VolumeDataDefault& Global::get_vol_def()
{
	return main_settings_.m_vol_def;
}

MovieDefault& Global::get_movie_def()
{
	return main_settings_.m_movie_def;
}

ColocalDefault& Global::get_colocal_def()
{
	return main_settings_.m_colocal_def;
}

//states
States& Global::get_states()
{
	return states_;
}

AsyncTimer* Global::getAsyncTimer(const std::string& name)
{
	return glbin_atmf->findFirst(name);
}

StopWatch* Global::getStopWatch(const std::string& name)
{
	return glbin_swhf->findFirst(name);
}

Object* Global::get(const std::string& name, Group* start)
{
	SearchVisitor visitor;
	visitor.matchName(name);
	if (start)
		start->accept(visitor);
	else
		origin_->accept(visitor);
	ObjectList* list = visitor.getResult();
	if (list->empty())
		return 0;
	else
		return (*list)[0];
}

AsyncTimerFactory* Global::getAsyncTimerFactory()
{
	Object* obj = get(gstAsyncTimerFactory);
	if (!obj)
		return 0;
	return dynamic_cast<AsyncTimerFactory*>(obj);
}

StopWatchFactory* Global::getStopWatchFactory()
{
	Object* obj = get(gstStopWatchFactory);
	if (!obj)
		return 0;
	return dynamic_cast<StopWatchFactory*>(obj);
}

void Global::BuildFactories()
{
	AsyncTimerFactory* f1 = new AsyncTimerFactory();
	f1->createDefault();
	origin_->addChild(f1);
	StopWatchFactory* f2 = new StopWatchFactory();
	f2->createDefault();
	origin_->addChild(f2);

}

BaseXrRenderer* Global::get_xr_renderer()
{
	if (m_xr_renderer)
		return m_xr_renderer.get();
	//create
	switch (glbin_settings.m_xr_api)
	{
	case 0://disabled
	default:
		return 0;
	case 1://openxr
		m_xr_renderer = std::make_unique<OpenXrRenderer>();
		return m_xr_renderer.get();
	case 2://openvr
		m_xr_renderer = std::make_unique<OpenVrRenderer>();
		return m_xr_renderer.get();
	case 3://wmr
		m_xr_renderer = std::make_unique<WmrRenderer>();
		return m_xr_renderer.get();
	case 4://hololens
		{
			HololensOptions options;
			options.host = split_host_name_and_port(glbin_settings.m_holo_ip, options.port);
			m_xr_renderer = std::make_unique<HololensRenderer>(options);
		}
		return m_xr_renderer.get();
	}
	return 0;
}

//jvm
JVMInitializer* Global::get_jvm_instance()
{
	if (m_pJVMInstance)
		return m_pJVMInstance.get();
	// Adding JVm initialization.
	m_pJVMInstance = std::make_unique<JVMInitializer>(glbin_settings.GetJvmArgs());
	return m_pJVMInstance.get();
}

//locale
void Global::InitLocale()
{
	std::setlocale(LC_ALL, "");
}