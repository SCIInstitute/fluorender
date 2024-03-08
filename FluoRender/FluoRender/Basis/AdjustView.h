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

#include <PropPanel.h>
#include <Types/Color.h>

class VRenderFrame;
class VRenderGLView;
class VolumeData;
class DataGroup;
class wxSingleSlider;
class AdjustView: public PropPanel
{
public:
	AdjustView(VRenderFrame* frame,
		const wxPoint& pos=wxDefaultPosition,
		const wxSize& size=wxDefaultSize,
		long style=0,
		const wxString& name="AdjustView");
	~AdjustView();

	virtual void FluoUpdate(const fluo::ValueCollection& vc = {});

	//disable/enable
	void EnableAll(bool val);
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

	void SetSync(int i, bool val, bool update = true);
	void SetGamma(int i, double val, bool update = true);
	void SetBrightness(int i, double val, bool update = true);
	void SetHdr(int i, double val, bool update = true);

	void UpdateSync();

	void ClearUndo();

private:
	VRenderFrame* m_frame;
	int m_type;
	VRenderGLView *m_view;
	VolumeData* m_vd;
	DataGroup* m_group;
	bool m_link_group;
	bool m_enable_all;
	//sync flags
	bool m_sync[3];//for rgb

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
	//multifunc
	void OnRGammaMF(wxCommandEvent& event);
	void OnGGammaMF(wxCommandEvent& event);
	void OnBGammaMF(wxCommandEvent& event);
	void OnRBrightnessMF(wxCommandEvent& event);
	void OnGBrightnessMF(wxCommandEvent& event);
	void OnBBrightnessMF(wxCommandEvent& event);
	void OnRHdrMF(wxCommandEvent& event);
	void OnGHdrMF(wxCommandEvent& event);
	void OnBHdrMF(wxCommandEvent& event);
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

	void SyncColor(fluo::Color& c, double val);
	void SyncGamma(fluo::Color& c, int i, double val, fluo::ValueCollection& vc);
	void SyncBrightness(fluo::Color& c, int i, double val, fluo::ValueCollection& vc);
	void SyncHdr(fluo::Color& c, int i, double val, fluo::ValueCollection& vc);
	void SyncGamma(int i);
	void SyncBrightness(int i);
	void SyncHdr(int i);
};

#endif//_ADJUSTVIEW_H_
