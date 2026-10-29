/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2026 Scientific Computing and Imaging Institute,
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

#include <MainFrameAgent.h>
#include <MainFrame.h>
#include <RenderViewPanel.h>
#include <RenderCanvas.h>
#include <RenderCanvasAgent.h>
#include <RenderViewPanelAgent.h>
#include <Global.h>
#include <Tokens.h>
#include <Root.h>
#include <DataManager.h>
#include <RenderView.h>
#include <VolumeData.h>
#include <Coordinator.h>
#include <MainSettings.h>
#include <ScriptProc.h>
#include <CurrentObjects.h>
#include <GlobalStates.h>
#include <PyBase.h>
#include <Directory.h>
#include <TextRenderer.h>
#include <MovieMaker.h>
#include <KernelProgram.h>
#include <TextureRenderer.h>
#include <ModalDlg.h>
#include <Project.h>
#include <compatibility.h>

MainFrameAgent::MainFrameAgent(
	MainFrame* frame) :
	Agent(frame)
{

}

void MainFrameAgent::UpdateUI(const UpdateRequest& request)
{
	auto frame = GetMainFrame();
	if (!frame)
		return;

	bool update_all = request.values.empty();

	if (update_all || request.HasValue(gstProjPanel))
	{
		frame->UpdateProjPanel();
	}
	if (update_all || request.HasValue(gstMoviePanel))
	{
		frame->UpdateMoviePanel();
	}
	if (update_all || request.HasValue(gstPropertyPanel))
	{
		frame->UpdatePropertyPanel();
	}
	if (update_all || request.HasValue(gstOutAdjPanel))
	{
		frame->UpdateOutAdjPanel();
	}
	if (update_all || request.HasValue(gstClipPlanePanel))
	{
		frame->UpdateClipPlanePanel();
	}
	if (update_all || request.HasValue(gstMainToolbar))
	{
		frame->UpdateMainToolbar();
	}
	if (update_all || request.HasValue(gstProjectTitle))
	{
		auto str = glbin_data_manager.GetProjectFile();
		frame->UpdateMainFrameTitle(str);
	}
	if (update_all || request.HasValue(gstBenchmarkTitle))
	{
		frame->UpdateMainFrameTitle(s2ws(m_benchmark_str));
	}
	if (update_all || request.HasValue(gstMainStatusbarText))
	{
		frame->UpdateMainStatusbarText(glbin_states.m_status_str);
	}
	if (request.HasValue(gstMainStatusbarPush))
	{
		frame->PushMainStatusbarText(glbin_states.m_status_str);
	}
	if (request.HasValue(gstMainStatusbarPop))
	{
		frame->PopMainStatusbarText();
	}
	if (update_all || request.HasValue(gstFullscreenMenu))
	{
		frame->UpdateFullscreenMenu();
	}

	if (request.HasValue(gstCreateRenderViewPanel))
		CreateRenderViewPanel();
	if (request.HasValue(gstRenderViewPanelLayout))
		LayoutRenderViewPanels();
	if (request.HasValue(gstOutAdjPanelLayout))
		LayoutOutAdjPanel();
	if (request.HasValue(gstRenderViewPanel))
		SyncRenderViewPanels();
	if (request.HasValue(gstScriptBreakDlg))
		ToggleScriptBreakDlg();
	if (request.HasValue(gstShowAllPanels))
		frame->ToggleAllPanels(1);
	if (request.HasValue(gstHideAllPanels))
		frame->ToggleAllPanels(0);
	if (request.HasValue(gstToggleAllPanels))
		frame->ToggleAllPanels(2);
	if (request.HasValue(gstToggleToolbar))
		ToggleToolbar();
	if (request.HasValue(gstToggleProjPanel))
		ToggleProjPanel();
	if (request.HasValue(gstToggleMoviePanel))
		ToggleMoviePanel();
	if (request.HasValue(gstToggleOutAdjPanel))
		ToggleOutAdjPanel();
	if (request.HasValue(gstToggleClipPlanePanel))
		ToggleClipPlanePanel();
	if (request.HasValue(gstTogglePropertyPanel))
		TogglePropertyPanel();

	if (request.HasValue(gstLastTool))
		LastTool();
	if (request.HasValue(gstBrushToolDlg))
		ShowBrushDlg();
	if (request.HasValue(gstCalculationDlg))
		ShowCalculationDlg();
	if (request.HasValue(gstColocalizationDlg))
		ShowColocalizationDlg();
	if (request.HasValue(gstComponentDlg))
		ShowComponentDlg();
	if (request.HasValue(gstConvertDlg))
		ShowConvertDlg();
	if (request.HasValue(gstCountingDlg))
		ShowCountingDlg();
	if (request.HasValue(gstMachineLearningDlg))
		ShowMachineLearningDlg();
	if (request.HasValue(gstMeasureDlg))
		ShowMeasureDlg();
	if (request.HasValue(gstNoiseCancellingDlg))
		ShowNoiseCancellingDlg();
	if (request.HasValue(gstOclDlg))
		ShowOclDlg();
	if (request.HasValue(gstSettingDlg))
		ShowSettingDlg();
	if (request.HasValue(gstTrackDlg))
		ShowTrackDlg();

	if (request.HasValue(gstAboutDlg))
		ShowAboutDlg();
	if (request.HasValue(gstScriptProcFileDlg))
		ShowScriptProcFileDlg();

	if (request.HasValue(gstResetLayout))
		ResetLayout();

	if (request.HasValue(gstFullscreenFrame))
		ToggleFullscreen();

	if (request.HasValue(gstQuitCanvasFullscreen))
		QuitCanvasFullscreen();

	if (request.HasValue(gstMainFrameClose))
		Close();
}

