/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2018 Scientific Computing and Imaging Institute,
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
#include <wx/wx.h>

#ifndef _BRUSHTOOLDLG_H_
#define _BRUSHTOOLDLG_H_

#define BRUSH_TOOL_ITER_WEAK	10
#define BRUSH_TOOL_ITER_NORMAL	30
#define BRUSH_TOOL_ITER_STRONG	60

class VRenderView;
class BrushToolDlg : public wxPanel
{
public:
	enum
	{
		//group1
		//toolbar
		ID_BrushAppend = ID_BRUSH_TOOL,
		ID_BrushDesel,
		ID_BrushDiffuse,
		ID_BrushErase,
		ID_BrushClear,
		ID_BrushCreate,
		ID_BrushSolid,
		ID_BrushUndo,
		ID_BrushRedo,
		//selection strength
		//falloff
		ID_BrushSclTranslateSldr,
		ID_BrushSclTranslateText,
		//edge detect strength
		ID_BrushGmFalloffSldr,
		ID_BrushGmFalloffText,
		//2dinfl
		ID_Brush2dinflSldr,
		ID_Brush2dinflText,
		//edge detect
		ID_BrushEdgeDetectChk,
		//hidden removal
		ID_BrushHiddenRemovalChk,
		//select group
		ID_BrushSelectGroupChk,
		//estimate threshold
		ID_EstimateThreshChk,
		//brush properties
		//brush size 1
		ID_BrushSize1Sldr,
		ID_BrushSize1Text,
		//brush size 2
		ID_BrushSize2Chk,
		ID_BrushSize2Sldr,
		ID_BrushSize2Text,
		//iterations
		ID_BrushIterWRd,
		ID_BrushIterSRd,
		ID_BrushIterSSRd,
		//size relation
		ID_BrushSizeDataRd,
		ID_BrushSizeScreenRd
	};

	BrushToolDlg(wxWindow* frame,
		wxWindow* parent);
	~BrushToolDlg();

	void GetSettings(VRenderView* vrv);

	//set the brush icon down
	void SelectBrush(int id);
	//update undo status
	void UpdateUndoRedo();

private:
	wxWindow* m_frame;

	//current view
	VRenderView *m_cur_view;

	//max volume value
	double m_max_value;
	//default brush properties
	double m_dft_gm_falloff;
	double m_dft_scl_translate;

	//paint tools
	//toolbar
	wxToolBar *m_toolbar;

	//stop at boundary
	wxCheckBox* m_edge_detect_chk;
	//hidden removal
	wxCheckBox* m_hidden_removal_chk;
	//group selection
	wxCheckBox* m_select_group_chk;
	//estimate threshold
	wxCheckBox* m_estimate_thresh_chk;
	//selection strength
	//translate
	wxSlider* m_brush_scl_translate_sldr;
	wxTextCtrl* m_brush_scl_translate_text;
	//gm falloff
	wxSlider* m_brush_gm_falloff_sldr;
	wxTextCtrl* m_brush_gm_falloff_text;
	//2d influence
	wxSlider* m_brush_2dinfl_sldr;
	wxTextCtrl* m_brush_2dinfl_text;
	//brush properties
	//size 1
	wxSlider* m_brush_size1_sldr;
	wxTextCtrl *m_brush_size1_text;
	//size 2
	wxCheckBox* m_brush_size2_chk;
	wxSlider* m_brush_size2_sldr;
	wxTextCtrl* m_brush_size2_text;
	//growth
	wxRadioButton* m_brush_iterw_rb;
	wxRadioButton* m_brush_iters_rb;
	wxRadioButton* m_brush_iterss_rb;
	//size relation
	wxRadioButton* m_brush_size_data_rb;
	wxRadioButton* m_brush_size_screen_rb;

private:
	//event handling
	//paint tools
	//brush commands
	void OnBrushUndo(wxCommandEvent& event);
	void OnBrushRedo(wxCommandEvent& event);
	void OnBrushAppend(wxCommandEvent& event);
	void OnBrushDesel(wxCommandEvent& event);
	void OnBrushDiffuse(wxCommandEvent& event);
	void OnBrushErase(wxCommandEvent& event);
	void OnBrushClear(wxCommandEvent& event);
	void OnBrushCreate(wxCommandEvent& event);
	void OnBrushSolid(wxCommandEvent& event);
	//selection adjustment
	//2d influence
	void OnBrush2dinflChange(wxScrollEvent &event);
	void OnBrush2dinflText(wxCommandEvent &event);
	//edge detect
	void OnBrushEdgeDetectChk(wxCommandEvent &event);
	//hidden removal
	void OnBrushHiddenRemovalChk(wxCommandEvent &event);
	//estimate thresh
	void OnEstimateThreshChk(wxCommandEvent &event);
	//select group
	void OnBrushSelectGroupChk(wxCommandEvent &event);
	//translate
	void OnBrushSclTranslateChange(wxScrollEvent &event);
	void OnBrushSclTranslateText(wxCommandEvent &event);
	//gm falloff
	void OnBrushGmFalloffChange(wxScrollEvent &event);
	void OnBrushGmFalloffText(wxCommandEvent &event);
	//brush properties
	//brush size 1
	void OnBrushSize1Change(wxScrollEvent &event);
	void OnBrushSize1Text(wxCommandEvent &event);
	//brush size 2
	void OnBrushSize2Chk(wxCommandEvent &event);
	void OnBrushSize2Change(wxScrollEvent &event);
	void OnBrushSize2Text(wxCommandEvent &event);
	//brush iterations
	void OnBrushIterCheck(wxCommandEvent& event);
	//brush size relation
	void OnBrushSizeRelationCheck(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()
};

#endif//_BRUSHTOOLDLG_H_
