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
#include "RecorderDlg.h"
#include <Renderview.hpp>
#include <Global.hpp>
#include <Root.hpp>
#include <tiffio.h>
#include <wx/aboutdlg.h>
#include <wx/valnum.h>
#include <wx/stdpaths.h>
#include "png_resource.h"
#include "img/icons.h"

BEGIN_EVENT_TABLE(VMovieView, wxPanel)
//time sequence
EVT_CHECKBOX(ID_SeqChk, VMovieView::OnSequenceChecked)
EVT_CHECKBOX(ID_BatChk, VMovieView::OnBatchChecked)
EVT_BUTTON(ID_IncTimeBtn, VMovieView::OnUpFrame)
EVT_BUTTON(ID_DecTimeBtn, VMovieView::OnDownFrame)
EVT_TEXT(ID_CurFrameText, VMovieView::OnCurFrameText)
EVT_TEXT(ID_StartFrameText, VMovieView::OnStartFrameText)
EVT_TEXT(ID_EndFrameText, VMovieView::OnEndFrameText)
EVT_TEXT(ID_MovieLenText, VMovieView::OnMovieLenText)
//rotations
EVT_CHECKBOX(ID_RotChk, VMovieView::OnRotateChecked)
EVT_RADIOBUTTON(ID_XRd, VMovieView::OnRotAxis)
EVT_RADIOBUTTON(ID_YRd, VMovieView::OnRotAxis)
EVT_RADIOBUTTON(ID_ZRd, VMovieView::OnRotAxis)
EVT_TEXT(ID_DegreeText, VMovieView::OnDegreeText)
EVT_COMBOBOX(ID_RotIntCmb, VMovieView::OnRotIntCmb)
//fps, view combo, help
EVT_TEXT(ID_FPS_Text, VMovieView::OnFpsEdit)
EVT_COMBOBOX(ID_ViewsCombo, VMovieView::OnViewSelected)
//main controls
EVT_BUTTON(ID_PlayPause, VMovieView::OnPrev)
EVT_BUTTON(ID_Rewind, VMovieView::OnRewind)
EVT_COMMAND_SCROLL(ID_ProgressSldr, VMovieView::OnTimeChange)
EVT_TEXT(ID_ProgressText, VMovieView::OnTimeText)
EVT_BUTTON(ID_SaveMovie, VMovieView::OnRun)
//script
EVT_CHECKBOX(ID_RunScriptChk, VMovieView::OnRunScriptChk)
EVT_TEXT(ID_ScriptFileText, VMovieView::OnScriptFileEdit)
EVT_BUTTON(ID_ScriptClearBtn, VMovieView::OnScriptClearBtn)
EVT_BUTTON(ID_ScriptFileBtn, VMovieView::OnScriptFileBtn)
EVT_LIST_ITEM_SELECTED(ID_ScriptList, VMovieView::OnScriptListSelected)
EVT_LIST_ITEM_ACTIVATED(ID_ScriptList, VMovieView::OnScriptListSelected)
//auto key
EVT_BUTTON(ID_GenKeyBtn, VMovieView::OnGenKey)
EVT_LIST_ITEM_ACTIVATED(ID_AutoKeyList, VMovieView::OnListItemAct)
//cropping
EVT_CHECKBOX(ID_CropChk, VMovieView::OnCropCheck)
EVT_BUTTON(ID_ResetBtn, VMovieView::OnResetCrop)
EVT_TEXT(ID_CenterXText, VMovieView::OnEditCrop)
EVT_TEXT(ID_CenterYText, VMovieView::OnEditCrop)
EVT_TEXT(ID_WidthText, VMovieView::OnEditCrop)
EVT_TEXT(ID_HeightText, VMovieView::OnEditCrop)
EVT_SPIN_UP(ID_CenterXSpin, VMovieView::OnCropSpinUp)
EVT_SPIN_UP(ID_CenterYSpin, VMovieView::OnCropSpinUp)
EVT_SPIN_UP(ID_WidthSpin, VMovieView::OnCropSpinUp)
EVT_SPIN_UP(ID_HeightSpin, VMovieView::OnCropSpinUp)
EVT_SPIN_DOWN(ID_CenterXSpin, VMovieView::OnCropSpinDown)
EVT_SPIN_DOWN(ID_CenterYSpin, VMovieView::OnCropSpinDown)
EVT_SPIN_DOWN(ID_WidthSpin, VMovieView::OnCropSpinDown)
EVT_SPIN_DOWN(ID_HeightSpin, VMovieView::OnCropSpinDown)
//timer
EVT_TIMER(ID_Timer, VMovieView::OnTimer)
//notebook
EVT_NOTEBOOK_PAGE_CHANGED(ID_Notebook, VMovieView::OnNbPageChange)
END_EVENT_TABLE()

double VMovieView::m_Mbitrate = 1;
double VMovieView::m_movie_len = 5;
wxTextCtrl * VMovieView::m_estimated_size_text = 0;