void MainFrameAgent::UpdateData(const UpdateRequest& request)
{
	if (request.HasValue(gstLastOpen))
		LastOpen();
	if (request.HasValue(gstOpenVolume))
		OpenVolume();
	if (request.HasValue(gstImportVolume))
		ImportVolume();
	if (request.HasValue(gstOpenMesh))
		OpenMesh();
	if (request.HasValue(gstNewProject))
		NewProject();
	if (request.HasValue(gstOpenProject))
		OpenProject();
	if (request.HasValue(gstSaveProject))
		SaveProject();
	if (request.HasValue(gstSaveAsProject))
		SaveAsProject();
}

MainFrame* MainFrameAgent::GetMainFrame() const
{
	return static_cast<MainFrame*>(GetOwner());
}

void MainFrameAgent::LoadSettings()
{
	auto mainframe = GetMainFrame();
	if (!mainframe)
		return;

	int reset = mainframe->GetReset();
	//create this first to read the settings
	if (reset == 0)
		glbin_settings.Read();
	else if (reset == 2)
		glbin_settings.Read("fluorender_default");
}

void MainFrameAgent::ApplySettings()
{
	auto mainframe = GetMainFrame();
	if (!mainframe)
		return;

	double mem_size = mainframe->GetMainMemorySizeInMB();
	if (mem_size > flvr::TextureRenderer::get_mainmem_buf_size())
		flvr::TextureRenderer::set_mainmem_buf_size(mem_size);

	glbin.apply_processor_settings();
	glbin_comp_def.Apply(glbin_clusterizer);
	glbin_comp_def.Apply(glbin_comp_analyzer);
	glbin_comp_def.Apply(glbin_comp_generator);
	glbin_comp_def.Apply(glbin_comp_selector);
	glbin_brush_def.Apply(glbin_vol_selector);
	glbin_mesh_def.Apply(glbin_conv_vol_mesh);
	glbin_mov_def.Apply(glbin_moviemaker);
	glbin_data_manager.UpdateStreamMode(-1.0);

	//set frame
	//glbin_script_proc.SetFrame(this);
	//glbin_data_manager.SetFrame(this);
	//glbin_current.mainframe = this;
	//progress
	//these global objects are constructed before mainframe. therefore need to set progress functions again
	glbin.InitProgress(std::bind(&MainFrame::UpdateProgress, mainframe,
		std::placeholders::_1, std::placeholders::_2));

	glbin_states.m_benchmark = mainframe->GetBenchmark();

	//python
	flrd::PyBase::SetHighVer(glbin_settings.m_python_ver);
	//font
	std::wstring font_file = glbin_settings.m_font_file;
	std::filesystem::path p = GetDataRoot();
	p /= "Fonts";
	if (font_file != L"")
		p /= font_file;
	else
		p /= "FreeSans.ttf";
	font_file = p.wstring();
	glbin_text_tex_manager.load_face(font_file);
	glbin_text_tex_manager.set_size(glbin_settings.m_text_size);

	glbin_states.m_status_str = std::string(fluo::AppTitle) + " started normally.";

	//if (!renderview_agent_list_.empty())
	//{
	//	auto view = renderview_agent_list_.begin();
	//	glbin_moviemaker.SetView(*view);
	//}
	//glbin_mov_def.Apply(glbin_moviemaker);
	if (fluo::InEpsilon(glbin_settings.m_dpi_scale_factor,
		mainframe->GetDPIScaleFactor()))
		mainframe->LoadPerspective(glbin_settings.m_layout);
	//glbin_moviemaker.SetMainFrame(this);
}

