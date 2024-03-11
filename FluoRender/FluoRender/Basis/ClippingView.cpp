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
#include <Global/Global.h>
#include <VRenderFrame.h>
#include <VRenderGLView.h>
#include <compatibility.h>
#include <wxDoubleSlider.h>
#include <wxSingleSlider.h>
#include <wx/valnum.h>
#include "png_resource.h"
#include "img/icons.h"
#include <Debug.h>

ClippingView::ClippingView(
	VRenderFrame* frame,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name) :
	PropPanel(frame, frame, pos, size, style, name),
m_view(0),
m_sel_type(0),
m_vd(0),
m_md(0),
m_draw_clip(false),
m_hold_planes(false),
m_chann_link(false),
m_plane_mode(kNormal),
m_enable_all(true)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);
	Freeze();
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
	m_toolbar->Bind(wxEVT_TOOL, &ClippingView::OnToolbar, this);
	m_toolbar->Realize();
	sizer_1->Add(5, 5, 0);
	sizer_1->Add(m_toolbar, 0, wxALIGN_CENTER, 0);

	//sliders for clipping planes
	bool inverse_slider = glbin_settings.m_inverse_slider;
	long ls = inverse_slider ? wxSL_VERTICAL : (wxSL_VERTICAL | wxSL_INVERSE);
	//x
	wxBoxSizer* sizer_cx = new wxBoxSizer(wxVERTICAL);
	m_clip_x_st = new wxButton(this, wxID_ANY, "X",
		wxDefaultPosition, FromDIP(wxSize(34, 20)));
	m_clipx_sldr = new wxDoubleSlider(this, wxID_ANY, 0, 512, 0, 512,
		wxDefaultPosition, wxDefaultSize, ls);
	m_clipx_sldr->SetRangeColor(wxColor(255, 128, 128));
	m_clipx_sldr->SetThumbColor(wxColor(255, 128, 128), wxColor(255, 128, 255));
	m_x1_clip_text = new wxTextCtrl(this, wxID_ANY, "0",
		wxDefaultPosition, FromDIP(wxSize(34, 20)), 0, vald_int);
	m_x2_clip_text = new wxTextCtrl(this, wxID_ANY, "512",
		wxDefaultPosition, FromDIP(wxSize(34, 20)), 0, vald_int);
	m_linkx_tb = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	bitmap = wxGetBitmapFromMemory(unlink);
	m_linkx_tb->AddCheckTool(0, "Lock X Planes",
		bitmap, wxNullBitmap, "Lock X Planes");
	m_clip_x_st->Bind(wxEVT_BUTTON, &ClippingView::OnClipXMF, this);
	m_clipx_sldr->Bind(wxEVT_SCROLL_CHANGED, &ClippingView::OnClipXChange, this);
	m_x1_clip_text->Bind(wxEVT_TEXT, &ClippingView::OnX1ClipEdit, this);
	m_x2_clip_text->Bind(wxEVT_TEXT, &ClippingView::OnX2ClipEdit, this);
	m_linkx_tb->Bind(wxEVT_TOOL, &ClippingView::OnLinkXCheck, this);
	//add the items
	sizer_cx->Add(5, 5, 0);
	sizer_cx->Add(m_clip_x_st, 0, wxALIGN_CENTER, 0);
	if (inverse_slider)
	{
		sizer_cx->Add(m_x1_clip_text, 0, wxALIGN_CENTER, 0);
		sizer_cx->Add(5, 5, 0);
		sizer_cx->Add(m_clipx_sldr, 1, wxEXPAND, 0);
		sizer_cx->Add(m_x2_clip_text, 0, wxALIGN_CENTER, 0);
	}
	else
	{
		sizer_cx->Add(m_x2_clip_text, 0, wxALIGN_CENTER, 0);
		sizer_cx->Add(5, 5, 0);
		sizer_cx->Add(m_clipx_sldr, 1, wxEXPAND, 0);
		sizer_cx->Add(m_x1_clip_text, 0, wxALIGN_CENTER, 0);
	}
	sizer_cx->Add(5, 5, 0);
	sizer_cx->Add(m_linkx_tb, 0, wxALIGN_CENTER, 0);
	m_linkx_tb->Realize();

	//y
	wxBoxSizer* sizer_cy = new wxBoxSizer(wxVERTICAL);
	//wxPanel * ypanel = new wxPanel(this);
	m_clip_y_st = new wxButton(this, wxID_ANY, "Y",
		wxDefaultPosition, FromDIP(wxSize(34, 20)));
	m_clipy_sldr = new wxDoubleSlider(this, wxID_ANY, 0, 512, 0, 512,
		wxDefaultPosition, wxDefaultSize, ls);
	m_clipy_sldr->SetRangeColor(wxColor(128, 255, 128));
	m_clipy_sldr->SetThumbColor(wxColor(128, 255, 128), wxColor(255, 255, 128));
	m_y1_clip_text = new wxTextCtrl(this, wxID_ANY, "0",
		wxDefaultPosition, FromDIP(wxSize(34, 20)), 0, vald_int);
	m_y2_clip_text = new wxTextCtrl(this, wxID_ANY, "512",
		wxDefaultPosition, FromDIP(wxSize(34, 20)), 0, vald_int);
	m_linky_tb = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	m_linky_tb->AddCheckTool(0, "Lock Y Planes",
		bitmap, wxNullBitmap, "Lock Y Planes");
	m_clip_y_st->Bind(wxEVT_BUTTON, &ClippingView::OnClipYMF, this);
	m_clipy_sldr->Bind(wxEVT_SCROLL_CHANGED, &ClippingView::OnClipYChange, this);
	m_y1_clip_text->Bind(wxEVT_TEXT, &ClippingView::OnY1ClipEdit, this);
	m_y2_clip_text->Bind(wxEVT_TEXT, &ClippingView::OnY2ClipEdit, this);
	m_linky_tb->Bind(wxEVT_TOOL, &ClippingView::OnLinkYCheck, this);
	//add the items
	sizer_cy->Add(5, 5, 0);
	sizer_cy->Add(m_clip_y_st, 0, wxALIGN_CENTER, 0);
	if (inverse_slider)
	{
		sizer_cy->Add(m_y1_clip_text, 0, wxALIGN_CENTER, 0);
		sizer_cy->Add(5, 5, 0);
		sizer_cy->Add(m_clipy_sldr, 1, wxEXPAND, 0);
		sizer_cy->Add(m_y2_clip_text, 0, wxALIGN_CENTER, 0);
	}
	else
	{
		sizer_cy->Add(m_y2_clip_text, 0, wxALIGN_CENTER, 0);
		sizer_cy->Add(5, 5, 0);
		sizer_cy->Add(m_clipy_sldr, 1, wxEXPAND, 0);
		sizer_cy->Add(m_y1_clip_text, 0, wxALIGN_CENTER, 0);
	}
	sizer_cy->Add(5, 5, 0);
	sizer_cy->Add(m_linky_tb, 0, wxALIGN_CENTER, 0);
	m_linky_tb->Realize();

	//z
	wxBoxSizer* sizer_cz = new wxBoxSizer(wxVERTICAL);
	//wxPanel * zpanel = new wxPanel(this);
	m_clip_z_st = new wxButton(this, wxID_ANY, "Z",
		wxDefaultPosition, FromDIP(wxSize(34, 20)));
	m_clipz_sldr = new wxDoubleSlider(this, wxID_ANY, 0, 512, 0, 512,
		wxPoint(0,0), wxDefaultSize, ls);
	m_clipz_sldr->SetRangeColor(wxColor(128, 128, 255));
	m_clipz_sldr->SetThumbColor(wxColor(128, 128, 255), wxColor(128, 255, 255));
	m_z1_clip_text = new wxTextCtrl(this, wxID_ANY, "0",
		wxDefaultPosition, FromDIP(wxSize(34, 20)), 0, vald_int);
	m_z2_clip_text = new wxTextCtrl(this, wxID_ANY, "512",
		wxDefaultPosition, FromDIP(wxSize(34, 20)), 0, vald_int);
	m_linkz_tb = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	m_linkz_tb->AddCheckTool(0, "Lock Z Planes",
		bitmap, wxNullBitmap, "Lock Z Planes");
	m_clip_z_st->Bind(wxEVT_BUTTON, &ClippingView::OnClipZMF, this);
	m_clipz_sldr->Bind(wxEVT_SCROLL_CHANGED, &ClippingView::OnClipZChange, this);
	m_z1_clip_text->Bind(wxEVT_TEXT, &ClippingView::OnZ1ClipEdit, this);
	m_z2_clip_text->Bind(wxEVT_TEXT, &ClippingView::OnZ2ClipEdit, this);
	m_linkz_tb->Bind(wxEVT_TOOL, &ClippingView::OnLinkZCheck, this);
	//add the items
	sizer_cz->Add(5, 5, 0);
	sizer_cz->Add(m_clip_z_st, 0, wxALIGN_CENTER, 0);
	if (inverse_slider)
	{
		sizer_cz->Add(m_z1_clip_text, 0, wxALIGN_CENTER, 0);
		sizer_cz->Add(5, 5, 0);
		sizer_cz->Add(m_clipz_sldr, 1, wxEXPAND, 0);
		sizer_cz->Add(m_z2_clip_text, 0, wxALIGN_CENTER, 0);
	}
	else
	{
		sizer_cz->Add(m_z2_clip_text, 0, wxALIGN_CENTER, 0);
		sizer_cz->Add(5, 5, 0);
		sizer_cz->Add(m_clipz_sldr, 1, wxEXPAND, 0);
		sizer_cz->Add(m_z1_clip_text, 0, wxALIGN_CENTER, 0);
	}
	sizer_cz->Add(5, 5, 0);
	sizer_cz->Add(m_linkz_tb, 0, wxALIGN_CENTER, 0);
	m_linkz_tb->Realize();

	//link sliders
	m_clipx_sldr->Bind(wxEVT_RIGHT_DOWN, &ClippingView::OnClipXRClick, this);
	m_clipy_sldr->Bind(wxEVT_RIGHT_DOWN, &ClippingView::OnClipYRClick, this);
	m_clipz_sldr->Bind(wxEVT_RIGHT_DOWN, &ClippingView::OnClipZRClick, this);

	//2
	wxBoxSizer *sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	sizer_2->Add(sizer_cx, 1, wxEXPAND);
	sizer_2->Add(sizer_cy, 1, wxEXPAND);
	sizer_2->Add(sizer_cz, 1, wxEXPAND);

	//clip buttons 4
	wxBoxSizer* sizer_4 = new wxBoxSizer(wxHORIZONTAL);
	m_yz_clip_btn = new wxButton(this, wxID_ANY, "YZ",
		wxDefaultPosition, FromDIP(wxSize(34, 22)));
	m_xz_clip_btn = new wxButton(this, wxID_ANY, "XZ",
		wxDefaultPosition, FromDIP(wxSize(34, 22)));
	m_xy_clip_btn = new wxButton(this, wxID_ANY, "XY",
		wxDefaultPosition, FromDIP(wxSize(34, 22)));
	m_yz_clip_btn->Bind(wxEVT_BUTTON, &ClippingView::OnYZClipBtn, this);
	m_xz_clip_btn->Bind(wxEVT_BUTTON, &ClippingView::OnXZClipBtn, this);
	m_xy_clip_btn->Bind(wxEVT_BUTTON, &ClippingView::OnXYClipBtn, this);
	sizer_4->Add(m_yz_clip_btn, 1, wxEXPAND);
	sizer_4->AddSpacer(5);
	sizer_4->Add(m_xz_clip_btn, 1, wxEXPAND);
	sizer_4->AddSpacer(5);
	sizer_4->Add(m_xy_clip_btn, 1, wxEXPAND);

	//clip distance 5
	wxBoxSizer* sizer_5 = new wxBoxSizer(wxHORIZONTAL);
	m_yz_dist_text = new wxTextCtrl(this, wxID_ANY, "1",
		wxDefaultPosition, FromDIP(wxSize(34, 22)), 0, vald_int);
	m_xz_dist_text = new wxTextCtrl(this, wxID_ANY, "1",
		wxDefaultPosition, FromDIP(wxSize(34, 22)), 0, vald_int);
	m_xy_dist_text = new wxTextCtrl(this, wxID_ANY, "1",
		wxDefaultPosition, FromDIP(wxSize(34, 22)), 0, vald_int);
	m_yz_dist_text->Bind(wxEVT_TEXT, &ClippingView::OnClipDistXEdit, this);
	m_xz_dist_text->Bind(wxEVT_TEXT, &ClippingView::OnClipDistYEdit, this);
	m_xy_dist_text->Bind(wxEVT_TEXT, &ClippingView::OnClipDistZEdit, this);
	sizer_5->Add(m_yz_dist_text, 1, wxEXPAND);
	sizer_5->AddSpacer(5);
	sizer_5->Add(m_xz_dist_text, 1, wxEXPAND);
	sizer_5->AddSpacer(5);
	sizer_5->Add(m_xy_dist_text, 1, wxEXPAND);
	
	//reset clipping 6
	wxBoxSizer* sizer_6 = new wxBoxSizer(wxHORIZONTAL);
