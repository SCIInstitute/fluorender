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
#include "TraceDlg.h"
#include "VRenderFrame.h"
#include "VRenderView.h"
#include "Components/CompSelector.h"
#include "Components/CompAnalyzer.h"
#include <wx/valnum.h>
#include <wx/clipbrd.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/dirdlg.h>
#include "png_resource.h"
#include "img/icons.h"
#include <boost/chrono.hpp>
#include <set>
#include <limits>

using namespace boost::chrono;

BEGIN_EVENT_TABLE(TraceListCtrl, wxListCtrl)
EVT_KEY_DOWN(TraceListCtrl::OnKeyDown)
EVT_CONTEXT_MENU(TraceListCtrl::OnContextMenu)
EVT_MENU(Menu_CopyText, TraceListCtrl::OnCopySelection)
EVT_MENU(Menu_Delete, TraceListCtrl::OnDeleteSelection)
END_EVENT_TABLE()

TraceListCtrl::TraceListCtrl(
	wxWindow* frame,
	wxWindow* parent,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size,
	long style) :
	wxListCtrl(parent, id, pos, size, style),
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

void TraceListCtrl::UpdateTraces(VRenderView* vrv)
{
	if (vrv)
		m_view = vrv;

	TraceGroup* traces = m_view->GetTraceGroup();
	if (!traces)
		return;
	int shuffle = 0;
	if (m_view->m_glview->m_cur_vol)
		shuffle = m_view->m_glview->m_cur_vol->GetShuffle();

	DeleteAllItems();

	fls::CelpList sel_cells = traces->GetCellList();
	std::vector<fls::Celp> cells;
	for (auto siter = sel_cells.begin();
	siter != sel_cells.end(); ++siter)
		cells.push_back(siter->second);

	if (cells.empty())
		return;
	else
		std::sort(cells.begin(), cells.end(), sort_cells);

	wxString gtype;
	unsigned int id;
	unsigned int vid;
	fluo::Color c;
	wxColor wxc;
	int size;
	fluo::Point center;
	bool prev, next;

	for (size_t i = 0; i < cells.size(); ++i)
	{
		id = cells[i]->Id();
		vid = cells[i]->GetVertexId();
		c = fluo::Color(id, shuffle);
		wxColor color(c.r() * 255, c.g() * 255, c.b() * 255);
		size = (int)(cells[i]->GetSizeUi());
		center = cells[i]->GetCenter();

		if (vid == 0)
			gtype = L"\u25ef";
		else
		{
			if (i == 0)
				prev = false;
			else
				prev = cells[i - 1]->GetVertexId() == vid;
			if (i == cells.size() - 1)
				next = false;
			else
				next = cells[i + 1]->GetVertexId() == vid;
			if (prev)
			{
				if (next)
					gtype = L"\u2502";
				else
					gtype = L"\u2514";
			}
			else
			{
				if (next)
					gtype = L"\u250c";
				else
					gtype = L"\u2500";
			}
		}

		Append(gtype, id, color, size,
			center.x(), center.y(), center.z());
	}
}

