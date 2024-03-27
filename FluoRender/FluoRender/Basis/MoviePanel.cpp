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
#include <MoviePanel.h>
#include <Global/Global.h>
#include <RecorderDlg.h>
#include <MainFrame.h>
#include <RenderCanvas.h>
#include <RenderViewPanel.h>
#include <wxUndoableScrollBar.h>
#include <tiffio.h>
#include <wx/aboutdlg.h>
#include <wx/valnum.h>
#include <wx/stdpaths.h>
#include <png_resource.h>
#include <img/icons.h>
#include <Debug.h>

wxWindow* MoviePanel::CreateSimplePage(wxWindow *parent)
{
	wxScrolledWindow *page = new wxScrolledWindow(parent);

	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;

	wxStaticText *st = 0, *st2 = 0;
	//sizers

	//rotations
	wxBoxSizer* sizer1 = new wxBoxSizer(wxHORIZONTAL);
	m_rot_chk = new wxCheckBox(page, wxID_ANY, "Rotation");
	m_rot_chk->Bind(wxEVT_CHECKBOX, &MoviePanel::OnRotateChecked, this);
	sizer1->Add(10, 10);
	sizer1->Add(m_rot_chk, 0, wxALIGN_CENTER);

	//axis
	wxBoxSizer* sizer2 = new wxBoxSizer(wxHORIZONTAL);
	m_x_rd = new wxRadioButton(page, wxID_ANY, "X",
		wxDefaultPosition, FromDIP(wxSize(30, 22)));
	m_y_rd = new wxRadioButton(page, wxID_ANY, "Y",
		wxDefaultPosition, FromDIP(wxSize(30, 22)));
	m_z_rd = new wxRadioButton(page, wxID_ANY, "Z",
		wxDefaultPosition, FromDIP(wxSize(30, 22)));
	m_x_rd->Bind(wxEVT_RADIOBUTTON, &MoviePanel::OnRotAxis, this);
	m_y_rd->Bind(wxEVT_RADIOBUTTON, &MoviePanel::OnRotAxis, this);
	m_z_rd->Bind(wxEVT_RADIOBUTTON, &MoviePanel::OnRotAxis, this);
	sizer2->Add(20, 5);
	sizer2->Add(m_x_rd, 0, wxALIGN_CENTER);
	sizer2->Add(20, 5);
	sizer2->Add(m_y_rd, 0, wxALIGN_CENTER);
	sizer2->Add(20, 5);
	sizer2->Add(m_z_rd, 0, wxALIGN_CENTER);

	//degrees
	wxBoxSizer* sizer3 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, wxID_ANY, "Rotation Angles:");
	m_degree_text = new wxTextCtrl(page, wxID_ANY, "360",
		wxDefaultPosition, FromDIP(wxSize(40, -1)));
	m_degree_text->Bind(wxEVT_TEXT, &MoviePanel::OnDegreeText, this);
	st2 = new wxStaticText(page, wxID_ANY, "Deg.");
	sizer3->Add(20, 5, 0);
	sizer3->Add(st, 0, wxALIGN_CENTER);
	sizer3->Add(20, 5, 0);
	sizer3->Add(m_degree_text, 0, wxALIGN_CENTER);
	sizer3->Add(20, 5, 0);
	sizer3->Add(st2, 0, wxALIGN_CENTER);

	//rotation interpolation
	wxBoxSizer* sizer4 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, wxID_ANY, "Interpolation Method:");
	m_rot_int_cmb = new wxComboBox(page, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(65, -1)), 0, NULL, wxCB_READONLY);
	std::vector<wxString> list = { "Linear", "Smooth" };
	m_rot_int_cmb->Append(list);
	m_rot_int_cmb->Select(0);
	m_rot_int_cmb->Bind(wxEVT_COMBOBOX, &MoviePanel::OnRotIntCmb, this);
	sizer4->Add(20, 5, 0);
	sizer4->Add(st, 0, wxALIGN_CENTER);
	sizer4->Add(20, 5, 0);
	sizer4->Add(m_rot_int_cmb, 0, wxALIGN_CENTER);

	//type
	wxBoxSizer* sizer5 = new wxBoxSizer(wxHORIZONTAL);
	m_seq_chk = new wxCheckBox(page, wxID_ANY, "Time Sequence");
	m_seq_chk->Bind(wxEVT_CHECKBOX, &MoviePanel::OnSequenceChecked, this);
	sizer5->Add(10, 10, 0);
	sizer5->Add(m_seq_chk, 0, wxALIGN_CENTER);

	wxBoxSizer* sizer6 = new wxBoxSizer(wxHORIZONTAL);
	m_bat_chk = new wxCheckBox(page, wxID_ANY, "Batch Process");
	m_bat_chk->Bind(wxEVT_CHECKBOX, &MoviePanel::OnBatchChecked, this);
	sizer6->Add(10, 10, 0);
	sizer6->Add(m_bat_chk, 0, wxALIGN_CENTER);

	//vertical sizer
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
	sizer_v->Add(5, 5, 0);
	sizer_v->Add(sizer1, 0, wxEXPAND);
	sizer_v->Add(5, 5, 0);
	sizer_v->Add(sizer2, 0, wxEXPAND);
	sizer_v->Add(5, 5, 0);
	sizer_v->Add(sizer3, 0, wxEXPAND);
	sizer_v->Add(5, 5, 0);
	sizer_v->Add(sizer4, 0, wxEXPAND);
	sizer_v->Add(5, 20, 0);
	sizer_v->Add(sizer5, 0, wxEXPAND);
	sizer_v->Add(5, 20, 0);
	sizer_v->Add(sizer6, 0, wxEXPAND);
	//set the page
	page->SetSizer(sizer_v);
	page->SetAutoLayout(true);
	page->SetScrollRate(10, 10);
	return page;
}

wxWindow* MoviePanel::CreateAdvancedPage(wxWindow *parent)
{
	wxScrolledWindow* page = new wxScrolledWindow(parent);

	m_keyframe_chk = new wxCheckBox(page, wxID_ANY, "Enable keyframe movie");
	m_keyframe_chk->Bind(wxEVT_CHECKBOX, &MoviePanel::OnKeyframeChk, this);
	m_advanced_movie = new RecorderDlg(m_frame, page);
	m_frame->SetRecorderDlg(m_advanced_movie);

	//vertical sizer
	wxBoxSizer* sizerv = new wxBoxSizer(wxVERTICAL);
	sizerv->Add(5, 5, 0);
	sizerv->Add(m_keyframe_chk, 0, wxALIGN_CENTER);
	sizerv->Add(m_advanced_movie, 1, wxEXPAND);
	//set the page
	page->SetSizer(sizerv);
	page->SetAutoLayout(true);
	page->SetScrollRate(10, 10);
	return page;
}

