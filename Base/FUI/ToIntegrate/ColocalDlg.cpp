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
#include "ColocalDlg.h"
#include <VRenderFrame.h>
#include <Scenegraph/VolumeData.h>
#include <Global/Global.h>
//#include <wx/valnum.h>

using namespace FUI;

BEGIN_EVENT_TABLE(ColocalDlg, wxPanel)
EVT_TEXT(ID_OutputText, ColocalDlg::OnOutputText)
EVT_BUTTON(ID_OutputBtn, ColocalDlg::OnOutputBtn)
EVT_BUTTON(ID_CalcColocalizationBtn, ColocalDlg::OnColocalizationBtn)
END_EVENT_TABLE()

ColocalDlg::ColocalDlg(
	wxWindow* parent) :
wxPanel(parent, wxID_ANY,
wxDefaultPosition, wxSize(400, 165),
0, "ColocalDlg")
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);

	m_agent =
		FL::Global::instance().getAgentFactory().
		getOrAddColocalAgent("ColocalDlg", *this);

	//validator: integer
	//wxIntegerValidator<unsigned int> vald_int;

	wxStaticText *st = 0;

	//output
	wxBoxSizer *sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Output file:",
		wxDefaultPosition, wxSize(75, 20));
	m_output_text = new wxTextCtrl(this, ID_OutputText, "");
	m_output_btn = new wxButton(this, ID_OutputBtn, "Browse...");
	sizer_1->Add(10, 10);
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	sizer_1->Add(m_output_text, 1, wxALIGN_CENTER);
	sizer_1->Add(m_output_btn, 0, wxALIGN_CENTER);
	sizer_1->Add(10, 10);
	//calculate
	wxBoxSizer *sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	m_colocalization_btn = new wxButton(this, ID_CalcColocalizationBtn, "Colocalization",
		wxDefaultPosition, wxDefaultSize);
	sizer_2->AddStretchSpacer(1);
	sizer_2->Add(m_colocalization_btn, 0, wxALIGN_CENTER);
	sizer_2->Add(10, 10);

	wxBoxSizer *sizerV = new wxBoxSizer(wxVERTICAL);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_1, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_2, 0, wxEXPAND);
	sizerV->Add(10, 10);
	
	SetSizer(sizerV);
	Layout();
}

ColocalDlg::~ColocalDlg()
{
}

void ColocalDlg::OnOutputText(wxCommandEvent &event)
{
	m_output_file = m_output_text->GetValue();
}

void ColocalDlg::OnOutputBtn(wxCommandEvent &event)
{
	wxFileDialog *fopendlg = new wxFileDialog(
		this, "Choose the output file",
		"", "colocalization_result.txt",
		"*.txt", wxFD_OPEN);

	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		m_output_file = fopendlg->GetPath();
		m_output_text->SetValue(m_output_file);
	}

	if (fopendlg)
		delete fopendlg;
}

void ColocalDlg::OnColocalizationBtn(wxCommandEvent &event)
{
	m_agent->setValue("output file", m_output_file.ToStdWstring());
	m_agent->Run();
}