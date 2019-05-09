/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2018 Scientific Computing and Imaging Institute,
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
#include "Main.h"
#include "DataManager.h"
#include "Components/CompAnalyzer.h"
#include <wx/wx.h>
#include <wx/collpane.h>
#include <wx/notebook.h>
#include <wx/spinctrl.h>
#include <wx/tglbtn.h>

#ifndef _COMPONENTDLG_H_
#define _COMPONENTDLG_H_

class VRenderView;
class VolumeData;

DECLARE_APP(VRenderApp)
class ComponentDlg : public wxPanel
{
public:
	enum
	{
		// load / save
		ID_LoadSettingsText = ID_COMPONENT,
		ID_LoadSettingsBtn,
		ID_SaveSettingsBtn,
		ID_SaveasSettingsBtn,

		//generate page
		//iterations
		ID_BasicIterSldr,
		ID_BasicIterText,
		//threshold
		ID_BasicThreshSldr,
		ID_BasicThreshText,
		//distance field
		ID_BasicUseDistFieldCheck,
		ID_BasicDistStrengthSldr,
		ID_BasicDistStrengthText,
		ID_BasicMaxDistSldr,
		ID_BasicMaxDistText,
		ID_BasicDistThreshSldr,
		ID_BasicDistThreshText,
		//falloff
		ID_BasicDiffCheck,
		ID_BasicFalloffSldr,
		ID_BasicFalloffText,
		//size
		ID_BasicSizeCheck,
		ID_BasicSizeSldr,
		ID_BasicSizeText,
		//density
		ID_BasicDensityCheck,
		ID_BasicDensitySldr,
		ID_BasicDensityText,
		ID_BasicDensityWindowSizeSldr,
		ID_BasicDensityWindowsSizeText,
		ID_BasicDensityStatsSizeSldr,
		ID_BasicDensityStatsSizeText,
		//clean
		ID_BasicCleanCheck,
		ID_BasicCleanBtn,
		ID_BasicCleanIterSldr,
		ID_BasicCleanIterText,
		ID_BasicCleanLimitSldr,
		ID_BasicCleanLimitText,

		//clustering page
		ID_ClusterMethodExmaxRd,
		ID_ClusterMethodDbscanRd,
		ID_ClusterMethodKmeansRd,
		//parameters
		ID_ClusterClnumSldr,
		ID_ClusterClnumText,
		ID_ClusterMaxIterSldr,
		ID_ClusterMaxIterText,
		ID_ClusterTolSldr,
		ID_ClusterTolText,
		ID_ClusterSizeSldr,
		ID_ClusterSizeText,
		ID_ClusterEpsSldr,
		ID_ClusterEpsText,

		//analysis page
		//selection
		ID_CompIdText,
		ID_CompIdXBtn,
		ID_AnalysisMinCheck,
		ID_AnalysisMinSpin,
		ID_AnalysisMaxCheck,
		ID_AnalysisMaxSpin,
		ID_CompFullBtn,
		ID_CompExclusiveBtn,
		ID_CompAppendBtn,
		ID_CompAllBtn,
		ID_CompClearBtn,
		//options
		ID_ConsistentCheck,
		ID_ColocalCheck,
		//output
		ID_OutputMultiRb,
		ID_OutputRgbRb,
		ID_OutputRandomBtn,
		ID_OutputSizeBtn,
		ID_OutputAnnBtn,
		//Distance
		ID_DistNeighborSldr,
		ID_DistNeighborText,
		ID_DistOutputBtn,

		//execute
		ID_Notebook,
		ID_UseSelChk,
		ID_GeneratePrg,
		ID_GenerateBtn,
		ID_AutoUpdateBtn,
		ID_ClusterBtn,
		ID_AnalyzeBtn,
		ID_AnalyzeSelBtn,

		//output
		ID_StatText
	};

	ComponentDlg(wxWindow* frame,
		wxWindow* parent);
	~ComponentDlg();

