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
#include <MainFrame.h>
#include <Global.h>
#include <compatibility.h>
#include <DragDrop.h>
#include <TreePanel.h>
#include <ListPanel.h>
#include <RenderViewPanel.h>
#include <RenderCanvas.h>
#include <PropPanel.h>
#include <VolumePropPanel.h>
#include <MeshPropPanel.h>
#include <AnnotatPropPanel.h>
#include <ManipPropPanel.h>
#include <MoviePanel.h>
#include <ClipPlanePanel.h>
#include <OutputAdjPanel.h>
#include <SettingDlg.h>
#include <HelpDlg.h>
#include <BrushToolDlg.h>
#include <NoiseCancellingDlg.h>
#include <CountingDlg.h>
#include <ConvertDlg.h>
#include <ColocalizationDlg.h>
#include <MeasureDlg.h>
#include <TraceDlg.h>
#include <OclDlg.h>
#include <ComponentDlg.h>
#include <CalculationDlg.h>
#include <MachineLearningDlg.h>
#include <ScriptBreakDlg.h>
#include <AsyncTimerFactory.hpp>
#include <Tester.h>
#include <JVMInitializer.h>
#include <Formats/png_resource.h>
#include <Formats/msk_writer.h>
#include <Formats/msk_reader.h>
#include <Converters/VolumeMeshConv.h>
#include <wx/artprov.h>
#include <wx/wfstream.h>
#include <wx/fileconf.h>
#include <wx/aboutdlg.h>
#include <wx/progdlg.h>
#include <wx/hyperlink.h>
#include <wx/stdpaths.h>
#include <wx/accel.h>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <cctype>

//resources
#include <img/icons.h>

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
	EVT_MENU(wxID_EXIT, MainFrame::OnExit)
	EVT_MENU(ID_ViewNew, MainFrame::OnNewView)
	EVT_MENU(ID_Layout, MainFrame::OnLayout)
	EVT_MENU(ID_Reset, MainFrame::OnReset)
	EVT_MENU(ID_FullScreen, MainFrame::OnFullScreen)
	EVT_MENU(ID_OpenVolume, MainFrame::OnOpenVolume)
	EVT_MENU(ID_OpenMesh, MainFrame::OnOpenMesh)
	EVT_MENU(ID_ViewOrganize, MainFrame::OnOrganize)
	EVT_MENU(ID_CheckUpdates, MainFrame::OnCheckUpdates)
	EVT_MENU(ID_Info, MainFrame::OnInfo)
	EVT_MENU(ID_NewProject, MainFrame::OnNewProject)
	EVT_MENU(ID_SaveProject, MainFrame::OnSaveProject)
	EVT_MENU(ID_SaveAsProject, MainFrame::OnSaveAsProject)
	EVT_MENU(ID_OpenProject, MainFrame::OnOpenProject)
	EVT_MENU(ID_Settings, MainFrame::OnSettings)
	EVT_MENU(ID_ImportVolume, MainFrame::OnImportVolume)
	EVT_MENU(ID_Undo, MainFrame::OnUndo)
	EVT_MENU(ID_Redo, MainFrame::OnRedo)
	//tools
	EVT_MENU(ID_LastTool, MainFrame::OnLastTool)
	EVT_MENU(ID_PaintTool, MainFrame::OnPaintTool)
	EVT_MENU(ID_Measure, MainFrame::OnMeasure)
	EVT_MENU(ID_Trace, MainFrame::OnTrace)
	EVT_MENU(ID_NoiseCancelling, MainFrame::OnNoiseCancelling)
	EVT_MENU(ID_Counting, MainFrame::OnCounting)
	EVT_MENU(ID_Colocalization, MainFrame::OnColocalization)
	EVT_MENU(ID_Convert, MainFrame::OnConvert)
	EVT_MENU(ID_Ocl, MainFrame::OnOcl)
	EVT_MENU(ID_Component, MainFrame::OnComponent)
	EVT_MENU(ID_Calculations, MainFrame::OnCalculations)
	EVT_MENU(ID_MachineLearning, MainFrame::OnMachineLearning)
	//
	EVT_MENU(ID_Youtube, MainFrame::OnYoutube)
	EVT_MENU(ID_Twitter, MainFrame::OnTwitter)
	EVT_MENU(ID_Facebook, MainFrame::OnFacebook)
	EVT_MENU(ID_Manual, MainFrame::OnManual)
	EVT_MENU(ID_Tutorial, MainFrame::OnTutorial)
	EVT_MENU(ID_ShowHideUI, MainFrame::OnShowHideUI)
	EVT_MENU(ID_ShowHideToolbar, MainFrame::OnShowHideToolbar)
	//ui menu events
	EVT_MENU(ID_UIProjView, MainFrame::OnShowHideView)
	EVT_MENU(ID_UIMovieView, MainFrame::OnShowHideView)
	EVT_MENU(ID_UIAdjView, MainFrame::OnShowHideView)
	EVT_MENU(ID_UIClipView, MainFrame::OnShowHideView)
	EVT_MENU(ID_UIPropView, MainFrame::OnShowHideView)
	//toolbar menus
	EVT_AUITOOLBAR_TOOL_DROPDOWN(wxID_ANY, MainFrame::OnToolbarMenu)
	//panes
	EVT_AUI_PANE_CLOSE(MainFrame::OnPaneClose)
	//prop panel
	EVT_AUINOTEBOOK_PAGE_CLOSE(wxID_ANY, MainFrame::OnPropPageClose)
	//draw background
	EVT_PAINT(MainFrame::OnDraw)
	//process key event
	EVT_KEY_DOWN(MainFrame::OnKeyDown)
	//close
	EVT_CLOSE(MainFrame::OnClose)
END_EVENT_TABLE()

MainFrame::MainFrame(
	wxFrame* frame,
	const wxString& title,
	int x, int y,
	int w, int h,
	bool benchmark,
	bool fullscreen,
	bool windowed,
	bool hidepanels)
	: wxFrame(frame, wxID_ANY, title, wxPoint(x, y), wxSize(w, h),wxDEFAULT_FRAME_STYLE),
	m_title(title),
	m_movie_panel(0),
	m_tree_panel(0),
	m_list_panel(0),
	m_prop_panel(0),
	m_clip_view(0),
	m_adjust_view(0),
	m_setting_dlg(0),
	m_help_dlg(0),
	m_brush_tool_dlg(0),
	m_noise_cancelling_dlg(0),
	m_counting_dlg(0),
	m_convert_dlg(0),
	m_colocalization_dlg(0),
	m_measure_dlg(0),
	m_trace_dlg(0),
	m_ocl_dlg(0),
	m_component_dlg(0),
	m_machine_learning_dlg(0),
	m_script_break_dlg(0),
	//m_volume_prop(0),
	//m_mesh_prop(0),
	//m_mesh_manip(0),
	//m_annotation_prop(0),
	m_ui_state(true),
	m_cur_sel_type(-1),
	m_cur_sel_vol(-1),
	m_cur_sel_mesh(-1),
	m_benchmark(benchmark),
	m_vd_copy(0),
	m_copy_data(false),
	m_waker(0)
{
#ifdef _DARWIN
	SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#endif
	//create this first to read the settings
	glbin_settings.Read();
	glbin.apply_processor_settings();
	glbin_vol_calculator.SetFrame(this);
	glbin_script_proc.SetFrame(this);

	// tell wxAuiManager to manage this frame
	m_aui_mgr.SetManagedWindow(this);

	glbin_data_manager.SetFrame(this);

	// set frame icon
	wxIcon icon;
	icon.CopyFromBitmap(wxGetBitmapFromMemory(icon_32));
	SetIcon(icon);
	wxBitmap bitmap;

	// create the main toolbar
	wxMenuItem* m;
	wxAuiToolBarItemArray prepend_items;
	wxAuiToolBarItemArray append_items;
	m_main_tb = new wxAuiToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxAUI_TB_DEFAULT_STYLE | wxAUI_TB_OVERFLOW | wxAUI_TB_HORIZONTAL);
	m_main_tb->SetName("Toolbar");
	//add tools
	bitmap = wxGetBitmapFromMemory(icon_open_volume);
	m_main_tb->AddTool(ID_OpenVolume, "Open Volume", bitmap,
		"Open single or multiple volume data file(s)");
	if (JVMInitializer::getInstance(glbin_settings.GetJvmArgs()) != nullptr)
	{
		bitmap = wxGetBitmapFromMemory(icon_import);
		m_main_tb->AddTool(ID_ImportVolume, "Import Volume", bitmap,
			"Import single or multiple volume data file(s) using ImageJ");
		m_main_tb->AddSeparator();
	}
	bitmap = wxGetBitmapFromMemory(icon_open_project);
	m_main_tb->AddTool(ID_OpenProject, "Open Project", bitmap,
		"Open a saved project");
	bitmap = wxGetBitmapFromMemory(icon_save_project);
	m_main_tb->AddTool(ID_SaveProject, "Save Project", bitmap,
		"Save current work as a project");
	m_main_tb->AddSeparator();
	bitmap = wxGetBitmapFromMemory(icon_new_view);
	m_main_tb->AddTool(ID_ViewNew, "New View", bitmap,
		"Create a new render viewport");
	bitmap = wxGetBitmapFromMemory(icon_show_hide_ui);
	m_main_tb->AddTool(ID_ShowHideUI, "Show/Hide UI", bitmap,
		"Show or hide all control panels");
	m_main_tb->SetToolDropDown(ID_ShowHideUI, true);
	m_main_tb->AddSeparator();
	bitmap = wxGetBitmapFromMemory(icon_open_mesh);
	m_main_tb->AddTool(ID_OpenMesh, "Open Mesh", bitmap,
		"Open single or multiple mesh file(s)");
	bitmap = wxGetBitmapFromMemory(icon_paint_brush);
	m_main_tb->AddTool(ID_LastTool, "Analyze", bitmap,
		"Tools for analyzing selected channel");
	m_main_tb->SetToolDropDown(ID_LastTool, true);
	//set analyze icon
	switch (glbin_settings.m_last_tool)
	{
	case TOOL_PAINT_BRUSH:
	default:
		m_main_tb->SetToolBitmap(ID_LastTool,
			wxGetBitmapFromMemory(icon_paint_brush));
		break;
	case TOOL_MEASUREMENT:
		m_main_tb->SetToolBitmap(ID_LastTool,
			wxGetBitmapFromMemory(icon_measurement));
		break;
	case TOOL_TRACKING:
		m_main_tb->SetToolBitmap(ID_LastTool,
			wxGetBitmapFromMemory(icon_tracking));
		break;
	case TOOL_NOISE_REDUCTION:
		m_main_tb->SetToolBitmap(ID_LastTool,
			wxGetBitmapFromMemory(icon_noise_reduc));
		break;
	case TOOL_VOLUME_SIZE:
		m_main_tb->SetToolBitmap(ID_LastTool,
			wxGetBitmapFromMemory(icon_volume_size));
		break;
	case TOOL_COLOCALIZATION:
		m_main_tb->SetToolBitmap(ID_LastTool,
			wxGetBitmapFromMemory(icon_colocalization));
		break;
	case TOOL_CONVERT:
		m_main_tb->SetToolBitmap(ID_LastTool,
			wxGetBitmapFromMemory(icon_convert));
		break;
	case TOOL_OPENCL:
		m_main_tb->SetToolBitmap(ID_LastTool,
			wxGetBitmapFromMemory(icon_opencl));
		break;
	case TOOL_COMPONENT:
		m_main_tb->SetToolBitmap(ID_LastTool,
			wxGetBitmapFromMemory(icon_components));
		break;
	case TOOL_CALCULATIONS:
		m_main_tb->SetToolBitmap(ID_LastTool,
			wxGetBitmapFromMemory(icon_calculations));
		break;
	case TOOL_MACHINE_LEARNING:
		m_main_tb->SetToolBitmap(ID_LastTool,
			wxGetBitmapFromMemory(icon_machine_learning));
		break;
	}
	m_main_tb->AddSeparator();
	bitmap = wxGetBitmapFromMemory(icon_undo);
	m_main_tb->AddTool(ID_Undo, "Undo", bitmap,
		"Undo");
	bitmap = wxGetBitmapFromMemory(icon_redo);
	m_main_tb->AddTool(ID_Redo, "Redo", bitmap,
		"Redo");
	//right-side items
	m_main_tb->AddStretchSpacer();
	bitmap = wxGetBitmapFromMemory(icon_settings);
	m_main_tb->AddTool(ID_Settings, "Configurations", bitmap,
		"Settings of FluoRender");
	m_main_tb->AddSeparator();
	int num = rand() % 4;
	wxString str1, str2, str3;
	int item_id;
	switch (num)
	{
	case 0:
		bitmap = wxGetBitmapFromMemory(icon_check_updates);
		str1 = "Check Updates";
		str2 = "Check if there is a new release";
		str3 = "Check if there is a new release (requires Internet connection)";
		item_id = ID_CheckUpdates;
		break;
	case 1:
		bitmap = wxGetBitmapFromMemory(icon_youtube);
		str1 = "Video Tutorials";
		str2 = "FluoRender's YouTube channel & Tutorials";
		str3 = "FluoRender's YouTube channel & Tutorials (requires Internet connection)";
		item_id = ID_Youtube;
		break;
	case 2:
		bitmap = wxGetBitmapFromMemory(icon_twitter);
		str1 = "Twitter";
		str2 = "Follow FluoRender on Twitter";
		str3 = "Follow FluoRender on Twitter (requires Internet connection)";
		item_id = ID_Twitter;
		break;
	case 3:
	default:
		bitmap = wxGetBitmapFromMemory(icon_about);
		str1 = "About";
		str2 = "FluoRender information";
		str3 = "FluoRender information";
		item_id = ID_Info;
		break;
	}
	m_main_tb->AddTool(item_id, str1, bitmap, str3);
	m_main_tb->SetToolDropDown(item_id, true);
	m_main_tb->SetCustomOverflowItems(prepend_items, append_items);
	m_main_tb->Realize();

	//create the menu for UI management
	m_tb_menu_ui = new wxMenu;
	m = m_tb_menu_ui->Append(ID_UIProjView, UITEXT_PROJECT,
		"Show/hide the project panel", wxITEM_CHECK);
	m->SetBitmap(wxGetBitmapFromMemory(disp_project));
	m = m_tb_menu_ui->Append(ID_UIMovieView, UITEXT_MAKEMOVIE,
		"Show/hide the movie making panel", wxITEM_CHECK);
	m->SetBitmap(wxGetBitmapFromMemory(disp_makemovie));
	m = m_tb_menu_ui->Append(ID_UIAdjView, UITEXT_ADJUST,
		"Show/hide the output adjustment panel", wxITEM_CHECK);
	m->SetBitmap(wxGetBitmapFromMemory(disp_adjust));
	m = m_tb_menu_ui->Append(ID_UIClipView, UITEXT_CLIPPING,
		"Show/hide the clipping plane control panel", wxITEM_CHECK);
	m->SetBitmap(wxGetBitmapFromMemory(disp_clipping));
	m = m_tb_menu_ui->Append(ID_UIPropView, UITEXT_PROPERTIES,
		"Show/hide the property panel", wxITEM_CHECK);
	m->SetBitmap(wxGetBitmapFromMemory(disp_properties));
	//check all the items
	m_tb_menu_ui->Check(ID_UIProjView, true);
	m_tb_menu_ui->Check(ID_UIMovieView, true);
	m_tb_menu_ui->Check(ID_UIAdjView, true);
	m_tb_menu_ui->Check(ID_UIClipView, true);
	m_tb_menu_ui->Check(ID_UIPropView, true);
	//create the menu for edit/convert
	m_tb_menu_edit = new wxMenu;
	m = m_tb_menu_edit->Append(ID_PaintTool, "Paint Brush...",
		"Use the paint brush to select regions of interest in 3D");
	m->SetBitmap(wxGetBitmapFromMemory(icon_paint_brush_mini));
	m = m_tb_menu_edit->Append(ID_Measure, "Measurement...",
		"Make measurements with the ruler tools");
	m->SetBitmap(wxGetBitmapFromMemory(icon_measurement_mini));
	m = m_tb_menu_edit->Append(ID_Component, "Component Analyzer...",
		"Segment structures into components and perform analysis");
	m->SetBitmap(wxGetBitmapFromMemory(icon_components_mini));
	m = m_tb_menu_edit->Append(ID_Trace, "Tracking...",
		"Track the movements of structures");
	m->SetBitmap(wxGetBitmapFromMemory(icon_tracking_mini));
	m = m_tb_menu_edit->Append(ID_Calculations, "Calculations...",
		"Calculate a new volume channels from existing channels");
	m->SetBitmap(wxGetBitmapFromMemory(icon_calculations_mini));
	m = m_tb_menu_edit->Append(ID_NoiseCancelling, "Noise Reduction...",
		"Remove noise signals from a volume channel");
	m->SetBitmap(wxGetBitmapFromMemory(icon_noise_reduc_mini));
	m = m_tb_menu_edit->Append(ID_Counting, "Volume Size...",
		"Calculate the size of a volume channel");
	m->SetBitmap(wxGetBitmapFromMemory(icon_volume_size_mini));
	m = m_tb_menu_edit->Append(ID_Colocalization, "Colocalization...",
		"Analyze the colocalized areas among channels");
	m->SetBitmap(wxGetBitmapFromMemory(icon_colocalization_mini));
	m = m_tb_menu_edit->Append(ID_Convert, "Convert...",
		"Conver a volume channel to a mesh object");
	m->SetBitmap(wxGetBitmapFromMemory(icon_convert_mini));
	m = m_tb_menu_edit->Append(ID_Ocl, "OpenCL Kernel Editor...",
		"Edit and apply kernel filters using OpenCL");
	m->SetBitmap(wxGetBitmapFromMemory(icon_opencl_mini));
	m = m_tb_menu_edit->Append(ID_MachineLearning, "Machine Learning Manager...",
		"Manage machine-learning libraries for various functions");
	m->SetBitmap(wxGetBitmapFromMemory(icon_machine_learning_mini));
	//right-side items
	m_tb_menu_update = new wxMenu;
	m = m_tb_menu_update->Append(ID_CheckUpdates, "Check Updates...",
		"Check if a new version of FluoRender is available");
	m->SetBitmap(wxGetBitmapFromMemory(icon_check_updates_mini));
	m = m_tb_menu_update->Append(ID_Youtube, "Video Tutorials...",
		"Watch FluoRender tutorial videos on YouTube");
	m->SetBitmap(wxGetBitmapFromMemory(icon_youtube_mini));
	m = m_tb_menu_update->Append(ID_Twitter, "Twitter...",
		"Follow FluoRender on Twitter");
	m->SetBitmap(wxGetBitmapFromMemory(icon_twitter_mini));
	m = m_tb_menu_update->Append(ID_Info, "About...",
		"Information about FluoRender");
	m->SetBitmap(wxGetBitmapFromMemory(icon_about_mini));

	//create render view
	RenderViewPanel *vrv = new RenderViewPanel(this);
	vrv->m_canvas->InitView();
	m_vrv_list.push_back(vrv);

	wxSize panel_size(FromDIP(wxSize(350, 450)));
	//use the project panel for both tree and list
	m_proj_panel = new wxAuiNotebook(this, wxID_ANY,
		wxDefaultPosition, panel_size,
		wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE | 
		wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_TAB_EXTERNAL_MOVE |
		wxAUI_NB_WINDOWLIST_BUTTON | wxNO_BORDER);
	m_proj_panel->SetName("ProjectPanel");
	//create list view
	m_list_panel = new ListPanel(this,
		wxDefaultPosition, panel_size);
	//create tree view
	m_tree_panel = new TreePanel(this,
		wxDefaultPosition, panel_size);
	m_proj_panel->AddPage(m_list_panel, UITEXT_DATAVIEW, false);
	m_proj_panel->AddPage(m_tree_panel, UITEXT_TREEVIEW, true);

	//create movie view (sets the m_recorder_dlg)
	m_movie_panel = new MoviePanel(this,
		wxDefaultPosition, panel_size);

	//create prop panel
	m_prop_panel = new wxAuiNotebook(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize,
		wxAUI_NB_DEFAULT_STYLE | wxAUI_NB_TAB_EXTERNAL_MOVE |
		wxAUI_NB_WINDOWLIST_BUTTON | wxNO_BORDER);
	m_prop_panel->SetName("PropPanel");

	//clipping view
	m_clip_view = new ClipPlanePanel(this,
		wxDefaultPosition, FromDIP(wxSize(140,700)));
	m_clip_view->SetPlaneMode(static_cast<PLANE_MODES>(glbin_settings.m_plane_mode));

	//adjust view
	m_adjust_view = new OutputAdjPanel(this,
		wxDefaultPosition, FromDIP(wxSize(140, 700)));

	//settings dialog
	m_setting_dlg = new SettingDlg(this);

	//brush tool dialog
	m_brush_tool_dlg = new BrushToolDlg(this);

	//noise cancelling dialog
	m_noise_cancelling_dlg = new NoiseCancellingDlg(this);

	//counting dialog
	m_counting_dlg = new CountingDlg(this);

	//convert dialog
	m_convert_dlg = new ConvertDlg(this);

	//colocalization dialog
	m_colocalization_dlg = new ColocalizationDlg(this);

	//measure dialog
	m_measure_dlg = new MeasureDlg(this);

	//trace dialog
	m_trace_dlg = new TraceDlg(this);

	//ocl dialog
	m_ocl_dlg = new OclDlg(this);

	//component dialog
	m_component_dlg = new ComponentDlg(this);

	//calculation dialog
	m_calculation_dlg = new CalculationDlg(this);

	//machine learing dialog
	m_machine_learning_dlg = new MachineLearningDlg(this);

	//script break dialog
	m_script_break_dlg = new ScriptBreakDlg(this);

	//help dialog
	m_help_dlg = new HelpDlg(this);
	//m_help_dlg->LoadPage("C:\\!wanyong!\\TEMP\\wxHtmlWindow.htm");

	//tester
	//shown for testing parameters
	m_tester = new TesterDlg(this);
	if (glbin_settings.m_test_param)
		m_tester->Show(true);
	else
		m_tester->Show(false);

	//Add to the manager
	m_aui_mgr.AddPane(m_main_tb, wxAuiPaneInfo().
		Name("m_main_tb").Caption("Toolbar").ToolbarPane().Top().Row(1).Resizable());
	m_aui_mgr.AddPane(m_proj_panel, wxAuiPaneInfo().
		Name("m_proj_panel").Caption(UITEXT_PROJECT).
		BestSize(FromDIP(wxSize(320, 500))).
		FloatingSize(FromDIP(wxSize(320, 500))).
		Left().CloseButton(true).Layer(3));
	m_aui_mgr.AddPane(m_movie_panel, wxAuiPaneInfo().
		Name("m_movie_panel").Caption(UITEXT_MAKEMOVIE).
		BestSize(FromDIP(wxSize(320, 500))).
		FloatingSize(FromDIP(wxSize(320, 500))).
		Left().CloseButton(true).Layer(3));
	m_aui_mgr.AddPane(m_prop_panel, wxAuiPaneInfo().
		Name("m_prop_panel").Caption(UITEXT_PROPERTIES).
		BestSize(FromDIP(wxSize(1100, 160))).
		FloatingSize(FromDIP(wxSize(1100, 160))).
		Bottom().CloseButton(true).Layer(2));
	m_aui_mgr.AddPane(m_adjust_view, wxAuiPaneInfo().
		Name("m_adjust_view").Caption(UITEXT_ADJUST).
		BestSize(FromDIP(wxSize(150, 800))).
		FloatingSize(FromDIP(wxSize(150, 800))).
		Left().CloseButton(true).Layer(1));
	m_aui_mgr.AddPane(m_clip_view, wxAuiPaneInfo().
		Name("m_clip_view").Caption(UITEXT_CLIPPING).
		BestSize(FromDIP(wxSize(150, 800))).
		FloatingSize(FromDIP(wxSize(150, 800))).
		Right().CloseButton(true).Layer(1));
	m_aui_mgr.AddPane(vrv, wxAuiPaneInfo().
		Name(vrv->GetName()).Caption(vrv->GetName()).
		Dockable(true).CloseButton(false).
		Layer(0).Centre());

	//dialogs
	//brush tool dialog
	m_aui_mgr.AddPane(m_brush_tool_dlg, wxAuiPaneInfo().
		Name("m_brush_tool_dlg").Caption("Paint Brush").
		Dockable(false).CloseButton(true).
		MaximizeButton(true));
	m_aui_mgr.GetPane(m_brush_tool_dlg).Float();
	m_aui_mgr.GetPane(m_brush_tool_dlg).Hide();
	//noise cancelling dialog
	m_aui_mgr.AddPane(m_noise_cancelling_dlg, wxAuiPaneInfo().
		Name("m_noise_cancelling_dlg").Caption("Noise Reduction").
		Dockable(false).CloseButton(true).
		MaximizeButton(true));
	m_aui_mgr.GetPane(m_noise_cancelling_dlg).Float();
	m_aui_mgr.GetPane(m_noise_cancelling_dlg).Hide();
	//counting dialog
	m_aui_mgr.AddPane(m_counting_dlg, wxAuiPaneInfo().
		Name("m_counting_dlg").Caption("Volume Size").
		Dockable(false).CloseButton(true).
		MaximizeButton(true));
	m_aui_mgr.GetPane(m_counting_dlg).Float();
	m_aui_mgr.GetPane(m_counting_dlg).Hide();
	//convert dialog
	m_aui_mgr.AddPane(m_convert_dlg, wxAuiPaneInfo().
		Name("m_convert_dlg").Caption("Convert").
		Dockable(false).CloseButton(true).
		MaximizeButton(true));
	m_aui_mgr.GetPane(m_convert_dlg).Float();
	m_aui_mgr.GetPane(m_convert_dlg).Hide();
	//colocalization dialog
	m_aui_mgr.AddPane(m_colocalization_dlg, wxAuiPaneInfo().
		Name("m_colocalization_dlg").Caption("Colocalization").
		Dockable(false).CloseButton(true).
		MaximizeButton(true));
	m_aui_mgr.GetPane(m_colocalization_dlg).Float();
	m_aui_mgr.GetPane(m_colocalization_dlg).Hide();
	//measure dialog
	m_aui_mgr.AddPane(m_measure_dlg, wxAuiPaneInfo().
		Name("m_measure_dlg").Caption("Measurement").
		Dockable(false).CloseButton(true).
		MaximizeButton(true));
	m_aui_mgr.GetPane(m_measure_dlg).Float();
	m_aui_mgr.GetPane(m_measure_dlg).Hide();
	//trace dialog
	m_aui_mgr.AddPane(m_trace_dlg, wxAuiPaneInfo().
		Name("m_trace_dlg").Caption("Tracking").
		Dockable(false).CloseButton(true).
		MaximizeButton(true));
	m_aui_mgr.GetPane(m_trace_dlg).Float();
	m_aui_mgr.GetPane(m_trace_dlg).Hide();
	//ocl fialog
	m_aui_mgr.AddPane(m_ocl_dlg, wxAuiPaneInfo().
		Name("m_ocl_dlg").Caption("OpenCL Kernel Editor").
		Dockable(false).CloseButton(true).
		MaximizeButton(true));
	m_aui_mgr.GetPane(m_ocl_dlg).Float();
	m_aui_mgr.GetPane(m_ocl_dlg).Hide();
	//component dialog
	m_aui_mgr.AddPane(m_component_dlg, wxAuiPaneInfo().
		Name("m_component_dlg").Caption("Component Analyzer").
		Dockable(false).CloseButton(true).
		MaximizeButton(true));
	m_aui_mgr.GetPane(m_component_dlg).Float();
	m_aui_mgr.GetPane(m_component_dlg).Hide();
	//calculation dialog
	m_aui_mgr.AddPane(m_calculation_dlg, wxAuiPaneInfo().
		Name("m_calculation_dlg").Caption("Calculations").
		Dockable(false).CloseButton(true).
		MaximizeButton(true));
	m_aui_mgr.GetPane(m_calculation_dlg).Float();
	m_aui_mgr.GetPane(m_calculation_dlg).Hide();
	//machine learning
	m_aui_mgr.AddPane(m_machine_learning_dlg, wxAuiPaneInfo().
		Name("m_machine_learning_dlg").Caption("Machine Learning").
		Dockable(false).CloseButton(true).
		MaximizeButton(true));
	m_aui_mgr.GetPane(m_machine_learning_dlg).Float();
	m_aui_mgr.GetPane(m_machine_learning_dlg).Hide();
	//settings
	m_aui_mgr.AddPane(m_setting_dlg, wxAuiPaneInfo().
		Name("m_setting_dlg").Caption("Configurations").
		Dockable(false).CloseButton(true).
		MaximizeButton(true));
	m_aui_mgr.GetPane(m_setting_dlg).Float();
	m_aui_mgr.GetPane(m_setting_dlg).Hide();
	//help
	m_aui_mgr.AddPane(m_help_dlg, wxAuiPaneInfo().
		Name("m_help_dlg").Caption("Help").
		Dockable(false).CloseButton(true).
		MaximizeButton(true));
	m_aui_mgr.GetPane(m_help_dlg).Float();
	m_aui_mgr.GetPane(m_help_dlg).Hide();
	//script break
	m_aui_mgr.AddPane(m_script_break_dlg, wxAuiPaneInfo().
		Name("m_script_break_dlg").Caption("Script").
		Dockable(false).CloseButton(true).
		MaximizeButton(true));
	m_aui_mgr.GetPane(m_script_break_dlg).Float();
	m_aui_mgr.GetPane(m_script_break_dlg).Hide();

	for (auto it : m_vrv_list)
		it->LoadSettings();

	SetMinSize(FromDIP(wxSize(800,600)));

	if (!windowed)
		Maximize();

	if (hidepanels)
	{
		m_ui_state = true;
		ToggleAllTools(false);
	}

	//drop target
	SetDropTarget(new DnDFile(this));

	CreateStatusBar(2);
	GetStatusBar()->SetStatusText(wxString(FLUORENDER_TITLE)+
		wxString(" started normally."));

	//main top menu
	m_top_menu = new wxMenuBar;
	m_top_file = new wxMenu;
	m_top_tools = new wxMenu;
	m_top_window = new wxMenu;
	m_top_help = new wxMenu;

	//file options
	m = m_top_file->Append(ID_NewProject, "&New Project",
		"Clean the workspace by creating a new project");
	m->SetBitmap(wxGetBitmapFromMemory(icon_new_project_mini));
	m = m_top_file->Append(ID_OpenVolume, "Open &Volume",
		"Open a volume data set and add it to the render view");
	m->SetBitmap(wxGetBitmapFromMemory(icon_open_volume_mini));
	if (JVMInitializer::getInstance(glbin_settings.GetJvmArgs()) != nullptr)
	{
		m = m_top_file->Append(ID_ImportVolume, "&Import Volume",
			"Import a volume data set using ImageJ");
		m->SetBitmap(wxGetBitmapFromMemory(icon_import_mini));
	}	
	m = m_top_file->Append(ID_OpenMesh, "Open &Mesh",
		"Open a mesh data set and add it the render view");
	m->SetBitmap(wxGetBitmapFromMemory(icon_open_mesh_mini));
	m = m_top_file->Append(ID_OpenProject, "Open &Project",
		"Open a previously saved project");
	m->SetBitmap(wxGetBitmapFromMemory(icon_open_project_mini));
	m = m_top_file->Append(ID_SaveProject, "&Save Project",
		"Save the workspace to current project file");
	m->SetBitmap(wxGetBitmapFromMemory(icon_save_project_mini));
	m = m_top_file->Append(ID_SaveAsProject, "Save &As Project",
		"Save the workspace in a new project file");
	m->SetBitmap(wxGetBitmapFromMemory(icon_save_project_mini));
	m_top_file->AppendSeparator();
	m = m_top_file->Append(wxID_EXIT, "E&xit",
		"Shut down FluoRender");
	m->SetBitmap(wxGetBitmapFromMemory(exit));
	//tool options
	m = m_top_tools->Append(ID_PaintTool, "&Paint Brush...",
		"Use the paint brush to select structures of interest in 3D");
	m->SetBitmap(wxGetBitmapFromMemory(icon_paint_brush_mini));
	m = m_top_tools->Append(ID_Measure, "&Measurement...",
		"Use the ruler tools to make measurements and analysis in 3D");
	m->SetBitmap(wxGetBitmapFromMemory(icon_measurement_mini));
	m = m_top_tools->Append(ID_Component, "Component Anal&yzer...",
		"Segment structures into components and make analysis");
	m->SetBitmap(wxGetBitmapFromMemory(icon_components_mini));
	m = m_top_tools->Append(ID_Trace, "&Tracking...",
		"Track the movements of segmented structures");
	m->SetBitmap(wxGetBitmapFromMemory(icon_tracking_mini));
	m = m_top_tools->Append( ID_Calculations, "Ca&lculations...",
		"Compute a new volume channel from existing channels");
	m->SetBitmap(wxGetBitmapFromMemory(icon_calculations_mini));
	m = m_top_tools->Append(ID_NoiseCancelling, "Noise &Reduction...",
		"Remove noisy signals");
	m->SetBitmap(wxGetBitmapFromMemory(icon_noise_reduc_mini));
	m = m_top_tools->Append(ID_Counting, "&Volume Size...",
		"Compute the size of a volume channel");
	m->SetBitmap(wxGetBitmapFromMemory(icon_volume_size_mini));
	m = m_top_tools->Append(ID_Colocalization, "&Colocalization...",
		"Make analysis on the overlapped regions among channels");
	m->SetBitmap(wxGetBitmapFromMemory(icon_colocalization_mini));
	m = m_top_tools->Append(ID_Convert, "Co&nvert...",
		"Conver a volume channel to a mesh object");
	m->SetBitmap(wxGetBitmapFromMemory(icon_convert_mini));
	m = m_top_tools->Append(ID_Ocl, "&OpenCL Kernel Editor...",
		"Edit and apply filtering with OpenCL kernels");
	m->SetBitmap(wxGetBitmapFromMemory(icon_opencl_mini));
	m = m_top_tools->Append(ID_MachineLearning, "M&achine Learning Manager...",
		"Manage machine-learning models and training");
	m->SetBitmap(wxGetBitmapFromMemory(icon_machine_learning_mini));
	m_top_tools->Append(wxID_SEPARATOR);
	m = m_top_tools->Append(ID_Settings, "Con&figurations...",
		"Manage settings of FluoRender");
	m->SetBitmap(wxGetBitmapFromMemory(icon_settings_mini));
	//window option
	m = m_top_window->Append(ID_ShowHideToolbar, "Show/Hide &Toolbar",
		"The commonly used main menu functions can be accessed from the toolbar", wxITEM_CHECK);
	m->SetBitmap(wxGetBitmapFromMemory(disp_toolbar));
	m_top_window->Check(ID_ShowHideToolbar, true);
	m_top_window->Append(wxID_SEPARATOR);
	m = m_top_window->Append(ID_ShowHideUI, "Show/Hide &UI",
		"Show or hide all panels around the render view");
	m->SetBitmap(wxGetBitmapFromMemory(icon_show_hide_ui_mini));
	m = m_top_window->Append(ID_UIProjView, "&Project",
		"Shoe or hide the project panel", wxITEM_CHECK);
	m->SetBitmap(wxGetBitmapFromMemory(disp_project));
	m_top_window->Check(ID_UIProjView, true);
	m = m_top_window->Append(ID_UIMovieView, "&Movie Making",
		"Show or hide the movie making panel", wxITEM_CHECK);
	m->SetBitmap(wxGetBitmapFromMemory(disp_makemovie));
	m_top_window->Check(ID_UIMovieView, true);
	m = m_top_window->Append(ID_UIAdjView, "&Output Adjustments",
		"Show or hide the output adjustment panel", wxITEM_CHECK);
	m->SetBitmap(wxGetBitmapFromMemory(disp_adjust));
	m_top_window->Check(ID_UIAdjView, true);
	m = m_top_window->Append(ID_UIClipView, "&Clipping Planes",
		"Show or hide the clipping plane panel", wxITEM_CHECK);
	m->SetBitmap(wxGetBitmapFromMemory(disp_clipping));
	m_top_window->Check(ID_UIClipView, true);
	m = m_top_window->Append(ID_UIPropView,"P&roperties",
		"Show or hide the property panel", wxITEM_CHECK);
	m->SetBitmap(wxGetBitmapFromMemory(disp_properties));
	m_top_window->Check(ID_UIPropView, true);
	m_top_window->Append(wxID_SEPARATOR);
	m = m_top_window->Append(ID_Layout, "&Layout",
		"Resize the render view panels");
	m->SetBitmap(wxGetBitmapFromMemory(layout));
	m = m_top_window->Append(ID_Reset, "R&eset",
		"Reset the user interface layout");
	m->SetBitmap(wxGetBitmapFromMemory(reset_mini));
	m = m_top_window->Append(ID_ViewNew, "&New View",
		"Create a new render view panel");
	m->SetBitmap(wxGetBitmapFromMemory(icon_new_view_mini));
	m = m_top_window->Append(ID_FullScreen, "&Full Screen",
		"Enlarge the window to fill the screen");
	m->SetBitmap(wxGetBitmapFromMemory(full_screen_menu));
	//help menu
	m = m_top_help->Append(ID_CheckUpdates, "&Check for Updates",
		"Check if there is a later version of FluoRender available for download");
	m->SetBitmap(wxGetBitmapFromMemory(icon_check_updates_mini));
	m = m_top_help->Append( ID_Youtube, "&Video Tutorials",
		"Watch FluoRender tutorials on YouTube");
	m->SetBitmap(wxGetBitmapFromMemory(icon_youtube_mini));
	m = m_top_help->Append(ID_Twitter, "&Twitter",
		"Check information on Twitter");
	m->SetBitmap(wxGetBitmapFromMemory(icon_twitter_mini));
	m = m_top_help->Append(ID_Facebook, "&Facebook",
		"Check information on facebook");
	m->SetBitmap(wxGetBitmapFromMemory(icon_facebook_mini));
	m = m_top_help->Append(ID_Manual, "&Manual PDF",
		"Download the PDF of FluoRender operation manual");
	m->SetBitmap(wxGetBitmapFromMemory(web_pdf_mini));
	m = m_top_help->Append(ID_Tutorial, "T&utorials PDF",
		"Download the PDF of FluoRender tutorials");
	m->SetBitmap(wxGetBitmapFromMemory(web_pdf_mini));
	m = m_top_help->Append(ID_Info, "&About FluoRender...",
		"Developer team information of FluoRender");
	m->SetBitmap(wxGetBitmapFromMemory(icon_about_mini));
	//add the menus
	m_top_menu->Append(m_top_file,"&File");
	m_top_menu->Append(m_top_tools,"&Tools");
	m_top_menu->Append(m_top_window,"&Windows");
	m_top_menu->Append(m_top_help,"&Help");
	SetMenuBar(m_top_menu);

	if (fullscreen)
	{
		vrv->SetFullScreen();
		Iconize();
	}

	wxMemorySize free_mem_size = wxGetFreeMemory();
	double mainmem_buf_size = free_mem_size.ToDouble() * 0.8 / 1024.0 / 1024.0;
	if (mainmem_buf_size > flvr::TextureRenderer::get_mainmem_buf_size())
		flvr::TextureRenderer::set_mainmem_buf_size(mainmem_buf_size);

	//python
	flrd::PyBase::SetHighVer(glbin_settings.m_python_ver);
	//font
	wxString font_file = glbin_settings.m_font_file;
	wxString exePath = wxStandardPaths::Get().GetExecutablePath();
	exePath = wxPathOnly(exePath);
	if (font_file != "")
		font_file = exePath + GETSLASH() + "Fonts" +
		GETSLASH() + font_file;
	else
		font_file = exePath + GETSLASH() + "Fonts" +
		GETSLASH() + "FreeSans.ttf";
	glbin_text_tex_manager.load_face(font_file.ToStdString());
	glbin_text_tex_manager.SetSize(glbin_settings.m_text_size);
	//hologram
	if (glbin_settings.m_hologram_mode == 1)
		m_vrv_list[0]->m_canvas->InitOpenVR();

	//keyboard shortcuts
	wxAcceleratorEntry entries[5];
	entries[0].Set(wxACCEL_CTRL, (int)'N', ID_NewProject);
	entries[1].Set(wxACCEL_CTRL, (int)'S', ID_SaveProject);
	entries[2].Set(wxACCEL_CTRL, (int)'Z', ID_Undo);
	entries[3].Set(wxACCEL_CTRL | wxACCEL_SHIFT, (int)'Z', ID_Redo);
	entries[4].Set(wxACCEL_CTRL, (int)'O', ID_OpenVolume);
	wxAcceleratorTable accel(5, entries);
	SetAcceleratorTable(accel);

	if (fluo::InEpsilon(glbin_settings.m_dpi_scale_factor,
		GetDPIScaleFactor()))
		m_aui_mgr.LoadPerspective(glbin_settings.m_layout);
	if (glbin_settings.m_prj_panel_split)
		m_proj_panel->Split(1, wxBOTTOM);
	m_adjust_view->LoadPerspective();
	m_clip_view->LoadPerspective();
	glbin_moviemaker.SetMainFrame(this);
	glbin_moviemaker.SetView(vrv->m_canvas);
	glbin_mov_def.Apply(&glbin_moviemaker);
	UpdateProps({});

	m_aui_mgr.Update();

	m_waker = new wxTimer(this);
	m_waker->Bind(wxEVT_TIMER, [](wxTimerEvent& event) { wxWakeUpIdle(); });
	m_waker->Start(100);
}

