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
#include <wx/slider.h>
#include <wx/tglbtn.h>

using namespace std;

class VRenderFrame;
class VRenderGLView;
namespace fluo
{
	class VolumeData;
	class VolumeGroup;
}
class VPropView: public wxPanel
{
	enum
	{
		ID_GammaSync = ID_VPROP_VIEW,
		ID_GammaSldr,
		ID_GammaText,
		ID_SaturationSync,
		ID_SaturationSldr,
		ID_SaturationText,
		ID_LuminanceSync,
		ID_LuminanceSldr,
		ID_LuminanceText,
		ID_AlphaSync,
		ID_AlphaChk,
		ID_AlphaSldr,
		ID_Alpha_Text,
		ID_ShadingSync,
		ID_ShadingEnableChk,
		ID_LowShadingSldr,
		ID_LowShadingText,

		ID_BoundarySync,
		ID_BoundarySldr,
		ID_BoundaryText,
		ID_ThreshSync,
		ID_LeftThreshSldr,
		ID_LeftThreshText,
		ID_RightThreshSldr,
		ID_RightThreshText,
		ID_HiShadingSync,
		ID_HiShadingSldr,
		ID_HiShadingText,
		ID_ShadowSync,
		ID_ShadowChk,
		ID_ShadowSldr,
		ID_ShadowText,
		ID_SampleSync,
		ID_SampleSldr,
		ID_SampleText,
		ID_ColormapSync,
		ID_ColormapEnableChk,
		ID_ColormapHighValueSldr,
		ID_ColormapHighValueText,
		ID_ColormapLowValueSldr,
		ID_ColormapLowValueText,

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
};

public:
	VPropView(VRenderFrame* frame,
		wxWindow* parent,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0,
		const wxString& name = "VPropView");
	~VPropView();

	void SetVolumeData(fluo::VolumeData* vd);
	fluo::VolumeData* GetVolumeData();
	void RefreshVRenderViews(bool tree=false, bool interactive=false);
	void InitVRenderViews(unsigned int type);

	//sync group
	void SetGroup(fluo::VolumeGroup* group);
	fluo::VolumeGroup* GetGroup();

	//sync view in depth mode
	void SetView(VRenderGLView* view);
	VRenderGLView* GetView();

private:
	VRenderFrame* m_frame;
	fluo::VolumeData* m_vd;

	bool m_lumi_change;
	bool m_sync_group;
	fluo::VolumeGroup* m_group;
	VRenderGLView* m_view;
	double m_max_val;

	//1st line
	//gamma
	wxStaticText *m_gamma_st;
	wxSlider *m_gamma_sldr;
	wxTextCtrl *m_gamma_text;
	//boundary
	wxStaticText *m_boundary_st;
	wxSlider *m_boundary_sldr;
	wxTextCtrl *m_boundary_text;

	//2nd line
	//saturation point
	wxStaticText *m_saturation_st;
	wxSlider *m_saturation_sldr;
	wxTextCtrl *m_saturation_text;
	//thresholds
	wxStaticText *m_threh_st;
	wxSlider *m_left_thresh_sldr;
	wxTextCtrl *m_left_thresh_text;
	wxSlider *m_right_thresh_sldr;
	wxTextCtrl *m_right_thresh_text;

	//3rd line
	//luminance
	wxStaticText *m_luminance_st;
	wxSlider *m_luminance_sldr;
	wxTextCtrl* m_luminance_text;
	//shadow
	wxSlider *m_shadow_sldr;
	wxTextCtrl *m_shadow_text;
	wxToolBar *m_shadow_tool;

	//4th line
	//alpha
	wxToolBar *m_alpha_tool;
	wxSlider *m_alpha_sldr;
	wxTextCtrl* m_alpha_text;
	//sample rate
	wxStaticText* m_sample_st;
	wxSlider *m_sample_sldr;
	wxTextCtrl *m_sample_text;

	//5th line
	//highlight
	wxSlider *m_hi_shading_sldr;
	wxTextCtrl *m_hi_shading_text;
	//shading
	wxSlider *m_low_shading_sldr;
	wxTextCtrl *m_low_shading_text;
	wxToolBar *m_shade_tool;
	//colormap
	wxToolBar *m_colormap_tool;
	wxSlider *m_colormap_high_value_sldr;
	wxTextCtrl *m_colormap_high_value_text;
	wxSlider *m_colormap_low_value_sldr;
	wxTextCtrl *m_colormap_low_value_text;

	//others
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
	void EnableAlpha();
	void DisableAlpha();
	void EnableShading();
	void DisableShading();
	void EnableShadow();
	void DisableShadow();
	void EnableColormap();
	void DisableColormap();
	void EnableMip();
	void DisableMip();

	//update max value
	void UpdateMaxVal(double value);

	//1
	void OnGammaSync(wxMouseEvent& event);
	void OnGammaChange(wxScrollEvent &event);
	void OnGammaText(wxCommandEvent &event);
	void OnBoundarySync(wxMouseEvent& event);
	void OnBoundaryChange(wxScrollEvent &event);
	void OnBoundaryText(wxCommandEvent &event);
	//2
	void OnSaturationSync(wxMouseEvent& event);
	void OnSaturationChange(wxScrollEvent &event);
	void OnSaturationText(wxCommandEvent &event);
	void OnThreshSync(wxMouseEvent& event);
	void OnLeftThreshChange(wxScrollEvent &event);
	void OnLeftThreshText(wxCommandEvent &event);
	void OnRightThreshChange(wxScrollEvent &event);
	void OnRightThreshText(wxCommandEvent &event);
	//3
	void OnLuminanceSync(wxMouseEvent& event);
	void OnLuminanceChange(wxScrollEvent &event);
	void OnLuminanceText(wxCommandEvent &event);
	//shading
	void OnHiShadingChange(wxScrollEvent &event);
	void OnHiShadingText(wxCommandEvent &event);
	//shadow
	void OnShadowSync(wxMouseEvent& event);
	void OnShadowEnable(wxCommandEvent &event);
	void OnShadowChange(wxScrollEvent &event);
	void OnShadowText(wxCommandEvent &event);
	//4
	void OnAlphaSync(wxMouseEvent& event);
	void OnAlphaCheck(wxCommandEvent &event);
	void OnAlphaChange(wxScrollEvent & event);
	void OnAlphaText(wxCommandEvent& event);
	void OnSampleSync(wxMouseEvent& event);
	void OnSampleChange(wxScrollEvent &event);
	void OnSampleText(wxCommandEvent &event);
	//5
	void OnShadingSync(wxMouseEvent& event);
	void OnLowShadingChange(wxScrollEvent &event);
	void OnLowShadingText(wxCommandEvent &event);
	void OnShadingEnable(wxCommandEvent &event);
	//colormap
	void OnColormapSync(wxMouseEvent& event);
	void OnEnableColormap(wxCommandEvent &event);
	void OnColormapHighValueChange(wxScrollEvent &event);
	void OnColormapHighValueText(wxCommandEvent &event);
	void OnColormapLowValueChange(wxScrollEvent &event);
	void OnColormapLowValueText(wxCommandEvent &event);
	
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

	DECLARE_EVENT_TABLE()
};

#endif//_VPROPVIEW_H_
