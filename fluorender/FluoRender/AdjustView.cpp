#include "AdjustView.h"
#include "VRenderFrame.h"
#include <wx/valnum.h>
#include "png_resource.h"
#include "img/listicon_save.h"
#include "img/refresh.h"
#include "img/link.h"
#include "img/unlink.h"

BEGIN_EVENT_TABLE(AdjustView, wxPanel)
	//set gamme
	EVT_COMMAND_SCROLL(ID_RGammaSldr, AdjustView::OnRGammaChange)
	EVT_TEXT(ID_RGammaText, AdjustView::OnRGammaText)
	EVT_COMMAND_SCROLL(ID_GGammaSldr, AdjustView::OnGGammaChange)
	EVT_TEXT(ID_GGammaText, AdjustView::OnGGammaText)
	EVT_COMMAND_SCROLL(ID_BGammaSldr, AdjustView::OnBGammaChange)
	EVT_TEXT(ID_BGammaText, AdjustView::OnBGammaText)
	//set brightness
	EVT_COMMAND_SCROLL(ID_RBrightnessSldr, AdjustView::OnRBrightnessChange)
	EVT_TEXT(ID_RBrightnessText, AdjustView::OnRBrightnessText)
	EVT_COMMAND_SCROLL(ID_GBrightnessSldr, AdjustView::OnGBrightnessChange)
	EVT_TEXT(ID_GBrightnessText, AdjustView::OnGBrightnessText)
	EVT_COMMAND_SCROLL(ID_BBrightnessSldr, AdjustView::OnBBrightnessChange)
	EVT_TEXT(ID_BBrightnessText, AdjustView::OnBBrightnessText)
	//set hdr
	EVT_COMMAND_SCROLL(ID_RHdrSldr, AdjustView::OnRHdrChange)
	EVT_TEXT(ID_RHdrText, AdjustView::OnRHdrText)
	EVT_COMMAND_SCROLL(ID_GHdrSldr, AdjustView::OnGHdrChange)
	EVT_TEXT(ID_GHdrText, AdjustView::OnGHdrText)
	EVT_COMMAND_SCROLL(ID_BHdrSldr, AdjustView::OnBHdrChange)
	EVT_TEXT(ID_BHdrText, AdjustView::OnBHdrText)
	//reset
	EVT_BUTTON(ID_RResetBtn, AdjustView::OnRReset)
	EVT_BUTTON(ID_GResetBtn, AdjustView::OnGReset)
	EVT_BUTTON(ID_BResetBtn, AdjustView::OnBReset)
	//set sync
	EVT_TOOL(ID_SyncRChk, AdjustView::OnSyncRCheck)
	EVT_TOOL(ID_SyncGChk, AdjustView::OnSyncGCheck)
	EVT_TOOL(ID_SyncBChk, AdjustView::OnSyncBCheck)
	//set default
	EVT_BUTTON(ID_DefaultBtn, AdjustView::OnSaveDefault)
END_EVENT_TABLE()

AdjustView::AdjustView(wxWindow* frame,
					   wxWindow* parent,
					   wxWindowID id,
					   const wxPoint& pos,
					   const wxSize& size,
					   long style,
					   const wxString& name):
