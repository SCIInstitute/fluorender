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
#include "Main.h"
#include "DataManager.h"
#include <wx/wx.h>
#include <wx/listctrl.h>
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
		Menu_CopyText = wxID_HIGHEST+2251,
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

	void Append(unsigned int id, wxColor color,
		int size, double cx, double cy, double cz);
	void UpdateTraces(VRenderView* vrv=0);
	void DeleteSelection();
	wxString GetText(long item, int col);

	friend class TraceDlg;

private:
	VRenderView *m_view;
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
		ID_LoadTraceText = wxID_HIGHEST+2201,
		ID_LoadTraceBtn,
		ID_SaveTraceBtn,
		ID_SaveasTraceBtn,
		ID_GhostNumSldr,
		ID_GhostNumText,
		ID_GhostShowTailChk,
		ID_GhostShowLeadChk,
		ID_ManualAssistCheck,
		ID_AddLabelBtn,
		ID_AnalyzeBtn,
		ID_SaveAnalyzeBtn,
		ID_CellSizeSldr,
		ID_CellSizeText,
		ID_CellUpdateBtn,
		ID_CellLinkBtn,
		ID_CellExclusiveLinkBtn,
		ID_CellUnlinkBtn,
		ID_CellNewIDText,
		ID_CellModifyBtn,
		ID_CellNewIDBtn,
		ID_CellCombineIDBtn,
		ID_CellMagic0Btn,
		ID_CellMagic1Btn,
		ID_CellMagic2Btn,
		ID_CellMagic3Btn,
		ID_CellPrevBtn,
		ID_CellNextBtn,
		ID_StatText,
		ID_CompClearBtn,
		ID_CompFullBtn,
		ID_CompIDText,
		ID_CompAppendBtn,
		ID_CompExclusiveBtn,
		ID_GenMapPrg,
		ID_GenMapBtn
	};

	TraceDlg(wxWindow* frame,
		wxWindow* parent);
	~TraceDlg();

	void GetSettings(VRenderView* vrv);
	VRenderView* GetView();
	void UpdateList();

	//manual assist
	bool GetManualAssist() {return m_manual_assist;}
	//cell operations
	void CellUpdate();
	void CellFull();
	void CellLink(bool exclusive, bool idid=false);
	void CellNewID();
	//assign exclusive ID to selection
	//mode: how to deal with conflicts
	//0--delete; 1--select
	void CellExclusiveID(int mode);
	void CellAppendID(vector<unsigned int> &id_list);
	void CompDelete();

	//measurement
	void Measure();
	void OutputMeasureResult(wxString &str);
	void SaveMeasureResult(wxString &filename);

	//automatic tracking
	void GenMap();

