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
#include <CountingDlg.h>
#include <CountingDlgAgent.h>
#include <GridHelper.h>
#include <wx/valnum.h>
#include <wx/clipbrd.h>

CountingDlg::CountingDlg(wxWindow *parent) :
	TabbedPanel(parent,
		wxDefaultPosition,
		parent->FromDIP(wxSize(400, 150)),
		0, "CountingDlg")
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
	m_notebook->AddPage(CreateSettingPage(m_notebook), "Count Volume Voxels", true);
	m_notebook->AddPage(CreateInfoPage(m_notebook), "Information");

	Bind(wxEVT_SIZE, &CountingDlg::OnSize, this);

	//vertical sizer
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
	sizer_v->Add(m_notebook, 1, wxEXPAND | wxALL);

	SetSizer(sizer_v);
	Layout();
	SetAutoLayout(true);
	SetScrollRate(10, 10);
	Thaw();
}

CountingDlg::~CountingDlg()
{
}

wxWindow* CountingDlg::CreateSettingPage(wxWindow* parent)
{
	wxScrolledWindow* page = new wxScrolledWindow(parent);

	//validator: floating point 1
	wxFloatingPointValidator<double> vald_fp1(1);
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;

	wxStaticText* st = 0;

	//component analyzer
	//size of ccl
	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	m_ca_select_only_chk = new wxCheckBox(this, wxID_ANY, "Selct. Only",
		wxDefaultPosition, FromDIP(wxSize(95, 20)));
	m_ca_select_only_chk->Bind(wxEVT_CHECKBOX, &CountingDlg::OnUseSelChk, this);
	sizer_1->Add(m_ca_select_only_chk, 0, wxALIGN_CENTER);
	sizer_1->AddStretchSpacer();
	st = new wxStaticText(this, 0, "Min:",
		wxDefaultPosition, FromDIP(wxSize(35, 15)));
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	m_ca_min_text = new wxTextCtrl(this, wxID_ANY, "0",
		wxDefaultPosition, FromDIP(wxSize(40, 20)), wxTE_RIGHT, vald_int);
	m_ca_min_text->Bind(wxEVT_TEXT, &CountingDlg::OnMinText, this);
	sizer_1->Add(m_ca_min_text, 0, wxALIGN_CENTER);
	st = new wxStaticText(this, 0, "vx",
		wxDefaultPosition, FromDIP(wxSize(15, 15)));
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	sizer_1->AddStretchSpacer();
	st = new wxStaticText(this, 0, "Max:",
		wxDefaultPosition, FromDIP(wxSize(35, 15)));
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	m_ca_max_text = new wxTextCtrl(this, wxID_ANY, "1000",
		wxDefaultPosition, FromDIP(wxSize(40, 20)), wxTE_RIGHT, vald_int);
	m_ca_max_text->Bind(wxEVT_TEXT, &CountingDlg::OnMaxText, this);
	sizer_1->Add(m_ca_max_text, 0, wxALIGN_CENTER);
	st = new wxStaticText(this, 0, "vx",
		wxDefaultPosition, FromDIP(wxSize(15, 15)));
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	sizer_1->AddStretchSpacer();
	m_ca_ignore_max_chk = new wxCheckBox(this, wxID_ANY, "Ignore Max");
	m_ca_ignore_max_chk->Bind(wxEVT_CHECKBOX, &CountingDlg::OnIgnoreMaxChk, this);
	sizer_1->Add(m_ca_ignore_max_chk, 0, wxALIGN_CENTER);
	//export
	wxBoxSizer* sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	sizer_2->AddStretchSpacer();
	m_ca_analyze_btn = new wxButton(this, wxID_ANY, "Analyze",
		wxDefaultPosition, FromDIP(wxSize(-1, 23)));
	m_ca_analyze_btn->Bind(wxEVT_BUTTON, &CountingDlg::OnAnalyzeBtn, this);
	sizer_2->Add(m_ca_analyze_btn, 0, wxALIGN_CENTER);

	//all controls
	wxBoxSizer* sizerV = new wxBoxSizer(wxVERTICAL);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_1, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_2, 0, wxEXPAND);
	sizerV->Add(10, 10);

	page->SetSizer(sizerV);
	page->SetAutoLayout(true);
	page->SetScrollRate(10, 10);
	return page;
}

