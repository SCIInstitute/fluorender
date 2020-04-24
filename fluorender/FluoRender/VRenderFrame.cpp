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
#include "VRenderFrame.h"
#include "DragDrop.h"
#include <wx/artprov.h>
#include <wx/wfstream.h>
#include <wx/fileconf.h>
#include <wx/aboutdlg.h>
#include <wx/progdlg.h>
#include <wx/hyperlink.h>
#include <wx/stdpaths.h>
#include "Formats/png_resource.h"
#include "Formats/msk_writer.h"
#include "Formats/msk_reader.h"
#include "Converters/VolumeMeshConv.h"
#include <Selection/VolumeSelector.h>
#include "compatibility.h"
#include <FLIVR/TextRenderer.h>
#include <FLIVR/VertexArray.h>
#include <FLIVR/Framebuffer.h>
#include <FLIVR/VolShader.h>
#include <FLIVR/SegShader.h>
#include <FLIVR/VolCalShader.h>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <cctype>

//resources
#include "img/icons.h"

BEGIN_EVENT_TABLE(VRenderFrame, wxFrame)
	EVT_MENU(wxID_EXIT, VRenderFrame::OnExit)
	EVT_MENU(ID_ViewNew, VRenderFrame::OnNewView)
	EVT_MENU(ID_Layout, VRenderFrame::OnLayout)
	EVT_MENU(ID_FullScreen, VRenderFrame::OnFullScreen)
	EVT_MENU(ID_OpenVolume, VRenderFrame::OnOpenVolume)
	EVT_MENU(ID_OpenMesh, VRenderFrame::OnOpenMesh)
	EVT_MENU(ID_ViewOrganize, VRenderFrame::OnOrganize)
	EVT_MENU(ID_CheckUpdates, VRenderFrame::OnCheckUpdates)
	EVT_MENU(ID_Info, VRenderFrame::OnInfo)
	EVT_MENU(ID_CreateCube, VRenderFrame::OnCreateCube)
	EVT_MENU(ID_CreateSphere, VRenderFrame::OnCreateSphere)
	EVT_MENU(ID_CreateCone, VRenderFrame::OnCreateCone)
	EVT_MENU(ID_SaveProject, VRenderFrame::OnSaveProject)
	EVT_MENU(ID_OpenProject, VRenderFrame::OnOpenProject)
	EVT_MENU(ID_Settings, VRenderFrame::OnSettings)
	EVT_MENU(ID_ImportVolume, VRenderFrame::OnImportVolume)
	//tools
	EVT_MENU(ID_LastTool, VRenderFrame::OnLastTool)
	EVT_MENU(ID_PaintTool, VRenderFrame::OnPaintTool)
	EVT_MENU(ID_Measure, VRenderFrame::OnMeasure)
	EVT_MENU(ID_Trace, VRenderFrame::OnTrace)
	EVT_MENU(ID_NoiseCancelling, VRenderFrame::OnNoiseCancelling)
	EVT_MENU(ID_Counting, VRenderFrame::OnCounting)
	EVT_MENU(ID_Colocalization, VRenderFrame::OnColocalization)
	EVT_MENU(ID_Convert, VRenderFrame::OnConvert)
	EVT_MENU(ID_Ocl, VRenderFrame::OnOcl)
	EVT_MENU(ID_Component, VRenderFrame::OnComponent)
	EVT_MENU(ID_Calculations, VRenderFrame::OnCalculations)
	//
	EVT_MENU(ID_Youtube, VRenderFrame::OnYoutube)
	EVT_MENU(ID_Twitter, VRenderFrame::OnTwitter)
	EVT_MENU(ID_Facebook, VRenderFrame::OnFacebook)
	EVT_MENU(ID_Manual, VRenderFrame::OnManual)
	EVT_MENU(ID_Tutorial, VRenderFrame::OnTutorial)
	EVT_MENU(ID_ShowHideUI, VRenderFrame::OnShowHideUI)
	EVT_MENU(ID_ShowHideToolbar, VRenderFrame::OnShowHideToolbar)
	//ui menu events
	EVT_MENU(ID_UIListView, VRenderFrame::OnShowHideView)
	EVT_MENU(ID_UITreeView, VRenderFrame::OnShowHideView)
	EVT_MENU(ID_UIMovieView, VRenderFrame::OnShowHideView)
	EVT_MENU(ID_UIAdjView, VRenderFrame::OnShowHideView)
	EVT_MENU(ID_UIClipView, VRenderFrame::OnShowHideView)
	EVT_MENU(ID_UIPropView, VRenderFrame::OnShowHideView)
	//panes
	EVT_AUI_PANE_CLOSE(VRenderFrame::OnPaneClose)
	//draw background
	EVT_PAINT(VRenderFrame::OnDraw)
	//process key event
	EVT_KEY_DOWN(VRenderFrame::OnKeyDown)
	//close
	EVT_CLOSE(VRenderFrame::OnClose)
END_EVENT_TABLE()

bool VRenderFrame::m_sliceSequence = false;
bool VRenderFrame::m_channSequence = false;
int VRenderFrame::m_digitOrder = 0;
bool VRenderFrame::m_compression = false;
bool VRenderFrame::m_skip_brick = false;
wxString VRenderFrame::m_time_id = "_T";
bool VRenderFrame::m_load_mask = true;
bool VRenderFrame::m_save_compress = true;
bool VRenderFrame::m_vrp_embed = false;
bool VRenderFrame::m_save_project = false;
bool VRenderFrame::m_save_alpha = false;
bool VRenderFrame::m_save_float = false;

VRenderFrame::VRenderFrame(
	wxFrame* frame,
	const wxString& title,
	int x, int y,
	int w, int h,
	bool benchmark,
	bool fullscreen,
	bool windowed,
	bool hidepanels)
	: wxFrame(frame, wxID_ANY, title, wxPoint(x, y), wxSize(w, h),wxDEFAULT_FRAME_STYLE),
	m_mov_view(0),
	m_movie_view(0),
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
	m_volume_prop(0),
	m_mesh_prop(0),
	m_mesh_manip(0),
	m_annotation_prop(0),
	m_ui_state(true),
	m_cur_sel_type(-1),
	m_cur_sel_vol(-1),
	m_cur_sel_mesh(-1),
	m_benchmark(benchmark),
	m_vd_copy(0),
	m_copy_data(false)
{
#ifdef _DARWIN
	SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#endif
	//create this first to read the settings
	m_setting_dlg = new SettingDlg(this, this);

	// tell wxAuiManager to manage this frame
	m_aui_mgr.SetManagedWindow(this);

	// set frame icon
	wxIcon icon;
	icon.CopyFromBitmap(wxGetBitmapFromMemory(icon_32));
	SetIcon(icon);

	// create the main toolbar
	m_main_tb = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxTB_FLAT|wxTB_TOP|wxTB_NODIVIDER);
	//create the menu for UI management
	m_tb_menu_ui = new wxMenu;
	m_tb_menu_ui->Append(ID_UIListView, UITEXT_DATAVIEW,
		"Show/hide the data list panel", wxITEM_CHECK);
	m_tb_menu_ui->Append(ID_UITreeView, UITEXT_TREEVIEW,
		"Show/hide the workspace panel", wxITEM_CHECK);
	m_tb_menu_ui->Append(ID_UIMovieView, UITEXT_MAKEMOVIE,
		"Show/hide the movie export panel", wxITEM_CHECK);
	m_tb_menu_ui->Append(ID_UIAdjView, UITEXT_ADJUST,
		"Show/hide the output adjustment panel", wxITEM_CHECK);
	m_tb_menu_ui->Append(ID_UIClipView, UITEXT_CLIPPING,
		"Show/hide the clipping plane control panel", wxITEM_CHECK);
	m_tb_menu_ui->Append(ID_UIPropView, UITEXT_PROPERTIES,
		"Show/hide the property panel", wxITEM_CHECK);
	//check all the items
	m_tb_menu_ui->Check(ID_UIListView, true);
	m_tb_menu_ui->Check(ID_UITreeView, true);
	m_tb_menu_ui->Check(ID_UIMovieView, true);
	m_tb_menu_ui->Check(ID_UIAdjView, true);
	m_tb_menu_ui->Check(ID_UIClipView, true);
	m_tb_menu_ui->Check(ID_UIPropView, true);
	//create the menu for edit/convert
	m_tb_menu_edit = new wxMenu;
	wxMenuItem *m = new wxMenuItem(m_tb_menu_edit, ID_PaintTool, wxT("Paint Brush..."));
	m->SetBitmap(wxGetBitmapFromMemory(icon_paint_brush_mini));
	m_tb_menu_edit->Append(m);
	m = new wxMenuItem(m_tb_menu_edit, ID_Measure, wxT("Measurement..."));
	m->SetBitmap(wxGetBitmapFromMemory(icon_measurement_mini));
	m_tb_menu_edit->Append(m);
	m = new wxMenuItem(m_tb_menu_edit, ID_Component, wxT("Component Analyzer..."));
	m->SetBitmap(wxGetBitmapFromMemory(icon_components_mini));
	m_tb_menu_edit->Append(m);
	m = new wxMenuItem(m_tb_menu_edit, ID_Trace, wxT("Tracking..."));
	m->SetBitmap(wxGetBitmapFromMemory(icon_tracking_mini));
	m_tb_menu_edit->Append(m);
	m = new wxMenuItem(m_tb_menu_edit, ID_Calculations, wxT("Calculations..."));
	m->SetBitmap(wxGetBitmapFromMemory(icon_calculations_mini));
	m_tb_menu_edit->Append(m);
	m = new wxMenuItem(m_tb_menu_edit, ID_NoiseCancelling, wxT("Noise Reduction..."));
	m->SetBitmap(wxGetBitmapFromMemory(icon_noise_reduc_mini));
	m_tb_menu_edit->Append(m);
	m = new wxMenuItem(m_tb_menu_edit, ID_Counting, wxT("Volume Size..."));
	m->SetBitmap(wxGetBitmapFromMemory(icon_volume_size_mini));
	m_tb_menu_edit->Append(m);
	m = new wxMenuItem(m_tb_menu_edit, ID_Colocalization, wxT("Colocalization..."));
	m->SetBitmap(wxGetBitmapFromMemory(icon_colocalization_mini));
	m_tb_menu_edit->Append(m);
	m = new wxMenuItem(m_tb_menu_edit, ID_Convert, wxT("Convert..."));
	m->SetBitmap(wxGetBitmapFromMemory(icon_convert_mini));
	m_tb_menu_edit->Append(m);
	m = new wxMenuItem(m_tb_menu_edit, ID_Ocl, wxT("OpenCL Kernel Editor..."));
	m->SetBitmap(wxGetBitmapFromMemory(icon_opencl_mini));
	m_tb_menu_edit->Append(m);
	//build the main toolbar
	//add tools
	wxBitmap bitmap;

	bitmap = wxGetBitmapFromMemory(icon_open_volume);
#ifdef _DARWIN
	m_main_tb->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_main_tb->AddTool(ID_OpenVolume, "Open Volume",
		bitmap, wxNullBitmap, wxITEM_NORMAL,
		"Open single or multiple volume data file(s)",
		"Open single or multiple volume data file(s)");

	if (JVMInitializer::getInstance(m_setting_dlg->GetJvmArgs()) != nullptr) {
		bitmap = wxGetBitmapFromMemory(icon_import);
#ifdef _DARWIN
		m_main_tb->SetToolBitmapSize(bitmap.GetSize());
#endif
		m_main_tb->AddTool(ID_ImportVolume, "Import Volume",
			bitmap, wxNullBitmap, wxITEM_NORMAL,
			"Import single or multiple volume data file(s) using ImageJ",
			"Import single or multiple volume data file(s) using ImageJ");
		m_main_tb->AddSeparator();
	}

	bitmap = wxGetBitmapFromMemory(icon_open_project);
#ifdef _DARWIN
	m_main_tb->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_main_tb->AddTool(ID_OpenProject, "Open Project",
		bitmap, wxNullBitmap, wxITEM_NORMAL,
		"Open a saved project",
		"Open a saved project");
	bitmap = wxGetBitmapFromMemory(icon_save_project);
#ifdef _DARWIN
	m_main_tb->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_main_tb->AddTool(ID_SaveProject, "Save Project",
		bitmap, wxNullBitmap, wxITEM_NORMAL,
		"Save current work as a project",
		"Save current work as a project");
	m_main_tb->AddSeparator();
	bitmap = wxGetBitmapFromMemory(icon_new_view);
#ifdef _DARWIN
	m_main_tb->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_main_tb->AddTool(ID_ViewNew, "New View",
		bitmap, wxNullBitmap, wxITEM_NORMAL,
		"Create a new render viewport",
		"Create a new render viewport");
	bitmap = wxGetBitmapFromMemory(icon_show_hide_ui);
#ifdef _DARWIN
	m_main_tb->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_main_tb->AddTool(ID_ShowHideUI, "Show/Hide UI",
		bitmap, wxNullBitmap, wxITEM_DROPDOWN,
		"Show or hide all control panels",
		"Show or hide all control panels");
	m_main_tb->SetDropdownMenu(ID_ShowHideUI, m_tb_menu_ui);
	m_main_tb->AddSeparator();
	bitmap = wxGetBitmapFromMemory(icon_open_mesh);
#ifdef _DARWIN
	m_main_tb->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_main_tb->AddTool(ID_OpenMesh, "Open Mesh",
		bitmap, wxNullBitmap, wxITEM_NORMAL,
		"Open single or multiple mesh file(s)",
		"Open single or multiple mesh file(s)");
	bitmap = wxGetBitmapFromMemory(icon_paint_brush);
#ifdef _DARWIN
	m_main_tb->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_main_tb->AddTool(ID_LastTool, "Analyze",
		bitmap, wxNullBitmap,
		wxITEM_DROPDOWN,
		"Tools for analyzing selected channel",
		"Tools for analyzing selected channel");
	m_main_tb->SetDropdownMenu(ID_LastTool, m_tb_menu_edit);
	m_main_tb->AddSeparator();
	bitmap = wxGetBitmapFromMemory(icon_settings);
#ifdef _DARWIN
	m_main_tb->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_main_tb->AddTool(ID_Settings, "Settings",
		bitmap, wxNullBitmap, wxITEM_NORMAL,
		"Settings of FluoRender",
		"Settings of FluoRender");

	m_main_tb->AddStretchableSpace();
	m_tb_menu_update = new wxMenu;
	m = new wxMenuItem(m_tb_menu_update, ID_CheckUpdates, wxT("Check Updates..."));
	m->SetBitmap(wxGetBitmapFromMemory(icon_check_updates_mini));
	m_tb_menu_update->Append(m);
	m = new wxMenuItem(m_tb_menu_update, ID_Youtube, wxT("Video Tutorials..."));
	m->SetBitmap(wxGetBitmapFromMemory(icon_youtube_mini));
	m_tb_menu_update->Append(m);
	m = new wxMenuItem(m_tb_menu_update, ID_Facebook, wxT("Facebook..."));
	m->SetBitmap(wxGetBitmapFromMemory(icon_facebook_mini));
	m_tb_menu_update->Append(m);
	m = new wxMenuItem(m_tb_menu_update, ID_Twitter, wxT("Twitter..."));
	m->SetBitmap(wxGetBitmapFromMemory(icon_twitter_mini));
	m_tb_menu_update->Append(m);
	m = new wxMenuItem(m_tb_menu_update, ID_Info, wxT("About..."));
	m->SetBitmap(wxGetBitmapFromMemory(icon_about_mini));
	m_tb_menu_update->Append(m);
	//last item
	int num = rand() % 5;
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
		bitmap = wxGetBitmapFromMemory(icon_facebook);
		str1 = "Facebook";
		str2 = "FluoRender's facebook page";
		str3 = "FluoRender's facebook page (requires Internet connection)";
		item_id = ID_Facebook;
		break;
	case 3:
		bitmap = wxGetBitmapFromMemory(icon_twitter);
		str1 = "Twitter";
		str2 = "Follow FluoRender on Twitter";
		str3 = "Follow FluoRender on Twitter (requires Internet connection)";
		item_id = ID_Twitter;
		break;
	case 4:
	default:
		bitmap = wxGetBitmapFromMemory(icon_about);
		str1 = "About";
		str2 = "FluoRender information";
		str3 = "FluoRender information";
		item_id = ID_Info;
		break;
	}
