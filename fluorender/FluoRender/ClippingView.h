#include "DataManager.h"
#include <wx/wx.h>
#include <wx/panel.h>
#include <wx/slider.h>
#include <wx/spinbutt.h>

#ifndef _CLIPPINGVIEW_H_
#define _CLIPPINGVIEW_H_

class ClippingView: public wxPanel
{
	enum
	{
		ID_LinkChannelsChk = wxID_HIGHEST+1,
		ID_RotatePlanesChk,
		ID_ClipResetBtn,
		ID_SetZeroBtn,
		ID_RotResetBtn,
		//rotation sliders
		ID_XRotSldr,
		ID_YRotSldr,
		ID_ZRotSldr,
		ID_XRotText,
		ID_YRotText,
		ID_ZRotText,
		ID_XRotSpin,
		ID_YRotSpin,
		ID_ZRotSpin,
		//clipping sliders
		ID_X1ClipSldr,
		ID_X2ClipSldr,
		ID_Y1ClipSldr,
		ID_Y2ClipSldr,
		ID_Z1ClipSldr,
		ID_Z2ClipSldr,
		ID_X1ClipText,
		ID_X2ClipText,
		ID_Y1ClipText,
		ID_Y2ClipText,
		ID_Z1ClipText,
		ID_Z2ClipText,
		ID_LinkXChk,
		ID_LinkYChk,
		ID_LinkZChk,
		ID_YZClipBtn,
		ID_XZClipBtn,
		ID_XYClipBtn,
		ID_YZDistText,
		ID_XZDistText,
		ID_XYDistText
	};

public:
	ClippingView(wxWindow* frame,
		wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos=wxDefaultPosition,
		const wxSize& size=wxDefaultSize,
		long style=0,
		const wxString& name="ClippingView");
	~ClippingView();

	void SetVolumeData(VolumeData* vd);
	void SetMeshData(MeshData* md);
	void SetDataManager(DataManager* mgr);
	int GetSelType();
	VolumeData* GetVolumeData();
	MeshData* GetMeshData();
	void RefreshVRenderViews(bool interactive=false);

	bool GetChannLink()
	{
		return m_link_channels->GetToolState(ID_LinkChannelsChk);
	}
	void SetChannLink(bool chann);
	bool GetXLink()
	{
		return m_check_tb->GetToolState(ID_LinkXChk);
	}
	bool GetYLink()
	{
		return m_check_tb->GetToolState(ID_LinkYChk);
	}
	bool GetZLink()
	{
		return m_check_tb->GetToolState(ID_LinkZChk);
	}
	void SetXLink(bool link)
	{
		m_check_tb->ToggleTool(ID_LinkXChk,link);
		m_link_x = link;
		wxCommandEvent ev;
		OnLinkXCheck(ev);
	}
	void SetYLink(bool link)
	{
		m_check_tb->ToggleTool(ID_LinkYChk,link);
		m_link_y = link;
		wxCommandEvent ev;
		OnLinkYCheck(ev);
	}
	void SetZLink(bool link)
	{
		m_check_tb->ToggleTool(ID_LinkZChk,link);
		m_link_z  = link;
		wxCommandEvent ev;
		OnLinkZCheck(ev);
	}

	void SetClippingPlaneRotations(double rotx, double roty, double rotz)
	{
		m_x_rot_sldr->SetValue(int(rotx));
		m_y_rot_sldr->SetValue(int(roty));
		m_z_rot_sldr->SetValue(int(rotz));
		m_x_rot_text->SetValue(wxString::Format("%.1f", rotx));
		m_y_rot_text->SetValue(wxString::Format("%.1f", roty));
		m_z_rot_text->SetValue(wxString::Format("%.1f", rotz));
	}

	//move linked clipping planes
	//dir: 0-lower; 1-higher
	void MoveLinkedClippingPlanes(int dir);

private:
	wxWindow* m_frame;

	int m_sel_type;		//curent selection type
	VolumeData* m_vd;	//current volume data
	MeshData* m_md;		//current mesh data
	DataManager* m_mgr;	//manage all if clipping planes are synced
	bool m_draw_clip;

	int m_x_sldr_dist;
	int m_y_sldr_dist;
	int m_z_sldr_dist;
	bool m_link_x;
	bool m_link_y;
	bool m_link_z;

