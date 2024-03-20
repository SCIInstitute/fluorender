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

BEGIN_EVENT_TABLE(MoviePanel, wxPanel)
//time sequence
EVT_CHECKBOX(ID_SeqChk, MoviePanel::OnSequenceChecked)
EVT_CHECKBOX(ID_BatChk, MoviePanel::OnBatchChecked)
EVT_BUTTON(ID_IncTimeBtn, MoviePanel::OnUpFrame)
EVT_BUTTON(ID_DecTimeBtn, MoviePanel::OnDownFrame)
//rotations
EVT_CHECKBOX(ID_RotChk, MoviePanel::OnRotateChecked)
EVT_RADIOBUTTON(ID_XRd, MoviePanel::OnRotAxis)
EVT_RADIOBUTTON(ID_YRd, MoviePanel::OnRotAxis)
EVT_RADIOBUTTON(ID_ZRd, MoviePanel::OnRotAxis)
EVT_TEXT(ID_DegreeText, MoviePanel::OnDegreeText)
EVT_COMBOBOX(ID_RotIntCmb, MoviePanel::OnRotIntCmb)
//main controls
EVT_BUTTON(ID_PlayPause, MoviePanel::OnPrev)
EVT_BUTTON(ID_Rewind, MoviePanel::OnRewind)
EVT_COMMAND_SCROLL(ID_ProgressSldr, MoviePanel::OnTimeChange)
EVT_TEXT(ID_ProgressText, MoviePanel::OnTimeText)
EVT_BUTTON(ID_SaveMovie, MoviePanel::OnRun)
//script
EVT_CHECKBOX(ID_RunScriptChk, MoviePanel::OnRunScriptChk)
EVT_TEXT(ID_ScriptFileText, MoviePanel::OnScriptFileEdit)
EVT_BUTTON(ID_ScriptClearBtn, MoviePanel::OnScriptClearBtn)
EVT_BUTTON(ID_ScriptFileBtn, MoviePanel::OnScriptFileBtn)
EVT_LIST_ITEM_SELECTED(ID_ScriptList, MoviePanel::OnScriptListSelected)
EVT_LIST_ITEM_ACTIVATED(ID_ScriptList, MoviePanel::OnScriptListSelected)
//auto key
EVT_BUTTON(ID_GenKeyBtn, MoviePanel::OnGenKey)
EVT_LIST_ITEM_ACTIVATED(ID_AutoKeyList, MoviePanel::OnListItemAct)
//cropping
EVT_CHECKBOX(ID_CropChk, MoviePanel::OnCropCheck)
EVT_BUTTON(ID_ResetBtn, MoviePanel::OnResetCrop)
EVT_TEXT(ID_CenterXText, MoviePanel::OnEditCrop)
EVT_TEXT(ID_CenterYText, MoviePanel::OnEditCrop)
EVT_TEXT(ID_WidthText, MoviePanel::OnEditCrop)
EVT_TEXT(ID_HeightText, MoviePanel::OnEditCrop)
EVT_SPIN_UP(ID_CenterXSpin, MoviePanel::OnCropSpinUp)
EVT_SPIN_UP(ID_CenterYSpin, MoviePanel::OnCropSpinUp)
EVT_SPIN_UP(ID_WidthSpin, MoviePanel::OnCropSpinUp)
EVT_SPIN_UP(ID_HeightSpin, MoviePanel::OnCropSpinUp)
EVT_SPIN_DOWN(ID_CenterXSpin, MoviePanel::OnCropSpinDown)
EVT_SPIN_DOWN(ID_CenterYSpin, MoviePanel::OnCropSpinDown)
EVT_SPIN_DOWN(ID_WidthSpin, MoviePanel::OnCropSpinDown)
EVT_SPIN_DOWN(ID_HeightSpin, MoviePanel::OnCropSpinDown)
//notebook
EVT_NOTEBOOK_PAGE_CHANGED(ID_Notebook, MoviePanel::OnNbPageChange)
END_EVENT_TABLE()

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
	m_advanced_movie = new RecorderDlg(m_frame, this);
	m_frame->m_recorder_dlg = m_advanced_movie;

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
	m_auto_key_list = new wxListCtrl(page, ID_AutoKeyList,
		wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
	wxListItem itemCol;
	itemCol.SetText("ID");
	m_auto_key_list->InsertColumn(0, itemCol);
	itemCol.SetText("Auto Key Type");
	m_auto_key_list->InsertColumn(1, itemCol);
	//options
	//channel comb 1
	long tmp = m_auto_key_list->InsertItem(0, "1", 0);
	m_auto_key_list->SetItem(tmp, 1, "Channel combination nC1");
	tmp = m_auto_key_list->InsertItem(1, "2", 0);
	m_auto_key_list->SetItem(tmp, 1, "Channel combination nC2");
	tmp = m_auto_key_list->InsertItem(2, "3", 0);
	m_auto_key_list->SetItem(tmp, 1, "Channel combination nC3");
	m_auto_key_list->SetColumnWidth(1, -1);

	//button
	m_gen_keys_btn = new wxButton(page, ID_GenKeyBtn, "Generate");

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
	m_crop_chk = new wxCheckBox(page, ID_CropChk, "");
	m_reset_btn = new wxButton(page, ID_ResetBtn, "Reset",
		wxDefaultPosition, FromDIP(wxSize(110, 30)));
	m_reset_btn->SetBitmap(wxGetBitmapFromMemory(reset));
	sizer_8->Add(5, 5, 0);
	sizer_8->Add(st, 0, wxALIGN_CENTER);
	sizer_8->Add(m_crop_chk, 0, wxALIGN_CENTER);
	sizer_8->Add(100, 5, 0);
	sizer_8->Add(m_reset_btn, 0, wxALIGN_CENTER);
	//9th line
	st = new wxStaticText(page, 0, "Center:  X:",
		wxDefaultPosition, FromDIP(wxSize(85, 20)));
	m_center_x_text = new wxTextCtrl(page, ID_CenterXText, "",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), 0, vald_int);
	m_center_x_spin = new wxSpinButton(page, ID_CenterXSpin,
		wxDefaultPosition, FromDIP(wxSize(20, 20)));
	m_center_x_spin->SetRange(-0x8000, 0x7fff);
	sizer_9->Add(5, 5, 0);
	sizer_9->Add(st, 0, wxALIGN_CENTER);
	sizer_9->Add(m_center_x_text, 0, wxALIGN_CENTER);
	sizer_9->Add(m_center_x_spin, 0, wxALIGN_CENTER);
	st = new wxStaticText(page, 0, "       Y:",
		wxDefaultPosition, FromDIP(wxSize(50, 20)));
	m_center_y_text = new wxTextCtrl(page, ID_CenterYText, "",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), 0, vald_int);
	m_center_y_spin = new wxSpinButton(page, ID_CenterYSpin,
		wxDefaultPosition, FromDIP(wxSize(20, 20)));
	m_center_y_spin->SetRange(-0x8000, 0x7fff);
	sizer_9->Add(st, 0, wxALIGN_CENTER);
	sizer_9->Add(m_center_y_text, 0, wxALIGN_CENTER);
	sizer_9->Add(m_center_y_spin, 0, wxALIGN_CENTER);
	//10th line
	st = new wxStaticText(page, 0, "Size:    Width:",
		wxDefaultPosition, FromDIP(wxSize(85, 20)));
	m_width_text = new wxTextCtrl(page, ID_WidthText, "",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), 0, vald_int);
	m_width_spin = new wxSpinButton(page, ID_WidthSpin,
		wxDefaultPosition, FromDIP(wxSize(20, 20)));
	m_width_spin->SetRange(-0x8000, 0x7fff);
	sizer_10->Add(5, 5, 0);
	sizer_10->Add(st, 0, wxALIGN_CENTER);
	sizer_10->Add(m_width_text, 0, wxALIGN_CENTER);
	sizer_10->Add(m_width_spin, 0, wxALIGN_CENTER);
	st = new wxStaticText(page, 0, "   Height:",
		wxDefaultPosition, FromDIP(wxSize(50, 20)));
	m_height_text = new wxTextCtrl(page, ID_HeightText, "",
		wxDefaultPosition, FromDIP(wxSize(60, 20)), 0, vald_int);
	m_height_spin = new wxSpinButton(page, ID_HeightSpin,
		wxDefaultPosition, FromDIP(wxSize(20, 20)));
	m_height_spin->SetRange(-0x8000, 0x7fff);
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
	m_run_script_chk = new wxCheckBox(page, ID_RunScriptChk,
		"Enable execution of a script during playback.");
	wxStaticText* st;
	sizer_v->Add(10, 10);
	sizer_v->Add(m_run_script_chk);
	sizer_v->Add(5, 5);

	//browse button
	wxBoxSizer *sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Script File:",
		wxDefaultPosition, FromDIP(wxSize(80, -1)));
	m_script_file_btn = new wxButton(page, ID_ScriptFileBtn, "Browse...",
		wxDefaultPosition, FromDIP(wxSize(80, -1)));
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	sizer_1->AddStretchSpacer(1);
	sizer_1->Add(m_script_file_btn, 0, wxALIGN_CENTER);

	//file name
	wxBoxSizer *sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	m_script_file_text = new wxTextCtrl(page, ID_ScriptFileText, "",
		wxDefaultPosition, wxDefaultSize);
	m_script_clear_btn = new wxButton(page, ID_ScriptClearBtn, "X",
		wxDefaultPosition, FromDIP(wxSize(25, -1)));
	sizer_2->Add(m_script_file_text, 1, wxEXPAND);
	sizer_2->Add(m_script_clear_btn, 0, wxALIGN_CENTER);

	//script list
	m_script_list = new wxListCtrl(page, ID_ScriptList,
		wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
	wxListItem itemCol;
	itemCol.SetText("Built-in Scripte Files");
	m_script_list->InsertColumn(0, itemCol);
	m_script_list->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);

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
	SetDoubleBuffered(true);

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
	wxFont f;
	wxBoxSizer* sizer3 = new wxBoxSizer(wxHORIZONTAL);
	m_start_frame_text = new wxTextCtrl(this, wxID_ANY, "1",
		wxDefaultPosition, FromDIP(wxSize(40, -1)));
	f = m_start_frame_text->GetFont();
	f.MakeLarger();
	m_start_frame_text->SetFont(f);
	m_start_frame_text->Bind(wxEVT_TEXT, &MoviePanel::OnStartFrameText, this);
	m_end_frame_text = new wxTextCtrl(this, wxID_ANY, "10",
		wxDefaultPosition, FromDIP(wxSize(40, -1)));
	m_end_frame_text->SetFont(f);
	m_end_frame_text->Bind(wxEVT_TEXT, &MoviePanel::OnEndFrameText, this);
	m_progress_text = new wxTextCtrl(this, wxID_ANY, "0.00",
		wxDefaultPosition, FromDIP(wxSize(50, -1)));
	m_progress_text->SetFont(f);
	m_progress_text->Bind(wxEVT_TEXT, &MoviePanel::OnCurTimeText, this);
	st = new wxStaticText(this, 0, "Sec.");
	wxSize bs = FromDIP(wxSize(30, 30));
	sizer3->Add(5, 5);
	sizer3->Add(bs.x * 2, bs.y);
	sizer3->Add(m_start_frame_text, 0, wxALIGN_CENTER);
	sizer3->Add(m_end_frame_text, 0, wxALIGN_CENTER);
	sizer3->Add(bs.x, bs.y);
	sizer3->Add(m_progress_text, 0, wxALIGN_CENTER);
	sizer3->Add(st, 0, wxALIGN_CENTER);
	sizer3->Add(5, 5);

	wxBoxSizer* sizer4 = new wxBoxSizer(wxHORIZONTAL);
	m_play_btn = new wxButton(this, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(30, 30)));
	m_play_btn->SetBitmap(wxGetBitmapFromMemory(play));
	m_play_btn->Bind(wxEVT_BUTTON, &MoviePanel::OnPlay, this);
	m_rewind_btn = new wxButton(this, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(30, 30)));
	m_rewind_btn->SetBitmap(wxGetBitmapFromMemory(rewind));
	m_play_btn->Bind(wxEVT_BUTTON, &MoviePanel::OnRewind, this);
	m_start_frame_st = new wxButton(this, wxID_ANY, "Start:",
		wxDefaultPosition, FromDIP(wxSize(40, 30)));
	m_start_frame_st->Bind(wxEVT_BUTTON, &MoviePanel::OnStartFrameBtn, this);
	m_end_frame_st = new wxButton(this, wxID_ANY, "End:",
		wxDefaultPosition, FromDIP(wxSize(40, 30)));
	m_end_frame_st->Bind(wxEVT_BUTTON, &MoviePanel::OnEndFrameBtn, this);
	m_inc_time_btn = new wxButton(this, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(30, 30)));
	m_inc_time_btn->SetBitmap(wxGetBitmapFromMemory(plus));
	m_inc_time_btn->Bind(wxEVT_BUTTON, &MoviePanel::OnIncFrame, this);
	m_cur_frame_text = new wxTextCtrl(this, wxID_ANY, "0",
		wxDefaultPosition, FromDIP(wxSize(50, 30)));
	m_cur_frame_text->SetFont(f);
	m_cur_frame_text->Bind(wxEVT_TEXT, &MoviePanel::OnCurFrameText, this);
	m_dec_time_btn = new wxButton(this, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(30, 30)));
	m_dec_time_btn->SetBitmap(wxGetBitmapFromMemory(minus));
	m_dec_time_btn->Bind(wxEVT_BUTTON, &MoviePanel::OnDecFrame, this);
	m_save_btn = new wxButton(this, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(30, 30)));
	m_save_btn->SetBitmap(wxGetBitmapFromMemory(save));
	m_save_btn->Bind(wxEVT_BUTTON, &MoviePanel::OnSave, this);
	sizer4->Add(5, 5);
	sizer4->Add(m_play_btn, 0, wxALIGN_CENTER);
	sizer4->Add(m_rewind_btn, 0, wxALIGN_CENTER);
	sizer4->Add(m_start_frame_st, 0, wxALIGN_CENTER);
	sizer4->Add(m_end_frame_st, 0, wxALIGN_CENTER);
	sizer4->Add(m_dec_time_btn, 0, wxALIGN_CENTER);
	sizer4->Add(m_cur_frame_text, 0, wxALIGN_CENTER);
	sizer4->Add(m_inc_time_btn, 0, wxALIGN_CENTER);
	sizer4->AddStretchSpacer();
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
	SetSizerAndFit(sizerv);
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

	//modes
	if (update_all || FOUND_VALUE(gstMovFps))
		m_fps_text->ChangeValue(wxString::Format("%.0f", glbin_moviemaker.GetFps()));

	if (update_all || FOUND_VALUE(gstMovLength))
		m_movie_len_text->ChangeValue(wxString::Format("%.2f", (int)(glbin_moviemaker.GetMovieLength())));

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
		m_cur_frame_text->ChangeValue(wxString::Format("%d",
			glbin_moviemaker.GetCurrentFrame()));

	if (update_all || FOUND_VALUE(gstMovCurTime))
		m_progress_text->ChangeValue(wxString::Format("%.2f",
			glbin_moviemaker.GetCurrentTime()));

	if (update_all || FOUND_VALUE(gstMovPlay))
	{
		if (glbin_moviemaker.IsRunning())
		{
			if (glbin_settings.m_run_script)
				m_play_btn->SetBitmap(wxGetBitmapFromMemory(playscript));
			else
				m_play_btn->SetBitmap(wxGetBitmapFromMemory(play));
		}
		else
			m_play_btn->SetBitmap(wxGetBitmapFromMemory(pause));
	}

	if (update_all || FOUND_VALUE(gstCaptureParam))
		m_keyframe_chk->SetValue(glbin_moviemaker.GetKeyframeEnable());
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
	if (glbin_moviemaker.IsRunning())
		return;
	glbin_moviemaker.SetCurrentFrame(val);

	fluo::ValueCollection vc = { gstMovCurTime, gstCurrentFrame };
	if (notify)
		vc.insert(gstMovProgSlider);
	FluoRefresh(false, true, 2, vc);
}

