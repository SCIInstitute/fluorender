#include "MeasureDlg.h"
#include "VRenderFrame.h"
#include <sstream>
#include <fstream>
#include <wx/artprov.h>
#include <wx/valnum.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include "Formats/png_resource.h"
#include "ruler.xpm"

//resources
#include "img/listicon_delall.h"
#include "img/listicon_delete.h"
#include "img/listicon_save.h"
#include "img/listicon_locator.h"
#include "img/listicon_addruler.h"
#include "img/listicon_ruleredit.h"

BEGIN_EVENT_TABLE(RulerListCtrl, wxListCtrl)
EVT_KEY_DOWN(RulerListCtrl::OnKeyDown)
END_EVENT_TABLE()

RulerListCtrl::RulerListCtrl(
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
   itemCol.SetText("ID");
    this->InsertColumn(0, itemCol);
    SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
   itemCol.SetText("Length");
    this->InsertColumn(1, itemCol);
    SetColumnWidth(1, wxLIST_AUTOSIZE_USEHEADER);
   itemCol.SetText("Angle");
    this->InsertColumn(2, itemCol);
    SetColumnWidth(2, wxLIST_AUTOSIZE_USEHEADER);
   itemCol.SetText("Start/End Points (X, Y, Z)");
   this->InsertColumn(3, itemCol);
   SetColumnWidth(3, wxLIST_AUTOSIZE_USEHEADER);
   itemCol.SetText("Time");
    this->InsertColumn(4, itemCol);
    SetColumnWidth(4, wxLIST_AUTOSIZE_USEHEADER);
   itemCol.SetText("Volumes");
    this->InsertColumn(5, itemCol);
    SetColumnWidth(5, wxLIST_AUTOSIZE_USEHEADER);

   m_images = new wxImageList(16, 16, true);
   wxIcon icon = wxIcon(ruler_xpm);
   m_images->Add(icon);
   AssignImageList(m_images, wxIMAGE_LIST_SMALL);
}

RulerListCtrl::~RulerListCtrl()
{
}

void RulerListCtrl::Append(wxString name, double length, wxString &unit,
      double angle, wxString &points, bool time_dep, int time, wxString extra)
{
   long tmp = InsertItem(GetItemCount(), name, 0);
   wxString str = wxString::Format("%.2f", length) + unit;
   SetItem(tmp, 1, str);
   str = wxString::Format("%.1f", angle) + "Deg";
   SetItem(tmp, 2, str);
   SetItem(tmp, 3, points);
   SetColumnWidth(3, wxLIST_AUTOSIZE);
   if (time_dep)
      str = wxString::Format("%d", time);
   else
      str = "N/A";
   SetItem(tmp, 4, str);
   SetItem(tmp, 5, extra);
}

