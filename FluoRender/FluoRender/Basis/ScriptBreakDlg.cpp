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
#include "ScriptBreakDlg.h"
#include <Global.h>
#include <VRenderFrame.h>
#include <VMovieView.h>

BEGIN_EVENT_TABLE(ScriptBreakDlg, wxPanel)
EVT_CHECKBOX(ID_ShownChk, ScriptBreakDlg::OnShownChk)
EVT_BUTTON(ID_StopBtn, ScriptBreakDlg::OnStopBtn)
EVT_BUTTON(ID_ContinueBtn, ScriptBreakDlg::OnContinueBtn)
END_EVENT_TABLE()

ScriptBreakDlg::ScriptBreakDlg(VRenderFrame* frame) :
	wxPanel(frame, wxID_ANY,
		wxDefaultPosition,
		frame->FromDIP(wxSize(400, 300)),
		0, ""),
	m_frame(frame)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);

	wxBoxSizer* sizer1 = new wxBoxSizer(wxVERTICAL);
	m_info_text = new wxTextCtrl(this, wxID_ANY, "",
		wxDefaultPosition, wxDefaultSize, wxTE_READONLY|wxTE_MULTILINE);
	sizer1->Add(m_info_text, 1, wxEXPAND);
	wxBoxSizer* sizer2 = new wxBoxSizer(wxHORIZONTAL);
	m_shown_chk = new wxCheckBox(this, ID_ShownChk, "Do not show again",
		wxDefaultPosition, wxDefaultSize);
	m_stop_btn = new wxButton(this, ID_StopBtn, "Stop",
		wxDefaultPosition, wxDefaultSize);
	m_continue_btn = new wxButton(this, ID_ContinueBtn, "Continue",
		wxDefaultPosition, wxDefaultSize);
	sizer2->Add(5, 5);
	sizer2->Add(m_shown_chk, 0, wxALIGN_CENTER);
	sizer2->AddStretchSpacer(1);
	sizer2->Add(m_stop_btn, 0, wxALIGN_CENTER);
	sizer2->Add(5, 5);
	sizer2->Add(m_continue_btn, 0, wxALIGN_CENTER);
	sizer2->Add(5, 5);

	//
	wxBoxSizer* sizerv = new wxBoxSizer(wxVERTICAL);
	sizerv->Add(10, 10);
	sizerv->Add(sizer1, 1, wxEXPAND);
	sizerv->Add(10, 10);
	sizerv->Add(sizer2, 0, wxEXPAND);
	sizerv->Add(10, 10);

	SetSizer(sizerv);
	Layout();
}

ScriptBreakDlg::~ScriptBreakDlg()
{
}

void ScriptBreakDlg::SetScriptName(const wxString& str)
{
	SetLabel(str);
}

void ScriptBreakDlg::SetInfo(const wxString& str)
{
	m_info_text->SetValue(str);
}

void ScriptBreakDlg::Hold()
{
	if (!m_frame)
		return;
	m_frame->GetMovieView()->HoldRun();
	m_frame->ShowScriptBreakDlg();
}

void ScriptBreakDlg::OnShownChk(wxCommandEvent& event)
{
	glbin_settings.m_script_break = m_shown_chk->GetValue();
}

void ScriptBreakDlg::OnStopBtn(wxCommandEvent& event)
{
	if (!m_frame)
		return;
	m_frame->GetMovieView()->Stop();
	m_frame->GetMovieView()->Rewind();
	m_frame->ShowScriptBreakDlg(false);
}

void ScriptBreakDlg::OnContinueBtn(wxCommandEvent& event)
{
	if (!m_frame)
		return;
	m_frame->GetMovieView()->ResumeRun();
	m_frame->ShowScriptBreakDlg(false);
}
