/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2022 Scientific Computing and Imaging Institute,
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
#include <TrackDlg.h>
#include <RenderFrame.h>
#include <Global.hpp>
#include <AgentFactory.hpp>
#include <wx/valnum.h>
#include <wx/clipbrd.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/dirdlg.h>
#include <png_resource.h>
#include <img/icons.h>

BEGIN_EVENT_TABLE(TraceListCtrl, wxListCtrl)
EVT_KEY_DOWN(TraceListCtrl::OnKeyDown)
EVT_CONTEXT_MENU(TraceListCtrl::OnContextMenu)
EVT_MENU(Menu_CopyText, TraceListCtrl::OnCopySelection)
EVT_MENU(Menu_Delete, TraceListCtrl::OnDeleteSelection)
END_EVENT_TABLE()

TraceListCtrl::TraceListCtrl(
	RenderFrame* frame,
	wxWindow* parent,
	const wxPoint& pos,
	const wxSize& size,
	long style) :
	wxListCtrl(parent, wxID_ANY, pos, size, style),
	m_type(0)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);

	wxListItem itemCol;
	itemCol.SetText("");
	InsertColumn(0, itemCol);
	SetColumnWidth(0, 20);
	itemCol.SetText("ID");
	InsertColumn(1, itemCol);
	SetColumnWidth(1, wxLIST_AUTOSIZE_USEHEADER);
	itemCol.SetText("Size");
	InsertColumn(2, itemCol);
	SetColumnWidth(2, wxLIST_AUTOSIZE_USEHEADER);
	itemCol.SetText("X");
	InsertColumn(3, itemCol);
	SetColumnWidth(3, wxLIST_AUTOSIZE_USEHEADER);
	itemCol.SetText("Y");
	InsertColumn(4, itemCol);
	SetColumnWidth(4, wxLIST_AUTOSIZE_USEHEADER);
	itemCol.SetText("Z");
	InsertColumn(5, itemCol);
	SetColumnWidth(5, wxLIST_AUTOSIZE_USEHEADER);
}

TraceListCtrl::~TraceListCtrl()
{
}

void TraceListCtrl::Append(wxString &gtype, unsigned int id, wxColor color,
	int size, double cx, double cy, double cz)
{
	wxString str = "";
	long tmp = InsertItem(GetItemCount(), gtype, 0);
	str = wxString::Format("%u", id);
	SetItem(tmp, 1, str);
	SetColumnWidth(1, wxLIST_AUTOSIZE);
	str = wxString::Format("%d", size);
	SetItem(tmp, 2, str);
	SetColumnWidth(2, wxLIST_AUTOSIZE);
	str = wxString::Format("%f", cx);
	SetItem(tmp, 3, str);
	SetColumnWidth(3, wxLIST_AUTOSIZE);
	str = wxString::Format("%f", cy);
	SetItem(tmp, 4, str);
	SetColumnWidth(4, wxLIST_AUTOSIZE);
	str = wxString::Format("%f", cz);
	SetItem(tmp, 5, str);
	SetColumnWidth(5, wxLIST_AUTOSIZE);

	SetItemBackgroundColour(tmp, color);
}

void TraceListCtrl::DeleteSelection()
{
	if (m_type == 0)
	{
		wxWindow* parent = GetParent();
		if (parent)
		{
			m_agent->CompDelete();
			//if (((TrackDlg*)parent)->GetManualAssist())
			//	((TrackDlg*)parent)->CellLink(true);
		}
	}
	else if (m_type == 1)
	{
		long item = -1;
		for (;;)
		{
			item = GetNextItem(item,
				wxLIST_NEXT_ALL,
				wxLIST_STATE_SELECTED);
			if (item == -1)
				break;
			else
				DeleteItem(item);
		}
	}
}

wxString TraceListCtrl::GetText(long item, int col)
{
	wxListItem info;
	info.SetId(item);
	info.SetColumn(col);
	info.SetMask(wxLIST_MASK_TEXT);
	GetItem(info);
	return info.GetText();
}

void TraceListCtrl::OnKeyDown(wxKeyEvent& event)
{
	if (event.GetKeyCode() == WXK_DELETE ||
		event.GetKeyCode() == WXK_BACK)
		DeleteSelection();
	if (event.GetKeyCode() == wxKeyCode('C') &&
		wxGetKeyState(WXK_CONTROL))
	{
		wxCommandEvent evt;
		OnCopySelection(evt);
	}
	event.Skip();
}

void TraceListCtrl::OnContextMenu(wxContextMenuEvent &event)
{
	if (GetSelectedItemCount() > 0)
	{
		wxPoint point = event.GetPosition();
		//if from keyboard
		if (point.x == -1 && point.y == -1)
		{
			wxSize size = GetSize();
			point.x = size.x / 2;
			point.y = size.y / 2;
		}
		else
		{
			point = ScreenToClient(point);
		}

		wxMenu menu;
		menu.Append(Menu_CopyText, "Copy");
		menu.Append(Menu_Delete, "Delete");

		PopupMenu(&menu, point.x, point.y);
	}
}

void TraceListCtrl::OnCopySelection(wxCommandEvent& event)
{
	long item = GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	if (item != -1)
	{
		wxString name = GetItemText(item, 1);
		if (wxTheClipboard->Open())
		{
			wxTheClipboard->SetData(new wxTextDataObject(name));
			wxTheClipboard->Close();
		}
	}
}

