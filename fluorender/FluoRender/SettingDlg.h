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
#include <wx/wx.h>

#ifndef _SETTINGDLG_H_
#define _SETTINGDLG_H_

#define TOOL_PAINT_BRUSH	1
#define TOOL_MEASUREMENT	2
#define TOOL_TRACKING		3
#define TOOL_NOISE_REDUCTION	4
#define TOOL_VOLUME_SIZE	5
#define TOOL_COLOCALIZATION	6
#define TOOL_CONVERT		7
#define TOOL_OPENCL			8
#define TOOL_COMPONENT		9
#define TOOL_CALCULATIONS	10

//enum BitmapFontType;
class SettingDlg : public wxPanel
{
	enum
	{
		//project save
		ID_PrjSaveChk = ID_SETTING,
		//real time compress
		ID_RealtimeCmpChk,
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
		//paint history depth
		ID_PaintHistDepthSldr,
		ID_PaintHistDepthText,
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
		ID_RadioButtonFiji
	};

public:
	SettingDlg(wxWindow* frame,
		wxWindow* parent);
	~SettingDlg();

	void GetSettings();
	void UpdateUI();
	void SaveSettings();
	void UpdateTextureSize();

	//get settings from ui
	int GetGMCMode() {return m_gmc_mode;}
	bool GetProjSave() {return m_prj_save;}
	bool GetSaveAlpha() { return m_save_alpha; }
	void SetSaveAlpha(bool val) { m_save_alpha = val; }
	bool GetSaveFloat() { return m_save_float; }
	void SetSaveFloat(bool val) { m_save_float = val; }
	bool GetRealtimeCompress() {return m_realtime_compress;}
	void SetRealtimeCompress(bool val) {m_realtime_compress = val;}
	bool GetSkipBricks() {return m_skip_bricks;}
	void SetSkipBricks(bool val) {m_skip_bricks = val;}
	bool GetTestMode(int type);	//type	1:speed test
								//		2:parameter test
								//		3:wireframe mode
	int GetPeelingLyers();
	bool GetMicroBlend();
	void GetShadowDir(double& x, double &y);
	bool GetMouseInt() {return m_mouse_int;}
	int GetWavelengthColor(int n);
	wxString GetTimeId() {return m_time_id;}
	void SetTimeId(wxString id) {m_time_id = id;}
	//gradient background
	bool GetGradBg() {return m_grad_bg;}
	void SetGradBg(bool val) {m_grad_bg = val;}
	//rot center anchor thresh
	double GetPinThreshold() { return m_pin_threshold; }
	//override vox
	bool GetOverrideVox() {return m_override_vox;}
	void SetOverrideVox(bool val) {m_override_vox = val;}
	//soft threshold
	double GetSoftThreshold() {return m_soft_threshold;}
	void SetSoftThreshold(double val) {m_soft_threshold = val;}
	//run script
	bool GetRunScript() {return m_run_script;}
	void SetRunScript(bool val) {m_run_script = val;}
	wxString GetScriptFile() { return m_script_file; }
	void SetScriptFile(wxString &file) { m_script_file = file; }
	//paint history depth
	int GetPaintHistDepth() {return m_paint_hist_depth;}
	void SetPaintHistDepth(int val) {m_paint_hist_depth = val;}
	//text font
	wxString GetFontFile() { return m_font_file; }
	void SetFontFile(wxString &file) { m_font_file = file; }
	int GetTextSize() {return m_text_size;}
	void SetTextSize(int size) {m_text_size = size;}
	int GetTextColor() { return m_text_color; }
	void SetTextColor(int text_color) { m_text_color = text_color; }
	//full screen
	bool GetStayTop() { return m_stay_top; }
	bool GetShowCursor() { return m_show_cursor; }
	//memory settings
	bool GetMemSwap() {return m_mem_swap;}
	void SetMemSwap(bool val) {m_mem_swap = val;}
	double GetGraphicsMem() {return m_graphics_mem;}
	void SetGraphicsMem(double val) {m_graphics_mem = val;}
	double GetLargeDataSize() {return m_large_data_size;}
	void SetLargeDataSize(double val) {m_large_data_size = val;}
	int GetForceBrickSize() {return m_force_brick_size;}
	void SetForceBrickSize(int val) {m_force_brick_size = val;}
	int GetResponseTime() {return m_up_time;}
	void SetResponseTime(int val) {m_up_time = val;}
	int GetUpdateOrder() {return m_mem_swap?m_update_order:0;}
	void SetUpdateOrder(int val) {m_update_order = val;}
	bool GetInvalidateTex() { return m_invalidate_tex; }
	void SetInvalidateTex(bool val) { m_invalidate_tex = val; }
	int GetDetailLevelOffset() { return m_detail_level_offset; }
	void SetDetailLevelOffset(int val) { m_detail_level_offset = val; }
	//point volume mode
	int GetPointVolumeMode() {return m_point_volume_mode;}
	void SetPointVolumeMode(int mode) {m_point_volume_mode = mode;}
	//ruler use transfer function
	bool GetRulerUseTransf() {return m_ruler_use_transf;}
	void SetRulerUseTransf(bool val) {m_ruler_use_transf = val;}
	//ruler time dependent
	bool GetRulerTimeDep() {return m_ruler_time_dep;}
	void SetRulerTimeDep(bool val) {m_ruler_time_dep = val;}
	//ruler exports df/f
	bool GetRulerDF_F() { return m_ruler_df_f; }
	void SetRulerDF_F(bool val) { m_ruler_df_f = val; }
	//flags for pvxml flipping
	bool GetPvxmlFlipX() {return m_pvxml_flip_x;}
	void SetPvxmlFlipX(bool flip) {m_pvxml_flip_x = flip;}
	bool GetPvxmlFlipY() {return m_pvxml_flip_y;}
	void SetPvxmlFlipY(bool flip) {m_pvxml_flip_y = flip;}
	//pixel format
	int GetApiType() { return m_api_type; }
	int GetRedBit() {return m_red_bit;}
	int GetGreenBit() {return m_green_bit;}
	int GetBlueBit() {return m_blue_bit;}
	int GetAlphaBit() {return m_alpha_bit;}
	int GetDepthBit() {return m_depth_bit;}
	int GetSamples() {return m_samples;}
	//context attrib
	int GetGLMajorVer() {return m_gl_major_ver;}
	int GetGLMinorVer() {return m_gl_minor_ver;}
	int GetGLProfileMask() {return m_gl_profile_mask;}
	//cl device
	int GetCLPlatformID() { return m_cl_platform_id; }
	int GetCLDeviceID() {return m_cl_device_id;}
	//last tool
	void SetLastTool(int tool) { m_last_tool = tool; }
	int GetLastTool() { return m_last_tool; }
	//component size
	void SetComponentSize(double size) { m_component_size = size; }
	double GetComponentSize() { return m_component_size; }
	void SetContactFactor(double fact) { m_contact_factor = fact; }
	double GetContactFactor() { return m_contact_factor; }
	void SetSimilarity(double siml) { m_similarity = siml; }
	double GetSimilarity() { return m_similarity; }
	//texture size
	void SetUseMaxtextureSize(bool val) { m_use_max_texture_size = val; }
	bool GetUseMaxTextureSize() { return m_use_max_texture_size; }
	void SetMaxTextureSize(int size) { m_max_texture_size = size; }
	int GetMaxTextureSize() { return m_max_texture_size; }
	int GetPlaneMode() { return m_plane_mode; }
	
