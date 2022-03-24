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
#include <ComponentDlg.h>
#include <RenderFrame.h>
#include <wx/valnum.h>

BEGIN_EVENT_TABLE(ComponentDlg, wxPanel)
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
	EVT_CHECKBOX(ID_AlignCenter, ComponentDlg::OnAlignCenter)
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

ComponentDlg::ComponentDlg(RenderFrame *frame)
	: wxPanel(frame, wxID_ANY,
		wxDefaultPosition,
		wxSize(600, 800),
		0, "ComponentDlg"),
	m_frame(frame)
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
}

ComponentDlg::~ComponentDlg()
{
	//m_agent->SaveSettings("");
}

void ComponentDlg::AssociateRenderview(fluo::Renderview* view)
{
	m_agent->setObject(view);
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
	m_iter_sldr->SetValue(val);
	m_agent->setValue(gstIteration, val);
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
	m_thresh_sldr->SetValue(int(val * 1000.0 + 0.5));
	m_agent->setValue(gstThreshold, val);
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
	m_dist_strength_sldr->SetValue(int(val * 1000.0 + 0.5));
	m_agent->setValue(gstDistFieldStrength, val);
}

void ComponentDlg::OnUseDistFieldCheck(wxCommandEvent &event)
{
	bool bval = m_use_dist_field_check->GetValue();
	m_agent->setValue(gstUseDistField, bval);
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
	m_dist_filter_size_sldr->SetValue(val);
	m_agent->setValue(gstDistFieldFilterSize, val);
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
	m_max_dist_sldr->SetValue(val);
	m_agent->setValue(gstMaxDist, val);
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
	m_dist_thresh_sldr->SetValue(int(val * 1000.0 + 0.5));
	m_agent->setValue(gstDistFieldThresh, val);
}

void ComponentDlg::OnDiffCheck(wxCommandEvent &event)
{
	bool bval = m_diff_check->GetValue();
	m_agent->setValue(gstUseDiffusion, bval);
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
	m_falloff_sldr->SetValue(int(val * 1000.0 + 0.5));
	m_agent->setValue(gstDiffusionFalloff, val);
}

void ComponentDlg::OnSizeCheck(wxCommandEvent &event)
{
}

void ComponentDlg::OnSizeSldr(wxScrollEvent &event)
{
}

void ComponentDlg::OnSizeText(wxCommandEvent &event)
{
}

void ComponentDlg::OnDensityCheck(wxCommandEvent &event)
{
	bool bval = m_density_check->GetValue();
	m_agent->setValue(gstUseDensityField, bval);
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
	m_density_sldr->SetValue(int(val * 1000.0 + 0.5));
	m_agent->setValue(gstDensityFieldThresh, val);
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
	m_varth_sldr->SetValue(int(val * 10000.0 + 0.5));
	m_agent->setValue(gstDensityVarThresh, val);
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
	m_density_window_size_sldr->SetValue(val);
	m_agent->setValue(gstDensityWindowSize, val);
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
	m_density_stats_size_sldr->SetValue(val);
	m_agent->setValue(gstDensityStatsSize, val);
}

void ComponentDlg::OnFixateCheck(wxCommandEvent &event)
{
	bool bval = m_fixate_check->GetValue();
	m_agent->setValue(gstFixateEnable, bval);
}

void ComponentDlg::OnFixUpdateBtn(wxCommandEvent &event)
{
	m_agent->Fixate(true);
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
	m_fix_size_sldr->SetValue(val);
	m_agent->setValue(gstFixateSize, val);
}

void ComponentDlg::OnCleanCheck(wxCommandEvent &event)
{
	bool bval = m_clean_check->GetValue();
	m_agent->setValue(gstCleanEnable, bval);
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
	m_clean_iter_sldr->SetValue(val);
	m_agent->setValue(gstCleanIteration, val);
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
	m_clean_limit_sldr->SetValue(val);
	m_agent->setValue(gstCleanSize, val);
}

void ComponentDlg::OnRecordCmd(wxCommandEvent &event)
{
	bool bval = m_record_cmd_btn->GetValue();
	m_agent->setValue(gstRecordCmd, bval);
}

void ComponentDlg::OnPlayCmd(wxCommandEvent &event)
{
	bool bval;
	m_agent->getValue(gstUseSelection, bval);
	m_agent->PlayCmd(bval, 1.0);
}

void ComponentDlg::OnResetCmd(wxCommandEvent &event)
{
	m_agent->ResetCmd();
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

	m_agent->LoadCmd(filename);
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

	m_agent->SaveCmd(filename);
}

//clustering page
void ComponentDlg::OnClusterMethodExmaxCheck(wxCommandEvent &event)
{
	m_agent->setValue(gstClusterMethod, long(1));
}

void ComponentDlg::OnClusterMethodDbscanCheck(wxCommandEvent &event)
{
	m_agent->setValue(gstClusterMethod, long(2));
}

