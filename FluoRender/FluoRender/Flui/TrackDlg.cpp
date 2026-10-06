/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2026 Scientific Computing and Imaging Institute,
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
#include <TrackDlgAgent.h>
#include <wxSingleSlider.h>
#include <wxNumTextCtrl.h>
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

TrackListCtrl::TrackListCtrl(
	wxWindow* parent,
	const wxPoint& pos,
	const wxSize& size,
	long style) :
	wxListCtrl(parent, wxID_ANY, pos, size, style),
	m_type(0)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);
	//SetDoubleBuffered(true);

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

TrackListCtrl::~TrackListCtrl()
{
}

void TrackListCtrl::Append(const wxString &gtype, unsigned int id, wxColor color,
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

void TrackListCtrl::DeleteSelection()
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

void TrackListCtrl::CopySelection()
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

wxString TrackListCtrl::GetText(long item, int col)
{
	wxListItem info;
	info.SetId(item);
	info.SetColumn(col);
	info.SetMask(wxLIST_MASK_TEXT);
	GetItem(info);
	return info.GetText();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
TrackDlg::TrackDlg(wxWindow* parent)
	: TabbedPanel(parent,
		wxDefaultPosition,
		parent->FromDIP(wxSize(500, 620)),
		0, "TrackDlg")
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);
	Freeze();
	SetDoubleBuffered(true);

	//notebook
	m_notebook = new wxAuiNotebook(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize,
		wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE |
		wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_TAB_EXTERNAL_MOVE | wxNO_BORDER);
	m_notebook->AddPage(CreateMapPage(m_notebook), L"Track Map", true);
	m_notebook->AddPage(CreateSelectPage(m_notebook), L"Selection");
	m_notebook->AddPage(CreateModifyPage(m_notebook), L"Modify");
	m_notebook->AddPage(CreateLinkPage(m_notebook), L"Linkage");
	m_notebook->AddPage(CreateAnalysisPage(m_notebook), "Analysis");
	m_notebook->AddPage(CreateListPage(m_notebook), "Tracks");
	m_notebook->AddPage(CreateOutputPage(m_notebook), "Information");

	Bind(wxEVT_MENU, &TrackDlg::OnMenuItem, this);

	//vertical sizer
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
	sizer_v->Add(m_notebook, 1, wxEXPAND | wxALL);

	SetSizer(sizer_v);
	Layout();
	SetAutoLayout(true);
	SetScrollRate(10, 10);
	Thaw();
}

TrackDlg::~TrackDlg()
{
}

