#include "TreePanel.h"
#include "VRenderFrame.h"
#include "tick.xpm"
#include "cross.xpm"
#include "Formats/png_resource.h"

//resources
#include "img/listicon_addgroup.h"
#include "img/listicon_addmgroup.h"
#include "img/listicon_brushappend.h"
#include "img/listicon_brushclear.h"
#include "img/listicon_brushcreate.h"
#include "img/listicon_brushdiffuse.h"
#include "img/listicon_brushdesel.h"
#include "img/listicon_brusherase.h"
#include "img/listicon_delete.h"
#include "img/listicon_toggle.h"

BEGIN_EVENT_TABLE(DataTreeCtrl, wxTreeCtrl)
EVT_CONTEXT_MENU(DataTreeCtrl::OnContextMenu)
EVT_MENU(ID_ToggleDisp, DataTreeCtrl::OnToggleDisp)
EVT_MENU(ID_Isolate, DataTreeCtrl::OnIsolate)
EVT_MENU(ID_RemoveData, DataTreeCtrl::OnRemoveData)
EVT_MENU(ID_CloseView, DataTreeCtrl::OnCloseView)
EVT_MENU(ID_ManipulateData, DataTreeCtrl::OnManipulateData)
EVT_MENU(ID_AddDataGroup, DataTreeCtrl::OnAddDataGroup)
EVT_MENU(ID_AddMeshGroup, DataTreeCtrl::OnAddMeshGroup)
EVT_MENU(ID_Expand, DataTreeCtrl::OnExpand)
EVT_MENU(ID_Edit, DataTreeCtrl::OnEdit)
EVT_MENU(ID_Measurement, DataTreeCtrl::OnMeasurement)
EVT_MENU(ID_NoiseCancelling, DataTreeCtrl::OnNoiseCancelling)
EVT_MENU(ID_Counting, DataTreeCtrl::OnCounting)
EVT_MENU(ID_Colocalization, DataTreeCtrl::OnColocalization)
EVT_MENU(ID_RandomizeColor, DataTreeCtrl::OnRandomizeColor)
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
      wxWindow* frame,
      wxWindow* parent,
      wxWindowID id,
      const wxPoint& pos,
      const wxSize& size,
      long style) :
   wxTreeCtrl(parent, id, pos, size, style),
   m_frame(frame),
   m_fixed(false),
   m_scroll_pos(-1)
{
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
   VRenderFrame* vr_frame = (VRenderFrame*)m_frame;

   if (sel_item.IsOk() && vr_frame)
   {
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
               VRenderView* vrv = vr_frame->GetView(par_name);
               if (!vrv)
                  break;
               LayerInfo* item_data = (LayerInfo*)GetItemData(sel_item);
               if (item_data)
               {
                  if (item_data->type == 2)//volume data
                  {
                     VolumeData* vd = vrv->GetVolumeData(name_data);
                     if (vd)
                     {
                        vd->SetDisp(true);
                        vrv->RemoveVolumeData(name_data);
                        if (vrv->GetVolMethod() == VOL_METHOD_MULTI)
                        {
                           AdjustView* adjust_view = vr_frame->GetAdjustView();
                           if (adjust_view)
                           {
                              adjust_view->SetRenderView(vrv);
                              adjust_view->UpdateSync();
                           }
                        }
                     }
                  }
                  else if (item_data->type == 3)//mesh data
                  {
                     MeshData* md = vrv->GetMeshData(name_data);
                     if (md)
                     {
                        md->SetDisp(true);
                        vrv->RemoveMeshData(name_data);
                     }
                  }
                  else if (item_data->type == 4)//annotations
                  {
                     Annotations* ann = vrv->GetAnnotations(name_data);
                     if (ann)
                     {
                        ann->SetDisp(true);
                        vrv->RemoveAnnotations(name_data);
                     }
                  }
                  else if (item_data->type == 5)//group
                  {
                     vrv->RemoveGroup(name_data);
                  }
                  else if (item_data->type == 6)//mesh group
                  {
                     vrv->RemoveGroup(name_data);
                  }
               }
               vr_frame->UpdateTree();
               vr_frame->RefreshVRenderViews();
               vr_frame->OnSelection(1);
            }
            break;
         case 5://group
            {
               wxTreeItemId gpar_item = GetItemParent(par_item);
               wxString gpar_name = GetItemText(gpar_item);
               VRenderView* vrv = vr_frame->GetView(gpar_name);
               if (!vrv)
                  break;
               LayerInfo* item_data = (LayerInfo*)GetItemData(sel_item);
               if (item_data && item_data->type == 2)
                  vrv->RemoveVolumeData(name_data);
               vr_frame->UpdateTree();
               vr_frame->RefreshVRenderViews();
               vr_frame->OnSelection(1);

               if (vrv->GetVolMethod() == VOL_METHOD_MULTI)
               {
                  AdjustView* adjust_view = vr_frame->GetAdjustView();
                  if (adjust_view)
                  {
                     adjust_view->SetRenderView(vrv);
                     adjust_view->UpdateSync();
                  }
               }
            }
            break;
         case 6://mesh group
            {
               wxTreeItemId gpar_item = GetItemParent(par_item);
               wxString gpar_name = GetItemText(gpar_item);
               VRenderView* vrv = vr_frame->GetView(gpar_name);
               if (!vrv)
                  break;
               LayerInfo* item_data = (LayerInfo*)GetItemData(sel_item);
               if (item_data && item_data->type==3)
                  vrv->RemoveMeshData(name_data);
               vr_frame->UpdateTree();
               vr_frame->RefreshVRenderViews();
               vr_frame->OnSelection(1);
            }
            break;
         }
      }
   }
}

