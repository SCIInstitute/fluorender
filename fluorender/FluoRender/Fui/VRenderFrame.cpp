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
#include "VRenderFrame.h"
#include "DragDrop.h"
#include <Global.hpp>
#include <Root.hpp>
#include <Renderview.hpp>
#include <VolumeData.hpp>
#include <VolumeGroup.hpp>
#include <MeshData.hpp>
#include <MeshGroup.hpp>
#include <Annotations.hpp>
#include <VolumeFactory.hpp>
#include <Formats/msk_writer.h>
#include <Formats/msk_reader.h>
#include <Converters/VolumeMeshConv.h>
#include <Selection/VolumeSelector.h>
#include <FLIVR/TextRenderer.h>
#include <FLIVR/VertexArray.h>
#include <FLIVR/Framebuffer.h>
#include <FLIVR/VolShader.h>
#include <FLIVR/SegShader.h>
#include <FLIVR/VolCalShader.h>
#include <FLIVR/VolumeRenderer.h>
#include <FLIVR/MultiVolumeRenderer.h>
#include <FLIVR/KernelProgram.h>
#include <FLIVR/VolKernel.h>
#include <compatibility.h>
#include <wx/artprov.h>
#include <wx/wfstream.h>
#include <wx/fileconf.h>
#include <wx/aboutdlg.h>
#include <wx/progdlg.h>
#include <wx/hyperlink.h>
#include <wx/stdpaths.h>
#include <png_resource.h>
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
int VRenderFrame::m_ser_num = 0;
bool VRenderFrame::m_compression = false;
bool VRenderFrame::m_skip_brick = false;
wxString VRenderFrame::m_time_id = "_T";
bool VRenderFrame::m_load_mask = true;
bool VRenderFrame::m_save_crop = false;
int VRenderFrame::m_save_filter = 0;
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
	m_setting_dlg = new SettingDlg(this);

	// tell wxAuiManager to manage this frame
	m_aui_mgr.SetManagedWindow(this);

	// set frame icon
	wxIcon icon;
	icon.CopyFromBitmap(wxGetBitmapFromMemory(icon_32));
	SetIcon(icon);

	glbin.initIcons();

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
	VRenderView *vrv = new VRenderView(this);
	//vrv->m_glview->InitView();
	//vrv->UpdateView();
	m_vrv_list.push_back(vrv);

#ifdef _WIN32
	wxSize panel_size(350, 300);
#else
	wxSize panel_size(400, 300);
#endif
	//create list view
	m_list_panel = new ListPanel(this,
		wxDefaultPosition, panel_size);

	//create tree view
	m_tree_panel = new TreePanel(this,
		wxDefaultPosition, panel_size);

	//create movie view (sets the m_recorder_dlg)
	m_movie_view = new VMovieView(this,
		wxDefaultPosition, panel_size);

	//create prop panel
	m_prop_panel = new wxPanel(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, 0, "PropPanel");
	//prop panel chidren
	m_prop_sizer = new wxBoxSizer(wxHORIZONTAL);
	m_volume_prop = new VPropView(this, m_prop_panel);
	m_mesh_prop = new MPropView(this, m_prop_panel);
	m_mesh_manip = new MManipulator(this, m_prop_panel);
	m_annotation_prop = new APropView(this, m_prop_panel);
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
	m_clip_view = new ClippingView(this,
		wxDefaultPosition, wxSize(130,700));
	m_clip_view->SetDataManager(&m_data_mgr);
	m_clip_view->SetPlaneMode(static_cast<PLANE_MODES>(
		m_setting_dlg->GetPlaneMode()));

	//adjust view
	m_adjust_view = new AdjustView(this,
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
	flvr::TextRenderer::text_texture_manager_.load_face(font_file.ToStdString());
	flvr::TextRenderer::text_texture_manager_.SetSize(m_setting_dlg->GetTextSize());

	//settings dialog
	//if (m_setting_dlg->GetTestMode(1))
	//	m_vrv_list[0]->m_glview->m_test_speed = true;
	//if (m_setting_dlg->GetTestMode(3))
	//{
	//	m_vrv_list[0]->m_glview->m_test_wiref = true;
	//	m_vrv_list[0]->m_glview->m_draw_bounds = true;
	//	m_vrv_list[0]->m_glview->m_draw_grid = true;
	//	m_data_mgr.m_vol_test_wiref = true;
	//}
	//int c1 = m_setting_dlg->GetWavelengthColor(1);
	//int c2 = m_setting_dlg->GetWavelengthColor(2);
	//int c3 = m_setting_dlg->GetWavelengthColor(3);
	//int c4 = m_setting_dlg->GetWavelengthColor(4);
	//if (c1 && c2 && c3 && c4)
	//	m_data_mgr.SetWavelengthColor(c1, c2, c3, c4);
	//m_vrv_list[0]->SetPinThreshold(m_setting_dlg->GetPinThreshold());
	//m_vrv_list[0]->m_glview->SetPeelingLayers(m_setting_dlg->GetPeelingLyers());
	//m_vrv_list[0]->m_glview->SetBlendSlices(m_setting_dlg->GetMicroBlend());
	//m_vrv_list[0]->m_glview->SetAdaptive(m_setting_dlg->GetMouseInt());
	//m_vrv_list[0]->m_glview->SetGradBg(m_setting_dlg->GetGradBg());
	//m_vrv_list[0]->m_glview->SetPointVolumeMode(m_setting_dlg->GetPointVolumeMode());
	//m_vrv_list[0]->m_glview->SetRulerUseTransf(m_setting_dlg->GetRulerUseTransf());
	//m_vrv_list[0]->m_glview->SetRulerTimeDep(m_setting_dlg->GetRulerTimeDep());
	//m_vrv_list[0]->m_glview->SetStereo(m_setting_dlg->GetStereo());
	//m_vrv_list[0]->m_glview->SetEyeDist(m_setting_dlg->GetEyeDist());
	//if (m_setting_dlg->GetStereo()) m_vrv_list[0]->InitOpenVR();
	m_time_id = m_setting_dlg->GetTimeId();
	m_data_mgr.SetOverrideVox(m_setting_dlg->GetOverrideVox());
	m_data_mgr.SetPvxmlFlipX(m_setting_dlg->GetPvxmlFlipX());
	m_data_mgr.SetPvxmlFlipY(m_setting_dlg->GetPvxmlFlipY());
	m_data_mgr.SetPvxmlSeqType(m_setting_dlg->GetPvxmlSeqType());
	flvr::VolumeRenderer::set_soft_threshold(m_setting_dlg->GetSoftThreshold());
	flvr::MultiVolumeRenderer::set_soft_threshold(m_setting_dlg->GetSoftThreshold());
	//TreeLayer::SetSoftThreshsold(m_setting_dlg->GetSoftThreshold());
	VolumeMeshConv::SetSoftThreshold(m_setting_dlg->GetSoftThreshold());

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
	m_trace_dlg->SetCellSize(m_setting_dlg->GetComponentSize());

	//ocl dialog
	m_ocl_dlg = new OclDlg(this);

	//component dialog
	m_component_dlg = new ComponentDlg(this);

	//calculation dialog
	m_calculation_dlg = new CalculationDlg(this);

	//help dialog
	m_help_dlg = new HelpDlg(this);
	//m_help_dlg->LoadPage("C:\\!wanyong!\\TEMP\\wxHtmlWindow.htm");

	//tester
	//shown for testing parameters
	m_tester = new TesterDlg(this);
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
		Left().CloseButton(true).BestSize(panel_size).
		FloatingSize(wxSize(400, 600)).Layer(3));
	m_aui_mgr.AddPane(m_tree_panel, wxAuiPaneInfo().
		Name("m_tree_panel").Caption(UITEXT_TREEVIEW).
		Left().CloseButton(true).BestSize(panel_size).
		FloatingSize(wxSize(400, 600)).Layer(3));
	m_aui_mgr.AddPane(m_movie_view, wxAuiPaneInfo().
		Name("m_movie_view").Caption(UITEXT_MAKEMOVIE).
		Left().CloseButton(true).BestSize(panel_size).
		FloatingSize(wxSize(400, 600)).Layer(3));
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
		//vrv->m_glview->SetSyncR(true);
		//vrv->m_glview->SetSyncG(true);
		//vrv->m_glview->SetSyncB(true);
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
	if (mainmem_buf_size > flvr::TextureRenderer::get_mainmem_buf_size())
		flvr::TextureRenderer::set_mainmem_buf_size(mainmem_buf_size);
}

VRenderFrame::~VRenderFrame()
{
	//release?
	flvr::TextureRenderer::vol_kernel_factory_.clear();
	flvr::TextureRenderer::framebuffer_manager_.clear();
	flvr::TextureRenderer::vertex_array_manager_.clear();
	flvr::TextureRenderer::vol_shader_factory_.clear();
	flvr::TextureRenderer::seg_shader_factory_.clear();
	flvr::TextureRenderer::cal_shader_factory_.clear();
	flvr::TextureRenderer::img_shader_factory_.clear();
	flvr::TextRenderer::text_texture_manager_.clear();
	//for (int i=0; i<GetViewNum(); i++)
	//{
	//	RenderCanvas* view = GetView(i);
	//	if (view) view->ClearAll();
	//}
	m_aui_mgr.UnInit();
	flvr::KernelProgram::release();
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
		vrv = new VRenderView(this, sharedContext);
	}
	else
	{
		vrv = new VRenderView(this);
	}

	if (!vrv)
		return "NO_NAME";
	//RenderCanvas* view = vrv->m_glview;
	//if (!view)
	//	return "NO_NAME";

	m_vrv_list.push_back(vrv);
	if (m_movie_view)
		m_movie_view->AddView(vrv->GetName());
	//if (m_setting_dlg->GetTestMode(3))
	//{
	//	view->m_test_wiref = true;
	//	view->m_draw_bounds = true;
	//	view->m_draw_grid = true;
	//}
	//view->SetPeelingLayers(m_setting_dlg->GetPeelingLyers());
	//view->SetBlendSlices(m_setting_dlg->GetMicroBlend());
	//view->SetAdaptive(m_setting_dlg->GetMouseInt());
	//view->SetGradBg(m_setting_dlg->GetGradBg());
	//view->SetPointVolumeMode(m_setting_dlg->GetPointVolumeMode());
	//view->SetRulerUseTransf(m_setting_dlg->GetRulerUseTransf());
	//view->SetRulerTimeDep(m_setting_dlg->GetRulerTimeDep());
	//vrv->SetPinThreshold(m_setting_dlg->GetPinThreshold());

	//reset gl
	for (int i = 0; i < GetViewNum(); ++i)
	{
		if (GetView(i))
			GetView(i)->ResetGl();
	}

	//m_aui_mgr.Update();
	OrganizeVRenderViews(1);

	//set view default settings
	if (m_adjust_view)
	{
		/*Color gamma, brightness, hdr;
		bool sync_r, sync_g, sync_b;
		m_adjust_view->GetDefaults(gamma, brightness, hdr, sync_r, sync_g, sync_b);
		vrv->m_glview->SetGamma(gamma);
		vrv->m_glview->SetBrightness(brightness);
		vrv->m_glview->SetHdr(hdr);*/
		//view->SetSyncR(true);
		//view->SetSyncG(true);
		//view->SetSyncB(true);
	}

	//add volumes
	//if (GetViewNum() > 0)
	//{
	//	RenderCanvas* view0 = GetView(0);
	//	if (view0)
	//	{
	//		for (int i = 0; i < view0->GetDispVolumeNum(); ++i)
	//		{
	//			fluo::VolumeData* vd = view0->GetDispVolumeData(i);
	//			if (vd)
	//			{
	//				fluo::VolumeData* vd_add = m_data_mgr.DuplicateVolumeData(vd);

	//				if (vd_add)
	//				{
	//					int chan_num = view->GetAny();
	//					fluo::Color color(1.0, 1.0, 1.0);
	//					if (chan_num == 0)
	//						color = fluo::Color(1.0, 0.0, 0.0);
	//					else if (chan_num == 1)
	//						color = fluo::Color(0.0, 1.0, 0.0);
	//					else if (chan_num == 2)
	//						color = fluo::Color(0.0, 0.0, 1.0);

	//					if (chan_num >= 0 && chan_num < 3)
	//						vd_add->setValue(gstColor, color);

	//					view->AddVolumeData(vd_add);
	//				}
	//			}
	//		}
	//	}
	//	//update
	//	view->InitView(INIT_BOUNDS | INIT_CENTER | INIT_TRANSL | INIT_ROTATE);
	//	vrv->UpdateView();
	//}

	UpdateTree();

	return vrv->GetName();
}

