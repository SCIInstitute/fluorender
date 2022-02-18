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
#include "ComponentDlg.h"
#include "VRenderFrame.h"
#include <Renderview.hpp>
#include <VolumeData.hpp>
#include <VolumeGroup.hpp>
#include <Annotations.hpp>
#include <Global.hpp>
#include <AgentFactory.hpp>
#include <AnnotationFactory.hpp>
#include <Components/CompSelector.h>
#include <Components/CompEditor.h>
#include <Cluster/dbscan.h>
#include <Cluster/kmeans.h>
#include <Cluster/exmax.h>
#include <FLIVR/Texture.h>
#include <FLIVR/VolumeRenderer.h>
#include <wx/valnum.h>
#include <wx/stdpaths.h>
#include <boost/signals2.hpp>
#include <limits>
#include <string>
#include <cctype>
#include <fstream>

BEGIN_EVENT_TABLE(ComponentDlg, wxPanel)
	//EVT_COLLAPSIBLEPANE_CHANGED(wxID_ANY, ComponentDlg::OnPaneChange)
	//load save settings
	EVT_BUTTON(ID_LoadSettingsBtn, ComponentDlg::OnLoadSettings)
	EVT_BUTTON(ID_SaveSettingsBtn, ComponentDlg::OnSaveSettings)
	EVT_BUTTON(ID_SaveasSettingsBtn, ComponentDlg::OnSaveasSettings)
	//comp gen page
	EVT_COMMAND_SCROLL(ID_IterSldr, ComponentDlg::OnIterSldr)
	EVT_TEXT(ID_IterText, ComponentDlg::OnIterText)
	EVT_COMMAND_SCROLL(ID_ThreshSldr, ComponentDlg::OnThreshSldr)
	EVT_TEXT(ID_ThreshText, ComponentDlg::OnThreshText)
	//distance field
	EVT_CHECKBOX(ID_UseDistFieldCheck, ComponentDlg::OnUseDistFieldCheck)
	EVT_COMMAND_SCROLL(ID_DistStrengthSldr, ComponentDlg::OnDistStrengthSldr)
	EVT_TEXT(ID_DistStrengthText, ComponentDlg::OnDistStrengthText)
	EVT_COMMAND_SCROLL(ID_DistFilterSizeSldr, ComponentDlg::OnDistFilterSizeSldr)
	EVT_TEXT(ID_DistFilterSizeText, ComponentDlg::OnDistFitlerSizeText)
	EVT_COMMAND_SCROLL(ID_MaxDistSldr, ComponentDlg::OnMaxDistSldr)
	EVT_TEXT(ID_MaxDistText, ComponentDlg::OnMaxDistText)
	EVT_COMMAND_SCROLL(ID_DistThreshSldr, ComponentDlg::OnDistThreshSldr)
	EVT_TEXT(ID_DistThreshText, ComponentDlg::OnDistThreshText)
	//diffusion
	EVT_CHECKBOX(ID_DiffCheck, ComponentDlg::OnDiffCheck)
	EVT_COMMAND_SCROLL(ID_FalloffSldr, ComponentDlg::OnFalloffSldr)
	EVT_TEXT(ID_FalloffText, ComponentDlg::OnFalloffText)
	EVT_CHECKBOX(ID_SizeCheck, ComponentDlg::OnSizeCheck)
	EVT_COMMAND_SCROLL(ID_SizeSldr, ComponentDlg::OnSizeSldr)
	EVT_TEXT(ID_SizeText, ComponentDlg::OnSizeText)
	//density
	EVT_CHECKBOX(ID_DensityCheck, ComponentDlg::OnDensityCheck)
	EVT_COMMAND_SCROLL(ID_DensitySldr, ComponentDlg::OnDensitySldr)
	EVT_TEXT(ID_DensityText, ComponentDlg::OnDensityText)
	EVT_COMMAND_SCROLL(ID_VarthSldr, ComponentDlg::OnVarthSldr)
	EVT_TEXT(ID_VarthText, ComponentDlg::OnVarthText)
	EVT_COMMAND_SCROLL(ID_DensityWindowSizeSldr, ComponentDlg::OnDensityWindowSizeSldr)
	EVT_TEXT(ID_DensityWindowsSizeText, ComponentDlg::OnDensityWindowSizeText)
	EVT_COMMAND_SCROLL(ID_DensityStatsSizeSldr, ComponentDlg::OnDensityStatsSizeSldr)
	EVT_TEXT(ID_DensityStatsSizeText, ComponentDlg::OnDensityStatsSizeText)
	//fixate
	EVT_CHECKBOX(ID_FixateCheck, ComponentDlg::OnFixateCheck)
	EVT_BUTTON(ID_FixUpdateBtn, ComponentDlg::OnFixUpdateBtn)
	EVT_COMMAND_SCROLL(ID_FixSizeSldr, ComponentDlg::OnFixSizeSldr)
	EVT_TEXT(ID_FixSizeText, ComponentDlg::OnFixSizeText)
	//clean
	EVT_CHECKBOX(ID_CleanCheck, ComponentDlg::OnCleanCheck)
	EVT_BUTTON(ID_CleanBtn, ComponentDlg::OnCleanBtn)
	EVT_COMMAND_SCROLL(ID_CleanIterSldr, ComponentDlg::OnCleanIterSldr)
	EVT_TEXT(ID_CleanIterText, ComponentDlg::OnCleanIterText)
	EVT_COMMAND_SCROLL(ID_CleanLimitSldr, ComponentDlg::OnCleanLimitSldr)
	EVT_TEXT(ID_CleanLimitText, ComponentDlg::OnCleanLimitText)
	//record
	EVT_TOGGLEBUTTON(ID_RecordCmdBtn, ComponentDlg::OnRecordCmd)
	EVT_BUTTON(ID_PlayCmdBtn, ComponentDlg::OnPlayCmd)
	EVT_BUTTON(ID_ResetCmdBtn, ComponentDlg::OnResetCmd)
	EVT_BUTTON(ID_SaveCmdBtn, ComponentDlg::OnSaveCmd)
	EVT_BUTTON(ID_LoadCmdBtn, ComponentDlg::OnLoadCmd)

	//clustering page
	EVT_RADIOBUTTON(ID_ClusterMethodExmaxRd, ComponentDlg::OnClusterMethodExmaxCheck)
	EVT_RADIOBUTTON(ID_ClusterMethodDbscanRd, ComponentDlg::OnClusterMethodDbscanCheck)
	EVT_RADIOBUTTON(ID_ClusterMethodKmeansRd, ComponentDlg::OnClusterMethodKmeansCheck)
	//parameters
	EVT_COMMAND_SCROLL(ID_ClusterClnumSldr, ComponentDlg::OnClusterClnumSldr)
	EVT_TEXT(ID_ClusterClnumText, ComponentDlg::OnClusterClnumText)
	EVT_COMMAND_SCROLL(ID_ClusterMaxIterSldr, ComponentDlg::OnClusterMaxiterSldr)
	EVT_TEXT(ID_ClusterMaxIterText, ComponentDlg::OnClusterMaxiterText)
	EVT_COMMAND_SCROLL(ID_ClusterTolSldr, ComponentDlg::OnClusterTolSldr)
	EVT_TEXT(ID_ClusterTolText, ComponentDlg::OnClusterTolText)
	EVT_COMMAND_SCROLL(ID_ClusterSizeSldr, ComponentDlg::OnClusterSizeSldr)
	EVT_TEXT(ID_ClusterSizeText, ComponentDlg::OnClusterSizeText)
	EVT_COMMAND_SCROLL(ID_ClusterEpsSldr, ComponentDlg::OnClusterEpsSldr)
	EVT_TEXT(ID_ClusterEpsText, ComponentDlg::OnClusterepsText)

	//analysis page
	EVT_TEXT(ID_CompIdText, ComponentDlg::OnCompIdText)
	EVT_TEXT_ENTER(ID_CompIdText, ComponentDlg::OnCompFull)
	EVT_BUTTON(ID_CompIdXBtn, ComponentDlg::OnCompIdXBtn)
	EVT_CHECKBOX(ID_AnalysisMinCheck, ComponentDlg::OnAnalysisMinCheck)
	EVT_SPINCTRL(ID_AnalysisMinSpin, ComponentDlg::OnAnalysisMinSpin)
	EVT_TEXT(ID_AnalysisMinSpin, ComponentDlg::OnAnalysisMinText)
	EVT_CHECKBOX(ID_AnalysisMaxCheck, ComponentDlg::OnAnalysisMaxCheck)
	EVT_SPINCTRL(ID_AnalysisMaxSpin, ComponentDlg::OnAnalysisMaxSpin)
	EVT_TEXT(ID_AnalysisMaxSpin, ComponentDlg::OnAnalysisMaxText)
	EVT_BUTTON(ID_CompFullBtn, ComponentDlg::OnCompFull)
	EVT_BUTTON(ID_CompExclusiveBtn, ComponentDlg::OnCompExclusive)
	EVT_BUTTON(ID_CompAppendBtn, ComponentDlg::OnCompAppend)
	EVT_BUTTON(ID_CompAllBtn, ComponentDlg::OnCompAll)
	EVT_BUTTON(ID_CompClearBtn, ComponentDlg::OnCompClear)
	EVT_BUTTON(ID_ShuffleBtn, ComponentDlg::OnShuffle)
	//modify
	EVT_TEXT(ID_NewIdText, ComponentDlg::OnNewIDText)
	EVT_BUTTON(ID_NewIdXBtn, ComponentDlg::OnNewIDX)
	EVT_BUTTON(ID_CompNewBtn, ComponentDlg::OnCompNew)
	EVT_BUTTON(ID_CompAddBtn, ComponentDlg::OnCompAdd)
	EVT_BUTTON(ID_CompReplaceBtn, ComponentDlg::OnCompReplace)
	EVT_BUTTON(ID_CompCleanBkgBtn, ComponentDlg::OnCompCleanBkg)
	EVT_BUTTON(ID_CompCombineBtn, ComponentDlg::OnCompCombine)
	//options
	EVT_COMMAND_SCROLL(ID_ConSizeSldr, ComponentDlg::OnConSizeSldr)
	EVT_TEXT(ID_ConSizeText, ComponentDlg::OnConSizeText)
	EVT_CHECKBOX(ID_ConsistentCheck, ComponentDlg::OnConsistentCheck)
	EVT_CHECKBOX(ID_ColocalCheck, ComponentDlg::OnColocalCheck)
	//output
	EVT_RADIOBUTTON(ID_OutputMultiRb, ComponentDlg::OnOutputTypeRadio)
	EVT_RADIOBUTTON(ID_OutputRgbRb, ComponentDlg::OnOutputTypeRadio)
	EVT_BUTTON(ID_OutputRandomBtn, ComponentDlg::OnOutputChannels)
	EVT_BUTTON(ID_OutputSizeBtn, ComponentDlg::OnOutputChannels)
	EVT_BUTTON(ID_OutputIdBtn, ComponentDlg::OnOutputAnnotation)
	EVT_BUTTON(ID_OutputSnBtn, ComponentDlg::OnOutputAnnotation)
	//distance
	EVT_CHECKBOX(ID_DistNeighborCheck, ComponentDlg::OnDistNeighborCheck)
	EVT_CHECKBOX(ID_DistAllChanCheck, ComponentDlg::OnDistAllChanCheck)
	EVT_COMMAND_SCROLL(ID_DistNeighborSldr, ComponentDlg::OnDistNeighborSldr)
	EVT_TEXT(ID_DistNeighborText, ComponentDlg::OnDistNeighborText)
	EVT_BUTTON(ID_DistOutputBtn, ComponentDlg::OnDistOutput)
	//align
	EVT_BUTTON(ID_AlignXYZ, ComponentDlg::OnAlignPca)
	EVT_BUTTON(ID_AlignYXZ, ComponentDlg::OnAlignPca)
	EVT_BUTTON(ID_AlignZXY, ComponentDlg::OnAlignPca)
	EVT_BUTTON(ID_AlignXZY, ComponentDlg::OnAlignPca)
	EVT_BUTTON(ID_AlignYZX, ComponentDlg::OnAlignPca)
	EVT_BUTTON(ID_AlignZYX, ComponentDlg::OnAlignPca)

	//execute
	EVT_NOTEBOOK_PAGE_CHANGED(ID_Notebook, ComponentDlg::OnNotebook)
	EVT_CHECKBOX(ID_UseSelChk, ComponentDlg::OnUseSelChk)
	EVT_BUTTON(ID_GenerateBtn, ComponentDlg::OnGenerate)
	EVT_TOGGLEBUTTON(ID_AutoUpdateBtn, ComponentDlg::OnAutoUpdate)
	EVT_BUTTON(ID_ClusterBtn, ComponentDlg::OnCluster)
	EVT_BUTTON(ID_AnalyzeBtn, ComponentDlg::OnAnalyze)
	EVT_BUTTON(ID_AnalyzeSelBtn, ComponentDlg::OnAnalyzeSel)
	//output
	EVT_BUTTON(ID_IncludeBtn, ComponentDlg::OnIncludeBtn)
	EVT_BUTTON(ID_ExcludeBtn, ComponentDlg::OnExcludeBtn)
	EVT_CHECKBOX(ID_HistoryChk, ComponentDlg::OnHistoryChk)
	EVT_BUTTON(ID_ClearHistBtn, ComponentDlg::OnClearHistBtn)
	//EVT_KEY_DOWN(ComponentDlg::OnKeyDown)
	EVT_GRID_SELECT_CELL(ComponentDlg::OnSelectCell)
	EVT_GRID_RANGE_SELECT(ComponentDlg::OnRangeSelect)
	EVT_GRID_LABEL_LEFT_CLICK(ComponentDlg::OnGridLabelClick)
	//split
	EVT_SPLITTER_DCLICK(wxID_ANY, ComponentDlg::OnSplitterDclick)
END_EVENT_TABLE()

ComponentDlg::ComponentDlg(VRenderFrame *frame)
	: wxPanel(frame, wxID_ANY,
		wxDefaultPosition,
		wxSize(600, 800),
		0, "ComponentDlg"),
	m_frame(frame),
	m_view(0),
	m_hold_history(false),
	m_test_speed(false),
	m_cell_new_id(0),
	m_cell_new_id_empty(true)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);
	SetMinSize(wxSize(100, 100));

	wxBoxSizer *mainsizer = new wxBoxSizer(wxHORIZONTAL);
	wxSplitterWindow *splittermain = new wxSplitterWindow(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxSP_THIN_SASH | wxSP_BORDER | wxSP_LIVE_UPDATE);
	splittermain->SetMinimumPaneSize(160);
	mainsizer->Add(splittermain, 1, wxBOTTOM | wxLEFT | wxEXPAND, 5);

	panel_top = new wxPanel(splittermain, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER);
	wxBoxSizer* sizerT = new wxBoxSizer(wxHORIZONTAL);
	//notebook
	m_notebook = new wxNotebook(panel_top, ID_Notebook);
	m_notebook->AddPage(CreateCompGenPage(m_notebook), "Generate");
	m_notebook->AddPage(CreateClusteringPage(m_notebook), "Cluster");
	m_notebook->AddPage(CreateAnalysisPage(m_notebook), "Analysis");
	sizerT->Add(m_notebook, 1, wxALL | wxEXPAND, 5);
	panel_top->SetSizer(sizerT);

	panel_bot = new wxPanel(splittermain, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER);
	wxBoxSizer* sizer1 = new wxBoxSizer(wxHORIZONTAL);
	m_shuffle_btn = new wxButton(panel_bot, ID_ShuffleBtn, "Shuffle",
		wxDefaultPosition, wxDefaultSize);
	m_use_sel_chk = new wxCheckBox(panel_bot, ID_UseSelChk, "Use Sel.",
		wxDefaultPosition, wxDefaultSize);
	m_generate_btn = new wxButton(panel_bot, ID_GenerateBtn, "Generate",
		wxDefaultPosition, wxSize(75, -1));
	m_auto_update_btn = new wxToggleButton(panel_bot, ID_AutoUpdateBtn, "Auto Update",
		wxDefaultPosition, wxSize(75, -1));
	m_cluster_btn = new wxButton(panel_bot, ID_ClusterBtn, "Cluster",
		wxDefaultPosition, wxSize(75, -1));
	m_analyze_btn = new wxButton(panel_bot, ID_AnalyzeBtn, "Analyze",
		wxDefaultPosition, wxSize(75, -1));
	m_analyze_sel_btn = new wxButton(panel_bot, ID_AnalyzeSelBtn, "Anlyz. Sel.",
		wxDefaultPosition, wxSize(75, -1));
	sizer1->Add(m_shuffle_btn, 0, wxALIGN_CENTER);
	sizer1->AddStretchSpacer();
	sizer1->Add(m_use_sel_chk, 0, wxALIGN_CENTER);
	sizer1->Add(m_generate_btn, 0, wxALIGN_CENTER);
	sizer1->Add(m_auto_update_btn, 0, wxALIGN_CENTER);
	sizer1->Add(m_cluster_btn, 0, wxALIGN_CENTER);
	sizer1->Add(m_analyze_btn, 0, wxALIGN_CENTER);
	sizer1->Add(m_analyze_sel_btn, 0, wxALIGN_CENTER);
	sizer1->Add(10, 10);

	//stats text
	wxBoxSizer *sizer2 = new wxStaticBoxSizer(
		new wxStaticBox(panel_bot, wxID_ANY, "Output"),
		wxVERTICAL);
	wxBoxSizer *sizer2_1 = new wxBoxSizer(wxHORIZONTAL);
	m_include_btn = new wxButton(panel_bot, ID_IncludeBtn,
		"Include", wxDefaultPosition, wxSize(75, -1));
	m_exclude_btn = new wxButton(panel_bot, ID_ExcludeBtn,
		"Exclude", wxDefaultPosition, wxSize(75, -1));
	m_history_chk = new wxCheckBox(panel_bot, ID_HistoryChk,
		"Hold History", wxDefaultPosition, wxSize(85, 20), wxALIGN_LEFT);
	m_clear_hist_btn = new wxButton(panel_bot, ID_ClearHistBtn,
		"Clear History", wxDefaultPosition, wxSize(75, -1));
	sizer2_1->Add(m_include_btn, 0, wxALIGN_CENTER);
	sizer2_1->Add(m_exclude_btn, 0, wxALIGN_CENTER);
	sizer2_1->AddStretchSpacer(1);
	sizer2_1->Add(m_history_chk, 0, wxALIGN_CENTER);
	sizer2_1->Add(5, 5);
	sizer2_1->Add(m_clear_hist_btn, 0, wxALIGN_CENTER);
	//grid
	m_output_grid = new wxGrid(panel_bot, ID_OutputGrid);
	m_output_grid->CreateGrid(0, 1);
	m_output_grid->Fit();
	m_output_grid->Connect(wxID_ANY, wxEVT_KEY_DOWN,
		wxKeyEventHandler(ComponentDlg::OnKeyDown), NULL, this);
	sizer2->Add(5, 5);
	sizer2->Add(sizer2_1, 0, wxEXPAND);
	sizer2->Add(5, 5);
	sizer2->Add(m_output_grid, 1, wxEXPAND);
	sizer2->Add(5, 5);

	//all controls
	wxBoxSizer* sizerB = new wxBoxSizer(wxVERTICAL);
	sizerB->Add(10, 10);
	sizerB->Add(sizer1, 0, wxEXPAND);
	sizerB->Add(10, 10);
	sizerB->Add(sizer2, 1, wxEXPAND);
	sizerB->Add(10, 10);
	panel_bot->SetSizer(sizerB);

	splittermain->SetSashGravity(0.0);
	splittermain->SplitHorizontally(panel_top, panel_bot, 500);

	SetSizer(mainsizer);
	panel_top->Layout();
	panel_bot->Layout();

	GetSettings();
}