void MoviePanel::SetCurrentFrame(int val, bool notify)
{
	if (glbin_moviemaker.IsRunning())
		return;
	glbin_moviemaker.SetCurrentFrame(val);

	fluo::ValueCollection vc = { gstMovCurTime, gstMovProgSlider };
	if (notify)
		vc.insert(gstCurrentFrame);
	FluoRefresh(false, true, 2, vc);
}

void MoviePanel::SetCurrentTime(double val, bool notify)
{
	if (glbin_moviemaker.IsRunning())
		return;

	glbin_moviemaker.SetCurrentTime(val);

	fluo::ValueCollection vc = { gstCurrentFrame, gstMovProgSlider };
	if (notify)
		vc.insert(gstMovCurTime);
	FluoRefresh(false, true, 2, vc);
}

void MoviePanel::Play()
{
	glbin_moviemaker.Play();

	fluo::ValueCollection vc = { gstMovPlay };
	FluoRefresh(false, true, 2, vc);
}

void MoviePanel::Rewind()
{
	glbin_moviemaker.Rewind();
}

void MoviePanel::IncFrame()
{
	if (glbin_moviemaker.IsRunning())
		return;
	int frame = glbin_moviemaker.GetCurrentFrame();
	frame++;
	glbin_moviemaker.SetCurrentFrame(frame);
	fluo::ValueCollection vc = { gstMovCurTime, gstMovProgSlider, gstCurrentFrame };
	FluoRefresh(false, true, 2, vc);
}

