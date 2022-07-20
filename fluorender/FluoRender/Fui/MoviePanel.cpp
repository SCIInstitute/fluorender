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
#include <MoviePanel.h>
#include <RenderFrame.h>
#include <RecorderDlg.h>
#include <png_resource.h>
#include <img/icons.h>
#include <wx/aboutdlg.h>
#include <wx/valnum.h>
#include <wx/stdpaths.h>

BEGIN_EVENT_TABLE(MoviePanel, wxPanel)
//time sequence
EVT_CHECKBOX(ID_SeqChk, MoviePanel::OnSequenceChecked)
EVT_CHECKBOX(ID_BatChk, MoviePanel::OnBatchChecked)
EVT_BUTTON(ID_IncTimeBtn, MoviePanel::OnUpFrame)
EVT_BUTTON(ID_DecTimeBtn, MoviePanel::OnDownFrame)
EVT_TEXT(ID_CurFrameText, MoviePanel::OnCurFrameText)
EVT_TEXT(ID_StartFrameText, MoviePanel::OnStartFrameText)
EVT_TEXT(ID_EndFrameText, MoviePanel::OnEndFrameText)
EVT_TEXT(ID_MovieLenText, MoviePanel::OnMovieLenText)
//rotations
EVT_CHECKBOX(ID_RotChk, MoviePanel::OnRotateChecked)
EVT_RADIOBUTTON(ID_XRd, MoviePanel::OnRotAxis)
EVT_RADIOBUTTON(ID_YRd, MoviePanel::OnRotAxis)
EVT_RADIOBUTTON(ID_ZRd, MoviePanel::OnRotAxis)
EVT_TEXT(ID_DegreeText, MoviePanel::OnDegreeText)
EVT_COMBOBOX(ID_RotIntCmb, MoviePanel::OnRotIntCmb)
//fps, view combo, help
EVT_TEXT(ID_FPS_Text, MoviePanel::OnFpsEdit)
EVT_COMBOBOX(ID_ViewsCombo, MoviePanel::OnViewSelected)
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

wxTextCtrl * MoviePanel::m_estimated_size_text = 0;

wxWindow* MoviePanel::CreateSimplePage(wxWindow *parent)
{
	wxPanel *page = new wxPanel(parent);

	SetDoubleBuffered(true);

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

wxWindow* MoviePanel::CreateAdvancedPage(wxWindow *parent)
{
	wxPanel *page = new wxPanel(parent);
	//vertical sizer
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
	m_advanced_movie = new RecorderDlg(m_frame, page);
	//m_frame->m_recorder_dlg = m_advanced_movie;
	sizer_v->Add(m_advanced_movie, 0, wxEXPAND);
	page->SetSizer(sizer_v);
	return page;
}

wxWindow* MoviePanel::CreateAutoKeyPage(wxWindow *parent) {
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

wxWindow* MoviePanel::CreateCroppingPage(wxWindow *parent) {
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

wxWindow* MoviePanel::CreateScriptPage(wxWindow *parent)
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

MoviePanel::MoviePanel(RenderFrame* frame,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name) :
	wxPanel(frame, wxID_ANY, pos, size, style, name)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);

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
	long lval;
	m_agent->getValue(gstMovSliderRange, lval);
	m_progress_sldr = new wxSlider(this, ID_ProgressSldr, 0, 0, lval);
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
	//Init();
}

MoviePanel::~MoviePanel() {}

void MoviePanel::OnViewSelected(wxCommandEvent& event)
{
	std::string str = m_views_cmb->GetStringSelection().ToStdString();
	m_agent->Select(str);
}

void MoviePanel::OnNbPageChange(wxBookCtrlEvent& event)
{
	int sel = event.GetSelection();
	int old_sel = event.GetOldSelection();
	if (sel <=1)
		m_agent->setValue(gstMovCurrentPage, long(sel));
}

void MoviePanel::OnPrev(wxCommandEvent& event)
{
	m_agent->Prev();
}

void MoviePanel::OnRun(wxCommandEvent& event)
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
		wxString filename = fopendlg->GetPath();
		m_agent->setValue(gstMovFilename, filename.ToStdWstring());
		m_agent->setValue(gstKeepEnlarge, true);
		m_agent->Run();
	}
	delete fopendlg;
}

