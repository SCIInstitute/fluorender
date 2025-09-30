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
#include <ComponentDlg.h>
#include <Global.h>
#include <Names.h>
#include <MainFrame.h>
#include <ComponentDefault.h>
#include <AutomateDefault.h>
#include <BrushDefault.h>
#include <CompEditor.h>
#include <RecordHistParams.h>
#include <CompGenerator.h>
#include <Clusterizer.h>
#include <CompSelector.h>
#include <CompAnalyzer.h>
#include <VolumeSelector.h>
#include <Colocalize.h>
#include <BaseConvVolMesh.h>
#include <ColorCompMesh.h>
#include <Ruler.h>
#include <RulerAlign.h>
#include <VolumeData.h>
#include <RenderView.h>
#include <CurrentObjects.h>
#include <DataManager.h>
#include <ModalDlg.h>
#include <wxSingleSlider.h>
#include <Progress.h>
#include <wx/scrolwin.h>
#include <wx/valnum.h>
#include <limits>
#include <string>
#include <cctype>
#include <set>
#include <fstream>
#include <Debug.h>

ComponentDlg::ComponentDlg(MainFrame *frame)
	: TabbedPanel(frame, frame,
		wxDefaultPosition,
		frame->FromDIP(wxSize(500, 620)),
		0, "ComponentDlg"),
	m_hold_history(false),
	m_auto_update_timer(this),
	m_max_lines(1000)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);
	Freeze();
	SetDoubleBuffered(true);

	//notebook
	m_notebook = new wxAuiNotebook(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize,
		wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE |
		wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_TAB_EXTERNAL_MOVE | wxNO_BORDER);
	m_notebook->AddPage(CreateCompGenPage(m_notebook), "Generate", true);
	m_notebook->AddPage(CreateAnalysisPage(m_notebook), "Analyze");
	m_notebook->AddPage(CreateClusteringPage(m_notebook), "Cluster");
	m_notebook->AddPage(CreateOutputPage(m_notebook), "Information");

	Bind(wxEVT_SIZE, &ComponentDlg::OnSize, this);
	Bind(wxEVT_TIMER, &ComponentDlg::OnAutoUpdateTimer, this);

	wxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
	sizer_v->Add(m_notebook, 1, wxEXPAND | wxALL);
	SetSizer(sizer_v);
	Layout();
	SetAutoLayout(true);
	SetScrollRate(10, 10);

	Thaw();
}

ComponentDlg::~ComponentDlg()
{
}