void MoviePanel::DecFrame()
{
	if (glbin_moviemaker.IsRunning())
		return;
	int frame = glbin_moviemaker.GetCurrentFrame();
	frame--;
	glbin_moviemaker.SetCurrentFrame(frame);
	fluo::ValueCollection vc = { gstMovCurTime, gstMovProgSlider, gstCurrentFrame };
	FluoRefresh(false, true, 2, vc);
}

void MoviePanel::Save(const wxString& filename)
{
	if (glbin_moviemaker.IsRunning())
		return;

	glbin_moviemaker.SetFileName(filename);
	glbin_moviemaker.PlaySave();

	fluo::ValueCollection vc = { gstMovPlay };
	FluoRefresh(false, true, 2, vc);
}

void MoviePanel::SetKeyframeMovie(bool val)
{
	glbin_moviemaker.SetKeyframeEnable(val);

	FluoUpdate({ gstCaptureParam });
}

void MoviePanel::OnNotebookPage(wxAuiNotebookEvent& event)
{
	int sel = event.GetSelection();
	if (sel == 0)
		glbin_moviemaker.SetKeyframeEnable(false);
	else if (sel == 1)
		glbin_moviemaker.SetKeyframeEnable(true);

	FluoUpdate({ gstCaptureParam });
}

void MoviePanel::OnFpsEdit(wxCommandEvent& event)
{
	wxString str = m_fps_text->GetValue();
	double val;
	if (str.ToDouble(&val))
		SetFps(val);
}