	//Getting the java paths.
	wxString getJVMPath();
	wxString getIJPath();
	wxString getBioformatsPath();
	std::vector<std::string> GetJvmArgs();

	bool getIJMode() { return m_ij_mode; }

private:
	wxWindow* m_frame;

	int m_gmc_mode;			//1-pre-calculated (removed);
							//2-real-time 7 sample;
							//3-real-time 4 samples (removed);
							//4-pre-calculated 4 samples (removed);
	bool m_prj_save;		//save project automatically
	bool m_save_alpha;		//save alpha channel in captured images
	bool m_save_float;		//save float values in captured images
	bool m_realtime_compress;//real time compress
	bool m_skip_bricks;		//skip empty bricks
	bool m_test_speed;		//test fps
	bool m_test_param;		//using parameter test window
	bool m_test_wiref;		//draw wireframe of volumes
	int m_peeling_layers;	//peeling layer number
	bool m_micro_blend;		//blending slice in depth mode
	bool m_shadow_dir;		//enable directional shaow
	double m_shadow_dir_x;	//x comp of shadow direction
	double m_shadow_dir_y;	//y comp of shadow direction
	bool m_mouse_int;		//enable lower sample rate for mouse interactions
	int m_wav_color1;		//wavelength to color
	int m_wav_color2;		//1-red; 2-green; 3-blue; 4-purple; 5-white
	int m_wav_color3;
	int m_wav_color4;
	wxString m_time_id;		//identfier for time sequence
	bool m_grad_bg;
	bool m_override_vox;
	double m_soft_threshold;
	//script
	bool m_run_script;
	wxString m_script_file;
	//text size
	wxString m_font_file;	//font lib file in the Fonts folder
	int m_text_size;		//text size in viewport
	int m_text_color;		//text color: 0- contrast to bg; 1-same as bg; 2-volume sec color
	//memory limit
	bool m_mem_swap;		//enable memory swap
	double m_graphics_mem;	//in MB
							//it's the user setting
							//final value is determined by both reading from the card and this value
	double m_large_data_size;//data size considered as large and needs forced bricking
	int m_force_brick_size;	//in pixels
							//it's the user setting
							//final value is determined by both reading from the card and this value
	int m_up_time;			//response time in ms
	int m_update_order;		//0:back-to-front; 1:front-to-back
	bool m_invalidate_tex;	//invalidate texture in every loop
	int m_detail_level_offset;	//an offset value to current level of detail (for multiresolution data only)
	//point volume mode
	int m_point_volume_mode;
	//ruler use transfer function
	bool m_ruler_use_transf;
	//ruler time dependent
	bool m_ruler_time_dep;
	//ruler exports df/f
	bool m_ruler_df_f;
	//flip pvxml frame
	bool m_pvxml_flip_x;
	bool m_pvxml_flip_y;
	//pixel format
	int m_api_type;//0-default; 1-amd; 2-nv
	int m_red_bit;
	int m_green_bit;
	int m_blue_bit;
	int m_alpha_bit;
	int m_depth_bit;
	int m_samples;
	//context attrib
	int m_gl_major_ver;
	int m_gl_minor_ver;
	int m_gl_profile_mask;
	//cl device
	int m_cl_platform_id;
	int m_cl_device_id;
	//paint history depth
	int m_paint_hist_depth;
	//full screen
	bool m_stay_top;
	bool m_show_cursor;
	//last tool
	int m_last_tool;
	//component size
	double m_component_size;
	double m_contact_factor;
	double m_similarity;
	//max texture size
	bool m_use_max_texture_size;
	int m_max_texture_size;
	//rot center anchor thresh
	double m_pin_threshold;
	//clipping plane display mode
	int m_plane_mode;

