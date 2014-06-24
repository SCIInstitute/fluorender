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
		ID_CnvVolMeshConvertBtn
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