	void Update();
	void GetSettings();
	void LoadSettings(wxString filename);
	void SaveSettings(wxString filename);
	void SetView(VRenderView* vrv) {
		m_view = vrv;
	}
	VRenderView* GetView() {
		return m_view;
	}

	//update progress
	void UpdateProgress()
	{
		m_prog += m_prog_bit;
		m_generate_prg->SetValue(int(m_prog));
		wxGetApp().Yield();
	}

	//type: 0-basic; 1-advanced
	//mode: 0-generate; 1-refine
	void GenerateComp();
	void SelectFullComp();

	FL::ComponentAnalyzer* GetAnalyzer()
	{
		return &m_comp_analyzer;
	}

private:
	wxWindow* m_frame;
	VRenderView* m_view;

	//progress
	float m_prog_bit;
	float m_prog;

	//generate settings
	int m_basic_iter;
	double m_basic_thresh;
	//distance field
	bool m_use_dist_field;
	double m_basic_dist_strength;
	int m_basic_max_dist;
	double m_basic_dist_thresh;
	//diffusion
	bool m_basic_diff;
	double m_basic_falloff;
	bool m_basic_size;
	int m_basic_size_lm;
	//density
	bool m_basic_density;
	double m_basic_density_thresh;
	int m_basic_density_window_size;
	int m_basic_density_stats_size;
	//clean
	bool m_basic_clean;
	int m_basic_clean_iter;
	int m_basic_clean_size_vl;

	//cluster settings
	bool m_cluster_method_exmax;
	bool m_cluster_method_dbscan;
	bool m_cluster_method_kmeans;
	//parameters
	int m_cluster_clnum;
	int m_cluster_maxiter;
	float m_cluster_tol;
	int m_cluster_size;
	double m_cluster_eps;

	//selection
	bool m_use_min;
	int m_min_num;
	bool m_use_max;
	int m_max_num;
	//options
	bool m_consistent;
	bool m_colocal;

	//distance
	int m_dist_neighbor;

	//output
	int m_output_type;//1-multi; 2-rgb;

	//auto udate
	bool m_auto_update;

	FL::ComponentAnalyzer m_comp_analyzer;

	//tab control
	wxNotebook *m_notebook;
	//wxScrolledWindow* m_adv_page;

	//load/save settings
	wxTextCtrl* m_load_settings_text;
	wxButton* m_load_settings_btn;
	wxButton* m_save_settings_btn;
	wxButton* m_saveas_settings_btn;

	//wxCollapsiblePane* m_initial_grow_pane;

	//generate page
	wxSlider* m_basic_iter_sldr;
	wxTextCtrl* m_basic_iter_text;
	wxSlider* m_basic_thresh_sldr;
	wxTextCtrl* m_basic_thresh_text;
	//distance field
	wxCheckBox* m_use_dist_field_check;
	wxSlider* m_basic_dist_strength_sldr;
	wxTextCtrl* m_basic_dist_strength_text;
	wxSlider* m_basic_max_dist_sldr;
	wxTextCtrl* m_basic_max_dist_text;
	wxSlider* m_basic_dist_thresh_sldr;
	wxTextCtrl* m_basic_dist_thresh_text;
	//diffusion
	wxCheckBox* m_basic_diff_check;
	wxSlider* m_basic_falloff_sldr;
	wxTextCtrl* m_basic_falloff_text;
	wxCheckBox* m_basic_size_check;
	wxSlider* m_basic_size_sldr;
	wxTextCtrl* m_basic_size_text;
	//density
	wxCheckBox* m_basic_density_check;
	wxSlider* m_basic_density_sldr;
	wxTextCtrl* m_basic_density_text;
	wxSlider* m_basic_density_window_size_sldr;
	wxTextCtrl* m_basic_density_window_size_text;
	wxSlider* m_basic_density_stats_size_sldr;
	wxTextCtrl* m_basic_density_stats_size_text;
	//clean
	wxCheckBox* m_basic_clean_check;
	wxButton* m_basic_clean_btn;
	wxSlider* m_basic_clean_iter_sldr;
	wxTextCtrl* m_basic_clean_iter_text;
	wxSlider* m_basic_clean_limit_sldr;
	wxTextCtrl* m_basic_clean_limit_text;

