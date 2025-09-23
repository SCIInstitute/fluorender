/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2025 Scientific Computing and Imaging Institute,
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
#include <Global.h>
#include <Names.h>
#include <MainSettings.h>
#include <MainFrame.h>
#include <RenderView.h>
#include <CurrentObjects.h>
#include <VolumeData.h>
#include <VolumeGroup.h>
#include <TrackGroup.h>
#include <DataManager.h>
#include <CompEditor.h>
#include <CompSelector.h>
#include <CompAnalyzer.h>
#include <Cell.h>
#include <TrackMap.h>
#include <MovieMaker.h>
#include <Color.h>
#include <wxSingleSlider.h>
#include <ModalDlg.h>
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

void TrackListCtrl::Append(wxString &gtype, unsigned int id, wxColor color,
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
TrackDlg::TrackDlg(MainFrame* frame)
	: TabbedPanel(frame, frame,
		wxDefaultPosition,
		frame->FromDIP(wxSize(500, 620)),
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
	glbin_trackmap_proc.RegisterInfoOutFunc(
		std::bind(&TrackDlg::WriteInfo, this, std::placeholders::_1));

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
	m_comp_id_text = new wxTextCtrl(page, wxID_ANY, "",
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
	m_cell_size_text = new wxTextCtrl(page, wxID_ANY, "20",
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
	m_comp_uncertain_low_text = new wxTextCtrl(page, wxID_ANY, "0",
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
	m_comp_id_text2 = new wxTextCtrl(page, wxID_ANY, "",
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
	m_cell_new_id_text = new wxTextCtrl(page, wxID_ANY, "",
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
	m_ghost_num_text = new wxTextCtrl(page, wxID_ANY, "10",
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

void TrackDlg::FluoUpdate(const fluo::ValueCollection& vc)
{
	if (FOUND_VALUE(gstNull))
		return;

	bool update_all = vc.empty();

	TrackGroup* trkg = glbin_current.GetTrackGroup();
	if (!trkg)
		return;

	wxString str;
	int ival;
	double dval;

	//create page
	if (update_all || FOUND_VALUE(gstTrackFile))
	{
		//track file
		str = trkg->GetPath();
		if (str.IsEmpty())
			m_load_trace_text->ChangeValue("No track map or track map not saved");
		else
			m_load_trace_text->ChangeValue(str);
	}

	if (update_all || FOUND_VALUE(gstTrackIter))
		m_map_iter_spin->SetValue(glbin_settings.m_track_iter);

	if (update_all || FOUND_VALUE(gstTrackSize))
		m_map_size_spin->SetValue(glbin_settings.m_component_size);

	if (update_all || FOUND_VALUE(gstTrackSimilarity))
		m_map_similar_spin->SetValue(glbin_settings.m_similarity);

	if (update_all || FOUND_VALUE(gstTrackContactFactor))
		m_map_contact_spin->SetValue(glbin_settings.m_contact_factor);

	if (update_all || FOUND_VALUE(gstTrackConsistent))
		m_map_consistent_btn->SetValue(glbin_settings.m_consistent_color);

	if (update_all || FOUND_VALUE(gstTrackMerge))
		m_map_merge_btn->SetValue(glbin_settings.m_try_merge);

	if (update_all || FOUND_VALUE(gstTrackSplit))
		m_map_split_btn->SetValue(glbin_settings.m_try_split);

	//select page
	if (update_all || FOUND_VALUE(gstTrackCompId))
	{
		m_comp_id_text->ChangeValue(m_comp_id);
		m_comp_id_text2->ChangeValue(m_comp_id);
		unsigned long id;
		wxColor color(255, 255, 255);
		if (m_comp_id.ToULong(&id))
		{
			if (!id)
				color = wxColor(24, 167, 181);
			else
			{
				auto vd = glbin_current.vol_data.lock();
				bool shuffle = vd ? vd->GetShuffle() : 0;
				fluo::Color c(id, shuffle);
				color = wxColor(c.r() * 255, c.g() * 255, c.b() * 255);
			}
		}
		m_comp_id_text->SetBackgroundColour(color);
		m_comp_id_text2->SetBackgroundColour(color);
	}

	if (update_all || FOUND_VALUE(gstTrackCellSize))
	{
		dval = glbin_settings.m_component_size;
		m_cell_size_sldr->ChangeValue(int(std::round(dval)));
		m_cell_size_text->ChangeValue(wxString::Format("%.0f", dval));
	}

	if (update_all || FOUND_VALUE(gstTrackUncertainLow))
	{
		ival = trkg->GetUncertainLow();
		m_comp_uncertain_low_sldr->ChangeValue(ival);
		m_cell_size_text->ChangeValue(wxString::Format("%d", ival));
	}

	//modify page
	if (update_all || FOUND_VALUE(gstTrackNewCompId))
	{
		m_cell_new_id_text->ChangeValue(m_comp_id3);
		unsigned long id;
		wxColor color(255, 255, 255);
		if (m_comp_id3.ToULong(&id))
		{
			if (!id)
				color = wxColor(24, 167, 181);
			else
			{
				auto vd = glbin_current.vol_data.lock();
				bool shuffle = vd ? vd->GetShuffle() : 0;
				fluo::Color c(id, shuffle);
				color = wxColor(c.r() * 255, c.g() * 255, c.b() * 255);
			}
		}
		m_cell_new_id_text->SetBackgroundColour(color);
	}

	if (update_all || FOUND_VALUE(gstTrackClusterNum))
	{
		ival = glbin_trackmap_proc.GetClusterNum();
		m_cell_segment_spin->SetValue(wxString::Format("%d", ival));
	}

	//analysis page (empty)
	//lists
	if (update_all || FOUND_VALUE(gstGhostNum))
	{
		ival = trkg->GetGhostNum();
		m_ghost_num_sldr->ChangeValue(ival);
		m_ghost_num_text->ChangeValue(wxString::Format("%d", ival));
	}

	if (update_all || FOUND_VALUE(gstGhostEnable))
	{
		m_ghost_show_tail_chk->SetValue(trkg->GetDrawTail());
		m_ghost_show_lead_chk->SetValue(trkg->GetDrawLead());
	}

	if (update_all || FOUND_VALUE(gstTrackList))
	{
		UpdateTrackList();
		UpdateTracks();
		//Layout();
	}
}

void TrackDlg::UpdateTrackList()
{
	TrackGroup* trkg = glbin_current.GetTrackGroup();
	if (!trkg)
		return;
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;

	int shuffle = vd->GetShuffle();
	int cur_time = trkg->GetCurTime();
	int prv_time = trkg->GetPrvTime();
	wxString item_gtype;
	wxString item_id;
	wxString item_size;
	wxString item_x;
	wxString item_y;
	wxString item_z;
	unsigned long id;
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

		//set tiem text
		wxString str;
		str = wxString::Format("\tCurrent T: %d", cur_time);
		m_cell_time_curr_st->SetLabel(str);
		if (prv_time != cur_time)
			m_cell_time_prev_st->SetLabel(
				wxString::Format("\tPrevious T: %d", prv_time));
		else
			m_cell_time_prev_st->SetLabel("\tPrevious T");
	}
}

void TrackDlg::UpdateTracks()
{
	TrackGroup* trkg = glbin_current.GetTrackGroup();
	if (!trkg)
		return;
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;

	int shuffle = vd->GetShuffle();

	m_trace_list_curr->DeleteAllItems();

	flrd::CelpList sel_cells = trkg->GetCellList();
	std::vector<flrd::Celp> cells;
	for (auto siter = sel_cells.begin();
		siter != sel_cells.end(); ++siter)
		cells.push_back(siter->second);

	if (cells.empty())
		return;
	else
		std::sort(cells.begin(), cells.end(),
			[](const flrd::Celp& c1, const flrd::Celp& c2) -> bool
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

		m_trace_list_curr->Append(gtype, id, color, size,
			center.x(), center.y(), center.z());
	}
	Thaw();
}

void TrackDlg::LoadTrackFile(const std::wstring& file)
{
	auto view = glbin_current.render_view.lock();
	view->LoadTrackGroup(file);
	FluoUpdate({ gstTrackFile });
}

bool TrackDlg::SaveTrackFile()
{
	auto view = glbin_current.render_view.lock();
	if (!view)
		return false;
	TrackGroup* trkg = glbin_current.GetTrackGroup();
	if (!trkg)
		return false;
	std::wstring str = trkg->GetPath();
	if (std::filesystem::exists(str))
		return view->SaveTrackGroup(str);
	return false;
}

void TrackDlg::SaveTrackFile(const std::wstring& file)
{
	auto view = glbin_current.render_view.lock();
	view->SaveTrackGroup(file);
}

void TrackDlg::SaveasTrackFile()
{
	ModalDlg fopendlg(
		m_frame, "Save a FluoRender track file",
		"", "", "*.track", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	int rval = fopendlg.ShowModal();
	if (rval == wxID_OK)
	{
		std::wstring filename = fopendlg.GetPath().ToStdWstring();
		SaveTrackFile(filename);
	}
}

void TrackDlg::DeleteSelection(int type)
{
	if (type == 0)
	{
		glbin_comp_selector.DeleteList();
		FluoRefresh(0, { gstTrackList, gstSelUndoRedo },
			{ glbin_current.GetViewId() });
	}
	else
		m_active_list->DeleteSelection();
}

void TrackDlg::WriteInfo(const std::wstring& str)
{
	(*m_stat_text) << str;
}

void TrackDlg::OnClearTrace(wxCommandEvent& event)
{
	TrackGroup* trkg = glbin_current.GetTrackGroup();
	if (trkg)
		trkg->Clear();
	FluoUpdate({ gstTrackFile });
}

void TrackDlg::OnLoadTrace(wxCommandEvent& event)
{
	ModalDlg fopendlg(
		m_frame, "Choose a FluoRender track file",
		"", "", "*.track", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	int rval = fopendlg.ShowModal();
	if (rval == wxID_OK)
	{
		std::wstring filename = fopendlg.GetPath().ToStdWstring();
		LoadTrackFile(filename);
	}
}

void TrackDlg::OnSaveTrace(wxCommandEvent& event)
{
	if (!SaveTrackFile())
		SaveasTrackFile();
}

void TrackDlg::OnSaveasTrace(wxCommandEvent& event)
{
	SaveasTrackFile();
}

//auto tracking
void TrackDlg::OnGenMapBtn(wxCommandEvent& event)
{
	glbin_trackmap_proc.GenMap();
	//enable script
	glbin_settings.m_run_script = true;
	std::filesystem::path p = std::filesystem::current_path();
	p = p / "Scripts" / "track_selected_results.txt";
	glbin_settings.m_script_file = p.wstring();
	m_frame->UpdateProps({ gstMovPlay, gstRunScript, gstScriptFile, gstScriptSelect });
}

void TrackDlg::OnRefineTBtn(wxCommandEvent& event)
{
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;

	glbin_trackmap_proc.RefineMap(view->m_tseq_cur_num);
}

void TrackDlg::OnRefineAllBtn(wxCommandEvent& event)
{
	glbin_trackmap_proc.RefineMap();
}

//settings
void TrackDlg::OnMapIterSpin(wxSpinEvent& event)
{
	glbin_settings.m_track_iter = m_map_iter_spin->GetValue();
}

void TrackDlg::OnMapIterText(wxCommandEvent& event)
{
	glbin_settings.m_track_iter = m_map_iter_spin->GetValue();
}

void TrackDlg::OnMapSizeSpin(wxSpinEvent& event)
{
	glbin_settings.m_component_size = m_map_size_spin->GetValue();
}

void TrackDlg::OnMapSizeText(wxCommandEvent& event)
{
	glbin_settings.m_component_size = m_map_size_spin->GetValue();
}

void TrackDlg::OnMapConsistentBtn(wxCommandEvent& event)
{
	glbin_settings.m_consistent_color = m_map_consistent_btn->GetValue();
}

void TrackDlg::OnMapMergeBtn(wxCommandEvent& event)
{
	glbin_settings.m_try_merge = m_map_merge_btn->GetValue();
}

void TrackDlg::OnMapSplitBtn(wxCommandEvent& event)
{
	glbin_settings.m_try_split = m_map_split_btn->GetValue();
}

void TrackDlg::OnMapSimilarSpin(wxSpinDoubleEvent& event)
{
	glbin_settings.m_similarity = m_map_similar_spin->GetValue();
}

void TrackDlg::OnMapSimilarText(wxCommandEvent& event)
{
	glbin_settings.m_similarity = m_map_similar_spin->GetValue();
}

void TrackDlg::OnMapContactSpin(wxSpinDoubleEvent& event)
{
	glbin_settings.m_contact_factor = m_map_contact_spin->GetValue();
}

void TrackDlg::OnMapContactText(wxCommandEvent& event)
{
	glbin_settings.m_contact_factor = m_map_contact_spin->GetValue();
}

//selection page
void TrackDlg::OnCompIDText(wxCommandEvent& event)
{
	m_comp_id = m_comp_id_text->GetValue();
	unsigned long ival;
	if (m_comp_id.IsEmpty())
		glbin_comp_selector.SetId(0, true);
	else if (m_comp_id.ToULong(&ival))
		glbin_comp_selector.SetId(ival, false);

	FluoUpdate({ gstTrackCompId });
}

void TrackDlg::OnCompIDXBtn(wxCommandEvent& event)
{
	m_comp_id = "";
	glbin_comp_selector.SetId(0, true);
	FluoUpdate({ gstTrackCompId });
}

void TrackDlg::OnCompFull(wxCommandEvent& event)
{
	if (m_comp_id.empty())
		glbin_comp_selector.CompFull();
	else
	{
		wxCommandEvent e;
		OnCompAppend(e);
	}
}

void TrackDlg::OnCompExclusive(wxCommandEvent& event)
{
	glbin_comp_selector.Exclusive();
	auto view = glbin_current.render_view.lock();
	if (view)
		view->GetTraces(false);
	FluoRefresh(0, { gstTrackList, gstSelUndoRedo },
		{ glbin_current.GetViewId(view.get()) });
}

void TrackDlg::OnCompAppend(wxCommandEvent& event)
{
	bool get_all = m_comp_id.Lower() == "all" ? true : false;
	glbin_comp_selector.Select(get_all);
	auto view = glbin_current.render_view.lock();
	if (view)
		view->GetTraces(false);
	FluoRefresh(0, { gstTrackList, gstSelUndoRedo },
		{ glbin_current.GetViewId(view.get()) });
}

void TrackDlg::OnCompClear(wxCommandEvent& event)
{
	glbin_comp_selector.Clear();
	auto view = glbin_current.render_view.lock();
	if (view)
		view->GetTraces(false);
	FluoRefresh(0, { gstTrackList, gstSelUndoRedo },
		{ glbin_current.GetViewId(view.get()) });
}

void TrackDlg::OnShuffle(wxCommandEvent& event)
{
	//get current vd
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;

	vd->IncShuffle();
	FluoRefresh(3, { gstNull },
		{ glbin_current.GetViewId() });
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
	wxString str = m_cell_size_text->GetValue();
	unsigned long ival = 0;
	str.ToULong(&ival);
	m_cell_size_sldr->ChangeValue(ival);

	if (ival > 0)
	{
		glbin_comp_selector.SetUseMin(true);
		glbin_comp_selector.SetMinNum(ival);
		TrackGroup* trkg = glbin_current.GetTrackGroup();
		if (trkg)
			trkg->SetCellSize(ival);
	}
	else
		glbin_comp_selector.SetUseMin(false);
}

void TrackDlg::OnCompUncertainBtn(wxCommandEvent& event)
{
	glbin_trackmap_proc.GetCellsByUncertainty(false);
	FluoRefresh(0, { gstTrackList, gstSelUndoRedo },
		{ glbin_current.GetViewId() });
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
	wxString str = m_comp_uncertain_low_text->GetValue();
	long ival;
	str.ToLong(&ival);
	m_comp_uncertain_low_sldr->ChangeValue(ival);

	glbin_trackmap_proc.SetUncertainLow(ival);
	glbin_trackmap_proc.GetCellsByUncertainty(true);
	FluoRefresh(0, { gstTrackList, gstSelUndoRedo },
		{ glbin_current.GetViewId() });
}

//link page
void TrackDlg::OnCompId2Text(wxCommandEvent& event)
{
	m_comp_id = m_comp_id_text2->GetValue();
	unsigned long ival;
	if (m_comp_id.IsEmpty())
		glbin_comp_selector.SetId(0, true);
	else if (m_comp_id.ToULong(&ival))
		glbin_comp_selector.SetId(ival, false);

	FluoUpdate({ gstTrackCompId });
}

void TrackDlg::OnCompId2XBtn(wxCommandEvent& event)
{
	m_comp_id = "";
	glbin_comp_selector.SetId(0, true);
	FluoUpdate({ gstTrackCompId });
}

void TrackDlg::OnCellExclusiveLink(wxCommandEvent& event)
{
	glbin_trackmap_proc.LinkCells(true);
	FluoRefresh(3, { gstNull },
		{ glbin_current.GetViewId() });
}

void TrackDlg::OnCellLink(wxCommandEvent& event)
{
	glbin_trackmap_proc.LinkCells(false);
	FluoRefresh(3, { gstNull },
		{ glbin_current.GetViewId() });
}

void TrackDlg::OnCellLinkAll(wxCommandEvent& event)
{
	glbin_trackmap_proc.LinkAllCells();
	FluoRefresh(0, { gstTrackList, gstSelUndoRedo },
		{ glbin_current.GetViewId() });
}

void TrackDlg::OnCellIsolate(wxCommandEvent& event)
{
	glbin_trackmap_proc.IsolateCells();
	FluoRefresh(0, { gstTrackList, gstSelUndoRedo },
		{ glbin_current.GetViewId() });
}

void TrackDlg::OnCellUnlink(wxCommandEvent& event)
{
	glbin_trackmap_proc.UnlinkCells();
	FluoRefresh(0, { gstTrackList, gstSelUndoRedo },
		{ glbin_current.GetViewId() });
}

//modify page
//ID edit controls
void TrackDlg::OnCellNewIDText(wxCommandEvent& event)
{
	m_comp_id3 = m_cell_new_id_text->GetValue();
	unsigned long ival;
	if (m_comp_id3.IsEmpty())
		glbin_comp_editor.SetId(0, true);
	else if (m_comp_id.ToULong(&ival))
		glbin_comp_editor.SetId(ival, false);

	FluoUpdate({ gstTrackNewCompId });
}

void TrackDlg::OnCellNewIDX(wxCommandEvent& event)
{
	m_comp_id3 = "";
	glbin_comp_editor.SetId(0, true);
	FluoUpdate({ gstTrackNewCompId });
}

void TrackDlg::OnCellNewID(wxCommandEvent& event)
{
	glbin_comp_editor.NewId(false, true);
	FluoRefresh(0, { gstTrackList, gstSelUndoRedo },
		{ glbin_current.GetViewId() });
}

void TrackDlg::OnCellAppendID(wxCommandEvent& event)
{
	glbin_comp_editor.NewId(true, true);
	FluoRefresh(0, { gstTrackList, gstSelUndoRedo },
		{ glbin_current.GetViewId() });
}

void TrackDlg::OnCellReplaceID(wxCommandEvent& event)
{
	glbin_comp_editor.ReplaceList();
	FluoRefresh(0, { gstTrackList, gstSelUndoRedo },
		{ glbin_current.GetViewId() });
}

void TrackDlg::OnCellCombineID(wxCommandEvent& event)
{
	glbin_comp_editor.CombineList();
	FluoRefresh(0, { gstTrackList, gstSelUndoRedo },
		{ glbin_current.GetViewId() });
}

void TrackDlg::OnCellSeparateID(wxCommandEvent& event)
{
	glbin_trackmap_proc.DivideCells();
	FluoRefresh(0, { gstTrackList, gstSelUndoRedo },
		{ glbin_current.GetViewId() });
}

void TrackDlg::OnCellSegment(wxCommandEvent& event)
{
	glbin_trackmap_proc.SegmentCells();
	FluoRefresh(0, { gstTrackList, gstSelUndoRedo },
		{ glbin_current.GetViewId() });
}

void TrackDlg::OnCellSegSpin(wxSpinEvent& event)
{
	glbin_trackmap_proc.SetClusterNum(m_cell_segment_spin->GetValue());
}

void TrackDlg::OnCellSegText(wxCommandEvent& event)
{
	glbin_trackmap_proc.SetClusterNum(m_cell_segment_spin->GetValue());
}

//analysis
void TrackDlg::OnConvertToRulers(wxCommandEvent& event)
{
	glbin_trackmap_proc.ConvertRulers();
	FluoRefresh(0, { gstTrackList, gstSelUndoRedo, gstRulerList },
		{ glbin_current.GetViewId() });
}

void TrackDlg::OnConvertConsistent(wxCommandEvent& event)
{
	glbin_trackmap_proc.ConvertConsistent();
	FluoRefresh(0, { gstTrackList, gstSelUndoRedo },
		{ glbin_current.GetViewId() });
}

void TrackDlg::OnAnalyzeComp(wxCommandEvent& event)
{
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;

	glbin_comp_analyzer.SetVolume(vd);
	glbin_comp_analyzer.Analyze();
	std::string str;
	glbin_comp_analyzer.OutputCompListStr(str, 1);
	m_stat_text->ChangeValue(str);
}

void TrackDlg::OnAnalyzeLink(wxCommandEvent& event)
{
	glbin_trackmap_proc.AnalyzeLink();
}

void TrackDlg::OnAnalyzeUncertainHist(wxCommandEvent& event)
{
	m_stat_text->ChangeValue("");
	glbin_trackmap_proc.AnalyzeUncertainty();
}

void TrackDlg::OnAnalyzePath(wxCommandEvent& event)
{
	m_stat_text->ChangeValue("");
	glbin_trackmap_proc.AnalyzePath();
}

void TrackDlg::SaveOutputResult(wxString &filename)
{
	std::ofstream os;
	OutputStreamOpen(os, filename.ToStdString());

	wxString str;
	str = m_stat_text->GetValue();

	os << str;

	os.close();
}

void TrackDlg::OnSaveResult(wxCommandEvent& event)
{
	ModalDlg fopendlg(
		m_frame, "Save results", "", "",
		"Text file (*.txt)|*.txt",
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	int rval = fopendlg.ShowModal();
	if (rval == wxID_OK)
	{
		wxString filename = fopendlg.GetPath();
		SaveOutputResult(filename);
	}
}

void TrackDlg::OnCellPrev(wxCommandEvent& event)
{
	if (glbin_moviemaker.IsRunning())
		return;
	int frame = glbin_moviemaker.GetCurrentFrame();
	frame--;
	glbin_moviemaker.SetCurrentFrame(frame);
	fluo::ValueCollection vc = { gstMovCurTime, gstMovProgSlider, gstCurrentFrame, gstMovSeqNum, gstTrackList };
	FluoRefresh(0, vc,
		{ glbin_current.GetViewId()});
}

void TrackDlg::OnCellNext(wxCommandEvent& event)
{
	if (glbin_moviemaker.IsRunning())
		return;
	int frame = glbin_moviemaker.GetCurrentFrame();
	frame++;
	glbin_moviemaker.SetCurrentFrame(frame);
	fluo::ValueCollection vc = { gstMovCurTime, gstMovProgSlider, gstCurrentFrame, gstMovSeqNum, gstTrackList };
	FluoRefresh(0, vc,
		{ glbin_current.GetViewId() });
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
	wxString str = m_ghost_num_text->GetValue();
	long ival;
	str.ToLong(&ival);
	m_ghost_num_sldr->ChangeValue(ival);

	TrackGroup* trkg = glbin_current.GetTrackGroup();
	if (!trkg)
		return;

	trkg->SetGhostNum(ival);
	FluoRefresh(3, { gstNull },
		{ glbin_current.GetViewId() });
}

void TrackDlg::OnGhostShowTail(wxCommandEvent& event)
{
	bool bval = m_ghost_show_tail_chk->GetValue();

	TrackGroup* trkg = glbin_current.GetTrackGroup();
	if (!trkg)
		return;

	trkg->SetDrawTail(bval);
	FluoRefresh(3, { gstNull },
		{ glbin_current.GetViewId() });
}

void TrackDlg::OnGhostShowLead(wxCommandEvent& event)
{
	bool bval = m_ghost_show_lead_chk->GetValue();

	TrackGroup* trkg = glbin_current.GetTrackGroup();
	if (!trkg)
		return;

	trkg->SetDrawLead(bval);
	FluoRefresh(3, { gstNull },
		{ glbin_current.GetViewId() });
}

void TrackDlg::AddLabel(long item, TrackListCtrl* trace_list_ctrl, flrd::CelpList& list)
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

void TrackDlg::OnSelectionChanged(wxListEvent& event)
{
	m_active_list = dynamic_cast<TrackListCtrl*>(event.GetEventObject());
	if (!m_active_list)
		return;
	flrd::CelpList list;
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

	if (m_active_list == m_trace_list_curr)
	{
		glbin_trackmap_proc.SetListIn(list);
		glbin_comp_editor.SetList(list);
		glbin_comp_selector.SetList(list);
	}
	else if (m_active_list == m_trace_list_prev)
		glbin_trackmap_proc.SetListOut(list);
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
		DeleteSelection(m_active_list->m_type);
		break;
	}
}

void TrackDlg::OnKeyDown(wxKeyEvent& event)
{
	if (!m_active_list)
		return;

	if (event.GetKeyCode() == WXK_DELETE ||
		event.GetKeyCode() == WXK_BACK)
		DeleteSelection(m_active_list->m_type);
	if (event.GetKeyCode() == wxKeyCode('C') &&
		wxGetKeyState(WXK_CONTROL))
		m_active_list->CopySelection();
}