void TraceListCtrl::OnDeleteSelection(wxCommandEvent& event)
{
	DeleteSelection();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
BEGIN_EVENT_TABLE(TrackDlg, wxPanel)
//map page
//load/save trace
EVT_BUTTON(ID_ClearTraceBtn, TrackDlg::OnClearTrace)
EVT_BUTTON(ID_LoadTraceBtn, TrackDlg::OnLoadTrace)
EVT_BUTTON(ID_SaveTraceBtn, TrackDlg::OnSaveTrace)
EVT_BUTTON(ID_SaveasTraceBtn, TrackDlg::OnSaveasTrace)
//auto tracking
EVT_BUTTON(ID_GenMapBtn, TrackDlg::OnGenMapBtn)
EVT_BUTTON(ID_RefineTBtn, TrackDlg::OnRefineTBtn)
EVT_BUTTON(ID_RefineAllBtn, TrackDlg::OnRefineAllBtn)
//settings
EVT_SPINCTRL(ID_MapIterSpin, TrackDlg::OnMapIterSpin)
EVT_TEXT(ID_MapIterSpin, TrackDlg::OnMapIterText)
EVT_SPINCTRL(ID_MapSizeSpin, TrackDlg::OnMapSizeSpin)
EVT_TEXT(ID_MapSizeSpin, TrackDlg::OnMapSizeText)
EVT_TOGGLEBUTTON(ID_MapConsistentBtn, TrackDlg::OnMapConsistentBtn)
EVT_TOGGLEBUTTON(ID_MapMergeBtn, TrackDlg::OnMapMergeBtn)
EVT_TOGGLEBUTTON(ID_MapSplitBtn, TrackDlg::OnMapSplitBtn)
EVT_SPINCTRLDOUBLE(ID_MapSimilarSpin, TrackDlg::OnMapSimilarSpin)
EVT_TEXT(ID_MapSimilarSpin, TrackDlg::OnMapSimilarText)
EVT_SPINCTRLDOUBLE(ID_MapContactSpin, TrackDlg::OnMapContactSpin)
EVT_TEXT(ID_MapContactSpin, TrackDlg::OnMapContactText)
//selection page
//component tools
EVT_TEXT(ID_CompIDText, TrackDlg::OnCompIDText)
EVT_TEXT_ENTER(ID_CompIDText, TrackDlg::OnCompFull)
EVT_BUTTON(ID_CompIDXBtn, TrackDlg::OnCompIDXBtn)
EVT_BUTTON(ID_CompFullBtn, TrackDlg::OnCompFull)
EVT_BUTTON(ID_CompExclusiveBtn, TrackDlg::OnCompExclusive)
EVT_BUTTON(ID_CompAppendBtn, TrackDlg::OnCompAppend)
EVT_BUTTON(ID_CompClearBtn, TrackDlg::OnCompClear)
EVT_BUTTON(ID_ShuffleBtn, TrackDlg::OnShuffle)
//cell size filter
EVT_COMMAND_SCROLL(ID_CellSizeSldr, TrackDlg::OnCellSizeChange)
EVT_TEXT(ID_CellSizeText, TrackDlg::OnCellSizeText)
//uncertainty filter
EVT_BUTTON(ID_CompUncertainBtn, TrackDlg::OnCompUncertainBtn)
EVT_COMMAND_SCROLL(ID_CompUncertainLowSldr, TrackDlg::OnCompUncertainLowChange)
EVT_TEXT(ID_CompUncertainLowText, TrackDlg::OnCompUncertainLowText)
//link page
EVT_TEXT(ID_CompIDText2, TrackDlg::OnCompIDText)
EVT_TEXT_ENTER(ID_CompIDText2, TrackDlg::OnCompAppend)
//ID link controls
EVT_BUTTON(ID_CellExclusiveLinkBtn, TrackDlg::OnCellExclusiveLink)
EVT_BUTTON(ID_CellLinkBtn, TrackDlg::OnCellLink)
EVT_BUTTON(ID_CellLinkAllBtn, TrackDlg::OnCellLinkAll)
EVT_BUTTON(ID_CellIsolateBtn, TrackDlg::OnCellIsolate)
EVT_BUTTON(ID_CellUnlinkBtn, TrackDlg::OnCellUnlink)
//modify page
//ID edit controls
EVT_TEXT(ID_CellNewIDText, TrackDlg::OnCellNewIDText)
EVT_TEXT_ENTER(ID_CellNewIDText, TrackDlg::OnCompAppend)
EVT_BUTTON(ID_CellNewIDXBtn, TrackDlg::OnCellNewIDX)
EVT_BUTTON(ID_CompAppend2Btn, TrackDlg::OnCompAppend)
EVT_BUTTON(ID_CellNewIDBtn, TrackDlg::OnCellNewID)
EVT_BUTTON(ID_CellAppendIDBtn, TrackDlg::OnCellAppendID)
EVT_BUTTON(ID_CellReplaceIDBtn, TrackDlg::OnCellReplaceID)
EVT_BUTTON(ID_CellCombineIDBtn, TrackDlg::OnCellCombineID)
EVT_BUTTON(ID_CellSeparateBtn, TrackDlg::OnCellSeparateID)
EVT_BUTTON(ID_CellSegBtn, TrackDlg::OnCellSegment)
EVT_SPINCTRL(ID_CellSegText, TrackDlg::OnCellSegSpin)
EVT_TEXT(ID_CellSegText, TrackDlg::OnCellSegText)
//analysis page
//conversion
EVT_BUTTON(ID_ConvertToRulersBtn, TrackDlg::OnConvertToRulers)
EVT_BUTTON(ID_ConvertConsistentBtn, TrackDlg::OnConvertConsistent)
//analysis
EVT_BUTTON(ID_AnalyzeCompBtn, TrackDlg::OnAnalyzeComp)
EVT_BUTTON(ID_AnalyzeLinkBtn, TrackDlg::OnAnalyzeLink)
EVT_BUTTON(ID_AnalyzeUncertainHistBtn, TrackDlg::OnAnalyzeUncertainHist)
EVT_BUTTON(ID_AnalyzePathBtn, TrackDlg::OnAnalyzePath)
EVT_BUTTON(ID_SaveResultBtn, TrackDlg::OnSaveResult)
//ghost num
EVT_COMMAND_SCROLL(ID_GhostNumSldr, TrackDlg::OnGhostNumChange)
EVT_TEXT(ID_GhostNumText, TrackDlg::OnGhostNumText)
EVT_CHECKBOX(ID_GhostShowTailChk, TrackDlg::OnGhostShowTail)
EVT_CHECKBOX(ID_GhostShowLeadChk, TrackDlg::OnGhostShowLead)
//time controls
EVT_BUTTON(ID_CellPrevBtn, TrackDlg::OnCellPrev)
EVT_BUTTON(ID_CellNextBtn, TrackDlg::OnCellNext)
END_EVENT_TABLE()

wxWindow* TrackDlg::CreateMapPage(wxWindow *parent)
{
	wxPanel *page = new wxPanel(parent);

	wxStaticText *st = 0;

	//load trace
	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Track map:",
		wxDefaultPosition, wxSize(70, 20));
	m_load_trace_text = new wxTextCtrl(page, ID_LoadTraceText, "",
		wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_clear_trace_btn = new wxButton(page, ID_ClearTraceBtn, "X",
		wxDefaultPosition, wxSize(23, 23));
	m_load_trace_btn = new wxButton(page, ID_LoadTraceBtn, "Load",
		wxDefaultPosition, wxSize(65, 23));
	m_save_trace_btn = new wxButton(page, ID_SaveTraceBtn, "Save",
		wxDefaultPosition, wxSize(65, 23));
	m_saveas_trace_btn = new wxButton(page, ID_SaveasTraceBtn, "Save As",
		wxDefaultPosition, wxSize(65, 23));
	sizer_1->Add(5, 5);
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	sizer_1->Add(m_load_trace_text, 1, wxEXPAND);
	sizer_1->Add(m_clear_trace_btn, 0, wxALIGN_CENTER);
	sizer_1->Add(m_load_trace_btn, 0, wxALIGN_CENTER);
	sizer_1->Add(m_save_trace_btn, 0, wxALIGN_CENTER);
	sizer_1->Add(m_saveas_trace_btn, 0, wxALIGN_CENTER);

	//generate
	//settings
	wxBoxSizer* sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Iterations:",
		wxDefaultPosition, wxDefaultSize);
	m_map_iter_spin = new wxSpinCtrl(page, ID_MapIterSpin, "3",
		wxDefaultPosition, wxSize(50, 23));
	sizer_2->Add(5, 5);
	sizer_2->Add(st, 0, wxALIGN_CENTER);
	sizer_2->Add(5, 5);
	sizer_2->Add(m_map_iter_spin, 0, wxALIGN_CENTER);
	st = new wxStaticText(page, 0, "Size Threshold:",
		wxDefaultPosition, wxDefaultSize);
	m_map_size_spin = new wxSpinCtrl(page, ID_MapSizeSpin, "100",
		wxDefaultPosition, wxSize(50, 23));
	m_map_size_spin->SetRange(1, std::numeric_limits<int>::max());
	sizer_2->Add(5, 5);
	sizer_2->Add(st, 0, wxALIGN_CENTER);
	sizer_2->Add(5, 5);
	sizer_2->Add(m_map_size_spin, 0, wxALIGN_CENTER);
	m_gen_map_btn = new wxButton(page, ID_GenMapBtn, "Generate",
		wxDefaultPosition, wxSize(100, 23));
	m_refine_t_btn = new wxButton(page, ID_RefineTBtn, "Refine T",
		wxDefaultPosition, wxSize(100, 23));
	m_refine_all_btn = new wxButton(page, ID_RefineAllBtn, "Refine All",
		wxDefaultPosition, wxSize(100, 23));
	sizer_2->Add(5, 5);
	sizer_2->Add(m_gen_map_btn, 0, wxALIGN_CENTER);
	sizer_2->Add(m_refine_t_btn, 0, wxALIGN_CENTER);
	sizer_2->Add(m_refine_all_btn, 0, wxALIGN_CENTER);

	//
	wxBoxSizer* sizer_3 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Similarity:",
		wxDefaultPosition, wxDefaultSize);
	m_map_similar_spin = new wxSpinCtrlDouble(
		page, ID_MapSimilarSpin, "0.2",
		wxDefaultPosition, wxSize(50, 23),
		wxSP_ARROW_KEYS| wxSP_WRAP,
		0, 1, 0.2, 0.01);
	sizer_3->Add(5, 5);
	sizer_3->Add(st, 0, wxALIGN_CENTER);
	sizer_3->Add(5, 5);
	sizer_3->Add(m_map_similar_spin, 0, wxALIGN_CENTER);
	st = new wxStaticText(page, 0, "Contact Factor:",
		wxDefaultPosition, wxDefaultSize);
	m_map_contact_spin = new wxSpinCtrlDouble(
		page, ID_MapContactSpin, "0.6",
		wxDefaultPosition, wxSize(50, 23),
		wxSP_ARROW_KEYS | wxSP_WRAP,
		0, 1, 0.6, 0.01);
	sizer_3->Add(5, 5);
	sizer_3->Add(st, 0, wxALIGN_CENTER);
	sizer_3->Add(5, 5);
	sizer_3->Add(m_map_contact_spin, 0, wxALIGN_CENTER);
	m_map_consistent_btn = new wxToggleButton(page, ID_MapConsistentBtn,
		"Consistent Colors", wxDefaultPosition, wxSize(100, 23));
	m_map_merge_btn = new wxToggleButton(
		page, ID_MapMergeBtn, "Try Merging",
		wxDefaultPosition, wxSize(100, 23));
	m_map_split_btn = new wxToggleButton(
		page, ID_MapSplitBtn, "Try Splitting",
		wxDefaultPosition, wxSize(100, 23));
	sizer_3->Add(5, 5);
	sizer_3->Add(m_map_consistent_btn, 0, wxALIGN_CENTER);
	sizer_3->Add(m_map_merge_btn, 0, wxALIGN_CENTER);
	sizer_3->Add(m_map_split_btn, 0, wxALIGN_CENTER);

	//vertical sizer
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
	sizer_v->Add(10, 10);
	sizer_v->Add(sizer_1, 0, wxEXPAND);
	sizer_v->Add(10, 10);
	sizer_v->Add(sizer_2, 0, wxEXPAND);
	sizer_v->Add(10, 10);
	sizer_v->Add(sizer_3, 0, wxEXPAND);
	sizer_v->Add(10, 10);

	//set the page
	page->SetSizer(sizer_v);
	return page;
}

