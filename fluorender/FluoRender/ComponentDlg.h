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
		ID_IGParamTransitionCheck,
		ID_IGIterationsSldr,
		ID_IGIterationsText,
		//translate
		ID_IGTranslateSldr,
		ID_IGTranslateText,
		ID_IGTranslate2Sldr,
		ID_IGTranslate2Text,
		//scalar falloff
		ID_IGScalarFalloffSldr,
		ID_IGScalarFalloffText,
		ID_IGScalarFalloff2Sldr,
		ID_IGScalarFalloff2Text,
		//grad falloff
		ID_IGGradFalloffSldr,
		ID_IGGradFalloffText,
		ID_IGGradFalloff2Sldr,
		ID_IGGradFalloff2Text,
		//variance falloff
		ID_IGVarFalloffSldr,
		ID_IGVarFalloffText,
		ID_IGVarFalloff2Sldr,
		ID_IGVarFalloff2Text,
		//angle falloff
		ID_IGAngleFalloffSldr,
		ID_IGAngleFalloffText,
		ID_IGAngleFalloff2Sldr,
		ID_IGAngleFalloff2Text,
		
		//sized grow pane
		ID_SizedGrowPane,
		ID_SizedGrowCheck,
		ID_SGParamTransitionCheck,
		ID_SGIterationsSldr,
		ID_SGIterationsText,
		//size limiter
		ID_SGSizeLimiterSldr,
		ID_SGSizeLimiterText,
		ID_SGSizeLimiter2Sldr,
		ID_SGSizeLimiter2Text,
		//translate
		ID_SGTranslateSldr,
		ID_SGTranslateText,
		ID_SGTranslate2Sldr,
		ID_SGTranslate2Text,
		//scalar falloff
		ID_SGScalarFalloffSldr,
		ID_SGScalarFalloffText,
		ID_SGScalarFalloff2Sldr,
		ID_SGScalarFalloff2Text,
		//grad falloff
		ID_SGGradFalloffSldr,
		ID_SGGradFalloffText,
		ID_SGGradFalloff2Sldr,
		ID_SGGradFalloff2Text,
		//variance falloff
		ID_SGVarFalloffSldr,
		ID_SGVarFalloffText,
		ID_SGVarFalloff2Sldr,
		ID_SGVarFalloff2Text,
		//angle falloff
		ID_SGAngleFalloffSldr,
		ID_SGAngleFalloffText,
		ID_SGAngleFalloff2Sldr,
		ID_SGAngleFalloff2Text
	};

	ComponentDlg(wxWindow* frame,
		wxWindow* parent);
	~ComponentDlg();

	void GetSettings();
	VRenderView* GetView();

private:
	wxWindow* m_frame;
	VRenderView* m_view;

	//initial grow
	bool m_initial_grow;
	bool m_ig_param_transition;

	//sized grow
	bool m_sized_grow;
	bool m_sg_param_transition;

	//tab control
	wxNotebook *m_notebook;
	wxScrolledWindow* m_adv_page;

	//initial grow pane
	wxCollapsiblePane* m_initial_grow_pane;
	wxCheckBox* m_initial_grow_check;
	wxCheckBox* m_ig_param_transition_check;
	wxSlider* m_ig_iterations_sldr;
	wxTextCtrl* m_ig_iterations_text;
	//translate
	wxSlider* m_ig_translate_sldr;
	wxTextCtrl* m_ig_translate_text;
	wxSlider* m_ig_translate2_sldr;
	wxTextCtrl* m_ig_translate2_text;
	//scalar falloff
	wxSlider* m_ig_scalar_falloff_sldr;
	wxTextCtrl* m_ig_scalar_falloff_text;
	wxSlider* m_ig_scalar_falloff2_sldr;
	wxTextCtrl* m_ig_scalar_falloff2_text;
	//grad falloff
	wxSlider* m_ig_grad_falloff_sldr;
	wxTextCtrl* m_ig_grad_falloff_text;
	wxSlider* m_ig_grad_falloff2_sldr;
	wxTextCtrl* m_ig_grad_falloff2_text;
	//variance falloff
	wxSlider* m_ig_var_falloff_sldr;
	wxTextCtrl* m_ig_var_falloff_text;
	wxSlider* m_ig_var_falloff2_sldr;
	wxTextCtrl* m_ig_var_falloff2_text;
	//angle falloff
	wxSlider* m_ig_angle_falloff_sldr;
	wxTextCtrl* m_ig_angle_falloff_text;
	wxSlider* m_ig_angle_falloff2_sldr;
	wxTextCtrl* m_ig_angle_falloff2_text;

	//sized grow pane
	wxCollapsiblePane* m_sized_grow_pane;
	wxCheckBox* m_sized_grow_check;
	wxCheckBox* m_sg_param_transition_check;
	wxSlider* m_sg_iterations_sldr;
	wxTextCtrl* m_sg_iterations_text;
	//size limiter
	wxSlider* m_sg_size_limiter_sldr;
	wxTextCtrl* m_sg_size_limiter_text;
	wxSlider* m_sg_size_limiter2_sldr;
	wxTextCtrl* m_sg_size_limiter2_text;
	//translate
	wxSlider* m_sg_translate_sldr;
	wxTextCtrl* m_sg_translate_text;
	wxSlider* m_sg_translate2_sldr;
	wxTextCtrl* m_sg_translate2_text;
	//scalar falloff
	wxSlider* m_sg_scalar_falloff_sldr;
	wxTextCtrl* m_sg_scalar_falloff_text;
	wxSlider* m_sg_scalar_falloff2_sldr;
	wxTextCtrl* m_sg_scalar_falloff2_text;
	//grad falloff
	wxSlider* m_sg_grad_falloff_sldr;
	wxTextCtrl* m_sg_grad_falloff_text;
	wxSlider* m_sg_grad_falloff2_sldr;
	wxTextCtrl* m_sg_grad_falloff2_text;
	//variance falloff
	wxSlider* m_sg_var_falloff_sldr;
	wxTextCtrl* m_sg_var_falloff_text;
	wxSlider* m_sg_var_falloff2_sldr;
	wxTextCtrl* m_sg_var_falloff2_text;
	//angle falloff
	wxSlider* m_sg_angle_falloff_sldr;
	wxTextCtrl* m_sg_angle_falloff_text;
	wxSlider* m_sg_angle_falloff2_sldr;
	wxTextCtrl* m_sg_angle_falloff2_text;

private:
	wxWindow* Create3DAnalysisPage(wxWindow *parent);
	wxWindow* Create2DAnalysisPage(wxWindow *parent);
	void OnPaneChange(wxCollapsiblePaneEvent& event);
	wxCollapsiblePane* CreateInitialGrowPane(wxWindow *parent);
	wxCollapsiblePane* CreateSizedGrowPane(wxWindow *parent);
	wxCollapsiblePane* CreateCleanupPane(wxWindow *parent);
	wxCollapsiblePane* CreateMatchSlicesPane(wxWindow *parent);

	//initial grow pane
	void EnableInitialGrow(bool value);
	void EnableIGParamTransition(bool value);
	void OnInitialGrowCheck(wxCommandEvent &event);
	void OnIGParamTransitionCheck(wxCommandEvent &event);

	//sized grow pane
	void EnableSizedGrow(bool value);
	void EnableSGParamTransition(bool value);
	void OnSizedGrowCheck(wxCommandEvent &event);
	void OnSGParamTransitionCheck(wxCommandEvent &event);

	DECLARE_EVENT_TABLE();
};

#endif//_COMPONENTDLG_H_