//views
int VRenderFrame::GetViewNum()
{
	return m_vrv_list.size();
}

RenderCanvas* VRenderFrame::GetView(int index)
{
	if (index >= 0 && index < (int)m_vrv_list.size())
	{
		VRenderView* v = m_vrv_list[index];
		if (v)
			return v->GetCanvas();
	}
	return 0;
}

RenderCanvas* VRenderFrame::GetView(const wxString& name)
{
	for (size_t i=0; i < m_vrv_list.size(); ++i)
	{
		VRenderView* v = m_vrv_list[i];
		if (v && v->GetName() == name)
			return v->GetCanvas();
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

void VRenderFrame::OnTxt2Change(wxCommandEvent &event)
{
	wxTextCtrl* txt2 = (wxTextCtrl*)event.GetEventObject();
	if (txt2)
	{
		wxString str = txt2->GetValue();
		long lval;
		if (str.ToLong(&lval))
			m_ser_num = lval;
	}
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
		wxCommandEventHandler(VRenderFrame::OnCh11Check), NULL, panel);
	ch11->SetValue(m_sliceSequence);

	//slice sequence check box
	wxCheckBox* ch12 = new wxCheckBox(panel, ID_READ_CHANN,
		"Read file# as channels");
	ch12->Connect(ch12->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(VRenderFrame::OnCh12Check), NULL, panel);
	ch12->SetValue(m_channSequence);

	//digit order
	st1 = new wxStaticText(panel, 0,
		"Digit order:");
	wxComboBox* combo = new wxComboBox(panel, ID_DIGI_ORDER,
		"Order", wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY);
	combo->Connect(combo->GetId(), wxEVT_COMMAND_COMBOBOX_SELECTED,
		wxCommandEventHandler(VRenderFrame::OnCmbChange), NULL, panel);
	std::vector<std::string> combo_list;
	combo_list.push_back("Channel first");
	combo_list.push_back("Z section first");
	for (size_t i = 0; i < combo_list.size(); ++i)
		combo->Append(combo_list[i]);
	combo->SetSelection(m_digitOrder);

	//series number
	st2 = new wxStaticText(panel, 0,
		"Serial:");
	wxTextCtrl* txt2 = new wxTextCtrl(panel, ID_SER_NUM,
		"", wxDefaultPosition, wxDefaultSize);
	txt2->Connect(txt2->GetId(), wxEVT_COMMAND_TEXT_UPDATED,
		wxCommandEventHandler(VRenderFrame::OnTxt2Change), NULL, panel);

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
		"", wxDefaultPosition, wxDefaultSize);
	txt1->SetValue(m_time_id);
	txt1->Connect(txt1->GetId(), wxEVT_COMMAND_TEXT_UPDATED,
		wxCommandEventHandler(VRenderFrame::OnTxt1Change), NULL, panel);
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

wxWindow* VRenderFrame::CreateExtraControlVolumeForImport(wxWindow* parent)
{
	wxPanel* panel = new wxPanel(parent);
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
	group1->Add(10, 10);
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
		"All Supported|*.tif;*.tiff;*.lif;*.lof;*.nd2;*.oib;*.oif;*.xml;*.lsm;*.czi;*.nrrd;*.vvd|"\
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
		"Janelia Brick files (*.vvd)|*.vvd",
#else
		"All Supported|*.tif;*.tiff;*.lif;*.lof;*.oib;*.oif;*.xml;*.lsm;*.czi;*.nrrd;*.vvd|"\
		"Tiff Files (*.tif, *.tiff)|*.tif;*.tiff|"\
		"Leica Image File Format (*.lif)|*.lif|"\
		"Leica Microsystems Object File Format (*.lof)|*.lof|"\
		"Olympus Image Binary Files (*.oib)|*.oib|"\
		"Olympus Original Imaging Format (*.oif)|*.oif|"\
		"Bruker/Prairie View XML (*.xml)|*.xml|"\
		"Zeiss Laser Scanning Microscope (*.lsm)|*.lsm|"\
		"Zeiss ZISRAW File Format (*.czi)|*.czi|"\
		"Utah Nrrd files (*.nrrd)|*.nrrd|"\
		"Janelia Brick files (*.vvd)|*.vvd",
#endif
		wxFD_OPEN|wxFD_MULTIPLE);
	fopendlg->SetExtraControlCreator(CreateExtraControlVolume);

	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		//RenderCanvas* view = GetView(0);

		wxArrayString paths;
		fopendlg->GetPaths(paths);
		LoadVolumes(paths, false, 0);

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
		//RenderCanvas* view = GetView(0);

		wxArrayString paths;
		fopendlg->GetPaths(paths);
		LoadVolumes(paths, true, 0);

		if (m_setting_dlg)
		{
			m_setting_dlg->SetRealtimeCompress(m_compression);
			m_setting_dlg->SetSkipBricks(m_skip_brick);
			m_setting_dlg->UpdateUI();
		}
	}

	delete fopendlg;
}

void VRenderFrame::LoadVolumes(wxArrayString files, bool withImageJ, fluo::Renderview* view)
{
	int j;

	fluo::VolumeData* vd_sel = 0;
	fluo::VolumeGroup* group_sel = 0;
	fluo::Renderview* v = 0;

	if (view)
		v = view;
	else
		v = glbin_root->getCurrentRenderview();

	wxProgressDialog *prg_diag = 0;
	if (v)
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
		m_data_mgr.SetSerNum(m_ser_num);
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
			else if (suffix == ".lif")
				ch_num = m_data_mgr.LoadVolumeData(filename, LOAD_TYPE_LIF, false);
			else if (suffix == ".lof")
				ch_num = m_data_mgr.LoadVolumeData(filename, LOAD_TYPE_LOF, false);

			if (ch_num > 1)
			{
				fluo::VolumeGroup* group = v->addVolumeGroup();
				if (group)
				{
					for (int i=ch_num; i>0; i--)
					{
						fluo::VolumeData* vd = m_data_mgr.GetVolumeData(ch_num-i);
						if (vd)
						{
							v->addVolumeData(vd, group);
							wxString vol_name = vd->getName();
							if (vol_name.Find("_1ch")!=-1 &&
								(i==1 || i==2))
								vd->setValue(gstDisplay, false);
							if (vol_name.Find("_2ch")!=-1 && i==1)
								vd->setValue(gstDisplay, false);

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
						group->setValue(gstDisplay, false);
				}
			}
			else if (ch_num == 1)
			{
				fluo::VolumeData* vd = m_data_mgr.GetVolumeData(0);
				if (vd)
				{
					int chan_num = v->GetVolListSize();
					fluo::Color color(1.0, 1.0, 1.0);
					if (chan_num == 0)
						color = fluo::Color(1.0, 0.0, 0.0);
					else if (chan_num == 1)
						color = fluo::Color(0.0, 1.0, 0.0);
					else if (chan_num == 2)
						color = fluo::Color(0.0, 0.0, 1.0);

					bool bval;
					if (chan_num >=0 && chan_num <3)
						vd->setValue(gstColor, color);
					else
						vd->flipValue(gstRandomizeColor, bval);

					v->addVolumeData(vd, 0);
					vd_sel = vd;

					if (vd->GetReader() && vd->GetReader()->GetTimeNum()>1)
					{
						v->setValue(gstCurrentFrame,
							long(vd->GetReader()->GetCurTime()));
						enable_4d = true;
					}
				}
			}
			else { //TODO: other cases?

			}
		}

		UpdateList();
		if (vd_sel)
			UpdateTree(vd_sel->getName());
		else
			UpdateTree();
		//v->RefreshGL(39);

		v->InitView(fluo::Renderview::INIT_BOUNDS|
			fluo::Renderview::INIT_CENTER);
		//v->m_vrv->UpdateScaleFactor(false);

		if (enable_4d)
		{
			m_movie_view->SetTimeSeq(true);
			m_movie_view->SetRotate(false);
			long lval;
			v->getValue(gstCurrentFrame, lval);
			m_movie_view->SetCurrentTime(lval);
		}

		delete prg_diag;
	}

	//v->RefreshGL(39);//added by Takashi
}

void VRenderFrame::StartupLoad(wxArrayString files, bool run_mov, bool with_imagej)
{
	//if (m_vrv_list[0])
	//	m_vrv_list[0]->m_glview->Init();

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
			suffix == ".lof")
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

void VRenderFrame::LoadMeshes(wxArrayString files, RenderCanvas* view)
{
	//if (!view)
	//	view = GetView(0);

	//fluo::MeshData* md_sel = 0;

	//wxProgressDialog *prg_diag = new wxProgressDialog(
	//	"FluoRender: Loading mesh data...",
	//	"Reading and processing selected mesh data. Please wait.",
	//	100, 0, wxPD_SMOOTH|wxPD_ELAPSED_TIME|wxPD_AUTO_HIDE);

	//fluo::MeshGroup* group = 0;
	//if (files.Count() > 1)
	//	group = view->AddOrGetMGroup();

	//for (int i=0; i<(int)files.Count(); i++)
	//{
	//	prg_diag->Update(90*(i+1)/(int)files.Count());

	//	wxString filename = files[i];
	//	m_data_mgr.LoadMeshData(filename);

	//	fluo::MeshData* md = m_data_mgr.GetLastMeshData();
	//	if (view && md)
	//	{
	//		if (group)
	//		{
	//			group->insertChild(group->getNumChildren()-1, md);
	//			view->SetMeshPopDirty();
	//		}
	//		else
	//			view->AddMeshData(md);

	//		if (i==int(files.Count()-1))
	//			md_sel = md;
	//	}
	//}

	//UpdateList();
	//if (md_sel)
	//	UpdateTree(md_sel->getName());
	//else
	//	UpdateTree();

	//if (view)
	//	view->InitView(INIT_BOUNDS|INIT_CENTER);

	//delete prg_diag;
}

