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
#include <Fui/ClipPlanePanel.h>
#include <Global/Global.h>
#include <wx/valnum.h>
#include <png_resource.h>
#include <img/icons.h>

using namespace FUI;

BEGIN_EVENT_TABLE(ClipPlanePanel, wxPanel)
EVT_SIZE(ClipPlanePanel::OnResize)

EVT_TOOL(ID_LinkChannelsBtn, ClipPlanePanel::OnLinkChannelsBtn)
EVT_TOOL(ID_HoldPlanesBtn, ClipPlanePanel::OnHoldPlanesBtn)
EVT_TOOL(ID_PlaneModesBtn, ClipPlanePanel::OnPlaneModesBtn)
EVT_BUTTON(ID_SetZeroBtn, ClipPlanePanel::OnSetZeroBtn)
EVT_BUTTON(ID_RotResetBtn, ClipPlanePanel::OnRotResetBtn)
EVT_BUTTON(ID_ClipResetBtn, ClipPlanePanel::OnClipResetBtn)

EVT_COMMAND_SCROLL(ID_X1ClipSldr, ClipPlanePanel::OnX1ClipChange)
EVT_COMMAND_SCROLL(ID_X2ClipSldr, ClipPlanePanel::OnX2ClipChange)
EVT_COMMAND_SCROLL(ID_Y1ClipSldr, ClipPlanePanel::OnY1ClipChange)
EVT_COMMAND_SCROLL(ID_Y2ClipSldr, ClipPlanePanel::OnY2ClipChange)
EVT_COMMAND_SCROLL(ID_Z1ClipSldr, ClipPlanePanel::OnZ1ClipChange)
EVT_COMMAND_SCROLL(ID_Z2ClipSldr, ClipPlanePanel::OnZ2ClipChange)

EVT_TEXT(ID_X1ClipText, ClipPlanePanel::OnX1ClipEdit)
EVT_TEXT(ID_X2ClipText, ClipPlanePanel::OnX2ClipEdit)
EVT_TEXT(ID_Y1ClipText, ClipPlanePanel::OnY1ClipEdit)
EVT_TEXT(ID_Y2ClipText, ClipPlanePanel::OnY2ClipEdit)
EVT_TEXT(ID_Z1ClipText, ClipPlanePanel::OnZ1ClipEdit)
EVT_TEXT(ID_Z2ClipText, ClipPlanePanel::OnZ2ClipEdit)

EVT_IDLE(ClipPlanePanel::OnIdle)

EVT_TOOL(ID_LinkXChk, ClipPlanePanel::OnLinkXCheck)
EVT_TOOL(ID_LinkYChk, ClipPlanePanel::OnLinkYCheck)
EVT_TOOL(ID_LinkZChk, ClipPlanePanel::OnLinkZCheck)

EVT_TEXT(ID_YZDistText, ClipPlanePanel::OnClipDistXEdit)
EVT_TEXT(ID_XZDistText, ClipPlanePanel::OnClipDistYEdit)
EVT_TEXT(ID_XYDistText, ClipPlanePanel::OnClipDistZEdit)

EVT_COMMAND_SCROLL(ID_XRotSldr, ClipPlanePanel::OnXRotChange)
EVT_COMMAND_SCROLL(ID_YRotSldr, ClipPlanePanel::OnYRotChange)
EVT_COMMAND_SCROLL(ID_ZRotSldr, ClipPlanePanel::OnZRotChange)
EVT_TEXT(ID_XRotText, ClipPlanePanel::OnXRotEdit)
EVT_TEXT(ID_YRotText, ClipPlanePanel::OnYRotEdit)
EVT_TEXT(ID_ZRotText, ClipPlanePanel::OnZRotEdit)

//spin buttons
EVT_SPIN_UP(ID_XRotSpin, ClipPlanePanel::OnXRotSpinUp)
EVT_SPIN_DOWN(ID_XRotSpin, ClipPlanePanel::OnXRotSpinDown)
EVT_SPIN_UP(ID_YRotSpin, ClipPlanePanel::OnYRotSpinUp)
EVT_SPIN_DOWN(ID_YRotSpin, ClipPlanePanel::OnYRotSpinDown)
EVT_SPIN_UP(ID_ZRotSpin, ClipPlanePanel::OnZRotSpinUp)
EVT_SPIN_DOWN(ID_ZRotSpin, ClipPlanePanel::OnZRotSpinDown)
END_EVENT_TABLE()

ClipPlanePanel::ClipPlanePanel(wxWindow* frame,
	wxWindow* parent,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name) :
	wxPanel(parent, id, pos, size, style, name),
	m_frame(frame),
	m_sel_type(0),
	m_draw_clip(false),
	m_hold_planes(false),
	m_plane_mode(kNormal)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);

	m_agent =
		FL::Global::instance().getAgentFactory().
		getOrAddClipPlaneAgent("ClipPlanePanel", *this);

	//validator: floating point 1
	wxFloatingPointValidator<double> vald_fp1(1);
	vald_fp1.SetRange(-180.0, 180.0);
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;

	wxStaticText *st = 0;

	SetDoubleBuffered(true);

	//sync channels 1
	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	m_toolbar = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	wxBitmap bitmap;
	bitmap = wxGetBitmapFromMemory(sync_chan);
#ifdef _DARWIN
	m_toolbar->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_toolbar->AddCheckTool(ID_LinkChannelsBtn, "Sync All Channels",
		bitmap, wxNullBitmap,
		"Link all data channels to this cropping",
		"Link all data channels to this cropping");
	m_toolbar->ToggleTool(ID_LinkChannelsBtn, false);
	bitmap = wxGetBitmapFromMemory(hold_clip);
	m_toolbar->AddCheckTool(ID_HoldPlanesBtn, "Hold Plane Display",
		bitmap, wxNullBitmap,
		"Clipping planes are always shown",
		"Clipping planes are always shown");
	m_toolbar->ToggleTool(ID_HoldPlanesBtn, false);
	bitmap = wxGetBitmapFromMemory(clip_normal);
	m_toolbar->AddTool(ID_PlaneModesBtn, "Display Modes",
		bitmap, "Toggle clipping plane display modes");
	bitmap = wxGetBitmapFromMemory(save_settings);
	m_toolbar->AddTool(ID_SaveBtn, "Save Default Settings",
		bitmap, "Save default settings");
	m_toolbar->Realize();
	sizer_1->Add(5, 5, 0);
	sizer_1->Add(m_toolbar, 0, wxALIGN_CENTER, 0);

	wxStaticText* st_cb = 0;

	//sliders for clipping planes
	//x
	wxBoxSizer* sizer_cx = new wxBoxSizer(wxVERTICAL);
	st = new wxStaticText(this, 0, "X");
	m_x1_clip_text = new wxTextCtrl(this, ID_X1ClipText, "0",
		wxDefaultPosition, wxSize(34, 20), 0, vald_int);
	m_xpanel = new wxPanel(this);
	m_x1_clip_sldr = new wxSlider(m_xpanel, ID_X1ClipSldr, 0, 0, 512,
		wxPoint(0, 0), wxSize(20, -1), wxSL_RIGHT);
	m_xBar = new wxStaticText(m_xpanel, 0, "",
		wxPoint(20, 10), wxSize(3, 0));
	m_xBar->SetBackgroundColour(wxColor(255, 128, 128));
	m_x2_clip_sldr = new wxSlider(m_xpanel, ID_X2ClipSldr, 512, 0, 512,
		wxPoint(23, 0), wxDefaultSize, wxSL_LEFT);
	//
	st_cb = new wxStaticText(this, 0, "",
		wxDefaultPosition, wxSize(5, 5));
	st_cb->SetBackgroundColour(wxColor(255, 128, 128));
	m_x2_clip_text = new wxTextCtrl(this, ID_X2ClipText, "512",
		wxDefaultPosition, wxSize(34, 20), 0, vald_int);
	//link x
	m_link_x_tb = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	bitmap = wxGetBitmapFromMemory(unlink);
