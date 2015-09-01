/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2014 Scientific Computing and Imaging Institute,
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
#include "ListPanel.h"
#include "DataManager.h"
#include "VRenderFrame.h"
#include "Formats/png_resource.h"

//resources
#include "img/icons.h"

BEGIN_EVENT_TABLE(DataListCtrl, wxListCtrl)
EVT_LIST_ITEM_ACTIVATED(wxID_ANY, DataListCtrl::OnAct)
EVT_LIST_ITEM_SELECTED(wxID_ANY, DataListCtrl::OnSelect)
EVT_CONTEXT_MENU(DataListCtrl::OnContextMenu)
EVT_MENU_RANGE(Menu_View_start, Menu_View_start+10, DataListCtrl::OnAddToView)
EVT_MENU(Menu_Del, DataListCtrl::OnDelete)
EVT_MENU(Menu_Rename, DataListCtrl::OnRename)
EVT_MENU(Menu_Save, DataListCtrl::OnSave)
EVT_MENU(Menu_Bake, DataListCtrl::OnBake)
EVT_KEY_DOWN(DataListCtrl::OnKeyDown)
EVT_KEY_UP(DataListCtrl::OnKeyUp)
EVT_MOUSE_EVENTS(DataListCtrl::OnMouse)
EVT_TEXT_ENTER(ID_RenameText, DataListCtrl::OnEndEditName)
EVT_SCROLLWIN(DataListCtrl::OnScroll)
EVT_MOUSEWHEEL(DataListCtrl::OnScroll)
END_EVENT_TABLE()

