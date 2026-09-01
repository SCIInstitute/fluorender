/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2026 Scientific Computing and Imaging Institute,
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
#include <OutputAdjPanel.h>
#include <Global.h>
#include <Names.h>
#include <MainSettings.h>
#include <MainFrame.h>
#include <RenderView.h>
#include <CurrentObjects.h>
#include <VolumeData.h>
#include <VolumeGroup.h>
#include <Color.h>
#include <ShaderProgram.h>
#include <wxFadeButton.h>
#include <wxSingleSlider.h>
#include <wxUndoableToolbar.h>
#include <wx/valnum.h>
#include <wx/gbsizer.h>
#include <png_resource.h>
#include <icons.h>

OutputAdjPanel::OutputAdjPanel(MainFrame* frame,
					   const wxPoint& pos,
					   const wxSize& size,
					   long style,
					   const wxString& name) :
	TabbedPanel(frame, pos, size, style, name)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);
	Freeze();
	SetDoubleBuffered(true);

	//notebook
	m_notebook = new wxAuiNotebook(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize,
		wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE |
		wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_TAB_EXTERNAL_MOVE | wxNO_BORDER);
	wxSize s = wxSize(size.x, size.y / 3);
	m_notebook->AddPage(CreateRedPage(m_notebook, s), "Red", true);
	m_notebook->AddPage(CreateGreenPage(m_notebook, s), "Green");
	m_notebook->AddPage(CreateBluePage(m_notebook, s), "Blue");

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

	m_dft_btn = new wxButton(this, wxID_ANY, "Set Default",
							 wxDefaultPosition, FromDIP(wxSize(95, 22)));
	m_dft_btn->SetBitmap(wxGetBitmap(save_settings));
	m_dft_btn->Bind(wxEVT_BUTTON, &OutputAdjPanel::OnSaveDefault, this);

	sizer->Add(m_notebook, 1, wxEXPAND);
	sizer->Add(m_dft_btn, 0, wxEXPAND);
	SetSizer(sizer);
	Layout();
	SetAutoLayout(true);
	SetScrollRate(10, 10);

	EnableAll(false);

	//add sliders for undo and redo
	glbin.add_undo_control(m_sync_r_chk);
	glbin.add_undo_control(m_r_gamma_sldr);
	glbin.add_undo_control(m_r_brightness_sldr);
	glbin.add_undo_control(m_r_hdr_sldr);
	glbin.add_undo_control(m_sync_g_chk);
	glbin.add_undo_control(m_g_gamma_sldr);
	glbin.add_undo_control(m_g_brightness_sldr);
	glbin.add_undo_control(m_g_hdr_sldr);
	glbin.add_undo_control(m_sync_b_chk);
	glbin.add_undo_control(m_b_gamma_sldr);
	glbin.add_undo_control(m_b_brightness_sldr);
	glbin.add_undo_control(m_b_hdr_sldr);

	Thaw();
}

OutputAdjPanel::~OutputAdjPanel()
{
	//delete sliders for undo and redo
	glbin.del_undo_control(m_sync_r_chk);
	glbin.del_undo_control(m_r_gamma_sldr);
	glbin.del_undo_control(m_r_brightness_sldr);
	glbin.del_undo_control(m_r_hdr_sldr);
	glbin.del_undo_control(m_sync_g_chk);
	glbin.del_undo_control(m_g_gamma_sldr);
	glbin.del_undo_control(m_g_brightness_sldr);
	glbin.del_undo_control(m_g_hdr_sldr);
	glbin.del_undo_control(m_sync_b_chk);
	glbin.del_undo_control(m_b_gamma_sldr);
	glbin.del_undo_control(m_b_brightness_sldr);
	glbin.del_undo_control(m_b_hdr_sldr);

	//m_frame->SetFocusVRenderViews(0);
}