#ifdef _DARWIN
	m_main_tb->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_main_tb->AddTool(item_id, str1,
		bitmap, wxNullBitmap, wxITEM_DROPDOWN,
		str2, str3);
	m_main_tb->SetDropdownMenu(item_id, m_tb_menu_update);

	m_main_tb->Realize();

	//create render view
	VRenderView *vrv = new VRenderView(this, this, wxID_ANY);
	vrv->InitView();
	m_vrv_list.push_back(vrv);

	//create list view
	m_list_panel = new ListPanel(this, this, wxID_ANY,
		wxDefaultPosition, wxSize(350, 300));

	//create tree view
	m_tree_panel = new TreePanel(this, this, wxID_ANY,
		wxDefaultPosition, wxSize(350, 300));

	//create movie view (sets the m_recorder_dlg)
	m_movie_view = new VMovieView(this, this, wxID_ANY,
		wxDefaultPosition, wxSize(350, 300));

	//create prop panel
	m_prop_panel = new wxPanel(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, 0, "PropPanel");
	//prop panel chidren
	m_prop_sizer = new wxBoxSizer(wxHORIZONTAL);
	m_volume_prop = new VPropView(this, m_prop_panel, wxID_ANY);
	m_mesh_prop = new MPropView(this, m_prop_panel, wxID_ANY);
	m_mesh_manip = new MManipulator(this, m_prop_panel, wxID_ANY);
	m_annotation_prop = new APropView(this, m_prop_panel, wxID_ANY);
	m_prop_panel->SetSizer(m_prop_sizer);
	m_prop_sizer->Add(m_volume_prop, 1, wxEXPAND, 0);
	m_prop_sizer->Add(m_mesh_prop, 1, wxEXPAND, 0);
	m_prop_sizer->Add(m_mesh_manip, 1, wxEXPAND, 0);
	m_prop_sizer->Add(m_annotation_prop, 1, wxEXPAND, 0);
	m_volume_prop->Show(false);
	m_mesh_prop->Show(false);
	m_mesh_manip->Show(false);
	m_annotation_prop->Show(false);

	//clipping view
	m_clip_view = new ClippingView(this, this, wxID_ANY,
		wxDefaultPosition, wxSize(130,700));
	m_clip_view->SetDataManager(&m_data_mgr);
	m_clip_view->SetPlaneMode(static_cast<PLANE_MODES>(
		m_setting_dlg->GetPlaneMode()));

	//adjust view
	m_adjust_view = new AdjustView(this, this, wxID_ANY,
		wxDefaultPosition, wxSize(130, 700));

	wxString font_file = m_setting_dlg->GetFontFile();
	wxString exePath = wxStandardPaths::Get().GetExecutablePath();
	exePath = wxPathOnly(exePath);
	if (font_file != "")
		font_file = exePath + GETSLASH() + "Fonts" +
			GETSLASH() + font_file;
	else
		font_file = exePath + GETSLASH() + "Fonts" +
			GETSLASH() + "FreeSans.ttf";
	TextRenderer::text_texture_manager_.load_face(font_file.ToStdString());
	TextRenderer::text_texture_manager_.SetSize(m_setting_dlg->GetTextSize());

	//settings dialog
	if (m_setting_dlg->GetTestMode(1))
		m_vrv_list[0]->m_glview->m_test_speed = true;
	if (m_setting_dlg->GetTestMode(3))
	{
		m_vrv_list[0]->m_glview->m_test_wiref = true;
		m_vrv_list[0]->m_glview->m_draw_bounds = true;
		m_vrv_list[0]->m_glview->m_draw_grid = true;
		m_data_mgr.m_vol_test_wiref = true;
	}
	int c1 = m_setting_dlg->GetWavelengthColor(1);
	int c2 = m_setting_dlg->GetWavelengthColor(2);
	int c3 = m_setting_dlg->GetWavelengthColor(3);
	int c4 = m_setting_dlg->GetWavelengthColor(4);
	if (c1 && c2 && c3 && c4)
		m_data_mgr.SetWavelengthColor(c1, c2, c3, c4);
	m_vrv_list[0]->SetPeelingLayers(m_setting_dlg->GetPeelingLyers());
	m_vrv_list[0]->SetBlendSlices(m_setting_dlg->GetMicroBlend());
	m_vrv_list[0]->SetAdaptive(m_setting_dlg->GetMouseInt());
	m_vrv_list[0]->SetGradBg(m_setting_dlg->GetGradBg());
	m_vrv_list[0]->SetPinThreshold(m_setting_dlg->GetPinThreshold());
	m_vrv_list[0]->SetPointVolumeMode(m_setting_dlg->GetPointVolumeMode());
	m_vrv_list[0]->SetRulerUseTransf(m_setting_dlg->GetRulerUseTransf());
	m_vrv_list[0]->SetRulerTimeDep(m_setting_dlg->GetRulerTimeDep());
	m_vrv_list[0]->SetStereo(m_setting_dlg->GetStereo());
	m_vrv_list[0]->SetEyeDist(m_setting_dlg->GetEyeDist());
	if (m_setting_dlg->GetStereo()) m_vrv_list[0]->InitOpenVR();
	m_time_id = m_setting_dlg->GetTimeId();
	m_data_mgr.SetOverrideVox(m_setting_dlg->GetOverrideVox());
	m_data_mgr.SetPvxmlFlipX(m_setting_dlg->GetPvxmlFlipX());
	m_data_mgr.SetPvxmlFlipY(m_setting_dlg->GetPvxmlFlipY());
	VolumeRenderer::set_soft_threshold(m_setting_dlg->GetSoftThreshold());
	MultiVolumeRenderer::set_soft_threshold(m_setting_dlg->GetSoftThreshold());
	TreeLayer::SetSoftThreshsold(m_setting_dlg->GetSoftThreshold());
	VolumeMeshConv::SetSoftThreshold(m_setting_dlg->GetSoftThreshold());

	//brush tool dialog
	m_brush_tool_dlg = new BrushToolDlg(this, this);

	//noise cancelling dialog
	m_noise_cancelling_dlg = new NoiseCancellingDlg(this, this);

	//counting dialog
	m_counting_dlg = new CountingDlg(this, this);

	//convert dialog
	m_convert_dlg = new ConvertDlg(this, this);

	//colocalization dialog
	m_colocalization_dlg = new ColocalizationDlg(this, this);

	//measure dialog
	m_measure_dlg = new MeasureDlg(this, this);

	//trace dialog
	m_trace_dlg = new TraceDlg(this, this);
	m_trace_dlg->SetCellSize(m_setting_dlg->GetComponentSize());

	//ocl dialog
	m_ocl_dlg = new OclDlg(this, this);

	//component dialog
	m_component_dlg = new ComponentDlg(this, this);

	//calculation dialog
	m_calculation_dlg = new CalculationDlg(this, this);

	//help dialog
	m_help_dlg = new HelpDlg(this, this);
	//m_help_dlg->LoadPage("C:\\!wanyong!\\TEMP\\wxHtmlWindow.htm");

	//tester
	//shown for testing parameters
	m_tester = new TesterDlg(this, this);
	if (m_setting_dlg->GetTestMode(2))
		m_tester->Show(true);
	else
		m_tester->Show(false);

	//Add to the manager
	m_aui_mgr.AddPane(m_main_tb, wxAuiPaneInfo().
		Name("m_main_tb").Caption("Toolbar").CaptionVisible(false).
		MinSize(wxSize(-1, 49)).MaxSize(wxSize(-1, 50)).
		Top().CloseButton(false).Layer(4));
	m_aui_mgr.AddPane(m_list_panel, wxAuiPaneInfo().
		Name("m_list_panel").Caption(UITEXT_DATAVIEW).
		Left().CloseButton(true).BestSize(wxSize(350, 280)).
		FloatingSize(wxSize(350, 300)).Layer(3));
	m_aui_mgr.AddPane(m_tree_panel, wxAuiPaneInfo().
		Name("m_tree_panel").Caption(UITEXT_TREEVIEW).
		Left().CloseButton(true).BestSize(wxSize(350, 300)).
		FloatingSize(wxSize(350, 300)).Layer(3));
	m_aui_mgr.AddPane(m_movie_view, wxAuiPaneInfo().
		Name("m_movie_view").Caption(UITEXT_MAKEMOVIE).
		Left().CloseButton(true).BestSize(wxSize(350, 320)).
		FloatingSize(wxSize(350, 300)).Layer(3));
	m_aui_mgr.AddPane(m_prop_panel, wxAuiPaneInfo().
		Name("m_prop_panel").Caption(UITEXT_PROPERTIES).
		Bottom().CloseButton(true).MinSize(wxSize(300, 130)).
		FloatingSize(wxSize(1100, 130)).Layer(2));
	m_aui_mgr.AddPane(m_adjust_view, wxAuiPaneInfo().
		Name("m_adjust_view").Caption(UITEXT_ADJUST).
		Left().CloseButton(true).MinSize(wxSize(110, 700)).
		FloatingSize(wxSize(110, 700)).Layer(1));
	m_aui_mgr.AddPane(m_clip_view, wxAuiPaneInfo().
		Name("m_clip_view").Caption(UITEXT_CLIPPING).
		Right().CloseButton(true).MinSize(wxSize(130, 700)).
		FloatingSize(wxSize(130, 700)).Layer(1));
	m_aui_mgr.AddPane(vrv, wxAuiPaneInfo().
		Name(vrv->GetName()).Caption(vrv->GetName()).
		Dockable(true).CloseButton(false).
		FloatingSize(wxSize(600, 400)).MinSize(wxSize(300, 200)).
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
	//settings
	m_aui_mgr.AddPane(m_setting_dlg, wxAuiPaneInfo().
		Name("m_setting_dlg").Caption("Settings").
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


	UpdateTree();

	SetMinSize(wxSize(800,600));

	m_aui_mgr.Update();
	
	if (!windowed)
		Maximize();

	if (hidepanels)
	{
		m_ui_state = true;
		ToggleAllTools(false);
	}

	//make movie settings
	m_mov_view = 0;
	m_mov_axis = 1;
	m_mov_rewind = false;

	//set view default settings
	if (m_adjust_view && vrv)
	{
		//Color gamma, brightness, hdr;
		//bool sync_r, sync_g, sync_b;
		//m_adjust_view->GetDefaults(gamma, brightness, hdr,
		//	sync_r, sync_g, sync_b);
		//vrv->m_glview->SetGamma(gamma);
		//vrv->m_glview->SetBrightness(brightness);
		//vrv->m_glview->SetHdr(hdr);
		vrv->m_glview->SetSyncR(true);
		vrv->m_glview->SetSyncG(true);
		vrv->m_glview->SetSyncB(true);
	}

	//drop target
	SetDropTarget(new DnDFile(this));

#if wxUSE_STATUSBAR
	CreateStatusBar(2);
	GetStatusBar()->SetStatusText(wxString(FLUORENDER_TITLE)+
		wxString(" started normally."));
#endif // wxUSE_STATUSBAR

	//main top menu
	m_top_menu = new wxMenuBar;
	m_top_file = new wxMenu;
	m_top_tools = new wxMenu;
	m_top_window = new wxMenu;
	m_top_help = new wxMenu;

	//file options
	m = new wxMenuItem(m_top_file,ID_OpenVolume, wxT("Open &Volume"));
	m->SetBitmap(wxGetBitmapFromMemory(icon_open_volume_mini));
	m_top_file->Append(m);

	if (JVMInitializer::getInstance(m_setting_dlg->GetJvmArgs()) != nullptr) {
		m = new wxMenuItem(m_top_file, ID_ImportVolume, wxT("Import &Volume"));
		m->SetBitmap(wxGetBitmapFromMemory(icon_import_mini));
		m_top_file->Append(m);
	}	

	m = new wxMenuItem(m_top_file,ID_OpenMesh, wxT("Open &Mesh"));
	m->SetBitmap(wxGetBitmapFromMemory(icon_open_mesh_mini));
	m_top_file->Append(m);

	m = new wxMenuItem(m_top_file,ID_OpenProject, wxT("Open &Project"));
	m->SetBitmap(wxGetBitmapFromMemory(icon_open_project_mini));
	m_top_file->Append(m);

	m = new wxMenuItem(m_top_file,ID_SaveProject, wxT("&Save Project"));
	m->SetBitmap(wxGetBitmapFromMemory(icon_save_project_mini));
	m_top_file->Append(m);

	m_top_file->Append(wxID_SEPARATOR);
	wxMenuItem *quit = new wxMenuItem(m_top_file, wxID_EXIT);
	quit->SetBitmap(wxArtProvider::GetBitmap(wxART_QUIT));
	m_top_file->Append(quit);
	//tool options
	m = new wxMenuItem(m_top_tools,ID_PaintTool, wxT("&Paint Brush..."));
	m->SetBitmap(wxGetBitmapFromMemory(icon_paint_brush_mini));
	m_top_tools->Append(m);
	m = new wxMenuItem(m_top_tools,ID_Measure, wxT("&Measurement..."));
	m->SetBitmap(wxGetBitmapFromMemory(icon_measurement_mini));
	m_top_tools->Append(m);
	m = new wxMenuItem(m_top_tools, ID_Component, wxT("Component &Analyzer..."));
	m->SetBitmap(wxGetBitmapFromMemory(icon_components_mini));
	m_top_tools->Append(m);
	m = new wxMenuItem(m_top_tools,ID_Trace, wxT("&Tracking..."));
	m->SetBitmap(wxGetBitmapFromMemory(icon_tracking_mini));
	m_top_tools->Append(m);
	m = new wxMenuItem(m_top_tools, ID_Calculations, wxT("Ca&lculations..."));
	m->SetBitmap(wxGetBitmapFromMemory(icon_calculations_mini));
	m_top_tools->Append(m);
	m = new wxMenuItem(m_top_tools,ID_NoiseCancelling, wxT("Noise &Reduction..."));
	m->SetBitmap(wxGetBitmapFromMemory(icon_noise_reduc_mini));
	m_top_tools->Append(m);
	m = new wxMenuItem(m_top_tools,ID_Counting, wxT("&Volume Size..."));
	m->SetBitmap(wxGetBitmapFromMemory(icon_volume_size_mini));
	m_top_tools->Append(m);
	m = new wxMenuItem(m_top_tools,ID_Colocalization, wxT("&Colocalization..."));
	m->SetBitmap(wxGetBitmapFromMemory(icon_colocalization_mini));
	m_top_tools->Append(m);
	m = new wxMenuItem(m_top_tools,ID_Convert, wxT("Co&nvert..."));
	m->SetBitmap(wxGetBitmapFromMemory(icon_convert_mini));
	m_top_tools->Append(m);
	m = new wxMenuItem(m_top_tools, ID_Ocl, wxT("&OpenCL Kernel Editor..."));
	m->SetBitmap(wxGetBitmapFromMemory(icon_opencl_mini));
	m_top_tools->Append(m);
	m_top_tools->Append(wxID_SEPARATOR);
	m = new wxMenuItem(m_top_tools,ID_Settings, wxT("&Settings..."));
	m->SetBitmap(wxGetBitmapFromMemory(icon_settings_mini));
	m_top_tools->Append(m);
	//window option
	m = new wxMenuItem(m_top_window,ID_ShowHideToolbar, wxT("Show/Hide &Toolbar"), wxEmptyString, wxITEM_CHECK);
	m_top_window->Append(m);
	m_top_window->Check(ID_ShowHideToolbar, true);
	m_top_window->Append(wxID_SEPARATOR);
	m = new wxMenuItem(m_top_window,ID_ShowHideUI, wxT("Show/Hide &UI"));
	m->SetBitmap(wxGetBitmapFromMemory(icon_show_hide_ui_mini));
	m_top_window->Append(m);
	m = new wxMenuItem(m_top_window,ID_UIListView, wxT("&Datasets"), wxEmptyString, wxITEM_CHECK);
	m_top_window->Append(m);
	m_top_window->Check(ID_UIListView, true);
	m = new wxMenuItem(m_top_window,ID_UITreeView, wxT("&Workspace"), wxEmptyString, wxITEM_CHECK);
	m_top_window->Append(m);
	m_top_window->Check(ID_UITreeView, true);
	m = new wxMenuItem(m_top_window,ID_UIMovieView, wxT("&Export"), wxEmptyString, wxITEM_CHECK);
	m_top_window->Append(m);
	m_top_window->Check(ID_UIMovieView, true);
	m = new wxMenuItem(m_top_window,ID_UIAdjView, wxT("&Output Adjustments"), wxEmptyString, wxITEM_CHECK);
	m_top_window->Append(m);
	m_top_window->Check(ID_UIAdjView, true);
	m = new wxMenuItem(m_top_window,ID_UIClipView, wxT("&Clipping Planes"), wxEmptyString, wxITEM_CHECK);
	m_top_window->Append(m);
	m_top_window->Check(ID_UIClipView, true);
	m = new wxMenuItem(m_top_window,ID_UIPropView, wxT("&Properties"), wxEmptyString, wxITEM_CHECK);
	m_top_window->Append(m);
	m_top_window->Check(ID_UIPropView, true);
	m_top_window->Append(wxID_SEPARATOR);
	m = new wxMenuItem(m_top_window, ID_Layout, wxT("Layout"));
	m_top_window->Append(m);
	m = new wxMenuItem(m_top_window, ID_ViewNew, wxT("&New View"));
	m->SetBitmap(wxGetBitmapFromMemory(icon_new_view_mini));
	m_top_window->Append(m);
#ifndef _DARWIN
	m = new wxMenuItem(m_top_window, ID_FullScreen, wxT("&Full Screen"));
	m->SetBitmap(wxGetBitmapFromMemory(full_screen_menu));
	m_top_window->Append(m);
#endif
	//help menu
	m = new wxMenuItem(m_top_help,ID_CheckUpdates, wxT("&Check for Updates"));
	m->SetBitmap(wxGetBitmapFromMemory(icon_check_updates_mini));
	m_top_help->Append(m);
	m = new wxMenuItem(m_top_help, ID_Youtube, wxT("&Video Tutorials"));
	m->SetBitmap(wxGetBitmapFromMemory(icon_youtube_mini));
	m_top_help->Append(m);
	m = new wxMenuItem(m_top_help,ID_Twitter, wxT("&Twitter"));
	m->SetBitmap(wxGetBitmapFromMemory(icon_twitter_mini));
	m_top_help->Append(m);
	m = new wxMenuItem(m_top_help,ID_Facebook, wxT("&Facebook"));
	m->SetBitmap(wxGetBitmapFromMemory(icon_facebook_mini));
	m_top_help->Append(m);
	m = new wxMenuItem(m_top_help,ID_Manual, wxT("&Online Manual"));
	m->SetBitmap(wxGetBitmapFromMemory(web_pdf_mini));
	m_top_help->Append(m);
	m = new wxMenuItem(m_top_help,ID_Tutorial, wxT("Online T&utorials"));
	m->SetBitmap(wxGetBitmapFromMemory(web_pdf_mini));
	m_top_help->Append(m);
	m = new wxMenuItem(m_top_help,ID_Info, wxT("&About FluoRender..."));
	m->SetBitmap(wxGetBitmapFromMemory(icon_about_mini));
	m_top_help->Append(m);
	//add the menus
	m_top_menu->Append(m_top_file,wxT("&File"));
	m_top_menu->Append(m_top_tools,wxT("&Tools"));
	m_top_menu->Append(m_top_window,wxT("&Windows"));
	m_top_menu->Append(m_top_help,wxT("&Help"));
	SetMenuBar(m_top_menu);

	//set analyze icon
	if (m_setting_dlg)
	{
		switch (m_setting_dlg->GetLastTool())
		{
		case TOOL_PAINT_BRUSH:
		default:
			m_main_tb->SetToolNormalBitmap(ID_LastTool,
				wxGetBitmapFromMemory(icon_paint_brush));
			break;
		case TOOL_MEASUREMENT:
			m_main_tb->SetToolNormalBitmap(ID_LastTool,
				wxGetBitmapFromMemory(icon_measurement));
			break;
		case TOOL_TRACKING:
			m_main_tb->SetToolNormalBitmap(ID_LastTool,
				wxGetBitmapFromMemory(icon_tracking));
			break;
		case TOOL_NOISE_REDUCTION:
			m_main_tb->SetToolNormalBitmap(ID_LastTool,
				wxGetBitmapFromMemory(icon_noise_reduc));
			break;
		case TOOL_VOLUME_SIZE:
			m_main_tb->SetToolNormalBitmap(ID_LastTool,
				wxGetBitmapFromMemory(icon_volume_size));
			break;
		case TOOL_COLOCALIZATION:
			m_main_tb->SetToolNormalBitmap(ID_LastTool,
				wxGetBitmapFromMemory(icon_colocalization));
			break;
		case TOOL_CONVERT:
			m_main_tb->SetToolNormalBitmap(ID_LastTool,
				wxGetBitmapFromMemory(icon_convert));
			break;
		case TOOL_OPENCL:
			m_main_tb->SetToolNormalBitmap(ID_LastTool,
				wxGetBitmapFromMemory(icon_opencl));
			break;
		case TOOL_COMPONENT:
			m_main_tb->SetToolNormalBitmap(ID_LastTool,
				wxGetBitmapFromMemory(icon_components));
			break;
		case TOOL_CALCULATIONS:
			m_main_tb->SetToolNormalBitmap(ID_LastTool,
				wxGetBitmapFromMemory(icon_calculations));
			break;
		}
		SetSaveProject(m_setting_dlg->GetProjSave());
		SetSaveAlpha(m_setting_dlg->GetSaveAlpha());
		SetSaveFloat(m_setting_dlg->GetSaveFloat());
	}

	if (fullscreen)
	{
		vrv->SetFullScreen();
		Iconize();
	}

	wxMemorySize free_mem_size = wxGetFreeMemory();
	double mainmem_buf_size = free_mem_size.ToDouble() * 0.8 / 1024.0 / 1024.0;
	if (mainmem_buf_size > TextureRenderer::get_mainmem_buf_size())
		TextureRenderer::set_mainmem_buf_size(mainmem_buf_size);
}

VRenderFrame::~VRenderFrame()
{
	//release?
	TextureRenderer::vol_kernel_factory_.clear();
	TextureRenderer::framebuffer_manager_.clear();
	TextureRenderer::vertex_array_manager_.clear();
	TextureRenderer::vol_shader_factory_.clear();
	TextureRenderer::seg_shader_factory_.clear();
	TextureRenderer::cal_shader_factory_.clear();
	TextureRenderer::img_shader_factory_.clear();
	TextRenderer::text_texture_manager_.clear();
	for (int i=0; i<(int)m_vrv_list.size(); i++)
	{
		VRenderView* vrv = m_vrv_list[i];
		if (vrv) vrv->Clear();
	}
	m_aui_mgr.UnInit();
	KernelProgram::release();
}

void VRenderFrame::OnExit(wxCommandEvent& WXUNUSED(event))
{
	Close(true);
}

void VRenderFrame::OnClose(wxCloseEvent &event)
{
	m_setting_dlg->SaveSettings();
	bool vrv_saved = false;
	for (unsigned int i=0; i<m_vrv_list.size(); ++i)
	{
		if (m_vrv_list[i]->m_default_saved)
		{
			vrv_saved = true;
			break;
		}
	}
	if (!vrv_saved && !m_vrv_list.empty())
		m_vrv_list[0]->SaveDefault(0xaff);
	event.Skip();
}

wxString VRenderFrame::CreateView(int row)
{
	VRenderView* vrv = 0;
	if (m_vrv_list.size()>0)
	{
		wxGLContext* sharedContext = m_vrv_list[0]->GetContext();
		vrv = new VRenderView(this, this, wxID_ANY, sharedContext);
	}
	else
	{
		vrv = new VRenderView(this, this, wxID_ANY);
	}

	if (vrv)
	{
		m_vrv_list.push_back(vrv);
		if (m_movie_view)
			m_movie_view->AddView(vrv->GetName());
		if (m_setting_dlg->GetTestMode(3))
		{
			vrv->m_glview->m_test_wiref = true;
			vrv->m_glview->m_draw_bounds = true;
			vrv->m_glview->m_draw_grid = true;
		}
		vrv->SetPeelingLayers(m_setting_dlg->GetPeelingLyers());
		vrv->SetBlendSlices(m_setting_dlg->GetMicroBlend());
		vrv->SetAdaptive(m_setting_dlg->GetMouseInt());
		vrv->SetGradBg(m_setting_dlg->GetGradBg());
		vrv->SetPinThreshold(m_setting_dlg->GetPinThreshold());
		vrv->SetPointVolumeMode(m_setting_dlg->GetPointVolumeMode());
		vrv->SetRulerUseTransf(m_setting_dlg->GetRulerUseTransf());
		vrv->SetRulerTimeDep(m_setting_dlg->GetRulerTimeDep());
	}

	//reset gl
	for (int i = 0; i < m_vrv_list.size(); ++i)
	{
		if (m_vrv_list[i])
			m_vrv_list[i]->m_glview->m_set_gl = false;
	}

	//m_aui_mgr.Update();
	OrganizeVRenderViews(1);

	//set view default settings
	if (m_adjust_view && vrv)
	{
		/*Color gamma, brightness, hdr;
		bool sync_r, sync_g, sync_b;
		m_adjust_view->GetDefaults(gamma, brightness, hdr, sync_r, sync_g, sync_b);
		vrv->m_glview->SetGamma(gamma);
		vrv->m_glview->SetBrightness(brightness);
		vrv->m_glview->SetHdr(hdr);*/
		vrv->m_glview->SetSyncR(true);
		vrv->m_glview->SetSyncG(true);
		vrv->m_glview->SetSyncB(true);
	}

	//add volumes
	if (m_vrv_list.size() > 0 && vrv)
	{
		VRenderView* vrv0 = m_vrv_list[0];
		if (vrv0)
		{
			for (int i = 0; i < vrv0->GetDispVolumeNum(); ++i)
			{
				VolumeData* vd = vrv0->GetDispVolumeData(i);
				if (vd)
				{
					VolumeData* vd_add = m_data_mgr.DuplicateVolumeData(vd);

					if (vd_add)
					{
						int chan_num = vrv->GetAny();
						Color color(1.0, 1.0, 1.0);
						if (chan_num == 0)
							color = Color(1.0, 0.0, 0.0);
						else if (chan_num == 1)
							color = Color(0.0, 1.0, 0.0);
						else if (chan_num == 2)
							color = Color(0.0, 0.0, 1.0);

						if (chan_num >= 0 && chan_num < 3)
							vd_add->SetColor(color);

						vrv->AddVolumeData(vd_add);
					}
				}
			}
		}
		//update
		vrv->InitView(INIT_BOUNDS | INIT_CENTER | INIT_TRANSL | INIT_ROTATE);
	}

	UpdateTree();

	if (vrv)
		return vrv->GetName();
	else
		return wxString("NO_NAME");
}

//views
int VRenderFrame::GetViewNum()
{
	return m_vrv_list.size();
}

vector <VRenderView*>* VRenderFrame::GetViewList()
{
	return &m_vrv_list;
}

VRenderView* VRenderFrame::GetView(int index)
{
	if (index>=0 && index<(int)m_vrv_list.size())
		return m_vrv_list[index];
	else
		return 0;
}

VRenderView* VRenderFrame::GetView(wxString& name)
{
	for (int i=0; i<(int)m_vrv_list.size(); i++)
	{
		VRenderView* vrv = m_vrv_list[i];
		if (vrv && vrv->GetName() == name)
		{
			return vrv;
		}
	}
	return 0;
}

void VRenderFrame::OnNewView(wxCommandEvent& WXUNUSED(event))
{
	wxString str = CreateView();
}

void VRenderFrame::OnLayout(wxCommandEvent& WXUNUSED(event))
{
	OrganizeVRenderViews(1);
}

void VRenderFrame::OnFullScreen(wxCommandEvent& WXUNUSED(event))
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
void VRenderFrame::OnCh11Check(wxCommandEvent &event)
{
	wxCheckBox* ch11 = (wxCheckBox*)event.GetEventObject();
	if (ch11)
		m_sliceSequence = ch11->GetValue();
}

void VRenderFrame::OnCh12Check(wxCommandEvent &event)
{
	wxCheckBox* ch12 = (wxCheckBox*)event.GetEventObject();
	if (ch12)
		m_channSequence = ch12->GetValue();
}

void VRenderFrame::OnCmbChange(wxCommandEvent &event)
{
	wxComboBox* combo = (wxComboBox*)event.GetEventObject();
	if (combo)
		m_digitOrder = combo->GetSelection();
}

void VRenderFrame::OnTxt1Change(wxCommandEvent &event)
{
	wxTextCtrl* txt1 = (wxTextCtrl*)event.GetEventObject();
	if (txt1)
		m_time_id = txt1->GetValue();
}

void VRenderFrame::OnCh2Check(wxCommandEvent &event)
{
	wxCheckBox* ch2 = (wxCheckBox*)event.GetEventObject();
	if (ch2)
		m_compression = ch2->GetValue();
}

void VRenderFrame::OnCh3Check(wxCommandEvent &event)
{
	wxCheckBox* ch3 = (wxCheckBox*)event.GetEventObject();
	if (ch3)
		m_skip_brick = ch3->GetValue();
}

wxWindow* VRenderFrame::CreateExtraControlVolume(wxWindow* parent)
{
	wxPanel* panel = new wxPanel(parent, 0, wxDefaultPosition, wxSize(640, 110));
#ifdef _DARWIN
	panel->SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#endif
	wxStaticText* st;

	wxBoxSizer *group1 = new wxStaticBoxSizer(
		new wxStaticBox(panel, wxID_ANY, "Additional Options"), wxVERTICAL );

	//slice sequence check box
	wxCheckBox* ch11 = new wxCheckBox(panel, ID_READ_ZSLICE,
		"Read file# as Z sections");
	ch11->Connect(ch11->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(VRenderFrame::OnCh11Check), NULL, panel);
	ch11->SetValue(m_sliceSequence);

	//slice sequence check box
	wxCheckBox* ch12 = new wxCheckBox(panel, ID_READ_CHANN,
		"Read file# as channels");
	ch12->Connect(ch12->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(VRenderFrame::OnCh12Check), NULL, panel);
	ch12->SetValue(m_channSequence);

	//digit order
	wxComboBox* combo = new wxComboBox(panel, ID_DIGI_ORDER,
		"Order", wxDefaultPosition, wxSize(-1, 10), 0, NULL, wxCB_READONLY);
	combo->Connect(combo->GetId(), wxEVT_COMMAND_COMBOBOX_SELECTED,
		wxCommandEventHandler(VRenderFrame::OnCmbChange), NULL, panel);
	std::vector<std::string> combo_list;
	combo_list.push_back("Channel first");
	combo_list.push_back("Z section first");
	for (size_t i = 0; i < combo_list.size(); ++i)
		combo->Append(combo_list[i]);
	combo->SetSelection(m_digitOrder);

	wxBoxSizer* sizer1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(panel, 0,
		"Digit order:");
	sizer1->Add(ch11);
	sizer1->Add(10, 10);
	sizer1->Add(ch12);
	sizer1->Add(10, 10);
	sizer1->Add(st);
	sizer1->Add(5, 5);
	sizer1->Add(combo, 0, wxALIGN_TOP);

	//compression
	wxCheckBox* ch2 = new wxCheckBox(panel, ID_COMPRESS,
		"Compress data (loading will take longer time and data are compressed in graphics memory)");
	ch2->Connect(ch2->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(VRenderFrame::OnCh2Check), NULL, panel);
	ch2->SetValue(m_compression);

	//empty brick skipping
	wxCheckBox* ch3 = new wxCheckBox(panel, ID_SKIP_BRICKS,
		"Skip empty bricks during rendering (loading takes longer time)");
	ch3->Connect(ch3->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(VRenderFrame::OnCh3Check), NULL, panel);
	ch3->SetValue(m_skip_brick);

	//time sequence identifier
	wxBoxSizer* sizer2 = new wxBoxSizer(wxHORIZONTAL);
	wxTextCtrl* txt1 = new wxTextCtrl(panel, ID_TSEQ_ID,
		"", wxDefaultPosition, wxSize(80, 20));
	txt1->SetValue(m_time_id);
	txt1->Connect(txt1->GetId(), wxEVT_COMMAND_TEXT_UPDATED,
		wxCommandEventHandler(VRenderFrame::OnTxt1Change), NULL, panel);
	st = new wxStaticText(panel, 0,
		"Time sequence identifier (digits after the identifier in filenames are used as time index)");
	sizer2->Add(txt1);
	sizer2->Add(10, 10);
	sizer2->Add(st);

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

wxWindow* VRenderFrame::CreateExtraControlVolumeForImport(wxWindow* parent)
{
	wxPanel* panel = new wxPanel(parent, 0, wxDefaultPosition, wxSize(640, 110));
#ifdef _DARWIN
	panel->SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#endif
	wxBoxSizer *group1 = new wxStaticBoxSizer(
		new wxStaticBox(panel, wxID_ANY, "Additional Options"), wxVERTICAL);

	//slice sequence check box. TODO: Not suppotred as of now.
	/*
	wxCheckBox* ch1 = new wxCheckBox(panel, ID_READ_ZSLICES,
		"Read a sequence as Z slices (the last digits in filenames are used to identify the sequence)");
	ch1->Connect(ch1->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(VRenderFrame::OnCh1Check), NULL, panel);
	ch1->SetValue(m_sliceSequence);
	*/

	//compression
	wxCheckBox* ch2 = new wxCheckBox(panel, ID_COMPRESS,
		"Compress data (loading will take longer time and data are compressed in graphics memory)");
	ch2->Connect(ch2->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(VRenderFrame::OnCh2Check), NULL, panel);
	ch2->SetValue(m_compression);

	//empty brick skipping
	wxCheckBox* ch3 = new wxCheckBox(panel, ID_SKIP_BRICKS,
		"Skip empty bricks during rendering (loading takes longer time)");
	ch3->Connect(ch3->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(VRenderFrame::OnCh3Check), NULL, panel);
	ch3->SetValue(m_skip_brick);

	//time sequence identifier. TODO: Not supported as of now.
	/*
	wxBoxSizer* sizer1 = new wxBoxSizer(wxHORIZONTAL);
	wxTextCtrl* txt1 = new wxTextCtrl(panel, ID_TSEQ_ID,
		"", wxDefaultPosition, wxSize(80, 20));
	txt1->SetValue(m_time_id);
	txt1->Connect(txt1->GetId(), wxEVT_COMMAND_TEXT_UPDATED,
		wxCommandEventHandler(VRenderFrame::OnTxt1Change), NULL, panel);
	wxStaticText* st = new wxStaticText(panel, 0,
		"Time sequence identifier (digits after the identifier in filenames are used as time index)");
	sizer1->Add(txt1);
	sizer1->Add(10, 10);
	sizer1->Add(st);
	*/

	//group1->Add(10, 10);
	//group1->Add(ch1);
	group1->Add(10, 10);
	group1->Add(ch2);
	group1->Add(10, 10);
	group1->Add(ch3);
	//group1->Add(10, 10);
	//group1->Add(sizer1);
	//group1->Add(10, 10);

	panel->SetSizerAndFit(group1);
	panel->Layout();

	return panel;
}

void VRenderFrame::OnOpenVolume(wxCommandEvent& WXUNUSED(event))
{
	if (m_setting_dlg)
	{
		m_compression = m_setting_dlg->GetRealtimeCompress();
		m_skip_brick = m_setting_dlg->GetSkipBricks();
	}

	wxFileDialog *fopendlg = new wxFileDialog(
		this, "Choose the volume data file", "", "",
#ifndef _DARWIN
		"All Supported|*.tif;*.tiff;*.oib;*.oif;*.lsm;*.czi;*.nd2;*.xml;*.nrrd;*.vvd|"\
		"Tiff Files (*.tif, *.tiff)|*.tif;*.tiff|"\
		"Olympus Image Binary Files (*.oib)|*.oib|"\
		"Olympus Original Imaging Format (*.oif)|*.oif|"\
		"Zeiss Laser Scanning Microscope (*.lsm)|*.lsm|"\
		"Zeiss ZISRAW File Format (*.czi)|*.czi|"\
		"Nikon ND2 File Format (*.nd2)|*.nd2|"\
		"Bruker/Prairie View XML (*.xml)|*.xml|"\
		"Utah Nrrd files (*.nrrd)|*.nrrd|"\
		"Janelia Brick files (*.vvd)|*.vvd",
#else
		"All Supported|*.tif;*.tiff;*.oib;*.oif;*.lsm;*.czi;*.xml;*.nrrd;*.vvd|"\
		"Tiff Files (*.tif, *.tiff)|*.tif;*.tiff|"\
		"Olympus Image Binary Files (*.oib)|*.oib|"\
		"Olympus Original Imaging Format (*.oif)|*.oif|"\
		"Zeiss Laser Scanning Microscope (*.lsm)|*.lsm|"\
		"Zeiss ZISRAW File Format (*.czi)|*.czi|"\
		"Bruker/Prairie View XML (*.xml)|*.xml|"\
		"Utah Nrrd files (*.nrrd)|*.nrrd|"\
		"Janelia Brick files (*.vvd)|*.vvd",
#endif
		wxFD_OPEN|wxFD_MULTIPLE);
	fopendlg->SetExtraControlCreator(CreateExtraControlVolume);

	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		VRenderView* vrv = GetView(0);

		wxArrayString paths;
		fopendlg->GetPaths(paths);
		LoadVolumes(paths, false, vrv);

		if (m_setting_dlg)
		{
			m_setting_dlg->SetRealtimeCompress(m_compression);
			m_setting_dlg->SetSkipBricks(m_skip_brick);
			m_setting_dlg->UpdateUI();
		}
	}

	delete fopendlg;
}

void VRenderFrame::OnImportVolume(wxCommandEvent& WXUNUSED(event))
{
	if (m_setting_dlg)
	{
		m_compression = m_setting_dlg->GetRealtimeCompress();
		m_skip_brick = m_setting_dlg->GetSkipBricks();
	}

	wxFileDialog *fopendlg = new wxFileDialog(
		this, "Choose the volume data file", "", "", "All Files|*.*",
		wxFD_OPEN | wxFD_MULTIPLE);
	fopendlg->SetExtraControlCreator(CreateExtraControlVolumeForImport);

	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		VRenderView* vrv = GetView(0);

		wxArrayString paths;
		fopendlg->GetPaths(paths);
		LoadVolumes(paths, true, vrv);

		if (m_setting_dlg)
		{
			m_setting_dlg->SetRealtimeCompress(m_compression);
			m_setting_dlg->SetSkipBricks(m_skip_brick);
			m_setting_dlg->UpdateUI();
		}
	}

	delete fopendlg;
}

