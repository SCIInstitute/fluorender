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
#ifndef MainFrameAgent_h
#define MainFrameAgent_h

#include <Agent.h>
#include <Tokens.h>
#include <set>

class MainFrame;
class RenderViewPanelAgent;
class wxBasisSlider;
class MainFrameAgent : public Agent
{
public:
	MainFrameAgent(
		MainFrame* frame);

	virtual ~MainFrameAgent() = default;

	MainFrame* GetMainFrame() const;

	virtual void LoadSettings() override;
	virtual void ApplySettings() override;
	virtual void PreShutdown() override;

	void ShutdownAll();

	void SetFocusVRenderViews(wxBasisSlider* slider);
	void SetBenchmarkString(const std::string& str)
	{
		m_benchmark_str = str;
	}

	void AddRenderView(RenderViewPanelAgent* agent);
	void RemoveRenderView(RenderViewPanelAgent* agent);
	void DeleteRenderViewPanel(RenderViewPanelAgent* agent);

protected:
	std::span < const std::string_view>
		AcceptedValues() const override
	{
		return kAcceptedValues;
	}

	void UpdateUI(const UpdateRequest& request) override;

	void UpdateData(const UpdateRequest& request) override;

private:
	static constexpr std::string_view kAcceptedValues[] =
	{
		gstCurrentSelect,
	};

	std::set<RenderViewPanelAgent*> renderview_agent_list_;

	std::string m_benchmark_str;

private:
	void CreateRenderViewPanel();
	void LayoutRenderViewPanels();
	void SyncRenderViewPanels();
	void LayoutOutAdjPanel();
	void ToggleScriptBreakDlg();
	void LastOpen();
	void LastTool();

	void ToggleToolbar();

	//panels
	void ToggleProjPanel();
	void ToggleMoviePanel();
	void ToggleOutAdjPanel();
	void ToggleClipPlanePanel();
	void TogglePropertyPanel();

	//dialogs
	void ShowBrushDlg();
	void ShowCalculationDlg();
	void ShowColocalizationDlg();
	void ShowComponentDlg();
	void ShowConvertDlg();
	void ShowCountingDlg();
	void ShowMachineLearningDlg();
	void ShowMeasureDlg();
	void ShowNoiseCancellingDlg();
	void ShowOclDlg();
	void ShowSettingDlg();
	void ShowTrackDlg();

	void ShowAboutDlg();
	void ShowScriptProcFileDlg();

	//reset layout
	void ResetLayout();
	//fullscreen
	void ToggleFullscreen();

	//volume
	void OpenVolume();
	void ImportVolume();
	//mesh
	void OpenMesh();
	//project
	void NewProject();
	void OpenProject();
	void SaveProject();
	void SaveAsProject();
};

#endif // MainFrameAgent_h