	//1st line
	wxToolBar *m_link_channels;
	wxButton *m_clip_reset_btn;
	//fix plane rotations
	wxButton *m_set_zero_btn;
	wxButton *m_rot_reset_btn;

	//sliders for rotating clipping planes
	wxSlider *m_x_rot_sldr;
	wxSlider *m_y_rot_sldr;
	wxSlider *m_z_rot_sldr;
	wxTextCtrl *m_x_rot_text;
	wxTextCtrl *m_y_rot_text;
	wxTextCtrl *m_z_rot_text;
	wxSpinButton* m_x_rot_spin;
	wxSpinButton* m_y_rot_spin;
	wxSpinButton* m_z_rot_spin;

	//sliders for clipping planes
	//x1
	wxSlider *m_x1_clip_sldr;
	wxTextCtrl *m_x1_clip_text;
	//x2
	wxSlider *m_x2_clip_sldr;
	wxTextCtrl *m_x2_clip_text;
	//y1
	wxSlider *m_y1_clip_sldr;
	wxTextCtrl *m_y1_clip_text;
	//y2
	wxSlider *m_y2_clip_sldr;
	wxTextCtrl *m_y2_clip_text;
	//z1
	wxSlider *m_z1_clip_sldr;
	wxTextCtrl *m_z1_clip_text;
	//z2
	wxSlider *m_z2_clip_sldr;
	wxTextCtrl *m_z2_clip_text;
	//keep 1 panel for sizing reasons
	wxPanel * m_xpanel;
	//highlighters
	wxStaticText * m_xBar, * m_yBar, * m_zBar;

	wxToolBar * m_check_tb;

	//buttons
	wxButton *m_yz_clip_btn;
	wxButton *m_xz_clip_btn;
	wxButton *m_xy_clip_btn;
	//distance text
	wxTextCtrl *m_yz_dist_text;
	wxTextCtrl *m_xz_dist_text;
	wxTextCtrl *m_xy_dist_text;

private:
	void GetSettings();
	
	void OnIdle(wxIdleEvent &event);

	void OnLinkChannelsCheck(wxCommandEvent &event);
	void OnClipResetBtn(wxCommandEvent &event);

	void EnableAll();
	void DisableAll();

	void OnX1ClipChange(wxScrollEvent &event);
	void OnX2ClipChange(wxScrollEvent &event);
	void OnY1ClipChange(wxScrollEvent &event);
	void OnY2ClipChange(wxScrollEvent &event);
	void OnZ1ClipChange(wxScrollEvent &event);
	void OnZ2ClipChange(wxScrollEvent &event);
	void OnX1ClipEdit(wxCommandEvent &event);
	void OnX2ClipEdit(wxCommandEvent &event);
	void OnY1ClipEdit(wxCommandEvent &event);
	void OnY2ClipEdit(wxCommandEvent &event);
	void OnZ1ClipEdit(wxCommandEvent &event);
	void OnZ2ClipEdit(wxCommandEvent &event);

	void OnLinkXCheck(wxCommandEvent &event);
	void OnLinkYCheck(wxCommandEvent &event);
	void OnLinkZCheck(wxCommandEvent &event);

	void OnSetZeroBtn(wxCommandEvent &event);
	void OnRotResetBtn(wxCommandEvent &event);

	void OnXRotChange(wxScrollEvent &event);
	void OnYRotChange(wxScrollEvent &event);
	void OnZRotChange(wxScrollEvent &event);
	void OnXRotEdit(wxCommandEvent &event);
	void OnYRotEdit(wxCommandEvent &event);
	void OnZRotEdit(wxCommandEvent &event);

	//spin buttons
	void OnXRotSpinUp(wxSpinEvent& event);
	void OnXRotSpinDown(wxSpinEvent& event);
	void OnYRotSpinUp(wxSpinEvent& event);
	void OnYRotSpinDown(wxSpinEvent& event);
	void OnZRotSpinUp(wxSpinEvent& event);
	void OnZRotSpinDown(wxSpinEvent& event);

	//mouse
	void OnSliderRClick(wxCommandEvent& event);

	//clip buttons
	void OnYZClipBtn(wxCommandEvent& event);
	void OnXZClipBtn(wxCommandEvent& event);
	void OnXYClipBtn(wxCommandEvent& event);

	//key down
	void OnSliderKeyDown(wxKeyEvent& event);

	DECLARE_EVENT_TABLE();

};

#endif//_CLIPPINGVIEW_H_