void TraceListCtrl::DeleteSelection()
{
	if (m_type == 0)
	{
		wxWindow* parent = GetParent();
		if (parent)
		{
			((TraceDlg*)parent)->CompDelete();
			//if (((TraceDlg*)parent)->GetManualAssist())
			//	((TraceDlg*)parent)->CellLink(true);
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
BEGIN_EVENT_TABLE(TraceDlg, wxPanel)
//map page
//load/save trace
EVT_BUTTON(ID_LoadTraceBtn, TraceDlg::OnLoadTrace)
EVT_BUTTON(ID_SaveTraceBtn, TraceDlg::OnSaveTrace)
EVT_BUTTON(ID_SaveasTraceBtn, TraceDlg::OnSaveasTrace)
//auto tracking
EVT_BUTTON(ID_GenMapBtn, TraceDlg::OnGenMapBtn)
EVT_BUTTON(ID_RefineTBtn, TraceDlg::OnRefineTBtn)
EVT_BUTTON(ID_RefineAllBtn, TraceDlg::OnRefineAllBtn)
//settings
EVT_SPINCTRL(ID_MapIterSpin, TraceDlg::OnMapIterSpin)
EVT_TEXT(ID_MapIterSpin, TraceDlg::OnMapIterText)
EVT_SPINCTRL(ID_MapSizeSpin, TraceDlg::OnMapSizeSpin)
EVT_TEXT(ID_MapSizeSpin, TraceDlg::OnMapSizeText)
EVT_TOGGLEBUTTON(ID_MapConsistentBtn, TraceDlg::OnMapConsistentBtn)
EVT_TOGGLEBUTTON(ID_MapMergeBtn, TraceDlg::OnMapMergeBtn)
EVT_TOGGLEBUTTON(ID_MapSplitBtn, TraceDlg::OnMapSplitBtn)
EVT_SPINCTRLDOUBLE(ID_MapSimilarSpin, TraceDlg::OnMapSimilarSpin)
EVT_TEXT(ID_MapSimilarSpin, TraceDlg::OnMapSimilarText)
EVT_SPINCTRLDOUBLE(ID_MapContactSpin, TraceDlg::OnMapContactSpin)
EVT_TEXT(ID_MapContactSpin, TraceDlg::OnMapContactText)
//selection page
//component tools
EVT_TEXT(ID_CompIDText, TraceDlg::OnCompIDText)
EVT_TEXT_ENTER(ID_CompIDText, TraceDlg::OnCompFull)
EVT_BUTTON(ID_CompIDXBtn, TraceDlg::OnCompIDXBtn)
EVT_BUTTON(ID_CompFullBtn, TraceDlg::OnCompFull)
EVT_BUTTON(ID_CompExclusiveBtn, TraceDlg::OnCompExclusive)
EVT_BUTTON(ID_CompAppendBtn, TraceDlg::OnCompAppend)
EVT_BUTTON(ID_CompClearBtn, TraceDlg::OnCompClear)
EVT_BUTTON(ID_ShuffleBtn, TraceDlg::OnShuffle)
//cell size filter
EVT_COMMAND_SCROLL(ID_CellSizeSldr, TraceDlg::OnCellSizeChange)
EVT_TEXT(ID_CellSizeText, TraceDlg::OnCellSizeText)
//uncertainty filter
EVT_BUTTON(ID_CompUncertainBtn, TraceDlg::OnCompUncertainBtn)
EVT_COMMAND_SCROLL(ID_CompUncertainLowSldr, TraceDlg::OnCompUncertainLowChange)
EVT_TEXT(ID_CompUncertainLowText, TraceDlg::OnCompUncertainLowText)
//link page
EVT_TEXT(ID_CompIDText2, TraceDlg::OnCompIDText)
EVT_TEXT_ENTER(ID_CompIDText2, TraceDlg::OnCompAppend)
//ID link controls
EVT_BUTTON(ID_CellExclusiveLinkBtn, TraceDlg::OnCellExclusiveLink)
EVT_BUTTON(ID_CellLinkBtn, TraceDlg::OnCellLink)
EVT_BUTTON(ID_CellLinkAllBtn, TraceDlg::OnCellLinkAll)
EVT_BUTTON(ID_CellIsolateBtn, TraceDlg::OnCellIsolate)
EVT_BUTTON(ID_CellUnlinkBtn, TraceDlg::OnCellUnlink)
//modify page
//ID edit controls
EVT_TEXT(ID_CellNewIDText, TraceDlg::OnCellNewIDText)
EVT_TEXT_ENTER(ID_CellNewIDText, TraceDlg::OnCompAppend)
EVT_BUTTON(ID_CellNewIDXBtn, TraceDlg::OnCellNewIDX)
EVT_BUTTON(ID_CompAppend2Btn, TraceDlg::OnCompAppend)
EVT_BUTTON(ID_CellNewIDBtn, TraceDlg::OnCellNewID)
EVT_BUTTON(ID_CellAppendIDBtn, TraceDlg::OnCellAppendID)
EVT_BUTTON(ID_CellReplaceIDBtn, TraceDlg::OnCellReplaceID)
EVT_BUTTON(ID_CellCombineIDBtn, TraceDlg::OnCellCombineID)
EVT_BUTTON(ID_CellSeparateBtn, TraceDlg::OnCellSeparateID)
EVT_BUTTON(ID_CellSegBtn, TraceDlg::OnCellSegment)
EVT_SPINCTRL(ID_CellSegText, TraceDlg::OnCellSegSpin)
EVT_TEXT(ID_CellSegText, TraceDlg::OnCellSegText)
//analysis page
//conversion
EVT_BUTTON(ID_ConvertToRulersBtn, TraceDlg::OnConvertToRulers)
EVT_BUTTON(ID_ConvertConsistentBtn, TraceDlg::OnConvertConsistent)
//analysis
EVT_BUTTON(ID_AnalyzeCompBtn, TraceDlg::OnAnalyzeComp)
EVT_BUTTON(ID_AnalyzeLinkBtn, TraceDlg::OnAnalyzeLink)
EVT_BUTTON(ID_AnalyzeUncertainHistBtn, TraceDlg::OnAnalyzeUncertainHist)
EVT_BUTTON(ID_AnalyzePathBtn, TraceDlg::OnAnalyzePath)
EVT_BUTTON(ID_SaveResultBtn, TraceDlg::OnSaveResult)
//ghost num
EVT_COMMAND_SCROLL(ID_GhostNumSldr, TraceDlg::OnGhostNumChange)
EVT_TEXT(ID_GhostNumText, TraceDlg::OnGhostNumText)
EVT_CHECKBOX(ID_GhostShowTailChk, TraceDlg::OnGhostShowTail)
EVT_CHECKBOX(ID_GhostShowLeadChk, TraceDlg::OnGhostShowLead)
//time controls
EVT_BUTTON(ID_CellPrevBtn, TraceDlg::OnCellPrev)
EVT_BUTTON(ID_CellNextBtn, TraceDlg::OnCellNext)
END_EVENT_TABLE()

wxWindow* TraceDlg::CreateMapPage(wxWindow *parent)
{
	wxPanel *page = new wxPanel(parent);

	wxStaticText *st = 0;

	//load trace
	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Track map:",
		wxDefaultPosition, wxSize(70, 20));
	m_load_trace_text = new wxTextCtrl(page, ID_LoadTraceText, "",
		wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_load_trace_btn = new wxButton(page, ID_LoadTraceBtn, "Load",
		wxDefaultPosition, wxSize(65, 23));
	m_save_trace_btn = new wxButton(page, ID_SaveTraceBtn, "Save",
		wxDefaultPosition, wxSize(65, 23));
	m_saveas_trace_btn = new wxButton(page, ID_SaveasTraceBtn, "Save As",
		wxDefaultPosition, wxSize(65, 23));
	sizer_1->Add(5, 5);
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	sizer_1->Add(m_load_trace_text, 1, wxEXPAND);
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

wxWindow* TraceDlg::CreateSelectPage(wxWindow *parent)
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

wxWindow* TraceDlg::CreateLinkPage(wxWindow *parent)
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

wxWindow* TraceDlg::CreateModifyPage(wxWindow *parent)
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

wxWindow* TraceDlg::CreateAnalysisPage(wxWindow *parent)
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

TraceDlg::TraceDlg(wxWindow* frame, wxWindow* parent)
	: wxPanel(parent, wxID_ANY,
		wxDefaultPosition, wxSize(550, 650),
		0, "TraceDlg"),
	m_frame(parent),
	m_view(0),
	//m_mask(0),
	m_cur_time(-1),
	m_prv_time(-1),
	m_clnum(2),
	m_iter_num(3),
	m_size_thresh(50.0),
	m_consistent_color(true),
	m_try_merge(false),
	m_try_split(false),
	m_similarity(0.2),
	m_contact_factor(0.6)
{
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
	m_trace_list_curr = new TraceListCtrl(frame, this, wxID_ANY);
	m_trace_list_curr->m_type = 0;
	m_trace_list_prev = new TraceListCtrl(frame, this, wxID_ANY);
	m_trace_list_prev->m_type = 1;
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

TraceDlg::~TraceDlg()
{
	//if (m_mask)
	//{
	//	delete[] reinterpret_cast<char*>(m_mask->data);
	//	nrrdNix(m_mask);
	//}
}

void TraceDlg::GetSettings(VRenderView* vrv)
{
	if (!vrv) return;
	m_view = vrv;
	m_cur_time = m_view->m_glview->m_tseq_cur_num;
	m_prv_time = m_view->m_glview->m_tseq_prv_num;

	TraceGroup* trace_group = m_view->GetTraceGroup();
	if (trace_group)
	{
		wxString str;
		//cell size filter
		str = m_cell_size_text->GetValue();
		unsigned long ival;
		str.ToULong(&ival);
		trace_group->SetCellSize(ival);

		str = trace_group->GetPath();
		if (str != "")
			m_load_trace_text->SetValue(str);
		else
			m_load_trace_text->SetValue("Track map created but not saved");
		UpdateList();

		int ghost_num = trace_group->GetGhostNum();
		m_ghost_num_text->ChangeValue(wxString::Format("%d", ghost_num));
		m_ghost_num_sldr->SetValue(ghost_num);
		m_ghost_show_tail_chk->SetValue(trace_group->GetDrawTail());
		m_ghost_show_lead_chk->SetValue(trace_group->GetDrawLead());
	}
	else
	{
		m_load_trace_text->SetValue("No Track map");
	}

	//settings for tracking
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetSettingDlg())
	{
		m_iter_num =
			vr_frame->GetSettingDlg()->GetTrackIter();
		m_size_thresh =
			vr_frame->GetSettingDlg()->GetComponentSize();
		m_consistent_color =
			vr_frame->GetSettingDlg()->GetConsistentColor();
		m_try_merge =
			vr_frame->GetSettingDlg()->GetTryMerge();
		m_try_split =
			vr_frame->GetSettingDlg()->GetTrySplit();
		m_similarity =
			vr_frame->GetSettingDlg()->GetSimilarity();
		m_contact_factor =
			vr_frame->GetSettingDlg()->GetContactFactor();
		//
		m_map_iter_spin->SetValue(m_iter_num);
		m_map_size_spin->SetValue(m_size_thresh);
		m_map_consistent_btn->SetValue(m_consistent_color);
		m_map_merge_btn->SetValue(m_try_merge);
		m_map_split_btn->SetValue(m_try_split);
		m_map_similar_spin->SetValue(m_similarity);
		m_map_contact_spin->SetValue(m_contact_factor);
	}
}

void TraceDlg::SetCellSize(int size)
{
	if (m_cell_size_text)
		m_cell_size_text->SetValue(wxString::Format("%d", size));

}

VRenderView* TraceDlg::GetView()
{
	return m_view;
}

void TraceDlg::UpdateList()
{
	if (!m_view) return;
	int shuffle = 0;
	if (m_view->m_glview->m_cur_vol)
		shuffle = m_view->m_glview->m_cur_vol->GetShuffle();
	TraceGroup* trace_group = m_view->GetTraceGroup();
	if (trace_group)
	{
		int cur_time = trace_group->GetCurTime();
		int prv_time = trace_group->GetPrvTime();
		wxString item_gtype;
		wxString item_id;
		wxString item_size;
		wxString item_x;
		wxString item_y;
		wxString item_z;
		unsigned long id;
		double hue;
		long size;
		double x, y, z;
		//copy current to previous
		if (cur_time != prv_time)
		{
			m_trace_list_prev->DeleteAllItems();
			long item = -1;
			for (;;)
			{
				item = m_trace_list_curr->GetNextItem(item,
					wxLIST_NEXT_ALL,
					wxLIST_STATE_DONTCARE);
				if (item != -1)
				{
					item_gtype = m_trace_list_curr->GetText(item, 0);
					item_id = m_trace_list_curr->GetText(item, 1);
					item_size = m_trace_list_curr->GetText(item, 2);
					item_x = m_trace_list_curr->GetText(item, 3);
					item_y = m_trace_list_curr->GetText(item, 4);
					item_z = m_trace_list_curr->GetText(item, 5);
					item_id.ToULong(&id);
					fluo::Color c(id, shuffle);
					wxColor color(c.r() * 255, c.g() * 255, c.b() * 255);
					item_size.ToLong(&size);
					item_x.ToDouble(&x);
					item_y.ToDouble(&y);
					item_z.ToDouble(&z);
					m_trace_list_prev->Append(item_gtype, id, color, size, x, y, z);
				}
				else break;
			}

			if (cur_time >= 0) m_cur_time = cur_time;
			if (prv_time >= 0) m_prv_time = prv_time;

			//set tiem text
			wxString str;
			str = wxString::Format("\tCurrent T: %d", m_cur_time);
			m_cell_time_curr_st->SetLabel(str);
			if (m_prv_time != m_cur_time)
				m_cell_time_prev_st->SetLabel(
					wxString::Format("\tPrevious T: %d", m_prv_time));
			else
				m_cell_time_prev_st->SetLabel("\tPrevious T");
		}
	}
	m_trace_list_curr->UpdateTraces(m_view);
	Layout();
}

void TraceDlg::OnLoadTrace(wxCommandEvent& event)
{
	if (!m_view) return;

	wxFileDialog *fopendlg = new wxFileDialog(
		m_frame, "Choose a FluoRender track file",
		"", "", "*.track", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		wxString filename = fopendlg->GetPath();
		LoadTrackFile(filename);
	}

	if (fopendlg)
		delete fopendlg;
}

void TraceDlg::OnSaveTrace(wxCommandEvent& event)
{
	if (!m_view) return;

	wxString filename;
	TraceGroup* trace_group = m_view->GetTraceGroup();
	if (trace_group)
		filename = trace_group->GetPath();
	if (wxFileExists(filename))
		m_view->SaveTraceGroup(filename);
	else
	{
		wxCommandEvent e;
		OnSaveasTrace(e);
	}
}

void TraceDlg::OnSaveasTrace(wxCommandEvent& event)
{
	if (!m_view) return;

	wxFileDialog *fopendlg = new wxFileDialog(
		m_frame, "Save a FluoRender track file",
		"", "", "*.track", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		wxString filename = fopendlg->GetPath();
		SaveTrackFile(filename);
	}

	if (fopendlg)
		delete fopendlg;
}

void TraceDlg::OnGhostNumChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	wxString str = wxString::Format("%d", ival);
	if (str != m_ghost_num_text->GetValue())
		m_ghost_num_text->SetValue(str);
}

void TraceDlg::OnGhostNumText(wxCommandEvent &event)
{
	wxString str = m_ghost_num_text->GetValue();
	long ival;
	str.ToLong(&ival);
	m_ghost_num_sldr->SetValue(ival);

	if (m_view)
	{
		TraceGroup* trace_group = m_view->GetTraceGroup();
		if (trace_group)
		{
			trace_group->SetGhostNum(ival);
			m_view->RefreshGL();
		}
	}
}

void TraceDlg::OnGhostShowTail(wxCommandEvent &event)
{
	bool show = m_ghost_show_tail_chk->GetValue();

	if (m_view)
	{
		TraceGroup* trace_group = m_view->GetTraceGroup();
		if (trace_group)
		{
			trace_group->SetDrawTail(show);
			m_view->RefreshGL();
		}
	}
}

void TraceDlg::OnGhostShowLead(wxCommandEvent &event)
{
	bool show = m_ghost_show_lead_chk->GetValue();

	if (m_view)
	{
		TraceGroup* trace_group = m_view->GetTraceGroup();
		if (trace_group)
		{
			trace_group->SetDrawLead(show);
			m_view->RefreshGL();
		}
	}
}

//cell size filter
void TraceDlg::OnCellSizeChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	wxString str = wxString::Format("%d", ival);
	if (str != m_cell_size_text->GetValue())
		m_cell_size_text->SetValue(str);
}

void TraceDlg::OnCellSizeText(wxCommandEvent &event)
{
	wxString str = m_cell_size_text->GetValue();
	long ival;
	str.ToLong(&ival);
	m_cell_size_sldr->SetValue(ival);

	if (m_view)
	{
		TraceGroup* trace_group = m_view->GetTraceGroup();
		if (trace_group)
		{
			trace_group->SetCellSize(ival);
		}
	}
}

//uncertainty filter
void TraceDlg::UncertainFilter(bool input)
{
	if (!m_view)
		return;
	//trace group
	TraceGroup *trace_group = m_view->GetTraceGroup();
	if (!trace_group)
		return;
	if (!trace_group->GetTrackMap()->GetFrameNum())
		return;
	fls::CelpList list_in, list_out;

	//fill inlist
	if (input)
	{
		long item = -1;
		while (true)
		{
			item = m_trace_list_curr->GetNextItem(
				item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
			if (item == -1)
				break;
			else
				AddLabel(item, m_trace_list_curr, list_in);
		}
		if (list_in.size() == 0)
		{
			item = -1;
			while (true)
			{
				item = m_trace_list_curr->GetNextItem(
					item, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
				if (item == -1)
					break;
				else
					AddLabel(item, m_trace_list_curr, list_in);
			}
		}
		if (list_in.empty())
			return;
	}

	fls::pTrackMap track_map = trace_group->GetTrackMap();
	fls::TrackMapProcessor tm_processor(track_map);
	wxString str = m_comp_uncertain_low_text->GetValue();
	long ival;
	str.ToLong(&ival);
	tm_processor.SetUncertainLow(ival);
	tm_processor.GetCellsByUncertainty(list_in, list_out, m_cur_time);

	VolumeData* vd = m_view->m_glview->m_cur_vol;
	fls::ComponentSelector comp_selector(vd);
	comp_selector.SelectList(list_out);

	//update view
	CellUpdate();

	//frame
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetBrushToolDlg())
		vr_frame->GetBrushToolDlg()->UpdateUndoRedo();
}

void TraceDlg::OnCompUncertainBtn(wxCommandEvent &event)
{
	UncertainFilter();
}

void TraceDlg::OnCompUncertainLowChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	wxString str = wxString::Format("%d", ival);
	if (str != m_comp_uncertain_low_text->GetValue())
		m_comp_uncertain_low_text->SetValue(str);
}

void TraceDlg::OnCompUncertainLowText(wxCommandEvent &event)
{
	wxString str = m_comp_uncertain_low_text->GetValue();
	long ival;
	str.ToLong(&ival);
	m_comp_uncertain_low_sldr->SetValue(ival);

	if (m_view)
	{
		TraceGroup* trace_group = m_view->GetTraceGroup();
		if (trace_group)
		{
			trace_group->SetUncertainLow(ival);
		}
	}

	UncertainFilter(true);
}

//auto tracking
void TraceDlg::OnGenMapBtn(wxCommandEvent &event)
{
	GenMap();
}

void TraceDlg::OnRefineTBtn(wxCommandEvent &event)
{
	RefineMap(m_cur_time);
}

void TraceDlg::OnRefineAllBtn(wxCommandEvent &event)
{
	RefineMap();
}

//settings
void TraceDlg::OnMapIterSpin(wxSpinEvent& event)
{
	m_iter_num = m_map_iter_spin->GetValue();
	//save settings
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetSettingDlg())
		vr_frame->GetSettingDlg()->SetTrackIter(m_iter_num);
}

void TraceDlg::OnMapIterText(wxCommandEvent& event)
{
	m_iter_num = m_map_iter_spin->GetValue();
	//save settings
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetSettingDlg())
		vr_frame->GetSettingDlg()->SetTrackIter(m_iter_num);
}

