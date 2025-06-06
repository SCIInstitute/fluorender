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
#include <XmlFile.h>
#include <wx/aui/auibook.h>

void wxNotebookSerializer::BeforeSave()
{
	m_config = std::make_unique<XmlFile>();

	//m_root = new wxXmlNode(wxXML_ELEMENT_NODE, "aui-layout");
	//m_root->AddAttribute("version", "3.3.0");

	//m_doc.SetRoot(m_root);
}

void wxNotebookSerializer::BeforeSavePanes()
{
	//m_panes.reset(new wxXmlNode(wxXML_ELEMENT_NODE, "panes"));
}

void wxNotebookSerializer::SavePane(const wxAuiPaneLayoutInfo& pane)
{
	m_config->SetPath("./pane");
	//auto node = new wxXmlNode(wxXML_ELEMENT_NODE, "pane");
	m_config->Write("name", pane.name.ToStdString());
	//node->AddAttribute("name", pane.name);

	AddDockLayout(pane);

	AddChild("floating-rect",
		wxRect(pane.floating_pos, pane.floating_size));

	// Don't bother creating many "maximized" nodes with value 0 when we
	// can have at most one of them with value 1.
	if (pane.is_maximized)
		AddChild("maximized", 1);

	// Also don't mark visible pages (as most of them are) as being so.
	if (pane.is_hidden)
		AddChild("hidden", 1);

	//m_panes->AddChild(node);
}

void wxNotebookSerializer::AfterSavePanes()
{
	//m_root->AddChild(m_panes.release());
}

void wxNotebookSerializer::BeforeSaveNotebooks()
{
	//m_books.reset(new wxXmlNode(wxXML_ELEMENT_NODE, "notebooks"));
}

void wxNotebookSerializer::BeforeSaveNotebook(const wxString& name)
{
	//m_book.reset(new wxXmlNode(wxXML_ELEMENT_NODE, "notebook"));
	//m_book->AddAttribute("name", name);
}

void wxNotebookSerializer::SaveNotebookTabControl(const wxAuiTabLayoutInfo& tab)
{
	//auto node = new wxXmlNode(wxXML_ELEMENT_NODE, "tab");

	AddDockLayout(tab);

	AddPagesList("pages", tab.pages);
	AddPagesList("pinned", tab.pinned);
	AddChild("active", tab.active);

	//m_book->AddChild(node);
}

void wxNotebookSerializer::AfterSaveNotebook()
{
	//m_books->AddChild(m_book.release());
}

void wxNotebookSerializer::AfterSaveNotebooks()
{
	//m_root->AddChild(m_books.release());
}

void wxNotebookSerializer::AfterSave()
{
}

void wxNotebookSerializer::AddChild(const wxString& name, const wxString& value)
{
	m_config->Write(name.ToStdString(), value.ToStdString());
	//auto node = new wxXmlNode(parent, wxXML_ELEMENT_NODE, name);
	//node->AddChild(new wxXmlNode(wxXML_TEXT_NODE, {}, value));
}

void wxNotebookSerializer::AddChild(const wxString& name, int value)
{
	// Don't save 0 values, they're the default.
	if (value)
		AddChild(name, wxString::Format("%u", value));
}

void wxNotebookSerializer::AddChild(const wxString& name, const wxRect& rect)
{
	if (rect.GetPosition() != wxDefaultPosition ||
		rect.GetSize() != wxDefaultSize)
	{
		AddChild(name,
			wxString::Format("%d,%d %dx%d",
				rect.x, rect.y, rect.width, rect.height));
	}
}

void wxNotebookSerializer::AddDockLayout(const wxAuiDockLayoutInfo& layout)
{
	AddChild("direction", layout.dock_direction);
	AddChild("layer", layout.dock_layer);
	AddChild("row", layout.dock_row);
	AddChild("position", layout.dock_pos);
	AddChild("proportion", layout.dock_proportion);
	AddChild("size", layout.dock_size);
}

void wxNotebookSerializer::AddPagesList(
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

		AddChild(name, pagesList);
	}
}

wxNotebookDeserializer::wxNotebookDeserializer(wxAuiManager& manager)
	: wxAuiDeserializer(manager)
{
	//wxStringInputStream iss(xml);
	//if (!m_doc.Load(iss))
	//	throw std::runtime_error("Failed to load XML document.");

	//const auto root = m_doc.GetDocumentNode();
	//const auto layout = root->GetChildren();
	//if (!layout)
	//	throw std::runtime_error("Missing layout node");
	//if (layout->GetName() != "aui-layout")
	//	throw std::runtime_error("Unexpected XML node name");
	//if (layout->GetAttribute("version") != "3.3.0")
	//	throw std::runtime_error("Unexpected XML version");
	//if (layout->GetNext())
	//	throw std::runtime_error("Unexpected multiple layout nodes");

	// Check that we only have the top level nodes that we expect.
	//
	// This is nice to detect errors in this sample, but note that this
	// might not be the best strategy for a real application, which might
	// decide to to gracefully ignore unknown nodes instead of failing, or
	// at least save the format version in the XML file to be able to give
	// a better error message.
	//for (wxXmlNode* node = layout->GetChildren(); node; node = node->GetNext())
	//{
	//	if (node->GetName() == "panes")
	//	{
	//		if (m_panes)
	//			throw std::runtime_error("Unexpected multiple panes nodes");

	//		m_panes = node;
	//	}
	//	else if (node->GetName() == "notebooks")
	//	{
	//		if (m_books)
	//			throw std::runtime_error("Unexpected multiple notebooks nodes");

	//		m_books = node;
	//	}
	//	else
	//	{
	//		throw std::runtime_error("Unexpected node name");
	//	}
	//}
}

