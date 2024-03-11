/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2018 Scientific Computing and Imaging Institute,
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

#ifndef _VRENDERFRAME_H_
#define _VRENDERFRAME_H_

#include <Value.hpp>
#include <wx/wx.h>
#include <wx/menu.h>
#include <wx/aui/aui.h>
#include <wx/aui/auibook.h>
#include <vector>

using namespace std;

#define VERSION_CONTACT "http://www.sci.utah.edu/software/fluorender.html"
#define VERSION_AUTHORS "YONG WAN\t\tHIDEO OTSUNA\nCHUCK HANSEN\tCHI-BIN CHIEN\n"\
						"BRIG BAGLEY\t\tTAKASHI KAWASE\nKEI ITO\t\tREMALDEEP SINGH"
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

#define UITEXT_DATAVIEW		"Datasets"
#define UITEXT_TREEVIEW		"Workspace"
#define UITEXT_MAKEMOVIE	"Record/Export"
#define UITEXT_ADJUST		"Output Adjustments"
#define UITEXT_CLIPPING		"Clipping Planes"
#define UITEXT_PROPERTIES	"Properties"

class TreePanel;
class ListPanel;
class VRenderView;
class VRenderGLView;
class DataGroup;
class VolumeData;
class MeshData;
class Annotations;
class PropPanel;
class VolumePropPanel;
class MeshPropPanel;
class AnnotatPropPanel;
class ManipPropPanel;
class AdjustView;
class VMovieView;
class SettingDlg;
class HelpDlg;
class ClippingView;
class BrushToolDlg;
class NoiseCancellingDlg;
class CountingDlg;
class ConvertDlg;
class ColocalizationDlg;
class RecorderDlg;
class MeasureDlg;
class TraceDlg;
class OclDlg;
class ComponentDlg;
class CalculationDlg;
class ScriptBreakDlg;
class MachineLearningDlg;
class TesterDlg;

class VRenderFrame: public wxFrame
{
	enum
	{
		//file
		//file\new
		ID_NewProject = ID_VRENDER_FRAME,
		ID_SaveProject,
		ID_SaveAsProject,
		//file\open
		ID_Open,
		ID_OpenProject,
		ID_OpenVolume,
		ID_OpenMesh,
		//ImageJ\open
		ID_ImportVolume,
		//undo/redo
		ID_Undo,
		ID_Redo,
		//view
		ID_FullScreen,
		ID_ViewNew,
		ID_Layout,
		ID_ShowHideUI,
		//tools
		ID_LastTool,
		ID_PaintTool,
		ID_Measure,
		ID_Trace,
		ID_NoiseCancelling,
		ID_Counting,
		ID_Colocalization,
		ID_Convert,
		ID_Ocl,
		ID_Component,
		ID_Calculations,
		ID_MachineLearning,
		//
		ID_Settings,
		//UI menu
		ID_UIListView,
		ID_UITreeView,
		ID_UIMovieView,
		ID_UIAdjView,
		ID_UIClipView,
		ID_UIPropView,
		ID_ViewOrganize,
		//right aligned items
		ID_CheckUpdates,
		ID_Youtube,
		ID_Facebook,
		ID_Manual,
		ID_Tutorial,
		ID_Twitter,
		ID_Info,
		ID_ShowHideToolbar
	};

public:
	VRenderFrame(wxFrame* frame,
		const wxString& title,
		int x, int y,
		int w, int h,
		bool benchmark,
		bool fullscreen,
		bool windowed,
		bool hidepanels);
	~VRenderFrame();

	TreePanel *GetTree();
	ListPanel *GetList();
	void UpdateTree(const wxString& name = "");
	void UpdateTreeColors();
	void UpdateTreeIcons();
	void UpdateList();

	//views
	int GetViewNum();
	VRenderGLView* GetView(int index);
	VRenderGLView* GetView(wxString& name);
	void RefreshVRenderViews(bool tree=false, bool interactive=false);
	void DeleteVRenderView(int i);
	void DeleteVRenderView(wxString &name);

	//organize render views
	void OrganizeVRenderViews(int mode);
	//hide/show tools
	void ToggleAllTools(bool cur_state);
	//show/hide panes
	void ShowPane(wxPanel* pane, bool show=true);

