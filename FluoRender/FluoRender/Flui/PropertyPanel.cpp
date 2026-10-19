/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2026 Scientific Computing and Imaging Institute,
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
#include <PropertyPanelAgent.h>

PropertyPanel::PropertyPanel(wxWindow* parent,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name) :
	TabbedPanel(parent, pos, size, style, name)
{
}

void PropertyPanel::CreateUI()
{
	wxEventBlocker blocker(this);
	Freeze();
	m_notebook = new wxAuiNotebook(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize,
		wxAUI_NB_DEFAULT_STYLE | wxAUI_NB_TAB_EXTERNAL_MOVE |
		wxAUI_NB_WINDOWLIST_BUTTON | wxNO_BORDER);
	m_notebook->Bind(wxEVT_AUINOTEBOOK_PAGE_CLOSE, &PropertyPanel::OnPropPageClose, this);
	wxBoxSizer* sizerv = new wxBoxSizer(wxVERTICAL);
	sizerv->Add(m_notebook, 1, wxEXPAND);
	SetSizer(sizerv);
	Layout();
	SetAutoLayout(true);
	SetScrollRate(10, 10);
	Thaw();
}

void PropertyPanel::RegisterPage(
	const PropPageInfo& info)
{
	m_prop_pages.push_back({
		info.key,
		info.panel});
}

void PropertyPanel::ShowPage(
	const PropPageInfo& info)
{
	auto page = FindPage(info.key);

	if (!page || !page->panel)
		return;

	int page_no =
		m_notebook->FindPage(page->panel);

	if (page_no == wxNOT_FOUND)
	{
		m_notebook->AddPage(
			page->panel,
			info.title,
			true);

		page->panel->Show();
	}
	else
	{
		m_notebook->SetSelection(page_no);
	}
}

void PropertyPanel::HidePage(
	const PropPageInfo& info)
{
	auto page = FindPage(info.key);

	if (!page || !page->panel)
		return;

	int page_no =
		m_notebook->FindPage(page->panel);

	if (page_no == wxNOT_FOUND)
		return;

	page->panel->Hide();

	m_notebook->RemovePage(page_no);
}

void PropertyPanel::SelectPage(
	const PropPageInfo& info)
{
	auto page = FindPage(info.key);

	if (!page || !page->panel)
		return;

	int page_no =
		m_notebook->FindPage(page->panel);

	if (page_no != wxNOT_FOUND)
		m_notebook->SetSelection(page_no);
}

void PropertyPanel::RenamePage(
	const PropPageInfo& info)
{
	auto page = FindPage(info.key);

	if (!page || !page->panel)
		return;

	int page_no =
		m_notebook->FindPage(page->panel);

	if (page_no != wxNOT_FOUND)
	{
		m_notebook->SetPageText(
			page_no,
			info.title);
	}
}

void PropertyPanel::DeletePage(
	const PropPageInfo& info)
{
	auto it = std::find_if(
		m_prop_pages.begin(),
		m_prop_pages.end(),
		[&](const PropUiPage& page)
		{
			return page.key == info.key;
		});

	if (it == m_prop_pages.end())
		return;

	if (it->panel)
	{
		int page_no =
			m_notebook->FindPage(it->panel);

		if (page_no != wxNOT_FOUND)
			m_notebook->RemovePage(page_no);

		it->panel->Destroy();
	}

	m_prop_pages.erase(it);
}

PropUiPage*
PropertyPanel::FindPage(const PageKey& key)
{
	auto it = std::find_if(
		m_prop_pages.begin(),
		m_prop_pages.end(),
		[&](const PropUiPage& page)
		{
			return page.key == key;
		});

	return it == m_prop_pages.end() ?
		nullptr :
		&(*it);
}

PropUiPage*
PropertyPanel::FindPage(
	AgentPanel* panel)
{
	auto it = std::find_if(
		m_prop_pages.begin(),
		m_prop_pages.end(),
		[&](const PropUiPage& page)
		{
			return page.panel == panel;
		});

	return it == m_prop_pages.end() ?
		nullptr :
		&(*it);
}

void PropertyPanel::OnPropPageClose(
	wxAuiNotebookEvent& event)
{
	if (!m_notebook)
		return;

	auto* page =
		dynamic_cast<AgentPanel*>(
			m_notebook->GetPage(
				event.GetSelection()));

	if (!page)
		return;

	page->Hide();

	m_notebook->RemovePage(
		event.GetSelection());

	m_page = page;
	auto agent = GetAgentAs<PropertyPanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstPropertyPageClose });

	event.Veto();
}