#ifndef _DARWIN
	m_clip_reset_btn = new wxButton(this, wxID_ANY, "Reset Clips",
									wxDefaultPosition, FromDIP(wxSize(120, 22)));
#else
	m_clip_reset_btn = new wxButton(this, wxID_ANY, "Reset Clips",
									wxDefaultPosition, FromDIP(wxSize(125, 30)));
#endif
	m_clip_reset_btn->SetBitmap(wxGetBitmapFromMemory(reset));
	m_clip_reset_btn->Bind(wxEVT_BUTTON, &ClippingView::OnClipResetBtn, this);
	sizer_6->Add(5, 5, 0);
	sizer_6->Add(m_clip_reset_btn, 0, wxALIGN_CENTER);

	//rotations
	//set sero rotation for clipping planes 7
	wxBoxSizer* sizer_7 = new wxBoxSizer(wxHORIZONTAL);
#ifndef _DARWIN
	m_set_zero_btn = new wxButton(this, wxID_ANY, "Align to View",
								  wxDefaultPosition, FromDIP(wxSize(120, 22)));
#else
	m_set_zero_btn = new wxButton(this, wxID_ANY, "Align to View",
								  wxDefaultPosition, FromDIP(wxSize(125, 30)));
#endif
	m_set_zero_btn->SetBitmap(wxGetBitmapFromMemory(align));
	m_set_zero_btn->Bind(wxEVT_BUTTON, &ClippingView::OnSetZeroBtn, this);
	sizer_7->Add(5, 5, 0);
	sizer_7->Add(m_set_zero_btn, 0, wxALIGN_CENTER);

	//reset rotations 8
	wxBoxSizer* sizer_8 = new wxBoxSizer(wxHORIZONTAL);
