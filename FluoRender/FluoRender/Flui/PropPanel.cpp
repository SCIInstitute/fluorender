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
#include <PropPanel.h>
#include <Global.h>
#include <Names.h>
#include <MainFrame.h>
#include <RenderCanvas.h>
#include <RenderViewPanel.h>
#include <RenderView.h>
#include <Root.h>
#include <DataManager.h>
#include <RefreshScheduler.h>
#include <wxNotebookSerializer.h>

PropBase::PropBase(MainFrame* frame):
	m_frame(frame)
{
}

void PropBase::FluoRefresh(int excl_self,
	const fluo::ValueCollection& vc, const std::set<int>& views)
{
	if (!m_frame)
		return;
	int view_excl = 0;
	if (dynamic_cast<RenderViewPanel*>(this))
		view_excl = excl_self;
	int origin_id = 0;
	if (vc.find(gstCamRotation) != vc.end())
		origin_id = *views.begin();
	glbin_refresh_scheduler_manager.requestDraw(DrawRequest("PropBase refresh", views,
		true, false, true, false, true, origin_id));
	wxWindow* win = dynamic_cast<wxWindow*>(this);
	m_frame->UpdateProps(vc, excl_self, win);//update ui but exclude this
}

double PropBase::getDpiScaleFactor()
{
	wxWindow* win = dynamic_cast<wxWindow*>(this);
	if (win)
	{
		double dpi_sf = win->GetDPIScaleFactor();
		double dpi_sf2 = std::round(dpi_sf - 0.1);
		return dpi_sf2 < dpi_sf ? dpi_sf : 1;
	}
	return 1;
}

PropPanel::PropPanel(MainFrame* frame,
	wxWindow* parent,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name) :
	PropBase(frame),
	wxScrolledWindow(parent, wxID_ANY, pos, size, style, name)
{

}

PropDialog::PropDialog(MainFrame* frame,
	wxWindow* parent,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name) :
	PropBase(frame),
	wxDialog(parent, wxID_ANY, name, pos, size, style)
{

}

TabbedPanel::TabbedPanel(MainFrame* frame,
	wxWindow* parent,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name) :
	PropPanel(frame, parent, pos, size, style, name)
{
}

void TabbedPanel::LoadPerspective(const std::string& str)
{
	if (!m_notebook)
		return;

	wxAuiManager mgr;
	wxNotebookDeserializer deser(mgr);
	deser.SetXML(str);
	m_notebook->LoadLayout(GetName(), deser);
}

std::string TabbedPanel::SavePerspective()
{
	if (!m_notebook)
		return "";

	wxNotebookSerializer ser;
	ser.BeforeSave();
	ser.BeforeSaveNotebooks();
	m_notebook->SaveLayout(GetName(), ser);
	ser.AfterSaveNotebooks();
	ser.AfterSave();
	return ser.GetXML().ToStdString();
}

void TabbedPanel::AddPage(wxWindow* page, const wxString& caption, bool select)
{
	if (!m_notebook || !page)
		return;
	m_notebook->AddPage(page, caption, select);
}

int TabbedPanel::FindPage(const wxWindow* page) const
{
	if (!m_notebook || !page)
		return -1;
	return m_notebook->FindPage(page);
}

bool TabbedPanel::DeletePage(size_t page)
{
	if (!m_notebook || page >= m_notebook->GetPageCount())
		return false;
	return m_notebook->DeletePage(page);
}

bool TabbedPanel::DeleteAllPages()
{
	if (!m_notebook)
		return false;
	return m_notebook->DeleteAllPages();
}

int TabbedPanel::SetSelection(size_t page)
{
	if (!m_notebook || page >= m_notebook->GetPageCount())
		return -1;
	return m_notebook->SetSelection(page);
}
