﻿/*
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
#ifndef _VOLUMEPROPPANEL_H_
#define _VOLUMEPROPPANEL_H_

#include <PropPanel.h>

class RenderView;
class VolumeData;
class DataGroup;
class wxFadeButton;
class wxBasisSlider;
class wxDoubleSlider;
class wxSingleSlider;
class wxUndoableCheckBox;
class wxUndoableToolbar;
class wxUndoableComboBox;
class wxUndoableColorPicker;
class wxColourPickerEvent;
class wxUndoableTextCtrl;
class VolumePropPanel: public PropPanel
{
	enum
	{
		//toolbar
		ID_UseMlChk = 0,
		ID_TranspChk,
		ID_MipChk,
		ID_InvChk,
		ID_CompChk,
		ID_InterpolateChk,
		ID_NRChk,
		ID_SyncGroupChk,
		ID_DepthChk,
		ID_LegendChk,
		ID_ResetDefault,
		ID_SaveDefault
	};

public:
	VolumePropPanel(MainFrame* frame,
		wxWindow* parent,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0,
		const wxString& name = "VolumePropPanel");
	~VolumePropPanel();

	virtual void FluoUpdate(const fluo::ValueCollection& vc = {});

	void SetVolumeData(VolumeData* vd);
	VolumeData* GetVolumeData();
	void InitVRenderViews(unsigned int type);

	//sync group
	void SetGroup(DataGroup* group);
	DataGroup* GetGroup();

	//sync view in depth mode
	void SetView(RenderView* view);
	RenderView* GetView();

	void ApplyMl();
	void SaveMl();

	void ClearUndo();

private:
	VolumeData* m_vd;

	bool m_lumi_change;
	bool m_sync_group;
	DataGroup* m_group;
	RenderView* m_view;
	double m_max_val;

	//1
	//gamma
	wxFadeButton *m_gamma_st;
	wxSingleSlider *m_gamma_sldr;
	wxTextCtrl *m_gamma_text;
	wxUndoableCheckBox* m_gamma_chk;
	//saturation point
	wxFadeButton *m_saturation_st;
	wxSingleSlider *m_saturation_sldr;
	wxTextCtrl *m_saturation_text;
	wxUndoableCheckBox* m_saturation_chk;
	//luminance
	wxFadeButton *m_luminance_st;
	wxSingleSlider *m_luminance_sldr;
	wxTextCtrl* m_luminance_text;
	wxUndoableCheckBox* m_luminance_chk;
	//alpha
	wxFadeButton *m_alpha_st;
	wxSingleSlider *m_alpha_sldr;
	wxTextCtrl* m_alpha_text;
	wxUndoableCheckBox* m_alpha_chk;
	//shading
	wxFadeButton* m_shade_st;
	wxSingleSlider *m_hi_shading_sldr;
	wxTextCtrl *m_hi_shading_text;
	wxSingleSlider *m_low_shading_sldr;
	wxTextCtrl *m_low_shading_text;
	wxUndoableCheckBox* m_shade_chk;

	//2
	//boundary
	wxFadeButton* m_boundary_st;
	wxSingleSlider *m_boundary_sldr;
	wxTextCtrl *m_boundary_text;
	wxUndoableCheckBox* m_boundary_chk;
	//thresholds
	wxFadeButton* m_thresh_st;
	wxDoubleSlider *m_thresh_sldr;
	wxTextCtrl *m_left_thresh_text;
	wxTextCtrl *m_right_thresh_text;
	wxToolBar* m_thresh_link_tb;
	wxUndoableCheckBox* m_thresh_chk;
	//shadow
	wxFadeButton* m_shadow_st;
	wxSingleSlider *m_shadow_sldr;
	wxTextCtrl *m_shadow_text;
	wxUndoableCheckBox* m_shadow_chk;
	wxSingleSlider* m_shadow_dir_sldr;
	wxTextCtrl* m_shadow_dir_text;
	wxUndoableCheckBox* m_shadow_dir_chk;
	//sample rate
	wxFadeButton* m_sample_st;
	wxSingleSlider *m_sample_sldr;
	wxTextCtrl *m_sample_text;
	wxUndoableCheckBox* m_sample_chk;
	//colormap
	wxFadeButton* m_colormap_st;
	wxDoubleSlider *m_colormap_sldr;
	wxTextCtrl *m_colormap_low_text;
	wxTextCtrl *m_colormap_hi_text;
	wxUndoableCheckBox* m_colormap_chk;
	wxToolBar* m_colormap_link_tb;

	//3
	//color
	wxTextCtrl *m_color_text;
	wxUndoableColorPicker* m_color_btn;
	wxTextCtrl *m_color2_text;
	wxUndoableColorPicker* m_color2_btn;
	//space
	wxUndoableTextCtrl* m_space_x_text;
	wxUndoableTextCtrl* m_space_y_text;
	wxUndoableTextCtrl* m_space_z_text;
	wxUndoableToolbar* m_colormap_inv_btn;
	wxUndoableComboBox* m_colormap_combo;
	wxUndoableComboBox* m_colormap_combo2;

	//buttons
	wxUndoableToolbar* m_options_toolbar;

private:
	bool SetSpacings();

	//update max value
	void UpdateMaxVal(double value);

	//enable/disable
	//1
	void EnableGamma(bool);
	void EnableSaturation(bool);
	void EnableLuminance(bool);
	void EnableAlpha(bool);
	void EnableShading(bool);
	//2
	void EnableBoundary(bool);
	void EnableThresh(bool);
	void EnableShadow(bool);
	void EnableShadowDir(bool);
	void EnableSample(bool);
	void EnableColormap(bool);
	//3
	void EnableMip(bool);
	void EnableTransparent(bool);

	//set values
	void SetGamma(double, bool);
	void SetSaturation(double, bool);
	void SetLuminance(double, bool);
	void SetAlpha(double, bool);
	void SetLowShading(double, bool);
	void SetHiShading(double, bool);
	void SetBoundary(double, bool);
	void SetThresh(double, double, bool);
	void SetShadowInt(double, bool);
	void SetShadowDir(double, bool);
	void SetSampleRate(double, bool);
	void SetColormapVal(double, double, bool);

	//sync values
	void SyncGamma(double);
	void SyncSaturation(double);
	void SyncLuminance(double);
	void SyncAlpha(double);
	void SyncLowShading(double);
	void SyncHiShading(double);
	void SyncBoundary(double);
	void SyncThresh(double, double);
	void SyncShadowInt(double);
	void SyncSampleRate(double);
	void SyncColormapVal(double, double);

	//optioins
	void SetMachineLearning();
	void SetTransparent();
	void SetMIP();
	void SetInvert();
	void SetComponentDisplay();
	void SetInterpolate();
	void SetNoiseReduction();
	void SetSyncGroup();
	void SetBlendDepth();
	void SetLegend();
	void SaveDefault();
	void ResetDefault();

	//1
	void OnGammaMF(wxCommandEvent& event);
	void OnGammaChange(wxScrollEvent& event);
	void OnGammaText(wxCommandEvent& event);
	void OnGammaChk(wxCommandEvent& event);
	//
	void OnSaturationMF(wxCommandEvent& event);
	void OnSaturationChange(wxScrollEvent& event);
	void OnSaturationText(wxCommandEvent& event);
	void OnSaturationChk(wxCommandEvent& event);
	//
	void OnLuminanceMF(wxCommandEvent& event);
	void OnLuminanceChange(wxScrollEvent& event);
	void OnLuminanceText(wxCommandEvent& event);
	void OnLuminanceChk(wxCommandEvent& event);
	//
	void OnAlphaMF(wxCommandEvent& event);
	void OnAlphaChange(wxScrollEvent& event);
	void OnAlphaText(wxCommandEvent& event);
	void OnAlphaCheck(wxCommandEvent& event);
	//
	void OnShadingMF(wxCommandEvent& event);
	void OnLowShadingChange(wxScrollEvent& event);
	void OnLowShadingText(wxCommandEvent& event);
	void OnHiShadingChange(wxScrollEvent& event);
	void OnHiShadingText(wxCommandEvent& event);
	void OnShadingChk(wxCommandEvent& event);

	//2
	void OnBoundaryMF(wxCommandEvent& event);
	void OnBoundaryChange(wxScrollEvent& event);
	void OnBoundaryText(wxCommandEvent& event);
	void OnBoundaryChk(wxCommandEvent& event);
	//
	void OnThreshMF(wxCommandEvent& event);
	void OnThreshChange(wxScrollEvent& event);
	void OnThreshText(wxCommandEvent& event);
	void OnThreshLink(wxCommandEvent& event);
	void OnThreshChk(wxCommandEvent& event);
	//
	void OnShadowMF(wxCommandEvent& event);
	void OnShadowChange(wxScrollEvent& event);
	void OnShadowText(wxCommandEvent& event);
	void OnShadowChk(wxCommandEvent& event);
	void OnShadowDirCheck(wxCommandEvent& event);
	void OnShadowDirChange(wxScrollEvent& event);
	void OnShadowDirEdit(wxCommandEvent& event);
	//
	void OnSampleMF(wxCommandEvent& event);
	void OnSampleChange(wxScrollEvent& event);
	void OnSampleText(wxCommandEvent& event);
	void OnSampleChk(wxCommandEvent& event);
	//
	void OnColormapMF(wxCommandEvent& event);
	void OnColormapChange(wxScrollEvent& event);
	void OnColormapText(wxCommandEvent& event);
	void OnColormapLink(wxCommandEvent& event);
	void OnColormapChk(wxCommandEvent& event);

	//others
	void OnOptions(wxCommandEvent& event);
	//color / colormap
	void OnColormapInvBtn(wxCommandEvent& event);
	void OnColormapCombo(wxCommandEvent& event);
	void OnColormapCombo2(wxCommandEvent& event);
	int GetColorString(wxString& str, wxColor& wxc);
	void OnColorChange(wxColor c);
	void OnColorTextChange(wxCommandEvent& event);
	void OnColorTextFocus(wxMouseEvent& event);
	void OnColorBtn(wxColourPickerEvent& event);
	void OnColor2Change(wxColor c);
	void OnColor2TextChange(wxCommandEvent& event);
	void OnColor2TextFocus(wxMouseEvent& event);
	void OnColor2Btn(wxColourPickerEvent& event);
	//spacings
	void OnSpaceText(wxCommandEvent& event);
};

#endif//_VOLUMEPROPPANEL_H_
