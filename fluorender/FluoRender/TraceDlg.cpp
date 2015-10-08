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
#include <wx/valnum.h>
#include <wx/clipbrd.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
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
	itemCol.SetText("ID");
	InsertColumn(0, itemCol);
	SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
	itemCol.SetText("Size");
	InsertColumn(1, itemCol);
	SetColumnWidth(1, wxLIST_AUTOSIZE_USEHEADER);
	itemCol.SetText("X");
	InsertColumn(2, itemCol);
	SetColumnWidth(2, wxLIST_AUTOSIZE_USEHEADER);
	itemCol.SetText("Y");
	InsertColumn(3, itemCol);
	SetColumnWidth(3, wxLIST_AUTOSIZE_USEHEADER);
	itemCol.SetText("Z");
	InsertColumn(4, itemCol);
	SetColumnWidth(4, wxLIST_AUTOSIZE_USEHEADER);
}

TraceListCtrl::~TraceListCtrl()
{
}

void TraceListCtrl::Append(unsigned int id, wxColor color,
	int size, double cx, double cy, double cz)
{
	wxString str;
	str = wxString::Format("%u", id);
	long tmp = InsertItem(GetItemCount(), str, 0);
	SetColumnWidth(0, wxLIST_AUTOSIZE);
	str = wxString::Format("%d", size);
	SetItem(tmp, 1, str);
	SetColumnWidth(1, wxLIST_AUTOSIZE);
	str = wxString::Format("%f", cx);
	SetItem(tmp, 2, str);
	SetColumnWidth(2, wxLIST_AUTOSIZE);
	str = wxString::Format("%f", cy);
	SetItem(tmp, 3, str);
	SetColumnWidth(3, wxLIST_AUTOSIZE);
	str = wxString::Format("%f", cz);
	SetItem(tmp, 4, str);
	SetColumnWidth(4, wxLIST_AUTOSIZE);

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
	FL::CellListIter iter;

	unsigned int id;
	Color c;
	wxColor wxc;
	int size;
	Point center;

	for (iter = sel_cells.begin();
	iter != sel_cells.end(); ++iter)
	{
		id = iter->second->Id();
		c = HSVColor(id % 360, 1.0, 1.0);
		wxColor color(c.r() * 255, c.g() * 255, c.b() * 255);
		size = (int)(iter->second->GetSizeUi());
		center = iter->second->GetCenter();
		Append(id, color, size,
			center.x(), center.y(), center.z());
	}
}

