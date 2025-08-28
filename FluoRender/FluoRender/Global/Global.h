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

#include <string>
#include <memory>
#include <unordered_map>
#include <functional>

class TreeFileFactory;
class VideoEncoder;
class Undoable;
class MainSettings;
class AutomateDefault;
class BrushDefault;
class ComponentDefault;
class OutAdjDefault;
class ViewDefault;
class VolumeDataDefault;
class MovieDefault;
class ColocalDefault;
class GlobalStates;
class KernelExecutor;
class Interpolator;
class MovieMaker;
class DataManager;
class VolumeLoader;
class LookingGlassRenderer;
class BaseXrRenderer;
class JVMInitializer;
struct CurrentObjects;
class Project;
class RenderView;
namespace fluo
{
	class AsyncTimer;
	class StopWatch;
	class AsyncTimerFactory;
	class StopWatchFactory;
}
namespace flrd
{
	class EntryParams;
	class TableHistParams;
	class PyBase;
	class PyDlc;
	class ComponentGenerator;
	class ComponentAnalyzer;
	class ComponentSelector;
	class ComponentEditor;
	class VolumeSelector;
	class VolumeCalculator;
	class ScriptProc;
	class RulerAlign;
	class TrackMapProcessor;
	class RulerHandler;
	class RulerRenderer;
	class VolumePoint;
	class SegGrow;
	class DistCalculator;
	class Colocalize;
	class Clusterizer;
	class BaseConvVolMesh;
}
namespace flvr
{
	class VolKernelFactory;
	class FramebufferManager;
	class VertexArrayManager;
	class VolShaderFactory;
	class SegShaderFactory;
	class VolCalShaderFactory;
	class ImgShaderFactory;
	class LightFieldShaderFactory;
	class TextTextureManager;
	class MshShaderFactory;
}

#define glbin fluo::Global::instance()
#define glbin_cache_queue fluo::Global::instance().get_cache_queue()
//config file handlers
#define glbin_tree_file_factory fluo::Global::instance().get_tree_file_factory()
//settings
#define glbin_settings fluo::Global::instance().get_settings()
#define glbin_automate_def fluo::Global::instance().get_automate_def()
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
#define glbin_vol_loader fluo::Global::instance().get_vol_loader()
#define glbin_lg_renderer fluo::Global::instance().get_looking_glass_renderer()
#define glbin_xr_renderer fluo::Global::instance().get_xr_renderer()
#define glbin_colocalizer fluo::Global::instance().get_colocalizer()
#define glbin_clusterizer fluo::Global::instance().get_clusterizer()
#define glbin_conv_vol_mesh fluo::Global::instance().get_conv_vol_mesh()
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
#define glbin_msh_shader_factory fluo::Global::instance().get_msh_shader_factory()

//time
#define glbin_atmf fluo::Global::instance().getAsyncTimerFactory()
#define glbin_swhf fluo::Global::instance().getStopWatchFactory()

//current selection
#define glbin_current fluo::Global::instance().get_current_objects()
//project
#define glbin_project fluo::Global::instance().get_project()

//help url
#define glbin_help_url fluo::Global::instance().get_help_url()

//linked rotation
#define glbin_linked_rot fluo::Global::instance().get_linked_rot()
#define glbin_master_linked_view fluo::Global::instance().get_master_linked_view()

//jvm
#define glbin_jvm_instance fluo::Global::instance().get_jvm_instance()

//video encoder
#define glbin_video_encoder fluo::Global::instance().get_video_encoder()

//ml output size
#define glbin_vp_size 26

namespace fluo
{
	class Global
	{
	public:
		static Global& instance() { return instance_; }

		//initialize processors by setting progress func
		void InitProgress(const std::function<void(int, const std::string&)>& f);

		//config file handlers
		TreeFileFactory& get_tree_file_factory();

		//video encoder
		VideoEncoder& get_video_encoder();

		//comp gen
		void set_cg_table_enable(bool value) { comp_gen_table_enable_ = value; }
		bool get_cg_table_enable() { return comp_gen_table_enable_; }
		flrd::EntryParams& get_cg_entry();
		flrd::TableHistParams& get_cg_table();
		//vol prop
		void set_vp_table_enable(bool value) { vol_prop_table_enable_ = value; }
		bool get_vp_table_enable() { return vol_prop_table_enable_; }
		flrd::TableHistParams& get_vp_table();

