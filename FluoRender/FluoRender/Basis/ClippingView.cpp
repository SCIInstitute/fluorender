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
#include "ClippingView.h"
#include "VRenderFrame.h"
#include <DataManager.h>
#include <compatibility.h>
#include <wxDoubleSlider.h>
#include <wxSingleSlider.h>
#include <wx/valnum.h>
#include "png_resource.h"
#include "img/icons.h"

BEGIN_EVENT_TABLE(ClippingView, wxPanel)
	EVT_TOOL(ID_LinkChannelsBtn, ClippingView::OnLinkChannelsBtn)
	EVT_TOOL(ID_HoldPlanesBtn, ClippingView::OnHoldPlanesBtn)
	EVT_TOOL(ID_PlaneModesBtn, ClippingView::OnPlaneModesBtn)
	EVT_BUTTON(ID_SetZeroBtn, ClippingView::OnSetZeroBtn)
	EVT_BUTTON(ID_RotResetBtn, ClippingView::OnRotResetBtn)
	EVT_BUTTON(ID_ClipResetBtn, ClippingView::OnClipResetBtn)

	EVT_COMMAND_SCROLL(ID_X1ClipSldr, ClippingView::OnX1ClipChange)
	//EVT_COMMAND_SCROLL(ID_X2ClipSldr, ClippingView::OnX2ClipChange)
	EVT_COMMAND_SCROLL(ID_Y1ClipSldr, ClippingView::OnY1ClipChange)
	//EVT_COMMAND_SCROLL(ID_Y2ClipSldr, ClippingView::OnY2ClipChange)
	EVT_COMMAND_SCROLL(ID_Z1ClipSldr, ClippingView::OnZ1ClipChange)
	//EVT_COMMAND_SCROLL(ID_Z2ClipSldr, ClippingView::OnZ2ClipChange)

	EVT_TEXT(ID_X1ClipText, ClippingView::OnX1ClipEdit)
	EVT_TEXT(ID_X2ClipText, ClippingView::OnX2ClipEdit)
	EVT_TEXT(ID_Y1ClipText, ClippingView::OnY1ClipEdit)
	EVT_TEXT(ID_Y2ClipText, ClippingView::OnY2ClipEdit)
	EVT_TEXT(ID_Z1ClipText, ClippingView::OnZ1ClipEdit)
	EVT_TEXT(ID_Z2ClipText, ClippingView::OnZ2ClipEdit)

	EVT_IDLE(ClippingView::OnIdle)

	EVT_TOOL(ID_LinkXChk, ClippingView::OnLinkXCheck)
	EVT_TOOL(ID_LinkYChk, ClippingView::OnLinkYCheck)
	EVT_TOOL(ID_LinkZChk, ClippingView::OnLinkZCheck)

	EVT_COMMAND_SCROLL(ID_XRotSldr, ClippingView::OnXRotChange)
	EVT_COMMAND_SCROLL(ID_YRotSldr, ClippingView::OnYRotChange)
	EVT_COMMAND_SCROLL(ID_ZRotSldr, ClippingView::OnZRotChange)
	EVT_TEXT(ID_XRotText, ClippingView::OnXRotEdit)
	EVT_TEXT(ID_YRotText, ClippingView::OnYRotEdit)
	EVT_TEXT(ID_ZRotText, ClippingView::OnZRotEdit)

	//spin buttons
	EVT_SPIN_UP(ID_XRotSpin, ClippingView::OnXRotSpinUp)
	EVT_SPIN_DOWN(ID_XRotSpin, ClippingView::OnXRotSpinDown)
	EVT_SPIN_UP(ID_YRotSpin, ClippingView::OnYRotSpinUp)
	EVT_SPIN_DOWN(ID_YRotSpin, ClippingView::OnYRotSpinDown)
	EVT_SPIN_UP(ID_ZRotSpin, ClippingView::OnZRotSpinUp)
	EVT_SPIN_DOWN(ID_ZRotSpin, ClippingView::OnZRotSpinDown)

	//clip buttons
	EVT_BUTTON(ID_YZClipBtn, ClippingView::OnYZClipBtn)
	EVT_BUTTON(ID_XZClipBtn, ClippingView::OnXZClipBtn)
	EVT_BUTTON(ID_XYClipBtn, ClippingView::OnXYClipBtn)
END_EVENT_TABLE()

ClippingView::ClippingView(
	VRenderFrame* frame,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name) :
wxPanel(frame, wxID_ANY, pos, size, style, name),
m_view(0),
m_sel_type(0),
m_vd(0),
m_md(0),
m_draw_clip(false),
m_hold_planes(false),
m_plane_mode(kNormal)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);
	SetDoubleBuffered(true);

	//validator: floating point 1
	wxFloatingPointValidator<double> vald_fp1(1);
	vald_fp1.SetRange(-180.0, 180.0);
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;

	wxStaticText *st = 0;

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
	m_toolbar->Realize();
	sizer_1->Add(5, 5, 0);
	sizer_1->Add(m_toolbar, 0, wxALIGN_CENTER, 0);

	//sliders for clipping planes
	bool inverse_slider = frame->GetSettingDlg()->GetInverseSlider();
	long ls = inverse_slider ? wxSL_VERTICAL : (wxSL_VERTICAL | wxSL_INVERSE);
	//x
	wxBoxSizer* sizer_cx = new wxBoxSizer(wxVERTICAL);
	st = new wxStaticText(this, 0, "X");
	m_x1_clip_sldr = new wxDoubleSlider(this, ID_X1ClipSldr, 0, 512, 0, 512,
		wxDefaultPosition, wxDefaultSize, ls);
	m_x1_clip_sldr->SetRangeColor(wxColor(255, 128, 128));
	m_x1_clip_sldr->SetThumbColor(wxColor(255, 128, 128), wxColor(255, 128, 255));
	m_x1_clip_text = new wxTextCtrl(this, ID_X1ClipText, "0",
		wxDefaultPosition, FromDIP(wxSize(34, 20)), 0, vald_int);
	m_x2_clip_text = new wxTextCtrl(this, ID_X2ClipText, "512",
		wxDefaultPosition, FromDIP(wxSize(34, 20)), 0, vald_int);
	m_linkx_tb = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	bitmap = wxGetBitmapFromMemory(unlink);
	m_linkx_tb->AddCheckTool(ID_LinkXChk, "Lock X Planes",
		bitmap, wxNullBitmap, "Lock X Planes");
	//add the items
	sizer_cx->Add(5, 5, 0);
	sizer_cx->Add(st, 0, wxALIGN_CENTER, 0);
	if (inverse_slider)
	{
		sizer_cx->Add(m_x1_clip_text, 0, wxALIGN_CENTER, 0);
		sizer_cx->Add(5, 5, 0);
		sizer_cx->Add(m_x1_clip_sldr, 1, wxEXPAND, 0);
		sizer_cx->Add(m_x2_clip_text, 0, wxALIGN_CENTER, 0);
	}
	else
	{
		sizer_cx->Add(m_x2_clip_text, 0, wxALIGN_CENTER, 0);
		sizer_cx->Add(5, 5, 0);
		sizer_cx->Add(m_x1_clip_sldr, 1, wxEXPAND, 0);
		sizer_cx->Add(m_x1_clip_text, 0, wxALIGN_CENTER, 0);
	}
	sizer_cx->Add(5, 5, 0);
	sizer_cx->Add(m_linkx_tb, 0, wxALIGN_CENTER, 0);
	m_linkx_tb->Realize();

	//y
	wxBoxSizer* sizer_cy = new wxBoxSizer(wxVERTICAL);
	//wxPanel * ypanel = new wxPanel(this);
	st = new wxStaticText(this, 0, "Y");
	m_y1_clip_sldr = new wxDoubleSlider(this, ID_Y1ClipSldr, 0, 512, 0, 512,
		wxDefaultPosition, wxDefaultSize, ls);
	m_y1_clip_sldr->SetRangeColor(wxColor(128, 255, 128));
	m_y1_clip_sldr->SetThumbColor(wxColor(128, 255, 128), wxColor(255, 255, 128));
	m_y1_clip_text = new wxTextCtrl(this, ID_Y1ClipText, "0",
		wxDefaultPosition, FromDIP(wxSize(34, 20)), 0, vald_int);
	m_y2_clip_text = new wxTextCtrl(this, ID_Y2ClipText, "512",
		wxDefaultPosition, FromDIP(wxSize(34, 20)), 0, vald_int);
	m_linky_tb = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	m_linky_tb->AddCheckTool(ID_LinkYChk, "Lock Y Planes",
		bitmap, wxNullBitmap, "Lock Y Planes");
	//add the items
	sizer_cy->Add(5, 5, 0);
	sizer_cy->Add(st, 0, wxALIGN_CENTER, 0);
	if (inverse_slider)
	{
		sizer_cy->Add(m_y1_clip_text, 0, wxALIGN_CENTER, 0);
		sizer_cy->Add(5, 5, 0);
		sizer_cy->Add(m_y1_clip_sldr, 1, wxEXPAND, 0);
		sizer_cy->Add(m_y2_clip_text, 0, wxALIGN_CENTER, 0);
	}
	else
	{
		sizer_cy->Add(m_y2_clip_text, 0, wxALIGN_CENTER, 0);
		sizer_cy->Add(5, 5, 0);
		sizer_cy->Add(m_y1_clip_sldr, 1, wxEXPAND, 0);
		sizer_cy->Add(m_y1_clip_text, 0, wxALIGN_CENTER, 0);
	}
	sizer_cy->Add(5, 5, 0);
	sizer_cy->Add(m_linky_tb, 0, wxALIGN_CENTER, 0);
	m_linky_tb->Realize();

	//z
	wxBoxSizer* sizer_cz = new wxBoxSizer(wxVERTICAL);
	//wxPanel * zpanel = new wxPanel(this);
	st = new wxStaticText(this, 0, "Z");
	m_z1_clip_sldr = new wxDoubleSlider(this, ID_Z1ClipSldr, 0, 512, 0, 512,
		wxPoint(0,0), wxDefaultSize, ls);
	m_z1_clip_sldr->SetRangeColor(wxColor(128, 128, 255));
	m_z1_clip_sldr->SetThumbColor(wxColor(128, 128, 255), wxColor(128, 255, 255));
	m_z1_clip_text = new wxTextCtrl(this, ID_Z1ClipText, "0",
		wxDefaultPosition, FromDIP(wxSize(34, 20)), 0, vald_int);
	m_z2_clip_text = new wxTextCtrl(this, ID_Z2ClipText, "512",
		wxDefaultPosition, FromDIP(wxSize(34, 20)), 0, vald_int);
	m_linkz_tb = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	m_linkz_tb->AddCheckTool(ID_LinkZChk, "Lock Z Planes",
		bitmap, wxNullBitmap, "Lock Z Planes");
	//add the items
	sizer_cz->Add(5, 5, 0);
	sizer_cz->Add(st, 0, wxALIGN_CENTER, 0);
	if (inverse_slider)
	{
		sizer_cz->Add(m_z1_clip_text, 0, wxALIGN_CENTER, 0);
		sizer_cz->Add(5, 5, 0);
		sizer_cz->Add(m_z1_clip_sldr, 1, wxEXPAND, 0);
		sizer_cz->Add(m_z2_clip_text, 0, wxALIGN_CENTER, 0);
	}
	else
	{
		sizer_cz->Add(m_z2_clip_text, 0, wxALIGN_CENTER, 0);
		sizer_cz->Add(5, 5, 0);
		sizer_cz->Add(m_z1_clip_sldr, 1, wxEXPAND, 0);
		sizer_cz->Add(m_z1_clip_text, 0, wxALIGN_CENTER, 0);
	}
	sizer_cz->Add(5, 5, 0);
	sizer_cz->Add(m_linkz_tb, 0, wxALIGN_CENTER, 0);
	m_linkz_tb->Realize();

	//link sliders
	m_x1_clip_sldr->Connect(ID_X1ClipSldr, wxEVT_RIGHT_DOWN,
		wxCommandEventHandler(ClippingView::OnSliderRClick),
		NULL, this);
	m_y1_clip_sldr->Connect(ID_Y1ClipSldr, wxEVT_RIGHT_DOWN,
		wxCommandEventHandler(ClippingView::OnSliderRClick),
		NULL, this);
	m_z1_clip_sldr->Connect(ID_Z1ClipSldr, wxEVT_RIGHT_DOWN,
		wxCommandEventHandler(ClippingView::OnSliderRClick),
		NULL, this);

	//keys
	m_x1_clip_sldr->Connect(ID_X1ClipSldr, wxEVT_KEY_DOWN,
		wxKeyEventHandler(ClippingView::OnSliderKeyDown),
		NULL, this);
	m_y1_clip_sldr->Connect(ID_Y1ClipSldr, wxEVT_KEY_DOWN,
		wxKeyEventHandler(ClippingView::OnSliderKeyDown),
		NULL, this);
	m_z1_clip_sldr->Connect(ID_Z1ClipSldr, wxEVT_KEY_DOWN,
		wxKeyEventHandler(ClippingView::OnSliderKeyDown),
		NULL, this);

	//2
	wxBoxSizer *sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	sizer_2->Add(sizer_cx, 1, wxEXPAND);
	sizer_2->Add(sizer_cy, 1, wxEXPAND);
	sizer_2->Add(sizer_cz, 1, wxEXPAND);

	//clip buttons 4
	wxBoxSizer* sizer_4 = new wxBoxSizer(wxHORIZONTAL);
	m_yz_clip_btn = new wxButton(this, ID_YZClipBtn, "YZ",
		wxDefaultPosition, FromDIP(wxSize(34, 22)));
	m_xz_clip_btn = new wxButton(this, ID_XZClipBtn, "XZ",
		wxDefaultPosition, FromDIP(wxSize(34, 22)));
	m_xy_clip_btn = new wxButton(this, ID_XYClipBtn, "XY",
		wxDefaultPosition, FromDIP(wxSize(34, 22)));
	sizer_4->Add(m_yz_clip_btn, 1, wxEXPAND);
	sizer_4->AddSpacer(5);
	sizer_4->Add(m_xz_clip_btn, 1, wxEXPAND);
	sizer_4->AddSpacer(5);
	sizer_4->Add(m_xy_clip_btn, 1, wxEXPAND);

	//clip distance 5
	wxBoxSizer* sizer_5 = new wxBoxSizer(wxHORIZONTAL);
	m_yz_dist_text = new wxTextCtrl(this, ID_YZDistText, "1",
		wxDefaultPosition, FromDIP(wxSize(34, 22)), 0, vald_int);
	m_xz_dist_text = new wxTextCtrl(this, ID_XZDistText, "1",
		wxDefaultPosition, FromDIP(wxSize(34, 22)), 0, vald_int);
	m_xy_dist_text = new wxTextCtrl(this, ID_XYDistText, "1",
		wxDefaultPosition, FromDIP(wxSize(34, 22)), 0, vald_int);
	sizer_5->Add(m_yz_dist_text, 1, wxEXPAND);
	sizer_5->AddSpacer(5);
	sizer_5->Add(m_xz_dist_text, 1, wxEXPAND);
	sizer_5->AddSpacer(5);
	sizer_5->Add(m_xy_dist_text, 1, wxEXPAND);
	
	//reset clipping 6
	wxBoxSizer* sizer_6 = new wxBoxSizer(wxHORIZONTAL);
