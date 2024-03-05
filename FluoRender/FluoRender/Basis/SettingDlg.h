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
#ifndef _SETTINGDLG_H_
#define _SETTINGDLG_H_

#include <wx/wx.h>
#include <wx/treectrl.h>
#include <vector>

//enum BitmapFontType;
class VRenderFrame;
class wxSingleSlider;
class SettingDlg : public wxPanel
{
	enum
	{
		//project save
		ID_PrjSaveChk = ID_SETTING,
		ID_PrjSaveIncChk,
		//real time compress
		ID_RealtimeCmpChk,
		//allow script break
		ID_ScriptBreakChk,
		//inverse vertical sliders
		ID_InverseSliderChk,
		//multi function button
		ID_MulFuncBtnComb,
		//mouse interactions
		ID_MouseIntChk,
		//depth peeling
		ID_PeelingLayersSldr,
		ID_PeelingLayersText,
		//micro blending
		ID_MicroBlendChk,
		//shadow
		ID_ShadowDirChk,
		ID_ShadowDirSldr,
		ID_ShadowDirText,
		//gradient background
		ID_GradBgChk,
		//auto rot center anchor threshold
		ID_PinThreshSldr,
		ID_PinThreshText,
		//link rotations
		ID_RotLinkChk,
		//stereo
		ID_StereoChk,
		ID_SBSChk,
		ID_EyeDistSldr,
		ID_EyeDistText,
		//display id
		ID_DispIdSldr,
		ID_DispIdText,
		//override vox
		ID_OverrideVoxChk,
		//component size
		ID_ComponentSizeText,
		ID_ContactFactorText,
		//wavelength to color
		ID_WavColor1Cmb,
		ID_WavColor2Cmb,
		ID_WavColor3Cmb,
		ID_WavColor4Cmb,
		//memory settings
		ID_StreamingChk,
		ID_UpdateOrderRbox,
		ID_GraphicsMemSldr,
		ID_GraphicsMemText,
		ID_LargeDataSldr,
		ID_LargeDataText,
		ID_BlockSizeSldr,
		ID_BlockSizeText,
		ID_ResponseTimeSldr,
		ID_ResponseTimeText,
		ID_DetailLevelOffsetSldr,
		ID_DetailLevelOffsetText,
		//texture size
		ID_MaxTextureSizeChk,
		ID_MaxTextureSizeText,
		//font
		ID_FontCmb,
		ID_FontSizeCmb,
		ID_TextColorCmb,
		//line width
		ID_LineWidthSldr,
		ID_LineWidthText,
		//paint history depth
		ID_PaintHistDepthSldr,
		ID_PaintHistDepthText,
		//pencil distance
		ID_PencilDistSldr,
		ID_PencilDistText,
		//save/close
		ID_SaveBtn,
		ID_CloseBtn,
		//Java settings
		ID_JavaJVMText,
		ID_JavaIJText,
		ID_JavaBioformatsText,
		ID_JavaJvmBrowseBtn,
		ID_JavaIJBrowseBtn,
		ID_JavaBioformatsBrowseBtn,
		ID_RadioButtonImageJ,
		ID_RadioButtonFiji,
		//device tree
		ID_DeviceTree
	};

public:
	SettingDlg(VRenderFrame* frame);
	~SettingDlg();

