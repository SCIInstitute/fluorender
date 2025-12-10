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
#include <ClipPlanePanel.h>
#include <Global.h>
#include <Names.h>
#include <MainSettings.h>
#include <GlobalStates.h>
#include <MainFrame.h>
#include <RenderView.h>
#include <CurrentObjects.h>
#include <VolumeData.h>
#include <MeshData.h>
#include <DataManager.h>
#include <VolumeRenderer.h>
#include <MeshRenderer.h>
#include <ClippingBoxRenderer.h>
#include <RendererFactory.h>
#include <compatibility.h>
#include <png_resource.h>
#include <icons.h>
#include <wxFadeButton.h>
#include <wxDoubleSlider.h>
#include <wxSingleSlider.h>
#include <wxUndoableToolbar.h>
#include <wx/valnum.h>
#include <wx/gbsizer.h>
#include <Debug.h>

ClipPlanePanel::ClipPlanePanel(
	MainFrame* frame,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name) :
	TabbedPanel(frame, frame, pos, size, style, name),
m_enable_all(true)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);
	Freeze();
	//SetDoubleBuffered(true);

	//sync channels 1
	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	m_toolbar = new wxUndoableToolbar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	wxBitmapBundle bitmap;
	bitmap = wxGetBitmap(sync_chan);
	m_toolbar->AddCheckTool(ID_LinkChannelsBtn, "Sync All Channels",
		bitmap, wxNullBitmap,
		"Link all data channels to this cropping",
		"Link all data channels to this cropping");
	bitmap = wxGetBitmap(hold_clip);
	m_toolbar->AddCheckTool(ID_HoldPlanesBtn, "Hold Plane Display",
		bitmap, wxNullBitmap,
		"Clipping planes are always shown",
		"Clipping planes are always shown");
	bitmap = wxGetBitmap(clip_normal);
	m_toolbar->AddTool(ID_PlaneModesBtn, "Display Modes",
		bitmap, "Toggle clipping plane display modes");
	m_toolbar->SetToolLongHelp(ID_PlaneModesBtn, "Toggle clipping plane display modes");
	m_toolbar->Bind(wxEVT_TOOL, &ClipPlanePanel::OnToolbar, this);
	m_toolbar->Realize();
	sizer_1->Add(5, 5, 0);
	sizer_1->Add(m_toolbar, 0, wxALIGN_CENTER, 0);

	//notebook
	m_notebook = new wxAuiNotebook(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize,
		wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE |
		wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_TAB_EXTERNAL_MOVE | wxNO_BORDER);
	m_notebook->AddPage(CreateTranslatePage(m_notebook), "Translate", true);
	m_notebook->AddPage(CreateRotatePage(m_notebook), "Rotate");

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(sizer_1, 0, wxALIGN_CENTER);
	sizer->Add(m_notebook, 1, wxEXPAND);
	SetSizer(sizer);
	Layout();
	SetAutoLayout(true);
	SetScrollRate(10, 10);

	Bind(wxEVT_IDLE, &ClipPlanePanel::OnIdle, this);

	EnableAll(false);

	//add controls
	glbin.add_undo_control(m_clipx_sldr);
	glbin.add_undo_control(m_clipy_sldr);
	glbin.add_undo_control(m_clipz_sldr);
	glbin.add_undo_control(m_x_rot_sldr);
	glbin.add_undo_control(m_y_rot_sldr);
	glbin.add_undo_control(m_z_rot_sldr);
	glbin.add_undo_control(m_toolbar);

	Thaw();
}

ClipPlanePanel::~ClipPlanePanel()
{
	//delete controls
	glbin.del_undo_control(m_clipx_sldr);
	glbin.del_undo_control(m_clipy_sldr);
	glbin.del_undo_control(m_clipz_sldr);
	glbin.del_undo_control(m_x_rot_sldr);
	glbin.del_undo_control(m_y_rot_sldr);
	glbin.del_undo_control(m_z_rot_sldr);
	glbin.del_undo_control(m_toolbar);

	m_frame->SetFocusVRenderViews(0);
}

