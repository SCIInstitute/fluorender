#include <wx/wx.h>
#include <wx/listctrl.h>

#ifndef _TRACEDLG_H_
#define _TRACEDLG_H_

using namespace std;

class VRenderView;

class TraceListCtrl : public wxListCtrl
{
	enum
	{
		Menu_ExportText = wxID_HIGHEST+2251
	};

public:
	TraceListCtrl(wxWindow *frame,
		wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxLC_REPORT|wxLC_SINGLE_SEL);
	~TraceListCtrl();

	void Append(unsigned int id, wxColor color, int size, int stime, int etime);
	void Update(VRenderView* vrv=0);
	void DeleteSelection();
	void DeleteAll();

	//menu operations
	void ExportSelection();

	friend class TraceDlg;

private:
	wxWindow* m_frame;
	VRenderView *m_view;

private:
	void OnKeyDown(wxKeyEvent& event);
	void OnContextMenu(wxContextMenuEvent &event);
	void OnExportSelection(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
class TraceDlg : public wxPanel
{
public:
	enum
	{
		ID_LoadTraceText = wxID_HIGHEST+2201,
		ID_LoadTraceBtn,
		ID_GhostNumSldr,
		ID_GhostNumText,
		ID_GhostUpdateBtn
	};

	TraceDlg(wxWindow* frame,
		wxWindow* parent);
	~TraceDlg();

	void GetSettings(VRenderView* vrv);
	VRenderView* GetView();
	void UpdateList();

private:
	wxWindow* m_frame;
	//current view
	VRenderView* m_view;

	//list ctrl
	TraceListCtrl *m_tracelist;

	//load trace
	wxTextCtrl* m_load_trace_text;
	wxButton* m_load_trace_btn;
	//ghost num
	wxSlider* m_ghost_num_sldr;
	wxTextCtrl* m_ghost_num_text;
	wxButton* m_ghost_update_btn;

private:
	void OnLoadTrace(wxCommandEvent& event);
	void OnGhostNumChange(wxScrollEvent &event);
	void OnGhostNumText(wxCommandEvent &event);
	void OnGhostUpdate(wxCommandEvent& event);

	DECLARE_EVENT_TABLE();
};

#endif//_TRACEDLG_H_
