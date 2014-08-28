#include "ClippingView.h"
#include "VRenderFrame.h"
#include "compatibility.h"
#include <wx/valnum.h>
#include "png_resource.h"
#include "img/link.h"
#include "img/unlink.h"
#include "img/refresh.h"
#include "img/align.h"
#include "img/link_text.h"
#include "img/unlink_text.h"

BEGIN_EVENT_TABLE(ClippingView, wxPanel)
	EVT_TOOL(ID_LinkChannelsChk, ClippingView::OnLinkChannelsCheck)
	EVT_BUTTON(ID_SetZeroBtn, ClippingView::OnSetZeroBtn)
	EVT_BUTTON(ID_RotResetBtn, ClippingView::OnRotResetBtn)
	EVT_BUTTON(ID_ClipResetBtn, ClippingView::OnClipResetBtn)

	EVT_COMMAND_SCROLL(ID_X1ClipSldr, ClippingView::OnX1ClipChange)
	EVT_COMMAND_SCROLL(ID_X2ClipSldr, ClippingView::OnX2ClipChange)
	EVT_COMMAND_SCROLL(ID_Y1ClipSldr, ClippingView::OnY1ClipChange)
	EVT_COMMAND_SCROLL(ID_Y2ClipSldr, ClippingView::OnY2ClipChange)
	EVT_COMMAND_SCROLL(ID_Z1ClipSldr, ClippingView::OnZ1ClipChange)
	EVT_COMMAND_SCROLL(ID_Z2ClipSldr, ClippingView::OnZ2ClipChange)

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

ClippingView::ClippingView(wxWindow* frame,
						   wxWindow* parent,
						   wxWindowID id,
						   const wxPoint& pos,
						   const wxSize& size,
						   long style,
						   const wxString& name):
