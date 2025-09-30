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
#include <Names.h>
#include <TreeFileFactory.h>
#include <VideoEncoder.h>
#include <Reshape.h>
#include <EntryParams.h>
#include <TableHistParams.h>
#include <PyDlc.h>
#include <Undoable.h>
#include <MainSettings.h>
#include <GlobalStates.h>
//processors
#include <CompGenerator.h>
#include <CompAnalyzer.h>
#include <CompSelector.h>
#include <CompEditor.h>
#include <VolumeSelector.h>
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
#include <Interpolator.h>
#include <MovieMaker.h>
#include <DataManager.h>
#include <MeshData.h>
#include <RenderView.h>
#include <CurrentObjects.h>
#include <VolumeLoader.h>
#include <Colocalize.h>
#include <Clusterizer.h>
#include <ConvVolMeshSw.h>
#include <ConvVolMesh.h>
#include <ColorCompMesh.h>
#include <LookingGlassRenderer.h>
#include <HololensRenderer.h>
#include <OpenXrRenderer.h>
#include <OpenVrRenderer.h>
#include <AsyncTimerFactory.hpp>
#include <StopWatchFactory.hpp>
#include <JVMInitializer.h>
#include <VolKernel.h>
#include <Framebuffer.h>
#include <VertexArray.h>
#include <VolShader.h>
#include <SegShader.h>
#include <VolCalShader.h>
#include <ImgShader.h>
#include <LightFieldShader.h>
#include <TextRenderer.h>
#include <MshShader.h>
#include <Project.h>
#include <MeshDefault.h>

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
std::string IniFile::path_sep_s_ = "";

Global Global::instance_;

Global::Global() :
	tree_file_factory_(std::make_unique<TreeFileFactory>()),
	encoder_(std::make_unique<VideoEncoder>()),
	main_settings_(std::make_unique<MainSettings>()),
	states_(std::make_unique<GlobalStates>()),
	//processors
	m_comp_generator(std::make_unique<flrd::ComponentGenerator>()),
	m_comp_analyzer(std::make_unique<flrd::ComponentAnalyzer>()),
	m_comp_selector(std::make_unique<flrd::ComponentSelector>()),
	m_comp_editor(std::make_unique<flrd::ComponentEditor>()),
	m_vol_selector(std::make_unique<flrd::VolumeSelector>()),
	m_vol_calculator(std::make_unique<flrd::VolumeCalculator>()),
	m_kernel_executor(std::make_unique<KernelExecutor>()),
	m_script_proc(std::make_unique<flrd::ScriptProc>()),
	m_aligner(std::make_unique<flrd::RulerAlign>()),
	m_trackmap_proc(std::make_unique<flrd::TrackMapProcessor>()),
	m_ruler_handler(std::make_unique<flrd::RulerHandler>()),
	m_ruler_renderer(std::make_unique<flrd::RulerRenderer>()),
	m_volume_point(std::make_unique<flrd::VolumePoint>()),
	m_seg_grow(std::make_unique<flrd::SegGrow>()),
	m_dist_calculator(std::make_unique<flrd::DistCalculator>()),
	m_interpolator(std::make_unique<Interpolator>()),
	m_movie_maker(std::make_unique<MovieMaker>()),
	m_data_manager(std::make_unique<DataManager>()),
	m_vol_loader(std::make_unique<VolumeLoader>()),
	m_colocalizer(std::make_unique<flrd::Colocalize>()),
	m_clusterizer(std::make_unique<flrd::Clusterizer>()),
	//m_conv_vol_mesh(std::make_unique<flrd::ConvVolMeshSw>()),
	m_color_comp_mesh(std::make_unique<flrd::ColorCompMesh>()),
	m_lg_renderer(std::make_unique<LookingGlassRenderer>()),
	m_atmf(std::make_unique<fluo::AsyncTimerFactory>()),
	m_swhf(std::make_unique<fluo::StopWatchFactory>()),
	vol_kernel_factory_(std::make_unique<flvr::VolKernelFactory>()),
	framebuffer_manager_(std::make_unique<flvr::FramebufferManager>()),
	vertex_array_manager_(std::make_unique<flvr::VertexArrayManager>()),
	vol_shader_factory_(std::make_unique<flvr::VolShaderFactory>()),
	seg_shader_factory_(std::make_unique<flvr::SegShaderFactory>()),
	cal_shader_factory_(std::make_unique<flvr::VolCalShaderFactory>()),
	img_shader_factory_(std::make_unique<flvr::ImgShaderFactory>()),
	light_field_shader_factory_(std::make_unique<flvr::LightFieldShaderFactory>()),
	text_texture_manager_(std::make_unique<flvr::TextTextureManager>()),
	msh_shader_factory_(std::make_unique<flvr::MshShaderFactory>()),
	current_objects_(std::make_unique<CurrentObjects>()),
	project_(std::make_unique<Project>()),
	m_linked_rot(false)
{
	Init();
}

