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
#include <OutputAdjPanel.h>
#include <Global/Global.h>
#include <MainFrame.h>
#include <RenderCanvas.h>
#include <wxSingleSlider.h>
#include <wx/valnum.h>
#include <wx/stdpaths.h>
#include <png_resource.h>
#include <img/icons.h>

OutputAdjPanel::OutputAdjPanel(MainFrame* frame,
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
m_enable_all(true)
{
	m_sync[0] = true;
	m_sync[1] = true;
	m_sync[2] = true;
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
	m_dft_btn->SetBitmap(wxGetBitmapFromMemory(save_settings));
	m_dft_btn->Bind(wxEVT_BUTTON, &OutputAdjPanel::OnSaveDefault, this);

	sizer->Add(m_notebook, 1, wxEXPAND);
	sizer->Add(m_dft_btn, 0, wxEXPAND);
	SetSizer(sizer);
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

OutputAdjPanel::~OutputAdjPanel()
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

wxWindow* OutputAdjPanel::CreateRedPage(wxWindow* parent, wxSize& size)
{
	wxScrolledWindow* page = new wxScrolledWindow(parent);
	SetSize(size);

	long ls = glbin_settings.m_inverse_slider ? wxSL_VERTICAL : (wxSL_VERTICAL | wxSL_INVERSE);
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
	wxStaticText* st;
	//validator: floating point 2
	wxFloatingPointValidator<double> vald_fp2(2);
	//validator: integer
	wxIntegerValidator<int> vald_int;
	vald_int.SetRange(-256, 256);
	wxBitmap bitmap;

	//multifunc buttons
	wxBoxSizer* sizer1 = new wxBoxSizer(wxHORIZONTAL);
	m_r_gamma_st = new wxButton(page, wxID_ANY, "Gam.R.",
		wxDefaultPosition, FromDIP(wxSize(30, 20)));
	m_r_gamma_st->Bind(wxEVT_BUTTON, &OutputAdjPanel::OnRGammaMF, this);
	sizer1->Add(m_r_gamma_st, 1, wxEXPAND);
	m_r_brightness_st = new wxButton(page, wxID_ANY, "Lum.R.",
		wxDefaultPosition, FromDIP(wxSize(30, 20)));
	m_r_brightness_st->Bind(wxEVT_BUTTON, &OutputAdjPanel::OnRBrightnessMF, this);
	sizer1->Add(m_r_brightness_st, 1, wxEXPAND);
	m_r_hdr_st = new wxButton(page, wxID_ANY, "Eql.R.",
		wxDefaultPosition, FromDIP(wxSize(30, 20)));
	m_r_hdr_st->Bind(wxEVT_BUTTON, &OutputAdjPanel::OnRHdrMF, this);
	sizer1->Add(m_r_hdr_st, 1, wxEXPAND);

	//sliders
	wxBoxSizer* sizer2 = new wxBoxSizer(wxHORIZONTAL);
	m_r_gamma_sldr = new wxSingleSlider(page, wxID_ANY, 100, 10, 400,
		wxDefaultPosition, wxDefaultSize, ls);
	m_r_gamma_sldr->Bind(wxEVT_SCROLL_CHANGED, &OutputAdjPanel::OnRGammaChange, this);
	m_r_gamma_sldr->SetRangeColor(*wxRED);
	sizer2->Add(m_r_gamma_sldr, 1, wxEXPAND);
	m_r_brightness_sldr = new wxSingleSlider(page, wxID_ANY, 0, -256, 256,
		wxDefaultPosition, wxDefaultSize, ls);
	m_r_brightness_sldr->SetRangeStyle(2);
	m_r_brightness_sldr->SetRangeColor(*wxRED);
	m_r_brightness_sldr->Bind(wxEVT_SCROLL_CHANGED, &OutputAdjPanel::OnRBrightnessChange, this);
	sizer2->Add(m_r_brightness_sldr, 1, wxEXPAND);
	m_r_hdr_sldr = new wxSingleSlider(page, wxID_ANY, 0, 0, 100,
		wxDefaultPosition, wxDefaultSize, ls);
	m_r_hdr_sldr->SetRangeColor(*wxRED);
	m_r_hdr_sldr->Bind(wxEVT_SCROLL_CHANGED, &OutputAdjPanel::OnRHdrChange, this);
	sizer2->Add(m_r_hdr_sldr, 1, wxEXPAND);

	//input boxes
	wxBoxSizer* sizer3 = new wxBoxSizer(wxHORIZONTAL);
	vald_fp2.SetRange(0.0, 10.0);
	m_r_gamma_text = new wxTextCtrl(page, wxID_ANY, "1.00",
		wxDefaultPosition, FromDIP(wxSize(30, 20)), wxTE_CENTER, vald_fp2);
	m_r_gamma_text->Bind(wxEVT_TEXT, &OutputAdjPanel::OnRGammaText, this);
	sizer3->Add(m_r_gamma_text, 1, wxEXPAND);
	m_r_brightness_text = new wxTextCtrl(page, wxID_ANY, "0",
		wxDefaultPosition, FromDIP(wxSize(30, 20)), wxTE_CENTER, vald_int);
	m_r_brightness_text->Bind(wxEVT_TEXT, &OutputAdjPanel::OnRBrightnessText, this);
	sizer3->Add(m_r_brightness_text, 1, wxEXPAND);
	vald_fp2.SetRange(0.0, 1.0);
	m_r_hdr_text = new wxTextCtrl(page, wxID_ANY, "0.00",
		wxDefaultPosition, FromDIP(wxSize(30, 20)), wxTE_CENTER, vald_fp2);
	m_r_hdr_text->Bind(wxEVT_TEXT, &OutputAdjPanel::OnRHdrText, this);
	sizer3->Add(m_r_hdr_text, 1, wxEXPAND);

	//reset buttons
	wxBoxSizer* sizer4 = new wxBoxSizer(wxHORIZONTAL);
	m_r_reset_btn = new wxButton(page, wxID_ANY, "Reset",
		wxDefaultPosition, FromDIP(wxSize(30, 22)));
	m_r_reset_btn->SetBitmap(wxGetBitmapFromMemory(reset));
	m_r_reset_btn->Bind(wxEVT_BUTTON, &OutputAdjPanel::OnRReset, this);
	m_sync_r_chk = new wxToolBar(page, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	bitmap = wxGetBitmapFromMemory(unlink);
#ifdef _DARWIN
	m_sync_r_chk->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_sync_r_chk->AddCheckTool(0, "Link",
		bitmap, wxNullBitmap,
		"Link Red Properties with Linked Green or Blue",
		"Link Red Properties with Linked Green or Blue");
	m_sync_r_chk->Bind(wxEVT_TOOL, &OutputAdjPanel::OnSyncRCheck, this);
	m_sync_r_chk->Realize();
	sizer4->Add(m_r_reset_btn, 1, wxEXPAND);
	sizer4->Add(5, 5);
	sizer4->Add(m_sync_r_chk, 0, wxALIGN_CENTER);

	sizer_v->Add(sizer1, 0, wxEXPAND);
	sizer_v->Add(sizer2, 1, wxEXPAND);
	sizer_v->Add(sizer3, 0, wxEXPAND);
	sizer_v->Add(sizer4, 0, wxEXPAND);

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
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
	wxStaticText* st;
	//validator: floating point 2
	wxFloatingPointValidator<double> vald_fp2(2);
	//validator: integer
	wxIntegerValidator<int> vald_int;
	vald_int.SetRange(-256, 256);
	wxBitmap bitmap;

	//buttons
	wxBoxSizer* sizer1 = new wxBoxSizer(wxHORIZONTAL);
	m_g_gamma_st = new wxButton(page, wxID_ANY, "Gam.G.",
		wxDefaultPosition, FromDIP(wxSize(30, 20)));
	m_g_gamma_st->Bind(wxEVT_BUTTON, &OutputAdjPanel::OnGGammaMF, this);
	sizer1->Add(m_g_gamma_st, 1, wxEXPAND);
	m_g_brightness_st = new wxButton(page, wxID_ANY, "Lum.G.",
		wxDefaultPosition, FromDIP(wxSize(30, 20)));
	m_g_brightness_st->Bind(wxEVT_BUTTON, &OutputAdjPanel::OnGBrightnessMF, this);
	sizer1->Add(m_g_brightness_st, 1, wxEXPAND);
	m_g_hdr_st = new wxButton(page, wxID_ANY, "Eql.G.",
		wxDefaultPosition, FromDIP(wxSize(30, 20)));
	m_g_hdr_st->Bind(wxEVT_BUTTON, &OutputAdjPanel::OnGHdrMF, this);
	sizer1->Add(m_g_hdr_st, 1, wxEXPAND);

	//sliders
	wxBoxSizer* sizer2 = new wxBoxSizer(wxHORIZONTAL);
	m_g_gamma_sldr = new wxSingleSlider(page, wxID_ANY, 100, 10, 400,
		wxDefaultPosition, wxDefaultSize, ls);
	m_g_gamma_sldr->Bind(wxEVT_SCROLL_CHANGED, &OutputAdjPanel::OnGGammaChange, this);
	m_g_gamma_sldr->SetRangeColor(*wxGREEN);
	sizer2->Add(m_g_gamma_sldr, 1, wxEXPAND);
	m_g_brightness_sldr = new wxSingleSlider(page, wxID_ANY, 0, -256, 256,
		wxDefaultPosition, wxDefaultSize, ls);
	m_g_brightness_sldr->SetRangeStyle(2);
	m_g_brightness_sldr->SetRangeColor(*wxGREEN);
	m_g_brightness_sldr->Bind(wxEVT_SCROLL_CHANGED, &OutputAdjPanel::OnGBrightnessChange, this);
	sizer2->Add(m_g_brightness_sldr, 1, wxEXPAND);
	m_g_hdr_sldr = new wxSingleSlider(page, wxID_ANY, 0, 0, 100,
		wxDefaultPosition, wxDefaultSize, ls);
	m_g_hdr_sldr->SetRangeColor(*wxGREEN);
	m_g_hdr_sldr->Bind(wxEVT_SCROLL_CHANGED, &OutputAdjPanel::OnGHdrChange, this);
	sizer2->Add(m_g_hdr_sldr, 1, wxEXPAND);

	//input boxes
	wxBoxSizer* sizer3 = new wxBoxSizer(wxHORIZONTAL);
	vald_fp2.SetRange(0.0, 10.0);
	m_g_gamma_text = new wxTextCtrl(page, wxID_ANY, "1.00",
		wxDefaultPosition, FromDIP(wxSize(30, 20)), wxTE_CENTER, vald_fp2);
	m_g_gamma_text->Bind(wxEVT_TEXT, &OutputAdjPanel::OnGGammaText, this);
	sizer3->Add(m_g_gamma_text, 1, wxEXPAND);
	m_g_brightness_text = new wxTextCtrl(page, wxID_ANY, "0",
		wxDefaultPosition, FromDIP(wxSize(30, 20)), wxTE_CENTER, vald_int);
	m_g_brightness_text->Bind(wxEVT_TEXT, &OutputAdjPanel::OnGBrightnessText, this);
	sizer3->Add(m_g_brightness_text, 1, wxEXPAND);
	vald_fp2.SetRange(0.0, 1.0);
	m_g_hdr_text = new wxTextCtrl(page, wxID_ANY, "0.00",
		wxDefaultPosition, FromDIP(wxSize(30, 20)), wxTE_CENTER, vald_fp2);
	m_g_hdr_text->Bind(wxEVT_TEXT, &OutputAdjPanel::OnGHdrText, this);
	sizer3->Add(m_g_hdr_text, 1, wxEXPAND);

	//reset buttons
	wxBoxSizer* sizer4 = new wxBoxSizer(wxHORIZONTAL);
	m_g_reset_btn = new wxButton(page, wxID_ANY, "Reset",
		wxDefaultPosition, FromDIP(wxSize(30, 22)));
	m_g_reset_btn->SetBitmap(wxGetBitmapFromMemory(reset));
	m_g_reset_btn->Bind(wxEVT_BUTTON, &OutputAdjPanel::OnGReset, this);
	wxBoxSizer* sizer_h_6 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Green:",
		wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
	m_sync_g_chk = new wxToolBar(page, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	bitmap = wxGetBitmapFromMemory(unlink);
#ifdef _DARWIN
	m_sync_g_chk->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_sync_g_chk->AddCheckTool(0, "Link",
		bitmap, wxNullBitmap,
		"Link Green Properties with Linked Red or Blue",
		"Link Green Properties with Linked Red or Blue");
	m_sync_g_chk->Bind(wxEVT_TOOL, &OutputAdjPanel::OnSyncGCheck, this);
	m_sync_g_chk->Realize();
	sizer4->Add(m_g_reset_btn, 1, wxEXPAND);
	sizer4->Add(5, 5);
	sizer4->Add(m_sync_g_chk, 0, wxALIGN_CENTER);

	sizer_v->Add(sizer1, 0, wxEXPAND);
	sizer_v->Add(sizer2, 1, wxEXPAND);
	sizer_v->Add(sizer3, 0, wxEXPAND);
	sizer_v->Add(sizer4, 0, wxEXPAND);

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
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
	wxStaticText* st;
	//validator: floating point 2
	wxFloatingPointValidator<double> vald_fp2(2);
	//validator: integer
	wxIntegerValidator<int> vald_int;
	vald_int.SetRange(-256, 256);
	wxBitmap bitmap;

	//multifunc buttons
	wxBoxSizer* sizer1 = new wxBoxSizer(wxHORIZONTAL);
	m_b_gamma_st = new wxButton(page, wxID_ANY, "Gam.B.",
		wxDefaultPosition, FromDIP(wxSize(30, 20)));
	m_b_gamma_st->Bind(wxEVT_BUTTON, &OutputAdjPanel::OnBGammaMF, this);
	sizer1->Add(m_b_gamma_st, 1, wxEXPAND);
	m_b_brightness_st = new wxButton(page, wxID_ANY, "Lum.B.",
		wxDefaultPosition, FromDIP(wxSize(30, 20)));
	m_b_brightness_st->Bind(wxEVT_BUTTON, &OutputAdjPanel::OnBBrightnessMF, this);
	sizer1->Add(m_b_brightness_st, 1, wxEXPAND);
	m_b_hdr_st = new wxButton(page, wxID_ANY, "Eql.B.",
		wxDefaultPosition, FromDIP(wxSize(30, 20)));
	m_b_hdr_st->Bind(wxEVT_BUTTON, &OutputAdjPanel::OnBHdrMF, this);
	sizer1->Add(m_b_hdr_st, 1, wxEXPAND);

	//sliders
	wxBoxSizer* sizer2 = new wxBoxSizer(wxHORIZONTAL);
	m_b_gamma_sldr = new wxSingleSlider(page, wxID_ANY, 100, 10, 400,
		wxDefaultPosition, wxDefaultSize, ls);
	m_b_gamma_sldr->SetRangeColor(*wxBLUE);
	m_b_gamma_sldr->Bind(wxEVT_SCROLL_CHANGED, &OutputAdjPanel::OnBGammaChange, this);
	sizer2->Add(m_b_gamma_sldr, 1, wxEXPAND);
	m_b_brightness_sldr = new wxSingleSlider(page, wxID_ANY, 0, -256, 256,
		wxDefaultPosition, wxDefaultSize, ls);
	m_b_brightness_sldr->SetRangeStyle(2);
	m_b_brightness_sldr->SetRangeColor(*wxBLUE);
	m_b_brightness_sldr->Bind(wxEVT_SCROLL_CHANGED, &OutputAdjPanel::OnBBrightnessChange, this);
	sizer2->Add(m_b_brightness_sldr, 1, wxEXPAND);
	m_b_hdr_sldr = new wxSingleSlider(page, wxID_ANY, 0, 0, 100,
		wxDefaultPosition, wxDefaultSize, ls);
	m_b_hdr_sldr->SetRangeColor(*wxBLUE);
	m_b_hdr_sldr->Bind(wxEVT_SCROLL_CHANGED, &OutputAdjPanel::OnBHdrChange, this);
	sizer2->Add(m_b_hdr_sldr, 1, wxEXPAND);

	//input boxes
	wxBoxSizer* sizer3 = new wxBoxSizer(wxHORIZONTAL);
	vald_fp2.SetRange(0.0, 10.0);
	m_b_gamma_text = new wxTextCtrl(page, wxID_ANY, "1.00",
		wxDefaultPosition, FromDIP(wxSize(30, 20)), wxTE_CENTER, vald_fp2);
	m_b_gamma_text->Bind(wxEVT_TEXT, &OutputAdjPanel::OnBGammaText, this);
	sizer3->Add(m_b_gamma_text, 1, wxEXPAND);
	m_b_brightness_text = new wxTextCtrl(page, wxID_ANY, "0",
		wxDefaultPosition, FromDIP(wxSize(30, 20)), wxTE_CENTER, vald_int);
	m_b_brightness_text->Bind(wxEVT_TEXT, &OutputAdjPanel::OnBBrightnessText, this);
	sizer3->Add(m_b_brightness_text, 1, wxEXPAND);
	vald_fp2.SetRange(0.0, 1.0);
	m_b_hdr_text = new wxTextCtrl(page, wxID_ANY, "0.00",
		wxDefaultPosition, FromDIP(wxSize(30, 20)), wxTE_CENTER, vald_fp2);
	m_b_hdr_text->Bind(wxEVT_TEXT, &OutputAdjPanel::OnBHdrText, this);
	sizer3->Add(m_b_hdr_text, 1, wxEXPAND);

	//reset buttons
	wxBoxSizer* sizer4 = new wxBoxSizer(wxHORIZONTAL);
	m_b_reset_btn = new wxButton(page, wxID_ANY, "Reset",
		wxDefaultPosition, FromDIP(wxSize(30, 22)));
	m_b_reset_btn->SetBitmap(wxGetBitmapFromMemory(reset));
	m_b_reset_btn->Bind(wxEVT_BUTTON, &OutputAdjPanel::OnBReset, this);
	m_sync_b_chk = new wxToolBar(page, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	bitmap = wxGetBitmapFromMemory(unlink);
#ifdef _DARWIN
	m_sync_b_chk->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_sync_b_chk->AddCheckTool(0, "Link",
		bitmap, wxNullBitmap,
		"Link Blue Properties with Linked Red or Green",
		"Link Blue Properties with Linked Red or Green");
	m_sync_b_chk->Bind(wxEVT_TOOL, &OutputAdjPanel::OnSyncBCheck, this);
	m_sync_b_chk->Realize();
	sizer4->Add(m_b_reset_btn, 1, wxEXPAND);
	sizer4->Add(5, 5);
	sizer4->Add(m_sync_b_chk, 0, wxALIGN_CENTER);

	sizer_v->Add(sizer1, 0, wxEXPAND);
	sizer_v->Add(sizer2, 1, wxEXPAND);
	sizer_v->Add(sizer3, 0, wxEXPAND);
	sizer_v->Add(sizer4, 0, wxEXPAND);

	page->SetSizer(sizer_v);
	page->SetAutoLayout(true);
	page->SetScrollRate(10, 10);
	return page;
}

void OutputAdjPanel::LoadPerspective()
{
	if (glbin_outadj_def.m_split)
	{
		m_notebook->Split(1, wxBOTTOM);
		m_notebook->Split(2, wxBOTTOM);
	}
}

void OutputAdjPanel::SavePerspective()
{
	glbin_outadj_def.m_split = m_notebook->IsSplit();
}

void OutputAdjPanel::FluoUpdate(const fluo::ValueCollection& vc)
{
	if (m_type != 1 && m_type != 2 && m_type != 5)
	{
		EnableAll(false);
		return;
	}
	if (FOUND_VALUE(gstNull))
		return;
	bool update_all = vc.empty();

	//mf button tips
	if (update_all || FOUND_VALUE(gstMultiFuncTips))
	{
		switch (glbin_settings.m_mulfunc)
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

void OutputAdjPanel::EnableAll(bool val)
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
void OutputAdjPanel::SetRenderView(RenderCanvas *view)
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

RenderCanvas* OutputAdjPanel::GetRenderView()
{
	return m_view;
}

//set volume data
void OutputAdjPanel::SetVolumeData(VolumeData* vd)
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

VolumeData* OutputAdjPanel::GetVolumeData()
{
	return m_vd;
}

//set group
void OutputAdjPanel::SetGroup(DataGroup *group)
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

DataGroup* OutputAdjPanel::GetGroup()
{
	return m_group;
}

//set volume adjustment to link to group
void OutputAdjPanel::SetGroupLink(DataGroup *group)
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

void OutputAdjPanel::OnGGammaMF(wxCommandEvent& event)
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

void OutputAdjPanel::OnBGammaMF(wxCommandEvent& event)
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

void OutputAdjPanel::OnRBrightnessMF(wxCommandEvent& event)
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

void OutputAdjPanel::OnGBrightnessMF(wxCommandEvent& event)
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

void OutputAdjPanel::OnBBrightnessMF(wxCommandEvent& event)
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

void OutputAdjPanel::OnRHdrMF(wxCommandEvent& event)
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

void OutputAdjPanel::OnGHdrMF(wxCommandEvent& event)
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

void OutputAdjPanel::OnBHdrMF(wxCommandEvent& event)
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

void OutputAdjPanel::OnRHdrChange(wxScrollEvent &event)
{
	double val = m_r_hdr_sldr->GetValue() / 100.0;
	wxString str = wxString::Format("%.2f", val);
	m_r_hdr_text->ChangeValue(str);
	SetHdr(0, val, false);
}

void OutputAdjPanel::OnRHdrText(wxCommandEvent &event)
{
	wxString str = m_r_hdr_text->GetValue();
	double val;
	str.ToDouble(&val);
	m_r_hdr_sldr->ChangeValue(std::round(val * 100));
	SetHdr(0, val, false);
}

void OutputAdjPanel::OnGHdrChange(wxScrollEvent &event)
{
	double val = m_g_hdr_sldr->GetValue() / 100.0;
	wxString str = wxString::Format("%.2f", val);
	m_g_hdr_text->ChangeValue(str);
	SetHdr(1, val, false);
}

void OutputAdjPanel::OnGHdrText(wxCommandEvent &event)
{
	wxString str = m_g_hdr_text->GetValue();
	double val;
	str.ToDouble(&val);
	m_g_hdr_sldr->ChangeValue(std::round(val * 100));
	SetHdr(1, val, false);
}

void OutputAdjPanel::OnBHdrChange(wxScrollEvent &event)
{
	double val = m_b_hdr_sldr->GetValue() / 100.0;
	wxString str = wxString::Format("%.2f", val);
	m_b_hdr_text->ChangeValue(str);
	SetHdr(2, val, false);
}

void OutputAdjPanel::OnBHdrText(wxCommandEvent &event)
{
	wxString str = m_b_hdr_text->GetValue();
	double val;
	str.ToDouble(&val);
	m_b_hdr_sldr->ChangeValue(std::round(val * 100));
	SetHdr(2, val, false);
}

void OutputAdjPanel::OnSyncRCheck(wxCommandEvent &event)
{
	SetSync(0, m_sync_r_chk->GetToolState(0));
}

void OutputAdjPanel::OnSyncGCheck(wxCommandEvent &event)
{
	SetSync(1, m_sync_g_chk->GetToolState(0));
}

void OutputAdjPanel::OnSyncBCheck(wxCommandEvent &event)
{
	SetSync(2, m_sync_b_chk->GetToolState(0));
}

void OutputAdjPanel::OnSaveDefault(wxCommandEvent &event)
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

void OutputAdjPanel::SyncColor(fluo::Color& c, double val)
{
	for (int i : {0, 1, 2})
		if (m_sync[i])
			c[i] = val;
}

void OutputAdjPanel::SyncGamma(fluo::Color& c, int i, double val, fluo::ValueCollection& vc, bool notify)
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
			if (notify == (i == j))
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
	if (vc.empty())
		vc.insert(gstNull);
}

void OutputAdjPanel::SyncBrightness(fluo::Color& c, int i, double val, fluo::ValueCollection& vc, bool notify)
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
			if (notify == (i == j))
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
	if (vc.empty())
		vc.insert(gstNull);
}

void OutputAdjPanel::SyncHdr(fluo::Color& c, int i, double val, fluo::ValueCollection& vc, bool notify)
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
			if (notify == (i == j))
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
	if (vc.empty())
		vc.insert(gstNull);
}

void OutputAdjPanel::SyncGamma(int i)
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
	FluoRefresh(true, 2, vc, {m_frame->GetView(m_view)});
}

void OutputAdjPanel::SyncBrightness(int i)
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
	FluoRefresh(true, 2, vc, { m_frame->GetView(m_view) });
}

void OutputAdjPanel::SyncHdr(int i)
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
	FluoRefresh(true, 2, vc, { m_frame->GetView(m_view) });
}

void OutputAdjPanel::SetSync(int i, bool val, bool update)
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
		if (i == 0)
			vc.insert(gstSyncR);
		if (i == 1)
			vc.insert(gstSyncG);
		if (i == 2)
			vc.insert(gstSyncB);

		FluoRefresh(true, 2, vc, { m_frame->GetView(m_view) });
	}
}

