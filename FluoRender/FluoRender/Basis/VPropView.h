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
#ifndef _VPROPVIEW_H_
#define _VPROPVIEW_H_

#include <wx/wx.h>
#include <wx/panel.h>
#include <wx/clrpicker.h>
#include <wx/tglbtn.h>

using namespace std;

class VRenderFrame;
class VRenderGLView;
class VolumeData;
class DataGroup;
class wxBasisSlider;
class wxDoubleSlider;
class wxSingleSlider;
class VPropView: public wxPanel
{
	enum
	{
		//1
		ID_GammaSync = ID_VPROP_VIEW,
		ID_GammaChk,
		ID_GammaSldr,
		ID_GammaText,
		//
		ID_SaturationSync,
		ID_SaturationChk,
		ID_SaturationSldr,
		ID_SaturationText,
		//
		ID_LuminanceSync,
		ID_LuminanceChk,
		ID_LuminanceSldr,
		ID_LuminanceText,
		//
		ID_AlphaSync,
		ID_AlphaChk,
		ID_AlphaSldr,
		ID_Alpha_Text,
		//
		ID_ShadingSync,
		ID_ShadingChk,
		ID_LowShadingSldr,
		ID_LowShadingText,
		ID_HiShadingSync,
		ID_HiShadingSldr,
		ID_HiShadingText,

		//2
		ID_BoundarySync,
		ID_BoundaryChk,
		ID_BoundarySldr,
		ID_BoundaryText,
		//
		ID_ThreshSync,
		ID_ThreshChk,
		ID_ThreshSldr,
		ID_ThreshLowText,
		ID_TreshHiText,
		ID_ThreshLinkTb,
		//
		ID_ShadowSync,
		ID_ShadowChk,
		ID_ShadowSldr,
		ID_ShadowText,
		//
		ID_SampleSync,
		ID_SampleChk,
		ID_SampleSldr,
		ID_SampleText,
		//
		ID_ColormapSync,
		ID_ColormapChk,
		ID_ColormapSldr,
		ID_ColormapHiText,
		ID_ColormapLowText,
		ID_ColormapLinkTb,

		//3
		ID_ColorText,
		ID_Color2Text,
		ID_ColorBtn,
		ID_Color2Btn,
		ID_SpaceXText,
		ID_SpaceYText,
		ID_SpaceZText,
		ID_LegendChk,
		ID_InterpolateChk,
		ID_SyncGroupChk,
		ID_SaveDefault,
		ID_ResetDefault,
		ID_ColormapInvBtn,
		ID_ColormapCombo,
		ID_ColormapCombo2,
		ID_InvChk,
		ID_MipChk,
		ID_NRChk,
		ID_DepthChk,
		ID_TranspChk,
		ID_CompChk,
		ID_UseMlChk
};

public:
	VPropView(VRenderFrame* frame,
		wxWindow* parent,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0,
		const wxString& name = "VPropView");
	~VPropView();

	void SetVolumeData(VolumeData* vd);
	VolumeData* GetVolumeData();
	void RefreshVRenderViews(bool tree=false, bool interactive=false);
	void InitVRenderViews(unsigned int type);
	void SetFocusVRenderViews(wxBasisSlider* slider);

	//sync group
	void SetGroup(DataGroup* group);
	DataGroup* GetGroup();

	//sync view in depth mode
	void SetView(VRenderGLView* view);
	VRenderGLView* GetView();

	void ApplyMl();

	void ClearUndo();

private:
	VRenderFrame* m_frame;
	VolumeData* m_vd;

	bool m_lumi_change;
	bool m_sync_group;
	DataGroup* m_group;
	VRenderGLView* m_view;
	double m_max_val;

	//1
	//gamma
	wxButton *m_gamma_st;
	wxSingleSlider *m_gamma_sldr;
	wxTextCtrl *m_gamma_text;
	wxCheckBox* m_gamma_chk;
	//saturation point
	wxButton *m_saturation_st;
	wxSingleSlider *m_saturation_sldr;
	wxTextCtrl *m_saturation_text;
	wxCheckBox* m_saturation_chk;
	//luminance
	wxButton *m_luminance_st;
	wxSingleSlider *m_luminance_sldr;
	wxTextCtrl* m_luminance_text;
	wxCheckBox* m_luminance_chk;
	//alpha
	wxButton *m_alpha_st;
	wxSingleSlider *m_alpha_sldr;
	wxTextCtrl* m_alpha_text;
	wxCheckBox* m_alpha_chk;
	//shading
	wxButton* m_shade_st;
	wxSingleSlider *m_hi_shading_sldr;
	wxTextCtrl *m_hi_shading_text;
	wxSingleSlider *m_low_shading_sldr;
	wxTextCtrl *m_low_shading_text;
	wxCheckBox* m_shade_chk;

	//2
	//boundary
	wxButton* m_boundary_st;
	wxSingleSlider *m_boundary_sldr;
	wxTextCtrl *m_boundary_text;
	wxCheckBox* m_boundary_chk;
	//thresholds
	wxButton* m_thresh_st;
	wxDoubleSlider *m_thresh_sldr;
	wxTextCtrl *m_left_thresh_text;
	wxTextCtrl *m_right_thresh_text;
	wxToolBar* m_thresh_link_tb;
	wxCheckBox* m_thresh_chk;
	//shadow
	wxButton* m_shadow_st;
	wxSingleSlider *m_shadow_sldr;
	wxTextCtrl *m_shadow_text;
	wxCheckBox*m_shadow_chk;
	//sample rate
	wxButton* m_sample_st;
	wxSingleSlider *m_sample_sldr;
	wxTextCtrl *m_sample_text;
	wxCheckBox* m_sample_chk;
	//colormap
	wxButton* m_colormap_st;
	wxDoubleSlider *m_colormap_sldr;
	wxTextCtrl *m_colormap_low_text;
	wxTextCtrl *m_colormap_hi_text;
	wxCheckBox* m_colormap_chk;
	wxToolBar* m_colormap_link_tb;

