/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2024 Scientific Computing and Imaging Institute,
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

#include <PropPanel.h>
#include <wx/clrpicker.h>
#include <wx/spinbutt.h>

class MainFrame;
class RenderCanvas;
class wxGLContext;
class wxSingleSlider;
class wxUndoableScrollBar;
class wxUndoableToolbar;
class wxUndoableColorPicker;
class RenderViewPanel: public PropPanel
{
public:
	enum
	{
		ID_VolumeSeqRd = 0,
		ID_VolumeMultiRd,
		ID_VolumeCompRd,
		ID_CaptureBtn,
		ID_InfoChk,
		ID_CamCtrChk,
		ID_LegendChk,
		ID_ColormapChk,
		ID_ScaleBar
	};
	enum
	{
		ID_FreeChk = 0,
		ID_DefaultBtn,
	};
	enum
	{
		ID_RotLockChk = 0,
		ID_RotSliderType
	};
	enum
	{
		ID_RotXScroll = 0,
		ID_RotYScroll,
		ID_RotZScroll
	};
	enum
	{
		ID_ZeroRotBtn = 0,
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

	RenderViewPanel(MainFrame* frame,
		wxGLContext* sharedContext=0,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0,
		const wxString& name = "RenderView");
	~RenderViewPanel();

	virtual void FluoUpdate(const fluo::ValueCollection& vc = {});

	//reset counter
	static void ResetID();

	//update
	void SetVolumeMethod(int val);
	void Capture();
	void SetInfo(bool val);
	void SetDrawCamCtr(bool val);
	void SetLegend(bool val);
	void SetDrawColormap(bool val);
	void SetDrawScalebar(int val);
	void SetScaleText(double val);
	void SetScaleUnit(int val);
	void SetBgColor(fluo::Color val);
	void SetBgColorInvert(bool val);
	void SetAov(double val, bool notify);
	void SetFree(bool val);
	void SetFullScreen();
	void CloseFullScreen();

	void SetDepthAttenEnable(bool val);
	void SetDepthAtten(double val, bool notify);

	void SetCenter();
	void SetScale121();
	void SetScaleFactor(double val);
	void SetScaleMode(int val);

	void SetRotLock(bool val);
	void SetSliderType(bool val);
	void SetRotations(double rotx, double roty, double rotz, bool notify);
	void SetZeroRotations();

	//get rendering context
	wxGLContext* GetContext();

	//refresh glview
	void RefreshGL(bool intactive=true, bool start_loop=true);

	//bit mask for items to save
	bool m_default_saved;
	void SaveDefault(unsigned int mask = 0xffffffff);
	void LoadSettings();

public:
	static int m_max_id;
	int m_id;
	int m_draw_scalebar;
	bool m_bg_color_inv;
	//rot slider style
	bool m_rot_slider;
	double m_dpi_sf, m_dpi_sf2;
	int m_pin_by_user;//override pin by scale: 0:by scale; 1:always pin; 2:always not pin
	bool m_pin_by_scale;

	//render view///////////////////////////////////////////////
	RenderCanvas *m_canvas;
	wxFrame* m_full_frame;
	wxBoxSizer* m_view_sizer;

	//top bar///////////////////////////////////////////////////
	wxUndoableToolbar* m_options_toolbar;
	wxTextCtrl *m_scale_text;
	wxComboBox *m_scale_cmb;
	wxUndoableColorPicker* m_bg_color_picker;
	wxToolBar* m_bg_inv_btn;
	wxSingleSlider* m_aov_sldr;
	wxTextCtrl* m_aov_text;
	wxUndoableToolbar* m_options_toolbar2;
	wxToolBar *m_full_screen_btn;

	//left bar///////////////////////////////////////////////////
	wxUndoableToolbar* m_depth_atten_btn;
	wxSingleSlider *m_depth_atten_factor_sldr;
	wxToolBar *m_depth_atten_reset_btn;
	wxTextCtrl *m_depth_atten_factor_text;

	//right bar///////////////////////////////////////////////////
	wxToolBar *m_pin_btn;
	wxToolBar *m_center_btn;
	wxToolBar *m_scale_121_btn;
	wxSingleSlider *m_scale_factor_sldr;
	wxTextCtrl *m_scale_factor_text;
	wxSpinButton* m_scale_factor_spin;
	wxToolBar *m_scale_mode_btn;
	wxToolBar *m_scale_reset_btn;

	//bottom bar///////////////////////////////////////////////////
	wxUndoableToolbar* m_rot_lock_btn;
	wxTextCtrl *m_x_rot_text;
	wxTextCtrl *m_y_rot_text;
	wxTextCtrl *m_z_rot_text;
	wxUndoableScrollBar* m_x_rot_sldr;
	wxUndoableScrollBar* m_y_rot_sldr;
	wxUndoableScrollBar* m_z_rot_sldr;
	wxComboBox *m_ortho_view_cmb;
	wxToolBar* m_reset_btn;

private:
	//called when updated from bars
	void CreateBar();

	//bar top
	void OnToolBar(wxCommandEvent& event);
	void OnScaleText(wxCommandEvent& event);
	void OnScaleUnit(wxCommandEvent& event);
	void OnBgColorChange(wxColourPickerEvent& event);
	void OnBgInvBtn(wxCommandEvent& event);
	void OnAovSldrIdle(wxIdleEvent& event);
	void OnAovChange(wxScrollEvent& event);
	void OnAovText(wxCommandEvent& event);
	void OnToolBar2(wxCommandEvent& event);
	void OnFullScreen(wxCommandEvent& event);

	//bar left
	void OnDepthAttenCheck(wxCommandEvent& event);
	void OnDepthAttenChange(wxScrollEvent& event);
	void OnDepthAttenEdit(wxCommandEvent& event);
	void OnDepthAttenReset(wxCommandEvent& event);

	//bar right
	void OnPin(wxCommandEvent& event);
	void OnCenter(wxCommandEvent& event);
	void OnScale121(wxCommandEvent& event);
	void OnScaleFactorChange(wxScrollEvent& event);
	void OnScaleFactorEdit(wxCommandEvent& event);
	void OnScaleMode(wxCommandEvent& event);
	void OnScaleReset(wxCommandEvent& event);
	void OnScaleFactorSpinUp(wxSpinEvent& event);
	void OnScaleFactorSpinDown(wxSpinEvent& event);

	//bar bottom
	void OnRotLockCheck(wxCommandEvent& event);
	void OnRotEdit(wxCommandEvent& event);
	void OnRotScroll(wxScrollEvent& event);

	void OnRotReset(wxCommandEvent& event);
	void OnOrthoViewSelected(wxCommandEvent& event);

	//capture options
	void OnCh1Check(wxCommandEvent& event);
	void OnChAlphaCheck(wxCommandEvent& event);
	void OnChFloatCheck(wxCommandEvent& event);
	void OnDpiText(wxCommandEvent& event);
	void OnChEmbedCheck(wxCommandEvent& event);
	void OnChEnlargeCheck(wxCommandEvent& event);
	void OnSlEnlargeScroll(wxScrollEvent& event);
	void OnTxEnlargeText(wxCommandEvent& event);
	static wxWindow* CreateExtraCaptureControl(wxWindow* parent);
};

#endif//_RENDERVIEWPANEL_H_