wxWindow* ComponentDlg::CreateCompGenPage(wxWindow *parent)
{
	wxScrolledWindow *page = new wxScrolledWindow(parent);

	wxStaticText *st = 0;
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;
	//validator: floating point 3
	wxFloatingPointValidator<double> vald_fp3(3);

	//buttons
	wxBoxSizer* sizer1 = new wxBoxSizer(wxHORIZONTAL);
	m_shf_gen_btn = new wxButton(page, wxID_ANY, "Shuffle Colors",
		wxDefaultPosition, wxDefaultSize);
	m_use_ml_chk = new wxCheckBox(page, wxID_ANY, "Get Settings by M.L.",
		wxDefaultPosition, wxDefaultSize);
	m_use_sel_gen_chk = new wxCheckBox(page, wxID_ANY, "Paint-Selected Data Only",
		wxDefaultPosition, wxDefaultSize);
	m_generate_btn = new wxButton(page, wxID_ANY, "Generate",
		wxDefaultPosition, FromDIP(wxSize(75, -1)));
	m_shf_gen_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnShuffle, this);
	m_use_ml_chk->Bind(wxEVT_CHECKBOX, &ComponentDlg::OnUseMlChk, this);
	m_use_sel_gen_chk->Bind(wxEVT_CHECKBOX, &ComponentDlg::OnUseSelChk, this);
	m_generate_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnGenerate, this);
	sizer1->Add(m_shf_gen_btn, 0, wxALIGN_CENTER);
	sizer1->AddStretchSpacer();
	sizer1->Add(m_use_ml_chk, 0, wxALIGN_CENTER);
	sizer1->Add(m_use_sel_gen_chk, 0, wxALIGN_CENTER);
	sizer1->Add(m_generate_btn, 0, wxALIGN_CENTER);

	//iterations
	wxBoxSizer* sizer2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Iterations:",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_iter_sldr = new wxSingleSlider(page, wxID_ANY, 0, 0, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_iter_text = new wxTextCtrl(page, wxID_ANY, "0",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_int);
	m_iter_sldr->Bind(wxEVT_SCROLL_CHANGED, &ComponentDlg::OnIterSldr, this);
	m_iter_text->Bind(wxEVT_TEXT, &ComponentDlg::OnIterText, this);
	sizer2->Add(2, 2);
	sizer2->Add(st, 0, wxALIGN_CENTER);
	sizer2->Add(m_iter_sldr, 1, wxEXPAND);
	sizer2->Add(m_iter_text, 0, wxALIGN_CENTER);
	sizer2->Add(2, 2);

	//threshold
	wxBoxSizer* sizer3 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Threshold:",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_thresh_sldr = new wxSingleSlider(page, wxID_ANY, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_thresh_text = new wxTextCtrl(page, wxID_ANY, "0.000",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_fp3);
	m_thresh_sldr->Bind(wxEVT_SCROLL_CHANGED, &ComponentDlg::OnThreshSldr, this);
	m_thresh_text->Bind(wxEVT_TEXT, &ComponentDlg::OnThreshText, this);
	sizer3->Add(2, 2);
	sizer3->Add(st, 0, wxALIGN_CENTER);
	sizer3->Add(m_thresh_sldr, 1, wxEXPAND);
	sizer3->Add(m_thresh_text, 0, wxALIGN_CENTER);
	sizer3->Add(2, 2);

	//diffusion
	wxBoxSizer* sizer4 = new wxBoxSizer(wxHORIZONTAL);
	m_diff_check = new wxCheckBox(page, wxID_ANY, "Enable Diffusion",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_diff_check->Bind(wxEVT_CHECKBOX, &ComponentDlg::OnDiffCheck, this);
	sizer4->Add(2, 2);
	sizer4->Add(m_diff_check, 0, wxALIGN_CENTER);

	wxBoxSizer* sizer5 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Smoothness:",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_falloff_sldr = new wxSingleSlider(page, wxID_ANY, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_falloff_text = new wxTextCtrl(page, wxID_ANY, "0.000",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_fp3);
	m_falloff_sldr->Bind(wxEVT_SCROLL_CHANGED, &ComponentDlg::OnFalloffSldr, this);
	m_falloff_text->Bind(wxEVT_TEXT, &ComponentDlg::OnFalloffText, this);
	sizer5->Add(2, 2);
	sizer5->Add(st, 0, wxALIGN_CENTER);
	sizer5->Add(m_falloff_sldr, 1, wxEXPAND);
	sizer5->Add(m_falloff_text, 0, wxALIGN_CENTER);
	sizer5->Add(2, 2);

	//clean
	wxBoxSizer* sizer6 = new wxBoxSizer(wxHORIZONTAL);
	m_clean_check = new wxCheckBox(page, wxID_ANY, "Clean Up",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_clean_btn = new wxButton(page, wxID_ANY, "Clean More",
		wxDefaultPosition, FromDIP(wxSize(75, -1)), wxALIGN_LEFT);
	m_clean_check->Bind(wxEVT_CHECKBOX, &ComponentDlg::OnCleanCheck, this);
	m_clean_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnCleanBtn, this);
	sizer6->Add(2, 2);
	sizer6->Add(m_clean_check, 0, wxALIGN_CENTER);
	sizer6->AddStretchSpacer(1);
	sizer6->Add(m_clean_btn, 0, wxALIGN_CENTER);
	sizer6->Add(2, 2);

	//iterations
	wxBoxSizer* sizer7 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Iterations:",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_clean_iter_sldr = new wxSingleSlider(page, wxID_ANY, 5, 1, 50,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_clean_iter_text = new wxTextCtrl(page, wxID_ANY, "5",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_int);
	m_clean_iter_sldr->Bind(wxEVT_SCROLL_CHANGED, &ComponentDlg::OnCleanIterSldr, this);
	m_clean_iter_text->Bind(wxEVT_TEXT, &ComponentDlg::OnCleanIterText, this);
	sizer7->Add(2, 2);
	sizer7->Add(st, 0, wxALIGN_CENTER);
	sizer7->Add(m_clean_iter_sldr, 1, wxEXPAND);
	sizer7->Add(m_clean_iter_text, 0, wxALIGN_CENTER);
	sizer7->Add(2, 2);

	//limit
	wxBoxSizer* sizer8 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Stop Size:",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_clean_limit_sldr = new wxSingleSlider(page, wxID_ANY, 5, 1, 50,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_clean_limit_text = new wxTextCtrl(page, wxID_ANY, "5",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_int);
	m_clean_limit_sldr->Bind(wxEVT_SCROLL_CHANGED, &ComponentDlg::OnCleanLimitSldr, this);
	m_clean_limit_text->Bind(wxEVT_TEXT, &ComponentDlg::OnCleanLimitText, this);
	sizer8->Add(2, 2);
	sizer8->Add(st, 0, wxALIGN_CENTER);
	sizer8->Add(m_clean_limit_sldr, 1, wxEXPAND);
	sizer8->Add(m_clean_limit_text, 0, wxALIGN_CENTER);
	sizer8->Add(2, 2);

	//fixate
	wxBoxSizer* sizer9 = new wxBoxSizer(wxHORIZONTAL);
	m_fixate_check = new wxCheckBox(page, wxID_ANY, "Freeze Merged Regions",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_grow_fixed_check = new wxCheckBox(page, wxID_ANY, "Growth from Freezed Regions",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_fix_update_btn = new wxButton(page, wxID_ANY, "Refreeze",
		wxDefaultPosition, FromDIP(wxSize(75, -1)), wxALIGN_LEFT);
	m_fixate_check->Bind(wxEVT_CHECKBOX, &ComponentDlg::OnFixateCheck, this);
	m_grow_fixed_check->Bind(wxEVT_CHECKBOX, &ComponentDlg::OnGrowFixedCheck, this);
	m_fix_update_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnFixUpdateBtn, this);
	sizer9->Add(2, 2);
	sizer9->Add(m_fixate_check, 0, wxALIGN_CENTER);
	sizer9->Add(2, 2);
	sizer9->Add(m_grow_fixed_check, 0, wxALIGN_CENTER);
	sizer9->AddStretchSpacer(1);
	sizer9->Add(m_fix_update_btn, 0, wxALIGN_CENTER);
	sizer9->Add(2, 2);

	wxBoxSizer* sizer10 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Stop Size:",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_fix_size_sldr = new wxSingleSlider(page, wxID_ANY, 50, 1, 200,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_fix_size_text = new wxTextCtrl(page, wxID_ANY, "50",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_int);
	m_fix_size_sldr->Bind(wxEVT_SCROLL_CHANGED, &ComponentDlg::OnFixSizeSldr, this);
	m_fix_size_text->Bind(wxEVT_TEXT, &ComponentDlg::OnFixSizeText, this);
	sizer10->Add(2, 2);
	sizer10->Add(st, 0, wxALIGN_CENTER);
	sizer10->Add(m_fix_size_sldr, 1, wxEXPAND);
	sizer10->Add(m_fix_size_text, 0, wxALIGN_CENTER);
	sizer10->Add(2, 2);

	//density
	wxBoxSizer* sizer11 = new wxBoxSizer(wxHORIZONTAL);
	m_density_check = new wxCheckBox(page, wxID_ANY, "Use Density Field",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_density_check->Bind(wxEVT_CHECKBOX, &ComponentDlg::OnDensityCheck, this);
	sizer11->Add(2, 2);
	sizer11->Add(m_density_check, 0, wxALIGN_CENTER);

	wxBoxSizer* sizer12 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Separation:",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_density_sldr = new wxSingleSlider(page, wxID_ANY, 1000, 0, 10000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_density_text = new wxTextCtrl(page, wxID_ANY, "1.0",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_fp3);
	m_density_sldr->Bind(wxEVT_SCROLL_CHANGED, &ComponentDlg::OnDensitySldr, this);
	m_density_text->Bind(wxEVT_TEXT, &ComponentDlg::OnDensityText, this);
	sizer12->Add(2, 2);
	sizer12->Add(st, 0, wxALIGN_CENTER);
	sizer12->Add(m_density_sldr, 1, wxEXPAND);
	sizer12->Add(m_density_text, 0, wxALIGN_CENTER);
	sizer12->Add(2, 2);

	wxBoxSizer* sizer13 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Noise Level:",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_varth_sldr = new wxSingleSlider(page, wxID_ANY, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_varth_text = new wxTextCtrl(page, wxID_ANY, "0.0",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_fp3);
	m_varth_sldr->Bind(wxEVT_SCROLL_CHANGED, &ComponentDlg::OnVarthSldr, this);
	m_varth_text->Bind(wxEVT_TEXT, &ComponentDlg::OnVarthText, this);
	sizer13->Add(2, 2);
	sizer13->Add(st, 0, wxALIGN_CENTER);
	sizer13->Add(m_varth_sldr, 1, wxEXPAND);
	sizer13->Add(m_varth_text, 0, wxALIGN_CENTER);
	sizer13->Add(2, 2);

	wxBoxSizer* sizer14 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Filter Size:",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_density_window_size_sldr = new wxSingleSlider(page, wxID_ANY, 5, 1, 20,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_density_window_size_text = new wxTextCtrl(page, wxID_ANY, "5",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_int);
	m_density_window_size_sldr->Bind(wxEVT_SCROLL_CHANGED, &ComponentDlg::OnDensityWindowSizeSldr, this);
	m_density_window_size_text->Bind(wxEVT_TEXT, &ComponentDlg::OnDensityWindowSizeText, this);
	sizer14->Add(2, 2);
	sizer14->Add(st, 0, wxALIGN_CENTER);
	sizer14->Add(m_density_window_size_sldr, 1, wxEXPAND);
	sizer14->Add(m_density_window_size_text, 0, wxALIGN_CENTER);
	sizer14->Add(2, 2);

	wxBoxSizer* sizer15 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Feature Size:",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_density_stats_size_sldr = new wxSingleSlider(page, wxID_ANY, 15, 1, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_density_stats_size_text = new wxTextCtrl(page, wxID_ANY, "15",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_int);
	m_density_stats_size_sldr->Bind(wxEVT_SCROLL_CHANGED, &ComponentDlg::OnDensityStatsSizeSldr, this);
	m_density_stats_size_text->Bind(wxEVT_TEXT, &ComponentDlg::OnDensityStatsSizeText, this);
	sizer15->Add(2, 2);
	sizer15->Add(st, 0, wxALIGN_CENTER);
	sizer15->Add(m_density_stats_size_sldr, 1, wxEXPAND);
	sizer15->Add(m_density_stats_size_text, 0, wxALIGN_CENTER);
	sizer15->Add(2, 2);

	//distance field
	wxBoxSizer* sizer16 = new wxBoxSizer(wxHORIZONTAL);
	m_use_dist_field_check = new wxCheckBox(page, wxID_ANY, "Use Distance Field",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_use_dist_field_check->Bind(wxEVT_CHECKBOX, &ComponentDlg::OnUseDistFieldCheck, this);
	sizer16->Add(2, 2);
	sizer16->Add(m_use_dist_field_check, 0, wxALIGN_CENTER);

	wxBoxSizer* sizer17 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Strength:",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_dist_strength_sldr = new wxSingleSlider(page, wxID_ANY, 500, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_dist_strength_text = new wxTextCtrl(page, wxID_ANY, "0.5",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_fp3);
	m_dist_strength_sldr->Bind(wxEVT_SCROLL_CHANGED, &ComponentDlg::OnDistStrengthSldr, this);
	m_dist_strength_text->Bind(wxEVT_TEXT, &ComponentDlg::OnDistStrengthText, this);
	sizer17->Add(2, 2);
	sizer17->Add(st, 0, wxALIGN_CENTER);
	sizer17->Add(m_dist_strength_sldr, 1, wxEXPAND);
	sizer17->Add(m_dist_strength_text, 0, wxALIGN_CENTER);
	sizer17->Add(2, 2);

	wxBoxSizer* sizer18 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Perimeter Value:",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_dist_thresh_sldr = new wxSingleSlider(page, wxID_ANY, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_dist_thresh_text = new wxTextCtrl(page, wxID_ANY, "0.000",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_fp3);
	m_dist_thresh_sldr->Bind(wxEVT_SCROLL_CHANGED, &ComponentDlg::OnDistThreshSldr, this);
	m_dist_thresh_text->Bind(wxEVT_TEXT, &ComponentDlg::OnDistThreshText, this);
	sizer18->Add(2, 2);
	sizer18->Add(st, 0, wxALIGN_CENTER);
	sizer18->Add(m_dist_thresh_sldr, 1, wxEXPAND);
	sizer18->Add(m_dist_thresh_text, 0, wxALIGN_CENTER);
	sizer18->Add(2, 2);

	wxBoxSizer* sizer19 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Filter Size:",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_dist_filter_size_sldr = new wxSingleSlider(page, wxID_ANY, 3, 1, 20,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_dist_filter_size_text = new wxTextCtrl(page, wxID_ANY, "3",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_int);
	m_dist_filter_size_sldr->Bind(wxEVT_SCROLL_CHANGED, &ComponentDlg::OnDistFilterSizeSldr, this);
	m_dist_filter_size_text->Bind(wxEVT_TEXT, &ComponentDlg::OnDistFilterSizeText, this);
	sizer19->Add(2, 2);
	sizer19->Add(st, 0, wxALIGN_CENTER);
	sizer19->Add(m_dist_filter_size_sldr, 1, wxEXPAND);
	sizer19->Add(m_dist_filter_size_text, 0, wxALIGN_CENTER);
	sizer19->Add(2, 2);

	wxBoxSizer* sizer20 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Feature Size:",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_max_dist_sldr = new wxSingleSlider(page, wxID_ANY, 30, 1, 255,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_max_dist_text = new wxTextCtrl(page, wxID_ANY, "30",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_int);
	m_max_dist_sldr->Bind(wxEVT_SCROLL_CHANGED, &ComponentDlg::OnMaxDistSldr, this);
	m_max_dist_text->Bind(wxEVT_TEXT, &ComponentDlg::OnMaxDistText, this);
	sizer20->Add(2, 2);
	sizer20->Add(st, 0, wxALIGN_CENTER);
	sizer20->Add(m_max_dist_sldr, 1, wxEXPAND);
	sizer20->Add(m_max_dist_text, 0, wxALIGN_CENTER);
	sizer20->Add(2, 2);

	//command record
	wxBoxSizer* sizer21 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Actions:",
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
	sizer21->Add(2, 2);
	sizer21->Add(st, 0, wxALIGN_CENTER);
	sizer21->AddStretchSpacer();
	sizer21->Add(m_cmd_count_text, 0, wxALIGN_CENTER);
	sizer21->Add(m_record_cmd_btn, 0, wxALIGN_CENTER);
	sizer21->Add(m_play_cmd_btn, 0, wxALIGN_CENTER);
	sizer21->Add(m_reset_cmd_btn, 0, wxALIGN_CENTER);
	sizer21->Add(2, 2);

	wxBoxSizer* sizer22 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Save Actions:",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_cmd_file_text = new wxTextCtrl(page, wxID_ANY, "",
		wxDefaultPosition, wxDefaultSize);
	m_load_cmd_btn = new wxButton(page, wxID_ANY, "Load",
		wxDefaultPosition, FromDIP(wxSize(75, -1)));
	m_save_cmd_btn = new wxButton(page, wxID_ANY, "Save",
		wxDefaultPosition, FromDIP(wxSize(75, -1)));
	m_load_cmd_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnLoadCmd, this);
	m_save_cmd_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnSaveCmd, this);
	sizer22->Add(2, 2);
	sizer22->Add(st, 0, wxALIGN_CENTER);
	sizer22->Add(m_cmd_file_text, 1, wxALIGN_CENTER);
	sizer22->Add(m_load_cmd_btn, 0, wxALIGN_CENTER);
	sizer22->Add(m_save_cmd_btn, 0, wxALIGN_CENTER);
	sizer22->Add(2, 2);

	wxStaticBoxSizer *group1 = new wxStaticBoxSizer(
		wxVERTICAL, page, "Simple Segmentation");
	group1->Add(5, 5);
	group1->Add(sizer2, 0, wxEXPAND);
	group1->Add(5, 5);
	group1->Add(sizer3, 0, wxEXPAND);
	group1->Add(5, 5);
	group1->Add(sizer4, 0, wxEXPAND);
	group1->Add(5, 5);
	group1->Add(sizer5, 0, wxEXPAND);
	group1->Add(5, 5);

	wxStaticBoxSizer *group2 = new wxStaticBoxSizer(
		wxVERTICAL, page, "Noise Reduction");
	group2->Add(sizer6, 0, wxEXPAND);
	group2->Add(5, 5);
	group2->Add(sizer7, 0, wxEXPAND);
	group2->Add(5, 5);
	group2->Add(sizer8, 0, wxEXPAND);
	group2->Add(5, 5);
	group2->Add(sizer9, 0, wxEXPAND);
	group2->Add(5, 5);
	group2->Add(sizer10, 0, wxEXPAND);
	group2->Add(5, 5);

	wxStaticBoxSizer *group3 = new wxStaticBoxSizer(
		wxVERTICAL, page, "Advanced Growth && Merge");
	group3->Add(sizer11, 0, wxEXPAND);
	group3->Add(5, 5);
	group3->Add(sizer12, 0, wxEXPAND);
	group3->Add(5, 5);
	group3->Add(sizer13, 0, wxEXPAND);
	group3->Add(5, 5);
	group3->Add(sizer14, 0, wxEXPAND);
	group3->Add(5, 5);
	group3->Add(sizer15, 0, wxEXPAND);
	group3->Add(5, 5);
	group3->Add(sizer16, 0, wxEXPAND);
	group3->Add(5, 5);
	group3->Add(sizer17, 0, wxEXPAND);
	group3->Add(5, 5);
	group3->Add(sizer18, 0, wxEXPAND);
	group3->Add(5, 5);
	group3->Add(sizer19, 0, wxEXPAND);
	group3->Add(5, 5);
	group3->Add(sizer20, 0, wxEXPAND);
	group3->Add(5, 5);

	wxStaticBoxSizer *group4 = new wxStaticBoxSizer(
		wxVERTICAL, page, "Action Recording and Playback");
	group4->Add(sizer21, 0, wxEXPAND);
	group4->Add(5, 5);
	group4->Add(sizer22, 0, wxEXPAND);
	group4->Add(5, 5);

	wxBoxSizer* sizerv = new wxBoxSizer(wxVERTICAL);
	sizerv->Add(10, 10);
	sizerv->Add(sizer1, 0, wxEXPAND);
	sizerv->Add(10, 10);
	sizerv->Add(group1, 0, wxEXPAND);
	sizerv->Add(10, 10);
	sizerv->Add(group2, 0, wxEXPAND);
	sizerv->Add(10, 10);
	sizerv->Add(group3, 0, wxEXPAND);
	sizerv->Add(10, 10);
	sizerv->Add(group4, 0, wxEXPAND);
	sizerv->Add(10, 10);

	page->SetSizer(sizerv);
	page->SetAutoLayout(true);
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

	//buttons
	wxBoxSizer* sizer1 = new wxBoxSizer(wxHORIZONTAL);
	m_shf_cls_btn = new wxButton(page, wxID_ANY, "Shuffle Colors",
		wxDefaultPosition, wxDefaultSize);
	m_cluster_btn = new wxButton(page, wxID_ANY, "Cluster",
		wxDefaultPosition, FromDIP(wxSize(75, -1)));
	m_shf_cls_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnShuffle, this);
	m_cluster_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnCluster, this);
	sizer1->Add(m_shf_cls_btn, 0, wxALIGN_CENTER);
	sizer1->AddStretchSpacer();
	sizer1->Add(m_cluster_btn, 0, wxALIGN_CENTER);

	//clustering methods
	wxStaticBoxSizer *sizer2 = new wxStaticBoxSizer(
		wxVERTICAL, page, "Clustering Method");
	wxBoxSizer* sizer2_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Choose Method:",
		wxDefaultPosition, FromDIP(wxSize(100, 20)));
	m_cluster_method_exmax_rd = new wxRadioButton(page, ID_ClusterMethodExmaxRd,
		"EM", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	m_cluster_method_dbscan_rd = new wxRadioButton(page, ID_ClusterMethodDbscanRd,
		"DBSCAN", wxDefaultPosition, wxDefaultSize);
	m_cluster_method_kmeans_rd = new wxRadioButton(page, ID_ClusterMethodKmeansRd,
		"K-Means", wxDefaultPosition, wxDefaultSize);
	m_cluster_method_exmax_rd->Bind(wxEVT_RADIOBUTTON, &ComponentDlg::OnClusterMethodCheck, this);
	m_cluster_method_dbscan_rd->Bind(wxEVT_RADIOBUTTON, &ComponentDlg::OnClusterMethodCheck, this);
	m_cluster_method_kmeans_rd->Bind(wxEVT_RADIOBUTTON, &ComponentDlg::OnClusterMethodCheck, this);
	sizer2_1->Add(5, 5);
	sizer2_1->Add(st, 0, wxALIGN_CENTER);
	sizer2_1->Add(m_cluster_method_kmeans_rd, 0, wxALIGN_CENTER);
	sizer2_1->Add(10, 10);
	sizer2_1->Add(m_cluster_method_exmax_rd, 0, wxALIGN_CENTER);
	sizer2_1->Add(10, 10);
	sizer2_1->Add(m_cluster_method_dbscan_rd, 0, wxALIGN_CENTER);
	//
	sizer2->Add(10, 10);
	sizer2->Add(sizer2_1, 0, wxEXPAND);
	sizer2->Add(10, 10);

	//parameters
	wxStaticBoxSizer *sizer3 = new wxStaticBoxSizer(
		wxVERTICAL, page, "Settings");
	//clnum
	wxBoxSizer *sizer3_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Cluster Number:",
		wxDefaultPosition, FromDIP(wxSize(100, 20)));
	m_cluster_clnum_sldr = new wxSingleSlider(page, wxID_ANY, 2, 2, 10,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_cluster_clnum_text = new wxTextCtrl(page, wxID_ANY, "2",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_int);
	m_cluster_clnum_sldr->Bind(wxEVT_SCROLL_CHANGED, &ComponentDlg::OnClusterClnumSldr, this);
	m_cluster_clnum_text->Bind(wxEVT_TEXT, &ComponentDlg::OnClusterClnumText, this);
	sizer3_1->Add(5, 5);
	sizer3_1->Add(st, 0, wxALIGN_CENTER);
	sizer3_1->Add(m_cluster_clnum_sldr, 1, wxEXPAND);
	sizer3_1->Add(m_cluster_clnum_text, 0, wxALIGN_CENTER);
	sizer3_1->Add(5, 5);
	//maxiter
	wxBoxSizer *sizer3_2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Max Iterations:",
		wxDefaultPosition, FromDIP(wxSize(100, 20)));
	m_cluster_maxiter_sldr = new wxSingleSlider(page, wxID_ANY, 200, 1, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_cluster_maxiter_text = new wxTextCtrl(page, wxID_ANY, "200",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_int);
	m_cluster_maxiter_sldr->Bind(wxEVT_SCROLL_CHANGED, &ComponentDlg::OnClusterMaxiterSldr, this);
	m_cluster_maxiter_text->Bind(wxEVT_TEXT, &ComponentDlg::OnClusterMaxiterText, this);
	sizer3_2->Add(5, 5);
	sizer3_2->Add(st, 0, wxALIGN_CENTER);
	sizer3_2->Add(m_cluster_maxiter_sldr, 1, wxEXPAND);
	sizer3_2->Add(m_cluster_maxiter_text, 0, wxALIGN_CENTER);
	sizer3_2->Add(5, 5);
	//tol
	wxBoxSizer *sizer3_3 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Tolerance:",
		wxDefaultPosition, FromDIP(wxSize(100, 20)));
	m_cluster_tol_sldr = new wxSingleSlider(page, wxID_ANY, 0.90, 1, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_cluster_tol_text = new wxTextCtrl(page, wxID_ANY, "0.90",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_fp2);
	m_cluster_tol_sldr->Bind(wxEVT_SCROLL_CHANGED, &ComponentDlg::OnClusterTolSldr, this);
	m_cluster_tol_text->Bind(wxEVT_TEXT, &ComponentDlg::OnClusterTolText, this);
	sizer3_3->Add(5, 5);
	sizer3_3->Add(st, 0, wxALIGN_CENTER);
	sizer3_3->Add(m_cluster_tol_sldr, 1, wxEXPAND);
	sizer3_3->Add(m_cluster_tol_text, 0, wxALIGN_CENTER);
	sizer3_3->Add(5, 5);
	//size
	wxBoxSizer *sizer3_4 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Min. Size:",
		wxDefaultPosition, FromDIP(wxSize(100, 20)));
	m_cluster_size_sldr = new wxSingleSlider(page, wxID_ANY, 60, 1, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_cluster_size_text = new wxTextCtrl(page, wxID_ANY, "60",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_int);
	m_cluster_size_sldr->Bind(wxEVT_SCROLL_CHANGED, &ComponentDlg::OnClusterSizeSldr, this);
	m_cluster_size_text->Bind(wxEVT_TEXT, &ComponentDlg::OnClusterSizeText, this);
	sizer3_4->Add(5, 5);
	sizer3_4->Add(st, 0, wxALIGN_CENTER);
	sizer3_4->Add(m_cluster_size_sldr, 1, wxEXPAND);
	sizer3_4->Add(m_cluster_size_text, 0, wxALIGN_CENTER);
	sizer3_4->Add(5, 5);
	//eps
	wxBoxSizer *sizer3_5 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Neighborhood:",
		wxDefaultPosition, FromDIP(wxSize(100, 20)));
	m_cluster_eps_sldr = new wxSingleSlider(page, wxID_ANY, 25, 5, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_cluster_eps_text = new wxTextCtrl(page, wxID_ANY, "2.5",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_fp1);
	m_cluster_eps_sldr->Bind(wxEVT_SCROLL_CHANGED, &ComponentDlg::OnClusterEpsSldr, this);
	m_cluster_eps_text->Bind(wxEVT_TEXT, &ComponentDlg::OnClusterepsText, this);
	sizer3_5->Add(5, 5);
	sizer3_5->Add(st, 0, wxALIGN_CENTER);
	sizer3_5->Add(m_cluster_eps_sldr, 1, wxEXPAND);
	sizer3_5->Add(m_cluster_eps_text, 0, wxALIGN_CENTER);
	sizer3_5->Add(5, 5);
	//
	sizer3->Add(10, 10);
	sizer3->Add(sizer3_1, 0, wxEXPAND);
	sizer3->Add(10, 10);
	sizer3->Add(sizer3_2, 0, wxEXPAND);
	sizer3->Add(10, 10);
	sizer3->Add(sizer3_3, 0, wxEXPAND);
	sizer3->Add(10, 10);
	sizer3->Add(sizer3_4, 0, wxEXPAND);
	sizer3->Add(10, 10);
	sizer3->Add(sizer3_5, 0, wxEXPAND);
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
	page->SetAutoLayout(true);
	page->SetScrollRate(10, 10);

	return page;
}

