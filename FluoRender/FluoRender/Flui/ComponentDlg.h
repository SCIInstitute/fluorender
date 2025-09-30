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
#ifndef _COMPONENTDLG_H_
#define _COMPONENTDLG_H_

#include <PropPanel.h>
#include <wx/tglbtn.h>
#include <wx/spinctrl.h>
#include <wx/grid.h>
#include <wx/clipbrd.h>

class wxSingleSlider;
class ComponentDlg : public TabbedPanel
{
public:
	enum
	{
		//clustering page
		ID_ClusterMethodExmaxRd = 0,
		ID_ClusterMethodDbscanRd,
		ID_ClusterMethodKmeansRd
	};
	enum
	{
		//output
		ID_OutputMultiRb = 0,
		ID_OutputRgbRb
	};
	enum
	{
		ID_OutputRandomBtn = 0,
		ID_OutputSizeBtn,
		ID_OutputIdBtn,
		ID_OutputSnBtn
	};
	enum
	{
		//align
		ID_AlignXYZ = 0,
		ID_AlignYXZ,
		ID_AlignZXY,
		ID_AlignXZY,
		ID_AlignYZX,
		ID_AlignZYX,
	};

	ComponentDlg(MainFrame* frame);
	~ComponentDlg();

	virtual void FluoUpdate(const fluo::ValueCollection& vc = {});

	//output
	void CopyData();
	void PasteData();

	void OutputAnalysis(wxString &titles, wxString &values);
	void OutputDistance();
	void AlignPca(int axis_type);

	//selection
	void IncludeComps();
	void ExcludeComps();
	void UpdateCompSelection();
	void SelectGridCells();
	void DeleteGridRows();

	//setting funcs for sliders
	void SetIter(int val);
	void SetThresh(double val);
	void SetDistStrength(double val);
	void SetDistFilterSize(int val);
	void SetMaxDist(int val);
	void SetDistThresh(double val);
	void SetFalloff(double val);
	void SetDensity(double val);
	void SetVarth(double val);
	void SetDensityWindowSize(int val);
	void SetDensityStatsSize(int val);
	void SetFixSize(int val);
	void SetCleanIter(int val);
	void SetCleanLimit(int val);

private:
	int m_max_lines;

	//selected rows of output grid
	std::set<size_t> m_sel;
	bool m_supress_select = false;

	//output
	bool m_hold_history;

	//load/save settings
	wxTextCtrl* m_load_settings_text;
	wxButton* m_load_settings_btn;
	wxButton* m_save_settings_btn;
	wxButton* m_saveas_settings_btn;

	//generate page
	wxSingleSlider* m_iter_sldr;
	wxTextCtrl* m_iter_text;
	wxSingleSlider* m_thresh_sldr;
	wxTextCtrl* m_thresh_text;
	//distance field
	wxCheckBox* m_use_dist_field_check;
	wxSingleSlider* m_dist_strength_sldr;
	wxTextCtrl* m_dist_strength_text;
	wxSingleSlider* m_dist_filter_size_sldr;
	wxTextCtrl* m_dist_filter_size_text;
	wxSingleSlider* m_max_dist_sldr;
	wxTextCtrl* m_max_dist_text;
	wxSingleSlider* m_dist_thresh_sldr;
	wxTextCtrl* m_dist_thresh_text;
	//diffusion
	wxCheckBox* m_diff_check;
	wxSingleSlider* m_falloff_sldr;
	wxTextCtrl* m_falloff_text;
	wxCheckBox* m_size_check;
	wxSingleSlider* m_size_sldr;
	wxTextCtrl* m_size_text;
	//density
	wxCheckBox* m_density_check;
	wxSingleSlider* m_density_sldr;
	wxTextCtrl* m_density_text;
	wxSingleSlider* m_varth_sldr;
	wxTextCtrl* m_varth_text;
	wxSingleSlider* m_density_window_size_sldr;
	wxTextCtrl* m_density_window_size_text;
	wxSingleSlider* m_density_stats_size_sldr;
	wxTextCtrl* m_density_stats_size_text;
	//fixate
	wxCheckBox* m_fixate_check;
	wxCheckBox* m_grow_fixed_check;
	wxButton* m_fix_update_btn;
	wxSingleSlider* m_fix_size_sldr;
	wxTextCtrl* m_fix_size_text;
	//clean
	wxCheckBox* m_clean_check;
	wxButton* m_clean_btn;
	wxSingleSlider* m_clean_iter_sldr;
	wxTextCtrl* m_clean_iter_text;
	wxSingleSlider* m_clean_limit_sldr;
	wxTextCtrl* m_clean_limit_text;
	//record
	wxTextCtrl* m_cmd_count_text;
	wxToggleButton* m_record_cmd_btn;
	wxButton* m_play_cmd_btn;
	wxButton* m_reset_cmd_btn;
	wxTextCtrl* m_cmd_file_text;
	wxButton* m_save_cmd_btn;
	wxButton* m_load_cmd_btn;
	//auto update
	wxTimer m_auto_update_timer;