wxWindow* TrackDlg::CreateSelectPage(wxWindow *parent)
{
	wxPanel *page = new wxPanel(parent);

	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;
	wxStaticText *st = 0;

	//selection tools
	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Selection tools:",
		wxDefaultPosition, wxSize(100, 20));
	m_comp_id_text = new wxTextCtrl(page, ID_CompIDText, "",
		wxDefaultPosition, wxSize(77, 23), wxTE_PROCESS_ENTER);
	m_comp_id_x_btn = new wxButton(page, ID_CompIDXBtn, "X",
		wxDefaultPosition, wxSize(23, 23));
	m_comp_full_btn = new wxButton(page, ID_CompFullBtn, "FullCompt",
		wxDefaultPosition, wxSize(64, 23));
	m_comp_exclusive_btn = new wxButton(page, ID_CompExclusiveBtn, "Replace",
		wxDefaultPosition, wxSize(64, 23));
	m_comp_append_btn = new wxButton(page, ID_CompAppendBtn, "Append",
		wxDefaultPosition, wxSize(64, 23));
	m_comp_clear_btn = new wxButton(page, ID_CompClearBtn, "Clear",
		wxDefaultPosition, wxSize(64, 23));
	m_shuffle_btn = new wxButton(page, ID_ShuffleBtn, "Shuffle",
		wxDefaultPosition, wxSize(64, 23));
	sizer_1->Add(5, 5);
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	sizer_1->Add(m_comp_id_text, 0, wxALIGN_CENTER);
	sizer_1->Add(m_comp_id_x_btn, 0, wxALIGN_CENTER);
	sizer_1->Add(10, 23);
	sizer_1->Add(m_comp_full_btn, 0, wxALIGN_CENTER);
	sizer_1->Add(m_comp_exclusive_btn, 0, wxALIGN_CENTER);
	sizer_1->Add(m_comp_append_btn, 0, wxALIGN_CENTER);
	sizer_1->Add(m_comp_clear_btn, 0, wxALIGN_CENTER);
	sizer_1->Add(m_shuffle_btn, 0, wxALIGN_CENTER);
	//cell size filter
	wxBoxSizer* sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Component size:",
		wxDefaultPosition, wxSize(110, 20));
	m_cell_size_sldr = new wxSlider(page, ID_CellSizeSldr, 20, 0, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_cell_size_text = new wxTextCtrl(page, ID_CellSizeText, "20",
		wxDefaultPosition, wxSize(60, 23), 0, vald_int);
	sizer_2->Add(5, 5);
	sizer_2->Add(st, 0, wxALIGN_CENTER);
	sizer_2->Add(m_cell_size_sldr, 1, wxEXPAND);
	sizer_2->Add(m_cell_size_text, 0, wxALIGN_CENTER);
	//uncertainty filter
	wxBoxSizer* sizer_3 = new wxBoxSizer(wxHORIZONTAL);
	m_comp_uncertain_btn = new wxButton(page, ID_CompUncertainBtn, "Uncertainty",
		wxDefaultPosition, wxSize(80, 23));
	m_comp_uncertain_low_sldr = new wxSlider(page, ID_CompUncertainLowSldr, 0, 0, 20,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_comp_uncertain_low_text = new wxTextCtrl(page, ID_CompUncertainLowText, "0",
		wxDefaultPosition, wxSize(60, 23), 0, vald_int);
	sizer_3->Add(5, 5);
	sizer_3->Add(m_comp_uncertain_btn, 0, wxALIGN_CENTER);
	sizer_3->Add(30, 23);
	sizer_3->Add(m_comp_uncertain_low_sldr, 1, wxEXPAND);
	sizer_3->Add(m_comp_uncertain_low_text, 0, wxALIGN_CENTER);

	//vertical sizer
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
	sizer_v->Add(10, 10);
	sizer_v->Add(sizer_1, 0, wxEXPAND);
	sizer_v->Add(10, 10);
	sizer_v->Add(sizer_2, 0, wxEXPAND);
	sizer_v->Add(10, 10);
	sizer_v->Add(sizer_3, 0, wxEXPAND);
	sizer_v->Add(10, 10);

	//set the page
	page->SetSizer(sizer_v);
	return page;
}

wxWindow* TrackDlg::CreateLinkPage(wxWindow *parent)
{
	wxPanel *page = new wxPanel(parent);

	wxStaticText *st = 0;

	//selection
	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Selection tools:",
		wxDefaultPosition, wxSize(100, 20));
	m_comp_id_text2 = new wxTextCtrl(page, ID_CompIDText2, "",
		wxDefaultPosition, wxSize(77, 23), wxTE_PROCESS_ENTER);
	m_comp_id_x_btn = new wxButton(page, ID_CompIDXBtn, "X",
		wxDefaultPosition, wxSize(23, 23));
	m_comp_append_btn = new wxButton(page, ID_CompAppendBtn, "Append",
		wxDefaultPosition, wxSize(80, 23));
	m_comp_clear_btn = new wxButton(page, ID_CompClearBtn, "Clear",
		wxDefaultPosition, wxSize(80, 23));
	sizer_1->Add(5, 5);
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	sizer_1->Add(m_comp_id_text2, 0, wxALIGN_CENTER);
	sizer_1->Add(m_comp_id_x_btn, 0, wxALIGN_CENTER);
	sizer_1->Add(10, 23);
	sizer_1->Add(m_comp_append_btn, 0, wxALIGN_CENTER);
	sizer_1->Add(m_comp_clear_btn, 0, wxALIGN_CENTER);
	sizer_1->AddStretchSpacer();

	//ID link controls
	wxBoxSizer* sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	m_cell_exclusive_link_btn = new wxButton(page, ID_CellExclusiveLinkBtn, "Excl. Link",
		wxDefaultPosition, wxSize(80, 23));
	m_cell_link_btn = new wxButton(page, ID_CellLinkBtn, "Link IDs",
		wxDefaultPosition, wxSize(80, 23));
	m_cell_link_all_btn = new wxButton(page, ID_CellLinkAllBtn, "Link New IDs",
		wxDefaultPosition, wxSize(80, 23));
	sizer_2->AddStretchSpacer();
	sizer_2->Add(m_cell_exclusive_link_btn, 0, wxALIGN_CENTER);
	sizer_2->Add(m_cell_link_btn, 0, wxALIGN_CENTER);
	sizer_2->Add(m_cell_link_all_btn, 0, wxALIGN_CENTER);
	sizer_2->AddStretchSpacer();

	//ID unlink controls
	wxBoxSizer* sizer_3 = new wxBoxSizer(wxHORIZONTAL);
	m_cell_isolate_btn = new wxButton(page, ID_CellIsolateBtn, "Isolate",
		wxDefaultPosition, wxSize(80, 23));
	m_cell_unlink_btn = new wxButton(page, ID_CellUnlinkBtn, "Unlink IDs",
		wxDefaultPosition, wxSize(80, 23));
	sizer_3->AddStretchSpacer();
	sizer_3->Add(m_cell_isolate_btn, 0, wxALIGN_CENTER);
	sizer_3->Add(m_cell_unlink_btn, 0, wxALIGN_CENTER);
	sizer_3->AddStretchSpacer();

	//vertical sizer
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
	sizer_v->Add(10, 10);
	sizer_v->Add(sizer_1, 0, wxEXPAND);
	sizer_v->Add(10, 10);
	sizer_v->Add(sizer_2, 0, wxEXPAND);
	sizer_v->Add(10, 10);
	sizer_v->Add(sizer_3, 0, wxEXPAND);
	sizer_v->Add(10, 10);

	//set the page
	page->SetSizer(sizer_v);
	return page;
}