void MainFrameAgent::PreShutdown()
{
	auto mainframe = GetMainFrame();
	if (!mainframe)
		return;

	glbin_moviemaker.Stop();

	glbin.clear_python();

	//release
	glbin_text_tex_manager.clear();
	flvr::KernelProgram::release_context();

	glbin_brush_def.Set(glbin_vol_selector);
	glbin_comp_def.Set(glbin_comp_generator);
	glbin_comp_def.Set(glbin_clusterizer);
	glbin_comp_def.Set(glbin_comp_selector);
	glbin_comp_def.Set(glbin_comp_analyzer);
	glbin_mov_def.Set(glbin_moviemaker);
	glbin_mesh_def.Set(glbin_conv_vol_mesh);
	//ui settings
	//mainframe layout
	glbin_settings.m_mainframe_disp_id = mainframe->GetDisplayId();
	auto rect = mainframe->GetMainFrameRect();
	glbin_settings.m_mainframe_x = rect[0];
	glbin_settings.m_mainframe_y = rect[1];
	glbin_settings.m_mainframe_w = rect[2];
	glbin_settings.m_mainframe_h = rect[3];
	//dpi scale factor
	glbin_settings.m_dpi_scale_factor = mainframe->GetDPIScaleFactor();
	//frame layout
	glbin_settings.m_layout = mainframe->SavePerspective();
	glbin_settings.Save();
}

void MainFrameAgent::ShutdownAll()
{
	glbin_coordinator.AdvanceToState(AgentState::PreShutdown);
}

void MainFrameAgent::SetFocusVRenderViews(wxBasisSlider* slider)
{
	for (auto view_agent : renderview_agent_list_)
	{
		if (!view_agent)
			continue;

		auto canvas_agent = view_agent->GetRenderCanvasAgent();
		if (canvas_agent)
		{
			RenderCanvas* canvas = canvas_agent->GetCanvas();
			if (canvas)
				canvas->SetFocusedSlider(slider);
		}
	}
}

void MainFrameAgent::AddRenderView(RenderViewPanelAgent* agent)
{
	if (!agent)
		return;
	renderview_agent_list_.insert(agent);
}

void MainFrameAgent::RemoveRenderView(RenderViewPanelAgent* agent)
{
	if (!agent)
		return;
	renderview_agent_list_.erase(agent);
}

void MainFrameAgent::DeleteRenderViewPanel(RenderViewPanelAgent* agent)
{
	auto frame = GetMainFrame();
	if (frame)
	{
		auto panel = agent->GetPanel();
		int i = frame->DeleteRenderViewPanel(panel);
		if (glbin_mov_def.m_view_idx >= i)
			glbin_mov_def.m_view_idx--;
	}
}

