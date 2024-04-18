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
#include <TreePanel.h>
#include <Global.h>
#include <MainFrame.h>
#include <RenderViewPanel.h>
#include <RenderCanvas.h>
#include <OutputAdjPanel.h>
#include <ListPanel.h>
#include <BrushToolDlg.h>
#include <ColocalizationDlg.h>
#include <MeasureDlg.h>
#include <TraceDlg.h>
#include <OclDlg.h>
#include <ComponentDlg.h>
#include <compatibility.h>
//resources
#include <Formats/png_resource.h>
#include "tick.xpm"
#include "cross.xpm"
#include <img/icons.h>

BEGIN_EVENT_TABLE(DataTreeCtrl, wxTreeCtrl)
	EVT_CONTEXT_MENU(DataTreeCtrl::OnContextMenu)
	EVT_MENU(ID_ToggleDisp, DataTreeCtrl::OnToggleDisp)
	EVT_MENU(ID_Isolate, DataTreeCtrl::OnIsolate)
	EVT_MENU(ID_ShowAll, DataTreeCtrl::OnShowAll)
	EVT_MENU(ID_RemoveData, DataTreeCtrl::OnRemoveData)
	EVT_MENU(ID_CloseView, DataTreeCtrl::OnCloseView)
	EVT_MENU(ID_ManipulateData, DataTreeCtrl::OnManipulateData)
	EVT_MENU(ID_AddDataGroup, DataTreeCtrl::OnAddDataGroup)
	EVT_MENU(ID_AddMeshGroup, DataTreeCtrl::OnAddMeshGroup)
	EVT_MENU(ID_Expand, DataTreeCtrl::OnExpand)
	EVT_MENU(ID_Edit, DataTreeCtrl::OnEdit)
	EVT_MENU(ID_Measurement, DataTreeCtrl::OnMeasurement)
	EVT_MENU(ID_Trace, DataTreeCtrl::OnTrace)
	EVT_MENU(ID_NoiseCancelling, DataTreeCtrl::OnNoiseCancelling)
	EVT_MENU(ID_Counting, DataTreeCtrl::OnCounting)
	EVT_MENU(ID_Colocalization, DataTreeCtrl::OnColocalization)
	EVT_MENU(ID_Convert, DataTreeCtrl::OnConvert)
	EVT_MENU(ID_Ocl, DataTreeCtrl::OnOcl)
	EVT_MENU(ID_Component, DataTreeCtrl::OnComponent)
	EVT_MENU(ID_Calculations, DataTreeCtrl::OnCalculations)
	EVT_MENU(ID_MachineLearning, DataTreeCtrl::OnMachineLearning)
	EVT_MENU(ID_RandomizeColor, DataTreeCtrl::OnRandomizeColor)
	EVT_MENU(ID_CopyMask, DataTreeCtrl::OnCopyMask)
	EVT_MENU(ID_PasteMask, DataTreeCtrl::OnPasteMask)
	EVT_MENU(ID_MergeMask, DataTreeCtrl::OnMergeMask)
	EVT_MENU(ID_ExcludeMask, DataTreeCtrl::OnExcludeMask)
	EVT_MENU(ID_IntersectMask, DataTreeCtrl::OnIntersectMask)
	EVT_TREE_SEL_CHANGED(wxID_ANY, DataTreeCtrl::OnSelChanged)
	EVT_TREE_SEL_CHANGING(wxID_ANY, DataTreeCtrl::OnSelChanging)
	EVT_TREE_DELETE_ITEM(wxID_ANY, DataTreeCtrl::OnDeleting)
	EVT_TREE_ITEM_ACTIVATED(wxID_ANY, DataTreeCtrl::OnAct)
	EVT_TREE_BEGIN_DRAG(wxID_ANY, DataTreeCtrl::OnBeginDrag)
	EVT_TREE_END_DRAG(wxID_ANY, DataTreeCtrl::OnEndDrag)
	EVT_KEY_DOWN(DataTreeCtrl::OnKeyDown)
	EVT_KEY_UP(DataTreeCtrl::OnKeyUp)
END_EVENT_TABLE()

DataTreeCtrl::DataTreeCtrl(
	MainFrame* frame,
	wxWindow* parent,
	const wxPoint& pos,
	const wxSize& size,
	long style) :
wxTreeCtrl(parent, wxID_ANY, pos, size, style),
	m_frame(frame),
	m_fixed(false),
	m_scroll_pos(-1)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);
	SetDoubleBuffered(true);

	wxImageList *images = new wxImageList(16, 16, true);
	wxIcon icons[2];
	icons[0] = wxIcon(cross_xpm);
	icons[1] = wxIcon(tick_xpm);
	images->Add(icons[0]);
	images->Add(icons[1]);
	AssignImageList(images);
	SetDoubleBuffered(true);
}

DataTreeCtrl::~DataTreeCtrl()
{
	TraversalDelete(GetRootItem());
}

//delete
void DataTreeCtrl::DeleteAll()
{
	if (!IsEmpty())
	{
		//safe deletion, may be unnecessary
		TraversalDelete(GetRootItem());
		DeleteAllItems();
	}
}

//traversal delete
void DataTreeCtrl::TraversalDelete(wxTreeItemId item)
{
	wxTreeItemIdValue cookie;
	wxTreeItemId child_item = GetFirstChild(item, cookie);
	if (child_item.IsOk())
		TraversalDelete(child_item);
	child_item = GetNextChild(item, cookie);
	while (child_item.IsOk())
	{
		TraversalDelete(child_item);
		child_item = GetNextChild(item, cookie);
	}

	LayerInfo* item_data = (LayerInfo*)GetItemData(item);
	delete item_data;
	SetItemData(item, 0);
}

void DataTreeCtrl::DeleteSelection()
{
	if (m_fixed)
		return;

	wxTreeItemId sel_item = GetSelection();
	bool refresh = false;

	if (sel_item.IsOk() && m_frame)
	{
		LayerInfo* item_data = (LayerInfo*)GetItemData(sel_item);
		if (item_data && item_data->type == 1)//view
		{
			wxString name = GetItemText(sel_item);
			m_frame->DeleteVRenderView(name);
		}

		wxString name_data = GetItemText(sel_item);
		wxTreeItemId par_item = GetItemParent(sel_item);
		if (!par_item.IsOk())
			return;
		wxString par_name = GetItemText(par_item);

		LayerInfo* par_item_data = (LayerInfo*)GetItemData(par_item);
		if (par_item_data)
		{
			switch (par_item_data->type)
			{
			case 1://view
				{
					RenderCanvas* view = m_frame->GetView(par_name);
					if (!view)
						break;
					LayerInfo* item_data = (LayerInfo*)GetItemData(sel_item);
					if (item_data)
					{
						if (item_data->type == 2)//volume data
						{
							VolumeData* vd = view->GetVolumeData(name_data);
							if (vd)
							{
								vd->SetDisp(true);
								view->RemoveVolumeData(name_data);
								if (view->GetVolMethod() == VOL_METHOD_MULTI)
								{
									OutputAdjPanel* adjust_view = m_frame->GetAdjustView();
									if (adjust_view)
									{
										adjust_view->SetRenderView(view);
										adjust_view->UpdateSync();
									}
								}
							}
						}
						else if (item_data->type == 3)//mesh data
						{
							MeshData* md = view->GetMeshData(name_data);
							if (md)
							{
								md->SetDisp(true);
								view->RemoveMeshData(name_data);
							}
						}
						else if (item_data->type == 4)//annotations
						{
							Annotations* ann = view->GetAnnotations(name_data);
							if (ann)
							{
								ann->SetDisp(true);
								view->RemoveAnnotations(name_data);
							}
						}
						else if (item_data->type == 5)//group
						{
							view->RemoveGroup(name_data);
						}
						else if (item_data->type == 6)//mesh group
						{
							view->RemoveGroup(name_data);
						}
					}
					refresh = true;
				}
				break;
			case 5://group
				{
					wxTreeItemId gpar_item = GetItemParent(par_item);
					wxString gpar_name = GetItemText(gpar_item);
					RenderCanvas* view = m_frame->GetView(gpar_name);
					if (!view)
						break;
					LayerInfo* item_data = (LayerInfo*)GetItemData(sel_item);
					if (item_data && item_data->type == 2)
						view->RemoveVolumeData(name_data);
					refresh = true;

					if (view->GetVolMethod() == VOL_METHOD_MULTI)
					{
						OutputAdjPanel* adjust_view = m_frame->GetAdjustView();
						if (adjust_view)
						{
							adjust_view->SetRenderView(view);
							adjust_view->UpdateSync();
						}
					}
				}
				break;
			case 6://mesh group
				{
					wxTreeItemId gpar_item = GetItemParent(par_item);
					wxString gpar_name = GetItemText(gpar_item);
					RenderCanvas* view = m_frame->GetView(gpar_name);
					if (!view)
						break;
					LayerInfo* item_data = (LayerInfo*)GetItemData(sel_item);
					if (item_data && item_data->type==3)
						view->RemoveMeshData(name_data);
					refresh = true;
			}
				break;
			}
		}
	}

	if (refresh)
	{
		glbin.set_tree_selection("");
		m_frame->GetTree()->FluoRefresh(false, 2, {gstTreeCtrl});
		m_frame->OnSelection(1);
	}
}

void DataTreeCtrl::OnContextMenu(wxContextMenuEvent &event )
{
	if (m_fixed)
		return;

	int flag;
	wxTreeItemId sel_item = HitTest(ScreenToClient(event.GetPosition()), flag);
	if (!m_frame) return;

	if (sel_item.IsOk())
	{
		SelectItem(sel_item);

		wxPoint point = event.GetPosition();
		// If from keyboard
		if (point.x == -1 && point.y == -1) {
			wxSize size = GetSize();
			point.x = size.x / 2;
			point.y = size.y / 2;
		} else {
			point = ScreenToClient(point);
		}

		wxMenu menu;
		LayerInfo* item_data = (LayerInfo*)GetItemData(sel_item);
		if (item_data)
		{
			switch (item_data->type)
			{
			case 0:  //root
				if (IsExpanded(sel_item))
					menu.Append(ID_Expand, "Collapse");
				else
					menu.Append(ID_Expand, "Expand");
				break;
			case 1:  //view
				{
					menu.Append(ID_ToggleDisp, "Toggle Visibility");
					if (IsExpanded(sel_item))
						menu.Append(ID_Expand, "Collapse");
					else
						menu.Append(ID_Expand, "Expand");
					menu.AppendSeparator();
					menu.Append(ID_RandomizeColor, "Randomize Colors");
					menu.Append(ID_AddDataGroup, "Add Volume Group");
					menu.Append(ID_AddMeshGroup, "Add Mesh Group");
					wxString str = GetItemText(sel_item);
					if (str != m_frame->GetView(0)->GetName())
						menu.Append(ID_CloseView, "Close");
				}
				break;
			case 2:  //volume data
				menu.Append(ID_ToggleDisp, "Toggle Visibility");
				menu.Append(ID_Isolate, "Isolate");
				menu.Append(ID_ShowAll, "Show All");
				menu.AppendSeparator();
				menu.Append(ID_RandomizeColor, "Randomize Colors");
				menu.Append(ID_AddDataGroup, "Add Volume Group");
				menu.Append(ID_RemoveData, "Delete");
				menu.AppendSeparator();
				menu.Append(ID_CopyMask, "Copy Mask");
				if (m_frame->m_vd_copy)
				{
					menu.Append(ID_PasteMask, "Paste Mask");
					menu.Append(ID_MergeMask, "Merge Mask");
					menu.Append(ID_ExcludeMask, "Exclude Mask");
					menu.Append(ID_IntersectMask, "Intersect Mask");
				}
				menu.AppendSeparator();
				menu.Append(ID_Edit, "Paint Brush...");
				menu.Append(ID_Measurement, "Measurement...");
				menu.Append(ID_Component, "Component Analyzer...");
				menu.Append(ID_Trace, "Tracking...");
				menu.Append(ID_Calculations, "Calculations...");
				menu.Append(ID_NoiseCancelling, "Noise Reduction...");
				menu.Append(ID_Counting, "Volume Size...");
				menu.Append(ID_Colocalization, "Colocalization...");
				menu.Append(ID_Convert, "Convert...");
				menu.Append(ID_Ocl, "OpenCL Kernel Editor...");
				menu.Append(ID_MachineLearning, "Machine Learning Manager...");
				break;
			case 3:  //mesh data
				menu.Append(ID_ToggleDisp, "Toggle Visibility");
				menu.Append(ID_Isolate, "Isolate");
				menu.Append(ID_ShowAll, "Show All");
				menu.AppendSeparator();
				menu.Append(ID_RandomizeColor, "Randomize Colors");
				menu.Append(ID_AddMeshGroup, "Add Mesh Group");
				menu.Append(ID_RemoveData, "Delete");
				menu.AppendSeparator();
				menu.Append(ID_ManipulateData, "Manipulate");
				break;
			case 4:  //annotations
				break;
			case 5:  //data group
				menu.Append(ID_ToggleDisp, "Toggle Visibility");
				menu.Append(ID_Isolate, "Isolate");
				menu.Append(ID_ShowAll, "Show All");
				menu.AppendSeparator();
				if (IsExpanded(sel_item))
					menu.Append(ID_Expand, "Collapse");
				else
					menu.Append(ID_Expand, "Expand");
				menu.AppendSeparator();
				menu.Append(ID_RandomizeColor, "Randomize Colors");
				menu.Append(ID_AddDataGroup, "Add Volume Group");
				menu.Append(ID_RemoveData, "Delete");
				break;
			case 6:  //mesh group
				menu.Append(ID_ToggleDisp, "Toggle Visibility");
				menu.Append(ID_Isolate, "Isolate");
				menu.Append(ID_ShowAll, "Show All");
				menu.AppendSeparator();
				if (IsExpanded(sel_item))
					menu.Append(ID_Expand, "Collapse");
				else
					menu.Append(ID_Expand, "Expand");
				menu.AppendSeparator();
				menu.Append(ID_RandomizeColor, "Randomize Colors");
				menu.Append(ID_AddMeshGroup, "Add Mesh Group");
				menu.Append(ID_RemoveData, "Delete");
				break;
			}
			PopupMenu( &menu, point.x, point.y );
		}
	}
}