wxWindow* TrackDlg::CreateModifyPage(wxWindow *parent)
{
	wxPanel *page = new wxPanel(parent);

	wxStaticText *st = 0;

	//ID input
	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "New ID/Selection:",
		wxDefaultPosition, wxSize(100, 20));
	m_cell_new_id_text = new wxTextCtrl(page, ID_CellNewIDText, "",
		wxDefaultPosition, wxSize(77, 23), wxTE_PROCESS_ENTER);
	m_cell_new_id_x_btn = new wxButton(page, ID_CellNewIDXBtn, "X",
		wxDefaultPosition, wxSize(23, 23));
	m_comp_append2_btn = new wxButton(page, ID_CompAppend2Btn, "Append",
		wxDefaultPosition, wxSize(80, 23));
	m_comp_clear_btn = new wxButton(page, ID_CompClearBtn, "Clear",
		wxDefaultPosition, wxSize(80, 23));
	sizer_1->Add(5, 5);
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	sizer_1->Add(m_cell_new_id_text, 0, wxALIGN_CENTER);
	sizer_1->Add(m_cell_new_id_x_btn, 0, wxALIGN_CENTER);
	sizer_1->Add(10, 23);
	sizer_1->Add(m_comp_append2_btn, 0, wxALIGN_CENTER);
	sizer_1->Add(m_comp_clear_btn, 0, wxALIGN_CENTER);
	sizer_1->AddStretchSpacer();

	//controls
	wxBoxSizer* sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	m_cell_new_id_btn = new wxButton(page, ID_CellNewIDBtn, "Assign ID",
		wxDefaultPosition, wxSize(85, 23));
	m_cell_append_id_btn = new wxButton(page, ID_CellAppendIDBtn, "Add ID",
		wxDefaultPosition, wxSize(85, 23));
	m_cell_replace_id_btn = new wxButton(page, ID_CellReplaceIDBtn, "Replace ID",
		wxDefaultPosition, wxSize(85, 23));
	sizer_2->AddStretchSpacer();
	sizer_2->Add(m_cell_new_id_btn, 0, wxALIGN_CENTER);
	sizer_2->Add(m_cell_append_id_btn, 0, wxALIGN_CENTER);
	sizer_2->Add(m_cell_replace_id_btn, 0, wxALIGN_CENTER);
	sizer_2->AddStretchSpacer();

	wxBoxSizer* sizer_3 = new wxBoxSizer(wxHORIZONTAL);
	m_cell_combine_id_btn = new wxButton(page, ID_CellCombineIDBtn, "Combine",
		wxDefaultPosition, wxSize(85, 23));
	m_cell_separate_id_btn = new wxButton(page, ID_CellSeparateBtn, "Separate",
		wxDefaultPosition, wxSize(85, 23));
	m_cell_segment_text = new wxSpinCtrl(page, ID_CellSegText, "2",
		wxDefaultPosition, wxSize(40, 21));
	m_cell_segment_btn = new wxButton(page, ID_CellSegBtn, "Segment",
		wxDefaultPosition, wxSize(65, 23));
	sizer_3->AddStretchSpacer();
	sizer_3->Add(m_cell_combine_id_btn, 0, wxALIGN_CENTER);
	sizer_3->Add(m_cell_separate_id_btn, 0, wxALIGN_CENTER);
	sizer_3->Add(10, 10);
	sizer_3->Add(m_cell_segment_text, 0, wxALIGN_CENTER);
	sizer_3->Add(m_cell_segment_btn, 0, wxALIGN_CENTER);
	sizer_3->AddStretchSpacer();

	//vertical sizer
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
	sizer_v->Add(10, 10);
	sizer_v->Add(sizer_1, 0, wxEXPAND);
	sizer_v->Add(10, 10);
	sizer_v->Add(sizer_2, 0, wxEXPAND);
	sizer_v->Add(10, 10);
	sizer_v->Add(sizer_3, 0, wxEXPAND);
	sizer_v->Add(10, 10);

	//set the page
	page->SetSizer(sizer_v);
	return page;
}