void MoviePanel::OnMovieLenText(wxCommandEvent& event)
{
	wxString str = m_movie_len_text->GetValue();
	double val;
	if (str.ToDouble(&val))
		SetMovieLength(val);
}

void MoviePanel::OnViewSelected(wxCommandEvent& event)
{
	int val = m_views_cmb->GetCurrentSelection();
	SetView(val);
}

void MoviePanel::OnSliderStyle(wxCommandEvent& event)
{
	bool val = m_slider_btn->GetToolState(0);
	SetSliderStyle(val);
}

void MoviePanel::OnProgressScroll(wxScrollEvent& event)
{
	int val = m_progress_sldr->GetValue();
	SetScrollFrame(val, false);
}

void MoviePanel::OnStartFrameText(wxCommandEvent& event)
{
	wxString str = m_start_frame_text->GetValue();
	long lval;
	if (str.ToLong(&lval))
		SetStartFrame(lval);
}

void MoviePanel::OnEndFrameText(wxCommandEvent& event)
{
	wxString str = m_end_frame_text->GetValue();
	long lval;
	if (str.ToLong(&lval))
		SetEndFrame(lval);
}

void MoviePanel::OnCurFrameText(wxCommandEvent& event)
{
	wxString str = m_cur_frame_text->GetValue();
	long lval;
	if (str.ToLong(&lval))
		SetCurrentFrame(lval, false);
}

