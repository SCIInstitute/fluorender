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

#include <FluiBuilder.h>
#include <Global.h>
#include <AnnotatPropPanel.h>
#include <AnnotatPropPanelAgent.h>
#include <BrushToolDlg.h>
#include <BrushToolDlgAgent.h>
#include <CalculationDlg.h>
#include <CalculationDlgAgent.h>
#include <ClipPlanePanel.h>
#include <ClipPlanePanelAgent.h>
#include <ColocalizationDlg.h>
#include <ColocalizationDlgAgent.h>
#include <ComponentDlg.h>
#include <ComponentDlgAgent.h>
#include <ConvertDlg.h>
#include <ConvertDlgAgent.h>
#include <CountingDlg.h>
#include <CountingDlgAgent.h>
#include <FpRangeDlg.h>
#include <FpRangeDlgAgent.h>
#include <ListPanel.h>
#include <ListPanelAgent.h>
#include <MachineLearningDlg.h>
#include <MachineLearningDlgAgent.h>
#include <MainFrame.h>
#include <MainFrameAgent.h>
#include <ManipPropPanel.h>
#include <ManipPropPanelAgent.h>
#include <MeasureDlg.h>
#include <MeasureDlgAgent.h>
#include <MeshPropPanel.h>
#include <MeshPropPanelAgent.h>
#include <MoviePanel.h>
#include <MoviePanelAgent.h>
#include <OclDlg.h>
#include <OclDlgAgent.h>
#include <OutputAdjPanel.h>
#include <OutputAdjPanelAgent.h>
#include <RenderCanvas.h>
#include <RenderCanvasAgent.h>
#include <RenderViewPanel.h>
#include <RenderViewPanelAgent.h>
#include <ScriptBreakDlg.h>
#include <ScriptBreakDlgAgent.h>
#include <SettingDlg.h>
#include <SettingDlgAgent.h>
#include <TrackDlg.h>
#include <TrackDlgAgent.h>
#include <TreePanel.h>
#include <TreePanelAgent.h>
#include <VolumePropPanel.h>
#include <VolumePropPanelAgent.h>

#include <CurrentObjects.h>
#include <DataManager.h>
#include <RenderView.h>
#include <Root.h>
#include <Coordinator.h>


#include <wx/glcanvas.h>

AnnotatPropPanel*
FluiBuilder::BuildAnnotatPropPanel(
	wxWindow* parent,
	const std::shared_ptr<AnnotData>& ann)
{
	return BuildUi<AnnotatPropPanel,
		AnnotatPropPanelAgent>(parent, ann);
}

BrushToolDlg* FluiBuilder::BuildBrushToolDlg(
	wxWindow* parent)
{
	return BuildUi<BrushToolDlg,
		BrushToolDlgAgent>(parent);
}

CalculationDlg* FluiBuilder::BuildCalculationDlg(
	wxWindow* parent)
{
	return BuildUi<CalculationDlg,
		CalculationDlgAgent>(parent);
}

ClipPlanePanel* FluiBuilder::BuildClipPlanePanel(
	wxWindow* parent,
	const std::shared_ptr<TreeLayer>& layer)
{
	return BuildUi<ClipPlanePanel,
		ClipPlanePanelAgent>(parent, layer);
}

ColocalizationDlg* FluiBuilder::BuildColocalizationDlg(
	wxWindow* parent)
{
	return BuildUi<ColocalizationDlg,
		ColocalizationDlgAgent>(parent);
}

ComponentDlg* FluiBuilder::BuildComponentDlg(
	wxWindow* parent)
{
	return BuildUi<ComponentDlg,
		ComponentDlgAgent>(parent);
}

ConvertDlg* FluiBuilder::BuildConvertDlg(
	wxWindow* parent)
{
	return BuildUi<ConvertDlg,
		ConvertDlgAgent>(parent);
}

CountingDlg* FluiBuilder::BuildCountingDlg(
	wxWindow* parent)
{
	return BuildUi<CountingDlg,
		CountingDlgAgent>(parent);
}

FpRangeDlg* FluiBuilder::BuildFpRangeDlg(
	wxWindow* parent)
{
	return BuildUi<FpRangeDlg,
		FpRangeDlgAgent>(parent);
}

ListPanel* FluiBuilder::BuildListPanel(
	wxWindow* parent)
{
	return BuildUi<ListPanel,
		ListPanelAgent>(parent);
}

MachineLearningDlg* FluiBuilder::BuildMachineLearningDlg(
	wxWindow* parent)
{
	return BuildUi<MachineLearningDlg,
		MachineLearningDlgAgent>(parent);
}