void TraceDlg::OnMapSizeSpin(wxSpinEvent& event)
{
	m_size_thresh = m_map_size_spin->GetValue();
	//save settings
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetSettingDlg())
		vr_frame->GetSettingDlg()->SetComponentSize(m_size_thresh);
}

void TraceDlg::OnMapSizeText(wxCommandEvent& event)
{
	m_size_thresh = m_map_size_spin->GetValue();
	//save settings
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetSettingDlg())
		vr_frame->GetSettingDlg()->SetComponentSize(m_size_thresh);
}

void TraceDlg::OnMapConsistentBtn(wxCommandEvent& event)
{
	m_consistent_color = m_map_consistent_btn->GetValue();
	//save settings
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetSettingDlg())
		vr_frame->GetSettingDlg()->SetConsistentColor(m_consistent_color);
}

void TraceDlg::OnMapMergeBtn(wxCommandEvent& event)
{
	m_try_merge = m_map_merge_btn->GetValue();
	//save settings
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetSettingDlg())
		vr_frame->GetSettingDlg()->SetTryMerge(m_try_merge);
}

void TraceDlg::OnMapSplitBtn(wxCommandEvent& event)
{
	m_try_split = m_map_split_btn->GetValue();
	//save settings
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetSettingDlg())
		vr_frame->GetSettingDlg()->SetTrySplit(m_try_split);
}

void TraceDlg::OnMapSimilarSpin(wxSpinDoubleEvent& event)
{
	m_similarity = m_map_similar_spin->GetValue();
	//save settings
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetSettingDlg())
		vr_frame->GetSettingDlg()->SetSimilarity(m_similarity);
}

void TraceDlg::OnMapSimilarText(wxCommandEvent& event)
{
	wxString str = m_map_similar_spin->GetText()->GetValue();
	double dval;
	if (str.ToDouble(&dval))
	{
		m_similarity = dval;
		//save settings
		VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
		if (vr_frame && vr_frame->GetSettingDlg())
			vr_frame->GetSettingDlg()->SetSimilarity(m_similarity);
	}
}

void TraceDlg::OnMapContactSpin(wxSpinDoubleEvent& event)
{
	m_contact_factor = m_map_contact_spin->GetValue();
	//save settings
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetSettingDlg())
		vr_frame->GetSettingDlg()->SetContactFactor(m_contact_factor);
}

void TraceDlg::OnMapContactText(wxCommandEvent& event)
{
	wxString str = m_map_contact_spin->GetText()->GetValue();
	double dval;
	if (str.ToDouble(&dval))
	{
		m_contact_factor = dval;
		//save settings
		VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
		if (vr_frame && vr_frame->GetSettingDlg())
			vr_frame->GetSettingDlg()->SetContactFactor(m_contact_factor);
	}
}

//analysis
void TraceDlg::OnConvertToRulers(wxCommandEvent& event)
{
	if (!m_view)
		return;

	TraceGroup* trace_group = m_view->GetTraceGroup();
	if (!trace_group)
		return;
	VolumeData* vd = m_view->m_glview->m_cur_vol;
	if (!vd)
		return;
	double spcx, spcy, spcz;
	vd->GetSpacings(spcx, spcy, spcz);

	//get rulers
	fls::RulerList rulers;
	trace_group->GetMappedRulers(rulers);
	for (auto iter = rulers.begin();
	iter != rulers.end(); ++iter)
	{
		(*iter)->Scale(spcx, spcy, spcz);
		m_view->GetRulerList()->push_back(*iter);
	}
	m_view->RefreshGL();
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetMeasureDlg())
		vr_frame->GetMeasureDlg()->GetSettings(m_view);
	flvr::TextureRenderer::vertex_array_manager_.set_dirty(flvr::VA_Rulers);
}

void TraceDlg::OnConvertConsistent(wxCommandEvent &event)
{
	if (!m_view)
		return;

	VolumeData* vd = m_view->m_glview->m_cur_vol;
	if (!vd)
		return;
	TraceGroup *trace_group = m_view->GetTraceGroup();
	if (!trace_group)
		return;

	m_stat_text->SetValue("Generating consistent IDs in");
	wxGetApp().Yield();

	fls::pTrackMap track_map = trace_group->GetTrackMap();
	fls::TrackMapProcessor tm_processor(track_map);
	int resx, resy, resz;
	vd->GetResolution(resx, resy, resz);
	double spcx, spcy, spcz;
	vd->GetSpacings(spcx, spcy, spcz);
	tm_processor.SetBits(vd->GetBits());
	tm_processor.SetScale(vd->GetScalarScale());
	tm_processor.SetSizes(resx, resy, resz);
	tm_processor.SetSpacings(spcx, spcy, spcz);
	tm_processor.RegisterCacheQueueFuncs(
		std::bind(&TraceDlg::ReadVolCache, this, std::placeholders::_1),
		std::bind(&TraceDlg::DelVolCache, this, std::placeholders::_1));
	tm_processor.SetVolCacheSize(2);

	(*m_stat_text) << wxString::Format("Frame %d\n", 0);
	wxGetApp().Yield();
	tm_processor.MakeConsistent(0);

	//remaining frames
	for (size_t fi = 1; fi < track_map->GetFrameNum(); ++fi)
	{
		(*m_stat_text) << wxString::Format("Frame %d\n", int(fi));
		wxGetApp().Yield();
		tm_processor.MakeConsistent(fi - 1, fi);
	}

	CellUpdate();
}

void TraceDlg::OnAnalyzeComp(wxCommandEvent &event)
{
	if (!m_view)
		return;
	VolumeData* vd = m_view->m_glview->m_cur_vol;
	fls::ComponentAnalyzer comp_analyzer(vd);
	comp_analyzer.Analyze(true, true);
	string str;
	comp_analyzer.OutputCompListStr(str, 1);
	m_stat_text->SetValue(str);
}

void TraceDlg::OnAnalyzeLink(wxCommandEvent &event)
{
	if (!m_view)
		return;

	TraceGroup* trace_group = m_view->GetTraceGroup();
	if (!trace_group)
		return;
	size_t frames = trace_group->GetTrackMap()->GetFrameNum();
	if (frames == 0)
		m_stat_text->SetValue("ERROR! Generate a track map first.\n");
	else
		m_stat_text->SetValue(
			wxString::Format("Time point number: %d\n", int(frames)));

	(*m_stat_text) << "Time\tIn Orphan\tOut Orphan\tIn Multi\tOut Multi\n";
	fls::VertexList in_orphan_list;
	fls::VertexList out_orphan_list;
	fls::VertexList in_multi_list;
	fls::VertexList out_multi_list;
	for (size_t fi = 0; fi < frames; ++fi)
	{
		trace_group->GetLinkLists(fi,
			in_orphan_list, out_orphan_list,
			in_multi_list, out_multi_list);
		(*m_stat_text) << int(fi) << "\t" <<
			int(in_orphan_list.size()) << "\t" <<
			int(out_orphan_list.size()) << "\t" <<
			int(in_multi_list.size()) << "\t" <<
			int(out_multi_list.size()) << "\n";
	}
}