void MoviePanel::OnCurTimeText(wxCommandEvent& event)
{
	wxString str = m_progress_text->GetValue();
	double val;
	if (str.ToDouble(&val))
		SetCurrentTime(val, false);
}

void MoviePanel::OnPlay(wxCommandEvent& event)
{
	Play();
}

void MoviePanel::OnRewind(wxCommandEvent& event)
{
	Rewind();
}

void MoviePanel::OnStartFrameBtn(wxCommandEvent& event)
{
	int frame = glbin_moviemaker.GetCurrentFrame();
	SetStartFrame(frame);
}

void MoviePanel::OnEndFrameBtn(wxCommandEvent& event)
{
	int frame = glbin_moviemaker.GetCurrentFrame();
	SetEndFrame(frame);
}

void MoviePanel::OnIncFrame(wxCommandEvent& event)
{
	IncFrame();
}

void MoviePanel::OnDecFrame(wxCommandEvent& event)
{
	DecFrame();
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
}

void MoviePanel::OnRotateChecked(wxCommandEvent& event)
{
	bool val = m_rot_chk->GetValue();
	glbin_moviemaker.SetRotateEnable(val);
	FluoUpdate({ gstMovRotEnable });
}

void MoviePanel::OnRotAxis(wxCommandEvent& event)
{
	if (m_x_rd->GetValue())
		m_rot_axis = 0;
	else if (m_y_rd->GetValue())
		m_rot_axis = 1;
	else if (m_z_rd->GetValue())
		m_rot_axis = 2;
}