void VRenderFrame::LoadVolumes(wxArrayString files, bool withImageJ, VRenderView* view)
{
	int j;

	VolumeData* vd_sel = 0;
	DataGroup* group_sel = 0;
	VRenderView* vrv = 0;

	if (view)
		vrv = view;
	else
		vrv = GetView(0);

	wxProgressDialog *prg_diag = 0;
	if (vrv)
	{
		bool streaming = m_setting_dlg->GetMemSwap();
		double gpu_size = m_setting_dlg->GetGraphicsMem();
		double data_size = m_setting_dlg->GetLargeDataSize();
		int brick_size = m_setting_dlg->GetForceBrickSize();
		int resp_time = m_setting_dlg->GetResponseTime();
		wxString str_streaming;
		if (streaming)
		{
			str_streaming = "Large data streaming is currently ON\n";
			str_streaming += wxString::Format("FluoRender will use %dMB GPU Memory\n", int(gpu_size));
			str_streaming += wxString::Format("Data channel larger than %dMB will be divided into bricks of %d voxels\n",
				int (data_size), brick_size);
			str_streaming += wxString::Format("System response time is %dms", resp_time);
		}
		else
			str_streaming = "Large data streaming is currently OFF";

		prg_diag = new wxProgressDialog(
			"FluoRender: Loading volume data...",
			"",
			100, 0, wxPD_SMOOTH|wxPD_ELAPSED_TIME|wxPD_AUTO_HIDE);

		m_data_mgr.SetSliceSequence(m_sliceSequence);
		m_data_mgr.SetChannSequence(m_channSequence);
		m_data_mgr.SetDigitOrder(m_digitOrder);
		m_data_mgr.SetCompression(m_compression);
		m_data_mgr.SetSkipBrick(m_skip_brick);
		m_data_mgr.SetTimeId(m_time_id);
		m_data_mgr.SetLoadMask(m_load_mask);
		m_setting_dlg->SetTimeId(m_time_id);

		bool enable_4d = false;

		for (j=0; j<(int)files.Count(); j++)
		{
			wxGetApp().Yield();
			prg_diag->Update(90*(j+1)/(int)files.Count(),
				str_streaming);

			int ch_num = 0;
			wxString filename = files[j];
			wxString suffix = filename.Mid(filename.Find('.', true)).MakeLower();

			if (withImageJ)
				ch_num = m_data_mgr.LoadVolumeData(filename, LOAD_TYPE_IMAGEJ, true); //The type of data doesnt matter.
			else if (suffix == ".nrrd" || suffix == ".msk" || suffix == ".lbl")
				ch_num = m_data_mgr.LoadVolumeData(filename, LOAD_TYPE_NRRD, false);
			else if (suffix == ".tif" || suffix == ".tiff")
				ch_num = m_data_mgr.LoadVolumeData(filename, LOAD_TYPE_TIFF, false);
			else if (suffix == ".oib")
				ch_num = m_data_mgr.LoadVolumeData(filename, LOAD_TYPE_OIB, false);
			else if (suffix == ".oif")
				ch_num = m_data_mgr.LoadVolumeData(filename, LOAD_TYPE_OIF, false);
			else if (suffix == ".lsm")
				ch_num = m_data_mgr.LoadVolumeData(filename, LOAD_TYPE_LSM, false);
			else if (suffix == ".xml")
				ch_num = m_data_mgr.LoadVolumeData(filename, LOAD_TYPE_PVXML, false);
			else if (suffix == ".vvd")
				ch_num = m_data_mgr.LoadVolumeData(filename, LOAD_TYPE_BRKXML, false);
			else if (suffix == ".czi")
				ch_num = m_data_mgr.LoadVolumeData(filename, LOAD_TYPE_CZI, false);
			else if (suffix == ".nd2")
				ch_num = m_data_mgr.LoadVolumeData(filename, LOAD_TYPE_ND2, false);

			if (ch_num > 1)
			{
				DataGroup* group = vrv->AddOrGetGroup();
				if (group)
				{
					for (int i=ch_num; i>0; i--)
					{
						VolumeData* vd = m_data_mgr.GetVolumeData(m_data_mgr.GetVolumeNum()-i);
						if (vd)
						{
							vrv->AddVolumeData(vd, group->GetName());
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
				VolumeData* vd = m_data_mgr.GetVolumeData(m_data_mgr.GetVolumeNum()-1);
				if (vd)
				{
					int chan_num = vrv->GetDispVolumeNum();
					Color color(1.0, 1.0, 1.0);
					if (chan_num == 0)
						color = Color(1.0, 0.0, 0.0);
					else if (chan_num == 1)
						color = Color(0.0, 1.0, 0.0);
					else if (chan_num == 2)
						color = Color(0.0, 0.0, 1.0);

					if (chan_num >=0 && chan_num <3)
						vd->SetColor(color);
					else
						vd->RandomizeColor();

					vrv->AddVolumeData(vd);
					vd_sel = vd;

					if (vd->GetReader() && vd->GetReader()->GetTimeNum()>1){
						vrv->m_glview->m_tseq_cur_num = vd->GetReader()->GetCurTime();
						enable_4d = true;
					}
				}
			}
			else { //TODO: Consult Wan here.

			}
		}

		UpdateList();
		if (vd_sel)
			UpdateTree(vd_sel->GetName());
		else
			UpdateTree();
		vrv->RefreshGL();

		vrv->InitView(INIT_BOUNDS|INIT_CENTER);
		vrv->UpdateScaleFactor(false);

		if (enable_4d) {
			m_movie_view->EnableTime();
			m_movie_view->DisableRot();
			m_movie_view->SetCurrentTime(vrv->m_glview->m_tseq_cur_num);
		}

		delete prg_diag;
	}

	vrv->RefreshGL();//added by Takashi
}

void VRenderFrame::StartupLoad(wxArrayString files, bool run_mov, bool with_imagej)
{
	if (m_vrv_list[0])
		m_vrv_list[0]->m_glview->Init();

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
			suffix == ".czi")
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

	if (run_mov && m_movie_view)
		m_movie_view->Run();
}

void VRenderFrame::LoadMeshes(wxArrayString files, VRenderView* vrv)
{
	if (!vrv)
		vrv = GetView(0);

	MeshData* md_sel = 0;

	wxProgressDialog *prg_diag = new wxProgressDialog(
		"FluoRender: Loading mesh data...",
		"Reading and processing selected mesh data. Please wait.",
		100, 0, wxPD_SMOOTH|wxPD_ELAPSED_TIME|wxPD_AUTO_HIDE);

	MeshGroup* group = 0;
	if (files.Count() > 1)
		group = vrv->AddOrGetMGroup();

	for (int i=0; i<(int)files.Count(); i++)
	{
		prg_diag->Update(90*(i+1)/(int)files.Count());

		wxString filename = files[i];
		m_data_mgr.LoadMeshData(filename);

		MeshData* md = m_data_mgr.GetLastMeshData();
		if (vrv && md)
		{
			if (group)
			{
				group->InsertMeshData(group->GetMeshNum()-1, md);
				vrv->SetMeshPopDirty();
			}
			else
				vrv->AddMeshData(md);

			if (i==int(files.Count()-1))
				md_sel = md;
		}
	}

	UpdateList();
	if (md_sel)
		UpdateTree(md_sel->GetName());
	else
		UpdateTree();

	if (vrv)
		vrv->InitView(INIT_BOUNDS|INIT_CENTER);

	delete prg_diag;
}

void VRenderFrame::OnOpenMesh(wxCommandEvent& WXUNUSED(event))
{
	wxFileDialog *fopendlg = new wxFileDialog(
		this, "Choose the mesh data file",
		"", "", "*.obj", wxFD_OPEN|wxFD_MULTIPLE);

	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		VRenderView* vrv = GetView(0);
		wxArrayString files;
		fopendlg->GetPaths(files);

		LoadMeshes(files, vrv);
	}

	if (fopendlg)
		delete fopendlg;
}

void VRenderFrame::OnOrganize(wxCommandEvent& WXUNUSED(event))
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

void VRenderFrame::OnCheckUpdates(wxCommandEvent &)
{
	::wxLaunchDefaultBrowser(VERSION_UPDATES);
}

void VRenderFrame::OnInfo(wxCommandEvent& WXUNUSED(event))
{

	wxString time = wxNow();
	int psJan = time.Find("Jan");
	int psDec = time.Find("Dec");
	wxDialog* d = new wxDialog(this,wxID_ANY,"About FluoRender",wxDefaultPosition,
		wxSize(600,200),wxDEFAULT_DIALOG_STYLE );
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
		wxDefaultPosition,wxSize(-1,-1));
	wxFont font = wxFont(15,wxFONTFAMILY_ROMAN,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL );
	txt->SetFont(font);
	right->Add(txt,0,wxEXPAND);
	txt = new wxStaticText(d,wxID_ANY,"Version: " +
		wxString::Format("%d.%.1f", VERSION_MAJOR, float(VERSION_MINOR)),
		wxDefaultPosition,wxSize(50,-1));
	font = wxFont(12,wxFONTFAMILY_ROMAN,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL );
	txt->SetFont(font);
	right->Add(txt,0,wxEXPAND);
	txt = new wxStaticText(d,wxID_ANY,wxString("Copyright (c) ") + VERSION_COPYRIGHT,
		wxDefaultPosition,wxSize(-1,-1));
	font = wxFont(11,wxFONTFAMILY_ROMAN,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL );
	txt->SetFont(font);
	right->Add(txt,0,wxEXPAND);
	right->Add(3,5,0);
	txt = new wxStaticText(d,wxID_ANY,VERSION_AUTHORS,
		wxDefaultPosition,wxSize(300,-1));
	font = wxFont(7,wxFONTFAMILY_ROMAN,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL );
	txt->SetFont(font);
	right->Add(txt,0,wxEXPAND);
	wxHyperlinkCtrl* hyp = new wxHyperlinkCtrl(d,wxID_ANY,"Contact Info",
		VERSION_CONTACT,
		wxDefaultPosition,wxSize(-1,-1));
	right->Add(hyp,0,wxEXPAND);
	right->AddStretchSpacer();
	//put together
	main->Add(left,0,wxEXPAND);
	main->Add(right,0,wxEXPAND);
	d->SetSizer(main);
	d->ShowModal();
}

