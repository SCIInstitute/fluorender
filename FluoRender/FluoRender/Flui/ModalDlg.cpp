/*M
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
#include <ModalDlg.h>
#include <Global.h>

ModalDlg::ModalDlg(wxWindow* parent,
	const wxString& message,
	const wxString& defaultDir,
	const wxString& defaultFile,
	const wxString& wildcard,
	long style,
	const wxPoint& pos,
	const wxSize& size,
	const wxString& name):
wxFileDialog(parent, message, defaultDir, defaultFile, wildcard, style, pos, size, name)
{
}

ModalDlg::~ModalDlg()
{
}

int ModalDlg::ShowModal()
{
	glbin_states.m_modal_shown = true;
	int r = wxFileDialog::ShowModal();
	wxMilliSleep(110);
	glbin_states.m_modal_shown = false;
	return r;
}