void TraceListCtrl::DeleteSelection()
{
	if (m_type == 0)
	{
		wxWindow* parent = GetParent();
		if (parent)
			((TraceDlg*)parent)->CompDelete();
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
		wxString name = GetItemText(item);
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
EVT_BUTTON(ID_RefineMapBtn, TraceDlg::OnRefineMapBtn)
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
//link page
EVT_TEXT(ID_CompIDText2, TraceDlg::OnCompIDText)
EVT_TEXT_ENTER(ID_CompIDText2, TraceDlg::OnCompAppend)
//ID link controls
EVT_BUTTON(ID_CellExclusiveLinkBtn, TraceDlg::OnCellExclusiveLink)
EVT_BUTTON(ID_CellLinkBtn, TraceDlg::OnCellLink)
EVT_BUTTON(ID_CellIsolateBtn, TraceDlg::OnCellIsolate)
EVT_BUTTON(ID_CellUnlinkBtn, TraceDlg::OnCellUnlink)
//manual assist
EVT_CHECKBOX(ID_ManualAssistCheck, TraceDlg::OnManualAssistCheck)
//modify page
//ID edit controls
EVT_TEXT_ENTER(ID_CellNewIDText, TraceDlg::OnCompAppend)
EVT_BUTTON(ID_CellNewIDXBtn, TraceDlg::OnCellNewIDX)
EVT_BUTTON(ID_CompAppend2Btn, TraceDlg::OnCompAppend)
EVT_CHECKBOX(ID_AutoIDChk, TraceDlg::OnAutoIDChk)
EVT_BUTTON(ID_CellNewIDBtn, TraceDlg::OnCellNewID)
EVT_BUTTON(ID_CellAppendIDBtn, TraceDlg::OnCellAppendID)
EVT_BUTTON(ID_CellCombineIDBtn, TraceDlg::OnCellCombineID)
EVT_BUTTON(ID_CellDivideIDBtn, TraceDlg::OnCellDivideID)
EVT_BUTTON(ID_CellSegmentBtn, TraceDlg::OnCellSegment)
//analysis page
EVT_BUTTON(ID_AnalyzeBtn, TraceDlg::OnAnalyze)
EVT_BUTTON(ID_SaveAnalyzeBtn, TraceDlg::OnSaveAnalyze)
EVT_BUTTON(ID_ConvertToRulersBtn, TraceDlg::OnConvertToRulers)
//magic tool
//EVT_BUTTON(ID_CellMagic0Btn, TraceDlg::OnCellMagic0Btn)
//EVT_BUTTON(ID_CellMagic1Btn, TraceDlg::OnCellMagic1Btn)
//EVT_BUTTON(ID_CellMagic2Btn, TraceDlg::OnCellMagic2Btn)
//EVT_BUTTON(ID_CellMagic3Btn, TraceDlg::OnCellMagic3Btn)
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
		wxDefaultPosition, wxSize(60, 23));
	m_save_trace_btn = new wxButton(page, ID_SaveTraceBtn, "Save",
		wxDefaultPosition, wxSize(60, 23));
	m_saveas_trace_btn = new wxButton(page, ID_SaveasTraceBtn, "Save As",
		wxDefaultPosition, wxSize(60, 23));
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
	sizer_2->Add(5, 5);
	sizer_2->Add(st, 0, wxALIGN_CENTER);
	sizer_2->Add(m_gen_map_prg, 1, wxEXPAND);
	st = new wxStaticText(page, 0, "X",
		wxDefaultPosition, wxSize(10, -1));
	m_gen_map_spin = new wxSpinCtrl(page, ID_GenMapSpin, "1",
		wxDefaultPosition, wxSize(50, 23));
	sizer_2->Add(10, 10);
	sizer_2->Add(st, 0, wxALIGN_CENTER);
	sizer_2->Add(m_gen_map_spin, 0, wxALIGN_CENTER);
	st = new wxStaticText(page, 0, "times",
		wxDefaultPosition, wxSize(40, -1));
	m_gen_map_btn = new wxButton(page, ID_GenMapBtn, "Generate",
		wxDefaultPosition, wxSize(60, 23));
	m_refine_map_btn = new wxButton(page, ID_RefineMapBtn, "Refine",
		wxDefaultPosition, wxSize(60, 23));
	sizer_2->Add(10, 10);
	sizer_2->Add(st, 0, wxALIGN_CENTER);
	sizer_2->Add(m_gen_map_btn, 0, wxALIGN_CENTER);
	sizer_2->Add(m_refine_map_btn, 0, wxALIGN_CENTER);

	//vertical sizer
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
	sizer_v->Add(10, 10);
	sizer_v->Add(sizer_1, 0, wxEXPAND);
	sizer_v->Add(10, 10);
	sizer_v->Add(sizer_2, 0, wxEXPAND);
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
		wxDefaultPosition, wxSize(65, 23));
	m_comp_exclusive_btn = new wxButton(page, ID_CompExclusiveBtn, "Replace",
		wxDefaultPosition, wxSize(65, 23));
	m_comp_append_btn = new wxButton(page, ID_CompAppendBtn, "Append",
		wxDefaultPosition, wxSize(65, 23));
	m_comp_clear_btn = new wxButton(page, ID_CompClearBtn, "Clear",
		wxDefaultPosition, wxSize(65, 23));
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

	//vertical sizer
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
	sizer_v->Add(10, 10);
	sizer_v->Add(sizer_1, 0, wxEXPAND);
	sizer_v->Add(10, 10);
	sizer_v->Add(sizer_2, 0, wxEXPAND);
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
		wxDefaultPosition, wxSize(65, 23));
	m_comp_clear_btn = new wxButton(page, ID_CompClearBtn, "Clear",
		wxDefaultPosition, wxSize(65, 23));
	m_manual_assist_check = new wxCheckBox(page, ID_ManualAssistCheck, "Auto Link",
		wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
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
		wxDefaultPosition, wxSize(65, 23));
	m_cell_link_btn = new wxButton(page, ID_CellLinkBtn, "Link IDs",
		wxDefaultPosition, wxSize(65, 23));
	m_cell_isolate_btn = new wxButton(page, ID_CellIsolateBtn, "Isolate",
		wxDefaultPosition, wxSize(65, 23));
	m_cell_unlink_btn = new wxButton(page, ID_CellUnlinkBtn, "Unlink IDs",
		wxDefaultPosition, wxSize(65, 23));
	sizer_2->AddStretchSpacer();
	sizer_2->Add(m_cell_exclusive_link_btn, 0, wxALIGN_CENTER);
	sizer_2->Add(m_cell_link_btn, 0, wxALIGN_CENTER);
	sizer_2->Add(m_cell_isolate_btn, 0, wxALIGN_CENTER);
	sizer_2->Add(m_cell_unlink_btn, 0, wxALIGN_CENTER);
	sizer_2->AddStretchSpacer();

	//vertical sizer
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
	sizer_v->Add(10, 10);
	sizer_v->Add(sizer_1, 0, wxEXPAND);
	sizer_v->Add(10, 10);
	sizer_v->Add(sizer_2, 0, wxEXPAND);
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
		wxDefaultPosition, wxSize(65, 23));
	m_comp_clear_btn = new wxButton(page, ID_CompClearBtn, "Clear",
		wxDefaultPosition, wxSize(65, 23));
	m_auto_id_chk = new wxCheckBox(page, ID_AutoIDChk, "Auto Assign ID");
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
		wxDefaultPosition, wxSize(65, 23));
	m_cell_append_id_btn = new wxButton(page, ID_CellAppendIDBtn, "Add ID",
		wxDefaultPosition, wxSize(65, 23));
	m_cell_combine_id_btn = new wxButton(page, ID_CellCombineIDBtn, "Combine",
		wxDefaultPosition, wxSize(65, 23));
	m_cell_divide_id_btn = new wxButton(page, ID_CellDivideIDBtn, "Divide",
		wxDefaultPosition, wxSize(65, 23));
	m_cell_segment_btn = new wxButton(page, ID_CellSegmentBtn, "Segment",
		wxDefaultPosition, wxSize(65, 23));
	sizer_2->AddStretchSpacer();
	sizer_2->Add(m_cell_new_id_btn, 0, wxALIGN_CENTER);
	sizer_2->Add(m_cell_append_id_btn, 0, wxALIGN_CENTER);
	sizer_2->Add(m_cell_combine_id_btn, 0, wxALIGN_CENTER);
	sizer_2->Add(m_cell_divide_id_btn, 0, wxALIGN_CENTER);
	sizer_2->Add(m_cell_segment_btn, 0, wxALIGN_CENTER);
	m_cell_segment_btn->Hide();
	sizer_2->AddStretchSpacer();

	//vertical sizer
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
	sizer_v->Add(10, 10);
	sizer_v->Add(sizer_1, 0, wxEXPAND);
	sizer_v->Add(10, 10);
	sizer_v->Add(sizer_2, 0, wxEXPAND);
	sizer_v->Add(10, 10);

	//set the page
	page->SetSizer(sizer_v);
	return page;
}