void VRenderFrame::UpdateTreeIcons()
{
	int i, j, k;
	if (!m_tree_panel || !m_tree_panel->GetTreeCtrl())
		return;

	DataTreeCtrl* treectrl = m_tree_panel->GetTreeCtrl();
	wxTreeItemId root = treectrl->GetRootItem();
	wxTreeItemIdValue ck_view;
	int counter = 0;
	for (i=0; i<(int)m_vrv_list.size(); i++)
	{
		VRenderView *vrv = m_vrv_list[i];
		wxTreeItemId vrv_item;
		if (i==0)
			vrv_item = treectrl->GetFirstChild(root, ck_view);
		else
			vrv_item = treectrl->GetNextChild(root, ck_view);

		if (!vrv_item.IsOk())
			continue;

		m_tree_panel->SetViewItemImage(vrv_item, vrv->GetDraw());

		wxTreeItemIdValue ck_layer;
		for (j=0; j<vrv->GetLayerNum(); j++)
		{
			TreeLayer* layer = vrv->GetLayer(j);
			wxTreeItemId layer_item;
			if (j==0)
				layer_item = treectrl->GetFirstChild(vrv_item, ck_layer);
			else
				layer_item = treectrl->GetNextChild(vrv_item, ck_layer);

			if (!layer_item.IsOk())
				continue;

			switch (layer->IsA())
			{
			case 2://volume
				{
					VolumeData* vd = (VolumeData*)layer;
					if (!vd)
						break;
					counter++;
					m_tree_panel->SetVolItemImage(layer_item, vd->GetDisp()?2*counter+1:2*counter);
				}
				break;
			case 3://mesh
				{
					MeshData* md = (MeshData*)layer;
					if (!md)
						break;
					counter++;
					m_tree_panel->SetMeshItemImage(layer_item, md->GetDisp()?2*counter+1:2*counter);
				}
				break;
			case 4://annotations
				{
					Annotations* ann = (Annotations*)layer;
					if (!ann)
						break;
					counter++;
					m_tree_panel->SetAnnotationItemImage(layer_item, ann->GetDisp()?2*counter+1:2*counter);
				}
				break;
			case 5://volume group
				{
					DataGroup* group = (DataGroup*)layer;
					if (!group)
						break;
					m_tree_panel->SetGroupItemImage(layer_item, int(group->GetDisp()));
					wxTreeItemIdValue ck_volume;
					for (k=0; k<group->GetVolumeNum(); k++)
					{
						VolumeData* vd = group->GetVolumeData(k);
						if (!vd)
							continue;
						wxTreeItemId volume_item;
						if (k==0)
							volume_item = treectrl->GetFirstChild(layer_item, ck_volume);
						else
							volume_item = treectrl->GetNextChild(layer_item, ck_volume);
						if (!volume_item.IsOk())
							continue;
						counter++;
						m_tree_panel->SetVolItemImage(volume_item, vd->GetDisp()?2*counter+1:2*counter);
					}
				}
				break;
			case 6://mesh group
				{
					MeshGroup* group = (MeshGroup*)layer;
					if (!group)
						break;
					m_tree_panel->SetMGroupItemImage(layer_item, int(group->GetDisp()));
					wxTreeItemIdValue ck_mesh;
					for (k=0; k<group->GetMeshNum(); k++)
					{
						MeshData* md = group->GetMeshData(k);
						if (!md)
							continue;
						wxTreeItemId mesh_item;
						if (k==0)
							mesh_item = treectrl->GetFirstChild(layer_item, ck_mesh);
						else
							mesh_item = treectrl->GetNextChild(layer_item, ck_mesh);
						if (!mesh_item.IsOk())
							continue;
						counter++;
						m_tree_panel->SetMeshItemImage(mesh_item, md->GetDisp()?2*counter+1:2*counter);
					}
				}
				break;
			}
		}
	}
	m_tree_panel->Refresh(false);
}

void VRenderFrame::UpdateTreeColors()
{
	int i, j, k;
	int counter = 0;
	for (i=0 ; i<(int)m_vrv_list.size() ; i++)
	{
		VRenderView *vrv = m_vrv_list[i];

		for (j=0; j<vrv->GetLayerNum(); j++)
		{
			TreeLayer* layer = vrv->GetLayer(j);
			switch (layer->IsA())
			{
			case 0://root
				break;
			case 1://view
				break;
			case 2://volume
				{
					VolumeData* vd = (VolumeData*)layer;
					if (!vd)
						break;
					Color c = vd->GetColor();
					wxColor wxc(
						(unsigned char)(c.r()*255),
						(unsigned char)(c.g()*255),
						(unsigned char)(c.b()*255));
					m_tree_panel->ChangeIconColor(counter+1, wxc);
					counter++;
				}
				break;
			case 3://mesh
				{
					MeshData* md = (MeshData*)layer;
					if (!md)
						break;
					Color amb, diff, spec;
					double shine, alpha;
					md->GetMaterial(amb, diff, spec, shine, alpha);
					wxColor wxc(
						(unsigned char)(diff.r()*255),
						(unsigned char)(diff.g()*255),
						(unsigned char)(diff.b()*255));
					m_tree_panel->ChangeIconColor(counter+1, wxc);
					counter++;
				}
				break;
			case 4://annotations
				{
					Annotations* ann = (Annotations*)layer;
					if (!ann)
						break;
					wxColor wxc(255, 255, 255);
					m_tree_panel->ChangeIconColor(counter+1, wxc);
					counter++;
				}
				break;
			case 5://group
				{
					DataGroup* group = (DataGroup*)layer;
					if (!group)
						break;
					for (k=0; k<group->GetVolumeNum(); k++)
					{
						VolumeData* vd = group->GetVolumeData(k);
						if (!vd)
							break;
						Color c = vd->GetColor();
						wxColor wxc(
							(unsigned char)(c.r()*255),
							(unsigned char)(c.g()*255),
							(unsigned char)(c.b()*255));
						m_tree_panel->ChangeIconColor(counter+1, wxc);
						counter++;
					}
				}
				break;
			case 6://mesh group
				{
					MeshGroup* group = (MeshGroup*)layer;
					if (!group)
						break;
					for (k=0; k<group->GetMeshNum(); k++)
					{
						MeshData* md = group->GetMeshData(k);
						if (!md)
							break;
						Color amb, diff, spec;
						double shine, alpha;
						md->GetMaterial(amb, diff, spec, shine, alpha);
						wxColor wxc(
							(unsigned char)(diff.r()*255),
							(unsigned char)(diff.g()*255),
							(unsigned char)(diff.b()*255));
						m_tree_panel->ChangeIconColor(counter+1, wxc);
						counter++;
					}
				}
				break;
			}
		}
	}
	m_tree_panel->Refresh(false);
}

void VRenderFrame::UpdateTree(wxString name)
{
	if (!m_tree_panel)
		return;

	m_tree_panel->DeleteAll();
	m_tree_panel->ClearIcons();

	wxString root_str = "Active Datasets";
	wxTreeItemId root_item = m_tree_panel->AddRootItem(root_str);
	if (name == root_str)
		m_tree_panel->SelectItem(root_item);
	//append non-color icons for views
	m_tree_panel->AppendIcon();
	m_tree_panel->Expand(root_item);
	m_tree_panel->ChangeIconColor(0, wxColor(255, 255, 255));

	wxTreeItemId sel_item;

	for (int i=0 ; i<(int)m_vrv_list.size() ; i++)
	{
		int j, k;
		VRenderView *vrv = m_vrv_list[i];
		if (!vrv)
			continue;

		vrv->OrganizeLayers();
		wxTreeItemId vrv_item = m_tree_panel->AddViewItem(vrv->GetName());
		m_tree_panel->SetViewItemImage(vrv_item, vrv->GetDraw());
		if (name == vrv->GetName())
			m_tree_panel->SelectItem(vrv_item);

		for (j=0; j<vrv->GetLayerNum(); j++)
		{
			TreeLayer* layer = vrv->GetLayer(j);
			switch (layer->IsA())
			{
			case 0://root
				break;
			case 1://view
				break;
			case 2://volume data
				{
					VolumeData* vd = (VolumeData*)layer;
					if (!vd)
						break;
					//append icon for volume
					m_tree_panel->AppendIcon();
					Color c = vd->GetColor();
					wxColor wxc(
						(unsigned char)(c.r()*255),
						(unsigned char)(c.g()*255),
						(unsigned char)(c.b()*255));
					int ii = m_tree_panel->GetIconNum()-1;
					m_tree_panel->ChangeIconColor(ii, wxc);
					wxTreeItemId item = m_tree_panel->AddVolItem(vrv_item, vd->GetName());
					m_tree_panel->SetVolItemImage(item, vd->GetDisp()?2*ii+1:2*ii);
					if (name == vd->GetName())
					{
						sel_item = item;
						vrv->SetVolumeA(vd);
						GetBrushToolDlg()->GetSettings(vrv);
						GetMeasureDlg()->GetSettings(vrv);
						GetTraceDlg()->GetSettings(vrv);
						GetOclDlg()->GetSettings(vrv);
						GetComponentDlg()->SetView(vrv);
						GetColocalizationDlg()->SetView(vrv);
					}
				}
				break;
			case 3://mesh data
				{
					MeshData* md = (MeshData*)layer;
					if (!md)
						break;
					//append icon for mesh
					m_tree_panel->AppendIcon();
					Color amb, diff, spec;
					double shine, alpha;
					md->GetMaterial(amb, diff, spec, shine, alpha);
					wxColor wxc(
						(unsigned char)(diff.r()*255),
						(unsigned char)(diff.g()*255),
						(unsigned char)(diff.b()*255));
					int ii = m_tree_panel->GetIconNum()-1;
					m_tree_panel->ChangeIconColor(ii, wxc);
					wxTreeItemId item = m_tree_panel->AddMeshItem(vrv_item, md->GetName());
					m_tree_panel->SetMeshItemImage(item, md->GetDisp()?2*ii+1:2*ii);
					if (name == md->GetName())
						sel_item = item;
				}
				break;
			case 4://annotations
				{
					Annotations* ann = (Annotations*)layer;
					if (!ann)
						break;
					//append icon for annotations
					m_tree_panel->AppendIcon();
					wxColor wxc(255, 255, 255);
					int ii = m_tree_panel->GetIconNum()-1;
					m_tree_panel->ChangeIconColor(ii, wxc);
					wxTreeItemId item = m_tree_panel->AddAnnotationItem(vrv_item, ann->GetName());
					m_tree_panel->SetAnnotationItemImage(item, ann->GetDisp()?2*ii+1:2*ii);
					if (name == ann->GetName())
						sel_item = item;
				}
				break;
			case 5://group
				{
					DataGroup* group = (DataGroup*)layer;
					if (!group)
						break;
					//append group item to tree
					wxTreeItemId group_item = m_tree_panel->AddGroupItem(vrv_item, group->GetName());
					m_tree_panel->SetGroupItemImage(group_item, int(group->GetDisp()));
					//append volume data to group
					for (k=0; k<group->GetVolumeNum(); k++)
					{
						VolumeData* vd = group->GetVolumeData(k);
						if (!vd)
							continue;
						//add icon
						m_tree_panel->AppendIcon();
						Color c = vd->GetColor();
						wxColor wxc(
							(unsigned char)(c.r()*255),
							(unsigned char)(c.g()*255),
							(unsigned char)(c.b()*255));
						int ii = m_tree_panel->GetIconNum()-1;
						m_tree_panel->ChangeIconColor(ii, wxc);
						wxTreeItemId item = m_tree_panel->AddVolItem(group_item, vd->GetName());
						m_tree_panel->SetVolItemImage(item, vd->GetDisp()?2*ii+1:2*ii);
						if (name == vd->GetName())
						{
							sel_item = item;
							vrv->SetVolumeA(vd);
							GetBrushToolDlg()->GetSettings(vrv);
							GetMeasureDlg()->GetSettings(vrv);
							GetTraceDlg()->GetSettings(vrv);
							GetOclDlg()->GetSettings(vrv);
							GetComponentDlg()->SetView(vrv);
							GetColocalizationDlg()->SetView(vrv);
						}
					}
					if (name == group->GetName())
						sel_item = group_item;
				}
				break;
			case 6://mesh group
				{
					MeshGroup* group = (MeshGroup*)layer;
					if (!group)
						break;
					//append group item to tree
					wxTreeItemId group_item = m_tree_panel->AddMGroupItem(vrv_item, group->GetName());
					m_tree_panel->SetMGroupItemImage(group_item, int(group->GetDisp()));
					//append mesh data to group
					for (k=0; k<group->GetMeshNum(); k++)
					{
						MeshData* md = group->GetMeshData(k);
						if (!md)
							continue;
						//add icon
						m_tree_panel->AppendIcon();
						Color amb, diff, spec;
						double shine, alpha;
						md->GetMaterial(amb, diff, spec, shine, alpha);
						wxColor wxc(
							(unsigned char)(diff.r()*255),
							(unsigned char)(diff.g()*255),
							(unsigned char)(diff.b()*255));
						int ii = m_tree_panel->GetIconNum()-1;
						m_tree_panel->ChangeIconColor(ii, wxc);
						wxTreeItemId item = m_tree_panel->AddMeshItem(group_item, md->GetName());
						m_tree_panel->SetMeshItemImage(item, md->GetDisp()?2*ii+1:2*ii);
						if (name == md->GetName())
							sel_item = item;
					}
					if (name == group->GetName())
						sel_item = group_item;
				}
				break;
			}
		}
	}

	if (sel_item.IsOk())
		m_tree_panel->SelectItem(sel_item);
	m_tree_panel->ExpandAll();
}

void VRenderFrame::UpdateList()
{
	m_list_panel->DeleteAllItems();

	for (int i=0 ; i<m_data_mgr.GetVolumeNum() ; i++)
	{
		VolumeData* vd = m_data_mgr.GetVolumeData(i);
		if (vd && !vd->GetDup())
		{
			wxString name = vd->GetName();
			wxString path = vd->GetPath();
			m_list_panel->Append(DATA_VOLUME, name, path);
		}
	}

	for (int i=0 ; i<m_data_mgr.GetMeshNum() ; i++)
	{
		MeshData* md = m_data_mgr.GetMeshData(i);
		if (md)
		{
			wxString name = md->GetName();
			wxString path = md->GetPath();
			m_list_panel->Append(DATA_MESH, name, path);
		}
	}

	for (int i=0; i<m_data_mgr.GetAnnotationNum(); i++)
	{
		Annotations* ann = m_data_mgr.GetAnnotations(i);
		if (ann)
		{
			wxString name = ann->GetName();
			wxString path = ann->GetPath();
			m_list_panel->Append(DATA_ANNOTATIONS, name, path);
		}
	}
}

DataManager* VRenderFrame::GetDataManager()
{
	return &m_data_mgr;
}