	//clustering page
	wxRadioButton* m_cluster_method_exmax_rd;
	wxRadioButton* m_cluster_method_dbscan_rd;
	wxRadioButton* m_cluster_method_kmeans_rd;
	//parameters
	wxSlider* m_cluster_clnum_sldr;
	wxTextCtrl* m_cluster_clnum_text;
	wxSlider* m_cluster_maxiter_sldr;
	wxTextCtrl* m_cluster_maxiter_text;
	wxSlider* m_cluster_tol_sldr;
	wxTextCtrl* m_cluster_tol_text;
	wxSlider* m_cluster_size_sldr;
	wxTextCtrl* m_cluster_size_text;
	wxSlider* m_cluster_eps_sldr;
	wxTextCtrl* m_cluster_eps_text;

	//analysis page
	//selection
	wxTextCtrl* m_comp_id_text;
	wxButton* m_comp_id_x_btn;
	wxButton* m_comp_full_btn;
	wxButton* m_comp_exclusive_btn;
	wxButton* m_comp_append_btn;
	wxButton* m_comp_all_btn;
	wxButton* m_comp_clear_btn;
	//stats
	wxCheckBox* m_analysis_min_check;
	wxSpinCtrl* m_analysis_min_spin;
	wxCheckBox* m_analysis_max_check;
	wxSpinCtrl* m_analysis_max_spin;
	//options
	wxCheckBox* m_consistent_check;
	wxCheckBox* m_colocal_check;
	//output
	wxRadioButton* m_output_multi_rb;
	wxRadioButton* m_output_rgb_rb;
	wxButton* m_output_random_btn;
	wxButton* m_output_size_btn;
	wxButton* m_output_ann_btn;
	//distance
	wxSlider* m_dist_neighbor_sldr;
	wxTextCtrl* m_dist_neighbor_text;
	wxButton* m_dist_output_btn;

	//execute
	wxGauge* m_generate_prg;
	wxCheckBox* m_use_sel_chk;
	wxButton* m_generate_btn;
	wxToggleButton* m_auto_update_btn;
	wxButton* m_cluster_btn;
	wxButton* m_analyze_btn;
	wxButton* m_analyze_sel_btn;

	//output
	wxTextCtrl* m_stat_text;

private:
	void Cluster();
	bool GetIds(std::string &str, unsigned int &id, int &brick_id);

	wxWindow* CreateCompGenPage(wxWindow *parent);
	wxWindow* CreateClusteringPage(wxWindow *parent);
	wxWindow* CreateAnalysisPage(wxWindow *parent);

	//load/save settings
	void OnLoadSettings(wxCommandEvent &event);
	void OnSaveSettings(wxCommandEvent &event);
	void OnSaveasSettings(wxCommandEvent &event);
	//void OnPaneChange(wxCollapsiblePaneEvent& event);
	//wxCollapsiblePane* CreateInitialGrowPane(wxWindow *parent);

