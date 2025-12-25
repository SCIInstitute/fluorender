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
#ifndef _CONVERTDLG_H_
#define _CONVERTDLG_H_

#include <PropPanel.h>

class wxSingleSlider;
class ConvertDlg : public PropPanel
{
public:
	ConvertDlg(MainFrame* frame);
	~ConvertDlg();

	virtual void FluoUpdate(const fluo::ValueCollection& vc = {});

private:
	//convert from volume to polygon mesh
	wxButton* m_cnv_vol_mesh_convert_btn;
	wxButton* m_cnv_vol_mesh_update_btn;
	wxButton* m_cnv_vol_mesh_weld_btn;
	wxButton* m_cnv_vol_mesh_color_btn;
	wxButton* m_cnv_vol_mesh_simplify_btn;
	wxButton* m_cnv_vol_mesh_smooth_btn;

	wxCheckBox* m_cnv_vol_mesh_selected_chk;
	wxCheckBox* m_cnv_vol_mesh_usetransf_chk;

	wxSingleSlider* m_cnv_vol_mesh_thresh_sldr;
	wxTextCtrl* m_cnv_vol_mesh_thresh_text;
	wxSingleSlider* m_cnv_vol_mesh_downsample_sldr;
	wxTextCtrl* m_cnv_vol_mesh_downsample_text;
	wxSingleSlider* m_cnv_vol_mesh_downsample_z_sldr;
	wxTextCtrl* m_cnv_vol_mesh_downsample_z_text;

private:
	void OnCnvVolMeshConvert(wxCommandEvent& event);
	void OnCnvVolMeshUpdate(wxCommandEvent& event);
	void OnCnvVolMeshWeldVertices(wxCommandEvent& event);
	void OnCnvVolMeshColor(wxCommandEvent& event);
	void OnCnvVolMeshSimplify(wxCommandEvent& event);
	void OnCnvVolMeshSmooth(wxCommandEvent& event);

	void OnCnvVolMeshUseSelCheck(wxCommandEvent& event);
	void OnCnvVolMeshUseTransfCheck(wxCommandEvent& event);

	void OnCnvVolMeshThreshChange(wxScrollEvent& event);
	void OnCnvVolMeshThreshText(wxCommandEvent& event);
	void OnCnvVolMeshDownsampleChange(wxScrollEvent& event);
	void OnCnvVolMeshDownsampleText(wxCommandEvent& event);
	void OnCnvVolMeshDownsampleZChange(wxScrollEvent& event);
	void OnCnvVolMeshDownsampleZText(wxCommandEvent& event);
};

#endif//_CONVERTDLG_H_