#ifndef _DARWIN
	m_clip_reset_btn = new wxButton(this, ID_ClipResetBtn, "Reset Clips",
									wxDefaultPosition, FromDIP(wxSize(120, 22)));
#else
	m_clip_reset_btn = new wxButton(this, ID_ClipResetBtn, "Reset Clips",
									wxDefaultPosition, FromDIP(wxSize(125, 30)));
#endif
	m_clip_reset_btn->SetBitmap(wxGetBitmapFromMemory(reset));
	sizer_6->Add(5, 5, 0);
	sizer_6->Add(m_clip_reset_btn, 0, wxALIGN_CENTER);

	//rotations
	//set sero rotation for clipping planes 7
	wxBoxSizer* sizer_7 = new wxBoxSizer(wxHORIZONTAL);
#ifndef _DARWIN
	m_set_zero_btn = new wxButton(this, ID_SetZeroBtn, "Align to View",
								  wxDefaultPosition, FromDIP(wxSize(120, 22)));
#else
	m_set_zero_btn = new wxButton(this, ID_SetZeroBtn, "Align to View",
								  wxDefaultPosition, FromDIP(wxSize(125, 30)));
#endif
	m_set_zero_btn->SetBitmap(wxGetBitmapFromMemory(align));
	sizer_7->Add(5, 5, 0);
	sizer_7->Add(m_set_zero_btn, 0, wxALIGN_CENTER);

	//reset rotations 8
	wxBoxSizer* sizer_8 = new wxBoxSizer(wxHORIZONTAL);
#ifndef _DARWIN
	m_rot_reset_btn = new wxButton(this, ID_RotResetBtn, "Reset to 0",
								   wxDefaultPosition, FromDIP(wxSize(120, 22)));
#else
	m_rot_reset_btn = new wxButton(this, ID_RotResetBtn, "Reset to 0",
								   wxDefaultPosition, FromDIP(wxSize(125, 30)));
#endif
	m_rot_reset_btn->SetBitmap(wxGetBitmapFromMemory(reset));
	sizer_8->Add(5, 5, 0);
	sizer_8->Add(m_rot_reset_btn, 0, wxALIGN_CENTER);

	ls = inverse_slider ? wxSL_VERTICAL : (wxSL_VERTICAL | wxSL_INVERSE);
	//sliders for rotating clipping planes 
	//x
	wxBoxSizer* sizer_rx = new wxBoxSizer(wxVERTICAL);
	st = new wxStaticText(this, 0, "X");
	m_x_rot_sldr = new wxSingleSlider(this, ID_XRotSldr, 0, -180, 180,
		wxDefaultPosition, wxDefaultSize, ls);
	m_x_rot_sldr->SetRangeColor(wxColor(0, 0, 255));
	m_x_rot_sldr->SetRangeStyle(2);
	m_x_rot_text = new wxTextCtrl(this, ID_XRotText, "0.0",
		wxDefaultPosition, FromDIP(wxSize(34, 20)), 0, vald_fp1);
	m_x_rot_spin = new wxSpinButton(this, ID_XRotSpin,
		wxDefaultPosition, FromDIP(wxSize(30, 20)), wxSP_VERTICAL);
	m_x_rot_spin->SetRange(-0x8000, 0x7fff);
	sizer_rx->Add(5, 5, 0);
	sizer_rx->Add(m_x_rot_text, 0, wxALIGN_CENTER, 0);
	sizer_rx->Add(st, 0, wxALIGN_CENTER, 0);
	sizer_rx->Add(5, 5, 0);
	sizer_rx->Add(m_x_rot_spin, 0, wxALIGN_CENTER, 0);
	sizer_rx->Add(m_x_rot_sldr, 1, wxEXPAND, 0);
	//y
	wxBoxSizer* sizer_ry = new wxBoxSizer(wxVERTICAL);
	st = new wxStaticText(this, 0, "Y");
	m_y_rot_sldr = new wxSingleSlider(this, ID_YRotSldr, 0, -180, 180,
		wxDefaultPosition, wxDefaultSize, ls);
	m_y_rot_sldr->SetRangeColor(wxColor(0, 0, 255));
	m_y_rot_sldr->SetRangeStyle(2);
	m_y_rot_text = new wxTextCtrl(this, ID_YRotText, "0.0",
		wxDefaultPosition, FromDIP(wxSize(34, 20)), 0, vald_fp1);
	m_y_rot_spin = new wxSpinButton(this, ID_YRotSpin,
		wxDefaultPosition, FromDIP(wxSize(30, 20)), wxSP_VERTICAL);
	m_y_rot_spin->SetRange(-0x8000, 0x7fff);
	sizer_ry->Add(5, 5, 0);
	sizer_ry->Add(m_y_rot_text, 0, wxALIGN_CENTER, 0);
	sizer_ry->Add(st, 0, wxALIGN_CENTER, 0);
	sizer_ry->Add(5, 5, 0);
	sizer_ry->Add(m_y_rot_spin, 0, wxALIGN_CENTER, 0);
	sizer_ry->Add(m_y_rot_sldr, 1, wxEXPAND, 0);
	//z
	wxBoxSizer* sizer_rz = new wxBoxSizer(wxVERTICAL);
	st = new wxStaticText(this, 0, "Z");
	m_z_rot_sldr = new wxSingleSlider(this, ID_ZRotSldr, 0, -180, 180,
		wxDefaultPosition, wxDefaultSize, ls);
	m_z_rot_sldr->SetRangeColor(wxColor(0, 0, 255));
	m_z_rot_sldr->SetRangeStyle(2);
	m_z_rot_text = new wxTextCtrl(this, ID_ZRotText, "0.0",
		wxDefaultPosition, FromDIP(wxSize(34, 20)), 0, vald_fp1);
	m_z_rot_spin = new wxSpinButton(this, ID_ZRotSpin,
		wxDefaultPosition, FromDIP(wxSize(30, 20)), wxSP_VERTICAL);
	m_z_rot_spin->SetRange(-0x8000, 0x7fff);
	sizer_rz->Add(5, 5, 0);
	sizer_rz->Add(m_z_rot_text, 0, wxALIGN_CENTER, 0);
	sizer_rz->Add(st, 0, wxALIGN_CENTER, 0);
	sizer_rz->Add(5, 5, 0);
	sizer_rz->Add(m_z_rot_spin, 0, wxALIGN_CENTER, 0);
	sizer_rz->Add(m_z_rot_sldr, 1, wxEXPAND, 0);
	
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
	sizer_v->Add(new wxStaticText(this,0,
		"Set Clip Slab Width"), 0, wxALIGN_CENTER);
	sizer_v->Add(5, 5, 0);
	sizer_v->Add(sizer_4, 0, wxALIGN_CENTER);
	sizer_v->Add(5, 5, 0);
	sizer_v->Add(sizer_5, 0, wxALIGN_CENTER);
	sizer_v->Add(5, 5, 0);
	sizer_v->Add(sizer_6, 0, wxALIGN_CENTER);
	sizer_v->Add(5, 5, 0);

	st = new wxStaticText(this, 0, "", wxDefaultPosition, FromDIP(wxSize(5, 5)));
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

