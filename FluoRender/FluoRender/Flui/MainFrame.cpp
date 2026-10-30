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
#include <Main.h>
#include <MainFrame.h>
#include <MainFrameAgent.h>
#include <Global.h>
#include <Tokens.h>
#include <MainSettings.h>
#include <FluiBuilder.h>
#include <JVMInitializer.h>
#include <PanelsAndDialogs.h>
#include <UiText.h>
#include <Debug.h>
#include <wxGaugeStatusbar.h>
#include <wxBasisSlider.h>
#include <wxToolbarArt.h>
#include <wx/aboutdlg.h>
#include <wx/hyperlink.h>
#include <wx/accel.h>
#include <wx/display.h>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <cctype>
//resources
#include <icons.h>
#include <png_resource.h>

MainFrame::MainFrame(
	const wxString& title,
	int x, int y,
	int w, int h,
	int reset,
	bool benchmark,
	bool fullscreen,
	bool windowed,
	bool hidepanels)
	: wxFrame(0, wxID_ANY, title, wxPoint(x, y), wxSize(w, h),wxDEFAULT_FRAME_STYLE),
	m_reset(reset),
	m_benchmark(benchmark),
	m_fullscreen(fullscreen),
	m_windowed(windowed),
	m_hidepanels(hidepanels),
	m_title(title),
	m_movie_panel(0),
	m_tree_panel(0),
	m_list_panel(0),
	m_property_panel(0),
	m_clip_plane_panel(0),
	m_output_adj_panel(0),
	m_setting_dlg(0),
	m_help_dlg(0),
	m_brush_tool_dlg(0),
	m_noise_cancelling_dlg(0),
	m_counting_dlg(0),
	m_convert_dlg(0),
	m_colocalization_dlg(0),
	m_measure_dlg(0),
	m_track_dlg(0),
	m_ocl_dlg(0),
	m_component_dlg(0),
	m_machine_learning_dlg(0),
	m_script_break_dlg(0),
	m_fp_range_dlg(0),
	m_waker(this)
{
}

MainFrame::~MainFrame()
{
	m_aui_mgr.UnInit();

	m_waker.Stop();
}

void MainFrame::CreateUI()
{
#ifdef _DARWIN
	SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#elifdef __linux__
	SetWindowVariant(wxWINDOW_VARIANT_MINI);
#endif

	// tell wxAuiManager to manage this frame
	m_aui_mgr.SetManagedWindow(this);

	// set frame icon
	wxIcon icon;
	icon.CopyFromBitmap(wxGetBitmap(icon_32).GetBitmap(wxSize(32, 32)));
	SetIcon(icon);
	wxBitmapBundle bitmap;

	// create the main toolbar
	wxMenuItem* m;
	wxAuiToolBarItemArray prepend_items;
	wxAuiToolBarItemArray append_items;
	m_main_tb = new wxAuiToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxAUI_TB_DEFAULT_STYLE | wxAUI_TB_OVERFLOW | wxAUI_TB_HORIZONTAL);
	m_main_tb->SetExtraStyle(m_main_tb->GetExtraStyle() | wxWS_EX_PROCESS_UI_UPDATES);
	m_main_tb->SetName("Toolbar");
#ifdef _WIN32
	m_main_tb->SetArtProvider(new wxToolbarArt());