	//on selections
	void OnSelection(int type,	//0: nothing; 1:view; 2: volume; 3:mesh; 4:annotations; 5:group; 6:mesh manip
		VRenderGLView* view=0,
		DataGroup* group=0,
		VolumeData* vd=0,
		MeshData* md=0,
		Annotations* ann=0);

	wxWindow* AddProps(int type,//follow above
		VRenderGLView* view = 0,
		DataGroup* group = 0,
		VolumeData* vd = 0,
		MeshData* md = 0,
		Annotations* ann = 0);
	void DeleteProps(int type, const wxString& name);
	void ShowPropPage(int type,
		VRenderGLView* view = 0,
		DataGroup* group = 0,
		VolumeData* vd = 0,
		MeshData* md = 0,
		Annotations* ann = 0,
		bool show = true);
	void UpdateProps(const fluo::ValueCollection &vc, int excl_self = 1, PropPanel* panel = 0);
	VolumePropPanel* FindVolumeProps(VolumeData* vd);
	MeshPropPanel* FindMeshProps(MeshData* md);
	AnnotatPropPanel* FindAnnotationProps(Annotations* ad);
	ManipPropPanel* FindMeshManip(MeshData* md);
	VolumePropPanel* FindVolumeProps(const wxString& str);
	MeshPropPanel* FindMeshProps(const wxString& str);
	AnnotatPropPanel* FindAnnotationProps(const wxString& str);
	ManipPropPanel* FindMeshManip(const wxString& str);

	//prop view
	AdjustView* GetAdjustView();
	//movie view
	VMovieView* GetMovieView();
	//system settings
	SettingDlg* GetSettingDlg();
	//help dialog
	HelpDlg* GetHelpDlg();
	//clipping view
	ClippingView* GetClippingView();
	//brush dialog
	BrushToolDlg* GetBrushToolDlg();
	//noise cancelling dialog
	NoiseCancellingDlg* GetNoiseCancellingDlg();
	//counting dialog
	CountingDlg* GetCountingDlg();
	//convert dialog
	ConvertDlg* GetConvertDlg();
	ColocalizationDlg* GetColocalizationDlg();
	//recorder dialog
	RecorderDlg* GetRecorderDlg();
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

	//selection
	int GetCurSelType();
	VolumeData* GetCurSelVol();
	//get current selected mesh
	MeshData* GetCurSelMesh();

	void StartupLoad(wxArrayString files, bool run_mov, bool with_imagej);
	void OpenProject(wxString& filename);
	void SaveProject(wxString& filename, bool inc);//inc: save incrementally
	void LoadVolumes(wxArrayString files, bool withImageJ, VRenderGLView* view = 0);
	void LoadMeshes(wxArrayString files, VRenderGLView* view = 0);

	//show dialogs
	void ShowPaintTool();
	void ShowMeasureDlg();
	void ShowTraceDlg();
	void ShowNoiseCancellingDlg();
	void ShowCountingDlg();
	void ShowColocalizationDlg();
	void ShowConvertDlg();
	void ShowOclDlg();
	void ShowComponentDlg();
	void ShowCalculationDlg();
	void ShowMachineLearningDlg();
	void ShowScriptBreakDlg(bool show=true);

	//tex renderer settings
	void SetTextureRendererSettings();
	void SetTextureUndos();

	//quit option
	void OnQuit(wxCommandEvent& event);
	//show info
	void OnInfo(wxCommandEvent& event);

	bool GetBenchmark();

	void ClearVrvList();

	wxString ScriptDialog(const wxString& title,
		const wxString& wildcard, long style);

public: //public so export window can see it and set it. 
	RecorderDlg* m_recorder_dlg;
	VMovieView* m_movie_view;
	VolumeData* m_vd_copy;//for copying mask source
	bool m_copy_data;//copy data or mask

private:
	wxAuiManager m_aui_mgr;
	wxMenu* m_tb_menu_ui;
	wxMenu* m_tb_menu_edit;
	wxMenu* m_tb_menu_update;
	wxToolBar* m_main_tb;
	//main top menu
	wxMenuBar* m_top_menu;
	wxMenu* m_top_file;
	wxMenu* m_top_tools;
	wxMenu* m_top_window;
	wxMenu* m_top_help;