wxWindow* MoviePanel::CreateAutoKeyPage(wxWindow *parent)
{
	wxScrolledWindow* page = new wxScrolledWindow(parent);

	wxStaticText * st = new wxStaticText(page, 0, "Choose an auto key type");

	//list of options
	m_auto_key_list = new wxListCtrl(page, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
	wxListItem itemCol;
	itemCol.SetText("ID");
	m_auto_key_list->InsertColumn(0, itemCol);
	itemCol.SetText("Auto Key Type");
	m_auto_key_list->InsertColumn(1, itemCol);
	//options
	//channel comb 1
	long tmp;
	std::vector<std::string> str_list = glbin_moviemaker.GetAutoKeyTypes();
	int i = 0;
	wxString str;
	for (auto& it : str_list)
	{
		str = wxString::Format("%d", i + 1);
		tmp = m_auto_key_list->InsertItem(i, str, 0);
		m_auto_key_list->SetItem(tmp, 1, it);
	}
	m_auto_key_list->SetColumnWidth(1, -1);
	m_auto_key_list->Bind(wxEVT_LIST_ITEM_ACTIVATED, &MoviePanel::OnGenKey, this);

	//button
	m_gen_keys_btn = new wxButton(page, wxID_ANY, "Generate");
	m_gen_keys_btn->Bind(wxEVT_BUTTON, &MoviePanel::OnGenKey, this);

	//vertical sizer
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
	sizer_v->Add(10, 10, 0);
	sizer_v->Add(st, 0, wxALIGN_LEFT);
	sizer_v->Add(10, 10, 0);
	sizer_v->Add(m_auto_key_list, 1, wxEXPAND);
	sizer_v->Add(10, 10, 0);
	sizer_v->Add(m_gen_keys_btn, 0, wxALIGN_RIGHT);
	sizer_v->Add(10, 10, 0);

	page->SetSizer(sizer_v);
	page->SetAutoLayout(true);
	page->SetScrollRate(10, 10);
	return page;

}

wxWindow* MoviePanel::CreateCroppingPage(wxWindow *parent)
{
	wxScrolledWindow* page = new wxScrolledWindow(parent);

	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;

	wxStaticText *st = 0;
	//sizers
	wxBoxSizer* sizer_8 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizer_9 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizer_10 = new wxBoxSizer(wxHORIZONTAL);
	//vertical sizer
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
	//8th line
	st = new wxStaticText(page, 0, "Enable cropping:",
		wxDefaultPosition, FromDIP(wxSize(110, 20)));
	m_crop_chk = new wxCheckBox(page, wxID_ANY, "");
	m_crop_chk->Bind(wxEVT_CHECKBOX, &MoviePanel::OnCropCheck, this);
	m_reset_btn = new wxButton(page, wxID_ANY, "Reset",
		wxDefaultPosition, FromDIP(wxSize(110, 30)));
	m_reset_btn->Bind(wxEVT_BUTTON, &MoviePanel::OnResetCrop, this);
	m_reset_btn->SetBitmap(wxGetBitmapFromMemory(reset));
	sizer_8->Add(5, 5, 0);
	sizer_8->Add(st, 0, wxALIGN_CENTER);
	sizer_8->Add(m_crop_chk, 0, wxALIGN_CENTER);
	sizer_8->Add(100, 5, 0);
	sizer_8->Add(m_reset_btn, 0, wxALIGN_CENTER);
	//9th line
	st = new wxStaticText(page, 0, "Center:  X:",
		wxDefaultPosition, FromDIP(wxSize(85, 20)));
	m_center_x_text = new wxTextCtrl(page, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), 0, vald_int);
	m_center_x_text->Bind(wxEVT_TEXT, &MoviePanel::OnEditCrop, this);
	m_center_x_spin = new wxSpinButton(page, wxID_ANY,
		wxDefaultPosition, FromDIP(wxSize(20, 20)));
	m_center_x_spin->SetRange(-0x8000, 0x7fff);
	m_center_x_spin->Bind(wxEVT_SPIN_UP, &MoviePanel::OnCropSpinUp, this);
	m_center_x_spin->Bind(wxEVT_SPIN_DOWN, &MoviePanel::OnCropSpinDown, this);
	sizer_9->Add(5, 5, 0);
	sizer_9->Add(st, 0, wxALIGN_CENTER);
	sizer_9->Add(m_center_x_text, 0, wxALIGN_CENTER);
	sizer_9->Add(m_center_x_spin, 0, wxALIGN_CENTER);
	st = new wxStaticText(page, 0, "       Y:",
		wxDefaultPosition, FromDIP(wxSize(50, 20)));
	m_center_y_text = new wxTextCtrl(page, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), 0, vald_int);
	m_center_y_text->Bind(wxEVT_TEXT, &MoviePanel::OnEditCrop, this);
	m_center_y_spin = new wxSpinButton(page, wxID_ANY,
		wxDefaultPosition, FromDIP(wxSize(20, 20)));
	m_center_y_spin->SetRange(-0x8000, 0x7fff);
	m_center_y_spin->Bind(wxEVT_SPIN_UP, &MoviePanel::OnCropSpinUp, this);
	m_center_y_spin->Bind(wxEVT_SPIN_DOWN, &MoviePanel::OnCropSpinDown, this);
	sizer_9->Add(st, 0, wxALIGN_CENTER);
	sizer_9->Add(m_center_y_text, 0, wxALIGN_CENTER);
	sizer_9->Add(m_center_y_spin, 0, wxALIGN_CENTER);
	//10th line
	st = new wxStaticText(page, 0, "Size:    Width:",
		wxDefaultPosition, FromDIP(wxSize(85, 20)));
	m_width_text = new wxTextCtrl(page, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), 0, vald_int);
	m_width_text->Bind(wxEVT_TEXT, &MoviePanel::OnEditCrop, this);
	m_width_spin = new wxSpinButton(page, wxID_ANY,
		wxDefaultPosition, FromDIP(wxSize(20, 20)));
	m_width_spin->SetRange(-0x8000, 0x7fff);
	m_width_spin->Bind(wxEVT_SPIN_UP, &MoviePanel::OnCropSpinUp, this);
	m_width_spin->Bind(wxEVT_SPIN_DOWN, &MoviePanel::OnCropSpinDown, this);
	sizer_10->Add(5, 5, 0);
	sizer_10->Add(st, 0, wxALIGN_CENTER);
	sizer_10->Add(m_width_text, 0, wxALIGN_CENTER);
	sizer_10->Add(m_width_spin, 0, wxALIGN_CENTER);
	st = new wxStaticText(page, 0, "   Height:",
		wxDefaultPosition, FromDIP(wxSize(50, 20)));
	m_height_text = new wxTextCtrl(page, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), 0, vald_int);
	m_height_text->Bind(wxEVT_TEXT, &MoviePanel::OnEditCrop, this);
	m_height_spin = new wxSpinButton(page, wxID_ANY,
		wxDefaultPosition, FromDIP(wxSize(20, 20)));
	m_height_spin->SetRange(-0x8000, 0x7fff);
	m_height_spin->Bind(wxEVT_SPIN_UP, &MoviePanel::OnCropSpinUp, this);
	m_height_spin->Bind(wxEVT_SPIN_DOWN, &MoviePanel::OnCropSpinDown, this);
	sizer_10->Add(st, 0, wxALIGN_CENTER);
	sizer_10->Add(m_height_text, 0, wxALIGN_CENTER);
	sizer_10->Add(m_height_spin, 0, wxALIGN_CENTER);
	// add vertical lines
	sizer_v->Add(10, 10, 0);
	sizer_v->Add(sizer_8, 0, wxEXPAND);
	sizer_v->Add(10, 10, 0);
	sizer_v->Add(sizer_9, 0, wxEXPAND);
	sizer_v->Add(10, 10, 0);
	sizer_v->Add(sizer_10, 0, wxEXPAND);
	sizer_v->AddStretchSpacer();

	page->SetSizer(sizer_v);
	page->SetAutoLayout(true);
	page->SetScrollRate(10, 10);
	return page;

}

wxWindow* MoviePanel::CreateScriptPage(wxWindow *parent)
{
	wxScrolledWindow* page = new wxScrolledWindow(parent);
	//script
	//vertical sizer
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
	m_run_script_chk = new wxCheckBox(page, wxID_ANY,
		"Enable execution of a script during playback.");
	m_run_script_chk->Bind(wxEVT_CHECKBOX, &MoviePanel::OnRunScriptChk, this);
	wxStaticText* st;
	sizer_v->Add(10, 10);
	sizer_v->Add(m_run_script_chk);
	sizer_v->Add(5, 5);

	//browse button
	wxBoxSizer *sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Script File:",
		wxDefaultPosition, FromDIP(wxSize(80, -1)));
	m_script_file_btn = new wxButton(page, wxID_ANY, "Browse...",
		wxDefaultPosition, FromDIP(wxSize(80, -1)));
	m_script_file_btn->Bind(wxEVT_BUTTON, &MoviePanel::OnScriptFileBtn, this);
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	sizer_1->AddStretchSpacer(1);
	sizer_1->Add(m_script_file_btn, 0, wxALIGN_CENTER);

	//file name
	wxBoxSizer *sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	m_script_file_text = new wxTextCtrl(page, wxID_ANY, "",
		wxDefaultPosition, wxDefaultSize);
	m_script_clear_btn = new wxButton(page, wxID_ANY, "X",
		wxDefaultPosition, FromDIP(wxSize(25, -1)));
	m_script_file_text->Bind(wxEVT_TEXT, &MoviePanel::OnScriptFileEdit, this);
	m_script_clear_btn->Bind(wxEVT_BUTTON, &MoviePanel::OnScriptClearBtn, this);
	sizer_2->Add(m_script_file_text, 1, wxEXPAND);
	sizer_2->Add(m_script_clear_btn, 0, wxALIGN_CENTER);

	//script list
	m_script_list = new wxListCtrl(page, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
	wxListItem itemCol;
	itemCol.SetText("Built-in Scripte Files");
	m_script_list->InsertColumn(0, itemCol);
	m_script_list->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
	m_script_list->Bind(wxEVT_LIST_ITEM_SELECTED, &MoviePanel::OnScriptListSelected, this);
	m_script_list->Bind(wxEVT_LIST_ITEM_ACTIVATED, &MoviePanel::OnScriptListSelected, this);

	sizer_v->Add(5, 5);
	sizer_v->Add(sizer_1, 0, wxEXPAND);
	sizer_v->Add(5, 5);
	sizer_v->Add(sizer_2, 0, wxEXPAND);
	sizer_v->Add(5, 5);
	sizer_v->Add(m_script_list, 1, wxEXPAND);

	//set the page
	page->SetSizer(sizer_v);
	page->SetAutoLayout(true);
	page->SetScrollRate(10, 10);
	return page;
}

