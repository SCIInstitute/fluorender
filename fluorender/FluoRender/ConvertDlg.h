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
#include <wx/wx.h>

#ifndef _CONVERTDLG_H_
#define _CONVERTDLG_H_

class VRenderView;

class ConvertDlg : public wxPanel
{
public:
	enum
	{
		ID_CnvVolMeshThreshSldr = wxID_HIGHEST+1401,
		ID_CnvVolMeshThreshText,
		ID_CnvVolMeshDownsampleSldr,
		ID_CnvVolMeshDownsampleText,
		ID_CnvVolMeshDownsampleZSldr,
		ID_CnvVolMeshDownsampleZText,
		ID_CnvVolMeshUsetransfChk,
		ID_CnvVolMeshSelectedChk,
		ID_CnvVolMeshWeldChk,
		ID_CnvVolMeshConvertBtn,
		//output
		ID_StatText
	};

	ConvertDlg(wxWindow* frame, wxWindow* parent);
	~ConvertDlg();

	void GetSettings(VRenderView* vrv);

private:
	wxWindow* m_frame;

	//convert from volume to polygon mesh
	wxSlider* m_cnv_vol_mesh_thresh_sldr;
	wxTextCtrl* m_cnv_vol_mesh_thresh_text;
	wxSlider* m_cnv_vol_mesh_downsample_sldr;
	wxTextCtrl* m_cnv_vol_mesh_downsample_text;
	wxSlider* m_cnv_vol_mesh_downsample_z_sldr;
	wxTextCtrl* m_cnv_vol_mesh_downsample_z_text;
	wxCheckBox* m_cnv_vol_mesh_usetransf_chk;
	wxCheckBox* m_cnv_vol_mesh_selected_chk;
	wxCheckBox* m_cnv_vol_mesh_weld_chk;
	wxButton* m_cnv_vol_mesh_convert_btn;
	//output
	wxTextCtrl* m_stat_text;

	//convert from volume to mesh
	void OnCnvVolMeshThreshChange(wxScrollEvent &event);
	void OnCnvVolMeshThreshText(wxCommandEvent &event);
	void OnCnvVolMeshDownsampleChange(wxScrollEvent &event);
	void OnCnvVolMeshDownsampleText(wxCommandEvent &event);
	void OnCnvVolMeshDownsampleZChange(wxScrollEvent &event);
	void OnCnvVolMeshDownsampleZText(wxCommandEvent &event);
	void OnCnvVolMeshConvert(wxCommandEvent& event);

	DECLARE_EVENT_TABLE();
};

#endif//_CONVERTDLG_H_