ClippingView::~ClippingView()
{
}

void ClippingView::SetChannLink(bool chann)
{
	m_toolbar->ToggleTool(ID_LinkChannelsBtn,chann);
}

void ClippingView::SetHoldPlanes(bool hold)
{
	if (!m_view)
		return;
	m_hold_planes = hold;
	m_toolbar->ToggleTool(ID_HoldPlanesBtn, hold);
	if (hold)
	{
		m_view->m_draw_clip = true;
		m_view->m_clip_mask = -1;
	}
}

void ClippingView::SetPlaneMode(PLANE_MODES mode)
{
	m_plane_mode = mode;
	switch (m_plane_mode)
	{
	case kNormal:
		m_toolbar->SetToolNormalBitmap(ID_PlaneModesBtn,
			wxGetBitmapFromMemory(clip_normal));
		break;
	case kFrame6:
		m_toolbar->SetToolNormalBitmap(ID_PlaneModesBtn,
			wxGetBitmapFromMemory(clip_frame6));
		break;
	case kFrame3:
		m_toolbar->SetToolNormalBitmap(ID_PlaneModesBtn,
			wxGetBitmapFromMemory(clip_frame3));
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

int ClippingView::GetSelType()
{
	return m_sel_type;
}

VolumeData* ClippingView::GetVolumeData()
{
	return m_vd;
}

MeshData* ClippingView::GetMeshData()
{
	return m_md;
}

void ClippingView::SetVolumeData(VolumeData* vd)
{
	if (!vd) return;
	m_vd = vd;
	m_sel_type = 2;
}

void ClippingView::SetMeshData(MeshData* md)
{
	if (!md) return;
	m_md = md;
	m_sel_type = 3;
}

void ClippingView::SetDataManager(DataManager* mgr)
{
	m_mgr = mgr;
}

void ClippingView::GetSettings(VRenderGLView* view)
{
	m_view = view;
	if (!m_view ||
		(!m_vd && !m_md))
	{
		DisableAll();
		return;
	}

	EnableAll();

	int resx, resy, resz;
	int resx_n, resy_n, resz_n;
	fluo::Color fc;
	switch (m_sel_type)
	{
	case 2:	//volume
		m_vd->GetResolution(resx, resy, resz);
		resx_n = resy_n = resz_n = 0;
		fc = m_vd->GetColor();
		break;
	case 3:	//mesh
		resx = resy = resz = 0;
		resx_n = resy_n = resz_n = 0;
		break;
	}
	wxColor c(fc.r()*255, fc.g()*255, fc.b()*255);
	//slider range
	m_x1_clip_sldr->SetRange(resx_n, resx);
	if (m_sel_type == 2)
		m_x1_clip_sldr->SetRangeColor(c);
	//m_x2_clip_sldr->SetRange(resx_n, resx);
	m_y1_clip_sldr->SetRange(resy_n, resy);
	if (m_sel_type == 2)
		m_y1_clip_sldr->SetRangeColor(c);
	//m_y2_clip_sldr->SetRange(resy_n, resy);
	m_z1_clip_sldr->SetRange(resz_n, resz);
	if (m_sel_type == 2)
		m_z1_clip_sldr->SetRangeColor(c);
	//m_z2_clip_sldr->SetRange(resz_n, resz);
	//text range
	wxIntegerValidator<int>* vald_i;
	if ((vald_i = (wxIntegerValidator<int>*)m_x1_clip_text->GetValidator()))
		vald_i->SetRange(0, int(resx));
	if ((vald_i = (wxIntegerValidator<int>*)m_x2_clip_text->GetValidator()))
		vald_i->SetRange(0, int(resx));
	if ((vald_i = (wxIntegerValidator<int>*)m_y1_clip_text->GetValidator()))
		vald_i->SetRange(0, int(resy));
	if ((vald_i = (wxIntegerValidator<int>*)m_y2_clip_text->GetValidator()))
		vald_i->SetRange(0, int(resy));
	if ((vald_i = (wxIntegerValidator<int>*)m_z1_clip_text->GetValidator()))
		vald_i->SetRange(0, int(resz));
	if ((vald_i = (wxIntegerValidator<int>*)m_z2_clip_text->GetValidator()))
		vald_i->SetRange(0, int(resz));

	//clip distance
	switch (m_sel_type)
	{
	case 2:	//volume
		{
			int distx, disty, distz;
			m_vd->GetClipDistance(distx, disty, distz);
			if (distx == 0)
			{
				distx = resx/20;
				distx = distx==0?1:distx;
			}
			if (disty == 0)
			{
				disty = resy/20;
				disty = disty==0?1:disty;
			}
			if (distz == 0)
			{
				distz = resz/20;
				distz = distz==0?1:distz;
			}
			m_yz_dist_text->SetValue(
				wxString::Format("%d", distx));
			m_xz_dist_text->SetValue(
				wxString::Format("%d", disty));
			m_xy_dist_text->SetValue(
				wxString::Format("%d", distz));
			m_vd->SetClipDistance(distx, disty, distz);
		}
		break;
	case 3:	//mesh
		break;
	}

	vector<fluo::Plane*> *planes = 0;
	switch (m_sel_type)
	{
	case 2:	//volume
		if (m_vd->GetVR())
			planes = m_vd->GetVR()->get_planes();
		break;
	case 3:	//mesh
		if (m_md->GetMR())
			planes = m_md->GetMR()->get_planes();
		break;
	}
	if (!planes)
		return;
	if (planes->size()!=6)	//it has to be 6
		return;

	wxString str;
	fluo::Plane* plane = 0;
	int val = 0;
	double abcd[4];

	//x1
	plane = (*planes)[0];
	plane->get_copy(abcd);
	val = fabs(abcd[3]*resx)+0.499;
	m_x1_clip_sldr->SetLowValue(val);
	str = wxString::Format("%d", val);
	m_x1_clip_text->ChangeValue(str);
	//x2
	plane = (*planes)[1];
	plane->get_copy(abcd);
	val = fabs(abcd[3]*resx)+0.499;
	m_x1_clip_sldr->SetHighValue(val);
	str = wxString::Format("%d", val);
	m_x2_clip_text->ChangeValue(str);
	//y1
	plane = (*planes)[2];
	plane->get_copy(abcd);
	val = fabs(abcd[3]*resy)+0.499;
	m_y1_clip_sldr->SetLowValue(val);
	str = wxString::Format("%d", val);
	m_y1_clip_text->ChangeValue(str);
	//y2
	plane = (*planes)[3];
	plane->get_copy(abcd);
	val = fabs(abcd[3]*resy)+0.499;
	m_y1_clip_sldr->SetHighValue(val);
	str = wxString::Format("%d", val);
	m_y2_clip_text->ChangeValue(str);
	//z1
	plane = (*planes)[4];
	plane->get_copy(abcd);
	val = fabs(abcd[3]*resz)+0.499;
	m_z1_clip_sldr->SetLowValue(val);
	str = wxString::Format("%d", val);
	m_z1_clip_text->ChangeValue(str);
	//z2
	plane = (*planes)[5];
	plane->get_copy(abcd);
	val = fabs(abcd[3]*resz)+0.499;
	m_z1_clip_sldr->SetHighValue(val);
	str = wxString::Format("%d", val);
	m_z2_clip_text->ChangeValue(str);

	//rotations
	double rotx, roty, rotz;
	m_view->GetClippingPlaneRotations(rotx, roty, rotz);
	//x
	m_x_rot_text->ChangeValue(wxString::Format("%.1f", double(rotx)));
	m_x_rot_sldr->SetValue(int(std::round(rotx)));
	//y
	m_y_rot_text->ChangeValue(wxString::Format("%.1f", double(roty)));
	m_y_rot_sldr->SetValue(int(std::round(roty)));
	//z
	m_z_rot_text->ChangeValue(wxString::Format("%.1f", double(rotz)));
	m_z_rot_sldr->SetValue(int(std::round(rotz)));
}

void ClippingView::OnLinkChannelsBtn(wxCommandEvent &event)
{
	if (m_toolbar->GetToolState(ID_LinkChannelsBtn))
	{
		if (!m_mgr)
			return;

		wxString str;
		//x1
		str = m_x1_clip_text->GetValue();
		long x1_val = 0;
		str.ToLong(&x1_val);
		//x2
		str = m_x2_clip_text->GetValue();
		long x2_val = 0;
		str.ToLong(&x2_val);
		//y1
		str = m_y1_clip_text->GetValue();
		long y1_val = 0;
		str.ToLong(&y1_val);
		//y2
		str = m_y2_clip_text->GetValue();
		long y2_val = 0;
		str.ToLong(&y2_val);
		//z1
		str = m_z1_clip_text->GetValue();
		long z1_val = 0;
		str.ToLong(&z1_val);
		//z2
		str = m_z2_clip_text->GetValue();
		long z2_val = 0;
		str.ToLong(&z2_val);

		int i;

		for (i=0; i<m_mgr->GetVolumeNum(); i++)
		{
			VolumeData* vd = m_mgr->GetVolumeData(i);
			if (!vd || vd == m_vd)
				continue;

			vector<fluo::Plane*> *planes = 0;
			if (vd->GetVR())
				planes = vd->GetVR()->get_planes();
			if (!planes)
				continue;
			if (planes->size() != 6)
				continue;

			int resx, resy, resz;
			vd->GetResolution(resx, resy, resz);

			(*planes)[0]->ChangePlane(fluo::Point(double(x1_val)/double(resx), 0.0, 0.0), fluo::Vector(1.0, 0.0, 0.0));
			(*planes)[1]->ChangePlane(fluo::Point(double(x2_val)/double(resx), 0.0, 0.0), fluo::Vector(-1.0, 0.0, 0.0));
			(*planes)[2]->ChangePlane(fluo::Point(0.0, double(y1_val)/double(resy), 0.0), fluo::Vector(0.0, 1.0, 0.0));
			(*planes)[3]->ChangePlane(fluo::Point(0.0, double(y2_val)/double(resy), 0.0), fluo::Vector(0.0, -1.0, 0.0));
			(*planes)[4]->ChangePlane(fluo::Point(0.0, 0.0, double(z1_val)/double(resz)), fluo::Vector(0.0, 0.0, 1.0));
			(*planes)[5]->ChangePlane(fluo::Point(0.0, 0.0, double(z2_val)/double(resz)), fluo::Vector(0.0, 0.0, -1.0));
		}

		if (m_view)
			m_view->RefreshGL(51);
	}
}

void ClippingView::OnHoldPlanesBtn(wxCommandEvent &event)
{
	m_hold_planes = m_toolbar->GetToolState(ID_HoldPlanesBtn);
}

void ClippingView::OnPlaneModesBtn(wxCommandEvent &event)
{
	switch (m_plane_mode)
	{
	case kNormal:
		m_plane_mode = kFrame6;
		m_toolbar->SetToolNormalBitmap(ID_PlaneModesBtn,
			wxGetBitmapFromMemory(clip_frame6));
		break;
	case kFrame6:
		m_plane_mode = kFrame3;
		m_toolbar->SetToolNormalBitmap(ID_PlaneModesBtn,
			wxGetBitmapFromMemory(clip_frame3));
		break;
	case kFrame3:
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

	if (m_view)
		m_view->RefreshGL(51);
}

void ClippingView::OnClipResetBtn(wxCommandEvent &event)
{
	if (!m_vd)
		return;
	int resx, resy, resz;
	m_vd->GetResolution(resx, resy, resz);

	vector<fluo::Plane*> *planes = 0;
	if (m_vd->GetVR())
		planes = m_vd->GetVR()->get_planes();
	if(!planes)
		return;
	if (planes->size()!=6)
		return;

	fluo::Plane* plane = (*planes)[0];
	plane->ChangePlane(fluo::Point(0.0, 0.0, 0.0), fluo::Vector(1.0, 0.0, 0.0));
	plane = (*planes)[1];
	plane->ChangePlane(fluo::Point(1.0, 0.0, 0.0), fluo::Vector(-1.0, 0.0, 0.0));
	plane = (*planes)[2];
	plane->ChangePlane(fluo::Point(0.0, 0.0, 0.0), fluo::Vector(0.0, 1.0, 0.0));
	plane = (*planes)[3];
	plane->ChangePlane(fluo::Point(0.0, 1.0, 0.0), fluo::Vector(0.0, -1.0, 0.0));
	plane = (*planes)[4];
	plane->ChangePlane(fluo::Point(0.0, 0.0, 0.0), fluo::Vector(0.0, 0.0, 1.0));
	plane = (*planes)[5];
	plane->ChangePlane(fluo::Point(0.0, 0.0, 1.0), fluo::Vector(0.0, 0.0, -1.0));

	//links
	SetXLink(false);
	SetYLink(false);
	SetZLink(false);

	//controls
	//texts
	m_x1_clip_text->SetValue("0");
	m_x2_clip_text->SetValue(wxString::Format("%d", resx));
	m_y1_clip_text->SetValue("0");
	m_y2_clip_text->SetValue(wxString::Format("%d", resy));
	m_z1_clip_text->SetValue("0");
	m_z2_clip_text->SetValue(wxString::Format("%d", resz));

	//link
	if (m_toolbar->GetToolState(ID_LinkChannelsBtn))
	{
		if (m_mgr)
		{
			int i;
			for (i=0; i<m_mgr->GetVolumeNum(); i++)
			{
				VolumeData* vd = m_mgr->GetVolumeData(i);
				if (!vd || vd == m_vd)
					continue;

				planes = 0;
				if (m_vd->GetVR())
					planes = vd->GetVR()->get_planes();
				if (!planes)
					continue;
				if (planes->size() != 6)
					continue;

				(*planes)[0]->ChangePlane(fluo::Point(0.0, 0.0, 0.0), fluo::Vector(1.0, 0.0, 0.0));
				(*planes)[1]->ChangePlane(fluo::Point(1.0, 0.0, 0.0), fluo::Vector(-1.0, 0.0, 0.0));
				(*planes)[2]->ChangePlane(fluo::Point(0.0, 0.0, 0.0), fluo::Vector(0.0, 1.0, 0.0));
				(*planes)[3]->ChangePlane(fluo::Point(0.0, 1.0, 0.0), fluo::Vector(0.0, -1.0, 0.0));
				(*planes)[4]->ChangePlane(fluo::Point(0.0, 0.0, 0.0), fluo::Vector(0.0, 0.0, 1.0));
				(*planes)[5]->ChangePlane(fluo::Point(0.0, 0.0, 1.0), fluo::Vector(0.0, 0.0, -1.0));
			}
		}
	}

	//views
	m_view->m_clip_mask = -1;
	m_view->UpdateClips();
	m_view->RefreshGL(51);
}

void ClippingView::OnX1ClipChange(wxScrollEvent &event)
{
	int ival = m_x1_clip_sldr->GetLowValue();
	wxString str = wxString::Format("%d", ival);
	if (str != m_x1_clip_text->GetValue())
		m_x1_clip_text->SetValue(str);

	ival = m_x1_clip_sldr->GetHighValue();
	str = wxString::Format("%d", ival);
	if (str != m_x2_clip_text->GetValue())
		m_x2_clip_text->SetValue(str);
}

void ClippingView::OnX1ClipEdit(wxCommandEvent &event)
{
	if (!m_view || !m_vd)
		return;

	int resx, resy, resz;
	m_vd->GetResolution(resx, resy, resz);

	vector<fluo::Plane*> *planes = 0;
	if (m_vd->GetVR())
		planes = m_vd->GetVR()->get_planes();
	if(!planes)
		return;
	if (planes->size()!=6)
		return;

	wxString str = m_x1_clip_text->GetValue();
	long ival = 0;
	str.ToLong(&ival);
	double val, val2;
	bool link = m_x1_clip_sldr->GetLink();

	val = (double)ival/(double)resx;
	m_x1_clip_sldr->SetLowValue(ival);
	int ival2 = m_x1_clip_sldr->GetHighValue();
	val2 = (double)ival2 / (double)resx;
	fluo::Plane* plane = (*planes)[0];
	plane->ChangePlane(fluo::Point(val, 0.0, 0.0), fluo::Vector(1.0, 0.0, 0.0));

	if (m_toolbar->GetToolState(ID_LinkChannelsBtn))
	{
		if (m_mgr)
		{
			int i;
			for (i=0; i<m_mgr->GetVolumeNum(); i++)
			{
				VolumeData* vd = m_mgr->GetVolumeData(i);
				if (!vd || vd == m_vd)
					continue;

				planes = 0;
				if (m_vd->GetVR())
					planes = vd->GetVR()->get_planes();
				if (!planes)
					continue;
				if (planes->size() != 6)
					continue;

				(*planes)[0]->ChangePlane(fluo::Point(val, 0.0, 0.0), fluo::Vector(1.0, 0.0, 0.0));
				if (link)
					(*planes)[1]->ChangePlane(fluo::Point(val2, 0.0, 0.0), fluo::Vector(-1.0, 0.0, 0.0));
			}
		}
	}

	if (link)
		m_view->m_clip_mask = 3;
	else
		m_view->m_clip_mask = 1;
	m_view->UpdateClips();
	m_view->RefreshGL(51);
}

void ClippingView::OnX2ClipEdit(wxCommandEvent &event)
{
	if (!m_view || !m_vd)
		return;
	int resx, resy, resz;
	m_vd->GetResolution(resx, resy, resz);

	vector<fluo::Plane*> *planes = 0;
	if (m_vd->GetVR())
		planes = m_vd->GetVR()->get_planes();
	if (!planes)
		return;
	if (planes->size()!=6)
		return;

	wxString str = m_x2_clip_text->GetValue();
	long ival = 0;
	str.ToLong(&ival);
	double val, val2;
	bool link = m_x1_clip_sldr->GetLink();

	val = (double)ival/(double)resx;
	m_x1_clip_sldr->SetHighValue(ival);
	int ival2 = m_x1_clip_sldr->GetLowValue();
	val2 = (double)ival2 / (double)resx;
	fluo::Plane* plane = (*planes)[1];
	plane->ChangePlane(fluo::Point(val, 0.0, 0.0), fluo::Vector(-1.0, 0.0, 0.0));

	if (m_toolbar->GetToolState(ID_LinkChannelsBtn))
	{
		if (m_mgr)
		{
			int i;
			for (i=0; i<m_mgr->GetVolumeNum(); i++)
			{
				VolumeData* vd = m_mgr->GetVolumeData(i);
				if (!vd || vd == m_vd)
					continue;

				planes = 0;
				if (m_vd->GetVR())
					planes = vd->GetVR()->get_planes();
				if (!planes)
					continue;
				if (planes->size() != 6)
					continue;
				(*planes)[1]->ChangePlane(fluo::Point(val, 0.0, 0.0), fluo::Vector(-1.0, 0.0, 0.0));
				if (link)
					(*planes)[0]->ChangePlane(fluo::Point(val2, 0.0, 0.0), fluo::Vector(1.0, 0.0, 0.0));
			}
		}
	}

	if (link)
		m_view->m_clip_mask = 3;
	else
		m_view->m_clip_mask = 2;
	m_view->UpdateClips();

	m_view->RefreshGL(51);
}

void ClippingView::OnY1ClipChange(wxScrollEvent &event)
{
	int ival = m_y1_clip_sldr->GetLowValue();
	wxString str = wxString::Format("%d", ival);
	if (str != m_y1_clip_text->GetValue())
		m_y1_clip_text->SetValue(str);

	ival = m_y1_clip_sldr->GetHighValue();
	str = wxString::Format("%d", ival);
	if (str != m_y2_clip_text->GetValue())
		m_y2_clip_text->SetValue(str);
}

void ClippingView::OnY1ClipEdit(wxCommandEvent &event)
{
	if (!m_view || !m_vd)
		return;
	int resx, resy, resz;
	m_vd->GetResolution(resx, resy, resz);

	vector<fluo::Plane*> *planes = 0;
	if (m_vd->GetVR())
		planes = m_vd->GetVR()->get_planes();
	if (!planes)
		return;
	if (planes->size()!=6)
		return;

	wxString str = m_y1_clip_text->GetValue();
	long ival = 0;
	str.ToLong(&ival);
	double val, val2;
	bool link = m_y1_clip_sldr->GetLink();

	val = (double)ival/(double)resy;
	m_y1_clip_sldr->SetLowValue(ival);
	int ival2 = m_y1_clip_sldr->GetHighValue();
	val2 = (double)ival2 / (double)resy;
	fluo::Plane* plane = (*planes)[2];
	plane->ChangePlane(fluo::Point(0.0, val, 0.0), fluo::Vector(0.0, 1.0, 0.0));

	if (m_toolbar->GetToolState(ID_LinkChannelsBtn))
	{
		if (m_mgr)
		{
			int i;
			for (i=0; i<m_mgr->GetVolumeNum(); i++)
			{
				VolumeData* vd = m_mgr->GetVolumeData(i);
				if (!vd || vd == m_vd)
					continue;

				planes = 0;
				if (m_vd->GetVR())
					planes = vd->GetVR()->get_planes();
				if (!planes)
					continue;
				if (planes->size() != 6)
					continue;

				(*planes)[2]->ChangePlane(fluo::Point(0.0, val, 0.0), fluo::Vector(0.0, 1.0, 0.0));
				if (link)
					(*planes)[3]->ChangePlane(fluo::Point(0.0, val2, 0.0), fluo::Vector(0.0, -1.0, 0.0));
			}
		}
	}

	if (link)
		m_view->m_clip_mask = 12;
	else
		m_view->m_clip_mask = 4;
	m_view->UpdateClips();

	m_view->RefreshGL(51);
}

void ClippingView::OnY2ClipEdit(wxCommandEvent &event)
{
	if (!m_view || !m_vd)
		return;
	int resx, resy, resz;
	m_vd->GetResolution(resx, resy, resz);
	vector<fluo::Plane*> *planes = 0;
	if (m_vd->GetVR())
		planes = m_vd->GetVR()->get_planes();
	if (!planes)
		return;
	if (planes->size()!=6)
		return;

	wxString str = m_y2_clip_text->GetValue();
	long ival = 0;
	str.ToLong(&ival);
	double val, val2;
	bool link = m_y1_clip_sldr->GetLink();

	val = (double)ival/(double)resy;
	m_y1_clip_sldr->SetHighValue(ival);
	int ival2 = m_y1_clip_sldr->GetLowValue();
	val2 = (double)ival2 / (double)resy;
	fluo::Plane* plane = (*planes)[3];
	plane->ChangePlane(fluo::Point(0.0, val, 0.0), fluo::Vector(0.0, -1.0, 0.0));

	if (m_toolbar->GetToolState(ID_LinkChannelsBtn))
	{
		if (m_mgr)
		{
			int i;
			for (i=0; i<m_mgr->GetVolumeNum(); i++)
			{
				VolumeData* vd = m_mgr->GetVolumeData(i);
				if (!vd || vd == m_vd)
					continue;

				planes = 0;
				if (m_vd->GetVR())
					planes = vd->GetVR()->get_planes();
				if (!planes)
					continue;
				if (planes->size() != 6)
					continue;

				(*planes)[3]->ChangePlane(fluo::Point(0.0, val, 0.0), fluo::Vector(0.0, -1.0, 0.0));
				if (link)
					(*planes)[2]->ChangePlane(fluo::Point(0.0, val2, 0.0), fluo::Vector(0.0, 1.0, 0.0));
			}
		}
	}

	if (link)
		m_view->m_clip_mask = 12;
	else
		m_view->m_clip_mask = 8;
	m_view->UpdateClips();

	m_view->RefreshGL(51);
}

void ClippingView::OnZ1ClipChange(wxScrollEvent &event)
{
	int ival = m_z1_clip_sldr->GetLowValue();
	wxString str = wxString::Format("%d", ival);
	if (str != m_z1_clip_text->GetValue())
		m_z1_clip_text->SetValue(str);

	ival = m_z1_clip_sldr->GetHighValue();
	str = wxString::Format("%d", ival);
	if (str != m_z2_clip_text->GetValue())
		m_z2_clip_text->SetValue(str);
}

void ClippingView::OnZ1ClipEdit(wxCommandEvent &event)
{
	if (!m_view || !m_vd)
		return;
	int resx, resy, resz;
	m_vd->GetResolution(resx, resy, resz);
	vector<fluo::Plane*> *planes = 0;
	if (m_vd->GetVR())
		planes = m_vd->GetVR()->get_planes();
	if (!planes)
		return;
	if (planes->size()!=6)
		return;

	wxString str = m_z1_clip_text->GetValue();
	long ival = 0;
	str.ToLong(&ival);
	double val, val2;
	bool link = m_z1_clip_sldr->GetLink();

	val = (double)ival/(double)resz;
	m_z1_clip_sldr->SetLowValue(ival);
	int ival2 = m_z1_clip_sldr->GetHighValue();
	val2 = (double)ival2 / (double)resz;
	fluo::Plane* plane = (*planes)[4];
	plane->ChangePlane(fluo::Point(0.0, 0.0, val), fluo::Vector(0.0, 0.0, 1.0));

	if (m_toolbar->GetToolState(ID_LinkChannelsBtn))
	{
		if (m_mgr)
		{
			int i;
			for (i=0; i<m_mgr->GetVolumeNum(); i++)
			{
				VolumeData* vd = m_mgr->GetVolumeData(i);
				if (!vd || vd == m_vd)
					continue;

				planes = 0;
				if (m_vd->GetVR())
					planes = vd->GetVR()->get_planes();
				if (!planes)
					continue;
				if (planes->size() != 6)
					continue;

				(*planes)[4]->ChangePlane(fluo::Point(0.0, 0.0, val), fluo::Vector(0.0, 0.0, 1.0));
				if (link)
					(*planes)[5]->ChangePlane(fluo::Point(0.0, 0.0, val2), fluo::Vector(0.0, 0.0, -1.0));
			}
		}
	}

	if (link)
		m_view->m_clip_mask = 48;
	else
		m_view->m_clip_mask = 16;
	m_view->UpdateClips();

	m_view->RefreshGL(51);
}

void ClippingView::OnZ2ClipEdit(wxCommandEvent &event)
{
	if (!m_view || !m_vd)
		return;
	int resx, resy, resz;
	m_vd->GetResolution(resx, resy, resz);
	vector<fluo::Plane*> *planes = 0;
	if (m_vd->GetVR())
		planes = m_vd->GetVR()->get_planes();
	if (!planes)
		return;
	if (planes->size()!=6)
		return;

	wxString str = m_z2_clip_text->GetValue();
	long ival = 0;
	str.ToLong(&ival);
	double val, val2;
	bool link = m_z1_clip_sldr->GetLink();

	val = (double)ival/(double)resz;
	m_z1_clip_sldr->SetHighValue(ival);
	int ival2 = m_z1_clip_sldr->GetLowValue();
	val2 = (double)ival2 / (double)resz;
	fluo::Plane* plane = (*planes)[5];
	plane->ChangePlane(fluo::Point(0.0, 0.0, val), fluo::Vector(0.0, 0.0, -1.0));

	if (m_toolbar->GetToolState(ID_LinkChannelsBtn))
	{
		if (m_mgr)
		{
			int i;
			for (i=0; i<m_mgr->GetVolumeNum(); i++)
			{
				VolumeData* vd = m_mgr->GetVolumeData(i);
				if (!vd || vd == m_vd)
					continue;

				planes = 0;
				if (vd->GetVR())
					planes = vd->GetVR()->get_planes();
				if (!planes)
					continue;
				if (planes->size() != 6)
					continue;

				(*planes)[5]->ChangePlane(fluo::Point(0.0, 0.0, val), fluo::Vector(0.0, 0.0, -1.0));
				if (link)
					(*planes)[4]->ChangePlane(fluo::Point(0.0, 0.0, val2), fluo::Vector(0.0, 0.0, 1.0));
			}
		}
	}

	if (link)
		m_view->m_clip_mask = 48;
	else
		m_view->m_clip_mask = 32;
	m_view->UpdateClips();

	m_view->RefreshGL(51);
}

void ClippingView::OnIdle(wxIdleEvent &event)
{
	if (!IsShown())
		return;
	if (!m_view)
		return;

	if (m_hold_planes)
	{
		m_draw_clip = true;
		return;
	}

	if (m_view->m_capture)
		return;

	wxPoint pos = wxGetMousePosition();
	wxRect reg = GetScreenRect();
	wxWindow *window = wxWindow::FindFocus();
	if (window && reg.Contains(pos))
	{
		if (!m_draw_clip)
		{
			m_view->m_draw_clip = true;
			m_view->m_clip_mask = -1;
			m_view->RefreshGL(51);
			m_draw_clip = true;
		}
	}
	else
	{
		if (m_draw_clip)
		{
			m_view->m_draw_clip = false;
			m_view->RefreshGL(51);
			m_draw_clip = false;
		}
	}

}

bool ClippingView::GetXLink()
{
	return m_x1_clip_sldr->GetLink();
}
bool ClippingView::GetYLink()
{
	return m_y1_clip_sldr->GetLink();
}
bool ClippingView::GetZLink()
{
	return m_z1_clip_sldr->GetLink();
}

void ClippingView::SetXLink(bool val)
{
	m_x1_clip_sldr->SetLink(val);
	m_linkx_tb->ToggleTool(ID_LinkXChk, val);
	if (val)
		m_linkx_tb->SetToolNormalBitmap(ID_LinkXChk,
			wxGetBitmapFromMemory(link));
	else
		m_linkx_tb->SetToolNormalBitmap(ID_LinkXChk,
			wxGetBitmapFromMemory(unlink));
}

void ClippingView::SetYLink(bool val)
{
	m_y1_clip_sldr->SetLink(val);
	m_linky_tb->ToggleTool(ID_LinkYChk, val);
	if (val)
		m_linky_tb->SetToolNormalBitmap(ID_LinkYChk,
			wxGetBitmapFromMemory(link));
	else
		m_linky_tb->SetToolNormalBitmap(ID_LinkYChk,
			wxGetBitmapFromMemory(unlink));
}

void ClippingView::SetZLink(bool val)
{
	m_z1_clip_sldr->SetLink(val);
	m_linkz_tb->ToggleTool(ID_LinkZChk, val);
	if (val)
		m_linkz_tb->SetToolNormalBitmap(ID_LinkZChk,
			wxGetBitmapFromMemory(link));
	else
		m_linkz_tb->SetToolNormalBitmap(ID_LinkZChk,
			wxGetBitmapFromMemory(unlink));
}

void ClippingView::SetClippingPlaneRotations(double rotx, double roty, double rotz)
{
	m_x_rot_sldr->SetValue(int(rotx));
	m_y_rot_sldr->SetValue(int(roty));
	m_z_rot_sldr->SetValue(int(rotz));
	m_x_rot_text->SetValue(wxString::Format("%.1f", rotx));
	m_y_rot_text->SetValue(wxString::Format("%.1f", roty));
	m_z_rot_text->SetValue(wxString::Format("%.1f", rotz));
}

void ClippingView::OnLinkXCheck(wxCommandEvent &event)
{
	bool link = m_linkx_tb->GetToolState(ID_LinkXChk);
	SetXLink(link);
}

void ClippingView::OnLinkYCheck(wxCommandEvent &event)
{
	bool link = m_linky_tb->GetToolState(ID_LinkYChk);
	SetYLink(link);
}

void ClippingView::OnLinkZCheck(wxCommandEvent &event)
{
	bool link = m_linkz_tb->GetToolState(ID_LinkZChk);
	SetZLink(link);
}

void ClippingView::OnSetZeroBtn(wxCommandEvent &event)
{
	if (!m_view)
		return;

	m_view->SetClipMode(2);
	m_view->RefreshGL(51);
	double rotx, roty, rotz;
	m_view->GetClippingPlaneRotations(rotx, roty, rotz);
	m_x_rot_sldr->SetValue(int(std::round(rotx)));
	m_y_rot_sldr->SetValue(int(std::round(roty)));
	m_z_rot_sldr->SetValue(int(std::round(rotz)));
	m_x_rot_text->ChangeValue(wxString::Format("%.1f", rotx));
	m_y_rot_text->ChangeValue(wxString::Format("%.1f", roty));
	m_z_rot_text->ChangeValue(wxString::Format("%.1f", rotz));
}

void ClippingView::OnRotResetBtn(wxCommandEvent &event)
{
	if (!m_view)
		return;

	//reset rotations
	m_view->SetClippingPlaneRotations(0.0, 0.0, 0.0);
	m_view->RefreshGL(51);

	wxString str = "0.0";
	m_x_rot_sldr->SetValue(0);
	m_x_rot_text->ChangeValue(str);
	m_y_rot_sldr->SetValue(0);
	m_y_rot_text->ChangeValue(str);
	m_z_rot_sldr->SetValue(0);
	m_z_rot_text->ChangeValue(str);
}

void ClippingView::OnXRotChange(wxScrollEvent &event)
{
	int val = m_x_rot_sldr->GetValue();
	wxString str = wxString::Format("%.1f", double(val));
	if (str != m_x_rot_text->GetValue())
		m_x_rot_text->SetValue(str);
}

void ClippingView::OnXRotEdit(wxCommandEvent &event)
{
	if (!m_view)
		return;

	wxString str = m_x_rot_text->GetValue();
	double val = 0.0;
	str.ToDouble(&val);
	m_x_rot_sldr->SetValue(int(std::round(val)));

	double rotx, roty, rotz;
	m_view->GetClippingPlaneRotations(rotx, roty, rotz);
	m_view->SetClippingPlaneRotations(val, roty, rotz);
	m_view->RefreshGL(51);
}

void ClippingView::OnYRotChange(wxScrollEvent &event)
{
	int val = m_y_rot_sldr->GetValue();
	wxString str = wxString::Format("%.1f", double(val));
	if (str != m_y_rot_text->GetValue())
		m_y_rot_text->SetValue(str);
}

void ClippingView::OnYRotEdit(wxCommandEvent &event)
{
	if (!m_view)
		return;

	wxString str = m_y_rot_text->GetValue();
	double val = 0.0;
	str.ToDouble(&val);
	m_y_rot_sldr->SetValue(int(std::round(val)));

	double rotx, roty, rotz;
	m_view->GetClippingPlaneRotations(rotx, roty, rotz);
	m_view->SetClippingPlaneRotations(rotx, val, rotz);
	m_view->RefreshGL(51);
}

void ClippingView::OnZRotChange(wxScrollEvent &event)
{
	int val = m_z_rot_sldr->GetValue();
	wxString str = wxString::Format("%.1f", double(val));
	if (str != m_z_rot_text->GetValue())
		m_z_rot_text->SetValue(str);
}

void ClippingView::OnZRotEdit(wxCommandEvent &event)
{
	if (!m_view)
		return;
	wxString str = m_z_rot_text->GetValue();
	double val = 0.0;
	str.ToDouble(&val);
	m_z_rot_sldr->SetValue(int(std::round(val)));

	double rotx, roty, rotz;
	m_view->GetClippingPlaneRotations(rotx, roty, rotz);
	m_view->SetClippingPlaneRotations(rotx, roty, val);
	m_view->RefreshGL(51);
}

void ClippingView::OnXRotSpinUp(wxSpinEvent& event)
{
	wxString str_val = m_x_rot_text->GetValue();
	double val = STOD(str_val.fn_str()) + 1.0;
	if (val > 180.0) val -= 360.0;
	if (val <-180.0) val += 360.0;
	wxString str = wxString::Format("%.1f", val);
	m_x_rot_text->SetValue(str);
}

void ClippingView::OnXRotSpinDown(wxSpinEvent& event)
{
	wxString str_val = m_x_rot_text->GetValue();
	double val = STOD(str_val.fn_str()) - 1.0;
	if (val > 180.0) val -= 360.0;
	if (val <-180.0) val += 360.0;
	wxString str = wxString::Format("%.1f", val);
	m_x_rot_text->SetValue(str);
}

void ClippingView::OnYRotSpinUp(wxSpinEvent& event)
{
	wxString str_val = m_y_rot_text->GetValue();
	double val = STOD(str_val.fn_str()) + 1.0;
	if (val > 180.0) val -= 360.0;
	if (val <-180.0) val += 360.0;
	wxString str = wxString::Format("%.1f", val);
	m_y_rot_text->SetValue(str);
}

void ClippingView::OnYRotSpinDown(wxSpinEvent& event)
{
	wxString str_val = m_y_rot_text->GetValue();
	double val = STOD(str_val.fn_str()) - 1.0;
	if (val > 180.0) val -= 360.0;
	if (val <-180.0) val += 360.0;
	wxString str = wxString::Format("%.1f", val);
	m_y_rot_text->SetValue(str);
}

void ClippingView::OnZRotSpinUp(wxSpinEvent& event)
{
	wxString str_val = m_z_rot_text->GetValue();
	double val = STOD(str_val.fn_str()) + 1.0;
	if (val > 180.0) val -= 360.0;
	if (val <-180.0) val += 360.0;
	wxString str = wxString::Format("%.1f", val);
	m_z_rot_text->SetValue(str);
}

void ClippingView::OnZRotSpinDown(wxSpinEvent& event)
{
	wxString str_val = m_z_rot_text->GetValue();
	double val = STOD(str_val.fn_str()) - 1.0;
	if (val > 180.0) val -= 360.0;
	if (val <-180.0) val += 360.0;
	wxString str = wxString::Format("%.1f", val);
	m_z_rot_text->SetValue(str);
}

void ClippingView::OnSliderRClick(wxCommandEvent& event)
{
	if (m_sel_type!=2 || !m_vd)
		return;

	int id = event.GetId();

	wxString str;
	double val;

	//good rate
	if (m_vd->GetSampleRate()<2.0)
		m_vd->SetSampleRate(2.0);
	if (m_toolbar->GetToolState(ID_LinkChannelsBtn))
	{
		if (m_mgr)
		{
			int i;
			for (i=0; i<m_mgr->GetVolumeNum(); i++)
			{
				VolumeData* vd = m_mgr->GetVolumeData(i);
				if (!vd || vd == m_vd)
					continue;
				if (vd->GetSampleRate()<2.0)
					vd->SetSampleRate(2.0);
			}
		}
	}

	int resx, resy, resz;
	m_vd->GetResolution(resx, resy, resz);

	if (id == ID_X1ClipSldr)
	{
		str = m_x1_clip_text->GetValue();
		str.ToDouble(&val);
		if (val < resx)
		{
			SetXLink(false);
			m_x2_clip_text->SetValue(
				wxString::Format("%d", (int)(val+1.0)));
			SetXLink(true);
		}
		m_x1_clip_sldr->SetFocus();
	}
	else if (id == ID_X2ClipSldr)
	{
		str = m_x2_clip_text->GetValue();
		str.ToDouble(&val);
		if (val > 0)
		{
			SetXLink(false);
			m_x1_clip_text->SetValue(
				wxString::Format("%d", (int)(val-1.0)));
			SetXLink(true);
		}
		//m_x2_clip_sldr->SetFocus();
	}
	else if (id == ID_Y1ClipSldr)
	{
		str = m_y1_clip_text->GetValue();
		str.ToDouble(&val);
		if (val < resy)
		{
			SetYLink(false);
			m_y2_clip_text->SetValue(
				wxString::Format("%d", (int)(val+1.0)));
			SetYLink(true);
		}
		m_y1_clip_sldr->SetFocus();
	}
	else if (id == ID_Y2ClipSldr)
	{
		str = m_y2_clip_text->GetValue();
		str.ToDouble(&val);
		if (val > 0)
		{
			SetYLink(false);
			m_y1_clip_text->SetValue(
				wxString::Format("%d", (int)(val-1.0)));
			SetYLink(true);
		}
		//m_y2_clip_sldr->SetFocus();
	}
	else if (id == ID_Z1ClipSldr)
	{
		str = m_z1_clip_text->GetValue();
		str.ToDouble(&val);
		if (val < resz)
		{
			SetZLink(false);
			m_z2_clip_text->SetValue(
				wxString::Format("%d", (int)(val+1.0)));
			SetZLink(true);
		}
		m_z1_clip_sldr->SetFocus();
	}
	else if (id == ID_Z2ClipSldr)
	{
		str = m_z2_clip_text->GetValue();
		str.ToDouble(&val);
		if (val > 0)
		{
			SetZLink(false);
			m_z1_clip_text->SetValue(
				wxString::Format("%d", (int)(val-1.0)));
			SetZLink(true);
		}
		//m_z2_clip_sldr->SetFocus();
	}

	//reset others
	vector<fluo::Plane*> *planes = 0;
	if (m_vd->GetVR())
		planes = m_vd->GetVR()->get_planes();
	if(!planes)
		return;
	if (planes->size()!=6)
		return;

	if (id == ID_X1ClipSldr ||
		id == ID_X2ClipSldr)
	{
		//m_link_y = false;
		//m_check_tb->ToggleTool(ID_LinkYChk,false);
		//m_check_tb->SetToolNormalBitmap(ID_LinkYChk,
		//	wxGetBitmapFromMemory(unlink));
		//m_link_z = false;
		//m_check_tb->ToggleTool(ID_LinkZChk,false);
		//m_check_tb->SetToolNormalBitmap(ID_LinkZChk,
		//	wxGetBitmapFromMemory(unlink));
		SetYLink(false);
		SetZLink(false);
		m_y1_clip_text->SetValue("0");
		m_y2_clip_text->SetValue(
			wxString::Format("%d", resy));
		m_z1_clip_text->SetValue("0");
		m_z2_clip_text->SetValue(
			wxString::Format("%d", resz));
	}
	else if (id == ID_Y1ClipSldr ||
		id == ID_Y2ClipSldr)
	{
		//m_link_x = false;
		//m_check_tb->ToggleTool(ID_LinkXChk,false);
		//m_check_tb->SetToolNormalBitmap(ID_LinkXChk,
		//	wxGetBitmapFromMemory(unlink));
		//m_link_z = false;
		//m_check_tb->ToggleTool(ID_LinkZChk,false);
		//m_check_tb->SetToolNormalBitmap(ID_LinkZChk,
		//	wxGetBitmapFromMemory(unlink));
		SetXLink(false);
		SetZLink(false);
		m_x1_clip_text->SetValue("0");
		m_x2_clip_text->SetValue(
			wxString::Format("%d", resx));
		m_z1_clip_text->SetValue("0");
		m_z2_clip_text->SetValue(
			wxString::Format("%d", resz));
	}
	else if (id == ID_Z1ClipSldr ||
		id == ID_Z2ClipSldr)
	{
		//m_link_x = false;
		//m_check_tb->ToggleTool(ID_LinkXChk,false);
		//m_check_tb->SetToolNormalBitmap(ID_LinkXChk,
		//	wxGetBitmapFromMemory(unlink));
		//m_link_y = false;
		//m_check_tb->ToggleTool(ID_LinkYChk,false);
		//m_check_tb->SetToolNormalBitmap(ID_LinkYChk,
		//	wxGetBitmapFromMemory(unlink));
		SetXLink(false);
		SetYLink(false);
		m_x1_clip_text->SetValue("0");
		m_x2_clip_text->SetValue(
			wxString::Format("%d", resx));
		m_y1_clip_text->SetValue("0");
		m_y2_clip_text->SetValue(
			wxString::Format("%d", resy));
	}
}

void ClippingView::OnYZClipBtn(wxCommandEvent& event)
{
	if (m_sel_type!=2 || !m_vd)
		return;

	int resx, resy, resz;
	m_vd->GetResolution(resx, resy, resz);

	wxString str = m_yz_dist_text->GetValue();
	long dist;
	str.ToLong(&dist);

	//reset yz
	//m_link_y = false;
	//m_check_tb->ToggleTool(ID_LinkYChk,false);
	//m_check_tb->SetToolNormalBitmap(ID_LinkYChk,
	//	wxGetBitmapFromMemory(unlink));
	//m_link_z = false;
	//m_check_tb->ToggleTool(ID_LinkZChk,false);
	//m_check_tb->SetToolNormalBitmap(ID_LinkZChk,
	//	wxGetBitmapFromMemory(unlink));
	SetYLink(false);
	SetZLink(false);
	m_y1_clip_text->SetValue("0");
	m_y2_clip_text->SetValue(
		wxString::Format("%d", resy));
	m_z1_clip_text->SetValue("0");
	m_z2_clip_text->SetValue(
		wxString::Format("%d", resz));

	//set x
	SetXLink(false);
	if (dist < resx)
	{
		int x1 = resx/2-dist/2;
		int x2 = x1 + dist;
		m_x1_clip_text->SetValue(
			wxString::Format("%d", x1));
		m_x2_clip_text->SetValue(
			wxString::Format("%d", x2));
		SetXLink(true);
		int distx, disty, distz;
		m_vd->GetClipDistance(distx, disty, distz);
		m_vd->SetClipDistance(dist, disty, distz);
	}
	else
	{
		m_x1_clip_text->SetValue("0");
		m_x2_clip_text->SetValue(
			wxString::Format("%d", resx));
	}

	m_x1_clip_sldr->SetFocus();
}

void ClippingView::OnXZClipBtn(wxCommandEvent& event)
{
	if (m_sel_type!=2 || !m_vd)
		return;

	int resx, resy, resz;
	m_vd->GetResolution(resx, resy, resz);

	wxString str = m_xz_dist_text->GetValue();
	long dist;
	str.ToLong(&dist);

	//reset xz
	//m_link_x = false;
	//m_check_tb->ToggleTool(ID_LinkXChk,false);
	//m_check_tb->SetToolNormalBitmap(ID_LinkXChk,
	//	wxGetBitmapFromMemory(unlink));
	//m_link_z = false;
	//m_check_tb->ToggleTool(ID_LinkZChk,false);
	//m_check_tb->SetToolNormalBitmap(ID_LinkZChk,
	//	wxGetBitmapFromMemory(unlink));
	SetXLink(false);
	SetZLink(false);
	m_x1_clip_text->SetValue("0");
	m_x2_clip_text->SetValue(
		wxString::Format("%d", resx));
	m_z1_clip_text->SetValue("0");
	m_z2_clip_text->SetValue(
		wxString::Format("%d", resz));

	//set y
	SetYLink(false);
	if (dist < resy)
	{
		int y1 = resy/2-dist/2;
		int y2 = y1 + dist;
		m_y1_clip_text->SetValue(
			wxString::Format("%d", y1));
		m_y2_clip_text->SetValue(
			wxString::Format("%d", y2));
		SetYLink(true);
		int distx, disty, distz;
		m_vd->GetClipDistance(distx, disty, distz);
		m_vd->SetClipDistance(distx, dist, distz);
	}
	else
	{
		m_y1_clip_text->SetValue("0");
		m_y2_clip_text->SetValue(
			wxString::Format("%d", resy));
	}

	m_y1_clip_sldr->SetFocus();
}

void ClippingView::OnXYClipBtn(wxCommandEvent& event)
{
	if (m_sel_type!=2 || !m_vd)
		return;

	int resx, resy, resz;
	m_vd->GetResolution(resx, resy, resz);

	wxString str = m_xy_dist_text->GetValue();
	long dist;
	str.ToLong(&dist);

	//reset xy
	//m_link_x = false;
	//m_check_tb->ToggleTool(ID_LinkXChk,false);
	//m_check_tb->SetToolNormalBitmap(ID_LinkXChk,
	//	wxGetBitmapFromMemory(unlink));
	//m_link_y = false;
	//m_check_tb->ToggleTool(ID_LinkYChk,false);
	//m_check_tb->SetToolNormalBitmap(ID_LinkYChk,
	//	wxGetBitmapFromMemory(unlink));
	SetXLink(false);
	SetYLink(false);
	m_x1_clip_text->SetValue("0");
	m_x2_clip_text->SetValue(
		wxString::Format("%d", resx));
	m_y1_clip_text->SetValue("0");
	m_y2_clip_text->SetValue(
		wxString::Format("%d", resy));

	//set z
	SetZLink(false);
	if (dist < resz)
	{
		int z1 = resz/2-dist/2;
		int z2 = z1 + dist;
		m_z1_clip_text->SetValue(
			wxString::Format("%d", z1));
		m_z2_clip_text->SetValue(
			wxString::Format("%d", z2));
		SetZLink(true);
		int distx, disty, distz;
		m_vd->GetClipDistance(distx, disty, distz);
		m_vd->SetClipDistance(distx, disty, dist);
	}
	else
	{
		m_z1_clip_text->SetValue("0");
		m_z2_clip_text->SetValue(
			wxString::Format("%d", resz));
	}

	m_z1_clip_sldr->SetFocus();
}

//move linked clipping planes
//dir: 0-lower; 1-higher
void ClippingView::MoveLinkedClippingPlanes(int dir)
{
	if (m_sel_type!=2 || !m_vd)
		return;

	int resx, resy, resz;
	m_vd->GetResolution(resx, resy, resz);

	wxString str;
	long dist;

/*	if (dir == 0)
	{
		//moving lower
		if (m_link_x)
		{
			str = m_yz_dist_text->GetValue();
			str.ToLong(&dist);
			dist = dist<2?dist+1:dist;
			str = m_x1_clip_text->GetValue();
			long x1;
			str.ToLong(&x1);
			x1 = x1 - dist + 1;
			x1 = x1<0?0:x1;
			m_x1_clip_text->SetValue(
				wxString::Format("%d", x1));
		}
		if (m_link_y)
		{
			str = m_xz_dist_text->GetValue();
			str.ToLong(&dist);
			dist = dist<2?dist+1:dist;
			str = m_y1_clip_text->GetValue();
			long y1;
			str.ToLong(&y1);
			y1 = y1 - dist + 1;
			y1 = y1<0?0:y1;
			m_y1_clip_text->SetValue(
				wxString::Format("%d", y1));
		}
		if (m_link_z)
		{
			str = m_xy_dist_text->GetValue();
			str.ToLong(&dist);
			dist = dist<2?dist+1:dist;
			str = m_z1_clip_text->GetValue();
			long z1;
			str.ToLong(&z1);
			z1 = z1 - dist + 1;
			z1 = z1<0?0:z1;
			m_z1_clip_text->SetValue(
				wxString::Format("%d", z1));
		}
	}
	else
	{
		//moving higher
		if (m_link_x)
		{
			str = m_yz_dist_text->GetValue();
			str.ToLong(&dist);
			dist = dist<2?dist+1:dist;
			str = m_x2_clip_text->GetValue();
			long x2;
			str.ToLong(&x2);
			x2 = x2 + dist - 1;
			x2 = x2>resx?resx:x2;
			m_x2_clip_text->SetValue(
				wxString::Format("%d", x2));
		}
		if (m_link_y)
		{
			str = m_xz_dist_text->GetValue();
			str.ToLong(&dist);
			dist = dist<2?dist+1:dist;
			str = m_y2_clip_text->GetValue();
			long y2;
			str.ToLong(&y2);
			y2 = y2 + dist - 1;
			y2 = y2>resy?resy:y2;
			m_y2_clip_text->SetValue(
				wxString::Format("%d", y2));
		}
		if (m_link_z)
		{
			str = m_xy_dist_text->GetValue();
			str.ToLong(&dist);
			dist = dist<2?dist+1:dist;
			str = m_z2_clip_text->GetValue();
			long z2;
			str.ToLong(&z2);
			z2 = z2 + dist - 1;
			z2 = z2>resz?resz:z2;
			m_z2_clip_text->SetValue(
				wxString::Format("%d", z2));
		}
	}*/
}

void ClippingView::OnSliderKeyDown(wxKeyEvent& event)
{
	if (m_sel_type!=2 || !m_vd)
		return;

	int resx, resy, resz;
	m_vd->GetResolution(resx, resy, resz);

	int id = event.GetId();
	int key = event.GetKeyCode();

	wxString str;
	long dist;

/*	if (key == wxKeyCode(','))
	{
		if (id == ID_X1ClipSldr ||
			id == ID_X2ClipSldr)
		{
			if (!m_link_x)
			{
				event.Skip();
				return;
			}

			str = m_yz_dist_text->GetValue();
			str.ToLong(&dist);
			dist = dist<2?dist+1:dist;
			str = m_x1_clip_text->GetValue();
			long x1;
			str.ToLong(&x1);
			x1 = x1 - dist + 1;
			x1 = x1<0?0:x1;
			m_x1_clip_text->SetValue(
				wxString::Format("%d", x1));
		}
		else if (id == ID_Y1ClipSldr ||
			id == ID_Y2ClipSldr)
		{
			if (!m_link_y)
			{
				event.Skip();
				return;
			}

			str = m_xz_dist_text->GetValue();
			str.ToLong(&dist);
			dist = dist<2?dist+1:dist;
			str = m_y1_clip_text->GetValue();
			long y1;
			str.ToLong(&y1);
			y1 = y1 - dist + 1;
			y1 = y1<0?0:y1;
			m_y1_clip_text->SetValue(
				wxString::Format("%d", y1));
		}
		else if (id == ID_Z1ClipSldr ||
			id == ID_Z2ClipSldr)
		{
			if (!m_link_z)
			{
				event.Skip();
				return;
			}

			str = m_xy_dist_text->GetValue();
			str.ToLong(&dist);
			dist = dist<2?dist+1:dist;
			str = m_z1_clip_text->GetValue();
			long z1;
			str.ToLong(&z1);
			z1 = z1 - dist + 1;
			z1 = z1<0?0:z1;
			m_z1_clip_text->SetValue(
				wxString::Format("%d", z1));
		}
	}
	else if (key == wxKeyCode('.'))
	{
		if (id == ID_X1ClipSldr ||
			id == ID_X2ClipSldr)
		{
			if (!m_link_x)
			{
				event.Skip();
				return;
			}

			str = m_yz_dist_text->GetValue();
			str.ToLong(&dist);
			dist = dist<2?dist+1:dist;
			str = m_x2_clip_text->GetValue();
			long x2;
			str.ToLong(&x2);
			x2 = x2 + dist - 1;
			x2 = x2>resx?resx:x2;
			m_x2_clip_text->SetValue(
				wxString::Format("%d", x2));
		}
		else if (id == ID_Y1ClipSldr ||
			id == ID_Y2ClipSldr)
		{
			if (!m_link_y)
			{
				event.Skip();
				return;
			}

			str = m_xz_dist_text->GetValue();
			str.ToLong(&dist);
			dist = dist<2?dist+1:dist;
			str = m_y2_clip_text->GetValue();
			long y2;
			str.ToLong(&y2);
			y2 = y2 + dist - 1;
			y2 = y2>resy?resy:y2;
			m_y2_clip_text->SetValue(
				wxString::Format("%d", y2));
		}
		else if (id == ID_Z1ClipSldr ||
			id == ID_Z2ClipSldr)
		{
			if (!m_link_z)
			{
				event.Skip();
				return;
			}

			str = m_xy_dist_text->GetValue();
			str.ToLong(&dist);
			dist = dist<2?dist+1:dist;
			str = m_z2_clip_text->GetValue();
			long z2;
			str.ToLong(&z2);
			z2 = z2 + dist - 1;
			z2 = z2>resz?resz:z2;
			m_z2_clip_text->SetValue(
				wxString::Format("%d", z2));
		}
	}*/

	event.Skip();
}

void ClippingView::EnableAll()
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
	//m_x2_clip_sldr->Enable();
	m_x2_clip_text->Enable();
	m_y1_clip_sldr->Enable();
	m_y1_clip_text->Enable();
	//m_y2_clip_sldr->Enable();
	m_y2_clip_text->Enable();
	m_z1_clip_sldr->Enable();
	m_z1_clip_text->Enable();
	//m_z2_clip_sldr->Enable();
	m_z2_clip_text->Enable();
	m_linkx_tb->Enable();
	m_linky_tb->Enable();
	m_linkz_tb->Enable();
	m_clip_reset_btn->Enable();
	m_yz_clip_btn->Enable();
	m_xz_clip_btn->Enable();
	m_xy_clip_btn->Enable();
	m_yz_dist_text->Enable();
	m_xz_dist_text->Enable();
	m_xy_dist_text->Enable();
}

void ClippingView::DisableAll()
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
	//m_x2_clip_sldr->Disable();
	m_x2_clip_text->Disable();
	m_y1_clip_sldr->Disable();
	m_y1_clip_text->Disable();
	//m_y2_clip_sldr->Disable();
	m_y2_clip_text->Disable();
	m_z1_clip_sldr->Disable();
	m_z1_clip_text->Disable();
	//m_z2_clip_sldr->Disable();
	m_z2_clip_text->Disable();
	m_linkx_tb->Disable();
	m_linky_tb->Disable();
	m_linkz_tb->Disable();
	m_clip_reset_btn->Disable();
	m_yz_clip_btn->Disable();
	m_xz_clip_btn->Disable();
	m_xy_clip_btn->Disable();
	m_yz_dist_text->Disable();
	m_xz_dist_text->Disable();
	m_xy_dist_text->Disable();
}
