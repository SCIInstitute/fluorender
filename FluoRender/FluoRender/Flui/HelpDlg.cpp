﻿/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2025 Scientific Computing and Imaging Institute,
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
#include <HelpDlg.h>
#include <Global.h>
#include <Names.h>
#include <MainFrame.h>

HelpDlg::HelpDlg(MainFrame *frame) :
	PropPanel(frame, frame,
	wxDefaultPosition,
	frame->FromDIP(wxSize(600, 600)),
	0, "HelpDlg"),
m_html(0)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);

	m_name = "";
	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	m_html = new wxHtmlWindow(this);
	sizer->Add(m_html, 1, wxEXPAND);
	SetSizer(sizer);
	Layout();
}

HelpDlg::~HelpDlg()
{
}

void HelpDlg::FluoUpdate(const fluo::ValueCollection& vc)
{
	//update user interface
	if (FOUND_VALUE(gstNull))
		return;
	bool update_all = vc.empty();

	//if (update_all || FOUND_VALUE(gstHelpUrl))
	//	m_html->LoadPage(glbin_help_url);
}