void DataTreeCtrl::OnToggleDisp(wxCommandEvent& event)
{
	wxTreeEvent tevent;
	OnAct(tevent);
}

void DataTreeCtrl::OnIsolate(wxCommandEvent& event)
{
	if (m_fixed)
		return;

	wxTreeItemId sel_item = GetSelection();
	bool refresh = false;

	if (sel_item.IsOk() && m_frame)
	{
		wxString viewname = "";
		wxString itemname = "";
		int item_type = 0;

		LayerInfo* item_data = (LayerInfo*)GetItemData(sel_item);
		if (item_data)
		{
			item_type = item_data->type;
			itemname = GetItemText(sel_item);
			wxTreeItemId par_item = GetItemParent(sel_item);
			if (par_item.IsOk())
			{
				LayerInfo* par_data = (LayerInfo*)GetItemData(par_item);
				if (par_data)
				{
					if (par_data->type == 1)
					{
						//view
						viewname = GetItemText(par_item);
					}
					else if (par_data->type == 5 ||
						par_data->type == 6)
					{
						wxTreeItemId gpar_item = GetItemParent(par_item);
						if (gpar_item.IsOk())
						{
							LayerInfo* gpar_data = (LayerInfo*)GetItemData(gpar_item);
							if (gpar_data && gpar_data->type==1)
							{
								//view
								viewname = GetItemText(gpar_item);
							}
						}
					}
				}
			}
		}

		RenderCanvas* view = m_frame->GetView(viewname);
		if (view)
		{
			view->Isolate(item_type, itemname);
			refresh = true;
		}

		UpdateSelection();
	}

	if (refresh)
	{
		m_frame->GetTree()->FluoRefresh(false, 2, { gstTreeIcons });
	}
}

void DataTreeCtrl::OnShowAll(wxCommandEvent& event)
{
	if (m_fixed)
		return;

	wxTreeItemId sel_item = GetSelection();
	bool refresh = false;

	if (sel_item.IsOk() && m_frame)
	{
		wxString viewname = "";
		wxString itemname = "";
		int item_type = 0;

		LayerInfo* item_data = (LayerInfo*)GetItemData(sel_item);
		if (item_data)
		{
			item_type = item_data->type;
			itemname = GetItemText(sel_item);
			wxTreeItemId par_item = GetItemParent(sel_item);
			if (par_item.IsOk())
			{
				LayerInfo* par_data = (LayerInfo*)GetItemData(par_item);
				if (par_data)
				{
					if (par_data->type == 1)
					{
						//view
						viewname = GetItemText(par_item);
					}
					else if (par_data->type == 5 ||
						par_data->type == 6)
					{
						wxTreeItemId gpar_item = GetItemParent(par_item);
						if (gpar_item.IsOk())
						{
							LayerInfo* gpar_data = (LayerInfo*)GetItemData(gpar_item);
							if (gpar_data && gpar_data->type==1)
							{
								//view
								viewname = GetItemText(gpar_item);
							}
						}
					}
				}
			}
		}

		RenderCanvas* view = m_frame->GetView(viewname);
		if (view)
		{
			view->ShowAll();
			refresh = true;
		}

		UpdateSelection();
	}

	if (refresh)
	{
		m_frame->GetTree()->FluoRefresh(false, 2, { gstTreeIcons });
	}
}

void DataTreeCtrl::OnRemoveData(wxCommandEvent& event)
{
	DeleteSelection();
}

void DataTreeCtrl::OnCloseView(wxCommandEvent& event)
{
	if (m_fixed)
		return;

	wxTreeItemId sel_item = GetSelection();

	if (sel_item.IsOk() && m_frame)
	{
		LayerInfo* item_data = (LayerInfo*)GetItemData(sel_item);
		if (item_data && item_data->type == 1)//view
		{
			wxString name = GetItemText(sel_item);
			m_frame->DeleteVRenderView(name);
		}
	}
}

void DataTreeCtrl::OnManipulateData(wxCommandEvent& event)
{
	if (m_fixed)
		return;

	wxTreeItemId sel_item = GetSelection();

	if (sel_item.IsOk() && m_frame)
	{
		LayerInfo* item_data = (LayerInfo*)GetItemData(sel_item);
		if (item_data && item_data->type == 3)//mesh data
		{
			wxString name = GetItemText(sel_item);
			MeshData* md = glbin_data_manager.GetMeshData(name);
			m_frame->OnSelection(6, 0, 0, 0, md);
		}
	}
}

void DataTreeCtrl::OnAddMeshGroup(wxCommandEvent &event)
{
	if (m_fixed)
		return;

	wxTreeItemId sel_item = GetSelection();
	if (!m_frame) return;

	if (!sel_item.IsOk())
	{
		wxTreeItemIdValue cookie;
		sel_item = GetFirstChild(GetRootItem(), cookie);
	}

	if (sel_item.IsOk())
	{
		LayerInfo* item_data = (LayerInfo*)GetItemData(sel_item);
		if (item_data && item_data->type == 1)
		{
			//view
			wxString name = GetItemText(sel_item);
			RenderCanvas* view = m_frame->GetView(name);
			if (view)
			{
				wxString group_name = view->AddMGroup("");
				AddMGroupItem(sel_item, group_name);
				Expand(sel_item);
			}
		}
		else if (item_data && item_data->type == 2)
		{
			//volume
			wxTreeItemId par_item = GetItemParent(sel_item);
			if (par_item.IsOk())
			{
				LayerInfo* par_data = (LayerInfo*) GetItemData(par_item);
				if (par_data && par_data->type == 1)
				{
					//volume in view
					wxString name = GetItemText(par_item);
					RenderCanvas* view = m_frame->GetView(name);
					if (view)
					{
						wxString group_name = view->AddMGroup("");
						AddMGroupItem(par_item, group_name);
					}
				}
				else if (par_data && par_data->type == 5)
				{
					//volume in group
					wxTreeItemId gpar_item = GetItemParent(par_item);
					if (gpar_item.IsOk())
					{
						LayerInfo* gpar_data = (LayerInfo*)GetItemData(gpar_item);
						if (gpar_data && gpar_data->type == 1)
						{
							wxString name = GetItemText(gpar_item);
							RenderCanvas* view = m_frame->GetView(name);
							if (view)
							{
								wxString group_name = view->AddMGroup("");
								AddMGroupItem(gpar_item, group_name);
							}
						}
					}
				}
			}
		}
		else if (item_data && item_data->type == 3)
		{
			//mesh
			wxTreeItemId par_item = GetItemParent(sel_item);
			if (par_item.IsOk())
			{
				LayerInfo* par_data = (LayerInfo*) GetItemData(par_item);
				if (par_data && par_data->type == 1)
				{
					//mesh in view
					wxString name = GetItemText(par_item);
					RenderCanvas* view = m_frame->GetView(name);
					if (view)
					{
						wxString group_name = view->AddMGroup("");
						AddMGroupItem(par_item, group_name);
					}
				}
				else if (par_data && par_data->type == 6)
				{
					//mesh in group
					wxTreeItemId gpar_item = GetItemParent(par_item);
					if (gpar_item.IsOk())
					{
						LayerInfo* gpar_data = (LayerInfo*)GetItemData(gpar_item);
						if (gpar_data && gpar_data->type == 1)
						{
							wxString name = GetItemText(gpar_item);
							RenderCanvas* view = m_frame->GetView(name);
							if (view)
							{
								wxString group_name = view->AddMGroup("");
								AddMGroupItem(gpar_item, group_name);
							}
						}
					}
				}
			}
		}
		else if ((item_data && item_data->type == 5) ||
			(item_data && item_data->type == 6))
		{
			//group
			wxTreeItemId par_item = GetItemParent(sel_item);
			if (par_item.IsOk())
			{
				LayerInfo* par_data = (LayerInfo*)GetItemData(par_item);
				if (par_data && par_data->type == 1)
				{
					//group in view
					wxString name = GetItemText(par_item);
					RenderCanvas* view = m_frame->GetView(name);
					if (view)
					{
						wxString group_name = view->AddMGroup("");
						AddMGroupItem(par_item, group_name);
					}
				}
			}
		}
	}
}