MoviePanel::MoviePanel(MainFrame* frame,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name) :
	PropPanel(frame, frame, pos, size, style, name)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);
	Freeze();
	//SetDoubleBuffered(true);

	if (m_frame)
		m_view = m_frame->GetView(glbin_mov_def.m_view_idx);

	//notebook
	m_notebook = new wxAuiNotebook(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize,
		wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE |
		wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_TAB_EXTERNAL_MOVE |
		wxAUI_NB_WINDOWLIST_BUTTON | wxNO_BORDER);
	m_notebook->AddPage(CreateSimplePage(m_notebook), "Basic", true);
	m_notebook->AddPage(CreateAdvancedPage(m_notebook), "Advanced");
	m_notebook->AddPage(CreateAutoKeyPage(m_notebook), "Auto Key");
	m_notebook->AddPage(CreateCroppingPage(m_notebook), "Cropping");
	m_notebook->AddPage(CreateScriptPage(m_notebook), "Script");
	m_notebook->Bind(wxEVT_AUINOTEBOOK_PAGE_CHANGED, &MoviePanel::OnNotebookPage, this);

	wxStaticText* st = 0, *st2 = 0;
	//common settings
	wxBoxSizer* sizer1 = new wxBoxSizer(wxHORIZONTAL);
	//FPS
	st = new wxStaticText(this, wxID_ANY, "FPS: ");
	m_fps_text = new wxTextCtrl(this, wxID_ANY, "30",
		wxDefaultPosition, FromDIP(wxSize(30, -1)));
	m_fps_text->Bind(wxEVT_TEXT, &MoviePanel::OnFpsEdit, this);
	sizer1->Add(5, 5);
	sizer1->Add(st, 0, wxALIGN_CENTER);
	sizer1->Add(m_fps_text, 0, wxALIGN_CENTER);
	//movie length
	st = new wxStaticText(this, wxID_ANY, "Length: ");
	st2 = new wxStaticText(this, wxID_ANY, "Sec.");
	m_movie_len_text = new wxTextCtrl(this, wxID_ANY, "5.00",
		wxDefaultPosition, FromDIP(wxSize(50, -1)));
	m_movie_len_text->Bind(wxEVT_TEXT, &MoviePanel::OnMovieLenText, this);
	sizer1->Add(5, 5);
	sizer1->Add(st, 0, wxALIGN_CENTER);
	sizer1->Add(5, 5);
	sizer1->Add(m_movie_len_text, 0, wxALIGN_CENTER);
	sizer1->Add(st2, 0, wxALIGN_CENTER);
	//view
	st = new wxStaticText(this, wxID_ANY, "Capture: ");
	m_views_cmb = new wxComboBox(this, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(100, -1)), 0, NULL, wxCB_READONLY);
	m_views_cmb->Bind(wxEVT_COMBOBOX, &MoviePanel::OnViewSelected, this);
	sizer1->AddStretchSpacer();
	sizer1->Add(st, 0, wxALIGN_CENTER);
	sizer1->Add(m_views_cmb, 0, wxALIGN_CENTER);
	sizer1->Add(5, 5);

	//slider
	wxBoxSizer* sizer2 = new wxBoxSizer(wxHORIZONTAL);
	m_slider_btn = new wxToolBar(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
	m_slider_btn->SetDoubleBuffered(true);
	m_slider_btn->AddCheckTool(0, "Slider style",
		wxGetBitmapFromMemory(slider_type_pos), wxNullBitmap,
		"Choose slider style between jog and normal",
		"Choose slider style between jog and normal");
	m_slider_btn->Bind(wxEVT_TOOL, &MoviePanel::OnSliderStyle, this);
	m_slider_btn->Realize();
	m_progress_sldr = new wxUndoableScrollBar(this, wxID_ANY,
		wxDefaultPosition, FromDIP(wxSize(-1, 20)));
	m_progress_sldr->SetScrollbar(
		glbin_moviemaker.GetCurrentFrame(), 40,
		glbin_moviemaker.GetFrameNum() + 40, 1);
	m_progress_sldr->Bind(wxEVT_SCROLL_CHANGED, &MoviePanel::OnProgressScroll, this);
	sizer2->Add(5, 5);
	sizer2->Add(m_slider_btn, 0, wxALIGN_CENTER);
	sizer2->Add(5, 5);
	sizer2->Add(m_progress_sldr, 1, wxALIGN_CENTER);
	sizer2->Add(5, 5);

	//controls
	wxSize bs = FromDIP(wxSize(26, 26));
	wxFont f;
	wxBoxSizer* sizer3 = new wxBoxSizer(wxHORIZONTAL);
	m_start_btn = new wxButton(this, wxID_ANY, "",
		wxDefaultPosition, bs);
	m_start_btn->SetBitmap(wxGetBitmapFromMemory(start));
	m_start_btn->Bind(wxEVT_BUTTON, &MoviePanel::OnStartFrameBtn, this);
	m_start_frame_text = new wxTextCtrl(this, wxID_ANY, "1",
		wxDefaultPosition, FromDIP(wxSize(39, -1)), wxTE_RIGHT);
	f = m_start_frame_text->GetFont();
	f.MakeLarger();
	m_start_frame_text->SetFont(f);
	m_start_frame_text->Bind(wxEVT_TEXT, &MoviePanel::OnStartFrameText, this);
	m_end_frame_text = new wxTextCtrl(this, wxID_ANY, "10",
		wxDefaultPosition, FromDIP(wxSize(39, -1)), wxTE_RIGHT);
	m_end_frame_text->SetFont(f);
	m_end_frame_text->Bind(wxEVT_TEXT, &MoviePanel::OnEndFrameText, this);
	m_end_btn = new wxButton(this, wxID_ANY, "",
		wxDefaultPosition, bs);
	m_end_btn->SetBitmap(wxGetBitmapFromMemory(end));
	m_end_btn->Bind(wxEVT_BUTTON, &MoviePanel::OnEndFrameBtn, this);
	m_dec_time_btn = new wxButton(this, wxID_ANY, "",
		wxDefaultPosition, bs);
	m_dec_time_btn->SetBitmap(wxGetBitmapFromMemory(minus));
	m_dec_time_btn->Bind(wxEVT_BUTTON, &MoviePanel::OnDecFrame, this);
	m_cur_frame_text = new wxTextCtrl(this, wxID_ANY, "0",
		wxDefaultPosition, FromDIP(wxSize(50, -1)), wxTE_RIGHT);
	m_cur_frame_text->SetFont(f);
	m_cur_frame_text->Bind(wxEVT_TEXT, &MoviePanel::OnCurFrameText, this);
	m_inc_time_btn = new wxButton(this, wxID_ANY, "",
		wxDefaultPosition, bs);
	m_inc_time_btn->SetBitmap(wxGetBitmapFromMemory(plus));
	m_inc_time_btn->Bind(wxEVT_BUTTON, &MoviePanel::OnIncFrame, this);
	sizer3->AddStretchSpacer(2);
	sizer3->Add(m_start_btn, 0, wxALIGN_CENTER);
	sizer3->Add(m_start_frame_text, 0, wxALIGN_CENTER);
	sizer3->Add(m_end_frame_text, 0, wxALIGN_CENTER);
	sizer3->Add(m_end_btn, 0, wxALIGN_CENTER);
	sizer3->AddStretchSpacer(1);
	sizer3->Add(m_dec_time_btn, 0, wxALIGN_CENTER);
	sizer3->Add(m_cur_frame_text, 0, wxALIGN_CENTER);
	sizer3->Add(m_inc_time_btn, 0, wxALIGN_CENTER);
	sizer3->AddStretchSpacer(2);
	sizer3->Add(bs.x, bs.y);
	sizer3->Add(5, 5);

	wxBoxSizer* sizer4 = new wxBoxSizer(wxHORIZONTAL);
	m_rewind_btn = new wxButton(this, wxID_ANY, "",
		wxDefaultPosition, bs);
	m_rewind_btn->SetBitmap(wxGetBitmapFromMemory(rewind));
	m_rewind_btn->Bind(wxEVT_BUTTON, &MoviePanel::OnRewind, this);
	m_play_inv_btn = new wxButton(this, wxID_ANY, "",
		wxDefaultPosition, bs);
	m_play_inv_btn->SetBitmap(wxGetBitmapFromMemory(play_inv));
	m_play_inv_btn->Bind(wxEVT_BUTTON, &MoviePanel::OnPlayInv, this);
	m_play_btn = new wxButton(this, wxID_ANY, "",
		wxDefaultPosition, bs);
	m_play_btn->SetBitmap(wxGetBitmapFromMemory(play));
	m_play_btn->Bind(wxEVT_BUTTON, &MoviePanel::OnPlay, this);
	m_forward_btn = new wxButton(this, wxID_ANY, "",
		wxDefaultPosition, bs);
	m_forward_btn->SetBitmap(wxGetBitmapFromMemory(forward));
	m_forward_btn->Bind(wxEVT_BUTTON, &MoviePanel::OnForward, this);
	m_loop_btn = new wxToggleButton(this, wxID_ANY, "",
		wxDefaultPosition, bs);
	m_loop_btn->SetBitmap(wxGetBitmapFromMemory(loop));
	m_loop_btn->Bind(wxEVT_TOGGLEBUTTON, &MoviePanel::OnLoop, this);
	m_progress_text = new wxTextCtrl(this, wxID_ANY, "0.00",
		wxDefaultPosition, FromDIP(wxSize(50, -1)), wxTE_RIGHT);
	m_progress_text->SetFont(f);
	m_progress_text->Bind(wxEVT_TEXT, &MoviePanel::OnCurTimeText, this);
	st = new wxStaticText(this, wxID_ANY, "Sec.",
		wxDefaultPosition, FromDIP(wxSize(26, -1)));
	m_save_btn = new wxButton(this, wxID_ANY, "",
		wxDefaultPosition, bs);
	m_save_btn->SetBitmap(wxGetBitmapFromMemory(save));
	m_save_btn->Bind(wxEVT_BUTTON, &MoviePanel::OnSave, this);
	sizer4->AddStretchSpacer(2);
	sizer4->Add(m_rewind_btn, 0, wxALIGN_CENTER);
	sizer4->Add(m_play_inv_btn, 0, wxALIGN_CENTER);
	sizer4->Add(m_play_btn, 0, wxALIGN_CENTER);
	sizer4->Add(m_forward_btn, 0, wxALIGN_CENTER);
	sizer4->Add(m_loop_btn, 0, wxALIGN_CENTER);
	sizer4->AddStretchSpacer(1);
	sizer4->Add(bs.x, bs.y);
	sizer4->Add(m_progress_text, 0, wxALIGN_CENTER);
	sizer4->Add(st, 0, wxALIGN_CENTER);
	sizer4->AddStretchSpacer(2);
	sizer4->Add(m_save_btn, 0, wxALIGN_CENTER);
	sizer4->Add(5, 5);

	//sizer
	wxBoxSizer *sizerv = new wxBoxSizer(wxVERTICAL);
	sizerv->Add(m_notebook, 1, wxEXPAND);
	sizerv->Add(sizer1, 0, wxEXPAND);
	sizerv->Add(5, 5);
	sizerv->Add(sizer2, 0, wxEXPAND);
	sizerv->Add(5, 5);
	sizerv->Add(sizer3, 0, wxEXPAND);
	sizerv->Add(sizer4, 0, wxEXPAND);
	SetSizer(sizerv);
	Layout();
	SetAutoLayout(true);
	SetScrollRate(10, 10);
	Thaw();
}