MainFrame::~MainFrame()
{
	//release?
	glbin_vol_kernel_factory.clear();
	glbin_framebuffer_manager.clear();
	glbin_vertex_array_manager.clear();
	glbin_vol_shader_factory.clear();
	glbin_seg_shader_factory.clear();
	glbin_vol_cal_shader_factory.clear();
	glbin_img_shader_factory.clear();
	glbin_light_field_shader_factory.clear();
	glbin_text_tex_manager.clear();

	for (int i=0; i<GetViewNum(); i++)
	{
		RenderCanvas* canvas = GetView(i);
		if (canvas)
		{
			canvas->ClearAll();
			if (i == 0)
				glbin_brush_def.Set(&glbin_vol_selector);
		}
	}
	flvr::KernelProgram::release();

	glbin_comp_def.Set(&glbin_comp_generator);
	glbin_mov_def.Set(&glbin_moviemaker);
	glbin_settings.m_dpi_scale_factor = GetDPIScaleFactor();
	glbin_settings.m_layout = m_aui_mgr.SavePerspective();
	glbin_settings.m_prj_panel_split = m_proj_panel->IsSplit();
	m_adjust_view->SavePerspective();
	m_clip_view->SavePerspective();
	glbin_settings.Save();

	m_aui_mgr.UnInit();

	if (m_waker)
	{
		m_waker->Stop();
		delete m_waker;
	}
}

void MainFrame::OnExit(wxCommandEvent& event)
{
	Close(true);
}

void MainFrame::OnClose(wxCloseEvent &event)
{
	glbin_moviemaker.Stop();

	bool vrv_saved = false;
	for (unsigned int i=0; i<m_vrv_list.size(); ++i)
	{
		if (m_vrv_list[i]->m_default_saved)
		{
			vrv_saved = true;
			break;
		}
		m_vrv_list[i]->CloseFullScreen();
	}
	if (!vrv_saved && !m_vrv_list.empty())
		m_vrv_list[0]->SaveDefault(0xaff);
	glbin.clear_python();
	event.Skip();
}

wxString MainFrame::CreateView(int row)
{
	RenderViewPanel* vrv = 0;
	if (m_vrv_list.size()>0)
	{
		wxGLContext* sharedContext = m_vrv_list[0]->GetContext();
		vrv = new RenderViewPanel(this, sharedContext);
	}
	else
	{
		vrv = new RenderViewPanel(this);
	}

	if (!vrv)
		return "NO_NAME";
	RenderCanvas* canvas = vrv->m_canvas;
	if (!canvas)
		return "NO_NAME";

	m_vrv_list.push_back(vrv);
	if (m_movie_panel)
		m_movie_panel->FluoUpdate({ gstMovViewList });

	//reset gl
	for (int i = 0; i < GetViewNum(); ++i)
	{
		if (GetView(i))
			GetView(i)->m_set_gl = false;
	}

	//m_aui_mgr.Update();
	OrganizeVRenderViews(1);

	//set view default settings
	//if (m_adjust_view)
	//{
		/*Color gamma, brightness, hdr;
		bool sync_r, sync_g, sync_b;
		m_adjust_view->GetDefaults(gamma, brightness, hdr, sync_r, sync_g, sync_b);
		vrv->m_canvas->SetGamma(gamma);
		vrv->m_canvas->SetBrightness(brightness);
		vrv->m_canvas->SetHdr(hdr);*/
		//view->SetSyncR(true);
		//view->SetSyncG(true);
		//view->SetSyncB(true);
	//}

	//add volumes
	if (GetViewNum() > 0)
	{
		RenderCanvas* view0 = GetView(0);
		if (view0)
		{
			for (int i = 0; i < view0->GetDispVolumeNum(); ++i)
			{
				VolumeData* vd = view0->GetDispVolumeData(i);
				if (vd)
				{
					VolumeData* vd_add = glbin_data_manager.DuplicateVolumeData(vd);

					if (vd_add)
					{
						int chan_num = canvas->GetAny();
						fluo::Color color(1.0, 1.0, 1.0);
						if (chan_num == 0)
							color = fluo::Color(1.0, 0.0, 0.0);
						else if (chan_num == 1)
							color = fluo::Color(0.0, 1.0, 0.0);
						else if (chan_num == 2)
							color = fluo::Color(0.0, 0.0, 1.0);

						if (chan_num >= 0 && chan_num < 3)
							vd_add->SetColor(color);

						canvas->AddVolumeData(vd_add);
					}
				}
			}
		}
		//update
		canvas->InitView(INIT_BOUNDS | INIT_CENTER | INIT_TRANSL | INIT_ROTATE);
	}

	vrv->LoadSettings();

	UpdateProps({ gstMovViewList, gstMovViewIndex, gstTreeCtrl });

	return vrv->GetName();
}

RenderCanvas* MainFrame::GetLastView()
{
	return m_vrv_list[m_vrv_list.size() - 1]->m_canvas;
}

//views
int MainFrame::GetViewNum()
{
	return m_vrv_list.size();
}

RenderCanvas* MainFrame::GetView(int index)
{
	if (index >= 0 && index < (int)m_vrv_list.size())
	{
		RenderViewPanel* v = m_vrv_list[index];
		if (v)
			return v->m_canvas;
	}
	return 0;
}

RenderCanvas* MainFrame::GetView(wxString& name)
{
	for (size_t i=0; i < m_vrv_list.size(); ++i)
	{
		RenderViewPanel* v = m_vrv_list[i];
		if (v && v->GetName() == name)
			return v->m_canvas;
	}
	return 0;
}

int MainFrame::GetView(RenderCanvas* canvas)
{
	if (!canvas)
		return 0;
	if (m_vrv_list.size() == 1)
		return 0;

	for (size_t i = 0; i < m_vrv_list.size(); ++i)
	{
		RenderViewPanel* v = m_vrv_list[i];
		if (v && v->m_canvas == canvas)
			return i;
	}

	return 0;
}

void MainFrame::OnNewView(wxCommandEvent& event)
{
	wxString str = CreateView();
}

void MainFrame::OnLayout(wxCommandEvent& event)
{
	OrganizeVRenderViews(1);
}

void MainFrame::OnReset(wxCommandEvent& event)
{
	ResetLayout();
}