	//clustering page
	wxRadioButton* m_cluster_method_exmax_rd;
	wxRadioButton* m_cluster_method_dbscan_rd;
	wxRadioButton* m_cluster_method_kmeans_rd;
	//parameters
	wxSingleSlider* m_cluster_clnum_sldr;
	wxTextCtrl* m_cluster_clnum_text;
	wxSingleSlider* m_cluster_maxiter_sldr;
	wxTextCtrl* m_cluster_maxiter_text;
	wxSingleSlider* m_cluster_tol_sldr;
	wxTextCtrl* m_cluster_tol_text;
	wxSingleSlider* m_cluster_size_sldr;
	wxTextCtrl* m_cluster_size_text;
	wxSingleSlider* m_cluster_eps_sldr;
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
	//modify
	wxButton* m_comp_new_btn;
	wxButton* m_comp_add_btn;
	wxButton* m_comp_replace_btn;
	wxButton* m_comp_clean_bkg_btn;
	wxButton* m_comp_combine_btn;
	//stats
	wxCheckBox* m_analysis_min_check;
	wxSpinCtrl* m_analysis_min_spin;
	wxCheckBox* m_analysis_max_check;
	wxSpinCtrl* m_analysis_max_spin;
	//options
	wxSingleSlider* m_con_size_sldr;
	wxTextCtrl* m_con_size_text;
	wxCheckBox* m_consistent_check;
	wxCheckBox* m_colocal_check;
	//output
	wxRadioButton* m_output_multi_rb;
	wxRadioButton* m_output_rgb_rb;
	wxButton* m_output_random_btn;
	wxButton* m_output_size_btn;
	wxButton* m_output_mesh_btn;
	wxButton* m_output_id_btn;
	wxButton* m_output_sn_btn;
	//distance
	wxCheckBox* m_dist_neighbor_check;
	wxCheckBox* m_dist_all_chan_check;
	wxSingleSlider* m_dist_neighbor_sldr;
	wxTextCtrl* m_dist_neighbor_text;
	wxButton* m_dist_output_btn;
	//align
	wxCheckBox* m_align_center_chk;
	wxButton* m_align_xyz;
	wxButton* m_align_yxz;
	wxButton* m_align_zxy;
	wxButton* m_align_xzy;
	wxButton* m_align_yzx;
	wxButton* m_align_zyx;

	//execute
	wxButton* m_shf_gen_btn;
	wxButton* m_shf_cls_btn;
	wxButton* m_shf_anl_btn;
	wxCheckBox* m_use_ml_chk;
	wxCheckBox* m_use_sel_gen_chk;
	wxCheckBox* m_use_sel_anl_chk;
	wxButton* m_generate_btn;
	wxButton* m_cluster_btn;
	wxButton* m_analyze_btn;

	//output
	wxButton* m_include_btn;
	wxButton* m_exclude_btn;
	wxCheckBox* m_history_chk;
	wxButton* m_clear_hist_btn;
	wxGrid *m_output_grid;

private:
	void AddSelArrayInt(std::vector<unsigned int>& ids,
		std::vector<unsigned int>& bids, wxArrayInt& sel, bool bricks);
	void AddSelCoordArray(std::vector<unsigned int>& ids,
		std::vector<unsigned int>& bids, wxGridCellCoordsArray& sel, bool bricks);
	void UpdateSelectedRows();

private:
	//pages
	wxWindow* CreateCompGenPage(wxWindow *parent);
	wxWindow* CreateClusteringPage(wxWindow *parent);
	wxWindow* CreateAnalysisPage(wxWindow *parent);
	wxWindow* CreateOutputPage(wxWindow* parent);

	//common ops
	void OnShuffle(wxCommandEvent& event);
	void OnUseSelChk(wxCommandEvent& event);
	void OnUseMlChk(wxCommandEvent& event);
	void OnGenerate(wxCommandEvent& event);
	void OnCluster(wxCommandEvent& event);
	void OnAnalyze(wxCommandEvent& event);

	//stats text
	void OnIncludeBtn(wxCommandEvent& event);
	void OnExcludeBtn(wxCommandEvent& event);
	void OnHistoryChk(wxCommandEvent& event);
	void OnClearHistBtn(wxCommandEvent& event);

	//grid
	void OnKeyDown(wxKeyEvent& event);
	void OnSelectCell(wxGridEvent& event);
	void OnRangeSelect(wxGridRangeSelectEvent& event);