void MainFrameAgent::CreateRenderViewPanel()
{
	auto frame = GetMainFrame();
	if (!frame)
		return;

	//create ui
	auto panel = frame->CreateRenderViewPanel();
	if (!panel)
		return;

	auto view_agent = panel->GetAgent()->As<RenderViewPanelAgent>();
	if (!view_agent)
		return;
	auto view = view_agent->GetView();
	if (!view)
		return;

	//set view default settings
	//if (m_output_adj_panel)
	//{
		/*Color gamma, brightness, hdr;
		bool sync_r, sync_g, sync_b;
		m_output_adj_panel->GetDefaults(gamma, brightness, hdr, sync_r, sync_g, sync_b);
		vrv->m_canvas->SetGamma(gamma);
		vrv->m_canvas->SetBrightness(brightness);
		vrv->m_canvas->SetHdr(hdr);*/
		//view->SetSyncR(true);
		//view->SetSyncG(true);
		//view->SetSyncB(true);
	//}

	//add volumes
	Root* root = glbin_data_manager.GetRoot();
	if (root && root->GetViewNum() > 0)
	{
		auto view0 = root->GetView(0);
		if (view0)
		{
			for (int i = 0; i < view0->GetDispVolumeNum(); ++i)
			{
				auto vd = view0->GetDispVolumeData(i);
				if (vd)
				{
					auto vd_add = glbin_data_manager.DuplicateVolumeData(vd);

					if (vd_add)
					{
						int chan_num = view->GetAny();
						fluo::Color color(1.0, 1.0, 1.0);
						if (chan_num == 0)
							color = fluo::Color(1.0, 0.0, 0.0);
						else if (chan_num == 1)
							color = fluo::Color(0.0, 1.0, 0.0);
						else if (chan_num == 2)
							color = fluo::Color(0.0, 0.0, 1.0);

						if (chan_num >= 0 && chan_num < 3)
							vd_add->SetColor(color);

						view->AddVolumeData(vd_add);
					}
				}
			}
		}
		//update
		view->InitView(INIT_BOUNDS | INIT_CENTER | INIT_TRANSL | INIT_ROTATE);
	}

	//bring view state up to running
	glbin_coordinator.BringToState(view_agent, AgentState::Running);

	NotifyDataToUI({ gstRenderViewPanelLayout, gstMovViewList, gstMovViewIndex, gstTreeCtrl, gstUpdateSync });
}

void MainFrameAgent::LayoutRenderViewPanels()
{
	auto frame = GetMainFrame();
	if (!frame)
		return;

	frame->LayoutRenderViewPanels(
		frame->GetRenderViewLayout());
}

void MainFrameAgent::SyncRenderViewPanels()
{
	//update render view panels by views under root
}

void MainFrameAgent::ToggleScriptBreakDlg()
{
	auto mainframe = GetMainFrame();
	if (mainframe)
		mainframe->ToggleScriptBreakDlg();
}

void MainFrameAgent::LastOpen()
{
	unsigned int last_open = glbin_settings.m_last_open_type;
	switch (last_open)
	{
	case 0://volume
		OpenVolume();
		break;
	case 1://mesh
		OpenMesh();
		break;
	case 2://imagej
		ImportVolume();
		break;
	}
}

void MainFrameAgent::LastTool()
{
	unsigned int tool = glbin_settings.m_last_tool;
	switch (tool)
	{
	case 0:
	case TOOL_PAINT_BRUSH:
	default:
		ShowBrushDlg();
		break;
	case TOOL_MEASUREMENT:
		ShowMeasureDlg();
		break;
	case TOOL_TRACKING:
		ShowTrackDlg();
		break;
	case TOOL_NOISE_REDUCTION:
		ShowNoiseCancellingDlg();
		break;
	case TOOL_VOLUME_SIZE:
		ShowCountingDlg();
		break;
	case TOOL_COLOCALIZATION:
		ShowColocalizationDlg();
		break;
	case TOOL_CONVERT:
		ShowConvertDlg();
		break;
	case TOOL_OPENCL:
		ShowOclDlg();
		break;
	case TOOL_COMPONENT:
		ShowComponentDlg();
		break;
	case TOOL_CALCULATIONS:
		ShowCalculationDlg();
		break;
	case TOOL_MACHINE_LEARNING:
		ShowMachineLearningDlg();
		break;
	}
}

