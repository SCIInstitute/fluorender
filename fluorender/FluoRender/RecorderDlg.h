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
#include <vector>

#ifndef _RECORDERDLG_H_
#define _RECORDERDLG_H_

using namespace std;

class VRenderView;

class KeyListCtrl : public wxListCtrl
{
   enum
   {
      ID_FrameText = wxID_HIGHEST+1751,
      ID_DurationText,
      ID_InterpoCmb,
      ID_DescriptionText
   };

   public:
   KeyListCtrl(wxWindow *frame,
         wxWindow* parent,
         wxWindowID id,
         const wxPoint& pos = wxDefaultPosition,
         const wxSize& size = wxDefaultSize,
         long style=wxLC_REPORT|wxLC_SINGLE_SEL);
   ~KeyListCtrl();

   void Append(int id, int time, int duration, int interp, string &description);
   void DeleteSel();
   void DeleteAll();
   wxString GetText(long item, int col);
   void SetText(long item, int col, wxString &str);
   void Update();
   void UpdateText();

   friend class RecorderDlg;

   private:
   wxWindow* m_frame;
   wxImageList *m_images;

   wxTextCtrl *m_frame_text;
   wxTextCtrl *m_duration_text;
   wxComboBox *m_interpolation_cmb;
   wxTextCtrl *m_description_text;

   long m_editing_item;
   long m_dragging_to_item;

   private:
   void EndEdit(bool update=true);

   private:
   void OnAct(wxListEvent &event);
   void OnSelection(wxListEvent &event);
   void OnEndSelection(wxListEvent &event);
   void OnFrameText(wxCommandEvent& event);
   void OnDurationText(wxCommandEvent& event);
   void OnInterpoCmb(wxCommandEvent& event);
   void OnDescritionText(wxCommandEvent& event);
   void OnBeginDrag(wxListEvent& event);
   void OnDragging(wxMouseEvent& event);
   void OnEndDrag(wxMouseEvent& event);

   void OnKeyDown(wxKeyEvent& event);
   void OnKeyUp(wxKeyEvent& event);
   void OnScroll(wxScrollWinEvent& event);
   void OnScroll(wxMouseEvent& event);

   DECLARE_EVENT_TABLE()
   protected: //Possible TODO
      wxSize GetSizeAvailableForScrollTarget(const wxSize& size) { 
         return size - GetEffectiveMinSize(); 
      }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
class RecorderDlg : public wxPanel
{
   public:
      enum
      {
         ID_SetKeyBtn = wxID_HIGHEST+1701,
         ID_DurationText,
         ID_InterpolationCmb,
         ID_DelKeyBtn,
         ID_InsKeyBtn,
         ID_DelAllBtn,
         ID_AutoKeyCmb,
         ID_AutoKeyBtn
      };

      RecorderDlg(wxWindow* frame,
            wxWindow* parent);
      ~RecorderDlg();

      void GetSettings(VRenderView* vrv);
      VRenderView* GetView()
      { return m_view; }
      void UpdateList()
      { m_keylist->Update(); }
      void SetSelection(int index);
	  void Save() { 
		wxCommandEvent e;
		OnPlay(e); 
	  }
	  void Rewind() { 
		wxCommandEvent e;
		OnReset(e); 
	  }
	  void Stop() { 
		wxCommandEvent e;
		OnStop(e); 
	  }
	  void Play() { 
		wxCommandEvent e;
		OnPreview(e); 
	  }

      void AutoKeyChanComb(int comb);

   private:
      wxWindow* m_frame;
      //current view
      VRenderView* m_view;

      //automatic keys
      wxComboBox *m_auto_key_cmb;
      //generate
      //wxButton *m_auto_key_btn;

      //list ctrl
      KeyListCtrl *m_keylist;

      //default duration
      wxTextCtrl *m_duration_text;
      //default interpolation
      wxComboBox *m_interpolation_cmb;
      //set key
      wxButton *m_set_key_btn;
      //insert key
      //wxButton *m_insert_key_btn;
      //delete key
      wxButton *m_del_key_btn;
      //delete all keys
      wxButton *m_del_all_btn;

   private:
      //insert/append key
      void InsertKey(int index, double duration, int interpolation);
      bool MoveOne(vector<bool>& chan_mask, int lv);
      bool GetMask(vector<bool>& chan_mask);

      void OnCh1Check(wxCommandEvent &event);
      static wxWindow* CreateExtraCaptureControl(wxWindow* parent);
      void OnAutoKey(wxCommandEvent &event);
      void OnSetKey(wxCommandEvent &event);
      void OnInsKey(wxCommandEvent &event);
      void OnDelKey(wxCommandEvent &event);
      void OnDelAll(wxCommandEvent &event);
      void OnPreview(wxCommandEvent &event);
      void OnReset(wxCommandEvent &event);
      void OnPlay(wxCommandEvent &event);
      void OnStop(wxCommandEvent &event);

      DECLARE_EVENT_TABLE();
};

#endif//_RECORDERDLG_H_