#endif

	//add tools
	bool jvm_valid = glbin_jvm_instance->IsValid();
	int last_type = glbin_settings.m_last_open_type;
	if (!jvm_valid && last_type == 2)
		last_type = 0;
	bitmap = wxGetBitmap(icon_open_volume);
	m_main_tb->AddTool(ID_LastOpen, "Open Volume", bitmap,
		"Open single or multiple volume data file(s)");
	m_main_tb->SetToolDropDown(ID_LastOpen, true);
	switch (last_type)
	{
	case 0://volume
		m_main_tb->SetToolBitmap(ID_LastOpen,
			wxGetBitmap(icon_open_volume));
		m_main_tb->SetToolLabel(ID_LastOpen, "Open Volume");
		m_main_tb->SetToolShortHelp(ID_LastOpen, "Open single or multiple volume data file(s)");
		m_main_tb->SetToolLongHelp(ID_LastOpen, "Open single or multiple volume data file(s)");
		break;
	case 1://mesh
		m_main_tb->SetToolBitmap(ID_LastOpen,
			wxGetBitmap(icon_open_mesh));
		m_main_tb->SetToolLabel(ID_LastOpen, "Open Mesh");
		m_main_tb->SetToolShortHelp(ID_LastOpen, "Open single or multiple mesh file(s)");
		m_main_tb->SetToolLongHelp(ID_LastOpen, "Open single or multiple mesh file(s)");
		break;
	case 2://imagej
		m_main_tb->SetToolBitmap(ID_LastOpen,
			wxGetBitmap(icon_import));
		m_main_tb->SetToolLabel(ID_LastOpen, "Import Volume");
		m_main_tb->SetToolShortHelp(ID_LastOpen, "Import single or multiple volume data file(s) using ImageJ");
		m_main_tb->SetToolLongHelp(ID_LastOpen, "Import single or multiple volume data file(s) using ImageJ");
		break;
	}
	m_main_tb->AddSeparator();
	bitmap = wxGetBitmap(icon_open_project);
	m_main_tb->AddTool(ID_OpenProject, "Open Project", bitmap,
		"Open a saved project");
	m_main_tb->SetToolLongHelp(ID_OpenProject, "Open a saved project (will close current project if there is one)");
	bitmap = wxGetBitmap(icon_save_project);
	m_main_tb->AddTool(ID_SaveProject, "Save Project", bitmap,
		"Save current work as a project");
	m_main_tb->SetToolLongHelp(ID_SaveProject, "Save current work as a project (will save all opened data and current settings)");
	m_main_tb->AddSeparator();
	bitmap = wxGetBitmap(icon_new_view);
	m_main_tb->AddTool(ID_ViewNew, "New View", bitmap,
		"Create a new render viewport");
	m_main_tb->SetToolLongHelp(ID_ViewNew, "Create a new render viewport (will create a new view with the same settings as current view)");
	bitmap = wxGetBitmap(icon_show_hide_ui);
	m_main_tb->AddTool(ID_Panels, "Show/Hide UI", bitmap,
		"Show or hide all control panels");
	m_main_tb->SetToolLongHelp(ID_Panels, "Show or hide all control panels");
	m_main_tb->SetToolDropDown(ID_Panels, true);
	m_main_tb->AddSeparator();
	bitmap = wxGetBitmap(icon_paint_brush);
	m_main_tb->AddTool(ID_LastTool, "Analyze", bitmap,
		"Tools for analyzing selected channel");
	m_main_tb->SetToolDropDown(ID_LastTool, true);
	//set analyze icon
	switch (glbin_settings.m_last_tool)
	{
	case TOOL_PAINT_BRUSH:
	default:
		m_main_tb->SetToolBitmap(ID_LastTool,
			wxGetBitmap(icon_paint_brush));
		m_main_tb->SetToolLongHelp(ID_LastTool, "Paint Brush: Use brush to select voxels in the volume for further analysis");
		break;
	case TOOL_MEASUREMENT:
		m_main_tb->SetToolBitmap(ID_LastTool,
			wxGetBitmap(icon_measurement));
		m_main_tb->SetToolLongHelp(ID_LastTool, "Measurement: Measure distances, angles, and intensity in the volume");
		break;
	case TOOL_TRACKING:
		m_main_tb->SetToolBitmap(ID_LastTool,
			wxGetBitmap(icon_tracking));
		m_main_tb->SetToolLongHelp(ID_LastTool, "Tracking: Track objects or features in the volume");
		break;
	case TOOL_NOISE_REDUCTION:
		m_main_tb->SetToolBitmap(ID_LastTool,
			wxGetBitmap(icon_noise_reduc));
		m_main_tb->SetToolLongHelp(ID_LastTool, "Noise Reduction: Reduce noise in the volume data");
		break;
	case TOOL_VOLUME_SIZE:
		m_main_tb->SetToolBitmap(ID_LastTool,
			wxGetBitmap(icon_volume_size));
		m_main_tb->SetToolLongHelp(ID_LastTool, "Volume Size: Analyze the size of the volume data");
		break;
	case TOOL_COLOCALIZATION:
		m_main_tb->SetToolBitmap(ID_LastTool,
			wxGetBitmap(icon_colocalization));
		m_main_tb->SetToolLongHelp(ID_LastTool, "Colocalization: Analyze the colocalization of different channels in the volume");
		break;
	case TOOL_CONVERT:
		m_main_tb->SetToolBitmap(ID_LastTool,
			wxGetBitmap(icon_convert));
		m_main_tb->SetToolLongHelp(ID_LastTool, "Convert: Convert volume data to mesh objects");
		break;
	case TOOL_OPENCL:
		m_main_tb->SetToolBitmap(ID_LastTool,
			wxGetBitmap(tb_volume_filter));
		m_main_tb->SetToolLongHelp(ID_LastTool, "Volume Filter: Edit and apply volume filters using OpenCL");
		break;
	case TOOL_COMPONENT:
		m_main_tb->SetToolBitmap(ID_LastTool,
			wxGetBitmap(icon_components));
		m_main_tb->SetToolLongHelp(ID_LastTool, "Component Analyzer: Segment structures into components and perform analysis");
		break;
	case TOOL_CALCULATIONS:
		m_main_tb->SetToolBitmap(ID_LastTool,
			wxGetBitmap(icon_calculations));
		m_main_tb->SetToolLongHelp(ID_LastTool, "Calculations: Perform calculations on the volume data");
		break;
	case TOOL_MACHINE_LEARNING:
		m_main_tb->SetToolBitmap(ID_LastTool,
			wxGetBitmap(icon_machine_learning));
		m_main_tb->SetToolLongHelp(ID_LastTool, "Machine Learning: Use machine learning algorithms to analyze the volume data");
		break;
	}
	m_main_tb->AddSeparator();
	bitmap = wxGetBitmap(icon_undo);
	m_main_tb->AddTool(ID_Undo, "Undo", bitmap,
		"Undo");
	m_main_tb->SetToolLongHelp(ID_Undo, "Undo the last action");
	bitmap = wxGetBitmap(icon_redo);
	m_main_tb->AddTool(ID_Redo, "Redo", bitmap,
		"Redo");
	m_main_tb->SetToolLongHelp(ID_Redo, "Redo the last undone action");
	//right-side items
	m_main_tb->AddStretchSpacer();
	bitmap = wxGetBitmap(icon_settings);
	m_main_tb->AddTool(ID_Settings, "Configurations", bitmap,
		"Settings of FluoRender");
	m_main_tb->SetToolLongHelp(ID_Settings, "Settings of FluoRender (will open the settings dialog)");
	m_main_tb->AddSeparator();
	int num = std::rand() % 4;
	wxString str1, str2, str3;
	int item_id;
	switch (num)
	{
	case 0:
		bitmap = wxGetBitmap(icon_check_updates);
		str1 = "Check Updates";
		str2 = "Check if there is a new release";
		str3 = "Check if there is a new release (requires Internet connection)";
		item_id = ID_CheckUpdates;
		break;
	case 1:
		bitmap = wxGetBitmap(icon_youtube);
		str1 = "Video Tutorials";
		str2 = "FluoRender's YouTube channel & Tutorials";
		str3 = "FluoRender's YouTube channel & Tutorials (requires Internet connection)";
		item_id = ID_Youtube;
		break;
	case 2:
		bitmap = wxGetBitmap(icon_twitter);
		str1 = "Twitter";
		str2 = "Follow FluoRender on Twitter";
		str3 = "Follow FluoRender on Twitter (requires Internet connection)";
		item_id = ID_Twitter;
		break;
	case 3:
	default:
		bitmap = wxGetBitmap(icon_about);
		str1 = "About";
		str2 = "FluoRender information";
		str3 = "FluoRender information";
		item_id = ID_Info;
		break;
	}
	m_main_tb->AddTool(item_id, str1, bitmap, str2);
	m_main_tb->SetToolLongHelp(item_id, str3);
	m_main_tb->SetToolDropDown(item_id, true);
	m_main_tb->SetCustomOverflowItems(prepend_items, append_items);
	m_main_tb->Bind(wxEVT_AUITOOLBAR_TOOL_DROPDOWN, &MainFrame::OnToolbarMenu, this);
	m_main_tb->Realize();

	//menu for open
	m_tb_menu_open = std::make_unique<wxMenu>();
	m = m_tb_menu_open->Append(ID_OpenVolume, "Open Volume...",
		"Open single or multiple volume data file(s)");
	m->SetBitmap(wxGetBitmap(icon_open_volume_mini));
	m = m_tb_menu_open->Append(ID_OpenMesh, "Open Mesh...",
		"Open single or multiple mesh file(s)");
	m->SetBitmap(wxGetBitmap(icon_open_mesh_mini));
	if (jvm_valid)
	{
		m = m_tb_menu_open->Append(ID_ImportVolume, "Import Volume...",
			"Import single or multiple volume data file(s) using ImageJ");
		m->SetBitmap(wxGetBitmap(icon_import_mini));
	}
	//create the menu for UI management
	m_tb_menu_ui = std::make_unique<wxMenu>();
	m = m_tb_menu_ui->Append(ID_ProjPanel, UITEXT_PROJECT,
		"Show/hide the project panel", wxITEM_CHECK);
	m->SetBitmap(wxGetBitmap(disp_project));
	m = m_tb_menu_ui->Append(ID_MoviePanel, UITEXT_MAKEMOVIE,
		"Show/hide the movie making panel", wxITEM_CHECK);
	m->SetBitmap(wxGetBitmap(disp_makemovie));
	m = m_tb_menu_ui->Append(ID_OutAdjPanel, UITEXT_ADJUST,
		"Show/hide the output adjustment panel", wxITEM_CHECK);
	m->SetBitmap(wxGetBitmap(disp_adjust));
	m = m_tb_menu_ui->Append(ID_ClipPlanePanel, UITEXT_CLIPPING,
		"Show/hide the clipping plane control panel", wxITEM_CHECK);
	m->SetBitmap(wxGetBitmap(disp_clipping));
	m = m_tb_menu_ui->Append(ID_PropPanel, UITEXT_PROPERTIES,
		"Show/hide the property panel", wxITEM_CHECK);
	m->SetBitmap(wxGetBitmap(disp_properties));
	//create the menu for edit/convert
	m_tb_menu_edit = std::make_unique<wxMenu>();
	m = m_tb_menu_edit->Append(ID_OclDlg, "Volume Filter...",
		"Edit and apply volume filters using OpenCL");
	m->SetBitmap(wxGetBitmap(icon_filter_mini));
	m = m_tb_menu_edit->Append(ID_BrushDlg, "Paint Brush...",
		"Use the paint brush to select regions of interest in 3D");
	m->SetBitmap(wxGetBitmap(icon_paint_brush_mini));
	m = m_tb_menu_edit->Append(ID_MeasureDlg, "Measurement...",
		"Make measurements with the ruler tools");
	m->SetBitmap(wxGetBitmap(icon_measurement_mini));
	m = m_tb_menu_edit->Append(ID_ComponentDlg, "Component Analyzer...",
		"Segment structures into components and perform analysis");
	m->SetBitmap(wxGetBitmap(icon_components_mini));
	m = m_tb_menu_edit->Append(ID_TrackDlg, "Tracking...",
		"Track the movements of structures");
	m->SetBitmap(wxGetBitmap(icon_tracking_mini));
	m = m_tb_menu_edit->Append(ID_CalcDlg, "Calculations...",
		"Calculate a new volume channels from existing channels");
	m->SetBitmap(wxGetBitmap(icon_calculations_mini));
	m = m_tb_menu_edit->Append(ID_NoiseCancelDlg, "Noise Reduction...",
		"Remove noise signals from a volume channel");
	m->SetBitmap(wxGetBitmap(icon_noise_reduc_mini));
	m = m_tb_menu_edit->Append(ID_CountDlg, "Volume Size...",
		"Calculate the size of a volume channel");
	m->SetBitmap(wxGetBitmap(icon_volume_size_mini));
	m = m_tb_menu_edit->Append(ID_ColocalDlg, "Colocalization...",
		"Analyze the colocalized areas among channels");
	m->SetBitmap(wxGetBitmap(icon_colocalization_mini));
	m = m_tb_menu_edit->Append(ID_ConvertDlg, "Convert...",
		"Conver a volume channel to a mesh object");
	m->SetBitmap(wxGetBitmap(icon_convert_mini));
	m = m_tb_menu_edit->Append(ID_MlDlg, "Machine Learning Manager...",
		"Manage machine-learning libraries for various functions");
	m->SetBitmap(wxGetBitmap(icon_machine_learning_mini));
	//right-side items
	m_tb_menu_update = std::make_unique<wxMenu>();
	m = m_tb_menu_update->Append(ID_CheckUpdates, "Check Updates...",
		"Check if a new version of FluoRender is available");
	m->SetBitmap(wxGetBitmap(icon_check_updates_mini));
	m = m_tb_menu_update->Append(ID_Youtube, "Video Tutorials...",
		"Watch FluoRender tutorial videos on YouTube");
	m->SetBitmap(wxGetBitmap(icon_youtube_mini));
	m = m_tb_menu_update->Append(ID_Twitter, "Twitter...",
		"Follow FluoRender on Twitter");
	m->SetBitmap(wxGetBitmap(icon_twitter_mini));
	m = m_tb_menu_update->Append(ID_Info, "About...",
		"Information about FluoRender");
	m->SetBitmap(wxGetBitmap(icon_about_mini));

	//create primary render view
	RenderViewPanel* vrv = glbin_flui_builder.BuildRenderViewPanel(this, nullptr);
	m_renderview_panels.push_back(vrv);

	//use the project panel for both tree and list
	m_proj_panel = glbin_flui_builder.BuildProjectPanel(this);
	//create list view
	m_list_panel = glbin_flui_builder.BuildListPanel(m_proj_panel);
	//create tree view
	m_tree_panel = glbin_flui_builder.BuildTreePanel(m_proj_panel);
	m_proj_panel->AddPage(m_list_panel, UITEXT_DATAVIEW, false);
	m_proj_panel->AddPage(m_tree_panel, UITEXT_TREEVIEW, true);

	//create movie view (sets the m_recorder_dlg)
	m_movie_panel = glbin_flui_builder.BuildMoviePanel(this);
	m_movie_panel->SetSize(FromDIP(wxSize(350, 450)));

	//create prop panel
	m_property_panel = glbin_flui_builder.BuildPropertyPanel(this);

	//clipping view
	m_clip_plane_panel = glbin_flui_builder.BuildClipPlanePanel(this);
	m_clip_plane_panel->SetSize(FromDIP(wxSize(140, 700)));

	//adjust view
	m_output_adj_panel = glbin_flui_builder.BuildOutputAdjPanel(this);
	m_output_adj_panel->SetSize(FromDIP(wxSize(140, 700)));

	//settings dialog
	m_setting_dlg = glbin_flui_builder.BuildSettingDlg(this);

	//brush tool dialog
	m_brush_tool_dlg = glbin_flui_builder.BuildBrushToolDlg(this);

	//noise cancelling dialog
	m_noise_cancelling_dlg = glbin_flui_builder.BuildNoiseCancellingDlg(this);

	//counting dialog
	m_counting_dlg = glbin_flui_builder.BuildCountingDlg(this);

	//convert dialog
	m_convert_dlg = glbin_flui_builder.BuildConvertDlg(this);

	//colocalization dialog
	m_colocalization_dlg = glbin_flui_builder.BuildColocalizationDlg(this);

	//measure dialog
	m_measure_dlg = glbin_flui_builder.BuildMeasureDlg(this);

	//trace dialog
	m_track_dlg = glbin_flui_builder.BuildTrackDlg(this);

	//ocl dialog
	m_ocl_dlg = glbin_flui_builder.BuildOclDlg(this);

	//component dialog
	m_component_dlg = glbin_flui_builder.BuildComponentDlg(this);

	//calculation dialog
	m_calculation_dlg = glbin_flui_builder.BuildCalculationDlg(this);

	//machine learing dialog
	m_machine_learning_dlg = glbin_flui_builder.BuildMachineLearningDlg(this);

	//script break dialog
	m_script_break_dlg = glbin_flui_builder.BuildScriptBreakDlg(this);

	//floating point volume range
	m_fp_range_dlg = glbin_flui_builder.BuildFpRangeDlg(this);
	m_fp_range_dlg->Hide();

	//help dialog
	m_help_dlg = glbin_flui_builder.BuildHelpDlg(this);

	//tester
	//shown for testing parameters
	m_teser_dlg = glbin_flui_builder.BuildTesterDlg(this);

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
	m_aui_mgr.AddPane(m_property_panel, wxAuiPaneInfo().
		Name("m_property_panel").Caption(UITEXT_PROPERTIES).
		BestSize(FromDIP(wxSize(1100, 160))).
		FloatingSize(FromDIP(wxSize(1100, 160))).
		Bottom().CloseButton(true).Layer(2));
	m_aui_mgr.AddPane(m_output_adj_panel, wxAuiPaneInfo().
		Name("m_output_adj_panel").Caption(UITEXT_ADJUST).
		BestSize(FromDIP(wxSize(150, 800))).
		FloatingSize(FromDIP(wxSize(150, 800))).
		Left().CloseButton(true).Layer(1));
	m_aui_mgr.AddPane(m_clip_plane_panel, wxAuiPaneInfo().
		Name("m_clip_plane_panel").Caption(UITEXT_CLIPPING).
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
	m_aui_mgr.AddPane(m_track_dlg, wxAuiPaneInfo().
		Name("m_track_dlg").Caption("Tracking").
		Dockable(false).CloseButton(true).
		MaximizeButton(true));
	m_aui_mgr.GetPane(m_track_dlg).Float();
	m_aui_mgr.GetPane(m_track_dlg).Hide();
	//ocl dialog
	m_aui_mgr.AddPane(m_ocl_dlg, wxAuiPaneInfo().
		Name("m_ocl_dlg").Caption("Volume Filter").
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
	//tester
	m_aui_mgr.AddPane(m_teser_dlg, wxAuiPaneInfo().
		Name("m_teser_dlg").Caption("Tests").
		Dockable(false).CloseButton(true).
		MaximizeButton(true));
	m_aui_mgr.GetPane(m_teser_dlg).Float();
	if (glbin_settings.m_test_param)
		m_aui_mgr.GetPane(m_teser_dlg).Show();
	else
		m_aui_mgr.GetPane(m_teser_dlg).Hide();

	SetMinSize(FromDIP(wxSize(800, 600)));

	if (!m_windowed)
		StartupMaximize();

	//drop target
	SetDropTarget(glbin_flui_builder.BuildDragDrop());

	m_statusbar = new wxGaugeStatusbar(this, wxID_ANY);
	SetStatusBar(m_statusbar);

	//main top menu
	m_top_menu = new wxMenuBar;
	m_top_file = new wxMenu;
	m_top_edit = new wxMenu;
	m_top_tools = new wxMenu;
	m_top_window = new wxMenu;
	m_top_help = new wxMenu;

	//file options
	m = m_top_file->Append(ID_NewProjMenu, "&New Project",
		"Clean the workspace by creating a new project");
	m->SetBitmap(wxGetBitmap(icon_new_project_mini));
	m = m_top_file->Append(ID_OpenVolumeMenu, "Open &Volume",
		"Open a volume data set and add it to the render view");
	m->SetBitmap(wxGetBitmap(icon_open_volume_mini));
	if (glbin_jvm_instance->IsValid())
	{
		m = m_top_file->Append(ID_ImportVolumeMenu, "&Import Volume",
			"Import a volume data set using ImageJ");
		m->SetBitmap(wxGetBitmap(icon_import_mini));
	}
	m = m_top_file->Append(ID_OpenMeshMenu, "Open &Mesh",
		"Open a mesh data set and add it the render view");
	m->SetBitmap(wxGetBitmap(icon_open_mesh_mini));
	m = m_top_file->Append(ID_OpenProjMenu, "Open &Project",
		"Open a previously saved project");
	m->SetBitmap(wxGetBitmap(icon_open_project_mini));
	m = m_top_file->Append(ID_SaveProjMenu, "&Save Project",
		"Save the workspace to current project file");
	m->SetBitmap(wxGetBitmap(icon_save_project_mini));
	m = m_top_file->Append(ID_SaveAsProjMenu, "Save &As Project",
		"Save the workspace in a new project file");
	m->SetBitmap(wxGetBitmap(icon_save_project_mini));
	m_top_file->AppendSeparator();
	m = m_top_file->Append(ID_Exit, "E&xit",
		"Shut down FluoRender");
	m->SetBitmap(wxGetBitmap(exit));
	//edit
	m = m_top_edit->Append(ID_UndoMenu, "&Undo",
		"Undo last operation");
	m->SetBitmap(wxGetBitmap(undo_mini));
	m = m_top_edit->Append(ID_RedoMenu, "&Redo",
		"Redo last undone operation");
	m->SetBitmap(wxGetBitmap(redo_mini));
	//tool options
	m = m_top_tools->Append(ID_OclDlgMenu, "Volume &Filter...",
		"Edit and apply volume filters using OpenCL");
	m->SetBitmap(wxGetBitmap(icon_filter_mini));
	m = m_top_tools->Append(ID_BrushDlgMenu, "&Paint Brush...",
		"Use the paint brush to select structures of interest in 3D");
	m->SetBitmap(wxGetBitmap(icon_paint_brush_mini));
	m = m_top_tools->Append(ID_MeasureDlgMenu, "&Measurement...",
		"Use the ruler tools to make measurements and analysis in 3D");
	m->SetBitmap(wxGetBitmap(icon_measurement_mini));
	m = m_top_tools->Append(ID_CompDlgMenu, "Component Anal&yzer...",
		"Segment structures into components and make analysis");
	m->SetBitmap(wxGetBitmap(icon_components_mini));
	m = m_top_tools->Append(ID_TrackDlgMenu, "&Tracking...",
		"Track the movements of segmented structures");
	m->SetBitmap(wxGetBitmap(icon_tracking_mini));
	m = m_top_tools->Append(ID_CalcDlgMenu, "Ca&lculations...",
		"Compute a new volume channel from existing channels");
	m->SetBitmap(wxGetBitmap(icon_calculations_mini));
	m = m_top_tools->Append(ID_NoiseCancelDlgMenu, "Noise &Reduction...",
		"Remove noisy signals");
	m->SetBitmap(wxGetBitmap(icon_noise_reduc_mini));
	m = m_top_tools->Append(ID_CountDlgMenu, "&Volume Size...",
		"Compute the size of a volume channel");
	m->SetBitmap(wxGetBitmap(icon_volume_size_mini));
	m = m_top_tools->Append(ID_ColocalDlgMenu, "&Colocalization...",
		"Make analysis on the overlapped regions among channels");
	m->SetBitmap(wxGetBitmap(icon_colocalization_mini));
	m = m_top_tools->Append(ID_ConvertDlgMenu, "Co&nvert...",
		"Conver a volume channel to a mesh object");
	m->SetBitmap(wxGetBitmap(icon_convert_mini));
	m = m_top_tools->Append(ID_MlDlgMenu, "M&achine Learning Manager...",
		"Manage machine-learning models and training");
	m->SetBitmap(wxGetBitmap(icon_machine_learning_mini));
	m_top_tools->Append(wxID_SEPARATOR);
	m = m_top_tools->Append(ID_Settings, "Confi&gurations...",
		"Manage settings of FluoRender");
	m->SetBitmap(wxGetBitmap(icon_settings_mini));
	//window option
	m = m_top_window->Append(ID_ToolbarMenu, "Show/Hide &Toolbar",
		"The commonly used main menu functions can be accessed from the toolbar", wxITEM_CHECK);
	m->SetBitmap(wxGetBitmap(disp_toolbar));
	m_top_window->Append(wxID_SEPARATOR);
	m = m_top_window->Append(ID_PanelsMenu, "Show/Hide &UI",
		"Show or hide all panels around the render view");
	m->SetBitmap(wxGetBitmap(icon_show_hide_ui_mini));
	m = m_top_window->Append(ID_ProjPanelMenu, "&Project",
		"Shoe or hide the project panel", wxITEM_CHECK);
	m->SetBitmap(wxGetBitmap(disp_project));
	m = m_top_window->Append(ID_MoviePanelMenu, "&Movie Making",
		"Show or hide the movie making panel", wxITEM_CHECK);
	m->SetBitmap(wxGetBitmap(disp_makemovie));
	m = m_top_window->Append(ID_OutAdjPanelMenu, "&Output Adjustments",
		"Show or hide the output adjustment panel", wxITEM_CHECK);
	m->SetBitmap(wxGetBitmap(disp_adjust));
	m = m_top_window->Append(ID_ClipPlanePanelMenu, "&Clipping Planes",
		"Show or hide the clipping plane panel", wxITEM_CHECK);
	m->SetBitmap(wxGetBitmap(disp_clipping));
	m = m_top_window->Append(ID_PropPanelMenu, "P&roperties",
		"Show or hide the property panel", wxITEM_CHECK);
	m->SetBitmap(wxGetBitmap(disp_properties));
	m_top_window->Append(wxID_SEPARATOR);
	m = m_top_window->Append(ID_LayoutMenu, "&Layout",
		"Resize the render view panels");
	m->SetBitmap(wxGetBitmap(layout));
	m = m_top_window->Append(ID_ResetMenu, "R&eset",
		"Reset the user interface layout");
	m->SetBitmap(wxGetBitmap(reset_mini));
	m = m_top_window->Append(ID_ViewNewMenu, "&New View",
		"Create a new render view panel");
	m->SetBitmap(wxGetBitmap(icon_new_view_mini));
	m = m_top_window->Append(ID_FullscreenMenu, "&Full Screen",
		"Enlarge the window to fill the screen");
	m->SetBitmap(wxGetBitmap(full_screen_menu));
	//help menu
	m = m_top_help->Append(ID_CheckUpdatesMenu, "&Check for Updates",
		"Check if there is a later version of FluoRender available for download");
	m->SetBitmap(wxGetBitmap(icon_check_updates_mini));
	m = m_top_help->Append(ID_YoutubeMenu, "&Video Tutorials",
		"Watch FluoRender tutorials on YouTube");
	m->SetBitmap(wxGetBitmap(icon_youtube_mini));
	m = m_top_help->Append(ID_TwitterMenu, "&Twitter",
		"Check information on Twitter");
	m->SetBitmap(wxGetBitmap(icon_twitter_mini));
	m = m_top_help->Append(ID_FacebookMenu, "&Facebook",
		"Check information on facebook");
	m->SetBitmap(wxGetBitmap(icon_facebook_mini));
	m = m_top_help->Append(ID_ManualMenu, "&Manual PDF",
		"Download the PDF of FluoRender operation manual");
	m->SetBitmap(wxGetBitmap(web_pdf_mini));
	m = m_top_help->Append(ID_TutorialMenu, "T&utorials PDF",
		"Download the PDF of FluoRender tutorials");
	m->SetBitmap(wxGetBitmap(web_pdf_mini));
	m = m_top_help->Append(ID_InfoMenu, "&About FluoRender...",
		"Developer team information of FluoRender");
	m->SetBitmap(wxGetBitmap(icon_about_mini));
	//add the menus
	m_top_menu->Append(m_top_file, "&File");
	m_top_menu->Append(m_top_edit, "&Edit");
	m_top_menu->Append(m_top_tools, "&Tools");
	m_top_menu->Append(m_top_window, "&Windows");
	m_top_menu->Append(m_top_help, "&Help");
	SetMenuBar(m_top_menu);

	if (m_fullscreen)
	{
		vrv->SetFullScreen();
		Iconize();
	}

	if (m_hidepanels)
	{
		auto agent = m_agent->As<MainFrameAgent>();
		if (agent)
			agent->UpdateDataToUI({ gstHideAllPanels });
	}

	//keyboard shortcuts
	wxAcceleratorEntry entries[5];
	entries[0].Set(wxACCEL_CTRL, (int)'N', ID_NewProjMenu);
	entries[1].Set(wxACCEL_CTRL, (int)'S', ID_SaveProjMenu);
	entries[2].Set(wxACCEL_CTRL, (int)'Z', ID_UndoMenu);
	entries[3].Set(wxACCEL_CTRL | wxACCEL_SHIFT, (int)'Z', ID_RedoMenu);
	entries[4].Set(wxACCEL_CTRL, (int)'O', ID_OpenVolumeMenu);
	wxAcceleratorTable accel(5, entries);
	SetAcceleratorTable(accel);

	//events
	Bind(wxEVT_MENU, &MainFrame::OnMainMenu, this);
	Bind(wxEVT_AUI_PANE_CLOSE, &MainFrame::OnPaneClose, this);
	Bind(wxEVT_CLOSE_WINDOW, &MainFrame::OnClose, this);
	Bind(wxEVT_TIMER, [](wxTimerEvent& event) { wxWakeUpIdle(); });
	Bind(wxEVT_SYS_COLOUR_CHANGED, &MainFrame::OnSysColorChanged, this);

	m_waker.Start(100);

	m_aui_mgr.Update();
}

void MainFrame::LoadPerspective(const std::string& str)
{
	m_aui_mgr.LoadPerspective(str);
	m_aui_mgr.Update();
}

std::string MainFrame::SavePerspective()
{
	return m_aui_mgr.SavePerspective().ToStdString();
}

void MainFrame::UpdateLastTool(int id)
{
	switch (id)
	{
	case ID_BrushDlg:
		m_main_tb->SetToolBitmap(ID_LastTool,
			wxGetBitmap(icon_paint_brush));
		m_main_tb->SetToolShortHelp(ID_LastTool, "Paint Brush");
		m_main_tb->SetToolLongHelp(ID_LastTool, "Paint Brush: Use brush to select voxels in the volume for further analysis");
		break;
	case ID_CalcDlg:
		m_main_tb->SetToolBitmap(ID_LastTool,
			wxGetBitmap(icon_calculations));
		m_main_tb->SetToolShortHelp(ID_LastTool, "Calculations");
		m_main_tb->SetToolLongHelp(ID_LastTool, "Calculations: Perform calculations on the volume data");
		break;
	case ID_ColocalDlg:
		m_main_tb->SetToolBitmap(ID_LastTool,
			wxGetBitmap(icon_colocalization));
		m_main_tb->SetToolShortHelp(ID_LastTool, "Colocalization");
		m_main_tb->SetToolLongHelp(ID_LastTool, "Colocalization: Analyze the colocalization of different channels in the volume");
		break;
	case ID_ComponentDlg:
		m_main_tb->SetToolBitmap(ID_LastTool,
			wxGetBitmap(icon_components));
		m_main_tb->SetToolShortHelp(ID_LastTool, "Component Analyzer");
		m_main_tb->SetToolLongHelp(ID_LastTool, "Component Analyzer: Segment structures into components and perform analysis");
		break;
	case ID_ConvertDlg:
		m_main_tb->SetToolBitmap(ID_LastTool,
			wxGetBitmap(icon_convert));
		m_main_tb->SetToolShortHelp(ID_LastTool, "Convert");
		m_main_tb->SetToolLongHelp(ID_LastTool, "Convert: Convert volume data to mesh objects");
		break;
	case ID_CountDlg:
		m_main_tb->SetToolBitmap(ID_LastTool,
			wxGetBitmap(icon_volume_size));
		m_main_tb->SetToolShortHelp(ID_LastTool, "Volume Size");
		m_main_tb->SetToolLongHelp(ID_LastTool, "Volume Size: Analyze the size of the volume data");
		break;
	case ID_MlDlg:
		m_main_tb->SetToolBitmap(ID_LastTool,
			wxGetBitmap(icon_machine_learning));
		m_main_tb->SetToolShortHelp(ID_LastTool, "Machine Learning");
		m_main_tb->SetToolLongHelp(ID_LastTool, "Machine Learning: Use machine learning algorithms to analyze the volume data");
		break;
	case ID_MeasureDlg:
		m_main_tb->SetToolBitmap(ID_LastTool,
			wxGetBitmap(icon_measurement));
		m_main_tb->SetToolShortHelp(ID_LastTool, "Measurement");
		m_main_tb->SetToolLongHelp(ID_LastTool, "Measurement: Measure distance, angle, and intensity in the volume");
		break;
	case ID_NoiseCancelDlg:
		m_main_tb->SetToolBitmap(ID_LastTool,
			wxGetBitmap(icon_noise_reduc));
		m_main_tb->SetToolShortHelp(ID_LastTool, "Noise Reduction");
		m_main_tb->SetToolLongHelp(ID_LastTool, "Noise Reduction: Reduce noise in the volume data");
		break;
	case ID_OclDlg:
		m_main_tb->SetToolBitmap(ID_LastTool,
			wxGetBitmap(tb_volume_filter));
		m_main_tb->SetToolShortHelp(ID_LastTool, "Volume Filter");
		m_main_tb->SetToolLongHelp(ID_LastTool, "Volume Filter: Edit and apply volume filters using OpenCL");
		break;
	case ID_TrackDlg:
		m_main_tb->SetToolBitmap(ID_LastTool,
			wxGetBitmap(icon_tracking));
		m_main_tb->SetToolShortHelp(ID_LastTool, "Tracking");
		m_main_tb->SetToolLongHelp(ID_LastTool, "Tracking: Track objects or features in the volume");
		break;
	}
}

void MainFrame::UpdateLastOpen(int id)
{
	switch (id)
	{
	case ID_OpenVolume:
		m_main_tb->SetToolBitmap(ID_LastOpen,
			wxGetBitmap(icon_open_volume));
		m_main_tb->SetToolLabel(ID_LastOpen, "Open Volume");
		m_main_tb->SetToolShortHelp(ID_LastOpen, "Open single or multiple volume data file(s)");
		m_main_tb->SetToolLongHelp(ID_LastOpen, "Open single or multiple volume data file(s)");
		break;
	case ID_ImportVolume:
		m_main_tb->SetToolBitmap(ID_LastOpen,
			wxGetBitmap(icon_import));
		m_main_tb->SetToolLabel(ID_LastOpen, "Import Volume");
		m_main_tb->SetToolShortHelp(ID_LastOpen, "Import single or multiple volume data file(s) using ImageJ");
		m_main_tb->SetToolLongHelp(ID_LastOpen, "Import single or multiple volume data file(s) using ImageJ");
		break;
	case ID_OpenMesh:
		m_main_tb->SetToolBitmap(ID_LastOpen,
			wxGetBitmap(icon_open_mesh));
		m_main_tb->SetToolLabel(ID_LastOpen, "Open Mesh");
		m_main_tb->SetToolShortHelp(ID_LastOpen, "Open single or multiple mesh file(s)");
		m_main_tb->SetToolLongHelp(ID_LastOpen, "Open single or multiple mesh file(s)");
		break;
	}
}

void MainFrame::UpdateProjPanel()
{
	bool bval = m_aui_mgr.GetPane(m_proj_panel).IsShown();
	m_tb_menu_ui->Check(ID_ProjPanel, bval);
	m_top_window->Check(ID_ProjPanelMenu, bval);
}

void MainFrame::UpdateMoviePanel()
{
	bool bval = m_aui_mgr.GetPane(m_movie_panel).IsShown();
	m_tb_menu_ui->Check(ID_MoviePanel, bval);
	m_top_window->Check(ID_MoviePanelMenu, bval);
}

void MainFrame::UpdatePropertyPanel()
{
	bool bval = m_aui_mgr.GetPane(m_property_panel).IsShown();
	m_tb_menu_ui->Check(ID_PropPanel, bval);
	m_top_window->Check(ID_PropPanelMenu, bval);
}

void MainFrame::UpdateOutAdjPanel()
{
	bool bval = m_aui_mgr.GetPane(m_output_adj_panel).IsShown();
	m_tb_menu_ui->Check(ID_OutAdjPanel, bval);
	m_top_window->Check(ID_OutAdjPanelMenu, bval);
}

void MainFrame::UpdateClipPlanePanel()
{
	bool bval = m_aui_mgr.GetPane(m_clip_plane_panel).IsShown();
	m_tb_menu_ui->Check(ID_ClipPlanePanel, bval);
	m_top_window->Check(ID_ClipPlanePanelMenu, bval);
}

void MainFrame::UpdateMainToolbar()
{
	bool bval = m_aui_mgr.GetPane(m_main_tb).IsShown();
	m_top_window->Check(ID_ToolbarMenu, bval);
}

void MainFrame::UpdateMainFrameTitle(const std::wstring& title)
{
	wxString str = title;
	if (str.IsEmpty())
		str = m_title;
	else
		str = m_title + " - " + str;
	SetTitle(str);
}

void MainFrame::UpdateMainStatusbarText(const std::string& text)
{
	m_statusbar->SetStatusText(text);
}

void MainFrame::PushMainStatusbarText(const std::string& text)
{
	m_statusbar->PushStatusText(text);
}

void MainFrame::PopMainStatusbarText()
{
	m_statusbar->PopStatusText();
}

void MainFrame::UpdateProgress(int val, const std::string& str)
{
	wxGetApp().Yield();
	m_statusbar->SetGaugeValue(val);
	if (str != "NOT_SET")
		m_statusbar->SetGaugeText(str);
}

void MainFrame::UpdateFullscreenMenu()
{
	wxMenuItem* m = m_top_window->FindItem(ID_FullscreenMenu);
	if (!m)
		return;

	if (IsFullScreen())
	{
		m->SetBitmap(wxGetBitmap(full_screen_back_menu));
	}
	else
	{
		m->SetBitmap(wxGetBitmap(full_screen_menu));
	}
}

RenderViewPanel* MainFrame::CreateRenderViewPanel()
{
	RenderViewPanel* panel = 0;
	if (m_renderview_panels.size()>0)
	{
		wxGLContext* sharedContext = m_renderview_panels[0]->GetContext();
		panel = glbin_flui_builder.BuildRenderViewPanel(this, sharedContext);
	}
	else
	{
		panel = glbin_flui_builder.BuildRenderViewPanel(this, nullptr);
	}

	if (!panel)
		return nullptr;

	m_renderview_panels.push_back(panel);

	return panel;
}

void MainFrame::LayoutRenderViewPanels(RenderViewLayout layout)
{
	const int paneNum =
		static_cast<int>(m_renderview_panels.size());

	if (paneNum == 0)
		return;

	// determine available area
	int minx = 0, miny = 0;
	int maxx = 0, maxy = 0;

	for (int i = 0; i < paneNum; ++i)
	{
		auto panel = m_renderview_panels[i];
		if (!panel)
			continue;

		auto& pane = m_aui_mgr.GetPane(panel);
		if (!pane.IsOk())
			continue;

		wxPoint pos = panel->GetPosition();
		wxSize size = panel->GetSize();

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
			minx = std::min(minx, x1);
			miny = std::min(miny, y1);
			maxx = std::max(maxx, x2);
			maxy = std::max(maxy, y2);
		}
	}

	const int width = std::max(100, maxx - minx);
	const int height = std::max(100, maxy - miny);

	// detach all panes
	for (auto panel : m_renderview_panels)
	{
		if (panel)
			m_aui_mgr.DetachPane(panel);
	}

	//--------------------------------------------------
	// Horizontal (1 x N)
	//--------------------------------------------------

	if (layout == RenderViewLayout::Horizontal)
	{
		const int w = width / paneNum;

		for (int i = 0; i < paneNum; ++i)
		{
			auto panel = m_renderview_panels[i];

			wxAuiPaneInfo info;
			info.Name(panel->GetName())
				.Caption(panel->GetName())
				.Dockable(true)
				.CloseButton(false)
				.Resizable()
				.BestSize(w, height);

			if (i == 0)
				info.Centre();
			else
				info.Right();

			m_aui_mgr.AddPane(panel, info);
		}
	}

	//--------------------------------------------------
	// Vertical (N x 1)
	//--------------------------------------------------

	else if (layout == RenderViewLayout::Vertical)
	{
		const int h = height / paneNum;

		for (int i = 0; i < paneNum; ++i)
		{
			auto panel = m_renderview_panels[i];

			wxAuiPaneInfo info;
			info.Name(panel->GetName())
				.Caption(panel->GetName())
				.Dockable(true)
				.CloseButton(false)
				.Resizable()
				.BestSize(width, h);

			if (i == 0)
				info.Centre();
			else
				info.Bottom();

			m_aui_mgr.AddPane(panel, info);
		}
	}

	//--------------------------------------------------
	// 3 views: 2 top, 1 bottom
	//--------------------------------------------------

	else if (layout == RenderViewLayout::ThreeTopBottom &&
		paneNum >= 3)
	{
		m_aui_mgr.AddPane(
			m_renderview_panels[0],
			wxAuiPaneInfo().
			Name(m_renderview_panels[0]->GetName()).
			Caption(m_renderview_panels[0]->GetName()).
			Centre());

		m_aui_mgr.AddPane(
			m_renderview_panels[1],
			wxAuiPaneInfo().
			Name(m_renderview_panels[1]->GetName()).
			Caption(m_renderview_panels[1]->GetName()).
			Right());

		m_aui_mgr.AddPane(
			m_renderview_panels[2],
			wxAuiPaneInfo().
			Name(m_renderview_panels[2]->GetName()).
			Caption(m_renderview_panels[2]->GetName()).
			Bottom());

		// extra panels
		for (int i = 3; i < paneNum; ++i)
		{
			m_aui_mgr.AddPane(
				m_renderview_panels[i],
				wxAuiPaneInfo().
				Name(m_renderview_panels[i]->GetName()).
				Caption(m_renderview_panels[i]->GetName()).
				Right());
		}
	}

	//--------------------------------------------------
	// 3 views: 2 left, 1 right
	//--------------------------------------------------

	else if (layout == RenderViewLayout::ThreeLeftRight &&
		paneNum >= 3)
	{
		m_aui_mgr.AddPane(
			m_renderview_panels[0],
			wxAuiPaneInfo().
			Name(m_renderview_panels[0]->GetName()).
			Caption(m_renderview_panels[0]->GetName()).
			Centre());

		m_aui_mgr.AddPane(
			m_renderview_panels[1],
			wxAuiPaneInfo().
			Name(m_renderview_panels[1]->GetName()).
			Caption(m_renderview_panels[1]->GetName()).
			Bottom());

		m_aui_mgr.AddPane(
			m_renderview_panels[2],
			wxAuiPaneInfo().
			Name(m_renderview_panels[2]->GetName()).
			Caption(m_renderview_panels[2]->GetName()).
			Right());

		for (int i = 3; i < paneNum; ++i)
		{
			m_aui_mgr.AddPane(
				m_renderview_panels[i],
				wxAuiPaneInfo().
				Name(m_renderview_panels[i]->GetName()).
				Caption(m_renderview_panels[i]->GetName()).
				Bottom());
		}
	}

	//--------------------------------------------------
	// 4 views grid
	//--------------------------------------------------

	else if (layout == RenderViewLayout::Grid2x2 &&
		paneNum >= 4)
	{
		m_aui_mgr.AddPane(
			m_renderview_panels[0],
			wxAuiPaneInfo().
			Name(m_renderview_panels[0]->GetName()).
			Caption(m_renderview_panels[0]->GetName()).
			Centre());

		m_aui_mgr.AddPane(
			m_renderview_panels[1],
			wxAuiPaneInfo().
			Name(m_renderview_panels[1]->GetName()).
			Caption(m_renderview_panels[1]->GetName()).
			Right());

		m_aui_mgr.AddPane(
			m_renderview_panels[2],
			wxAuiPaneInfo().
			Name(m_renderview_panels[2]->GetName()).
			Caption(m_renderview_panels[2]->GetName()).
			Bottom());

		m_aui_mgr.AddPane(
			m_renderview_panels[3],
			wxAuiPaneInfo().
			Name(m_renderview_panels[3]->GetName()).
			Caption(m_renderview_panels[3]->GetName()).
			Bottom());

		for (int i = 4; i < paneNum; ++i)
		{
			m_aui_mgr.AddPane(
				m_renderview_panels[i],
				wxAuiPaneInfo().
				Name(m_renderview_panels[i]->GetName()).
				Caption(m_renderview_panels[i]->GetName()).
				Right());
		}
	}

	//--------------------------------------------------
	// fallback
	//--------------------------------------------------

	else
	{
		LayoutRenderViewPanels(RenderViewLayout::Horizontal);
		return;
	}

	m_aui_mgr.Update();

	// release size locks
	for (auto panel : m_renderview_panels)
	{
		if (!panel)
			continue;

		auto& pane = m_aui_mgr.GetPane(panel);

		pane.MinSize(-1, -1);
		pane.MaxSize(-1, -1);
	}

	m_aui_mgr.Update();
}

