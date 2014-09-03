#include "DataManager.h"
#include "VRenderView.h"
#include <wx/wx.h>
#include <wx/panel.h>
#include <wx/slider.h>

#ifndef _ADJUSTVIEW_H_
#define _ADJUSTVIEW_H_

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

class AdjustView: public wxPanel
{
	enum
	{
		//gamma
		ID_RGammaSldr = wxID_HIGHEST+1101,
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
	AdjustView(wxWindow* frame,
			   wxWindow* parent,
			   wxWindowID id,
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
	void SetRenderView(VRenderView *view)
	{
		if (view)
		{
			m_glview = view->m_glview;
			m_type = 1;
			GetSettings();
		}
		else
		{
			m_type = -1;
			DisableAll();
		}
	}
	VRenderGLView* GetRenderView()
	{
		return m_glview;
	}
	//set volume data
	void SetVolumeData(VolumeData* vd)
	{
		if (vd)
		{
			m_vd = vd;
			m_type = 2;
			GetSettings();
		}
		else
		{
			m_type = -1;
			DisableAll();
		}
	}
	VolumeData* GetVolumeData()
	{
		return m_vd;
	}
	//set group
	void SetGroup(DataGroup *group)
	{
		if (group)
		{
			m_group = group;
			m_type = 5;
			GetSettings();
		}
		else
		{
			m_type = -1;
			DisableAll();
		}
	}
	DataGroup* GetGroup()
	{
		return m_group;
	}
	//set volume adjustment to link to group
	void SetGroupLink(DataGroup *group)
	{
		if (group)
		{
			m_group = group;
			m_link_group = true;
		}
		else
		{
			m_group = 0;
			m_link_group = false;
		}
	}

	//load default settings
	void LoadSettings();

	//get defaults
	void GetDefaults(Color &gamma, Color &brightness, Color &hdr,
		bool &snyc_r, bool &sync_g, bool &sync_b);

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

private:
	wxWindow* m_frame;
	int m_type;
	VRenderGLView *m_glview;
	VolumeData* m_vd;
	DataGroup* m_group;
	bool m_link_group;

	//sync flags
	bool m_sync_r;
	bool m_sync_g;
	bool m_sync_b;

	//default values
	bool m_use_dft_settings;
	Color m_dft_gamma;
	Color m_dft_brightness;
	Color m_dft_hdr;
	bool m_dft_sync_r;
	bool m_dft_sync_g;
	bool m_dft_sync_b;

	//sync red
	wxToolBar *m_sync_r_chk;
	//red sliders
	wxSlider *m_r_gamma_sldr;
	wxSlider *m_r_brightness_sldr;
	wxSlider *m_r_hdr_sldr;
	//red reset buttons
	wxButton *m_r_reset_btn;
	//wxButton *m_r_brightness_reset_btn;
	//red input boxes
	wxTextCtrl *m_r_gamma_text;
	wxTextCtrl *m_r_brightness_text;
	wxTextCtrl *m_r_hdr_text;

	//sync green
	wxToolBar *m_sync_g_chk;
	//green sliders
	wxSlider *m_g_gamma_sldr;
	wxSlider *m_g_brightness_sldr;
	wxSlider *m_g_hdr_sldr;
	//green reset buttons
	wxButton *m_g_reset_btn;
	//wxButton *m_g_brightness_reset_btn;
	//green input boxes
	wxTextCtrl *m_g_gamma_text;
	wxTextCtrl *m_g_brightness_text;
	wxTextCtrl *m_g_hdr_text;

	//sync blue
	wxToolBar *m_sync_b_chk;
	//blue sliders
	wxSlider *m_b_gamma_sldr;
	wxSlider *m_b_brightness_sldr;
	wxSlider *m_b_hdr_sldr;
	//blue reset buttons
	wxButton *m_b_reset_btn;
	//wxButton *m_b_brightness_reset_btn;
	//blue input boxes
	wxTextCtrl *m_b_gamma_text;
	wxTextCtrl *m_b_brightness_text;
	wxTextCtrl *m_b_hdr_text;

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

	DECLARE_EVENT_TABLE();
};

#endif//_ADJUSTVIEW_H_
