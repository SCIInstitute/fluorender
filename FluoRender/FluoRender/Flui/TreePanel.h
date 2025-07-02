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

class VolumeData;
class TreePanel;
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

	void SelectItemSilently(const wxTreeItemId& item)
	{
		m_silent_select = true;
		SelectItem(item);
		m_silent_select = false;
	}

private:
	wxTreeItemId m_selected;
	bool m_silent_select = false;

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
		//rulers
		ID_RulerLine,
		ID_RulerPolyline,
		ID_RulerPencil,
		//separator
		ID_RulerEdit,
		ID_RulerDeletePoint,
		//brush
		ID_BrushRuler,
		//separator
		ID_BrushGrow,
		ID_BrushAppend,
		ID_BrushComp,
		ID_BrushDiffuse,
		ID_BrushUnselect,
		//separator
		ID_BrushClear,
		ID_BrushExtract,
		ID_BrushDelete,
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
		ID_Brush,
		ID_Measurement,
		ID_Component,
		ID_Track,
		ID_Calculation,
		ID_NoiseReduct,
		ID_VolumeSize,
		ID_Colocalization,
		ID_Convert,
		ID_Ocl,
		ID_MachineLearning,
		ID_ManipulateData
	};

	TreePanel(MainFrame* frame,
		wxWindow* parent,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0,
		const wxString& name = "TreePanel");
	~TreePanel();

	virtual void FluoUpdate(const fluo::ValueCollection& vc = {});

	//selection change
	void Select();

	//update
	void UpdateTree();
	void UpdateTreeIcons();
	void UpdateTreeColors();
	void UpdateTreeSel();

	//double click
	void Action();
	void AddVolGroup();
	void AddMeshGroup();
	void RemoveData();
	void RulerLine();
	void RulerPolyline();
	void RulerPencil();
	void RulerEdit();
	void RulerDeletePoint();
	void BrushRuler();
	void BrushGrow();
	void BrushAppend();
	void BrushComp();
	void BrushDiffuse();
	void BrushUnselect();
	void BrushClear();
	void BrushExtract();
	void BrushDelete();

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
	wxToolBar* m_toolbar2;

	//save the pos
	int m_scroll_pos;
	bool m_suppress_event = false;
	//drag
	wxTreeItemId m_drag_item;

	void OnContextMenu(wxContextMenuEvent& event);
	void OnToolbar(wxCommandEvent& event);
	void OnMenu(wxCommandEvent& event);
	void OnSelChanged(wxTreeEvent& event);
	void OnDeleting(wxTreeEvent& event);
	void OnAct(wxTreeEvent& event);
	void OnBeginDrag(wxTreeEvent& event);
	void OnEndDrag(wxTreeEvent& event);
	void OnKeyDown(wxKeyEvent& event);
};

#endif//_TREEPANEL_H_
