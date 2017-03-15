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

	DeleteAllItems();

	FL::CellList sel_cells = traces->GetCellList();
	std::vector<FL::pCell> cells;
	for (FL::CellListIter siter = sel_cells.begin();
	siter != sel_cells.end(); ++siter)
		cells.push_back(siter->second);

	if (cells.empty())
		return;
	else
		std::sort(cells.begin(), cells.end(), sort_cells);

	wxString gtype;
	unsigned int id;
	unsigned int vid;
	Color c;
	wxColor wxc;
	int size;
	Point center;
	bool prev, next;

	for (size_t i = 0; i < cells.size(); ++i)
	{
		id = cells[i]->Id();
		vid = cells[i]->GetVertexId();
		c = HSVColor(id % 360, 1.0, 1.0);
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
			if (((TraceDlg*)parent)->GetManualAssist())
				((TraceDlg*)parent)->CellLink(true);
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
//selection page
//component tools
EVT_TEXT(ID_CompIDText, TraceDlg::OnCompIDText)
EVT_TEXT_ENTER(ID_CompIDText, TraceDlg::OnCompFull)
EVT_BUTTON(ID_CompIDXBtn, TraceDlg::OnCompIDXBtn)
EVT_BUTTON(ID_CompFullBtn, TraceDlg::OnCompFull)
EVT_BUTTON(ID_CompExclusiveBtn, TraceDlg::OnCompExclusive)
EVT_BUTTON(ID_CompAppendBtn, TraceDlg::OnCompAppend)
EVT_BUTTON(ID_CompClearBtn, TraceDlg::OnCompClear)
//cell size filter
EVT_COMMAND_SCROLL(ID_CellSizeSldr, TraceDlg::OnCellSizeChange)
EVT_TEXT(ID_CellSizeText, TraceDlg::OnCellSizeText)
//uncertainty filter
EVT_BUTTON(ID_CompUncertainBtn, TraceDlg::OnCompUncertainBtn)
EVT_COMMAND_SCROLL(ID_CompUncertainLowSldr, TraceDlg::OnCompUncertainLowChange)
EVT_TEXT(ID_CompUncertainLowText, TraceDlg::OnCompUncertainLowText)
EVT_COMMAND_SCROLL(ID_CompUncertainHiSldr, TraceDlg::OnCompUncertainHiChange)
EVT_TEXT(ID_CompUncertainHiText, TraceDlg::OnCompUncertainHiText)
//link page
EVT_TEXT(ID_CompIDText2, TraceDlg::OnCompIDText)
EVT_TEXT_ENTER(ID_CompIDText2, TraceDlg::OnCompAppend)
//ID link controls
EVT_BUTTON(ID_CellExclusiveLinkBtn, TraceDlg::OnCellExclusiveLink)
EVT_BUTTON(ID_CellLinkBtn, TraceDlg::OnCellLink)
EVT_BUTTON(ID_CellIsolateBtn, TraceDlg::OnCellIsolate)
EVT_BUTTON(ID_CellUnlinkBtn, TraceDlg::OnCellUnlink)
//manual assist
EVT_TOOL(ID_ManualAssistCheck, TraceDlg::OnManualAssistCheck)
//modify page
//ID edit controls
EVT_TEXT(ID_CellNewIDText, TraceDlg::OnCellNewIDText)
EVT_TEXT_ENTER(ID_CellNewIDText, TraceDlg::OnCompAppend)
EVT_BUTTON(ID_CellNewIDXBtn, TraceDlg::OnCellNewIDX)
EVT_BUTTON(ID_CompAppend2Btn, TraceDlg::OnCompAppend)
EVT_TOOL(ID_AutoIDChk, TraceDlg::OnAutoIDChk)
EVT_BUTTON(ID_CellNewIDBtn, TraceDlg::OnCellNewID)
EVT_BUTTON(ID_CellAppendIDBtn, TraceDlg::OnCellAppendID)
EVT_BUTTON(ID_CellReplaceIDBtn, TraceDlg::OnCellReplaceID)
EVT_BUTTON(ID_CellCombineIDBtn, TraceDlg::OnCellCombineID)
EVT_BUTTON(ID_CellSeparateBtn, TraceDlg::OnCellSeparateID)
EVT_BUTTON(ID_CellDivideIDBtn, TraceDlg::OnCellDivideID)
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
	wxBoxSizer* sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "New map:",
		wxDefaultPosition, wxSize(70, 20));
	m_gen_map_prg = new wxGauge(page, ID_GenMapPrg, 100,
		wxDefaultPosition, wxSize(-1, 18));
	m_gen_map_btn = new wxButton(page, ID_GenMapBtn, "Gen.",
		wxDefaultPosition, wxSize(65, 23));
	m_refine_t_btn = new wxButton(page, ID_RefineTBtn, "Ref. T",
		wxDefaultPosition, wxSize(65, 23));
	m_refine_all_btn = new wxButton(page, ID_RefineAllBtn, "Ref. All",
		wxDefaultPosition, wxSize(65, 23));
	sizer_2->Add(5, 5);
	sizer_2->Add(st, 0, wxALIGN_CENTER);
	sizer_2->Add(m_gen_map_prg, 1, wxEXPAND);
	sizer_2->Add(m_gen_map_btn, 0, wxALIGN_CENTER);
	sizer_2->Add(m_refine_t_btn, 0, wxALIGN_CENTER);
	sizer_2->Add(m_refine_all_btn, 0, wxALIGN_CENTER);

	//settings
	wxBoxSizer* sizer_3 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Times:",
		wxDefaultPosition, wxDefaultSize);
	m_map_iter_spin = new wxSpinCtrl(page, ID_MapIterSpin, "3",
		wxDefaultPosition, wxSize(50, 23));
	sizer_3->Add(5, 5);
	sizer_3->Add(st, 0, wxALIGN_CENTER);
	sizer_3->Add(m_map_iter_spin, 0, wxALIGN_CENTER);
	st = new wxStaticText(page, 0, "Size Thr.:",
		wxDefaultPosition, wxDefaultSize);
	m_map_size_spin = new wxSpinCtrl(page, ID_MapSizeSpin, "25",
		wxDefaultPosition, wxSize(50, 23));
	sizer_3->AddStretchSpacer(1);
	sizer_3->Add(st, 0, wxALIGN_CENTER);
	sizer_3->Add(m_map_size_spin, 0, wxALIGN_CENTER);
	st = new wxStaticText(page, 0, "Contact F.:",
		wxDefaultPosition, wxDefaultSize);
	m_map_cntct_spin = new wxSpinCtrlDouble(
		page, ID_MapCntctSpin, "0.6",
		wxDefaultPosition, wxSize(50, 23),
		wxSP_ARROW_KEYS | wxSP_WRAP,
		0, 1, 0.6, 0.01);
	sizer_3->AddStretchSpacer(1);
	sizer_3->Add(st, 0, wxALIGN_CENTER);
	sizer_3->Add(m_map_cntct_spin, 0, wxALIGN_CENTER);
	st = new wxStaticText(page, 0, "Similarity:",
		wxDefaultPosition, wxDefaultSize);
	m_map_simlr_spin = new wxSpinCtrlDouble(
		page, ID_MapSimlrSpin, "0.2",
		wxDefaultPosition, wxSize(50, 23),
		wxSP_ARROW_KEYS| wxSP_WRAP,
		0, 1, 0.2, 0.01);
	sizer_3->AddStretchSpacer(1);
	sizer_3->Add(st, 0, wxALIGN_CENTER);
	sizer_3->Add(m_map_simlr_spin, 0, wxALIGN_CENTER);
	sizer_3->Add(5, 5);
	m_map_merge_chk = new wxCheckBox(
		page, ID_MapMergeChk, "Merge",
		wxDefaultPosition, wxDefaultSize,
		wxALIGN_CENTER);
	m_map_split_chk = new wxCheckBox(
		page, ID_MapSplitChk, "Split",
		wxDefaultPosition, wxDefaultSize,
		wxALIGN_CENTER);
	sizer_3->AddStretchSpacer(1);
	sizer_3->Add(m_map_merge_chk, 0, wxALIGN_CENTER);
	sizer_3->Add(m_map_split_chk, 0, wxALIGN_CENTER);

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
		wxDefaultPosition, wxSize(80, 23));
	m_comp_exclusive_btn = new wxButton(page, ID_CompExclusiveBtn, "Replace",
		wxDefaultPosition, wxSize(80, 23));
	m_comp_append_btn = new wxButton(page, ID_CompAppendBtn, "Append",
		wxDefaultPosition, wxSize(80, 23));
	m_comp_clear_btn = new wxButton(page, ID_CompClearBtn, "Clear",
		wxDefaultPosition, wxSize(80, 23));
	sizer_1->Add(5, 5);
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	sizer_1->Add(m_comp_id_text, 0, wxALIGN_CENTER);
	sizer_1->Add(m_comp_id_x_btn, 0, wxALIGN_CENTER);
	sizer_1->Add(10, 23);
	sizer_1->Add(m_comp_full_btn, 0, wxALIGN_CENTER);
	sizer_1->Add(m_comp_exclusive_btn, 0, wxALIGN_CENTER);
	sizer_1->Add(m_comp_append_btn, 0, wxALIGN_CENTER);
	sizer_1->Add(m_comp_clear_btn, 0, wxALIGN_CENTER);
	//cell size filter
	wxBoxSizer* sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Component size:",
		wxDefaultPosition, wxSize(130, 20));
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
	m_comp_uncertain_btn = new wxButton(page, ID_CompUncertainBtn, "Uncertainty filter:",
		wxDefaultPosition, wxSize(130, 23));
	m_comp_uncertain_low_sldr = new wxSlider(page, ID_CompUncertainLowSldr, 0, 0, 20,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_comp_uncertain_low_text = new wxTextCtrl(page, ID_CompUncertainLowText, "0",
		wxDefaultPosition, wxSize(60, 23), 0, vald_int);
	m_comp_uncertain_hi_sldr = new wxSlider(page, ID_CompUncertainHiSldr, 20, 0, 20,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_comp_uncertain_hi_text = new wxTextCtrl(page, ID_CompUncertainHiText, "20",
		wxDefaultPosition, wxSize(60, 23), 0, vald_int);
	sizer_3->Add(5, 5);
	sizer_3->Add(m_comp_uncertain_btn, 0, wxALIGN_CENTER);
	sizer_3->Add(m_comp_uncertain_low_sldr, 1, wxEXPAND);
	sizer_3->Add(m_comp_uncertain_low_text, 0, wxALIGN_CENTER);
	sizer_3->Add(5, 5);
	sizer_3->Add(m_comp_uncertain_hi_sldr, 1, wxEXPAND);
	sizer_3->Add(m_comp_uncertain_hi_text, 0, wxALIGN_CENTER);

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
	m_manual_assist_check = new wxToolBar(page, wxID_ANY,
		wxDefaultPosition, wxSize(-1, 23), wxTB_NODIVIDER);
	wxBitmap bitmap = wxGetBitmapFromMemory(auto_link_off);
#ifdef _DARWIN
	m_manual_assist_check->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_manual_assist_check->AddCheckTool(ID_ManualAssistCheck, "Auto Link",
		bitmap, wxNullBitmap,
		"Automatically link selected IDs after each paint brush stroke",
		"Automatically link selected IDs after each paint brush stroke");
	m_manual_assist_check->SetBackgroundColour(m_notebook->GetThemeBackgroundColour());
	m_manual_assist_check->Realize();
	sizer_1->Add(5, 5);
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	sizer_1->Add(m_comp_id_text2, 0, wxALIGN_CENTER);
	sizer_1->Add(m_comp_id_x_btn, 0, wxALIGN_CENTER);
	sizer_1->Add(10, 23);
	sizer_1->Add(m_comp_append_btn, 0, wxALIGN_CENTER);
	sizer_1->Add(m_comp_clear_btn, 0, wxALIGN_CENTER);
	sizer_1->AddStretchSpacer();
	sizer_1->Add(m_manual_assist_check, 0, wxALIGN_CENTER);

	//ID link controls
	wxBoxSizer* sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	m_cell_exclusive_link_btn = new wxButton(page, ID_CellExclusiveLinkBtn, "Excl. Link",
		wxDefaultPosition, wxSize(80, 23));
	m_cell_link_btn = new wxButton(page, ID_CellLinkBtn, "Link IDs",
		wxDefaultPosition, wxSize(80, 23));
	sizer_2->AddStretchSpacer();
	sizer_2->Add(m_cell_exclusive_link_btn, 0, wxALIGN_CENTER);
	sizer_2->Add(m_cell_link_btn, 0, wxALIGN_CENTER);
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
	m_auto_id_chk = new wxToolBar(page, wxID_ANY,
		wxDefaultPosition, wxSize(-1, 23), wxTB_NODIVIDER);
	wxBitmap bitmap = wxGetBitmapFromMemory(auto_assign_off);
#ifdef _DARWIN
	m_auto_id_chk->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_auto_id_chk->AddCheckTool(ID_AutoIDChk, "Auto Assign ID",
		bitmap, wxNullBitmap,
		"Automatically assign an ID to selection after each paint brush stroke",
		"Automatically assign an ID to selection after each paint brush stroke");
	m_auto_id_chk->SetBackgroundColour(m_notebook->GetThemeBackgroundColour());
	m_auto_id_chk->Realize();
	sizer_1->Add(5, 5);
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	sizer_1->Add(m_cell_new_id_text, 0, wxALIGN_CENTER);
	sizer_1->Add(m_cell_new_id_x_btn, 0, wxALIGN_CENTER);
	sizer_1->Add(10, 23);
	sizer_1->Add(m_comp_append2_btn, 0, wxALIGN_CENTER);
	sizer_1->Add(m_comp_clear_btn, 0, wxALIGN_CENTER);
	sizer_1->AddStretchSpacer();
	sizer_1->Add(m_auto_id_chk, 0, wxALIGN_CENTER);

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
	m_cell_divide_id_btn = new wxButton(page, ID_CellDivideIDBtn, "Divide",
		wxDefaultPosition, wxSize(85, 23));
	sizer_3->AddStretchSpacer();
	sizer_3->Add(m_cell_combine_id_btn, 0, wxALIGN_CENTER);
	sizer_3->Add(m_cell_separate_id_btn, 0, wxALIGN_CENTER);
	sizer_3->Add(m_cell_divide_id_btn, 0, wxALIGN_CENTER);
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
	m_mask(0),
	m_cur_time(-1),
	m_prv_time(-1),
	m_manual_assist(false),
	m_auto_id(false)
{
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;

	//notebook
	m_notebook = new wxNotebook(this, wxID_ANY);
	m_notebook->AddPage(CreateMapPage(m_notebook), L"Track Map \u21e8");
	m_notebook->AddPage(CreateSelectPage(m_notebook), L"Selection \u21e8");
	m_notebook->AddPage(CreateLinkPage(m_notebook), L"Linkage \u21e8");
	m_notebook->AddPage(CreateModifyPage(m_notebook), L"Modify \u21e8");
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
	if (m_mask)
	{
		delete[] reinterpret_cast<char*>(m_mask->data);
		nrrdNix(m_mask);
	}
}

void TraceDlg::GetSettings(VRenderView* vrv)
{
	if (!vrv) return;
	m_view = vrv;

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
		m_cur_time = m_view->m_glview->m_tseq_cur_num;
		m_load_trace_text->SetValue("No Track map");
	}

	//manual tracking assist
	m_manual_assist_check->ToggleTool(ID_ManualAssistCheck, m_manual_assist);
	if (m_manual_assist)
		m_manual_assist_check->SetToolNormalBitmap(
			ID_ManualAssistCheck, wxGetBitmapFromMemory(auto_link_on));
	else
		m_manual_assist_check->SetToolNormalBitmap(
			ID_ManualAssistCheck, wxGetBitmapFromMemory(auto_link_off));
	//auto id
	m_auto_id_chk->ToggleTool(ID_AutoIDChk, m_auto_id);
	if (m_auto_id)
		m_auto_id_chk->SetToolNormalBitmap(
			ID_AutoIDChk, wxGetBitmapFromMemory(auto_assign_on));
	else
		m_auto_id_chk->SetToolNormalBitmap(
			ID_AutoIDChk, wxGetBitmapFromMemory(auto_assign_off));

	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetSettingDlg())
	{
		double size_thresh =
			vr_frame->GetSettingDlg()->GetComponentSize();
		double con_factor =
			vr_frame->GetSettingDlg()->GetContactFactor();
		double sim_thresh =
			vr_frame->GetSettingDlg()->GetSimilarity();
		m_map_size_spin->SetValue(size_thresh);
		m_map_cntct_spin->SetValue(con_factor);
		m_map_simlr_spin->SetValue(sim_thresh);
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
	TraceGroup* trace_group = m_view->GetTraceGroup();
	if (trace_group)
	{
		int cur_time = trace_group->GetCurTime();
		int prv_time = trace_group->GetPrvTime();
		if (cur_time != m_cur_time)
		{
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
					hue = id % 360;
					Color c(HSVColor(hue, 1.0, 1.0));
					wxColor color(c.r() * 255, c.g() * 255, c.b() * 255);
					item_size.ToLong(&size);
					item_x.ToDouble(&x);
					item_y.ToDouble(&y);
					item_z.ToDouble(&z);
					m_trace_list_prev->Append(item_gtype, id, color, size, x, y, z);
				}
				else break;
			}

			m_cur_time = cur_time;
			m_prv_time = prv_time;
		}

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
		rval = m_view->LoadTraceGroup(filename);
		if (rval)
			m_load_trace_text->SetValue(filename);
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
		rval = m_view->SaveTraceGroup(filename);
		if (rval)
			m_load_trace_text->SetValue(filename);
	}

	if (fopendlg)
		delete fopendlg;
}