MainFrame* FluiBuilder::BuildMainFrame(
	const std::string& title,
	int x, int y,
	int w, int h,
	int reset,
	bool benchmark,
	bool fullscreen,
	bool windowed,
	bool hidepanels)
{
	auto frame = new MainFrame(
		title,
		x, y,
		w, h,
		reset,
		benchmark,
		fullscreen,
		windowed,
		hidepanels);
	
	auto agent = std::make_unique<MainFrameAgent>(frame);

	frame->SetAgent(std::move(agent));

	glbin_coordinator.Register(agent.get());
}

ManipPropPanel* FluiBuilder::BuildManipPropPanel(
	wxWindow* parent,
	const std::shared_ptr<MeshData>& md)
{
	return BuildUi<ManipPropPanel,
		ManipPropPanelAgent>(parent, md);
}

MeasureDlg* FluiBuilder::BuildMeasureDlg(
	wxWindow* parent)
{
	return BuildUi<MeasureDlg,
		MeasureDlgAgent>(parent);
}

MeshPropPanel* FluiBuilder::BuildMeshPropPanel(
	wxWindow* parent,
	const std::shared_ptr<MeshData>& md)
{
	return BuildUi<MeshPropPanel,
		MeshPropPanelAgent>(parent, md);
}

MoviePanel* FluiBuilder::BuildMoviePanel(
	wxWindow* parent)
{
	return BuildUi<MoviePanel,
		MoviePanelAgent>(parent);
}

OclDlg* FluiBuilder::BuildOclDlg(
	wxWindow* parent)
{
	return BuildUi<OclDlg,
		OclDlgAgent>(parent);
}

OutputAdjPanel* FluiBuilder::BuildOutputAdjPanel(
	wxWindow* parent)
{
	return BuildUi<OutputAdjPanel,
		OutputAdjPanelAgent>(parent);
}

RenderCanvas*
FluiBuilder::BuildRenderCanvas(
	wxWindow* parent,
	wxGLContext* sharedContext,
	const std::shared_ptr<RenderView>& view
)
{
	auto canvas = new RenderCanvas(parent, sharedContext);

	auto agent = std::make_unique<RenderCanvasAgent>(canvas, view);

	canvas->SetAgent(std::move(agent));
	
	glbin_coordinator.Register(agent.get());

	return canvas;
}

RenderViewPanel*
FluiBuilder::BuildRenderViewPanel(
	wxWindow* parent,
	wxGLContext* sharedContext)
{
	auto panel = new RenderViewPanel(parent, sharedContext);

	auto view = std::make_shared<RenderView>();

	auto agent = std::make_unique<RenderViewPanelAgent>(panel, view);
	panel->SetAgent(std::move(agent));
	glbin_coordinator.Register(agent.get());

	auto canvas = BuildRenderCanvas(panel, sharedContext, view);
	canvas->SetCanFocus(false);
#ifdef _WIN32
	view->SetHandle((void*)canvas->GetHWND());
#endif
	auto size = canvas->GetSize();
	view->SetSize(size.x, size.y);
	Root* root = glbin_data_manager.GetRoot();
	root->AddView(view);
	glbin_current.render_view_drawing = view;

	return panel;

}

ScriptBreakDlg* FluiBuilder::BuildScriptBreakDlg(
	wxWindow* parent)
{
	return BuildUi<ScriptBreakDlg,
		ScriptBreakDlgAgent>(parent);
}

SettingDlg* FluiBuilder::BuildSettingDlg(
	wxWindow* parent)
{
	return BuildUi<SettingDlg,
		SettingDlgAgent>(parent);
}

TrackDlg* FluiBuilder::BuildTrackDlg(
	wxWindow* parent)
{
	return BuildUi<TrackDlg,
		TrackDlgAgent>(parent);
}

TreePanel* FluiBuilder::BuildTreePanel(
	wxWindow* parent)
{
	return BuildUi<TreePanel,
		TreePanelAgent>(parent);
}

VolumePropPanel*
FluiBuilder::BuildVolumePropPanel(
	wxWindow* parent,
	const std::shared_ptr<VolumeData>& vd)
{
	return BuildUi<VolumePropPanel,
		VolumePropPanelAgent>(parent, vd);
}

template<
	class UiT,
	class AgentT,
	class... Args>
UiT* FluiBuilder::BuildUi(
	wxWindow* parent,
	Args&&... args)
{
	auto ui = new UiT(parent);

	auto agent =
		std::make_unique<AgentT>(
			ui,
			std::forward<Args>(args)...);

	auto* agent_ptr = agent.get();

	ui->SetAgent(std::move(agent));

	glbin_coordinator.Register(agent_ptr);

	return ui;
}