int MainFrame::DeleteRenderViewPanel(RenderViewPanel* panel)
{
	int i = -1;
	for (int pi = 0; pi < (int)m_renderview_panels.size(); ++i)
	{
		if (m_renderview_panels[pi] == panel)
		{
			i = pi;
			break;
		}
	}
	if (i > 0)
	{
		m_renderview_panels.erase(m_renderview_panels.begin() + i);
		m_aui_mgr.DetachPane(panel);
		panel->Close();
		panel->Destroy();
		m_aui_mgr.Update();
		return i;
	}
	return static_cast<int>(m_renderview_panels.size());
}


int MainFrame::GetDisplayId()
{
	return wxDisplay::GetFromWindow(this);
}

std::array<int, 4> MainFrame::GetMainFrameRect()
{
#ifdef __WXMSW__
	HWND hwnd = (HWND)GetHandle();

	WINDOWPLACEMENT wp;
	wp.length = sizeof(WINDOWPLACEMENT);

	if (GetWindowPlacement(hwnd, &wp))
	{
		RECT r = wp.rcNormalPosition;

		return { r.left, r.top,
			r.right - r.left,
			r.bottom - r.top };
	}
#endif
	// fallback (non-Windows)
	wxRect r = GetScreenRect();

	return { r.x, r.y,
		r.width, r.height };
}

