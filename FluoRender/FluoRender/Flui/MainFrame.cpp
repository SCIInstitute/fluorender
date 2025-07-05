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
#include <KernelProgram.h>
#include <Main.h>
#include <MainFrame.h>
#include <Global.h>
#include <Names.h>
#include <GlobalStates.h>
#include <MainSettings.h>
#include <compatibility.h>
#include <DragDrop.h>
#include <PropPanel.h>
#include <ProjectPanel.h>
#include <TreePanel.h>
#include <ListPanel.h>
#include <RenderViewPanel.h>
#include <RenderCanvas.h>
#include <PropertyPanel.h>
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
#include <TrackDlg.h>
#include <OclDlg.h>
#include <ComponentDlg.h>
#include <CalculationDlg.h>
#include <MachineLearningDlg.h>
#include <ScriptBreakDlg.h>
#include <FpRangeDlg.h>
#include <AsyncTimerFactory.hpp>
#include <Tester.h>
#include <TextureRenderer.h>
#include <msk_writer.h>
#include <msk_reader.h>
#include <VolumeMeshConv.h>
#include <DataManager.h>
#include <RenderView.h>
#include <ScriptProc.h>
#include <Project.h>
#include <VolumeCalculator.h>
#include <CompGenerator.h>
#include <CompAnalyzer.h>
#include <Clusterizer.h>
#include <TrackMap.h>
#include <KernelExecutor.h>
#include <JVMInitializer.h>
#include <PyBase.h>
#include <MovieMaker.h>
#include <VolKernel.h>
#include <Framebuffer.h>
#include <VertexArray.h>
#include <VolShader.h>
#include <SegShader.h>
#include <VolCalShader.h>
#include <ImgShader.h>
#include <LightFieldShader.h>
#include <TextRenderer.h>
#include <Debug.h>
#include <wxGaugeStatusbar.h>
#include <wxToolbarArt.h>
#include <ModalDlg.h>
#include <wx/aboutdlg.h>
#include <wx/hyperlink.h>
#include <wx/accel.h>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <cctype>
//resources
#include <icons.h>
#include <png_resource.h>

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
	//m_vd_copy(0),
	//m_copy_data(false),
	m_waker(this)
{
#ifdef _DARWIN
	SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#elifdef __linux__
	SetWindowVariant(wxWINDOW_VARIANT_MINI);
#endif
	//create this first to read the settings
	glbin_settings.Read();
	glbin.apply_processor_settings();
	glbin_comp_def.Apply(&glbin_clusterizer);
	glbin_comp_def.Apply(&glbin_comp_analyzer);
	glbin_comp_def.Apply(&glbin_comp_generator);
	glbin_comp_def.Apply(&glbin_comp_selector);
	glbin_brush_def.Apply(&glbin_vol_selector);
	glbin_data_manager.UpdateStreamMode(-1.0);

	//set frame
	glbin_script_proc.SetFrame(this);
	glbin_data_manager.SetFrame(this);
	glbin_current.mainframe = this;
	//progress
	//these global objects are constructed before mainframe. therefore need to set progress functions again
	glbin.InitProgress(std::bind(&MainFrame::SetProgress, this,
			std::placeholders::_1, std::placeholders::_2));

	glbin_states.m_benchmark = benchmark;

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
	m_main_tb->SetArtProvider(new wxToolbarArt());

	//add tools
	bitmap = wxGetBitmap(icon_open_volume);
	m_main_tb->AddTool(ID_OpenVolume, "Open Volume", bitmap,
		"Open single or multiple volume data file(s)");
	if (glbin_jvm_instance->IsValid())
	{
		bitmap = wxGetBitmap(icon_import);
		m_main_tb->AddTool(ID_ImportVolume, "Import Volume", bitmap,
			"Import single or multiple volume data file(s) using ImageJ");
		m_main_tb->AddSeparator();
	}
	bitmap = wxGetBitmap(icon_open_project);
	m_main_tb->AddTool(ID_OpenProject, "Open Project", bitmap,
		"Open a saved project");
	bitmap = wxGetBitmap(icon_save_project);
	m_main_tb->AddTool(ID_SaveProject, "Save Project", bitmap,
		"Save current work as a project");
	m_main_tb->AddSeparator();
	bitmap = wxGetBitmap(icon_new_view);
	m_main_tb->AddTool(ID_ViewNew, "New View", bitmap,
		"Create a new render viewport");
	bitmap = wxGetBitmap(icon_show_hide_ui);
	m_main_tb->AddTool(ID_Panels, "Show/Hide UI", bitmap,
		"Show or hide all control panels");
	m_main_tb->SetToolDropDown(ID_Panels, true);
	m_main_tb->AddSeparator();
	bitmap = wxGetBitmap(icon_open_mesh);
	m_main_tb->AddTool(ID_OpenMesh, "Open Mesh", bitmap,
		"Open single or multiple mesh file(s)");
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
		break;
	case TOOL_MEASUREMENT:
		m_main_tb->SetToolBitmap(ID_LastTool,
			wxGetBitmap(icon_measurement));
		break;
	case TOOL_TRACKING:
		m_main_tb->SetToolBitmap(ID_LastTool,
			wxGetBitmap(icon_tracking));
		break;
	case TOOL_NOISE_REDUCTION:
		m_main_tb->SetToolBitmap(ID_LastTool,
			wxGetBitmap(icon_noise_reduc));
		break;
	case TOOL_VOLUME_SIZE:
		m_main_tb->SetToolBitmap(ID_LastTool,
			wxGetBitmap(icon_volume_size));
		break;
	case TOOL_COLOCALIZATION:
		m_main_tb->SetToolBitmap(ID_LastTool,
			wxGetBitmap(icon_colocalization));
		break;
	case TOOL_CONVERT:
		m_main_tb->SetToolBitmap(ID_LastTool,
			wxGetBitmap(icon_convert));
		break;
	case TOOL_OPENCL:
		m_main_tb->SetToolBitmap(ID_LastTool,
			wxGetBitmap(tb_volume_filter));
		break;
	case TOOL_COMPONENT:
		m_main_tb->SetToolBitmap(ID_LastTool,
			wxGetBitmap(icon_components));
		break;
	case TOOL_CALCULATIONS:
		m_main_tb->SetToolBitmap(ID_LastTool,
			wxGetBitmap(icon_calculations));
		break;
	case TOOL_MACHINE_LEARNING:
		m_main_tb->SetToolBitmap(ID_LastTool,
			wxGetBitmap(icon_machine_learning));
		break;
	}
	m_main_tb->AddSeparator();
	bitmap = wxGetBitmap(icon_undo);
	m_main_tb->AddTool(ID_Undo, "Undo", bitmap,
		"Undo");
	bitmap = wxGetBitmap(icon_redo);
	m_main_tb->AddTool(ID_Redo, "Redo", bitmap,
		"Redo");
	//right-side items
	m_main_tb->AddStretchSpacer();
	bitmap = wxGetBitmap(icon_settings);
	m_main_tb->AddTool(ID_Settings, "Configurations", bitmap,
		"Settings of FluoRender");
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
	m_main_tb->AddTool(item_id, str1, bitmap, str3);
	m_main_tb->SetToolDropDown(item_id, true);
	m_main_tb->SetCustomOverflowItems(prepend_items, append_items);
	m_main_tb->Bind(wxEVT_AUITOOLBAR_TOOL_DROPDOWN, &MainFrame::OnToolbarMenu, this);
	m_main_tb->Realize();

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

	//create render view
	RenderViewPanel *vrv = new RenderViewPanel(this);
	Root* root = glbin_data_manager.GetRoot();
	std::shared_ptr<RenderView> view;
	if (root)
	{
		view = root->GetLastView();
		if (view)
		{
			view->Init();
			view->InitView();
		}
	}
	m_render_view_panels.push_back(vrv);

	wxSize panel_size(FromDIP(wxSize(350, 450)));
	//use the project panel for both tree and list
	m_proj_panel = new ProjectPanel(this);
	//m_proj_panel->SetName("ProjectPanel");
	//create list view
	m_list_panel = new ListPanel(this, m_proj_panel);
	//create tree view
	m_tree_panel = new TreePanel(this, m_proj_panel);
	m_proj_panel->AddPage(m_list_panel, UITEXT_DATAVIEW, false);
	m_proj_panel->AddPage(m_tree_panel, UITEXT_TREEVIEW, true);

	//create movie view (sets the m_recorder_dlg)
	m_movie_panel = new MoviePanel(this,
		wxDefaultPosition, panel_size);

	//create prop panel
	m_prop_panel = new PropertyPanel(this);
	//m_prop_panel->SetName("PropPanel");

	//clipping view
	m_clip_plane_panel = new ClipPlanePanel(this,
		wxDefaultPosition, FromDIP(wxSize(140,700)));

	//adjust view
	m_output_adj_panel = new OutputAdjPanel(this,
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
	m_track_dlg = new TrackDlg(this);

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

	//floating point volume range
	m_fp_range_dlg = new FpRangeDlg(this);
	m_fp_range_dlg->Hide();

	//help dialog
	m_help_dlg = new HelpDlg(this);
	//m_help_dlg->LoadPage("C:\\!wanyong!\\TEMP\\wxHtmlWindow.htm");

	//tester
	//shown for testing parameters
	m_teser_dlg = new TesterDlg(this);

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

	for (auto it : m_render_view_panels)
		it->LoadSettings();

	SetMinSize(FromDIP(wxSize(800,600)));

	if (!windowed)
		Maximize();

	//drop target
	SetDropTarget(new DnDFile(this));

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
	m = m_top_window->Append(ID_PropPanelMenu,"P&roperties",
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
	m_top_menu->Append(m_top_file,"&File");
	m_top_menu->Append(m_top_edit, "&Edit");
	m_top_menu->Append(m_top_tools,"&Tools");
	m_top_menu->Append(m_top_window,"&Windows");
	m_top_menu->Append(m_top_help,"&Help");
	SetMenuBar(m_top_menu);

	if (fullscreen)
	{
		vrv->SetFullScreen();
		Iconize();
	}

	if (hidepanels)
		ToggleAllPanels(false);

	wxMemorySize free_mem_size = wxGetFreeMemory();
	double mainmem_buf_size = free_mem_size.ToDouble() * 0.8 / 1024.0 / 1024.0;
	if (mainmem_buf_size > flvr::TextureRenderer::get_mainmem_buf_size())
		flvr::TextureRenderer::set_mainmem_buf_size(mainmem_buf_size);

	//python
	flrd::PyBase::SetHighVer(glbin_settings.m_python_ver);
	//font
	std::wstring font_file = glbin_settings.m_font_file;
	std::filesystem::path p = std::filesystem::current_path();
	p /= "Fonts";
	if (font_file != L"")
		p /= font_file;
	else
		p /= "FreeSans.ttf";
	font_file = p.wstring();
	glbin_text_tex_manager.load_face(font_file);
	glbin_text_tex_manager.SetSize(glbin_settings.m_text_size);

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

	if (fluo::InEpsilon(glbin_settings.m_dpi_scale_factor,
		GetDPIScaleFactor()))
		m_aui_mgr.LoadPerspective(glbin_settings.m_layout);
	m_clip_plane_panel->LoadPerspective(glbin_settings.m_layout_clip);
	m_movie_panel->LoadPerspective(glbin_settings.m_layout_movie);
	m_output_adj_panel->LoadPerspective(glbin_settings.m_layout_outadj);
	m_proj_panel->LoadPerspective(glbin_settings.m_layout_project);
	m_brush_tool_dlg->LoadPerspective(glbin_settings.m_layout_brush);
	m_component_dlg->LoadPerspective(glbin_settings.m_layout_component);
	m_machine_learning_dlg->LoadPerspective(glbin_settings.m_layout_machine_learning);
	m_measure_dlg->LoadPerspective(glbin_settings.m_layout_measure);
	m_setting_dlg->LoadPerspective(glbin_settings.m_layout_settings);
	m_track_dlg->LoadPerspective(glbin_settings.m_layout_track);
	glbin_moviemaker.SetMainFrame(this);
	glbin_moviemaker.SetView(view);
	glbin_mov_def.Apply(&glbin_moviemaker);

	m_waker.Start(100);

	m_aui_mgr.Update();

	glbin_states.m_status_str = std::string(FLUORENDER_TITLE) + " started normally.";
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
	flvr::KernelProgram::release();

	glbin_brush_def.Set(&glbin_vol_selector);
	glbin_comp_def.Set(&glbin_comp_generator);
	glbin_comp_def.Set(&glbin_clusterizer);
	glbin_comp_def.Set(&glbin_comp_selector);
	glbin_comp_def.Set(&glbin_comp_analyzer);
	glbin_mov_def.Set(&glbin_moviemaker);
	glbin_settings.m_dpi_scale_factor = GetDPIScaleFactor();
	//frame layout
	glbin_settings.m_layout = m_aui_mgr.SavePerspective();
	//panels
	glbin_settings.m_layout_clip = m_clip_plane_panel->SavePerspective();
	glbin_settings.m_layout_movie = m_movie_panel->SavePerspective();
	glbin_settings.m_layout_outadj = m_output_adj_panel->SavePerspective();
	glbin_settings.m_layout_project = m_proj_panel->SavePerspective();
	//dialogs
	glbin_settings.m_layout_brush = m_brush_tool_dlg->SavePerspective();
	glbin_settings.m_layout_component = m_component_dlg->SavePerspective();
	glbin_settings.m_layout_machine_learning = m_machine_learning_dlg->SavePerspective();
	glbin_settings.m_layout_measure = m_measure_dlg->SavePerspective();
	glbin_settings.m_layout_settings = m_setting_dlg->SavePerspective();
	glbin_settings.m_layout_track = m_track_dlg->SavePerspective();
	glbin_settings.Save();

	m_aui_mgr.UnInit();

	m_waker.Stop();
}

wxString MainFrame::CreateRenderViewPanel(int row)
{
	RenderViewPanel* vrv = 0;
	if (m_render_view_panels.size()>0)
	{
		wxGLContext* sharedContext = m_render_view_panels[0]->GetContext();
		vrv = new RenderViewPanel(this, sharedContext);
	}
	else
	{
		vrv = new RenderViewPanel(this);
	}

	if (!vrv)
		return "NO_NAME";

	RenderView* this_view = vrv->GetRenderView();;
	m_render_view_panels.push_back(vrv);
	if (m_movie_panel)
		m_movie_panel->FluoUpdate({ gstMovViewList });

	//reset gl
	for (auto& it : m_render_view_panels)
	{
		if (it)
		{
			it->SetGL(false);
		}
	}

	//m_aui_mgr.Update();
	LayoutRenderViewPanels(1);

	//set view default settings
	//if (m_output_adj_panel)
	//{
		/*Color gamma, brightness, hdr;
		bool sync_r, sync_g, sync_b;
		m_output_adj_panel->GetDefaults(gamma, brightness, hdr, sync_r, sync_g, sync_b);
		vrv->m_canvas->SetGamma(gamma);
		vrv->m_canvas->SetBrightness(brightness);
		vrv->m_canvas->SetHdr(hdr);*/
		//view->SetSyncR(true);
		//view->SetSyncG(true);
		//view->SetSyncB(true);
	//}

	//add volumes
	Root* root = glbin_data_manager.GetRoot();
	if (root && root->GetViewNum() > 0)
	{
		auto view0 = root->GetView(0);
		if (view0)
		{
			for (int i = 0; i < view0->GetDispVolumeNum(); ++i)
			{
				auto vd = view0->GetDispVolumeData(i);
				if (vd)
				{
					auto vd_add = glbin_data_manager.DuplicateVolumeData(vd);

					if (vd_add)
					{
						int chan_num = this_view->GetAny();
						fluo::Color color(1.0, 1.0, 1.0);
						if (chan_num == 0)
							color = fluo::Color(1.0, 0.0, 0.0);
						else if (chan_num == 1)
							color = fluo::Color(0.0, 1.0, 0.0);
						else if (chan_num == 2)
							color = fluo::Color(0.0, 0.0, 1.0);

						if (chan_num >= 0 && chan_num < 3)
							vd_add->SetColor(color);

						this_view->AddVolumeData(vd_add);
					}
				}
			}
		}
		//update
		this_view->InitView(INIT_BOUNDS | INIT_CENTER | INIT_TRANSL | INIT_ROTATE);
	}

	vrv->LoadSettings();

	UpdateProps({ gstMovViewList, gstMovViewIndex, gstTreeCtrl, gstUpdateSync });

	return vrv->GetName();
}

void MainFrame::LoadPerspective(const wxString& str)
{
	m_aui_mgr.LoadPerspective(str);
	m_aui_mgr.Update();
}

wxString MainFrame::SavePerspective()
{
	return m_aui_mgr.SavePerspective();
}

ProjectPanel* MainFrame::GetProjectPanel()
{
	return m_proj_panel;
}

TreePanel *MainFrame::GetTreePanel()
{
	return m_tree_panel;
}

ListPanel *MainFrame::GetListPanel()
{
	return m_list_panel;
}

//prop panels
void MainFrame::FluoUpdate(const fluo::ValueCollection& vc)
{
	int type = 0;
	if (FOUND_VALUE(gstVolumePropPanel))
		type = 2;
	else if (FOUND_VALUE(gstMeshPropPanel))
		type = 3;
	else if (FOUND_VALUE(gstManipPropPanel))
		type = 6;
	else if (FOUND_VALUE(gstAnnotatPropPanel))
		type = 4;
	ShowPropPage(type,
		glbin_current.render_view.lock().get(),
		glbin_current.vol_group.lock().get(),
		glbin_current.vol_data.lock().get(),
		glbin_current.mesh_data.lock().get(),
		glbin_current.ann_data.lock().get());

	//clear mesh bounds
	glbin_data_manager.ClearMeshSelection();
	auto md = glbin_current.mesh_data.lock();
	if (md)
		md->SetDrawBounds(true);

	bool update_all = vc.empty();
	bool bval;
	wxString str;
	if (update_all || FOUND_VALUE(gstProjPanel))
	{
		//proj panel
		bval = m_aui_mgr.GetPane(m_proj_panel).IsShown();
		m_tb_menu_ui->Check(ID_ProjPanel, bval);
		m_top_window->Check(ID_ProjPanelMenu, bval);
	}
	if (update_all || FOUND_VALUE(gstMoviePanel))
	{
		//movie panel
		bval = m_aui_mgr.GetPane(m_movie_panel).IsShown();
		m_tb_menu_ui->Check(ID_MoviePanel, bval);
		m_top_window->Check(ID_MoviePanelMenu, bval);
	}
	if (update_all || FOUND_VALUE(gstPropPanel))
	{
		//prop panel
		bval = m_aui_mgr.GetPane(m_prop_panel).IsShown();
		m_tb_menu_ui->Check(ID_PropPanel, bval);
		m_top_window->Check(ID_PropPanelMenu, bval);
	}
	if (update_all || FOUND_VALUE(gstOutAdjPanel))
	{
		//adjust panel
		bval = m_aui_mgr.GetPane(m_output_adj_panel).IsShown();
		m_tb_menu_ui->Check(ID_OutAdjPanel, bval);
		m_top_window->Check(ID_OutAdjPanelMenu, bval);
	}
	if (update_all || FOUND_VALUE(gstClipPlanePanel))
	{
		//clipping plane panel
		bval = m_aui_mgr.GetPane(m_clip_plane_panel).IsShown();
		m_tb_menu_ui->Check(ID_ClipPlanePanel, bval);
		m_top_window->Check(ID_ClipPlanePanelMenu, bval);
	}
	if (update_all || FOUND_VALUE(gstMainToolbar))
	{
		//main toolbar
		bval = m_aui_mgr.GetPane(m_main_tb).IsShown();
		m_top_window->Check(ID_ToolbarMenu, bval);
	}

	if (update_all || FOUND_VALUE(gstMainFrameTitle))
	{
		str = glbin_data_manager.GetProjectFile();
		if (str.IsEmpty())
			str = m_title;
		else
			str = m_title + " - " + str;
		SetTitle(str);
	}

	if (update_all || FOUND_VALUE(gstMainStatusbarText))
	{
		m_statusbar->SetStatusText(glbin_states.m_status_str);
	}
	if (FOUND_VALUE(gstMainStatusbarPush))
	{
		m_statusbar->PushStatusText(glbin_states.m_status_str);
	}
	if (FOUND_VALUE(gstMainStatusbarPop))
	{
		m_statusbar->PopStatusText();
	}
}

wxWindow* MainFrame::AddProps(int type,
	RenderView* view,
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
			pane->SetView(view);
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
			pane->SetView(view);
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
			pane->SetName(ann->GetName());
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
			pane->SetName(md->GetName() + L" M");
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
	}
}

