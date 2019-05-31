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
#include "Components/CompGenerator.h"
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
		ID_IterSldr,
		ID_IterText,
		//threshold
		ID_ThreshSldr,
		ID_ThreshText,
		//distance field
		ID_UseDistFieldCheck,
		ID_DistStrengthSldr,
		ID_DistStrengthText,
		ID_DistFilterSizeSldr,
		ID_DistFilterSizeText,
		ID_MaxDistSldr,
		ID_MaxDistText,
		ID_DistThreshSldr,
		ID_DistThreshText,
		//falloff
		ID_DiffCheck,
		ID_FalloffSldr,
		ID_FalloffText,
		//size
		ID_SizeCheck,
		ID_SizeSldr,
		ID_SizeText,
		//density
		ID_DensityCheck,
		ID_DensitySldr,
		ID_DensityText,
		ID_DensityWindowSizeSldr,
		ID_DensityWindowsSizeText,
		ID_DensityStatsSizeSldr,
		ID_DensityStatsSizeText,
		//fixate
		ID_FixateCheck,
		ID_FixUpdateBtn,
		ID_FixSizeSldr,
		ID_FixSizeText,
		//clean
		ID_CleanCheck,
		ID_CleanBtn,
		ID_CleanIterSldr,
		ID_CleanIterText,
		ID_CleanLimitSldr,
		ID_CleanLimitText,
		//record
		ID_CmdCountText,
		ID_RecordCmdBtn,
		ID_PlayCmdBtn,
		ID_ResetCmdBtn,
		ID_CmdFileText,
		ID_SaveCmdBtn,
		ID_LoadCmdBtn,

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

	void GenerateComp(bool command=true);
	void Fixate(bool command = true);
	void Clean(bool command = true);
	void SelectFullComp();

	FL::ComponentAnalyzer* GetAnalyzer()
	{
		return &m_comp_analyzer;
	}

	void AddCmd(const std::string &type);
	void ResetCmd();
	void PlayCmd();

private:
	wxWindow* m_frame;
	VRenderView* m_view;

	//progress
	float m_prog_bit;
	float m_prog;

	//generate settings
	int m_iter;
	double m_thresh;
	//distance field
	bool m_use_dist_field;
	double m_dist_strength;
	int m_dist_filter_size;
	int m_max_dist;
	double m_dist_thresh;
	//diffusion
	bool m_diff;
	double m_falloff;
	bool m_size;
	int m_size_lm;
	//density
	bool m_density;
	double m_density_thresh;
	int m_density_window_size;
	int m_density_stats_size;
	//fixate
	bool m_fixate;
	int m_fix_size;
	//clean
	bool m_clean;
	int m_clean_iter;
	int m_clean_size_vl;

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

	//record
	bool m_record_cmd;
	FL::CompCommand m_command;

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
	wxSlider* m_iter_sldr;
	wxTextCtrl* m_iter_text;
	wxSlider* m_thresh_sldr;
	wxTextCtrl* m_thresh_text;
	//distance field
	wxCheckBox* m_use_dist_field_check;
	wxSlider* m_dist_strength_sldr;
	wxTextCtrl* m_dist_strength_text;
	wxSlider* m_dist_filter_size_sldr;
	wxTextCtrl* m_dist_filter_size_text;
	wxSlider* m_max_dist_sldr;
	wxTextCtrl* m_max_dist_text;
	wxSlider* m_dist_thresh_sldr;
	wxTextCtrl* m_dist_thresh_text;
	//diffusion
	wxCheckBox* m_diff_check;
	wxSlider* m_falloff_sldr;
	wxTextCtrl* m_falloff_text;
	wxCheckBox* m_size_check;
	wxSlider* m_size_sldr;
	wxTextCtrl* m_size_text;
	//density
	wxCheckBox* m_density_check;
	wxSlider* m_density_sldr;
	wxTextCtrl* m_density_text;
	wxSlider* m_density_window_size_sldr;
	wxTextCtrl* m_density_window_size_text;
	wxSlider* m_density_stats_size_sldr;
	wxTextCtrl* m_density_stats_size_text;
	//fixate
	wxCheckBox* m_fixate_check;
	wxButton* m_fix_update_btn;
	wxSlider* m_fix_size_sldr;
	wxTextCtrl* m_fix_size_text;
	//clean
	wxCheckBox* m_clean_check;
	wxButton* m_clean_btn;
	wxSlider* m_clean_iter_sldr;
	wxTextCtrl* m_clean_iter_text;
	wxSlider* m_clean_limit_sldr;
	wxTextCtrl* m_clean_limit_text;
	//record
	wxTextCtrl* m_cmd_count_text;
	wxToggleButton* m_record_cmd_btn;
	wxButton* m_play_cmd_btn;
	wxButton* m_reset_cmd_btn;
	wxTextCtrl* m_cmd_file_text;
	wxButton* m_save_cmd_btn;
	wxButton* m_load_cmd_btn;

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
	void OnIterSldr(wxScrollEvent &event);
	void OnIterText(wxCommandEvent &event);
	void OnThreshSldr(wxScrollEvent &event);
	void OnThreshText(wxCommandEvent &event);
	//dist field
	void OnUseDistFieldCheck(wxCommandEvent &event);
	void EnableUseDistField(bool value);
	void OnDistStrengthSldr(wxScrollEvent &event);
	void OnDistStrengthText(wxCommandEvent &event);
	void OnDistFilterSizeSldr(wxScrollEvent &event);
	void OnDistFitlerSizeText(wxCommandEvent &event);
	void OnMaxDistSldr(wxScrollEvent &event);
	void OnMaxDistText(wxCommandEvent &event);
	void OnDistThreshSldr(wxScrollEvent &event);
	void OnDistThreshText(wxCommandEvent &event);
	//diff
	void EnableDiff(bool value);
	void OnDiffCheck(wxCommandEvent &event);
	void OnFalloffSldr(wxScrollEvent &event);
	void OnFalloffText(wxCommandEvent &event);
	void EnableSize(bool value);
	void OnSizeCheck(wxCommandEvent &event);
	void OnSizeSldr(wxScrollEvent &event);
	void OnSizeText(wxCommandEvent &event);
	//density
	void EnableDensity(bool value);
	void OnDensityCheck(wxCommandEvent &event);
	void OnDensitySldr(wxScrollEvent &event);
	void OnDensityText(wxCommandEvent &event);
	void OnDensityWindowSizeSldr(wxScrollEvent &event);
	void OnDensityWindowSizeText(wxCommandEvent &event);
	void OnDensityStatsSizeSldr(wxScrollEvent &event);
	void OnDensityStatsSizeText(wxCommandEvent &event);
	//fixate
	void OnFixateCheck(wxCommandEvent &event);
	void EnableFixate(bool value);
	void OnFixUpdateBtn(wxCommandEvent &event);
	void OnFixSizeSldr(wxScrollEvent &event);
	void OnFixSizeText(wxCommandEvent &event);
	//clean
	void EnableClean(bool value);
	void OnCleanCheck(wxCommandEvent &event);
	void OnCleanBtn(wxCommandEvent &event);
	void OnCleanIterSldr(wxScrollEvent &event);
	void OnCleanIterText(wxCommandEvent &event);
	void OnCleanLimitSldr(wxScrollEvent &event);
	void OnCleanLimitText(wxCommandEvent &event);
	//record
	void OnRecordCmd(wxCommandEvent &event);
	void OnPlayCmd(wxCommandEvent &event);
	void OnResetCmd(wxCommandEvent &event);
	void OnSaveCmd(wxCommandEvent &event);
	void OnLoadCmd(wxCommandEvent &event);

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
