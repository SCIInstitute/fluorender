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

#ifndef _MAINFRAME_H_
#define _MAINFRAME_H_

#include <AgentOwner.h>
#include <Value.hpp>
#include <Version.h>
#include <Layout.h>
#include <wx/wx.h>
#include <wx/menu.h>
#include <wx/aui/aui.h>
#include <wx/aui/auibook.h>
#include <wx/aui/auibar.h>
#include <wx/filedlgcustomize.h>
#include <vector>
#include <memory>
#include <array>

struct OpenVolumeOptions
{
	bool sliceSequence = false;
	bool channelSequence = false;
	int digitOrder = 0; // 0=Channel first, 1=Z section first

	std::string serialNumber;

	bool compressData = false;
	bool skipEmptyBricks = false;

	std::wstring timeSequenceIdentifier;
};

class OpenVolumeHook :
	public wxFileDialogCustomizeHook
{
public:
	OpenVolumeHook(
		const OpenVolumeOptions& options) :
		m_options(options)
	{
	}

	void AddCustomControls(
		wxFileDialogCustomize& customizer) override;

	void TransferDataFromCustomControls() override;

	const OpenVolumeOptions& GetOptions() const
	{
		return m_options;
	}

private:
	OpenVolumeOptions m_options;

	wxFileDialogCheckBox* m_slice_chk = nullptr;
	wxFileDialogCheckBox* m_channel_chk = nullptr;

	wxFileDialogChoice* m_digit_order_choice = nullptr;

	wxFileDialogTextCtrl* m_serial_txt = nullptr;

	wxFileDialogCheckBox* m_compress_chk = nullptr;
	wxFileDialogCheckBox* m_skip_bricks_chk = nullptr;

	wxFileDialogTextCtrl* m_time_id_txt = nullptr;
};

struct ImportVolumeOptions
{
	bool compressData = false;
	bool skipEmptyBricks = false;
};

class ImportVolumeHook :
	public wxFileDialogCustomizeHook
{
public:
	ImportVolumeHook(
		const ImportVolumeOptions& options) :
		m_options(options)
	{
	}

	void AddCustomControls(
		wxFileDialogCustomize& customizer) override;

	void TransferDataFromCustomControls() override;

	const ImportVolumeOptions& GetOptions() const
	{
		return m_options;
	}

private:
	ImportVolumeOptions m_options;

	wxFileDialogCheckBox* m_compress_chk = nullptr;
	wxFileDialogCheckBox* m_skip_bricks_chk = nullptr;
};

struct SaveProjectOptions
{
	bool embedFiles = false;
	bool compressProject = false;
};