void MoviePanel::OnDegreeText(wxCommandEvent& event)
{
	wxString str = m_degree_text->GetValue();
	long ival = 0;
	str.ToLong(&ival);
	m_rot_deg = ival;
}

void MoviePanel::OnRotIntCmb(wxCommandEvent& event)
{
	m_rot_int_type = m_rot_int_cmb->GetCurrentSelection();
}

void MoviePanel::OnSequenceChecked(wxCommandEvent& event)
{
	if (m_seq_chk->GetValue())
	{
		m_seq_mode = 1;
		SetTimeSeq(true);
	}
	else
	{
		m_seq_mode = 0;
		SetTimeSeq(false);
	}
}

void MoviePanel::OnBatchChecked(wxCommandEvent& event)
{
	if (m_bat_chk->GetValue())
	{
		m_seq_mode = 2;
		SetTimeSeq(true);
	}
	else
	{
		m_seq_mode = 0;
		SetTimeSeq(false);
	}
}



void MoviePanel::OnKeyframeChk(wxCommandEvent& event)
{
	bool val = m_keyframe_chk->GetValue();
	SetKeyframeMovie(val);
}

void MoviePanel::GetSettings()
{
	if (!m_view) return;

	SetTimeSeq(m_time_seq);
	SetRotate(m_rotate);
	SetRotAxis(m_rot_axis);
	SetRotDeg(m_rot_deg);
	SetCurrentTime(m_start_frame);
	SetCrop(m_view->m_draw_frame);
	AddScriptToList();
	GetScriptSettings(true);
	if (m_advanced_movie)
		m_advanced_movie->GetSettings(m_view);
}

void MoviePanel::GetScriptSettings(bool sel)
{
	//script
	if (!m_view)
		return;

	bool run_script = glbin_settings.m_run_script;
	m_view->SetRun4DScript(run_script);
	m_run_script_chk->SetValue(run_script);
	wxString script_file = glbin_settings.m_script_file;
	m_script_file_text->SetValue(script_file);
	m_view->SetScriptFile(script_file);
	//highlight if builtin
	wxArrayString list;
	if (GetScriptFiles(list))
	{
		int idx = -1;
		for (size_t i = 0; i < list.GetCount(); ++i)
		{
			if (script_file == list[i])
			{
				idx = i;
				break;
			}
		}
		if (idx >= 0 && sel)
		{
			m_script_list->SetItemState(idx,
				wxLIST_STATE_SELECTED,
				wxLIST_STATE_SELECTED);
			//wxSize ss = m_script_list->GetItemSpacing();
			//m_script_list->ScrollList(0, ss.y*idx);
		}
	}
	//change icon
	if (run_script)
	{
		if (m_running)
			m_play_btn->SetBitmap(wxGetBitmapFromMemory(pause));
		else
			m_play_btn->SetBitmap(wxGetBitmapFromMemory(playscript));
		m_notebook->SetPageText(4, "Script (Enabled)");
	}
	else
	{
		if (m_running)
			m_play_btn->SetBitmap(wxGetBitmapFromMemory(pause));
		else
			m_play_btn->SetBitmap(wxGetBitmapFromMemory(play));
		m_notebook->SetPageText(4, "Script");
	}
}

