#ifndef _COUNTINGDLG_H_
#define _COUNTINGDLG_H_

#include <wx/wx.h>

class VRenderView;
class VolumeData;
class DataGroup;

class CountingDlg : public wxPanel
{
public:
	enum
	{
		ID_CASelectOnlyChk = wxID_HIGHEST+1901,
		ID_CAMinText,
		ID_CAMaxText,
		ID_CAIgnoreMaxChk,
		ID_CAThreshSldr,
		ID_CAThreshText,
		ID_CAAnalyzeBtn,
		ID_CAMultiChannBtn,
		ID_CARandomColorBtn,
		ID_CAAnnotationsBtn,
		ID_CACompsText,
		ID_CAVolumeText,
		ID_CAVolUnitText
	};

	CountingDlg(wxWindow* frame,
		wxWindow* parent);
	~CountingDlg();

	void GetSettings(VRenderView* vrv);

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

	//component analyzer
	wxCheckBox *m_ca_select_only_chk;
	wxTextCtrl *m_ca_min_text;
	wxTextCtrl *m_ca_max_text;
	wxCheckBox *m_ca_ignore_max_chk;
	wxSlider *m_ca_thresh_sldr;
	wxTextCtrl *m_ca_thresh_text;
	wxButton *m_ca_analyze_btn;
	wxButton *m_ca_multi_chann_btn;
	wxButton *m_ca_random_color_btn;
	wxButton *m_ca_annotations_btn;
	wxTextCtrl *m_ca_comps_text;
	wxTextCtrl *m_ca_volume_text;
	wxTextCtrl *m_ca_vol_unit_text;

private:
	void LoadDefault();

	//component analyzer
	void OnCAThreshChange(wxScrollEvent &event);
	void OnCAThreshText(wxCommandEvent &event);
	void OnCAAnalyzeBtn(wxCommandEvent &event);
	void OnCAIgnoreMaxChk(wxCommandEvent &event);
	void OnCAMultiChannBtn(wxCommandEvent &event);
	void OnCARandomColorBtn(wxCommandEvent &event);
	void OnCAAnnotationsBtn(wxCommandEvent &event);

	DECLARE_EVENT_TABLE();
};

#endif//_COUNTINGDLG_H_