	// java settings strings.
	wxString m_jvm_path;
	wxString m_ij_path;
	wxString m_bioformats_path;
	int m_ij_mode;//0: imagej; 1: fiji

private:
	//save project
	wxCheckBox *m_prj_save_chk;
	//realtime compress
	wxCheckBox *m_realtime_cmp_chk;
	//mouse interations
	wxCheckBox *m_mouse_int_chk;
	//depth peeling
	wxSlider *m_peeling_layers_sldr;
	wxTextCtrl *m_peeling_layers_text;
	//micro blending
	wxCheckBox *m_micro_blend_chk;
	//shaodw direction
	wxCheckBox *m_shadow_dir_chk;
	wxSlider *m_shadow_dir_sldr;
	wxTextCtrl *m_shadow_dir_text;
	//gradient background
	wxCheckBox *m_grad_bg_chk;
	//rot center anchor
	wxSlider *m_pin_threshold_sldr;
	wxTextCtrl *m_pin_threshold_text;
	//rotations link
	wxCheckBox *m_rot_link_chk;
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
	wxSlider *m_graphics_mem_sldr;
	wxTextCtrl *m_graphics_mem_text;
	wxSlider *m_large_data_sldr;
	wxTextCtrl *m_large_data_text;
	wxSlider *m_block_size_sldr;
	wxTextCtrl *m_block_size_text;
	wxSlider *m_response_time_sldr;
	wxTextCtrl *m_response_time_text;
	wxSlider *m_detail_level_offset_sldr;
	wxTextCtrl *m_detail_level_offset_text;
	//font
	wxComboBox *m_font_cmb;
	wxComboBox *m_font_size_cmb;
	wxComboBox *m_text_color_cmb;
	//history depth
	wxSlider *m_paint_hist_depth_sldr;
	wxTextCtrl *m_paint_hist_depth_text;

	// Java settings.
	wxTextCtrl *m_java_jvm_text;
	wxTextCtrl *m_java_ij_text;
	wxTextCtrl *m_java_bioformats_text;
	wxButton* m_browse_jvm_btn;
	wxButton* m_browse_ij_btn;
	wxButton* m_browse_bioformats_btn;
	wxRadioButton* mp_radio_button_imagej;
	wxRadioButton* mp_radio_button_fiji;

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
	wxWindow* CreatePerformancePage(wxWindow *parent);
	wxWindow* CreateRenderingPage(wxWindow *parent);
	wxWindow* CreateFormatPage(wxWindow *parent);
	wxWindow* CreateJavaPage(wxWindow *parent);

	//projec save
	void OnProjectSaveCheck(wxCommandEvent &event);
	void OnRealtimeCompressCheck(wxCommandEvent &event);
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
	//paint history depth
	void OnPaintHistDepthChange(wxScrollEvent &event);
	void OnPaintHistDepthEdit(wxCommandEvent &event);
	// Java settings.
	void OnJavaJvmEdit(wxCommandEvent &event);
	void OnJavaIJEdit(wxCommandEvent &event);
	void OnJavaBioformatsEdit(wxCommandEvent &event);
	void onJavaJvmBrowse(wxCommandEvent &event);
	void onJavaIJBrowse(wxCommandEvent &event);
	void onJavaBioformatsBrowse(wxCommandEvent &event);
	void onJavaRadioButtonImageJ(wxCommandEvent &event);
	void onJavaRadioButtonFiji(wxCommandEvent &event);

	DECLARE_EVENT_TABLE()
};

#endif//_SETTINGDLG_H_