ComponentDlg::~ComponentDlg()
{
	SaveSettings("");
}

wxWindow* ComponentDlg::CreateCompGenPage(wxWindow *parent)
{
	wxScrolledWindow *page = new wxScrolledWindow(parent);
	wxStaticText *st = 0;
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;
	//validator: floating point 3
	wxFloatingPointValidator<double> vald_fp3(3);

	wxBoxSizer* sizer1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Iterations:",
		wxDefaultPosition, wxSize(100, 23));
	m_iter_sldr = new wxSlider(page, ID_IterSldr, 0, 0, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_iter_text = new wxTextCtrl(page, ID_IterText, "0",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	sizer1->Add(2, 2);
	sizer1->Add(st, 0, wxALIGN_CENTER);
	sizer1->Add(m_iter_sldr, 1, wxEXPAND);
	sizer1->Add(m_iter_text, 0, wxALIGN_CENTER);
	sizer1->Add(2, 2);

	wxBoxSizer* sizer2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Threshold:",
		wxDefaultPosition, wxSize(100, 23));
	m_thresh_sldr = new wxSlider(page, ID_ThreshSldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_thresh_text = new wxTextCtrl(page, ID_ThreshText, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	sizer2->Add(2, 2);
	sizer2->Add(st, 0, wxALIGN_CENTER);
	sizer2->Add(m_thresh_sldr, 1, wxEXPAND);
	sizer2->Add(m_thresh_text, 0, wxALIGN_CENTER);
	sizer2->Add(2, 2);

	wxBoxSizer* sizer3 = new wxBoxSizer(wxHORIZONTAL);
	m_diff_check = new wxCheckBox(page, ID_DiffCheck, "Enable Diffusion",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	sizer3->Add(2, 2);
	sizer3->Add(m_diff_check, 0, wxALIGN_CENTER);

	wxBoxSizer* sizer4 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Smoothness:",
		wxDefaultPosition, wxSize(100, 23));
	m_falloff_sldr = new wxSlider(page, ID_FalloffSldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_falloff_text = new wxTextCtrl(page, ID_FalloffText, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	sizer4->Add(2, 2);
	sizer4->Add(st, 0, wxALIGN_CENTER);
	sizer4->Add(m_falloff_sldr, 1, wxEXPAND);
	sizer4->Add(m_falloff_text, 0, wxALIGN_CENTER);
	sizer4->Add(2, 2);

	//size not used
	//m_size_check = new wxCheckBox(page, ID_SizeCheck, "Enable Size Limiter",
	//	wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	//m_size_check->Hide();
	//m_size_sldr = new wxSlider(page, ID_SizeSldr, 100, 0, 500,
	//	wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	//m_size_text = new wxTextCtrl(page, ID_SizeText, "100",
	//	wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	//m_size_sldr->Hide();
	//m_size_text->Hide();

	//density
	wxBoxSizer* sizer5 = new wxBoxSizer(wxHORIZONTAL);
	m_density_check = new wxCheckBox(page, ID_DensityCheck, "Use Density Field",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	sizer5->Add(2, 2);
	sizer5->Add(m_density_check, 0, wxALIGN_CENTER);
	//
	wxBoxSizer* sizer6 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Separation:",
		wxDefaultPosition, wxSize(100, 23));
	m_density_sldr = new wxSlider(page, ID_DensitySldr, 1000, 0, 10000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_density_text = new wxTextCtrl(page, ID_DensityText, "1.0",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	sizer6->Add(2, 2);
	sizer6->Add(st, 0, wxALIGN_CENTER);
	sizer6->Add(m_density_sldr, 1, wxEXPAND);
	sizer6->Add(m_density_text, 0, wxALIGN_CENTER);
	sizer6->Add(2, 2);
	//
	wxBoxSizer* sizer61 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Noise Level:",
		wxDefaultPosition, wxSize(100, 23));
	m_varth_sldr = new wxSlider(page, ID_VarthSldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_varth_text = new wxTextCtrl(page, ID_VarthText, "0.0",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	sizer61->Add(2, 2);
	sizer61->Add(st, 0, wxALIGN_CENTER);
	sizer61->Add(m_varth_sldr, 1, wxEXPAND);
	sizer61->Add(m_varth_text, 0, wxALIGN_CENTER);
	sizer61->Add(2, 2);

	wxBoxSizer* sizer7 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Filter Size:",
		wxDefaultPosition, wxSize(100, 23));
	m_density_window_size_sldr = new wxSlider(page, ID_DensityWindowSizeSldr, 5, 1, 20,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_density_window_size_text = new wxTextCtrl(page, ID_DensityWindowsSizeText, "5",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	sizer7->Add(2, 2);
	sizer7->Add(st, 0, wxALIGN_CENTER);
	sizer7->Add(m_density_window_size_sldr, 1, wxEXPAND);
	sizer7->Add(m_density_window_size_text, 0, wxALIGN_CENTER);
	sizer7->Add(2, 2);

	wxBoxSizer* sizer8 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Feature Size:",
		wxDefaultPosition, wxSize(100, 23));
	m_density_stats_size_sldr = new wxSlider(page, ID_DensityStatsSizeSldr, 15, 1, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_density_stats_size_text = new wxTextCtrl(page, ID_DensityStatsSizeText, "15",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	sizer8->Add(2, 2);
	sizer8->Add(st, 0, wxALIGN_CENTER);
	sizer8->Add(m_density_stats_size_sldr, 1, wxEXPAND);
	sizer8->Add(m_density_stats_size_text, 0, wxALIGN_CENTER);
	sizer8->Add(2, 2);

	//distance field
	wxBoxSizer* sizer9 = new wxBoxSizer(wxHORIZONTAL);
	m_use_dist_field_check = new wxCheckBox(page, ID_UseDistFieldCheck, "Use Distance Field",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	sizer9->Add(2, 2);
	sizer9->Add(m_use_dist_field_check, 0, wxALIGN_CENTER);
	wxBoxSizer* sizer10 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Strength:",
		wxDefaultPosition, wxSize(100, 23));
	m_dist_strength_sldr = new wxSlider(page, ID_DistStrengthSldr, 500, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_dist_strength_text = new wxTextCtrl(page, ID_DistStrengthText, "0.5",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	sizer10->Add(2, 2);
	sizer10->Add(st, 0, wxALIGN_CENTER);
	sizer10->Add(m_dist_strength_sldr, 1, wxEXPAND);
	sizer10->Add(m_dist_strength_text, 0, wxALIGN_CENTER);
	sizer10->Add(2, 2);
	wxBoxSizer* sizer11 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Perimeter Value:",
		wxDefaultPosition, wxSize(100, 23));
	m_dist_thresh_sldr = new wxSlider(page, ID_DistThreshSldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_dist_thresh_text = new wxTextCtrl(page, ID_DistThreshText, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	sizer11->Add(2, 2);
	sizer11->Add(st, 0, wxALIGN_CENTER);
	sizer11->Add(m_dist_thresh_sldr, 1, wxEXPAND);
	sizer11->Add(m_dist_thresh_text, 0, wxALIGN_CENTER);
	sizer11->Add(2, 2);
	wxBoxSizer* sizer12 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Filter Size:",
		wxDefaultPosition, wxSize(100, 23));
	m_dist_filter_size_sldr = new wxSlider(page, ID_DistFilterSizeSldr, 3, 1, 20,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_dist_filter_size_text = new wxTextCtrl(page, ID_DistFilterSizeText, "3",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	sizer12->Add(2, 2);
	sizer12->Add(st, 0, wxALIGN_CENTER);
	sizer12->Add(m_dist_filter_size_sldr, 1, wxEXPAND);
	sizer12->Add(m_dist_filter_size_text, 0, wxALIGN_CENTER);
	sizer12->Add(2, 2);
	wxBoxSizer* sizer13 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Feature Size:",
		wxDefaultPosition, wxSize(100, 23));
	m_max_dist_sldr = new wxSlider(page, ID_MaxDistSldr, 30, 1, 255,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_max_dist_text = new wxTextCtrl(page, ID_MaxDistText, "30",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	sizer13->Add(2, 2);
	sizer13->Add(st, 0, wxALIGN_CENTER);
	sizer13->Add(m_max_dist_sldr, 1, wxEXPAND);
	sizer13->Add(m_max_dist_text, 0, wxALIGN_CENTER);
	sizer13->Add(2, 2);

	//fixate
	wxBoxSizer* sizer14 = new wxBoxSizer(wxHORIZONTAL);
	m_fixate_check = new wxCheckBox(page, ID_FixateCheck, "Fixate Grown Regions",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_fix_update_btn = new wxButton(page, ID_FixUpdateBtn, "Refix",
		wxDefaultPosition, wxSize(75, -1), wxALIGN_LEFT);
	sizer14->Add(2, 2);
	sizer14->Add(m_fixate_check, 0, wxALIGN_CENTER);
	sizer14->AddStretchSpacer(1);
	sizer14->Add(m_fix_update_btn, 0, wxALIGN_CENTER);
	sizer14->Add(2, 2);
	wxBoxSizer* sizer15 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Stop Size:",
		wxDefaultPosition, wxSize(100, 23));
	m_fix_size_sldr = new wxSlider(page, ID_FixSizeSldr, 50, 1, 200,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_fix_size_text = new wxTextCtrl(page, ID_FixSizeText, "50",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	sizer15->Add(2, 2);
	sizer15->Add(st, 0, wxALIGN_CENTER);
	sizer15->Add(m_fix_size_sldr, 1, wxEXPAND);
	sizer15->Add(m_fix_size_text, 0, wxALIGN_CENTER);
	sizer15->Add(2, 2);

	//clean
	wxBoxSizer* sizer16 = new wxBoxSizer(wxHORIZONTAL);
	m_clean_check = new wxCheckBox(page, ID_CleanCheck, "Clean Up",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_clean_btn = new wxButton(page, ID_CleanBtn, "Clean More",
		wxDefaultPosition, wxSize(75, -1), wxALIGN_LEFT);
	sizer16->Add(2, 2);
	sizer16->Add(m_clean_check, 0, wxALIGN_CENTER);
	sizer16->AddStretchSpacer(1);
	sizer16->Add(m_clean_btn, 0, wxALIGN_CENTER);
	sizer16->Add(2, 2);

	//iterations
	wxBoxSizer* sizer17 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Iterations:",
		wxDefaultPosition, wxSize(100, 23));
	m_clean_iter_sldr = new wxSlider(page, ID_CleanIterSldr, 5, 1, 50,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_clean_iter_text = new wxTextCtrl(page, ID_CleanIterText, "5",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	sizer17->Add(2, 2);
	sizer17->Add(st, 0, wxALIGN_CENTER);
	sizer17->Add(m_clean_iter_sldr, 1, wxEXPAND);
	sizer17->Add(m_clean_iter_text, 0, wxALIGN_CENTER);
	sizer17->Add(2, 2);
	//iterations
	wxBoxSizer* sizer18 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Stop Size:",
		wxDefaultPosition, wxSize(100, 23));
	m_clean_limit_sldr = new wxSlider(page, ID_CleanLimitSldr, 5, 1, 50,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_clean_limit_text = new wxTextCtrl(page, ID_CleanLimitText, "5",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	sizer18->Add(2, 2);
	sizer18->Add(st, 0, wxALIGN_CENTER);
	sizer18->Add(m_clean_limit_sldr, 1, wxEXPAND);
	sizer18->Add(m_clean_limit_text, 0, wxALIGN_CENTER);
	sizer18->Add(2, 2);

	//command record
	wxBoxSizer* sizer19 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Recorder:",
		wxDefaultPosition, wxSize(100, 23));
	m_cmd_count_text = new wxTextCtrl(page, ID_CmdCountText, "",
		wxDefaultPosition, wxSize(75, -1), wxTE_READONLY);
	m_record_cmd_btn = new wxToggleButton(page, ID_RecordCmdBtn, "Record",
		wxDefaultPosition, wxSize(75, -1));
	m_play_cmd_btn = new wxButton(page, ID_PlayCmdBtn, "Play",
		wxDefaultPosition, wxSize(75, -1));
	m_reset_cmd_btn = new wxButton(page, ID_ResetCmdBtn, "Reset",
		wxDefaultPosition, wxSize(75, -1));
	sizer19->Add(2, 2);
	sizer19->Add(st, 0, wxALIGN_CENTER);
	sizer19->AddStretchSpacer();
	sizer19->Add(m_cmd_count_text, 0, wxALIGN_CENTER);
	sizer19->Add(m_record_cmd_btn, 0, wxALIGN_CENTER);
	sizer19->Add(m_play_cmd_btn, 0, wxALIGN_CENTER);
	sizer19->Add(m_reset_cmd_btn, 0, wxALIGN_CENTER);
	sizer19->Add(2, 2);
	wxBoxSizer* sizer20 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Save File:",
		wxDefaultPosition, wxSize(100, 23));
	m_cmd_file_text = new wxTextCtrl(page, ID_CmdFileText, "",
		wxDefaultPosition, wxDefaultSize);
	m_load_cmd_btn = new wxButton(page, ID_LoadCmdBtn, "Load",
		wxDefaultPosition, wxSize(75, -1));
	m_save_cmd_btn = new wxButton(page, ID_SaveCmdBtn, "Save",
		wxDefaultPosition, wxSize(75, -1));
	sizer20->Add(2, 2);
	sizer20->Add(st, 0, wxALIGN_CENTER);
	sizer20->Add(m_cmd_file_text, 1, wxALIGN_CENTER);
	sizer20->Add(m_load_cmd_btn, 0, wxALIGN_CENTER);
	sizer20->Add(m_save_cmd_btn, 0, wxALIGN_CENTER);
	sizer20->Add(2, 2);

	wxBoxSizer *group1 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Region Growth && Merge"), wxVERTICAL);
	group1->Add(5, 5);
	group1->Add(sizer1, 0, wxEXPAND);
	group1->Add(5, 5);
	group1->Add(sizer2, 0, wxEXPAND);
	group1->Add(5, 5);
	group1->Add(sizer3, 0, wxEXPAND);
	group1->Add(5, 5);
	group1->Add(sizer4, 0, wxEXPAND);
	group1->Add(5, 5);
	group1->Add(sizer5, 0, wxEXPAND);
	group1->Add(5, 5);
	group1->Add(sizer6, 0, wxEXPAND);
	group1->Add(5, 5);
	group1->Add(sizer61, 0, wxEXPAND);
	group1->Add(5, 5);
	group1->Add(sizer7, 0, wxEXPAND);
	group1->Add(5, 5);
	group1->Add(sizer8, 0, wxEXPAND);
	group1->Add(5, 5);
	group1->Add(sizer9, 0, wxEXPAND);
	group1->Add(5, 5);
	group1->Add(sizer10, 0, wxEXPAND);
	group1->Add(5, 5);
	group1->Add(sizer11, 0, wxEXPAND);
	group1->Add(5, 5);
	group1->Add(sizer12, 0, wxEXPAND);
	group1->Add(5, 5);
	group1->Add(sizer13, 0, wxEXPAND);
	group1->Add(5, 5);
	group1->Add(sizer14, 0, wxEXPAND);
	group1->Add(5, 5);
	group1->Add(sizer15, 0, wxEXPAND);
	group1->Add(5, 5);

	wxBoxSizer *group2 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Post Cleanup"), wxVERTICAL);
	group2->Add(sizer16, 0, wxEXPAND);
	group2->Add(5, 5);
	group2->Add(sizer17, 0, wxEXPAND);
	group2->Add(5, 5);
	group2->Add(sizer18, 0, wxEXPAND);
	group2->Add(5, 5);

	wxBoxSizer *group3 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Macro Command"), wxVERTICAL);
	group3->Add(sizer19, 0, wxEXPAND);
	group3->Add(5, 5);
	group3->Add(sizer20, 0, wxEXPAND);
	group3->Add(5, 5);

	wxBoxSizer* sizerv = new wxBoxSizer(wxVERTICAL);
	sizerv->Add(10, 10);
	sizerv->Add(group1, 0, wxEXPAND);
	sizerv->Add(10, 10);
	sizerv->Add(group2, 0, wxEXPAND);
	sizerv->Add(10, 10);
	sizerv->Add(group3, 0, wxEXPAND);
	sizerv->Add(10, 10);
	page->SetSizer(sizerv);
	page->SetScrollRate(10, 10);

	return page;
}

wxWindow* ComponentDlg::CreateClusteringPage(wxWindow *parent)
{
	wxScrolledWindow *page = new wxScrolledWindow(parent);
	wxStaticText *st = 0;
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;
	//validator: floating point 3
	wxFloatingPointValidator<double> vald_fp1(1);
	wxFloatingPointValidator<double> vald_fp2(2);

	//clustering methods
	wxBoxSizer *sizer1 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Clustering Method"),
		wxVERTICAL);
	wxBoxSizer* sizer11 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Choose Method:",
		wxDefaultPosition, wxSize(100, 20));
	m_cluster_method_exmax_rd = new wxRadioButton(page, ID_ClusterMethodExmaxRd,
		"EM", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	m_cluster_method_dbscan_rd = new wxRadioButton(page, ID_ClusterMethodDbscanRd,
		"DBSCAN", wxDefaultPosition, wxDefaultSize);
	m_cluster_method_kmeans_rd = new wxRadioButton(page, ID_ClusterMethodKmeansRd,
		"K-Means", wxDefaultPosition, wxDefaultSize);
	sizer11->Add(5, 5);
	sizer11->Add(st, 0, wxALIGN_CENTER);
	sizer11->Add(m_cluster_method_kmeans_rd, 0, wxALIGN_CENTER);
	sizer11->Add(10, 10);
	sizer11->Add(m_cluster_method_exmax_rd, 0, wxALIGN_CENTER);
	sizer11->Add(10, 10);
	sizer11->Add(m_cluster_method_dbscan_rd, 0, wxALIGN_CENTER);
	//
	sizer1->Add(10, 10);
	sizer1->Add(sizer11, 0, wxEXPAND);
	sizer1->Add(10, 10);

	//parameters
	wxBoxSizer *sizer2 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Settings"),
		wxVERTICAL);
	//clnum
	wxBoxSizer *sizer21 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Cluster Number:",
		wxDefaultPosition, wxSize(100, 20));
	m_cluster_clnum_sldr = new wxSlider(page, ID_ClusterClnumSldr, 2, 2, 10,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_cluster_clnum_text = new wxTextCtrl(page, ID_ClusterClnumText, "2",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	sizer21->Add(5, 5);
	sizer21->Add(st, 0, wxALIGN_CENTER);
	sizer21->Add(m_cluster_clnum_sldr, 1, wxEXPAND);
	sizer21->Add(m_cluster_clnum_text, 0, wxALIGN_CENTER);
	sizer21->Add(5, 5);
	//maxiter
	wxBoxSizer *sizer22 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Max Iterations:",
		wxDefaultPosition, wxSize(100, 20));
	m_cluster_maxiter_sldr = new wxSlider(page, ID_ClusterMaxIterSldr, 200, 1, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_cluster_maxiter_text = new wxTextCtrl(page, ID_ClusterMaxIterText, "200",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	sizer22->Add(5, 5);
	sizer22->Add(st, 0, wxALIGN_CENTER);
	sizer22->Add(m_cluster_maxiter_sldr, 1, wxEXPAND);
	sizer22->Add(m_cluster_maxiter_text, 0, wxALIGN_CENTER);
	sizer22->Add(5, 5);
	//tol
	wxBoxSizer *sizer23 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Tolerance:",
		wxDefaultPosition, wxSize(100, 20));
	m_cluster_tol_sldr = new wxSlider(page, ID_ClusterTolSldr, 0.90, 1, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_cluster_tol_text = new wxTextCtrl(page, ID_ClusterTolText, "0.90",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp2);
	sizer23->Add(5, 5);
	sizer23->Add(st, 0, wxALIGN_CENTER);
	sizer23->Add(m_cluster_tol_sldr, 1, wxEXPAND);
	sizer23->Add(m_cluster_tol_text, 0, wxALIGN_CENTER);
	sizer23->Add(5, 5);
	//size
	wxBoxSizer *sizer24 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Min. Size:",
		wxDefaultPosition, wxSize(100, 20));
	m_cluster_size_sldr = new wxSlider(page, ID_ClusterSizeSldr, 60, 1, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_cluster_size_text = new wxTextCtrl(page, ID_ClusterSizeText, "60",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	sizer24->Add(5, 5);
	sizer24->Add(st, 0, wxALIGN_CENTER);
	sizer24->Add(m_cluster_size_sldr, 1, wxEXPAND);
	sizer24->Add(m_cluster_size_text, 0, wxALIGN_CENTER);
	sizer24->Add(5, 5);
	//eps
	wxBoxSizer *sizer25 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Neighborhood:",
		wxDefaultPosition, wxSize(100, 20));
	m_cluster_eps_sldr = new wxSlider(page, ID_ClusterEpsSldr, 25, 5, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_cluster_eps_text = new wxTextCtrl(page, ID_ClusterEpsText, "2.5",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp1);
	sizer25->Add(5, 5);
	sizer25->Add(st, 0, wxALIGN_CENTER);
	sizer25->Add(m_cluster_eps_sldr, 1, wxEXPAND);
	sizer25->Add(m_cluster_eps_text, 0, wxALIGN_CENTER);
	sizer25->Add(5, 5);
	//
	sizer2->Add(10, 10);
	sizer2->Add(sizer21, 0, wxEXPAND);
	sizer2->Add(10, 10);
	sizer2->Add(sizer22, 0, wxEXPAND);
	sizer2->Add(10, 10);
	sizer2->Add(sizer23, 0, wxEXPAND);
	sizer2->Add(10, 10);
	sizer2->Add(sizer24, 0, wxEXPAND);
	sizer2->Add(10, 10);
	sizer2->Add(sizer25, 0, wxEXPAND);
	sizer2->Add(10, 10);

	//note
	wxBoxSizer *sizer3 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "N.B."),
		wxVERTICAL);
	st = new wxStaticText(page, 0,
		"Make selection with paint brush first, and then compute clustering on the selection.");
	sizer3->Add(10, 10);
	sizer3->Add(st, 0);
	sizer3->Add(10, 10);

	//all
	wxBoxSizer* sizerv = new wxBoxSizer(wxVERTICAL);
	sizerv->Add(10, 10);
	sizerv->Add(sizer1, 0, wxEXPAND);
	sizerv->Add(10, 10);
	sizerv->Add(sizer2, 0, wxEXPAND);
	sizerv->Add(10, 10);
	sizerv->Add(sizer3, 0, wxEXPAND);
	sizerv->Add(10, 10);

	page->SetSizer(sizerv);
	page->SetScrollRate(10, 10);

	return page;
}

wxWindow* ComponentDlg::CreateAnalysisPage(wxWindow *parent)
{
	wxScrolledWindow *page = new wxScrolledWindow(parent);
	wxStaticText *st = 0;
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;

	//selection tools
	wxBoxSizer *sizer1 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Selection Tools"),
		wxVERTICAL);
	wxBoxSizer* sizer11 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "ID:",
		wxDefaultPosition, wxDefaultSize);
	m_comp_id_text = new wxTextCtrl(page, ID_CompIdText, "",
		wxDefaultPosition, wxSize(80, 23), wxTE_PROCESS_ENTER);
	m_comp_id_x_btn = new wxButton(page, ID_CompIdXBtn, "X",
		wxDefaultPosition, wxSize(23, 23));
	//size limiters
	m_analysis_min_check = new wxCheckBox(page, ID_AnalysisMinCheck, "Min:",
		wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_analysis_min_spin = new wxSpinCtrl(page, ID_AnalysisMinSpin, "0",
		wxDefaultPosition, wxSize(80, 23), wxSP_ARROW_KEYS, 0,
		std::numeric_limits<int>::max(), 0);
	m_analysis_max_check = new wxCheckBox(page, ID_AnalysisMaxCheck, "Max:",
		wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_analysis_max_spin = new wxSpinCtrl(page, ID_AnalysisMaxSpin, "100",
		wxDefaultPosition, wxSize(80, 23), wxSP_ARROW_KEYS, 0,
		std::numeric_limits<int>::max(), 100);
	sizer11->Add(5, 5);
	sizer11->Add(st, 0, wxALIGN_CENTER);
	sizer11->Add(m_comp_id_text, 0, wxALIGN_CENTER);
	sizer11->Add(m_comp_id_x_btn, 0, wxALIGN_CENTER);
	sizer11->Add(10, 10);
	sizer11->Add(m_analysis_min_check, 0, wxALIGN_CENTER);
	sizer11->Add(10, 10);
	sizer11->Add(m_analysis_min_spin, 0, wxALIGN_CENTER);
	sizer11->Add(10, 10);
	sizer11->Add(m_analysis_max_check, 0, wxALIGN_CENTER);
	sizer11->Add(10, 10);
	sizer11->Add(m_analysis_max_spin, 0, wxALIGN_CENTER);
	//buttons
	wxBoxSizer* sizer12 = new wxBoxSizer(wxHORIZONTAL);
	m_comp_append_btn = new wxButton(page, ID_CompAppendBtn, "Append",
		wxDefaultPosition, wxDefaultSize);
	m_comp_exclusive_btn = new wxButton(page, ID_CompExclusiveBtn, "Exclusive",
		wxDefaultPosition, wxDefaultSize);
	m_comp_all_btn = new wxButton(page, ID_CompAllBtn, "AllVox",
		wxDefaultPosition, wxDefaultSize);
	m_comp_full_btn = new wxButton(page, ID_CompFullBtn, "Fulfill",
		wxDefaultPosition, wxDefaultSize);
	m_comp_clear_btn = new wxButton(page, ID_CompClearBtn, "Clear",
		wxDefaultPosition, wxDefaultSize);
	sizer12->Add(5, 5);
	sizer12->Add(m_comp_append_btn, 1, wxEXPAND);
	sizer12->Add(m_comp_exclusive_btn, 1, wxEXPAND);
	sizer12->Add(m_comp_all_btn, 1, wxEXPAND);
	sizer12->Add(5, 5);
	sizer12->Add(m_comp_full_btn, 1, wxEXPAND);
	sizer12->Add(m_comp_clear_btn, 1, wxEXPAND);
	//
	sizer1->Add(10, 10);
	sizer1->Add(sizer11, 1, wxEXPAND);
	sizer1->Add(10, 10);
	sizer1->Add(sizer12, 1, wxEXPAND);
	sizer1->Add(10, 10);

	//modify
	wxBoxSizer *sizer2 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Modify IDs"),
		wxVERTICAL);
	wxBoxSizer* sizer21 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "ID:",
		wxDefaultPosition, wxDefaultSize);
	m_new_id_text = new wxTextCtrl(page, ID_NewIdText, "",
		wxDefaultPosition, wxSize(80, 23));
	m_new_id_x_btn = new wxButton(page, ID_NewIdXBtn, "X",
		wxDefaultPosition, wxSize(23, 23));
	sizer21->Add(5, 5);
	sizer21->Add(st, 0, wxALIGN_CENTER);
	sizer21->Add(m_new_id_text, 0, wxALIGN_CENTER);
	sizer21->Add(m_new_id_x_btn, 0, wxALIGN_CENTER);
	//buttons
	wxBoxSizer* sizer22 = new wxBoxSizer(wxHORIZONTAL);
	m_comp_new_btn = new wxButton(page, ID_CompNewBtn, "Assign",
		wxDefaultPosition, wxDefaultSize);
	m_comp_add_btn = new wxButton(page, ID_CompAddBtn, "Add",
		wxDefaultPosition, wxDefaultSize);
	m_comp_replace_btn = new wxButton(page, ID_CompReplaceBtn, "Replace",
		wxDefaultPosition, wxDefaultSize);
	m_comp_clean_bkg_btn = new wxButton(page, ID_CompCleanBkgBtn, "Clean Sel.",
		wxDefaultPosition, wxDefaultSize);
	m_comp_combine_btn = new wxButton(page, ID_CompCombineBtn, "Combine",
		wxDefaultPosition, wxDefaultSize);
	sizer22->Add(5, 5);
	sizer22->Add(m_comp_new_btn, 1, wxEXPAND);
	sizer22->Add(m_comp_add_btn, 1, wxEXPAND);
	sizer22->Add(m_comp_replace_btn, 1, wxEXPAND);
	sizer22->Add(5, 5);
	sizer22->Add(m_comp_clean_bkg_btn, 1, wxEXPAND);
	sizer22->Add(m_comp_combine_btn, 1, wxEXPAND);
	//
	sizer2->Add(10, 10);
	sizer2->Add(sizer21, 1, wxEXPAND);
	sizer2->Add(10, 10);
	sizer2->Add(sizer22, 1, wxEXPAND);
	sizer2->Add(10, 10);

	//Options
	wxBoxSizer *sizer3 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Options"),
		wxVERTICAL);
	wxBoxSizer *sizer31 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Contact Size:",
		wxDefaultPosition, wxSize(100, 23));
	m_con_size_sldr = new wxSlider(page, ID_ConSizeSldr, 5, 0, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_con_size_text = new wxTextCtrl(page, ID_ConSizeText, "5",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	sizer31->Add(5, 5);
	sizer31->Add(st, 0, wxALIGN_CENTER);
	sizer31->Add(5, 5);
	sizer31->Add(m_con_size_sldr, 1, wxEXPAND);
	sizer31->Add(5, 5);
	sizer31->Add(m_con_size_text, 0, wxALIGN_CENTER);
	sizer31->Add(5, 5);
	wxBoxSizer *sizer32 = new wxBoxSizer(wxHORIZONTAL);
	m_consistent_check = new wxCheckBox(page, ID_ConsistentCheck, "Make color consistent for multiple bricks",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	sizer32->Add(5, 5);
	sizer32->Add(m_consistent_check, 0, wxALIGN_CENTER);
	wxBoxSizer *sizer33 = new wxBoxSizer(wxHORIZONTAL);
	m_colocal_check = new wxCheckBox(page, ID_ColocalCheck, "Compute colocalization with other channels",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	sizer33->Add(5, 5);
	sizer33->Add(m_colocal_check, 0, wxALIGN_CENTER);
	//
	sizer3->Add(10, 10);
	sizer3->Add(sizer31, 0, wxEXPAND);
	sizer3->Add(10, 10);
	sizer3->Add(sizer32, 0, wxEXPAND);
	sizer3->Add(10, 10);
	sizer3->Add(sizer33, 0, wxEXPAND);
	sizer3->Add(10, 10);

	//output
	wxBoxSizer *sizer4 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Output as New Channels"),
		wxVERTICAL);
	//radios
	wxBoxSizer *sizer41 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Channel Type:",
		wxDefaultPosition, wxSize(100, 20));
	m_output_multi_rb = new wxRadioButton(page, ID_OutputMultiRb, "Each Comp.",
		wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	m_output_rgb_rb = new wxRadioButton(page, ID_OutputRgbRb, "R+G+B",
		wxDefaultPosition, wxDefaultSize);
	sizer41->Add(5, 5);
	sizer41->Add(st, 0, wxALIGN_CENTER);
	sizer41->Add(m_output_multi_rb, 0, wxALIGN_CENTER);
	sizer41->Add(5, 5);
	sizer41->Add(m_output_rgb_rb, 0, wxALIGN_CENTER);
	sizer41->Add(5, 5);
	//buttons
	wxBoxSizer *sizer42 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Output:",
		wxDefaultPosition, wxSize(100, 20));
	m_output_random_btn = new wxButton(page, ID_OutputRandomBtn, "Random Colors",
		wxDefaultPosition, wxSize(100, 23));
	m_output_size_btn = new wxButton(page, ID_OutputSizeBtn, "Size-based",
		wxDefaultPosition, wxSize(85, 23));
	m_output_id_btn = new wxButton(page, ID_OutputIdBtn, "IDs",
		wxDefaultPosition, wxSize(65, 23));
	m_output_sn_btn = new wxButton(page, ID_OutputSnBtn, "Serial No.",
		wxDefaultPosition, wxSize(75, 23));
	sizer42->Add(5, 5);
	sizer42->Add(st, 0, wxALIGN_CENTER);
	sizer42->Add(m_output_random_btn, 1, wxEXPAND);
	sizer42->Add(m_output_size_btn, 1, wxEXPAND);
	sizer42->Add(m_output_id_btn, 1, wxEXPAND);
	sizer42->Add(m_output_sn_btn, 1, wxEXPAND);
	sizer42->Add(5, 5);
	//
	sizer4->Add(10, 10);
	sizer4->Add(sizer41, 1, wxEXPAND);
	sizer4->Add(10, 10);
	sizer4->Add(sizer42, 1, wxEXPAND);
	sizer4->Add(10, 10);

	wxBoxSizer *sizer5 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Distances"),
		wxVERTICAL);
	wxBoxSizer *sizer51 = new wxBoxSizer(wxHORIZONTAL);
	m_dist_neighbor_check = new wxCheckBox(page, ID_DistNeighborCheck, "Filter Nearest Neighbors",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_dist_all_chan_check = new wxCheckBox(page, ID_DistAllChanCheck, "All Channel Results",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_dist_output_btn = new wxButton(page, ID_DistOutputBtn, "Compute",
		wxDefaultPosition, wxDefaultSize);
	sizer51->Add(5, 5);
	sizer51->Add(m_dist_neighbor_check, 0, wxALIGN_CENTER);
	sizer51->Add(5, 5);
	sizer51->Add(m_dist_all_chan_check, 0, wxALIGN_CENTER);
	sizer51->AddStretchSpacer(1);
	sizer51->Add(m_dist_output_btn, 0, wxALIGN_CENTER);
	sizer51->Add(5, 5);
	wxBoxSizer *sizer52 = new wxBoxSizer(wxHORIZONTAL);
	m_dist_neighbor_sldr = new wxSlider(page, ID_DistNeighborSldr, 1, 1, 20,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_dist_neighbor_text = new wxTextCtrl(page, ID_DistNeighborText, "1",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	sizer52->Add(5, 5);
	sizer52->Add(m_dist_neighbor_sldr, 1, wxEXPAND);
	sizer52->Add(5, 5);
	sizer52->Add(m_dist_neighbor_text, 0, wxALIGN_CENTER);
	sizer52->Add(5, 5);
	//
	sizer5->Add(10, 10);
	sizer5->Add(sizer51, 0, wxEXPAND);
	sizer5->Add(10, 10);
	sizer5->Add(sizer52, 0, wxEXPAND);
	sizer5->Add(10, 10);

	//alignment
	wxBoxSizer *sizer6 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Align Render View to Analyzed Components"),
		wxVERTICAL);
	wxBoxSizer* sizer61 = new wxBoxSizer(wxHORIZONTAL);
	m_align_center = new wxCheckBox(page, ID_AlignCenter,
		"Move to Center", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	sizer61->Add(5, 5);
	sizer61->Add(m_align_center, 0, wxALIGN_CENTER);
	wxBoxSizer* sizer62 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Tri Axes:",
		wxDefaultPosition, wxSize(50, 22));
	m_align_xyz = new wxButton(page, ID_AlignXYZ, "XYZ",
		wxDefaultPosition, wxSize(65, 22));
	m_align_yxz = new wxButton(page, ID_AlignYXZ, "YXZ",
		wxDefaultPosition, wxSize(65, 22));
	m_align_zxy = new wxButton(page, ID_AlignZXY, "ZXY",
		wxDefaultPosition, wxSize(65, 22));
	m_align_xzy = new wxButton(page, ID_AlignXZY, "XZY",
		wxDefaultPosition, wxSize(65, 22));
	m_align_yzx = new wxButton(page, ID_AlignYZX, "YZX",
		wxDefaultPosition, wxSize(65, 22));
	m_align_zyx = new wxButton(page, ID_AlignZYX, "ZYX",
		wxDefaultPosition, wxSize(65, 22));
	sizer62->Add(5, 5);
	sizer62->Add(st, 0, wxALIGN_CENTER);
	sizer62->Add(m_align_xyz, 1, wxEXPAND);
	sizer62->Add(m_align_yxz, 1, wxEXPAND);
	sizer62->Add(m_align_zxy, 1, wxEXPAND);
	sizer62->Add(m_align_xzy, 1, wxEXPAND);
	sizer62->Add(m_align_yzx, 1, wxEXPAND);
	sizer62->Add(m_align_zyx, 1, wxEXPAND);
	//
	sizer6->Add(5, 5);
	sizer6->Add(sizer61, 1, wxEXPAND);
	sizer6->Add(5, 5);
	sizer6->Add(sizer62, 1, wxEXPAND);
	sizer6->Add(5, 5);

	//all
	wxBoxSizer* sizerv = new wxBoxSizer(wxVERTICAL);
	sizerv->Add(10, 10);
	sizerv->Add(sizer1, 0, wxEXPAND);
	sizerv->Add(10, 10);
	sizerv->Add(sizer2, 0, wxEXPAND);
	sizerv->Add(10, 10);
	sizerv->Add(sizer3, 0, wxEXPAND);
	sizerv->Add(10, 10);
	sizerv->Add(sizer4, 0, wxEXPAND);
	sizerv->Add(10, 10);
	sizerv->Add(sizer5, 0, wxEXPAND);
	sizerv->Add(10, 10);
	sizerv->Add(sizer6, 0, wxEXPAND);
	sizerv->Add(10, 10);

	page->SetSizer(sizerv);
	page->SetScrollRate(10, 10);

	m_analysis_min_check->SetValue(false);
	m_analysis_min_spin->Disable();
	m_analysis_max_check->SetValue(false);
	m_analysis_max_spin->Disable();

	return page;
}

void ComponentDlg::Update()
{
	//update ui
	m_use_sel_chk->SetValue(m_use_sel);
	//comp generate page
	m_iter_text->SetValue(wxString::Format("%d", m_iter));
	m_thresh_text->SetValue(wxString::Format("%.3f", m_thresh));
	//dist
	m_use_dist_field_check->SetValue(m_use_dist_field);
	EnableUseDistField(m_use_dist_field);
	m_dist_strength_text->SetValue(wxString::Format("%.3f", m_dist_strength));
	m_dist_filter_size_text->SetValue(wxString::Format("%d", m_dist_filter_size));
	m_max_dist_text->SetValue(wxString::Format("%d", m_max_dist));
	m_dist_thresh_text->SetValue(wxString::Format("%.3f", m_dist_thresh));
	m_diff_check->SetValue(m_diff);
	EnableDiff(m_diff);
	m_falloff_text->SetValue(wxString::Format("%.3f", m_falloff));
	//m_size_check->SetValue(m_size);
	EnableSize(m_size);
	//m_size_text->SetValue(wxString::Format("%d", m_size_lm));
	EnableDensity(m_density);
	m_density_check->SetValue(m_density);
	m_density_text->SetValue(wxString::Format("%.3f", m_density_thresh));
	m_varth_text->SetValue(wxString::Format("%.4f", m_varth));
	m_density_window_size_text->SetValue(wxString::Format("%d", m_density_window_size));
	m_density_stats_size_text->SetValue(wxString::Format("%d", m_density_stats_size));
	//fixate
	m_fixate_check->SetValue(m_fixate);
	EnableFixate(m_fixate);
	m_fix_size_text->SetValue(wxString::Format("%d", m_fix_size));
	//clean
	EnableClean(m_clean);
	m_clean_check->SetValue(m_clean);
	m_clean_iter_text->SetValue(wxString::Format("%d", m_clean_iter));
	m_clean_limit_text->SetValue(wxString::Format("%d", m_clean_size_vl));
	//record
	int ival = m_command.size();
	m_cmd_count_text->SetValue(wxString::Format("%d", ival));

	//cluster page
	m_cluster_method_exmax_rd->SetValue(m_cluster_method_exmax);
	m_cluster_method_dbscan_rd->SetValue(m_cluster_method_dbscan);
	m_cluster_method_kmeans_rd->SetValue(m_cluster_method_kmeans);
	//parameters
	m_cluster_clnum_text->SetValue(wxString::Format("%d", m_cluster_clnum));
	m_cluster_maxiter_text->SetValue(wxString::Format("%d", m_cluster_maxiter));
	m_cluster_tol_text->SetValue(wxString::Format("%.2f", m_cluster_tol));
	m_cluster_size_text->SetValue(wxString::Format("%d", m_cluster_size));
	m_cluster_eps_text->SetValue(wxString::Format("%.1f", m_cluster_eps));
	UpdateClusterMethod();

	//selection
	if (m_use_min)
	{
		m_analysis_min_check->SetValue(true);
		m_analysis_min_spin->Enable();
	}
	else
	{
		m_analysis_min_check->SetValue(false);
		m_analysis_min_spin->Disable();
	}
	m_analysis_min_spin->SetValue(m_min_num);
	if (m_use_max)
	{
		m_analysis_max_check->SetValue(true);
		m_analysis_max_spin->Enable();
	}
	else
	{
		m_analysis_max_check->SetValue(false);
		m_analysis_max_spin->Disable();
	}
	m_analysis_max_spin->SetValue(m_max_num);

	//options
	m_consistent_check->SetValue(m_consistent);
	m_colocal_check->SetValue(m_colocal);

	//output type
	m_output_multi_rb->SetValue(false);
	m_output_rgb_rb->SetValue(false);
	if (m_output_type == 1)
		m_output_multi_rb->SetValue(true);
	else if (m_output_type == 2)
		m_output_rgb_rb->SetValue(true);

	m_dist_neighbor_check->SetValue(m_use_dist_neighbor);
	m_dist_neighbor_sldr->Enable(m_use_dist_neighbor);
	m_dist_neighbor_text->Enable(m_use_dist_neighbor);
	m_dist_all_chan_check->SetValue(m_use_dist_allchan);

	//generate
	EnableGenerate();

	//output
	m_history_chk->SetValue(m_hold_history);
}

void ComponentDlg::GetSettings()
{
	//defaults
	//comp generate page
	m_use_sel = false;
	m_iter = 50;
	m_thresh = 0.5;
	m_tfactor = 1.0;
	m_use_dist_field = false;
	m_dist_strength = 0.5;
	m_max_dist = 30;
	m_dist_thresh = 0.25;
	m_dist_filter_size = 3;
	m_diff = false;
	m_falloff = 0.01;
	m_size = false;
	m_size_lm = 100;
	m_density = false;
	m_density_thresh = 1.0;
	m_varth = 0.0001;
	m_density_window_size = 5;
	m_density_stats_size = 15;
	m_fixate = false;
	m_fix_size = 50;
	m_clean = false;
	m_clean_iter = 5;
	m_clean_size_vl = 5;

	//cluster
	m_cluster_method_kmeans = true;
	m_cluster_method_exmax = false;
	m_cluster_method_dbscan = false;
	m_cluster_clnum = 2;
	m_cluster_maxiter = 200;
	m_cluster_tol = 0.9f;
	m_cluster_size = 60;
	m_cluster_eps = 2.5;

	//selection
	m_use_min = false;
	m_min_num = 0;
	m_use_max = false;
	m_max_num = 0;
	//colocalization
	m_colocal = false;
	m_consistent = false;

	//distance
	m_use_dist_neighbor = false;
	m_dist_neighbor = 1;
	m_use_dist_allchan = false;

	//update
	m_auto_update = false;

	//record
	m_record_cmd = false;

	//output
	m_output_type = 1;

	//read values
	LoadSettings("");
	Update();
}

void ComponentDlg::LoadSettings(wxString filename)
{
	bool get_basic = false;
	if (!wxFileExists(filename))
	{
		wxString expath = wxStandardPaths::Get().GetExecutablePath();
		expath = wxPathOnly(expath);
		filename = expath + GETSLASH() + "default_component_settings.dft";
		get_basic = true;
	}
	wxFileInputStream is(filename);
	if (!is.IsOk())
		return;
	wxFileConfig fconfig(is);

	//basic settings
	fconfig.Read("use_sel", &m_use_sel);
	fconfig.Read("iter", &m_iter);
	fconfig.Read("thresh", &m_thresh);
	fconfig.Read("use_dist_field", &m_use_dist_field);
	fconfig.Read("dist_strength", &m_dist_strength);
	fconfig.Read("dist_filter_size", &m_dist_filter_size);
	fconfig.Read("max_dist", &m_max_dist);
	fconfig.Read("dist_thresh", &m_dist_thresh);
	fconfig.Read("diff", &m_diff);
	fconfig.Read("falloff", &m_falloff);
	fconfig.Read("size", &m_size);
	fconfig.Read("size_lm", &m_size_lm);
	fconfig.Read("density", &m_density);
	fconfig.Read("density_thresh", &m_density_thresh);
	fconfig.Read("varth", &m_varth);
	fconfig.Read("density_window_size", &m_density_window_size);
	fconfig.Read("density_stats_size", &m_density_stats_size);
	fconfig.Read("clean", &m_clean);
	fconfig.Read("clean_iter", &m_clean_iter);
	fconfig.Read("clean_size_vl", &m_clean_size_vl);

	//cluster
	fconfig.Read("cluster_method_kmeans", &m_cluster_method_kmeans);
	fconfig.Read("cluster_method_exmax", &m_cluster_method_exmax);
	fconfig.Read("cluster_method_dbscan", &m_cluster_method_dbscan);
	//parameters
	fconfig.Read("cluster_clnum", &m_cluster_clnum);
	fconfig.Read("cluster_maxiter", &m_cluster_maxiter);
	fconfig.Read("cluster_tol", &m_cluster_tol);
	fconfig.Read("cluster_size", &m_cluster_size);
	fconfig.Read("cluster_eps", &m_cluster_eps);

	//selection
	fconfig.Read("use_min", &m_use_min);
	fconfig.Read("min_num", &m_min_num);
	fconfig.Read("use_max", &m_use_max);
	fconfig.Read("max_num", &m_max_num);
	//colocalization
	fconfig.Read("colocal", &m_colocal);
	//output
	fconfig.Read("output_type", &m_output_type);

	//m_load_settings_text->SetValue(filename);
}

void ComponentDlg::SaveSettings(wxString filename)
{
	wxString app_name = "FluoRender " +
		wxString::Format("%d.%.1f", VERSION_MAJOR, float(VERSION_MINOR));
	wxString vendor_name = "FluoRender";
	wxString local_name = "default_component_settings.dft";
	wxFileConfig fconfig(app_name, vendor_name, local_name, "",
		wxCONFIG_USE_LOCAL_FILE);

	//comp generate settings
	fconfig.Write("use_sel", m_use_sel);
	fconfig.Write("iter", m_iter);
	fconfig.Write("thresh", m_thresh);
	fconfig.Write("use_dist_field", m_use_dist_field);
	fconfig.Write("dist_strength", m_dist_strength);
	fconfig.Write("dist_filter_size", m_dist_filter_size);
	fconfig.Write("max_dist", m_max_dist);
	fconfig.Write("dist_thresh", m_dist_thresh);
	fconfig.Write("diff", m_diff);
	fconfig.Write("falloff", m_falloff);
	fconfig.Write("size", m_size);
	fconfig.Write("size_lm", m_size_lm);
	fconfig.Write("density", m_density);
	fconfig.Write("density_thresh", m_density_thresh);
	fconfig.Write("varth", m_varth);
	fconfig.Write("density_window_size", m_density_window_size);
	fconfig.Write("density_stats_size", m_density_stats_size);
	fconfig.Write("clean", m_clean);
	fconfig.Write("clean_iter", m_clean_iter);
	fconfig.Write("clean_size_vl", m_clean_size_vl);

	//cluster
	fconfig.Write("cluster_method_kmeans", m_cluster_method_kmeans);
	fconfig.Write("cluster_method_exmax", m_cluster_method_exmax);
	fconfig.Write("cluster_method_dbscan", m_cluster_method_dbscan);
	//parameters
	fconfig.Write("cluster_clnum", m_cluster_clnum);
	fconfig.Write("cluster_maxiter", m_cluster_maxiter);
	fconfig.Write("cluster_tol", m_cluster_tol);
	fconfig.Write("cluster_size", m_cluster_size);
	fconfig.Write("cluster_eps", m_cluster_eps);

	//selection
	fconfig.Write("use_min", m_use_min);
	fconfig.Write("min_num", m_min_num);
	fconfig.Write("use_max", m_use_max);
	fconfig.Write("max_num", m_max_num);
	//colocalization
	fconfig.Write("colocal", m_colocal);
	//output
	fconfig.Write("output_type", m_output_type);

	if (filename == "")
	{
		wxString expath = wxStandardPaths::Get().GetExecutablePath();
		expath = wxPathOnly(expath);
		filename = expath + GETSLASH() + "default_component_settings.dft";
	}

	SaveConfig(fconfig, filename);
}

void ComponentDlg::OnLoadSettings(wxCommandEvent& event)
{
	wxFileDialog *fopendlg = new wxFileDialog(
		m_frame, "Choose a FluoRender component generator setting file",
		"", "", "*.txt;*.dft", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		wxString filename = fopendlg->GetPath();
		LoadSettings(filename);
		Update();
	}

	if (fopendlg)
		delete fopendlg;
}

void ComponentDlg::OnSaveSettings(wxCommandEvent& event)
{
	wxString filename = m_load_settings_text->GetValue();
	if (wxFileExists(filename))
		SaveSettings(filename);
	else
	{
		wxCommandEvent e;
		OnSaveasSettings(e);
	}
}

void ComponentDlg::OnSaveasSettings(wxCommandEvent& event)
{
	wxFileDialog *fopendlg = new wxFileDialog(
		m_frame, "Save a FluoRender component generator setting file",
		"", "", "*.txt", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		wxString filename = fopendlg->GetPath();
		SaveSettings(filename);
		m_load_settings_text->SetValue(filename);
	}

	if (fopendlg)
		delete fopendlg;
}

void ComponentDlg::SetView(fluo::Renderview* view)
{
	m_view = view;
	if (m_view)
		m_aligner.SetView(m_view);
}

//comp generate page
void ComponentDlg::OnIterSldr(wxScrollEvent &event)
{
	int val = event.GetPosition();
	wxString str = wxString::Format("%d", val);
	if (str != m_iter_text->GetValue())
		m_iter_text->SetValue(str);
}

void ComponentDlg::OnIterText(wxCommandEvent &event)
{
	long val = 0;
	m_iter_text->GetValue().ToLong(&val);
	m_iter = val;
	m_iter_sldr->SetValue(m_iter);

	if (m_auto_update)
		GenerateComp(m_use_sel);
}

void ComponentDlg::OnThreshSldr(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 1000.0;
	wxString str = wxString::Format("%.3f", val);
	if (str != m_thresh_text->GetValue())
		m_thresh_text->SetValue(str);
}

void ComponentDlg::OnThreshText(wxCommandEvent &event)
{
	double val = 0.0;
	m_thresh_text->GetValue().ToDouble(&val);
	m_thresh = val;
	m_thresh_sldr->SetValue(int(m_thresh * 1000.0 + 0.5));

	if (m_auto_update)
		GenerateComp(m_use_sel);
}

void ComponentDlg::EnableUseDistField(bool value)
{
	m_use_dist_field = value;
	if (m_use_dist_field)
	{
		m_dist_strength_sldr->Enable();
		m_dist_strength_text->Enable();
		m_dist_filter_size_sldr->Enable();
		m_dist_filter_size_text->Enable();
		m_max_dist_sldr->Enable();
		m_max_dist_text->Enable();
		m_dist_thresh_sldr->Enable();
		m_dist_thresh_text->Enable();
	}
	else
	{
		m_dist_strength_sldr->Disable();
		m_dist_strength_text->Disable();
		m_dist_filter_size_sldr->Disable();
		m_dist_filter_size_text->Disable();
		m_max_dist_sldr->Disable();
		m_max_dist_text->Disable();
		m_dist_thresh_sldr->Disable();
		m_dist_thresh_text->Disable();
	}
}

void ComponentDlg::EnableDiff(bool value)
{
	m_diff = value;
	if (m_diff)
	{
		m_falloff_sldr->Enable();
		m_falloff_text->Enable();
	}
	else
	{
		m_falloff_sldr->Disable();
		m_falloff_text->Disable();
	}
}

void ComponentDlg::OnDistStrengthSldr(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 1000.0;
	wxString str = wxString::Format("%.3f", val);
	if (str != m_dist_strength_text->GetValue())
		m_dist_strength_text->SetValue(str);
}

void ComponentDlg::OnDistStrengthText(wxCommandEvent &event)
{
	double val = 0.0;
	m_dist_strength_text->GetValue().ToDouble(&val);
	m_dist_strength = val;
	m_dist_strength_sldr->SetValue(int(m_dist_strength * 1000.0 + 0.5));

	if (m_auto_update)
		GenerateComp(m_use_sel);
}

void ComponentDlg::OnUseDistFieldCheck(wxCommandEvent &event)
{
	EnableUseDistField(m_use_dist_field_check->GetValue());

	if (m_auto_update)
		GenerateComp(m_use_sel);
}

void ComponentDlg::OnDistFilterSizeSldr(wxScrollEvent &event)
{
	int val = event.GetPosition();
	wxString str = wxString::Format("%d", val);
	if (str != m_dist_filter_size_text->GetValue())
		m_dist_filter_size_text->SetValue(str);
}

void ComponentDlg::OnDistFitlerSizeText(wxCommandEvent &event)
{
	long val = 0;
	m_dist_filter_size_text->GetValue().ToLong(&val);
	m_dist_filter_size = val;
	m_dist_filter_size_sldr->SetValue(m_dist_filter_size);

	if (m_auto_update)
		GenerateComp(m_use_sel);
}

void ComponentDlg::OnMaxDistSldr(wxScrollEvent &event)
{
	int val = event.GetPosition();
	wxString str = wxString::Format("%d", val);
	if (str != m_max_dist_text->GetValue())
		m_max_dist_text->SetValue(str);
}

void ComponentDlg::OnMaxDistText(wxCommandEvent &event)
{
	long val = 1;
	m_max_dist_text->GetValue().ToLong(&val);
	if (val > 255)
		val = 255;
	m_max_dist = val;
	m_max_dist_sldr->SetValue(m_max_dist);

	if (m_auto_update)
		GenerateComp(m_use_sel);
}

void ComponentDlg::OnDistThreshSldr(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 1000.0;
	wxString str = wxString::Format("%.3f", val);
	if (str != m_dist_thresh_text->GetValue())
		m_dist_thresh_text->SetValue(str);
}

void ComponentDlg::OnDistThreshText(wxCommandEvent &event)
{
	double val = 0.0;
	m_dist_thresh_text->GetValue().ToDouble(&val);
	m_dist_thresh = val;
	m_dist_thresh_sldr->SetValue(int(m_dist_thresh * 1000.0 + 0.5));

	if (m_auto_update)
		GenerateComp(m_use_sel);
}

void ComponentDlg::OnDiffCheck(wxCommandEvent &event)
{
	EnableDiff(m_diff_check->GetValue());

	if (m_auto_update)
		GenerateComp(m_use_sel);
}

void ComponentDlg::OnFalloffSldr(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 1000.0;
	wxString str = wxString::Format("%.3f", val);
	if (str != m_falloff_text->GetValue())
		m_falloff_text->SetValue(str);
}

void ComponentDlg::OnFalloffText(wxCommandEvent &event)
{
	double val = 0.0;
	m_falloff_text->GetValue().ToDouble(&val);
	m_falloff = val;
	m_falloff_sldr->SetValue(int(m_falloff * 1000.0 + 0.5));

	if (m_auto_update)
		GenerateComp(m_use_sel);
}

void ComponentDlg::EnableSize(bool value)
{
	m_size = value;
	if (m_size)
	{
		//m_size_sldr->Enable();
		//m_size_text->Enable();
	}
	else
	{
		//m_size_sldr->Disable();
		//m_size_text->Disable();
	}
}

void ComponentDlg::OnSizeCheck(wxCommandEvent &event)
{
	//EnableSize(m_size_check->GetValue());
}

void ComponentDlg::OnSizeSldr(wxScrollEvent &event)
{
	//int val = event.GetPosition();
	//m_size_text->SetValue(wxString::Format("%d", val));
}

void ComponentDlg::OnSizeText(wxCommandEvent &event)
{
	//long val = 0;
	//m_size_text->GetValue().ToLong(&val);
	//m_size_lm = (int)val;
	//m_size_sldr->SetValue(m_size_lm);
}

void ComponentDlg::EnableDensity(bool value)
{
	m_density = value;
	if (m_density)
	{
		m_density_sldr->Enable();
		m_density_text->Enable();
		m_varth_sldr->Enable();
		m_varth_text->Enable();
		m_density_window_size_sldr->Enable();
		m_density_window_size_text->Enable();
		m_density_stats_size_sldr->Enable();
		m_density_stats_size_text->Enable();
	}
	else
	{
		m_density_sldr->Disable();
		m_density_text->Disable();
		m_varth_sldr->Disable();
		m_varth_text->Disable();
		m_density_window_size_sldr->Disable();
		m_density_window_size_text->Disable();
		m_density_stats_size_sldr->Disable();
		m_density_stats_size_text->Disable();
	}
}

void ComponentDlg::OnDensityCheck(wxCommandEvent &event)
{
	EnableDensity(m_density_check->GetValue());

	if (m_auto_update)
		GenerateComp(m_use_sel);
}

void ComponentDlg::OnDensitySldr(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 1000.0;
	wxString str = wxString::Format("%.3f", val);
	if (str != m_density_text->GetValue())
		m_density_text->SetValue(str);
}

void ComponentDlg::OnDensityText(wxCommandEvent &event)
{
	double val = 0.0;
	m_density_text->GetValue().ToDouble(&val);
	m_density_thresh = val;
	m_density_sldr->SetValue(int(m_density_thresh * 1000.0 + 0.5));

	if (m_auto_update)
		GenerateComp(m_use_sel);
}

void ComponentDlg::OnVarthSldr(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 10000.0;
	wxString str = wxString::Format("%.4f", val);
	if (str != m_varth_text->GetValue())
		m_varth_text->SetValue(str);
}

void ComponentDlg::OnVarthText(wxCommandEvent &event)
{
	double val = 0.0;
	m_varth_text->GetValue().ToDouble(&val);
	m_varth = val;
	m_varth_sldr->SetValue(int(m_varth * 10000.0 + 0.5));

	if (m_auto_update)
		GenerateComp(m_use_sel);
}

void ComponentDlg::OnDensityWindowSizeSldr(wxScrollEvent &event)
{
	int val = event.GetPosition();
	wxString str = wxString::Format("%d", val);
	if (str != m_density_window_size_text->GetValue())
		m_density_window_size_text->SetValue(str);
}

void ComponentDlg::OnDensityWindowSizeText(wxCommandEvent &event)
{
	long val = 0;
	m_density_window_size_text->GetValue().ToLong(&val);
	m_density_window_size = val;
	m_density_window_size_sldr->SetValue(m_density_window_size);

	if (m_auto_update)
		GenerateComp(m_use_sel);
}

void ComponentDlg::OnDensityStatsSizeSldr(wxScrollEvent &event)
{
	int val = event.GetPosition();
	wxString str = wxString::Format("%d", val);
	if (str != m_density_stats_size_text->GetValue())
		m_density_stats_size_text->SetValue(str);
}

void ComponentDlg::OnDensityStatsSizeText(wxCommandEvent &event)
{
	long val = 0;
	m_density_stats_size_text->GetValue().ToLong(&val);
	m_density_stats_size = val;
	m_density_stats_size_sldr->SetValue(m_density_stats_size);

	if (m_auto_update)
		GenerateComp(m_use_sel);
}

void ComponentDlg::EnableFixate(bool value)
{
	if (value)
	{
		m_fix_update_btn->Enable();
		m_fix_size_sldr->Enable();
		m_fix_size_text->Enable();
	}
	else
	{
		m_fix_update_btn->Disable();
		m_fix_size_sldr->Disable();
		m_fix_size_text->Disable();
	}
}

void ComponentDlg::OnFixateCheck(wxCommandEvent &event)
{
	m_fixate = m_fixate_check->GetValue();
	EnableFixate(m_fixate);

	if (m_fixate)
		Fixate();

	if (m_auto_update)
	{
		bool bval = m_clean;
		m_clean = false;
		GenerateComp(m_use_sel, false);
		m_clean = bval;
	}
}

void ComponentDlg::OnFixUpdateBtn(wxCommandEvent &event)
{
	Fixate();

	if (m_auto_update)
	{
		bool bval = m_clean;
		m_clean = false;
		GenerateComp(m_use_sel, false);
		m_clean = bval;
	}
}

void ComponentDlg::OnFixSizeSldr(wxScrollEvent &event)
{
	int val = event.GetPosition();
	wxString str = wxString::Format("%d", val);
	if (str != m_fix_size_text->GetValue())
		m_fix_size_text->SetValue(str);
}

void ComponentDlg::OnFixSizeText(wxCommandEvent &event)
{
	long val = 0;
	m_fix_size_text->GetValue().ToLong(&val);
	m_fix_size = val;
	m_fix_size_sldr->SetValue(m_fix_size);

	if (m_auto_update)
		GenerateComp(m_use_sel, false);
	if (m_record_cmd)
		AddCmd("fixate");
}

void ComponentDlg::EnableClean(bool value)
{
	m_clean = value;
	if (m_clean)
	{
		m_clean_btn->Enable();
		m_clean_iter_sldr->Enable();
		m_clean_iter_text->Enable();
		m_clean_limit_sldr->Enable();
		m_clean_limit_text->Enable();
	}
	else
	{
		m_clean_btn->Disable();
		m_clean_iter_sldr->Disable();
		m_clean_iter_text->Disable();
		m_clean_limit_sldr->Disable();
		m_clean_limit_text->Disable();
	}
}

void ComponentDlg::OnCleanCheck(wxCommandEvent &event)
{
	EnableClean(m_clean_check->GetValue());

	if (m_auto_update)
		GenerateComp(m_use_sel);
}

void ComponentDlg::OnCleanIterSldr(wxScrollEvent &event)
{
	int val = event.GetPosition();
	wxString str = wxString::Format("%d", val);
	if (str != m_clean_iter_text->GetValue())
		m_clean_iter_text->SetValue(str);
}

void ComponentDlg::OnCleanIterText(wxCommandEvent &event)
{
	long val = 0;
	m_clean_iter_text->GetValue().ToLong(&val);
	m_clean_iter = (int)val;
	m_clean_iter_sldr->SetValue(m_clean_iter);

	if (m_auto_update)
		GenerateComp(m_use_sel);
}

void ComponentDlg::OnCleanLimitSldr(wxScrollEvent &event)
{
	int val = event.GetPosition();
	wxString str = wxString::Format("%d", val);
	if (str != m_clean_limit_text->GetValue())
		m_clean_limit_text->SetValue(str);
}

void ComponentDlg::OnCleanLimitText(wxCommandEvent &event)
{
	long val = 0;
	m_clean_limit_text->GetValue().ToLong(&val);
	m_clean_size_vl = (int)val;
	m_clean_limit_sldr->SetValue(m_clean_size_vl);

	if (m_auto_update)
		GenerateComp(m_use_sel);
}

//record
void ComponentDlg::AddCmd(const std::string &type)
{
	if (!m_command.empty())
	{
		flrd::CompCmdParams &params = m_command.back();
		if (!params.empty())
		{
			if ((params[0] == "generate" ||
				params[0] == "fixate") &&
				params[0] == type)
			{
				//replace
				m_command.pop_back();
			}
			//else do nothing
		}
	}
	//add
	flrd::CompCmdParams params;
	if (type == "generate")
	{
		params.push_back("generate");
		params.push_back("iter"); params.push_back(std::to_string(m_iter));
		params.push_back("thresh"); params.push_back(std::to_string(m_thresh));
		params.push_back("use_dist_field"); params.push_back(std::to_string(m_use_dist_field));
		params.push_back("dist_strength"); params.push_back(std::to_string(m_dist_strength));
		params.push_back("dist_filter_size"); params.push_back(std::to_string(m_dist_filter_size));
		params.push_back("max_dist"); params.push_back(std::to_string(m_max_dist));
		params.push_back("dist_thresh"); params.push_back(std::to_string(m_dist_thresh));
		params.push_back("diff"); params.push_back(std::to_string(m_diff));
		params.push_back("falloff"); params.push_back(std::to_string(m_falloff));
		params.push_back("density"); params.push_back(std::to_string(m_density));
		params.push_back("density_thresh"); params.push_back(std::to_string(m_density_thresh));
		params.push_back("varth"); params.push_back(std::to_string(m_varth));
		params.push_back("density_window_size"); params.push_back(std::to_string(m_density_window_size));
		params.push_back("density_stats_size"); params.push_back(std::to_string(m_density_stats_size));
		params.push_back("cleanb"); params.push_back(std::to_string(m_clean));
		params.push_back("clean_iter"); params.push_back(std::to_string(m_clean_iter));
		params.push_back("clean_size_vl"); params.push_back(std::to_string(m_clean_size_vl));
	}
	else if (type == "clean")
	{
		params.push_back("clean");
		params.push_back("clean_iter"); params.push_back(std::to_string(m_clean_iter));
		params.push_back("clean_size_vl"); params.push_back(std::to_string(m_clean_size_vl));
	}
	else if (type == "fixate")
	{
		params.push_back("fixate");
		params.push_back("fix_size"); params.push_back(std::to_string(m_fix_size));
	}
	m_command.push_back(params);

	//record
	int ival = m_command.size();
	m_cmd_count_text->SetValue(wxString::Format("%d", ival));
}

void ComponentDlg::ResetCmd()
{
	m_command.clear();
	m_record_cmd_btn->SetValue(false);
	m_record_cmd = false;
	//record
	int ival = m_command.size();
	m_cmd_count_text->SetValue(wxString::Format("%d", ival));
}

void ComponentDlg::PlayCmd(bool use_sel, double tfactor)
{
	//disable first
	m_fixate = false;
	m_auto_update = false;
	m_auto_update_btn->SetValue(false);

	if (m_command.empty())
	{
		//the threshold factor is used to lower the threshold value for semi auto segmentation
		m_tfactor = tfactor;
		GenerateComp(use_sel, false);
		m_tfactor = 1.0;
		return;
	}

	for (auto it = m_command.begin();
		it != m_command.end(); ++it)
	{
		if (it->empty())
			continue;
		if ((*it)[0] == "generate")
		{
			for (auto it2 = it->begin();
				it2 != it->end(); ++it2)
			{
				if (*it2 == "iter")
					m_iter = std::stoi(*(++it2));
				else if (*it2 == "thresh")
					m_thresh = std::stod(*(++it2));
				else if (*it2 == "use_dist_field")
					m_use_dist_field = std::stoi(*(++it2));
				else if (*it2 == "dist_strength")
					m_dist_strength = std::stod(*(++it2));
				else if (*it2 == "dist_filter_size")
					m_dist_filter_size = std::stod(*(++it2));
				else if (*it2 == "max_dist")
					m_max_dist = std::stoi(*(++it2));
				else if (*it2 == "dist_thresh")
					m_dist_thresh = std::stod(*(++it2));
				else if (*it2 == "diff")
					m_diff = std::stoi(*(++it2));
				else if (*it2 == "falloff")
					m_falloff = std::stod(*(++it2));
				else if (*it2 == "density")
					m_density = std::stoi(*(++it2));
				else if (*it2 == "density_thresh")
					m_density_thresh = std::stod(*(++it2));
				else if (*it2 == "varth")
					m_varth = std::stod(*(++it2));
				else if (*it2 == "density_window_size")
					m_density_window_size = std::stoi(*(++it2));
				else if (*it2 == "density_stats_size")
					m_density_stats_size = std::stoi(*(++it2));
				else if (*it2 == "cleanb")
					m_clean = std::stoi(*(++it2));
				else if (*it2 == "clean_iter")
					m_clean_iter = std::stoi(*(++it2));
				else if (*it2 == "clean_size_vl")
					m_clean_size_vl = std::stoi(*(++it2));
			}
			GenerateComp(use_sel, false);
		}
		else if ((*it)[0] == "clean")
		{
			m_clean = true;
			for (auto it2 = it->begin();
				it2 != it->end(); ++it2)
			{
				if (*it2 == "clean_iter")
					m_clean_iter = std::stoi(*(++it2));
				else if (*it2 == "clean_size_vl")
					m_clean_size_vl = std::stoi(*(++it2));
			}
			Clean(use_sel, false);
		}
		else if ((*it)[0] == "fixate")
		{
			m_fixate = true;
			for (auto it2 = it->begin();
				it2 != it->end(); ++it2)
			{
				if (*it2 == "fix_size")
					m_fix_size = std::stoi(*(++it2));
			}
			//GenerateComp(false);
			Fixate(false);
			//return;
		}
	}
	Update();
}

void ComponentDlg::OnRecordCmd(wxCommandEvent &event)
{
	m_record_cmd = m_record_cmd_btn->GetValue();
}

void ComponentDlg::OnPlayCmd(wxCommandEvent &event)
{
	PlayCmd(m_use_sel, 1.0);
}

void ComponentDlg::OnResetCmd(wxCommandEvent &event)
{
	ResetCmd();
}

void ComponentDlg::OnLoadCmd(wxCommandEvent &event)
{
	wxFileDialog *fopendlg = new wxFileDialog(
		m_frame, "Choose a FluoRender component generator macro command",
		"", "", "*.txt;*.dft", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	int rval = fopendlg->ShowModal();
	if (rval != wxID_OK)
	{
		delete fopendlg;
		return;
	}
	wxString filename = fopendlg->GetPath();
	delete fopendlg;

	LoadCmd(filename);
}

void ComponentDlg::OnSaveCmd(wxCommandEvent &event)
{
	wxFileDialog *fopendlg = new wxFileDialog(
		m_frame, "Save a FluoRender component generator macro command",
		"", "", "*.txt", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	int rval = fopendlg->ShowModal();
	if (rval != wxID_OK)
	{
		delete fopendlg;
		return;
	}
	wxString filename = fopendlg->GetPath();
	delete fopendlg;

	SaveCmd(filename);
}

//clustering page
void ComponentDlg::UpdateClusterMethod()
{
	if (m_cluster_method_exmax ||
		m_cluster_method_kmeans)
	{
		m_cluster_clnum_sldr->Enable();
		m_cluster_clnum_text->Enable();
		m_cluster_size_sldr->Disable();
		m_cluster_size_text->Disable();
		m_cluster_eps_sldr->Disable();
		m_cluster_eps_text->Disable();
	}
	if (m_cluster_method_dbscan)
	{
		m_cluster_clnum_sldr->Disable();
		m_cluster_clnum_text->Disable();
		m_cluster_size_sldr->Enable();
		m_cluster_size_text->Enable();
		m_cluster_eps_sldr->Enable();
		m_cluster_eps_text->Enable();
	}
}

void ComponentDlg::OnClusterMethodExmaxCheck(wxCommandEvent &event)
{
	m_cluster_method_exmax = true;
	m_cluster_method_dbscan = false;
	m_cluster_method_kmeans = false;
	UpdateClusterMethod();
}

void ComponentDlg::OnClusterMethodDbscanCheck(wxCommandEvent &event)
{
	m_cluster_method_exmax = false;
	m_cluster_method_dbscan = true;
	m_cluster_method_kmeans = false;
	UpdateClusterMethod();
}

void ComponentDlg::OnClusterMethodKmeansCheck(wxCommandEvent &event)
{
	m_cluster_method_exmax = false;
	m_cluster_method_dbscan = false;
	m_cluster_method_kmeans = true;
	UpdateClusterMethod();
}

//parameters
void ComponentDlg::OnClusterClnumSldr(wxScrollEvent &event)
{
	int val = event.GetPosition();
	wxString str = wxString::Format("%d", val);
	if (str != m_cluster_clnum_text->GetValue())
		m_cluster_clnum_text->SetValue(str);
}

void ComponentDlg::OnClusterClnumText(wxCommandEvent &event)
{
	long val = 0;
	m_cluster_clnum_text->GetValue().ToLong(&val);
	m_cluster_clnum = (int)val;
	m_cluster_clnum_sldr->SetValue(m_cluster_clnum);
}

void ComponentDlg::OnClusterMaxiterSldr(wxScrollEvent &event)
{
	int val = event.GetPosition();
	wxString str = wxString::Format("%d", val);
	if (str != m_cluster_maxiter_text->GetValue())
		m_cluster_maxiter_text->SetValue(str);
}

void ComponentDlg::OnClusterMaxiterText(wxCommandEvent &event)
{
	long val = 0;
	m_cluster_maxiter_text->GetValue().ToLong(&val);
	m_cluster_maxiter = (int)val;
	m_cluster_maxiter_sldr->SetValue(m_cluster_maxiter);
}

void ComponentDlg::OnClusterTolSldr(wxScrollEvent &event)
{
	int val = event.GetPosition();
	wxString str = wxString::Format("%.2f", double(val) / 100.0);
	if (str != m_cluster_tol_text->GetValue())
		m_cluster_tol_text->SetValue(str);
}

void ComponentDlg::OnClusterTolText(wxCommandEvent &event)
{
	double val = 0.9;
	m_cluster_tol_text->GetValue().ToDouble(&val);
	m_cluster_tol = (float)val;
	m_cluster_tol_sldr->SetValue(int(val * 100));
}

void ComponentDlg::OnClusterSizeSldr(wxScrollEvent &event)
{
	int val = event.GetPosition();
	wxString str = wxString::Format("%d", val);
	if (str != m_cluster_size_text->GetValue())
		m_cluster_size_text->SetValue(str);
}

void ComponentDlg::OnClusterSizeText(wxCommandEvent &event)
{
	long val = 0;
	m_cluster_size_text->GetValue().ToLong(&val);
	m_cluster_size = (int)val;
	m_cluster_size_sldr->SetValue(m_cluster_size);
}

void ComponentDlg::OnClusterEpsSldr(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 10.0;
	wxString str = wxString::Format("%.1f", val);
	if (str != m_cluster_eps_text->GetValue())
		m_cluster_eps_text->SetValue(str);
}

void ComponentDlg::OnClusterepsText(wxCommandEvent &event)
{
	double val = 0.0;
	m_cluster_eps_text->GetValue().ToDouble(&val);
	m_cluster_eps = val;
	m_cluster_eps_sldr->SetValue(int(m_cluster_eps * 10.0 + 0.5));
}

//analysis page
void ComponentDlg::OnCompIdText(wxCommandEvent &event)
{
	if (!m_view) return;
	fluo::VolumeData* vd = m_view->GetCurrentVolume();
	if (!vd) return;
	int shuffle = vd->GetShuffle();

	wxString str = m_comp_id_text->GetValue();
	unsigned long id;
	wxColor color(255, 255, 255);
	if (str.ToULong(&id))
	{
		if (!id)
			color = wxColor(24, 167, 181);
		else
		{
			fluo::Color c(id, shuffle);
			color = wxColor(c.r() * 255, c.g() * 255, c.b() * 255);
		}
	}
	m_comp_id_text->SetBackgroundColour(color);
	m_comp_id_text->Refresh();
}

void ComponentDlg::OnCompIdXBtn(wxCommandEvent &event)
{
	m_comp_id_text->Clear();
}

void ComponentDlg::OnAnalysisMinCheck(wxCommandEvent &event)
{
	if (m_analysis_min_check->GetValue())
	{
		m_analysis_min_spin->Enable();
		m_use_min = true;
	}
	else
	{
		m_analysis_min_spin->Disable();
		m_use_min = false;
	}
}

void ComponentDlg::OnAnalysisMinSpin(wxSpinEvent &event)
{
	m_min_num = m_analysis_min_spin->GetValue();
}

void ComponentDlg::OnAnalysisMinText(wxCommandEvent &event)
{
	m_min_num = m_analysis_min_spin->GetValue();
}

void ComponentDlg::OnAnalysisMaxCheck(wxCommandEvent &event)
{
	if (m_analysis_max_check->GetValue())
	{
		m_analysis_max_spin->Enable();
		m_use_max = true;
	}
	else
	{
		m_analysis_max_spin->Disable();
		m_use_max = false;
	}
}

void ComponentDlg::OnAnalysisMaxSpin(wxSpinEvent &event)
{
	m_max_num = m_analysis_max_spin->GetValue();
}

void ComponentDlg::OnAnalysisMaxText(wxCommandEvent &event)
{
	m_max_num = m_analysis_max_spin->GetValue();
}

void ComponentDlg::OnCompFull(wxCommandEvent &event)
{
	SelectFullComp();
}

void ComponentDlg::OnCompExclusive(wxCommandEvent &event)
{
	if (!m_view)
		return;

	bool bval;
	wxString str;
	std::string sstr;
	//get id
	unsigned int id;
	int brick_id;
	str = m_comp_id_text->GetValue();
	sstr = str.ToStdString();

	if (GetIds(sstr, id, brick_id))
	{
		//get current mask
		fluo::VolumeData* vd = m_view->GetCurrentVolume();
		flrd::ComponentSelector comp_selector(vd);
		comp_selector.SetId(flrd::Cell::GetKey(id, brick_id));

		//cell size filter
		bool use = m_analysis_min_check->GetValue();
		unsigned int num = (unsigned int)(m_analysis_min_spin->GetValue());
		comp_selector.SetMinNum(use, num);
		use = m_analysis_max_check->GetValue();
		num = (unsigned int)(m_analysis_max_spin->GetValue());
		comp_selector.SetMaxNum(use, num);
		comp_selector.SetAnalyzer(&m_comp_analyzer);
		comp_selector.Exclusive();

		m_view->Update(39);

		//frame
		if (m_frame)
		{
			if (m_frame->GetBrushToolDlg())
			{
				m_view->getValue(gstPaintCount, bval);
				if (bval)
					m_frame->GetBrushToolDlg()->Update(0);
				m_frame->GetBrushToolDlg()->UpdateUndoRedo();
			}
			if (m_frame->GetColocalizationDlg())
			{
				m_view->getValue(gstPaintColocalize, bval);
				if (bval)
					glbin_agtf->findFirst(gstColocalAgent)->asColocalAgent()->Run();
			}
		}
	}
}

void ComponentDlg::OnCompAppend(wxCommandEvent &event)
{
	if (!m_view)
		return;

	wxString str;
	std::string sstr;
	//get id
	unsigned int id;
	int brick_id;
	str = m_comp_id_text->GetValue();
	sstr = str.ToStdString();
	bool get_all = GetIds(sstr, id, brick_id);
	get_all = !get_all;

	//get current mask
	fluo::VolumeData* vd = m_view->GetCurrentVolume();
	flrd::ComponentSelector comp_selector(vd);
	comp_selector.SetId(flrd::Cell::GetKey(id, brick_id));

	//cell size filter
	bool use = m_analysis_min_check->GetValue();
	unsigned int num = (unsigned int)(m_analysis_min_spin->GetValue());
	comp_selector.SetMinNum(use, num);
	use = m_analysis_max_check->GetValue();
	num = (unsigned int)(m_analysis_max_spin->GetValue());
	comp_selector.SetMaxNum(use, num);
	comp_selector.SetAnalyzer(&m_comp_analyzer);
	comp_selector.Select(get_all);

	m_view->Update(39);

	//frame
	bool bval;
	if (m_frame)
	{
		if (m_frame->GetBrushToolDlg())
		{
			m_view->getValue(gstPaintCount, bval);
			if (bval)
				m_frame->GetBrushToolDlg()->Update(0);
			m_frame->GetBrushToolDlg()->UpdateUndoRedo();
		}
		if (m_frame->GetColocalizationDlg())
		{
			m_view->getValue(gstPaintColocalize, bval);
			if (bval)
				glbin_agtf->findFirst(gstColocalAgent)->asColocalAgent()->Run();
		}
	}
}

void ComponentDlg::OnCompAll(wxCommandEvent &event)
{
	if (!m_view)
		return;

	//get current vd
	fluo::VolumeData* vd = m_view->GetCurrentVolume();
	flrd::ComponentSelector comp_selector(vd);
	comp_selector.All();

	m_view->Update(39);

	//frame
	bool bval;
	if (m_frame)
	{
		if (m_frame->GetBrushToolDlg())
		{
			m_view->getValue(gstPaintCount, bval);
			if (bval)
				m_frame->GetBrushToolDlg()->Update(0);
			m_frame->GetBrushToolDlg()->UpdateUndoRedo();
		}
		if (m_frame->GetColocalizationDlg())
		{
			m_view->getValue(gstPaintColocalize, bval);
			if (bval)
				glbin_agtf->findFirst(gstColocalAgent)->asColocalAgent()->Run();
		}
	}
}

void ComponentDlg::OnCompClear(wxCommandEvent &event)
{
	if (!m_view)
		return;

	//get current vd
	fluo::VolumeData* vd = m_view->GetCurrentVolume();
	flrd::ComponentSelector comp_selector(vd);
	comp_selector.Clear();

	m_view->Update(39);

	//frame
	if (m_frame && m_frame->GetBrushToolDlg())
		m_frame->GetBrushToolDlg()->UpdateUndoRedo();
}

void ComponentDlg::OnShuffle(wxCommandEvent &event)
{
	if (!m_view)
		return;

	//get current vd
	fluo::VolumeData* vd = m_view->GetCurrentVolume();
	if (!vd)
		return;

	vd->IncShuffle();
	m_view->Update(39);
}

//modify
void ComponentDlg::OnNewIDText(wxCommandEvent &event)
{
	if (!m_view) return;
	fluo::VolumeData* vd = m_view->GetCurrentVolume();
	if (!vd) return;
	int shuffle = vd->GetShuffle();

	wxString str = m_new_id_text->GetValue();
	unsigned long id;
	wxColor color(255, 255, 255);
	if (str.ToULong(&id))
	{
		if (!id)
			color = wxColor(24, 167, 181);
		else
		{
			fluo::Color c(id, shuffle);
			color = wxColor(c.r() * 255, c.g() * 255, c.b() * 255);
		}
		m_new_id_text->SetBackgroundColour(color);
		m_cell_new_id = id;
		m_cell_new_id_empty = false;
	}
	else
	{
		m_new_id_text->SetBackgroundColour(color);
		m_cell_new_id_empty = true;
	}
	m_new_id_text->Refresh();
}

void ComponentDlg::OnNewIDX(wxCommandEvent& event)
{
	m_new_id_text->Clear();
}

void ComponentDlg::OnCompNew(wxCommandEvent& event)
{
	if (!m_view)
		return;
	flrd::ComponentEditor editor;
	editor.SetView(m_view);
	editor.NewId(m_cell_new_id,
		m_cell_new_id_empty, false);
	m_view->Update(39);
}

void ComponentDlg::OnCompAdd(wxCommandEvent& event)
{
	if (!m_view)
		return;
	flrd::ComponentEditor editor;
	editor.SetView(m_view);
	editor.NewId(m_cell_new_id,
		m_cell_new_id_empty, true);
	m_view->Update(39);
}

void ComponentDlg::OnCompReplace(wxCommandEvent& event)
{
	if (!m_view)
		return;
	flrd::ComponentEditor editor;
	editor.SetView(m_view);
	editor.Replace(m_cell_new_id,
		m_cell_new_id_empty);
	m_view->Update(39);
}

void ComponentDlg::OnCompCleanBkg(wxCommandEvent& event)
{
	if (!m_view)
		return;
	flrd::ComponentEditor editor;
	editor.SetVolume(m_view->GetCurrentVolume());
	editor.Clean(0);
	m_view->Update(39);
}

void ComponentDlg::OnCompCombine(wxCommandEvent& event)
{
	if (!m_view)
		return;
	flrd::ComponentEditor editor;
	editor.SetView(m_view);
	editor.Combine();
	m_view->Update(39);
}

void ComponentDlg::OnConSizeSldr(wxScrollEvent &event)
{
	int val = event.GetPosition();
	wxString str = wxString::Format("%d", val);
	if (str != m_con_size_text->GetValue())
		m_con_size_text->SetValue(str);
}

void ComponentDlg::OnConSizeText(wxCommandEvent &event)
{
	long val = 0;
	m_con_size_text->GetValue().ToLong(&val);
	m_con_size_sldr->SetValue(val);
	m_comp_analyzer.SetSizeLimit(val);
}

void ComponentDlg::OnConsistentCheck(wxCommandEvent &event)
{
	m_consistent = m_consistent_check->GetValue();
}

void ComponentDlg::OnColocalCheck(wxCommandEvent &event)
{
	m_colocal = m_colocal_check->GetValue();
}

void ComponentDlg::EnableGenerate()
{
	int page = m_notebook->GetSelection();
	switch (page)
	{
	case 0:
	default:
		m_use_sel_chk->Show();
		m_generate_btn->Show();
		m_auto_update_btn->Show();
		m_cluster_btn->Hide();
		m_analyze_btn->Hide();
		m_analyze_sel_btn->Hide();
		break;
	case 1:
		m_use_sel_chk->Hide();
		m_generate_btn->Hide();
		m_auto_update_btn->Hide();
		m_cluster_btn->Show();
		m_analyze_btn->Hide();
		m_analyze_sel_btn->Hide();
		break;
	case 2:
		m_use_sel_chk->Hide();
		m_generate_btn->Hide();
		m_auto_update_btn->Hide();
		m_cluster_btn->Hide();
		m_analyze_btn->Show();
		m_analyze_sel_btn->Show();
		break;
	}
	panel_top->Layout();
	panel_bot->Layout();
}

//output
void ComponentDlg::OnOutputTypeRadio(wxCommandEvent &event)
{
	int id = event.GetId();
	switch (id)
	{
	case ID_OutputMultiRb:
		m_output_type = 1;
		break;
	case ID_OutputRgbRb:
		m_output_type = 2;
		break;
	}
}

void ComponentDlg::OutputMulti(int color_type)
{
	if (!m_view)
		return;
	fluo::VolumeData* vd = m_view->GetCurrentVolume();
	m_comp_analyzer.SetVolume(vd);
	list<fluo::VolumeData*> channs;
	if (m_comp_analyzer.GenMultiChannels(channs, color_type, m_consistent))
	{
		if (m_frame)
		{
			fluo::VolumeGroup* group = 0;
			for (auto i = channs.begin(); i != channs.end(); ++i)
			{
				fluo::VolumeData* vd = *i;
				if (vd)
				{
					m_frame->GetDataManager()->AddVolumeData(vd);
					if (i == channs.begin())
					{
						group = m_view->addVolumeGroup("");
					}
					m_view->addVolumeData(vd, group);
				}
			}
			//if (group)
			//{
			//	//group->SetSyncRAll(true);
			//	//group->SetSyncGAll(true);
			//	//group->SetSyncBAll(true);
			//	fluo::Color col = vd->GetGamma();
			//	group->SetGammaAll(col);
			//	col = vd->GetBrightness();
			//	group->SetBrightnessAll(col);
			//	col = vd->GetHdr();
			//	group->SetHdrAll(col);
			//}
			m_frame->UpdateList();
			m_frame->UpdateTree(vd->getName());
			m_view->Update(39);
		}
	}
}

void ComponentDlg::OutputRgb(int color_type)
{
	if (!m_view)
		return;
	fluo::VolumeData* vd = m_view->GetCurrentVolume();
	m_comp_analyzer.SetVolume(vd);
	list<fluo::VolumeData*> channs;
	if (m_comp_analyzer.GenRgbChannels(channs, color_type, m_consistent))
	{
		if (m_frame)
		{
			fluo::VolumeGroup* group = 0;
			for (auto i = channs.begin(); i != channs.end(); ++i)
			{
				fluo::VolumeData* vd = *i;
				if (vd)
				{
					//m_frame->GetDataManager()->AddVolumeData(vd);
					if (i == channs.begin())
					{
						group = m_view->addVolumeGroup("");
					}
					m_view->addVolumeData(vd, group);
				}
			}
			//if (group)
			//{
			//	//group->SetSyncRAll(true);
			//	//group->SetSyncGAll(true);
			//	//group->SetSyncBAll(true);
			//	fluo::Color col = vd->GetGamma();
			//	group->SetGammaAll(col);
			//	col = vd->GetBrightness();
			//	group->SetBrightnessAll(col);
			//	col = vd->GetHdr();
			//	group->SetHdrAll(col);
			//}
			m_frame->UpdateList();
			m_frame->UpdateTree(vd->getName());
			m_view->Update(39);
		}
	}
}

void ComponentDlg::OnOutputChannels(wxCommandEvent &event)
{
	int id = event.GetId();
	int color_type;
	if (id == ID_OutputRandomBtn)
		color_type = 1;
	else if (id == ID_OutputSizeBtn)
		color_type = 2;

	if (m_output_type == 1)
		OutputMulti(color_type);
	else if (m_output_type == 2)
		OutputRgb(color_type);
}

void ComponentDlg::OnOutputAnnotation(wxCommandEvent &event)
{
	int type = 0;
	if (event.GetId() == ID_OutputSnBtn)
		type = 1;
	if (!m_view)
		return;
	fluo::VolumeData* vd = m_view->GetCurrentVolume();
	m_comp_analyzer.SetVolume(vd);
	fluo::Annotations* ann = glbin_annf->build();
	if (m_comp_analyzer.GenAnnotations(ann, m_consistent, type))
	{
		ann->setRvalu(gstVolume, vd);
		ann->setValue(gstTransform, vd->GetTexture()->transform());
		if (m_frame)
		{
			//DataManager* mgr = m_frame->GetDataManager();
			//if (mgr)
			//	mgr->AddAnnotations(ann);
			m_view->addChild(ann);
			m_frame->UpdateList();
			m_frame->UpdateTree(vd->getName());
		}
		m_view->Update(39);
	}
}

//distance
void ComponentDlg::OnDistNeighborCheck(wxCommandEvent &event)
{
	m_use_dist_neighbor = m_dist_neighbor_check->GetValue();
	m_dist_neighbor_sldr->Enable(m_use_dist_neighbor);
	m_dist_neighbor_text->Enable(m_use_dist_neighbor);
}

void ComponentDlg::OnDistAllChanCheck(wxCommandEvent &event)
{
	m_use_dist_allchan = m_dist_all_chan_check->GetValue();
}

void ComponentDlg::OnDistNeighborSldr(wxScrollEvent &event)
{
	int val = event.GetPosition();
	wxString str = wxString::Format("%d", val);
	if (str != m_dist_neighbor_text->GetValue())
		m_dist_neighbor_text->SetValue(str);
}

void ComponentDlg::OnDistNeighborText(wxCommandEvent &event)
{
	long val = 0;
	m_dist_neighbor_text->GetValue().ToLong(&val);
	m_dist_neighbor = (int)val;
	m_dist_neighbor_sldr->SetValue(m_dist_neighbor);
}

int ComponentDlg::GetDistMatSize()
{
	int gsize = m_comp_analyzer.GetCompGroupSize();
	if (m_use_dist_allchan && gsize > 1)
	{
		int matsize = 0;
		for (int i = 0; i < gsize; ++i)
		{
			flrd::CompGroup* compgroup = m_comp_analyzer.GetCompGroup(i);
			if (!compgroup)
				continue;
			matsize += compgroup->celps.size();
		}
		return matsize;
	}
	else
	{
		flrd::CelpList* list = m_comp_analyzer.GetCelpList();
		if (!list)
			return 0;
		return list->size();
	}
}

void ComponentDlg::OnDistOutput(wxCommandEvent &event)
{
	int num = GetDistMatSize();
	if (num <= 0)
		return;
	int gsize = m_comp_analyzer.GetCompGroupSize();
	int bn = m_comp_analyzer.GetBrickNum();

	//result
	std::string str;
	std::vector<std::vector<double>> rm;//result matrix
	std::vector<std::string> nl;//name list
	std::vector<int> gn;//group number
	rm.reserve(num);
	nl.reserve(num);
	if (gsize > 1)
		gn.reserve(num);
	for (size_t i = 0; i < num; ++i)
	{
		rm.push_back(std::vector<double>());
		rm[i].reserve(num);
		for (size_t j = 0; j < num; ++j)
			rm[i].push_back(0);
	}

	//compute
	double sx, sy, sz;
	std::vector<fluo::Point> pos;
	pos.reserve(num);
	int num2 = 0;//actual number
	if (m_use_dist_allchan && gsize > 1)
	{
		for (int i = 0; i < gsize; ++i)
		{
			flrd::CompGroup* compgroup = m_comp_analyzer.GetCompGroup(i);
			if (!compgroup)
				continue;

			flrd::CellGraph &graph = compgroup->graph;
			flrd::CelpList* list = &(compgroup->celps);
			sx = list->sx;
			sy = list->sy;
			sz = list->sz;
			if (bn > 1)
				graph.ClearVisited();

			for (auto it = list->begin();
				it != list->end(); ++it)
			{
				if (bn > 1)
				{
					if (graph.Visited(it->second))
						continue;
					flrd::CelpList links;
					graph.GetLinkedComps(it->second, links,
						m_comp_analyzer.GetSizeLimit());
				}

				pos.push_back(it->second->GetCenter(sx, sy, sz));
				str = std::to_string(i + 1);
				str += ":";
				str += std::to_string(it->second->Id());
				nl.push_back(str);
				gn.push_back(i);
				num2++;
			}
		}
	}
	else
	{
		flrd::CellGraph &graph = m_comp_analyzer.GetCompGroup(0)->graph;
		flrd::CelpList* list = m_comp_analyzer.GetCelpList();
		sx = list->sx;
		sy = list->sy;
		sz = list->sz;
		if (bn > 1)
			graph.ClearVisited();

		for (auto it = list->begin();
			it != list->end(); ++it)
		{
			if (bn > 1)
			{
				if (graph.Visited(it->second))
					continue;
				flrd::CelpList links;
				graph.GetLinkedComps(it->second, links,
					m_comp_analyzer.GetSizeLimit());
			}

			pos.push_back(it->second->GetCenter(sx, sy, sz));
			str = std::to_string(it->second->Id());
			nl.push_back(str);
			num2++;
		}
	}
	double dist = 0;
	for (int i = 0; i < num2; ++i)
	{
		for (int j = i; j < num2; ++j)
		{
			dist = (pos[i] - pos[j]).length();
			rm[i][j] = dist;
			rm[j][i] = dist;
		}
	}

	bool bdist = m_use_dist_neighbor &&
		m_dist_neighbor > 0 &&
		m_dist_neighbor < num2-1;

	std::vector<double> in_group;//distances with in a group
	std::vector<double> out_group;//distance between groups
	in_group.reserve(num2*num2 / 2);
	out_group.reserve(num2*num2 / 2);
	std::vector<std::vector<int>> im;//index matrix
	if (bdist)
	{
		//sort with indices
		im.reserve(num2);
		for (size_t i = 0; i < num2; ++i)
		{
			im.push_back(std::vector<int>());
			im[i].reserve(num2);
			for (size_t j = 0; j < num2; ++j)
				im[i].push_back(j);
		}
		//copy rm
		std::vector<std::vector<double>> rm2 = rm;
		//sort
		for (size_t i = 0; i < num2; ++i)
		{
			std::sort(im[i].begin(), im[i].end(),
				[&](int ii, int jj) {return rm2[i][ii] < rm2[i][jj]; });
		}
		//fill rm
		for (size_t i = 0; i < num2; ++i)
			for (size_t j = 0; j < num2; ++j)
			{
				rm[i][j] = rm2[i][im[i][j]];
				if (gsize > 1 && j > 0 &&
					j <= m_dist_neighbor)
				{
					if (gn[i] == gn[im[i][j]])
						in_group.push_back(rm[i][j]);
					else
						out_group.push_back(rm[i][j]);
				}
			}
	}
	else
	{
		if (gsize > 1)
		{
			for (int i = 0; i < num2; ++i)
				for (int j = i + 1; j < num2; ++j)
				{
					if (gn[i] == gn[j])
						in_group.push_back(rm[i][j]);
					else
						out_group.push_back(rm[i][j]);
				}
		}
	}

	wxFileDialog *fopendlg = new wxFileDialog(
		this, "Save Analysis Data", "", "",
		"Text file (*.txt)|*.txt",
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		wxString filename = fopendlg->GetPath();
		string str = filename.ToStdString();
		std::ofstream outfile;
		outfile.open(str, std::ofstream::out);
		//output result matrix
		size_t dnum = bdist ? (m_dist_neighbor+1) : num2;
		for (size_t i = 0; i < num2; ++i)
		{
			outfile << nl[i] << "\t";
			for (size_t j = bdist?1:0; j < dnum; ++j)
			{
				outfile << rm[i][j];
				if (j < dnum - 1)
					outfile << "\t";
			}
			outfile << "\n";
		}
		//index matrix
		if (bdist)
		{
			outfile << "\n";
			for (size_t i = 0; i < num2; ++i)
			{
				outfile << nl[i] << "\t";
				for (size_t j = bdist ? 1 : 0; j < dnum; ++j)
				{
					outfile << im[i][j] + 1;
					if (j < dnum - 1)
						outfile << "\t";
				}
				outfile << "\n";
			}
		}
		//output in_group and out_group distances
		if (gsize > 1)
		{
			outfile << "\nIn group Distances\t";
			for (size_t i = 0; i < in_group.size(); ++i)
			{
				outfile << in_group[i];
				if (i < in_group.size() - 1)
					outfile << "\t";
			}
			outfile << "\n";
			outfile << "Out group Distances\t";
			for (size_t i = 0; i < out_group.size(); ++i)
			{
				outfile << out_group[i];
				if (i < out_group.size() - 1)
					outfile << "\t";
			}
			outfile << "\n";
		}
		outfile.close();
	}
	if (fopendlg)
		delete fopendlg;
}

void ComponentDlg::AlignCenter(flrd::Ruler* ruler)
{
	if (!ruler)
		return;
	fluo::Point center = ruler->GetCenter();
	double tx, ty, tz;
	m_view->getValue(gstObjCtrX, tx);
	m_view->getValue(gstObjCtrY, ty);
	m_view->getValue(gstObjCtrZ, tz);
	m_view->setValue(gstObjTransX, tx - center.x());
	m_view->setValue(gstObjTransY, center.y() - ty);
	m_view->setValue(gstObjTransZ, center.z() - tz);
}

void ComponentDlg::OnAlignPca(wxCommandEvent& event)
{
	flrd::CelpList* list = m_comp_analyzer.GetCelpList();
	if (!list || list->size() < 3)
		return;

	double sx = list->sx;
	double sy = list->sy;
	double sz = list->sz;
	flrd::RulerList rulerlist;
	flrd::Ruler ruler;
	ruler.SetRulerType(1);
	fluo::Point pt;
	for (auto it = list->begin();
		it != list->end(); ++it)
	{
		pt = it->second->GetCenter(sx, sy, sz);
		ruler.AddPoint(pt);
	}
	ruler.SetFinished();

	int axis_type = 0;
	switch (event.GetId())
	{
	case ID_AlignXYZ:
		axis_type = 0;
		break;
	case ID_AlignYXZ:
		axis_type = 1;
		break;
	case ID_AlignZXY:
		axis_type = 2;
		break;
	case ID_AlignXZY:
		axis_type = 3;
		break;
	case ID_AlignYZX:
		axis_type = 4;
		break;
	case ID_AlignZYX:
		axis_type = 5;
		break;
	}
	rulerlist.push_back(&ruler);
	m_aligner.SetRulerList(&rulerlist);
	m_aligner.AlignPca(axis_type);
	if (m_align_center->GetValue())
		AlignCenter(&ruler);
}

void ComponentDlg::ClearOutputGrid()
{
	int row = m_output_grid->GetNumberRows();
	m_output_grid->DeleteRows(0, row, true);
}

void ComponentDlg::OnNotebook(wxBookCtrlEvent &event)
{
	EnableGenerate();
}

void ComponentDlg::OnUseSelChk(wxCommandEvent &event)
{
	m_use_sel = m_use_sel_chk->GetValue();
}

void ComponentDlg::OnGenerate(wxCommandEvent &event)
{
	GenerateComp(m_use_sel);
}

void ComponentDlg::OnAutoUpdate(wxCommandEvent &event)
{
	m_auto_update = m_auto_update_btn->GetValue();
	if (m_auto_update)
		GenerateComp(m_use_sel);
}

void ComponentDlg::OnCluster(wxCommandEvent &event)
{
	Cluster();
}

void ComponentDlg::OnCleanBtn(wxCommandEvent &event)
{
	Clean(m_use_sel);
}

void ComponentDlg::Cluster()
{
	m_in_cells.clear();
	m_out_cells.clear();

	if (!m_view)
		return;
	fluo::VolumeData* vd = m_view->GetCurrentVolume();
	if (!vd)
		return;
	flvr::Texture* tex = vd->GetTexture();
	if (!tex)
		return;
	Nrrd* nrrd_data = tex->get_nrrd(0);
	if (!nrrd_data)
		return;
	long bits;
	vd->getValue(gstBits, bits);
	void* data_data = nrrd_data->data;
	if (!data_data)
		return;
	//get mask
	Nrrd* nrrd_mask = vd->GetMask(true);
	if (!nrrd_mask)
		return;
	unsigned char* data_mask = (unsigned char*)(nrrd_mask->data);
	if (!data_mask)
		return;
	Nrrd* nrrd_label = tex->get_nrrd(tex->nlabel());
	if (!nrrd_label)
	{
		vd->AddEmptyLabel(0, true);
		nrrd_label = tex->get_nrrd(tex->nlabel());
	}
	unsigned int* data_label = (unsigned int*)(nrrd_label->data);
	if (!data_label)
		return;

	long nx, ny, nz;
	vd->getValue(gstResX, nx);
	vd->getValue(gstResY, ny);
	vd->getValue(gstResZ, nz);
	double scale;
	vd->getValue(gstIntScale, scale);
	double spcx, spcy, spcz;
	vd->getValue(gstSpcX, spcx);
	vd->getValue(gstSpcY, spcy);
	vd->getValue(gstSpcZ, spcz);

	flrd::ClusterMethod* method = 0;
	//switch method
	if (m_cluster_method_exmax)
	{
		flrd::ClusterExmax* method_exmax = new flrd::ClusterExmax();
		method_exmax->SetClnum(m_cluster_clnum);
		method_exmax->SetMaxiter(m_cluster_maxiter);
		method_exmax->SetProbTol(m_cluster_tol);
		method = method_exmax;
	}
	else if (m_cluster_method_dbscan)
	{
		flrd::ClusterDbscan* method_dbscan = new flrd::ClusterDbscan();
		method_dbscan->SetSize(m_cluster_size);
		method_dbscan->SetEps(m_cluster_eps);
		method = method_dbscan;
	}
	else if (m_cluster_method_kmeans)
	{
		flrd::ClusterKmeans* method_kmeans = new flrd::ClusterKmeans();
		method_kmeans->SetClnum(m_cluster_clnum);
		method_kmeans->SetMaxiter(m_cluster_maxiter);
		method = method_kmeans;
	}

	if (!method)
		return;

	method->SetSpacings(spcx, spcy, spcz);

	//add cluster points
	size_t i, j, k;
	size_t index;
	size_t nxyz = nx * ny * nz;
	unsigned char mask_value;
	float data_value;
	unsigned int label_value;
	bool use_init_cluster = false;
	struct CmpCnt
	{
		unsigned int id;
		unsigned int size;
		bool operator<(const CmpCnt &cc) const
		{
			return size > cc.size;
		}
	};
	std::unordered_map<unsigned int, CmpCnt> init_clusters;
	std::set<CmpCnt> ordered_clusters;
	if (m_cluster_method_exmax)
	{
		for (index = 0; index < nxyz; ++index)
		{
			mask_value = data_mask[index];
			if (!mask_value)
				continue;
			label_value = data_label[index];
			if (!label_value)
				continue;
			auto it = init_clusters.find(label_value);
			if (it == init_clusters.end())
			{
				CmpCnt cc = { label_value, 1 };
				init_clusters.insert(std::pair<unsigned int, CmpCnt>(
					label_value, cc));
			}
			else
			{
				it->second.size++;
			}
		}
		if (init_clusters.size() >= m_cluster_clnum)
		{
			for (auto it = init_clusters.begin();
				it != init_clusters.end(); ++it)
				ordered_clusters.insert(it->second);
			use_init_cluster = true;
		}
	}

	for (i = 0; i < nx; ++i)
	for (j = 0; j < ny; ++j)
	for (k = 0; k < nz; ++k)
	{
		index = nx * ny*k + nx * j + i;
		mask_value = data_mask[index];
		if (mask_value)
		{
			if (bits == 8)
				data_value = ((unsigned char*)data_data)[index] / 255.0f;
			else if (bits == 16)
				data_value = ((unsigned short*)data_data)[index] * scale / 65535.0f;
			flrd::EmVec pnt = { static_cast<double>(i), static_cast<double>(j), static_cast<double>(k) };
			label_value = data_label[index];
			int cid = -1;
			if (use_init_cluster)
			{
				cid = 0;
				bool found = false;
				for (auto it = ordered_clusters.begin();
					it != ordered_clusters.end(); ++it)
				{
					if (label_value == it->id)
					{
						found = true;
						break;
					}
					cid++;
				}
				if (!found)
					cid = -1;
			}
			method->AddClusterPoint(
				pnt, data_value, cid);

			//add to list
			auto iter = m_in_cells.find(label_value);
			if (iter != m_in_cells.end())
			{
				iter->second->Inc(i, j, k, data_value);
			}
			else
			{
				flrd::Cell* cell = new flrd::Cell(label_value);
				cell->Inc(i, j, k, data_value);
				m_in_cells.insert(std::pair<unsigned int, flrd::Celp>
					(label_value, flrd::Celp(cell)));
			}
		}
	}

	if (method->Execute())
	{
		method->GenerateNewIDs(0, (void*)data_label, nx, ny, nz, true);
		m_out_cells = method->GetCellList();
		vd->GetRenderer()->clear_tex_label();
		m_view->Update(39);
	}

	delete method;
}

bool ComponentDlg::GetIds(std::string &str, unsigned int &id, int &brick_id)
{
	std::string::size_type sz;
	try
	{
		id = std::stoul(str, &sz);
	}
	catch (...)
	{
		return false;
	}
	std::string str2;
	if (sz < str.size())
	{
		brick_id = id;
		for (size_t i = sz; i< str.size() - 1; ++i)
		{
			if (std::isdigit(static_cast<unsigned char>(str[i])))
			{
				str2 = str.substr(i);
				try
				{
					id = std::stoul(str2);
				}
				catch(...)
				{
					return false;
				}
				return true;
			}
		}
	}
	brick_id = 0;
	return true;
}

void ComponentDlg::GenerateComp(bool use_sel, bool command)
{
	if (!m_view)
		return;
	fluo::VolumeData* vd = m_view->GetCurrentVolume();
	if (!vd)
		return;

	int clean_iter = m_clean_iter;
	int clean_size = m_clean_size_vl;
	if (!m_clean)
	{
		clean_iter = 0;
		clean_size = 0;
	}

	//get brick number
	int bn = vd->GetAllBrickNum();
	double scale;
	vd->getValue(gstIntScale, scale);

	flrd::ComponentGenerator cg(vd);
	boost::signals2::connection preconn =
		cg.prework.connect(std::bind(
			&ComponentDlg::StartTimer, this, std::placeholders::_1));
	boost::signals2::connection postconn =
		cg.postwork.connect(std::bind(
			&ComponentDlg::StopTimer, this, std::placeholders::_1));
	m_titles.Clear();
	m_values.Clear();
	m_tps.clear();
	m_tps.push_back(std::chrono::high_resolution_clock::now());

	cg.SetUseMask(use_sel);

	vd->AddEmptyMask(cg.GetUseMask()?2:1, true);//select all if no mask, otherwise keep
	if (m_fixate && vd->GetLabel(false))
	{
		vd->LoadLabel2();
		cg.SetIDBit(m_fix_size);
	}
	else
	{
		vd->AddEmptyLabel(0, !use_sel);
		cg.ShuffleID();
	}

	if (m_use_dist_field)
	{
		if (m_density)
		{
			cg.DistDensityField(
				m_diff, m_iter,
				m_thresh*m_tfactor,
				m_falloff,
				m_dist_filter_size,
				m_max_dist,
				m_dist_thresh,
				m_dist_strength,
				m_density_window_size,
				m_density_stats_size,
				m_density_thresh,
				m_varth,
				scale);
		}
		else
		{
			cg.DistGrow(
				m_diff, m_iter,
				m_thresh*m_tfactor,
				m_falloff ,
				m_dist_filter_size,
				m_max_dist,
				m_dist_thresh,
				scale,
				m_dist_strength);
		}
	}
	else
	{
		if (m_density)
		{
			cg.DensityField(
				m_density_window_size,
				m_density_stats_size,
				m_diff, m_iter,
				m_thresh*m_tfactor,
				m_falloff,
				m_density_thresh,
				m_varth,
				scale);
		}
		else
		{
			cg.Grow(
				m_diff,
				m_iter,
				m_thresh*m_tfactor,
				m_falloff,
				scale);
		}
	}

	if (clean_iter > 0)
		cg.Cleanup(clean_iter, clean_size);

	if (bn > 1)
		cg.FillBorders(0.1);

	m_tps.push_back(std::chrono::high_resolution_clock::now());
	std::chrono::duration<double> time_span =
		std::chrono::duration_cast<std::chrono::duration<double>>(
			m_tps.back() - m_tps.front());
	if (m_test_speed)
	{
		m_titles += "Function\t";
		m_titles += "Time\n";
		m_values += "Total\t";
	}
	else
	{
		m_titles += "Total time\n";
	}
	m_values += wxString::Format("%.4f", time_span.count());
	m_values += " sec.\n";
	SetOutput(m_titles, m_values);

	//update
	m_view->Update(39);

	if (command && m_record_cmd)
		AddCmd("generate");
}

void ComponentDlg::Fixate(bool command)
{
	if (!m_view)
		return;
	fluo::VolumeData* vd = m_view->GetCurrentVolume();
	if (!vd)
		return;
	vd->PushLabel(true);

	if (command && m_record_cmd)
		AddCmd("fixate");
}

void ComponentDlg::Clean(bool use_sel, bool command)
{
	if (!m_view)
		return;
	fluo::VolumeData* vd = m_view->GetCurrentVolume();
	if (!vd)
		return;

	int clean_iter = m_clean_iter;
	int clean_size = m_clean_size_vl;
	//if (!m_clean)
	//{
	//	clean_iter = 0;
	//	clean_size = 0;
	//}

	//get brick number
	int bn = vd->GetAllBrickNum();

	flrd::ComponentGenerator cg(vd);
	//boost::signals2::connection connection =
	//	cg.m_sig_progress.connect(std::bind(
	//		&ComponentDlg::UpdateProgress, this));

	cg.SetUseMask(use_sel);

	vd->AddEmptyMask(1, !use_sel);

	if (bn > 1)
		cg.ClearBorders();

	if (clean_iter > 0)
		cg.Cleanup(clean_iter, clean_size);

	if (bn > 1)
		cg.FillBorders(0.1);

	m_view->Update(39);

	if (command && m_record_cmd)
		AddCmd("clean");
}

void ComponentDlg::SelectFullComp()
{
	//get id
	wxString str = m_comp_id_text->GetValue();
	if (str.empty())
	{
		if (!m_view)
			return;
		//get current mask
		fluo::VolumeData* vd = m_view->GetCurrentVolume();
		flrd::ComponentSelector comp_selector(vd);
		//cell size filter
		bool use = m_analysis_min_check->GetValue();
		unsigned int num = (unsigned int)(m_analysis_min_spin->GetValue());
		comp_selector.SetMinNum(use, num);
		use = m_analysis_max_check->GetValue();
		num = (unsigned int)(m_analysis_max_spin->GetValue());
		comp_selector.SetMaxNum(use, num);
		comp_selector.SetAnalyzer(&m_comp_analyzer);
		comp_selector.CompFull();
	}
	else
	{
		wxCommandEvent e;
		OnCompAppend(e);
	}

	m_view->Update(39);

	//frame
	bool bval;
	if (m_frame)
	{
		if (m_frame->GetBrushToolDlg())
		{
			m_view->getValue(gstPaintCount, bval);
			if (bval)
				m_frame->GetBrushToolDlg()->Update(0);
			m_frame->GetBrushToolDlg()->UpdateUndoRedo();
		}
		if (m_frame->GetColocalizationDlg())
		{
			m_view->getValue(gstPaintColocalize, bval);
			if (bval)
				glbin_agtf->findFirst(gstColocalAgent)->asColocalAgent()->Run();
		}
	}
}

void ComponentDlg::OnAnalyze(wxCommandEvent &event)
{
	Analyze(false);
}

void ComponentDlg::OnAnalyzeSel(wxCommandEvent &event)
{
	Analyze(true);
}

void ComponentDlg::Analyze(bool sel)
{
	if (!m_view)
		return;
	fluo::VolumeData* vd = m_view->GetCurrentVolume();
	if (!vd)
		return;

	int bn = vd->GetAllBrickNum();
	m_prog_bit = 97.0f / float(bn * 2 + (m_consistent?1:0));
	m_prog = 0.0f;

	//boost::signals2::connection connection =
	//	m_comp_analyzer.m_sig_progress.connect(std::bind(
	//	&ComponentDlg::UpdateProgress, this));

	m_comp_analyzer.SetVolume(vd);
	if (m_colocal)
	{
		m_comp_analyzer.ClearCoVolumes();
		fluo::VolumeList list = m_view->GetVolList();
		for (auto vdi : list)
		{
			if (vdi != vd)
				m_comp_analyzer.AddCoVolume(vdi);
		}
	}
	m_comp_analyzer.Analyze(sel, m_consistent, m_colocal);

	if (m_consistent)
	{
		//invalidate label mask in gpu
		vd->GetRenderer()->clear_tex_label();
		m_view->Update(39);
	}

	if (m_comp_analyzer.GetListSize() > 10000)
	{
		wxFileDialog *fopendlg = new wxFileDialog(
			this, "Save Analysis Data", "", "",
			"Text file (*.txt)|*.txt",
			wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
		int rval = fopendlg->ShowModal();
		if (rval == wxID_OK)
		{
			wxString filename = fopendlg->GetPath();
			string str = filename.ToStdString();
			m_comp_analyzer.OutputCompListFile(str, 1);
		}
		if (fopendlg)
			delete fopendlg;
	}
	else
	{
		string titles, values;
		m_comp_analyzer.OutputFormHeader(titles);
		m_comp_analyzer.OutputCompListStr(values, 0);
		wxString str1(titles), str2(values);
		SetOutput(str1, str2);
	}

	//connection.disconnect();
}

void ComponentDlg::SetOutput(wxString &titles, wxString &values)
{
	wxString copy_data;
	wxString cur_field;
	wxString cur_line;
	int i, k;
	int id_idx = -1;

	k = 0;
	cur_line = titles;
	do
	{
		cur_field = cur_line.BeforeFirst('\t');
		cur_line = cur_line.AfterFirst('\t');
		if (m_output_grid->GetNumberCols() <= k)
			m_output_grid->InsertCols(k);
		m_output_grid->SetColLabelValue(k, cur_field);
		if (cur_field == "ID")
			id_idx = k;
		++k;
	} while (cur_line.IsEmpty() == false);

	fluo::Color c;
	fluo::VolumeData* vd = 0;
	if (m_view)
		vd = m_view->GetCurrentVolume();
	unsigned long lval;
	wxColor color;

	i = 0;
	copy_data = values;
	do
	{
		k = 0;
		cur_line = copy_data.BeforeFirst('\n');
		copy_data = copy_data.AfterFirst('\n');
		if (m_output_grid->GetNumberRows() <= i ||
			m_hold_history)
			m_output_grid->InsertRows(i);
		do
		{
			cur_field = cur_line.BeforeFirst('\t');
			cur_line = cur_line.AfterFirst('\t');
			m_output_grid->SetCellValue(i, k, cur_field);
			if (k == id_idx && vd)
			{
				if (cur_field.ToULong(&lval))
				{
					c = fluo::Color(lval, vd->GetShuffle());
					color = wxColor(c.r() * 255, c.g() * 255, c.b() * 255);
				}
				else
					color = wxColor(255, 255, 255);
				m_output_grid->SetCellBackgroundColour(i, k, color);
			}
			++k;
		} while (cur_line.IsEmpty() == false);
		++i;
	} while (copy_data.IsEmpty() == false);

	//delete columnsand rows if the old has more
	if (!m_hold_history)
	{
		if (m_output_grid->GetNumberCols() > k)
			m_output_grid->DeleteCols(k,
				m_output_grid->GetNumberCols() - k);
		if (m_output_grid->GetNumberRows() > i)
			m_output_grid->DeleteRows(i,
				m_output_grid->GetNumberRows() - i);
	}

	m_output_grid->AutoSizeColumns();
	m_output_grid->ClearSelection();
}

void ComponentDlg::OnIncludeBtn(wxCommandEvent &event)
{
	IncludeComps();
}

void ComponentDlg::OnExcludeBtn(wxCommandEvent &event)
{
	ExcludeComps();
}

void ComponentDlg::OnHistoryChk(wxCommandEvent& event)
{
	m_hold_history = m_history_chk->GetValue();
}

void ComponentDlg::OnClearHistBtn(wxCommandEvent& event)
{
	m_output_grid->DeleteRows(0, m_output_grid->GetNumberRows());
}

void ComponentDlg::OnKeyDown(wxKeyEvent& event)
{
	if (wxGetKeyState(WXK_CONTROL))
	{
		if (event.GetKeyCode() == wxKeyCode('C'))
			CopyData();
		else if (event.GetKeyCode() == wxKeyCode('V'))
			PasteData();
	}
	event.Skip();
}

void ComponentDlg::OnSelectCell(wxGridEvent& event)
{
	//int r = event.GetRow();
	//int c = event.GetCol();
	//m_output_grid->SelectBlock(r, c, r, c);

	GetCompSelection();

	event.Skip();
}

void ComponentDlg::OnRangeSelect(wxGridRangeSelectEvent& event)
{
	GetCompSelection();

	event.Skip();
}

void ComponentDlg::OnGridLabelClick(wxGridEvent& event)
{
	m_output_grid->SetFocus();
	event.Skip();
}

void ComponentDlg::OnSplitterDclick(wxSplitterEvent& event)
{
	event.Skip(false);
}

void ComponentDlg::CopyData()
{
	int i, k;
	wxString copy_data;
	bool something_in_this_line;

	copy_data.Clear();

	bool t = m_output_grid->IsSelection();

	for (i = 0; i < m_output_grid->GetNumberRows(); i++)
	{
		something_in_this_line = false;
		for (k = 0; k < m_output_grid->GetNumberCols(); k++)
		{
			if (m_output_grid->IsInSelection(i, k))
			{
				if (something_in_this_line == false)
				{  // first field in this line => may need a linefeed
					if (copy_data.IsEmpty() == false)
					{     // ... if it is not the very first field
						copy_data = copy_data + wxT("\n");  // next LINE
					}
					something_in_this_line = true;
				}
				else
				{
					// if not the first field in this line we need a field seperator (TAB)
					copy_data = copy_data + wxT("\t");  // next COLUMN
				}
				copy_data = copy_data + m_output_grid->GetCellValue(i, k);    // finally we need the field value :-)
			}
		}
	}

	if (wxTheClipboard->Open())
	{
		// This data objects are held by the clipboard,
		// so do not delete them in the app.
		wxTheClipboard->SetData(new wxTextDataObject(copy_data));
		wxTheClipboard->Close();
	}
}

void ComponentDlg::PasteData()
{
	/*	wxString copy_data;
		wxString cur_field;
		wxString cur_line;
		int i, k, k2;

		if (wxTheClipboard->Open())
		{
			if (wxTheClipboard->IsSupported(wxDF_TEXT))
			{
				wxTextDataObject data;
				wxTheClipboard->GetData(data);
				copy_data = data.GetText();
			}
			wxTheClipboard->Close();
		}

		i = m_output_grid->GetGridCursorRow();
		k = m_output_grid->GetGridCursorCol();
		k2 = k;

		do
		{
			cur_line = copy_data.BeforeFirst('\n');
			copy_data = copy_data.AfterFirst('\n');
			do
			{
				cur_field = cur_line.BeforeFirst('\t');
				cur_line = cur_line.AfterFirst('\t');
				m_output_grid->SetCellValue(i, k, cur_field);
				k++;
			} while (cur_line.IsEmpty() == false);
			i++;
			k = k2;
		} while (copy_data.IsEmpty() == false);
	*/
}

bool ComponentDlg::GetCellList(flrd::CelpList &cl, bool links)
{
	flrd::CelpList* list = m_comp_analyzer.GetCelpList();
	if (!list || list->empty())
		return false;

	cl.min = list->min;
	cl.max = list->max;
	cl.sx = list->sx;
	cl.sy = list->sy;
	cl.sz = list->sz;

	bool sel_all = false;
	std::vector<unsigned int> ids;
	std::vector<unsigned int> bids;
	int bn = m_comp_analyzer.GetBrickNum();

	//selected cells are retrieved using different functions
	wxArrayInt seli = m_output_grid->GetSelectedCols();
	if (seli.GetCount() > 0)
		sel_all = true;
	if (!sel_all)
	{
		seli = m_output_grid->GetSelectedRows();
		AddSelArrayInt(ids, bids, seli, bn > 1);
		//wxGridCellCoordsArray sela =
		//	m_output_grid->GetSelectionBlockBottomRight();
		//AddSelCoordArray(ids, bids, sela, bn > 1);
		//sela = m_output_grid->GetSelectionBlockTopLeft();
		//AddSelCoordArray(ids, bids, sela, bn > 1);
		//sela = m_output_grid->GetSelectedCells();
		//AddSelCoordArray(ids, bids, sela, bn > 1);
	}

	double sx = list->sx;
	double sy = list->sy;
	double sz = list->sz;
	if (sel_all)
	{
		for (auto it = list->begin(); it != list->end(); ++it)
			FindCelps(cl, it, links);
	}
	else
	{
		for (size_t i = 0; i < ids.size(); ++i)
		{
			unsigned long long key = ids[i];
			unsigned int bid = 0;
			if (bn > 1)
			{
				key = bids[i];
				key = (key << 32) | ids[i];
				bid = bids[i];
			}
			auto it = list->find(key);
			if (it != list->end())
				FindCelps(cl, it, links);
		}
	}

	if (cl.empty())
		return false;
	return true;
}

void ComponentDlg::AddSelArrayInt(std::vector<unsigned int> &ids,
	std::vector<unsigned int> &bids, wxArrayInt &sel, bool bricks)
{
	wxString str;
	unsigned long ulval;
	for (size_t i = 0; i < sel.GetCount(); ++i)
	{
		str = m_output_grid->GetCellValue(sel[i], 0);
		if (str.ToULong(&ulval))
			ids.push_back(ulval);
		if (bricks)
		{
			str = m_output_grid->GetCellValue(sel[i], 1);
			if (str.ToULong(&ulval))
				bids.push_back(ulval);
		}
	}
}

void ComponentDlg::AddSelCoordArray(std::vector<unsigned int> &ids,
	std::vector<unsigned int> &bids, wxGridCellCoordsArray &sel, bool bricks)
{
	wxString str;
	unsigned long ulval;
	for (size_t i = 0; i < sel.GetCount(); ++i)
	{
		str = m_output_grid->GetCellValue(sel[i].GetRow(), 0);
		if (str.ToULong(&ulval))
			ids.push_back(ulval);
		if (bricks)
		{
			str = m_output_grid->GetCellValue(sel[i].GetRow(), 1);
			if (str.ToULong(&ulval))
				bids.push_back(ulval);
		}
	}
}

void ComponentDlg::FindCelps(flrd::CelpList &list,
	flrd::CelpListIter &it, bool links)
{
	list.insert(std::pair<unsigned long long, flrd::Celp>
		(it->second->GetEId(), it->second));

	if (links)
	{
		flrd::CellGraph* graph = m_comp_analyzer.GetCellGraph();
		graph->ClearVisited();
		flrd::CelpList links;
		if (graph->GetLinkedComps(it->second, links,
			m_comp_analyzer.GetSizeLimit()))
		{
			for (auto it2 = links.begin();
				it2 != links.end(); ++it2)
			{
				list.insert(std::pair<unsigned long long, flrd::Celp>
					(it2->second->GetEId(), it2->second));
			}
		}
	}
}

//command
void ComponentDlg::LoadCmd(const wxString &filename)
{
	wxFileInputStream is(filename);
	if (!is.IsOk())
		return;
	wxFileConfig fconfig(is);
	m_cmd_file_text->SetValue(filename);

	m_command.clear();
	int cmd_count = 0;
	wxString str;
	std::string cmd_str = "/cmd" + std::to_string(cmd_count);
	while (fconfig.Exists(cmd_str))
	{
		flrd::CompCmdParams params;
		fconfig.SetPath(cmd_str);
		str = fconfig.Read("type", "");
		if (str == "generate" ||
			str == "clean" ||
			str == "fixate")
			params.push_back(str.ToStdString());
		else
			continue;
		long lval;
		if (fconfig.Read("iter", &lval))
		{
			params.push_back("iter"); params.push_back(std::to_string(lval));
		}
		if (fconfig.Read("use_dist_field", &lval))
		{
			params.push_back("use_dist_field"); params.push_back(std::to_string(lval));
		}
		if (fconfig.Read("dist_filter_size", &lval))
		{
			params.push_back("dist_filter_size"); params.push_back(std::to_string(lval));
		}
		if (fconfig.Read("max_dist", &lval))
		{
			params.push_back("max_dist"); params.push_back(std::to_string(lval));
		}
		if (fconfig.Read("diff", &lval))
		{
			params.push_back("diff"); params.push_back(std::to_string(lval));
		}
		if (fconfig.Read("density", &lval))
		{
			params.push_back("density"); params.push_back(std::to_string(lval));
		}
		if (fconfig.Read("density_window_size", &lval))
		{
			params.push_back("density_window_size"); params.push_back(std::to_string(lval));
		}
		if (fconfig.Read("density_stats_size", &lval))
		{
			params.push_back("density_stats_size"); params.push_back(std::to_string(lval));
		}
		if (fconfig.Read("cleanb", &lval))
		{
			params.push_back("cleanb"); params.push_back(std::to_string(lval));
		}
		if (fconfig.Read("clean_iter", &lval))
		{
			params.push_back("clean_iter"); params.push_back(std::to_string(lval));
		}
		if (fconfig.Read("clean_size_vl", &lval))
		{
			params.push_back("clean_size_vl"); params.push_back(std::to_string(lval));
		}
		if (fconfig.Read("fix_size", &lval))
		{
			params.push_back("fix_size"); params.push_back(std::to_string(lval));
		}
		double dval;
		if (fconfig.Read("thresh", &dval))
		{
			params.push_back("thresh"); params.push_back(std::to_string(dval));
		}
		if (fconfig.Read("dist_strength", &dval))
		{
			params.push_back("dist_strength"); params.push_back(std::to_string(dval));
		}
		if (fconfig.Read("dist_thresh", &dval))
		{
			params.push_back("dist_thresh"); params.push_back(std::to_string(dval));
		}
		if (fconfig.Read("falloff", &dval))
		{
			params.push_back("falloff"); params.push_back(std::to_string(dval));
		}
		if (fconfig.Read("density_thresh", &dval))
		{
			params.push_back("density_thresh"); params.push_back(std::to_string(dval));
		}

		m_command.push_back(params);
		cmd_count++;
		cmd_str = "/cmd" + std::to_string(cmd_count);
	}
	//record
	int ival = m_command.size();
	m_cmd_count_text->SetValue(wxString::Format("%d", ival));
}

void ComponentDlg::SaveCmd(const wxString &filename)
{
	if (m_command.empty())
	{
		AddCmd("generate");
	}

	wxFileConfig fconfig("", "", filename, "",
		wxCONFIG_USE_LOCAL_FILE);
	fconfig.DeleteAll();

	int cmd_count = 0;

	for (auto it = m_command.begin();
		it != m_command.end(); ++it)
	{
		if (it->empty())
			continue;
		if ((*it)[0] == "generate" ||
			(*it)[0] == "clean" ||
			(*it)[0] == "fixate")
		{
			std::string str = "/cmd" + std::to_string(cmd_count++);
			fconfig.SetPath(str);
			str = (*it)[0];
			fconfig.Write("type", wxString(str));
		}
		for (auto it2 = it->begin();
			it2 != it->end(); ++it2)
		{
			if (*it2 == "iter" ||
				*it2 == "use_dist_field" ||
				*it2 == "dist_filter_size" ||
				*it2 == "max_dist" ||
				*it2 == "diff" ||
				*it2 == "density" ||
				*it2 == "density_window_size" ||
				*it2 == "density_stats_size" ||
				*it2 == "cleanb" ||
				*it2 == "clean_iter" ||
				*it2 == "clean_size_vl" ||
				*it2 == "fix_size")
			{
				fconfig.Write(*it2, std::stoi(*(++it2)));
			}
			else if (*it2 == "thresh" ||
				*it2 == "dist_strength" ||
				*it2 == "dist_thresh" ||
				*it2 == "falloff" ||
				*it2 == "density_thresh")
			{
				fconfig.Write(*it2, std::stod(*(++it2)));
			}
		}
	}

	SaveConfig(fconfig, filename);
	m_cmd_file_text->SetValue(filename);
}

void ComponentDlg::StartTimer(std::string str)
{
	if (m_test_speed)
	{
		m_tps.push_back(std::chrono::high_resolution_clock::now());
	}
}

void ComponentDlg::StopTimer(std::string str)
{
	if (m_test_speed)
	{
		auto t0 = m_tps.back();
		m_tps.push_back(std::chrono::high_resolution_clock::now());
		std::chrono::duration<double> time_span =
			std::chrono::duration_cast<std::chrono::duration<double>>(
				m_tps.back() - t0);

		m_values += str + "\t";
		m_values += wxString::Format("%.4f", time_span.count());
		m_values += " sec.\n";
	}
}

void ComponentDlg::GetCompSelection()
{
	if (m_view)
	{
		flrd::CelpList cl;
		GetCellList(cl);
		m_view->SetCellList(cl);
		m_view->setValue(gstInteractive, false);
		//m_view->Update(39);
	}
}

void ComponentDlg::SetCompSelection(std::set<unsigned long long>& ids, int mode)
{
	if (ids.empty())
		return;

	int bn = m_comp_analyzer.GetBrickNum();

	wxString str;
	unsigned long ulv;
	unsigned long long ull;
	bool flag = mode==1;
	int lasti = -1;
	wxArrayInt sel = m_output_grid->GetSelectedRows();
	std::set<int> rows;
	for (int i = 0; i < sel.GetCount(); ++i)
		rows.insert(sel[i]);
	for (int i = 0; i < m_output_grid->GetNumberRows(); ++i)
	{
		str = m_output_grid->GetCellValue(i, 0);
		if (!str.ToULong(&ulv))
			continue;
		if (bn > 1)
		{
			str = m_output_grid->GetCellValue(i, 1);
			if (!str.ToULongLong(&ull))
				continue;
			ull = (ull << 32) | ulv;
		}
		else
			ull = ulv;
		if (ids.find(ull) != ids.end())
		{
			if (!flag)
			{
				m_output_grid->ClearSelection();
				flag = true;
			}
			if (mode == 0)
			{
				m_output_grid->SelectRow(i, true);
				lasti = i;
			}
			else
			{
				if (rows.find(i) != rows.end())
					m_output_grid->DeselectRow(i);
				else
				{
					m_output_grid->SelectRow(i, true);
					lasti = i;
				}
			}
		}
	}

	if (flag)
	{
		GetCompSelection();
		if (lasti >= 0)
			m_output_grid->GoToCell(lasti, 0);
	}
}

void ComponentDlg::IncludeComps()
{
	if (!m_view)
		return;
	fluo::VolumeData* vd = m_view->GetCurrentVolume();
	if (!vd)
		return;

	flrd::CelpList cl;
	if (GetCellList(cl, true))
	{
		//clear complist
		flrd::CelpList *list = m_comp_analyzer.GetCelpList();
		for (auto it = list->begin();
			it != list->end();)
		{
			if (cl.find(it->second->GetEId()) == cl.end())
				it = list->erase(it);
			else
				++it;
		}
		//select cl
		flrd::ComponentSelector comp_selector(vd);
		comp_selector.SelectList(cl);
		ClearOutputGrid();
		string titles, values;
		m_comp_analyzer.OutputFormHeader(titles);
		m_comp_analyzer.OutputCompListStr(values, 0);
		wxString str1(titles), str2(values);
		SetOutput(str1, str2);

		cl.clear();
		m_view->SetCellList(cl);
		m_view->setValue(gstInteractive, false);
		//m_view->Update(39);

		//frame
		bool bval;
		if (m_frame)
		{
			if (m_frame->GetBrushToolDlg())
			{
				m_view->getValue(gstPaintCount, bval);
				if (bval)
					m_frame->GetBrushToolDlg()->Update(0);
				m_frame->GetBrushToolDlg()->UpdateUndoRedo();
			}
			if (m_frame->GetColocalizationDlg())
			{
				m_view->getValue(gstPaintColocalize, bval);
				if (bval)
					glbin_agtf->findFirst(gstColocalAgent)->asColocalAgent()->Run();
			}
		}
	}
}

void ComponentDlg::ExcludeComps()
{
	if (!m_view)
		return;
	fluo::VolumeData* vd = m_view->GetCurrentVolume();
	if (!vd)
		return;

	flrd::CelpList cl;
	if (GetCellList(cl, true))
	{
		//clear complist
		flrd::CelpList *list = m_comp_analyzer.GetCelpList();
		for (auto it = list->begin();
			it != list->end();)
		{
			if (cl.find(it->second->GetEId()) != cl.end())
				it = list->erase(it);
			else
				++it;
		}
		flrd::ComponentSelector comp_selector(vd);
		std::vector<unsigned long long> ids;
		for (auto it = list->begin();
			it != list->end(); ++it)
			ids.push_back(it->second->GetEId());
		comp_selector.Delete(ids);
		ClearOutputGrid();
		string titles, values;
		m_comp_analyzer.OutputFormHeader(titles);
		m_comp_analyzer.OutputCompListStr(values, 0);
		wxString str1(titles), str2(values);
		SetOutput(str1, str2);

		cl.clear();
		m_view->SetCellList(cl);
		m_view->setValue(gstInteractive, false);
		//m_view->RefreshGL(39);

		//frame
		bool bval;
		if (m_frame)
		{
			if (m_frame->GetBrushToolDlg())
			{
				m_view->getValue(gstPaintCount, bval);
				if (bval)
					m_frame->GetBrushToolDlg()->Update(0);
				m_frame->GetBrushToolDlg()->UpdateUndoRedo();
			}
			if (m_frame->GetColocalizationDlg())
			{
				m_view->getValue(gstPaintColocalize, bval);
				if (bval)
					glbin_agtf->findFirst(gstColocalAgent)->asColocalAgent()->Run();
			}
		}
	}
}