void RulerListCtrl::Update(VRenderView* vrv)
{
   if (vrv)
      m_view = vrv;

   vector<Ruler*>* ruler_list = m_view->GetRulerList();
   if (!ruler_list) return;

   DeleteAllItems();

   wxString points;
   Point *p;
   int num_points;
   for (int i=0; i<(int)ruler_list->size(); i++)
   {
      Ruler* ruler = (*ruler_list)[i];
      if (!ruler) continue;
      if (ruler->GetTimeDep() &&
            ruler->GetTime() != m_view->m_glview->m_tseq_cur_num)
         continue;

      wxString unit;
      switch (m_view->m_glview->m_sb_unit)
      {
      case 0:
         unit = "nm";
         break;
      case 1:
      default:
         unit = L"\u03BCm";
         break;
      case 2:
         unit = "mm";
         break;
      }

      points = "";
      num_points = ruler->GetNumPoint();
      if (num_points > 0)
      {
         p = ruler->GetPoint(0);
			points += wxString::Format("(%.2f, %.2f, %.2f)", p->x(), p->y(), p->z());
		}
		if (num_points > 1)
		{
			p = ruler->GetPoint(num_points - 1);
			points += ", ";
			points += wxString::Format("(%.2f, %.2f, %.2f)", p->x(), p->y(), p->z());
		}
		Append(ruler->GetName(), ruler->GetLength(), unit,
			ruler->GetAngle(), points, ruler->GetTimeDep(), ruler->GetTime(), ruler->GetDelInfoValues(", "));
   }

   long item = GetItemCount() - 1;
   if (item != -1)
      SetItemState(item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
}

void RulerListCtrl::DeleteSelection()
{
   if (!m_view) return;

   long item = GetNextItem(-1,
         wxLIST_NEXT_ALL,
         wxLIST_STATE_SELECTED);
   if (item != -1)
   {
      wxString name = GetItemText(item);
      vector<Ruler*>* ruler_list = m_view->GetRulerList();
      if (ruler_list)
      {
         for (int i=0; i<(int)ruler_list->size(); i++)
         {
            Ruler* ruler = (*ruler_list)[i];
            if (ruler && ruler->GetName()==name)
            {
               ruler_list->erase(ruler_list->begin()+i);
               delete ruler;
            }
         }
         Update();
         m_view->RefreshGL();
      }
   }
}

void RulerListCtrl::DeleteAll(bool cur_time)
{
   if (!m_view) return;

   vector<Ruler*>* ruler_list = m_view->GetRulerList();
   if (ruler_list)
   {
      if (cur_time)
      {
         int tseq = m_view->m_glview->m_tseq_cur_num;
         for (int i=ruler_list->size()-1; i>=0; i--)
         {
            Ruler* ruler = (*ruler_list)[i];
            if (ruler &&
                  ruler->GetTimeDep() &&
                  ruler->GetTime() == tseq)
            {
               ruler_list->erase(ruler_list->begin()+i);
               delete ruler;
            }
         }
      }
      else
      {
         for (int i=ruler_list->size()-1; i>=0; i--)
         {
            Ruler* ruler = (*ruler_list)[i];
            if (ruler)
               delete ruler;
         }
         ruler_list->clear();
      }

      Update();
      m_view->RefreshGL();
   }
}

void RulerListCtrl::Export(wxString filename)
{
   if (!m_view) return;
   vector<Ruler*>* ruler_list = m_view->GetRulerList();
   if (ruler_list)
   {
      wxFileOutputStream fos(filename);
      if (!fos.Ok())
         return;
      wxTextOutputStream tos(fos);

      wxString str;
      wxString unit;
      int num_points;
      Point *p;
      Ruler* ruler;
      switch (m_view->m_glview->m_sb_unit)
      {
      case 0:
         unit = "nm";
         break;
      case 1:
      default:
         unit = L"\u03BCm";
         break;
      case 2:
         unit = "mm";
         break;
      }

      tos << "ID\tLength(" << unit << ")\tAngle(Deg)\tx1\ty1\tz1\txn\tyn\tzn\tTime\tv1\tv2\n";

      for (int i=0; i<ruler_list->size(); i++)
      {
         ruler = (*ruler_list)[i];
         if (!ruler) continue;

         tos << ruler->GetName() << "\t";
         str = wxString::Format("%.2f", ruler->GetLength());
         tos << str << "\t";
         str = wxString::Format("%.1f", ruler->GetAngle());
         tos << str << "\t";
         str = "";
         num_points = ruler->GetNumPoint();
         if (num_points > 0)
         {
            p = ruler->GetPoint(0);
			str += wxString::Format("%.2f\t%.2f\t%.2f", p->x(), p->y(), p->z());
         }
         if (num_points > 1)
         {
            p = ruler->GetPoint(num_points - 1);
            str += "\t";
			str += wxString::Format("%.2f\t%.2f\t%.2f", p->x(), p->y(), p->z());
         }
         tos << str << "\t";
         if (ruler->GetTimeDep())
            str = wxString::Format("%d", ruler->GetTime());
         else
            str = "N/A";
         tos << str << "\t";
         tos << ruler->GetInfoValues() << "\n";
      }
   }
}

void RulerListCtrl::OnKeyDown(wxKeyEvent& event)
{
   if ( event.GetKeyCode() == WXK_DELETE ||
         event.GetKeyCode() == WXK_BACK)
      DeleteSelection();
   event.Skip();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
BEGIN_EVENT_TABLE(MeasureDlg, wxPanel)
EVT_MENU(ID_LocatorBtn, MeasureDlg::OnNewLocator)
EVT_MENU(ID_RulerBtn, MeasureDlg::OnNewRuler)
EVT_MENU(ID_RulerMPBtn, MeasureDlg::OnNewRulerMP)
EVT_MENU(ID_RulerEditBtn, MeasureDlg::OnRulerEdit)
EVT_MENU(ID_DeleteBtn, MeasureDlg::OnDelete)
EVT_MENU(ID_DeleteAllBtn, MeasureDlg::OnDeleteAll)
EVT_MENU(ID_ExportBtn, MeasureDlg::OnExport)
EVT_RADIOBUTTON(ID_ViewPlaneRd, MeasureDlg::OnIntensityMethodCheck)
EVT_RADIOBUTTON(ID_MaxIntensityRd, MeasureDlg::OnIntensityMethodCheck)
EVT_RADIOBUTTON(ID_AccIntensityRd, MeasureDlg::OnIntensityMethodCheck)
EVT_CHECKBOX(ID_UseTransferChk, MeasureDlg::OnUseTransferCheck)
EVT_CHECKBOX(ID_TransientChk, MeasureDlg::OnTransientCheck)
END_EVENT_TABLE()

MeasureDlg::MeasureDlg(wxWindow* frame, wxWindow* parent)
: wxPanel(parent,wxID_ANY,
      wxPoint(500, 150), wxSize(450, 600),
      0, "MeasureDlg"),
   m_frame(parent),
   m_view(0)
{
   //toolbar
   m_toolbar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
         wxTB_FLAT|wxTB_TOP|wxTB_NODIVIDER|wxTB_TEXT);
   m_toolbar->AddCheckTool(ID_LocatorBtn, "Locator",
         wxGetBitmapFromMemory(listicon_locator),
         wxNullBitmap,
         "Add locators to the render view by clicking");
   m_toolbar->AddCheckTool(ID_RulerBtn, "2pt Ruler",
         wxGetBitmapFromMemory(listicon_addruler),
         wxNullBitmap,
         "Add rulers to the render view by clicking two end points");
   m_toolbar->AddCheckTool(ID_RulerMPBtn, "2+pt Ruler",
         wxGetBitmapFromMemory(listicon_addruler),
         wxNullBitmap,
         "Add a polyline ruler to the render view by clicking its points");
   m_toolbar->AddCheckTool(ID_RulerEditBtn, "Edit",
         wxGetBitmapFromMemory(listicon_ruleredit),
         wxNullBitmap,
         "Select and move ruler points");
   m_toolbar->AddTool(ID_DeleteBtn, "Delete",
         wxGetBitmapFromMemory(listicon_delete),
         "Delete a selected ruler");
   m_toolbar->AddTool(ID_DeleteAllBtn,"Delete All",
         wxGetBitmapFromMemory(listicon_delall),
         "Delete all rulers");
   m_toolbar->AddTool(ID_ExportBtn, "Export",
         wxGetBitmapFromMemory(listicon_save),
         "Export rulers to a text file");
   m_toolbar->Realize();

   //options
   wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
   m_view_plane_rd = new wxRadioButton(this, ID_ViewPlaneRd, "View Plane",
         wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
   m_max_intensity_rd = new wxRadioButton(this, ID_MaxIntensityRd, "Maximum Intensity",
         wxDefaultPosition, wxDefaultSize);
   m_acc_intensity_rd = new wxRadioButton(this, ID_AccIntensityRd, "Accumulated Intensity",
         wxDefaultPosition, wxDefaultSize);
   m_view_plane_rd->SetValue(false);
   m_max_intensity_rd->SetValue(true);
   m_acc_intensity_rd->SetValue(false);
   sizer_1->Add(10, 10);
   sizer_1->Add(m_view_plane_rd, 0, wxALIGN_CENTER);
   sizer_1->Add(10, 10);
   sizer_1->Add(m_max_intensity_rd, 0, wxALIGN_CENTER);
   sizer_1->Add(10, 10);
   sizer_1->Add(m_acc_intensity_rd, 0, wxALIGN_CENTER);

   //more options
   wxBoxSizer* sizer_2 = new wxBoxSizer(wxHORIZONTAL);
   m_transient_chk = new wxCheckBox(this, ID_TransientChk, "Transient",
         wxDefaultPosition, wxDefaultSize);
   m_use_transfer_chk = new wxCheckBox(this, ID_UseTransferChk, "Use Volume Properties",
         wxDefaultPosition, wxDefaultSize);
   sizer_2->Add(10, 10);
   sizer_2->Add(m_transient_chk, 0, wxALIGN_CENTER);
   sizer_2->Add(10, 10);
   sizer_2->Add(m_use_transfer_chk, 0, wxALIGN_CENTER);

   //list
   m_rulerlist = new RulerListCtrl(frame, this, wxID_ANY);

   //sizer
   wxBoxSizer *sizerV = new wxBoxSizer(wxVERTICAL);
   sizerV->Add(10, 10);
   sizerV->Add(m_toolbar, 0, wxEXPAND);
   sizerV->Add(10, 10);
   sizerV->Add(sizer_1, 0, wxEXPAND);
   sizerV->Add(10, 10);
   sizerV->Add(sizer_2, 0, wxEXPAND);
   sizerV->Add(10, 10);
   sizerV->Add(m_rulerlist, 1, wxEXPAND);

   SetSizer(sizerV);
   Layout();
}

MeasureDlg::~MeasureDlg()
{
}

void MeasureDlg::GetSettings(VRenderView* vrv)
{
   m_view = vrv;
   UpdateList();
   if (m_view && m_view->m_glview)
   {
      m_toolbar->ToggleTool(ID_LocatorBtn, false);
      m_toolbar->ToggleTool(ID_RulerBtn, false);
      m_toolbar->ToggleTool(ID_RulerMPBtn, false);
      m_toolbar->ToggleTool(ID_RulerEditBtn, false);

      int int_mode = m_view->m_glview->GetIntMode();
		if (int_mode == 5 || int_mode == 7)
      {
         int ruler_type = m_view->GetRulerType();
         if (ruler_type == 0)
            m_toolbar->ToggleTool(ID_RulerBtn, true);
         else if (ruler_type == 1)
            m_toolbar->ToggleTool(ID_RulerMPBtn, true);
         else if (ruler_type == 2)
            m_toolbar->ToggleTool(ID_LocatorBtn, true);
      }
      else if (int_mode == 6)
         m_toolbar->ToggleTool(ID_RulerEditBtn, true);

      switch (m_view->m_glview->m_point_volume_mode)
      {
      case 0:
         m_view_plane_rd->SetValue(true);
         m_max_intensity_rd->SetValue(false);
         m_acc_intensity_rd->SetValue(false);
         break;
      case 1:
         m_view_plane_rd->SetValue(false);
         m_max_intensity_rd->SetValue(true);
         m_acc_intensity_rd->SetValue(false);
         break;
      case 2:
         m_view_plane_rd->SetValue(false);
         m_max_intensity_rd->SetValue(false);
         m_acc_intensity_rd->SetValue(true);
         break;
      }

      m_use_transfer_chk->SetValue(m_view->m_glview->m_ruler_use_transf);
      m_transient_chk->SetValue(m_view->m_glview->m_ruler_time_dep);
   }
}

VRenderView* MeasureDlg::GetView()
{
   return m_view;
}

void MeasureDlg::UpdateList()
{
   if (!m_view) return;
   m_rulerlist->Update(m_view);
}

void MeasureDlg::OnNewLocator(wxCommandEvent& event)
{
   if (!m_view) return;

   if (m_toolbar->GetToolState(ID_RulerMPBtn))
      m_view->FinishRuler();

   m_toolbar->ToggleTool(ID_RulerBtn, false);
   m_toolbar->ToggleTool(ID_RulerMPBtn, false);
   m_toolbar->ToggleTool(ID_RulerEditBtn, false);

   if (m_toolbar->GetToolState(ID_LocatorBtn))
   {
      m_view->SetIntMode(5);
      m_view->SetRulerType(2);
   }
   else
   {
      m_view->SetIntMode(1);
   }
}

void MeasureDlg::OnNewRuler(wxCommandEvent& event)
{
   if (!m_view) return;

   if (m_toolbar->GetToolState(ID_RulerMPBtn))
      m_view->FinishRuler();

   m_toolbar->ToggleTool(ID_LocatorBtn, false);
   m_toolbar->ToggleTool(ID_RulerMPBtn, false);
   m_toolbar->ToggleTool(ID_RulerEditBtn, false);

   if (m_toolbar->GetToolState(ID_RulerBtn))
   {
      m_view->SetIntMode(5);
      m_view->SetRulerType(0);
   }
   else
   {
      m_view->SetIntMode(1);
   }
}

void MeasureDlg::OnNewRulerMP(wxCommandEvent& event)
{
   if (!m_view) return;

   m_toolbar->ToggleTool(ID_LocatorBtn, false);
   m_toolbar->ToggleTool(ID_RulerBtn, false);
   m_toolbar->ToggleTool(ID_RulerEditBtn, false);

   if (m_toolbar->GetToolState(ID_RulerMPBtn))
   {
      m_view->SetIntMode(5);
      m_view->SetRulerType(1);
   }
   else
   {
      m_view->SetIntMode(1);
      m_view->FinishRuler();
   }
}

void MeasureDlg::OnRulerEdit(wxCommandEvent& event)
{
   if (!m_view) return;

   if (m_toolbar->GetToolState(ID_RulerMPBtn))
      m_view->FinishRuler();

   m_toolbar->ToggleTool(ID_LocatorBtn, false);
   m_toolbar->ToggleTool(ID_RulerBtn, false);
   m_toolbar->ToggleTool(ID_RulerMPBtn, false);

   if (m_toolbar->GetToolState(ID_RulerEditBtn))
      m_view->SetIntMode(6);
   else
      m_view->SetIntMode(1);
}

void MeasureDlg::OnDelete(wxCommandEvent& event)
{
   m_rulerlist->DeleteSelection();
}

void MeasureDlg::OnDeleteAll(wxCommandEvent& event)
{
   m_rulerlist->DeleteAll();
}

void MeasureDlg::OnExport(wxCommandEvent& event)
{
   wxFileDialog *fopendlg = new wxFileDialog(
         this, "Export rulers", "", "",
         "Text file (*.txt)|*.txt",
         wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

   int rval = fopendlg->ShowModal();

   if (rval == wxID_OK)
   {
      wxString filename = fopendlg->GetPath();
      m_rulerlist->Export(filename);
   }

   if (fopendlg)
      delete fopendlg;
}

void MeasureDlg::OnIntensityMethodCheck(wxCommandEvent& event)
{
   if (!m_view || !m_view->m_glview)
      return;

   int mode = 0;
   int sender_id = event.GetId();
   switch (sender_id)
   {
   case ID_ViewPlaneRd:
      mode = 0;
      break;
   case ID_MaxIntensityRd:
      mode = 1;
      break;
   case ID_AccIntensityRd:
      mode = 2;
      break;
   }
   m_view->SetPointVolumeMode(mode);
   VRenderFrame* frame = (VRenderFrame*)m_frame;
   if (frame && frame->GetSettingDlg())
      frame->GetSettingDlg()->SetPointVolumeMode(mode);
}

void MeasureDlg::OnUseTransferCheck(wxCommandEvent& event)
{
   if (!m_view || !m_view->m_glview)
      return;

   bool use_transfer = m_use_transfer_chk->GetValue();
   m_view->SetRulerUseTransf(use_transfer);
   VRenderFrame* frame = (VRenderFrame*)m_frame;
   if (frame && frame->GetSettingDlg())
      frame->GetSettingDlg()->SetRulerUseTransf(use_transfer);
}

void MeasureDlg::OnTransientCheck(wxCommandEvent& event)
{
   if (!m_view || !m_view->m_glview)
      return;

   bool val = m_transient_chk->GetValue();
   m_view->SetRulerTimeDep(val);
   VRenderFrame* frame = (VRenderFrame*)m_frame;
   if (frame && frame->GetSettingDlg())
      frame->GetSettingDlg()->SetRulerTimeDep(val);
}