wxWindow* CountingDlg::CreateInfoPage(wxWindow* parent)
{
	wxScrolledWindow* page = new wxScrolledWindow(parent);

	//output
	wxBoxSizer* sizer1 = new wxBoxSizer(wxHORIZONTAL);
	m_history_chk = new wxCheckBox(page, wxID_ANY,
		"Hold History", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_history_chk->Bind(wxEVT_CHECKBOX, &CountingDlg::OnHistoryChk, this);
	m_clear_hist_btn = new wxButton(page, wxID_ANY,
		"Clear History", wxDefaultPosition, wxDefaultSize);
	m_clear_hist_btn->Bind(wxEVT_BUTTON, &CountingDlg::OnClearHistBtn, this);
	sizer1->AddStretchSpacer(1);
	sizer1->Add(m_history_chk, 0, wxALIGN_CENTER);
	sizer1->Add(5, 5);
	sizer1->Add(m_clear_hist_btn, 0, wxALIGN_CENTER);
	//grid
	m_output_grid = new wxGrid(page, wxID_ANY);
	m_output_grid->CreateGrid(0, 5);
	m_output_grid->SetColLabelValue(0, "Surface Area");
	m_output_grid->SetColLabelValue(1, "Volume");
	m_output_grid->SetColLabelValue(2, "Vertex Count");
	m_output_grid->SetColLabelValue(3, "Triangle Count");
	m_output_grid->SetColLabelValue(4, "Normal Count");
	//m_output_grid->Fit();
	m_output_grid->Bind(wxEVT_GRID_SELECT_CELL, &CountingDlg::OnSelectCell, this);
	m_output_grid->Bind(wxEVT_KEY_DOWN, &CountingDlg::OnKeyDown, this);

	//sizer
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
	sizer_v->Add(5, 5);
	sizer_v->Add(sizer1, 0, wxEXPAND);
	sizer_v->Add(5, 5);
	sizer_v->Add(m_output_grid, 1, wxEXPAND);
	sizer_v->Add(5, 5);

	page->SetSizer(sizer_v);
	page->SetAutoLayout(true);
	page->SetScrollRate(10, 10);
	return page;
}

void CountingDlg::UpdateUseSelection(bool bval)
{
	m_ca_select_only_chk->SetValue(bval);
}

void CountingDlg::UpdateCountMinValue(int ival)
{
	auto str = wxString::Format("%d", ival);
	m_ca_min_text->ChangeValue(str);
}

void CountingDlg::UpdateCountMaxValue(int ival)
{
	auto str = wxString::Format("%d", ival);
	m_ca_max_text->ChangeValue(str);
}

void CountingDlg::UpdateCountUseMax(bool bval)
{
	m_ca_ignore_max_chk->SetValue(bval);
	m_ca_max_text->Enable(bval);
}

void CountingDlg::CopyData()
{
	auto text =
		GridHelper::CopySelection(
			m_output_grid);

	if (text.empty())
		return;

	if (wxTheClipboard->Open())
	{
		wxTheClipboard->SetData(
			new wxTextDataObject(text));

		wxTheClipboard->Close();
	}
}

void CountingDlg::UpdateGrid(const GridData& data)
{
	GridPopulateOptions options;
	options.append_rows = false;
	options.remove_extra_rows = !m_hold_history;
	options.remove_extra_cols = !m_hold_history;

	GridHelper::Populate(
		m_output_grid,
		data,
		options);

	m_output_grid->ClearSelection();
}

void CountingDlg::OnUseSelChk(wxCommandEvent& event)
{
	bool bval = m_ca_select_only_chk->GetValue();

	auto agent = m_agent->As<CountingDlgAgent>();
	if (agent)
		agent->SetUseSelection(bval);
}

void CountingDlg::OnMinText(wxCommandEvent& event)
{
	long ival;
	wxString str = m_ca_min_text->GetValue();
	if (str.ToLong(&ival))
	{
		auto agent = m_agent->As<CountingDlgAgent>();
		if (agent)
			agent->SetMinNum(ival);
	}
}

void CountingDlg::OnMaxText(wxCommandEvent& event)
{
	long ival;
	wxString str = m_ca_max_text->GetValue();
	if (str.ToLong(&ival))
	{
		auto agent = m_agent->As<CountingDlgAgent>();
		if (agent)
			agent->SetMaxNum(ival);
	}
}

void CountingDlg::OnIgnoreMaxChk(wxCommandEvent& event)
{
	bool bval = m_ca_ignore_max_chk->GetValue();
	auto agent = m_agent->As<CountingDlgAgent>();
	if (agent)
		agent->SetUseMax(!bval);
}

//component analyze
void CountingDlg::OnAnalyzeBtn(wxCommandEvent& event)
{
	auto agent = m_agent->As<CountingDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstCountAnalyze });
}

void CountingDlg::OnHistoryChk(wxCommandEvent& event)
{
	m_hold_history = m_history_chk->GetValue();
}

void CountingDlg::OnClearHistBtn(wxCommandEvent& event)
{
	m_output_grid->DeleteRows(0, m_output_grid->GetNumberRows());
}

void CountingDlg::OnKeyDown(wxKeyEvent& event)
{
	if (wxGetKeyState(WXK_CONTROL))
	{
		if (event.GetKeyCode() == wxKeyCode('C'))
			CopyData();
		//else if (event.GetKeyCode() == wxKeyCode('V'))
		//	PasteData();
	}
}

void CountingDlg::OnSelectCell(wxGridEvent& event)
{
	int r = event.GetRow();
	int c = event.GetCol();
	m_output_grid->SelectBlock(r, c, r, c);
}

void CountingDlg::OnSize(wxSizeEvent& event)
{
	if (!m_output_grid)
		return;

	wxSize size = GetSize();
	wxPoint p1 = GetScreenPosition();
	wxPoint p2 = m_output_grid->GetScreenPosition();
	int height, margin;
	if (m_output_grid->GetNumberRows())
		height = m_output_grid->GetRowSize(0) * 8;
	else
		height = 80;
	margin = size.y + p1.y - p2.y - 20;
	if (margin > height)
		size.y = margin;
	else
		size.y = height;
	size.x -= 15;
	m_output_grid->SetMaxSize(size);
}