wxWindow* ClipPlanePanel::CreateTranslatePage(wxWindow* parent)
{
	wxScrolledWindow* page = new wxScrolledWindow(parent);

	//validator: integer
	wxIntegerValidator<int> vald_int;
	wxBitmapBundle bitmap;

	//sliders for clipping planes
	bool inverse_slider = glbin_settings.m_inverse_slider;
	long ls = inverse_slider ? wxSL_VERTICAL : (wxSL_VERTICAL | wxSL_INVERSE);
	
	wxGridBagSizer* sizer_v = new wxGridBagSizer(5, 0);
	//x
	m_clip_x_st = new wxFadeButton(page, wxID_ANY, "X",
		wxDefaultPosition, FromDIP(wxSize(34, 20)));
	m_clipx_sldr = new wxDoubleSlider(page, wxID_ANY, 0, 512, 0, 512,
		wxDefaultPosition, wxDefaultSize, ls);
	m_clipx_sldr->SetRangeColor(wxColor(255, 128, 128));
	m_clipx_sldr->SetThumbColor(wxColor(255, 128, 128), wxColor(255, 128, 255));
	m_x1_clip_text = new wxTextCtrl(page, wxID_ANY, "0",
		wxDefaultPosition, FromDIP(wxSize(34, 20)), wxTE_CENTRE, vald_int);
	m_x2_clip_text = new wxTextCtrl(page, wxID_ANY, "512",
		wxDefaultPosition, FromDIP(wxSize(34, 20)), wxTE_CENTRE, vald_int);
	m_linkx_tb = new wxToolBar(page, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	bitmap = wxGetBitmap(unlink);
	m_linkx_tb->AddCheckTool(0, "",
		bitmap, wxNullBitmap,
		"Link two X clipping planes",
		"Link two X clipping planes");
	m_clip_x_st->SetFontBold();
	m_clip_x_st->Bind(wxEVT_BUTTON, &ClipPlanePanel::OnClipXMF, this);
	m_clipx_sldr->Bind(wxEVT_SCROLL_CHANGED, &ClipPlanePanel::OnClipXChange, this);
	m_clipx_sldr->SetHistoryIndicator(m_clip_x_st);
	m_x1_clip_text->Bind(wxEVT_TEXT, &ClipPlanePanel::OnX1ClipEdit, this);
	m_x2_clip_text->Bind(wxEVT_TEXT, &ClipPlanePanel::OnX2ClipEdit, this);
	m_linkx_tb->Bind(wxEVT_TOOL, &ClipPlanePanel::OnLinkXCheck, this);
	m_linkx_tb->Realize();
	//add the items
	sizer_v->Add(m_clip_x_st, wxGBPosition(0, 0), wxGBSpan(1, 1), wxEXPAND);
	if (inverse_slider)
	{
		sizer_v->Add(m_x1_clip_text, wxGBPosition(1, 0), wxGBSpan(1, 1), wxEXPAND);
		sizer_v->Add(m_clipx_sldr, wxGBPosition(2, 0), wxGBSpan(1, 1), wxEXPAND);
		sizer_v->Add(m_x2_clip_text, wxGBPosition(3, 0), wxGBSpan(1, 1), wxEXPAND);
	}
	else
	{
		sizer_v->Add(m_x2_clip_text, wxGBPosition(1, 0), wxGBSpan(1, 1), wxEXPAND);
		sizer_v->Add(m_clipx_sldr, wxGBPosition(2, 0), wxGBSpan(1, 1), wxEXPAND);
		sizer_v->Add(m_x1_clip_text, wxGBPosition(3, 0), wxGBSpan(1, 1), wxEXPAND);
	}
	sizer_v->Add(m_linkx_tb, wxGBPosition(4, 0), wxGBSpan(1, 1), wxEXPAND | wxALIGN_CENTER);

	//y
	m_clip_y_st = new wxFadeButton(page, wxID_ANY, "Y",
		wxDefaultPosition, FromDIP(wxSize(34, 20)));
	m_clipy_sldr = new wxDoubleSlider(page, wxID_ANY, 0, 512, 0, 512,
		wxDefaultPosition, wxDefaultSize, ls);
	m_clipy_sldr->SetRangeColor(wxColor(128, 255, 128));
	m_clipy_sldr->SetThumbColor(wxColor(128, 255, 128), wxColor(255, 255, 128));
	m_y1_clip_text = new wxTextCtrl(page, wxID_ANY, "0",
		wxDefaultPosition, FromDIP(wxSize(34, 20)), wxTE_CENTRE, vald_int);
	m_y2_clip_text = new wxTextCtrl(page, wxID_ANY, "512",
		wxDefaultPosition, FromDIP(wxSize(34, 20)), wxTE_CENTRE, vald_int);
	m_linky_tb = new wxToolBar(page, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	m_linky_tb->AddCheckTool(0, "",
		bitmap, wxNullBitmap,
		"Link two Y clipping planes",
		"Link two Y clipping planes");
	m_clip_y_st->SetFontBold();
	m_clip_y_st->Bind(wxEVT_BUTTON, &ClipPlanePanel::OnClipYMF, this);
	m_clipy_sldr->Bind(wxEVT_SCROLL_CHANGED, &ClipPlanePanel::OnClipYChange, this);
	m_clipy_sldr->SetHistoryIndicator(m_clip_y_st);
	m_y1_clip_text->Bind(wxEVT_TEXT, &ClipPlanePanel::OnY1ClipEdit, this);
	m_y2_clip_text->Bind(wxEVT_TEXT, &ClipPlanePanel::OnY2ClipEdit, this);
	m_linky_tb->Bind(wxEVT_TOOL, &ClipPlanePanel::OnLinkYCheck, this);
	m_linky_tb->Realize();
	//add the items
	sizer_v->Add(m_clip_y_st, wxGBPosition(0, 1), wxGBSpan(1, 1), wxEXPAND);
	if (inverse_slider)
	{
		sizer_v->Add(m_y1_clip_text, wxGBPosition(1, 1), wxGBSpan(1, 1), wxEXPAND);
		sizer_v->Add(m_clipy_sldr, wxGBPosition(2, 1), wxGBSpan(1, 1), wxEXPAND);
		sizer_v->Add(m_y2_clip_text, wxGBPosition(3, 1), wxGBSpan(1, 1), wxEXPAND);
	}
	else
	{
		sizer_v->Add(m_y2_clip_text, wxGBPosition(1, 1), wxGBSpan(1, 1), wxEXPAND);
		sizer_v->Add(m_clipy_sldr, wxGBPosition(2, 1), wxGBSpan(1, 1), wxEXPAND);
		sizer_v->Add(m_y1_clip_text, wxGBPosition(3, 1), wxGBSpan(1, 1), wxEXPAND);
	}
	sizer_v->Add(m_linky_tb, wxGBPosition(4, 1), wxGBSpan(1, 1), wxEXPAND | wxALIGN_CENTER);

	//z
	//wxPanel * zpanel = new wxPanel(page);
	m_clip_z_st = new wxFadeButton(page, wxID_ANY, "Z",
		wxDefaultPosition, FromDIP(wxSize(34, 20)));
	m_clipz_sldr = new wxDoubleSlider(page, wxID_ANY, 0, 512, 0, 512,
		wxPoint(0, 0), wxDefaultSize, ls);
	m_clipz_sldr->SetRangeColor(wxColor(128, 128, 255));
	m_clipz_sldr->SetThumbColor(wxColor(128, 128, 255), wxColor(128, 255, 255));
	m_z1_clip_text = new wxTextCtrl(page, wxID_ANY, "0",
		wxDefaultPosition, FromDIP(wxSize(34, 20)), wxTE_CENTRE, vald_int);
	m_z2_clip_text = new wxTextCtrl(page, wxID_ANY, "512",
		wxDefaultPosition, FromDIP(wxSize(34, 20)), wxTE_CENTRE, vald_int);
	m_linkz_tb = new wxToolBar(page, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	m_linkz_tb->AddCheckTool(0, "",
		bitmap, wxNullBitmap,
		"Link two Z clipping planes",
		"Link two Z clipping planes");
	m_clip_z_st->SetFontBold();
	m_clip_z_st->Bind(wxEVT_BUTTON, &ClipPlanePanel::OnClipZMF, this);
	m_clipz_sldr->Bind(wxEVT_SCROLL_CHANGED, &ClipPlanePanel::OnClipZChange, this);
	m_clipz_sldr->SetHistoryIndicator(m_clip_z_st);
	m_z1_clip_text->Bind(wxEVT_TEXT, &ClipPlanePanel::OnZ1ClipEdit, this);
	m_z2_clip_text->Bind(wxEVT_TEXT, &ClipPlanePanel::OnZ2ClipEdit, this);
	m_linkz_tb->Bind(wxEVT_TOOL, &ClipPlanePanel::OnLinkZCheck, this);
	m_linkz_tb->Realize();
	//add the items
	sizer_v->Add(m_clip_z_st, wxGBPosition(0, 2), wxGBSpan(1, 1), wxEXPAND);
	if (inverse_slider)
	{
		sizer_v->Add(m_z1_clip_text, wxGBPosition(1, 2), wxGBSpan(1, 1), wxEXPAND);
		sizer_v->Add(m_clipz_sldr, wxGBPosition(2, 2), wxGBSpan(1, 1), wxEXPAND);
		sizer_v->Add(m_z2_clip_text, wxGBPosition(3, 2), wxGBSpan(1, 1), wxEXPAND);
	}
	else
	{
		sizer_v->Add(m_z2_clip_text, wxGBPosition(1, 2), wxGBSpan(1, 1), wxEXPAND);
		sizer_v->Add(m_clipz_sldr, wxGBPosition(2, 2), wxGBSpan(1, 1), wxEXPAND);
		sizer_v->Add(m_z1_clip_text, wxGBPosition(3, 2), wxGBSpan(1, 1), wxEXPAND);
	}
	sizer_v->Add(m_linkz_tb, wxGBPosition(4, 2), wxGBSpan(1, 1), wxEXPAND | wxALIGN_CENTER);

	//link sliders
	m_clipx_sldr->Bind(wxEVT_RIGHT_DOWN, &ClipPlanePanel::OnClipXRClick, this);
	m_clipy_sldr->Bind(wxEVT_RIGHT_DOWN, &ClipPlanePanel::OnClipYRClick, this);
	m_clipz_sldr->Bind(wxEVT_RIGHT_DOWN, &ClipPlanePanel::OnClipZRClick, this);

	//text
	sizer_v->Add(new wxStaticText(page, 0, "Clip Section Size"),
		wxGBPosition(5, 0), wxGBSpan(1, 3), wxEXPAND | wxALIGN_CENTER);

	//clip buttons
	m_yz_clip_btn = new wxButton(page, wxID_ANY, "YZ",
		wxDefaultPosition, FromDIP(wxSize(34, 22)));
	m_xz_clip_btn = new wxButton(page, wxID_ANY, "XZ",
		wxDefaultPosition, FromDIP(wxSize(34, 22)));
	m_xy_clip_btn = new wxButton(page, wxID_ANY, "XY",
		wxDefaultPosition, FromDIP(wxSize(34, 22)));
	m_yz_clip_btn->Bind(wxEVT_BUTTON, &ClipPlanePanel::OnYZClipBtn, this);
	m_xz_clip_btn->Bind(wxEVT_BUTTON, &ClipPlanePanel::OnXZClipBtn, this);
	m_xy_clip_btn->Bind(wxEVT_BUTTON, &ClipPlanePanel::OnXYClipBtn, this);
	sizer_v->Add(m_yz_clip_btn, wxGBPosition(6, 0), wxGBSpan(1, 1), wxEXPAND);
	sizer_v->Add(m_xz_clip_btn, wxGBPosition(6, 1), wxGBSpan(1, 1), wxEXPAND);
	sizer_v->Add(m_xy_clip_btn, wxGBPosition(6, 2), wxGBSpan(1, 1), wxEXPAND);

	//clip distance
	m_yz_dist_text = new wxTextCtrl(page, wxID_ANY, "1",
		wxDefaultPosition, FromDIP(wxSize(34, 22)), wxTE_CENTRE, vald_int);
	m_xz_dist_text = new wxTextCtrl(page, wxID_ANY, "1",
		wxDefaultPosition, FromDIP(wxSize(34, 22)), wxTE_CENTRE, vald_int);
	m_xy_dist_text = new wxTextCtrl(page, wxID_ANY, "1",
		wxDefaultPosition, FromDIP(wxSize(34, 22)), wxTE_CENTRE, vald_int);
	m_yz_dist_text->Bind(wxEVT_TEXT, &ClipPlanePanel::OnClipDistXEdit, this);
	m_xz_dist_text->Bind(wxEVT_TEXT, &ClipPlanePanel::OnClipDistYEdit, this);
	m_xy_dist_text->Bind(wxEVT_TEXT, &ClipPlanePanel::OnClipDistZEdit, this);
	sizer_v->Add(m_yz_dist_text, wxGBPosition(7, 0), wxGBSpan(1, 1), wxEXPAND);
	sizer_v->Add(m_xz_dist_text, wxGBPosition(7, 1), wxGBSpan(1, 1), wxEXPAND);
	sizer_v->Add(m_xy_dist_text, wxGBPosition(7, 2), wxGBSpan(1, 1), wxEXPAND);

	//reset clipping
	m_clip_reset_btn = new wxButton(page, wxID_ANY, "Reset Clips",
		wxDefaultPosition, wxDefaultSize);
	m_clip_reset_btn->SetBitmap(wxGetBitmap(reset));
	m_clip_reset_btn->Bind(wxEVT_BUTTON, &ClipPlanePanel::OnClipResetBtn, this);
	sizer_v->Add(m_clip_reset_btn, wxGBPosition(8, 0), wxGBSpan(1, 3), wxEXPAND);

	sizer_v->AddGrowableCol(0);
	sizer_v->AddGrowableCol(1);
	sizer_v->AddGrowableCol(2);
	sizer_v->AddGrowableRow(2);
	page->SetSizer(sizer_v);
	page->SetAutoLayout(true);
	page->SetScrollRate(10, 10);
	return page;
}

wxWindow* ClipPlanePanel::CreateRotatePage(wxWindow* parent)
{
	wxScrolledWindow* page = new wxScrolledWindow(parent);

	//validator: floating point 1
	wxFloatingPointValidator<double> vald_fp1(1);
	vald_fp1.SetRange(-180.0, 180.0);

	wxGridBagSizer* sizer_v = new wxGridBagSizer(5, 0);
	//rotations
	bool inverse_slider = glbin_settings.m_inverse_slider;
	long ls = inverse_slider ? wxSL_VERTICAL : (wxSL_VERTICAL | wxSL_INVERSE);
	//sliders for rotating clipping planes 
	//x
	m_rot_x_st = new wxFadeButton(page, wxID_ANY, "X",
		wxDefaultPosition, FromDIP(wxSize(34, 20)));
	m_x_rot_sldr = new wxSingleSlider(page, wxID_ANY, 0, -180, 180,
		wxDefaultPosition, wxDefaultSize, ls);
	m_x_rot_sldr->SetRangeStyle(2);
	m_x_rot_text = new wxTextCtrl(page, wxID_ANY, "0.0",
		wxDefaultPosition, FromDIP(wxSize(34, 20)), wxTE_CENTRE, vald_fp1);
	m_x_rot_spin = new wxSpinButton(page, wxID_ANY,
		wxDefaultPosition, FromDIP(wxSize(30, 20)), wxSP_VERTICAL);
	m_x_rot_spin->SetRange(-0x8000, 0x7fff);
	m_rot_x_st->SetFontBold();
	m_rot_x_st->Bind(wxEVT_BUTTON, &ClipPlanePanel::OnRotXMF, this);
	m_x_rot_sldr->Bind(wxEVT_SCROLL_CHANGED, &ClipPlanePanel::OnXRotChange, this);
	m_x_rot_sldr->SetHistoryIndicator(m_rot_x_st);
	m_x_rot_text->Bind(wxEVT_TEXT, &ClipPlanePanel::OnXRotEdit, this);
	m_x_rot_spin->Bind(wxEVT_SPIN_UP, &ClipPlanePanel::OnXRotSpinUp, this);
	m_x_rot_spin->Bind(wxEVT_SPIN_DOWN, &ClipPlanePanel::OnXRotSpinDown, this);
	sizer_v->Add(m_rot_x_st, wxGBPosition(0, 0), wxGBSpan(1, 1), wxEXPAND);
	sizer_v->Add(m_x_rot_sldr, wxGBPosition(1, 0), wxGBSpan(1, 1), wxEXPAND);
	sizer_v->Add(m_x_rot_spin, wxGBPosition(2, 0), wxGBSpan(1, 1), wxEXPAND);
	sizer_v->Add(m_x_rot_text, wxGBPosition(3, 0), wxGBSpan(1, 1), wxEXPAND);
	//y
	m_rot_y_st = new wxFadeButton(page, wxID_ANY, "Y",
		wxDefaultPosition, FromDIP(wxSize(34, 20)));
	m_y_rot_sldr = new wxSingleSlider(page, wxID_ANY, 0, -180, 180,
		wxDefaultPosition, wxDefaultSize, ls);
	m_y_rot_sldr->SetRangeStyle(2);
	m_y_rot_text = new wxTextCtrl(page, wxID_ANY, "0.0",
		wxDefaultPosition, FromDIP(wxSize(34, 20)), wxTE_CENTRE, vald_fp1);
	m_y_rot_spin = new wxSpinButton(page, wxID_ANY,
		wxDefaultPosition, FromDIP(wxSize(30, 20)), wxSP_VERTICAL);
	m_y_rot_spin->SetRange(-0x8000, 0x7fff);
	m_rot_y_st->SetFontBold();
	m_rot_y_st->Bind(wxEVT_BUTTON, &ClipPlanePanel::OnRotXMF, this);
	m_y_rot_sldr->Bind(wxEVT_SCROLL_CHANGED, &ClipPlanePanel::OnYRotChange, this);
	m_y_rot_sldr->SetHistoryIndicator(m_rot_y_st);
	m_y_rot_text->Bind(wxEVT_TEXT, &ClipPlanePanel::OnYRotEdit, this);
	m_y_rot_spin->Bind(wxEVT_SPIN_UP, &ClipPlanePanel::OnYRotSpinUp, this);
	m_y_rot_spin->Bind(wxEVT_SPIN_DOWN, &ClipPlanePanel::OnYRotSpinDown, this);
	sizer_v->Add(m_rot_y_st, wxGBPosition(0, 1), wxGBSpan(1, 1), wxEXPAND);
	sizer_v->Add(m_y_rot_sldr, wxGBPosition(1, 1), wxGBSpan(1, 1), wxEXPAND);
	sizer_v->Add(m_y_rot_spin, wxGBPosition(2, 1), wxGBSpan(1, 1), wxEXPAND);
	sizer_v->Add(m_y_rot_text, wxGBPosition(3, 1), wxGBSpan(1, 1), wxEXPAND);
	//z
	m_rot_z_st = new wxFadeButton(page, wxID_ANY, "Z",
		wxDefaultPosition, FromDIP(wxSize(34, 20)));
	m_z_rot_sldr = new wxSingleSlider(page, wxID_ANY, 0, -180, 180,
		wxDefaultPosition, wxDefaultSize, ls);
	m_z_rot_sldr->SetRangeStyle(2);
	m_z_rot_text = new wxTextCtrl(page, wxID_ANY, "0.0",
		wxDefaultPosition, FromDIP(wxSize(34, 20)), wxTE_CENTRE, vald_fp1);
	m_z_rot_spin = new wxSpinButton(page, wxID_ANY,
		wxDefaultPosition, FromDIP(wxSize(30, 20)), wxSP_VERTICAL);
	m_z_rot_spin->SetRange(-0x8000, 0x7fff);
	m_rot_z_st->SetFontBold();
	m_rot_z_st->Bind(wxEVT_BUTTON, &ClipPlanePanel::OnRotZMF, this);
	m_z_rot_sldr->Bind(wxEVT_SCROLL_CHANGED, &ClipPlanePanel::OnZRotChange, this);
	m_z_rot_sldr->SetHistoryIndicator(m_rot_z_st);
	m_z_rot_text->Bind(wxEVT_TEXT, &ClipPlanePanel::OnZRotEdit, this);
	m_z_rot_spin->Bind(wxEVT_SPIN_UP, &ClipPlanePanel::OnZRotSpinUp, this);
	m_z_rot_spin->Bind(wxEVT_SPIN_DOWN, &ClipPlanePanel::OnZRotSpinDown, this);
	sizer_v->Add(m_rot_z_st, wxGBPosition(0, 2), wxGBSpan(1, 1), wxEXPAND);
	sizer_v->Add(m_z_rot_sldr, wxGBPosition(1, 2), wxGBSpan(1, 1), wxEXPAND);
	sizer_v->Add(m_z_rot_spin, wxGBPosition(2, 2), wxGBSpan(1, 1), wxEXPAND);
	sizer_v->Add(m_z_rot_text, wxGBPosition(3, 2), wxGBSpan(1, 1), wxEXPAND);

	//set sero rotation for clipping planes 7
	m_set_zero_btn = new wxButton(page, wxID_ANY, "Align to View",
		wxDefaultPosition, wxDefaultSize);
	m_set_zero_btn->SetBitmap(wxGetBitmap(align));
	m_set_zero_btn->Bind(wxEVT_BUTTON, &ClipPlanePanel::OnSetZeroBtn, this);
	sizer_v->Add(m_set_zero_btn, wxGBPosition(4, 0), wxGBSpan(1, 3), wxEXPAND);

	//reset rotation
	wxBoxSizer* sizer_3 = new wxBoxSizer(wxHORIZONTAL);
	m_rot_reset_btn = new wxButton(page, wxID_ANY, "Reset to 0",
		wxDefaultPosition, wxDefaultSize);
	m_rot_reset_btn->SetBitmap(wxGetBitmap(reset));
	m_rot_reset_btn->Bind(wxEVT_BUTTON, &ClipPlanePanel::OnRotResetBtn, this);
	sizer_v->Add(m_rot_reset_btn, wxGBPosition(5, 0), wxGBSpan(1, 3), wxEXPAND);

	sizer_v->AddGrowableCol(0);
	sizer_v->AddGrowableCol(1);
	sizer_v->AddGrowableCol(2);
	sizer_v->AddGrowableRow(1);
	page->SetSizer(sizer_v);
	page->SetAutoLayout(true);
	page->SetScrollRate(10, 10);
	return page;
}

void ClipPlanePanel::FluoUpdate(const fluo::ValueCollection& vc)
{
	if (FOUND_VALUE(gstNull))
		return;
	bool update_all = vc.empty() || FOUND_VALUE(gstCurrentSelect);

	auto obj = GetObject();
	if (!obj)
	{
		EnableAll(false);
		return;
	}
	auto& cb = obj->GetClippingBox();
	auto fc = obj->GetColor();

	//mf button tips
	if (update_all || FOUND_VALUE(gstMultiFuncTips))
	{
		switch (glbin_settings.m_mulfunc)
		{
		case 0:
			m_clip_x_st->SetToolTip("Synchronize the X clipping values for channels in render view");
			m_clip_y_st->SetToolTip("Synchronize the Y clipping values for channels in render view");
			m_clip_z_st->SetToolTip("Synchronize the Z clipping values for channels in render view");
			m_rot_x_st->SetToolTip("Synchronize the X rotation values for channels in render view");
			m_rot_y_st->SetToolTip("Synchronize the Y rotation values for channels in render view");
			m_rot_z_st->SetToolTip("Synchronize the Z rotation values for channels in render view");
			break;
		case 1:
			m_clip_x_st->SetToolTip("Move mouse cursor in render view and change X clipping using the mouse wheel");
			m_clip_y_st->SetToolTip("Move mouse cursor in render view and change Y clipping using the mouse wheel");
			m_clip_z_st->SetToolTip("Move mouse cursor in render view and change Z clipping using the mouse wheel");
			m_rot_x_st->SetToolTip("Move mouse cursor in render view and change X rotation using the mouse wheel");
			m_rot_y_st->SetToolTip("Move mouse cursor in render view and change Y rotation using the mouse wheel");
			m_rot_z_st->SetToolTip("Move mouse cursor in render view and change Z rotation using the mouse wheel");
			break;
		case 2:
			m_clip_x_st->SetToolTip("Reset X clipping values");
			m_clip_y_st->SetToolTip("Reset Y clipping values");
			m_clip_z_st->SetToolTip("Reset Z clipping values");
			m_rot_x_st->SetToolTip("Reset X rotation values");
			m_rot_y_st->SetToolTip("Reset Y rotation values");
			m_rot_z_st->SetToolTip("Reset Z rotation values");
			break;
		case 4:
			m_clip_x_st->SetToolTip("Undo X clipping value changes");
			m_clip_y_st->SetToolTip("Undo Y clipping value changes");
			m_clip_z_st->SetToolTip("Undo Z clipping value changes");
			m_rot_x_st->SetToolTip("Undo X rotation value changes");
			m_rot_y_st->SetToolTip("Undo Y rotation value changes");
			m_rot_z_st->SetToolTip("Undo Z rotation value changes");
			break;
		case 3:
		case 5:
			m_clip_x_st->SetToolTip("No function assigned");
			m_clip_y_st->SetToolTip("No function assigned");
			m_clip_z_st->SetToolTip("No function assigned");
			m_rot_x_st->SetToolTip("No function assigned");
			m_rot_y_st->SetToolTip("No function assigned");
			m_rot_z_st->SetToolTip("No function assigned");
			break;
		}
	}

	//link channels in view
	if (update_all || FOUND_VALUE(gstClipLinkChan))
	{
		m_toolbar->ToggleTool(ID_LinkChannelsBtn,
			glbin_settings.m_clip_link);
	}

	//hold clipping planes
	if (update_all || FOUND_VALUE(gstClipHold))
	{
		m_toolbar->ToggleTool(ID_HoldPlanesBtn,
			glbin_settings.m_clip_hold);
	}

	//modes
	if (update_all || FOUND_VALUE(gstClipPlaneMode))
	{
		auto mode = static_cast<flrd::ClippingRenderMode>(glbin_settings.m_clip_mode);
		switch (mode)
		{
		case flrd::ClippingRenderMode::ColoredFront:
			m_toolbar->SetToolNormalBitmap(ID_PlaneModesBtn,
				wxGetBitmap(clip_normal));
			break;
		case flrd::ClippingRenderMode::FrameAll:
			m_toolbar->SetToolNormalBitmap(ID_PlaneModesBtn,
				wxGetBitmap(clip_frame6));
			break;
		case flrd::ClippingRenderMode::FrameFront:
			m_toolbar->SetToolNormalBitmap(ID_PlaneModesBtn,
				wxGetBitmap(clip_frame3));
			break;
		case flrd::ClippingRenderMode::TransFront:
			m_toolbar->SetToolNormalBitmap(ID_PlaneModesBtn,
				wxGetBitmap(clip_low));
			break;
		case flrd::ClippingRenderMode::TransBack:
			m_toolbar->SetToolNormalBitmap(ID_PlaneModesBtn,
				wxGetBitmap(clip_low_back));
			break;
		case flrd::ClippingRenderMode::ColoredBack:
			m_toolbar->SetToolNormalBitmap(ID_PlaneModesBtn,
				wxGetBitmap(clip_normal_back));
			break;
		case flrd::ClippingRenderMode::None:
			m_toolbar->SetToolNormalBitmap(ID_PlaneModesBtn,
				wxGetBitmap(clip_none));
			break;
		}
	}

	wxColor c(fc.r() * 255, fc.g() * 255, fc.b() * 255);

	if (update_all || FOUND_VALUE(gstClipPlaneRanges))
	{
		//slider range
		auto bbox = cb.GetBBoxIndex();
		m_clipx_sldr->SetRange(bbox.minintx(), bbox.maxintx());
		m_clipy_sldr->SetRange(bbox.mininty(), bbox.maxinty());
		m_clipz_sldr->SetRange(bbox.minintz(), bbox.maxintz());
	}

	if (update_all || FOUND_VALUE(gstClipPlaneRangeColor))
	{
		m_clipx_sldr->SetRangeColor(c);
		m_clipy_sldr->SetRangeColor(c);
		m_clipz_sldr->SetRangeColor(c);
		m_clipx_sldr->Refresh();
		m_clipy_sldr->Refresh();
		m_clipz_sldr->Refresh();
	}

	//clip distance
	if (update_all || FOUND_VALUE(gstClipDist))
	{
		int dx = 1, dy = 1, dz = 1;
		dx = cb.GetLinkedDistIndex(fluo::ClipPlane::XNeg);
		dy = cb.GetLinkedDistIndex(fluo::ClipPlane::YNeg);
		dz = cb.GetLinkedDistIndex(fluo::ClipPlane::ZNeg);
		m_yz_dist_text->ChangeValue(
			wxString::Format("%d", dx));
		m_xz_dist_text->ChangeValue(
			wxString::Format("%d", dy));
		m_xy_dist_text->ChangeValue(
			wxString::Format("%d", dz));
	}

	bool bval;
	double dval;
	int ival;
	wxString str;

	bool linkx = m_clipx_sldr->GetLink();
	bool linky = m_clipy_sldr->GetLink();
	bool linkz = m_clipz_sldr->GetLink();
	m_clipx_sldr->SetLink(false);
	m_clipy_sldr->SetLink(false);
	m_clipz_sldr->SetLink(false);
	//x1
	if (update_all || FOUND_VALUE(gstClipX1))
	{
		dval = cb.GetClipIndex(fluo::ClipPlane::XNeg);
		ival = static_cast<int>(std::round(dval));
		m_clipx_sldr->ChangeLowValue(ival);
		str = wxString::Format("%d", ival);
		m_x1_clip_text->ChangeValue(str);
		m_x1_clip_text->Update();
	}
	//x2
	if (update_all || FOUND_VALUE(gstClipX2))
	{
		dval = cb.GetClipIndex(fluo::ClipPlane::XPos);
		ival = static_cast<int>(std::round(dval));
		m_clipx_sldr->ChangeHighValue(ival);
		str = wxString::Format("%d", ival);
		m_x2_clip_text->ChangeValue(str);
		m_x2_clip_text->Update();
	}
	//y1
	if (update_all || FOUND_VALUE(gstClipY1))
	{
		dval = cb.GetClipIndex(fluo::ClipPlane::YNeg);
		ival = static_cast<int>(std::round(dval));
		m_clipy_sldr->ChangeLowValue(ival);
		str = wxString::Format("%d", ival);
		m_y1_clip_text->ChangeValue(str);
		m_y1_clip_text->Update();
	}
	//y2
	if (update_all || FOUND_VALUE(gstClipY2))
	{
		dval = cb.GetClipIndex(fluo::ClipPlane::YPos);
		ival = static_cast<int>(std::round(dval));
		m_clipy_sldr->ChangeHighValue(ival);
		str = wxString::Format("%d", ival);
		m_y2_clip_text->ChangeValue(str);
		m_y2_clip_text->Update();
	}
	//z1
	if (update_all || FOUND_VALUE(gstClipZ1))
	{
		dval = cb.GetClipIndex(fluo::ClipPlane::ZNeg);
		ival = static_cast<int>(std::round(dval));
		m_clipz_sldr->ChangeLowValue(ival);
		str = wxString::Format("%d", ival);
		m_z1_clip_text->ChangeValue(str);
		m_z1_clip_text->Update();
	}
	//z2
	if (update_all || FOUND_VALUE(gstClipZ2))
	{
		dval = cb.GetClipIndex(fluo::ClipPlane::ZPos);
		ival = static_cast<int>(std::round(dval));
		m_clipz_sldr->ChangeHighValue(ival);
		str = wxString::Format("%d", ival);
		m_z2_clip_text->ChangeValue(str);
		m_z2_clip_text->Update();
	}
	m_clipx_sldr->SetLink(linkx);
	m_clipy_sldr->SetLink(linky);
	m_clipz_sldr->SetLink(linkz);

	//link
	if (update_all || FOUND_VALUE(gstClipLinkX))
	{
		bval = m_clipx_sldr->GetLink();
		if (bval != m_linkx_tb->GetToolState(0))
		{
			m_linkx_tb->ToggleTool(0, bval);
			if (bval)
				m_linkx_tb->SetToolNormalBitmap(0,
					wxGetBitmap(link));
			else
				m_linkx_tb->SetToolNormalBitmap(0,
					wxGetBitmap(unlink));
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
					wxGetBitmap(link));
			else
				m_linky_tb->SetToolNormalBitmap(0,
					wxGetBitmap(unlink));
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
					wxGetBitmap(link));
			else
				m_linkz_tb->SetToolNormalBitmap(0,
					wxGetBitmap(unlink));
		}
	}

	//rotations
	fluo::Vector rot = cb.GetEuler();
	//x
	if (update_all || FOUND_VALUE(gstClipRotX))
	{
		m_x_rot_sldr->ChangeValue(static_cast<int>(std::round(rot.x())));
		m_x_rot_text->ChangeValue(wxString::Format("%.1f", rot.x()));
		m_x_rot_text->Update();
	}
	//y
	if (update_all || FOUND_VALUE(gstClipRotY))
	{
		m_y_rot_sldr->ChangeValue(static_cast<int>(std::round(rot.y())));
		m_y_rot_text->ChangeValue(wxString::Format("%.1f", rot.y()));
		m_y_rot_text->Update();
	}
	//z
	if (update_all || FOUND_VALUE(gstClipRotZ))
	{
		m_z_rot_sldr->ChangeValue(static_cast<int>(std::round(rot.z())));
		m_z_rot_text->ChangeValue(wxString::Format("%.1f", rot.z()));
		m_z_rot_text->Update();
	}

	EnableAll(true);

}

void ClipPlanePanel::OnToolbar(wxCommandEvent& event)
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

void ClipPlanePanel::LinkChannels()
{
	//bool bval = m_toolbar->GetToolState(ID_LinkChannelsBtn);
	glbin_settings.m_clip_link = !glbin_settings.m_clip_link;
	bool bval = glbin_settings.m_clip_link;
	if (bval)
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

		std::array<int, 6> val = { (int)x1_val, (int)x2_val, (int)y1_val, (int)y2_val, (int)z1_val, (int)z2_val };
		SetClipValues(val);
	}
	FluoUpdate({ gstClipLinkChan });
}

