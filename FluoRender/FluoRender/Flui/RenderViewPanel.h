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

#ifndef _RENDERVIEWPANEL_H_
#define _RENDERVIEWPANEL_H_

#include <AgentOwner.h>
#include <wx/clrpicker.h>
#include <wx/spinbutt.h>
#include <wx/filedlgcustomize.h>

struct CaptureOptions
{
	bool compress = false;

	bool saveAlpha = false;
	bool saveFloat = false;

	int dpi = 72;

	bool enlarge = false;
	double enlargeScale = 1.0;

	bool embedFiles = false;
};

class CaptureHook :
	public wxFileDialogCustomizeHook
{
public:
	CaptureHook(
		const CaptureOptions& options);

	void AddCustomControls(
		wxFileDialogCustomize& customizer) override;

	void TransferDataFromCustomControls() override;

	const CaptureOptions& GetOptions() const
	{
		return m_options;
	}

private:
	CaptureOptions m_options;

	wxFileDialogCheckBox* m_compressChk = nullptr;

	wxFileDialogCheckBox* m_alphaChk = nullptr;
	wxFileDialogCheckBox* m_floatChk = nullptr;

	wxFileDialogTextCtrl* m_dpiTxt = nullptr;

	wxFileDialogCheckBox* m_enlargeChk = nullptr;
	wxFileDialogTextCtrl* m_enlargeTxt = nullptr;

	wxFileDialogCheckBox* m_embedChk = nullptr;
};

class RenderCanvas;
class wxGLContext;
class wxSingleSlider;
class wxUndoableScrollBar;
class wxUndoableToolbar;
class wxUndoableColorPicker;
class wxNumTextCtrl;
enum class ChannelMixMode : int;
class RenderViewPanel: public AgentPanel
{
public:
	enum
	{
		ID_ChannelMixLayered = 0,
		ID_ChannelMixDepth,
		ID_ChannelMixCompositeAdd
	};
	enum
	{
		ID_InfoChk = 0,
		ID_CamCtrChk,
		ID_LegendChk,
		ID_Colormap,
		ID_ScaleBar
	};
	enum
	{
		ID_OrthoPerspBtn = 0,
		ID_CamModeBtn,
		ID_DefaultBtn
	};
	enum
	{
		ID_VrChk = 0,
		ID_LookingGlassChk,
		ID_FullScreenBtn
	};
	enum
	{
		ID_RotXScroll = 0,
		ID_RotYScroll,
		ID_RotZScroll
	};
	enum
	{
		ID_RotLockChk = 0,
		ID_ZeroRotBtn,
		ID_RotResetBtn
	};
	enum
	{
		ID_LZW_COMP,
		ID_SAVE_ALPHA,
		ID_SAVE_FLOAT,
		ID_DPI,
		ID_ENLARGE_CHK,
		ID_ENLARGE_SLDR,
		ID_ENLARGE_TEXT,
		ID_EMBED_FILES
	};

	RenderViewPanel(wxWindow* parent,
		wxGLContext* sharedContext=0,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0,
		const wxString& name = "RenderView");
	~RenderViewPanel();

	void SetRenderCanvas(RenderCanvas* canvas) { m_canvas = canvas; }
	RenderCanvas* GetRenderCanvas() { return m_canvas; }
	void SetFullFrame(wxFrame* frame) { m_full_frame = frame; }
	wxFrame* GetFullFrame() { return m_full_frame; }
	wxBoxSizer* GetViewSizer() { return m_view_sizer; }

	//update
	void UpdateMixMethod(ChannelMixMode mode);
	void UpdateDrawInfo(bool bval);
	void UpdateDrawCamCtr(bool bval);
	void UpdateDrawLegend(bool bval);
	void UpdateDrawColormap(int ival);
	void UpdateDrawScalebar(int ival);
	void UpdateScaleBarValue(double dval);
	void UpdateScaleBarUnit(int ival);
	void UpdateBgColor(const fluo::Color& c);
	void UpdateBgColorInvert(bool bval);
	void UpdateAov(int ival, bool bval);
	void UpdateCamMode(int ival);
	void UpdateHologramMode(int ival);
	void UpdateFreehandToolState(bool bval);
	
	void UpdateDepthAtten(bool bval);
	void UpdateDepthAttenFactor(double dval);

	void UpdateScaleFactor(int ival);
	void UpdateScaleMode(int ival);
	void UpdatePinRotCenter(bool bval);

	void UpdateGearedEnable(bool bval);
	void UpdateRotSliderMode(bool bval);
	void UpdateCamRotation(const fluo::Vector& val, int ival);

	//get rendering context
	wxGLContext* GetContext();

	bool SetFullScreen();
	void CloseFullScreen();
	void FocusCanvas();

