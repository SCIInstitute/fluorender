#include "HelpDlg.h"

BEGIN_EVENT_TABLE(HelpDlg, wxPanel)
END_EVENT_TABLE()

HelpDlg::HelpDlg(wxWindow *frame, 
				 wxWindow *parent) :
wxPanel(parent, wxID_ANY,
		 wxDefaultPosition, wxSize(600, 600),
		 0, "HelpDlg"),
m_html(0)
{
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

void HelpDlg::LoadPage(wxString name)
{
	if (m_html)
		m_html->LoadPage(name);
}
