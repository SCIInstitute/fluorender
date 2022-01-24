/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2018 Scientific Computing and Imaging Institute,
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

#include "Main.h"
#include <Tracking/Cell.h>
#include <Tracking/VolCache.h>
#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/spinctrl.h>
#include <wx/notebook.h>
#include <wx/tglbtn.h>
#include <vector>

using namespace std;

class VRenderFrame;
class RenderCanvas;

class TraceListCtrl : public wxListCtrl
{
	enum
	{
		Menu_CopyText = ID_TRACE1,
		Menu_Delete
	};

public:
	TraceListCtrl(VRenderFrame *frame,
		wxWindow* parent,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxSize(100, 100),
		long style = wxLC_REPORT);
	~TraceListCtrl();

	void Append(wxString &gtype, unsigned int id, wxColor color,
		int size, double cx, double cy, double cz);
	void UpdateTraces(RenderCanvas* vrv=0);
	void DeleteSelection();
	wxString GetText(long item, int col);

	friend class TraceDlg;

private:
	RenderCanvas *m_view;
	int m_type;//0-current; 1-previous

private:
	void OnKeyDown(wxKeyEvent& event);
	void OnContextMenu(wxContextMenuEvent &event);
	void OnCopySelection(wxCommandEvent& event);
	void OnDeleteSelection(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
DECLARE_APP(VRenderApp)
class TraceDlg : public wxPanel
{
public:
	enum
	{
		//map page
		//load/save trace
		ID_LoadTraceText = ID_TRACE2,
		ID_ClearTraceBtn,
		ID_LoadTraceBtn,
		ID_SaveTraceBtn,
		ID_SaveasTraceBtn,
		//auto tracking
		ID_GenMapPrg,
		ID_GenMapBtn,
		ID_RefineTBtn,
		ID_RefineAllBtn,
		//settings
		ID_MapIterSpin,
		ID_MapSizeSpin,
		ID_MapConsistentBtn,
		ID_MapMergeBtn,
		ID_MapSplitBtn,
		ID_MapSimilarSpin,
		ID_MapContactSpin,
		//selection page
		//component tools
		ID_CompIDText,
		ID_CompIDXBtn,
		ID_CompFullBtn,
		ID_CompExclusiveBtn,
		ID_CompAppendBtn,
		ID_CompClearBtn,
		ID_ShuffleBtn,
		//comp size filter
		ID_CellSizeSldr,
		ID_CellSizeText,
		//uncertainty filter
		ID_CompUncertainBtn,
		ID_CompUncertainLowSldr,
		ID_CompUncertainLowText,
		//link page
		ID_CompIDText2,
		ID_CellExclusiveLinkBtn,
		ID_CellLinkBtn,
		ID_CellLinkAllBtn,
		ID_CellIsolateBtn,
		ID_CellUnlinkBtn,
		//modify page
		//ID edit controls
		ID_CellNewIDText,
		ID_CellNewIDXBtn,
		ID_CompAppend2Btn,
		ID_CellNewIDBtn,
		ID_CellAppendIDBtn,
		ID_CellReplaceIDBtn,
		ID_CellCombineIDBtn,
		ID_CellSeparateBtn,
		ID_CellSegBtn,
		ID_CellSegText,
		//analysis page
		//conversion
		ID_ConvertToRulersBtn,
		ID_ConvertConsistentBtn,
		//analysis
		ID_AnalyzeCompBtn,
		ID_AnalyzeLinkBtn,
		ID_AnalyzeUncertainHistBtn,
		ID_AnalyzePathBtn,
		ID_SaveResultBtn,
		//ghost num
		ID_GhostNumSldr,
		ID_GhostNumText,
		ID_GhostShowTailChk,
		ID_GhostShowLeadChk,
		//time controls
		ID_CellPrevBtn,
		ID_CellNextBtn,
		//output
		ID_StatText
	};

	TraceDlg(VRenderFrame* frame);
	~TraceDlg();

	void GetSettings(RenderCanvas* vrv);
	RenderCanvas* GetView();
	void UpdateList();
	void SetCellSize(int size);

	//cell operations
	void CellUpdate();
	void CellFull();
	void CellLink(bool exclusive);
	void CellNewID(bool append);
	void CellEraseID();
	void CellReplaceID();
	void CellCombineID();
	void CompDelete();
	void CompClear();
	//uncertain filtering
	void UncertainFilter(bool input = false);
	//link for external call
	void LinkAddedCells(flrd::CelpList &list);

	//measurement
	void SaveOutputResult(wxString &filename);

	//automatic tracking
	void GenMap();
	void RefineMap(int t=-1, bool erase_v=true);

	//track map file
	int GetTrackFileExist(bool save);//0:no trace group; 1:trace groups exists not saved; 2:saved
	wxString GetTrackFile();
	void LoadTrackFile(wxString &file);
	void SaveTrackFile(wxString &file);

private:
	typedef struct
	{
		unsigned int id;
		int total_num;
		int surface_num;
		int contact_num;
	} comp_info;

	VRenderFrame* m_frame;
	//current view
	RenderCanvas* m_view;
	//tab control
	wxNotebook *m_notebook;

	//time sequence setting
	int m_cur_time;
	int m_prv_time;

	//cluster number
	int m_clnum;

	//settings
	size_t m_iter_num;
	double m_size_thresh;
	bool m_consistent_color;
	bool m_try_merge;
	bool m_try_split;
	double m_similarity;
	double m_contact_factor;

	wxString m_track_file;

	//ids
	unsigned int m_cell_new_id;
	bool m_cell_new_id_empty;

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
	wxSlider* m_cell_size_sldr;
	wxTextCtrl* m_cell_size_text;
	//uncertainty filter
	wxButton* m_comp_uncertain_btn;
	wxSlider* m_comp_uncertain_low_sldr;
	wxTextCtrl* m_comp_uncertain_low_text;

	//link page
	wxTextCtrl* m_comp_id_text2;
	//same append button from selection page
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
	wxButton* m_comp_append2_btn;
	wxButton* m_cell_new_id_btn;
	wxButton* m_cell_append_id_btn;
	wxButton* m_cell_replace_id_btn;
	wxButton* m_cell_combine_id_btn;
	wxButton* m_cell_separate_id_btn;
	wxButton* m_cell_segment_btn;
	wxSpinCtrl* m_cell_segment_text;

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
	wxSlider* m_ghost_num_sldr;
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

	//output
	wxTextCtrl* m_stat_text;

private:
	void AddLabel(long item, TraceListCtrl* trace_list_ctrl, flrd::CelpList &list);
	wxWindow* CreateMapPage(wxWindow *parent);
	wxWindow* CreateSelectPage(wxWindow *parent);
	wxWindow* CreateLinkPage(wxWindow *parent);
	wxWindow* CreateModifyPage(wxWindow *parent);
	wxWindow* CreateAnalysisPage(wxWindow *parent);

	//read/delete volume cache from file
	void ReadVolCache(flrd::VolCache& vol_cache);
	void DelVolCache(flrd::VolCache& vol_cache);

private:
	//map page
	//load/save trace
	void OnClearTrace(wxCommandEvent& event);
	void OnLoadTrace(wxCommandEvent& event);
	void OnSaveTrace(wxCommandEvent& event);
	void OnSaveasTrace(wxCommandEvent& event);
	//auto tracking
	void OnGenMapBtn(wxCommandEvent &event);
	void OnRefineTBtn(wxCommandEvent &event);
	void OnRefineAllBtn(wxCommandEvent &event);
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
	void OnCompIDText(wxCommandEvent &event);
	void OnCompIDXBtn(wxCommandEvent &event);
	void OnCompFull(wxCommandEvent &event);
	void OnCompExclusive(wxCommandEvent &event);
	void OnCompAppend(wxCommandEvent &event);
	void OnCompClear(wxCommandEvent &event);
	void OnShuffle(wxCommandEvent &event);
	//cell size filter
	void OnCellSizeChange(wxScrollEvent &event);
	void OnCellSizeText(wxCommandEvent &event);
	//uncertainty filter
	void OnCompUncertainBtn(wxCommandEvent &event);
	void OnCompUncertainLowChange(wxScrollEvent &event);
	void OnCompUncertainLowText(wxCommandEvent &event);
	//link page
	//ID lnik controls
	void OnCellExclusiveLink(wxCommandEvent& event);
	void OnCellLink(wxCommandEvent& event);
	void OnCellLinkAll(wxCommandEvent& event);
	void OnCellIsolate(wxCommandEvent& event);
	void OnCellUnlink(wxCommandEvent& event);
	//modify page
	//ID edit controls
	void OnCellNewIDText(wxCommandEvent &event);
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
	void OnConvertToRulers(wxCommandEvent &event);
	void OnConvertConsistent(wxCommandEvent &event);
	//analysis
	void OnAnalyzeComp(wxCommandEvent &event);
	void OnAnalyzeLink(wxCommandEvent &event);
	void OnAnalyzeUncertainHist(wxCommandEvent &event);
	void OnAnalyzePath(wxCommandEvent &event);
	void OnSaveResult(wxCommandEvent &event);
	//ghost number
	void OnGhostNumChange(wxScrollEvent &event);
	void OnGhostNumText(wxCommandEvent &event);
	void OnGhostShowTail(wxCommandEvent &event);
	void OnGhostShowLead(wxCommandEvent &event);
	//time controls
	void OnCellPrev(wxCommandEvent& event);
	void OnCellNext(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()
};

#endif//_TRACEDLG_H_
