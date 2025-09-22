/*
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
#ifndef _ADJUSTVIEW_H_
#define _ADJUSTVIEW_H_

#include <PropPanel.h>

class VolumeGroup;
class wxFadeButton;
class wxSingleSlider;
class wxUndoableToolbar;
namespace fluo
{
	class Color;
}
class OutputAdjPanel: public TabbedPanel
{
public:
	OutputAdjPanel(MainFrame* frame,
		const wxPoint& pos=wxDefaultPosition,
		const wxSize& size=wxDefaultSize,
		long style=0,
		const wxString& name="OutputAdjPanel");
	~OutputAdjPanel();

	virtual void FluoUpdate(const fluo::ValueCollection& vc = {});
	virtual void LoadPerspective(const std::string& str) override;

	void UpdateSync();

	//disable/enable
	void EnableAll(bool val);

	void SetSync(int i, bool val, bool update = true);
	void SetGamma(int i, double val, bool update = true);
	void SetBrightness(int i, double val, bool update = true);
	void SetHdr(int i, double val, bool update = true);


	void ClearUndo();

private:
	bool m_enable_all;
	//sync flags
	bool m_sync[3];//for rgb

	//sync red
	wxUndoableToolbar* m_sync_r_chk;
	//buttons
	wxFadeButton* m_r_gamma_st;
	wxFadeButton* m_r_brightness_st;
	wxFadeButton* m_r_hdr_st;
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
	wxUndoableToolbar* m_sync_g_chk;
	//buttons
	wxFadeButton* m_g_gamma_st;
	wxFadeButton* m_g_brightness_st;
	wxFadeButton* m_g_hdr_st;
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
	wxUndoableToolbar* m_sync_b_chk;
	//buttons
	wxFadeButton* m_b_gamma_st;
	wxFadeButton* m_b_brightness_st;
	wxFadeButton* m_b_hdr_st;
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
	wxWindow* CreateRedPage(wxWindow* parent, wxSize& size);
	wxWindow* CreateGreenPage(wxWindow* parent, wxSize& size);
	wxWindow* CreateBluePage(wxWindow* parent, wxSize& size);

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
	void OnRGammaChange(wxScrollEvent& event);
	void OnRGammaText(wxCommandEvent& event);
	void OnGGammaChange(wxScrollEvent& event);
	void OnGGammaText(wxCommandEvent& event);
	void OnBGammaChange(wxScrollEvent& event);
	void OnBGammaText(wxCommandEvent& event);
	//brightness
	void OnRBrightnessChange(wxScrollEvent& event);
	void OnRBrightnessText(wxCommandEvent& event);
	void OnGBrightnessChange(wxScrollEvent& event);
	void OnGBrightnessText(wxCommandEvent& event);
	void OnBBrightnessChange(wxScrollEvent& event);
	void OnBBrightnessText(wxCommandEvent& event);
	//hdr
	void OnRHdrChange(wxScrollEvent& event);
	void OnRHdrText(wxCommandEvent& event);
	void OnGHdrChange(wxScrollEvent& event);
	void OnGHdrText(wxCommandEvent& event);
	void OnBHdrChange(wxScrollEvent& event);
	void OnBHdrText(wxCommandEvent& event);
	//reset
	void OnRReset(wxCommandEvent& event);
	void OnGReset(wxCommandEvent& event);
	void OnBReset(wxCommandEvent& event);
	//sync check
	void OnSyncRCheck(wxCommandEvent& event);
	void OnSyncGCheck(wxCommandEvent& event);
	void OnSyncBCheck(wxCommandEvent& event);
	//set default
	void OnSaveDefault(wxCommandEvent& event);

	void SyncColor(fluo::Color& c, double val);
	void SyncGamma(fluo::Color& c, int i, double val, fluo::ValueCollection& vc, bool notify);
	void SyncBrightness(fluo::Color& c, int i, double val, fluo::ValueCollection& vc, bool notify);
	void SyncHdr(fluo::Color& c, int i, double val, fluo::ValueCollection& vc, bool notify);
	void SyncGamma(int i);
	void SyncBrightness(int i);
	void SyncHdr(int i);
};

#endif//_ADJUSTVIEW_H_
