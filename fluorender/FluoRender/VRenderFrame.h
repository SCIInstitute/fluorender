/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2014 Scientific Computing and Imaging Institute,
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
#include "DataManager.h"
#include "TreePanel.h"
#include "ListPanel.h"
#include "VRenderView.h"
#include "VPropView.h"
#include "MPropView.h"
#include "APropView.h"
#include "MManipulator.h"
#include "VMovieView.h"
#include "ClippingView.h"
#include "AdjustView.h"
#include "SettingDlg.h"
#include "HelpDlg.h"
#include "BrushToolDlg.h"
#include "NoiseCancellingDlg.h"
#include "CountingDlg.h"
#include "ConvertDlg.h"
#include "ColocalizationDlg.h"
#include "RecorderDlg.h"
#include "MeasureDlg.h"
#include "TraceDlg.h"
#include "OclDlg.h"
#include "ComponentDlg.h"
#include "CalculationDlg.h"
#include "Tester.h"
#include "Animator/Interpolator.h"
#include "compatibility.h"
#include "JVMInitializer.h"

#include <wx/wx.h>
#include <wx/menu.h>
#include <wx/aui/aui.h>

#include <vector>

#ifndef _VRENDERFRAME_H_
#define _VRENDERFRAME_H_

using namespace std;

#define VERSION_CONTACT "http://www.sci.utah.edu/software/fluorender.html"
#define VERSION_AUTHORS "YONG WAN\t\tHIDEO OTSUNA\nCHUCK HANSEN\tCHI-BIN CHIEN\n"\
						"BRIG BAGLEY\t\tTAKASHI KAWASE\nKEI ITO"
#define VERSION_UPDATES "http://www.sci.utah.edu/releases/fluorender_v" \
	               VERSION_MAJOR_TAG \
				   "." \
				   VERSION_MINOR_TAG \
				   "/"
#define HELP_MANUAL "http://www.sci.utah.edu/releases/fluorender_v"\
	               VERSION_MAJOR_TAG \
				   "." \
				   VERSION_MINOR_TAG \
				   "/FluoRender" \
	               VERSION_MAJOR_TAG \
				   "." \
				   VERSION_MINOR_TAG \
				   "_Manual.pdf"
#define HELP_TUTORIAL "http://www.sci.utah.edu/releases/fluorender_v"\
	               VERSION_MAJOR_TAG \
				   "." \
				   VERSION_MINOR_TAG \
				   "/FluoRender" \
	               VERSION_MAJOR_TAG \
				   "." \
				   VERSION_MINOR_TAG \
				   "_Tutorials.pdf"
#define BATCH_INFO HELP_MANUAL
#define HELP_PAINT HELP_MANUAL

#define UITEXT_DATAVIEW		"Datasets"
#define UITEXT_TREEVIEW		"Workspace"
#define UITEXT_MAKEMOVIE	"Record/Export"
#define UITEXT_ADJUST		"Output Adjustments"
#define UITEXT_CLIPPING		"Clipping Planes"
#define UITEXT_PROPERTIES	"Properties"