TreePanel *VRenderFrame::GetTree()
{
	return m_tree_panel;
}

ListPanel *VRenderFrame::GetList()
{
	return m_list_panel;
}

//on selections
void VRenderFrame::OnSelection(int type,
	VRenderView* vrv,
	DataGroup* group,
	VolumeData* vd,
	MeshData* md,
	Annotations* ann)
{
	if (m_adjust_view)
	{
		m_adjust_view->SetRenderView(vrv);
		if (!vrv || vd)
			m_adjust_view->SetVolumeData(vd);
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
	}

	m_cur_sel_type = type;
	//clear mesh boundbox
	if (m_data_mgr.GetMeshData(m_cur_sel_mesh))
		m_data_mgr.GetMeshData(m_cur_sel_mesh)->SetDrawBounds(false);

	if (m_brush_tool_dlg)
		m_brush_tool_dlg->GetSettings(vrv);
	if (m_colocalization_dlg)
		m_colocalization_dlg->SetView(vrv);
	if (m_component_dlg)
		m_component_dlg->SetView(vrv);
	if (m_counting_dlg)
		m_counting_dlg->GetSettings(vrv);
	if (m_measure_dlg)
		m_measure_dlg->GetSettings(vrv);
	if (m_noise_cancelling_dlg)
		m_noise_cancelling_dlg->GetSettings(vrv);
	if (m_ocl_dlg)
		m_ocl_dlg->GetSettings(vrv);
	if (m_recorder_dlg)
		m_recorder_dlg->GetSettings(vrv);
	if (m_trace_dlg)
		m_trace_dlg->GetSettings(vrv);

	switch (type)
	{
	case 0:  //root
		break;
	case 1:  //view
		if (m_volume_prop)
			m_volume_prop->Show(false);
		if (m_mesh_prop)
			m_mesh_prop->Show(false);
		if (m_mesh_manip)
			m_mesh_manip->Show(false);
		if (m_annotation_prop)
			m_annotation_prop->Show(false);
		if (m_colocalization_dlg)
			m_colocalization_dlg->SetGroup(group);
		m_aui_mgr.GetPane(m_prop_panel).Caption(UITEXT_PROPERTIES);
		m_aui_mgr.Update();
		break;
	case 2:  //volume
		if (vd && vd->GetDisp())
		{
			m_volume_prop->SetVolumeData(vd);
			m_volume_prop->SetGroup(group);
			m_volume_prop->SetView(vrv);
			if (!m_volume_prop->IsShown())
			{
				m_volume_prop->Show(true);
				m_prop_sizer->Clear();
				m_prop_sizer->Add(m_volume_prop, 1, wxEXPAND, 0);
				m_prop_panel->SetSizer(m_prop_sizer);
				m_prop_panel->Layout();
			}
			m_aui_mgr.GetPane(m_prop_panel).Caption(
				wxString(UITEXT_PROPERTIES)+wxString(" - ")+vd->GetName());
			m_aui_mgr.Update();
			wxString str = vd->GetName();
			m_cur_sel_vol = m_data_mgr.GetVolumeIndex(str);

			for (int i=0; i<(int)m_vrv_list.size(); i++)
			{
				VRenderView* vrv = m_vrv_list[i];
				if (!vrv)
					continue;
				vrv->m_glview->m_cur_vol = vd;
			}

			if (m_volume_prop)
				m_volume_prop->Show(true);
			if (m_mesh_prop)
				m_mesh_prop->Show(false);
			if (m_mesh_manip)
				m_mesh_manip->Show(false);
			if (m_annotation_prop)
				m_annotation_prop->Show(false);
		}
		else
		{
			if (m_volume_prop)
				m_volume_prop->Show(false);
			if (m_mesh_prop)
				m_mesh_prop->Show(false);
			if (m_mesh_manip)
				m_mesh_manip->Show(false);
			if (m_annotation_prop)
				m_annotation_prop->Show(false);
		}
		if (m_colocalization_dlg)
			m_colocalization_dlg->SetGroup(group);
		break;
	case 3:  //mesh
		if (md)
		{
			m_mesh_prop->SetMeshData(md, vrv);
			if (!m_mesh_prop->IsShown())
			{
				m_mesh_prop->Show(true);
				m_prop_sizer->Clear();
				m_prop_sizer->Add(m_mesh_prop, 1, wxEXPAND, 0);
				m_prop_panel->SetSizer(m_prop_sizer);
				m_prop_panel->Layout();
			}
			m_aui_mgr.GetPane(m_prop_panel).Caption(
				wxString(UITEXT_PROPERTIES)+wxString(" - ")+md->GetName());
			m_aui_mgr.Update();
			wxString str = md->GetName();
			m_cur_sel_mesh = m_data_mgr.GetMeshIndex(str);
			md->SetDrawBounds(true);
		}

		if (m_volume_prop)
			m_volume_prop->Show(false);
		if (m_mesh_prop && md)
			m_mesh_prop->Show(true);
		if (m_mesh_manip)
			m_mesh_manip->Show(false);
		if (m_annotation_prop)
			m_annotation_prop->Show(false);
		if (m_colocalization_dlg)
			m_colocalization_dlg->SetGroup(0);
		break;
	case 4:  //annotations
		if (ann)
		{
			m_annotation_prop->SetAnnotations(ann, vrv);
			if (!m_annotation_prop->IsShown())
			{
				m_annotation_prop->Show(true);
				m_prop_sizer->Clear();
				m_prop_sizer->Add(m_annotation_prop, 1, wxEXPAND, 0);
				m_prop_panel->SetSizer(m_prop_sizer);
				m_prop_panel->Layout();
			}
			m_aui_mgr.GetPane(m_prop_panel).Caption(
				wxString(UITEXT_PROPERTIES)+wxString(" - ")+ann->GetName());
			m_aui_mgr.Update();
		}

		if (m_volume_prop)
			m_volume_prop->Show(false);
		if (m_mesh_prop)
			m_mesh_prop->Show(false);
		if (m_mesh_manip)
			m_mesh_manip->Show(false);
		if (m_annotation_prop && ann)
			m_annotation_prop->Show(true);
		if (m_colocalization_dlg)
			m_colocalization_dlg->SetGroup(0);
		break;
	case 5:  //group
		if (m_adjust_view)
			m_adjust_view->SetGroup(group);
		if (m_calculation_dlg)
			m_calculation_dlg->SetGroup(group);
		if (m_volume_prop)
			m_volume_prop->Show(false);
		if (m_mesh_prop)
			m_mesh_prop->Show(false);
		if (m_mesh_manip)
			m_mesh_manip->Show(false);
		if (m_annotation_prop)
			m_annotation_prop->Show(false);
		if (m_colocalization_dlg)
			m_colocalization_dlg->SetGroup(group);
		m_aui_mgr.GetPane(m_prop_panel).Caption(UITEXT_PROPERTIES);
		m_aui_mgr.Update();
		break;
	case 6:  //mesh manip
		if (md)
		{
			m_mesh_manip->SetMeshData(md);
			m_mesh_manip->GetData();
			if (!m_mesh_manip->IsShown())
			{
				m_mesh_manip->Show(true);
				m_prop_sizer->Clear();
				m_prop_sizer->Add(m_mesh_manip, 1, wxEXPAND, 0);
				m_prop_panel->SetSizer(m_prop_sizer);
				m_prop_panel->Layout();
			}
			m_aui_mgr.GetPane(m_prop_panel).Caption(
				wxString("Manipulations - ")+md->GetName());
			m_aui_mgr.Update();
		}

		if (m_volume_prop)
			m_volume_prop->Show(false);
		if (m_mesh_prop)
			m_mesh_prop->Show(false);
		if (m_mesh_manip && md)
			m_mesh_manip->Show(true);
		if (m_annotation_prop)
			m_annotation_prop->Show(false);
		if (m_colocalization_dlg)
			m_colocalization_dlg->SetGroup(0);
		break;
	default:
		if (m_volume_prop)
			m_volume_prop->Show(false);
		if (m_mesh_prop)
			m_mesh_prop->Show(false);
		if (m_mesh_manip)
			m_mesh_manip->Show(false);
		if (m_annotation_prop)
			m_annotation_prop->Show(false);
		if (m_colocalization_dlg)
			m_colocalization_dlg->SetGroup(0);
		m_aui_mgr.GetPane(m_prop_panel).Caption(UITEXT_PROPERTIES);
		m_aui_mgr.Update();
	}
}

void VRenderFrame::RefreshVRenderViews(bool tree, bool interactive)
{
	for (int i=0 ; i<(int)m_vrv_list.size() ; i++)
	{
		if (m_vrv_list[i])
			m_vrv_list[i]->RefreshGL(interactive);
	}

	//incase volume color changes
	//change icon color of the tree panel
	if (tree)
		UpdateTreeColors();
}

void VRenderFrame::DeleteVRenderView(int i)
{
	if (m_vrv_list[i])
	{
		int j;
		wxString str = m_vrv_list[i]->GetName();

		for (j=0 ; j<m_vrv_list[i]->GetAllVolumeNum() ; j++)
			m_vrv_list[i]->GetAllVolumeData(j)->SetDisp(true);
		for (j=0 ; j<m_vrv_list[i]->GetMeshNum() ; j++)
			m_vrv_list[i]->GetMeshData(j)->SetDisp(true);
		VRenderView* vrv = m_vrv_list[i];
		m_vrv_list.erase(m_vrv_list.begin()+i);
		m_aui_mgr.DetachPane(vrv);
		vrv->Close();
		delete vrv;
		m_aui_mgr.Update();
		UpdateTree();

		if (m_movie_view)
			m_movie_view->DeleteView(str);
	}
}

void VRenderFrame::DeleteVRenderView(wxString &name)
{
	for (int i=0; i<GetViewNum(); i++)
	{
		VRenderView* vrv = GetView(i);
		if (vrv && name == vrv->GetName() && vrv->m_id > 1)
		{
			DeleteVRenderView(i);
			return;
		}
	}
}

AdjustView* VRenderFrame::GetAdjustView()
{
	return m_adjust_view;
}

//organize render views
void VRenderFrame::OrganizeVRenderViews(int mode)
{
	int width = 800;
	int height = 600;
	int minx = 0;
	int miny = 0;
	int maxx = 0;
	int maxy = 0;
	int paneNum = (int)m_vrv_list.size();
	int i;
	//get total area
	for (i = 0; i < paneNum; i++)
	{
		VRenderView* vrv = m_vrv_list[i];
		if (vrv && m_aui_mgr.GetPane(vrv).IsOk())
		{
			wxPoint pos = vrv->GetPosition();
			wxSize size = vrv->GetSize();
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
		VRenderView* vrv = m_vrv_list[i];
		if (vrv)
			m_aui_mgr.DetachPane(vrv);
	}
	//add back
	for (i=0; i<paneNum; i++)
	{
		VRenderView* vrv = m_vrv_list[i];
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
					//MinSize(width, height / paneNum).
					//MaxSize(width, height / paneNum).
					BestSize(width, height / paneNum).
					Layer(0).Centre());
				break;
			case 1://left-right
				vrv->SetSize(width / paneNum, height);
				if (i == 0)
					m_aui_mgr.AddPane(vrv, wxAuiPaneInfo().
						Name(vrv->GetName()).Caption(vrv->GetName()).
						Dockable(true).CloseButton(false).Resizable().
						FloatingSize(width / paneNum, height).
						//MinSize(width / paneNum, height).
						//MaxSize(width / paneNum, height).
						BestSize(width / paneNum, height).
						Layer(0).Centre());
				else
				m_aui_mgr.AddPane(vrv, wxAuiPaneInfo().
					Name(vrv->GetName()).Caption(vrv->GetName()).
					Dockable(true).CloseButton(false).Resizable().
					FloatingSize(width / paneNum, height).
					//MinSize(width / paneNum, height).
					//MaxSize(width / paneNum, height).
					BestSize(width / paneNum, height).
					Layer(0).Centre().Right());
				break;
			}
		}
	}
	m_aui_mgr.Update();
}