		//python
		flrd::PyBase* get_add_pybase(const std::string& name);
		flrd::PyDlc* get_add_pydlc(const std::string& name);
		void clear_python();

		//undo sliders
		void add_undo_control(Undoable* control);
		void del_undo_control(Undoable* control);
		void undo();
		void redo();

		//settings
		MainSettings& get_settings();
		AutomateDefault& get_automate_def();
		BrushDefault& get_brush_def();
		ComponentDefault& get_comp_def();
		OutAdjDefault& get_outadj_def();
		ViewDefault& get_view_def();
		VolumeDataDefault& get_vol_def();
		MovieDefault& get_movie_def();
		ColocalDefault& get_colocal_def();

		//states
		GlobalStates& get_states();

		//data processors
		void apply_processor_settings();
		flrd::ComponentGenerator& get_comp_generator();
		flrd::ComponentAnalyzer& get_comp_analyzer();
		flrd::ComponentSelector& get_comp_selector();
		flrd::ComponentEditor& get_comp_editor();
		flrd::VolumeSelector& get_vol_selector();
		flrd::VolumeCalculator& get_vol_calculator();
		KernelExecutor& get_kernel_executor();
		flrd::ScriptProc& get_script_proc();
		flrd::RulerAlign& get_aligner();
		flrd::TrackMapProcessor& get_trackmap_proc();
		flrd::RulerHandler& get_ruler_handler();
		flrd::RulerRenderer& get_ruler_renderer();
		flrd::VolumePoint& get_volume_point();
		flrd::SegGrow& get_seg_grow();
		flrd::DistCalculator& get_dist_calculator();
		Interpolator& get_interpolator();
		MovieMaker& get_movie_maker();
		DataManager& get_data_manager();
		VolumeLoader& get_vol_loader();
		flrd::Colocalize& get_colocalizer();
		flrd::Clusterizer& get_clusterizer();

		//mesh converter
		flrd::BaseConvVolMesh* get_conv_vol_mesh();

		//xr renderer
		LookingGlassRenderer& get_looking_glass_renderer();
		BaseXrRenderer* get_xr_renderer();

		//time
		AsyncTimer* getAsyncTimer(const std::string& name);
		StopWatch* getStopWatch(const std::string& name);
		AsyncTimerFactory& getAsyncTimerFactory();
		StopWatchFactory& getStopWatchFactory();

		//jvm
		JVMInitializer* get_jvm_instance();

		//graphics resources
		flvr::VolKernelFactory& get_vol_kernel_factory();
		flvr::FramebufferManager& get_framebuffer_manager();
		flvr::VertexArrayManager& get_vertex_array_manager();
		flvr::VolShaderFactory& get_vol_shader_factory();
		flvr::SegShaderFactory& get_seg_shader_factory();
		flvr::VolCalShaderFactory& get_vol_cal_shader_factory();
		flvr::ImgShaderFactory& get_img_shader_factory();
		flvr::LightFieldShaderFactory& get_light_field_shader_factory();
		flvr::TextTextureManager& get_text_tex_manager();
		flvr::MshShaderFactory& get_msh_shader_factory();

		//current selection
		CurrentObjects& get_current_objects();

		//project
		Project& get_project();

		std::string& get_help_url() { return help_url_; }

		//linked rotation
		bool get_linked_rot() { return m_linked_rot; }
		void set_linked_rot(bool value) { m_linked_rot = value; }
		std::shared_ptr<RenderView> get_master_linked_view() { return m_master_linked_view.lock(); }
		void set_master_linked_view(const std::shared_ptr<RenderView>& view) { m_master_linked_view = view; }

	private:
		static Global instance_;

		//config file handlers
		std::unique_ptr<TreeFileFactory> tree_file_factory_;

		//video encoder
		std::unique_ptr<VideoEncoder> encoder_;

		//comp gen
		bool comp_gen_table_enable_;//add records from ui
		std::unique_ptr<flrd::EntryParams> comp_gen_entry_;//temporary entry to save cg params
		std::unique_ptr<flrd::TableHistParams> comp_gen_table_;//records for learning comp generation settings
		//vol prop
		bool vol_prop_table_enable_;//add records for vol prop
		std::unique_ptr<flrd::TableHistParams> vol_prop_table_;//records for learning vol props

