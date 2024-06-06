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
#ifndef _NOISECANCELLINGDLG_H_
#define _NOISECANCELLINGDLG_H_

#include <wx/wx.h>
#include <Types/Color.h>

class MainFrame;
class RenderCanvas;
class VolumeData;
class DataGroup;
class wxSingleSlider;
class NoiseCancellingDlg : public wxPanel
{
public:
	enum
	{
		ID_ThresholdSldr = ID_NOISE_CANCEL,
		ID_ThresholdText,
		ID_VoxelSldr,
		ID_VoxelText,
		ID_CASelectOnlyChk,
		ID_PreviewBtn,
		ID_EraseBtn,
		ID_EnhanceSelChk
	};

	NoiseCancellingDlg(MainFrame* frame);
	~NoiseCancellingDlg();

	void GetSettings(RenderCanvas* view);
	void SetDftThresh(double thresh) {m_dft_thresh = thresh;}
	void SetDftSize(double size) {m_dft_size = size;}

	void Preview(bool select, double size, double thresh);

private:
	MainFrame* m_frame;
	//current view
	RenderCanvas *m_view;

	//max volume value
	double m_max_value;
	//default cs thresh
	double m_dft_thresh;
	//default nr size
	double m_dft_size;

	//remember previous hdr
	bool m_previewed;
	fluo::Color m_hdr;

	wxCheckBox *m_ca_select_only_chk;
	//threshold
	wxSingleSlider *m_threshold_sldr;
	wxTextCtrl *m_threshold_text;
	//voxel size threhsold
	wxSingleSlider *m_voxel_sldr;
	wxTextCtrl *m_voxel_text;
	//preview
	wxButton *m_preview_btn;
	//erase
	wxButton *m_erase_btn;
	//enhance selection
	wxCheckBox *m_enhance_sel_chk;

private:
	//threhsold
	void OnThresholdChange(wxScrollEvent &event);
	void OnThresholdText(wxCommandEvent &event);
	void OnVoxelChange(wxScrollEvent &event);
	void OnVoxelText(wxCommandEvent &event);
	void OnPreviewBtn(wxCommandEvent &event);
	void OnEraseBtn(wxCommandEvent &event);
	void OnEnhanceSelChk(wxCommandEvent &event);

	DECLARE_EVENT_TABLE()
};

#endif//_NOISECANCELLINGDLG_H_