void VRenderFrame::OnOpenMesh(wxCommandEvent& WXUNUSED(event))
{
	wxFileDialog *fopendlg = new wxFileDialog(
		this, "Choose the mesh data file",
		"", "", "*.obj", wxFD_OPEN|wxFD_MULTIPLE);

	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		RenderCanvas* view = GetView(0);
		wxArrayString files;
		fopendlg->GetPaths(files);

		LoadMeshes(files, view);
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
	//int i, j, k;
	//if (!m_tree_panel || !m_tree_panel->GetTreeCtrl())
	//	return;

	//DataTreeCtrl* treectrl = m_tree_panel->GetTreeCtrl();
	//wxTreeItemId root = treectrl->GetRootItem();
	//wxTreeItemIdValue ck_view;
	//int counter = 0;
	//for (i=0; i<GetViewNum(); i++)
	//{
	//	RenderCanvas *view = GetView(i);
	//	wxTreeItemId vrv_item;
	//	if (i==0)
	//		vrv_item = treectrl->GetFirstChild(root, ck_view);
	//	else
	//		vrv_item = treectrl->GetNextChild(root, ck_view);

	//	if (!vrv_item.IsOk())
	//		continue;

	//	m_tree_panel->SetViewItemImage(vrv_item, view->GetDraw());

	//	wxTreeItemIdValue ck_layer;
	//	for (j=0; j< view->GetLayerNum(); j++)
	//	{
	//		fluo::Object* layer = view->GetLayer(j);
	//		wxTreeItemId layer_item;
	//		if (j==0)
	//			layer_item = treectrl->GetFirstChild(vrv_item, ck_layer);
	//		else
	//			layer_item = treectrl->GetNextChild(vrv_item, ck_layer);

	//		if (!layer_item.IsOk())
	//			continue;

	//		if (fluo::VolumeData* vd = dynamic_cast<fluo::VolumeData*>(layer))
	//		{
	//			counter++;
	//			bool disp;
	//			vd->getValue(gstDisplay, disp);
	//			m_tree_panel->SetVolItemImage(layer_item, disp?2*counter+1:2*counter);
	//		}
	//		else if (fluo::MeshData* md = dynamic_cast<fluo::MeshData*>(layer))
	//		{
	//			counter++;
	//			bool disp;
	//			md->getValue(gstDisplay, disp);
	//			m_tree_panel->SetMeshItemImage(layer_item, disp?2*counter+1:2*counter);
	//		}
	//		else if (fluo::Annotations* ann = dynamic_cast<fluo::Annotations*>(layer))
	//		{
	//			counter++;
	//			bool disp;
	//			ann->getValue(gstDisplay, disp);
	//			m_tree_panel->SetAnnotationItemImage(layer_item, disp?2*counter+1:2*counter);
	//		}
	//		else if (fluo::VolumeGroup* group = dynamic_cast<fluo::VolumeGroup*>(layer))
	//		{
	//			bool disp;
	//			group->getValue(gstDisplay, disp);
	//			m_tree_panel->SetGroupItemImage(layer_item, int(disp));
	//			wxTreeItemIdValue ck_volume;
	//			for (k=0; k<group->getNumChildren(); k++)
	//			{
	//				fluo::VolumeData* vd = group->getChild(k)->asVolumeData();
	//				if (!vd)
	//					continue;
	//				wxTreeItemId volume_item;
	//				if (k==0)
	//					volume_item = treectrl->GetFirstChild(layer_item, ck_volume);
	//				else
	//					volume_item = treectrl->GetNextChild(layer_item, ck_volume);
	//				if (!volume_item.IsOk())
	//					continue;
	//				counter++;
	//				vd->getValue(gstDisplay, disp);
	//				m_tree_panel->SetVolItemImage(volume_item, disp?2*counter+1:2*counter);
	//			}
	//		}
	//		else if (fluo::MeshGroup* group = dynamic_cast<fluo::MeshGroup*>(layer))
	//		{
	//			bool disp;
	//			group->getValue(gstDisplay, disp);
	//			m_tree_panel->SetMGroupItemImage(layer_item, int(disp));
	//			wxTreeItemIdValue ck_mesh;
	//			for (k=0; k<group->getNumChildren(); k++)
	//			{
	//				fluo::MeshData* md = group->getChild(k)->asMeshData();
	//				if (!md)
	//					continue;
	//				wxTreeItemId mesh_item;
	//				if (k==0)
	//					mesh_item = treectrl->GetFirstChild(layer_item, ck_mesh);
	//				else
	//					mesh_item = treectrl->GetNextChild(layer_item, ck_mesh);
	//				if (!mesh_item.IsOk())
	//					continue;
	//				counter++;
	//				bool disp;
	//				md->getValue(gstDisplay, disp);
	//				m_tree_panel->SetMeshItemImage(mesh_item, disp?2*counter+1:2*counter);
	//			}
	//		}
	//	}
	//}
	//m_tree_panel->Refresh(false);
}

void VRenderFrame::UpdateTreeColors()
{
	//int i, j, k;
	//int counter = 0;
	//for (i=0 ; i<GetViewNum() ; i++)
	//{
	//	RenderCanvas *view = GetView(i);

	//	for (j=0; j< view->GetLayerNum(); j++)
	//	{
	//		fluo::Object* layer = view->GetLayer(j);
	//		if (fluo::Node* vd = dynamic_cast<fluo::Node*>(layer))
	//		{
	//			fluo::Color c;
	//			vd->getValue(gstColor, c);
	//			wxColor wxc(
	//				(unsigned char)(c.r()*255),
	//				(unsigned char)(c.g()*255),
	//				(unsigned char)(c.b()*255));
	//			m_tree_panel->ChangeIconColor(counter+1, wxc);
	//			counter++;
	//		}
	//		else if (fluo::Group* group = dynamic_cast<fluo::Group*>(layer))
	//		{
	//			for (k=0; k<group->getNumChildren(); k++)
	//			{
	//				fluo::VolumeData* vd = group->getChild(k)->asVolumeData();
	//				if (!vd)
	//					break;
	//				fluo::Color c;
	//				vd->getValue(gstColor, c);
	//				wxColor wxc(
	//					(unsigned char)(c.r()*255),
	//					(unsigned char)(c.g()*255),
	//					(unsigned char)(c.b()*255));
	//				m_tree_panel->ChangeIconColor(counter+1, wxc);
	//				counter++;
	//			}
	//		}
	//	}
	//}
	//m_tree_panel->Refresh(false);
}

void VRenderFrame::UpdateTree(wxString name)
{
	//if (!m_tree_panel)
	//	return;

	//m_tree_panel->DeleteAll();
	//m_tree_panel->ClearIcons();

	//wxString root_str = "Active Datasets";
	//wxTreeItemId root_item = m_tree_panel->AddRootItem(root_str);
	//if (name == root_str)
	//	m_tree_panel->SelectItem(root_item);
	////append non-color icons for views
	//m_tree_panel->AppendIcon();
	//m_tree_panel->Expand(root_item);
	//m_tree_panel->ChangeIconColor(0, wxColor(255, 255, 255));

	//wxTreeItemId sel_item;

	//for (int i = 0; i < GetViewNum(); i++)
	//{
	//	RenderCanvas* view = GetView(i);
	//	if (!view)
	//		continue;
	//	int j, k;

	//	wxString view_name = view->m_vrv->GetName();
	//	view->OrganizeLayers();
	//	wxTreeItemId vrv_item = m_tree_panel->AddViewItem(view_name);
	//	m_tree_panel->SetViewItemImage(vrv_item, view->GetDraw());
	//	if (name == view_name)
	//		m_tree_panel->SelectItem(vrv_item);

	//	for (j=0; j< view->GetLayerNum(); j++)
	//	{
	//		fluo::Object* layer = view->GetLayer(j);
	//		if (fluo::VolumeData* vd = dynamic_cast<fluo::VolumeData*>(layer))
	//		{
	//			//append icon for volume
	//			m_tree_panel->AppendIcon();
	//			fluo::Color c;
	//			vd->getValue(gstColor, c);
	//			wxColor wxc(
	//				(unsigned char)(c.r()*255),
	//				(unsigned char)(c.g()*255),
	//				(unsigned char)(c.b()*255));
	//			int ii = m_tree_panel->GetIconNum()-1;
	//			m_tree_panel->ChangeIconColor(ii, wxc);
	//			wxTreeItemId item = m_tree_panel->AddVolItem(vrv_item, vd->getName());
	//			bool disp;
	//			vd->getValue(gstDisplay, disp);
	//			m_tree_panel->SetVolItemImage(item, disp?2*ii+1:2*ii);
	//			if (name == vd->getName())
	//			{
	//				sel_item = item;
	//				view->SetVolumeA(vd);
	//				GetBrushToolDlg()->GetSettings(view);
	//				GetMeasureDlg()->GetSettings(view);
	//				GetTraceDlg()->GetSettings(view);
	//				GetOclDlg()->GetSettings(view);
	//				GetComponentDlg()->SetView(view);
	//				GetColocalizationDlg()->SetView(view);
	//			}
	//		}
	//		else if (fluo::MeshData* md = dynamic_cast<fluo::MeshData*>(layer))
	//		{
	//			//append icon for mesh
	//			m_tree_panel->AppendIcon();
	//			fluo::Color color;
	//			md->getValue(gstColor, color);
	//			wxColor wxc(
	//				(unsigned char)(color.r()*255),
	//				(unsigned char)(color.g()*255),
	//				(unsigned char)(color.b()*255));
	//			int ii = m_tree_panel->GetIconNum()-1;
	//			m_tree_panel->ChangeIconColor(ii, wxc);
	//			wxTreeItemId item = m_tree_panel->AddMeshItem(vrv_item, md->getName());
	//			bool disp;
	//			md->getValue(gstDisplay, disp);
	//			m_tree_panel->SetMeshItemImage(item, disp ?2*ii+1:2*ii);
	//			if (name == md->getName())
	//				sel_item = item;
	//		}
	//		else if (fluo::Annotations* ann = dynamic_cast<fluo::Annotations*>(layer))
	//		{
	//			//append icon for annotations
	//			m_tree_panel->AppendIcon();
	//			wxColor wxc(255, 255, 255);
	//			int ii = m_tree_panel->GetIconNum()-1;
	//			m_tree_panel->ChangeIconColor(ii, wxc);
	//			wxTreeItemId item = m_tree_panel->AddAnnotationItem(vrv_item, ann->getName());
	//			bool disp;
	//			ann->getValue(gstDisplay, disp);
	//			m_tree_panel->SetAnnotationItemImage(item, disp?2*ii+1:2*ii);
	//			if (name == ann->getName())
	//				sel_item = item;
	//		}
	//		else if (fluo::VolumeGroup* group = dynamic_cast<fluo::VolumeGroup*>(layer))
	//		{
	//			//append group item to tree
	//			wxTreeItemId group_item = m_tree_panel->AddGroupItem(vrv_item, group->getName());
	//			bool disp;
	//			group->getValue(gstDisplay, disp);
	//			m_tree_panel->SetGroupItemImage(group_item, int(disp));
	//			//append volume data to group
	//			for (k=0; k<group->getNumChildren(); k++)
	//			{
	//				fluo::VolumeData* vd = group->getChild(k)->asVolumeData();
	//				if (!vd)
	//					continue;
	//				//add icon
	//				m_tree_panel->AppendIcon();
	//				fluo::Color c;
	//				vd->getValue(gstColor, c);
	//				wxColor wxc(
	//					(unsigned char)(c.r()*255),
	//					(unsigned char)(c.g()*255),
	//					(unsigned char)(c.b()*255));
	//				int ii = m_tree_panel->GetIconNum()-1;
	//				m_tree_panel->ChangeIconColor(ii, wxc);
	//				wxTreeItemId item = m_tree_panel->AddVolItem(group_item, vd->getName());
	//				bool disp;
	//				vd->getValue(gstDisplay, disp);
	//				m_tree_panel->SetVolItemImage(item, disp?2*ii+1:2*ii);
	//				if (name == vd->getName())
	//				{
	//					sel_item = item;
	//					view->SetVolumeA(vd);
	//					GetBrushToolDlg()->GetSettings(view);
	//					GetMeasureDlg()->GetSettings(view);
	//					GetTraceDlg()->GetSettings(view);
	//					GetOclDlg()->GetSettings(view);
	//					GetComponentDlg()->SetView(view);
	//					GetColocalizationDlg()->SetView(view);
	//				}
	//			}
	//			if (name == group->getName())
	//				sel_item = group_item;
	//		}
	//		else if (fluo::MeshGroup* group = dynamic_cast<fluo::MeshGroup*>(layer))
	//		{
	//			//append group item to tree
	//			wxTreeItemId group_item = m_tree_panel->AddMGroupItem(vrv_item, group->getName());
	//			bool disp;
	//			group->getValue(gstDisplay, disp);
	//			m_tree_panel->SetMGroupItemImage(group_item, int(disp));
	//			//append mesh data to group
	//			for (k=0; k<group->getNumChildren(); k++)
	//			{
	//				fluo::MeshData* md = group->getChild(k)->asMeshData();
	//				if (!md)
	//					continue;
	//				//add icon
	//				m_tree_panel->AppendIcon();
	//				fluo::Color color;
	//				md->getValue(gstColor, color);
	//				wxColor wxc(
	//					(unsigned char)(color.r()*255),
	//					(unsigned char)(color.g()*255),
	//					(unsigned char)(color.b()*255));
	//				int ii = m_tree_panel->GetIconNum()-1;
	//				m_tree_panel->ChangeIconColor(ii, wxc);
	//				wxTreeItemId item = m_tree_panel->AddMeshItem(group_item, md->getName());
	//				bool disp;
	//				md->getValue(gstDisplay, disp);
	//				m_tree_panel->SetMeshItemImage(item, disp?2*ii+1:2*ii);
	//				if (name == md->getName())
	//					sel_item = item;
	//			}
	//			if (name == group->getName())
	//				sel_item = group_item;
	//		}
	//	}
	//}

	//if (sel_item.IsOk())
	//	m_tree_panel->SelectItem(sel_item);
	//m_tree_panel->ExpandAll();
}

