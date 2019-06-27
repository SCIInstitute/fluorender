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
#include "Main.h"
#include "DataManager.h"
#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/spinctrl.h>
#include <wx/notebook.h>
#include "teem/Nrrd/nrrd.h"
#include <vector>

#ifndef _TRACEDLG_H_
#define _TRACEDLG_H_

using namespace std;

class VRenderView;

class TraceListCtrl : public wxListCtrl
{
	enum
	{
		Menu_CopyText = ID_TRACE1,
		Menu_Delete
	};

public:
	TraceListCtrl(wxWindow *frame,
		wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxSize(100, 100),
		long style = wxLC_REPORT);
	~TraceListCtrl();

	void Append(wxString &gtype, unsigned int id, wxColor color,
		int size, double cx, double cy, double cz);
	void UpdateTraces(VRenderView* vrv=0);
	void DeleteSelection();
	wxString GetText(long item, int col);

	friend class TraceDlg;

private:
	VRenderView *m_view;
	int m_type;//0-current; 1-previous

private:
	static bool sort_cells(const FL::pCell c1, const FL::pCell c2)
	{
		unsigned int vid1 = c1->GetVertexId();
		unsigned int vid2 = c2->GetVertexId();
		if (vid1 == vid2)
			return c1->GetSizeUi() > c2->GetSizeUi();
		else
			return vid1 < vid2;
	};

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
		ID_LoadTraceBtn,
		ID_SaveTraceBtn,
		ID_SaveasTraceBtn,
		//auto tracking
		ID_GenMapPrg,
		ID_GenMapBtn,
		ID_RefineTBtn,
		ID_RefineAllBtn,
		ID_MapIterSpin,
		ID_MapSizeSpin,
		ID_MapSimlrSpin,
		ID_MapCntctSpin,
		ID_MapMergeChk,
		ID_MapSplitChk,
		//selection page
		//component tools
		ID_CompIDText,
		ID_CompIDXBtn,
		ID_CompFullBtn,
		ID_CompExclusiveBtn,
		ID_CompAppendBtn,
		ID_CompClearBtn,
		//comp size filter
		ID_CellSizeSldr,
		ID_CellSizeText,
		//uncertainty filter
		ID_CompUncertainBtn,
		ID_CompUncertainLowSldr,
		ID_CompUncertainLowText,
		ID_CompUncertainHiSldr,
		ID_CompUncertainHiText,
		//link page
		ID_CompIDText2,
		ID_CellExclusiveLinkBtn,
		ID_CellLinkBtn,
		ID_CellLinkAllBtn,
		ID_CellIsolateBtn,
		ID_CellUnlinkBtn,
		//assist
		ID_ManualAssistCheck,
		//modify page
		//ID edit controls
		ID_CellNewIDText,
		ID_CellNewIDXBtn,
		ID_CompAppend2Btn,
		ID_AutoIDChk,
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

	TraceDlg(wxWindow* frame,
		wxWindow* parent);
	~TraceDlg();

	void GetSettings(VRenderView* vrv);
	VRenderView* GetView();
	void UpdateList();
	void SetCellSize(int size);

	//manual assist
	bool GetManualAssist() {return m_manual_assist;}
	//auto id
	bool GetAutoID() { return m_auto_id; }
	//cell operations
	void CellUpdate();
	void CellFull();
	void CellLink(bool exclusive);
	void CellNewID(bool append);
	void CellEraseID();
	void CompDelete();
	void CompClear();

	//measurement
	void SaveOutputResult(wxString &filename);

	//automatic tracking
	void GenMap();
	void RefineMap(int t=-1);

private:
	typedef struct
	{
		unsigned int id;
		int total_num;
		int surface_num;
		int contact_num;
	} comp_info;

	typedef boost::unordered_map<unsigned int, unsigned int> CellMap;
	typedef boost::unordered_map<unsigned int, unsigned int>::iterator CellMapIter;

	wxWindow* m_frame;
	//current view
	VRenderView* m_view;
	//tab control
	wxNotebook *m_notebook;

	//temporary mask
	//Nrrd* m_mask;

	//time sequence setting
	int m_cur_time;
	int m_prv_time;

	//enable manual assist
	bool m_manual_assist;
	//auto id
	bool m_auto_id;

	//cluster number
	int m_clnum;