#ifdef _DARWIN
	m_link_x_tb->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_link_x_tb->AddCheckTool(ID_LinkXChk, "Lock X Planes",
		bitmap, wxNullBitmap, "Lock X Planes");
	m_link_x_tb->Realize();
	//add the items
	sizer_cx->Add(5, 5, 0);
	sizer_cx->Add(st, 0, wxALIGN_CENTER, 0);
	sizer_cx->Add(m_x1_clip_text, 0, wxALIGN_CENTER, 0);
	sizer_cx->Add(5, 5, 0);
	sizer_cx->Add(st_cb, 0, wxEXPAND);
	sizer_cx->Add(m_xpanel, 1, wxALIGN_CENTER, 0);
	st_cb = new wxStaticText(this, 0, "",
		wxDefaultPosition, wxSize(5, 5));
	st_cb->SetBackgroundColour(wxColor(255, 128, 255));
	sizer_cx->Add(st_cb, 0, wxEXPAND);
	sizer_cx->Add(5, 5, 0);
	sizer_cx->Add(m_x2_clip_text, 0, wxALIGN_CENTER, 0);
	sizer_cx->Add(5, 5, 0);
	sizer_cx->Add(m_link_x_tb, 0, wxALIGN_CENTER, 0);

	//y
	wxBoxSizer* sizer_cy = new wxBoxSizer(wxVERTICAL);
	st = new wxStaticText(this, 0, "Y");
	m_y1_clip_text = new wxTextCtrl(this, ID_Y1ClipText, "0",
		wxDefaultPosition, wxSize(34, 20), 0, vald_int);
	m_ypanel = new wxPanel(this);
	m_y1_clip_sldr = new wxSlider(m_ypanel, ID_Y1ClipSldr, 0, 0, 512,
		wxPoint(0, 0), wxSize(20, -1), wxSL_RIGHT);
	m_yBar = new wxStaticText(m_ypanel, 0, "",
		wxPoint(20, 10), wxSize(3, 0));
	m_yBar->SetBackgroundColour(wxColor(128, 255, 128));
	m_y2_clip_sldr = new wxSlider(m_ypanel, ID_Y2ClipSldr, 512, 0, 512,
		wxPoint(23, 0), wxDefaultSize, wxSL_LEFT);
	//
	st_cb = new wxStaticText(this, 0, "",
		wxDefaultPosition, wxSize(5, 5));
	st_cb->SetBackgroundColour(wxColor(128, 255, 128));
	m_y2_clip_text = new wxTextCtrl(this, ID_Y2ClipText, "512",
		wxDefaultPosition, wxSize(34, 20), 0, vald_int);
	//link y
	m_link_y_tb = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	bitmap = wxGetBitmapFromMemory(unlink);
#ifdef _DARWIN
	m_link_y_tb->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_link_y_tb->AddCheckTool(ID_LinkYChk, "Lock Y Planes",
		bitmap, wxNullBitmap, "Lock Y Planes");
	m_link_y_tb->Realize();
	//add the items
	sizer_cy->Add(5, 5, 0);
	sizer_cy->Add(st, 0, wxALIGN_CENTER, 0);
	sizer_cy->Add(m_y1_clip_text, 0, wxALIGN_CENTER, 0);
	sizer_cy->Add(5, 5, 0);
	sizer_cy->Add(st_cb, 0, wxEXPAND);
	sizer_cy->Add(m_ypanel, 1, wxALIGN_CENTER, 0);
	st_cb = new wxStaticText(this, 0, "",
		wxDefaultPosition, wxSize(5, 5));
	st_cb->SetBackgroundColour(wxColor(255, 255, 128));
	sizer_cy->Add(st_cb, 0, wxEXPAND);
	sizer_cy->Add(5, 5, 0);
	sizer_cy->Add(m_y2_clip_text, 0, wxALIGN_CENTER, 0);
	sizer_cy->Add(5, 5, 0);
	sizer_cy->Add(m_link_y_tb, 0, wxALIGN_CENTER);

	//z
	wxBoxSizer* sizer_cz = new wxBoxSizer(wxVERTICAL);
	st = new wxStaticText(this, 0, "Z");
	m_z1_clip_text = new wxTextCtrl(this, ID_Z1ClipText, "0",
		wxDefaultPosition, wxSize(34, 20), 0, vald_int);
	m_zpanel = new wxPanel(this);
	m_z1_clip_sldr = new wxSlider(m_zpanel, ID_Z1ClipSldr, 0, 0, 512,
		wxPoint(0, 0), wxSize(20, -1), wxSL_RIGHT);
	m_zBar = new wxStaticText(m_zpanel, 0, "",
		wxPoint(20, 10), wxSize(3, 0));
	m_zBar->SetBackgroundColour(wxColor(128, 128, 255));
	m_z2_clip_sldr = new wxSlider(m_zpanel, ID_Z2ClipSldr, 512, 0, 512,
		wxPoint(23, 0), wxDefaultSize, wxSL_LEFT);
	//
	st_cb = new wxStaticText(this, 0, "",
		wxDefaultPosition, wxSize(5, 5));
	st_cb->SetBackgroundColour(wxColor(128, 128, 255));
	m_z2_clip_text = new wxTextCtrl(this, ID_Z2ClipText, "512",
		wxDefaultPosition, wxSize(34, 20), 0, vald_int);
	//link z
	m_link_z_tb = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	bitmap = wxGetBitmapFromMemory(unlink);
#ifdef _DARWIN
	m_link_z_tb->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_link_z_tb->AddCheckTool(ID_LinkZChk, "Lock Z Planes",
		bitmap, wxNullBitmap, "Lock Z Planes");
	m_link_z_tb->Realize();
	//add the items
	sizer_cz->Add(5, 5, 0);
	sizer_cz->Add(st, 0, wxALIGN_CENTER, 0);
	sizer_cz->Add(m_z1_clip_text, 0, wxALIGN_CENTER, 0);
	sizer_cz->Add(5, 5, 0);
	sizer_cz->Add(st_cb, 0, wxEXPAND);
	sizer_cz->Add(m_zpanel, 1, wxALIGN_CENTER, 0);
	st_cb = new wxStaticText(this, 0, "",
		wxDefaultPosition, wxSize(5, 5));
	st_cb->SetBackgroundColour(wxColor(128, 255, 255));
	sizer_cz->Add(st_cb, 0, wxEXPAND);
	sizer_cz->Add(5, 5, 0);
	sizer_cz->Add(m_z2_clip_text, 0, wxALIGN_CENTER, 0);
	sizer_cz->Add(5, 5, 0);
	sizer_cz->Add(m_link_z_tb, 0, wxALIGN_CENTER);

	//mouse click
	m_x1_clip_sldr->Connect(ID_X1ClipSldr, wxEVT_RIGHT_DOWN,
		wxCommandEventHandler(ClipPlanePanel::OnSliderRClick),
		NULL, this);
	m_x2_clip_sldr->Connect(ID_X2ClipSldr, wxEVT_RIGHT_DOWN,
		wxCommandEventHandler(ClipPlanePanel::OnSliderRClick),
		NULL, this);
	m_y1_clip_sldr->Connect(ID_Y1ClipSldr, wxEVT_RIGHT_DOWN,
		wxCommandEventHandler(ClipPlanePanel::OnSliderRClick),
		NULL, this);
	m_y2_clip_sldr->Connect(ID_Y2ClipSldr, wxEVT_RIGHT_DOWN,
		wxCommandEventHandler(ClipPlanePanel::OnSliderRClick),
		NULL, this);
	m_z1_clip_sldr->Connect(ID_Z1ClipSldr, wxEVT_RIGHT_DOWN,
		wxCommandEventHandler(ClipPlanePanel::OnSliderRClick),
		NULL, this);
	m_z2_clip_sldr->Connect(ID_Z2ClipSldr, wxEVT_RIGHT_DOWN,
		wxCommandEventHandler(ClipPlanePanel::OnSliderRClick),
		NULL, this);

	//keys
	m_x1_clip_sldr->Connect(ID_X1ClipSldr, wxEVT_KEY_DOWN,
		wxKeyEventHandler(ClipPlanePanel::OnSliderKeyDown),
		NULL, this);
	m_x2_clip_sldr->Connect(ID_X2ClipSldr, wxEVT_KEY_DOWN,
		wxKeyEventHandler(ClipPlanePanel::OnSliderKeyDown),
		NULL, this);
	m_y1_clip_sldr->Connect(ID_Y1ClipSldr, wxEVT_KEY_DOWN,
		wxKeyEventHandler(ClipPlanePanel::OnSliderKeyDown),
		NULL, this);
	m_y2_clip_sldr->Connect(ID_Y2ClipSldr, wxEVT_KEY_DOWN,
		wxKeyEventHandler(ClipPlanePanel::OnSliderKeyDown),
		NULL, this);
	m_z1_clip_sldr->Connect(ID_Z1ClipSldr, wxEVT_KEY_DOWN,
		wxKeyEventHandler(ClipPlanePanel::OnSliderKeyDown),
		NULL, this);
	m_z2_clip_sldr->Connect(ID_Z2ClipSldr, wxEVT_KEY_DOWN,
		wxKeyEventHandler(ClipPlanePanel::OnSliderKeyDown),
		NULL, this);

	//2
	wxBoxSizer *sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	sizer_2->Add(sizer_cx, 1, wxEXPAND);
	sizer_2->Add(sizer_cy, 1, wxEXPAND);
	sizer_2->Add(sizer_cz, 1, wxEXPAND);

	//clip distance
	wxBoxSizer* sizer_3 = new wxBoxSizer(wxHORIZONTAL);
	m_yz_dist_text = new wxTextCtrl(this, ID_YZDistText, "512",
		wxDefaultPosition, wxSize(40, 22), 0, vald_int);
	m_xz_dist_text = new wxTextCtrl(this, ID_XZDistText, "512",
		wxDefaultPosition, wxSize(40, 22), 0, vald_int);
	m_xy_dist_text = new wxTextCtrl(this, ID_XYDistText, "512",
		wxDefaultPosition, wxSize(40, 22), 0, vald_int);
	sizer_3->Add(m_yz_dist_text, 1, wxEXPAND);
	sizer_3->Add(5, 5);
	sizer_3->Add(m_xz_dist_text, 1, wxEXPAND);
	sizer_3->Add(5, 5);
	sizer_3->Add(m_xy_dist_text, 1, wxEXPAND);

	//reset clipping 6
	wxBoxSizer* sizer_6 = new wxBoxSizer(wxHORIZONTAL);
	m_clip_reset_btn = new wxButton(this, ID_ClipResetBtn, "Reset Clip Ranges",
		wxDefaultPosition, wxSize(125, 33));
	m_clip_reset_btn->SetBitmap(wxGetBitmapFromMemory(reset));
	sizer_6->Add(5, 5, 0);
	sizer_6->Add(m_clip_reset_btn, 0, wxALIGN_CENTER);

	//rotations
	//set sero rotation for clipping planes 7
	wxBoxSizer* sizer_7 = new wxBoxSizer(wxHORIZONTAL);
