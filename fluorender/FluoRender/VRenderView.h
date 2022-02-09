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

#ifndef _VRENDERVIEW_H_
#define _VRENDERVIEW_H_

//#include "RenderCanvas.h"
#include <wx/wx.h>
#include <wx/panel.h>
#include <wx/clrpicker.h>
#include <wx/spinbutt.h>

class wxGLContext;
class VRenderFrame;
class RenderCanvas;
namespace fluo
{
	class RenderCanvasAgent;
}
class VRenderView: public wxPanel
{
public:
	enum
	{
		ID_VolumeSeqRd = ID_VRENDER_VIEW2,
		ID_VolumeMultiRd,
		ID_VolumeCompRd,
		ID_CaptureBtn,
		ID_BgColorPicker,
		ID_BgInvBtn,
		ID_RotLinkChk,
		ID_ZeroRotBtn,
		ID_RotResetBtn,
		ID_XRotText,
		ID_YRotText,
		ID_ZRotText,
		ID_XRotSldr,
		ID_YRotSldr,
		ID_ZRotSldr,
		ID_RotateTimer,
		ID_RotLockChk,
		ID_RotSliderType,
		ID_OrthoViewCmb,
		ID_DepthAttenChk,
		ID_DepthAttenFactorSldr,
		ID_DepthAttenResetBtn,
		ID_DepthAttenFactorText,
		ID_FullScreenBtn,
		ID_PinBtn,
		ID_CenterBtn,
		ID_Scale121Btn,
		ID_ScaleFactorSldr,
		ID_ScaleFactorText,
		ID_ScaleFactorSpin,
		ID_ScaleModeBtn,
		ID_ScaleResetBtn,
		ID_CamCtrChk,
		ID_FpsChk,
		ID_LegendChk,
		ID_ColormapChk,
		ID_ScaleBar,
		ID_ScaleText,
		ID_ScaleCmb,
		ID_DefaultBtn,
		ID_AovSldr,
		ID_AovText,
		ID_FreeChk
	};

	VRenderView(VRenderFrame* frame,
		wxGLContext* sharedContext=0,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0);
	~VRenderView();

	//update
	void UpdateView(bool ui_update = true);
	void UpdateScaleFactor(bool update_text = true);
	void SetScaleFactor(double s, bool update);
	void SetScaleMode(int mode, bool update);
	//rot center anchor thresh
	void SetPinThreshold(double value);
	//orient cmb
	void UpdateOrientCmb(int index);

	//reset counter
	int GetID() { return m_id; }
	static void ResetID();

	//get rendering context
	wxGLContext* GetContext();

	//bit mask for items to save
	bool m_default_saved;
	void SaveDefault(unsigned int mask = 0xffffffff);

	//set full screen
	void SetFullScreen();
	void SetCanvas(RenderCanvas* canvas) { m_canvas = canvas; }
	RenderCanvas* GetCanvas() { return m_canvas; }
	void SetFullFrame(wxFrame* ff) { m_full_frame = ff; }
	wxFrame* GetFullFrame() { return m_full_frame; }
	void HideFullFrame() { m_full_frame->Hide(); }
	void AddCanvas(RenderCanvas* canvas);

private:
	static int m_max_id;
	int m_id;

	fluo::RenderCanvasAgent* m_agent;
	wxWindow* m_frame;
	//render view///////////////////////////////////////////////
	RenderCanvas *m_canvas;
	bool m_mouse_focus;
	wxFrame* m_full_frame;
	wxBoxSizer* m_view_sizer;

	//top bar///////////////////////////////////////////////////
	wxPanel* m_panel_1;
	wxColourPickerCtrl *m_bg_color_picker;
	wxButton* m_bg_inv_btn;
	wxSlider* m_aov_sldr;
	wxTextCtrl* m_aov_text;
	wxToolBar * m_options_toolbar;
	//wxToolBar * m_options_toolbar2;
	wxToolBar * m_left_toolbar;
	wxToolBar * m_right_toolbar2;
	wxToolBar * m_lower_toolbar;
	//scale bar
	wxTextCtrl *m_scale_text;
	wxComboBox *m_scale_cmb;

	//bottom bar///////////////////////////////////////////////////
	wxPanel* m_panel_2;
	wxScrollBar *m_x_rot_sldr;
	wxTextCtrl *m_x_rot_text;
	wxScrollBar *m_y_rot_sldr;
	wxTextCtrl *m_y_rot_text;
	wxScrollBar *m_z_rot_sldr;
	wxTextCtrl *m_z_rot_text;
	wxTimer m_timer;
	bool m_x_rotating, m_y_rotating, m_z_rotating;
	bool m_skip_thumb;
	wxToolBar *m_rot_lock_btn;
	wxComboBox *m_ortho_view_cmb;