void DataTreeCtrl::OnAddDataGroup(wxCommandEvent& event)
{
	if (m_fixed)
		return;

	wxTreeItemId sel_item = GetSelection();
	if (!m_frame) return;

	if (!sel_item.IsOk())
	{
		wxTreeItemIdValue cookie;
		sel_item = GetFirstChild(GetRootItem(), cookie);
	}

	if (sel_item.IsOk())
	{
		LayerInfo* item_data = (LayerInfo*)GetItemData(sel_item);
		if (item_data && item_data->type == 1)
		{
			//view
			wxString name = GetItemText(sel_item);
			RenderCanvas* view = m_frame->GetView(name);
			if (view)
			{
				wxString group_name = view->AddGroup("");
				AddGroupItem(sel_item, group_name);
				Expand(sel_item);
			}
		}
		else if (item_data && item_data->type == 2)
		{
			//volume
			wxTreeItemId par_item = GetItemParent(sel_item);
			if (par_item.IsOk())
			{
				LayerInfo* par_data = (LayerInfo*) GetItemData(par_item);
				if (par_data && par_data->type == 1)
				{
					//volume in view
					wxString name = GetItemText(par_item);
					RenderCanvas* view = m_frame->GetView(name);
					if (view)
					{
						wxString group_name = view->AddGroup("");
						AddGroupItem(par_item, group_name);
					}
				}
				else if (par_data && par_data->type == 5)
				{
					//volume in group
					wxTreeItemId gpar_item = GetItemParent(par_item);
					if (gpar_item.IsOk())
					{
						LayerInfo* gpar_data = (LayerInfo*)GetItemData(gpar_item);
						if (gpar_data && gpar_data->type == 1)
						{
							wxString name = GetItemText(gpar_item);
							RenderCanvas* view = m_frame->GetView(name);
							if (view)
							{
								wxString group_name = view->AddGroup("");
								AddGroupItem(gpar_item, group_name);
							}
						}
					}
				}
			}
		}
		else if (item_data && item_data->type == 3)
		{
			//mesh
			wxTreeItemId par_item = GetItemParent(sel_item);
			if (par_item.IsOk())
			{
				LayerInfo* par_data = (LayerInfo*) GetItemData(par_item);
				if (par_data && par_data->type == 1)
				{
					//mesh in view
					wxString name = GetItemText(par_item);
					RenderCanvas* view = m_frame->GetView(name);
					if (view)
					{
						wxString group_name = view->AddGroup("");
						AddGroupItem(par_item, group_name);
					}
				}
				else if (par_data && par_data->type == 6)
				{
					//mesh in group
					wxTreeItemId gpar_item = GetItemParent(par_item);
					if (gpar_item.IsOk())
					{
						LayerInfo* gpar_data = (LayerInfo*)GetItemData(gpar_item);
						if (gpar_data && gpar_data->type == 1)
						{
							wxString name = GetItemText(gpar_item);
							RenderCanvas* view = m_frame->GetView(name);
							if (view)
							{
								wxString group_name = view->AddGroup("");
								AddGroupItem(gpar_item, group_name);
							}
						}
					}
				}
			}
		}
		else if ((item_data && item_data->type == 5) ||
			(item_data && item_data->type == 6))
		{
			//group
			wxTreeItemId par_item = GetItemParent(sel_item);
			if (par_item.IsOk())
			{
				LayerInfo* par_data = (LayerInfo*)GetItemData(par_item);
				if (par_data && par_data->type == 1)
				{
					//group in view
					wxString name = GetItemText(par_item);
					RenderCanvas* view = m_frame->GetView(name);
					if (view)
					{
						wxString group_name = view->AddGroup("");
						AddGroupItem(par_item,group_name);
					}
				}
			}
		}
	}
}

void DataTreeCtrl::OnExpand(wxCommandEvent &event)
{
	wxTreeItemId sel_item = GetSelection();
	if (IsExpanded(sel_item))
		Collapse(sel_item);
	else
		Expand(sel_item);
}

//edit
void DataTreeCtrl::OnEdit(wxCommandEvent &event)
{
	if (m_frame)
		m_frame->ShowPaintTool();
}

//measurement
void DataTreeCtrl::OnMeasurement(wxCommandEvent &event)
{
	if (m_frame)
		m_frame->ShowMeasureDlg();
}

//trace
void DataTreeCtrl::OnTrace(wxCommandEvent &event)
{
	if (m_frame)
		m_frame->ShowTraceDlg();
}

//noise cancelling
void DataTreeCtrl::OnNoiseCancelling(wxCommandEvent &event)
{
	if (m_frame)
		m_frame->ShowNoiseCancellingDlg();
}

//counting
void DataTreeCtrl::OnCounting(wxCommandEvent& event)
{
	if (m_frame)
		m_frame->ShowCountingDlg();
}

//colocalzation
void DataTreeCtrl::OnColocalization(wxCommandEvent& event)
{
	if (m_frame)
		m_frame->ShowColocalizationDlg();
}

//convert
void DataTreeCtrl::OnConvert(wxCommandEvent& event)
{
	if (m_frame)
		m_frame->ShowConvertDlg();
}

//ocl
void DataTreeCtrl::OnOcl(wxCommandEvent& event)
{
	if (m_frame)
		m_frame->ShowOclDlg();
}

//component
void DataTreeCtrl::OnComponent(wxCommandEvent& event)
{
	if (m_frame)
		m_frame->ShowComponentDlg();
}

//calculations
void DataTreeCtrl::OnCalculations(wxCommandEvent& event)
{
	if (m_frame)
		m_frame->ShowCalculationDlg();
}

//machine learning
void DataTreeCtrl::OnMachineLearning(wxCommandEvent& event)
{
	if (m_frame)
		m_frame->ShowMachineLearningDlg();
}

//randomize color
void DataTreeCtrl::OnRandomizeColor(wxCommandEvent& event)
{
	if (m_fixed)
		return;

	wxTreeItemId sel_item = GetSelection();
	if (!m_frame) return;
	if (!sel_item.IsOk()) return;

	LayerInfo* item_data = (LayerInfo*)GetItemData(sel_item);
	if (!item_data) return;

	wxString name = GetItemText(sel_item);
	if (item_data->type == 1)
	{
		//view
		RenderCanvas* view = m_frame->GetView(name);
		if (view)
			view->RandomizeColor();
	}
	else if (item_data->type == 2)
	{
		//volume
		VolumeData* vd = glbin_data_manager.GetVolumeData(name);
		if (vd)
			vd->RandomizeColor();
	}
	else if (item_data->type == 3)
	{
		//mesh
		MeshData* md = glbin_data_manager.GetMeshData(name);
		if (md)
			md->RandomizeColor();
	}
	else if (item_data->type == 5)
	{
		//volume group
		wxString par_name = GetItemText(GetItemParent(sel_item));
		RenderCanvas* view = m_frame->GetView(par_name);
		if (view)
		{
			DataGroup* group = view->GetGroup(name);
			if (group)
				group->RandomizeColor();
		}
	}
	else if (item_data->type == 6)
	{
		//mesh group
		wxString par_name = GetItemText(GetItemParent(sel_item));
		RenderCanvas* view = m_frame->GetView(par_name);
		if (view)
		{
			MeshGroup* group = view->GetMGroup(name);
			if (group)
				group->RandomizeColor();
		}
	}

	m_scroll_pos = GetScrollPos(wxVERTICAL);
	SetScrollPos(wxVERTICAL, m_scroll_pos);
	UpdateSelection();
	glbin.set_tree_selection(name.ToStdString());
	m_frame->GetTree()->FluoRefresh(false, 2, {gstTreeCtrl});
}

void DataTreeCtrl::OnCopyMask(wxCommandEvent& event)
{
	if (m_fixed)
		return;

	CopyMask(false);
}

void DataTreeCtrl::OnPasteMask(wxCommandEvent& event)
{
	PasteMask(0);
}

void DataTreeCtrl::OnMergeMask(wxCommandEvent& event)
{
	PasteMask(1);
}

void DataTreeCtrl::OnExcludeMask(wxCommandEvent& event)
{
	PasteMask(2);
}

void DataTreeCtrl::OnIntersectMask(wxCommandEvent& event)
{
	PasteMask(3);
}

//
void DataTreeCtrl::UpdateSelection()
{
	wxTreeItemId sel_item = GetSelection();

	if (!m_frame)
		return;

	//clear volume A for all views
	for (int i=0; i< m_frame->GetViewNum(); i++)
	{
		RenderCanvas* view = m_frame->GetView(i);
		if (view)
			view->SetVolumeA(0);
	}

	if (sel_item.IsOk())
	{
		//select data
		wxString name = GetItemText(sel_item);
		LayerInfo* item_data = (LayerInfo*)GetItemData(sel_item);
		if (item_data)
		{
			switch (item_data->type)
			{
			case 0://root
				m_frame->OnSelection(0);
				break;
			case 1://view
				//m_frame->OnSelection(0);
				{
					wxString str = GetItemText(sel_item);
					RenderCanvas* view = m_frame->GetView(str);
					m_frame->OnSelection(1, view, 0, 0, 0);
				}
				break;
			case 2://volume data
				{
					if (m_frame->GetAdjustView())
					{
						wxTreeItemId par_item = GetItemParent(sel_item);
						if (par_item.IsOk())
						{
							LayerInfo* par_item_data = (LayerInfo*)GetItemData(par_item);
							if (par_item_data && par_item_data->type == 5)
							{
								//par is group
								wxString str = GetItemText(GetItemParent(par_item));
								RenderCanvas* view = m_frame->GetView(str);
								if (view)
								{
									VolumeData* vd = glbin_data_manager.GetVolumeData(name);
									str = GetItemText(par_item);
									DataGroup* group = view->GetGroup(str);
									m_frame->GetAdjustView()->SetGroupLink(group);
									m_frame->OnSelection(2, view, group, vd, 0);
									view->SetVolumeA(vd);
								}
							}
							else if (par_item_data && par_item_data->type == 1)
							{
								//par is view
								wxString str = GetItemText(par_item);
								RenderCanvas* view = m_frame->GetView(str);
								if (view)
								{
									VolumeData* vd = glbin_data_manager.GetVolumeData(name);
									m_frame->GetAdjustView()->SetGroupLink(0);
									m_frame->OnSelection(2, view, 0, vd);
									view->SetVolumeA(vd);
								}
							}
						}
					}
				}
				break;
			case 3://mesh data
				{
					wxTreeItemId par_item = GetItemParent(sel_item);
					if (par_item.IsOk())
					{
						LayerInfo* par_item_data = (LayerInfo*)GetItemData(par_item);
						if (par_item_data && par_item_data->type == 6)
						{
							//par is group
							wxString str = GetItemText(GetItemParent(par_item));
							RenderCanvas* view = m_frame->GetView(str);
							if (view)
							{
								MeshData* md = glbin_data_manager.GetMeshData(name);
								m_frame->OnSelection(3, view, 0, 0, md);
							}
						}
						else if (par_item_data && par_item_data->type == 1)
						{
							//par is view
							wxString str = GetItemText(par_item);
							RenderCanvas* view = m_frame->GetView(str);
							if (view)
							{
								MeshData* md = glbin_data_manager.GetMeshData(name);
								m_frame->OnSelection(3, view, 0, 0, md);
							}
						}
					}
				}
				break;
			case 4://annotations
				{
					wxString par_name = GetItemText(GetItemParent(sel_item));
					RenderCanvas* view = m_frame->GetView(par_name);
					Annotations* ann = glbin_data_manager.GetAnnotations(name);
					m_frame->OnSelection(4, view, 0, 0, 0, ann);
				}
				break;
			case 5://group
				{
					wxString par_name = GetItemText(GetItemParent(sel_item));
					RenderCanvas* view = m_frame->GetView(par_name);
					if (view)
					{
						DataGroup* group = view->GetGroup(name);
						m_frame->OnSelection(5, view, group);
					}
				}
				break;
			case 6://mesh group
				{
					wxString par_name = GetItemText(GetItemParent(sel_item));
					RenderCanvas* view = m_frame->GetView(par_name);
					if (view)
					{
						m_frame->OnSelection(0);
					}
				}
				break;
			}

			if (item_data->type == 2 ||
				item_data->type == 3 ||
				item_data->type == 4)
			{
				int list_type = 0;
				if (item_data->type == 2)
					list_type = DATA_VOLUME;
				else if (item_data->type == 3)
					list_type = DATA_MESH;
				else if (item_data->type == 4)
					list_type = DATA_ANNOTATIONS;
				if (m_frame->GetList())
					m_frame->GetList()->SetSelection(list_type, name);
			}
		}
	}
}

wxString DataTreeCtrl::GetCurrentSel()
{
	wxTreeItemId sel_item = GetSelection();
	if (sel_item.IsOk())
	{
		return GetItemText(sel_item);
	}

	return "";
}

