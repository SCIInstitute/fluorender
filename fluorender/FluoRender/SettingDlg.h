/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2014 Scientific Computing and Imaging Institute,
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

#define SETTING_FILE_NAME "fluorender.set"

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
		ID_PrjSaveChk = wxID_HIGHEST+1201,
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
		//font
		ID_FontCmb,
		ID_FontSizeCmb,
		ID_TextColorCmb,
		//script
		ID_RunScriptChk,
		ID_ScriptFileText,
		ID_ScriptFileBtn,
		//paint history depth
		ID_PaintHistDepthSldr,
		ID_PaintHistDepthText,
		//save/close
		ID_SaveBtn,
		ID_CloseBtn
	};

public:
	SettingDlg(wxWindow* frame,
		wxWindow* parent);
	~SettingDlg();

	void GetSettings();
	void UpdateUI();
	void SaveSettings();

	//get settings from ui
	int GetGMCMode() {return m_gmc_mode;}
	bool GetProjSave() {return m_prj_save;}
	bool GetSaveAlpha() { return m_save_alpha; }
	void SetSaveAlpha(bool val) { m_save_alpha = val; }
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
	//point volume mode
	int GetPointVolumeMode() {return m_point_volume_mode;}
	void SetPointVolumeMode(int mode) {m_point_volume_mode = mode;}
	//ruler use transfer function
	bool GetRulerUseTransf() {return m_ruler_use_transf;}
	void SetRulerUseTransf(bool val) {m_ruler_use_transf = val;}
	//ruler time dependent
	bool GetRulerTimeDep() {return m_ruler_time_dep;}
	void SetRulerTimeDep(bool val) {m_ruler_time_dep = val;}
	//flags for pvxml flipping
	bool GetPvxmlFlipX() {return m_pvxml_flip_x;}
	void SetPvxmlFlipX(bool flip) {m_pvxml_flip_x = flip;}
	bool GetPvxmlFlipY() {return m_pvxml_flip_y;}
	void SetPvxmlFlipY(bool flip) {m_pvxml_flip_y = flip;}
	//pixel format
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
	int GetCLDeviceID() {return m_cl_device_id;}
	//last tool
	void SetLastTool(int tool) { m_last_tool = tool; }
	int GetLastTool() { return m_last_tool; }
	//component size
	void SetComponentSize(double size) { m_component_size = size; }
	double GetComponentSize() { return m_component_size; }
	void SetContactFactor(double fact) { m_contact_factor = fact; }
	double GetContactFactor() { return m_contact_factor; }

private:
	wxWindow* m_frame;

	int m_gmc_mode;			//1-pre-calculated (removed);
							//2-real-time 7 sample;
							//3-real-time 4 samples (removed);
							//4-pre-calculated 4 samples (removed);
	bool m_prj_save;		//save project automatically
	bool m_save_alpha;		//save alpha channel in captured images
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
	//point volume mode
	int m_point_volume_mode;
	//ruler use transfer function
	bool m_ruler_use_transf;
	//ruler time dependent
	bool m_ruler_time_dep;
	//flip pvxml frame
	bool m_pvxml_flip_x;
	bool m_pvxml_flip_y;
	//pixel format
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
	//rotations link
	
	wxCheckBox *m_rot_link_chk;
	//override vox
	wxCheckBox *m_override_vox_chk;
	//wavelength to color
	wxComboBox *m_wav_color1_cmb;
	wxComboBox *m_wav_color2_cmb;
	wxComboBox *m_wav_color3_cmb;
	wxComboBox *m_wav_color4_cmb;
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
	//font
	wxComboBox *m_font_cmb;
	wxComboBox *m_font_size_cmb;
	wxComboBox *m_text_color_cmb;
	//script
	wxCheckBox *m_run_script_chk;
	wxTextCtrl *m_script_file_text;
	wxButton *m_script_file_btn;
	//history depth
	wxSlider *m_paint_hist_depth_sldr;
	wxTextCtrl *m_paint_hist_depth_text;
	//component size
	wxTextCtrl *m_component_size_text;
	wxTextCtrl *m_contact_factor_text;

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
	//link rotations
	void OnRotLink(wxCommandEvent& event);
	//override vox
	void OnOverrideVoxCheck(wxCommandEvent &event);
	//wavelength color
	void OnWavColor1Change(wxCommandEvent &event);
	void OnWavColor2Change(wxCommandEvent &event);
	void OnWavColor3Change(wxCommandEvent &event);
	void OnWavColor4Change(wxCommandEvent &event);
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
	//font
	void OnFontChange(wxCommandEvent &event);
	void OnFontSizeChange(wxCommandEvent &event);
	void OnTextColorChange(wxCommandEvent &event);
	//script
	void OnRunScriptChk(wxCommandEvent &event);
	void OnScriptFileEdit(wxCommandEvent &event);
	void OnScriptFileBtn(wxCommandEvent &event);
	//paint history depth
	void OnPaintHistDepthChange(wxScrollEvent &event);
	void OnPaintHistDepthEdit(wxCommandEvent &event);
	//component size
	void OnComponentSizeEdit(wxCommandEvent &event);
	void OnContactFactorEdit(wxCommandEvent &event);

	DECLARE_EVENT_TABLE();
};

#endif//_SETTINGDLG_H_