void VRenderFrame::UpdateList()
{
	//m_list_panel->DeleteAllItems();

	//for (int i=0 ; i<m_data_mgr.GetVolumeNum() ; i++)
	//{
	//	fluo::VolumeData* vd = m_data_mgr.GetVolumeData(i);
	//	bool dup;
	//	vd->getValue(gstDuplicate, dup);
	//	if (vd && !dup)
	//	{
	//		wxString name = vd->getName();
	//		std::wstring str;
	//		vd->getValue(gstDataPath, str);
	//		wxString path = str;
	//		m_list_panel->Append(1, name, path);
	//	}
	//}

	//for (int i=0 ; i<m_data_mgr.GetMeshNum() ; i++)
	//{
	//	fluo::MeshData* md = m_data_mgr.GetMeshData(i);
	//	if (md)
	//	{
	//		wxString name = md->getName();
	//		std::wstring wstr;
	//		md->getValue(gstDataPath, wstr);
	//		wxString path = wstr;
	//		m_list_panel->Append(2, name, path);
	//	}
	//}

	//for (int i=0; i<m_data_mgr.GetAnnotationNum(); i++)
	//{
	//	fluo::Annotations* ann = m_data_mgr.GetAnnotations(i);
	//	if (ann)
	//	{
	//		wxString name = ann->getName();
	//		std::wstring wstr;
	//		ann->getValue(gstDataPath, wstr);
	//		wxString path = wstr;
	//		m_list_panel->Append(3, name, path);
	//	}
	//}
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
	fluo::Renderview* view,
	fluo::VolumeGroup* group,
	fluo::VolumeData* vd,
	fluo::MeshData* md,
	fluo::Annotations* ann)
{
	if (m_adjust_view)
	{
		m_adjust_view->SetView(view);
		if (!view || vd)
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
				fluo::Referenced* ref;
				ann->getRvalu(gstVolume, &ref);
				fluo::VolumeData* vd_ann = dynamic_cast<fluo::VolumeData*>(ref);
				m_clip_view->SetVolumeData(vd_ann);
			}
			break;
		}
	}

	m_cur_sel_type = type;
	//clear mesh boundbox
	if (m_data_mgr.GetMeshData(m_cur_sel_mesh))
		m_data_mgr.GetMeshData(m_cur_sel_mesh)->setValue(gstDrawBounds, false);

	if (m_brush_tool_dlg)
		m_brush_tool_dlg->GetSettings(view);
	if (m_colocalization_dlg)
		m_colocalization_dlg->SetView(view);
	if (m_component_dlg)
		m_component_dlg->SetView(view);
	if (m_counting_dlg)
		m_counting_dlg->GetSettings(view);
	if (m_measure_dlg)
		m_measure_dlg->GetSettings(view);
	if (m_noise_cancelling_dlg)
		m_noise_cancelling_dlg->GetSettings(view);
	if (m_ocl_dlg)
		m_ocl_dlg->GetSettings(view);
	if (m_recorder_dlg)
		m_recorder_dlg->GetSettings(view);
	if (m_trace_dlg)
		m_trace_dlg->GetSettings(view);

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
		if (vd)
		{
			bool disp;
			vd->getValue(gstDisplay, disp);
			if (!disp)
				break;
			m_volume_prop->SetVolumeData(vd);
			m_volume_prop->SetGroup(group);
			m_volume_prop->SetView(view);
			if (!m_volume_prop->IsShown())
			{
				m_volume_prop->Show(true);
				m_prop_sizer->Clear();
				m_prop_sizer->Add(m_volume_prop, 1, wxEXPAND, 0);
				m_prop_panel->SetSizer(m_prop_sizer);
				m_prop_panel->Layout();
			}
			m_aui_mgr.GetPane(m_prop_panel).Caption(
				wxString(UITEXT_PROPERTIES)+wxString(" - ")+vd->getName());
			m_aui_mgr.Update();
			std::string str = vd->getName();
			m_cur_sel_vol = m_data_mgr.GetVolumeIndex(str);

			for (size_t i=0; i< GetViewNum(); ++i)
			{
				RenderCanvas* v = GetView(i);
				if (!v)
					continue;
				//v->m_cur_vol = vd;
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
			m_mesh_prop->SetView(view);
			m_mesh_prop->SetMeshData(md);
			if (!m_mesh_prop->IsShown())
			{
				m_mesh_prop->Show(true);
				m_prop_sizer->Clear();
				m_prop_sizer->Add(m_mesh_prop, 1, wxEXPAND, 0);
				m_prop_panel->SetSizer(m_prop_sizer);
				m_prop_panel->Layout();
			}
			m_aui_mgr.GetPane(m_prop_panel).Caption(
				wxString(UITEXT_PROPERTIES)+wxString(" - ")+md->getName());
			m_aui_mgr.Update();
			wxString str = md->getName();
			m_cur_sel_mesh = m_data_mgr.GetMeshIndex(str.ToStdString());
			md->setValue(gstDrawBounds, true);
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
			m_annotation_prop->SetAnnotations(ann);
			if (!m_annotation_prop->IsShown())
			{
				m_annotation_prop->Show(true);
				m_prop_sizer->Clear();
				m_prop_sizer->Add(m_annotation_prop, 1, wxEXPAND, 0);
				m_prop_panel->SetSizer(m_prop_sizer);
				m_prop_panel->Layout();
			}
			m_aui_mgr.GetPane(m_prop_panel).Caption(
				wxString(UITEXT_PROPERTIES)+wxString(" - ")+ann->getName());
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
				wxString("Manipulations - ")+md->getName());
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
	//for (int i=0 ; i<(int)m_vrv_list.size() ; i++)
	//{
	//	if (m_vrv_list[i])
	//		m_vrv_list[i]->RefreshGL(interactive);
	//}

	//incase volume color changes
	//change icon color of the tree panel
	if (tree)
		UpdateTreeColors();
}

void VRenderFrame::DeleteVRenderView(int i)
{
	//if (m_vrv_list[i])
	//{
	//	int j;
	//	wxString str = m_vrv_list[i]->GetName();

	//	for (j=0 ; j<GetView(i)->GetAllVolumeNum() ; j++)
	//		GetView(i)->GetAllVolumeData(j)->setValue(gstDisplay, true);
	//	for (j=0 ; j< GetView(i)->GetMeshNum() ; j++)
	//		GetView(i)->GetMeshData(j)->setValue(gstDisplay, true);
	//	VRenderView* vrv = m_vrv_list[i];
	//	m_vrv_list.erase(m_vrv_list.begin()+i);
	//	m_aui_mgr.DetachPane(vrv);
	//	vrv->Close();
	//	delete vrv;
	//	m_aui_mgr.Update();
	//	UpdateTree();

	//	if (m_movie_view)
	//		m_movie_view->DeleteView(str);
	//}
}