wxWindow* TrackDlg::CreateMapPage(wxWindow *parent)
{
	wxScrolledWindow *page = new wxScrolledWindow(parent);

	wxStaticText *st = 0;

	//load trace
	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Track map:",
		wxDefaultPosition, FromDIP(wxSize(70, 20)));
	m_load_trace_text = new wxTextCtrl(page, wxID_ANY, "",
		wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_clear_trace_btn = new wxButton(page, wxID_ANY, "X",
		wxDefaultPosition, FromDIP(wxSize(23, 23)));
	m_load_trace_btn = new wxButton(page, wxID_ANY, "Load",
		wxDefaultPosition, FromDIP(wxSize(65, 23)));
	m_save_trace_btn = new wxButton(page, wxID_ANY, "Save",
		wxDefaultPosition, FromDIP(wxSize(65, 23)));
	m_saveas_trace_btn = new wxButton(page, wxID_ANY, "Save As",
		wxDefaultPosition, FromDIP(wxSize(65, 23)));
	m_clear_trace_btn->Bind(wxEVT_BUTTON, &TrackDlg::OnClearTrace, this);
	m_load_trace_btn->Bind(wxEVT_BUTTON, &TrackDlg::OnLoadTrace, this);
	m_save_trace_btn->Bind(wxEVT_BUTTON, &TrackDlg::OnSaveTrace, this);
	m_saveas_trace_btn->Bind(wxEVT_BUTTON, &TrackDlg::OnSaveasTrace, this);
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
	m_map_iter_spin = new wxSpinCtrl(page, wxID_ANY, "3",
		wxDefaultPosition, FromDIP(wxSize(50, 23)));
	m_map_iter_spin->Bind(wxEVT_SPINCTRL, &TrackDlg::OnMapIterSpin, this);
	m_map_iter_spin->Bind(wxEVT_TEXT, &TrackDlg::OnMapIterText, this);
	sizer_2->Add(5, 5);
	sizer_2->Add(st, 0, wxALIGN_CENTER);
	sizer_2->Add(5, 5);
	sizer_2->Add(m_map_iter_spin, 0, wxALIGN_CENTER);
	st = new wxStaticText(page, 0, "Size Threshold:",
		wxDefaultPosition, wxDefaultSize);
	m_map_size_spin = new wxSpinCtrl(page, wxID_ANY, "100",
		wxDefaultPosition, FromDIP(wxSize(50, 23)));
	m_map_size_spin->SetRange(1, std::numeric_limits<int>::max());
	m_map_size_spin->Bind(wxEVT_SPINCTRL, &TrackDlg::OnMapSizeSpin, this);
	m_map_size_spin->Bind(wxEVT_TEXT, &TrackDlg::OnMapSizeText, this);
	sizer_2->Add(5, 5);
	sizer_2->Add(st, 0, wxALIGN_CENTER);
	sizer_2->Add(5, 5);
	sizer_2->Add(m_map_size_spin, 0, wxALIGN_CENTER);
	m_gen_map_btn = new wxButton(page, wxID_ANY, "Generate",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_refine_t_btn = new wxButton(page, wxID_ANY, "Refine T",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_refine_all_btn = new wxButton(page, wxID_ANY, "Refine All",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_gen_map_btn->Bind(wxEVT_BUTTON, &TrackDlg::OnGenMapBtn, this);
	m_refine_t_btn->Bind(wxEVT_BUTTON, &TrackDlg::OnRefineTBtn, this);
	m_refine_all_btn->Bind(wxEVT_BUTTON, &TrackDlg::OnRefineAllBtn, this);
	sizer_2->Add(5, 5);
	sizer_2->Add(m_gen_map_btn, 0, wxALIGN_CENTER);
	sizer_2->Add(m_refine_t_btn, 0, wxALIGN_CENTER);
	sizer_2->Add(m_refine_all_btn, 0, wxALIGN_CENTER);

	//
	wxBoxSizer* sizer_3 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Similarity:",
		wxDefaultPosition, wxDefaultSize);
	m_map_similar_spin = new wxSpinCtrlDouble(
		page, wxID_ANY, "0.2",
		wxDefaultPosition, FromDIP(wxSize(50, 23)),
		wxSP_ARROW_KEYS| wxSP_WRAP,
		0, 1, 0.2, 0.01);
	m_map_similar_spin->Bind(wxEVT_SPINCTRLDOUBLE, &TrackDlg::OnMapSimilarSpin, this);
	m_map_similar_spin->Bind(wxEVT_TEXT, &TrackDlg::OnMapSimilarText, this);
	sizer_3->Add(5, 5);
	sizer_3->Add(st, 0, wxALIGN_CENTER);
	sizer_3->Add(5, 5);
	sizer_3->Add(m_map_similar_spin, 0, wxALIGN_CENTER);
	st = new wxStaticText(page, 0, "Contact Factor:",
		wxDefaultPosition, wxDefaultSize);
	m_map_contact_spin = new wxSpinCtrlDouble(
		page, wxID_ANY, "0.6",
		wxDefaultPosition, FromDIP(wxSize(50, 23)),
		wxSP_ARROW_KEYS | wxSP_WRAP,
		0, 1, 0.6, 0.01);
	m_map_contact_spin->Bind(wxEVT_SPINCTRLDOUBLE, &TrackDlg::OnMapContactSpin, this);
	m_map_contact_spin->Bind(wxEVT_TEXT, &TrackDlg::OnMapContactText, this);
	sizer_3->Add(5, 5);
	sizer_3->Add(st, 0, wxALIGN_CENTER);
	sizer_3->Add(5, 5);
	sizer_3->Add(m_map_contact_spin, 0, wxALIGN_CENTER);
	m_map_consistent_btn = new wxToggleButton(page, wxID_ANY,
		"Consistent Colors", wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_map_merge_btn = new wxToggleButton(
		page, wxID_ANY, "Try Merging",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_map_split_btn = new wxToggleButton(
		page, wxID_ANY, "Try Splitting",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_map_merge_btn->Bind(wxEVT_BUTTON, &TrackDlg::OnMapMergeBtn, this);
	m_map_split_btn->Bind(wxEVT_BUTTON, &TrackDlg::OnMapSplitBtn, this);
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
	page->SetAutoLayout(true);
	page->SetScrollRate(10, 10);
	return page;
}

wxWindow* TrackDlg::CreateSelectPage(wxWindow *parent)
{
	wxScrolledWindow *page = new wxScrolledWindow(parent);

	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;
	wxStaticText *st = 0;

	//selection tools
	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Selection tools:",
		wxDefaultPosition, FromDIP(wxSize(100, 20)));
	m_comp_id_text = new wxNumTextCtrl(page, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(77, 23)), wxTE_PROCESS_ENTER | wxTE_RIGHT);
	m_comp_id_x_btn = new wxButton(page, wxID_ANY, "X",
		wxDefaultPosition, FromDIP(wxSize(23, 23)));
	m_comp_full_btn = new wxButton(page, wxID_ANY, "FullCompt",
		wxDefaultPosition, FromDIP(wxSize(64, 23)));
	m_comp_exclusive_btn = new wxButton(page, wxID_ANY, "Replace",
		wxDefaultPosition, FromDIP(wxSize(64, 23)));
	m_comp_append_btn = new wxButton(page, wxID_ANY, "Append",
		wxDefaultPosition, FromDIP(wxSize(64, 23)));
	m_comp_clear_btn = new wxButton(page, wxID_ANY, "Clear",
		wxDefaultPosition, FromDIP(wxSize(64, 23)));
	m_shuffle_btn = new wxButton(page, wxID_ANY, "Shuffle",
		wxDefaultPosition, FromDIP(wxSize(64, 23)));
	m_comp_id_text->Bind(wxEVT_TEXT, &TrackDlg::OnCompIDText, this);
	m_comp_id_text->Bind(wxEVT_TEXT_ENTER, &TrackDlg::OnCompFull, this);
	m_comp_id_x_btn->Bind(wxEVT_BUTTON, &TrackDlg::OnCompIDXBtn, this);
	m_comp_full_btn->Bind(wxEVT_BUTTON, &TrackDlg::OnCompFull, this);
	m_comp_exclusive_btn->Bind(wxEVT_BUTTON, &TrackDlg::OnCompExclusive, this);
	m_comp_append_btn->Bind(wxEVT_BUTTON, &TrackDlg::OnCompAppend, this);
	m_comp_clear_btn->Bind(wxEVT_BUTTON, &TrackDlg::OnCompClear, this);
	m_shuffle_btn->Bind(wxEVT_BUTTON, &TrackDlg::OnShuffle, this);
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
	m_cell_size_sldr = new wxSingleSlider(page, wxID_ANY, 20, 0, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_cell_size_text = new wxNumTextCtrl(page, wxID_ANY, "20",
		wxDefaultPosition, FromDIP(wxSize(60, 23)), wxTE_RIGHT, vald_int);
	m_cell_size_sldr->Bind(wxEVT_SCROLL_CHANGED, &TrackDlg::OnCellSizeChange, this);
	m_cell_size_text->Bind(wxEVT_TEXT, &TrackDlg::OnCellSizeText, this);
	sizer_2->Add(5, 5);
	sizer_2->Add(st, 0, wxALIGN_CENTER);
	sizer_2->Add(m_cell_size_sldr, 1, wxEXPAND);
	sizer_2->Add(m_cell_size_text, 0, wxALIGN_CENTER);
	//uncertainty filter
	wxBoxSizer* sizer_3 = new wxBoxSizer(wxHORIZONTAL);
	m_comp_uncertain_btn = new wxButton(page, wxID_ANY, "Uncertainty",
		wxDefaultPosition, FromDIP(wxSize(80, 23)));
	m_comp_uncertain_low_sldr = new wxSingleSlider(page, wxID_ANY, 0, 0, 20,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_comp_uncertain_low_text = new wxNumTextCtrl(page, wxID_ANY, "0",
		wxDefaultPosition, FromDIP(wxSize(60, 23)), wxTE_RIGHT, vald_int);
	m_comp_uncertain_btn->Bind(wxEVT_BUTTON, &TrackDlg::OnCompUncertainBtn, this);
	m_comp_uncertain_low_sldr->Bind(wxEVT_SCROLL_CHANGED, &TrackDlg::OnCompUncertainLowChange, this);
	m_comp_uncertain_low_text->Bind(wxEVT_TEXT, &TrackDlg::OnCompUncertainLowText, this);
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
	page->SetAutoLayout(true);
	page->SetScrollRate(10, 10);
	return page;
}

wxWindow* TrackDlg::CreateLinkPage(wxWindow *parent)
{
	wxScrolledWindow *page = new wxScrolledWindow(parent);

	wxStaticText *st = 0;

	//selection
	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Selection tools:",
		wxDefaultPosition, FromDIP(wxSize(100, 20)));
	m_comp_id_text2 = new wxNumTextCtrl(page, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(77, 23)), wxTE_PROCESS_ENTER | wxTE_RIGHT);
	m_comp_id_x_btn2 = new wxButton(page, wxID_ANY, "X",
		wxDefaultPosition, FromDIP(wxSize(23, 23)));
	m_comp_append_btn2 = new wxButton(page, wxID_ANY, "Append",
		wxDefaultPosition, FromDIP(wxSize(80, 23)));
	m_comp_clear_btn2 = new wxButton(page, wxID_ANY, "Clear",
		wxDefaultPosition, FromDIP(wxSize(80, 23)));
	m_comp_id_text2->Bind(wxEVT_TEXT, &TrackDlg::OnCompId2Text, this);
	m_comp_id_text2->Bind(wxEVT_TEXT_ENTER, &TrackDlg::OnCompAppend, this);
	m_comp_id_x_btn2->Bind(wxEVT_BUTTON, &TrackDlg::OnCompId2XBtn, this);
	m_comp_append_btn2->Bind(wxEVT_BUTTON, &TrackDlg::OnCompAppend, this);
	m_comp_clear_btn2->Bind(wxEVT_BUTTON, &TrackDlg::OnCompClear, this);
	sizer_1->Add(5, 5);
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	sizer_1->Add(m_comp_id_text2, 0, wxALIGN_CENTER);
	sizer_1->Add(m_comp_id_x_btn2, 0, wxALIGN_CENTER);
	sizer_1->Add(10, 23);
	sizer_1->Add(m_comp_append_btn2, 0, wxALIGN_CENTER);
	sizer_1->Add(m_comp_clear_btn2, 0, wxALIGN_CENTER);
	sizer_1->AddStretchSpacer();

	//ID link controls
	wxBoxSizer* sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	m_cell_exclusive_link_btn = new wxButton(page, wxID_ANY, "Excl. Link",
		wxDefaultPosition, FromDIP(wxSize(80, 23)));
	m_cell_link_btn = new wxButton(page, wxID_ANY, "Link IDs",
		wxDefaultPosition, FromDIP(wxSize(80, 23)));
	m_cell_link_all_btn = new wxButton(page, wxID_ANY, "Link New IDs",
		wxDefaultPosition, FromDIP(wxSize(80, 23)));
	m_cell_exclusive_link_btn->Bind(wxEVT_BUTTON, &TrackDlg::OnCellExclusiveLink, this);
	m_cell_link_btn->Bind(wxEVT_BUTTON, &TrackDlg::OnCellLink, this);
	m_cell_link_all_btn->Bind(wxEVT_BUTTON, &TrackDlg::OnCellLinkAll, this);
	sizer_2->AddStretchSpacer();
	sizer_2->Add(m_cell_exclusive_link_btn, 0, wxALIGN_CENTER);
	sizer_2->Add(m_cell_link_btn, 0, wxALIGN_CENTER);
	sizer_2->Add(m_cell_link_all_btn, 0, wxALIGN_CENTER);
	sizer_2->AddStretchSpacer();

	//ID unlink controls
	wxBoxSizer* sizer_3 = new wxBoxSizer(wxHORIZONTAL);
	m_cell_isolate_btn = new wxButton(page, wxID_ANY, "Isolate",
		wxDefaultPosition, FromDIP(wxSize(80, 23)));
	m_cell_unlink_btn = new wxButton(page, wxID_ANY, "Unlink IDs",
		wxDefaultPosition, FromDIP(wxSize(80, 23)));
	m_cell_isolate_btn->Bind(wxEVT_BUTTON, &TrackDlg::OnCellIsolate, this);
	m_cell_unlink_btn->Bind(wxEVT_BUTTON, &TrackDlg::OnCellUnlink, this);
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
	page->SetAutoLayout(true);
	page->SetScrollRate(10, 10);
	return page;
}

wxWindow* TrackDlg::CreateModifyPage(wxWindow *parent)
{
	wxScrolledWindow *page = new wxScrolledWindow(parent);

	wxStaticText *st = 0;

	//ID input
	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "New ID/Selection:",
		wxDefaultPosition, FromDIP(wxSize(100, 20)));
	m_cell_new_id_text = new wxNumTextCtrl(page, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(77, 23)), wxTE_PROCESS_ENTER | wxTE_RIGHT);
	m_cell_new_id_x_btn = new wxButton(page, wxID_ANY, "X",
		wxDefaultPosition, FromDIP(wxSize(23, 23)));
	m_comp_append_btn3 = new wxButton(page, wxID_ANY, "Append",
		wxDefaultPosition, FromDIP(wxSize(80, 23)));
	m_comp_clear_btn3 = new wxButton(page, wxID_ANY, "Clear",
		wxDefaultPosition, FromDIP(wxSize(80, 23)));
	m_cell_new_id_text->Bind(wxEVT_TEXT, &TrackDlg::OnCellNewIDText, this);
	m_cell_new_id_text->Bind(wxEVT_TEXT_ENTER, &TrackDlg::OnCompAppend, this);
	m_cell_new_id_x_btn->Bind(wxEVT_BUTTON, &TrackDlg::OnCellNewIDX, this);
	m_comp_append_btn3->Bind(wxEVT_BUTTON, &TrackDlg::OnCompAppend, this);
	m_comp_clear_btn3->Bind(wxEVT_BUTTON, &TrackDlg::OnCompClear, this);
	sizer_1->Add(5, 5);
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	sizer_1->Add(m_cell_new_id_text, 0, wxALIGN_CENTER);
	sizer_1->Add(m_cell_new_id_x_btn, 0, wxALIGN_CENTER);
	sizer_1->Add(10, 23);
	sizer_1->Add(m_comp_append_btn3, 0, wxALIGN_CENTER);
	sizer_1->Add(m_comp_clear_btn3, 0, wxALIGN_CENTER);
	sizer_1->AddStretchSpacer();

	//controls
	wxBoxSizer* sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	m_cell_new_id_btn = new wxButton(page, wxID_ANY, "Assign ID",
		wxDefaultPosition, FromDIP(wxSize(85, 23)));
	m_cell_append_id_btn = new wxButton(page, wxID_ANY, "Add ID",
		wxDefaultPosition, FromDIP(wxSize(85, 23)));
	m_cell_replace_id_btn = new wxButton(page, wxID_ANY, "Replace ID",
		wxDefaultPosition, FromDIP(wxSize(85, 23)));
	m_cell_new_id_btn->Bind(wxEVT_BUTTON, &TrackDlg::OnCellNewID, this);
	m_cell_append_id_btn->Bind(wxEVT_BUTTON, &TrackDlg::OnCellAppendID, this);
	m_cell_replace_id_btn->Bind(wxEVT_BUTTON, &TrackDlg::OnCellReplaceID, this);
	sizer_2->AddStretchSpacer();
	sizer_2->Add(m_cell_new_id_btn, 0, wxALIGN_CENTER);
	sizer_2->Add(m_cell_append_id_btn, 0, wxALIGN_CENTER);
	sizer_2->Add(m_cell_replace_id_btn, 0, wxALIGN_CENTER);
	sizer_2->AddStretchSpacer();

	wxBoxSizer* sizer_3 = new wxBoxSizer(wxHORIZONTAL);
	m_cell_combine_id_btn = new wxButton(page, wxID_ANY, "Combine",
		wxDefaultPosition, FromDIP(wxSize(85, 23)));
	m_cell_separate_id_btn = new wxButton(page, wxID_ANY, "Separate",
		wxDefaultPosition, FromDIP(wxSize(85, 23)));
	m_cell_segment_spin = new wxSpinCtrl(page, wxID_ANY, "2",
		wxDefaultPosition, FromDIP(wxSize(40, 21)));
	m_cell_segment_btn = new wxButton(page, wxID_ANY, "Segment",
		wxDefaultPosition, FromDIP(wxSize(65, 23)));
	m_cell_combine_id_btn->Bind(wxEVT_BUTTON, &TrackDlg::OnCellCombineID, this);
	m_cell_separate_id_btn->Bind(wxEVT_BUTTON, &TrackDlg::OnCellSeparateID, this);
	m_cell_segment_spin->Bind(wxEVT_SPINCTRL, &TrackDlg::OnCellSegSpin, this);
	m_cell_segment_spin->Bind(wxEVT_TEXT, &TrackDlg::OnCellSegText, this);
	m_cell_segment_btn->Bind(wxEVT_BUTTON, &TrackDlg::OnCellSegment, this);
	sizer_3->AddStretchSpacer();
	sizer_3->Add(m_cell_combine_id_btn, 0, wxALIGN_CENTER);
	sizer_3->Add(m_cell_separate_id_btn, 0, wxALIGN_CENTER);
	sizer_3->Add(10, 10);
	sizer_3->Add(m_cell_segment_spin, 0, wxALIGN_CENTER);
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
	page->SetAutoLayout(true);
	page->SetScrollRate(10, 10);
	return page;
}

wxWindow* TrackDlg::CreateAnalysisPage(wxWindow *parent)
{
	wxScrolledWindow *page = new wxScrolledWindow(parent);

	wxStaticText *st = 0;

	//conversion
	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Convert to:",
		wxDefaultPosition, FromDIP(wxSize(100, 20)));
	m_convert_to_rulers_btn = new wxButton(page, wxID_ANY, "Rulers",
		wxDefaultPosition, FromDIP(wxSize(80, 23)));
	m_convert_consistent_btn = new wxButton(page, wxID_ANY, "UniIDs",
		wxDefaultPosition, FromDIP(wxSize(80, 23)));
	m_convert_to_rulers_btn->Bind(wxEVT_BUTTON, &TrackDlg::OnConvertToRulers, this);
	m_convert_consistent_btn->Bind(wxEVT_BUTTON, &TrackDlg::OnConvertConsistent, this);
	sizer_1->Add(5, 5);
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	sizer_1->Add(m_convert_to_rulers_btn, 0, wxALIGN_CENTER);
	sizer_1->Add(m_convert_consistent_btn, 0, wxALIGN_CENTER);

	//analysis
	wxBoxSizer* sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Information:",
		wxDefaultPosition, FromDIP(wxSize(100, 20)));
	m_analyze_comp_btn = new wxButton(page, wxID_ANY, "Compnts",
		wxDefaultPosition, FromDIP(wxSize(80, 23)));
	m_analyze_link_btn = new wxButton(page, wxID_ANY, "Links",
		wxDefaultPosition, FromDIP(wxSize(80, 23)));
	m_analyze_uncertain_hist_btn = new wxButton(page, wxID_ANY, "Uncertainty",
		wxDefaultPosition, FromDIP(wxSize(80, 23)));
	m_analyze_path_btn = new wxButton(page, wxID_ANY, "Paths",
		wxDefaultPosition, FromDIP(wxSize(80, 23)));
	m_analyze_comp_btn->Bind(wxEVT_BUTTON, &TrackDlg::OnAnalyzeComp, this);
	m_analyze_link_btn->Bind(wxEVT_BUTTON, &TrackDlg::OnAnalyzeLink, this);
	m_analyze_uncertain_hist_btn->Bind(wxEVT_BUTTON, &TrackDlg::OnAnalyzeUncertainHist, this);
	m_analyze_path_btn->Bind(wxEVT_BUTTON, &TrackDlg::OnAnalyzePath, this);
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
	m_save_result_btn = new wxButton(page, wxID_ANY, "Save As",
		wxDefaultPosition, FromDIP(wxSize(80, 23)));
	m_save_result_btn->Bind(wxEVT_BUTTON, &TrackDlg::OnSaveResult, this);
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
	page->SetAutoLayout(true);
	page->SetScrollRate(10, 10);
	return page;
}