wxWindow* TrackDlg::CreateAnalysisPage(wxWindow *parent)
{
	wxPanel *page = new wxPanel(parent);

	wxStaticText *st = 0;

	//conversion
	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Convert to:",
		wxDefaultPosition, wxSize(100, 20));
	m_convert_to_rulers_btn = new wxButton(page, ID_ConvertToRulersBtn, "Rulers",
		wxDefaultPosition, wxSize(80, 23));
	m_convert_consistent_btn = new wxButton(page, ID_ConvertConsistentBtn, "UniIDs",
		wxDefaultPosition, wxSize(80, 23));
	sizer_1->Add(5, 5);
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	sizer_1->Add(m_convert_to_rulers_btn, 0, wxALIGN_CENTER);
	sizer_1->Add(m_convert_consistent_btn, 0, wxALIGN_CENTER);

	//analysis
	wxBoxSizer* sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Information:",
		wxDefaultPosition, wxSize(100, 20));
	m_analyze_comp_btn = new wxButton(page, ID_AnalyzeCompBtn, "Compnts",
		wxDefaultPosition, wxSize(80, 23));
	m_analyze_link_btn = new wxButton(page, ID_AnalyzeLinkBtn, "Links",
		wxDefaultPosition, wxSize(80, 23));
	m_analyze_uncertain_hist_btn = new wxButton(page, ID_AnalyzeUncertainHistBtn, "Uncertainty",
		wxDefaultPosition, wxSize(80, 23));
	m_analyze_path_btn = new wxButton(page, ID_AnalyzePathBtn, "Paths",
		wxDefaultPosition, wxSize(80, 23));
	sizer_2->Add(5, 5);
	sizer_2->Add(st, 0, wxALIGN_CENTER);
	sizer_2->Add(m_analyze_comp_btn, 0, wxALIGN_CENTER);
	sizer_2->Add(m_analyze_link_btn, 0, wxALIGN_CENTER);
	sizer_2->Add(m_analyze_uncertain_hist_btn, 0, wxALIGN_CENTER);
	sizer_2->Add(m_analyze_path_btn, 0, wxALIGN_CENTER);

	//Export
	wxBoxSizer* sizer_3 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Export:",
		wxDefaultPosition, wxSize(100, 20));
	m_save_result_btn = new wxButton(page, ID_SaveResultBtn, "Save As",
		wxDefaultPosition, wxSize(80, 23));
	sizer_3->Add(5, 5);
	sizer_3->Add(st, 0, wxALIGN_CENTER);
	sizer_3->Add(m_save_result_btn, 0, wxALIGN_CENTER);

	//vertical sizer
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
	sizer_v->Add(10, 10);
	sizer_v->Add(sizer_1, 0, wxEXPAND);
	sizer_v->Add(10, 10);
	sizer_v->Add(sizer_2, 0, wxEXPAND);
	sizer_v->Add(10, 10);
	sizer_v->Add(sizer_3, 0, wxEXPAND);
	sizer_v->Add(10, 10);

	//set the page
	page->SetSizer(sizer_v);
	return page;
}