void VRenderFrame::DeleteVRenderView(const wxString &name)
{
	//for (int i=0; i<GetViewNum(); i++)
	//{
	//	RenderCanvas* view = GetView(i);
	//	if (view && name == view->m_vrv->GetName() && view->m_vrv->m_id > 1)
	//	{
	//		DeleteVRenderView(i);
	//		return;
	//	}
	//}
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

	panel->SetSizerAndFit(group1);
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

	bool bval;
	long lval;
	double dval;
	std::wstring wsval;
	double dx, dy, dz;
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

		fluo::VolumeData* vd = m_data_mgr.GetVolumeData(i);
		if (vd)
		{
			str = wxString::Format("/data/volume/%d", i);
			//name
			fconfig.SetPath(str);
			str = vd->getName();
			fconfig.Write("name", str);
			//compression
			fconfig.Write("compression", m_compression);
			//skip brick
			vd->getValue(gstSkipBrick, bval);
			fconfig.Write("skip_brick", bval);
			//path
			vd->getValue(gstDataPath, wsval);
			str = wsval;
			bool new_chan = false;
			if (str == "" || m_vrp_embed)
			{
				wxString new_folder;
				new_folder = filename + "_files";
				MkDirW(new_folder.ToStdWstring());
				str = new_folder + GETSLASH() + vd->getName() + ".tif";
				fluo::Quaternion qtemp;
				vd->SaveData(str.ToStdWstring(), 0, false, 0, false, VRenderFrame::GetCompression(), qtemp);
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
			vd->getValue(gstTime, lval);
			fconfig.Write("cur_time", lval);
			vd->getValue(gstChannel, lval);
			fconfig.Write("cur_chan", new_chan?0:lval);

			//volume properties
			fconfig.SetPath("properties");
			vd->getValue(gstDisplay, bval);
			fconfig.Write("display", bval);

			//properties
			vd->getValue(gstGamma3d, dval);
			fconfig.Write("3dgamma", dval);
			vd->getValue(gstExtractBoundary, dval);
			fconfig.Write("boundary", dval);
			vd->getValue(gstSaturation, dval);
			fconfig.Write("contrast", dval);
			vd->getValue(gstLowThreshold, dval);
			fconfig.Write("left_thresh", dval);
			vd->getValue(gstHighThreshold, dval);
			fconfig.Write("right_thresh", dval);
			fluo::Color color;
			vd->getValue(gstColor, color);
			str = wxString::Format("%f %f %f", color.r(), color.g(), color.b());
			fconfig.Write("color", str);
			fluo::HSVColor hsv;
			vd->getValue(gstHsv, hsv);
			str = wxString::Format("%f %f %f", hsv.hue(), hsv.sat(), hsv.val());
			fconfig.Write("hsv", str);
			vd->getValue(gstSecColor, color);
			str = wxString::Format("%f %f %f", color.r(), color.g(), color.b());
			fconfig.Write("mask_color", str);
			vd->getValue(gstSecColorSet, bval);
			fconfig.Write("mask_color_set", bval);
			vd->getValue(gstAlphaEnable, bval);
			fconfig.Write("enable_alpha", bval);
			vd->getValue(gstAlpha, dval);
			fconfig.Write("alpha", dval);
			vd->getValue(gstMatAmb, dval);
			fconfig.Write("ambient", dval);
			vd->getValue(gstMatDiff, dval);
			fconfig.Write("diffuse", dval);
			vd->getValue(gstMatSpec, dval);
			fconfig.Write("specular", dval);
			vd->getValue(gstMatShine, dval);
			fconfig.Write("shininess", dval);
			vd->getValue(gstShadingEnable, bval);
			fconfig.Write("shading", bval);
			vd->getValue(gstSampleRate, dval);
			fconfig.Write("samplerate", dval);

			//resolution scale
			vd->getValue(gstSpcX, dx);
			vd->getValue(gstSpcY, dy);
			vd->getValue(gstSpcZ, dz);
			str = wxString::Format("%lf %lf %lf", dx, dy, dz);
			fconfig.Write("res", str);
			vd->getValue(gstBaseSpcX, dx);
			vd->getValue(gstBaseSpcY, dy);
			vd->getValue(gstBaseSpcZ, dz);
			str = wxString::Format("%lf %lf %lf", dx, dy, dz);
			fconfig.Write("b_res", str);
			vd->getValue(gstSpcSclX, dx);
			vd->getValue(gstSpcSclY, dy);
			vd->getValue(gstSpcSclZ, dz);
			str = wxString::Format("%lf %lf %lf", dx, dy, dz);
			fconfig.Write("s_res", str);
			vd->getValue(gstScaleX, dx);
			vd->getValue(gstScaleY, dy);
			vd->getValue(gstScaleZ, dz);
			str = wxString::Format("%lf %lf %lf", dx, dy, dz);
			fconfig.Write("scl", str);

			//planes
			vector<fluo::Plane*> *planes = 0;
			if (vd->GetRenderer())
				planes = vd->GetRenderer()->get_planes();
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
			vd->getValue(gstGammaR, dx);
			vd->getValue(gstGammaG, dy);
			vd->getValue(gstGammaB, dz);
			str = wxString::Format("%f %f %f", dx, dy, dz);
			fconfig.Write("gamma", str);
			vd->getValue(gstBrightnessR, dx);
			vd->getValue(gstBrightnessG, dy);
			vd->getValue(gstBrightnessB, dz);
			str = wxString::Format("%f %f %f", dx, dy, dz);
			fconfig.Write("brightness", str);
			vd->getValue(gstEqualizeR, dx);
			vd->getValue(gstEqualizeG, dy);
			vd->getValue(gstEqualizeB, dz);
			str = wxString::Format("%f %f %f", dx, dy, dz);
			fconfig.Write("hdr", str);
			vd->getValue(gstSyncR, bval);
			fconfig.Write("sync_r", bval);
			vd->getValue(gstSyncG, bval);
			fconfig.Write("sync_g", bval);
			vd->getValue(gstSyncB, bval);
			fconfig.Write("sync_b", bval);

			//colormap settings
			vd->getValue(gstColormapMode, lval);
			fconfig.Write("colormap_mode", lval);
			vd->getValue(gstColormapInv, dval);
			fconfig.Write("colormap_inv", dval);
			vd->getValue(gstColormapType, lval);
			fconfig.Write("colormap", lval);
			vd->getValue(gstColormapProj, lval);
			fconfig.Write("colormap_proj", lval);
			vd->getValue(gstColormapLow, dval);
			fconfig.Write("colormap_lo_value", dval);
			vd->getValue(gstColormapHigh, dval);
			fconfig.Write("colormap_hi_value", dval);

			//high transp
			vd->getValue(gstAlphaPower, dval);
			fconfig.Write("alpha_power", dval);
			//inversion
			vd->getValue(gstInvert, bval);
			fconfig.Write("inv", bval);
			//mip enable
			vd->getValue(gstMipMode, lval);
			fconfig.Write("mode", lval);
			//noise reduction
			vd->getValue(gstNoiseRedct, bval);
			fconfig.Write("noise_red", bval);
			//depth override
			vd->getValue(gstBlendMode, lval);
			fconfig.Write("depth_ovrd", lval);

			//shadow
			vd->getValue(gstShadowEnable, bval);
			fconfig.Write("shadow", bval);
			//shadow intensity
			vd->getValue(gstShadowInt, dval);
			fconfig.Write("shadow_darkness", dval);

			//legend
			vd->getValue(gstLegend, bval);
			fconfig.Write("legend", bval);

			//mask
			long time, chan;
			vd->getValue(gstTime, time);
			vd->getValue(gstChannel, chan);
			vd->SaveMask(true, time, chan);
			vd->SaveLabel(true, time, chan);
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

		fluo::MeshData* md = m_data_mgr.GetMeshData(i);
		if (md)
		{
			std::wstring wstr;
			md->getValue(gstDataPath, wstr);
			if (wstr == "" || m_vrp_embed)
			{
				wxString new_folder;
				new_folder = filename + "_files";
				MkDirW(new_folder.ToStdWstring());
				str = new_folder + GETSLASH() + md->getName() + ".obj";
				md->SaveData(str.ToStdString());
			}
			str = wxString::Format("/data/mesh/%d", i);
			fconfig.SetPath(str);
			str = md->getName();
			fconfig.Write("name", str);
			str = wstr;
			fconfig.Write("path", str);
			//mesh prperties
			fconfig.SetPath("properties");
			bool bval;
			md->getValue(gstDisplay, bval);
			fconfig.Write("display", bval);
			//lighting
			md->getValue(gstShadingEnable, bval);
			fconfig.Write("lighting", bval);
			//material
			fluo::Color color;
			double amb, diff, spec, shine, alpha;
			md->getValue(gstColor, color);
			md->getValue(gstMatAmb, amb);
			md->getValue(gstMatDiff, diff);
			md->getValue(gstMatSpec, spec);
			md->getValue(gstMatShine, shine);
			md->getValue(gstAlpha, alpha);
			str = wxString::Format("%f %f %f", color.r(), color.g(), color.b());
			fconfig.Write("color", str);
			fconfig.Write("ambient", amb);
			fconfig.Write("diffuse", diff);
			fconfig.Write("specular", spec);
			fconfig.Write("shininess", shine);
			fconfig.Write("alpha", alpha);
			//2d adjustment settings
			double r, g, b;
			md->getValue(gstGammaR, r);
			md->getValue(gstGammaG, g);
			md->getValue(gstGammaB, b);
			str = wxString::Format("%f %f %f", r, g, b);
			fconfig.Write("gamma", str);
			md->getValue(gstBrightnessR, r);
			md->getValue(gstBrightnessG, g);
			md->getValue(gstBrightnessB, b);
			str = wxString::Format("%f %f %f", r, g, b);
			fconfig.Write("brightness", str);
			md->getValue(gstEqualizeR, r);
			md->getValue(gstEqualizeG, g);
			md->getValue(gstEqualizeB, b);
			str = wxString::Format("%f %f %f", r, g, b);
			fconfig.Write("hdr", str);
			md->getValue(gstSyncR, bval);
			fconfig.Write("sync_r", bval);
			md->getValue(gstSyncG, bval);
			fconfig.Write("sync_g", bval);
			md->getValue(gstSyncB, bval);
			fconfig.Write("sync_b", bval);
			//shadow
			md->getValue(gstShadowEnable, bval);
			fconfig.Write("shadow", bval);
			md->getValue(gstShadowInt, dval);
			fconfig.Write("shadow_darkness", dval);

			//mesh transform
			fconfig.SetPath("../transform");
			double x, y, z;
			md->getValue(gstTransX, x);
			md->getValue(gstTransY, y);
			md->getValue(gstTransZ, z);
			str = wxString::Format("%f %f %f", x, y, z);
			fconfig.Write("translation", str);
			md->getValue(gstRotX, x);
			md->getValue(gstRotY, y);
			md->getValue(gstRotZ, z);
			str = wxString::Format("%f %f %f", x, y, z);
			fconfig.Write("rotation", str);
			md->getValue(gstScaleX, x);
			md->getValue(gstScaleY, y);
			md->getValue(gstScaleZ, z);
			str = wxString::Format("%f %f %f", x, y, z);
			fconfig.Write("scaling", str);
		}
	}
	//annotations
	fconfig.SetPath("/data/annotations");
	fconfig.Write("num", m_data_mgr.GetAnnotationNum());
	for (i=0; i<m_data_mgr.GetAnnotationNum(); i++)
	{
		fluo::Annotations* ann = m_data_mgr.GetAnnotations(i);
		if (ann)
		{
			std::wstring wstr;
			ann->getValue(gstDataPath, wstr);
			if (wstr.empty())
			{
				wxString new_folder;
				new_folder = filename + "_files";
				MkDirW(new_folder.ToStdWstring());
				str = new_folder + GETSLASH() + ann->getName() + ".txt";
				ann->SaveData(str.ToStdWstring());
			}
			str = wxString::Format("/data/annotations/%d", i);
			fconfig.SetPath(str);
			str = ann->getName();
			fconfig.Write("name", str);
			str = wstr;
			fconfig.Write("path", str);
		}
	}
	//views
	fconfig.SetPath("/views");
	fconfig.Write("num", GetViewNum());
	for (i=0; i<GetViewNum(); i++)
	{
		RenderCanvas* view = GetView(i);
		if (view)
		{
			str = wxString::Format("/views/%d", i);
			fconfig.SetPath(str);
			//view layers
			str = wxString::Format("/views/%d/layers", i);
			fconfig.SetPath(str);
			//fconfig.Write("num", view->GetLayerNum());
			//for (j=0; j< view->GetLayerNum(); j++)
			//{
			//	fluo::Object* layer = view->GetLayer(j);
			//	if (!layer)
			//		continue;
			//	str = wxString::Format("/views/%d/layers/%d", i, j);
			//	fconfig.SetPath(str);
			//	if (fluo::VolumeData* vd = dynamic_cast<fluo::VolumeData*>(layer))
			//	{
			//		fconfig.Write("type", 2);
			//		fconfig.Write("name", layer->getName());
			//	}
			//	else if (fluo::MeshData* md = dynamic_cast<fluo::MeshData*>(layer))
			//	{
			//		fconfig.Write("type", 3);
			//		fconfig.Write("name", layer->getName());
			//	}
			//	else if (fluo::Annotations* ann = dynamic_cast<fluo::Annotations*>(layer))
			//	{
			//		fconfig.Write("type", 4);
			//		fconfig.Write("name", layer->getName());
			//	}
			//	else if (fluo::VolumeGroup* group = dynamic_cast<fluo::VolumeGroup*>(layer))
			//	{
			//		fconfig.Write("type", 5);
			//		fconfig.Write("name", group->getName());
			//		fconfig.Write("id", group->getId());
			//		//dispaly
			//		group->getValue(gstDisplay, bval);
			//		fconfig.Write("display", bval);
			//		//2d adjustment
			//		group->getValue(gstGammaR, dx);
			//		group->getValue(gstGammaG, dy);
			//		group->getValue(gstGammaB, dz);
			//		str = wxString::Format("%f %f %f", dx, dy, dz);
			//		fconfig.Write("gamma", str);
			//		group->getValue(gstBrightnessR, dx);
			//		group->getValue(gstBrightnessG, dy);
			//		group->getValue(gstBrightnessB, dz);
			//		str = wxString::Format("%f %f %f", dx, dy, dz);
			//		fconfig.Write("brightness", str);
			//		group->getValue(gstEqualizeR, dx);
			//		group->getValue(gstEqualizeG, dy);
			//		group->getValue(gstEqualizeB, dz);
			//		str = wxString::Format("%f %f %f", dx, dy, dz);
			//		fconfig.Write("hdr", str);
			//		group->getValue(gstSyncR, bval);
			//		fconfig.Write("sync_r", bval);
			//		group->getValue(gstSyncG, bval);
			//		fconfig.Write("sync_g", bval);
			//		group->getValue(gstSyncB, bval);
			//		fconfig.Write("sync_b", bval);
			//		//sync volume properties
			//		group->getValue(gstSyncGroup, bval);
			//		fconfig.Write("sync_vp", bval);
			//		//volumes
			//		str = wxString::Format("/views/%d/layers/%d/volumes", i, j);
			//		fconfig.SetPath(str);
			//		fconfig.Write("num", group->getNumChildren());
			//		for (k=0; k<group->getNumChildren(); k++)
			//			fconfig.Write(wxString::Format("vol_%d", k), group->getChild(k)->getName());
			//	}
			//	else if (fluo::MeshGroup* group = dynamic_cast<fluo::MeshGroup*>(layer))
			//	{
			//		fconfig.Write("type", 6);
			//		fconfig.Write("name", layer->getName());
			//		//fconfig.Write("id", MeshGroup::GetID());
			//		//display
			//		group->getValue(gstDisplay, bval);
			//		fconfig.Write("display", bval);
			//		//sync mesh properties
			//		group->getValue(gstSyncGroup, bval);
			//		fconfig.Write("sync_mp", bval);
			//		//meshes
			//		str = wxString::Format("/views/%d/layers/%d/meshes", i, j);
			//		fconfig.SetPath(str);
			//		fconfig.Write("num", group->getNumChildren());
			//		for (k=0; k<group->getNumChildren(); k++)
			//			fconfig.Write(wxString::Format("mesh_%d", k), group->getChild(k)->getName());
			//	}
			//}

			////properties
			//fconfig.SetPath(wxString::Format("/views/%d/properties", i));
			//fconfig.Write("drawall", view->GetDraw());
			//fconfig.Write("persp", view->GetPersp());
			//fconfig.Write("free", view->GetFree());
			//fconfig.Write("aov", view->GetAov());
			//fconfig.Write("nearclip", view->GetNearClip());
			//fconfig.Write("farclip", view->GetFarClip());
			//fluo::Color bkcolor;
			//bkcolor = view->GetBackgroundColor();
			//str = wxString::Format("%f %f %f", bkcolor.r(), bkcolor.g(), bkcolor.b());
			//fconfig.Write("backgroundcolor", str);
			//fconfig.Write("drawtype", view->GetDrawType());
			//fconfig.Write("volmethod", view->GetVolMethod());
			//fconfig.Write("peellayers", view->GetPeelingLayers());
			//fconfig.Write("fog", view->GetFog());
			//fconfig.Write("fogintensity", (double)view->GetFogIntensity());
			//fconfig.Write("draw_camctr", view->m_draw_camctr);
			//fconfig.Write("draw_info", view->m_draw_info);
			//fconfig.Write("draw_legend", view->m_draw_legend);

			//double x, y, z;
			////camera
			//view->GetTranslations(x, y, z);
			//str = wxString::Format("%f %f %f", x, y, z);
			//fconfig.Write("translation", str);
			//view->GetRotations(x, y, z);
			//str = wxString::Format("%f %f %f", x, y, z);
			//fconfig.Write("rotation", str);
			//fluo::Quaternion q = view->GetZeroQuat();
			//str = wxString::Format("%f %f %f %f", q.x, q.y, q.z, q.w);
			//fconfig.Write("zero_quat", str);
			//view->GetCenters(x, y, z);
			//str = wxString::Format("%f %f %f", x, y, z);
			//fconfig.Write("center", str);
			//fconfig.Write("centereyedist", view->GetCenterEyeDist());
			//fconfig.Write("radius", view->GetRadius());
			//fconfig.Write("initdist", view->GetInitDist());
			//fconfig.Write("scale_mode", view->m_scale_mode);
			//fconfig.Write("scale", view->m_scale_factor);
			//fconfig.Write("pin_rot_center", view->m_pin_rot_center);
			////object
			//view->GetObjCenters(x, y, z);
			//str = wxString::Format("%f %f %f", x, y, z);
			//fconfig.Write("obj_center", str);
			//view->GetObjTrans(x, y, z);
			//str = wxString::Format("%f %f %f", x, y, z);
			//fconfig.Write("obj_trans", str);
			//view->GetObjRot(x, y, z);
			//str = wxString::Format("%f %f %f", x, y, z);
			//fconfig.Write("obj_rot", str);
			////scale bar
			//fconfig.Write("disp_scale_bar", view->m_disp_scale_bar);
			//fconfig.Write("disp_scale_bar_text", view->m_disp_scale_bar_text);
			//fconfig.Write("sb_length", view->m_sb_length);
			//str = view->m_sb_text;
			//fconfig.Write("sb_text", str);
			//str = view->m_sb_num;
			//fconfig.Write("sb_num", str);
			//fconfig.Write("sb_unit", view->m_sb_unit);

			////2d adjustment
			//str = wxString::Format("%f %f %f", view->GetGamma().r(),
			//	view->GetGamma().g(), view->GetGamma().b());
			//fconfig.Write("gamma", str);
			//str = wxString::Format("%f %f %f", view->GetBrightness().r(),
			//	view->GetBrightness().g(), view->GetBrightness().b());
			//fconfig.Write("brightness", str);
			//str = wxString::Format("%f %f %f", view->GetHdr().r(),
			//	view->GetHdr().g(), view->GetHdr().b());
			//fconfig.Write("hdr", str);
			//fconfig.Write("sync_r", view->GetSyncR());
			//fconfig.Write("sync_g", view->GetSyncG());
			//fconfig.Write("sync_b", view->GetSyncB());

			////clipping plane rotations
			//fconfig.Write("clip_mode", view->GetClipMode());
			//double rotx_cl, roty_cl, rotz_cl;
			//view->GetClippingPlaneRotations(rotx_cl, roty_cl, rotz_cl);
			//fconfig.Write("rotx_cl", rotx_cl);
			//fconfig.Write("roty_cl", roty_cl);
			//fconfig.Write("rotz_cl", rotz_cl);

			////painting parameters
			//flrd::VolumeSelector* selector = view->GetVolumeSelector();
			//if (selector)
			//{
			//	fconfig.Write("brush_use_pres", selector->GetBrushUsePres());
			//	fconfig.Write("brush_size_1", selector->GetBrushSize1());
			//	fconfig.Write("brush_size_2", selector->GetBrushSize2());
			//	fconfig.Write("brush_spacing", selector->GetBrushSpacing());
			//	fconfig.Write("brush_iteration", selector->GetBrushIteration());
			//	fconfig.Write("brush_translate", selector->GetBrushSclTranslate());
			//	fconfig.Write("w2d", selector->GetW2d());
			//}

			////rulers
			//fconfig.SetPath(wxString::Format("/views/%d/rulers", i));
			//view->GetRulerHandler()->Save(fconfig, i);
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
	fconfig.Write("views_cmb", m_movie_view->GetView());
	fconfig.Write("rot_check", m_movie_view->GetRotate());
	fconfig.Write("seq_check", m_movie_view->GetTimeSeq());
	fconfig.Write("rot_axis", m_movie_view->GetRotAxis());
	fconfig.Write("rot_deg", m_movie_view->GetRotDeg());
	fconfig.Write("movie_len", m_movie_view->GetMovieLen());
	fconfig.Write("fps", m_movie_view->GetFps());
	fconfig.Write("crop", m_movie_view->GetCrop());
	fconfig.Write("crop_x", m_movie_view->GetCropX());
	fconfig.Write("crop_y", m_movie_view->GetCropY());
	fconfig.Write("crop_w", m_movie_view->GetCropW());
	fconfig.Write("crop_h", m_movie_view->GetCropH());
	fconfig.Write("start_frame", m_movie_view->GetStartFrame());
	fconfig.Write("end_frame", m_movie_view->GetEndFrame());
	fconfig.Write("run_script", m_setting_dlg->GetRunScript());
	fconfig.Write("script_file", m_setting_dlg->GetScriptFile());
	//tracking diag
	fconfig.SetPath("/track_diag");
	int ival = m_trace_dlg->GetTrackFileExist(true);
	if (ival == 1)
	{
		std::wstring new_folder;
		new_folder = filename.ToStdWstring() + L"_files";
		MkDirW(new_folder);
		std::wstring wstr = new_folder + GETSLASH() + GET_NAME(filename.ToStdWstring()) + L".track";
		m_trace_dlg->SaveTrackFile(wstr);
	}
	fconfig.Write("track_file", wxString(m_trace_dlg->GetTrackFile()));
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
							dval = ((FlKeyDouble*)key)->GetValue();
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
							bval = ((FlKeyBoolean*)key)->GetValue();
							fconfig.Write("val", bval);
						}
						break;
					case FLKEY_TYPE_INT:
						{
							int ival = ((FlKeyInt*)key)->GetValue();
							fconfig.Write("val", ival);
						}
					}
				}
			}
		}
	}

	SaveConfig(fconfig, filename);
	UpdateList();

	delete prg_diag;
}