MoviePanel::~MoviePanel()
{
}

void MoviePanel::LoadPerspective()
{

}

void MoviePanel::SavePerspective()
{

}

void MoviePanel::FluoUpdate(const fluo::ValueCollection& vc)
{
	if (FOUND_VALUE(gstNull))
		return;

	bool update_all = vc.empty();
	bool bval;
	int ival;

	//modes
	if (update_all || FOUND_VALUE(gstMovFps))
		m_fps_text->ChangeValue(wxString::Format("%.0f", glbin_moviemaker.GetFps()));

	if (update_all || FOUND_VALUE(gstMovLength))
		m_movie_len_text->ChangeValue(wxString::Format("%.2f", glbin_moviemaker.GetMovieLength()));

	if (update_all || FOUND_VALUE(gstMovViewList))
	{
		int i = 0;
		m_views_cmb->Clear();
		for (i = 0; i < m_frame->GetViewNum(); i++)
		{
			RenderCanvas* view = m_frame->GetView(i);
			if (view && m_views_cmb)
				m_views_cmb->Append(view->m_vrv->GetName());
		}
	}
	if (update_all || FOUND_VALUE(gstMovViewIndex))
		m_views_cmb->Select(glbin_mov_def.m_view_idx);

	if (update_all || FOUND_VALUE(gstMovSliderStyle))
	{
		bval = glbin_mov_def.m_slider_style;
		m_progress_sldr->SetMode(bval ? 1 : 0);
		if (bval)
			m_slider_btn->SetToolNormalBitmap(0, wxGetBitmapFromMemory(slider_type_rot));
		else
			m_slider_btn->SetToolNormalBitmap(0, wxGetBitmapFromMemory(slider_type_pos));
	}

	if (update_all || FOUND_VALUE(gstMovProgSlider))
	{
		m_progress_sldr->SetScrollbar(
			glbin_moviemaker.GetCurrentFrame(), 40,
			glbin_moviemaker.GetFrameNum() + 40, 1);
		m_progress_sldr->ChangeValue(glbin_moviemaker.GetCurrentFrame());
	}

	if (update_all || FOUND_VALUE(gstBeginFrame))
		m_start_frame_text->ChangeValue(wxString::Format("%d",
			glbin_moviemaker.GetStartFrame()));

	if (update_all || FOUND_VALUE(gstEndFrame))
		m_end_frame_text->ChangeValue(wxString::Format("%d",
			glbin_moviemaker.GetEndFrame()));

	if (update_all || FOUND_VALUE(gstCurrentFrame))
	{
		m_cur_frame_text->ChangeValue(wxString::Format("%d",
			glbin_moviemaker.GetCurrentFrame()));
		m_cur_frame_text->Update();
	}

	if (update_all || FOUND_VALUE(gstMovCurTime))
	{
		m_progress_text->ChangeValue(wxString::Format("%.2f",
			glbin_moviemaker.GetCurrentTime()));
		m_progress_text->Update();
	}

	if (update_all || FOUND_VALUE(gstMovPlay))
	{
		if (glbin_moviemaker.IsRunning())
			m_play_btn->SetBitmap(wxGetBitmapFromMemory(pause));
		else
		{
			if (glbin_settings.m_run_script)
				m_play_btn->SetBitmap(wxGetBitmapFromMemory(play_script));
			else
				m_play_btn->SetBitmap(wxGetBitmapFromMemory(play));
		}
	}

	if (update_all || FOUND_VALUE(gstMovRotEnable))
	{
		bval = glbin_moviemaker.GetRotateEnable();
		m_rot_chk->SetValue(bval);
		m_x_rd->Enable(bval);
		m_y_rd->Enable(bval);
		m_z_rd->Enable(bval);
	}
	
	if (update_all || FOUND_VALUE(gstMovRotAxis))
	{
		ival = glbin_moviemaker.GetRotateAxis();
		if (ival == 0)
			m_x_rd->SetValue(true);
		else if (ival == 1)
			m_y_rd->SetValue(true);
		else if (ival == 2)
			m_z_rd->SetValue(true);
	}

	if (update_all || FOUND_VALUE(gstMovRotAng))
		m_degree_text->SetValue(wxString::Format("%d", glbin_moviemaker.GetRotateDeg()));

	if (update_all || FOUND_VALUE(gstMovIntrpMode))
		m_rot_int_cmb->SetSelection(glbin_moviemaker.GetRotIntType());

	if (update_all || FOUND_VALUE(gstMovSeqMode))
	{
		bval = glbin_moviemaker.GetTimeSeqEnable();
		if (bval)
		{
			ival = glbin_moviemaker.GetSeqMode();
			if (ival == 1)
			{
				m_seq_chk->SetValue(true);
				m_bat_chk->SetValue(false);
			}
			else if (ival == 2)
			{
				m_seq_chk->SetValue(false);
				m_bat_chk->SetValue(true);
			}
		}
		else
		{
			m_seq_chk->SetValue(false);
			m_bat_chk->SetValue(false);
		}
	}

	if (update_all || FOUND_VALUE(gstCaptureParam))
		m_keyframe_chk->SetValue(glbin_moviemaker.GetKeyframeEnable());

	if (update_all || FOUND_VALUE(gstCropEnable))
	{
		bval = glbin_moviemaker.GetCropEnable();
		m_crop_chk->SetValue(bval);
		m_center_x_text->Enable(bval);
		m_center_y_text->Enable(bval);
		m_width_text->Enable(bval);
		m_height_text->Enable(bval);
	}

	if (update_all || FOUND_VALUE(gstCropValues))
	{
		m_center_x_text->ChangeValue(wxString::Format("%d", glbin_moviemaker.GetCropX()));
		m_center_y_text->ChangeValue(wxString::Format("%d", glbin_moviemaker.GetCropY()));
		m_width_text->ChangeValue(wxString::Format("%d", glbin_moviemaker.GetCropW()));
		m_height_text->ChangeValue(wxString::Format("%d", glbin_moviemaker.GetCropH()));
	}

	if (update_all || FOUND_VALUE(gstRunScript))
	{
		bval = glbin_settings.m_run_script;
		m_run_script_chk->SetValue(bval);
		if (bval)
			m_notebook->SetPageText(4, "Script (Enabled)");
		else
			m_notebook->SetPageText(4, "Script");
	}

	if (update_all || FOUND_VALUE(gstScriptFile))
	{
		m_script_file_text->SetValue(glbin_settings.m_script_file);
	}

	if (update_all || FOUND_VALUE(gstScriptList))
	{
		wxArrayString list;
		if (GetScriptFiles(list))
		{
			int idx = -1;
			for (size_t i = 0; i < list.GetCount(); ++i)
			{
				if (glbin_settings.m_script_file == list[i])
				{
					idx = i;
					break;
				}
			}
			if (idx >= 0)
			{
				m_script_list->SetItemState(idx,
					wxLIST_STATE_SELECTED,
					wxLIST_STATE_SELECTED);
				//wxSize ss = m_script_list->GetItemSpacing();
				//m_script_list->ScrollList(0, ss.y*idx);
			}
		}
	}
}

