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
#include <ComponentDlg.h>
#include <Global.h>
#include <MainFrame.h>
#include <RenderCanvas.h>
#include <BrushToolDlg.h>
#include <ColocalizationDlg.h>
#include <Components/CompEditor.h>
#include <Database/RecordHistParams.h>
#include <wxSingleSlider.h>
#include <wx/scrolwin.h>
#include <wx/valnum.h>
#include <wx/stdpaths.h>
#include <limits>
#include <string>
#include <cctype>
#include <set>
#include <fstream>

ComponentDlg::ComponentDlg(MainFrame *frame)
	: PropPanel(frame, frame,
		wxDefaultPosition,
		frame->FromDIP(wxSize(600, 800)),
		0, "ComponentDlg"),
	m_hold_history(false)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);
	SetDoubleBuffered(true);
	SetMinSize(FromDIP(wxSize(100, 100)));

	wxBoxSizer *mainsizer = new wxBoxSizer(wxHORIZONTAL);
	wxSplitterWindow *splittermain = new wxSplitterWindow(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxSP_THIN_SASH | wxSP_BORDER | wxSP_LIVE_UPDATE);
	splittermain->SetMinimumPaneSize(160);
	mainsizer->Add(splittermain, 1, wxBOTTOM | wxLEFT | wxEXPAND, 5);

	panel_top = new wxPanel(splittermain, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER);
	wxBoxSizer* sizerT = new wxBoxSizer(wxHORIZONTAL);
	//notebook
	m_notebook = new wxNotebook(panel_top, wxID_ANY);
	m_notebook->AddPage(CreateCompGenPage(m_notebook), "Generate");
	m_notebook->AddPage(CreateClusteringPage(m_notebook), "Cluster");
	m_notebook->AddPage(CreateAnalysisPage(m_notebook), "Analysis");
	sizerT->Add(m_notebook, 1, wxALL | wxEXPAND, 5);
	panel_top->SetSizer(sizerT);

	panel_bot = new wxPanel(splittermain, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER);
	wxBoxSizer* sizer1 = new wxBoxSizer(wxHORIZONTAL);
	m_shuffle_btn = new wxButton(panel_bot, wxID_ANY, "Shuffle",
		wxDefaultPosition, wxDefaultSize);
	m_use_sel_chk = new wxCheckBox(panel_bot, wxID_ANY, "Use Paint",
		wxDefaultPosition, wxDefaultSize);
	m_use_ml_chk = new wxCheckBox(panel_bot, wxID_ANY, "Use M.L.",
		wxDefaultPosition, wxDefaultSize);
	m_generate_btn = new wxButton(panel_bot, wxID_ANY, "Generate",
		wxDefaultPosition, FromDIP(wxSize(75, -1)));
	m_auto_update_btn = new wxToggleButton(panel_bot, wxID_ANY, "Auto Update",
		wxDefaultPosition, FromDIP(wxSize(75, -1)));
	m_cluster_btn = new wxButton(panel_bot, wxID_ANY, "Cluster",
		wxDefaultPosition, FromDIP(wxSize(75, -1)));
	m_analyze_btn = new wxButton(panel_bot, wxID_ANY, "Analyze",
		wxDefaultPosition, wxDefaultSize);
	m_analyze_sel_btn = new wxButton(panel_bot, wxID_ANY, "Analyze Paint",
		wxDefaultPosition, wxDefaultSize);
	m_shuffle_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnShuffle, this);
	m_use_sel_chk->Bind(wxEVT_CHECKBOX, &ComponentDlg::OnUseSelChk, this);
	m_use_ml_chk->Bind(wxEVT_CHECKBOX, &ComponentDlg::OnUseMlChk, this);
	m_generate_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnGenerate, this);
	m_auto_update_btn->Bind(wxEVT_TOGGLEBUTTON, &ComponentDlg::OnAutoUpdate, this);
	m_cluster_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnCluster, this);
	m_analyze_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnAnalyze, this);
	m_analyze_sel_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnAnalyzeSel, this);
	sizer1->Add(m_shuffle_btn, 0, wxALIGN_CENTER);
	sizer1->AddStretchSpacer();
	sizer1->Add(m_use_ml_chk, 0, wxALIGN_CENTER);
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
	m_include_btn = new wxButton(panel_bot, wxID_ANY,
		"Include", wxDefaultPosition, wxDefaultSize);
	m_exclude_btn = new wxButton(panel_bot, wxID_ANY,
		"Exclude", wxDefaultPosition, wxDefaultSize);
	m_history_chk = new wxCheckBox(panel_bot, wxID_ANY,
		"Hold History", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_clear_hist_btn = new wxButton(panel_bot, wxID_ANY,
		"Clear History", wxDefaultPosition, wxDefaultSize);
	m_include_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnIncludeBtn, this);
	m_exclude_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnExcludeBtn, this);
	m_history_chk->Bind(wxEVT_CHECKBOX, &ComponentDlg::OnHistoryChk, this);
	m_clear_hist_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnClearHistBtn, this);
	sizer2_1->Add(m_include_btn, 0, wxALIGN_CENTER);
	sizer2_1->Add(m_exclude_btn, 0, wxALIGN_CENTER);
	sizer2_1->AddStretchSpacer(1);
	sizer2_1->Add(m_history_chk, 0, wxALIGN_CENTER);
	sizer2_1->Add(5, 5);
	sizer2_1->Add(m_clear_hist_btn, 0, wxALIGN_CENTER);
	//grid
	m_output_grid = new wxGrid(panel_bot, wxID_ANY);
	m_output_grid->CreateGrid(0, 1);
	m_output_grid->Fit();
	m_output_grid->Bind(wxEVT_KEY_DOWN, &ComponentDlg::OnKeyDown, this);
	m_output_grid->Bind(wxEVT_GRID_SELECT_CELL, &ComponentDlg::OnSelectCell, this);
	m_output_grid->Bind(wxEVT_GRID_RANGE_SELECT, &ComponentDlg::OnRangeSelect, this);
	m_output_grid->Bind(wxEVT_GRID_LABEL_LEFT_CLICK, &ComponentDlg::OnGridLabelClick, this);
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
	Bind(wxEVT_SPLITTER_DOUBLECLICKED, &ComponentDlg::OnSplitterDclick, this);
	Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &ComponentDlg::OnNotebook, this);

	SetSizer(mainsizer);
	panel_top->Layout();
	panel_bot->Layout();

	//GetSettings();
	//LoadTable();
}

ComponentDlg::~ComponentDlg()
{
}