double MainFrame::GetMainMemorySizeInMB()
{
	wxMemorySize free_mem_size = wxGetFreeMemory();
	return free_mem_size.ToDouble() * 0.8 / 1024.0 / 1024.0;
}

bool MainFrame::GetToolbarShown()
{
	return m_aui_mgr.GetPane(m_main_tb).IsShown();
}

bool MainFrame::GetProjPanelShown()
{
	return m_aui_mgr.GetPane(m_proj_panel).IsShown();
}

bool MainFrame::GetMoviePanelShown()
{
	return m_aui_mgr.GetPane(m_movie_panel).IsShown();
}

bool MainFrame::GetOutAdjPanelShown()
{
	return m_aui_mgr.GetPane(m_output_adj_panel).IsShown();
}

bool MainFrame::GetClipPlanePanelShown()
{
	return m_aui_mgr.GetPane(m_clip_plane_panel).IsShown();
}

bool MainFrame::GetPropertyPanelShown()
{
	return m_aui_mgr.GetPane(m_property_panel).IsShown();
}

void MainFrame::StartupMaximize()
{
	const auto& s = glbin_settings;

	int chosenDisplay = wxNOT_FOUND;
	bool chosenLandscape = true;

	// ----------------------------
	// 1. Try using saved rect first
	// ----------------------------
	if (s.m_mainframe_w > 0 && s.m_mainframe_h > 0)
	{
		wxPoint pt(s.m_mainframe_x, s.m_mainframe_y);

		int disp = wxDisplay::GetFromPoint(pt);
		if (disp != wxNOT_FOUND)
		{
			chosenDisplay = disp;

			// Restore position/size first
			SetSize(s.m_mainframe_x,
				s.m_mainframe_y,
				s.m_mainframe_w,
				s.m_mainframe_h);
		}
		if (chosenDisplay != wxNOT_FOUND)
		{
			wxDisplay d(chosenDisplay);
			wxRect r = d.GetClientArea();
			chosenDisplay = wxDisplay::GetFromRect(r);
			chosenLandscape = r.width >= r.height;
		}
	}

	// -----------------------------------
	// 2. Fallback to saved display ID
	// -----------------------------------
	if (chosenDisplay == wxNOT_FOUND ||
		!chosenLandscape)
	{
		if (s.m_mainframe_disp_id >= 0 &&
			s.m_mainframe_disp_id < wxDisplay::GetCount())
		{
			chosenDisplay = s.m_mainframe_disp_id;
		}
		if (chosenDisplay != wxNOT_FOUND)
		{
			wxDisplay d(chosenDisplay);
			wxRect r = d.GetClientArea();
			chosenDisplay = wxDisplay::GetFromRect(r);
			chosenLandscape = r.width >= r.height;
		}
	}

	// -----------------------------------
	// 3. Intelligent fallback selection
	// -----------------------------------
	if (chosenDisplay == wxNOT_FOUND ||
		!chosenLandscape)
	{
		int bestDisplay = wxNOT_FOUND;
		int bestX = INT_MAX;

		for (int i = 0; i < wxDisplay::GetCount(); ++i)
		{
			wxDisplay d(i);
			wxRect r = d.GetClientArea();

			// Prefer landscape first
			if (r.width < r.height)
				continue;

			// Find left-most (smallest x)
			if (r.x < bestX)
			{
				bestX = r.x;
				bestDisplay = i;
			}
		}

		// If no landscape found, just pick left-most
		if (bestDisplay == wxNOT_FOUND)
		{
			for (int i = 0; i < wxDisplay::GetCount(); ++i)
			{
				wxDisplay d(i);
				wxRect r = d.GetClientArea();

				if (r.x < bestX)
				{
					bestX = r.x;
					bestDisplay = i;
				}
			}
		}

		chosenDisplay = (bestDisplay != wxNOT_FOUND) ? bestDisplay : 0;
	}

	// -----------------------------------
	// 4. Apply display placement if needed
	// -----------------------------------
	if (chosenDisplay != wxNOT_FOUND)
	{
		wxDisplay d(chosenDisplay);
		wxRect rect = d.GetClientArea();

		SetPosition(rect.GetTopLeft());
		SetSize(rect.GetSize());
	}

	// -----------------------------------
	// 5. Maximize last (important)
	// -----------------------------------
	Maximize();
}