int DataTreeCtrl::TraversalSelect(wxTreeItemId item, wxString name)
{
	int found = 0;
	wxTreeItemIdValue cookie;
	wxTreeItemId child_item = GetFirstChild(item, cookie);
	if (child_item.IsOk())
		found = TraversalSelect(child_item, name);
	child_item = GetNextChild(item, cookie);
	while (!found && child_item.IsOk())
	{
		found = TraversalSelect(child_item, name);
		child_item = GetNextChild(item, cookie);
	}

	wxString item_name = GetItemText(item);
	if (item_name == name)
	{
		found = 1;
		SelectItem(item);
	}
	return found;
}

void DataTreeCtrl::Select(wxString view, wxString name)
{
	wxTreeItemIdValue cookie;
	wxTreeItemId root = GetRootItem();
	if (root.IsOk())
	{
		int found = 0;
		wxTreeItemId view_item = GetFirstChild(root, cookie);
		if (view_item.IsOk())
		{
			wxString view_name = GetItemText(view_item);
			if (view_name == view ||
				view == "")
			{
				if (name == "")
				{
					SelectItem(view_item);
					found = 1;
				}
				else
					found = TraversalSelect(view_item, name);
			}
		}
		view_item = GetNextChild(root, cookie);
		while (!found && view_item.IsOk())
		{
			wxString view_name = GetItemText(view_item);
			if (view_name == view ||
				view == "")
			{
				if (name == "")
				{
					SelectItem(view_item);
					found = 1;
				}
				else
					found = TraversalSelect(view_item, name);
			}
			view_item = GetNextChild(root, cookie);
		}

		if (!found)
			SelectItem(GetRootItem());
	}
}

void DataTreeCtrl::OnSelChanged(wxTreeEvent& event)
{
	UpdateSelection();
}

void DataTreeCtrl::OnSelChanging(wxTreeEvent &event)
{
	if (m_fixed)
		event.Veto();
}

void DataTreeCtrl::OnDeleting(wxTreeEvent& event)
{
	UpdateSelection();
}

void DataTreeCtrl::OnAct(wxTreeEvent &event)
{
	if (m_fixed)
		return;

	wxTreeItemId sel_item = GetSelection();
	wxString name = "";
	bool rc = wxGetKeyState(WXK_CONTROL);
	bool refresh = false;
	fluo::ValueCollection vc;

	if (sel_item.IsOk() && m_frame)
	{
		name = GetItemText(sel_item);
		LayerInfo* item_data = (LayerInfo*)GetItemData(sel_item);
		if (item_data)
		{
			switch (item_data->type)
			{
			case 1://view
				{
					RenderCanvas* view = m_frame->GetView(name);
					if (view)
					{
						if (rc)
							view->RandomizeColor();
						else
							view->ToggleDraw();
					}
				}
				break;
			case 2://volume data
				{
					VolumeData* vd = glbin_data_manager.GetVolumeData(name);
					if (vd)
					{
						if (rc)
							vd->RandomizeColor();
						else
						{
							vd->ToggleDisp();
							for (int i=0; i< m_frame->GetViewNum(); i++)
							{
								RenderCanvas* view = m_frame->GetView(i);
								if (view)
									view->SetVolPopDirty();
							}
						}
					}
				}
				break;
			case 3://mesh data
				{
					MeshData* md = glbin_data_manager.GetMeshData(name);
					if (md)
					{
						if (rc)
							md->RandomizeColor();
						else
						{
							md->ToggleDisp();
							for (int i=0; i< m_frame->GetViewNum(); i++)
							{
								RenderCanvas* view = m_frame->GetView(i);
								if (view)
									view->SetMeshPopDirty();
							}
						}
					}
				}
				break;
			case 4://annotations
				{
					Annotations* ann = glbin_data_manager.GetAnnotations(name);
					if (ann)
					{
						ann->ToggleDisp();
					}
				}
				break;
			case 5://group
				{
					wxString par_name = GetItemText(GetItemParent(sel_item));
					RenderCanvas* view = m_frame->GetView(par_name);
					if (view)
					{
						DataGroup* group = view->GetGroup(name);
						if (group)
						{
							if (rc)
								group->RandomizeColor();
							else
							{
								group->ToggleDisp();
								view->SetVolPopDirty();
							}
						}
					}
				}
				break;
			case 6://mesh group
				{
					wxString par_name = GetItemText(GetItemParent(sel_item));
					RenderCanvas* view = m_frame->GetView(par_name);
					if (view)
					{
						MeshGroup* group = view->GetMGroup(name);
						if (group)
						{
							if (rc)
								group->RandomizeColor();
							else
							{
								group->ToggleDisp();
								view->SetMeshPopDirty();
							}
						}
					}
				}
				break;
			}
		}

		m_scroll_pos = GetScrollPos(wxVERTICAL);
		refresh = true;
		if (rc)
		{
			vc.insert(gstTreeCtrl);
			glbin.set_tree_selection(name.ToStdString());
		}
		else
			vc.insert(gstTreeIcons);
		SetScrollPos(wxVERTICAL, m_scroll_pos);
		UpdateSelection();
	}
	if (refresh)
		m_frame->GetTree()->FluoRefresh(false, 2, vc);
}

void DataTreeCtrl::OnBeginDrag(wxTreeEvent& event)
{
	if (m_fixed)
		return;

	//remember pos
	m_scroll_pos = GetScrollPos(wxVERTICAL);

	m_drag_item = event.GetItem();
	if (m_drag_item.IsOk())
	{
		LayerInfo* item_data = (LayerInfo*)GetItemData(m_drag_item);
		if (item_data)
		{
			switch (item_data->type)
			{
			case 0://root
				break;
			case 1://view
				break;
			case 2://volume data
				event.Allow();
				break;
			case 3://mesh data
				event.Allow();
				break;
			case 4://annotations
				event.Allow();
				break;
			case 5://group
				event.Allow();
				break;
			case 6://mesh group
				event.Allow();
				break;
			}
		}
	}
}

void DataTreeCtrl::OnEndDrag(wxTreeEvent& event)
{
	if (m_fixed)
		return;

	wxTreeItemId src_item = m_drag_item,
		dst_item = event.GetItem(),
		src_par_item = src_item.IsOk()?GetItemParent(src_item):0,
		dst_par_item = dst_item.IsOk()?GetItemParent(dst_item):0;
	m_drag_item = (wxTreeItemId)0l;
	bool refresh = false;

	if (src_item.IsOk() && dst_item.IsOk() &&
		src_par_item.IsOk() &&
		dst_par_item.IsOk() && m_frame)
	{
		int src_type = ((LayerInfo*)GetItemData(src_item))->type;
		int src_par_type = ((LayerInfo*)GetItemData(src_par_item))->type;
		int dst_type = ((LayerInfo*)GetItemData(dst_item))->type;
		int dst_par_type = ((LayerInfo*)GetItemData(dst_par_item))->type;

		wxString src_name = GetItemText(src_item);
		wxString src_par_name = GetItemText(src_par_item);
		wxString dst_name = GetItemText(dst_item);
		wxString dst_par_name = GetItemText(dst_par_item);

		if (src_par_type == 1 &&
			dst_par_type == 1 &&
			src_par_name == dst_par_name &&
			src_name != dst_name)
		{
			//move within the same view
			if (src_type == 2 && dst_type == 5)
			{
				//move volume to the group in the same view
				RenderCanvas* view = m_frame->GetView(src_par_name);
				if (view)
				{
					wxString str("");
					view->MoveLayertoGroup(dst_name, src_name, str);
				}
			}
			else if (src_type==3 && dst_type==6)
			{
				//move mesh into a group
				RenderCanvas* view = m_frame->GetView(src_par_name);
				if (view)
				{
					wxString str("");
					view->MoveMeshtoGroup(dst_name, src_name, str);
				}
			}
			else
			{
				RenderCanvas* view = m_frame->GetView(src_par_name);
				if (view)
					view->MoveLayerinView(src_name, dst_name);
			}
		}
		else if (src_par_type == 5 &&
			dst_par_type == 5 &&
			src_par_name == dst_par_name &&
			src_name != dst_name)
		{
			//move volume within the same group
			wxString str = GetItemText(GetItemParent(src_par_item));
			RenderCanvas* view = m_frame->GetView(str);
			if (view)
				view->MoveLayerinGroup(src_par_name, src_name, dst_name);
		}
		else if (src_par_type == 5 && //par is group
			src_type == 2 && //src is volume
			dst_par_type == 1 && //dst's par is view
			dst_par_name == GetItemText(GetItemParent(src_par_item))) //in same view
		{
			//move volume outside of the group
			if (dst_type == 5) //dst is group
			{
				RenderCanvas* view = m_frame->GetView(dst_par_name);
				if (view)
				{
					wxString str("");
					view->MoveLayerfromtoGroup(src_par_name, dst_name, src_name, str);
				}
			}
			else
			{
				RenderCanvas *view = m_frame->GetView(dst_par_name);
				if (view)
					view->MoveLayertoView(src_par_name, src_name, dst_name);
			}
		}
		else if (src_par_type == 1 && //src's par is view
			src_type == 2 && //src is volume
			dst_par_type == 5 && //dst's par is group
			src_par_name == GetItemText(GetItemParent(dst_par_item))) //in the same view
		{
			//move volume into group
			RenderCanvas* view = m_frame->GetView(src_par_name);
			if (view)
				view->MoveLayertoGroup(dst_par_name, src_name, dst_name);
		}
		else if (src_par_type == 5 && //src's par is group
			src_type == 2 && // src is volume
			dst_par_type == 5 && //dst's par is group
			dst_type == 2 && //dst is volume
			GetItemText(GetItemParent(src_par_item)) == GetItemText(GetItemParent(dst_par_item)) && // in the same view
			GetItemText(src_par_item) != GetItemText(dst_par_item))// par groups are different
		{
			//move volume from one group to another
			wxString str = GetItemText(GetItemParent(src_par_item));
			RenderCanvas* view = m_frame->GetView(str);
			if (view)
				view->MoveLayerfromtoGroup(src_par_name, dst_par_name, src_name, dst_name);
		}
		else if (src_type == 2 && //src is volume
			src_par_type == 5 && //src's par is group
			dst_type == 1 && //dst is view
			GetItemText(GetItemParent(src_par_item)) == dst_name) //in the same view
		{
			//move volume outside of the group
			RenderCanvas* view = m_frame->GetView(dst_name);
			if (view)
			{
				wxString str("");
				view->MoveLayertoView(src_par_name, src_name, str);
			}
		}
		else if (src_par_type == 6 &&
			dst_par_type == 6 &&
			src_par_name == dst_par_name &&
			src_name != dst_name)
		{
			//move mesh within the same group
			wxString str = GetItemText(GetItemParent(src_par_item));
			RenderCanvas* view = m_frame->GetView(str);
			if (view)
				view->MoveMeshinGroup(src_par_name, src_name, dst_name);
		}
		else if (src_par_type == 6 && //par is group
			src_type == 3 && //src is mesh
			dst_par_type == 1 && //dst's par is view
			dst_par_name == GetItemText(GetItemParent(src_par_item))) //in same view
		{
			//move mesh outside of the group
			if (dst_type == 6) //dst is group
			{
				RenderCanvas* view = m_frame->GetView(dst_par_name);
				if (view)
				{
					wxString str("");
					view->MoveMeshfromtoGroup(src_par_name, dst_name, src_name, str);
				}
			}
			else
			{
				RenderCanvas *view = m_frame->GetView(dst_par_name);
				if (view)
					view->MoveMeshtoView(src_par_name, src_name, dst_name);
			}
		}
		else if (src_par_type == 1 && //src's par is view
			src_type == 3 && //src is mesh
			dst_par_type == 6 && //dst's par is group
			src_par_name == GetItemText(GetItemParent(dst_par_item))) //in the same view
		{
			//move mesh into group
			RenderCanvas* view = m_frame->GetView(src_par_name);
			if (view)
				view->MoveMeshtoGroup(dst_par_name, src_name, dst_name);
		}
		else if (src_par_type == 6 && //src's par is group
			src_type == 3 && // src is mesh
			dst_par_type == 6 && //dst's par is group
			dst_type == 3 && //dst is mesh
			GetItemText(GetItemParent(src_par_item)) == GetItemText(GetItemParent(dst_par_item)) && // in the same view
			GetItemText(src_par_item) != GetItemText(dst_par_item))// par groups are different
		{
			//move mesh from one group to another
			wxString str = GetItemText(GetItemParent(src_par_item));
			RenderCanvas* view = m_frame->GetView(str);
			if (view)
				view->MoveMeshfromtoGroup(src_par_name, dst_par_name, src_name, dst_name);
		}
		else if (src_type == 3 && //src is mesh
			src_par_type == 6 && //src's par is group
			dst_type == 1 && //dst is view
			GetItemText(GetItemParent(src_par_item)) == dst_name) //in the same view
		{
			//move mesh outside of the group
			RenderCanvas* view = m_frame->GetView(dst_name);
			if (view)
			{
				wxString str("");
				view->MoveMeshtoView(src_par_name, src_name, str);
			}
		}

		glbin.set_tree_selection(src_name.ToStdString());
		refresh = true;
	}
	else if (src_item.IsOk() && src_par_item.IsOk() &&
		!dst_item.IsOk() && m_frame)
	{
		//move volume out of the group
		int src_type = ((LayerInfo*)GetItemData(src_item))->type;
		int src_par_type = ((LayerInfo*)GetItemData(src_par_item))->type;

		wxString src_name = GetItemText(src_item);
		wxString src_par_name = GetItemText(src_par_item);

		if (src_type == 2 && src_par_type == 5)
		{
			wxString str = GetItemText(GetItemParent(src_par_item));
			RenderCanvas* view = m_frame->GetView(str);
			if (view)
			{
				wxString str("");
				view->MoveLayertoView(src_par_name, src_name, str);

				glbin.set_tree_selection(src_name.ToStdString());
				refresh = true;
			}
		}
	}

	if (refresh)
		m_frame->GetTree()->FluoRefresh(false, 2, { gstTreeCtrl });

	SetScrollPos(wxVERTICAL, m_scroll_pos);
}