void MoviePanel::OnStop(wxCommandEvent& event)
{
	m_agent->Stop();
}

void MoviePanel::OnRewind(wxCommandEvent& event)
{
	m_agent->Rewind();
}

void MoviePanel::OnCropCheck(wxCommandEvent& event)
{
	bool bval = m_crop_chk->GetValue();
	m_agent->setValue(gstDrawCropFrame, bval);
}

void MoviePanel::OnResetCrop(wxCommandEvent& event)
{
	m_agent->setValue(gstDrawCropFrame, true);
}

void MoviePanel::OnEditCrop(wxCommandEvent& event)
{
	long x, y, w, h;
	wxString temp;
	temp = m_center_x_text->GetValue();
	x = STOI(temp.fn_str());
	temp = m_center_y_text->GetValue();
	y = STOI(temp.fn_str());
	temp = m_width_text->GetValue();
	w = STOI(temp.fn_str());
	temp = m_height_text->GetValue();
	h = STOI(temp.fn_str());

	m_agent->setValue(gstCropX, long(x - w / 2.0 + 0.5));
	m_agent->setValue(gstCropY, long(y - h / 2.0 + 0.5));
	m_agent->setValue(gstCropW, w);
	m_agent->setValue(gstCropH, h);
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
	long item = m_auto_key_list->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	m_agent->setValue(gstAutoKeyIndex, item);
	m_agent->AutoKey();
}

//script
void MoviePanel::OnRunScriptChk(wxCommandEvent &event)
{
	bool run_script = m_run_script_chk->GetValue();
	m_agent->setValue(gstRunScript, run_script);
	//m_view->RefreshGL(39);
}

void MoviePanel::OnScriptFileEdit(wxCommandEvent &event)
{
	wxString str = m_script_file_text->GetValue();
	if (!str.IsEmpty())
		m_agent->setValue(gstScriptFile, str.ToStdWstring());
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
		m_script_file_text->SetValue(file);
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
	}
}

//auto key
void MoviePanel::OnGenKey(wxCommandEvent& event)
{
	long item = m_auto_key_list->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	m_agent->setValue(gstAutoKeyIndex, item);
	m_agent->AutoKey();
}

void MoviePanel::OnTimeChange(wxScrollEvent &event)
{
	//if (m_running) return;
	int prg = event.GetPosition();
	long sldr_range;
	m_agent->getValue(gstMovSliderRange, sldr_range);
	double pcnt = (double)prg / sldr_range;
	double len;
	m_agent->getValue(gstMovLength, len);
	double cur_time = pcnt * len;
	wxString str = wxString::Format("%.2f", cur_time);
	if (str != m_progress_text->GetValue())
		m_progress_text->SetValue(str);
}

void MoviePanel::OnTimeText(wxCommandEvent& event)
{
	//if (m_running) return;
	double cur_time;
	wxString str = m_progress_text->GetValue();
	if (!str.ToDouble(&cur_time))
		cur_time = 0;
	m_agent->setValue(gstMovCurTime, cur_time);
}


void MoviePanel::OnRotateChecked(wxCommandEvent& event)
{
	bool bval = m_rot_chk->GetValue();
	m_agent->setValue(gstMovRotEnable, bval);
}

void MoviePanel::OnRotAxis(wxCommandEvent& event)
{
	long lval;
	if (m_x_rd->GetValue())
		lval = 0;
	else if (m_y_rd->GetValue())
		lval = 1;
	else if (m_z_rd->GetValue())
		lval = 2;
	m_agent->setValue(gstMovRotAxis, lval);
}

void MoviePanel::OnDegreeText(wxCommandEvent &event)
{
	wxString str = m_degree_text->GetValue();
	long lval = 0;
	str.ToLong(&lval);
	m_agent->setValue(gstMovRotAng, lval);
}