#ifndef _DARWIN
	m_set_zero_btn = new wxButton(this, ID_SetZeroBtn, "Align to View",
		wxDefaultPosition, wxSize(120, 22));
#else
	m_set_zero_btn = new wxButton(this, ID_SetZeroBtn, "Align to View",
		wxDefaultPosition, wxSize(125, 30));
#endif
	m_set_zero_btn->SetBitmap(wxGetBitmapFromMemory(align));
	sizer_7->Add(5, 5, 0);
	sizer_7->Add(m_set_zero_btn, 0, wxALIGN_CENTER);

	//reset rotations 8
	wxBoxSizer* sizer_8 = new wxBoxSizer(wxHORIZONTAL);
#ifndef _DARWIN
	m_rot_reset_btn = new wxButton(this, ID_RotResetBtn, "Reset to 0",
		wxDefaultPosition, wxSize(120, 22));
#else
	m_rot_reset_btn = new wxButton(this, ID_RotResetBtn, "Reset to 0",
		wxDefaultPosition, wxSize(125, 30));
#endif
	m_rot_reset_btn->SetBitmap(wxGetBitmapFromMemory(reset));
	sizer_8->Add(5, 5, 0);
	sizer_8->Add(m_rot_reset_btn, 0, wxALIGN_CENTER);

	//sliders for rotating clipping planes 
	//x
	wxBoxSizer* sizer_rx = new wxBoxSizer(wxVERTICAL);
	st = new wxStaticText(this, 0, "X");
	m_x_rot_sldr = new wxSlider(this, ID_XRotSldr, 0, -180, 180,
		wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL | wxSL_INVERSE);
	m_x_rot_text = new wxTextCtrl(this, ID_XRotText, "0.0",
		wxDefaultPosition, wxSize(34, 20), 0, vald_fp1);
	m_x_rot_spin = new wxSpinButton(this, ID_XRotSpin,
		wxDefaultPosition, wxSize(30, 20), wxSP_VERTICAL);
	sizer_rx->Add(5, 5, 0);
	sizer_rx->Add(m_x_rot_text, 0, wxALIGN_CENTER, 0);
	sizer_rx->Add(st, 0, wxALIGN_CENTER, 0);
	sizer_rx->Add(5, 5, 0);
	sizer_rx->Add(m_x_rot_spin, 0, wxALIGN_CENTER, 0);
	sizer_rx->Add(m_x_rot_sldr, 1, wxALIGN_CENTER, 0);
	//y
	wxBoxSizer* sizer_ry = new wxBoxSizer(wxVERTICAL);
	st = new wxStaticText(this, 0, "Y");
	m_y_rot_sldr = new wxSlider(this, ID_YRotSldr, 0, -180, 180,
		wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL | wxSL_INVERSE);
	m_y_rot_text = new wxTextCtrl(this, ID_YRotText, "0.0",
		wxDefaultPosition, wxSize(34, 20), 0, vald_fp1);
	m_y_rot_spin = new wxSpinButton(this, ID_YRotSpin,
		wxDefaultPosition, wxSize(30, 20), wxSP_VERTICAL);
	sizer_ry->Add(5, 5, 0);
	sizer_ry->Add(m_y_rot_text, 0, wxALIGN_CENTER, 0);
	sizer_ry->Add(st, 0, wxALIGN_CENTER, 0);
	sizer_ry->Add(5, 5, 0);
	sizer_ry->Add(m_y_rot_spin, 0, wxALIGN_CENTER, 0);
	sizer_ry->Add(m_y_rot_sldr, 1, wxALIGN_CENTER, 0);
	//z
	wxBoxSizer* sizer_rz = new wxBoxSizer(wxVERTICAL);
	st = new wxStaticText(this, 0, "Z");
	m_z_rot_sldr = new wxSlider(this, ID_ZRotSldr, 0, -180, 180,
		wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL | wxSL_INVERSE);
	m_z_rot_text = new wxTextCtrl(this, ID_ZRotText, "0.0",
		wxDefaultPosition, wxSize(34, 20), 0, vald_fp1);
	m_z_rot_spin = new wxSpinButton(this, ID_ZRotSpin,
		wxDefaultPosition, wxSize(30, 20), wxSP_VERTICAL);
	sizer_rz->Add(5, 5, 0);
	sizer_rz->Add(m_z_rot_text, 0, wxALIGN_CENTER, 0);
	sizer_rz->Add(st, 0, wxALIGN_CENTER, 0);
	sizer_rz->Add(5, 5, 0);
	sizer_rz->Add(m_z_rot_spin, 0, wxALIGN_CENTER, 0);
	sizer_rz->Add(m_z_rot_sldr, 1, wxALIGN_CENTER, 0);

	//sizer 9
	wxBoxSizer *sizer_9 = new wxBoxSizer(wxHORIZONTAL);
	sizer_9->Add(sizer_rx, 1, wxEXPAND);
	sizer_9->Add(sizer_ry, 1, wxEXPAND);
	sizer_9->Add(sizer_rz, 1, wxEXPAND);

	//v
	wxBoxSizer *sizer_v = new wxBoxSizer(wxVERTICAL);
	sizer_v->Add(sizer_1, 0, wxALIGN_CENTER);
	sizer_v->Add(5, 5, 0);
	sizer_v->Add(sizer_2, 1, wxEXPAND);
	sizer_v->AddSpacer(5);
	sizer_v->Add(new wxStaticText(this, 0,
		"Clipping Ranges"), 0, wxALIGN_CENTER);
	sizer_v->Add(5, 5, 0);
	sizer_v->Add(sizer_3, 0, wxEXPAND);
	sizer_v->Add(5, 10, 0);
	sizer_v->Add(sizer_6, 0, wxALIGN_CENTER);
	sizer_v->Add(5, 5, 0);

	st = new wxStaticText(this, 0, "", wxDefaultPosition, wxSize(5, 5));
	st->SetBackgroundColour(wxColor(100, 100, 100));
	sizer_v->Add(10, 10, 0);
	sizer_v->Add(st, 0, wxEXPAND);

	st = new wxStaticText(this, 0, "Rotations:");
	sizer_v->Add(10, 10, 0);
	sizer_v->Add(st, 0, wxALIGN_CENTER);
	sizer_v->Add(10, 10, 0);
	sizer_v->Add(sizer_7, 0, wxALIGN_CENTER);
	sizer_v->Add(sizer_8, 0, wxALIGN_CENTER);
	sizer_v->Add(10, 10, 0);
	sizer_v->Add(sizer_9, 1, wxEXPAND);

	SetSizer(sizer_v);
	Layout();

	DisableAll();
}