DataListCtrl::DataListCtrl(
      wxWindow* frame,
      wxWindow* parent,
      wxWindowID id,
      const wxPoint& pos,
      const wxSize& size,
      long style) :
   wxListCtrl(parent, id, pos, size, style),
   m_frame(frame)
{
   wxListItem itemCol;
   itemCol.SetText("Type");
   this->InsertColumn(0, itemCol);

   itemCol.SetText("Name");
   this->InsertColumn(1, itemCol);

   itemCol.SetText("Path");
   this->InsertColumn(2, itemCol);

   m_rename_text = new wxTextCtrl(this, ID_RenameText, "",
         wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
   m_rename_text->Hide();
}

DataListCtrl::~DataListCtrl()
{
	delete m_rename_text;
}

void DataListCtrl::Append(int type, wxString name, wxString path)
{
   long tmp = 0;
   if (type == DATA_VOLUME)
      tmp = InsertItem(GetItemCount(), "Volume");
   else if (type == DATA_MESH)
      tmp = InsertItem(GetItemCount(), "Mesh");
   else if (type == DATA_ANNOTATIONS)
      tmp = InsertItem(GetItemCount(), "Annotations");

   SetItem(tmp, 1, name);
   SetItem(tmp, 2, path);
   SetColumnWidth(0, wxLIST_AUTOSIZE);
   SetColumnWidth(1, wxLIST_AUTOSIZE);
   SetColumnWidth(2, wxLIST_AUTOSIZE);
}

wxString DataListCtrl::GetText(long item, int col)
{
   wxListItem info;
   info.SetId(item);
   info.SetColumn(col);
   info.SetMask(wxLIST_MASK_TEXT);
   GetItem(info);
   return info.GetText();
}

void DataListCtrl::SetText(long item, int col, wxString &str)
{
   wxListItem info;
   info.SetId(item);
   info.SetColumn(col);
   info.SetMask(wxLIST_MASK_TEXT);
   GetItem(info);
   info.SetText(str);
   SetItem(info);
}

void DataListCtrl::SetSelection(int type, wxString &name)
{
   wxString str_type;
   switch (type)
   {
   case DATA_VOLUME:
      str_type = "Volume";
      break;
   case DATA_MESH:
      str_type = "Mesh";
      break;
   case DATA_ANNOTATIONS:
      str_type = "Annotations";
      break;
   }

   long item = -1;
   for (;;)
   {
      item = GetNextItem(item,
            wxLIST_NEXT_ALL,
            wxLIST_STATE_DONTCARE);
      if (item != -1)
      {
         wxString item_type = GetText(item, 0);
         wxString item_name = GetText(item, 1);

         if (item_type == str_type &&
               item_name == name)
         {
            SetItemState(item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
            break;
         }
      }
      else
         break;
   }
}

void DataListCtrl::OnContextMenu(wxContextMenuEvent &event )
{
   if (GetSelectedItemCount()>0)
   {
      wxPoint point = event.GetPosition();
      // If from keyboard
      if (point.x == -1 && point.y == -1)
      {
         wxSize size = GetSize();
         point.x = size.x / 2;
         point.y = size.y / 2;
      }
      else
      {
         point = ScreenToClient(point);
      }

      VRenderFrame* vr_frame = (VRenderFrame*)m_frame;

      if (vr_frame)
      {
         wxMenu menu;
         wxMenu *add_to_menu = new wxMenu;
         for (int i=0 ; i<(int)vr_frame->GetViewList()->size() ; i++)
         {
            add_to_menu->Append(Menu_View_start+i, (*vr_frame->GetViewList())[i]->GetName());
         }

         menu.Append(Menu_AddTo, "Add to", add_to_menu);
         if (GetSelectedItemCount()==1)
         {
            menu.Append(Menu_Del, "Delete");
            menu.Append(Menu_Rename, "Rename");
            //save/save as
            long item = GetNextItem(-1,
                  wxLIST_NEXT_ALL,
                  wxLIST_STATE_SELECTED);
            if (item!=-1)
            {
               if (GetItemText(item) == "Volume")
               {
                  wxString name = GetText(item, 1);
                  VolumeData* vd = vr_frame->GetDataManager()->GetVolumeData(name);
                  if (vd)
                  {
                     if (vd->GetPath() == "")
                        menu.Append(Menu_Save, "Save...");
                     else
                        menu.Append(Menu_Save, "Save As...");
                     menu.Append(Menu_Bake, "Bake...");
                  }
               }
               else if (GetItemText(item) == "Mesh")
               {
                  wxString name = GetText(item, 1);
                  MeshData* md = vr_frame->GetDataManager()->GetMeshData(name);
                  if (md)
                  {
                     if (md->GetPath() == "")
                        menu.Append(Menu_Save, "Save...");
                     else
                        menu.Append(Menu_Save, "Save As...");
                  }
               }
               else if (GetItemText(item) == "Annotations")
               {
                  wxString name = GetText(item, 1);
                  Annotations* ann = vr_frame->GetDataManager()->GetAnnotations(name);
                  if (ann)
                  {
                     if (ann->GetPath() == "")
                        menu.Append(Menu_Save, "Save...");
                     else
                        menu.Append(Menu_Save, "Save As...");
                  }
               }
            }
         }

         PopupMenu( &menu, point.x, point.y);
      }
   }
}

void DataListCtrl::AddToView(int menu_index, long item)
{
   VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
   bool view_empty = true;
   wxString name = "";

   if (vr_frame)
   {
      if (GetItemText(item) == "Volume")
      {
         name = GetText(item, 1);
         VolumeData* vd = vr_frame->GetDataManager()->GetVolumeData(name);
         if (vd)
         {
            VRenderView* view = (*vr_frame->GetViewList())[menu_index];
            if (view)
            {
               VolumeData* vd_add = vd;

               for (int i=0; i<(int)vr_frame->GetViewList()->size(); i++)
               {
                  VRenderView* vrv = (*vr_frame->GetViewList())[i];
                  if (vrv && vrv->GetVolumeData(name))
                  {
                     vd_add = vr_frame->GetDataManager()->DuplicateVolumeData(vd);
                     break;
                  }
               }

               int chan_num = view->GetAny();
               view_empty = chan_num>0?false:view_empty;
               Color color(1.0, 1.0, 1.0);
               if (chan_num == 0)
                  color = Color(1.0, 0.0, 0.0);
               else if (chan_num == 1)
                  color = Color(0.0, 1.0, 0.0);
               else if (chan_num == 2)
                  color = Color(0.0, 0.0, 1.0);

               if (chan_num >=0 && chan_num <3)
                  vd_add->SetColor(color);

               DataGroup *group = view->AddVolumeData(vd_add);
               vr_frame->OnSelection(2, view, group, vd_add, 0);
               if (view->GetVolMethod() == VOL_METHOD_MULTI)
               {
                  AdjustView* adjust_view = vr_frame->GetAdjustView();
                  if (adjust_view)
                  {
                     adjust_view->SetRenderView(view);
                     adjust_view->UpdateSync();
                  }
               }
            }
         }
      }
      else if (GetItemText(item) == "Mesh")
      {
         name = GetText(item, 1);
         MeshData* md = vr_frame->GetDataManager()->GetMeshData(name);
         if (md)
         {
            VRenderView* view = (*vr_frame->GetViewList())[menu_index];
            if (view)
            {
               int chan_num = view->GetAny();
               view_empty = chan_num>0?false:view_empty;
               view->AddMeshData(md);
            }
         }
      }
      else if (GetItemText(item) == "Annotations")
      {
         name = GetText(item, 1);
         Annotations* ann = vr_frame->GetDataManager()->GetAnnotations(name);
         if (ann)
         {
            VRenderView* view = (*vr_frame->GetViewList())[menu_index];
            if (view)
            {
               int chan_num = view->GetAny();
               view_empty = chan_num>0?false:view_empty;
               view->AddAnnotations(ann);
            }
         }
      }

      //update
      if (view_empty)
         (*vr_frame->GetViewList())[menu_index]->InitView(INIT_BOUNDS|INIT_CENTER|INIT_TRANSL|INIT_ROTATE);
      else
         (*vr_frame->GetViewList())[menu_index]->InitView(INIT_BOUNDS|INIT_CENTER);
      (*vr_frame->GetViewList())[menu_index]->RefreshGL();
      vr_frame->UpdateTree(name);
   }
}

void DataListCtrl::OnAddToView(wxCommandEvent& event)
{
   int menu_index = event.GetId() - Menu_View_start;
   int num = GetSelectedItemCount();

   if (num > 0)
   {
      long item = -1;
      for ( ;; )
      {
         item = GetNextItem(item,
               wxLIST_NEXT_ALL,
               wxLIST_STATE_SELECTED);
         if ( item == -1 )
            break;

         AddToView(menu_index, item);
      }
   }
}

void DataListCtrl::OnDelete(wxCommandEvent &event)
{
   DeleteSelection();
}

void DataListCtrl::OnRename(wxCommandEvent& event)
{
   long item = GetNextItem(-1,
         wxLIST_NEXT_ALL,
         wxLIST_STATE_SELECTED);
   if (item != -1)
   {
      wxRect rect;
      GetSubItemRect(item, 1, rect);
      wxString str = GetText(item, 1);
      m_rename_text->SetPosition(rect.GetTopLeft());
      m_rename_text->SetSize(rect.GetSize());
      m_rename_text->SetValue(str);
      m_rename_text->SetFocus();
      m_rename_text->SetSelection(-1, -1);
      m_rename_text->Show();
   }
}

//ch1
void DataListCtrl::OnCh1Check(wxCommandEvent &event)
{
   wxCheckBox* ch1 = (wxCheckBox*)event.GetEventObject();
   if (ch1)
      VRenderFrame::SetCompression(ch1->GetValue());
}

wxWindow* DataListCtrl::CreateExtraControl(wxWindow* parent)
{
   wxPanel* panel = new wxPanel(parent, 0, wxDefaultPosition, wxSize(400, 90));

   wxBoxSizer *group1 = new wxStaticBoxSizer(
         new wxStaticBox(panel, wxID_ANY, "Additional Options"), wxVERTICAL);

   //compressed
   wxCheckBox* ch1 = new wxCheckBox(panel, wxID_HIGHEST+3004,
         "Lempel-Ziv-Welch Compression");
   ch1->Connect(ch1->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
         wxCommandEventHandler(DataListCtrl::OnCh1Check), NULL, panel);
   if (ch1)
      ch1->SetValue(VRenderFrame::GetCompression());

   //group
   group1->Add(10, 10);
   group1->Add(ch1);
   group1->Add(10, 10);

   panel->SetSizer(group1);
   panel->Layout();

   return panel;
}

void DataListCtrl::OnSave(wxCommandEvent& event)
{
   long item = GetNextItem(-1,
         wxLIST_NEXT_ALL,
         wxLIST_STATE_SELECTED);
   if (item != -1)
   {
      wxString name = GetText(item, 1);

      if (GetItemText(item) == "Volume")
      {
         wxFileDialog *fopendlg = new wxFileDialog(
               m_frame, "Save Volume Data", "", "",
               "Muti-page Tiff file (*.tif, *.tiff)|*.tif;*.tiff|"\
               "Single-page Tiff sequence (*.tif)|*.tif;*.tiff|"\
               "Nrrd file (*.nrrd)|*.nrrd",
               wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
         fopendlg->SetExtraControlCreator(CreateExtraControl);

         int rval = fopendlg->ShowModal();

         if (rval == wxID_OK)
         {
             wxString filename = fopendlg->GetPath();

            VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
            if (vr_frame)
            {
               VolumeData* vd = vr_frame->GetDataManager()->GetVolumeData(name);
               if (vd)
               {
                  vd->Save(filename, fopendlg->GetFilterIndex(), false, VRenderFrame::GetCompression());
                  wxString str = vd->GetPath();
                  SetText(item, 2, str);
               }
            }
         }
         delete fopendlg;
      }
      else if (GetItemText(item) == "Mesh")
      {
         wxFileDialog *fopendlg = new wxFileDialog(
               m_frame, "Save Mesh Data", "", "",
               "OBJ file (*.obj)|*.obj",
               wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

         int rval = fopendlg->ShowModal();

         if (rval == wxID_OK)
         {
            wxString filename = fopendlg->GetPath();

            VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
            if(vr_frame)
            {
               MeshData* md = vr_frame->GetDataManager()->GetMeshData(name);
               if (md)
               {
                  md->Save(filename);
                  wxString str = md->GetPath();
                  SetText(item, 2, str);
               }
            }
         }
         delete fopendlg;
      }
      else if (GetItemText(item) == "Annotations")
      {
         wxFileDialog *fopendlg = new wxFileDialog(
               m_frame, "Save Annotations", "", "",
               "Text file (*.txt)|*.txt",
               wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

         int rval = fopendlg->ShowModal();

         if (rval == wxID_OK)
         {
            wxString filename = fopendlg->GetPath();

            VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
            if(vr_frame)
            {
               Annotations* ann = vr_frame->GetDataManager()->GetAnnotations(name);
               if (ann)
               {
                  ann->Save(filename);
                  wxString str = ann->GetPath();
                  SetText(item, 2, str);
               }
            }
         }
         delete fopendlg;
      }
   }
}

void DataListCtrl::OnBake(wxCommandEvent& event)
{
   long item = GetNextItem(-1,
         wxLIST_NEXT_ALL,
         wxLIST_STATE_SELECTED);
   if (item != -1 && GetItemText(item) == "Volume")
   {
      wxString name = GetText(item, 1);

      wxFileDialog *fopendlg = new wxFileDialog(
            m_frame, "Bake Volume Data", "", "",
            "Muti-page Tiff file (*.tif, *.tiff)|*.tif;*.tiff|"\
            "Single-page Tiff sequence (*.tif)|*.tif;*.tiff|"\
            "Nrrd file (*.nrrd)|*.nrrd",
            wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
      fopendlg->SetExtraControlCreator(CreateExtraControl);

      int rval = fopendlg->ShowModal();

      if (rval == wxID_OK)
      {
         wxString filename = fopendlg->GetPath();

         VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
         if (vr_frame)
         {
            VolumeData* vd = vr_frame->GetDataManager()->GetVolumeData(name);
            if (vd)
            {
               vd->Save(filename, fopendlg->GetFilterIndex(), true, VRenderFrame::GetCompression());
               wxString str = vd->GetPath();
               SetText(item, 2, str);
            }
         }
      }

      delete fopendlg;
   }
}

void DataListCtrl::OnSelect(wxListEvent &event)
{
   long item = GetNextItem(-1,
         wxLIST_NEXT_ALL,
         wxLIST_STATE_SELECTED);

   if (item != -1)
   {
      wxString name = GetText(item, 1);
      VRenderFrame *vr_frame = (VRenderFrame*)m_frame;
      if (vr_frame)
         vr_frame->GetTree()->Select("", name);
   }
}

void DataListCtrl::OnAct(wxListEvent &event)
{
   int index = 0;
   long item = GetNextItem(-1,
         wxLIST_NEXT_ALL,
         wxLIST_STATE_SELECTED);
   VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
   if (vr_frame && item!=-1)
   {
      index = vr_frame->GetViewList()->size()-1;
      AddToView(index, item);
   }
}

void DataListCtrl::OnKeyDown(wxKeyEvent& event)
{
   if ( event.GetKeyCode() == WXK_DELETE ||
         event.GetKeyCode() == WXK_BACK)
      DeleteSelection();
   //event.Skip();
}

void DataListCtrl::OnKeyUp(wxKeyEvent& event)
{
   event.Skip();
}

void DataListCtrl::OnMouse(wxMouseEvent &event)
{
   if (event.Button(wxMOUSE_BTN_ANY))
      m_rename_text->Hide();
   event.Skip();
}

void DataListCtrl::EndEdit(bool update)
{
	if (!m_rename_text->IsShown())
		return;

	if (update)
	{
		wxString new_name = m_rename_text->GetValue();

		long item = GetNextItem(-1,
			wxLIST_NEXT_ALL,
			wxLIST_STATE_SELECTED);
		VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
		DataManager* mgr = vr_frame?vr_frame->GetDataManager():0;

		if (item != -1 && mgr)
		{
			wxString name = GetText(item, 1);

			if (new_name != name)
			{
				wxString new_name2 = new_name;
				for (int i=1; mgr->CheckNames(new_name2); i++)
					new_name2 = new_name+wxString::Format("_%d", i);


				if (GetItemText(item) == "Volume")
				{
					VolumeData* vd = mgr->GetVolumeData(name);
					if (vd)
						vd->SetName(new_name2);
				}
				else if (GetItemText(item) == "Mesh")
				{
					MeshData* md = mgr->GetMeshData(name);
					if (md)
						md->SetName(new_name2);
				}
				else if (GetItemText(item) == "Annotations")
				{
					Annotations* ann = mgr->GetAnnotations(name);
					if (ann)
						ann->SetName(new_name2);
				}

				//update ui
				SetText(item, 1, new_name2);
				vr_frame->UpdateTree();
			}
		}
	}

   m_rename_text->Hide();
}

void DataListCtrl::OnEndEditName(wxCommandEvent& event)
{
	EndEdit();
}

void DataListCtrl::DeleteSelection()
{
   VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
   wxString name = "";

   if (vr_frame && GetSelectedItemCount()>0)
   {
      long item = GetNextItem(-1,
            wxLIST_NEXT_ALL,
            wxLIST_STATE_SELECTED);
      if (item != -1)
      {
         if (GetItemText(item) == "Volume")
         {
            name = GetText(item, 1);
            int i;
            //from view
            for (i=0; i<(int)vr_frame->GetViewList()->size(); i++)
            {
               VRenderView* view = (*vr_frame->GetViewList())[i];
               if (view)
               {
                  view->RemoveVolumeData(name);
               }
            }
            //from datamanager
            DataManager* mgr = vr_frame->GetDataManager();
            if (mgr)
            {
               int index = mgr->GetVolumeIndex(name);
               if (index != -1)
               {
                  mgr->RemoveVolumeData(index);
               }
            }
         }
         else if (GetItemText(item) == "Mesh")
         {
            name = GetText(item, 1);
            int i;
            //from view
            for (i=0; i<(int)vr_frame->GetViewList()->size(); i++)
            {
               VRenderView* view = (*vr_frame->GetViewList())[i];
               if (view)
               {
                  view->RemoveMeshData(name);
               }
            }
            //from datamanager
            DataManager* mgr = vr_frame->GetDataManager();
            if (mgr)
            {
               int index = mgr->GetMeshIndex(name);
               if (index != -1)
               {
                  mgr->RemoveMeshData(index);
               }
            }
         }
         else if (GetItemText(item) == "Annotations")
         {
            name = GetText(item, 1);
            int i;
            //from view
            for (i=0; i<(int)vr_frame->GetViewList()->size(); i++)
            {
               VRenderView* view = (*vr_frame->GetViewList())[i];
               if (view)
                  view->RemoveAnnotations(name);
            }
            //from datamanager
            DataManager* mgr = vr_frame->GetDataManager();
            if (mgr)
            {
               int index = mgr->GetAnnotationIndex(name);
               if (index != -1)
                  mgr->RemoveAnnotations(index);
            }
         }
      }
      vr_frame->UpdateList();
      vr_frame->UpdateTree(name);
      vr_frame->RefreshVRenderViews();
   }
}

void DataListCtrl::DeleteAll()
{
   VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
   wxString name = "";

   long item = GetNextItem(-1);
   while (item != -1 && vr_frame)
   {
      if (GetItemText(item) == "Volume")
      {
         name = GetText(item, 1);
         int i;
         //from view
         for (i=0; i<(int)vr_frame->GetViewList()->size(); i++)
         {
            VRenderView* view = (*vr_frame->GetViewList())[i];
            if (view)
               view->RemoveVolumeData(name);
         }
         //from datamanager
         DataManager* mgr = vr_frame->GetDataManager();
         if (mgr)
         {
            int index = mgr->GetVolumeIndex(name);
            if (index != -1)
               mgr->RemoveVolumeData(index);
         }
      }
      else if (GetItemText(item) == "Mesh")
      {
         name = GetText(item, 1);
         int i;
         //from view
         for (i=0; i<(int)vr_frame->GetViewList()->size(); i++)
         {
            VRenderView* view = (*vr_frame->GetViewList())[i];
            if (view)
               view->RemoveMeshData(name);
         }
         //from datamanager
         DataManager* mgr = vr_frame->GetDataManager();
         if (mgr)
         {
            int index = mgr->GetMeshIndex(name);
            if (index != -1)
               mgr->RemoveMeshData(index);
         }
      }
      else if (GetItemText(item) == "Annotations")
      {
         name = GetText(item, 1);
         int i;
         //from view
         for (i=0; i<(int)vr_frame->GetViewList()->size(); i++)
         {
            VRenderView* view = (*vr_frame->GetViewList())[i];
            if (view)
               view->RemoveAnnotations(name);
         }
         //from datamanager
         DataManager* mgr = vr_frame->GetDataManager();
         if (mgr)
         {
            int index = mgr->GetAnnotationIndex(name);
            if (index != -1)
               mgr->RemoveAnnotations(index);
         }
      }

      item = GetNextItem(item);
   }

   DeleteAllItems();
   if (vr_frame)
   {
      vr_frame->UpdateTree();
      vr_frame->RefreshVRenderViews();
   }
}

void DataListCtrl::OnScroll(wxScrollWinEvent& event)
{
	EndEdit(false);
	event.Skip(true);
}

void DataListCtrl::OnScroll(wxMouseEvent& event)
{
	EndEdit(false);
	event.Skip(true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(ListPanel, wxPanel)
EVT_TOOL(ID_AddToView, ListPanel::OnAddToView)
EVT_TOOL(ID_Rename, ListPanel::OnRename)
EVT_TOOL(ID_Save, ListPanel::OnSave)
EVT_TOOL(ID_Bake, ListPanel::OnBake)
EVT_TOOL(ID_Delete, ListPanel::OnDelete)
EVT_TOOL(ID_DeleteAll, ListPanel::OnDeleteAll)
END_EVENT_TABLE()

ListPanel::ListPanel(wxWindow *frame,
      wxWindow *parent,
      wxWindowID id,
      const wxPoint &pos,
      const wxSize &size,
      long style,
      const wxString& name) :
wxPanel(parent, id, pos, size, style, name)//,
//m_frame(frame)
{
   //create data list
   m_datalist = new DataListCtrl(frame, this, wxID_ANY);

   //create tool bar
   m_toolbar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
         wxTB_FLAT|wxTB_TOP|wxTB_NODIVIDER);
   m_toolbar->AddTool(ID_AddToView, "Add to View",
         wxGetBitmapFromMemory(listicon_view),
         "Add: Add the selected dataset to render view");
   m_toolbar->AddTool(ID_Rename, "Rename",
         wxGetBitmapFromMemory(listicon_rename),
         "Rename: Rename the selected dataset");
   m_toolbar->AddTool(ID_Save, "Save As",
         wxGetBitmapFromMemory(listicon_save),
         "Save: Save the selected volume dataset");
   m_toolbar->AddTool(ID_Bake, "Bake",
         wxGetBitmapFromMemory(listicon_bake),
         "Bake: Apply the volume properties and save");
   m_toolbar->AddTool(ID_Delete, "Delete",
         wxGetBitmapFromMemory(listicon_delete),
         "Delete: Delete the selected dataset");
   m_toolbar->AddTool(ID_DeleteAll, "Delete All",
         wxGetBitmapFromMemory(listicon_delall),
         "Delete All: Delete all datasets");
   m_toolbar->Realize();

   //organize positions
   wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);

   sizer_v->Add(m_toolbar, 0, wxEXPAND);
   sizer_v->Add(m_datalist, 1, wxEXPAND);

   SetSizer(sizer_v);
   Layout();
}

ListPanel::~ListPanel()
{
}

void ListPanel::Append(int type, wxString name, wxString path)
{
   if (m_datalist)
      m_datalist->Append(type, name, path);
}

wxString ListPanel::GetText(long item, int col)
{
   wxString str = "";
   if (m_datalist)
      str = m_datalist->GetText(item, col);
   return str;
}

void ListPanel::SetText(long item, int col, wxString &str)
{
   if (m_datalist)
      m_datalist->SetText(item, col, str);
}

void ListPanel::DeleteAllItems()
{
   if (m_datalist)
      m_datalist->DeleteAllItems();
}

void ListPanel::SetSelection(int type, wxString &name)
{
   if (m_datalist)
      m_datalist->SetSelection(type, name);
}

void ListPanel::OnAddToView(wxCommandEvent& event)
{
   wxListEvent list_event;
   m_datalist->OnAct(list_event);
}

void ListPanel::OnRename(wxCommandEvent& event)
{
   m_datalist->OnRename(event);
}

void ListPanel::OnSave(wxCommandEvent& event)
{
   m_datalist->OnSave(event);
}

void ListPanel::OnBake(wxCommandEvent& event)
{
   m_datalist->OnBake(event);
}

void ListPanel::OnDelete(wxCommandEvent& event)
{
   m_datalist->DeleteSelection();
}

void ListPanel::OnDeleteAll(wxCommandEvent &event)
{
   m_datalist->DeleteAll();
}