wxWindow* ComponentDlg::CreateAnalysisPage(wxWindow *parent)
{
	wxScrolledWindow *page = new wxScrolledWindow(parent);

	wxStaticText *st = 0;
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;

	//buttons
	wxBoxSizer* sizer1 = new wxBoxSizer(wxHORIZONTAL);
	m_shf_anl_btn = new wxButton(page, wxID_ANY, "Shuffle Colors",
		wxDefaultPosition, wxDefaultSize);
	m_use_sel_anl_chk = new wxCheckBox(page, wxID_ANY, "Paint-Selected Data Only",
		wxDefaultPosition, wxDefaultSize);
	m_analyze_btn = new wxButton(page, wxID_ANY, "Analyze",
		wxDefaultPosition, FromDIP(wxSize(75, -1)));
	m_shf_anl_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnShuffle, this);
	m_use_sel_anl_chk->Bind(wxEVT_CHECKBOX, &ComponentDlg::OnUseSelChk, this);
	m_analyze_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnAnalyze, this);
	sizer1->Add(m_shf_anl_btn, 0, wxALIGN_CENTER);
	sizer1->AddStretchSpacer();
	sizer1->Add(m_use_sel_anl_chk, 0, wxALIGN_CENTER);
	sizer1->Add(m_analyze_btn, 0, wxALIGN_CENTER);

	//id tools
	wxStaticBoxSizer *sizer2 = new wxStaticBoxSizer(
		wxVERTICAL, page, "Selection and Modification by IDs");
	wxBoxSizer* sizer2_1 = new wxBoxSizer(wxHORIZONTAL);
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
	sizer2_1->Add(5, 5);
	sizer2_1->Add(st, 0, wxALIGN_CENTER);
	sizer2_1->Add(m_comp_id_text, 0, wxALIGN_CENTER);
	sizer2_1->Add(m_comp_id_x_btn, 0, wxALIGN_CENTER);
	sizer2_1->Add(10, 10);
	sizer2_1->Add(m_analysis_min_check, 0, wxALIGN_CENTER);
	sizer2_1->Add(10, 10);
	sizer2_1->Add(m_analysis_min_spin, 0, wxALIGN_CENTER);
	sizer2_1->Add(10, 10);
	sizer2_1->Add(m_analysis_max_check, 0, wxALIGN_CENTER);
	sizer2_1->Add(10, 10);
	sizer2_1->Add(m_analysis_max_spin, 0, wxALIGN_CENTER);
	//selection
	wxBoxSizer* sizer2_2 = new wxBoxSizer(wxHORIZONTAL);
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
	sizer2_2->Add(5, 5);
	sizer2_2->Add(m_comp_append_btn, 1, wxEXPAND);
	sizer2_2->Add(m_comp_exclusive_btn, 1, wxEXPAND);
	sizer2_2->Add(m_comp_all_btn, 1, wxEXPAND);
	sizer2_2->Add(5, 5);
	sizer2_2->Add(m_comp_full_btn, 1, wxEXPAND);
	sizer2_2->Add(m_comp_clear_btn, 1, wxEXPAND);
	//modify
	wxBoxSizer* sizer2_3 = new wxBoxSizer(wxHORIZONTAL);
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
	sizer2_3->Add(5, 5);
	sizer2_3->Add(m_comp_new_btn, 1, wxEXPAND);
	sizer2_3->Add(m_comp_add_btn, 1, wxEXPAND);
	sizer2_3->Add(m_comp_replace_btn, 1, wxEXPAND);
	sizer2_3->Add(5, 5);
	sizer2_3->Add(m_comp_clean_bkg_btn, 1, wxEXPAND);
	sizer2_3->Add(m_comp_combine_btn, 1, wxEXPAND);
	//
	sizer2->Add(10, 10);
	sizer2->Add(sizer2_1, 1, wxEXPAND);
	sizer2->Add(10, 10);
	sizer2->Add(sizer2_2, 1, wxEXPAND);
	sizer2->Add(10, 10);
	sizer2->Add(sizer2_3, 1, wxEXPAND);
	sizer2->Add(10, 10);

	//Options
	wxStaticBoxSizer *sizer3 = new wxStaticBoxSizer(
		wxVERTICAL, page, "Options");
	wxBoxSizer *sizer3_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Contact Size:",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_con_size_sldr = new wxSingleSlider(page, wxID_ANY, 5, 0, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_con_size_text = new wxTextCtrl(page, wxID_ANY, "5",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_int);
	m_con_size_sldr->Bind(wxEVT_SCROLL_CHANGED, &ComponentDlg::OnConSizeSldr, this);
	m_con_size_text->Bind(wxEVT_TEXT, &ComponentDlg::OnConSizeText, this);
	sizer3_1->Add(5, 5);
	sizer3_1->Add(st, 0, wxALIGN_CENTER);
	sizer3_1->Add(5, 5);
	sizer3_1->Add(m_con_size_sldr, 1, wxEXPAND);
	sizer3_1->Add(5, 5);
	sizer3_1->Add(m_con_size_text, 0, wxALIGN_CENTER);
	sizer3_1->Add(5, 5);
	wxBoxSizer *sizer3_2 = new wxBoxSizer(wxHORIZONTAL);
	m_consistent_check = new wxCheckBox(page, wxID_ANY, "Make color consistent for multiple bricks",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	sizer3_2->Add(5, 5);
	sizer3_2->Add(m_consistent_check, 0, wxALIGN_CENTER);
	wxBoxSizer *sizer3_3 = new wxBoxSizer(wxHORIZONTAL);
	m_colocal_check = new wxCheckBox(page, wxID_ANY, "Compute colocalization with other channels",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_consistent_check->Bind(wxEVT_CHECKBOX, &ComponentDlg::OnConsistentCheck, this);
	m_colocal_check->Bind(wxEVT_CHECKBOX, &ComponentDlg::OnColocalCheck, this);
	sizer3_3->Add(5, 5);
	sizer3_3->Add(m_colocal_check, 0, wxALIGN_CENTER);
	//
	sizer3->Add(10, 10);
	sizer3->Add(sizer3_1, 0, wxEXPAND);
	sizer3->Add(10, 10);
	sizer3->Add(sizer3_2, 0, wxEXPAND);
	sizer3->Add(10, 10);
	sizer3->Add(sizer3_3, 0, wxEXPAND);
	sizer3->Add(10, 10);

	//output
	wxStaticBoxSizer *sizer4 = new wxStaticBoxSizer(
		wxVERTICAL, page, "Output as New Channels");
	//radios
	wxBoxSizer *sizer4_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Channel Type:",
		wxDefaultPosition, FromDIP(wxSize(100, 20)));
	m_output_multi_rb = new wxRadioButton(page, ID_OutputMultiRb, "Each Comp.",
		wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	m_output_rgb_rb = new wxRadioButton(page, ID_OutputRgbRb, "R+G+B",
		wxDefaultPosition, wxDefaultSize);
	m_output_multi_rb->Bind(wxEVT_RADIOBUTTON, &ComponentDlg::OnOutputTypeRadio, this);
	m_output_rgb_rb->Bind(wxEVT_RADIOBUTTON, &ComponentDlg::OnOutputTypeRadio, this);
	sizer4_1->Add(5, 5);
	sizer4_1->Add(st, 0, wxALIGN_CENTER);
	sizer4_1->Add(m_output_multi_rb, 0, wxALIGN_CENTER);
	sizer4_1->Add(5, 5);
	sizer4_1->Add(m_output_rgb_rb, 0, wxALIGN_CENTER);
	sizer4_1->Add(5, 5);
	//buttons
	wxBoxSizer *sizer4_2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Output:",
		wxDefaultPosition, wxDefaultSize);
	m_output_random_btn = new wxButton(page, ID_OutputRandomBtn, "Colors",
		wxDefaultPosition, wxDefaultSize);
	m_output_size_btn = new wxButton(page, ID_OutputSizeBtn, "Sizes",
		wxDefaultPosition, wxDefaultSize);
	m_output_mesh_btn = new wxButton(page, wxID_ANY, "Mesh",
		wxDefaultPosition, wxDefaultSize);
	m_output_id_btn = new wxButton(page, ID_OutputIdBtn, "IDs",
		wxDefaultPosition, wxDefaultSize);
	m_output_sn_btn = new wxButton(page, ID_OutputSnBtn, "Serial No.",
		wxDefaultPosition, wxDefaultSize);
	m_output_random_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnOutputChannels, this);
	m_output_size_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnOutputChannels, this);
	m_output_id_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnOutputAnnotData, this);
	m_output_sn_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnOutputAnnotData, this);
	m_output_mesh_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnOutputMeshData, this);
	sizer4_2->Add(5, 5);
	sizer4_2->Add(st, 0, wxALIGN_CENTER);
	sizer4_2->Add(m_output_random_btn, 1, wxEXPAND);
	sizer4_2->Add(m_output_size_btn, 1, wxEXPAND);
	sizer4_2->Add(m_output_mesh_btn, 1, wxEXPAND);
	sizer4_2->Add(m_output_id_btn, 1, wxEXPAND);
	sizer4_2->Add(m_output_sn_btn, 1, wxEXPAND);
	sizer4_2->Add(5, 5);
	//
	sizer4->Add(10, 10);
	sizer4->Add(sizer4_1, 1, wxEXPAND);
	sizer4->Add(10, 10);
	sizer4->Add(sizer4_2, 1, wxEXPAND);
	sizer4->Add(10, 10);

	wxStaticBoxSizer *sizer5 = new wxStaticBoxSizer(
		wxVERTICAL, page, "Distances");
	wxBoxSizer *sizer5_1 = new wxBoxSizer(wxHORIZONTAL);
	m_dist_neighbor_check = new wxCheckBox(page, wxID_ANY, "Filter Nearest Neighbors",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_dist_all_chan_check = new wxCheckBox(page, wxID_ANY, "All Channel Results",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_dist_output_btn = new wxButton(page, wxID_ANY, "Compute",
		wxDefaultPosition, wxDefaultSize);
	m_dist_neighbor_check->Bind(wxEVT_CHECKBOX, &ComponentDlg::OnDistNeighborCheck, this);
	m_dist_all_chan_check->Bind(wxEVT_CHECKBOX, &ComponentDlg::OnDistAllChanCheck, this);
	m_dist_output_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnDistOutput, this);
	sizer5_1->Add(5, 5);
	sizer5_1->Add(m_dist_neighbor_check, 0, wxALIGN_CENTER);
	sizer5_1->Add(5, 5);
	sizer5_1->Add(m_dist_all_chan_check, 0, wxALIGN_CENTER);
	sizer5_1->AddStretchSpacer(1);
	sizer5_1->Add(m_dist_output_btn, 0, wxALIGN_CENTER);
	sizer5_1->Add(5, 5);
	wxBoxSizer *sizer5_2 = new wxBoxSizer(wxHORIZONTAL);
	m_dist_neighbor_sldr = new wxSingleSlider(page, wxID_ANY, 1, 1, 20,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_dist_neighbor_text = new wxTextCtrl(page, wxID_ANY, "1",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), wxTE_RIGHT, vald_int);
	m_dist_neighbor_sldr->Bind(wxEVT_SCROLL_CHANGED, &ComponentDlg::OnDistNeighborSldr, this);
	m_dist_neighbor_text->Bind(wxEVT_TEXT, &ComponentDlg::OnDistNeighborText, this);
	sizer5_2->Add(5, 5);
	sizer5_2->Add(m_dist_neighbor_sldr, 1, wxEXPAND);
	sizer5_2->Add(5, 5);
	sizer5_2->Add(m_dist_neighbor_text, 0, wxALIGN_CENTER);
	sizer5_2->Add(5, 5);
	//
	sizer5->Add(10, 10);
	sizer5->Add(sizer5_1, 0, wxEXPAND);
	sizer5->Add(10, 10);
	sizer5->Add(sizer5_2, 0, wxEXPAND);
	sizer5->Add(10, 10);

	//alignment
	wxStaticBoxSizer *sizer6 = new wxStaticBoxSizer(
		wxVERTICAL, page, "Align Render View to Analyzed Components");
	wxBoxSizer* sizer6_1 = new wxBoxSizer(wxHORIZONTAL);
	m_align_center_chk = new wxCheckBox(page, wxID_ANY,
		"Move to Center", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_align_center_chk->Bind(wxEVT_CHECKBOX, &ComponentDlg::OnAlignCenterChk, this);
	sizer6_1->Add(5, 5);
	sizer6_1->Add(m_align_center_chk, 0, wxALIGN_CENTER);
	wxBoxSizer* sizer6_2 = new wxBoxSizer(wxHORIZONTAL);
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
	sizer6_2->Add(5, 5);
	sizer6_2->Add(st, 0, wxALIGN_CENTER);
	sizer6_2->Add(m_align_xyz, 1, wxEXPAND);
	sizer6_2->Add(m_align_yxz, 1, wxEXPAND);
	sizer6_2->Add(m_align_zxy, 1, wxEXPAND);
	sizer6_2->Add(m_align_xzy, 1, wxEXPAND);
	sizer6_2->Add(m_align_yzx, 1, wxEXPAND);
	sizer6_2->Add(m_align_zyx, 1, wxEXPAND);
	//
	sizer6->Add(5, 5);
	sizer6->Add(sizer6_1, 1, wxEXPAND);
	sizer6->Add(5, 5);
	sizer6->Add(sizer6_2, 1, wxEXPAND);
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

	return page;
}

wxWindow* ComponentDlg::CreateOutputPage(wxWindow* parent)
{
	wxScrolledWindow *page = new wxScrolledWindow(parent);

	//stats text
	wxBoxSizer *sizer1 = new wxBoxSizer(wxHORIZONTAL);
	m_include_btn = new wxButton(page, wxID_ANY,
		"Include", wxDefaultPosition, wxDefaultSize);
	m_exclude_btn = new wxButton(page, wxID_ANY,
		"Exclude", wxDefaultPosition, wxDefaultSize);
	m_history_chk = new wxCheckBox(page, wxID_ANY,
		"Hold History", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_clear_hist_btn = new wxButton(page, wxID_ANY,
		"Clear History", wxDefaultPosition, wxDefaultSize);
	m_include_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnIncludeBtn, this);
	m_exclude_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnExcludeBtn, this);
	m_history_chk->Bind(wxEVT_CHECKBOX, &ComponentDlg::OnHistoryChk, this);
	m_clear_hist_btn->Bind(wxEVT_BUTTON, &ComponentDlg::OnClearHistBtn, this);
	sizer1->Add(m_include_btn, 0, wxALIGN_CENTER);
	sizer1->Add(m_exclude_btn, 0, wxALIGN_CENTER);
	sizer1->AddStretchSpacer(1);
	sizer1->Add(m_history_chk, 0, wxALIGN_CENTER);
	sizer1->Add(5, 5);
	sizer1->Add(m_clear_hist_btn, 0, wxALIGN_CENTER);
	//grid
	m_output_grid = new wxGrid(page, wxID_ANY);
	m_output_grid->CreateGrid(0, 1);
	m_output_grid->SetSelectionMode(wxGrid::wxGridSelectCells);
	//m_output_grid->Fit();
	m_output_grid->Bind(wxEVT_KEY_DOWN, &ComponentDlg::OnKeyDown, this);
	m_output_grid->Bind(wxEVT_GRID_SELECT_CELL, &ComponentDlg::OnSelectCell, this);
	m_output_grid->Bind(wxEVT_GRID_RANGE_SELECT, &ComponentDlg::OnRangeSelect, this);
	m_output_grid->Bind(wxEVT_GRID_LABEL_LEFT_CLICK, &ComponentDlg::OnSelectCell, this);

	wxBoxSizer* sizerv = new wxBoxSizer(wxVERTICAL);
	sizerv->Add(10, 10);
	sizerv->Add(sizer1, 0, wxEXPAND);
	sizerv->Add(10, 10);
	sizerv->Add(m_output_grid, 1, wxEXPAND);
	sizerv->Add(10, 10);

	page->SetSizer(sizerv);
	page->SetScrollRate(10, 10);

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

	if (update_all || FOUND_VALUE(gstUseSelection))
	{
		m_use_sel_gen_chk->SetValue(glbin_comp_generator.GetUseSel());
		m_use_sel_anl_chk->SetValue(glbin_comp_analyzer.GetUseSel());
	}

	if (update_all || FOUND_VALUE(gstUseMachineLearning))
		m_use_ml_chk->SetValue(glbin_comp_generator.GetUseMl());

	bool brush_update = FOUND_VALUE(gstBrushCountAutoUpdate);
	if (FOUND_VALUE(gstCompAutoUpdate) ||
		brush_update)
	{
		auto mode = glbin_vol_selector.GetSelectMode();
		if (mode == flrd::SelectMode::Segment ||
			mode == flrd::SelectMode::Mesh)
			return;
		if (brush_update)
		{
			if (glbin_comp_generator.GetUseSel())
			{
				auto vd = glbin_comp_generator.GetVolumeData();
				if (!vd->GetLabel(false))
					return;
			}
			else
				return;
		}
		if (glbin_comp_generator.GetAutoCompGen())
			LaunchAutoUpdateTimer();
	}

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
		m_density_window_size_sldr->ChangeValue(ival);
		m_density_window_size_text->ChangeValue(wxString::Format("%d", ival));
	}
	if (update_all || FOUND_VALUE(gstDensityStatsSize))
	{
		ival = glbin_comp_generator.GetDensityStatSize();
		m_density_stats_size_sldr->ChangeValue(ival);
		m_density_stats_size_text->ChangeValue(wxString::Format("%d", ival));
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
		m_dist_filter_size_sldr->ChangeValue(ival);
		m_dist_filter_size_text->ChangeValue(wxString::Format("%d", ival));
	}
	if (update_all || FOUND_VALUE(gstMaxDist))
	{
		ival = glbin_comp_generator.GetMaxDist();
		m_max_dist_sldr->ChangeValue(ival);
		m_max_dist_text->ChangeValue(wxString::Format("%d", ival));
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
		dval = glbin_clusterizer.GetEps();
		m_cluster_eps_sldr->ChangeValue(std::round(dval * 10.0));
		m_cluster_eps_text->SetValue(wxString::Format("%.1f", dval));
	}

	//analysis page
	//id text
	if (update_all || FOUND_VALUE(gstCompIdColor))
	{
		fluo::Color color = glbin_comp_editor.GetColor();
		wxColor wxc = wxColor(color.r() * 255, color.g() * 255, color.b() * 255);
		m_comp_id_text->SetBackgroundColour(wxc);
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

	//analyzer settings
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
		bval = glbin_comp_analyzer.GetUseDistNeighbor();
		m_dist_neighbor_check->SetValue(bval);
		m_dist_neighbor_sldr->Enable(bval);
		m_dist_neighbor_text->Enable(bval);
	}
	if (update_all || FOUND_VALUE(gstDistNeighborValue))
	{
		ival = glbin_comp_analyzer.GetDistNeighborNum();
		m_dist_neighbor_sldr->ChangeValue(ival);
		m_dist_neighbor_text->ChangeValue(wxString::Format("%d", ival));
	}
	if (update_all || FOUND_VALUE(gstDistAllChan))
	{
		bval = glbin_comp_analyzer.GetUseDistAllchan();
		m_dist_all_chan_check->SetValue(bval);
	}

	//align center
	if (update_all || FOUND_VALUE(gstAlignCenter))
	{
		bval = glbin_aligner.GetAlignCenter();
		m_align_center_chk->SetValue(bval);
	}

	//output
	if (FOUND_VALUE(gstCompGenOutput))
	{
		DeleteGridRows();
		wxString str1, str2;
		str1 = glbin_comp_generator.GetTitles();
		str2 = glbin_comp_generator.GetValues();
		OutputAnalysis(str1, str2);
	}

	if (FOUND_VALUE(gstCompAnalysisResult))
	{
		DeleteGridRows();
		size_t size = glbin_comp_analyzer.GetListSize();
		bool saved = false;
		if (size > m_max_lines)
		{
			ModalDlg fopendlg(this,
				wxString::Format("Component count is over %d. Save in a file?", m_max_lines),
				"", "", "Text file (*.txt)|*.txt",
				wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
			int rval = fopendlg.ShowModal();
			if (rval == wxID_OK)
			{
				wxString filename = fopendlg.GetPath();
				std::wstring str = filename.ToStdWstring();
				glbin_comp_analyzer.OutputCompListFile(str, 1);
				saved = true;
			}
		}
		if (!saved)
		{
			std::string titles, values;
			glbin_comp_analyzer.OutputFormHeader(titles);
			glbin_comp_analyzer.OutputCompListStr(values, 0);
			wxString str1(titles), str2(values);
			OutputAnalysis(str1, str2);
		}
	}

	if (FOUND_VALUE(gstCompListSelection))
		UpdateCompSelection();
}