#ifndef _DARWIN
	m_rot_reset_btn = new wxButton(this, wxID_ANY, "Reset to 0",
								   wxDefaultPosition, FromDIP(wxSize(120, 22)));
#else
	m_rot_reset_btn = new wxButton(this, wxID_ANY, "Reset to 0",
								   wxDefaultPosition, FromDIP(wxSize(125, 30)));
#endif
	m_rot_reset_btn->SetBitmap(wxGetBitmapFromMemory(reset));
	m_rot_reset_btn->Bind(wxEVT_BUTTON, &ClippingView::OnRotResetBtn, this);
	sizer_8->Add(5, 5, 0);
	sizer_8->Add(m_rot_reset_btn, 0, wxALIGN_CENTER);

	ls = inverse_slider ? wxSL_VERTICAL : (wxSL_VERTICAL | wxSL_INVERSE);
	//sliders for rotating clipping planes 
	//x
	wxBoxSizer* sizer_rx = new wxBoxSizer(wxVERTICAL);
	m_rot_x_st = new wxButton(this, wxID_ANY, "X",
		wxDefaultPosition, FromDIP(wxSize(34, 20)));
	m_x_rot_sldr = new wxSingleSlider(this, wxID_ANY, 0, -180, 180,
		wxDefaultPosition, wxDefaultSize, ls);
	m_x_rot_sldr->SetRangeStyle(2);
	m_x_rot_text = new wxTextCtrl(this, wxID_ANY, "0.0",
		wxDefaultPosition, FromDIP(wxSize(34, 20)), 0, vald_fp1);
	m_x_rot_spin = new wxSpinButton(this, wxID_ANY,
		wxDefaultPosition, FromDIP(wxSize(30, 20)), wxSP_VERTICAL);
	m_x_rot_spin->SetRange(-0x8000, 0x7fff);
	m_rot_x_st->Bind(wxEVT_BUTTON, &ClippingView::OnRotXMF, this);
	m_x_rot_sldr->Bind(wxEVT_SCROLL_CHANGED, &ClippingView::OnXRotChange, this);
	m_x_rot_text->Bind(wxEVT_TEXT, &ClippingView::OnXRotEdit, this);
	m_x_rot_spin->Bind(wxEVT_SPIN_UP, &ClippingView::OnXRotSpinUp, this);
	m_x_rot_spin->Bind(wxEVT_SPIN_DOWN, &ClippingView::OnXRotSpinDown, this);
	sizer_rx->Add(5, 5, 0);
	sizer_rx->Add(m_rot_x_st, 0, wxALIGN_CENTER, 0);
	sizer_rx->Add(m_x_rot_text, 0, wxALIGN_CENTER, 0);
	sizer_rx->Add(5, 5, 0);
	sizer_rx->Add(m_x_rot_spin, 0, wxALIGN_CENTER, 0);
	sizer_rx->Add(m_x_rot_sldr, 1, wxEXPAND, 0);
	//y
	wxBoxSizer* sizer_ry = new wxBoxSizer(wxVERTICAL);
	m_rot_y_st = new wxButton(this, wxID_ANY, "Y",
		wxDefaultPosition, FromDIP(wxSize(34, 20)));
	m_y_rot_sldr = new wxSingleSlider(this, wxID_ANY, 0, -180, 180,
		wxDefaultPosition, wxDefaultSize, ls);
	m_y_rot_sldr->SetRangeStyle(2);
	m_y_rot_text = new wxTextCtrl(this, wxID_ANY, "0.0",
		wxDefaultPosition, FromDIP(wxSize(34, 20)), 0, vald_fp1);
	m_y_rot_spin = new wxSpinButton(this, wxID_ANY,
		wxDefaultPosition, FromDIP(wxSize(30, 20)), wxSP_VERTICAL);
	m_y_rot_spin->SetRange(-0x8000, 0x7fff);
	m_rot_y_st->Bind(wxEVT_BUTTON, &ClippingView::OnRotXMF, this);
	m_y_rot_sldr->Bind(wxEVT_SCROLL_CHANGED, &ClippingView::OnYRotChange, this);
	m_y_rot_text->Bind(wxEVT_TEXT, &ClippingView::OnYRotEdit, this);
	m_y_rot_spin->Bind(wxEVT_SPIN_UP, &ClippingView::OnYRotSpinUp, this);
	m_y_rot_spin->Bind(wxEVT_SPIN_DOWN, &ClippingView::OnYRotSpinDown, this);
	sizer_ry->Add(5, 5, 0);
	sizer_ry->Add(m_rot_y_st, 0, wxALIGN_CENTER, 0);
	sizer_ry->Add(m_y_rot_text, 0, wxALIGN_CENTER, 0);
	sizer_ry->Add(5, 5, 0);
	sizer_ry->Add(m_y_rot_spin, 0, wxALIGN_CENTER, 0);
	sizer_ry->Add(m_y_rot_sldr, 1, wxEXPAND, 0);
	//z
	wxBoxSizer* sizer_rz = new wxBoxSizer(wxVERTICAL);
	m_rot_z_st = new wxButton(this, wxID_ANY, "Z",
		wxDefaultPosition, FromDIP(wxSize(34, 20)));
	m_z_rot_sldr = new wxSingleSlider(this, wxID_ANY, 0, -180, 180,
		wxDefaultPosition, wxDefaultSize, ls);
	m_z_rot_sldr->SetRangeStyle(2);
	m_z_rot_text = new wxTextCtrl(this, wxID_ANY, "0.0",
		wxDefaultPosition, FromDIP(wxSize(34, 20)), 0, vald_fp1);
	m_z_rot_spin = new wxSpinButton(this, wxID_ANY,
		wxDefaultPosition, FromDIP(wxSize(30, 20)), wxSP_VERTICAL);
	m_z_rot_spin->SetRange(-0x8000, 0x7fff);
	m_rot_z_st->Bind(wxEVT_BUTTON, &ClippingView::OnRotZMF, this);
	m_z_rot_sldr->Bind(wxEVT_SCROLL_CHANGED, &ClippingView::OnZRotChange, this);
	m_z_rot_text->Bind(wxEVT_TEXT, &ClippingView::OnZRotEdit, this);
	m_z_rot_spin->Bind(wxEVT_SPIN_UP, &ClippingView::OnZRotSpinUp, this);
	m_z_rot_spin->Bind(wxEVT_SPIN_DOWN, &ClippingView::OnZRotSpinDown, this);
	sizer_rz->Add(5, 5, 0);
	sizer_rz->Add(m_rot_z_st, 0, wxALIGN_CENTER, 0);
	sizer_rz->Add(m_z_rot_text, 0, wxALIGN_CENTER, 0);
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
	SetAutoLayout(true);
	SetScrollRate(10, 10);

	Bind(wxEVT_IDLE, &ClippingView::OnIdle, this);

	EnableAll(false);

	//add controls
	glbin.add_undo_control(m_clipx_sldr);
	glbin.add_undo_control(m_clipy_sldr);
	glbin.add_undo_control(m_clipz_sldr);
	glbin.add_undo_control(m_x_rot_sldr);
	glbin.add_undo_control(m_y_rot_sldr);
	glbin.add_undo_control(m_z_rot_sldr);

	Thaw();
}