void ClipPlanePanel::HoldPlanes()
{
	glbin_settings.m_clip_hold = m_toolbar->GetToolState(ID_HoldPlanesBtn);
	glbin_states.ClipDisplayChanged();
	SetPlaneMask(-1);
	FluoRefresh(2, { gstClipHold },
		{ glbin_current.GetViewId() });
}

void ClipPlanePanel::SetPlaneMode()
{
	int ival = glbin_settings.m_clip_mode;
	ival++;
	ival = ival > static_cast<int>(flrd::ClippingRenderMode::TransBack) ?
		static_cast<int>(flrd::ClippingRenderMode::None) : ival;
	glbin_settings.m_clip_mode = ival;
	auto base = glbin_renderer_factory.getOrCreate(gstClippingBoxRenderer);
	auto renderer = std::dynamic_pointer_cast<flrd::ClippingBoxRenderer>(base);
	if (renderer)
	{
		auto settings = std::dynamic_pointer_cast<flrd::ClippingBoxSettings>(renderer->getSettings());
		settings->mode = static_cast<flrd::ClippingRenderMode>(ival);
	}
	FluoRefresh(2, { gstClipPlaneMode },
		{ glbin_current.GetViewId() });
}

void ClipPlanePanel::OnClipResetBtn(wxCommandEvent& event)
{
	ResetClipValues();
}