wxPanel(parent, id, pos, size, style, name),
m_frame(frame),
m_sel_type(0),
m_vd(0),
m_md(0),
m_draw_clip(false),
m_link_x(false),
m_link_y(false),
m_link_z(false)
{
	//validator: floating point 1
	wxFloatingPointValidator<double> vald_fp1(1);
	vald_fp1.SetRange(-180.0, 180.0);
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;

	wxStaticText *st = 0;

	//sync channels 1
	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	m_link_channels = new wxToolBar(this,wxID_ANY);
	m_link_channels->AddCheckTool(ID_LinkChannelsChk, "Sync All Chan.",
		wxGetBitmapFromMemory(unlink_text), wxNullBitmap,
		"Link all data channels to this cropping",
		"Link all data channels to this cropping");
	m_link_channels->ToggleTool(ID_LinkChannelsChk,false);
	m_link_channels->Realize();
	sizer_1->Add(5, 5, 0);
	sizer_1->Add(m_link_channels, 0, wxALIGN_CENTER, 0);

	wxStaticText* st_cb = 0;

	//sliders for clipping planes
	//x
	wxBoxSizer* sizer_cx = new wxBoxSizer(wxVERTICAL);
	m_xpanel = new wxPanel(this);
	st = new wxStaticText(this, 0, "X");
	m_x1_clip_sldr = new wxSlider(m_xpanel, ID_X1ClipSldr, 0, 0, 512,
		wxPoint(0,0), wxSize(20,-1), wxSL_VERTICAL);
	m_xBar = new wxStaticText(m_xpanel, 0, "",
		wxPoint(20,10), wxSize(3, m_x1_clip_sldr->GetSize().GetHeight() - 20));
	m_xBar->SetBackgroundColour(wxColor(255, 128, 128));
	m_x2_clip_sldr = new wxSlider(m_xpanel, ID_X2ClipSldr, 512, 0, 512,
		wxPoint(23,0), wxSize(20,-1), wxSL_VERTICAL);
	m_x1_clip_text = new wxTextCtrl(this, ID_X1ClipText, "0",
		wxDefaultPosition, wxSize(34, 20), 0, vald_int);
	st_cb = new wxStaticText(this, 0, "",
		wxDefaultPosition, wxSize(5, 5));
	st_cb->SetBackgroundColour(wxColor(255, 128, 128));
	m_x2_clip_text = new wxTextCtrl(this, ID_X2ClipText, "512",
		wxDefaultPosition, wxSize(34, 20), 0, vald_int);
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

	//y
	wxBoxSizer* sizer_cy = new wxBoxSizer(wxVERTICAL);
	wxPanel * ypanel = new wxPanel(this);
	st = new wxStaticText(this, 0, "Y");
	m_y1_clip_sldr = new wxSlider(ypanel, ID_Y1ClipSldr, 0, 0, 512,
		wxPoint(0,0),  wxSize(20,-1), wxSL_VERTICAL);
	m_yBar = new wxStaticText(ypanel, 0, "",
		wxPoint(20,10), wxSize(3, m_x1_clip_sldr->GetSize().GetHeight() - 20));
	m_yBar->SetBackgroundColour(wxColor(128, 255, 128));
	m_y2_clip_sldr = new wxSlider(ypanel, ID_Y2ClipSldr, 512, 0, 512,
		wxPoint(23,0),  wxSize(20,-1), wxSL_VERTICAL);
	m_y1_clip_text = new wxTextCtrl(this, ID_Y1ClipText, "0",
		wxDefaultPosition, wxSize(34, 20), 0, vald_int);
	st_cb = new wxStaticText(this, 0, "",
		wxDefaultPosition, wxSize(5, 5));
	st_cb->SetBackgroundColour(wxColor(128, 255, 128));
	m_y2_clip_text = new wxTextCtrl(this, ID_Y2ClipText, "512",
		wxDefaultPosition, wxSize(34, 20), 0, vald_int);
	//add the items
	sizer_cy->Add(5, 5, 0);
	sizer_cy->Add(st, 0, wxALIGN_CENTER, 0);
	sizer_cy->Add(m_y1_clip_text, 0, wxALIGN_CENTER, 0);
	sizer_cy->Add(5, 5, 0);
	sizer_cy->Add(st_cb, 0, wxEXPAND);
	sizer_cy->Add(ypanel, 1, wxALIGN_CENTER, 0);
	st_cb = new wxStaticText(this, 0, "",
		wxDefaultPosition, wxSize(5, 5));
	st_cb->SetBackgroundColour(wxColor(255, 255, 128));
	sizer_cy->Add(st_cb, 0, wxEXPAND);
	sizer_cy->Add(5, 5, 0);
	sizer_cy->Add(m_y2_clip_text, 0, wxALIGN_CENTER, 0);

	//z
	wxBoxSizer* sizer_cz = new wxBoxSizer(wxVERTICAL);
	wxPanel * zpanel = new wxPanel(this);
	st = new wxStaticText(this, 0, "Z");
	m_z1_clip_sldr = new wxSlider(zpanel, ID_Z1ClipSldr, 0, 0, 512,
		wxPoint(0,0),  wxSize(20,-1), wxSL_VERTICAL);
	m_zBar = new wxStaticText(zpanel, 0, "",
		wxPoint(20,10), wxSize(3, m_x1_clip_sldr->GetSize().GetHeight() - 20));
	m_zBar->SetBackgroundColour(wxColor(128, 128, 255));
	m_z2_clip_sldr = new wxSlider(zpanel, ID_Z2ClipSldr, 512, 0, 512,
		wxPoint(23,0),  wxSize(20,-1), wxSL_VERTICAL);
	m_z1_clip_text = new wxTextCtrl(this, ID_Z1ClipText, "0",
		wxDefaultPosition, wxSize(34, 20), 0, vald_int);
	st_cb = new wxStaticText(this, 0, "",
		wxDefaultPosition, wxSize(5, 5));
	st_cb->SetBackgroundColour(wxColor(128, 128, 255));
	m_z2_clip_text = new wxTextCtrl(this, ID_Z2ClipText, "512",
		wxDefaultPosition, wxSize(34, 20), 0, vald_int);
	//add the items
	sizer_cz->Add(5, 5, 0);
	sizer_cz->Add(st, 0, wxALIGN_CENTER, 0);
	sizer_cz->Add(m_z1_clip_text, 0, wxALIGN_CENTER, 0);
	sizer_cz->Add(5, 5, 0);
	sizer_cz->Add(st_cb, 0, wxEXPAND);
	sizer_cz->Add(zpanel, 1, wxALIGN_CENTER, 0);
	st_cb = new wxStaticText(this, 0, "",
		wxDefaultPosition, wxSize(5, 5));
	st_cb->SetBackgroundColour(wxColor(128, 255, 255));
	sizer_cz->Add(st_cb, 0, wxEXPAND);
	sizer_cz->Add(5, 5, 0);
	sizer_cz->Add(m_z2_clip_text, 0, wxALIGN_CENTER, 0);

	//link sliders
	m_x1_clip_sldr->Connect(ID_X1ClipSldr, wxEVT_RIGHT_DOWN,
		wxCommandEventHandler(ClippingView::OnSliderRClick),
		NULL, this);
	m_x2_clip_sldr->Connect(ID_X2ClipSldr, wxEVT_RIGHT_DOWN,
		wxCommandEventHandler(ClippingView::OnSliderRClick),
		NULL, this);
	m_y1_clip_sldr->Connect(ID_Y1ClipSldr, wxEVT_RIGHT_DOWN,
		wxCommandEventHandler(ClippingView::OnSliderRClick),
		NULL, this);
	m_y2_clip_sldr->Connect(ID_Y2ClipSldr, wxEVT_RIGHT_DOWN,
		wxCommandEventHandler(ClippingView::OnSliderRClick),
		NULL, this);
	m_z1_clip_sldr->Connect(ID_Z1ClipSldr, wxEVT_RIGHT_DOWN,
		wxCommandEventHandler(ClippingView::OnSliderRClick),
		NULL, this);
	m_z2_clip_sldr->Connect(ID_Z2ClipSldr, wxEVT_RIGHT_DOWN,
		wxCommandEventHandler(ClippingView::OnSliderRClick),
		NULL, this);

	//keys
	m_x1_clip_sldr->Connect(ID_X1ClipSldr, wxEVT_KEY_DOWN,
		wxKeyEventHandler(ClippingView::OnSliderKeyDown),
		NULL, this);
	m_x2_clip_sldr->Connect(ID_X2ClipSldr, wxEVT_KEY_DOWN,
		wxKeyEventHandler(ClippingView::OnSliderKeyDown),
		NULL, this);
	m_y1_clip_sldr->Connect(ID_Y1ClipSldr, wxEVT_KEY_DOWN,
		wxKeyEventHandler(ClippingView::OnSliderKeyDown),
		NULL, this);
	m_y2_clip_sldr->Connect(ID_Y2ClipSldr, wxEVT_KEY_DOWN,
		wxKeyEventHandler(ClippingView::OnSliderKeyDown),
		NULL, this);
	m_z1_clip_sldr->Connect(ID_Z1ClipSldr, wxEVT_KEY_DOWN,
		wxKeyEventHandler(ClippingView::OnSliderKeyDown),
		NULL, this);
	m_z2_clip_sldr->Connect(ID_Z2ClipSldr, wxEVT_KEY_DOWN,
		wxKeyEventHandler(ClippingView::OnSliderKeyDown),
		NULL, this);

	//2
	wxBoxSizer *sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	sizer_2->Add(sizer_cx, 1, wxEXPAND);
	sizer_2->Add(sizer_cy, 1, wxEXPAND);
	sizer_2->Add(sizer_cz, 1, wxEXPAND);

	//3 -- link checkbox images
	wxBoxSizer *sizer_3 = new wxBoxSizer(wxHORIZONTAL);
	m_check_tb = new wxToolBar(this,wxID_ANY);
	m_check_tb->AddCheckTool(ID_LinkXChk,"Lock X Planes",
		wxGetBitmapFromMemory(unlink),
		wxNullBitmap,"Lock X Planes");
	m_check_tb->AddSeparator();
	m_check_tb->AddSeparator();
	m_check_tb->AddCheckTool(ID_LinkYChk,"Lock Y Planes",
		wxGetBitmapFromMemory(unlink),
		wxNullBitmap,"Lock Y Planes");
	m_check_tb->AddSeparator();
	m_check_tb->AddSeparator();
	m_check_tb->AddCheckTool(ID_LinkZChk,"Lock Z Planes",
		wxGetBitmapFromMemory(unlink),
		wxNullBitmap,"Lock Z Planes");
	m_check_tb->Realize();
	sizer_3->Add(5,5,0);
	sizer_3->Add(m_check_tb, 1, wxALIGN_TOP|wxSHRINK );

	//clip buttons 4
	wxBoxSizer* sizer_4 = new wxBoxSizer(wxHORIZONTAL);
	m_yz_clip_btn = new wxButton(this, ID_YZClipBtn, "YZ",
		wxDefaultPosition, wxSize(34, 22));
	m_xz_clip_btn = new wxButton(this, ID_XZClipBtn, "XZ",
		wxDefaultPosition, wxSize(34, 22));
	m_xy_clip_btn = new wxButton(this, ID_XYClipBtn, "XY",
		wxDefaultPosition, wxSize(34, 22));
	sizer_4->Add(m_yz_clip_btn, 1, wxEXPAND|wxALIGN_CENTER);
	sizer_4->AddSpacer(5);
	sizer_4->Add(m_xz_clip_btn, 1, wxEXPAND|wxALIGN_CENTER);
	sizer_4->AddSpacer(5);
	sizer_4->Add(m_xy_clip_btn, 1, wxEXPAND|wxALIGN_CENTER);

	//clip distance 5
	wxBoxSizer* sizer_5 = new wxBoxSizer(wxHORIZONTAL);
	m_yz_dist_text = new wxTextCtrl(this, ID_YZDistText, "1",
		wxDefaultPosition, wxSize(34, 22), 0, vald_int);
	m_xz_dist_text = new wxTextCtrl(this, ID_XZDistText, "1",
		wxDefaultPosition, wxSize(34, 22), 0, vald_int);
	m_xy_dist_text = new wxTextCtrl(this, ID_XYDistText, "1",
		wxDefaultPosition, wxSize(34, 22), 0, vald_int);
	sizer_5->Add(m_yz_dist_text, 1, wxEXPAND|wxALIGN_CENTER);
	sizer_5->AddSpacer(5);
	sizer_5->Add(m_xz_dist_text, 1, wxEXPAND|wxALIGN_CENTER);
	sizer_5->AddSpacer(5);
	sizer_5->Add(m_xy_dist_text, 1, wxEXPAND|wxALIGN_CENTER);
	
	//reset clipping 6
	wxBoxSizer* sizer_6 = new wxBoxSizer(wxHORIZONTAL);
#ifndef _DARWIN
	m_clip_reset_btn = new wxButton(this, ID_ClipResetBtn, "Reset Clips",
									wxDefaultPosition, wxSize(120, 22));
#else
	m_clip_reset_btn = new wxButton(this, ID_ClipResetBtn, "Reset Clips",
									wxDefaultPosition, wxSize(125, 30));
#endif
	m_clip_reset_btn->SetBitmap(wxGetBitmapFromMemory(refresh));
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
	m_rot_reset_btn->SetBitmap(wxGetBitmapFromMemory(refresh));
	sizer_8->Add(5, 5, 0);
	sizer_8->Add(m_rot_reset_btn, 0, wxALIGN_CENTER);

	//sliders for rotating clipping planes 
	//x
	wxBoxSizer* sizer_rx = new wxBoxSizer(wxVERTICAL);
	st = new wxStaticText(this, 0, "X");
	m_x_rot_sldr = new wxSlider(this, ID_XRotSldr, 0, -180, 180,
		wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL|wxSL_INVERSE);
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
		wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL|wxSL_INVERSE);
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
		wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL|wxSL_INVERSE);
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
	sizer_v->Add(10, 10, 0);
	sizer_v->Add(sizer_1, 0, wxALIGN_CENTER);
	sizer_v->Add(5, 5, 0);
	sizer_v->Add(sizer_2, 1, wxEXPAND);
	sizer_v->Add(5, 5, 0);
	sizer_v->Add(sizer_3, 0, wxALIGN_CENTER|wxEXPAND);
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