//hide/show tools
void VRenderFrame::ToggleAllTools(bool cur_state)
{
	if (cur_state)
	{
		if (m_aui_mgr.GetPane(m_list_panel).IsShown() &&
			m_aui_mgr.GetPane(m_tree_panel).IsShown() &&
			m_aui_mgr.GetPane(m_movie_view).IsShown() &&
			m_aui_mgr.GetPane(m_prop_panel).IsShown() &&
			m_aui_mgr.GetPane(m_adjust_view).IsShown() &&
			m_aui_mgr.GetPane(m_clip_view).IsShown())
			m_ui_state = true;
		else if (!m_aui_mgr.GetPane(m_list_panel).IsShown() &&
			!m_aui_mgr.GetPane(m_tree_panel).IsShown() &&
			!m_aui_mgr.GetPane(m_movie_view).IsShown() &&
			!m_aui_mgr.GetPane(m_prop_panel).IsShown() &&
			!m_aui_mgr.GetPane(m_adjust_view).IsShown() &&
			!m_aui_mgr.GetPane(m_clip_view).IsShown())
			m_ui_state = false;
	}

	if (m_ui_state)
	{
		//hide all
		//data view
		m_aui_mgr.GetPane(m_list_panel).Hide();
		m_tb_menu_ui->Check(ID_UIListView, false);
		//scene view
		m_aui_mgr.GetPane(m_tree_panel).Hide();
		m_tb_menu_ui->Check(ID_UITreeView, false);
		//movie view (float only)
		m_aui_mgr.GetPane(m_movie_view).Hide();
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
		m_aui_mgr.GetPane(m_list_panel).Show();
		m_tb_menu_ui->Check(ID_UIListView, true);
		//scene view
		m_aui_mgr.GetPane(m_tree_panel).Show();
		m_tb_menu_ui->Check(ID_UITreeView, true);
		//movie view (float only)
		m_aui_mgr.GetPane(m_movie_view).Show();
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

void VRenderFrame::ShowPane(wxPanel* pane, bool show)
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

void VRenderFrame::OnChEmbedCheck(wxCommandEvent &event)
{
	wxCheckBox* ch_embed = (wxCheckBox*)event.GetEventObject();
	if (ch_embed)
		m_vrp_embed = ch_embed->GetValue();
}

void VRenderFrame::OnChSaveCmpCheck(wxCommandEvent &event)
{
	wxCheckBox* ch_cmp = (wxCheckBox*)event.GetEventObject();
	if (ch_cmp)
		m_save_compress = ch_cmp->GetValue();
}

wxWindow* VRenderFrame::CreateExtraControlProjectSave(wxWindow* parent)
{
	wxPanel* panel = new wxPanel(parent, 0, wxDefaultPosition, wxSize(600, 90));
#ifdef _DARWIN
	panel->SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#endif
	wxBoxSizer *group1 = new wxStaticBoxSizer(
		new wxStaticBox(panel, wxID_ANY, "Additional Options"), wxVERTICAL );

	//copy all files check box
	wxCheckBox* ch_embed = new wxCheckBox(panel, ID_EMBED_FILES,
		"Embed all files in the project folder");
	ch_embed->Connect(ch_embed->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(VRenderFrame::OnChEmbedCheck), NULL, panel);
	ch_embed->SetValue(m_vrp_embed);

	//compressed
	wxCheckBox* ch_cmp = new wxCheckBox(panel, ID_LZW_COMP,
		"Lempel-Ziv-Welch Compression");
	ch_cmp->Connect(ch_cmp->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(VRenderFrame::OnChSaveCmpCheck), NULL, panel);
	if (ch_cmp)
		ch_cmp->SetValue(m_save_compress);

	//group
	group1->Add(10, 10);
	group1->Add(ch_embed);
	group1->Add(10, 10);
	group1->Add(ch_cmp);
	group1->Add(10, 10);

	panel->SetSizer(group1);
	panel->Layout();

	return panel;
}

void VRenderFrame::OnSaveProject(wxCommandEvent& WXUNUSED(event))
{
	wxFileDialog *fopendlg = new wxFileDialog(
		this, "Save Project File",
		"", "", "*.vrp", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
	fopendlg->SetExtraControlCreator(CreateExtraControlProjectSave);

	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		wxString filename = fopendlg->GetPath();
		SaveProject(filename);
	}

	delete fopendlg;
}

void VRenderFrame::OnOpenProject(wxCommandEvent& WXUNUSED(event))
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

void VRenderFrame::SaveProject(wxString& filename)
{
	wxString app_name = "FluoRender " +
		wxString::Format("%d.%.1f", VERSION_MAJOR, float(VERSION_MINOR));
	wxString vendor_name = "FluoRender";
	wxString local_name = filename;
	wxFileConfig fconfig(app_name, vendor_name, local_name, "",
		wxCONFIG_USE_LOCAL_FILE);

	int i, j, k;
	fconfig.Write("ver_major", VERSION_MAJOR_TAG);
	fconfig.Write("ver_minor", VERSION_MINOR_TAG);

	int ticks = m_data_mgr.GetVolumeNum() + m_data_mgr.GetMeshNum();
	int tick_cnt = 1;
	fconfig.Write("ticks", ticks);
	wxProgressDialog *prg_diag = 0;
	prg_diag = new wxProgressDialog(
		"FluoRender: Saving project...",
		"Saving project file. Please wait.",
		100, 0, wxPD_SMOOTH|wxPD_ELAPSED_TIME|wxPD_AUTO_HIDE);

	wxString str;

	//save streaming mode
	fconfig.SetPath("/memory settings");
	fconfig.Write("mem swap", m_setting_dlg->GetMemSwap());
	fconfig.Write("graphics mem", m_setting_dlg->GetGraphicsMem());
	fconfig.Write("large data size", m_setting_dlg->GetLargeDataSize());
	fconfig.Write("force brick size", m_setting_dlg->GetForceBrickSize());
	fconfig.Write("up time", m_setting_dlg->GetResponseTime());
	fconfig.Write("update order", m_setting_dlg->GetUpdateOrder());

	//save data list
	//volume
	fconfig.SetPath("/data/volume");
	fconfig.Write("num", m_data_mgr.GetVolumeNum());
	for (i=0; i<m_data_mgr.GetVolumeNum(); i++)
	{
		if (ticks && prg_diag)
			prg_diag->Update(90*tick_cnt/ticks,
			"Saving volume data. Please wait.");
		tick_cnt++;

		VolumeData* vd = m_data_mgr.GetVolumeData(i);
		if (vd)
		{
			str = wxString::Format("/data/volume/%d", i);
			//name
			fconfig.SetPath(str);
			str = vd->GetName();
			fconfig.Write("name", str);
			//compression
			fconfig.Write("compression", m_compression);
			//skip brick
			fconfig.Write("skip_brick", vd->GetSkipBrick());
			//path
			str = vd->GetPath();
			bool new_chan = false;
			if (str == "" || m_vrp_embed)
			{
				wxString new_folder;
				new_folder = filename + "_files";
				wxFileName::Mkdir(new_folder, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
				str = new_folder + GETSLASH() + vd->GetName() + ".tif";
				vd->Save(str, 0, false, VRenderFrame::GetCompression());
				fconfig.Write("path", str);
				new_chan = true;
			}
			else
				fconfig.Write("path", str);
			if (vd->GetReader())
			{
				//reader type
				fconfig.Write("reader_type", vd->GetReader()->GetType());
				fconfig.Write("slice_seq", vd->GetReader()->GetSliceSeq());
				str = vd->GetReader()->GetTimeId();
				fconfig.Write("time_id", str);
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
			fconfig.Write("3dgamma", vd->Get3DGamma());
			fconfig.Write("boundary", vd->GetBoundary());
			fconfig.Write("contrast", vd->GetOffset());
			fconfig.Write("left_thresh", vd->GetLeftThresh());
			fconfig.Write("right_thresh", vd->GetRightThresh());
			Color color = vd->GetColor();
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
			fconfig.Write("enable_alpha", vd->GetEnableAlpha());
			fconfig.Write("alpha", vd->GetAlpha());
			double amb, diff, spec, shine;
			vd->GetMaterial(amb, diff, spec, shine);
			fconfig.Write("ambient", amb);
			fconfig.Write("diffuse", diff);
			fconfig.Write("specular", spec);
			fconfig.Write("shininess", shine);
			fconfig.Write("shading", vd->GetShading());
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
			vector<Plane*> *planes = 0;
			if (vd->GetVR())
				planes = vd->GetVR()->get_planes();
			if (planes && planes->size() == 6)
			{
				Plane* plane = 0;
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
			str = wxString::Format("%f %f %f", vd->GetGamma().r(), vd->GetGamma().g(), vd->GetGamma().b());
			fconfig.Write("gamma", str);
			str = wxString::Format("%f %f %f", vd->GetBrightness().r(), vd->GetBrightness().g(), vd->GetBrightness().b());
			fconfig.Write("brightness", str);
			str = wxString::Format("%f %f %f", vd->GetHdr().r(), vd->GetHdr().g(), vd->GetHdr().b());
			fconfig.Write("hdr", str);
			fconfig.Write("sync_r", vd->GetSyncR());
			fconfig.Write("sync_g", vd->GetSyncG());
			fconfig.Write("sync_b", vd->GetSyncB());

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
			fconfig.Write("shadow", vd->GetShadow());
			//shadow intensity
			double shadow_int;
			vd->GetShadowParams(shadow_int);
			fconfig.Write("shadow_darkness", shadow_int);

			//legend
			fconfig.Write("legend", vd->GetLegend());

			//mask
			vd->SaveMask(true, vd->GetCurTime(), vd->GetCurChannel());
			vd->SaveLabel(true, vd->GetCurTime(), vd->GetCurChannel());
		}
	}
	//mesh
	fconfig.SetPath("/data/mesh");
	fconfig.Write("num", m_data_mgr.GetMeshNum());
	for (i=0; i<m_data_mgr.GetMeshNum(); i++)
	{
		if (ticks && prg_diag)
			prg_diag->Update(90*tick_cnt/ticks,
			"Saving mesh data. Please wait.");
		tick_cnt++;

		MeshData* md = m_data_mgr.GetMeshData(i);
		if (md)
		{
			if (md->GetPath() == "" || m_vrp_embed)
			{
				wxString new_folder;
				new_folder = filename + "_files";
				wxFileName::Mkdir(new_folder, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
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
			Color amb, diff, spec;
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
			str = wxString::Format("%f %f %f", md->GetGamma().r(), md->GetGamma().g(), md->GetGamma().b());
			fconfig.Write("gamma", str);
			str = wxString::Format("%f %f %f", md->GetBrightness().r(), md->GetBrightness().g(), md->GetBrightness().b());
			fconfig.Write("brightness", str);
			str = wxString::Format("%f %f %f", md->GetHdr().r(), md->GetHdr().g(), md->GetHdr().b());
			fconfig.Write("hdr", str);
			fconfig.Write("sync_r", md->GetSyncR());
			fconfig.Write("sync_g", md->GetSyncG());
			fconfig.Write("sync_b", md->GetSyncB());
			//shadow
			fconfig.Write("shadow", md->GetShadow());
			double darkness;
			md->GetShadowParams(darkness);
			fconfig.Write("shadow_darkness", darkness);

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
	fconfig.Write("num", m_data_mgr.GetAnnotationNum());
	for (i=0; i<m_data_mgr.GetAnnotationNum(); i++)
	{
		Annotations* ann = m_data_mgr.GetAnnotations(i);
		if (ann)
		{
			if (ann->GetPath() == "")
			{
				wxString new_folder;
				new_folder = filename + "_files";
				wxFileName::Mkdir(new_folder, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
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
	fconfig.Write("num", (int)m_vrv_list.size());
	for (i=0; i<(int)m_vrv_list.size(); i++)
	{
		VRenderView* vrv = m_vrv_list[i];
		if (vrv)
		{
			str = wxString::Format("/views/%d", i);
			fconfig.SetPath(str);
			//view layers
			str = wxString::Format("/views/%d/layers", i);
			fconfig.SetPath(str);
			fconfig.Write("num", vrv->GetLayerNum());
			for (j=0; j<vrv->GetLayerNum(); j++)
			{
				TreeLayer* layer = vrv->GetLayer(j);
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
						str = wxString::Format("%f %f %f", group->GetGamma().r(),
							group->GetGamma().g(), group->GetGamma().b());
						fconfig.Write("gamma", str);
						str = wxString::Format("%f %f %f", group->GetBrightness().r(),
							group->GetBrightness().g(), group->GetBrightness().b());
						fconfig.Write("brightness", str);
						str = wxString::Format("%f %f %f", group->GetHdr().r(),
							group->GetHdr().g(), group->GetHdr().b());
						fconfig.Write("hdr", str);
						fconfig.Write("sync_r", group->GetSyncR());
						fconfig.Write("sync_g", group->GetSyncG());
						fconfig.Write("sync_b", group->GetSyncB());
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
			fconfig.Write("drawall", vrv->GetDraw());
			fconfig.Write("persp", vrv->GetPersp());
			fconfig.Write("free", vrv->GetFree());
			fconfig.Write("aov", vrv->GetAov());
			fconfig.Write("nearclip", vrv->GetNearClip());
			fconfig.Write("farclip", vrv->GetFarClip());
			Color bkcolor;
			bkcolor = vrv->GetBackgroundColor();
			str = wxString::Format("%f %f %f", bkcolor.r(), bkcolor.g(), bkcolor.b());
			fconfig.Write("backgroundcolor", str);
			fconfig.Write("drawtype", vrv->GetDrawType());
			fconfig.Write("volmethod", vrv->GetVolMethod());
			fconfig.Write("peellayers", vrv->GetPeelingLayers());
			fconfig.Write("fog", vrv->GetFog());
			fconfig.Write("fogintensity", (double)vrv->GetFogIntensity());
			fconfig.Write("draw_camctr", vrv->m_glview->m_draw_camctr);
			fconfig.Write("draw_info", vrv->m_glview->m_draw_info);
			fconfig.Write("draw_legend", vrv->m_glview->m_draw_legend);

			double x, y, z;
			//camera
			vrv->GetTranslations(x, y, z);
			str = wxString::Format("%f %f %f", x, y, z);
			fconfig.Write("translation", str);
			vrv->GetRotations(x, y, z);
			str = wxString::Format("%f %f %f", x, y, z);
			fconfig.Write("rotation", str);
			FLIVR::Quaternion q = vrv->m_glview->GetZeroQuat();
			str = wxString::Format("%f %f %f %f", q.x, q.y, q.z, q.w);
			fconfig.Write("zero_quat", str);
			vrv->GetCenters(x, y, z);
			str = wxString::Format("%f %f %f", x, y, z);
			fconfig.Write("center", str);
			fconfig.Write("centereyedist", vrv->GetCenterEyeDist());
			fconfig.Write("radius", vrv->GetRadius());
			fconfig.Write("initdist", vrv->m_glview->GetInitDist());
			fconfig.Write("scale_mode", vrv->m_glview->m_scale_mode);
			fconfig.Write("scale", vrv->m_glview->m_scale_factor);
			fconfig.Write("pin_rot_center", vrv->m_glview->m_pin_rot_center);
			//object
			vrv->GetObjCenters(x, y, z);
			str = wxString::Format("%f %f %f", x, y, z);
			fconfig.Write("obj_center", str);
			vrv->GetObjTrans(x, y, z);
			str = wxString::Format("%f %f %f", x, y, z);
			fconfig.Write("obj_trans", str);
			vrv->GetObjRot(x, y, z);
			str = wxString::Format("%f %f %f", x, y, z);
			fconfig.Write("obj_rot", str);
			//scale bar
			fconfig.Write("disp_scale_bar", vrv->m_glview->m_disp_scale_bar);
			fconfig.Write("disp_scale_bar_text", vrv->m_glview->m_disp_scale_bar_text);
			fconfig.Write("sb_length", vrv->m_glview->m_sb_length);
			str = vrv->m_glview->m_sb_text;
			fconfig.Write("sb_text", str);
			str = vrv->m_glview->m_sb_num;
			fconfig.Write("sb_num", str);
			fconfig.Write("sb_unit", vrv->m_glview->m_sb_unit);

			//2d adjustment
			str = wxString::Format("%f %f %f", vrv->m_glview->GetGamma().r(),
				vrv->m_glview->GetGamma().g(), vrv->m_glview->GetGamma().b());
			fconfig.Write("gamma", str);
			str = wxString::Format("%f %f %f", vrv->m_glview->GetBrightness().r(),
				vrv->m_glview->GetBrightness().g(), vrv->m_glview->GetBrightness().b());
			fconfig.Write("brightness", str);
			str = wxString::Format("%f %f %f", vrv->m_glview->GetHdr().r(),
				vrv->m_glview->GetHdr().g(), vrv->m_glview->GetHdr().b());
			fconfig.Write("hdr", str);
			fconfig.Write("sync_r", vrv->m_glview->GetSyncR());
			fconfig.Write("sync_g", vrv->m_glview->GetSyncG());
			fconfig.Write("sync_b", vrv->m_glview->GetSyncB());

			//clipping plane rotations
			fconfig.Write("clip_mode", vrv->GetClipMode());
			double rotx_cl, roty_cl, rotz_cl;
			vrv->GetClippingPlaneRotations(rotx_cl, roty_cl, rotz_cl);
			fconfig.Write("rotx_cl", rotx_cl);
			fconfig.Write("roty_cl", roty_cl);
			fconfig.Write("rotz_cl", rotz_cl);

			//painting parameters
			FL::VolumeSelector* selector = vrv->GetVolumeSelector();
			if (selector)
			{
				fconfig.Write("brush_use_pres", selector->GetBrushUsePres());
				fconfig.Write("brush_size_1", selector->GetBrushSize1());
				fconfig.Write("brush_size_2", selector->GetBrushSize2());
				fconfig.Write("brush_spacing", selector->GetBrushSpacing());
				fconfig.Write("brush_iteration", selector->GetBrushIteration());
				fconfig.Write("brush_translate", selector->GetBrushSclTranslate());
				fconfig.Write("w2d", selector->GetW2d());
			}

			//rulers
			fconfig.SetPath(wxString::Format("/views/%d/rulers", i));
			vrv->GetRulerHandler()->Save(fconfig, i);
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
	fconfig.Write("cur_page", m_movie_view->GetCurrentPage());
	fconfig.Write("views_cmb", m_movie_view->m_views_cmb->GetCurrentSelection());
	fconfig.Write("rot_check", m_movie_view->m_rot_chk->GetValue());
	fconfig.Write("seq_check", m_movie_view->m_seq_chk->GetValue());
	fconfig.Write("frame_range", m_movie_view->m_progress_sldr->GetMax());
	fconfig.Write("time_frame", m_movie_view->m_progress_sldr->GetValue());
	fconfig.Write("x_rd", m_movie_view->m_x_rd->GetValue());
	fconfig.Write("y_rd", m_movie_view->m_y_rd->GetValue());
	fconfig.Write("z_rd", m_movie_view->m_z_rd->GetValue());
	fconfig.Write("angle_end_text", m_movie_view->m_degree_end->GetValue());
	fconfig.Write("step_text", m_movie_view->m_movie_time->GetValue());
	fconfig.Write("frames_text", m_movie_view->m_fps_text->GetValue());
	fconfig.Write("frame_chk", m_movie_view->m_frame_chk->GetValue());
	fconfig.Write("center_x_text", m_movie_view->m_center_x_text->GetValue());
	fconfig.Write("center_y_text", m_movie_view->m_center_y_text->GetValue());
	fconfig.Write("width_text", m_movie_view->m_width_text->GetValue());
	fconfig.Write("height_text", m_movie_view->m_height_text->GetValue());
	fconfig.Write("time_start_text", m_movie_view->m_time_start_text->GetValue());
	fconfig.Write("time_end_text", m_movie_view->m_time_end_text->GetValue());
	fconfig.Write("run_script", m_setting_dlg->GetRunScript());
	fconfig.Write("script_file", m_setting_dlg->GetScriptFile());
	//tracking diag
	fconfig.SetPath("/track_diag");
	int ival = m_trace_dlg->GetTrackFileExist(true);
	if (ival == 1)
	{
		wxString new_folder;
		new_folder = filename + "_files";
		wxFileName::Mkdir(new_folder, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
		std::wstring wstr = filename.ToStdWstring();
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
	fconfig.Write("ui_main_tb", m_main_tb->IsShown());
	fconfig.Write("ui_list_view", m_list_panel->IsShown());
	fconfig.Write("ui_tree_view", m_tree_panel->IsShown());
	fconfig.Write("ui_movie_view", m_movie_view->IsShown());
	fconfig.Write("ui_adjust_view", m_adjust_view->IsShown());
	fconfig.Write("ui_clip_view", m_clip_view->IsShown());
	fconfig.Write("ui_prop_view", m_prop_panel->IsShown());
	fconfig.Write("ui_main_tb_float", m_aui_mgr.GetPane(m_main_tb).IsOk()?
		m_aui_mgr.GetPane(m_main_tb).IsFloating():false);
	fconfig.Write("ui_list_view_float", m_aui_mgr.GetPane(m_list_panel).IsOk()?
		m_aui_mgr.GetPane(m_list_panel).IsFloating():false);
	fconfig.Write("ui_tree_view_float", m_aui_mgr.GetPane(m_tree_panel).IsOk()?
		m_aui_mgr.GetPane(m_tree_panel).IsFloating():false);
	fconfig.Write("ui_movie_view_float", m_aui_mgr.GetPane(m_movie_view).IsOk()?
		m_aui_mgr.GetPane(m_movie_view).IsFloating():false);
	fconfig.Write("ui_adjust_view_float", m_aui_mgr.GetPane(m_adjust_view).IsOk()?
		m_aui_mgr.GetPane(m_adjust_view).IsFloating():false);
	fconfig.Write("ui_clip_view_float", m_aui_mgr.GetPane(m_clip_view).IsOk()?
		m_aui_mgr.GetPane(m_clip_view).IsFloating():false);
	fconfig.Write("ui_prop_view_float", m_aui_mgr.GetPane(m_prop_panel).IsOk()?
		m_aui_mgr.GetPane(m_prop_panel).IsFloating():false);
	//interpolator
	fconfig.SetPath("/interpolator");
	fconfig.Write("max_id", Interpolator::m_id);
	int group_num = m_interpolator.GetKeyNum();
	fconfig.Write("num", group_num);
	for (i=0; i<group_num; i++)
	{
		FlKeyGroup* key_group = m_interpolator.GetKeyGroup(i);
		if (key_group)
		{
			str = wxString::Format("/interpolator/%d", i);
			fconfig.SetPath(str);
			fconfig.Write("id", key_group->id);
			fconfig.Write("t", key_group->t);
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
							Quaternion qval = ((FlKeyQuaternion*)key)->GetValue();
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
					}
				}
			}
		}
	}

	wxFileOutputStream os(filename);
	fconfig.Save(os);
	UpdateList();

	delete prg_diag;
}

void VRenderFrame::OpenProject(wxString& filename)
{
	m_data_mgr.SetProjectPath(filename);

	int iVal;
	int i, j, k;
	//clear
	m_data_mgr.ClearAll();
	DataGroup::ResetID();
	MeshGroup::ResetID();
	m_adjust_view->SetVolumeData(0);
	m_adjust_view->SetGroup(0);
	m_adjust_view->SetGroupLink(0);
	m_vrv_list[0]->Clear();
	for (i = m_vrv_list.size() - 1; i > 0; i--)
		DeleteVRenderView(i);
	//VRenderView::ResetID();
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
		100, 0, wxPD_SMOOTH|wxPD_ELAPSED_TIME|wxPD_AUTO_HIDE);

	//read streaming mode
	if (fconfig.Exists("/memory settings"))
	{
		fconfig.SetPath("/memory settings");
		bool mem_swap = false;
		fconfig.Read("mem swap", &mem_swap);
		double graphics_mem = 1000.0;
		fconfig.Read("graphics mem", &graphics_mem);
		double large_data_size = 1000.0;
		fconfig.Read("large data size", &large_data_size);
		int force_brick_size = 128;
		fconfig.Read("force brick size", &force_brick_size);
		int up_time = 100;
		fconfig.Read("up time", &up_time);
		int update_order = 0;
		fconfig.Read("update order", &update_order);

		m_setting_dlg->SetMemSwap(mem_swap);
		m_setting_dlg->SetGraphicsMem(graphics_mem);
		m_setting_dlg->SetLargeDataSize(large_data_size);
		m_setting_dlg->SetForceBrickSize(force_brick_size);
		m_setting_dlg->SetResponseTime(up_time);
		m_setting_dlg->SetUpdateOrder(update_order);
		m_setting_dlg->UpdateUI();

		SetTextureRendererSettings();
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
				m_data_mgr.SetCompression(compression);
				bool skip_brick = false;
				fconfig.Read("skip_brick", &skip_brick);
				m_data_mgr.SetSkipBrick(skip_brick);
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
					m_data_mgr.SetSliceSequence(slice_seq);
					wxString time_id;
					fconfig.Read("time_id", &time_id);
					m_data_mgr.SetTimeId(time_id);
					wxString suffix = str.Mid(str.Find('.', true)).MakeLower();
					if (reader_type == READER_IMAGEJ_TYPE)
						loaded_num = m_data_mgr.LoadVolumeData(str, LOAD_TYPE_IMAGEJ, true, cur_chan, cur_time);
					else if (suffix == ".nrrd" || suffix == ".msk" || suffix == ".lbl")
						loaded_num = m_data_mgr.LoadVolumeData(str, LOAD_TYPE_NRRD, false, cur_chan, cur_time);
					else if (suffix == ".tif" || suffix == ".tiff")
						loaded_num = m_data_mgr.LoadVolumeData(str, LOAD_TYPE_TIFF, false, cur_chan, cur_time);
					else if (suffix == ".oib")
						loaded_num = m_data_mgr.LoadVolumeData(str, LOAD_TYPE_OIB, false, cur_chan, cur_time);
					else if (suffix == ".oif")
						loaded_num = m_data_mgr.LoadVolumeData(str, LOAD_TYPE_OIF, false, cur_chan, cur_time);
					else if (suffix == ".lsm")
						loaded_num = m_data_mgr.LoadVolumeData(str, LOAD_TYPE_LSM, false, cur_chan, cur_time);
					else if (suffix == ".xml")
						loaded_num = m_data_mgr.LoadVolumeData(str, LOAD_TYPE_PVXML, false, cur_chan, cur_time);
					else if (suffix == ".vvd")
						loaded_num = m_data_mgr.LoadVolumeData(str, LOAD_TYPE_BRKXML, false, cur_chan, cur_time);
					else if (suffix == ".czi")
						loaded_num = m_data_mgr.LoadVolumeData(str, LOAD_TYPE_CZI, false, cur_chan, cur_time);
					else if (suffix == ".nd2")
						loaded_num = m_data_mgr.LoadVolumeData(str, LOAD_TYPE_ND2, false, cur_chan, cur_time);
				}
				VolumeData* vd = 0;
				if (loaded_num)
					vd = m_data_mgr.GetLastVolumeData();
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
							wchar_t token[256];
							token[255] = '\0';
							const wchar_t* sstr = str.wc_str();
							std::wstringstream ss(sstr);
							ss.read(token,255);
							wchar_t c = 'x';
							while(!isspace(c)) ss.read(&c,1);
							ss >> type >> left_x >> left_y >> width >>
								height >> offset1 >> offset2 >> gamma;
							vd->Set3DGamma(gamma);
							vd->SetBoundary(left_y);
							vd->SetOffset(offset1);
							vd->SetLeftThresh(left_x);
							vd->SetRightThresh(left_x+width);
							if (fconfig.Read("widgetcolor", &str))
							{
								float red, green, blue;
								if (SSCANF(str.c_str(), "%f%f%f", &red, &green, &blue)){
									FLIVR::Color col(red,green,blue);
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
							vd->Set3DGamma(dval);
						if (fconfig.Read("boundary", &dval))
							vd->SetBoundary(dval);
						if (fconfig.Read("contrast", &dval))
							vd->SetOffset(dval);
						if (fconfig.Read("left_thresh", &dval))
							vd->SetLeftThresh(dval);
						if (fconfig.Read("right_thresh", &dval))
							vd->SetRightThresh(dval);
						if (fconfig.Read("color", &str))
						{
							float red, green, blue;
							if (SSCANF(str.c_str(), "%f%f%f", &red, &green, &blue)){
								FLIVR::Color col(red,green,blue);
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
								FLIVR::Color col(red,green,blue);
								if (fconfig.Read("mask_color_set", &bval))
									vd->SetMaskColor(col, bval);
								else
									vd->SetMaskColor(col);
							}
						}
						if (fconfig.Read("enable_alpha", &bval))
							vd->SetEnableAlpha(bval);
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
							vd->SetShading(shading);
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
									vd->SetSpacings(resx, resy, resz);
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

						vector<Plane*> *planes = 0;
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
								(*planes)[0]->ChangePlane(Point(abs(val/iresx), 0.0, 0.0),
								Vector(1.0, 0.0, 0.0));
							else if (fconfig.Read("x1_val", &val))
								(*planes)[0]->ChangePlane(Point(abs(val), 0.0, 0.0),
								Vector(1.0, 0.0, 0.0));

							//x2
							if (fconfig.Read("x2_vali", &val))
								(*planes)[1]->ChangePlane(Point(abs(val/iresx), 0.0, 0.0),
								Vector(-1.0, 0.0, 0.0));
							else if (fconfig.Read("x2_val", &val))
								(*planes)[1]->ChangePlane(Point(abs(val), 0.0, 0.0),
								Vector(-1.0, 0.0, 0.0));

							//y1
							if (fconfig.Read("y1_vali", &val))
								(*planes)[2]->ChangePlane(Point(0.0, abs(val/iresy), 0.0),
								Vector(0.0, 1.0, 0.0));
							else if (fconfig.Read("y1_val", &val))
								(*planes)[2]->ChangePlane(Point(0.0, abs(val), 0.0),
								Vector(0.0, 1.0, 0.0));

							//y2
							if (fconfig.Read("y2_vali", &val))
								(*planes)[3]->ChangePlane(Point(0.0, abs(val/iresy), 0.0),
								Vector(0.0, -1.0, 0.0));
							else if (fconfig.Read("y2_val", &val))
								(*planes)[3]->ChangePlane(Point(0.0, abs(val), 0.0),
								Vector(0.0, -1.0, 0.0));

							//z1
							if (fconfig.Read("z1_vali", &val))
								(*planes)[4]->ChangePlane(Point(0.0, 0.0, abs(val/iresz)),
								Vector(0.0, 0.0, 1.0));
							else if (fconfig.Read("z1_val", &val))
								(*planes)[4]->ChangePlane(Point(0.0, 0.0, abs(val)),
								Vector(0.0, 0.0, 1.0));

							//z2
							if (fconfig.Read("z2_vali", &val))
								(*planes)[5]->ChangePlane(Point(0.0, 0.0, abs(val/iresz)),
								Vector(0.0, 0.0, -1.0));
							else if (fconfig.Read("z2_val", &val))
								(*planes)[5]->ChangePlane(Point(0.0, 0.0, abs(val)),
								Vector(0.0, 0.0, -1.0));
						}

						//2d adjustment settings
						if (fconfig.Read("gamma", &str))
						{
							float r, g, b;
							if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)){
								FLIVR::Color col(r,g,b);
								vd->SetGamma(col);
							}
						}
						if (fconfig.Read("brightness", &str))
						{
							float r, g, b;
							if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)){
								FLIVR::Color col(r,g,b);
								vd->SetBrightness(col);
							}
						}
						if (fconfig.Read("hdr", &str))
						{
							float r, g, b;
							if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)){
								FLIVR::Color col(r,g,b);
								vd->SetHdr(col);
							}
						}
						bool bVal;
						if (fconfig.Read("sync_r", &bVal))
							vd->SetSyncR(bVal);
						if (fconfig.Read("sync_g", &bVal))
							vd->SetSyncG(bVal);
						if (fconfig.Read("sync_b", &bVal))
							vd->SetSyncB(bVal);

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
							vd->SetShadow(bVal);
						//shaodw intensity
						if (fconfig.Read("shadow_darkness", &dval))
							vd->SetShadowParams(dval);

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
					m_data_mgr.LoadMeshData(str);
				}
				MeshData* md = m_data_mgr.GetLastMeshData();
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
						Color amb(r, g, b);
						if (fconfig.Read("diffuse", &str))
							SSCANF(str.c_str(), "%f%f%f", &r, &g, &b);
						Color diff(r, g, b);
						if (fconfig.Read("specular", &str))
							SSCANF(str.c_str(), "%f%f%f", &r, &g, &b);
						Color spec(r, g, b);
						fconfig.Read("shininess", &shine, 30.0);
						fconfig.Read("alpha", &alpha, 0.5);
						md->SetMaterial(amb, diff, spec, shine, alpha);
						//2d adjusment settings
						if (fconfig.Read("gamma", &str))
						{
							float r, g, b;
							if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)){
								FLIVR::Color col(r,g,b);
								md->SetGamma(col);
							}
						}
						if (fconfig.Read("brightness", &str))
						{
							float r, g, b;
							if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)){
								FLIVR::Color col(r,g,b);
								md->SetBrightness(col);
							}
						}
						if (fconfig.Read("hdr", &str))
						{
							float r, g, b;
							if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)){
								FLIVR::Color col(r,g,b);
								md->SetHdr(col);
							}
						}
						bool bVal;
						if (fconfig.Read("sync_r", &bVal))
							md->SetSyncG(bVal);
						if (fconfig.Read("sync_g", &bVal))
							md->SetSyncG(bVal);
						if (fconfig.Read("sync_b", &bVal))
							md->SetSyncB(bVal);
						//shadow
						if (fconfig.Read("shadow", &bVal))
							md->SetShadow(bVal);
						double darkness;
						if (fconfig.Read("shadow_darkness", &darkness))
							md->SetShadowParams(darkness);

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
					m_data_mgr.LoadAnnotations(str);
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
			VRenderView* vrv = GetLastView();
			if (!vrv)
				continue;

			vrv->Clear();

			if (i==0 && m_setting_dlg && m_setting_dlg->GetTestMode(1))
				vrv->m_glview->m_test_speed = true;

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
						VolumeData* vd = m_data_mgr.GetVolumeData(str);
						if (vd)
							vrv->AddVolumeData(vd);
					}
				}
				vrv->SetVolPopDirty();
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
						MeshData* md = m_data_mgr.GetMeshData(str);
						if (md)
							vrv->AddMeshData(md);
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
										VolumeData* vd = m_data_mgr.GetVolumeData(str);
										if (vd)
											vrv->AddVolumeData(vd);
									}
								}
								break;
							case 3://mesh data
								{
									if (fconfig.Read("name", &str))
									{
										MeshData* md = m_data_mgr.GetMeshData(str);
										if (md)
											vrv->AddMeshData(md);
									}
								}
								break;
							case 4://annotations
								{
									if (fconfig.Read("name", &str))
									{
										Annotations* ann = m_data_mgr.GetAnnotations(str);
										if (ann)
											vrv->AddAnnotations(ann);
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
										str = vrv->AddGroup(str);
										DataGroup* group = vrv->GetGroup(str);
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
													FLIVR::Color col(r,g,b);
													group->SetGamma(col);
												}
											}
											if (fconfig.Read("brightness", &str))
											{
												float r, g, b;
												if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)){
													FLIVR::Color col(r,g,b);
													group->SetBrightness(col);
												}
											}
											if (fconfig.Read("hdr", &str))
											{
												float r, g, b;
												if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)){
													FLIVR::Color col(r,g,b);
													group->SetHdr(col);
												}
											}
											if (fconfig.Read("sync_r", &bVal))
												group->SetSyncR(bVal);
											if (fconfig.Read("sync_g", &bVal))
												group->SetSyncG(bVal);
											if (fconfig.Read("sync_b", &bVal))
												group->SetSyncB(bVal);
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
														VolumeData* vd = m_data_mgr.GetVolumeData(str);
														if (vd)
															group->InsertVolumeData(k-1, vd);
													}
												}
											}
										}
										vrv->SetVolPopDirty();
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
										str = vrv->AddMGroup(str);
										MeshGroup* group = vrv->GetMGroup(str);
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
														MeshData* md = m_data_mgr.GetMeshData(str);
														if (md)
															group->InsertMeshData(k-1, md);
													}
												}
											}
										}
										vrv->SetMeshPopDirty();
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
					vrv->SetDraw(draw);
				//properties
				bool persp;
				if (fconfig.Read("persp", &persp))
					vrv->SetPersp(persp);
				else
					vrv->SetPersp(true);
				bool free;
				if (fconfig.Read("free", &free))
					vrv->SetFree(free);
				else
					vrv->SetFree(false);
				double aov;
				if (fconfig.Read("aov", &aov))
					vrv->SetAov(aov);
				double nearclip;
				if (fconfig.Read("nearclip", &nearclip))
					vrv->SetNearClip(nearclip);
				double farclip;
				if (fconfig.Read("farclip", &farclip))
					vrv->SetFarClip(farclip);
				if (fconfig.Read("backgroundcolor", &str))
				{
					float r, g, b;
					if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)){
						FLIVR::Color col(r,g,b);
						vrv->SetBackgroundColor(col);
					}
				}
				int volmethod;
				if (fconfig.Read("volmethod", &volmethod))
					vrv->SetVolMethod(volmethod);
				int peellayers;
				if (fconfig.Read("peellayers", &peellayers))
					vrv->SetPeelingLayers(peellayers);
				bool fog;
				if (fconfig.Read("fog", &fog))
					vrv->SetFog(fog);
				double fogintensity;
				if (fconfig.Read("fogintensity", &fogintensity))
					vrv->m_depth_atten_factor_text->SetValue(wxString::Format("%.2f",fogintensity));
				if (fconfig.Read("draw_camctr", &bVal))
				{
					vrv->m_glview->m_draw_camctr = bVal;
					vrv->m_options_toolbar->ToggleTool(VRenderView::ID_CamCtrChk,bVal);
				}
				if (fconfig.Read("draw_info", &iVal))
				{
					vrv->m_glview->m_draw_info = iVal;
					vrv->m_options_toolbar->ToggleTool(VRenderView::ID_FpsChk, iVal & INFO_DISP);
				}
				if (fconfig.Read("draw_legend", &bVal))
				{
					vrv->m_glview->m_draw_legend = bVal;
					vrv->m_options_toolbar->ToggleTool(VRenderView::ID_LegendChk,bVal);
				}

				//camera
				if (fconfig.Read("translation", &str))
				{
					if (SSCANF(str.c_str(), "%f%f%f", &x, &y, &z))
						vrv->SetTranslations(x, y, z);
				}
				if (fconfig.Read("rotation", &str))
				{
					if (SSCANF(str.c_str(), "%f%f%f", &x, &y, &z))
						vrv->SetRotations(x, y, z);
				}
				if (fconfig.Read("zero_quat", &str))
				{
					if (SSCANF(str.c_str(), "%f%f%f%f", &x, &y, &z, &w))
						vrv->m_glview->SetZeroQuat(x, y, z, w);
				}
				if (fconfig.Read("center", &str))
				{
					if (SSCANF(str.c_str(), "%f%f%f", &x, &y, &z))
						vrv->SetCenters(x, y, z);
				}
				double dist;
				if (fconfig.Read("centereyedist", &dist))
					vrv->SetCenterEyeDist(dist);
				double radius = 5.0;
				if (fconfig.Read("radius", &radius))
					vrv->SetRadius(radius);
				double initdist;
				if (fconfig.Read("initdist", &initdist))
					vrv->m_glview->SetInitDist(initdist);
				else
					vrv->m_glview->SetInitDist(radius/tan(d2r(vrv->GetAov()/2.0)));
				bool scale_mode;
				if (fconfig.Read("scale_mode", &scale_mode))
					vrv->SetScaleMode(scale_mode, false);
				double scale;
				if (!fconfig.Read("scale", &scale))
					scale = radius / tan(d2r(vrv->GetAov() / 2.0)) / dist;
				vrv->m_glview->m_scale_factor = scale;
				vrv->UpdateScaleFactor(false);
				bool pin_rot_center;
				if (fconfig.Read("pin_rot_center", &pin_rot_center))
				{
					vrv->m_glview->m_pin_rot_center = pin_rot_center;
					if (pin_rot_center)
						vrv->m_glview->m_rot_center_dirty = true;
				}
				//object
				if (fconfig.Read("obj_center", &str))
				{
					if (SSCANF(str.c_str(), "%f%f%f", &x, &y, &z))
						vrv->SetObjCenters(x, y, z);
				}
				if (fconfig.Read("obj_trans", &str))
				{
					if (SSCANF(str.c_str(), "%f%f%f", &x, &y, &z))
						vrv->SetObjTrans(x, y, z);
				}
				if (fconfig.Read("obj_rot", &str))
				{
					if (SSCANF(str.c_str(), "%f%f%f", &x, &y, &z))
					{
						if (l_major <= 2 && d_minor < 24.3)
							vrv->SetObjRot(x, y+180.0, z+180.0);
						else
							vrv->SetObjRot(x, y, z);
					}
				}
				//scale bar
				bool disp;
				if (fconfig.Read("disp_scale_bar", &disp))
					vrv->m_glview->m_disp_scale_bar = disp;
				if (fconfig.Read("disp_scale_bar_text", &disp))
					vrv->m_glview->m_disp_scale_bar_text = disp;
				double length;
				if (fconfig.Read("sb_length", &length))
					vrv->m_glview->m_sb_length = length;
				if (fconfig.Read("sb_text", &str))
					vrv->m_glview->m_sb_text = str;
				if (fconfig.Read("sb_num", &str))
					vrv->m_glview->m_sb_num = str;
				int unit;
				if (fconfig.Read("sb_unit", &unit))
					vrv->m_glview->m_sb_unit = unit;

				//2d sdjustment settings
				if (fconfig.Read("gamma", &str))
				{
					float r, g, b;
					if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)){
						FLIVR::Color col(r,g,b);
						vrv->m_glview->SetGamma(col);
					}
				}
				if (fconfig.Read("brightness", &str))
				{
					float r, g, b;
					if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)){
						FLIVR::Color col(r,g,b);
						vrv->m_glview->SetBrightness(col);
					}
				}
				if (fconfig.Read("hdr", &str))
				{
					float r, g, b;
					if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)){
						FLIVR::Color col(r,g,b);
						vrv->m_glview->SetHdr(col);
					}
				}
				if (fconfig.Read("sync_r", &bVal))
					vrv->m_glview->SetSyncR(bVal);
				if (fconfig.Read("sync_g", &bVal))
					vrv->m_glview->SetSyncG(bVal);
				if (fconfig.Read("sync_b", &bVal))
					vrv->m_glview->SetSyncB(bVal);

				//clipping plane rotations
				int clip_mode;
				if (fconfig.Read("clip_mode", &clip_mode))
					vrv->SetClipMode(clip_mode);
				double rotx_cl, roty_cl, rotz_cl;
				if (fconfig.Read("rotx_cl", &rotx_cl) &&
					fconfig.Read("roty_cl", &roty_cl) &&
					fconfig.Read("rotz_cl", &rotz_cl))
				{
					vrv->SetClippingPlaneRotations(rotx_cl, roty_cl, rotz_cl);
					m_clip_view->SetClippingPlaneRotations(rotx_cl, roty_cl, rotz_cl);
				}

				//painting parameters
				double dVal;
				FL::VolumeSelector* selector = vrv->GetVolumeSelector();
				if (selector)
				{
					if (fconfig.Read("brush_use_pres", &bVal))
						selector->SetBrushUsePres(bVal);
					double size1, size2;
					if (fconfig.Read("brush_size_1", &size1) &&
						fconfig.Read("brush_size_2", &size2))
						selector->SetBrushSize(size1, size2);
					if (fconfig.Read("brush_spacing", &dVal))
						selector->SetBrushSpacing(dVal);
					if (fconfig.Read("brush_iteration", &dVal))
						selector->SetBrushIteration(dVal);
					if (fconfig.Read("brush_size_data", &bVal))
						selector->SetBrushSizeData(bVal);
					if (fconfig.Read("brush_translate", &dVal))
						selector->SetBrushSclTranslate(dVal);
					if (fconfig.Read("w2d", &dVal))
						selector->SetW2d(dVal);
				}

				//rulers
				if (vrv->GetRulerList() &&
					fconfig.Exists(wxString::Format("/views/%d/rulers", i)))
				{
					fconfig.SetPath(wxString::Format("/views/%d/rulers", i));
					vrv->GetRulerHandler()->Read(fconfig, i);
				}
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
				OnSelection(2, 0, 0, m_data_mgr.GetVolumeData(cur_sel_vol));
				break;
			case 3:  //mesh
				OnSelection(3, 0, 0, 0, m_data_mgr.GetMeshData(cur_sel_mesh));
				break;
			}
		}
		else if (fconfig.Read("cur_sel_vol", &cur_sel_vol))
		{
			m_cur_sel_vol = cur_sel_vol;
			if (m_cur_sel_vol != -1)
			{
				VolumeData* vd = m_data_mgr.GetVolumeData(m_cur_sel_vol);
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

		//set settings for frame
		VRenderView* vrv = 0;
		if (fconfig.Read("cur_page", &iVal))
		{
			m_movie_view->SetCurrentPage(iVal);
		}
		if (fconfig.Read("views_cmb", &iVal))
		{
			m_movie_view->m_views_cmb->SetSelection(iVal);
			m_mov_view = iVal;
			vrv = (*GetViewList())[m_mov_view];
		}
		if (fconfig.Read("rot_check", &bVal))
		{
			if (bVal)
				m_movie_view->EnableRot();
			else
				m_movie_view->DisableRot();
		}
		if (fconfig.Read("seq_check", &bVal))
		{
			if (bVal)
				m_movie_view->EnableTime();
			else
				m_movie_view->DisableTime();
		}
		if (fconfig.Read("x_rd", &bVal))
		{
			m_movie_view->m_x_rd->SetValue(bVal);
			if (bVal)
				m_mov_axis = 1;
		}
		if (fconfig.Read("y_rd", &bVal))
		{
			m_movie_view->m_y_rd->SetValue(bVal);
			if (bVal)
				m_mov_axis = 2;
		}
		if (fconfig.Read("angle_end_text", &sVal))
		{
			m_movie_view->m_degree_end->SetValue(sVal);
			m_mov_angle_end = sVal;
		}
		if (fconfig.Read("step_text", &sVal))
		{
			m_movie_view->m_movie_time->SetValue(sVal);
			m_mov_step = sVal;
		}
		if (fconfig.Read("frames_text", &sVal))
		{
			m_movie_view->m_fps_text->SetValue(sVal);
			m_mov_frames = sVal;
		}
		if (fconfig.Read("frame_chk", &bVal))
		{
			m_movie_view->m_frame_chk->SetValue(bVal);
			if (vrv)
			{
				if (bVal)
					vrv->EnableFrame();
				else
					vrv->DisableFrame();
			}
		}
		long frame_x, frame_y, frame_w, frame_h;
		bool b_x, b_y, b_w, b_h;
		b_x = b_y = b_w = b_h = false;
		if (fconfig.Read("center_x_text", &sVal) && sVal!="")
		{
			m_movie_view->m_center_x_text->SetValue(sVal);
			sVal.ToLong(&frame_x);
			b_x = true;
		}
		if (fconfig.Read("center_y_text", &sVal) && sVal!="")
		{
			m_movie_view->m_center_y_text->SetValue(sVal);
			sVal.ToLong(&frame_y);
			b_y = true;
		}
		if (fconfig.Read("width_text", &sVal) && sVal!="")
		{
			m_movie_view->m_width_text->SetValue(sVal);
			sVal.ToLong(&frame_w);
			b_w = true;
		}
		if (fconfig.Read("height_text", &sVal) && sVal!="")
		{
			m_movie_view->m_height_text->SetValue(sVal);
			sVal.ToLong(&frame_h);
			b_h = true;
		}
		if (vrv && b_x && b_y && b_w && b_h)
		{
			vrv->SetFrame(int(frame_x-frame_w/2.0+0.5),
				int(frame_y-frame_h/2.0+0.5),
				frame_w, frame_h);
		}
		if (fconfig.Read("time_start_text", &sVal))
			m_movie_view->m_time_start_text->SetValue(sVal);
		if (fconfig.Read("time_end_text", &sVal))
			m_movie_view->m_time_end_text->SetValue(sVal);
		if (fconfig.Read("run_script", &bVal))
			m_setting_dlg->SetRunScript(bVal);
		if (fconfig.Read("script_file", &sVal))
			m_setting_dlg->SetScriptFile(sVal);
		m_movie_view->GetScriptSettings();
	}

	//tracking diag
	if (fconfig.Exists("/track_diag"))
	{
		fconfig.SetPath("/track_diag");
		wxString sVal;
		if (fconfig.Read("track_file", &sVal))
		{
			m_trace_dlg->GetSettings(m_vrv_list[0]);
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
		bool bVal;

		if (fconfig.Read("ui_main_tb", &bVal))
		{
			if (bVal)
			{
				m_aui_mgr.GetPane(m_main_tb).Show();
				bool fl;
				if (fconfig.Read("ui_main_tb_float", &fl))
				{
					if (fl)
						m_aui_mgr.GetPane(m_main_tb).Float();
					else
						m_aui_mgr.GetPane(m_main_tb).Dock();
				}
			}
			else
			{
				if (m_aui_mgr.GetPane(m_main_tb).IsOk())
					m_aui_mgr.GetPane(m_main_tb).Hide();
			}
		}
		if (fconfig.Read("ui_list_view", &bVal))
		{
			if (bVal)
			{
				m_aui_mgr.GetPane(m_list_panel).Show();
				m_tb_menu_ui->Check(ID_UIListView, true);
				bool fl;
				if (fconfig.Read("ui_list_view_float", &fl))
				{
					if (fl)
						m_aui_mgr.GetPane(m_list_panel).Float();
					else
						m_aui_mgr.GetPane(m_list_panel).Dock();
				}
			}
			else
			{
				if (m_aui_mgr.GetPane(m_list_panel).IsOk())
					m_aui_mgr.GetPane(m_list_panel).Hide();
				m_tb_menu_ui->Check(ID_UIListView, false);
			}
		}
		if (fconfig.Read("ui_tree_view", &bVal))
		{
			if (bVal)
			{
				m_aui_mgr.GetPane(m_tree_panel).Show();
				m_tb_menu_ui->Check(ID_UITreeView, true);
				bool fl;
				if (fconfig.Read("ui_tree_view_float", &fl))
				{
					if (fl)
						m_aui_mgr.GetPane(m_tree_panel).Float();
					else
						m_aui_mgr.GetPane(m_tree_panel).Dock();
				}
			}
			else
			{
				if (m_aui_mgr.GetPane(m_tree_panel).IsOk())
					m_aui_mgr.GetPane(m_tree_panel).Hide();
				m_tb_menu_ui->Check(ID_UITreeView, false);
			}
		}
		if (fconfig.Read("ui_movie_view", &bVal))
		{
			if (bVal)
			{
				m_aui_mgr.GetPane(m_movie_view).Show();
				m_tb_menu_ui->Check(ID_UIMovieView, true);
				bool fl;
				if (fconfig.Read("ui_movie_view_float", &fl))
				{
					if (fl)
						m_aui_mgr.GetPane(m_movie_view).Float();
					else
						m_aui_mgr.GetPane(m_movie_view).Dock();
				}
			}
			else
			{
				if (m_aui_mgr.GetPane(m_movie_view).IsOk())
					m_aui_mgr.GetPane(m_movie_view).Hide();
				m_tb_menu_ui->Check(ID_UIMovieView, false);
			}
		}
		if (fconfig.Read("ui_adjust_view", &bVal))
		{
			if (bVal)
			{
				m_aui_mgr.GetPane(m_adjust_view).Show();
				m_tb_menu_ui->Check(ID_UIAdjView, true);
				bool fl;
				if (fconfig.Read("ui_adjust_view_float", &fl))
				{
					if (fl)
						m_aui_mgr.GetPane(m_adjust_view).Float();
					else
						m_aui_mgr.GetPane(m_adjust_view).Dock();
				}
			}
			else
			{
				if (m_aui_mgr.GetPane(m_adjust_view).IsOk())
					m_aui_mgr.GetPane(m_adjust_view).Hide();
				m_tb_menu_ui->Check(ID_UIAdjView, false);
			}
		}
		if (fconfig.Read("ui_clip_view", &bVal))
		{
			if (bVal)
			{
				m_aui_mgr.GetPane(m_clip_view).Show();
				m_tb_menu_ui->Check(ID_UIClipView, true);
				bool fl;
				if (fconfig.Read("ui_clip_view_float", &fl))
				{
					if (fl)
						m_aui_mgr.GetPane(m_clip_view).Float();
					else
						m_aui_mgr.GetPane(m_clip_view).Dock();
				}
			}
			else
			{
				if (m_aui_mgr.GetPane(m_clip_view).IsOk())
					m_aui_mgr.GetPane(m_clip_view).Hide();
				m_tb_menu_ui->Check(ID_UIClipView, false);
			}
		}
		if (fconfig.Read("ui_prop_view", &bVal))
		{
			if (bVal)
			{
				m_aui_mgr.GetPane(m_prop_panel).Show();
				m_tb_menu_ui->Check(ID_UIPropView, true);
				bool fl;
				if (fconfig.Read("ui_prop_view_float", &fl))
				{
					if (fl)
						m_aui_mgr.GetPane(m_prop_panel).Float();
					else
						m_aui_mgr.GetPane(m_prop_panel).Dock();
				}
			}
			else
			{
				if (m_aui_mgr.GetPane(m_prop_panel).IsOk())
					m_aui_mgr.GetPane(m_prop_panel).Hide();
				m_tb_menu_ui->Check(ID_UIPropView, false);
			}
		}

		m_aui_mgr.Update();
	}

	//interpolator
	if (fconfig.Exists("/interpolator"))
	{
		wxString str;
		wxString sVal;
		double dVal;

		fconfig.SetPath("/interpolator");
		m_interpolator.Clear();
		if (fconfig.Read("max_id", &iVal))
			Interpolator::m_id = iVal;
		vector<FlKeyGroup*>* key_list = m_interpolator.GetKeyList();
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
												Quaternion qval = Quaternion(x, y, z, w);
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
								}
							}
						}
					}
				}
				key_list->push_back(key_group);
			}
		}
		m_recorder_dlg->UpdateList();
	}

	UpdateList();
	if (m_cur_sel_type != -1)
	{
		switch (m_cur_sel_type)
		{
		case 2:  //volume
			if (m_data_mgr.GetVolumeData(m_cur_sel_vol))
				UpdateTree(m_data_mgr.GetVolumeData(m_cur_sel_vol)->GetName());
			else
				UpdateTree();
			break;
		case 3:  //mesh
			if (m_data_mgr.GetMeshData(m_cur_sel_mesh))
				UpdateTree(m_data_mgr.GetMeshData(m_cur_sel_mesh)->GetName());
			else
				UpdateTree();
			break;
		default:
			UpdateTree();
		}
	}
	else if (m_cur_sel_vol != -1)
	{
		if (m_data_mgr.GetVolumeData(m_cur_sel_vol))
			UpdateTree(m_data_mgr.GetVolumeData(m_cur_sel_vol)->GetName());
		else
			UpdateTree();
	}
	else
		UpdateTree();

	if (m_movie_view)
		m_movie_view->SetView(0);
	delete prg_diag;

	RefreshVRenderViews(true, true);
}