wxWindow* TrackDlg::CreateListPage(wxWindow* parent)
{
	wxScrolledWindow *page = new wxScrolledWindow(parent);

	wxStaticText *st = 0;
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;

	//ghost num
	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Tracks:",
		wxDefaultPosition, FromDIP(wxSize(70, 20)));
	m_ghost_show_tail_chk = new wxCheckBox(page, wxID_ANY, "Tail",
		wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
	m_ghost_num_sldr = new wxSingleSlider(page, wxID_ANY, 10, 0, 20,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_ghost_num_text = new wxNumTextCtrl(page, wxID_ANY, "10",
		wxDefaultPosition, FromDIP(wxSize(60, 23)), wxTE_RIGHT, vald_int);
	m_ghost_show_lead_chk = new wxCheckBox(page, wxID_ANY, "Lead",
		wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
	m_ghost_show_tail_chk->Bind(wxEVT_CHECKBOX, &TrackDlg::OnGhostShowTail, this);
	m_ghost_num_sldr->Bind(wxEVT_SCROLL_CHANGED, &TrackDlg::OnGhostNumChange, this);
	m_ghost_num_text->Bind(wxEVT_TEXT, &TrackDlg::OnGhostNumText, this);
	m_ghost_show_lead_chk->Bind(wxEVT_CHECKBOX, &TrackDlg::OnGhostShowLead, this);
	sizer_1->Add(5, 5);
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	sizer_1->Add(m_ghost_show_tail_chk, 0, wxALIGN_CENTER);
	sizer_1->Add(5, 5);
	sizer_1->Add(m_ghost_num_sldr, 1, wxEXPAND);
	sizer_1->Add(m_ghost_num_text, 0, wxALIGN_CENTER);
	sizer_1->Add(5, 5);
	sizer_1->Add(m_ghost_show_lead_chk, 0, wxALIGN_CENTER);

	//lists
	wxStaticBoxSizer *sizer_2 = new wxStaticBoxSizer(
		wxVERTICAL, page, "ID Lists");
	//titles
	wxBoxSizer* sizer_21 = new wxBoxSizer(wxHORIZONTAL);
	m_cell_time_curr_st = new wxStaticText(page, 0, "\tCurrent T",
		wxDefaultPosition, wxDefaultSize);
	m_cell_time_prev_st = new wxStaticText(page, 0, "\tPrevious T",
		wxDefaultPosition, wxDefaultSize);
	m_cell_prev_btn = new wxButton(page, wxID_ANY, L"\u21e6 Backward (A)",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_cell_next_btn = new wxButton(page, wxID_ANY, L"Forward (D) \u21e8",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_cell_prev_btn->Bind(wxEVT_BUTTON, &TrackDlg::OnCellPrev, this);
	m_cell_next_btn->Bind(wxEVT_BUTTON, &TrackDlg::OnCellNext, this);
	sizer_21->Add(m_cell_time_curr_st, 1, wxEXPAND);
	sizer_21->Add(m_cell_prev_btn, 0, wxALIGN_CENTER);
	sizer_21->Add(m_cell_next_btn, 0, wxALIGN_CENTER);
	sizer_21->Add(m_cell_time_prev_st, 1, wxEXPAND);
	//controls
	wxBoxSizer* sizer_22 = new wxBoxSizer(wxHORIZONTAL);
	m_trace_list_curr = new TrackListCtrl(page);
	m_trace_list_curr->m_type = 0;
	m_trace_list_prev = new TrackListCtrl(page);
	m_trace_list_prev->m_type = 1;
	m_active_list = 0;
	m_trace_list_curr->Bind(wxEVT_KEY_DOWN, &TrackDlg::OnKeyDown, this);
	m_trace_list_curr->Bind(wxEVT_CONTEXT_MENU, &TrackDlg::OnContextMenu, this);
	m_trace_list_curr->Bind(wxEVT_LIST_ITEM_SELECTED, &TrackDlg::OnSelectionChanged, this);
	m_trace_list_prev->Bind(wxEVT_KEY_DOWN, &TrackDlg::OnKeyDown, this);
	m_trace_list_prev->Bind(wxEVT_CONTEXT_MENU, &TrackDlg::OnContextMenu, this);
	m_trace_list_prev->Bind(wxEVT_LIST_ITEM_SELECTED, &TrackDlg::OnSelectionChanged, this);
	sizer_22->Add(m_trace_list_curr, 1, wxEXPAND);
	sizer_22->Add(m_trace_list_prev, 1, wxEXPAND);
	//
	sizer_2->Add(sizer_21, 0, wxEXPAND);
	sizer_2->Add(sizer_22, 1, wxEXPAND);

	wxBoxSizer *sizer_v = new wxBoxSizer(wxVERTICAL);
	sizer_v->Add(10, 10);
	sizer_v->Add(sizer_1, 0, wxEXPAND);
	sizer_v->Add(10, 10);
	sizer_v->Add(sizer_2, 1, wxEXPAND);
	sizer_v->Add(10, 10);

	page->SetSizer(sizer_v);
	page->SetAutoLayout(true);
	page->SetScrollRate(10, 10);
	return page;
}

wxWindow* TrackDlg::CreateOutputPage(wxWindow* parent)
{
	wxScrolledWindow *page = new wxScrolledWindow(parent);

	//stats text
	m_stat_text = new wxTextCtrl(page, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(-1, 100)), wxTE_MULTILINE);
	m_stat_text->SetEditable(false);

	wxBoxSizer *sizer_v = new wxBoxSizer(wxVERTICAL);
	sizer_v->Add(m_stat_text, 1, wxEXPAND);

	page->SetSizer(sizer_v);
	page->SetAutoLayout(true);
	page->SetScrollRate(10, 10);
	return page;
}

void TrackDlg::UpdateTrackFile(const std::wstring& str)
{
	if (str.empty())
		m_load_trace_text->ChangeValue("No track map or track map not saved");
	else
		m_load_trace_text->ChangeValue(str);
}

void TrackDlg::UpdateTrackIter(int ival)
{
	m_map_iter_spin->SetValue(ival);
}

void TrackDlg::UpdateTrackSize(double dval)
{
	m_map_size_spin->SetValue(dval);
}

void TrackDlg::UpdateTrackSimilarity(double dval)
{
	m_map_similar_spin->SetValue(dval);
}

void TrackDlg::UpdateTrackContactFactor(double dval)
{
	m_map_contact_spin->SetValue(dval);
}

void TrackDlg::UpdateTrackConsistent(bool bval)
{
	m_map_consistent_btn->SetValue(bval);
}

void TrackDlg::UpdateTrackMerge(bool bval)
{
	m_map_merge_btn->SetValue(bval);
}

void TrackDlg::UpdateTrackSplit(bool bval)
{
	m_map_split_btn->SetValue(bval);
}

void TrackDlg::UpdateTrackCompId(const std::string& str, const fluo::Color& color)
{
	m_comp_id_text->ChangeValue(str);
	m_comp_id_text2->ChangeValue(str);
	m_comp_id_text->SetBackgroundColour(wxColor(color.r() * 255, color.g() * 255, color.b() * 255));
	m_comp_id_text2->SetBackgroundColour(wxColor(color.r() * 255, color.g() * 255, color.b() * 255));
}

void TrackDlg::UpdateTrackCellSize(double dval)
{
	m_cell_size_sldr->ChangeValue(int(std::round(dval)));
	m_cell_size_text->ChangeValue(wxString::Format("%.0f", dval));
}

void TrackDlg::UpdateTrackUncertainLow(int ival)
{
	m_comp_uncertain_low_sldr->ChangeValue(ival);
	m_cell_size_text->ChangeValue(wxString::Format("%d", ival));
}

void TrackDlg::UpdateTrackNewCompId(const std::string& str, const fluo::Color& color)
{
	m_cell_new_id_text->ChangeValue(str);
	m_cell_new_id_text->SetBackgroundColour(wxColor(color.r() * 255, color.g() * 255, color.b() * 255));
}

void TrackDlg::UpdateTrackClusterNum(int ival)
{
	m_cell_segment_spin->SetValue(wxString::Format("%d", ival));
}

void TrackDlg::UpdateGhostNum(int ival)
{
	m_ghost_num_sldr->ChangeValue(ival);
	m_ghost_num_text->ChangeValue(wxString::Format("%d", ival));
}

void TrackDlg::UpdateGhostEnable(bool bval1, bool bval2)
{
	m_ghost_show_tail_chk->SetValue(bval1);
	m_ghost_show_lead_chk->SetValue(bval2);
}

void TrackDlg::PopulateTrackList(
	TrackListCtrl* list,
	const std::vector<TrackItem>& items)
{
	list->Freeze();

	list->DeleteAllItems();

	for (const auto& item : items)
	{
		wxColor color(
			item.color.r() * 255,
			item.color.g() * 255,
			item.color.b() * 255);

		list->Append(
			item.glyph,
			item.id,
			color,
			item.size,
			item.x,
			item.y,
			item.z);
	}

	list->Thaw();
}

void TrackDlg::UpdateTracks(
	const TrackViewData& data)
{
	PopulateTrackList(
		m_trace_list_curr,
		data.current);

	PopulateTrackList(
		m_trace_list_prev,
		data.previous);

	m_cell_time_curr_st->SetLabel(
		wxString::Format(
			"\tCurrent T: %d",
			data.cur_time));

	if (data.cur_time != data.prv_time)
	{
		m_cell_time_prev_st->SetLabel(
			wxString::Format(
				"\tPrevious T: %d",
				data.prv_time));
	}
	else
	{
		m_cell_time_prev_st->SetLabel(
			"\tPrevious T");
	}
}

void TrackDlg::UpdateStatText(const std::wstring& str)
{
	(*m_stat_text) << str;
}

int TrackDlg::GetMapIter()
{
	return m_map_iter_spin->GetValue();
}

int TrackDlg::GetMapSize()
{
	return m_map_size_spin->GetValue();
}

bool TrackDlg::GetMapConsistent()
{
	return m_map_consistent_btn->GetValue();
}

bool TrackDlg::GetMapMerge()
{
	return m_map_merge_btn->GetValue();
}

bool TrackDlg::GetMapSplit()
{
	return m_map_split_btn->GetValue();
}

std::string TrackDlg::GetCompId()
{
	return m_comp_id_text->GetValue().ToStdString();
}

int TrackDlg::GetCellSize()
{
	wxString str = m_cell_size_text->GetValue();
	unsigned long ival = 0;
	if (str.ToULong(&ival))
		return static_cast<int>(ival);
	return 0;
}

int TrackDlg::GetCompUncertainLow()
{
	wxString str = m_comp_uncertain_low_text->GetValue();
	long ival;
	if (str.ToLong(&ival))
		return static_cast<int>(ival);
	return 0;
}

std::string TrackDlg::GetCompId2()
{
	return m_comp_id_text2->GetValue().ToStdString();
}

std::string TrackDlg::GetCellNewId()
{
	return m_cell_new_id_text->GetValue().ToStdString();
}

int TrackDlg::GetClusterNum()
{
	return m_cell_segment_spin->GetValue();
}

std::string TrackDlg::GetStatText()
{
	return m_stat_text->GetValue().ToStdString();
}

int TrackDlg::GetGhostNum()
{
	wxString str = m_ghost_num_text->GetValue();
	long ival;
	if (str.ToLong(&ival))
		return static_cast<int>(ival);
	return 0;
}

bool TrackDlg::GetGhostShowTail()
{
	return m_ghost_show_tail_chk->GetValue();
}

bool TrackDlg::GetGhostShowLead()
{
	return m_ghost_show_lead_chk->GetValue();
}

int TrackDlg::GetActiveList()
{
	if (m_active_list == m_trace_list_curr)
		return 0;
	else if (m_active_list == m_trace_list_prev)
		return 1;
	return -1;
}

std::vector<TrackItem> TrackDlg::GetSelection()
{
	std::vector<TrackItem> list;
	long item = -1;
	while (true)
	{
		item = m_active_list->GetNextItem(
			item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (item == -1)
			break;
		else
			AddLabel(item, m_active_list, list);
	}
	if (list.size() == 0)
	{
		item = -1;
		while (true)
		{
			item = m_active_list->GetNextItem(
				item, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
			if (item == -1)
				break;
			else
				AddLabel(item, m_active_list, list);
		}
	}
	return list;
}

void TrackDlg::OnClearTrace(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstClearTrack });
}

void TrackDlg::OnLoadTrace(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstTrackFile });
}

void TrackDlg::OnSaveTrace(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstSaveTrackFile });
}

