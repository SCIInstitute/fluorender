/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2026 Scientific Computing and Imaging Institute,
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
#ifndef _SETTINGDLG_H_
#define _SETTINGDLG_H_

#include <PropPanel.h>
#include <wx/treectrl.h>
#include <vector>
#include <string>

class wxSingleSlider;
class MainFrame;
struct SaveProjectInfo
{
	bool prj_save;
	bool prj_save_inc;
	bool realtime_cmp;
	bool script_break;
	bool inverse_slider;
	bool mulfunc;
	bool config_file_type;
	bool y_dir;
};
struct StreamInfo
{
	int stream_rendering;
	int update_order;
	double graphics_mem;
	double large_data_size;
	int force_block_size;
	int up_time;
	int detail_level_offset;
};
struct AutomateInfo
{
	int histogram;
	int paint_size;
	int comp_gen;
	int colocalize;
	int relax_ruler;
	int conv_vol_mesh;
};
struct HologramInfo
{
	int hologram_mode;
	int xr_api;
	std::string holo_ip;
	bool mv_hmd;
	bool sbs;
	double eye_dist;
	double lg_offset;
	int hologram_debug;
	int hologram_camera_mode;
};
struct DeviceTreeInfo
{
	std::vector<std::vector<std::string>> tree;
	int platform_id{ -1 };
	int device_id{ -1 };
};
class SettingDlg : public TabbedPanel
{
public:
	SettingDlg(MainFrame* frame);
	~SettingDlg();

	//update
	void UpdateSaveProjectEnable(const SaveProjectInfo& info);
	void UpdateFontFile(const std::vector<std::string>& list);
	void UpdateSettingsFont(const std::string& str, int text_size, int text_color);
	void UpdateLineWidth(double dval);
	void UpdatePaintHistory(int ival);
	void UpdatePencilDist(double dval);
	void UpdateMicroBlendEnable(bool bval);
	void UpdatePeelNum(int ival);
	void UpdateSettingRot(double dval);
	void UpdateGradBg(bool bval);
	void UpdateClearColorBg(bool bval);
	void UpdateMouseInt(int ival);
	void UpdateStreamEable(const StreamInfo& info);
	void UpdateAutomate(const AutomateInfo& info);
	void UpdateHologramMode(const HologramInfo& info);
	void UpdateFullscreenDisplay(int ival);
	void UpdateDisplayColorDepth(int ival);
	void UpdateWavelengthColor(int ival1, int ival2, int ival3, int ival4);
	void UpdateMaxTextureSize(bool bval, int ival);
	void UpdateDeviceTree(const DeviceTreeInfo& info);
	void UpdateSettingsJava(const std::wstring& jvm, const std::wstring& ij, const std::wstring& bioformats, int ival);

private:
	//save project
	wxCheckBox* m_prj_save_chk;
	wxCheckBox* m_prj_save_inc_chk;
	//realtime compress
	wxCheckBox* m_realtime_cmp_chk;
	//script break
	wxCheckBox* m_script_break_chk;
	//inverse sliders
	wxCheckBox* m_inverse_slider_chk;
	//multifunc button use
	wxComboBox* m_mul_func_btn_comb;
	//project format
	wxComboBox* m_config_file_type_comb;
	//y direction
	wxComboBox* m_y_dir_comb;
	//mouse interations
	wxComboBox* m_mouse_int_comb;
	//depth peeling
	wxSingleSlider* m_peeling_layers_sldr;
	wxTextCtrl* m_peeling_layers_text;
	//micro blending
	wxCheckBox* m_micro_blend_chk;
	//background
	wxCheckBox* m_grad_bg_chk;
	wxCheckBox* m_clear_color_bg_chk;
	//rot center anchor
	wxSingleSlider* m_pin_threshold_sldr;
	wxTextCtrl* m_pin_threshold_text;
	//rotations link
	wxCheckBox* m_rot_link_chk;
	//vr
	wxCheckBox* m_stereo_chk;
	wxComboBox* m_xr_api_cmb;
	wxCheckBox* m_mv_hmd_chk;
	wxCheckBox* m_sbs_chk;
	wxSingleSlider* m_eye_dist_sldr;
	wxTextCtrl* m_eye_dist_text;
	wxTextCtrl* m_holo_ip_text;
	wxCheckBox* m_looking_glass_chk;
	wxSingleSlider* m_lg_offset_sldr;
	wxTextCtrl* m_lg_offset_text;
	wxComboBox* m_lg_quilt_cmb;
	wxComboBox* m_lg_camera_mode_cmb;
	//display
	wxComboBox* m_disp_id_comb;
	//color depth
	wxComboBox* m_color_depth_comb;
	//wavelength to color
	wxComboBox* m_wav_color1_cmb;
	wxComboBox* m_wav_color2_cmb;
	wxComboBox* m_wav_color3_cmb;
	wxComboBox* m_wav_color4_cmb;
	//texture size
	wxCheckBox* m_max_texture_size_chk;
	wxTextCtrl* m_max_texture_size_text;
	//memory settings
	wxComboBox* m_streaming_comb;
	wxComboBox* m_update_order_comb;
	wxSingleSlider* m_graphics_mem_sldr;
	wxTextCtrl* m_graphics_mem_text;
	wxSingleSlider* m_large_data_sldr;
	wxTextCtrl* m_large_data_text;
	wxSingleSlider* m_block_size_sldr;
	wxTextCtrl* m_block_size_text;
	wxSingleSlider* m_response_time_sldr;
	wxTextCtrl* m_response_time_text;
	wxSingleSlider* m_detail_level_offset_sldr;
	wxTextCtrl* m_detail_level_offset_text;
	//font
	wxComboBox* m_font_cmb;
	wxComboBox* m_font_size_cmb;
	wxComboBox* m_text_color_cmb;
	//line width
	wxSingleSlider* m_line_width_sldr;
	wxTextCtrl* m_line_width_text;
	//history depth
	wxSingleSlider* m_paint_hist_depth_sldr;
	wxTextCtrl* m_paint_hist_depth_text;
	//pencil distance
	wxSingleSlider* m_pencil_dist_sldr;
	wxTextCtrl* m_pencil_dist_text;
	//reset
	wxButton* m_reset_btn;
	wxButton* m_recommended_btn;