TrackDlg::TrackDlg(RenderFrame* frame)
	: wxPanel(frame, wxID_ANY,
		wxDefaultPosition, wxSize(550, 650),
		0, "TrackDlg"),
	m_frame(frame)
{
	m_agent = glbin_agtf->addTrackAgent(gstTrackAgent, *this);

	// temporarily block events during constructor:
	wxEventBlocker blocker(this);

	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;

	//notebook
	m_notebook = new wxNotebook(this, wxID_ANY);
	m_notebook->AddPage(CreateMapPage(m_notebook), L"Track Map");
	m_notebook->AddPage(CreateSelectPage(m_notebook), L"Selection");
	m_notebook->AddPage(CreateModifyPage(m_notebook), L"Modify");
	m_notebook->AddPage(CreateLinkPage(m_notebook), L"Linkage");
	m_notebook->AddPage(CreateAnalysisPage(m_notebook), "Analysis");

	wxStaticText *st = 0;

	//ghost num
	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Tracks:",
		wxDefaultPosition, wxSize(70, 20));
	m_ghost_show_tail_chk = new wxCheckBox(this, ID_GhostShowTailChk, "Tail",
		wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
	m_ghost_num_sldr = new wxSlider(this, ID_GhostNumSldr, 10, 0, 20,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_ghost_num_text = new wxTextCtrl(this, ID_GhostNumText, "10",
		wxDefaultPosition, wxSize(60, 23), 0, vald_int);
	m_ghost_show_lead_chk = new wxCheckBox(this, ID_GhostShowLeadChk, "Lead",
		wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
	sizer_1->Add(5, 5);
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	sizer_1->Add(m_ghost_show_tail_chk, 0, wxALIGN_CENTER);
	sizer_1->Add(5, 5);
	sizer_1->Add(m_ghost_num_sldr, 1, wxEXPAND);
	sizer_1->Add(m_ghost_num_text, 0, wxALIGN_CENTER);
	sizer_1->Add(5, 5);
	sizer_1->Add(m_ghost_show_lead_chk, 0, wxALIGN_CENTER);

	//lists
	wxBoxSizer *sizer_2 = new wxStaticBoxSizer(
		new wxStaticBox(this, wxID_ANY, "ID Lists"),
		wxVERTICAL);
	//titles
	wxBoxSizer* sizer_21 = new wxBoxSizer(wxHORIZONTAL);
	m_cell_time_curr_st = new wxStaticText(this, 0, "\tCurrent T",
		wxDefaultPosition, wxDefaultSize);
	m_cell_time_prev_st = new wxStaticText(this, 0, "\tPrevious T",
		wxDefaultPosition, wxDefaultSize);
	m_cell_prev_btn = new wxButton(this, ID_CellPrevBtn, L"\u21e6 Backward (A)",
		wxDefaultPosition, wxSize(100, 23));
	m_cell_next_btn = new wxButton(this, ID_CellNextBtn, L"Forward (D) \u21e8",
		wxDefaultPosition, wxSize(100, 23));
	sizer_21->Add(m_cell_time_curr_st, 1, wxEXPAND);
	sizer_21->Add(m_cell_prev_btn, 0, wxALIGN_CENTER);
	sizer_21->Add(m_cell_next_btn, 0, wxALIGN_CENTER);
	sizer_21->Add(m_cell_time_prev_st, 1, wxEXPAND);
	//controls
	wxBoxSizer* sizer_22 = new wxBoxSizer(wxHORIZONTAL);
	m_trace_list_curr = new TraceListCtrl(frame, this);
	m_trace_list_curr->m_type = 0;
	m_trace_list_curr->m_agent = m_agent;
	m_trace_list_prev = new TraceListCtrl(frame, this);
	m_trace_list_prev->m_type = 1;
	m_trace_list_prev->m_agent = m_agent;
	sizer_22->Add(m_trace_list_curr, 1, wxEXPAND);
	sizer_22->Add(m_trace_list_prev, 1, wxEXPAND);
	//
	sizer_2->Add(sizer_21, 0, wxEXPAND);
	sizer_2->Add(sizer_22, 1, wxEXPAND);

	//stats text
	wxBoxSizer *sizer_3 = new wxStaticBoxSizer(
		new wxStaticBox(this, wxID_ANY, "Output"),
		wxVERTICAL);
	m_stat_text = new wxTextCtrl(this, ID_StatText, "",
		wxDefaultPosition, wxSize(-1, 100), wxTE_MULTILINE);
	m_stat_text->SetEditable(false);
	sizer_3->Add(m_stat_text, 1, wxEXPAND);

	//all controls
	wxBoxSizer *sizer_v = new wxBoxSizer(wxVERTICAL);
	sizer_v->Add(10, 10);
	sizer_v->Add(m_notebook, 0, wxEXPAND);
	sizer_v->Add(10, 10);
	sizer_v->Add(sizer_1, 0, wxEXPAND);
	sizer_v->Add(10, 10);
	sizer_v->Add(sizer_2, 1, wxEXPAND);
	sizer_v->Add(10, 10);
	sizer_v->Add(sizer_3, 0, wxEXPAND);
	sizer_v->Add(10, 10);

	SetSizer(sizer_v);
	Layout();
}

TrackDlg::~TrackDlg()
{
	//if (m_mask)
	//{
	//	delete[] reinterpret_cast<char*>(m_mask->data);
	//	nrrdNix(m_mask);
	//}
}

void TrackDlg::SetComponentAgent()
{
	m_compagent = glbin_agtf->getComponentAgent();
}

void TrackDlg::SetMovieAgent()
{
	m_movieagent = glbin_agtf->getMovieAgent();
}

void TrackDlg::OnClearTrace(wxCommandEvent& event)
{
	m_agent->ClearTrack();
}

void TrackDlg::OnLoadTrace(wxCommandEvent& event)
{
	wxFileDialog *fopendlg = new wxFileDialog(
		m_frame, "Choose a FluoRender track file",
		"", "", "*.track", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		std::wstring filename = fopendlg->GetPath();
		m_agent->LoadTrackFile(filename);
	}

	if (fopendlg)
		delete fopendlg;
}

void TrackDlg::OnSaveTrace(wxCommandEvent& event)
{
	std::wstring filename;
	m_agent->getValue(gstTrackFile, filename);
	if (wxFileExists(filename))
		m_agent->SaveTrackFile(filename);
	else
		OnSaveasTrace(event);
}

void TrackDlg::OnSaveasTrace(wxCommandEvent& event)
{
	wxFileDialog *fopendlg = new wxFileDialog(
		m_frame, "Save a FluoRender track file",
		"", "", "*.track", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		wxString filename = fopendlg->GetPath();
		m_agent->SaveTrackFile(filename.ToStdWstring());
	}

	if (fopendlg)
		delete fopendlg;
}

void TrackDlg::OnGhostNumChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	wxString str = wxString::Format("%d", ival);
	if (str != m_ghost_num_text->GetValue())
		m_ghost_num_text->SetValue(str);
}

void TrackDlg::OnGhostNumText(wxCommandEvent &event)
{
	wxString str = m_ghost_num_text->GetValue();
	long ival;
	str.ToLong(&ival);
	m_ghost_num_sldr->SetValue(ival);
	m_agent->setValue(gstGhostNum, ival);
}

void TrackDlg::OnGhostShowTail(wxCommandEvent &event)
{
	bool bval = m_ghost_show_tail_chk->GetValue();
	m_agent->setValue(gstGhostTailEnable, bval);
}

void TrackDlg::OnGhostShowLead(wxCommandEvent &event)
{
	bool bval = m_ghost_show_lead_chk->GetValue();
	m_agent->setValue(gstGhostLeadEnable, bval);
}

//cell size filter
void TrackDlg::OnCellSizeChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	wxString str = wxString::Format("%d", ival);
	if (str != m_cell_size_text->GetValue())
		m_cell_size_text->SetValue(str);
}