void MoviePanel::OnRotIntCmb(wxCommandEvent& event)
{
	long lval = m_rot_int_cmb->GetCurrentSelection();
	m_agent->setValue(gstMovIntrpMode, lval);
}

void MoviePanel::OnSequenceChecked(wxCommandEvent& event)
{
	if (m_seq_chk->GetValue())
	{
		m_agent->setValue(gstMovSeqMode, long(1));
		m_agent->setValue(gstMovTimeSeqEnable, true);
	}
	else
	{
		m_agent->setValue(gstMovSeqMode, long(0));
		m_agent->setValue(gstMovTimeSeqEnable, false);
	}
}

void MoviePanel::OnBatchChecked(wxCommandEvent& event)
{
	if (m_bat_chk->GetValue())
	{
		m_agent->setValue(gstMovSeqMode, long(2));
		m_agent->setValue(gstMovTimeSeqEnable, true);
	}
	else
	{
		m_agent->setValue(gstMovSeqMode, long(0));
		m_agent->setValue(gstMovTimeSeqEnable, false);
	}
}

void MoviePanel::OnUpFrame(wxCommandEvent& event)
{
	m_agent->UpFrame();
}

void MoviePanel::OnDownFrame(wxCommandEvent& event)
{
	m_agent->DownFrame();
}

void MoviePanel::OnCurFrameText(wxCommandEvent& event)
{
	//if (m_running) return;
	wxString str = m_cur_frame_text->GetValue();
	long lval;
	if (str.ToLong(&lval))
		m_agent->setValue(gstCurrentFrame, lval);
}

void MoviePanel::OnStartFrameText(wxCommandEvent& event)
{
	wxString str = m_start_frame_text->GetValue();
	long lval;
	if (str.ToLong(&lval))
		m_agent->setValue(gstBeginFrame, lval);
}

void MoviePanel::OnEndFrameText(wxCommandEvent& event)
{
	wxString str = m_end_frame_text->GetValue();
	long lval;
	if (str.ToLong(&lval))
		m_agent->setValue(gstEndFrame, lval);
}

void MoviePanel::OnMovieLenText(wxCommandEvent& event)
{
	wxString str = m_movie_len_text->GetValue();
	double dval;
	if (str.ToDouble(&dval))
		m_agent->setValue(gstMovLength, dval);
}

void MoviePanel::OnFpsEdit(wxCommandEvent& event)
{
	wxString str = m_fps_text->GetValue();
	double dval;
	if (str.ToDouble(&dval))
		m_agent->setValue(gstMovFps, dval);
}

//ch1
void MoviePanel::OnCh1Check(wxCommandEvent &event)
{
	wxCheckBox* ch1 = (wxCheckBox*)event.GetEventObject();
	bool bval = ch1->GetValue();
	glbin_root->setValue(gstCaptureCompress, bval);
}

//ch2
void MoviePanel::OnCh2Check(wxCommandEvent &event)
{
	wxCheckBox* ch2 = (wxCheckBox*)event.GetEventObject();
	bool bval = ch2->GetValue();
	glbin_root->setValue(gstCaptureAlpha, bval);
}

//ch3
void MoviePanel::OnCh3Check(wxCommandEvent &event)
{
	wxCheckBox* ch3 = (wxCheckBox*)event.GetEventObject();
	bool bval = ch3->GetValue();
	glbin_root->setValue(gstCaptureFloat, bval);
}

//enlarge output image
void MoviePanel::OnChEnlargeCheck(wxCommandEvent &event)
{
	wxCheckBox* ch_enlarge = (wxCheckBox*)event.GetEventObject();
	if (ch_enlarge)
	{
		bool enlarge = ch_enlarge->GetValue();
		m_agent->setValue(gstEnlarge, enlarge);
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
			tx_enlarge->SetValue(str);
		}
	}
}

