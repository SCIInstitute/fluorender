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
		ID_CalcLoadABtn = wxID_HIGHEST + 2001,
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

	DECLARE_EVENT_TABLE();
};

#endif//_COLOCALIZATIONDLG_H_