private:
	typedef struct
	{
		unsigned int id;
		int total_num;
		int surface_num;
		int contact_num;
	} comp_info;

	struct measure_info
	{
		unsigned int id;
		unsigned int total_num;
		double mean;
		double variance;
		double m2;
		double min;
		double max;

		static bool cmp_id(const measure_info info1, const measure_info info2)
		{ return info1.id < info2.id; }
	};
	vector<measure_info> m_info_list;

	wxWindow* m_frame;
	//current view
	VRenderView* m_view;

	//temporary mask
	Nrrd* m_mask;

	//time sequence setting
	int m_cur_time;
	int m_prv_time;

	//enable manual assist
	bool m_manual_assist;

	//ui
	//list ctrl
	TraceListCtrl *m_trace_list_curr;
	TraceListCtrl *m_trace_list_prev;

	//load/save trace
	wxTextCtrl* m_load_trace_text;
	wxButton* m_load_trace_btn;
	wxButton* m_save_trace_btn;
	wxButton* m_saveas_trace_btn;
	//ghost num
	wxSlider* m_ghost_num_sldr;
	wxTextCtrl* m_ghost_num_text;
	wxCheckBox* m_ghost_show_tail_chk;
	wxCheckBox* m_ghost_show_lead_chk;
	//edit tools
	wxButton* m_add_label_btn;
	wxButton* m_analyze_btn;
	wxButton* m_save_analyze_btn;
	wxCheckBox* m_manual_assist_check;
	//cell size filter
	wxSlider* m_cell_size_sldr;
	wxTextCtrl* m_cell_size_text;
	//auto tracking
	wxGauge* m_gen_map_prg;
	wxButton* m_gen_map_btn;
	//component tools
	wxButton* m_comp_clear_btn;
	wxButton* m_comp_full_btn;
	wxTextCtrl* m_comp_id_text;
	wxButton* m_comp_append_btn;
	wxButton* m_comp_exclusive_btn;
	//ID link controls
	wxButton* m_cell_update_btn;
	wxButton* m_cell_link_btn;
	wxButton* m_cell_exclusive_link_btn;
	wxButton* m_cell_unlink_btn;
	//ID edit controls
	wxTextCtrl* m_cell_new_id_text;
	wxButton* m_cell_modify_btn;
	wxButton* m_cell_new_id_btn;
	wxButton* m_cell_combine_id_btn;
	//magic tool
	wxButton* m_cell_magic0_btn;
	wxButton* m_cell_magic1_btn;
	wxButton* m_cell_magic2_btn;
	wxButton* m_cell_magic3_btn;
	//time controls
	wxButton* m_cell_prev_btn;
	wxButton* m_cell_next_btn;
	//time text
	wxStaticText* m_cell_time_curr_st;
	wxStaticText* m_cell_time_prev_st;

	//statistics output
	wxTextCtrl* m_stat_text;

private:
//	void AddLabel(long item, TraceListCtrl* trace_list_ctrl, vector<Lbl> *list);

	//tests
	void Test1();
	void Test2(int type);

private:
	//load/save trace
	void OnLoadTrace(wxCommandEvent& event);
	void OnSaveTrace(wxCommandEvent& event);
	void OnSaveasTrace(wxCommandEvent& event);
	//ghost number
	void OnGhostNumChange(wxScrollEvent &event);
	void OnGhostNumText(wxCommandEvent &event);
	void OnGhostShowTail(wxCommandEvent &event);
	void OnGhostShowLead(wxCommandEvent &event);
	//manual tracking assistant
	void OnAddLabel(wxCommandEvent &event);
	void OnAnalyze(wxCommandEvent &event);
	void OnSaveAnalyze(wxCommandEvent &event);
	void OnManualAssistCheck(wxCommandEvent &event);
	//cell size filter
	void OnCellSizeChange(wxScrollEvent &event);
	void OnCellSizeText(wxCommandEvent &event);
	//auto tracking
	void OnGenMapBtn(wxCommandEvent &event);
	//component tools
	void OnCompClear(wxCommandEvent &event);
	void OnCompFull(wxCommandEvent &event);
	void OnCompAppend(wxCommandEvent &event);
	void OnCompExclusive(wxCommandEvent &event);
	//ID lnik controls
	void OnCellUpdate(wxCommandEvent& event);
	void OnCellLink(wxCommandEvent& event);
	void OnCellExclusiveLink(wxCommandEvent& event);
	void OnCellUnlink(wxCommandEvent& event);
	//ID edit controls
	void OnCellModify(wxCommandEvent& event);
	void OnCellNewID(wxCommandEvent& event);
	void OnCellCombineID(wxCommandEvent& event);
	//magic tool
	void OnCellMagic0Btn(wxCommandEvent& event);
	void OnCellMagic1Btn(wxCommandEvent& event);
	void OnCellMagic2Btn(wxCommandEvent& event);
	void OnCellMagic3Btn(wxCommandEvent& event);
	//time controls
	void OnCellPrev(wxCommandEvent& event);
	void OnCellNext(wxCommandEvent& event);

	DECLARE_EVENT_TABLE();
};

#endif//_TRACEDLG_H_