void ComponentDlg::OnClusterMethodKmeansCheck(wxCommandEvent &event)
{
	m_agent->setValue(gstClusterMethod, long(0));
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
	m_cluster_clnum_sldr->SetValue(val);
	m_agent->setValue(gstClusterNum, val);
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
	m_cluster_maxiter_sldr->SetValue(val);
	m_agent->setValue(gstClusterMaxIter, val);
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
	m_cluster_tol_sldr->SetValue(int(val * 100));
	m_agent->setValue(gstClusterTol, val);
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
	m_cluster_size_sldr->SetValue(val);
	m_agent->setValue(gstClusterSize, val);
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
	m_cluster_eps_sldr->SetValue(int(val * 10.0 + 0.5));
	m_agent->setValue(gstClusterEps, val);
}

//analysis page
void ComponentDlg::OnCompIdText(wxCommandEvent &event)
{
	int shuffle = m_agent->GetShuffle();

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
	m_agent->setValue(gstCompIdStr, str.ToStdString());
	unsigned long ulval;
	if (str.ToULong(&ulval))
		m_agent->setValue(gstCompId, ulval);
}

void ComponentDlg::OnCompIdXBtn(wxCommandEvent &event)
{
	m_comp_id_text->Clear();
	m_agent->setValue(gstCompIdStr, std::string(""));
	m_agent->setValue(gstCompId, unsigned long(0));
}

void ComponentDlg::OnAnalysisMinCheck(wxCommandEvent &event)
{
	bool bval = m_analysis_min_check->GetValue();
	m_agent->setValue(gstUseMin, bval);
}

void ComponentDlg::OnAnalysisMinSpin(wxSpinEvent &event)
{
	long lval = m_analysis_min_spin->GetValue();
	m_agent->setValue(gstMinValue, lval);
}

void ComponentDlg::OnAnalysisMinText(wxCommandEvent &event)
{
	long lval = m_analysis_min_spin->GetValue();
	m_agent->setValue(gstMinValue, lval);
}

void ComponentDlg::OnAnalysisMaxCheck(wxCommandEvent &event)
{
	bool bval = m_analysis_max_check->GetValue();
	m_agent->setValue(gstUseMax, bval);
}

void ComponentDlg::OnAnalysisMaxSpin(wxSpinEvent &event)
{
	bool lval = m_analysis_max_spin->GetValue();
	m_agent->setValue(gstMaxValue, lval);
}

void ComponentDlg::OnAnalysisMaxText(wxCommandEvent &event)
{
	bool lval = m_analysis_max_spin->GetValue();
	m_agent->setValue(gstMaxValue, lval);
}

void ComponentDlg::OnCompFull(wxCommandEvent &event)
{
	m_agent->CompFull();
}

void ComponentDlg::OnCompExclusive(wxCommandEvent &event)
{
	m_agent->CompExclusive();
}

void ComponentDlg::OnCompAppend(wxCommandEvent &event)
{
	m_agent->CompAppend();
}

void ComponentDlg::OnCompAll(wxCommandEvent &event)
{
	m_agent->CompAll();
}

void ComponentDlg::OnCompClear(wxCommandEvent &event)
{
	m_agent->CompClear();
}

void ComponentDlg::OnShuffle(wxCommandEvent &event)
{
	m_agent->ShuffleCurVolume();
}

//modify
void ComponentDlg::OnNewIDText(wxCommandEvent &event)
{
	int shuffle = m_agent->GetShuffle();

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
		//m_cell_new_id = id;
		//m_cell_new_id_empty = false;
	}
	m_new_id_text->SetBackgroundColour(color);
	//m_cell_new_id_empty = true;
	m_new_id_text->Refresh();
	m_agent->setValue(gstCompIdStr, str.ToStdString());
	unsigned long ulval;
	if (str.ToULong(&ulval))
		m_agent->setValue(gstCompId, ulval);
}

void ComponentDlg::OnNewIDX(wxCommandEvent& event)
{
	m_new_id_text->Clear();
	m_agent->setValue(gstCompIdStr, std::string(""));
	m_agent->setValue(gstCompId, unsigned long(0));
}

void ComponentDlg::OnCompNew(wxCommandEvent& event)
{
	m_agent->CompNew();
}

void ComponentDlg::OnCompAdd(wxCommandEvent& event)
{
	m_agent->CompAdd();
}

void ComponentDlg::OnCompReplace(wxCommandEvent& event)
{
	m_agent->CompReplace();
}

void ComponentDlg::OnCompCleanBkg(wxCommandEvent& event)
{
	m_agent->CompCleanBkg();
}

void ComponentDlg::OnCompCombine(wxCommandEvent& event)
{
	m_agent->CompCombine();
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
	m_agent->setValue(gstCompSizeLimit, val);
	//m_comp_analyzer.SetSizeLimit(val);
}

void ComponentDlg::OnConsistentCheck(wxCommandEvent &event)
{
	bool bval = m_consistent_check->GetValue();
	m_agent->setValue(gstCompConsistent, bval);
}