void MainFrame::ShowPropPage(int type,
	RenderView* view,
	DataGroup* group,
	VolumeData* vd,
	MeshData* md,
	Annotations* ann,
	bool show )
{
	if (!type)
	{
		int n = 0;
		n += glbin_data_manager.GetVolumeNum();
		n += glbin_data_manager.GetMeshNum();
		n += glbin_data_manager.GetAnnotationNum();
		if (!n)
		{
			m_prop_panel->DeleteAllPages();
			m_prop_pages.clear();
			return;
		}
	}
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
		page = AddProps(type, view, group, vd, md, ann);
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
		m_prop_panel->DeletePage(page_no);
	}
}

bool MainFrame::update_props(int excl_self, wxWindow* p1, wxWindow* p2)
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

void MainFrame::UpdateProps(const fluo::ValueCollection& vc, int excl_self, wxWindow* panel)
{
	//frame
	if (update_props(excl_self, this, panel))
		FluoUpdate(vc);
	//panels
	if (update_props(excl_self, m_list_panel, panel))
		m_list_panel->FluoUpdate(vc);
	if (update_props(excl_self, m_tree_panel, panel))
		m_tree_panel->FluoUpdate(vc);
	if (update_props(excl_self, m_movie_panel, panel))
		m_movie_panel->FluoUpdate(vc);
	if (update_props(excl_self, m_output_adj_panel, panel))
		m_output_adj_panel->FluoUpdate(vc);
	if (update_props(excl_self, m_clip_plane_panel, panel))
		m_clip_plane_panel->FluoUpdate(vc);
	for (auto i : m_render_view_panels)
		if (update_props(excl_self, i, panel))
			i->FluoUpdate(vc);
	for (auto i : m_prop_pages)
		if (update_props(excl_self, i, panel))
			i->FluoUpdate(vc);
	//dialogs
	if (update_props(excl_self, m_brush_tool_dlg, panel))
		m_brush_tool_dlg->FluoUpdate(vc);
	if (update_props(excl_self, m_calculation_dlg, panel))
		m_calculation_dlg->FluoUpdate(vc);
	if (update_props(excl_self, m_colocalization_dlg, panel))
		m_colocalization_dlg->FluoUpdate(vc);
	if (update_props(excl_self, m_component_dlg, panel))
		m_component_dlg->FluoUpdate(vc);
	if (update_props(excl_self, m_convert_dlg, panel))
		m_convert_dlg->FluoUpdate(vc);
	if (update_props(excl_self, m_counting_dlg, panel))
		m_counting_dlg->FluoUpdate(vc);
	if (update_props(excl_self, m_fp_range_dlg, panel))
		m_fp_range_dlg->FluoUpdate(vc);
	if (update_props(excl_self, m_help_dlg, panel))
		m_help_dlg->FluoUpdate(vc);
	if (update_props(excl_self, m_machine_learning_dlg, panel))
		m_machine_learning_dlg->FluoUpdate(vc);
	if (update_props(excl_self, m_measure_dlg, panel))
		m_measure_dlg->FluoUpdate(vc);
	if (update_props(excl_self, m_noise_cancelling_dlg, panel))
		m_noise_cancelling_dlg->FluoUpdate(vc);
	if (update_props(excl_self, m_ocl_dlg, panel))
		m_ocl_dlg->FluoUpdate(vc);
	if (update_props(excl_self, m_script_break_dlg, panel))
		m_script_break_dlg->FluoUpdate(vc);
	if (update_props(excl_self, m_setting_dlg, panel))
		m_setting_dlg->FluoUpdate(vc);
	if (update_props(excl_self, m_teser_dlg, panel))
		m_teser_dlg->FluoUpdate(vc);
	if (update_props(excl_self, m_track_dlg, panel))
		m_track_dlg->FluoUpdate(vc);
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
OutputAdjPanel* MainFrame::GetOutAdjPanel()
{
	return m_output_adj_panel;
}

//movie view
MoviePanel* MainFrame::GetMoviePanel()
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
ClipPlanePanel* MainFrame::GetClipPlanePanel()
{
	return m_clip_plane_panel;
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
TrackDlg* MainFrame::GetTrackDlg()
{
	return m_track_dlg;
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

//floating point volume range
FpRangeDlg* MainFrame::GetFpRangeDlg()
{
	return m_fp_range_dlg;
}

MachineLearningDlg* MainFrame::GetMachineLearningDlg()
{
	return m_machine_learning_dlg;
}

void MainFrame::RefreshCanvases(const std::set<int>& canvases)
{
	if (canvases.find(-1) != canvases.end())
		return;
	bool update_all = canvases.empty();

	for (int i=0 ; i<(int)m_render_view_panels.size() ; i++)
	{
		if (!m_render_view_panels[i])
			continue;

		if (update_all || canvases.find(i) != canvases.end())
			m_render_view_panels[i]->RefreshGL();
	}
}

void MainFrame::DeleteRenderViewPanel(int i)
{
	if (m_render_view_panels[i])
	{
		RenderViewPanel* vrv = m_render_view_panels[i];
		RenderView* view = vrv->GetRenderView();
		for (int j=0 ; j<view->GetAllVolumeNum() ; j++)
			view->GetAllVolumeData(j)->SetDisp(true);
		for (int j=0 ; j< view->GetMeshNum() ; j++)
			view->GetMeshData(j)->SetDisp(true);

		m_render_view_panels.erase(m_render_view_panels.begin()+i);
		m_aui_mgr.DetachPane(vrv);
		vrv->Close();
		vrv->Destroy();
		m_aui_mgr.Update();

		if (glbin_mov_def.m_view_idx >= i)
			glbin_mov_def.m_view_idx--;
	}
}

void MainFrame::DeleteRenderViewPanel(const std::wstring &name)
{
	for (size_t i=0; i<m_render_view_panels.size(); i++)
	{
		RenderView* view = m_render_view_panels[i]->GetRenderView();
		if (view && name == view->GetName() && view->Id() > 1)
		{
			DeleteRenderViewPanel(i);
			break;
		}
	}
}

//hide/show tools
void MainFrame::ToggleAllPanels(bool cur_state)
{
	bool bval = true;
	if (cur_state)
	{
		if (m_aui_mgr.GetPane(m_proj_panel).IsShown() &&
			m_aui_mgr.GetPane(m_movie_panel).IsShown() &&
			m_aui_mgr.GetPane(m_prop_panel).IsShown() &&
			m_aui_mgr.GetPane(m_output_adj_panel).IsShown() &&
			m_aui_mgr.GetPane(m_clip_plane_panel).IsShown())
			bval = true;
		else if (!m_aui_mgr.GetPane(m_proj_panel).IsShown() &&
			!m_aui_mgr.GetPane(m_movie_panel).IsShown() &&
			!m_aui_mgr.GetPane(m_prop_panel).IsShown() &&
			!m_aui_mgr.GetPane(m_output_adj_panel).IsShown() &&
			!m_aui_mgr.GetPane(m_clip_plane_panel).IsShown())
			bval = false;
		bval = !bval;
	}

	//data view
	m_aui_mgr.GetPane(m_proj_panel).Show(bval);
	//movie view (float only)
	m_aui_mgr.GetPane(m_movie_panel).Show(bval);
	//properties
	m_aui_mgr.GetPane(m_prop_panel).Show(bval);
	//adjust view
	m_aui_mgr.GetPane(m_output_adj_panel).Show(bval);
	//clipping view
	m_aui_mgr.GetPane(m_clip_plane_panel).Show(bval);

	m_aui_mgr.Update();

	FluoUpdate({ gstProjPanel, gstMoviePanel, gstPropPanel, gstOutAdjPanel, gstClipPlanePanel });
}

void MainFrame::ToggleLastTool()
{
	unsigned int tool = glbin_settings.m_last_tool;
	switch (tool)
	{
	case 0:
	case TOOL_PAINT_BRUSH:
	default:
		ShowBrushDlg();
		break;
	case TOOL_MEASUREMENT:
		ShowMeasureDlg();
		break;
	case TOOL_TRACKING:
		ShowTrackDlg();
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

void MainFrame::ShowPanel(wxPanel* pane, bool show)
{
	if (m_aui_mgr.GetPane(pane).IsOk())
	{
		m_aui_mgr.GetPane(pane).Show(show);
		m_aui_mgr.Update();
	}
}

void MainFrame::ShowSettingDlg()
{
	m_aui_mgr.GetPane(m_setting_dlg).Show();
	m_aui_mgr.Update();
}

void MainFrame::ShowBrushDlg()
{
	m_aui_mgr.GetPane(m_brush_tool_dlg).Show();
	glbin_settings.m_last_tool = TOOL_PAINT_BRUSH;
	m_main_tb->SetToolBitmap(ID_LastTool,
		wxGetBitmap(icon_paint_brush));
	m_aui_mgr.Update();
}

void MainFrame::ShowMeasureDlg()
{
	m_aui_mgr.GetPane(m_measure_dlg).Show();
	glbin_settings.m_last_tool = TOOL_MEASUREMENT;
	m_main_tb->SetToolBitmap(ID_LastTool,
		wxGetBitmap(icon_measurement));
	m_aui_mgr.Update();
}

void MainFrame::ShowTrackDlg()
{
	m_aui_mgr.GetPane(m_track_dlg).Show();
	glbin_settings.m_last_tool = TOOL_TRACKING;
	m_main_tb->SetToolBitmap(ID_LastTool,
		wxGetBitmap(icon_tracking));
	m_aui_mgr.Update();
}

void MainFrame::ShowNoiseCancellingDlg()
{
	m_aui_mgr.GetPane(m_noise_cancelling_dlg).Show();
	glbin_settings.m_last_tool = TOOL_NOISE_REDUCTION;
	m_main_tb->SetToolBitmap(ID_LastTool,
		wxGetBitmap(icon_noise_reduc));
	m_aui_mgr.Update();
}

void MainFrame::ShowCountingDlg()
{
	m_aui_mgr.GetPane(m_counting_dlg).Show();
	glbin_settings.m_last_tool = TOOL_VOLUME_SIZE;
	m_main_tb->SetToolBitmap(ID_LastTool,
		wxGetBitmap(icon_volume_size));
	m_aui_mgr.Update();
}

void MainFrame::ShowColocalizationDlg()
{
	m_aui_mgr.GetPane(m_colocalization_dlg).Show();
	glbin_settings.m_last_tool = TOOL_COLOCALIZATION;
	m_main_tb->SetToolBitmap(ID_LastTool,
		wxGetBitmap(icon_colocalization));
	m_aui_mgr.Update();
}

void MainFrame::ShowConvertDlg()
{
	m_aui_mgr.GetPane(m_convert_dlg).Show();
	glbin_settings.m_last_tool = TOOL_CONVERT;
	m_main_tb->SetToolBitmap(ID_LastTool,
		wxGetBitmap(icon_convert));
	m_aui_mgr.Update();
}

void MainFrame::ShowOclDlg()
{
	m_aui_mgr.GetPane(m_ocl_dlg).Show();
	glbin_settings.m_last_tool = TOOL_OPENCL;
	m_main_tb->SetToolBitmap(ID_LastTool,
		wxGetBitmap(tb_volume_filter));
	m_aui_mgr.Update();
}

void MainFrame::ShowComponentDlg()
{
	m_aui_mgr.GetPane(m_component_dlg).Show();
	glbin_settings.m_last_tool = TOOL_COMPONENT;
	m_main_tb->SetToolBitmap(ID_LastTool,
		wxGetBitmap(icon_components));
	m_aui_mgr.Update();
}

void MainFrame::ShowCalculationDlg()
{
	m_aui_mgr.GetPane(m_calculation_dlg).Show();
	glbin_settings.m_last_tool = TOOL_CALCULATIONS;
	m_main_tb->SetToolBitmap(ID_LastTool,
		wxGetBitmap(icon_calculations));
	m_aui_mgr.Update();
}

void MainFrame::ShowMachineLearningDlg()
{
	m_aui_mgr.GetPane(m_machine_learning_dlg).Show();
	glbin_settings.m_last_tool = TOOL_MACHINE_LEARNING;
	m_main_tb->SetToolBitmap(ID_LastTool,
		wxGetBitmap(icon_machine_learning));
	m_aui_mgr.Update();
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

void MainFrame::ShowInfo()
{
	wxString time = wxNow();
	int psJan = time.Find("Jan");
	int psDec = time.Find("Dec");
	wxDialog* d = new wxDialog(this, wxID_ANY, "About FluoRender", wxDefaultPosition,
		FromDIP(wxSize(600, 200)), wxDEFAULT_DIALOG_STYLE);
	wxBoxSizer* main = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* left = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* right = new wxBoxSizer(wxVERTICAL);
	//left
	// FluoRender Image (rows 4-5)
	wxToolBar* logo = new wxToolBar(d, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	wxBitmapBundle bitmap;
	if (psJan != wxNOT_FOUND || psDec != wxNOT_FOUND)
		bitmap = wxGetBitmap(logo_snow);
	else
		bitmap = wxGetBitmap(logo);
	logo->AddTool(0, "", bitmap,
		"Thank you for using FluoRender");
	logo->SetToolLongHelp(0,
		"Thank you for using FluoRender");
	logo->Realize();
	left->Add(logo, 0, wxEXPAND);
	//right
	wxStaticText* txt = new wxStaticText(d, wxID_ANY, FLUORENDER_TITLE,
		wxDefaultPosition, FromDIP(wxSize(-1, -1)));
	wxFont font = wxFont(15, wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	txt->SetFont(font);
	right->Add(txt, 0, wxEXPAND);
	txt = new wxStaticText(d, wxID_ANY, "Version: " +
		std::format("{}.{}", VERSION_MAJOR, std::format("{:.1f}", float(VERSION_MINOR)))),
		wxDefaultPosition, FromDIP(wxSize(50, -1));
	font = wxFont(12, wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	txt->SetFont(font);
	right->Add(txt, 0, wxEXPAND);
	txt = new wxStaticText(d, wxID_ANY, wxString("Copyright (c) ") + VERSION_COPYRIGHT,
		wxDefaultPosition, FromDIP(wxSize(-1, -1)));
	font = wxFont(11, wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	txt->SetFont(font);
	right->Add(txt, 0, wxEXPAND);
	right->Add(3, 5, 0);
	txt = new wxStaticText(d, wxID_ANY, VERSION_AUTHORS,
		wxDefaultPosition, FromDIP(wxSize(300, -1)));
	font = wxFont(7, wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	txt->SetFont(font);
	right->Add(txt, 0, wxEXPAND);
	wxHyperlinkCtrl* hyp = new wxHyperlinkCtrl(d, wxID_ANY, "Contact Info",
		VERSION_CONTACT,
		wxDefaultPosition, FromDIP(wxSize(-1, -1)));
	right->Add(hyp, 0, wxEXPAND);
	right->AddStretchSpacer();
	//put together
	main->Add(left, 0, wxEXPAND);
	main->Add(right, 0, wxEXPAND);
	d->SetSizer(main);
	d->ShowModal();
}

std::wstring MainFrame::ScriptDialog(const std::wstring& title,
	const std::wstring& wildcard, long style)
{
	glbin_moviemaker.Hold();
	std::wstring result;
	ModalDlg dlg(
		this, title, "", "",
		wildcard, style);
	int rval = dlg.ShowModal();
	if (rval == wxID_OK)
		result = dlg.GetPath().ToStdWstring();
	glbin_moviemaker.Resume();
	return result;
}

void MainFrame::ShowProjPanel()
{
	bool bval = m_aui_mgr.GetPane(m_proj_panel).IsShown();
	bval = !bval;
	m_aui_mgr.GetPane(m_proj_panel).Show(bval);
	m_aui_mgr.Update();
	FluoUpdate({ gstProjPanel });
}

void MainFrame::ShowMoviePanel()
{
	bool bval = m_aui_mgr.GetPane(m_movie_panel).IsShown();
	bval = !bval;
	m_aui_mgr.GetPane(m_movie_panel).Show(bval);
	m_aui_mgr.Update();
	FluoUpdate({ gstMoviePanel });
}

void MainFrame::ShowOutAdjPanel()
{
	bool bval = m_aui_mgr.GetPane(m_output_adj_panel).IsShown();
	bval = !bval;
	m_aui_mgr.GetPane(m_output_adj_panel).Show(bval);
	m_aui_mgr.Update();
	FluoUpdate({ gstOutAdjPanel });
}

void MainFrame::ShowClipPlanePanel()
{
	bool bval = m_aui_mgr.GetPane(m_clip_plane_panel).IsShown();
	bval = !bval;
	m_aui_mgr.GetPane(m_clip_plane_panel).Show(bval);
	m_aui_mgr.Update();
	FluoUpdate({ gstClipPlanePanel });
}

void MainFrame::ShowPropPanel()
{
	bool bval = m_aui_mgr.GetPane(m_prop_panel).IsShown();
	bval = !bval;
	m_aui_mgr.GetPane(m_prop_panel).Show(bval);
	m_aui_mgr.Update();
	FluoUpdate({ gstPropPanel });
}

void MainFrame::ShowToolbar()
{
	bool bval = m_aui_mgr.GetPane(m_main_tb).IsShown();
	bval = !bval;
	m_aui_mgr.GetPane(m_main_tb).Show(bval);
	m_aui_mgr.Update();
	FluoUpdate({ gstMainToolbar });
}

//organize render views
void MainFrame::LayoutRenderViewPanels(int mode)
{
	int width = 0;
	int height = 0;
	int minx = 0;
	int miny = 0;
	int maxx = 0;
	int maxy = 0;
	int paneNum = (int)m_render_view_panels.size();
	int i;
	//get total area
	for (i = 0; i < paneNum; i++)
	{
		RenderViewPanel* vrv = m_render_view_panels[i];
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
		RenderViewPanel* vrv = m_render_view_panels[i];
		if (vrv)
			m_aui_mgr.DetachPane(vrv);
	}
	//add back
	for (i = 0; i < paneNum; i++)
	{
		RenderViewPanel* vrv = m_render_view_panels[i];
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
		RenderViewPanel* vrv = m_render_view_panels[i];
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
	m_aui_mgr.GetPane(m_output_adj_panel).Show().Dock().
		BestSize(FromDIP(wxSize(150, 800))).
		FloatingSize(FromDIP(wxSize(150, 800))).
		Left().Layer(1);
	m_aui_mgr.GetPane(m_clip_plane_panel).Show().Dock().
		BestSize(FromDIP(wxSize(150, 800))).
		FloatingSize(FromDIP(wxSize(150, 800))).
		Right().Show().Dock().Layer(1);
	LayoutRenderViewPanels(1);
	glbin_settings.m_layout.clear();
	FluoUpdate({ gstMainToolbar, gstProjPanel, gstMoviePanel, gstPropPanel, gstOutAdjPanel, gstClipPlanePanel });
}

void MainFrame::FullScreen()
{
	if (IsFullScreen())
	{
		ShowFullScreen(false);
		wxMenuItem* m = m_top_window->FindItem(ID_FullscreenMenu);
		if (m)
			m->SetBitmap(wxGetBitmap(full_screen_menu));
	}
	else
	{
		ShowFullScreen(true,
			wxFULLSCREEN_NOBORDER |
			wxFULLSCREEN_NOCAPTION);
		wxMenuItem* m = m_top_window->FindItem(ID_FullscreenMenu);
		if (m)
			m->SetBitmap(wxGetBitmap(full_screen_back_menu));
	}
}

void MainFrame::SetProgress(int val, const std::string& str)
{
	wxGetApp().Yield();
	m_statusbar->SetGaugeValue(val);
	if (str != "NOT_SET")
		m_statusbar->SetGaugeText(str);
}

void MainFrame::OpenVolume()
{
	ModalDlg fopendlg(
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
		wxFD_OPEN | wxFD_MULTIPLE);
	fopendlg.SetExtraControlCreator(CreateExtraControlVolume);

	int rval = fopendlg.ShowModal();
	if (rval == wxID_OK)
	{
		wxArrayString paths;
		fopendlg.GetPaths(paths);
		std::vector<std::wstring> std_filenames;
		std_filenames.reserve(paths.size());
		for (const auto& filename : paths) {
			std_filenames.push_back(filename.ToStdWstring());
		}
		glbin_data_manager.LoadVolumes(std_filenames, false);
	}
}

void MainFrame::ImportVolume()
{
	ModalDlg fopendlg(
		this, "Choose the volume data file", "", "", "All Files|*.*",
		wxFD_OPEN | wxFD_MULTIPLE);
	fopendlg.SetExtraControlCreator(CreateExtraControlVolumeForImport);

	int rval = fopendlg.ShowModal();
	if (rval == wxID_OK)
	{
		wxArrayString paths;
		fopendlg.GetPaths(paths);
		std::vector<std::wstring> std_filenames;
		std_filenames.reserve(paths.size());
		for (const auto& filename : paths) {
			std_filenames.push_back(filename.ToStdWstring());
		}
		glbin_data_manager.LoadVolumes(std_filenames, true);
	}
}

void MainFrame::OpenMesh()
{
	ModalDlg fopendlg(
		this, "Choose the mesh data file",
		"", "", "*.obj", wxFD_OPEN | wxFD_MULTIPLE);

	int rval = fopendlg.ShowModal();
	if (rval == wxID_OK)
	{
		wxArrayString files;
		fopendlg.GetPaths(files);
		std::vector<std::wstring> std_filenames;
		std_filenames.reserve(files.size());
		for (const auto& filename : files) {
			std_filenames.push_back(filename.ToStdWstring());
		}
		glbin_data_manager.LoadMeshes(std_filenames);
	}
}

void MainFrame::NewProject()
{
	glbin_project.Reset();
	RefreshCanvases();
	UpdateProps({ gstListCtrl, gstTreeCtrl, gstParamList });
}

void MainFrame::OpenProject()
{
	ModalDlg fopendlg(
		this, "Choose Project File",
		"", "", "*.vrp", wxFD_OPEN);

	int rval = fopendlg.ShowModal();
	if (rval == wxID_OK)
	{
		std::wstring path = fopendlg.GetPath().ToStdWstring();
		glbin_project.Open(path);
		//FluoUpdate({ gstMainFrameTitle });
	}
}

void MainFrame::SaveProject()
{
	std::wstring default_path = glbin_data_manager.GetProjectFile();
	if (default_path.empty())
	{
		SaveAsProject();
	}
	else
	{
		bool inc = glbin_settings.m_prj_save_inc;
		glbin_project.Save(default_path, inc);
		FluoUpdate({ gstMainFrameTitle });
	}
}

void MainFrame::SaveAsProject()
{
	std::wstring default_path = glbin_data_manager.GetProjectFile();
	std::wstring path;
	std::wstring filename;
	bool default_valid = SEP_PATH_NAME(default_path, path, filename);

	ModalDlg fopendlg(
		this, "Save Project File",
		default_valid ? path : L"", default_valid ? filename : L"",
		"*.vrp", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	fopendlg.SetExtraControlCreator(CreateExtraControlProjectSave);

	int rval = fopendlg.ShowModal();
	if (rval == wxID_OK)
	{
		filename = fopendlg.GetPath().ToStdWstring();
		glbin_project.Save(filename, false);
		FluoUpdate({ gstMainFrameTitle });
	}
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
	int id = event.GetId();

	switch (id)
	{
		//toolbar
	case ID_OpenVolume:
		OpenVolume();
		break;
	case ID_ImportVolume:
		ImportVolume();
		break;
	case ID_OpenProject:
		OpenProject();
		break;
	case ID_SaveProject:
		SaveProject();
		break;
	case ID_ViewNew:
		CreateRenderViewPanel();
		break;
	case ID_Panels:
		ToggleAllPanels(true);
		break;
	case ID_OpenMesh:
		OpenMesh();
		break;
	case ID_LastTool:
		ToggleLastTool();
		break;
	case ID_Undo:
		glbin.undo();
		break;
	case ID_Redo:
		glbin.redo();
		break;
	case ID_Settings:
		ShowSettingDlg();
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
		ShowInfo();
		break;
		//toolbar menu items
	case ID_ProjPanel:
		ShowProjPanel();
		break;
	case ID_MoviePanel:
		ShowMoviePanel();
		break;
	case ID_OutAdjPanel:
		ShowOutAdjPanel();
		break;
	case ID_ClipPlanePanel:
		ShowClipPlanePanel();
		break;
	case ID_PropPanel:
		ShowPropPanel();
		break;
	case ID_BrushDlg:
		ShowBrushDlg();
		break;
	case ID_MeasureDlg:
		ShowMeasureDlg();
		break;
	case ID_ComponentDlg:
		ShowComponentDlg();
		break;
	case ID_TrackDlg:
		ShowTrackDlg();
		break;
	case ID_CalcDlg:
		ShowCalculationDlg();
		break;
	case ID_NoiseCancelDlg:
		ShowNoiseCancellingDlg();
		break;
	case ID_CountDlg:
		ShowCountingDlg();
		break;
	case ID_ColocalDlg:
		ShowColocalizationDlg();
		break;
	case ID_ConvertDlg:
		ShowConvertDlg();
		break;
	case ID_OclDlg:
		ShowOclDlg();
		break;
	case ID_MlDlg:
		ShowMachineLearningDlg();
		break;
		//main menu items
		//file
	case ID_NewProjMenu:
		NewProject();
		break;
	case ID_OpenVolumeMenu:
		OpenVolume();
		break;
	case ID_ImportVolumeMenu:
		ImportVolume();
		break;
	case ID_OpenMeshMenu:
		OpenMesh();
		break;
	case ID_OpenProjMenu:
		OpenProject();
		break;
	case ID_SaveProjMenu:
		SaveProject();
		break;
	case ID_SaveAsProjMenu:
		SaveAsProject();
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
		ShowBrushDlg();
		break;
	case ID_MeasureDlgMenu:
		ShowMeasureDlg();
		break;
	case ID_CompDlgMenu:
		ShowComponentDlg();
		break;
	case ID_TrackDlgMenu:
		ShowTrackDlg();
		break;
	case ID_CalcDlgMenu:
		ShowCalculationDlg();
		break;
	case ID_NoiseCancelDlgMenu:
		ShowNoiseCancellingDlg();
		break;
	case ID_CountDlgMenu:
		ShowCountingDlg();
		break;
	case ID_ColocalDlgMenu:
		ShowColocalizationDlg();
		break;
	case ID_ConvertDlgMenu:
		ShowConvertDlg();
		break;
	case ID_OclDlgMenu:
		ShowOclDlg();
		break;
	case ID_MlDlgMenu:
		ShowMachineLearningDlg();
		break;
		//window
	case ID_ToolbarMenu:
		ShowToolbar();
		break;
	case ID_PanelsMenu:
		ToggleAllPanels(true);
		break;
	case ID_ProjPanelMenu:
		ShowProjPanel();
		break;
	case ID_MoviePanelMenu:
		ShowMoviePanel();
		break;
	case ID_OutAdjPanelMenu:
		ShowOutAdjPanel();
		break;
	case ID_ClipPlanePanelMenu:
		ShowClipPlanePanel();
		break;
	case ID_PropPanelMenu:
		ShowPropPanel();
		break;
	case ID_LayoutMenu:
		LayoutRenderViewPanels(1);
		break;
	case ID_ResetMenu:
		ResetLayout();
		break;
	case ID_ViewNewMenu:
		CreateRenderViewPanel();
		break;
	case ID_FullscreenMenu:
		FullScreen();
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
		ShowInfo();
		break;
	}

	//event.StopPropagation();
}

//panes
void MainFrame::OnPaneClose(wxAuiManagerEvent& event)
{
	FluoUpdate({ gstProjPanel, gstMoviePanel, gstPropPanel, gstOutAdjPanel, gstClipPlanePanel });
}

void MainFrame::OnClose(wxCloseEvent& event)
{
	glbin_moviemaker.Stop();

	bool vrv_saved = false;
	for (unsigned int i = 0; i < m_render_view_panels.size(); ++i)
	{
		if (m_render_view_panels[i]->m_default_saved)
		{
			vrv_saved = true;
			break;
		}
		m_render_view_panels[i]->CloseFullScreen();
	}
	if (!vrv_saved && !m_render_view_panels.empty())
		m_render_view_panels[0]->SaveDefault(0xaff);
	glbin.clear_python();
	event.Skip();
}

wxWindow* MainFrame::CreateExtraControlVolume(wxWindow* parent)
{
	wxPanel* panel = new wxPanel(parent);
#ifdef _DARWIN
	panel->SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#elifdef __linux__
	panel->SetWindowVariant(wxWINDOW_VARIANT_MINI);
#endif
	wxStaticText* st1, * st2;

	wxStaticBoxSizer* group1 = new wxStaticBoxSizer(
		wxVERTICAL, panel, "Additional Options");

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
#elifdef __linux__
	panel->SetWindowVariant(wxWINDOW_VARIANT_MINI);
#endif
	wxStaticBoxSizer* group1 = new wxStaticBoxSizer(
		wxVERTICAL, panel, "Additional Options");

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

wxWindow* MainFrame::CreateExtraControlProjectSave(wxWindow* parent)
{
	wxPanel* panel = new wxPanel(parent);
#ifdef _DARWIN
	panel->SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#elifdef __linux__
	panel->SetWindowVariant(wxWINDOW_VARIANT_MINI);
#endif
	wxStaticBoxSizer* group1 = new wxStaticBoxSizer(
		wxVERTICAL, panel, "Additional Options");

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

//open dialog options
void MainFrame::OnCh11Check(wxCommandEvent& event)
{
	wxCheckBox* ch11 = (wxCheckBox*)event.GetEventObject();
	if (ch11)
		glbin_settings.m_slice_sequence = ch11->GetValue();
}

void MainFrame::OnCh12Check(wxCommandEvent& event)
{
	wxCheckBox* ch12 = (wxCheckBox*)event.GetEventObject();
	if (ch12)
		glbin_settings.m_chann_sequence = ch12->GetValue();
}

void MainFrame::OnCmbChange(wxCommandEvent& event)
{
	wxComboBox* combo = (wxComboBox*)event.GetEventObject();
	if (combo)
		glbin_settings.m_digit_order = combo->GetSelection();
}

void MainFrame::OnTxt1Change(wxCommandEvent& event)
{
	wxTextCtrl* txt1 = (wxTextCtrl*)event.GetEventObject();
	if (txt1)
		glbin_settings.m_time_id = txt1->GetValue();
}

void MainFrame::OnTxt2Change(wxCommandEvent& event)
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

void MainFrame::OnCh2Check(wxCommandEvent& event)
{
	wxCheckBox* ch2 = (wxCheckBox*)event.GetEventObject();
	if (ch2)
		glbin_settings.m_realtime_compress = ch2->GetValue();
}

void MainFrame::OnCh3Check(wxCommandEvent& event)
{
	wxCheckBox* ch3 = (wxCheckBox*)event.GetEventObject();
	if (ch3)
		glbin_settings.m_skip_brick = ch3->GetValue();
}

void MainFrame::OnChEmbedCheck(wxCommandEvent& event)
{
	wxCheckBox* ch_embed = (wxCheckBox*)event.GetEventObject();
	if (ch_embed)
		glbin_settings.m_vrp_embed = ch_embed->GetValue();
}

void MainFrame::OnChSaveCmpCheck(wxCommandEvent& event)
{
	wxCheckBox* ch_cmp = (wxCheckBox*)event.GetEventObject();
	if (ch_cmp)
		glbin_settings.m_save_compress = ch_cmp->GetValue();
}

void MainFrame::OnSysColorChanged(wxSysColourChangedEvent& event)
{
	auto appearance = wxSystemSettings::GetAppearance();
	wxApp::GetGUIInstance()->SetAppearance(wxApp::Appearance::System);

	// Optionally, refresh UI elements
	Refresh();
	event.Skip(); // Allow default processing
}