ClippingView::~ClippingView()
{
	//delete controls
	glbin.del_undo_control(m_clipx_sldr);
	glbin.del_undo_control(m_clipy_sldr);
	glbin.del_undo_control(m_clipz_sldr);
	glbin.del_undo_control(m_x_rot_sldr);
	glbin.del_undo_control(m_y_rot_sldr);
	glbin.del_undo_control(m_z_rot_sldr);

	SetFocusVRenderViews(0);
}

void ClippingView::FluoUpdate(const fluo::ValueCollection& vc)
{
	if (!m_view ||
		(!m_vd && !m_md))
	{
		EnableAll(false);
		return;
	}
	if (FOUND_VALUE(gstNull))
		return;

	bool update_all = vc.empty();

	//modes
	if (update_all || FOUND_VALUE(gstPlaneMode))
	{
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
		fc = m_md->GetColor();
		break;
	}
	wxColor c(fc.r() * 255, fc.g() * 255, fc.b() * 255);

	wxIntegerValidator<int>* vald_i;

	if (update_all || FOUND_VALUE(gstClipRange))
	{
		//slider range
		m_clipx_sldr->SetRange(resx_n, resx);
		m_clipx_sldr->SetRangeColor(c);
		m_clipy_sldr->SetRange(resy_n, resy);
		m_clipy_sldr->SetRangeColor(c);
		m_clipz_sldr->SetRange(resz_n, resz);
		m_clipz_sldr->SetRangeColor(c);
		//text range
		if ((vald_i = (wxIntegerValidator<int>*)m_x1_clip_text->GetValidator()))
			vald_i->SetRange(0, resx);
		if ((vald_i = (wxIntegerValidator<int>*)m_x2_clip_text->GetValidator()))
			vald_i->SetRange(0, resx);
		if ((vald_i = (wxIntegerValidator<int>*)m_y1_clip_text->GetValidator()))
			vald_i->SetRange(0, resy);
		if ((vald_i = (wxIntegerValidator<int>*)m_y2_clip_text->GetValidator()))
			vald_i->SetRange(0, resy);
		if ((vald_i = (wxIntegerValidator<int>*)m_z1_clip_text->GetValidator()))
			vald_i->SetRange(0, resz);
		if ((vald_i = (wxIntegerValidator<int>*)m_z2_clip_text->GetValidator()))
			vald_i->SetRange(0, resz);
	}

	//clip distance
	if (update_all || FOUND_VALUE(gstClipDistance))
	{
		switch (m_sel_type)
		{
		case 2:	//volume
		{
			m_yz_dist_text->ChangeValue(
				wxString::Format("%d", m_vd->GetClipDistX()));
			m_xz_dist_text->ChangeValue(
				wxString::Format("%d", m_vd->GetClipDistY()));
			m_xy_dist_text->ChangeValue(
				wxString::Format("%d", m_vd->GetClipDistZ()));
		}
		break;
		case 3:	//mesh
			break;
		}
	}

	vector<fluo::Plane*>* planes = 0;
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
	if (planes->size() != 6)	//it has to be 6
		return;

	bool bval;
	wxString str;
	fluo::Plane* plane = 0;
	int val = 0;
	double abcd[4];

	//x1
	if (update_all || FOUND_VALUE(gstClipX1))
	{
		plane = (*planes)[0];
		plane->get_copy(abcd);
		val = std::round(fabs(abcd[3] * resx));
		m_clipx_sldr->ChangeLowValue(val);
		str = wxString::Format("%d", val);
		m_x1_clip_text->ChangeValue(str);
	}
	//x2
	if (update_all || FOUND_VALUE(gstClipX2))
	{
		plane = (*planes)[1];
		plane->get_copy(abcd);
		val = std::round(fabs(abcd[3] * resx));
		m_clipx_sldr->ChangeHighValue(val);
		str = wxString::Format("%d", val);
		m_x2_clip_text->ChangeValue(str);
	}
	//y1
	if (update_all || FOUND_VALUE(gstClipY1))
	{
		plane = (*planes)[2];
		plane->get_copy(abcd);
		val = std::round(fabs(abcd[3] * resy));
		m_clipy_sldr->ChangeLowValue(val);
		str = wxString::Format("%d", val);
		m_y1_clip_text->ChangeValue(str);
	}
	//y2
	if (update_all || FOUND_VALUE(gstClipY2))
	{
		plane = (*planes)[3];
		plane->get_copy(abcd);
		val = std::round(fabs(abcd[3] * resy));
		m_clipy_sldr->ChangeHighValue(val);
		str = wxString::Format("%d", val);
		m_y2_clip_text->ChangeValue(str);
	}
	//z1
	if (update_all || FOUND_VALUE(gstClipZ1))
	{
		plane = (*planes)[4];
		plane->get_copy(abcd);
		val = std::round(fabs(abcd[3] * resz));
		m_clipz_sldr->ChangeLowValue(val);
		str = wxString::Format("%d", val);
		m_z1_clip_text->ChangeValue(str);
	}
	//z2
	if (update_all || FOUND_VALUE(gstClipZ2))
	{
		plane = (*planes)[5];
		plane->get_copy(abcd);
		val = std::round(fabs(abcd[3] * resz));
		m_clipz_sldr->ChangeHighValue(val);
		str = wxString::Format("%d", val);
		m_z2_clip_text->ChangeValue(str);
	}

	//link
	if (update_all || FOUND_VALUE(gstClipLinkX))
	{
		bval = m_clipx_sldr->GetLink();
		if (bval != m_linkx_tb->GetToolState(0))
		{
			m_linkx_tb->ToggleTool(0, bval);
			if (bval)
				m_linkx_tb->SetToolNormalBitmap(0,
					wxGetBitmapFromMemory(link));
			else
				m_linkx_tb->SetToolNormalBitmap(0,
					wxGetBitmapFromMemory(unlink));
		}
	}
	if (update_all || FOUND_VALUE(gstClipLinkY))
	{
		bval = m_clipy_sldr->GetLink();
		if (bval != m_linky_tb->GetToolState(0))
		{
			m_linky_tb->ToggleTool(0, bval);
			if (bval)
				m_linky_tb->SetToolNormalBitmap(0,
					wxGetBitmapFromMemory(link));
			else
				m_linky_tb->SetToolNormalBitmap(0,
					wxGetBitmapFromMemory(unlink));
		}
	}
	if (update_all || FOUND_VALUE(gstClipLinkZ))
	{
		bval = m_clipz_sldr->GetLink();
		if (bval != m_linkz_tb->GetToolState(0))
		{
			m_linkz_tb->ToggleTool(0, bval);
			if (bval)
				m_linkz_tb->SetToolNormalBitmap(0,
					wxGetBitmapFromMemory(link));
			else
				m_linkz_tb->SetToolNormalBitmap(0,
					wxGetBitmapFromMemory(unlink));
		}
	}

	//rotations
	double rotx, roty, rotz;
	m_view->GetClippingPlaneRotations(rotx, roty, rotz);
	//x
	if (update_all || FOUND_VALUE(gstClipRotX))
	{
		m_x_rot_text->ChangeValue(wxString::Format("%.1f", double(rotx)));
		m_x_rot_sldr->ChangeValue(std::round(rotx));
	}
	//y
	if (update_all || FOUND_VALUE(gstClipRotY))
	{
		m_y_rot_text->ChangeValue(wxString::Format("%.1f", double(roty)));
		m_y_rot_sldr->ChangeValue(std::round(roty));
	}
	//z
	if (update_all || FOUND_VALUE(gstClipRotZ))
	{
		m_z_rot_text->ChangeValue(wxString::Format("%.1f", double(rotz)));
		m_z_rot_sldr->ChangeValue(std::round(rotz));
	}

	EnableAll(true);

}