ClippingView::~ClippingView()
{
}

void ClippingView::SetChannLink(bool chann)
{
	m_link_channels->ToggleTool(ID_LinkChannelsChk,chann);
	if(chann)
		m_link_channels->SetToolNormalBitmap(ID_LinkChannelsChk,
			wxGetBitmapFromMemory(link_text));
	else
		m_link_channels->SetToolNormalBitmap(ID_LinkChannelsChk,
			wxGetBitmapFromMemory(unlink_text));

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
	m_vd = vd;
	m_sel_type = 2;
	GetSettings();
}

void ClippingView::SetMeshData(MeshData* md)
{
	m_md = md;
	m_sel_type = 3;
	GetSettings();
}

void ClippingView::SetDataManager(DataManager* mgr)
{
	m_mgr = mgr;
}

void ClippingView::RefreshVRenderViews(bool interactive)
{
	VRenderFrame* vrender_frame = (VRenderFrame*)m_frame;
	vrender_frame->RefreshVRenderViews(false, interactive);
}

void ClippingView::GetSettings()
{
	if (!m_vd && !m_md)
	{
		DisableAll();
		return;
	}

	EnableAll();

	int resx, resy, resz;
	int resx_n, resy_n, resz_n;
	switch (m_sel_type)
	{
	case 2:	//volume
		m_vd->GetResolution(resx, resy, resz);
		resx_n = resy_n = resz_n = 0;
		break;
	case 3:	//mesh
		resx = resy = resz = 0;
		resx_n = resy_n = resz_n = 0;
		break;
	}
	//slider range
	m_x1_clip_sldr->SetRange(resx_n, resx);
	m_x2_clip_sldr->SetRange(resx_n, resx);
	m_y1_clip_sldr->SetRange(resy_n, resy);
	m_y2_clip_sldr->SetRange(resy_n, resy);
	m_z1_clip_sldr->SetRange(resz_n, resz);
	m_z2_clip_sldr->SetRange(resz_n, resz);
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

	vector<Plane*> *planes = 0;
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
	Plane* plane = 0;
	int val = 0;
	double abcd[4];

	//x1
	plane = (*planes)[0];
	plane->get_copy(abcd);
	val = fabs(abcd[3]*resx)+0.499;
	m_x1_clip_sldr->SetValue(val);
	double percent = (double)val/(double)m_x1_clip_sldr->GetMax();
	int barsize = (m_x1_clip_sldr->GetSize().GetHeight() - 20);
	str = wxString::Format("%d", val);
	m_x1_clip_text->ChangeValue(str);
	//x2
	plane = (*planes)[1];
	plane->get_copy(abcd);
	val = fabs(abcd[3]*resx)+0.499;
	m_x2_clip_sldr->SetValue(val);
	m_xBar->SetPosition(wxPoint(20,10+percent*barsize));
	m_xBar->SetSize(wxSize(3,barsize*((double)
		(val - m_x1_clip_sldr->GetValue())/(double)m_x1_clip_sldr->GetMax())));
	str = wxString::Format("%d", val);
	m_x2_clip_text->ChangeValue(str);
	//y1
	plane = (*planes)[2];
	plane->get_copy(abcd);
	val = fabs(abcd[3]*resy)+0.499;
	m_y1_clip_sldr->SetValue(val);
	percent = (double)val/(double)m_y1_clip_sldr->GetMax();
	barsize = (m_y1_clip_sldr->GetSize().GetHeight() - 20);
	str = wxString::Format("%d", val);
	m_y1_clip_text->ChangeValue(str);
	//y2
	plane = (*planes)[3];
	plane->get_copy(abcd);
	val = fabs(abcd[3]*resy)+0.499;
	m_y2_clip_sldr->SetValue(val);
	m_yBar->SetPosition(wxPoint(20,10+percent*barsize));
	m_yBar->SetSize(wxSize(3,barsize*((double)
		(val - m_y1_clip_sldr->GetValue())/(double)m_y1_clip_sldr->GetMax())));
	str = wxString::Format("%d", val);
	m_y2_clip_text->ChangeValue(str);
	//z1
	plane = (*planes)[4];
	plane->get_copy(abcd);
	val = fabs(abcd[3]*resz)+0.499;
	m_z1_clip_sldr->SetValue(val);
	percent = (double)val/(double)m_z1_clip_sldr->GetMax();
	barsize = (m_z1_clip_sldr->GetSize().GetHeight() - 20);
	str = wxString::Format("%d", val);
	m_z1_clip_text->ChangeValue(str);
	//z2
	plane = (*planes)[5];
	plane->get_copy(abcd);
	val = fabs(abcd[3]*resz)+0.499;
	m_zBar->SetPosition(wxPoint(20,10+percent*barsize));
	m_zBar->SetSize(wxSize(3,barsize*((double)
		(val - m_z1_clip_sldr->GetValue())/(double)m_z1_clip_sldr->GetMax())));
	m_z2_clip_sldr->SetValue(val);
	str = wxString::Format("%d", val);
	m_z2_clip_text->ChangeValue(str);

}

void ClippingView::OnLinkChannelsCheck(wxCommandEvent &event)
{
	if (m_link_channels->GetToolState(ID_LinkChannelsChk))
	{
		if (!m_mgr)
			return;
		m_link_channels->SetToolNormalBitmap(ID_LinkChannelsChk,
			wxGetBitmapFromMemory(link_text));

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

			vector<Plane*> *planes = 0;
			if (vd->GetVR())
				planes = vd->GetVR()->get_planes();
			if (!planes)
				continue;
			if (planes->size() != 6)
				continue;

			int resx, resy, resz;
			vd->GetResolution(resx, resy, resz);

			(*planes)[0]->ChangePlane(Point(double(x1_val)/double(resx), 0.0, 0.0), Vector(1.0, 0.0, 0.0));
			(*planes)[1]->ChangePlane(Point(double(x2_val)/double(resx), 0.0, 0.0), Vector(-1.0, 0.0, 0.0));
			(*planes)[2]->ChangePlane(Point(0.0, double(y1_val)/double(resy), 0.0), Vector(0.0, 1.0, 0.0));
			(*planes)[3]->ChangePlane(Point(0.0, double(y2_val)/double(resy), 0.0), Vector(0.0, -1.0, 0.0));
			(*planes)[4]->ChangePlane(Point(0.0, 0.0, double(z1_val)/double(resz)), Vector(0.0, 0.0, 1.0));
			(*planes)[5]->ChangePlane(Point(0.0, 0.0, double(z2_val)/double(resz)), Vector(0.0, 0.0, -1.0));
		}

		RefreshVRenderViews();

	}
	else
		m_link_channels->SetToolNormalBitmap(ID_LinkChannelsChk,
			wxGetBitmapFromMemory(unlink_text));
}