void MainFrame::OnFullScreen(wxCommandEvent& event)
{
	if (IsFullScreen())
	{
		ShowFullScreen(false);
		wxMenuItem* m = m_top_window->FindItem(ID_FullScreen);
		if (m)
			m->SetBitmap(wxGetBitmapFromMemory(full_screen_menu));
	}
	else
	{
		ShowFullScreen(true,
			wxFULLSCREEN_NOBORDER |
			wxFULLSCREEN_NOCAPTION);
		wxMenuItem* m = m_top_window->FindItem(ID_FullScreen);
		if (m)
			m->SetBitmap(wxGetBitmapFromMemory(full_screen_back_menu));
	}
}

//open dialog options
void MainFrame::OnCh11Check(wxCommandEvent &event)
{
	wxCheckBox* ch11 = (wxCheckBox*)event.GetEventObject();
	if (ch11)
		glbin_settings.m_slice_sequence = ch11->GetValue();
}

void MainFrame::OnCh12Check(wxCommandEvent &event)
{
	wxCheckBox* ch12 = (wxCheckBox*)event.GetEventObject();
	if (ch12)
		glbin_settings.m_chann_sequence = ch12->GetValue();
}

void MainFrame::OnCmbChange(wxCommandEvent &event)
{
	wxComboBox* combo = (wxComboBox*)event.GetEventObject();
	if (combo)
		glbin_settings.m_digit_order = combo->GetSelection();
}

void MainFrame::OnTxt1Change(wxCommandEvent &event)
{
	wxTextCtrl* txt1 = (wxTextCtrl*)event.GetEventObject();
	if (txt1)
		glbin_settings.m_time_id = txt1->GetValue();
}

void MainFrame::OnTxt2Change(wxCommandEvent &event)
{
	wxTextCtrl* txt2 = (wxTextCtrl*)event.GetEventObject();
	if (txt2)
	{
		wxString str = txt2->GetValue();
		long lval;
		if (str.ToLong(&lval))
			glbin_settings.m_ser_num = lval;
	}
}

void MainFrame::OnCh2Check(wxCommandEvent &event)
{
	wxCheckBox* ch2 = (wxCheckBox*)event.GetEventObject();
	if (ch2)
		glbin_settings.m_realtime_compress = ch2->GetValue();
}

void MainFrame::OnCh3Check(wxCommandEvent &event)
{
	wxCheckBox* ch3 = (wxCheckBox*)event.GetEventObject();
	if (ch3)
		glbin_settings.m_skip_brick = ch3->GetValue();
}

wxWindow* MainFrame::CreateExtraControlVolume(wxWindow* parent)
{
	wxPanel* panel = new wxPanel(parent);
#ifdef _DARWIN
	panel->SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#endif
	wxStaticText *st1, *st2;

	wxBoxSizer *group1 = new wxStaticBoxSizer(
		new wxStaticBox(panel, wxID_ANY, "Additional Options"), wxVERTICAL );

	//slice sequence check box
	wxCheckBox* ch11 = new wxCheckBox(panel, ID_READ_ZSLICE,
		"Read file# as Z sections");
	ch11->Connect(ch11->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(MainFrame::OnCh11Check), NULL, panel);
	ch11->SetValue(glbin_settings.m_slice_sequence);

	//slice sequence check box
	wxCheckBox* ch12 = new wxCheckBox(panel, ID_READ_CHANN,
		"Read file# as channels");
	ch12->Connect(ch12->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(MainFrame::OnCh12Check), NULL, panel);
	ch12->SetValue(glbin_settings.m_chann_sequence);

	//digit order
	st1 = new wxStaticText(panel, 0,
		"Digit order:");
	wxComboBox* combo = new wxComboBox(panel, ID_DIGI_ORDER,
		"Order", wxDefaultPosition, parent->FromDIP(wxSize(100, 20)), 0, NULL, wxCB_READONLY);
	combo->Connect(combo->GetId(), wxEVT_COMMAND_COMBOBOX_SELECTED,
		wxCommandEventHandler(MainFrame::OnCmbChange), NULL, panel);
	std::vector<std::string> combo_list;
	combo_list.push_back("Channel first");
	combo_list.push_back("Z section first");
	for (size_t i = 0; i < combo_list.size(); ++i)
		combo->Append(combo_list[i]);
	combo->SetSelection(glbin_settings.m_digit_order);

	//series number
	st2 = new wxStaticText(panel, 0,
		"Serial:");
	wxTextCtrl* txt2 = new wxTextCtrl(panel, ID_SER_NUM,
		"", wxDefaultPosition, parent->FromDIP(wxSize(40, 20)), wxTE_RIGHT);
	txt2->Connect(txt2->GetId(), wxEVT_COMMAND_TEXT_UPDATED,
		wxCommandEventHandler(MainFrame::OnTxt2Change), NULL, panel);

	wxBoxSizer* sizer1 = new wxBoxSizer(wxHORIZONTAL);
	sizer1->Add(ch11);
	sizer1->Add(10, 10);
	sizer1->Add(ch12);
	sizer1->Add(10, 10);
	sizer1->Add(st1);
	sizer1->Add(5, 5);
	sizer1->Add(combo, 0, wxALIGN_TOP);
	sizer1->Add(10, 10);
	sizer1->Add(st2);
	sizer1->Add(5, 5);
	sizer1->Add(txt2, 0, wxALIGN_CENTER);

	//compression
	wxCheckBox* ch2 = new wxCheckBox(panel, ID_COMPRESS,
		"Compress data (loading will take longer time and data are compressed in graphics memory)");
	ch2->Connect(ch2->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(MainFrame::OnCh2Check), NULL, panel);
	ch2->SetValue(glbin_settings.m_realtime_compress);

	//empty brick skipping
	wxCheckBox* ch3 = new wxCheckBox(panel, ID_SKIP_BRICKS,
		"Skip empty bricks during rendering (loading takes longer time)");
	ch3->Connect(ch3->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(MainFrame::OnCh3Check), NULL, panel);
	ch3->SetValue(glbin_settings.m_skip_brick);

	//time sequence identifier
	wxBoxSizer* sizer2 = new wxBoxSizer(wxHORIZONTAL);
	wxTextCtrl* txt1 = new wxTextCtrl(panel, ID_TSEQ_ID,
		"", wxDefaultPosition, parent->FromDIP(wxSize(40, 20)));
	txt1->ChangeValue(glbin_settings.m_time_id);
	txt1->Connect(txt1->GetId(), wxEVT_COMMAND_TEXT_UPDATED,
		wxCommandEventHandler(MainFrame::OnTxt1Change), NULL, panel);
	st1 = new wxStaticText(panel, 0,
		"Time sequence identifier (digits after the identifier in filenames are used as time index)\n");
	sizer2->Add(txt1);
	sizer2->Add(10, 10);
	sizer2->Add(st1);

	group1->Add(10, 10);
	group1->Add(sizer1);
	group1->Add(10, 10);
	group1->Add(ch2);
	group1->Add(10, 10);
	group1->Add(ch3);
	group1->Add(10, 10);
	group1->Add(sizer2);
	group1->Add(10, 10);

	panel->SetSizerAndFit(group1);
	panel->Layout();

	return panel;
}

wxWindow* MainFrame::CreateExtraControlVolumeForImport(wxWindow* parent)
{
	wxPanel* panel = new wxPanel(parent);
#ifdef _DARWIN
	panel->SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#endif
	wxBoxSizer *group1 = new wxStaticBoxSizer(
		new wxStaticBox(panel, wxID_ANY, "Additional Options"), wxVERTICAL);

	//compression
	wxCheckBox* ch2 = new wxCheckBox(panel, ID_COMPRESS,
		"Compress data (loading will take longer time and data are compressed in graphics memory)");
	ch2->Connect(ch2->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(MainFrame::OnCh2Check), NULL, panel);
	ch2->SetValue(glbin_settings.m_realtime_compress);

	//empty brick skipping
	wxCheckBox* ch3 = new wxCheckBox(panel, ID_SKIP_BRICKS,
		"Skip empty bricks during rendering (loading takes longer time)");
	ch3->Connect(ch3->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(MainFrame::OnCh3Check), NULL, panel);
	ch3->SetValue(glbin_settings.m_skip_brick);

	group1->Add(10, 10);
	group1->Add(ch2);
	group1->Add(10, 10);
	group1->Add(ch3);
	group1->Add(10, 10);

	panel->SetSizerAndFit(group1);
	panel->Layout();

	return panel;
}

void MainFrame::OnOpenVolume(wxCommandEvent& event)
{
	wxFileDialog *fopendlg = new wxFileDialog(
		this, "Choose the volume data file", "", "",
		"All Supported|*.tif;*.tiff;*.lif;*.lof;*.nd2;*.oib;*.oif;*.xml;*.lsm;*.czi;*.nrrd;*.vvd;*.mp4;*.m4v;*.mov;*.avi;*.wmv|"\
		"Tiff Files (*.tif, *.tiff)|*.tif;*.tiff|"\
		"Leica Image File Format (*.lif)|*.lif|"\
		"Leica Microsystems Object File Format (*.lof)|*.lof|"\
		"Nikon ND2 File Format (*.nd2)|*.nd2|"\
		"Olympus Image Binary Files (*.oib)|*.oib|"\
		"Olympus Original Imaging Format (*.oif)|*.oif|"\
		"Bruker/Prairie View XML (*.xml)|*.xml|"\
		"Zeiss Laser Scanning Microscope (*.lsm)|*.lsm|"\
		"Zeiss ZISRAW File Format (*.czi)|*.czi|"\
		"Utah Nrrd files (*.nrrd)|*.nrrd|"\
		"Janelia Brick files (*.vvd)|*.vvd|"\
		"Video files (*.mp4, *.m4v, *.mov, *.avi, *.wmv)|*.mp4;*.m4v;*.mov;*.avi;*.wmv",
		wxFD_OPEN|wxFD_MULTIPLE);
	fopendlg->SetExtraControlCreator(CreateExtraControlVolume);

	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		RenderCanvas* canvas = GetView(0);

		wxArrayString paths;
		fopendlg->GetPaths(paths);
		LoadVolumes(paths, false, canvas);
	}

	delete fopendlg;
}

void MainFrame::OnImportVolume(wxCommandEvent& event)
{
	wxFileDialog *fopendlg = new wxFileDialog(
		this, "Choose the volume data file", "", "", "All Files|*.*",
		wxFD_OPEN | wxFD_MULTIPLE);
	fopendlg->SetExtraControlCreator(CreateExtraControlVolumeForImport);

	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		RenderCanvas* canvas = GetView(0);

		wxArrayString paths;
		fopendlg->GetPaths(paths);
		LoadVolumes(paths, true, canvas);
	}

	delete fopendlg;
}

void MainFrame::LoadVolumes(wxArrayString files, bool withImageJ, RenderCanvas* canvas)
{
	int j;

	VolumeData* vd_sel = 0;
	DataGroup* group_sel = 0;
	RenderCanvas* v = 0;

	if (canvas)
		v = canvas;
	else
		v = GetView(0);

	bool refresh = false;
	fluo::ValueCollection vc;
	wxProgressDialog *prg_diag = 0;
	if (v)
	{
		bool streaming = glbin_settings.m_mem_swap;
		double gpu_size = glbin_settings.m_graphics_mem;
		double data_size = glbin_settings.m_large_data_size;
		int brick_size = glbin_settings.m_force_brick_size;
		int resp_time = glbin_settings.m_up_time;
		wxString str_streaming;
		if (streaming)
		{
			str_streaming = "Large data streaming is currently ON\n";
			str_streaming += wxString::Format("FluoRender uses up to %dMB GPU memory\n", int(std::round(gpu_size)));
			str_streaming += wxString::Format("Data >%dMB are divided into %d voxel bricks\n",
				int (data_size), brick_size);
			str_streaming += wxString::Format("System response time: %dms", resp_time);
		}
		else
			str_streaming = "Large data streaming is currently OFF";

		prg_diag = new wxProgressDialog(
			"FluoRender: Loading volume data...",
			"",
			100, this, wxPD_SMOOTH|wxPD_ELAPSED_TIME|wxPD_AUTO_HIDE);

		bool enable_4d = false;

		for (j=0; j<(int)files.Count(); j++)
		{
			wxGetApp().Yield();
			prg_diag->Update(90*(j+1)/(int)files.Count(),
				str_streaming);
			prg_diag->CenterOnParent();

			int ch_num = 0;
			wxString filename = files[j];
			wxString suffix = filename.Mid(filename.Find('.', true)).MakeLower();

			if (withImageJ)
				ch_num = glbin_data_manager.LoadVolumeData(filename, LOAD_TYPE_IMAGEJ, true); //The type of data doesnt matter.
			else if (suffix == ".nrrd" || suffix == ".msk" || suffix == ".lbl")
				ch_num = glbin_data_manager.LoadVolumeData(filename, LOAD_TYPE_NRRD, false);
			else if (suffix == ".tif" || suffix == ".tiff")
				ch_num = glbin_data_manager.LoadVolumeData(filename, LOAD_TYPE_TIFF, false);
			else if (suffix == ".oib")
				ch_num = glbin_data_manager.LoadVolumeData(filename, LOAD_TYPE_OIB, false);
			else if (suffix == ".oif")
				ch_num = glbin_data_manager.LoadVolumeData(filename, LOAD_TYPE_OIF, false);
			else if (suffix == ".lsm")
				ch_num = glbin_data_manager.LoadVolumeData(filename, LOAD_TYPE_LSM, false);
			else if (suffix == ".xml")
				ch_num = glbin_data_manager.LoadVolumeData(filename, LOAD_TYPE_PVXML, false);
			else if (suffix == ".vvd")
				ch_num = glbin_data_manager.LoadVolumeData(filename, LOAD_TYPE_BRKXML, false);
			else if (suffix == ".czi")
				ch_num = glbin_data_manager.LoadVolumeData(filename, LOAD_TYPE_CZI, false);
			else if (suffix == ".nd2")
				ch_num = glbin_data_manager.LoadVolumeData(filename, LOAD_TYPE_ND2, false);
			else if (suffix == ".lif")
				ch_num = glbin_data_manager.LoadVolumeData(filename, LOAD_TYPE_LIF, false);
			else if (suffix == ".lof")
				ch_num = glbin_data_manager.LoadVolumeData(filename, LOAD_TYPE_LOF, false);
			else if (suffix == ".mp4" || suffix == ".m4v" || suffix == ".mov" || suffix == ".avi" || suffix == ".wmv")
				ch_num = glbin_data_manager.LoadVolumeData(filename, LOAD_TYPE_MPG, false);

			if (ch_num > 1)
			{
				DataGroup* group = v->AddOrGetGroup();
				if (group)
				{
					for (int i=ch_num; i>0; i--)
					{
						VolumeData* vd = glbin_data_manager.GetVolumeData(glbin_data_manager.GetVolumeNum()-i);
						if (vd)
						{
							v->AddVolumeData(vd, group->GetName());
							wxString vol_name = vd->GetName();
							if (vol_name.Find("_1ch")!=-1 &&
								(i==1 || i==2))
								vd->SetDisp(false);
							if (vol_name.Find("_2ch")!=-1 && i==1)
								vd->SetDisp(false);

							if (i==ch_num)
							{
								vd_sel = vd;
								group_sel = group;
							}

							if (vd->GetReader() && vd->GetReader()->GetTimeNum()>1)
								enable_4d = true;
						}
					}
					if (j > 0)
						group->SetDisp(false);
				}
			}
			else if (ch_num == 1)
			{
				VolumeData* vd = glbin_data_manager.GetVolumeData(glbin_data_manager.GetVolumeNum()-1);
				if (vd)
				{
					if (!vd->GetWlColor())
					{
						int chan_num = v->GetDispVolumeNum();
						fluo::Color color(1.0, 1.0, 1.0);
						if (chan_num == 0)
							color = fluo::Color(1.0, 0.0, 0.0);
						else if (chan_num == 1)
							color = fluo::Color(0.0, 1.0, 0.0);
						else if (chan_num == 2)
							color = fluo::Color(0.0, 0.0, 1.0);

						if (chan_num >= 0 && chan_num < 3)
							vd->SetColor(color);
						else
							vd->RandomizeColor();
					}

					v->AddVolumeData(vd);
					vd_sel = vd;

					if (vd->GetReader() && vd->GetReader()->GetTimeNum()>1)
					{
						v->m_tseq_cur_num = vd->GetReader()->GetCurTime();
						enable_4d = true;
					}
				}
			}
			else { //TODO: Consult Wan here.

			}
		}

		//UpdateList();
		vc.insert(gstListCtrl);
		vc.insert(gstTreeCtrl);
		if (vd_sel)
			glbin.set_tree_selection(vd_sel->GetName().ToStdString());
		else
			glbin.set_tree_selection("");
		v->InitView(INIT_BOUNDS|INIT_CENTER);
		refresh = true;
		vc.insert(gstScaleFactor);

		if (enable_4d)
		{
			glbin_moviemaker.SetSeqMode(1);
			vc.insert(gstMovieAgent);
		}

		delete prg_diag;
	}

	if (refresh)
	{
		RefreshCanvases({ GetView(v) });
		UpdateProps(vc);
	}
}

void MainFrame::StartupLoad(wxArrayString files, bool run_mov, bool with_imagej)
{
	if (m_vrv_list[0])
		m_vrv_list[0]->m_canvas->Init();

	if (files.Count())
	{
		wxString filename = files[0];
		wxString suffix = filename.Mid(filename.Find('.', true)).MakeLower();

		if (suffix == ".vrp")
		{
			OpenProject(files[0]);
		}
		else if (suffix == ".nrrd" ||
			suffix == ".msk" ||
			suffix == ".lbl" ||
			suffix == ".tif" ||
			suffix == ".tiff" ||
			suffix == ".oib" ||
			suffix == ".oif" ||
			suffix == ".lsm" ||
			suffix == ".xml" ||
			suffix == ".vvd" ||
#ifndef _DARWIN
			suffix == ".nd2" ||
#endif
			suffix == ".czi" ||
			suffix == ".lif" ||
			suffix == ".lof" ||
			suffix == ".mp4" ||
			suffix == ".m4v" ||
			suffix == ".mov" ||
			suffix == ".avi" ||
			suffix == ".wmv")
		{
			LoadVolumes(files, with_imagej);
		}
		else if (suffix == ".obj")
		{
			LoadMeshes(files);
		}
		else if (with_imagej)
		{
			LoadVolumes(files, with_imagej);
		}
	}

	if (run_mov)
	{
		glbin_moviemaker.SetFileName(glbin_settings.m_mov_filename);
		glbin_moviemaker.PlaySave();
	}
}

void MainFrame::LoadMeshes(wxArrayString files, RenderCanvas* canvas)
{
	if (!canvas)
		canvas = GetView(0);

	MeshData* md_sel = 0;

	wxProgressDialog *prg_diag = new wxProgressDialog(
		"FluoRender: Loading mesh data...",
		"Reading and processing selected mesh data. Please wait.",
		100, this, wxPD_SMOOTH|wxPD_ELAPSED_TIME|wxPD_AUTO_HIDE);

	MeshGroup* group = 0;
	if (files.Count() > 1)
		group = canvas->AddOrGetMGroup();

	for (int i=0; i<(int)files.Count(); i++)
	{
		prg_diag->Update(90*(i+1)/(int)files.Count());

		wxString filename = files[i];
		glbin_data_manager.LoadMeshData(filename);

		MeshData* md = glbin_data_manager.GetLastMeshData();
		if (canvas && md)
		{
			if (group)
			{
				group->InsertMeshData(group->GetMeshNum()-1, md);
				canvas->SetMeshPopDirty();
			}
			else
				canvas->AddMeshData(md);

			if (i==int(files.Count()-1))
				md_sel = md;
		}
	}

	//UpdateList();
	if (md_sel)
		glbin.set_tree_selection(md_sel->GetName().ToStdString());
	else
		glbin.set_tree_selection("");

	if (canvas)
		canvas->InitView(INIT_BOUNDS|INIT_CENTER);

	RefreshCanvases({ GetView(canvas) });
	UpdateProps({ gstListCtrl, gstTreeCtrl });

	delete prg_diag;
}

void MainFrame::OnOpenMesh(wxCommandEvent& event)
{
	wxFileDialog *fopendlg = new wxFileDialog(
		this, "Choose the mesh data file",
		"", "", "*.obj", wxFD_OPEN|wxFD_MULTIPLE);

	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		RenderCanvas* canvas = GetView(0);
		wxArrayString files;
		fopendlg->GetPaths(files);

		LoadMeshes(files, canvas);
	}

	if (fopendlg)
		delete fopendlg;
}

void MainFrame::OnOrganize(wxCommandEvent& event)
{
	int w, h;
	GetClientSize(&w, &h);
	int view_num = m_vrv_list.size();
	if (view_num>1)
	{
		for (int i=0 ; i<view_num ; i++)
		{
			m_aui_mgr.GetPane(m_vrv_list[i]->GetName()).Float();
		}
		m_aui_mgr.Update();
	}
}

void MainFrame::OnCheckUpdates(wxCommandEvent &)
{
	::wxLaunchDefaultBrowser(VERSION_UPDATES);
}

