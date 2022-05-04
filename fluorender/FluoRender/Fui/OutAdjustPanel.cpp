/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2022 Scientific Computing and Imaging Institute,
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
#include <OutAdjustPanel.h>
#include <RenderFrame.h>
#include <Global.hpp>
#include <AgentFactory.hpp>
#include <Renderview.hpp>
#include <VolumeData.hpp>
#include <VolumeGroup.hpp>
#include <VolumeFactory.hpp>
#include <wx/valnum.h>
#include <png_resource.h>
#include <img/icons.h>

BEGIN_EVENT_TABLE(OutAdjustPanel, wxPanel)
	//set gamma
	EVT_COMMAND_SCROLL(ID_RGammaSldr, OutAdjustPanel::OnRGammaChange)
	EVT_TEXT(ID_RGammaText, OutAdjustPanel::OnRGammaText)
	EVT_COMMAND_SCROLL(ID_GGammaSldr, OutAdjustPanel::OnGGammaChange)
	EVT_TEXT(ID_GGammaText, OutAdjustPanel::OnGGammaText)
	EVT_COMMAND_SCROLL(ID_BGammaSldr, OutAdjustPanel::OnBGammaChange)
	EVT_TEXT(ID_BGammaText, OutAdjustPanel::OnBGammaText)
	//set brightness
	EVT_COMMAND_SCROLL(ID_RBrightnessSldr, OutAdjustPanel::OnRBrightnessChange)
	EVT_TEXT(ID_RBrightnessText, OutAdjustPanel::OnRBrightnessText)
	EVT_COMMAND_SCROLL(ID_GBrightnessSldr, OutAdjustPanel::OnGBrightnessChange)
	EVT_TEXT(ID_GBrightnessText, OutAdjustPanel::OnGBrightnessText)
	EVT_COMMAND_SCROLL(ID_BBrightnessSldr, OutAdjustPanel::OnBBrightnessChange)
	EVT_TEXT(ID_BBrightnessText, OutAdjustPanel::OnBBrightnessText)
	//set hdr
	EVT_COMMAND_SCROLL(ID_RHdrSldr, OutAdjustPanel::OnRHdrChange)
	EVT_TEXT(ID_RHdrText, OutAdjustPanel::OnRHdrText)
	EVT_COMMAND_SCROLL(ID_GHdrSldr, OutAdjustPanel::OnGHdrChange)
	EVT_TEXT(ID_GHdrText, OutAdjustPanel::OnGHdrText)
	EVT_COMMAND_SCROLL(ID_BHdrSldr, OutAdjustPanel::OnBHdrChange)
	EVT_TEXT(ID_BHdrText, OutAdjustPanel::OnBHdrText)
	//reset
	EVT_BUTTON(ID_RResetBtn, OutAdjustPanel::OnRReset)
	EVT_BUTTON(ID_GResetBtn, OutAdjustPanel::OnGReset)
	EVT_BUTTON(ID_BResetBtn, OutAdjustPanel::OnBReset)
	//set sync
	EVT_TOOL(ID_SyncRChk, OutAdjustPanel::OnSyncRCheck)
	EVT_TOOL(ID_SyncGChk, OutAdjustPanel::OnSyncGCheck)
	EVT_TOOL(ID_SyncBChk, OutAdjustPanel::OnSyncBCheck)
	//set default
	EVT_BUTTON(ID_DefaultBtn, OutAdjustPanel::OnSaveDefault)
END_EVENT_TABLE()