void TrackDlg::OnSaveasTrace(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstSaveAsTrackFile });
}

//auto tracking
void TrackDlg::OnGenMapBtn(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstGenerateMap });
}

void TrackDlg::OnRefineTBtn(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstRefineTime });
}

void TrackDlg::OnRefineAllBtn(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstRefineAll });
}

//settings
void TrackDlg::OnMapIterSpin(wxSpinEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstTrackIter });
}

void TrackDlg::OnMapIterText(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstTrackIter });
}

void TrackDlg::OnMapSizeSpin(wxSpinEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstTrackSize });
}

void TrackDlg::OnMapSizeText(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstTrackSize });
}

void TrackDlg::OnMapConsistentBtn(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstTrackConsistent });
}

void TrackDlg::OnMapMergeBtn(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstTrackMerge });
}

void TrackDlg::OnMapSplitBtn(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstTrackSplit });
}

void TrackDlg::OnMapSimilarSpin(wxSpinDoubleEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstTrackSimilarity });
}

void TrackDlg::OnMapSimilarText(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstTrackSimilarity });
}

void TrackDlg::OnMapContactSpin(wxSpinDoubleEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstTrackContactFactor });
}

void TrackDlg::OnMapContactText(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstTrackContactFactor });
}

//selection page
void TrackDlg::OnCompIDText(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstTrackCompId });
}

