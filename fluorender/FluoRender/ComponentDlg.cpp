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

wxWindow* ComponentDlg::Create3DAnalysisPage(wxWindow *parent)
{
	wxPanel *page = new wxPanel(parent);
	wxStaticText *st = 0;

	return page;
}

wxWindow* ComponentDlg::Create2DAnalysisPage(wxWindow *parent)
{
	wxPanel *page = new wxPanel(parent);
	wxStaticText *st = 0;

	wxBoxSizer* sizer1 = new wxBoxSizer(wxVERTICAL);
	sizer1->Add(CreateInitialGrowPane(page), 0, wxGROW | wxALL, 5);

	wxBoxSizer* sizerv = new wxBoxSizer(wxVERTICAL);
	sizerv->Add(10, 10);
	sizerv->Add(sizer1, 0, wxEXPAND);
	sizerv->Add(10, 10);

	page->SetSizer(sizerv);

	return page;
}

wxCollapsiblePane* ComponentDlg::CreateInitialGrowPane(wxWindow *parent)
{
	wxCollapsiblePane* collpane = new wxCollapsiblePane(parent, wxID_ANY, "Initial Grow");
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
	m_param_transition_check = new wxCheckBox(pane, ID_ParamTransitionCheck, "Parameter Transition",
		wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	sizer1->Add(5, 5);
	sizer1->Add(m_initial_grow_check, 0, wxALIGN_CENTER);
	sizer1->Add(5, 5);
	sizer1->Add(m_param_transition_check, 0, wxALIGN_CENTER);
	//iterations
	wxBoxSizer *sizer2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(pane, 0, "Iterations:",
		wxDefaultPosition, wxSize(100, 23));
	m_iterations_sldr = new wxSlider(pane, ID_IterationsSldr, 0, 0, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_iterations_text = new wxTextCtrl(pane, ID_IterationsText, "0",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	sizer2->Add(5, 5);
	sizer2->Add(st, 0, wxALIGN_CENTER);
	sizer2->Add(m_iterations_sldr, 1, wxEXPAND);
	sizer2->Add(m_iterations_text, 0, wxALIGN_CENTER);
	sizer2->Add(5, 5);
	//translate
	wxBoxSizer *sizer3 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(pane, 0, "Translation:",
		wxDefaultPosition, wxSize(100, 23));
	m_translate_sldr = new wxSlider(pane, ID_TranslateSldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_translate_text = new wxTextCtrl(pane, ID_TranslateText, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	m_translate2_sldr = new wxSlider(pane, ID_Translate2Sldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_translate2_text = new wxTextCtrl(pane, ID_Translate2Text, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	sizer3->Add(5, 5);
	sizer3->Add(st, 0, wxALIGN_CENTER);
	sizer3->Add(m_translate_sldr, 1, wxEXPAND);
	sizer3->Add(m_translate_text, 0, wxALIGN_CENTER);
	sizer3->Add(5, 5);
	sizer3->Add(m_translate2_sldr, 1, wxEXPAND);
	sizer3->Add(m_translate2_text, 0, wxALIGN_CENTER);
	sizer3->Add(5, 5);
	//scalar falloff
	wxBoxSizer *sizer4 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(pane, 0, "Scalar Falloff:",
		wxDefaultPosition, wxSize(100, 23));
	m_scalar_falloff_sldr = new wxSlider(pane, ID_ScalarFalloffSldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_scalar_falloff_text = new wxTextCtrl(pane, ID_ScalarFalloffText, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	m_scalar_falloff2_sldr = new wxSlider(pane, ID_ScalarFalloff2Sldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_scalar_falloff2_text = new wxTextCtrl(pane, ID_ScalarFalloff2Text, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	sizer4->Add(5, 5);
	sizer4->Add(st, 0, wxALIGN_CENTER);
	sizer4->Add(m_scalar_falloff_sldr, 1, wxEXPAND);
	sizer4->Add(m_scalar_falloff_text, 0, wxALIGN_CENTER);
	sizer4->Add(5, 5);
	sizer4->Add(m_scalar_falloff2_sldr, 1, wxEXPAND);
	sizer4->Add(m_scalar_falloff2_text, 0, wxALIGN_CENTER);
	sizer4->Add(5, 5);
	//grad falloff
	wxBoxSizer *sizer5 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(pane, 0, "Grad Falloff:",
		wxDefaultPosition, wxSize(100, 23));
	m_grad_falloff_sldr = new wxSlider(pane, ID_GradFalloffSldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_grad_falloff_text = new wxTextCtrl(pane, ID_GradFalloffText, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	m_grad_falloff2_sldr = new wxSlider(pane, ID_GradFalloff2Sldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_grad_falloff2_text = new wxTextCtrl(pane, ID_GradFalloff2Text, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	sizer5->Add(5, 5);
	sizer5->Add(st, 0, wxALIGN_CENTER);
	sizer5->Add(m_grad_falloff_sldr, 1, wxEXPAND);
	sizer5->Add(m_grad_falloff_text, 0, wxALIGN_CENTER);
	sizer5->Add(5, 5);
	sizer5->Add(m_grad_falloff2_sldr, 1, wxEXPAND);
	sizer5->Add(m_grad_falloff2_text, 0, wxALIGN_CENTER);
	sizer5->Add(5, 5);
	//variance falloff
	wxBoxSizer *sizer6 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(pane, 0, "Var Falloff:",
		wxDefaultPosition, wxSize(100, 23));
	m_var_falloff_sldr = new wxSlider(pane, ID_VarFalloffSldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_var_falloff_text = new wxTextCtrl(pane, ID_VarFalloffText, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	m_var_falloff2_sldr = new wxSlider(pane, ID_VarFalloff2Sldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_var_falloff2_text = new wxTextCtrl(pane, ID_VarFalloff2Text, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	sizer6->Add(5, 5);
	sizer6->Add(st, 0, wxALIGN_CENTER);
	sizer6->Add(m_var_falloff_sldr, 1, wxEXPAND);
	sizer6->Add(m_var_falloff_text, 0, wxALIGN_CENTER);
	sizer6->Add(5, 5);
	sizer6->Add(m_var_falloff2_sldr, 1, wxEXPAND);
	sizer6->Add(m_var_falloff2_text, 0, wxALIGN_CENTER);
	sizer6->Add(5, 5);
	//angle falloff
	wxBoxSizer *sizer7 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(pane, 0, "Angle Falloff:",
		wxDefaultPosition, wxSize(100, 23));
	m_angle_falloff_sldr = new wxSlider(pane, ID_AngleFalloffSldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_angle_falloff_text = new wxTextCtrl(pane, ID_AngleFalloffText, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	m_angle_falloff2_sldr = new wxSlider(pane, ID_AngleFalloff2Sldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_angle_falloff2_text = new wxTextCtrl(pane, ID_AngleFalloff2Text, "0.000",
		wxDefaultPosition, wxSize(60, 20), 0, vald_fp3);
	sizer7->Add(5, 5);
	sizer7->Add(st, 0, wxALIGN_CENTER);
	sizer7->Add(m_angle_falloff_sldr, 1, wxEXPAND);
	sizer7->Add(m_angle_falloff_text, 0, wxALIGN_CENTER);
	sizer7->Add(5, 5);
	sizer7->Add(m_angle_falloff2_sldr, 1, wxEXPAND);
	sizer7->Add(m_angle_falloff2_text, 0, wxALIGN_CENTER);
	sizer7->Add(5, 5);

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
	wxCollapsiblePane* collpane = new wxCollapsiblePane(parent, wxID_ANY, "Initial Grow");

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
		wxPoint(500, 150),
		wxSize(400, 550),
		0, "ComponentDlg"),
	m_frame(parent),
	m_view(0)
{
	//notebook
	m_notebook = new wxNotebook(this, wxID_ANY);
	m_notebook->AddPage(Create3DAnalysisPage(m_notebook), "Basic");
	m_notebook->AddPage(Create2DAnalysisPage(m_notebook), "Advanced");

	//all controls
	wxBoxSizer *sizerv = new wxBoxSizer(wxVERTICAL);
	sizerv->Add(10, 10);
	sizerv->Add(m_notebook, 0, wxEXPAND);
	sizerv->Add(10, 10);

	SetSizerAndFit(sizerv);
	Layout();
}

ComponentDlg::~ComponentDlg()
{

}