void MainFrame::OnInfo(wxCommandEvent& event)
{

	wxString time = wxNow();
	int psJan = time.Find("Jan");
	int psDec = time.Find("Dec");
	wxDialog* d = new wxDialog(this,wxID_ANY,"About FluoRender",wxDefaultPosition,
		FromDIP(wxSize(600,200)),wxDEFAULT_DIALOG_STYLE );
	wxBoxSizer * main = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer * left = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer * right = new wxBoxSizer(wxVERTICAL);
	//left
	// FluoRender Image (rows 4-5)
	wxToolBar * logo= new wxToolBar(d, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	wxBitmap bitmap;
	if (psJan!=wxNOT_FOUND || psDec!=wxNOT_FOUND)
		bitmap = wxGetBitmapFromMemory(logo_snow);
	else
		bitmap = wxGetBitmapFromMemory(logo);
#ifdef _DARWIN
	logo->SetToolBitmapSize(bitmap.GetSize());
#endif
	logo->AddTool(wxID_ANY, "", bitmap);
	logo->Realize();
	left->Add(logo,0,wxEXPAND);
	//right
	wxStaticText *txt = new wxStaticText(d,wxID_ANY,FLUORENDER_TITLE,
		wxDefaultPosition, FromDIP(wxSize(-1,-1)));
	wxFont font = wxFont(15,wxFONTFAMILY_ROMAN,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL );
	txt->SetFont(font);
	right->Add(txt,0,wxEXPAND);
	txt = new wxStaticText(d,wxID_ANY,"Version: " +
		wxString::Format("%d.%.1f", VERSION_MAJOR, float(VERSION_MINOR)),
		wxDefaultPosition, FromDIP(wxSize(50,-1)));
	font = wxFont(12,wxFONTFAMILY_ROMAN,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL );
	txt->SetFont(font);
	right->Add(txt,0,wxEXPAND);
	txt = new wxStaticText(d,wxID_ANY,wxString("Copyright (c) ") + VERSION_COPYRIGHT,
		wxDefaultPosition, FromDIP(wxSize(-1,-1)));
	font = wxFont(11,wxFONTFAMILY_ROMAN,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL );
	txt->SetFont(font);
	right->Add(txt,0,wxEXPAND);
	right->Add(3,5,0);
	txt = new wxStaticText(d,wxID_ANY,VERSION_AUTHORS,
		wxDefaultPosition, FromDIP(wxSize(300,-1)));
	font = wxFont(7,wxFONTFAMILY_ROMAN,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL );
	txt->SetFont(font);
	right->Add(txt,0,wxEXPAND);
	wxHyperlinkCtrl* hyp = new wxHyperlinkCtrl(d,wxID_ANY,"Contact Info",
		VERSION_CONTACT,
		wxDefaultPosition, FromDIP(wxSize(-1,-1)));
	right->Add(hyp,0,wxEXPAND);
	right->AddStretchSpacer();
	//put together
	main->Add(left,0,wxEXPAND);
	main->Add(right,0,wxEXPAND);
	d->SetSizer(main);
	d->ShowModal();
}

bool MainFrame::GetBenchmark()
{
	return m_benchmark;
}

void MainFrame::ClearVrvList()
{
	m_vrv_list.clear();
}

wxString MainFrame::ScriptDialog(const wxString& title,
	const wxString& wildcard, long style)
{
	glbin_moviemaker.Hold();
	wxString result;
	wxFileDialog* dlg = new wxFileDialog(
		this, title, "", "",
		wildcard, style);
	int rval = dlg->ShowModal();
	if (rval == wxID_OK)
		result = dlg->GetPath();
	delete dlg;
	glbin_moviemaker.Resume();
	return result;
}

TreePanel *MainFrame::GetTree()
{
	return m_tree_panel;
}

ListPanel *MainFrame::GetList()
{
	return m_list_panel;
}

//on selections
void MainFrame::OnSelection(int type,
	RenderCanvas* canvas,
	DataGroup* group,
	VolumeData* vd,
	MeshData* md,
	Annotations* ann)
{
	if (canvas)
	{
		glbin_ruler_handler.SetView(canvas);
		glbin_ruler_handler.SetRulerList(canvas->GetRulerList());
		glbin_ruler_renderer.SetView(canvas);
		glbin_ruler_renderer.SetRulerList(canvas->GetRulerList());
		glbin_volume_point.SetView(canvas);
		glbin_vol_selector.SetView(canvas);
		glbin_vol_calculator.SetView(canvas);
		glbin_script_proc.SetView(canvas);
	}

	if (m_clip_view)
	{
		switch (type)
		{
		case 2:
			m_clip_view->SetVolumeData(vd);
			break;
		case 3:
			m_clip_view->SetMeshData(md);
			break;
		case 4:
			if (ann)
			{
				VolumeData* vd_ann = ann->GetVolume();
				m_clip_view->SetVolumeData(vd_ann);
			}
			break;
		}
		m_clip_view->SetRenderView(canvas);
	}

	m_cur_sel_type = type;
	//clear mesh boundbox
	if (glbin_data_manager.GetMeshData(m_cur_sel_mesh))
		glbin_data_manager.GetMeshData(m_cur_sel_mesh)->SetDrawBounds(false);

	if (m_brush_tool_dlg)
		m_brush_tool_dlg->GetSettings(canvas);
	if (m_colocalization_dlg)
		m_colocalization_dlg->SetView(canvas);
	if (m_component_dlg)
		m_component_dlg->SetView(canvas);
	if (m_counting_dlg)
		m_counting_dlg->GetSettings(canvas);
	if (m_measure_dlg)
		m_measure_dlg->GetSettings(canvas);
	if (m_noise_cancelling_dlg)
		m_noise_cancelling_dlg->GetSettings(canvas);
	if (m_ocl_dlg)
		m_ocl_dlg->GetSettings(canvas);
	if (m_trace_dlg)
		m_trace_dlg->GetSettings(canvas);

	switch (type)
	{
	case 0:  //root
		break;
	case 1:  //view
		if (m_colocalization_dlg)
			m_colocalization_dlg->SetGroup(group);
		if (m_adjust_view)
			m_adjust_view->SetRenderView(canvas);
		//m_aui_mgr.GetPane(m_prop_panel).Caption(UITEXT_PROPERTIES);
		//m_aui_mgr.Update();
		break;
	case 2:  //volume
		if (vd && vd->GetDisp())
		{
			wxString str = vd->GetName();
			ShowPropPage(2, canvas, group, vd, md, ann);
			m_cur_sel_vol = glbin_data_manager.GetVolumeIndex(str);

			for (size_t i=0; i< GetViewNum(); ++i)
			{
				RenderCanvas* v = GetView(i);
				if (!v)
					continue;
				v->m_cur_vol = vd;
			}
		}
		if (m_adjust_view)
			m_adjust_view->SetVolumeData(vd);
		if (m_colocalization_dlg)
			m_colocalization_dlg->SetGroup(group);
		break;
	case 3:  //mesh
		if (md)
		{
			wxString str = md->GetName();
			ShowPropPage(3, canvas, group, vd, md, ann);
			m_cur_sel_mesh = glbin_data_manager.GetMeshIndex(str);
			md->SetDrawBounds(true);
		}
		if (m_colocalization_dlg)
			m_colocalization_dlg->SetGroup(0);
		break;
	case 4:  //annotations
		if (ann)
		{
			wxString str = ann->GetName();
			ShowPropPage(4, canvas, group, vd, md, ann);
		}
		if (m_colocalization_dlg)
			m_colocalization_dlg->SetGroup(0);
		break;
	case 5:  //group
		if (m_adjust_view)
			m_adjust_view->SetGroup(group);
		if (m_calculation_dlg)
			m_calculation_dlg->SetGroup(group);
		if (m_colocalization_dlg)
			m_colocalization_dlg->SetGroup(group);
		//m_aui_mgr.GetPane(m_prop_panel).Caption(UITEXT_PROPERTIES);
		//m_aui_mgr.Update();
		break;
	case 6:  //mesh manip
		if (md)
		{
			wxString str = md->GetName();
			ShowPropPage(6, canvas, group, vd, md, ann);
		}
		if (m_colocalization_dlg)
			m_colocalization_dlg->SetGroup(0);
		break;
	default:
		if (m_colocalization_dlg)
			m_colocalization_dlg->SetGroup(0);
		//m_aui_mgr.GetPane(m_prop_panel).Caption(UITEXT_PROPERTIES);
		//m_aui_mgr.Update();
	}
}

wxWindow* MainFrame::AddProps(int type,
	RenderCanvas* canvas,
	DataGroup* group,
	VolumeData* vd,
	MeshData* md,
	Annotations* ann)
{
	//wxString str;
	//if (canvas)
	//	str = canvas->GetName() + ":";
	wxWindow* result = 0;
	switch (type)
	{
	case 2://volume
		if (vd)
		{
			VolumePropPanel* pane = new VolumePropPanel(this, m_prop_panel);
			pane->SetVolumeData(vd);
			pane->SetGroup(group);
			pane->SetView(canvas);
			pane->SetName(vd->GetName());
			pane->Hide();
			m_prop_pages.push_back(pane);
			result = pane;
		}
		break;
	case 3://mesh
		if (md)
		{
			MeshPropPanel* pane = new MeshPropPanel(this, m_prop_panel);
			pane->SetMeshData(md);
			//pane->SetMGroup(group);
			pane->SetView(canvas);
			pane->SetName(md->GetName());
			pane->Hide();
			m_prop_pages.push_back(pane);
			result = pane;
		}
		break;
	case 4://annotations
		if (ann)
		{
			AnnotatPropPanel* pane = new AnnotatPropPanel(this, m_prop_panel);
			pane->SetAnnotations(ann);
			pane->SetName(md->GetName());
			pane->Hide();
			m_prop_pages.push_back(pane);
			result = pane;
		}
		break;
	case 6://mesh manip
		if (md)
		{
			ManipPropPanel* pane = new ManipPropPanel(this, m_prop_panel);
			pane->SetMeshData(md);
			pane->GetData();
			pane->SetName(md->GetName() + " M");
			pane->Hide();
			m_prop_pages.push_back(pane);
			result = pane;
		}
		break;
	default:
		break;
	}

	return result;
}

void MainFrame::DeleteProps(int type, const wxString& name)
{
	//find page
	wxWindow* page = 0;
	switch (type)
	{
		case 2://volume
			page = FindVolumeProps(name);
			break;
		case 3://mesh
			page = FindMeshProps(name);
			break;
		case 4://annotations
			page = FindAnnotationProps(name);
			break;
		case 6://mesh manip
			page = FindMeshManip(name + " M");
			break;
	}
	if (!page)
		return;

	auto it = std::find(m_prop_pages.begin(), m_prop_pages.end(), page);
	if (it != m_prop_pages.end())
	{
		m_prop_pages.erase(it);
		int page_no = m_prop_panel->FindPage(page);
		if (page_no != wxNOT_FOUND)
			m_prop_panel->DeletePage(page_no);
		//delete page;
	}
}

void MainFrame::ShowPropPage(int type,
	RenderCanvas* canvas,
	DataGroup* group,
	VolumeData* vd,
	MeshData* md,
	Annotations* ann,
	bool show )
{
	//find page
	wxWindow* page = 0;
	wxString name;
	switch (type)
	{
	case 2://volume
		if (vd)
		{
			name = vd->GetName();
			page = FindVolumeProps(name);
		}
		break;
	case 3://mesh
		if (md)
		{
			name = md->GetName();
			page = FindMeshProps(name);
		}
		break;
	case 4://annotations
		if (ann)
		{
			name = ann->GetName();
			page = FindAnnotationProps(name);
		}
		break;
	case 6://mesh manip
		if (md)
		{
			name = md->GetName();
			page = FindMeshManip(name + " M");
		}
		break;
	}
	if (!page)
	{
		page = AddProps(type, canvas, group, vd, md, ann);
		if (!page)
			return;
	}
	int page_no = m_prop_panel->FindPage(page);
	bool added = page_no != wxNOT_FOUND;

	if (added && show)
		m_prop_panel->SetSelection(page_no);
	if (!added && show)
	{
		m_prop_panel->AddPage(page, page->GetName(), true);
		page->Show();
	}
	if (added && !show)
	{
		page->Hide();
		m_prop_panel->RemovePage(page_no);
	}
}

bool MainFrame::update_props(int excl_self, PropPanel* p1, PropPanel* p2)
{
	switch (excl_self)
	{
	case 0:
		return true;
	case 1:
		return p1 != p2;
	case 2:
		return p1 == p2;
	case 3:
		return false;
	}
	return false;
}

void MainFrame::UpdateProps(const fluo::ValueCollection& vc, int excl_self, PropPanel* panel)
{
	//panels
	if (update_props(excl_self, m_list_panel, panel))
		m_list_panel->FluoUpdate(vc);
	if (update_props(excl_self, m_tree_panel, panel))
		m_tree_panel->FluoUpdate(vc);
	if (update_props(excl_self, m_movie_panel, panel))
		m_movie_panel->FluoUpdate(vc);
	if (update_props(excl_self, m_adjust_view, panel))
		m_adjust_view->FluoUpdate(vc);
	if (update_props(excl_self, m_clip_view, panel))
		m_clip_view->FluoUpdate(vc);
	for (auto i : m_vrv_list)
		if (update_props(excl_self, i, panel))
			i->FluoUpdate(vc);
	for (auto i : m_prop_pages)
		if (update_props(excl_self, i, panel))
			i->FluoUpdate(vc);
	//dialogs
	if (update_props(excl_self, m_setting_dlg, panel))
		m_setting_dlg->FluoUpdate(vc);
}

VolumePropPanel* MainFrame::FindVolumeProps(const wxString& name)
{
	VolumePropPanel* result = 0;
	for (auto i : m_prop_pages)
	{
		if (i && i->GetName() == name)
		{
			result = dynamic_cast<VolumePropPanel*>(i);
			if (result)
				return result;
		}
	}
	return 0;
}

VolumePropPanel* MainFrame::FindVolumeProps(VolumeData* vd)
{
	if (vd)
		return FindVolumeProps(vd->GetName());
	return 0;
}

MeshPropPanel* MainFrame::FindMeshProps(const wxString& name)
{
	MeshPropPanel* result = 0;
	for (auto i : m_prop_pages)
	{
		if (i && i->GetName() == name)
		{
			result = dynamic_cast<MeshPropPanel*>(i);
			if (result)
				return result;
		}
	}
	return 0;
}

MeshPropPanel* MainFrame::FindMeshProps(MeshData* md)
{
	if (md)
		return FindMeshProps(md->GetName());
	return 0;
}

AnnotatPropPanel* MainFrame::FindAnnotationProps(const wxString& name)
{
	AnnotatPropPanel* result = 0;
	for (auto i : m_prop_pages)
	{
		if (i && i->GetName() == name)
		{
			result = dynamic_cast<AnnotatPropPanel*>(i);
			if (result)
				return result;
		}
	}
	return 0;
}

AnnotatPropPanel* MainFrame::FindAnnotationProps(Annotations* ad)
{
	if (ad)
		return FindAnnotationProps(ad->GetName());
	return 0;
}

ManipPropPanel* MainFrame::FindMeshManip(const wxString& name)
{
	ManipPropPanel* result = 0;
	for (auto i : m_prop_pages)
	{
		if (i && i->GetName() == name)
		{
			result = dynamic_cast<ManipPropPanel*>(i);
			if (result)
				return result;
		}
	}
	return 0;
}

ManipPropPanel* MainFrame::FindMeshManip(MeshData* md)
{
	if (md)
		return FindMeshManip(md->GetName());
	return 0;
}

//prop view
OutputAdjPanel* MainFrame::GetAdjustView()
{
	return m_adjust_view;
}

//movie view
MoviePanel* MainFrame::GetMovieView()
{
	return m_movie_panel;
}

//system settings
SettingDlg* MainFrame::GetSettingDlg()
{
	return m_setting_dlg;
}

//help dialog
HelpDlg* MainFrame::GetHelpDlg()
{
	return m_help_dlg;
}

//clipping view
ClipPlanePanel* MainFrame::GetClippingView()
{
	return m_clip_view;
}

//brush dialog
BrushToolDlg* MainFrame::GetBrushToolDlg()
{
	return m_brush_tool_dlg;
}

//noise cancelling dialog
NoiseCancellingDlg* MainFrame::GetNoiseCancellingDlg()
{
	return m_noise_cancelling_dlg;
}

//counting dialog
CountingDlg* MainFrame::GetCountingDlg()
{
	return m_counting_dlg;
}

//convert dialog
ConvertDlg* MainFrame::GetConvertDlg()
{
	return m_convert_dlg;
}

ColocalizationDlg* MainFrame::GetColocalizationDlg()
{
	return m_colocalization_dlg;
}

//measure dialog
MeasureDlg* MainFrame::GetMeasureDlg()
{
	return m_measure_dlg;
}

//trace dialog
TraceDlg* MainFrame::GetTraceDlg()
{
	return m_trace_dlg;
}

//ocl dialog
OclDlg* MainFrame::GetOclDlg()
{
	return m_ocl_dlg;
}

//component dialog
ComponentDlg* MainFrame::GetComponentDlg()
{
	return m_component_dlg;
}

//calculation dialog
CalculationDlg* MainFrame::GetCalculationDlg()
{
	return m_calculation_dlg;
}

//script break dialog
ScriptBreakDlg* MainFrame::GetScriptBreakDlg()
{
	return m_script_break_dlg;
}

//selection
int MainFrame::GetCurSelType()
{
	return m_cur_sel_type;
}

//get current selected volume
VolumeData* MainFrame::GetCurSelVol()
{
	return glbin_data_manager.GetVolumeData(m_cur_sel_vol);
}

//get current selected mesh
MeshData* MainFrame::GetCurSelMesh()
{
	return glbin_data_manager.GetMeshData(m_cur_sel_mesh);
}

void MainFrame::RefreshCanvases(const std::set<int>& views)
{
	if (views.find(-1) != views.end())
		return;
	bool update_all = views.empty();

	for (int i=0 ; i<(int)m_vrv_list.size() ; i++)
	{
		if (!m_vrv_list[i])
			continue;

		if (update_all || views.find(i) != views.end())
			m_vrv_list[i]->RefreshGL(false);
	}

	//incase volume color changes
	//change icon color of the tree panel
	//if (tree)
	//	UpdateTreeColors();
}

void MainFrame::DeleteVRenderView(int i)
{
	if (m_vrv_list[i])
	{
		int j;
		wxString str = m_vrv_list[i]->GetName();

		for (j=0 ; j<GetView(i)->GetAllVolumeNum() ; j++)
			GetView(i)->GetAllVolumeData(j)->SetDisp(true);
		for (j=0 ; j< GetView(i)->GetMeshNum() ; j++)
			GetView(i)->GetMeshData(j)->SetDisp(true);
		RenderViewPanel* vrv = m_vrv_list[i];
		m_vrv_list.erase(m_vrv_list.begin()+i);
		m_aui_mgr.DetachPane(vrv);
		vrv->Close();
		delete vrv;
		m_aui_mgr.Update();
		//UpdateTree();

		if (glbin_mov_def.m_view_idx >= i)
			glbin_mov_def.m_view_idx--;
		UpdateProps({ gstTreeCtrl, gstMovViewList, gstMovViewIndex });
	}
}

void MainFrame::DeleteVRenderView(wxString &name)
{
	for (int i=0; i<GetViewNum(); i++)
	{
		RenderCanvas* canvas = GetView(i);
		if (canvas && name == canvas->m_renderview_panel->GetName() && canvas->m_renderview_panel->m_id > 1)
		{
			DeleteVRenderView(i);
			return;
		}
	}
}

//organize render views
void MainFrame::OrganizeVRenderViews(int mode)
{
	int width = 0;
	int height = 0;
	int minx = 0;
	int miny = 0;
	int maxx = 0;
	int maxy = 0;
	int paneNum = (int)m_vrv_list.size();
	int i;
	//get total area
	for (i = 0; i < paneNum; i++)
	{
		RenderViewPanel* vrv = m_vrv_list[i];
		if (vrv && m_aui_mgr.GetPane(vrv).IsOk())
		{
			wxPoint pos = vrv->GetPosition();
			wxSize size = vrv->GetSize();
			//width += size.x;
			//height += size.y;
			int x1 = pos.x;
			int y1 = pos.y;
			int x2 = x1 + size.x;
			int y2 = y1 + size.y;
			if (i == 0)
			{
				minx = x1;
				miny = y1;
				maxx = x2;
				maxy = y2;
			}
			else
			{
				minx = x1 < minx ? x1 : minx;
				miny = y1 < miny ? y1 : miny;
				maxx = x2 > maxx ? x2 : maxx;
				maxy = y2 > maxy ? y2 : maxy;
			}
		}
	}
	if (maxx - minx > 0 && maxy - miny > 0)
	{
		width = maxx - minx;
		height = maxy - miny;
	}
	//detach all panes
	for (i = 0; i < paneNum; ++i)
	{
		RenderViewPanel* vrv = m_vrv_list[i];
		if (vrv)
			m_aui_mgr.DetachPane(vrv);
	}
	//add back
	for (i=0; i<paneNum; i++)
	{
		RenderViewPanel* vrv = m_vrv_list[i];
		if (vrv)
		{
			switch (mode)
			{
			case 0://top-bottom
				vrv->SetSize(width, height / paneNum);
				m_aui_mgr.AddPane(vrv, wxAuiPaneInfo().
					Name(vrv->GetName()).Caption(vrv->GetName()).
					Dockable(true).CloseButton(false).Resizable().
					FloatingSize(width, height / paneNum).
					MinSize(width, height / paneNum).
					MaxSize(width, height / paneNum).
					Layer(0).Centre());
				break;
			case 1://left-right
				vrv->SetSize(width / paneNum, height);
				if (i == 0)
					m_aui_mgr.AddPane(vrv, wxAuiPaneInfo().
						Name(vrv->GetName()).Caption(vrv->GetName()).
						Dockable(true).CloseButton(false).Resizable().
						FloatingSize(width / paneNum, height).
						MinSize(width / paneNum, height).
						MaxSize(width / paneNum, height).
						Layer(0).Centre());
				else
					m_aui_mgr.AddPane(vrv, wxAuiPaneInfo().
						Name(vrv->GetName()).Caption(vrv->GetName()).
						Dockable(true).CloseButton(false).Resizable().
						FloatingSize(width / paneNum, height).
						MinSize(width / paneNum, height).
						MaxSize(width / paneNum, height).
						Layer(0).Centre().Right());
				break;
			}
		}
	}
	m_aui_mgr.Update();
	for (i = 0; i < paneNum; ++i)
	{
		RenderViewPanel* vrv = m_vrv_list[i];
		if (vrv)
		{
			switch (mode)
			{
			case 0://top-bottom
				m_aui_mgr.GetPane(vrv).
					MinSize(-1, -1).MaxSize(-1, -1).
					BestSize(width, height / paneNum);
				break;
			case 1://left-right
				m_aui_mgr.GetPane(vrv).
					MinSize(-1, -1).MaxSize(-1, -1).
					BestSize(width / paneNum, height);
				break;
			}
		}
	}
	m_aui_mgr.Update();
}

//hide/show tools
void MainFrame::ToggleAllTools(bool cur_state)
{
	if (cur_state)
	{
		if (m_aui_mgr.GetPane(m_proj_panel).IsShown() &&
			m_aui_mgr.GetPane(m_movie_panel).IsShown() &&
			m_aui_mgr.GetPane(m_prop_panel).IsShown() &&
			m_aui_mgr.GetPane(m_adjust_view).IsShown() &&
			m_aui_mgr.GetPane(m_clip_view).IsShown())
			m_ui_state = true;
		else if (!m_aui_mgr.GetPane(m_proj_panel).IsShown() &&
			!m_aui_mgr.GetPane(m_movie_panel).IsShown() &&
			!m_aui_mgr.GetPane(m_prop_panel).IsShown() &&
			!m_aui_mgr.GetPane(m_adjust_view).IsShown() &&
			!m_aui_mgr.GetPane(m_clip_view).IsShown())
			m_ui_state = false;
	}

	if (m_ui_state)
	{
		//hide all
		//data view
		m_aui_mgr.GetPane(m_proj_panel).Hide();
		m_tb_menu_ui->Check(ID_UIProjView, false);
		//movie view (float only)
		m_aui_mgr.GetPane(m_movie_panel).Hide();
		m_tb_menu_ui->Check(ID_UIMovieView, false);
		//properties
		m_aui_mgr.GetPane(m_prop_panel).Hide();
		m_tb_menu_ui->Check(ID_UIPropView, false);
		//adjust view
		m_aui_mgr.GetPane(m_adjust_view).Hide();
		m_tb_menu_ui->Check(ID_UIAdjView, false);
		//clipping view
		m_aui_mgr.GetPane(m_clip_view).Hide();
		m_tb_menu_ui->Check(ID_UIClipView, false);

		m_ui_state = false;
	}
	else
	{
		//show all
		//data view
		m_aui_mgr.GetPane(m_proj_panel).Show();
		m_tb_menu_ui->Check(ID_UIProjView, true);
		//movie view (float only)
		m_aui_mgr.GetPane(m_movie_panel).Show();
		m_tb_menu_ui->Check(ID_UIMovieView, true);
		//properties
		m_aui_mgr.GetPane(m_prop_panel).Show();
		m_tb_menu_ui->Check(ID_UIPropView, true);
		//adjust view
		m_aui_mgr.GetPane(m_adjust_view).Show();
		m_tb_menu_ui->Check(ID_UIAdjView, true);
		//clipping view
		m_aui_mgr.GetPane(m_clip_view).Show();
		m_tb_menu_ui->Check(ID_UIClipView, true);

		m_ui_state = true;
	}

	m_aui_mgr.Update();
}

void MainFrame::ShowPane(wxPanel* pane, bool show)
{
	if (m_aui_mgr.GetPane(pane).IsOk())
	{
		if (show)
			m_aui_mgr.GetPane(pane).Show();
		else
			m_aui_mgr.GetPane(pane).Hide();
		m_aui_mgr.Update();
	}
}

//reset layout
void MainFrame::ResetLayout()
{
	m_aui_mgr.GetPane(m_main_tb).Show().Dock().
		Top().Row(1).Resizable();
	m_aui_mgr.GetPane(m_proj_panel).Show().Dock().
		BestSize(FromDIP(wxSize(320, 500))).
		FloatingSize(FromDIP(wxSize(320, 500))).
		Left().Layer(3);
	m_aui_mgr.GetPane(m_movie_panel).Show().Dock().
		BestSize(FromDIP(wxSize(320, 500))).
		FloatingSize(FromDIP(wxSize(320, 500))).
		Left().Layer(3);
	m_aui_mgr.GetPane(m_prop_panel).Show().Dock().
		BestSize(FromDIP(wxSize(1100, 160))).
		FloatingSize(FromDIP(wxSize(1100, 160))).
		Bottom().Layer(2);
	m_aui_mgr.GetPane(m_adjust_view).Show().Dock().
		BestSize(FromDIP(wxSize(150, 800))).
		FloatingSize(FromDIP(wxSize(150, 800))).
		Left().Layer(1);
	m_aui_mgr.GetPane(m_clip_view).Show().Dock().
		BestSize(FromDIP(wxSize(150, 800))).
		FloatingSize(FromDIP(wxSize(150, 800))).
		Right().Show().Dock().Layer(1);
	m_top_window->Check(ID_ShowHideToolbar, true);
	m_top_window->Check(ID_UIProjView, true);
	m_top_window->Check(ID_UIMovieView, true);
	m_top_window->Check(ID_UIAdjView, true);
	m_top_window->Check(ID_UIClipView, true);
	m_top_window->Check(ID_UIPropView, true);
	OrganizeVRenderViews(1);
	glbin_settings.m_layout.Clear();
}

void MainFrame::OnChEmbedCheck(wxCommandEvent &event)
{
	wxCheckBox* ch_embed = (wxCheckBox*)event.GetEventObject();
	if (ch_embed)
		glbin_settings.m_vrp_embed = ch_embed->GetValue();
}

void MainFrame::OnChSaveCmpCheck(wxCommandEvent &event)
{
	wxCheckBox* ch_cmp = (wxCheckBox*)event.GetEventObject();
	if (ch_cmp)
		glbin_settings.m_save_compress = ch_cmp->GetValue();
}

wxWindow* MainFrame::CreateExtraControlProjectSave(wxWindow* parent)
{
	wxPanel* panel = new wxPanel(parent);
#ifdef _DARWIN
	panel->SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#endif
	wxBoxSizer *group1 = new wxStaticBoxSizer(
		new wxStaticBox(panel, wxID_ANY, "Additional Options"), wxVERTICAL );

	//copy all files check box
	wxCheckBox* ch_embed = new wxCheckBox(panel, ID_EMBED_FILES,
		"Embed all files in the project folder");
	ch_embed->Connect(ch_embed->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(MainFrame::OnChEmbedCheck), NULL, panel);
	ch_embed->SetValue(glbin_settings.m_vrp_embed);

	//compressed
	wxCheckBox* ch_cmp = new wxCheckBox(panel, ID_LZW_COMP,
		"Lempel-Ziv-Welch Compression");
	ch_cmp->Connect(ch_cmp->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(MainFrame::OnChSaveCmpCheck), NULL, panel);
	if (ch_cmp)
		ch_cmp->SetValue(glbin_settings.m_save_compress);

	//group
	group1->Add(10, 10);
	group1->Add(ch_embed);
	group1->Add(10, 10);
	group1->Add(ch_cmp);
	group1->Add(10, 10);

	panel->SetSizerAndFit(group1);
	panel->Layout();

	return panel;
}

void MainFrame::OnNewProject(wxCommandEvent& event)
{
	glbin_data_manager.SetProjectPath("");
	SetTitle(m_title);
	//clear
	glbin_data_manager.ClearAll();
	DataGroup::ResetID();
	MeshGroup::ResetID();
	m_adjust_view->SetVolumeData(0);
	m_adjust_view->SetGroup(0);
	m_adjust_view->SetGroupLink(0);
	GetView(0)->ClearAll();
	for (int i = m_vrv_list.size() - 1; i > 0; i--)
		DeleteVRenderView(i);
	RenderViewPanel::ResetID();
	DataGroup::ResetID();
	MeshGroup::ResetID();
	m_cur_sel_type = 0;
	m_cur_sel_vol = 0;
	m_cur_sel_mesh = 0;
	glbin_moviemaker.Stop();
	glbin_moviemaker.SetView(GetView(0));
	glbin_mov_def.Apply(&glbin_moviemaker);
	m_trace_dlg->GetSettings(GetView(0));
	glbin_interpolator.Clear();
	RefreshCanvases();
	UpdateProps({ gstListCtrl, gstTreeCtrl, gstParamList });

}

void MainFrame::OnSaveProject(wxCommandEvent& event)
{
	std::wstring default_path = glbin_data_manager.GetProjectFile().ToStdWstring();
	if (default_path.empty())
	{
		wxCommandEvent e;
		OnSaveAsProject(e);
	}
	else
	{
		wxString filename = default_path;
		bool inc = glbin_settings.m_prj_save_inc;
		SaveProject(filename, inc);
	}
}

void MainFrame::OnSaveAsProject(wxCommandEvent& event)
{
	std::wstring default_path = glbin_data_manager.GetProjectFile().ToStdWstring();
	std::wstring path;
	std::wstring filename;
	bool default_valid = SEP_PATH_NAME(default_path, path, filename);

	wxFileDialog* fopendlg = new wxFileDialog(
		this, "Save Project File",
		default_valid ? path : L"", default_valid ? filename : L"",
		"*.vrp", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	fopendlg->SetExtraControlCreator(CreateExtraControlProjectSave);

	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		wxString filename = fopendlg->GetPath();
		SaveProject(filename, false);
	}

	delete fopendlg;
}

void MainFrame::OnOpenProject(wxCommandEvent& event)
{
	wxFileDialog *fopendlg = new wxFileDialog(
		this, "Choose Project File",
		"", "", "*.vrp", wxFD_OPEN);

	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		wxString path = fopendlg->GetPath();
		OpenProject(path);
	}

	delete fopendlg;
}

