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
#ifndef GLOBAL_H
#define GLOBAL_H

#include <Names.h>
#include <Group.hpp>
#include <MainSettings.h>
#include <States.h>
#include <VolCache.h>
#include <Params.h>
#include <EntryParams.h>
#include <TableHistParams.h>
#include <Undoable.h>
#include <QVideoEncoder.h>
#include <PyBase.h>
#include <PyDlc.h>
#include <CompGenerator.h>
#include <CompSelector.h>
#include <CompEditor.h>
#include <VolumeSelector.h>
#include <CompAnalyzer.h>
#include <VolumeCalculator.h>
#include <KernelExecutor.h>
#include <ScriptProc.h>
#include <RulerAlign.h>
#include <TrackMap.h>
#include <RulerHandler.h>
#include <RulerRenderer.h>
#include <VolumePoint.h>
#include <SegGrow.h>
#include <DistCalculator.h>
#include <Colocalize.h>
#include <Interpolator.h>
#include <MovieMaker.h>
#include <DataManager.h>
#include <VolKernel.h>
#include <Framebuffer.h>
#include <VertexArray.h>
#include <VolShader.h>
#include <SegShader.h>
#include <VolCalShader.h>
#include <ImgShader.h>
#include <LightFieldShader.h>
#include <TextRenderer.h>
#include <LookingGlassRenderer.h>
#include <Clusterizer.h>
#include <VolumeMeshConv.h>
#include <Project.h>
#include <BaseXrRenderer.h>
#include <JVMInitializer.h>
#include <TreeFileFactory.h>

#define glbin fluo::Global::instance()
#define glbin_cache_queue fluo::Global::instance().get_cache_queue()
#define glbin_reg_cache_queue_func(obj, read_func, del_func) \
	fluo::Global::instance().get_cache_queue().\
	RegisterCacheQueueFuncs(\
	std::bind(&read_func, obj, std::placeholders::_1),\
	std::bind(&del_func, obj, std::placeholders::_1))
//config file handlers
#define glbin_tree_file_factory fluo::Global::instance().get_tree_file_factory()
//settings
#define glbin_settings fluo::Global::instance().get_settings()
#define glbin_brush_def fluo::Global::instance().get_brush_def()
#define glbin_comp_def fluo::Global::instance().get_comp_def()
#define glbin_outadj_def fluo::Global::instance().get_outadj_def()
#define glbin_view_def fluo::Global::instance().get_view_def()
#define glbin_vol_def fluo::Global::instance().get_vol_def()
#define glbin_mov_def fluo::Global::instance().get_movie_def()
#define glbin_colocal_def fluo::Global::instance().get_colocal_def()
//states
#define glbin_states fluo::Global::instance().get_states()
//processors
#define glbin_comp_generator fluo::Global::instance().get_comp_generator()
#define glbin_comp_analyzer fluo::Global::instance().get_comp_analyzer()
#define glbin_comp_selector fluo::Global::instance().get_comp_selector()
#define glbin_comp_editor fluo::Global::instance().get_comp_editor()
#define glbin_vol_selector fluo::Global::instance().get_vol_selector()
#define glbin_vol_calculator fluo::Global::instance().get_vol_calculator()
#define glbin_kernel_executor fluo::Global::instance().get_kernel_executor()
#define glbin_script_proc fluo::Global::instance().get_script_proc()
#define glbin_aligner fluo::Global::instance().get_aligner()
#define glbin_trackmap_proc fluo::Global::instance().get_trackmap_proc()
#define glbin_ruler_handler fluo::Global::instance().get_ruler_handler()
#define glbin_ruler_renderer fluo::Global::instance().get_ruler_renderer()
#define glbin_volume_point fluo::Global::instance().get_volume_point()
#define glbin_seg_grow fluo::Global::instance().get_seg_grow()
#define glbin_dist_calculator fluo::Global::instance().get_dist_calculator()
#define glbin_interpolator fluo::Global::instance().get_interpolator()
#define glbin_moviemaker fluo::Global::instance().get_movie_maker()
#define glbin_data_manager fluo::Global::instance().get_data_manager()
#define glbin_lg_renderer fluo::Global::instance().get_looking_glass_renderer()
#define glbin_xr_renderer fluo::Global::instance().get_xr_renderer()
#define glbin_colocalizer fluo::Global::instance().get_colocalizer()
#define glbin_clusterizer fluo::Global::instance().get_clusterizer()
#define glbin_vol_converter fluo::Global::instance().get_vol_converter()
//graphics resources
#define glbin_vol_kernel_factory fluo::Global::instance().get_vol_kernel_factory()
#define glbin_framebuffer_manager fluo::Global::instance().get_framebuffer_manager()
#define glbin_vertex_array_manager fluo::Global::instance().get_vertex_array_manager()
#define glbin_vol_shader_factory fluo::Global::instance().get_vol_shader_factory()
#define glbin_seg_shader_factory fluo::Global::instance().get_seg_shader_factory()
#define glbin_vol_cal_shader_factory fluo::Global::instance().get_vol_cal_shader_factory()
#define glbin_img_shader_factory fluo::Global::instance().get_img_shader_factory()
#define glbin_light_field_shader_factory fluo::Global::instance().get_light_field_shader_factory()
#define glbin_text_tex_manager fluo::Global::instance().get_text_tex_manager()