void ComponentDlg::OutputAnalysis(wxString& titles, wxString& values)
{
	wxString copy_data;
	wxString cur_field;
	wxString cur_line;
	int i, k;
	int id_idx = -1;

	m_supress_select = true;
	m_output_grid->BeginBatch();

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
	auto vd = glbin_current.vol_data.lock();
	unsigned long lval;
	wxColor color;

	Progress prg;
	prg.SetProgressFunc(glbin_data_manager.GetProgressFunc());
	prg.SetProgress(0, "Updating component list.");

	i = 0;
	copy_data = values;
	do
	{
		if (i % 10 == 0)
			prg.SetProgress(100.0 * i / m_max_lines, "Updating component list.");

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

	} while (copy_data.IsEmpty() == false &&
		i < m_max_lines);

	//delete columns and rows if the old has more
	if (!m_hold_history)
	{
		if (m_output_grid->GetNumberCols() > k)
			m_output_grid->DeleteCols(k,
				m_output_grid->GetNumberCols() - k);
		if (m_output_grid->GetNumberRows() > i)
			m_output_grid->DeleteRows(i,
				m_output_grid->GetNumberRows() - i);
	}

	//m_output_grid->AutoSizeColumns();
	m_output_grid->ClearSelection();
	m_output_grid->EndBatch();
	m_supress_select = false;

	prg.SetProgress(0, "");
}

