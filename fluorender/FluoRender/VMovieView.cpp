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
#include "VMovieView.h"
#include "VRenderFrame.h"
#include <tiffio.h>
#include <wx/aboutdlg.h>
#include <wx/valnum.h>
#include <wx/stdpaths.h>
#include "png_resource.h"
#include "img/icons.h"

BEGIN_EVENT_TABLE(VMovieView, wxPanel)
//time sequence
EVT_CHECKBOX(ID_SeqChk, VMovieView::OnSequenceChecked)
EVT_BUTTON(ID_IncTimeBtn, VMovieView::OnUpFrame)
EVT_BUTTON(ID_DecTimeBtn, VMovieView::OnDownFrame)
EVT_TEXT(ID_CurrentTimeText, VMovieView::OnTimeText)
//rotations
EVT_CHECKBOX(ID_RotChk, VMovieView::OnRotateChecked)
EVT_COMBOBOX(ID_RotIntCmb, VMovieView::OnRotIntCmb)
//fps, view combo, help
EVT_COMBOBOX(ID_ViewsCombo, VMovieView::OnViewSelected)
EVT_BUTTON(ID_HelpBtn, VMovieView::OnHelpBtn)
//main controls
EVT_BUTTON(ID_PlayPause, VMovieView::OnPrev)
EVT_BUTTON(ID_Rewind, VMovieView::OnRewind)
EVT_COMMAND_SCROLL(ID_ProgressSldr, VMovieView::OnTimeChange)
EVT_TEXT(ID_ProgressText, VMovieView::OnTimeEnter)
EVT_BUTTON(ID_SaveMovie, VMovieView::OnRun)
//script
EVT_CHECKBOX(ID_RunScriptChk, VMovieView::OnRunScriptChk)
EVT_TEXT(ID_ScriptFileText, VMovieView::OnScriptFileEdit)
EVT_BUTTON(ID_ScriptClearBtn, VMovieView::OnScriptClearBtn)
EVT_BUTTON(ID_ScriptFileBtn, VMovieView::OnScriptFileBtn)
EVT_LIST_ITEM_SELECTED(ID_ScriptList, VMovieView::OnScriptListSelected)
//auto key
EVT_BUTTON(ID_GenKeyBtn, VMovieView::OnGenKey)
EVT_LIST_ITEM_ACTIVATED(ID_AutoKeyList, VMovieView::OnListItemAct)
//cropping
EVT_CHECKBOX(ID_FrameChk, VMovieView::OnFrameCheck)
EVT_BUTTON(ID_ResetBtn, VMovieView::OnResetFrame)
EVT_TEXT(ID_CenterXText, VMovieView::OnFrameEditing)
EVT_TEXT(ID_CenterYText, VMovieView::OnFrameEditing)
EVT_TEXT(ID_WidthText, VMovieView::OnFrameEditing)
EVT_TEXT(ID_HeightText, VMovieView::OnFrameEditing)
EVT_SPIN_UP(ID_CenterXSpin, VMovieView::OnFrameSpinUp)
EVT_SPIN_UP(ID_CenterYSpin, VMovieView::OnFrameSpinUp)
EVT_SPIN_UP(ID_WidthSpin, VMovieView::OnFrameSpinUp)
EVT_SPIN_UP(ID_HeightSpin, VMovieView::OnFrameSpinUp)
EVT_SPIN_DOWN(ID_CenterXSpin, VMovieView::OnFrameSpinDown)
EVT_SPIN_DOWN(ID_CenterYSpin, VMovieView::OnFrameSpinDown)
EVT_SPIN_DOWN(ID_WidthSpin, VMovieView::OnFrameSpinDown)
EVT_SPIN_DOWN(ID_HeightSpin, VMovieView::OnFrameSpinDown)
//timer
EVT_TIMER(ID_Timer, VMovieView::OnTimer)
//notebook
EVT_NOTEBOOK_PAGE_CHANGED(ID_Notebook, VMovieView::OnNbPageChange)
END_EVENT_TABLE()

double VMovieView::m_Mbitrate = 1;
wxTextCtrl * VMovieView::m_estimated_size_text = 0;
wxTextCtrl * VMovieView::m_movie_time = 0;

