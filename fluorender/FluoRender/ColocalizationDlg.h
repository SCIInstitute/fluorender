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
#ifndef _COLOCALIZATIONDLG_H_
#define _COLOCALIZATIONDLG_H_

#include <wx/wx.h>

class VRenderView;
class VolumeData;
class DataGroup;
class Annotations;

class ColocalizationDlg : public wxPanel
{
public:
	enum
	{
		//operand A
		ID_CalcLoadABtn = ID_COLOCALIZE,
		ID_CalcAText,
		//operand B
		ID_CalcLoadBBtn,
		ID_CalcBText,
		ID_MinSizeSldr,
		ID_MinSizeText,
		ID_MaxSizeSldr,
		ID_MaxSizeText,
		ID_BrushSelectBothChk,
		ID_CalcColocalizationBtn
	};

	ColocalizationDlg(wxWindow* frame,
		wxWindow* parent);
	~ColocalizationDlg();

	void GetSettings(VRenderView* vrv);
	void SetVolumeA(VolumeData* vd);
	void SetVolumeB(VolumeData* vd);

private:
	wxWindow* m_frame;

	//current view
	VRenderView *m_view;
	//volume A
	VolumeData *m_vol_a;
	//volume B
	VolumeData *m_vol_b;

	//interface
	wxButton *m_calc_load_a_btn;
	wxTextCtrl *m_calc_a_text;
	wxButton *m_calc_load_b_btn;
	wxTextCtrl *m_calc_b_text;
	//min size
	wxSlider *m_min_size_sldr;
	wxTextCtrl *m_min_size_text;
	//max size
	wxSlider *m_max_size_sldr;
	wxTextCtrl *m_max_size_text;
	//select both
	wxCheckBox *m_select_both_chk;
	//colocalization
	wxButton *m_colocalization_btn;

private:
	//load
	void OnLoadA(wxCommandEvent &event);
	void OnLoadB(wxCommandEvent &event);
	//min size
	void OnMinSizeChange(wxScrollEvent &event);
	void OnMinSizeText(wxCommandEvent &event);
	//max size
	void OnMaxSizeChange(wxScrollEvent &event);
	void OnMaxSizeText(wxCommandEvent &event);
	//select both
	void OnSelectBothChk(wxCommandEvent &event);
	//calculate
	void OnColocalizationBtn(wxCommandEvent &event);

	DECLARE_EVENT_TABLE()
};

#endif//_COLOCALIZATIONDLG_H_
