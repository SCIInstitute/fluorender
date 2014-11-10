/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2014 Scientific Computing and Imaging Institute,
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
#include "OclDlg.h"
#include "VRenderFrame.h"
#include "VRenderView.h"
#include <wx/wfstream.h>
#include <wx/txtstrm.h>

BEGIN_EVENT_TABLE(OclDlg, wxPanel)
	EVT_BUTTON(ID_BrowseBtn, OclDlg::OnBrowseBtn)
	EVT_BUTTON(ID_SaveBtn, OclDlg::OnSaveBtn)
	EVT_BUTTON(ID_SaveAsBtn, OclDlg::OnSaveAsBtn)
	EVT_BUTTON(ID_ExecuteBtn, OclDlg::OnExecuteBtn)
END_EVENT_TABLE()

OclDlg::OclDlg(wxWindow* frame,
wxWindow* parent) :
wxPanel(parent, wxID_ANY,
wxPoint(500, 150), wxSize(500, 600),
0, "OclDlg"),
m_frame(parent),
m_view(0)
{
	wxStaticText *st = 0;
	//
	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Kernel file:",
		wxDefaultPosition, wxSize(70, 20));
	m_kernel_file_txt = new wxTextCtrl(this, ID_KernelFileTxt, "",
		wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_browse_btn = new wxButton(this, ID_BrowseBtn, "Browse",
		wxDefaultPosition, wxSize(60, 23));
	m_save_btn = new wxButton(this, ID_SaveBtn, "Save",
		wxDefaultPosition, wxSize(60, 23));
	m_saveas_btn = new wxButton(this, ID_SaveAsBtn, "Save As",
		wxDefaultPosition, wxSize(60, 23));
	sizer_1->Add(5, 5);
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	sizer_1->Add(m_kernel_file_txt, 1, wxEXPAND|wxALIGN_CENTER);
	sizer_1->Add(m_browse_btn, 0, wxALIGN_CENTER);
	sizer_1->Add(m_save_btn, 0, wxALIGN_CENTER);
	sizer_1->Add(m_saveas_btn, 0, wxALIGN_CENTER);

	//controls
	wxBoxSizer* sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Controls:",
		wxDefaultPosition, wxSize(70, 20));
	m_execute_btn = new wxButton(this, ID_ExecuteBtn, "Run",
		wxDefaultPosition, wxSize(60, 23));
	sizer_2->Add(5, 5);
	sizer_2->Add(st, 0, wxALIGN_CENTER);
	sizer_2->Add(m_execute_btn, 0, wxALIGN_CENTER);

	//output
	m_output_txt = new wxTextCtrl(this, ID_OutputTxt, "",
		wxDefaultPosition, wxSize(-1, 100), wxTE_READONLY);

	//
    m_LineNrID = 0;
    m_DividerID = 1;
    m_FoldingID = 2;
	m_kernel_edit_stc = new wxStyledTextCtrl(
		this, ID_KernelEditStc,
		wxDefaultPosition, wxDefaultSize);
    wxFont font (10, wxMODERN, wxNORMAL, wxNORMAL);
    m_kernel_edit_stc->StyleSetFont (wxSTC_STYLE_DEFAULT, font);
    m_kernel_edit_stc->StyleSetForeground (wxSTC_STYLE_DEFAULT, *wxBLACK);
    m_kernel_edit_stc->StyleSetBackground (wxSTC_STYLE_DEFAULT, *wxWHITE);
    m_kernel_edit_stc->StyleSetForeground (wxSTC_STYLE_LINENUMBER, wxColour (wxT("DARK GREY")));
    m_kernel_edit_stc->StyleSetBackground (wxSTC_STYLE_LINENUMBER, *wxWHITE);
    m_kernel_edit_stc->StyleSetForeground(wxSTC_STYLE_INDENTGUIDE, wxColour (wxT("DARK GREY")));
	m_kernel_edit_stc->SetLexer(wxSTC_LEX_CPP);
	m_kernel_edit_stc->SetMarginType(m_LineNrID, wxSTC_MARGIN_NUMBER);
	m_kernel_edit_stc->StyleSetForeground(wxSTC_STYLE_LINENUMBER, wxColour(wxT("DARK GREY")));
	m_kernel_edit_stc->StyleSetBackground(wxSTC_STYLE_LINENUMBER, *wxWHITE);
	m_kernel_edit_stc->SetMarginWidth(m_LineNrID, 50);
    m_kernel_edit_stc->StyleSetBackground(wxSTC_STYLE_LASTPREDEFINED + 1, wxColour(244, 220, 220));
    m_kernel_edit_stc->StyleSetForeground(wxSTC_STYLE_LASTPREDEFINED + 1, *wxBLACK);
    m_kernel_edit_stc->StyleSetSizeFractional(wxSTC_STYLE_LASTPREDEFINED + 1,
            (m_kernel_edit_stc->StyleGetSizeFractional(wxSTC_STYLE_DEFAULT)*4)/5);
    //// default fonts for all styles!
    //int Nr;
    //for (Nr = 0; Nr < wxSTC_STYLE_LASTPREDEFINED; Nr++) {
    //    wxFont font (10, wxMODERN, wxNORMAL, wxNORMAL);
    //    m_kernel_edit_stc->StyleSetFont (Nr, font);
    //}
	m_kernel_edit_stc->SetMarginType (m_FoldingID, wxSTC_MARGIN_SYMBOL);
	m_kernel_edit_stc->SetMarginMask (m_FoldingID, wxSTC_MASK_FOLDERS);
	m_kernel_edit_stc->StyleSetBackground (m_FoldingID, *wxWHITE);
    // markers
    m_kernel_edit_stc->MarkerDefine (wxSTC_MARKNUM_FOLDER,        wxSTC_MARK_DOTDOTDOT, wxT("BLACK"), wxT("BLACK"));
    m_kernel_edit_stc->MarkerDefine (wxSTC_MARKNUM_FOLDEROPEN,    wxSTC_MARK_ARROWDOWN, wxT("BLACK"), wxT("BLACK"));
    m_kernel_edit_stc->MarkerDefine (wxSTC_MARKNUM_FOLDERSUB,     wxSTC_MARK_EMPTY,     wxT("BLACK"), wxT("BLACK"));
    m_kernel_edit_stc->MarkerDefine (wxSTC_MARKNUM_FOLDEREND,     wxSTC_MARK_DOTDOTDOT, wxT("BLACK"), wxT("WHITE"));
    m_kernel_edit_stc->MarkerDefine (wxSTC_MARKNUM_FOLDEROPENMID, wxSTC_MARK_ARROWDOWN, wxT("BLACK"), wxT("WHITE"));
    m_kernel_edit_stc->MarkerDefine (wxSTC_MARKNUM_FOLDERMIDTAIL, wxSTC_MARK_EMPTY,     wxT("BLACK"), wxT("BLACK"));
    m_kernel_edit_stc->MarkerDefine (wxSTC_MARKNUM_FOLDERTAIL,    wxSTC_MARK_EMPTY,     wxT("BLACK"), wxT("BLACK"));

	//all controls
	wxBoxSizer *sizerV = new wxBoxSizer(wxVERTICAL);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_1, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_2, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(m_kernel_edit_stc, 1, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(m_output_txt, 0, wxEXPAND);

	SetSizer(sizerV);
	Layout();
}

OclDlg::~OclDlg()
{
}

void OclDlg::OnBrowseBtn(wxCommandEvent& event)
{
	wxFileDialog *fopendlg = new wxFileDialog(
		this, "Choose an OpenCL kernel file", 
		"", "", "OpenCL kernel file|*.cl;*.txt", wxFD_OPEN|wxFD_FILE_MUST_EXIST);

	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		wxString filename = fopendlg->GetPath();
		m_kernel_edit_stc->LoadFile(filename);
		m_kernel_edit_stc->EmptyUndoBuffer();
		m_kernel_file_txt->SetValue(filename);
	}

	if (fopendlg)
		delete fopendlg;
}

void OclDlg::OnSaveBtn(wxCommandEvent& event)
{
}

void OclDlg::OnSaveAsBtn(wxCommandEvent& event)
{
}

void OclDlg::OnExecuteBtn(wxCommandEvent& event)
{
}