void DataTreeCtrl::OnKeyDown(wxKeyEvent& event)
{
	if ( event.GetKeyCode() == WXK_DELETE ||
		event.GetKeyCode() == WXK_BACK)
		DeleteSelection();
	//event.Skip();
}

void DataTreeCtrl::OnKeyUp(wxKeyEvent& event)
{
	event.Skip();
}

//icons
void DataTreeCtrl::AppendIcon()
{
	wxImageList *images = GetImageList();
	if (!images)
		return;

	wxIcon icon0 = wxIcon(cross_xpm);
	wxIcon icon1 = wxIcon(tick_xpm);
	images->Add(icon0);
	images->Add(icon1);
}

void DataTreeCtrl::ClearIcons()
{
	wxImageList *images = GetImageList();
	if (!images)
		return;

	images->RemoveAll();
}

int DataTreeCtrl::GetIconNum()
{
	wxImageList* images = GetImageList();
	if (images)
		return images->GetImageCount()/2;
	else
		return 0;
}

void DataTreeCtrl::ChangeIconColor(int i, wxColor c)
{
	ChangeIconColor(i*2, c, 0);
	ChangeIconColor(i*2+1, c, 1);
}

void DataTreeCtrl::ChangeIconColor(int which, wxColor c, int type)
{
	int i;
	int icon_lines = 0;
	int icon_height = 0;

	wxImageList *images = GetImageList();
	if (!images)
		return;

	const char **orgn_data = type?tick_xpm:cross_xpm;
	char cc[8];

	int dummy;
	SSCANF(orgn_data[0], "%d %d %d %d",
		&dummy, &icon_height, &icon_lines, &dummy);
	icon_lines += icon_height+1;
	char **data = new char*[icon_lines];

	sprintf(cc, "#%02X%02X%02X", c.Red(), c.Green(), c.Blue());

	for (i=0; i<icon_lines; i++)
	{
		if (i==icon_change)
		{
			int len = strlen(orgn_data[i]);
			int len_key = strlen(icon_key);
			int len_chng = len+strlen(cc)-len_key+1;
			data[i] = new char[len_chng]();
			char *temp = new char[len_key+1]();
			int index = 0;
			for (int j=0; j<len; j++)
			{
				char val = orgn_data[i][j];
				if (j>=len_key-1)
				{
					for (int k=0; k<len_key; k++)
						temp[k] = orgn_data[i][j-(len_key-k-1)];
					if (!strcmp(temp, icon_key))
					{
						strcpy(data[i]+index-len_key+1, cc);
						index = index-len_key+1+strlen(cc);
						continue;
					}
				}
				data[i][index++] = val;
			}
			delete[] temp;
		}
		else
		{
			int len = strlen(orgn_data[i]);
			data[i] = new char[len+1];
			memcpy(data[i], orgn_data[i], len+1);
		}
	}
	wxIcon icon = wxIcon(data);
	images->Replace(which, icon);
	for (i=0; i<icon_lines; i++)
	{
		delete[] data[i];
	}
	delete[] data;
}

//item operations
//root item
wxTreeItemId DataTreeCtrl::AddRootItem(const wxString &text)
{
	wxTreeItemId item = AddRoot(text);
	LayerInfo* item_data = new LayerInfo;
	item_data->type = 0;//root;
	SetItemData(item, item_data);
	return item;
}

void DataTreeCtrl::ExpandRootItem()
{
	Expand(GetRootItem());
}

//view item
wxTreeItemId DataTreeCtrl::AddViewItem(const wxString &text)
{
	wxTreeItemId item = AppendItem(GetRootItem(),text, 1);
	LayerInfo* item_data = new LayerInfo;
	item_data->type = 1;//view
	SetItemData(item, item_data);
	return item;
}

void DataTreeCtrl::SetViewItemImage(const wxTreeItemId& item, int image)
{
	SetItemImage(item , image);
}

//volume data item
wxTreeItemId DataTreeCtrl::AddVolItem(wxTreeItemId par_item, const wxString &text)
{
	wxTreeItemId item = AppendItem(par_item, text, 1);
	LayerInfo* item_data = new LayerInfo;
	item_data->type = 2;//volume data
	SetItemData(item, item_data);
	return item;
}

void DataTreeCtrl::SetVolItemImage(const wxTreeItemId item, int image)
{
	SetItemImage(item, image);
}

//mesh data item
wxTreeItemId DataTreeCtrl::AddMeshItem(wxTreeItemId par_item, const wxString &text)
{
	wxTreeItemId item = AppendItem(par_item, text, 1);
	LayerInfo* item_data = new LayerInfo;
	item_data->type = 3;//mesh data
	SetItemData(item, item_data);
	return item;
}

void DataTreeCtrl::SetMeshItemImage(const wxTreeItemId item, int image)
{
	SetItemImage(item, image);
}

//annotation item
wxTreeItemId DataTreeCtrl::AddAnnotationItem(wxTreeItemId par_item, const wxString &text)
{
	wxTreeItemId item = AppendItem(par_item, text, 1);
	LayerInfo* item_data = new LayerInfo;
	item_data->type = 4;//annotations
	SetItemData(item, item_data);
	return item;
}

void DataTreeCtrl::SetAnnotationItemImage(const wxTreeItemId item, int image)
{
	SetItemImage(item, image);
}

//group item
wxTreeItemId DataTreeCtrl::AddGroupItem(wxTreeItemId par_item, const wxString &text)
{
	wxTreeItemId item = AppendItem(par_item, text, 1);
	LayerInfo* item_data = new LayerInfo;
	item_data->type = 5;//group
	SetItemData(item, item_data);
	return item;
}

void DataTreeCtrl::SetGroupItemImage(const wxTreeItemId item, int image)
{
	SetItemImage(item, image);
}

//mesh group item
wxTreeItemId DataTreeCtrl::AddMGroupItem(wxTreeItemId par_item, const wxString &text)
{
	wxTreeItemId item = AppendItem(par_item, text, 1);
	LayerInfo* item_data = new LayerInfo;
	item_data->type = 6;//mesh group
	SetItemData(item, item_data);
	return item;
}

void DataTreeCtrl::SetMGroupItemImage(const wxTreeItemId item, int image)
{
	SetItemImage(item, image);
}

//brush commands (from the panel)
void DataTreeCtrl::BrushClear()
{
	wxTreeItemId sel_item = GetSelection();

	if (sel_item.IsOk() && m_frame)
	{
		//select data
		wxString name = GetItemText(sel_item);
		LayerInfo* item_data = (LayerInfo*)GetItemData(sel_item);
		if (item_data && item_data->type==2)
		{
			wxTreeItemId par_item = GetItemParent(sel_item);
			if (par_item.IsOk())
			{
				LayerInfo* par_item_data = (LayerInfo*)GetItemData(par_item);
				if (par_item_data && par_item_data->type == 5)
				{
					//par is group
					wxString str = GetItemText(GetItemParent(par_item));
					RenderCanvas* view = m_frame->GetView(str);
					if (view)
					{
						VolumeData* vd = glbin_data_manager.GetVolumeData(name);
						if (vd)
						{
							int int_mode = view->GetIntMode();
							int paint_mode = view->GetPaintMode();
							view->SetVolumeA(vd);
							view->SetPaintMode(6);
							view->Segment(true);
							view->RefreshGL(39);
							view->SetPaintMode(paint_mode);
							view->SetIntMode(int_mode);
						}
					}
				}
				else if (par_item_data && par_item_data->type == 1)
				{
					//par is view
					wxString str = GetItemText(par_item);
					RenderCanvas* view = m_frame->GetView(str);
					if (view)
					{
						VolumeData* vd = glbin_data_manager.GetVolumeData(name);
						if (vd)
						{
							int int_mode = view->GetIntMode();
							int paint_mode = view->GetPaintMode();
							view->SetVolumeA(vd);
							view->SetPaintMode(6);
							view->Segment(true);
							view->RefreshGL(39);
							view->SetPaintMode(paint_mode);
							view->SetIntMode(int_mode);
						}
					}
				}
			}
		}
	}
}

void DataTreeCtrl::BrushCreate()
{
	wxTreeItemId sel_item = GetSelection();

	if (sel_item.IsOk() && m_frame)
	{
		//select data
		wxString name = GetItemText(sel_item);
		LayerInfo* item_data = (LayerInfo*)GetItemData(sel_item);
		if (item_data && item_data->type==2)
		{
			wxTreeItemId par_item = GetItemParent(sel_item);
			if (par_item.IsOk())
			{
				LayerInfo* par_item_data = (LayerInfo*)GetItemData(par_item);
				if (par_item_data && par_item_data->type == 5)
				{
					//par is group
					wxString group_name = GetItemText(par_item);
					wxString str = GetItemText(GetItemParent(par_item));
					RenderCanvas* vrv = m_frame->GetView(str);
					if (vrv)
					{
						VolumeData* vd = glbin_data_manager.GetVolumeData(name);
						if (vd)
						{
							glbin_vol_calculator.SetVolumeA(vd);
							glbin_vol_calculator.CalculateGroup(5, group_name);
						}
					}
				}
				else if (par_item_data && par_item_data->type == 1)
				{
					//par is view
					wxString str = GetItemText(par_item);
					RenderCanvas* view = m_frame->GetView(str);
					if (view)
					{
						VolumeData* vd = glbin_data_manager.GetVolumeData(name);
						if (vd)
						{
							glbin_vol_calculator.SetVolumeA(vd);
							glbin_vol_calculator.CalculateGroup(5);
						}
					}
				}
			}
		}
	}
}