void TraceDlg::OnGhostNumChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	wxString str = wxString::Format("%d", ival);
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
void TraceDlg::OnCompUncertainBtn(wxCommandEvent &event)
{
	if (!m_view)
		return;
	//trace group
	TraceGroup *trace_group = m_view->GetTraceGroup();
	if (!trace_group)
		return;
	if (!trace_group->GetTrackMap()->GetFrameNum())
		return;
	FL::CellList list_in, list_out;
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

	FL::TrackMapProcessor tm_processor(trace_group->GetTrackMap());
	wxString str = m_comp_uncertain_low_text->GetValue();
	long ival;
	str.ToLong(&ival);
	tm_processor.SetUncertainLow(ival);
	str = m_comp_uncertain_hi_text->GetValue();
	str.ToLong(&ival);
	tm_processor.SetUncertainHigh(ival);
	tm_processor.GetCellsByUncertainty(list_in, list_out, m_cur_time);

	VolumeData* vd = m_view->m_glview->m_cur_vol;
	FL::ComponentSelector comp_selector(vd);
	comp_selector.SelectList(list_out);

	//update view
	CellUpdate();

	//frame
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetBrushToolDlg())
		vr_frame->GetBrushToolDlg()->UpdateUndoRedo();
}

void TraceDlg::OnCompUncertainLowChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	wxString str = wxString::Format("%d", ival);
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
}

