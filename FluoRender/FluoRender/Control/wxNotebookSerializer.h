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
#ifndef WXNOTEBOOKSERIALIZER_H
#define WXNOTEBOOKSERIALIZER_H

#include <BaseTreeFile.h>
#include <wx/aui/serializer.h>
#include <string>
#include <memory>

// Sample serializer and deserializer implementations for saving and loading layouts.
class wxNotebookSerializer : public wxAuiSerializer
{
public:
	wxNotebookSerializer() = default;

	std::string GetConfig()
	{
		return m_config_str;
	}

	// Implement wxAuiSerializer methods.
	virtual void BeforeSave() override;

	virtual void BeforeSavePanes() override;

	virtual void SavePane(const wxAuiPaneLayoutInfo& pane) override;

	virtual void AfterSavePanes() override;

	virtual void BeforeSaveNotebooks() override;

	virtual void BeforeSaveNotebook(const wxString& name) override;

	virtual void SaveNotebookTabControl(const wxAuiTabLayoutInfo& tab) override;

	virtual void AfterSaveNotebook() override;

	virtual void AfterSaveNotebooks() override;

	virtual void AfterSave() override;

private:
	void AddChild(const wxString& name, const wxString& value);

	void AddChild(const wxString& name, int value);

	void AddChild(const wxString& name, const wxRect& rect);

	// Common helper of SavePane() and SaveNotebookTabControl() which both need
	// to save the same layout information.
	void AddDockLayout(const wxAuiDockLayoutInfo& layout);

	// Helper of SaveNotebookTabControl(): add a node with the given name
	// containing the comma-separated list of page indices if there are any.
	void AddPagesList(
		const wxString& name,
		const std::vector<int>& pages);

	std::string m_config_str;
	std::unique_ptr<BaseTreeFile> m_config;
	//wxXmlDocument m_doc;

	// Non-owning pointer to the root node of m_doc.
	//wxXmlNode* m_root = nullptr;

	// The other pointers are only set temporarily, until they are added to the
	// document -- this ensures that we don't leak memory if an exception is
	// thrown before this happens.
	//std::unique_ptr<wxXmlNode> m_panes;
	//std::unique_ptr<wxXmlNode> m_books;
	//std::unique_ptr<wxXmlNode> m_book;
};

class wxNotebookDeserializer : public wxAuiDeserializer
{
public:
	explicit wxNotebookDeserializer(wxAuiManager& manager);

	void SetConfig(const std::string& str)
	{
		m_config_str = str;
	}

	// Implement wxAuiDeserializer methods.
	virtual std::vector<wxAuiPaneLayoutInfo> LoadPanes() override;

	virtual std::vector<wxAuiTabLayoutInfo>
		LoadNotebookTabs(const wxString& name) override;

	// Overriding this function is optional and normally it is not going to be
	// called at all. We only do it here to show the different possibilities,
	// but the serialized pages need to be manually edited to see them.
	virtual bool
		HandleOrphanedPage(wxAuiNotebook& book,
			int page,
			wxAuiTabCtrl** tabCtrl,
			int* tabIndex) override;

	virtual wxWindow* CreatePaneWindow(wxAuiPaneInfo& pane) override;

private:
	int GetInt(const wxString& str);

	wxSize GetSize(const wxString& str);

	wxRect GetRect(const wxString& str);

	// Common helper of LoadPanes() and LoadNotebookTabs() which both need to
	// load the dock layout information.
	//
	// Returns true if we processed this node.
	bool LoadDockLayout(wxAuiDockLayoutInfo& info);

	std::vector<wxAuiTabLayoutInfo> LoadNotebookTabs();

	std::string m_config_str;
	std::unique_ptr<BaseTreeFile> m_config;
	//wxXmlDocument m_doc;

	// Non-owning pointers to the nodes in m_doc.
	//wxXmlNode* m_panes = nullptr;
	//wxXmlNode* m_books = nullptr;
};

#endif// WXNOTEBOOKSERIALIZER_H