ClipPlanePanel::~ClipPlanePanel()
{
}

void ClipPlanePanel::AssociateNode(FL::Node* node)
{
	m_agent->setObject(node);
}

void ClipPlanePanel::SetChannLink(bool chann)
{
	m_toolbar->ToggleTool(ID_LinkChannelsBtn, chann);
}

void ClipPlanePanel::SetHoldPlanes(bool hold)
{
	m_hold_planes = hold;
	m_toolbar->ToggleTool(ID_HoldPlanesBtn, hold);
	if (hold)
	{
		//VRenderFrame* vrender_frame = (VRenderFrame*)m_frame;
		//if (!vrender_frame)
		//	return;
		//vector <VRenderView*>* vrv_list = vrender_frame->GetViewList();
		//for (int i = 0; i<(int)vrv_list->size(); i++)
		//{
		//	if ((*vrv_list)[i])
		//	{
		//		(*vrv_list)[i]->m_glview->m_draw_clip = true;
		//		(*vrv_list)[i]->m_glview->m_clip_mask = -1;
		//	}
		//}
	}
}

void ClipPlanePanel::SetPlaneMode(PLANE_MODES mode)
{
	m_plane_mode = mode;
	switch (m_plane_mode)
	{
	case kNormal:
		m_toolbar->SetToolNormalBitmap(ID_PlaneModesBtn,
			wxGetBitmapFromMemory(clip_normal));
		break;
	case kFrame:
		m_toolbar->SetToolNormalBitmap(ID_PlaneModesBtn,
			wxGetBitmapFromMemory(clip_frame));
		break;
	case kLowTrans:
		m_toolbar->SetToolNormalBitmap(ID_PlaneModesBtn,
			wxGetBitmapFromMemory(clip_low));
		break;
	case kLowTransBack:
		m_toolbar->SetToolNormalBitmap(ID_PlaneModesBtn,
			wxGetBitmapFromMemory(clip_low_back));
		break;
	case kNormalBack:
		m_toolbar->SetToolNormalBitmap(ID_PlaneModesBtn,
			wxGetBitmapFromMemory(clip_normal_back));
		break;
	case kNone:
		m_toolbar->SetToolNormalBitmap(ID_PlaneModesBtn,
			wxGetBitmapFromMemory(clip_none));
		break;
	}
}

void ClipPlanePanel::OnLinkChannelsBtn(wxCommandEvent &event)
{
	if (m_toolbar->GetToolState(ID_LinkChannelsBtn))
	{
		//if (!m_mgr)
		//	return;

		//wxString str;
		////x1
		//str = m_x1_clip_text->GetValue();
		//long x1_val = 0;
		//str.ToLong(&x1_val);
		////x2
		//str = m_x2_clip_text->GetValue();
		//long x2_val = 0;
		//str.ToLong(&x2_val);
		////y1
		//str = m_y1_clip_text->GetValue();
		//long y1_val = 0;
		//str.ToLong(&y1_val);
		////y2
		//str = m_y2_clip_text->GetValue();
		//long y2_val = 0;
		//str.ToLong(&y2_val);
		////z1
		//str = m_z1_clip_text->GetValue();
		//long z1_val = 0;
		//str.ToLong(&z1_val);
		////z2
		//str = m_z2_clip_text->GetValue();
		//long z2_val = 0;
		//str.ToLong(&z2_val);

		//size_t num = FL::Global::instance().getVolumeFactory().getNum();
		//for (size_t i = 0; i<num; i++)
		//{
		//	FL::VolumeData* vd =
		//		FL::Global::instance().getVolumeFactory().get(i);
		//	if (!vd || vd == m_vd)
		//		continue;

		//	vector<Plane*> *planes = 0;
		//	if (vd->GetRenderer())
		//		planes = vd->GetRenderer()->get_planes();
		//	if (!planes)
		//		continue;
		//	if (planes->size() != 6)
		//		continue;

		//	long resx, resy, resz;
		//	//vd->GetResolution(resx, resy, resz);
		//	vd->getValue("res x", resx);
		//	vd->getValue("res y", resy);
		//	vd->getValue("res z", resz);

		//	(*planes)[0]->ChangePlane(Point(double(x1_val) / double(resx), 0.0, 0.0), Vector(1.0, 0.0, 0.0));
		//	(*planes)[1]->ChangePlane(Point(double(x2_val) / double(resx), 0.0, 0.0), Vector(-1.0, 0.0, 0.0));
		//	(*planes)[2]->ChangePlane(Point(0.0, double(y1_val) / double(resy), 0.0), Vector(0.0, 1.0, 0.0));
		//	(*planes)[3]->ChangePlane(Point(0.0, double(y2_val) / double(resy), 0.0), Vector(0.0, -1.0, 0.0));
		//	(*planes)[4]->ChangePlane(Point(0.0, 0.0, double(z1_val) / double(resz)), Vector(0.0, 0.0, 1.0));
		//	(*planes)[5]->ChangePlane(Point(0.0, 0.0, double(z2_val) / double(resz)), Vector(0.0, 0.0, -1.0));
		//}

		//RefreshVRenderViews();
	}
}

void ClipPlanePanel::OnHoldPlanesBtn(wxCommandEvent &event)
{
	m_hold_planes = m_toolbar->GetToolState(ID_HoldPlanesBtn);
}

void ClipPlanePanel::OnPlaneModesBtn(wxCommandEvent &event)
{
	switch (m_plane_mode)
	{
	case kNormal:
		m_plane_mode = kFrame;
		m_toolbar->SetToolNormalBitmap(ID_PlaneModesBtn,
			wxGetBitmapFromMemory(clip_frame));
		break;
	case kFrame:
		m_plane_mode = kLowTrans;
		m_toolbar->SetToolNormalBitmap(ID_PlaneModesBtn,
			wxGetBitmapFromMemory(clip_low));
		break;
	case kLowTrans:
		m_plane_mode = kLowTransBack;
		m_toolbar->SetToolNormalBitmap(ID_PlaneModesBtn,
			wxGetBitmapFromMemory(clip_low_back));
		break;
	case kLowTransBack:
		m_plane_mode = kNormalBack;
		m_toolbar->SetToolNormalBitmap(ID_PlaneModesBtn,
			wxGetBitmapFromMemory(clip_normal_back));
		break;
	case kNormalBack:
		m_plane_mode = kNone;
		m_toolbar->SetToolNormalBitmap(ID_PlaneModesBtn,
			wxGetBitmapFromMemory(clip_none));
		break;
	case kNone:
		m_plane_mode = kNormal;
		m_toolbar->SetToolNormalBitmap(ID_PlaneModesBtn,
			wxGetBitmapFromMemory(clip_normal));
		break;
	}

	//RefreshVRenderViews();
}