wxWindow* VMovieView::CreateSimplePage(wxWindow *parent) {
	wxPanel *page = new wxPanel(parent);

	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;

	wxStaticText *st = 0, *st2 = 0;
	//sizers
	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizer_3 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizer_4 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizer_5 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizer_6 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizer_7 = new wxBoxSizer(wxHORIZONTAL);
	//vertical sizer
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
	m_seq_chk = new wxCheckBox(page, ID_SeqChk,
		"Time Sequence / Batch");
	m_time_start_text = new wxTextCtrl(page, ID_TimeStartText, "1",
		wxDefaultPosition, wxSize(35, -1));
	m_time_end_text = new wxTextCtrl(page, ID_TimeEndText, "10",
		wxDefaultPosition, wxSize(35, -1));
	st2 = new wxStaticText(page, wxID_ANY, "End Time: ",
		wxDefaultPosition, wxSize(75, -1));
	//sizer 1
	sizer_1->Add(m_seq_chk, 0, wxALIGN_CENTER);
	sizer_1->AddStretchSpacer();
	st = new wxStaticText(page, wxID_ANY, "Current Time");
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	sizer_1->Add(20, 5, 0);
	//sizer 2
	st = new wxStaticText(page, wxID_ANY, "Start Time: ",
		wxDefaultPosition, wxSize(75, -1));
	sizer_2->Add(5, 5, 0);
	sizer_2->Add(st, 0, wxALIGN_CENTER);
	sizer_2->Add(m_time_start_text, 0, wxALIGN_CENTER);
	sizer_2->Add(5, 5, 0);
	sizer_2->Add(st2, 0, wxALIGN_CENTER);
	sizer_2->Add(m_time_end_text, 0, wxALIGN_CENTER);
	m_inc_time_btn = new wxButton(page, ID_IncTimeBtn, "",
		wxDefaultPosition, wxSize(30, 30));
	m_dec_time_btn = new wxButton(page, ID_DecTimeBtn, "",
		wxDefaultPosition, wxSize(30, 30));
	m_time_current_text = new wxTextCtrl(page, ID_CurrentTimeText, "0",
		wxDefaultPosition, wxSize(35, -1));
	m_inc_time_btn->SetBitmap(wxGetBitmapFromMemory(plus));
	m_dec_time_btn->SetBitmap(wxGetBitmapFromMemory(minus));
	sizer_2->AddStretchSpacer();
	sizer_2->Add(m_dec_time_btn, 0, wxALIGN_CENTER);
	sizer_2->Add(5, 15, 0);
	sizer_2->Add(m_time_current_text, 0, wxALIGN_CENTER);
	sizer_2->Add(5, 15, 0);
	sizer_2->Add(m_inc_time_btn, 0, wxALIGN_CENTER);
	sizer_2->Add(5, 15, 0);
	//rotations
	m_rot_chk = new wxCheckBox(page, ID_RotChk,
		"Rotation");
	m_rot_chk->SetValue(true);
	//axis
	m_x_rd = new wxRadioButton(page, ID_XRd, "X", wxDefaultPosition, wxSize(30, 22));
	m_y_rd = new wxRadioButton(page, ID_YRd, "Y", wxDefaultPosition, wxSize(30, 22));
	m_z_rd = new wxRadioButton(page, ID_ZRd, "Z", wxDefaultPosition, wxSize(30, 22));
	m_y_rd->SetValue(true);
	//degrees
	m_degree_end = new wxTextCtrl(page, ID_DegreeEndText, "360",
		wxDefaultPosition, wxSize(50, -1));
	st2 = new wxStaticText(page, wxID_ANY, "Degrees",
		wxDefaultPosition, wxSize(60, -1));
	//sizer3
	sizer_3->Add(m_rot_chk, 0, wxALIGN_CENTER);
	//sizer 4
	sizer_4->Add(20, 5, 0);
	sizer_4->Add(m_x_rd, 0, wxALIGN_CENTER);
	//sizer 5
	sizer_5->Add(20, 5, 0);
	sizer_5->Add(m_y_rd, 0, wxALIGN_CENTER);
	sizer_5->Add(20, 5, 0);
	sizer_5->Add(m_degree_end, 0, wxALIGN_CENTER);
	sizer_5->Add(5, 5, 0);
	sizer_5->Add(st2, 0, wxALIGN_CENTER);
	//rotation interpolation
	st2 = new wxStaticText(page, wxID_ANY, "Style:");
	m_rot_int_cmb = new wxComboBox(page, ID_RotIntCmb, "",
		wxDefaultPosition, wxSize(65, -1), 0, NULL, wxCB_READONLY);
	m_rot_int_cmb->Append("Linear");
	m_rot_int_cmb->Append("Smooth");
	m_rot_int_cmb->Select(0);
	sizer_5->Add(5, 5);
	sizer_5->Add(st2, 0, wxALIGN_CENTER);
	sizer_5->Add(5, 5);
	sizer_5->Add(m_rot_int_cmb, 0, wxALIGN_CENTER);
	//sizer 6
	sizer_6->Add(20, 5, 0);
	sizer_6->Add(m_z_rd, 0, wxALIGN_CENTER);
	//movie length
	st = new wxStaticText(page, wxID_ANY, "Movie Length: ",
		wxDefaultPosition, wxSize(95, -1));
	st2 = new wxStaticText(page, wxID_ANY, "seconds",
		wxDefaultPosition, wxSize(60, -1));
	m_movie_time = new wxTextCtrl(page, ID_MovieTimeText, "5",
		wxDefaultPosition, wxSize(40, -1));
	//sizer 7
	sizer_7->AddStretchSpacer();
	sizer_7->Add(st, 0, wxALIGN_CENTER);
	sizer_7->Add(m_movie_time, 0, wxALIGN_CENTER);
	sizer_7->Add(5, 5, 0);
	sizer_7->Add(st2, 0, wxALIGN_CENTER);
	//all sizers
	sizer_v->Add(5, 5, 0);
	sizer_v->Add(sizer_3, 0, wxEXPAND);
	sizer_v->Add(5, 5, 0);
	sizer_v->Add(sizer_4, 0, wxEXPAND);
	sizer_v->Add(5, 5, 0);
	sizer_v->Add(sizer_5, 0, wxEXPAND);
	sizer_v->Add(5, 5, 0);
	sizer_v->Add(sizer_6, 0, wxEXPAND);
	sizer_v->Add(5, 10, 0);
	sizer_v->Add(sizer_1, 0, wxEXPAND);
	sizer_v->Add(5, 5, 0);
	sizer_v->Add(sizer_2, 0, wxEXPAND);
	sizer_v->Add(5, 10, 0);
	sizer_v->Add(sizer_7, 0, wxEXPAND);
	sizer_v->AddStretchSpacer();
	//set the page
	page->SetSizer(sizer_v);
	return page;
}

wxWindow* VMovieView::CreateAdvancedPage(wxWindow *parent) {
	wxPanel *page = new wxPanel(parent);
	//vertical sizer
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
	m_advanced_movie = new RecorderDlg(m_frame, page);
	(reinterpret_cast<VRenderFrame*>(m_frame))->m_recorder_dlg
		= m_advanced_movie;
	sizer_v->Add(m_advanced_movie, 0, wxEXPAND);
	page->SetSizer(sizer_v);
	return page;
}

wxWindow* VMovieView::CreateAutoKeyPage(wxWindow *parent) {
	wxPanel *page = new wxPanel(parent);

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
	return page;

}