void ClippingView::OnClipResetBtn(wxCommandEvent &event)
{
	if (!m_vd)
		return;
	int resx, resy, resz;
	m_vd->GetResolution(resx, resy, resz);

	vector<Plane*> *planes = 0;
	if (m_vd->GetVR())
		planes = m_vd->GetVR()->get_planes();
	if(!planes)
		return;
	if (planes->size()!=6)
		return;

	Plane* plane = (*planes)[0];
	plane->ChangePlane(Point(0.0, 0.0, 0.0), Vector(1.0, 0.0, 0.0));
	plane = (*planes)[1];
	plane->ChangePlane(Point(1.0, 0.0, 0.0), Vector(-1.0, 0.0, 0.0));
	plane = (*planes)[2];
	plane->ChangePlane(Point(0.0, 0.0, 0.0), Vector(0.0, 1.0, 0.0));
	plane = (*planes)[3];
	plane->ChangePlane(Point(0.0, 1.0, 0.0), Vector(0.0, -1.0, 0.0));
	plane = (*planes)[4];
	plane->ChangePlane(Point(0.0, 0.0, 0.0), Vector(0.0, 0.0, 1.0));
	plane = (*planes)[5];
	plane->ChangePlane(Point(0.0, 0.0, 1.0), Vector(0.0, 0.0, -1.0));

	//links
	m_link_x = false;
	m_link_y = false;
	m_link_z = false;
	m_check_tb->ToggleTool(ID_LinkXChk,false);
	m_check_tb->ToggleTool(ID_LinkYChk,false);
	m_check_tb->ToggleTool(ID_LinkZChk,false);
	m_check_tb->SetToolNormalBitmap(ID_LinkXChk,
		wxGetBitmapFromMemory(unlink));
	m_check_tb->SetToolNormalBitmap(ID_LinkYChk,
		wxGetBitmapFromMemory(unlink));
	m_check_tb->SetToolNormalBitmap(ID_LinkZChk,
		wxGetBitmapFromMemory(unlink));

	//controls
	//sliders
	m_x1_clip_sldr->SetValue(0);
	m_x2_clip_sldr->SetValue(resx);
	m_y1_clip_sldr->SetValue(0);
	m_y2_clip_sldr->SetValue(resy);
	m_z1_clip_sldr->SetValue(0);
	m_z2_clip_sldr->SetValue(resz);
	//texts
	m_x1_clip_text->SetValue("0");
	m_x2_clip_text->SetValue(wxString::Format("%d", resx));
	m_y1_clip_text->SetValue("0");
	m_y2_clip_text->SetValue(wxString::Format("%d", resy));
	m_z1_clip_text->SetValue("0");
	m_z2_clip_text->SetValue(wxString::Format("%d", resz));

	//link
	if (m_link_channels->GetToolState(ID_LinkChannelsChk))
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

				(*planes)[0]->ChangePlane(Point(0.0, 0.0, 0.0), Vector(1.0, 0.0, 0.0));
				(*planes)[1]->ChangePlane(Point(1.0, 0.0, 0.0), Vector(-1.0, 0.0, 0.0));
				(*planes)[2]->ChangePlane(Point(0.0, 0.0, 0.0), Vector(0.0, 1.0, 0.0));
				(*planes)[3]->ChangePlane(Point(0.0, 1.0, 0.0), Vector(0.0, -1.0, 0.0));
				(*planes)[4]->ChangePlane(Point(0.0, 0.0, 0.0), Vector(0.0, 0.0, 1.0));
				(*planes)[5]->ChangePlane(Point(0.0, 0.0, 1.0), Vector(0.0, 0.0, -1.0));
			}
		}
	}

	//views
	VRenderFrame* vrender_frame = (VRenderFrame*)m_frame;
	if (vrender_frame)
	{
		for (int i=0; i<(int)vrender_frame->GetViewList()->size(); i++)
		{
			VRenderView *vrv = (*vrender_frame->GetViewList())[i];
			if (vrv)
			{
				vrv->m_glview->m_clip_mask = -1;

				double rotx, roty, rotz;
				vrv->GetRotations(rotx, roty, rotz);
				vrv->SetRotations(rotx, roty, rotz);

				vrv->RefreshGL();
			}
		}
	}
}

void ClippingView::OnX1ClipChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	m_x1_clip_text->SetValue(wxString::Format("%d", ival));
}

void ClippingView::OnX1ClipEdit(wxCommandEvent &event)
{
	if (!m_vd)
		return;
	int resx, resy, resz;
	m_vd->GetResolution(resx, resy, resz);

	vector<Plane*> *planes = 0;
	if (m_vd->GetVR())
		planes = m_vd->GetVR()->get_planes();
	if(!planes)
		return;
	if (planes->size()!=6)
		return;

	wxString str = m_x1_clip_text->GetValue();
	long ival = 0;
	str.ToLong(&ival);
	int ival2 = m_x2_clip_sldr->GetValue();
	double val, val2;

	if (m_link_x)
	{
		if (ival + m_x_sldr_dist > resx)
		{
			ival = resx - m_x_sldr_dist;
			ival2 = resx;
		}
		else
			ival2 = ival+m_x_sldr_dist;
	}
	else if (ival > ival2)
		ival = ival2;

	val = (double)ival/(double)resx;
	//str = wxString::Format("%d", ival);
	//m_x1_clip_text->ChangeValue(str);
	m_x1_clip_sldr->SetValue(ival);
	int barsize = (m_x1_clip_sldr->GetSize().GetHeight() - 20);
	m_xBar->SetPosition(wxPoint(20,10+val*barsize));
	m_xBar->SetSize(wxSize(3,barsize*((double)
		(m_x2_clip_sldr->GetValue()-ival)/(double)m_x1_clip_sldr->GetMax())));
	Plane* plane = (*planes)[0];
	plane->ChangePlane(Point(val, 0.0, 0.0), Vector(1.0, 0.0, 0.0));
	if (m_link_x)
	{
		val2 = (double)ival2/(double)resx;
		str = wxString::Format("%d", ival2);
		m_x2_clip_text->ChangeValue(str);
		m_x2_clip_sldr->SetValue(ival2);
		plane = (*planes)[1];
		plane->ChangePlane(Point(val2, 0.0, 0.0), Vector(-1.0, 0.0, 0.0));
	}

	if (m_link_channels->GetToolState(ID_LinkChannelsChk))
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

				(*planes)[0]->ChangePlane(Point(val, 0.0, 0.0), Vector(1.0, 0.0, 0.0));
				if (m_link_x)
					(*planes)[1]->ChangePlane(Point(val2, 0.0, 0.0), Vector(-1.0, 0.0, 0.0));
			}
		}
	}

	VRenderFrame* vrender_frame = (VRenderFrame*)m_frame;
	if (vrender_frame)
	{
		for (int i=0; i<(int)vrender_frame->GetViewList()->size(); i++)
		{
			VRenderView *vrv = (*vrender_frame->GetViewList())[i];
			if (vrv)
			{
				if (m_link_x)
					vrv->m_glview->m_clip_mask = 3;
				else
					vrv->m_glview->m_clip_mask = 1;

				double rotx, roty, rotz;
				vrv->GetRotations(rotx, roty, rotz);
				vrv->SetRotations(rotx, roty, rotz);
			}
		}
	}
	RefreshVRenderViews(true);
}