		//python
		using PyList = std::unordered_map<std::string, flrd::PyBase*>;//python list
		PyList python_list_;

		//controls for undo and redo
		std::vector<Undoable*> undo_ctrls_;

		//settings
		std::unique_ptr<MainSettings> main_settings_;

		//states
		std::unique_ptr<GlobalStates> states_;

		//the data processors
		std::unique_ptr<flrd::ComponentGenerator> m_comp_generator;
		std::unique_ptr<flrd::ComponentAnalyzer> m_comp_analyzer;
		std::unique_ptr<flrd::ComponentSelector> m_comp_selector;
		std::unique_ptr<flrd::ComponentEditor> m_comp_editor;
		std::unique_ptr<flrd::VolumeSelector> m_vol_selector;
		std::unique_ptr<flrd::VolumeCalculator> m_vol_calculator;
		std::unique_ptr<KernelExecutor> m_kernel_executor;
		std::unique_ptr<flrd::ScriptProc> m_script_proc;
		std::unique_ptr<flrd::RulerAlign> m_aligner;
		std::unique_ptr<flrd::TrackMapProcessor> m_trackmap_proc;
		std::unique_ptr<flrd::RulerHandler> m_ruler_handler;
		std::unique_ptr<flrd::RulerRenderer> m_ruler_renderer;
		std::unique_ptr<flrd::VolumePoint> m_volume_point;
		std::unique_ptr<flrd::SegGrow> m_seg_grow;
		std::unique_ptr<flrd::DistCalculator> m_dist_calculator;
		std::unique_ptr<Interpolator> m_interpolator;
		std::unique_ptr<MovieMaker> m_movie_maker;
		std::unique_ptr<DataManager> m_data_manager;
		std::unique_ptr<VolumeLoader> m_vol_loader;
		std::unique_ptr<flrd::Colocalize> m_colocalizer;
		std::unique_ptr<flrd::Clusterizer> m_clusterizer;
		std::unique_ptr<flrd::BaseConvVolMesh> m_conv_vol_mesh;

		//xr renderer
		std::unique_ptr<LookingGlassRenderer> m_lg_renderer;
		std::unique_ptr<BaseXrRenderer> m_xr_renderer = nullptr;

		//time
		std::unique_ptr<fluo::AsyncTimerFactory> m_atmf = nullptr;
		std::unique_ptr<fluo::StopWatchFactory> m_swhf = nullptr;

		//jvm
		std::unique_ptr<JVMInitializer> m_pJVMInstance = nullptr;

		//graphics resources
		//kernel for calculation
		std::unique_ptr<flvr::VolKernelFactory> vol_kernel_factory_;
		//framebuffers for everything
		std::unique_ptr<flvr::FramebufferManager> framebuffer_manager_;
		//vertex arrays
		std::unique_ptr<flvr::VertexArrayManager> vertex_array_manager_;
		//sahder for volume rendering
		std::unique_ptr<flvr::VolShaderFactory> vol_shader_factory_;
		//shader for segmentation
		std::unique_ptr<flvr::SegShaderFactory> seg_shader_factory_;
		//shader for calculation
		std::unique_ptr<flvr::VolCalShaderFactory> cal_shader_factory_;
		//smooth filter
		std::unique_ptr<flvr::ImgShaderFactory> img_shader_factory_;
		//for looking glass
		std::unique_ptr<flvr::LightFieldShaderFactory> light_field_shader_factory_;
		//text texture
		std::unique_ptr<flvr::TextTextureManager> text_texture_manager_;
		//mesh shader
		std::unique_ptr<flvr::MshShaderFactory> msh_shader_factory_;

		//current selection
		std::unique_ptr<CurrentObjects> current_objects_;

		//project management
		std::unique_ptr<Project> project_;

		//help url
		std::string help_url_;

		//linked rotation
		bool m_linked_rot;
		std::weak_ptr<RenderView> m_master_linked_view;

	private:
		Global();
		~Global();
		Global(const Global&) = delete;
		Global& operator=(const Global&) = delete;
		Global(Global&&) = delete;
		Global& operator=(Global&&) = delete;

		void Init();
		void InitDatabase();
		void BuildFactories();
		void InitLocale();
	};

}
#endif