void DataTreeCtrl::OnContextMenu(wxContextMenuEvent &event )
{
   if (m_fixed)
      return;

   int flag;
   wxTreeItemId sel_item = HitTest(ScreenToClient(event.GetPosition()), flag);
   VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
   if (!vr_frame) return;

   if (sel_item.IsOk() && vr_frame)
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
               wxString str = GetItemText(sel_item);
               if (str != vr_frame->GetView(0)->GetName())
                  menu.Append(ID_CloseView, "Close");
               menu.Append(ID_AddDataGroup, "Add Volume Group");
               menu.Append(ID_AddMeshGroup, "Add Mesh Group");
               menu.Append(ID_RandomizeColor, "Randomize Colors");
            }
            break;
         case 2:  //volume data
            menu.Append(ID_ToggleDisp, "Toggle Visibility");
            menu.Append(ID_Isolate, "Isolate");
            menu.Append(ID_RemoveData, "Delete");
            menu.Append(ID_AddDataGroup, "Add Volume Group");
            menu.Append(ID_Edit, "Edit...");
            menu.Append(ID_Measurement, "Measurement...");
            menu.Append(ID_NoiseCancelling, "Noise Reduction...");
            menu.Append(ID_Counting, "Counting and Volume...");
            menu.Append(ID_Colocalization, "Colocalization Analysis...");
            menu.Append(ID_RandomizeColor, "Randomize Colors");
            break;
         case 3:  //mesh data
            menu.Append(ID_ToggleDisp, "Toggle Visibility");
            menu.Append(ID_Isolate, "Isolate");
            menu.Append(ID_RemoveData, "Delete");
            menu.Append(ID_ManipulateData, "Manipulate");
            menu.Append(ID_AddMeshGroup, "Add Mesh Group");
            menu.Append(ID_RandomizeColor, "Randomize Colors");
            break;
         case 4:  //annotations
            break;
         case 5:  //data group
            menu.Append(ID_ToggleDisp, "Toggle Visibility");
            if (IsExpanded(sel_item))
               menu.Append(ID_Expand, "Collapse");
            else
               menu.Append(ID_Expand, "Expand");
            menu.Append(ID_RemoveData, "Delete");
            menu.Append(ID_AddDataGroup, "Add Volume Group");
            menu.Append(ID_RandomizeColor, "Randomize Colors");
            break;
         case 6:  //mesh group
            menu.Append(ID_ToggleDisp, "Toggle Visibility");
            if (IsExpanded(sel_item))
               menu.Append(ID_Expand, "Collapse");
            else
               menu.Append(ID_Expand, "Expand");
            menu.Append(ID_RemoveData, "Delete");
            menu.Append(ID_AddMeshGroup, "Add Mesh Group");
            menu.Append(ID_RandomizeColor, "Randomize Colors");
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
   VRenderFrame* vr_frame = (VRenderFrame*)m_frame;

   if (sel_item.IsOk() && vr_frame)
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

      VRenderView* vrv = vr_frame->GetView(viewname);
      if (vrv)
      {
         vrv->Isolate(item_type, itemname);
         vrv->RefreshGL();
         vr_frame->UpdateTreeIcons();
      }
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
   VRenderFrame* vr_frame = (VRenderFrame*)m_frame;

   if (sel_item.IsOk() && vr_frame)
   {
      LayerInfo* item_data = (LayerInfo*)GetItemData(sel_item);
      if (item_data && item_data->type == 1)//view
      {
         wxString name = GetItemText(sel_item);
         vr_frame->DeleteVRenderView(name);
      }
   }
}

void DataTreeCtrl::OnManipulateData(wxCommandEvent& event)
{
   if (m_fixed)
      return;

   wxTreeItemId sel_item = GetSelection();
   VRenderFrame* vr_frame = (VRenderFrame*)m_frame;

   if (sel_item.IsOk() && vr_frame)
   {
      LayerInfo* item_data = (LayerInfo*)GetItemData(sel_item);
      if (item_data && item_data->type == 3)//mesh data
      {
         wxString name = GetItemText(sel_item);
         MeshData* md = vr_frame->GetDataManager()->GetMeshData(name);
         vr_frame->OnSelection(6, 0, 0, 0, md);
      }
   }
}

