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
#include "DataManager.h"
#include <wx/wx.h>
#include <wx/collpane.h>
#include <wx/notebook.h>

#ifndef _COMPONENTDLG_H_
#define _COMPONENTDLG_H_

class VRenderView;
class VolumeData;

class ComponentDlg : public wxPanel
{
public:
	enum
	{
		//initial grow pane
		ID_InitialGrowPane,
		ID_InitialGrowCheck,
		ID_ParamTransitionCheck,
		ID_IterationsSldr,
		ID_IterationsText,
		//translate
		ID_TranslateSldr,
		ID_TranslateText,
		ID_Translate2Sldr,
		ID_Translate2Text,
		//scalar falloff
		ID_ScalarFalloffSldr,
		ID_ScalarFalloffText,
		ID_ScalarFalloff2Sldr,
		ID_ScalarFalloff2Text,
		//grad falloff
		ID_GradFalloffSldr,
		ID_GradFalloffText,
		ID_GradFalloff2Sldr,
		ID_GradFalloff2Text,
		//variance falloff
		ID_VarFalloffSldr,
		ID_VarFalloffText,
		ID_VarFalloff2Sldr,
		ID_VarFalloff2Text,
		//angle falloff
		ID_AngleFalloffSldr,
		ID_AngleFalloffText,
		ID_AngleFalloff2Sldr,
		ID_AngleFalloff2Text
	};

	ComponentDlg(wxWindow* frame,
		wxWindow* parent);
	~ComponentDlg();

	void GetSettings();
	VRenderView* GetView();

private:
	wxWindow* m_frame;
	VRenderView* m_view;

	//sizer
	wxBoxSizer *m_sizerv;
	//tab control
	wxNotebook *m_notebook;

	//initial grow pane
	wxCheckBox* m_initial_grow_check;
	wxCheckBox* m_param_transition_check;
	wxSlider* m_iterations_sldr;
	wxTextCtrl* m_iterations_text;
	//translate
	wxSlider* m_translate_sldr;
	wxTextCtrl* m_translate_text;
	wxSlider* m_translate2_sldr;
	wxTextCtrl* m_translate2_text;
	//scalar falloff
	wxSlider* m_scalar_falloff_sldr;
	wxTextCtrl* m_scalar_falloff_text;
	wxSlider* m_scalar_falloff2_sldr;
	wxTextCtrl* m_scalar_falloff2_text;
	//grad falloff
	wxSlider* m_grad_falloff_sldr;
	wxTextCtrl* m_grad_falloff_text;
	wxSlider* m_grad_falloff2_sldr;
	wxTextCtrl* m_grad_falloff2_text;
	//variance falloff
	wxSlider* m_var_falloff_sldr;
	wxTextCtrl* m_var_falloff_text;
	wxSlider* m_var_falloff2_sldr;
	wxTextCtrl* m_var_falloff2_text;
	//angle falloff
	wxSlider* m_angle_falloff_sldr;
	wxTextCtrl* m_angle_falloff_text;
	wxSlider* m_angle_falloff2_sldr;
	wxTextCtrl* m_angle_falloff2_text;

private:
	wxWindow* Create3DAnalysisPage(wxWindow *parent);
	wxWindow* Create2DAnalysisPage(wxWindow *parent);
	wxCollapsiblePane* CreateInitialGrowPane(wxWindow *parent);
	wxCollapsiblePane* CreateSizedGrowPane(wxWindow *parent);
	wxCollapsiblePane* CreateCleanupPane(wxWindow *parent);
	wxCollapsiblePane* CreateMatchSlicesPane(wxWindow *parent);

	//initial grow pane
	void OnInitialGrowPane(wxCollapsiblePaneEvent& event);

	DECLARE_EVENT_TABLE();
};

#endif//_COMPONENTDLG_H_