	//get
	ChannelMixMode GetChannelMixMethod();
	bool GetInfo();
	bool GetCamCtr();
	bool GetLegend();
	double GetScalebarValue();
	int GetScalebarUnit();
	fluo::Color GetBgColor();
	bool GetMouseInAovSldr();
	int GetAov();
	bool GetDepthAttenEnable();
	double GetDepthAttenValue();
	bool GetPin();
	double GetScaleFactor();
	fluo::Vector GetRotations();
	fluo::Vector GetRotationsScroll();
	int GetOrthoView();

private:
	//render view///////////////////////////////////////////////
	RenderCanvas *m_canvas;
	wxFrame* m_full_frame;
	wxBoxSizer* m_view_sizer;

	//top bar///////////////////////////////////////////////////
	wxUndoableToolbar* m_mix_mode_tb;
	wxUndoableToolbar* m_hud_tb;
	wxTextCtrl *m_scale_text;
	wxComboBox *m_scale_cmb;
	wxUndoableColorPicker* m_bg_color_picker;
	wxToolBar* m_bg_inv_btn;
	wxButton* m_snapshot_btn;
	wxUndoableToolbar* m_view_manip_btn;
	wxSingleSlider* m_aov_sldr;
	wxNumTextCtrl* m_aov_text;
	wxUndoableToolbar* m_cam_op_tb;
	wxToolBar *m_full_screen_toolbar;

	//left bar///////////////////////////////////////////////////
	wxUndoableToolbar* m_depth_atten_btn;
	wxSingleSlider *m_depth_atten_factor_sldr;
	wxToolBar *m_depth_atten_reset_btn;
	wxNumTextCtrl *m_depth_atten_factor_text;

	//right bar///////////////////////////////////////////////////
	wxToolBar *m_pin_btn;
	wxToolBar *m_center_btn;
	wxToolBar* m_center_click_btn;
	wxToolBar *m_scale_121_btn;
	wxSingleSlider *m_scale_factor_sldr;
	wxNumTextCtrl *m_scale_factor_text;
	wxSpinButton* m_scale_factor_spin;
	wxToolBar *m_scale_mode_btn;
	wxToolBar *m_scale_reset_btn;

	//bottom bar///////////////////////////////////////////////////
	wxUndoableToolbar* m_slider_mode_btn;
	wxNumTextCtrl *m_x_rot_text;
	wxNumTextCtrl *m_y_rot_text;
	wxNumTextCtrl *m_z_rot_text;
	wxUndoableScrollBar* m_x_rot_sldr;
	wxUndoableScrollBar* m_y_rot_sldr;
	wxUndoableScrollBar* m_z_rot_sldr;
	wxComboBox *m_ortho_view_cmb;
	wxToolBar* m_rot_btn;

	//values set by ui
	ChannelMixMode m_channel_mix_mode;

private:
	//called when updated from bars
	void CreateBar();

	//bar top
	void OnChannelMixMode(wxCommandEvent& event);
	void OnHud(wxCommandEvent& event);
	void OnScaleText(wxCommandEvent& event);
	void OnScaleUnit(wxCommandEvent& event);
	void OnBgColorChange(wxColourPickerEvent& event);
	void OnBgInvBtn(wxCommandEvent& event);
	void OnSnapshotBtn(wxCommandEvent& event);
	void OnViewManipBtn(wxCommandEvent& event);
	void OnAovSldrIdle(wxIdleEvent& event);
	void OnAovChange(wxScrollEvent& event);
	void OnAovText(wxCommandEvent& event);
	void OnToolBar2(wxCommandEvent& event);
	void OnFullScreenToolbar(wxCommandEvent& event);

	//bar left
	void OnDepthAttenCheck(wxCommandEvent& event);
	void OnDepthAttenChange(wxScrollEvent& event);
	void OnDepthAttenEdit(wxCommandEvent& event);
	void OnDepthAttenReset(wxCommandEvent& event);

	//bar right
	void OnPin(wxCommandEvent& event);
	void OnCenter(wxCommandEvent& event);
	void OnCenterClick(wxCommandEvent& event);
	void OnScale121(wxCommandEvent& event);
	void OnScaleFactorChange(wxScrollEvent& event);
	void OnScaleFactorEdit(wxCommandEvent& event);
	void OnScaleMode(wxCommandEvent& event);
	void OnScaleReset(wxCommandEvent& event);
	void OnScaleFactorSpinUp(wxSpinEvent& event);
	void OnScaleFactorSpinDown(wxSpinEvent& event);

	//bar bottom
	void OnRotSliderMode(wxCommandEvent& event);
	void OnRotEdit(wxCommandEvent& event);
	void OnRotScroll(wxScrollEvent& event);

	void OnRotSettings(wxCommandEvent& event);
	void OnOrthoViewSelected(wxCommandEvent& event);
};

#endif//_RENDERVIEWPANEL_H_
