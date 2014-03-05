#include <wx/wx.h>
#include <wx/html/htmlwin.h>

#ifndef _HELPDLG_H_
#define _HELPDLG_H_

class HelpDlg : public wxPanel
{
public:
	HelpDlg(wxWindow* frame,
			wxWindow* parent);
	~HelpDlg();

	void LoadPage(wxString name);

private:
	wxHtmlWindow *m_html;
	wxString m_name;

	DECLARE_EVENT_TABLE()
};

#endif//_HELPDLG_H_