	void UpdateUI();
	void UpdateDeviceTree();
	void UpdateTextureSize();

private:
	VRenderFrame* m_frame;

private:
	//save project
	wxCheckBox *m_prj_save_chk;
	wxCheckBox* m_prj_save_inc_chk;
	//realtime compress
	wxCheckBox *m_realtime_cmp_chk;
	//script break
	wxCheckBox *m_script_break_chk;
	//inverse sliders
	wxCheckBox* m_inverse_slider_chk;
	//multifunc button use
	wxComboBox* m_mul_func_btn_comb;
	//mouse interations
	wxCheckBox *m_mouse_int_chk;
	//depth peeling
	wxSingleSlider *m_peeling_layers_sldr;
	wxTextCtrl *m_peeling_layers_text;
	//micro blending
	wxCheckBox *m_micro_blend_chk;
	//shaodw direction
	wxCheckBox *m_shadow_dir_chk;
	wxSingleSlider *m_shadow_dir_sldr;
	wxTextCtrl *m_shadow_dir_text;
	//gradient background
	wxCheckBox *m_grad_bg_chk;
	//rot center anchor
	wxSingleSlider *m_pin_threshold_sldr;
	wxTextCtrl *m_pin_threshold_text;
	//rotations link
	wxCheckBox *m_rot_link_chk;
	//stereo
	wxCheckBox *m_stereo_chk;
	wxCheckBox* m_sbs_chk;
	wxSingleSlider *m_eye_dist_sldr;
	wxTextCtrl *m_eye_dist_text;
	//display
	wxSingleSlider* m_disp_id_sldr;
	wxTextCtrl* m_disp_id_text;
	//override vox
	wxCheckBox *m_override_vox_chk;
	//wavelength to color
	wxComboBox *m_wav_color1_cmb;
	wxComboBox *m_wav_color2_cmb;
	wxComboBox *m_wav_color3_cmb;
	wxComboBox *m_wav_color4_cmb;
	//texture size
	wxCheckBox *m_max_texture_size_chk;
	wxTextCtrl *m_max_texture_size_text;
	//memory settings
	wxCheckBox *m_streaming_chk;
	wxRadioBox *m_update_order_rbox;
	wxSingleSlider *m_graphics_mem_sldr;
	wxTextCtrl *m_graphics_mem_text;
	wxSingleSlider *m_large_data_sldr;
	wxTextCtrl *m_large_data_text;
	wxSingleSlider *m_block_size_sldr;
	wxTextCtrl *m_block_size_text;
	wxSingleSlider *m_response_time_sldr;
	wxTextCtrl *m_response_time_text;
	wxSingleSlider *m_detail_level_offset_sldr;
	wxTextCtrl *m_detail_level_offset_text;
	//font
	wxComboBox *m_font_cmb;
	wxComboBox *m_font_size_cmb;
	wxComboBox *m_text_color_cmb;
	//line width
	wxSingleSlider *m_line_width_sldr;
	wxTextCtrl *m_line_width_text;
	//history depth
	wxSingleSlider *m_paint_hist_depth_sldr;
	wxTextCtrl *m_paint_hist_depth_text;
	//pencil distance
	wxSingleSlider* m_pencil_dist_sldr;
	wxTextCtrl* m_pencil_dist_text;

	// Java settings.
	wxTextCtrl *m_java_jvm_text;
	wxTextCtrl *m_java_ij_text;
	wxTextCtrl *m_java_bioformats_text;
	wxButton* m_browse_jvm_btn;
	wxButton* m_browse_ij_btn;
	wxButton* m_browse_bioformats_btn;
	wxRadioButton* mp_radio_button_imagej;
	wxRadioButton* mp_radio_button_fiji;

	//device tree
	wxTreeCtrl* m_device_tree;

	//save
	wxButton *m_save_btn;
	//close
	wxButton *m_close_btn;

private:
	//streaming disable/enable
	void EnableStreaming(bool enable);

	//shadow direction
	void SetShadowDir(double deg);
	double GetShadowDir();

	//event handling
	void OnSave(wxCommandEvent &event);
	void OnClose(wxCommandEvent &event);
	void OnShow(wxShowEvent &event);

	wxWindow* CreateProjectPage(wxWindow *parent);
	wxWindow* CreateRenderingPage(wxWindow *parent);
	wxWindow* CreatePerformancePage(wxWindow *parent);
	wxWindow* CreateDisplayPage(wxWindow* parent);
	wxWindow* CreateFormatPage(wxWindow *parent);
	wxWindow* CreateJavaPage(wxWindow *parent);