class SaveProjectHook :
	public wxFileDialogCustomizeHook
{
public:
	SaveProjectHook(
		const SaveProjectOptions& options) :
		m_options(options)
	{
	}

	void AddCustomControls(
		wxFileDialogCustomize& customizer) override;

	void TransferDataFromCustomControls() override;

	const SaveProjectOptions& GetOptions() const
	{
		return m_options;
	}

private:
	SaveProjectOptions m_options;

	wxFileDialogCheckBox* m_embed_chk = nullptr;
	wxFileDialogCheckBox* m_compress_chk = nullptr;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
class ProjectPanel;
class TreePanel;
class ListPanel;
class RenderViewPanel;
class RenderView;
class VolumeGroup;
class VolumeData;
class MeshData;
class MeshGroup;
class AnnotData;
class AgentPanel;
class PropertyPanel;
class VolumePropPanel;
class MeshPropPanel;
class AnnotatPropPanel;
class ManipPropPanel;
class OutputAdjPanel;
class MoviePanel;
class SettingDlg;
class HelpDlg;
class ClipPlanePanel;
class BrushToolDlg;
class NoiseCancellingDlg;
class CountingDlg;
class ConvertDlg;
class ColocalizationDlg;
class MeasureDlg;
class TrackDlg;
class OclDlg;
class ComponentDlg;
class CalculationDlg;
class ScriptBreakDlg;
class MachineLearningDlg;
class TesterDlg;
class FpRangeDlg;
class wxGaugeStatusbar;
class wxBasisSlider;

class MainFrame : public wxFrame, public AgentOwner
{
public:
	enum
	{
		//toolbar
		ID_Null = 0,
		ID_LastOpen,
		ID_OpenVolume,
		ID_ImportVolume,
		ID_OpenProject,
		ID_SaveProject,
		ID_ViewNew,
		ID_Panels,
		ID_OpenMesh,
		ID_LastTool,
		ID_Undo,
		ID_Redo,
		ID_Settings,
		ID_CheckUpdates,
		ID_Youtube,
		ID_Twitter,
		ID_Info,
		//toolbar menu items
		ID_ProjPanel,
		ID_MoviePanel,
		ID_OutAdjPanel,
		ID_ClipPlanePanel,
		ID_PropPanel,
		ID_BrushDlg,
		ID_MeasureDlg,
		ID_ComponentDlg,
		ID_TrackDlg,
		ID_CalcDlg,
		ID_NoiseCancelDlg,
		ID_CountDlg,
		ID_ColocalDlg,
		ID_ConvertDlg,
		ID_OclDlg,
		ID_MlDlg,
		//main menu items
		//file
		ID_NewProjMenu,
		ID_OpenVolumeMenu,
		ID_ImportVolumeMenu,
		ID_OpenMeshMenu,
		ID_OpenProjMenu,
		ID_SaveProjMenu,
		ID_SaveAsProjMenu,
		ID_Exit,
		//edit
		ID_UndoMenu,
		ID_RedoMenu,
		//tools
		ID_OclDlgMenu,
		ID_BrushDlgMenu,
		ID_MeasureDlgMenu,
		ID_CompDlgMenu,
		ID_TrackDlgMenu,
		ID_CalcDlgMenu,
		ID_NoiseCancelDlgMenu,
		ID_CountDlgMenu,
		ID_ColocalDlgMenu,
		ID_ConvertDlgMenu,
		ID_MlDlgMenu,
		//window
		ID_ToolbarMenu,
		ID_PanelsMenu,
		ID_ProjPanelMenu,
		ID_MoviePanelMenu,
		ID_OutAdjPanelMenu,
		ID_ClipPlanePanelMenu,
		ID_PropPanelMenu,
		ID_LayoutMenu,
		ID_ResetMenu,
		ID_ViewNewMenu,
		ID_FullscreenMenu,
		//help
		ID_CheckUpdatesMenu,
		ID_YoutubeMenu,
		ID_TwitterMenu,
		ID_FacebookMenu,
		ID_ManualMenu,
		ID_TutorialMenu,
		ID_InfoMenu
	};
	//extra options
	enum
	{
		ID_READ_ZSLICE = 0,
		ID_READ_CHANN,
		ID_DIGI_ORDER,
		ID_SER_NUM,
		ID_COMPRESS,
		ID_SKIP_BRICKS,
		ID_TSEQ_ID,
		ID_EMBED_FILES,
		ID_LZW_COMP
	};

	MainFrame(
		const wxString& title,
		int x, int y,
		int w, int h,
		int reset,
		bool benchmark,
		bool fullscreen,
		bool windowed,
		bool hidepanels);
	~MainFrame();

	virtual void CreateUI() override;

	void LoadPerspective(const std::string& str);
	std::string SavePerspective();

	//update
	void UpdateLastTool(int id);
	void UpdateLastOpen(int id);
	void UpdateProjPanel();
	void UpdateMoviePanel();
	void UpdatePropertyPanel();
	void UpdateOutAdjPanel();
	void UpdateClipPlanePanel();
	void UpdateMainToolbar();
	void UpdateMainFrameTitle(const std::wstring& title);
	void UpdateMainStatusbarText(const std::string& text);
	void PushMainStatusbarText(const std::string& text);
	void PopMainStatusbarText();
	void UpdateProgress(int val, const std::string& str);
	void UpdateFullscreenMenu();

	RenderViewPanel* CreateRenderViewPanel();
	void LayoutRenderViewPanels(RenderViewLayout layout);
	int DeleteRenderViewPanel(RenderViewPanel* panel);

	//get
	int GetReset() { return m_reset; }
	bool GetBenchmark() { return m_benchmark; }
	int GetDisplayId();
	std::array<int, 4> GetMainFrameRect();
	double GetMainMemorySizeInMB();
	RenderViewLayout GetRenderViewLayout() { return m_renderview_layout; }
	bool GetToolbarShown();
	bool GetProjPanelShown();
	bool GetMoviePanelShown();
	bool GetOutAdjPanelShown();
	bool GetClipPlanePanelShown();
	bool GetPropertyPanelShown();

	//startup maximize
	void StartupMaximize();

	//child panels/dlgs
	void ToggleScriptBreakDlg();
	void ToggleAllPanels(int mode);//0:hide;1:show;2:use current
	void ShowToolbar(bool bval);
	void ShowProjPanel(bool bval);
	void ShowMoviePanel(bool bval);
	void ShowOutAdjPanel(bool bval);
	void ShowClipPlanePanel(bool bval);
	void ShowPropertyPanel(bool bval);

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

	void LayoutOutputAdjPanel();
	void ResetLayout();


private:
	//cmd settings
	int m_reset{ false };
	bool m_benchmark{ false };
	bool m_fullscreen{ false };
	bool m_windowed{ true };
	bool m_hidepanels{ false };

	//layout
	RenderViewLayout m_renderview_layout{ RenderViewLayout::Horizontal };
	std::vector<RenderViewPanel*> m_renderview_panels;

	//aui manager
	wxAuiManager m_aui_mgr;
	//main toolbar
	wxAuiToolBar* m_main_tb;
	std::unique_ptr<wxMenu> m_tb_menu_open;
	std::unique_ptr<wxMenu> m_tb_menu_ui;
	std::unique_ptr<wxMenu> m_tb_menu_edit;
	std::unique_ptr<wxMenu> m_tb_menu_update;
	//main menu
	wxMenuBar* m_top_menu = 0;
	wxMenu* m_top_file = 0;
	wxMenu* m_top_edit = 0;
	wxMenu* m_top_tools = 0;
	wxMenu* m_top_window = 0;
	wxMenu* m_top_help = 0;

	ProjectPanel* m_proj_panel;
	TreePanel* m_tree_panel;
	ListPanel* m_list_panel;
	PropertyPanel* m_property_panel;
	ClipPlanePanel* m_clip_plane_panel;
	OutputAdjPanel* m_output_adj_panel;
	MoviePanel* m_movie_panel;
	SettingDlg* m_setting_dlg;
	HelpDlg* m_help_dlg;
	BrushToolDlg* m_brush_tool_dlg;
	NoiseCancellingDlg* m_noise_cancelling_dlg;
	CountingDlg* m_counting_dlg;
	ConvertDlg* m_convert_dlg;
	ColocalizationDlg* m_colocalization_dlg;
	MeasureDlg* m_measure_dlg;
	TrackDlg* m_track_dlg;
	OclDlg* m_ocl_dlg;
	ComponentDlg* m_component_dlg;
	CalculationDlg* m_calculation_dlg;
	MachineLearningDlg* m_machine_learning_dlg;
	ScriptBreakDlg* m_script_break_dlg;
	FpRangeDlg* m_fp_range_dlg;
	//tester
	TesterDlg* m_teser_dlg;

	//status
	wxGaugeStatusbar* m_statusbar;

	wxString m_title;
	//mac address
	wxString m_address;
	wxTimer m_waker;

private:
	//toolbar menus
	void OnToolbarMenu(wxAuiToolBarEvent& event);
	//main menu
	void OnMainMenu(wxCommandEvent& event);
	//panes
	void OnPaneClose(wxAuiManagerEvent& event);
	//close
	void OnClose(wxCloseEvent& event);
	void OnSysColorChanged(wxSysColourChangedEvent& event);
};

#endif//_MAINFRAME_H_
