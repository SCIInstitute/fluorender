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
#ifndef _CLIPPLANEPANEL_H_
#define _CLIPPLANEPANEL_H_

#include <wx/wx.h>
#include <wx/panel.h>
#include <wx/slider.h>
#include <wx/spinbutt.h>
#include <Fui/ClipPlaneAgent.h>

namespace FUI
{
	class ClipPlanePanel : public wxPanel
	{
		enum
		{
			ID_LinkChannelsBtn = ID_CLIP_VIEW,
			ID_HoldPlanesBtn,
			ID_PlaneModesBtn,
			ID_SaveBtn,
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
			ID_YZDistText,
			ID_XZDistText,
			ID_XYDistText
		};

	public:
		//plane modes
		enum PLANE_MODES
		{
			kNormal,
			kFrame,
			kLowTrans,
			kLowTransBack,
			kNormalBack,
			kNone
		};

		ClipPlanePanel(wxWindow* frame,
			wxWindow* parent,
			wxWindowID id,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = 0,
			const wxString& name = "ClipPlanePanel");
		~ClipPlanePanel();

		void AssociateNode(FL::Node* node);

		bool GetChannLink()
		{
			return m_toolbar->GetToolState(ID_LinkChannelsBtn);
		}
		void SetChannLink(bool chann);
		bool GetHoldPlanes()
		{
			return m_hold_planes;
		}
		void SetHoldPlanes(bool hold);
		PLANE_MODES GetPlaneMode()
		{
			return m_plane_mode;
		}
		void SetPlaneMode(PLANE_MODES mode);

		//move linked clipping planes
		//dir: 0-lower; 1-higher
		void MoveLinkedClippingPlanes(int dir);

	private:
		wxWindow* m_frame;

		ClipPlaneAgent* m_agent;

		int m_sel_type;		//curent selection type
		bool m_draw_clip;
		bool m_hold_planes;
		PLANE_MODES m_plane_mode;

		int m_x_sldr_dist;
		int m_y_sldr_dist;
		int m_z_sldr_dist;

		//1st line
		wxToolBar *m_toolbar;
		wxButton *m_clip_reset_btn;
		//fix plane rotations
		wxButton *m_set_zero_btn;
		wxButton *m_rot_reset_btn;

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
		//highlighters
		wxStaticText * m_xBar, *m_yBar, *m_zBar;
		//slider panels
		wxPanel *m_xpanel, *m_ypanel, *m_zpanel;
		//link position
		wxToolBar *m_link_x_tb;
		wxToolBar *m_link_y_tb;
		wxToolBar *m_link_z_tb;
		//distance text
		wxTextCtrl *m_yz_dist_text;
		wxTextCtrl *m_xz_dist_text;
		wxTextCtrl *m_xy_dist_text;

		//sliders for rotating clipping planes
		wxTextCtrl *m_x_rot_text;
		wxTextCtrl *m_y_rot_text;
		wxTextCtrl *m_z_rot_text;
		wxScrollBar *m_x_rot_sldr;
		wxScrollBar *m_y_rot_sldr;
		wxScrollBar *m_z_rot_sldr;

		friend class ClipPlaneAgent;

	private:
		void OnResize(wxSizeEvent &event);
		void OnIdle(wxIdleEvent &event);

		void OnLinkChannelsBtn(wxCommandEvent &event);
		void OnHoldPlanesBtn(wxCommandEvent &event);
		void OnPlaneModesBtn(wxCommandEvent &event);
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

		void OnClipDistXEdit(wxCommandEvent &event);
		void OnClipDistYEdit(wxCommandEvent &event);
		void OnClipDistZEdit(wxCommandEvent &event);

		void OnSetZeroBtn(wxCommandEvent &event);
		void OnRotResetBtn(wxCommandEvent &event);

		void OnXRotChange(wxScrollEvent &event);
		void OnYRotChange(wxScrollEvent &event);
		void OnZRotChange(wxScrollEvent &event);
		void OnXRotEdit(wxCommandEvent &event);
		void OnYRotEdit(wxCommandEvent &event);
		void OnZRotEdit(wxCommandEvent &event);

		//mouse
		void OnSliderRClick(wxCommandEvent& event);

		//key down
		void OnSliderKeyDown(wxKeyEvent& event);

		DECLARE_EVENT_TABLE()

	};
}
#endif//_CLIPPLANEPANEL_H_