void DataTreeCtrl::BrushCreateInv()
{
	wxTreeItemId sel_item = GetSelection();

	if (sel_item.IsOk() && m_frame)
	{
		//select data
		wxString name = GetItemText(sel_item);
		int cal_type = 6;
		if (name.Find("_DELETED")==wxNOT_FOUND)
			cal_type = 6;
		else
			cal_type = 7;

		LayerInfo* item_data = (LayerInfo*)GetItemData(sel_item);
		if (item_data && item_data->type==2)
		{
			wxTreeItemId par_item = GetItemParent(sel_item);
			if (par_item.IsOk())
			{
				LayerInfo* par_item_data = (LayerInfo*)GetItemData(par_item);
				if (par_item_data && par_item_data->type == 5)
				{
					//par is group
					wxString group_name = GetItemText(par_item);
					wxString str = GetItemText(GetItemParent(par_item));
					RenderCanvas* view = m_frame->GetView(str);
					if (view)
					{
						VolumeData* vd = glbin_data_manager.GetVolumeData(name);
						if (vd)
						{
							glbin_vol_calculator.SetVolumeA(vd);
							glbin_vol_calculator.CalculateGroup(cal_type, group_name);
						}
					}
				}
				else if (par_item_data && par_item_data->type == 1)
				{
					//par is view
					wxString str = GetItemText(par_item);
					RenderCanvas* view = m_frame->GetView(str);
					if (view)
					{
						VolumeData* vd = glbin_data_manager.GetVolumeData(name);
						if (vd)
						{
							glbin_vol_calculator.SetVolumeA(vd);
							glbin_vol_calculator.CalculateGroup(cal_type);
						}
					}
				}
			}
		}
	}
}

//mask operations
void DataTreeCtrl::CopyMask(bool copy_data)
{
	wxTreeItemId sel_item = GetSelection();
	if (!sel_item.IsOk()) return;
	if (!m_frame) return;

	wxString name = GetItemText(sel_item);
	VolumeData* vd = glbin_data_manager.GetVolumeData(name);
	if (vd)
	{
		m_frame->m_vd_copy = vd;
		m_frame->m_copy_data = copy_data;
	}
}

void DataTreeCtrl::PasteMask(int op)
{
	if (m_fixed)
		return;
	wxTreeItemId sel_item = GetSelection();
	if (!sel_item.IsOk()) return;
	if (!m_frame) return;

	wxString name = GetItemText(sel_item);
	VolumeData* vd = glbin_data_manager.GetVolumeData(name);
	RenderCanvas* view = 0;
	DataGroup* group = 0;
	wxTreeItemId par_item = GetItemParent(sel_item);
	if (par_item.IsOk())
	{
		LayerInfo* par_item_data = (LayerInfo*)GetItemData(par_item);
		if (par_item_data && par_item_data->type == 5)
		{
			//par is group
			wxString str = GetItemText(GetItemParent(par_item));
			view = m_frame->GetView(str);
			str = GetItemText(par_item);
			group = view->GetGroup(str);
		}
		else if (par_item_data && par_item_data->type == 1)
		{
			//par is view
			wxString str = GetItemText(par_item);
			view = m_frame->GetView(str);
		}
	}
	bool apply_group = glbin_vol_selector.GetSelectGroup();

	if (vd && m_frame->m_vd_copy)
	{
		//prevent self copying
		if (!m_frame->m_copy_data &&
			vd == m_frame->m_vd_copy)
			return;

		//undo/redo
		if (flvr::Texture::mask_undo_num_ > 0 &&
			vd->GetTexture())
			vd->GetTexture()->push_mask();
		if (m_frame->m_copy_data)
		{
			Nrrd* data = m_frame->m_vd_copy->GetVolume(false);
			if (m_frame->m_vd_copy->GetBits() == 16)
				vd->AddMask16(data, op, m_frame->m_vd_copy->GetScalarScale());
			else
				vd->AddMask(data, op);
		}
		else
			vd->AddMask(m_frame->m_vd_copy->GetMask(false), op);

		if (apply_group)
		{
			Nrrd* data = vd->GetMask(false);
			if (data)
			{
				if (group)
					group->AddMask(data, 0);
				else
				{
					for (int i = 0; i < view->GetGroupNum(); ++i)
						view->GetGroup(i)->AddMask(data, 0);
				}

			}
		}

		m_frame->RefreshCanvases();
		if (m_frame->GetBrushToolDlg())
			m_frame->GetBrushToolDlg()->UpdateUndoRedo();
		if (m_frame->GetColocalizationDlg() &&
			view->m_paint_colocalize)
			m_frame->GetColocalizationDlg()->Colocalize();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(TreePanel, wxPanel)
	EVT_TOOL(ID_ToggleView, TreePanel::OnToggleView)
	EVT_TOOL(ID_AddGroup, TreePanel::OnAddGroup)
	EVT_TOOL(ID_AddMGroup, TreePanel::OnAddMGroup)
	EVT_TOOL(ID_RemoveData, TreePanel::OnRemoveData)
	//brush commands
	EVT_TOOL(ID_BrushAppend, TreePanel::OnBrushAppend)
	EVT_TOOL(ID_BrushDesel, TreePanel::OnBrushDesel)
	EVT_TOOL(ID_BrushDiffuse, TreePanel::OnBrushDiffuse)
	EVT_TOOL(ID_BrushClear, TreePanel::OnBrushClear)
	EVT_TOOL(ID_BrushErase, TreePanel::OnBrushErase)
	EVT_TOOL(ID_BrushCreate, TreePanel::OnBrushCreate)
	END_EVENT_TABLE()

TreePanel::TreePanel(MainFrame* frame,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name) :
	PropPanel(frame, frame, pos, size, style, name)
{
	wxEventBlocker blocker(this);
	SetDoubleBuffered(true);
	//create data tree
	m_datatree = new DataTreeCtrl(frame, this);

	//create tool bar
	m_toolbar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxTB_FLAT|wxTB_TOP|wxTB_NODIVIDER);
	wxBitmap bitmap = wxGetBitmapFromMemory(toggle_disp);
#ifdef _DARWIN
	m_toolbar->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_toolbar->AddTool(ID_ToggleView, "Toggle View", bitmap,
		"Toggle the visibility of current selection");
	bitmap = wxGetBitmapFromMemory(add_group);
	m_toolbar->AddTool(ID_AddGroup, "Add Group", bitmap,
		"Add a volume data group to the selected view");
	bitmap = wxGetBitmapFromMemory(add_mgroup);
	m_toolbar->AddTool(ID_AddMGroup, "Add Mesh Group", bitmap,
		"Add a mesh data group to the selected view");
	bitmap = wxGetBitmapFromMemory(delet);
	m_toolbar->AddTool(ID_RemoveData, "Delete", bitmap,
		"Delete current selection");
	m_toolbar->AddSeparator();
	bitmap = wxGetBitmapFromMemory(brush_append);
	m_toolbar->AddCheckTool(ID_BrushAppend, "Highlight",
		bitmap, wxNullBitmap,
		"Highlight structures by painting on the render view (hold Shift)");
	bitmap = wxGetBitmapFromMemory(brush_diffuse);
	m_toolbar->AddCheckTool(ID_BrushDiffuse, "Diffuse",
		bitmap, wxNullBitmap,
		"Diffuse highlighted structures by painting (hold Z)");
	bitmap = wxGetBitmapFromMemory(brush_desel);
	m_toolbar->AddCheckTool(ID_BrushDesel, "Reset",
		bitmap, wxNullBitmap,
		"Reset highlighted structures by painting (hold X)");
	m_toolbar->AddSeparator();
	bitmap = wxGetBitmapFromMemory(brush_erase);
	m_toolbar->AddTool(ID_BrushErase, "Erase",
		bitmap, "Erase highlighted structures");
	bitmap = wxGetBitmapFromMemory(brush_create);
	m_toolbar->AddTool(ID_BrushCreate, "Extract", bitmap,
		"Extract highlighted structures out and create a new volume");
	m_toolbar->AddSeparator();
	bitmap = wxGetBitmapFromMemory(brush_clear);
	m_toolbar->AddTool(ID_BrushClear, "Reset All",
		bitmap, "Reset all highlighted structures");
	m_toolbar->Realize();

	//organize positions
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);

	sizer_v->Add(m_toolbar, 0, wxEXPAND);
	sizer_v->Add(m_datatree, 1, wxEXPAND);

	SetSizer(sizer_v);
	Layout();
}

TreePanel::~TreePanel()
{
}

DataTreeCtrl* TreePanel::GetTreeCtrl()
{
	return m_datatree;
}

void TreePanel::LoadPerspective()
{

}

void TreePanel::SavePerspective()
{

}

void TreePanel::FluoUpdate(const fluo::ValueCollection& vc)
{
	if (FOUND_VALUE(gstNull))
		return;

	bool update_all = vc.empty();

	//update icons only
	if (update_all || FOUND_VALUE(gstTreeCtrl))
		UpdateTree();
	else
	{
		if (update_all || FOUND_VALUE(gstTreeIcons))
			UpdateTreeIcons();
		if (update_all || FOUND_VALUE(gstTreeColors))
			UpdateTreeColors();
	}

	if (update_all || FOUND_VALUE(gstTreeSelection))
		UpdateSelection();
}

void TreePanel::UpdateTreeIcons()
{
	if (!m_frame)
		return;
	int i, j, k;

	wxTreeItemId root = m_datatree->GetRootItem();
	wxTreeItemIdValue ck_view;
	int counter = 0;
	for (i = 0; i < m_frame->GetViewNum(); i++)
	{
		RenderCanvas* view = m_frame->GetView(i);
		wxTreeItemId vrv_item;
		if (i == 0)
			vrv_item = m_datatree->GetFirstChild(root, ck_view);
		else
			vrv_item = m_datatree->GetNextChild(root, ck_view);

		if (!vrv_item.IsOk())
			continue;

		SetViewItemImage(vrv_item, view->GetDraw());

		wxTreeItemIdValue ck_layer;
		for (j = 0; j < view->GetLayerNum(); j++)
		{
			TreeLayer* layer = view->GetLayer(j);
			wxTreeItemId layer_item;
			if (j == 0)
				layer_item = m_datatree->GetFirstChild(vrv_item, ck_layer);
			else
				layer_item = m_datatree->GetNextChild(vrv_item, ck_layer);

			if (!layer_item.IsOk())
				continue;

			switch (layer->IsA())
			{
			case 2://volume
			{
				VolumeData* vd = (VolumeData*)layer;
				if (!vd)
					break;
				counter++;
				SetVolItemImage(layer_item, vd->GetDisp() ? 2 * counter + 1 : 2 * counter);
			}
			break;
			case 3://mesh
			{
				MeshData* md = (MeshData*)layer;
				if (!md)
					break;
				counter++;
				SetMeshItemImage(layer_item, md->GetDisp() ? 2 * counter + 1 : 2 * counter);
			}
			break;
			case 4://annotations
			{
				Annotations* ann = (Annotations*)layer;
				if (!ann)
					break;
				counter++;
				SetAnnotationItemImage(layer_item, ann->GetDisp() ? 2 * counter + 1 : 2 * counter);
			}
			break;
			case 5://volume group
			{
				DataGroup* group = (DataGroup*)layer;
				if (!group)
					break;
				SetGroupItemImage(layer_item, int(group->GetDisp()));
				wxTreeItemIdValue ck_volume;
				for (k = 0; k < group->GetVolumeNum(); k++)
				{
					VolumeData* vd = group->GetVolumeData(k);
					if (!vd)
						continue;
					wxTreeItemId volume_item;
					if (k == 0)
						volume_item = m_datatree->GetFirstChild(layer_item, ck_volume);
					else
						volume_item = m_datatree->GetNextChild(layer_item, ck_volume);
					if (!volume_item.IsOk())
						continue;
					counter++;
					SetVolItemImage(volume_item, vd->GetDisp() ? 2 * counter + 1 : 2 * counter);
				}
			}
			break;
			case 6://mesh group
			{
				MeshGroup* group = (MeshGroup*)layer;
				if (!group)
					break;
				SetMGroupItemImage(layer_item, int(group->GetDisp()));
				wxTreeItemIdValue ck_mesh;
				for (k = 0; k < group->GetMeshNum(); k++)
				{
					MeshData* md = group->GetMeshData(k);
					if (!md)
						continue;
					wxTreeItemId mesh_item;
					if (k == 0)
						mesh_item = m_datatree->GetFirstChild(layer_item, ck_mesh);
					else
						mesh_item = m_datatree->GetNextChild(layer_item, ck_mesh);
					if (!mesh_item.IsOk())
						continue;
					counter++;
					SetMeshItemImage(mesh_item, md->GetDisp() ? 2 * counter + 1 : 2 * counter);
				}
			}
			break;
			}
		}
	}
	Refresh(false);
}