	//comp generate page
	void OnBasicIterSldr(wxScrollEvent &event);
	void OnBasicIterText(wxCommandEvent &event);
	void OnBasicThreshSldr(wxScrollEvent &event);
	void OnBasicThreshText(wxCommandEvent &event);
	void OnBasicUseDistFieldCheck(wxCommandEvent &event);
	void EnableUseDistField(bool value);
	void OnBasicDistStrengthSldr(wxScrollEvent &event);
	void OnBasicDistStrengthText(wxCommandEvent &event);
	void OnBasicMaxDistSldr(wxScrollEvent &event);
	void OnBasicMaxDistText(wxCommandEvent &event);
	void OnBasicDistThreshSldr(wxScrollEvent &event);
	void OnBasicDistThreshText(wxCommandEvent &event);
	void EnableBasicDiff(bool value);
	void OnBasicDiffCheck(wxCommandEvent &event);
	void OnBasicFalloffSldr(wxScrollEvent &event);
	void OnBasicFalloffText(wxCommandEvent &event);
	void EnableBasicSize(bool value);
	void OnBasicSizeCheck(wxCommandEvent &event);
	void OnBasicSizeSldr(wxScrollEvent &event);
	void OnBasicSizeText(wxCommandEvent &event);
	//density
	void EnableBasicDensity(bool value);
	void OnBasicDensityCheck(wxCommandEvent &event);
	void OnBasicDensitySldr(wxScrollEvent &event);
	void OnBasicDensityText(wxCommandEvent &event);
	void OnBasicDensityWindowSizeSldr(wxScrollEvent &event);
	void OnBasicDensityWindowSizeText(wxCommandEvent &event);
	void OnBasicDensityStatsSizeSldr(wxScrollEvent &event);
	void OnBasicDensityStatsSizeText(wxCommandEvent &event);
	//clean
	void EnableBasicClean(bool value);
	void OnBasicCleanCheck(wxCommandEvent &event);
	void OnBasicCleanBtn(wxCommandEvent &event);
	void OnBasicCleanIterSldr(wxScrollEvent &event);
	void OnBasicCleanIterText(wxCommandEvent &event);
	void OnBasicCleanLimitSldr(wxScrollEvent &event);
	void OnBasicCleanLimitText(wxCommandEvent &event);

	//clustering page
	void UpdateClusterMethod();
	void OnClusterMethodExmaxCheck(wxCommandEvent &event);
	void OnClusterMethodDbscanCheck(wxCommandEvent &event);
	void OnClusterMethodKmeansCheck(wxCommandEvent &event);
	//parameters
	void OnClusterClnumSldr(wxScrollEvent &event);
	void OnClusterClnumText(wxCommandEvent &event);
	void OnClusterMaxiterSldr(wxScrollEvent &event);
	void OnClusterMaxiterText(wxCommandEvent &event);
	void OnClusterTolSldr(wxScrollEvent &event);
	void OnClusterTolText(wxCommandEvent &event);
	void OnClusterSizeSldr(wxScrollEvent &event);
	void OnClusterSizeText(wxCommandEvent &event);
	void OnClusterEpsSldr(wxScrollEvent &event);
	void OnClusterepsText(wxCommandEvent &event);

	//analysis page
	void OnCompIdText(wxCommandEvent &event);
	void OnCompIdXBtn(wxCommandEvent &event);
	void OnAnalysisMinCheck(wxCommandEvent &event);
	void OnAnalysisMinSpin(wxSpinEvent &event);
	void OnAnalysisMaxCheck(wxCommandEvent &event);
	void OnAnalysisMaxSpin(wxSpinEvent &event);
	void OnCompFull(wxCommandEvent &event);
	void OnCompExclusive(wxCommandEvent &event);
	void OnCompAppend(wxCommandEvent &event);
	void OnCompAll(wxCommandEvent &event);
	void OnCompClear(wxCommandEvent &event);
	void OnConsistentCheck(wxCommandEvent &event);
	void OnColocalCheck(wxCommandEvent &event);
	//output
	void OnOutputTypeRadio(wxCommandEvent &event);
	void OutputMulti(int color_type);
	void OutputRgb(int color_type);
	void OnOutputChannels(wxCommandEvent &event);
	void OnOutputAnn(wxCommandEvent &event);
	//distance
	void OnDistNeighborSldr(wxScrollEvent &event);
	void OnDistNeighborText(wxCommandEvent &event);
	void OnDistOutput(wxCommandEvent &event);

	//execute
	void EnableGenerate();
	void OnNotebook(wxBookCtrlEvent &event);
	void OnGenerate(wxCommandEvent &event);
	void OnAutoUpdate(wxCommandEvent &event);
	void OnCluster(wxCommandEvent &event);
	void OnAnalyze(wxCommandEvent &event);
	void OnAnalyzeSel(wxCommandEvent &event);
	void Analyze(bool sel);

	//select output
	void OnKey(wxKeyEvent &event);

	DECLARE_EVENT_TABLE()
};

#endif//_COMPONENTDLG_H_