void ComponentDlg::OnColocalCheck(wxCommandEvent &event)
{
	bool bval = m_colocal_check->GetValue();
	m_agent->setValue(gstCompColocal, bval);
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
	long lval = 0;
	int id = event.GetId();
	switch (id)
	{
	case ID_OutputMultiRb:
		lval = 1;
		break;
	case ID_OutputRgbRb:
		lval = 2;
		break;
	}
	m_agent->setValue(gstCompOutputType, lval);
}

void ComponentDlg::OnOutputChannels(wxCommandEvent &event)
{
	int id = event.GetId();
	int color_type;
	if (id == ID_OutputRandomBtn)
		color_type = 1;
	else if (id == ID_OutputSizeBtn)
		color_type = 2;
	m_agent->CompOutput(color_type);
}

void ComponentDlg::OnOutputAnnotation(wxCommandEvent &event)
{
	int type = 0;
	if (event.GetId() == ID_OutputSnBtn)
		type = 1;
	m_agent->OutputAnnotation(type);
}

//distance
void ComponentDlg::OnDistNeighborCheck(wxCommandEvent &event)
{
	bool bval = m_dist_neighbor_check->GetValue();
	m_dist_neighbor_sldr->Enable(bval);
	m_dist_neighbor_text->Enable(bval);
	m_agent->setValue(gstDistNeighbor, bval);
}

void ComponentDlg::OnDistAllChanCheck(wxCommandEvent &event)
{
	bool bval = m_dist_all_chan_check->GetValue();
	m_agent->setValue(gstDistAllChan, bval);
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
	m_dist_neighbor_sldr->SetValue(val);
	m_agent->setValue(gstDistNeighborValue, val);
}

void ComponentDlg::OnDistOutput(wxCommandEvent &event)
{
	wxFileDialog *fopendlg = new wxFileDialog(
		this, "Save Analysis Data", "", "",
		"Text file (*.txt)|*.txt",
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		wxString filename = fopendlg->GetPath();
		std::wstring str = filename.ToStdWstring();
		m_agent->DistOutput(str);
	}
	if (fopendlg)
		delete fopendlg;
}

void ComponentDlg::OnAlignCenter(wxCommandEvent &event)
{
	bool bval = m_align_center->GetValue();
	m_agent->setValue(gstAlignCenter, bval);
}

void ComponentDlg::OnAlignPca(wxCommandEvent& event)
{
	long axis_type = 0;
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
	m_agent->setValue(gstAlignAxisType, axis_type);
	m_agent->AlignPca();
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
	bool bval = m_use_sel_chk->GetValue();
	m_agent->setValue(gstUseSelection, bval);
}

void ComponentDlg::OnGenerate(wxCommandEvent &event)
{
	m_agent->GenerateComp();
}

void ComponentDlg::OnAutoUpdate(wxCommandEvent &event)
{
	bool bval = m_auto_update_btn->GetValue();
	m_agent->setValue(gstAutoUpdate, bval);
}

void ComponentDlg::OnCluster(wxCommandEvent &event)
{
	m_agent->Cluster();
}

void ComponentDlg::OnCleanBtn(wxCommandEvent &event)
{
	m_agent->Clean();
}

void ComponentDlg::OnAnalyze(wxCommandEvent &event)
{
	m_agent->Analyze(false);
}

void ComponentDlg::OnAnalyzeSel(wxCommandEvent &event)
{
	m_agent->Analyze(true);
}

void ComponentDlg::SetOutput(const wxString &titles, const wxString &values)
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
	wxColor color;
	unsigned long lval;
	int shuffle = m_agent->GetShuffle();
	bool hold_history;
	m_agent->getValue(gstHoldHistory, hold_history);

	i = 0;
	copy_data = values;
	do
	{
		k = 0;
		cur_line = copy_data.BeforeFirst('\n');
		copy_data = copy_data.AfterFirst('\n');
		if (m_output_grid->GetNumberRows() <= i ||
			hold_history)
			m_output_grid->InsertRows(i);
		do
		{
			cur_field = cur_line.BeforeFirst('\t');
			cur_line = cur_line.AfterFirst('\t');
			m_output_grid->SetCellValue(i, k, cur_field);
			if (k == id_idx)
			{
				if (cur_field.ToULong(&lval))
				{
					c = fluo::Color(lval, shuffle);
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
	if (!hold_history)
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
	m_agent->IncludeComps();
}

void ComponentDlg::OnExcludeBtn(wxCommandEvent &event)
{
	m_agent->ExcludeComps();
}

void ComponentDlg::OnHistoryChk(wxCommandEvent& event)
{
	bool bval = m_history_chk->GetValue();
	m_agent->setValue(gstHoldHistory, bval);
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

	m_agent->GetCompSelection();

	event.Skip();
}

void ComponentDlg::OnRangeSelect(wxGridRangeSelectEvent& event)
{
	m_agent->GetCompSelection();

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