void MoviePanel::SetFps(double val)
{
	glbin_moviemaker.SetFps(val);
	
	FluoUpdate({ gstMovFps, gstMovLength });
}

void MoviePanel::SetMovieLength(double val)
{
	glbin_moviemaker.SetMovieLength(val);

	FluoUpdate({ gstMovFps, gstMovLength });
}

void MoviePanel::SetView(int index)
{
	glbin_mov_def.m_view_idx = index;

	FluoUpdate({ gstMovViewIndex });
}

void MoviePanel::SetSliderStyle(bool val)
{
	glbin_mov_def.m_slider_style = val;

	FluoUpdate({ gstMovSliderStyle });
}

void MoviePanel::SetStartFrame(int val)
{
	if (glbin_moviemaker.IsRunning())
		return;
	glbin_moviemaker.SetStartFrame(val);

	FluoUpdate({ gstBeginFrame, gstMovFps, gstMovLength });
}

void MoviePanel::SetEndFrame(int val)
{
	if (glbin_moviemaker.IsRunning())
		return;
	glbin_moviemaker.SetEndFrame(val);

	FluoUpdate({ gstEndFrame, gstMovFps, gstMovLength });
}

void MoviePanel::SetScrollFrame(int val, bool notify)
{
	//if (glbin_moviemaker.IsRunning())
	//	return;
	glbin_moviemaker.SetCurrentFrame(val);

	fluo::ValueCollection vc = { gstMovCurTime, gstCurrentFrame };
	if (notify)
		vc.insert(gstMovProgSlider);
	FluoRefresh(true, 2, vc, { glbin_mov_def.m_view_idx });
}

void MoviePanel::SetCurrentFrame(int val, bool notify)
{
	//if (glbin_moviemaker.IsRunning())
	//	return;
	glbin_moviemaker.SetCurrentFrame(val);

	fluo::ValueCollection vc = { gstMovCurTime, gstMovProgSlider };
	if (notify)
		vc.insert(gstCurrentFrame);
	FluoRefresh(true, 2, vc, { glbin_mov_def.m_view_idx });
}

void MoviePanel::SetCurrentTime(double val, bool notify)
{
	if (glbin_moviemaker.IsRunning())
		return;

	glbin_moviemaker.SetCurrentTime(val);

	fluo::ValueCollection vc = { gstCurrentFrame, gstMovProgSlider };
	if (notify)
		vc.insert(gstMovCurTime);
	FluoRefresh(true, 2, vc, { glbin_mov_def.m_view_idx });
}

void MoviePanel::Play()
{
	glbin_moviemaker.Play(false);

	fluo::ValueCollection vc = { gstMovPlay };
	FluoRefresh(true, 2, vc, { glbin_mov_def.m_view_idx });
}

void MoviePanel::PlayInv()
{
	glbin_moviemaker.Play(true);

	fluo::ValueCollection vc = { gstMovPlay };
	FluoRefresh(true, 2, vc, { glbin_mov_def.m_view_idx });
}

void MoviePanel::Rewind()
{
	glbin_moviemaker.Rewind();

	fluo::ValueCollection vc = { gstCurrentFrame, gstMovCurTime, gstMovProgSlider };
	FluoRefresh(true, 2, vc, { glbin_mov_def.m_view_idx });
}

void MoviePanel::Forward()
{
	glbin_moviemaker.Forward();

	fluo::ValueCollection vc = { gstCurrentFrame, gstMovCurTime, gstMovProgSlider };
	FluoRefresh(true, 2, vc, { glbin_mov_def.m_view_idx });
}

void MoviePanel::Loop(bool val)
{
	glbin_moviemaker.SetLoop(val);
}

void MoviePanel::IncFrame()
{
	if (glbin_moviemaker.IsRunning())
		return;
	int frame = glbin_moviemaker.GetCurrentFrame();
	frame++;
	glbin_moviemaker.SetCurrentFrame(frame);
	fluo::ValueCollection vc = { gstMovCurTime, gstMovProgSlider, gstCurrentFrame };
	FluoRefresh(true, 2, vc, { glbin_mov_def.m_view_idx });
}

void MoviePanel::DecFrame()
{
	if (glbin_moviemaker.IsRunning())
		return;
	int frame = glbin_moviemaker.GetCurrentFrame();
	frame--;
	glbin_moviemaker.SetCurrentFrame(frame);
	fluo::ValueCollection vc = { gstMovCurTime, gstMovProgSlider, gstCurrentFrame };
	FluoRefresh(true, 2, vc, { glbin_mov_def.m_view_idx });
}

void MoviePanel::Save(const wxString& filename)
{
	if (glbin_moviemaker.IsRunning())
		return;

	glbin_moviemaker.SetFileName(filename);
	glbin_moviemaker.PlaySave();

	fluo::ValueCollection vc = { gstMovPlay };
	FluoRefresh(true, 2, vc, { glbin_mov_def.m_view_idx });
}

void MoviePanel::SetKeyframeMovie(bool val)
{
	glbin_moviemaker.SetKeyframeEnable(val);

	FluoUpdate({ gstCaptureParam });
}

void MoviePanel::GenKey()
{
	long item = m_auto_key_list->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);

	if (item != -1)
	{
		m_advanced_movie->GenKey(item);
		m_notebook->SetSelection(1);
	}
}

void MoviePanel::SetCropEnable(bool val)
{
	glbin_moviemaker.SetCropEnable(val);
	FluoRefresh(false, 2, { gstCropEnable, gstCropValues }, { glbin_mov_def.m_view_idx });
}

void MoviePanel::SetCropValues(int x, int y, int w, int h)
{
	glbin_moviemaker.SetCropValues(x, y, w, h);
	FluoRefresh(false, 2, { gstCropValues }, { glbin_mov_def.m_view_idx });
}