void TrackDlg::OnCompIDXBtn(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstTrackClearCompId });
}

void TrackDlg::OnCompFull(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstCompFull });
}

void TrackDlg::OnCompExclusive(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstCompExclusive });
}

void TrackDlg::OnCompAppend(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstCompAppend });
}

void TrackDlg::OnCompClear(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstCompClear });
}

void TrackDlg::OnShuffle(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstShuffle });
}

//cell size filter
void TrackDlg::OnCellSizeChange(wxScrollEvent& event)
{
	int ival = m_cell_size_sldr->GetValue();
	wxString str = wxString::Format("%d", ival);
	if (str != m_cell_size_text->GetValue())
		m_cell_size_text->SetValue(str);
}

void TrackDlg::OnCellSizeText(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstTrackCellSize });
}

void TrackDlg::OnCompUncertainBtn(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstComputeUncertainty });
}

void TrackDlg::OnCompUncertainLowChange(wxScrollEvent& event)
{
	int ival = m_comp_uncertain_low_sldr->GetValue();
	wxString str = wxString::Format("%d", ival);
	if (str != m_comp_uncertain_low_text->GetValue())
		m_comp_uncertain_low_text->SetValue(str);
}

void TrackDlg::OnCompUncertainLowText(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstTrackUncertainLow });
}