wxWindow* OutputAdjPanel::CreateRedPage(wxWindow* parent, wxSize& size)
{
	wxScrolledWindow* page = new wxScrolledWindow(parent);
	SetSize(size);

	long ls = glbin_settings.m_inverse_slider ? wxSL_VERTICAL : (wxSL_VERTICAL | wxSL_INVERSE);
	//validator: floating point 2
	wxFloatingPointValidator<double> vald_fp2(2);
	//validator: integer
	wxIntegerValidator<int> vald_int;
	vald_int.SetRange(-256, 256);
	wxBitmapBundle bitmap;

	wxGridBagSizer* sizer_v = new wxGridBagSizer(5, 0);
	//multifunc buttons
	m_r_gamma_st = new wxFadeButton(page, wxID_ANY, "Gam.R.",
		wxDefaultPosition, FromDIP(wxSize(30, 20)));
	m_r_gamma_st->Bind(wxEVT_BUTTON, &OutputAdjPanel::OnRGammaMF, this);
	m_r_brightness_st = new wxFadeButton(page, wxID_ANY, "Lum.R.",
		wxDefaultPosition, FromDIP(wxSize(30, 20)));
	m_r_brightness_st->Bind(wxEVT_BUTTON, &OutputAdjPanel::OnRBrightnessMF, this);
	m_r_hdr_st = new wxFadeButton(page, wxID_ANY, "Eql.R.",
		wxDefaultPosition, FromDIP(wxSize(30, 20)));
	m_r_gamma_st->SetFontBold();
	m_r_brightness_st->SetFontBold();
	m_r_hdr_st->SetFontBold();
	m_r_hdr_st->Bind(wxEVT_BUTTON, &OutputAdjPanel::OnRHdrMF, this);
	sizer_v->Add(m_r_gamma_st, wxGBPosition(0, 0), wxGBSpan(1, 1), wxEXPAND);
	sizer_v->Add(m_r_brightness_st, wxGBPosition(0, 1), wxGBSpan(1, 1), wxEXPAND);
	sizer_v->Add(m_r_hdr_st, wxGBPosition(0, 2), wxGBSpan(1, 1), wxEXPAND);

	//sliders
	m_r_gamma_sldr = new wxSingleSlider(page, wxID_ANY, 100, 10, 400,
		wxDefaultPosition, wxDefaultSize, ls);
	m_r_gamma_sldr->Bind(wxEVT_SCROLL_CHANGED, &OutputAdjPanel::OnRGammaChange, this);
	m_r_gamma_sldr->SetRangeColor(*wxRED);
	m_r_gamma_sldr->SetHistoryIndicator(m_r_gamma_st);
	m_r_brightness_sldr = new wxSingleSlider(page, wxID_ANY, 0, -256, 256,
		wxDefaultPosition, wxDefaultSize, ls);
	m_r_brightness_sldr->SetRangeStyle(2);
	m_r_brightness_sldr->SetRangeColor(*wxRED);
	m_r_brightness_sldr->Bind(wxEVT_SCROLL_CHANGED, &OutputAdjPanel::OnRBrightnessChange, this);
	m_r_brightness_sldr->SetHistoryIndicator(m_r_brightness_st);
	m_r_hdr_sldr = new wxSingleSlider(page, wxID_ANY, 0, 0, 100,
		wxDefaultPosition, wxDefaultSize, ls);
	m_r_hdr_sldr->SetRangeColor(*wxRED);
	m_r_hdr_sldr->Bind(wxEVT_SCROLL_CHANGED, &OutputAdjPanel::OnRHdrChange, this);
	m_r_hdr_sldr->SetHistoryIndicator(m_r_hdr_st);
	sizer_v->Add(m_r_gamma_sldr, wxGBPosition(1, 0), wxGBSpan(1, 1), wxEXPAND);
	sizer_v->Add(m_r_brightness_sldr, wxGBPosition(1, 1), wxGBSpan(1, 1), wxEXPAND);
	sizer_v->Add(m_r_hdr_sldr, wxGBPosition(1, 2), wxGBSpan(1, 1), wxEXPAND);

	//input boxes
	vald_fp2.SetRange(0.0, 10.0);
	m_r_gamma_text = new wxTextCtrl(page, wxID_ANY, "1.00",
		wxDefaultPosition, FromDIP(wxSize(30, 20)), wxTE_CENTER, vald_fp2);
	m_r_gamma_text->Bind(wxEVT_TEXT, &OutputAdjPanel::OnRGammaText, this);
	m_r_brightness_text = new wxTextCtrl(page, wxID_ANY, "0",
		wxDefaultPosition, FromDIP(wxSize(30, 20)), wxTE_CENTER, vald_int);
	m_r_brightness_text->Bind(wxEVT_TEXT, &OutputAdjPanel::OnRBrightnessText, this);
	vald_fp2.SetRange(0.0, 1.0);
	m_r_hdr_text = new wxTextCtrl(page, wxID_ANY, "0.00",
		wxDefaultPosition, FromDIP(wxSize(30, 20)), wxTE_CENTER, vald_fp2);
	m_r_hdr_text->Bind(wxEVT_TEXT, &OutputAdjPanel::OnRHdrText, this);
	sizer_v->Add(m_r_gamma_text, wxGBPosition(2, 0), wxGBSpan(1, 1), wxEXPAND);
	sizer_v->Add(m_r_brightness_text, wxGBPosition(2, 1), wxGBSpan(1, 1), wxEXPAND);
	sizer_v->Add(m_r_hdr_text, wxGBPosition(2, 2), wxGBSpan(1, 1), wxEXPAND);

	//reset buttons
	m_r_reset_btn = new wxButton(page, wxID_ANY, "Reset",
		wxDefaultPosition, FromDIP(wxSize(30, 22)));
	m_r_reset_btn->SetBitmap(wxGetBitmap(reset));
	m_r_reset_btn->Bind(wxEVT_BUTTON, &OutputAdjPanel::OnRReset, this);
	m_sync_r_chk = new wxUndoableToolbar(page, 0,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	bitmap = wxGetBitmap(unlink);
	m_sync_r_chk->AddCheckTool(0, "Link",
		bitmap, wxNullBitmap,
		"Link Red Properties with Linked Green or Blue",
		"Link Red Properties with Linked Green or Blue");
	m_sync_r_chk->Bind(wxEVT_TOOL, &OutputAdjPanel::OnSyncRCheck, this);
	m_sync_r_chk->Realize();
	sizer_v->Add(m_r_reset_btn, wxGBPosition(3, 0), wxGBSpan(1, 2), wxEXPAND);
	sizer_v->Add(m_sync_r_chk, wxGBPosition(3, 2), wxGBSpan(1, 1), wxEXPAND|wxALIGN_RIGHT);

	sizer_v->AddGrowableCol(0);
	sizer_v->AddGrowableCol(1);
	sizer_v->AddGrowableCol(2);
	sizer_v->AddGrowableRow(1);
	page->SetSizer(sizer_v);
	page->SetAutoLayout(true);
	page->SetScrollRate(10, 10);
	return page;
}

wxWindow* OutputAdjPanel::CreateGreenPage(wxWindow* parent, wxSize& size)
{
	wxScrolledWindow* page = new wxScrolledWindow(parent);
	SetSize(size);

	long ls = glbin_settings.m_inverse_slider ? wxSL_VERTICAL : (wxSL_VERTICAL | wxSL_INVERSE);
	//validator: floating point 2
	wxFloatingPointValidator<double> vald_fp2(2);
	//validator: integer
	wxIntegerValidator<int> vald_int;
	vald_int.SetRange(-256, 256);
	wxBitmapBundle bitmap;

	wxGridBagSizer* sizer_v = new wxGridBagSizer(5, 0);
	//buttons
	m_g_gamma_st = new wxFadeButton(page, wxID_ANY, "Gam.G.",
		wxDefaultPosition, FromDIP(wxSize(30, 20)));
	m_g_gamma_st->Bind(wxEVT_BUTTON, &OutputAdjPanel::OnGGammaMF, this);
	m_g_brightness_st = new wxFadeButton(page, wxID_ANY, "Lum.G.",
		wxDefaultPosition, FromDIP(wxSize(30, 20)));
	m_g_brightness_st->Bind(wxEVT_BUTTON, &OutputAdjPanel::OnGBrightnessMF, this);
	m_g_hdr_st = new wxFadeButton(page, wxID_ANY, "Eql.G.",
		wxDefaultPosition, FromDIP(wxSize(30, 20)));
	m_g_hdr_st->Bind(wxEVT_BUTTON, &OutputAdjPanel::OnGHdrMF, this);
	m_g_gamma_st->SetFontBold();
	m_g_brightness_st->SetFontBold();
	m_g_hdr_st->SetFontBold();
	sizer_v->Add(m_g_gamma_st, wxGBPosition(0, 0), wxGBSpan(1, 1), wxEXPAND);
	sizer_v->Add(m_g_brightness_st, wxGBPosition(0, 1), wxGBSpan(1, 1), wxEXPAND);
	sizer_v->Add(m_g_hdr_st, wxGBPosition(0, 2), wxGBSpan(1, 1), wxEXPAND);

	//sliders
	m_g_gamma_sldr = new wxSingleSlider(page, wxID_ANY, 100, 10, 400,
		wxDefaultPosition, wxDefaultSize, ls);
	m_g_gamma_sldr->Bind(wxEVT_SCROLL_CHANGED, &OutputAdjPanel::OnGGammaChange, this);
	m_g_gamma_sldr->SetRangeColor(*wxGREEN);
	m_g_gamma_sldr->SetHistoryIndicator(m_g_gamma_st);
	m_g_brightness_sldr = new wxSingleSlider(page, wxID_ANY, 0, -256, 256,
		wxDefaultPosition, wxDefaultSize, ls);
	m_g_brightness_sldr->SetRangeStyle(2);
	m_g_brightness_sldr->SetRangeColor(*wxGREEN);
	m_g_brightness_sldr->Bind(wxEVT_SCROLL_CHANGED, &OutputAdjPanel::OnGBrightnessChange, this);
	m_g_brightness_sldr->SetHistoryIndicator(m_g_brightness_st);
	m_g_hdr_sldr = new wxSingleSlider(page, wxID_ANY, 0, 0, 100,
		wxDefaultPosition, wxDefaultSize, ls);
	m_g_hdr_sldr->SetRangeColor(*wxGREEN);
	m_g_hdr_sldr->Bind(wxEVT_SCROLL_CHANGED, &OutputAdjPanel::OnGHdrChange, this);
	m_g_hdr_sldr->SetHistoryIndicator(m_g_hdr_st);
	sizer_v->Add(m_g_gamma_sldr, wxGBPosition(1, 0), wxGBSpan(1, 1), wxEXPAND);
	sizer_v->Add(m_g_brightness_sldr, wxGBPosition(1, 1), wxGBSpan(1, 1), wxEXPAND);
	sizer_v->Add(m_g_hdr_sldr, wxGBPosition(1, 2), wxGBSpan(1, 1), wxEXPAND);

	//input boxes
	vald_fp2.SetRange(0.0, 10.0);
	m_g_gamma_text = new wxTextCtrl(page, wxID_ANY, "1.00",
		wxDefaultPosition, FromDIP(wxSize(30, 20)), wxTE_CENTER, vald_fp2);
	m_g_gamma_text->Bind(wxEVT_TEXT, &OutputAdjPanel::OnGGammaText, this);
	m_g_brightness_text = new wxTextCtrl(page, wxID_ANY, "0",
		wxDefaultPosition, FromDIP(wxSize(30, 20)), wxTE_CENTER, vald_int);
	m_g_brightness_text->Bind(wxEVT_TEXT, &OutputAdjPanel::OnGBrightnessText, this);
	vald_fp2.SetRange(0.0, 1.0);
	m_g_hdr_text = new wxTextCtrl(page, wxID_ANY, "0.00",
		wxDefaultPosition, FromDIP(wxSize(30, 20)), wxTE_CENTER, vald_fp2);
	m_g_hdr_text->Bind(wxEVT_TEXT, &OutputAdjPanel::OnGHdrText, this);
	sizer_v->Add(m_g_gamma_text, wxGBPosition(2, 0), wxGBSpan(1, 1), wxEXPAND);
	sizer_v->Add(m_g_brightness_text, wxGBPosition(2, 1), wxGBSpan(1, 1), wxEXPAND);
	sizer_v->Add(m_g_hdr_text, wxGBPosition(2, 2), wxGBSpan(1, 1), wxEXPAND);

	//reset buttons
	m_g_reset_btn = new wxButton(page, wxID_ANY, "Reset",
		wxDefaultPosition, FromDIP(wxSize(30, 22)));
	m_g_reset_btn->SetBitmap(wxGetBitmap(reset));
	m_g_reset_btn->Bind(wxEVT_BUTTON, &OutputAdjPanel::OnGReset, this);
	m_sync_g_chk = new wxUndoableToolbar(page, 0,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	bitmap = wxGetBitmap(unlink);
	m_sync_g_chk->AddCheckTool(0, "Link",
		bitmap, wxNullBitmap,
		"Link Green Properties with Linked Red or Blue",
		"Link Green Properties with Linked Red or Blue");
	m_sync_g_chk->Bind(wxEVT_TOOL, &OutputAdjPanel::OnSyncGCheck, this);
	m_sync_g_chk->Realize();
	sizer_v->Add(m_g_reset_btn, wxGBPosition(3, 0), wxGBSpan(1, 2), wxEXPAND);
	sizer_v->Add(m_sync_g_chk, wxGBPosition(3, 2), wxGBSpan(1, 1), wxEXPAND | wxALIGN_RIGHT);

	sizer_v->AddGrowableCol(0);
	sizer_v->AddGrowableCol(1);
	sizer_v->AddGrowableCol(2);
	sizer_v->AddGrowableRow(1);
	page->SetSizer(sizer_v);
	page->SetAutoLayout(true);
	page->SetScrollRate(10, 10);
	return page;
}

wxWindow* OutputAdjPanel::CreateBluePage(wxWindow* parent, wxSize& size)
{
	wxScrolledWindow* page = new wxScrolledWindow(parent);
	SetSize(size);

	long ls = glbin_settings.m_inverse_slider ? wxSL_VERTICAL : (wxSL_VERTICAL | wxSL_INVERSE);
	//validator: floating point 2
	wxFloatingPointValidator<double> vald_fp2(2);
	//validator: integer
	wxIntegerValidator<int> vald_int;
	vald_int.SetRange(-256, 256);
	wxBitmapBundle bitmap;

	wxGridBagSizer* sizer_v = new wxGridBagSizer(5, 0);
	//multifunc buttons
	m_b_gamma_st = new wxFadeButton(page, wxID_ANY, "Gam.B.",
		wxDefaultPosition, FromDIP(wxSize(30, 20)));
	m_b_gamma_st->Bind(wxEVT_BUTTON, &OutputAdjPanel::OnBGammaMF, this);
	m_b_brightness_st = new wxFadeButton(page, wxID_ANY, "Lum.B.",
		wxDefaultPosition, FromDIP(wxSize(30, 20)));
	m_b_brightness_st->Bind(wxEVT_BUTTON, &OutputAdjPanel::OnBBrightnessMF, this);
	m_b_hdr_st = new wxFadeButton(page, wxID_ANY, "Eql.B.",
		wxDefaultPosition, FromDIP(wxSize(30, 20)));
	m_b_hdr_st->Bind(wxEVT_BUTTON, &OutputAdjPanel::OnBHdrMF, this);
	m_b_gamma_st->SetFontBold();
	m_b_brightness_st->SetFontBold();
	m_b_hdr_st->SetFontBold();
	sizer_v->Add(m_b_gamma_st, wxGBPosition(0, 0), wxGBSpan(1, 1), wxEXPAND);
	sizer_v->Add(m_b_brightness_st, wxGBPosition(0, 1), wxGBSpan(1, 1), wxEXPAND);
	sizer_v->Add(m_b_hdr_st, wxGBPosition(0, 2), wxGBSpan(1, 1), wxEXPAND);

	//sliders
	m_b_gamma_sldr = new wxSingleSlider(page, wxID_ANY, 100, 10, 400,
		wxDefaultPosition, wxDefaultSize, ls);
	m_b_gamma_sldr->SetRangeColor(*wxBLUE);
	m_b_gamma_sldr->Bind(wxEVT_SCROLL_CHANGED, &OutputAdjPanel::OnBGammaChange, this);
	m_b_gamma_sldr->SetHistoryIndicator(m_b_gamma_st);
	m_b_brightness_sldr = new wxSingleSlider(page, wxID_ANY, 0, -256, 256,
		wxDefaultPosition, wxDefaultSize, ls);
	m_b_brightness_sldr->SetRangeStyle(2);
	m_b_brightness_sldr->SetRangeColor(*wxBLUE);
	m_b_brightness_sldr->Bind(wxEVT_SCROLL_CHANGED, &OutputAdjPanel::OnBBrightnessChange, this);
	m_b_brightness_sldr->SetHistoryIndicator(m_b_brightness_st);
	m_b_hdr_sldr = new wxSingleSlider(page, wxID_ANY, 0, 0, 100,
		wxDefaultPosition, wxDefaultSize, ls);
	m_b_hdr_sldr->SetRangeColor(*wxBLUE);
	m_b_hdr_sldr->Bind(wxEVT_SCROLL_CHANGED, &OutputAdjPanel::OnBHdrChange, this);
	m_b_hdr_sldr->SetHistoryIndicator(m_b_hdr_st);
	sizer_v->Add(m_b_gamma_sldr, wxGBPosition(1, 0), wxGBSpan(1, 1), wxEXPAND);
	sizer_v->Add(m_b_brightness_sldr, wxGBPosition(1, 1), wxGBSpan(1, 1), wxEXPAND);
	sizer_v->Add(m_b_hdr_sldr, wxGBPosition(1, 2), wxGBSpan(1, 1), wxEXPAND);

	//input boxes
	vald_fp2.SetRange(0.0, 10.0);
	m_b_gamma_text = new wxTextCtrl(page, wxID_ANY, "1.00",
		wxDefaultPosition, FromDIP(wxSize(30, 20)), wxTE_CENTER, vald_fp2);
	m_b_gamma_text->Bind(wxEVT_TEXT, &OutputAdjPanel::OnBGammaText, this);
	m_b_brightness_text = new wxTextCtrl(page, wxID_ANY, "0",
		wxDefaultPosition, FromDIP(wxSize(30, 20)), wxTE_CENTER, vald_int);
	m_b_brightness_text->Bind(wxEVT_TEXT, &OutputAdjPanel::OnBBrightnessText, this);
	vald_fp2.SetRange(0.0, 1.0);
	m_b_hdr_text = new wxTextCtrl(page, wxID_ANY, "0.00",
		wxDefaultPosition, FromDIP(wxSize(30, 20)), wxTE_CENTER, vald_fp2);
	m_b_hdr_text->Bind(wxEVT_TEXT, &OutputAdjPanel::OnBHdrText, this);
	sizer_v->Add(m_b_gamma_text, wxGBPosition(2, 0), wxGBSpan(1, 1), wxEXPAND);
	sizer_v->Add(m_b_brightness_text, wxGBPosition(2, 1), wxGBSpan(1, 1), wxEXPAND);
	sizer_v->Add(m_b_hdr_text, wxGBPosition(2, 2), wxGBSpan(1, 1), wxEXPAND);

	//reset buttons
	m_b_reset_btn = new wxButton(page, wxID_ANY, "Reset",
		wxDefaultPosition, FromDIP(wxSize(30, 22)));
	m_b_reset_btn->SetBitmap(wxGetBitmap(reset));
	m_b_reset_btn->Bind(wxEVT_BUTTON, &OutputAdjPanel::OnBReset, this);
	m_sync_b_chk = new wxUndoableToolbar(page, 0,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	bitmap = wxGetBitmap(unlink);
	m_sync_b_chk->AddCheckTool(0, "Link",
		bitmap, wxNullBitmap,
		"Link Blue Properties with Linked Red or Green",
		"Link Blue Properties with Linked Red or Green");
	m_sync_b_chk->Bind(wxEVT_TOOL, &OutputAdjPanel::OnSyncBCheck, this);
	m_sync_b_chk->Realize();
	sizer_v->Add(m_b_reset_btn, wxGBPosition(3, 0), wxGBSpan(1, 2), wxEXPAND);
	sizer_v->Add(m_sync_b_chk, wxGBPosition(3, 2), wxGBSpan(1, 1), wxEXPAND | wxALIGN_RIGHT);

	sizer_v->AddGrowableCol(0);
	sizer_v->AddGrowableCol(1);
	sizer_v->AddGrowableCol(2);
	sizer_v->AddGrowableRow(1);
	page->SetSizer(sizer_v);
	page->SetAutoLayout(true);
	page->SetScrollRate(10, 10);
	return page;
}

void OutputAdjPanel::LoadPerspective(const std::string& str)
{
	if (!fluo::InEpsilon(glbin_settings.m_dpi_scale_factor,
		GetDPIScaleFactor()) || str.empty())
	{
		m_notebook->Split(1, wxBOTTOM);
		m_notebook->Split(2, wxBOTTOM);
		m_notebook->SetCalculateMoveSplitSize(true);
		return;
	}
	TabbedPanel::LoadPerspective(str);
}

void OutputAdjPanel::UpdateMultiFuncTips(int ival)
{
	switch (ival)
	{
	case 0:
		m_r_gamma_st->SetToolTip("Synchronize the gamma values of RGB channels");
		m_g_gamma_st->SetToolTip("Synchronize the gamma values of RGB channels");
		m_b_gamma_st->SetToolTip("Synchronize the gamma values of RGB channels");
		m_r_brightness_st->SetToolTip("Synchronize the brightness values of RGB channels");
		m_g_brightness_st->SetToolTip("Synchronize the brightness values of RGB channels");
		m_b_brightness_st->SetToolTip("Synchronize the brightness values of RGB channels");
		m_r_hdr_st->SetToolTip("Synchronize the equalization values of RGB channels");
		m_g_hdr_st->SetToolTip("Synchronize the equalization values of RGB channels");
		m_b_hdr_st->SetToolTip("Synchronize the equalization values of RGB channels");
		break;
	case 1:
		m_r_gamma_st->SetToolTip("Move the mouse cursor in render view and change the gamma value using the mouse wheel");
		m_g_gamma_st->SetToolTip("Move the mouse cursor in render view and change the gamma value using the mouse wheel");
		m_b_gamma_st->SetToolTip("Move the mouse cursor in render view and change the gamma value using the mouse wheel");
		m_r_brightness_st->SetToolTip("Move the mouse cursor in render view and change the brightness value using the mouse wheel");
		m_g_brightness_st->SetToolTip("Move the mouse cursor in render view and change the brightness value using the mouse wheel");
		m_b_brightness_st->SetToolTip("Move the mouse cursor in render view and change the brightness value using the mouse wheel");
		m_r_hdr_st->SetToolTip("Move the mouse cursor in render view and change the equalization value using the mouse wheel");
		m_g_hdr_st->SetToolTip("Move the mouse cursor in render view and change the equalization value using the mouse wheel");
		m_b_hdr_st->SetToolTip("Move the mouse cursor in render view and change the equalization value using the mouse wheel");
		break;
	case 2:
		m_r_gamma_st->SetToolTip("Reset the gamma value");
		m_g_gamma_st->SetToolTip("Reset the gamma value");
		m_b_gamma_st->SetToolTip("Reset the gamma value");
		m_r_brightness_st->SetToolTip("Reset the brightness value");
		m_g_brightness_st->SetToolTip("Reset the brightness value");
		m_b_brightness_st->SetToolTip("Reset the brightness value");
		m_r_hdr_st->SetToolTip("Reset the eqaualization value");
		m_g_hdr_st->SetToolTip("Reset the eqaualization value");
		m_b_hdr_st->SetToolTip("Reset the eqaualization value");
		break;
	case 4:
		m_r_gamma_st->SetToolTip("Undo the gamma value changes");
		m_g_gamma_st->SetToolTip("Undo the gamma value changes");
		m_b_gamma_st->SetToolTip("Undo the gamma value changes");
		m_r_brightness_st->SetToolTip("Undo the brightness value changes");
		m_g_brightness_st->SetToolTip("Undo the brightness value changes");
		m_b_brightness_st->SetToolTip("Undo the brightness value changes");
		m_r_hdr_st->SetToolTip("Undo the equalization value changes");
		m_g_hdr_st->SetToolTip("Undo the equalization value changes");
		m_b_hdr_st->SetToolTip("Undo the equalization value changes");
		break;
	case 3:
	case 5:
		m_r_gamma_st->SetToolTip("No function assigned");
		m_g_gamma_st->SetToolTip("No function assigned");
		m_b_gamma_st->SetToolTip("No function assigned");
		m_r_brightness_st->SetToolTip("No function assigned");
		m_g_brightness_st->SetToolTip("No function assigned");
		m_b_brightness_st->SetToolTip("No function assigned");
		m_r_hdr_st->SetToolTip("No function assigned");
		m_g_hdr_st->SetToolTip("No function assigned");
		m_b_hdr_st->SetToolTip("No function assigned");
		break;
	}
}

void OutputAdjPanel::UpdateSyncR(bool bval)
{
	m_sync_r_chk->ToggleTool(0, bval);
	m_sync_r_chk->SetToolNormalBitmap(0,
		bval ? wxGetBitmap(link) : wxGetBitmap(unlink));
}

void OutputAdjPanel::UpdateGammaR(double dval)
{
	m_r_gamma_sldr->ChangeValue(std::round(dval * 100.0));
	m_r_gamma_text->ChangeValue(wxString::Format("%.2f", dval));
}

void OutputAdjPanel::UpdateBrightnessR(double dval)
{
	m_r_brightness_sldr->ChangeValue(std::round(dval));
	m_r_brightness_text->ChangeValue(wxString::Format("%d", int(std::round(dval))));
}

void OutputAdjPanel::UpdateHdrR(double dval)
{
	m_r_hdr_sldr->ChangeValue(std::round(dval));
	m_r_hdr_text->ChangeValue(wxString::Format("%.2f", dval));
}

void OutputAdjPanel::UpdateSyncG(bool bval)
{
	m_sync_g_chk->ToggleTool(0, bval);
	m_sync_g_chk->SetToolNormalBitmap(0,
		bval ? wxGetBitmap(link) : wxGetBitmap(unlink));
}

void OutputAdjPanel::UpdateGammaG(double dval)
{
	m_g_gamma_sldr->ChangeValue(std::round(dval * 100.0));
	m_g_gamma_text->ChangeValue(wxString::Format("%.2f", dval));
}

void OutputAdjPanel::UpdateBrightnessG(double dval)
{
	m_g_brightness_sldr->ChangeValue(std::round(dval));
	m_g_brightness_text->ChangeValue(wxString::Format("%d", int(std::round(dval))));
}

void OutputAdjPanel::UpdateHdrG(double dval)
{
	m_g_hdr_sldr->ChangeValue(std::round(dval * 100.0));
	m_g_hdr_text->ChangeValue(wxString::Format("%.2f", dval));
}

void OutputAdjPanel::UpdateSyncB(bool bval)
{
	m_sync_b_chk->ToggleTool(0, bval);
	m_sync_b_chk->SetToolNormalBitmap(0,
		bval ? wxGetBitmap(link) : wxGetBitmap(unlink));
}

void OutputAdjPanel::UpdateGammaB(double dval)
{
	m_b_gamma_sldr->ChangeValue(std::round(dval * 100.0));
	m_b_gamma_text->ChangeValue(wxString::Format("%.2f", dval));
}

void OutputAdjPanel::UpdateBrightnessB(double dval)
{
	m_b_brightness_sldr->ChangeValue(std::round(dval));
	m_b_brightness_text->ChangeValue(wxString::Format("%d", int(std::round(dval))));
}

void OutputAdjPanel::UpdateHdrB(double dval)
{
	m_b_hdr_sldr->ChangeValue(std::round(dval * 100.0));
	m_b_hdr_text->ChangeValue(wxString::Format("%.2f", dval));
}

void OutputAdjPanel::EnableAll(bool val)
{
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

void OutputAdjPanel::ClearUndo()
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
void OutputAdjPanel::OnRGammaMF(wxCommandEvent& event)
{
	switch (glbin_settings.m_mulfunc)
	{
	case 0:
		SyncGamma(0);
		break;
	case 1:
		m_frame->SetFocusVRenderViews(m_r_gamma_sldr);
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

void OutputAdjPanel::OnGGammaMF(wxCommandEvent& event)
{
	switch (glbin_settings.m_mulfunc)
	{
	case 0:
		SyncGamma(1);
		break;
	case 1:
		m_frame->SetFocusVRenderViews(m_g_gamma_sldr);
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

void OutputAdjPanel::OnBGammaMF(wxCommandEvent& event)
{
	switch (glbin_settings.m_mulfunc)
	{
	case 0:
		SyncGamma(2);
		break;
	case 1:
		m_frame->SetFocusVRenderViews(m_b_gamma_sldr);
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

void OutputAdjPanel::OnRBrightnessMF(wxCommandEvent& event)
{
	switch (glbin_settings.m_mulfunc)
	{
	case 0:
		SyncBrightness(0);
		break;
	case 1:
		m_frame->SetFocusVRenderViews(m_r_brightness_sldr);
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

void OutputAdjPanel::OnGBrightnessMF(wxCommandEvent& event)
{
	switch (glbin_settings.m_mulfunc)
	{
	case 0:
		SyncBrightness(1);
		break;
	case 1:
		m_frame->SetFocusVRenderViews(m_g_brightness_sldr);
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

void OutputAdjPanel::OnBBrightnessMF(wxCommandEvent& event)
{
	switch (glbin_settings.m_mulfunc)
	{
	case 0:
		SyncBrightness(2);
		break;
	case 1:
		m_frame->SetFocusVRenderViews(m_b_brightness_sldr);
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

void OutputAdjPanel::OnRHdrMF(wxCommandEvent& event)
{
	switch (glbin_settings.m_mulfunc)
	{
	case 0:
		SyncHdr(0);
		break;
	case 1:
		m_frame->SetFocusVRenderViews(m_r_hdr_sldr);
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

void OutputAdjPanel::OnGHdrMF(wxCommandEvent& event)
{
	switch (glbin_settings.m_mulfunc)
	{
	case 0:
		SyncHdr(1);
		break;
	case 1:
		m_frame->SetFocusVRenderViews(m_g_hdr_sldr);
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

void OutputAdjPanel::OnBHdrMF(wxCommandEvent& event)
{
	switch (glbin_settings.m_mulfunc)
	{
	case 0:
		SyncHdr(2);
		break;
	case 1:
		m_frame->SetFocusVRenderViews(m_b_hdr_sldr);
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

void OutputAdjPanel::OnRGammaChange(wxScrollEvent & event)
{
	double val = m_r_gamma_sldr->GetValue() / 100.0;
	wxString str = wxString::Format("%.2f", val);
	m_r_gamma_text->ChangeValue(str);
	SetGamma(0, 1.0 / val, false);
}

void OutputAdjPanel::OnRGammaText(wxCommandEvent& event)
{
	wxString str = m_r_gamma_text->GetValue();
	double val;
	str.ToDouble(&val);
	m_r_gamma_sldr->ChangeValue(std::round(val * 100));
	SetGamma(0, 1.0 / val, false);
}

void OutputAdjPanel::OnGGammaChange(wxScrollEvent & event)
{
	double val = m_g_gamma_sldr->GetValue() / 100.0;
	wxString str = wxString::Format("%.2f", val);
	m_g_gamma_text->ChangeValue(str);
	SetGamma(1, 1.0 / val, false);
}

void OutputAdjPanel::OnGGammaText(wxCommandEvent& event)
{
	wxString str = m_g_gamma_text->GetValue();
	double val;
	str.ToDouble(&val);
	m_g_gamma_sldr->ChangeValue(std::round(val * 100));
	SetGamma(1, 1.0 / val, false);
}

void OutputAdjPanel::OnBGammaChange(wxScrollEvent & event)
{
	double val = m_b_gamma_sldr->GetValue() / 100.0;
	wxString str = wxString::Format("%.2f", val);
	m_b_gamma_text->ChangeValue(str);
	SetGamma(2, 1.0 / val, false);
}

void OutputAdjPanel::OnBGammaText(wxCommandEvent& event)
{
	wxString str = m_b_gamma_text->GetValue();
	double val;
	str.ToDouble(&val);
	m_b_gamma_sldr->ChangeValue(std::round(val * 100));
	SetGamma(2, 1.0 / val, false);
}

//brightness
void OutputAdjPanel::OnRBrightnessChange(wxScrollEvent & event)
{
	int ival = m_r_brightness_sldr->GetValue();
	double dval = ival / 256.0 + 1.0;
	wxString str = wxString::Format("%d", ival);
	m_r_brightness_text->ChangeValue(str);
	SetBrightness(0, dval, false);
}

void OutputAdjPanel::OnRBrightnessText(wxCommandEvent& event)
{
	wxString str = m_r_brightness_text->GetValue();
	double val;
	str.ToDouble(&val);
	m_r_brightness_sldr->ChangeValue(std::round(val));
	val = val / 256.0 + 1.0;
	SetBrightness(0, val, false);
}

void OutputAdjPanel::OnGBrightnessChange(wxScrollEvent & event)
{
	int ival = m_g_brightness_sldr->GetValue();
	double dval = ival / 256.0 + 1.0;
	wxString str = wxString::Format("%d", ival);
	m_g_brightness_text->ChangeValue(str);
	SetBrightness(1, dval, false);
}

void OutputAdjPanel::OnGBrightnessText(wxCommandEvent& event)
{
	wxString str = m_g_brightness_text->GetValue();
	double val;
	str.ToDouble(&val);
	m_g_brightness_sldr->ChangeValue(std::round(val));
	val = val / 256.0 + 1.0;
	SetBrightness(1, val, false);
}

void OutputAdjPanel::OnBBrightnessChange(wxScrollEvent & event)
{
	int ival = m_b_brightness_sldr->GetValue();
	double dval = ival / 256.0 + 1.0;
	wxString str = wxString::Format("%d", ival);
	m_b_brightness_text->ChangeValue(str);
	SetBrightness(2, dval, false);
}

void OutputAdjPanel::OnBBrightnessText(wxCommandEvent& event)
{
	wxString str = m_b_brightness_text->GetValue();
	double val;
	str.ToDouble(&val);
	m_b_brightness_sldr->ChangeValue(std::round(val));
	val = val / 256.0 + 1.0;
	SetBrightness(2, val, false);
}

void OutputAdjPanel::OnRHdrChange(wxScrollEvent& event)
{
	double val = m_r_hdr_sldr->GetValue() / 100.0;
	wxString str = wxString::Format("%.2f", val);
	m_r_hdr_text->ChangeValue(str);
	SetHdr(0, val, false);
}

void OutputAdjPanel::OnRHdrText(wxCommandEvent& event)
{
	wxString str = m_r_hdr_text->GetValue();
	double val;
	str.ToDouble(&val);
	m_r_hdr_sldr->ChangeValue(std::round(val * 100));
	SetHdr(0, val, false);
}

void OutputAdjPanel::OnGHdrChange(wxScrollEvent& event)
{
	double val = m_g_hdr_sldr->GetValue() / 100.0;
	wxString str = wxString::Format("%.2f", val);
	m_g_hdr_text->ChangeValue(str);
	SetHdr(1, val, false);
}

void OutputAdjPanel::OnGHdrText(wxCommandEvent& event)
{
	wxString str = m_g_hdr_text->GetValue();
	double val;
	str.ToDouble(&val);
	m_g_hdr_sldr->ChangeValue(std::round(val * 100));
	SetHdr(1, val, false);
}

void OutputAdjPanel::OnBHdrChange(wxScrollEvent& event)
{
	double val = m_b_hdr_sldr->GetValue() / 100.0;
	wxString str = wxString::Format("%.2f", val);
	m_b_hdr_text->ChangeValue(str);
	SetHdr(2, val, false);
}

void OutputAdjPanel::OnBHdrText(wxCommandEvent& event)
{
	wxString str = m_b_hdr_text->GetValue();
	double val;
	str.ToDouble(&val);
	m_b_hdr_sldr->ChangeValue(std::round(val * 100));
	SetHdr(2, val, false);
}

void OutputAdjPanel::OnSyncRCheck(wxCommandEvent& event)
{
	SetSync(0, m_sync_r_chk->GetToolState(0));
}

void OutputAdjPanel::OnSyncGCheck(wxCommandEvent& event)
{
	SetSync(1, m_sync_g_chk->GetToolState(0));
}

void OutputAdjPanel::OnSyncBCheck(wxCommandEvent& event)
{
	SetSync(2, m_sync_b_chk->GetToolState(0));
}

void OutputAdjPanel::OnSaveDefault(wxCommandEvent& event)
{
	int type = glbin_current.GetType();
	switch (type)
	{
	case 1://view
	case 3://mesh
	case 6://mesh group
		glbin_outadj_def.Set(*glbin_current.render_view.lock());
		break;
	case 2://volume data
		glbin_outadj_def.Set(*glbin_current.vol_data.lock());
		break;
	case 5://group
		glbin_outadj_def.Set(*glbin_current.vol_group.lock());
		break;
	}
}

void OutputAdjPanel::OnRReset(wxCommandEvent& event)
{
	double dval;
	dval = 1.0 / glbin_outadj_def.m_gamma_r;
	m_r_gamma_sldr->ChangeValue(std::round(dval * 100.0));
	m_r_gamma_text->ChangeValue(wxString::Format("%.2f", dval));
	SetGamma(0, glbin_outadj_def.m_gamma_r, false);
	dval = (glbin_outadj_def.m_brightness_r - 1.0) * 256.0;
	m_r_brightness_sldr->ChangeValue(std::round(dval));
	m_r_brightness_text->ChangeValue(wxString::Format("%d", int(std::round(dval))));
	SetBrightness(0, glbin_outadj_def.m_brightness_r, false);
	dval = glbin_outadj_def.m_hdr_r;
	m_r_hdr_sldr->ChangeValue(std::round(dval * 100.0));
	m_r_hdr_text->ChangeValue(wxString::Format("%.2f", dval));
	SetHdr(0, glbin_outadj_def.m_hdr_r, false);
}

void OutputAdjPanel::OnGReset(wxCommandEvent& event)
{
	double dval;
	dval = 1.0 / glbin_outadj_def.m_gamma_g;
	m_g_gamma_sldr->ChangeValue(std::round(dval * 100.0));
	m_g_gamma_text->ChangeValue(wxString::Format("%.2f", dval));
	SetGamma(1, glbin_outadj_def.m_gamma_g, false);
	dval = (glbin_outadj_def.m_brightness_g - 1.0) * 256.0;
	m_g_brightness_sldr->ChangeValue(std::round(dval));
	m_g_brightness_text->ChangeValue(wxString::Format("%d", int(std::round(dval))));
	SetBrightness(1, glbin_outadj_def.m_brightness_g, false);
	dval = glbin_outadj_def.m_hdr_g;
	m_g_hdr_sldr->ChangeValue(std::round(dval * 100.0));
	m_g_hdr_text->ChangeValue(wxString::Format("%.2f", dval));
	SetHdr(1, glbin_outadj_def.m_hdr_g, false);
}

void OutputAdjPanel::OnBReset(wxCommandEvent& event)
{
	double dval;
	dval = 1.0 / glbin_outadj_def.m_gamma_b;
	m_b_gamma_sldr->ChangeValue(std::round(dval * 100.0));
	m_b_gamma_text->ChangeValue(wxString::Format("%.2f", dval));
	SetGamma(2, glbin_outadj_def.m_gamma_b, false);
	dval = (glbin_outadj_def.m_brightness_b - 1.0) * 256.0;
	m_b_brightness_sldr->ChangeValue(std::round(dval));
	m_b_brightness_text->ChangeValue(wxString::Format("%d", int(std::round(dval))));
	SetBrightness(2, glbin_outadj_def.m_brightness_b, false);
	dval = glbin_outadj_def.m_hdr_b;
	m_b_hdr_sldr->ChangeValue(std::round(dval * 100.0));
	m_b_hdr_text->ChangeValue(wxString::Format("%.2f", dval));
	SetHdr(2, glbin_outadj_def.m_hdr_b, false);
}