//setting funcs for sliders
void ComponentDlg::SetIter(int val)
{
	glbin_comp_generator.SetIter(val);
	FluoUpdate({ gstIteration, gstCompAutoUpdate, gstRecordCmd });
}

void ComponentDlg::SetThresh(double val)
{
	glbin_comp_generator.SetThresh(val);
	FluoUpdate({ gstCompThreshold, gstCompAutoUpdate, gstRecordCmd });
}

void ComponentDlg::SetDistStrength(double val)
{
	glbin_comp_generator.SetDistStrength(val);
	FluoUpdate({ gstDistFieldStrength, gstCompAutoUpdate, gstRecordCmd });
}

void ComponentDlg::SetDistFilterSize(int val)
{
	glbin_comp_generator.SetDistFilterSize(val);
	FluoUpdate({ gstDistFieldFilterSize, gstCompAutoUpdate, gstRecordCmd });
}

void ComponentDlg::SetMaxDist(int val)
{
	glbin_comp_generator.SetMaxDist(val);
	FluoUpdate({ gstMaxDist, gstCompAutoUpdate, gstRecordCmd });
}

void ComponentDlg::SetDistThresh(double val)
{
	glbin_comp_generator.SetDistThresh(val);
	FluoUpdate({ gstDistFieldThresh, gstCompAutoUpdate, gstRecordCmd });
}

void ComponentDlg::SetFalloff(double val)
{
	glbin_comp_generator.SetFalloff(val);
	FluoUpdate({ gstDiffusionFalloff, gstCompAutoUpdate, gstRecordCmd });
}

void ComponentDlg::SetDensity(double val)
{
	glbin_comp_generator.SetDensityThresh(val);
	FluoUpdate({ gstDensityFieldThresh, gstCompAutoUpdate, gstRecordCmd });
}

void ComponentDlg::SetVarth(double val)
{
	glbin_comp_generator.SetVarThresh(val);
	FluoUpdate({ gstDensityVarThresh, gstCompAutoUpdate, gstRecordCmd });
}

void ComponentDlg::SetDensityWindowSize(int val)
{
	glbin_comp_generator.SetDensityWinSize(val);
	FluoUpdate({ gstDensityWindowSize, gstCompAutoUpdate, gstRecordCmd });
}

void ComponentDlg::SetDensityStatsSize(int val)
{
	glbin_comp_generator.SetDensityStatSize(val);
	FluoUpdate({ gstDensityStatsSize, gstCompAutoUpdate, gstRecordCmd });
}

void ComponentDlg::SetFixSize(int val)
{
	glbin_comp_generator.SetFixSize(val);
	FluoUpdate({ gstFixateSize, gstCompAutoUpdate, gstRecordCmd });
}

void ComponentDlg::SetCleanIter(int val)
{
	glbin_comp_generator.SetCleanIter(val);
	FluoUpdate({ gstCleanIteration, gstCompAutoUpdate, gstRecordCmd });
}

void ComponentDlg::SetCleanLimit(int val)
{
	glbin_comp_generator.SetCleanSize(val);
	FluoUpdate({ gstCleanSize, gstCompAutoUpdate, gstRecordCmd });
}