void MainFrame::ToggleScriptBreakDlg()
{
	bool show = !m_script_break_dlg->IsShown();
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

void MainFrame::ToggleAllPanels(int mode)
{
	bool bval = true;
	if (mode == 2)
	{
		if (m_aui_mgr.GetPane(m_proj_panel).IsShown() &&
			m_aui_mgr.GetPane(m_movie_panel).IsShown() &&
			m_aui_mgr.GetPane(m_property_panel).IsShown() &&
			m_aui_mgr.GetPane(m_output_adj_panel).IsShown() &&
			m_aui_mgr.GetPane(m_clip_plane_panel).IsShown())
			bval = true;
		else if (!m_aui_mgr.GetPane(m_proj_panel).IsShown() &&
			!m_aui_mgr.GetPane(m_movie_panel).IsShown() &&
			!m_aui_mgr.GetPane(m_property_panel).IsShown() &&
			!m_aui_mgr.GetPane(m_output_adj_panel).IsShown() &&
			!m_aui_mgr.GetPane(m_clip_plane_panel).IsShown())
			bval = false;
		bval = !bval;
	}
	else if (mode == 0)
		bval = false;
	else if (mode == 1)
		bval = true;

	//data view
	m_aui_mgr.GetPane(m_proj_panel).Show(bval);
	//movie view (float only)
	m_aui_mgr.GetPane(m_movie_panel).Show(bval);
	//properties
	m_aui_mgr.GetPane(m_property_panel).Show(bval);
	//adjust view
	m_aui_mgr.GetPane(m_output_adj_panel).Show(bval);
	//clipping view
	m_aui_mgr.GetPane(m_clip_plane_panel).Show(bval);

	m_aui_mgr.Update();

	auto agent = m_agent->As<MainFrameAgent>();
	agent->NotifyDataToUI({ gstProjPanel, gstMoviePanel, gstPropertyPanel, gstOutAdjPanel, gstClipPlanePanel });
}

void MainFrame::ShowToolbar(bool bval)
{
	m_aui_mgr.GetPane(m_main_tb).Show(bval);
	m_aui_mgr.Update();
}

void MainFrame::ShowProjPanel(bool bval)
{
	m_aui_mgr.GetPane(m_proj_panel).Show(bval);
	m_aui_mgr.Update();
}

void MainFrame::ShowMoviePanel(bool bval)
{
	m_aui_mgr.GetPane(m_movie_panel).Show(bval);
	m_aui_mgr.Update();
}

void MainFrame::ShowOutAdjPanel(bool bval)
{
	m_aui_mgr.GetPane(m_output_adj_panel).Show(bval);
	m_aui_mgr.Update();
}

void MainFrame::ShowClipPlanePanel(bool bval)
{
	m_aui_mgr.GetPane(m_clip_plane_panel).Show(bval);
	m_aui_mgr.Update();
}

void MainFrame::ShowPropertyPanel(bool bval)
{
	m_aui_mgr.GetPane(m_property_panel).Show(bval);
	m_aui_mgr.Update();
}

void MainFrame::ShowBrushDlg()
{
	m_aui_mgr.GetPane(m_brush_tool_dlg).Show();
	m_aui_mgr.Update();
}

void MainFrame::ShowCalculationDlg()
{
	m_aui_mgr.GetPane(m_calculation_dlg).Show();
	m_aui_mgr.Update();
}

void MainFrame::ShowColocalizationDlg()
{
	m_aui_mgr.GetPane(m_colocalization_dlg).Show();
	m_aui_mgr.Update();
}

void MainFrame::ShowComponentDlg()
{
	m_aui_mgr.GetPane(m_component_dlg).Show();
	m_aui_mgr.Update();
}

void MainFrame::ShowConvertDlg()
{
	m_aui_mgr.GetPane(m_convert_dlg).Show();
	m_aui_mgr.Update();
}

void MainFrame::ShowCountingDlg()
{
	m_aui_mgr.GetPane(m_counting_dlg).Show();
	m_aui_mgr.Update();
}

void MainFrame::ShowMachineLearningDlg()
{
	m_aui_mgr.GetPane(m_machine_learning_dlg).Show();
	m_aui_mgr.Update();
}

void MainFrame::ShowMeasureDlg()
{
	m_aui_mgr.GetPane(m_measure_dlg).Show();
	m_aui_mgr.Update();
}

void MainFrame::ShowNoiseCancellingDlg()
{
	m_aui_mgr.GetPane(m_noise_cancelling_dlg).Show();
	m_aui_mgr.Update();
}

void MainFrame::ShowOclDlg()
{
	m_aui_mgr.GetPane(m_ocl_dlg).Show();
	m_aui_mgr.Update();
}

void MainFrame::ShowSettingDlg()
{
	m_aui_mgr.GetPane(m_setting_dlg).Show();
	m_aui_mgr.Update();
}

void MainFrame::ShowTrackDlg()
{
	m_aui_mgr.GetPane(m_track_dlg).Show();
	m_aui_mgr.Update();
}

void MainFrame::ShowAboutDlg()
{
	AboutDlg dlg(this);
	dlg.ShowModal();
}

void MainFrame::LayoutOutputAdjPanel()
{
	m_output_adj_panel->LoadPerspective("");
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
	m_aui_mgr.GetPane(m_property_panel).Show().Dock().
		BestSize(FromDIP(wxSize(1100, 160))).
		FloatingSize(FromDIP(wxSize(1100, 160))).
		Bottom().Layer(2);
	m_aui_mgr.GetPane(m_output_adj_panel).Show().Dock().
		BestSize(FromDIP(wxSize(150, 800))).
		FloatingSize(FromDIP(wxSize(150, 800))).
		Left().Layer(1);
	m_aui_mgr.GetPane(m_clip_plane_panel).Show().Dock().
		BestSize(FromDIP(wxSize(150, 800))).
		FloatingSize(FromDIP(wxSize(150, 800))).
		Right().Show().Dock().Layer(1);
}

void MainFrame::QuitCanvasFullscreen()
{
#ifdef _WIN32
	if (glbin_settings.m_show_cursor)
		ShowCursor(true);
#endif
	Iconize(false);
	SetFocus();
	Raise();
	Show();
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
		case ID_LastOpen:
			menu = m_tb_menu_open.get();
			break;
		case ID_Panels:
			menu = m_tb_menu_ui.get();
			break;
		case ID_LastTool:
			menu = m_tb_menu_edit.get();
			break;
		case ID_CheckUpdates:
		case ID_Youtube:
		case ID_Twitter:
		case ID_Info:
			menu = m_tb_menu_update.get();
			break;
		}
		if (menu)
			PopupMenu(menu, pt);

		// make sure the button is "un-stuck"
		tb->SetToolSticky(event.GetId(), false);

	}
	else
		event.Skip();
}

