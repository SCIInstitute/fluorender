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

#ifndef _MAINFRAME_H_
#define _MAINFRAME_H_

#include <Value.hpp>
#include <wx/wx.h>
#include <wx/menu.h>
#include <wx/aui/aui.h>
#include <wx/aui/auibook.h>
#include <wx/aui/auibar.h>
#include <vector>
#include <memory>

#define VERSION_CONTACT "https://sciinstitute.github.io/fluorender/"
#define VERSION_AUTHORS "YONG WAN\t\tHIDEO OTSUNA\nCHUCK HANSEN\tCHI-BIN CHIEN\n"\
						"BRIG BAGLEY\tTAKASHI KAWASE\nKEI ITO\t\tREMALDEEP SINGH\n"\
						"HOLLY A. HOLMAN"
#define VERSION_UPDATES "https://sciinstitute.github.io/fluorender/Releases/fluorender_v" \
				   VERSION_MAJOR_TAG \
				   "." \
				   VERSION_MINOR_TAG \
				   "/"
#define HELP_MANUAL "https://github.com/SCIInstitute/fluorender/releases/download/v"\
				   VERSION_MAJOR_TAG \
				   "." \
				   VERSION_MINOR_TAG \
				   "/FluoRender" \
				   VERSION_MAJOR_TAG \
				   "." \
				   VERSION_MINOR_TAG \
				   "_Manual.pdf"
#define HELP_TUTORIAL "https://github.com/SCIInstitute/fluorender/releases/download/v"\
				   VERSION_MAJOR_TAG \
				   "." \
				   VERSION_MINOR_TAG \
				   "/FluoRender" \
				   VERSION_MAJOR_TAG \
				   "." \
				   VERSION_MINOR_TAG \
				   "_Tutorials.pdf"
#define YOUTUBE_URL "https://www.youtube.com/playlist?list=PLSBz7un7RyZhO51UWhmGiusaJebmaWPbc"
#define TWITTER_URL "https://twitter.com/FluoRender"
#define FACEBOOK_URL "https://www.facebook.com/fluorender"
#define BATCH_INFO HELP_MANUAL
#define HELP_PAINT HELP_MANUAL

#define UITEXT_PROJECT		"Project"
#define UITEXT_DATAVIEW		"Data Sets"
#define UITEXT_TREEVIEW		"Workspace"
#define UITEXT_MAKEMOVIE	"Movie Making"
#define UITEXT_ADJUST		"Output Adjustments"
#define UITEXT_CLIPPING		"Clipping Planes"
#define UITEXT_PROPERTIES	"Properties"

class ProjectPanel;
class TreePanel;
class ListPanel;
class RenderViewPanel;
class RenderView;
class DataGroup;
class VolumeData;
class MeshData;
class Annotations;
class PropPanel;
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

class MainFrame: public wxFrame
{
	enum
	{
		//toolbar
		ID_Null = 0,
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

public:
	MainFrame(wxFrame* frame,
		const wxString& title,
		int x, int y,
		int w, int h,
		bool benchmark,
		bool fullscreen,
		bool windowed,
		bool hidepanels);
	~MainFrame();

	void LoadPerspective(const wxString& str);
	wxString SavePerspective();

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

	//prop panels
	wxWindow* AddProps(int type,//follow above
		RenderView* view = 0,
		DataGroup* group = 0,
		VolumeData* vd = 0,
		MeshData* md = 0,
		Annotations* ann = 0);
	void DeleteProps(int type, const wxString& name);
	void ShowPropPage(int type,
		RenderView* view = 0,
		DataGroup* group = 0,
		VolumeData* vd = 0,
		MeshData* md = 0,
		Annotations* ann = 0,
		bool show = true);
	void UpdateProps(const fluo::ValueCollection &vc, int excl_self = 1, wxWindow* panel = 0);
	void FluoUpdate(const fluo::ValueCollection& vc);
	VolumePropPanel* FindVolumeProps(VolumeData* vd);
	MeshPropPanel* FindMeshProps(MeshData* md);
	AnnotatPropPanel* FindAnnotationProps(Annotations* ad);
	ManipPropPanel* FindMeshManip(MeshData* md);
	VolumePropPanel* FindVolumeProps(const wxString& str);
	MeshPropPanel* FindMeshProps(const wxString& str);
	AnnotatPropPanel* FindAnnotationProps(const wxString& str);
	ManipPropPanel* FindMeshManip(const wxString& str);