void TrackDlg::OnCellSizeText(wxCommandEvent &event)
{
	wxString str = m_cell_size_text->GetValue();
	long ival;
	str.ToLong(&ival);
	m_cell_size_sldr->SetValue(ival);
	m_agent->setValue(gstTrackCellSize, ival);
}

void TrackDlg::OnCompUncertainBtn(wxCommandEvent &event)
{
	m_agent->UncertainFilter();
}

void TrackDlg::OnCompUncertainLowChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	wxString str = wxString::Format("%d", ival);
	if (str != m_comp_uncertain_low_text->GetValue())
		m_comp_uncertain_low_text->SetValue(str);
}

void TrackDlg::OnCompUncertainLowText(wxCommandEvent &event)
{
	wxString str = m_comp_uncertain_low_text->GetValue();
	long ival;
	str.ToLong(&ival);
	m_comp_uncertain_low_sldr->SetValue(ival);
	m_agent->setValue(gstCompUncertainLow, ival);
}

//auto tracking
void TrackDlg::OnGenMapBtn(wxCommandEvent &event)
{
	m_agent->GenMap();
}

void TrackDlg::OnRefineTBtn(wxCommandEvent &event)
{
	m_agent->RefineMap();
}

void TrackDlg::OnRefineAllBtn(wxCommandEvent &event)
{
	m_agent->RefineMap(false);
}

//settings
void TrackDlg::OnMapIterSpin(wxSpinEvent& event)
{
	long lval = m_map_iter_spin->GetValue();
	m_agent->setValue(gstTrackIter, lval);
}

void TrackDlg::OnMapIterText(wxCommandEvent& event)
{
	long lval = m_map_iter_spin->GetValue();
	m_agent->setValue(gstTrackIter, lval);
}

void TrackDlg::OnMapSizeSpin(wxSpinEvent& event)
{
	long lval = m_map_size_spin->GetValue();
	m_agent->setValue(gstCompSizeLimit, lval);
}

void TrackDlg::OnMapSizeText(wxCommandEvent& event)
{
	long lval = m_map_size_spin->GetValue();
	m_agent->setValue(gstCompSizeLimit, lval);
}

void TrackDlg::OnMapConsistentBtn(wxCommandEvent& event)
{
	bool bval = m_map_consistent_btn->GetValue();
	m_agent->setValue(gstCompConsistent, bval);
}

void TrackDlg::OnMapMergeBtn(wxCommandEvent& event)
{
	bool bval = m_map_merge_btn->GetValue();
	m_agent->setValue(gstTryMerge, bval);
}

void TrackDlg::OnMapSplitBtn(wxCommandEvent& event)
{
	bool bval = m_map_split_btn->GetValue();
	m_agent->setValue(gstTrySplit, bval);
}

void TrackDlg::OnMapSimilarSpin(wxSpinDoubleEvent& event)
{
	double dval = m_map_similar_spin->GetValue();
	m_agent->setValue(gstSimilarity, dval);
}

void TrackDlg::OnMapSimilarText(wxCommandEvent& event)
{
	wxString str = m_map_similar_spin->GetText()->GetValue();
	double dval;
	if (str.ToDouble(&dval))
		m_agent->setValue(gstSimilarity, dval);
}