wxWindow* VMovieView::CreateCroppingPage(wxWindow *parent) {
	wxPanel *page = new wxPanel(parent);

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
		wxDefaultPosition, wxSize(110, 20));
	m_frame_chk = new wxCheckBox(page, ID_FrameChk, "");
	m_reset_btn = new wxButton(page, ID_ResetBtn, "Reset",
		wxDefaultPosition, wxSize(110, 30));
	m_reset_btn->SetBitmap(wxGetBitmapFromMemory(reset));
	sizer_8->Add(5, 5, 0);
	sizer_8->Add(st, 0, wxALIGN_CENTER);
	sizer_8->Add(m_frame_chk, 0, wxALIGN_CENTER);
	sizer_8->Add(100, 5, 0);
	sizer_8->Add(m_reset_btn, 0, wxALIGN_CENTER);
	//9th line
	st = new wxStaticText(page, 0, "Center:  X:",
		wxDefaultPosition, wxSize(85, 20));
	m_center_x_text = new wxTextCtrl(page, ID_CenterXText, "",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	m_center_x_spin = new wxSpinButton(page, ID_CenterXSpin,
		wxDefaultPosition, wxSize(20, 20));
	sizer_9->Add(5, 5, 0);
	sizer_9->Add(st, 0, wxALIGN_CENTER);
	sizer_9->Add(m_center_x_text, 0, wxALIGN_CENTER);
	sizer_9->Add(m_center_x_spin, 0, wxALIGN_CENTER);
	st = new wxStaticText(page, 0, "       Y:",
		wxDefaultPosition, wxSize(50, 20));
	m_center_y_text = new wxTextCtrl(page, ID_CenterYText, "",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	m_center_y_spin = new wxSpinButton(page, ID_CenterYSpin,
		wxDefaultPosition, wxSize(20, 20));
	sizer_9->Add(st, 0, wxALIGN_CENTER);
	sizer_9->Add(m_center_y_text, 0, wxALIGN_CENTER);
	sizer_9->Add(m_center_y_spin, 0, wxALIGN_CENTER);
	//10th line
	st = new wxStaticText(page, 0, "Size:    Width:",
		wxDefaultPosition, wxSize(85, 20));
	m_width_text = new wxTextCtrl(page, ID_WidthText, "",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	m_width_spin = new wxSpinButton(page, ID_WidthSpin,
		wxDefaultPosition, wxSize(20, 20));
	sizer_10->Add(5, 5, 0);
	sizer_10->Add(st, 0, wxALIGN_CENTER);
	sizer_10->Add(m_width_text, 0, wxALIGN_CENTER);
	sizer_10->Add(m_width_spin, 0, wxALIGN_CENTER);
	st = new wxStaticText(page, 0, "   Height:",
		wxDefaultPosition, wxSize(50, 20));
	m_height_text = new wxTextCtrl(page, ID_HeightText, "",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	m_height_spin = new wxSpinButton(page, ID_HeightSpin,
		wxDefaultPosition, wxSize(20, 20));
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
	return page;

}

wxWindow* VMovieView::CreateScriptPage(wxWindow *parent)
{
	wxPanel *page = new wxPanel(parent);
	//script
	//vertical sizer
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
	m_run_script_chk = new wxCheckBox(page, ID_RunScriptChk,
		"Enable execution of a script on 4D data during playback.");
	wxStaticText* st = new wxStaticText(page, 0,
		"Also enable this option to show component colors.");
	sizer_v->Add(10, 10);
	sizer_v->Add(m_run_script_chk);
	sizer_v->Add(5, 5);
	sizer_v->Add(st, 0, wxALIGN_LEFT);
	sizer_v->Add(5, 5);

	//browse button
	wxBoxSizer *sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Script File:",
		wxDefaultPosition, wxSize(80, -1));
	m_script_file_btn = new wxButton(page, ID_ScriptFileBtn, "Browse...",
		wxDefaultPosition, wxSize(80, -1));
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	sizer_1->AddStretchSpacer(1);
	sizer_1->Add(m_script_file_btn, 0, wxALIGN_CENTER);

	//file name
	wxBoxSizer *sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	m_script_file_text = new wxTextCtrl(page, ID_ScriptFileText, "",
		wxDefaultPosition, wxDefaultSize);
	m_script_clear_btn = new wxButton(page, ID_ScriptClearBtn, "X",
		wxDefaultPosition, wxSize(25, -1));
	sizer_2->Add(m_script_file_text, 1, wxEXPAND);
	sizer_2->Add(m_script_clear_btn, 0, wxALIGN_CENTER);

	//script list
	m_script_list = new wxListCtrl(page, ID_ScriptList,
		wxDefaultPosition, wxSize(-1, -1), wxLC_REPORT | wxLC_SINGLE_SEL);
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
	return page;
}

VMovieView::VMovieView(wxWindow* frame,
	wxWindow* parent,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name) :
	wxPanel(parent, id, pos, size, style, name),
	m_frame(frame),
	m_starting_rot(0.),
	m_timer(this, ID_Timer),
	m_cur_time(0.0),
	m_running(false),
	m_record(false),
	m_current_page(0),
	m_rot_int_type(0),
	m_delayed_stop(false)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);

	//notebook
	m_notebook = new wxNotebook(this, ID_Notebook);
	m_notebook->AddPage(CreateSimplePage(m_notebook), "Basic");
	m_notebook->AddPage(CreateAdvancedPage(m_notebook), "Advanced");
	m_notebook->AddPage(CreateAutoKeyPage(m_notebook), "Auto Key");
	m_notebook->AddPage(CreateCroppingPage(m_notebook), "Cropping");
	m_notebook->AddPage(CreateScriptPage(m_notebook), "4D Script");
	//renderview selector
	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	//FPS
	m_fps_text = new wxTextCtrl(this, ID_FPS_Text, "30",
		wxDefaultPosition, wxSize(30, -1));
	//other
	m_views_cmb = new wxComboBox(this, ID_ViewsCombo, "",
		wxDefaultPosition, wxSize(120, -1), 0, NULL, wxCB_READONLY);
	m_help_btn = new wxButton(this, ID_HelpBtn, "?",
		wxDefaultPosition, wxSize(25, 25));
	//sizer 1
	sizer_1->Add(5, 5, 0);
	wxStaticText * st = new wxStaticText(this, wxID_ANY, "FPS:",
		wxDefaultPosition, wxSize(30, -1));
	wxStaticText * st2 = new wxStaticText(this, wxID_ANY, "Capture:",
		wxDefaultPosition, wxSize(60, -1));
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	sizer_1->Add(m_fps_text, 0, wxALIGN_CENTER);
	sizer_1->AddStretchSpacer();
	sizer_1->Add(st2, 0, wxALIGN_CENTER);
	sizer_1->Add(m_views_cmb, 0, wxALIGN_CENTER);
	sizer_1->Add(5, 5, 0);
	sizer_1->Add(m_help_btn, 0, wxALIGN_CENTER);

	//the play/rewind/slider/save
	wxBoxSizer *sizerH = new wxBoxSizer(wxHORIZONTAL);
	m_play_btn = new wxButton(this, ID_PlayPause, "",
		wxDefaultPosition, wxSize(30, 30));
	m_play_btn->SetBitmap(wxGetBitmapFromMemory(play));
	sizerH->Add(m_play_btn, 0, wxEXPAND);
	m_rewind_btn = new wxButton(this, ID_Rewind, "",
		wxDefaultPosition, wxSize(30, 30));
	m_rewind_btn->SetBitmap(wxGetBitmapFromMemory(rewind));
	sizerH->Add(m_rewind_btn, 0, wxEXPAND);
	m_progress_sldr = new wxSlider(this, ID_ProgressSldr, 0, 0, 360);
	sizerH->Add(m_progress_sldr, 1, wxEXPAND);
	m_progress_text = new wxTextCtrl(this, ID_ProgressText, "0.00",
		wxDefaultPosition, wxSize(50, -1));
	sizerH->Add(m_progress_text, 0, wxEXPAND);
	wxStaticText * st3 = new wxStaticText(this, 0, "s",
		wxDefaultPosition, wxSize(20, 20));
	sizerH->Add(5, 5, 0);
	sizerH->Add(st3, 0, wxALIGN_CENTER);
	m_save_btn = new wxButton(this, ID_SaveMovie, "Save...",
		wxDefaultPosition, wxSize(80, 30));
	m_save_btn->SetBitmap(wxGetBitmapFromMemory(save));
	sizerH->Add(m_save_btn, 0, wxEXPAND);
	//interface
	wxBoxSizer *sizerV = new wxBoxSizer(wxVERTICAL);
	sizerV->Add(m_notebook, 1, wxEXPAND);
	sizerV->AddStretchSpacer();
	sizerV->Add(sizer_1, 0, wxEXPAND);
	sizerV->Add(sizerH, 0, wxEXPAND);
	SetSizerAndFit(sizerV);
	Layout();
	Init();
}

VMovieView::~VMovieView() {}

void VMovieView::OnViewSelected(wxCommandEvent& event)
{
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (!vr_frame) return;
	vr_frame->m_mov_view = m_views_cmb->GetCurrentSelection();
	GetSettings(vr_frame->m_mov_view);
}

