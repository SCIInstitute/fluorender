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

#include <wxNotebookSerializer.h>
#include <wx/aui/auibook.h>
#include <wx/sstream.h>

wxString wxNotebookSerializer::GetXML() const
{
	wxStringOutputStream oss;
	m_doc.Save(oss);

	return oss.GetString();
}

void wxNotebookSerializer::BeforeSave()
{
	m_root = new wxXmlNode(wxXML_ELEMENT_NODE, "aui-layout");
	m_root->AddAttribute("version", "3.3.0");

	m_doc.SetRoot(m_root);
}

void wxNotebookSerializer::BeforeSavePanes()
{
	m_panes.reset(new wxXmlNode(wxXML_ELEMENT_NODE, "panes"));
}

void wxNotebookSerializer::SavePane(const wxAuiPaneLayoutInfo& pane)
{
	auto node = new wxXmlNode(wxXML_ELEMENT_NODE, "pane");
	node->AddAttribute("name", pane.name);

	AddDockLayout(node, pane);

	AddChild(node, "floating-rect",
		wxRect(pane.floating_pos, pane.floating_size));

	// Don't bother creating many "maximized" nodes with value 0 when we
	// can have at most one of them with value 1.
	if (pane.is_maximized)
		AddChild(node, "maximized", 1);

	// Also don't mark visible pages (as most of them are) as being so.
	if (pane.is_hidden)
		AddChild(node, "hidden", 1);

	m_panes->AddChild(node);
}

void wxNotebookSerializer::AfterSavePanes()
{
	m_root->AddChild(m_panes.release());
}

void wxNotebookSerializer::BeforeSaveNotebooks()
{
	m_books.reset(new wxXmlNode(wxXML_ELEMENT_NODE, "notebooks"));
}

void wxNotebookSerializer::BeforeSaveNotebook(const wxString& name)
{
	m_book.reset(new wxXmlNode(wxXML_ELEMENT_NODE, "notebook"));
	m_book->AddAttribute("name", name);
}

void wxNotebookSerializer::SaveNotebookTabControl(const wxAuiTabLayoutInfo& tab)
{
	auto node = new wxXmlNode(wxXML_ELEMENT_NODE, "tab");

	AddDockLayout(node, tab);

	AddPagesList(node, "pages", tab.pages);
	AddPagesList(node, "pinned", tab.pinned);
	AddChild(node, "active", tab.active);

	m_book->AddChild(node);
}

void wxNotebookSerializer::AfterSaveNotebook()
{
	m_books->AddChild(m_book.release());
}

void wxNotebookSerializer::AfterSaveNotebooks()
{
	m_root->AddChild(m_books.release());
}

void wxNotebookSerializer::AfterSave()
{
}

void wxNotebookSerializer::AddChild(wxXmlNode* parent, const wxString& name, const wxString& value)
{
	auto node = new wxXmlNode(parent, wxXML_ELEMENT_NODE, name);
	node->AddChild(new wxXmlNode(wxXML_TEXT_NODE, {}, value));
}

void wxNotebookSerializer::AddChild(wxXmlNode* parent, const wxString& name, int value)
{
	// Don't save 0 values, they're the default.
	if (value)
		AddChild(parent, name, wxString::Format("%u", value));
}

void wxNotebookSerializer::AddChild(wxXmlNode* parent, const wxString& name, const wxRect& rect)
{
	if (rect.GetPosition() != wxDefaultPosition ||
		rect.GetSize() != wxDefaultSize)
	{
		AddChild(parent, name,
			wxString::Format("%d,%d %dx%d",
				rect.x, rect.y, rect.width, rect.height));
	}
}

void wxNotebookSerializer::AddDockLayout(wxXmlNode* node, const wxAuiDockLayoutInfo& layout)
{
	AddChild(node, "direction", layout.dock_direction);
	AddChild(node, "layer", layout.dock_layer);
	AddChild(node, "row", layout.dock_row);
	AddChild(node, "position", layout.dock_pos);
	AddChild(node, "proportion", layout.dock_proportion);
	AddChild(node, "size", layout.dock_size);
}

void wxNotebookSerializer::AddPagesList(wxXmlNode* node,
	const wxString& name,
	const std::vector<int>& pages)
{
	if (!pages.empty())
	{
		wxString pagesList;
		for (auto page : pages)
		{
			if (!pagesList.empty())
				pagesList << ',';

			pagesList << page;
		}

		AddChild(node, name, pagesList);
	}
}

void wxNotebookDeserializer::SetXML(const wxString& xml)
{
	if (xml.IsEmpty())
		return;

	wxStringInputStream iss(xml);
	if (!m_doc.Load(iss))
		return;

	const auto root = m_doc.GetDocumentNode();
	const auto layout = root->GetChildren();
	if (!layout)
		return;
	if (layout->GetName() != "aui-layout")
		return;
	if (layout->GetAttribute("version") != "3.3.0")
		return;
	if (layout->GetNext())
		return;

	// Check that we only have the top level nodes that we expect.
	//
	// This is nice to detect errors in this sample, but note that this
	// might not be the best strategy for a real application, which might
	// decide to to gracefully ignore unknown nodes instead of failing, or
	// at least save the format version in the XML file to be able to give
	// a better error message.
	for (wxXmlNode* node = layout->GetChildren(); node; node = node->GetNext())
	{
		if (node->GetName() == "panes")
		{
			if (m_panes)
				return;

			m_panes = node;
		}
		else if (node->GetName() == "notebooks")
		{
			if (m_books)
				return;

			m_books = node;
		}
	}
}