	//map page
	//load/save trace
	wxTextCtrl* m_load_trace_text;
	wxButton* m_load_trace_btn;
	wxButton* m_save_trace_btn;
	wxButton* m_saveas_trace_btn;
	//auto tracking
	wxGauge* m_gen_map_prg;
	wxButton* m_gen_map_btn;
	wxButton* m_refine_t_btn;
	wxButton* m_refine_all_btn;
	//settings
	wxSpinCtrl* m_map_iter_spin;
	wxSpinCtrl* m_map_size_spin;
	wxSpinCtrlDouble* m_map_simlr_spin;
	wxSpinCtrlDouble* m_map_cntct_spin;
	wxCheckBox* m_map_merge_chk;
	wxCheckBox* m_map_split_chk;

	//selection page
	//component tools
	wxTextCtrl* m_comp_id_text;
	wxButton* m_comp_id_x_btn;
	wxButton* m_comp_full_btn;
	wxButton* m_comp_exclusive_btn;
	wxButton* m_comp_append_btn;
	wxButton* m_comp_clear_btn;
	//cell size filter
	wxSlider* m_cell_size_sldr;
	wxTextCtrl* m_cell_size_text;
	//uncertainty filter
	wxButton* m_comp_uncertain_btn;
	wxSlider* m_comp_uncertain_low_sldr;
	wxTextCtrl* m_comp_uncertain_low_text;
	wxSlider* m_comp_uncertain_hi_sldr;
	wxTextCtrl* m_comp_uncertain_hi_text;

	//link page
	wxTextCtrl* m_comp_id_text2;
	//same append button from selection page
	//assist
	wxToolBar* m_manual_assist_check;
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
	wxToolBar* m_auto_id_chk;
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
	void AddLabel(long item, TraceListCtrl* trace_list_ctrl, FL::CellList &list);
	wxWindow* CreateMapPage(wxWindow *parent);
	wxWindow* CreateSelectPage(wxWindow *parent);
	wxWindow* CreateLinkPage(wxWindow *parent);
	wxWindow* CreateModifyPage(wxWindow *parent);
	wxWindow* CreateAnalysisPage(wxWindow *parent);

	unsigned int GetMappedID(unsigned int id, unsigned int* data_label1,
		unsigned int* data_label2, unsigned long long size);
	//tests
	void Test1();
	void Test2(int type);

	//read/delete volume cache from file
	void ReadVolCache(FL::VolCache& vol_cache);
	void DelVolCache(FL::VolCache& vol_cache);

private:
	//map page
	//load/save trace
	void OnLoadTrace(wxCommandEvent& event);
	void OnSaveTrace(wxCommandEvent& event);
	void OnSaveasTrace(wxCommandEvent& event);
	//auto tracking
	void OnGenMapBtn(wxCommandEvent &event);
	void OnRefineTBtn(wxCommandEvent &event);
	void OnRefineAllBtn(wxCommandEvent &event);
	//selection page
	//component tools
	void OnCompIDText(wxCommandEvent &event);
	void OnCompIDXBtn(wxCommandEvent &event);
	void OnCompFull(wxCommandEvent &event);
	void OnCompExclusive(wxCommandEvent &event);
	void OnCompAppend(wxCommandEvent &event);
	void OnCompClear(wxCommandEvent &event);
	//cell size filter
	void OnCellSizeChange(wxScrollEvent &event);
	void OnCellSizeText(wxCommandEvent &event);
	//uncertainty filter
	void OnCompUncertainBtn(wxCommandEvent &event);
	void OnCompUncertainLowChange(wxScrollEvent &event);
	void OnCompUncertainLowText(wxCommandEvent &event);
	void OnCompUncertainHiChange(wxScrollEvent &event);
	void OnCompUncertainHiText(wxCommandEvent &event);
	//link page
	//ID lnik controls
	void OnCellExclusiveLink(wxCommandEvent& event);
	void OnCellLink(wxCommandEvent& event);
	void OnCellLinkAll(wxCommandEvent& event);
	void OnCellIsolate(wxCommandEvent& event);
	void OnCellUnlink(wxCommandEvent& event);
	//manual tracking assistant
	void OnManualAssistCheck(wxCommandEvent &event);
	//modify page
	//ID edit controls
	void OnCellNewIDText(wxCommandEvent &event);
	void OnCellNewIDX(wxCommandEvent& event);
	void OnAutoIDChk(wxCommandEvent& event);
	void OnCellNewID(wxCommandEvent& event);
	void OnCellAppendID(wxCommandEvent& event);
	void OnCellReplaceID(wxCommandEvent& event);
	void OnCellCombineID(wxCommandEvent& event);
	void OnCellSeparateID(wxCommandEvent& event);
	void OnCellSegment(wxCommandEvent& event);
	void OnCellSegText(wxSpinEvent& event);
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