Global::~Global() = default;

void Global::InitProgress(const std::function<void(int, const std::string&)>& f)
{
	//set progress functions for all processors
	m_comp_generator->SetProgressFunc(f);
	m_comp_analyzer->SetProgressFunc(f);
	m_vol_calculator->SetProgressFunc(f);
	m_kernel_executor->SetProgressFunc(f);
	m_trackmap_proc->SetProgressFunc(f);
	m_data_manager->SetProgressFunc(f);
	m_clusterizer->SetProgressFunc(f);
	//m_conv_vol_mesh->SetProgressFunc(f);
	project_->SetProgressFunc(f);
}

void Global::Init()
{
	InitDatabase();
	BuildFactories();
	help_url_ = "https://github.com/SCIInstitute/fluorender";
	InitLocale();
}

void Global::InitDatabase()
{
	//comp gen
	comp_gen_table_enable_ = false;
	comp_gen_entry_ = std::make_unique<flrd::EntryParams>();
	comp_gen_table_ = std::make_unique<flrd::TableHistParams>();
	comp_gen_entry_->setParams(flrd::Reshape::get_params("comp_gen"));
	comp_gen_table_->setParams(flrd::Reshape::get_params("comp_gen"));
	//vol prop
	vol_prop_table_enable_ = false;
	vol_prop_table_ = std::make_unique<flrd::TableHistParams>();
	vol_prop_table_->setParams(flrd::Reshape::get_params("vol_prop"));
}

void Global::BuildFactories()
{
	m_atmf->createDefault();
	m_swhf->createDefault();
}

//locale
void Global::InitLocale()
{
	std::setlocale(LC_ALL, "en_US.UTF-8");
}

TreeFileFactory& Global::get_tree_file_factory()
{
	return *tree_file_factory_;
}

VideoEncoder& Global::get_video_encoder()
{
	return *encoder_;
}

flrd::EntryParams& Global::get_cg_entry()
{
	return *comp_gen_entry_;
}

flrd::TableHistParams& Global::get_cg_table()
{
	return *comp_gen_table_;
}