void VRenderFrame::OnSettings(wxCommandEvent& WXUNUSED(event))
{
	m_aui_mgr.GetPane(m_setting_dlg).Show();
	m_aui_mgr.GetPane(m_setting_dlg).Float();
	m_aui_mgr.Update();
}

//tools
void VRenderFrame::OnLastTool(wxCommandEvent& WXUNUSED(event))
{
	if (!m_setting_dlg)
	{
		ShowPaintTool();
		return;
	}

	unsigned int tool = m_setting_dlg->GetLastTool();
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
	}
}

void VRenderFrame::OnPaintTool(wxCommandEvent& WXUNUSED(event))
{
	ShowPaintTool();
}

void VRenderFrame::OnMeasure(wxCommandEvent& WXUNUSED(event))
{
	ShowMeasureDlg();
}

void VRenderFrame::OnNoiseCancelling(wxCommandEvent& WXUNUSED(event))
{
	ShowNoiseCancellingDlg();
}

void VRenderFrame::OnCounting(wxCommandEvent& WXUNUSED(event))
{
	ShowCountingDlg();
}

void VRenderFrame::OnColocalization(wxCommandEvent& WXUNUSED(event))
{
	ShowColocalizationDlg();
}

void VRenderFrame::OnConvert(wxCommandEvent& WXUNUSED(event))
{
	ShowConvertDlg();
}