void MainFrameAgent::ToggleToolbar()
{
	auto frame = GetMainFrame();
	if (frame)
	{
		frame->ShowToolbar(!frame->GetToolbarShown());
		UpdateDataToUI({ gstMainToolbar });
	}
}

void MainFrameAgent::ToggleProjPanel()
{
	auto frame = GetMainFrame();
	if (frame)
	{
		frame->ShowToolbar(!frame->GetProjPanelShown());
		UpdateDataToUI({ gstProjPanel });
	}
}

void MainFrameAgent::ToggleMoviePanel()
{
	auto frame = GetMainFrame();
	if (frame)
	{
		frame->ShowToolbar(!frame->GetMoviePanelShown());
		UpdateDataToUI({ gstMoviePanel });
	}
}

void MainFrameAgent::ToggleOutAdjPanel()
{
	auto frame = GetMainFrame();
	if (frame)
	{
		frame->ShowToolbar(!frame->GetOutAdjPanelShown());
		UpdateDataToUI({ gstOutAdjPanel });
	}
}

void MainFrameAgent::ToggleClipPlanePanel()
{
	auto frame = GetMainFrame();
	if (frame)
	{
		frame->ShowToolbar(!frame->GetClipPlanePanelShown());
		UpdateDataToUI({ gstClipPlanePanel });
	}
}

void MainFrameAgent::TogglePropertyPanel()
{
	auto frame = GetMainFrame();
	if (frame)
	{
		frame->ShowToolbar(!frame->GetPropertyPanelShown());
		UpdateDataToUI({ gstPropertyPanel });
	}
}

void MainFrameAgent::ShowBrushDlg()
{
	glbin_settings.m_last_tool = TOOL_PAINT_BRUSH;
	auto frame = GetMainFrame();
	if (frame)
	{
		frame->ShowBrushDlg();
		frame->UpdateLastTool(MainFrame::ID_BrushDlg);
	}
}

void MainFrameAgent::ShowCalculationDlg()
{
	glbin_settings.m_last_tool = TOOL_CALCULATIONS;
	auto frame = GetMainFrame();
	if (frame)
	{
		frame->ShowCalculationDlg();
		frame->UpdateLastTool(MainFrame::ID_CalcDlg);
	}
}

void MainFrameAgent::ShowColocalizationDlg()
{
	glbin_settings.m_last_tool = TOOL_COLOCALIZATION;
	auto frame = GetMainFrame();
	if (frame)
	{
		frame->ShowColocalizationDlg();
		frame->UpdateLastTool(MainFrame::ID_ColocalDlg);
	}
}

void MainFrameAgent::ShowComponentDlg()
{
	glbin_settings.m_last_tool = TOOL_COMPONENT;
	auto frame = GetMainFrame();
	if (frame)
	{
		frame->ShowComponentDlg();
		frame->UpdateLastTool(MainFrame::ID_ComponentDlg);
	}
}

void MainFrameAgent::ShowConvertDlg()
{
	glbin_settings.m_last_tool = TOOL_CONVERT;
	auto frame = GetMainFrame();
	if (frame)
	{
		frame->ShowConvertDlg();
		frame->UpdateLastTool(MainFrame::ID_ConvertDlg);
	}
}

void MainFrameAgent::ShowCountingDlg()
{
	glbin_settings.m_last_tool = TOOL_VOLUME_SIZE;
	auto frame = GetMainFrame();
	if (frame)
	{
		frame->ShowCountingDlg();
		frame->UpdateLastTool(MainFrame::ID_CountDlg);
	}
}

