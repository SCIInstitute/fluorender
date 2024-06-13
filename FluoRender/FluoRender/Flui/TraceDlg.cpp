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
#include <TraceDlg.h>
#include <Global.h>
#include <MainFrame.h>
#include <RenderCanvas.h>
#include <BrushToolDlg.h>
#include <MeasureDlg.h>
#include <ComponentDlg.h>
#include <MoviePanel.h>
#include <CompEditor.h>
#include <wxSingleSlider.h>
#include <wx/valnum.h>
#include <wx/clipbrd.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/dirdlg.h>
#include <png_resource.h>
#include <icons.h>
#include <set>
#include <limits>
#include <chrono>

BEGIN_EVENT_TABLE(TraceListCtrl, wxListCtrl)
EVT_KEY_DOWN(TraceListCtrl::OnKeyDown)
EVT_CONTEXT_MENU(TraceListCtrl::OnContextMenu)
EVT_MENU(Menu_CopyText, TraceListCtrl::OnCopySelection)
EVT_MENU(Menu_Delete, TraceListCtrl::OnDeleteSelection)
END_EVENT_TABLE()

TraceListCtrl::TraceListCtrl(
	MainFrame* frame,
	wxWindow* parent,
	const wxPoint& pos,
	const wxSize& size,
	long style) :
	wxListCtrl(parent, wxID_ANY, pos, size, style),
	m_type(0)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);
	SetDoubleBuffered(true);

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

