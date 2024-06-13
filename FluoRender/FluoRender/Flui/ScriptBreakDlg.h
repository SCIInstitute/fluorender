/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2024 Scientific Computing and Imaging Institute,
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
#ifndef _SCRIPTBREAK_H_
#define _SCRIPTBREAK_H_

#include <wx/wx.h>

class MainFrame;
class ScriptBreakDlg : public wxPanel
{
public:
	enum
	{
		ID_ShownChk = ID_SCRIPT_BREAK,
		ID_StopBtn,
		ID_ContinueBtn
	};
	ScriptBreakDlg(MainFrame* frame);
	~ScriptBreakDlg();

	void SetScriptName(const wxString& str);
	void SetInfo(const wxString& str);
	void Hold();

private:
	MainFrame* m_frame;
	wxTextCtrl* m_info_text;
	wxCheckBox* m_shown_chk;
	wxButton* m_stop_btn;
	wxButton* m_continue_btn;

private:
	void OnShownChk(wxCommandEvent& event);
	void OnStopBtn(wxCommandEvent& event);
	void OnContinueBtn(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()
};

#endif//_SCRIPTBREAK_H_