	TreePanel *m_tree_panel;
	ListPanel *m_list_panel;
	std::vector<VRenderView*> m_vrv_list;
	wxAuiNotebook *m_prop_panel;
	ClippingView *m_clip_view;
	AdjustView* m_adjust_view;
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
	//prop panel children
	std::vector<PropPanel*> m_prop_pages;
	//tester
	TesterDlg* m_tester;
	//flag for show/hide views
	bool m_ui_state;

	//current selection (allow only one)
	//selection type
	int m_cur_sel_type; //0:root; 1:view; 2:volume; 3:mesh; 5:volume group; 6:mesh group
	//current selected volume index
	int m_cur_sel_vol;
	//mesh index
	int m_cur_sel_mesh;

	//mac address
	wxString m_address;

	//benchmark mode
	bool m_benchmark;

	wxString m_title;

private:
	//views
	wxString CreateView(int row=1);
	VRenderGLView* GetLastView();
	static wxWindow* CreateExtraControlVolume(wxWindow* parent);
	static wxWindow* CreateExtraControlVolumeForImport(wxWindow* parent);
	static wxWindow* CreateExtraControlProjectSave(wxWindow* parent);

	//open dialog options
	void OnCh11Check(wxCommandEvent &event);
	void OnCh12Check(wxCommandEvent &event);
	void OnCmbChange(wxCommandEvent &event);
	void OnTxt1Change(wxCommandEvent &event);
	void OnTxt2Change(wxCommandEvent &event);
	void OnCh2Check(wxCommandEvent &event);
	void OnCh3Check(wxCommandEvent &event);
	void OnChEmbedCheck(wxCommandEvent &event);
	void OnChSaveCmpCheck(wxCommandEvent &event);

	void OnClose(wxCloseEvent &event);
	void OnExit(wxCommandEvent& event);
	void OnNewView(wxCommandEvent& event);
	void OnLayout(wxCommandEvent& event);
	void OnFullScreen(wxCommandEvent& event);
	void OnOpenVolume(wxCommandEvent& event);
	void OnImportVolume(wxCommandEvent& event);
	void OnOpenMesh(wxCommandEvent& event);
	void OnOrganize(wxCommandEvent& event);
	void OnCheckUpdates(wxCommandEvent& event);
	void OnFacebook(wxCommandEvent& event);
	void OnManual(wxCommandEvent& event);
	void OnTutorial(wxCommandEvent& event);
	void OnYoutube(wxCommandEvent& event);
	void OnTwitter(wxCommandEvent& event);
	void OnShowHideUI(wxCommandEvent& event);
	void OnShowHideToolbar(wxCommandEvent& event);
	void OnShowHideView(wxCommandEvent &event);

	//panes
	void OnPaneClose(wxAuiManagerEvent& event);

	//prop pages
	void OnPropPageClose(wxAuiNotebookEvent& event);

	//project
	void OnNewProject(wxCommandEvent& event);
	void OnSaveProject(wxCommandEvent& event);
	void OnSaveAsProject(wxCommandEvent& event);
	void OnOpenProject(wxCommandEvent& event);

	void OnSettings(wxCommandEvent& event);

	//undo redo
	void OnUndo(wxCommandEvent& event);
	void OnRedo(wxCommandEvent& event);

	//tools
	void OnLastTool(wxCommandEvent& event);
	void OnPaintTool(wxCommandEvent& event);
	void OnMeasure(wxCommandEvent& event);
	void OnTrace(wxCommandEvent& event);
	void OnNoiseCancelling(wxCommandEvent& event);
	void OnCounting(wxCommandEvent& event);
	void OnColocalization(wxCommandEvent& event);
	void OnConvert(wxCommandEvent& event);
	void OnOcl(wxCommandEvent& event);
	void OnComponent(wxCommandEvent& event);
	void OnCalculations(wxCommandEvent& event);
	void OnMachineLearning(wxCommandEvent& event);

	void OnDraw(wxPaintEvent& event);
	void OnKeyDown(wxKeyEvent& event);

	bool update_props(int excl_self, PropPanel* p1, PropPanel* p2);

	DECLARE_EVENT_TABLE()
};

#endif//_VRENDERFRAME_H_