	//left bar///////////////////////////////////////////////////
	wxPanel* m_panel_3;
	wxSlider *m_depth_atten_factor_sldr;
	wxToolBar *m_depth_atten_reset_btn;
	wxTextCtrl *m_depth_atten_factor_text;

	//right bar///////////////////////////////////////////////////
	wxPanel* m_panel_4;
	wxToolBar *m_full_screen_btn;
	wxToolBar *m_pin_btn;
	wxToolBar *m_center_btn;
	wxToolBar *m_scale_121_btn;
	wxSlider *m_scale_factor_sldr;
	wxTextCtrl *m_scale_factor_text;
	wxToolBar *m_scale_mode_btn;
	wxToolBar *m_scale_reset_btn;
	wxSpinButton* m_scale_factor_spin;

	//draw clip
	bool m_draw_clip;

	//draw scalebar
	enum SCALEBAR_STATE {
		kOn,
		kOff,
		kText
	};
	SCALEBAR_STATE m_draw_scalebar;

	double m_pin_scale_thresh;//scale factor theshold value for auto update
	//rot slider style
	bool m_rot_slider;

	bool m_use_dft_settings;
	double m_dft_x_rot;
	double m_dft_y_rot;
	double m_dft_z_rot;
	double m_dft_depth_atten_factor;
	double m_dft_scale_factor;
	int m_dft_scale_factor_mode;//0:view; 1:pixel; 2:data(pixel * xy spc)

private:
	//called when updated from bars
	void CreateBar();
	//load default settings from file
	void LoadSettings();

	//bar top
	void OnVolumeMethodCheck(wxCommandEvent& event);
	void OnCh1Check(wxCommandEvent &event);
	void OnChAlphaCheck(wxCommandEvent &event);
	void OnChFloatCheck(wxCommandEvent &event);
	void OnChEmbedCheck(wxCommandEvent &event);
	void OnChEnlargeCheck(wxCommandEvent &event);
	void OnSlEnlargeScroll(wxScrollEvent &event);
	void OnTxEnlargeText(wxCommandEvent &event);
	static wxWindow* CreateExtraCaptureControl(wxWindow* parent);
	void OnCapture(wxCommandEvent& event);
	void OnBgColorChange(wxColourPickerEvent& event);
	void OnBgInvBtn(wxCommandEvent& event);
	void OnCamCtrCheck(wxCommandEvent& event);
	void OnFpsCheck(wxCommandEvent& event);
	void OnLegendCheck(wxCommandEvent& event);
	void OnColormapCheck(wxCommandEvent& event);
	void OnScaleBar(wxCommandEvent& event);
	void OnScaleTextEditing(wxCommandEvent& event);
	void OnScaleUnitSelected(wxCommandEvent& event);
	void OnAovSldrIdle(wxIdleEvent& event);
	void OnAovChange(wxScrollEvent& event);
	void OnAovText(wxCommandEvent &event);
	void OnFreeChk(wxCommandEvent& event);
	void OnFullScreen(wxCommandEvent& event);
	//bar left
	void OnDepthAttenCheck(wxCommandEvent& event);
	void OnDepthAttenFactorChange(wxScrollEvent& event);
	void OnDepthAttenFactorEdit(wxCommandEvent& event);
	void OnDepthAttenReset(wxCommandEvent &event);
	//bar right
	void OnPin(wxCommandEvent &event);
	void OnCenter(wxCommandEvent &event);
	void OnScale121(wxCommandEvent &event);
	void OnScaleFactorChange(wxScrollEvent& event);
	void OnScaleFactorEdit(wxCommandEvent& event);
	void OnScaleMode(wxCommandEvent& event);
	void OnScaleReset(wxCommandEvent &event);
	void OnScaleFactorSpinUp(wxSpinEvent& event);
	void OnScaleFactorSpinDown(wxSpinEvent& event);
	//bar bottom
	void OnZeroRot(wxCommandEvent& event);
	void OnRotReset(wxCommandEvent &event);
	void OnValueEdit(wxCommandEvent& event);
	void OnXRotScroll(wxScrollEvent &event);
	void OnYRotScroll(wxScrollEvent &event);
	void OnZRotScroll(wxScrollEvent &event);
	void OnRotLockCheck(wxCommandEvent& event);
	void OnRotSliderType(wxCommandEvent& event);
	void OnOrthoViewSelected(wxCommandEvent& event);

	void OnSaveDefault(wxCommandEvent &event);

	void OnKeyDown(wxKeyEvent& event);
	//idle
	void OnTimer(wxTimerEvent& event);

	DECLARE_EVENT_TABLE()

};

#endif//_VRENDERVIEW_H_