void TraceDlg::OnCompUncertainHiChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	wxString str = wxString::Format("%d", ival);
	m_comp_uncertain_hi_text->SetValue(str);
}

void TraceDlg::OnCompUncertainHiText(wxCommandEvent &event)
{
	wxString str = m_comp_uncertain_hi_text->GetValue();
	long ival;
	str.ToLong(&ival);
	m_comp_uncertain_hi_sldr->SetValue(ival);

	if (m_view)
	{
		TraceGroup* trace_group = m_view->GetTraceGroup();
		if (trace_group)
		{
			trace_group->SetUncertainHigh(ival);
		}
	}
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
	RulerList rulers;
	trace_group->GetMappedRulers(rulers);
	for (RulerListIter iter = rulers.begin();
	iter != rulers.end(); ++iter)
	{
		(*iter)->Scale(spcx, spcy, spcz);
		m_view->GetRulerList()->push_back(*iter);
	}
	m_view->RefreshGL();
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetMeasureDlg())
		vr_frame->GetMeasureDlg()->GetSettings(m_view);
}

void TraceDlg::OnConvertConsistent(wxCommandEvent &event)
{
	if (!m_view)
		return;

	VolumeData* vd = m_view->m_glview->m_cur_vol;
	if (!vd)
		return;
	BaseReader* reader = vd->GetReader();
	if (!reader)
		return;
	LBLReader lbl_reader;
	MSKWriter lbl_writer;
	TraceGroup *trace_group = m_view->GetTraceGroup();
	if (!trace_group)
		return;

	wxDirDialog *dirdlg = new wxDirDialog(
		m_frame, "Save labels in", "",
		wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
	int rval = dirdlg->ShowModal();
	wxString out_dir;
	if (rval == wxID_OK)
	{
		out_dir = dirdlg->GetPath();
		delete dirdlg;
	}
	else
	{
		delete dirdlg;
		return;
	}

	m_stat_text->SetValue("Generating consistent IDs in");
	wxGetApp().Yield();
	FL::TrackMapProcessor tm_processor(trace_group->GetTrackMap());
	int chan = vd->GetCurChannel();
	int nx, ny, nz;
	vd->GetResolution(nx, ny, nz);
	unsigned long long size = (unsigned long long)nx *
		(unsigned long long)ny * (unsigned long long)nz;
	unsigned long long index;

	Nrrd* nrrd_label_in1 = 0;
	Nrrd* nrrd_label_in2 = 0;
	Nrrd* nrrd_label_out1 = 0;
	Nrrd* nrrd_label_out2 = 0;
	wstring lblname;

	CellMap cell_map;
	CellMapIter iter;
	unsigned int label_in1, label_in2;

	//read first frame
	lblname = reader->GetCurLabelName(0, chan);
	lbl_reader.SetFile(lblname);
	nrrd_label_in1 = lbl_reader.Convert(0, chan, true);
	(*m_stat_text) << wxString::Format("Label in 1 of frame %d read.\n", 0);
	wxGetApp().Yield();
	//duplicate
	nrrd_label_in2 = nrrdNew();
	nrrdCopy(nrrd_label_in2, nrrd_label_in1);
	for (index = 0; index < size; ++index)
	{
		label_in1 = ((unsigned int*)(nrrd_label_in1->data))[index];
		iter = cell_map.find(label_in1);
		if (iter != cell_map.end())
		{
			((unsigned int*)(nrrd_label_in2->data))[index] = iter->second;
		}
		else
		{
			if (tm_processor.GetMappedID(label_in1, label_in2, 0))
			{
				((unsigned int*)(nrrd_label_in2->data))[index] = label_in2;
				cell_map.insert(pair<unsigned int, unsigned int>(label_in1, label_in2));
			}
		}
	}
	lblname = out_dir.ToStdWstring() + GET_NAME(lblname);
	lbl_writer.SetData(nrrd_label_in2);
	lbl_writer.Save(lblname, 1);
	(*m_stat_text) << wxString::Format("Label in 2 of frame %d written.\n", 0);
	wxGetApp().Yield();

	unsigned int label_out1, label_out2;
	//remaining frames
	for (size_t fi = 1; fi < trace_group->GetTrackMap()->GetFrameNum(); ++fi)
	{
		cell_map.clear();

		//read fi frame
		lblname = reader->GetCurLabelName(fi, chan);
		lbl_reader.SetFile(lblname);
		nrrd_label_out1 = lbl_reader.Convert(fi, chan, true);
		(*m_stat_text) << wxString::Format("Label out 1 of frame %d read.\n", int(fi));
		wxGetApp().Yield();

		//copy
		nrrd_label_out2 = nrrdNew();
		nrrdCopy(nrrd_label_out2, nrrd_label_out1);

		for (index = 0; index < size; ++index)
		{
			label_out1 = ((unsigned int*)(nrrd_label_out1->data))[index];
			iter = cell_map.find(label_out1);
			if (iter != cell_map.end())
			{
				((unsigned int*)(nrrd_label_out2->data))[index] = iter->second;
			}
			else
			{
				if (tm_processor.GetMappedID(label_out1, label_in1, fi, fi - 1))
				{
					label_out2 = GetMappedID(label_in1,
						(unsigned int*)(nrrd_label_in1->data),
						(unsigned int*)(nrrd_label_in2->data),
						size);
					if (label_out2)
					{
						((unsigned int*)(nrrd_label_out2->data))[index] = label_out2;
						cell_map.insert(pair<unsigned int, unsigned int>(label_out1, label_out2));
					}
				}
			}
		}

		//save
		lblname = out_dir.ToStdWstring() + GET_NAME(lblname);
		lbl_writer.SetData(nrrd_label_out2);
		lbl_writer.Save(lblname, 1);
		(*m_stat_text) << wxString::Format("Label out 2 of frame %d written.\n", int(fi));
		wxGetApp().Yield();

		//swap
		nrrdNuke(nrrd_label_in1);
		nrrdNuke(nrrd_label_in2);
		nrrd_label_in1 = nrrd_label_out1;
		nrrd_label_in2 = nrrd_label_out2;
	}

	//release
	nrrdNuke(nrrd_label_out1);
	nrrdNuke(nrrd_label_out2);

	(*m_stat_text) << "All done.\n";
}

void TraceDlg::OnAnalyzeComp(wxCommandEvent &event)
{
	if (!m_view)
		return;
	VolumeData* vd = m_view->m_glview->m_cur_vol;
	FL::ComponentAnalyzer comp_analyzer(vd);
	comp_analyzer.Analyze(true);
	string str;
	comp_analyzer.OutputCompList(str, 1);
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
	FL::VertexList in_orphan_list;
	FL::VertexList out_orphan_list;
	FL::VertexList in_multi_list;
	FL::VertexList out_multi_list;
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
	FL::CellList list_in;
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

	FL::TrackMapProcessor tm_processor(trace_group->GetTrackMap());
	if (list_in.empty())
	{
		FL::UncertainHist hist1, hist2;
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
		for (FL::CellListIter iter = list_in.begin();
			iter != list_in.end(); ++iter)
		{
			wxString sid = wxString::Format("%u", iter->second->Id());
			(*m_stat_text) << sid << "\t" <<
				int(iter->second->GetSizeUi()) << "\t" <<
				int(iter->second->GetExternalUi()) << "\n";
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
	FL::CellList list_in;
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

	FL::TrackMapProcessor tm_processor(trace_group->GetTrackMap());
	if (list_in.empty())
		return;

	m_stat_text->SetValue("");
	std::ostream os(m_stat_text);

	if (m_cur_time > 0)
	{
		(*m_stat_text) << "Paths of T" << m_cur_time << " to T" << m_cur_time - 1 << ":\n";
		FL::PathList paths_prv;
		tm_processor.GetPaths(list_in, paths_prv, m_cur_time, m_cur_time - 1);
		for (size_t i = 0; i < paths_prv.size(); ++i)
			os << paths_prv[i];
	}
	if (m_cur_time < trace_group->GetTrackMap()->GetFrameNum() - 1)
	{
		(*m_stat_text) << "Paths of T" << m_cur_time << " to T" << m_cur_time + 1 << ":\n";
		FL::PathList paths_nxt;
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

//manual tracking assist
void TraceDlg::OnManualAssistCheck(wxCommandEvent &event)
{
	m_manual_assist = m_manual_assist_check->GetToolState(ID_ManualAssistCheck);
	if (m_manual_assist)
	{
		m_manual_assist_check->SetToolNormalBitmap(
			ID_ManualAssistCheck,
			wxGetBitmapFromMemory(auto_link_on));
		(*m_stat_text) << "Auto link enabled. Remember to turn it off after tracking!\n";
	}
	else
	{
		m_manual_assist_check->SetToolNormalBitmap(
			ID_ManualAssistCheck,
			wxGetBitmapFromMemory(auto_link_off));
		if (!m_auto_id)
			m_stat_text->Clear();
	}
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
	FL::ComponentSelector comp_selector(vd);
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
	FL::ComponentSelector comp_selector(vd);
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
		FL::ComponentSelector comp_selector(vd);
		comp_selector.SetId(id);
		comp_selector.SetMinNum(true, slimit);
		comp_selector.Append(get_all);
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
		FL::ComponentSelector comp_selector(vd);
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

		m_view->m_glview->GetTraces();
		m_view->RefreshGL();
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
	FL::ComponentSelector comp_selector(vd);
	comp_selector.SetMinNum(true, slimit);
	comp_selector.CompFull();
	//update view
	CellUpdate();

	//frame
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetBrushToolDlg())
		vr_frame->GetBrushToolDlg()->UpdateUndoRedo();
}

void TraceDlg::AddLabel(long item, TraceListCtrl* trace_list_ctrl, FL::CellList &list)
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

	FL::pCell cell(new FL::Cell(id));
	cell->SetSizeUi(size);
	cell->SetSizeF(float(size));
	FLIVR::Point p(x, y, z);
	cell->SetCenter(p);
	list.insert(pair<unsigned int, FL::pCell>
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
	if (m_auto_id)
		//get prev mask
		nrrd_mask = vd->GetMask(false);
	else
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
	Texture* tex = vd->GetTexture();
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
			if (m_auto_id)
			{
				new_id = id_vol;
				inc = 0;
			}
			else
			{
				new_id = id_vol + 10;
				inc = 10;
			}
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
			if (m_auto_id)
				inc = 0;
			else
				inc = 360;
		}
		else
		{
			new_id = 0;
			inc = 0;
		}
	}
	unsigned int stop_id = new_id;
	if (inc)
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

	if (m_auto_id)
	{
		//get current mask
		nrrd_mask = vd->GetMask(true);
		if (!nrrd_mask)
			return;
	}

	//update label volume, set mask region to the new ID
	int i, j, k;
	FL::pCell cell;
	if (new_id)
		cell = FL::pCell(new FL::Cell(new_id));
	for (i = 0; i < nx; ++i)
		for (j = 0; j < ny; ++j)
			for (k = 0; k < nz; ++k)
			{
				index = nx*ny*k + nx*j + i;
				if (data_mask[index])
				{
					if (m_auto_id)
					{
						if (data_label[index] &&
							data_label[index] != new_id)
						{
							data_mask[index] = 0;
							continue;
						}
					}
					else if (append && data_label[index])
						continue;
					data_label[index] = new_id;
					if (new_id)
						cell->Inc(i, j, k, 1.0f);
				}
			}

	if (new_id)
		trace_group->AddCell(cell, m_cur_time);

	//invalidate label mask in gpu
	vd->GetVR()->clear_tex_pool();
	//save label mask to disk
	vd->SaveLabel(true, m_cur_time, vd->GetCurChannel());

	CellUpdate();
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
	Nrrd* nrrd_label = vd->GetLabel(false);
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
		vd->GetVR()->clear_tex_pool();
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
	FL::CellList list_cur;
	//previous T
	FL::CellList list_prv;
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
	FL::CellList list_cur;

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
	FL::CellList list_cur;
	//previous T
	FL::CellList list_prv;
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
	wxString str = m_cell_new_id_text->GetValue();
	unsigned long id;
	wxColor color(255, 255, 255);
	if (str.ToULong(&id))
	{
		if (!id)
			color = wxColor(24, 167, 181);
		else
		{
			Color c = HSVColor(id % 360, 1.0, 1.0);
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

void TraceDlg::OnAutoIDChk(wxCommandEvent &event)
{
	m_auto_id = m_auto_id_chk->GetToolState(ID_AutoIDChk);
	if (m_auto_id)
	{
		m_auto_id_chk->SetToolNormalBitmap(
			ID_AutoIDChk,
			wxGetBitmapFromMemory(auto_assign_on));
		(*m_stat_text) << "Auto ID assignment enabled. Remember to turn it off after finish tracking!\n";
	}
	else
	{
		m_auto_id_chk->SetToolNormalBitmap(
			ID_AutoIDChk,
			wxGetBitmapFromMemory(auto_assign_off));
		if (!m_manual_assist)
			m_stat_text->Clear();
	}
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
	FL::CellList list_cur;
	FL::CellListIter cell_iter;
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
	Texture* tex = vd->GetTexture();
	if (!tex)
		return;
	Nrrd* nrrd_label = tex->get_nrrd(tex->nlabel());
	if (!nrrd_label)
		return;
	unsigned int* data_label = (unsigned int*)(nrrd_label->data);
	if (!data_label)
		return;

	//replace ID
	boost::unordered_map<unsigned int, unsigned int> list_rep;
	boost::unordered_map<unsigned int, unsigned int>::iterator list_rep_iter;
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
				new_id += 360;
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
	vd->GetVR()->clear_tex_pool();
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
	FL::CellList list_cur;
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
	FL::pCell cell;
	FL::CellListIter cell_iter;
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
	Texture* tex = vd->GetTexture();
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
	vd->GetVR()->clear_tex_pool();
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
	FL::CellList list_cur;
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

void TraceDlg::OnCellDivideID(wxCommandEvent& event)
{
	if (!m_view)
		return;

	//current T
	FL::CellList list_cur;
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
	Texture* tex = vd->GetTexture();
	if (!tex)
		return;

	Nrrd* nrrd_data = tex->get_nrrd(0);
	if (!nrrd_data)
		return;
	Nrrd* nrrd_label = tex->get_nrrd(tex->nlabel());
	if (!nrrd_label)
		return;

	TraceGroup *trace_group = m_view->GetTraceGroup();
	if (!trace_group)
		return;

	FL::TrackMapProcessor tm_processor(trace_group->GetTrackMap());
	tm_processor.SetBits(vd->GetBits());
	tm_processor.SetScale(vd->GetScalarScale());
	tm_processor.SetSizes(resx, resy, resz);
	tm_processor.SegmentCells(nrrd_data->data,
		nrrd_label->data, list_cur, m_cur_time);

	//invalidate label mask in gpu
	vd->GetVR()->clear_tex_pool();
	//save label mask to disk
//	vd->SaveLabel(true, m_cur_time, vd->GetCurChannel());

	//update view
	CellUpdate();
}

//magic
/*void TraceDlg::OnCellMagic0Btn(wxCommandEvent &event)
{
}

void TraceDlg::OnCellMagic1Btn(wxCommandEvent &event)
{
	Test2(1);
}

void TraceDlg::OnCellMagic2Btn(wxCommandEvent &event)
{
	Test2(2);
}

void TraceDlg::OnCellMagic3Btn(wxCommandEvent &event)
{
	Test1();
}*/

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

unsigned int TraceDlg::GetMappedID(
	unsigned int id, unsigned int* data_label1,
	unsigned int* data_label2, unsigned long long size)
{
	for (unsigned long long i = 0; i < size; ++i)
		if (data_label1[i] == id)
			return data_label2[i];
	return 0;
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
	Texture* tex = vd->GetTexture();
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
void TraceDlg::ReadVolCache(FL::VolCache& vol_cache)
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

	Nrrd* data = reader->Convert(frame, chan, true);
	vol_cache.nrrd_data = data;
	vol_cache.data = data->data;
	wstring lblname = reader->GetCurLabelName(frame, chan);
	lbl_reader.SetFile(lblname);
	Nrrd* label = lbl_reader.Convert(frame, chan, true);
	vol_cache.nrrd_label = label;
	vol_cache.label = label->data;
	if (data && label)
		vol_cache.valid = true;
}

void TraceDlg::DelVolCache(FL::VolCache& vol_cache)
{
	while (vol_cache.valid && vol_cache.modified)
	{
		//save it first if modified
		//assume that only label is modified
		if (!m_view || !m_view->m_glview)
			break;
		VolumeData* vd = m_view->m_glview->m_cur_vol;
		if (!vd)
			break;
		BaseReader* reader = vd->GetReader();
		if (!reader)
			break;
		MSKWriter msk_writer;
		msk_writer.SetData((Nrrd*)vol_cache.nrrd_label);
		double spcx, spcy, spcz;
		vd->GetSpacings(spcx, spcy, spcz);
		msk_writer.SetSpacings(spcx, spcy, spcz);
		int chan = vd->GetCurChannel();
		int frame = vol_cache.frame;
		wstring filename = reader->GetCurLabelName(frame, chan);
		msk_writer.Save(filename, 1);
		break;
	}

	vol_cache.valid = false;
	if (vol_cache.data)
	{
		delete[] vol_cache.data;
		nrrdNix((Nrrd*)vol_cache.nrrd_data);
		vol_cache.data = 0;
		vol_cache.nrrd_data = 0;
	}
	if (vol_cache.label)
	{
		delete[] vol_cache.label;
		nrrdNix((Nrrd*)vol_cache.nrrd_label);
		vol_cache.label = 0;
		vol_cache.nrrd_label = 0;
	}
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

	//get settings
	size_t iter_num = (size_t)m_map_iter_spin->GetValue();
	float size_thresh = m_map_size_spin->GetValue();
	float sim_thresh = m_map_simlr_spin->GetValue();
	float con_factor = m_map_cntct_spin->GetValue();
	//save settings
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetSettingDlg())
	{
		vr_frame->GetSettingDlg()->SetComponentSize(size_thresh);
		vr_frame->GetSettingDlg()->SetContactFactor(con_factor);
		vr_frame->GetSettingDlg()->SetSimilarity(sim_thresh);
	}

	//start progress
	m_stat_text->SetValue("Generating track map.\n");
	wxGetApp().Yield();
	int frames = reader->GetTimeNum();
	float prog_bit = 99.0f / float(frames * (4 + iter_num));
	float prog = 0.0f;
	m_gen_map_prg->SetValue(int(prog));

	//get and set parameters
	FL::TrackMapProcessor tm_processor(trace_group->GetTrackMap());
	int resx, resy, resz;
	vd->GetResolution(resx, resy, resz);
	double spcx, spcy, spcz;
	vd->GetSpacings(spcx, spcy, spcz);
	tm_processor.SetBits(vd->GetBits());
	tm_processor.SetScale(vd->GetScalarScale());
	tm_processor.SetSizes(resx, resy, resz);
	tm_processor.SetSpacings(spcx, spcy, spcz);
	tm_processor.SetSizeThresh(size_thresh);
	tm_processor.SetContactThresh(con_factor);
	tm_processor.SetSimilarThresh(sim_thresh);
	//register file reading and deleteing functions
	tm_processor.RegisterCacheQueueFuncs(
		boost::bind(&TraceDlg::ReadVolCache, this, _1),
		boost::bind(&TraceDlg::DelVolCache, this, _1));
	tm_processor.SetVolCacheSize(4);
	//merge/split
	tm_processor.SetMerge(m_map_merge_chk->GetValue());
	tm_processor.SetSplit(m_map_split_chk->GetValue());

	//start timing
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	//initialization
	for (int i = 0; i < frames; ++i)
	{
		tm_processor.InitializeFrame(i);
		prog += prog_bit;
		m_gen_map_prg->SetValue(int(prog));
		(*m_stat_text) << wxString::Format("Time point %d initialized.\n", i);
		wxGetApp().Yield();

		if (i < 1)
			continue;

		//link maps 1 and 2
		tm_processor.LinkFrames(i - 1, i);
		prog += prog_bit;
		m_gen_map_prg->SetValue(int(prog));
		(*m_stat_text) << wxString::Format("Time point %d linked.\n", i);
		wxGetApp().Yield();

		//check contacts and merge cells
		tm_processor.ResolveGraph(i - 1, i);
		tm_processor.ResolveGraph(i, i - 1);
		prog += prog_bit;
		m_gen_map_prg->SetValue(int(prog));
		(*m_stat_text) << wxString::Format("Time point %d merged.\n", i - 1);
		wxGetApp().Yield();

		if (i < 2)
			continue;

		//further process
		tm_processor.ProcessFrames(i - 2, i - 1);
		tm_processor.ProcessFrames(i - 1, i - 2);
		prog += prog_bit;
		m_gen_map_prg->SetValue(int(prog));
		(*m_stat_text) << wxString::Format("Time point %d processed.\n", i - 1);
		wxGetApp().Yield();
	}
	//last frame
	tm_processor.ProcessFrames(frames - 2, frames - 1);
	tm_processor.ProcessFrames(frames - 1, frames - 2);
	prog += prog_bit;
	m_gen_map_prg->SetValue(int(prog));
	(*m_stat_text) << wxString::Format("Time point %d processed.\n", frames - 1);
	wxGetApp().Yield();

	//iterations
	for (size_t iteri = 0; iteri < iter_num; ++iteri)
	{
		for (int i = 2; i <= frames; ++i)
		{
			//further process
			tm_processor.ProcessFrames(i - 2, i - 1);
			tm_processor.ProcessFrames(i - 1, i - 2);
			prog += prog_bit;
			m_gen_map_prg->SetValue(int(prog));
			(*m_stat_text) << wxString::Format("Time point %d processed.\n", i - 1);
			wxGetApp().Yield();
		}
	}

	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
	(*m_stat_text) << wxString::Format("Wall clock time: %.4fs\n", time_span.count());

	m_gen_map_prg->SetValue(100);

	GetSettings(m_view);
}

void TraceDlg::RefineMap(int t)
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

	//get settings
	size_t iter_num = (size_t)m_map_iter_spin->GetValue();
	float size_thresh = m_map_size_spin->GetValue();
	float sim_thresh = m_map_simlr_spin->GetValue();
	float con_factor = m_map_cntct_spin->GetValue();
	//save settings
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetSettingDlg())
	{
		vr_frame->GetSettingDlg()->SetComponentSize(size_thresh);
		vr_frame->GetSettingDlg()->SetContactFactor(con_factor);
		vr_frame->GetSettingDlg()->SetSimilarity(sim_thresh);
	}

	//start progress
	FL::pTrackMap track_map = trace_group->GetTrackMap();
	int start_frame, end_frame;
	if (t < 0)
	{
		start_frame = 0;
		end_frame = track_map->GetFrameNum() - 1;
	}
	else
		start_frame = end_frame = t;
	float prog_bit = 99.0f / float(
		(end_frame - start_frame + 1)
		* (4 + iter_num));
	float prog = 0.0f;
	m_gen_map_prg->SetValue(int(prog));

	//get and set parameters
	FL::TrackMapProcessor tm_processor(track_map);
	int resx, resy, resz;
	vd->GetResolution(resx, resy, resz);
	double spcx, spcy, spcz;
	vd->GetSpacings(spcx, spcy, spcz);
	tm_processor.SetBits(vd->GetBits());
	tm_processor.SetScale(vd->GetScalarScale());
	tm_processor.SetSizes(resx, resy, resz);
	tm_processor.SetSpacings(spcx, spcy, spcz);
	tm_processor.SetSizeThresh(size_thresh);
	tm_processor.SetContactThresh(con_factor);
	tm_processor.SetSimilarThresh(sim_thresh);
	//register file reading and deleteing functions
	tm_processor.RegisterCacheQueueFuncs(
		boost::bind(&TraceDlg::ReadVolCache, this, _1),
		boost::bind(&TraceDlg::DelVolCache, this, _1));
	tm_processor.SetVolCacheSize(4);
	//merge/split
	tm_processor.SetMerge(m_map_merge_chk->GetValue());
	tm_processor.SetSplit(m_map_split_chk->GetValue());

	high_resolution_clock::time_point t1 = high_resolution_clock::now();

	//iterations
	for (size_t iteri = 0; iteri < iter_num; ++iteri)
	{
		for (int i = start_frame - 1; i <= end_frame; ++i)
		{
			//further process
			tm_processor.ProcessFrames(i, i + 1);
			tm_processor.ProcessFrames(i + 1, i);
			prog += prog_bit;
			m_gen_map_prg->SetValue(int(prog));
			(*m_stat_text) << wxString::Format("Time point %d processed.\n", i + 1);
			wxGetApp().Yield();
		}
	}

	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
	(*m_stat_text) << wxString::Format("Wall clock time: %.4fs\n", time_span.count());

	m_gen_map_prg->SetValue(100);
}