void MainFrame::SaveProject(wxString& filename, bool inc)
{
	wxString filename2 = filename;
	if (inc)
		filename2 = INC_NUM_EXIST(filename);

	wxString app_name = "FluoRender " +
		wxString::Format("%d.%.1f", VERSION_MAJOR, float(VERSION_MINOR));
	wxString vendor_name = "FluoRender";
	wxString local_name = filename2;
	wxFileConfig fconfig(app_name, vendor_name, local_name, "",
		wxCONFIG_USE_LOCAL_FILE);

	int i, j, k;
	fconfig.Write("ver_major", VERSION_MAJOR_TAG);
	fconfig.Write("ver_minor", VERSION_MINOR_TAG);

	int ticks = glbin_data_manager.GetVolumeNum() + glbin_data_manager.GetMeshNum();
	int tick_cnt = 1;
	fconfig.Write("ticks", ticks);
	wxProgressDialog *prg_diag = 0;
	prg_diag = new wxProgressDialog(
		"FluoRender: Saving project...",
		"Saving project file. Please wait.",
		100, this, wxPD_SMOOTH|wxPD_ELAPSED_TIME|wxPD_AUTO_HIDE);

	wxString str;

	fconfig.SetPath("/settings");
	//save streaming mode
	fconfig.Write("mouse int", glbin_settings.m_mouse_int);
	fconfig.Write("mem swap", glbin_settings.m_mem_swap);
	fconfig.Write("graphics mem", glbin_settings.m_graphics_mem);
	fconfig.Write("large data size", glbin_settings.m_large_data_size);
	fconfig.Write("force brick size", glbin_settings.m_force_brick_size);
	fconfig.Write("up time", glbin_settings.m_up_time);
	fconfig.Write("update order", glbin_settings.m_update_order);
	fconfig.Write("inf loop", glbin_settings.m_inf_loop);
	//save peeling layers
	fconfig.Write("peeling layers", glbin_settings.m_peeling_layers);

	//save data list
	//volume
	fconfig.SetPath("/data/volume");
	fconfig.Write("num", glbin_data_manager.GetVolumeNum());
	for (i=0; i<glbin_data_manager.GetVolumeNum(); i++)
	{
		if (ticks && prg_diag)
			prg_diag->Update(90*tick_cnt/ticks,
			"Saving volume data. Please wait.");
		tick_cnt++;

		VolumeData* vd = glbin_data_manager.GetVolumeData(i);
		if (vd)
		{
			str = wxString::Format("/data/volume/%d", i);
			//name
			fconfig.SetPath(str);
			str = vd->GetName();
			fconfig.Write("name", str);
			//compression
			fconfig.Write("compression", glbin_settings.m_realtime_compress);
			//skip brick
			fconfig.Write("skip_brick", vd->GetSkipBrick());
			//path
			str = vd->GetPath();
			bool new_chan = false;
			if (str == "" || glbin_settings.m_vrp_embed)
			{
				wxString new_folder;
				new_folder = filename2 + "_files";
				MkDirW(new_folder.ToStdWstring());
				str = new_folder + GETSLASH() + vd->GetName() + ".tif";
				vd->Save(str, 0, 3, false,
					false, 0, false, glbin_settings.m_save_compress,
					fluo::Point(), fluo::Quaternion(), fluo::Point(), false);
				fconfig.Write("path", str);
				new_chan = true;
			}
			else
				fconfig.Write("path", str);
			BaseReader* reader = vd->GetReader();
			if (reader)
			{
				//reader type
				fconfig.Write("reader_type", reader->GetType());
				fconfig.Write("slice_seq", reader->GetSliceSeq());
				str = reader->GetTimeId();
				fconfig.Write("time_id", str);
				//float convert
				fconfig.Write("fp_convert", reader->GetFpConvert());
				double minv, maxv;
				reader->GetFpRange(minv, maxv);
				fconfig.Write("fp_min", minv);
				fconfig.Write("fp_max", maxv);
			}
			else
			{
				fconfig.Write("slice_seq", false);
				fconfig.Write("time_id", "");
			}
			fconfig.Write("cur_time", vd->GetCurTime());
			fconfig.Write("cur_chan", new_chan?0:vd->GetCurChannel());

			//volume properties
			fconfig.SetPath("properties");
			fconfig.Write("display", vd->GetDisp());

			//properties
			fconfig.Write("3dgamma", vd->GetGamma());
			fconfig.Write("boundary", vd->GetBoundary());
			fconfig.Write("contrast", vd->GetSaturation());
			fconfig.Write("left_thresh", vd->GetLeftThresh());
			fconfig.Write("right_thresh", vd->GetRightThresh());
			fluo::Color color = vd->GetColor();
			str = wxString::Format("%f %f %f", color.r(), color.g(), color.b());
			fconfig.Write("color", str);
			double hue, sat, val;
			vd->GetHSV(hue, sat, val);
			str = wxString::Format("%f %f %f", hue, sat, val);
			fconfig.Write("hsv", str);
			color = vd->GetMaskColor();
			str = wxString::Format("%f %f %f", color.r(), color.g(), color.b());
			fconfig.Write("mask_color", str);
			fconfig.Write("mask_color_set", vd->GetMaskColorSet());
			fconfig.Write("enable_alpha", vd->GetAlphaEnable());
			fconfig.Write("alpha", vd->GetAlpha());
			double amb, diff, spec, shine;
			vd->GetMaterial(amb, diff, spec, shine);
			fconfig.Write("ambient", amb);
			fconfig.Write("diffuse", diff);
			fconfig.Write("specular", spec);
			fconfig.Write("shininess", shine);
			fconfig.Write("shading", vd->GetShadingEnable());
			fconfig.Write("samplerate", vd->GetSampleRate());

			//resolution scale
			double resx, resy, resz;
			double b_resx, b_resy, b_resz;
			double s_resx, s_resy, s_resz;
			double sclx, scly, sclz;
			vd->GetSpacings(resx, resy, resz);
			vd->GetBaseSpacings(b_resx, b_resy, b_resz);
			vd->GetSpacingScales(s_resx, s_resy, s_resz);
			vd->GetScalings(sclx, scly, sclz);
			str = wxString::Format("%lf %lf %lf", resx, resy, resz);
			fconfig.Write("res", str);
			str = wxString::Format("%lf %lf %lf", b_resx, b_resy, b_resz);
			fconfig.Write("b_res", str);
			str = wxString::Format("%lf %lf %lf", s_resx, s_resy, s_resz);
			fconfig.Write("s_res", str);
			str = wxString::Format("%lf %lf %lf", sclx, scly, sclz);
			fconfig.Write("scl", str);

			//planes
			vector<fluo::Plane*> *planes = 0;
			if (vd->GetVR())
				planes = vd->GetVR()->get_planes();
			if (planes && planes->size() == 6)
			{
				fluo::Plane* plane = 0;
				double abcd[4];

				//x1
				plane = (*planes)[0];
				plane->get_copy(abcd);
				fconfig.Write("x1_val", abcd[3]);
				//x2
				plane = (*planes)[1];
				plane->get_copy(abcd);
				fconfig.Write("x2_val", abcd[3]);
				//y1
				plane = (*planes)[2];
				plane->get_copy(abcd);
				fconfig.Write("y1_val", abcd[3]);
				//y2
				plane = (*planes)[3];
				plane->get_copy(abcd);
				fconfig.Write("y2_val", abcd[3]);
				//z1
				plane = (*planes)[4];
				plane->get_copy(abcd);
				fconfig.Write("z1_val", abcd[3]);
				//z2
				plane = (*planes)[5];
				plane->get_copy(abcd);
				fconfig.Write("z2_val", abcd[3]);
			}

			//2d adjustment settings
			str = wxString::Format("%f %f %f", vd->GetGammaColor().r(), vd->GetGammaColor().g(), vd->GetGammaColor().b());
			fconfig.Write("gamma", str);
			str = wxString::Format("%f %f %f", vd->GetBrightness().r(), vd->GetBrightness().g(), vd->GetBrightness().b());
			fconfig.Write("brightness", str);
			str = wxString::Format("%f %f %f", vd->GetHdr().r(), vd->GetHdr().g(), vd->GetHdr().b());
			fconfig.Write("hdr", str);
			fconfig.Write("sync_r", vd->GetSync(0));
			fconfig.Write("sync_g", vd->GetSync(1));
			fconfig.Write("sync_b", vd->GetSync(2));

			//colormap settings
			fconfig.Write("colormap_mode", vd->GetColormapMode());
			fconfig.Write("colormap_inv", vd->GetColormapInv());
			fconfig.Write("colormap", vd->GetColormap());
			fconfig.Write("colormap_proj", vd->GetColormapProj());
			double low, high;
			vd->GetColormapValues(low, high);
			fconfig.Write("colormap_lo_value", low);
			fconfig.Write("colormap_hi_value", high);

			//high transp
			fconfig.Write("alpha_power", vd->GetAlphaPower());
			//inversion
			fconfig.Write("inv", vd->GetInvert());
			//mip enable
			fconfig.Write("mode", vd->GetMode());
			//noise reduction
			fconfig.Write("noise_red", vd->GetNR());
			//depth override
			fconfig.Write("depth_ovrd", vd->GetBlendMode());

			//shadow
			fconfig.Write("shadow", vd->GetShadowEnable());
			//shadow intensity
			fconfig.Write("shadow_darkness", vd->GetShadowIntensity());

			//legend
			fconfig.Write("legend", vd->GetLegend());

			//mask
			vd->SaveMask(true, vd->GetCurTime(), vd->GetCurChannel());
			vd->SaveLabel(true, vd->GetCurTime(), vd->GetCurChannel());
		}
	}
	//mesh
	fconfig.SetPath("/data/mesh");
	fconfig.Write("num", glbin_data_manager.GetMeshNum());
	for (i=0; i<glbin_data_manager.GetMeshNum(); i++)
	{
		if (ticks && prg_diag)
			prg_diag->Update(90*tick_cnt/ticks,
			"Saving mesh data. Please wait.");
		tick_cnt++;

		MeshData* md = glbin_data_manager.GetMeshData(i);
		if (md)
		{
			if (md->GetPath() == "" || glbin_settings.m_vrp_embed)
			{
				wxString new_folder;
				new_folder = filename2 + "_files";
				MkDirW(new_folder.ToStdWstring());
				str = new_folder + GETSLASH() + md->GetName() + ".obj";
				md->Save(str);
			}
			str = wxString::Format("/data/mesh/%d", i);
			fconfig.SetPath(str);
			str = md->GetName();
			fconfig.Write("name", str);
			str = md->GetPath();
			fconfig.Write("path", str);
			//mesh prperties
			fconfig.SetPath("properties");
			fconfig.Write("display", md->GetDisp());
			//lighting
			fconfig.Write("lighting", md->GetLighting());
			//material
			fluo::Color amb, diff, spec;
			double shine, alpha;
			md->GetMaterial(amb, diff, spec, shine, alpha);
			str = wxString::Format("%f %f %f", amb.r(), amb.g(), amb.b());
			fconfig.Write("ambient", str);
			str = wxString::Format("%f %f %f", diff.r(), diff.g(), diff.b());
			fconfig.Write("diffuse", str);
			str = wxString::Format("%f %f %f", spec.r(), spec.g(), spec.b());
			fconfig.Write("specular", str);
			fconfig.Write("shininess", shine);
			fconfig.Write("alpha", alpha);
			//2d adjustment settings
			str = wxString::Format("%f %f %f", md->GetGammaColor().r(), md->GetGammaColor().g(), md->GetGammaColor().b());
			fconfig.Write("gamma", str);
			str = wxString::Format("%f %f %f", md->GetBrightness().r(), md->GetBrightness().g(), md->GetBrightness().b());
			fconfig.Write("brightness", str);
			str = wxString::Format("%f %f %f", md->GetHdr().r(), md->GetHdr().g(), md->GetHdr().b());
			fconfig.Write("hdr", str);
			fconfig.Write("sync_r", md->GetSync(0));
			fconfig.Write("sync_g", md->GetSync(1));
			fconfig.Write("sync_b", md->GetSync(2));
			//shadow
			fconfig.Write("shadow", md->GetShadowEnable());
			fconfig.Write("shadow_darkness", md->GetShadowIntensity());

			//mesh transform
			fconfig.SetPath("../transform");
			double x, y, z;
			md->GetTranslation(x, y, z);
			str = wxString::Format("%f %f %f", x, y, z);
			fconfig.Write("translation", str);
			md->GetRotation(x, y, z);
			str = wxString::Format("%f %f %f", x, y, z);
			fconfig.Write("rotation", str);
			md->GetScaling(x, y, z);
			str = wxString::Format("%f %f %f", x, y, z);
			fconfig.Write("scaling", str);
		}
	}
	//annotations
	fconfig.SetPath("/data/annotations");
	fconfig.Write("num", glbin_data_manager.GetAnnotationNum());
	for (i=0; i<glbin_data_manager.GetAnnotationNum(); i++)
	{
		Annotations* ann = glbin_data_manager.GetAnnotations(i);
		if (ann)
		{
			if (ann->GetPath() == "")
			{
				wxString new_folder;
				new_folder = filename2 + "_files";
				MkDirW(new_folder.ToStdWstring());
				str = new_folder + GETSLASH() + ann->GetName() + ".txt";
				ann->Save(str);
			}
			str = wxString::Format("/data/annotations/%d", i);
			fconfig.SetPath(str);
			str = ann->GetName();
			fconfig.Write("name", str);
			str = ann->GetPath();
			fconfig.Write("path", str);
		}
	}
	//views
	fconfig.SetPath("/views");
	fconfig.Write("num", GetViewNum());
	for (i=0; i<GetViewNum(); i++)
	{
		RenderCanvas* canvas = GetView(i);
		if (canvas)
		{
			str = wxString::Format("/views/%d", i);
			fconfig.SetPath(str);
			//view layers
			str = wxString::Format("/views/%d/layers", i);
			fconfig.SetPath(str);
			fconfig.Write("num", canvas->GetLayerNum());
			for (j=0; j< canvas->GetLayerNum(); j++)
			{
				TreeLayer* layer = canvas->GetLayer(j);
				if (!layer)
					continue;
				str = wxString::Format("/views/%d/layers/%d", i, j);
				fconfig.SetPath(str);
				switch (layer->IsA())
				{
				case 2://volume data
					fconfig.Write("type", 2);
					fconfig.Write("name", layer->GetName());
					break;
				case 3://mesh data
					fconfig.Write("type", 3);
					fconfig.Write("name", layer->GetName());
					break;
				case 4://annotations
					fconfig.Write("type", 4);
					fconfig.Write("name", layer->GetName());
					break;
				case 5://group
					{
						DataGroup* group = (DataGroup*)layer;

						fconfig.Write("type", 5);
						fconfig.Write("name", layer->GetName());
						fconfig.Write("id", DataGroup::GetID());
						//dispaly
						fconfig.Write("display", group->GetDisp());
						//2d adjustment
						str = wxString::Format("%f %f %f", group->GetGammaColor().r(),
							group->GetGammaColor().g(), group->GetGammaColor().b());
						fconfig.Write("gamma", str);
						str = wxString::Format("%f %f %f", group->GetBrightness().r(),
							group->GetBrightness().g(), group->GetBrightness().b());
						fconfig.Write("brightness", str);
						str = wxString::Format("%f %f %f", group->GetHdr().r(),
							group->GetHdr().g(), group->GetHdr().b());
						fconfig.Write("hdr", str);
						fconfig.Write("sync_r", group->GetSync(0));
						fconfig.Write("sync_g", group->GetSync(1));
						fconfig.Write("sync_b", group->GetSync(2));
						//sync volume properties
						fconfig.Write("sync_vp", group->GetVolumeSyncProp());
						//volumes
						str = wxString::Format("/views/%d/layers/%d/volumes", i, j);
						fconfig.SetPath(str);
						fconfig.Write("num", group->GetVolumeNum());
						for (k=0; k<group->GetVolumeNum(); k++)
							fconfig.Write(wxString::Format("vol_%d", k), group->GetVolumeData(k)->GetName());

					}
					break;
				case 6://mesh group
					{
						MeshGroup* group = (MeshGroup*)layer;

						fconfig.Write("type", 6);
						fconfig.Write("name", layer->GetName());
						fconfig.Write("id", MeshGroup::GetID());
						//display
						fconfig.Write("display", group->GetDisp());
						//sync mesh properties
						fconfig.Write("sync_mp", group->GetMeshSyncProp());
						//meshes
						str = wxString::Format("/views/%d/layers/%d/meshes", i, j);
						fconfig.SetPath(str);
						fconfig.Write("num", group->GetMeshNum());
						for (k=0; k<group->GetMeshNum(); k++)
							fconfig.Write(wxString::Format("mesh_%d", k), group->GetMeshData(k)->GetName());
					}
					break;
				}
			}

			//properties
			fconfig.SetPath(wxString::Format("/views/%d/properties", i));
			fconfig.Write("drawall", canvas->GetDraw());
			fconfig.Write("persp", canvas->GetPersp());
			fconfig.Write("free", canvas->GetFree());
			fconfig.Write("aov", canvas->GetAov());
			fconfig.Write("nearclip", canvas->GetNearClip());
			fconfig.Write("farclip", canvas->GetFarClip());
			fluo::Color bkcolor;
			bkcolor = canvas->GetBackgroundColor();
			str = wxString::Format("%f %f %f", bkcolor.r(), bkcolor.g(), bkcolor.b());
			fconfig.Write("backgroundcolor", str);
			fconfig.Write("drawtype", canvas->GetDrawType());
			fconfig.Write("volmethod", canvas->GetVolMethod());
			fconfig.Write("fog", canvas->GetFog());
			fconfig.Write("fogintensity", (double)canvas->GetFogIntensity());
			fconfig.Write("draw_camctr", canvas->m_draw_camctr);
			fconfig.Write("draw_info", canvas->m_draw_info);
			fconfig.Write("draw_legend", canvas->m_draw_legend);

			double x, y, z;
			//camera
			canvas->GetTranslations(x, y, z);
			str = wxString::Format("%f %f %f", x, y, z);
			fconfig.Write("translation", str);
			canvas->GetRotations(x, y, z);
			str = wxString::Format("%f %f %f", x, y, z);
			fconfig.Write("rotation", str);
			fluo::Quaternion q = canvas->GetZeroQuat();
			str = wxString::Format("%f %f %f %f", q.x, q.y, q.z, q.w);
			fconfig.Write("zero_quat", str);
			canvas->GetCenters(x, y, z);
			str = wxString::Format("%f %f %f", x, y, z);
			fconfig.Write("center", str);
			fconfig.Write("centereyedist", canvas->GetCenterEyeDist());
			fconfig.Write("radius", canvas->GetRadius());
			fconfig.Write("initdist", canvas->GetInitDist());
			fconfig.Write("scale_mode", canvas->m_scale_mode);
			fconfig.Write("scale", canvas->m_scale_factor);
			fconfig.Write("pin_rot_center", canvas->m_pin_rot_ctr);
			//object
			canvas->GetObjCenters(x, y, z);
			str = wxString::Format("%f %f %f", x, y, z);
			fconfig.Write("obj_center", str);
			canvas->GetObjTrans(x, y, z);
			str = wxString::Format("%f %f %f", x, y, z);
			fconfig.Write("obj_trans", str);
			canvas->GetObjRot(x, y, z);
			str = wxString::Format("%f %f %f", x, y, z);
			fconfig.Write("obj_rot", str);
			//scale bar
			fconfig.Write("disp_scale_bar", canvas->m_disp_scale_bar);
			fconfig.Write("disp_scale_bar_text", canvas->m_disp_scale_bar_text);
			fconfig.Write("sb_length", canvas->m_sb_length);
			str = canvas->m_sb_text;
			fconfig.Write("sb_text", str);
			str = canvas->m_sb_num;
			fconfig.Write("sb_num", str);
			fconfig.Write("sb_unit", canvas->m_sb_unit);

			//2d adjustment
			str = wxString::Format("%f %f %f", canvas->GetGammaColor().r(),
				canvas->GetGammaColor().g(), canvas->GetGammaColor().b());
			fconfig.Write("gamma", str);
			str = wxString::Format("%f %f %f", canvas->GetBrightness().r(),
				canvas->GetBrightness().g(), canvas->GetBrightness().b());
			fconfig.Write("brightness", str);
			str = wxString::Format("%f %f %f", canvas->GetHdr().r(),
				canvas->GetHdr().g(), canvas->GetHdr().b());
			fconfig.Write("hdr", str);
			fconfig.Write("sync_r", canvas->GetSync(0));
			fconfig.Write("sync_g", canvas->GetSync(1));
			fconfig.Write("sync_b", canvas->GetSync(2));

			//clipping plane rotations
			fconfig.Write("clip_mode", canvas->GetClipMode());
			double rotx_cl, roty_cl, rotz_cl;
			canvas->GetClippingPlaneRotations(rotx_cl, roty_cl, rotz_cl);
			fconfig.Write("rotx_cl", rotx_cl);
			fconfig.Write("roty_cl", roty_cl);
			fconfig.Write("rotz_cl", rotz_cl);

			//painting parameters
			fconfig.Write("brush_use_pres", glbin_vol_selector.GetBrushUsePres());
			fconfig.Write("brush_size_1", glbin_vol_selector.GetBrushSize1());
			fconfig.Write("brush_size_2", glbin_vol_selector.GetBrushSize2());
			fconfig.Write("brush_spacing", glbin_vol_selector.GetBrushSpacing());
			fconfig.Write("brush_iteration", glbin_vol_selector.GetBrushIteration());
			fconfig.Write("brush_translate", glbin_vol_selector.GetBrushSclTranslate());
			fconfig.Write("w2d", glbin_vol_selector.GetW2d());

			//rulers
			fconfig.SetPath(wxString::Format("/views/%d/rulers", i));
			glbin_ruler_handler.Save(fconfig, i);
		}
	}
	//clipping planes
	fconfig.SetPath("/prop_panel");
	fconfig.Write("cur_sel_type", m_cur_sel_type);
	fconfig.Write("cur_sel_vol", m_cur_sel_vol);
	fconfig.Write("cur_sel_mesh", m_cur_sel_mesh);
	fconfig.Write("chann_link", m_clip_view->GetChannLink());
	fconfig.Write("hold planes", m_clip_view->GetHoldPlanes());
	fconfig.Write("plane mode", int(m_clip_view->GetPlaneMode()));
	fconfig.Write("x_link", m_clip_view->GetXLink());
	fconfig.Write("y_link", m_clip_view->GetYLink());
	fconfig.Write("z_link", m_clip_view->GetZLink());
	//movie view
	fconfig.SetPath("/movie_panel");
	fconfig.Write("key frame enable", glbin_moviemaker.GetKeyframeEnable());
	fconfig.Write("views_cmb", glbin_moviemaker.GetViewIndex());
	fconfig.Write("rot_check", glbin_moviemaker.GetRotateEnable());
	fconfig.Write("seq_mode", glbin_moviemaker.GetSeqMode());
	fconfig.Write("rot_axis", glbin_moviemaker.GetRotateAxis());
	fconfig.Write("rot_deg", glbin_moviemaker.GetRotateDeg());
	fconfig.Write("movie_len", glbin_moviemaker.GetMovieLength());
	fconfig.Write("fps", glbin_moviemaker.GetFps());
	fconfig.Write("crop", glbin_moviemaker.GetCropEnable());
	fconfig.Write("crop_x", glbin_moviemaker.GetCropX());
	fconfig.Write("crop_y", glbin_moviemaker.GetCropY());
	fconfig.Write("crop_w", glbin_moviemaker.GetCropW());
	fconfig.Write("crop_h", glbin_moviemaker.GetCropH());
	fconfig.Write("cur_frame", glbin_moviemaker.GetCurrentFrame());
	fconfig.Write("full frame num", glbin_moviemaker.GetFullFrameNum());
	fconfig.Write("start_frame", glbin_moviemaker.GetClipStartFrame());
	fconfig.Write("end_frame", glbin_moviemaker.GetClipEndFrame());
	fconfig.Write("run_script", glbin_settings.m_run_script);
	fconfig.Write("script_file", glbin_settings.m_script_file);
	//tracking diag
	fconfig.SetPath("/track_diag");
	int ival = m_trace_dlg->GetTrackFileExist(true);
	if (ival == 1)
	{
		wxString new_folder;
		new_folder = filename2 + "_files";
		MkDirW(new_folder.ToStdWstring());
		std::wstring wstr = filename2.ToStdWstring();
		str = new_folder + GETSLASH() + GET_NAME(wstr) + ".track";
		m_trace_dlg->SaveTrackFile(str);
	}
	fconfig.Write("track_file", m_trace_dlg->GetTrackFile());
/*	//brushtool diag
	fconfig.SetPath("/brush_diag");
	fconfig.Write("ca_min", m_brush_tool_dlg->GetDftCAMin());
	fconfig.Write("ca_max", m_brush_tool_dlg->GetDftCAMax());
	fconfig.Write("ca_thresh", m_brush_tool_dlg->GetDftCAThresh());
	fconfig.Write("nr_thresh", m_brush_tool_dlg->GetDftNRThresh());
	fconfig.Write("nr_size", m_brush_tool_dlg->GetDftNRSize());*/
	//ui layout
	fconfig.SetPath("/ui_layout");
	fconfig.Write("dpi scale factor", GetDPIScaleFactor());
	fconfig.Write("layout", m_aui_mgr.SavePerspective());
	//interpolator
	fconfig.SetPath("/interpolator");
	fconfig.Write("max_id", Interpolator::m_id);
	int group_num = glbin_interpolator.GetKeyNum();
	fconfig.Write("num", group_num);
	for (i=0; i<group_num; i++)
	{
		FlKeyGroup* key_group = glbin_interpolator.GetKeyGroup(i);
		if (key_group)
		{
			str = wxString::Format("/interpolator/%d", i);
			fconfig.SetPath(str);
			fconfig.Write("id", key_group->id);
			fconfig.Write("t", key_group->t);
			fconfig.Write("dt", key_group->dt);
			fconfig.Write("type", key_group->type);
			str = key_group->desc;
			fconfig.Write("desc", str);
			int key_num = (int)key_group->keys.size();
			str = wxString::Format("/interpolator/%d/keys", i);
			fconfig.SetPath(str);
			fconfig.Write("num", key_num);
			for (j=0; j<key_num; j++)
			{
				FlKey* key = key_group->keys[j];
				if (key)
				{
					str = wxString::Format("/interpolator/%d/keys/%d", i, j);
					fconfig.SetPath(str);
					int key_type = key->GetType();
					fconfig.Write("type", key_type);
					FlKeyCode code = key->GetKeyCode();
					fconfig.Write("l0", code.l0);
					str = code.l0_name;
					fconfig.Write("l0_name", str);
					fconfig.Write("l1", code.l1);
					str = code.l1_name;
					fconfig.Write("l1_name", str);
					fconfig.Write("l2", code.l2);
					str = code.l2_name;
					fconfig.Write("l2_name", str);
					switch (key_type)
					{
					case FLKEY_TYPE_DOUBLE:
						{
							double dval = ((FlKeyDouble*)key)->GetValue();
							fconfig.Write("val", dval);
						}
						break;
					case FLKEY_TYPE_QUATER:
						{
							fluo::Quaternion qval = ((FlKeyQuaternion*)key)->GetValue();
							str = wxString::Format("%lf %lf %lf %lf",
								qval.x, qval.y, qval.z, qval.w);
							fconfig.Write("val", str);
						}
						break;
					case FLKEY_TYPE_BOOLEAN:
						{
							bool bval = ((FlKeyBoolean*)key)->GetValue();
							fconfig.Write("val", bval);
						}
						break;
					case FLKEY_TYPE_INT:
						{
							int ival = ((FlKeyInt*)key)->GetValue();
							fconfig.Write("val", ival);
						}
						break;
					case FLKEY_TYPE_COLOR:
						{
							fluo::Color cval = ((FlKeyColor*)key)->GetValue();
							str = wxString::Format("%lf %lf %lf",
								cval.r(), cval.g(), cval.b());
							fconfig.Write("val", str);
						}
						break;
					}
				}
			}
		}
	}

	SaveConfig(fconfig, filename2);
	//UpdateList();

	delete prg_diag;
	glbin_data_manager.SetProjectPath(filename2);
	SetTitle(m_title + " - " + filename2);

	UpdateProps({ gstListCtrl });
}