void MoviePanel::OnNotebookPage(wxAuiNotebookEvent& event)
{
	int sel = event.GetSelection();
	if (sel == 0)
		glbin_moviemaker.SetKeyframeEnable(false);
	else if (sel == 1)
		glbin_moviemaker.SetKeyframeEnable(true);

	FluoUpdate({ gstCaptureParam });
	event.Skip();
}

void MoviePanel::OnFpsEdit(wxCommandEvent& event)
{
	wxString str = m_fps_text->GetValue();
	double val;
	if (str.ToDouble(&val))
		SetFps(val);
	event.Skip();
}

void MoviePanel::OnMovieLenText(wxCommandEvent& event)
{
	wxString str = m_movie_len_text->GetValue();
	double val;
	if (str.ToDouble(&val))
		SetMovieLength(val);
	event.Skip();
}

void MoviePanel::OnViewSelected(wxCommandEvent& event)
{
	int val = m_views_cmb->GetCurrentSelection();
	SetView(val);
	event.Skip();
}

void MoviePanel::OnSliderStyle(wxCommandEvent& event)
{
	bool val = m_slider_btn->GetToolState(0);
	SetSliderStyle(val);
	event.Skip();
}

void MoviePanel::OnProgressScroll(wxScrollEvent& event)
{
	int val = m_progress_sldr->GetValue();
	SetScrollFrame(val, false);
	event.Skip();
}

void MoviePanel::OnStartFrameText(wxCommandEvent& event)
{
	wxString str = m_start_frame_text->GetValue();
	long lval;
	if (str.ToLong(&lval))
		SetStartFrame(lval);
	event.Skip();
}

void MoviePanel::OnEndFrameText(wxCommandEvent& event)
{
	wxString str = m_end_frame_text->GetValue();
	long lval;
	if (str.ToLong(&lval))
		SetEndFrame(lval);
	event.Skip();
}

void MoviePanel::OnCurFrameText(wxCommandEvent& event)
{
	wxString str = m_cur_frame_text->GetValue();
	long lval;
	if (str.ToLong(&lval))
		SetCurrentFrame(lval, false);
	event.Skip();
}

void MoviePanel::OnCurTimeText(wxCommandEvent& event)
{
	wxString str = m_progress_text->GetValue();
	double val;
	if (str.ToDouble(&val))
		SetCurrentTime(val, false);
	event.Skip();
}

void MoviePanel::OnPlay(wxCommandEvent& event)
{
	Play();
	event.Skip();
}

void MoviePanel::OnPlayInv(wxCommandEvent& event)
{
	PlayInv();
	event.Skip();
}

void MoviePanel::OnRewind(wxCommandEvent& event)
{
	Rewind();
	event.Skip();
}

void MoviePanel::OnForward(wxCommandEvent& event)
{
	Forward();
	event.Skip();
}

void MoviePanel::OnLoop(wxCommandEvent& event)
{
	Loop(m_loop_btn->GetValue());
	event.Skip();
}

void MoviePanel::OnStartFrameBtn(wxCommandEvent& event)
{
	int frame = glbin_moviemaker.GetCurrentFrame();
	SetStartFrame(frame);
	event.Skip();
}

void MoviePanel::OnEndFrameBtn(wxCommandEvent& event)
{
	int frame = glbin_moviemaker.GetCurrentFrame();
	SetEndFrame(frame);
	event.Skip();
}

void MoviePanel::OnIncFrame(wxCommandEvent& event)
{
	IncFrame();
	event.Skip();
}

void MoviePanel::OnDecFrame(wxCommandEvent& event)
{
	DecFrame();
	event.Skip();
}

void MoviePanel::OnSave(wxCommandEvent& event)
{
	wxFileDialog* fopendlg = new wxFileDialog(
		m_frame, "Save Movie Sequence",
		"", "output", "MOV file (*.mov)|*.mov|TIF files (*.tif)|*.tif",
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	fopendlg->SetExtraControlCreator(CreateExtraCaptureControl);
	fopendlg->CenterOnParent();
	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		Save(fopendlg->GetPath());
	}
	delete fopendlg;
	event.Skip();
}

void MoviePanel::OnRotateChecked(wxCommandEvent& event)
{
	bool val = m_rot_chk->GetValue();
	glbin_moviemaker.SetRotateEnable(val);
	FluoUpdate({ gstMovRotEnable });
	event.Skip();
}

void MoviePanel::OnRotAxis(wxCommandEvent& event)
{
	int val = 0;
	if (m_x_rd->GetValue())
		val = 0;
	else if (m_y_rd->GetValue())
		val = 1;
	else if (m_z_rd->GetValue())
		val = 2;
	glbin_moviemaker.SetRotateAxis(val);
	//FluoUpdate({ gstMovRotAxis });
	event.Skip();
}

void MoviePanel::OnDegreeText(wxCommandEvent& event)
{
	wxString str = m_degree_text->GetValue();
	long ival = 0;
	str.ToLong(&ival);
	glbin_moviemaker.SetRotateDeg(ival);
	event.Skip();
}

void MoviePanel::OnRotIntCmb(wxCommandEvent& event)
{
	int val = m_rot_int_cmb->GetCurrentSelection();
	glbin_moviemaker.SetRotIntType(val);
	event.Skip();
}

void MoviePanel::OnSequenceChecked(wxCommandEvent& event)
{
	bool val = m_seq_chk->GetValue();
	if (val)
		glbin_moviemaker.SetSeqMode(1);
	glbin_moviemaker.SetTimeSeqEnable(val);
	FluoUpdate({});
	event.Skip();
}

void MoviePanel::OnBatchChecked(wxCommandEvent& event)
{
	int val = m_bat_chk->GetValue();
	if (val)
		glbin_moviemaker.SetSeqMode(2);
	glbin_moviemaker.SetTimeSeqEnable(val);
	FluoUpdate({});
	event.Skip();
}

void MoviePanel::OnKeyframeChk(wxCommandEvent& event)
{
	bool val = m_keyframe_chk->GetValue();
	SetKeyframeMovie(val);
	event.Skip();
}

//auto key
void MoviePanel::OnGenKey(wxCommandEvent& event)
{
	GenKey();
	event.Skip();
}

void MoviePanel::OnCropCheck(wxCommandEvent& event)
{
	SetCropEnable(m_crop_chk->GetValue());
	event.Skip();
}

void MoviePanel::OnResetCrop(wxCommandEvent& event)
{
	SetCropEnable(true);
	event.Skip();
}

void MoviePanel::OnEditCrop(wxCommandEvent& event)
{
	wxString temp;
	long x, y, w, h;
	temp = m_center_x_text->GetValue();
	temp.ToLong(&x);
	temp = m_center_y_text->GetValue();
	temp.ToLong(&y);
	temp = m_width_text->GetValue();
	temp.ToLong(&w);
	temp = m_height_text->GetValue();
	temp.ToLong(&h);

	SetCropValues(x, y, w, h);
	event.Skip();
}

void MoviePanel::OnCropSpinUp(wxSpinEvent& event)
{
	wxObject* obj = event.GetEventObject();
	wxTextCtrl* text_ctrl = 0;
	if (obj == m_center_x_spin)
		text_ctrl = m_center_x_text;
	if (obj == m_center_y_spin)
		text_ctrl = m_center_y_text;
	if (obj == m_width_spin)
		text_ctrl = m_width_text;
	if (obj == m_height_spin)
		text_ctrl = m_height_text;

	if (text_ctrl)
	{
		wxString str = text_ctrl->GetValue();
		long ival;
		if (str.ToLong(&ival))
			text_ctrl->SetValue(wxString::Format(
				"%d", ival + 1));
	}
	event.Skip();
}

void MoviePanel::OnCropSpinDown(wxSpinEvent& event)
{
	wxObject* obj = event.GetEventObject();
	wxTextCtrl* text_ctrl = 0;
	if (obj == m_center_x_spin)
		text_ctrl = m_center_x_text;
	if (obj == m_center_y_spin)
		text_ctrl = m_center_y_text;
	if (obj == m_width_spin)
		text_ctrl = m_width_text;
	if (obj == m_height_spin)
		text_ctrl = m_height_text;

	if (text_ctrl)
	{
		wxString str = text_ctrl->GetValue();
		long ival;
		if (str.ToLong(&ival))
			text_ctrl->SetValue(wxString::Format(
				"%d", ival - 1));
	}
	event.Skip();
}