wxPanel(parent, id, pos, size, style, name),
m_frame(frame),
m_type(-1),
m_glview(0),
m_vd(0),
m_group(0),
m_link_group(false),
m_sync_r(false),
m_sync_g(false),
m_sync_b(false),
m_use_dft_settings(false),
m_dft_gamma(Color(1.0, 1.0, 1.0)),
m_dft_brightness(Color(0.0, 0.0, 0.0)),
m_dft_hdr(Color(0.0, 0.0, 0.0)),
m_dft_sync_r(false),
m_dft_sync_g(false),
m_dft_sync_b(false)
{
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
	sizer_h_1->Add(st, 1, wxEXPAND|wxALIGN_CENTER);
	st = new wxStaticText(this, 0, "Lum.",
		wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
	sizer_h_1->Add(st, 1, wxEXPAND|wxALIGN_CENTER);
	st = new wxStaticText(this, 0, "Eql.",
                          wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
#ifdef _WIN32
	sizer_h_1->Add(st, 1, wxEXPAND|wxALIGN_CENTER);
#else
    st->Hide();
#endif
	sizer_v->Add(sizer_h_1, 0, wxEXPAND);
	//space
	sizer_v->Add(5, 5, 0);

	//second line: red
	wxBoxSizer *sizer_h_2 = new wxBoxSizer(wxHORIZONTAL);
	m_sync_r_chk = new wxToolBar(this, wxID_ANY);
	st = new wxStaticText(m_sync_r_chk, 0, "Red:",
		wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
	m_sync_r_chk->AddControl(st);
	m_sync_r_chk->AddStretchableSpace();
	m_sync_r_chk->AddCheckTool(ID_SyncRChk, "Link",
		wxGetBitmapFromMemory(unlink), wxNullBitmap,
		"Link Red Properties with Linked Green or Blue",
		"Link Red Properties with Linked Green or Blue");
	sizer_h_2->Add(m_sync_r_chk, 1, wxEXPAND|wxALIGN_RIGHT);
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
		wxDefaultPosition, wxSize(25,-1), wxSL_VERTICAL);
	sizer_h_3->Add(m_r_gamma_sldr, 1, wxEXPAND|wxALIGN_CENTER);
	m_r_brightness_sldr = new wxSlider(this, ID_RBrightnessSldr, 0, -256, 256,
		wxDefaultPosition, wxSize(25,-1), wxSL_VERTICAL);
	sizer_h_3->Add(m_r_brightness_sldr, 1, wxEXPAND|wxALIGN_CENTER);
    
	m_r_hdr_sldr = new wxSlider(this, ID_RHdrSldr, 0, 0, 100,
                                wxDefaultPosition, wxSize(25,-1), wxSL_VERTICAL);
#ifdef _WIN32
	sizer_h_3->Add(m_r_hdr_sldr, 1, wxEXPAND|wxALIGN_CENTER);
#else
    m_r_hdr_sldr->Hide();
#endif
	sizer_v->Add(sizer_h_3, 1, wxEXPAND);

	//fifth line: reset buttons
#ifndef _DARWIN
	m_r_reset_btn = new wxButton(this, ID_RResetBtn, "Reset",
								 wxDefaultPosition, wxSize(30, 22));
#else
	m_r_reset_btn = new wxButton(this, ID_RResetBtn, "Reset",
								 wxDefaultPosition, wxSize(30, 30));
#endif
	m_r_reset_btn->SetBitmap(wxGetBitmapFromMemory(refresh));
	sizer_v->Add(m_r_reset_btn, 0, wxEXPAND);

	//6th line: input boxes
	wxBoxSizer *sizer_h_5 = new wxBoxSizer(wxHORIZONTAL);
	vald_fp2.SetRange(0.0, 10.0);
	m_r_gamma_text = new wxTextCtrl(this, ID_RGammaText, "1.00",
		wxDefaultPosition, wxSize(30, 20), 0, vald_fp2);
	sizer_h_5->Add(m_r_gamma_text, 1, wxEXPAND|wxALIGN_CENTER);
	m_r_brightness_text = new wxTextCtrl(this, ID_RBrightnessText, "0",
		wxDefaultPosition, wxSize(30, 20), 0, vald_int);
	sizer_h_5->Add(m_r_brightness_text, 1, wxEXPAND|wxALIGN_CENTER);
	vald_fp2.SetRange(0.0, 1.0);
	m_r_hdr_text = new wxTextCtrl(this, ID_RHdrText, "0.00",
                                  wxDefaultPosition, wxSize(30, 20), 0, vald_fp2);
#ifdef _WIN32
	sizer_h_5->Add(m_r_hdr_text, 1, wxEXPAND|wxALIGN_CENTER);
#else
    m_r_hdr_text->Hide();
#endif
	sizer_v->Add(sizer_h_5, 0, wxEXPAND);

	//space
	sizer_v->Add(5, 5, 0);

	//7th line: green
	wxBoxSizer *sizer_h_6 = new wxBoxSizer(wxHORIZONTAL);
	m_sync_g_chk = new wxToolBar(this, wxID_ANY);
	st = new wxStaticText(m_sync_g_chk, 0, "Green:",
		wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
	m_sync_g_chk->AddControl(st);
	m_sync_g_chk->AddStretchableSpace();
	m_sync_g_chk->AddCheckTool(ID_SyncGChk, "Link",
		wxGetBitmapFromMemory(unlink), wxNullBitmap,
		"Link Green Properties with Linked Red or Blue",
		"Link Green Properties with Linked Red or Blue");
	sizer_h_6->Add(m_sync_g_chk, 1, wxEXPAND|wxALIGN_RIGHT);
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
		wxDefaultPosition, wxSize(25,-1), wxSL_VERTICAL);
	sizer_h_7->Add(m_g_gamma_sldr, 1, wxEXPAND|wxALIGN_CENTER);
	m_g_brightness_sldr = new wxSlider(this, ID_GBrightnessSldr, 0, -256, 256,
		wxDefaultPosition, wxSize(25,-1), wxSL_VERTICAL);
	sizer_h_7->Add(m_g_brightness_sldr, 1, wxEXPAND|wxALIGN_CENTER);
	m_g_hdr_sldr = new wxSlider(this, ID_GHdrSldr, 0, 0, 100,
                                wxDefaultPosition, wxSize(25,-1), wxSL_VERTICAL);
#ifdef _WIN32
	sizer_h_7->Add(m_g_hdr_sldr, 1, wxEXPAND|wxALIGN_CENTER);
#else
    m_g_hdr_sldr->Hide();
#endif
	sizer_v->Add(sizer_h_7, 1, wxEXPAND);

	//10th line: reset buttons
#ifndef _DARWIN
	m_g_reset_btn = new wxButton(this, ID_GResetBtn, "Reset",
								 wxDefaultPosition, wxSize(30, 22));
#else
	m_g_reset_btn = new wxButton(this, ID_GResetBtn, "Reset",
								 wxDefaultPosition, wxSize(30, 30));
#endif
	m_g_reset_btn->SetBitmap(wxGetBitmapFromMemory(refresh));
	sizer_v->Add(m_g_reset_btn, 0, wxEXPAND);

	//11th line: input boxes
	wxBoxSizer *sizer_h_9 = new wxBoxSizer(wxHORIZONTAL);
	vald_fp2.SetRange(0.0, 10.0);
	m_g_gamma_text = new wxTextCtrl(this, ID_GGammaText, "1.00",
		wxDefaultPosition, wxSize(30, 20), 0, vald_fp2);
	sizer_h_9->Add(m_g_gamma_text, 1, wxEXPAND|wxALIGN_CENTER);
	m_g_brightness_text = new wxTextCtrl(this, ID_GBrightnessText, "0",
		wxDefaultPosition, wxSize(30, 20), 0, vald_int);
	sizer_h_9->Add(m_g_brightness_text, 1, wxEXPAND|wxALIGN_CENTER);
	vald_fp2.SetRange(0.0, 1.0);
	m_g_hdr_text = new wxTextCtrl(this, ID_GHdrText, "0.00",
                                  wxDefaultPosition, wxSize(30, 20), 0, vald_fp2);
#ifdef _WIN32
	sizer_h_9->Add(m_g_hdr_text, 1, wxEXPAND|wxALIGN_CENTER);
#else
    m_g_hdr_text->Hide();
#endif
	sizer_v->Add(sizer_h_9, 0, wxEXPAND);

	//space
	sizer_v->Add(5, 5, 0);

	//12th line: blue
	wxBoxSizer *sizer_h_10 = new wxBoxSizer(wxHORIZONTAL);
	m_sync_b_chk = new wxToolBar(this, wxID_ANY);
	st = new wxStaticText(m_sync_b_chk, 0, "Blue:",
		wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
	m_sync_b_chk->AddControl(st);
	m_sync_b_chk->AddStretchableSpace();
	m_sync_b_chk->AddCheckTool(ID_SyncBChk, "Link",
		wxGetBitmapFromMemory(unlink), wxNullBitmap,
		"Link Blue Properties with Linked Red or Green",
		"Link Blue Properties with Linked Red or Green");
	sizer_h_10->Add(m_sync_b_chk, 1, wxEXPAND|wxALIGN_RIGHT);
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
		wxDefaultPosition, wxSize(25,-1), wxSL_VERTICAL);
	sizer_h_11->Add(m_b_gamma_sldr, 1, wxEXPAND|wxALIGN_CENTER);
	m_b_brightness_sldr = new wxSlider(this, ID_BBrightnessSldr, 0, -256, 256,
		wxDefaultPosition, wxSize(25,-1), wxSL_VERTICAL);
	sizer_h_11->Add(m_b_brightness_sldr, 1, wxEXPAND|wxALIGN_CENTER);
	m_b_hdr_sldr = new wxSlider(this, ID_BHdrSldr, 0, 0, 100,
                                wxDefaultPosition, wxSize(25,-1), wxSL_VERTICAL);
#ifdef _WIN32
	sizer_h_11->Add(m_b_hdr_sldr, 1, wxEXPAND|wxALIGN_CENTER);
#else
    m_b_hdr_sldr->Hide();
#endif
	sizer_v->Add(sizer_h_11, 1, wxEXPAND);
    

	//15th line: reset buttons
#ifndef _DARWIN
	m_b_reset_btn = new wxButton(this, ID_BResetBtn, "Reset",
								 wxDefaultPosition, wxSize(30, 22));
#else
	m_b_reset_btn = new wxButton(this, ID_BResetBtn, "Reset",
								 wxDefaultPosition, wxSize(30, 30));
#endif
	m_b_reset_btn->SetBitmap(wxGetBitmapFromMemory(refresh));
	sizer_v->Add(m_b_reset_btn, 0, wxEXPAND);

	//16th line: input boxes
	wxBoxSizer *sizer_h_13 = new wxBoxSizer(wxHORIZONTAL);
	vald_fp2.SetRange(0.0, 10.0);
	m_b_gamma_text = new wxTextCtrl(this, ID_BGammaText, "1.00",
		wxDefaultPosition, wxSize(30, 20), 0, vald_fp2);
	sizer_h_13->Add(m_b_gamma_text, 1, wxEXPAND|wxALIGN_CENTER);
	m_b_brightness_text = new wxTextCtrl(this, ID_BBrightnessText, "0",
		wxDefaultPosition, wxSize(30, 20), 0, vald_int);
	sizer_h_13->Add(m_b_brightness_text, 1, wxEXPAND|wxALIGN_CENTER);
	vald_fp2.SetRange(0.0, 1.0);
	m_b_hdr_text = new wxTextCtrl(this, ID_BHdrText, "0.00",
                                  wxDefaultPosition, wxSize(30, 20), 0, vald_fp2);
#ifdef _WIN32
	sizer_h_13->Add(m_b_hdr_text, 1, wxEXPAND|wxALIGN_CENTER);
#else
    m_b_hdr_text->Hide();
#endif
	sizer_v->Add(sizer_h_13, 0, wxEXPAND);

	//17th line: default button
#ifndef _DARWIN
	m_dft_btn = new wxButton(this, ID_DefaultBtn, "Save as Default",
							 wxDefaultPosition, wxSize(95, 22));
#else
	m_dft_btn = new wxButton(this, ID_DefaultBtn, "Set Default",
							 wxDefaultPosition, wxSize(95, 30));
#endif
	m_dft_btn->SetBitmap(wxGetBitmapFromMemory(listicon_save));
	sizer_v->Add(m_dft_btn, 0, wxEXPAND);

	SetSizer(sizer_v);
	Layout();

	DisableAll();

	LoadSettings();
}

AdjustView::~AdjustView()
{
}

void AdjustView::RefreshVRenderViews(bool interactive)
{
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	vr_frame->RefreshVRenderViews(false, interactive);
}

void AdjustView::GetSettings()
{
	//red
	bool sync_r = false;
	double r_gamma = 1.0;
	double r_brightness = 0.0;
	double r_hdr = 0.0;
	//green
	bool sync_g = false;
	double g_gamma = 1.0;
	double g_brightness = 0.0;
	double g_hdr = 0.0;
	//blue
	bool sync_b = false;
	double b_gamma = 1.0;
	double b_brightness = 0.0;
	double b_hdr = 0.0;

	switch (m_type)
	{
	case 1://view
		if (m_glview)
		{
			//red
			sync_r = m_glview->GetSyncR();
			Gamma2UI(m_glview->GetGamma().r(), r_gamma);
			Brightness2UI(m_glview->GetBrightness().r(), r_brightness);
			Hdr2UI(m_glview->GetHdr().r(), r_hdr);
			//green
			sync_g = m_glview->GetSyncG();
			Gamma2UI(m_glview->GetGamma().g(), g_gamma);
			Brightness2UI(m_glview->GetBrightness().g(), g_brightness);
			Hdr2UI(m_glview->GetHdr().g(), g_hdr);
			//blue
			sync_b = m_glview->GetSyncB();
			Gamma2UI(m_glview->GetGamma().b(), b_gamma);
			Brightness2UI(m_glview->GetBrightness().b(), b_brightness);
			Hdr2UI(m_glview->GetHdr().b(), b_hdr);
		}
		break;
	case 2://volume data
	case 5://group
		{
			TreeLayer *layer = 0;
			if (m_type == 2 && m_vd)
				layer = (TreeLayer*)m_vd;
			else if (m_type == 5 && m_group)
				layer = (TreeLayer*)m_group;

			if (layer)
			{
				//red
				sync_r = layer->GetSyncR();
				Gamma2UI(layer->GetGamma().r(), r_gamma);
				Brightness2UI(layer->GetBrightness().r(), r_brightness);
				Hdr2UI(layer->GetHdr().r(), r_hdr);
				//green
				sync_g = layer->GetSyncG();
				Gamma2UI(layer->GetGamma().g(), g_gamma);
				Brightness2UI(layer->GetBrightness().g(), g_brightness);
				Hdr2UI(layer->GetHdr().g(), g_hdr);
				//blue
				sync_b = layer->GetSyncB();
				Gamma2UI(layer->GetGamma().b(), b_gamma);
				Brightness2UI(layer->GetBrightness().b(), b_brightness);
				Hdr2UI(layer->GetHdr().b(), b_hdr);
			}
		}
		break;
	}

	if (m_type == 1 || m_type == 2 || m_type == 5)
	{
		//red
		m_sync_r_chk->ToggleTool(ID_SyncRChk,sync_r);
		m_sync_r_chk->SetToolNormalBitmap(ID_SyncRChk,
			sync_r?wxGetBitmapFromMemory(link):wxGetBitmapFromMemory(unlink));
		m_sync_r = sync_r;
		m_r_gamma_sldr->SetValue(Gamma2UIP(r_gamma));
		m_r_brightness_sldr->SetValue(Brightness2UIP(r_brightness));
		m_r_hdr_sldr->SetValue(Hdr2UIP(r_hdr));
		m_r_gamma_text->ChangeValue(wxString::Format("%.2f", r_gamma));
		m_r_brightness_text->ChangeValue(wxString::Format("%d", Brightness2UIP(r_brightness)));
		m_r_hdr_text->ChangeValue(wxString::Format("%.2f", r_hdr));
		//green
		m_sync_g_chk->ToggleTool(ID_SyncGChk,sync_g);
		m_sync_g_chk->SetToolNormalBitmap(ID_SyncGChk,
			sync_g?wxGetBitmapFromMemory(link):wxGetBitmapFromMemory(unlink));
		m_sync_g = sync_g;
		m_g_gamma_sldr->SetValue(Gamma2UIP(g_gamma));
		m_g_brightness_sldr->SetValue(Brightness2UIP(g_brightness));
		m_g_hdr_sldr->SetValue(Hdr2UIP(g_hdr));
		m_g_gamma_text->ChangeValue(wxString::Format("%.2f", g_gamma));
		m_g_brightness_text->ChangeValue(wxString::Format("%d", int(g_brightness)));
		m_g_hdr_text->ChangeValue(wxString::Format("%.2f", g_hdr));
		//blue
		m_sync_b_chk->ToggleTool(ID_SyncBChk,sync_b);
		m_sync_b_chk->SetToolNormalBitmap(ID_SyncBChk,
			sync_b?wxGetBitmapFromMemory(link):wxGetBitmapFromMemory(unlink));
		m_sync_b = sync_b;
		m_b_gamma_sldr->SetValue(Gamma2UIP(b_gamma));
		m_b_brightness_sldr->SetValue(Brightness2UIP(b_brightness));
		m_b_hdr_sldr->SetValue(Hdr2UIP(b_hdr));
		m_b_gamma_text->ChangeValue(wxString::Format("%.2f", b_gamma));
		m_b_brightness_text->ChangeValue(wxString::Format("%d", Brightness2UIP(b_brightness)));
		m_b_hdr_text->ChangeValue(wxString::Format("%.2f", b_hdr));
		EnableAll();
	}
	else
		DisableAll();
}

void AdjustView::DisableAll()
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

void AdjustView::EnableAll()
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

void AdjustView::OnRGammaChange(wxScrollEvent & event)
{
	double val = (double)event.GetPosition() / 100.0;
	wxString str = wxString::Format("%.2f", val);
	m_r_gamma_text->SetValue(str);
}

void AdjustView::OnRGammaText(wxCommandEvent& event)
{
	wxString str = m_r_gamma_text->GetValue();
	double val;
	str.ToDouble(&val);
	m_r_gamma_sldr->SetValue(int(val*100));

	if (m_sync_r)
	{
		if (m_sync_g)
		{
			m_g_gamma_sldr->SetValue(int(val*100));
			m_g_gamma_text->ChangeValue(str);
		}
		if (m_sync_b)
		{
			m_b_gamma_sldr->SetValue(int(val*100));
			m_b_gamma_text->ChangeValue(str);
		}
	}

	if (m_glview)
	{
		Color gamma = m_glview->GetGamma();
		GammaUI2(val, gamma[0]);
		if (m_sync_r)
		{
			if (m_sync_g)
				GammaUI2(val, gamma[1]);
			if (m_sync_b)
				GammaUI2(val, gamma[2]);
		}
		m_glview->SetGamma(gamma);
	}

	TreeLayer *layer = 0;
	if (m_type == 2 && m_vd)
		layer = (TreeLayer*)m_vd;
	else if (m_type == 5 && m_group)
		layer = (TreeLayer*)m_group;
	if (layer)
	{
		Color gamma = layer->GetGamma();
		GammaUI2(val, gamma[0]);
		if (m_sync_r)
		{
			if (m_sync_g)
				GammaUI2(val, gamma[1]);
			if (m_sync_b)
				GammaUI2(val, gamma[2]);
		}
		layer->SetGamma(gamma);

		if (m_link_group && m_group)
			m_group->SetGammaAll(gamma);

		RefreshVRenderViews(true);
	}
}

void AdjustView::OnGGammaChange(wxScrollEvent & event)
{
	double val = (double)event.GetPosition() / 100.0;
	wxString str = wxString::Format("%.2f", val);
	m_g_gamma_text->SetValue(str);
}

void AdjustView::OnGGammaText(wxCommandEvent& event)
{
	wxString str = m_g_gamma_text->GetValue();
	double val;
	str.ToDouble(&val);
	m_g_gamma_sldr->SetValue(int(val*100));

	if (m_sync_g)
	{
		if (m_sync_r)
		{
			m_r_gamma_sldr->SetValue(int(val*100));
			m_r_gamma_text->ChangeValue(str);
		}
		if (m_sync_b)
		{
			m_b_gamma_sldr->SetValue(int(val*100));
			m_b_gamma_text->ChangeValue(str);
		}
	}

	if (m_glview)
	{
		Color gamma = m_glview->GetGamma();
		GammaUI2(val, gamma[1]);
		if (m_sync_g)
		{
			if (m_sync_r)
				GammaUI2(val, gamma[0]);
			if (m_sync_b)
				GammaUI2(val, gamma[2]);
		}
		m_glview->SetGamma(gamma);
	}

	TreeLayer *layer = 0;
	if (m_type == 2 && m_vd)
		layer = (TreeLayer*)m_vd;
	else if (m_type == 5 && m_group)
		layer = (TreeLayer*)m_group;
	if (layer)
	{
		Color gamma = layer->GetGamma();
		GammaUI2(val, gamma[1]);
		if (m_sync_g)
		{
			if (m_sync_r)
				GammaUI2(val, gamma[0]);
			if (m_sync_b)
				GammaUI2(val, gamma[2]);
		}
		layer->SetGamma(gamma);

		if (m_link_group && m_group)
			m_group->SetGammaAll(gamma);

		RefreshVRenderViews(true);
	}
}

void AdjustView::OnBGammaChange(wxScrollEvent & event)
{
	double val = (double)event.GetPosition() / 100.0;
	wxString str = wxString::Format("%.2f", val);
	m_b_gamma_text->SetValue(str);
}

void AdjustView::OnBGammaText(wxCommandEvent& event)
{
	wxString str = m_b_gamma_text->GetValue();
	double val;
	str.ToDouble(&val);
	m_b_gamma_sldr->SetValue(int(val*100));

	if (m_sync_b)
	{
		if (m_sync_r)
		{
			m_r_gamma_sldr->SetValue(int(val*100));
			m_r_gamma_text->ChangeValue(str);
		}
		if (m_sync_g)
		{
			m_g_gamma_sldr->SetValue(int(val*100));
			m_g_gamma_text->ChangeValue(str);
		}
	}

	if (m_glview)
	{
		Color gamma = m_glview->GetGamma();
		GammaUI2(val, gamma[2]);
		if (m_sync_b)
		{
			if (m_sync_r)
				GammaUI2(val, gamma[0]);
			if (m_sync_g)
				GammaUI2(val, gamma[1]);
		}
		m_glview->SetGamma(gamma);
	}

	TreeLayer *layer = 0;
	if (m_type == 2 && m_vd)
		layer = (TreeLayer*)m_vd;
	else if (m_type == 5 && m_group)
		layer = (TreeLayer*)m_group;
	if (layer)
	{
		Color gamma = layer->GetGamma();
		GammaUI2(val, gamma[2]);
		if (m_sync_b)
		{
			if (m_sync_r)
				GammaUI2(val, gamma[0]);
			if (m_sync_g)
				GammaUI2(val, gamma[1]);
		}
		layer->SetGamma(gamma);

		if (m_link_group && m_group)
			m_group->SetGammaAll(gamma);

		RefreshVRenderViews(true);
	}
}

//brightness
void AdjustView::OnRBrightnessChange(wxScrollEvent & event)
{
	double val = (double)event.GetPosition();
	wxString str = wxString::Format("%d", int(val));
	m_r_brightness_text->SetValue(str);
}

void AdjustView::OnRBrightnessText(wxCommandEvent& event)
{
	wxString str = m_r_brightness_text->GetValue();
	double val;
	str.ToDouble(&val);
	m_r_brightness_sldr->SetValue(int(val));

	if (m_sync_r)
	{
		if (m_sync_g)
		{
			m_g_brightness_sldr->SetValue(int(val));
			m_g_brightness_text->ChangeValue(str);
		}
		if (m_sync_b)
		{
			m_b_brightness_sldr->SetValue(int(val));
			m_b_brightness_text->ChangeValue(str);
		}
	}

	if (m_glview)
	{
		Color brightness = m_glview->GetBrightness();
		BrightnessUI2(val, brightness[0]);
		if (m_sync_r)
		{
			if (m_sync_g)
				BrightnessUI2(val, brightness[1]);
			if (m_sync_b)
				BrightnessUI2(val, brightness[2]);
		}
		m_glview->SetBrightness(brightness);
	}

	TreeLayer *layer = 0;
	if (m_type == 2 && m_vd)
		layer = (TreeLayer*)m_vd;
	else if (m_type == 5 && m_group)
		layer = (TreeLayer*)m_group;
	if (layer)
	{
		Color brightness = layer->GetBrightness();
		BrightnessUI2(val, brightness[0]);
		if (m_sync_r)
		{
			if (m_sync_g)
				BrightnessUI2(val, brightness[1]);
			if (m_sync_b)
				BrightnessUI2(val, brightness[2]);
		}
		layer->SetBrightness(brightness);

		if (m_link_group && m_group)
			m_group->SetBrightnessAll(brightness);

		RefreshVRenderViews(true);
	}
}

void AdjustView::OnGBrightnessChange(wxScrollEvent & event)
{
	double val = (double)event.GetPosition();
	wxString str = wxString::Format("%d", int(val));
	m_g_brightness_text->SetValue(str);
}

void AdjustView::OnGBrightnessText(wxCommandEvent& event)
{
	wxString str = m_g_brightness_text->GetValue();
	double val;
	str.ToDouble(&val);
	m_g_brightness_sldr->SetValue(int(val));

	if (m_sync_g)
	{
		if (m_sync_r)
		{
			m_r_brightness_sldr->SetValue(int(val));
			m_r_brightness_text->ChangeValue(str);
		}
		if (m_sync_b)
		{
			m_b_brightness_sldr->SetValue(int(val));
			m_b_brightness_text->ChangeValue(str);
		}
	}

	if (m_glview)
	{
		Color brightness = m_glview->GetBrightness();
		BrightnessUI2(val, brightness[1]);
		if (m_sync_g)
		{
			if (m_sync_r)
				BrightnessUI2(val, brightness[0]);
			if (m_sync_b)
				BrightnessUI2(val, brightness[2]);
		}
		m_glview->SetBrightness(brightness);
	}

	TreeLayer *layer = 0;
	if (m_type == 2 && m_vd)
		layer = (TreeLayer*)m_vd;
	else if (m_type == 5 && m_group)
		layer = (TreeLayer*)m_group;
	if (layer)
	{
		Color brightness = layer->GetBrightness();
		BrightnessUI2(val, brightness[1]);
		if (m_sync_g)
		{
			if (m_sync_r)
				BrightnessUI2(val, brightness[0]);
			if (m_sync_b)
				BrightnessUI2(val, brightness[2]);
		}
		layer->SetBrightness(brightness);

		if (m_link_group && m_group)
			m_group->SetBrightnessAll(brightness);

		RefreshVRenderViews(true);
	}
}

void AdjustView::OnBBrightnessChange(wxScrollEvent & event)
{
	double val = (double)event.GetPosition();
	wxString str = wxString::Format("%d", int(val));
	m_b_brightness_text->SetValue(str);
}

void AdjustView::OnBBrightnessText(wxCommandEvent& event)
{
	wxString str = m_b_brightness_text->GetValue();
	double val;
	str.ToDouble(&val);
	m_b_brightness_sldr->SetValue(int(val));

	if (m_sync_b)
	{
		if (m_sync_r)
		{
			m_r_brightness_sldr->SetValue(int(val));
			m_r_brightness_text->ChangeValue(str);
		}
		if (m_sync_g)
		{
			m_g_brightness_sldr->SetValue(int(val));
			m_g_brightness_text->ChangeValue(str);
		}
	}

	if (m_glview)
	{
		Color brightness = m_glview->GetBrightness();
		BrightnessUI2(val, brightness[2]);
		if (m_sync_b)
		{
			if (m_sync_r)
				BrightnessUI2(val, brightness[0]);
			if (m_sync_g)
				BrightnessUI2(val, brightness[1]);
		}
		m_glview->SetBrightness(brightness);
	}

	TreeLayer *layer = 0;
	if (m_type == 2 && m_vd)
		layer = (TreeLayer*)m_vd;
	else if (m_type == 5 && m_group)
		layer = (TreeLayer*)m_group;
	if (layer)
	{
		Color brightness = layer->GetBrightness();
		BrightnessUI2(val, brightness[2]);
		if (m_sync_b)
		{
			if (m_sync_r)
				BrightnessUI2(val, brightness[0]);
			if (m_sync_g)
				BrightnessUI2(val, brightness[1]);
		}
		layer->SetBrightness(brightness);

		if (m_link_group && m_group)
			m_group->SetBrightnessAll(brightness);

		RefreshVRenderViews(true);
	}
}

void AdjustView::OnRHdrChange(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 100.0;
	wxString str = wxString::Format("%.2f", val);
	m_r_hdr_text->SetValue(str);
}

void AdjustView::OnRHdrText(wxCommandEvent &event)
{
	wxString str = m_r_hdr_text->GetValue();
	double val;
	str.ToDouble(&val);
	m_r_hdr_sldr->SetValue(int(val*100));

	if (m_sync_r)
	{
		if (m_sync_g)
		{
			m_g_hdr_sldr->SetValue(int(val*100));
			m_g_hdr_text->ChangeValue(str);
		}
		if (m_sync_b)
		{
			m_b_hdr_sldr->SetValue(int(val*100));
			m_b_hdr_text->ChangeValue(str);
		}
	}

	if (m_glview)
	{
		Color hdr = m_glview->GetHdr();
		HdrUI2(val, hdr[0]);
		if (m_sync_r)
		{
			if (m_sync_g)
				HdrUI2(val, hdr[1]);
			if (m_sync_b)
				HdrUI2(val, hdr[2]);
		}
		m_glview->SetHdr(hdr);
	}

	TreeLayer *layer = 0;
	if (m_type == 2 && m_vd)
		layer = (TreeLayer*)m_vd;
	else if (m_type == 5 && m_group)
		layer = (TreeLayer*)m_group;
	if (layer)
	{
		Color hdr = layer->GetHdr();
		HdrUI2(val, hdr[0]);
		if (m_sync_r)
		{
			if (m_sync_g)
				HdrUI2(val, hdr[1]);
			if (m_sync_b)
				HdrUI2(val, hdr[2]);
		}
		layer->SetHdr(hdr);

		if (m_link_group && m_group)
			m_group->SetHdrAll(hdr);

		RefreshVRenderViews(true);
	}
}

void AdjustView::OnGHdrChange(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 100.0;
	wxString str = wxString::Format("%.2f", val);
	m_g_hdr_text->SetValue(str);
}

void AdjustView::OnGHdrText(wxCommandEvent &event)
{
	wxString str = m_g_hdr_text->GetValue();
	double val;
	str.ToDouble(&val);
	m_g_hdr_sldr->SetValue(int(val*100));

	if (m_sync_g)
	{
		if (m_sync_r)
		{
			m_r_hdr_sldr->SetValue(int(val*100));
			m_r_hdr_text->ChangeValue(str);
		}
		if (m_sync_b)
		{
			m_b_hdr_sldr->SetValue(int(val*100));
			m_b_hdr_text->ChangeValue(str);
		}
	}

	if (m_glview)
	{
		Color hdr = m_glview->GetHdr();
		HdrUI2(val, hdr[1]);
		if (m_sync_g)
		{
			if (m_sync_r)
				HdrUI2(val, hdr[0]);
			if (m_sync_b)
				HdrUI2(val, hdr[2]);
		}
		m_glview->SetHdr(hdr);
	}

	TreeLayer *layer = 0;
	if (m_type == 2 && m_vd)
		layer = (TreeLayer*)m_vd;
	else if (m_type == 5 && m_group)
		layer = (TreeLayer*)m_group;
	if (layer)
	{
		Color hdr = layer->GetHdr();
		HdrUI2(val, hdr[1]);
		if (m_sync_g)
		{
			if (m_sync_r)
				HdrUI2(val, hdr[0]);
			if (m_sync_b)
				HdrUI2(val, hdr[2]);
		}
		layer->SetHdr(hdr);

		if (m_link_group && m_group)
			m_group->SetHdrAll(hdr);

		RefreshVRenderViews(true);
	}
}

void AdjustView::OnBHdrChange(wxScrollEvent &event)
{
	double val = (double)event.GetPosition() / 100.0;
	wxString str = wxString::Format("%.2f", val);
	m_b_hdr_text->SetValue(str);
}

void AdjustView::OnBHdrText(wxCommandEvent &event)
{
	wxString str = m_b_hdr_text->GetValue();
	double val;
	str.ToDouble(&val);
	m_b_hdr_sldr->SetValue(int(val*100));

	if (m_sync_b)
	{
		if (m_sync_r)
		{
			m_r_hdr_sldr->SetValue(int(val*100));
			m_r_hdr_text->ChangeValue(str);
		}
		if (m_sync_g)
		{
			m_g_hdr_sldr->SetValue(int(val*100));
			m_g_hdr_text->ChangeValue(str);
		}
	}

	if (m_glview)
	{
		Color hdr = m_glview->GetHdr();
		HdrUI2(val, hdr[2]);
		if (m_sync_b)
		{
			if (m_sync_r)
				HdrUI2(val, hdr[0]);
			if (m_sync_g)
				HdrUI2(val, hdr[1]);
		}
		m_glview->SetHdr(hdr);
	}

	TreeLayer *layer = 0;
	if (m_type == 2 && m_vd)
		layer = (TreeLayer*)m_vd;
	else if (m_type == 5 && m_group)
		layer = (TreeLayer*)m_group;
	if (layer)
	{
		Color hdr = layer->GetHdr();
		HdrUI2(val, hdr[2]);
		if (m_sync_b)
		{
			if (m_sync_r)
				HdrUI2(val, hdr[0]);
			if (m_sync_g)
				HdrUI2(val, hdr[1]);
		}
		layer->SetHdr(hdr);

		if (m_link_group && m_group)
			m_group->SetHdrAll(hdr);

		RefreshVRenderViews(true);
	}
}

void AdjustView::OnSyncRCheck(wxCommandEvent &event)
{
	m_sync_r = m_sync_r_chk->GetToolState(ID_SyncRChk);
	m_sync_r_chk->SetToolNormalBitmap(ID_SyncRChk,
			m_sync_r?wxGetBitmapFromMemory(link):wxGetBitmapFromMemory(unlink));
	switch (m_type)
	{
	case 1://view
		if (m_glview)
			m_glview->SetSyncR(m_sync_r);
		break;
	case 2://volume data
		if (m_vd)
			m_vd->SetSyncR(m_sync_r);
		break;
	case 5://group
		if (m_group)
			m_group->SetSyncR(m_sync_r);
		break;
	}

	if ((m_type == 2 || m_type == 5) && 
		m_link_group && m_group)
		m_group->SetSyncRAll(m_sync_r);
}

void AdjustView::OnSyncGCheck(wxCommandEvent &event)
{
	m_sync_g = m_sync_g_chk->GetToolState(ID_SyncGChk);
	m_sync_g_chk->SetToolNormalBitmap(ID_SyncGChk,
			m_sync_g?wxGetBitmapFromMemory(link):wxGetBitmapFromMemory(unlink));
	switch (m_type)
	{
	case 1://view
		if (m_glview)
			m_glview->SetSyncG(m_sync_g);
		break;
	case 2://volume data
		if (m_vd)
			m_vd->SetSyncG(m_sync_g);
		break;
	case 5://group
		if (m_group)
			m_group->SetSyncG(m_sync_g);
		break;
	}

	if ((m_type == 2 || m_type == 5) && 
		m_link_group && m_group)
		m_group->SetSyncGAll(m_sync_g);
}

void AdjustView::OnSyncBCheck(wxCommandEvent &event)
{
	m_sync_b = m_sync_b_chk->GetToolState(ID_SyncBChk);
	m_sync_b_chk->SetToolNormalBitmap(ID_SyncBChk,
			m_sync_b?wxGetBitmapFromMemory(link):wxGetBitmapFromMemory(unlink));
	switch (m_type)
	{
	case 1://view
		if (m_glview)
			m_glview->SetSyncB(m_sync_b);
		break;
	case 2://volume data
		if (m_vd)
			m_vd->SetSyncB(m_sync_b);
		break;
	case 5://group
		if (m_group)
			m_group->SetSyncB(m_sync_b);
		break;
	}

	if ((m_type == 2 || m_type == 5) && 
		m_link_group && m_group)
		m_group->SetSyncBAll(m_sync_b);
}

void AdjustView::OnSaveDefault(wxCommandEvent &event)
{
	wxFileConfig fconfig("FluoRender default 2D adjustment settings");
	wxString str;
	double dft_r_gamma, dft_g_gamma, dft_b_gamma;
	double dft_r_brightness, dft_g_brightness, dft_b_brightness;
	double dft_r_hdr, dft_g_hdr, dft_b_hdr;

	//red
	fconfig.Write("sync_r_chk", m_sync_r_chk->GetToolState(ID_SyncRChk));
	str = m_r_gamma_text->GetValue();
	str.ToDouble(&dft_r_gamma);
	fconfig.Write("r_gamma_text", dft_r_gamma);
	str = m_r_brightness_text->GetValue();
	str.ToDouble(&dft_r_brightness);
	fconfig.Write("r_brightness_text", dft_r_brightness);
	str = m_r_hdr_text->GetValue();
	str.ToDouble(&dft_r_hdr);
	fconfig.Write("r_hdr_text", dft_r_hdr);

	//green
	fconfig.Write("sync_g_chk", m_sync_g_chk->GetToolState(ID_SyncGChk));
	str = m_g_gamma_text->GetValue();
	str.ToDouble(&dft_g_gamma);
	fconfig.Write("g_gamma_text", dft_g_gamma);
	str = m_g_brightness_text->GetValue();
	str.ToDouble(&dft_g_brightness);
	fconfig.Write("g_brightness_text", dft_g_brightness);
	str = m_g_hdr_text->GetValue();
	str.ToDouble(&dft_g_hdr);
	fconfig.Write("g_hdr_text", dft_g_hdr);

	//blue
	fconfig.Write("sync_b_chk", m_sync_b_chk->GetToolState(ID_SyncBChk));
	str = m_b_gamma_text->GetValue();
	str.ToDouble(&dft_b_gamma);
	fconfig.Write("b_gamma_text", dft_b_gamma);
	str = m_b_brightness_text->GetValue();
	str.ToDouble(&dft_b_brightness);
	fconfig.Write("b_brightness_text", dft_b_brightness);
	str = m_b_hdr_text->GetValue();
	str.ToDouble(&dft_b_hdr);
	fconfig.Write("b_hdr_text", dft_b_hdr);

	m_dft_gamma = Color(dft_r_gamma, dft_g_gamma, dft_b_gamma);
	m_dft_brightness = Color(dft_r_brightness, dft_g_brightness, dft_b_brightness);
	m_dft_hdr = Color(dft_r_hdr, dft_g_hdr, dft_b_hdr);
#ifdef _DARWIN
    wxString dft = wxString(getenv("HOME")) + "/Fluorender.settings/";
    mkdir(dft,0777);
    chmod(dft,0777);
    dft = dft + "default_2d_adjustment_settings.dft";
#else
    wxString dft = "default_2d_adjustment_settings.dft";
#endif
	wxFileOutputStream os(dft);
	fconfig.Save(os);
}

void AdjustView::LoadSettings()
{
#ifdef _DARWIN
    wxString dft = wxString(getenv("HOME")) + "/Fluorender.settings/default_2d_adjustment_settings.dft";
    std::ifstream tmp(dft);
    if (!tmp.good())
        dft = "FluoRender.app/Contents/Resources/default_2d_adjustment_settings.dft";
    else
        tmp.close();
#else
    wxString dft = "default_2d_adjustment_settings.dft";
#endif
    
	wxFileInputStream is(dft);
	if (!is.IsOk())
        return;
	wxFileConfig fconfig(is);

	wxString sVal;
	bool bVal;
	double dft_r_gamma = 1.0;
	double dft_g_gamma = 1.0;
	double dft_b_gamma = 1.0;
	double dft_r_brightness = 0.0;
	double dft_g_brightness = 0.0;
	double dft_b_brightness = 0.0;
	double dft_r_hdr = 0.0;
	double dft_g_hdr = 0.0;
	double dft_b_hdr = 0.0;

	//red
	if (fconfig.Read("sync_r_chk", &bVal))
		m_dft_sync_r = bVal;
	if (fconfig.Read("r_gamma_text", &sVal))
		sVal.ToDouble(&dft_r_gamma);
	if (fconfig.Read("r_brightness_text", &sVal))
		sVal.ToDouble(&dft_r_brightness);
	if (fconfig.Read("r_hdr_text", &sVal))
		sVal.ToDouble(&dft_r_hdr);

	//green
	if (fconfig.Read("sync_g_chk", &bVal))
		m_dft_sync_g = bVal;
	if (fconfig.Read("g_gamma_text", &sVal))
		sVal.ToDouble(&dft_g_gamma);
	if (fconfig.Read("g_brightness_text", &sVal))
		sVal.ToDouble(&dft_g_brightness);
	if (fconfig.Read("g_hdr_text", &sVal))
		sVal.ToDouble(&dft_g_hdr);

	//blue
	if (fconfig.Read("sync_b_chk", &bVal))
		m_dft_sync_b = bVal;
	if (fconfig.Read("b_gamma_text", &sVal))
		sVal.ToDouble(&dft_b_gamma);
	if (fconfig.Read("b_brightness_text", &sVal))
		sVal.ToDouble(&dft_b_brightness);
	if (fconfig.Read("b_hdr_text", &sVal))
		sVal.ToDouble(&dft_b_hdr);

	m_dft_gamma = Color(dft_r_gamma, dft_g_gamma, dft_b_gamma);
	m_dft_brightness = Color(dft_r_brightness, dft_g_brightness, dft_b_brightness);
	m_dft_hdr = Color(dft_r_hdr, dft_g_hdr, dft_b_hdr);

	m_use_dft_settings = true;

}

void AdjustView::GetDefaults(Color &gamma, Color &brightness, Color &hdr,
							 bool &sync_r, bool &sync_g, bool &sync_b)
{
	GammaUI2(m_dft_gamma.r(), gamma[0]);
	GammaUI2(m_dft_gamma.g(), gamma[1]);
	GammaUI2(m_dft_gamma.b(), gamma[2]);
	BrightnessUI2(m_dft_brightness.r(), brightness[0]);
	BrightnessUI2(m_dft_brightness.g(), brightness[1]);
	BrightnessUI2(m_dft_brightness.b(), brightness[2]);
	hdr = m_dft_hdr;
	sync_r = m_dft_sync_r;
	sync_g = m_dft_sync_g;
	sync_b = m_dft_sync_b;
}

//change settings externally
void AdjustView::ChangeRGamma(double gamma_r)
{
	Gamma2UI(gamma_r, gamma_r);
	m_r_gamma_text->SetValue(wxString::Format("%.2f", gamma_r));
}

void AdjustView::ChangeGGamma(double gamma_g)
{
	Gamma2UI(gamma_g, gamma_g);
	m_g_gamma_text->SetValue(wxString::Format("%.2f", gamma_g));
}

void AdjustView::ChangeBGamma(double gamma_b)
{
	Gamma2UI(gamma_b, gamma_b);
	m_b_gamma_text->SetValue(wxString::Format("%.2f", gamma_b));
}

void AdjustView::ChangeRBrightness(double brightness_r)
{
	Brightness2UI(brightness_r, brightness_r);
	m_r_brightness_text->SetValue(wxString::Format("%.0f", brightness_r));
}

void AdjustView::ChangeGBrightness(double brightness_g)
{
	Brightness2UI(brightness_g, brightness_g);
	m_g_brightness_text->SetValue(wxString::Format("%.0f", brightness_g));
}

void AdjustView::ChangeBBrightness(double brightness_b)
{
	Brightness2UI(brightness_b, brightness_b);
	m_b_brightness_text->SetValue(wxString::Format("%.0f", brightness_b));
}

void AdjustView::ChangeRHdr(double hdr_r)
{
	Hdr2UI(hdr_r, hdr_r);
	m_r_hdr_text->SetValue(wxString::Format("%.2f", hdr_r));
}

void AdjustView::ChangeGHdr(double hdr_g)
{
	Hdr2UI(hdr_g, hdr_g);
	m_g_hdr_text->SetValue(wxString::Format("%.2f", hdr_g));
}

void AdjustView::ChangeBHdr(double hdr_b)
{
	Hdr2UI(hdr_b, hdr_b);
	m_b_hdr_text->SetValue(wxString::Format("%.2f", hdr_b));
}

void AdjustView::ChangeRSync(bool sync_r)
{
	m_sync_r_chk->ToggleTool(ID_SyncRChk,sync_r);
	wxCommandEvent event;
	OnSyncRCheck(event);
}

void AdjustView::ChangeGSync(bool sync_g)
{
	m_sync_g_chk->ToggleTool(ID_SyncGChk,sync_g);
	wxCommandEvent event;
	OnSyncGCheck(event);
}

void AdjustView::ChangeBSync(bool sync_b)
{
	m_sync_b_chk->ToggleTool(ID_SyncBChk,sync_b);
	wxCommandEvent event;
	OnSyncBCheck(event);
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
					Color c = vd->GetColor();
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
		ChangeRSync(r_v);
		ChangeGSync(g_v);
		ChangeBSync(b_v);

		cnt = 0;
		if (r_v) cnt++;
		if (g_v) cnt++;
		if (b_v) cnt++;

		if (cnt > 1)
		{
			double gamma = 1.0, brightness = 1.0, hdr = 0.0;
			if (r_v)
			{
				gamma = m_group->GetGamma().r();
				brightness = m_group->GetBrightness().r();
				hdr = m_group->GetHdr().r();
			}
			else if (g_v)
			{
				gamma = m_group->GetGamma().g();
				brightness = m_group->GetBrightness().g();
				hdr = m_group->GetHdr().g();
			}

			if (g_v)
			{
				ChangeGGamma(gamma);
				ChangeGBrightness(brightness);
				ChangeGHdr(hdr);
			}
			if (b_v)
			{
				ChangeBGamma(gamma);
				ChangeBBrightness(brightness);
				ChangeBHdr(hdr);
			}
		}
	}
	else if (m_type == 2 && !m_link_group && m_vd)
	{
		//use volume
	}
	else if (m_type == 1 && m_glview)
	{
		//means this is depth mode
		if (m_glview->GetVolMethod() != VOL_METHOD_MULTI)
			return;
		
		for (i=0; i<m_glview->GetDispVolumeNum(); i++)
		{
			VolumeData* vd = m_glview->GetDispVolumeData(i);
			if (vd)
			{
				if (vd->GetColormapMode())
				{
					r_v = g_v = b_v = true;
				}
				else
				{
					Color c = vd->GetColor();
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

		ChangeRSync(r_v);
		ChangeGSync(g_v);
		ChangeBSync(b_v);

		cnt = 0;

		if (r_v) cnt++;
		if (g_v) cnt++;
		if (b_v) cnt++;

		if (cnt > 1)
		{
			double gamma = 1.0, brightness = 1.0, hdr = 0.0;
			if (r_v)
			{
				gamma = m_glview->GetGamma().r();
				brightness = m_glview->GetBrightness().r();
				hdr = m_glview->GetHdr().r();
			}
			else if (g_v)
			{
				gamma = m_glview->GetGamma().g();
				brightness = m_glview->GetBrightness().g();
				hdr = m_glview->GetHdr().g();
			}

			if (g_v)
			{
				ChangeGGamma(gamma);
				ChangeGBrightness(brightness);
				ChangeGHdr(hdr);
			}
			if (b_v)
			{
				ChangeBGamma(gamma);
				ChangeBBrightness(brightness);
				ChangeBHdr(hdr);
			}
		}
	}

}

void AdjustView::OnRReset(wxCommandEvent &event)
{
	//reset gamma
	double dft_value = 1.0;
	if (m_use_dft_settings)
		dft_value = m_dft_gamma.r();

	m_r_gamma_sldr->SetValue(int(dft_value*100.0));
	wxString str = wxString::Format("%.2f", dft_value);
	m_r_gamma_text->ChangeValue(str);
	if (m_sync_r)
	{
		if (m_sync_g)
		{
			m_g_gamma_sldr->SetValue(int(dft_value*100.0));
			m_g_gamma_text->ChangeValue(str);
		}
		if (m_sync_b)
		{
			m_b_gamma_sldr->SetValue(int(dft_value*100.0));
			m_b_gamma_text->ChangeValue(str);
		}
	}

	if (m_glview)
	{
		Color gamma = m_glview->GetGamma();
		GammaUI2(dft_value, gamma[0]);
		if (m_sync_r)
		{
			if (m_sync_g)
				GammaUI2(dft_value, gamma[1]);
			if (m_sync_b)
				GammaUI2(dft_value, gamma[2]);
		}
		m_glview->SetGamma(gamma);
	}

	TreeLayer *layer = 0;
	if (m_type == 2 && m_vd)
		layer = (TreeLayer*)m_vd;
	else if (m_type == 5 && m_group)
		layer = (TreeLayer*)m_group;
	if (layer)
	{
		Color gamma = layer->GetGamma();
		GammaUI2(dft_value, gamma[0]);
		if (m_sync_r)
		{
			if (m_sync_g)
				GammaUI2(dft_value, gamma[1]);
			if (m_sync_b)
				GammaUI2(dft_value, gamma[2]);
		}
		layer->SetGamma(gamma);

		if (m_link_group && m_group)
			m_group->SetGammaAll(gamma);
	}

	//reset brightness
	dft_value = 0;
	if (m_use_dft_settings)
		dft_value = m_dft_brightness.r();

	m_r_brightness_sldr->SetValue(int(dft_value));
	str = wxString::Format("%d", int(dft_value));
	m_r_brightness_text->ChangeValue(str);
	if (m_sync_r)
	{
		if (m_sync_g)
		{
			m_g_brightness_sldr->SetValue(int(dft_value));
			m_g_brightness_text->ChangeValue(str);
		}
		if (m_sync_b)
		{
			m_b_brightness_sldr->SetValue(int(dft_value));
			m_b_brightness_text->ChangeValue(str);
		}
	}

	if (m_glview)
	{
		Color brightness = m_glview->GetBrightness();
		BrightnessUI2(dft_value, brightness[0]);
		if (m_sync_r)
		{
			if (m_sync_g)
				BrightnessUI2(dft_value, brightness[1]);
			if (m_sync_b)
				BrightnessUI2(dft_value, brightness[2]);
		}
		m_glview->SetBrightness(brightness);
	}

	if (layer)
	{
		Color brightness = layer->GetBrightness();
		BrightnessUI2(dft_value, brightness[0]);
		if (m_sync_r)
		{
			if (m_sync_g)
				BrightnessUI2(dft_value, brightness[1]);
			if (m_sync_b)
				BrightnessUI2(dft_value, brightness[2]);
		}
		layer->SetBrightness(brightness);

		if (m_link_group && m_group)
			m_group->SetBrightnessAll(brightness);
	}

	//reset hdr
	dft_value = 0;
	if (m_use_dft_settings)
		dft_value = m_dft_hdr.r();

	m_r_hdr_sldr->SetValue(int(dft_value*100.0));
	str = wxString::Format("%.2f", dft_value);
	m_r_hdr_text->ChangeValue(str);
	if (m_sync_r)
	{
		if (m_sync_g)
		{
			m_g_hdr_sldr->SetValue(int(dft_value*100.0));
			m_g_hdr_text->ChangeValue(str);
		}
		if (m_sync_b)
		{
			m_b_hdr_sldr->SetValue(int(dft_value*100.0));
			m_b_hdr_text->ChangeValue(str);
		}
	}

	if (m_glview)
	{
		Color hdr = m_glview->GetHdr();
		hdr[0] = dft_value;
		if (m_sync_r)
		{
			if (m_sync_g)
				hdr[1] = dft_value;
			if (m_sync_b)
				hdr[2] = dft_value;
		}
		m_glview->SetHdr(hdr);
	}

	if (layer)
	{
		Color hdr = layer->GetHdr();
		hdr[0] = dft_value;
		if (m_sync_r)
		{
			if (m_sync_g)
				hdr[1] = dft_value;
			if (m_sync_b)
				hdr[2] = dft_value;
		}
		layer->SetHdr(hdr);

		if (m_link_group && m_group)
			m_group->SetHdrAll(hdr);
	}

	RefreshVRenderViews();
}

void AdjustView::OnGReset(wxCommandEvent &event)
{
	//reset gamma
	double dft_value = 1.0;
	if (m_use_dft_settings)
		dft_value = m_dft_gamma.g();

	m_g_gamma_sldr->SetValue(int(dft_value*100.0));
	wxString str = wxString::Format("%.2f", dft_value);
	m_g_gamma_text->ChangeValue(str);
	if (m_sync_g)
	{
		if (m_sync_r)
		{
			m_r_gamma_sldr->SetValue(int(dft_value*100.0));
			m_r_gamma_text->ChangeValue(str);
		}
		if (m_sync_b)
		{
			m_b_gamma_sldr->SetValue(int(dft_value*100.0));
			m_b_gamma_text->ChangeValue(str);
		}
	}

	if (m_glview)
	{
		Color gamma = m_glview->GetGamma();
		GammaUI2(dft_value, gamma[1]);
		if (m_sync_g)
		{
			if (m_sync_r)
				GammaUI2(dft_value, gamma[0]);
			if (m_sync_b)
				GammaUI2(dft_value, gamma[2]);
		}
		m_glview->SetGamma(gamma);
	}

	TreeLayer *layer = 0;
	if (m_type == 2 && m_vd)
		layer = (TreeLayer*)m_vd;
	else if (m_type == 5 && m_group)
		layer = (TreeLayer*)m_group;
	if (layer)
	{
		Color gamma = layer->GetGamma();
		GammaUI2(dft_value, gamma[1]);
		if (m_sync_g)
		{
			if (m_sync_r)
				GammaUI2(dft_value, gamma[0]);
			if (m_sync_b)
				GammaUI2(dft_value, gamma[2]);
		}
		layer->SetGamma(gamma);

		if (m_link_group && m_group)
			m_group->SetGammaAll(gamma);
	}

	//reset brightness
	dft_value = 0.0;
	if (m_use_dft_settings)
		dft_value = m_dft_brightness.g();

	m_g_brightness_sldr->SetValue(int(dft_value));
	str = wxString::Format("%d", int(dft_value));
	m_g_brightness_text->ChangeValue(str);
	if (m_sync_g)
	{
		if (m_sync_r)
		{
			m_r_brightness_sldr->SetValue(int(dft_value));
			m_r_brightness_text->ChangeValue(str);
		}
		if (m_sync_b)
		{
			m_b_brightness_sldr->SetValue(int(dft_value));
			m_b_brightness_text->ChangeValue(str);
		}
	}

	if (m_glview)
	{
		Color brightness = m_glview->GetBrightness();
		BrightnessUI2(dft_value, brightness[1]);
		if (m_sync_g)
		{
			if (m_sync_r)
				BrightnessUI2(dft_value, brightness[0]);
			if (m_sync_b)
				BrightnessUI2(dft_value, brightness[2]);
		}
		m_glview->SetBrightness(brightness);
	}

	if (layer)
	{
		Color brightness = layer->GetBrightness();
		BrightnessUI2(dft_value, brightness[1]);
		if (m_sync_g)
		{
			if (m_sync_r)
				BrightnessUI2(dft_value, brightness[0]);
			if (m_sync_b)
				BrightnessUI2(dft_value, brightness[2]);
		}
		layer->SetBrightness(brightness);

		if (m_link_group && m_group)
			m_group->SetBrightnessAll(brightness);
	}

	//reset hdr
	dft_value = 0;
	if (m_use_dft_settings)
		dft_value = m_dft_hdr.g();

	m_g_hdr_sldr->SetValue(int(dft_value*100.0));
	str = wxString::Format("%.2f", dft_value);
	m_g_hdr_text->ChangeValue(str);
	if (m_sync_g)
	{
		if (m_sync_r)
		{
			m_r_hdr_sldr->SetValue(int(dft_value*100.0));
			m_r_hdr_text->ChangeValue(str);
		}
		if (m_sync_b)
		{
			m_b_hdr_sldr->SetValue(int(dft_value*100.0));
			m_b_hdr_text->ChangeValue(str);
		}
	}

	if (m_glview)
	{
		Color hdr = m_glview->GetHdr();
		hdr[1] = dft_value;
		if (m_sync_g)
		{
			if (m_sync_r)
				hdr[0] = dft_value;
			if (m_sync_b)
				hdr[2] = dft_value;
		}
		m_glview->SetHdr(hdr);
	}

	if (layer)
	{
		Color hdr = layer->GetHdr();
		hdr[1] = dft_value;
		if (m_sync_g)
		{
			if (m_sync_r)
				hdr[0] = dft_value;
			if (m_sync_b)
				hdr[2] = dft_value;
		}
		layer->SetHdr(hdr);

		if (m_link_group && m_group)
			m_group->SetHdrAll(hdr);
	}

	RefreshVRenderViews();
}

void AdjustView::OnBReset(wxCommandEvent &event)
{
	//reset gamma
	double dft_value = 1.0;
	if (m_use_dft_settings)
		dft_value = m_dft_gamma.b();

	m_b_gamma_sldr->SetValue(int(dft_value*100.0));
	wxString str = wxString::Format("%.2f", dft_value);
	m_b_gamma_text->ChangeValue(str);
	if (m_sync_b)
	{
		if (m_sync_r)
		{
			m_r_gamma_sldr->SetValue(int(dft_value*100.0));
			m_r_gamma_text->ChangeValue(str);
		}
		if (m_sync_g)
		{
			m_g_gamma_sldr->SetValue(int(dft_value*100.0));
			m_g_gamma_text->ChangeValue(str);
		}
	}

	if (m_glview)
	{
		Color gamma = m_glview->GetGamma();
		GammaUI2(dft_value, gamma[2]);
		if (m_sync_b)
		{
			if (m_sync_r)
				GammaUI2(dft_value, gamma[0]);
			if (m_sync_g)
				GammaUI2(dft_value, gamma[1]);
		}
		m_glview->SetGamma(gamma);
	}

	TreeLayer *layer = 0;
	if (m_type == 2 && m_vd)
		layer = (TreeLayer*)m_vd;
	else if (m_type == 5 && m_group)
		layer = (TreeLayer*)m_group;
	if (layer)
	{
		Color gamma = layer->GetGamma();
		GammaUI2(dft_value, gamma[2]);
		if (m_sync_b)
		{
			if (m_sync_r)
				GammaUI2(dft_value, gamma[0]);
			if (m_sync_g)
				GammaUI2(dft_value, gamma[1]);
		}
		layer->SetGamma(gamma);

		if (m_link_group && m_group)
			m_group->SetGammaAll(gamma);
	}

	//reset brightness
	dft_value = 1.0;
	if (m_use_dft_settings)
		dft_value = m_dft_brightness.b();

	m_b_brightness_sldr->SetValue(int(dft_value));
	str = wxString::Format("%d", int(dft_value));
	m_b_brightness_text->ChangeValue(str);
	if (m_sync_b)
	{
		if (m_sync_r)
		{
			m_r_brightness_sldr->SetValue(int(dft_value));
			m_r_brightness_text->ChangeValue(str);
		}
		if (m_sync_g)
		{
			m_g_brightness_sldr->SetValue(int(dft_value));
			m_g_brightness_text->ChangeValue(str);
		}
	}

	if (m_glview)
	{
		Color brightness = m_glview->GetBrightness();
		BrightnessUI2(dft_value, brightness[2]);
		if (m_sync_b)
		{
			if (m_sync_r)
				BrightnessUI2(dft_value, brightness[0]);
			if (m_sync_g)
				BrightnessUI2(dft_value, brightness[1]);
		}
		m_glview->SetBrightness(brightness);
	}

	if (layer)
	{
		Color brightness = layer->GetBrightness();
		BrightnessUI2(dft_value, brightness[2]);
		if (m_sync_b)
		{
			if (m_sync_r)
				BrightnessUI2(dft_value, brightness[0]);
			if (m_sync_g)
				BrightnessUI2(dft_value, brightness[1]);
		}
		layer->SetBrightness(brightness);

		if (m_link_group && m_group)
			m_group->SetBrightnessAll(brightness);
	}

	//reset hdr
	dft_value = 0;
	if (m_use_dft_settings)
		dft_value = m_dft_hdr.b();

	m_b_hdr_sldr->SetValue(int(dft_value*100.0));
	str = wxString::Format("%.2f", dft_value);
	m_b_hdr_text->ChangeValue(str);
	if (m_sync_b)
	{
		if (m_sync_r)
		{
			m_r_hdr_sldr->SetValue(int(dft_value*100.0));
			m_r_hdr_text->ChangeValue(str);
		}
		if (m_sync_g)
		{
			m_g_hdr_sldr->SetValue(int(dft_value*100.0));
			m_g_hdr_text->ChangeValue(str);
		}
	}

	if (m_glview)
	{
		Color hdr = m_glview->GetHdr();
		hdr[2] = dft_value;
		if (m_sync_b)
		{
			if (m_sync_r)
				hdr[0] = dft_value;
			if (m_sync_g)
				hdr[1] = dft_value;
		}
		m_glview->SetHdr(hdr);
	}

	if (layer)
	{
		Color hdr = layer->GetHdr();
		hdr[2] = dft_value;
		if (m_sync_b)
		{
			if (m_sync_r)
				hdr[0] = dft_value;
			if (m_sync_g)
				hdr[1] = dft_value;
		}
		layer->SetHdr(hdr);

		if (m_link_group && m_group)
			m_group->SetHdrAll(hdr);
	}

	RefreshVRenderViews();
}