void ClippingView::OnX2ClipChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	int ival2 = m_x1_clip_sldr->GetValue();

	if (!m_link_x && ival<ival2)
	{
		ival = ival2;
		m_x2_clip_sldr->SetValue(ival);
	}
	m_x2_clip_text->SetValue(wxString::Format("%d", ival));
}

void ClippingView::OnX2ClipEdit(wxCommandEvent &event)
{
	if (!m_vd)
		return;
	int resx, resy, resz;
	m_vd->GetResolution(resx, resy, resz);

	vector<Plane*> *planes = 0;
	if (m_vd->GetVR())
		planes = m_vd->GetVR()->get_planes();
	if (!planes)
		return;
	if (planes->size()!=6)
		return;

	wxString str = m_x2_clip_text->GetValue();
	long ival = 0;
	str.ToLong(&ival);
	int ival2 = m_x1_clip_sldr->GetValue();
	double val, val2;

	if (m_link_x)
	{
		if (ival - m_x_sldr_dist < 0)
		{
			ival = m_x_sldr_dist;
			ival2 = 0;
		}
		else
			ival2 = ival - m_x_sldr_dist;
	}
	else if (ival < ival2)
		return;

	val = (double)ival/(double)resx;
	//str = wxString::Format("%d", ival);
	//m_x2_clip_text->ChangeValue(str);
	m_x2_clip_sldr->SetValue(ival);
	int barsize = (m_x1_clip_sldr->GetSize().GetHeight() - 20);
	m_xBar->SetPosition(wxPoint(20,10+((double)m_x1_clip_sldr->GetValue()/
		(double)m_x1_clip_sldr->GetMax())*barsize));
	m_xBar->SetSize(wxSize(3,barsize*((double)
		(ival - m_x1_clip_sldr->GetValue())/(double)m_x1_clip_sldr->GetMax())));
	Plane* plane = (*planes)[1];
	plane->ChangePlane(Point(val, 0.0, 0.0), Vector(-1.0, 0.0, 0.0));
	if (m_link_x)
	{
		val2 = (double)ival2/(double)resx;
		str = wxString::Format("%d", ival2);
		m_x1_clip_text->ChangeValue(str);
		m_x1_clip_sldr->SetValue(ival2);
		plane = (*planes)[0];
		plane->ChangePlane(Point(val2, 0.0, 0.0), Vector(1.0, 0.0, 0.0));
	}

	if (m_link_channels->GetToolState(ID_LinkChannelsChk))
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
				(*planes)[1]->ChangePlane(Point(val, 0.0, 0.0), Vector(-1.0, 0.0, 0.0));
				if (m_link_x)
					(*planes)[0]->ChangePlane(Point(val2, 0.0, 0.0), Vector(1.0, 0.0, 0.0));
			}
		}
	}

	VRenderFrame* vrender_frame = (VRenderFrame*)m_frame;
	if (vrender_frame)
	{
		for (int i=0; i<(int)vrender_frame->GetViewList()->size(); i++)
		{
			VRenderView *vrv = (*vrender_frame->GetViewList())[i];
			if (vrv)
			{
				if (m_link_x)
					vrv->m_glview->m_clip_mask = 3;
				else
					vrv->m_glview->m_clip_mask = 2;

				double rotx, roty, rotz;
				vrv->GetRotations(rotx, roty, rotz);
				vrv->SetRotations(rotx, roty, rotz);
			}
		}
	}

	RefreshVRenderViews(true);
}

void ClippingView::OnY1ClipChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	m_y1_clip_text->SetValue(wxString::Format("%d", ival));
}

void ClippingView::OnY1ClipEdit(wxCommandEvent &event)
{
	if (!m_vd)
		return;
	int resx, resy, resz;
	m_vd->GetResolution(resx, resy, resz);

	vector<Plane*> *planes = 0;
	if (m_vd->GetVR())
		planes = m_vd->GetVR()->get_planes();
	if (!planes)
		return;
	if (planes->size()!=6)
		return;

	wxString str = m_y1_clip_text->GetValue();
	long ival = 0;
	str.ToLong(&ival);
	int ival2 = m_y2_clip_sldr->GetValue();
	double val, val2;

	if (m_link_y)
	{
		if (ival + m_y_sldr_dist >resy)
		{
			ival = resy - m_y_sldr_dist;
			ival2 = resy;
		}
		else
			ival2 = ival+m_y_sldr_dist;
	}
	else if (ival > ival2)
		ival = ival2;

	val = (double)ival/(double)resy;
	//str = wxString::Format("%d", ival);
	//m_y1_clip_text->ChangeValue(str);
	m_y1_clip_sldr->SetValue(ival);
	int barsize = (m_y1_clip_sldr->GetSize().GetHeight() - 20);
	m_yBar->SetPosition(wxPoint(20,10+val*barsize));
	m_yBar->SetSize(wxSize(3,barsize*((double)
		(m_y2_clip_sldr->GetValue()-ival)/(double)m_y1_clip_sldr->GetMax())));
	Plane* plane = (*planes)[2];
	plane->ChangePlane(Point(0.0, val, 0.0), Vector(0.0, 1.0, 0.0));
	if (m_link_y)
	{
		val2 = (double)ival2/(double)resy;
		str = wxString::Format("%d", ival2);
		m_y2_clip_text->ChangeValue(str);
		m_y2_clip_sldr->SetValue(ival2);
		plane = (*planes)[3];
		plane->ChangePlane(Point(0.0, val2, 0.0), Vector(0.0, -1.0, 0.0));
	}

	if (m_link_channels->GetToolState(ID_LinkChannelsChk))
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

				(*planes)[2]->ChangePlane(Point(0.0, val, 0.0), Vector(0.0, 1.0, 0.0));
				if (m_link_y)
					(*planes)[3]->ChangePlane(Point(0.0, val2, 0.0), Vector(0.0, -1.0, 0.0));
			}
		}
	}

	VRenderFrame* vrender_frame = (VRenderFrame*)m_frame;
	if (vrender_frame)
	{
		for (int i=0; i<(int)vrender_frame->GetViewList()->size(); i++)
		{
			VRenderView *vrv = (*vrender_frame->GetViewList())[i];
			if (vrv)
			{
				if (m_link_y)
					vrv->m_glview->m_clip_mask = 12;
				else
					vrv->m_glview->m_clip_mask = 4;

				double rotx, roty, rotz;
				vrv->GetRotations(rotx, roty, rotz);
				vrv->SetRotations(rotx, roty, rotz);
			}
		}
	}

	RefreshVRenderViews(true);
}

void ClippingView::OnY2ClipChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	int ival2 = m_y1_clip_sldr->GetValue();

	if (!m_link_y && ival<ival2)
	{
		ival = ival2;
		m_y2_clip_sldr->SetValue(ival);
	}
	m_y2_clip_text->SetValue(wxString::Format("%d", ival));
}

