/*
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
#include <PropertyPanel.h>
#include <MainFrame.h>

PropertyPanel::PropertyPanel(MainFrame* frame,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name) :
	TabbedPanel(frame, frame, pos, size, style, name)
{
	wxEventBlocker blocker(this);
	Freeze();
	m_notebook = new wxAuiNotebook(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize,
		wxAUI_NB_DEFAULT_STYLE | wxAUI_NB_TAB_EXTERNAL_MOVE |
		wxAUI_NB_WINDOWLIST_BUTTON | wxNO_BORDER);
	m_notebook->Bind(wxEVT_AUINOTEBOOK_PAGE_CLOSE, &PropertyPanel::OnPropPageClose, this);
	wxBoxSizer *sizerv = new wxBoxSizer(wxVERTICAL);
	sizerv->Add(m_notebook, 1, wxEXPAND);
	SetSizer(sizerv);
	Layout();
	SetAutoLayout(true);
	SetScrollRate(10, 10);
	Thaw();
}

//prop pages
void PropertyPanel::OnPropPageClose(wxAuiNotebookEvent& event)
{
	if (!m_notebook)
		return;
	wxAuiNotebook* panel = (wxAuiNotebook*)event.GetEventObject();
	wxWindow* page = panel->GetPage(event.GetSelection());
	if (page)
	{
		int page_no = m_notebook->FindPage(page);
		page->Hide();
		m_notebook->RemovePage(page_no);
	}
	event.Veto();
}

