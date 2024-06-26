/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2018 Scientific Computing and Imaging Institute,
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
#include <wx/wx.h>
#include <wx/listctrl.h>

class MainFrame;
class VolumeData;
class DataListCtrl : public wxListCtrl
{
	enum
	{
		Menu_AddTo = ID_LIST_PANEL1,
		Menu_Prop,
		Menu_Del,
		Menu_Rename,
		Menu_Save,
		Menu_Bake,
		Menu_SaveMask,
		ID_RenameText,
		Menu_View_start
	};

	enum
	{
		ID_LZW_COMP,
		ID_CROP,
		ID_RESIZE_CHK,
		ID_RESIZE_X_TXT,
		ID_RESIZE_Y_TXT,
		ID_RESIZE_Z_TXT,
		ID_FILTER
	};

public:
	DataListCtrl(MainFrame *frame,
		wxWindow* parent,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxLC_REPORT | wxLC_SINGLE_SEL);
	~DataListCtrl();

	void Append(int type, wxString name, wxString path);
	wxString GetText(long item, int col);
	void SetText(long item, int col, wxString &str);
	void SetSelection(int type, wxString &name);
	void SaveSelMask();
	void SaveAllMasks();

	friend class ListPanel;

private:
	MainFrame* m_frame;
	static VolumeData* m_vd;

	wxTextCtrl *m_rename_text;

private:
	void DeleteSelection();
	void DeleteAll();
	void AddToView(int menu_index, long item);
	void EndEdit(bool update = true);

	//
	void OnContextMenu(wxContextMenuEvent &event);
	void OnAddToView(wxCommandEvent& event);
	void OnDelete(wxCommandEvent& event);
	void OnRename(wxCommandEvent& event);
	void OnCropCheck(wxCommandEvent& event);
	void OnCompCheck(wxCommandEvent &event);
	void OnResizeCheck(wxCommandEvent &event);
	void OnSizeXText(wxCommandEvent &event);
	void OnSizeYText(wxCommandEvent &event);
	void OnSizeZText(wxCommandEvent &event);
	void OnFilterChange(wxCommandEvent &event);
	static wxWindow* CreateExtraControl(wxWindow* parent);
	void OnSave(wxCommandEvent& event);
	void OnBake(wxCommandEvent& event);
	void OnSaveMask(wxCommandEvent& event);

	void OnSelect(wxListEvent &event);
	void OnAct(wxListEvent &event);
	void OnKeyDown(wxKeyEvent& event);
	void OnKeyUp(wxKeyEvent& event);
	void OnMouse(wxMouseEvent& event);
	void OnEndEditName(wxCommandEvent& event);
	void OnScroll(wxScrollWinEvent& event);
	void OnScroll(wxMouseEvent& event);

	DECLARE_EVENT_TABLE()
protected: //Possible TODO
	wxSize GetSizeAvailableForScrollTarget(const wxSize& size) {
		return size - GetEffectiveMinSize();
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////

class ListPanel : public PropPanel
{
	enum
	{
		ID_AddToView = ID_LIST_PANEL2,
		ID_Rename,
		ID_Save,
		ID_Bake,
		ID_SaveMask,
		ID_Delete,
		ID_DeleteAll
	};
public:
	ListPanel(MainFrame* frame,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0,
		const wxString& name = "ListPanel");
	~ListPanel();

	virtual void LoadPerspective();
	virtual void SavePerspective();
	virtual void FluoUpdate(const fluo::ValueCollection& vc = {});

	void UpdateList();

	void Append(int type, wxString name, wxString path);
	wxString GetText(long item, int col);
	void SetText(long item, int col, wxString &str);
	void DeleteAll();
	void DeleteAllItems();
	void SetSelection(int type, wxString &name);
	void SaveAllMasks();

private:
	DataListCtrl *m_datalist;
	wxToolBar *m_toolbar;

	void OnAddToView(wxCommandEvent& event);
	void OnRename(wxCommandEvent& event);
	void OnSave(wxCommandEvent& event);
	void OnBake(wxCommandEvent& event);
	void OnSaveMask(wxCommandEvent& event);
	void OnDelete(wxCommandEvent& event);
	void OnDeleteAll(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()
};

#endif//_LISTPANEL_H_
