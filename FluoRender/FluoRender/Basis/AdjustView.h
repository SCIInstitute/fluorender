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
#ifndef _ADJUSTVIEW_H_
#define _ADJUSTVIEW_H_

#include <Types/Color.h>
#include <wx/wx.h>
#include <wx/panel.h>

//all convert v1 to v2
#define GammaUI2(v1, v2) \
	{v2 = 1.0/v1;}
#define Gamma2UI(v1, v2) \
	{v2 = 1.0/v1;}
#define Gamma2UIP(v) \
	int(v*100.0+0.5)
#define BrightnessUI2(v1, v2) \
	{v2 = v1/256.0 + 1.0;}
#define Brightness2UI(v1, v2) \
	{v2 = (v1-1.0)*256.0;}
#define Brightness2UIP(v) \
	int(v+0.5)
#define HdrUI2(v1, v2) \
	{v2 = v1;}
#define Hdr2UI(v1, v2) \
	{v2 = v1;}
#define Hdr2UIP(v) \
	int(v*100.0+0.5)

class VRenderFrame;
class VRenderGLView;
class VolumeData;
class DataGroup;
class wxSingleSlider;
class AdjustView: public wxPanel
{
	enum
	{
		//gamma
		ID_RGammaSldr = ID_ADJUST_VIEW,
		ID_RGammaText,
		ID_GGammaSldr,
		ID_GGammaText,
		ID_BGammaSldr,
		ID_BGammaText,
		//brightness
		ID_RBrightnessSldr,
		ID_RBrightnessText,
		ID_GBrightnessSldr,
		ID_GBrightnessText,
		ID_BBrightnessSldr,
		ID_BBrightnessText,
		//hdr
		ID_RHdrSldr,
		ID_RHdrText,
		ID_GHdrSldr,
		ID_GHdrText,
		ID_BHdrSldr,
		ID_BHdrText,
		//reset
		ID_RResetBtn,
		ID_GResetBtn,
		ID_BResetBtn,
		//sync
		ID_SyncRChk,
		ID_SyncGChk,
		ID_SyncBChk,
		ID_DefaultBtn
	};

public:
	AdjustView(VRenderFrame* frame,
		const wxPoint& pos=wxDefaultPosition,
		const wxSize& size=wxDefaultSize,
		long style=0,
		const wxString& name="AdjustView");
	~AdjustView();

	//refresh
	void RefreshVRenderViews(bool interactive=false);
	//get settings
	void GetSettings();
	//disable/enable
	void DisableAll();
	void EnableAll();
	//get type
	int GetType()
	{
		return m_type;
	}
	//set view
	void SetRenderView(VRenderGLView *view);
	VRenderGLView* GetRenderView();
	//set volume data
	void SetVolumeData(VolumeData* vd);
	VolumeData* GetVolumeData();
	//set group
	void SetGroup(DataGroup *group);
	DataGroup* GetGroup();
	//set volume adjustment to link to group
	void SetGroupLink(DataGroup *group);

	//change settings externally
	void ChangeRGamma(double gamma_r);
	void ChangeGGamma(double gamma_g);
	void ChangeBGamma(double gamma_b);
	void ChangeRBrightness(double brightness_r);
	void ChangeGBrightness(double brightness_g);
	void ChangeBBrightness(double brightness_b);
	void ChangeRHdr(double hdr_r);
	void ChangeGHdr(double hdr_g);
	void ChangeBHdr(double hdr_b);
	void ChangeRSync(bool sync_r);
	void ChangeGSync(bool sync_g);
	void ChangeBSync(bool sync_b);

	bool GetRSync() {return m_sync_r;}
	bool GetGSync() {return m_sync_g;}
	bool GetBSync() {return m_sync_b;}

	void UpdateSync();

	void ClearUndo();

private:
	VRenderFrame* m_frame;
	int m_type;
	VRenderGLView *m_view;
	VolumeData* m_vd;
	DataGroup* m_group;
	bool m_link_group;

	//sync flags
	bool m_sync_r;
	bool m_sync_g;
	bool m_sync_b;

	//sync red
	wxToolBar *m_sync_r_chk;
	//buttons
	wxButton* m_r_gamma_st;
	wxButton* m_r_brightness_st;
	wxButton* m_r_hdr_st;
	//red sliders
	wxSingleSlider *m_r_gamma_sldr;
	wxSingleSlider *m_r_brightness_sldr;
	wxSingleSlider *m_r_hdr_sldr;
	//red input boxes
	wxTextCtrl *m_r_gamma_text;
	wxTextCtrl *m_r_brightness_text;
	wxTextCtrl *m_r_hdr_text;
	//red reset buttons
	wxButton *m_r_reset_btn;

	//sync green
	wxToolBar *m_sync_g_chk;
	//buttons
	wxButton* m_g_gamma_st;
	wxButton* m_g_brightness_st;
	wxButton* m_g_hdr_st;
	//green sliders
	wxSingleSlider *m_g_gamma_sldr;
	wxSingleSlider *m_g_brightness_sldr;
	wxSingleSlider *m_g_hdr_sldr;
	//green input boxes
	wxTextCtrl *m_g_gamma_text;
	wxTextCtrl *m_g_brightness_text;
	wxTextCtrl *m_g_hdr_text;
	//green reset buttons
	wxButton *m_g_reset_btn;

	//sync blue
	wxToolBar *m_sync_b_chk;
	//buttons
	wxButton* m_b_gamma_st;
	wxButton* m_b_brightness_st;
	wxButton* m_b_hdr_st;
	//blue sliders
	wxSingleSlider *m_b_gamma_sldr;
	wxSingleSlider *m_b_brightness_sldr;
	wxSingleSlider *m_b_hdr_sldr;
	//blue input boxes
	wxTextCtrl *m_b_gamma_text;
	wxTextCtrl *m_b_brightness_text;
	wxTextCtrl *m_b_hdr_text;
	//blue reset buttons
	wxButton *m_b_reset_btn;

	//set default
	wxButton *m_dft_btn;

private:
	//gamma
	void OnRGammaChange(wxScrollEvent &event);
	void OnRGammaText(wxCommandEvent &event);
	void OnGGammaChange(wxScrollEvent &event);
	void OnGGammaText(wxCommandEvent &event);
	void OnBGammaChange(wxScrollEvent &event);
	void OnBGammaText(wxCommandEvent &event);
	//brightness
	void OnRBrightnessChange(wxScrollEvent &event);
	void OnRBrightnessText(wxCommandEvent &event);
	void OnGBrightnessChange(wxScrollEvent &event);
	void OnGBrightnessText(wxCommandEvent &event);
	void OnBBrightnessChange(wxScrollEvent &event);
	void OnBBrightnessText(wxCommandEvent &event);
	//hdr
	void OnRHdrChange(wxScrollEvent &event);
	void OnRHdrText(wxCommandEvent &event);
	void OnGHdrChange(wxScrollEvent &event);
	void OnGHdrText(wxCommandEvent &event);
	void OnBHdrChange(wxScrollEvent &event);
	void OnBHdrText(wxCommandEvent &event);
	//reset
	void OnRReset(wxCommandEvent &event);
	void OnGReset(wxCommandEvent &event);
	void OnBReset(wxCommandEvent &event);
	//sync check
	void OnSyncRCheck(wxCommandEvent &event);
	void OnSyncGCheck(wxCommandEvent &event);
	void OnSyncBCheck(wxCommandEvent &event);
	//set default
	void OnSaveDefault(wxCommandEvent &event);

	DECLARE_EVENT_TABLE()
};

#endif//_ADJUSTVIEW_H_
