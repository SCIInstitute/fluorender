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
#include <wx/wx.h>
#include <wx/listctrl.h>

#ifndef _MEASUREDLG_H_
#define _MEASUREDLG_H_

using namespace std;

class VRenderView;

class RulerListCtrl : public wxListCtrl
{
   enum
   {
      ID_NameText = wxID_HIGHEST+2351
   };

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
      void UpdateRulers(VRenderView* vrv=0);

	  int GetCurrSelection();
      void DeleteSelection();
      void DeleteAll(bool cur_time=true);

      void Export(wxString filename);

      wxString GetText(long item, int col);
      void SetText(long item, int col, wxString &str);

      friend class MeasureDlg;

   private:
      //wxWindow* m_frame;
      VRenderView *m_view;
      wxImageList *m_images;
      wxTextCtrl *m_name_text;
      long m_editing_item;

   private:
      void OnKeyDown(wxKeyEvent& event);
      void OnSelection(wxListEvent &event);
      void OnEndSelection(wxListEvent &event);
      void EndEdit();
      void OnNameText(wxCommandEvent& event);

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
		 ID_ProbeBtn,
         ID_RulerBtn,
         ID_RulerMPBtn,
         ID_RulerEditBtn,
		 ID_ProfileBtn,
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
	  void OnNewProbe(wxCommandEvent& event);
      void OnNewRuler(wxCommandEvent& event);
      void OnNewRulerMP(wxCommandEvent& event);
      void OnRulerEdit(wxCommandEvent& event);
	  void OnProfile(wxCommandEvent& event);
      void OnDelete(wxCommandEvent& event);
      void OnDeleteAll(wxCommandEvent& event);
      void OnExport(wxCommandEvent& event);
      void OnIntensityMethodCheck(wxCommandEvent& event);
      void OnUseTransferCheck(wxCommandEvent& event);
      void OnTransientCheck(wxCommandEvent& event);

      DECLARE_EVENT_TABLE();
};

#endif//_MEASUREDLG_H_