//toolbar
void MainFrame::OnMainMenu(wxCommandEvent& event)
{
	fluo::ValueCollection vc_data;
	fluo::ValueCollection vc_ui;
	int id = event.GetId();

	switch (id)
	{
		//toolbar
	case ID_LastOpen:
		vc_data.insert(gstLastOpen);
		break;
	case ID_OpenVolume:
		vc_data.insert(gstOpenVolume);
		break;
	case ID_ImportVolume:
		vc_data.insert(gstImportVolume);
		break;
	case ID_OpenMesh:
		vc_data.insert(gstOpenMesh);
		break;
	case ID_OpenProject:
		vc_data.insert(gstOpenProject);
		break;
	case ID_SaveProject:
		vc_data.insert(gstSaveProject);
		break;
	case ID_ViewNew:
		vc_ui.insert(gstCreateRenderViewPanel);
		break;
	case ID_Panels:
		vc_ui.insert(gstToggleAllPanels);
		break;
	case ID_LastTool:
		vc_ui.insert(gstLastTool);
		break;
	case ID_Undo:
		glbin.undo();
		break;
	case ID_Redo:
		glbin.redo();
		break;
	case ID_Settings:
		vc_ui.insert(gstSettingDlg);
		break;
	case ID_CheckUpdates:
		::wxLaunchDefaultBrowser(VERSION_UPDATES);
		break;
	case ID_Youtube:
		::wxLaunchDefaultBrowser(YOUTUBE_URL);
		break;
	case ID_Twitter:
		::wxLaunchDefaultBrowser(TWITTER_URL);
		break;
	case ID_Info:
		vc_ui.insert(gstAboutDlg);
		break;
		//toolbar menu items
	case ID_ProjPanel:
		vc_ui.insert(gstToggleProjPanel);
		break;
	case ID_MoviePanel:
		vc_ui.insert(gstToggleMoviePanel);
		break;
	case ID_OutAdjPanel:
		vc_ui.insert(gstToggleOutAdjPanel);
		break;
	case ID_ClipPlanePanel:
		vc_ui.insert(gstToggleClipPlanePanel);
		break;
	case ID_PropPanel:
		vc_ui.insert(gstTogglePropertyPanel);
		break;
	case ID_BrushDlg:
		vc_ui.insert(gstBrushToolDlg);
		break;
	case ID_MeasureDlg:
		vc_ui.insert(gstMeasureDlg);
		break;
	case ID_ComponentDlg:
		vc_ui.insert(gstComponentDlg);
		break;
	case ID_TrackDlg:
		vc_ui.insert(gstTrackDlg);
		break;
	case ID_CalcDlg:
		vc_ui.insert(gstCalculationDlg);
		break;
	case ID_NoiseCancelDlg:
		vc_ui.insert(gstNoiseCancellingDlg);
		break;
	case ID_CountDlg:
		vc_ui.insert(gstCountingDlg);
		break;
	case ID_ColocalDlg:
		vc_ui.insert(gstColocalizationDlg);
		break;
	case ID_ConvertDlg:
		vc_ui.insert(gstConvertDlg);
		break;
	case ID_OclDlg:
		vc_ui.insert(gstOclDlg);
		break;
	case ID_MlDlg:
		vc_ui.insert(gstMachineLearningDlg);
		break;
		//main menu items
		//file
	case ID_NewProjMenu:
		vc_data.insert(gstNewProject);
		break;
	case ID_OpenVolumeMenu:
		vc_data.insert(gstOpenVolume);
		break;
	case ID_ImportVolumeMenu:
		vc_data.insert(gstImportVolume);
		break;
	case ID_OpenMeshMenu:
		vc_data.insert(gstOpenMesh);
		break;
	case ID_OpenProjMenu:
		vc_data.insert(gstOpenProject);
		break;
	case ID_SaveProjMenu:
		vc_data.insert(gstSaveProject);
		break;
	case ID_SaveAsProjMenu:
		vc_data.insert(gstSaveAsProject);
		break;
	case ID_Exit:
		Close(true);
		break;
		//edit
	case ID_UndoMenu:
		glbin.undo();
		break;
	case ID_RedoMenu:
		glbin.redo();
		break;
		//tools
	case ID_BrushDlgMenu:
		vc_ui.insert(gstBrushToolDlg);
		break;
	case ID_MeasureDlgMenu:
		vc_ui.insert(gstMeasureDlg);
		break;
	case ID_CompDlgMenu:
		vc_ui.insert(gstComponentDlg);
		break;
	case ID_TrackDlgMenu:
		vc_ui.insert(gstTrackDlg);
		break;
	case ID_CalcDlgMenu:
		vc_ui.insert(gstCalculationDlg);
		break;
	case ID_NoiseCancelDlgMenu:
		vc_ui.insert(gstNoiseCancellingDlg);
		break;
	case ID_CountDlgMenu:
		vc_ui.insert(gstCountingDlg);
		break;
	case ID_ColocalDlgMenu:
		vc_ui.insert(gstColocalizationDlg);
		break;
	case ID_ConvertDlgMenu:
		vc_ui.insert(gstConvertDlg);
		break;
	case ID_OclDlgMenu:
		vc_ui.insert(gstOclDlg);
		break;
	case ID_MlDlgMenu:
		vc_ui.insert(gstMachineLearningDlg);
		break;
		//window
	case ID_ToolbarMenu:
		vc_ui.insert(gstToggleToolbar);
		break;
	case ID_PanelsMenu:
		vc_ui.insert(gstToggleAllPanels);
		break;
	case ID_ProjPanelMenu:
		vc_ui.insert(gstToggleProjPanel);
		break;
	case ID_MoviePanelMenu:
		vc_ui.insert(gstToggleMoviePanel);
		break;
	case ID_OutAdjPanelMenu:
		vc_ui.insert(gstToggleOutAdjPanel);
		break;
	case ID_ClipPlanePanelMenu:
		vc_ui.insert(gstToggleClipPlanePanel);
		break;
	case ID_PropPanelMenu:
		vc_ui.insert(gstTogglePropertyPanel);
		break;
	case ID_LayoutMenu:
		vc_ui.insert({
			gstRenderViewPanelLayout,
			gstOutAdjPanelLayout });
		break;
	case ID_ResetMenu:
		vc_ui.insert(gstResetLayout);
		break;
	case ID_ViewNewMenu:
		vc_ui.insert(gstCreateRenderViewPanel);
		break;
	case ID_FullscreenMenu:
		vc_ui.insert(gstFullscreenFrame);
		break;
		//help
	case ID_CheckUpdatesMenu:
		::wxLaunchDefaultBrowser(VERSION_UPDATES);
		break;
	case ID_YoutubeMenu:
		::wxLaunchDefaultBrowser(YOUTUBE_URL);
		break;
	case ID_TwitterMenu:
		::wxLaunchDefaultBrowser(TWITTER_URL);
		break;
	case ID_FacebookMenu:
		::wxLaunchDefaultBrowser(FACEBOOK_URL);
		break;
	case ID_ManualMenu:
		::wxLaunchDefaultBrowser(HELP_MANUAL);
		break;
	case ID_TutorialMenu:
		::wxLaunchDefaultBrowser(HELP_TUTORIAL);
		break;
	case ID_InfoMenu:
		vc_ui.insert(gstAboutDlg);
		break;
	}

	auto agent = m_agent->As<MainFrameAgent>();
	if (agent)
	{
		if (!vc_data.empty())
			agent->UpdateUIToData(vc_data);
		if (!vc_ui.empty())
			agent->UpdateDataToUI(vc_ui);
	}
	//event.StopPropagation();
}