void MoviePanel::OnTxEnlargeText(wxCommandEvent &event)
{
	wxString str = event.GetString();
	double dval;
	str.ToDouble(&dval);
	m_agent->setValue(gstEnlargeScale, dval);
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
void MoviePanel::OnMovieQuality(wxCommandEvent &event)
{
	wxString str = ((wxTextCtrl*)event.GetEventObject())->GetValue();
	double dval;
	if (str.ToDouble(&dval))
	{
		m_agent->setValue(gstMovBitrate, dval);
		double len;
		m_agent->getValue(gstMovLength, len);
		double size = dval * len / 8.;
		m_estimated_size_text->SetValue(wxString::Format("%.2f", size));
	}
}

//embed project
void MoviePanel::OnChEmbedCheck(wxCommandEvent &event)
{
	wxCheckBox* ch_embed = (wxCheckBox*)event.GetEventObject();
	bool bval = ch_embed->GetValue();
	glbin_root->setValue(gstEmbedDataInProject, bval);
}

wxWindow* MoviePanel::CreateExtraCaptureControl(wxWindow* parent)
{
	bool bval;
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
		wxCommandEventHandler(MoviePanel::OnCh1Check), NULL, panel);
	glbin_root->getValue(gstCaptureCompress, bval);
	ch1->SetValue(bval);
	wxCheckBox *ch2 = new wxCheckBox(panel, ID_SAVE_ALPHA,
		"Save alpha");
	ch2->Connect(ch2->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(MoviePanel::OnCh2Check), NULL, panel);
	glbin_root->getValue(gstCaptureAlpha, bval);
	ch2->SetValue(bval);
	wxCheckBox *ch3 = new wxCheckBox(panel, ID_SAVE_FLOAT,
		"Save float channel");
	ch3->Connect(ch3->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(MoviePanel::OnCh3Check), NULL, panel);
	glbin_root->getValue(gstCaptureFloat, bval);
	ch3->SetValue(bval);
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
		wxCommandEventHandler(MoviePanel::OnChEnlargeCheck), NULL, panel);
	wxSlider* sl_enlarge = new wxSlider(panel, ID_ENLARGE_SLDR,
		10, 10, 100);
	sl_enlarge->Connect(sl_enlarge->GetId(), wxEVT_COMMAND_SLIDER_UPDATED,
		wxScrollEventHandler(MoviePanel::OnSlEnlargeScroll), NULL, panel);
	sl_enlarge->Disable();
	wxFloatingPointValidator<double> vald_fp(1);
	wxTextCtrl* tx_enlarge = new wxTextCtrl(panel, ID_ENLARGE_TEXT,
		"1.0", wxDefaultPosition, wxDefaultSize, 0, vald_fp);
	tx_enlarge->Connect(tx_enlarge->GetId(), wxEVT_COMMAND_TEXT_UPDATED,
		wxCommandEventHandler(MoviePanel::OnTxEnlargeText), NULL, panel);
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
		wxCommandEventHandler(MoviePanel::OnMovieQuality), NULL, panel);
	wxStaticText *st = new wxStaticText(panel, wxID_ANY, "Bitrate:",
		wxDefaultPosition, wxDefaultSize);
	wxStaticText *st2 = new wxStaticText(panel, wxID_ANY, "Mbps",
		wxDefaultPosition, wxDefaultSize);
	wxStaticText *st3 = new wxStaticText(panel, wxID_ANY, "Estimated size:",
		wxDefaultPosition, wxDefaultSize);
	m_estimated_size_text = new wxTextCtrl(panel, ID_BitrateText, "2.5",
		wxDefaultPosition, wxDefaultSize);
	m_estimated_size_text->Disable();
	m_estimated_size_text->SetValue(wxString::Format("%.2f", 20.0));

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
	glbin_root->getValue(gstSaveProjectEnable, bval);
	if (bval)
	{
		ch_embed = new wxCheckBox(panel, ID_EMBED_FILES,
			"Embed all files in the project folder");
		ch_embed->Connect(ch_embed->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
			wxCommandEventHandler(MoviePanel::OnChEmbedCheck), NULL, panel);
		ch_embed->SetValue(bval);
	}
	//group
	if (bval && ch_embed)
	{
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