//comp generate page
void ComponentDlg::OnIterSldr(wxScrollEvent& event)
{
	int val = m_iter_sldr->GetValue();
	wxString str = wxString::Format("%d", val);
	if (str != m_iter_text->GetValue())
	{
		m_iter_text->ChangeValue(str);
		SetIter(val);
	}
}

void ComponentDlg::OnIterText(wxCommandEvent& event)
{
	long val = 0;
	m_iter_text->GetValue().ToLong(&val);
	m_iter_sldr->ChangeValue(val);
	SetIter(val);
}

void ComponentDlg::OnThreshSldr(wxScrollEvent& event)
{
	double val = m_thresh_sldr->GetValue() / 1000.0;
	wxString str = wxString::Format("%.3f", val);
	if (str != m_thresh_text->GetValue())
	{
		m_thresh_text->SetValue(str);
		SetThresh(val);
	}
}

void ComponentDlg::OnThreshText(wxCommandEvent& event)
{
	double val = 0.0;
	m_thresh_text->GetValue().ToDouble(&val);
	m_thresh_sldr->ChangeValue(std::round(val * 1000.0));
	SetThresh(val);
}

void ComponentDlg::OnDistStrengthSldr(wxScrollEvent& event)
{
	double val = m_dist_strength_sldr->GetValue() / 1000.0;
	wxString str = wxString::Format("%.3f", val);
	if (str != m_dist_strength_text->GetValue())
	{
		m_dist_strength_text->SetValue(str);
		SetDistStrength(val);
	}
}

void ComponentDlg::OnDistStrengthText(wxCommandEvent& event)
{
	double val = 0.0;
	m_dist_strength_text->GetValue().ToDouble(&val);
	m_dist_strength_sldr->ChangeValue(std::round(val * 1000.0));
	SetDistStrength(val);
}

void ComponentDlg::OnUseDistFieldCheck(wxCommandEvent& event)
{
	bool bval = m_use_dist_field_check->GetValue();
	glbin_comp_generator.SetUseDistField(bval);
	FluoUpdate({ gstUseDistField, gstCompAutoUpdate, gstRecordCmd });
}

void ComponentDlg::OnDistFilterSizeSldr(wxScrollEvent& event)
{
	int val = m_dist_filter_size_sldr->GetValue();
	wxString str = wxString::Format("%d", val);
	if (str != m_dist_filter_size_text->GetValue())
	{
		m_dist_filter_size_text->SetValue(str);
		SetDistFilterSize(val);
	}
}

void ComponentDlg::OnDistFilterSizeText(wxCommandEvent& event)
{
	long val = 0;
	m_dist_filter_size_text->GetValue().ToLong(&val);
	m_dist_filter_size_sldr->ChangeValue(val);
	SetDistFilterSize(val);
}

void ComponentDlg::OnMaxDistSldr(wxScrollEvent& event)
{
	int val = m_max_dist_sldr->GetValue();
	wxString str = wxString::Format("%d", val);
	if (str != m_max_dist_text->GetValue())
	{
		m_max_dist_text->SetValue(str);
		SetMaxDist(val);
	}
}

void ComponentDlg::OnMaxDistText(wxCommandEvent& event)
{
	long val = 1;
	m_max_dist_text->GetValue().ToLong(&val);
	if (val > 255)
		val = 255;
	m_max_dist_sldr->ChangeValue(val);
	SetMaxDist(val);
}

void ComponentDlg::OnDistThreshSldr(wxScrollEvent& event)
{
	double val = m_dist_thresh_sldr->GetValue() / 1000.0;
	wxString str = wxString::Format("%.3f", val);
	if (str != m_dist_thresh_text->GetValue())
	{
		m_dist_thresh_text->SetValue(str);
		SetDistThresh(val);
	}
}

void ComponentDlg::OnDistThreshText(wxCommandEvent& event)
{
	double val = 0.0;
	m_dist_thresh_text->GetValue().ToDouble(&val);
	m_dist_thresh_sldr->ChangeValue(std::round(val * 1000.0));
	SetDistThresh(val);
}

void ComponentDlg::OnDiffCheck(wxCommandEvent& event)
{
	bool bval = m_diff_check->GetValue();
	glbin_comp_generator.SetDiffusion(bval);
	FluoUpdate({ gstUseDiffusion, gstCompAutoUpdate, gstRecordCmd });
}

void ComponentDlg::OnFalloffSldr(wxScrollEvent& event)
{
	double val = m_falloff_sldr->GetValue() / 1000.0;
	wxString str = wxString::Format("%.3f", val);
	if (str != m_falloff_text->GetValue())
	{
		m_falloff_text->SetValue(str);
		SetFalloff(val);
	}
}

void ComponentDlg::OnFalloffText(wxCommandEvent& event)
{
	double val = 0.0;
	m_falloff_text->GetValue().ToDouble(&val);
	m_falloff_sldr->ChangeValue(std::round(val * 1000.0));
	SetFalloff(val);
}

void ComponentDlg::OnDensityCheck(wxCommandEvent& event)
{
	bool bval = m_density_check->GetValue();
	glbin_comp_generator.SetDensity(bval);
	FluoUpdate({ gstUseDensityField, gstCompAutoUpdate, gstRecordCmd });
}

void ComponentDlg::OnDensitySldr(wxScrollEvent& event)
{
	double val = m_density_sldr->GetValue() / 1000.0;
	wxString str = wxString::Format("%.3f", val);
	if (str != m_density_text->GetValue())
	{
		m_density_text->SetValue(str);
		SetDensity(val);
	}
}

void ComponentDlg::OnDensityText(wxCommandEvent& event)
{
	double val = 0.0;
	m_density_text->GetValue().ToDouble(&val);
	m_density_sldr->ChangeValue(std::round(val * 1000.0));
	SetDensity(val);
}

void ComponentDlg::OnVarthSldr(wxScrollEvent& event)
{
	double val = m_varth_sldr->GetValue() / 10000.0;
	wxString str = wxString::Format("%.4f", val);
	if (str != m_varth_text->GetValue())
	{
		m_varth_text->SetValue(str);
		SetVarth(val);
	}
}

void ComponentDlg::OnVarthText(wxCommandEvent& event)
{
	double val = 0.0;
	m_varth_text->GetValue().ToDouble(&val);
	m_varth_sldr->ChangeValue(std::round(val * 10000.0));
	SetVarth(val);
}

void ComponentDlg::OnDensityWindowSizeSldr(wxScrollEvent& event)
{
	int val = m_density_window_size_sldr->GetValue();
	wxString str = wxString::Format("%d", val);
	if (str != m_density_window_size_text->GetValue())
	{
		m_density_window_size_text->SetValue(str);
		SetDensityWindowSize(val);
	}
}

void ComponentDlg::OnDensityWindowSizeText(wxCommandEvent& event)
{
	long val = 0;
	m_density_window_size_text->GetValue().ToLong(&val);
	m_density_window_size_sldr->ChangeValue(val);
	SetDensityWindowSize(val);
}

void ComponentDlg::OnDensityStatsSizeSldr(wxScrollEvent& event)
{
	int val = m_density_stats_size_sldr->GetValue();
	wxString str = wxString::Format("%d", val);
	if (str != m_density_stats_size_text->GetValue())
	{
		m_density_stats_size_text->SetValue(str);
		SetDensityStatsSize(val);
	}
}

void ComponentDlg::OnDensityStatsSizeText(wxCommandEvent& event)
{
	long val = 0;
	m_density_stats_size_text->GetValue().ToLong(&val);
	m_density_stats_size_sldr->ChangeValue(val);
	SetDensityStatsSize(val);
}

void ComponentDlg::OnFixateCheck(wxCommandEvent& event)
{
	bool bval = m_fixate_check->GetValue();
	glbin_comp_generator.SetFixate(bval);

	if (bval)
		glbin_comp_generator.Fixate();

	if (glbin_comp_generator.GetAutoCompGen())
	{
		bval = glbin_comp_generator.GetClean();
		glbin_comp_generator.SetClean(false);
		glbin_comp_generator.SetUseSel(m_use_sel_gen_chk->GetValue());
		glbin_comp_generator.GenerateComp(false);
		glbin_comp_generator.SetClean(bval);
		FluoRefresh(2, { gstFixateEnable });
	}
	else
		FluoUpdate({ gstFixateEnable });
}

void ComponentDlg::OnGrowFixedCheck(wxCommandEvent& event)
{
	bool bval = m_grow_fixed_check->GetValue();
	glbin_comp_generator.SetGrowFixed(bval);

	if (glbin_comp_generator.GetAutoCompGen())
	{
		bval = glbin_comp_generator.GetClean();
		glbin_comp_generator.SetClean(false);
		glbin_comp_generator.SetUseSel(m_use_sel_gen_chk->GetValue());
		glbin_comp_generator.GenerateComp(false);
		glbin_comp_generator.SetClean(bval);
		FluoRefresh(3, { gstNull });
	}
}

void ComponentDlg::OnFixUpdateBtn(wxCommandEvent& event)
{
	glbin_comp_generator.Fixate();

	if (glbin_comp_generator.GetAutoCompGen())
	{
		bool bval = glbin_comp_generator.GetClean();
		glbin_comp_generator.SetClean(false);
		glbin_comp_generator.SetUseSel(m_use_sel_gen_chk->GetValue());
		glbin_comp_generator.GenerateComp(false);
		glbin_comp_generator.SetClean(bval);
		FluoRefresh(3, { gstNull });
	}
}

void ComponentDlg::OnFixSizeSldr(wxScrollEvent& event)
{
	int val = m_fix_size_sldr->GetValue();
	wxString str = wxString::Format("%d", val);
	if (str != m_fix_size_text->GetValue())
	{
		m_fix_size_text->SetValue(str);
		SetFixSize(val);
	}
}

void ComponentDlg::OnFixSizeText(wxCommandEvent& event)
{
	long val = 0;
	m_fix_size_text->GetValue().ToLong(&val);
	m_fix_size_sldr->ChangeValue(val);
	SetFixSize(val);
}

void ComponentDlg::OnCleanCheck(wxCommandEvent& event)
{
	bool bval = m_clean_check->GetValue();
	glbin_comp_generator.SetClean(bval);
	FluoUpdate({ gstCleanEnable, gstCompAutoUpdate, gstRecordCmd });
}

void ComponentDlg::OnCleanBtn(wxCommandEvent& event)
{
	bool bval = m_use_sel_gen_chk->GetValue();
	glbin_comp_generator.SetUseSel(bval);
	glbin_comp_generator.Clean();
	FluoRefresh(3, { gstNull });
}

void ComponentDlg::OnCleanIterSldr(wxScrollEvent& event)
{
	int val = m_clean_iter_sldr->GetValue();
	wxString str = wxString::Format("%d", val);
	if (str != m_clean_iter_text->GetValue())
	{
		m_clean_iter_text->SetValue(str);
		SetCleanIter(val);
	}
}

void ComponentDlg::OnCleanIterText(wxCommandEvent& event)
{
	long val = 0;
	m_clean_iter_text->GetValue().ToLong(&val);
	m_clean_iter_sldr->ChangeValue(val);
	SetCleanIter(val);
}

void ComponentDlg::OnCleanLimitSldr(wxScrollEvent& event)
{
	int val = m_clean_limit_sldr->GetValue();
	wxString str = wxString::Format("%d", val);
	if (str != m_clean_limit_text->GetValue())
	{
		m_clean_limit_text->SetValue(str);
		SetCleanLimit(val);
	}
}

