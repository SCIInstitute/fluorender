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

#include <Global.h>
#include <AsyncTimerFactory.hpp>
#include <StopWatchFactory.hpp>
#include <SearchVisitor.hpp>
#include <Reshape.h>

using namespace fluo;

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