void ClippingView::OnY2ClipEdit(wxCommandEvent &event)
{
	if (!m_vd)
		return;
	int resx, resy, resz;
	m_vd->GetResolution(resx, resy, resz);
	vector<Plane*> *planes = 0;
	if (m_vd->GetVR())
		planes = m_vd->GetVR()->get_planes();
	if (!planes)
		return;
	if (planes->size()!=6)
		return;

	wxString str = m_y2_clip_text->GetValue();
	long ival = 0;
	str.ToLong(&ival);
	int ival2 = m_y1_clip_sldr->GetValue();
	double val, val2;

	if (m_link_y)
	{
		if (ival - m_y_sldr_dist < 0)
		{
			ival = m_y_sldr_dist;
			ival2 = 0;
		}
		else
			ival2 = ival - m_y_sldr_dist;
	}
	else if (ival < ival2)
		return;
	
	val = (double)ival/(double)resy;
	//str = wxString::Format("%d", ival);
	//m_y2_clip_text->ChangeValue(str);
	m_y2_clip_sldr->SetValue(ival);
	int barsize = (m_y1_clip_sldr->GetSize().GetHeight() - 20);
	m_yBar->SetPosition(wxPoint(20,10+((double)m_y1_clip_sldr->GetValue()/
		(double)m_y1_clip_sldr->GetMax())*barsize));
	m_yBar->SetSize(wxSize(3,barsize*((double)
		(ival - m_y1_clip_sldr->GetValue())/(double)m_y1_clip_sldr->GetMax())));
	Plane* plane = (*planes)[3];
	plane->ChangePlane(Point(0.0, val, 0.0), Vector(0.0, -1.0, 0.0));
	if (m_link_y)
	{
		val2 = (double)ival2/(double)resy;
		str = wxString::Format("%d", ival2);
		m_y1_clip_text->ChangeValue(str);
		m_y1_clip_sldr->SetValue(ival2);
		plane = (*planes)[2];
		plane->ChangePlane(Point(0.0, val2, 0.0), Vector(0.0, 1.0, 0.0));
	}

	if (m_link_channels->GetToolState(ID_LinkChannelsChk))
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

				(*planes)[3]->ChangePlane(Point(0.0, val, 0.0), Vector(0.0, -1.0, 0.0));
				if (m_link_y)
					(*planes)[2]->ChangePlane(Point(0.0, val2, 0.0), Vector(0.0, 1.0, 0.0));
			}
		}
	}

	VRenderFrame* vrender_frame = (VRenderFrame*)m_frame;
	if (vrender_frame)
	{
		for (int i=0; i<(int)vrender_frame->GetViewList()->size(); i++)
		{
			VRenderView *vrv = (*vrender_frame->GetViewList())[i];
			if (vrv)
			{
				if (m_link_y)
					vrv->m_glview->m_clip_mask = 12;
				else
					vrv->m_glview->m_clip_mask = 8;

				double rotx, roty, rotz;
				vrv->GetRotations(rotx, roty, rotz);
				vrv->SetRotations(rotx, roty, rotz);
			}
		}
	}

	RefreshVRenderViews(true);
}

void ClippingView::OnZ1ClipChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	m_z1_clip_text->SetValue(wxString::Format("%d", ival));
}

void ClippingView::OnZ1ClipEdit(wxCommandEvent &event)
{
	if (!m_vd)
		return;
	int resx, resy, resz;
	m_vd->GetResolution(resx, resy, resz);
	vector<Plane*> *planes = 0;
	if (m_vd->GetVR())
		planes = m_vd->GetVR()->get_planes();
	if (!planes)
		return;
	if (planes->size()!=6)
		return;

	wxString str = m_z1_clip_text->GetValue();
	long ival = 0;
	str.ToLong(&ival);
	int ival2 = m_z2_clip_sldr->GetValue();
	double val, val2;

	if (m_link_z)
	{
		if (ival + m_z_sldr_dist > resz)
		{
			ival = resz - m_z_sldr_dist;
			ival2 = resz;
		}
		else
			ival2 = ival+m_z_sldr_dist;
	}
	else if (ival > ival2)
		ival = ival2;
	
	val = (double)ival/(double)resz;
	//str = wxString::Format("%d", ival);
	//m_z1_clip_text->ChangeValue(str);
	m_z1_clip_sldr->SetValue(ival);
	int barsize = (m_z1_clip_sldr->GetSize().GetHeight() - 20);
	m_zBar->SetPosition(wxPoint(20,10+val*barsize));
	m_zBar->SetSize(wxSize(3,barsize*((double)
		(m_z2_clip_sldr->GetValue()-ival)/(double)m_z1_clip_sldr->GetMax())));
	Plane* plane = (*planes)[4];
	plane->ChangePlane(Point(0.0, 0.0, val), Vector(0.0, 0.0, 1.0));
	if (m_link_z)
	{
		val2 = (double)ival2/(double)resz;
		str = wxString::Format("%d", ival2);
		m_z2_clip_text->ChangeValue(str);
		m_z2_clip_sldr->SetValue(ival2);
		plane = (*planes)[5];
		plane->ChangePlane(Point(0.0, 0.0, val2), Vector(0.0, 0.0, -1.0));
	}

	if (m_link_channels->GetToolState(ID_LinkChannelsChk))
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

				(*planes)[4]->ChangePlane(Point(0.0, 0.0, val), Vector(0.0, 0.0, 1.0));
				if (m_link_z)
					(*planes)[5]->ChangePlane(Point(0.0, 0.0, val2), Vector(0.0, 0.0, -1.0));
			}
		}
	}

	VRenderFrame* vrender_frame = (VRenderFrame*)m_frame;
	if (vrender_frame)
	{
		for (int i=0; i<(int)vrender_frame->GetViewList()->size(); i++)
		{
			VRenderView *vrv = (*vrender_frame->GetViewList())[i];
			if (vrv)
			{
				if (m_link_z)
					vrv->m_glview->m_clip_mask = 48;
				else
					vrv->m_glview->m_clip_mask = 16;

				double rotx, roty, rotz;
				vrv->GetRotations(rotx, roty, rotz);
				vrv->SetRotations(rotx, roty, rotz);
			}
		}
	}

	RefreshVRenderViews(true);
}

void ClippingView::OnZ2ClipChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	int ival2 = m_z1_clip_sldr->GetValue();

	if (!m_link_z && ival<ival2)
	{
		ival = ival2;
		m_z2_clip_sldr->SetValue(ival);
	}
	m_z2_clip_text->SetValue(wxString::Format("%d", ival));
}