	//comp gen page
	void OnIterSldr(wxScrollEvent& event);
	void OnIterText(wxCommandEvent& event);
	void OnThreshSldr(wxScrollEvent& event);
	void OnThreshText(wxCommandEvent& event);
	//diff
	void OnDiffCheck(wxCommandEvent& event);
	void OnFalloffSldr(wxScrollEvent& event);
	void OnFalloffText(wxCommandEvent& event);
	//density
	void OnDensityCheck(wxCommandEvent& event);
	void OnDensitySldr(wxScrollEvent& event);
	void OnDensityText(wxCommandEvent& event);
	void OnVarthSldr(wxScrollEvent& event);
	void OnVarthText(wxCommandEvent& event);
	void OnDensityWindowSizeSldr(wxScrollEvent& event);
	void OnDensityWindowSizeText(wxCommandEvent& event);
	void OnDensityStatsSizeSldr(wxScrollEvent& event);
	void OnDensityStatsSizeText(wxCommandEvent& event);
	//dist field
	void OnUseDistFieldCheck(wxCommandEvent& event);
	void OnDistStrengthSldr(wxScrollEvent& event);
	void OnDistStrengthText(wxCommandEvent& event);
	void OnDistThreshSldr(wxScrollEvent& event);
	void OnDistThreshText(wxCommandEvent& event);
	void OnDistFilterSizeSldr(wxScrollEvent& event);
	void OnDistFilterSizeText(wxCommandEvent& event);
	void OnMaxDistSldr(wxScrollEvent& event);
	void OnMaxDistText(wxCommandEvent& event);
	//fixate
	void OnFixateCheck(wxCommandEvent& event);
	void OnGrowFixedCheck(wxCommandEvent& event);
	void OnFixUpdateBtn(wxCommandEvent& event);
	void OnFixSizeSldr(wxScrollEvent& event);
	void OnFixSizeText(wxCommandEvent& event);
	//clean
	void OnCleanCheck(wxCommandEvent& event);
	void OnCleanBtn(wxCommandEvent& event);
	void OnCleanIterSldr(wxScrollEvent& event);
	void OnCleanIterText(wxCommandEvent& event);
	void OnCleanLimitSldr(wxScrollEvent& event);
	void OnCleanLimitText(wxCommandEvent& event);
	//record
	void OnRecordCmd(wxCommandEvent& event);
	void OnPlayCmd(wxCommandEvent& event);
	void OnResetCmd(wxCommandEvent& event);
	void OnLoadCmd(wxCommandEvent& event);
	void OnSaveCmd(wxCommandEvent& event);
	//auto update
	void LaunchAutoUpdateTimer();
	void OnAutoUpdateTimer(wxTimerEvent& event);

	//clustering page
	void OnClusterMethodCheck(wxCommandEvent& event);
	//parameters
	void OnClusterClnumSldr(wxScrollEvent& event);
	void OnClusterClnumText(wxCommandEvent& event);
	void OnClusterMaxiterSldr(wxScrollEvent& event);
	void OnClusterMaxiterText(wxCommandEvent& event);
	void OnClusterTolSldr(wxScrollEvent& event);
	void OnClusterTolText(wxCommandEvent& event);
	void OnClusterSizeSldr(wxScrollEvent& event);
	void OnClusterSizeText(wxCommandEvent& event);
	void OnClusterEpsSldr(wxScrollEvent& event);
	void OnClusterepsText(wxCommandEvent& event);

	//analysis page
	//id
	void OnCompIdText(wxCommandEvent& event);
	void OnCompIdXBtn(wxCommandEvent& event);
	void OnAnalysisMinCheck(wxCommandEvent& event);
	void OnAnalysisMinSpin(wxSpinEvent& event);
	void OnAnalysisMinText(wxCommandEvent& event);
	void OnAnalysisMaxCheck(wxCommandEvent& event);
	void OnAnalysisMaxSpin(wxSpinEvent& event);
	void OnAnalysisMaxText(wxCommandEvent& event);
	//select
	void OnCompAppend(wxCommandEvent& event);
	void OnCompExclusive(wxCommandEvent& event);
	void OnCompAll(wxCommandEvent& event);
	void OnCompFull(wxCommandEvent& event);
	void OnCompClear(wxCommandEvent& event);
	//modify
	void OnCompNew(wxCommandEvent& event);
	void OnCompAdd(wxCommandEvent& event);
	void OnCompReplace(wxCommandEvent& event);
	void OnCompCleanBkg(wxCommandEvent& event);
	void OnCompCombine(wxCommandEvent& event);
	//options
	void OnConSizeSldr(wxScrollEvent& event);
	void OnConSizeText(wxCommandEvent& event);
	void OnConsistentCheck(wxCommandEvent& event);
	void OnColocalCheck(wxCommandEvent& event);
	//output
	void OnOutputTypeRadio(wxCommandEvent& event);
	void OnOutputChannels(wxCommandEvent& event);
	void OnOutputAnnotData(wxCommandEvent& event);
	void OnOutputMeshData(wxCommandEvent& event);
	//distance
	void OnDistNeighborCheck(wxCommandEvent& event);
	void OnDistAllChanCheck(wxCommandEvent& event);
	void OnDistNeighborSldr(wxScrollEvent& event);
	void OnDistNeighborText(wxCommandEvent& event);
	void OnDistOutput(wxCommandEvent& event);
	//align
	void OnAlignCenterChk(wxCommandEvent& event);
	void OnAlignPca(wxCommandEvent& event);
	//resize
	void OnSize(wxSizeEvent& event);
};

#endif//_COMPONENTDLG_H_