void VRenderFrame::OpenProject(wxString& filename)
{
	m_data_mgr.SetProjectPath(filename);

	long iVal;
	int i, j, k;
	//clear
	m_data_mgr.ClearAll();
	//fluo::VolumeGroup::ResetID();
	//MeshGroup::ResetID();
	m_adjust_view->SetVolumeData(0);
	m_adjust_view->SetGroup(0);
	m_adjust_view->SetGroupLink(0);
	glbin_root->getCurrentRenderview()->Clear();
	for (i = m_vrv_list.size() - 1; i > 0; i--)
		DeleteVRenderView(i);
	//VRenderView::ResetID();
	//fluo::VolumeGroup::ResetID();
	//MeshGroup::ResetID();
	double dx, dy, dz;
	long lval;
	double dval;
	bool bval;

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
					else if (suffix == ".lif")
						loaded_num = m_data_mgr.LoadVolumeData(str, LOAD_TYPE_LIF, false, cur_chan, cur_time);
					else if (suffix == ".lof")
						loaded_num = m_data_mgr.LoadVolumeData(str, LOAD_TYPE_LOF, false, cur_chan, cur_time);
				}
				fluo::VolumeData* vd = 0;
				if (loaded_num)
					vd = m_data_mgr.GetLastVolumeData();
				if (vd)
				{
					if (fconfig.Read("name", &str))
						vd->setName(str.ToStdString());//setname
					//volume properties
					if (fconfig.Exists("properties"))
					{
						fconfig.SetPath("properties");
						bool disp;
						if (fconfig.Read("display", &disp))
							vd->setValue(gstDisplay, disp);

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
							vd->setValue(gstGamma3d, double(gamma));
							vd->setValue(gstExtractBoundary, double(left_y));
							vd->setValue(gstSaturation, double(offset1));
							vd->setValue(gstLowThreshold, double(left_x));
							vd->setValue(gstHighThreshold, double(left_x+width));
							if (fconfig.Read("widgetcolor", &str))
							{
								float red, green, blue;
								if (SSCANF(str.c_str(), "%f%f%f", &red, &green, &blue)){
									fluo::Color col(red,green,blue);
									vd->setValue(gstColor, col);
								}
							}
							double alpha;
							if (fconfig.Read("widgetalpha", &alpha))
								vd->setValue(gstAlpha, alpha);
						}

						//transfer function
						if (fconfig.Read("3dgamma", &dval))
							vd->setValue(gstGamma3d, dval);
						if (fconfig.Read("boundary", &dval))
							vd->setValue(gstExtractBoundary, dval);
						if (fconfig.Read("contrast", &dval))
							vd->setValue(gstSaturation, dval);
						if (fconfig.Read("left_thresh", &dval))
							vd->setValue(gstLowThreshold, dval);
						if (fconfig.Read("right_thresh", &dval))
							vd->setValue(gstHighThreshold, dval);
						if (fconfig.Read("color", &str))
						{
							float red, green, blue;
							if (SSCANF(str.c_str(), "%f%f%f", &red, &green, &blue)){
								fluo::Color col(red,green,blue);
								vd->setValue(gstColor, col);
							}
						}
						if (fconfig.Read("hsv", &str))
						{
							float hue, sat, val;
							if (SSCANF(str.c_str(), "%f%f%f", &hue, &sat, &val))
								vd->setValue(gstHsv, fluo::HSVColor(hue, sat, val));
						}
						if (fconfig.Read("mask_color", &str))
						{
							float red, green, blue;
							if (SSCANF(str.c_str(), "%f%f%f", &red, &green, &blue)){
								fluo::Color col(red,green,blue);
								if (fconfig.Read("mask_color_set", &bval))
								{
									vd->setValue(gstSecColorSet, bval);
									vd->setValue(gstSecColor, col);
								}
							}
						}
						if (fconfig.Read("enable_alpha", &bval))
							vd->setValue(gstAlphaEnable, bval);
						if (fconfig.Read("alpha", &dval))
							vd->setValue(gstAlpha, dval);

						//shading
						if (fconfig.Read("ambient", &dval))
							vd->setValue(gstMatAmb, dval);
						if (fconfig.Read("diffuse", &dval))
							vd->setValue(gstMatDiff, dval);
						if (fconfig.Read("specular", &dval))
							vd->setValue(gstMatSpec, dval);
						if (fconfig.Read("shininess", &dval))
							vd->setValue(gstMatShine, dval);
						if (fconfig.Read("shading", &bval))
							vd->setValue(gstShadingEnable, bval);
						if (fconfig.Read("samplerate", &dval))
							vd->setValue(gstSampleRate, dval);

						//spacings and scales
						bool multires = false;
						vd->getValue(gstMultires, multires);
						if (!multires)
						{
							if (fconfig.Read("res", &str))
							{
								if (SSCANF(str.c_str(), "%lf%lf%lf", &dx, &dy, &dz))
								{
									vd->setValue(gstSpcX, dx);
									vd->setValue(gstSpcY, dy);
									vd->setValue(gstSpcZ, dz);
								}
							}
						}
						else
						{
							if (fconfig.Read("b_res", &str))
							{
								if (SSCANF(str.c_str(), "%lf%lf%lf", &dx, &dy, &dz))
								{
									vd->setValue(gstBaseSpcX, dx);
									vd->setValue(gstBaseSpcY, dy);
									vd->setValue(gstBaseSpcZ, dz);
								}
							}
							if (fconfig.Read("s_res", &str))
							{
								if (SSCANF(str.c_str(), "%lf%lf%lf", &dx, &dy, &dz))
								{
									vd->setValue(gstSpcSclX, dx);
									vd->setValue(gstSpcSclY, dy);
									vd->setValue(gstSpcSclZ, dz);
								}
							}
						}
						if (fconfig.Read("scl", &str))
						{
							if (SSCANF(str.c_str(), "%lf%lf%lf", &dx, &dy, &dz))
							{
								vd->setValue(gstScaleX, dx);
								vd->setValue(gstScaleY, dy);
								vd->setValue(gstScaleZ, dz);
							}
						}

						vector<fluo::Plane*> *planes = 0;
						if (vd->GetRenderer())
							planes = vd->GetRenderer()->get_planes();
						long iresx, iresy, iresz;
						vd->getValue(gstResX, iresx);
						vd->getValue(gstResY, iresy);
						vd->getValue(gstResZ, iresz);
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

						float r, g, b;
						//2d adjustment settings
						if (fconfig.Read("gamma", &str))
						{
							if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)){
								vd->setValue(gstGammaR, double(r));
								vd->setValue(gstGammaG, double(g));
								vd->setValue(gstGammaB, double(b));
							}
						}
						if (fconfig.Read("brightness", &str))
						{
							if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)){
								vd->setValue(gstBrightnessR, double(r));
								vd->setValue(gstBrightnessG, double(g));
								vd->setValue(gstBrightnessB, double(b));
							}
						}
						if (fconfig.Read("hdr", &str))
						{
							if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)){
								vd->setValue(gstEqualizeR, double(r));
								vd->setValue(gstEqualizeG, double(g));
								vd->setValue(gstEqualizeB, double(b));
							}
						}
						if (fconfig.Read("sync_r", &bval))
							vd->setValue(gstSyncR, bval);
						if (fconfig.Read("sync_g", &bval))
							vd->setValue(gstSyncG, bval);
						if (fconfig.Read("sync_b", &bval))
							vd->setValue(gstSyncB, bval);

						//colormap settings
						if (fconfig.Read("colormap_mode", &lval))
							vd->setValue(gstColormapMode, lval);
						if (fconfig.Read("colormap_inv", &dval))
							vd->setValue(gstColormapInv, dval);
						if (fconfig.Read("colormap", &lval))
							vd->setValue(gstColormapType, lval);
						if (fconfig.Read("colormap_proj", &lval))
							vd->setValue(gstColormapProj, lval);
						if (fconfig.Read("colormap_lo_value", &dval))
							vd->setValue(gstColormapLow, dval);
						if (fconfig.Read("colormap_hi_value", &dval))
							vd->setValue(gstColormapHigh, dval);

						//high transp
						if (fconfig.Read("alpha_power", &dval))
							vd->setValue(gstAlphaPower, dval);
						//inversion
						if (fconfig.Read("inv", &bval))
							vd->setValue(gstInvert, bval);
						//mip enable
						if (fconfig.Read("mode", &lval))
							vd->setValue(gstMipMode, lval);
						//noise reduction
						if (fconfig.Read("noise_red", &bval))
							vd->setValue(gstNoiseRedct, bval);
						//depth override
						if (fconfig.Read("depth_ovrd", &lval))
							vd->setValue(gstBlendMode, lval);

						//shadow
						if (fconfig.Read("shadow", &bval))
							vd->setValue(gstShadowEnable, bval);
						//shaodw intensity
						if (fconfig.Read("shadow_darkness", &dval))
							vd->setValue(gstShadowInt, dval);

						//legend
						if (fconfig.Read("legend", &bval))
							vd->setValue(gstLegend, bval);

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
				fluo::MeshData* md = m_data_mgr.GetLastMeshData();
				if (md)
				{
					if (fconfig.Read("name", &str))
						md->setName(str.ToStdString());//setname
					//mesh properties
					if (fconfig.Exists("properties"))
					{
						fconfig.SetPath("properties");
						bool disp;
						if (fconfig.Read("display", &disp))
							md->setValue(gstDisplay, disp);
						//lighting
						bool lighting;
						if (fconfig.Read("lighting", &lighting))
							md->setValue(gstShadingEnable, lighting);
						float r=0.0f, g=0.0f, b=0.0f;
						if (fconfig.Read("color", &str))
						{
							SSCANF(str.c_str(), "%f%f%f", &r, &g, &b);
							fluo::Color color(r, g, b);
							md->setValue(gstColor, color);
						}
						if (fconfig.Read("ambient", &dval))
							md->setValue(gstMatAmb, dval);
						if (fconfig.Read("diffuse", &dval))
							md->setValue(gstMatDiff, dval);
						if (fconfig.Read("specular", &dval))
							md->setValue(gstMatSpec, dval);
						if (fconfig.Read("shininess", &dval))
							md->setValue(gstMatShine, dval);
						if (fconfig.Read("alpha", &dval))
							md->setValue(gstAlpha, dval);
						//2d adjusment settings
						if (fconfig.Read("gamma", &str))
						{
							float r, g, b;
							if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b))
							{
								md->setValue(gstGammaR, double(r));
								md->setValue(gstGammaG, double(g));
								md->setValue(gstGammaB, double(b));
							}
						}
						if (fconfig.Read("brightness", &str))
						{
							float r, g, b;
							if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b))
							{
								md->setValue(gstBrightnessR, double(r));
								md->setValue(gstBrightnessG, double(g));
								md->setValue(gstBrightnessB, double(b));
							}
						}
						if (fconfig.Read("hdr", &str))
						{
							float r, g, b;
							if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b))
							{
								md->setValue(gstEqualizeR, double(r));
								md->setValue(gstEqualizeG, double(g));
								md->setValue(gstEqualizeB, double(b));
							}
						}
						if (fconfig.Read("sync_r", &bval))
							md->setValue(gstSyncR, bval);
						if (fconfig.Read("sync_g", &bval))
							md->setValue(gstSyncG, bval);
						if (fconfig.Read("sync_b", &bval))
							md->setValue(gstSyncB, bval);
						//shadow
						if (fconfig.Read("shadow", &bval))
							md->setValue(gstShadowEnable, bval);
						if (fconfig.Read("shadow_darkness", &dval))
							md->setValue(gstShadowInt, dval);

						//mesh transform
						if (fconfig.Exists("../transform"))
						{
							fconfig.SetPath("../transform");
							float x, y, z;
							if (fconfig.Read("translation", &str) &&
								SSCANF(str.c_str(), "%f%f%f", &x, &y, &z))
							{
									md->setValue(gstTransX, double(x));
									md->setValue(gstTransY, double(y));
									md->setValue(gstTransZ, double(z));
							}
							if (fconfig.Read("rotation", &str) &&
								SSCANF(str.c_str(), "%f%f%f", &x, &y, &z))
							{
								md->setValue(gstRotX, double(x));
								md->setValue(gstRotY, double(y));
								md->setValue(gstRotZ, double(z));
							}
							if (fconfig.Read("scaling", &str) &&
								SSCANF(str.c_str(), "%f%f%f", &x, &y, &z))
							{
								md->setValue(gstScaleX, double(x));
								md->setValue(gstScaleY, double(y));
								md->setValue(gstScaleZ, double(z));
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
			fluo::Renderview* view = glbin_root->getChild(0)->asRenderview();
			if (!view)
				continue;

			view->Clear();

			if (i == 0 && m_setting_dlg && m_setting_dlg->GetTestMode(1))
				view->setValue(gstTestSpeed, true);

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
						fluo::VolumeData* vd = glbin_volf->findFirst(str.ToStdString());
						if (vd)
							view->addVolumeData(vd, 0);
					}
				}
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
						fluo::MeshData* md = m_data_mgr.GetMeshData(str.ToStdString());
						if (md)
							view->addMeshData(md, 0);
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
										fluo::VolumeData* vd = glbin_volf->findFirst(str.ToStdString());
										if (vd)
											view->addVolumeData(vd, 0);
									}
								}
								break;
							case 3://mesh data
								{
									if (fconfig.Read("name", &str))
									{
										fluo::MeshData* md = m_data_mgr.GetMeshData(str.ToStdString());
										if (md)
											view->addMeshData(md, 0);
									}
								}
								break;
							case 4://annotations
								{
									if (fconfig.Read("name", &str))
									{
										fluo::Annotations* ann = m_data_mgr.GetAnnotations(str.ToStdString());
										if (ann)
											view->addAnnotations(ann);
									}
								}
								break;
							case 5://group
								{
									if (fconfig.Read("name", &str))
									{
										int id;
										//if (fconfig.Read("id", &id))
										//	fluo::VolumeGroup::SetID(id);
										fluo::VolumeGroup* group = view->addVolumeGroup(str.ToStdString());
										if (group)
										{
											//display
											if (fconfig.Read("display", &bval))
											{
												group->setValue(gstDisplay, bval);
											}
											float r, g, b;
											//2d adjustment
											if (fconfig.Read("gamma", &str))
											{
												if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)) {
													group->setValue(gstGammaR, double(r));
													group->setValue(gstGammaG, double(g));
													group->setValue(gstGammaB, double(b));
												}
											}
											if (fconfig.Read("brightness", &str))
											{
												if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)) {
													group->setValue(gstBrightnessR, double(r));
													group->setValue(gstBrightnessG, double(g));
													group->setValue(gstBrightnessB, double(b));
												}
											}
											if (fconfig.Read("hdr", &str))
											{
												if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)) {
													group->setValue(gstEqualizeR, double(r));
													group->setValue(gstEqualizeG, double(g));
													group->setValue(gstEqualizeB, double(b));
												}
											}
											if (fconfig.Read("sync_r", &bval))
												group->setValue(gstSyncR, bval);
											if (fconfig.Read("sync_g", &bval))
												group->setValue(gstSyncG, bval);
											if (fconfig.Read("sync_b", &bval))
												group->setValue(gstSyncB, bval);
											//sync volume properties
											if (fconfig.Read("sync_vp", &bval))
												group->setValue(gstSyncGroup, bval);
											//volumes
											if (fconfig.Exists(wxString::Format("/views/%d/layers/%d/volumes", i, j)))
											{
												fconfig.SetPath(wxString::Format("/views/%d/layers/%d/volumes", i, j));
												int vol_num = fconfig.Read("num", 0l);
												for (k=0; k<vol_num; k++)
												{
													if (fconfig.Read(wxString::Format("vol_%d", k), &str))
													{
														fluo::VolumeData* vd = glbin_volf->findFirst(str.ToStdString());
														view->addVolumeData(vd, group);
													}
												}
											}
										}
									}
								}
								break;
							case 6://mesh group
								{
									if (fconfig.Read("name", &str))
									{
										int id;
										//if (fconfig.Read("id", &id))
										//	MeshGroup::SetID(id);
										fluo::MeshGroup* group = view->addMeshGroup(str.ToStdString());
										if (group)
										{
											//display
											if (fconfig.Read("display", &bval))
												group->setValue(gstDisplay, bval);
											//sync mesh properties
											if (fconfig.Read("sync_mp", &bval))
												group->setValue(gstSyncGroup, bval);
											//meshes
											if (fconfig.Exists(wxString::Format("/views/%d/layers/%d/meshes", i, j)))
											{
												fconfig.SetPath(wxString::Format("/views/%d/layers/%d/meshes", i, j));
												int mesh_num = fconfig.Read("num", 0l);
												for (k=0; k<mesh_num; k++)
												{
													if (fconfig.Read(wxString::Format("mesh_%d", k), &str))
													{
														fluo::MeshData* md = m_data_mgr.GetMeshData(str.ToStdString());
														view->addMeshData(md, group);
													}
												}
											}
										}
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
					view->setValue(gstDrawAll, draw);
				//properties
				bool persp;
				if (fconfig.Read("persp", &persp))
					view->setValue(gstPerspective, persp);
				else
					view->setValue(gstPerspective, true);
				bool free;
				if (fconfig.Read("free", &free))
					view->setValue(gstFree, free);
				else
					view->setValue(gstFree, false);
				double aov;
				if (fconfig.Read("aov", &aov))
					view->setValue(gstAov, aov);
				double nearclip;
				if (fconfig.Read("nearclip", &nearclip))
					view->setValue(gstDaStart, nearclip);
				double farclip;
				if (fconfig.Read("farclip", &farclip))
					view->setValue(gstDaEnd, farclip);
				if (fconfig.Read("backgroundcolor", &str))
				{
					float r, g, b;
					if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)){
						fluo::Color col(r,g,b);
						view->setValue(gstBgColor, col);
					}
				}
				long volmethod;
				if (fconfig.Read("volmethod", &volmethod))
					view->setValue(gstMixMethod, volmethod);
				long peellayers;
				if (fconfig.Read("peellayers", &peellayers))
					view->setValue(gstPeelNum, peellayers);
				bool fog;
				if (fconfig.Read("fog", &fog))
					view->setValue(gstDepthAtten, fog);
				double fogintensity;
				if (fconfig.Read("fogintensity", &fogintensity))
					view->setValue(gstDaInt, fogintensity);
					//view->m_vrv->m_depth_atten_factor_text->SetValue(wxString::Format("%.2f",fogintensity));
				if (fconfig.Read("draw_camctr", &bVal))
				{
					view->setValue(gstDrawCamCtr, bVal);
					//view->m_vrv->m_options_toolbar->ToggleTool(VRenderView::ID_CamCtrChk,bVal);
				}
				if (fconfig.Read("draw_info", &iVal))
				{
					view->setValue(gstDrawInfo, iVal);
					//view->m_vrv->m_options_toolbar->ToggleTool(VRenderView::ID_FpsChk, iVal & INFO_DISP);
				}
				if (fconfig.Read("draw_legend", &bVal))
				{
					view->setValue(gstDrawLegend, bVal);
					//view->m_vrv->m_options_toolbar->ToggleTool(VRenderView::ID_LegendChk,bVal);
				}

				//camera
				//if (fconfig.Read("translation", &str))
				//{
				//	if (SSCANF(str.c_str(), "%f%f%f", &x, &y, &z))
				//		view->SetTranslations(x, y, z);
				//}
				//if (fconfig.Read("rotation", &str))
				//{
				//	if (SSCANF(str.c_str(), "%f%f%f", &x, &y, &z))
				//		view->SetRotations(x, y, z);
				//}
				//if (fconfig.Read("zero_quat", &str))
				//{
				//	if (SSCANF(str.c_str(), "%f%f%f%f", &x, &y, &z, &w))
				//		view->SetZeroQuat(x, y, z, w);
				//}
				//if (fconfig.Read("center", &str))
				//{
				//	if (SSCANF(str.c_str(), "%f%f%f", &x, &y, &z))
				//		view->SetCenters(x, y, z);
				//}
				//double dist;
				//if (fconfig.Read("centereyedist", &dist))
				//	view->SetCenterEyeDist(dist);
				//double radius = 5.0;
				//if (fconfig.Read("radius", &radius))
				//	view->SetRadius(radius);
				//double initdist;
				//if (fconfig.Read("initdist", &initdist))
				//	view->SetInitDist(initdist);
				//else
				//	view->SetInitDist(radius/tan(d2r(view->GetAov()/2.0)));
				//int scale_mode;
				//if (fconfig.Read("scale_mode", &scale_mode))
				//	view->m_vrv->SetScaleMode(scale_mode, false);
				//double scale;
				//if (!fconfig.Read("scale", &scale))
				//	scale = radius / tan(d2r(view->GetAov() / 2.0)) / dist;
				//view->m_scale_factor = scale;
				//view->m_vrv->UpdateScaleFactor(false);
				//bool pin_rot_center;
				//if (fconfig.Read("pin_rot_center", &pin_rot_center))
				//{
				//	view->m_pin_rot_center = pin_rot_center;
				//	if (pin_rot_center)
				//		view->m_rot_center_dirty = true;
				//}
				////object
				//if (fconfig.Read("obj_center", &str))
				//{
				//	if (SSCANF(str.c_str(), "%f%f%f", &x, &y, &z))
				//		view->SetObjCenters(x, y, z);
				//}
				//if (fconfig.Read("obj_trans", &str))
				//{
				//	if (SSCANF(str.c_str(), "%f%f%f", &x, &y, &z))
				//		view->SetObjTrans(x, y, z);
				//}
				//if (fconfig.Read("obj_rot", &str))
				//{
				//	if (SSCANF(str.c_str(), "%f%f%f", &x, &y, &z))
				//	{
				//		if (l_major <= 2 && d_minor < 24.3)
				//			view->SetObjRot(x, y+180.0, z+180.0);
				//		else
				//			view->SetObjRot(x, y, z);
				//	}
				//}
				////scale bar
				//bool disp;
				//if (fconfig.Read("disp_scale_bar", &disp))
				//	view->m_disp_scale_bar = disp;
				//if (fconfig.Read("disp_scale_bar_text", &disp))
				//	view->m_disp_scale_bar_text = disp;
				//double length;
				//if (fconfig.Read("sb_length", &length))
				//	view->m_sb_length = length;
				//if (fconfig.Read("sb_text", &str))
				//	view->m_sb_text = str;
				//if (fconfig.Read("sb_num", &str))
				//	view->m_sb_num = str;
				//int unit;
				//if (fconfig.Read("sb_unit", &unit))
				//	view->m_sb_unit = unit;

				////2d sdjustment settings
				//if (fconfig.Read("gamma", &str))
				//{
				//	float r, g, b;
				//	if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)){
				//		fluo::Color col(r,g,b);
				//		view->SetGamma(col);
				//	}
				//}
				//if (fconfig.Read("brightness", &str))
				//{
				//	float r, g, b;
				//	if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)){
				//		fluo::Color col(r,g,b);
				//		view->SetBrightness(col);
				//	}
				//}
				//if (fconfig.Read("hdr", &str))
				//{
				//	float r, g, b;
				//	if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)){
				//		fluo::Color col(r,g,b);
				//		view->SetHdr(col);
				//	}
				//}
				//if (fconfig.Read("sync_r", &bVal))
				//	view->SetSyncR(bVal);
				//if (fconfig.Read("sync_g", &bVal))
				//	view->SetSyncG(bVal);
				//if (fconfig.Read("sync_b", &bVal))
				//	view->SetSyncB(bVal);

				////clipping plane rotations
				//int clip_mode;
				//if (fconfig.Read("clip_mode", &clip_mode))
				//	view->SetClipMode(clip_mode);
				//double rotx_cl, roty_cl, rotz_cl;
				//if (fconfig.Read("rotx_cl", &rotx_cl) &&
				//	fconfig.Read("roty_cl", &roty_cl) &&
				//	fconfig.Read("rotz_cl", &rotz_cl))
				//{
				//	view->SetClippingPlaneRotations(rotx_cl, roty_cl, rotz_cl);
				//	m_clip_view->SetClippingPlaneRotations(rotx_cl, roty_cl, rotz_cl);
				//}

				////painting parameters
				//double dVal;
				//flrd::VolumeSelector* selector = view->GetVolumeSelector();
				//if (selector)
				//{
				//	if (fconfig.Read("brush_use_pres", &bVal))
				//		selector->SetBrushUsePres(bVal);
				//	double size1, size2;
				//	if (fconfig.Read("brush_size_1", &size1) &&
				//		fconfig.Read("brush_size_2", &size2))
				//		selector->SetBrushSize(size1, size2);
				//	if (fconfig.Read("brush_spacing", &dVal))
				//		selector->SetBrushSpacing(dVal);
				//	if (fconfig.Read("brush_iteration", &dVal))
				//		selector->SetBrushIteration(dVal);
				//	if (fconfig.Read("brush_size_data", &bVal))
				//		selector->SetBrushSizeData(bVal);
				//	if (fconfig.Read("brush_translate", &dVal))
				//		selector->SetBrushSclTranslate(dVal);
				//	if (fconfig.Read("w2d", &dVal))
				//		selector->SetW2d(dVal);
				//}

				////rulers
				//if (view->GetRulerList() &&
				//	fconfig.Exists(wxString::Format("/views/%d/rulers", i)))
				//{
				//	fconfig.SetPath(wxString::Format("/views/%d/rulers", i));
				//	view->GetRulerHandler()->Read(fconfig, i);
				//}
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
				fluo::VolumeData* vd = m_data_mgr.GetVolumeData(m_cur_sel_vol);
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
		RenderCanvas* view = 0;
		if (fconfig.Read("cur_page", &iVal))
		{
			m_movie_view->SetCurrentPage(iVal);
		}
		if (fconfig.Read("views_cmb", &iVal))
		{
			m_movie_view->SetView(iVal);
			view = GetView(iVal);
		}
		if (fconfig.Read("rot_check", &bVal))
		{
			m_movie_view->SetRotate(bVal);
		}
		if (fconfig.Read("seq_check", &bVal))
		{
			m_movie_view->SetTimeSeq(bVal);
		}
		if (fconfig.Read("x_rd", &bVal))
		{
			if (bVal)
				m_movie_view->SetRotAxis(0);
		}
		if (fconfig.Read("y_rd", &bVal))
		{
			if (bVal)
				m_movie_view->SetRotAxis(1);
		}
		if (fconfig.Read("z_rd", &bVal))
		{
			if (bVal)
				m_movie_view->SetRotAxis(2);
		}
		if (fconfig.Read("rot_axis", &iVal))
		{
			m_movie_view->SetRotAxis(iVal);
		}
		if (fconfig.Read("rot_deg", &iVal))
		{
			m_movie_view->SetRotDeg(iVal);
		}
		if (fconfig.Read("movie_len", &dVal))
		{
			m_movie_view->SetMovieLen(dVal);
		}
		if (fconfig.Read("fps", &dVal))
		{
			m_movie_view->SetFps(dVal);
		}
		if (fconfig.Read("crop", &bVal))
		{
			m_movie_view->SetCrop(bVal);
		}
		bool b_x, b_y, b_w, b_h;
		b_x = b_y = b_w = b_h = false;
		if (fconfig.Read("crop_x", &iVal))
		{
			m_movie_view->SetCropX(iVal);
			b_x = true;
		}
		if (fconfig.Read("crop_y", &iVal))
		{
			m_movie_view->SetCropY(iVal);
			b_y = true;
		}
		if (fconfig.Read("crop_w", &iVal))
		{
			m_movie_view->SetCropW(iVal);
			b_w = true;
		}
		if (fconfig.Read("crop_h", &iVal))
		{
			m_movie_view->SetCropH(iVal);
			b_h = true;
		}
		if (b_x && b_y && b_w && b_h)
		{
			m_movie_view->UpdateCrop();
		}
		if (fconfig.Read("start_frame", &iVal))
			m_movie_view->SetStartFrame(iVal);
		if (fconfig.Read("end_frame", &iVal))
			m_movie_view->SetEndFrame(iVal);
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
			//m_trace_dlg->GetSettings(m_vrv_list[0]->GetCanvas());
			//m_trace_dlg->LoadTrackFile(sVal.ToStdWstring());
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
				UpdateTree(m_data_mgr.GetVolumeData(m_cur_sel_vol)->getName());
			else
				UpdateTree();
			break;
		case 3:  //mesh
			if (m_data_mgr.GetMeshData(m_cur_sel_mesh))
				UpdateTree(m_data_mgr.GetMeshData(m_cur_sel_mesh)->getName());
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
			UpdateTree(m_data_mgr.GetVolumeData(m_cur_sel_vol)->getName());
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
	m_setting_dlg->UpdateDeviceTree();
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
		flvr::Texture::mask_undo_num_ = (size_t)(m_setting_dlg->GetPaintHistDepth());
}

void VRenderFrame::SetTextureRendererSettings()
{
	if (!m_setting_dlg)
		return;

	flvr::TextureRenderer::set_mem_swap(m_setting_dlg->GetMemSwap());
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
	flvr::TextureRenderer::set_use_mem_limit(use_mem_limit);
	flvr::TextureRenderer::set_mem_limit(use_mem_limit?
		m_setting_dlg->GetGraphicsMem():mem_info[0]/1024.0);
	flvr::TextureRenderer::set_available_mem(use_mem_limit?
		m_setting_dlg->GetGraphicsMem():mem_info[0]/1024.0);
	flvr::TextureRenderer::set_large_data_size(m_setting_dlg->GetLargeDataSize());
	flvr::TextureRenderer::set_force_brick_size(m_setting_dlg->GetForceBrickSize());
	flvr::TextureRenderer::set_up_time(m_setting_dlg->GetResponseTime());
	flvr::TextureRenderer::set_update_order(m_setting_dlg->GetUpdateOrder());
	flvr::TextureRenderer::set_invalidate_tex(m_setting_dlg->GetInvalidateTex());
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