std::vector<wxAuiPaneLayoutInfo> wxNotebookDeserializer::LoadPanes()
{
	std::vector<wxAuiPaneLayoutInfo> panes;

	//for (wxXmlNode* node = m_panes->GetChildren(); node; node = node->GetNext())
	//{
	//	if (node->GetName() != "pane")
	//		throw std::runtime_error("Unexpected pane node name");

	//	{
	//		wxAuiPaneLayoutInfo pane{ node->GetAttribute("name") };

	//		for (wxXmlNode* child = node->GetChildren(); child; child = child->GetNext())
	//		{
	//			if (LoadDockLayout(child, pane))
	//				continue;

	//			const wxString& name = child->GetName();
	//			const wxString& content = child->GetNodeContent();

	//			if (name == "floating-rect")
	//			{
	//				auto rect = GetRect(content);

	//				pane.floating_pos = rect.GetPosition();
	//				pane.floating_size = rect.GetSize();
	//			}
	//			else if (name == "maximized")
	//			{
	//				pane.is_maximized = GetInt(content) != 0;
	//			}
	//			else if (name == "hidden")
	//			{
	//				pane.is_hidden = GetInt(content) != 0;
	//			}
	//			else
	//			{
	//				throw std::runtime_error("Unexpected pane child node name");
	//			}
	//		}

	//		panes.push_back(pane);
	//	}
	//}

	return panes;
}

std::vector<wxAuiTabLayoutInfo> wxNotebookDeserializer::LoadNotebookTabs(const wxString& name)
{
	// Find the notebook with the given name.
	//for (wxXmlNode* node = m_books->GetChildren(); node; node = node->GetNext())
	//{
	//	if (node->GetName() != "notebook")
	//		throw std::runtime_error("Unexpected notebook node name");

	//	if (node->GetAttribute("name") == name)
	//		return LoadNotebookTabs(node);
	//}

	// As above, this might not be the best thing to do in a real
	// application, where, perhaps, the XML file was saved by a newer
	// version of the problem, but here we do this for simplicity and to
	// make sure we detect any errors.
	//throw std::runtime_error("Notebook with the given name not found");
	return LoadNotebookTabs("");
}

bool wxNotebookDeserializer::HandleOrphanedPage(wxAuiNotebook& book,
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
	int value;
	if (!str.ToInt(&value))
		throw std::runtime_error("Failed to parse integer");

	return value;
}

wxSize wxNotebookDeserializer::GetSize(const wxString& str)
{
	wxString strH;
	const wxString strW = str.BeforeFirst('x', &strH);

	// Special case which wouldn't be parse by ToUInt() below.
	if (strW == "-1" && strH == strW)
		return wxDefaultSize;

	unsigned int w, h;
	if (!strW.ToUInt(&w) || !strH.ToUInt(&h))
		throw std::runtime_error("Failed to parse size");

	return wxSize(w, h);
}

wxRect wxNotebookDeserializer::GetRect(const wxString& str)
{
	wxString strWH;
	const wxString strXY = str.BeforeFirst(' ', &strWH);

	wxString strY;
	const wxString strX = strXY.BeforeFirst(',', &strY);

	int x, y;
	if (!strX.ToInt(&x) || !strY.ToInt(&y))
		throw std::runtime_error("Failed to parse position");

	return wxRect(wxPoint(x, y), GetSize(strWH));
}

bool wxNotebookDeserializer::LoadDockLayout(wxAuiDockLayoutInfo& info)
{
	const wxString& name = "";// node->GetName();
	const wxString& content = "";// node->GetNodeContent();

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

std::vector<wxAuiTabLayoutInfo> wxNotebookDeserializer::LoadNotebookTabs()
{
	std::vector<wxAuiTabLayoutInfo> tabs;

	//for (wxXmlNode* node = book->GetChildren(); node; node = node->GetNext())
	//{
	//	if (node->GetName() != "tab")
	//		throw std::runtime_error("Unexpected tab node name");

	//	wxAuiTabLayoutInfo tab;
	//	for (wxXmlNode* child = node->GetChildren(); child; child = child->GetNext())
	//	{
	//		if (LoadDockLayout(child, tab))
	//			continue;

	//		const auto& pageIndices = wxSplit(child->GetNodeContent(), ',');
	//		if (child->GetName() == "pages")
	//		{
	//			for (const auto& s : pageIndices)
	//				tab.pages.push_back(GetInt(s));
	//		}
	//		else if (child->GetName() == "pinned")
	//		{
	//			for (const auto& s : pageIndices)
	//				tab.pinned.push_back(GetInt(s));
	//		}
	//		else if (child->GetName() == "active")
	//		{
	//			tab.active = GetInt(child->GetNodeContent());
	//		}
	//		else
	//		{
	//			throw std::runtime_error("Unexpected tab child node name");
	//		}
	//	}

	//	tabs.push_back(tab);
	//}

	return tabs;
}