void MainFrameAgent::ShowMachineLearningDlg()
{
	glbin_settings.m_last_tool = TOOL_MACHINE_LEARNING;
	auto frame = GetMainFrame();
	if (frame)
	{
		frame->ShowMachineLearningDlg();
		frame->UpdateLastTool(MainFrame::ID_MlDlg);
	}
}

void MainFrameAgent::ShowMeasureDlg()
{
	glbin_settings.m_last_tool = TOOL_MEASUREMENT;
	auto frame = GetMainFrame();
	if (frame)
	{
		frame->ShowMeasureDlg();
		frame->UpdateLastTool(MainFrame::ID_MeasureDlg);
	}
}

void MainFrameAgent::ShowNoiseCancellingDlg()
{
	glbin_settings.m_last_tool = TOOL_NOISE_REDUCTION;
	auto frame = GetMainFrame();
	if (frame)
	{
		frame->ShowNoiseCancellingDlg();
		frame->UpdateLastTool(MainFrame::ID_NoiseCancelDlg);
	}
}

void MainFrameAgent::ShowOclDlg()
{
	glbin_settings.m_last_tool = TOOL_OPENCL;
	auto frame = GetMainFrame();
	if (frame)
	{
		frame->ShowOclDlg();
		frame->UpdateLastTool(MainFrame::ID_OclDlg);
	}
}

void MainFrameAgent::ShowSettingDlg()
{
	auto frame = GetMainFrame();
	if (frame)
	{
		frame->ShowSettingDlg();
	}
}

void MainFrameAgent::ShowTrackDlg()
{
	glbin_settings.m_last_tool = TOOL_TRACKING;
	auto frame = GetMainFrame();
	if (frame)
	{
		frame->ShowTrackDlg();
		frame->UpdateLastTool(MainFrame::ID_TrackDlg);
	}
}

void MainFrameAgent::ShowAboutDlg()
{
	auto frame = GetMainFrame();
	if (frame)
		frame->ShowAboutDlg();
}

void MainFrameAgent::ShowScriptProcFileDlg()
{
	auto frame = GetMainFrame();
	if (!frame)
		return;

	auto info = glbin_script_proc.GetScriptDlgInfo();

	glbin_moviemaker.Hold();
	ModalDlg dlg(
		frame,
		info.title,
		"", "",
		info.wildcard,
		info.style);
	int rval = dlg.ShowModal();
	if (rval == wxID_OK)
	{
		info.valid = true;
		info.result = dlg.GetPath().ToStdWstring();
		glbin_script_proc.SetScriptDlgInfo(info);
	}

	glbin_moviemaker.Resume();
}

void MainFrameAgent::LayoutOutAdjPanel()
{
	auto frame = GetMainFrame();
	if (frame)
		frame->LayoutOutputAdjPanel();
}

void MainFrameAgent::ResetLayout()
{
	auto frame = GetMainFrame();
	if (!frame)
		return;

	frame->ResetLayout();
	glbin_settings.m_layout.clear();
	UpdateDataToUI({
		gstMainToolbar,
		gstProjPanel,
		gstMoviePanel,
		gstPropertyPanel,
		gstOutAdjPanel,
		gstClipPlanePanel,
		gstRenderViewPanelLayout,
		gstOutAdjPanelLayout });
}

void MainFrameAgent::ToggleFullscreen()
{
	auto frame = GetMainFrame();
	if (!frame)
		return;

	if (frame->IsFullScreen())
		frame->ShowFullScreen(false);
	else
		frame->ShowFullScreen(true,
			wxFULLSCREEN_NOBORDER |
			wxFULLSCREEN_NOCAPTION);

	UpdateDataToUI({ gstFullscreenMenu });
}

void MainFrameAgent::QuitCanvasFullscreen()
{
	auto frame = GetMainFrame();
	if (frame)
		frame->QuitCanvasFullscreen();
}

void MainFrameAgent::Close()
{
	auto frame = GetMainFrame();
	if (frame)
		frame->Close();
}