void VMovieView::GetSettings(int view)
{
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (!vr_frame) return;
	VRenderView* vrv = (*vr_frame->GetViewList())[view];
	if (!vrv) return;
	m_views_cmb->SetSelection(vr_frame->m_mov_view);
	if (vr_frame->m_mov_axis == 1)
		m_x_rd->SetValue(true);
	else if (vr_frame->m_mov_axis == 2)
		m_y_rd->SetValue(true);
	else if (vr_frame->m_mov_axis == 3)
		m_z_rd->SetValue(true);
	if (vr_frame->m_mov_angle_end != "")
		m_degree_end->SetValue(vr_frame->m_mov_angle_end);
	if (vr_frame->m_mov_step != "")
		m_movie_time->SetValue(vr_frame->m_mov_step);
	if (vr_frame->m_mov_frames != "")
		m_fps_text->ChangeValue(vr_frame->m_mov_frames);

	if (vrv->GetFrameEnabled()) {
		m_frame_chk->SetValue(true);
		int x, y, w, h;
		vrv->GetFrame(x, y, w, h);
		m_center_x_text->SetValue(wxString::Format("%d",
			int(x + w / 2.0 + 0.5)));
		m_center_y_text->SetValue(wxString::Format("%d",
			int(y + h / 2.0 + 0.5)));
		m_width_text->SetValue(wxString::Format("%d", w));
		m_height_text->SetValue(wxString::Format("%d", h));
	}
	else
		m_frame_chk->SetValue(false);

	AddScriptToList();
	GetScriptSettings();

	if (m_advanced_movie)
		m_advanced_movie->GetSettings(vrv);
}

void VMovieView::GetScriptSettings()
{
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	//script
	if (vr_frame && vr_frame->GetSettingDlg())
	{
		bool run_script = vr_frame->GetSettingDlg()->GetRunScript();
		m_run_script_chk->SetValue(
			run_script);
		wxString script_file =
			vr_frame->GetSettingDlg()->GetScriptFile();
		m_script_file_text->SetValue(script_file);
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
			if (idx >= 0)
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
			m_notebook->SetPageText(4, "4D Script (Enabled)");
		}
		else
		{
			if (m_running)
				m_play_btn->SetBitmap(wxGetBitmapFromMemory(pause));
			else
				m_play_btn->SetBitmap(wxGetBitmapFromMemory(play));
			m_notebook->SetPageText(4, "4D Script");
		}
	}
}

void VMovieView::SetCurrentPage(int page)
{
	if (m_notebook)
		m_notebook->SetSelection(page);
}

int VMovieView::GetScriptFiles(wxArrayString& list)
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
	return list.GetCount();
}

void VMovieView::AddScriptToList()
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

void VMovieView::Init() {
	DisableTime();
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (!vr_frame) return;
	int i = 0;
	m_views_cmb->Clear();
	for (i = 0; i < vr_frame->GetViewNum(); i++) {
		VRenderView* vrv = vr_frame->GetView(i);
		if (vrv && m_views_cmb)
			m_views_cmb->Append(vrv->GetName());
	}
	if (i)
		m_views_cmb->Select(0);
	GetSettings(0);
}

void VMovieView::GenKey()
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

void VMovieView::AddView(wxString view) {
	if (m_views_cmb)
		m_views_cmb->Append(view);
}

void VMovieView::DeleteView(wxString view) {
	if (!m_views_cmb)
		return;
	int cur_sel = m_views_cmb->GetCurrentSelection();
	int del = m_views_cmb->FindString(view, true);
	if (del != wxNOT_FOUND)
		m_views_cmb->Delete(del);
	if (cur_sel == del)
		m_views_cmb->Select(0);
}

void VMovieView::SetView(int index) {
	if (m_views_cmb)
		m_views_cmb->SetSelection(index);
}

void VMovieView::OnTimer(wxTimerEvent& event) {
	//get all of the progress info
	double len;
	long fps;
	m_movie_time->GetValue().ToDouble(&len);
	m_fps_text->GetValue().ToLong(&fps);

	if (m_delayed_stop)
	{
		if (m_record)
			WriteFrameToFile(int(fps*len + 0.5));
		m_delayed_stop = false;
		wxCommandEvent e;
		OnStop(e);
		return;
	}

	if (TextureRenderer::get_mem_swap() &&
		TextureRenderer::get_start_update_loop() &&
		!TextureRenderer::get_done_update_loop())
	{
		wxString str = m_views_cmb->GetValue();
		VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
		if (!vr_frame) return;
		VRenderView* vrv = vr_frame->GetView(str);
		if (!vrv) return;
		vrv->RefreshGL(false, false);
		return;
	}

	//move forward in time (limits FPS usability to 100 FPS)
	m_cur_time += 1.0 / double(fps);
	//frame only increments when time passes a whole number
	int frame = int(fps * m_cur_time + 0.5);
	SetProgress(m_cur_time / len);
	//update the rendering frame since we have advanced.
	if (frame != m_last_frame) {
		int start_time = STOI(m_time_start_text->GetValue().fn_str());
		int end_time = STOI(m_time_end_text->GetValue().fn_str());
		int tot_time = end_time - start_time + 1;
		m_time_current_text->ChangeValue(wxString::Format("%d",
			((int)(start_time + tot_time * m_cur_time / len + 0.5))));
		if (m_record)
			WriteFrameToFile(int(fps*len + 0.5));
		m_last_frame = frame;
		SetRendering(m_cur_time / len);
	}
	if (len - m_cur_time < 0.1 / double(fps) || m_cur_time > len)
		m_delayed_stop = true;
}

void VMovieView::OnNbPageChange(wxBookCtrlEvent& event)
{
	int sel = event.GetSelection();
	int old_sel = event.GetOldSelection();
	if (sel <=1)
		m_current_page = sel;
}

void VMovieView::OnPrev(wxCommandEvent& event)
{
	if (m_running) {
		wxCommandEvent e;
		OnStop(e);
		return;
	}
	m_running = true;
	FLIVR::TextureRenderer::maximize_uptime_ = true;
	m_play_btn->SetBitmap(wxGetBitmapFromMemory(pause));
	int slider_pos = m_progress_sldr->GetValue();
	long fps;
	m_fps_text->GetValue().ToLong(&fps);
	double len;
	m_movie_time->GetValue().ToDouble(&len);
	if (slider_pos < 360 && slider_pos > 0 &&
		!(len - m_cur_time < 0.1 / double(fps) || m_cur_time > len)) {
		m_timer.Start(int(1000.0 / double(fps) + 0.5));
		return;
	}
	wxString str = m_views_cmb->GetValue();
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (!vr_frame) return;
	VRenderView* vrv = vr_frame->GetView(str);
	if (!vrv) return;

	//basic options
	int rot_axis = 1;
	if (m_x_rd->GetValue())
		rot_axis = 1;
	if (m_y_rd->GetValue())
		rot_axis = 2;
	if (m_z_rd->GetValue())
		rot_axis = 3;

	double x, y, z;
	vrv->GetRotations(x, y, z);
	if (rot_axis == 1)
		m_starting_rot = x;
	else if (rot_axis == 2)
		m_starting_rot = y;
	else
		m_starting_rot = z;
	while (m_starting_rot > 360.) m_starting_rot -= 360.;
	while (m_starting_rot < -360.) m_starting_rot += 360.;
	if (360. - std::abs(m_starting_rot) < 0.001)
		m_starting_rot = 0.;
	if (m_current_page == 1)
	{
		Interpolator *interpolator = vr_frame->GetInterpolator();
		if (interpolator && interpolator->GetLastIndex() > 0)
		{
			int frames = int(interpolator->GetLastT());
			double runtime = (double)frames / (double)fps;
			m_movie_time->ChangeValue(wxString::Format("%.2f", runtime));
		}
	}
	SetProgress(0.);
	SetRendering(0.);
	m_last_frame = 0;
	m_timer.Start(int(1000.0 / double(fps) + 0.5));
}