//link page
void TrackDlg::OnCompId2Text(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstTrackCompId2 });
}

void TrackDlg::OnCompId2XBtn(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstTrackClearCompId });
}

void TrackDlg::OnCellExclusiveLink(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstCellExclusiveLink });
}

void TrackDlg::OnCellLink(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstCellLink });
}

void TrackDlg::OnCellLinkAll(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstCellLinkAll });
}

void TrackDlg::OnCellIsolate(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstCellIsolate });
}

void TrackDlg::OnCellUnlink(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstCellUnlink });
}

//modify page
//ID edit controls
void TrackDlg::OnCellNewIDText(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstTrackNewCompId });
}

void TrackDlg::OnCellNewIDX(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstTrackClearCompNewId });
}

void TrackDlg::OnCellNewID(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstCreateCellNewId });
}

void TrackDlg::OnCellAppendID(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstCellAppendId });
}

void TrackDlg::OnCellReplaceID(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstCellReplaceId });
}

void TrackDlg::OnCellCombineID(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstCellCombineId });
}

void TrackDlg::OnCellSeparateID(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstCellSeparateId });
}

void TrackDlg::OnCellSegment(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstCellSegment });
}

void TrackDlg::OnCellSegSpin(wxSpinEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstCellClusterNum });
}

void TrackDlg::OnCellSegText(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstCellClusterNum });
}