void VRenderFrame::OnTrace(wxCommandEvent& WXUNUSED(event))
{
	ShowTraceDlg();
}

void VRenderFrame::OnOcl(wxCommandEvent& WXUNUSED(event))
{
	ShowOclDlg();
}

void VRenderFrame::OnComponent(wxCommandEvent& WXUNUSED(event))
{
	ShowComponentDlg();
}

void VRenderFrame::OnCalculations(wxCommandEvent& WXUNUSED(event))
{
	ShowCalculationDlg();
}

void VRenderFrame::ShowPaintTool()
{
	m_aui_mgr.GetPane(m_brush_tool_dlg).Show();
	m_aui_mgr.GetPane(m_brush_tool_dlg).Float();
	m_aui_mgr.Update();
	if (m_setting_dlg)
		m_setting_dlg->SetLastTool(TOOL_PAINT_BRUSH);
	m_main_tb->SetToolNormalBitmap(ID_LastTool,
		wxGetBitmapFromMemory(icon_paint_brush));
}

void VRenderFrame::ShowMeasureDlg()
{
	m_aui_mgr.GetPane(m_measure_dlg).Show();
	m_aui_mgr.GetPane(m_measure_dlg).Float();
	m_aui_mgr.Update();
	if (m_setting_dlg)
		m_setting_dlg->SetLastTool(TOOL_MEASUREMENT);
	m_main_tb->SetToolNormalBitmap(ID_LastTool,
		wxGetBitmapFromMemory(icon_measurement));
}

void VRenderFrame::ShowTraceDlg()
{
	m_aui_mgr.GetPane(m_trace_dlg).Show();
	m_aui_mgr.GetPane(m_trace_dlg).Float();
	m_aui_mgr.Update();
	if (m_setting_dlg)
		m_setting_dlg->SetLastTool(TOOL_TRACKING);
	m_main_tb->SetToolNormalBitmap(ID_LastTool,
		wxGetBitmapFromMemory(icon_tracking));
}

void VRenderFrame::ShowNoiseCancellingDlg()
{
	m_aui_mgr.GetPane(m_noise_cancelling_dlg).Show();
	m_aui_mgr.GetPane(m_noise_cancelling_dlg).Float();
	m_aui_mgr.Update();
	if (m_setting_dlg)
		m_setting_dlg->SetLastTool(TOOL_NOISE_REDUCTION);
	m_main_tb->SetToolNormalBitmap(ID_LastTool,
		wxGetBitmapFromMemory(icon_noise_reduc));
}

void VRenderFrame::ShowCountingDlg()
{
	m_aui_mgr.GetPane(m_counting_dlg).Show();
	m_aui_mgr.GetPane(m_counting_dlg).Float();
	m_aui_mgr.Update();
	if (m_setting_dlg)
		m_setting_dlg->SetLastTool(TOOL_VOLUME_SIZE);
	m_main_tb->SetToolNormalBitmap(ID_LastTool,
		wxGetBitmapFromMemory(icon_volume_size));
}

void VRenderFrame::ShowColocalizationDlg()
{
	m_aui_mgr.GetPane(m_colocalization_dlg).Show();
	m_aui_mgr.GetPane(m_colocalization_dlg).Float();
	m_aui_mgr.Update();
	if (m_setting_dlg)
		m_setting_dlg->SetLastTool(TOOL_COLOCALIZATION);
	m_main_tb->SetToolNormalBitmap(ID_LastTool,
		wxGetBitmapFromMemory(icon_colocalization));
}

void VRenderFrame::ShowConvertDlg()
{
	m_aui_mgr.GetPane(m_convert_dlg).Show();
	m_aui_mgr.GetPane(m_convert_dlg).Float();
	m_aui_mgr.Update();
	if (m_setting_dlg)
		m_setting_dlg->SetLastTool(TOOL_CONVERT);
	m_main_tb->SetToolNormalBitmap(ID_LastTool,
		wxGetBitmapFromMemory(icon_convert));
}

void VRenderFrame::ShowOclDlg()
{
	m_aui_mgr.GetPane(m_ocl_dlg).Show();
	m_aui_mgr.GetPane(m_ocl_dlg).Float();
	m_aui_mgr.Update();
	if (m_setting_dlg)
		m_setting_dlg->SetLastTool(TOOL_OPENCL);
	m_main_tb->SetToolNormalBitmap(ID_LastTool,
		wxGetBitmapFromMemory(icon_opencl));
}

void VRenderFrame::ShowComponentDlg()
{
	m_aui_mgr.GetPane(m_component_dlg).Show();
	m_aui_mgr.GetPane(m_component_dlg).Float();
	m_aui_mgr.Update();
	if (m_setting_dlg)
		m_setting_dlg->SetLastTool(TOOL_COMPONENT);
	m_main_tb->SetToolNormalBitmap(ID_LastTool,
		wxGetBitmapFromMemory(icon_components));
}

void VRenderFrame::ShowCalculationDlg()
{
	m_aui_mgr.GetPane(m_calculation_dlg).Show();
	m_aui_mgr.GetPane(m_calculation_dlg).Float();
	m_aui_mgr.Update();
	if (m_setting_dlg)
		m_setting_dlg->SetLastTool(TOOL_CALCULATIONS);
	m_main_tb->SetToolNormalBitmap(ID_LastTool,
		wxGetBitmapFromMemory(icon_calculations));
}

void VRenderFrame::SetTextureUndos()
{
	if (m_setting_dlg)
		Texture::mask_undo_num_ = (size_t)(m_setting_dlg->GetPaintHistDepth());
}

void VRenderFrame::SetTextureRendererSettings()
{
	if (!m_setting_dlg)
		return;

	TextureRenderer::set_mem_swap(m_setting_dlg->GetMemSwap());
	bool use_mem_limit = false;
	GLenum error = glGetError();
	GLint mem_info[4] = {0, 0, 0, 0};
	glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, mem_info);
	error = glGetError();
	if (error == GL_INVALID_ENUM)
	{
		glGetIntegerv(GL_TEXTURE_FREE_MEMORY_ATI, mem_info);
		error = glGetError();
		if (error == GL_INVALID_ENUM)
			use_mem_limit = true;
	}
	if (m_setting_dlg->GetGraphicsMem() > mem_info[0]/1024.0)
		use_mem_limit = true;
	TextureRenderer::set_use_mem_limit(use_mem_limit);
	TextureRenderer::set_mem_limit(use_mem_limit?
		m_setting_dlg->GetGraphicsMem():mem_info[0]/1024.0);
	TextureRenderer::set_available_mem(use_mem_limit?
		m_setting_dlg->GetGraphicsMem():mem_info[0]/1024.0);
	TextureRenderer::set_large_data_size(m_setting_dlg->GetLargeDataSize());
	TextureRenderer::set_force_brick_size(m_setting_dlg->GetForceBrickSize());
	TextureRenderer::set_up_time(m_setting_dlg->GetResponseTime());
	TextureRenderer::set_update_order(m_setting_dlg->GetUpdateOrder());
	TextureRenderer::set_invalidate_tex(m_setting_dlg->GetInvalidateTex());
}

void VRenderFrame::OnFacebook(wxCommandEvent& WXUNUSED(event))
{
	::wxLaunchDefaultBrowser(FACEBOOK_URL);
}

void VRenderFrame::OnManual(wxCommandEvent& WXUNUSED(event))
{
	::wxLaunchDefaultBrowser(HELP_MANUAL);
}


void VRenderFrame::OnTutorial(wxCommandEvent& WXUNUSED(event))
{
	::wxLaunchDefaultBrowser(HELP_TUTORIAL);
}

void VRenderFrame::OnTwitter(wxCommandEvent& WXUNUSED(event))
{
	::wxLaunchDefaultBrowser(TWITTER_URL);
}

void VRenderFrame::OnYoutube(wxCommandEvent& WXUNUSED(event))
{
	::wxLaunchDefaultBrowser(YOUTUBE_URL);
}

void VRenderFrame::OnShowHideUI(wxCommandEvent& WXUNUSED(event))
{
	ToggleAllTools(true);
}

void VRenderFrame::OnShowHideToolbar(wxCommandEvent& WXUNUSED(event))
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

void VRenderFrame::OnShowHideView(wxCommandEvent &event)
{
	int id = event.GetId();

	switch (id)
	{
	case ID_UIListView:
		//data view
		if (m_aui_mgr.GetPane(m_list_panel).IsShown())
		{
			m_aui_mgr.GetPane(m_list_panel).Hide();
			m_tb_menu_ui->Check(ID_UIListView, false);
		}
		else
		{
			m_aui_mgr.GetPane(m_list_panel).Show();
			m_tb_menu_ui->Check(ID_UIListView, true);
		}
		break;
	case ID_UITreeView:
		//scene view
		if (m_aui_mgr.GetPane(m_tree_panel).IsShown())
		{
			m_aui_mgr.GetPane(m_tree_panel).Hide();
			m_tb_menu_ui->Check(ID_UITreeView, false);
		}
		else
		{
			m_aui_mgr.GetPane(m_tree_panel).Show();
			m_tb_menu_ui->Check(ID_UITreeView, true);
		}
		break;
	case ID_UIMovieView:
		//movie view
		if (m_aui_mgr.GetPane(m_movie_view).IsShown())
		{
			m_aui_mgr.GetPane(m_movie_view).Hide();
			m_tb_menu_ui->Check(ID_UIMovieView, false);
		}
		else
		{
			m_aui_mgr.GetPane(m_movie_view).Show();
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

//panes
void VRenderFrame::OnPaneClose(wxAuiManagerEvent& event)
{
	wxWindow* wnd = event.pane->window;
	wxString name = wnd->GetName();

	if (name == "ListPanel")
		m_tb_menu_ui->Check(ID_UIListView, false);
	else if (name == "TreePanel")
		m_tb_menu_ui->Check(ID_UITreeView, false);
	else if (name == "VMovieView")
		m_tb_menu_ui->Check(ID_UIMovieView, false);
	else if (name == "PropPanel")
		m_tb_menu_ui->Check(ID_UIPropView, false);
	else if (name == "AdjustView")
		m_tb_menu_ui->Check(ID_UIAdjView, false);
	else if (name == "ClippingView")
		m_tb_menu_ui->Check(ID_UIClipView, false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void VRenderFrame::OnCreateCube(wxCommandEvent& WXUNUSED(event))
{
}

void VRenderFrame::OnCreateSphere(wxCommandEvent& WXUNUSED(event))
{
}

void VRenderFrame::OnCreateCone(wxCommandEvent& WXUNUSED(event))
{
}

void VRenderFrame::OnDraw(wxPaintEvent& event)
{
	//wxPaintDC dc(this);

	//wxRect windowRect(wxPoint(0, 0), GetClientSize());
	//dc.SetBrush(wxBrush(*wxGREEN, wxSOLID));
	//dc.DrawRectangle(windowRect);
}

void VRenderFrame::OnKeyDown(wxKeyEvent& event)
{
	event.Skip();
}