void ClipPlanePanel::OnClipResetBtn(wxCommandEvent &event)
{
	m_agent->setValue("clip link x", false);
	m_agent->setValue("clip link y", false);
	m_agent->setValue("clip link z", false);

	m_agent->setValue("clip x1", double(0));
	m_agent->setValue("clip x2", double(1));
	m_agent->setValue("clip y1", double(0));
	m_agent->setValue("clip y2", double(1));
	m_agent->setValue("clip z1", double(0));
	m_agent->setValue("clip z2", double(1));
}

void ClipPlanePanel::OnX1ClipChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	m_x1_clip_text->SetValue(wxString::Format("%d", ival));
}

void ClipPlanePanel::OnX1ClipEdit(wxCommandEvent &event)
{
	long resx;
	m_agent->getValue("res x", resx);
	bool link;
	m_agent->getValue("clip link x", link);

	wxString str = m_x1_clip_text->GetValue();
	long ival = 0;
	double dval = 0.0;
	str.ToLong(&ival);
	if (!link)
	{
		str = m_x2_clip_text->GetValue();
		long ival2;
		str.ToLong(&ival2);
		if (ival >= ival2)
			ival = ival2 - 1;
	}
	m_x1_clip_sldr->SetValue(ival);
	dval = (double)ival / (double)resx;

	m_agent->setValue("clip x1", dval);
}

void ClipPlanePanel::OnX2ClipChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	m_x2_clip_text->SetValue(wxString::Format("%d", ival));
}

void ClipPlanePanel::OnX2ClipEdit(wxCommandEvent &event)
{
	long resx;
	m_agent->getValue("res x", resx);
	bool link;
	m_agent->getValue("clip link x", link);

	wxString str = m_x2_clip_text->GetValue();
	long ival = 0;
	double dval = 0.0;
	str.ToLong(&ival);
	if (!link)
	{
		str = m_x1_clip_text->GetValue();
		long ival2;
		str.ToLong(&ival2);
		if (ival <= ival2)
			ival = ival2 + 1;
	}
	m_x2_clip_sldr->SetValue(ival);
	dval = (double)ival / (double)resx;

	m_agent->setValue("clip x2", dval);
}

void ClipPlanePanel::OnY1ClipChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	m_y1_clip_text->SetValue(wxString::Format("%d", ival));
}

void ClipPlanePanel::OnY1ClipEdit(wxCommandEvent &event)
{
	long resy;
	m_agent->getValue("res y", resy);
	bool link;
	m_agent->getValue("clip link y", link);

	wxString str = m_y1_clip_text->GetValue();
	long ival = 0;
	double dval = 0.0;
	str.ToLong(&ival);
	if (!link)
	{
		str = m_y2_clip_text->GetValue();
		long ival2;
		str.ToLong(&ival2);
		if (ival >= ival2)
			ival = ival2 - 1;
	}
	m_y1_clip_sldr->SetValue(ival);
	dval = (double)ival / (double)resy;

	m_agent->setValue("clip y1", dval);
}

void ClipPlanePanel::OnY2ClipChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	m_y2_clip_text->SetValue(wxString::Format("%d", ival));
}

void ClipPlanePanel::OnY2ClipEdit(wxCommandEvent &event)
{
	long resy;
	m_agent->getValue("res y", resy);
	bool link;
	m_agent->getValue("clip link y", link);

	wxString str = m_y2_clip_text->GetValue();
	long ival = 0;
	double dval = 0.0;
	str.ToLong(&ival);
	if (!link)
	{
		str = m_y1_clip_text->GetValue();
		long ival2;
		str.ToLong(&ival2);
		if (ival <= ival2)
			ival = ival2 + 1;
	}
	m_y2_clip_sldr->SetValue(ival);
	dval = (double)ival / (double)resy;

	m_agent->setValue("clip y2", dval);
}

void ClipPlanePanel::OnZ1ClipChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	m_z1_clip_text->SetValue(wxString::Format("%d", ival));
}

void ClipPlanePanel::OnZ1ClipEdit(wxCommandEvent &event)
{
	long resz;
	m_agent->getValue("res z", resz);
	bool link;
	m_agent->getValue("clip link z", link);

	wxString str = m_z1_clip_text->GetValue();
	long ival = 0;
	double dval = 0.0;
	str.ToLong(&ival);
	if (!link)
	{
		str = m_z2_clip_text->GetValue();
		long ival2;
		str.ToLong(&ival2);
		if (ival >= ival2)
			ival = ival2 - 1;
	}
	m_z1_clip_sldr->SetValue(ival);
	dval = (double)ival / (double)resz;

	m_agent->setValue("clip z1", dval);
}

void ClipPlanePanel::OnZ2ClipChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	m_z2_clip_text->SetValue(wxString::Format("%d", ival));
}

void ClipPlanePanel::OnZ2ClipEdit(wxCommandEvent &event)
{
	long resz;
	m_agent->getValue("res z", resz);
	bool link;
	m_agent->getValue("clip link z", link);

	wxString str = m_z2_clip_text->GetValue();
	long ival = 0;
	double dval = 0.0;
	str.ToLong(&ival);
	if (!link)
	{
		str = m_z1_clip_text->GetValue();
		long ival2;
		str.ToLong(&ival2);
		if (ival <= ival2)
			ival = ival2 + 1;
	}
	m_z2_clip_sldr->SetValue(ival);
	dval = (double)ival / (double)resz;

	m_agent->setValue("clip z2", dval);
}

void ClipPlanePanel::OnResize(wxSizeEvent &event)
{
	int sz = m_xpanel->GetSize().GetHeight();
	m_x1_clip_sldr->SetSize(20, sz);
	m_x2_clip_sldr->SetSize(20, sz);
	m_y1_clip_sldr->SetSize(20, sz);
	m_y2_clip_sldr->SetSize(20, sz);
	m_z1_clip_sldr->SetSize(20, sz);
	m_z2_clip_sldr->SetSize(20, sz);

	Layout();
}

void ClipPlanePanel::OnIdle(wxIdleEvent &event)
{
	if (!IsShown())
		return;

	//if (m_hold_planes)
	//{
	//	m_draw_clip = true;
	//	return;
	//}

	//int i;
	//VRenderFrame* vrender_frame = (VRenderFrame*)m_frame;
	//if (!vrender_frame)
	//	return;

	//for (i = 0; i<vrender_frame->GetViewNum(); i++)
	//{
	//	VRenderView *vrv = vrender_frame->GetView(i);
	//	if (vrv)
	//	{
	//		if (vrv->m_glview->m_capture)
	//			return;
	//	}
	//}

	//wxPoint pos = wxGetMousePosition();
	//wxRect reg = GetScreenRect();
	//wxWindow *window = wxWindow::FindFocus();
	//if (window && reg.Contains(pos))
	//{
	//	if (!m_draw_clip)
	//	{
	//		vector <VRenderView*>* vrv_list = vrender_frame->GetViewList();
	//		for (i = 0; i<(int)vrv_list->size(); i++)
	//		{
	//			if ((*vrv_list)[i])
	//			{
	//				(*vrv_list)[i]->m_glview->m_draw_clip = true;
	//				(*vrv_list)[i]->m_glview->m_clip_mask = -1;
	//			}
	//		}
	//		RefreshVRenderViews();
	//		m_draw_clip = true;
	//	}
	//}
	//else
	//{
	//	if (m_draw_clip)
	//	{
	//		vector <VRenderView*>* vrv_list = vrender_frame->GetViewList();
	//		for (i = 0; i<(int)vrv_list->size(); i++)
	//		{
	//			if ((*vrv_list)[i])
	//				(*vrv_list)[i]->m_glview->m_draw_clip = false;
	//		}
	//		RefreshVRenderViews();
	//		m_draw_clip = false;
	//	}
	//}

}

void ClipPlanePanel::OnLinkXCheck(wxCommandEvent &event)
{
	bool bval = m_link_x_tb->GetToolState(ID_LinkXChk);
	m_agent->setValue("clip link x", bval);
	if (bval)
		m_link_x_tb->SetToolNormalBitmap(ID_LinkXChk,
			wxGetBitmapFromMemory(link));
	else
		m_link_x_tb->SetToolNormalBitmap(ID_LinkXChk,
			wxGetBitmapFromMemory(unlink));
}