//time
#define glbin_atmf fluo::Global::instance().getAsyncTimerFactory()
#define glbin_swhf fluo::Global::instance().getStopWatchFactory()

//current selection
#define glbin_current fluo::Global::instance().get_current_objects()
//project
#define glbin_project fluo::Global::instance().get_project()

//help url
#define glbin_help_url fluo::Global::instance().get_help_url()

//jvm
#define glbin_jvm_instance fluo::Global::instance().get_jvm_instance()

//ml output size
#define glbin_vp_size 26

namespace fluo
{
	class AsyncTimer;
	class StopWatch;
	class AsyncTimerFactory;
	class StopWatchFactory;
	class Global
	{
	public:
		static Global& instance() { return instance_; }

		flrd::CacheQueue& get_cache_queue() { return cache_queue_; }

		//config file handlers
		TreeFileFactory& get_tree_file_factory() { return tree_file_factory_; }

		//video encoder
		QVideoEncoder& get_video_encoder() { return encoder_; }

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
		MovieDefault& get_movie_def();
		ColocalDefault& get_colocal_def();

		//states
		States& get_states();

		//data processors
		void apply_processor_settings();
		flrd::ComponentGenerator& get_comp_generator() { return m_comp_generator; }
		flrd::ComponentAnalyzer& get_comp_analyzer() { return m_comp_analyzer; }
		flrd::ComponentSelector& get_comp_selector() { return m_comp_selector; }
		flrd::ComponentEditor& get_comp_editor() { return m_comp_editor; }
		flrd::VolumeSelector& get_vol_selector() { return m_vol_selector; }
		flrd::VolumeCalculator& get_vol_calculator() { return m_vol_calculator; }
		KernelExecutor& get_kernel_executor() { return m_kernel_executor; }
		flrd::ScriptProc& get_script_proc() { return m_script_proc; }
		flrd::RulerAlign& get_aligner() { return m_aligner; }
		flrd::TrackMapProcessor& get_trackmap_proc() { return m_trackmap_proc; }
		flrd::RulerHandler& get_ruler_handler() { return m_ruler_handler; }
		flrd::RulerRenderer& get_ruler_renderer() { return m_ruler_renderer; }
		flrd::VolumePoint& get_volume_point() { return m_volume_point; }
		flrd::SegGrow& get_seg_grow() { return m_seg_grow; }
		flrd::DistCalculator& get_dist_calculator() { return m_dist_calculator; }
		Interpolator& get_interpolator() { return m_interpolator; }
		MovieMaker& get_movie_maker() { return m_movie_maker; }
		DataManager& get_data_manager() { return m_data_manager; }
		LookingGlassRenderer& get_looking_glass_renderer() { return m_lg_renderer; }
		flrd::Colocalize& get_colocalizer() { return m_colocalizer; }
		flrd::Clusterizer& get_clusterizer() { return m_clusterizer; }
		flrd::VolumeMeshConv& get_vol_converter() { return m_vol_converter; }