std::vector<wxAuiPaneLayoutInfo> wxNotebookDeserializer::LoadPanes()
{
	std::vector<wxAuiPaneLayoutInfo> panes;

	if (!m_panes)
		return panes;

	for (wxXmlNode* node = m_panes->GetChildren(); node; node = node->GetNext())
	{
		if (node->GetName() != "pane")
			continue;

		{
			wxAuiPaneLayoutInfo pane{ node->GetAttribute("name") };

			for (wxXmlNode* child = node->GetChildren(); child; child = child->GetNext())
			{
				if (LoadDockLayout(child, pane))
					continue;

				const wxString& name = child->GetName();
				const wxString& content = child->GetNodeContent();

				if (name == "floating-rect")
				{
					auto rect = GetRect(content);

					pane.floating_pos = rect.GetPosition();
					pane.floating_size = rect.GetSize();
				}
				else if (name == "maximized")
				{
					pane.is_maximized = GetInt(content) != 0;
				}
				else if (name == "hidden")
				{
					pane.is_hidden = GetInt(content) != 0;
				}
			}

			panes.push_back(pane);
		}
	}

	return panes;
}

std::vector<wxAuiTabLayoutInfo> wxNotebookDeserializer::LoadNotebookTabs(const wxString& name)
{
	if (!m_books)
		return std::vector<wxAuiTabLayoutInfo>();

	// Find the notebook with the given name.
	for (wxXmlNode* node = m_books->GetChildren(); node; node = node->GetNext())
	{
		if (node->GetName() != "notebook")
			continue;

		if (node->GetAttribute("name") == name)
			return LoadNotebookTabs(node);
	}

	return std::vector<wxAuiTabLayoutInfo>();
}

bool wxNotebookDeserializer::HandleOrphanedPage(
	wxAuiNotebook& book,
	int page,
	wxAuiTabCtrl** tabCtrl,
	int* tabIndex)
{
	// If the first ("Welcome") page is not attached, insert it in the
	// beginning of the main tab control.
	if (page == 0)
	{
		// We don't need to change tabCtrl, it's set to the main one by
		// default, but check that this is indeed the case.
		if (*tabCtrl != book.GetMainTabCtrl())
		{
			//wxLogWarning("Unexpected tab control for an orphaned page!");
			return false;
		}

		*tabIndex = 0;

		return true;
	}

	// This is completely artificial, but just remove the pages after the
	// third one if they are not attached to any tab control.
	return page <= 2;
}

wxWindow* wxNotebookDeserializer::CreatePaneWindow(wxAuiPaneInfo& pane)
{
	//wxLogWarning("Unknown pane \"%s\"", pane.name);
	return nullptr;
}

int wxNotebookDeserializer::GetInt(const wxString& str)
{
	int value = 0;
	str.ToInt(&value);
	return value;
}

wxSize wxNotebookDeserializer::GetSize(const wxString& str)
{
	wxString strH;
	const wxString strW = str.BeforeFirst('x', &strH);

	// Special case which wouldn't be parse by ToUInt() below.
	if (strW == "-1" && strH == strW)
		return wxDefaultSize;

	unsigned int w = 0, h = 0;
	strW.ToUInt(&w);
	strH.ToUInt(&h);

	return wxSize(w, h);
}

wxRect wxNotebookDeserializer::GetRect(const wxString& str)
{
	wxString strWH;
	const wxString strXY = str.BeforeFirst(' ', &strWH);

	wxString strY;
	const wxString strX = strXY.BeforeFirst(',', &strY);

	int x = 0, y = 0;
	strX.ToInt(&x);
	strY.ToInt(&y);

	return wxRect(wxPoint(x, y), GetSize(strWH));
}

bool wxNotebookDeserializer::LoadDockLayout(wxXmlNode* node, wxAuiDockLayoutInfo& info)
{
	const wxString& name = node->GetName();
	const wxString& content = node->GetNodeContent();

	if (name == "direction")
	{
		info.dock_direction = GetInt(content);
	}
	else if (name == "layer")
	{
		info.dock_layer = GetInt(content);
	}
	else if (name == "row")
	{
		info.dock_row = GetInt(content);
	}
	else if (name == "position")
	{
		info.dock_pos = GetInt(content);
	}
	else if (name == "proportion")
	{
		info.dock_proportion = GetInt(content);
	}
	else if (name == "size")
	{
		info.dock_size = GetInt(content);
	}
	else
	{
		return false;
	}

	return true;
}

std::vector<wxAuiTabLayoutInfo> wxNotebookDeserializer::LoadNotebookTabs(wxXmlNode* book)
{
	std::vector<wxAuiTabLayoutInfo> tabs;

	for (wxXmlNode* node = book->GetChildren(); node; node = node->GetNext())
	{
		if (node->GetName() != "tab")
			continue;

		wxAuiTabLayoutInfo tab;
		for (wxXmlNode* child = node->GetChildren(); child; child = child->GetNext())
		{
			if (LoadDockLayout(child, tab))
				continue;

			const auto& pageIndices = wxSplit(child->GetNodeContent(), ',');
			if (child->GetName() == "pages")
			{
				for (const auto& s : pageIndices)
					tab.pages.push_back(GetInt(s));
			}
			else if (child->GetName() == "pinned")
			{
				for (const auto& s : pageIndices)
					tab.pinned.push_back(GetInt(s));
			}
			else if (child->GetName() == "active")
			{
				tab.active = GetInt(child->GetNodeContent());
			}
		}

		tabs.push_back(tab);
	}

	return tabs;
}
