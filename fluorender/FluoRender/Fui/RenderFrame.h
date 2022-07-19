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

#ifndef _VRENDERFRAME_H_
#define _VRENDERFRAME_H_

#include <wx/wx.h>
#include <wx/menu.h>
#include <wx/aui/aui.h>
#include <TreePanel.h>
#include <ListPanel.h>
#include <RenderviewPanel.h>
#include <RenderCanvas.h>
#include <VolumePropPanel.h>
#include <MeshPropPanel.h>
#include <AnnotationPropPanel.h>
#include <MeshTransPanel.h>
#include <MoviePanel.h>
#include <ClipPlanePanel.h>
#include <OutAdjustPanel.h>
#include <SettingDlg.h>
#include <HelpDlg.h>
#include <BrushToolDlg.h>
#include <NoiseReduceDlg.h>
#include <CountingDlg.h>
#include <ConvertDlg.h>
#include <ColocalDlg.h>
#include <RecorderDlg.h>
#include <MeasureDlg.h>
#include <TrackDlg.h>
#include <ClKernelDlg.h>
#include <ComponentDlg.h>
#include <CalculationDlg.h>
#include <Tester.h>
#include <compatibility.h>
#include <RenderFrameAgent.hpp>
#include <vector>

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

namespace fluo
{
	class Root;
}
class RenderFrame: public wxFrame
{
	enum
	{
		//file
		//file\new
		ID_Save = ID_VRENDER_FRAME,
		ID_SaveProject,
		//file\open
		ID_Open,
		ID_OpenProject,
		ID_OpenVolume,
		ID_OpenMesh,
		//ImageJ\open
		ID_ImportVolume,
		//Mesh
		//Mesh\Create
		ID_Create,
		ID_CreateCube,
		ID_CreateSphere,
		ID_CreateCone,
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
	RenderFrame(
		const wxString& title,
		int x, int y,
		int w, int h);
	~RenderFrame();

	void Init(
		bool benchmark,
		bool fullscreen,
		bool windowed,
		bool hidepanels,
		bool imagej);

	//void AssociateRoot();

	//views
	void RefreshVRenderViews(bool tree=false, bool interactive=false);
	void DeleteVRenderView(int i);
	void DeleteVRenderView(const wxString &name);

	//organize render views
	void OrganizeVRenderViews(int mode);
	//hide/show tools
	void ToggleAllTools(bool cur_state);
	//show/hide panes
	void ShowPane(wxPanel* pane, bool show=true);

	//on selections
	void OnSelection(const std::string& name);

	RenderviewPanel* GetRenderview(int i)
	{
		if (i >= 0 && i < m_vrv_list.size())
			return m_vrv_list[i];
		return nullptr;
	}
	TreePanel *GetTree() { return m_tree_panel; }
	ListPanel *GetList() { return m_list_panel; }
	//prop view
	OutAdjustPanel* GetAdjustView() { return m_adjust_view; }
	//tool views
	VolumePropPanel* GetPropView() { return m_volume_prop; }
	//movie view
	MoviePanel* GetMovieView() { return m_movie_view; }
	//system settings
	SettingDlg* GetSettingDlg() { return m_setting_dlg; }
	//help dialog
	HelpDlg* GetHelpDlg() { return m_help_dlg; }
	//clipping view
	ClipPlanePanel* GetClippingView() { return m_clip_view; }
	//brush dialog
	BrushToolDlg* GetBrushToolDlg() { return m_brush_tool_dlg; }
	//noise cancelling dialog
	NoiseReduceDlg* GetNoiseCancellingDlg() { return m_noise_cancelling_dlg; }
	//counting dialog
	CountingDlg* GetCountingDlg() { return m_counting_dlg; }
	//convert dialog
	ConvertDlg* GetConvertDlg() { return m_convert_dlg; }
	//colocalization analysis dialog
	ColocalDlg* GetColocalizationDlg() { return m_colocalization_dlg; }
	//recorder dialog
	RecorderDlg* GetRecorderDlg() { return m_recorder_dlg; }
	//measure dialog
	MeasureDlg* GetMeasureDlg() { return m_measure_dlg; }
	//trace dialog
	TrackDlg* GetTraceDlg() { return m_trace_dlg; }
	//ocl dialog
	ClKernelDlg* GetOclDlg() { return m_ocl_dlg; }
	//component dialog
	ComponentDlg* GetComponentDlg() { return m_component_dlg; }
	//calculation dialog
	CalculationDlg* GetCalculationDlg() { return m_calculation_dlg; }
	//annotation
	AnnotationPropPanel* GetAnnotationPropPanel() { return m_annotation_prop; }

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

	//tex renderer settings
	//void SetTextureRendererSettings();
	//void SetTextureUndos();