void TreePanel::UpdateTreeColors()
{
	if (!m_frame)
		return;
	int i, j, k;
	int counter = 0;
	for (i = 0; i < m_frame->GetViewNum(); i++)
	{
		RenderCanvas* view = m_frame->GetView(i);

		for (j = 0; j < view->GetLayerNum(); j++)
		{
			TreeLayer* layer = view->GetLayer(j);
			switch (layer->IsA())
			{
			case 0://root
				break;
			case 1://view
				break;
			case 2://volume
			{
				VolumeData* vd = (VolumeData*)layer;
				if (!vd)
					break;
				fluo::Color c = vd->GetColor();
				wxColor wxc(
					(unsigned char)(c.r() * 255),
					(unsigned char)(c.g() * 255),
					(unsigned char)(c.b() * 255));
				ChangeIconColor(counter + 1, wxc);
				counter++;
			}
			break;
			case 3://mesh
			{
				MeshData* md = (MeshData*)layer;
				if (!md)
					break;
				fluo::Color amb, diff, spec;
				double shine, alpha;
				md->GetMaterial(amb, diff, spec, shine, alpha);
				wxColor wxc(
					(unsigned char)(diff.r() * 255),
					(unsigned char)(diff.g() * 255),
					(unsigned char)(diff.b() * 255));
				ChangeIconColor(counter + 1, wxc);
				counter++;
			}
			break;
			case 4://annotations
			{
				Annotations* ann = (Annotations*)layer;
				if (!ann)
					break;
				wxColor wxc(255, 255, 255);
				ChangeIconColor(counter + 1, wxc);
				counter++;
			}
			break;
			case 5://group
			{
				DataGroup* group = (DataGroup*)layer;
				if (!group)
					break;
				for (k = 0; k < group->GetVolumeNum(); k++)
				{
					VolumeData* vd = group->GetVolumeData(k);
					if (!vd)
						break;
					fluo::Color c = vd->GetColor();
					wxColor wxc(
						(unsigned char)(c.r() * 255),
						(unsigned char)(c.g() * 255),
						(unsigned char)(c.b() * 255));
					ChangeIconColor(counter + 1, wxc);
					counter++;
				}
			}
			break;
			case 6://mesh group
			{
				MeshGroup* group = (MeshGroup*)layer;
				if (!group)
					break;
				for (k = 0; k < group->GetMeshNum(); k++)
				{
					MeshData* md = group->GetMeshData(k);
					if (!md)
						break;
					fluo::Color amb, diff, spec;
					double shine, alpha;
					md->GetMaterial(amb, diff, spec, shine, alpha);
					wxColor wxc(
						(unsigned char)(diff.r() * 255),
						(unsigned char)(diff.g() * 255),
						(unsigned char)(diff.b() * 255));
					ChangeIconColor(counter + 1, wxc);
					counter++;
				}
			}
			break;
			}
		}
	}
	Refresh(false);
}

void TreePanel::UpdateTree()
{
	if (!m_frame)
		return;

	DeleteAll();
	ClearIcons();

	wxString root_str = "Scene Graph";
	wxTreeItemId root_item = AddRootItem(root_str);
	if (glbin_tree_sel == root_str.ToStdString())
		SelectItem(root_item);
	//append non-color icons for views
	AppendIcon();
	Expand(root_item);
	ChangeIconColor(0, wxColor(255, 255, 255));

	wxTreeItemId sel_item;

	for (int i = 0; i < m_frame->GetViewNum(); i++)
	{
		RenderCanvas* view = m_frame->GetView(i);
		if (!view)
			continue;
		int j, k;

		wxString view_name = view->m_vrv->GetName();
		view->OrganizeLayers();
		wxTreeItemId vrv_item = AddViewItem(view_name);
		SetViewItemImage(vrv_item, view->GetDraw());
		if (glbin_tree_sel == view_name.ToStdString())
			SelectItem(vrv_item);

		for (j = 0; j < view->GetLayerNum(); j++)
		{
			TreeLayer* layer = view->GetLayer(j);
			switch (layer->IsA())
			{
			case 0://root
				break;
			case 1://view
				break;
			case 2://volume data
			{
				VolumeData* vd = (VolumeData*)layer;
				if (!vd)
					break;
				//append icon for volume
				AppendIcon();
				fluo::Color c = vd->GetColor();
				wxColor wxc(
					(unsigned char)(c.r() * 255),
					(unsigned char)(c.g() * 255),
					(unsigned char)(c.b() * 255));
				int ii = GetIconNum() - 1;
				ChangeIconColor(ii, wxc);
				wxTreeItemId item = AddVolItem(vrv_item, vd->GetName());
				SetVolItemImage(item, vd->GetDisp() ? 2 * ii + 1 : 2 * ii);
				if (glbin_tree_sel == vd->GetName().ToStdString())
				{
					sel_item = item;
					view->SetVolumeA(vd);
					m_frame->GetBrushToolDlg()->GetSettings(view);
					m_frame->GetMeasureDlg()->GetSettings(view);
					m_frame->GetTraceDlg()->GetSettings(view);
					m_frame->GetOclDlg()->GetSettings(view);
					m_frame->GetComponentDlg()->SetView(view);
					m_frame->GetColocalizationDlg()->SetView(view);
				}
			}
			break;
			case 3://mesh data
			{
				MeshData* md = (MeshData*)layer;
				if (!md)
					break;
				//append icon for mesh
				AppendIcon();
				fluo::Color amb, diff, spec;
				double shine, alpha;
				md->GetMaterial(amb, diff, spec, shine, alpha);
				wxColor wxc(
					(unsigned char)(diff.r() * 255),
					(unsigned char)(diff.g() * 255),
					(unsigned char)(diff.b() * 255));
				int ii = GetIconNum() - 1;
				ChangeIconColor(ii, wxc);
				wxTreeItemId item = AddMeshItem(vrv_item, md->GetName());
				SetMeshItemImage(item, md->GetDisp() ? 2 * ii + 1 : 2 * ii);
				if (glbin_tree_sel == md->GetName().ToStdString())
					sel_item = item;
			}
			break;
			case 4://annotations
			{
				Annotations* ann = (Annotations*)layer;
				if (!ann)
					break;
				//append icon for annotations
				AppendIcon();
				wxColor wxc(255, 255, 255);
				int ii = GetIconNum() - 1;
				ChangeIconColor(ii, wxc);
				wxTreeItemId item = AddAnnotationItem(vrv_item, ann->GetName());
				SetAnnotationItemImage(item, ann->GetDisp() ? 2 * ii + 1 : 2 * ii);
				if (glbin_tree_sel == ann->GetName().ToStdString())
					sel_item = item;
			}
			break;
			case 5://group
			{
				DataGroup* group = (DataGroup*)layer;
				if (!group)
					break;
				//append group item to tree
				wxTreeItemId group_item = AddGroupItem(vrv_item, group->GetName());
				SetGroupItemImage(group_item, int(group->GetDisp()));
				//append volume data to group
				for (k = 0; k < group->GetVolumeNum(); k++)
				{
					VolumeData* vd = group->GetVolumeData(k);
					if (!vd)
						continue;
					//add icon
					AppendIcon();
					fluo::Color c = vd->GetColor();
					wxColor wxc(
						(unsigned char)(c.r() * 255),
						(unsigned char)(c.g() * 255),
						(unsigned char)(c.b() * 255));
					int ii = GetIconNum() - 1;
					ChangeIconColor(ii, wxc);
					wxTreeItemId item = AddVolItem(group_item, vd->GetName());
					SetVolItemImage(item, vd->GetDisp() ? 2 * ii + 1 : 2 * ii);
					if (glbin_tree_sel == vd->GetName().ToStdString())
					{
						sel_item = item;
						view->SetVolumeA(vd);
						m_frame->GetBrushToolDlg()->GetSettings(view);
						m_frame->GetMeasureDlg()->GetSettings(view);
						m_frame->GetTraceDlg()->GetSettings(view);
						m_frame->GetOclDlg()->GetSettings(view);
						m_frame->GetComponentDlg()->SetView(view);
						m_frame->GetColocalizationDlg()->SetView(view);
					}
				}
				if (glbin_tree_sel == group->GetName().ToStdString())
					sel_item = group_item;
			}
			break;
			case 6://mesh group
			{
				MeshGroup* group = (MeshGroup*)layer;
				if (!group)
					break;
				//append group item to tree
				wxTreeItemId group_item = AddMGroupItem(vrv_item, group->GetName());
				SetMGroupItemImage(group_item, int(group->GetDisp()));
				//append mesh data to group
				for (k = 0; k < group->GetMeshNum(); k++)
				{
					MeshData* md = group->GetMeshData(k);
					if (!md)
						continue;
					//add icon
					AppendIcon();
					fluo::Color amb, diff, spec;
					double shine, alpha;
					md->GetMaterial(amb, diff, spec, shine, alpha);
					wxColor wxc(
						(unsigned char)(diff.r() * 255),
						(unsigned char)(diff.g() * 255),
						(unsigned char)(diff.b() * 255));
					int ii = GetIconNum() - 1;
					ChangeIconColor(ii, wxc);
					wxTreeItemId item = AddMeshItem(group_item, md->GetName());
					SetMeshItemImage(item, md->GetDisp() ? 2 * ii + 1 : 2 * ii);
					if (glbin_tree_sel == md->GetName().ToStdString())
						sel_item = item;
				}
				if (glbin_tree_sel == group->GetName().ToStdString())
					sel_item = group_item;
			}
			break;
			}
		}
	}

	if (sel_item.IsOk())
		SelectItem(sel_item);
	ExpandAll();
}

void TreePanel::ChangeIconColor(int i, wxColor c)
{
	if (m_datatree)
		m_datatree->ChangeIconColor(i, c);
}

void TreePanel::AppendIcon()
{
	if (m_datatree)
		m_datatree->AppendIcon();
}

void TreePanel::ClearIcons()
{
	if (m_datatree)
		m_datatree->ClearIcons();
}

int TreePanel::GetIconNum()
{
	int num = 0;
	if (m_datatree)
		num = m_datatree->GetIconNum();
	return num;
}

void TreePanel::SelectItem(wxTreeItemId item)
{
	if (m_datatree)
		m_datatree->SelectItem(item);
}

void TreePanel::Expand(wxTreeItemId item)
{
	if (m_datatree)
		m_datatree->Expand(item);
}

void TreePanel::ExpandAll()
{
	if (m_datatree)
	{
		m_datatree->ExpandAll();
		m_datatree->SetScrollPos(wxVERTICAL, 0);
	}
}

void TreePanel::DeleteAll()
{
	if (m_datatree)
	{
		m_datatree->DeleteAll();
		if (m_frame)
			m_frame->m_vd_copy = 0;
	}
}

void TreePanel::TraversalDelete(wxTreeItemId item)
{
	if (m_datatree)
		m_datatree->TraversalDelete(item);
}

wxTreeItemId TreePanel::AddRootItem(const wxString &text)
{
	wxTreeItemId id;
	if (m_datatree)
		id = m_datatree->AddRootItem(text);
	return id;
}