OutAdjustPanel::OutAdjustPanel(RenderFrame* frame,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name) :
wxPanel(frame, wxID_ANY, pos, size, style, name)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);

	m_agent = glbin_agtf->addOutAdjustAgent(gstOutAdjustAgent, *this);

	SetDoubleBuffered(true);

	this->SetSize(75,-1);
	//validator: floating point 2
	wxFloatingPointValidator<double> vald_fp2(2);
	//validator: integer
	wxIntegerValidator<int> vald_int;
	vald_int.SetRange(-256, 256);

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

	//second line: red
	wxBoxSizer *sizer_h_2 = new wxBoxSizer(wxHORIZONTAL);
	m_sync_r_chk = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	st = new wxStaticText(m_sync_r_chk, 0, "Red:",
		wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
	m_sync_r_chk->AddControl(st);
	m_sync_r_chk->AddStretchableSpace();
	wxBitmap bitmap;
	bitmap = wxGetBitmapFromMemory(unlink);
#ifdef _DARWIN
	m_sync_r_chk->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_sync_r_chk->AddCheckTool(ID_SyncRChk, "Link",
		bitmap, wxNullBitmap,
		"Link Red Properties with Linked Green or Blue",
		"Link Red Properties with Linked Green or Blue");
	sizer_h_2->Add(m_sync_r_chk, 1, wxEXPAND);
	m_sync_r_chk->Realize();
	sizer_v->Add(sizer_h_2, 0, wxEXPAND);
	sizer_v->Add(3,3,0);

	//third line: red bar
	st = new wxStaticText(this, 0, "", wxDefaultPosition, wxSize(5,5));
	st->SetBackgroundColour(wxColor(255, 0, 0));
	sizer_v->Add(st, 0, wxEXPAND);

	//fourth line: sliders
	wxBoxSizer *sizer_h_3 = new wxBoxSizer(wxHORIZONTAL);
	m_r_gamma_sldr = new wxSlider(this, ID_RGammaSldr, 100, 10, 400,
		wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL);
	sizer_h_3->Add(m_r_gamma_sldr, 1, wxEXPAND);
	m_r_brightness_sldr = new wxSlider(this, ID_RBrightnessSldr, 0, -256, 256,
		wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL);
	sizer_h_3->Add(m_r_brightness_sldr, 1, wxEXPAND);
	m_r_hdr_sldr = new wxSlider(this, ID_RHdrSldr, 0, 0, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL);
	sizer_h_3->Add(m_r_hdr_sldr, 1, wxEXPAND);
	sizer_v->Add(sizer_h_3, 1, wxEXPAND);

	//fifth line: reset buttons
#ifndef _DARWIN
	m_r_reset_btn = new wxButton(this, ID_RResetBtn, "Reset",
								 wxDefaultPosition, wxSize(30, 22));
#else
	m_r_reset_btn = new wxButton(this, ID_RResetBtn, "Reset",
								 wxDefaultPosition, wxSize(30, 30));
#endif
	m_r_reset_btn->SetBitmap(wxGetBitmapFromMemory(reset));
	sizer_v->Add(m_r_reset_btn, 0, wxEXPAND);

	//6th line: input boxes
	wxBoxSizer *sizer_h_5 = new wxBoxSizer(wxHORIZONTAL);
	vald_fp2.SetRange(0.0, 10.0);
	m_r_gamma_text = new wxTextCtrl(this, ID_RGammaText, "1.00",
		wxDefaultPosition, wxSize(30, 20), 0, vald_fp2);
	sizer_h_5->Add(m_r_gamma_text, 1, wxEXPAND);
	m_r_brightness_text = new wxTextCtrl(this, ID_RBrightnessText, "0",
		wxDefaultPosition, wxSize(30, 20), 0, vald_int);
	sizer_h_5->Add(m_r_brightness_text, 1, wxEXPAND);
	vald_fp2.SetRange(0.0, 1.0);
	m_r_hdr_text = new wxTextCtrl(this, ID_RHdrText, "0.00",
                                  wxDefaultPosition, wxSize(30, 20), 0, vald_fp2);
	sizer_h_5->Add(m_r_hdr_text, 1, wxEXPAND);
	sizer_v->Add(sizer_h_5, 0, wxEXPAND);

	//space
	sizer_v->Add(5, 5, 0);

	//7th line: green
	wxBoxSizer *sizer_h_6 = new wxBoxSizer(wxHORIZONTAL);
	m_sync_g_chk = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	st = new wxStaticText(m_sync_g_chk, 0, "Green:",
		wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
	m_sync_g_chk->AddControl(st);
	m_sync_g_chk->AddStretchableSpace();
	bitmap = wxGetBitmapFromMemory(unlink);
#ifdef _DARWIN
	m_sync_g_chk->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_sync_g_chk->AddCheckTool(ID_SyncGChk, "Link",
		bitmap, wxNullBitmap,
		"Link Green Properties with Linked Red or Blue",
		"Link Green Properties with Linked Red or Blue");
	sizer_h_6->Add(m_sync_g_chk, 1, wxEXPAND);
	m_sync_g_chk->Realize();
	sizer_v->Add(sizer_h_6, 0, wxEXPAND);
	sizer_v->Add(3,3,0);

	//8th line: green bar
	st = new wxStaticText(this, 0, "", wxDefaultPosition, wxSize(5, 5));
	st->SetBackgroundColour(wxColor(0, 255, 0));
	sizer_v->Add(st, 0, wxEXPAND);

	//9th line: sliders
	wxBoxSizer *sizer_h_7 = new wxBoxSizer(wxHORIZONTAL);
	m_g_gamma_sldr = new wxSlider(this, ID_GGammaSldr, 100, 10, 400,
		wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL);
	sizer_h_7->Add(m_g_gamma_sldr, 1, wxEXPAND);
	m_g_brightness_sldr = new wxSlider(this, ID_GBrightnessSldr, 0, -256, 256,
		wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL);
	sizer_h_7->Add(m_g_brightness_sldr, 1, wxEXPAND);
	m_g_hdr_sldr = new wxSlider(this, ID_GHdrSldr, 0, 0, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL);
	sizer_h_7->Add(m_g_hdr_sldr, 1, wxEXPAND);
	sizer_v->Add(sizer_h_7, 1, wxEXPAND);

	//10th line: reset buttons
#ifndef _DARWIN
	m_g_reset_btn = new wxButton(this, ID_GResetBtn, "Reset",
								 wxDefaultPosition, wxSize(30, 22));
#else
	m_g_reset_btn = new wxButton(this, ID_GResetBtn, "Reset",
								 wxDefaultPosition, wxSize(30, 30));
#endif
	m_g_reset_btn->SetBitmap(wxGetBitmapFromMemory(reset));
	sizer_v->Add(m_g_reset_btn, 0, wxEXPAND);

	//11th line: input boxes
	wxBoxSizer *sizer_h_9 = new wxBoxSizer(wxHORIZONTAL);
	vald_fp2.SetRange(0.0, 10.0);
	m_g_gamma_text = new wxTextCtrl(this, ID_GGammaText, "1.00",
		wxDefaultPosition, wxSize(30, 20), 0, vald_fp2);
	sizer_h_9->Add(m_g_gamma_text, 1, wxEXPAND);
	m_g_brightness_text = new wxTextCtrl(this, ID_GBrightnessText, "0",
		wxDefaultPosition, wxSize(30, 20), 0, vald_int);
	sizer_h_9->Add(m_g_brightness_text, 1, wxEXPAND);
	vald_fp2.SetRange(0.0, 1.0);
	m_g_hdr_text = new wxTextCtrl(this, ID_GHdrText, "0.00",
                                  wxDefaultPosition, wxSize(30, 20), 0, vald_fp2);
	sizer_h_9->Add(m_g_hdr_text, 1, wxEXPAND);
	sizer_v->Add(sizer_h_9, 0, wxEXPAND);

	//space
	sizer_v->Add(5, 5, 0);

	//12th line: blue
	wxBoxSizer *sizer_h_10 = new wxBoxSizer(wxHORIZONTAL);
	m_sync_b_chk = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	st = new wxStaticText(m_sync_b_chk, 0, "Blue:",
		wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
	m_sync_b_chk->AddControl(st);
	m_sync_b_chk->AddStretchableSpace();
	bitmap = wxGetBitmapFromMemory(unlink);
#ifdef _DARWIN
	m_sync_b_chk->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_sync_b_chk->AddCheckTool(ID_SyncBChk, "Link",
		bitmap, wxNullBitmap,
		"Link Blue Properties with Linked Red or Green",
		"Link Blue Properties with Linked Red or Green");
	sizer_h_10->Add(m_sync_b_chk, 1, wxEXPAND);
	m_sync_b_chk->Realize();
	sizer_v->Add(sizer_h_10, 0, wxEXPAND);
	sizer_v->Add(3,3,0);

	//13th line:blue bar
	st = new wxStaticText(this, 0, "", wxDefaultPosition, wxSize(5, 5));
	st->SetBackgroundColour(wxColor(0, 0, 255));
	sizer_v->Add(st, 0, wxEXPAND);

	//14th line: sliders
	wxBoxSizer *sizer_h_11 = new wxBoxSizer(wxHORIZONTAL);
	m_b_gamma_sldr = new wxSlider(this, ID_BGammaSldr, 100, 10, 400,
		wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL);
	sizer_h_11->Add(m_b_gamma_sldr, 1, wxEXPAND);
	m_b_brightness_sldr = new wxSlider(this, ID_BBrightnessSldr, 0, -256, 256,
		wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL);
	sizer_h_11->Add(m_b_brightness_sldr, 1, wxEXPAND);
	m_b_hdr_sldr = new wxSlider(this, ID_BHdrSldr, 0, 0, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL);
	sizer_h_11->Add(m_b_hdr_sldr, 1, wxEXPAND);
	sizer_v->Add(sizer_h_11, 1, wxEXPAND);

	//15th line: reset buttons
#ifndef _DARWIN
	m_b_reset_btn = new wxButton(this, ID_BResetBtn, "Reset",
								 wxDefaultPosition, wxSize(30, 22));
#else
	m_b_reset_btn = new wxButton(this, ID_BResetBtn, "Reset",
								 wxDefaultPosition, wxSize(30, 30));
#endif
	m_b_reset_btn->SetBitmap(wxGetBitmapFromMemory(reset));
	sizer_v->Add(m_b_reset_btn, 0, wxEXPAND);

	//16th line: input boxes
	wxBoxSizer *sizer_h_13 = new wxBoxSizer(wxHORIZONTAL);
	vald_fp2.SetRange(0.0, 10.0);
	m_b_gamma_text = new wxTextCtrl(this, ID_BGammaText, "1.00",
		wxDefaultPosition, wxSize(30, 20), 0, vald_fp2);
	sizer_h_13->Add(m_b_gamma_text, 1, wxEXPAND);
	m_b_brightness_text = new wxTextCtrl(this, ID_BBrightnessText, "0",
		wxDefaultPosition, wxSize(30, 20), 0, vald_int);
	sizer_h_13->Add(m_b_brightness_text, 1, wxEXPAND);
	vald_fp2.SetRange(0.0, 1.0);
	m_b_hdr_text = new wxTextCtrl(this, ID_BHdrText, "0.00",
                                  wxDefaultPosition, wxSize(30, 20), 0, vald_fp2);
	sizer_h_13->Add(m_b_hdr_text, 1, wxEXPAND);
	sizer_v->Add(sizer_h_13, 0, wxEXPAND);

	//17th line: default button
#ifndef _DARWIN
	m_dft_btn = new wxButton(this, ID_DefaultBtn, "Set Default",
							 wxDefaultPosition, wxSize(95, 22));
#else
	m_dft_btn = new wxButton(this, ID_DefaultBtn, "Set Default",
							 wxDefaultPosition, wxSize(95, 30));
#endif
	m_dft_btn->SetBitmap(wxGetBitmapFromMemory(save_settings));
	sizer_v->Add(m_dft_btn, 0, wxEXPAND);

	SetSizer(sizer_v);
	Layout();

	DisableAll();
}

OutAdjustPanel::~OutAdjustPanel()
{
}

void OutAdjustPanel::AssociateNode(fluo::Node* node)
{
	m_agent->setObject(node);
}

void OutAdjustPanel::DisableAll()
{
	//red
	m_sync_r_chk->Disable();
	m_r_gamma_sldr->Disable();
	m_r_brightness_sldr->Disable();
	m_r_hdr_sldr->Disable();
	m_r_gamma_text->Disable();
	m_r_brightness_text->Disable();
	m_r_hdr_text->Disable();
	//green
	m_sync_g_chk->Disable();
	m_g_gamma_sldr->Disable();
	m_g_brightness_sldr->Disable();
	m_g_hdr_sldr->Disable();
	m_g_gamma_text->Disable();
	m_g_brightness_text->Disable();
	m_g_hdr_text->Disable();
	//blue
	m_sync_b_chk->Disable();
	m_b_gamma_sldr->Disable();
	m_b_brightness_sldr->Disable();
	m_b_hdr_sldr->Disable();
	m_b_gamma_text->Disable();
	m_b_brightness_text->Disable();
	m_b_hdr_text->Disable();
	//reset
	m_r_reset_btn->Disable();
	m_g_reset_btn->Disable();
	m_b_reset_btn->Disable();
	//save as default
	m_dft_btn->Disable();
}

void OutAdjustPanel::EnableAll()
{
	//red
	m_sync_r_chk->Enable();
	m_r_gamma_sldr->Enable();
	m_r_brightness_sldr->Enable();
	m_r_hdr_sldr->Enable();
	m_r_gamma_text->Enable();
	m_r_brightness_text->Enable();
	m_r_hdr_text->Enable();
	//green
	m_sync_g_chk->Enable();
	m_g_gamma_sldr->Enable();
	m_g_brightness_sldr->Enable();
	m_g_hdr_sldr->Enable();
	m_g_gamma_text->Enable();
	m_g_brightness_text->Enable();
	m_g_hdr_text->Enable();
	//blue
	m_sync_b_chk->Enable();
	m_b_gamma_sldr->Enable();
	m_b_brightness_sldr->Enable();
	m_b_hdr_sldr->Enable();
	m_b_gamma_text->Enable();
	m_b_brightness_text->Enable();
	m_b_hdr_text->Enable();
	//reset
	m_r_reset_btn->Enable();
	m_g_reset_btn->Enable();
	m_b_reset_btn->Enable();
	//save as default
	m_dft_btn->Enable();
}

void OutAdjustPanel::OnRGammaChange(wxScrollEvent & event)
{
	double val = (double)event.GetPosition() / 100.0;
	wxString str = wxString::Format("%.2f", val);
	m_r_gamma_text->SetValue(str);
}

void OutAdjustPanel::OnRGammaText(wxCommandEvent& event)
{
	wxString str = m_r_gamma_text->GetValue();
	double val;
	if (str.ToDouble(&val))
	{
		m_r_gamma_sldr->SetValue(int(val * 100));
		m_agent->updateValue(gstGammaR, 1 / val);
	}
}

void OutAdjustPanel::OnGGammaChange(wxScrollEvent & event)
{
	double val = (double)event.GetPosition() / 100.0;
	wxString str = wxString::Format("%.2f", val);
	m_g_gamma_text->SetValue(str);
}

void OutAdjustPanel::OnGGammaText(wxCommandEvent& event)
{
	wxString str = m_g_gamma_text->GetValue();
	double val;
	if (str.ToDouble(&val))
	{
		m_g_gamma_sldr->SetValue(int(val * 100));
		m_agent->updateValue(gstGammaG, 1 / val);
	}
}

void OutAdjustPanel::OnBGammaChange(wxScrollEvent & event)
{
	double val = (double)event.GetPosition() / 100.0;
	wxString str = wxString::Format("%.2f", val);
	m_b_gamma_text->SetValue(str);
}

void OutAdjustPanel::OnBGammaText(wxCommandEvent& event)
{
	wxString str = m_b_gamma_text->GetValue();
	double val;
	if (str.ToDouble(&val))
	{
		m_b_gamma_sldr->SetValue(int(val * 100));
		m_agent->updateValue(gstGammaB, 1 / val);
	}
}

//brightness
void OutAdjustPanel::OnRBrightnessChange(wxScrollEvent & event)
{
	double val = (double)event.GetPosition();
	wxString str = wxString::Format("%d", int(val));
	m_r_brightness_text->SetValue(str);
}

void OutAdjustPanel::OnRBrightnessText(wxCommandEvent& event)
{
	wxString str = m_r_brightness_text->GetValue();
	double val;
	if (str.ToDouble(&val))
	{
		m_r_brightness_sldr->SetValue(int(val));
		m_agent->updateValue(gstBrightnessR, val / 256 + 1);
	}
}

void OutAdjustPanel::OnGBrightnessChange(wxScrollEvent & event)
{
	double val = (double)event.GetPosition();
	wxString str = wxString::Format("%d", int(val));
	m_g_brightness_text->SetValue(str);
}

void OutAdjustPanel::OnGBrightnessText(wxCommandEvent& event)
{
	wxString str = m_g_brightness_text->GetValue();
	double val;
	if (str.ToDouble(&val))
	{
		m_g_brightness_sldr->SetValue(int(val));
		m_agent->updateValue(gstBrightnessG, val / 256 + 1);
	}
}

void OutAdjustPanel::OnBBrightnessChange(wxScrollEvent & event)
{
	double val = (double)event.GetPosition();
	wxString str = wxString::Format("%d", int(val));
	m_b_brightness_text->SetValue(str);
}

void OutAdjustPanel::OnBBrightnessText(wxCommandEvent& event)
{
	wxString str = m_b_brightness_text->GetValue();
	double val;
	if (str.ToDouble(&val))
	{
		m_b_brightness_sldr->SetValue(int(val));
		m_agent->updateValue(gstBrightnessB, double(val / 256.0 + 1.0));
	}
}

void OutAdjustPanel::OnRHdrChange(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 100.0;
	wxString str = wxString::Format("%.2f", val);
	m_r_hdr_text->SetValue(str);
}

void OutAdjustPanel::OnRHdrText(wxCommandEvent &event)
{
	wxString str = m_r_hdr_text->GetValue();
	double val;
	if (str.ToDouble(&val))
	{
		m_r_hdr_sldr->SetValue(int(val * 100));
		m_agent->updateValue(gstEqualizeR, val);
	}
}

void OutAdjustPanel::OnGHdrChange(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 100.0;
	wxString str = wxString::Format("%.2f", val);
	m_g_hdr_text->SetValue(str);
}

void OutAdjustPanel::OnGHdrText(wxCommandEvent &event)
{
	wxString str = m_g_hdr_text->GetValue();
	double val;
	if (str.ToDouble(&val))
	{
		m_g_hdr_sldr->SetValue(int(val * 100));
		m_agent->updateValue(gstEqualizeG, val);
	}
}

void OutAdjustPanel::OnBHdrChange(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 100.0;
	wxString str = wxString::Format("%.2f", val);
	m_b_hdr_text->SetValue(str);
}

void OutAdjustPanel::OnBHdrText(wxCommandEvent &event)
{
	wxString str = m_b_hdr_text->GetValue();
	double val;
	if (str.ToDouble(&val))
	{
		m_b_hdr_sldr->SetValue(int(val * 100));
		m_agent->updateValue(gstEqualizeB, val);
	}
}

void OutAdjustPanel::OnSyncRCheck(wxCommandEvent &event)
{
	bool bval = m_sync_r_chk->GetToolState(ID_SyncRChk);
	m_sync_r_chk->SetToolNormalBitmap(ID_SyncRChk,
		bval ? wxGetBitmapFromMemory(link) : wxGetBitmapFromMemory(unlink));
	m_agent->updateValue(gstSyncR, bval);
}

void OutAdjustPanel::OnSyncGCheck(wxCommandEvent &event)
{
	bool bval = m_sync_g_chk->GetToolState(ID_SyncGChk);
	m_sync_g_chk->SetToolNormalBitmap(ID_SyncGChk,
		bval ? wxGetBitmapFromMemory(link) : wxGetBitmapFromMemory(unlink));
	m_agent->updateValue(gstSyncG, bval);
}

void OutAdjustPanel::OnSyncBCheck(wxCommandEvent &event)
{
	bool bval = m_sync_b_chk->GetToolState(ID_SyncBChk);
	m_sync_b_chk->SetToolNormalBitmap(ID_SyncBChk,
		bval ? wxGetBitmapFromMemory(link) : wxGetBitmapFromMemory(unlink));
	m_agent->updateValue(gstSyncB, bval);
}

void OutAdjustPanel::OnRReset(wxCommandEvent &event)
{
	fluo::ValueCollection names{ gstGammaR, gstBrightnessR, gstEqualizeR };
	glbin_volf->propValuesFromDefault(m_agent, names);
}

void OutAdjustPanel::OnGReset(wxCommandEvent &event)
{
	fluo::ValueCollection names{ gstGammaB, gstBrightnessB, gstEqualizeB };
	glbin_volf->propValuesFromDefault(m_agent, names);
}

void OutAdjustPanel::OnBReset(wxCommandEvent &event)
{
	fluo::ValueCollection names{ gstGammaB, gstBrightnessB, gstEqualizeB };
	glbin_volf->propValuesFromDefault(m_agent, names);
}

void OutAdjustPanel::OnSaveDefault(wxCommandEvent &event)
{
	std::string ss[] = {
		gstGammaR, gstGammaG, gstGammaB,
		gstBrightnessR, gstBrightnessG, gstBrightnessB,
		gstEqualizeR, gstEqualizeG, gstEqualizeB };
	fluo::ValueCollection names(std::begin(ss), std::end(ss));//values to save
	glbin_volf->propValuesToDefault(m_agent, names);
	glbin_volf->writeDefault(names);
}