	//projec save
	void OnProjectSaveCheck(wxCommandEvent &event);
	void OnProjectSaveIncCheck(wxCommandEvent& event);
	void OnRealtimeCompressCheck(wxCommandEvent &event);
	void OnScriptBreakCheck(wxCommandEvent& event);
	void OnInverseSliderCheck(wxCommandEvent& event);
	void OnMulFuncBtnComb(wxCommandEvent& event);
	//mouse interaction
	void OnMouseIntCheck(wxCommandEvent &event);
	//peeling
	void OnPeelingLayersChange(wxScrollEvent &event);
	void OnPeelingLayersEdit(wxCommandEvent &event);
	//blending
	void OnMicroBlendCheck(wxCommandEvent &event);
	//shadow
	void OnShadowDirCheck(wxCommandEvent &event);
	void OnShadowDirChange(wxScrollEvent &event);
	void OnShadowDirEdit(wxCommandEvent &event);
	//gradient background
	void OnGradBgCheck(wxCommandEvent &event);
	//rot center anchor threshold
	void OnPinThresholdChange(wxScrollEvent &event);
	void OnPinThresholdEdit(wxCommandEvent &event);
	//link rotations
	void OnRotLink(wxCommandEvent& event);
	//stereo
	void OnStereoCheck(wxCommandEvent &event);
	void OnSBSCheck(wxCommandEvent& event);
	void OnEyeDistChange(wxScrollEvent &event);
	void OnEyeDistEdit(wxCommandEvent &event);
	//display id
	void OnDispIdChange(wxScrollEvent& event);
	void OnDispIdEdit(wxCommandEvent& event);
	//override vox
	void OnOverrideVoxCheck(wxCommandEvent &event);
	//wavelength color
	void OnWavColor1Change(wxCommandEvent &event);
	void OnWavColor2Change(wxCommandEvent &event);
	void OnWavColor3Change(wxCommandEvent &event);
	void OnWavColor4Change(wxCommandEvent &event);
	//texture size
	void OnMaxTextureSizeChk(wxCommandEvent &event);
	void OnMaxTextureSizeEdit(wxCommandEvent &event);
	//memory settings
	void OnStreamingChk(wxCommandEvent &event);
	void OnUpdateOrderChange(wxCommandEvent & event);
	void OnGraphicsMemChange(wxScrollEvent &event);
	void OnGraphicsMemEdit(wxCommandEvent &event);
	void OnLargeDataChange(wxScrollEvent &event);
	void OnLargeDataEdit(wxCommandEvent &event);
	void OnBlockSizeChange(wxScrollEvent &event);
	void OnBlockSizeEdit(wxCommandEvent &event);
	void OnResponseTimeChange(wxScrollEvent &event);
	void OnResponseTimeEdit(wxCommandEvent &event);
	void OnDetailLevelOffsetChange(wxScrollEvent &event);
	void OnDetailLevelOffsetEdit(wxCommandEvent &event);
	//font
	void OnFontChange(wxCommandEvent &event);
	void OnFontSizeChange(wxCommandEvent &event);
	void OnTextColorChange(wxCommandEvent &event);
	//line width
	void OnLineWidthSldr(wxScrollEvent &event);
	void OnLineWidthText(wxCommandEvent &event);
	//paint history depth
	void OnPaintHistDepthChange(wxScrollEvent &event);
	void OnPaintHistDepthEdit(wxCommandEvent &event);
	//pencil distance
	void OnPencilDistChange(wxScrollEvent& event);
	void OnPencilDistEdit(wxCommandEvent& event);
	// Java settings.
	void OnJavaJvmEdit(wxCommandEvent &event);
	void OnJavaIJEdit(wxCommandEvent &event);
	void OnJavaBioformatsEdit(wxCommandEvent &event);
	void onJavaJvmBrowse(wxCommandEvent &event);
	void onJavaIJBrowse(wxCommandEvent &event);
	void onJavaBioformatsBrowse(wxCommandEvent &event);
	void onJavaRadioButtonImageJ(wxCommandEvent &event);
	void onJavaRadioButtonFiji(wxCommandEvent &event);
	//device tree
	void OnSelChanged(wxTreeEvent& event);

	DECLARE_EVENT_TABLE()
};

#endif//_SETTINGDLG_H_