void ComponentDlg::OnCleanLimitText(wxCommandEvent& event)
{
	long val = 0;
	m_clean_limit_text->GetValue().ToLong(&val);
	m_clean_limit_sldr->ChangeValue(val);
	SetCleanLimit(val);
}

//record
void ComponentDlg::OnRecordCmd(wxCommandEvent& event)
{
	bool val = m_record_cmd_btn->GetValue();
	glbin_comp_generator.SetRecordCmd(val);
}

void ComponentDlg::OnPlayCmd(wxCommandEvent& event)
{
	bool bval = m_use_sel_gen_chk->GetValue();
	glbin_comp_generator.SetUseSel(bval);
	glbin_comp_generator.PlayCmd(1.0);
	FluoRefresh(3, { gstNull });
}

void ComponentDlg::OnResetCmd(wxCommandEvent& event)
{
	glbin_comp_generator.ResetCmd();
	FluoUpdate({ gstRecordCmd });
}

void ComponentDlg::OnLoadCmd(wxCommandEvent& event)
{
	ModalDlg fopendlg(
		m_frame, "Choose a FluoRender component generator macro command",
		"", "", "*.txt;*.dft", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	int rval = fopendlg.ShowModal();
	if (rval != wxID_OK)
		return;
	std::wstring filename = fopendlg.GetPath().ToStdWstring();

	glbin_comp_generator.LoadCmd(filename);
	FluoUpdate({ gstRecordCmd });
}

void ComponentDlg::OnSaveCmd(wxCommandEvent& event)
{
	ModalDlg fopendlg(
		m_frame, "Save a FluoRender component generator macro command",
		"", "", "*.txt", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	int rval = fopendlg.ShowModal();
	if (rval != wxID_OK)
		return;

	std::wstring filename = fopendlg.GetPath().ToStdWstring();

	glbin_comp_generator.SaveCmd(filename);
}

//auto update
void ComponentDlg::LaunchAutoUpdateTimer()
{
	if (!glbin_comp_generator.GetAutoCompGen())
		return;
	m_auto_update_timer.Start(100);
}

void ComponentDlg::OnAutoUpdateTimer(wxTimerEvent& event)
{
	if (glbin_comp_generator.IsBusy())
		return;
	m_auto_update_timer.Stop();
	fluo::ValueCollection vc;
	bool bval = m_use_sel_gen_chk->GetValue();
	glbin_comp_generator.SetUseSel(bval);
	if (glbin_comp_generator.GetAutoThreshold())
		vc.insert({ gstBrushThreshold, gstCompThreshold, gstVolMeshThresh });
	glbin_comp_generator.GenerateComp();
	vc.insert(gstCompGenOutput);
	FluoRefresh(2, vc);
}

//clustering page
void ComponentDlg::OnClusterMethodCheck(wxCommandEvent& event)
{
	int id = event.GetId();
	glbin_clusterizer.SetMethod(id);
	FluoUpdate({ gstClusterMethod });
}

//parameters
void ComponentDlg::OnClusterClnumSldr(wxScrollEvent& event)
{
	int val = m_cluster_clnum_sldr->GetValue();
	wxString str = wxString::Format("%d", val);
	if (str != m_cluster_clnum_text->GetValue())
		m_cluster_clnum_text->SetValue(str);
}

void ComponentDlg::OnClusterClnumText(wxCommandEvent& event)
{
	long val = 0;
	m_cluster_clnum_text->GetValue().ToLong(&val);
	m_cluster_clnum_sldr->ChangeValue(val);
	glbin_clusterizer.SetNum(val);
}

void ComponentDlg::OnClusterMaxiterSldr(wxScrollEvent& event)
{
	int val = m_cluster_maxiter_sldr->GetValue();
	wxString str = wxString::Format("%d", val);
	if (str != m_cluster_maxiter_text->GetValue())
		m_cluster_maxiter_text->SetValue(str);
}

void ComponentDlg::OnClusterMaxiterText(wxCommandEvent& event)
{
	long val = 0;
	m_cluster_maxiter_text->GetValue().ToLong(&val);
	m_cluster_maxiter_sldr->ChangeValue(val);
	glbin_clusterizer.SetMaxIter(val);
}

void ComponentDlg::OnClusterTolSldr(wxScrollEvent& event)
{
	int val = m_cluster_tol_sldr->GetValue();
	wxString str = wxString::Format("%.2f", double(val) / 100.0);
	if (str != m_cluster_tol_text->GetValue())
		m_cluster_tol_text->SetValue(str);
}

void ComponentDlg::OnClusterTolText(wxCommandEvent& event)
{
	double val = 0.9;
	m_cluster_tol_text->GetValue().ToDouble(&val);
	m_cluster_tol_sldr->ChangeValue(std::round(val * 100));
	glbin_clusterizer.SetTol((float)val);
}

void ComponentDlg::OnClusterSizeSldr(wxScrollEvent& event)
{
	int val = m_cluster_size_sldr->GetValue();
	wxString str = wxString::Format("%d", val);
	if (str != m_cluster_size_text->GetValue())
		m_cluster_size_text->SetValue(str);
}

void ComponentDlg::OnClusterSizeText(wxCommandEvent& event)
{
	long val = 0;
	m_cluster_size_text->GetValue().ToLong(&val);
	m_cluster_size_sldr->ChangeValue(val);
	glbin_clusterizer.SetSize(val);
}

void ComponentDlg::OnClusterEpsSldr(wxScrollEvent& event)
{
	double val = m_cluster_eps_sldr->GetValue() / 10.0;
	wxString str = wxString::Format("%.1f", val);
	if (str != m_cluster_eps_text->GetValue())
		m_cluster_eps_text->SetValue(str);
}

void ComponentDlg::OnClusterepsText(wxCommandEvent& event)
{
	double val = 0.0;
	m_cluster_eps_text->GetValue().ToDouble(&val);
	m_cluster_eps_sldr->ChangeValue(std::round(val * 10.0));
	glbin_clusterizer.SetEps(val);
}

//analysis page
void ComponentDlg::OnCompIdText(wxCommandEvent& event)
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

void ComponentDlg::OnCompIdXBtn(wxCommandEvent& event)
{
	m_comp_id_text->Clear();
	FluoUpdate({ gstCompIdColor });
}

void ComponentDlg::OnAnalysisMinCheck(wxCommandEvent& event)
{
	bool bval = m_analysis_min_check->GetValue();
	m_analysis_min_spin->Enable(bval);
	glbin_comp_selector.SetUseMin(bval);
	glbin_comp_analyzer.SetUseMin(bval);
	FluoUpdate({ gstUseMin });
}

void ComponentDlg::OnAnalysisMinSpin(wxSpinEvent& event)
{
	int val = m_analysis_min_spin->GetValue();
	glbin_comp_selector.SetMinNum(val);
	glbin_comp_analyzer.SetMinNum(val);
}

void ComponentDlg::OnAnalysisMinText(wxCommandEvent& event)
{
	int val = m_analysis_min_spin->GetValue();
	glbin_comp_selector.SetMinNum(val);
	glbin_comp_analyzer.SetMinNum(val);
}

void ComponentDlg::OnAnalysisMaxCheck(wxCommandEvent& event)
{
	bool bval = m_analysis_max_check->GetValue();
	m_analysis_max_spin->Enable(bval);
	glbin_comp_selector.SetUseMax(bval);
	glbin_comp_analyzer.SetUseMax(bval);
	FluoUpdate({ gstUseMax });
}

void ComponentDlg::OnAnalysisMaxSpin(wxSpinEvent& event)
{
	int val = m_analysis_max_spin->GetValue();
	glbin_comp_selector.SetMaxNum(val);
	glbin_comp_analyzer.SetMaxNum(val);
}

void ComponentDlg::OnAnalysisMaxText(wxCommandEvent& event)
{
	int val = m_analysis_max_spin->GetValue();
	glbin_comp_selector.SetMaxNum(val);
	glbin_comp_analyzer.SetMaxNum(val);
}

void ComponentDlg::OnCompFull(wxCommandEvent& event)
{
	glbin_comp_selector.SelectFullComp();
	FluoRefresh(0, { gstCompAnalysisResult, gstSelUndo });
}

void ComponentDlg::OnCompExclusive(wxCommandEvent& event)
{
	glbin_comp_selector.Exclusive();
	FluoRefresh(0, { gstCompAnalysisResult, gstSelUndo, gstBrushCountAutoUpdate, gstColocalAutoUpdate }, { glbin_current.GetViewId() });
}

void ComponentDlg::OnCompAppend(wxCommandEvent& event)
{
	bool get_all = glbin_comp_selector.GetIdEmpty();
	glbin_comp_selector.Select(get_all);
	FluoRefresh(0, { gstCompAnalysisResult, gstSelUndo, gstBrushCountAutoUpdate, gstColocalAutoUpdate }, { glbin_current.GetViewId() });
}

void ComponentDlg::OnCompAll(wxCommandEvent& event)
{
	glbin_comp_selector.All();
	FluoRefresh(0, { gstCompAnalysisResult, gstSelUndo, gstBrushCountAutoUpdate, gstColocalAutoUpdate }, { glbin_current.GetViewId() });
}

void ComponentDlg::OnCompClear(wxCommandEvent& event)
{
	glbin_vol_selector.Clear();
	glbin_comp_selector.Clear();
	FluoRefresh(0, { gstCompAnalysisResult, gstSelUndo });
}

void ComponentDlg::OnShuffle(wxCommandEvent& event)
{
	//get current vd
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;

	vd->IncShuffle();
	FluoRefresh(2, { gstCompAnalysisResult });
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
	glbin_comp_editor.ReplaceId();
	FluoRefresh(3, { gstNull });
}

void ComponentDlg::OnCompCleanBkg(wxCommandEvent& event)
{
	glbin_comp_editor.Clean(0);
	FluoRefresh(3, { gstNull });
}

void ComponentDlg::OnCompCombine(wxCommandEvent& event)
{
	glbin_comp_editor.CombineId();
	FluoRefresh(3, { gstNull });
}

void ComponentDlg::OnConSizeSldr(wxScrollEvent& event)
{
	int val = m_con_size_sldr->GetValue();
	wxString str = wxString::Format("%d", val);
	if (str != m_con_size_text->GetValue())
		m_con_size_text->SetValue(str);
}

void ComponentDlg::OnConSizeText(wxCommandEvent& event)
{
	long val = 0;
	m_con_size_text->GetValue().ToLong(&val);
	m_con_size_sldr->ChangeValue(val);
	glbin_comp_analyzer.SetSizeLimit(val);
}

void ComponentDlg::OnConsistentCheck(wxCommandEvent& event)
{
	bool bval = m_consistent_check->GetValue();
	glbin_comp_analyzer.SetConsistent(bval);
}

void ComponentDlg::OnColocalCheck(wxCommandEvent& event)
{
	bool bval = m_colocal_check->GetValue();
	glbin_comp_analyzer.SetColocal(bval);
}

//output
void ComponentDlg::OnOutputTypeRadio(wxCommandEvent& event)
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
	int val = 1;
	if (id == ID_OutputRandomBtn)
		val = 1;
	else if (id == ID_OutputSizeBtn)
		val = 2;

	glbin_comp_analyzer.SetColorType(val);
	glbin_comp_analyzer.OutputChannels();
	FluoRefresh(0, { gstListCtrl, gstTreeCtrl });
}

void ComponentDlg::OnOutputAnnotData(wxCommandEvent& event)
{
	int id = event.GetId();
	int val = 0;
	if (id == ID_OutputIdBtn)
		val = 1;
	else if (id == ID_OutputSnBtn)
		val = 2;

	glbin_comp_analyzer.SetAnnotType(val);
	glbin_comp_analyzer.OutputAnnotData();
	FluoRefresh(0, { gstListCtrl, gstTreeCtrl });
}

void ComponentDlg::OnOutputMeshData(wxCommandEvent& event)
{
	auto vd = glbin_current.vol_data.lock();
	glbin_conv_vol_mesh->SetVolumeData(vd);
	glbin_conv_vol_mesh->Convert();
	glbin_conv_vol_mesh->MergeVertices(true);
	auto md = glbin_conv_vol_mesh->GetMeshData();
	glbin_color_comp_mesh.SetVolumeData(vd);
	glbin_color_comp_mesh.SetMeshData(md);
	glbin_color_comp_mesh.Update();
	if (md)
	{
		glbin_data_manager.AddMeshData(md);
		auto view = glbin_current.render_view.lock();
		if (view)
			view->AddMeshData(md);
	}
	FluoRefresh(0, { gstVolMeshThresh, gstVolMeshInfo, gstListCtrl, gstTreeCtrl },
		{ glbin_current.GetViewId() });
}

//distance
void ComponentDlg::OnDistNeighborCheck(wxCommandEvent& event)
{
	bool bval = m_dist_neighbor_check->GetValue();
	glbin_comp_analyzer.SetUseDistNeighbor(bval);
	FluoUpdate({ gstDistNeighbor });
}

void ComponentDlg::OnDistAllChanCheck(wxCommandEvent& event)
{
	bool bval = m_dist_all_chan_check->GetValue();
	glbin_comp_analyzer.SetUseDistAllchan(bval);
}

void ComponentDlg::OnDistNeighborSldr(wxScrollEvent& event)
{
	int val = m_dist_neighbor_sldr->GetValue();
	wxString str = wxString::Format("%d", val);
	if (str != m_dist_neighbor_text->GetValue())
		m_dist_neighbor_text->SetValue(str);
}

void ComponentDlg::OnDistNeighborText(wxCommandEvent& event)
{
	long val = 0;
	m_dist_neighbor_text->GetValue().ToLong(&val);
	m_dist_neighbor_sldr->ChangeValue(val);
	glbin_comp_analyzer.SetDistNeighborNum(val);
}

void ComponentDlg::OnDistOutput(wxCommandEvent& event)
{
	OutputDistance();
}

void ComponentDlg::OnAlignCenterChk(wxCommandEvent& event)
{
	bool bval = m_align_center_chk->GetValue();
	glbin_aligner.SetAlignCenter(bval);
	FluoRefresh(1, { gstAlignCenter }, { -1 });
}

void ComponentDlg::OnAlignPca(wxCommandEvent& event)
{
	AlignPca(event.GetId());
	FluoRefresh(3, { gstNull }, { glbin_current.GetViewId()});
}

void ComponentDlg::OnSize(wxSizeEvent& event)
{
	if (!m_output_grid)
		return;

	wxSize size = GetSize();
	wxPoint p1 = GetScreenPosition();
	wxPoint p2 = m_output_grid->GetScreenPosition();
	int height, margin;
	if (m_output_grid->GetNumberRows())
		height = m_output_grid->GetRowSize(0) * 8;
	else
		height = 80;
	margin = size.y + p1.y - p2.y - 20;
	if (margin > height)
		size.y = margin;
	else
		size.y = height;
	size.x -= 15;
	m_output_grid->SetMaxSize(size);
}

void ComponentDlg::OnUseSelChk(wxCommandEvent& event)
{
	bool bval = event.IsChecked();
	glbin_comp_generator.SetUseSel(bval);
	glbin_comp_analyzer.SetUseSel(bval);
}

void ComponentDlg::OnUseMlChk(wxCommandEvent& event)
{
	bool bval = m_use_ml_chk->GetValue();
	glbin_comp_generator.SetUseMl(bval);
}

void ComponentDlg::OnGenerate(wxCommandEvent& event)
{
	fluo::ValueCollection vc;
	bool bval = m_use_sel_gen_chk->GetValue();
	glbin_comp_generator.SetUseSel(bval);
	if (glbin_comp_generator.GetAutoThreshold())
		vc.insert({ gstBrushThreshold, gstCompThreshold, gstVolMeshThresh });
	glbin_comp_generator.Compute();
	vc.insert({ gstCompGenOutput, gstRecordCmd });
	FluoRefresh(2, vc);
}

void ComponentDlg::OnCluster(wxCommandEvent& event)
{
	glbin_clusterizer.Compute();
	FluoRefresh(3, { gstNull });
}

void ComponentDlg::OnAnalyze(wxCommandEvent& event)
{
	glbin_comp_analyzer.Analyze();
	FluoUpdate({ gstCompAnalysisResult });
}

void ComponentDlg::OnIncludeBtn(wxCommandEvent& event)
{
	IncludeComps();
}

void ComponentDlg::OnExcludeBtn(wxCommandEvent& event)
{
	ExcludeComps();
}

void ComponentDlg::OnHistoryChk(wxCommandEvent& event)
{
	m_hold_history = m_history_chk->GetValue();
}

void ComponentDlg::OnClearHistBtn(wxCommandEvent& event)
{
	DeleteGridRows();
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
}

void ComponentDlg::OnSelectCell(wxGridEvent& event)
{
	if (m_supress_select)
		return;

	CallAfter([this] {
		UpdateSelectedRows();
		SelectGridCells();
	});

	event.Skip();
}

void ComponentDlg::OnRangeSelect(wxGridRangeSelectEvent& event)
{
	if (m_supress_select)
		return;

	CallAfter([this] {
		UpdateSelectedRows();
		SelectGridCells();
	});

	event.Skip();
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

void ComponentDlg::OutputDistance()
{
	ModalDlg fopendlg(
		this, "Save Analysis Data", "", "",
		"Text file (*.txt)|*.txt",
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	int rval = fopendlg.ShowModal();
	if (rval == wxID_OK)
	{
		wxString filename = fopendlg.GetPath();
		std::string str = filename.ToStdString();
		std::ofstream outfile;
		outfile.open(str, std::ofstream::out);
		//output result matrix
		glbin_comp_analyzer.OutputDistance(outfile);
		outfile.close();
	}
}

void ComponentDlg::AlignPca(int axis_type)
{
	flrd::RulerList list;
	glbin_comp_analyzer.GetRulerListFromCelp(list);
	glbin_aligner.SetRulerList(&list);
	glbin_aligner.SetAxisType(axis_type);
	glbin_aligner.SetView(glbin_current.render_view.lock());
	glbin_aligner.AlignPca(true);
	FluoRefresh(3, { gstNull }, { glbin_current.GetViewId() });
}

void ComponentDlg::IncludeComps()
{
	//get list of selected comps
	wxArrayInt seli = m_output_grid->GetSelectedCols();
	bool sel_all = seli.GetCount();
	flrd::CelpList cl;
	if (!sel_all)
	{
		std::vector<unsigned int> ids;
		std::vector<unsigned int> bids;
		seli = m_output_grid->GetSelectedRows();
		int bn = glbin_comp_analyzer.GetBrickNum();
		AddSelArrayInt(ids, bids, seli, bn > 1);
		glbin_comp_analyzer.SetSelectedIds(ids, bids);
		glbin_comp_analyzer.GetSelectedCelp(cl, true);
	}
	else
		glbin_comp_analyzer.GetAllCelp(cl, true);

	glbin_comp_selector.SetList(cl);
	glbin_comp_selector.SelectList();

	FluoRefresh(2, { gstCompAnalysisResult });
}

void ComponentDlg::ExcludeComps()
{
	//get list of selected comps
	wxArrayInt seli = m_output_grid->GetSelectedCols();
	bool sel_all = seli.GetCount();
	flrd::CelpList cl;
	if (!sel_all)
	{
		std::vector<unsigned int> ids;
		std::vector<unsigned int> bids;
		seli = m_output_grid->GetSelectedRows();
		int bn = glbin_comp_analyzer.GetBrickNum();
		AddSelArrayInt(ids, bids, seli, bn > 1);
		glbin_comp_analyzer.SetSelectedIds(ids, bids);
		glbin_comp_analyzer.GetSelectedCelp(cl, true);
	}
	else
		glbin_comp_analyzer.GetAllCelp(cl, true);

	glbin_comp_selector.SetList(cl);
	glbin_comp_selector.EraseList();

	FluoRefresh(2, { gstCompAnalysisResult });
}

void ComponentDlg::UpdateCompSelection()
{
	std::set<unsigned long long> ids;
	int mode = glbin_comp_selector.GetSelCompIdsMode();
	glbin_comp_selector.GetSelectedCompIds(ids);
	if (ids.empty())
	{
		m_output_grid->ClearSelection();
		return;
	}

	int bn = glbin_comp_analyzer.GetBrickNum();

	wxString str;
	unsigned long ulv;
	unsigned long long ull;
	bool flag = mode == 1;
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
		//SelectCompsCanvas();
		if (lasti >= 0)
			m_output_grid->GoToCell(lasti, 0);
	}
}

void ComponentDlg::SelectGridCells()
{
	int bn = glbin_comp_analyzer.GetBrickNum();
	std::vector<unsigned long long> ids;
	//selected cells are retrieved using different functions
	bool sel_all = false;
	//if (seli.GetCount() >= m_output_grid->GetNumberRows())
	//	sel_all = true;
	wxString str;
	unsigned long ulval;
	unsigned long long id;
	for (auto it : m_sel)
	{
		id = 0;
		str = m_output_grid->GetCellValue(it, 0);
		if (str.ToULong(&ulval))
			id = ulval;
		if (bn > 1)
		{
			str = m_output_grid->GetCellValue(it, 1);
			if (str.ToULong(&ulval))
				id = ((unsigned long long)(ulval) << 32) | id;
		}
		if (id)
			ids.push_back(id);
	}
	glbin_comp_selector.SelectCompsCanvas(ids, sel_all);
	FluoRefresh(3, { gstNull });
}

void ComponentDlg::DeleteGridRows()
{
	int rn = m_output_grid->GetNumberRows();
	if (rn)
	{
		m_output_grid->DeleteRows(0, rn);
		m_sel.clear();
	}
}

void ComponentDlg::AddSelArrayInt(std::vector<unsigned int>& ids,
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

void ComponentDlg::UpdateSelectedRows()
{
	m_sel.clear();

	// 1. Add individually selected cells
	const auto& cells = m_output_grid->GetSelectedCells();
	for (const auto& cell : cells)
		m_sel.insert(cell.GetRow());

	// 2. Add selected blocks
	const auto& topLeft = m_output_grid->GetSelectionBlockTopLeft();
	const auto& bottomRight = m_output_grid->GetSelectionBlockBottomRight();
	for (size_t i = 0; i < topLeft.size(); ++i)
	{
		int top = topLeft[i].GetRow();
		int bottom = bottomRight[i].GetRow();
		for (int row = top; row <= bottom; ++row)
			m_sel.insert(row);
	}

	// 3. Add the current cell if nothing else is selected
	if (cells.empty() && topLeft.empty())
	{
		int row = m_output_grid->GetGridCursorRow();
		if (row >= 0)
			m_sel.insert(row);
	}
}
