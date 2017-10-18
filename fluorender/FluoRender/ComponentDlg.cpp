/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2014 Scientific Computing and Imaging Institute,
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
#include "Components/CompGenerator.h"
#include "Components/CompSelector.h"
#include "Cluster/dbscan.h"
#include "Cluster/kmeans.h"
#include "Cluster/exmax.h"
#include <wx/valnum.h>
#include <wx/stdpaths.h>
#include <boost/signals2.hpp>
#include <boost/bind.hpp>
#include <limits>
#include <string>
#include <cctype>

BEGIN_EVENT_TABLE(ComponentDlg, wxPanel)
	EVT_COLLAPSIBLEPANE_CHANGED(wxID_ANY, ComponentDlg::OnPaneChange)
	//load save settings
	EVT_BUTTON(ID_LoadSettingsBtn, ComponentDlg::OnLoadSettings)
	EVT_BUTTON(ID_SaveSettingsBtn, ComponentDlg::OnSaveSettings)
	EVT_BUTTON(ID_SaveasSettingsBtn, ComponentDlg::OnSaveasSettings)
	//initial grow
	EVT_CHECKBOX(ID_InitialGrowCheck, ComponentDlg::OnInitialGrowCheck)
	EVT_CHECKBOX(ID_IGParamTransitionCheck, ComponentDlg::OnIGParamTransitionCheck)
	EVT_COMMAND_SCROLL(ID_IGIterationsSldr, ComponentDlg::OnIGIterationsSldr)
	EVT_TEXT(ID_IGIterationsText, ComponentDlg::OnIGIterationsText)
	//translate
	EVT_COMMAND_SCROLL(ID_IGTranslateSldr, ComponentDlg::OnIGTranslateSldr)
	EVT_TEXT(ID_IGTranslateText, ComponentDlg::OnIGTranslateText)
	EVT_COMMAND_SCROLL(ID_IGTranslate2Sldr, ComponentDlg::OnIGTranslate2Sldr)
	EVT_TEXT(ID_IGTranslate2Text, ComponentDlg::OnIGTranslate2Text)
	//scalar falloff
	EVT_COMMAND_SCROLL(ID_IGScalarFalloffSldr, ComponentDlg::OnIGScalarFalloffSldr)
	EVT_TEXT(ID_IGScalarFalloffText, ComponentDlg::OnIGScalarFalloffText)
	EVT_COMMAND_SCROLL(ID_IGScalarFalloff2Sldr, ComponentDlg::OnIGScalarFalloff2Sldr)
	EVT_TEXT(ID_IGScalarFalloff2Text, ComponentDlg::OnIGScalarFalloff2Text)
	//grad falloff
	EVT_COMMAND_SCROLL(ID_IGGradFalloffSldr, ComponentDlg::OnIGGradFalloffSldr)
	EVT_TEXT(ID_IGGradFalloffText, ComponentDlg::OnIGGradFalloffText)
	EVT_COMMAND_SCROLL(ID_IGGradFalloff2Sldr, ComponentDlg::OnIGGradFalloff2Sldr)
	EVT_TEXT(ID_IGGradFalloff2Text, ComponentDlg::OnIGGradFalloff2Text)
	//variance falloff
	EVT_COMMAND_SCROLL(ID_IGVarFalloffSldr, ComponentDlg::OnIGVarFalloffSldr)
	EVT_TEXT(ID_IGVarFalloffText, ComponentDlg::OnIGVarFalloffText)
	EVT_COMMAND_SCROLL(ID_IGVarFalloff2Sldr, ComponentDlg::OnIGVarFalloff2Sldr)
	EVT_TEXT(ID_IGVarFalloff2Text, ComponentDlg::OnIGVarFalloff2Text)
	//angle falloff
	EVT_COMMAND_SCROLL(ID_IGAngleFalloffSldr, ComponentDlg::OnIGAngleFalloffSldr)
	EVT_TEXT(ID_IGAngleFalloffText, ComponentDlg::OnIGAngleFalloffText)
	EVT_COMMAND_SCROLL(ID_IGAngleFalloff2Sldr, ComponentDlg::OnIGAngleFalloff2Sldr)
	EVT_TEXT(ID_IGAngleFalloff2Text, ComponentDlg::OnIGAngleFalloff2Text)

	//sized grow
	EVT_CHECKBOX(ID_SizedGrowCheck, ComponentDlg::OnSizedGrowCheck)
	EVT_CHECKBOX(ID_SGParamTransitionCheck, ComponentDlg::OnSGParamTransitionCheck)
	EVT_COMMAND_SCROLL(ID_SGIterationsSldr, ComponentDlg::OnSGIterationsSldr)
	EVT_TEXT(ID_SGIterationsText, ComponentDlg::OnSGIterationsText)
	//size limiter
	EVT_COMMAND_SCROLL(ID_SGSizeLimiterSldr, ComponentDlg::OnSGSizeLimiterSldr)
	EVT_TEXT(ID_SGSizeLimiterText, ComponentDlg::OnSGSizeLimiterText)
	EVT_COMMAND_SCROLL(ID_SGSizeLimiter2Sldr, ComponentDlg::OnSGSizeLimiter2Sldr)
	EVT_TEXT(ID_SGSizeLimiter2Text, ComponentDlg::OnSGSizeLimiter2Text)
	//translate
	EVT_COMMAND_SCROLL(ID_SGTranslateSldr, ComponentDlg::OnSGTranslateSldr)
	EVT_TEXT(ID_SGTranslateText, ComponentDlg::OnSGTranslateText)
	EVT_COMMAND_SCROLL(ID_SGTranslate2Sldr, ComponentDlg::OnSGTranslate2Sldr)
	EVT_TEXT(ID_SGTranslate2Text, ComponentDlg::OnSGTranslate2Text)
	//scalar falloff
	EVT_COMMAND_SCROLL(ID_SGScalarFalloffSldr, ComponentDlg::OnSGScalarFalloffSldr)
	EVT_TEXT(ID_SGScalarFalloffText, ComponentDlg::OnSGScalarFalloffText)
	EVT_COMMAND_SCROLL(ID_SGScalarFalloff2Sldr, ComponentDlg::OnSGScalarFalloff2Sldr)
	EVT_TEXT(ID_SGScalarFalloff2Text, ComponentDlg::OnSGScalarFalloff2Text)
	//grad falloff
	EVT_COMMAND_SCROLL(ID_SGGradFalloffSldr, ComponentDlg::OnSGGradFalloffSldr)
	EVT_TEXT(ID_SGGradFalloffText, ComponentDlg::OnSGGradFalloffText)
	EVT_COMMAND_SCROLL(ID_SGGradFalloff2Sldr, ComponentDlg::OnSGGradFalloff2Sldr)
	EVT_TEXT(ID_SGGradFalloff2Text, ComponentDlg::OnSGGradFalloff2Text)
	//variance falloff
	EVT_COMMAND_SCROLL(ID_SGVarFalloffSldr, ComponentDlg::OnSGVarFalloffSldr)
	EVT_TEXT(ID_SGVarFalloffText, ComponentDlg::OnSGVarFalloffText)
	EVT_COMMAND_SCROLL(ID_SGVarFalloff2Sldr, ComponentDlg::OnSGVarFalloff2Sldr)
	EVT_TEXT(ID_SGVarFalloff2Text, ComponentDlg::OnSGVarFalloff2Text)
	//angle falloff
	EVT_COMMAND_SCROLL(ID_SGAngleFalloffSldr, ComponentDlg::OnSGAngleFalloffSldr)
	EVT_TEXT(ID_SGAngleFalloffText, ComponentDlg::OnSGAngleFalloffText)
	EVT_COMMAND_SCROLL(ID_SGAngleFalloff2Sldr, ComponentDlg::OnSGAngleFalloff2Sldr)
	EVT_TEXT(ID_SGAngleFalloff2Text, ComponentDlg::OnSGAngleFalloff2Text)

	//cleanup
	EVT_CHECKBOX(ID_CleanupCheck, ComponentDlg::OnCleanupCheck)
	EVT_COMMAND_SCROLL(ID_CLIterationsSldr, ComponentDlg::OnCLIterationsSldr)
	EVT_TEXT(ID_CLIterationsText, ComponentDlg::OnCLIterationsText)
	EVT_COMMAND_SCROLL(ID_CLSizeLimiterSldr, ComponentDlg::OnCLSizeLimiterSldr)
	EVT_TEXT(ID_CLSizeLimiterText, ComponentDlg::OnCLSizeLimiterText)

	//match slices
	EVT_CHECKBOX(ID_MatchSlicesCheck, ComponentDlg::OnMatchSlicesCheck)
	EVT_CHECKBOX(ID_BidirMatchCheck, ComponentDlg::OnBidirMatchCheck)
	EVT_COMMAND_SCROLL(ID_SizeThreshSldr, ComponentDlg::OnSizeThreshSldr)
	EVT_TEXT(ID_SizeThreshText, ComponentDlg::OnSizeThreshText)
	EVT_COMMAND_SCROLL(ID_SizeRatioSldr, ComponentDlg::OnSizeRatioSldr)
	EVT_TEXT(ID_SizeRatioText, ComponentDlg::OnSizeRatioText)
	EVT_COMMAND_SCROLL(ID_DistThreshSldr, ComponentDlg::OnDistThreshSldr)
	EVT_TEXT(ID_DistThreshText, ComponentDlg::OnDistThreshText)
	EVT_COMMAND_SCROLL(ID_AngleThreshSldr, ComponentDlg::OnAngleThreshSldr)
	EVT_TEXT(ID_AngleThreshText, ComponentDlg::OnAngleThreshText)

	//basic page
	EVT_COMMAND_SCROLL(ID_BasicIterSldr, ComponentDlg::OnBasicIterSldr)
	EVT_TEXT(ID_BasicIterText, ComponentDlg::OnBasicIterText)
	EVT_COMMAND_SCROLL(ID_BasicThreshSldr, ComponentDlg::OnBasicThreshSldr)
	EVT_TEXT(ID_BasicThreshText, ComponentDlg::OnBasicThreshText)
	EVT_CHECKBOX(ID_BasicDiffCheck, ComponentDlg::OnBasicDiffCheck)
	EVT_COMMAND_SCROLL(ID_BasicFalloffSldr, ComponentDlg::OnBasicFalloffSldr)
	EVT_TEXT(ID_BasicFalloffText, ComponentDlg::OnBasicFalloffText)
	EVT_CHECKBOX(ID_BasicSizeCheck, ComponentDlg::OnBasicSizeCheck)
	EVT_COMMAND_SCROLL(ID_BasicSizeSldr, ComponentDlg::OnBasicSizeSldr)
	EVT_TEXT(ID_BasicSizeText, ComponentDlg::OnBasicSizeText)
	//density
	EVT_CHECKBOX(ID_BasicDensityCheck, ComponentDlg::OnBasicDensityCheck)
	EVT_COMMAND_SCROLL(ID_BasicDensitySldr, ComponentDlg::OnBasicDensitySldr)
	EVT_TEXT(ID_BasicDensityText, ComponentDlg::OnBasicDensityText)
	//clean
	EVT_CHECKBOX(ID_BasicCleanCheck, ComponentDlg::OnBasicCleanCheck)
	EVT_COMMAND_SCROLL(ID_BasicCleanIterSldr, ComponentDlg::OnBasicCleanIterSldr)
	EVT_TEXT(ID_BasicCleanIterText, ComponentDlg::OnBasicCleanIterText)
	EVT_COMMAND_SCROLL(ID_BasicCleanLimitSldr, ComponentDlg::OnBasicCleanLimitSldr)
	EVT_TEXT(ID_BasicCleanLimitText, ComponentDlg::OnBasicCleanLimitText)

	//clustering page
	EVT_RADIOBUTTON(ID_ClusterMethodExmaxRd, ComponentDlg::OnClusterMethodExmaxCheck)
	EVT_RADIOBUTTON(ID_ClusterMethodDbscanRd, ComponentDlg::OnClusterMethodDbscanCheck)
	EVT_RADIOBUTTON(ID_ClusterMethodKmeansRd, ComponentDlg::OnClusterMethodKmeansCheck)
	//parameters
	EVT_COMMAND_SCROLL(ID_ClusterClnumSldr, ComponentDlg::OnClusterClnumSldr)
	EVT_TEXT(ID_ClusterClnumText, ComponentDlg::OnClusterClnumText)
	EVT_COMMAND_SCROLL(ID_ClusterMaxIterSldr, ComponentDlg::OnClusterMaxiterSldr)
	EVT_TEXT(ID_ClusterMaxIterText, ComponentDlg::OnClusterMaxiterText)
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
	EVT_CHECKBOX(ID_AnalysisMaxCheck, ComponentDlg::OnAnalysisMaxCheck)
	EVT_SPINCTRL(ID_AnalysisMaxSpin, ComponentDlg::OnAnalysisMaxSpin)
	EVT_BUTTON(ID_CompFullBtn, ComponentDlg::OnCompFull)
	EVT_BUTTON(ID_CompExclusiveBtn, ComponentDlg::OnCompExclusive)
	EVT_BUTTON(ID_CompAppendBtn, ComponentDlg::OnCompAppend)
	EVT_BUTTON(ID_CompAllBtn, ComponentDlg::OnCompAll)
	EVT_BUTTON(ID_CompClearBtn, ComponentDlg::OnCompClear)
	EVT_CHECKBOX(ID_ConsistentCheck, ComponentDlg::OnConsistentCheck)
	EVT_CHECKBOX(ID_ColocalCheck, ComponentDlg::OnColocalCheck)
	//output
	EVT_RADIOBUTTON(ID_OutputMultiRb, ComponentDlg::OnOutputTypeRadio)
	EVT_RADIOBUTTON(ID_OutputRgbRb, ComponentDlg::OnOutputTypeRadio)
	EVT_BUTTON(ID_OutputRandomBtn, ComponentDlg::OnOutputChannels)
	EVT_BUTTON(ID_OutputSizeBtn, ComponentDlg::OnOutputChannels)
	EVT_BUTTON(ID_OutputAnnBtn, ComponentDlg::OnOutputAnn)

	//execute
	EVT_NOTEBOOK_PAGE_CHANGED(ID_Notebook, ComponentDlg::OnNotebook)
	EVT_BUTTON(ID_GenerateBtn, ComponentDlg::OnGenerate)
	EVT_BUTTON(ID_RefineBtn, ComponentDlg::OnRefine)
	EVT_BUTTON(ID_ClusterBtn, ComponentDlg::OnCluster)
	EVT_BUTTON(ID_AnalyzeBtn, ComponentDlg::OnAnalyze)
	EVT_BUTTON(ID_AnalyzeSelBtn, ComponentDlg::OnAnalyzeSel)
END_EVENT_TABLE()