wxWindow* ComponentDlg::CreateCompGenPage(wxWindow *parent)
{
	wxScrolledWindow *page = new wxScrolledWindow(parent);
	page->SetBackgroundColour(((wxNotebook*)parent)->GetThemeBackgroundColour());

	wxStaticText *st = 0;
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;
	//validator: floating point 3
	wxFloatingPointValidator<double> vald_fp3(3);

	wxBoxSizer* sizer1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Iterations:",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_iter_sldr = new wxSingleSlider(page, wxID_ANY, 0, 0, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_iter_text = new wxTextCtrl(page, wxID_ANY, "0",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_int);
	m_iter_sldr->Bind(wxEVT_SCROLL_CHANGED, &ComponentDlg::OnIterSldr, this);
	m_iter_text->Bind(wxEVT_TEXT, &ComponentDlg::OnIterText, this);
	sizer1->Add(2, 2);
	sizer1->Add(st, 0, wxALIGN_CENTER);
	sizer1->Add(m_iter_sldr, 1, wxEXPAND);
	sizer1->Add(m_iter_text, 0, wxALIGN_CENTER);
	sizer1->Add(2, 2);

	wxBoxSizer* sizer2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Threshold:",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_thresh_sldr = new wxSingleSlider(page, wxID_ANY, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_thresh_text = new wxTextCtrl(page, wxID_ANY, "0.000",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_fp3);
	m_thresh_sldr->Bind(wxEVT_SCROLL_CHANGED, &ComponentDlg::OnThreshSldr, this);
	m_thresh_text->Bind(wxEVT_TEXT, &ComponentDlg::OnThreshText, this);
	sizer2->Add(2, 2);
	sizer2->Add(st, 0, wxALIGN_CENTER);
	sizer2->Add(m_thresh_sldr, 1, wxEXPAND);
	sizer2->Add(m_thresh_text, 0, wxALIGN_CENTER);
	sizer2->Add(2, 2);

	wxBoxSizer* sizer3 = new wxBoxSizer(wxHORIZONTAL);
	m_diff_check = new wxCheckBox(page, wxID_ANY, "Enable Diffusion",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_diff_check->Bind(wxEVT_CHECKBOX, &ComponentDlg::OnDiffCheck, this);
	sizer3->Add(2, 2);
	sizer3->Add(m_diff_check, 0, wxALIGN_CENTER);

	wxBoxSizer* sizer4 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Smoothness:",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_falloff_sldr = new wxSingleSlider(page, wxID_ANY, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_falloff_text = new wxTextCtrl(page, wxID_ANY, "0.000",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_fp3);
	m_falloff_sldr->Bind(wxEVT_SCROLL_CHANGED, &ComponentDlg::OnFalloffSldr, this);
	m_falloff_text->Bind(wxEVT_TEXT, &ComponentDlg::OnFalloffText, this);
	sizer4->Add(2, 2);
	sizer4->Add(st, 0, wxALIGN_CENTER);
	sizer4->Add(m_falloff_sldr, 1, wxEXPAND);
	sizer4->Add(m_falloff_text, 0, wxALIGN_CENTER);
	sizer4->Add(2, 2);

	//density
	wxBoxSizer* sizer5 = new wxBoxSizer(wxHORIZONTAL);
	m_density_check = new wxCheckBox(page, wxID_ANY, "Use Density Field",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_density_check->Bind(wxEVT_CHECKBOX, &ComponentDlg::OnDensityCheck, this);
	sizer5->Add(2, 2);
	sizer5->Add(m_density_check, 0, wxALIGN_CENTER);

	wxBoxSizer* sizer6 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Separation:",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_density_sldr = new wxSingleSlider(page, wxID_ANY, 1000, 0, 10000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_density_text = new wxTextCtrl(page, wxID_ANY, "1.0",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_fp3);
	m_density_sldr->Bind(wxEVT_SCROLL_CHANGED, &ComponentDlg::OnDensitySldr, this);
	m_density_text->Bind(wxEVT_TEXT, &ComponentDlg::OnDensityText, this);
	sizer6->Add(2, 2);
	sizer6->Add(st, 0, wxALIGN_CENTER);
	sizer6->Add(m_density_sldr, 1, wxEXPAND);
	sizer6->Add(m_density_text, 0, wxALIGN_CENTER);
	sizer6->Add(2, 2);

	wxBoxSizer* sizer61 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Noise Level:",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_varth_sldr = new wxSingleSlider(page, wxID_ANY, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_varth_text = new wxTextCtrl(page, wxID_ANY, "0.0",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_fp3);
	m_varth_sldr->Bind(wxEVT_SCROLL_CHANGED, &ComponentDlg::OnVarthSldr, this);
	m_varth_text->Bind(wxEVT_TEXT, &ComponentDlg::OnVarthText, this);
	sizer61->Add(2, 2);
	sizer61->Add(st, 0, wxALIGN_CENTER);
	sizer61->Add(m_varth_sldr, 1, wxEXPAND);
	sizer61->Add(m_varth_text, 0, wxALIGN_CENTER);
	sizer61->Add(2, 2);

	wxBoxSizer* sizer7 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Filter Size:",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_density_window_size_sldr = new wxSingleSlider(page, wxID_ANY, 5, 1, 20,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_density_window_size_text = new wxTextCtrl(page, wxID_ANY, "5",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_int);
	m_density_window_size_sldr->Bind(wxEVT_SCROLL_CHANGED, &ComponentDlg::OnDensityWindowSizeSldr, this);
	m_density_window_size_text->Bind(wxEVT_TEXT, &ComponentDlg::OnDensityWindowSizeText, this);
	sizer7->Add(2, 2);
	sizer7->Add(st, 0, wxALIGN_CENTER);
	sizer7->Add(m_density_window_size_sldr, 1, wxEXPAND);
	sizer7->Add(m_density_window_size_text, 0, wxALIGN_CENTER);
	sizer7->Add(2, 2);

	wxBoxSizer* sizer8 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Feature Size:",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_density_stats_size_sldr = new wxSingleSlider(page, wxID_ANY, 15, 1, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_density_stats_size_text = new wxTextCtrl(page, wxID_ANY, "15",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_int);
	m_density_stats_size_sldr->Bind(wxEVT_SCROLL_CHANGED, &ComponentDlg::OnDensityStatsSizeSldr, this);
	m_density_stats_size_text->Bind(wxEVT_TEXT, &ComponentDlg::OnDensityStatsSizeText, this);
	sizer8->Add(2, 2);
	sizer8->Add(st, 0, wxALIGN_CENTER);
	sizer8->Add(m_density_stats_size_sldr, 1, wxEXPAND);
	sizer8->Add(m_density_stats_size_text, 0, wxALIGN_CENTER);
	sizer8->Add(2, 2);

	//distance field
	wxBoxSizer* sizer9 = new wxBoxSizer(wxHORIZONTAL);
	m_use_dist_field_check = new wxCheckBox(page, wxID_ANY, "Use Distance Field",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_use_dist_field_check->Bind(wxEVT_CHECKBOX, &ComponentDlg::OnUseDistFieldCheck, this);
	sizer9->Add(2, 2);
	sizer9->Add(m_use_dist_field_check, 0, wxALIGN_CENTER);

	wxBoxSizer* sizer10 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Strength:",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_dist_strength_sldr = new wxSingleSlider(page, wxID_ANY, 500, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_dist_strength_text = new wxTextCtrl(page, wxID_ANY, "0.5",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_fp3);
	m_dist_strength_sldr->Bind(wxEVT_SCROLL_CHANGED, &ComponentDlg::OnDistStrengthSldr, this);
	m_dist_strength_text->Bind(wxEVT_TEXT, &ComponentDlg::OnDistStrengthText, this);
	sizer10->Add(2, 2);
	sizer10->Add(st, 0, wxALIGN_CENTER);
	sizer10->Add(m_dist_strength_sldr, 1, wxEXPAND);
	sizer10->Add(m_dist_strength_text, 0, wxALIGN_CENTER);
	sizer10->Add(2, 2);

	wxBoxSizer* sizer11 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Perimeter Value:",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_dist_thresh_sldr = new wxSingleSlider(page, wxID_ANY, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_dist_thresh_text = new wxTextCtrl(page, wxID_ANY, "0.000",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_fp3);
	m_dist_thresh_sldr->Bind(wxEVT_SCROLL_CHANGED, &ComponentDlg::OnDistThreshSldr, this);
	m_dist_thresh_text->Bind(wxEVT_TEXT, &ComponentDlg::OnDistThreshText, this);
	sizer11->Add(2, 2);
	sizer11->Add(st, 0, wxALIGN_CENTER);
	sizer11->Add(m_dist_thresh_sldr, 1, wxEXPAND);
	sizer11->Add(m_dist_thresh_text, 0, wxALIGN_CENTER);
	sizer11->Add(2, 2);

	wxBoxSizer* sizer12 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Filter Size:",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_dist_filter_size_sldr = new wxSingleSlider(page, wxID_ANY, 3, 1, 20,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_dist_filter_size_text = new wxTextCtrl(page, wxID_ANY, "3",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_int);
	m_dist_filter_size_sldr->Bind(wxEVT_SCROLL_CHANGED, &ComponentDlg::OnDistFilterSizeSldr, this);
	m_dist_filter_size_text->Bind(wxEVT_TEXT, &ComponentDlg::OnDistFitlerSizeText, this);
	sizer12->Add(2, 2);
	sizer12->Add(st, 0, wxALIGN_CENTER);
	sizer12->Add(m_dist_filter_size_sldr, 1, wxEXPAND);
	sizer12->Add(m_dist_filter_size_text, 0, wxALIGN_CENTER);
	sizer12->Add(2, 2);

	wxBoxSizer* sizer13 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Feature Size:",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_max_dist_sldr = new wxSingleSlider(page, wxID_ANY, 30, 1, 255,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_max_dist_text = new wxTextCtrl(page, wxID_ANY, "30",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_int);
	m_max_dist_sldr->Bind(wxEVT_SCROLL_CHANGED, &ComponentDlg::OnMaxDistSldr, this);
	m_max_dist_text->Bind(wxEVT_TEXT, &ComponentDlg::OnMaxDistText, this);
	sizer13->Add(2, 2);
	sizer13->Add(st, 0, wxALIGN_CENTER);
	sizer13->Add(m_max_dist_sldr, 1, wxEXPAND);
	sizer13->Add(m_max_dist_text, 0, wxALIGN_CENTER);
	sizer13->Add(2, 2);

	//fixate
	wxBoxSizer* sizer14 = new wxBoxSizer(wxHORIZONTAL);
	m_fixate_check = new wxCheckBox(page, wxID_ANY, "Fixate Grown Regions",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_grow_fixed_check = new wxCheckBox(page, wxID_ANY, "Continue on Fixiated Regions",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_fix_update_btn = new wxButton(page, wxID_ANY, "Refix",
		wxDefaultPosition, FromDIP(wxSize(75, -1)), wxALIGN_LEFT);
	m_fixate_check->Bind(wxEVT_CHECKBOX, &ComponentDlg::OnFixateCheck, this);
	m_grow_fixed_check->Bind(wxEVT_CHECKBOX, &ComponentDlg::OnGrowFixedCheck, this);
	m_fix_update_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnFixUpdateBtn, this);
	sizer14->Add(2, 2);
	sizer14->Add(m_fixate_check, 0, wxALIGN_CENTER);
	sizer14->Add(2, 2);
	sizer14->Add(m_grow_fixed_check, 0, wxALIGN_CENTER);
	sizer14->AddStretchSpacer(1);
	sizer14->Add(m_fix_update_btn, 0, wxALIGN_CENTER);
	sizer14->Add(2, 2);

	wxBoxSizer* sizer15 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Stop Size:",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_fix_size_sldr = new wxSingleSlider(page, wxID_ANY, 50, 1, 200,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_fix_size_text = new wxTextCtrl(page, wxID_ANY, "50",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_int);
	m_fix_size_sldr->Bind(wxEVT_SCROLL_CHANGED, &ComponentDlg::OnFixSizeSldr, this);
	m_fix_size_text->Bind(wxEVT_TEXT, &ComponentDlg::OnFixSizeText, this);
	sizer15->Add(2, 2);
	sizer15->Add(st, 0, wxALIGN_CENTER);
	sizer15->Add(m_fix_size_sldr, 1, wxEXPAND);
	sizer15->Add(m_fix_size_text, 0, wxALIGN_CENTER);
	sizer15->Add(2, 2);

	//clean
	wxBoxSizer* sizer16 = new wxBoxSizer(wxHORIZONTAL);
	m_clean_check = new wxCheckBox(page, wxID_ANY, "Clean Up",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_clean_btn = new wxButton(page, wxID_ANY, "Clean More",
		wxDefaultPosition, FromDIP(wxSize(75, -1)), wxALIGN_LEFT);
	m_clean_check->Bind(wxEVT_CHECKBOX, &ComponentDlg::OnCleanCheck, this);
	m_clean_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnCleanBtn, this);
	sizer16->Add(2, 2);
	sizer16->Add(m_clean_check, 0, wxALIGN_CENTER);
	sizer16->AddStretchSpacer(1);
	sizer16->Add(m_clean_btn, 0, wxALIGN_CENTER);
	sizer16->Add(2, 2);

	//iterations
	wxBoxSizer* sizer17 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Iterations:",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_clean_iter_sldr = new wxSingleSlider(page, wxID_ANY, 5, 1, 50,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_clean_iter_text = new wxTextCtrl(page, wxID_ANY, "5",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_int);
	m_clean_iter_sldr->Bind(wxEVT_SCROLL_CHANGED, &ComponentDlg::OnCleanIterSldr, this);
	m_clean_iter_text->Bind(wxEVT_TEXT, &ComponentDlg::OnCleanIterText, this);
	sizer17->Add(2, 2);
	sizer17->Add(st, 0, wxALIGN_CENTER);
	sizer17->Add(m_clean_iter_sldr, 1, wxEXPAND);
	sizer17->Add(m_clean_iter_text, 0, wxALIGN_CENTER);
	sizer17->Add(2, 2);

	//limit
	wxBoxSizer* sizer18 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Stop Size:",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_clean_limit_sldr = new wxSingleSlider(page, wxID_ANY, 5, 1, 50,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_clean_limit_text = new wxTextCtrl(page, wxID_ANY, "5",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_int);
	m_clean_limit_sldr->Bind(wxEVT_SCROLL_CHANGED, &ComponentDlg::OnCleanLimitSldr, this);
	m_clean_limit_text->Bind(wxEVT_TEXT, &ComponentDlg::OnCleanLimitText, this);
	sizer18->Add(2, 2);
	sizer18->Add(st, 0, wxALIGN_CENTER);
	sizer18->Add(m_clean_limit_sldr, 1, wxEXPAND);
	sizer18->Add(m_clean_limit_text, 0, wxALIGN_CENTER);
	sizer18->Add(2, 2);

	//command record
	wxBoxSizer* sizer19 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Recorder:",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_cmd_count_text = new wxTextCtrl(page, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(75, -1)), wxTE_READONLY | wxTE_RIGHT);
	m_record_cmd_btn = new wxToggleButton(page, wxID_ANY, "Record",
		wxDefaultPosition, FromDIP(wxSize(75, -1)));
	m_play_cmd_btn = new wxButton(page, wxID_ANY, "Play",
		wxDefaultPosition, FromDIP(wxSize(75, -1)));
	m_reset_cmd_btn = new wxButton(page, wxID_ANY, "Reset",
		wxDefaultPosition, FromDIP(wxSize(75, -1)));
	m_record_cmd_btn->Bind(wxEVT_TOGGLEBUTTON, &ComponentDlg::OnRecordCmd, this);
	m_play_cmd_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnPlayCmd, this);
	m_reset_cmd_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnResetCmd, this);
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
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_cmd_file_text = new wxTextCtrl(page, wxID_ANY, "",
		wxDefaultPosition, wxDefaultSize);
	m_load_cmd_btn = new wxButton(page, wxID_ANY, "Load",
		wxDefaultPosition, FromDIP(wxSize(75, -1)));
	m_save_cmd_btn = new wxButton(page, wxID_ANY, "Save",
		wxDefaultPosition, FromDIP(wxSize(75, -1)));
	m_load_cmd_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnLoadCmd, this);
	m_save_cmd_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnSaveCmd, this);
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
	page->SetBackgroundColour(((wxNotebook*)parent)->GetThemeBackgroundColour());

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
		wxDefaultPosition, FromDIP(wxSize(100, 20)));
	m_cluster_method_exmax_rd = new wxRadioButton(page, ID_ClusterMethodExmaxRd,
		"EM", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	m_cluster_method_dbscan_rd = new wxRadioButton(page, ID_ClusterMethodDbscanRd,
		"DBSCAN", wxDefaultPosition, wxDefaultSize);
	m_cluster_method_kmeans_rd = new wxRadioButton(page, ID_ClusterMethodKmeansRd,
		"K-Means", wxDefaultPosition, wxDefaultSize);
	m_cluster_method_exmax_rd->Bind(wxEVT_RADIOBUTTON, &ComponentDlg::OnClusterMethodCheck, this);
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
		wxDefaultPosition, FromDIP(wxSize(100, 20)));
	m_cluster_clnum_sldr = new wxSingleSlider(page, wxID_ANY, 2, 2, 10,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_cluster_clnum_text = new wxTextCtrl(page, wxID_ANY, "2",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_int);
	m_cluster_clnum_sldr->Bind(wxEVT_SCROLL_CHANGED, &ComponentDlg::OnClusterClnumSldr, this);
	m_cluster_clnum_text->Bind(wxEVT_TEXT, &ComponentDlg::OnClusterClnumText, this);
	sizer21->Add(5, 5);
	sizer21->Add(st, 0, wxALIGN_CENTER);
	sizer21->Add(m_cluster_clnum_sldr, 1, wxEXPAND);
	sizer21->Add(m_cluster_clnum_text, 0, wxALIGN_CENTER);
	sizer21->Add(5, 5);
	//maxiter
	wxBoxSizer *sizer22 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Max Iterations:",
		wxDefaultPosition, FromDIP(wxSize(100, 20)));
	m_cluster_maxiter_sldr = new wxSingleSlider(page, wxID_ANY, 200, 1, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_cluster_maxiter_text = new wxTextCtrl(page, wxID_ANY, "200",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_int);
	m_cluster_maxiter_sldr->Bind(wxEVT_SCROLL_CHANGED, &ComponentDlg::OnClusterMaxiterSldr, this);
	m_cluster_maxiter_text->Bind(wxEVT_TEXT, &ComponentDlg::OnClusterMaxiterText, this);
	sizer22->Add(5, 5);
	sizer22->Add(st, 0, wxALIGN_CENTER);
	sizer22->Add(m_cluster_maxiter_sldr, 1, wxEXPAND);
	sizer22->Add(m_cluster_maxiter_text, 0, wxALIGN_CENTER);
	sizer22->Add(5, 5);
	//tol
	wxBoxSizer *sizer23 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Tolerance:",
		wxDefaultPosition, FromDIP(wxSize(100, 20)));
	m_cluster_tol_sldr = new wxSingleSlider(page, wxID_ANY, 0.90, 1, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_cluster_tol_text = new wxTextCtrl(page, wxID_ANY, "0.90",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_fp2);
	m_cluster_tol_sldr->Bind(wxEVT_SCROLL_CHANGED, &ComponentDlg::OnClusterTolSldr, this);
	m_cluster_tol_text->Bind(wxEVT_TEXT, &ComponentDlg::OnClusterTolText, this);
	sizer23->Add(5, 5);
	sizer23->Add(st, 0, wxALIGN_CENTER);
	sizer23->Add(m_cluster_tol_sldr, 1, wxEXPAND);
	sizer23->Add(m_cluster_tol_text, 0, wxALIGN_CENTER);
	sizer23->Add(5, 5);
	//size
	wxBoxSizer *sizer24 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Min. Size:",
		wxDefaultPosition, FromDIP(wxSize(100, 20)));
	m_cluster_size_sldr = new wxSingleSlider(page, wxID_ANY, 60, 1, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_cluster_size_text = new wxTextCtrl(page, wxID_ANY, "60",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_int);
	m_cluster_size_sldr->Bind(wxEVT_SCROLL_CHANGED, &ComponentDlg::OnClusterSizeSldr, this);
	m_cluster_size_text->Bind(wxEVT_TEXT, &ComponentDlg::OnClusterSizeText, this);
	sizer24->Add(5, 5);
	sizer24->Add(st, 0, wxALIGN_CENTER);
	sizer24->Add(m_cluster_size_sldr, 1, wxEXPAND);
	sizer24->Add(m_cluster_size_text, 0, wxALIGN_CENTER);
	sizer24->Add(5, 5);
	//eps
	wxBoxSizer *sizer25 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Neighborhood:",
		wxDefaultPosition, FromDIP(wxSize(100, 20)));
	m_cluster_eps_sldr = new wxSingleSlider(page, wxID_ANY, 25, 5, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_cluster_eps_text = new wxTextCtrl(page, wxID_ANY, "2.5",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_fp1);
	m_cluster_eps_sldr->Bind(wxEVT_SCROLL_CHANGED, &ComponentDlg::OnClusterEpsSldr, this);
	m_cluster_eps_text->Bind(wxEVT_TEXT, &ComponentDlg::OnClusterepsText, this);
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
	page->SetBackgroundColour(((wxNotebook*)parent)->GetThemeBackgroundColour());

	wxStaticText *st = 0;
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;

	//id tools
	wxBoxSizer *sizer1 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Selection and Modification by IDs"),
		wxVERTICAL);
	wxBoxSizer* sizer11 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "ID:",
		wxDefaultPosition, wxDefaultSize);
	m_comp_id_text = new wxTextCtrl(page, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(80, 23)), wxTE_PROCESS_ENTER | wxTE_RIGHT);
	m_comp_id_x_btn = new wxButton(page, wxID_ANY, "X",
		wxDefaultPosition, FromDIP(wxSize(23, 23)));
	//size limiters
	m_analysis_min_check = new wxCheckBox(page, wxID_ANY, "Min:",
		wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_analysis_min_spin = new wxSpinCtrl(page, wxID_ANY, "0",
		wxDefaultPosition, FromDIP(wxSize(80, 23)), wxSP_ARROW_KEYS, 0,
		std::numeric_limits<int>::max(), 0);
	m_analysis_max_check = new wxCheckBox(page, wxID_ANY, "Max:",
		wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_analysis_max_spin = new wxSpinCtrl(page, wxID_ANY, "100",
		wxDefaultPosition, FromDIP(wxSize(80, 23)), wxSP_ARROW_KEYS, 0,
		std::numeric_limits<int>::max(), 100);
	m_comp_id_text->Bind(wxEVT_TEXT, &ComponentDlg::OnCompIdText, this);
	m_comp_id_text->Bind(wxEVT_TEXT_ENTER, &ComponentDlg::OnCompFull, this);
	m_comp_id_x_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnCompIdXBtn, this);
	m_analysis_min_check->Bind(wxEVT_CHECKBOX, &ComponentDlg::OnAnalysisMinCheck, this);
	m_analysis_min_spin->Bind(wxEVT_SPINCTRL, &ComponentDlg::OnAnalysisMinSpin, this);
	m_analysis_min_spin->Bind(wxEVT_TEXT, &ComponentDlg::OnAnalysisMinText, this);
	m_analysis_max_check->Bind(wxEVT_CHECKBOX, &ComponentDlg::OnAnalysisMaxCheck, this);
	m_analysis_max_spin->Bind(wxEVT_SPINCTRL, &ComponentDlg::OnAnalysisMaxSpin, this);
	m_analysis_max_spin->Bind(wxEVT_TEXT, &ComponentDlg::OnAnalysisMaxText, this);
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
	//selection
	wxBoxSizer* sizer12 = new wxBoxSizer(wxHORIZONTAL);
	m_comp_append_btn = new wxButton(page, wxID_ANY, "Append",
		wxDefaultPosition, wxDefaultSize);
	m_comp_exclusive_btn = new wxButton(page, wxID_ANY, "Exclusive",
		wxDefaultPosition, wxDefaultSize);
	m_comp_all_btn = new wxButton(page, wxID_ANY, "AllVox",
		wxDefaultPosition, wxDefaultSize);
	m_comp_full_btn = new wxButton(page, wxID_ANY, "Fulfill",
		wxDefaultPosition, wxDefaultSize);
	m_comp_clear_btn = new wxButton(page, wxID_ANY, "Clear",
		wxDefaultPosition, wxDefaultSize);
	m_comp_append_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnCompAppend, this);
	m_comp_exclusive_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnCompExclusive, this);
	m_comp_all_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnCompAll, this);
	m_comp_full_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnCompFull, this);
	m_comp_clear_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnCompClear, this);
	sizer12->Add(5, 5);
	sizer12->Add(m_comp_append_btn, 1, wxEXPAND);
	sizer12->Add(m_comp_exclusive_btn, 1, wxEXPAND);
	sizer12->Add(m_comp_all_btn, 1, wxEXPAND);
	sizer12->Add(5, 5);
	sizer12->Add(m_comp_full_btn, 1, wxEXPAND);
	sizer12->Add(m_comp_clear_btn, 1, wxEXPAND);
	//modify
	wxBoxSizer* sizer13 = new wxBoxSizer(wxHORIZONTAL);
	m_comp_new_btn = new wxButton(page, wxID_ANY, "Assign",
		wxDefaultPosition, wxDefaultSize);
	m_comp_add_btn = new wxButton(page, wxID_ANY, "Add",
		wxDefaultPosition, wxDefaultSize);
	m_comp_replace_btn = new wxButton(page, wxID_ANY, "Replace",
		wxDefaultPosition, wxDefaultSize);
	m_comp_clean_bkg_btn = new wxButton(page, wxID_ANY, "Clear Paint",
		wxDefaultPosition, wxDefaultSize);
	m_comp_combine_btn = new wxButton(page, wxID_ANY, "Combine",
		wxDefaultPosition, wxDefaultSize);
	m_comp_new_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnCompNew, this);
	m_comp_add_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnCompAdd, this);
	m_comp_replace_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnCompReplace, this);
	m_comp_clean_bkg_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnCompCleanBkg, this);
	m_comp_combine_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnCompCombine, this);
	sizer13->Add(5, 5);
	sizer13->Add(m_comp_new_btn, 1, wxEXPAND);
	sizer13->Add(m_comp_add_btn, 1, wxEXPAND);
	sizer13->Add(m_comp_replace_btn, 1, wxEXPAND);
	sizer13->Add(5, 5);
	sizer13->Add(m_comp_clean_bkg_btn, 1, wxEXPAND);
	sizer13->Add(m_comp_combine_btn, 1, wxEXPAND);
	//
	sizer1->Add(10, 10);
	sizer1->Add(sizer11, 1, wxEXPAND);
	sizer1->Add(10, 10);
	sizer1->Add(sizer12, 1, wxEXPAND);
	sizer1->Add(10, 10);
	sizer1->Add(sizer13, 1, wxEXPAND);
	sizer1->Add(10, 10);

	//Options
	wxBoxSizer *sizer2 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Options"),
		wxVERTICAL);
	wxBoxSizer *sizer21 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Contact Size:",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_con_size_sldr = new wxSingleSlider(page, wxID_ANY, 5, 0, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_con_size_text = new wxTextCtrl(page, wxID_ANY, "5",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_int);
	m_con_size_sldr->Bind(wxEVT_SCROLL_CHANGED, &ComponentDlg::OnConSizeSldr, this);
	m_con_size_text->Bind(wxEVT_TEXT, &ComponentDlg::OnConSizeText, this);
	sizer21->Add(5, 5);
	sizer21->Add(st, 0, wxALIGN_CENTER);
	sizer21->Add(5, 5);
	sizer21->Add(m_con_size_sldr, 1, wxEXPAND);
	sizer21->Add(5, 5);
	sizer21->Add(m_con_size_text, 0, wxALIGN_CENTER);
	sizer21->Add(5, 5);
	wxBoxSizer *sizer22 = new wxBoxSizer(wxHORIZONTAL);
	m_consistent_check = new wxCheckBox(page, wxID_ANY, "Make color consistent for multiple bricks",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	sizer22->Add(5, 5);
	sizer22->Add(m_consistent_check, 0, wxALIGN_CENTER);
	wxBoxSizer *sizer23 = new wxBoxSizer(wxHORIZONTAL);
	m_colocal_check = new wxCheckBox(page, wxID_ANY, "Compute colocalization with other channels",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_consistent_check->Bind(wxEVT_CHECKBOX, &ComponentDlg::OnConsistentCheck, this);
	m_colocal_check->Bind(wxEVT_CHECKBOX, &ComponentDlg::OnColocalCheck, this);
	sizer23->Add(5, 5);
	sizer23->Add(m_colocal_check, 0, wxALIGN_CENTER);
	//
	sizer2->Add(10, 10);
	sizer2->Add(sizer21, 0, wxEXPAND);
	sizer2->Add(10, 10);
	sizer2->Add(sizer22, 0, wxEXPAND);
	sizer2->Add(10, 10);
	sizer2->Add(sizer23, 0, wxEXPAND);
	sizer2->Add(10, 10);

	//output
	wxBoxSizer *sizer3 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Output as New Channels"),
		wxVERTICAL);
	//radios
	wxBoxSizer *sizer31 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Channel Type:",
		wxDefaultPosition, FromDIP(wxSize(100, 20)));
	m_output_multi_rb = new wxRadioButton(page, ID_OutputMultiRb, "Each Comp.",
		wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	m_output_rgb_rb = new wxRadioButton(page, ID_OutputRgbRb, "R+G+B",
		wxDefaultPosition, wxDefaultSize);
	m_output_multi_rb->Bind(wxEVT_RADIOBUTTON, &ComponentDlg::OnOutputTypeRadio, this);
	m_output_rgb_rb->Bind(wxEVT_RADIOBUTTON, &ComponentDlg::OnOutputTypeRadio, this);
	sizer31->Add(5, 5);
	sizer31->Add(st, 0, wxALIGN_CENTER);
	sizer31->Add(m_output_multi_rb, 0, wxALIGN_CENTER);
	sizer31->Add(5, 5);
	sizer31->Add(m_output_rgb_rb, 0, wxALIGN_CENTER);
	sizer31->Add(5, 5);
	//buttons
	wxBoxSizer *sizer32 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Output:",
		wxDefaultPosition, FromDIP(wxSize(100, 20)));
	m_output_random_btn = new wxButton(page, ID_OutputRandomBtn, "Random Colors",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_output_size_btn = new wxButton(page, ID_OutputSizeBtn, "Size-based",
		wxDefaultPosition, FromDIP(wxSize(85, 23)));
	m_output_id_btn = new wxButton(page, ID_OutputIdBtn, "IDs",
		wxDefaultPosition, FromDIP(wxSize(65, 23)));
	m_output_sn_btn = new wxButton(page, ID_OutputSnBtn, "Serial No.",
		wxDefaultPosition, FromDIP(wxSize(75, 23)));
	m_output_random_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnOutputChannels, this);
	m_output_size_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnOutputChannels, this);
	m_output_id_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnOutputAnnotation, this);
	m_output_sn_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnOutputAnnotation, this);
	sizer32->Add(5, 5);
	sizer32->Add(st, 0, wxALIGN_CENTER);
	sizer32->Add(m_output_random_btn, 1, wxEXPAND);
	sizer32->Add(m_output_size_btn, 1, wxEXPAND);
	sizer32->Add(m_output_id_btn, 1, wxEXPAND);
	sizer32->Add(m_output_sn_btn, 1, wxEXPAND);
	sizer32->Add(5, 5);
	//
	sizer3->Add(10, 10);
	sizer3->Add(sizer31, 1, wxEXPAND);
	sizer3->Add(10, 10);
	sizer3->Add(sizer32, 1, wxEXPAND);
	sizer3->Add(10, 10);

	wxBoxSizer *sizer4 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Distances"),
		wxVERTICAL);
	wxBoxSizer *sizer41 = new wxBoxSizer(wxHORIZONTAL);
	m_dist_neighbor_check = new wxCheckBox(page, wxID_ANY, "Filter Nearest Neighbors",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_dist_all_chan_check = new wxCheckBox(page, wxID_ANY, "All Channel Results",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_dist_output_btn = new wxButton(page, wxID_ANY, "Compute",
		wxDefaultPosition, wxDefaultSize);
	m_dist_neighbor_check->Bind(wxEVT_CHECKBOX, &ComponentDlg::OnDistNeighborCheck, this);
	m_dist_all_chan_check->Bind(wxEVT_CHECKBOX, &ComponentDlg::OnDistAllChanCheck, this);
	m_dist_output_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnDistOutput, this);
	sizer41->Add(5, 5);
	sizer41->Add(m_dist_neighbor_check, 0, wxALIGN_CENTER);
	sizer41->Add(5, 5);
	sizer41->Add(m_dist_all_chan_check, 0, wxALIGN_CENTER);
	sizer41->AddStretchSpacer(1);
	sizer41->Add(m_dist_output_btn, 0, wxALIGN_CENTER);
	sizer41->Add(5, 5);
	wxBoxSizer *sizer42 = new wxBoxSizer(wxHORIZONTAL);
	m_dist_neighbor_sldr = new wxSingleSlider(page, wxID_ANY, 1, 1, 20,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_dist_neighbor_text = new wxTextCtrl(page, wxID_ANY, "1",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_int);
	m_dist_neighbor_sldr->Bind(wxEVT_SCROLL_CHANGED, &ComponentDlg::OnDistNeighborSldr, this);
	m_dist_neighbor_text->Bind(wxEVT_TEXT, &ComponentDlg::OnDistNeighborText, this);
	sizer42->Add(5, 5);
	sizer42->Add(m_dist_neighbor_sldr, 1, wxEXPAND);
	sizer42->Add(5, 5);
	sizer42->Add(m_dist_neighbor_text, 0, wxALIGN_CENTER);
	sizer42->Add(5, 5);
	//
	sizer4->Add(10, 10);
	sizer4->Add(sizer41, 0, wxEXPAND);
	sizer4->Add(10, 10);
	sizer4->Add(sizer42, 0, wxEXPAND);
	sizer4->Add(10, 10);

	//alignment
	wxBoxSizer *sizer5 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Align Render View to Analyzed Components"),
		wxVERTICAL);
	wxBoxSizer* sizer51 = new wxBoxSizer(wxHORIZONTAL);
	m_align_center = new wxCheckBox(page, wxID_ANY,
		"Move to Center", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_align_center->Bind(wxEVT_CHECKBOX, &ComponentDlg::OnAlignCenterChk, this);
	sizer51->Add(5, 5);
	sizer51->Add(m_align_center, 0, wxALIGN_CENTER);
	wxBoxSizer* sizer52 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Tri Axes:",
		wxDefaultPosition, FromDIP(wxSize(50, 22)));
	m_align_xyz = new wxButton(page, ID_AlignXYZ, "XYZ",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_align_yxz = new wxButton(page, ID_AlignYXZ, "YXZ",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_align_zxy = new wxButton(page, ID_AlignZXY, "ZXY",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_align_xzy = new wxButton(page, ID_AlignXZY, "XZY",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_align_yzx = new wxButton(page, ID_AlignYZX, "YZX",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_align_zyx = new wxButton(page, ID_AlignZYX, "ZYX",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_align_xyz->Bind(wxEVT_BUTTON, &ComponentDlg::OnAlignPca, this);
	m_align_yxz->Bind(wxEVT_BUTTON, &ComponentDlg::OnAlignPca, this);
	m_align_zxy->Bind(wxEVT_BUTTON, &ComponentDlg::OnAlignPca, this);
	m_align_xzy->Bind(wxEVT_BUTTON, &ComponentDlg::OnAlignPca, this);
	m_align_yzx->Bind(wxEVT_BUTTON, &ComponentDlg::OnAlignPca, this);
	m_align_zyx->Bind(wxEVT_BUTTON, &ComponentDlg::OnAlignPca, this);
	sizer52->Add(5, 5);
	sizer52->Add(st, 0, wxALIGN_CENTER);
	sizer52->Add(m_align_xyz, 1, wxEXPAND);
	sizer52->Add(m_align_yxz, 1, wxEXPAND);
	sizer52->Add(m_align_zxy, 1, wxEXPAND);
	sizer52->Add(m_align_xzy, 1, wxEXPAND);
	sizer52->Add(m_align_yzx, 1, wxEXPAND);
	sizer52->Add(m_align_zyx, 1, wxEXPAND);
	//
	sizer5->Add(5, 5);
	sizer5->Add(sizer51, 1, wxEXPAND);
	sizer5->Add(5, 5);
	sizer5->Add(sizer52, 1, wxEXPAND);
	sizer5->Add(5, 5);

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

	page->SetSizer(sizerv);
	page->SetScrollRate(10, 10);

	m_analysis_min_check->SetValue(false);
	m_analysis_min_spin->Disable();
	m_analysis_max_check->SetValue(false);
	m_analysis_max_spin->Disable();

	return page;
}

void ComponentDlg::FluoUpdate(const fluo::ValueCollection& vc)
{
	//update user interface
	if (FOUND_VALUE(gstNull))
		return;
	bool update_all = vc.empty();

	int ival;
	double dval;
	bool bval;
	unsigned long long uval;

	if (update_all || FOUND_VALUE(gstUseSelection))
		m_use_sel_chk->SetValue(glbin_comp_generator.GetUseSel());

	if (update_all || FOUND_VALUE(gstUseMachineLearning))
		m_use_ml_chk->SetValue(glbin_comp_generator.GetUseMl());

	if (update_all || FOUND_VALUE(gstAutoUpdate))
		m_auto_update_btn->SetValue(glbin_comp_def.m_auto_update);

	//comp generate page
	if (update_all || FOUND_VALUE(gstIteration))
	{
		ival = glbin_comp_generator.GetIter();
		m_iter_sldr->ChangeValue(ival);
		m_iter_text->ChangeValue(wxString::Format("%d", ival));
	}
	if (update_all || FOUND_VALUE(gstCompThreshold))
	{
		dval = glbin_comp_generator.GetThresh();
		m_thresh_sldr->ChangeValue(std::round(dval * 1000.0));
		m_thresh_text->ChangeValue(wxString::Format("%.3f", dval));
	}
	//diffusion
	if (update_all || FOUND_VALUE(gstUseDiffusion))
	{
		bval = glbin_comp_generator.GetDiffusion();
		m_diff_check->SetValue(bval);
		m_falloff_sldr->Enable(bval);
		m_falloff_text->Enable(bval);
	}
	if (update_all || FOUND_VALUE(gstDiffusionFalloff))
	{
		dval = glbin_comp_generator.GetFalloff();
		m_falloff_sldr->ChangeValue(std::round(dval * 1000.0));
		m_falloff_text->ChangeValue(wxString::Format("%.3f", dval));
	}
	//density
	if (update_all || FOUND_VALUE(gstUseDensityField))
	{
		bval = glbin_comp_generator.GetDensity();
		m_density_check->SetValue(bval);
		m_density_sldr->Enable(bval);
		m_density_text->Enable(bval);
		m_varth_sldr->Enable(bval);
		m_varth_text->Enable(bval);
		m_density_window_size_sldr->Enable(bval);
		m_density_window_size_text->Enable(bval);
		m_density_stats_size_sldr->Enable(bval);
		m_density_stats_size_text->Enable(bval);
	}
	if (update_all || FOUND_VALUE(gstDensityFieldThresh))
	{
		dval = glbin_comp_generator.GetDensityThresh();
		m_density_sldr->ChangeValue(std::round(dval * 1000.0));
		m_density_text->ChangeValue(wxString::Format("%.3f", dval));
	}
	if (update_all || FOUND_VALUE(gstDensityVarThresh))
	{
		dval = glbin_comp_generator.GetVarThresh();
		m_varth_sldr->ChangeValue(std::round(dval * 10000.0));
		m_varth_text->ChangeValue(wxString::Format("%.4f", dval));
	}
	if (update_all || FOUND_VALUE(gstDensityWindowSize))
	{
		ival = glbin_comp_generator.GetDensityWinSize();
		m_density_window_size_sldr->ChangeValue(dval);
		m_density_window_size_text->ChangeValue(wxString::Format("%d", dval));
	}
	if (update_all || FOUND_VALUE(gstDensityStatsSize))
	{
		ival = glbin_comp_generator.GetDensityStatSize();
		m_density_stats_size_sldr->ChangeValue(dval);
		m_density_stats_size_text->ChangeValue(wxString::Format("%d", dval));
	}
	//dist
	if (update_all || FOUND_VALUE(gstUseDistField))
	{
		bval = glbin_comp_generator.GetUseDistField();
		m_use_dist_field_check->SetValue(bval);
		m_dist_strength_sldr->Enable(bval);
		m_dist_strength_text->Enable(bval);
		m_dist_filter_size_sldr->Enable(bval);
		m_dist_filter_size_text->Enable(bval);
		m_max_dist_sldr->Enable(bval);
		m_max_dist_text->Enable(bval);
		m_dist_thresh_sldr->Enable(bval);
		m_dist_thresh_text->Enable(bval);
	}
	if (update_all || FOUND_VALUE(gstDistFieldStrength))
	{
		dval = glbin_comp_generator.GetDistStrength();
		m_dist_strength_sldr->ChangeValue(std::round(dval * 1000.0));
		m_dist_strength_text->ChangeValue(wxString::Format("%.3f", dval));
	}
	if (update_all || FOUND_VALUE(gstDistFieldFilterSize))
	{
		ival = glbin_comp_generator.GetDistFilterSize();
		m_dist_filter_size_sldr->ChangeValue(dval);
		m_dist_filter_size_text->ChangeValue(wxString::Format("%d", dval));
	}
	if (update_all || FOUND_VALUE(gstMaxDist))
	{
		ival = glbin_comp_generator.GetMaxDist();
		m_max_dist_sldr->ChangeValue(dval);
		m_max_dist_text->ChangeValue(wxString::Format("%d", dval));
	}
	if (update_all || FOUND_VALUE(gstDistFieldThresh))
	{
		dval = glbin_comp_generator.GetDistThresh();
		m_dist_thresh_sldr->ChangeValue(std::round(dval * 1000.0));
		m_dist_thresh_text->ChangeValue(wxString::Format("%.3f", dval));
	}

	//fixate
	if (update_all || FOUND_VALUE(gstFixateEnable))
	{
		bval = glbin_comp_generator.GetFixate();
		m_fixate_check->SetValue(bval);
		m_grow_fixed_check->Enable(bval);
		m_fix_update_btn->Enable(bval);
		m_fix_size_sldr->Enable(bval);
		m_fix_size_text->Enable(bval);
	}
	if (update_all || FOUND_VALUE(gstGrowFixed))
	{
		bval = glbin_comp_generator.GetGrowFixed();
		m_grow_fixed_check->SetValue(bval);
	}
	if (update_all || FOUND_VALUE(gstFixateSize))
	{
		ival = glbin_comp_generator.GetFixSize();
		m_fix_size_sldr->ChangeValue(ival);
		m_fix_size_text->ChangeValue(wxString::Format("%d", ival));
	}
	//clean
	if (update_all || FOUND_VALUE(gstCleanEnable))
	{
		bval = glbin_comp_generator.GetClean();
		m_clean_check->SetValue(bval);
		m_clean_btn->Enable(bval);
		m_clean_iter_sldr->Enable(bval);
		m_clean_iter_text->Enable(bval);
		m_clean_limit_sldr->Enable(bval);
		m_clean_limit_text->Enable(bval);
	}
	if (update_all || FOUND_VALUE(gstCleanIteration))
	{
		ival = glbin_comp_generator.GetCleanIter();
		m_clean_iter_sldr->ChangeValue(ival);
		m_clean_iter_text->ChangeValue(wxString::Format("%d", ival));
	}
	if (update_all || FOUND_VALUE(gstCleanSize))
	{
		ival = glbin_comp_generator.GetCleanSize();
		m_clean_limit_sldr->ChangeValue(ival);
		m_clean_limit_text->ChangeValue(wxString::Format("%d", ival));
	}
	//record
	if (update_all || FOUND_VALUE(gstRecordCmd))
	{
		ival = glbin_comp_generator.GetCmdNum();
		m_cmd_count_text->ChangeValue(wxString::Format("%d", ival));
		bval = glbin_comp_generator.GetRecordCmd();
		m_record_cmd_btn->SetValue(bval);
	}

	//cluster page
	if (update_all || FOUND_VALUE(gstClusterMethod))
	{
		ival = glbin_clusterizer.GetMethod();
		m_cluster_method_exmax_rd->SetValue(ival == 0);
		m_cluster_method_dbscan_rd->SetValue(ival == 1);
		m_cluster_method_kmeans_rd->SetValue(ival == 2);
		m_cluster_clnum_sldr->Enable(ival == 0 || ival == 2);
		m_cluster_clnum_text->Enable(ival == 0 || ival == 2);
		m_cluster_size_sldr->Enable(ival == 1);
		m_cluster_size_text->Enable(ival == 1);
		m_cluster_eps_sldr->Enable(ival == 1);
		m_cluster_eps_text->Enable(ival == 1);
	}
	//parameters
	if (update_all || FOUND_VALUE(gstClusterNum))
	{
		ival = glbin_clusterizer.GetNum();
		m_cluster_clnum_sldr->ChangeValue(ival);
		m_cluster_clnum_text->ChangeValue(wxString::Format("%d", ival));
	}
	if (update_all || FOUND_VALUE(gstClusterMaxIter))
	{
		ival = glbin_clusterizer.GetMaxIter();
		m_cluster_maxiter_sldr->ChangeValue(ival);
		m_cluster_maxiter_text->ChangeValue(wxString::Format("%d", ival));
	}
	if (update_all || FOUND_VALUE(gstClusterTol))
	{
		dval = glbin_clusterizer.GetTol();
		m_cluster_tol_sldr->ChangeValue(std::round(dval * 100));
		m_cluster_tol_text->ChangeValue(wxString::Format("%.2f", dval));
	}
	if (update_all || FOUND_VALUE(gstClusterSize))
	{
		ival = glbin_clusterizer.GetSize();
		m_cluster_size_sldr->ChangeValue(ival);
		m_cluster_size_text->ChangeValue(wxString::Format("%d", ival));
	}
	if (update_all || FOUND_VALUE(gstClusterEps))
	{
		ival = glbin_clusterizer.GetEps();
		m_cluster_eps_sldr->ChangeValue(std::round(dval * 10.0));
		m_cluster_eps_text->SetValue(wxString::Format("%.1f", dval));
	}

	//analysis page
	//id text
	if (update_all || FOUND_VALUE(gstCompIdColor))
	{
		wxColor color = glbin_comp_editor.GetWxColor();
		m_comp_id_text->SetBackgroundColour(color);
		m_comp_id_text->Refresh();
	}
	//size limiters
	if (update_all || FOUND_VALUE(gstUseMin))
	{
		bval = glbin_comp_selector.GetUseMin();
		m_analysis_min_check->SetValue(bval);
		m_analysis_min_spin->Enable(bval);
	}
	if (update_all || FOUND_VALUE(gstMinValue))
	{
		ival = glbin_comp_selector.GetMinNum();
		m_analysis_min_spin->SetValue(ival);
	}
	if (update_all || FOUND_VALUE(gstUseMax))
	{
		bval = glbin_comp_selector.GetUseMax();
		m_analysis_max_check->SetValue(bval);
		m_analysis_max_spin->Enable(bval);
	}
	if (update_all || FOUND_VALUE(gstMaxValue))
	{
		ival = glbin_comp_selector.GetMaxNum();
		m_analysis_max_spin->SetValue(ival);
	}

	//options
	if (update_all || FOUND_VALUE(gstCompConsistent))
	{
		bval = glbin_comp_analyzer.GetConsistent();
		m_consistent_check->SetValue(bval);
	}
	if (update_all || FOUND_VALUE(gstCompColocal))
	{
		bval = glbin_comp_analyzer.GetColocal();
		m_colocal_check->SetValue(bval);
	}

	//output type
	if (update_all || FOUND_VALUE(gstCompOutputType))
	{
		ival = glbin_comp_analyzer.GetColorType();
		m_output_multi_rb->SetValue(ival == 1);
		m_output_rgb_rb->SetValue(ival == 2);
	}

	//Distances
	if (update_all || FOUND_VALUE(gstDistNeighbor))
	{
		bval = glbin_comp_def.m_use_dist_neighbor;
		m_dist_neighbor_check->SetValue(bval);
		m_dist_neighbor_sldr->Enable(bval);
		m_dist_neighbor_text->Enable(bval);
	}
	if (update_all || FOUND_VALUE(gstDistNeighborValue))
	{
		ival = glbin_comp_def.m_dist_neighbor;
		m_dist_neighbor_sldr->ChangeValue(ival);
		m_dist_neighbor_text->ChangeValue(wxString::Format("%d", ival));
	}
	if (update_all || FOUND_VALUE(gstDistAllChan))
		m_dist_all_chan_check->SetValue(glbin_comp_def.m_use_dist_allchan);

	//page
	if (update_all || FOUND_VALUE(gstCompPage))
	{
		ival = m_notebook->GetSelection();
		m_use_sel_chk->Show(ival == 0);
		m_use_ml_chk->Show(ival == 0);
		m_generate_btn->Show(ival == 0);
		m_auto_update_btn->Show(ival == 0);
		m_cluster_btn->Show(ival == 1);
		m_analyze_btn->Show(ival == 2);
		m_analyze_sel_btn->Show(ival == 2);

		panel_top->Layout();
		panel_bot->Layout();
	}

	////output
	//m_history_chk->SetValue(m_hold_history);
}

//comp generate page
void ComponentDlg::OnIterSldr(wxScrollEvent &event)
{
	int val = m_iter_sldr->GetValue();
	wxString str = wxString::Format("%d", val);
	if (str != m_iter_text->GetValue())
		m_iter_text->SetValue(str);
}

void ComponentDlg::OnIterText(wxCommandEvent &event)
{
	long val = 0;
	m_iter_text->GetValue().ToLong(&val);
	m_iter_sldr->ChangeValue(val);
	glbin_comp_generator.SetIter(val);

	if (glbin_comp_def.m_auto_update)
	{
		glbin_comp_generator.GenerateComp();
		FluoRefresh(3, { gstNull });
	}
}

void ComponentDlg::OnThreshSldr(wxScrollEvent &event)
{
	double val = m_thresh_sldr->GetValue() / 1000.0;
	wxString str = wxString::Format("%.3f", val);
	if (str != m_thresh_text->GetValue())
		m_thresh_text->SetValue(str);
}

void ComponentDlg::OnThreshText(wxCommandEvent &event)
{
	double val = 0.0;
	m_thresh_text->GetValue().ToDouble(&val);
	m_thresh_sldr->ChangeValue(std::round(val * 1000.0));
	glbin_comp_generator.SetThresh(val);

	if (glbin_comp_def.m_auto_update)
	{
		glbin_comp_generator.GenerateComp();
		FluoRefresh(3, { gstNull });
	}
}

void ComponentDlg::OnDistStrengthSldr(wxScrollEvent &event)
{
	double val = m_dist_strength_sldr->GetValue() / 1000.0;
	wxString str = wxString::Format("%.3f", val);
	if (str != m_dist_strength_text->GetValue())
		m_dist_strength_text->SetValue(str);
}

void ComponentDlg::OnDistStrengthText(wxCommandEvent &event)
{
	double val = 0.0;
	m_dist_strength_text->GetValue().ToDouble(&val);
	m_dist_strength_sldr->ChangeValue(std::round(val * 1000.0));
	glbin_comp_generator.SetDistStrength(val);

	if (glbin_comp_def.m_auto_update)
	{
		glbin_comp_generator.GenerateComp();
		FluoRefresh(3, { gstNull });
	}
}

void ComponentDlg::OnUseDistFieldCheck(wxCommandEvent &event)
{
	bool bval = m_use_dist_field_check->GetValue();
	glbin_comp_generator.SetUseDistField(bval);

	if (glbin_comp_def.m_auto_update)
	{
		glbin_comp_generator.GenerateComp();
		FluoRefresh(2, { gstUseDistField });
	}
	else
		FluoUpdate({ gstUseDistField });
}

void ComponentDlg::OnDistFilterSizeSldr(wxScrollEvent &event)
{
	int val = m_dist_filter_size_sldr->GetValue();
	wxString str = wxString::Format("%d", val);
	if (str != m_dist_filter_size_text->GetValue())
		m_dist_filter_size_text->SetValue(str);
}

void ComponentDlg::OnDistFitlerSizeText(wxCommandEvent &event)
{
	long val = 0;
	m_dist_filter_size_text->GetValue().ToLong(&val);
	m_dist_filter_size_sldr->ChangeValue(val);
	glbin_comp_generator.SetDistFilterSize(val);

	if (glbin_comp_def.m_auto_update)
	{
		glbin_comp_generator.GenerateComp();
		FluoRefresh(3, { gstNull });
	}
}

void ComponentDlg::OnMaxDistSldr(wxScrollEvent &event)
{
	int val = m_max_dist_sldr->GetValue();
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
	m_max_dist_sldr->ChangeValue(val);
	glbin_comp_generator.SetMaxDist(val);

	if (glbin_comp_def.m_auto_update)
	{
		glbin_comp_generator.GenerateComp();
		FluoRefresh(3, { gstNull });
	}
}

void ComponentDlg::OnDistThreshSldr(wxScrollEvent &event)
{
	double val = m_dist_thresh_sldr->GetValue() / 1000.0;
	wxString str = wxString::Format("%.3f", val);
	if (str != m_dist_thresh_text->GetValue())
		m_dist_thresh_text->SetValue(str);
}

void ComponentDlg::OnDistThreshText(wxCommandEvent &event)
{
	double val = 0.0;
	m_dist_thresh_text->GetValue().ToDouble(&val);
	m_dist_thresh_sldr->ChangeValue(std::round(val * 1000.0));
	glbin_comp_generator.SetDistThresh(val);

	if (glbin_comp_def.m_auto_update)
	{
		glbin_comp_generator.GenerateComp();
		FluoRefresh(3, { gstNull });
	}
}

void ComponentDlg::OnDiffCheck(wxCommandEvent &event)
{
	bool bval = m_diff_check->GetValue();
	glbin_comp_generator.SetDiffusion(bval);

	if (glbin_comp_def.m_auto_update)
	{
		glbin_comp_generator.GenerateComp();
		FluoRefresh(2, { gstUseDiffusion });
	}
	else
		FluoUpdate({ gstUseDiffusion });
}

void ComponentDlg::OnFalloffSldr(wxScrollEvent &event)
{
	double val = m_falloff_sldr->GetValue() / 1000.0;
	wxString str = wxString::Format("%.3f", val);
	if (str != m_falloff_text->GetValue())
		m_falloff_text->SetValue(str);
}

void ComponentDlg::OnFalloffText(wxCommandEvent &event)
{
	double val = 0.0;
	m_falloff_text->GetValue().ToDouble(&val);
	m_falloff_sldr->ChangeValue(std::round(val * 1000.0));
	glbin_comp_generator.SetFalloff(val);

	if (glbin_comp_def.m_auto_update)
	{
		glbin_comp_generator.GenerateComp();
		FluoRefresh(3, { gstNull });
	}
}

void ComponentDlg::OnDensityCheck(wxCommandEvent &event)
{
	bool bval = m_density_check->GetValue();
	glbin_comp_generator.SetDensity(bval);

	if (glbin_comp_def.m_auto_update)
	{
		glbin_comp_generator.GenerateComp();
		FluoRefresh(2, { gstUseDensityField });
	}
	else
		FluoUpdate({ gstUseDensityField });
}

void ComponentDlg::OnDensitySldr(wxScrollEvent &event)
{
	double val = m_density_sldr->GetValue() / 1000.0;
	wxString str = wxString::Format("%.3f", val);
	if (str != m_density_text->GetValue())
		m_density_text->SetValue(str);
}

void ComponentDlg::OnDensityText(wxCommandEvent &event)
{
	double val = 0.0;
	m_density_text->GetValue().ToDouble(&val);
	m_density_sldr->ChangeValue(std::round(val * 1000.0));
	glbin_comp_generator.SetDensityThresh(val);

	if (glbin_comp_def.m_auto_update)
	{
		glbin_comp_generator.GenerateComp();
		FluoRefresh(3, { gstNull });
	}
}

void ComponentDlg::OnVarthSldr(wxScrollEvent &event)
{
	double val = m_varth_sldr->GetValue() / 10000.0;
	wxString str = wxString::Format("%.4f", val);
	if (str != m_varth_text->GetValue())
		m_varth_text->SetValue(str);
}

void ComponentDlg::OnVarthText(wxCommandEvent &event)
{
	double val = 0.0;
	m_varth_text->GetValue().ToDouble(&val);
	m_varth_sldr->ChangeValue(std::round(val * 10000.0));
	glbin_comp_generator.SetVarThresh(val);

	if (glbin_comp_def.m_auto_update)
	{
		glbin_comp_generator.GenerateComp();
		FluoRefresh(3, { gstNull });
	}
}

void ComponentDlg::OnDensityWindowSizeSldr(wxScrollEvent &event)
{
	int val = m_density_window_size_sldr->GetValue();
	wxString str = wxString::Format("%d", val);
	if (str != m_density_window_size_text->GetValue())
		m_density_window_size_text->SetValue(str);
}

void ComponentDlg::OnDensityWindowSizeText(wxCommandEvent &event)
{
	long val = 0;
	m_density_window_size_text->GetValue().ToLong(&val);
	m_density_window_size_sldr->ChangeValue(val);
	glbin_comp_generator.SetDensityWinSize(val);

	if (glbin_comp_def.m_auto_update)
	{
		glbin_comp_generator.GenerateComp();
		FluoRefresh(3, { gstNull });
	}
}

void ComponentDlg::OnDensityStatsSizeSldr(wxScrollEvent &event)
{
	int val = m_density_stats_size_sldr->GetValue();
	wxString str = wxString::Format("%d", val);
	if (str != m_density_stats_size_text->GetValue())
		m_density_stats_size_text->SetValue(str);
}

void ComponentDlg::OnDensityStatsSizeText(wxCommandEvent &event)
{
	long val = 0;
	m_density_stats_size_text->GetValue().ToLong(&val);
	m_density_stats_size_sldr->ChangeValue(val);
	glbin_comp_generator.SetDensityStatSize(val);

	if (glbin_comp_def.m_auto_update)
	{
		glbin_comp_generator.GenerateComp();
		FluoRefresh(3, { gstNull });
	}
}

void ComponentDlg::OnFixateCheck(wxCommandEvent &event)
{
	bool bval = m_fixate_check->GetValue();
	glbin_comp_generator.SetFixate(bval);

	if (bval)
		glbin_comp_generator.Fixate();

	if (glbin_comp_def.m_auto_update)
	{
		bval = glbin_comp_generator.GetClean();
		glbin_comp_generator.SetClean(false);
		glbin_comp_generator.GenerateComp(false);
		glbin_comp_generator.SetClean(bval);
		FluoRefresh(2, { gstFixateEnable });
	}
	else
		FluoUpdate({ gstFixateEnable });
}

void ComponentDlg::OnGrowFixedCheck(wxCommandEvent &event)
{
	bool bval = m_grow_fixed_check->GetValue();
	glbin_comp_generator.SetGrowFixed(bval);

	if (glbin_comp_def.m_auto_update)
	{
		bval = glbin_comp_generator.GetClean();
		glbin_comp_generator.SetClean(false);
		glbin_comp_generator.GenerateComp(false);
		glbin_comp_generator.SetClean(bval);
		FluoRefresh(3, { gstNull });
	}
}

void ComponentDlg::OnFixUpdateBtn(wxCommandEvent &event)
{
	glbin_comp_generator.Fixate();

	if (glbin_comp_def.m_auto_update)
	{
		bool bval = glbin_comp_generator.GetClean();
		glbin_comp_generator.SetClean(false);
		glbin_comp_generator.GenerateComp(false);
		glbin_comp_generator.SetClean(bval);
		FluoRefresh(3, { gstNull });
	}
}

void ComponentDlg::OnFixSizeSldr(wxScrollEvent &event)
{
	int val = m_fix_size_sldr->GetValue();
	wxString str = wxString::Format("%d", val);
	if (str != m_fix_size_text->GetValue())
		m_fix_size_text->SetValue(str);
}

void ComponentDlg::OnFixSizeText(wxCommandEvent &event)
{
	long val = 0;
	m_fix_size_text->GetValue().ToLong(&val);
	m_fix_size_sldr->ChangeValue(val);
	glbin_comp_generator.SetFixSize(val);

	if (glbin_comp_def.m_auto_update)
		glbin_comp_generator.GenerateComp(false);
	if (glbin_comp_generator.GetRecordCmd())
		glbin_comp_generator.AddCmd("fixate");
	FluoRefresh(2, { gstRecordCmd });
}

void ComponentDlg::OnCleanCheck(wxCommandEvent &event)
{
	bool bval = m_clean_check->GetValue();
	glbin_comp_generator.SetClean(bval);

	if (glbin_comp_def.m_auto_update)
	{
		glbin_comp_generator.GenerateComp();
		FluoRefresh(3, { gstNull });
	}
}

void ComponentDlg::OnCleanIterSldr(wxScrollEvent &event)
{
	int val = m_clean_iter_sldr->GetValue();
	wxString str = wxString::Format("%d", val);
	if (str != m_clean_iter_text->GetValue())
		m_clean_iter_text->SetValue(str);
}

void ComponentDlg::OnCleanIterText(wxCommandEvent &event)
{
	long val = 0;
	m_clean_iter_text->GetValue().ToLong(&val);
	m_clean_iter_sldr->ChangeValue(val);
	glbin_comp_generator.SetCleanIter(val);

	if (glbin_comp_def.m_auto_update)
	{
		glbin_comp_generator.GenerateComp();
		FluoRefresh(3, { gstNull });
	}
}

void ComponentDlg::OnCleanLimitSldr(wxScrollEvent &event)
{
	int val = m_clean_limit_sldr->GetValue();
	wxString str = wxString::Format("%d", val);
	if (str != m_clean_limit_text->GetValue())
		m_clean_limit_text->SetValue(str);
}

void ComponentDlg::OnCleanLimitText(wxCommandEvent &event)
{
	long val = 0;
	m_clean_limit_text->GetValue().ToLong(&val);
	m_clean_limit_sldr->ChangeValue(val);
	glbin_comp_generator.SetCleanSize(val);

	if (glbin_comp_def.m_auto_update)
	{
		glbin_comp_generator.GenerateComp();
		FluoRefresh(3, { gstNull });
	}
}

//record
void ComponentDlg::OnRecordCmd(wxCommandEvent &event)
{
	bool val = m_record_cmd_btn->GetValue();
	glbin_comp_generator.SetRecordCmd(val);
}

void ComponentDlg::OnPlayCmd(wxCommandEvent &event)
{
	glbin_comp_generator.PlayCmd(1.0);
}

void ComponentDlg::OnResetCmd(wxCommandEvent &event)
{
	glbin_comp_generator.ResetCmd();
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

	glbin_comp_generator.LoadCmd(filename);
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

	glbin_comp_generator.SaveCmd(filename);
}

//clustering page
void ComponentDlg::OnClusterMethodCheck(wxCommandEvent &event)
{
	int id = event.GetId();
	glbin_clusterizer.SetMethod(id);
	FluoUpdate({ gstClusterMethod });
}

//parameters
void ComponentDlg::OnClusterClnumSldr(wxScrollEvent &event)
{
	int val = m_cluster_clnum_sldr->GetValue();
	wxString str = wxString::Format("%d", val);
	if (str != m_cluster_clnum_text->GetValue())
		m_cluster_clnum_text->SetValue(str);
}

void ComponentDlg::OnClusterClnumText(wxCommandEvent &event)
{
	long val = 0;
	m_cluster_clnum_text->GetValue().ToLong(&val);
	m_cluster_clnum_sldr->ChangeValue(val);
	glbin_clusterizer.SetNum(val);
}

void ComponentDlg::OnClusterMaxiterSldr(wxScrollEvent &event)
{
	int val = m_cluster_maxiter_sldr->GetValue();
	wxString str = wxString::Format("%d", val);
	if (str != m_cluster_maxiter_text->GetValue())
		m_cluster_maxiter_text->SetValue(str);
}

void ComponentDlg::OnClusterMaxiterText(wxCommandEvent &event)
{
	long val = 0;
	m_cluster_maxiter_text->GetValue().ToLong(&val);
	m_cluster_maxiter_sldr->ChangeValue(val);
	glbin_clusterizer.SetMaxIter(val);
}

void ComponentDlg::OnClusterTolSldr(wxScrollEvent &event)
{
	int val = m_cluster_tol_sldr->GetValue();
	wxString str = wxString::Format("%.2f", double(val) / 100.0);
	if (str != m_cluster_tol_text->GetValue())
		m_cluster_tol_text->SetValue(str);
}

void ComponentDlg::OnClusterTolText(wxCommandEvent &event)
{
	double val = 0.9;
	m_cluster_tol_text->GetValue().ToDouble(&val);
	m_cluster_tol_sldr->ChangeValue(std::round(val * 100));
	glbin_clusterizer.SetTol((float)val);
}

void ComponentDlg::OnClusterSizeSldr(wxScrollEvent &event)
{
	int val = m_cluster_size_sldr->GetValue();
	wxString str = wxString::Format("%d", val);
	if (str != m_cluster_size_text->GetValue())
		m_cluster_size_text->SetValue(str);
}

void ComponentDlg::OnClusterSizeText(wxCommandEvent &event)
{
	long val = 0;
	m_cluster_size_text->GetValue().ToLong(&val);
	m_cluster_size_sldr->ChangeValue(val);
	glbin_clusterizer.SetSize(val);
}

void ComponentDlg::OnClusterEpsSldr(wxScrollEvent &event)
{
	double val = m_cluster_eps_sldr->GetValue() / 10.0;
	wxString str = wxString::Format("%.1f", val);
	if (str != m_cluster_eps_text->GetValue())
		m_cluster_eps_text->SetValue(str);
}

void ComponentDlg::OnClusterepsText(wxCommandEvent &event)
{
	double val = 0.0;
	m_cluster_eps_text->GetValue().ToDouble(&val);
	m_cluster_eps_sldr->ChangeValue(std::round(val * 10.0));
	glbin_clusterizer.SetEps(val);
}

//analysis page
void ComponentDlg::OnCompIdText(wxCommandEvent &event)
{
	unsigned long id;
	wxString str = m_comp_id_text->GetValue();
	glbin_comp_selector.SetId(str.ToStdString());
	if (str.ToULong(&id))
		glbin_comp_editor.SetId(id, false);
	else
		glbin_comp_editor.SetId(0, true);
	FluoUpdate({ gstCompIdColor });
}

void ComponentDlg::OnCompIdXBtn(wxCommandEvent &event)
{
	m_comp_id_text->Clear();
	FluoUpdate({ gstCompIdColor });
}

void ComponentDlg::OnAnalysisMinCheck(wxCommandEvent &event)
{
	bool bval = m_analysis_min_check->GetValue();
	m_analysis_min_spin->Enable(bval);
	glbin_comp_selector.SetUseMin(bval);
	FluoUpdate({ gstUseMin });
}

void ComponentDlg::OnAnalysisMinSpin(wxSpinEvent &event)
{
	int val = m_analysis_min_spin->GetValue();
	glbin_comp_selector.SetMinNum(val);
}

void ComponentDlg::OnAnalysisMinText(wxCommandEvent &event)
{
	int val = m_analysis_min_spin->GetValue();
	glbin_comp_selector.SetMinNum(val);
}

void ComponentDlg::OnAnalysisMaxCheck(wxCommandEvent &event)
{
	bool bval = m_analysis_max_check->GetValue();
	m_analysis_max_spin->Enable(bval);
	glbin_comp_selector.SetUseMax(bval);
	FluoUpdate({ gstUseMax });
}

void ComponentDlg::OnAnalysisMaxSpin(wxSpinEvent &event)
{
	int val = m_analysis_max_spin->GetValue();
	glbin_comp_selector.SetMaxNum(val);
}

void ComponentDlg::OnAnalysisMaxText(wxCommandEvent &event)
{
	int val = m_analysis_max_spin->GetValue();
	glbin_comp_selector.SetMaxNum(val);
}

void ComponentDlg::OnCompFull(wxCommandEvent &event)
{
	glbin_comp_selector.SelectFullComp();
	FluoRefresh(0, { gstComp, gstSelUndo });
}

void ComponentDlg::OnCompExclusive(wxCommandEvent &event)
{
	glbin_comp_selector.Exclusive();
	FluoRefresh(0, { gstComp, gstSelUndo });

	//frame
	//if (m_frame)
	//{
	//	if (m_frame->GetBrushToolDlg())
	//	{
	//		if (m_view->m_paint_count)
	//			m_frame->GetBrushToolDlg()->Update(0);
	//		m_frame->GetBrushToolDlg()->UpdateUndoRedo();
	//	}
	//	if (m_frame->GetColocalizationDlg() &&
	//		m_view->m_paint_colocalize)
	//		m_frame->GetColocalizationDlg()->Colocalize();
	//}
}

void ComponentDlg::OnCompAppend(wxCommandEvent &event)
{
	bool get_all = glbin_comp_selector.GetIdEmpty();
	glbin_comp_selector.Select(get_all);
	FluoRefresh(0, { gstComp, gstSelUndo });
	//frame
	//if (m_frame)
	//{
	//	if (m_frame->GetBrushToolDlg())
	//	{
	//		if (m_view->m_paint_count)
	//			m_frame->GetBrushToolDlg()->Update(0);
	//		m_frame->GetBrushToolDlg()->UpdateUndoRedo();
	//	}
	//	if (m_frame->GetColocalizationDlg() &&
	//		m_view->m_paint_colocalize)
	//		m_frame->GetColocalizationDlg()->Colocalize();
	//}
}

void ComponentDlg::OnCompAll(wxCommandEvent &event)
{
	glbin_comp_selector.All();
	FluoRefresh(0, { gstComp, gstSelUndo });

	//frame
	//if (m_frame)
	//{
	//	if (m_frame->GetBrushToolDlg())
	//	{
	//		if (m_view->m_paint_count)
	//			m_frame->GetBrushToolDlg()->Update(0);
	//		m_frame->GetBrushToolDlg()->UpdateUndoRedo();
	//	}
	//	if (m_frame->GetColocalizationDlg() &&
	//		m_view->m_paint_colocalize)
	//		m_frame->GetColocalizationDlg()->Colocalize();
	//}
}

void ComponentDlg::OnCompClear(wxCommandEvent &event)
{
	glbin_comp_selector.Clear();
	FluoRefresh(0, { gstComp, gstSelUndo });
}

void ComponentDlg::OnShuffle(wxCommandEvent &event)
{
	//get current vd
	VolumeData* vd = glbin_current.vol_data;
	if (!vd)
		return;

	vd->IncShuffle();
	FluoRefresh(3, { gstNull });
}

void ComponentDlg::OnCompNew(wxCommandEvent& event)
{
	glbin_comp_editor.NewId(false, false);
	FluoRefresh(3, { gstNull });
}

void ComponentDlg::OnCompAdd(wxCommandEvent& event)
{
	glbin_comp_editor.NewId(true, false);
	FluoRefresh(3, { gstNull });
}

void ComponentDlg::OnCompReplace(wxCommandEvent& event)
{
	glbin_comp_editor.Replace();
	FluoRefresh(3, { gstNull });
}

void ComponentDlg::OnCompCleanBkg(wxCommandEvent& event)
{
	glbin_comp_editor.Clean(0);
	FluoRefresh(3, { gstNull });
}

void ComponentDlg::OnCompCombine(wxCommandEvent& event)
{
	glbin_comp_editor.Combine();
	FluoRefresh(3, { gstNull });
}

void ComponentDlg::OnConSizeSldr(wxScrollEvent &event)
{
	int val = m_con_size_sldr->GetValue();
	wxString str = wxString::Format("%d", val);
	if (str != m_con_size_text->GetValue())
		m_con_size_text->SetValue(str);
}

void ComponentDlg::OnConSizeText(wxCommandEvent &event)
{
	long val = 0;
	m_con_size_text->GetValue().ToLong(&val);
	m_con_size_sldr->ChangeValue(val);
	glbin_comp_analyzer.SetSizeLimit(val);
}

void ComponentDlg::OnConsistentCheck(wxCommandEvent &event)
{
	bool bval = m_consistent_check->GetValue();
	glbin_comp_analyzer.SetConsistent(bval);
}

void ComponentDlg::OnColocalCheck(wxCommandEvent &event)
{
	bool bval = m_colocal_check->GetValue();
	glbin_comp_analyzer.SetColocal(bval);
}

//output
void ComponentDlg::OnOutputTypeRadio(wxCommandEvent &event)
{
	int id = event.GetId();
	switch (id)
	{
	case ID_OutputMultiRb:
		glbin_comp_analyzer.SetChannelType(1);
		break;
	case ID_OutputRgbRb:
		glbin_comp_analyzer.SetChannelType(2);
		break;
	}
}

void ComponentDlg::OnOutputChannels(wxCommandEvent& event)
{
	int id = event.GetId();
	int val;
	if (id == ID_OutputRandomBtn)
		val = 1;
	else if (id == ID_OutputSizeBtn)
		val = 2;

	glbin_comp_analyzer.SetColorType(val);
	glbin_comp_analyzer.OutputChannels();
	FluoRefresh(0, { gstListCtrl, gstTreeCtrl });
}

void ComponentDlg::OnOutputAnnotation(wxCommandEvent &event)
{
	int id = event.GetId();
	int val;
	if (id == ID_OutputIdBtn)
		val = 1;
	else if (id == ID_OutputSnBtn)
		val = 2;

	glbin_comp_analyzer.SetAnnotType(val);
	glbin_comp_analyzer.OutputAnnotations();
	FluoRefresh(0, { gstListCtrl, gstTreeCtrl });
}

//distance
void ComponentDlg::OnDistNeighborCheck(wxCommandEvent &event)
{
	glbin_comp_def.m_use_dist_neighbor = m_dist_neighbor_check->GetValue();
	m_dist_neighbor_sldr->Enable(glbin_comp_def.m_use_dist_neighbor);
	m_dist_neighbor_text->Enable(glbin_comp_def.m_use_dist_neighbor);
}

void ComponentDlg::OnDistAllChanCheck(wxCommandEvent &event)
{
	glbin_comp_def.m_use_dist_allchan = m_dist_all_chan_check->GetValue();
}

void ComponentDlg::OnDistNeighborSldr(wxScrollEvent &event)
{
	int val = m_dist_neighbor_sldr->GetValue();
	wxString str = wxString::Format("%d", val);
	if (str != m_dist_neighbor_text->GetValue())
		m_dist_neighbor_text->SetValue(str);
}

void ComponentDlg::OnDistNeighborText(wxCommandEvent &event)
{
	long val = 0;
	m_dist_neighbor_text->GetValue().ToLong(&val);
	glbin_comp_def.m_dist_neighbor = (int)val;
	m_dist_neighbor_sldr->ChangeValue(glbin_comp_def.m_dist_neighbor);
}

int ComponentDlg::GetDistMatSize()
{
	int gsize = glbin_comp_analyzer.GetCompGroupSize();
	if (glbin_comp_def.m_use_dist_allchan && gsize > 1)
	{
		int matsize = 0;
		for (int i = 0; i < gsize; ++i)
		{
			flrd::CompGroup* compgroup = glbin_comp_analyzer.GetCompGroup(i);
			if (!compgroup)
				continue;
			matsize += compgroup->celps.size();
		}
		return matsize;
	}
	else
	{
		flrd::CelpList* list = glbin_comp_analyzer.GetCelpList();
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
	int gsize = glbin_comp_analyzer.GetCompGroupSize();
	int bn = glbin_comp_analyzer.GetBrickNum();

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
	if (glbin_comp_def.m_use_dist_allchan && gsize > 1)
	{
		for (int i = 0; i < gsize; ++i)
		{
			flrd::CompGroup* compgroup = glbin_comp_analyzer.GetCompGroup(i);
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
						glbin_comp_analyzer.GetSizeLimit());
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
		flrd::CellGraph &graph = glbin_comp_analyzer.GetCompGroup(0)->graph;
		flrd::CelpList* list = glbin_comp_analyzer.GetCelpList();
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
					glbin_comp_analyzer.GetSizeLimit());
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

	bool bdist = glbin_comp_def.m_use_dist_neighbor &&
		glbin_comp_def.m_dist_neighbor > 0 &&
		glbin_comp_def.m_dist_neighbor < num2-1;

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
					j <= glbin_comp_def.m_dist_neighbor)
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
		size_t dnum = bdist ? (glbin_comp_def.m_dist_neighbor+1) : num2;
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
	m_view->GetObjCenters(tx, ty, tz);
	m_view->SetObjTrans(
		tx - center.x(),
		center.y() - ty,
		center.z() - tz);
}

void ComponentDlg::OnAlignPca(wxCommandEvent& event)
{
	flrd::CelpList* list = glbin_comp_analyzer.GetCelpList();
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
	glbin_aligner.SetRulerList(&rulerlist);
	glbin_aligner.SetAxisType(axis_type);
	glbin_aligner.SetAlignCenter(m_align_center->GetValue());
	glbin_aligner.SetView(glbin_current.canvas);
	glbin_aligner.AlignPca(true);
}

void ComponentDlg::ClearOutputGrid()
{
	int row = m_output_grid->GetNumberRows();
	m_output_grid->DeleteRows(0, row, true);
}

void ComponentDlg::OnNotebook(wxBookCtrlEvent &event)
{
	FluoUpdate({ gstCompPage });
}

void ComponentDlg::OnUseSelChk(wxCommandEvent &event)
{
	glbin_comp_def.m_use_sel = m_use_sel_chk->GetValue();
}

void ComponentDlg::OnUseMlChk(wxCommandEvent &event)
{
	glbin_comp_def.m_use_ml = m_use_ml_chk->GetValue();
}

void ComponentDlg::OnGenerate(wxCommandEvent &event)
{
	if (glbin_comp_def.m_use_ml)
		ApplyRecord();
	else
		GenerateComp();
}

void ComponentDlg::OnAutoUpdate(wxCommandEvent &event)
{
	glbin_comp_def.m_auto_update = m_auto_update_btn->GetValue();
	if (glbin_comp_def.m_auto_update)
		GenerateComp();
}

void ComponentDlg::OnCluster(wxCommandEvent &event)
{
	Cluster();
}

void ComponentDlg::OnCleanBtn(wxCommandEvent &event)
{
	Clean();
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
	VolumeData* vd = m_view->m_cur_vol;
	if (!vd)
		return;

	int bn = vd->GetAllBrickNum();
	m_prog_bit = 97.0f / float(bn * 2 + (glbin_comp_def.m_consistent?1:0));
	m_prog = 0.0f;

	glbin_comp_analyzer.SetVolume(vd);
	if (glbin_comp_def.m_colocal)
	{
		glbin_comp_analyzer.ClearCoVolumes();
		for (int i = 0; i < m_view->GetDispVolumeNum(); ++i)
		{
			VolumeData* vdi = m_view->GetDispVolumeData(i);
			if (vdi != vd)
				glbin_comp_analyzer.AddCoVolume(vdi);
		}
	}
	glbin_comp_analyzer.Analyze(sel, glbin_comp_def.m_consistent, glbin_comp_def.m_colocal);

	if (glbin_comp_def.m_consistent)
	{
		//invalidate label mask in gpu
		vd->GetVR()->clear_tex_label();
		m_view->RefreshGL(39);
	}

	if (glbin_comp_analyzer.GetListSize() > 10000)
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
			glbin_comp_analyzer.OutputCompListFile(str, 1);
		}
		if (fopendlg)
			delete fopendlg;
	}
	else
	{
		string titles, values;
		glbin_comp_analyzer.OutputFormHeader(titles);
		glbin_comp_analyzer.OutputCompListStr(values, 0);
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
	VolumeData* vd = 0;
	if (m_view && m_view->m_cur_vol)
		vd = m_view->m_cur_vol;
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