void MoviePanel::SetCurrentPage(int page)
{
	if (m_notebook)
		m_notebook->SetSelection(page);
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

void MoviePanel::AddScriptToList()
{
	m_script_list->DeleteAllItems();
	wxArrayString list;
	wxString filename;
	if (GetScriptFiles(list))
	{
		for (size_t i = 0; i < list.GetCount(); ++i)
		{
			filename = wxFileNameFromPath(list[i]);
			filename = filename.BeforeLast('.');
			m_script_list->InsertItem(
				m_script_list->GetItemCount(), filename);
		}
	}
}

void MoviePanel::GenKey()
{
	long item = m_auto_key_list->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);

	if (item != -1)
	{
		if (item == 0)
			m_advanced_movie->AutoKeyChanComb(1);
		else if (item == 1)
			m_advanced_movie->AutoKeyChanComb(2);
		else if (item == 2)
			m_advanced_movie->AutoKeyChanComb(3);

		m_notebook->SetSelection(1);
	}
}

void MoviePanel::SetCrop(bool value)
{
	m_crop = value;
	m_crop_chk->SetValue(m_crop);

	if (!m_view)
		return;
	if (m_crop)
	{
		m_view->CalcFrame();
		m_view->GetFrame(m_crop_x, m_crop_y, m_crop_w, m_crop_h);
		m_crop_x = std::round(m_crop_x + m_crop_w / 2.0);
		m_crop_y = std::round(m_crop_y + m_crop_h / 2.0);
		m_center_x_text->ChangeValue(wxString::Format("%d", m_crop_x));
		m_center_y_text->ChangeValue(wxString::Format("%d", m_crop_y));
		m_width_text->ChangeValue(wxString::Format("%d", m_crop_w));
		m_height_text->ChangeValue(wxString::Format("%d", m_crop_h));
		m_view->EnableFrame();
	}
	else
		m_view->DisableFrame();

	m_view->RefreshGL(39);
}

void MoviePanel::UpdateCrop()
{
	if (!m_view)
		return;

	m_view->SetFrame(std::round(m_crop_x - m_crop_w / 2.0),
		std::round(m_crop_y - m_crop_h / 2.0), m_crop_w, m_crop_h);
	if (m_crop)
		m_view->RefreshGL(39);
}

void MoviePanel::OnCropCheck(wxCommandEvent& event)
{
	SetCrop(m_crop_chk->GetValue());
}

void MoviePanel::OnResetCrop(wxCommandEvent& event)
{
	SetCrop(true);
}

void MoviePanel::OnEditCrop(wxCommandEvent& event)
{
	wxString temp;
	temp = m_center_x_text->GetValue();
	m_crop_x = STOI(temp.fn_str());
	temp = m_center_y_text->GetValue();
	m_crop_y = STOI(temp.fn_str());
	temp = m_width_text->GetValue();
	m_crop_w = STOI(temp.fn_str());
	temp = m_height_text->GetValue();
	m_crop_h = STOI(temp.fn_str());

	UpdateCrop();
}

void MoviePanel::OnCropSpinUp(wxSpinEvent& event)
{
	int sender_id = event.GetId();
	wxTextCtrl* text_ctrl = 0;
	switch (sender_id)
	{
	case ID_CenterXSpin:
		text_ctrl = m_center_x_text;
		break;
	case ID_CenterYSpin:
		text_ctrl = m_center_y_text;
		break;
	case ID_WidthSpin:
		text_ctrl = m_width_text;
		break;
	case ID_HeightSpin:
		text_ctrl = m_height_text;
		break;
	}
	if (text_ctrl)
	{
		wxString str = text_ctrl->GetValue();
		long ival;
		if (str.ToLong(&ival))
			text_ctrl->SetValue(wxString::Format(
				"%d", ival + 1));
	}
}

