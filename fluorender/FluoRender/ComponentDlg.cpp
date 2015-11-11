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
#include <wx/valnum.h>

BEGIN_EVENT_TABLE(ComponentDlg, wxPanel)
	EVT_COLLAPSIBLEPANE_CHANGED(wxID_ANY, ComponentDlg::OnPaneChange)
	//initial grow
	EVT_CHECKBOX(ID_InitialGrowCheck, ComponentDlg::OnInitialGrowCheck)
	EVT_CHECKBOX(ID_IGParamTransitionCheck, ComponentDlg::OnIGParamTransitionCheck)
	//sized grow
	EVT_CHECKBOX(ID_SizedGrowCheck, ComponentDlg::OnSizedGrowCheck)
	EVT_CHECKBOX(ID_SGParamTransitionCheck, ComponentDlg::OnSGParamTransitionCheck)
END_EVENT_TABLE()

wxWindow* ComponentDlg::Create3DAnalysisPage(wxWindow *parent)
{
	wxPanel *page = new wxPanel(parent);
	wxStaticText *st = 0;

	return page;
}

wxWindow* ComponentDlg::Create2DAnalysisPage(wxWindow *parent)
{
	m_adv_page = new wxScrolledWindow(parent);
	wxStaticText *st = 0;

	//initial grow
	wxBoxSizer* sizer1 = new wxBoxSizer(wxVERTICAL);
	m_initial_grow_pane = CreateInitialGrowPane(m_adv_page);
	sizer1->Add(m_initial_grow_pane, 0, wxEXPAND);
	//sized grow
	wxBoxSizer* sizer2 = new wxBoxSizer(wxVERTICAL);
	m_sized_grow_pane = CreateSizedGrowPane(m_adv_page);
	sizer2->Add(m_sized_grow_pane, 0, wxEXPAND);

	wxBoxSizer* sizerv = new wxBoxSizer(wxVERTICAL);
	sizerv->Add(10, 10);
	sizerv->Add(sizer1, 0, wxEXPAND);
	sizerv->Add(10, 10);
	sizerv->Add(sizer2, 0, wxEXPAND);
	sizerv->Add(10, 10);

	m_adv_page->SetScrollRate(10, 10);
	m_adv_page->SetSizer(sizerv);

	return m_adv_page;
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
	m_ig_translate2_sldr = new wxSlider(pane, ID_IGTranslate2Sldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_ig_translate2_text = new wxTextCtrl(pane, ID_IGTranslate2Text, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	sizer3->Add(5, 5);
	sizer3->Add(st, 0, wxALIGN_CENTER);
	sizer3->Add(m_ig_translate_sldr, 1, wxEXPAND);
	sizer3->Add(m_ig_translate_text, 0, wxALIGN_CENTER);
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
	m_ig_scalar_falloff2_sldr = new wxSlider(pane, ID_IGScalarFalloff2Sldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_ig_scalar_falloff2_text = new wxTextCtrl(pane, ID_IGScalarFalloff2Text, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	sizer4->Add(5, 5);
	sizer4->Add(st, 0, wxALIGN_CENTER);
	sizer4->Add(m_ig_scalar_falloff_sldr, 1, wxEXPAND);
	sizer4->Add(m_ig_scalar_falloff_text, 0, wxALIGN_CENTER);
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
	m_ig_grad_falloff2_sldr = new wxSlider(pane, ID_IGGradFalloff2Sldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_ig_grad_falloff2_text = new wxTextCtrl(pane, ID_IGGradFalloff2Text, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	sizer5->Add(5, 5);
	sizer5->Add(st, 0, wxALIGN_CENTER);
	sizer5->Add(m_ig_grad_falloff_sldr, 1, wxEXPAND);
	sizer5->Add(m_ig_grad_falloff_text, 0, wxALIGN_CENTER);
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
	m_ig_var_falloff2_sldr = new wxSlider(pane, ID_IGVarFalloff2Sldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_ig_var_falloff2_text = new wxTextCtrl(pane, ID_IGVarFalloff2Text, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	sizer6->Add(5, 5);
	sizer6->Add(st, 0, wxALIGN_CENTER);
	sizer6->Add(m_ig_var_falloff_sldr, 1, wxEXPAND);
	sizer6->Add(m_ig_var_falloff_text, 0, wxALIGN_CENTER);
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
	m_ig_angle_falloff2_sldr = new wxSlider(pane, ID_IGAngleFalloff2Sldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_ig_angle_falloff2_text = new wxTextCtrl(pane, ID_IGAngleFalloff2Text, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	sizer7->Add(5, 5);
	sizer7->Add(st, 0, wxALIGN_CENTER);
	sizer7->Add(m_ig_angle_falloff_sldr, 1, wxEXPAND);
	sizer7->Add(m_ig_angle_falloff_text, 0, wxALIGN_CENTER);
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
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	m_sg_size_limiter2_sldr = new wxSlider(pane, ID_SGSizeLimiter2Sldr, 0, 0, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_sg_size_limiter2_text = new wxTextCtrl(pane, ID_SGSizeLimiter2Text, "0",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	sizer3->Add(5, 5);
	sizer3->Add(st, 0, wxALIGN_CENTER);
	sizer3->Add(m_sg_size_limiter_sldr, 1, wxEXPAND);
	sizer3->Add(m_sg_size_limiter_text, 0, wxALIGN_CENTER);
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
	m_sg_translate2_sldr = new wxSlider(pane, ID_SGTranslate2Sldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_sg_translate2_text = new wxTextCtrl(pane, ID_SGTranslate2Text, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	sizer4->Add(5, 5);
	sizer4->Add(st, 0, wxALIGN_CENTER);
	sizer4->Add(m_sg_translate_sldr, 1, wxEXPAND);
	sizer4->Add(m_sg_translate_text, 0, wxALIGN_CENTER);
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
	m_sg_scalar_falloff2_sldr = new wxSlider(pane, ID_SGScalarFalloff2Sldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_sg_scalar_falloff2_text = new wxTextCtrl(pane, ID_SGScalarFalloff2Text, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	sizer5->Add(5, 5);
	sizer5->Add(st, 0, wxALIGN_CENTER);
	sizer5->Add(m_sg_scalar_falloff_sldr, 1, wxEXPAND);
	sizer5->Add(m_sg_scalar_falloff_text, 0, wxALIGN_CENTER);
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
	m_sg_grad_falloff2_sldr = new wxSlider(pane, ID_SGGradFalloff2Sldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_sg_grad_falloff2_text = new wxTextCtrl(pane, ID_SGGradFalloff2Text, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	sizer6->Add(5, 5);
	sizer6->Add(st, 0, wxALIGN_CENTER);
	sizer6->Add(m_sg_grad_falloff_sldr, 1, wxEXPAND);
	sizer6->Add(m_sg_grad_falloff_text, 0, wxALIGN_CENTER);
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
	m_sg_var_falloff2_sldr = new wxSlider(pane, ID_SGVarFalloff2Sldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_sg_var_falloff2_text = new wxTextCtrl(pane, ID_SGVarFalloff2Text, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	sizer7->Add(5, 5);
	sizer7->Add(st, 0, wxALIGN_CENTER);
	sizer7->Add(m_sg_var_falloff_sldr, 1, wxEXPAND);
	sizer7->Add(m_sg_var_falloff_text, 0, wxALIGN_CENTER);
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
	m_sg_angle_falloff2_sldr = new wxSlider(pane, ID_SGAngleFalloff2Sldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_sg_angle_falloff2_text = new wxTextCtrl(pane, ID_SGAngleFalloff2Text, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	sizer8->Add(5, 5);
	sizer8->Add(st, 0, wxALIGN_CENTER);
	sizer8->Add(m_sg_angle_falloff_sldr, 1, wxEXPAND);
	sizer8->Add(m_sg_angle_falloff_text, 0, wxALIGN_CENTER);
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
	wxCollapsiblePane* collpane = new wxCollapsiblePane(parent, wxID_ANY, "Initial Grow");

	return collpane;
}

wxCollapsiblePane* ComponentDlg::CreateMatchSlicesPane(wxWindow *parent)
{
	wxCollapsiblePane* collpane = new wxCollapsiblePane(parent, wxID_ANY, "Initial Grow");

	return collpane;
}

ComponentDlg::ComponentDlg(wxWindow *frame, wxWindow *parent)
	: wxPanel(parent, wxID_ANY,
		wxDefaultPosition,
		wxSize(450, 500),
		0, "ComponentDlg"),
	m_frame(parent),
	m_view(0)
{
	//notebook
	m_notebook = new wxNotebook(this, wxID_ANY);
	m_notebook->AddPage(Create3DAnalysisPage(m_notebook), "Basic");
	m_notebook->AddPage(Create2DAnalysisPage(m_notebook), "Advanced");

	//all controls
	wxBoxSizer* sizerv = new wxBoxSizer(wxVERTICAL);
	sizerv->Add(10, 10);
	sizerv->Add(m_notebook, 1, wxEXPAND);
	sizerv->Add(10, 10);

	SetSizer(sizerv);
	Layout();

	GetSettings();
}

ComponentDlg::~ComponentDlg()
{
}

void ComponentDlg::GetSettings()
{
	//defaults
	//initial grow
	m_initial_grow = false;
	m_ig_param_transition = false;

	//sized grow
	m_sized_grow = false;
	m_sg_param_transition = false;

	//read values

	//update ui
	//initial grow
	EnableInitialGrow(m_initial_grow);
	if (m_initial_grow)
		m_initial_grow_pane->Expand();
	else
		m_initial_grow_pane->Collapse();
	EnableIGParamTransition(m_ig_param_transition);

	//sized grow
	EnableSizedGrow(m_sized_grow);
	if (m_sized_grow)
		m_sized_grow_pane->Expand();
	else
		m_sized_grow_pane->Collapse();
	EnableSGParamTransition(m_sg_param_transition);
}

void ComponentDlg::OnPaneChange(wxCollapsiblePaneEvent& event)
{
	if (m_adv_page)
		m_adv_page->SendSizeEvent();
}

void ComponentDlg::EnableInitialGrow(bool value)
{
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
		m_ig_translate2_sldr->Show();
		m_ig_translate2_text->Show();
		m_ig_scalar_falloff2_sldr->Show();
		m_ig_scalar_falloff2_text->Show();
		m_ig_grad_falloff2_sldr->Show();
		m_ig_grad_falloff2_text->Show();
		m_ig_var_falloff2_sldr->Show();
		m_ig_var_falloff2_text->Show();
		m_ig_angle_falloff2_sldr->Show();
		m_ig_angle_falloff2_text->Show();
	}
	else
	{
		m_ig_translate2_sldr->Hide();
		m_ig_translate2_text->Hide();
		m_ig_scalar_falloff2_sldr->Hide();
		m_ig_scalar_falloff2_text->Hide();
		m_ig_grad_falloff2_sldr->Hide();
		m_ig_grad_falloff2_text->Hide();
		m_ig_var_falloff2_sldr->Hide();
		m_ig_var_falloff2_text->Hide();
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
}

//sized grow
void ComponentDlg::EnableSizedGrow(bool value)
{
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
		m_sg_size_limiter2_sldr->Show();
		m_sg_size_limiter2_text->Show();
		m_sg_translate2_sldr->Show();
		m_sg_translate2_text->Show();
		m_sg_scalar_falloff2_sldr->Show();
		m_sg_scalar_falloff2_text->Show();
		m_sg_grad_falloff2_sldr->Show();
		m_sg_grad_falloff2_text->Show();
		m_sg_var_falloff2_sldr->Show();
		m_sg_var_falloff2_text->Show();
		m_sg_angle_falloff2_sldr->Show();
		m_sg_angle_falloff2_text->Show();
	}
	else
	{
		m_sg_size_limiter2_sldr->Hide();
		m_sg_size_limiter2_text->Hide();
		m_sg_translate2_sldr->Hide();
		m_sg_translate2_text->Hide();
		m_sg_scalar_falloff2_sldr->Hide();
		m_sg_scalar_falloff2_text->Hide();
		m_sg_grad_falloff2_sldr->Hide();
		m_sg_grad_falloff2_text->Hide();
		m_sg_var_falloff2_sldr->Hide();
		m_sg_var_falloff2_text->Hide();
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
}