//analysis
void TrackDlg::OnConvertToRulers(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstTrackConvertRulers });
}

void TrackDlg::OnConvertConsistent(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstTrackConsistent });
}

void TrackDlg::OnAnalyzeComp(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstAnalyzeComps });
}

void TrackDlg::OnAnalyzeLink(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstAnalyzeLinks });
}

void TrackDlg::OnAnalyzeUncertainHist(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstAnalyzeUncertainty });
}

void TrackDlg::OnAnalyzePath(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstAnalyzePaths });
}

void TrackDlg::OnSaveResult(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstTrackSaveResult });
}

void TrackDlg::OnCellPrev(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstCellPrev });
}

void TrackDlg::OnCellNext(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstCellNext });
}

void TrackDlg::OnGhostNumChange(wxScrollEvent& event)
{
	int ival = m_ghost_num_sldr->GetValue();
	wxString str = wxString::Format("%d", ival);
	if (str != m_ghost_num_text->GetValue())
		m_ghost_num_text->SetValue(str);
}

void TrackDlg::OnGhostNumText(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstGhostNum });
}

void TrackDlg::OnGhostShowTail(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstGhostShowTail });
}

void TrackDlg::OnGhostShowLead(wxCommandEvent& event)
{
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstGhostShowLead });
}

void TrackDlg::AddLabel(long item, TrackListCtrl* trace_list_ctrl, std::vector<TrackItem>& list)
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

	list.push_back(TrackItem(
		L"",
		static_cast<unsigned int>(id),
		fluo::Color(),
		static_cast<int>(size),
		x, y, z));
}

