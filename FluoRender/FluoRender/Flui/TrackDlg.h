/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2024 Scientific Computing and Imaging Institute,
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
#ifndef _TRACEDLG_H_
#define _TRACEDLG_H_

#include <PropPanel.h>
#include <Cell.h>
#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/spinctrl.h>
#include <wx/notebook.h>
#include <wx/tglbtn.h>
#include <vector>

class TrackDlg;
class wxSingleSlider;
class TraceListCtrl : public wxListCtrl
{
public:
	TraceListCtrl(MainFrame *frame,
		wxWindow* parent,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxSize(100, 100),
		long style = wxLC_REPORT);
	~TraceListCtrl();

	void Append(wxString &gtype, unsigned int id, wxColor color,
		int size, double cx, double cy, double cz);
	void DeleteSelection();
	void CopySelection();
	wxString GetText(long item, int col);

	friend class TrackDlg;

private:
	int m_type;//0-current; 1-previous
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
class TrackDlg : public PropPanel
{
public:
	enum
	{
		//menu items
		ID_CopyText,
		ID_Delete
	};

	TrackDlg(MainFrame* frame);
	~TrackDlg();

	virtual void FluoUpdate(const fluo::ValueCollection& vc = {});
	void UpdateList();
	void UpdateTraces();

	void LoadTrackFile(const wxString &file);
	bool SaveTrackFile();
	void SaveTrackFile(const wxString &file);
	void SaveasTrackFile();

	//cell operations
	void CellEraseID();
	void CellReplaceID();
	void CellCombineID();
	void CompDelete();
	//link for external call
	void LinkAddedCells(flrd::CelpList &list);

	//measurement
	void SaveOutputResult(wxString &filename);

private:
	typedef struct
	{
		unsigned int id;
		int total_num;
		int surface_num;
		int contact_num;
	} comp_info;

	wxString m_comp_id;//select
	wxString m_comp_id3;//modify / new id

	//tab control
	wxNotebook *m_notebook;
	//map page
	//load/save trace
	wxTextCtrl* m_load_trace_text;
	wxButton* m_clear_trace_btn;
	wxButton* m_load_trace_btn;
	wxButton* m_save_trace_btn;
	wxButton* m_saveas_trace_btn;
	//auto tracking
	wxButton* m_gen_map_btn;
	wxButton* m_refine_t_btn;
	wxButton* m_refine_all_btn;
	//settings
	wxSpinCtrl* m_map_iter_spin;
	wxSpinCtrl* m_map_size_spin;
	wxToggleButton* m_map_consistent_btn;
	wxToggleButton* m_map_merge_btn;
	wxToggleButton* m_map_split_btn;
	wxSpinCtrlDouble* m_map_similar_spin;
	wxSpinCtrlDouble* m_map_contact_spin;

	//selection page
	//component tools
	wxTextCtrl* m_comp_id_text;
	wxButton* m_comp_id_x_btn;
	wxButton* m_comp_full_btn;
	wxButton* m_comp_exclusive_btn;
	wxButton* m_comp_append_btn;
	wxButton* m_comp_clear_btn;
	wxButton* m_shuffle_btn;
	//cell size filter
	wxSingleSlider* m_cell_size_sldr;
	wxTextCtrl* m_cell_size_text;
	//uncertainty filter
	wxButton* m_comp_uncertain_btn;
	wxSingleSlider* m_comp_uncertain_low_sldr;
	wxTextCtrl* m_comp_uncertain_low_text;

	//link page
	wxTextCtrl* m_comp_id_text2;
	wxButton* m_comp_id_x_btn2;
	wxButton* m_comp_append_btn2;
	wxButton* m_comp_clear_btn2;
	//ID link controls
	wxButton* m_cell_exclusive_link_btn;
	wxButton* m_cell_link_btn;
	wxButton* m_cell_link_all_btn;
	wxButton* m_cell_isolate_btn;
	wxButton* m_cell_unlink_btn;

	//modify page
	//ID edit controls
	wxTextCtrl* m_cell_new_id_text;
	wxButton* m_cell_new_id_x_btn;
	wxButton* m_comp_append_btn3;
	wxButton* m_comp_clear_btn3;
	wxButton* m_cell_new_id_btn;
	wxButton* m_cell_append_id_btn;
	wxButton* m_cell_replace_id_btn;
	wxButton* m_cell_combine_id_btn;
	wxButton* m_cell_separate_id_btn;
	wxButton* m_cell_segment_btn;
	wxSpinCtrl* m_cell_segment_spin;

	//analysis page
	//conversion
	wxButton* m_convert_to_rulers_btn;
	wxButton* m_convert_consistent_btn;
	//analysis
	wxButton* m_analyze_comp_btn;
	wxButton* m_analyze_link_btn;
	wxButton* m_analyze_uncertain_hist_btn;
	wxButton* m_analyze_path_btn;
	wxButton* m_save_result_btn;

	//ghost num
	wxSingleSlider* m_ghost_num_sldr;
	wxTextCtrl* m_ghost_num_text;
	wxCheckBox* m_ghost_show_tail_chk;
	wxCheckBox* m_ghost_show_lead_chk;