//script
void MoviePanel::OnRunScriptChk(wxCommandEvent& event)
{
	if (!m_frame || !m_view)
		return;
	bool run_script = m_run_script_chk->GetValue();
	glbin_settings.m_run_script = run_script;
	m_view->SetRun4DScript(run_script);
	wxString str = m_script_file_text->GetValue();
	if (!str.IsEmpty())
	{
		glbin_settings.m_script_file = str;
		m_view->SetScriptFile(str);
	}
	if (run_script)
	{
		m_play_btn->SetBitmap(wxGetBitmapFromMemory(play_script));
		m_notebook->SetPageText(4, "Script (Enabled)");
	}
	else
	{
		m_play_btn->SetBitmap(wxGetBitmapFromMemory(play));
		m_notebook->SetPageText(4, "Script");
	}
	m_view->RefreshGL(39);
	event.Skip();
}

void MoviePanel::OnScriptFileEdit(wxCommandEvent& event)
{
	wxString str = m_script_file_text->GetValue();
	glbin_settings.m_script_file = str;
	m_view->SetScriptFile(str);
	event.Skip();
}

void MoviePanel::OnScriptClearBtn(wxCommandEvent& event)
{
	m_script_file_text->Clear();
	event.Skip();
}

void MoviePanel::OnScriptFileBtn(wxCommandEvent& event)
{
	wxFileDialog* fopendlg = new wxFileDialog(
		m_frame, "Choose a 4D script file", "", "",
		"4D script file (*.txt)|*.txt",
		wxFD_OPEN);

	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		wxString file = fopendlg->GetPath();
		glbin_settings.m_script_file = file;
		m_view->SetScriptFile(file);
		m_script_file_text->SetValue(file);

		//enable script if not
		if (!m_run_script_chk->GetValue())
		{
			m_run_script_chk->SetValue(true);
			OnRunScriptChk(event);
		}
	}

	delete fopendlg;
	event.Skip();
}

void MoviePanel::OnScriptListSelected(wxListEvent& event)
{
	long item = m_script_list->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);

	if (item != -1)
	{
		wxString file = m_script_list->GetItemText(item);
		wxString exePath = wxStandardPaths::Get().GetExecutablePath();
		exePath = wxPathOnly(exePath);
		file = exePath + GETSLASH() + "Scripts" +
			GETSLASH() + file + ".txt";
		m_script_file_text->SetValue(file);

		//enable script if not
		if (!m_run_script_chk->GetValue())
		{
			m_run_script_chk->SetValue(true);
			OnRunScriptChk(event);
		}
	}
	event.Skip();
}

int MoviePanel::GetScriptFiles(wxArrayString& list)
{
	wxString exePath = wxStandardPaths::Get().GetExecutablePath();
	exePath = wxPathOnly(exePath);
	wxString loc = exePath + GETSLASH() + "Scripts" +
		GETSLASH() + "*.txt";
	wxLogNull logNo;
	wxString file = wxFindFirstFile(loc);
	while (!file.empty())
	{
		list.Add(file);
		file = wxFindNextFile();
	}
	list.Sort();
	return list.GetCount();
}

//ch1
void MoviePanel::OnCh1Check(wxCommandEvent &event)
{
	wxCheckBox* ch1 = (wxCheckBox*)event.GetEventObject();
	if (ch1)
		glbin_settings.m_save_compress = ch1->GetValue();
}

//ch2
void MoviePanel::OnCh2Check(wxCommandEvent &event)
{
	wxCheckBox* ch2 = (wxCheckBox*)event.GetEventObject();
	if (ch2)
		glbin_settings.m_save_alpha = ch2->GetValue();
}

//ch3
void MoviePanel::OnCh3Check(wxCommandEvent &event)
{
	wxCheckBox* ch3 = (wxCheckBox*)event.GetEventObject();
	if (ch3)
		glbin_settings.m_save_float = ch3->GetValue();
}

void MoviePanel::OnDpiText(wxCommandEvent& event)
{
	wxTextCtrl* tx_dpi = (wxTextCtrl*)event.GetEventObject();
	wxString str = event.GetString();
	long lval;
	str.ToLong(&lval);
	glbin_settings.m_dpi = lval;
	if (!tx_dpi)
		return;
	wxCheckBox* ch_enlarge = (wxCheckBox*)tx_dpi->GetParent()->FindWindow(ID_ENLARGE_CHK);
	wxSlider* sl_enlarge = (wxSlider*)tx_dpi->GetParent()->FindWindow(ID_ENLARGE_SLDR);
	wxTextCtrl* tx_enlarge = (wxTextCtrl*)tx_dpi->GetParent()->FindWindow(ID_ENLARGE_TEXT);
	bool enlarge = lval > 72;
	RenderCanvas::SetEnlarge(enlarge);
	if (ch_enlarge)
		ch_enlarge->SetValue(enlarge);
	double enlarge_scale = (double)lval / 72.0;
	if (sl_enlarge)
	{
		sl_enlarge->Enable(enlarge);
		sl_enlarge->SetValue(std::round(enlarge_scale * 10));
	}
	if (tx_enlarge)
	{
		tx_enlarge->Enable(enlarge);
		tx_enlarge->ChangeValue(wxString::Format("%.1f", enlarge_scale));
	}
}

//enlarge output image
void MoviePanel::OnChEnlargeCheck(wxCommandEvent &event)
{
	wxCheckBox* ch_enlarge = (wxCheckBox*)event.GetEventObject();
	if (ch_enlarge)
	{
		bool enlarge = ch_enlarge->GetValue();
		RenderCanvas::SetEnlarge(enlarge);
		if (ch_enlarge->GetParent())
		{
			wxSlider* sl_enlarge = (wxSlider*)
				ch_enlarge->GetParent()->FindWindow(
					ID_ENLARGE_SLDR);
			wxTextCtrl* tx_enlarge = (wxTextCtrl*)
				ch_enlarge->GetParent()->FindWindow(
					ID_ENLARGE_TEXT);
			if (sl_enlarge && tx_enlarge)
			{
				if (enlarge)
				{
					sl_enlarge->Enable();
					tx_enlarge->Enable();
				}
				else
				{
					sl_enlarge->Disable();
					tx_enlarge->Disable();
				}
			}
		}
	}
}

void MoviePanel::OnSlEnlargeScroll(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	wxSlider* sl_enlarge = (wxSlider*)event.GetEventObject();
	if (sl_enlarge && sl_enlarge->GetParent())
	{
		wxTextCtrl* tx_enlarge = (wxTextCtrl*)
			sl_enlarge->GetParent()->FindWindow(
				ID_ENLARGE_TEXT);
		if (tx_enlarge)
		{
			wxString str = wxString::Format("%.1f", ival / 10.0);
			tx_enlarge->ChangeValue(str);
		}
	}
}

void MoviePanel::OnTxEnlargeText(wxCommandEvent &event)
{
	wxString str = event.GetString();
	double dval;
	str.ToDouble(&dval);
	RenderCanvas::SetEnlargeScale(dval);
	int ival = std::round(dval * 10);
	wxTextCtrl* tx_enlarge = (wxTextCtrl*)event.GetEventObject();
	if (tx_enlarge && tx_enlarge->GetParent())
	{
		wxSlider* sl_enlarge = (wxSlider*)
			tx_enlarge->GetParent()->FindWindow(
				ID_ENLARGE_SLDR);
		if (sl_enlarge)
			sl_enlarge->SetValue(ival);
	}
}

//movie quality
void MoviePanel::OnMovieQuality(wxCommandEvent &event)
{
	wxString str = event.GetString();
	double dval;
	str.ToDouble(&dval);
	glbin_settings.m_mov_bitrate = dval;
	wxTextCtrl* tx_qual = (wxTextCtrl*)event.GetEventObject();
	if (tx_qual && tx_qual->GetParent())
	{
		wxTextCtrl* tx_estimate = (wxTextCtrl*)
			tx_qual->GetParent()->FindWindow(ID_MOV_ESTIMATE_TEXT);
		if (tx_estimate)
		{
			double size = glbin_settings.m_mov_bitrate *
				glbin_moviemaker.GetMovieLength() / 8.;
			tx_estimate->SetValue(wxString::Format("%.2f", size));
		}
	}

}
//embed project
void MoviePanel::OnChEmbedCheck(wxCommandEvent &event)
{
	wxCheckBox* ch_embed = (wxCheckBox*)event.GetEventObject();
	if (ch_embed)
		glbin_settings.m_vrp_embed = ch_embed->GetValue();
}