wxWindow* VMovieView::CreateSimplePage(wxWindow *parent)
{
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
	m_seq_chk = new wxCheckBox(page, ID_SeqChk, "Time Sequence");
	m_bat_chk = new wxCheckBox(page, ID_BatChk, "Batch Process");
	m_start_frame_text = new wxTextCtrl(page, ID_StartFrameText, "1",
		wxDefaultPosition, wxSize(40, -1));
	m_end_frame_text = new wxTextCtrl(page, ID_EndFrameText, "10",
		wxDefaultPosition, wxSize(40, -1));
	st2 = new wxStaticText(page, wxID_ANY, "End Frame: ");
	//sizer 1
	sizer_1->Add(m_seq_chk, 0, wxALIGN_CENTER);
	sizer_1->Add(m_bat_chk, 0, wxALIGN_CENTER);
	sizer_1->AddStretchSpacer();
	st = new wxStaticText(page, wxID_ANY, "Current Frame");
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	sizer_1->Add(20, 5, 0);
	//sizer 2
	st = new wxStaticText(page, wxID_ANY, "Start Frame: ");
	sizer_2->Add(5, 5, 0);
	sizer_2->Add(st, 0, wxALIGN_CENTER);
	sizer_2->Add(m_start_frame_text, 0, wxALIGN_CENTER);
	sizer_2->Add(5, 5, 0);
	sizer_2->Add(st2, 0, wxALIGN_CENTER);
	sizer_2->Add(m_end_frame_text, 0, wxALIGN_CENTER);
	m_inc_time_btn = new wxButton(page, ID_IncTimeBtn, "",
		wxDefaultPosition, wxSize(30, 30));
	m_dec_time_btn = new wxButton(page, ID_DecTimeBtn, "",
		wxDefaultPosition, wxSize(30, 30));
	m_cur_frame_text = new wxTextCtrl(page, ID_CurFrameText, "0",
		wxDefaultPosition, wxSize(40, -1));
	m_inc_time_btn->SetBitmap(wxGetBitmapFromMemory(plus));
	m_dec_time_btn->SetBitmap(wxGetBitmapFromMemory(minus));
	sizer_2->AddStretchSpacer();
	sizer_2->Add(m_dec_time_btn, 0, wxALIGN_CENTER);
	sizer_2->Add(5, 15, 0);
	sizer_2->Add(m_cur_frame_text, 0, wxALIGN_CENTER);
	sizer_2->Add(5, 15, 0);
	sizer_2->Add(m_inc_time_btn, 0, wxALIGN_CENTER);
	sizer_2->Add(5, 15, 0);
	//rotations
	m_rot_chk = new wxCheckBox(page, ID_RotChk, "Rotation");
	//axis
	m_x_rd = new wxRadioButton(page, ID_XRd, "X",
		wxDefaultPosition, wxSize(30, 22));
	m_y_rd = new wxRadioButton(page, ID_YRd, "Y",
		wxDefaultPosition, wxSize(30, 22));
	m_z_rd = new wxRadioButton(page, ID_ZRd, "Z",
		wxDefaultPosition, wxSize(30, 22));
	m_y_rd->SetValue(true);
	//degrees
	m_degree_text = new wxTextCtrl(page, ID_DegreeText, "360",
		wxDefaultPosition, wxSize(40, -1));
	st2 = new wxStaticText(page, wxID_ANY, "Deg.");
	//sizer3
	sizer_3->Add(m_rot_chk, 0, wxALIGN_CENTER);
	//sizer 4
	sizer_4->Add(20, 5, 0);
	sizer_4->Add(m_x_rd, 0, wxALIGN_CENTER);
	//sizer 5
	sizer_5->Add(20, 5, 0);
	sizer_5->Add(m_y_rd, 0, wxALIGN_CENTER);
	sizer_5->Add(20, 5, 0);
	sizer_5->Add(m_degree_text, 0, wxALIGN_CENTER);
	sizer_5->Add(5, 5, 0);
	sizer_5->Add(st2, 0, wxALIGN_CENTER);
	//rotation interpolation
	st2 = new wxStaticText(page, wxID_ANY, "Interpolation");
	m_rot_int_cmb = new wxComboBox(page, ID_RotIntCmb, "",
		wxDefaultPosition, wxSize(65, -1), 0, NULL, wxCB_READONLY);
	m_rot_int_cmb->Append("Linear");
	m_rot_int_cmb->Append("Smooth");
	m_rot_int_cmb->Select(0);
	sizer_5->AddStretchSpacer();
	sizer_5->Add(m_rot_int_cmb, 0, wxALIGN_CENTER);
	sizer_5->Add(5, 5);
	sizer_5->Add(st2, 0, wxALIGN_CENTER);
	sizer_5->Add(5, 5);
	//sizer 6
	sizer_6->Add(20, 5, 0);
	sizer_6->Add(m_z_rd, 0, wxALIGN_CENTER);
	//movie length
	st = new wxStaticText(page, wxID_ANY, "Movie Time: ");
	st2 = new wxStaticText(page, wxID_ANY, "Sec.");
	m_movie_len_text = new wxTextCtrl(page, ID_MovieLenText, "5.00",
		wxDefaultPosition, wxSize(50, -1));
	//sizer 7
	sizer_7->AddStretchSpacer();
	sizer_7->Add(st, 0, wxALIGN_CENTER);
	sizer_7->Add(m_movie_len_text, 0, wxALIGN_CENTER);
	sizer_7->Add(5, 5, 0);
	sizer_7->Add(st2, 0, wxALIGN_CENTER);
	sizer_7->Add(5, 5, 0);
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

wxWindow* VMovieView::CreateAdvancedPage(wxWindow *parent)
{
	wxPanel *page = new wxPanel(parent);
	//vertical sizer
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
	m_advanced_movie = new RecorderDlg(m_frame, page);
	m_frame->m_recorder_dlg = m_advanced_movie;
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
	m_crop_chk = new wxCheckBox(page, ID_CropChk, "");
	m_reset_btn = new wxButton(page, ID_ResetBtn, "Reset",
		wxDefaultPosition, wxSize(110, 30));
	m_reset_btn->SetBitmap(wxGetBitmapFromMemory(reset));
	sizer_8->Add(5, 5, 0);
	sizer_8->Add(st, 0, wxALIGN_CENTER);
	sizer_8->Add(m_crop_chk, 0, wxALIGN_CENTER);
	sizer_8->Add(100, 5, 0);
	sizer_8->Add(m_reset_btn, 0, wxALIGN_CENTER);
	//9th line
	st = new wxStaticText(page, 0, "Center:  X:",
		wxDefaultPosition, wxSize(85, 20));
	m_center_x_text = new wxTextCtrl(page, ID_CenterXText, "",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	m_center_x_spin = new wxSpinButton(page, ID_CenterXSpin,
		wxDefaultPosition, wxSize(20, 20));
	m_center_x_spin->SetRange(-0x8000, 0x7fff);
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
	m_center_y_spin->SetRange(-0x8000, 0x7fff);
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
	m_width_spin->SetRange(-0x8000, 0x7fff);
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
	return page;

}

wxWindow* VMovieView::CreateScriptPage(wxWindow *parent)
{
	wxPanel *page = new wxPanel(parent);
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

VMovieView::VMovieView(VRenderFrame* frame,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name) :
	wxPanel(frame, wxID_ANY, pos, size, style, name),
	m_frame(frame),
	m_view_idx(0),
	m_rotate(true),
	m_time_seq(false),
	m_rot_axis(1),
	m_rot_deg(360),
	m_start_frame(0),
	m_cur_frame(0),
	m_starting_rot(0.),
	m_cur_time(0.0),
	m_running(false),
	m_record(false),
	m_current_page(0),
	m_rot_int_type(0),
	m_delayed_stop(false),
	m_seq_mode(0),
	m_fps(30),
	m_timer(this, ID_Timer),
	m_crop(false)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);

	m_view = glbin_root->getChild(m_view_idx)->asRenderview();

	//notebook
	m_notebook = new wxNotebook(this, ID_Notebook);
	m_notebook->AddPage(CreateSimplePage(m_notebook), "Basic");
	m_notebook->AddPage(CreateAdvancedPage(m_notebook), "Advanced");
	m_notebook->AddPage(CreateAutoKeyPage(m_notebook), "Auto Key");
	m_notebook->AddPage(CreateCroppingPage(m_notebook), "Cropping");
	m_notebook->AddPage(CreateScriptPage(m_notebook), "Script");
	//renderview selector
	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	//FPS
	m_fps_text = new wxTextCtrl(this, ID_FPS_Text, "30",
		wxDefaultPosition, wxSize(50, -1));
	//other
	m_views_cmb = new wxComboBox(this, ID_ViewsCombo, "",
		wxDefaultPosition, wxSize(120, -1), 0, NULL, wxCB_READONLY);
	//sizer 1
	sizer_1->Add(5, 5, 0);
	wxStaticText * st = new wxStaticText(this, wxID_ANY, "FPS: ");
	wxStaticText * st2 = new wxStaticText(this, wxID_ANY, "Capture: ");
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	sizer_1->Add(m_fps_text, 0, wxALIGN_CENTER);
	sizer_1->AddStretchSpacer();
	sizer_1->Add(st2, 0, wxALIGN_CENTER);
	sizer_1->Add(m_views_cmb, 0, wxALIGN_CENTER);

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
	m_progress_sldr = new wxSlider(this, ID_ProgressSldr, 0, 0, PROG_SLDR_MAX);
	sizerH->Add(m_progress_sldr, 1, wxEXPAND);
	m_progress_text = new wxTextCtrl(this, ID_ProgressText, "0.00",
		wxDefaultPosition, wxSize(50, -1));
	sizerH->Add(m_progress_text, 0, wxEXPAND);
	wxStaticText * st3 = new wxStaticText(this, 0, "Sec.");
	sizerH->Add(5, 5, 0);
	sizerH->Add(st3, 0, wxALIGN_CENTER);
	sizerH->Add(5, 5);
	m_save_btn = new wxButton(this, ID_SaveMovie, "",
		wxDefaultPosition, wxSize(30, 30));
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
	m_view_idx = m_views_cmb->GetCurrentSelection();
	m_view = glbin_root->getChild(m_view_idx)->asRenderview();;
	GetSettings();
}

void VMovieView::GetSettings()
{
	if (!m_view) return;

	SetTimeSeq(m_time_seq);
	SetRotate(m_rotate);
	SetRotAxis(m_rot_axis);
	SetRotDeg(m_rot_deg);
	SetCurrentTime(m_start_frame);
	bool bval;
	m_view->getValue(gstDrawCropFrame, bval);
	SetCrop(bval);
	AddScriptToList();
	GetScriptSettings();
	if (m_advanced_movie)
		m_advanced_movie->GetSettings(m_view);
}

void VMovieView::GetScriptSettings()
{
	//script
	if (!m_view || !m_frame || !m_frame->GetSettingDlg())
		return;

	bool run_script = m_frame->GetSettingDlg()->GetRunScript();
	m_view->setValue(gstRunScript, run_script);
	m_run_script_chk->SetValue(run_script);
	wxString script_file =
		m_frame->GetSettingDlg()->GetScriptFile();
	m_script_file_text->SetValue(script_file);
	m_view->setValue(gstScriptFile, script_file.ToStdWstring());
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
	list.Sort();
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

void VMovieView::Init()
{
	if (!m_frame) return;
	m_views_cmb->Clear();
	for (size_t i = 0; i < glbin_root->getNumChildren(); ++i)
	{
		fluo::Renderview* view = glbin_root->getChild(i)->asRenderview();
		if (view)
			m_views_cmb->Append(view->getName());
	}
	if (glbin_root->getNumChildren() > 0)
		m_views_cmb->Select(0);
	GetSettings();
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

void VMovieView::AddView(wxString view)
{
	if (m_views_cmb)
		m_views_cmb->Append(view);
}

void VMovieView::DeleteView(wxString view)
{
	if (!m_views_cmb)
		return;
	int cur_sel = m_views_cmb->GetCurrentSelection();
	int del = m_views_cmb->FindString(view, true);
	if (del != wxNOT_FOUND)
		m_views_cmb->Delete(del);
	if (cur_sel == del)
		m_views_cmb->Select(0);
}

void VMovieView::SetView(int index)
{
	if (m_views_cmb)
		m_views_cmb->SetSelection(index);
}

void VMovieView::SetTimeSeq(bool value)
{
	m_time_seq = value;
	if (m_time_seq)
	{
		if (!m_seq_mode)
			m_seq_mode = 1;

		if (m_seq_mode == 1)
		{
			m_seq_chk->SetValue(true);
			m_bat_chk->SetValue(false);
			if (m_view)
				m_view->Get4DSeqRange(m_start_frame, m_end_frame);
		}
		else if (m_seq_mode == 2)
		{
			m_seq_chk->SetValue(false);
			m_bat_chk->SetValue(true);
			if (m_view)
				m_view->Get3DBatRange(m_start_frame, m_end_frame);
		}
	}
	else
	{
		m_seq_chk->SetValue(false);
		m_bat_chk->SetValue(false);
		m_start_frame = 0;
		m_end_frame = m_rot_deg;
	}
	m_movie_len = (m_end_frame - m_start_frame + 1) / m_fps;
	m_start_frame_text->ChangeValue(wxString::Format("%d", m_start_frame));
	m_end_frame_text->ChangeValue(wxString::Format("%d", m_end_frame));
	m_movie_len_text->ChangeValue(wxString::Format("%.2f", m_movie_len));
}

void VMovieView::SetCrop(bool value)
{
	m_crop = value;
	m_crop_chk->SetValue(m_crop);

	if (!m_view)
		return;
	if (m_crop)
	{
		m_view->CalculateCrop();
		m_view->getValue(gstCropX, m_crop_x); m_view->getValue(gstCropY, m_crop_y);
		m_view->getValue(gstCropW, m_crop_w); m_view->getValue(gstCropH, m_crop_h);
		m_crop_x = long(m_crop_x + m_crop_w / 2.0 + 0.5);
		m_crop_y = long(m_crop_y + m_crop_h / 2.0 + 0.5);
		m_center_x_text->ChangeValue(wxString::Format("%d", m_crop_x));
		m_center_y_text->ChangeValue(wxString::Format("%d", m_crop_y));
		m_width_text->ChangeValue(wxString::Format("%d", m_crop_w));
		m_height_text->ChangeValue(wxString::Format("%d", m_crop_h));
		m_view->setValue(gstDrawCropFrame, true);
	}
	else
		m_view->setValue(gstDrawCropFrame, false);

	m_view->RefreshGL(39);
}

void VMovieView::UpdateCrop()
{
	if (!m_view)
		return;

	m_view->setValue(gstCropX, long(m_crop_x - m_crop_w / 2.0 + 0.5));
	m_view->setValue(gstCropY, long(m_crop_y - m_crop_h / 2.0 + 0.5));
	m_view->setValue(gstCropW, m_crop_w);
	m_view->setValue(gstCropH, m_crop_h);
	if (m_crop)
		m_view->RefreshGL(39);
}

void VMovieView::OnTimer(wxTimerEvent& event)
{
	//get all of the progress info
	if (m_delayed_stop)
	{
		if (m_record)
			WriteFrameToFile(int(m_fps*m_movie_len + 0.5));
		m_delayed_stop = false;
		Stop();
		if (m_view)
			m_view->setValue(gstKeepEnlarge, false);
		return;
	}

	if (flvr::TextureRenderer::get_mem_swap() &&
		flvr::TextureRenderer::get_start_update_loop() &&
		!flvr::TextureRenderer::get_done_update_loop())
	{
		if (!m_view) return;
		m_view->setValue(gstInteractive, false);
		m_view->RefreshGL(39, false, false);
		return;
	}

	//move forward in time (limits FPS usability to 100 FPS)
	m_cur_time += 1.0 / m_fps;
	//frame only increments when time passes a whole number
	int time = m_end_frame - m_start_frame + 1;
	m_cur_frame = (int)(m_start_frame + time * m_cur_time / m_movie_len + 0.5);
	double pcnt = (double)(m_cur_frame - m_start_frame) / (double)time;
	SetProgress(pcnt);
	//update the rendering frame since we have advanced.
	if (m_last_frame != m_cur_frame)
	{
		m_cur_frame_text->ChangeValue(wxString::Format("%d", m_cur_frame));
		if (m_record)
			WriteFrameToFile(int(m_fps*m_movie_len + 0.5));
		m_last_frame = m_cur_frame;
		SetRendering(m_cur_time / m_movie_len);
	}
	if (m_movie_len - m_cur_time < 0.1 / m_fps ||
		m_cur_time > m_movie_len)
		m_delayed_stop = true;
}

void VMovieView::OnNbPageChange(wxBookCtrlEvent& event)
{
	int sel = event.GetSelection();
	int old_sel = event.GetOldSelection();
	if (sel <=1)
		m_current_page = sel;
}

void VMovieView::Prev()
{
	if (m_running)
	{
		Stop();
		return;
	}
	m_running = true;
	flvr::TextureRenderer::maximize_uptime_ = true;
	m_play_btn->SetBitmap(wxGetBitmapFromMemory(pause));
	int slider_pos = m_progress_sldr->GetValue();
	if (slider_pos < PROG_SLDR_MAX && slider_pos > 0 &&
		!(m_movie_len - m_cur_time < 0.1 / m_fps ||
			m_cur_time > m_movie_len))
	{
		TimerRun();
		return;
	}

	if (!m_frame || !m_view)
		return;
	//basic options
	double rval[3];
	m_view->getValue(gstCamRotX, rval[0]);
	m_view->getValue(gstCamRotY, rval[1]);
	m_view->getValue(gstCamRotZ, rval[2]);
	m_starting_rot = rval[m_rot_axis];
	while (m_starting_rot > 360.) m_starting_rot -= 360.;
	while (m_starting_rot < -360.) m_starting_rot += 360.;
	if (360. - std::abs(m_starting_rot) < 0.001)
		m_starting_rot = 0.;
	if (m_current_page == 1)
	{
		Interpolator *interpolator = m_frame->GetInterpolator();
		if (interpolator && interpolator->GetLastIndex() > 0)
		{
			int frames = int(interpolator->GetLastT());
			m_movie_len = (double)frames / m_fps;
			m_movie_len_text->ChangeValue(wxString::Format("%.2f", m_movie_len));
		}
	}
	SetProgress(0.);
	SetRendering(0.);
	m_last_frame = 0;
	TimerRun();
}

void VMovieView::OnPrev(wxCommandEvent& event)
{
	Prev();
}

void VMovieView::OnRun(wxCommandEvent& event)
{
	wxFileDialog *fopendlg = new wxFileDialog(
		m_frame, "Save Movie Sequence",
		"", "output", "MOV file (*.mov)|*.mov|TIF files (*.tif)|*.tif",
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	fopendlg->SetExtraControlCreator(CreateExtraCaptureControl);
	fopendlg->CenterOnParent();
	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		m_filename = fopendlg->GetPath();
		if (m_view)
			m_view->setValue(gstKeepEnlarge, true);
		Run();
	}
	delete fopendlg;
}

void VMovieView::Stop()
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
	flvr::TextureRenderer::maximize_uptime_ = false;
}

void VMovieView::OnStop(wxCommandEvent& event)
{
	Stop();
}

void VMovieView::Rewind()
{
	if (!m_view)
		return;
	m_view->SetParams(0.);
	Stop();
	m_cur_frame = m_start_frame;
	m_cur_frame_text->ChangeValue(wxString::Format("%d", m_cur_frame));
	SetProgress(0.);
	SetRendering(0., false);
}

void VMovieView::OnRewind(wxCommandEvent& event)
{
	Rewind();
}

void VMovieView::OnCropCheck(wxCommandEvent& event)
{
	SetCrop(m_crop_chk->GetValue());
}

void VMovieView::OnResetCrop(wxCommandEvent& event)
{
	SetCrop(true);
}

void VMovieView::OnEditCrop(wxCommandEvent& event)
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

void VMovieView::OnCropSpinUp(wxSpinEvent& event)
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

void VMovieView::OnCropSpinDown(wxSpinEvent& event)
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

void VMovieView::UpFrame()
{
	if (m_running) return;
	if (m_cur_frame < m_start_frame)
		m_cur_frame = m_start_frame;
	m_cur_frame++;
	bool rewind = false;
	if (m_cur_frame > m_end_frame)
	{
		m_cur_frame = m_start_frame;
		rewind = true;
	}
	int time = m_end_frame - m_start_frame + 1;
	double pcnt = (double)(m_cur_frame - m_start_frame) / (double)time;
	m_cur_frame_text->ChangeValue(wxString::Format("%d", m_cur_frame));
	SetProgress(pcnt);
	SetRendering(pcnt, rewind);
}

void VMovieView::DownFrame()
{
	if (m_running) return;
	if (m_cur_frame > m_end_frame)
		m_cur_frame = m_end_frame;
	m_cur_frame--;
	if (m_cur_frame < m_start_frame) m_cur_frame = m_end_frame;
	int time = m_end_frame - m_start_frame + 1;
	double pcnt = (double)(m_cur_frame - m_start_frame) / (double)time;
	m_cur_frame_text->ChangeValue(wxString::Format("%d", m_cur_frame));
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
	if (!m_frame || !m_view)
		return;
	bool run_script = m_run_script_chk->GetValue();
	if (m_frame->GetSettingDlg())
		m_frame->GetSettingDlg()->SetRunScript(run_script);
	m_view->setValue(gstRunScript, run_script);
	wxString str = m_script_file_text->GetValue();
	if (!str.IsEmpty())
	{
		m_frame->GetSettingDlg()->SetScriptFile(str);
		m_view->setValue(gstScriptFile, str.ToStdWstring());
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

void VMovieView::OnScriptFileEdit(wxCommandEvent &event)
{
	if (!m_frame || !m_frame->GetSettingDlg())
		return;

	wxString str = m_script_file_text->GetValue();
	m_frame->GetSettingDlg()->SetScriptFile(str);
	m_view->setValue(gstScriptFile, str.ToStdWstring());
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
		if (m_frame && m_frame->GetSettingDlg())
			m_frame->GetSettingDlg()->SetScriptFile(file);
		m_view->setValue(gstScriptFile, file.ToStdWstring());
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

		//enable script if not
		if (!m_run_script_chk->GetValue())
		{
			m_run_script_chk->SetValue(true);
			OnRunScriptChk(event);
		}
	}
}

//auto key
void VMovieView::OnGenKey(wxCommandEvent& event) {
	GenKey();
}

void VMovieView::OnTimeChange(wxScrollEvent &event)
{
	if (m_running) return;
	int prg = event.GetPosition();
	double pcnt = (double)prg / PROG_SLDR_MAX;
	m_cur_time = pcnt * m_movie_len;
	wxString str = wxString::Format("%.2f", m_cur_time);
	if (str != m_progress_text->GetValue())
		m_progress_text->SetValue(str);
}

void VMovieView::OnTimeText(wxCommandEvent& event)
{
	if (m_running) return;
	wxString str = m_progress_text->GetValue();
	if (!str.ToDouble(&m_cur_time))
		m_cur_time = 0;
	double pcnt = (m_cur_time / m_movie_len);
	m_progress_sldr->SetValue(PROG_SLDR_MAX * pcnt);
	int time = m_end_frame - m_start_frame + 1;
	m_cur_frame = (int)(m_start_frame + time * pcnt);
	m_cur_frame_text->ChangeValue(wxString::Format("%d", m_cur_frame));

	SetRendering(pcnt);
}

void VMovieView::SetRendering(double pcnt, bool rewind)
{
	if (!m_frame || !m_view)
		return;

	m_view->setValue(gstCamLockObjEnable, false);
	//advanced options
	if (m_current_page == 1)
	{
		Interpolator *interpolator = m_frame->GetInterpolator();
		if (interpolator && interpolator->GetLastIndex() > 0)
		{
			if (m_advanced_movie->GetCamLock() && m_timer.IsRunning())
				m_view->setValue(gstCamLockObjEnable, true);
			int end_frame = int(interpolator->GetLastT());
			m_view->SetParams(pcnt * end_frame);
			m_view->setValue(gstInteractive, false);
			m_view->RefreshGL(39);
			return;
		}
	}
	//basic options
	int time = m_end_frame - m_start_frame + 1;
	time = int(m_start_frame + time * pcnt + 0.5);

	if (m_seq_mode == 1)
	{
		m_view->Set4DSeqFrame(time, m_start_frame, m_end_frame, rewind);
	}
	else if (m_seq_mode == 2)
	{
		m_view->Set3DBatFrame(time, m_start_frame, m_end_frame, rewind);
	}

	//rotate animation
	if (m_rotate)
	{
		std::string sval;
		switch (m_rot_axis)
		{
		case 0:
			sval = gstCamRotX;
			break;
		case 1:
			sval = gstCamRotY;
			break;
		case 2:
			sval = gstCamRotZ;
			break;
		}
		double dval;
		m_view->getValue(sval, dval);
		if (m_rot_int_type == 0)
			dval = m_starting_rot + pcnt * m_rot_deg;
		else if (m_rot_int_type == 1)
			dval = m_starting_rot +
			(-2.0*pcnt*pcnt*pcnt + 3.0*pcnt*pcnt) * m_rot_deg;
		m_view->setValue(sval, dval);
	}

	m_view->setValue(gstInteractive, false);
	m_view->RefreshGL(39);
}

void VMovieView::OnRotateChecked(wxCommandEvent& event)
{
	m_rotate = m_rot_chk->GetValue();
	if (m_rotate)
	{
		m_x_rd->Enable();
		m_y_rd->Enable();
		m_z_rd->Enable();
		m_degree_text->Enable();
		m_rot_int_cmb->Enable();
	}
	else
	{
		m_x_rd->Disable();
		m_y_rd->Disable();
		m_z_rd->Disable();
		m_degree_text->Disable();
		m_rot_int_cmb->Disable();
		//enable time seq if not
		if (m_seq_mode == 0)
			SetTimeSeq(true);
	}
}

void VMovieView::OnRotAxis(wxCommandEvent& event)
{
	if (m_x_rd->GetValue())
		m_rot_axis = 0;
	else if (m_y_rd->GetValue())
		m_rot_axis = 1;
	else if (m_z_rd->GetValue())
		m_rot_axis = 2;
}

void VMovieView::OnDegreeText(wxCommandEvent &event)
{
	wxString str = m_degree_text->GetValue();
	long ival = 0;
	str.ToLong(&ival);
	m_rot_deg = ival;
}

void VMovieView::OnRotIntCmb(wxCommandEvent& event)
{
	m_rot_int_type = m_rot_int_cmb->GetCurrentSelection();
}

void VMovieView::OnSequenceChecked(wxCommandEvent& event)
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

void VMovieView::OnBatchChecked(wxCommandEvent& event)
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

void VMovieView::SetProgress(double pcnt)
{
	pcnt = std::abs(pcnt);
	m_progress_sldr->SetValue(pcnt * PROG_SLDR_MAX);
	m_cur_time = pcnt*m_movie_len;
	wxString st = wxString::Format("%.2f", m_cur_time);
	m_progress_text->ChangeValue(st);
}

void VMovieView::WriteFrameToFile(int total_frames)
{
	if (!m_view)
		return;

	wxString s_length = wxString::Format("%d", total_frames);
	int length = s_length.Length();
	wxString format = wxString::Format("_%%0%dd", length);
	wxString outputfilename = wxString::Format("%s" + format + "%s", m_filename,
		m_last_frame, ".tif");

	//capture
	bool bmov = filetype_.IsSameAs(".mov");
	int chann = VRenderFrame::GetSaveAlpha() ? 4 : 3;
	bool fp32 = bmov?false:VRenderFrame::GetSaveFloat();
	long x, y, w, h;
	void* image = 0;
	m_view->ReadPixels(chann, fp32, x, y, w, h, &image);

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

void VMovieView::OnUpFrame(wxCommandEvent& event)
{
	UpFrame();
}

void VMovieView::OnDownFrame(wxCommandEvent& event)
{
	DownFrame();
}

void VMovieView::SetCurrentTime(size_t t)
{
	m_cur_frame = t;
	m_cur_frame_text->ChangeValue(wxString::Format("%d", m_cur_frame));
	if (m_cur_frame < m_start_frame) m_cur_frame = m_end_frame;
	if (m_cur_frame > m_end_frame) m_cur_frame = m_start_frame;
	int time = m_end_frame - m_start_frame + 1;
	double pcnt = (double)(m_cur_frame - m_start_frame) / (double)time;
	SetProgress(pcnt);
}

void VMovieView::Run()
{
	if (!m_frame || !m_view)
		return;

	if (m_frame->GetSettingDlg())
	{
		VRenderFrame::SetSaveProject(m_frame->GetSettingDlg()->GetProjSave());
		VRenderFrame::SetSaveAlpha(m_frame->GetSettingDlg()->GetSaveAlpha());
		VRenderFrame::SetSaveFloat(m_frame->GetSettingDlg()->GetSaveFloat());
	}

	Rewind();

	filetype_ = m_filename.SubString(m_filename.Len() - 4,
		m_filename.Len() - 1);
	if (filetype_.IsSameAs(wxString(".mov")))
	{
		if (!m_crop)
		{
			m_crop_x = 0;
			m_crop_y = 0;
			m_view->getValue(gstSizeX, m_crop_w);
			m_view->getValue(gstSizeY, m_crop_h);
		}
		bool bval = false;
		if (m_view)
			m_view->getValue(gstEnlarge, bval);
		if (bval)
		{
			double scale;
			m_view->getValue(gstEnlargeScale, scale);
			m_crop_w *= scale;
			m_crop_h *= scale;
		}

		encoder_.open(m_filename.ToStdString(), m_crop_w, m_crop_h, m_fps,
			m_Mbitrate * 1e6);
	}
	m_filename = m_filename.SubString(0, m_filename.Len() - 5);
	m_record = true;
	if (m_frame->GetSettingDlg())
	{
		m_frame->GetSettingDlg()->SetSaveAlpha(VRenderFrame::GetSaveAlpha());
		m_frame->GetSettingDlg()->SetSaveFloat(VRenderFrame::GetSaveFloat());
		if (m_frame->GetSettingDlg()->GetProjSave())
		{
			wxString new_folder;
			new_folder = m_filename + "_project";
			MkDirW(new_folder.ToStdWstring());
			wstring name = m_filename.ToStdWstring();
			name = GET_NAME(name);
			wxString prop_file = new_folder + GETSLASH()
				+ name + "_project.vrp";
			m_frame->SaveProject(prop_file);
		}
	}

	Prev();
}

void VMovieView::OnCurFrameText(wxCommandEvent& event)
{
	if (m_running) return;
	m_cur_frame = STOI(m_cur_frame_text->GetValue().fn_str());
	if (m_cur_frame < m_start_frame) m_cur_frame = m_end_frame;
	if (m_cur_frame > m_end_frame) m_cur_frame = m_start_frame;
	int time = m_end_frame - m_start_frame + 1;
	double pcnt = (double)(m_cur_frame - m_start_frame) / (double)time;
	SetProgress(pcnt);
	SetRendering(pcnt);
}

void VMovieView::OnStartFrameText(wxCommandEvent& event)
{
	wxString str = m_start_frame_text->GetValue();
	long lval;
	if (str.ToLong(&lval))
		m_start_frame = lval;
	OnFpsEdit(event);
}

void VMovieView::OnEndFrameText(wxCommandEvent& event)
{
	wxString str = m_end_frame_text->GetValue();
	long lval;
	if (str.ToLong(&lval))
		m_end_frame = lval;
	OnFpsEdit(event);
}

void VMovieView::OnMovieLenText(wxCommandEvent& event)
{
	wxString str = m_movie_len_text->GetValue();
	str.ToDouble(&m_movie_len);

	//update fps
	m_fps = double(m_end_frame - m_start_frame + 1) / m_movie_len;
	m_fps_text->ChangeValue(wxString::Format("%.0f", m_fps));
}

void VMovieView::OnFpsEdit(wxCommandEvent& event)
{
	m_fps_text->GetValue().ToDouble(&m_fps);

	//update length
	m_movie_len = double(m_end_frame - m_start_frame + 1) / m_fps;
	m_movie_len_text->ChangeValue(wxString::Format("%.2f", m_movie_len));
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
//enlarge output image
void VMovieView::OnChEnlargeCheck(wxCommandEvent &event)
{
	wxCheckBox* ch_enlarge = (wxCheckBox*)event.GetEventObject();
	if (ch_enlarge)
	{
		bool enlarge = ch_enlarge->GetValue();
		if (m_view)
			m_view->setValue(gstEnlarge, enlarge);
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

void VMovieView::OnSlEnlargeScroll(wxScrollEvent &event)
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
			tx_enlarge->SetValue(str);
		}
	}
}

void VMovieView::OnTxEnlargeText(wxCommandEvent &event)
{
	wxString str = event.GetString();
	double dval;
	str.ToDouble(&dval);
	if (m_view)
		m_view->setValue(gstEnlargeScale, dval);
	int ival = int(dval * 10 + 0.5);
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
void VMovieView::OnMovieQuality(wxCommandEvent &event)
{
	m_Mbitrate = STOD(((wxTextCtrl*)event.GetEventObject())
		->GetValue().fn_str());
	double size = m_Mbitrate * m_movie_len / 8.;
	m_estimated_size_text->SetValue(wxString::Format("%.2f", size));
}
//embed project
void VMovieView::OnChEmbedCheck(wxCommandEvent &event) {
	wxCheckBox* ch_embed = (wxCheckBox*)event.GetEventObject();
	if (ch_embed)
		VRenderFrame::SetEmbedProject(ch_embed->GetValue());
}

wxWindow* VMovieView::CreateExtraCaptureControl(wxWindow* parent)
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
		"Save float channel");
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

	//enlarge
	wxCheckBox* ch_enlarge = new wxCheckBox(panel, ID_ENLARGE_CHK,
		"Enlarge output image");
	ch_enlarge->Connect(ch_enlarge->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(VMovieView::OnChEnlargeCheck), NULL, panel);
	wxSlider* sl_enlarge = new wxSlider(panel, ID_ENLARGE_SLDR,
		10, 10, 100);
	sl_enlarge->Connect(sl_enlarge->GetId(), wxEVT_COMMAND_SLIDER_UPDATED,
		wxScrollEventHandler(VMovieView::OnSlEnlargeScroll), NULL, panel);
	sl_enlarge->Disable();
	wxFloatingPointValidator<double> vald_fp(1);
	wxTextCtrl* tx_enlarge = new wxTextCtrl(panel, ID_ENLARGE_TEXT,
		"1.0", wxDefaultPosition, wxDefaultSize, 0, vald_fp);
	tx_enlarge->Connect(tx_enlarge->GetId(), wxEVT_COMMAND_TEXT_UPDATED,
		wxCommandEventHandler(VMovieView::OnTxEnlargeText), NULL, panel);
	tx_enlarge->Disable();
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
		wxCommandEventHandler(VMovieView::OnMovieQuality), NULL, panel);
	wxStaticText *st = new wxStaticText(panel, wxID_ANY, "Bitrate:",
		wxDefaultPosition, wxDefaultSize);
	wxStaticText *st2 = new wxStaticText(panel, wxID_ANY, "Mbps",
		wxDefaultPosition, wxDefaultSize);
	wxStaticText *st3 = new wxStaticText(panel, wxID_ANY, "Estimated size:",
		wxDefaultPosition, wxDefaultSize);
	m_estimated_size_text = new wxTextCtrl(panel, ID_BitrateText, "2.5",
		wxDefaultPosition, wxDefaultSize);
	m_estimated_size_text->Disable();
	m_Mbitrate = STOD(bitrate_text->GetValue().fn_str());
	double size = m_Mbitrate * m_movie_len / 8.;
	m_estimated_size_text->SetValue(wxString::Format("%.2f", size));

	line3->Add(MOVopts, 0, wxALIGN_CENTER);
	line3->Add(st, 0, wxALIGN_CENTER);
	line3->Add(5, 5, wxALIGN_CENTER);
	line3->Add(bitrate_text, 0, wxALIGN_CENTER);
	line3->Add(5, 5, wxALIGN_CENTER);
	line3->Add(st2, 0, wxALIGN_CENTER);
	line3->AddStretchSpacer();
	line3->Add(st3, 0, wxALIGN_CENTER);
	line3->Add(m_estimated_size_text, 0, wxALIGN_CENTER);
	st2 = new wxStaticText(panel, wxID_ANY, "MB",
		wxDefaultPosition, wxDefaultSize);
	line3->Add(5, 5, wxALIGN_CENTER);
	line3->Add(st2, 0, wxALIGN_CENTER);
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