void ClipPlanePanel::SyncClipValue(int i)
{
	int val1, val2;
	switch (i)
	{
	case 0:
		val1 = m_clipx_sldr->GetLowValue();
		val2 = m_clipx_sldr->GetHighValue();
		SetClipValues(fluo::ClipPlane::XNeg, val1, val2);
		break;
	case 1:
		val1 = m_clipy_sldr->GetLowValue();
		val2 = m_clipy_sldr->GetHighValue();
		SetClipValues(fluo::ClipPlane::YNeg, val1, val2);
		break;
	case 2:
		val1 = m_clipz_sldr->GetLowValue();
		val2 = m_clipz_sldr->GetHighValue();
		SetClipValues(fluo::ClipPlane::ZNeg, val1, val2);
		break;
	}
}

void ClipPlanePanel::OnClipXMF(wxCommandEvent& event)
{
	switch (glbin_settings.m_mulfunc)
	{
	case 0:
		SyncClipValue(0);
		break;
	case 1:
		m_frame->SetFocusVRenderViews(m_clipx_sldr);
		break;
	case 2:
		ResetClipValues(fluo::ClipPlane::XNeg);
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

void ClipPlanePanel::OnClipYMF(wxCommandEvent& event)
{
	switch (glbin_settings.m_mulfunc)
	{
	case 0:
		SyncClipValue(1);
		break;
	case 1:
		m_frame->SetFocusVRenderViews(m_clipy_sldr);
		break;
	case 2:
		ResetClipValues(fluo::ClipPlane::YNeg);
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

void ClipPlanePanel::OnClipZMF(wxCommandEvent& event)
{
	switch (glbin_settings.m_mulfunc)
	{
	case 0:
		SyncClipValue(2);
		break;
	case 1:
		m_frame->SetFocusVRenderViews(m_clipz_sldr);
		break;
	case 2:
		ResetClipValues(fluo::ClipPlane::ZNeg);
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

void ClipPlanePanel::OnClipXChange(wxScrollEvent& event)
{
	int ival1 = m_clipx_sldr->GetLowValue();
	int ival2 = m_clipx_sldr->GetHighValue();
	wxString str = event.GetString();
	if (m_clipx_sldr->GetLink())
		SetClipValues(fluo::ClipPlane::XNeg, ival1, ival2);
	else
	{
		if (str.Find("low") != wxNOT_FOUND)
			SetClipValue(fluo::ClipPlane::XNeg, ival1);
		else if (str.Find("high") != wxNOT_FOUND)
			SetClipValue(fluo::ClipPlane::XPos, ival2);
	}
}

void ClipPlanePanel::OnX1ClipEdit(wxCommandEvent& event)
{
	wxString str = m_x1_clip_text->GetValue();
	long ival = 0;
	str.ToLong(&ival);
	bool link = m_clipx_sldr->GetLink();
	SetClipValue(fluo::ClipPlane::XNeg, ival, link);
}

void ClipPlanePanel::OnX2ClipEdit(wxCommandEvent& event)
{
	wxString str = m_x2_clip_text->GetValue();
	long ival = 0;
	str.ToLong(&ival);
	bool link = m_clipx_sldr->GetLink();
	SetClipValue(fluo::ClipPlane::XPos, ival, link);
}

void ClipPlanePanel::OnClipYChange(wxScrollEvent& event)
{
	int ival1 = m_clipy_sldr->GetLowValue();
	int ival2 = m_clipy_sldr->GetHighValue();
	wxString str = event.GetString();
	if (m_clipy_sldr->GetLink())
		SetClipValues(fluo::ClipPlane::YNeg, ival1, ival2);
	else
	{
		if (str.Find("low") != wxNOT_FOUND)
			SetClipValue(fluo::ClipPlane::YNeg, ival1);
		else if (str.Find("high") != wxNOT_FOUND)
			SetClipValue(fluo::ClipPlane::YPos, ival2);
	}
}

void ClipPlanePanel::OnY1ClipEdit(wxCommandEvent& event)
{
	wxString str = m_y1_clip_text->GetValue();
	long ival = 0;
	str.ToLong(&ival);
	bool link = m_clipy_sldr->GetLink();
	SetClipValue(fluo::ClipPlane::YNeg, ival, link);
}

void ClipPlanePanel::OnY2ClipEdit(wxCommandEvent& event)
{
	wxString str = m_y2_clip_text->GetValue();
	long ival = 0;
	str.ToLong(&ival);
	bool link = m_clipy_sldr->GetLink();
	SetClipValue(fluo::ClipPlane::YPos, ival, link);
}

void ClipPlanePanel::OnClipZChange(wxScrollEvent& event)
{
	int ival1 = m_clipz_sldr->GetLowValue();
	int ival2 = m_clipz_sldr->GetHighValue();
	wxString str = event.GetString();
	if (m_clipz_sldr->GetLink())
		SetClipValues(fluo::ClipPlane::ZNeg, ival1, ival2);
	else
	{
		if (str.Find("low") != wxNOT_FOUND)
			SetClipValue(fluo::ClipPlane::ZNeg, ival1);
		else if (str.Find("high") != wxNOT_FOUND)
			SetClipValue(fluo::ClipPlane::ZPos, ival2);
	}
}

void ClipPlanePanel::OnZ1ClipEdit(wxCommandEvent& event)
{
	wxString str = m_z1_clip_text->GetValue();
	long ival = 0;
	str.ToLong(&ival);
	bool link = m_clipz_sldr->GetLink();
	SetClipValue(fluo::ClipPlane::ZNeg, ival, link);
}

void ClipPlanePanel::OnZ2ClipEdit(wxCommandEvent& event)
{
	wxString str = m_z2_clip_text->GetValue();
	long ival = 0;
	str.ToLong(&ival);
	bool link = m_clipz_sldr->GetLink();
	SetClipValue(fluo::ClipPlane::ZPos, ival, link);
}

void ClipPlanePanel::OnIdle(wxIdleEvent &event)
{
	if (!IsShown())
		return;
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;

	if (view->m_capture)
		return;

	wxPoint pos = wxGetMousePosition();
	wxRect reg = GetScreenRect();
	wxWindow *window = wxWindow::FindFocus();
	bool bval = window && reg.Contains(pos);
	glbin_states.m_mouse_in_clip_plane_panel = bval;
	if (glbin_states.ClipDisplayChanged())
	{
		SetPlaneMask(-1);
		FluoRefresh(3, { gstNull },
			{ glbin_current.GetViewId() });
	}
}

bool ClipPlanePanel::GetXLink()
{
	return m_clipx_sldr->GetLink();
}
bool ClipPlanePanel::GetYLink()
{
	return m_clipy_sldr->GetLink();
}
bool ClipPlanePanel::GetZLink()
{
	return m_clipz_sldr->GetLink();
}

void ClipPlanePanel::SetXLink(bool val)
{
	m_clipx_sldr->SetLink(val);
	m_linkx_tb->ToggleTool(0, val);
	if (val)
		m_linkx_tb->SetToolNormalBitmap(0,
			wxGetBitmap(link));
	else
		m_linkx_tb->SetToolNormalBitmap(0,
			wxGetBitmap(unlink));
}

void ClipPlanePanel::SetYLink(bool val)
{
	m_clipy_sldr->SetLink(val);
	m_linky_tb->ToggleTool(0, val);
	if (val)
		m_linky_tb->SetToolNormalBitmap(0,
			wxGetBitmap(link));
	else
		m_linky_tb->SetToolNormalBitmap(0,
			wxGetBitmap(unlink));
}

void ClipPlanePanel::SetZLink(bool val)
{
	m_clipz_sldr->SetLink(val);
	m_linkz_tb->ToggleTool(0, val);
	if (val)
		m_linkz_tb->SetToolNormalBitmap(0,
			wxGetBitmap(link));
	else
		m_linkz_tb->SetToolNormalBitmap(0,
			wxGetBitmap(unlink));
}

void ClipPlanePanel::SetClipValue(fluo::ClipPlane i, int val, bool link)
{
	auto view = glbin_current.render_view.lock();
	auto obj = GetObject();
	if (!obj)
		return;
	obj->SetClipValue(i, val);
	int type = glbin_current.GetType();
	if (type == 1)
		view->SyncClippingBoxes(view->GetClippingBox());
	else if (glbin_settings.m_clip_link)
		view->SyncClippingBoxes(obj->GetClippingBox());

	int mask = -1;
	fluo::ValueCollection vc;
	switch (i)
	{
	case fluo::ClipPlane::XNeg:
		mask = link ? 3 : 1;
		vc.insert(gstClipX1);
		if (link)
			vc.insert(gstClipX2);
		break;
	case fluo::ClipPlane::XPos:
		mask = link ? 3 : 2;
		vc.insert(gstClipX2);
		if (link)
			vc.insert(gstClipX1);
		break;
	case fluo::ClipPlane::YNeg:
		mask = link ? 12 : 4;
		vc.insert(gstClipY1);
		if (link)
			vc.insert(gstClipY2);
		break;
	case fluo::ClipPlane::YPos:
		mask = link ? 12 : 8;
		vc.insert(gstClipY2);
		if (link)
			vc.insert(gstClipY1);
		break;
	case fluo::ClipPlane::ZNeg:
		mask = link ? 48 : 16;
		vc.insert(gstClipZ1);
		if (link)
			vc.insert(gstClipZ2);
		break;
	case fluo::ClipPlane::ZPos:
		mask = link ? 48 : 32;
		vc.insert(gstClipZ2);
		if (link)
			vc.insert(gstClipZ1);
		break;
	}

	SetPlaneMask(mask);
	vc.insert(gstConvVolMeshUpdateTransf);
	FluoRefresh(0, vc, { glbin_current.GetViewId() });
}

void ClipPlanePanel::SetClipValues(fluo::ClipPlane i, int val1, int val2)
{
	auto view = glbin_current.render_view.lock();
	auto obj = GetObject();
	if (!obj)
		return;
	obj->SetClipValues(i, val1, val2);
	int type = glbin_current.GetType();
	if (type == 1)
		view->SyncClippingBoxes(view->GetClippingBox());
	else if (glbin_settings.m_clip_link)
		view->SyncClippingBoxes(obj->GetClippingBox());

	int mask = -1;
	switch (i)
	{
	case fluo::ClipPlane::XNeg:
	case fluo::ClipPlane::XPos:
		mask = 3;
		break;
	case fluo::ClipPlane::YNeg:
	case fluo::ClipPlane::YPos:
		mask = 12;
		break;
	case fluo::ClipPlane::ZNeg:
	case fluo::ClipPlane::ZPos:
		mask = 48;
		break;
	}
	SetPlaneMask(mask);

	fluo::ValueCollection vc;
	if (mask & 1)
		vc.insert(gstClipX1);
	if (mask & 2)
		vc.insert(gstClipX2);
	if (mask & 4)
		vc.insert(gstClipY1);
	if (mask & 8)
		vc.insert(gstClipY2);
	if (mask & 16)
		vc.insert(gstClipZ1);
	if (mask & 32)
		vc.insert(gstClipZ2);

	vc.insert(gstConvVolMeshUpdateTransf);
	FluoRefresh(0, vc, { glbin_current.GetViewId() });
	double zmin, zmax;
	obj->GetClippingBox().GetClipPairIndex(fluo::ClipPlane::ZNeg, zmin, zmax);
	DBGPRINT(L"Clipping Zmin, Zmax: %f, %f\n", zmin, zmax);
}

void ClipPlanePanel::SetClipValues(const std::array<int, 6>& vals)
{
	auto view = glbin_current.render_view.lock();
	auto obj = GetObject();
	if (!obj)
		return;
	obj->SetClipValues(vals);
	int type = glbin_current.GetType();
	if (type == 1)
		view->SyncClippingBoxes(view->GetClippingBox());
	else if (glbin_settings.m_clip_link)
		view->SyncClippingBoxes(obj->GetClippingBox());
	SetPlaneMask(63);

	FluoRefresh(0,
		{ gstClipX1, gstClipX2, gstClipY1, gstClipY2, gstClipZ1, gstClipZ2, gstConvVolMeshUpdateTransf },
		{ glbin_current.GetViewId() });
}

void ClipPlanePanel::ResetClipValues()
{
	auto view = glbin_current.render_view.lock();
	auto obj = GetObject();
	if (!obj)
		return;
	obj->ResetClipValues();
	int type = glbin_current.GetType();
	if (type == 1)
		view->SyncClippingBoxes(view->GetClippingBox());
	else if (glbin_settings.m_clip_link)
		view->SyncClippingBoxes(obj->GetClippingBox());
	SetPlaneMask(-1);

	//links
	SetXLink(false);
	SetYLink(false);
	SetZLink(false);

	FluoRefresh(0, { gstClipX1, gstClipX2, gstClipY1, gstClipY2, gstClipZ1, gstClipZ2, gstConvVolMeshUpdateTransf },
		{ glbin_current.GetViewId() });
}

void ClipPlanePanel::ResetClipValues(fluo::ClipPlane i)
{
	auto view = glbin_current.render_view.lock();
	auto obj = GetObject();
	if (!obj)
		return;
	obj->ResetClipValues(i);
	int type = glbin_current.GetType();
	if (type == 1)
		view->SyncClippingBoxes(view->GetClippingBox());
	else if (glbin_settings.m_clip_link)
		view->SyncClippingBoxes(obj->GetClippingBox());
	SetPlaneMask(-1);

	fluo::ValueCollection vc = { gstConvVolMeshUpdateTransf };
	//links
	switch (i)
	{
	case fluo::ClipPlane::XNeg:
	case fluo::ClipPlane::XPos:
		SetXLink(false);
		vc.insert({ gstClipX1, gstClipX2 });
		break;
	case fluo::ClipPlane::YNeg:
	case fluo::ClipPlane::YPos:
		SetYLink(false);
		vc.insert({ gstClipY1, gstClipY2 });
		break;
	case fluo::ClipPlane::ZNeg:
	case fluo::ClipPlane::ZPos:
		SetZLink(false);
		vc.insert({ gstClipZ1, gstClipZ2 });
		break;
	}

	FluoRefresh(0, vc, { glbin_current.GetViewId() });
}

void ClipPlanePanel::OnLinkXCheck(wxCommandEvent& event)
{
	bool link = m_linkx_tb->GetToolState(0);
	SetXLink(link);
}

void ClipPlanePanel::OnLinkYCheck(wxCommandEvent& event)
{
	bool link = m_linky_tb->GetToolState(0);
	SetYLink(link);
}

void ClipPlanePanel::OnLinkZCheck(wxCommandEvent& event)
{
	bool link = m_linkz_tb->GetToolState(0);
	SetZLink(link);
}

void ClipPlanePanel::OnClipDistXEdit(wxCommandEvent& event)
{
	auto obj = GetObject();
	if (!obj)
		return;
	wxString str = m_yz_dist_text->GetValue();
	long val;
	if (str.ToLong(&val))
		obj->SetLinkedDist(fluo::ClipPlane::XNeg, val);
}

void ClipPlanePanel::OnClipDistYEdit(wxCommandEvent& event)
{
	auto obj = GetObject();
	if (!obj)
		return;
	wxString str = m_yz_dist_text->GetValue();
	long val;
	if (str.ToLong(&val))
		obj->SetLinkedDist(fluo::ClipPlane::YNeg, val);
}

void ClipPlanePanel::OnClipDistZEdit(wxCommandEvent& event)
{
	auto obj = GetObject();
	if (!obj)
		return;
	wxString str = m_yz_dist_text->GetValue();
	long val;
	if (str.ToLong(&val))
		obj->SetLinkedDist(fluo::ClipPlane::ZNeg, val);
}

void ClipPlanePanel::OnSetZeroBtn(wxCommandEvent& event)
{
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;

	view->SetClipMode(2);
	FluoRefresh(0, { gstClipRotX, gstClipRotY, gstClipRotZ, gstConvVolMeshUpdateTransf },
		{ glbin_current.GetViewId() });
}

void ClipPlanePanel::OnRotResetBtn(wxCommandEvent& event)
{
	auto view = glbin_current.render_view.lock();
	auto obj = GetObject();
	if (!obj)
		return;
	obj->SetClipRotation(fluo::Vector(0));
	int type = glbin_current.GetType();
	if (type == 1)
		view->SyncClippingBoxes(view->GetClippingBox());
	else if (glbin_settings.m_clip_link)
		view->SyncClippingBoxes(obj->GetClippingBox());
	FluoRefresh(0, { gstClipRotX, gstClipRotY, gstClipRotZ, gstConvVolMeshUpdateTransf },
		{ glbin_current.GetViewId() });
}

void ClipPlanePanel::OnRotXMF(wxCommandEvent& event)
{
	switch (glbin_settings.m_mulfunc)
	{
	case 0:
		break;
	case 1:
		m_frame->SetFocusVRenderViews(m_x_rot_sldr);
		break;
	case 2:
	{
		auto view = glbin_current.render_view.lock();
		auto obj = GetObject();
		if (!obj)
			return;
		obj->SetClipRotation(0, 0.0);
		int type = glbin_current.GetType();
		if (type == 1)
			view->SyncClippingBoxes(view->GetClippingBox());
		else if (glbin_settings.m_clip_link)
			view->SyncClippingBoxes(obj->GetClippingBox());
		FluoRefresh(0, { gstClipRotX, gstConvVolMeshUpdateTransf }, { glbin_current.GetViewId() });
	}
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

void ClipPlanePanel::OnRotYMF(wxCommandEvent& event)
{
	switch (glbin_settings.m_mulfunc)
	{
	case 0:
		break;
	case 1:
		m_frame->SetFocusVRenderViews(m_y_rot_sldr);
		break;
	case 2:
	{
		auto view = glbin_current.render_view.lock();
		auto obj = GetObject();
		if (!obj)
			return;
		obj->SetClipRotation(1, 0.0);
		int type = glbin_current.GetType();
		if (type == 1)
			view->SyncClippingBoxes(view->GetClippingBox());
		else if (glbin_settings.m_clip_link)
			view->SyncClippingBoxes(obj->GetClippingBox());
		FluoRefresh(0, { gstClipRotY, gstConvVolMeshUpdateTransf }, { glbin_current.GetViewId() });
	}
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

void ClipPlanePanel::OnRotZMF(wxCommandEvent& event)
{
	switch (glbin_settings.m_mulfunc)
	{
	case 0:
		break;
	case 1:
		m_frame->SetFocusVRenderViews(m_z_rot_sldr);
		break;
	case 2:
	{
		auto view = glbin_current.render_view.lock();
		auto obj = GetObject();
		if (!obj)
			return;
		obj->SetClipRotation(2, 0.0);
		int type = glbin_current.GetType();
		if (type == 1)
			view->SyncClippingBoxes(view->GetClippingBox());
		else if (glbin_settings.m_clip_link)
			view->SyncClippingBoxes(obj->GetClippingBox());
		FluoRefresh(0, { gstClipRotZ, gstConvVolMeshUpdateTransf }, { glbin_current.GetViewId() });
	}
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

void ClipPlanePanel::OnXRotChange(wxScrollEvent& event)
{
	int val = m_x_rot_sldr->GetValue();
	auto view = glbin_current.render_view.lock();
	auto obj = GetObject();
	if (!obj)
		return;
	obj->SetClipRotation(0, val);
	int type = glbin_current.GetType();
	if (type == 1)
		view->SyncClippingBoxes(view->GetClippingBox());
	else if (glbin_settings.m_clip_link)
		view->SyncClippingBoxes(obj->GetClippingBox());
	FluoRefresh(0, { gstClipRotX, gstConvVolMeshUpdateTransf }, { glbin_current.GetViewId() });
}

void ClipPlanePanel::OnXRotEdit(wxCommandEvent& event)
{
	wxString str = m_x_rot_text->GetValue();
	double val = 0.0;
	if (str.ToDouble(&val))
		m_x_rot_sldr->ChangeValue(std::round(val));
	else
		return;
	auto view = glbin_current.render_view.lock();
	auto obj = GetObject();
	if (!obj)
		return;
	obj->SetClipRotation(0, val);
	int type = glbin_current.GetType();
	if (type == 1)
		view->SyncClippingBoxes(view->GetClippingBox());
	else if (glbin_settings.m_clip_link)
		view->SyncClippingBoxes(obj->GetClippingBox());
	FluoRefresh(0, { gstConvVolMeshUpdateTransf }, { glbin_current.GetViewId() });
}

void ClipPlanePanel::OnYRotChange(wxScrollEvent& event)
{
	int val = m_y_rot_sldr->GetValue();
	auto view = glbin_current.render_view.lock();
	auto obj = GetObject();
	if (!obj)
		return;
	obj->SetClipRotation(1, val);
	int type = glbin_current.GetType();
	if (type == 1)
		view->SyncClippingBoxes(view->GetClippingBox());
	else if (glbin_settings.m_clip_link)
		view->SyncClippingBoxes(obj->GetClippingBox());
	FluoRefresh(0, { gstClipRotY, gstConvVolMeshUpdateTransf }, { glbin_current.GetViewId() });
}

void ClipPlanePanel::OnYRotEdit(wxCommandEvent& event)
{
	wxString str = m_y_rot_text->GetValue();
	double val = 0.0;
	if (str.ToDouble(&val))
		m_y_rot_sldr->ChangeValue(std::round(val));
	else
		return;
	auto view = glbin_current.render_view.lock();
	auto obj = GetObject();
	if (!obj)
		return;
	obj->SetClipRotation(1, val);
	int type = glbin_current.GetType();
	if (type == 1)
		view->SyncClippingBoxes(view->GetClippingBox());
	else if (glbin_settings.m_clip_link)
		view->SyncClippingBoxes(obj->GetClippingBox());
	FluoRefresh(0, { gstConvVolMeshUpdateTransf }, { glbin_current.GetViewId() });
}

void ClipPlanePanel::OnZRotChange(wxScrollEvent& event)
{
	int val = m_z_rot_sldr->GetValue();
	auto view = glbin_current.render_view.lock();
	auto obj = GetObject();
	if (!obj)
		return;
	obj->SetClipRotation(2, val);
	int type = glbin_current.GetType();
	if (type == 1)
		view->SyncClippingBoxes(view->GetClippingBox());
	else if (glbin_settings.m_clip_link)
		view->SyncClippingBoxes(obj->GetClippingBox());
	FluoRefresh(0, { gstClipRotZ, gstConvVolMeshUpdateTransf }, { glbin_current.GetViewId() });
}

void ClipPlanePanel::OnZRotEdit(wxCommandEvent& event)
{
	wxString str = m_z_rot_text->GetValue();
	double val = 0.0;
	if (str.ToDouble(&val))
		m_z_rot_sldr->ChangeValue(std::round(val));
	else
		return;
	auto view = glbin_current.render_view.lock();
	auto obj = GetObject();
	if (!obj)
		return;
	obj->SetClipRotation(2, val);
	int type = glbin_current.GetType();
	if (type == 1)
		view->SyncClippingBoxes(view->GetClippingBox());
	else if (glbin_settings.m_clip_link)
		view->SyncClippingBoxes(obj->GetClippingBox());
	FluoRefresh(0, { gstNull, gstConvVolMeshUpdateTransf }, { glbin_current.GetViewId() });
}

void ClipPlanePanel::OnXRotSpinUp(wxSpinEvent& event)
{
	wxString str_val = m_x_rot_text->GetValue();
	double val;
	str_val.ToDouble(&val);
	val += glbin_settings.m_inverse_slider ? -1 : 1;
	if (val > 180.0) val -= 360.0;
	if (val <-180.0) val += 360.0;
	auto view = glbin_current.render_view.lock();
	auto obj = GetObject();
	if (!obj)
		return;
	obj->SetClipRotation(0, val);
	int type = glbin_current.GetType();
	if (type == 1)
		view->SyncClippingBoxes(view->GetClippingBox());
	else if (glbin_settings.m_clip_link)
		view->SyncClippingBoxes(obj->GetClippingBox());
	FluoRefresh(0, { gstClipRotX, gstConvVolMeshUpdateTransf }, { glbin_current.GetViewId() });
}

void ClipPlanePanel::OnXRotSpinDown(wxSpinEvent& event)
{
	wxString str_val = m_x_rot_text->GetValue();
	double val;
	str_val.ToDouble(&val);
	val += glbin_settings.m_inverse_slider ? 1 : -1;
	if (val > 180.0) val -= 360.0;
	if (val <-180.0) val += 360.0;
	auto view = glbin_current.render_view.lock();
	auto obj = GetObject();
	if (!obj)
		return;
	obj->SetClipRotation(0, val);
	int type = glbin_current.GetType();
	if (type == 1)
		view->SyncClippingBoxes(view->GetClippingBox());
	else if (glbin_settings.m_clip_link)
		view->SyncClippingBoxes(obj->GetClippingBox());
	FluoRefresh(0, { gstClipRotX, gstConvVolMeshUpdateTransf }, { glbin_current.GetViewId() });
}

void ClipPlanePanel::OnYRotSpinUp(wxSpinEvent& event)
{
	wxString str_val = m_y_rot_text->GetValue();
	double val;
	str_val.ToDouble(&val);
	val += glbin_settings.m_inverse_slider ? -1 : 1;
	if (val > 180.0) val -= 360.0;
	if (val <-180.0) val += 360.0;
	auto view = glbin_current.render_view.lock();
	auto obj = GetObject();
	if (!obj)
		return;
	obj->SetClipRotation(1, val);
	int type = glbin_current.GetType();
	if (type == 1)
		view->SyncClippingBoxes(view->GetClippingBox());
	else if (glbin_settings.m_clip_link)
		view->SyncClippingBoxes(obj->GetClippingBox());
	FluoRefresh(0, { gstClipRotY, gstConvVolMeshUpdateTransf }, { glbin_current.GetViewId() });
}

void ClipPlanePanel::OnYRotSpinDown(wxSpinEvent& event)
{
	wxString str_val = m_y_rot_text->GetValue();
	double val;
	str_val.ToDouble(&val);
	val += glbin_settings.m_inverse_slider ? 1 : -1;
	if (val > 180.0) val -= 360.0;
	if (val <-180.0) val += 360.0;
	auto view = glbin_current.render_view.lock();
	auto obj = GetObject();
	if (!obj)
		return;
	obj->SetClipRotation(1, val);
	int type = glbin_current.GetType();
	if (type == 1)
		view->SyncClippingBoxes(view->GetClippingBox());
	else if (glbin_settings.m_clip_link)
		view->SyncClippingBoxes(obj->GetClippingBox());
	FluoRefresh(0, { gstClipRotY, gstConvVolMeshUpdateTransf }, { glbin_current.GetViewId() });
}

void ClipPlanePanel::OnZRotSpinUp(wxSpinEvent& event)
{
	wxString str_val = m_z_rot_text->GetValue();
	double val;
	str_val.ToDouble(&val);
	val += glbin_settings.m_inverse_slider ? -1 : 1;
	if (val > 180.0) val -= 360.0;
	if (val <-180.0) val += 360.0;
	auto view = glbin_current.render_view.lock();
	auto obj = GetObject();
	if (!obj)
		return;
	obj->SetClipRotation(2, val);
	int type = glbin_current.GetType();
	if (type == 1)
		view->SyncClippingBoxes(view->GetClippingBox());
	else if (glbin_settings.m_clip_link)
		view->SyncClippingBoxes(obj->GetClippingBox());
	FluoRefresh(0, { gstClipRotZ, gstConvVolMeshUpdateTransf }, { glbin_current.GetViewId() });
}

void ClipPlanePanel::OnZRotSpinDown(wxSpinEvent& event)
{
	wxString str_val = m_z_rot_text->GetValue();
	double val;
	str_val.ToDouble(&val);
	val += glbin_settings.m_inverse_slider ? 1 : -1;
	if (val > 180.0) val -= 360.0;
	if (val <-180.0) val += 360.0;
	auto view = glbin_current.render_view.lock();
	auto obj = GetObject();
	if (!obj)
		return;
	obj->SetClipRotation(2, val);
	int type = glbin_current.GetType();
	if (type == 1)
		view->SyncClippingBoxes(view->GetClippingBox());
	else if (glbin_settings.m_clip_link)
		view->SyncClippingBoxes(obj->GetClippingBox());
	FluoRefresh(0, { gstClipRotZ, gstConvVolMeshUpdateTransf }, { glbin_current.GetViewId() });
}

void ClipPlanePanel::UpdateSampleRate()
{
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;

	//good rate
	if (vd->GetSampleRate() < 2.0)
		vd->SetSampleRate(2.0);
	if (glbin_settings.m_clip_link)
	{
		int i;
		for (i = 0; i < glbin_data_manager.GetVolumeNum(); i++)
		{
			auto vd = glbin_data_manager.GetVolumeData(i);
			if (!vd || vd == vd)
				continue;
			if (vd->GetSampleRate() < 2.0)
				vd->SetSampleRate(2.0);
		}
	}
}

void ClipPlanePanel::OnClipXRClick(wxMouseEvent& event)
{
	int clip = m_clipx_sldr->GetLowValue();
	auto view = glbin_current.render_view.lock();
	auto obj = GetObject();
	if (!obj)
		return;

	obj->ResetClipValues();
	SetXLink(true);
	SetYLink(false);
	SetZLink(false);

	SetClipValues(fluo::ClipPlane::XNeg, clip, clip + 1);
}

void ClipPlanePanel::OnClipYRClick(wxMouseEvent& event)
{
	int clip = m_clipy_sldr->GetLowValue();
	auto view = glbin_current.render_view.lock();
	auto obj = GetObject();
	if (!obj)
		return;

	obj->ResetClipValues();
	SetXLink(false);
	SetYLink(true);
	SetZLink(false);

	SetClipValues(fluo::ClipPlane::YNeg, clip, clip + 1);
}

void ClipPlanePanel::OnClipZRClick(wxMouseEvent& event)
{
	int clip = m_clipz_sldr->GetLowValue();
	auto view = glbin_current.render_view.lock();
	auto obj = GetObject();
	if (!obj)
		return;

	obj->ResetClipValues();
	SetXLink(false);
	SetYLink(false);
	SetZLink(true);

	SetClipValues(fluo::ClipPlane::ZNeg, clip, clip + 1);
}

void ClipPlanePanel::OnYZClipBtn(wxCommandEvent& event)
{
	auto view = glbin_current.render_view.lock();
	auto obj = GetObject();
	if (!obj)
		return;

	int dist = obj->GetLinkedDist(fluo::ClipPlane::XNeg);
	obj->ResetClipValues();
	//reset yz
	SetYLink(false);
	SetZLink(false);
	//set x
	SetXLink(true);

	SetClipValues(fluo::ClipPlane::XNeg, 0, dist);
}

void ClipPlanePanel::OnXZClipBtn(wxCommandEvent& event)
{
	auto view = glbin_current.render_view.lock();
	auto obj = GetObject();
	if (!obj)
		return;

	int dist = obj->GetLinkedDist(fluo::ClipPlane::YNeg);
	obj->ResetClipValues();
	//reset xz
	SetXLink(false);
	SetZLink(false);
	//set y
	SetYLink(true);

	SetClipValues(fluo::ClipPlane::XNeg, 0, dist);
}

void ClipPlanePanel::OnXYClipBtn(wxCommandEvent& event)
{
	auto view = glbin_current.render_view.lock();
	auto obj = GetObject();
	if (!obj)
		return;

	int dist = obj->GetLinkedDist(fluo::ClipPlane::ZNeg);
	obj->ResetClipValues();
	//reset xy
	SetXLink(false);
	SetYLink(false);
	//set z
	SetZLink(true);

	SetClipValues(fluo::ClipPlane::ZNeg, 0, dist);
}

void ClipPlanePanel::SetPlaneMask(int val)
{
	auto base = glbin_renderer_factory.getOrCreate(gstClippingBoxRenderer);
	auto renderer = std::dynamic_pointer_cast<flrd::ClippingBoxRenderer>(base);
	if (renderer)
	{
		auto settings = std::dynamic_pointer_cast<flrd::ClippingBoxSettings>(renderer->getSettings());
		settings->plane_mask = val;
	}
}

void ClipPlanePanel::ClearUndo()
{
	m_clipx_sldr->Clear();
	m_clipy_sldr->Clear();
	m_clipz_sldr->Clear();
	m_x_rot_sldr->Clear();
	m_y_rot_sldr->Clear();
	m_z_rot_sldr->Clear();
}

void ClipPlanePanel::EnableAll(bool val)
{
	if (m_enable_all == val)
		return;
	m_enable_all = val;

	m_toolbar->Enable(val);
	m_set_zero_btn->Enable(val);
	m_rot_reset_btn->Enable(val);
	m_clip_x_st->Enable(val);
	m_clip_y_st->Enable(val);
	m_clip_z_st->Enable(val);
	m_rot_x_st->Enable(val);
	m_rot_y_st->Enable(val);
	m_rot_z_st->Enable(val);
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

std::shared_ptr<TreeLayer> ClipPlanePanel::GetObject()
{
	int type = glbin_current.GetType();
	if (type != 1 && type != 2 && type != 3)
	{
		return nullptr;
	}
	auto vd = glbin_current.vol_data.lock();
	auto md = glbin_current.mesh_data.lock();
	auto view = glbin_current.render_view.lock();

	switch (type)
	{
	case 1:	//render view
		return view;
	case 2:	//volume
		return vd;
	case 3:	//mesh
		return md;
	}

	return nullptr;
}