void DataTreeCtrl::OnAddMeshGroup(wxCommandEvent &event)
{
   if (m_fixed)
      return;

   wxTreeItemId sel_item = GetSelection();
   VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
   if (!vr_frame) return;

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
         VRenderView* vrv = vr_frame->GetView(name);
         if (vrv)
         {
            wxString group_name = vrv->AddMGroup();
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
               VRenderView* vrv = vr_frame->GetView(name);
               if (vrv)
               {
                  wxString group_name = vrv->AddMGroup();
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
                     VRenderView* vrv = vr_frame->GetView(name);
                     if (vrv)
                     {
                        wxString group_name = vrv->AddMGroup();
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
               VRenderView* vrv = vr_frame->GetView(name);
               if (vrv)
               {
                  wxString group_name = vrv->AddMGroup();
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
                     VRenderView* vrv = vr_frame->GetView(name);
                     if (vrv)
                     {
                        wxString group_name = vrv->AddMGroup();
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
               VRenderView* vrv = vr_frame->GetView(name);
               if (vrv)
               {
                  wxString group_name = vrv->AddMGroup();
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
   VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
   if (!vr_frame) return;

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
         VRenderView* vrv = vr_frame->GetView(name);
         if (vrv)
         {
            wxString group_name = vrv->AddGroup();
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
               VRenderView* vrv = vr_frame->GetView(name);
               if (vrv)
               {
                  wxString group_name = vrv->AddGroup();
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
                     VRenderView* vrv = vr_frame->GetView(name);
                     if (vrv)
                     {
                        wxString group_name = vrv->AddGroup();
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
               VRenderView* vrv = vr_frame->GetView(name);
               if (vrv)
               {
                  wxString group_name = vrv->AddGroup();
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
                     VRenderView* vrv = vr_frame->GetView(name);
                     if (vrv)
                     {
                        wxString group_name = vrv->AddGroup();
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
               VRenderView* vrv = vr_frame->GetView(name);
               if (vrv)
               {
                  wxString group_name = vrv->AddGroup();
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
   VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
   if (vr_frame)
      vr_frame->ShowPaintTool();
}

void DataTreeCtrl::OnMeasurement(wxCommandEvent &event)
{
   VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
   if (vr_frame)
      vr_frame->ShowMeasureDlg();
}

//noise cancelling
void DataTreeCtrl::OnNoiseCancelling(wxCommandEvent &event)
{
   VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
   if (vr_frame)
      vr_frame->ShowNoiseCancellingDlg();
}

//counting
void DataTreeCtrl::OnCounting(wxCommandEvent& event)
{
   VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
   if (vr_frame)
      vr_frame->ShowCountingDlg();
}

//colocalzation
void DataTreeCtrl::OnColocalization(wxCommandEvent& event)
{
   VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
   if (vr_frame)
      vr_frame->ShowColocalizationDlg();
}

//randomize color
void DataTreeCtrl::OnRandomizeColor(wxCommandEvent& event)
{
   if (m_fixed)
      return;

   wxTreeItemId sel_item = GetSelection();
   VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
   if (!vr_frame) return;
   if (!sel_item.IsOk()) return;

   LayerInfo* item_data = (LayerInfo*)GetItemData(sel_item);
   if (!item_data) return;

   wxString name = GetItemText(sel_item);
   if (item_data->type == 1)
   {
      //view
      VRenderView* vrv = vr_frame->GetView(name);
      if (vrv)
         vrv->RandomizeColor();
   }
   else if (item_data->type == 2)
   {
      //volume
      VolumeData* vd = vr_frame->GetDataManager()->GetVolumeData(name);
      if (vd)
         vd->RandomizeColor();
   }
   else if (item_data->type == 3)
   {
      //mesh
      MeshData* md = vr_frame->GetDataManager()->GetMeshData(name);
      if (md)
         md->RandomizeColor();
   }
   else if (item_data->type == 5)
   {
      //volume group
      wxString par_name = GetItemText(GetItemParent(sel_item));
      VRenderView* vrv = vr_frame->GetView(par_name);
      if (vrv)
      {
         DataGroup* group = vrv->GetGroup(name);
         if (group)
            group->RandomizeColor();
      }
   }
   else if (item_data->type == 6)
   {
      //mesh group
      wxString par_name = GetItemText(GetItemParent(sel_item));
      VRenderView* vrv = vr_frame->GetView(par_name);
      if (vrv)
      {
         MeshGroup* group = vrv->GetMGroup(name);
         if (group)
            group->RandomizeColor();
      }
   }

   m_scroll_pos = GetScrollPos(wxVERTICAL);
   vr_frame->UpdateTree(name);
   SetScrollPos(wxVERTICAL, m_scroll_pos);
   UpdateSelection();
   vr_frame->RefreshVRenderViews();
}

//
void DataTreeCtrl::UpdateSelection()
{
   wxTreeItemId sel_item = GetSelection();
   VRenderFrame* vr_frame = (VRenderFrame*)m_frame;

   if (!vr_frame)
      return;

   //clear volume A for all views
   for (int i=0; i<vr_frame->GetViewNum(); i++)
   {
      VRenderView* vrv = vr_frame->GetView(i);
      if (vrv)
         vrv->SetVolumeA(0);
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
         case 1://view
            vr_frame->OnSelection(0);
            break;
         case 2://volume data
            {
               if (vr_frame->GetAdjustView())
               {
                  wxTreeItemId par_item = GetItemParent(sel_item);
                  if (par_item.IsOk())
                  {
                     LayerInfo* par_item_data = (LayerInfo*)GetItemData(par_item);
                     if (par_item_data && par_item_data->type == 5)
                     {
                        //par is group
                        wxString str = GetItemText(GetItemParent(par_item));
                        VRenderView* vrv = vr_frame->GetView(str);
                        if (vrv)
                        {
                           VolumeData* vd = vr_frame->GetDataManager()->GetVolumeData(name);
                           str = GetItemText(par_item);
                           DataGroup* group = vrv->GetGroup(str);
                           vr_frame->GetAdjustView()->SetGroupLink(group);
                           vr_frame->OnSelection(2, vrv, group, vd, 0);
                           vrv->SetVolumeA(vd);
                           vr_frame->GetBrushToolDlg()->GetSettings(vrv);
                           vr_frame->GetMeasureDlg()->GetSettings(vrv);
                           vr_frame->GetTraceDlg()->GetSettings(vrv);
                        }
                     }
                     else if (par_item_data && par_item_data->type == 1)
                     {
                        //par is view
                        wxString str = GetItemText(par_item);
                        VRenderView* vrv = vr_frame->GetView(str);
                        if (vrv)
                        {
                           VolumeData* vd = vr_frame->GetDataManager()->GetVolumeData(name);
                           vr_frame->GetAdjustView()->SetGroupLink(0);
                           vr_frame->OnSelection(2, vrv, 0, vd);
                           vrv->SetVolumeA(vd);
                           vr_frame->GetBrushToolDlg()->GetSettings(vrv);
                           vr_frame->GetMeasureDlg()->GetSettings(vrv);
                           vr_frame->GetTraceDlg()->GetSettings(vrv);
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
                     VRenderView* vrv = vr_frame->GetView(str);
                     if (vrv)
                     {
                        MeshData* md = vr_frame->GetDataManager()->GetMeshData(name);
                        vr_frame->OnSelection(3, vrv, 0, 0, md);
                     }
                  }
                  else if (par_item_data && par_item_data->type == 1)
                  {
                     //par is view
                     wxString str = GetItemText(par_item);
                     VRenderView* vrv = vr_frame->GetView(str);
                     if (vrv)
                     {
                        MeshData* md = vr_frame->GetDataManager()->GetMeshData(name);
                        vr_frame->OnSelection(3, vrv, 0, 0, md);
                     }
                  }
               }
            }
            break;
         case 4://annotations
            {
               wxString par_name = GetItemText(GetItemParent(sel_item));
               VRenderView* vrv = vr_frame->GetView(par_name);
               Annotations* ann = vr_frame->GetDataManager()->GetAnnotations(name);
               vr_frame->OnSelection(4, vrv, 0, 0, 0, ann);
            }
            break;
         case 5://group
            {
               wxString par_name = GetItemText(GetItemParent(sel_item));
               VRenderView* vrv = vr_frame->GetView(par_name);
               if (vrv)
               {
                  DataGroup* group = vrv->GetGroup(name);
                  vr_frame->OnSelection(5, vrv, group);
               }
            }
            break;
         case 6://mesh group
            {
               wxString par_name = GetItemText(GetItemParent(sel_item));
               VRenderView* vrv = vr_frame->GetView(par_name);
               if (vrv)
               {
                  vr_frame->OnSelection(0);
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
            if (vr_frame->GetList())
               vr_frame->GetList()->SetSelection(list_type, name);
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
   VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
   wxString name = "";
   bool rc = wxGetKeyState(WXK_CONTROL);

   if (sel_item.IsOk() && vr_frame)
   {
      name = GetItemText(sel_item);
      LayerInfo* item_data = (LayerInfo*)GetItemData(sel_item);
      if (item_data)
      {
         switch (item_data->type)
         {
         case 1://view
            {
               VRenderView* vrv = vr_frame->GetView(name);
               if (vrv)
               {
                  if (rc)
                     vrv->RandomizeColor();
                  else
                     vrv->ToggleDraw();
               }
            }
            break;
         case 2://volume data
            {
               VolumeData* vd = vr_frame->GetDataManager()->GetVolumeData(name);
               if (vd)
               {
                  if (rc)
                     vd->RandomizeColor();
                  else
                  {
                     vd->ToggleDisp();
                     for (int i=0; i<vr_frame->GetViewNum(); i++)
                     {
                        VRenderView* vrv = vr_frame->GetView(i);
                        if (vrv)
                           vrv->SetVolPopDirty();
                     }
                  }
               }
            }
            break;
         case 3://mesh data
            {
               MeshData* md = vr_frame->GetDataManager()->GetMeshData(name);
               if (md)
               {
                  if (rc)
                     md->RandomizeColor();
                  else
                  {
                     md->ToggleDisp();
                     for (int i=0; i<vr_frame->GetViewNum(); i++)
                     {
                        VRenderView* vrv = vr_frame->GetView(i);
                        if (vrv)
                           vrv->SetMeshPopDirty();
                     }
                  }
               }
            }
            break;
         case 4://annotations
            {
               Annotations* ann = vr_frame->GetDataManager()->GetAnnotations(name);
               if (ann)
               {
                  ann->ToggleDisp();
               }
            }
            break;
         case 5://group
            {
               wxString par_name = GetItemText(GetItemParent(sel_item));
               VRenderView* vrv = vr_frame->GetView(par_name);
               if (vrv)
               {
                  DataGroup* group = vrv->GetGroup(name);
                  if (group)
                  {
                     if (rc)
                        group->RandomizeColor();
                     else
                     {
                        group->ToggleDisp();
                        vrv->SetVolPopDirty();
                     }
                  }
               }
            }
            break;
         case 6://mesh group
            {
               wxString par_name = GetItemText(GetItemParent(sel_item));
               VRenderView* vrv = vr_frame->GetView(par_name);
               if (vrv)
               {
                  MeshGroup* group = vrv->GetMGroup(name);
                  if (group)
                  {
                     if (rc)
                        group->RandomizeColor();
                     else
                     {
                        group->ToggleDisp();
                        vrv->SetMeshPopDirty();
                     }
                  }
               }
            }
            break;
         }
      }

      m_scroll_pos = GetScrollPos(wxVERTICAL);
      if (rc)
         vr_frame->UpdateTree(name);
      else
         vr_frame->UpdateTreeIcons();
      SetScrollPos(wxVERTICAL, m_scroll_pos);
      UpdateSelection();
      vr_frame->RefreshVRenderViews();
   }
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
   VRenderFrame* vr_frame = (VRenderFrame*)m_frame;

   if (src_item.IsOk() && dst_item.IsOk() &&
         src_par_item.IsOk() &&
         dst_par_item.IsOk() && vr_frame)
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
            VRenderView* vrv = vr_frame->GetView(src_par_name);
            if (vrv)
            {
               wxString str("");
               vrv->MoveLayertoGroup(dst_name, src_name, str);
            }
         }
         else if (src_type==3 && dst_type==6)
         {
            //move mesh into a group
            VRenderView* vrv = vr_frame->GetView(src_par_name);
            if (vrv)
            {
               wxString str("");
               vrv->MoveMeshtoGroup(dst_name, src_name, str);
            }
         }
         else
         {
            VRenderView* vrv = vr_frame->GetView(src_par_name);
            if (vrv)
               vrv->MoveLayerinView(src_name, dst_name);
         }
      }
      else if (src_par_type == 5 &&
            dst_par_type == 5 &&
            src_par_name == dst_par_name &&
            src_name != dst_name)
      {
         //move volume within the same group
         wxString str = GetItemText(GetItemParent(src_par_item));
         VRenderView* vrv = vr_frame->GetView(str);
         if (vrv)
            vrv->MoveLayerinGroup(src_par_name, src_name, dst_name);
      }
      else if (src_par_type == 5 && //par is group
            src_type == 2 && //src is volume
            dst_par_type == 1 && //dst's par is view
            dst_par_name == GetItemText(GetItemParent(src_par_item))) //in same view
      {
         //move volume outside of the group
         if (dst_type == 5) //dst is group
         {
            VRenderView* vrv = vr_frame->GetView(dst_par_name);
            if (vrv)
            {
               wxString str("");
               vrv->MoveLayerfromtoGroup(src_par_name, dst_name, src_name, str);
            }
         }
         else
         {
            VRenderView *vrv = vr_frame->GetView(dst_par_name);
            if (vrv)
               vrv->MoveLayertoView(src_par_name, src_name, dst_name);
         }
      }
      else if (src_par_type == 1 && //src's par is view
            src_type == 2 && //src is volume
            dst_par_type == 5 && //dst's par is group
            src_par_name == GetItemText(GetItemParent(dst_par_item))) //in the same view
      {
         //move volume into group
         VRenderView* vrv = vr_frame->GetView(src_par_name);
         if (vrv)
            vrv->MoveLayertoGroup(dst_par_name, src_name, dst_name);
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
         VRenderView* vrv = vr_frame->GetView(str);
         if (vrv)
            vrv->MoveLayerfromtoGroup(src_par_name, dst_par_name, src_name, dst_name);
      }
      else if (src_type == 2 && //src is volume
            src_par_type == 5 && //src's par is group
            dst_type == 1 && //dst is view
            GetItemText(GetItemParent(src_par_item)) == dst_name) //in the same view
      {
         //move volume outside of the group
         VRenderView* vrv = vr_frame->GetView(dst_name);
         if (vrv)
         {
            wxString str("");
            vrv->MoveLayertoView(src_par_name, src_name, str);
         }
      }
      else if (src_par_type == 6 &&
            dst_par_type == 6 &&
            src_par_name == dst_par_name &&
            src_name != dst_name)
      {
         //move mesh within the same group
         wxString str = GetItemText(GetItemParent(src_par_item));
         VRenderView* vrv = vr_frame->GetView(str);
         if (vrv)
            vrv->MoveMeshinGroup(src_par_name, src_name, dst_name);
      }
      else if (src_par_type == 6 && //par is group
            src_type == 3 && //src is mesh
            dst_par_type == 1 && //dst's par is view
            dst_par_name == GetItemText(GetItemParent(src_par_item))) //in same view
      {
         //move mesh outside of the group
         if (dst_type == 6) //dst is group
         {
            VRenderView* vrv = vr_frame->GetView(dst_par_name);
            if (vrv)
            {
               wxString str("");
               vrv->MoveMeshfromtoGroup(src_par_name, dst_name, src_name, str);
            }
         }
         else
         {
            VRenderView *vrv = vr_frame->GetView(dst_par_name);
            if (vrv)
               vrv->MoveMeshtoView(src_par_name, src_name, dst_name);
         }
      }
      else if (src_par_type == 1 && //src's par is view
            src_type == 3 && //src is mesh
            dst_par_type == 6 && //dst's par is group
            src_par_name == GetItemText(GetItemParent(dst_par_item))) //in the same view
      {
         //move mesh into group
         VRenderView* vrv = vr_frame->GetView(src_par_name);
         if (vrv)
            vrv->MoveMeshtoGroup(dst_par_name, src_name, dst_name);
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
         VRenderView* vrv = vr_frame->GetView(str);
         if (vrv)
            vrv->MoveMeshfromtoGroup(src_par_name, dst_par_name, src_name, dst_name);
      }
      else if (src_type == 3 && //src is mesh
            src_par_type == 6 && //src's par is group
            dst_type == 1 && //dst is view
            GetItemText(GetItemParent(src_par_item)) == dst_name) //in the same view
      {
         //move mesh outside of the group
         VRenderView* vrv = vr_frame->GetView(dst_name);
         if (vrv)
         {
            wxString str("");
            vrv->MoveMeshtoView(src_par_name, src_name, str);
         }
      }

      vr_frame->UpdateTree(src_name);
      vr_frame->RefreshVRenderViews();
   }
   else if (src_item.IsOk() && src_par_item.IsOk() &&
         !dst_item.IsOk() && vr_frame)
   {
      //move volume out of the group
      int src_type = ((LayerInfo*)GetItemData(src_item))->type;
      int src_par_type = ((LayerInfo*)GetItemData(src_par_item))->type;

      wxString src_name = GetItemText(src_item);
      wxString src_par_name = GetItemText(src_par_item);

      if (src_type == 2 && src_par_type == 5)
      {
         wxString str = GetItemText(GetItemParent(src_par_item));
         VRenderView* vrv = vr_frame->GetView(str);
         if (vrv)
         {
            wxString str("");
            vrv->MoveLayertoView(src_par_name, src_name, str);

            vr_frame->UpdateTree(src_name);
            vr_frame->RefreshVRenderViews();
         }
      }
   }

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
         data[i] = new char[len_chng];
         memset(data[i], 0, len_chng);
         char *temp = new char[len_key+1];
         memset(temp, 0, len_key+1);
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
         delete [] temp;
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
      delete [] data[i];
   }
   delete []data;
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
   VRenderFrame* vr_frame = (VRenderFrame*)m_frame;

   if (sel_item.IsOk() && vr_frame)
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
               VRenderView* vrv = vr_frame->GetView(str);
               if (vrv)
               {
                  VolumeData* vd = vr_frame->GetDataManager()->GetVolumeData(name);
                  if (vd)
                  {
                     int int_mode = vrv->GetIntMode();
                     int paint_mode = vrv->GetPaintMode();
                     vrv->SetVolumeA(vd);
                     vrv->SetPaintMode(6);
                     vrv->Segment();
                     vrv->RefreshGL();
                     vrv->SetPaintMode(paint_mode);
                     vrv->SetIntMode(int_mode);
                  }
               }
            }
            else if (par_item_data && par_item_data->type == 1)
            {
               //par is view
               wxString str = GetItemText(par_item);
               VRenderView* vrv = vr_frame->GetView(str);
               if (vrv)
               {
                  VolumeData* vd = vr_frame->GetDataManager()->GetVolumeData(name);
                  if (vd)
                  {
                     int int_mode = vrv->GetIntMode();
                     int paint_mode = vrv->GetPaintMode();
                     vrv->SetVolumeA(vd);
                     vrv->SetPaintMode(6);
                     vrv->Segment();
                     vrv->RefreshGL();
                     vrv->SetPaintMode(paint_mode);
                     vrv->SetIntMode(int_mode);
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
   VRenderFrame* vr_frame = (VRenderFrame*)m_frame;

   if (sel_item.IsOk() && vr_frame)
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
               VRenderView* vrv = vr_frame->GetView(str);
               if (vrv)
               {
                  VolumeData* vd = vr_frame->GetDataManager()->GetVolumeData(name);
                  if (vd)
                  {
                     vrv->SetVolumeA(vd);
                     vrv->Calculate(5, group_name);
                  }
               }
            }
            else if (par_item_data && par_item_data->type == 1)
            {
               //par is view
               wxString str = GetItemText(par_item);
               VRenderView* vrv = vr_frame->GetView(str);
               if (vrv)
               {
                  VolumeData* vd = vr_frame->GetDataManager()->GetVolumeData(name);
                  if (vd)
                  {
                     vrv->SetVolumeA(vd);
                     vrv->Calculate(5);
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
   VRenderFrame* vr_frame = (VRenderFrame*)m_frame;

   if (sel_item.IsOk() && vr_frame)
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
               VRenderView* vrv = vr_frame->GetView(str);
               if (vrv)
               {
                  VolumeData* vd = vr_frame->GetDataManager()->GetVolumeData(name);
                  if (vd)
                  {
                     vrv->SetVolumeA(vd);
                     vrv->Calculate(cal_type, group_name);
                  }
               }
            }
            else if (par_item_data && par_item_data->type == 1)
            {
               //par is view
               wxString str = GetItemText(par_item);
               VRenderView* vrv = vr_frame->GetView(str);
               if (vrv)
               {
                  VolumeData* vd = vr_frame->GetDataManager()->GetVolumeData(name);
                  if (vd)
                  {
                     vrv->SetVolumeA(vd);
                     vrv->Calculate(cal_type);
                  }
               }
            }
         }
      }
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

TreePanel::TreePanel(wxWindow* frame,
      wxWindow* parent,
      wxWindowID id,
      const wxPoint& pos,
      const wxSize& size,
      long style,
      const wxString& name) :
wxPanel(parent, id, pos, size, style, name),
m_frame(frame)
{
   //create data tree
   m_datatree = new DataTreeCtrl(frame, this, wxID_ANY);

   //create tool bar
   m_toolbar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
         wxTB_FLAT|wxTB_TOP|wxTB_NODIVIDER);
   m_toolbar->AddTool(ID_ToggleView, "Toggle View",
         wxGetBitmapFromMemory(listicon_toggle),
         "Toggle the visibility of current selection");
   m_toolbar->AddTool(ID_AddGroup, "Add Group",
         wxGetBitmapFromMemory(listicon_addgroup),
         "Add a volume data group to the selected view");
   m_toolbar->AddTool(ID_AddMGroup, "Add Mesh Group",
         wxGetBitmapFromMemory(listicon_addmgroup),
         "Add a mesh data group to the selected view");
   m_toolbar->AddTool(ID_RemoveData, "Delete",
         wxGetBitmapFromMemory(listicon_delete),
         "Delete current selection");
#ifdef _WIN32
   m_toolbar->AddSeparator();
   m_toolbar->AddCheckTool(ID_BrushAppend, "Highlight",
         wxGetBitmapFromMemory(listicon_brushappend),
         wxNullBitmap,
         "Highlight structures by painting on the render view (hold Shift)");
   m_toolbar->AddCheckTool(ID_BrushDiffuse, "Diffuse",
         wxGetBitmapFromMemory(listicon_brushdiffuse),
         wxNullBitmap,
         "Diffuse highlighted structures by painting (hold Z)");
   m_toolbar->AddCheckTool(ID_BrushDesel, "Reset",
         wxGetBitmapFromMemory(listicon_brushdesel),
         wxNullBitmap,
         "Reset highlighted structures by painting (hold X)");
   m_toolbar->AddTool(ID_BrushClear, "Reset All",
         wxGetBitmapFromMemory(listicon_brushclear),
         "Reset all highlighted structures");
   m_toolbar->AddSeparator();
   m_toolbar->AddTool(ID_BrushErase, "Erase",
         wxGetBitmapFromMemory(listicon_brusherase),
         "Erase highlighted structures");
   m_toolbar->AddTool(ID_BrushCreate, "Extract",
         wxGetBitmapFromMemory(listicon_brushcreate),
         "Extract highlighted structures out and create a new volume");
#endif
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
      m_datatree->DeleteAll();
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

   switch (id)
   {
   case ID_BrushAppend:
      m_toolbar->ToggleTool(ID_BrushAppend, true);
      m_datatree->m_fixed = true;
      break;
   case ID_BrushDiffuse:
      m_toolbar->ToggleTool(ID_BrushDiffuse, true);
      m_datatree->m_fixed = true;
      break;
   case ID_BrushDesel:
      m_toolbar->ToggleTool(ID_BrushDesel, true);
      m_datatree->m_fixed = true;
      break;
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
   VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
   if (vr_frame && vr_frame->GetBrushToolDlg())
   {
      if (m_toolbar->GetToolState(ID_BrushAppend))
         vr_frame->GetBrushToolDlg()->SelectBrush(BrushToolDlg::ID_BrushAppend);
      else
         vr_frame->GetBrushToolDlg()->SelectBrush(0);
   }
}

void TreePanel::OnBrushDiffuse(wxCommandEvent &event)
{
   BrushDiffuse();
   VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
   if (vr_frame && vr_frame->GetBrushToolDlg())
   {
      if (m_toolbar->GetToolState(ID_BrushDiffuse))
         vr_frame->GetBrushToolDlg()->SelectBrush(BrushToolDlg::ID_BrushDiffuse);
      else
         vr_frame->GetBrushToolDlg()->SelectBrush(0);
   }
}

void TreePanel::OnBrushDesel(wxCommandEvent &event)
{
   BrushDesel();
   VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
   if (vr_frame && vr_frame->GetBrushToolDlg())
   {
      if (m_toolbar->GetToolState(ID_BrushDesel))
         vr_frame->GetBrushToolDlg()->SelectBrush(BrushToolDlg::ID_BrushDesel);
      else
         vr_frame->GetBrushToolDlg()->SelectBrush(0);
   }
}

void TreePanel::OnBrushClear(wxCommandEvent &event)
{
   BrushClear();
}

void TreePanel::OnBrushErase(wxCommandEvent &event)
{
   BrushErase();
   VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
   if (vr_frame && vr_frame->GetBrushToolDlg())
      vr_frame->GetBrushToolDlg()->SelectBrush(0);
}

void TreePanel::OnBrushCreate(wxCommandEvent &event)
{
   BrushCreate();
   VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
   if (vr_frame && vr_frame->GetBrushToolDlg())
      vr_frame->GetBrushToolDlg()->SelectBrush(0);
}

//control from outside
void TreePanel::BrushAppend()
{
   m_toolbar->ToggleTool(ID_BrushDiffuse, false);
   m_toolbar->ToggleTool(ID_BrushDesel, false);

   if (m_toolbar->GetToolState(ID_BrushAppend))
   {
      VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
      if (vr_frame)
      {
         for (int i=0; i<vr_frame->GetViewNum(); i++)
         {
            VRenderView* vrv = vr_frame->GetView(i);
            if (vrv)
            {
               vrv->SetIntMode(2);
               vrv->SetPaintMode(2);
               m_datatree->m_fixed = true;
            }
         }
      }
   }
   else
   {
      VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
      if (vr_frame)
      {
         for (int i=0; i<vr_frame->GetViewNum(); i++)
         {
            VRenderView* vrv = vr_frame->GetView(i);
            if (vrv)
            {
               vrv->SetIntMode(1);
               m_datatree->m_fixed = false;
            }
         }
      }
   }
}

void TreePanel::BrushDiffuse()
{
   m_toolbar->ToggleTool(ID_BrushAppend, false);
   m_toolbar->ToggleTool(ID_BrushDesel, false);

   if (m_toolbar->GetToolState(ID_BrushDiffuse))
   {
      VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
      if (vr_frame)
      {
         for (int i=0; i<vr_frame->GetViewNum(); i++)
         {
            VRenderView* vrv = vr_frame->GetView(i);
            if (vrv)
            {
               vrv->SetIntMode(2);
               vrv->SetPaintMode(4);
               m_datatree->m_fixed = true;
            }
         }
      }
   }
   else
   {
      VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
      if (vr_frame)
      {
         for (int i=0; i<vr_frame->GetViewNum(); i++)
         {
            VRenderView* vrv = vr_frame->GetView(i);
            if (vrv)
            {
               vrv->SetIntMode(1);
               m_datatree->m_fixed = false;
            }
         }
      }
   }
}

void TreePanel::BrushDesel()
{
   m_toolbar->ToggleTool(ID_BrushAppend, false);
   m_toolbar->ToggleTool(ID_BrushDiffuse, false);

   if (m_toolbar->GetToolState(ID_BrushDesel))
   {
      VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
      if (vr_frame)
      {
         for (int i=0; i<vr_frame->GetViewNum(); i++)
         {
            VRenderView* vrv = vr_frame->GetView(i);
            if (vrv)
            {
               vrv->SetIntMode(2);
               vrv->SetPaintMode(3);
               m_datatree->m_fixed = true;
            }
         }
      }
   }
   else
   {
      VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
      if (vr_frame)
      {
         for (int i=0; i<vr_frame->GetViewNum(); i++)
         {
            VRenderView* vrv = vr_frame->GetView(i);
            if (vrv)
            {
               vrv->SetIntMode(1);
               m_datatree->m_fixed = false;
            }
         }
      }
   }
}

void TreePanel::BrushClear()
{
   if (m_datatree)
      m_datatree->BrushClear();

   VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
   if (vr_frame)
   {
      for (int i=0; i<vr_frame->GetViewNum(); i++)
      {
         VRenderView* vrv = vr_frame->GetView(i);
         if (vrv)
         {
            vrv->SetIntMode(4);
         }
      }
   }
}

void TreePanel::BrushErase()
{
   m_toolbar->ToggleTool(ID_BrushAppend, false);
   m_toolbar->ToggleTool(ID_BrushDiffuse, false);
   m_toolbar->ToggleTool(ID_BrushDesel, false);

   VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
   if (vr_frame)
   {
      for (int i=0; i<vr_frame->GetViewNum(); i++)
      {
         VRenderView* vrv = vr_frame->GetView(i);
         if (vrv)
         {
            vrv->SetIntMode(1);
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

   VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
   if (vr_frame)
   {
      for (int i=0; i<vr_frame->GetViewNum(); i++)
      {
         VRenderView* vrv = vr_frame->GetView(i);
         if (vrv)
         {
            vrv->SetIntMode(1);
            m_datatree->m_fixed = false;
         }
      }
   }

   if (m_datatree)
      m_datatree->BrushCreate();
}