void VMovieView::OnRun(wxCommandEvent& event)
{
	wxFileDialog *fopendlg = new wxFileDialog(
		m_frame, "Save Movie Sequence",
		"", "output", "MOV file (*.mov)|*.mov|TIF files (*.tif)|*.tif",
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	fopendlg->SetExtraControlCreator(CreateExtraCaptureControl);
	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		m_filename = fopendlg->GetPath();
		Run();
	}
	delete fopendlg;
}

void VMovieView::OnStop(wxCommandEvent& event)
{
	bool run_script = m_run_script_chk->GetValue();
	if (run_script)
		m_play_btn->SetBitmap(wxGetBitmapFromMemory(playscript));
	else
		m_play_btn->SetBitmap(wxGetBitmapFromMemory(play));

	m_timer.Stop();
	m_running = false;
	m_record = false;
	encoder_.close();
	FLIVR::TextureRenderer::maximize_uptime_ = false;
}

void VMovieView::OnRewind(wxCommandEvent& event) {
	wxString str = m_views_cmb->GetValue();
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (!vr_frame) return;
	VRenderView* vrv = vr_frame->GetView(str);
	if (!vrv) return;
	vrv->SetParams(0.);
	wxCommandEvent e;
	OnStop(e);
	m_time_current_text->ChangeValue(wxString::Format("%d", 0));
	SetProgress(0.);
	SetRendering(0.);
}

void VMovieView::OnFrameCheck(wxCommandEvent& event) {
	wxString str = m_views_cmb->GetValue();
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (!vr_frame) return;
	VRenderView* vrv = vr_frame->GetView(str);
	if (!vrv) return;
	if (m_frame_chk->GetValue())
	{
		int x, y, w, h;
		vrv->CalcFrame();
		vrv->GetFrame(x, y, w, h);
		m_center_x_text->SetValue(wxString::Format("%d", int(x + w / 2.0 + 0.5)));
		m_center_y_text->SetValue(wxString::Format("%d", int(y + h / 2.0 + 0.5)));
		m_width_text->SetValue(wxString::Format("%d", w));
		m_height_text->SetValue(wxString::Format("%d", h));
		vrv->EnableFrame();
	}
	else
		vrv->DisableFrame();

	vrv->RefreshGL();
}

void VMovieView::OnResetFrame(wxCommandEvent& event) {
	wxString str = m_views_cmb->GetValue();
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (!vr_frame) return;
	VRenderView* vrv = vr_frame->GetView(str);
	if (!vrv) return;
	int x, y, w, h;
	vrv->CalcFrame();
	vrv->GetFrame(x, y, w, h);
	m_center_x_text->SetValue(wxString::Format("%d", int(x + w / 2.0 + 0.5)));
	m_center_y_text->SetValue(wxString::Format("%d", int(y + h / 2.0 + 0.5)));
	m_width_text->SetValue(wxString::Format("%d", w));
	m_height_text->SetValue(wxString::Format("%d", h));
	vrv->RefreshGL();
}

void VMovieView::OnFrameEditing(wxCommandEvent& event) {
	wxString str = m_views_cmb->GetValue();
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (!vr_frame) return;
	VRenderView* vrv = vr_frame->GetView(str);
	if (!vrv) return;
	int x, y, w, h;
	wxString temp;

	temp = m_center_x_text->GetValue();
	x = STOI(temp.fn_str());
	temp = m_center_y_text->GetValue();
	y = STOI(temp.fn_str());
	temp = m_width_text->GetValue();
	w = STOI(temp.fn_str());
	temp = m_height_text->GetValue();
	h = STOI(temp.fn_str());

	vrv->SetFrame(int(x - w / 2.0 + 0.5), int(y - h / 2.0 + 0.5), w, h);
	if (vrv->GetFrameEnabled())
		vrv->RefreshGL();
}

void VMovieView::OnFrameSpinUp(wxSpinEvent& event) {
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
		wxString str_val = text_ctrl->GetValue();
		char str[256];
		sprintf(str, "%d", STOI(str_val.fn_str()) + 1);
		text_ctrl->SetValue(str);
		wxCommandEvent e;
		OnFrameEditing(e);
	}
}

void VMovieView::OnFrameSpinDown(wxSpinEvent& event) {
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
		wxString str_val = text_ctrl->GetValue();
		char str[256];
		sprintf(str, "%d", STOI(str_val.fn_str()) - 1);
		text_ctrl->SetValue(str);
		wxCommandEvent e;
		OnFrameEditing(e);
	}
}

void VMovieView::DisableRot() {
	m_x_rd->Disable();
	m_y_rd->Disable();
	m_z_rd->Disable();
	m_degree_end->Disable();
	m_rot_chk->SetValue(false);
}

void VMovieView::EnableRot() {
	m_x_rd->Enable();
	m_y_rd->Enable();
	m_z_rd->Enable();
	m_degree_end->Enable();
	m_rot_chk->SetValue(true);
}

void VMovieView::OnHelpBtn(wxCommandEvent &event) {
	::wxLaunchDefaultBrowser(wxString(HELP_MANUAL) + wxString("#page=58"));
}

void VMovieView::DisableTime() {
	m_time_start_text->Disable();
	m_time_end_text->Disable();
	m_inc_time_btn->Disable();
	m_dec_time_btn->Disable();
	m_time_current_text->Disable();
	m_seq_chk->SetValue(false);
}

void VMovieView::EnableTime()
{
	wxString str = m_views_cmb->GetValue();
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (!vr_frame) return;
	VRenderView* vrv = vr_frame->GetView(str);
	if (!vrv) return;
	int first, sec, tmp;
	vrv->Get4DSeqFrames(first, sec, tmp);
	if (sec - first == 0)
		vrv->Get3DBatFrames(first, sec, tmp);
	m_time_start_text->ChangeValue(wxString::Format("%d", first));
	m_time_end_text->ChangeValue(wxString::Format("%d", sec));
	m_time_start_text->Enable();
	m_time_end_text->Enable();
	m_inc_time_btn->Enable();
	m_dec_time_btn->Enable();
	m_time_current_text->Enable();
	m_seq_chk->SetValue(true);

	wxString fps_str = m_fps_text->GetValue();
	unsigned long fps;
	fps_str.ToULong(&fps);
	unsigned int mov_len = (sec - first + 1) / fps + 1;
	m_movie_time->SetValue(wxString::Format("%d", mov_len));
}

void VMovieView::UpFrame()
{
	if (m_running) return;
	int start_time = STOI(m_time_start_text->GetValue().fn_str());
	int end_time = STOI(m_time_end_text->GetValue().fn_str());
	int current_time = STOI(m_time_current_text->GetValue().fn_str()) + 1;
	if (current_time > end_time) current_time = start_time;
	int time = end_time - start_time + 1;
	double pcnt = (double)current_time / (double)time;
	m_time_current_text->ChangeValue(wxString::Format("%d", current_time));
	SetProgress(pcnt);
	SetRendering(pcnt);
}

