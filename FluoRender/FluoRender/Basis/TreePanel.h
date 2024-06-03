/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2024 Scientific Computing and Imaging Institute,
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
#ifndef _TREEPANEL_H_
#define _TREEPANEL_H_

#include <PropPanel.h>
#include <wx/wx.h>
#include <wx/treectrl.h>

//tree icon
#define icon_change	1
#define icon_key	"None"

//tree item data
class LayerInfo : public wxTreeItemData
{
public:
	LayerInfo() :
		wxTreeItemData(),
		type(0) {}
	int type;	//0-root; 1-view; 
				//2-volume data; 3-mesh data;
				//5-group; 6-mesh group
};

class MainFrame;
class VolumeData;
class DataTreeCtrl: public wxTreeCtrl
{
public:
	DataTreeCtrl(
		wxWindow* parent,
		const wxPoint& pos=wxDefaultPosition,
		const wxSize& size=wxDefaultSize,
		long style=wxTR_HAS_BUTTONS|
		wxTR_TWIST_BUTTONS|
		wxTR_LINES_AT_ROOT|
		wxTR_NO_LINES|
		wxTR_FULL_ROW_HIGHLIGHT);
	~DataTreeCtrl();

private:
	wxTreeItemId m_selected;

	//icon operations
	//change the color of the icon dual
	void ChangeIconColor(int i, wxColor c);
	void AppendIcon();
	void ClearIcons();
	int GetIconNum();

	//void TraversalDelete(wxTreeItemId item);
	//int TraversalSelect(wxTreeItemId item, wxString name);
	//item operations
	//root item
	wxTreeItemId AddRootItem(const wxString &text);
	void ExpandRootItem();
	//view item
	wxTreeItemId AddViewItem(const wxString &text);
	void SetViewItemImage(const wxTreeItemId& item, int image);
	//volume data item
	wxTreeItemId AddVolItem(wxTreeItemId par_item, const wxString &text);
	void SetVolItemImage(const wxTreeItemId item, int image);
	//mesh data item
	wxTreeItemId AddMeshItem(wxTreeItemId par_item, const wxString &text);
	void SetMeshItemImage(const wxTreeItemId item, int image);
	//annotation item
	wxTreeItemId AddAnnotationItem(wxTreeItemId par_item, const wxString &text);
	void SetAnnotationItemImage(const wxTreeItemId item, int image);
	//group item
	wxTreeItemId AddGroupItem(wxTreeItemId par_item, const wxString &text);
	void SetGroupItemImage(const wxTreeItemId item, int image);
	//mesh group item
	wxTreeItemId AddMGroupItem(wxTreeItemId par_item, const wxString &text);
	void SetMGroupItemImage(const wxTreeItemId item, int image);

	//change the color of just one icon of the dual,
	//either enable(type=0), or disable(type=1)
	void ChangeIconColor(int which, wxColor c, int type);

	//events
	void OnSelectionChanged(wxTreeEvent& event);

	friend class TreePanel;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
class TreePanel : public PropPanel
{
public:
	enum
	{
		//toobar
		ID_ToggleDisp = 0,
		ID_AddVolGroup,
		ID_AddMeshGroup,
		ID_RemoveData,
		ID_BrushAppend,
		ID_BrushUnselect,
		ID_BrushDiffuse,
		ID_BrushErase,
		ID_BrushClear,
		ID_BrushExtract,
		//menu
		ID_Expand,
		ID_RandomizeColor,
		ID_CloseView,
		ID_Isolate,
		ID_ShowAll,
		ID_CopyMask,
		ID_PasteMask,
		ID_MergeMask,
		ID_ExcludeMask,
		ID_IntersectMask,
		ID_Edit,
		ID_Measurement,
		ID_Component,
		ID_Trace,
		ID_Calculations,
		ID_NoiseCancelling,
		ID_Counting,
		ID_Colocalization,
		ID_Convert,
		ID_Ocl,
		ID_MachineLearning,
		ID_ManipulateData
	};

	TreePanel(MainFrame* frame,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0,
		const wxString& name = "TreePanel");
	~TreePanel();

	virtual void LoadPerspective();
	virtual void SavePerspective();
	virtual void FluoUpdate(const fluo::ValueCollection& vc = {});

	//double click
	void Action();
	//update
	void UpdateTree();
	void UpdateTreeIcons();
	void UpdateTreeColors();
	void UpdateTreeSel();

	//toolbar operations
	void AddVolumeGroup();
	void AddMeshGroup();
	//delete all
	void DeleteSelection();
	void DeleteAll();

	//menu operations
	void Expand();
	void ToggleDisplay();
	void RandomizeColor();
	void CloseView();
	void Isolate();
	void ShowAll();
	void ManipulateData();

private:
	void traversalSel(wxTreeItemId item);

private:
	DataTreeCtrl* m_datatree;
	wxToolBar *m_toolbar;
	//save the pos
	int m_scroll_pos;
	//drag
	wxTreeItemId m_drag_item;

	void OnContextMenu(wxContextMenuEvent& event);
	void OnToolbar(wxCommandEvent& event);
	void OnMenu(wxCommandEvent& event);
	void OnSelChanged(wxTreeEvent& event);
	void OnSelChanging(wxTreeEvent& event);
	void OnDeleting(wxTreeEvent& event);
	void OnAct(wxTreeEvent& event);
	void OnBeginDrag(wxTreeEvent& event);
	void OnEndDrag(wxTreeEvent& event);
	void OnKeyDown(wxKeyEvent& event);
};

#endif//_TREEPANEL_H_