void TraceListCtrl::UpdateTraces(RenderCanvas* vrv)
{
	m_view = vrv;
	if (!m_view)
		return;

	TraceGroup* traces = m_view->GetTraceGroup();
	if (!traces)
		return;
	int shuffle = 0;
	if (m_view->m_cur_vol)
		shuffle = m_view->m_cur_vol->GetShuffle();

	DeleteAllItems();

	flrd::CelpList sel_cells = traces->GetCellList();
	std::vector<flrd::Celp> cells;
	for (auto siter = sel_cells.begin();
	siter != sel_cells.end(); ++siter)
		cells.push_back(siter->second);

	if (cells.empty())
		return;
	else
		std::sort(cells.begin(), cells.end(),
		[](const flrd::Celp &c1, const flrd::Celp &c2) -> bool
	{
		unsigned int vid1 = c1->GetVertexId();
		unsigned int vid2 = c2->GetVertexId();
		if (vid1 == vid2)
			return c1->GetSizeUi() > c2->GetSizeUi();
		else
			return vid1 < vid2;
	});

	wxString gtype;
	unsigned int id;
	unsigned int vid;
	fluo::Color c;
	wxColor wxc;
	int size;
	fluo::Point center;
	bool prev, next;

	Freeze();
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
	Thaw();
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
EVT_BUTTON(ID_ClearTraceBtn, TraceDlg::OnClearTrace)
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
	page->SetBackgroundColour(((wxNotebook*)parent)->GetThemeBackgroundColour());

	wxStaticText *st = 0;

	//load trace
	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Track map:",
		wxDefaultPosition, FromDIP(wxSize(70, 20)));
	m_load_trace_text = new wxTextCtrl(page, ID_LoadTraceText, "",
		wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_clear_trace_btn = new wxButton(page, ID_ClearTraceBtn, "X",
		wxDefaultPosition, FromDIP(wxSize(23, 23)));
	m_load_trace_btn = new wxButton(page, ID_LoadTraceBtn, "Load",
		wxDefaultPosition, FromDIP(wxSize(65, 23)));
	m_save_trace_btn = new wxButton(page, ID_SaveTraceBtn, "Save",
		wxDefaultPosition, FromDIP(wxSize(65, 23)));
	m_saveas_trace_btn = new wxButton(page, ID_SaveasTraceBtn, "Save As",
		wxDefaultPosition, FromDIP(wxSize(65, 23)));
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
		wxDefaultPosition, FromDIP(wxSize(50, 23)));
	sizer_2->Add(5, 5);
	sizer_2->Add(st, 0, wxALIGN_CENTER);
	sizer_2->Add(5, 5);
	sizer_2->Add(m_map_iter_spin, 0, wxALIGN_CENTER);
	st = new wxStaticText(page, 0, "Size Threshold:",
		wxDefaultPosition, wxDefaultSize);
	m_map_size_spin = new wxSpinCtrl(page, ID_MapSizeSpin, "100",
		wxDefaultPosition, FromDIP(wxSize(50, 23)));
	m_map_size_spin->SetRange(1, std::numeric_limits<int>::max());
	sizer_2->Add(5, 5);
	sizer_2->Add(st, 0, wxALIGN_CENTER);
	sizer_2->Add(5, 5);
	sizer_2->Add(m_map_size_spin, 0, wxALIGN_CENTER);
	m_gen_map_btn = new wxButton(page, ID_GenMapBtn, "Generate",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_refine_t_btn = new wxButton(page, ID_RefineTBtn, "Refine T",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_refine_all_btn = new wxButton(page, ID_RefineAllBtn, "Refine All",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
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
		wxDefaultPosition, FromDIP(wxSize(50, 23)),
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
		wxDefaultPosition, FromDIP(wxSize(50, 23)),
		wxSP_ARROW_KEYS | wxSP_WRAP,
		0, 1, 0.6, 0.01);
	sizer_3->Add(5, 5);
	sizer_3->Add(st, 0, wxALIGN_CENTER);
	sizer_3->Add(5, 5);
	sizer_3->Add(m_map_contact_spin, 0, wxALIGN_CENTER);
	m_map_consistent_btn = new wxToggleButton(page, ID_MapConsistentBtn,
		"Consistent Colors", wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_map_merge_btn = new wxToggleButton(
		page, ID_MapMergeBtn, "Try Merging",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_map_split_btn = new wxToggleButton(
		page, ID_MapSplitBtn, "Try Splitting",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
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
	page->SetBackgroundColour(((wxNotebook*)parent)->GetThemeBackgroundColour());

	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;
	wxStaticText *st = 0;

	//selection tools
	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Selection tools:",
		wxDefaultPosition, FromDIP(wxSize(100, 20)));
	m_comp_id_text = new wxTextCtrl(page, ID_CompIDText, "",
		wxDefaultPosition, FromDIP(wxSize(77, 23)), wxTE_PROCESS_ENTER | wxTE_RIGHT);
	m_comp_id_x_btn = new wxButton(page, ID_CompIDXBtn, "X",
		wxDefaultPosition, FromDIP(wxSize(23, 23)));
	m_comp_full_btn = new wxButton(page, ID_CompFullBtn, "FullCompt",
		wxDefaultPosition, FromDIP(wxSize(64, 23)));
	m_comp_exclusive_btn = new wxButton(page, ID_CompExclusiveBtn, "Replace",
		wxDefaultPosition, FromDIP(wxSize(64, 23)));
	m_comp_append_btn = new wxButton(page, ID_CompAppendBtn, "Append",
		wxDefaultPosition, FromDIP(wxSize(64, 23)));
	m_comp_clear_btn = new wxButton(page, ID_CompClearBtn, "Clear",
		wxDefaultPosition, FromDIP(wxSize(64, 23)));
	m_shuffle_btn = new wxButton(page, ID_ShuffleBtn, "Shuffle",
		wxDefaultPosition, FromDIP(wxSize(64, 23)));
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
		wxDefaultPosition, FromDIP(wxSize(110, 20)));
	m_cell_size_sldr = new wxSingleSlider(page, ID_CellSizeSldr, 20, 0, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_cell_size_text = new wxTextCtrl(page, ID_CellSizeText, "20",
		wxDefaultPosition, FromDIP(wxSize(60, 23)), wxTE_RIGHT, vald_int);
	sizer_2->Add(5, 5);
	sizer_2->Add(st, 0, wxALIGN_CENTER);
	sizer_2->Add(m_cell_size_sldr, 1, wxEXPAND);
	sizer_2->Add(m_cell_size_text, 0, wxALIGN_CENTER);
	//uncertainty filter
	wxBoxSizer* sizer_3 = new wxBoxSizer(wxHORIZONTAL);
	m_comp_uncertain_btn = new wxButton(page, ID_CompUncertainBtn, "Uncertainty",
		wxDefaultPosition, FromDIP(wxSize(80, 23)));
	m_comp_uncertain_low_sldr = new wxSingleSlider(page, ID_CompUncertainLowSldr, 0, 0, 20,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_comp_uncertain_low_text = new wxTextCtrl(page, ID_CompUncertainLowText, "0",
		wxDefaultPosition, FromDIP(wxSize(60, 23)), wxTE_RIGHT, vald_int);
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
	page->SetBackgroundColour(((wxNotebook*)parent)->GetThemeBackgroundColour());

	wxStaticText *st = 0;

	//selection
	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Selection tools:",
		wxDefaultPosition, FromDIP(wxSize(100, 20)));
	m_comp_id_text2 = new wxTextCtrl(page, ID_CompIDText2, "",
		wxDefaultPosition, FromDIP(wxSize(77, 23)), wxTE_PROCESS_ENTER | wxTE_RIGHT);
	m_comp_id_x_btn = new wxButton(page, ID_CompIDXBtn, "X",
		wxDefaultPosition, FromDIP(wxSize(23, 23)));
	m_comp_append_btn = new wxButton(page, ID_CompAppendBtn, "Append",
		wxDefaultPosition, FromDIP(wxSize(80, 23)));
	m_comp_clear_btn = new wxButton(page, ID_CompClearBtn, "Clear",
		wxDefaultPosition, FromDIP(wxSize(80, 23)));
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
		wxDefaultPosition, FromDIP(wxSize(80, 23)));
	m_cell_link_btn = new wxButton(page, ID_CellLinkBtn, "Link IDs",
		wxDefaultPosition, FromDIP(wxSize(80, 23)));
	m_cell_link_all_btn = new wxButton(page, ID_CellLinkAllBtn, "Link New IDs",
		wxDefaultPosition, FromDIP(wxSize(80, 23)));
	sizer_2->AddStretchSpacer();
	sizer_2->Add(m_cell_exclusive_link_btn, 0, wxALIGN_CENTER);
	sizer_2->Add(m_cell_link_btn, 0, wxALIGN_CENTER);
	sizer_2->Add(m_cell_link_all_btn, 0, wxALIGN_CENTER);
	sizer_2->AddStretchSpacer();

	//ID unlink controls
	wxBoxSizer* sizer_3 = new wxBoxSizer(wxHORIZONTAL);
	m_cell_isolate_btn = new wxButton(page, ID_CellIsolateBtn, "Isolate",
		wxDefaultPosition, FromDIP(wxSize(80, 23)));
	m_cell_unlink_btn = new wxButton(page, ID_CellUnlinkBtn, "Unlink IDs",
		wxDefaultPosition, FromDIP(wxSize(80, 23)));
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
	page->SetBackgroundColour(((wxNotebook*)parent)->GetThemeBackgroundColour());

	wxStaticText *st = 0;

	//ID input
	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "New ID/Selection:",
		wxDefaultPosition, FromDIP(wxSize(100, 20)));
	m_cell_new_id_text = new wxTextCtrl(page, ID_CellNewIDText, "",
		wxDefaultPosition, FromDIP(wxSize(77, 23)), wxTE_PROCESS_ENTER | wxTE_RIGHT);
	m_cell_new_id_x_btn = new wxButton(page, ID_CellNewIDXBtn, "X",
		wxDefaultPosition, FromDIP(wxSize(23, 23)));
	m_comp_append2_btn = new wxButton(page, ID_CompAppend2Btn, "Append",
		wxDefaultPosition, FromDIP(wxSize(80, 23)));
	m_comp_clear_btn = new wxButton(page, ID_CompClearBtn, "Clear",
		wxDefaultPosition, FromDIP(wxSize(80, 23)));
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
		wxDefaultPosition, FromDIP(wxSize(85, 23)));
	m_cell_append_id_btn = new wxButton(page, ID_CellAppendIDBtn, "Add ID",
		wxDefaultPosition, FromDIP(wxSize(85, 23)));
	m_cell_replace_id_btn = new wxButton(page, ID_CellReplaceIDBtn, "Replace ID",
		wxDefaultPosition, FromDIP(wxSize(85, 23)));
	sizer_2->AddStretchSpacer();
	sizer_2->Add(m_cell_new_id_btn, 0, wxALIGN_CENTER);
	sizer_2->Add(m_cell_append_id_btn, 0, wxALIGN_CENTER);
	sizer_2->Add(m_cell_replace_id_btn, 0, wxALIGN_CENTER);
	sizer_2->AddStretchSpacer();

	wxBoxSizer* sizer_3 = new wxBoxSizer(wxHORIZONTAL);
	m_cell_combine_id_btn = new wxButton(page, ID_CellCombineIDBtn, "Combine",
		wxDefaultPosition, FromDIP(wxSize(85, 23)));
	m_cell_separate_id_btn = new wxButton(page, ID_CellSeparateBtn, "Separate",
		wxDefaultPosition, FromDIP(wxSize(85, 23)));
	m_cell_segment_text = new wxSpinCtrl(page, ID_CellSegText, "2",
		wxDefaultPosition, FromDIP(wxSize(40, 21)));
	m_cell_segment_btn = new wxButton(page, ID_CellSegBtn, "Segment",
		wxDefaultPosition, FromDIP(wxSize(65, 23)));
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
	page->SetBackgroundColour(((wxNotebook*)parent)->GetThemeBackgroundColour());

	wxStaticText *st = 0;

	//conversion
	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Convert to:",
		wxDefaultPosition, FromDIP(wxSize(100, 20)));
	m_convert_to_rulers_btn = new wxButton(page, ID_ConvertToRulersBtn, "Rulers",
		wxDefaultPosition, FromDIP(wxSize(80, 23)));
	m_convert_consistent_btn = new wxButton(page, ID_ConvertConsistentBtn, "UniIDs",
		wxDefaultPosition, FromDIP(wxSize(80, 23)));
	sizer_1->Add(5, 5);
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	sizer_1->Add(m_convert_to_rulers_btn, 0, wxALIGN_CENTER);
	sizer_1->Add(m_convert_consistent_btn, 0, wxALIGN_CENTER);

	//analysis
	wxBoxSizer* sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Information:",
		wxDefaultPosition, FromDIP(wxSize(100, 20)));
	m_analyze_comp_btn = new wxButton(page, ID_AnalyzeCompBtn, "Compnts",
		wxDefaultPosition, FromDIP(wxSize(80, 23)));
	m_analyze_link_btn = new wxButton(page, ID_AnalyzeLinkBtn, "Links",
		wxDefaultPosition, FromDIP(wxSize(80, 23)));
	m_analyze_uncertain_hist_btn = new wxButton(page, ID_AnalyzeUncertainHistBtn, "Uncertainty",
		wxDefaultPosition, FromDIP(wxSize(80, 23)));
	m_analyze_path_btn = new wxButton(page, ID_AnalyzePathBtn, "Paths",
		wxDefaultPosition, FromDIP(wxSize(80, 23)));
	sizer_2->Add(5, 5);
	sizer_2->Add(st, 0, wxALIGN_CENTER);
	sizer_2->Add(m_analyze_comp_btn, 0, wxALIGN_CENTER);
	sizer_2->Add(m_analyze_link_btn, 0, wxALIGN_CENTER);
	sizer_2->Add(m_analyze_uncertain_hist_btn, 0, wxALIGN_CENTER);
	sizer_2->Add(m_analyze_path_btn, 0, wxALIGN_CENTER);

	//Export
	wxBoxSizer* sizer_3 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Export:",
		wxDefaultPosition, FromDIP(wxSize(100, 20)));
	m_save_result_btn = new wxButton(page, ID_SaveResultBtn, "Save As",
		wxDefaultPosition, FromDIP(wxSize(80, 23)));
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

