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
#ifndef _CLIPPINGVIEW_H_
#define _CLIPPINGVIEW_H_

#include <PropPanel.h>
#include <wx/spinbutt.h>

namespace fluo
{
	enum class ClipPlane : int;
}
class wxFadeButton;
class wxDoubleSlider;
class wxSingleSlider;
class wxUndoableToolbar;
class TreeLayer;
class ClipPlanePanel: public TabbedPanel
{
	enum
	{
		ID_LinkChannelsBtn = 0,
		ID_HoldPlanesBtn,
		ID_PlaneModesBtn
	};

public:
	ClipPlanePanel(MainFrame* frame,
		const wxPoint& pos=wxDefaultPosition,
		const wxSize& size=wxDefaultSize,
		long style=0,
		const wxString& name="ClipPlanePanel");
	~ClipPlanePanel();

	virtual void FluoUpdate(const fluo::ValueCollection& vc = {});

	bool GetXLink();
	bool GetYLink();
	bool GetZLink();
	void SetXLink(bool val);
	void SetYLink(bool val);
	void SetZLink(bool val);

	void SetClipValue(fluo::ClipPlane i, int val, bool link = false);//index: 0~5 = X1~Z2
	void SetClipValues(fluo::ClipPlane i, int val1, int val2);//index: clip mask
	void SetClipValues(const std::array<int, 6>& vals);
	void ResetClipValues();
	void ResetClipValues(fluo::ClipPlane i);

	void SyncClipValue(int i);

	//move linked clipping planes
	void MoveLinkedClippingPlanes(int dir);

	void ClearUndo();

private:
	bool m_enable_all;

	//1st line
	wxUndoableToolbar* m_toolbar;
	wxButton *m_clip_reset_btn;
	//fix plane rotations
	wxButton *m_set_zero_btn;
	wxButton *m_rot_reset_btn;

	//buttons
	wxFadeButton* m_clip_x_st;
	wxFadeButton* m_clip_y_st;
	wxFadeButton* m_clip_z_st;
	//sliders for clipping planes
	//x
	wxDoubleSlider *m_clipx_sldr;
	wxTextCtrl *m_x1_clip_text;
	wxTextCtrl *m_x2_clip_text;
	//y
	wxDoubleSlider *m_clipy_sldr;
	wxTextCtrl *m_y1_clip_text;
	wxTextCtrl *m_y2_clip_text;
	//z
	wxDoubleSlider *m_clipz_sldr;
	wxTextCtrl *m_z1_clip_text;
	wxTextCtrl *m_z2_clip_text;

	wxToolBar *m_linkx_tb;
	wxToolBar* m_linky_tb;
	wxToolBar* m_linkz_tb;

	//buttons
	wxButton *m_yz_clip_btn;
	wxButton *m_xz_clip_btn;
	wxButton *m_xy_clip_btn;
	//distance text
	wxTextCtrl *m_yz_dist_text;
	wxTextCtrl *m_xz_dist_text;
	wxTextCtrl *m_xy_dist_text;

	//buttons
	wxFadeButton* m_rot_x_st;
	wxFadeButton* m_rot_y_st;
	wxFadeButton* m_rot_z_st;
	//sliders for rotating clipping planes
	wxSingleSlider *m_x_rot_sldr;
	wxSingleSlider *m_y_rot_sldr;
	wxSingleSlider *m_z_rot_sldr;
	wxTextCtrl *m_x_rot_text;
	wxTextCtrl *m_y_rot_text;
	wxTextCtrl *m_z_rot_text;
	wxSpinButton* m_x_rot_spin;
	wxSpinButton* m_y_rot_spin;
	wxSpinButton* m_z_rot_spin;

private:
	wxWindow* CreateTranslatePage(wxWindow* parent);
	wxWindow* CreateRotatePage(wxWindow* parent);

	void EnableAll(bool val);

	std::shared_ptr<TreeLayer> GetObject();

	void OnIdle(wxIdleEvent &event);

	void OnToolbar(wxCommandEvent& event);
	void LinkChannels();
	void HoldPlanes();
	void SetPlaneMode();

	void OnClipXMF(wxCommandEvent& event);
	void OnClipYMF(wxCommandEvent& event);
	void OnClipZMF(wxCommandEvent& event);

	void OnClipXChange(wxScrollEvent& event);
	void OnClipYChange(wxScrollEvent& event);
	void OnClipZChange(wxScrollEvent& event);

	void OnX1ClipEdit(wxCommandEvent& event);
	void OnX2ClipEdit(wxCommandEvent& event);
	void OnY1ClipEdit(wxCommandEvent& event);
	void OnY2ClipEdit(wxCommandEvent& event);
	void OnZ1ClipEdit(wxCommandEvent& event);
	void OnZ2ClipEdit(wxCommandEvent& event);

	void OnLinkXCheck(wxCommandEvent& event);
	void OnLinkYCheck(wxCommandEvent& event);
	void OnLinkZCheck(wxCommandEvent& event);

	//mouse
	void UpdateSampleRate();
	void OnClipXRClick(wxMouseEvent& event);
	void OnClipYRClick(wxMouseEvent& event);
	void OnClipZRClick(wxMouseEvent& event);

	//clip buttons
	void OnYZClipBtn(wxCommandEvent& event);
	void OnXZClipBtn(wxCommandEvent& event);
	void OnXYClipBtn(wxCommandEvent& event);

	void OnClipDistXEdit(wxCommandEvent& event);
	void OnClipDistYEdit(wxCommandEvent& event);
	void OnClipDistZEdit(wxCommandEvent& event);

	void OnClipResetBtn(wxCommandEvent& event);

	void OnSetZeroBtn(wxCommandEvent& event);
	void OnRotResetBtn(wxCommandEvent& event);

	void OnRotXMF(wxCommandEvent& event);
	void OnRotYMF(wxCommandEvent& event);
	void OnRotZMF(wxCommandEvent& event);

	void OnXRotChange(wxScrollEvent& event);
	void OnYRotChange(wxScrollEvent& event);
	void OnZRotChange(wxScrollEvent& event);
	void OnXRotEdit(wxCommandEvent& event);
	void OnYRotEdit(wxCommandEvent& event);
	void OnZRotEdit(wxCommandEvent& event);

	//spin buttons
	void OnXRotSpinUp(wxSpinEvent& event);
	void OnXRotSpinDown(wxSpinEvent& event);
	void OnYRotSpinUp(wxSpinEvent& event);
	void OnYRotSpinDown(wxSpinEvent& event);
	void OnZRotSpinUp(wxSpinEvent& event);
	void OnZRotSpinDown(wxSpinEvent& event);
};

#endif//_CLIPPINGVIEW_H_
