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
#include <Global.h>
#include <Tokens.h>
#include <Root.h>
#include <DataManager.h>
#include <RenderView.h>
#include <Coordinator.h>
#include <RenderCanvas.h>
#include <RenderCanvasAgent.h>
#include <RenderViewPanelAgent.h>
#include <MainSettings.h>
#include <ScriptProc.h>
#include <CurrentObjects.h>
#include <GlobalStates.h>

MainFrameAgent::MainFrameAgent(
	MainFrame* frame) :
	Agent(frame)
{

}

void MainFrameAgent::UpdateUI(const UpdateRequest& request)
{
	if (request.HasValue(gstRenderViewPanel))
		SyncRenderViewPanels();
	if (request.HasValue(gstScriptBreakDlg))
		ToggleScriptBreakDlg();
	if (request.HasValue(gstMachineLearningDlg))
		ShowMachineLearningDlg();
	if (request.HasValue(gstManipPropPanel))
		ShowManipPropPanel();
	if (request.HasValue(gstLoadRenderViewSettings))
		LoadRenderViewSettings();
}

void MainFrameAgent::UpdateData(const UpdateRequest& request)
{

}

MainFrame* MainFrameAgent::GetMainFrame() const
{
	return static_cast<MainFrame*>(GetOwner());
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

void MainFrameAgent::Initialize()
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
	glbin.InitProgress(std::bind(&MainFrame::SetProgress, mainframe,
		std::placeholders::_1, std::placeholders::_2));

	glbin_states.m_benchmark = mainframe->GetBenchmark();

}

void MainFrameAgent::SyncRenderViewPanels()
{
	//update render view panels by views under root
}

void MainFrameAgent::LoadRenderViewSettings()
{
	for (auto renderview_agent : renderview_agent_list_)
	{
		if (renderview_agent)
			renderview_agent->LoadSettings();
	}
}

void MainFrameAgent::ToggleScriptBreakDlg()
{
	auto mainframe = GetMainFrame();
	if (mainframe)
		mainframe->ToggleScriptBreakDlg();
}

void MainFrameAgent::ShowMachineLearningDlg()
{

}

void MainFrameAgent::ShowManipPropPanel()
{

}
