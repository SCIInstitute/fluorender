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
#include "MachineLearningDlg.h"
#include "VRenderFrame.h"

BEGIN_EVENT_TABLE(MachineLearningDlg, wxPanel)
END_EVENT_TABLE()

MachineLearningDlg::MachineLearningDlg(VRenderFrame *frame) :
	wxPanel(frame, wxID_ANY,
		wxDefaultPosition, wxSize(450, 750),
		0, "SettingDlg"),
	m_frame(frame)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);
	SetDoubleBuffered(true);

	//notebook
	wxNotebook *notebook = new wxNotebook(this, wxID_ANY);
	MLCompGenPanel* panel1 = new MLCompGenPanel(frame, notebook);
	notebook->AddPage(panel1, "Component Generator");

	//interface
	wxBoxSizer *sizerV = new wxBoxSizer(wxVERTICAL);
	sizerV->Add(notebook, 1, wxEXPAND);
	SetSizerAndFit(sizerV);
	Layout();

	//GetSettings();
}

MachineLearningDlg::~MachineLearningDlg()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
BEGIN_EVENT_TABLE(MachineLearningPanel, wxPanel)
END_EVENT_TABLE()

MachineLearningPanel::MachineLearningPanel(
	VRenderFrame* frame, wxWindow* parent) :
	wxPanel(parent, wxID_ANY,
		wxDefaultPosition, wxSize(450, 750),
		0, "MachineLearningPanel"),
	m_frame(frame)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);
	SetDoubleBuffered(true);
}

MachineLearningPanel::~MachineLearningPanel()
{

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
MLCompGenPanel::MLCompGenPanel(
	VRenderFrame* frame, wxWindow* parent) :
	MachineLearningPanel(frame, parent)
{

}

MLCompGenPanel::~MLCompGenPanel()
{

}