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
#include "RenderFrame.h"
#include "DragDrop.h"
#include <Global.hpp>
#include <AgentFactory.hpp>
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

BEGIN_EVENT_TABLE(RenderFrame, wxFrame)
	EVT_MENU(wxID_EXIT, RenderFrame::OnExit)
	EVT_MENU(ID_ViewNew, RenderFrame::OnNewView)
	EVT_MENU(ID_Layout, RenderFrame::OnLayout)
	EVT_MENU(ID_FullScreen, RenderFrame::OnFullScreen)
	EVT_MENU(ID_OpenVolume, RenderFrame::OnOpenVolume)
	EVT_MENU(ID_OpenMesh, RenderFrame::OnOpenMesh)
	EVT_MENU(ID_ViewOrganize, RenderFrame::OnOrganize)
	EVT_MENU(ID_CheckUpdates, RenderFrame::OnCheckUpdates)
	EVT_MENU(ID_Info, RenderFrame::OnInfo)
	EVT_MENU(ID_CreateCube, RenderFrame::OnCreateCube)
	EVT_MENU(ID_CreateSphere, RenderFrame::OnCreateSphere)
	EVT_MENU(ID_CreateCone, RenderFrame::OnCreateCone)
	EVT_MENU(ID_SaveProject, RenderFrame::OnSaveProject)
	EVT_MENU(ID_OpenProject, RenderFrame::OnOpenProject)
	EVT_MENU(ID_Settings, RenderFrame::OnSettings)
	EVT_MENU(ID_ImportVolume, RenderFrame::OnImportVolume)
	//tools
	EVT_MENU(ID_LastTool, RenderFrame::OnLastTool)
	EVT_MENU(ID_PaintTool, RenderFrame::OnPaintTool)
	EVT_MENU(ID_Measure, RenderFrame::OnMeasure)
	EVT_MENU(ID_Trace, RenderFrame::OnTrace)
	EVT_MENU(ID_NoiseCancelling, RenderFrame::OnNoiseCancelling)
	EVT_MENU(ID_Counting, RenderFrame::OnCounting)
	EVT_MENU(ID_Colocalization, RenderFrame::OnColocalization)
	EVT_MENU(ID_Convert, RenderFrame::OnConvert)
	EVT_MENU(ID_Ocl, RenderFrame::OnOcl)
	EVT_MENU(ID_Component, RenderFrame::OnComponent)
	EVT_MENU(ID_Calculations, RenderFrame::OnCalculations)
	//
	EVT_MENU(ID_Youtube, RenderFrame::OnYoutube)
	EVT_MENU(ID_Twitter, RenderFrame::OnTwitter)
	EVT_MENU(ID_Facebook, RenderFrame::OnFacebook)
	EVT_MENU(ID_Manual, RenderFrame::OnManual)
	EVT_MENU(ID_Tutorial, RenderFrame::OnTutorial)
	EVT_MENU(ID_ShowHideUI, RenderFrame::OnShowHideUI)
	EVT_MENU(ID_ShowHideToolbar, RenderFrame::OnShowHideToolbar)
	//ui menu events
	EVT_MENU(ID_UIListView, RenderFrame::OnShowHideView)
	EVT_MENU(ID_UITreeView, RenderFrame::OnShowHideView)
	EVT_MENU(ID_UIMovieView, RenderFrame::OnShowHideView)
	EVT_MENU(ID_UIAdjView, RenderFrame::OnShowHideView)
	EVT_MENU(ID_UIClipView, RenderFrame::OnShowHideView)
	EVT_MENU(ID_UIPropView, RenderFrame::OnShowHideView)
	//panes
	EVT_AUI_PANE_CLOSE(RenderFrame::OnPaneClose)
	//draw background
	EVT_PAINT(RenderFrame::OnDraw)
	//process key event
	EVT_KEY_DOWN(RenderFrame::OnKeyDown)
	//close
	EVT_CLOSE(RenderFrame::OnClose)
END_EVENT_TABLE()

