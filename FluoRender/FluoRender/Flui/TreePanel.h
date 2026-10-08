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
#ifndef _TREEPANEL_H_
#define _TREEPANEL_H_

#include <AgentOwner.h>
#include <wx/wx.h>
#include <wx/treectrl.h>
#include <vector>
#include <unordered_map>

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
	wxTreeItemId AddAnnotItem(wxTreeItemId par_item, const wxString &text);
	void SetAnnotItemImage(const wxTreeItemId item, int image);
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
using TreeNodeId = std::uintptr_t;
enum class InteractiveMode : int;
namespace flrd
{
	enum class SelectMode : int;
	enum class RulerMode : int;
}
enum class TreeNodeType : int
{
	Root = 0,
	View,
	Volume,
	Mesh,
	Annotation,
	VolumeGroup,
	MeshGroup
};

struct TreeColor
{
	uint8_t r = 255;
	uint8_t g = 255;
	uint8_t b = 255;
};

struct TreeItemData
{
	TreeNodeId id = 0;

	TreeNodeType type = TreeNodeType::Root;

	std::wstring name;

	bool visible = true;

	TreeColor color;

	std::vector<TreeItemData> children;
};

struct TreeUpdateData
{
	TreeItemData root;

	TreeNodeId selectedId = 0;
};

struct TreeIconUpdate
{
	TreeNodeId id = 0;

	bool visible = true;
};

struct TreeIconUpdateData
{
	std::vector<TreeIconUpdate> items;
};

struct TreeColorUpdate
{
	TreeNodeId id = 0;

	TreeColor color;
};

struct TreeColorUpdateData
{
	std::vector<TreeColorUpdate> items;
};

struct TreeSelectionData
{
	TreeNodeId selectedId = 0;
};

struct TreeUiInfo
{
	TreeNodeType type;

	int iconIndex = -1;
};

struct MenuItemData
{
	enum class Type
	{
		Action,
		Separator,
		SubMenu
	};

	Type type = Type::Action;

	int id = -1;
	std::wstring label;

	bool enabled = true;
	bool checked = false;

	std::vector<MenuItemData> children;
};
using MenuData = std::vector<MenuItemData>;

class TreePanel : public AgentPanel
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
		ID_RulerLocator,
		ID_RulerLine,
		ID_RulerPolyline,
		ID_RulerPencil,
		//separator
		ID_RulerEdit,
		ID_RulerDeletePoint,
		//brush
		ID_BrushGrow,
		ID_BrushAppend,
		ID_BrushDiffuse,
		ID_BrushUnselect,
		//separator
		ID_MeshConvert,
		ID_BrushRuler,
		ID_BrushComp,
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

	TreePanel(
		wxWindow* parent,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0,
		const wxString& name = "TreePanel");
	~TreePanel();

	//update
	void UpdateFreehandToolState(InteractiveMode int_mode,
		flrd::SelectMode sel_mode, flrd::RulerMode rul_mode);
	void UpdateTree(const TreeUpdateData& data);
	void UpdateTreeIcons(const TreeIconUpdateData& data);
	void UpdateTreeColors(const TreeColorUpdateData& data);
	void UpdateTreeSelection(const TreeSelectionData& data);
	void UpdateExpandSelectedItem();
	void UpdateScrollPos();

	void ShowContextMenu(const MenuData& data);

	//get
	bool GetTreeExpanded();
	std::wstring GetSelItemText();
	std::wstring GetSelItemParentText();
	LayerInfo* GetSelItemData();
	bool GetCtrlDown();

private:
	wxTreeItemId BuildTreeItem(const TreeItemData& node, wxTreeItemId parent = wxTreeItemId());
	void SelectItem(TreeNodeId id);
	void UpdateItemColor(TreeNodeId id, const TreeColor& color);
	void UpdateItemIcon(TreeNodeId id, bool visible);

private:
	DataTreeCtrl* m_datatree;
	wxToolBar *m_toolbar;
	wxToolBar* m_toolbar2;

	//save the pos
	int m_scroll_pos;
	bool m_suppress_event = false;
	//context menu pos
	wxPoint m_context_pos;
	//drag
	wxTreeItemId m_drag_item;
	//for updates
	std::unordered_map<TreeNodeId, wxTreeItemId> itemMap_;
	std::unordered_map<TreeNodeId, TreeUiInfo> itemInfo_;

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
