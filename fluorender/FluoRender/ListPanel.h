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

#ifndef _LISTPANEL_H_
#define _LISTPANEL_H_

class DataListCtrl: public wxListCtrl
{
   enum
   {
      Menu_AddTo = wxID_HIGHEST+201,
      Menu_Prop,
      Menu_Del,
      Menu_Rename,
      Menu_Save,
      Menu_Bake,
      ID_RenameText,
      Menu_View_start
   };

   public:
   DataListCtrl(wxWindow *frame,
         wxWindow* parent,
         wxWindowID id,
         const wxPoint& pos=wxDefaultPosition,
         const wxSize& size=wxDefaultSize,
         long style=wxLC_REPORT|wxLC_SINGLE_SEL);
   ~DataListCtrl();

   void Append(int type, wxString name, wxString path);
   wxString GetText(long item, int col);
   void SetText(long item, int col, wxString &str);
   void SetSelection(int type, wxString &name);

   friend class ListPanel;

   private:
   wxWindow* m_frame;

   wxTextCtrl *m_rename_text;

   private:
   void DeleteSelection();
   void DeleteAll();
   void AddToView(int menu_index, long item);
   void EndEdit(bool update=true);

   //
   void OnContextMenu(wxContextMenuEvent &event);
   void OnAddToView(wxCommandEvent& event);
   void OnDelete(wxCommandEvent& event);
   void OnRename(wxCommandEvent& event);
   void OnCh1Check(wxCommandEvent &event);
   static wxWindow* CreateExtraControl(wxWindow* parent);
   void OnSave(wxCommandEvent& event);
   void OnBake(wxCommandEvent& event);

   void OnSelect(wxListEvent &event);
   void OnAct(wxListEvent &event);
   void OnKeyDown(wxKeyEvent& event);
   void OnKeyUp(wxKeyEvent& event);
   void OnMouse(wxMouseEvent& event);
   void OnEndEditName(wxCommandEvent& event);
   void OnScroll(wxScrollWinEvent& event);
   void OnScroll(wxMouseEvent& event);

   DECLARE_EVENT_TABLE()
   protected: //Possible TODO
      wxSize GetSizeAvailableForScrollTarget(const wxSize& size) {
         return size - GetEffectiveMinSize();
      }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////

class ListPanel : public wxPanel
{
   enum
   {
      ID_AddToView = wxID_HIGHEST+251,
      ID_Rename,
      ID_Save,
      ID_Bake,
      ID_Delete,
      ID_DeleteAll
   };
   public:
   ListPanel(wxWindow* frame,
         wxWindow* parent,
         wxWindowID id,
         const wxPoint& pos = wxDefaultPosition,
         const wxSize& size = wxDefaultSize,
         long style = 0,
         const wxString& name="ListPanel");
   ~ListPanel();

   void Append(int type, wxString name, wxString path);
   wxString GetText(long item, int col);
   void SetText(long item, int col, wxString &str);
   void DeleteAllItems();
   void SetSelection(int type, wxString &name);

   private:
   //wxWindow* m_frame;
   DataListCtrl *m_datalist;
   wxToolBar *m_toolbar;

   void OnAddToView(wxCommandEvent& event);
   void OnRename(wxCommandEvent& event);
   void OnSave(wxCommandEvent& event);
   void OnBake(wxCommandEvent& event);
   void OnDelete(wxCommandEvent& event);
   void OnDeleteAll(wxCommandEvent& event);

   DECLARE_EVENT_TABLE()
};

#endif//_LISTPANEL_H_