void ClipPlanePanel::OnLinkYCheck(wxCommandEvent &event)
{
	bool bval = m_link_y_tb->GetToolState(ID_LinkYChk);
	m_agent->setValue("clip link y", bval);
	if (bval)
		m_link_y_tb->SetToolNormalBitmap(ID_LinkYChk,
			wxGetBitmapFromMemory(link));
	else
		m_link_y_tb->SetToolNormalBitmap(ID_LinkYChk,
			wxGetBitmapFromMemory(unlink));
}

void ClipPlanePanel::OnLinkZCheck(wxCommandEvent &event)
{
	bool bval = m_link_z_tb->GetToolState(ID_LinkZChk);
	m_agent->setValue("clip link z", bval);
	if (bval)
		m_link_z_tb->SetToolNormalBitmap(ID_LinkZChk,
			wxGetBitmapFromMemory(link));
	else
		m_link_z_tb->SetToolNormalBitmap(ID_LinkZChk,
			wxGetBitmapFromMemory(unlink));
}

void ClipPlanePanel::OnClipDistXEdit(wxCommandEvent &event)
{
	wxString str = m_yz_dist_text->GetValue();
	long lval;
	str.ToLong(&lval);
	long res;
	m_agent->getValue("res x", res);
	double clip_dist = double(lval) / double(res);
	m_agent->setValue("clip dist x", clip_dist);
}

void ClipPlanePanel::OnClipDistYEdit(wxCommandEvent &event)
{
	wxString str = m_xz_dist_text->GetValue();
	long lval;
	str.ToLong(&lval);
	long res;
	m_agent->getValue("res y", res);
	double clip_dist = double(lval) / double(res);
	m_agent->setValue("clip dist y", clip_dist);
}

void ClipPlanePanel::OnClipDistZEdit(wxCommandEvent &event)
{
	wxString str = m_xy_dist_text->GetValue();
	long lval;
	str.ToLong(&lval);
	long res;
	m_agent->getValue("res z", res);
	double clip_dist = double(lval) / double(res);
	m_agent->setValue("clip dist z", clip_dist);
}

void ClipPlanePanel::OnSetZeroBtn(wxCommandEvent &event)
{
	//get its view
	m_agent->alignRenderViewRot();
}

void ClipPlanePanel::OnRotResetBtn(wxCommandEvent &event)
{
	m_agent->setValue("clip rot x", double(0));
	m_agent->setValue("clip rot y", double(0));
	m_agent->setValue("clip rot z", double(0));
}

void ClipPlanePanel::OnXRotChange(wxScrollEvent &event)
{
	int val = event.GetPosition();
	wxString str = wxString::Format("%.1f", double(val));
	m_x_rot_text->SetValue(str);
}

void ClipPlanePanel::OnXRotEdit(wxCommandEvent &event)
{
	wxString str = m_x_rot_text->GetValue();
	double dval = 0.0;
	str.ToDouble(&dval);
	m_x_rot_sldr->SetValue(int(dval + 0.5));
	m_agent->setValue("clip rot x", dval);
}

void ClipPlanePanel::OnYRotChange(wxScrollEvent &event)
{
	int val = event.GetPosition();
	wxString str = wxString::Format("%.1f", double(val));
	m_y_rot_text->SetValue(str);
}

void ClipPlanePanel::OnYRotEdit(wxCommandEvent &event)
{
	wxString str = m_y_rot_text->GetValue();
	double dval = 0.0;
	str.ToDouble(&dval);
	m_y_rot_sldr->SetValue(int(dval + 0.5));
	m_agent->setValue("clip rot y", dval);
}

void ClipPlanePanel::OnZRotChange(wxScrollEvent &event)
{
	int val = event.GetPosition();
	wxString str = wxString::Format("%.1f", double(val));
	m_z_rot_text->SetValue(str);
}

void ClipPlanePanel::OnZRotEdit(wxCommandEvent &event)
{
	wxString str = m_z_rot_text->GetValue();
	double dval = 0.0;
	str.ToDouble(&dval);
	m_z_rot_sldr->SetValue(int(dval + 0.5));
	m_agent->setValue("clip rot z", dval);
}

void ClipPlanePanel::OnXRotSpinUp(wxSpinEvent& event)
{
	wxString str_val = m_x_rot_text->GetValue();
	double val;
	str_val.ToDouble(&val);
	val += 1.0;
	if (val > 180.0) val -= 360.0;
	if (val <-180.0) val += 360.0;
	wxString str = wxString::Format("%.1f", val);
	m_x_rot_text->SetValue(str);
}

void ClipPlanePanel::OnXRotSpinDown(wxSpinEvent& event)
{
	wxString str_val = m_x_rot_text->GetValue();
	double val;
	str_val.ToDouble(&val);
	val -= 1.0;
	if (val > 180.0) val -= 360.0;
	if (val <-180.0) val += 360.0;
	wxString str = wxString::Format("%.1f", val);
	m_x_rot_text->SetValue(str);
}

void ClipPlanePanel::OnYRotSpinUp(wxSpinEvent& event)
{
	wxString str_val = m_y_rot_text->GetValue();
	double val;
	str_val.ToDouble(&val);
	val += 1.0;
	if (val > 180.0) val -= 360.0;
	if (val <-180.0) val += 360.0;
	wxString str = wxString::Format("%.1f", val);
	m_y_rot_text->SetValue(str);
}

void ClipPlanePanel::OnYRotSpinDown(wxSpinEvent& event)
{
	wxString str_val = m_y_rot_text->GetValue();
	double val;
	str_val.ToDouble(&val);
	val -= 1.0;
	if (val > 180.0) val -= 360.0;
	if (val <-180.0) val += 360.0;
	wxString str = wxString::Format("%.1f", val);
	m_y_rot_text->SetValue(str);
}

void ClipPlanePanel::OnZRotSpinUp(wxSpinEvent& event)
{
	m_z_rot_spin->SetValue(10);
	wxString str_val = m_z_rot_text->GetValue();
	double val;
	str_val.ToDouble(&val);
	val += 1.0;
	if (val > 180.0) val -= 360.0;
	if (val <-180.0) val += 360.0;
	wxString str = wxString::Format("%.1f", val);
	m_z_rot_text->SetValue(str);
}

void ClipPlanePanel::OnZRotSpinDown(wxSpinEvent& event)
{
	m_z_rot_spin->SetValue(10);
	wxString str_val = m_z_rot_text->GetValue();
	double val;
	str_val.ToDouble(&val);
	val -= 1.0;
	if (val > 180.0) val -= 360.0;
	if (val <-180.0) val += 360.0;
	wxString str = wxString::Format("%.1f", val);
	m_z_rot_text->SetValue(str);
}