//panes
void MainFrame::OnPaneClose(wxAuiManagerEvent& event)
{
	auto agent = m_agent->As<MainFrameAgent>();
	if (agent)
		agent->UpdateDataToUI({
		gstProjPanel,
		gstMoviePanel,
		gstPropertyPanel,
		gstOutAdjPanel,
		gstClipPlanePanel });
}

void MainFrame::OnClose(wxCloseEvent& event)
{
	auto agent = m_agent->As<MainFrameAgent>();
	if (agent)
		agent->ShutdownAll();
	event.Skip();
}

void MainFrame::OnSysColorChanged(wxSysColourChangedEvent& event)
{
	auto appearance = wxSystemSettings::GetAppearance();
	wxApp::GetGUIInstance()->SetAppearance(wxApp::Appearance::System);

	// Optionally, refresh UI elements
	Refresh();
	event.Skip(); // Allow default processing
}

void OpenVolumeHook::AddCustomControls(
	wxFileDialogCustomize& customizer)
{
	// Read file# as Z sections
	m_slice_chk =
		customizer.AddCheckBox(
			"Read file# as Z sections");

	m_slice_chk->SetValue(
		m_options.sliceSequence);

	// Read file# as channels
	m_channel_chk =
		customizer.AddCheckBox(
			"Read file# as channels");

	m_channel_chk->SetValue(
		m_options.channelSequence);

	// Digit order
	static const wxString kDigitOrderChoices[] =
	{
		"Channel first",
		"Z section first"
	};

	m_digit_order_choice =
		customizer.AddChoice(
			WXSIZEOF(kDigitOrderChoices),
			kDigitOrderChoices);

	m_digit_order_choice->SetSelection(
		m_options.digitOrder);

	// Serial number
	m_serial_txt =
		customizer.AddTextCtrl(
			"Serial:");

	m_serial_txt->SetValue(
		m_options.serialNumber);

	// Compression
	m_compress_chk =
		customizer.AddCheckBox(
			"Compress data");

	m_compress_chk->SetValue(
		m_options.compressData);

	// Skip bricks
	m_skip_bricks_chk =
		customizer.AddCheckBox(
			"Skip empty bricks during rendering");

	m_skip_bricks_chk->SetValue(
		m_options.skipEmptyBricks);

	// Time sequence identifier
	m_time_id_txt =
		customizer.AddTextCtrl(
			"Time sequence identifier:");

	m_time_id_txt->SetValue(
		m_options.timeSequenceIdentifier);
}