void VMovieView::DownFrame()
{
	if (m_running) return;
	int start_time = STOI(m_time_start_text->GetValue().fn_str());
	int end_time = STOI(m_time_end_text->GetValue().fn_str());
	int current_time = STOI(m_time_current_text->GetValue().fn_str()) - 1;
	if (current_time < start_time) current_time = end_time;
	int time = end_time - start_time + 1;
	double pcnt = (double)current_time / (double)time;
	m_time_current_text->ChangeValue(wxString::Format("%d", current_time));
	SetProgress(pcnt);
	SetRendering(pcnt);
}

void VMovieView::OnListItemAct(wxListEvent &event)
{
	GenKey();
}

//script
void VMovieView::OnRunScriptChk(wxCommandEvent &event)
{
	bool run_script = m_run_script_chk->GetValue();
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetSettingDlg())
		vr_frame->GetSettingDlg()->SetRunScript(run_script);
	if (run_script)
	{
		m_play_btn->SetBitmap(wxGetBitmapFromMemory(playscript));
		m_notebook->SetPageText(4, "4D Script (Enabled)");
	}
	else
	{
		m_play_btn->SetBitmap(wxGetBitmapFromMemory(play));
		m_notebook->SetPageText(4, "4D Script");
	}
	if (vr_frame)
	{
		VRenderView* vrv = (*vr_frame->GetViewList())[m_views_cmb->GetCurrentSelection()];
		if (vrv)
			vrv->RefreshGL();
	}
}

void VMovieView::OnScriptFileEdit(wxCommandEvent &event)
{
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetSettingDlg())
	{
		wxString str = m_script_file_text->GetValue();
		vr_frame->GetSettingDlg()->SetScriptFile(str);
	}
}

void VMovieView::OnScriptClearBtn(wxCommandEvent &event)
{
	m_script_file_text->Clear();
}

void VMovieView::OnScriptFileBtn(wxCommandEvent &event)
{
	wxFileDialog *fopendlg = new wxFileDialog(
		m_frame, "Choose a 4D script file", "", "",
		"4D script file (*.txt)|*.txt",
		wxFD_OPEN);

	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		wxString file = fopendlg->GetPath();
		VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
		if (vr_frame && vr_frame->GetSettingDlg())
			vr_frame->GetSettingDlg()->SetScriptFile(file);
		m_script_file_text->SetValue(file);
	}

	delete fopendlg;
}

void VMovieView::OnScriptListSelected(wxListEvent &event)
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
	}
}

//auto key
void VMovieView::OnGenKey(wxCommandEvent& event) {
	GenKey();
}

void VMovieView::OnTimeChange(wxScrollEvent &event) {
	if (m_running) return;
	int frame = event.GetPosition();
	double pcnt = (((double)frame) / 360.);
	SetRendering(pcnt);

	int start_time = STOI(m_time_start_text->GetValue().fn_str());
	int end_time = STOI(m_time_end_text->GetValue().fn_str());
	int time = end_time - start_time + 1;
	m_time_current_text->ChangeValue(wxString::Format("%d",
		(int)(start_time + time * pcnt)));

	double movie_time;
	m_movie_time->GetValue().ToDouble(&movie_time);
	m_cur_time = pcnt*movie_time;
	wxString str = wxString::Format("%.2f", m_cur_time);
	if (str != m_progress_text->GetValue())
		m_progress_text->ChangeValue(str);
}

void VMovieView::SetRendering(double pcnt) {
	wxString str = m_views_cmb->GetValue();
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (!vr_frame) return;
	VRenderView* vrv = vr_frame->GetView(str);
	if (!vrv) return;
	//advanced options
	if (m_current_page == 1)
	{
		Interpolator *interpolator = vr_frame->GetInterpolator();
		if (interpolator && interpolator->GetLastIndex() > 0)
		{
			int end_frame = int(interpolator->GetLastT());
			vrv->SetParams(pcnt * end_frame);
			vrv->RefreshGL();
			return;
		}
	}
	//basic options
	int start_time = STOI(m_time_start_text->GetValue().fn_str());
	int end_time = STOI(m_time_end_text->GetValue().fn_str());
	int time = end_time - start_time + 1;
	time = int(start_time + time * pcnt + 0.5);

	if (m_seq_chk->GetValue()) {
		int first, sec, tmp;
		vrv->Get4DSeqFrames(first, sec, tmp);
		if (sec - first > 0) {
			vrv->Set4DSeqFrame(time, true);
		}
		else {
			vrv->Set3DBatFrame(time);
		}
	}
	if (m_rot_chk->GetValue()) {
		double x, y, z, val;
		vrv->GetRotations(x, y, z);
		if (m_x_rd->GetValue())
			val = x;
		if (m_y_rd->GetValue())
			val = y;
		if (m_z_rd->GetValue())
			val = z;
		double deg = STOD(m_degree_end->GetValue().fn_str());
		if (m_rot_int_type == 0)
			val = m_starting_rot + pcnt * deg;
		else if (m_rot_int_type == 1)
			val = m_starting_rot +
			(-2.0*pcnt*pcnt*pcnt + 3.0*pcnt*pcnt) * deg;
		if (m_x_rd->GetValue())
			vrv->SetRotations(val, y, z);
		else if (m_y_rd->GetValue())
			vrv->SetRotations(x, val, z);
		else if (m_z_rd->GetValue())
			vrv->SetRotations(x, y, val);
	}
	vrv->RefreshGL();
}

void VMovieView::OnTimeEnter(wxCommandEvent& event) {
	if (m_running) return;
	//wxString str = m_progress_text->GetValue();
	//double iVal;
	//str.ToDouble(&iVal);
	wxString str = m_movie_time->GetValue();
	double movie_time;
	str.ToDouble(&movie_time);
	double pcnt = (m_cur_time / movie_time);
	m_progress_sldr->SetValue(360. * pcnt);
	SetRendering(pcnt);

	int start_time = STOI(m_time_start_text->GetValue().fn_str());
	int end_time = STOI(m_time_end_text->GetValue().fn_str());
	int time = end_time - start_time + 1;
	m_time_current_text->ChangeValue(wxString::Format("%d",
		(int)(start_time + time * pcnt)));
}

void VMovieView::OnRotateChecked(wxCommandEvent& event) {
	if (m_rot_chk->GetValue())
		EnableRot();
	else
		DisableRot();
}

void VMovieView::OnRotIntCmb(wxCommandEvent& event)
{
	m_rot_int_type = m_rot_int_cmb->GetCurrentSelection();
}

void VMovieView::OnSequenceChecked(wxCommandEvent& event) {
	if (m_seq_chk->GetValue())
		EnableTime();
	else
		DisableTime();
}