wxWindow* MoviePanel::CreateExtraCaptureControl(wxWindow* parent)
{
	wxPanel* panel = new wxPanel(parent);
#ifdef _DARWIN
	panel->SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#endif
	wxBoxSizer *group1 = new wxStaticBoxSizer(
		new wxStaticBox(panel, wxID_ANY, "Additional Options"), wxVERTICAL);
	wxBoxSizer *line1 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *line2 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *line3 = new wxBoxSizer(wxHORIZONTAL);

	//compressed TIFF
	wxStaticText *tiffopts = new wxStaticText(panel, wxID_ANY, "TIFF Options:",
		wxDefaultPosition, wxDefaultSize);
	wxCheckBox *ch1 = new wxCheckBox(panel, wxID_ANY,
		"Lempel-Ziv-Welch Compression");
	ch1->Connect(ch1->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(MoviePanel::OnCh1Check), NULL, panel);
	if (ch1)
		ch1->SetValue(glbin_settings.m_save_compress);
	wxCheckBox *ch2 = new wxCheckBox(panel, wxID_ANY,
		"Save alpha");
	ch2->Connect(ch2->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(MoviePanel::OnCh2Check), NULL, panel);
	if (ch2)
		ch2->SetValue(glbin_settings.m_save_alpha);
	wxCheckBox *ch3 = new wxCheckBox(panel, wxID_ANY,
		"Save float channel");
	ch3->Connect(ch3->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(MoviePanel::OnCh3Check), NULL, panel);
	if (ch3)
		ch3->SetValue(glbin_settings.m_save_float);
	line1->Add(tiffopts, 0, wxALIGN_CENTER);
	line1->Add(ch1, 0, wxALIGN_CENTER);
	line1->Add(10, 10);
	line1->Add(ch2, 0, wxALIGN_CENTER);
	line1->Add(10, 10);
	line1->Add(ch3, 0, wxALIGN_CENTER);

	//dpi
	wxStaticText* st = new wxStaticText(panel, wxID_ANY, "DPI: ",
		wxDefaultPosition, wxDefaultSize);
	wxIntegerValidator<unsigned int> vald_int;
	wxTextCtrl* tx_dpi = new wxTextCtrl(panel, wxID_ANY,
		"", wxDefaultPosition, wxDefaultSize, 0, vald_int);
	tx_dpi->Connect(tx_dpi->GetId(), wxEVT_COMMAND_TEXT_UPDATED,
		wxCommandEventHandler(MoviePanel::OnDpiText), NULL, panel);
	float dpi = glbin_settings.m_dpi;
	tx_dpi->SetValue(wxString::Format("%.0f", dpi));
	//enlarge
	wxCheckBox* ch_enlarge = new wxCheckBox(panel, ID_ENLARGE_CHK,
		"Enlarge output image");
	ch_enlarge->Connect(ch_enlarge->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(MoviePanel::OnChEnlargeCheck), NULL, panel);
	bool enlarge = dpi > 72;
	double enlarge_scale = dpi / 72.0;
	ch_enlarge->SetValue(enlarge);
	wxSlider* sl_enlarge = new wxSlider(panel, ID_ENLARGE_SLDR,
		10, 10, 100);
	sl_enlarge->Connect(sl_enlarge->GetId(), wxEVT_COMMAND_SLIDER_UPDATED,
		wxScrollEventHandler(MoviePanel::OnSlEnlargeScroll), NULL, panel);
	sl_enlarge->Enable(enlarge);
	sl_enlarge->SetValue(std::round(enlarge_scale * 10));
	wxFloatingPointValidator<double> vald_fp(1);
	wxTextCtrl* tx_enlarge = new wxTextCtrl(panel, ID_ENLARGE_TEXT,
		"1.0", wxDefaultPosition, wxDefaultSize, 0, vald_fp);
	tx_enlarge->Connect(tx_enlarge->GetId(), wxEVT_COMMAND_TEXT_UPDATED,
		wxCommandEventHandler(MoviePanel::OnTxEnlargeText), NULL, panel);
	tx_enlarge->Enable(enlarge);
	tx_enlarge->SetValue(wxString::Format("%.1f", enlarge_scale));
	line2->Add(st, 0, wxALIGN_CENTER);
	line2->Add(tx_dpi, 0, wxALIGN_CENTER);
	line2->Add(10, 10);
	line2->Add(ch_enlarge, 0, wxALIGN_CENTER);
	line2->Add(10, 10);
	line2->Add(sl_enlarge, 1, wxEXPAND);
	line2->Add(10, 10);
	line2->Add(tx_enlarge, 0, wxALIGN_CENTER);

	// movie quality
	//bitrate
	wxStaticText *MOVopts = new wxStaticText(panel, wxID_ANY, "MOV Options:",
		wxDefaultPosition, wxDefaultSize);
	wxTextCtrl *bitrate_text = new wxTextCtrl(panel, wxID_ANY, "20.0",
		wxDefaultPosition, wxDefaultSize);
	bitrate_text->Connect(bitrate_text->GetId(), wxEVT_TEXT,
		wxCommandEventHandler(MoviePanel::OnMovieQuality), NULL, panel);
	st = new wxStaticText(panel, wxID_ANY, "Bitrate:",
		wxDefaultPosition, wxDefaultSize);
	wxStaticText *st2 = new wxStaticText(panel, wxID_ANY, "Mbps",
		wxDefaultPosition, wxDefaultSize);
	wxStaticText *st3 = new wxStaticText(panel, wxID_ANY, "Estimated size:",
		wxDefaultPosition, wxDefaultSize);
	wxTextCtrl* tx_estimate = new wxTextCtrl(panel, ID_MOV_ESTIMATE_TEXT, "2.5",
		wxDefaultPosition, wxDefaultSize);
	tx_estimate->Disable();
	glbin_settings.m_mov_bitrate = STOD(bitrate_text->GetValue().fn_str());
	double size = glbin_settings.m_mov_bitrate *
		glbin_moviemaker.GetMovieLength() / 8.;
	tx_estimate->SetValue(wxString::Format("%.2f", size));

	line3->Add(MOVopts, 0, wxALIGN_CENTER);
	line3->Add(st, 0, wxALIGN_CENTER);
	line3->Add(5, 5, wxALIGN_CENTER);
	line3->Add(bitrate_text, 0, wxALIGN_CENTER);
	line3->Add(5, 5, wxALIGN_CENTER);
	line3->Add(st2, 0, wxALIGN_CENTER);
	line3->AddStretchSpacer();
	line3->Add(st3, 0, wxALIGN_CENTER);
	line3->Add(tx_estimate, 0, wxALIGN_CENTER);
	st2 = new wxStaticText(panel, wxID_ANY, "MB",
		wxDefaultPosition, wxDefaultSize);
	line3->Add(5, 5, wxALIGN_CENTER);
	line3->Add(st2, 0, wxALIGN_CENTER);
	//copy all files check box
	wxCheckBox *ch_embed = 0;
	if (glbin_settings.m_prj_save)
	{
		ch_embed = new wxCheckBox(panel, wxID_ANY,
			"Embed all files in the project folder");
		ch_embed->Connect(ch_embed->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
			wxCommandEventHandler(MoviePanel::OnChEmbedCheck), NULL, panel);
		ch_embed->SetValue(glbin_settings.m_vrp_embed);
	}
	//group
	if (glbin_settings.m_prj_save && ch_embed) {
		wxBoxSizer *line3 = new wxBoxSizer(wxHORIZONTAL);
		line3->Add(ch_embed, 0, wxALIGN_CENTER);
		group1->Add(line3);
		group1->Add(5, 5, wxALIGN_CENTER);
	}
	group1->Add(line1);
	group1->Add(10, 10, wxALIGN_CENTER);
	group1->Add(line2);
	group1->Add(10, 10, wxALIGN_CENTER);
	group1->Add(line3);
	wxStaticText *mov_note = new wxStaticText(panel, wxID_ANY,
		"NOTE: Please make sure that the movie length is greater than 36 frames for correct compression!\n",
		wxDefaultPosition, wxDefaultSize);
	group1->Add(mov_note);
	group1->Add(10, 10, wxALIGN_CENTER);
	panel->SetSizerAndFit(group1);
	panel->Layout();

	return panel;
}