void ClipPlanePanel::OnSliderRClick(wxCommandEvent& event)
{
	//if (m_sel_type != 2 || !m_vd)
	//	return;

	//int id = event.GetId();

	//wxString str;
	//double val;

	////good rate
	//double sample_rate;
	//m_vd->getValue("sample rate", sample_rate);
	//if (sample_rate < 2.0)
	//{
	//	sample_rate = 2.0;
	//	m_vd->setValue("sample rate", sample_rate);
	//}
	//if (m_toolbar->GetToolState(ID_LinkChannelsBtn))
	//{
	//	if (m_mgr)
	//	{
	//		//int i;
	//		//for (i=0; i<m_mgr->GetVolumeNum(); i++)
	//		//{
	//		//	VolumeData* vd = m_mgr->GetVolumeData(i);
	//		//	if (!vd || vd == m_vd)
	//		//		continue;
	//		//	if (vd->GetSampleRate()<2.0)
	//		//		vd->SetSampleRate(2.0);
	//		//}
	//	}
	//}

	//long resx, resy, resz;
	////m_vd->GetResolution(resx, resy, resz);
	//m_vd->getValue("res x", resx);
	//m_vd->getValue("res y", resy);
	//m_vd->getValue("res z", resz);

	//if (id == ID_X1ClipSldr)
	//{
	//	str = m_x1_clip_text->GetValue();
	//	str.ToDouble(&val);
	//	if (val < resx)
	//	{
	//		SetXLink(false);
	//		m_x2_clip_text->SetValue(
	//			wxString::Format("%d", (int)(val + 1.0)));
	//		SetXLink(true);
	//	}
	//	m_x1_clip_sldr->SetFocus();
	//}
	//else if (id == ID_X2ClipSldr)
	//{
	//	str = m_x2_clip_text->GetValue();
	//	str.ToDouble(&val);
	//	if (val > 0)
	//	{
	//		SetXLink(false);
	//		m_x1_clip_text->SetValue(
	//			wxString::Format("%d", (int)(val - 1.0)));
	//		SetXLink(true);
	//	}
	//	m_x2_clip_sldr->SetFocus();
	//}
	//else if (id == ID_Y1ClipSldr)
	//{
	//	str = m_y1_clip_text->GetValue();
	//	str.ToDouble(&val);
	//	if (val < resy)
	//	{
	//		SetYLink(false);
	//		m_y2_clip_text->SetValue(
	//			wxString::Format("%d", (int)(val + 1.0)));
	//		SetYLink(true);
	//	}
	//	m_y1_clip_sldr->SetFocus();
	//}
	//else if (id == ID_Y2ClipSldr)
	//{
	//	str = m_y2_clip_text->GetValue();
	//	str.ToDouble(&val);
	//	if (val > 0)
	//	{
	//		SetYLink(false);
	//		m_y1_clip_text->SetValue(
	//			wxString::Format("%d", (int)(val - 1.0)));
	//		SetYLink(true);
	//	}
	//	m_y2_clip_sldr->SetFocus();
	//}
	//else if (id == ID_Z1ClipSldr)
	//{
	//	str = m_z1_clip_text->GetValue();
	//	str.ToDouble(&val);
	//	if (val < resz)
	//	{
	//		SetZLink(false);
	//		m_z2_clip_text->SetValue(
	//			wxString::Format("%d", (int)(val + 1.0)));
	//		SetZLink(true);
	//	}
	//	m_z1_clip_sldr->SetFocus();
	//}
	//else if (id == ID_Z2ClipSldr)
	//{
	//	str = m_z2_clip_text->GetValue();
	//	str.ToDouble(&val);
	//	if (val > 0)
	//	{
	//		SetZLink(false);
	//		m_z1_clip_text->SetValue(
	//			wxString::Format("%d", (int)(val - 1.0)));
	//		SetZLink(true);
	//	}
	//	m_z2_clip_sldr->SetFocus();
	//}

	////reset others
	//vector<Plane*> *planes = 0;
	//if (m_vd->GetRenderer())
	//	planes = m_vd->GetRenderer()->get_planes();
	//if (!planes)
	//	return;
	//if (planes->size() != 6)
	//	return;

	//if (id == ID_X1ClipSldr ||
	//	id == ID_X2ClipSldr)
	//{
	//	m_link_y = false;
	//	m_check_tb->ToggleTool(ID_LinkYChk, false);
	//	m_check_tb->SetToolNormalBitmap(ID_LinkYChk,
	//		wxGetBitmapFromMemory(unlink));
	//	m_link_z = false;
	//	m_check_tb->ToggleTool(ID_LinkZChk, false);
	//	m_check_tb->SetToolNormalBitmap(ID_LinkZChk,
	//		wxGetBitmapFromMemory(unlink));
	//	m_y1_clip_text->SetValue("0");
	//	m_y2_clip_text->SetValue(
	//		wxString::Format("%d", resy));
	//	m_z1_clip_text->SetValue("0");
	//	m_z2_clip_text->SetValue(
	//		wxString::Format("%d", resz));
	//}
	//else if (id == ID_Y1ClipSldr ||
	//	id == ID_Y2ClipSldr)
	//{
	//	m_link_x = false;
	//	m_check_tb->ToggleTool(ID_LinkXChk, false);
	//	m_check_tb->SetToolNormalBitmap(ID_LinkXChk,
	//		wxGetBitmapFromMemory(unlink));
	//	m_link_z = false;
	//	m_check_tb->ToggleTool(ID_LinkZChk, false);
	//	m_check_tb->SetToolNormalBitmap(ID_LinkZChk,
	//		wxGetBitmapFromMemory(unlink));
	//	m_x1_clip_text->SetValue("0");
	//	m_x2_clip_text->SetValue(
	//		wxString::Format("%d", resx));
	//	m_z1_clip_text->SetValue("0");
	//	m_z2_clip_text->SetValue(
	//		wxString::Format("%d", resz));
	//}
	//else if (id == ID_Z1ClipSldr ||
	//	id == ID_Z2ClipSldr)
	//{
	//	m_link_x = false;
	//	m_check_tb->ToggleTool(ID_LinkXChk, false);
	//	m_check_tb->SetToolNormalBitmap(ID_LinkXChk,
	//		wxGetBitmapFromMemory(unlink));
	//	m_link_y = false;
	//	m_check_tb->ToggleTool(ID_LinkYChk, false);
	//	m_check_tb->SetToolNormalBitmap(ID_LinkYChk,
	//		wxGetBitmapFromMemory(unlink));
	//	m_x1_clip_text->SetValue("0");
	//	m_x2_clip_text->SetValue(
	//		wxString::Format("%d", resx));
	//	m_y1_clip_text->SetValue("0");
	//	m_y2_clip_text->SetValue(
	//		wxString::Format("%d", resy));
	//}
}

//move linked clipping planes
//dir: 0-lower; 1-higher
void ClipPlanePanel::MoveLinkedClippingPlanes(int dir)
{
	//if (m_sel_type != 2 || !m_vd)
	//	return;

	//long resx, resy, resz;
	////m_vd->GetResolution(resx, resy, resz);
	//m_vd->getValue("res x", resx);
	//m_vd->getValue("res y", resy);
	//m_vd->getValue("res z", resz);

	//wxString str;
	//long dist;

	//if (dir == 0)
	//{
	//	//moving lower
	//	if (m_link_x)
	//	{
	//		str = m_yz_dist_text->GetValue();
	//		str.ToLong(&dist);
	//		dist = dist<2 ? dist + 1 : dist;
	//		str = m_x1_clip_text->GetValue();
	//		long x1;
	//		str.ToLong(&x1);
	//		x1 = x1 - dist + 1;
	//		x1 = x1<0 ? 0 : x1;
	//		m_x1_clip_text->SetValue(
	//			wxString::Format("%d", x1));
	//	}
	//	if (m_link_y)
	//	{
	//		str = m_xz_dist_text->GetValue();
	//		str.ToLong(&dist);
	//		dist = dist<2 ? dist + 1 : dist;
	//		str = m_y1_clip_text->GetValue();
	//		long y1;
	//		str.ToLong(&y1);
	//		y1 = y1 - dist + 1;
	//		y1 = y1<0 ? 0 : y1;
	//		m_y1_clip_text->SetValue(
	//			wxString::Format("%d", y1));
	//	}
	//	if (m_link_z)
	//	{
	//		str = m_xy_dist_text->GetValue();
	//		str.ToLong(&dist);
	//		dist = dist<2 ? dist + 1 : dist;
	//		str = m_z1_clip_text->GetValue();
	//		long z1;
	//		str.ToLong(&z1);
	//		z1 = z1 - dist + 1;
	//		z1 = z1<0 ? 0 : z1;
	//		m_z1_clip_text->SetValue(
	//			wxString::Format("%d", z1));
	//	}
	//}
	//else
	//{
	//	//moving higher
	//	if (m_link_x)
	//	{
	//		str = m_yz_dist_text->GetValue();
	//		str.ToLong(&dist);
	//		dist = dist<2 ? dist + 1 : dist;
	//		str = m_x2_clip_text->GetValue();
	//		long x2;
	//		str.ToLong(&x2);
	//		x2 = x2 + dist - 1;
	//		x2 = x2>resx ? resx : x2;
	//		m_x2_clip_text->SetValue(
	//			wxString::Format("%d", x2));
	//	}
	//	if (m_link_y)
	//	{
	//		str = m_xz_dist_text->GetValue();
	//		str.ToLong(&dist);
	//		dist = dist<2 ? dist + 1 : dist;
	//		str = m_y2_clip_text->GetValue();
	//		long y2;
	//		str.ToLong(&y2);
	//		y2 = y2 + dist - 1;
	//		y2 = y2>resy ? resy : y2;
	//		m_y2_clip_text->SetValue(
	//			wxString::Format("%d", y2));
	//	}
	//	if (m_link_z)
	//	{
	//		str = m_xy_dist_text->GetValue();
	//		str.ToLong(&dist);
	//		dist = dist<2 ? dist + 1 : dist;
	//		str = m_z2_clip_text->GetValue();
	//		long z2;
	//		str.ToLong(&z2);
	//		z2 = z2 + dist - 1;
	//		z2 = z2>resz ? resz : z2;
	//		m_z2_clip_text->SetValue(
	//			wxString::Format("%d", z2));
	//	}
	//}
}