TraceDlg::TraceDlg(MainFrame* frame)
	: wxPanel(frame, wxID_ANY,
		wxDefaultPosition,
		frame->FromDIP(wxSize(550, 650)),
		0, "TraceDlg"),
	m_frame(frame),
	m_view(0),
	//m_mask(0),
	m_cur_time(-1),
	m_prv_time(-1),
	m_cell_new_id(0),
	m_cell_new_id_empty(true)
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
		wxDefaultPosition, FromDIP(wxSize(70, 20)));
	m_ghost_show_tail_chk = new wxCheckBox(this, ID_GhostShowTailChk, "Tail",
		wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
	m_ghost_num_sldr = new wxSingleSlider(this, ID_GhostNumSldr, 10, 0, 20,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_ghost_num_text = new wxTextCtrl(this, ID_GhostNumText, "10",
		wxDefaultPosition, FromDIP(wxSize(60, 23)), wxTE_RIGHT, vald_int);
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
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_cell_next_btn = new wxButton(this, ID_CellNextBtn, L"Forward (D) \u21e8",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	sizer_21->Add(m_cell_time_curr_st, 1, wxEXPAND);
	sizer_21->Add(m_cell_prev_btn, 0, wxALIGN_CENTER);
	sizer_21->Add(m_cell_next_btn, 0, wxALIGN_CENTER);
	sizer_21->Add(m_cell_time_prev_st, 1, wxEXPAND);
	//controls
	wxBoxSizer* sizer_22 = new wxBoxSizer(wxHORIZONTAL);
	m_trace_list_curr = new TraceListCtrl(frame, this);
	m_trace_list_curr->m_type = 0;
	m_trace_list_prev = new TraceListCtrl(frame, this);
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
		wxDefaultPosition, FromDIP(wxSize(-1, 100)), wxTE_MULTILINE);
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

void TraceDlg::GetSettings(RenderCanvas* vrv)
{
	m_view = vrv;
	if (!m_view)
		return;

	m_cur_time = m_view->m_tseq_cur_num;
	m_prv_time = m_view->m_tseq_prv_num;
	m_cell_size_text->ChangeValue(wxString::Format("%.0f", glbin_settings.m_component_size));

	TraceGroup* trace_group = m_view->GetTraceGroup();
	if (trace_group)
	{
		wxString str;
		trace_group->SetCellSize(glbin_settings.m_component_size);

		str = trace_group->GetPath();
		if (str != "")
			m_load_trace_text->ChangeValue(str);
		else
			m_load_trace_text->ChangeValue("Track map created but not saved");
		UpdateList();

		int ghost_num = trace_group->GetGhostNum();
		m_ghost_num_text->ChangeValue(wxString::Format("%d", ghost_num));
		m_ghost_num_sldr->ChangeValue(ghost_num);
		m_ghost_show_tail_chk->SetValue(trace_group->GetDrawTail());
		m_ghost_show_lead_chk->SetValue(trace_group->GetDrawLead());
	}
	else
	{
		m_load_trace_text->ChangeValue("No Track map");
	}

	//settings for tracking
	m_map_iter_spin->SetValue(glbin_settings.m_track_iter);
	m_map_size_spin->SetValue(glbin_settings.m_component_size);
	m_map_consistent_btn->SetValue(glbin_settings.m_consistent_color);
	m_map_merge_btn->SetValue(glbin_settings.m_try_merge);
	m_map_split_btn->SetValue(glbin_settings.m_try_split);
	m_map_similar_spin->SetValue(glbin_settings.m_similarity);
	m_map_contact_spin->SetValue(glbin_settings.m_contact_factor);
}

RenderCanvas* TraceDlg::GetRenderCanvas()
{
	return m_view;
}

void TraceDlg::UpdateList()
{
	if (!m_view)
		return;

	int shuffle = 0;
	if (m_view->m_cur_vol)
		shuffle = m_view->m_cur_vol->GetShuffle();
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
			m_trace_list_prev->Freeze();
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
			m_trace_list_prev->Thaw();

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

void TraceDlg::OnClearTrace(wxCommandEvent& event)
{
	if (!m_view)
		return;

	TraceGroup* trace_group = m_view->GetTraceGroup();
	if (trace_group)
	{
		trace_group->Clear();
		m_load_trace_text->ChangeValue("No Track map");
	}
}

void TraceDlg::OnLoadTrace(wxCommandEvent& event)
{
	if (!m_view)
		return;

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
	if (!m_view)
		return;

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
	if (!m_view)
		return;

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

void TraceDlg::OnGhostNumChange(wxScrollEvent& event)
{
	int ival = m_ghost_num_sldr->GetValue();
	wxString str = wxString::Format("%d", ival);
	if (str != m_ghost_num_text->GetValue())
		m_ghost_num_text->SetValue(str);
}

void TraceDlg::OnGhostNumText(wxCommandEvent& event)
{
	wxString str = m_ghost_num_text->GetValue();
	long ival;
	str.ToLong(&ival);
	m_ghost_num_sldr->ChangeValue(ival);

	if (m_view)
	{
		TraceGroup* trace_group = m_view->GetTraceGroup();
		if (trace_group)
		{
			trace_group->SetGhostNum(ival);
			m_view->RefreshGL(39);
		}
	}
}

void TraceDlg::OnGhostShowTail(wxCommandEvent& event)
{
	bool show = m_ghost_show_tail_chk->GetValue();

	if (m_view)
	{
		TraceGroup* trace_group = m_view->GetTraceGroup();
		if (trace_group)
		{
			trace_group->SetDrawTail(show);
			m_view->RefreshGL(39);
		}
	}
}

void TraceDlg::OnGhostShowLead(wxCommandEvent& event)
{
	bool show = m_ghost_show_lead_chk->GetValue();

	if (m_view)
	{
		TraceGroup* trace_group = m_view->GetTraceGroup();
		if (trace_group)
		{
			trace_group->SetDrawLead(show);
			m_view->RefreshGL(39);
		}
	}
}

//cell size filter
void TraceDlg::OnCellSizeChange(wxScrollEvent& event)
{
	int ival = m_cell_size_sldr->GetValue();
	wxString str = wxString::Format("%d", ival);
	if (str != m_cell_size_text->GetValue())
		m_cell_size_text->SetValue(str);
}

void TraceDlg::OnCellSizeText(wxCommandEvent& event)
{
	wxString str = m_cell_size_text->GetValue();
	long ival;
	str.ToLong(&ival);
	m_cell_size_sldr->ChangeValue(ival);

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
	flrd::CelpList list_in, list_out;

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

	flrd::pTrackMap track_map = trace_group->GetTrackMap();
	glbin_trackmap_proc.SetTrackMap(track_map);
	wxString str = m_comp_uncertain_low_text->GetValue();
	long ival;
	str.ToLong(&ival);
	glbin_trackmap_proc.SetUncertainLow(ival);
	glbin_trackmap_proc.GetCellsByUncertainty(list_in, list_out, m_cur_time);

	glbin_comp_selector.SelectList(list_out);

	//update view
	CellUpdate();

	//frame
	//if (m_frame && m_frame->GetBrushToolDlg())
	//	m_frame->GetBrushToolDlg()->UpdateUndoRedo();
}

void TraceDlg::OnCompUncertainBtn(wxCommandEvent& event)
{
	UncertainFilter();
}

void TraceDlg::OnCompUncertainLowChange(wxScrollEvent& event)
{
	int ival = m_comp_uncertain_low_sldr->GetValue();
	wxString str = wxString::Format("%d", ival);
	if (str != m_comp_uncertain_low_text->GetValue())
		m_comp_uncertain_low_text->SetValue(str);
}

void TraceDlg::OnCompUncertainLowText(wxCommandEvent& event)
{
	wxString str = m_comp_uncertain_low_text->GetValue();
	long ival;
	str.ToLong(&ival);
	m_comp_uncertain_low_sldr->ChangeValue(ival);

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
void TraceDlg::OnGenMapBtn(wxCommandEvent& event)
{
	GenMap();
}

void TraceDlg::OnRefineTBtn(wxCommandEvent& event)
{
	RefineMap(m_cur_time);
}

void TraceDlg::OnRefineAllBtn(wxCommandEvent& event)
{
	RefineMap();
}

//settings
void TraceDlg::OnMapIterSpin(wxSpinEvent& event)
{
	glbin_settings.m_track_iter = m_map_iter_spin->GetValue();
}

void TraceDlg::OnMapIterText(wxCommandEvent& event)
{
	glbin_settings.m_track_iter = m_map_iter_spin->GetValue();
}

void TraceDlg::OnMapSizeSpin(wxSpinEvent& event)
{
	glbin_settings.m_component_size = m_map_size_spin->GetValue();
}

void TraceDlg::OnMapSizeText(wxCommandEvent& event)
{
	glbin_settings.m_component_size = m_map_size_spin->GetValue();
}

void TraceDlg::OnMapConsistentBtn(wxCommandEvent& event)
{
	glbin_settings.m_consistent_color = m_map_consistent_btn->GetValue();
}

void TraceDlg::OnMapMergeBtn(wxCommandEvent& event)
{
	glbin_settings.m_try_merge = m_map_merge_btn->GetValue();
}

void TraceDlg::OnMapSplitBtn(wxCommandEvent& event)
{
	glbin_settings.m_try_split = m_map_split_btn->GetValue();
}

void TraceDlg::OnMapSimilarSpin(wxSpinDoubleEvent& event)
{
	glbin_settings.m_similarity = m_map_similar_spin->GetValue();
}

void TraceDlg::OnMapSimilarText(wxCommandEvent& event)
{
	glbin_settings.m_similarity = m_map_similar_spin->GetValue();
}

void TraceDlg::OnMapContactSpin(wxSpinDoubleEvent& event)
{
	glbin_settings.m_contact_factor = m_map_contact_spin->GetValue();
}

void TraceDlg::OnMapContactText(wxCommandEvent& event)
{
	glbin_settings.m_contact_factor = m_map_contact_spin->GetValue();
}

//analysis
void TraceDlg::OnConvertToRulers(wxCommandEvent& event)
{
	if (!m_view)
		return;

	TraceGroup* trace_group = m_view->GetTraceGroup();
	if (!trace_group)
		return;
	VolumeData* vd = m_view->m_cur_vol;
	if (!vd)
		return;
	double spcx, spcy, spcz;
	vd->GetSpacings(spcx, spcy, spcz);

	//get rulers
	flrd::RulerList rulers;
	trace_group->GetMappedRulers(rulers);
	for (auto iter = rulers.begin();
	iter != rulers.end(); ++iter)
	{
		(*iter)->Scale(spcx, spcy, spcz);
		m_view->GetRulerList()->push_back(*iter);
	}
	m_view->RefreshGL(39);
	if (m_frame && m_frame->GetMeasureDlg())
		m_frame->GetMeasureDlg()->GetSettings(m_view);
	glbin_vertex_array_manager.set_dirty(flvr::VA_Rulers);
}

void TraceDlg::OnConvertConsistent(wxCommandEvent& event)
{
	if (!m_view)
		return;

	VolumeData* vd = m_view->m_cur_vol;
	if (!vd)
		return;
	TraceGroup *trace_group = m_view->GetTraceGroup();
	if (!trace_group)
		return;

	m_stat_text->ChangeValue("Generating consistent IDs in");
	wxGetApp().Yield();

	flrd::pTrackMap track_map = trace_group->GetTrackMap();
	glbin_trackmap_proc.SetTrackMap(track_map);
	int resx, resy, resz;
	vd->GetResolution(resx, resy, resz);
	double spcx, spcy, spcz;
	vd->GetSpacings(spcx, spcy, spcz);
	glbin_trackmap_proc.SetBits(vd->GetBits());
	glbin_trackmap_proc.SetScale(vd->GetScalarScale());
	glbin_trackmap_proc.SetSizes(resx, resy, resz);
	glbin_trackmap_proc.SetSpacings(spcx, spcy, spcz);
	glbin_reg_cache_queue_func(this, TraceDlg::ReadVolCache, TraceDlg::DelVolCache);
	glbin_cache_queue.set_max_size(2);

	(*m_stat_text) << wxString::Format("Frame %d\n", 0);
	wxGetApp().Yield();
	glbin_trackmap_proc.MakeConsistent(0);

	//remaining frames
	for (size_t fi = 1; fi < track_map->GetFrameNum(); ++fi)
	{
		(*m_stat_text) << wxString::Format("Frame %d\n", int(fi));
		wxGetApp().Yield();
		glbin_trackmap_proc.MakeConsistent(fi - 1, fi);
	}

	CellUpdate();
}

void TraceDlg::OnAnalyzeComp(wxCommandEvent& event)
{
	if (!m_view)
		return;
	VolumeData* vd = m_view->m_cur_vol;
	glbin_comp_analyzer.SetVolume(vd);
	glbin_comp_analyzer.Analyze(true);
	string str;
	glbin_comp_analyzer.OutputCompListStr(str, 1);
	m_stat_text->ChangeValue(str);
}

void TraceDlg::OnAnalyzeLink(wxCommandEvent& event)
{
	if (!m_view)
		return;

	TraceGroup* trace_group = m_view->GetTraceGroup();
	if (!trace_group)
		return;
	size_t frames = trace_group->GetTrackMap()->GetFrameNum();
	if (frames == 0)
		m_stat_text->ChangeValue("ERROR! Generate a track map first.\n");
	else
		m_stat_text->ChangeValue(
			wxString::Format("Time point number: %d\n", int(frames)));

	(*m_stat_text) << "Time\tIn Orphan\tOut Orphan\tIn Multi\tOut Multi\n";
	flrd::VertexList in_orphan_list;
	flrd::VertexList out_orphan_list;
	flrd::VertexList in_multi_list;
	flrd::VertexList out_multi_list;
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

void TraceDlg::OnAnalyzeUncertainHist(wxCommandEvent& event)
{
	if (!m_view)
		return;
	//trace group
	TraceGroup *trace_group = m_view->GetTraceGroup();
	if (!trace_group)
		return;
	if (!trace_group->GetTrackMap()->GetFrameNum())
		return;
	flrd::CelpList list_in;
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

	m_stat_text->ChangeValue("");

	flrd::pTrackMap track_map = trace_group->GetTrackMap();
	glbin_trackmap_proc.SetTrackMap(track_map);
	if (list_in.empty())
	{
		flrd::UncertainHist hist1, hist2;
		glbin_trackmap_proc.GetUncertainHist(hist1, hist2, m_cur_time);
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
		glbin_trackmap_proc.GetCellUncertainty(list_in, m_cur_time);
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

void TraceDlg::OnAnalyzePath(wxCommandEvent& event)
{
	if (!m_view)
		return;
	//trace group
	TraceGroup *trace_group = m_view->GetTraceGroup();
	if (!trace_group)
		return;
	if (!trace_group->GetTrackMap()->GetFrameNum())
		return;
	flrd::CelpList list_in;
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

	m_stat_text->ChangeValue("");

	flrd::pTrackMap track_map = trace_group->GetTrackMap();
	glbin_trackmap_proc.SetTrackMap(track_map);
	if (list_in.empty())
		return;

	m_stat_text->ChangeValue("");
	std::ostream os(m_stat_text);

	if (m_cur_time > 0)
	{
		(*m_stat_text) << "Paths of T" << m_cur_time << " to T" << m_cur_time - 1 << ":\n";
		flrd::PathList paths_prv;
		glbin_trackmap_proc.GetPaths(list_in, paths_prv, m_cur_time, m_cur_time - 1);
		for (size_t i = 0; i < paths_prv.size(); ++i)
			os << paths_prv[i];
	}
	if (m_cur_time < track_map->GetFrameNum() - 1)
	{
		(*m_stat_text) << "Paths of T" << m_cur_time << " to T" << m_cur_time + 1 << ":\n";
		flrd::PathList paths_nxt;
		glbin_trackmap_proc.GetPaths(list_in, paths_nxt, m_cur_time, m_cur_time + 1);
		for (size_t i = 0; i < paths_nxt.size(); ++i)
			os << paths_nxt[i];
	}
}

void TraceDlg::OnSaveResult(wxCommandEvent& event)
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
	vector<unsigned long long> ids;
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
	if (ids.size() == 1)
	{
		glbin_comp_selector.SetId(ids[0], false);
		glbin_comp_selector.Delete();
	}
	else
		glbin_comp_selector.Delete(ids);

	//update view
	CellUpdate();

	//frame
	//if (m_frame && m_frame->GetBrushToolDlg())
	//	m_frame->GetBrushToolDlg()->UpdateUndoRedo();
}

void TraceDlg::CompClear()
{
	if (!m_view)
		return;

	//get current vd
	glbin_comp_selector.Clear();

	//update view
	CellUpdate();
	m_trace_list_prev->DeleteAllItems();

	//frame
	//if (m_frame && m_frame->GetBrushToolDlg())
	//	m_frame->GetBrushToolDlg()->UpdateUndoRedo();
}

void TraceDlg::OnCompIDText(wxCommandEvent& event)
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

void TraceDlg::OnCompIDXBtn(wxCommandEvent& event)
{
	m_comp_id_text->Clear();
	m_comp_id_text2->Clear();
}

void TraceDlg::OnCompClear(wxCommandEvent& event)
{
	CompClear();
}

void TraceDlg::OnShuffle(wxCommandEvent& event)
{
	if (!m_view)
		return;

	//get current vd
	VolumeData* vd = m_view->m_cur_vol;
	if (!vd)
		return;

	vd->IncShuffle();
	m_view->RefreshGL(39);
}

void TraceDlg::OnCompFull(wxCommandEvent& event)
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

void TraceDlg::OnCompAppend(wxCommandEvent& event)
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
		glbin_comp_selector.SetId(id, false);
		glbin_comp_selector.SetUseMin(true);
		glbin_comp_selector.SetMinNum(slimit);
		glbin_comp_selector.Select(get_all);
	}

	//update view
	CellUpdate();

	//frame
	//if (m_frame && m_frame->GetBrushToolDlg())
	//	m_frame->GetBrushToolDlg()->UpdateUndoRedo();
}

void TraceDlg::OnCompExclusive(wxCommandEvent& event)
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
		glbin_comp_selector.SetId(id, false);
		glbin_comp_selector.SetUseMin(true);
		glbin_comp_selector.SetMinNum(slimit);
		glbin_comp_selector.Exclusive();
	}

	//update view
	CellUpdate();

	//frame
	//if (m_frame && m_frame->GetBrushToolDlg())
	//	m_frame->GetBrushToolDlg()->UpdateUndoRedo();
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

		m_view->GetTraces(false);
		m_view->RefreshGL(39);
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
	glbin_comp_selector.SetUseMin(true);
	glbin_comp_selector.SetMinNum(slimit);
	glbin_comp_selector.CompFull();
	//update view
	CellUpdate();

	//frame
	//if (m_frame && m_frame->GetBrushToolDlg())
	//	m_frame->GetBrushToolDlg()->UpdateUndoRedo();
}

void TraceDlg::AddLabel(long item, TraceListCtrl* trace_list_ctrl, flrd::CelpList &list)
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

	flrd::Celp cell(new flrd::Cell(id));
	cell->SetSizeUi(size);
	cell->SetSizeD(size);
	fluo::Point p(x, y, z);
	cell->SetCenter(p);
	list.insert(std::pair<unsigned int, flrd::Celp>
		(id, cell));
}

void TraceDlg::CellNewID(bool append)
{
	glbin_comp_editor.SetId(m_cell_new_id,
		m_cell_new_id_empty);
	glbin_comp_editor.NewId(append, true);
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

	VolumeData* vd = m_view->m_cur_vol;
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

void TraceDlg::CellReplaceID()
{
	//current T
	flrd::CelpList list_cur;
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

	glbin_comp_editor.SetId(m_cell_new_id,
		m_cell_new_id_empty);
	glbin_comp_editor.Replace(list_cur);

	CellUpdate();
}

void TraceDlg::CellCombineID()
{
	//current T
	flrd::CelpList list_cur;
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

	glbin_comp_editor.Combine(list_cur);

	//update view
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
	flrd::CelpList list_cur;
	//previous T
	flrd::CelpList list_prv;
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
	m_view->RefreshGL(39);
}

void TraceDlg::OnCellLink(wxCommandEvent& event)
{
	CellLink(false);
}

void TraceDlg::OnCellLinkAll(wxCommandEvent& event)
{
	if (!m_frame || !m_frame->GetComponentDlg())
		return;
	if (!m_view)
		return;
	TraceGroup *trace_group = m_view->GetTraceGroup();
	if (!trace_group)
		return;

	flrd::pTrackMap track_map = trace_group->GetTrackMap();
	glbin_trackmap_proc.SetTrackMap(track_map);
	//register file reading and deleteing functions
	glbin_reg_cache_queue_func(this, TraceDlg::ReadVolCache, TraceDlg::DelVolCache);
	glbin_cache_queue.set_max_size(3);
	flrd::CelpList in = glbin_clusterizer.GetInCells();
	flrd::CelpList out = glbin_clusterizer.GetOutCells();
	glbin_trackmap_proc.RelinkCells(in, out, m_cur_time);

	CellUpdate();
}

void TraceDlg::OnCellExclusiveLink(wxCommandEvent& event)
{
	CellLink(true);
}

void TraceDlg::OnCellIsolate(wxCommandEvent& event)
{
	if (!m_view)
		return;

	TraceGroup* trace_group = m_view->GetTraceGroup();
	if (!trace_group)
		return;

	//get selections
	long item;
	//current T
	flrd::CelpList list_cur;

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
	m_view->RefreshGL(39);
}

void TraceDlg::OnCellUnlink(wxCommandEvent& event)
{
	if (!m_view)
		return;

	TraceGroup* trace_group = m_view->GetTraceGroup();
	if (!trace_group)
		return;

	//get selections
	long item;
	//current T
	flrd::CelpList list_cur;
	//previous T
	flrd::CelpList list_prv;
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
	m_view->RefreshGL(39);
}

//ID edit controls
void TraceDlg::OnCellNewIDText(wxCommandEvent& event)
{
	int shuffle = 0;
	if (m_view && m_view->m_cur_vol)
		shuffle = m_view->m_cur_vol->GetShuffle();
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
		m_cell_new_id = id;
		m_cell_new_id_empty = false;
	}
	else
	{
		m_cell_new_id_text->SetBackgroundColour(color);
		m_cell_new_id_empty = true;
	}
	m_cell_new_id_text->Refresh();
}

void TraceDlg::OnCellNewIDX(wxCommandEvent& event)
{
	m_cell_new_id_text->Clear();
}

void TraceDlg::OnCellNewID(wxCommandEvent& event)
{
	CellNewID(false);
}

void TraceDlg::OnCellAppendID(wxCommandEvent& event)
{
	CellNewID(true);
}

void TraceDlg::OnCellReplaceID(wxCommandEvent& event)
{
	CellReplaceID();
}

void TraceDlg::OnCellCombineID(wxCommandEvent& event)
{
	CellCombineID();
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
	flrd::CelpList list_cur;
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
	flrd::CelpList list_cur;
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
	VolumeData* vd = m_view->m_cur_vol;
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

	flrd::pTrackMap track_map = trace_group->GetTrackMap();
	glbin_trackmap_proc.SetTrackMap(track_map);
	glbin_trackmap_proc.SetBits(vd->GetBits());
	glbin_trackmap_proc.SetScale(vd->GetScalarScale());
	glbin_trackmap_proc.SetSizes(resx, resy, resz);
	//register file reading and deleteing functions
	glbin_reg_cache_queue_func(this, TraceDlg::ReadVolCache, TraceDlg::DelVolCache);
	glbin_cache_queue.set_max_size(3);
	glbin_trackmap_proc.SegmentCells(list_cur, m_cur_time, m_clnum);

	//invalidate label mask in gpu
	vd->GetVR()->clear_tex_current();
	//m_view->RefreshGL();
	//update view
	//CellUpdate();
	RefineMap(m_cur_time);
}

void TraceDlg::LinkAddedCells(flrd::CelpList &list)
{
	if (!m_view)
		return;

	VolumeData* vd = m_view->m_cur_vol;
	if (!vd)
		return;
	int resx, resy, resz;
	vd->GetResolution(resx, resy, resz);
	TraceGroup *trace_group = m_view->GetTraceGroup();
	if (!trace_group)
		return;

	flrd::pTrackMap track_map = trace_group->GetTrackMap();
	glbin_trackmap_proc.SetTrackMap(track_map);
	glbin_trackmap_proc.SetBits(vd->GetBits());
	glbin_trackmap_proc.SetScale(vd->GetScalarScale());
	glbin_trackmap_proc.SetSizes(resx, resy, resz);
	//register file reading and deleteing functions
	glbin_reg_cache_queue_func(this, TraceDlg::ReadVolCache, TraceDlg::DelVolCache);
	glbin_cache_queue.set_max_size(3);
	glbin_trackmap_proc.LinkAddedCells(list, m_cur_time, m_cur_time - 1);
	glbin_trackmap_proc.LinkAddedCells(list, m_cur_time, m_cur_time + 1);
	RefineMap(m_cur_time, false);
}

void TraceDlg::SaveOutputResult(wxString &filename)
{
	std::ofstream os;
	OutputStreamOpen(os, filename.ToStdString());

	wxString str;
	str = m_stat_text->GetValue();

	os << str;

	os.close();
}

//read/delete volume cache
void TraceDlg::ReadVolCache(flrd::VolCache& vol_cache)
{
	//get volume, readers
	if (!m_view)
		return;
	VolumeData* vd = m_view->m_cur_vol;
	if (!vd)
		return;
	BaseReader* reader = vd->GetReader();
	if (!reader)
		return;
	LBLReader lbl_reader;

	int chan = vd->GetCurChannel();
	int frame = vol_cache.frame;

	if (frame == m_view->m_tseq_cur_num)
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

void TraceDlg::DelVolCache(flrd::VolCache& vol_cache)
{
	if (!m_view)
		return;
	VolumeData* vd = m_view->m_cur_vol;
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
	if (frame != m_view->m_tseq_cur_num)
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

void TraceDlg::OnCellPrev(wxCommandEvent& event)
{
	if (m_frame && m_frame->GetMovieView())
		m_frame->GetMovieView()->DecFrame();
}

void TraceDlg::OnCellNext(wxCommandEvent& event)
{
	if (m_frame && m_frame->GetMovieView())
		m_frame->GetMovieView()->IncFrame();
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

	VolumeData* vd = m_view->m_cur_vol;
	if (!vd)
		return;
	BaseReader* reader = vd->GetReader();
	if (!reader)
		return;

	//start progress
	m_stat_text->ChangeValue("Generating track map.\n");
	wxGetApp().Yield();
	int frames = reader->GetTimeNum();

	//get and set parameters
	flrd::pTrackMap track_map = trace_group->GetTrackMap();
	glbin_trackmap_proc.SetTrackMap(track_map);
	int resx, resy, resz;
	vd->GetResolution(resx, resy, resz);
	double spcx, spcy, spcz;
	vd->GetSpacings(spcx, spcy, spcz);
	glbin_trackmap_proc.SetBits(vd->GetBits());
	glbin_trackmap_proc.SetScale(vd->GetScalarScale());
	glbin_trackmap_proc.SetSizes(resx, resy, resz);
	glbin_trackmap_proc.SetSpacings(spcx, spcy, spcz);
	glbin_trackmap_proc.SetSizeThresh(glbin_settings.m_component_size);
	glbin_trackmap_proc.SetContactThresh(glbin_settings.m_contact_factor);
	glbin_trackmap_proc.SetSimilarThresh(glbin_settings.m_similarity);
	//register file reading and deleteing functions
	glbin_reg_cache_queue_func(this, TraceDlg::ReadVolCache, TraceDlg::DelVolCache);
	glbin_cache_queue.set_max_size(4);
	//merge/split
	glbin_trackmap_proc.SetMerge(glbin_settings.m_try_merge);
	glbin_trackmap_proc.SetSplit(glbin_settings.m_try_split);

	//start timing
	std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
	//initialization
	for (int i = 0; i < frames; ++i)
	{
		glbin_trackmap_proc.InitializeFrame(i);
		(*m_stat_text) << wxString::Format("Time point %d initialized.\n", i);
		wxGetApp().Yield();

		if (i < 1)
			continue;

		//link maps 1 and 2
		glbin_trackmap_proc.LinkFrames(i - 1, i);
		(*m_stat_text) << wxString::Format("Time point %d linked.\n", i);
		wxGetApp().Yield();

		//check contacts and merge cells
		glbin_trackmap_proc.ResolveGraph(i - 1, i);
		glbin_trackmap_proc.ResolveGraph(i, i - 1);
		(*m_stat_text) << wxString::Format("Time point %d merged.\n", i - 1);
		wxGetApp().Yield();

		if (i < 2)
			continue;

		//further process
		glbin_trackmap_proc.ProcessFrames(i - 2, i - 1);
		glbin_trackmap_proc.ProcessFrames(i - 1, i - 2);
		(*m_stat_text) << wxString::Format("Time point %d processed.\n", i - 1);
		wxGetApp().Yield();
	}
	//last frame
	glbin_trackmap_proc.ProcessFrames(frames - 2, frames - 1);
	glbin_trackmap_proc.ProcessFrames(frames - 1, frames - 2);
	(*m_stat_text) << wxString::Format("Time point %d processed.\n", frames - 1);
	wxGetApp().Yield();

	//iterations
	for (size_t iteri = 0; iteri < glbin_settings.m_track_iter; ++iteri)
	{
		for (int i = 2; i <= frames; ++i)
		{
			//further process
			glbin_trackmap_proc.ProcessFrames(i - 2, i - 1);
			glbin_trackmap_proc.ProcessFrames(i - 1, i - 2);
			(*m_stat_text) << wxString::Format("Time point %d processed.\n", i - 1);
			wxGetApp().Yield();
		}
	}

	//consistent colors
	if (glbin_settings.m_consistent_color)
	{
		(*m_stat_text) << wxString::Format("Set colors for frame 0\n");
		wxGetApp().Yield();
		glbin_trackmap_proc.MakeConsistent(0);
		//remaining frames
		for (size_t fi = 1; fi < track_map->GetFrameNum(); ++fi)
		{
			(*m_stat_text) << wxString::Format("Set colors for frame %d\n", int(fi));
			wxGetApp().Yield();
			glbin_trackmap_proc.MakeConsistent(fi - 1, fi);
		}
	}

	std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> time_span = duration_cast<std::chrono::duration<double>>(t2 - t1);
	(*m_stat_text) << wxString::Format("Wall clock time: %.4fs\n", time_span.count());

	GetSettings(m_view);

	//CellUpdate();
}

void TraceDlg::RefineMap(int t, bool erase_v)
{
	if (!m_view)
		return;

	//get trace group
	VolumeData* vd = m_view->m_cur_vol;
	if (!vd)
		return;
	TraceGroup *trace_group = m_view->GetTraceGroup();
	if (!trace_group)
		return;
	if (t < 0)
		m_stat_text->ChangeValue("Refining track map for all time points.\n");
	else
		m_stat_text->ChangeValue(wxString::Format(
			"Refining track map at time point %d.\n", t));
	wxGetApp().Yield();

	//start progress
	bool clear_counters = false;
	flrd::pTrackMap track_map = trace_group->GetTrackMap();
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
	glbin_trackmap_proc.SetTrackMap(track_map);
	int resx, resy, resz;
	vd->GetResolution(resx, resy, resz);
	double spcx, spcy, spcz;
	vd->GetSpacings(spcx, spcy, spcz);
	glbin_trackmap_proc.SetBits(vd->GetBits());
	glbin_trackmap_proc.SetScale(vd->GetScalarScale());
	glbin_trackmap_proc.SetSizes(resx, resy, resz);
	glbin_trackmap_proc.SetSpacings(spcx, spcy, spcz);
	glbin_trackmap_proc.SetSizeThresh(glbin_settings.m_component_size);
	glbin_trackmap_proc.SetContactThresh(glbin_settings.m_contact_factor);
	glbin_trackmap_proc.SetSimilarThresh(glbin_settings.m_similarity);
	//register file reading and deleteing functions
	glbin_reg_cache_queue_func(this, TraceDlg::ReadVolCache, TraceDlg::DelVolCache);
	glbin_cache_queue.set_max_size(4);
	//merge/split
	glbin_trackmap_proc.SetMerge(glbin_settings.m_try_merge);
	glbin_trackmap_proc.SetSplit(glbin_settings.m_try_split);

	std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

	//not sure if counters need to be cleared for all refinement
	//if (clear_counters)
	//	glbin_trackmap_proc.ClearCounters();
	//iterations
	for (size_t iteri = 0; iteri < glbin_settings.m_track_iter; ++iteri)
	{
		for (int i = start_frame - 1; i <= end_frame; ++i)
		{
			//further process
			glbin_trackmap_proc.ProcessFrames(i, i + 1, erase_v);
			glbin_trackmap_proc.ProcessFrames(i + 1, i, erase_v);
			(*m_stat_text) << wxString::Format("Time point %d processed.\n", i + 1);
			wxGetApp().Yield();
		}
	}

	//consistent colors
	if (glbin_settings.m_consistent_color)
	{
		if (t < 0)
		{
			(*m_stat_text) << wxString::Format("Set colors for frame 0\n");
			wxGetApp().Yield();
			glbin_trackmap_proc.MakeConsistent(0);
			//remaining frames
			for (size_t fi = 1; fi < track_map->GetFrameNum(); ++fi)
			{
				(*m_stat_text) << wxString::Format("Set colors for frame %d\n", int(fi));
				wxGetApp().Yield();
				glbin_trackmap_proc.MakeConsistent(fi - 1, fi);
			}
		}
		else
		{
			glbin_trackmap_proc.MakeConsistent(t - 1, t);
		}
	}

	std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> time_span = duration_cast<std::chrono::duration<double>>(t2 - t1);
	(*m_stat_text) << wxString::Format("Wall clock time: %.4fs\n", time_span.count());

	CellUpdate();
}

int TraceDlg::GetTrackFileExist(bool save)
{
	if (!m_view) return 0;
	TraceGroup* trace_group = m_view->GetTraceGroup();
	if (!trace_group)
		return 0;
	if (!trace_group->GetTrackMap())
		return 0;
	if (!trace_group->GetTrackMap()->GetFrameNum())
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
		m_load_trace_text->ChangeValue(file);
		m_track_file = file;
	}
}

void TraceDlg::SaveTrackFile(wxString &file)
{
	if (!m_view) return;
	int rval = m_view->SaveTraceGroup(file);
	if (rval)
	{
		m_load_trace_text->ChangeValue(file);
		m_track_file = file;
	}
}