void MainFrameAgent::OpenVolume()
{
	auto frame = GetMainFrame();
	if (!frame)
		return;

	OpenVolumeOptions initial_options;
	initial_options.sliceSequence = glbin_settings.m_slice_sequence;
	initial_options.channelSequence = glbin_settings.m_chann_sequence;
	initial_options.digitOrder = glbin_settings.m_digit_order;
	initial_options.serialNumber = std::to_string(glbin_settings.m_ser_num);
	initial_options.compressData = glbin_settings.m_realtime_compress;
	initial_options.skipEmptyBricks = glbin_settings.m_skip_brick;
	initial_options.timeSequenceIdentifier = glbin_settings.m_time_id;
	OpenVolumeHook hook(initial_options);

	ModalDlg fopendlg(
		frame, "Choose the volume data file", "", "",
		"All Supported|*.tif;*.tiff;*.png;*.jpg;*.jpeg;*.jp2;*.lif;*.lof;*.nd2;*.oib;*.oif;*.xml;*.lsm;*.czi;*.dcm;*.dicom;*.nrrd;*.vvd;*.mp4;*.m4v;*.mov;*.avi;*.wmv|"\
		"Tiff Files (*.tif, *.tiff)|*.tif;*.tiff|"\
		"Png Files (*.png)|*.png|"\
		"Jpeg Files (*.jpg, *.jpeg)|*.jpg;*.jpeg|"\
		"Jpeg2000 Files (*.jp2)|*.jp2|"\
		"Leica Image File Format (*.lif)|*.lif|"\
		"Leica Microsystems Object File Format (*.lof)|*.lof|"\
		"Nikon ND2 File Format (*.nd2)|*.nd2|"\
		"Olympus Image Binary Files (*.oib)|*.oib|"\
		"Olympus Original Imaging Format (*.oif)|*.oif|"\
		"Bruker/Prairie View XML (*.xml)|*.xml|"\
		"Zeiss Laser Scanning Microscope (*.lsm)|*.lsm|"\
		"Zeiss ZISRAW File Format (*.czi)|*.czi|"\
		"DICOM files (*.dcm, *.dicom)|*.dcm;*.dicom|"\
		"Utah Nrrd files (*.nrrd)|*.nrrd|"\
		"Janelia Brick files (*.vvd)|*.vvd|"\
		"Video files (*.mp4, *.m4v, *.mov, *.avi, *.wmv)|*.mp4;*.m4v;*.mov;*.avi;*.wmv",
		wxFD_OPEN | wxFD_MULTIPLE);
	fopendlg.SetCustomizeHook(hook);

	int rval = fopendlg.ShowModal();
	if (rval == wxID_OK)
	{
		const OpenVolumeOptions& selected_options = hook.GetOptions();
		glbin_settings.m_slice_sequence = selected_options.sliceSequence;
		glbin_settings.m_chann_sequence = selected_options.channelSequence;
		glbin_settings.m_digit_order = selected_options.digitOrder;
		int ival = 0;
		if (TryToInt(selected_options.serialNumber, ival))
			glbin_settings.m_ser_num = ival;
		glbin_settings.m_realtime_compress = selected_options.compressData;
		glbin_settings.m_skip_brick = selected_options.skipEmptyBricks;
		glbin_settings.m_time_id = selected_options.timeSequenceIdentifier;

		wxArrayString paths;
		fopendlg.GetPaths(paths);
		std::vector<std::wstring> std_filenames;
		std_filenames.reserve(paths.size());
		for (const auto& filename : paths) {
			std_filenames.push_back(filename.ToStdWstring());
		}
		glbin_data_manager.LoadVolumes(std_filenames, false);
	}

	glbin_settings.m_last_open_type = 0;
	frame->UpdateLastOpen(MainFrame::ID_OpenVolume);
}