class VRenderFrame: public wxFrame
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
	void UpdateTree(wxString name = "");
	void UpdateTreeColors();
	void UpdateTreeIcons();
	void UpdateList();

	//data manager
	DataManager* GetDataManager();
	
	//views
	int GetViewNum();
	vector <VRenderView*>* GetViewList();
	VRenderView* GetView(int index);
	VRenderView* GetView(wxString& name);
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
		VRenderView* vrv=0,
		DataGroup* group=0,
		VolumeData* vd=0,
		MeshData* md=0,
		Annotations* ann=0);

	//settings
	//make movie settings
	int m_mov_view;
	int m_mov_axis;	//1 for x; 2 for y
	bool m_mov_rewind;
	wxString m_mov_angle_start;
	wxString m_mov_angle_end;
	wxString m_mov_step;
	wxString m_mov_frames;

	//prop view
	AdjustView* GetAdjustView();
	//tool views
	VPropView* GetPropView()
	{ return m_volume_prop; }
	//movie view
	VMovieView* GetMovieView()
	{ return m_movie_view; }
	//system settings
	SettingDlg* GetSettingDlg()
	{ return m_setting_dlg; }
	//help dialog
	HelpDlg* GetHelpDlg()
	{ return m_help_dlg; }
	//clipping view
	ClippingView* GetClippingView()
	{ return m_clip_view; }
	//brush dialog
	BrushToolDlg* GetBrushToolDlg()
	{ return m_brush_tool_dlg; }
	//noise cancelling dialog
	NoiseCancellingDlg* GetNoiseCancellingDlg()
	{ return m_noise_cancelling_dlg; }
	//counting dialog
	CountingDlg* GetCountingDlg()
	{ return m_counting_dlg; }
	//convert dialog
	ConvertDlg* GetConvertDlg()
	{ return m_convert_dlg; }
	ColocalizationDlg* GetColocalizationDlg()
	{ return m_colocalization_dlg; }
	//recorder dialog
	RecorderDlg* GetRecorderDlg()
	{ return m_recorder_dlg; }
	//measure dialog
	MeasureDlg* GetMeasureDlg()
	{ return m_measure_dlg; }
	//trace dialog
	TraceDlg* GetTraceDlg()
	{ return m_trace_dlg; }
	//ocl dialog
	OclDlg* GetOclDlg()
	{ return m_ocl_dlg; }
	//component dialog
	ComponentDlg* GetComponentDlg()
	{ return m_component_dlg; }
	//calculation dialog
	CalculationDlg* GetCalculationDlg()
	{ return m_calculation_dlg; }

	//selection
	int GetCurSelType()
	{ return m_cur_sel_type; }
	//get current selected volume
	VolumeData* GetCurSelVol()
	{ return m_data_mgr.GetVolumeData(m_cur_sel_vol); }
	//get current selected mesh
	MeshData* GetCurSelMesh()
	{ return m_data_mgr.GetMeshData(m_cur_sel_mesh); }

	void StartupLoad(wxArrayString files);
	void OpenProject(wxString& filename);
	void SaveProject(wxString& filename);
	void LoadVolumes(wxArrayString files, bool withImageJ, VRenderView* view = 0);
	void LoadMeshes(wxArrayString files, VRenderView* view = 0);

	//compression
	static void SetCompression(bool value)
	{ m_save_compress = value; }
	static bool GetCompression()
	{ return m_save_compress; }
	//realtime compression
	static void SetRealtimeCompression(bool value)
	{ m_compression = value; }
	static bool GetRealtimeCompression()
	{ return m_compression; }
	//skip brick
	static void SetSkipBrick(bool value)
	{ m_skip_brick = value; }
	static bool GetSkipBrick()
	{ return m_skip_brick;}
	//save project
	static void SetSaveProject(bool value)
	{ m_save_project = value; }
	static bool GetSaveProject()
	{ return m_save_project; }
	//embed project
	static void SetEmbedProject(bool value)
	{ m_vrp_embed = value; }
	static bool GetEmbedProject()
	{ return m_vrp_embed; }
	//save alpha
	static void SetSaveAlpha(bool value)
	{ m_save_alpha = value; }
	static bool GetSaveAlpha()
	{ return m_save_alpha; }

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

	//get interpolator
	Interpolator* GetInterpolator()
	{ return &m_interpolator; }

	//tex renderer settings
	void SetTextureRendererSettings();
	void SetTextureUndos();

	//quit option
	void OnQuit(wxCommandEvent& WXUNUSED(event))
	{ Close(true); }
	//show info
	void OnInfo(wxCommandEvent& WXUNUSED(event));

	bool GetBenchmark()
	{ return m_benchmark; }

	void ClearVrvList()
	{ m_vrv_list.clear(); }

public: //public so export window can see it and set it. 
	RecorderDlg* m_recorder_dlg;
	VMovieView* m_movie_view;

private:
	wxAuiManager m_aui_mgr;
	wxMenu* m_tb_menu_ui;
	wxMenu* m_tb_menu_edit;
	wxToolBar* m_main_tb;
	//main top menu
	wxMenuBar* m_top_menu;
	wxMenu* m_top_file;
	wxMenu* m_top_tools;
	wxMenu* m_top_window;
	wxMenu* m_top_help;

	TreePanel *m_tree_panel;
	ListPanel *m_list_panel;
	vector <VRenderView*> m_vrv_list;
	DataManager m_data_mgr;
	wxPanel *m_prop_panel;
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
	//prop panel children
	wxBoxSizer* m_prop_sizer;
	VPropView* m_volume_prop;
	MPropView* m_mesh_prop;
	MManipulator* m_mesh_manip;
	APropView* m_annotation_prop;
	//tester
	TesterDlg* m_tester;
	//flag for show/hide views
	bool m_ui_state;
	//interpolator
	//it stores all keys
	//and does interpolaions too
	Interpolator m_interpolator;

	//current selection (allow only one)
	//selection type
	int m_cur_sel_type; //0:root; 1:view; 2:volume; 3:mesh; 5:volume group; 6:mesh group
	//current selected volume index
	int m_cur_sel_vol;
	//mesh index
	int m_cur_sel_mesh;

	//if slices are sequence
	static bool m_sliceSequence;
	//compression
	static bool m_compression;
	//brick skipping
	static bool m_skip_brick;
	//string for time id
	static wxString m_time_id;
	//load volume mask
	static bool m_load_mask;
	//save compressed
	static bool m_save_compress;
	//embed files in project
	static bool m_vrp_embed;
	//save project
	static bool m_save_project;
	//save alpha
	static bool m_save_alpha;

	//mac address
	wxString m_address;

	//benchmark mode
	bool m_benchmark;

private:
	//views
	wxString CreateView(int row=1);
	VRenderView* GetLastView() {return m_vrv_list[m_vrv_list.size()-1];}
	static wxWindow* CreateExtraControlVolume(wxWindow* parent);
	static wxWindow* CreateExtraControlVolumeForImport(wxWindow* parent);
	static wxWindow* CreateExtraControlProjectSave(wxWindow* parent);

	//open dialog options
	void OnCh1Check(wxCommandEvent &event);
	void OnTxt1Change(wxCommandEvent &event);
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