void MainFrame::OpenProject(wxString& filename)
{
	glbin_data_manager.SetProjectPath(filename);
	SetTitle(m_title + " - " + filename);

	int iVal;
	int i, j, k;
	//clear
	glbin_data_manager.ClearAll();
	DataGroup::ResetID();
	MeshGroup::ResetID();
	m_adjust_view->SetVolumeData(0);
	m_adjust_view->SetGroup(0);
	m_adjust_view->SetGroupLink(0);
	GetView(0)->ClearAll();
	for (i = m_vrv_list.size() - 1; i > 0; i--)
		DeleteVRenderView(i);
	//RenderViewPanel::ResetID();
	DataGroup::ResetID();
	MeshGroup::ResetID();


	wxFileInputStream is(filename);
	if (!is.IsOk())
		return;
	wxFileConfig fconfig(is);
	wxString ver_major, ver_minor;
	long l_major;
	double d_minor;
	l_major = 1;
	if (fconfig.Read("ver_major", &ver_major) &&
		fconfig.Read("ver_minor", &ver_minor))
	{
		ver_major.ToLong(&l_major);
		ver_minor.ToDouble(&d_minor);

		if (l_major>VERSION_MAJOR)
			::wxMessageBox("The project file is saved by a newer version of FluoRender.\n" \
			"Please check update and download the new version.");
		else if (d_minor>VERSION_MINOR)
			::wxMessageBox("The project file is saved by a newer version of FluoRender.\n" \
			"Please check update and download the new version.");
	}

	int ticks = 0;
	int tick_cnt = 1;
	fconfig.Read("ticks", &ticks);
	wxProgressDialog *prg_diag = 0;
	prg_diag = new wxProgressDialog(
		"FluoRender: Loading project...",
		"Reading project file. Please wait.",
		100, this, wxPD_SMOOTH|wxPD_ELAPSED_TIME|wxPD_AUTO_HIDE);

	bool bval;
	double dval;
	int ival;
	//read streaming mode
	if (fconfig.Exists("/settings"))
	{
		fconfig.SetPath("/settings");
		fconfig.Read("mouse int", &bval, true);
		glbin_settings.m_mouse_int = bval;
		fconfig.Read("mem swap", &bval, false);
		glbin_settings.m_mem_swap = bval;
		fconfig.Read("graphics mem", &dval, 1000.0);
		glbin_settings.m_graphics_mem = dval;
		fconfig.Read("large data size", &dval, 1000.0);
		glbin_settings.m_large_data_size = dval;
		fconfig.Read("force brick size", &ival, 128);
		glbin_settings.m_force_brick_size = ival;
		fconfig.Read("up time", &ival, 100);
		glbin_settings.m_up_time = ival;
		fconfig.Read("update order", &ival, 0);
		glbin_settings.m_update_order = ival;
		fconfig.Read("inf loop", &bval, false);
		glbin_settings.m_inf_loop = bval;
		//graphics memory setting may have changed
		glbin_settings.GetMemorySettings();
		//peeling layers
		fconfig.Read("peeling layers", &ival, 1);
		glbin_settings.m_peeling_layers = ival;
		UpdateProps({ gstMouseInt, gstStreamEnable, gstPeelNum });
	}

	//read data list
	//volume
	if (fconfig.Exists("/data/volume"))
	{
		fconfig.SetPath("/data/volume");
		int num = fconfig.Read("num", 0l);
		for (i=0; i<num; i++)
		{
			if (ticks && prg_diag)
				prg_diag->Update(90*tick_cnt/ticks,
				"Reading and processing volume data. Please wait.");

			wxString str;
			str = wxString::Format("/data/volume/%d", i);
			if (fconfig.Exists(str))
			{
				int loaded_num = 0;
				fconfig.SetPath(str);
				bool compression = false;
				fconfig.Read("compression", &compression);
				glbin_settings.m_realtime_compress = compression;
				bool skip_brick = false;
				fconfig.Read("skip_brick", &skip_brick);
				glbin_settings.m_skip_brick = skip_brick;
				//path
				if (fconfig.Read("path", &str))
				{
					int cur_chan = 0;
					if (!fconfig.Read("cur_chan", &cur_chan))
						if (fconfig.Read("tiff_chan", &cur_chan))
							cur_chan--;
					int cur_time = 0;
					fconfig.Read("cur_time", &cur_time);
					//reader type
					int reader_type = 0;
					fconfig.Read("reader_type", &reader_type);
					bool slice_seq = 0;
					fconfig.Read("slice_seq", &slice_seq);
					glbin_settings.m_slice_sequence = slice_seq;
					wxString time_id;
					fconfig.Read("time_id", &time_id);
					glbin_settings.m_time_id = time_id;
					bool fp_convert = false;
					double minv, maxv;
					fconfig.Read("fp_convert", &fp_convert, false);
					fconfig.Read("fp_min", &minv, 0);
					fconfig.Read("fp_max", &maxv, 1);
					glbin_settings.m_fp_convert = fp_convert;
					glbin_settings.m_fp_min = minv;
					glbin_settings.m_fp_max = maxv;
					wxString suffix = str.Mid(str.Find('.', true)).MakeLower();
					if (reader_type == READER_IMAGEJ_TYPE)
						loaded_num = glbin_data_manager.LoadVolumeData(str, LOAD_TYPE_IMAGEJ, true, cur_chan, cur_time);
					else if (suffix == ".nrrd" || suffix == ".msk" || suffix == ".lbl")
						loaded_num = glbin_data_manager.LoadVolumeData(str, LOAD_TYPE_NRRD, false, cur_chan, cur_time);
					else if (suffix == ".tif" || suffix == ".tiff")
						loaded_num = glbin_data_manager.LoadVolumeData(str, LOAD_TYPE_TIFF, false, cur_chan, cur_time);
					else if (suffix == ".oib")
						loaded_num = glbin_data_manager.LoadVolumeData(str, LOAD_TYPE_OIB, false, cur_chan, cur_time);
					else if (suffix == ".oif")
						loaded_num = glbin_data_manager.LoadVolumeData(str, LOAD_TYPE_OIF, false, cur_chan, cur_time);
					else if (suffix == ".lsm")
						loaded_num = glbin_data_manager.LoadVolumeData(str, LOAD_TYPE_LSM, false, cur_chan, cur_time);
					else if (suffix == ".xml")
						loaded_num = glbin_data_manager.LoadVolumeData(str, LOAD_TYPE_PVXML, false, cur_chan, cur_time);
					else if (suffix == ".vvd")
						loaded_num = glbin_data_manager.LoadVolumeData(str, LOAD_TYPE_BRKXML, false, cur_chan, cur_time);
					else if (suffix == ".czi")
						loaded_num = glbin_data_manager.LoadVolumeData(str, LOAD_TYPE_CZI, false, cur_chan, cur_time);
					else if (suffix == ".nd2")
						loaded_num = glbin_data_manager.LoadVolumeData(str, LOAD_TYPE_ND2, false, cur_chan, cur_time);
					else if (suffix == ".lif")
						loaded_num = glbin_data_manager.LoadVolumeData(str, LOAD_TYPE_LIF, false, cur_chan, cur_time);
					else if (suffix == ".lof")
						loaded_num = glbin_data_manager.LoadVolumeData(str, LOAD_TYPE_LOF, false, cur_chan, cur_time);
					else if (suffix == ".mp4" || suffix == ".m4v" || suffix == ".mov" || suffix == ".avi" || suffix == ".wmv")
						loaded_num = glbin_data_manager.LoadVolumeData(str, LOAD_TYPE_MPG, false, cur_chan, cur_time);
				}
				VolumeData* vd = 0;
				if (loaded_num)
					vd = glbin_data_manager.GetLastVolumeData();
				if (vd)
				{
					if (fconfig.Read("name", &str))
						vd->SetName(str);//setname
					//volume properties
					if (fconfig.Exists("properties"))
					{
						fconfig.SetPath("properties");
						bool disp;
						if (fconfig.Read("display", &disp))
							vd->SetDisp(disp);

						//old colormap
						if (fconfig.Read("widget", &str))
						{
							int type;
							float left_x, left_y, width, height, offset1, offset2, gamma;
							wchar_t token[256] = {};
							token[255] = '\0';
							const wchar_t* sstr = str.wc_str();
							std::wstringstream ss(sstr);
							ss.read(token,255);
							wchar_t c = 'x';
							while(!isspace(c)) ss.read(&c,1);
							ss >> type >> left_x >> left_y >> width >>
								height >> offset1 >> offset2 >> gamma;
							vd->SetGamma(gamma);
							vd->SetBoundary(left_y);
							vd->SetSaturation(offset1);
							vd->SetLeftThresh(left_x);
							vd->SetRightThresh(left_x+width);
							if (fconfig.Read("widgetcolor", &str))
							{
								float red, green, blue;
								if (SSCANF(str.c_str(), "%f%f%f", &red, &green, &blue)){
									fluo::Color col(red,green,blue);
									vd->SetColor(col);
								}
							}
							double alpha;
							if (fconfig.Read("widgetalpha", &alpha))
								vd->SetAlpha(alpha);
						}

						//transfer function
						double dval;
						bool bval;
						if (fconfig.Read("3dgamma", &dval))
							vd->SetGamma(dval);
						if (fconfig.Read("boundary", &dval))
							vd->SetBoundary(dval);
						if (fconfig.Read("contrast", &dval))
							vd->SetSaturation(dval);
						if (fconfig.Read("left_thresh", &dval))
							vd->SetLeftThresh(dval);
						if (fconfig.Read("right_thresh", &dval))
							vd->SetRightThresh(dval);
						if (fconfig.Read("color", &str))
						{
							float red, green, blue;
							if (SSCANF(str.c_str(), "%f%f%f", &red, &green, &blue)){
								fluo::Color col(red,green,blue);
								vd->SetColor(col);
							}
						}
						if (fconfig.Read("hsv", &str))
						{
							float hue, sat, val;
							if (SSCANF(str.c_str(), "%f%f%f", &hue, &sat, &val))
								vd->SetHSV(hue, sat, val);
						}
						if (fconfig.Read("mask_color", &str))
						{
							float red, green, blue;
							if (SSCANF(str.c_str(), "%f%f%f", &red, &green, &blue)){
								fluo::Color col(red,green,blue);
								if (fconfig.Read("mask_color_set", &bval))
									vd->SetMaskColor(col, bval);
								else
									vd->SetMaskColor(col);
							}
						}
						if (fconfig.Read("enable_alpha", &bval))
							vd->SetAlphaEnable(bval);
						if (fconfig.Read("alpha", &dval))
							vd->SetAlpha(dval);

						//shading
						double amb, diff, spec, shine;
						if (fconfig.Read("ambient", &amb)&&
							fconfig.Read("diffuse", &diff)&&
							fconfig.Read("specular", &spec)&&
							fconfig.Read("shininess", &shine))
							vd->SetMaterial(amb, diff, spec, shine);
						bool shading;
						if (fconfig.Read("shading", &shading))
							vd->SetShadingEnable(shading);
						double srate;
						if (fconfig.Read("samplerate", &srate))
						{
							if (l_major<2)
								vd->SetSampleRate(srate/5.0);
							else
								vd->SetSampleRate(srate);
						}

						//spacings and scales
						if (!vd->isBrxml())
						{
							if (fconfig.Read("res", &str))
							{
								double resx, resy, resz;
								if (SSCANF(str.c_str(), "%lf%lf%lf", &resx, &resy, &resz))
								{
									vd->SetBaseSpacings(resx, resy, resz);
								}
							}
						}
						else
						{
							if (fconfig.Read("b_res", &str))
							{
								double b_resx, b_resy, b_resz;
								if (SSCANF(str.c_str(), "%lf%lf%lf", &b_resx, &b_resy, &b_resz))
									vd->SetBaseSpacings(b_resx, b_resy, b_resz);
							}
							if (fconfig.Read("s_res", &str))
							{
								double s_resx, s_resy, s_resz;
								if (SSCANF(str.c_str(), "%lf%lf%lf", &s_resx, &s_resy, &s_resz))
									vd->SetSpacingScales(s_resx, s_resy, s_resz);
							}
						}
						if (fconfig.Read("scl", &str))
						{
							double sclx, scly, sclz;
							if (SSCANF(str.c_str(), "%lf%lf%lf", &sclx, &scly, &sclz))
								vd->SetScalings(sclx, scly, sclz);
						}

						vector<fluo::Plane*> *planes = 0;
						if (vd->GetVR())
							planes = vd->GetVR()->get_planes();
						int iresx, iresy, iresz;
						vd->GetResolution(iresx, iresy, iresz);
						if (planes && planes->size()==6)
						{
							double val;
							wxString splane;

							//x1
							if (fconfig.Read("x1_vali", &val))
								(*planes)[0]->ChangePlane(fluo::Point(abs(val/iresx), 0.0, 0.0),
									fluo::Vector(1.0, 0.0, 0.0));
							else if (fconfig.Read("x1_val", &val))
								(*planes)[0]->ChangePlane(fluo::Point(abs(val), 0.0, 0.0),
									fluo::Vector(1.0, 0.0, 0.0));

							//x2
							if (fconfig.Read("x2_vali", &val))
								(*planes)[1]->ChangePlane(fluo::Point(abs(val/iresx), 0.0, 0.0),
									fluo::Vector(-1.0, 0.0, 0.0));
							else if (fconfig.Read("x2_val", &val))
								(*planes)[1]->ChangePlane(fluo::Point(abs(val), 0.0, 0.0),
									fluo::Vector(-1.0, 0.0, 0.0));

							//y1
							if (fconfig.Read("y1_vali", &val))
								(*planes)[2]->ChangePlane(fluo::Point(0.0, abs(val/iresy), 0.0),
									fluo::Vector(0.0, 1.0, 0.0));
							else if (fconfig.Read("y1_val", &val))
								(*planes)[2]->ChangePlane(fluo::Point(0.0, abs(val), 0.0),
									fluo::Vector(0.0, 1.0, 0.0));

							//y2
							if (fconfig.Read("y2_vali", &val))
								(*planes)[3]->ChangePlane(fluo::Point(0.0, abs(val/iresy), 0.0),
									fluo::Vector(0.0, -1.0, 0.0));
							else if (fconfig.Read("y2_val", &val))
								(*planes)[3]->ChangePlane(fluo::Point(0.0, abs(val), 0.0),
									fluo::Vector(0.0, -1.0, 0.0));

							//z1
							if (fconfig.Read("z1_vali", &val))
								(*planes)[4]->ChangePlane(fluo::Point(0.0, 0.0, abs(val/iresz)),
									fluo::Vector(0.0, 0.0, 1.0));
							else if (fconfig.Read("z1_val", &val))
								(*planes)[4]->ChangePlane(fluo::Point(0.0, 0.0, abs(val)),
									fluo::Vector(0.0, 0.0, 1.0));

							//z2
							if (fconfig.Read("z2_vali", &val))
								(*planes)[5]->ChangePlane(fluo::Point(0.0, 0.0, abs(val/iresz)),
									fluo::Vector(0.0, 0.0, -1.0));
							else if (fconfig.Read("z2_val", &val))
								(*planes)[5]->ChangePlane(fluo::Point(0.0, 0.0, abs(val)),
									fluo::Vector(0.0, 0.0, -1.0));
						}

						//2d adjustment settings
						if (fconfig.Read("gamma", &str))
						{
							float r, g, b;
							if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)){
								fluo::Color col(r,g,b);
								vd->SetGammaColor(col);
							}
						}
						if (fconfig.Read("brightness", &str))
						{
							float r, g, b;
							if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)){
								fluo::Color col(r,g,b);
								vd->SetBrightness(col);
							}
						}
						if (fconfig.Read("hdr", &str))
						{
							float r, g, b;
							if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)){
								fluo::Color col(r,g,b);
								vd->SetHdr(col);
							}
						}
						bool bVal;
						if (fconfig.Read("sync_r", &bVal))
							vd->SetSync(0, bVal);
						if (fconfig.Read("sync_g", &bVal))
							vd->SetSync(1, bVal);
						if (fconfig.Read("sync_b", &bVal))
							vd->SetSync(2, bVal);

						//colormap settings
						if (fconfig.Read("colormap_mode", &iVal))
							vd->SetColormapMode(iVal);
						if (fconfig.Read("colormap_inv", &dval))
							vd->SetColormapInv(dval);
						if (fconfig.Read("colormap", &iVal))
							vd->SetColormap(iVal);
						if (fconfig.Read("colormap_proj", &iVal))
							vd->SetColormapProj(iVal);
						double low, high;
						if (fconfig.Read("colormap_lo_value", &low) &&
							fconfig.Read("colormap_hi_value", &high))
						{
							vd->SetColormapValues(low, high);
						}

						//high transp
						if (fconfig.Read("alpha_power", &dval))
							vd->SetAlphaPower(dval);
						//inversion
						if (fconfig.Read("inv", &bVal))
							vd->SetInvert(bVal);
						//mip enable
						if (fconfig.Read("mode", &iVal))
							vd->SetMode(iVal);
						//noise reduction
						if (fconfig.Read("noise_red", &bVal))
							vd->SetNR(bVal);
						//depth override
						if (fconfig.Read("depth_ovrd", &iVal))
							vd->SetBlendMode(iVal);

						//shadow
						if (fconfig.Read("shadow", &bVal))
							vd->SetShadowEnable(bVal);
						//shaodw intensity
						if (fconfig.Read("shadow_darkness", &dval))
							vd->SetShadowIntensity(dval);

						//legend
						if (fconfig.Read("legend", &bVal))
							vd->SetLegend(bVal);

						//mask
						if (fconfig.Read("mask", &str))
						{
							MSKReader msk_reader;
							wstring maskname = str.ToStdWstring();
							msk_reader.SetFile(maskname);
							BaseReader *br = &msk_reader;
							Nrrd* mask = br->Convert(true);
							if (mask)
								vd->LoadMask(mask);
						}
					}
				}
			}
			tick_cnt++;
		}
	}
	//mesh
	if (fconfig.Exists("/data/mesh"))
	{
		fconfig.SetPath("/data/mesh");
		int num = fconfig.Read("num", 0l);
		for (i=0; i<num; i++)
		{
			if (ticks && prg_diag)
				prg_diag->Update(90*tick_cnt/ticks,
				"Reading and processing mesh data. Please wait.");

			wxString str;
			str = wxString::Format("/data/mesh/%d", i);
			if (fconfig.Exists(str))
			{
				fconfig.SetPath(str);
				if (fconfig.Read("path", &str))
				{
					glbin_data_manager.LoadMeshData(str);
				}
				MeshData* md = glbin_data_manager.GetLastMeshData();
				if (md)
				{
					if (fconfig.Read("name", &str))
						md->SetName(str);//setname
					//mesh properties
					if (fconfig.Exists("properties"))
					{
						fconfig.SetPath("properties");
						bool disp;
						if (fconfig.Read("display", &disp))
							md->SetDisp(disp);
						//lighting
						bool lighting;
						if (fconfig.Read("lighting", &lighting))
							md->SetLighting(lighting);
						double shine, alpha;
						float r=0.0f, g=0.0f, b=0.0f;
						if (fconfig.Read("ambient", &str))
							SSCANF(str.c_str(), "%f%f%f", &r, &g, &b);
						fluo::Color amb(r, g, b);
						if (fconfig.Read("diffuse", &str))
							SSCANF(str.c_str(), "%f%f%f", &r, &g, &b);
						fluo::Color diff(r, g, b);
						if (fconfig.Read("specular", &str))
							SSCANF(str.c_str(), "%f%f%f", &r, &g, &b);
						fluo::Color spec(r, g, b);
						fconfig.Read("shininess", &shine, 30.0);
						fconfig.Read("alpha", &alpha, 0.5);
						md->SetMaterial(amb, diff, spec, shine, alpha);
						//2d adjusment settings
						if (fconfig.Read("gamma", &str))
						{
							float r, g, b;
							if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)){
								fluo::Color col(r,g,b);
								md->SetGammaColor(col);
							}
						}
						if (fconfig.Read("brightness", &str))
						{
							float r, g, b;
							if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)){
								fluo::Color col(r,g,b);
								md->SetBrightness(col);
							}
						}
						if (fconfig.Read("hdr", &str))
						{
							float r, g, b;
							if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)){
								fluo::Color col(r,g,b);
								md->SetHdr(col);
							}
						}
						bool bVal;
						if (fconfig.Read("sync_r", &bVal))
							md->SetSync(0, bVal);
						if (fconfig.Read("sync_g", &bVal))
							md->SetSync(1, bVal);
						if (fconfig.Read("sync_b", &bVal))
							md->SetSync(2, bVal);
						//shadow
						if (fconfig.Read("shadow", &bVal))
							md->SetShadowEnable(bVal);
						double darkness;
						if (fconfig.Read("shadow_darkness", &darkness))
							md->SetShadowIntensity(darkness);

						//mesh transform
						if (fconfig.Exists("../transform"))
						{
							fconfig.SetPath("../transform");
							float x, y, z;
							if (fconfig.Read("translation", &str))
							{
								if (SSCANF(str.c_str(), "%f%f%f", &x, &y, &z))
									md->SetTranslation(x, y, z);
							}
							if (fconfig.Read("rotation", &str))
							{
								if (SSCANF(str.c_str(), "%f%f%f", &x, &y, &z))
									md->SetRotation(x, y, z);
							}
							if (fconfig.Read("scaling", &str))
							{
								if (SSCANF(str.c_str(), "%f%f%f", &x, &y, &z))
									md->SetScaling(x, y, z);
							}
						}

					}
				}
			}
			tick_cnt++;
		}
	}
	//annotations
	if (fconfig.Exists("/data/annotations"))
	{
		fconfig.SetPath("/data/annotations");
		int num = fconfig.Read("num", 0l);
		for (i=0; i<num; i++)
		{
			wxString str;
			str = wxString::Format("/data/annotations/%d", i);
			if (fconfig.Exists(str))
			{
				fconfig.SetPath(str);
				if (fconfig.Read("path", &str))
				{
					glbin_data_manager.LoadAnnotations(str);
				}
			}
		}
	}

	bool bVal;
	//views
	if (fconfig.Exists("/views"))
	{
		fconfig.SetPath("/views");
		int num = fconfig.Read("num", 0l);

		for (i=0; i<num; i++)
		{
			if (i>0)
				CreateView();
			RenderCanvas* canvas = GetLastView();
			if (!canvas)
				continue;

			canvas->ClearAll();

			wxString str;
			//old
			//volumes
			str = wxString::Format("/views/%d/volumes", i);
			if (fconfig.Exists(str))
			{
				fconfig.SetPath(str);
				int num = fconfig.Read("num", 0l);
				for (j=0; j<num; j++)
				{
					if (fconfig.Read(wxString::Format("name%d", j), &str))
					{
						VolumeData* vd = glbin_data_manager.GetVolumeData(str);
						if (vd)
							canvas->AddVolumeData(vd);
					}
				}
				canvas->SetVolPopDirty();
			}
			//meshes
			str = wxString::Format("/views/%d/meshes", i);
			if (fconfig.Exists(str))
			{
				fconfig.SetPath(str);
				int num = fconfig.Read("num", 0l);
				for (j=0; j<num; j++)
				{
					if (fconfig.Read(wxString::Format("name%d", j), &str))
					{
						MeshData* md = glbin_data_manager.GetMeshData(str);
						if (md)
							canvas->AddMeshData(md);
					}
				}
			}

			//new
			str = wxString::Format("/views/%d/layers", i);
			if (fconfig.Exists(str))
			{
				fconfig.SetPath(str);

				//view layers
				int layer_num = fconfig.Read("num", 0l);
				for (j=0; j<layer_num; j++)
				{
					if (fconfig.Exists(wxString::Format("/views/%d/layers/%d", i, j)))
					{
						fconfig.SetPath(wxString::Format("/views/%d/layers/%d", i, j));
						int type;
						if (fconfig.Read("type", &type))
						{
							switch (type)
							{
							case 2://volume data
								{
									if (fconfig.Read("name", &str))
									{
										VolumeData* vd = glbin_data_manager.GetVolumeData(str);
										if (vd)
											canvas->AddVolumeData(vd);
									}
								}
								break;
							case 3://mesh data
								{
									if (fconfig.Read("name", &str))
									{
										MeshData* md = glbin_data_manager.GetMeshData(str);
										if (md)
											canvas->AddMeshData(md);
									}
								}
								break;
							case 4://annotations
								{
									if (fconfig.Read("name", &str))
									{
										Annotations* ann = glbin_data_manager.GetAnnotations(str);
										if (ann)
											canvas->AddAnnotations(ann);
									}
								}
								break;
							case 5://group
								{
									if (fconfig.Read("name", &str))
									{
										int id;
										if (fconfig.Read("id", &id))
											DataGroup::SetID(id);
										str = canvas->AddGroup(str);
										DataGroup* group = canvas->GetGroup(str);
										if (group)
										{
											//display
											if (fconfig.Read("display", &bVal))
											{
												group->SetDisp(bVal);
											}
											//2d adjustment
											if (fconfig.Read("gamma", &str))
											{
												float r, g, b;
												if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)){
													fluo::Color col(r,g,b);
													group->SetGammaColor(col);
												}
											}
											if (fconfig.Read("brightness", &str))
											{
												float r, g, b;
												if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)){
													fluo::Color col(r,g,b);
													group->SetBrightness(col);
												}
											}
											if (fconfig.Read("hdr", &str))
											{
												float r, g, b;
												if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)){
													fluo::Color col(r,g,b);
													group->SetHdr(col);
												}
											}
											if (fconfig.Read("sync_r", &bVal))
												group->SetSync(0, bVal);
											if (fconfig.Read("sync_g", &bVal))
												group->SetSync(1, bVal);
											if (fconfig.Read("sync_b", &bVal))
												group->SetSync(2, bVal);
											//sync volume properties
											if (fconfig.Read("sync_vp", &bVal))
												group->SetVolumeSyncProp(bVal);
											//volumes
											if (fconfig.Exists(wxString::Format("/views/%d/layers/%d/volumes", i, j)))
											{
												fconfig.SetPath(wxString::Format("/views/%d/layers/%d/volumes", i, j));
												int vol_num = fconfig.Read("num", 0l);
												for (k=0; k<vol_num; k++)
												{
													if (fconfig.Read(wxString::Format("vol_%d", k), &str))
													{
														VolumeData* vd = glbin_data_manager.GetVolumeData(str);
														if (vd)
														{
															group->InsertVolumeData(k-1, vd);
															//AddProps(2, view, group, vd);
														}
													}
												}
											}
										}
										canvas->SetVolPopDirty();
									}
								}
								break;
							case 6://mesh group
								{
									if (fconfig.Read("name", &str))
									{
										int id;
										if (fconfig.Read("id", &id))
											MeshGroup::SetID(id);
										str = canvas->AddMGroup(str);
										MeshGroup* group = canvas->GetMGroup(str);
										if (group)
										{
											//display
											if (fconfig.Read("display", &bVal))
												group->SetDisp(bVal);
											//sync mesh properties
											if (fconfig.Read("sync_mp", &bVal))
												group->SetMeshSyncProp(bVal);
											//meshes
											if (fconfig.Exists(wxString::Format("/views/%d/layers/%d/meshes", i, j)))
											{
												fconfig.SetPath(wxString::Format("/views/%d/layers/%d/meshes", i, j));
												int mesh_num = fconfig.Read("num", 0l);
												for (k=0; k<mesh_num; k++)
												{
													if (fconfig.Read(wxString::Format("mesh_%d", k), &str))
													{
														MeshData* md = glbin_data_manager.GetMeshData(str);
														if (md)
														{
															group->InsertMeshData(k-1, md);
															//AddProps(3, view, 0, 0, md);
															//AddProps(6, view, 0, 0, md);
														}
													}
												}
											}
										}
										canvas->SetMeshPopDirty();
									}
								}
								break;
							}
						}
					}
				}
			}

			//properties
			if (fconfig.Exists(wxString::Format("/views/%d/properties", i)))
			{
				float x, y, z, w;
				fconfig.SetPath(wxString::Format("/views/%d/properties", i));
				bool draw;
				if (fconfig.Read("drawall", &draw))
					canvas->SetDraw(draw);
				//properties
				bool persp;
				if (fconfig.Read("persp", &persp))
					canvas->SetPersp(persp);
				else
					canvas->SetPersp(true);
				bool free;
				if (fconfig.Read("free", &free))
					canvas->SetFree(free);
				else
					canvas->SetFree(false);
				double aov;
				if (fconfig.Read("aov", &aov))
					canvas->SetAov(aov);
				double nearclip;
				if (fconfig.Read("nearclip", &nearclip))
					canvas->SetNearClip(nearclip);
				double farclip;
				if (fconfig.Read("farclip", &farclip))
					canvas->SetFarClip(farclip);
				if (fconfig.Read("backgroundcolor", &str))
				{
					float r, g, b;
					if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)){
						fluo::Color col(r,g,b);
						canvas->SetBackgroundColor(col);
					}
				}
				int volmethod;
				if (fconfig.Read("volmethod", &volmethod))
					canvas->SetVolMethod(volmethod);
				bool fog;
				if (fconfig.Read("fog", &fog))
					canvas->SetFog(fog);
				double fogintensity;
				if (fconfig.Read("fogintensity", &fogintensity))
					canvas->SetFogIntensity(fogintensity);
				if (fconfig.Read("draw_camctr", &bVal))
				{
					canvas->m_draw_camctr = bVal;
				}
				if (fconfig.Read("draw_info", &iVal))
				{
					canvas->m_draw_info = iVal;
				}
				if (fconfig.Read("draw_legend", &bVal))
				{
					canvas->m_draw_legend = bVal;
				}

				//camera
				if (fconfig.Read("translation", &str))
				{
					if (SSCANF(str.c_str(), "%f%f%f", &x, &y, &z))
						canvas->SetTranslations(x, y, z);
				}
				if (fconfig.Read("rotation", &str))
				{
					if (SSCANF(str.c_str(), "%f%f%f", &x, &y, &z))
						canvas->SetRotations(x, y, z, false);
				}
				if (fconfig.Read("zero_quat", &str))
				{
					if (SSCANF(str.c_str(), "%f%f%f%f", &x, &y, &z, &w))
						canvas->SetZeroQuat(x, y, z, w);
				}
				if (fconfig.Read("center", &str))
				{
					if (SSCANF(str.c_str(), "%f%f%f", &x, &y, &z))
						canvas->SetCenters(x, y, z);
				}
				double dist;
				if (fconfig.Read("centereyedist", &dist))
					canvas->SetCenterEyeDist(dist);
				double radius = 5.0;
				if (fconfig.Read("radius", &radius))
					canvas->SetRadius(radius);
				double initdist;
				if (fconfig.Read("initdist", &initdist))
					canvas->SetInitDist(initdist);
				else
					canvas->SetInitDist(radius/tan(d2r(canvas->GetAov()/2.0)));
				int scale_mode;
				if (fconfig.Read("scale_mode", &scale_mode))
					canvas->m_scale_mode = scale_mode;
				double scale;
				if (!fconfig.Read("scale", &scale))
					scale = radius / tan(d2r(canvas->GetAov() / 2.0)) / dist;
				canvas->m_scale_factor = scale;
				bool pin_rot_center;
				if (fconfig.Read("pin_rot_center", &pin_rot_center))
					canvas->SetPinRotCenter(pin_rot_center);
				//object
				if (fconfig.Read("obj_center", &str))
				{
					if (SSCANF(str.c_str(), "%f%f%f", &x, &y, &z))
						canvas->SetObjCenters(x, y, z);
				}
				if (fconfig.Read("obj_trans", &str))
				{
					if (SSCANF(str.c_str(), "%f%f%f", &x, &y, &z))
						canvas->SetObjTrans(x, y, z);
				}
				if (fconfig.Read("obj_rot", &str))
				{
					if (SSCANF(str.c_str(), "%f%f%f", &x, &y, &z))
					{
						if (l_major <= 2 && d_minor < 24.3)
							canvas->SetObjRot(x, y+180.0, z+180.0);
						else
							canvas->SetObjRot(x, y, z);
					}
				}
				//scale bar
				bool disp;
				if (fconfig.Read("disp_scale_bar", &disp))
					canvas->m_disp_scale_bar = disp;
				if (fconfig.Read("disp_scale_bar_text", &disp))
					canvas->m_disp_scale_bar_text = disp;
				double length;
				if (fconfig.Read("sb_length", &length))
					canvas->m_sb_length = length;
				if (fconfig.Read("sb_text", &str))
					canvas->m_sb_text = str;
				if (fconfig.Read("sb_num", &str))
					canvas->m_sb_num = str;
				int unit;
				if (fconfig.Read("sb_unit", &unit))
					canvas->m_sb_unit = unit;

				//2d sdjustment settings
				if (fconfig.Read("gamma", &str))
				{
					float r, g, b;
					if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)){
						fluo::Color col(r,g,b);
						canvas->SetGammaColor(col);
					}
				}
				if (fconfig.Read("brightness", &str))
				{
					float r, g, b;
					if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)){
						fluo::Color col(r,g,b);
						canvas->SetBrightness(col);
					}
				}
				if (fconfig.Read("hdr", &str))
				{
					float r, g, b;
					if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)){
						fluo::Color col(r,g,b);
						canvas->SetHdr(col);
					}
				}
				if (fconfig.Read("sync_r", &bVal))
					canvas->SetSync(0, bVal);
				if (fconfig.Read("sync_g", &bVal))
					canvas->SetSync(1, bVal);
				if (fconfig.Read("sync_b", &bVal))
					canvas->SetSync(2, bVal);

				//clipping plane rotations
				int clip_mode;
				if (fconfig.Read("clip_mode", &clip_mode))
					canvas->SetClipMode(clip_mode);
				double rotx_cl, roty_cl, rotz_cl;
				if (fconfig.Read("rotx_cl", &rotx_cl) &&
					fconfig.Read("roty_cl", &roty_cl) &&
					fconfig.Read("rotz_cl", &rotz_cl))
				{
					canvas->SetClippingPlaneRotations(rotx_cl, roty_cl, rotz_cl);
				}

				//painting parameters
				double dVal;
				if (fconfig.Read("brush_use_pres", &bVal))
					glbin_vol_selector.SetBrushUsePres(bVal);
				double size1, size2;
				if (fconfig.Read("brush_size_1", &size1) &&
					fconfig.Read("brush_size_2", &size2))
					glbin_vol_selector.SetBrushSize(size1, size2);
				if (fconfig.Read("brush_spacing", &dVal))
					glbin_vol_selector.SetBrushSpacing(dVal);
				if (fconfig.Read("brush_iteration", &dVal))
					glbin_vol_selector.SetBrushIteration(dVal);
				if (fconfig.Read("brush_size_data", &bVal))
					glbin_vol_selector.SetBrushSizeData(bVal);
				if (fconfig.Read("brush_translate", &dVal))
					glbin_vol_selector.SetBrushSclTranslate(dVal);
				if (fconfig.Read("w2d", &dVal))
					glbin_vol_selector.SetW2d(dVal);

			}

			//rulers
			if (canvas->GetRulerList() &&
				fconfig.Exists(wxString::Format("/views/%d/rulers", i)))
			{
				fconfig.SetPath(wxString::Format("/views/%d/rulers", i));
				glbin_ruler_handler.Read(fconfig, i);
			}
		}
	}

	//current selected volume
	if (fconfig.Exists("/prop_panel"))
	{
		fconfig.SetPath("/prop_panel");
		int cur_sel_type, cur_sel_vol, cur_sel_mesh;
		if (fconfig.Read("cur_sel_type", &cur_sel_type) &&
			fconfig.Read("cur_sel_vol", &cur_sel_vol) &&
			fconfig.Read("cur_sel_mesh", &cur_sel_mesh))
		{
			m_cur_sel_type = cur_sel_type;
			m_cur_sel_vol = cur_sel_vol;
			m_cur_sel_mesh = cur_sel_mesh;
			switch (m_cur_sel_type)
			{
			case 2:  //volume
				OnSelection(2, 0, 0, glbin_data_manager.GetVolumeData(cur_sel_vol));
				break;
			case 3:  //mesh
				OnSelection(3, 0, 0, 0, glbin_data_manager.GetMeshData(cur_sel_mesh));
				break;
			}
		}
		else if (fconfig.Read("cur_sel_vol", &cur_sel_vol))
		{
			m_cur_sel_vol = cur_sel_vol;
			if (m_cur_sel_vol != -1)
			{
				VolumeData* vd = glbin_data_manager.GetVolumeData(m_cur_sel_vol);
				OnSelection(2, 0, 0, vd);
			}
		}
		bool bval;
		if (fconfig.Read("chann_link", &bval))
			m_clip_view->SetChannLink(bval);
		if (fconfig.Read("hold planes", &bval))
			m_clip_view->SetHoldPlanes(bval);
		int mode;
		if (fconfig.Read("plane mode", &mode))
			m_clip_view->SetPlaneMode(static_cast<PLANE_MODES>(mode));
		if (fconfig.Read("x_link", &bval))
			m_clip_view->SetXLink(bval);
		if (fconfig.Read("y_link", &bval))
			m_clip_view->SetYLink(bval);
		if (fconfig.Read("z_link", &bval))
			m_clip_view->SetZLink(bval);
	}

	//movie panel
	if (fconfig.Exists("/movie_panel"))
	{
		fconfig.SetPath("/movie_panel");
		wxString sVal;
		bool bVal;
		int iVal;
		double dVal;

		//set settings for frame
		RenderCanvas* canvas = 0;
		if (fconfig.Read("key frame enable", &bVal))
			glbin_moviemaker.SetKeyframeEnable(bVal);
		if (fconfig.Read("views_cmb", &iVal))
		{
			canvas = GetView(iVal);
			glbin_moviemaker.SetView(canvas);
		}
		if (fconfig.Read("rot_check", &bVal))
			glbin_moviemaker.SetRotateEnable(bVal);
		if (fconfig.Read("seq_mode", &iVal))
			glbin_moviemaker.SetSeqMode(iVal);
		if (fconfig.Read("x_rd", &bVal))
		{
			if (bVal)
				glbin_moviemaker.SetRotateAxis(0);
		}
		if (fconfig.Read("y_rd", &bVal))
		{
			if (bVal)
				glbin_moviemaker.SetRotateAxis(1);
		}
		if (fconfig.Read("z_rd", &bVal))
		{
			if (bVal)
				glbin_moviemaker.SetRotateAxis(2);
		}
		if (fconfig.Read("rot_axis", &iVal))
			glbin_moviemaker.SetRotateAxis(iVal);
		if (fconfig.Read("rot_deg", &iVal))
			glbin_moviemaker.SetRotateDeg(iVal);
		if (fconfig.Read("movie_len", &dVal))
			glbin_moviemaker.SetMovieLength(dVal);
		if (fconfig.Read("fps", &dVal))
			glbin_moviemaker.SetFps(dVal);
		if (fconfig.Read("crop", &bVal))
			glbin_moviemaker.SetCropEnable(bVal);
		if (fconfig.Read("crop_x", &iVal))
			glbin_moviemaker.SetCropX(iVal);
		if (fconfig.Read("crop_y", &iVal))
			glbin_moviemaker.SetCropY(iVal);
		if (fconfig.Read("crop_w", &iVal))
			glbin_moviemaker.SetCropW(iVal);
		if (fconfig.Read("crop_h", &iVal))
			glbin_moviemaker.SetCropH(iVal);
		if (fconfig.Read("full frame num", &iVal))
			glbin_moviemaker.SetFullFrameNum(iVal);
		int startf = 0, endf = 0, curf = 0;
		if (fconfig.Read("start_frame", &startf) &&
			fconfig.Read("end_frame", &endf))
			glbin_moviemaker.SetClipStartEndFrames(startf, endf);
		if (fconfig.Read("cur_frame", &curf))
		{
			if (curf && curf >= startf && curf <= endf)
			{
				glbin_moviemaker.SetCurrentTime(curf);
				RenderCanvas* canvas = GetLastView();
				if (canvas)
				{
					canvas->Set4DSeqFrame(curf, startf, endf, false);
				}
			}
		}
		if (fconfig.Read("run_script", &bVal))
			glbin_settings.m_run_script = bVal;
		if (fconfig.Read("script_file", &sVal))
			glbin_settings.m_script_file = sVal;
		m_movie_panel->FluoUpdate();
	}

	//tracking diag
	if (fconfig.Exists("/track_diag"))
	{
		fconfig.SetPath("/track_diag");
		wxString sVal;
		if (fconfig.Read("track_file", &sVal))
		{
			m_trace_dlg->GetSettings(m_vrv_list[0]->m_canvas);
			m_trace_dlg->LoadTrackFile(sVal);
		}
	}
	/*	//brushtool diag
	if (fconfig.Exists("/brush_diag"))
	{
		fconfig.SetPath("/brush_diag");
		double dval;

		if (fconfig.Read("ca_min", &dval))
			m_brush_tool_dlg->SetDftCAMin(dval);
		if (fconfig.Read("ca_max", &dval))
			m_brush_tool_dlg->SetDftCAMax(dval);
		if (fconfig.Read("ca_thresh", &dval))
			m_brush_tool_dlg->SetDftCAThresh(dval);
		if (fconfig.Read("nr_thresh", &dval))
		{
			m_brush_tool_dlg->SetDftNRThresh(dval);
			m_noise_cancelling_dlg->SetDftThresh(dval);
		}
		if (fconfig.Read("nr_size", &dval))
		{
			m_brush_tool_dlg->SetDftNRSize(dval);
			m_noise_cancelling_dlg->SetDftSize(dval);
		}
	}*/

	//ui layout
	if (fconfig.Exists("/ui_layout"))
	{
		fconfig.SetPath("/ui_layout");
		wxString str;
		double dval;
		bool update = false;
		if (fconfig.Read("dpi scale factor", &dval))
			update = fluo::InEpsilon(dval, GetDPIScaleFactor());
		if (update && fconfig.Read("layout", &str))
		{
			m_aui_mgr.LoadPerspective(str);
			m_aui_mgr.Update();
		}
	}

	//interpolator
	if (fconfig.Exists("/interpolator"))
	{
		wxString str;
		wxString sVal;
		double dVal;

		fconfig.SetPath("/interpolator");
		glbin_interpolator.Clear();
		if (fconfig.Read("max_id", &iVal))
			Interpolator::m_id = iVal;
		vector<FlKeyGroup*>* key_list = glbin_interpolator.GetKeyList();
		int group_num = fconfig.Read("num", 0l);
		for (i=0; i<group_num; i++)
		{
			str = wxString::Format("/interpolator/%d", i);
			if (fconfig.Exists(str))
			{
				fconfig.SetPath(str);
				FlKeyGroup* key_group = new FlKeyGroup;
				if (fconfig.Read("id", &iVal))
					key_group->id = iVal;
				if (fconfig.Read("t", &dVal))
					key_group->t = dVal;
				if (fconfig.Read("dt", &dVal))
					key_group->dt = dVal;
				else
				{
					if (key_list->empty())
						key_group->dt = 0;
					else
						key_group->dt = key_group->t - key_list->back()->t;
				}
				if (fconfig.Read("type", &iVal))
					key_group->type = iVal;
				if (fconfig.Read("desc", &sVal))
					key_group->desc = sVal.ToStdString();
				str = wxString::Format("/interpolator/%d/keys", i);
				if (fconfig.Exists(str))
				{
					fconfig.SetPath(str);
					int key_num = fconfig.Read("num", 0l);
					for (j=0; j<key_num; j++)
					{
						str = wxString::Format("/interpolator/%d/keys/%d", i, j);
						if (fconfig.Exists(str))
						{
							fconfig.SetPath(str);
							int key_type;
							if (fconfig.Read("type", &key_type))
							{
								FlKeyCode code;
								if (fconfig.Read("l0", &iVal))
									code.l0 = iVal;
								if (fconfig.Read("l0_name", &sVal))
									code.l0_name = sVal.ToStdString();
								if (fconfig.Read("l1", &iVal))
									code.l1 = iVal;
								if (fconfig.Read("l1_name", &sVal))
									code.l1_name = sVal.ToStdString();
								if (fconfig.Read("l2", &iVal))
									code.l2 = iVal;
								if (fconfig.Read("l2_name", &sVal))
									code.l2_name = sVal.ToStdString();
								switch (key_type)
								{
								case FLKEY_TYPE_DOUBLE:
									{
										if (fconfig.Read("val", &dVal))
										{
											FlKeyDouble* key = new FlKeyDouble(code, dVal);
											key_group->keys.push_back(key);
										}
									}
									break;
								case FLKEY_TYPE_QUATER:
									{
										if (fconfig.Read("val", &sVal))
										{
											double x, y, z, w;
											if (SSCANF(sVal.c_str(), "%lf%lf%lf%lf",
												&x, &y, &z, &w))
											{
												fluo::Quaternion qval = fluo::Quaternion(x, y, z, w);
												FlKeyQuaternion* key = new FlKeyQuaternion(code, qval);
												key_group->keys.push_back(key);
											}
										}
									}
									break;
								case FLKEY_TYPE_BOOLEAN:
									{
										if (fconfig.Read("val", &bVal))
										{
											FlKeyBoolean* key = new FlKeyBoolean(code, bVal);
											key_group->keys.push_back(key);
										}
									}
									break;
								case FLKEY_TYPE_INT:
									{
										if (fconfig.Read("val", &iVal))
										{
											FlKeyInt* key = new FlKeyInt(code, iVal);
											key_group->keys.push_back(key);
										}
									}
									break;
								case FLKEY_TYPE_COLOR:
									{
										if (fconfig.Read("val", &sVal))
										{
											double r, g, b;
											if (SSCANF(sVal.c_str(), "%lf%lf%lf",
												&r, &g, &b))
											{
												fluo::Color cval = fluo::Color(r, g, b);
												FlKeyColor* key = new FlKeyColor(code, cval);
												key_group->keys.push_back(key);
											}
										}
									}
									break;
								}
							}
						}
					}
				}
				key_list->push_back(key_group);
			}
		}
		//m_recorder_dlg->UpdateList();
	}

	if (m_cur_sel_type != -1)
	{
		switch (m_cur_sel_type)
		{
		case 2:  //volume
			if (glbin_data_manager.GetVolumeData(m_cur_sel_vol))
				glbin.set_tree_selection(glbin_data_manager.GetVolumeData(m_cur_sel_vol)->GetName().ToStdString());
			else
				glbin.set_tree_selection("");
			break;
		case 3:  //mesh
			if (glbin_data_manager.GetMeshData(m_cur_sel_mesh))
				glbin.set_tree_selection(glbin_data_manager.GetMeshData(m_cur_sel_mesh)->GetName().ToStdString());
			else
				glbin.set_tree_selection("");
			break;
		default:
			glbin.set_tree_selection("");
		}
	}
	else if (m_cur_sel_vol != -1)
	{
		if (glbin_data_manager.GetVolumeData(m_cur_sel_vol))
			glbin.set_tree_selection(glbin_data_manager.GetVolumeData(m_cur_sel_vol)->GetName().ToStdString());
		else
			glbin.set_tree_selection("");
	}
	else
		glbin.set_tree_selection("");

	if (m_movie_panel)
		m_movie_panel->SetView(0);
	delete prg_diag;

	RefreshCanvases();
	UpdateProps({}, 0, 0);
}