void MoviePanel::OnCropSpinDown(wxSpinEvent& event)
{
	int sender_id = event.GetId();
	wxTextCtrl* text_ctrl = 0;
	switch (sender_id)
	{
	case ID_CenterXSpin:
		text_ctrl = m_center_x_text;
		break;
	case ID_CenterYSpin:
		text_ctrl = m_center_y_text;
		break;
	case ID_WidthSpin:
		text_ctrl = m_width_text;
		break;
	case ID_HeightSpin:
		text_ctrl = m_height_text;
		break;
	}
	if (text_ctrl)
	{
		wxString str = text_ctrl->GetValue();
		long ival;
		if (str.ToLong(&ival))
			text_ctrl->SetValue(wxString::Format(
				"%d", ival - 1));
	}
}

void MoviePanel::OnListItemAct(wxListEvent &event)
{
	GenKey();
}

//script
void MoviePanel::OnRunScriptChk(wxCommandEvent &event)
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
		m_play_btn->SetBitmap(wxGetBitmapFromMemory(playscript));
		m_notebook->SetPageText(4, "Script (Enabled)");
	}
	else
	{
		m_play_btn->SetBitmap(wxGetBitmapFromMemory(play));
		m_notebook->SetPageText(4, "Script");
	}
	m_view->RefreshGL(39);
}

void MoviePanel::OnScriptFileEdit(wxCommandEvent &event)
{
	wxString str = m_script_file_text->GetValue();
	glbin_settings.m_script_file = str;
	m_view->SetScriptFile(str);
}

void MoviePanel::OnScriptClearBtn(wxCommandEvent &event)
{
	m_script_file_text->Clear();
}

void MoviePanel::OnScriptFileBtn(wxCommandEvent &event)
{
	wxFileDialog *fopendlg = new wxFileDialog(
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
}

void MoviePanel::OnScriptListSelected(wxListEvent &event)
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
}

//auto key
void MoviePanel::OnGenKey(wxCommandEvent& event) {
	GenKey();
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
	ch1->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &MoviePanel::OnCh1Check, panel);
	if (ch1)
		ch1->SetValue(glbin_settings.m_save_compress);
	wxCheckBox *ch2 = new wxCheckBox(panel, wxID_ANY,
		"Save alpha");
	ch2->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &MoviePanel::OnCh2Check, panel);
	if (ch2)
		ch2->SetValue(glbin_settings.m_save_alpha);
	wxCheckBox *ch3 = new wxCheckBox(panel, wxID_ANY,
		"Save float channel");
	ch3->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &MoviePanel::OnCh3Check, panel);
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
	tx_dpi->Bind(wxEVT_COMMAND_TEXT_UPDATED, &MoviePanel::OnDpiText, panel);
	float dpi = glbin_settings.m_dpi;
	tx_dpi->SetValue(wxString::Format("%.0f", dpi));
	//enlarge
	wxCheckBox* ch_enlarge = new wxCheckBox(panel, ID_ENLARGE_CHK,
		"Enlarge output image");
	ch_enlarge->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &MoviePanel::OnChEnlargeCheck, panel);
	bool enlarge = dpi > 72;
	double enlarge_scale = dpi / 72.0;
	ch_enlarge->SetValue(enlarge);
	wxSlider* sl_enlarge = new wxSlider(panel, ID_ENLARGE_SLDR,
		10, 10, 100);
	sl_enlarge->Bind(wxEVT_COMMAND_SLIDER_UPDATED, &MoviePanel::OnSlEnlargeScroll, panel);
	sl_enlarge->Enable(enlarge);
	sl_enlarge->SetValue(std::round(enlarge_scale * 10));
	wxFloatingPointValidator<double> vald_fp(1);
	wxTextCtrl* tx_enlarge = new wxTextCtrl(panel, ID_ENLARGE_TEXT,
		"1.0", wxDefaultPosition, wxDefaultSize, 0, vald_fp);
	tx_enlarge->Bind(wxEVT_COMMAND_TEXT_UPDATED, &MoviePanel::OnTxEnlargeText, panel);
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
	bitrate_text->Bind(wxEVT_TEXT, &MoviePanel::OnMovieQuality, panel);
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
	wxCheckBox *ch_embed;
	if (glbin_settings.m_prj_save)
	{
		ch_embed = new wxCheckBox(panel, wxID_ANY,
			"Embed all files in the project folder");
		ch_embed->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &MoviePanel::OnChEmbedCheck, panel);
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