	//3
	//color
	wxTextCtrl *m_color_text;
	wxColourPickerCtrl *m_color_btn;
	wxTextCtrl *m_color2_text;
	wxColourPickerCtrl *m_color2_btn;
	//space
	wxTextCtrl *m_space_x_text;
	wxTextCtrl *m_space_y_text;
	wxTextCtrl *m_space_z_text;
	wxToggleButton *m_colormap_inv_btn;
	wxComboBox *m_colormap_combo;
	wxComboBox *m_colormap_combo2;

	//buttons
	wxToolBar *m_options_toolbar;

private:
	void GetSettings();
	bool SetSpacings();

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
	void EnableSample(bool);
	void EnableColormap(bool);
	//3
	void EnableMip(bool);

	//update max value
	void UpdateMaxVal(double value);

	//1
	void OnGammaSync(wxCommandEvent& event);
	void OnGammaChange(wxScrollEvent &event);
	void OnGammaText(wxCommandEvent &event);
	void OnGammaChk(wxCommandEvent& event);
	//
	void OnSaturationSync(wxCommandEvent& event);
	void OnSaturationChange(wxScrollEvent &event);
	void OnSaturationText(wxCommandEvent &event);
	void OnSaturationChk(wxCommandEvent& event);
	//
	void OnLuminanceSync(wxCommandEvent& event);
	void OnLuminanceChange(wxScrollEvent& event);
	void OnLuminanceText(wxCommandEvent& event);
	void OnLuminanceChk(wxCommandEvent& event);
	//
	void OnAlphaSync(wxCommandEvent& event);
	void OnAlphaChange(wxScrollEvent& event);
	void OnAlphaText(wxCommandEvent& event);
	void OnAlphaCheck(wxCommandEvent& event);
	//
	void OnShadingSync(wxCommandEvent& event);
	void OnLowShadingChange(wxScrollEvent& event);
	void OnLowShadingText(wxCommandEvent& event);
	void OnHiShadingChange(wxScrollEvent& event);
	void OnHiShadingText(wxCommandEvent& event);
	void OnShadingChk(wxCommandEvent& event);

	//2
	void OnBoundarySync(wxCommandEvent& event);
	void OnBoundaryChange(wxScrollEvent &event);
	void OnBoundaryText(wxCommandEvent &event);
	void OnBoundaryChk(wxCommandEvent& event);
	//
	void OnThreshSync(wxCommandEvent& event);
	void OnThreshChange(wxScrollEvent &event);
	void OnThreshLowText(wxCommandEvent &event);
	void OnThreshHiText(wxCommandEvent &event);
	void OnThreshLink(wxCommandEvent& event);
	void OnThreshChk(wxCommandEvent& event);
	//
	void OnShadowSync(wxCommandEvent& event);
	void OnShadowChange(wxScrollEvent &event);
	void OnShadowText(wxCommandEvent &event);
	void OnShadowChk(wxCommandEvent &event);
	//
	void OnSampleSync(wxCommandEvent& event);
	void OnSampleChange(wxScrollEvent &event);
	void OnSampleText(wxCommandEvent &event);
	void OnSampleChk(wxCommandEvent& event);
	//
	void OnColormapSync(wxCommandEvent& event);
	void OnColormapChange(wxScrollEvent &event);
	void OnColormapHiText(wxCommandEvent &event);
	void OnColormapLowText(wxCommandEvent &event);
	void OnColormapLink(wxCommandEvent& event);
	void OnColormapChk(wxCommandEvent &event);

	//others
	void OnColormapInvBtn(wxCommandEvent &event);
	void OnColormapCombo(wxCommandEvent &event);
	void OnColormapCombo2(wxCommandEvent &event);
	int GetColorString(wxString& str, wxColor& wxc);
	void OnColorChange(wxColor c);
	void OnColorTextChange(wxCommandEvent& event);
	void OnColorTextFocus(wxCommandEvent& event);
	void OnColorBtn(wxColourPickerEvent& event);
	void OnColor2Change(wxColor c);
	void OnColor2TextChange(wxCommandEvent& event);
	void OnColor2TextFocus(wxCommandEvent& event);
	void OnColor2Btn(wxColourPickerEvent& event);
	//spacings
	void OnSpaceText(wxCommandEvent& event);
	//legend
	void OnLegendCheck(wxCommandEvent& event);
	//interpolate
	void OnInterpolateCheck(wxCommandEvent& event);
	//sync within group
	void OnSyncGroupCheck(wxCommandEvent& event);
	//save as default
	void OnSaveDefault(wxCommandEvent& event);
	void OnResetDefault(wxCommandEvent& event);
	//inversion
	void OnInvCheck(wxCommandEvent &event);
	//MIP
	void OnMIPCheck(wxCommandEvent &event);
	//noise reduction
	void OnNRCheck(wxCommandEvent &event);
	//depth omde
	void OnDepthCheck(wxCommandEvent &event);
	void OnFluoRender(wxCommandEvent &event);
	//transparency
	void OnTranspChk(wxCommandEvent &event);
	//component display
	void OnCompChk(wxCommandEvent &event);
	//ml
	void OnUseMlChk(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()
};

#endif//_VPROPVIEW_H_