void VMovieView::SetProgress(double pcnt) {
	pcnt = std::abs(pcnt);
	m_progress_sldr->SetValue(pcnt * 360.);
	double movie_time = STOD(m_movie_time->GetValue().fn_str());
	m_cur_time = pcnt*movie_time;
	wxString st = wxString::Format("%.2f", m_cur_time);
	m_progress_text->ChangeValue(st);
}

void VMovieView::WriteFrameToFile(int total_frames)
{
	wxString str = m_views_cmb->GetValue();
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (!vr_frame) return;
	VRenderView* vrv = vr_frame->GetView(str);
	if (!vrv) return;
	wxString s_length = wxString::Format("%d", total_frames);
	int length = s_length.Length();
	wxString format = wxString::Format("_%%0%dd", length);
	wxString outputfilename = wxString::Format("%s" + format + "%s", m_filename,
		m_last_frame, ".tif");

	//capture
	bool bmov = filetype_.IsSameAs(".mov");
	int chann = VRenderFrame::GetSaveAlpha() ? 4 : 3;
	bool fp32 = bmov?false:VRenderFrame::GetSaveFloat();
	int x, y, w, h;
	void* image = 0;
	vrv->m_glview->ReadPixels(chann, fp32,
		x, y, w, h, &image);

	/*if (m_frame_chk->GetValue())
		vrv->GetFrame(x, y, w, h);
	else {
		x = 0;
		y = 0;
		w = vrv->GetGLSize().x;
		h = vrv->GetGLSize().y;
	}

	glPixelStorei(GL_PACK_ROW_LENGTH, w);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	unsigned char *image = new unsigned char[w*h*chann];
	glReadBuffer(GL_FRONT);
	glReadPixels(x, y, w, h, chann == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, image);
	glPixelStorei(GL_PACK_ROW_LENGTH, 0);*/

	string str_fn = outputfilename.ToStdString();
	if (bmov)
	{
		//flip vertically 
		unsigned char *flip = new unsigned char[w*h * 3];
		for (size_t yy = 0; yy < (size_t)h; yy++)
			for (size_t xx = 0; xx < (size_t)w; xx++)
				memcpy(flip + 3 * (w * yy + xx), (unsigned char*)image + chann * (w * (h - yy - 1) + xx), 3);
		bool worked = encoder_.set_frame_rgb_data(flip);
		worked = encoder_.write_video_frame(m_last_frame);
		if (flip)
			delete[]flip;
		if (image)
			delete[]image;
	}
	else
	{
		TIFF *out = TIFFOpen(str_fn.c_str(), "wb");
		if (!out) return;
		TIFFSetField(out, TIFFTAG_IMAGEWIDTH, w);
		TIFFSetField(out, TIFFTAG_IMAGELENGTH, h);
		TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, chann);
		if (fp32)
		{
			TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 32);
			TIFFSetField(out, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
		}
		else
		{
			TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 8);
			//TIFFSetField(out, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
		}
		TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
		TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
		TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
		if (VRenderFrame::GetCompression())
			TIFFSetField(out, TIFFTAG_COMPRESSION, COMPRESSION_LZW);

		tsize_t linebytes = chann * w * (fp32 ? 4 : 1);
		void *buf = NULL;
		buf = _TIFFmalloc(linebytes);
		TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(out, 0));
		for (uint32 row = 0; row < (uint32)h; row++)
		{
			if (fp32)
			{
				float* line = ((float*)image) + (h - row - 1) * chann * w;
				memcpy(buf, line, linebytes);
			}
			else
			{// check the index here, and figure out why not using h*linebytes
				unsigned char* line = ((unsigned char*)image) + (h - row - 1) * chann * w;
				memcpy(buf, line, linebytes);
			}
			if (TIFFWriteScanline(out, buf, row, 0) < 0)
				break;
		}
		TIFFClose(out);
		if (buf)
			_TIFFfree(buf);
		if (image)
			delete[]image;
	}
}

void VMovieView::OnUpFrame(wxCommandEvent& event) {
	UpFrame();
}

void VMovieView::OnDownFrame(wxCommandEvent& event) {
	DownFrame();
}

void VMovieView::SetCurrentTime(size_t t)
{
	m_time_current_text->ChangeValue(wxString::Format(wxT("%i"), (int)t));
	//wxCommandEvent e;
	//OnTimeText(e);
	int start_time = STOI(m_time_start_text->GetValue().fn_str());
	int end_time = STOI(m_time_end_text->GetValue().fn_str());
	int current_time = STOI(m_time_current_text->GetValue().fn_str());
	if (current_time < start_time) current_time = end_time;
	if (current_time > end_time) current_time = start_time;
	int time = end_time - start_time + 1;
	double pcnt = (double)current_time / (double)time;
	SetProgress(pcnt);
}