	//project panels
	ProjectPanel* GetProjectPanel();
	TreePanel* GetTreePanel();
	ListPanel* GetListPanel();
	//outadj view
	OutputAdjPanel* GetOutAdjPanel();
	//movie view
	MoviePanel* GetMoviePanel();
	//clipping view
	ClipPlanePanel* GetClipPlanePanel();
	//system settings
	SettingDlg* GetSettingDlg();
	//help dialog
	HelpDlg* GetHelpDlg();
	//brush dialog
	BrushToolDlg* GetBrushToolDlg();
	//noise cancelling dialog
	NoiseCancellingDlg* GetNoiseCancellingDlg();
	//counting dialog
	CountingDlg* GetCountingDlg();
	//convert dialog
	ConvertDlg* GetConvertDlg();
	ColocalizationDlg* GetColocalizationDlg();
	//measure dialog
	MeasureDlg* GetMeasureDlg();
	//trace dialog
	TrackDlg* GetTrackDlg();
	//ocl dialog
	OclDlg* GetOclDlg();
	//component dialog
	ComponentDlg* GetComponentDlg();
	//calculation dialog
	CalculationDlg* GetCalculationDlg();
	//script break dialog
	ScriptBreakDlg* GetScriptBreakDlg();
	//floating point voluem range
	FpRangeDlg* GetFpRangeDlg();
	//machine learning dialog
	MachineLearningDlg* GetMachineLearningDlg();

	//views
	void RefreshCanvases(const std::set<int>& canvases = {});//view indices to update
	wxString CreateRenderViewPanel(int row = 1);
	void DeleteRenderViewPanel(int i);
	void DeleteRenderViewPanel(const std::wstring& name);

	//hide/show tools
	void ToggleAllPanels(bool cur_state);
	void ToggleLastTool();
	void ShowPanel(wxPanel* pane, bool show = true);
	//main toolbar
	void ShowToolbar();
	//panels
	void ShowProjPanel();
	void ShowMoviePanel();
	void ShowOutAdjPanel();
	void ShowClipPlanePanel();
	void ShowPropPanel();
	//dialogs
	void ShowSettingDlg();
	void ShowBrushDlg();
	void ShowMeasureDlg();
	void ShowComponentDlg();
	void ShowTrackDlg();
	void ShowCalculationDlg();
	void ShowNoiseCancellingDlg();
	void ShowCountingDlg();
	void ShowColocalizationDlg();
	void ShowConvertDlg();
	void ShowOclDlg();
	void ShowMachineLearningDlg();
	void ShowScriptBreakDlg(bool show = true);
	void ShowInfo();
	std::wstring ScriptDialog(const std::wstring& title,
		const std::wstring& wildcard, long style);
	//organize render views
	void LayoutRenderViewPanels(int mode);
	//reset layout
	void ResetLayout();
	//fullscreen
	void FullScreen();

	//set status bar callback
	void SetProgress(int val, const std::string& str);

private:
	//aui manager
	wxAuiManager m_aui_mgr;
	//main toolbar
	wxAuiToolBar* m_main_tb;
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
	TreePanel *m_tree_panel;
	ListPanel *m_list_panel;
	std::vector<RenderViewPanel*> m_render_view_panels;
	PropertyPanel *m_prop_panel;
	//prop panel children
	std::vector<PropPanel*> m_prop_pages;
	ClipPlanePanel *m_clip_plane_panel;
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
	void OnClose(wxCloseEvent &event);

	//extra controls in dialogs
	static wxWindow* CreateExtraControlVolume(wxWindow* parent);
	static wxWindow* CreateExtraControlVolumeForImport(wxWindow* parent);
	static wxWindow* CreateExtraControlProjectSave(wxWindow* parent);
	//open dialog options
	void OnCh11Check(wxCommandEvent& event);
	void OnCh12Check(wxCommandEvent& event);
	void OnCmbChange(wxCommandEvent& event);
	void OnTxt1Change(wxCommandEvent& event);
	void OnTxt2Change(wxCommandEvent& event);
	void OnCh2Check(wxCommandEvent& event);
	void OnCh3Check(wxCommandEvent& event);
	void OnChEmbedCheck(wxCommandEvent& event);
	void OnChSaveCmpCheck(wxCommandEvent& event);
	void OnSysColorChanged(wxSysColourChangedEvent& event);

private:
	bool update_props(int excl_self, wxWindow* p1, wxWindow* p2);
};

#endif//_MAINFRAME_H_
