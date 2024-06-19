/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2024 Scientific Computing and Imaging Institute,
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

#define VERSION_CONTACT "http://www.sci.utah.edu/software/fluorender.html"
#define VERSION_AUTHORS "YONG WAN\t\tHIDEO OTSUNA\nCHUCK HANSEN\tCHI-BIN CHIEN\n"\
						"BRIG BAGLEY\tTAKASHI KAWASE\nKEI ITO\t\tREMALDEEP SINGH"
#define VERSION_UPDATES "http://www.sci.utah.edu/releases/fluorender_v" \
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

class TreePanel;
class ListPanel;
class RenderViewPanel;
class RenderCanvas;
class DataGroup;
class VolumeData;
class MeshData;
class Annotations;
class PropPanel;
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
class TraceDlg;
class OclDlg;
class ComponentDlg;
class CalculationDlg;
class ScriptBreakDlg;
class MachineLearningDlg;
class TesterDlg;
class FpRangeDlg;

class MainFrame: public wxFrame
{
	enum
	{
		//toolbar
		ID_OpenVolume = 0,
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
		ID_BrushDlgMenu,
		ID_MeasureDlgMenu,
		ID_CompDlgMenu,
		ID_TrackDlgMenu,
		ID_CalcDlgMenu,
		ID_NoiseCancelDlgMenu,
		ID_CountDlgMenu,
		ID_ColocalDlgMenu,
		ID_ConvertDlgMenu,
		ID_OclDlgMenu,
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

	bool GetBenchmark();

	TreePanel *GetTree();
	ListPanel *GetList();

	//prop panels
	wxWindow* AddProps(int type,//follow above
		RenderCanvas* view = 0,
		DataGroup* group = 0,
		VolumeData* vd = 0,
		MeshData* md = 0,
		Annotations* ann = 0);
	void DeleteProps(int type, const wxString& name);
	void ShowPropPage(int type,
		RenderCanvas* view = 0,
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

	//prop view
	OutputAdjPanel* GetAdjustView();
	//movie view
	MoviePanel* GetMovieView();
	//system settings
	SettingDlg* GetSettingDlg();
	//help dialog
	HelpDlg* GetHelpDlg();
	//clipping view
	ClipPlanePanel* GetClippingView();
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
	TraceDlg* GetTraceDlg();
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

	//views
	void RefreshCanvases(const std::set<int>& views = {});//view indices to update
	int GetViewNum();
	RenderCanvas* GetRenderCanvas(int index);
	RenderCanvas* GetRenderCanvas(const wxString& name);
	int GetRenderCanvas(RenderCanvas* view);
	RenderCanvas* GetLastRenderCanvas();
	wxString CreateView(int row = 1);
	void DeleteVRenderView(int i);
	void DeleteVRenderView(const wxString& name);
	void ClearVrvList();

	//menu operations
	//volume
	void OpenVolume();
	void ImportVolume();
	void LoadVolumes(wxArrayString files, bool withImageJ, RenderCanvas* view = 0);
	void StartupLoad(wxArrayString files, bool run_mov, bool with_imagej);
	//mesh
	void OpenMesh();
	void LoadMeshes(wxArrayString files, RenderCanvas* view = 0);
	//project
	void NewProject();
	void OpenProject();
	void OpenProject(wxString& filename);
	void SaveProject();
	void SaveAsProject();
	void SaveProject(wxString& filename, bool inc);//inc: save incrementally
	//hide/show tools
	void ToggleAllPanels(bool cur_state);
	void ToggleLastTool();
	void ShowPane(wxPanel* pane, bool show = true);
	//show dialogs
	void ShowSettingDlg();
	void ShowBrushDlg();
	void ShowMeasureDlg();
	void ShowComponentDlg();
	void ShowTraceDlg();
	void ShowCalculationDlg();
	void ShowNoiseCancellingDlg();
	void ShowCountingDlg();
	void ShowColocalizationDlg();
	void ShowConvertDlg();
	void ShowOclDlg();
	void ShowMachineLearningDlg();
	void ShowScriptBreakDlg(bool show = true);
	void ShowInfo();
	wxString ScriptDialog(const wxString& title,
		const wxString& wildcard, long style);
	//panels
	void ShowProjPanel();
	void ShowMoviePanel();
	void ShowOutAdjPanel();
	void ShowClipPlanePanel();
	void ShowPropPanel();
	//main toolbar
	void ShowToolbar();
	//organize render views
	void OrganizeVRenderViews(int mode);
	//reset layout
	void ResetLayout();
	//fullscreen
	void FullScreen();

private:
	wxAuiManager m_aui_mgr;
	wxMenu* m_tb_menu_ui;
	wxMenu* m_tb_menu_edit;
	wxMenu* m_tb_menu_update;
	wxAuiToolBar* m_main_tb;
	//main top menu
	wxMenuBar* m_top_menu;
	wxMenu* m_top_file;
	wxMenu* m_top_edit;
	wxMenu* m_top_tools;
	wxMenu* m_top_window;
	wxMenu* m_top_help;

	wxAuiNotebook* m_proj_panel;
	TreePanel *m_tree_panel;
	ListPanel *m_list_panel;
	std::vector<RenderViewPanel*> m_vrv_list;
	wxAuiNotebook *m_prop_panel;
	ClipPlanePanel *m_clip_view;
	OutputAdjPanel* m_adjust_view;
	SettingDlg* m_setting_dlg;
	HelpDlg* m_help_dlg;
	BrushToolDlg* m_brush_tool_dlg;
	NoiseCancellingDlg* m_noise_cancelling_dlg;
	CountingDlg* m_counting_dlg;
	ConvertDlg* m_convert_dlg;
	ColocalizationDlg* m_colocalization_dlg;
	MeasureDlg* m_measure_dlg;
	TraceDlg* m_trace_dlg;
	OclDlg* m_ocl_dlg;
	ComponentDlg* m_component_dlg;
	CalculationDlg* m_calculation_dlg;
	MachineLearningDlg* m_machine_learning_dlg;
	ScriptBreakDlg* m_script_break_dlg;
	FpRangeDlg* m_fp_range_dlg;
	MoviePanel* m_movie_panel;
	//prop panel children
	std::vector<PropPanel*> m_prop_pages;
	//tester
	TesterDlg* m_tester;

	//mac address
	wxString m_address;

	//benchmark mode
	bool m_benchmark;

	wxString m_title;

	wxTimer* m_waker;

private:
	//toolbar menus
	void OnToolbarMenu(wxAuiToolBarEvent& event);
	//main menu
	void OnMainMenu(wxCommandEvent& event);
	//panes
	void OnPaneClose(wxAuiManagerEvent& event);
	//prop pages
	void OnPropPageClose(wxAuiNotebookEvent& event);
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

private:
	bool update_props(int excl_self, wxWindow* p1, wxWindow* p2);
};

#endif//_MAINFRAME_H_
