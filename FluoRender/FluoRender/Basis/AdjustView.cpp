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
#include "AdjustView.h"
#include <Global/Global.h>
#include <VRenderFrame.h>
#include <VRenderGLView.h>
#include <wxSingleSlider.h>
#include <wx/valnum.h>
#include <wx/stdpaths.h>
#include "png_resource.h"
#include "img/icons.h"

AdjustView::AdjustView(VRenderFrame* frame,
					   const wxPoint& pos,
					   const wxSize& size,
					   long style,
					   const wxString& name) :
	PropPanel(frame, frame, pos, size, style, name),
m_type(-1),
m_view(0),
m_vd(0),
m_group(0),
m_link_group(false),
m_enable_all(true),
m_sync()
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);
	Freeze();
	SetDoubleBuffered(true);

	//this->SetSize(75,-1);
	//validator: floating point 2
	wxFloatingPointValidator<double> vald_fp2(2);
	//validator: integer
	wxIntegerValidator<int> vald_int;
	vald_int.SetRange(-256, 256);

	long ls = glbin_settings.m_inverse_slider ? wxSL_VERTICAL : (wxSL_VERTICAL | wxSL_INVERSE);
	wxBoxSizer *sizer_v = new wxBoxSizer(wxVERTICAL);
	wxStaticText *st;

	//first line: text
	wxBoxSizer *sizer_h_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Gam.",
		wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
	sizer_h_1->Add(st, 1, wxEXPAND);
	st = new wxStaticText(this, 0, "Lum.",
		wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
	sizer_h_1->Add(st, 1, wxEXPAND);
	st = new wxStaticText(this, 0, "Eql.",
		wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
	sizer_h_1->Add(st, 1, wxEXPAND);
	sizer_v->Add(sizer_h_1, 0, wxEXPAND);
	//space
	sizer_v->Add(5, 5, 0);

	wxBitmap bitmap;
	//second line: red
	wxBoxSizer *sizer_h_2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Red:",
		wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
	m_sync_r_chk = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	bitmap = wxGetBitmapFromMemory(unlink);
#ifdef _DARWIN
	m_sync_r_chk->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_sync_r_chk->AddCheckTool(0, "Link",
		bitmap, wxNullBitmap,
		"Link Red Properties with Linked Green or Blue",
		"Link Red Properties with Linked Green or Blue");
	m_sync_r_chk->Bind(wxEVT_TOOL, &AdjustView::OnSyncRCheck, this);
	m_sync_r_chk->Realize();
	sizer_h_2->Add(3, 3, 0);
	sizer_h_2->Add(st, 0, wxALIGN_CENTER);
	sizer_h_2->AddStretchSpacer(1);
	sizer_h_2->Add(m_sync_r_chk, 0, wxALIGN_CENTER);
	sizer_v->Add(sizer_h_2, 0, wxEXPAND);
	sizer_v->Add(3, 3, 0);

	//third line: red bar
	st = new wxStaticText(this, 0, "", wxDefaultPosition, FromDIP(wxSize(5,5)));
	st->SetBackgroundColour(wxColor(255, 0, 0));
	sizer_v->Add(st, 0, wxEXPAND);

	//fourth line: sliders
	wxBoxSizer *sizer_h_3 = new wxBoxSizer(wxHORIZONTAL);
	m_r_gamma_sldr = new wxSingleSlider(this, wxID_ANY, 100, 10, 400,
		wxDefaultPosition, wxDefaultSize, ls);
	m_r_gamma_sldr->Bind(wxEVT_SCROLL_CHANGED, &AdjustView::OnRGammaChange, this);
	sizer_h_3->Add(m_r_gamma_sldr, 1, wxEXPAND);
	m_r_brightness_sldr = new wxSingleSlider(this, wxID_ANY, 0, -256, 256,
		wxDefaultPosition, wxDefaultSize, ls);
	m_r_brightness_sldr->SetRangeStyle(2);
	m_r_brightness_sldr->Bind(wxEVT_SCROLL_CHANGED, &AdjustView::OnRBrightnessChange, this);
	sizer_h_3->Add(m_r_brightness_sldr, 1, wxEXPAND);
	m_r_hdr_sldr = new wxSingleSlider(this, wxID_ANY, 0, 0, 100,
		wxDefaultPosition, wxDefaultSize, ls);
	m_r_hdr_sldr->Bind(wxEVT_SCROLL_CHANGED, &AdjustView::OnRHdrChange, this);
	sizer_h_3->Add(m_r_hdr_sldr, 1, wxEXPAND);
	sizer_v->Add(sizer_h_3, 1, wxEXPAND);

	//5th line: input boxes
	wxBoxSizer *sizer_h_4 = new wxBoxSizer(wxHORIZONTAL);
	vald_fp2.SetRange(0.0, 10.0);
	m_r_gamma_text = new wxTextCtrl(this, wxID_ANY, "1.00",
		wxDefaultPosition, FromDIP(wxSize(30, 20)), 0, vald_fp2);
	m_r_gamma_text->Bind(wxEVT_TEXT, &AdjustView::OnRGammaText, this);
	sizer_h_4->Add(m_r_gamma_text, 1, wxEXPAND);
	m_r_brightness_text = new wxTextCtrl(this, wxID_ANY, "0",
		wxDefaultPosition, FromDIP(wxSize(30, 20)), 0, vald_int);
	m_r_brightness_text->Bind(wxEVT_TEXT, &AdjustView::OnRBrightnessText, this);
	sizer_h_4->Add(m_r_brightness_text, 1, wxEXPAND);
	vald_fp2.SetRange(0.0, 1.0);
	m_r_hdr_text = new wxTextCtrl(this, wxID_ANY, "0.00",
		wxDefaultPosition, FromDIP(wxSize(30, 20)), 0, vald_fp2);
	m_r_hdr_text->Bind(wxEVT_TEXT, &AdjustView::OnRHdrText, this);
	sizer_h_4->Add(m_r_hdr_text, 1, wxEXPAND);
	sizer_v->Add(sizer_h_4, 0, wxEXPAND);

	//6th line: buttons
	wxBoxSizer* sizer_h_5 = new wxBoxSizer(wxHORIZONTAL);
	m_r_gamma_st = new wxButton(this, wxID_ANY, "Gam.R.",
		wxDefaultPosition, FromDIP(wxSize(30, 20)));
	m_r_gamma_st->Bind(wxEVT_BUTTON, &AdjustView::OnRGammaMF, this);
	sizer_h_5->Add(m_r_gamma_st, 1, wxEXPAND);
	m_r_brightness_st = new wxButton(this, wxID_ANY, "Lum.R.",
		wxDefaultPosition, FromDIP(wxSize(30, 20)));
	m_r_brightness_st->Bind(wxEVT_BUTTON, &AdjustView::OnRBrightnessMF, this);
	sizer_h_5->Add(m_r_brightness_st, 1, wxEXPAND);
	m_r_hdr_st = new wxButton(this, wxID_ANY, "Eql.R.",
		wxDefaultPosition, FromDIP(wxSize(30, 20)));
	m_r_hdr_st->Bind(wxEVT_BUTTON, &AdjustView::OnRHdrMF, this);
	sizer_h_5->Add(m_r_hdr_st, 1, wxEXPAND);
	sizer_v->Add(sizer_h_5, 0, wxEXPAND);

	//7th line: reset buttons
#ifndef _DARWIN
	m_r_reset_btn = new wxButton(this, wxID_ANY, "Reset",
								 wxDefaultPosition, FromDIP(wxSize(30, 22)));
#else
	m_r_reset_btn = new wxButton(this, ID_RResetBtn, "Reset",
								 wxDefaultPosition, FromDIP(wxSize(30, 30)));
#endif
	m_r_reset_btn->SetBitmap(wxGetBitmapFromMemory(reset));
	m_r_reset_btn->Bind(wxEVT_BUTTON, &AdjustView::OnRReset, this);
	sizer_v->Add(m_r_reset_btn, 0, wxEXPAND);

	//space
	sizer_v->Add(5, 5, 0);

	//8th line: green
	wxBoxSizer *sizer_h_6 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Green:",
		wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
	m_sync_g_chk = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	bitmap = wxGetBitmapFromMemory(unlink);
#ifdef _DARWIN
	m_sync_g_chk->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_sync_g_chk->AddCheckTool(0, "Link",
		bitmap, wxNullBitmap,
		"Link Green Properties with Linked Red or Blue",
		"Link Green Properties with Linked Red or Blue");
	m_sync_g_chk->Bind(wxEVT_TOOL, &AdjustView::OnSyncGCheck, this);
	m_sync_g_chk->Realize();
	sizer_h_6->Add(3, 3, 0);
	sizer_h_6->Add(st, 0, wxALIGN_CENTER);
	sizer_h_6->AddStretchSpacer(1);
	sizer_h_6->Add(m_sync_g_chk, 0, wxALIGN_CENTER);
	sizer_v->Add(sizer_h_6, 0, wxEXPAND);
	sizer_v->Add(3, 3, 0);

	//9th line: green bar
	st = new wxStaticText(this, 0, "", wxDefaultPosition, FromDIP(wxSize(5, 5)));
	st->SetBackgroundColour(wxColor(0, 255, 0));
	sizer_v->Add(st, 0, wxEXPAND);

	//10th line: sliders
	wxBoxSizer *sizer_h_7 = new wxBoxSizer(wxHORIZONTAL);
	m_g_gamma_sldr = new wxSingleSlider(this, wxID_ANY, 100, 10, 400,
		wxDefaultPosition, wxDefaultSize, ls);
	m_g_gamma_sldr->Bind(wxEVT_SCROLL_CHANGED, &AdjustView::OnGGammaChange, this);
	sizer_h_7->Add(m_g_gamma_sldr, 1, wxEXPAND);
	m_g_brightness_sldr = new wxSingleSlider(this, wxID_ANY, 0, -256, 256,
		wxDefaultPosition, wxDefaultSize, ls);
	m_g_brightness_sldr->SetRangeStyle(2);
	m_g_brightness_sldr->Bind(wxEVT_SCROLL_CHANGED, &AdjustView::OnGBrightnessChange, this);
	sizer_h_7->Add(m_g_brightness_sldr, 1, wxEXPAND);
	m_g_hdr_sldr = new wxSingleSlider(this, wxID_ANY, 0, 0, 100,
		wxDefaultPosition, wxDefaultSize, ls);
	m_g_hdr_sldr->Bind(wxEVT_SCROLL_CHANGED, &AdjustView::OnGHdrChange, this);
	sizer_h_7->Add(m_g_hdr_sldr, 1, wxEXPAND);
	sizer_v->Add(sizer_h_7, 1, wxEXPAND);

	//11th line: input boxes
	wxBoxSizer *sizer_h_8 = new wxBoxSizer(wxHORIZONTAL);
	vald_fp2.SetRange(0.0, 10.0);
	m_g_gamma_text = new wxTextCtrl(this, wxID_ANY, "1.00",
		wxDefaultPosition, FromDIP(wxSize(30, 20)), 0, vald_fp2);
	m_g_gamma_text->Bind(wxEVT_TEXT, &AdjustView::OnGGammaText, this);
	sizer_h_8->Add(m_g_gamma_text, 1, wxEXPAND);
	m_g_brightness_text = new wxTextCtrl(this, wxID_ANY, "0",
		wxDefaultPosition, FromDIP(wxSize(30, 20)), 0, vald_int);
	m_g_brightness_text->Bind(wxEVT_TEXT, &AdjustView::OnGBrightnessText, this);
	sizer_h_8->Add(m_g_brightness_text, 1, wxEXPAND);
	vald_fp2.SetRange(0.0, 1.0);
	m_g_hdr_text = new wxTextCtrl(this, wxID_ANY, "0.00",
		wxDefaultPosition, FromDIP(wxSize(30, 20)), 0, vald_fp2);
	m_g_hdr_text->Bind(wxEVT_TEXT, &AdjustView::OnGHdrText, this);
	sizer_h_8->Add(m_g_hdr_text, 1, wxEXPAND);
	sizer_v->Add(sizer_h_8, 0, wxEXPAND);

	//12th line: buttons
	wxBoxSizer* sizer_h_9 = new wxBoxSizer(wxHORIZONTAL);
	m_g_gamma_st = new wxButton(this, wxID_ANY, "Gam.G.",
		wxDefaultPosition, FromDIP(wxSize(30, 20)));
	m_g_gamma_st->Bind(wxEVT_BUTTON, &AdjustView::OnGGammaMF, this);
	sizer_h_9->Add(m_g_gamma_st, 1, wxEXPAND);
	m_g_brightness_st = new wxButton(this, wxID_ANY, "Lum.G.",
		wxDefaultPosition, FromDIP(wxSize(30, 20)));
	m_g_brightness_st->Bind(wxEVT_BUTTON, &AdjustView::OnGBrightnessMF, this);
	sizer_h_9->Add(m_g_brightness_st, 1, wxEXPAND);
	m_g_hdr_st = new wxButton(this, wxID_ANY, "Eql.G.",
		wxDefaultPosition, FromDIP(wxSize(30, 20)));
	m_g_hdr_st->Bind(wxEVT_BUTTON, &AdjustView::OnGHdrMF, this);
	sizer_h_9->Add(m_g_hdr_st, 1, wxEXPAND);
	sizer_v->Add(sizer_h_9, 0, wxEXPAND);

	//13th line: reset buttons
#ifndef _DARWIN
	m_g_reset_btn = new wxButton(this, wxID_ANY, "Reset",
								 wxDefaultPosition, FromDIP(wxSize(30, 22)));
#else
	m_g_reset_btn = new wxButton(this, ID_GResetBtn, "Reset",
								 wxDefaultPosition, FromDIP(wxSize(30, 30)));
#endif
	m_g_reset_btn->SetBitmap(wxGetBitmapFromMemory(reset));
	m_g_reset_btn->Bind(wxEVT_BUTTON, &AdjustView::OnGReset, this);
	sizer_v->Add(m_g_reset_btn, 0, wxEXPAND);

	//space
	sizer_v->Add(5, 5, 0);

	//14th line: blue
	wxBoxSizer *sizer_h_10 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Blue:",
		wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
	m_sync_b_chk = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	bitmap = wxGetBitmapFromMemory(unlink);
#ifdef _DARWIN
	m_sync_b_chk->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_sync_b_chk->AddCheckTool(0, "Link",
		bitmap, wxNullBitmap,
		"Link Blue Properties with Linked Red or Green",
		"Link Blue Properties with Linked Red or Green");
	m_sync_b_chk->Bind(wxEVT_TOOL, &AdjustView::OnSyncBCheck, this);
	m_sync_b_chk->Realize();
	sizer_h_10->Add(3, 3, 0);
	sizer_h_10->Add(st, 0, wxALIGN_CENTER);
	sizer_h_10->AddStretchSpacer(1);
	sizer_h_10->Add(m_sync_b_chk, 0, wxALIGN_CENTER);
	sizer_v->Add(sizer_h_10, 0, wxEXPAND);
	sizer_v->Add(3, 3, 0);

	//15th line:blue bar
	st = new wxStaticText(this, 0, "", wxDefaultPosition, FromDIP(wxSize(5, 5)));
	st->SetBackgroundColour(wxColor(0, 0, 255));
	sizer_v->Add(st, 0, wxEXPAND);

	//16th line: sliders
	wxBoxSizer *sizer_h_11 = new wxBoxSizer(wxHORIZONTAL);
	m_b_gamma_sldr = new wxSingleSlider(this, wxID_ANY, 100, 10, 400,
		wxDefaultPosition, wxDefaultSize, ls);
	m_b_gamma_sldr->Bind(wxEVT_SCROLL_CHANGED, &AdjustView::OnBGammaChange, this);
	sizer_h_11->Add(m_b_gamma_sldr, 1, wxEXPAND);
	m_b_brightness_sldr = new wxSingleSlider(this, wxID_ANY, 0, -256, 256,
		wxDefaultPosition, wxDefaultSize, ls);
	m_b_brightness_sldr->SetRangeStyle(2);
	m_b_brightness_sldr->Bind(wxEVT_SCROLL_CHANGED, &AdjustView::OnBBrightnessChange, this);
	sizer_h_11->Add(m_b_brightness_sldr, 1, wxEXPAND);
	m_b_hdr_sldr = new wxSingleSlider(this, wxID_ANY, 0, 0, 100,
		wxDefaultPosition, wxDefaultSize, ls);
	m_b_hdr_sldr->Bind(wxEVT_SCROLL_CHANGED, &AdjustView::OnBHdrChange, this);
	sizer_h_11->Add(m_b_hdr_sldr, 1, wxEXPAND);
	sizer_v->Add(sizer_h_11, 1, wxEXPAND);

	//17th line: input boxes
	wxBoxSizer* sizer_h_12 = new wxBoxSizer(wxHORIZONTAL);
	vald_fp2.SetRange(0.0, 10.0);
	m_b_gamma_text = new wxTextCtrl(this, wxID_ANY, "1.00",
		wxDefaultPosition, FromDIP(wxSize(30, 20)), 0, vald_fp2);
	m_b_gamma_text->Bind(wxEVT_TEXT, &AdjustView::OnBGammaText, this);
	sizer_h_12->Add(m_b_gamma_text, 1, wxEXPAND);
	m_b_brightness_text = new wxTextCtrl(this, wxID_ANY, "0",
		wxDefaultPosition, FromDIP(wxSize(30, 20)), 0, vald_int);
	m_b_brightness_text->Bind(wxEVT_TEXT, &AdjustView::OnBBrightnessText, this);
	sizer_h_12->Add(m_b_brightness_text, 1, wxEXPAND);
	vald_fp2.SetRange(0.0, 1.0);
	m_b_hdr_text = new wxTextCtrl(this, wxID_ANY, "0.00",
		wxDefaultPosition, FromDIP(wxSize(30, 20)), 0, vald_fp2);
	m_b_hdr_text->Bind(wxEVT_TEXT, &AdjustView::OnBHdrText, this);
	sizer_h_12->Add(m_b_hdr_text, 1, wxEXPAND);
	sizer_v->Add(sizer_h_12, 0, wxEXPAND);

	//18th line: buttons
	wxBoxSizer* sizer_h_13 = new wxBoxSizer(wxHORIZONTAL);
	m_b_gamma_st = new wxButton(this, wxID_ANY, "Gam.B.",
		wxDefaultPosition, FromDIP(wxSize(30, 20)));
	m_b_gamma_st->Bind(wxEVT_BUTTON, &AdjustView::OnBGammaMF, this);
	sizer_h_13->Add(m_b_gamma_st, 1, wxEXPAND);
	m_b_brightness_st = new wxButton(this, wxID_ANY, "Lum.B.",
		wxDefaultPosition, FromDIP(wxSize(30, 20)));
	m_b_brightness_st->Bind(wxEVT_BUTTON, &AdjustView::OnBBrightnessMF, this);
	sizer_h_13->Add(m_b_brightness_st, 1, wxEXPAND);
	m_b_hdr_st = new wxButton(this, wxID_ANY, "Eql.B.",
		wxDefaultPosition, FromDIP(wxSize(30, 20)));
	m_b_hdr_st->Bind(wxEVT_BUTTON, &AdjustView::OnBHdrMF, this);
	sizer_h_13->Add(m_b_hdr_st, 1, wxEXPAND);
	sizer_v->Add(sizer_h_13, 0, wxEXPAND);

	//19th line: reset buttons
#ifndef _DARWIN
	m_b_reset_btn = new wxButton(this, wxID_ANY, "Reset",
								 wxDefaultPosition, FromDIP(wxSize(30, 22)));
#else
	m_b_reset_btn = new wxButton(this, ID_BResetBtn, "Reset",
								 wxDefaultPosition, FromDIP(wxSize(30, 30)));
#endif
	m_b_reset_btn->SetBitmap(wxGetBitmapFromMemory(reset));
	m_b_reset_btn->Bind(wxEVT_BUTTON, &AdjustView::OnBReset, this);
	sizer_v->Add(m_b_reset_btn, 0, wxEXPAND);

	//20th line: default button
#ifndef _DARWIN
	m_dft_btn = new wxButton(this, wxID_ANY, "Set Default",
							 wxDefaultPosition, FromDIP(wxSize(95, 22)));
#else
	m_dft_btn = new wxButton(this, ID_DefaultBtn, "Set Default",
							 wxDefaultPosition, FromDIP(wxSize(95, 30)));
#endif
	m_dft_btn->SetBitmap(wxGetBitmapFromMemory(save_settings));
	m_dft_btn->Bind(wxEVT_BUTTON, &AdjustView::OnSaveDefault, this);
	sizer_v->Add(m_dft_btn, 0, wxEXPAND);

	SetSizer(sizer_v);
	Layout();
	SetAutoLayout(true);
	SetScrollRate(10, 10);

	EnableAll(false);

	//add sliders for undo and redo
	glbin.add_undo_control(m_r_gamma_sldr);
	glbin.add_undo_control(m_r_brightness_sldr);
	glbin.add_undo_control(m_r_hdr_sldr);
	glbin.add_undo_control(m_g_gamma_sldr);
	glbin.add_undo_control(m_g_brightness_sldr);
	glbin.add_undo_control(m_g_hdr_sldr);
	glbin.add_undo_control(m_b_gamma_sldr);
	glbin.add_undo_control(m_b_brightness_sldr);
	glbin.add_undo_control(m_b_hdr_sldr);

	Thaw();
}

AdjustView::~AdjustView()
{
	//delete sliders for undo and redo
	glbin.del_undo_control(m_r_gamma_sldr);
	glbin.del_undo_control(m_r_brightness_sldr);
	glbin.del_undo_control(m_r_hdr_sldr);
	glbin.del_undo_control(m_g_gamma_sldr);
	glbin.del_undo_control(m_g_brightness_sldr);
	glbin.del_undo_control(m_g_hdr_sldr);
	glbin.del_undo_control(m_b_gamma_sldr);
	glbin.del_undo_control(m_b_brightness_sldr);
	glbin.del_undo_control(m_b_hdr_sldr);

	SetFocusVRenderViews(0);
}

void AdjustView::FluoUpdate(const fluo::ValueCollection& vc)
{
	if (m_type != 1 && m_type != 2 && m_type != 5)
	{
		EnableAll(false);
		return;
	}
	if (FOUND_VALUE(gstNull))
		return;
	bool update_all = vc.empty();
	bool bSyncR = FOUND_VALUE(gstSyncR);
	bool bSyncG = FOUND_VALUE(gstSyncG);
	bool bSyncB = FOUND_VALUE(gstSyncB);
	bool bGammaR = FOUND_VALUE(gstGammaR);
	bool bGammaG = FOUND_VALUE(gstGammaG);
	bool bGammaB = FOUND_VALUE(gstGammaB);
	bool bBrightnessR = FOUND_VALUE(gstBrightnessR);
	bool bBrightnessG = FOUND_VALUE(gstBrightnessG);
	bool bBrightnessB = FOUND_VALUE(gstBrightnessB);
	bool bHdrR = FOUND_VALUE(gstEqualizeR);
	bool bHdrG = FOUND_VALUE(gstEqualizeG);
	bool bHdrB = FOUND_VALUE(gstEqualizeB);
	if (!(update_all ||
		bSyncR || bSyncG || bSyncB ||
		bGammaR || bGammaG || bGammaB ||
		bBrightnessR || bBrightnessG || bBrightnessB ||
		bHdrR || bHdrG || bHdrB))
		return;

	fluo::Color gamma;
	fluo::Color brightness;
	fluo::Color hdr;

	switch (m_type)
	{
	case 1://view
		if (m_view)
		{
			for (int i : {0, 1, 2})
				m_sync[i] = m_view->GetSync(i);
			gamma = m_view->GetGammaColor();
			brightness = m_view->GetBrightness();
			hdr = m_view->GetHdr();
		}
		break;
	case 2://volume data
	case 5://group
		{
			TreeLayer* layer = 0;
			if (m_type == 2 && m_vd)
				layer = (TreeLayer*)m_vd;
			else if (m_type == 5 && m_group)
				layer = (TreeLayer*)m_group;

			if (layer)
			{
				for (int i : {0, 1, 2})
					m_sync[i] = layer->GetSync(i);
				gamma = layer->GetGammaColor();
				brightness = layer->GetBrightness();
				hdr = layer->GetHdr();
			}
		}
	break;
	}

	//red
	if (update_all || bSyncR)
	{
		m_sync_r_chk->ToggleTool(0, m_sync[0]);
		m_sync_r_chk->SetToolNormalBitmap(0,
			m_sync[0] ? wxGetBitmapFromMemory(link) : wxGetBitmapFromMemory(unlink));
	}
	if (update_all || bGammaR || bSyncR)
	{
		m_r_gamma_sldr->ChangeValue(std::round((1.0 / gamma.r()) * 100.0));
		m_r_gamma_text->ChangeValue(wxString::Format("%.2f", 1.0 / gamma.r()));
	}
	if (update_all || bBrightnessR || bSyncR)
	{
		m_r_brightness_sldr->ChangeValue(std::round((brightness.r() - 1.0) * 256.0));
		m_r_brightness_text->ChangeValue(wxString::Format("%d", int(std::round((brightness.r()-1.0)*256.0))));
	}
	if (update_all || bHdrR || bSyncR)
	{
		m_r_hdr_sldr->ChangeValue(std::round(hdr.r() * 100.0));
		m_r_hdr_text->ChangeValue(wxString::Format("%.2f", hdr.r()));
	}
	//green
	if (update_all || bSyncG)
	{
		m_sync_g_chk->ToggleTool(0, m_sync[1]);
		m_sync_g_chk->SetToolNormalBitmap(0,
			m_sync[1] ? wxGetBitmapFromMemory(link) : wxGetBitmapFromMemory(unlink));
	}
	if (update_all || bGammaG || bSyncG)
	{
		m_g_gamma_sldr->ChangeValue(std::round((1.0 / gamma.g()) * 100.0));
		m_g_gamma_text->ChangeValue(wxString::Format("%.2f", 1.0 / gamma.g()));
	}
	if (update_all || bBrightnessG || bSyncG)
	{
		m_g_brightness_sldr->ChangeValue(std::round((brightness.g() - 1.0) * 256.0));
		m_g_brightness_text->ChangeValue(wxString::Format("%d", int(std::round((brightness.g() - 1.0) * 256.0))));
	}
	if (update_all || bHdrG || bSyncG)
	{
		m_g_hdr_sldr->ChangeValue(std::round(hdr.g() * 100.0));
		m_g_hdr_text->ChangeValue(wxString::Format("%.2f", hdr.g()));
	}
	//blue
	if (update_all || bSyncB)
	{
		m_sync_b_chk->ToggleTool(0, m_sync[2]);
		m_sync_b_chk->SetToolNormalBitmap(0,
			m_sync[2] ? wxGetBitmapFromMemory(link) : wxGetBitmapFromMemory(unlink));
	}
	if (update_all || bGammaB || bSyncB)
	{
		m_b_gamma_sldr->ChangeValue(std::round((1.0 / gamma.b()) * 100.0));
		m_b_gamma_text->ChangeValue(wxString::Format("%.2f", 1.0 / gamma.b()));
	}
	if (update_all || bBrightnessB || bSyncB)
	{
		m_b_brightness_sldr->ChangeValue(std::round((brightness.b() - 1.0) * 256.0));
		m_b_brightness_text->ChangeValue(wxString::Format("%d", int(std::round((brightness.b() - 1.0) * 256.0))));
	}
	if (update_all || bHdrB || bSyncB)
	{
		m_b_hdr_sldr->ChangeValue(std::round(hdr.b() * 100.0));
		m_b_hdr_text->ChangeValue(wxString::Format("%.2f", hdr.b()));
	}

	EnableAll(true);
}

void AdjustView::EnableAll(bool val)
{
	if (m_enable_all == val)
		return;
	m_enable_all = val;

	//red
	m_sync_r_chk->Enable(val);
	m_r_gamma_sldr->Enable(val);
	m_r_brightness_sldr->Enable(val);
	m_r_hdr_sldr->Enable(val);
	m_r_gamma_text->Enable(val);
	m_r_brightness_text->Enable(val);
	m_r_hdr_text->Enable(val);
	m_r_gamma_st->Enable(val);
	m_r_brightness_st->Enable(val);
	m_r_hdr_st->Enable(val);
	//green
	m_sync_g_chk->Enable(val);
	m_g_gamma_sldr->Enable(val);
	m_g_brightness_sldr->Enable(val);
	m_g_hdr_sldr->Enable(val);
	m_g_gamma_text->Enable(val);
	m_g_brightness_text->Enable(val);
	m_g_hdr_text->Enable(val);
	m_g_gamma_st->Enable(val);
	m_g_brightness_st->Enable(val);
	m_g_hdr_st->Enable(val);
	//blue
	m_sync_b_chk->Enable(val);
	m_b_gamma_sldr->Enable(val);
	m_b_brightness_sldr->Enable(val);
	m_b_hdr_sldr->Enable(val);
	m_b_gamma_text->Enable(val);
	m_b_brightness_text->Enable(val);
	m_b_hdr_text->Enable(val);
	m_b_gamma_st->Enable(val);
	m_b_brightness_st->Enable(val);
	m_b_hdr_st->Enable(val);
	//reset
	m_r_reset_btn->Enable(val);
	m_g_reset_btn->Enable(val);
	m_b_reset_btn->Enable(val);
	//save as default
	m_dft_btn->Enable(val);
}

//set view
void AdjustView::SetRenderView(VRenderGLView *view)
{
	if (view)
	{
		m_view = view;
		m_type = 1;
	}
	else
	{
		m_type = -1;
	}
	FluoUpdate();
}

VRenderGLView* AdjustView::GetRenderView()
{
	return m_view;
}

//set volume data
void AdjustView::SetVolumeData(VolumeData* vd)
{
	if (m_vd != vd)
		ClearUndo();

	if (vd)
	{
		m_vd = vd;
		m_type = 2;
	}
	else
	{
		m_type = -1;
	}
	FluoUpdate();
}

VolumeData* AdjustView::GetVolumeData()
{
	return m_vd;
}

//set group
void AdjustView::SetGroup(DataGroup *group)
{
	if (group)
	{
		m_group = group;
		m_type = 5;
	}
	else
	{
		m_type = -1;
	}
	FluoUpdate();
}

DataGroup* AdjustView::GetGroup()
{
	return m_group;
}

//set volume adjustment to link to group
void AdjustView::SetGroupLink(DataGroup *group)
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

void AdjustView::ClearUndo()
{
	m_r_gamma_sldr->Clear();
	m_r_brightness_sldr->Clear();
	m_r_hdr_sldr->Clear();
	m_g_gamma_sldr->Clear();
	m_g_brightness_sldr->Clear();
	m_g_hdr_sldr->Clear();
	m_b_gamma_sldr->Clear();
	m_b_brightness_sldr->Clear();
	m_b_hdr_sldr->Clear();
}

//multifunc
void AdjustView::OnRGammaMF(wxCommandEvent& event)
{
	switch (glbin_settings.m_mulfunc)
	{
	case 0:
		SyncGamma(0);
		break;
	case 1:
		SetFocusVRenderViews(m_r_gamma_sldr);
		break;
	case 2:
		SetGamma(0, glbin_outadj_def.m_gamma_r, true);
		break;
	case 3:
		break;
	case 4:
		m_r_gamma_sldr->Undo();
		break;
	case 5:
		break;
	}
}

void AdjustView::OnGGammaMF(wxCommandEvent& event)
{
	switch (glbin_settings.m_mulfunc)
	{
	case 0:
		SyncGamma(1);
		break;
	case 1:
		SetFocusVRenderViews(m_g_gamma_sldr);
		break;
	case 2:
		SetGamma(1, glbin_outadj_def.m_gamma_g, true);
		break;
	case 3:
		break;
	case 4:
		m_g_gamma_sldr->Undo();
		break;
	case 5:
		break;
	}
}

void AdjustView::OnBGammaMF(wxCommandEvent& event)
{
	switch (glbin_settings.m_mulfunc)
	{
	case 0:
		SyncGamma(2);
		break;
	case 1:
		SetFocusVRenderViews(m_b_gamma_sldr);
		break;
	case 2:
		SetGamma(2, glbin_outadj_def.m_gamma_b, true);
		break;
	case 3:
		break;
	case 4:
		m_b_gamma_sldr->Undo();
		break;
	case 5:
		break;
	}
}

void AdjustView::OnRBrightnessMF(wxCommandEvent& event)
{
	switch (glbin_settings.m_mulfunc)
	{
	case 0:
		SyncBrightness(0);
		break;
	case 1:
		SetFocusVRenderViews(m_r_brightness_sldr);
		break;
	case 2:
		SetBrightness(0, glbin_outadj_def.m_brightness_r, true);
		break;
	case 3:
		break;
	case 4:
		m_r_brightness_sldr->Undo();
		break;
	case 5:
		break;
	}
}

void AdjustView::OnGBrightnessMF(wxCommandEvent& event)
{
	switch (glbin_settings.m_mulfunc)
	{
	case 0:
		SyncBrightness(1);
		break;
	case 1:
		SetFocusVRenderViews(m_g_brightness_sldr);
		break;
	case 2:
		SetBrightness(1, glbin_outadj_def.m_brightness_g, true);
		break;
	case 3:
		break;
	case 4:
		m_g_brightness_sldr->Undo();
		break;
	case 5:
		break;
	}
}

void AdjustView::OnBBrightnessMF(wxCommandEvent& event)
{
	switch (glbin_settings.m_mulfunc)
	{
	case 0:
		SyncBrightness(2);
		break;
	case 1:
		SetFocusVRenderViews(m_b_brightness_sldr);
		break;
	case 2:
		SetBrightness(2, glbin_outadj_def.m_brightness_b, true);
		break;
	case 3:
		break;
	case 4:
		m_b_brightness_sldr->Undo();
		break;
	case 5:
		break;
	}
}

void AdjustView::OnRHdrMF(wxCommandEvent& event)
{
	switch (glbin_settings.m_mulfunc)
	{
	case 0:
		SyncHdr(0);
		break;
	case 1:
		SetFocusVRenderViews(m_r_hdr_sldr);
		break;
	case 2:
		SetHdr(0, glbin_outadj_def.m_hdr_r, true);
		break;
	case 3:
		break;
	case 4:
		m_r_hdr_sldr->Undo();
		break;
	case 5:
		break;
	}
}

void AdjustView::OnGHdrMF(wxCommandEvent& event)
{
	switch (glbin_settings.m_mulfunc)
	{
	case 0:
		SyncHdr(1);
		break;
	case 1:
		SetFocusVRenderViews(m_g_hdr_sldr);
		break;
	case 2:
		SetHdr(1, glbin_outadj_def.m_hdr_g, true);
		break;
	case 3:
		break;
	case 4:
		m_g_hdr_sldr->Undo();
		break;
	case 5:
		break;
	}
}

void AdjustView::OnBHdrMF(wxCommandEvent& event)
{
	switch (glbin_settings.m_mulfunc)
	{
	case 0:
		SyncHdr(2);
		break;
	case 1:
		SetFocusVRenderViews(m_b_hdr_sldr);
		break;
	case 2:
		SetHdr(2, glbin_outadj_def.m_hdr_b, true);
		break;
	case 3:
		break;
	case 4:
		m_b_hdr_sldr->Undo();
		break;
	case 5:
		break;
	}
}

void AdjustView::OnRGammaChange(wxScrollEvent & event)
{
	double val = m_r_gamma_sldr->GetValue() / 100.0;
	SetGamma(0, 1.0 / val);
}

void AdjustView::OnRGammaText(wxCommandEvent& event)
{
	wxString str = m_r_gamma_text->GetValue();
	double val;
	str.ToDouble(&val);
	SetGamma(0, 1.0 / val);
}

void AdjustView::OnGGammaChange(wxScrollEvent & event)
{
	double val = m_g_gamma_sldr->GetValue() / 100.0;
	SetGamma(1, 1.0 / val);
}

void AdjustView::OnGGammaText(wxCommandEvent& event)
{
	wxString str = m_g_gamma_text->GetValue();
	double val;
	str.ToDouble(&val);
	SetGamma(1, 1.0 / val);
}

void AdjustView::OnBGammaChange(wxScrollEvent & event)
{
	double val = m_b_gamma_sldr->GetValue() / 100.0;
	SetGamma(2, 1.0 / val);
}

void AdjustView::OnBGammaText(wxCommandEvent& event)
{
	wxString str = m_b_gamma_text->GetValue();
	double val;
	str.ToDouble(&val);
	SetGamma(2, 1.0 / val);
}

//brightness
void AdjustView::OnRBrightnessChange(wxScrollEvent & event)
{
	double val = m_r_brightness_sldr->GetValue() / 256.0 + 1.0;
	SetBrightness(0, val);
}

void AdjustView::OnRBrightnessText(wxCommandEvent& event)
{
	wxString str = m_r_brightness_text->GetValue();
	double val;
	str.ToDouble(&val);
	val = val / 256.0 + 1.0;
	SetBrightness(0, val);
}

void AdjustView::OnGBrightnessChange(wxScrollEvent & event)
{
	double val = m_g_brightness_sldr->GetValue() / 256.0 + 1.0;
	SetBrightness(1, val);
}

void AdjustView::OnGBrightnessText(wxCommandEvent& event)
{
	wxString str = m_g_brightness_text->GetValue();
	double val;
	str.ToDouble(&val);
	val = val / 256.0 + 1.0;
	SetBrightness(1, val);
}

void AdjustView::OnBBrightnessChange(wxScrollEvent & event)
{
	double val = m_b_brightness_sldr->GetValue() / 256.0 + 1.0;
	SetBrightness(2, val);
}

void AdjustView::OnBBrightnessText(wxCommandEvent& event)
{
	wxString str = m_b_brightness_text->GetValue();
	double val;
	str.ToDouble(&val);
	val = val / 256.0 + 1.0;
	SetBrightness(2, val);
}

void AdjustView::OnRHdrChange(wxScrollEvent &event)
{
	double val = m_r_hdr_sldr->GetValue() / 100.0;
	SetHdr(0, val);
}

void AdjustView::OnRHdrText(wxCommandEvent &event)
{
	wxString str = m_r_hdr_text->GetValue();
	double val;
	str.ToDouble(&val);
	SetHdr(0, val);
}

void AdjustView::OnGHdrChange(wxScrollEvent &event)
{
	double val = m_g_hdr_sldr->GetValue() / 100.0;
	SetHdr(1, val);
}

void AdjustView::OnGHdrText(wxCommandEvent &event)
{
	wxString str = m_g_hdr_text->GetValue();
	double val;
	str.ToDouble(&val);
	SetHdr(1, val);
}

void AdjustView::OnBHdrChange(wxScrollEvent &event)
{
	double val = m_b_hdr_sldr->GetValue() / 100.0;
	SetHdr(2, val);
}

void AdjustView::OnBHdrText(wxCommandEvent &event)
{
	wxString str = m_b_hdr_text->GetValue();
	double val;
	str.ToDouble(&val);
	SetHdr(2, val);
}

void AdjustView::OnSyncRCheck(wxCommandEvent &event)
{
	SetSync(0, m_sync_r_chk->GetToolState(0));
}

void AdjustView::OnSyncGCheck(wxCommandEvent &event)
{
	SetSync(1, m_sync_g_chk->GetToolState(0));
}

void AdjustView::OnSyncBCheck(wxCommandEvent &event)
{
	SetSync(2, m_sync_b_chk->GetToolState(0));
}

void AdjustView::OnSaveDefault(wxCommandEvent &event)
{
	switch (m_type)
	{
	case 1://view
		if (m_view)
			glbin_outadj_def.Set(m_view);
		break;
	case 2://volume data
		if (m_vd)
			glbin_outadj_def.Set(m_vd);
		break;
	case 5://group
		if (m_group)
			glbin_outadj_def.Set(m_group);
		break;
	}
}

void AdjustView::SyncColor(fluo::Color& c, double val)
{
	for (int i : {0, 1, 2})
		if (m_sync[i])
			c[i] = val;
}

void AdjustView::SyncGamma(fluo::Color& c, int i, double val, fluo::ValueCollection& vc)
{
	for (int j : {0, 1, 2})
	{
		bool changed = false;
		if (j == i)
		{
			c[j] = val;
			changed = true;
		}
		else if (m_sync[j] && m_sync[i])
		{
			c[j] = val;
			changed = true;
		}
		if (changed)
		{
			switch (j)
			{
			case 0:
				vc.insert(gstGammaR);
				break;
			case 1:
				vc.insert(gstGammaG);
				break;
			case 2:
				vc.insert(gstGammaB);
				break;
			}
		}
	}
}

void AdjustView::SyncBrightness(fluo::Color& c, int i, double val, fluo::ValueCollection& vc)
{
	for (int j : {0, 1, 2})
	{
		bool changed = false;
		if (j == i)
		{
			c[j] = val;
			changed = true;
		}
		else if (m_sync[j] && m_sync[i])
		{
			c[j] = val;
			changed = true;
		}
		if (changed)
		{
			switch (j)
			{
			case 0:
				vc.insert(gstBrightnessR);
				break;
			case 1:
				vc.insert(gstBrightnessG);
				break;
			case 2:
				vc.insert(gstBrightnessB);
				break;
			}
		}
	}
}

void AdjustView::SyncHdr(fluo::Color& c, int i, double val, fluo::ValueCollection& vc)
{
	for (int j : {0, 1, 2})
	{
		bool changed = false;
		if (j == i)
		{
			c[j] = val;
			changed = true;
		}
		else if (m_sync[j] && m_sync[i])
		{
			c[j] = val;
			changed = true;
		}
		if (changed)
		{
			switch (j)
			{
			case 0:
				vc.insert(gstEqualizeR);
				break;
			case 1:
				vc.insert(gstEqualizeG);
				break;
			case 2:
				vc.insert(gstEqualizeB);
				break;
			}
		}
	}
}

void AdjustView::SyncGamma(int i)
{
	fluo::Color gamma;
	switch (m_type)
	{
	case 1:
		if (m_view)
			gamma = m_view->GetGammaColor();
		break;
	case 2:
		if (m_vd)
			gamma = m_vd->GetGammaColor();
		break;
	case 5:
		if (m_group)
			gamma = m_group->GetGammaColor();
		break;
	}
	for (int j : {0, 1, 2})
		if (j != i) gamma[j] = gamma[i];
	switch (m_type)
	{
	case 1:
		if (m_view)
			m_view->SetGammaColor(gamma);
		break;
	case 2:
		if (m_vd)
			m_vd->SetGammaColor(gamma);
		if (m_link_group && m_group)
			m_group->SetGammaAll(gamma);
		break;
	case 3:
		if (m_group)
			m_group->SetGammaColor(gamma);
		if (m_link_group && m_group)
			m_group->SetGammaAll(gamma);
	}

	fluo::ValueCollection vc;
	if (i != 0)
		vc.insert(gstGammaR);
	if (i != 1)
		vc.insert(gstGammaG);
	if (i != 2)
		vc.insert(gstGammaB);
	FluoRefresh(false, true, false, vc);
}

void AdjustView::SyncBrightness(int i)
{
	fluo::Color brightness;
	switch (m_type)
	{
	case 1:
		if (m_view)
			brightness = m_view->GetBrightness();
		break;
	case 2:
		if (m_vd)
			brightness = m_vd->GetBrightness();
		break;
	case 5:
		if (m_group)
			brightness = m_group->GetBrightness();
		break;
	}
	for (int j : {0, 1, 2})
		if (j != i) brightness[j] = brightness[i];
	switch (m_type)
	{
	case 1:
		if (m_view)
			m_view->SetBrightness(brightness);
		break;
	case 2:
		if (m_vd)
			m_vd->SetBrightness(brightness);
		if (m_link_group && m_group)
			m_group->SetBrightnessAll(brightness);
		break;
	case 3:
		if (m_group)
			m_group->SetBrightness(brightness);
		if (m_link_group && m_group)
			m_group->SetBrightnessAll(brightness);
	}

	fluo::ValueCollection vc;
	if (i != 0)
		vc.insert(gstBrightnessR);
	if (i != 1)
		vc.insert(gstBrightnessG);
	if (i != 2)
		vc.insert(gstBrightnessB);
	FluoRefresh(false, true, false, vc);
}

void AdjustView::SyncHdr(int i)
{
	fluo::Color hdr;
	switch (m_type)
	{
	case 1:
		if (m_view)
			hdr = m_view->GetHdr();
		break;
	case 2:
		if (m_vd)
			hdr = m_vd->GetHdr();
		break;
	case 5:
		if (m_group)
			hdr = m_group->GetHdr();
		break;
	}
	for (int j : {0, 1, 2})
		if (j != i) hdr[j] = hdr[i];
	switch (m_type)
	{
	case 1:
		if (m_view)
			m_view->SetHdr(hdr);
		break;
	case 2:
		if (m_vd)
			m_vd->SetHdr(hdr);
		if (m_link_group && m_group)
			m_group->SetHdrAll(hdr);
		break;
	case 3:
		if (m_group)
			m_group->SetHdr(hdr);
		if (m_link_group && m_group)
			m_group->SetHdrAll(hdr);
	}

	fluo::ValueCollection vc;
	if (i != 0)
		vc.insert(gstEqualizeR);
	if (i != 1)
		vc.insert(gstEqualizeG);
	if (i != 2)
		vc.insert(gstEqualizeB);
	FluoRefresh(false, true, false, vc);
}

void AdjustView::SetSync(int i, bool val, bool update)
{
	m_sync[i] = val;
	fluo::Color gamma, brightness, hdr;

	switch (m_type)
	{
	case 1://view
		if (m_view)
		{
			m_view->SetSync(i, val);

			if (val)
			{
				gamma = m_view->GetGammaColor();
				brightness = m_view->GetBrightness();
				hdr = m_view->GetHdr();
				SyncColor(gamma, gamma[i]);
				SyncColor(brightness, brightness[i]);
				SyncColor(hdr, hdr[i]);
				m_view->SetGammaColor(gamma);
				m_view->SetBrightness(brightness);
				m_view->SetHdr(hdr);
			}
		}
		break;
	case 2://volume data
		if (m_vd)
		{
			m_vd->SetSync(i, val);
			if (m_link_group && m_group)
				m_group->SetSyncAll(i, val);
			if (val)
			{
				gamma = m_vd->GetGammaColor();
				brightness = m_vd->GetBrightness();
				hdr = m_vd->GetHdr();
				SyncColor(gamma, gamma[i]);
				SyncColor(brightness, brightness[i]);
				SyncColor(hdr, hdr[i]);
				m_vd->SetGammaColor(gamma);
				m_vd->SetBrightness(brightness);
				m_vd->SetHdr(hdr);
				if (m_link_group && m_group)
				{
					m_group->SetGammaAll(gamma);
					m_group->SetBrightnessAll(brightness);
					m_group->SetHdrAll(hdr);
				}
			}
		}
		break;
	case 5://group
		if (m_group)
		{
			m_group->SetSync(i, val);
			if (m_link_group)
				m_group->SetSyncAll(i, val);
			if (val)
			{
				gamma = m_group->GetGammaColor();
				brightness = m_group->GetBrightness();
				hdr = m_group->GetHdr();
				SyncColor(gamma, gamma[i]);
				SyncColor(brightness, brightness[i]);
				SyncColor(hdr, hdr[i]);
				m_group->SetGammaColor(gamma);
				m_group->SetBrightness(brightness);
				m_group->SetHdr(hdr);
				if (m_link_group)
				{
					m_group->SetGammaAll(gamma);
					m_group->SetBrightnessAll(brightness);
					m_group->SetHdrAll(hdr);
				}
			}
		}
		break;
	}

	if (update)
	{
		fluo::ValueCollection vc;
		if (m_sync[0])
			vc.insert(gstSyncR);
		if (m_sync[1])
			vc.insert(gstSyncG);
		if (m_sync[2])
			vc.insert(gstSyncB);

		FluoRefresh(false, true, false, vc);
	}
}

void AdjustView::SetGamma(int i, double val, bool update)
{
	fluo::Color gamma;
	switch (m_type)
	{
	case 1://view
		if (m_view)
			gamma = m_view->GetGammaColor();
		break;
	case 2://volume
		if (m_vd)
			gamma = m_vd->GetGammaColor();
		break;
	case 5://group
		if (m_group)
			gamma = m_group->GetGammaColor();
		break;
	}
	fluo::ValueCollection vc;
	SyncGamma(gamma, i, val, vc);
	switch (m_type)
	{
	case 1:
		if (m_view)
			m_view->SetGammaColor(gamma);
		break;
	case 2:
		if (m_vd)
			m_vd->SetGammaColor(gamma);
		if (m_link_group && m_group)
			m_group->SetGammaAll(gamma);
		break;
	case 3:
		if (m_group)
			m_group->SetGammaColor(gamma);
		if (m_link_group && m_group)
			m_group->SetGammaAll(gamma);
	}

	if (update)
		FluoRefresh(false, true, false, vc);
}

void AdjustView::SetBrightness(int i, double val, bool update)
{
	fluo::Color brightness;
	switch (m_type)
	{
	case 1://view
		if (m_view)
			brightness = m_view->GetBrightness();
		break;
	case 2://volume
		if (m_vd)
			brightness = m_vd->GetBrightness();
		break;
	case 5://group
		if (m_group)
			brightness = m_group->GetBrightness();
		break;
	}
	fluo::ValueCollection vc;
	SyncBrightness(brightness, i, val, vc);
	switch (m_type)
	{
	case 1:
		if (m_view)
			m_view->SetBrightness(brightness);
		break;
	case 2:
		if (m_vd)
			m_vd->SetBrightness(brightness);
		if (m_link_group && m_group)
			m_group->SetBrightnessAll(brightness);
		break;
	case 3:
		if (m_group)
			m_group->SetBrightness(brightness);
		if (m_link_group && m_group)
			m_group->SetBrightnessAll(brightness);
	}

	if (update)
		FluoRefresh(false, true, false, vc);
}

void AdjustView::SetHdr(int i, double val, bool update)
{
	fluo::Color hdr;
	switch (m_type)
	{
	case 1://view
		if (m_view)
			hdr = m_view->GetHdr();
		break;
	case 2://volume
		if (m_vd)
			hdr = m_vd->GetHdr();
		break;
	case 5://group
		if (m_group)
			hdr = m_group->GetHdr();
		break;
	}
	fluo::ValueCollection vc;
	SyncHdr(hdr, i, val, vc);
	switch (m_type)
	{
	case 1:
		if (m_view)
			m_view->SetHdr(hdr);
		break;
	case 2:
		if (m_vd)
			m_vd->SetHdr(hdr);
		if (m_link_group && m_group)
			m_group->SetHdrAll(hdr);
		break;
	case 3:
		if (m_group)
			m_group->SetHdr(hdr);
		if (m_link_group && m_group)
			m_group->SetHdrAll(hdr);
	}

	if (update)
		FluoRefresh(false, true, false, vc);
}

void AdjustView::UpdateSync()
{
	int i;
	int cnt;
	bool r_v = false;
	bool g_v = false;
	bool b_v = false;

	if ((m_type == 2 && m_link_group && m_group) ||
		(m_type == 5 && m_group))
	{
		//use group
		for (i=0; i<m_group->GetVolumeNum(); i++)
		{
			VolumeData* vd = m_group->GetVolumeData(i);
			if (vd)
			{
				if (vd->GetColormapMode())
				{
					r_v = g_v = b_v = true;
				}
				else
				{
					fluo::Color c = vd->GetColor();
					bool r, g, b;
					r = g = b = false;
					cnt = 0;
					if (c.r()>0) {cnt++; r=true;}
					if (c.g()>0) {cnt++; g=true;}
					if (c.b()>0) {cnt++; b=true;}

					if (cnt > 1)
					{
						r_v = r_v||r;
						g_v = g_v||g;
						b_v = b_v||b;
					}
				}
			}
		}
		cnt = 0;
		if (r_v) cnt++;
		if (g_v) cnt++;
		if (b_v) cnt++;

		SetSync(0, r_v, false);
		SetSync(1, g_v, false);
		SetSync(2, b_v, cnt > 1 ? false : true);

		if (cnt > 1)
		{
			double gamma = 1.0, brightness = 1.0, hdr = 0.0;
			if (r_v)
			{
				gamma = m_group->GetGammaColor().r();
				brightness = m_group->GetBrightness().r();
				hdr = m_group->GetHdr().r();
			}
			else if (g_v)
			{
				gamma = m_group->GetGammaColor().g();
				brightness = m_group->GetBrightness().g();
				hdr = m_group->GetHdr().g();
			}

			if (g_v)
			{
				SetGamma(1, gamma, false);
				SetBrightness(1, brightness, false);
				SetHdr(1, hdr, b_v ? false : true);
			}
			if (b_v)
			{
				SetGamma(2, gamma, false);
				SetBrightness(2, brightness, false);
				SetHdr(2, hdr, true);
			}
		}
	}
	else if (m_type == 2 && !m_link_group && m_vd)
	{
		//use volume
	}
	else if (m_type == 1 && m_view)
	{
		//means this is depth mode
		if (m_view->GetVolMethod() != VOL_METHOD_MULTI)
			return;
		
		for (i=0; i<m_view->GetDispVolumeNum(); i++)
		{
			VolumeData* vd = m_view->GetDispVolumeData(i);
			if (vd)
			{
				if (vd->GetColormapMode())
				{
					r_v = g_v = b_v = true;
				}
				else
				{
					fluo::Color c = vd->GetColor();
					bool r, g, b;
					r = g = b = false;
					cnt = 0;
					if (c.r()>0) {cnt++; r = true;}
					if (c.g()>0) {cnt++; g = true;}
					if (c.b()>0) {cnt++; b = true;}

					if (cnt > 1)
					{
						r_v = r_v||r;
						g_v = g_v||g;
						b_v = b_v||b;
					}
				}
			}
		}

		cnt = 0;

		if (r_v) cnt++;
		if (g_v) cnt++;
		if (b_v) cnt++;

		SetSync(0, r_v, false);
		SetSync(1, g_v, false);
		SetSync(2, b_v, cnt > 1 ? false : true);

		if (cnt > 1)
		{
			double gamma = 1.0, brightness = 1.0, hdr = 0.0;
			if (r_v)
			{
				gamma = m_view->GetGammaColor().r();
				brightness = m_view->GetBrightness().r();
				hdr = m_view->GetHdr().r();
			}
			else if (g_v)
			{
				gamma = m_view->GetGammaColor().g();
				brightness = m_view->GetBrightness().g();
				hdr = m_view->GetHdr().g();
			}

			if (g_v)
			{
				SetGamma(1, gamma, false);
				SetBrightness(1, brightness, false);
				SetHdr(1, hdr, b_v ? false : true);
			}
			if (b_v)
			{
				SetGamma(2, gamma, false);
				SetBrightness(2, brightness, false);
				SetHdr(2, hdr, true);
			}
		}
	}

}

void AdjustView::OnRReset(wxCommandEvent &event)
{
	SetGamma(0, glbin_outadj_def.m_gamma_r, false);
	SetBrightness(0, glbin_outadj_def.m_brightness_r, false);
	SetHdr(0, glbin_outadj_def.m_hdr_r);
}

void AdjustView::OnGReset(wxCommandEvent &event)
{
	SetGamma(1, glbin_outadj_def.m_gamma_g, false);
	SetBrightness(1, glbin_outadj_def.m_brightness_g, false);
	SetHdr(1, glbin_outadj_def.m_hdr_g);
}

void AdjustView::OnBReset(wxCommandEvent &event)
{
	SetGamma(2, glbin_outadj_def.m_gamma_b, false);
	SetBrightness(2, glbin_outadj_def.m_brightness_b, false);
	SetHdr(2, glbin_outadj_def.m_hdr_b);
}

