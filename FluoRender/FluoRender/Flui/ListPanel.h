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
#ifndef _LISTPANEL_H_
#define _LISTPANEL_H_

#include <PropPanel.h>
#include <wx/listctrl.h>

class DataListCtrl : public wxListCtrl
{
public:
	DataListCtrl(
		wxWindow* parent,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxLC_REPORT | wxLC_SINGLE_SEL);

	void SelectItemSilently(int i)
	{
		if (i < 0 || i >= GetItemCount())
			return;
		m_silent_select = true;
		SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
		m_silent_select = false;
	}

	void Append(int type, const wxString& name, const wxString& path);
	void SetText(long item, int col, const wxString &str);
	wxString GetText(long item, int col);
	void StartEdit();
	wxString EndEdit();

	friend class ListPanel;

private:
	wxTextCtrl *m_rename_text;
	long m_selected;
	bool m_silent_select = false;

private:
	void OnSelectionChanged(wxListEvent& event);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////

class ListPanel : public PropPanel
{
	enum
	{
		ID_LZW_COMP = 0,
		ID_CROP,
		ID_RESIZE_CHK,
		ID_RESIZE_X_TXT,
		ID_RESIZE_Y_TXT,
		ID_RESIZE_Z_TXT,
		ID_FILTER
	};

	enum
	{
		ID_AddToView = 0,
		ID_Rename,
		ID_Save,
		ID_Bake,
		ID_SaveMask,
		ID_Delete,
		ID_DeleteAll,
		ID_ViewID
	};

public:
	ListPanel(MainFrame* frame,
		wxWindow* parent,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0,
		const wxString& name = "ListPanel");
	~ListPanel();

	virtual void FluoUpdate(const fluo::ValueCollection& vc = {});

	void UpdateList();
	void UpdateSelection();

	void AddSelectionToView(int view);
	void AddSelToCurView();
	void RenameSelection(const std::wstring& name);
	void SaveSelection();
	void BakeSelection();
	void SaveSelMask();
	void DeleteSelection();
	void DeleteAll();

private:
	wxToolBar *m_toolbar;
	DataListCtrl *m_datalist;
	bool m_suppress_event = false;

private:
	void OnContextMenu(wxContextMenuEvent& event);
	void OnToolbar(wxCommandEvent& event);
	void OnMenu(wxCommandEvent& event);

	void OnSelect(wxListEvent& event);
	void OnAct(wxListEvent& event);
	void OnKeyDown(wxKeyEvent& event);
	void OnKeyUp(wxKeyEvent& event);
	void OnMouse(wxMouseEvent& event);
	void OnEndEditName(wxCommandEvent& event);
	void OnScrollWin(wxScrollWinEvent& event);
	void OnScroll(wxMouseEvent& event);

	void OnCropCheck(wxCommandEvent& event);
	void OnCompCheck(wxCommandEvent& event);
	void OnResizeCheck(wxCommandEvent& event);
	void OnSizeXText(wxCommandEvent& event);
	void OnSizeYText(wxCommandEvent& event);
	void OnSizeZText(wxCommandEvent& event);
	void OnFilterChange(wxCommandEvent& event);
	static wxWindow* CreateExtraControl(wxWindow* parent);
};

#endif//_LISTPANEL_H_
