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

#include <FuiManager.h>
#include <JVMInitializer.h>
#include <Global.hpp>
#include <StopWatch.hpp>
#include <AgentFactory.hpp>
#include <RenderFrame.h>
#include <FLIVR/VolumeRenderer.h>
#include <FLIVR/KernelProgram.h>
#include <FLIVR/VolKernel.h>
#include <FLIVR/Framebuffer.h>
#include <FLIVR/VertexArray.h>
#include <FLIVR/VolShader.h>
#include <FLIVR/SegShader.h>
#include <FLIVR/VolCalShader.h>
#include <FLIVR/TextRenderer.h>
#include <FLIVR/MultiVolumeRenderer.h>
#include <Converters/VolumeMeshConv.h>
#include <wx/stdpaths.h>

FuiManager::FuiManager() :
	m_frame(0),
	m_benchmark(false),
	m_fullscreen(false),
	m_windowed(false),
	m_hidepanels(false),
	m_win_width(1600),
	m_win_height(1000),
	m_lzw(false),
	m_save_alpha(false),
	m_save_float(false),
	m_bitrate(10.0),
	m_mov_file(""),
	m_imagej(false)
{
	char cpath[FILENAME_MAX];
	GETCURRENTDIR(cpath, sizeof(cpath));
	std::wstring wstr_path = s2ws(std::string(cpath));
	::wxSetWorkingDirectory(wstr_path);
	wxString expath = wxStandardPaths::Get().GetExecutablePath();
	expath = wxPathOnly(expath);
	glbin.setExecutablePath(expath.ToStdWstring());
	//random numbers
	srand((unsigned int)glbin.getStopWatch(gstStopWatch)->sys_time());
}

FuiManager::~FuiManager()
{
	JVMInitializer::destroyJVM();
	//release?
	flvr::TextureRenderer::vol_kernel_factory_.clear();
	flvr::TextureRenderer::framebuffer_manager_.clear();
	flvr::TextureRenderer::vertex_array_manager_.clear();
	flvr::TextureRenderer::vol_shader_factory_.clear();
	flvr::TextureRenderer::seg_shader_factory_.clear();
	flvr::TextureRenderer::cal_shader_factory_.clear();
	flvr::TextureRenderer::img_shader_factory_.clear();
	flvr::TextRenderer::text_texture_manager_.clear();
	flvr::KernelProgram::release();
}

RenderFrame* FuiManager::GetFrame()
{
	return m_frame;
}

