#include "DataManager.h"
#include <wx/wx.h>
#include <wx/panel.h>
#include <wx/glcanvas.h>
#include <wx/clrpicker.h>
#include <wx/slider.h>

#include "FLIVR/Color.h"
#include "FLIVR/VolumeRenderer.h"
#include "FLIVR/BBox.h"
#include "FLIVR/Point.h"
#include "FLIVR/MultiVolumeRenderer.h"


#ifndef _VPROPVIEW_H_
#define _VPROPVIEW_H_

using namespace std;

class VRenderView;

class VPropView: public wxPanel
{
	enum
	{
		ID_Load = wxID_HIGHEST+801,
		ID_Save,
		ID_AddRect,
		ID_AddTri,
		ID_Del,
		ID_ColorText,
		ID_ColorBtn,
		ID_AlphaChk,
		ID_AlphaSldr,
		ID_Alpha_Text,
		ID_SampleSldr,
		ID_SampleText,
		ID_BoundarySldr,
		ID_BoundaryText,
		ID_GammaSldr,
		ID_GammaText,
		ID_ContrastSldr,
		ID_ContrastText,
		ID_LeftThreshSldr,
		ID_LeftThreshText,
		ID_RightThreshSldr,
		ID_RightThreshText,
		ID_LowShadingSldr,
		ID_LowShadingText,
		ID_ShadingEnableChk,
		ID_HiShadingSldr,
		ID_HiShadingText,
		ID_ShadowChk,
		ID_ShadowSldr,
		ID_ShadowText,
		ID_SpaceXText,
		ID_SpaceYText,
		ID_SpaceZText,
		ID_LuminanceSldr,
		ID_LuminanceText,
		ID_LegendChk,
		ID_InterpolateChk,
		ID_SyncGroupChk,
		ID_SaveDefault,
		ID_ResetDefault,
		ID_ColormapEnableChk,
		ID_ColormapHighValueSldr,
		ID_ColormapHighValueText,
		ID_ColormapLowValueSldr,
		ID_ColormapLowValueText,
		ID_InvChk,
		ID_MipChk,
		ID_NRChk,
		ID_DepthChk
};

public:
	VPropView(wxWindow* frame, wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0,
		const wxString& name = "VPropView");
	~VPropView();

	void SetVolumeData(VolumeData* vd);
	VolumeData* GetVolumeData();
	void RefreshVRenderViews(bool tree=false, bool interactive=false);
	void InitVRenderViews(unsigned int type);

	//sync group
	void SetGroup(DataGroup* group);
	DataGroup* GetGroup();

	//sync view in depth mode
	void SetView(VRenderView* view);
	VRenderView* GetView();

private:
	wxWindow* m_frame;
	VolumeData* m_vd;

	bool m_lumi_change;
	bool m_sync_group;
	DataGroup* m_group;
	VRenderView* m_vrv;
	double m_max_val;

	//1st line
	//gamma
	wxSlider *m_gamma_sldr;
	wxTextCtrl *m_gamma_text;
	//boundary
	wxSlider *m_boundary_sldr;
	wxTextCtrl *m_boundary_text;

	//2nd line
	//saturation point
	wxSlider *m_contrast_sldr;
	wxTextCtrl *m_contrast_text;
	//thresholds
	wxStaticText *m_threh_st;
	wxSlider *m_left_thresh_sldr;
	wxTextCtrl *m_left_thresh_text;
	wxSlider *m_right_thresh_sldr;
	wxTextCtrl *m_right_thresh_text;

	//3rd line
	//luminance
	wxSlider *m_luminance_sldr;
	wxTextCtrl* m_luminance_text;
	//highlight
	wxSlider *m_hi_shading_sldr;
	wxTextCtrl *m_hi_shading_text;

	//4th line
	//alpha
	wxToolBar *m_alpha_tool;
	wxSlider *m_alpha_sldr;
	wxTextCtrl* m_alpha_text;
	//sample rate
	wxSlider *m_sample_sldr;
	wxTextCtrl *m_sample_text;

	//5th line
	//shading
	wxSlider *m_low_shading_sldr;
	wxTextCtrl *m_low_shading_text;
	wxToolBar *m_shade_tool;
	//shadow
	wxSlider *m_shadow_sldr;
	wxTextCtrl *m_shadow_text;
	wxToolBar *m_shadow_tool;

	//6th line
	//color
	//wxColourPickerCtrl *m_color_picker;
	wxTextCtrl *m_color_text;
	wxColourPickerCtrl *m_color_btn;
	//space
	wxTextCtrl *m_space_x_text;
	wxTextCtrl *m_space_y_text;
	wxTextCtrl *m_space_z_text;
	//colormap
	wxToolBar *m_colormap_tool;
	wxSlider *m_colormap_high_value_sldr;
	wxTextCtrl *m_colormap_high_value_text;
	wxSlider *m_colormap_low_value_sldr;
	wxTextCtrl *m_colormap_low_value_text;

	//buttons
	wxToolBar *m_options_toolbar;

private:
	void GetSettings();
	bool SetSpacings();

	//1
	void OnGammaChange(wxScrollEvent &event);
	void OnGammaText(wxCommandEvent &event);
	void OnBoundaryChange(wxScrollEvent &event);
	void OnBoundaryText(wxCommandEvent &event);
	//2
	void OnContrastChange(wxScrollEvent &event);
	void OnContrastText(wxCommandEvent &event);
	void OnLeftThreshChange(wxScrollEvent &event);
	void OnLeftThreshText(wxCommandEvent &event);
	void OnRightThreshChange(wxScrollEvent &event);
	void OnRightThreshText(wxCommandEvent &event);
	//3
	void OnLuminanceChange(wxScrollEvent &event);
	void OnLuminanceText(wxCommandEvent &event);
	//shadow
	void OnShadowEnable(wxCommandEvent &event);
	void OnShadowChange(wxScrollEvent &event);
	void OnShadowText(wxCommandEvent &event);
	void OnHiShadingChange(wxScrollEvent &event);
	void OnHiShadingText(wxCommandEvent &event);
	//4
	void OnAlphaCheck(wxCommandEvent &event);
	void OnAlphaChange(wxScrollEvent & event);
	void OnAlphaText(wxCommandEvent& event);
	void OnSampleChange(wxScrollEvent &event);
	void OnSampleText(wxCommandEvent &event);
	//5
	void OnLowShadingChange(wxScrollEvent &event);
	void OnLowShadingText(wxCommandEvent &event);
	void OnShadingEnable(wxCommandEvent &event);
	//colormap
	void OnEnableColormap(wxCommandEvent &event);
	void OnColormapHighValueChange(wxScrollEvent &event);
	void OnColormapHighValueText(wxCommandEvent &event);
	void OnColormapLowValueChange(wxScrollEvent &event);
	void OnColormapLowValueText(wxCommandEvent &event);
	//6
	void OnColorChange(wxColor c);
	void OnColorTextChange(wxCommandEvent& event);
	void OnColorTextFocus(wxCommandEvent& event);
	void OnColorBtn(wxColourPickerEvent& event);
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

	DECLARE_EVENT_TABLE();
};

#endif//_VPROPVIEW_H_
