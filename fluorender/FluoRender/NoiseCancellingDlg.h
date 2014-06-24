#include <wx/wx.h>
#include "FLIVR/Color.h"

#ifndef _NOISECANCELLINGDLG_H_
#define _NOISECANCELLINGDLG_H_

class VRenderView;
class VolumeData;
class DataGroup;

using namespace FLIVR;

class NoiseCancellingDlg : public wxPanel
{
public:
	enum
	{
		ID_ThresholdSldr = wxID_HIGHEST+1601,
		ID_ThresholdText,
		ID_VoxelSldr,
		ID_VoxelText,
		ID_PreviewBtn,
		ID_EraseBtn,
		ID_EnhanceSelChk
	};

	NoiseCancellingDlg(wxWindow* frame,
		wxWindow* parent);
	~NoiseCancellingDlg();

	void GetSettings(VRenderView* vrv);
	void SetDftThresh(double thresh) {m_dft_thresh = thresh;}
	void SetDftSize(double size) {m_dft_size = size;}

private:
	wxWindow* m_frame;

	//current view
	VRenderView *m_view;
	//current group
	DataGroup *m_group;
	//current volume
	VolumeData *m_vol;

	//max volume value
	double m_max_value;
	//default cs thresh
	double m_dft_thresh;
	//default nr size
	double m_dft_size;

	//remember previous hdr
	bool m_previewed;
	Color m_hdr;

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
	void OnThresholdChange(wxScrollEvent &event);
	void OnThresholdText(wxCommandEvent &event);
	void OnVoxelChange(wxScrollEvent &event);
	void OnVoxelText(wxCommandEvent &event);
	void OnPreviewBtn(wxCommandEvent &event);
	void OnEraseBtn(wxCommandEvent &event);
	void OnEnhanceSelChk(wxCommandEvent &event);

	DECLARE_EVENT_TABLE();
};

#endif//_NOISECANCELLINGDLG_H_
