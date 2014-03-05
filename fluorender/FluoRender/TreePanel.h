#include <wx/wx.h>
#include <wx/treectrl.h>
#include "compatibility.h"

#ifndef _TREEPANEL_H_
#define _TREEPANEL_H_

//tree icon
#define icon_change	1
#define icon_key	"None"

//tree item data
class LayerInfo : public wxTreeItemData
{
public:
	int type;	//0-root; 1-view; 
				//2-volume data; 3-mesh data;
				//5-group; 6-mesh group
};

class DataTreeCtrl: public wxTreeCtrl
{
	enum
	{
		ID_TreeCtrl = wxID_HIGHEST+501,
		ID_ToggleDisp,
		ID_Isolate,
		ID_RemoveData,
		ID_CloseView,
		ID_ManipulateData,
		ID_AddDataGroup,
		ID_AddMeshGroup,
		ID_Expand,
		ID_Edit,
		ID_Measurement,
		ID_NoiseCancelling,
		ID_Counting,
		ID_Colocalization,
		ID_RandomizeColor
	};

public:
	DataTreeCtrl(wxWindow* frame,
		wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos=wxDefaultPosition,
		const wxSize& size=wxDefaultSize,
		long style=wxTR_HAS_BUTTONS|
		wxTR_TWIST_BUTTONS|
		wxTR_LINES_AT_ROOT|
		wxTR_NO_LINES|
		wxTR_FULL_ROW_HIGHLIGHT);
	~DataTreeCtrl();

	//icon operations
	//change the color of the icon dual
	void ChangeIconColor(int i, wxColor c);
	void AppendIcon();
	void ClearIcons();
	int GetIconNum();

	//item operations
	//delete all
	void DeleteAll();
	void DeleteSelection();
	//traversal delete
	void TraversalDelete(wxTreeItemId item);
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

	void UpdateSelection();
	wxString GetCurrentSel();
	int TraversalSelect(wxTreeItemId item, wxString name);
	void Select(wxString view, wxString name);

	//brush commands (from the panel)
	void BrushClear();
	void BrushCreate();
	void BrushCreateInv();

	friend class TreePanel;

private:
	wxWindow* m_frame;

	//drag
	wxTreeItemId m_drag_item;
	//fix current selection
	bool m_fixed;
	//remember the pos
	int m_scroll_pos;

private:
	//change the color of just one icon of the dual,
	//either enable(type=0), or disable(type=1)
	void ChangeIconColor(int which, wxColor c, int type);

	void OnContextMenu(wxContextMenuEvent &event );

	void OnToggleDisp(wxCommandEvent& event);
	void OnIsolate(wxCommandEvent& event);
	void OnRemoveData(wxCommandEvent& event);
	void OnCloseView(wxCommandEvent& event);
	void OnManipulateData(wxCommandEvent& event);
	void OnAddDataGroup(wxCommandEvent& event);
	void OnAddMeshGroup(wxCommandEvent& event);
	void OnExpand(wxCommandEvent& event);
	void OnEdit(wxCommandEvent& event);
	void OnMeasurement(wxCommandEvent& event);
	void OnNoiseCancelling(wxCommandEvent& event);
	void OnCounting(wxCommandEvent& event);
	void OnColocalization(wxCommandEvent& event);
	void OnRandomizeColor(wxCommandEvent& event);

	void OnSelChanged(wxTreeEvent& event);
	void OnSelChanging(wxTreeEvent& event);
	void OnDeleting(wxTreeEvent& event);
	void OnAct(wxTreeEvent &event);
	void OnBeginDrag(wxTreeEvent& event);
	void OnEndDrag(wxTreeEvent& event);

	void OnKeyDown(wxKeyEvent& event);
	void OnKeyUp(wxKeyEvent& event);

	DECLARE_EVENT_TABLE()
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

class TreePanel : public wxPanel
{
public:
	enum
	{
		ID_ToggleView = wxID_HIGHEST+551,
		ID_AddGroup,
		ID_AddMGroup,
		ID_RemoveData,
		ID_BrushAppend,
		ID_BrushDesel,
		ID_BrushDiffuse,
		ID_BrushErase,
		ID_BrushClear,
		ID_BrushCreate
	};

	TreePanel(wxWindow* frame,
		wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0,
		const wxString& name = "TreePanel");
	~TreePanel();

	DataTreeCtrl* GetTreeCtrl();

	//icon operations
	void ChangeIconColor(int i, wxColor c);
	void AppendIcon();
	void ClearIcons();
	int GetIconNum();

	//item operations
	void SelectItem(wxTreeItemId item);
	void Expand(wxTreeItemId item);
	void ExpandAll();
	void DeleteAll();
	void TraversalDelete(wxTreeItemId item);
	wxTreeItemId AddRootItem(const wxString &text);
	void ExpandRootItem();
	wxTreeItemId AddViewItem(const wxString &text);
	void SetViewItemImage(const wxTreeItemId& item, int image);
	wxTreeItemId AddVolItem(wxTreeItemId par_item, const wxString &text);
	void SetVolItemImage(const wxTreeItemId item, int image);
	wxTreeItemId AddMeshItem(wxTreeItemId par_item, const wxString &text);
	void SetMeshItemImage(const wxTreeItemId item, int image);
	wxTreeItemId AddAnnotationItem(wxTreeItemId par_item, const wxString &text);
	void SetAnnotationItemImage(const wxTreeItemId item, int image);
	wxTreeItemId AddGroupItem(wxTreeItemId par_item, const wxString &text);
	void SetGroupItemImage(const wxTreeItemId item, int image);
	wxTreeItemId AddMGroupItem(wxTreeItemId par_item, const wxString &text);
	void SetMGroupItemImage(const wxTreeItemId item, int image);

	//seelction
	void UpdateSelection();
	wxString GetCurrentSel();
	void Select(wxString view, wxString name);

	//set the brush icon down
	void SelectBrush(int id);
	int GetBrushSelected();
	//control from outside
	void BrushAppend();
	void BrushDiffuse();
	void BrushDesel();
	void BrushClear();
	void BrushErase();
	void BrushCreate();

private:
	wxWindow* m_frame;
	DataTreeCtrl* m_datatree;
	wxToolBar *m_toolbar;

	void OnToggleView(wxCommandEvent& event);
	void OnAddGroup(wxCommandEvent& event);
	void OnAddMGroup(wxCommandEvent& event);
	void OnRemoveData(wxCommandEvent& event);
	//brush commands
	void OnBrushAppend(wxCommandEvent& event);
	void OnBrushDesel(wxCommandEvent& event);
	void OnBrushDiffuse(wxCommandEvent& event);
	void OnBrushErase(wxCommandEvent& event);
	void OnBrushClear(wxCommandEvent& event);
	void OnBrushCreate(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()
};

#endif//_TREEPANEL_H_
