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
	sizer_1->Add(5, 5);
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	sizer_1->Add(m_kernel_file_txt, 1, wxEXPAND|wxALIGN_CENTER);
	sizer_1->Add(m_browse_btn, 0, wxALIGN_CENTER);

	//
	m_kernel_edit_stc = new wxStyledTextCtrl(
		this, ID_KernelEditStc,
		wxDefaultPosition, wxDefaultSize);

	//all controls
	wxBoxSizer *sizerV = new wxBoxSizer(wxVERTICAL);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_1, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(m_kernel_edit_stc, 1, wxEXPAND);

	SetSizer(sizerV);
	Layout();
}

OclDlg::~OclDlg()
{
}

void OclDlg::OnBrowseBtn(wxCommandEvent& event)
{
}