RenderFrame::RenderFrame(
	wxFrame* frame,
	const wxString& title,
	int x, int y,
	int w, int h,
	bool benchmark,
	bool fullscreen,
	bool windowed,
	bool hidepanels)
	: wxFrame(frame, wxID_ANY, title, wxPoint(x, y), wxSize(w, h),wxDEFAULT_FRAME_STYLE)
{
	m_agent = glbin_agtf->addRenderFrameAgent(gstRenderFrameAgent, *this);

	// temporarily block events during constructor:
	//wxEventBlocker blocker(this);

#ifdef _DARWIN
	SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#endif

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

	if (JVMInitializer::getInstance(m_agent->GetJvmArgs()) != nullptr)
	{
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
	RenderviewPanel *vrv = new RenderviewPanel(this);
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
	m_tree_panel->SetScenegraph(glbin_root);

	//create movie view (sets the m_recorder_dlg)
	m_movie_view = new MoviePanel(this,
		wxDefaultPosition, panel_size);

	//create prop panel
	m_prop_panel = new wxPanel(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, 0, "PropPanel");
	//prop panel chidren
	m_prop_sizer = new wxBoxSizer(wxHORIZONTAL);
	m_volume_prop = new VolumePropPanel(m_prop_panel);
	m_mesh_prop = new MeshPropPanel(m_prop_panel);
	m_mesh_manip = new MeshTransPanel(m_prop_panel);
	m_annotation_prop = new AnnotationPropPanel(m_prop_panel);
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
	m_clip_view = new ClipPlanePanel(this,
		wxDefaultPosition, wxSize(130,700));
	//m_clip_view->SetDataManager(&m_data_mgr);
	//m_clip_view->SetPlaneMode(static_cast<PLANE_MODES>(
	//	m_setting_dlg->GetPlaneMode()));

	//adjust view
	m_adjust_view = new OutAdjustPanel(this,
		wxDefaultPosition, wxSize(130, 700));

	std::string sval;
	m_agent->getValue(gstFontFile, sval);
	wxString font_file = sval;
	wxString exePath = wxStandardPaths::Get().GetExecutablePath();
	exePath = wxPathOnly(exePath);
	if (font_file != "")
		font_file = exePath + GETSLASH() + "Fonts" +
			GETSLASH() + font_file;
	else
		font_file = exePath + GETSLASH() + "Fonts" +
			GETSLASH() + "FreeSans.ttf";
	flvr::TextRenderer::text_texture_manager_.load_face(font_file.ToStdString());
	double dval;
	m_agent->getValue(gstTextSize, dval);
	flvr::TextRenderer::text_texture_manager_.SetSize(dval);

	//settings dialog
	m_setting_dlg = new SettingDlg(this);
	m_setting_dlg->AssociateRoot();
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
	//m_time_id = m_setting_dlg->GetTimeId();
	bool bval;
	long lval;
	m_agent->getValue(gstSoftThresh, dval);
	flvr::VolumeRenderer::set_soft_threshold(dval);
	flvr::MultiVolumeRenderer::set_soft_threshold(dval);
	VolumeMeshConv::SetSoftThreshold(dval);

	//brush tool dialog
	m_brush_tool_dlg = new BrushToolDlg(this, m_tree_panel);
	m_tree_panel->SetBrushToolAgent();

	//noise cancelling dialog
	m_noise_cancelling_dlg = new NoiseReduceDlg(this);

	//counting dialog
	m_counting_dlg = new CountingDlg(this);

	//convert dialog
	m_convert_dlg = new ConvertDlg(this);

	//colocalization dialog
	m_colocalization_dlg = new ColocalDlg(this);

	//measure dialog
	m_measure_dlg = new MeasureDlg(this);

	//ocl dialog
	m_ocl_dlg = new ClKernelDlg(this);

	//component dialog
	m_component_dlg = new ComponentDlg(this);

	//trace dialog
	m_trace_dlg = new TrackDlg(this);
	m_trace_dlg->SetComponentAgent();
	m_trace_dlg->SetMovieAgent();
	//m_trace_dlg->SetCellSize(m_setting_dlg->GetComponentSize());

	//calculation dialog
	m_calculation_dlg = new CalculationDlg(this);

	//help dialog
	m_help_dlg = new HelpDlg(this);

	//tester
	//shown for testing parameters
	m_tester = new TesterDlg(this);
	m_agent->getValue(gstTestParam, bval);
	if (bval)
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

	//UpdateTree();

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

	if (JVMInitializer::getInstance(m_agent->GetJvmArgs()) != nullptr)
	{
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
	m_agent->getValue(gstLastTool, lval);
	switch (lval)
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
	//SetSaveProject(m_setting_dlg->GetProjSave());
	//SetSaveAlpha(m_setting_dlg->GetSaveAlpha());
	//SetSaveFloat(m_setting_dlg->GetSaveFloat());

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

RenderFrame::~RenderFrame()
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
	m_aui_mgr.UnInit();
	flvr::KernelProgram::release();
}

void RenderFrame::AssociateRoot()
{
	m_agent->setObject(glbin_root);
}

void RenderFrame::OnExit(wxCommandEvent& WXUNUSED(event))
{
	Close(true);
}

void RenderFrame::OnClose(wxCloseEvent &event)
{
	//m_setting_dlg->SaveSettings();
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

wxString RenderFrame::CreateView(int row)
{
	RenderviewPanel* vrv = 0;
	if (m_vrv_list.size()>0)
	{
		wxGLContext* sharedContext = m_vrv_list[0]->GetContext();
		vrv = new RenderviewPanel(this, sharedContext);
	}
	else
	{
		vrv = new RenderviewPanel(this);
	}

	if (!vrv)
		return "NO_NAME";
	//RenderCanvas* view = vrv->m_glview;
	//if (!view)
	//	return "NO_NAME";

	m_vrv_list.push_back(vrv);
	//if (m_movie_view)
	//	m_movie_view->AddView(vrv->GetName());
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
	glbin_agtf->getRenderCanvasAgent(GetName().ToStdString())->setValue(gstSetGl, false);

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

	//UpdateTree();

	return vrv->GetName();
}

void RenderFrame::OnNewView(wxCommandEvent& WXUNUSED(event))
{
	wxString str = CreateView();
}

void RenderFrame::OnLayout(wxCommandEvent& WXUNUSED(event))
{
	OrganizeVRenderViews(1);
}

void RenderFrame::OnFullScreen(wxCommandEvent& WXUNUSED(event))
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
void RenderFrame::OnCh11Check(wxCommandEvent &event)
{
	wxCheckBox* ch11 = (wxCheckBox*)event.GetEventObject();
	if (ch11)
	{
		bool bval = ch11->GetValue();
		m_agent->setValue(gstOpenSlices, bval);
	}
}

void RenderFrame::OnCh12Check(wxCommandEvent &event)
{
	wxCheckBox* ch12 = (wxCheckBox*)event.GetEventObject();
	if (ch12)
	{
		bool bval = ch12->GetValue();
		m_agent->setValue(gstOpenChanns, bval);
	}
}

void RenderFrame::OnCmbChange(wxCommandEvent &event)
{
	wxComboBox* combo = (wxComboBox*)event.GetEventObject();
	if (combo)
	{
		long lval = combo->GetSelection();
		m_agent->setValue(gstOpenDigitOrder, lval);
	}
}

void RenderFrame::OnTxt1Change(wxCommandEvent &event)
{
	wxTextCtrl* txt1 = (wxTextCtrl*)event.GetEventObject();
	if (txt1)
	{
		std::string sval = txt1->GetValue().ToStdString();
		m_agent->setValue(gstTimeFileId, sval);
	}
}

void RenderFrame::OnTxt2Change(wxCommandEvent &event)
{
	wxTextCtrl* txt2 = (wxTextCtrl*)event.GetEventObject();
	if (txt2)
	{
		wxString str = txt2->GetValue();
		long lval;
		if (str.ToLong(&lval))
			m_agent->setValue(gstOpenSeriesNum, lval);
	}
}

void RenderFrame::OnCh2Check(wxCommandEvent &event)
{
	wxCheckBox* ch2 = (wxCheckBox*)event.GetEventObject();
	if (ch2)
	{
		bool bval = ch2->GetValue();
		m_agent->setValue(gstHardwareCompress, bval);
	}
}

void RenderFrame::OnCh3Check(wxCommandEvent &event)
{
	wxCheckBox* ch3 = (wxCheckBox*)event.GetEventObject();
	if (ch3)
	{
		bool bval = ch3->GetValue();
		m_agent->setValue(gstSkipBrick, bval);
	}
}

wxWindow* RenderFrame::CreateExtraControlVolume(wxWindow* parent)
{
	fluo::RenderFrameAgent* agent = glbin_agtf->getRenderFrameAgent();
	bool bval; long lval; std::string sval;

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
		wxCommandEventHandler(RenderFrame::OnCh11Check), NULL, panel);
	agent->getValue(gstOpenSlices, bval);
	ch11->SetValue(bval);

	//slice sequence check box
	wxCheckBox* ch12 = new wxCheckBox(panel, ID_READ_CHANN,
		"Read file# as channels");
	ch12->Connect(ch12->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(RenderFrame::OnCh12Check), NULL, panel);
	agent->getValue(gstOpenChanns, bval);
	ch12->SetValue(bval);

	//digit order
	st1 = new wxStaticText(panel, 0,
		"Digit order:");
	wxComboBox* combo = new wxComboBox(panel, ID_DIGI_ORDER,
		"Order", wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY);
	combo->Connect(combo->GetId(), wxEVT_COMMAND_COMBOBOX_SELECTED,
		wxCommandEventHandler(RenderFrame::OnCmbChange), NULL, panel);
	std::vector<std::string> combo_list;
	combo_list.push_back("Channel first");
	combo_list.push_back("Z section first");
	for (size_t i = 0; i < combo_list.size(); ++i)
		combo->Append(combo_list[i]);
	agent->getValue(gstOpenDigitOrder, lval);
	combo->SetSelection(lval);

	//series number
	st2 = new wxStaticText(panel, 0,
		"Serial:");
	wxTextCtrl* txt2 = new wxTextCtrl(panel, ID_SER_NUM,
		"", wxDefaultPosition, wxDefaultSize);
	txt2->Connect(txt2->GetId(), wxEVT_COMMAND_TEXT_UPDATED,
		wxCommandEventHandler(RenderFrame::OnTxt2Change), NULL, panel);

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
		wxCommandEventHandler(RenderFrame::OnCh2Check), NULL, panel);
	agent->getValue(gstHardwareCompress, bval);
	ch2->SetValue(bval);

	//empty brick skipping
	wxCheckBox* ch3 = new wxCheckBox(panel, ID_SKIP_BRICKS,
		"Skip empty bricks during rendering (loading takes longer time)");
	ch3->Connect(ch3->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(RenderFrame::OnCh3Check), NULL, panel);
	agent->getValue(gstSkipBrick, bval);
	ch3->SetValue(bval);

	//time sequence identifier
	wxBoxSizer* sizer2 = new wxBoxSizer(wxHORIZONTAL);
	wxTextCtrl* txt1 = new wxTextCtrl(panel, ID_TSEQ_ID,
		"", wxDefaultPosition, wxDefaultSize);
	agent->getValue(gstTimeFileId, sval);
	txt1->SetValue(sval);
	txt1->Connect(txt1->GetId(), wxEVT_COMMAND_TEXT_UPDATED,
		wxCommandEventHandler(RenderFrame::OnTxt1Change), NULL, panel);
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

wxWindow* RenderFrame::CreateExtraControlVolumeForImport(wxWindow* parent)
{
	fluo::RenderFrameAgent* agent = glbin_agtf->getRenderFrameAgent();
	bool bval;

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
		wxCommandEventHandler(RenderFrame::OnCh1Check), NULL, panel);
	ch1->SetValue(m_sliceSequence);
	*/

	//compression
	wxCheckBox* ch2 = new wxCheckBox(panel, ID_COMPRESS,
		"Compress data (loading will take longer time and data are compressed in graphics memory)");
	ch2->Connect(ch2->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(RenderFrame::OnCh2Check), NULL, panel);
	agent->getValue(gstHardwareCompress, bval);
	ch2->SetValue(bval);

	//empty brick skipping
	wxCheckBox* ch3 = new wxCheckBox(panel, ID_SKIP_BRICKS,
		"Skip empty bricks during rendering (loading takes longer time)");
	ch3->Connect(ch3->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(RenderFrame::OnCh3Check), NULL, panel);
	agent->getValue(gstSkipBrick, bval);
	ch3->SetValue(bval);

	//time sequence identifier. TODO: Not supported as of now.
	/*
	wxBoxSizer* sizer1 = new wxBoxSizer(wxHORIZONTAL);
	wxTextCtrl* txt1 = new wxTextCtrl(panel, ID_TSEQ_ID,
		"", wxDefaultPosition, wxSize(80, 20));
	txt1->SetValue(m_time_id);
	txt1->Connect(txt1->GetId(), wxEVT_COMMAND_TEXT_UPDATED,
		wxCommandEventHandler(RenderFrame::OnTxt1Change), NULL, panel);
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

void RenderFrame::OnOpenVolume(wxCommandEvent& WXUNUSED(event))
{
	//if (m_setting_dlg)
	//{
	//	m_compression = m_setting_dlg->GetRealtimeCompress();
	//	m_skip_brick = m_setting_dlg->GetSkipBricks();
	//}

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
		m_agent->LoadVolumes(paths, false);

		//if (m_setting_dlg)
		//{
		//	m_setting_dlg->SetRealtimeCompress(m_compression);
		//	m_setting_dlg->SetSkipBricks(m_skip_brick);
		//	m_setting_dlg->UpdateUI();
		//}
	}

	delete fopendlg;
}

void RenderFrame::OnImportVolume(wxCommandEvent& WXUNUSED(event))
{
	//if (m_setting_dlg)
	//{
	//	m_compression = m_setting_dlg->GetRealtimeCompress();
	//	m_skip_brick = m_setting_dlg->GetSkipBricks();
	//}

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
		m_agent->LoadVolumes(paths, true);

		//if (m_setting_dlg)
		//{
		//	m_setting_dlg->SetRealtimeCompress(m_compression);
		//	m_setting_dlg->SetSkipBricks(m_skip_brick);
		//	m_setting_dlg->UpdateUI();
		//}
	}

	delete fopendlg;
}

void RenderFrame::OnOpenMesh(wxCommandEvent& WXUNUSED(event))
{
	wxFileDialog *fopendlg = new wxFileDialog(
		this, "Choose the mesh data file",
		"", "", "*.obj", wxFD_OPEN|wxFD_MULTIPLE);

	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		//RenderCanvas* view = GetView(0);
		wxArrayString files;
		fopendlg->GetPaths(files);

		m_agent->LoadMeshes(files);
	}

	if (fopendlg)
		delete fopendlg;
}

void RenderFrame::OnOrganize(wxCommandEvent& WXUNUSED(event))
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

void RenderFrame::OnCheckUpdates(wxCommandEvent &)
{
	::wxLaunchDefaultBrowser(VERSION_UPDATES);
}

void RenderFrame::OnInfo(wxCommandEvent& WXUNUSED(event))
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

wxString RenderFrame::ScriptDialog(const wxString& title,
	const wxString& wildcard, long style)
{
	fluo::MovieAgent* agent = glbin_agtf->getMovieAgent();
	if (agent) agent->HoldRun();
	wxString result;
	wxFileDialog *dlg = new wxFileDialog(
		this, title, "", "",
		wildcard, style);
	int rval = dlg->ShowModal();
	if (rval == wxID_OK)
		result = dlg->GetPath();
	delete dlg;
	if (agent) agent->ResumeRun();
	return result;
}

//on selections
void RenderFrame::OnSelection(fluo::Node *node)
{
	if (!node)
		return;

	if (m_adjust_view)
		m_adjust_view->AssociateNode(node);
	if (m_clip_view)
		m_clip_view->AssociateNode(node);

	//clip plane renderer
	//FLR::ClipPlaneRenderer* renderer =
	//	FL::Global::instance().getProcessorFactory().
	//	getOrAddClipPlaneRenderer("clip plane renderer");
	//if (renderer)
	//	renderer->copyInputValues(*node);

	//{
	//	m_adjust_view->SetRenderView(vrv);
	//	if (!vrv || vd)
	//		m_adjust_view->SetVolumeData(vd);
	//}

	//if (m_clip_view)
	//{
	//	switch (type)
	//	{
	//	case 2:
	//		m_clip_view->SetVolumeData(vd);
	//		break;
	//	case 3:
	//		m_clip_view->SetMeshData(md);
	//		break;
	//	case 4:
	//		if (ann)
	//		{
	//			VolumeData* vd_ann = ann->GetVolume();
	//			m_clip_view->SetVolumeData(vd_ann);
	//		}
	//		break;
	//	}
	//}

	//m_cur_sel_type = type;
	//clear mesh boundbox
	//if (m_data_mgr.GetMeshData(m_cur_sel_mesh))
	//	m_data_mgr.GetMeshData(m_cur_sel_mesh)->SetDrawBounds(false);

	std::string type(node->className());
	if (type == "VolumeData")
	{
		//	if (m_volume_prop)
		//		m_volume_prop->Show(false);
		//	if (m_mesh_prop)
		//		m_mesh_prop->Show(false);
		//	if (m_mesh_manip)
		//		m_mesh_manip->Show(false);
		//	if (m_annotation_prop)
		//		m_annotation_prop->Show(false);
		//	m_aui_mgr.GetPane(m_prop_panel).Caption(UITEXT_PROPERTIES);
		//	m_aui_mgr.Update();
		//	break;
		//case 2:  //volume
		fluo::VolumeData* vd = node->asVolumeData();
		bool display = false;
		if (vd)
			vd->getValue(gstDisplay, display);
		if (display)
		{
			m_volume_prop->AssociateVolumeData(vd);
			//m_volume_prop->SetGroup(group);
			//m_volume_prop->SetView(vrv);
			if (!m_volume_prop->IsShown())
			{
				m_volume_prop->Show(true);
				m_prop_sizer->Clear();
				m_prop_sizer->Add(m_volume_prop, 1, wxEXPAND, 0);
				m_prop_panel->SetSizer(m_prop_sizer);
				m_prop_panel->Layout();
			}
			m_aui_mgr.GetPane(m_prop_panel).Caption(
				wxString(UITEXT_PROPERTIES) + wxString(" - ") + vd->getName());
			m_aui_mgr.Update();
			//wxString str = vd->getName();
			//m_cur_sel_vol = m_data_mgr.GetVolumeIndex(str);

			glbin_root->setRvalu(gstCurrentVolume, vd);
			for (size_t i = 0; i < glbin_root->getNumChildren(); ++i)
			{
				fluo::Renderview* view = glbin_root->getChild(i)->asRenderview();
				if (!view)
					continue;
				view->setRvalu(gstCurrentVolume, vd);
			}

			if (m_volume_prop)
				m_volume_prop->Show(true);
			if (m_mesh_prop)
				m_mesh_prop->Show(false);
			if (m_mesh_manip)
				m_mesh_manip->Show(false);
			if (m_annotation_prop)
				m_annotation_prop->Show(false);

			//if (m_adjust_view)
			//	m_adjust_view->SetVolumeData(vd);

			//if (m_clip_view)
			//	m_clip_view->SetVolumeData(vd);
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
	}
	else if (type == "MeshData")
	{
		fluo::MeshData* md = node->asMeshData();
		bool display = false;
		if (md)
			md->getValue(gstDisplay, display);
		if (display)
		{
			m_mesh_prop->AssociateMeshData(md);
			if (!m_mesh_prop->IsShown())
			{
				m_mesh_prop->Show(true);
				m_prop_sizer->Clear();
				m_prop_sizer->Add(m_mesh_prop, 1, wxEXPAND, 0);
				m_prop_panel->SetSizer(m_prop_sizer);
				m_prop_panel->Layout();
			}
			m_aui_mgr.GetPane(m_prop_panel).Caption(
				wxString(UITEXT_PROPERTIES) + wxString(" - ") + md->getName());
			m_aui_mgr.Update();
			md->setValue(gstDrawBounds, true);

			if (m_volume_prop)
				m_volume_prop->Show(false);
			if (m_mesh_prop && md)
				m_mesh_prop->Show(true);
			if (m_mesh_manip)
				m_mesh_manip->Show(false);
			if (m_annotation_prop)
				m_annotation_prop->Show(false);
		}
	}
	//	break;
	//case 3:  //mesh
	//	if (md)
	//	{
	//		m_mesh_prop->SetMeshData(md, vrv);
	//		if (!m_mesh_prop->IsShown())
	//		{
	//			m_mesh_prop->Show(true);
	//			m_prop_sizer->Clear();
	//			m_prop_sizer->Add(m_mesh_prop, 1, wxEXPAND, 0);
	//			m_prop_panel->SetSizer(m_prop_sizer);
	//			m_prop_panel->Layout();
	//		}
	//		m_aui_mgr.GetPane(m_prop_panel).Caption(
	//			wxString(UITEXT_PROPERTIES)+wxString(" - ")+md->GetName());
	//		m_aui_mgr.Update();
	//		wxString str = md->GetName();
	//		m_cur_sel_mesh = m_data_mgr.GetMeshIndex(str);
	//		md->SetDrawBounds(true);
	//	}

	//	if (m_volume_prop)
	//		m_volume_prop->Show(false);
	//	if (m_mesh_prop && md)
	//		m_mesh_prop->Show(true);
	//	if (m_mesh_manip)
	//		m_mesh_manip->Show(false);
	//	if (m_annotation_prop)
	//		m_annotation_prop->Show(false);
	//	break;
	//case 4:  //annotations
	//	if (ann)
	//	{
	//		m_annotation_prop->SetAnnotations(ann, vrv);
	//		if (!m_annotation_prop->IsShown())
	//		{
	//			m_annotation_prop->Show(true);
	//			m_prop_sizer->Clear();
	//			m_prop_sizer->Add(m_annotation_prop, 1, wxEXPAND, 0);
	//			m_prop_panel->SetSizer(m_prop_sizer);
	//			m_prop_panel->Layout();
	//		}
	//		m_aui_mgr.GetPane(m_prop_panel).Caption(
	//			wxString(UITEXT_PROPERTIES)+wxString(" - ")+ann->GetName());
	//		m_aui_mgr.Update();
	//	}

	//	if (m_volume_prop)
	//		m_volume_prop->Show(false);
	//	if (m_mesh_prop)
	//		m_mesh_prop->Show(false);
	//	if (m_mesh_manip)
	//		m_mesh_manip->Show(false);
	//	if (m_annotation_prop && ann)
	//		m_annotation_prop->Show(true);
	//	break;
	//case 5:  //group
	//	if (m_adjust_view)
	//		m_adjust_view->SetGroup(group);
	//	if (m_calculation_dlg)
	//		m_calculation_dlg->SetGroup(group);
	//	if (m_volume_prop)
	//		m_volume_prop->Show(false);
	//	if (m_mesh_prop)
	//		m_mesh_prop->Show(false);
	//	if (m_mesh_manip)
	//		m_mesh_manip->Show(false);
	//	if (m_annotation_prop)
	//		m_annotation_prop->Show(false);
	//	m_aui_mgr.GetPane(m_prop_panel).Caption(UITEXT_PROPERTIES);
	//	m_aui_mgr.Update();
	//	break;
	//case 6:  //mesh manip
	//	if (md)
	//	{
	//		m_mesh_manip->SetMeshData(md);
	//		m_mesh_manip->GetData();
	//		if (!m_mesh_manip->IsShown())
	//		{
	//			m_mesh_manip->Show(true);
	//			m_prop_sizer->Clear();
	//			m_prop_sizer->Add(m_mesh_manip, 1, wxEXPAND, 0);
	//			m_prop_panel->SetSizer(m_prop_sizer);
	//			m_prop_panel->Layout();
	//		}
	//		m_aui_mgr.GetPane(m_prop_panel).Caption(
	//			wxString("Manipulations - ")+md->GetName());
	//		m_aui_mgr.Update();
	//	}

	//	if (m_volume_prop)
	//		m_volume_prop->Show(false);
	//	if (m_mesh_prop)
	//		m_mesh_prop->Show(false);
	//	if (m_mesh_manip && md)
	//		m_mesh_manip->Show(true);
	//	if (m_annotation_prop)
	//		m_annotation_prop->Show(false);
	//	break;
	//default:
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
		m_aui_mgr.GetPane(m_prop_panel).Caption(UITEXT_PROPERTIES);
		m_aui_mgr.Update();
	}

	RefreshVRenderViews();
}

void RenderFrame::RefreshVRenderViews(bool tree, bool interactive)
{
	//for (int i=0 ; i<(int)m_vrv_list.size() ; i++)
	//{
	//	if (m_vrv_list[i])
	//		m_vrv_list[i]->RefreshGL(interactive);
	//}

	//incase volume color changes
	//change icon color of the tree panel
	//if (tree)
	//	UpdateTreeColors();
}

void RenderFrame::DeleteVRenderView(int i)
{
	//if (m_vrv_list[i])
	//{
	//	int j;
	//	wxString str = m_vrv_list[i]->GetName();

	//	for (j=0 ; j<GetView(i)->GetAllVolumeNum() ; j++)
	//		GetView(i)->GetAllVolumeData(j)->setValue(gstDisplay, true);
	//	for (j=0 ; j< GetView(i)->GetMeshNum() ; j++)
	//		GetView(i)->GetMeshData(j)->setValue(gstDisplay, true);
	//	RenderviewPanel* vrv = m_vrv_list[i];
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

void RenderFrame::DeleteVRenderView(const wxString &name)
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

//organize render views
void RenderFrame::OrganizeVRenderViews(int mode)
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
		RenderviewPanel* vrv = m_vrv_list[i];
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
		RenderviewPanel* vrv = m_vrv_list[i];
		if (vrv)
			m_aui_mgr.DetachPane(vrv);
	}
	//add back
	for (i=0; i<paneNum; i++)
	{
		RenderviewPanel* vrv = m_vrv_list[i];
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
void RenderFrame::ToggleAllTools(bool cur_state)
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

void RenderFrame::ShowPane(wxPanel* pane, bool show)
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

void RenderFrame::OnChEmbedCheck(wxCommandEvent &event)
{
	wxCheckBox* ch_embed = (wxCheckBox*)event.GetEventObject();
	if (ch_embed)
	{
		bool bval = ch_embed->GetValue();
		m_agent->setValue(gstEmbedDataInProject, bval);
	}
}

void RenderFrame::OnChSaveCmpCheck(wxCommandEvent &event)
{
	wxCheckBox* ch_cmp = (wxCheckBox*)event.GetEventObject();
	if (ch_cmp)
	{
		bool bval = ch_cmp->GetValue();
		m_agent->setValue(gstCaptureCompress, bval);
	}
}

wxWindow* RenderFrame::CreateExtraControlProjectSave(wxWindow* parent)
{
	fluo::RenderFrameAgent* agent = glbin_agtf->getRenderFrameAgent();
	bool bval;

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
		wxCommandEventHandler(RenderFrame::OnChEmbedCheck), NULL, panel);
	agent->getValue(gstEmbedDataInProject, bval);
	ch_embed->SetValue(bval);

	//compressed
	wxCheckBox* ch_cmp = new wxCheckBox(panel, ID_LZW_COMP,
		"Lempel-Ziv-Welch Compression");
	ch_cmp->Connect(ch_cmp->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(RenderFrame::OnChSaveCmpCheck), NULL, panel);
	agent->getValue(gstCaptureCompress, bval);
	ch_cmp->SetValue(bval);

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

void RenderFrame::OnSaveProject(wxCommandEvent& WXUNUSED(event))
{
	wxFileDialog *fopendlg = new wxFileDialog(
		this, "Save Project File",
		"", "", "*.vrp", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
	fopendlg->SetExtraControlCreator(CreateExtraControlProjectSave);

	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		wxString filename = fopendlg->GetPath();
		m_agent->SaveProject(filename.ToStdWstring());
	}

	delete fopendlg;
}

void RenderFrame::OnOpenProject(wxCommandEvent& WXUNUSED(event))
{
	wxFileDialog *fopendlg = new wxFileDialog(
		this, "Choose Project File",
		"", "", "*.vrp", wxFD_OPEN);

	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		wxString path = fopendlg->GetPath();
		m_agent->OpenProject(path.ToStdWstring());
	}

	delete fopendlg;
}

void RenderFrame::OnSettings(wxCommandEvent& WXUNUSED(event))
{
	fluo::SettingAgent* agent = glbin_agtf->getSettingAgent();
	if (agent) agent->UpdateAllSettings();
	m_aui_mgr.GetPane(m_setting_dlg).Show();
	m_aui_mgr.GetPane(m_setting_dlg).Float();
	m_aui_mgr.Update();
}

//tools
void RenderFrame::OnLastTool(wxCommandEvent& WXUNUSED(event))
{
	if (!m_setting_dlg)
	{
		ShowPaintTool();
		return;
	}

	long lval;
	m_agent->getValue(gstLastTool, lval);
	switch (lval)
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

void RenderFrame::OnPaintTool(wxCommandEvent& WXUNUSED(event))
{
	ShowPaintTool();
}

void RenderFrame::OnMeasure(wxCommandEvent& WXUNUSED(event))
{
	ShowMeasureDlg();
}

void RenderFrame::OnNoiseCancelling(wxCommandEvent& WXUNUSED(event))
{
	ShowNoiseCancellingDlg();
}

void RenderFrame::OnCounting(wxCommandEvent& WXUNUSED(event))
{
	ShowCountingDlg();
}

void RenderFrame::OnColocalization(wxCommandEvent& WXUNUSED(event))
{
	ShowColocalizationDlg();
}

void RenderFrame::OnConvert(wxCommandEvent& WXUNUSED(event))
{
	ShowConvertDlg();
}

void RenderFrame::OnTrace(wxCommandEvent& WXUNUSED(event))
{
	ShowTraceDlg();
}

void RenderFrame::OnOcl(wxCommandEvent& WXUNUSED(event))
{
	ShowOclDlg();
}

void RenderFrame::OnComponent(wxCommandEvent& WXUNUSED(event))
{
	ShowComponentDlg();
}

void RenderFrame::OnCalculations(wxCommandEvent& WXUNUSED(event))
{
	ShowCalculationDlg();
}

void RenderFrame::ShowPaintTool()
{
	m_aui_mgr.GetPane(m_brush_tool_dlg).Show();
	m_aui_mgr.GetPane(m_brush_tool_dlg).Float();
	m_aui_mgr.Update();
	m_main_tb->SetToolNormalBitmap(ID_LastTool,
		wxGetBitmapFromMemory(icon_paint_brush));
	m_agent->setValue(gstLastTool, long(TOOL_PAINT_BRUSH));
}

void RenderFrame::ShowMeasureDlg()
{
	m_aui_mgr.GetPane(m_measure_dlg).Show();
	m_aui_mgr.GetPane(m_measure_dlg).Float();
	m_aui_mgr.Update();
	m_main_tb->SetToolNormalBitmap(ID_LastTool,
		wxGetBitmapFromMemory(icon_measurement));
	m_agent->setValue(gstLastTool, long(TOOL_MEASUREMENT));
}

void RenderFrame::ShowTraceDlg()
{
	m_aui_mgr.GetPane(m_trace_dlg).Show();
	m_aui_mgr.GetPane(m_trace_dlg).Float();
	m_aui_mgr.Update();
	m_main_tb->SetToolNormalBitmap(ID_LastTool,
		wxGetBitmapFromMemory(icon_tracking));
	m_agent->setValue(gstLastTool, long(TOOL_TRACKING));
}

void RenderFrame::ShowNoiseCancellingDlg()
{
	m_aui_mgr.GetPane(m_noise_cancelling_dlg).Show();
	m_aui_mgr.GetPane(m_noise_cancelling_dlg).Float();
	m_aui_mgr.Update();
	m_main_tb->SetToolNormalBitmap(ID_LastTool,
		wxGetBitmapFromMemory(icon_noise_reduc));
	m_agent->setValue(gstLastTool, long(TOOL_NOISE_REDUCTION));
}

void RenderFrame::ShowCountingDlg()
{
	m_aui_mgr.GetPane(m_counting_dlg).Show();
	m_aui_mgr.GetPane(m_counting_dlg).Float();
	m_aui_mgr.Update();
	m_main_tb->SetToolNormalBitmap(ID_LastTool,
		wxGetBitmapFromMemory(icon_volume_size));
	m_agent->setValue(gstLastTool, long(TOOL_VOLUME_SIZE));
}

void RenderFrame::ShowColocalizationDlg()
{
	m_aui_mgr.GetPane(m_colocalization_dlg).Show();
	m_aui_mgr.GetPane(m_colocalization_dlg).Float();
	m_aui_mgr.Update();
	m_main_tb->SetToolNormalBitmap(ID_LastTool,
		wxGetBitmapFromMemory(icon_colocalization));
	m_agent->setValue(gstLastTool, long(TOOL_COLOCALIZATION));
}

void RenderFrame::ShowConvertDlg()
{
	m_aui_mgr.GetPane(m_convert_dlg).Show();
	m_aui_mgr.GetPane(m_convert_dlg).Float();
	m_aui_mgr.Update();
	m_main_tb->SetToolNormalBitmap(ID_LastTool,
		wxGetBitmapFromMemory(icon_convert));
	m_agent->setValue(gstLastTool, long(TOOL_CONVERT));
}

void RenderFrame::ShowOclDlg()
{
	m_aui_mgr.GetPane(m_ocl_dlg).Show();
	m_aui_mgr.GetPane(m_ocl_dlg).Float();
	m_aui_mgr.Update();
	m_main_tb->SetToolNormalBitmap(ID_LastTool,
		wxGetBitmapFromMemory(icon_opencl));
	m_agent->setValue(gstLastTool, long(TOOL_OPENCL));
}

void RenderFrame::ShowComponentDlg()
{
	m_aui_mgr.GetPane(m_component_dlg).Show();
	m_aui_mgr.GetPane(m_component_dlg).Float();
	m_aui_mgr.Update();
	m_main_tb->SetToolNormalBitmap(ID_LastTool,
		wxGetBitmapFromMemory(icon_components));
	m_agent->setValue(gstLastTool, long(TOOL_COMPONENT));
}

void RenderFrame::ShowCalculationDlg()
{
	m_aui_mgr.GetPane(m_calculation_dlg).Show();
	m_aui_mgr.GetPane(m_calculation_dlg).Float();
	m_aui_mgr.Update();
	m_main_tb->SetToolNormalBitmap(ID_LastTool,
		wxGetBitmapFromMemory(icon_calculations));
	m_agent->setValue(gstLastTool, long(TOOL_CALCULATIONS));
}

void RenderFrame::OnFacebook(wxCommandEvent& WXUNUSED(event))
{
	::wxLaunchDefaultBrowser(FACEBOOK_URL);
}

void RenderFrame::OnManual(wxCommandEvent& WXUNUSED(event))
{
	::wxLaunchDefaultBrowser(HELP_MANUAL);
}


void RenderFrame::OnTutorial(wxCommandEvent& WXUNUSED(event))
{
	::wxLaunchDefaultBrowser(HELP_TUTORIAL);
}

void RenderFrame::OnTwitter(wxCommandEvent& WXUNUSED(event))
{
	::wxLaunchDefaultBrowser(TWITTER_URL);
}

void RenderFrame::OnYoutube(wxCommandEvent& WXUNUSED(event))
{
	::wxLaunchDefaultBrowser(YOUTUBE_URL);
}

void RenderFrame::OnShowHideUI(wxCommandEvent& WXUNUSED(event))
{
	ToggleAllTools(true);
}

void RenderFrame::OnShowHideToolbar(wxCommandEvent& WXUNUSED(event))
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

void RenderFrame::OnShowHideView(wxCommandEvent &event)
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
void RenderFrame::OnPaneClose(wxAuiManagerEvent& event)
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
	else if (name == "OutAdjustPanel")
		m_tb_menu_ui->Check(ID_UIAdjView, false);
	else if (name == "ClipPlanePanel")
		m_tb_menu_ui->Check(ID_UIClipView, false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void RenderFrame::OnCreateCube(wxCommandEvent& WXUNUSED(event))
{
}

void RenderFrame::OnCreateSphere(wxCommandEvent& WXUNUSED(event))
{
}

void RenderFrame::OnCreateCone(wxCommandEvent& WXUNUSED(event))
{
}

void RenderFrame::OnDraw(wxPaintEvent& event)
{
	//wxPaintDC dc(this);

	//wxRect windowRect(wxPoint(0, 0), GetClientSize());
	//dc.SetBrush(wxBrush(*wxGREEN, wxSOLID));
	//dc.DrawRectangle(windowRect);
}

void RenderFrame::OnKeyDown(wxKeyEvent& event)
{
	event.Skip();
}