wxWindow* TraceDlg::CreateAnalysisPage(wxWindow *parent)
{
	wxPanel *page = new wxPanel(parent);

	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	m_convert_to_rulers_btn = new wxButton(page, ID_ConvertToRulersBtn, "To Rulers",
		wxDefaultPosition, wxSize(65, 23));
	m_analyze_btn = new wxButton(page, ID_AnalyzeBtn, "Analyze",
		wxDefaultPosition, wxSize(65, 23));
	m_save_analyze_btn = new wxButton(page, ID_SaveAnalyzeBtn, "Save Text",
		wxDefaultPosition, wxSize(65, 23));
	sizer_1->AddStretchSpacer();
	sizer_1->Add(m_convert_to_rulers_btn, 0, wxALIGN_CENTER);
	sizer_1->Add(m_analyze_btn, 0, wxALIGN_CENTER);
	sizer_1->Add(m_save_analyze_btn, 0, wxALIGN_CENTER);
	sizer_1->AddStretchSpacer();

	//vertical sizer
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
	sizer_v->Add(10, 10);
	sizer_v->Add(sizer_1, 0, wxEXPAND);

	//set the page
	page->SetSizer(sizer_v);
	return page;
}

TraceDlg::TraceDlg(wxWindow* frame, wxWindow* parent)
	: wxPanel(parent, wxID_ANY,
		wxPoint(500, 150), wxSize(500, 600),
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
	m_notebook->AddPage(CreateMapPage(m_notebook), "Track Map");
	m_notebook->AddPage(CreateSelectPage(m_notebook), "Selection");
	m_notebook->AddPage(CreateLinkPage(m_notebook), "Linkage");
	m_notebook->AddPage(CreateModifyPage(m_notebook), "Modify");
	m_notebook->AddPage(CreateAnalysisPage(m_notebook), "Analysis");

	wxStaticText *st = 0;

	//ghost num
	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Ghosts:",
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
	m_cell_prev_btn = new wxButton(this, ID_CellPrevBtn, "Backward <",
		wxDefaultPosition, wxSize(80, 23));
	m_cell_next_btn = new wxButton(this, ID_CellNextBtn, "Forward >",
		wxDefaultPosition, wxSize(80, 23));
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
		wxString str = trace_group->GetPath();
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
	m_manual_assist_check->SetValue(m_manual_assist);
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
					item_id = m_trace_list_curr->GetText(item, 0);
					item_size = m_trace_list_curr->GetText(item, 1);
					item_x = m_trace_list_curr->GetText(item, 2);
					item_y = m_trace_list_curr->GetText(item, 3);
					item_z = m_trace_list_curr->GetText(item, 4);
					item_id.ToULong(&id);
					hue = id % 360;
					Color c(HSVColor(hue, 1.0, 1.0));
					wxColor color(c.r() * 255, c.g() * 255, c.b() * 255);
					item_size.ToLong(&size);
					item_x.ToDouble(&x);
					item_y.ToDouble(&y);
					item_z.ToDouble(&z);
					m_trace_list_prev->Append(id, color, size, x, y, z);
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

//auto tracking
void TraceDlg::OnGenMapBtn(wxCommandEvent &event)
{
	GenMap();
}

void TraceDlg::OnRefineMapBtn(wxCommandEvent &event)
{
	RefineMap();
}

//analysis
void TraceDlg::OnAnalyze(wxCommandEvent &event)
{
	Measure();
	wxString str;
	OutputMeasureResult(str);
	m_stat_text->SetValue(str);
}

void TraceDlg::OnSaveAnalyze(wxCommandEvent &event)
{
	Measure();
	wxString str;
	OutputMeasureResult(str);
	m_stat_text->SetValue(str);
	wxFileDialog *fopendlg = new wxFileDialog(
		m_frame, "Save results", "", "",
		"Text file (*.txt)|*.txt",
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		wxString filename = fopendlg->GetPath();
		SaveMeasureResult(filename);
	}
	if (fopendlg)
		delete fopendlg;
}

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
	FL::RulerList rulers;
	trace_group->GetMappedRulers(rulers);
	for (FL::RulerListIter iter = rulers.begin();
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

//manual tracking assist
void TraceDlg::OnManualAssistCheck(wxCommandEvent &event)
{
	m_manual_assist = m_manual_assist_check->GetValue();
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
			str = m_trace_list_curr->GetItemText(item);
			if (str.ToULong(&ival) && ival)
				ids.push_back(ival);
		}
	}

	bool clear_all = ids.empty();

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
	//select append
	int nx, ny, nz;
	vd->GetResolution(nx, ny, nz);
	unsigned long long index;
	unsigned long long for_size = (unsigned long long)nx *
		(unsigned long long)ny * (unsigned long long)nz;
	for (index = 0; index < for_size; ++index)
	{
		if (clear_all)
			data_mask[index] = 0;
		else if (find(ids.begin(), ids.end(), data_label[index])
			!= ids.end())
			data_mask[index] = 255;
		else
			data_mask[index] = 0;
	}
	//invalidate label mask in gpu
	vd->GetVR()->clear_tex_pool();
	//update view
	CellUpdate();
}