void TrackDlg::OnSelectionChanged(wxListEvent& event)
{
	m_active_list = dynamic_cast<TrackListCtrl*>(event.GetEventObject());
	if (!m_active_list)
		return;
	auto agent = m_agent->As<TrackDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstTrackListSel });
}

void TrackDlg::OnContextMenu(wxContextMenuEvent& event)
{
	m_active_list = dynamic_cast<TrackListCtrl*>(event.GetEventObject());
	if (!m_active_list)
		return;
	if (!m_active_list->GetSelectedItemCount())
		return;

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
	menu.Append(ID_CopyText, "Copy");
	menu.Append(ID_Delete, "Delete");

	PopupMenu(&menu, point.x, point.y);
}

void TrackDlg::OnMenuItem(wxCommandEvent& event)
{
	if (!m_active_list)
		return;
	int id = event.GetId();

	switch (id)
	{
	case ID_CopyText:
		m_active_list->CopySelection();
		break;
	case ID_Delete:
		if (auto agent = m_agent->As<TrackDlgAgent>())
			agent->UpdateUIToData({ gstTrackListDelete });
		break;
	}
}

void TrackDlg::OnKeyDown(wxKeyEvent& event)
{
	if (!m_active_list)
		return;

	if (event.GetKeyCode() == WXK_DELETE ||
		event.GetKeyCode() == WXK_BACK)
	{
		if (auto agent = m_agent->As<TrackDlgAgent>())
			agent->UpdateUIToData({ gstTrackListDelete });
	}
	if (event.GetKeyCode() == wxKeyCode('C') &&
		wxGetKeyState(WXK_CONTROL))
	{
		m_active_list->CopySelection();
	}
}

