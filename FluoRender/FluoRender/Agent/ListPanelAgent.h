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
#ifndef ListPanelAgent_h
#define ListPanelAgent_h

#include <Agent.h>
#include <Names.h>

enum class ListItemType : int
{
	Invalid = 0,
	Volume,
	Mesh,
	Annot
};

struct ListContextInfo
{
	ListItemType type;
	bool path_valid;
};
class ListPanel;
class ListPanelAgent : public Agent
{
public:
	ListPanelAgent(
		ListPanel* panel);

	virtual ~ListPanelAgent() = default;

	ListPanel* GetPanel() const;

	ListContextInfo GetListContextInfo();
	std::vector<std::wstring> GetViewNames();
	void SetSelName(const std::wstring& name);
	void SetCurrentSelection(ListItemType type, const std::wstring& name);

protected:
	std::span < const std::string_view>
		AcceptedValues() const override
	{
		return kAcceptedValues;
	}

	void UpdateUI(const UpdateRequest& request) override;

	void UpdateData(const UpdateRequest& request) override;

private:
	static constexpr std::string_view kAcceptedValues[] =
	{
		gstListCtrl,
		gstTreeLayerName,
		gstCurrentSelect,
		gstAddListSelToView,
		gstListSaveSelection,
		gstListBakeSelection,
		gstListSaveSelMask,
		gstListDeleteSelection,
		gstListDeleteAll
	};

	bool m_suppress_event = false;

private:
	void UpdateList();
	void UpdateSelection();

	void AddSelectionToView();
	void SaveSelection();
	void BakeSelection();
	void SaveSelMask();
	void DeleteSelection();
	void DeleteAll();
};

#endif // ListPanelAgent_h