void TraceDlg::CompClear()
{
	VRenderFrame* frame = (VRenderFrame*)m_frame;
	if (frame && frame->GetTree())
		frame->GetTree()->BrushClear();
	CellUpdate();
	m_trace_list_prev->DeleteAllItems();
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

	//get id
	wxString str;
	if (event.GetId() == ID_CompAppend2Btn ||
		event.GetId() == ID_CellNewIDText)
		str = m_cell_new_id_text->GetValue();
	else
		str = m_comp_id_text->GetValue();

	if (!str.empty())
	{
		bool get_all = false;
		unsigned long ival = 0;
		if (str.Lower() == "all")
			get_all = true;
		else
		{
			str.ToULong(&ival);
			if (ival == 0)
				return;
		}

		unsigned int id = ival;
		//get current mask
		VolumeData* vd = m_view->m_glview->m_cur_vol;
		if (!vd)
			return;
		Nrrd* nrrd_mask = vd->GetMask(true);
		if (!nrrd_mask)
		{
			vd->AddEmptyMask();
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
			return;
		unsigned int* data_label = (unsigned int*)(nrrd_label->data);
		if (!data_label)
			return;
		//select append
		int nx, ny, nz;
		vd->GetResolution(nx, ny, nz);
		unsigned long long for_size = (unsigned long long)nx*
			(unsigned long long)ny * (unsigned long long)nz;
		for (unsigned long long index = 0;
		index < for_size; ++index)
		{
			if (get_all)
			{
				if (data_label[index])
					data_mask[index] = 255;
			}
			else
			{
				if (data_label[index] == id)
					data_mask[index] = 255;
			}
		}
		//invalidate label mask in gpu
		vd->GetVR()->clear_tex_pool();
	}

	//update view
	CellUpdate();
}

void TraceDlg::OnCompExclusive(wxCommandEvent &event)
{
	if (!m_view)
		return;

	//get id
	wxString str = m_comp_id_text->GetValue();
	unsigned long ival;
	str.ToULong(&ival);
	if (ival != 0)
	{
		unsigned int id = ival;
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
		//select append
		int nx, ny, nz;
		vd->GetResolution(nx, ny, nz);
		unsigned long long for_size = (unsigned long long)nx *
			(unsigned long long)ny * (unsigned long long)nz;
		unsigned long long index;
		for (index = 0; index < for_size; ++index)
		{
			if (data_label[index] == id)
				data_mask[index] = 255;
			else
				data_mask[index] = 0;
		}
		//invalidate label mask in gpu
		vd->GetVR()->clear_tex_pool();
	}

	//update view
	CellUpdate();
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
	//get selected IDs
	int i, j, k;
	int nx, ny, nz;
	unsigned long long index;
	vd->GetResolution(nx, ny, nz);
	unsigned int label_value;
	FL::CellList sel_labels;
	FL::CellListIter label_iter;
	for (i = 0; i < nx; ++i)
		for (j = 0; j < ny; ++j)
			for (k = 0; k < nz; ++k)
			{
				index = nx*ny*k + nx*j + i;
				if (data_mask[index] &&
					data_label[index])
				{
					label_value = data_label[index];
					label_iter = sel_labels.find(label_value);
					if (label_iter == sel_labels.end())
					{
						FL::pCell cell(new FL::Cell(label_value));
						cell->Inc(i, j, k, 1.0f);
						sel_labels.insert(pair<unsigned int, FL::pCell>
							(label_value, cell));
					}
					else
						label_iter->second->Inc(i, j, k, 1.0f);
				}
			}

	//reselect
	for (i = 0; i < nx; ++i)
		for (j = 0; j < ny; ++j)
			for (k = 0; k<nz; ++k)
			{
				index = nx*ny*k + nx*j + i;
				if (data_label[index])
				{
					label_value = data_label[index];
					label_iter = sel_labels.find(label_value);
					if (label_iter != sel_labels.end() &&
						label_iter->second->GetSizeUi() > slimit)
						data_mask[index] = 255;
					else
						data_mask[index] = 0;
				}
			}
	//invalidate label mask in gpu
	vd->GetVR()->clear_tex_pool();
	//update view
	CellUpdate();
}

void TraceDlg::AddLabel(long item, TraceListCtrl* trace_list_ctrl, FL::CellList &list)
{
	wxString str;
	unsigned long id;
	unsigned long size;
	double x, y, z;

	str = trace_list_ctrl->GetText(item, 0);
	str.ToULong(&id);
	str = trace_list_ctrl->GetText(item, 1);
	str.ToULong(&size);
	str = trace_list_ctrl->GetText(item, 2);
	str.ToDouble(&x);
	str = trace_list_ctrl->GetText(item, 3);
	str.ToDouble(&y);
	str = trace_list_ctrl->GetText(item, 4);
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
		vd->AddEmptyMask();
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
	if (inc)
		while (vd->SearchLabel(new_id))
			new_id += inc;

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
					if (append && data_label[index])
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
	BaseReader* reader = vd->GetReader();
	if (reader)
	{
		wxString data_name = reader->GetCurName(m_cur_time, vd->GetCurChannel());
		wxString label_name = data_name.Left(data_name.find_last_of('.')) + ".lbl";
		MSKWriter msk_writer;
		msk_writer.SetData(nrrd_label);
		msk_writer.Save(label_name.ToStdWstring(), 1);
	}

	CellUpdate();
	//update view
	m_view->RefreshGL();
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
		BaseReader* reader = vd->GetReader();
		if (reader)
		{
			wxString data_name = reader->GetCurName(m_cur_time, vd->GetCurChannel());
			wxString label_name = data_name.Left(data_name.find_last_of('.')) + ".lbl";
			MSKWriter msk_writer;
			msk_writer.SetData(nrrd_label);
			msk_writer.Save(label_name.ToStdWstring(), 1);
		}
	}

	CellUpdate();
	//update view
	m_view->RefreshGL();
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
}