void TraceDlg::OnAnalyzeUncertainHist(wxCommandEvent &event)
{
	if (!m_view)
		return;
	//trace group
	TraceGroup *trace_group = m_view->GetTraceGroup();
	if (!trace_group)
		return;
	if (!trace_group->GetTrackMap()->GetFrameNum())
		return;
	fls::CelpList list_in;
	//fill inlist
	long item = -1;
	while (true)
	{
		item = m_trace_list_curr->GetNextItem(
			item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (item == -1)
			break;
		else
			AddLabel(item, m_trace_list_curr, list_in);
	}
	if (list_in.size() == 0)
	{
		item = -1;
		while (true)
		{
			item = m_trace_list_curr->GetNextItem(
				item, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
			if (item == -1)
				break;
			else
				AddLabel(item, m_trace_list_curr, list_in);
		}
	}

	m_stat_text->SetValue("");

	fls::pTrackMap track_map = trace_group->GetTrackMap();
	fls::TrackMapProcessor tm_processor(track_map);
	if (list_in.empty())
	{
		fls::UncertainHist hist1, hist2;
		tm_processor.GetUncertainHist(hist1, hist2, m_cur_time);
		//header
		(*m_stat_text) << "In\n";
		(*m_stat_text) << "Level\t" << "Frequency\n";
		int count = 0;
		for (auto iter = hist1.begin();
			iter != hist1.end(); ++iter)
		{
			while (iter->second.level > count)
			{
				(*m_stat_text) << count++ << "\t" << "0\n";
			}
			(*m_stat_text) << int(iter->second.level) << "\t" <<
				int(iter->second.count) << "\n";
			count++;
		}

		//header
		(*m_stat_text) << "\n";
		(*m_stat_text) << "Out\n";
		(*m_stat_text) << "Level\t" << "Frequency\n";
		count = 0;
		for (auto iter = hist2.begin();
		iter != hist2.end(); ++iter)
		{
			while (iter->second.level > count)
			{
				(*m_stat_text) << count++ << "\t" << "0\n";
			}
			(*m_stat_text) << int(iter->second.level) << "\t" <<
				int(iter->second.count) << "\n";
			count++;
		}
	}
	else
	{
		tm_processor.GetCellUncertainty(list_in, m_cur_time);
		//header
		(*m_stat_text) << "ID\t" << "In\t" << "Out\n";
		for (auto iter = list_in.begin();
			iter != list_in.end(); ++iter)
		{
			wxString sid = wxString::Format("%u", iter->second->Id());
			(*m_stat_text) << sid << "\t" <<
				int(iter->second->GetCount0()) << "\t" <<
				int(iter->second->GetCount1()) << "\n";
		}
	}
}

void TraceDlg::OnAnalyzePath(wxCommandEvent &event)
{
	if (!m_view)
		return;
	//trace group
	TraceGroup *trace_group = m_view->GetTraceGroup();
	if (!trace_group)
		return;
	if (!trace_group->GetTrackMap()->GetFrameNum())
		return;
	fls::CelpList list_in;
	//fill inlist
	long item = -1;
	while (true)
	{
		item = m_trace_list_curr->GetNextItem(
			item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (item == -1)
			break;
		else
			AddLabel(item, m_trace_list_curr, list_in);
	}
	if (list_in.size() == 0)
	{
		item = -1;
		while (true)
		{
			item = m_trace_list_curr->GetNextItem(
				item, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
			if (item == -1)
				break;
			else
				AddLabel(item, m_trace_list_curr, list_in);
		}
	}

	m_stat_text->SetValue("");

	fls::pTrackMap track_map = trace_group->GetTrackMap();
	fls::TrackMapProcessor tm_processor(track_map);
	if (list_in.empty())
		return;

	m_stat_text->SetValue("");
	std::ostream os(m_stat_text);

	if (m_cur_time > 0)
	{
		(*m_stat_text) << "Paths of T" << m_cur_time << " to T" << m_cur_time - 1 << ":\n";
		fls::PathList paths_prv;
		tm_processor.GetPaths(list_in, paths_prv, m_cur_time, m_cur_time - 1);
		for (size_t i = 0; i < paths_prv.size(); ++i)
			os << paths_prv[i];
	}
	if (m_cur_time < track_map->GetFrameNum() - 1)
	{
		(*m_stat_text) << "Paths of T" << m_cur_time << " to T" << m_cur_time + 1 << ":\n";
		fls::PathList paths_nxt;
		tm_processor.GetPaths(list_in, paths_nxt, m_cur_time, m_cur_time + 1);
		for (size_t i = 0; i < paths_nxt.size(); ++i)
			os << paths_nxt[i];
	}
}

void TraceDlg::OnSaveResult(wxCommandEvent &event)
{
	wxFileDialog *fopendlg = new wxFileDialog(
		m_frame, "Save results", "", "",
		"Text file (*.txt)|*.txt",
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		wxString filename = fopendlg->GetPath();
		SaveOutputResult(filename);
	}
	if (fopendlg)
		delete fopendlg;
}

//Component tools
void TraceDlg::CompDelete()
{
	if (!m_view)
		return;

	long item = -1;
	wxString str;
	unsigned long ival;
	vector<unsigned int> ids;
	for (;;)
	{
		item = m_trace_list_curr->GetNextItem(item,
			wxLIST_NEXT_ALL,
			wxLIST_STATE_DONTCARE);

		if (item == -1)
			break;
		else if (m_trace_list_curr->
			GetItemState(item, wxLIST_STATE_SELECTED)
			== wxLIST_STATE_SELECTED)
			continue;
		else
		{
			str = m_trace_list_curr->GetItemText(item, 1);
			if (str.ToULong(&ival) && ival)
				ids.push_back(ival);
		}
	}

	//get current vd
	VolumeData* vd = m_view->m_glview->m_cur_vol;
	fls::ComponentSelector comp_selector(vd);
	if (ids.size() == 1)
	{
		comp_selector.SetId(ids[0]);
		comp_selector.Delete();
	}
	else
		comp_selector.Delete(ids);

	//update view
	CellUpdate();

	//frame
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetBrushToolDlg())
		vr_frame->GetBrushToolDlg()->UpdateUndoRedo();
}

void TraceDlg::CompClear()
{
	if (!m_view)
		return;

	//get current vd
	VolumeData* vd = m_view->m_glview->m_cur_vol;
	fls::ComponentSelector comp_selector(vd);
	comp_selector.Clear();

	//update view
	CellUpdate();
	m_trace_list_prev->DeleteAllItems();

	//frame
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetBrushToolDlg())
		vr_frame->GetBrushToolDlg()->UpdateUndoRedo();
}

void TraceDlg::OnCompIDText(wxCommandEvent &event)
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

void TraceDlg::OnCompIDXBtn(wxCommandEvent &event)
{
	m_comp_id_text->Clear();
	m_comp_id_text2->Clear();
}

void TraceDlg::OnCompClear(wxCommandEvent &event)
{
	CompClear();
}

void TraceDlg::OnShuffle(wxCommandEvent &event)
{
	if (!m_view)
		return;

	//get current vd
	VolumeData* vd = m_view->m_glview->m_cur_vol;
	if (!vd)
		return;

	vd->IncShuffle();
	m_view->RefreshGL();
}

void TraceDlg::OnCompFull(wxCommandEvent &event)
{
	//get id
	wxString str = m_comp_id_text->GetValue();
	if (str.empty())
		CellFull();
	else
	{
		wxCommandEvent e;
		OnCompAppend(e);
	}
}

void TraceDlg::OnCompAppend(wxCommandEvent &event)
{
	if (!m_view)
		return;

	wxString str;
	//cell size filter
	str = m_cell_size_text->GetValue();
	unsigned long ival;
	str.ToULong(&ival);
	unsigned int slimit = (unsigned int)ival;

	//get id
	if (event.GetId() == ID_CompAppend2Btn ||
		event.GetId() == ID_CellNewIDText)
		str = m_cell_new_id_text->GetValue();
	else
		str = m_comp_id_text->GetValue();

	if (!str.empty())
	{
		bool get_all = false;
		ival = 0;
		if (str.Lower() == "all")
			get_all = true;
		else
		{
			str.ToULong(&ival);
			if (ival == 0)
				return;
		}

		unsigned int id = (unsigned int)ival;
		//get current mask
		VolumeData* vd = m_view->m_glview->m_cur_vol;
		fls::ComponentSelector comp_selector(vd);
		comp_selector.SetId(id);
		comp_selector.SetMinNum(true, slimit);
		comp_selector.Select(get_all);
	}

	//update view
	CellUpdate();

	//frame
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetBrushToolDlg())
		vr_frame->GetBrushToolDlg()->UpdateUndoRedo();
}

void TraceDlg::OnCompExclusive(wxCommandEvent &event)
{
	if (!m_view)
		return;

	wxString str;
	//cell size filter
	str = m_cell_size_text->GetValue();
	unsigned long ival;
	str.ToULong(&ival);
	unsigned int slimit = (unsigned int)ival;

	//get id
	str = m_comp_id_text->GetValue();
	str.ToULong(&ival);
	if (ival != 0)
	{
		unsigned int id = ival;
		//get current mask
		VolumeData* vd = m_view->m_glview->m_cur_vol;
		fls::ComponentSelector comp_selector(vd);
		comp_selector.SetId(id);
		comp_selector.SetMinNum(true, slimit);
		comp_selector.Exclusive();
	}

	//update view
	CellUpdate();

	//frame
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetBrushToolDlg())
		vr_frame->GetBrushToolDlg()->UpdateUndoRedo();
}

//ID link controls
void TraceDlg::CellUpdate()
{
	if (m_view)
	{
		wxString str = m_ghost_num_text->GetValue();
		long ival;
		str.ToLong(&ival);
		TraceGroup* trace_group = m_view->GetTraceGroup();
		if (trace_group)
			trace_group->SetGhostNum(ival);
		else
			m_view->CreateTraceGroup();

		m_view->m_glview->GetTraces(false);
		m_view->RefreshGL();
		GetSettings(m_view);
	}
}

void TraceDlg::CellFull()
{
	if (!m_view)
		return;

	//cell size filter
	wxString str = m_cell_size_text->GetValue();
	long ival;
	str.ToLong(&ival);
	unsigned int slimit = (unsigned int)ival;
	//get current mask
	VolumeData* vd = m_view->m_glview->m_cur_vol;
	fls::ComponentSelector comp_selector(vd);
	comp_selector.SetMinNum(true, slimit);
	comp_selector.CompFull();
	//update view
	CellUpdate();

	//frame
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetBrushToolDlg())
		vr_frame->GetBrushToolDlg()->UpdateUndoRedo();
}

void TraceDlg::AddLabel(long item, TraceListCtrl* trace_list_ctrl, fls::CelpList &list)
{
	wxString str;
	unsigned long id;
	unsigned long size;
	double x, y, z;

	str = trace_list_ctrl->GetText(item, 1);
	str.ToULong(&id);
	str = trace_list_ctrl->GetText(item, 2);
	str.ToULong(&size);
	str = trace_list_ctrl->GetText(item, 3);
	str.ToDouble(&x);
	str = trace_list_ctrl->GetText(item, 4);
	str.ToDouble(&y);
	str = trace_list_ctrl->GetText(item, 5);
	str.ToDouble(&z);

	fls::Celp cell(new fls::Cell(id));
	cell->SetSizeUi(size);
	cell->SetSizeD(size);
	fluo::Point p(x, y, z);
	cell->SetCenter(p);
	list.insert(std::pair<unsigned int, fls::Celp>
		(id, cell));
}