void OpenVolumeHook::TransferDataFromCustomControls()
{
	if (m_slice_chk)
		m_options.sliceSequence =
		m_slice_chk->GetValue();

	if (m_channel_chk)
		m_options.channelSequence =
		m_channel_chk->GetValue();

	if (m_digit_order_choice)
		m_options.digitOrder =
		m_digit_order_choice->GetSelection();

	if (m_serial_txt)
		m_options.serialNumber =
		m_serial_txt->GetValue().ToStdString();

	if (m_compress_chk)
		m_options.compressData =
		m_compress_chk->GetValue();

	if (m_skip_bricks_chk)
		m_options.skipEmptyBricks =
		m_skip_bricks_chk->GetValue();

	if (m_time_id_txt)
		m_options.timeSequenceIdentifier =
		m_time_id_txt->GetValue().ToStdWstring();
}

void ImportVolumeHook::AddCustomControls(
	wxFileDialogCustomize& customizer)
{
	m_compress_chk =
		customizer.AddCheckBox(
			"Compress data (loading will take longer time and data are compressed in graphics memory)");

	m_compress_chk->SetValue(
		m_options.compressData);

	m_skip_bricks_chk =
		customizer.AddCheckBox(
			"Skip empty bricks during rendering (loading takes longer time)");

	m_skip_bricks_chk->SetValue(
		m_options.skipEmptyBricks);
}

void ImportVolumeHook::TransferDataFromCustomControls()
{
	if (m_compress_chk)
	{
		m_options.compressData =
			m_compress_chk->GetValue();
	}

	if (m_skip_bricks_chk)
	{
		m_options.skipEmptyBricks =
			m_skip_bricks_chk->GetValue();
	}
}

void SaveProjectHook::AddCustomControls(
	wxFileDialogCustomize& customizer)
{
	m_embed_chk =
		customizer.AddCheckBox(
			"Embed all files in the project folder");

	m_embed_chk->SetValue(
		m_options.embedFiles);

	m_compress_chk =
		customizer.AddCheckBox(
			"Lempel-Ziv-Welch Compression");

	m_compress_chk->SetValue(
		m_options.compressProject);
}

void SaveProjectHook::TransferDataFromCustomControls()
{
	if (m_embed_chk)
	{
		m_options.embedFiles =
			m_embed_chk->GetValue();
	}

	if (m_compress_chk)
	{
		m_options.compressProject =
			m_compress_chk->GetValue();
	}
}