void ClippingView::SetChannLink(bool chann)
{
	m_chann_link = chann;
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
	FluoRefresh(false, true, 2);
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

//set view
void ClippingView::SetRenderView(VRenderGLView* view)
{
	m_view = view;
	FluoUpdate();
}

void ClippingView::SetVolumeData(VolumeData* vd)
{
	if (!vd) return;

	if (m_vd != vd)
		ClearUndo();

	m_vd = vd;
	m_sel_type = 2;
}

void ClippingView::SetMeshData(MeshData* md)
{
	if (!md) return;
	m_md = md;
	m_sel_type = 3;
}

void ClippingView::OnToolbar(wxCommandEvent& event)
{
	int id = event.GetId();
	switch (id)
	{
	case ID_LinkChannelsBtn:
		LinkChannels();
		break;
	case ID_HoldPlanesBtn:
		HoldPlanes();
		break;
	case ID_PlaneModesBtn:
		SetPlaneMode();
		break;
	}
}

void ClippingView::LinkChannels()
{
	m_chann_link = m_toolbar->GetToolState(ID_LinkChannelsBtn);
	if (m_chann_link)
	{
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

		int val[6] = { x1_val, x2_val, y1_val, y2_val, z1_val, z2_val };
		SetClipValues(val);
	}
}

void ClippingView::HoldPlanes()
{
	m_hold_planes = m_toolbar->GetToolState(ID_HoldPlanesBtn);
}

void ClippingView::SetPlaneMode()
{
	switch (m_plane_mode)
	{
	case kNormal:
		m_plane_mode = kFrame6;
		break;
	case kFrame6:
		m_plane_mode = kFrame3;
		break;
	case kFrame3:
		m_plane_mode = kLowTrans;
		break;
	case kLowTrans:
		m_plane_mode = kLowTransBack;
		break;
	case kLowTransBack:
		m_plane_mode = kNormalBack;
		break;
	case kNormalBack:
		m_plane_mode = kNone;
		break;
	case kNone:
		m_plane_mode = kNormal;
		break;
	}
	SetPlaneMode(m_plane_mode);
}

void ClippingView::OnClipResetBtn(wxCommandEvent &event)
{
	ResetClipValues();
}

void ClippingView::SyncClipValue(int i)
{
	int val1, val2;
	switch (i)
	{
	case 0:
		val1 = m_clipx_sldr->GetLowValue();
		val2 = m_clipx_sldr->GetHighValue();
		SetClipValues(3, val1, val2);
		break;
	case 1:
		val1 = m_clipy_sldr->GetLowValue();
		val2 = m_clipy_sldr->GetHighValue();
		SetClipValues(12, val1, val2);
		break;
	case 2:
		val1 = m_clipz_sldr->GetLowValue();
		val2 = m_clipz_sldr->GetHighValue();
		SetClipValues(48, val1, val2);
		break;
	}
}

void ClippingView::OnClipXMF(wxCommandEvent& event)
{
	switch (glbin_settings.m_mulfunc)
	{
	case 0:
		SyncClipValue(0);
		break;
	case 1:
		SetFocusVRenderViews(m_clipx_sldr);
		break;
	case 2:
		ResetClipValuesX();
		break;
	case 3:
		break;
	case 4:
		m_clipx_sldr->Undo();
		break;
	case 5:
		break;
	}
}

void ClippingView::OnClipYMF(wxCommandEvent& event)
{
	switch (glbin_settings.m_mulfunc)
	{
	case 0:
		SyncClipValue(1);
		break;
	case 1:
		SetFocusVRenderViews(m_clipy_sldr);
		break;
	case 2:
		ResetClipValuesY();
		break;
	case 3:
		break;
	case 4:
		m_clipy_sldr->Undo();
		break;
	case 5:
		break;
	}
}

void ClippingView::OnClipZMF(wxCommandEvent& event)
{
	switch (glbin_settings.m_mulfunc)
	{
	case 0:
		SyncClipValue(2);
		break;
	case 1:
		SetFocusVRenderViews(m_clipz_sldr);
		break;
	case 2:
		ResetClipValuesZ();
		break;
	case 3:
		break;
	case 4:
		m_clipz_sldr->Undo();
		break;
	case 5:
		break;
	}
}

void ClippingView::OnClipXChange(wxScrollEvent &event)
{
	int ival1 = m_clipx_sldr->GetLowValue();
	int ival2 = m_clipx_sldr->GetHighValue();
	wxString str = event.GetString();
	int i = 0;
	if (str.Find("low") != wxNOT_FOUND)
		i = 1;
	else if (str.Find("high") != wxNOT_FOUND)
		i = 2;
	else
		i = 3;
	SetClipValues(i, ival1, ival2);
}

void ClippingView::OnX1ClipEdit(wxCommandEvent &event)
{
	wxString str = m_x1_clip_text->GetValue();
	long ival = 0;
	str.ToLong(&ival);
	bool link = m_clipx_sldr->GetLink();
	SetClipValue(0, ival, link);
}

void ClippingView::OnX2ClipEdit(wxCommandEvent &event)
{
	wxString str = m_x2_clip_text->GetValue();
	long ival = 0;
	str.ToLong(&ival);
	bool link = m_clipx_sldr->GetLink();
	SetClipValue(1, ival, link);
}

void ClippingView::OnClipYChange(wxScrollEvent &event)
{
	int ival1 = m_clipy_sldr->GetLowValue();
	int ival2 = m_clipy_sldr->GetHighValue();
	wxString str = event.GetString();
	int i = 0;
	if (str.Find("low") != wxNOT_FOUND)
		i = 4;
	else if (str.Find("high") != wxNOT_FOUND)
		i = 8;
	else
		i = 12;
	SetClipValues(i, ival1, ival2);
}

void ClippingView::OnY1ClipEdit(wxCommandEvent &event)
{
	wxString str = m_y1_clip_text->GetValue();
	long ival = 0;
	str.ToLong(&ival);
	bool link = m_clipy_sldr->GetLink();
	SetClipValue(2, ival, link);
}

void ClippingView::OnY2ClipEdit(wxCommandEvent &event)
{
	wxString str = m_y2_clip_text->GetValue();
	long ival = 0;
	str.ToLong(&ival);
	bool link = m_clipy_sldr->GetLink();
	SetClipValue(3, ival, link);
}

void ClippingView::OnClipZChange(wxScrollEvent &event)
{
	int ival1 = m_clipz_sldr->GetLowValue();
	int ival2 = m_clipz_sldr->GetHighValue();
	wxString str = event.GetString();
	int i = 0;
	if (str.Find("low") != wxNOT_FOUND)
		i = 16;
	else if (str.Find("high") != wxNOT_FOUND)
		i = 32;
	else
		i = 48;
	SetClipValues(i, ival1, ival2);
}

void ClippingView::OnZ1ClipEdit(wxCommandEvent &event)
{
	wxString str = m_z1_clip_text->GetValue();
	long ival = 0;
	str.ToLong(&ival);
	bool link = m_clipz_sldr->GetLink();
	SetClipValue(4, ival, link);
}

void ClippingView::OnZ2ClipEdit(wxCommandEvent &event)
{
	wxString str = m_z2_clip_text->GetValue();
	long ival = 0;
	str.ToLong(&ival);
	bool link = m_clipz_sldr->GetLink();
	SetClipValue(5, ival, link);
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
	return m_clipx_sldr->GetLink();
}
bool ClippingView::GetYLink()
{
	return m_clipy_sldr->GetLink();
}
bool ClippingView::GetZLink()
{
	return m_clipz_sldr->GetLink();
}

void ClippingView::SetXLink(bool val)
{
	m_clipx_sldr->SetLink(val);
	m_linkx_tb->ToggleTool(0, val);
	if (val)
		m_linkx_tb->SetToolNormalBitmap(0,
			wxGetBitmapFromMemory(link));
	else
		m_linkx_tb->SetToolNormalBitmap(0,
			wxGetBitmapFromMemory(unlink));
}

void ClippingView::SetYLink(bool val)
{
	m_clipy_sldr->SetLink(val);
	m_linky_tb->ToggleTool(0, val);
	if (val)
		m_linky_tb->SetToolNormalBitmap(0,
			wxGetBitmapFromMemory(link));
	else
		m_linky_tb->SetToolNormalBitmap(0,
			wxGetBitmapFromMemory(unlink));
}

void ClippingView::SetZLink(bool val)
{
	m_clipz_sldr->SetLink(val);
	m_linkz_tb->ToggleTool(0, val);
	if (val)
		m_linkz_tb->SetToolNormalBitmap(0,
			wxGetBitmapFromMemory(link));
	else
		m_linkz_tb->SetToolNormalBitmap(0,
			wxGetBitmapFromMemory(unlink));
}

void ClippingView::SetClipValue(int i, int val, bool link)
{
	if (!m_vd || !m_view)
		return;
	if (m_chann_link)
		m_view->SetClipValue(i, val);
	else
		m_vd->SetClipValue(i, val);

	switch (i)
	{
	case 0:
		m_view->m_clip_mask = link ? 3 : 1;
		break;
	case 1:
		m_view->m_clip_mask = link ? 3 : 2;
		break;
	case 2:
		m_view->m_clip_mask = link ? 12 : 4;
		break;
	case 3:
		m_view->m_clip_mask = link ? 12 : 8;
		break;
	case 4:
		m_view->m_clip_mask = link ? 48 : 16;
		break;
	case 5:
		m_view->m_clip_mask = link ? 48 : 32;
		break;
	}
	m_view->UpdateClips();

	FluoRefresh(false, true, 2);
}

void ClippingView::SetClipValues(int i, int val1, int val2)
{
	if (!m_vd || !m_view)
		return;
	if (m_chann_link)
		m_view->SetClipValues(i, val1, val2);
	else
		m_vd->SetClipValues(i, val1, val2);

	m_view->m_clip_mask = i;
	m_view->UpdateClips();

	FluoRefresh(false, true, 2);
}

void ClippingView::SetClipValues(const int val[6])
{
	if (!m_vd || !m_view)
		return;
	if (m_chann_link)
		m_view->SetClipValues(val);
	else
		m_vd->SetClipValues(val);
	m_view->m_clip_mask = 63;
	m_view->UpdateClips();

	FluoRefresh(false, true, 2);
}

void ClippingView::ResetClipValues()
{
	if (!m_vd || !m_view)
		return;
	if (m_chann_link)
		m_view->ResetClipValues();
	else
		m_vd->ResetClipValues();
	m_view->m_clip_mask = -1;
	m_view->UpdateClips();

	//links
	SetXLink(false);
	SetYLink(false);
	SetZLink(false);

	FluoRefresh(false, true, 2);
}

void ClippingView::ResetClipValuesX()
{
	if (!m_vd || !m_view)
		return;
	if (m_chann_link)
		m_view->ResetClipValuesX();
	else
		m_vd->ResetClipValuesX();
	m_view->m_clip_mask = -1;
	m_view->UpdateClips();

	//links
	SetXLink(false);

	FluoRefresh(false, true, 2);
}

void ClippingView::ResetClipValuesY()
{
	if (!m_vd || !m_view)
		return;
	if (m_chann_link)
		m_view->ResetClipValuesY();
	else
		m_vd->ResetClipValuesY();
	m_view->m_clip_mask = -1;
	m_view->UpdateClips();

	//links
	SetYLink(false);

	FluoRefresh(false, true, 2);
}

void ClippingView::ResetClipValuesZ()
{
	if (!m_vd || !m_view)
		return;
	if (m_chann_link)
		m_view->ResetClipValuesZ();
	else
		m_vd->ResetClipValuesZ();
	m_view->m_clip_mask = -1;
	m_view->UpdateClips();

	//links
	SetZLink(false);

	FluoRefresh(false, true, 2);
}

void ClippingView::OnLinkXCheck(wxCommandEvent &event)
{
	bool link = m_linkx_tb->GetToolState(0);
	SetXLink(link);
}

void ClippingView::OnLinkYCheck(wxCommandEvent &event)
{
	bool link = m_linky_tb->GetToolState(0);
	SetYLink(link);
}

void ClippingView::OnLinkZCheck(wxCommandEvent &event)
{
	bool link = m_linkz_tb->GetToolState(0);
	SetZLink(link);
}

void ClippingView::OnClipDistXEdit(wxCommandEvent& event)
{
	if (!m_vd)
		return;

	wxString str = m_yz_dist_text->GetValue();
	long val;
	if (str.ToLong(&val))
		m_vd->SetClipDistX(val);
}

void ClippingView::OnClipDistYEdit(wxCommandEvent& event)
{
	if (!m_vd)
		return;

	wxString str = m_xz_dist_text->GetValue();
	long val;
	if (str.ToLong(&val))
		m_vd->SetClipDistY(val);
}

void ClippingView::OnClipDistZEdit(wxCommandEvent& event)
{
	if (!m_vd)
		return;

	wxString str = m_xy_dist_text->GetValue();
	long val;
	if (str.ToLong(&val))
		m_vd->SetClipDistZ(val);
}

void ClippingView::OnSetZeroBtn(wxCommandEvent &event)
{
	if (!m_view)
		return;

	m_view->SetClipMode(2);
	FluoRefresh(false, true, 2);
}

void ClippingView::OnRotResetBtn(wxCommandEvent &event)
{
	if (!m_view)
		return;

	//reset rotations
	m_view->SetClippingPlaneRotations(0.0, 0.0, 0.0);
	FluoRefresh(false, true, 2);
}

void ClippingView::OnRotXMF(wxCommandEvent& event)
{
	switch (glbin_settings.m_mulfunc)
	{
	case 0:
		break;
	case 1:
		SetFocusVRenderViews(m_x_rot_sldr);
		break;
	case 2:
		if (m_view) m_view->SetClipRotX(0.0);
		FluoRefresh(false, true, 2);
		break;
	case 3:
		break;
	case 4:
		m_x_rot_sldr->Undo();
		break;
	case 5:
		break;
	}
}

void ClippingView::OnRotYMF(wxCommandEvent& event)
{
	switch (glbin_settings.m_mulfunc)
	{
	case 0:
		break;
	case 1:
		SetFocusVRenderViews(m_y_rot_sldr);
		break;
	case 2:
		if (m_view) m_view->SetClipRotY(0.0);
		FluoRefresh(false, true, 2);
		break;
	case 3:
		break;
	case 4:
		m_y_rot_sldr->Undo();
		break;
	case 5:
		break;
	}
}

void ClippingView::OnRotZMF(wxCommandEvent& event)
{
	switch (glbin_settings.m_mulfunc)
	{
	case 0:
		break;
	case 1:
		SetFocusVRenderViews(m_z_rot_sldr);
		break;
	case 2:
		if (m_view) m_view->SetClipRotZ(0.0);
		FluoRefresh(false, true, 2);
		break;
	case 3:
		break;
	case 4:
		m_z_rot_sldr->Undo();
		break;
	case 5:
		break;
	}
}

void ClippingView::OnXRotChange(wxScrollEvent &event)
{
	if (!m_view)
		return;

	int val = m_x_rot_sldr->GetValue();
	m_view->SetClipRotX(val);
	FluoRefresh(false, true, 2);
}

void ClippingView::OnXRotEdit(wxCommandEvent &event)
{
	if (!m_view)
		return;

	wxString str = m_x_rot_text->GetValue();
	double val = 0.0;
	str.ToDouble(&val);
	m_view->SetClipRotX(val);
	FluoRefresh(false, true, 2);
}

void ClippingView::OnYRotChange(wxScrollEvent &event)
{
	if (!m_view)
		return;

	int val = m_y_rot_sldr->GetValue();
	m_view->SetClipRotY(val);
	FluoRefresh(false, true, 2);
}

void ClippingView::OnYRotEdit(wxCommandEvent &event)
{
	if (!m_view)
		return;

	wxString str = m_y_rot_text->GetValue();
	double val = 0.0;
	str.ToDouble(&val);
	m_view->SetClipRotY(val);
	FluoRefresh(false, true, 2);
}

void ClippingView::OnZRotChange(wxScrollEvent &event)
{
	if (!m_view)
		return;

	int val = m_z_rot_sldr->GetValue();
	m_view->SetClipRotZ(val);
	FluoRefresh(false, true, 2);
}

void ClippingView::OnZRotEdit(wxCommandEvent &event)
{
	if (!m_view)
		return;

	wxString str = m_z_rot_text->GetValue();
	double val = 0.0;
	str.ToDouble(&val);
	m_view->SetClipRotZ(val);
	FluoRefresh(false, true, 2);
}

void ClippingView::OnXRotSpinUp(wxSpinEvent& event)
{
	wxString str_val = m_x_rot_text->GetValue();
	double val;
	str_val.ToDouble(&val);
	val += glbin_settings.m_inverse_slider ? -1 : 1;
	if (val > 180.0) val -= 360.0;
	if (val <-180.0) val += 360.0;
	m_view->SetClipRotX(val);
	FluoRefresh(false, true, 2);
}

void ClippingView::OnXRotSpinDown(wxSpinEvent& event)
{
	wxString str_val = m_x_rot_text->GetValue();
	double val;
	str_val.ToDouble(&val);
	val += glbin_settings.m_inverse_slider ? 1 : -1;
	if (val > 180.0) val -= 360.0;
	if (val <-180.0) val += 360.0;
	m_view->SetClipRotX(val);
	FluoRefresh(false, true, 2);
}

void ClippingView::OnYRotSpinUp(wxSpinEvent& event)
{
	wxString str_val = m_y_rot_text->GetValue();
	double val;
	str_val.ToDouble(&val);
	val += glbin_settings.m_inverse_slider ? -1 : 1;
	if (val > 180.0) val -= 360.0;
	if (val <-180.0) val += 360.0;
	m_view->SetClipRotY(val);
	FluoRefresh(false, true, 2);
}

void ClippingView::OnYRotSpinDown(wxSpinEvent& event)
{
	wxString str_val = m_y_rot_text->GetValue();
	double val;
	str_val.ToDouble(&val);
	val += glbin_settings.m_inverse_slider ? 1 : -1;
	if (val > 180.0) val -= 360.0;
	if (val <-180.0) val += 360.0;
	m_view->SetClipRotY(val);
	FluoRefresh(false, true, 2);
}

void ClippingView::OnZRotSpinUp(wxSpinEvent& event)
{
	wxString str_val = m_z_rot_text->GetValue();
	double val;
	str_val.ToDouble(&val);
	val += glbin_settings.m_inverse_slider ? -1 : 1;
	if (val > 180.0) val -= 360.0;
	if (val <-180.0) val += 360.0;
	m_view->SetClipRotZ(val);
	FluoRefresh(false, true, 2);
}

void ClippingView::OnZRotSpinDown(wxSpinEvent& event)
{
	wxString str_val = m_z_rot_text->GetValue();
	double val;
	str_val.ToDouble(&val);
	val += glbin_settings.m_inverse_slider ? 1 : -1;
	if (val > 180.0) val -= 360.0;
	if (val <-180.0) val += 360.0;
	m_view->SetClipRotZ(val);
	FluoRefresh(false, true, 2);
}

void ClippingView::UpdateSampleRate()
{
	if (m_sel_type != 2 || !m_vd)
		return;

	//good rate
	if (m_vd->GetSampleRate() < 2.0)
		m_vd->SetSampleRate(2.0);
	if (m_chann_link)
	{
		int i;
		for (i = 0; i < glbin_data_manager.GetVolumeNum(); i++)
		{
			VolumeData* vd = glbin_data_manager.GetVolumeData(i);
			if (!vd || vd == m_vd)
				continue;
			if (vd->GetSampleRate() < 2.0)
				vd->SetSampleRate(2.0);
		}
	}
}

void ClippingView::OnClipXRClick(wxMouseEvent& event)
{
	if (m_sel_type != 2 || !m_vd)
		return;

	int resx, resy, resz;
	m_vd->GetResolution(resx, resy, resz);

	SetXLink(true);
	SetYLink(false);
	SetZLink(false);

	int clip = m_clipx_sldr->GetLowValue();
	int val[6] =
	{
		clip,
		clip + 1,
		0, resy,
		0, resz
	};
	SetClipValues(val);
}

void ClippingView::OnClipYRClick(wxMouseEvent& event)
{
	if (m_sel_type != 2 || !m_vd)
		return;

	int resx, resy, resz;
	m_vd->GetResolution(resx, resy, resz);

	SetXLink(false);
	SetYLink(true);
	SetZLink(false);

	int clip = m_clipy_sldr->GetLowValue();
	int val[6] =
	{
		0, resx,
		clip,
		clip + 1,
		0, resz
	};
	SetClipValues(val);
}

void ClippingView::OnClipZRClick(wxMouseEvent& event)
{
	if (m_sel_type != 2 || !m_vd)
		return;

	int resx, resy, resz;
	m_vd->GetResolution(resx, resy, resz);

	SetXLink(false);
	SetYLink(false);
	SetZLink(true);

	int clip = m_clipz_sldr->GetLowValue();
	int val[6] =
	{
		0, resx,
		0, resy,
		clip,
		clip + 1
	};
	SetClipValues(val);
}

void ClippingView::OnYZClipBtn(wxCommandEvent& event)
{
	if (m_sel_type!=2 || !m_vd)
		return;

	int resx, resy, resz;
	m_vd->GetResolution(resx, resy, resz);

	//reset yz
	SetYLink(false);
	SetZLink(false);
	//set x
	SetXLink(true);

	int val[6] = 
	{
		(resx - m_vd->GetClipDistX()) / 2,
		(resx + m_vd->GetClipDistX()) / 2,
		0, resy, 0, resz
	};
	SetClipValues(val);
}

void ClippingView::OnXZClipBtn(wxCommandEvent& event)
{
	if (m_sel_type!=2 || !m_vd)
		return;

	int resx, resy, resz;
	m_vd->GetResolution(resx, resy, resz);

	//reset xz
	SetXLink(false);
	SetZLink(false);
	//set y
	SetYLink(true);

	int val[6] =
	{
		0, resx,
		(resy - m_vd->GetClipDistY()) / 2,
		(resy + m_vd->GetClipDistY()) / 2,
		0, resz
	};
	SetClipValues(val);
}

void ClippingView::OnXYClipBtn(wxCommandEvent& event)
{
	if (m_sel_type!=2 || !m_vd)
		return;

	int resx, resy, resz;
	m_vd->GetResolution(resx, resy, resz);

	//reset xy
	SetXLink(false);
	SetYLink(false);
	//set z
	SetZLink(true);

	int val[6] =
	{
		0, resx,
		0, resy,
		(resz - m_vd->GetClipDistZ()) / 2,
		(resz + m_vd->GetClipDistZ()) / 2
	};
	SetClipValues(val);
}

//move linked clipping planes
void ClippingView::MoveLinkedClippingPlanes(int dir)
{
	if (m_sel_type!=2 || !m_vd)
		return;

	wxString str;
	long dist;

	//moving lower
	if (m_clipx_sldr->GetLink())
	{
		str = m_yz_dist_text->GetValue();
		str.ToLong(&dist);
		dist = dist < 2 ? dist + 1 : dist;
		m_clipx_sldr->Scroll(dir * dist);
	}
	if (m_clipy_sldr->GetLink())
	{
		str = m_xz_dist_text->GetValue();
		str.ToLong(&dist);
		dist = dist < 2 ? dist + 1 : dist;
		m_clipy_sldr->Scroll(dir * dist);
	}
	if (m_clipz_sldr->GetLink())
	{
		str = m_xy_dist_text->GetValue();
		str.ToLong(&dist);
		dist = dist < 2 ? dist + 1 : dist;
		m_clipz_sldr->Scroll(dir * dist);
	}
}

void ClippingView::ClearUndo()
{
	m_clipx_sldr->Clear();
	m_clipy_sldr->Clear();
	m_clipz_sldr->Clear();
	m_x_rot_sldr->Clear();
	m_y_rot_sldr->Clear();
	m_z_rot_sldr->Clear();
}

void ClippingView::EnableAll(bool val)
{
	if (m_enable_all == val)
		return;
	m_enable_all = val;

	m_toolbar->Enable(val);
	m_set_zero_btn->Enable(val);
	m_rot_reset_btn->Enable(val);
	m_x_rot_sldr->Enable(val);
	m_y_rot_sldr->Enable(val);
	m_z_rot_sldr->Enable(val);
	m_x_rot_text->Enable(val);
	m_y_rot_text->Enable(val);
	m_z_rot_text->Enable(val);
	m_x_rot_spin->Enable(val);
	m_y_rot_spin->Enable(val);
	m_z_rot_spin->Enable(val);
	m_clipx_sldr->Enable(val);
	m_x1_clip_text->Enable(val);
	m_x2_clip_text->Enable(val);
	m_clipy_sldr->Enable(val);
	m_y1_clip_text->Enable(val);
	m_y2_clip_text->Enable(val);
	m_clipz_sldr->Enable(val);
	m_z1_clip_text->Enable(val);
	m_z2_clip_text->Enable(val);
	m_linkx_tb->Enable(val);
	m_linky_tb->Enable(val);
	m_linkz_tb->Enable(val);
	m_clip_reset_btn->Enable(val);
	m_yz_clip_btn->Enable(val);
	m_xz_clip_btn->Enable(val);
	m_xy_clip_btn->Enable(val);
	m_yz_dist_text->Enable(val);
	m_xz_dist_text->Enable(val);
	m_xy_dist_text->Enable(val);
}