void MainFrame::OnSettings(wxCommandEvent& event)
{
	//m_setting_dlg->UpdateDeviceTree();
	m_aui_mgr.GetPane(m_setting_dlg).Show();
	m_aui_mgr.GetPane(m_setting_dlg).Float();
	m_aui_mgr.Update();
	m_setting_dlg->SetFocus();
}

//undo redo
void MainFrame::OnUndo(wxCommandEvent& event)
{
	glbin.undo();
}

void MainFrame::OnRedo(wxCommandEvent& event)
{
	glbin.redo();
}

//tools
void MainFrame::OnLastTool(wxCommandEvent& event)
{
	unsigned int tool = glbin_settings.m_last_tool;
	switch (tool)
	{
	case 0:
	case TOOL_PAINT_BRUSH:
	default:
		ShowPaintTool();
		break;
	case TOOL_MEASUREMENT:
		ShowMeasureDlg();
		break;
	case TOOL_TRACKING:
		ShowTraceDlg();
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

void MainFrame::OnPaintTool(wxCommandEvent& event)
{
	ShowPaintTool();
}

void MainFrame::OnMeasure(wxCommandEvent& event)
{
	ShowMeasureDlg();
}

void MainFrame::OnNoiseCancelling(wxCommandEvent& event)
{
	ShowNoiseCancellingDlg();
}

void MainFrame::OnCounting(wxCommandEvent& event)
{
	ShowCountingDlg();
}

void MainFrame::OnColocalization(wxCommandEvent& event)
{
	ShowColocalizationDlg();
}

void MainFrame::OnConvert(wxCommandEvent& event)
{
	ShowConvertDlg();
}

void MainFrame::OnTrace(wxCommandEvent& event)
{
	ShowTraceDlg();
}

void MainFrame::OnOcl(wxCommandEvent& event)
{
	ShowOclDlg();
}

void MainFrame::OnComponent(wxCommandEvent& event)
{
	ShowComponentDlg();
}

void MainFrame::OnCalculations(wxCommandEvent& event)
{
	ShowCalculationDlg();
}

void MainFrame::OnMachineLearning(wxCommandEvent& event)
{
	ShowMachineLearningDlg();
}

void MainFrame::ShowPaintTool()
{
	m_aui_mgr.GetPane(m_brush_tool_dlg).Show();
	m_aui_mgr.GetPane(m_brush_tool_dlg).Float();
	m_aui_mgr.Update();
	glbin_settings.m_last_tool = TOOL_PAINT_BRUSH;
	m_main_tb->SetToolBitmap(ID_LastTool,
		wxGetBitmapFromMemory(icon_paint_brush));
	m_brush_tool_dlg->SetFocus();
}

void MainFrame::ShowMeasureDlg()
{
	m_aui_mgr.GetPane(m_measure_dlg).Show();
	m_aui_mgr.GetPane(m_measure_dlg).Float();
	m_aui_mgr.Update();
	glbin_settings.m_last_tool = TOOL_MEASUREMENT;
	m_main_tb->SetToolBitmap(ID_LastTool,
		wxGetBitmapFromMemory(icon_measurement));
	m_measure_dlg->SetFocus();
}

void MainFrame::ShowTraceDlg()
{
	m_aui_mgr.GetPane(m_trace_dlg).Show();
	m_aui_mgr.GetPane(m_trace_dlg).Float();
	m_aui_mgr.Update();
	glbin_settings.m_last_tool = TOOL_TRACKING;
	m_main_tb->SetToolBitmap(ID_LastTool,
		wxGetBitmapFromMemory(icon_tracking));
	m_trace_dlg->SetFocus();
}

void MainFrame::ShowNoiseCancellingDlg()
{
	m_aui_mgr.GetPane(m_noise_cancelling_dlg).Show();
	m_aui_mgr.GetPane(m_noise_cancelling_dlg).Float();
	m_aui_mgr.Update();
	glbin_settings.m_last_tool = TOOL_NOISE_REDUCTION;
	m_main_tb->SetToolBitmap(ID_LastTool,
		wxGetBitmapFromMemory(icon_noise_reduc));
	m_noise_cancelling_dlg->SetFocus();
}

void MainFrame::ShowCountingDlg()
{
	m_aui_mgr.GetPane(m_counting_dlg).Show();
	m_aui_mgr.GetPane(m_counting_dlg).Float();
	m_aui_mgr.Update();
	glbin_settings.m_last_tool = TOOL_VOLUME_SIZE;
	m_main_tb->SetToolBitmap(ID_LastTool,
		wxGetBitmapFromMemory(icon_volume_size));
	m_counting_dlg->SetFocus();
}

void MainFrame::ShowColocalizationDlg()
{
	m_aui_mgr.GetPane(m_colocalization_dlg).Show();
	m_aui_mgr.GetPane(m_colocalization_dlg).Float();
	m_aui_mgr.Update();
	glbin_settings.m_last_tool = TOOL_COLOCALIZATION;
	m_main_tb->SetToolBitmap(ID_LastTool,
		wxGetBitmapFromMemory(icon_colocalization));
	m_colocalization_dlg->SetFocus();
}

void MainFrame::ShowConvertDlg()
{
	m_aui_mgr.GetPane(m_convert_dlg).Show();
	m_aui_mgr.GetPane(m_convert_dlg).Float();
	m_aui_mgr.Update();
	glbin_settings.m_last_tool = TOOL_CONVERT;
	m_main_tb->SetToolBitmap(ID_LastTool,
		wxGetBitmapFromMemory(icon_convert));
	m_convert_dlg->SetFocus();
}

void MainFrame::ShowOclDlg()
{
	m_aui_mgr.GetPane(m_ocl_dlg).Show();
	m_aui_mgr.GetPane(m_ocl_dlg).Float();
	m_aui_mgr.Update();
	glbin_settings.m_last_tool = TOOL_OPENCL;
	m_main_tb->SetToolBitmap(ID_LastTool,
		wxGetBitmapFromMemory(icon_opencl));
	m_ocl_dlg->SetFocus();
}

void MainFrame::ShowComponentDlg()
{
	m_aui_mgr.GetPane(m_component_dlg).Show();
	m_aui_mgr.GetPane(m_component_dlg).Float();
	m_aui_mgr.Update();
	glbin_settings.m_last_tool = TOOL_COMPONENT;
	m_main_tb->SetToolBitmap(ID_LastTool,
		wxGetBitmapFromMemory(icon_components));
	m_component_dlg->SetFocus();
}

void MainFrame::ShowCalculationDlg()
{
	m_aui_mgr.GetPane(m_calculation_dlg).Show();
	m_aui_mgr.GetPane(m_calculation_dlg).Float();
	m_aui_mgr.Update();
	glbin_settings.m_last_tool = TOOL_CALCULATIONS;
	m_main_tb->SetToolBitmap(ID_LastTool,
		wxGetBitmapFromMemory(icon_calculations));
	m_calculation_dlg->SetFocus();
}

void MainFrame::ShowMachineLearningDlg()
{
	m_aui_mgr.GetPane(m_machine_learning_dlg).Show();
	m_aui_mgr.GetPane(m_machine_learning_dlg).Float();
	m_aui_mgr.Update();
	glbin_settings.m_last_tool = TOOL_MACHINE_LEARNING;
	m_main_tb->SetToolBitmap(ID_LastTool,
		wxGetBitmapFromMemory(icon_machine_learning));
	m_machine_learning_dlg->SetFocus();
}

void MainFrame::ShowScriptBreakDlg(bool show)
{
	if (show)
		m_aui_mgr.GetPane(m_script_break_dlg).
			Show().Float().Caption(
			m_script_break_dlg->GetLabel());
	else
		m_aui_mgr.GetPane(m_script_break_dlg).Hide();
	m_aui_mgr.Update();
	if (show)
		m_script_break_dlg->SetFocus();
}

//quit option
void MainFrame::OnQuit(wxCommandEvent& event)
{
	Close(true);
}

void MainFrame::OnFacebook(wxCommandEvent& event)
{
	::wxLaunchDefaultBrowser(FACEBOOK_URL);
}

void MainFrame::OnManual(wxCommandEvent& event)
{
	::wxLaunchDefaultBrowser(HELP_MANUAL);
}


void MainFrame::OnTutorial(wxCommandEvent& event)
{
	::wxLaunchDefaultBrowser(HELP_TUTORIAL);
}

void MainFrame::OnTwitter(wxCommandEvent& event)
{
	::wxLaunchDefaultBrowser(TWITTER_URL);
}

void MainFrame::OnYoutube(wxCommandEvent& event)
{
	::wxLaunchDefaultBrowser(YOUTUBE_URL);
}

void MainFrame::OnShowHideUI(wxCommandEvent& event)
{
	ToggleAllTools(true);
}

void MainFrame::OnShowHideToolbar(wxCommandEvent& event)
{
	if (m_aui_mgr.GetPane(m_main_tb).IsShown()) {
		m_aui_mgr.GetPane(m_main_tb).Hide();
		m_top_window->Check(ID_ShowHideToolbar,false);
	} else {
		m_aui_mgr.GetPane(m_main_tb).Show();
		m_top_window->Check(ID_ShowHideToolbar,true);
	}
	m_aui_mgr.Update();
}

void MainFrame::OnShowHideView(wxCommandEvent &event)
{
	int id = event.GetId();

	switch (id)
	{
	case ID_UIProjView:
		//data view
		if (m_aui_mgr.GetPane(m_proj_panel).IsShown())
		{
			m_aui_mgr.GetPane(m_proj_panel).Hide();
			m_tb_menu_ui->Check(ID_UIProjView, false);
		}
		else
		{
			m_aui_mgr.GetPane(m_proj_panel).Show();
			m_tb_menu_ui->Check(ID_UIProjView, true);
		}
		break;
	case ID_UIMovieView:
		//movie view
		if (m_aui_mgr.GetPane(m_movie_panel).IsShown())
		{
			m_aui_mgr.GetPane(m_movie_panel).Hide();
			m_tb_menu_ui->Check(ID_UIMovieView, false);
		}
		else
		{
			m_aui_mgr.GetPane(m_movie_panel).Show();
			m_tb_menu_ui->Check(ID_UIMovieView, true);
		}
		break;
	case ID_UIAdjView:
		//adjust view
		if (m_aui_mgr.GetPane(m_adjust_view).IsShown())
		{
			m_aui_mgr.GetPane(m_adjust_view).Hide();
			m_tb_menu_ui->Check(ID_UIAdjView, false);
		}
		else
		{
			m_aui_mgr.GetPane(m_adjust_view).Show();
			m_tb_menu_ui->Check(ID_UIAdjView, true);
		}
		break;
	case ID_UIClipView:
		//clipping view
		if (m_aui_mgr.GetPane(m_clip_view).IsShown())
		{
			m_aui_mgr.GetPane(m_clip_view).Hide();
			m_tb_menu_ui->Check(ID_UIClipView, false);
		}
		else
		{
			m_aui_mgr.GetPane(m_clip_view).Show();
			m_tb_menu_ui->Check(ID_UIClipView, true);
		}
		break;
	case ID_UIPropView:
		//properties
		if (m_aui_mgr.GetPane(m_prop_panel).IsShown())
		{
			m_aui_mgr.GetPane(m_prop_panel).Hide();
			m_tb_menu_ui->Check(ID_UIPropView, false);
		}
		else
		{
			m_aui_mgr.GetPane(m_prop_panel).Show();
			m_tb_menu_ui->Check(ID_UIPropView, true);
		}
		break;
	}

	m_aui_mgr.Update();
}

//toolbar menus
void MainFrame::OnToolbarMenu(wxAuiToolBarEvent& event)
{
	if (event.IsDropDownClicked())
	{
		wxAuiToolBar* tb = static_cast<wxAuiToolBar*>(event.GetEventObject());

		tb->SetToolSticky(event.GetId(), true);

		// line up our menu with the button
		wxRect rect = tb->GetToolRect(event.GetId());
		wxPoint pt = tb->ClientToScreen(rect.GetBottomLeft());
		pt = ScreenToClient(pt);

		int id = event.GetId();
		wxMenu* menu = 0;
		switch (id)
		{
		case ID_ShowHideUI:
			menu = m_tb_menu_ui;
			break;
		case ID_LastTool:
			menu = m_tb_menu_edit;
			break;
		case ID_CheckUpdates:
		case ID_Youtube:
		case ID_Twitter:
		case ID_Info:
			menu = m_tb_menu_update;
			break;
		}
		if (menu)
			PopupMenu(menu, pt);

		// make sure the button is "un-stuck"
		tb->SetToolSticky(event.GetId(), false);
	}
	event.Skip();
}

//panes
void MainFrame::OnPaneClose(wxAuiManagerEvent& event)
{
	wxWindow* wnd = event.pane->window;
	wxString name = wnd->GetName();

	if (name == "Toolbar")
		m_top_window->Check(ID_ShowHideToolbar, false);
	else if (name == "ProjectPanel")
	{
		m_tb_menu_ui->Check(ID_UIProjView, false);
		m_top_window->Check(ID_UIProjView, false);
	}
	else if (name == "MoviePanel")
	{
		m_tb_menu_ui->Check(ID_UIMovieView, false);
		m_top_window->Check(ID_UIMovieView, false);
	}
	else if (name == "PropPanel")
	{
		m_tb_menu_ui->Check(ID_UIPropView, false);
		m_top_window->Check(ID_UIPropView, false);
	}
	else if (name == "OutputAdjPanel")
	{
		m_tb_menu_ui->Check(ID_UIAdjView, false);
		m_top_window->Check(ID_UIAdjView, false);
	}
	else if (name == "ClipPlanePanel")
	{
		m_tb_menu_ui->Check(ID_UIClipView, false);
		m_top_window->Check(ID_UIClipView, false);
	}
}

//prop pages
void MainFrame::OnPropPageClose(wxAuiNotebookEvent& event)
{
	wxAuiNotebook* panel = (wxAuiNotebook*)event.GetEventObject();
	wxWindow* page = panel->GetPage(event.GetSelection());
	if (page)
	{
		int page_no = m_prop_panel->FindPage(page);
		page->Hide();
		m_prop_panel->RemovePage(page_no);
	}
	event.Veto();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainFrame::OnDraw(wxPaintEvent& event)
{
	//wxPaintDC dc(this);

	//wxRect windowRect(wxPoint(0, 0), GetClientSize());
	//dc.SetBrush(wxBrush(*wxGREEN, wxSOLID));
	//dc.DrawRectangle(windowRect);
}

void MainFrame::OnKeyDown(wxKeyEvent& event)
{
	event.Skip();
}