	//time controls
	wxButton* m_cell_prev_btn;
	wxButton* m_cell_next_btn;
	//time text
	wxStaticText* m_cell_time_curr_st;
	wxStaticText* m_cell_time_prev_st;

	//list ctrls
	TraceListCtrl *m_trace_list_curr;
	TraceListCtrl *m_trace_list_prev;
	TraceListCtrl* m_active_list;

	//output
	wxTextCtrl* m_stat_text;

private:
	wxWindow* CreateMapPage(wxWindow *parent);
	wxWindow* CreateSelectPage(wxWindow *parent);
	wxWindow* CreateLinkPage(wxWindow *parent);
	wxWindow* CreateModifyPage(wxWindow *parent);
	wxWindow* CreateAnalysisPage(wxWindow *parent);

	//map page
	//load/save trace
	void OnClearTrace(wxCommandEvent& event);
	void OnLoadTrace(wxCommandEvent& event);
	void OnSaveTrace(wxCommandEvent& event);
	void OnSaveasTrace(wxCommandEvent& event);
	//auto tracking
	void OnGenMapBtn(wxCommandEvent& event);
	void OnRefineTBtn(wxCommandEvent& event);
	void OnRefineAllBtn(wxCommandEvent& event);
	//settings
	void OnMapIterSpin(wxSpinEvent& event);
	void OnMapIterText(wxCommandEvent& event);
	void OnMapSizeSpin(wxSpinEvent& event);
	void OnMapSizeText(wxCommandEvent& event);
	void OnMapConsistentBtn(wxCommandEvent& event);
	void OnMapMergeBtn(wxCommandEvent& event);
	void OnMapSplitBtn(wxCommandEvent& event);
	void OnMapSimilarSpin(wxSpinDoubleEvent& event);
	void OnMapSimilarText(wxCommandEvent& event);
	void OnMapContactSpin(wxSpinDoubleEvent& event);
	void OnMapContactText(wxCommandEvent& event);
	//selection page
	//component tools
	void OnCompIDText(wxCommandEvent& event);
	void OnCompIDXBtn(wxCommandEvent& event);
	void OnCompFull(wxCommandEvent& event);
	void OnCompExclusive(wxCommandEvent& event);
	void OnCompAppend(wxCommandEvent& event);
	void OnCompClear(wxCommandEvent& event);
	void OnShuffle(wxCommandEvent& event);
	//cell size filter
	void OnCellSizeChange(wxScrollEvent& event);
	void OnCellSizeText(wxCommandEvent& event);
	//uncertainty filter
	void OnCompUncertainBtn(wxCommandEvent& event);
	void OnCompUncertainLowChange(wxScrollEvent& event);
	void OnCompUncertainLowText(wxCommandEvent& event);
	//link page
	//ID lnik controls
	void OnCompId2Text(wxCommandEvent& event);
	void OnCompId2XBtn(wxCommandEvent& event);
	void OnCellExclusiveLink(wxCommandEvent& event);
	void OnCellLink(wxCommandEvent& event);
	void OnCellLinkAll(wxCommandEvent& event);
	void OnCellIsolate(wxCommandEvent& event);
	void OnCellUnlink(wxCommandEvent& event);
	//modify page
	//ID edit controls
	void OnCellNewIDText(wxCommandEvent& event);
	void OnCellNewIDX(wxCommandEvent& event);
	void OnCellNewID(wxCommandEvent& event);
	void OnCellAppendID(wxCommandEvent& event);
	void OnCellReplaceID(wxCommandEvent& event);
	void OnCellCombineID(wxCommandEvent& event);
	void OnCellSeparateID(wxCommandEvent& event);
	void OnCellSegment(wxCommandEvent& event);
	void OnCellSegSpin(wxSpinEvent& event);
	void OnCellSegText(wxCommandEvent& event);
	//analysis page
	//conversion
	void OnConvertToRulers(wxCommandEvent& event);
	void OnConvertConsistent(wxCommandEvent& event);
	//analysis
	void OnAnalyzeComp(wxCommandEvent& event);
	void OnAnalyzeLink(wxCommandEvent& event);
	void OnAnalyzeUncertainHist(wxCommandEvent& event);
	void OnAnalyzePath(wxCommandEvent& event);
	void OnSaveResult(wxCommandEvent& event);

	//ghost number
	void OnGhostNumChange(wxScrollEvent& event);
	void OnGhostNumText(wxCommandEvent& event);
	void OnGhostShowTail(wxCommandEvent& event);
	void OnGhostShowLead(wxCommandEvent& event);
	//time controls
	void OnCellPrev(wxCommandEvent& event);
	void OnCellNext(wxCommandEvent& event);

	//list
	void AddLabel(long item, TraceListCtrl* trace_list_ctrl, flrd::CelpList& list);
	void OnSelectionChanged(wxListEvent& event);
	void OnContextMenu(wxContextMenuEvent& event);
	void OnMenuItem(wxCommandEvent& event);
	void OnKeyDown(wxKeyEvent& event);
};

#endif//_TRACEDLG_H_