void TraceDlg::CellNewID(bool append)
{
	if (!m_view)
		return;

	//trace group
	TraceGroup *trace_group = m_view->GetTraceGroup();
	if (!trace_group)
	{
		m_view->CreateTraceGroup();
		trace_group = m_view->GetTraceGroup();
	}

	VolumeData* vd = m_view->m_glview->m_cur_vol;
	if (!vd)
		return;
	Nrrd* nrrd_mask = 0;
	//get current mask
	nrrd_mask = vd->GetMask(true);
	if (!nrrd_mask)
	{
		vd->AddEmptyMask(0);
		nrrd_mask = vd->GetMask(false);
	}
	unsigned char* data_mask = (unsigned char*)(nrrd_mask->data);
	if (!data_mask)
		return;

	//get current label
	flvr::Texture* tex = vd->GetTexture();
	if (!tex)
		return;
	Nrrd* nrrd_label = tex->get_nrrd(tex->nlabel());
	if (!nrrd_label)
	{
		vd->AddEmptyLabel();
		nrrd_label = tex->get_nrrd(tex->nlabel());
	}
	unsigned int* data_label = (unsigned int*)(nrrd_label->data);
	if (!data_label)
		return;

	int nx, ny, nz;
	vd->GetResolution(nx, ny, nz);
	unsigned long long for_size = (unsigned long long)nx *
		(unsigned long long)ny * (unsigned long long)nz;
	unsigned long long index;

	//get ID of currently/previously masked region
	unsigned long id_str;
	unsigned long id_vol = 0;
	bool id_str_empty = false;
	wxString str = m_cell_new_id_text->GetValue();
	if (!str.ToULong(&id_str))
	{
		id_str_empty = true;
		for (index = 0; index < for_size; ++index)
		{
			if (data_mask[index] &&
				data_label[index])
			{
				id_vol = data_label[index];
				break;
			}
		}
	}

	//generate a unique ID
	unsigned int new_id = 0;
	unsigned int inc = 0;
	if (id_str_empty)
	{
		if (id_vol)
		{
			new_id = id_vol + 10;
			inc = 10;
		}
		else
		{
			new_id = 10;
			inc = 10;
		}
	}
	else
	{
		if (id_str)
		{
			new_id = id_str;
			inc = 253;
		}
		else
		{
			new_id = 0;
			inc = 0;
		}
	}
	unsigned int stop_id = new_id;
	if (inc)
	{
		while (vd->SearchLabel(new_id))
		{
			new_id += inc;
			if (new_id == stop_id)
			{
				(*m_stat_text) << wxString::Format(
					"ID assignment failed. Type a different ID than %d",
					stop_id);
				return;
			}
		}
	}

	//update label volume, set mask region to the new ID
	int i, j, k;
	fls::Celp cell;
	if (new_id)
		cell = fls::Celp(new fls::Cell(new_id));
	for (i = 0; i < nx; ++i)
	for (j = 0; j < ny; ++j)
	for (k = 0; k < nz; ++k)
	{
		index = nx*ny*k + nx*j + i;
		if (data_mask[index])
		{
			if (append && data_label[index])
				continue;
			data_label[index] = new_id;
			if (new_id)
				cell->Inc(i, j, k, 1.0f);
		}
	}

	//save label mask to disk
	vd->SaveLabel(true, m_cur_time, vd->GetCurChannel());

	if (new_id)
	{
		//trace_group->AddCell(cell, m_cur_time);
		fls::pTrackMap track_map = trace_group->GetTrackMap();
		fls::TrackMapProcessor tm_processor(track_map);
		//register file reading and deleteing functions
		tm_processor.RegisterCacheQueueFuncs(
			std::bind(&TraceDlg::ReadVolCache, this, std::placeholders::_1),
			std::bind(&TraceDlg::DelVolCache, this, std::placeholders::_1));
		tm_processor.SetVolCacheSize(4);
		//add
		tm_processor.AddCellDup(cell, m_cur_time);
	}
	CellUpdate();

	//invalidate label mask in gpu
	vd->GetVR()->clear_tex_current();

}

void TraceDlg::CellEraseID()
{
	if (!m_view)
		return;

	//trace group
	TraceGroup *trace_group = m_view->GetTraceGroup();
	if (!trace_group)
	{
		m_view->CreateTraceGroup();
		trace_group = m_view->GetTraceGroup();
	}

	VolumeData* vd = m_view->m_glview->m_cur_vol;
	if (!vd)
		return;
	//get prev mask
	Nrrd* nrrd_mask = vd->GetMask(false);
	if (!nrrd_mask)
		return;
	unsigned char* data_mask = (unsigned char*)(nrrd_mask->data);
	if (!data_mask)
		return;

	//get prev label
	Nrrd* nrrd_label = vd->GetLabel(true);
	if (!nrrd_label)
		return;
	unsigned int* data_label = (unsigned int*)(nrrd_label->data);
	if (!data_label)
		return;

	int nx, ny, nz;
	vd->GetResolution(nx, ny, nz);
	unsigned long long for_size = (unsigned long long)nx *
		(unsigned long long)ny * (unsigned long long)nz;
	unsigned long long index;

	//find old IDs
	set<unsigned int> id_list;
	for (index = 0; index < for_size; ++index)
	{
		if (data_mask[index] && data_label[index])
			id_list.insert(data_label[index]);
	}

	if (!id_list.empty())
	{
		//current mask
		nrrd_mask = vd->GetMask(true);
		if (!nrrd_mask)
			return;
		data_mask = (unsigned char*)(nrrd_mask->data);
		if (!data_mask)
			return;

		for (index = 0; index < for_size; ++index)
		{
			if (data_label[index] &&
				id_list.find(data_label[index])
				!= id_list.end() &&
				!data_mask[index])
				data_label[index] = 0;
		}

		//invalidate label mask in gpu
		vd->GetVR()->clear_tex_current();
		//save label mask to disk
		vd->SaveLabel(true, m_cur_time, vd->GetCurChannel());
	}

	CellUpdate();
}