void OutputAdjPanel::SetGamma(int i, double val, bool notify)
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
	SyncGamma(gamma, i, val, vc, notify);
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

	FluoRefresh(true, 2, vc, { m_frame->GetView(m_view) });
}

void OutputAdjPanel::SetBrightness(int i, double val, bool notify)
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
	SyncBrightness(brightness, i, val, vc, notify);
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

	FluoRefresh(true, 2, vc, { m_frame->GetView(m_view) });
}

void OutputAdjPanel::SetHdr(int i, double val, bool notify)
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
	SyncHdr(hdr, i, val, vc, notify);
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

	FluoRefresh(true, 2, vc, { m_frame->GetView(m_view) });
}

void OutputAdjPanel::UpdateSync()
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

		SetSync(0, r_v, true);
		SetSync(1, g_v, true);
		SetSync(2, b_v, true);

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
				SetGamma(1, gamma, b_v ? false : true);
				SetBrightness(1, brightness, b_v ? false : true);
				SetHdr(1, hdr, b_v ? false : true);
			}
			if (b_v)
			{
				SetGamma(2, gamma, true);
				SetBrightness(2, brightness, true);
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

		SetSync(0, r_v, true);
		SetSync(1, g_v, true);
		SetSync(2, b_v, true);

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
				SetGamma(1, gamma, b_v ? false : true);
				SetBrightness(1, brightness, b_v ? false : true);
				SetHdr(1, hdr, b_v ? false : true);
			}
			if (b_v)
			{
				SetGamma(2, gamma, true);
				SetBrightness(2, brightness, true);
				SetHdr(2, hdr, true);
			}
		}
	}

}

void OutputAdjPanel::OnRReset(wxCommandEvent &event)
{
	SetGamma(0, glbin_outadj_def.m_gamma_r, false);
	SetBrightness(0, glbin_outadj_def.m_brightness_r, false);
	SetHdr(0, glbin_outadj_def.m_hdr_r);
}

void OutputAdjPanel::OnGReset(wxCommandEvent &event)
{
	SetGamma(1, glbin_outadj_def.m_gamma_g, false);
	SetBrightness(1, glbin_outadj_def.m_brightness_g, false);
	SetHdr(1, glbin_outadj_def.m_hdr_g);
}

void OutputAdjPanel::OnBReset(wxCommandEvent &event)
{
	SetGamma(2, glbin_outadj_def.m_gamma_b, false);
	SetBrightness(2, glbin_outadj_def.m_brightness_b, false);
	SetHdr(2, glbin_outadj_def.m_hdr_b);
}

