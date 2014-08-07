#include <wx/wx.h>

#ifndef _BRUSHTOOLDLG_H_
#define _BRUSHTOOLDLG_H_

class VRenderView;
class VolumeData;

#define BRUSH_TOOL_ITER_WEAK	10
#define BRUSH_TOOL_ITER_NORMAL	30
#define BRUSH_TOOL_ITER_STRONG	60

class BrushToolDlg : public wxPanel
{
public:
	enum
	{
		//group1
		//toolbar
		ID_BrushAppend = wxID_HIGHEST+1301,
		ID_BrushDesel,
		ID_BrushDiffuse,
		ID_BrushErase,
		ID_BrushClear,
		ID_BrushCreate,
		//selection strength
		//falloff
		ID_BrushSclTranslateSldr,
		ID_BrushSclTranslateText,
		//2dinfl
		ID_Brush2dinflSldr,
		ID_Brush2dinflText,
		//edge detect
		ID_BrushEdgeDetectChk,
		//hidden removal
		ID_BrushHiddenRemovalChk,
		//select group
		ID_BrushSelectGroupChk,
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
		//component analyze
		ID_CASelectOnlyChk,
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
		ID_CASizeMapChk,
		//noise removal
		ID_NRSizeSldr,
		ID_NRSizeText,
		ID_NRAnalyzeBtn,
		ID_NRRemoveBtn,
		//help
		ID_HelpBtn,
		//default
		ID_SaveDefault,

		//group2
		//calculations
		//operand A
		ID_CalcLoadABtn,
		ID_CalcAText,
		//operand B
		ID_CalcLoadBBtn,
		ID_CalcBText,
		//two-opeartors
		ID_CalcSubBtn,
		ID_CalcAddBtn,
		ID_CalcDivBtn,
		ID_CalcIscBtn,
		//one-opeartors
		ID_CalcFillBtn
	};

	BrushToolDlg(wxWindow* frame,
		wxWindow* parent);
	~BrushToolDlg();

	void GetSettings(VRenderView* vrv);

	//set the brush icon down
	void SelectBrush(int id);

	//get some default values
	double GetDftCAMin() {return m_dft_ca_min;}
	void SetDftCAMin(double min) {m_dft_ca_min = min;}
	double GetDftCAMax() {return m_dft_ca_max;}
	void SetDftCAMax(double max) {m_dft_ca_max = max;}
	double GetDftCAThresh() {return m_dft_ca_thresh;}
	void SetDftCAThresh(double thresh) {m_dft_ca_thresh = thresh;}
	double GetDftCAFalloff() {return m_dft_ca_falloff;}
	void SetDftCAFalloff(double falloff) {m_dft_ca_falloff = falloff;}
	double GetDftNRThresh() {return m_dft_nr_thresh;}
	void SetDftNRThresh(double thresh) {m_dft_nr_thresh = thresh;}
	double GetDftNRSize() {return m_dft_nr_size;}
	void SetDftNRSize(double size) {m_dft_nr_size = size;}

	//save default
	void SaveDefault();

private:
	wxWindow* m_frame;

	//current view
	VRenderView *m_cur_view;
	//current volume
	VolumeData *m_vol1;
	VolumeData *m_vol2;

	//max volume value
	double m_max_value;
	//default brush properties
	double m_dft_ini_thresh;
	double m_dft_gm_falloff;
	double m_dft_scl_falloff;
	double m_dft_scl_translate;
	//default ca properties
	double m_dft_ca_min;
	double m_dft_ca_max;
	double m_dft_ca_thresh;
	double m_dft_ca_falloff;
	double m_dft_nr_thresh;
	double m_dft_nr_size;

	//paint tools
	//toolbar
	wxToolBar *m_toolbar;

	//selection strength
	//translate
	wxSlider* m_brush_scl_translate_sldr;
	wxTextCtrl* m_brush_scl_translate_text;
	//stop at boundary
	wxCheckBox* m_edge_detect_chk;
	//hidden removal
	wxCheckBox* m_hidden_removal_chk;
	//group selection
	wxCheckBox* m_select_group_chk;
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
	wxCheckBox *m_ca_size_map_chk;
	//noise removal
	wxSlider *m_nr_size_sldr;
	wxTextCtrl *m_nr_size_text;
	wxButton *m_nr_analyze_btn;
	wxButton *m_nr_remove_btn;
	//help button
	wxButton* m_help_btn;

	//calculations
	//operands
	wxButton *m_calc_load_a_btn;
	wxTextCtrl *m_calc_a_text;
	wxButton *m_calc_load_b_btn;
	wxTextCtrl *m_calc_b_text;
	//two-operators
	wxButton *m_calc_sub_btn;
	wxButton *m_calc_add_btn;
	wxButton *m_calc_div_btn;
	wxButton *m_calc_isc_btn;
	//one-operators
	wxButton *m_calc_fill_btn;

private:
	void LoadDefault();

	//event handling
	//paint tools
	//brush commands
	void OnBrushAppend(wxCommandEvent& event);
	void OnBrushDesel(wxCommandEvent& event);
	void OnBrushDiffuse(wxCommandEvent& event);
	void OnBrushErase(wxCommandEvent& event);
	void OnBrushClear(wxCommandEvent& event);
	void OnBrushCreate(wxCommandEvent& event);
	//selection adjustment
	//translate
	void OnBrushSclTranslateChange(wxScrollEvent &event);
	void OnBrushSclTranslateText(wxCommandEvent &event);
	//2d influence
	void OnBrush2dinflChange(wxScrollEvent &event);
	void OnBrush2dinflText(wxCommandEvent &event);
	//edge detect
	void OnBrushEdgeDetectChk(wxCommandEvent &event);
	//hidden removal
	void OnBrushHiddenRemovalChk(wxCommandEvent &event);
	//select group
	void OnBrushSelectGroupChk(wxCommandEvent &event);
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
	//component analyzer
	void OnCAThreshChange(wxScrollEvent &event);
	void OnCAThreshText(wxCommandEvent &event);
	void OnCAAnalyzeBtn(wxCommandEvent &event);
	void OnCAIgnoreMaxChk(wxCommandEvent &event);
	void OnCAMultiChannBtn(wxCommandEvent &event);
	void OnCARandomColorBtn(wxCommandEvent &event);
	void OnCAAnnotationsBtn(wxCommandEvent &event);
	//noise removal
	void OnNRSizeChange(wxScrollEvent &event);
	void OnNRSizeText(wxCommandEvent &event);
	void OnNRAnalyzeBtn(wxCommandEvent &event);
	void OnNRRemoveBtn(wxCommandEvent &event);
	//help
	void OnHelpBtn(wxCommandEvent& event);

	//calculations
	//operands
	void OnLoadA(wxCommandEvent &event);
	void OnLoadB(wxCommandEvent &event);
	//operators
	void OnCalcSub(wxCommandEvent &event);
	void OnCalcAdd(wxCommandEvent &event);
	void OnCalcDiv(wxCommandEvent &event);
	void OnCalcIsc(wxCommandEvent &event);
	//one-operators
	void OnCalcFill(wxCommandEvent &event);

	DECLARE_EVENT_TABLE();
};

#endif//_BRUSHTOOLDLG_H_