void TraceDlg::CellLink(bool exclusive)
{
	if (!m_view)
		return;

	TraceGroup* trace_group = m_view->GetTraceGroup();
	if (!trace_group)
	{
		m_view->CreateTraceGroup();
		trace_group = m_view->GetTraceGroup();
	}

	//get selections
	long item;
	//current T
	fls::CelpList list_cur;
	//previous T
	fls::CelpList list_prv;
	//current list
	item = -1;
	while (true)
	{
		item = m_trace_list_curr->GetNextItem(
			item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (item == -1)
			break;
		else
			AddLabel(item, m_trace_list_curr, list_cur);
	}
	if (list_cur.size() == 0)
	{
		item = -1;
		while (true)
		{
			item = m_trace_list_curr->GetNextItem(
				item, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
			if (item == -1)
				break;
			else
				AddLabel(item, m_trace_list_curr, list_cur);
		}
	}
	//previous list
	item = -1;
	while (true)
	{
		item = m_trace_list_prev->GetNextItem(
			item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (item == -1)
			break;
		else
			AddLabel(item, m_trace_list_prev, list_prv);
	}
	if (list_prv.size() == 0)
	{
		item = -1;
		while (true)
		{
			item = m_trace_list_prev->GetNextItem(
				item, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
			if (item == -1)
				break;
			else
				AddLabel(item, m_trace_list_prev, list_prv);
		}
	}
	if (list_cur.size() == 0 ||
		list_prv.size() == 0)
		return;

	//link them
	trace_group->LinkCells(list_cur, list_prv,
		m_cur_time, m_prv_time, exclusive);

	//update view
	m_view->RefreshGL();
}

void TraceDlg::OnCellLink(wxCommandEvent &event)
{
	CellLink(false);
}

void TraceDlg::OnCellLinkAll(wxCommandEvent &event)
{
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (!vr_frame || !vr_frame->GetComponentDlg())
		return;
	if (!m_view)
		return;
	TraceGroup *trace_group = m_view->GetTraceGroup();
	if (!trace_group)
		return;

	fls::pTrackMap track_map = trace_group->GetTrackMap();
	fls::TrackMapProcessor tm_processor(track_map);
	//register file reading and deleteing functions
	tm_processor.RegisterCacheQueueFuncs(
		std::bind(&TraceDlg::ReadVolCache, this, std::placeholders::_1),
		std::bind(&TraceDlg::DelVolCache, this, std::placeholders::_1));
	tm_processor.SetVolCacheSize(3);
	fls::CelpList in = vr_frame->GetComponentDlg()->GetInCells();
	fls::CelpList out = vr_frame->GetComponentDlg()->GetOutCells();
	tm_processor.RelinkCells(in, out, m_cur_time);

	CellUpdate();
}

void TraceDlg::OnCellExclusiveLink(wxCommandEvent &event)
{
	CellLink(true);
}

void TraceDlg::OnCellIsolate(wxCommandEvent &event)
{
	if (!m_view)
		return;

	TraceGroup* trace_group = m_view->GetTraceGroup();
	if (!trace_group)
		return;

	//get selections
	long item;
	//current T
	fls::CelpList list_cur;

	//current list
	item = -1;
	while (true)
	{
		item = m_trace_list_curr->GetNextItem(
			item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (item == -1)
			break;
		else
			AddLabel(item, m_trace_list_curr, list_cur);
	}
	if (list_cur.size() == 0)
	{
		item = -1;
		while (true)
		{
			item = m_trace_list_curr->GetNextItem(
				item, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
			if (item == -1)
				break;
			else
				AddLabel(item, m_trace_list_curr, list_cur);
		}
	}

	if (list_cur.size() == 0)
		return;

	//isolate
	trace_group->IsolateCells(list_cur, m_cur_time);

	//update view
	m_view->RefreshGL();
}

void TraceDlg::OnCellUnlink(wxCommandEvent &event)
{
	if (!m_view)
		return;

	TraceGroup* trace_group = m_view->GetTraceGroup();
	if (!trace_group)
		return;

	//get selections
	long item;
	//current T
	fls::CelpList list_cur;
	//previous T
	fls::CelpList list_prv;
	//current list
	item = -1;
	while (true)
	{
		item = m_trace_list_curr->GetNextItem(
			item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (item == -1)
			break;
		else
			AddLabel(item, m_trace_list_curr, list_cur);
	}
	if (list_cur.size() == 0)
	{
		item = -1;
		while (true)
		{
			item = m_trace_list_curr->GetNextItem(
				item, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
			if (item == -1)
				break;
			else
				AddLabel(item, m_trace_list_curr, list_cur);
		}
	}
	//previous list
	item = -1;
	while (true)
	{
		item = m_trace_list_prev->GetNextItem(
			item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (item == -1)
			break;
		else
			AddLabel(item, m_trace_list_prev, list_prv);
	}
	if (list_prv.size() == 0)
	{
		item = -1;
		while (true)
		{
			item = m_trace_list_prev->GetNextItem(
				item, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
			if (item == -1)
				break;
			else
				AddLabel(item, m_trace_list_prev, list_prv);
		}
	}
	if (list_cur.size() == 0 ||
		list_prv.size() == 0)
		return;

	//unlink them
	trace_group->UnlinkCells(list_cur, list_prv,
		m_cur_time, m_prv_time);

	//update view
	m_view->RefreshGL();
}

//ID edit controls
void TraceDlg::OnCellNewIDText(wxCommandEvent &event)
{
	int shuffle = 0;
	if (m_view && m_view->m_glview->m_cur_vol)
		shuffle = m_view->m_glview->m_cur_vol->GetShuffle();
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
		m_cell_new_id_text->SetBackgroundColour(color);
	}
	else
		m_cell_new_id_text->SetBackgroundColour(color);
	m_cell_new_id_text->Refresh();
}

void TraceDlg::OnCellNewIDX(wxCommandEvent &event)
{
	m_cell_new_id_text->Clear();
}

void TraceDlg::OnCellNewID(wxCommandEvent &event)
{
	CellNewID(false);
}

void TraceDlg::OnCellAppendID(wxCommandEvent &event)
{
	CellNewID(true);
}

void TraceDlg::OnCellReplaceID(wxCommandEvent &event)
{
	if (!m_view)
		return;

	//get id
	wxString str = m_cell_new_id_text->GetValue();
	unsigned long id;
	if (!str.ToULong(&id))
		return;
	if (!id)
		return;

	//trace group
	TraceGroup *trace_group = m_view->GetTraceGroup();
	bool track_map = trace_group && trace_group->GetTrackMap()->GetFrameNum();

	//current T
	fls::CelpList list_cur;
	fls::CelpListIter cell_iter;
	//fill current list
	long item = -1;
	while (true)
	{
		item = m_trace_list_curr->GetNextItem(
			item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (item == -1)
			break;
		else
			AddLabel(item, m_trace_list_curr, list_cur);
	}
	if (list_cur.empty())
	{
		item = -1;
		while (true)
		{
			item = m_trace_list_curr->GetNextItem(
				item, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
			if (item == -1)
				break;
			else
				AddLabel(item, m_trace_list_curr, list_cur);
		}
	}

	//get current mask
	VolumeData* vd = m_view->m_glview->m_cur_vol;
	if (!vd)
		return;
	Nrrd* nrrd_mask = vd->GetMask(true);
	if (!nrrd_mask)
		return;
	unsigned char* data_mask = (unsigned char*)(nrrd_mask->data);
	if (!data_mask)
		return;
	//get current label
	flvr::Texture* tex = vd->GetTexture();
	if (!tex)
		return;
	Nrrd* nrrd_label = tex->get_nrrd(tex->nlabel());
	if (!nrrd_label)
		return;
	unsigned int* data_label = (unsigned int*)(nrrd_label->data);
	if (!data_label)
		return;

	//replace ID
	std::unordered_map<unsigned int, unsigned int> list_rep;
	std::unordered_map<unsigned int, unsigned int>::iterator list_rep_iter;
	unsigned int old_id, new_id;
	int nx, ny, nz;
	vd->GetResolution(nx, ny, nz);
	unsigned long long index;
	unsigned long long for_size = (unsigned long long)nx *
		(unsigned long long)ny * (unsigned long long)nz;
	for (index = 0; index < for_size; ++index)
	{
		old_id = data_label[index];
		if (!data_mask[index] ||
			!old_id ||
			old_id == id)
			continue;

		list_rep_iter = list_rep.find(old_id);
		if (list_rep_iter != list_rep.end())
		{
			data_label[index] = list_rep_iter->second;
			continue;
		}

		cell_iter = list_cur.find(old_id);
		if (cell_iter != list_cur.end())
		{
			new_id = id;
			while (vd->SearchLabel(new_id))
				new_id += 253;
			//add cell to list_rep
			list_rep.insert(pair<unsigned int, unsigned int>
				(old_id, new_id));
			if (track_map)
				trace_group->ReplaceCellID(old_id, new_id,
					m_cur_time);
			data_label[index] = new_id;
		}
	}
	//invalidate label mask in gpu
	vd->GetVR()->clear_tex_current();
	//save label mask to disk
	vd->SaveLabel(true, m_cur_time, vd->GetCurChannel());

	CellUpdate();
}

void TraceDlg::OnCellCombineID(wxCommandEvent &event)
{
	if (!m_view)
		return;

	//trace group
	TraceGroup *trace_group = m_view->GetTraceGroup();
	if (!trace_group)
		return;

	//current T
	fls::CelpList list_cur;
	//fill current list
	long item = -1;
	while (true)
	{
		item = m_trace_list_curr->GetNextItem(
			item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (item == -1)
			break;
		else
			AddLabel(item, m_trace_list_curr, list_cur);
	}
	if (list_cur.empty())
	{
		item = -1;
		while (true)
		{
			item = m_trace_list_curr->GetNextItem(
				item, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
			if (item == -1)
				break;
			else
				AddLabel(item, m_trace_list_curr, list_cur);
		}
	}
	if (list_cur.size() <= 1)
		//nothing to combine
		return;

	//find the largest cell in the list
	fls::Celp cell;
	fls::CelpListIter cell_iter;
	for (cell_iter = list_cur.begin();
	cell_iter != list_cur.end(); ++cell_iter)
	{
		if (cell)
		{
			if (cell_iter->second->GetSizeUi() >
				cell->GetSizeUi())
				cell = cell_iter->second;
		}
		else
			cell = cell_iter->second;
	}
	if (!cell)
		return;

	//get current mask
	VolumeData* vd = m_view->m_glview->m_cur_vol;
	if (!vd)
		return;
	Nrrd* nrrd_mask = vd->GetMask(true);
	if (!nrrd_mask)
		return;
	unsigned char* data_mask = (unsigned char*)(nrrd_mask->data);
	if (!data_mask)
		return;
	//get current label
	flvr::Texture* tex = vd->GetTexture();
	if (!tex)
		return;
	Nrrd* nrrd_label = tex->get_nrrd(tex->nlabel());
	if (!nrrd_label)
		return;
	unsigned int* data_label = (unsigned int*)(nrrd_label->data);
	if (!data_label)
		return;
	//combine IDs
	int nx, ny, nz;
	vd->GetResolution(nx, ny, nz);
	unsigned long long index;
	unsigned long long for_size = (unsigned long long)nx *
		(unsigned long long)ny * (unsigned long long)nz;
	for (index = 0; index < for_size; ++index)
	{
		if (!data_mask[index] ||
			!data_label[index])
			continue;
		cell_iter = list_cur.find(data_label[index]);
		if (cell_iter != list_cur.end())
			data_label[index] = cell->Id();
	}
	//invalidate label mask in gpu
	vd->GetVR()->clear_tex_current();
	//save label mask to disk
	vd->SaveLabel(true, m_cur_time, vd->GetCurChannel());

	//modify graphs
	trace_group->CombineCells(cell, list_cur,
		m_cur_time);

	//update view
	CellUpdate();
}

void TraceDlg::OnCellSeparateID(wxCommandEvent& event)
{
	if (!m_view)
		return;

	//trace group
	TraceGroup *trace_group = m_view->GetTraceGroup();
	if (!trace_group)
		return;

	//current T
	fls::CelpList list_cur;
	//fill current list
	long item = -1;
	while (true)
	{
		item = m_trace_list_curr->GetNextItem(
			item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (item == -1)
			break;
		else
			AddLabel(item, m_trace_list_curr, list_cur);
	}
	if (list_cur.empty())
	{
		item = -1;
		while (true)
		{
			item = m_trace_list_curr->GetNextItem(
				item, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
			if (item == -1)
				break;
			else
				AddLabel(item, m_trace_list_curr, list_cur);
		}
	}
	if (list_cur.size() <= 1)
		//nothing to divide
		return;

	//modify graphs
	trace_group->DivideCells(list_cur, m_cur_time);

}

void TraceDlg::OnCellSegSpin(wxSpinEvent& event)
{
	m_clnum = m_cell_segment_text->GetValue();
}

void TraceDlg::OnCellSegText(wxCommandEvent& event)
{
	m_clnum = m_cell_segment_text->GetValue();
}

void TraceDlg::OnCellSegment(wxCommandEvent& event)
{
	if (m_clnum < 1)
		return;
	else if (m_clnum == 1)
	{
		OnCellCombineID(event);
		return;
	}
	if (!m_view)
		return;

	//current T
	fls::CelpList list_cur;
	//fill current list
	long item = -1;
	while (true)
	{
		item = m_trace_list_curr->GetNextItem(
			item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (item == -1)
			break;
		else
			AddLabel(item, m_trace_list_curr, list_cur);
	}
	if (list_cur.empty())
	{
		item = -1;
		while (true)
		{
			item = m_trace_list_curr->GetNextItem(
				item, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
			if (item == -1)
				break;
			else
				AddLabel(item, m_trace_list_curr, list_cur);
		}
	}
	if (list_cur.size() == 0)
		//nothing to segment
		return;

	//modify graphs
	VolumeData* vd = m_view->m_glview->m_cur_vol;
	if (!vd)
		return;
	int resx, resy, resz;
	vd->GetResolution(resx, resy, resz);
	flvr::Texture* tex = vd->GetTexture();
	if (!tex)
		return;

	TraceGroup *trace_group = m_view->GetTraceGroup();
	if (!trace_group)
		return;

	fls::pTrackMap track_map = trace_group->GetTrackMap();
	fls::TrackMapProcessor tm_processor(track_map);
	tm_processor.SetBits(vd->GetBits());
	tm_processor.SetScale(vd->GetScalarScale());
	tm_processor.SetSizes(resx, resy, resz);
	//register file reading and deleteing functions
	tm_processor.RegisterCacheQueueFuncs(
		std::bind(&TraceDlg::ReadVolCache, this, std::placeholders::_1),
		std::bind(&TraceDlg::DelVolCache, this, std::placeholders::_1));
	tm_processor.SetVolCacheSize(3);
	tm_processor.SegmentCells(list_cur, m_cur_time, m_clnum);

	//invalidate label mask in gpu
	vd->GetVR()->clear_tex_current();
	//m_view->RefreshGL();
	//update view
	//CellUpdate();
	RefineMap(m_cur_time);
}

void TraceDlg::LinkAddedCells(fls::CelpList &list)
{
	if (!m_view)
		return;

	VolumeData* vd = m_view->m_glview->m_cur_vol;
	if (!vd)
		return;
	int resx, resy, resz;
	vd->GetResolution(resx, resy, resz);
	TraceGroup *trace_group = m_view->GetTraceGroup();
	if (!trace_group)
		return;

	fls::pTrackMap track_map = trace_group->GetTrackMap();
	fls::TrackMapProcessor tm_processor(track_map);
	tm_processor.SetBits(vd->GetBits());
	tm_processor.SetScale(vd->GetScalarScale());
	tm_processor.SetSizes(resx, resy, resz);
	//register file reading and deleteing functions
	tm_processor.RegisterCacheQueueFuncs(
		std::bind(&TraceDlg::ReadVolCache, this, std::placeholders::_1),
		std::bind(&TraceDlg::DelVolCache, this, std::placeholders::_1));
	tm_processor.SetVolCacheSize(3);
	tm_processor.LinkAddedCells(list, m_cur_time, m_cur_time - 1);
	tm_processor.LinkAddedCells(list, m_cur_time, m_cur_time + 1);
	RefineMap(m_cur_time, false);
}

void TraceDlg::SaveOutputResult(wxString &filename)
{
	wxFileOutputStream fos(filename);
	if (!fos.Ok())
		return;
	wxTextOutputStream tos(fos);

	wxString str;
	str = m_stat_text->GetValue();

	tos << str;
}

void TraceDlg::Test1()
{
	//wxMessageBox("It happens.");
	if (!m_view)
		return;

	//get data
	VolumeData* vd = m_view->m_glview->m_cur_vol;
	if (!vd)
		return;
	//get mask
	Nrrd* nrrd_mask = vd->GetMask(true);
	if (!nrrd_mask)
		return;
	unsigned char* data_mask = (unsigned char*)(nrrd_mask->data);
	if (!data_mask)
		return;
	//get label
	flvr::Texture* tex = vd->GetTexture();
	if (!tex)
		return;
	Nrrd* nrrd_label = tex->get_nrrd(tex->nlabel());
	if (!nrrd_label)
		return;
	unsigned int* data_label = (unsigned int*)(nrrd_label->data);
	if (!data_label)
		return;

	//get statistics on selection
	//get these values for each selected ID:
	//total voxels/surface voxels/contact voxels
	vector<comp_info> info_list;
	int ilist;
	int found;
	int i, j, k;
	int nx, ny, nz;
	unsigned long long index;
	unsigned long long indexn;
	unsigned int id;
	bool surface_vox, contact_vox;
	vd->GetResolution(nx, ny, nz);
	for (i = 0; i < nx; ++i)
		for (j = 0; j < ny; ++j)
			for (k = 0; k < nz; ++k)
			{
				index = nx*ny*k + nx*j + i;
				if (data_mask[index] &&
					data_label[index])
				{
					id = data_label[index];
					//determine the numbers
					if (i == 0 || i == nx - 1 ||
						j == 0 || j == ny - 1 ||
						k == 0 || k == nz - 1)
					{
						//border voxel
						surface_vox = true;
						//determine contact
						contact_vox = false;
						if (i > 0)
						{
							indexn = index - 1;
							if (data_label[indexn] &&
								data_label[indexn] != id)
								contact_vox = true;
						}
						if (!contact_vox && i < nx - 1)
						{
							indexn = index + 1;
							if (data_label[indexn] &&
								data_label[indexn] != id)
								contact_vox = true;
						}
						if (!contact_vox && j > 0)
						{
							indexn = index - nx;
							if (data_label[indexn] &&
								data_label[indexn] != id)
								contact_vox = true;
						}
						if (!contact_vox && j < ny - 1)
						{
							indexn = index + nx;
							if (data_label[indexn] &&
								data_label[indexn] != id)
								contact_vox = true;
						}
						if (!contact_vox && k > 0)
						{
							indexn = index - nx*ny;
							if (data_label[indexn] &&
								data_label[indexn] != id)
								contact_vox = true;
						}
						if (!contact_vox && k < nz - 1)
						{
							indexn = index + nx*ny;
							if (data_label[indexn] &&
								data_label[indexn] != id)
								contact_vox = true;
						}
					}
					else
					{
						surface_vox = false;
						contact_vox = false;
						//i-1
						indexn = index - 1;
						if (data_label[indexn] == 0)
							surface_vox = true;
						if (data_label[indexn] &&
							data_label[indexn] != id)
							surface_vox = contact_vox = true;
						//i+1
						if (!surface_vox || !contact_vox)
						{
							indexn = index + 1;
							if (data_label[indexn] == 0)
								surface_vox = true;
							if (data_label[indexn] &&
								data_label[indexn] != id)
								surface_vox = contact_vox = true;
						}
						//j-1
						if (!surface_vox || !contact_vox)
						{
							indexn = index - nx;
							if (data_label[indexn] == 0)
								surface_vox = true;
							if (data_label[indexn] &&
								data_label[indexn] != id)
								surface_vox = contact_vox = true;
						}
						//j+1
						if (!surface_vox || !contact_vox)
						{
							indexn = index + nx;
							if (data_label[indexn] == 0)
								surface_vox = true;
							if (data_label[indexn] &&
								data_label[indexn] != id)
								surface_vox = contact_vox = true;
						}
						//k-1
						if (!surface_vox || !contact_vox)
						{
							indexn = index - nx*ny;
							if (data_label[indexn] == 0)
								surface_vox = true;
							if (data_label[indexn] &&
								data_label[indexn] != id)
								surface_vox = contact_vox = true;
						}
						//k+1
						if (!surface_vox || !contact_vox)
						{
							indexn = index + nx*ny;
							if (data_label[indexn] == 0)
								surface_vox = true;
							if (data_label[indexn] &&
								data_label[indexn] != id)
								surface_vox = contact_vox = true;
						}
					}

					//update list
					//find in info list
					found = -1;
					for (ilist = 0; ilist < (int)info_list.size(); ++ilist)
					{
						if (info_list[ilist].id == id)
						{
							found = ilist;
							break;
						}
					}
					if (found == -1)
					{
						//not found
						comp_info info;
						info.id = id;
						info.total_num = 1;
						info.surface_num = surface_vox ? 1 : 0;
						info.contact_num = contact_vox ? 1 : 0;
						info_list.push_back(info);
					}
					else
					{
						//found
						info_list[found].total_num++;
						info_list[found].surface_num += surface_vox ? 1 : 0;
						info_list[found].contact_num += contact_vox ? 1 : 0;
					}
				}
			}
	wxString str = "Statistics on the selection:\n";
	for (i = 0; i < (int)info_list.size(); ++i)
	{
		str += wxString::Format("ID: %u, ", info_list[i].id);
		str += wxString::Format("TotalN: %d, ", info_list[i].total_num);
		str += wxString::Format("SurfaceN: %d, ", info_list[i].surface_num);
		str += wxString::Format("ContactN: %d, ", info_list[i].contact_num);
		str += wxString::Format("Ratio: %.2f\n", (double)info_list[i].contact_num / (double)info_list[i].surface_num);
	}
	m_stat_text->SetValue(str);
}

void TraceDlg::Test2(int type)
{
	/*	if (!m_view)
			return;
		//trace group
		TraceGroup *trace_group = m_view->GetTraceGroup();
		if (!trace_group)
			return;
		//get data
		VolumeData* vd = m_view->m_glview->m_cur_vol;
		if (!vd)
			return;
		//get mask
		Nrrd* nrrd_mask = vd->GetMask();
		if (!nrrd_mask)
			return;
		unsigned char* data_mask = (unsigned char*)(nrrd_mask->data);
		if (!data_mask)
			return;
		//get label
		Texture* tex = vd->GetTexture();
		if (!tex)
			return;
		Nrrd* nrrd_label = tex->get_nrrd(tex->nlabel());
		if (!nrrd_label)
			return;
		unsigned int* data_label = (unsigned int*)(nrrd_label->data);
		if (!data_label)
			return;

		//get current selection
		unsigned long id;
		wxString str;
		set<unsigned int> id_list;
		set<unsigned int>::iterator id_iter;
		long item = -1;
		for (;;)
		{
			item = m_trace_list_curr->GetNextItem(item,
				wxLIST_NEXT_ALL,
				wxLIST_STATE_DONTCARE);
			if (item != -1)
			{
				str = m_trace_list_curr->GetText(item ,0);
				str.ToULong(&id);
				id_list.insert(id);
			}
			else break;
		}
		if (id_list.empty())
			return;

		//search from current frame onward
		str = "";
		int time = m_view->m_glview->m_tseq_cur_num;
		for (id_iter=id_list.begin();
			id_iter!=id_list.end(); ++id_iter)
		{
			id = *id_iter;
			if (trace_group->FindPattern(type, id, time))
			{
				str += wxString::Format("ID: %u, ", id);
				str += wxString::Format("time: %d\n", time);
			}
		}
		m_stat_text->SetValue(str);*/
}

//read/delete volume cache
void TraceDlg::ReadVolCache(fls::VolCache& vol_cache)
{
	//get volume, readers
	if (!m_view || !m_view->m_glview)
		return;
	VolumeData* vd = m_view->m_glview->m_cur_vol;
	if (!vd)
		return;
	BaseReader* reader = vd->GetReader();
	if (!reader)
		return;
	LBLReader lbl_reader;

	int chan = vd->GetCurChannel();
	int frame = vol_cache.frame;

	if (frame == m_view->m_glview->m_tseq_cur_num)
	{
		flvr::Texture* tex = vd->GetTexture();
		if (!tex)
			return;

		Nrrd* data = tex->get_nrrd(0);
		vol_cache.nrrd_data = data;
		vol_cache.data = data->data;
		Nrrd* label = tex->get_nrrd(tex->nlabel());
		vol_cache.nrrd_label = label;
		vol_cache.label = label->data;
		if (data && label)
			vol_cache.valid = true;
	}
	else
	{
		Nrrd* data = reader->Convert(frame, chan, true);
		if (!data)
			return;
		vol_cache.nrrd_data = data;
		vol_cache.data = data->data;
		wstring lblname = reader->GetCurLabelName(frame, chan);
		lbl_reader.SetFile(lblname);
		Nrrd* label = lbl_reader.Convert(frame, chan, true);
		if (!label)
			return;
		vol_cache.nrrd_label = label;
		vol_cache.label = label->data;
		if (data && label)
			vol_cache.valid = true;
	}
}

void TraceDlg::DelVolCache(fls::VolCache& vol_cache)
{
	if (!m_view || !m_view->m_glview)
		return;
	VolumeData* vd = m_view->m_glview->m_cur_vol;
	if (!vd)
		return;
	BaseReader* reader = vd->GetReader();
	if (!reader)
		return;
	int chan = vd->GetCurChannel();
	int frame = vol_cache.frame;

	if (vol_cache.valid && vol_cache.modified)
	{
		//save it first if modified
		//assume that only label is modified
		MSKWriter msk_writer;
		msk_writer.SetData((Nrrd*)vol_cache.nrrd_label);
		double spcx, spcy, spcz;
		vd->GetSpacings(spcx, spcy, spcz);
		msk_writer.SetSpacings(spcx, spcy, spcz);
		wstring filename = reader->GetCurLabelName(frame, chan);
		msk_writer.Save(filename, 1);
	}

	vol_cache.valid = false;
	if (frame != m_view->m_glview->m_tseq_cur_num)
	{
		if (vol_cache.data)
			nrrdNuke((Nrrd*)vol_cache.nrrd_data);
		if (vol_cache.label)
			nrrdNuke((Nrrd*)vol_cache.nrrd_label);
	}
	vol_cache.data = 0;
	vol_cache.nrrd_data = 0;
	vol_cache.label = 0;
	vol_cache.nrrd_label = 0;
}

void TraceDlg::OnCellPrev(wxCommandEvent &event)
{
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetMovieView())
		vr_frame->GetMovieView()->DownFrame();
}

void TraceDlg::OnCellNext(wxCommandEvent &event)
{
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetMovieView())
		vr_frame->GetMovieView()->UpFrame();
}

//auto tracking
void TraceDlg::GenMap()
{
	if (!m_view)
		return;

	//get trace group
	m_view->CreateTraceGroup();
	TraceGroup *trace_group = m_view->GetTraceGroup();
	if (!trace_group)
		return;

	VolumeData* vd = m_view->m_glview->m_cur_vol;
	if (!vd)
		return;
	BaseReader* reader = vd->GetReader();
	if (!reader)
		return;

	//start progress
	m_stat_text->SetValue("Generating track map.\n");
	wxGetApp().Yield();
	int frames = reader->GetTimeNum();

	//get and set parameters
	fls::pTrackMap track_map = trace_group->GetTrackMap();
	fls::TrackMapProcessor tm_processor(track_map);
	int resx, resy, resz;
	vd->GetResolution(resx, resy, resz);
	double spcx, spcy, spcz;
	vd->GetSpacings(spcx, spcy, spcz);
	tm_processor.SetBits(vd->GetBits());
	tm_processor.SetScale(vd->GetScalarScale());
	tm_processor.SetSizes(resx, resy, resz);
	tm_processor.SetSpacings(spcx, spcy, spcz);
	tm_processor.SetSizeThresh(m_size_thresh);
	tm_processor.SetContactThresh(m_contact_factor);
	tm_processor.SetSimilarThresh(m_similarity);
	//register file reading and deleteing functions
	tm_processor.RegisterCacheQueueFuncs(
		std::bind(&TraceDlg::ReadVolCache, this, std::placeholders::_1),
		std::bind(&TraceDlg::DelVolCache, this, std::placeholders::_1));
	tm_processor.SetVolCacheSize(4);
	//merge/split
	tm_processor.SetMerge(m_try_merge);
	tm_processor.SetSplit(m_try_split);

	//start timing
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	//initialization
	for (int i = 0; i < frames; ++i)
	{
		tm_processor.InitializeFrame(i);
		(*m_stat_text) << wxString::Format("Time point %d initialized.\n", i);
		wxGetApp().Yield();

		if (i < 1)
			continue;

		//link maps 1 and 2
		tm_processor.LinkFrames(i - 1, i);
		(*m_stat_text) << wxString::Format("Time point %d linked.\n", i);
		wxGetApp().Yield();

		//check contacts and merge cells
		tm_processor.ResolveGraph(i - 1, i);
		tm_processor.ResolveGraph(i, i - 1);
		(*m_stat_text) << wxString::Format("Time point %d merged.\n", i - 1);
		wxGetApp().Yield();

		if (i < 2)
			continue;

		//further process
		tm_processor.ProcessFrames(i - 2, i - 1);
		tm_processor.ProcessFrames(i - 1, i - 2);
		(*m_stat_text) << wxString::Format("Time point %d processed.\n", i - 1);
		wxGetApp().Yield();
	}
	//last frame
	tm_processor.ProcessFrames(frames - 2, frames - 1);
	tm_processor.ProcessFrames(frames - 1, frames - 2);
	(*m_stat_text) << wxString::Format("Time point %d processed.\n", frames - 1);
	wxGetApp().Yield();

	//iterations
	for (size_t iteri = 0; iteri < m_iter_num; ++iteri)
	{
		for (int i = 2; i <= frames; ++i)
		{
			//further process
			tm_processor.ProcessFrames(i - 2, i - 1);
			tm_processor.ProcessFrames(i - 1, i - 2);
			(*m_stat_text) << wxString::Format("Time point %d processed.\n", i - 1);
			wxGetApp().Yield();
		}
	}

	//consistent colors
	if (m_consistent_color)
	{
		(*m_stat_text) << wxString::Format("Set colors for frame 0\n");
		wxGetApp().Yield();
		tm_processor.MakeConsistent(0);
		//remaining frames
		for (size_t fi = 1; fi < track_map->GetFrameNum(); ++fi)
		{
			(*m_stat_text) << wxString::Format("Set colors for frame %d\n", int(fi));
			wxGetApp().Yield();
			tm_processor.MakeConsistent(fi - 1, fi);
		}
	}

	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
	(*m_stat_text) << wxString::Format("Wall clock time: %.4fs\n", time_span.count());

	GetSettings(m_view);

	//CellUpdate();
}

void TraceDlg::RefineMap(int t, bool erase_v)
{
	if (!m_view)
		return;

	//get trace group
	VolumeData* vd = m_view->m_glview->m_cur_vol;
	if (!vd)
		return;
	TraceGroup *trace_group = m_view->GetTraceGroup();
	if (!trace_group)
		return;
	if (t < 0)
		m_stat_text->SetValue("Refining track map for all time points.\n");
	else
		m_stat_text->SetValue(wxString::Format(
			"Refining track map at time point %d.\n", t));
	wxGetApp().Yield();

	//start progress
	bool clear_counters = false;
	fls::pTrackMap track_map = trace_group->GetTrackMap();
	int start_frame, end_frame;
	if (t < 0)
	{
		start_frame = 0;
		end_frame = track_map->GetFrameNum() - 1;
		clear_counters = true;
	}
	else
		start_frame = end_frame = t;

	//get and set parameters
	fls::TrackMapProcessor tm_processor(track_map);
	int resx, resy, resz;
	vd->GetResolution(resx, resy, resz);
	double spcx, spcy, spcz;
	vd->GetSpacings(spcx, spcy, spcz);
	tm_processor.SetBits(vd->GetBits());
	tm_processor.SetScale(vd->GetScalarScale());
	tm_processor.SetSizes(resx, resy, resz);
	tm_processor.SetSpacings(spcx, spcy, spcz);
	tm_processor.SetSizeThresh(m_size_thresh);
	tm_processor.SetContactThresh(m_contact_factor);
	tm_processor.SetSimilarThresh(m_similarity);
	//register file reading and deleteing functions
	tm_processor.RegisterCacheQueueFuncs(
		std::bind(&TraceDlg::ReadVolCache, this, std::placeholders::_1),
		std::bind(&TraceDlg::DelVolCache, this, std::placeholders::_1));
	tm_processor.SetVolCacheSize(4);
	//merge/split
	tm_processor.SetMerge(m_try_merge);
	tm_processor.SetSplit(m_try_split);

	high_resolution_clock::time_point t1 = high_resolution_clock::now();

	//not sure if counters need to be cleared for all refinement
	//if (clear_counters)
	//	tm_processor.ClearCounters();
	//iterations
	for (size_t iteri = 0; iteri < m_iter_num; ++iteri)
	{
		for (int i = start_frame - 1; i <= end_frame; ++i)
		{
			//further process
			tm_processor.ProcessFrames(i, i + 1, erase_v);
			tm_processor.ProcessFrames(i + 1, i, erase_v);
			(*m_stat_text) << wxString::Format("Time point %d processed.\n", i + 1);
			wxGetApp().Yield();
		}
	}

	//consistent colors
	if (m_consistent_color)
	{
		if (t < 0)
		{
			(*m_stat_text) << wxString::Format("Set colors for frame 0\n");
			wxGetApp().Yield();
			tm_processor.MakeConsistent(0);
			//remaining frames
			for (size_t fi = 1; fi < track_map->GetFrameNum(); ++fi)
			{
				(*m_stat_text) << wxString::Format("Set colors for frame %d\n", int(fi));
				wxGetApp().Yield();
				tm_processor.MakeConsistent(fi - 1, fi);
			}
		}
		else
		{
			tm_processor.MakeConsistent(t - 1, t);
		}
	}

	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
	(*m_stat_text) << wxString::Format("Wall clock time: %.4fs\n", time_span.count());

	CellUpdate();
}

int TraceDlg::GetTrackFileExist(bool save)
{
	if (!m_view) return 0;
	TraceGroup* trace_group = m_view->GetTraceGroup();
	if (!trace_group)
		return 0;
	wxString filename = trace_group->GetPath();
	if (wxFileExists(filename))
	{
		if (save)
		{
			m_view->SaveTraceGroup(filename);
			m_track_file = filename;
		}
		return 2;
	}
	else
		return 1;
}

wxString TraceDlg::GetTrackFile()
{
	return m_track_file;
}

void TraceDlg::LoadTrackFile(wxString &file)
{
	if (!m_view) return;
	int rval = m_view->LoadTraceGroup(file);
	if (rval)
	{
		m_load_trace_text->SetValue(file);
		m_track_file = file;
	}
}

void TraceDlg::SaveTrackFile(wxString &file)
{
	if (!m_view) return;
	int rval = m_view->SaveTraceGroup(file);
	if (rval)
	{
		m_load_trace_text->SetValue(file);
		m_track_file = file;
	}
}