void ClipPlanePanel::OnSliderKeyDown(wxKeyEvent& event)
{
	//if (m_sel_type != 2 || !m_vd)
	//	return;

	//long resx, resy, resz;
	////m_vd->GetResolution(resx, resy, resz);
	//m_vd->getValue("res x", resx);
	//m_vd->getValue("res y", resy);
	//m_vd->getValue("res z", resz);

	//int id = event.GetId();
	//int key = event.GetKeyCode();

	//wxString str;
	//long dist;

	//if (key == wxKeyCode(','))
	//{
	//	if (id == ID_X1ClipSldr ||
	//		id == ID_X2ClipSldr)
	//	{
	//		if (!m_link_x)
	//		{
	//			event.Skip();
	//			return;
	//		}

	//		str = m_yz_dist_text->GetValue();
	//		str.ToLong(&dist);
	//		dist = dist<2 ? dist + 1 : dist;
	//		str = m_x1_clip_text->GetValue();
	//		long x1;
	//		str.ToLong(&x1);
	//		x1 = x1 - dist + 1;
	//		x1 = x1<0 ? 0 : x1;
	//		m_x1_clip_text->SetValue(
	//			wxString::Format("%d", x1));
	//	}
	//	else if (id == ID_Y1ClipSldr ||
	//		id == ID_Y2ClipSldr)
	//	{
	//		if (!m_link_y)
	//		{
	//			event.Skip();
	//			return;
	//		}

	//		str = m_xz_dist_text->GetValue();
	//		str.ToLong(&dist);
	//		dist = dist<2 ? dist + 1 : dist;
	//		str = m_y1_clip_text->GetValue();
	//		long y1;
	//		str.ToLong(&y1);
	//		y1 = y1 - dist + 1;
	//		y1 = y1<0 ? 0 : y1;
	//		m_y1_clip_text->SetValue(
	//			wxString::Format("%d", y1));
	//	}
	//	else if (id == ID_Z1ClipSldr ||
	//		id == ID_Z2ClipSldr)
	//	{
	//		if (!m_link_z)
	//		{
	//			event.Skip();
	//			return;
	//		}

	//		str = m_xy_dist_text->GetValue();
	//		str.ToLong(&dist);
	//		dist = dist<2 ? dist + 1 : dist;
	//		str = m_z1_clip_text->GetValue();
	//		long z1;
	//		str.ToLong(&z1);
	//		z1 = z1 - dist + 1;
	//		z1 = z1<0 ? 0 : z1;
	//		m_z1_clip_text->SetValue(
	//			wxString::Format("%d", z1));
	//	}
	//}
	//else if (key == wxKeyCode('.'))
	//{
	//	if (id == ID_X1ClipSldr ||
	//		id == ID_X2ClipSldr)
	//	{
	//		if (!m_link_x)
	//		{
	//			event.Skip();
	//			return;
	//		}

	//		str = m_yz_dist_text->GetValue();
	//		str.ToLong(&dist);
	//		dist = dist<2 ? dist + 1 : dist;
	//		str = m_x2_clip_text->GetValue();
	//		long x2;
	//		str.ToLong(&x2);
	//		x2 = x2 + dist - 1;
	//		x2 = x2>resx ? resx : x2;
	//		m_x2_clip_text->SetValue(
	//			wxString::Format("%d", x2));
	//	}
	//	else if (id == ID_Y1ClipSldr ||
	//		id == ID_Y2ClipSldr)
	//	{
	//		if (!m_link_y)
	//		{
	//			event.Skip();
	//			return;
	//		}

	//		str = m_xz_dist_text->GetValue();
	//		str.ToLong(&dist);
	//		dist = dist<2 ? dist + 1 : dist;
	//		str = m_y2_clip_text->GetValue();
	//		long y2;
	//		str.ToLong(&y2);
	//		y2 = y2 + dist - 1;
	//		y2 = y2>resy ? resy : y2;
	//		m_y2_clip_text->SetValue(
	//			wxString::Format("%d", y2));
	//	}
	//	else if (id == ID_Z1ClipSldr ||
	//		id == ID_Z2ClipSldr)
	//	{
	//		if (!m_link_z)
	//		{
	//			event.Skip();
	//			return;
	//		}

	//		str = m_xy_dist_text->GetValue();
	//		str.ToLong(&dist);
	//		dist = dist<2 ? dist + 1 : dist;
	//		str = m_z2_clip_text->GetValue();
	//		long z2;
	//		str.ToLong(&z2);
	//		z2 = z2 + dist - 1;
	//		z2 = z2>resz ? resz : z2;
	//		m_z2_clip_text->SetValue(
	//			wxString::Format("%d", z2));
	//	}
	//}

	event.Skip();
}

void ClipPlanePanel::EnableAll()
{
	m_toolbar->Enable();
	m_set_zero_btn->Enable();
	m_rot_reset_btn->Enable();
	m_x_rot_sldr->Enable();
	m_y_rot_sldr->Enable();
	m_z_rot_sldr->Enable();
	m_x_rot_text->Enable();
	m_y_rot_text->Enable();
	m_z_rot_text->Enable();
	m_x_rot_spin->Enable();
	m_y_rot_spin->Enable();
	m_z_rot_spin->Enable();
	m_x1_clip_sldr->Enable();
	m_x1_clip_text->Enable();
	m_x2_clip_sldr->Enable();
	m_x2_clip_text->Enable();
	m_y1_clip_sldr->Enable();
	m_y1_clip_text->Enable();
	m_y2_clip_sldr->Enable();
	m_y2_clip_text->Enable();
	m_z1_clip_sldr->Enable();
	m_z1_clip_text->Enable();
	m_z2_clip_sldr->Enable();
	m_z2_clip_text->Enable();
	m_link_x_tb->Enable();
	m_link_y_tb->Enable();
	m_link_z_tb->Enable();
	m_yz_dist_text->Enable();
	m_xz_dist_text->Enable();
	m_xy_dist_text->Enable();
	m_clip_reset_btn->Enable();
}

void ClipPlanePanel::DisableAll()
{
	m_toolbar->Disable();
	m_set_zero_btn->Disable();
	m_rot_reset_btn->Disable();
	m_x_rot_sldr->Disable();
	m_y_rot_sldr->Disable();
	m_z_rot_sldr->Disable();
	m_x_rot_text->Disable();
	m_y_rot_text->Disable();
	m_z_rot_text->Disable();
	m_x_rot_spin->Disable();
	m_y_rot_spin->Disable();
	m_z_rot_spin->Disable();
	m_x1_clip_sldr->Disable();
	m_x1_clip_text->Disable();
	m_x2_clip_sldr->Disable();
	m_x2_clip_text->Disable();
	m_y1_clip_sldr->Disable();
	m_y1_clip_text->Disable();
	m_y2_clip_sldr->Disable();
	m_y2_clip_text->Disable();
	m_z1_clip_sldr->Disable();
	m_z1_clip_text->Disable();
	m_z2_clip_sldr->Disable();
	m_z2_clip_text->Disable();
	m_link_x_tb->Disable();
	m_link_y_tb->Disable();
	m_link_z_tb->Disable();
	m_yz_dist_text->Disable();
	m_xz_dist_text->Disable();
	m_xy_dist_text->Disable();
	m_clip_reset_btn->Disable();
}