	// Java settings.
	wxTextCtrl* m_java_jvm_text;
	wxTextCtrl* m_java_ij_text;
	wxTextCtrl* m_java_bioformats_text;
	wxButton* m_browse_jvm_btn;
	wxButton* m_browse_ij_btn;
	wxButton* m_browse_bioformats_btn;
	wxRadioButton* mp_radio_button_imagej;
	wxRadioButton* mp_radio_button_fiji;

	//device tree
	wxTreeCtrl* m_device_tree;

	//automate
	struct ComboEntry
	{
		int id;
		wxString sid;
		wxString label;
		wxArrayString options;
		wxComboBox* combo;
	};
	std::unordered_map<std::string, ComboEntry> m_automate_combo;

private:
	wxWindow* CreateProjectPage(wxWindow* parent);
	wxWindow* CreateAutomationPage(wxWindow* parent);
	wxWindow* CreateRenderingPage(wxWindow* parent);
	wxWindow* CreatePerformancePage(wxWindow* parent);
	wxWindow* CreateDisplayPage(wxWindow* parent);
	wxWindow* CreateFormatPage(wxWindow* parent);
	wxWindow* CreateJavaPage(wxWindow* parent);

	//projec save
	void OnProjectSaveCheck(wxCommandEvent& event);
	void OnProjectSaveIncCheck(wxCommandEvent& event);
	void OnRealtimeCompressCheck(wxCommandEvent& event);
	void OnScriptBreakCheck(wxCommandEvent& event);
	void OnInverseSliderCheck(wxCommandEvent& event);
	void OnMulFuncBtnComb(wxCommandEvent& event);
	void OnConfigFileTypeComb(wxCommandEvent& event);
	void OnYDirComb(wxCommandEvent& event);
	//mouse interaction
	void OnMouseIntComb(wxCommandEvent& event);
	//peeling
	void OnPeelingLayersChange(wxScrollEvent& event);
	void OnPeelingLayersEdit(wxCommandEvent& event);
	//blending
	void OnMicroBlendCheck(wxCommandEvent& event);
	//background
	void OnGradBgCheck(wxCommandEvent& event);
	void OnClearColorBgCheck(wxCommandEvent& event);
	//rot center anchor threshold
	void OnPinThresholdChange(wxScrollEvent& event);
	void OnPinThresholdEdit(wxCommandEvent& event);
	//link rotations
	void OnRotLink(wxCommandEvent& event);
	//vr
	void OnStereoCheck(wxCommandEvent& event);
	void OnXrApiComb(wxCommandEvent& event);
	void OnMvHmdCheck(wxCommandEvent& event);
	void OnSBSCheck(wxCommandEvent& event);
	void OnEyeDistChange(wxScrollEvent& event);
	void OnEyeDistEdit(wxCommandEvent& event);
	void OnHoloIpEdit(wxCommandEvent& event);
	void OnLookingGlassCheck(wxCommandEvent& event);
	void OnLgOffsetChange(wxScrollEvent& event);
	void OnLgOffsetEdit(wxCommandEvent& event);
	void OnLgQuiltComb(wxCommandEvent& event);
	void OnLgCameraModeComb(wxCommandEvent& event);
	//display id
	void OnDispIdComb(wxCommandEvent& event);
	//color depth
	void OnColorDepthComb(wxCommandEvent& event);
	//wavelength color
	void OnWavColor1Change(wxCommandEvent& event);
	void OnWavColor2Change(wxCommandEvent& event);
	void OnWavColor3Change(wxCommandEvent& event);
	void OnWavColor4Change(wxCommandEvent& event);
	//texture size
	void OnMaxTextureSizeChk(wxCommandEvent& event);
	void OnMaxTextureSizeEdit(wxCommandEvent& event);
	//memory settings
	void OnStreamingComb(wxCommandEvent& event);
	void OnUpdateOrderChange(wxCommandEvent& event);
	void OnGraphicsMemChange(wxScrollEvent& event);
	void OnGraphicsMemEdit(wxCommandEvent& event);
	void OnLargeDataChange(wxScrollEvent& event);
	void OnLargeDataEdit(wxCommandEvent& event);
	void OnBlockSizeChange(wxScrollEvent& event);
	void OnBlockSizeEdit(wxCommandEvent& event);
	void OnResponseTimeChange(wxScrollEvent& event);
	void OnResponseTimeEdit(wxCommandEvent& event);
	void OnDetailLevelOffsetChange(wxScrollEvent& event);
	void OnDetailLevelOffsetEdit(wxCommandEvent& event);
	//font
	void OnFontChange(wxCommandEvent& event);
	void OnFontSizeChange(wxCommandEvent& event);
	void OnTextColorChange(wxCommandEvent& event);
	//line width
	void OnLineWidthSldr(wxScrollEvent& event);
	void OnLineWidthText(wxCommandEvent& event);
	//paint history depth
	void OnPaintHistDepthChange(wxScrollEvent& event);
	void OnPaintHistDepthEdit(wxCommandEvent& event);
	//pencil distance
	void OnPencilDistChange(wxScrollEvent& event);
	void OnPencilDistEdit(wxCommandEvent& event);
	// Java settings.
	void OnJavaJvmEdit(wxCommandEvent& event);
	void OnJavaIJEdit(wxCommandEvent& event);
	void OnJavaBioformatsEdit(wxCommandEvent& event);
	void onJavaJvmBrowse(wxCommandEvent& event);
	void onJavaIJBrowse(wxCommandEvent& event);
	void onJavaBioformatsBrowse(wxCommandEvent& event);
	void onJavaRadioButtonImageJ(wxCommandEvent& event);
	void onJavaRadioButtonFiji(wxCommandEvent& event);
	//device tree
	void OnSelChanged(wxTreeEvent& event);
	//automation
	void OnAutomationCombo(wxCommandEvent& event);
	//reset
	void OnResetBtn(wxCommandEvent& event);
	void OnRecommendedBtn(wxCommandEvent& event);
};

#endif//_SETTINGDLG_H_