void FuiManager::Init()
{
	//add png handler
	wxImage::AddHandler(new wxPNGHandler);
	glbin.initIcons();

	//the frame
	std::string title = std::string(FLUORENDER_TITLE) + std::string(" ") +
		std::string(VERSION_MAJOR_TAG) + std::string(".") +
		std::string(VERSION_MINOR_TAG);
	m_frame = new RenderFrame(
		wxString(title),
		-1, -1,
		m_win_width, m_win_height);

	SetupAgents();

	fluo::RenderFrameAgent* renderframeagent = glbin_agtf->getRenderFrameAgent();

	// Adding JVm initialization
	JVMInitializer*	pInstance = nullptr;
	if (renderframeagent)
		pInstance = JVMInitializer::getInstance(renderframeagent->GetJvmArgs());

	m_frame->Init(m_benchmark, m_fullscreen, m_windowed, m_hidepanels, pInstance != nullptr);

	wxMemorySize free_mem_size = wxGetFreeMemory();
	double mainmem_buf_size = free_mem_size.ToDouble() * 0.8 / 1024.0 / 1024.0;
	if (mainmem_buf_size > flvr::TextureRenderer::get_mainmem_buf_size())
		flvr::TextureRenderer::set_mainmem_buf_size(mainmem_buf_size);

	renderframeagent->SetTextureRendererSettings();
	renderframeagent->SetTextureUndos();

	bool bval; long lval; double dval;
	m_frame->m_agent->getValue(gstSoftThresh, dval);
	flvr::VolumeRenderer::set_soft_threshold(dval);
	flvr::MultiVolumeRenderer::set_soft_threshold(dval);

	renderframeagent->getValue(gstSoftThresh, dval);
	flvr::VolumeRenderer::set_soft_threshold(dval);
	flvr::MultiVolumeRenderer::set_soft_threshold(dval);
	VolumeMeshConv::SetSoftThreshold(dval);
	std::string sval;
	renderframeagent->getValue(gstFontFile, sval);
	wxString font_file = sval;
	wxString exePath = glbin.getExecutablePath();
	if (font_file != "")
		font_file = exePath + GETSLASH() + "Fonts" +
		GETSLASH() + font_file;
	else
		font_file = exePath + GETSLASH() + "Fonts" +
		GETSLASH() + "FreeSans.ttf";
	flvr::TextRenderer::text_texture_manager_.load_face(font_file.ToStdString());
	renderframeagent->getValue(gstTextSize, dval);
	flvr::TextRenderer::text_texture_manager_.SetSize(dval);

	bool run_mov = false;
	if (m_mov_file != "")
	{
		if (renderframeagent)
		{
			renderframeagent->setValue(gstCaptureCompress, m_lzw);
			renderframeagent->setValue(gstCaptureAlpha, m_save_alpha);
			renderframeagent->setValue(gstCaptureFloat, m_save_float);
		}
		fluo::MovieAgent* movieagent = glbin_agtf->getMovieAgent();
		if (movieagent)
		{
			movieagent->setValue(gstMovBitrate, m_bitrate);
			movieagent->setValue(gstMovFilename, m_mov_file.ToStdWstring());
		}
		run_mov = true;
	}

	if (m_files.GetCount() > 0 && renderframeagent)
		renderframeagent->StartupLoad(m_files, run_mov, m_imagej);

}