void ClippingView::OnZ2ClipEdit(wxCommandEvent &event)
{
	if (!m_vd)
		return;
	int resx, resy, resz;
	m_vd->GetResolution(resx, resy, resz);
	vector<Plane*> *planes = 0;
	if (m_vd->GetVR())
		planes = m_vd->GetVR()->get_planes();
	if (!planes)
		return;
	if (planes->size()!=6)
		return;

	wxString str = m_z2_clip_text->GetValue();
	long ival = 0;
	str.ToLong(&ival);
	int ival2 = m_z1_clip_sldr->GetValue();
	double val, val2;

	if (m_link_z)
	{
		if (ival - m_z_sldr_dist < 0)
		{
			ival = m_z_sldr_dist;
			ival2 = 0;
		}
		else
			ival2 = ival - m_z_sldr_dist;
	}
	else if (ival < ival2)
		return;
	
	val = (double)ival/(double)resz;
	//str = wxString::Format("%d", ival);
	//m_z2_clip_text->ChangeValue(str);
	m_z2_clip_sldr->SetValue(ival);
	int barsize = (m_z1_clip_sldr->GetSize().GetHeight() - 20);
	m_zBar->SetPosition(wxPoint(20,10+((double)m_z1_clip_sldr->GetValue()/
		(double)m_z1_clip_sldr->GetMax())*barsize));
	m_zBar->SetSize(wxSize(3,barsize*((double)
		(ival - m_z1_clip_sldr->GetValue())/(double)m_z1_clip_sldr->GetMax())));
	Plane* plane = (*planes)[5];
	plane->ChangePlane(Point(0.0, 0.0, val), Vector(0.0, 0.0, -1.0));
	if (m_link_z)
	{
		val2 = (double)ival2/(double)resz;
		str = wxString::Format("%d", ival2);
		m_z1_clip_text->ChangeValue(str);
		m_z1_clip_sldr->SetValue(ival2);
		plane = (*planes)[4];
		plane->ChangePlane(Point(0.0, 0.0, val2), Vector(0.0, 0.0, 1.0));
	}

	if (m_link_channels->GetToolState(ID_LinkChannelsChk))
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

				(*planes)[5]->ChangePlane(Point(0.0, 0.0, val), Vector(0.0, 0.0, -1.0));
				if (m_link_z)
					(*planes)[4]->ChangePlane(Point(0.0, 0.0, val2), Vector(0.0, 0.0, 1.0));
			}
		}
	}

	VRenderFrame* vrender_frame = (VRenderFrame*)m_frame;
	if (vrender_frame)
	{
		for (int i=0; i<(int)vrender_frame->GetViewList()->size(); i++)
		{
			VRenderView *vrv = (*vrender_frame->GetViewList())[i];
			if (vrv)
			{
				if (m_link_z)
					vrv->m_glview->m_clip_mask = 48;
				else
					vrv->m_glview->m_clip_mask = 32;

				double rotx, roty, rotz;
				vrv->GetRotations(rotx, roty, rotz);
				vrv->SetRotations(rotx, roty, rotz);
			}
		}
	}

	RefreshVRenderViews(true);
}

void ClippingView::OnIdle(wxIdleEvent &event)
{
	if (!IsShown())
		return;
	int sz = m_xpanel->GetSize().GetHeight();
	if (m_x1_clip_sldr->GetSize().GetHeight() != sz) {
		m_x1_clip_sldr->SetSize(20,sz);
		m_x2_clip_sldr->SetSize(20,sz);
		m_y1_clip_sldr->SetSize(20,sz);
		m_y2_clip_sldr->SetSize(20,sz);
		m_z1_clip_sldr->SetSize(20,sz);
		m_z2_clip_sldr->SetSize(20,sz);
		int barsize = sz - 20;
		//x
		int mx = m_x1_clip_sldr->GetMax();
		int v1 = m_x1_clip_sldr->GetValue();
		int v2 = m_x2_clip_sldr->GetValue();
		double clipSz = ((double)(v2 - v1))/((double)mx);
		double pct = ((double)v1)/((double)mx);
		m_xBar->SetPosition(wxPoint(20,10+pct*barsize));
		m_xBar->SetSize(wxSize(3,barsize*clipSz));
		//y
		mx = m_y1_clip_sldr->GetMax();
		v1 = m_y1_clip_sldr->GetValue();
		v2 = m_y2_clip_sldr->GetValue();
		clipSz = ((double)(v2 - v1))/((double)mx);
		pct = ((double)v1)/((double)mx);
		m_yBar->SetPosition(wxPoint(20,10+pct*barsize));
		m_yBar->SetSize(wxSize(3,barsize*clipSz));
		//z
		mx = m_z1_clip_sldr->GetMax();
		v1 = m_z1_clip_sldr->GetValue();
		v2 = m_z2_clip_sldr->GetValue();
		clipSz = ((double)(v2 - v1))/((double)mx);
		pct = ((double)v1)/((double)mx);
		m_zBar->SetPosition(wxPoint(20,10+pct*barsize));
		m_zBar->SetSize(wxSize(3,barsize*clipSz));
	}

	int i;
	VRenderFrame* vrender_frame = (VRenderFrame*)m_frame;
	if (!vrender_frame)
		return;

	for (i=0; i<vrender_frame->GetViewNum(); i++)
	{
		VRenderView *vrv = vrender_frame->GetView(i);
		if (vrv)
		{
			if (vrv->m_glview->m_capture)
				return;
		}
	}

	wxPoint pos = wxGetMousePosition();
	wxRect reg = GetScreenRect();
	if (reg.Contains(pos))
	{
		if (!m_draw_clip)
		{
			vector <VRenderView*>* vrv_list = vrender_frame->GetViewList();
			for (i=0; i<(int)vrv_list->size(); i++)
			{
				if ((*vrv_list)[i])
				{
					(*vrv_list)[i]->m_glview->m_draw_clip = true;
					(*vrv_list)[i]->m_glview->m_clip_mask = -1;
				}
			}
			RefreshVRenderViews();
			m_draw_clip = true;
		}
	}
	else
	{
		if (m_draw_clip)
		{
			vector <VRenderView*>* vrv_list = vrender_frame->GetViewList();
			for (i=0; i<(int)vrv_list->size(); i++)
			{
				if ((*vrv_list)[i])
					(*vrv_list)[i]->m_glview->m_draw_clip = false;
			}
			RefreshVRenderViews();
			m_draw_clip = false;
		}
	}

}

void ClippingView::OnLinkXCheck(wxCommandEvent &event)
{   
	m_link_x = m_check_tb->GetToolState(ID_LinkXChk);
	if (m_link_x)
	{
		m_x_sldr_dist = m_x2_clip_sldr->GetValue() -
			m_x1_clip_sldr->GetValue();
		m_check_tb->SetToolNormalBitmap(ID_LinkXChk,
			wxGetBitmapFromMemory(link));
	} else {
		m_check_tb->SetToolNormalBitmap(ID_LinkXChk,
			wxGetBitmapFromMemory(unlink));
	}
}

void ClippingView::OnLinkYCheck(wxCommandEvent &event)
{
	m_link_y = m_check_tb->GetToolState(ID_LinkYChk);
	if (m_link_y)
	{
		m_y_sldr_dist = m_y2_clip_sldr->GetValue() -
			m_y1_clip_sldr->GetValue();
		m_check_tb->SetToolNormalBitmap(ID_LinkYChk,
			wxGetBitmapFromMemory(link));
	} else {
		m_check_tb->SetToolNormalBitmap(ID_LinkYChk,
			wxGetBitmapFromMemory(unlink));
	}
}

void ClippingView::OnLinkZCheck(wxCommandEvent &event)
{
	m_link_z = m_check_tb->GetToolState(ID_LinkZChk);
	if (m_link_z)
	{
		m_z_sldr_dist = m_z2_clip_sldr->GetValue() -
			m_z1_clip_sldr->GetValue();
		m_check_tb->SetToolNormalBitmap(ID_LinkZChk,
			wxGetBitmapFromMemory(link));
	} else {
		m_check_tb->SetToolNormalBitmap(ID_LinkZChk,
			wxGetBitmapFromMemory(unlink));
	}
}

void ClippingView::OnSetZeroBtn(wxCommandEvent &event)
{
	VRenderFrame* vrender_frame = (VRenderFrame*)m_frame;
	if (vrender_frame)
	{
		for (int i=0; i<(int)vrender_frame->GetViewList()->size(); i++)
		{
			VRenderView *vrv = (*vrender_frame->GetViewList())[i];
			if (vrv)
			{
				vrv->SetClipMode(2);
				vrv->RefreshGL();
				double rotx, roty, rotz;
				vrv->GetClippingPlaneRotations(rotx, roty, rotz);
				m_x_rot_sldr->SetValue(int(rotx));
				m_y_rot_sldr->SetValue(int(roty));
				m_z_rot_sldr->SetValue(int(rotz));
				m_x_rot_text->ChangeValue(wxString::Format("%.1f", rotx));
				m_y_rot_text->ChangeValue(wxString::Format("%.1f", roty));
				m_z_rot_text->ChangeValue(wxString::Format("%.1f", rotz));
			}
		}
	}
}