void MainFrameAgent::ImportVolume()
{
	auto frame = GetMainFrame();
	if (!frame)
		return;

	ImportVolumeOptions options;
	options.compressData = glbin_settings.m_realtime_compress;
	options.skipEmptyBricks = glbin_settings.m_skip_brick;
	ImportVolumeHook hook(options);

	ModalDlg fopendlg(
		frame, "Choose the volume data file", "", "", "All Files|*.*",
		wxFD_OPEN | wxFD_MULTIPLE);
	fopendlg.SetCustomizeHook(hook);

	int rval = fopendlg.ShowModal();
	if (rval == wxID_OK)
	{
		const auto& opts = hook.GetOptions();
		glbin_settings.m_realtime_compress =
			opts.compressData;
		glbin_settings.m_skip_brick =
			opts.skipEmptyBricks;

		wxArrayString paths;
		fopendlg.GetPaths(paths);
		std::vector<std::wstring> std_filenames;
		std_filenames.reserve(paths.size());
		for (const auto& filename : paths) {
			std_filenames.push_back(filename.ToStdWstring());
		}
		glbin_data_manager.LoadVolumes(std_filenames, true);
	}

	glbin_settings.m_last_open_type = 2;
	frame->UpdateLastOpen(MainFrame::ID_ImportVolume);
}

void MainFrameAgent::OpenMesh()
{
	auto frame = GetMainFrame();
	if (!frame)
		return;

	ModalDlg fopendlg(
		frame, "Choose the mesh data file",
		"", "", "*.obj", wxFD_OPEN | wxFD_MULTIPLE);

	int rval = fopendlg.ShowModal();
	if (rval == wxID_OK)
	{
		wxArrayString files;
		fopendlg.GetPaths(files);
		std::vector<std::wstring> std_filenames;
		std_filenames.reserve(files.size());
		for (const auto& filename : files) {
			std_filenames.push_back(filename.ToStdWstring());
		}
		glbin_data_manager.LoadMeshFiles(std_filenames);
	}

	glbin_settings.m_last_open_type = 1;
	frame->UpdateLastOpen(MainFrame::ID_OpenMesh);
}

void MainFrameAgent::NewProject()
{
	glbin_project.Reset();
	NotifyViewUpdate({ gstListCtrl, gstTreeCtrl, gstParamList });
}

void MainFrameAgent::OpenProject()
{
	auto frame = GetMainFrame();
	if (!frame)
		return;

	ModalDlg fopendlg(
		frame, "Choose Project File",
		"", "", "*.vrp", wxFD_OPEN);

	int rval = fopendlg.ShowModal();
	if (rval == wxID_OK)
	{
		std::wstring path = fopendlg.GetPath().ToStdWstring();
		glbin_project.Open(path);
		UpdateDataToUI({ gstProjectTitle });
	}
}

void MainFrameAgent::SaveProject()
{
	std::wstring default_path = glbin_data_manager.GetProjectFile();
	if (default_path.empty())
	{
		SaveAsProject();
	}
	else
	{
		bool inc = glbin_settings.m_prj_save_inc;
		glbin_project.Save(default_path, inc);
		UpdateDataToUI({ gstProjectTitle });
	}
}

void MainFrameAgent::SaveAsProject()
{
	auto frame = GetMainFrame();
	if (!frame)
		return;

	std::wstring default_path = glbin_data_manager.GetProjectFile();
	std::wstring path;
	std::wstring filename;
	bool default_valid = SEP_PATH_NAME(default_path, path, filename);
	SaveProjectOptions options;
	options.embedFiles = glbin_settings.m_vrp_embed;
	options.compressProject = glbin_settings.m_save_compress;
	SaveProjectHook hook(options);

	ModalDlg fopendlg(
		frame, "Save Project File",
		default_valid ? path : L"", default_valid ? filename : L"",
		"*.vrp", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	fopendlg.SetCustomizeHook(hook);

	int rval = fopendlg.ShowModal();
	if (rval == wxID_OK)
	{
		const auto& opts = hook.GetOptions();
		glbin_settings.m_vrp_embed =
			opts.embedFiles;
		glbin_settings.m_save_compress =
			opts.compressProject;

		filename = fopendlg.GetPath().ToStdWstring();
		glbin_project.Save(filename, false);
		UpdateDataToUI({ gstProjectTitle });
	}
}