void FuiManager::SetupAgents()
{
	//renderframe
	if (m_frame)
	{
		m_frame->m_agent = glbin_agtf->addRenderFrameAgent(gstRenderFrameAgent, *m_frame);
		m_frame->m_agent->setObject(glbin_root);
	}
	//settings
	SettingDlg* setting_dlg = m_frame->GetSettingDlg();
	if (setting_dlg)
	{
		setting_dlg->m_agent = glbin_agtf->addSettingAgent(gstSettingAgent, *setting_dlg);
		setting_dlg->m_agent->setObject(glbin_root);
		setting_dlg->m_agent->ReadSettings();
	}
	//annotation
	AnnotationPropPanel* anno_panel = m_frame->GetAnnotationPropPanel();
	if (anno_panel)
	{
		anno_panel->m_agent = glbin_agtf->addAnnotationPropAgent(gstAnnotationPropAgent, *anno_panel);
	}
	//tree
	TreePanel* tree_panel = m_frame->GetTree();
	if (tree_panel)
	{
		tree_panel->m_tree_model = glbin_agtf->addTreeAgent(gstTreeAgent, *tree_panel);
		tree_panel->m_tree_model->setObject(glbin_root);
		tree_panel->SetScenegraph(glbin_root);
	}
	//list
	ListPanel* list_panel = m_frame->GetList();
	if (list_panel)
	{
		list_panel->m_list_model = glbin_agtf->addListAgent(gstListAgent, *list_panel);
	}
	//brush
	BrushToolDlg* brush_dlg = m_frame->GetBrushToolDlg();
	if (brush_dlg && tree_panel)
	{
		brush_dlg->m_agent = glbin_agtf->addBrushToolAgent(gstBrushToolAgent, *brush_dlg, *tree_panel);
		tree_panel->m_brushtool_agent = brush_dlg->m_agent;
	}
	//calculation
	CalculationDlg* calc_dlg = m_frame->GetCalculationDlg();
	if (calc_dlg)
	{
		calc_dlg->m_agent = glbin_agtf->addCalculationAgent(gstCalculationAgent, *calc_dlg);
	}
	//clip
	ClipPlanePanel* clip_panel = m_frame->GetClippingView();
	if (clip_panel)
	{
		clip_panel->m_agent = glbin_agtf->addClipPlaneAgent(gstClipPlaneAgent, *clip_panel);
	}
	//clkernel
	ClKernelDlg* cl_dlg = m_frame->GetOclDlg();
	if (cl_dlg)
	{
		cl_dlg->m_agent = glbin_agtf->addClKernelAgent(gstClKernelAgent, *cl_dlg);
	}
	//colocalization analysis
	ColocalDlg* colocal_dlg = m_frame->GetColocalizationDlg();
	if (colocal_dlg)
	{
		colocal_dlg->m_agent = glbin_agtf->addColocalAgent(gstColocalAgent, *colocal_dlg);
	}
	//components
	ComponentDlg* comp_dlg = m_frame->GetComponentDlg();
	if (comp_dlg)
	{
		comp_dlg->m_agent = glbin_agtf->addComponentAgent(gstComponentAgent, *comp_dlg);
	}
	//convert
	ConvertDlg* conv_dlg = m_frame->GetConvertDlg();
	if (conv_dlg)
	{
		conv_dlg->m_agent = glbin_agtf->addConvertAgent(gstConvertAgent, *conv_dlg);
	}
	//count
	CountingDlg* count_dlg = m_frame->GetCountingDlg();
	if (count_dlg)
	{
		count_dlg->m_agent = glbin_agtf->addCountingAgent(gstCountingAgent, *count_dlg);
	}
	//measurements
	MeasureDlg* measure_dlg = m_frame->GetMeasureDlg();
	{
		measure_dlg->m_agent = glbin_agtf->addMeasureAgent(gstMeasureAgent, *measure_dlg);
		measure_dlg->m_rulerlist->m_agent = measure_dlg->m_agent;
	}
	//mesh properties
	MeshPropPanel* mesh_panel = m_frame->GetMeshPropPanel();
	if (mesh_panel)
	{
		mesh_panel->m_agent = glbin_agtf->addMeshPropAgent(gstMeshPropAgent, *mesh_panel);
	}
	//mesh transform
	MeshTransPanel* mtrans_panel = m_frame->GetMeshTransPanel();
	if (mtrans_panel)
	{
		mtrans_panel->m_agent = glbin_agtf->addMeshTransAgent(gstMeshTransAgent, *mtrans_panel);
	}
	//movie
	MoviePanel* movie_panel = m_frame->GetMovieView();
	if (movie_panel)
	{
		movie_panel->m_agent = glbin_agtf->addMovieAgent(gstMovieAgent, *movie_panel);
	}
	//noise reduct
	NoiseReduceDlg* noise_dlg = m_frame->GetNoiseCancellingDlg();
	if (noise_dlg)
	{
		noise_dlg->m_agent = glbin_agtf->addNoiseReduceAgent(gstNoiseReduceAgent, *noise_dlg);
	}
	//output adjust
	OutAdjustPanel* out_panel = m_frame->GetAdjustView();
	if (out_panel)
	{
		out_panel->m_agent = glbin_agtf->addOutAdjustAgent(gstOutAdjustAgent, *out_panel);
	}
	//recorder
	RecorderDlg* rec_dlg = m_frame->GetRecorderDlg();
	if (rec_dlg)
	{
		rec_dlg->m_agent = glbin_agtf->addRecorderAgent(gstRecorderAgent, *rec_dlg);
		rec_dlg->m_keylist->m_agent = rec_dlg->m_agent;
	}
	//track
	TrackDlg* track_dlg = m_frame->GetTraceDlg();
	if (track_dlg)
	{
		track_dlg->m_agent = glbin_agtf->addTrackAgent(gstTrackAgent, *track_dlg);
		track_dlg->m_compagent = glbin_agtf->getComponentAgent();
		track_dlg->m_movieagent = glbin_agtf->getMovieAgent();
		track_dlg->m_trace_list_curr->m_agent = track_dlg->m_agent;
		track_dlg->m_trace_list_prev->m_agent = track_dlg->m_agent;
	}
	//volume properties
	VolumePropPanel* vol_panel = m_frame->GetPropView();
	if (vol_panel)
	{
		vol_panel->m_agent = glbin_agtf->addVolumePropAgent(gstVolumePropAgent, *vol_panel);
	}
}