void TrackDlg::OnMapContactSpin(wxSpinDoubleEvent& event)
{
	double dval = m_map_contact_spin->GetValue();
	m_agent->setValue(gstContactFactor, dval);
}

void TrackDlg::OnMapContactText(wxCommandEvent& event)
{
	wxString str = m_map_contact_spin->GetText()->GetValue();
	double dval;
	if (str.ToDouble(&dval))
		m_agent->setValue(gstContactFactor, dval);
}

//analysis
void TrackDlg::OnConvertToRulers(wxCommandEvent& event)
{
	m_agent->ConvertToRulers();
}

void TrackDlg::OnConvertConsistent(wxCommandEvent &event)
{
	m_agent->ConvertConsistent();
}

void TrackDlg::OnAnalyzeComp(wxCommandEvent &event)
{
	m_agent->AnalyzeComp();
}

void TrackDlg::OnAnalyzeLink(wxCommandEvent &event)
{
	m_agent->AnalyzeLink();
}

void TrackDlg::OnAnalyzeUncertainHist(wxCommandEvent &event)
{
	m_agent->AnalyzeUncertainHist();
}

void TrackDlg::OnAnalyzePath(wxCommandEvent &event)
{
	m_agent->AnalyzePath();
}

void TrackDlg::OnSaveResult(wxCommandEvent &event)
{
	wxFileDialog *fopendlg = new wxFileDialog(
		m_frame, "Save results", "", "",
		"Text file (*.txt)|*.txt",
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		wxString filename = fopendlg->GetPath();
		m_agent->SaveOutputResult(filename.ToStdString());
	}
	if (fopendlg)
		delete fopendlg;
}

void TrackDlg::OnCompIDText(wxCommandEvent &event)
{
	if (event.GetId() == ID_CompIDText)
	{
		m_comp_id_text2->ChangeValue(
			m_comp_id_text->GetValue());
		m_comp_id_text2->SelectNone();
	}
	else if (event.GetId() == ID_CompIDText2)
	{
		m_comp_id_text->ChangeValue(
			m_comp_id_text2->GetValue());
		m_comp_id_text->SelectNone();
	}
}

void TrackDlg::OnCompIDXBtn(wxCommandEvent &event)
{
	m_comp_id_text->Clear();
	m_comp_id_text2->Clear();
}

void TrackDlg::OnCompClear(wxCommandEvent &event)
{
	m_compagent->CompClear();
	m_agent->CellUpdate();
}

void TrackDlg::OnShuffle(wxCommandEvent &event)
{
	m_compagent->ShuffleCurVolume();
	m_agent->CellUpdate();
}

void TrackDlg::OnCompFull(wxCommandEvent &event)
{
	m_compagent->CompFull();
	m_agent->CellUpdate();
}

void TrackDlg::OnCompAppend(wxCommandEvent &event)
{
	m_compagent->CompAppend();
	m_agent->CellUpdate();
}

void TrackDlg::OnCompExclusive(wxCommandEvent &event)
{
	m_compagent->CompExclusive();
	m_agent->CellUpdate();
}

void TrackDlg::OnCellLink(wxCommandEvent &event)
{
	m_agent->CellLink(false);
}

void TrackDlg::OnCellLinkAll(wxCommandEvent &event)
{
	m_agent->CellLinkAll();
}

void TrackDlg::OnCellExclusiveLink(wxCommandEvent &event)
{
	m_agent->CellLink(true);
}

void TrackDlg::OnCellIsolate(wxCommandEvent &event)
{
	m_agent->CellIsolate();
}

void TrackDlg::OnCellUnlink(wxCommandEvent &event)
{
	m_agent->CellUnlink();
}

//ID edit controls
void TrackDlg::OnCellNewIDText(wxCommandEvent &event)
{
	int shuffle = m_compagent->GetShuffle();

	wxString str = m_cell_new_id_text->GetValue();
	unsigned long id;
	wxColor color(255, 255, 255);
	if (str.ToULong(&id))
	{
		if (!id)
			color = wxColor(24, 167, 181);
		else
		{
			fluo::Color c(id, shuffle);
			color = wxColor(c.r() * 255, c.g() * 255, c.b() * 255);
		}
		//m_cell_new_id = id;
		//m_cell_new_id_empty = false;
	}
	m_cell_new_id_text->SetBackgroundColour(color);
	//m_cell_new_id_empty = true;
	m_cell_new_id_text->Refresh();
	m_compagent->setValue(gstCompIdStr, str.ToStdString());
	unsigned long ulval;
	if (str.ToULong(&ulval))
		m_compagent->setValue(gstCompId, ulval);
}

void TrackDlg::OnCellNewIDX(wxCommandEvent &event)
{
	m_cell_new_id_text->Clear();
	m_compagent->setValue(gstCompIdStr, std::string(""));
	m_compagent->setValue(gstCompId, (unsigned long)(0));
}

void TrackDlg::OnCellNewID(wxCommandEvent &event)
{
	m_compagent->CompNew();
}

void TrackDlg::OnCellAppendID(wxCommandEvent &event)
{
	m_compagent->CompAdd();
}

void TrackDlg::OnCellReplaceID(wxCommandEvent &event)
{
	m_compagent->CompReplace();
}

void TrackDlg::OnCellCombineID(wxCommandEvent &event)
{
	m_compagent->CompCombine();
}

void TrackDlg::OnCellSeparateID(wxCommandEvent& event)
{
	m_agent->CellSeparate();
}

void TrackDlg::OnCellSegSpin(wxSpinEvent& event)
{
	long lval = m_cell_segment_text->GetValue();
	m_agent->setValue(gstClusterNum, lval);
}

void TrackDlg::OnCellSegText(wxCommandEvent& event)
{
	long lval = m_cell_segment_text->GetValue();
	m_agent->setValue(gstClusterNum, lval);
}

void TrackDlg::OnCellSegment(wxCommandEvent& event)
{
	m_agent->CellSegment();
}

//read/delete volume cache
void TrackDlg::OnCellPrev(wxCommandEvent &event)
{
	m_movieagent->DownFrame();
}

void TrackDlg::OnCellNext(wxCommandEvent &event)
{
	m_movieagent->UpFrame();
}

