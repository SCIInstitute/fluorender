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
#include <ClKernelDlg.h>
#include <RenderFrame.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/stdpaths.h>
#include <wx/valnum.h>

BEGIN_EVENT_TABLE(ClKernelDlg, wxPanel)
	EVT_BUTTON(ID_BrowseBtn, ClKernelDlg::OnBrowseBtn)
	EVT_BUTTON(ID_SaveBtn, ClKernelDlg::OnSaveBtn)
	EVT_BUTTON(ID_SaveAsBtn, ClKernelDlg::OnSaveAsBtn)
	EVT_BUTTON(ID_ExecuteBtn, ClKernelDlg::OnExecuteBtn)
	EVT_BUTTON(ID_ExecuteNBtn, ClKernelDlg::OnExecuteNBtn)
	EVT_COMMAND_SCROLL(ID_IterationsSldr, ClKernelDlg::OnIterationsChange)
	EVT_TEXT(ID_IterationsTxt, ClKernelDlg::OnIterationsEdit)
	EVT_LIST_ITEM_SELECTED(ID_KernelList, ClKernelDlg::OnKernelListSelected)
END_EVENT_TABLE()

ClKernelDlg::ClKernelDlg(RenderFrame* frame) :
wxPanel(frame, wxID_ANY,
wxDefaultPosition, wxSize(550, 600),
0, "ClKernelDlg")
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);

	SetDoubleBuffered(true);

	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;

	wxStaticText *st = 0;
	//
	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Kernel file:",
		wxDefaultPosition, wxSize(70, 20));
	m_kernel_file_txt = new wxTextCtrl(this, ID_KernelFileTxt, "",
		wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_browse_btn = new wxButton(this, ID_BrowseBtn, "Browse",
		wxDefaultPosition, wxSize(70, 23));
	m_save_btn = new wxButton(this, ID_SaveBtn, "Save",
		wxDefaultPosition, wxSize(70, 23));
	m_saveas_btn = new wxButton(this, ID_SaveAsBtn, "Save As",
		wxDefaultPosition, wxSize(70, 23));
	sizer_1->Add(5, 5);
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	sizer_1->Add(m_kernel_file_txt, 1, wxEXPAND);
	sizer_1->Add(m_browse_btn, 0, wxALIGN_CENTER);
	sizer_1->Add(m_save_btn, 0, wxALIGN_CENTER);
	sizer_1->Add(m_saveas_btn, 0, wxALIGN_CENTER);

	//controls
	wxBoxSizer* sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Controls:",
		wxDefaultPosition, wxSize(70, 20));
	m_execute_btn = new wxButton(this, ID_ExecuteBtn, "Run",
		wxDefaultPosition, wxSize(60, 23));
	m_execute_n_btn = new wxButton(this, ID_ExecuteNBtn, "Run N Times",
		wxDefaultPosition, wxSize(80, 23));
	m_iterations_sldr = new wxSlider(this, ID_IterationsSldr, 1, 1, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_iterations_txt = new wxTextCtrl(this, ID_IterationsTxt, "1",
		wxDefaultPosition, wxSize(40, 20), 0, vald_int);
	sizer_2->Add(5, 5);
	sizer_2->Add(st, 0, wxALIGN_CENTER);
	sizer_2->Add(m_execute_btn, 0, wxALIGN_CENTER);
	sizer_2->Add(m_execute_n_btn, 0, wxALIGN_CENTER);
	sizer_2->Add(5, 5);
	sizer_2->Add(m_iterations_sldr, 1, wxEXPAND);
	sizer_2->Add(m_iterations_txt, 0, wxALIGN_CENTER);
	sizer_2->Add(5, 5);

	//output
	m_output_txt = new wxTextCtrl(this, ID_OutputTxt, "",
		wxDefaultPosition, wxSize(-1, 100), wxTE_READONLY|wxTE_MULTILINE);

	//list
	m_kernel_list = new wxListCtrl(this, ID_KernelList,
		wxDefaultPosition, wxSize(-1, -1), wxLC_REPORT | wxLC_SINGLE_SEL);
	wxListItem itemCol;
	itemCol.SetText("Kernel Files");
	m_kernel_list->InsertColumn(0, itemCol);
	m_kernel_list->SetColumnWidth(0, 100);
	//stc
	m_LineNrID = 0;
	m_DividerID = 1;
	m_FoldingID = 2;
	m_kernel_edit_stc = new wxStyledTextCtrl(
		this, ID_KernelEditStc,
		wxDefaultPosition, wxDefaultSize);
	wxFont font(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	m_kernel_edit_stc->StyleSetFont(wxSTC_STYLE_DEFAULT, font);
	m_kernel_edit_stc->StyleSetForeground(wxSTC_STYLE_DEFAULT, *wxBLACK);
	m_kernel_edit_stc->StyleSetBackground(wxSTC_STYLE_DEFAULT, *wxWHITE);
	m_kernel_edit_stc->StyleSetForeground(wxSTC_STYLE_LINENUMBER, wxColour(wxT("DARK GREY")));
	m_kernel_edit_stc->StyleSetBackground(wxSTC_STYLE_LINENUMBER, *wxWHITE);
	m_kernel_edit_stc->StyleSetForeground(wxSTC_STYLE_INDENTGUIDE, wxColour(wxT("DARK GREY")));
	m_kernel_edit_stc->SetLexer(wxSTC_LEX_CPP);
	m_kernel_edit_stc->SetMarginType(m_LineNrID, wxSTC_MARGIN_NUMBER);
	m_kernel_edit_stc->StyleSetForeground(wxSTC_STYLE_LINENUMBER, wxColour(wxT("DARK GREY")));
	m_kernel_edit_stc->StyleSetBackground(wxSTC_STYLE_LINENUMBER, *wxWHITE);
	m_kernel_edit_stc->SetMarginWidth(m_LineNrID, 50);
	m_kernel_edit_stc->StyleSetBackground(wxSTC_STYLE_LASTPREDEFINED + 1, wxColour(244, 220, 220));
	m_kernel_edit_stc->StyleSetForeground(wxSTC_STYLE_LASTPREDEFINED + 1, *wxBLACK);
	m_kernel_edit_stc->StyleSetSizeFractional(wxSTC_STYLE_LASTPREDEFINED + 1,
		(m_kernel_edit_stc->StyleGetSizeFractional(wxSTC_STYLE_DEFAULT) * 4) / 5);
	m_kernel_edit_stc->SetWrapMode(wxSTC_WRAP_WORD); // other choice is wxSCI_WRAP_NONE
	m_kernel_edit_stc->StyleSetForeground(wxSTC_C_STRING, wxColour(150, 0, 0));
	m_kernel_edit_stc->StyleSetForeground(wxSTC_C_PREPROCESSOR, wxColour(165, 105, 0));
	m_kernel_edit_stc->StyleSetForeground(wxSTC_C_IDENTIFIER, wxColour(40, 0, 60));
	m_kernel_edit_stc->StyleSetForeground(wxSTC_C_NUMBER, wxColour(0, 150, 0));
	m_kernel_edit_stc->StyleSetForeground(wxSTC_C_CHARACTER, wxColour(150, 0, 0));
	m_kernel_edit_stc->StyleSetForeground(wxSTC_C_WORD, wxColour(0, 0, 150));
	m_kernel_edit_stc->StyleSetForeground(wxSTC_C_WORD2, wxColour(0, 150, 0));
	m_kernel_edit_stc->StyleSetForeground(wxSTC_C_COMMENT, wxColour(150, 150, 150));
	m_kernel_edit_stc->StyleSetForeground(wxSTC_C_COMMENTLINE, wxColour(150, 150, 150));
	m_kernel_edit_stc->StyleSetForeground(wxSTC_C_COMMENTDOC, wxColour(150, 150, 150));
	m_kernel_edit_stc->StyleSetForeground(wxSTC_C_COMMENTDOCKEYWORD, wxColour(0, 0, 200));
	m_kernel_edit_stc->StyleSetForeground(wxSTC_C_COMMENTDOCKEYWORDERROR, wxColour(0, 0, 200));
	m_kernel_edit_stc->StyleSetBold(wxSTC_C_WORD, true);
	m_kernel_edit_stc->StyleSetBold(wxSTC_C_WORD2, true);
	m_kernel_edit_stc->StyleSetBold(wxSTC_C_COMMENTDOCKEYWORD, true);
	// a sample list of keywords, I haven't included them all to keep it short...
	m_kernel_edit_stc->SetKeyWords(0, wxT("return for while break continue __kernel kernel_main __global"));
	m_kernel_edit_stc->SetKeyWords(1, wxT("const int float void char double unsigned int4 float4 signed"));
	m_kernel_edit_stc->SetMarginType(m_FoldingID, wxSTC_MARGIN_SYMBOL);
	m_kernel_edit_stc->SetMarginMask(m_FoldingID, wxSTC_MASK_FOLDERS);
	m_kernel_edit_stc->StyleSetBackground(m_FoldingID, *wxWHITE);
	// markers
	m_kernel_edit_stc->MarkerDefine(wxSTC_MARKNUM_FOLDER, wxSTC_MARK_DOTDOTDOT, wxT("BLACK"), wxT("BLACK"));
	m_kernel_edit_stc->MarkerDefine(wxSTC_MARKNUM_FOLDEROPEN, wxSTC_MARK_ARROWDOWN, wxT("BLACK"), wxT("BLACK"));
	m_kernel_edit_stc->MarkerDefine(wxSTC_MARKNUM_FOLDERSUB, wxSTC_MARK_EMPTY, wxT("BLACK"), wxT("BLACK"));
	m_kernel_edit_stc->MarkerDefine(wxSTC_MARKNUM_FOLDEREND, wxSTC_MARK_DOTDOTDOT, wxT("BLACK"), wxT("WHITE"));
	m_kernel_edit_stc->MarkerDefine(wxSTC_MARKNUM_FOLDEROPENMID, wxSTC_MARK_ARROWDOWN, wxT("BLACK"), wxT("WHITE"));
	m_kernel_edit_stc->MarkerDefine(wxSTC_MARKNUM_FOLDERMIDTAIL, wxSTC_MARK_EMPTY, wxT("BLACK"), wxT("BLACK"));
	m_kernel_edit_stc->MarkerDefine(wxSTC_MARKNUM_FOLDERTAIL, wxSTC_MARK_EMPTY, wxT("BLACK"), wxT("BLACK"));

	//sizer
	wxBoxSizer *sizer_3 = new wxBoxSizer(wxHORIZONTAL);
	sizer_3->Add(m_kernel_list, 0, wxEXPAND);
	wxStaticText * separator = new wxStaticText(this, 0, "", wxDefaultPosition, wxSize(5, -1));
	sizer_3->Add(separator, 0, wxEXPAND);
	sizer_3->Add(m_kernel_edit_stc, 1, wxEXPAND);

	//all controls
	wxBoxSizer *sizerV = new wxBoxSizer(wxVERTICAL);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_1, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_2, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_3, 4, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(m_output_txt, 1, wxEXPAND);

	SetSizer(sizerV);
	Layout();
}

ClKernelDlg::~ClKernelDlg()
{
}

void ClKernelDlg::OnBrowseBtn(wxCommandEvent& event)
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

void ClKernelDlg::OnSaveBtn(wxCommandEvent& event)
{
	wxString filename = m_kernel_file_txt->GetValue();
	if (filename == "")
	{
		wxCommandEvent e;
		OnSaveAsBtn(e);
	}
	else
		m_kernel_edit_stc->SaveFile(filename);
}

void ClKernelDlg::OnSaveAsBtn(wxCommandEvent& event)
{
	wxFileDialog *fopendlg = new wxFileDialog(
		this, "Choose an OpenCL kernel file", 
		"", "", "OpenCL kernel file|*.cl;*.txt", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		wxString filename = fopendlg->GetPath();
		rval = m_kernel_edit_stc->SaveFile(filename);
		if (rval) {
			m_kernel_file_txt->SetValue(filename);
			filename = wxFileNameFromPath(filename);
			wxString exePath = wxStandardPaths::Get().GetExecutablePath();
			exePath = wxPathOnly(exePath);
			wxString temp = exePath + GETSLASH() + "CL_code" +
				GETSLASH() + filename;
			m_kernel_edit_stc->SaveFile(temp);
			filename = filename.BeforeFirst('.');
			m_kernel_list->InsertItem(m_kernel_list->GetItemCount(), filename);
		}
	}

	if (fopendlg)
		delete fopendlg;
}

void ClKernelDlg::OnExecuteBtn(wxCommandEvent& event)
{
	m_agent->RunKernel(1);
}

void ClKernelDlg::OnExecuteNBtn(wxCommandEvent& event)
{
	wxString str = m_iterations_txt->GetValue();
	unsigned long ival;
	if (str.ToULong(&ival))
		m_agent->RunKernel(ival);
}

void ClKernelDlg::OnIterationsChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	wxString str = wxString::Format("%d", ival);
	if (str != m_iterations_txt->GetValue())
		m_iterations_txt->SetValue(str);
}

void ClKernelDlg::OnIterationsEdit(wxCommandEvent &event)
{
	wxString str = m_iterations_txt->GetValue();
	unsigned long ival;
	str.ToULong(&ival);
	m_iterations_sldr->SetValue(ival);
}

void ClKernelDlg::OnKernelListSelected(wxListEvent& event)
{
	long item = m_kernel_list->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);

	if (item != -1)
	{
		wxString file = m_kernel_list->GetItemText(item);
		wxString exePath = wxStandardPaths::Get().GetExecutablePath();
		exePath = wxPathOnly(exePath);
		file = exePath + GETSLASH() + "CL_code" +
			GETSLASH() + file + ".cl";
		m_kernel_edit_stc->LoadFile(file);
		m_kernel_edit_stc->EmptyUndoBuffer();
		m_kernel_file_txt->SetValue(file);
	}
}