		//xr renderer
		BaseXrRenderer* get_xr_renderer();

		//jvm
		JVMInitializer* get_jvm_instance();

		//graphics resources
		flvr::VolKernelFactory& get_vol_kernel_factory() { return vol_kernel_factory_; }
		flvr::FramebufferManager& get_framebuffer_manager() { return framebuffer_manager_; }
		flvr::VertexArrayManager& get_vertex_array_manager() { return vertex_array_manager_; }
		flvr::VolShaderFactory& get_vol_shader_factory() { return vol_shader_factory_; }
		flvr::SegShaderFactory& get_seg_shader_factory() { return seg_shader_factory_; }
		flvr::VolCalShaderFactory& get_vol_cal_shader_factory() { return cal_shader_factory_; }
		flvr::ImgShaderFactory& get_img_shader_factory() { return img_shader_factory_; }
		flvr::LightFieldShaderFactory& get_light_field_shader_factory() { return ligh_field_shader_factory_; }
		flvr::TextTextureManager& get_text_tex_manager() { return text_texture_manager_; }

		//time
		Object* get(const std::string& name, Group* start = nullptr);
		AsyncTimer* getAsyncTimer(const std::string& name);
		StopWatch* getStopWatch(const std::string& name);
		AsyncTimerFactory* getAsyncTimerFactory();
		StopWatchFactory* getStopWatchFactory();

		//current selection
		CurrentObjects& get_current_objects() { return current_objects_; }

		//project
		Project& get_project() { return project_; }

		std::string& get_help_url() { return help_url_; }

	private:
		static Global instance_;

		//config file handlers
		TreeFileFactory tree_file_factory_;

		flrd::CacheQueue cache_queue_;

		//video encoder
		QVideoEncoder encoder_;

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

		//states
		States states_;

		//the data processors
		flrd::ComponentGenerator m_comp_generator;
		flrd::ComponentAnalyzer m_comp_analyzer;
		flrd::ComponentSelector m_comp_selector;
		flrd::ComponentEditor m_comp_editor;
		flrd::VolumeSelector m_vol_selector;
		flrd::VolumeCalculator m_vol_calculator;
		KernelExecutor m_kernel_executor;
		flrd::ScriptProc m_script_proc;
		flrd::RulerAlign m_aligner;
		flrd::TrackMapProcessor m_trackmap_proc;
		flrd::RulerHandler m_ruler_handler;
		flrd::RulerRenderer m_ruler_renderer;
		flrd::VolumePoint m_volume_point;
		flrd::SegGrow m_seg_grow;
		flrd::DistCalculator m_dist_calculator;
		Interpolator m_interpolator;
		MovieMaker m_movie_maker;
		DataManager m_data_manager;
		LookingGlassRenderer m_lg_renderer;
		flrd::Colocalize m_colocalizer;
		flrd::Clusterizer m_clusterizer;
		flrd::VolumeMeshConv m_vol_converter;

		//xr renderer
		std::unique_ptr<BaseXrRenderer> m_xr_renderer = nullptr;

		//jvm
		std::unique_ptr<JVMInitializer> m_pJVMInstance = nullptr;

		//graphics resources
		//kernel for calculation
		flvr::VolKernelFactory vol_kernel_factory_;
		//framebuffers for everything
		flvr::FramebufferManager framebuffer_manager_;
		//vertex arrays
		flvr::VertexArrayManager vertex_array_manager_;
		//sahder for volume rendering
		flvr::VolShaderFactory vol_shader_factory_;
		//shader for segmentation
		flvr::SegShaderFactory seg_shader_factory_;
		//shader for calculation
		flvr::VolCalShaderFactory cal_shader_factory_;
		//smooth filter
		flvr::ImgShaderFactory img_shader_factory_;
		//for looking glass
		flvr::LightFieldShaderFactory ligh_field_shader_factory_;
		//text texture
		flvr::TextTextureManager text_texture_manager_;

		//time
		ref_ptr<Group> origin_;//the root of everything else

		//current selection
		CurrentObjects current_objects_;

		//project management
		Project project_;

		//help url
		std::string help_url_;

	private:
		Global();
		void BuildFactories();

		void InitLocale();
	};

}
#endif
