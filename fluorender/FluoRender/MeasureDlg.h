#include <wx/wx.h>
#include <wx/listctrl.h>

#ifndef _MEASUREDLG_H_
#define _MEASUREDLG_H_

using namespace std;

class VRenderView;

class RulerListCtrl : public wxListCtrl
{
   public:
      RulerListCtrl(wxWindow *frame,
            wxWindow* parent,
            wxWindowID id,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            long style=wxLC_REPORT|wxLC_SINGLE_SEL);
      ~RulerListCtrl();

      void Append(wxString name, double length, wxString &unit,
            double angle, wxString &points, bool time_dep, int time,
            wxString extra);
      void Update(VRenderView* vrv=0);

      void DeleteSelection();
      void DeleteAll(bool cur_time=true);

      void Export(wxString filename);

      friend class MeasureDlg;

   private:
      wxWindow* m_frame;
      VRenderView *m_view;
      wxImageList *m_images;

   private:
      void OnKeyDown(wxKeyEvent& event);

      DECLARE_EVENT_TABLE()
   protected: //Possible TODO
         wxSize GetSizeAvailableForScrollTarget(const wxSize& size) {
            return size - GetEffectiveMinSize();
         }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
class MeasureDlg : public wxPanel
{
   public:
      enum
      {
         ID_LocatorBtn = wxID_HIGHEST+2101,
         ID_RulerBtn,
         ID_RulerMPBtn,
         ID_RulerEditBtn,
         ID_DeleteBtn,
         ID_DeleteAllBtn,
         ID_ExportBtn,
         ID_ViewPlaneRd,
         ID_MaxIntensityRd,
         ID_AccIntensityRd,
         ID_UseTransferChk,
         ID_TransientChk
      };

      MeasureDlg(wxWindow* frame,
            wxWindow* parent);
      ~MeasureDlg();

      void GetSettings(VRenderView* vrv);
      VRenderView* GetView();
      void UpdateList();

   private:
      wxWindow* m_frame;
      //current view
      VRenderView* m_view;

      //list ctrl
      RulerListCtrl *m_rulerlist;
      //tool bar
      wxToolBar *m_toolbar;
      //options
      wxRadioButton *m_view_plane_rd;
      wxRadioButton *m_max_intensity_rd;
      wxRadioButton *m_acc_intensity_rd;
      wxCheckBox *m_use_transfer_chk;
      wxCheckBox *m_transient_chk;

   private:
      void OnNewLocator(wxCommandEvent& event);
      void OnNewRuler(wxCommandEvent& event);
      void OnNewRulerMP(wxCommandEvent& event);
      void OnRulerEdit(wxCommandEvent& event);
      void OnDelete(wxCommandEvent& event);
      void OnDeleteAll(wxCommandEvent& event);
      void OnExport(wxCommandEvent& event);
      void OnIntensityMethodCheck(wxCommandEvent& event);
      void OnUseTransferCheck(wxCommandEvent& event);
      void OnTransientCheck(wxCommandEvent& event);

      DECLARE_EVENT_TABLE();
};

#endif//_MEASUREDLG_H_