flrd::TableHistParams& Global::get_vp_table()
{
	return *vol_prop_table_;
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

//undo
void Global::add_undo_control(Undoable* control)
{
	undo_ctrls_.push_back(control);
}

void Global::del_undo_control(Undoable* control)
{
	auto it = std::find(undo_ctrls_.begin(), undo_ctrls_.end(), control);
	if (it != undo_ctrls_.end())
		undo_ctrls_.erase(it);
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

	double time_span = main_settings_->m_time_span;
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

	double time_span = main_settings_->m_time_span;
	for (auto i : undo_ctrls_)
	{
		double time = i->GetTimeRedo();
		if (std::fabs(time - t) < time_span)
			i->Redo();
	}
}

//settings
MainSettings& Global::get_settings()
{
	return *main_settings_;
}

AutomateDefault& Global::get_automate_def()
{
	return main_settings_->m_automate_def;
}

BrushDefault& Global::get_brush_def()
{
	return main_settings_->m_brush_def;
}

ComponentDefault& Global::get_comp_def()
{
	return main_settings_->m_comp_def;
}

OutAdjDefault& Global::get_outadj_def()
{
	return main_settings_->m_outadj_def;
}

ViewDefault& Global::get_view_def()
{
	return main_settings_->m_view_def;
}

VolumeDataDefault& Global::get_vol_def()
{
	return main_settings_->m_vol_def;
}

MeshDefault& Global::get_mesh_def()
{
	return main_settings_->m_mesh_def;
}

MovieDefault& Global::get_movie_def()
{
	return main_settings_->m_movie_def;
}

ColocalDefault& Global::get_colocal_def()
{
	return main_settings_->m_colocal_def;
}

//states
GlobalStates& Global::get_states()
{
	return *states_;
}

//processors
void Global::apply_processor_settings()
{
	m_script_proc->SetBreak(glbin_settings.m_script_break);
	m_seg_grow->SetRulerHandler(m_ruler_handler.get());
}

flrd::ComponentGenerator& Global::get_comp_generator()
{
	return *m_comp_generator;
}

flrd::ComponentAnalyzer& Global::get_comp_analyzer()
{
	return *m_comp_analyzer;
}

flrd::ComponentSelector& Global::get_comp_selector()
{
	return *m_comp_selector;
}

flrd::ComponentEditor& Global::get_comp_editor()
{
	return *m_comp_editor;
}

flrd::VolumeSelector& Global::get_vol_selector()
{
	return *m_vol_selector;
}

flrd::VolumeCalculator& Global::get_vol_calculator()
{
	return *m_vol_calculator;
}

KernelExecutor& Global::get_kernel_executor()
{
	return *m_kernel_executor;
}

flrd::ScriptProc& Global::get_script_proc()
{
	return *m_script_proc;
}

flrd::RulerAlign& Global::get_aligner()
{
	return *m_aligner;
}

flrd::TrackMapProcessor& Global::get_trackmap_proc()
{
	return *m_trackmap_proc;
}

flrd::RulerHandler& Global::get_ruler_handler()
{
	return *m_ruler_handler;
}

flrd::RulerRenderer& Global::get_ruler_renderer()
{
	return *m_ruler_renderer;
}

flrd::VolumePoint& Global::get_volume_point()
{
	return *m_volume_point;
}

flrd::SegGrow& Global::get_seg_grow()
{
	return *m_seg_grow;
}

flrd::DistCalculator& Global::get_dist_calculator()
{
	return *m_dist_calculator;
}

Interpolator& Global::get_interpolator()
{
	return *m_interpolator;
}

MovieMaker& Global::get_movie_maker()
{
	return *m_movie_maker;
}

DataManager& Global::get_data_manager()
{
	return *m_data_manager;
}

VolumeLoader& Global::get_vol_loader()
{
	return *m_vol_loader;
}

flrd::Colocalize& Global::get_colocalizer()
{
	return *m_colocalizer;
}

flrd::Clusterizer& Global::get_clusterizer()
{
	return *m_clusterizer;
}

flrd::BaseConvVolMesh* Global::get_conv_vol_mesh()
{
	if (m_conv_vol_mesh)
		return m_conv_vol_mesh.get();
	//create
	switch (glbin_settings.m_vol_mesh_conv_mode)
	{
	case 0://software
		m_conv_vol_mesh = std::make_unique<flrd::ConvVolMeshSw>();
		break;
	case 1://opencl
	default:
		m_conv_vol_mesh = std::make_unique<flrd::ConvVolMesh>();
		break;
	}
	m_conv_vol_mesh->SetProgressFunc(project_->GetProgressFunc());
	flrd::BaseConvVolMesh* cvm = m_conv_vol_mesh.get();
	glbin_mesh_def.Apply(cvm);

	return cvm;
}

flrd::ColorCompMesh& Global::get_color_comp_mesh()
{
	return *m_color_comp_mesh;
}

LookingGlassRenderer& Global::get_looking_glass_renderer()
{
	return *m_lg_renderer;
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

AsyncTimer* Global::getAsyncTimer(const std::string& name)
{
	return m_atmf->findFirst(name);
}

StopWatch* Global::getStopWatch(const std::string& name)
{
	return m_swhf->findFirst(name);
}

AsyncTimerFactory& Global::getAsyncTimerFactory()
{
	return *m_atmf;
}

StopWatchFactory& Global::getStopWatchFactory()
{
	return *m_swhf;
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

//graphics resources
flvr::VolKernelFactory& Global::get_vol_kernel_factory()
{
	return *vol_kernel_factory_;
}

flvr::FramebufferManager& Global::get_framebuffer_manager()
{
	return *framebuffer_manager_;
}

flvr::VertexArrayManager& Global::get_vertex_array_manager()
{
	return *vertex_array_manager_;
}

flvr::VolShaderFactory& Global::get_vol_shader_factory()
{
	return *vol_shader_factory_;
}

flvr::SegShaderFactory& Global::get_seg_shader_factory()
{
	return *seg_shader_factory_;
}

flvr::VolCalShaderFactory& Global::get_vol_cal_shader_factory()
{
	return *cal_shader_factory_;
}

flvr::ImgShaderFactory& Global::get_img_shader_factory()
{
	return *img_shader_factory_;
}

flvr::LightFieldShaderFactory& Global::get_light_field_shader_factory()
{
	return *light_field_shader_factory_;
}

flvr::TextTextureManager& Global::get_text_tex_manager()
{
	return *text_texture_manager_;
}

flvr::MshShaderFactory& Global::get_msh_shader_factory()
{
	return *msh_shader_factory_;
}

//current selection
CurrentObjects& Global::get_current_objects()
{
	return *current_objects_;
}

//project
Project& Global::get_project()
{
	return *project_;
}