void ClippingView::OnRotResetBtn(wxCommandEvent &event)
{
	VRenderFrame* vrender_frame = (VRenderFrame*)m_frame;
	if (vrender_frame)
	{
		for (int i=0; i<(int)vrender_frame->GetViewList()->size(); i++)
		{
			VRenderView *vrv = (*vrender_frame->GetViewList())[i];
			if (vrv)
			{
				//reset rotations
				vrv->SetClippingPlaneRotations(0.0, 0.0, 0.0);
				vrv->RefreshGL();
			}
		}
	}
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
	int val = event.GetPosition();
	wxString str = wxString::Format("%.1f", double(val));
	m_x_rot_text->SetValue(str);
}

void ClippingView::OnXRotEdit(wxCommandEvent &event)
{
	wxString str = m_x_rot_text->GetValue();
	double val = 0.0;
	str.ToDouble(&val);
	m_x_rot_sldr->SetValue(int(val));

	VRenderFrame* vrender_frame = (VRenderFrame*)m_frame;
	if (vrender_frame)
	{
		for (int i=0; i<(int)vrender_frame->GetViewList()->size(); i++)
		{
			VRenderView *vrv = (*vrender_frame->GetViewList())[i];
			if (vrv)
			{
				double rotx, roty, rotz;
				vrv->GetClippingPlaneRotations(rotx, roty, rotz);
				vrv->SetClippingPlaneRotations(val, roty, rotz);
				vrv->RefreshGL();
			}
		}
	}
}

void ClippingView::OnYRotChange(wxScrollEvent &event)
{
	int val = event.GetPosition();
	wxString str = wxString::Format("%.1f", double(val));
	m_y_rot_text->SetValue(str);
}

void ClippingView::OnYRotEdit(wxCommandEvent &event)
{
	wxString str = m_y_rot_text->GetValue();
	double val = 0.0;
	str.ToDouble(&val);
	m_y_rot_sldr->SetValue(int(val));

	VRenderFrame* vrender_frame = (VRenderFrame*)m_frame;
	if (vrender_frame)
	{
		for (int i=0; i<(int)vrender_frame->GetViewList()->size(); i++)
		{
			VRenderView *vrv = (*vrender_frame->GetViewList())[i];
			if (vrv)
			{
				double rotx, roty, rotz;
				vrv->GetClippingPlaneRotations(rotx, roty, rotz);
				vrv->SetClippingPlaneRotations(rotx, val, rotz);
				vrv->RefreshGL();
			}
		}
	}
}

void ClippingView::OnZRotChange(wxScrollEvent &event)
{
	int val = event.GetPosition();
	wxString str = wxString::Format("%.1f", double(val));
	m_z_rot_text->SetValue(str);
}

void ClippingView::OnZRotEdit(wxCommandEvent &event)
{
	wxString str = m_z_rot_text->GetValue();
	double val = 0.0;
	str.ToDouble(&val);
	m_z_rot_sldr->SetValue(int(val));

	VRenderFrame* vrender_frame = (VRenderFrame*)m_frame;
	if (vrender_frame)
	{
		for (int i=0; i<(int)vrender_frame->GetViewList()->size(); i++)
		{
			VRenderView *vrv = (*vrender_frame->GetViewList())[i];
			if (vrv)
			{
				double rotx, roty, rotz;
				vrv->GetClippingPlaneRotations(rotx, roty, rotz);
				vrv->SetClippingPlaneRotations(rotx, roty, val);
				vrv->RefreshGL();
			}
		}
	}
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
	if (m_link_channels->GetToolState(ID_LinkChannelsChk))
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
		m_x2_clip_sldr->SetFocus();
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
		m_y2_clip_sldr->SetFocus();
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
		m_z2_clip_sldr->SetFocus();
	}

	//reset others
	vector<Plane*> *planes = 0;
	if (m_vd->GetVR())
		planes = m_vd->GetVR()->get_planes();
	if(!planes)
		return;
	if (planes->size()!=6)
		return;

	if (id == ID_X1ClipSldr ||
		id == ID_X2ClipSldr)
	{
		m_link_y = false;
		m_check_tb->ToggleTool(ID_LinkYChk,false);
		m_check_tb->SetToolNormalBitmap(ID_LinkYChk,
			wxGetBitmapFromMemory(unlink));
		m_link_z = false;
		m_check_tb->ToggleTool(ID_LinkZChk,false);
		m_check_tb->SetToolNormalBitmap(ID_LinkZChk,
			wxGetBitmapFromMemory(unlink));
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
		m_link_x = false;
		m_check_tb->ToggleTool(ID_LinkXChk,false);
		m_check_tb->SetToolNormalBitmap(ID_LinkXChk,
			wxGetBitmapFromMemory(unlink));
		m_link_z = false;
		m_check_tb->ToggleTool(ID_LinkZChk,false);
		m_check_tb->SetToolNormalBitmap(ID_LinkZChk,
			wxGetBitmapFromMemory(unlink));
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
		m_link_x = false;
		m_check_tb->ToggleTool(ID_LinkXChk,false);
		m_check_tb->SetToolNormalBitmap(ID_LinkXChk,
			wxGetBitmapFromMemory(unlink));
		m_link_y = false;
		m_check_tb->ToggleTool(ID_LinkYChk,false);
		m_check_tb->SetToolNormalBitmap(ID_LinkYChk,
			wxGetBitmapFromMemory(unlink));
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
	m_link_y = false;
	m_check_tb->ToggleTool(ID_LinkYChk,false);
	m_check_tb->SetToolNormalBitmap(ID_LinkYChk,
		wxGetBitmapFromMemory(unlink));
	m_link_z = false;
	m_check_tb->ToggleTool(ID_LinkZChk,false);
	m_check_tb->SetToolNormalBitmap(ID_LinkZChk,
		wxGetBitmapFromMemory(unlink));
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
	m_link_x = false;
	m_check_tb->ToggleTool(ID_LinkXChk,false);
	m_check_tb->SetToolNormalBitmap(ID_LinkXChk,
		wxGetBitmapFromMemory(unlink));
	m_link_z = false;
	m_check_tb->ToggleTool(ID_LinkZChk,false);
	m_check_tb->SetToolNormalBitmap(ID_LinkZChk,
		wxGetBitmapFromMemory(unlink));
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
	m_link_x = false;
	m_check_tb->ToggleTool(ID_LinkXChk,false);
	m_check_tb->SetToolNormalBitmap(ID_LinkXChk,
		wxGetBitmapFromMemory(unlink));
	m_link_y = false;
	m_check_tb->ToggleTool(ID_LinkYChk,false);
	m_check_tb->SetToolNormalBitmap(ID_LinkYChk,
		wxGetBitmapFromMemory(unlink));
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

	if (dir == 0)
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
	}
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

	if (key == wxKeyCode(','))
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
	}

	event.Skip();
}

void ClippingView::EnableAll()
{
	m_link_channels->Enable();
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
	m_check_tb->Enable();
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
	m_link_channels->Disable();
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
	m_check_tb->Disable();
	m_clip_reset_btn->Disable();
	m_yz_clip_btn->Disable();
	m_xz_clip_btn->Disable();
	m_xy_clip_btn->Disable();
	m_yz_dist_text->Disable();
	m_xz_dist_text->Disable();
	m_xy_dist_text->Disable();
}
