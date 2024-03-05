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
#ifndef GLOBAL_H
#define GLOBAL_H

#include <Names.h>
#include <MainSettings.h>
#include <Tracking/VolCache.h>
#include <Database/Params.h>
#include <Database/EntryParams.h>
#include <Database/TableHistParams.h>
#include <Undoable.h>
#include <QVideoEncoder.h>
#include <Python/PyBase.h>
#include <Python/PyDlc.h>
#include <CompGenerator.h>
#include <CompSelector.h>
#include <VolumeSelector.h>
#include <CompAnalyzer.h>
#include <VolumeCalculator.h>
#include <KernelExecutor.h>
#include <ScriptProc.h>
#include <RulerAlign.h>

#define glbin fluo::Global::instance()
#define glbin_cache_queue fluo::Global::instance().get_cache_queue()
#define glbin_reg_cache_queue_func(obj, read_func, del_func) \
	fluo::Global::instance().get_cache_queue().\
	RegisterCacheQueueFuncs(\
	std::bind(&read_func, obj, std::placeholders::_1),\
	std::bind(&del_func, obj, std::placeholders::_1))
//settings
#define glbin_settings fluo::Global::instance().get_settings()
#define glbin_brush_def fluo::Global::instance().get_brush_def()
#define glbin_comp_def fluo::Global::instance().get_comp_def()
#define glbin_outadj_def fluo::Global::instance().get_outadj_def()
#define glbin_view_def fluo::Global::instance().get_view_def()
#define glbin_vol_def fluo::Global::instance().get_vol_def()
//processors
#define glbin_comp_generator fluo::Global::instance().get_comp_generator()
#define glbin_comp_analyzer fluo::Global::instance().get_comp_analyzer()
#define glbin_comp_selector fluo::Global::instance().get_comp_selector()
#define glbin_vol_selector fluo::Global::instance().get_vol_selector()
#define glbin_vol_calculator fluo::Global::instance().get_vol_calculator()
#define glbin_kernel_executor fluo::Global::instance().get_kernel_executor()
#define glbin_script_proc fluo::Global::instance().get_script_proc()
#define glbin_aligner fluo::Global::instance().get_aligner()

namespace fluo
{
	class Global
	{
	public:
		static Global& instance() { return instance_; }

		flrd::CacheQueue& get_cache_queue() { return cache_queue_; }

		//video encoder
		QVideoEncoder& get_video_encoder() { return encoder_; }

		//learning
		void gen_params_list();
		flrd::Params* get_params(const std::string& name)
		{
			auto it = params_list_.find(name);
			if (it != params_list_.end())
				return &(it->second);
			return nullptr;
		}
		//comp gen
		void set_cg_table_enable(bool value) { comp_gen_table_enable_ = value; }
		bool get_cg_table_enable() { return comp_gen_table_enable_; }
		flrd::EntryParams& get_cg_entry() { return comp_gen_entry_; }
		flrd::TableHistParams& get_cg_table() { return comp_gen_table_; }
		//vol prop
		void set_vp_table_enable(bool value) { vol_prop_table_enable_ = value; }
		bool get_vp_table_enable() { return vol_prop_table_enable_; }
		flrd::TableHistParams& get_vp_table() { return vol_prop_table_; }

		//python
		template <class T>
		T* get_add_python(const std::string& name)
		{
			auto it = python_list_.find(name);
			if (it == python_list_.end())
			{
				T* py = new T;
				size_t n = python_list_.size();
				python_list_.insert(std::pair<std::string, flrd::PyBase*>(name, py));
				if (python_list_.size() > n)
					return py;
				else
					return nullptr;
			}
			else
				return dynamic_cast<T*>(it->second);
		}
		flrd::PyBase* get_add_pybase(const std::string& name);
		flrd::PyDlc* get_add_pydlc(const std::string& name);
		void clear_python();

		//undo sliders
		void add_undo_control(Undoable* control) { undo_ctrls_.push_back(control); }
		void del_undo_control(Undoable* control)
		{
			auto it = std::find(undo_ctrls_.begin(), undo_ctrls_.end(), control);
			if (it != undo_ctrls_.end())
				undo_ctrls_.erase(it);
		}
		void undo();
		void redo();

		//default volume data settings
		MainSettings& get_settings();
		BrushDefault& get_brush_def();
		ComponentDefault& get_comp_def();
		OutAdjDefault& get_outadj_def();
		ViewDefault& get_view_def();
		VolumeDataDefault& get_vol_def();

		//data processors
		flrd::ComponentGenerator& get_comp_generator() { return m_comp_generator; }
		flrd::ComponentAnalyzer& get_comp_analyzer() { return m_comp_analyzer; }
		flrd::ComponentSelector& get_comp_selector() { return m_comp_selector; }
		flrd::VolumeSelector& get_vol_selector() { return m_vol_selector; }
		flrd::VolumeCalculator& get_vol_calculator() { return m_vol_calculator; }
		KernelExecutor& get_kernel_executor() { return m_kernel_executor; }
		flrd::ScriptProc& get_script_proc() { return m_script_proc; }
		flrd::RulerAlign& get_aligner() { return m_aligner; }

	private:
		Global();
		static Global instance_;

		flrd::CacheQueue cache_queue_;

		//video encoder
		QVideoEncoder encoder_;

		//machine learning
		std::unordered_map<std::string, flrd::Params> params_list_;//available params
		//comp gen
		bool comp_gen_table_enable_;//add records from ui
		flrd::EntryParams comp_gen_entry_;//temporary entry to save cg params
		flrd::TableHistParams comp_gen_table_;//records for learning comp generation settings
		//vol prop
		bool vol_prop_table_enable_;//add records for vol prop
		flrd::TableHistParams vol_prop_table_;//records for learning vol props

		//python
		std::unordered_map<std::string, flrd::PyBase*> python_list_;

		//controls for undo and redo
		std::vector<Undoable*> undo_ctrls_;

		//settings
		MainSettings main_settings_;

		//the data processors
		flrd::ComponentGenerator m_comp_generator;
		flrd::ComponentAnalyzer m_comp_analyzer;
		flrd::ComponentSelector m_comp_selector;
		flrd::VolumeSelector m_vol_selector;
		flrd::VolumeCalculator m_vol_calculator;
		KernelExecutor m_kernel_executor;
		flrd::ScriptProc m_script_proc;
		flrd::RulerAlign m_aligner;
	};

}
#endif
