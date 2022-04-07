/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2022 Scientific Computing and Imaging Institute,
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
#ifndef _NOISEREDUCEDLG_H_
#define _NOISEREDUCEDLG_H_

#include <wx/wx.h>
#include <NoiseReduceAgent.hpp>

class RenderFrame;
namespace fluo
{
	class Renderview;
}
class NoiseReduceDlg : public wxPanel
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

	NoiseReduceDlg(RenderFrame* frame);
	~NoiseReduceDlg();

	void AssociateRenderview(fluo::Renderview* view)
	{
		m_agent->setObject(view);
	}

	friend class fluo::NoiseReduceAgent;

private:
	fluo::NoiseReduceAgent* m_agent;

	wxCheckBox *m_ca_select_only_chk;
	//threshold
	wxSlider *m_threshold_sldr;
	wxTextCtrl *m_threshold_text;
	//voxel size threhsold
	wxSlider *m_voxel_sldr;
	wxTextCtrl *m_voxel_text;
	//preview
	wxButton *m_preview_btn;
	//erase
	wxButton *m_erase_btn;
	//enhance selection
	wxCheckBox *m_enhance_sel_chk;

private:
	//threhsold
	void OnSelectOnlyChk(wxCommandEvent &event);
	void OnThresholdChange(wxScrollEvent &event);
	void OnThresholdText(wxCommandEvent &event);
	void OnVoxelChange(wxScrollEvent &event);
	void OnVoxelText(wxCommandEvent &event);
	void OnPreviewBtn(wxCommandEvent &event);
	void OnEraseBtn(wxCommandEvent &event);
	void OnEnhanceSelChk(wxCommandEvent &event);

	DECLARE_EVENT_TABLE()
};

#endif//_NOISEREDUCEDLG_H_