	//quit option
	void OnQuit(wxCommandEvent& WXUNUSED(event))
	{ Close(true); }
	//show info
	void OnInfo(wxCommandEvent& WXUNUSED(event));

	void ClearVrvList()
	{ m_vrv_list.clear(); }

	wxString ScriptDialog(const wxString& title,
		const wxString& wildcard, long style);

	friend class fluo::RenderFrameAgent;
	fluo::RenderFrameAgent* m_agent;

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

	std::vector <RenderviewPanel*> m_vrv_list;
	TreePanel *m_tree_panel;
	ListPanel *m_list_panel;
	wxPanel *m_prop_panel;
	ClipPlanePanel *m_clip_view;
	OutAdjustPanel* m_adjust_view;
	SettingDlg* m_setting_dlg;
	HelpDlg* m_help_dlg;
	BrushToolDlg* m_brush_tool_dlg;
	NoiseReduceDlg* m_noise_cancelling_dlg;
	CountingDlg* m_counting_dlg;
	ConvertDlg* m_convert_dlg;
	ColocalDlg* m_colocalization_dlg;
	MeasureDlg* m_measure_dlg;
	TrackDlg* m_trace_dlg;
	ClKernelDlg* m_ocl_dlg;
	ComponentDlg* m_component_dlg;
	CalculationDlg* m_calculation_dlg;
	//prop panel children
	wxBoxSizer* m_prop_sizer;
	VolumePropPanel* m_volume_prop;
	MeshPropPanel* m_mesh_prop;
	MeshTransPanel* m_mesh_manip;
	AnnotationPropPanel* m_annotation_prop;
	RecorderDlg* m_recorder_dlg;
	MoviePanel* m_movie_view;
	//tester
	TesterDlg* m_tester;
	//flag for show/hide views
	bool m_ui_state;

private:
	//views
	wxString CreateView(int row=1);
	//RenderCanvas* GetLastView() {return m_vrv_list[m_vrv_list.size()-1]->m_glview;}
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
	void OnExit(wxCommandEvent& WXUNUSED(event));
	void OnNewView(wxCommandEvent& WXUNUSED(event));
	void OnLayout(wxCommandEvent& WXUNUSED(event));
	void OnFullScreen(wxCommandEvent& WXUNUSED(event));
	void OnOpenVolume(wxCommandEvent& WXUNUSED(event));
	void OnImportVolume(wxCommandEvent& WXUNUSED(event));
	void OnOpenMesh(wxCommandEvent& WXUNUSED(event));
	void OnOrganize(wxCommandEvent& WXUNUSED(event));
	void OnCheckUpdates(wxCommandEvent& WXUNUSED(event));
	void OnFacebook(wxCommandEvent& WXUNUSED(event));
	void OnManual(wxCommandEvent& WXUNUSED(event));
	void OnTutorial(wxCommandEvent& WXUNUSED(event));
	void OnYoutube(wxCommandEvent& WXUNUSED(event));
	void OnTwitter(wxCommandEvent& WXUNUSED(event));
	void OnShowHideUI(wxCommandEvent& WXUNUSED(event));
	void OnShowHideToolbar(wxCommandEvent& WXUNUSED(event));
	void OnShowHideView(wxCommandEvent &event);

	//panes
	void OnPaneClose(wxAuiManagerEvent& event);

	//test
	void OnCreateCube(wxCommandEvent& WXUNUSED(event));
	void OnCreateSphere(wxCommandEvent& WXUNUSED(event));
	void OnCreateCone(wxCommandEvent& WXUNUSED(event));

	void OnSaveProject(wxCommandEvent& WXUNUSED(event));
	void OnOpenProject(wxCommandEvent& WXUNUSED(event));

	void OnSettings(wxCommandEvent& WXUNUSED(event));
	//tools
	void OnLastTool(wxCommandEvent& WXUNUSED(event));
	void OnPaintTool(wxCommandEvent& WXUNUSED(event));
	void OnMeasure(wxCommandEvent& WXUNUSED(event));
	void OnTrace(wxCommandEvent& WXUNUSED(event));
	void OnNoiseCancelling(wxCommandEvent& WXUNUSED(event));
	void OnCounting(wxCommandEvent& WXUNUSED(event));
	void OnColocalization(wxCommandEvent& WXUNUSED(event));
	void OnConvert(wxCommandEvent& WXUNUSED(event));
	void OnOcl(wxCommandEvent& WXUNUSED(event));
	void OnComponent(wxCommandEvent& WXUNUSED(event));
	void OnCalculations(wxCommandEvent& WXUNUSED(event));

	void OnDraw(wxPaintEvent& event);
	void OnKeyDown(wxKeyEvent& event);

	DECLARE_EVENT_TABLE()
};

#endif//_VRENDERFRAME_H_