void VMovieView::Run()
{
	wxString str = m_views_cmb->GetValue();
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (!vr_frame) return;
	if (vr_frame && vr_frame->GetSettingDlg())
	{
		VRenderFrame::SetSaveProject(vr_frame->GetSettingDlg()->GetProjSave());
		VRenderFrame::SetSaveAlpha(vr_frame->GetSettingDlg()->GetSaveAlpha());
		VRenderFrame::SetSaveFloat(vr_frame->GetSettingDlg()->GetSaveFloat());
	}
	VRenderView* vrv = vr_frame->GetView(str);
	if (!vrv) return;

	wxCommandEvent e;
	OnRewind(e);

	filetype_ = m_filename.SubString(m_filename.Len() - 4,
		m_filename.Len() - 1);
	if (filetype_.IsSameAs(wxString(".mov"))) {
		int x, y, w, h;
		if (m_frame_chk->GetValue())
			vrv->GetFrame(x, y, w, h);
		else {
			x = 0;
			y = 0;
			w = vrv->GetGLSize().x;
			h = vrv->GetGLSize().y;
		}
		long fps;
		m_fps_text->GetValue().ToLong(&fps);
		encoder_.open(m_filename.ToStdString(), w, h, fps,
			m_Mbitrate * 1000000);
	}
	m_filename = m_filename.SubString(0, m_filename.Len() - 5);
	m_record = true;
	if (vr_frame && vr_frame->GetSettingDlg())
	{
		vr_frame->GetSettingDlg()->SetSaveAlpha(VRenderFrame::GetSaveAlpha());
		vr_frame->GetSettingDlg()->SetSaveFloat(VRenderFrame::GetSaveFloat());
		if (vr_frame->GetSettingDlg()->GetProjSave())
		{
			wxString new_folder;
			new_folder = m_filename + "_project";
			wxFileName::Mkdir(new_folder, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
			wstring name = m_filename.ToStdWstring();
			name = GET_NAME(name);
			wxString prop_file = new_folder + GETSLASH()
				+ name + "_project.vrp";
			vr_frame->SaveProject(prop_file);
		}
	}

	OnPrev(e);
}

void VMovieView::OnTimeText(wxCommandEvent& event) {
	if (m_running) return;
	int start_time = STOI(m_time_start_text->GetValue().fn_str());
	int end_time = STOI(m_time_end_text->GetValue().fn_str());
	int current_time = STOI(m_time_current_text->GetValue().fn_str());
	if (current_time < start_time) current_time = end_time;
	if (current_time > end_time) current_time = start_time;
	int time = end_time - start_time + 1;
	double pcnt = (double)current_time / (double)time;
	SetProgress(pcnt);
	SetRendering(pcnt);
}

//ch1
void VMovieView::OnCh1Check(wxCommandEvent &event) {
	wxCheckBox* ch1 = (wxCheckBox*)event.GetEventObject();
	if (ch1)
		VRenderFrame::SetCompression(ch1->GetValue());
}
//ch2
void VMovieView::OnCh2Check(wxCommandEvent &event) {
	wxCheckBox* ch2 = (wxCheckBox*)event.GetEventObject();
	if (ch2)
		VRenderFrame::SetSaveAlpha(ch2->GetValue());
}
//ch3
void VMovieView::OnCh3Check(wxCommandEvent &event) {
	wxCheckBox* ch3 = (wxCheckBox*)event.GetEventObject();
	if (ch3)
		VRenderFrame::SetSaveFloat(ch3->GetValue());
}
//movie quality
void VMovieView::OnMovieQuality(wxCommandEvent &event) {
	m_Mbitrate = STOD(((wxTextCtrl*)event.GetEventObject())
		->GetValue().fn_str());
	double size = m_Mbitrate * STOD(
		m_movie_time->GetValue().fn_str()) / 8.;
	m_estimated_size_text->SetValue(wxString::Format("%.2f", size));
}
//embed project
void VMovieView::OnChEmbedCheck(wxCommandEvent &event) {
	wxCheckBox* ch_embed = (wxCheckBox*)event.GetEventObject();
	if (ch_embed)
		VRenderFrame::SetEmbedProject(ch_embed->GetValue());
}

wxWindow* VMovieView::CreateExtraCaptureControl(wxWindow* parent) {
	wxPanel* panel = new wxPanel(parent, 0, wxDefaultPosition, wxSize(600, 100));
#ifdef _DARWIN
	panel->SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#endif
	wxBoxSizer *group1 = new wxStaticBoxSizer(
		new wxStaticBox(panel, wxID_ANY, "Additional Options"), wxVERTICAL);
	wxBoxSizer *line1 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *line2 = new wxBoxSizer(wxHORIZONTAL);

	//compressed TIFF
	wxStaticText *tiffopts = new wxStaticText(panel, wxID_ANY, "TIFF Options:",
		wxDefaultPosition, wxSize(95, -1));
	wxCheckBox *ch1 = new wxCheckBox(panel, ID_LZW_COMP,
		"Lempel-Ziv-Welch Compression");
	ch1->Connect(ch1->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(VMovieView::OnCh1Check), NULL, panel);
	if (ch1)
		ch1->SetValue(VRenderFrame::GetCompression());
	wxCheckBox *ch2 = new wxCheckBox(panel, ID_SAVE_ALPHA,
		"Save alpha");
	ch2->Connect(ch2->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(VMovieView::OnCh2Check), NULL, panel);
	if (ch2)
		ch2->SetValue(VRenderFrame::GetSaveAlpha());
	wxCheckBox *ch3 = new wxCheckBox(panel, ID_SAVE_FLOAT,
		"Save float");
	ch3->Connect(ch3->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(VMovieView::OnCh3Check), NULL, panel);
	if (ch3)
		ch3->SetValue(VRenderFrame::GetSaveFloat());
	line1->Add(tiffopts, 0, wxALIGN_CENTER);
	line1->Add(ch1, 0, wxALIGN_CENTER);
	line1->Add(10, 10);
	line1->Add(ch2, 0, wxALIGN_CENTER);
	line1->Add(10, 10);
	line1->Add(ch3, 0, wxALIGN_CENTER);
	// movie quality
	//bitrate
	wxStaticText *MOVopts = new wxStaticText(panel, wxID_ANY, "MOV Options:",
		wxDefaultPosition, wxSize(95, -1));
	wxTextCtrl *bitrate_text = new wxTextCtrl(panel, wxID_ANY, "20.0",
		wxDefaultPosition, wxSize(50, -1));
	bitrate_text->Connect(bitrate_text->GetId(), wxEVT_TEXT,
		wxCommandEventHandler(VMovieView::OnMovieQuality), NULL, panel);
	wxStaticText *st = new wxStaticText(panel, wxID_ANY, "Bitrate:",
		wxDefaultPosition, wxSize(50, -1));
	wxStaticText *st2 = new wxStaticText(panel, wxID_ANY, "Mbps",
		wxDefaultPosition, wxSize(140, -1));
	wxStaticText *st3 = new wxStaticText(panel, wxID_ANY, "Estimated size:",
		wxDefaultPosition, wxSize(110, -1));
	m_estimated_size_text = new wxTextCtrl(panel, ID_BitrateText, "2.5",
		wxDefaultPosition, wxSize(50, -1));
	m_estimated_size_text->Disable();
	m_Mbitrate = STOD(bitrate_text->GetValue().fn_str());
	double size = m_Mbitrate * STOI(
		m_movie_time->GetValue().fn_str()) / 8.;
	m_estimated_size_text->SetValue(wxString::Format("%.2f", size));

	line2->Add(MOVopts, 0, wxALIGN_CENTER);
	line2->Add(st, 0, wxALIGN_CENTER);
	line2->Add(5, 5, wxALIGN_CENTER);
	line2->Add(bitrate_text, 0, wxALIGN_CENTER);
	line2->Add(5, 5, wxALIGN_CENTER);
	line2->Add(st2, 0, wxALIGN_CENTER);
	line2->AddStretchSpacer();
	line2->Add(st3, 0, wxALIGN_CENTER);
	line2->Add(m_estimated_size_text, 0, wxALIGN_CENTER);
	st2 = new wxStaticText(panel, wxID_ANY, "MB",
		wxDefaultPosition, wxSize(30, -1));
	line2->Add(5, 5, wxALIGN_CENTER);
	line2->Add(st2, 0, wxALIGN_CENTER);
	//copy all files check box
	wxCheckBox *ch_embed;
	if (VRenderFrame::GetSaveProject()) {
		ch_embed = new wxCheckBox(panel, ID_EMBED_FILES,
			"Embed all files in the project folder");
		ch_embed->Connect(ch_embed->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
			wxCommandEventHandler(VMovieView::OnChEmbedCheck), NULL, panel);
		ch_embed->SetValue(VRenderFrame::GetEmbedProject());
	}
	//group
	if (VRenderFrame::GetSaveProject() && ch_embed) {
		wxBoxSizer *line3 = new wxBoxSizer(wxHORIZONTAL);
		line3->Add(ch_embed, 0, wxALIGN_CENTER);
		group1->Add(line3);
		group1->Add(5, 5, wxALIGN_CENTER);
	}
	group1->Add(line1);
	group1->Add(5, 5, wxALIGN_CENTER);
	group1->Add(line2);
	wxStaticText *mov_note = new wxStaticText(panel, wxID_ANY,
		"NOTE: Please make sure that the movie length is greater than 36 frames for correct compression!",
		wxDefaultPosition, wxSize(-1, -1));
	group1->Add(mov_note);
	panel->SetSizer(group1);
	panel->Layout();

	return panel;
}