void TreePanel::ExpandRootItem()
{
	if (m_datatree)
		m_datatree->ExpandRootItem();
}

wxTreeItemId TreePanel::AddViewItem(const wxString &text)
{
	wxTreeItemId id;
	if (m_datatree)
		id = m_datatree->AddViewItem(text);
	return id;
}

void TreePanel::SetViewItemImage(const wxTreeItemId& item, int image)
{
	if (m_datatree)
		m_datatree->SetViewItemImage(item, image);
}

wxTreeItemId TreePanel::AddVolItem(wxTreeItemId par_item, const wxString &text)
{
	wxTreeItemId id;
	if (m_datatree)
		id = m_datatree->AddVolItem(par_item, text);
	return id;
}

void TreePanel::SetVolItemImage(const wxTreeItemId item, int image)
{
	if (m_datatree)
		m_datatree->SetVolItemImage(item, image);
}

wxTreeItemId TreePanel::AddMeshItem(wxTreeItemId par_item, const wxString &text)
{
	wxTreeItemId id;
	if (m_datatree)
		id = m_datatree->AddMeshItem(par_item, text);
	return id;
}

void TreePanel::SetMeshItemImage(const wxTreeItemId item, int image)
{
	if (m_datatree)
		m_datatree->SetMeshItemImage(item, image);
}

wxTreeItemId TreePanel::AddAnnotationItem(wxTreeItemId par_item, const wxString &text)
{
	wxTreeItemId id;
	if (m_datatree)
		id = m_datatree->AddAnnotationItem(par_item, text);
	return id;
}

void TreePanel::SetAnnotationItemImage(const wxTreeItemId item, int image)
{
	if (m_datatree)
		m_datatree->SetAnnotationItemImage(item, image);
}

wxTreeItemId TreePanel::AddGroupItem(wxTreeItemId par_item, const wxString &text)
{
	wxTreeItemId id;
	if (m_datatree)
		id = m_datatree->AddGroupItem(par_item, text);
	return id;
}

void TreePanel::SetGroupItemImage(const wxTreeItemId item, int image)
{
	if (m_datatree)
		m_datatree->SetGroupItemImage(item, image);
}

wxTreeItemId TreePanel::AddMGroupItem(wxTreeItemId par_item, const wxString &text)
{
	wxTreeItemId id;
	if (m_datatree)
		id = m_datatree->AddMGroupItem(par_item, text);
	return id;
}

void TreePanel::SetMGroupItemImage(const wxTreeItemId item, int image)
{
	if (m_datatree)
		m_datatree->SetMGroupItemImage(item, image);
}

void TreePanel::UpdateSelection()
{
	if (m_datatree)
		m_datatree->UpdateSelection();
}

wxString TreePanel::GetCurrentSel()
{
	wxString str = "";
	if (m_datatree)
		str = m_datatree->GetCurrentSel();
	return str;
}

void TreePanel::Select(wxString view, wxString name)
{
	if (m_datatree)
		m_datatree->Select(view, name);
}

void TreePanel::SelectBrush(int id)
{
	m_toolbar->ToggleTool(ID_BrushAppend, false);
	m_toolbar->ToggleTool(ID_BrushDiffuse, false);
	m_toolbar->ToggleTool(ID_BrushDesel, false);
	m_datatree->m_fixed = false;

	if (id)
	{
		m_toolbar->ToggleTool(id, true);
		m_datatree->m_fixed = true;
	}
}

int TreePanel::GetBrushSelected()
{
	if (m_toolbar->GetToolState(ID_BrushAppend))
		return ID_BrushAppend;
	else if (m_toolbar->GetToolState(ID_BrushDiffuse))
		return ID_BrushDiffuse;
	else if (m_toolbar->GetToolState(ID_BrushDesel))
		return ID_BrushDesel;
	else
		return 0;
}

void TreePanel::OnToggleView(wxCommandEvent &event)
{
	if (m_datatree)
	{
		wxTreeEvent tree_event;
		m_datatree->OnAct(tree_event);
	}
}

void TreePanel::OnAddGroup(wxCommandEvent &event)
{
	if (m_datatree)
		m_datatree->OnAddDataGroup(event);
}

void TreePanel::OnAddMGroup(wxCommandEvent &event)
{
	if (m_datatree)
		m_datatree->OnAddMeshGroup(event);
}

void TreePanel::OnRemoveData(wxCommandEvent &event)
{
	if (m_datatree)
		m_datatree->DeleteSelection();
}

void TreePanel::OnBrushAppend(wxCommandEvent &event)
{
	BrushAppend();
	if (m_frame && m_frame->GetBrushToolDlg())
	{
		if (m_toolbar->GetToolState(ID_BrushAppend))
			m_frame->GetBrushToolDlg()->SelectBrush(BrushToolDlg::ID_BrushAppend);
		else
			m_frame->GetBrushToolDlg()->SelectBrush(0);
	}
}

void TreePanel::OnBrushDiffuse(wxCommandEvent &event)
{
	BrushDiffuse();
	if (m_frame && m_frame->GetBrushToolDlg())
	{
		if (m_toolbar->GetToolState(ID_BrushDiffuse))
			m_frame->GetBrushToolDlg()->SelectBrush(BrushToolDlg::ID_BrushDiffuse);
		else
			m_frame->GetBrushToolDlg()->SelectBrush(0);
	}
}

void TreePanel::OnBrushDesel(wxCommandEvent &event)
{
	BrushDesel();
	if (m_frame && m_frame->GetBrushToolDlg())
	{
		if (m_toolbar->GetToolState(ID_BrushDesel))
			m_frame->GetBrushToolDlg()->SelectBrush(BrushToolDlg::ID_BrushDesel);
		else
			m_frame->GetBrushToolDlg()->SelectBrush(0);
	}
}

void TreePanel::OnBrushClear(wxCommandEvent &event)
{
	BrushClear();
}

void TreePanel::OnBrushErase(wxCommandEvent &event)
{
	BrushErase();
	if (m_frame && m_frame->GetBrushToolDlg())
		m_frame->GetBrushToolDlg()->SelectBrush(0);
}

void TreePanel::OnBrushCreate(wxCommandEvent &event)
{
	BrushCreate();
	if (m_frame && m_frame->GetBrushToolDlg())
		m_frame->GetBrushToolDlg()->SelectBrush(0);
}

//control from outside
void TreePanel::BrushAppend()
{
	m_toolbar->ToggleTool(ID_BrushDiffuse, false);
	m_toolbar->ToggleTool(ID_BrushDesel, false);
	if (!m_frame) return;

	if (m_toolbar->GetToolState(ID_BrushAppend))
	{
		for (int i=0; i< m_frame->GetViewNum(); i++)
		{
			RenderCanvas* view = m_frame->GetView(i);
			if (view)
			{
				view->SetIntMode(2);
				view->SetPaintMode(2);
				m_datatree->m_fixed = true;
			}
		}
	}
	else
	{
		for (int i=0; i< m_frame->GetViewNum(); i++)
		{
			RenderCanvas* view = m_frame->GetView(i);
			if (view)
			{
				view->SetIntMode(1);
				m_datatree->m_fixed = false;
			}
		}
	}
}

void TreePanel::BrushDiffuse()
{
	m_toolbar->ToggleTool(ID_BrushAppend, false);
	m_toolbar->ToggleTool(ID_BrushDesel, false);
	if (!m_frame) return;

	if (m_toolbar->GetToolState(ID_BrushDiffuse))
	{
		for (int i=0; i< m_frame->GetViewNum(); i++)
		{
			RenderCanvas* view = m_frame->GetView(i);
			if (view)
			{
				view->SetIntMode(2);
				view->SetPaintMode(4);
				m_datatree->m_fixed = true;
			}
		}
	}
	else
	{
		for (int i=0; i< m_frame->GetViewNum(); i++)
		{
			RenderCanvas* view = m_frame->GetView(i);
			if (view)
			{
				view->SetIntMode(1);
				m_datatree->m_fixed = false;
			}
		}
	}
}

void TreePanel::BrushSolid(bool state)
{
	m_toolbar->ToggleTool(ID_BrushAppend, false);
	m_toolbar->ToggleTool(ID_BrushDiffuse, false);
	m_toolbar->ToggleTool(ID_BrushDesel, false);
	if (!m_frame) return;

	if (state)
	{
		for (int i=0; i< m_frame->GetViewNum(); i++)
		{
			RenderCanvas* view = m_frame->GetView(i);
			if (view)
			{
				view->SetIntMode(2);
				view->SetPaintMode(8);
				m_datatree->m_fixed = true;
			}
		}
	}
	else
	{
		for (int i=0; i< m_frame->GetViewNum(); i++)
		{
			RenderCanvas* view = m_frame->GetView(i);
			if (view)
			{
				view->SetIntMode(1);
				m_datatree->m_fixed = false;
			}
		}
	}
}

void TreePanel::BrushGrow(bool state)
{
	m_toolbar->ToggleTool(ID_BrushAppend, false);
	m_toolbar->ToggleTool(ID_BrushDiffuse, false);
	m_toolbar->ToggleTool(ID_BrushDesel, false);
	if (!m_frame) return;

	if (state)
	{
		for (int i = 0; i < m_frame->GetViewNum(); i++)
		{
			RenderCanvas* view = m_frame->GetView(i);
			if (view)
			{
				view->SetIntMode(10);
				m_datatree->m_fixed = true;
			}
		}
	}
	else
	{
		for (int i = 0; i < m_frame->GetViewNum(); i++)
		{
			RenderCanvas* view = m_frame->GetView(i);
			if (view)
			{
				view->SetIntMode(1);
				m_datatree->m_fixed = false;
			}
		}
	}
}

void TreePanel::BrushDesel()
{
	m_toolbar->ToggleTool(ID_BrushAppend, false);
	m_toolbar->ToggleTool(ID_BrushDiffuse, false);
	if (!m_frame) return;

	if (m_toolbar->GetToolState(ID_BrushDesel))
	{
		for (int i=0; i< m_frame->GetViewNum(); i++)
		{
			RenderCanvas* view = m_frame->GetView(i);
			if (view)
			{
				view->SetIntMode(2);
				view->SetPaintMode(3);
				m_datatree->m_fixed = true;
			}
		}
	}
	else
	{
		for (int i=0; i< m_frame->GetViewNum(); i++)
		{
			RenderCanvas* view = m_frame->GetView(i);
			if (view)
			{
				view->SetIntMode(1);
				m_datatree->m_fixed = false;
			}
		}
	}
}

void TreePanel::BrushClear()
{
	if (m_datatree)
		m_datatree->BrushClear();
}

void TreePanel::BrushErase()
{
	m_toolbar->ToggleTool(ID_BrushAppend, false);
	m_toolbar->ToggleTool(ID_BrushDiffuse, false);
	m_toolbar->ToggleTool(ID_BrushDesel, false);

	if (m_frame)
	{
		for (int i=0; i< m_frame->GetViewNum(); i++)
		{
			RenderCanvas* view = m_frame->GetView(i);
			if (view)
			{
				view->SetIntMode(1);
				m_datatree->m_fixed = false;
			}
		}
	}

	if (m_datatree)
		m_datatree->BrushCreateInv();
}

void TreePanel::BrushCreate()
{
	m_toolbar->ToggleTool(ID_BrushAppend, false);
	m_toolbar->ToggleTool(ID_BrushDiffuse, false);
	m_toolbar->ToggleTool(ID_BrushDesel, false);

	if (m_frame)
	{
		for (int i=0; i< m_frame->GetViewNum(); i++)
		{
			RenderCanvas* view = m_frame->GetView(i);
			if (view)
			{
				view->SetIntMode(1);
				m_datatree->m_fixed = false;
			}
		}
	}

	if (m_datatree)
		m_datatree->BrushCreate();
}
