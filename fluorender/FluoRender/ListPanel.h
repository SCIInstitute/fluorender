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
   wxWindow* m_frame;
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