wxWindow* ComponentDlg::Create3DAnalysisPage(wxWindow *parent)
{
	wxPanel *page = new wxPanel(parent);
	wxStaticText *st = 0;
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;
	//validator: floating point 3
	wxFloatingPointValidator<double> vald_fp3(3);

	wxBoxSizer* sizer1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Iterations:",
		wxDefaultPosition, wxSize(100, 23));
	m_basic_iter_sldr = new wxSlider(page, ID_BasicIterSldr, 0, 0, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_basic_iter_text = new wxTextCtrl(page, ID_BasicIterText, "0",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	sizer1->Add(5, 5);
	sizer1->Add(st, 0, wxALIGN_CENTER);
	sizer1->Add(m_basic_iter_sldr, 1, wxEXPAND);
	sizer1->Add(m_basic_iter_text, 0, wxALIGN_CENTER);
	sizer1->Add(5, 5);

	wxBoxSizer* sizer2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Threshold:",
		wxDefaultPosition, wxSize(100, 23));
	m_basic_thresh_sldr = new wxSlider(page, ID_BasicThreshSldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_basic_thresh_text = new wxTextCtrl(page, ID_BasicThreshText, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	sizer2->Add(5, 5);
	sizer2->Add(st, 0, wxALIGN_CENTER);
	sizer2->Add(m_basic_thresh_sldr, 1, wxEXPAND);
	sizer2->Add(m_basic_thresh_text, 0, wxALIGN_CENTER);
	sizer2->Add(5, 5);

	wxBoxSizer* sizer3 = new wxBoxSizer(wxHORIZONTAL);
	m_basic_diff_check = new wxCheckBox(page, ID_BasicDiffCheck, "Enable Diffusion",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	sizer3->Add(5, 5);
	sizer3->Add(m_basic_diff_check, 0, wxALIGN_CENTER);

	wxBoxSizer* sizer4 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Falloff:",
		wxDefaultPosition, wxSize(100, 23));
	m_basic_falloff_sldr = new wxSlider(page, ID_BasicFalloffSldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_basic_falloff_text = new wxTextCtrl(page, ID_BasicFalloffText, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	sizer4->Add(5, 5);
	sizer4->Add(st, 0, wxALIGN_CENTER);
	sizer4->Add(m_basic_falloff_sldr, 1, wxEXPAND);
	sizer4->Add(m_basic_falloff_text, 0, wxALIGN_CENTER);
	sizer4->Add(5, 5);

	wxBoxSizer* sizer5 = new wxBoxSizer(wxHORIZONTAL);
	m_basic_size_check = new wxCheckBox(page, ID_BasicSizeCheck, "Enable Size Limiter",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	sizer5->Add(5, 5);
	sizer5->Add(m_basic_size_check, 0, wxALIGN_CENTER);

	wxBoxSizer* sizer6 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Size:",
		wxDefaultPosition, wxSize(100, 23));
	m_basic_size_sldr = new wxSlider(page, ID_BasicSizeSldr, 100, 0, 500,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_basic_size_text = new wxTextCtrl(page, ID_BasicSizeText, "100",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	sizer6->Add(5, 5);
	sizer6->Add(st, 0, wxALIGN_CENTER);
	sizer6->Add(m_basic_size_sldr, 1, wxEXPAND);
	sizer6->Add(m_basic_size_text, 0, wxALIGN_CENTER);
	sizer6->Add(5, 5);

	//density
	wxBoxSizer* sizer7 = new wxBoxSizer(wxHORIZONTAL);
	m_basic_density_check = new wxCheckBox(page, ID_BasicDensityCheck, "Enable Density Limiter",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	sizer7->Add(5, 5);
	sizer7->Add(m_basic_density_check, 0, wxALIGN_CENTER);
	//
	wxBoxSizer* sizer8 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Density Threshold:",
		wxDefaultPosition, wxSize(100, 23));
	m_basic_density_sldr = new wxSlider(page, ID_BasicDensitySldr, 500, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_basic_density_text = new wxTextCtrl(page, ID_BasicDensityText, "0.5",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	sizer8->Add(5, 5);
	sizer8->Add(st, 0, wxALIGN_CENTER);
	sizer8->Add(m_basic_density_sldr, 1, wxEXPAND);
	sizer8->Add(m_basic_density_text, 0, wxALIGN_CENTER);
	sizer8->Add(5, 5);

	//clean
	wxBoxSizer* sizer9 = new wxBoxSizer(wxHORIZONTAL);
	m_basic_clean_check = new wxCheckBox(page, ID_BasicCleanCheck, "Clean Up",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	sizer9->Add(5, 5);
	sizer9->Add(m_basic_clean_check, 0, wxALIGN_CENTER);
	//iterations
	wxBoxSizer* sizer10 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Iterations:",
		wxDefaultPosition, wxSize(100, 23));
	m_basic_clean_iter_sldr = new wxSlider(page, ID_BasicCleanIterSldr, 5, 0, 50,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_basic_clean_iter_text = new wxTextCtrl(page, ID_BasicCleanIterText, "5",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	sizer10->Add(5, 5);
	sizer10->Add(st, 0, wxALIGN_CENTER);
	sizer10->Add(m_basic_clean_iter_sldr, 1, wxEXPAND);
	sizer10->Add(m_basic_clean_iter_text, 0, wxALIGN_CENTER);
	sizer10->Add(5, 5);
	//iterations
	wxBoxSizer* sizer11 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Size:",
		wxDefaultPosition, wxSize(100, 23));
	m_basic_clean_limit_sldr = new wxSlider(page, ID_BasicCleanLimitSldr, 5, 0, 50,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_basic_clean_limit_text = new wxTextCtrl(page, ID_BasicCleanLimitText, "5",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	sizer11->Add(5, 5);
	sizer11->Add(st, 0, wxALIGN_CENTER);
	sizer11->Add(m_basic_clean_limit_sldr, 1, wxEXPAND);
	sizer11->Add(m_basic_clean_limit_text, 0, wxALIGN_CENTER);
	sizer11->Add(5, 5);

	wxBoxSizer *group1 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Advanced Settings"), wxVERTICAL);
	group1->Add(10, 10);
	group1->Add(sizer3, 0, wxEXPAND);
	group1->Add(10, 10);
	group1->Add(sizer4, 0, wxEXPAND);
	group1->Add(10, 10);
	group1->Add(sizer5, 0, wxEXPAND);
	group1->Add(10, 10);
	group1->Add(sizer6, 0, wxEXPAND);
	group1->Add(10, 10);
	group1->Add(sizer7, 0, wxEXPAND);
	group1->Add(10, 10);
	group1->Add(sizer8, 0, wxEXPAND);
	group1->Add(10, 10);
	group1->Add(sizer9, 0, wxEXPAND);
	group1->Add(10, 10);
	group1->Add(sizer10, 0, wxEXPAND);
	group1->Add(10, 10);
	group1->Add(sizer11, 0, wxEXPAND);
	group1->Add(10, 10);

	wxBoxSizer* sizerv = new wxBoxSizer(wxVERTICAL);
	sizerv->Add(10, 10);
	sizerv->Add(sizer1, 0, wxEXPAND);
	sizerv->Add(10, 10);
	sizerv->Add(sizer2, 0, wxEXPAND);
	sizerv->Add(10, 10);
	sizerv->Add(group1, 0, wxEXPAND);
	page->SetSizer(sizerv);

	return page;
}

wxWindow* ComponentDlg::Create2DAnalysisPage(wxWindow *parent)
{
	m_adv_page = new wxScrolledWindow(parent);
	wxStaticText *st = 0;

	//load/save settings
	wxBoxSizer* sizer1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(m_adv_page, 0, "Setting File:",
		wxDefaultPosition, wxDefaultSize);
	m_load_settings_text = new wxTextCtrl(m_adv_page, ID_LoadSettingsText, "",
		wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_load_settings_btn = new wxButton(m_adv_page, ID_LoadSettingsBtn, "Load",
		wxDefaultPosition, wxSize(65, 23));
	m_save_settings_btn = new wxButton(m_adv_page, ID_SaveSettingsBtn, "Save",
		wxDefaultPosition, wxSize(65, 23));
	m_saveas_settings_btn = new wxButton(m_adv_page, ID_SaveasSettingsBtn, "Save As",
		wxDefaultPosition, wxSize(65, 23));
	sizer1->Add(5, 5);
	sizer1->Add(st, 0, wxALIGN_CENTER);
	sizer1->Add(10, 10);
	sizer1->Add(m_load_settings_text, 1, wxEXPAND);
	sizer1->Add(10, 10);
	sizer1->Add(m_load_settings_btn, 0, wxALIGN_CENTER);
	sizer1->Add(m_save_settings_btn, 0, wxALIGN_CENTER);
	sizer1->Add(m_saveas_settings_btn, 0, wxALIGN_CENTER);

	//initial grow
	wxBoxSizer* sizer2 = new wxBoxSizer(wxVERTICAL);
	m_initial_grow_pane = CreateInitialGrowPane(m_adv_page);
	sizer2->Add(m_initial_grow_pane, 0, wxEXPAND);
	//sized grow
	wxBoxSizer* sizer3 = new wxBoxSizer(wxVERTICAL);
	m_sized_grow_pane = CreateSizedGrowPane(m_adv_page);
	sizer3->Add(m_sized_grow_pane, 0, wxEXPAND);
	//cleanup
	wxBoxSizer* sizer4 = new wxBoxSizer(wxVERTICAL);
	m_cleanup_pane = CreateCleanupPane(m_adv_page);
	sizer4->Add(m_cleanup_pane, 0, wxEXPAND);
	//match slices
	wxBoxSizer* sizer5 = new wxBoxSizer(wxVERTICAL);
	m_match_slices_pane = CreateMatchSlicesPane(m_adv_page);
	sizer5->Add(m_match_slices_pane, 0, wxEXPAND);

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

	m_adv_page->SetScrollRate(10, 10);
	m_adv_page->SetSizer(sizerv);

	return m_adv_page;
}

wxWindow* ComponentDlg::CreateClusteringPage(wxWindow *parent)
{
	wxPanel *page = new wxPanel(parent);
	wxStaticText *st = 0;
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;
	//validator: floating point 3
	wxFloatingPointValidator<double> vald_fp1(1);

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
	sizer11->Add(m_cluster_method_exmax_rd, 0, wxALIGN_CENTER);
	sizer11->Add(10, 10);
	sizer11->Add(m_cluster_method_dbscan_rd, 0, wxALIGN_CENTER);
	sizer11->Add(10, 10);
	sizer11->Add(m_cluster_method_kmeans_rd, 0, wxALIGN_CENTER);
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
	//size
	wxBoxSizer *sizer23 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Min. Size:",
		wxDefaultPosition, wxSize(100, 20));
	m_cluster_size_sldr = new wxSlider(page, ID_ClusterSizeSldr, 60, 1, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_cluster_size_text = new wxTextCtrl(page, ID_ClusterSizeText, "60",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	sizer23->Add(5, 5);
	sizer23->Add(st, 0, wxALIGN_CENTER);
	sizer23->Add(m_cluster_size_sldr, 1, wxEXPAND);
	sizer23->Add(m_cluster_size_text, 0, wxALIGN_CENTER);
	sizer23->Add(5, 5);
	//eps
	wxBoxSizer *sizer24 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Neighborhood:",
		wxDefaultPosition, wxSize(100, 20));
	m_cluster_eps_sldr = new wxSlider(page, ID_ClusterEpsSldr, 25, 5, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_cluster_eps_text = new wxTextCtrl(page, ID_ClusterEpsText, "2.5",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp1);
	sizer24->Add(5, 5);
	sizer24->Add(st, 0, wxALIGN_CENTER);
	sizer24->Add(m_cluster_eps_sldr, 1, wxEXPAND);
	sizer24->Add(m_cluster_eps_text, 0, wxALIGN_CENTER);
	sizer24->Add(5, 5);
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

	return page;
}

wxWindow* ComponentDlg::CreateAnalysisPage(wxWindow *parent)
{
	wxPanel *page = new wxPanel(parent);
	wxStaticText *st = 0;

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
	m_analysis_min_check = new wxCheckBox(page, ID_AnalysisMinCheck, "Min Size:",
		wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_analysis_min_spin = new wxSpinCtrl(page, ID_AnalysisMinSpin, "0",
		wxDefaultPosition, wxSize(80, 23), wxSP_ARROW_KEYS, 0,
		std::numeric_limits<int>::max(), 0);
	m_analysis_max_check = new wxCheckBox(page, ID_AnalysisMaxCheck, "Max Size:",
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
		wxDefaultPosition, wxSize(65, 23));
	m_comp_all_btn = new wxButton(page, ID_CompAllBtn, "All",
		wxDefaultPosition, wxSize(65, 23));
	m_comp_full_btn = new wxButton(page, ID_CompFullBtn, "FullCompt",
		wxDefaultPosition, wxSize(80, 23));
	m_comp_exclusive_btn = new wxButton(page, ID_CompExclusiveBtn, "Replace",
		wxDefaultPosition, wxSize(65, 23));
	m_comp_clear_btn = new wxButton(page, ID_CompClearBtn, "Clear",
		wxDefaultPosition, wxSize(65, 23));
	sizer12->AddStretchSpacer();
	sizer12->Add(m_comp_append_btn, 0, wxALIGN_CENTER);
	sizer12->Add(m_comp_all_btn, 0, wxALIGN_CENTER);
	sizer12->Add(m_comp_full_btn, 0, wxALIGN_CENTER);
	sizer12->Add(m_comp_exclusive_btn, 0, wxALIGN_CENTER);
	sizer12->Add(m_comp_clear_btn, 0, wxALIGN_CENTER);
	sizer12->AddStretchSpacer();
	//
	sizer1->Add(10, 10);
	sizer1->Add(sizer11, 0, wxEXPAND);
	sizer1->Add(10, 10);
	sizer1->Add(sizer12, 0, wxEXPAND);
	sizer1->Add(10, 10);

	//colocalization
	wxBoxSizer *sizer2 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Options"),
		wxVERTICAL);
	wxBoxSizer *sizer21 = new wxBoxSizer(wxHORIZONTAL);
	m_consistent_check = new wxCheckBox(page, ID_ConsistentCheck, "Make color consistent for multiple bricks",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	sizer21->Add(5, 5);
	sizer21->Add(m_consistent_check, 0, wxALIGN_CENTER);
	wxBoxSizer *sizer22 = new wxBoxSizer(wxHORIZONTAL);
	m_colocal_check = new wxCheckBox(page, ID_ColocalCheck, "Compute colocalization with other channels",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	sizer22->Add(5, 5);
	sizer22->Add(m_colocal_check, 0, wxALIGN_CENTER);
	//
	sizer2->Add(10, 10);
	sizer2->Add(sizer21, 0, wxEXPAND);
	sizer2->Add(10, 10);
	sizer2->Add(sizer22, 0, wxEXPAND);
	sizer2->Add(10, 10);

	//output
	wxBoxSizer *sizer3 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Output as New Channels"),
		wxVERTICAL);
	//radios
	wxBoxSizer *sizer31 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Channel Type:",
		wxDefaultPosition, wxSize(100, 20));
	m_output_multi_rb = new wxRadioButton(page, ID_OutputMultiRb, "Each Comp.",
		wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	m_output_rgb_rb = new wxRadioButton(page, ID_OutputRgbRb, "R+G+B",
		wxDefaultPosition, wxDefaultSize);
	sizer31->Add(5, 5);
	sizer31->Add(st, 0, wxALIGN_CENTER);
	sizer31->Add(m_output_multi_rb, 0, wxALIGN_CENTER);
	sizer31->Add(5, 5);
	sizer31->Add(m_output_rgb_rb, 0, wxALIGN_CENTER);
	sizer31->Add(5, 5);
	//buttons
	wxBoxSizer *sizer32 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Output:",
		wxDefaultPosition, wxSize(100, 20));
	m_output_random_btn = new wxButton(page, ID_OutputRandomBtn, "Random Colors",
		wxDefaultPosition, wxSize(100, 23));
	m_output_size_btn = new wxButton(page, ID_OutputSizeBtn, "Size-based",
		wxDefaultPosition, wxSize(100, 23));
	m_output_ann_btn = new wxButton(page, ID_OutputAnnBtn, "Annotations",
		wxDefaultPosition, wxSize(100, 23));
	sizer32->Add(5, 5);
	sizer32->Add(st, 0, wxALIGN_CENTER);
	sizer32->Add(m_output_random_btn, 0, wxALIGN_CENTER);
	sizer32->Add(m_output_size_btn, 0, wxALIGN_CENTER);
	sizer32->Add(m_output_ann_btn, 0, wxALIGN_CENTER);
	sizer32->Add(5, 5);
	//
	sizer3->Add(10, 10);
	sizer3->Add(sizer31, 0, wxEXPAND);
	sizer3->Add(10, 10);
	sizer3->Add(sizer32, 0, wxEXPAND);
	sizer3->Add(10, 10);

	//note
	wxBoxSizer *sizer4 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "N.B."),
		wxVERTICAL);
	st = new wxStaticText(page, 0,
		"Enable 4D script in the settings to show component colors.");
	sizer4->Add(10, 10);
	sizer4->Add(st, 0);
	sizer4->Add(10, 10);

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

	page->SetSizer(sizerv);

	m_analysis_min_check->SetValue(false);
	m_analysis_min_spin->Disable();
	m_analysis_max_check->SetValue(false);
	m_analysis_max_spin->Disable();

	return page;
}

wxCollapsiblePane* ComponentDlg::CreateInitialGrowPane(wxWindow *parent)
{
	wxCollapsiblePane* collpane = new wxCollapsiblePane(parent,
		ID_InitialGrowPane, "Initial Grow",
		wxDefaultPosition, wxDefaultSize, wxCP_NO_TLW_RESIZE);
	collpane->SetBackgroundColour(m_notebook->GetThemeBackgroundColour());
	wxWindow *pane = collpane->GetPane();
	wxStaticText* st;
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;
	//validator: floating point 3
	wxFloatingPointValidator<double> vald_fp3(3);

	//enable
	wxBoxSizer *sizer1 = new wxBoxSizer(wxHORIZONTAL);
	m_initial_grow_check = new wxCheckBox(pane, ID_InitialGrowCheck, "Enable",
		wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_ig_param_transition_check = new wxCheckBox(pane, ID_IGParamTransitionCheck, "Parameter Transition",
		wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	sizer1->Add(5, 5);
	sizer1->Add(m_initial_grow_check, 0, wxALIGN_CENTER);
	sizer1->Add(5, 5);
	sizer1->Add(m_ig_param_transition_check, 0, wxALIGN_CENTER);
	//iterations
	wxBoxSizer *sizer2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(pane, 0, "Iterations:",
		wxDefaultPosition, wxSize(100, 23));
	m_ig_iterations_sldr = new wxSlider(pane, ID_IGIterationsSldr, 0, 0, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_ig_iterations_text = new wxTextCtrl(pane, ID_IGIterationsText, "0",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	sizer2->Add(5, 5);
	sizer2->Add(st, 0, wxALIGN_CENTER);
	sizer2->Add(m_ig_iterations_sldr, 1, wxEXPAND);
	sizer2->Add(m_ig_iterations_text, 0, wxALIGN_CENTER);
	//translate
	wxBoxSizer *sizer3 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(pane, 0, "Translation:",
		wxDefaultPosition, wxSize(100, 23));
	m_ig_translate_sldr = new wxSlider(pane, ID_IGTranslateSldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_ig_translate_text = new wxTextCtrl(pane, ID_IGTranslateText, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	m_ig_translate_st = new wxStaticText(pane, 0, L" \u27a0 ");
	m_ig_translate2_sldr = new wxSlider(pane, ID_IGTranslate2Sldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_ig_translate2_text = new wxTextCtrl(pane, ID_IGTranslate2Text, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	sizer3->Add(5, 5);
	sizer3->Add(st, 0, wxALIGN_CENTER);
	sizer3->Add(m_ig_translate_sldr, 1, wxEXPAND);
	sizer3->Add(m_ig_translate_text, 0, wxALIGN_CENTER);
	sizer3->Add(m_ig_translate_st, 0, wxALIGN_CENTER);
	sizer3->Add(m_ig_translate2_sldr, 1, wxEXPAND);
	sizer3->Add(m_ig_translate2_text, 0, wxALIGN_CENTER);
	//scalar falloff
	wxBoxSizer *sizer4 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(pane, 0, "Scalar Falloff:",
		wxDefaultPosition, wxSize(100, 23));
	m_ig_scalar_falloff_sldr = new wxSlider(pane, ID_IGScalarFalloffSldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_ig_scalar_falloff_text = new wxTextCtrl(pane, ID_IGScalarFalloffText, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	m_ig_scalar_falloff_st = new wxStaticText(pane, 0, L" \u27a0 ");
	m_ig_scalar_falloff2_sldr = new wxSlider(pane, ID_IGScalarFalloff2Sldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_ig_scalar_falloff2_text = new wxTextCtrl(pane, ID_IGScalarFalloff2Text, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	sizer4->Add(5, 5);
	sizer4->Add(st, 0, wxALIGN_CENTER);
	sizer4->Add(m_ig_scalar_falloff_sldr, 1, wxEXPAND);
	sizer4->Add(m_ig_scalar_falloff_text, 0, wxALIGN_CENTER);
	sizer4->Add(m_ig_scalar_falloff_st, 0, wxALIGN_CENTER);
	sizer4->Add(m_ig_scalar_falloff2_sldr, 1, wxEXPAND);
	sizer4->Add(m_ig_scalar_falloff2_text, 0, wxALIGN_CENTER);
	//grad falloff
	wxBoxSizer *sizer5 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(pane, 0, "Grad Falloff:",
		wxDefaultPosition, wxSize(100, 23));
	m_ig_grad_falloff_sldr = new wxSlider(pane, ID_IGGradFalloffSldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_ig_grad_falloff_text = new wxTextCtrl(pane, ID_IGGradFalloffText, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	m_ig_grad_falloff_st = new wxStaticText(pane, 0, L" \u27a0 ");
	m_ig_grad_falloff2_sldr = new wxSlider(pane, ID_IGGradFalloff2Sldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_ig_grad_falloff2_text = new wxTextCtrl(pane, ID_IGGradFalloff2Text, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	sizer5->Add(5, 5);
	sizer5->Add(st, 0, wxALIGN_CENTER);
	sizer5->Add(m_ig_grad_falloff_sldr, 1, wxEXPAND);
	sizer5->Add(m_ig_grad_falloff_text, 0, wxALIGN_CENTER);
	sizer5->Add(m_ig_grad_falloff_st, 0, wxALIGN_CENTER);
	sizer5->Add(m_ig_grad_falloff2_sldr, 1, wxEXPAND);
	sizer5->Add(m_ig_grad_falloff2_text, 0, wxALIGN_CENTER);
	//variance falloff
	wxBoxSizer *sizer6 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(pane, 0, "Var Falloff:",
		wxDefaultPosition, wxSize(100, 23));
	m_ig_var_falloff_sldr = new wxSlider(pane, ID_IGVarFalloffSldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_ig_var_falloff_text = new wxTextCtrl(pane, ID_IGVarFalloffText, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	m_ig_var_falloff_st = new wxStaticText(pane, 0, L" \u27a0 ");
	m_ig_var_falloff2_sldr = new wxSlider(pane, ID_IGVarFalloff2Sldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_ig_var_falloff2_text = new wxTextCtrl(pane, ID_IGVarFalloff2Text, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	sizer6->Add(5, 5);
	sizer6->Add(st, 0, wxALIGN_CENTER);
	sizer6->Add(m_ig_var_falloff_sldr, 1, wxEXPAND);
	sizer6->Add(m_ig_var_falloff_text, 0, wxALIGN_CENTER);
	sizer6->Add(m_ig_var_falloff_st, 0, wxALIGN_CENTER);
	sizer6->Add(m_ig_var_falloff2_sldr, 1, wxEXPAND);
	sizer6->Add(m_ig_var_falloff2_text, 0, wxALIGN_CENTER);
	//angle falloff
	wxBoxSizer *sizer7 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(pane, 0, "Angle Falloff:",
		wxDefaultPosition, wxSize(100, 23));
	m_ig_angle_falloff_sldr = new wxSlider(pane, ID_IGAngleFalloffSldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_ig_angle_falloff_text = new wxTextCtrl(pane, ID_IGAngleFalloffText, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	m_ig_angle_falloff_st = new wxStaticText(pane, 0, L" \u27a0 ");
	m_ig_angle_falloff2_sldr = new wxSlider(pane, ID_IGAngleFalloff2Sldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_ig_angle_falloff2_text = new wxTextCtrl(pane, ID_IGAngleFalloff2Text, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	sizer7->Add(5, 5);
	sizer7->Add(st, 0, wxALIGN_CENTER);
	sizer7->Add(m_ig_angle_falloff_sldr, 1, wxEXPAND);
	sizer7->Add(m_ig_angle_falloff_text, 0, wxALIGN_CENTER);
	sizer7->Add(m_ig_angle_falloff_st, 0, wxALIGN_CENTER);
	sizer7->Add(m_ig_angle_falloff2_sldr, 1, wxEXPAND);
	sizer7->Add(m_ig_angle_falloff2_text, 0, wxALIGN_CENTER);

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
	sizerv->Add(sizer7, 0, wxEXPAND);
	sizerv->Add(10, 10);

	pane->SetSizer(sizerv);
	sizerv->SetSizeHints(pane);

	return collpane;
}

wxCollapsiblePane* ComponentDlg::CreateSizedGrowPane(wxWindow *parent)
{
	wxCollapsiblePane* collpane = new wxCollapsiblePane(parent,
		ID_SizedGrowPane, "Sized Grow",
		wxDefaultPosition, wxDefaultSize, wxCP_NO_TLW_RESIZE);
	collpane->SetBackgroundColour(m_notebook->GetThemeBackgroundColour());
	wxWindow *pane = collpane->GetPane();
	wxStaticText* st;
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;
	//validator: floating point 3
	wxFloatingPointValidator<double> vald_fp3(3);

	//enable
	wxBoxSizer *sizer1 = new wxBoxSizer(wxHORIZONTAL);
	m_sized_grow_check = new wxCheckBox(pane, ID_SizedGrowCheck, "Enable",
		wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_sg_param_transition_check = new wxCheckBox(pane, ID_SGParamTransitionCheck, "Parameter Transition",
		wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	sizer1->Add(5, 5);
	sizer1->Add(m_sized_grow_check, 0, wxALIGN_CENTER);
	sizer1->Add(5, 5);
	sizer1->Add(m_sg_param_transition_check, 0, wxALIGN_CENTER);
	//iterations
	wxBoxSizer *sizer2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(pane, 0, "Iterations:",
		wxDefaultPosition, wxSize(100, 23));
	m_sg_iterations_sldr = new wxSlider(pane, ID_SGIterationsSldr, 0, 0, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_sg_iterations_text = new wxTextCtrl(pane, ID_SGIterationsText, "0",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	sizer2->Add(5, 5);
	sizer2->Add(st, 0, wxALIGN_CENTER);
	sizer2->Add(m_sg_iterations_sldr, 1, wxEXPAND);
	sizer2->Add(m_sg_iterations_text, 0, wxALIGN_CENTER);
	//size limiter
	wxBoxSizer *sizer3 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(pane, 0, "Size Limit:",
		wxDefaultPosition, wxSize(100, 23));
	m_sg_size_limiter_sldr = new wxSlider(pane, ID_SGSizeLimiterSldr, 0, 0, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_sg_size_limiter_text = new wxTextCtrl(pane, ID_SGSizeLimiterText, "0",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	m_sg_size_limiter_st = new wxStaticText(pane, 0, L" \u27a0 ");
	m_sg_size_limiter2_sldr = new wxSlider(pane, ID_SGSizeLimiter2Sldr, 0, 0, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_sg_size_limiter2_text = new wxTextCtrl(pane, ID_SGSizeLimiter2Text, "0",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	sizer3->Add(5, 5);
	sizer3->Add(st, 0, wxALIGN_CENTER);
	sizer3->Add(m_sg_size_limiter_sldr, 1, wxEXPAND);
	sizer3->Add(m_sg_size_limiter_text, 0, wxALIGN_CENTER);
	sizer3->Add(m_sg_size_limiter_st, 0, wxALIGN_CENTER);
	sizer3->Add(m_sg_size_limiter2_sldr, 1, wxEXPAND);
	sizer3->Add(m_sg_size_limiter2_text, 0, wxALIGN_CENTER);
	//translate
	wxBoxSizer *sizer4 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(pane, 0, "Translation:",
		wxDefaultPosition, wxSize(100, 23));
	m_sg_translate_sldr = new wxSlider(pane, ID_SGTranslateSldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_sg_translate_text = new wxTextCtrl(pane, ID_SGTranslateText, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	m_sg_translate_st = new wxStaticText(pane, 0, L" \u27a0 ");
	m_sg_translate2_sldr = new wxSlider(pane, ID_SGTranslate2Sldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_sg_translate2_text = new wxTextCtrl(pane, ID_SGTranslate2Text, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	sizer4->Add(5, 5);
	sizer4->Add(st, 0, wxALIGN_CENTER);
	sizer4->Add(m_sg_translate_sldr, 1, wxEXPAND);
	sizer4->Add(m_sg_translate_text, 0, wxALIGN_CENTER);
	sizer4->Add(m_sg_translate_st, 0, wxALIGN_CENTER);
	sizer4->Add(m_sg_translate2_sldr, 1, wxEXPAND);
	sizer4->Add(m_sg_translate2_text, 0, wxALIGN_CENTER);
	//scalar falloff
	wxBoxSizer *sizer5 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(pane, 0, "Scalar Falloff:",
		wxDefaultPosition, wxSize(100, 23));
	m_sg_scalar_falloff_sldr = new wxSlider(pane, ID_SGScalarFalloffSldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_sg_scalar_falloff_text = new wxTextCtrl(pane, ID_SGScalarFalloffText, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	m_sg_scalar_falloff_st = new wxStaticText(pane, 0, L" \u27a0 ");
	m_sg_scalar_falloff2_sldr = new wxSlider(pane, ID_SGScalarFalloff2Sldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_sg_scalar_falloff2_text = new wxTextCtrl(pane, ID_SGScalarFalloff2Text, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	sizer5->Add(5, 5);
	sizer5->Add(st, 0, wxALIGN_CENTER);
	sizer5->Add(m_sg_scalar_falloff_sldr, 1, wxEXPAND);
	sizer5->Add(m_sg_scalar_falloff_text, 0, wxALIGN_CENTER);
	sizer5->Add(m_sg_scalar_falloff_st, 0, wxALIGN_CENTER);
	sizer5->Add(m_sg_scalar_falloff2_sldr, 1, wxEXPAND);
	sizer5->Add(m_sg_scalar_falloff2_text, 0, wxALIGN_CENTER);
	//grad falloff
	wxBoxSizer *sizer6 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(pane, 0, "Grad Falloff:",
		wxDefaultPosition, wxSize(100, 23));
	m_sg_grad_falloff_sldr = new wxSlider(pane, ID_SGGradFalloffSldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_sg_grad_falloff_text = new wxTextCtrl(pane, ID_SGGradFalloffText, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	m_sg_grad_falloff_st = new wxStaticText(pane, 0, L" \u27a0 ");
	m_sg_grad_falloff2_sldr = new wxSlider(pane, ID_SGGradFalloff2Sldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_sg_grad_falloff2_text = new wxTextCtrl(pane, ID_SGGradFalloff2Text, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	sizer6->Add(5, 5);
	sizer6->Add(st, 0, wxALIGN_CENTER);
	sizer6->Add(m_sg_grad_falloff_sldr, 1, wxEXPAND);
	sizer6->Add(m_sg_grad_falloff_text, 0, wxALIGN_CENTER);
	sizer6->Add(m_sg_grad_falloff_st, 0, wxALIGN_CENTER);
	sizer6->Add(m_sg_grad_falloff2_sldr, 1, wxEXPAND);
	sizer6->Add(m_sg_grad_falloff2_text, 0, wxALIGN_CENTER);
	//variance falloff
	wxBoxSizer *sizer7 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(pane, 0, "Var Falloff:",
		wxDefaultPosition, wxSize(100, 23));
	m_sg_var_falloff_sldr = new wxSlider(pane, ID_SGVarFalloffSldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_sg_var_falloff_text = new wxTextCtrl(pane, ID_SGVarFalloffText, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	m_sg_var_falloff_st = new wxStaticText(pane, 0, L" \u27a0 ");
	m_sg_var_falloff2_sldr = new wxSlider(pane, ID_SGVarFalloff2Sldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_sg_var_falloff2_text = new wxTextCtrl(pane, ID_SGVarFalloff2Text, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	sizer7->Add(5, 5);
	sizer7->Add(st, 0, wxALIGN_CENTER);
	sizer7->Add(m_sg_var_falloff_sldr, 1, wxEXPAND);
	sizer7->Add(m_sg_var_falloff_text, 0, wxALIGN_CENTER);
	sizer7->Add(m_sg_var_falloff_st, 0, wxALIGN_CENTER);
	sizer7->Add(m_sg_var_falloff2_sldr, 1, wxEXPAND);
	sizer7->Add(m_sg_var_falloff2_text, 0, wxALIGN_CENTER);
	//angle falloff
	wxBoxSizer *sizer8 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(pane, 0, "Angle Falloff:",
		wxDefaultPosition, wxSize(100, 23));
	m_sg_angle_falloff_sldr = new wxSlider(pane, ID_SGAngleFalloffSldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_sg_angle_falloff_text = new wxTextCtrl(pane, ID_SGAngleFalloffText, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	m_sg_angle_falloff_st = new wxStaticText(pane, 0, L" \u27a0 ");
	m_sg_angle_falloff2_sldr = new wxSlider(pane, ID_SGAngleFalloff2Sldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_sg_angle_falloff2_text = new wxTextCtrl(pane, ID_SGAngleFalloff2Text, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	sizer8->Add(5, 5);
	sizer8->Add(st, 0, wxALIGN_CENTER);
	sizer8->Add(m_sg_angle_falloff_sldr, 1, wxEXPAND);
	sizer8->Add(m_sg_angle_falloff_text, 0, wxALIGN_CENTER);
	sizer8->Add(m_sg_angle_falloff_st, 0, wxALIGN_CENTER);
	sizer8->Add(m_sg_angle_falloff2_sldr, 1, wxEXPAND);
	sizer8->Add(m_sg_angle_falloff2_text, 0, wxALIGN_CENTER);

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
	sizerv->Add(sizer7, 0, wxEXPAND);
	sizerv->Add(10, 10);
	sizerv->Add(sizer8, 0, wxEXPAND);
	sizerv->Add(10, 10);

	pane->SetSizer(sizerv);
	sizerv->SetSizeHints(pane);

	return collpane;
}

wxCollapsiblePane* ComponentDlg::CreateCleanupPane(wxWindow *parent)
{
	wxCollapsiblePane* collpane = new wxCollapsiblePane(parent,
		ID_CleanupPane, "Cleanup",
		wxDefaultPosition, wxDefaultSize, wxCP_NO_TLW_RESIZE);
	collpane->SetBackgroundColour(m_notebook->GetThemeBackgroundColour());
	wxWindow *pane = collpane->GetPane();
	wxStaticText* st;
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;

	//enable
	wxBoxSizer *sizer1 = new wxBoxSizer(wxHORIZONTAL);
	m_cleanup_check = new wxCheckBox(pane, ID_CleanupCheck, "Enable",
		wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	sizer1->Add(5, 5);
	sizer1->Add(m_cleanup_check, 0, wxALIGN_CENTER);
	//iterations
	wxBoxSizer *sizer2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(pane, 0, "Iterations:",
		wxDefaultPosition, wxSize(100, 23));
	m_cl_iterations_sldr = new wxSlider(pane, ID_CLIterationsSldr, 0, 0, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_cl_iterations_text = new wxTextCtrl(pane, ID_CLIterationsText, "0",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	sizer2->Add(5, 5);
	sizer2->Add(st, 0, wxALIGN_CENTER);
	sizer2->Add(m_cl_iterations_sldr, 1, wxEXPAND);
	sizer2->Add(m_cl_iterations_text, 0, wxALIGN_CENTER);
	//size limiter
	wxBoxSizer *sizer3 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(pane, 0, "Size Limit:",
		wxDefaultPosition, wxSize(100, 23));
	m_cl_size_limiter_sldr = new wxSlider(pane, ID_CLSizeLimiterSldr, 0, 0, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_cl_size_limiter_text = new wxTextCtrl(pane, ID_CLSizeLimiterText, "0",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	sizer3->Add(5, 5);
	sizer3->Add(st, 0, wxALIGN_CENTER);
	sizer3->Add(m_cl_size_limiter_sldr, 1, wxEXPAND);
	sizer3->Add(m_cl_size_limiter_text, 0, wxALIGN_CENTER);

	wxBoxSizer* sizerv = new wxBoxSizer(wxVERTICAL);
	sizerv->Add(10, 10);
	sizerv->Add(sizer1, 0, wxEXPAND);
	sizerv->Add(10, 10);
	sizerv->Add(sizer2, 0, wxEXPAND);
	sizerv->Add(10, 10);
	sizerv->Add(sizer3, 0, wxEXPAND);
	sizerv->Add(10, 10);

	pane->SetSizer(sizerv);
	sizerv->SetSizeHints(pane);

	return collpane;
}

wxCollapsiblePane* ComponentDlg::CreateMatchSlicesPane(wxWindow *parent)
{
	wxCollapsiblePane* collpane = new wxCollapsiblePane(parent,
		ID_MatchSlicesPane, "Match Slices",
		wxDefaultPosition, wxDefaultSize, wxCP_NO_TLW_RESIZE);
	collpane->SetBackgroundColour(m_notebook->GetThemeBackgroundColour());
	wxWindow *pane = collpane->GetPane();
	wxStaticText* st;
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;
	//validator: floating point 3
	wxFloatingPointValidator<double> vald_fp3(3);

	//enable
	wxBoxSizer *sizer1 = new wxBoxSizer(wxHORIZONTAL);
	m_match_slices_check = new wxCheckBox(pane, ID_MatchSlicesCheck, "Enable",
		wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_bidir_match_check = new wxCheckBox(pane, ID_BidirMatchCheck, "Bidirectional",
		wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	sizer1->Add(5, 5);
	sizer1->Add(m_match_slices_check, 0, wxALIGN_CENTER);
	sizer1->Add(5, 5);
	sizer1->Add(m_bidir_match_check, 0, wxALIGN_CENTER);
	//size thresh
	wxBoxSizer *sizer2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(pane, 0, "Size Threshold:",
		wxDefaultPosition, wxSize(100, 23));
	m_size_thresh_sldr = new wxSlider(pane, ID_SizeThreshSldr, 0, 0, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_size_thresh_text = new wxTextCtrl(pane, ID_SizeThreshText, "0",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	sizer2->Add(5, 5);
	sizer2->Add(st, 0, wxALIGN_CENTER);
	sizer2->Add(m_size_thresh_sldr, 1, wxEXPAND);
	sizer2->Add(m_size_thresh_text, 0, wxALIGN_CENTER);
	//size ratio
	wxBoxSizer *sizer3 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(pane, 0, "Size Ratio:",
		wxDefaultPosition, wxSize(100, 23));
	m_size_ratio_sldr = new wxSlider(pane, ID_SizeRatioSldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_size_ratio_text = new wxTextCtrl(pane, ID_SizeRatioText, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	sizer3->Add(5, 5);
	sizer3->Add(st, 0, wxALIGN_CENTER);
	sizer3->Add(m_size_ratio_sldr, 1, wxEXPAND);
	sizer3->Add(m_size_ratio_text, 0, wxALIGN_CENTER);
	//dist thresh
	wxBoxSizer *sizer4 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(pane, 0, "Dist Threshold:",
		wxDefaultPosition, wxSize(100, 23));
	m_dist_thresh_sldr = new wxSlider(pane, ID_DistThreshSldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_dist_thresh_text = new wxTextCtrl(pane, ID_DistThreshText, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	sizer4->Add(5, 5);
	sizer4->Add(st, 0, wxALIGN_CENTER);
	sizer4->Add(m_dist_thresh_sldr, 1, wxEXPAND);
	sizer4->Add(m_dist_thresh_text, 0, wxALIGN_CENTER);
	//angle thresh
	wxBoxSizer *sizer5 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(pane, 0, "Ang Threshold:",
		wxDefaultPosition, wxSize(100, 23));
	m_angle_thresh_sldr = new wxSlider(pane, ID_AngleThreshSldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_angle_thresh_text = new wxTextCtrl(pane, ID_AngleThreshText, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	sizer5->Add(5, 5);
	sizer5->Add(st, 0, wxALIGN_CENTER);
	sizer5->Add(m_angle_thresh_sldr, 1, wxEXPAND);
	sizer5->Add(m_angle_thresh_text, 0, wxALIGN_CENTER);

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

	pane->SetSizer(sizerv);
	sizerv->SetSizeHints(pane);

	return collpane;
}

ComponentDlg::ComponentDlg(wxWindow *frame, wxWindow *parent)
	: wxPanel(parent, wxID_ANY,
		wxDefaultPosition,
		wxSize(500, 650),
		0, "ComponentDlg"),
	m_frame(parent),
	m_view(0)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);

	//notebook
	m_notebook = new wxNotebook(this, ID_Notebook);
	m_notebook->AddPage(Create3DAnalysisPage(m_notebook), "Basic");
	m_notebook->AddPage(Create2DAnalysisPage(m_notebook), "2D Slices");
	m_notebook->AddPage(CreateClusteringPage(m_notebook), "Clustering");
	m_notebook->AddPage(CreateAnalysisPage(m_notebook), "Analysis");

	wxBoxSizer* sizer1 = new wxBoxSizer(wxHORIZONTAL);
	m_generate_prg = new wxGauge(this, ID_GeneratePrg, 100,
		wxDefaultPosition, wxSize(-1, 18));
	m_use_sel_chk = new wxCheckBox(this, ID_UseSelChk, "Use Sel.",
		wxDefaultPosition, wxDefaultSize);
	m_generate_btn = new wxButton(this, ID_GenerateBtn, "Generate",
		wxDefaultPosition, wxSize(75, -1));
	m_refine_btn = new wxButton(this, ID_RefineBtn, "Refine",
		wxDefaultPosition, wxSize(75, -1));
	m_cluster_btn = new wxButton(this, ID_ClusterBtn, "Cluster",
		wxDefaultPosition, wxSize(75, -1));
	m_analyze_btn = new wxButton(this, ID_AnalyzeBtn, "Analyze",
		wxDefaultPosition, wxSize(75, -1));
	m_analyze_sel_btn = new wxButton(this, ID_AnalyzeSelBtn, "Anlyz. Sel.",
		wxDefaultPosition, wxSize(75, -1));
	sizer1->Add(10, 10);
	sizer1->Add(m_generate_prg, 1, wxEXPAND);
	sizer1->Add(10, 10);
	sizer1->Add(m_use_sel_chk, 0, wxALIGN_CENTER);
	sizer1->Add(m_generate_btn, 0, wxALIGN_CENTER);
	sizer1->Add(m_refine_btn, 0, wxALIGN_CENTER);
	sizer1->Add(m_cluster_btn, 0, wxALIGN_CENTER);
	sizer1->Add(m_analyze_btn, 0, wxALIGN_CENTER);
	sizer1->Add(m_analyze_sel_btn, 0, wxALIGN_CENTER);
	sizer1->Add(10, 10);

	//stats text
	wxBoxSizer *sizer2 = new wxStaticBoxSizer(
		new wxStaticBox(this, wxID_ANY, "Output"),
		wxVERTICAL);
	m_stat_text = new wxTextCtrl(this, ID_StatText, "",
		wxDefaultPosition, wxSize(-1, 150), wxTE_MULTILINE);
	m_stat_text->SetEditable(false);
	sizer2->Add(m_stat_text, 1, wxEXPAND);

	//all controls
	wxBoxSizer* sizerv = new wxBoxSizer(wxVERTICAL);
	sizerv->Add(10, 10);
	sizerv->Add(m_notebook, 1, wxEXPAND);
	sizerv->Add(10, 10);
	sizerv->Add(sizer1, 0, wxEXPAND);
	sizerv->Add(10, 10);
	sizerv->Add(sizer2, 0, wxEXPAND);
	sizerv->Add(10, 10);

	SetSizer(sizerv);
	Layout();

	GetSettings();
}

ComponentDlg::~ComponentDlg()
{
	SaveSettings("");
}

void ComponentDlg::Update()
{
	//update ui
	//initial grow
	m_initial_grow_check->SetValue(m_initial_grow);
	EnableInitialGrow(m_initial_grow);
	if (m_initial_grow)
		m_initial_grow_pane->Expand();
	else
		m_initial_grow_pane->Collapse();
	m_ig_param_transition_check->SetValue(m_ig_param_transition);
	EnableIGParamTransition(m_ig_param_transition);
	m_ig_iterations_text->SetValue(wxString::Format("%d", m_ig_iterations));
	m_ig_translate_text->SetValue(wxString::Format("%.3f", m_ig_translate));
	m_ig_scalar_falloff_text->SetValue(wxString::Format("%.3f", m_ig_scalar_falloff));
	m_ig_grad_falloff_text->SetValue(wxString::Format("%.3f", m_ig_grad_falloff));
	m_ig_var_falloff_text->SetValue(wxString::Format("%.3f", m_ig_var_falloff));
	m_ig_angle_falloff_text->SetValue(wxString::Format("%.3f", m_ig_angle_falloff));
	m_ig_translate2_text->SetValue(wxString::Format("%.3f", m_ig_translate2));
	m_ig_scalar_falloff2_text->SetValue(wxString::Format("%.3f", m_ig_scalar_falloff2));
	m_ig_grad_falloff2_text->SetValue(wxString::Format("%.3f", m_ig_grad_falloff2));
	m_ig_var_falloff2_text->SetValue(wxString::Format("%.3f", m_ig_var_falloff2));
	m_ig_angle_falloff2_text->SetValue(wxString::Format("%.3f", m_ig_angle_falloff2));

	//sized grow
	m_sized_grow_check->SetValue(m_sized_grow);
	EnableSizedGrow(m_sized_grow);
	if (m_sized_grow)
		m_sized_grow_pane->Expand();
	else
		m_sized_grow_pane->Collapse();
	m_sg_param_transition_check->SetValue(m_sg_param_transition);
	EnableSGParamTransition(m_sg_param_transition);
	m_sg_iterations_text->SetValue(wxString::Format("%d", m_sg_iterations));
	m_sg_size_limiter_text->SetValue(wxString::Format("%d", m_sg_size_limiter));
	m_sg_translate_text->SetValue(wxString::Format("%.3f", m_sg_translate));
	m_sg_scalar_falloff_text->SetValue(wxString::Format("%.3f", m_sg_scalar_falloff));
	m_sg_grad_falloff_text->SetValue(wxString::Format("%.3f", m_sg_grad_falloff));
	m_sg_var_falloff_text->SetValue(wxString::Format("%.3f", m_sg_var_falloff));
	m_sg_angle_falloff_text->SetValue(wxString::Format("%.3f", m_sg_angle_falloff));
	m_sg_size_limiter2_text->SetValue(wxString::Format("%d", m_sg_size_limiter2));
	m_sg_translate2_text->SetValue(wxString::Format("%.3f", m_sg_translate2));
	m_sg_scalar_falloff2_text->SetValue(wxString::Format("%.3f", m_sg_scalar_falloff2));
	m_sg_grad_falloff2_text->SetValue(wxString::Format("%.3f", m_sg_grad_falloff2));
	m_sg_var_falloff2_text->SetValue(wxString::Format("%.3f", m_sg_var_falloff2));
	m_sg_angle_falloff2_text->SetValue(wxString::Format("%.3f", m_sg_angle_falloff2));

	//cleanup
	m_cleanup_check->SetValue(m_cleanup);
	EnableCleanup(m_cleanup);
	if (m_cleanup)
		m_cleanup_pane->Expand();
	else
		m_cleanup_pane->Collapse();
	m_cl_iterations_text->SetValue(wxString::Format("%d", m_cl_iterations));
	m_cl_size_limiter_text->SetValue(wxString::Format("%d", m_cl_size_limiter));

	//match slices
	m_match_slices_check->SetValue(m_match_slices);
	m_bidir_match_check->SetValue(m_bidir_match);
	EnableMatchSlices(m_match_slices);
	if (m_match_slices)
		m_match_slices_pane->Expand();
	else
		m_match_slices_pane->Collapse();
	m_size_thresh_text->SetValue(wxString::Format("%d", m_size_thresh));
	m_size_ratio_text->SetValue(wxString::Format("%.3f", m_size_ratio));
	m_dist_thresh_text->SetValue(wxString::Format("%.3f", m_dist_thresh));
	m_angle_thresh_text->SetValue(wxString::Format("%.3f", m_angle_thresh));

	//basic page
	m_basic_iter_text->SetValue(wxString::Format("%d", m_basic_iter));
	m_basic_thresh_text->SetValue(wxString::Format("%.3f", m_basic_thresh));
	m_basic_diff_check->SetValue(m_basic_diff);
	EnableBasicDiff(m_basic_diff);
	m_basic_falloff_text->SetValue(wxString::Format("%.3f", m_basic_falloff));
	m_basic_size_check->SetValue(m_basic_size);
	EnableBasicSize(m_basic_size);
	m_basic_size_text->SetValue(wxString::Format("%d", m_basic_size_lm));
	EnableBasicDensity(m_basic_density);
	m_basic_density_check->SetValue(m_basic_density);
	m_basic_density_text->SetValue(wxString::Format("%.3f", m_basic_density_vl));
	EnableBasicClean(m_basic_clean);
	m_basic_clean_check->SetValue(m_basic_clean);
	m_basic_clean_iter_text->SetValue(wxString::Format("%d", m_basic_clean_iter));
	m_basic_clean_limit_text->SetValue(wxString::Format("%d", m_basic_clean_size_vl));

	//cluster page
	m_cluster_method_exmax_rd->SetValue(m_cluster_method_exmax);
	m_cluster_method_dbscan_rd->SetValue(m_cluster_method_dbscan);
	m_cluster_method_kmeans_rd->SetValue(m_cluster_method_kmeans);
	//parameters
	m_cluster_clnum_text->SetValue(wxString::Format("%d", m_cluster_clnum));
	m_cluster_maxiter_text->SetValue(wxString::Format("%d", m_cluster_maxiter));
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

	//generate
	EnableGenerate();
}

void ComponentDlg::GetSettings()
{
	//defaults
	//initial grow
	m_initial_grow = false;
	m_ig_param_transition = false;
	m_ig_iterations = 50;
	m_ig_translate = m_ig_translate2 = 1.0;
	m_ig_scalar_falloff = m_ig_scalar_falloff2 = 0.15;
	m_ig_grad_falloff = m_ig_grad_falloff2 = 0.1;
	m_ig_var_falloff = m_ig_var_falloff2 = 0.2;
	m_ig_angle_falloff = m_ig_angle_falloff2 = 0.2;

	//sized grow
	m_sized_grow = false;
	m_sg_param_transition = false;
	m_sg_iterations = 40;
	m_sg_size_limiter = m_sg_size_limiter2 = 20;
	m_sg_translate = m_sg_translate2 = 0.5;
	m_sg_scalar_falloff = m_sg_scalar_falloff2 = 0.25;
	m_sg_grad_falloff = m_sg_grad_falloff2 = 0.25;
	m_sg_var_falloff = m_sg_var_falloff2 = 0.35;
	m_sg_angle_falloff = m_sg_angle_falloff2 = 0.35;

	//cleanup
	m_cleanup = false;
	m_cl_iterations = 10;
	m_cl_size_limiter = 5;
	
	//match slices
	m_match_slices = false;
	m_bidir_match = false;
	m_size_thresh = 25;
	m_size_ratio = 0.6;
	m_dist_thresh = 2.5;
	m_angle_thresh = 0.7;

	//basic page
	m_basic_iter = 50;
	m_basic_thresh = 0.5;
	m_basic_diff = false;
	m_basic_falloff = 0.01;
	m_basic_size = false;
	m_basic_size_lm = 100;
	m_basic_density = false;
	m_basic_density_vl = 0.5;
	m_basic_clean = false;
	m_basic_clean_iter = 5;
	m_basic_clean_size_vl = 5;

	//cluster
	m_cluster_method_exmax = true;
	m_cluster_method_dbscan = false;
	m_cluster_method_kmeans = false;
	m_cluster_clnum = 2;
	m_cluster_maxiter = 200;
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
		filename = expath + "/default_component_settings.dft";
		get_basic = true;
	}
	wxFileInputStream is(filename);
	if (!is.IsOk())
		return;
	wxFileConfig fconfig(is);

	//initial grow
	fconfig.Read("initial_grow", &m_initial_grow);
	fconfig.Read("ig_param_transition", &m_ig_param_transition);
	fconfig.Read("ig_iterations", &m_ig_iterations);
	//translate
	fconfig.Read("ig_translate", &m_ig_translate);
	fconfig.Read("ig_translate2", &m_ig_translate2);
	//scalar falloff
	fconfig.Read("ig_scalar_falloff", &m_ig_scalar_falloff);
	fconfig.Read("ig_scalar_falloff2", &m_ig_scalar_falloff2);
	//grad falloff
	fconfig.Read("ig_grad_falloff", &m_ig_grad_falloff);
	fconfig.Read("ig_grad_falloff2", &m_ig_grad_falloff2);
	//variance falloff
	fconfig.Read("ig_var_falloff", &m_ig_var_falloff);
	fconfig.Read("ig_var_falloff2", &m_ig_var_falloff2);
	//angle falloff
	fconfig.Read("ig_angle_falloff", &m_ig_angle_falloff);
	fconfig.Read("ig_angle_falloff2", &m_ig_angle_falloff2);

	//sized grow
	fconfig.Read("sized_grow", &m_sized_grow);
	fconfig.Read("sg_param_transition", &m_sg_param_transition);
	fconfig.Read("sg_iterations", &m_sg_iterations);
	//size limiter
	fconfig.Read("sg_size_limiter", &m_sg_size_limiter);
	fconfig.Read("sg_size_limiter2", &m_sg_size_limiter2);
	//translate
	fconfig.Read("sg_translate", &m_sg_translate);
	fconfig.Read("sg_translate2", &m_sg_translate2);
	//scalar falloff
	fconfig.Read("sg_scalar_falloff", &m_sg_scalar_falloff);
	fconfig.Read("sg_scalar_falloff2", &m_sg_scalar_falloff2);
	//grad falloff
	fconfig.Read("sg_grad_falloff", &m_sg_grad_falloff);
	fconfig.Read("sg_grad_falloff2", &m_sg_grad_falloff2);
	//variance falloff
	fconfig.Read("sg_var_falloff", &m_sg_var_falloff);
	fconfig.Read("sg_var_falloff2", &m_sg_var_falloff2);
	//angle falloff
	fconfig.Read("sg_angle_falloff", &m_sg_angle_falloff);
	fconfig.Read("sg_angle_falloff2", &m_sg_angle_falloff2);

	//cleanup
	fconfig.Read("cleanup", &m_cleanup);
	fconfig.Read("cl_iterations", &m_cl_iterations);
	fconfig.Read("cl_size_limiter", &m_cl_size_limiter);

	//match slices
	fconfig.Read("match_slices", &m_match_slices);
	fconfig.Read("bidir_match", &m_bidir_match);
	fconfig.Read("size_thresh", &m_size_thresh);
	fconfig.Read("size_ratio", &m_size_ratio);
	fconfig.Read("dist_thresh", &m_dist_thresh);
	fconfig.Read("angle_thresh", &m_angle_thresh);

	//basic settings
	if (get_basic)
	{
		//
		fconfig.Read("basic_iter", &m_basic_iter);
		fconfig.Read("basic_thresh", &m_basic_thresh);
		fconfig.Read("basic_diff", &m_basic_diff);
		fconfig.Read("basic_falloff", &m_basic_falloff);
		fconfig.Read("basic_size", &m_basic_size);
		fconfig.Read("basic_size_lm", &m_basic_size_lm);
		fconfig.Read("basic_density", &m_basic_density);
		fconfig.Read("basic_density_vl", &m_basic_density_vl);
		fconfig.Read("basic_clean", &m_basic_clean);
		fconfig.Read("basic_clean_iter", &m_basic_clean_iter);
		fconfig.Read("basic_clean_size_vl", &m_basic_clean_size_vl);

		//cluster
		fconfig.Read("cluster_method_exmax", &m_cluster_method_exmax);
		fconfig.Read("cluster_method_dbscan", &m_cluster_method_dbscan);
		fconfig.Read("cluster_method_kmeans", &m_cluster_method_kmeans);
		//parameters
		fconfig.Read("cluster_clnum", &m_cluster_clnum);
		fconfig.Read("cluster_maxiter", &m_cluster_maxiter);
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
	}

	m_load_settings_text->SetValue(filename);
}

void ComponentDlg::SaveSettings(wxString filename)
{
	wxString app_name = "FluoRender " +
		wxString::Format("%d.%.1f", VERSION_MAJOR, float(VERSION_MINOR));
	wxString vendor_name = "FluoRender";
	wxString local_name = "default_component_settings.dft";
	wxFileConfig fconfig(app_name, vendor_name, local_name, "",
		wxCONFIG_USE_LOCAL_FILE);

	//initial grow
	fconfig.Write("initial_grow", m_initial_grow);
	fconfig.Write("ig_param_transition", m_ig_param_transition);
	fconfig.Write("ig_iterations", m_ig_iterations);
	//translate
	fconfig.Write("ig_translate", m_ig_translate);
	fconfig.Write("ig_translate2", m_ig_translate2);
	//scalar falloff
	fconfig.Write("ig_scalar_falloff", m_ig_scalar_falloff);
	fconfig.Write("ig_scalar_falloff2", m_ig_scalar_falloff2);
	//grad falloff
	fconfig.Write("ig_grad_falloff", m_ig_grad_falloff);
	fconfig.Write("ig_grad_falloff2", m_ig_grad_falloff2);
	//variance falloff
	fconfig.Write("ig_var_falloff", m_ig_var_falloff);
	fconfig.Write("ig_var_falloff2", m_ig_var_falloff2);
	//angle falloff
	fconfig.Write("ig_angle_falloff", m_ig_angle_falloff);
	fconfig.Write("ig_angle_falloff2", m_ig_angle_falloff2);

	//sized grow
	fconfig.Write("sized_grow", m_sized_grow);
	fconfig.Write("sg_param_transition", m_sg_param_transition);
	fconfig.Write("sg_iterations", m_sg_iterations);
	//size limiter
	fconfig.Write("sg_size_limiter", m_sg_size_limiter);
	fconfig.Write("sg_size_limiter2", m_sg_size_limiter2);
	//translate
	fconfig.Write("sg_translate", m_sg_translate);
	fconfig.Write("sg_translate2", m_sg_translate2);
	//scalar falloff
	fconfig.Write("sg_scalar_falloff", m_sg_scalar_falloff);
	fconfig.Write("sg_scalar_falloff2", m_sg_scalar_falloff2);
	//grad falloff
	fconfig.Write("sg_grad_falloff", m_sg_grad_falloff);
	fconfig.Write("sg_grad_falloff2", m_sg_grad_falloff2);
	//variance falloff
	fconfig.Write("sg_var_falloff", m_sg_var_falloff);
	fconfig.Write("sg_var_falloff2", m_sg_var_falloff2);
	//angle falloff
	fconfig.Write("sg_angle_falloff", m_sg_angle_falloff);
	fconfig.Write("sg_angle_falloff2", m_sg_angle_falloff2);

	//cleanup
	fconfig.Write("cleanup", m_cleanup);
	fconfig.Write("cl_iterations", m_cl_iterations);
	fconfig.Write("cl_size_limiter", m_cl_size_limiter);

	//match slices
	fconfig.Write("match_slices", m_match_slices);
	fconfig.Write("bidir_match", m_bidir_match);
	fconfig.Write("size_thresh", m_size_thresh);
	fconfig.Write("size_ratio", m_size_ratio);
	fconfig.Write("dist_thresh", m_dist_thresh);
	fconfig.Write("angle_thresh", m_angle_thresh);

	//basic settings
	fconfig.Write("basic_iter", m_basic_iter);
	fconfig.Write("basic_thresh", m_basic_thresh);
	fconfig.Write("basic_diff", m_basic_diff);
	fconfig.Write("basic_falloff", m_basic_falloff);
	fconfig.Write("basic_size", m_basic_size);
	fconfig.Write("basic_size_lm", m_basic_size_lm);
	fconfig.Write("basic_density", m_basic_density);
	fconfig.Write("basic_density_vl", m_basic_density_vl);
	fconfig.Write("basic_clean", m_basic_clean);
	fconfig.Write("basic_clean_iter", m_basic_clean_iter);
	fconfig.Write("basic_clean_size_vl", m_basic_clean_size_vl);

	//cluster
	fconfig.Write("cluster_method_exmax", m_cluster_method_exmax);
	fconfig.Write("cluster_method_dbscan", m_cluster_method_dbscan);
	fconfig.Write("cluster_method_kmeans", m_cluster_method_kmeans);
	//parameters
	fconfig.Write("cluster_clnum", m_cluster_clnum);
	fconfig.Write("cluster_maxiter", m_cluster_maxiter);
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
		filename = expath + "/default_component_settings.dft";
	}

	wxFileOutputStream os(filename);
	fconfig.Save(os);
}

void ComponentDlg::OnPaneChange(wxCollapsiblePaneEvent& event)
{
	int id = event.GetId();
	bool enable = !event.GetCollapsed();

	switch (id)
	{
	case ID_InitialGrowPane:
		EnableInitialGrow(enable);
		m_initial_grow_check->SetValue(enable);
		break;
	case ID_SizedGrowPane:
		EnableSizedGrow(enable);
		m_sized_grow_check->SetValue(enable);
		break;
	case ID_CleanupPane:
		EnableCleanup(enable);
		m_cleanup_check->SetValue(enable);
		break;
	case ID_MatchSlicesPane:
		EnableMatchSlices(enable);
		m_match_slices_check->SetValue(enable);
		break;
	}

	if (m_adv_page)
		m_adv_page->SendSizeEvent();
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

void ComponentDlg::EnableInitialGrow(bool value)
{
	if (m_initial_grow == value)
		return;
	m_initial_grow = value;
	if (m_initial_grow)
	{
		m_ig_param_transition_check->Enable();
		m_ig_iterations_sldr->Enable();
		m_ig_iterations_text->Enable();
		m_ig_translate_sldr->Enable();
		m_ig_translate_text->Enable();
		m_ig_translate2_sldr->Enable();
		m_ig_translate2_text->Enable();
		m_ig_scalar_falloff_sldr->Enable();
		m_ig_scalar_falloff_text->Enable();
		m_ig_scalar_falloff2_sldr->Enable();
		m_ig_scalar_falloff2_text->Enable();
		m_ig_grad_falloff_sldr->Enable();
		m_ig_grad_falloff_text->Enable();
		m_ig_grad_falloff2_sldr->Enable();
		m_ig_grad_falloff2_text->Enable();
		m_ig_var_falloff_sldr->Enable();
		m_ig_var_falloff_text->Enable();
		m_ig_var_falloff2_sldr->Enable();
		m_ig_var_falloff2_text->Enable();
		m_ig_angle_falloff_sldr->Enable();
		m_ig_angle_falloff_text->Enable();
		m_ig_angle_falloff2_sldr->Enable();
		m_ig_angle_falloff2_text->Enable();
	}
	else
	{
		m_ig_param_transition_check->Disable();
		m_ig_iterations_sldr->Disable();
		m_ig_iterations_text->Disable();
		m_ig_translate_sldr->Disable();
		m_ig_translate_text->Disable();
		m_ig_translate2_sldr->Disable();
		m_ig_translate2_text->Disable();
		m_ig_scalar_falloff_sldr->Disable();
		m_ig_scalar_falloff_text->Disable();
		m_ig_scalar_falloff2_sldr->Disable();
		m_ig_scalar_falloff2_text->Disable();
		m_ig_grad_falloff_sldr->Disable();
		m_ig_grad_falloff_text->Disable();
		m_ig_grad_falloff2_sldr->Disable();
		m_ig_grad_falloff2_text->Disable();
		m_ig_var_falloff_sldr->Disable();
		m_ig_var_falloff_text->Disable();
		m_ig_var_falloff2_sldr->Disable();
		m_ig_var_falloff2_text->Disable();
		m_ig_angle_falloff_sldr->Disable();
		m_ig_angle_falloff_text->Disable();
		m_ig_angle_falloff2_sldr->Disable();
		m_ig_angle_falloff2_text->Disable();
	}
}

void ComponentDlg::EnableIGParamTransition(bool value)
{
	m_ig_param_transition = value;
	if (m_ig_param_transition)
	{
		m_ig_translate_st->Show();
		m_ig_translate2_sldr->Show();
		m_ig_translate2_text->Show();
		m_ig_scalar_falloff_st->Show();
		m_ig_scalar_falloff2_sldr->Show();
		m_ig_scalar_falloff2_text->Show();
		m_ig_grad_falloff_st->Show();
		m_ig_grad_falloff2_sldr->Show();
		m_ig_grad_falloff2_text->Show();
		m_ig_var_falloff_st->Show();
		m_ig_var_falloff2_sldr->Show();
		m_ig_var_falloff2_text->Show();
		m_ig_angle_falloff_st->Show();
		m_ig_angle_falloff2_sldr->Show();
		m_ig_angle_falloff2_text->Show();
	}
	else
	{
		m_ig_translate_st->Hide();
		m_ig_translate2_sldr->Hide();
		m_ig_translate2_text->Hide();
		m_ig_scalar_falloff_st->Hide();
		m_ig_scalar_falloff2_sldr->Hide();
		m_ig_scalar_falloff2_text->Hide();
		m_ig_grad_falloff_st->Hide();
		m_ig_grad_falloff2_sldr->Hide();
		m_ig_grad_falloff2_text->Hide();
		m_ig_var_falloff_st->Hide();
		m_ig_var_falloff2_sldr->Hide();
		m_ig_var_falloff2_text->Hide();
		m_ig_angle_falloff_st->Hide();
		m_ig_angle_falloff2_sldr->Hide();
		m_ig_angle_falloff2_text->Hide();
	}

	if (m_adv_page)
		m_adv_page->SendSizeEvent();
}

void ComponentDlg::OnInitialGrowCheck(wxCommandEvent &event)
{
	EnableInitialGrow(m_initial_grow_check->GetValue());
}

void ComponentDlg::OnIGParamTransitionCheck(wxCommandEvent &event)
{
	EnableIGParamTransition(m_ig_param_transition_check->GetValue());

	if (m_ig_param_transition)
	{
		//copy values
		m_ig_translate2_text->SetValue(wxString::Format("%.3f", m_ig_translate));
		m_ig_scalar_falloff2_text->SetValue(wxString::Format("%.3f", m_ig_scalar_falloff));
		m_ig_grad_falloff2_text->SetValue(wxString::Format("%.3f", m_ig_grad_falloff));
		m_ig_var_falloff2_text->SetValue(wxString::Format("%.3f", m_ig_var_falloff));
		m_ig_angle_falloff2_text->SetValue(wxString::Format("%.3f", m_ig_angle_falloff));
	}
}

void ComponentDlg::OnIGIterationsSldr(wxScrollEvent &event)
{
	int val = event.GetPosition();
	m_ig_iterations_text->SetValue(wxString::Format("%d", val));
}

void ComponentDlg::OnIGIterationsText(wxCommandEvent &event)
{
	long val = 0;
	m_ig_iterations_text->GetValue().ToLong(&val);
	m_ig_iterations = val;
	m_ig_iterations_sldr->SetValue(m_ig_iterations);
}

//translate
void ComponentDlg::OnIGTranslateSldr(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 1000.0;
	m_ig_translate_text->SetValue(wxString::Format("%.3f", val));
}

void ComponentDlg::OnIGTranslateText(wxCommandEvent &event)
{
	double val = 0.0;
	m_ig_translate_text->GetValue().ToDouble(&val);
	m_ig_translate = val;
	m_ig_translate_sldr->SetValue(int(m_ig_translate * 1000.0 + 0.5));
}

void ComponentDlg::OnIGTranslate2Sldr(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 1000.0;
	m_ig_translate2_text->SetValue(wxString::Format("%.3f", val));
}

void ComponentDlg::OnIGTranslate2Text(wxCommandEvent &event)
{
	double val = 0.0;
	m_ig_translate2_text->GetValue().ToDouble(&val);
	m_ig_translate2 = val;
	m_ig_translate2_sldr->SetValue(int(m_ig_translate2 * 1000.0 + 0.5));
}

//scalar falloff
void ComponentDlg::OnIGScalarFalloffSldr(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 1000.0;
	m_ig_scalar_falloff_text->SetValue(wxString::Format("%.3f", val));
}

void ComponentDlg::OnIGScalarFalloffText(wxCommandEvent &event)
{
	double val = 0.0;
	m_ig_scalar_falloff_text->GetValue().ToDouble(&val);
	m_ig_scalar_falloff = val;
	m_ig_scalar_falloff_sldr->SetValue(int(m_ig_scalar_falloff * 1000.0 + 0.5));
}

void ComponentDlg::OnIGScalarFalloff2Sldr(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 1000.0;
	m_ig_scalar_falloff2_text->SetValue(wxString::Format("%.3f", val));
}

void ComponentDlg::OnIGScalarFalloff2Text(wxCommandEvent &event)
{
	double val = 0.0;
	m_ig_scalar_falloff2_text->GetValue().ToDouble(&val);
	m_ig_scalar_falloff2 = val;
	m_ig_scalar_falloff2_sldr->SetValue(int(m_ig_scalar_falloff2 * 1000.0 + 0.5));
}

//grad falloff
void ComponentDlg::OnIGGradFalloffSldr(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 1000.0;
	m_ig_grad_falloff_text->SetValue(wxString::Format("%.3f", val));
}

void ComponentDlg::OnIGGradFalloffText(wxCommandEvent &event)
{
	double val = 0.0;
	m_ig_grad_falloff_text->GetValue().ToDouble(&val);
	m_ig_grad_falloff = val;
	m_ig_grad_falloff_sldr->SetValue(int(m_ig_grad_falloff * 1000.0 + 0.5));
}

void ComponentDlg::OnIGGradFalloff2Sldr(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 1000.0;
	m_ig_grad_falloff2_text->SetValue(wxString::Format("%.3f", val));
}

void ComponentDlg::OnIGGradFalloff2Text(wxCommandEvent &event)
{
	double val = 0.0;
	m_ig_grad_falloff2_text->GetValue().ToDouble(&val);
	m_ig_grad_falloff2 = val;
	m_ig_grad_falloff2_sldr->SetValue(int(m_ig_grad_falloff2 * 1000.0 + 0.5));
}

//variance falloff
void ComponentDlg::OnIGVarFalloffSldr(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 1000.0;
	m_ig_var_falloff_text->SetValue(wxString::Format("%.3f", val));
}

void ComponentDlg::OnIGVarFalloffText(wxCommandEvent &event)
{
	double val = 0.0;
	m_ig_var_falloff_text->GetValue().ToDouble(&val);
	m_ig_var_falloff = val;
	m_ig_var_falloff_sldr->SetValue(int(m_ig_var_falloff * 1000.0 + 0.5));
}

void ComponentDlg::OnIGVarFalloff2Sldr(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 1000.0;
	m_ig_var_falloff2_text->SetValue(wxString::Format("%.3f", val));
}

void ComponentDlg::OnIGVarFalloff2Text(wxCommandEvent &event)
{
	double val = 0.0;
	m_ig_var_falloff2_text->GetValue().ToDouble(&val);
	m_ig_var_falloff2 = val;
	m_ig_var_falloff2_sldr->SetValue(int(m_ig_var_falloff2 * 1000.0 + 0.5));
}

//angle falloff
void ComponentDlg::OnIGAngleFalloffSldr(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 1000.0;
	m_ig_angle_falloff_text->SetValue(wxString::Format("%.3f", val));
}

void ComponentDlg::OnIGAngleFalloffText(wxCommandEvent &event)
{
	double val = 0.0;
	m_ig_angle_falloff_text->GetValue().ToDouble(&val);
	m_ig_angle_falloff = val;
	m_ig_angle_falloff_sldr->SetValue(int(m_ig_angle_falloff * 1000.0 + 0.5));
}

void ComponentDlg::OnIGAngleFalloff2Sldr(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 1000.0;
	m_ig_angle_falloff2_text->SetValue(wxString::Format("%.3f", val));
}

void ComponentDlg::OnIGAngleFalloff2Text(wxCommandEvent &event)
{
	double val = 0.0;
	m_ig_angle_falloff2_text->GetValue().ToDouble(&val);
	m_ig_angle_falloff2 = val;
	m_ig_angle_falloff2_sldr->SetValue(int(m_ig_angle_falloff2 * 1000.0 + 0.5));
}

//sized grow
void ComponentDlg::EnableSizedGrow(bool value)
{
	if (m_sized_grow == value)
		return;
	m_sized_grow = value;
	if (m_sized_grow)
	{
		m_sg_param_transition_check->Enable();
		m_sg_iterations_sldr->Enable();
		m_sg_iterations_text->Enable();
		m_sg_size_limiter_sldr->Enable();
		m_sg_size_limiter_text->Enable();
		m_sg_size_limiter2_sldr->Enable();
		m_sg_size_limiter2_text->Enable();
		m_sg_translate_sldr->Enable();
		m_sg_translate_text->Enable();
		m_sg_translate2_sldr->Enable();
		m_sg_translate2_text->Enable();
		m_sg_scalar_falloff_sldr->Enable();
		m_sg_scalar_falloff_text->Enable();
		m_sg_scalar_falloff2_sldr->Enable();
		m_sg_scalar_falloff2_text->Enable();
		m_sg_grad_falloff_sldr->Enable();
		m_sg_grad_falloff_text->Enable();
		m_sg_grad_falloff2_sldr->Enable();
		m_sg_grad_falloff2_text->Enable();
		m_sg_var_falloff_sldr->Enable();
		m_sg_var_falloff_text->Enable();
		m_sg_var_falloff2_sldr->Enable();
		m_sg_var_falloff2_text->Enable();
		m_sg_angle_falloff_sldr->Enable();
		m_sg_angle_falloff_text->Enable();
		m_sg_angle_falloff2_sldr->Enable();
		m_sg_angle_falloff2_text->Enable();
	}
	else
	{
		m_sg_param_transition_check->Disable();
		m_sg_iterations_sldr->Disable();
		m_sg_iterations_text->Disable();
		m_sg_size_limiter_sldr->Disable();
		m_sg_size_limiter_text->Disable();
		m_sg_size_limiter2_sldr->Disable();
		m_sg_size_limiter2_text->Disable();
		m_sg_translate_sldr->Disable();
		m_sg_translate_text->Disable();
		m_sg_translate2_sldr->Disable();
		m_sg_translate2_text->Disable();
		m_sg_scalar_falloff_sldr->Disable();
		m_sg_scalar_falloff_text->Disable();
		m_sg_scalar_falloff2_sldr->Disable();
		m_sg_scalar_falloff2_text->Disable();
		m_sg_grad_falloff_sldr->Disable();
		m_sg_grad_falloff_text->Disable();
		m_sg_grad_falloff2_sldr->Disable();
		m_sg_grad_falloff2_text->Disable();
		m_sg_var_falloff_sldr->Disable();
		m_sg_var_falloff_text->Disable();
		m_sg_var_falloff2_sldr->Disable();
		m_sg_var_falloff2_text->Disable();
		m_sg_angle_falloff_sldr->Disable();
		m_sg_angle_falloff_text->Disable();
		m_sg_angle_falloff2_sldr->Disable();
		m_sg_angle_falloff2_text->Disable();
	}
}

void ComponentDlg::EnableSGParamTransition(bool value)
{
	m_sg_param_transition = value;
	if (m_sg_param_transition)
	{
		m_sg_size_limiter_st->Show();
		m_sg_size_limiter2_sldr->Show();
		m_sg_size_limiter2_text->Show();
		m_sg_translate_st->Show();
		m_sg_translate2_sldr->Show();
		m_sg_translate2_text->Show();
		m_sg_scalar_falloff_st->Show();
		m_sg_scalar_falloff2_sldr->Show();
		m_sg_scalar_falloff2_text->Show();
		m_sg_grad_falloff_st->Show();
		m_sg_grad_falloff2_sldr->Show();
		m_sg_grad_falloff2_text->Show();
		m_sg_var_falloff_st->Show();
		m_sg_var_falloff2_sldr->Show();
		m_sg_var_falloff2_text->Show();
		m_sg_angle_falloff_st->Show();
		m_sg_angle_falloff2_sldr->Show();
		m_sg_angle_falloff2_text->Show();
	}
	else
	{
		m_sg_size_limiter_st->Hide();
		m_sg_size_limiter2_sldr->Hide();
		m_sg_size_limiter2_text->Hide();
		m_sg_translate_st->Hide();
		m_sg_translate2_sldr->Hide();
		m_sg_translate2_text->Hide();
		m_sg_scalar_falloff_st->Hide();
		m_sg_scalar_falloff2_sldr->Hide();
		m_sg_scalar_falloff2_text->Hide();
		m_sg_grad_falloff_st->Hide();
		m_sg_grad_falloff2_sldr->Hide();
		m_sg_grad_falloff2_text->Hide();
		m_sg_var_falloff_st->Hide();
		m_sg_var_falloff2_sldr->Hide();
		m_sg_var_falloff2_text->Hide();
		m_sg_angle_falloff_st->Hide();
		m_sg_angle_falloff2_sldr->Hide();
		m_sg_angle_falloff2_text->Hide();
	}

	if (m_adv_page)
		m_adv_page->SendSizeEvent();
}

void ComponentDlg::OnSizedGrowCheck(wxCommandEvent &event)
{
	EnableSizedGrow(m_sized_grow_check->GetValue());
}

void ComponentDlg::OnSGParamTransitionCheck(wxCommandEvent &event)
{
	EnableSGParamTransition(m_sg_param_transition_check->GetValue());

	if (m_sg_param_transition)
	{
		//copy values
		m_sg_size_limiter2_text->SetValue(wxString::Format("%d", m_sg_size_limiter));
		m_sg_translate2_text->SetValue(wxString::Format("%.3f", m_sg_translate));
		m_sg_scalar_falloff2_text->SetValue(wxString::Format("%.3f", m_sg_scalar_falloff));
		m_sg_grad_falloff2_text->SetValue(wxString::Format("%.3f", m_sg_grad_falloff));
		m_sg_var_falloff2_text->SetValue(wxString::Format("%.3f", m_sg_var_falloff));
		m_sg_angle_falloff2_text->SetValue(wxString::Format("%.3f", m_sg_angle_falloff));
	}
}

void ComponentDlg::OnSGIterationsSldr(wxScrollEvent &event)
{
	int val = event.GetPosition();
	m_sg_iterations_text->SetValue(wxString::Format("%d", val));
}

void ComponentDlg::OnSGIterationsText(wxCommandEvent &event)
{
	long val = 0;
	m_sg_iterations_text->GetValue().ToLong(&val);
	m_sg_iterations = val;
	m_sg_iterations_sldr->SetValue(m_sg_iterations);
}

//size limiter
void ComponentDlg::OnSGSizeLimiterSldr(wxScrollEvent &event)
{
	int val = event.GetPosition();
	m_sg_size_limiter_text->SetValue(wxString::Format("%d", val));
}

void ComponentDlg::OnSGSizeLimiterText(wxCommandEvent &event)
{
	long val = 0;
	m_sg_size_limiter_text->GetValue().ToLong(&val);
	m_sg_size_limiter = val;
	m_sg_size_limiter_sldr->SetValue(m_sg_size_limiter);
}

void ComponentDlg::OnSGSizeLimiter2Sldr(wxScrollEvent &event)
{
	int val = event.GetPosition();
	m_sg_size_limiter2_text->SetValue(wxString::Format("%d", val));
}

void ComponentDlg::OnSGSizeLimiter2Text(wxCommandEvent &event)
{
	long val = 0;
	m_sg_size_limiter2_text->GetValue().ToLong(&val);
	m_sg_size_limiter2 = val;
	m_sg_size_limiter2_sldr->SetValue(m_sg_size_limiter2);
}

//translate
void ComponentDlg::OnSGTranslateSldr(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 1000.0;
	m_sg_translate_text->SetValue(wxString::Format("%.3f", val));
}

void ComponentDlg::OnSGTranslateText(wxCommandEvent &event)
{
	double val = 0.0;
	m_sg_translate_text->GetValue().ToDouble(&val);
	m_sg_translate = val;
	m_sg_translate_sldr->SetValue(int(m_sg_translate * 1000.0 + 0.5));
}

void ComponentDlg::OnSGTranslate2Sldr(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 1000.0;
	m_sg_translate2_text->SetValue(wxString::Format("%.3f", val));
}

void ComponentDlg::OnSGTranslate2Text(wxCommandEvent &event)
{
	double val = 0.0;
	m_sg_translate2_text->GetValue().ToDouble(&val);
	m_sg_translate2 = val;
	m_sg_translate2_sldr->SetValue(int(m_sg_translate2 * 1000.0 + 0.5));
}

//scalar falloff
void ComponentDlg::OnSGScalarFalloffSldr(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 1000.0;
	m_sg_scalar_falloff_text->SetValue(wxString::Format("%.3f", val));
}

void ComponentDlg::OnSGScalarFalloffText(wxCommandEvent &event)
{
	double val = 0.0;
	m_sg_scalar_falloff_text->GetValue().ToDouble(&val);
	m_sg_scalar_falloff = val;
	m_sg_scalar_falloff_sldr->SetValue(int(m_sg_scalar_falloff * 1000.0 + 0.5));
}

void ComponentDlg::OnSGScalarFalloff2Sldr(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 1000.0;
	m_sg_scalar_falloff2_text->SetValue(wxString::Format("%.3f", val));
}

void ComponentDlg::OnSGScalarFalloff2Text(wxCommandEvent &event)
{
	double val = 0.0;
	m_sg_scalar_falloff2_text->GetValue().ToDouble(&val);
	m_sg_scalar_falloff2 = val;
	m_sg_scalar_falloff2_sldr->SetValue(int(m_sg_scalar_falloff2 * 1000.0 + 0.5));
}

//grad falloff
void ComponentDlg::OnSGGradFalloffSldr(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 1000.0;
	m_sg_grad_falloff_text->SetValue(wxString::Format("%.3f", val));
}

void ComponentDlg::OnSGGradFalloffText(wxCommandEvent &event)
{
	double val = 0.0;
	m_sg_grad_falloff_text->GetValue().ToDouble(&val);
	m_sg_grad_falloff = val;
	m_sg_grad_falloff_sldr->SetValue(int(m_sg_grad_falloff * 1000.0 + 0.5));
}

void ComponentDlg::OnSGGradFalloff2Sldr(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 1000.0;
	m_sg_grad_falloff2_text->SetValue(wxString::Format("%.3f", val));
}

void ComponentDlg::OnSGGradFalloff2Text(wxCommandEvent &event)
{
	double val = 0.0;
	m_sg_grad_falloff2_text->GetValue().ToDouble(&val);
	m_sg_grad_falloff2 = val;
	m_sg_grad_falloff2_sldr->SetValue(int(m_sg_grad_falloff2 * 1000.0 + 0.5));
}

//variance falloff
void ComponentDlg::OnSGVarFalloffSldr(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 1000.0;
	m_sg_var_falloff_text->SetValue(wxString::Format("%.3f", val));
}

void ComponentDlg::OnSGVarFalloffText(wxCommandEvent &event)
{
	double val = 0.0;
	m_sg_var_falloff_text->GetValue().ToDouble(&val);
	m_sg_var_falloff = val;
	m_sg_var_falloff_sldr->SetValue(int(m_sg_var_falloff * 1000.0 + 0.5));
}

void ComponentDlg::OnSGVarFalloff2Sldr(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 1000.0;
	m_sg_var_falloff2_text->SetValue(wxString::Format("%.3f", val));
}

void ComponentDlg::OnSGVarFalloff2Text(wxCommandEvent &event)
{
	double val = 0.0;
	m_sg_var_falloff2_text->GetValue().ToDouble(&val);
	m_sg_var_falloff2 = val;
	m_sg_var_falloff2_sldr->SetValue(int(m_sg_var_falloff2 * 1000.0 + 0.5));
}

//angle falloff
void ComponentDlg::OnSGAngleFalloffSldr(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 1000.0;
	m_sg_angle_falloff_text->SetValue(wxString::Format("%.3f", val));
}

void ComponentDlg::OnSGAngleFalloffText(wxCommandEvent &event)
{
	double val = 0.0;
	m_sg_angle_falloff_text->GetValue().ToDouble(&val);
	m_sg_angle_falloff = val;
	m_sg_angle_falloff_sldr->SetValue(int(m_sg_angle_falloff * 1000.0 + 0.5));
}

void ComponentDlg::OnSGAngleFalloff2Sldr(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 1000.0;
	m_sg_angle_falloff2_text->SetValue(wxString::Format("%.3f", val));
}

void ComponentDlg::OnSGAngleFalloff2Text(wxCommandEvent &event)
{
	double val = 0.0;
	m_sg_angle_falloff2_text->GetValue().ToDouble(&val);
	m_sg_angle_falloff2 = val;
	m_sg_angle_falloff2_sldr->SetValue(int(m_sg_angle_falloff2 * 1000.0 + 0.5));
}

//cleanup
void ComponentDlg::EnableCleanup(bool value)
{
	if (m_cleanup == value)
		return;
	m_cleanup = value;
	if (m_cleanup)
	{
		m_cl_iterations_sldr->Enable();
		m_cl_iterations_text->Enable();
		m_cl_size_limiter_sldr->Enable();
		m_cl_size_limiter_text->Enable();
	}
	else
	{
		m_cl_iterations_sldr->Disable();
		m_cl_iterations_text->Disable();
		m_cl_size_limiter_sldr->Disable();
		m_cl_size_limiter_text->Disable();
	}
}

void ComponentDlg::OnCleanupCheck(wxCommandEvent &event)
{
	EnableCleanup(m_cleanup_check->GetValue());
}

void ComponentDlg::OnCLIterationsSldr(wxScrollEvent &event)
{
	int val = event.GetPosition();
	m_cl_iterations_text->SetValue(wxString::Format("%d", val));
}

void ComponentDlg::OnCLIterationsText(wxCommandEvent &event)
{
	long val = 0;
	m_cl_iterations_text->GetValue().ToLong(&val);
	m_cl_iterations = val;
	m_cl_iterations_sldr->SetValue(m_cl_iterations);
}

void ComponentDlg::OnCLSizeLimiterSldr(wxScrollEvent &event)
{
	int val = event.GetPosition();
	m_cl_size_limiter_text->SetValue(wxString::Format("%d", val));
}

void ComponentDlg::OnCLSizeLimiterText(wxCommandEvent &event)
{
	long val = 0;
	m_cl_size_limiter_text->GetValue().ToLong(&val);
	m_cl_size_limiter = val;
	m_cl_size_limiter_sldr->SetValue(m_cl_size_limiter);
}

//match slices
void ComponentDlg::EnableMatchSlices(bool value)
{
	if (m_match_slices == value)
		return;
	m_match_slices = value;
	if (m_match_slices)
	{
		m_bidir_match_check->Enable();
		m_size_thresh_sldr->Enable();
		m_size_thresh_text->Enable();
		m_size_ratio_sldr->Enable();
		m_size_ratio_text->Enable();
		m_dist_thresh_sldr->Enable();
		m_dist_thresh_text->Enable();
		m_angle_thresh_sldr->Enable();
		m_angle_thresh_text->Enable();
	}
	else
	{
		m_bidir_match_check->Disable();
		m_size_thresh_sldr->Disable();
		m_size_thresh_text->Disable();
		m_size_ratio_sldr->Disable();
		m_size_ratio_text->Disable();
		m_dist_thresh_sldr->Disable();
		m_dist_thresh_text->Disable();
		m_angle_thresh_sldr->Disable();
		m_angle_thresh_text->Disable();
	}
}

void ComponentDlg::OnMatchSlicesCheck(wxCommandEvent &event)
{
	EnableMatchSlices(m_match_slices_check->GetValue());
}

void ComponentDlg::OnBidirMatchCheck(wxCommandEvent &event)
{
	m_bidir_match = m_bidir_match_check->GetValue();
}

void ComponentDlg::OnSizeThreshSldr(wxScrollEvent &event)
{
	int val = event.GetPosition();
	m_size_thresh_text->SetValue(wxString::Format("%d", val));
}

void ComponentDlg::OnSizeThreshText(wxCommandEvent &event)
{
	long val = 0;
	m_size_thresh_text->GetValue().ToLong(&val);
	m_size_thresh = val;
	m_size_thresh_sldr->SetValue(m_size_thresh);
}

void ComponentDlg::OnSizeRatioSldr(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 1000.0;
	m_size_ratio_text->SetValue(wxString::Format("%.3f", val));
}

void ComponentDlg::OnSizeRatioText(wxCommandEvent &event)
{
	double val = 0.0;
	m_size_ratio_text->GetValue().ToDouble(&val);
	m_size_ratio = val;
	m_size_ratio_sldr->SetValue(int(m_size_ratio * 1000.0 + 0.5));
}

void ComponentDlg::OnDistThreshSldr(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 100.0;
	m_dist_thresh_text->SetValue(wxString::Format("%.3f", val));
}

void ComponentDlg::OnDistThreshText(wxCommandEvent &event)
{
	double val = 0.0;
	m_dist_thresh_text->GetValue().ToDouble(&val);
	m_dist_thresh = val;
	m_dist_thresh_sldr->SetValue(int(m_dist_thresh * 100.0 + 0.5));
}

void ComponentDlg::OnAngleThreshSldr(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 1000.0;
	m_angle_thresh_text->SetValue(wxString::Format("%.3f", val));
}

void ComponentDlg::OnAngleThreshText(wxCommandEvent &event)
{
	double val = 0.0;
	m_angle_thresh_text->GetValue().ToDouble(&val);
	m_angle_thresh = val;
	m_angle_thresh_sldr->SetValue(int(m_angle_thresh * 1000.0 + 0.5));
}

//basic page
void ComponentDlg::OnBasicIterSldr(wxScrollEvent &event)
{
	int val = event.GetPosition();
	m_basic_iter_text->SetValue(wxString::Format("%d", val));
}

void ComponentDlg::OnBasicIterText(wxCommandEvent &event)
{
	long val = 0;
	m_basic_iter_text->GetValue().ToLong(&val);
	m_basic_iter = val;
	m_basic_iter_sldr->SetValue(m_basic_iter);
}

void ComponentDlg::OnBasicThreshSldr(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 1000.0;
	m_basic_thresh_text->SetValue(wxString::Format("%.3f", val));
}

void ComponentDlg::OnBasicThreshText(wxCommandEvent &event)
{
	double val = 0.0;
	m_basic_thresh_text->GetValue().ToDouble(&val);
	m_basic_thresh = val;
	m_basic_thresh_sldr->SetValue(int(m_basic_thresh * 1000.0 + 0.5));
}

void ComponentDlg::EnableBasicDiff(bool value)
{
	m_basic_diff = value;
	if (m_basic_diff)
	{
		m_basic_falloff_sldr->Enable();
		m_basic_falloff_text->Enable();
	}
	else
	{
		m_basic_falloff_sldr->Disable();
		m_basic_falloff_text->Disable();
	}
}

void ComponentDlg::OnBasicDiffCheck(wxCommandEvent &event)
{
	EnableBasicDiff(m_basic_diff_check->GetValue());
}

void ComponentDlg::OnBasicFalloffSldr(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 1000.0;
	m_basic_falloff_text->SetValue(wxString::Format("%.3f", val));
}

void ComponentDlg::OnBasicFalloffText(wxCommandEvent &event)
{
	double val = 0.0;
	m_basic_falloff_text->GetValue().ToDouble(&val);
	m_basic_falloff = val;
	m_basic_falloff_sldr->SetValue(int(m_basic_falloff * 1000.0 + 0.5));
}

void ComponentDlg::EnableBasicSize(bool value)
{
	m_basic_size = value;
	if (m_basic_size)
	{
		m_basic_size_sldr->Enable();
		m_basic_size_text->Enable();
	}
	else
	{
		m_basic_size_sldr->Disable();
		m_basic_size_text->Disable();
	}
}

void ComponentDlg::OnBasicSizeCheck(wxCommandEvent &event)
{
	EnableBasicSize(m_basic_size_check->GetValue());
}

void ComponentDlg::OnBasicSizeSldr(wxScrollEvent &event)
{
	int val = event.GetPosition();
	m_basic_size_text->SetValue(wxString::Format("%d", val));
}

void ComponentDlg::OnBasicSizeText(wxCommandEvent &event)
{
	long val = 0;
	m_basic_size_text->GetValue().ToLong(&val);
	m_basic_size_lm = (int)val;
	m_basic_size_sldr->SetValue(m_basic_size_lm);
}

void ComponentDlg::EnableBasicDensity(bool value)
{
	m_basic_density = value;
	if (m_basic_density)
	{
		m_basic_density_sldr->Enable();
		m_basic_density_text->Enable();
	}
	else
	{
		m_basic_density_sldr->Disable();
		m_basic_density_text->Disable();
	}
}

void ComponentDlg::OnBasicDensityCheck(wxCommandEvent &event)
{
	EnableBasicDensity(m_basic_density_check->GetValue());
}

void ComponentDlg::OnBasicDensitySldr(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 1000.0;
	m_basic_density_text->SetValue(wxString::Format("%.3f", val));
}

void ComponentDlg::OnBasicDensityText(wxCommandEvent &event)
{
	double val = 0.0;
	m_basic_density_text->GetValue().ToDouble(&val);
	m_basic_density_vl = val;
	m_basic_density_sldr->SetValue(int(m_basic_density_vl * 1000.0 + 0.5));
}

void ComponentDlg::EnableBasicClean(bool value)
{
	m_basic_clean = value;
	if (m_basic_clean)
	{
		m_basic_clean_iter_sldr->Enable();
		m_basic_clean_iter_text->Enable();
		m_basic_clean_limit_sldr->Enable();
		m_basic_clean_limit_text->Enable();
	}
	else
	{
		m_basic_clean_iter_sldr->Disable();
		m_basic_clean_iter_text->Disable();
		m_basic_clean_limit_sldr->Disable();
		m_basic_clean_limit_text->Disable();
	}
}

void ComponentDlg::OnBasicCleanCheck(wxCommandEvent &event)
{
	EnableBasicClean(m_basic_clean_check->GetValue());
}

void ComponentDlg::OnBasicCleanIterSldr(wxScrollEvent &event)
{
	int val = event.GetPosition();
	m_basic_clean_iter_text->SetValue(wxString::Format("%d", val));
}

void ComponentDlg::OnBasicCleanIterText(wxCommandEvent &event)
{
	long val = 0;
	m_basic_clean_iter_text->GetValue().ToLong(&val);
	m_basic_clean_iter = (int)val;
	m_basic_clean_iter_sldr->SetValue(m_basic_clean_iter);
}

void ComponentDlg::OnBasicCleanLimitSldr(wxScrollEvent &event)
{
	int val = event.GetPosition();
	m_basic_clean_limit_text->SetValue(wxString::Format("%d", val));
}

void ComponentDlg::OnBasicCleanLimitText(wxCommandEvent &event)
{
	long val = 0;
	m_basic_clean_limit_text->GetValue().ToLong(&val);
	m_basic_clean_size_vl = (int)val;
	m_basic_clean_limit_sldr->SetValue(m_basic_clean_size_vl);
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
	m_cluster_clnum_text->SetValue(wxString::Format("%d", val));
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
	m_cluster_maxiter_text->SetValue(wxString::Format("%d", val));
}

void ComponentDlg::OnClusterMaxiterText(wxCommandEvent &event)
{
	long val = 0;
	m_cluster_maxiter_text->GetValue().ToLong(&val);
	m_cluster_maxiter = (int)val;
	m_cluster_maxiter_sldr->SetValue(m_cluster_maxiter);
}

void ComponentDlg::OnClusterSizeSldr(wxScrollEvent &event)
{
	int val = event.GetPosition();
	m_cluster_size_text->SetValue(wxString::Format("%d", val));
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
	m_cluster_eps_text->SetValue(wxString::Format("%.1f", val));
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
	wxString str = m_comp_id_text->GetValue();
	unsigned long id;
	wxColor color(255, 255, 255);
	if (str.ToULong(&id))
	{
		if (!id)
			color = wxColor(24, 167, 181);
		else
		{
			Color c = HSVColor(id % 360, 1.0, 1.0);
			color = wxColor(c.r() * 255, c.g() * 255, c.b() * 255);
		}
		m_comp_id_text->SetBackgroundColour(color);
	}
	else
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

void ComponentDlg::OnCompFull(wxCommandEvent &event)
{
	SelectFullComp();
}

void ComponentDlg::OnCompExclusive(wxCommandEvent &event)
{
	if (!m_view)
		return;

	wxString str;
	unsigned long ival;
	//get id
	unsigned int id;
	int brick_id;
	str = m_comp_id_text->GetValue();

	if (GetIds(str.ToStdString(), id, brick_id))
	{
		//get current mask
		VolumeData* vd = m_view->m_glview->m_cur_vol;
		FL::ComponentSelector comp_selector(vd);
		comp_selector.SetId(id);
		comp_selector.SetBrickId(brick_id);

		//cell size filter
		bool use = m_analysis_min_check->GetValue();
		unsigned int num = (unsigned int)(m_analysis_min_spin->GetValue());
		comp_selector.SetMinNum(use, num);
		use = m_analysis_max_check->GetValue();
		num = (unsigned int)(m_analysis_max_spin->GetValue());
		comp_selector.SetMaxNum(use, num);
		comp_selector.SetAnalyzer(&m_comp_analyzer);
		comp_selector.Exclusive();

		m_view->RefreshGL();

		//frame
		VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
		if (vr_frame && vr_frame->GetBrushToolDlg())
			vr_frame->GetBrushToolDlg()->UpdateUndoRedo();
	}
}

void ComponentDlg::OnCompAppend(wxCommandEvent &event)
{
	if (!m_view)
		return;

	wxString str;
	//get id
	unsigned int id;
	int brick_id;
	str = m_comp_id_text->GetValue();
	bool get_all = GetIds(str.ToStdString(), id, brick_id);
	get_all = !get_all;

	//get current mask
	VolumeData* vd = m_view->m_glview->m_cur_vol;
	FL::ComponentSelector comp_selector(vd);
	comp_selector.SetId(id);
	comp_selector.SetBrickId(brick_id);

	//cell size filter
	bool use = m_analysis_min_check->GetValue();
	unsigned int num = (unsigned int)(m_analysis_min_spin->GetValue());
	comp_selector.SetMinNum(use, num);
	use = m_analysis_max_check->GetValue();
	num = (unsigned int)(m_analysis_max_spin->GetValue());
	comp_selector.SetMaxNum(use, num);
	comp_selector.SetAnalyzer(&m_comp_analyzer);
	comp_selector.Append(get_all);

	m_view->RefreshGL();

	//frame
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetBrushToolDlg())
		vr_frame->GetBrushToolDlg()->UpdateUndoRedo();
}

void ComponentDlg::OnCompAll(wxCommandEvent &event)
{
	if (!m_view)
		return;

	//get current vd
	VolumeData* vd = m_view->m_glview->m_cur_vol;
	FL::ComponentSelector comp_selector(vd);
	comp_selector.All();

	m_view->RefreshGL();

	//frame
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetBrushToolDlg())
		vr_frame->GetBrushToolDlg()->UpdateUndoRedo();
}

void ComponentDlg::OnCompClear(wxCommandEvent &event)
{
	if (!m_view)
		return;

	//get current vd
	VolumeData* vd = m_view->m_glview->m_cur_vol;
	FL::ComponentSelector comp_selector(vd);
	comp_selector.Clear();

	m_view->RefreshGL();

	//frame
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetBrushToolDlg())
		vr_frame->GetBrushToolDlg()->UpdateUndoRedo();
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
	case 1:
	default:
		m_use_sel_chk->Show();
		m_generate_btn->Show();
		m_refine_btn->Show();
		m_cluster_btn->Hide();
		m_analyze_btn->Hide();
		m_analyze_sel_btn->Hide();
		break;
	case 2:
		m_use_sel_chk->Hide();
		m_generate_btn->Hide();
		m_refine_btn->Hide();
		m_cluster_btn->Show();
		m_analyze_btn->Hide();
		m_analyze_sel_btn->Hide();
		break;
	case 3:
		m_use_sel_chk->Hide();
		m_generate_btn->Hide();
		m_refine_btn->Hide();
		m_cluster_btn->Hide();
		m_analyze_btn->Show();
		m_analyze_sel_btn->Show();
		break;
	}
	Layout();
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
	VolumeData* vd = m_view->m_glview->m_cur_vol;
	m_comp_analyzer.SetVolume(vd);
	list<VolumeData*> channs;
	if (m_comp_analyzer.GenMultiChannels(channs, color_type, m_consistent))
	{
		VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
		if (vr_frame)
		{
			wxString group_name = "";
			DataGroup* group = 0;
			for (auto i = channs.begin(); i != channs.end(); ++i)
			{
				VolumeData* vd = *i;
				if (vd)
				{
					vr_frame->GetDataManager()->AddVolumeData(vd);
					if (i == channs.begin())
					{
						group_name = m_view->AddGroup("");
						group = m_view->GetGroup(group_name);
					}
					m_view->AddVolumeData(vd, group_name);
				}
			}
			if (group)
			{
				//group->SetSyncRAll(true);
				//group->SetSyncGAll(true);
				//group->SetSyncBAll(true);
				FLIVR::Color col = vd->GetGamma();
				group->SetGammaAll(col);
				col = vd->GetBrightness();
				group->SetBrightnessAll(col);
				col = vd->GetHdr();
				group->SetHdrAll(col);
			}
			vr_frame->UpdateList();
			vr_frame->UpdateTree(vd->GetName());
			m_view->RefreshGL();
		}
	}
}

void ComponentDlg::OutputRgb(int color_type)
{
	if (!m_view)
		return;
	VolumeData* vd = m_view->m_glview->m_cur_vol;
	m_comp_analyzer.SetVolume(vd);
	list<VolumeData*> channs;
	if (m_comp_analyzer.GenRgbChannels(channs, color_type, m_consistent))
	{
		VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
		if (vr_frame)
		{
			wxString group_name = "";
			DataGroup* group = 0;
			for (auto i = channs.begin(); i != channs.end(); ++i)
			{
				VolumeData* vd = *i;
				if (vd)
				{
					vr_frame->GetDataManager()->AddVolumeData(vd);
					if (i == channs.begin())
					{
						group_name = m_view->AddGroup("");
						group = m_view->GetGroup(group_name);
					}
					m_view->AddVolumeData(vd, group_name);
				}
			}
			if (group)
			{
				//group->SetSyncRAll(true);
				//group->SetSyncGAll(true);
				//group->SetSyncBAll(true);
				FLIVR::Color col = vd->GetGamma();
				group->SetGammaAll(col);
				col = vd->GetBrightness();
				group->SetBrightnessAll(col);
				col = vd->GetHdr();
				group->SetHdrAll(col);
			}
			vr_frame->UpdateList();
			vr_frame->UpdateTree(vd->GetName());
			m_view->RefreshGL();
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

void ComponentDlg::OnOutputAnn(wxCommandEvent &event)
{
	if (!m_view)
		return;
	VolumeData* vd = m_view->m_glview->m_cur_vol;
	m_comp_analyzer.SetVolume(vd);
	Annotations* ann = new Annotations();
	if (m_comp_analyzer.GenAnnotations(*ann, m_consistent))
	{
		ann->SetVolume(vd);
		ann->SetTransform(vd->GetTexture()->transform());
		VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
		if (vr_frame)
		{
			DataManager* mgr = vr_frame->GetDataManager();
			if (mgr)
				mgr->AddAnnotations(ann);
			m_view->AddAnnotations(ann);
			vr_frame->UpdateList();
			vr_frame->UpdateTree(vd->GetName());
		}
		m_view->RefreshGL();
	}
}

void ComponentDlg::OnNotebook(wxBookCtrlEvent &event)
{
	EnableGenerate();
}

void ComponentDlg::OnGenerate(wxCommandEvent &event)
{
	int page = m_notebook->GetSelection();
	switch (page)
	{
	case 0:
	default:
		GenerateBsc(false);
		break;
	case 1:
		GenerateAdv(false);
		break;
	}
}

void ComponentDlg::OnRefine(wxCommandEvent &event)
{
	int page = m_notebook->GetSelection();
	switch (page)
	{
	case 0:
	default:
		GenerateBsc(true);
		break;
	case 1:
		GenerateAdv(true);
		break;
	}
}

void ComponentDlg::OnCluster(wxCommandEvent &event)
{
	Cluster();
}

void ComponentDlg::GenerateAdv(bool refine)
{
	if (!m_view)
		return;
	VolumeData* vd = m_view->m_glview->m_cur_vol;
	if (!vd)
		return;
	vd->AddEmptyMask(1);

	int bn = vd->GetAllBrickNum();
	m_prog_bit = 97.0f / float(bn * (1 +
		(m_initial_grow ? 1 : 0) +
		(m_sized_grow ? 1 : 0) +
		(m_cleanup ? 1 : 0) +
		(m_match_slices ?
		(m_bidir_match?2:1):0)));
	m_prog = 0.0f;
	m_generate_prg->SetValue(0);

	FL::ComponentGenerator cg(vd, KernelProgram::get_device_id());
	boost::signals2::connection connection =
		cg.m_sig_progress.connect(boost::bind(
			&ComponentDlg::UpdateProgress, this));

	cg.SetUseMask(m_use_sel_chk->GetValue());

	if (refine)
	{
		if (bn > 1)
			cg.ClearBorders3D();
	}
	else
	{
		vd->AddEmptyLabel();
		cg.ShuffleID_3D();
	}

	double scale = vd->GetScalarScale();
	double scale2 = scale * scale;

	if (m_initial_grow)
		cg.InitialGrow(m_ig_param_transition, m_ig_iterations,
			float(m_ig_translate / scale),
			float(m_ig_translate2 / scale),
			float(m_ig_scalar_falloff / scale2),
			float(m_ig_scalar_falloff2 / scale2),
			float(m_ig_grad_falloff / scale2),
			float(m_ig_grad_falloff2 / scale2),
			float(m_ig_var_falloff / scale2),
			float(m_ig_var_falloff2 / scale2),
			float(m_ig_angle_falloff / scale2),
			float(m_ig_angle_falloff2 / scale2));

	if (m_sized_grow)
		cg.SizedGrow(m_sg_param_transition, m_sg_iterations,
			(unsigned int)(m_sg_size_limiter), (unsigned int)(m_sg_size_limiter2),
			float(m_sg_translate / scale),
			float(m_sg_translate2 / scale),
			float(m_sg_scalar_falloff / scale2),
			float(m_sg_scalar_falloff2 / scale2),
			float(m_sg_grad_falloff / scale2),
			float(m_sg_grad_falloff2 / scale2),
			float(m_sg_var_falloff / scale2),
			float(m_sg_var_falloff2 / scale2),
			float(m_sg_angle_falloff / scale2),
			float(m_sg_angle_falloff2 / scale2));

	if (m_cleanup)
		cg.Cleanup(m_cl_iterations, (unsigned int)(m_cl_size_limiter));

	if (m_match_slices)
	{
		cg.MatchSlices_CPU(false,
			(unsigned int)(m_size_thresh),
			float(m_size_ratio), float(m_dist_thresh),
			float(m_angle_thresh));
		if (m_bidir_match)
			cg.MatchSlices_CPU(true,
				(unsigned int)(m_size_thresh),
				float(m_size_ratio), float(m_dist_thresh),
				float(m_angle_thresh));
	}

	vd->GetVR()->clear_tex_current();
	m_view->RefreshGL();

	m_generate_prg->SetValue(100);
	connection.disconnect();

	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
		vr_frame->GetSettingDlg()->SetRunScript(true);
		vr_frame->GetMovieView()->GetScriptSettings();
	}
}

void ComponentDlg::GenerateBsc(bool refine)
{
	if (!m_view)
		return;
	VolumeData* vd = m_view->m_glview->m_cur_vol;
	if (!vd)
		return;
	vd->AddEmptyMask(1);

	float density = m_basic_density_vl;
	if (!m_basic_density)
		density = 0.0f;
	int clean_iter = m_basic_clean_iter;
	int clean_size = m_basic_clean_size_vl;
	if (!m_basic_clean)
	{
		clean_iter = 0;
		clean_size = 0;
	}

	//get brick number
	int bn = vd->GetAllBrickNum();
	m_prog_bit = 97.0f / float(bn * 3);

	m_prog = 0.0f;
	m_generate_prg->SetValue(0);

	FL::ComponentGenerator cg(vd, KernelProgram::get_device_id());
	boost::signals2::connection connection =
		cg.m_sig_progress.connect(boost::bind(
			&ComponentDlg::UpdateProgress, this));

	cg.SetUseMask(m_use_sel_chk->GetValue());

	if (refine)
	{
		if (bn > 1)
			cg.ClearBorders3D();
	}
	else
	{
		vd->AddEmptyLabel();
		cg.ShuffleID_3D();
	}

	double scale = vd->GetScalarScale();
	double scale2 = scale * scale;

	if (m_basic_size)
		cg.Grow3DSized(m_basic_diff, m_basic_iter,
			float(m_basic_thresh / scale),
			float(m_basic_falloff / scale2),
			m_basic_size_lm, density,
			clean_iter, clean_size);
	else
		cg.Grow3D(m_basic_diff, m_basic_iter,
			float(m_basic_thresh / scale),
			float(m_basic_falloff / scale2),
			density, clean_iter, clean_size);

	if (bn > 1)
		cg.FillBorder3D(0.1);

	vd->GetVR()->clear_tex_current();
	m_view->RefreshGL();

	m_generate_prg->SetValue(100);
	connection.disconnect();

	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
		vr_frame->GetSettingDlg()->SetRunScript(true);
		vr_frame->GetMovieView()->GetScriptSettings();
	}
}

void ComponentDlg::Cluster()
{
	if (!m_view)
		return;
	VolumeData* vd = m_view->m_glview->m_cur_vol;
	if (!vd)
		return;
	Texture* tex = vd->GetTexture();
	if (!tex)
		return;
	Nrrd* nrrd_data = tex->get_nrrd(0);
	if (!nrrd_data)
		return;
	int bits = vd->GetBits();
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
		vd->AddEmptyLabel();
		nrrd_label = tex->get_nrrd(tex->nlabel());
	}
	unsigned int* data_label = (unsigned int*)(nrrd_label->data);
	if (!data_label)
		return;

	int nx, ny, nz;
	vd->GetResolution(nx, ny, nz);
	double scale = vd->GetScalarScale();

	FL::ClusterMethod* method = 0;
	//switch method
	if (m_cluster_method_exmax)
	{
		FL::ClusterExmax* method_exmax = new FL::ClusterExmax();
		method_exmax->SetClnum(m_cluster_clnum);
		method_exmax->SetMaxiter(m_cluster_maxiter);
		method = method_exmax;
	}
	else if (m_cluster_method_dbscan)
	{
		FL::ClusterDbscan* method_dbscan = new FL::ClusterDbscan();
		method_dbscan->SetSize(m_cluster_size);
		method_dbscan->SetEps(m_cluster_eps);
		method = method_dbscan;
	}
	else if (m_cluster_method_kmeans)
	{
		FL::ClusterKmeans* method_kmeans = new FL::ClusterKmeans();
		method_kmeans->SetClnum(m_cluster_clnum);
		method_kmeans->SetMaxiter(m_cluster_maxiter);
		method = method_kmeans;
	}

	if (!method)
		return;

	//add cluster points
	size_t i, j, k;
	size_t index;
	unsigned char mask_value;
	float data_value;
	for (i = 0; i < nx; ++i)
	for (j = 0; j < ny; ++j)
	for (k = 0; k < nz; ++k)
	{
		index = nx*ny*k + nx*j + i;
		mask_value = data_mask[index];
		if (mask_value)
		{
			if (bits == 8)
				data_value = ((unsigned char*)data_data)[index] / 255.0f;
			else if (bits == 16)
				data_value = ((unsigned short*)data_data)[index] * scale / 65535.0f;
			method->AddClusterPoint(
				FLIVR::Point(i, j, k), data_value);
		}
	}

	if (method->Execute())
	{
		method->GenerateNewIDs(0, (void*)data_label, nx, ny, nz, 60);
		vd->GetVR()->clear_tex_pool();
		m_view->RefreshGL();
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
	brick_id = -1;
	return true;
}

void ComponentDlg::GenerateComp(int type, int mode)
{
	switch (type)
	{
	case 0://basic
		{
			if (!m_view)
				return;
			VolumeData* vd = m_view->m_glview->m_cur_vol;
			if (!vd)
				return;
			FL::ComponentGenerator cg(vd, KernelProgram::get_device_id());
			bool use_mask = m_use_sel_chk->GetValue();
			cg.SetUseMask(use_mask);
			if (use_mask)
			{
				if (!vd->GetMask(true))
					return;
			}
			else
				vd->AddEmptyMask(1);

			if (mode == 0)
			{
				vd->AddEmptyLabel();
				cg.ShuffleID_3D();
			}
			else
			{
				if (vd->GetAllBrickNum() > 1)
					cg.ClearBorders3D();
			}

			double scale = vd->GetScalarScale();
			double scale2 = scale * scale;

			float density = m_basic_density_vl;
			if (!m_basic_density)
				density = 0.0f;
			int clean_iter = m_basic_clean_iter;
			int clean_size = m_basic_clean_size_vl;
			if (!m_basic_clean)
			{
				clean_iter = 0;
				clean_size = 0;
			}

			if (m_basic_size)
				cg.Grow3DSized(m_basic_diff, m_basic_iter,
					float(m_basic_thresh / scale),
					float(m_basic_falloff / scale2),
					m_basic_size_lm, density,
					clean_iter, clean_size);
			else
				cg.Grow3D(m_basic_diff, m_basic_iter,
					float(m_basic_thresh / scale),
					float(m_basic_falloff / scale2),
					density, clean_iter, clean_size);

			vd->GetVR()->clear_tex_current();
			m_view->RefreshGL();
		}
		break;
	}
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
		VolumeData* vd = m_view->m_glview->m_cur_vol;
		FL::ComponentSelector comp_selector(vd);
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

	m_view->RefreshGL();

	//frame
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetBrushToolDlg())
		vr_frame->GetBrushToolDlg()->UpdateUndoRedo();
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
	VolumeData* vd = m_view->m_glview->m_cur_vol;
	if (!vd)
		return;

	int bn = vd->GetAllBrickNum();
	m_prog_bit = 97.0f / float(bn * 2 + (m_consistent?1:0));
	m_prog = 0.0f;
	m_generate_prg->SetValue(0);

	boost::signals2::connection connection =
		m_comp_analyzer.m_sig_progress.connect(boost::bind(
		&ComponentDlg::UpdateProgress, this));

	m_comp_analyzer.SetVolume(vd);
	if (m_colocal)
	{
		m_comp_analyzer.ClearCoVolumes();
		for (int i = 0; i < m_view->GetDispVolumeNum(); ++i)
		{
			VolumeData* vdi = m_view->GetDispVolumeData(i);
			if (vdi != vd)
				m_comp_analyzer.AddCoVolume(vdi);
		}
	}
	m_comp_analyzer.Analyze(sel, m_consistent, m_colocal);

	if (m_consistent)
	{
		//invalidate label mask in gpu
		vd->GetVR()->clear_tex_pool();
		m_view->RefreshGL();
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
		string str;
		m_comp_analyzer.OutputCompListStr(str, 1);
		m_stat_text->SetValue(str);
	}

	m_generate_prg->SetValue(100);
	connection.disconnect();
}