//ID edit controls
void TraceDlg::OnCellNewIDX(wxCommandEvent &event)
{
	m_cell_new_id_text->Clear();
}

void TraceDlg::OnAutoIDChk(wxCommandEvent &event)
{
	m_auto_id = m_auto_id_chk->GetValue();
}

void TraceDlg::OnCellNewID(wxCommandEvent &event)
{
	CellNewID(false);
}

void TraceDlg::OnCellAppendID(wxCommandEvent &event)
{
	CellNewID(true);
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
	BaseReader* reader = vd->GetReader();
	if (reader)
	{
		wxString data_name = reader->GetCurName(m_cur_time, vd->GetCurChannel());
		wxString label_name = data_name.Left(data_name.find_last_of('.')) + ".lbl";
		MSKWriter msk_writer;
		msk_writer.SetData(nrrd_label);
		msk_writer.Save(label_name.ToStdWstring(), 1);
	}

	//modify graphs
	trace_group->CombineCells(cell, list_cur,
		m_cur_time);

	//update view
	CellUpdate();
}

void TraceDlg::OnCellDivideID(wxCommandEvent& event)
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

void TraceDlg::OnCellSegment(wxCommandEvent& event)
{

}

//magic
/*void TraceDlg::OnCellMagic0Btn(wxCommandEvent &event)
{
	Measure();
	wxString str;
	OutputMeasureResult(str);
	m_stat_text->SetValue(str);
	wxFileDialog *fopendlg = new wxFileDialog(
		m_frame, "Save results", "", "",
		"Text file (*.txt)|*.txt",
		wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		wxString filename = fopendlg->GetPath();
		SaveMeasureResult(filename);
	}
	if (fopendlg)
		delete fopendlg;
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

void TraceDlg::Measure()
{
	if (!m_view)
		return;

	//get data
	VolumeData* vd = m_view->m_glview->m_cur_vol;
	if (!vd)
		return;
	Texture* tex = vd->GetTexture();
	if (!tex)
		return;
	Nrrd* nrrd_data = tex->get_nrrd(0);
	if (!nrrd_data)
		return;
	int bits = nrrd_data->type;
	void* data_data = nrrd_data->data;
	if (!data_data)
		return;
	//get mask
	Nrrd* nrrd_mask = vd->GetMask(true);
	if (!nrrd_mask)
		return;
	unsigned char* data_mask = (unsigned char*)(nrrd_mask->data);
	if (!data_mask)
		return;
	//get label
	Nrrd* nrrd_label = tex->get_nrrd(tex->nlabel());
	if (!nrrd_label)
		return;
	unsigned int* data_label = (unsigned int*)(nrrd_label->data);
	if (!data_label)
		return;

	//clear list and start calculating
	m_info_list.clear();
	int ilist;
	int found;
	int nx, ny, nz;
	unsigned int id;
	double value;
	double delta;
	double ext;
	int i, j, k;
	vd->GetResolution(nx, ny, nz);
	unsigned long long for_size = (unsigned long long)nx *
		(unsigned long long)ny * (unsigned long long)nz;
	unsigned long long index;
	for (index = 0; index < for_size; ++index)
	{
		if (!data_mask[index] ||
			!data_label[index])
			continue;

		id = data_label[index];
		if (bits == nrrdTypeUChar)
			value = ((unsigned char*)data_data)[index];
		else if (bits == nrrdTypeUShort)
			value = ((unsigned short*)data_data)[index];

		if (value <= 1.0)
			continue;

		k = index / (nx*ny);
		j = index % (nx*ny);
		i = j % nx;
		j = j / nx;
		ext = GetExt(data_label, index, id, nx, ny, nz, i, j, k);

		//find in list
		found = -1;
		for (ilist = 0; ilist < (int)m_info_list.size(); ++ilist)
		{
			if (m_info_list[ilist].id == id)
			{
				found = ilist;
				break;
			}
		}
		if (found == -1)
		{
			//not found
			measure_info info;
			info.id = id;
			info.total_num = 1;
			info.mean = 0.0;
			info.variance = 0.0;
			info.m2 = 0.0;
			delta = value - info.mean;
			info.mean += delta / info.total_num;
			info.m2 += delta * (value - info.mean);
			info.min = value;
			info.max = value;
			info.ext_sum = ext;
			m_info_list.push_back(info);
		}
		else
		{
			m_info_list[found].total_num++;
			delta = value - m_info_list[found].mean;
			m_info_list[found].mean += delta / m_info_list[found].total_num;
			m_info_list[found].m2 += delta * (value - m_info_list[found].mean);
			m_info_list[found].min = value < m_info_list[found].min ? value : m_info_list[found].min;
			m_info_list[found].max = value > m_info_list[found].max ? value : m_info_list[found].max;
			m_info_list[found].ext_sum += ext;
		}
	}

	for (size_t i = 0; i < m_info_list.size(); ++i)
	{
		if (m_info_list[i].total_num > 0)
			m_info_list[i].variance = sqrt(m_info_list[i].m2 / (m_info_list[i].total_num));
	}

	std::sort(m_info_list.begin(), m_info_list.end(), measure_info::cmp_id);
}

void TraceDlg::OutputMeasureResult(wxString &str)
{
	str = "Statistics on the selection:\n";
	str += "A total of " +
		wxString::Format("%u", m_info_list.size()) +
		" component(s) selected\n";
	str += "ID\tTotalN\tSurfaceN\tMean\tSigma\tMinimum\tMaximum\n";
	for (size_t i = 0; i < m_info_list.size(); ++i)
	{
		str += wxString::Format("%u\t", m_info_list[i].id);
		str += wxString::Format("%u\t", m_info_list[i].total_num);
		str += wxString::Format("%.0f\t", m_info_list[i].ext_sum);
		str += wxString::Format("%.2f\t", m_info_list[i].mean);
		str += wxString::Format("%.2f\t", m_info_list[i].variance);
		str += wxString::Format("%.2f\t", m_info_list[i].min);
		str += wxString::Format("%.2f\n", m_info_list[i].max);
	}

}

void TraceDlg::SaveMeasureResult(wxString &filename)
{
	if (m_info_list.empty())
		return;

	wxFileOutputStream fos(filename);
	if (!fos.Ok())
		return;
	wxTextOutputStream tos(fos);

	wxString str;
	OutputMeasureResult(str);

	tos << str;
}

double TraceDlg::GetExt(unsigned int* data_label,
	unsigned long long index,
	unsigned int id,
	int nx, int ny, int nz,
	int i, int j, int k)
{
	bool surface_vox, contact_vox;
	unsigned long long indexn;
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

	return surface_vox ? 1.0 : 0.0;
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

	VolumeData* vd = m_view->m_glview->m_cur_vol;
	if (!vd)
		return;
	BaseReader* reader = vd->GetReader();
	if (!reader)
		return;
	LBLReader lbl_reader;
	m_view->CreateTraceGroup();
	TraceGroup *trace_group = m_view->GetTraceGroup();
	if (!trace_group)
		return;

	m_stat_text->SetValue("Generating track map.\n");
	wxGetApp().Yield();
	FL::TrackMap &track_map = trace_group->GetTrackMap();
	FL::TrackMapProcessor tm_processor;
	int chan = vd->GetCurChannel();
	int frames = reader->GetTimeNum();
	int resx, resy, resz;
	vd->GetResolution(resx, resy, resz);

	Nrrd* nrrd_data1 = 0;
	Nrrd* nrrd_data2 = 0;
	Nrrd* nrrd_label1 = 0;
	Nrrd* nrrd_label2 = 0;
	wxString data_name;
	wxString label_name;
	wstring lblname;
	bool file_err = false;

	size_t iter_num = (size_t)m_gen_map_spin->GetValue();
	iter_num *= 2;
	tm_processor.SetSizes(track_map,
		resx, resy, resz);
	//	tm_processor.SetContactThresh(0.2f);
	float prog_bit = 100.0f / float(frames * (2 + iter_num));
	float prog = 0.0f;
	m_gen_map_prg->SetValue(int(prog));
	for (int i = 0; i < frames; ++i)
	{
		if (i == 0)
		{
			nrrd_data1 = reader->Convert(i, chan, true);
			if (!nrrd_data1)
			{
				file_err = true;
				continue;
			}
			data_name = reader->GetCurName(i, chan);
			label_name = data_name.Left(data_name.find_last_of('.')) + ".lbl";
			lblname = label_name.ToStdWstring();
			lbl_reader.SetFile(lblname);
			nrrd_label1 = lbl_reader.Convert(i, chan, true);
			if (!nrrd_label1)
			{
				file_err = true;
				continue;
			}
			tm_processor.InitializeFrame(track_map,
				nrrd_data1->data, nrrd_label1->data, i);
		}
		else
		{
			nrrd_data2 = reader->Convert(i, chan, true);
			if (!nrrd_data2)
			{
				file_err = true;
				continue;
			}
			data_name = reader->GetCurName(i, chan);
			label_name = data_name.Left(data_name.find_last_of('.')) + ".lbl";
			lblname = label_name.ToStdWstring();
			lbl_reader.SetFile(lblname);
			nrrd_label2 = lbl_reader.Convert(i, chan, true);
			if (!nrrd_label2)
			{
				file_err = true;
				continue;
			}
			tm_processor.InitializeFrame(track_map,
				nrrd_data2->data, nrrd_label2->data, i);

			//link maps 1 and 2
			tm_processor.LinkMaps(track_map, i - 1, i,
				nrrd_data1->data, nrrd_data2->data,
				nrrd_label1->data, nrrd_label2->data);

			nrrdNuke(nrrd_data1);
			nrrdNuke(nrrd_label1);
			nrrd_data1 = nrrd_data2;
			nrrd_label1 = nrrd_label2;
		}
		prog += prog_bit;
		m_gen_map_prg->SetValue(int(prog));
		(*m_stat_text) << wxString::Format("Time point %d initialized.\n", i);
		wxGetApp().Yield();
	}

	if (file_err)
		(*m_stat_text) << "ERROR! Certain file(s) missing. Check if label files exist.\n";

	nrrdNuke(nrrd_data2);
	nrrdNuke(nrrd_label2);

	//resolve multiple links of single vertex
	for (size_t fi = 0; fi < track_map.GetFrameNum(); ++fi)
	{
		tm_processor.ResolveGraph(track_map, fi, fi + 1);
		tm_processor.ResolveGraph(track_map, fi, fi - 1);
		prog += prog_bit;
		m_gen_map_prg->SetValue(int(prog));
		(*m_stat_text) << wxString::Format("Time point %d resolved.\n", int(fi));
		wxGetApp().Yield();
	}

	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	//check branch
	for (size_t iteri = 0; iteri < iter_num; ++iteri)
	{
		for (size_t fi = 0; fi < track_map.GetFrameNum(); ++fi)
		{
			tm_processor.MatchFrames(track_map, fi, fi + 1, iteri != 0);
			tm_processor.MatchFrames(track_map, fi, fi - 1, iteri != 0);
			prog += prog_bit;
			m_gen_map_prg->SetValue(int(prog));
			(*m_stat_text) << wxString::Format("Time point %d linked.\n", int(fi));
			wxGetApp().Yield();
		}

		if (++iteri >= iter_num)
			break;

		for (size_t fi = 0; fi < track_map.GetFrameNum(); ++fi)
		{
			tm_processor.UnmatchFrames(track_map, fi, fi - 1);
			tm_processor.UnmatchFrames(track_map, fi, fi + 1);
			//link orphans
			tm_processor.ExMatchFrames(track_map, fi, fi + 1);
			tm_processor.ExMatchFrames(track_map, fi, fi - 1);
			prog += prog_bit;
			m_gen_map_prg->SetValue(int(prog));
			(*m_stat_text) << wxString::Format("Time point %d unlinked.\n", int(fi));
			wxGetApp().Yield();
		}
	}
	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
	(*m_stat_text) << wxString::Format("Wall clock time: %.4fs\n", time_span.count());

	m_gen_map_prg->SetValue(100);

	GetSettings(m_view);
}

#define LINK_FRAMES \
	for (size_t fi = 0; fi < frames; ++fi) \
	{ \
		tm_processor.MatchFrames(track_map, fi, fi + 1, false); \
		tm_processor.MatchFrames(track_map, fi, fi - 1, false); \
		prog += prog_bit; \
		m_gen_map_prg->SetValue(int(prog)); \
		(*m_stat_text) << wxString::Format("Time point %d linked.\n", int(fi)); \
		wxGetApp().Yield(); \
	}

#define UNLINK_FRAMES \
	for (size_t fi = 0; fi < frames; ++fi) \
	{ \
		tm_processor.UnmatchFrames(track_map, fi, fi - 1); \
		tm_processor.UnmatchFrames(track_map, fi, fi + 1); \
		tm_processor.ExMatchFrames(track_map, fi, fi + 1); \
		tm_processor.ExMatchFrames(track_map, fi, fi - 1); \
		prog += prog_bit; \
		m_gen_map_prg->SetValue(int(prog)); \
		(*m_stat_text) << wxString::Format("Time point %d unlinked.\n", int(fi)); \
		wxGetApp().Yield(); \
	}

void TraceDlg::RefineMap()
{
	if (!m_view)
		return;

	VolumeData* vd = m_view->m_glview->m_cur_vol;
	if (!vd)
		return;
	TraceGroup *trace_group = m_view->GetTraceGroup();
	if (!trace_group)
		return;
	m_stat_text->SetValue("Refining track map.\n");
	wxGetApp().Yield();
	FL::TrackMap &track_map = trace_group->GetTrackMap();
	FL::TrackMapProcessor tm_processor;
	int frames = track_map.GetFrameNum();
	int resx, resy, resz;
	vd->GetResolution(resx, resy, resz);
	size_t iter_num = (size_t)m_gen_map_spin->GetValue();
	iter_num *= 2;
	tm_processor.SetSizes(track_map,
		resx, resy, resz);
	//	tm_processor.SetContactThresh(0.2f);
	float prog_bit = 100.0f / float(frames * iter_num);
	float prog = 0.0f;
	m_gen_map_prg->SetValue(int(prog));

	unsigned int last_op = track_map.GetLastOp();
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	//check branch
	for (size_t iteri = 0; iteri < iter_num; ++iteri)
	{
		if (last_op == 1)
			UNLINK_FRAMES
		else
			LINK_FRAMES

			if (++iteri >= iter_num)
				break;

		if (last_op == 1)
			LINK_FRAMES
		else
			UNLINK_FRAMES;
	}
	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
	(*m_stat_text) << wxString::Format("Wall clock time: %.4fs\n", time_span.count());

	m_gen_map_prg->SetValue(100);
}