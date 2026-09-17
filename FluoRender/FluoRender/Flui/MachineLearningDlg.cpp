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
#include <MachineLearningDlg.h>
#include <MachineLearningDlgAgent.h>
#include <GridHelper.h>

MachineLearningDlg::MachineLearningDlg(wxWindow* parent) :
	TabbedPanel(parent,
		wxDefaultPosition,
		parent->FromDIP(wxSize(500, 620)),
		0, "MachineLearningDlg")
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
	MLVolPropPanel* panel1 = new MLVolPropPanel(m_notebook);
	MLCompGenPanel* panel2 = new MLCompGenPanel(m_notebook);
	m_notebook->AddPage(panel1, "Volume Properties", true);
	m_notebook->AddPage(panel2, "Component Generator");
	m_panels.push_back(panel1);
	m_panels.push_back(panel2);

	//interface
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
	sizer_v->Add(m_notebook, 1, wxEXPAND | wxALL);

	SetSizer(sizer_v);
	Layout();
	SetAutoLayout(true);
	SetScrollRate(10, 10);
	Thaw();
}

MachineLearningDlg::~MachineLearningDlg()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
MachineLearningPanel::MachineLearningPanel(wxWindow* parent) :
	PropPanel(parent,
		wxDefaultPosition,
		parent->FromDIP(wxSize(500, 620)),
		0, "MachineLearningPanel")
{
}

MachineLearningPanel::~MachineLearningPanel()
{
}

void MachineLearningPanel::Create()
{
	auto agent = m_agent->As<MachineLearningPanelAgent>();
	if (!agent)
		return;

	// temporarily block events during constructor:
	wxEventBlocker blocker(this);
	SetDoubleBuffered(true);
	wxStaticText* st = 0;

	wxBoxSizer* mainsizer = new wxBoxSizer(wxHORIZONTAL);
	m_splitter = new wxSplitterWindow(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxSP_THIN_SASH | wxSP_BORDER | wxSP_LIVE_UPDATE);
	m_splitter->SetMinimumPaneSize(160);
	mainsizer->Add(m_splitter, 1, wxBOTTOM | wxLEFT | wxEXPAND, 5);

	m_panel_top = new wxPanel(m_splitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER);
	wxBoxSizer* sizerTop = new wxBoxSizer(wxVERTICAL);
	st = new wxStaticText(m_panel_top, wxID_ANY, agent->GetTopGridName(),
		wxDefaultPosition, wxDefaultSize);
	m_top_grid = new wxGrid(m_panel_top, wxID_ANY);
	m_top_grid->CreateGrid(1, 5);
	m_top_grid->SetColLabelValue(0, "Name");
	m_top_grid->SetColLabelValue(1, "Records");
	m_top_grid->SetColLabelValue(2, "Notes");
	m_top_grid->SetColLabelValue(3, "Date modified");
	m_top_grid->SetColLabelValue(4, "Date created");
	m_top_grid->Bind(wxEVT_GRID_CELL_CHANGING, &MachineLearningPanel::OnTopGridCellChanging, this);
	m_top_grid->Bind(wxEVT_GRID_CELL_CHANGED, &MachineLearningPanel::OnTopGridCellChanged, this);
	//m_top_grid->Fit();
	wxBoxSizer* sizer1 = new wxBoxSizer(wxHORIZONTAL);
	m_new_table_btn = new wxButton(m_panel_top, wxID_ANY, "New",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_load_table_btn = new wxButton(m_panel_top, wxID_ANY, "Load",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_del_table_btn = new wxButton(m_panel_top, wxID_ANY, "Delete",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_dup_table_btn = new wxButton(m_panel_top, wxID_ANY, "Duplicate",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_auto_load_btn = new wxButton(m_panel_top, wxID_ANY, "Auto Load",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_new_table_btn->Bind(wxEVT_BUTTON, &MachineLearningPanel::OnNewTable, this);
	m_load_table_btn->Bind(wxEVT_BUTTON, &MachineLearningPanel::OnLoadTable, this);
	m_del_table_btn->Bind(wxEVT_BUTTON, &MachineLearningPanel::OnDelTable, this);
	m_dup_table_btn->Bind(wxEVT_BUTTON, &MachineLearningPanel::OnDupTable, this);
	m_auto_load_btn->Bind(wxEVT_BUTTON, &MachineLearningPanel::OnAutoLoad, this);
	sizer1->Add(5, 5);
	sizer1->Add(m_new_table_btn, 1, wxEXPAND);
	sizer1->Add(m_load_table_btn, 1, wxEXPAND);
	sizer1->Add(m_del_table_btn, 1, wxEXPAND);
	sizer1->Add(m_dup_table_btn, 1, wxEXPAND);
	sizer1->Add(m_auto_load_btn, 1, wxEXPAND);
	sizer1->Add(5, 5);
	//
	sizerTop->Add(10, 10);
	sizerTop->Add(st, 0, wxALIGN_CENTER);
	sizerTop->Add(10, 10);
	sizerTop->Add(sizer1, 0, wxEXPAND);
	sizerTop->Add(10, 10);
	sizerTop->Add(m_top_grid, 1, wxEXPAND);
	sizerTop->Add(10, 10);
	m_panel_top->SetSizer(sizerTop);

	m_panel_bot = new wxPanel(m_splitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER);
	wxBoxSizer* sizerBot = new wxBoxSizer(wxVERTICAL);
	st = new wxStaticText(m_panel_bot, wxID_ANY, agent->GetBotGridName(),
		wxDefaultPosition, wxDefaultSize);
	m_bot_grid = new wxGrid(m_panel_bot, wxID_ANY);
	m_bot_grid->CreateGrid(1, 2);
	m_bot_grid->SetColLabelValue(0, "Features");
	m_bot_grid->SetColLabelValue(1, "Parameters");
	m_bot_grid->Bind(wxEVT_GRID_COL_AUTO_SIZE, &MachineLearningPanel::OnBotGridAutoSize, this);
	m_bot_grid->Bind(wxEVT_GRID_CELL_CHANGING, &MachineLearningPanel::OnBotGridCellChanging, this);
	m_bot_grid->Bind(wxEVT_GRID_CELL_CHANGED, &MachineLearningPanel::OnBotGridCellChanged, this);
	//m_bot_grid->Fit();
	m_sizer2 = new wxBoxSizer(wxHORIZONTAL);
	m_bot_table_name = new wxStaticText(m_panel_bot, wxID_ANY, "No table loaded");
	m_auto_start_check = new wxCheckBox(m_panel_bot, wxID_ANY, "Auto Start",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_auto_start_check->Bind(wxEVT_CHECKBOX, &MachineLearningPanel::OnAutoStartRec, this);
	m_start_rec_btn = new wxToggleButton(m_panel_bot, wxID_ANY, "Start",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_start_rec_btn->Bind(wxEVT_TOGGLEBUTTON, &MachineLearningPanel::OnStartRec, this);
	//if (m_record)
	//{
	//	m_start_rec_btn->SetLabel("Started");
	//	m_start_rec_btn->SetValue(true);
	//}
	//else
	//{
	//	m_start_rec_btn->SetLabel("Start");
	//	m_start_rec_btn->SetValue(false);
	//}
	m_del_rec_btn = new wxButton(m_panel_bot, wxID_ANY, "Delete",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_del_rec_btn->Bind(wxEVT_BUTTON, &MachineLearningPanel::OnDelRec, this);
	m_apply_rec_btn = new wxButton(m_panel_bot, wxID_ANY, "Apply",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_apply_rec_btn->Bind(wxEVT_BUTTON, &MachineLearningPanel::OnApplyRec, this);
	m_sizer2->Add(5, 5);
	m_sizer2->Add(m_bot_table_name, 0, wxALIGN_CENTER);
	m_sizer2->AddStretchSpacer(1);
	m_sizer2->Add(m_auto_start_check, 0, wxALIGN_CENTER);
	m_sizer2->Add(m_start_rec_btn, 0);
	m_sizer2->Add(m_del_rec_btn, 0);
	m_sizer2->Add(m_apply_rec_btn, 0);
	m_sizer2->Add(5, 5);
	//
	wxBoxSizer* sizer3 = new wxBoxSizer(wxHORIZONTAL);
	m_start_prompt_text = new wxStaticText(m_panel_bot, wxID_ANY, "Click Start to begin/stop learning");
	sizer3->AddStretchSpacer(1);
	sizer3->Add(m_start_prompt_text, 0, wxALIGN_CENTER);
	sizer3->Add(5, 5);
	//
	sizerBot->Add(10, 10);
	sizerBot->Add(st, 0, wxALIGN_CENTER);
	sizerBot->Add(10, 10);
	sizerBot->Add(m_sizer2, 0, wxEXPAND);
	sizerBot->Add(10, 10);
	sizerBot->Add(sizer3, 0, wxEXPAND);
	sizerBot->Add(10, 10);
	sizerBot->Add(m_bot_grid, 1, wxEXPAND);
	sizerBot->Add(10, 10);
	m_panel_bot->SetSizer(sizerBot);

	m_splitter->SetSashGravity(0.0);
	m_splitter->SplitHorizontally(m_panel_top, m_panel_bot, 100);

	SetSizer(mainsizer);
	m_panel_top->Layout();
	m_panel_bot->Layout();
}

void MachineLearningPanel::PopTopList(const GridData& data)
{
	int row = m_top_grid->GetNumberRows();
	if (row)
		m_top_grid->DeleteRows(0, row, true);

	GridHelper::Populate(
		m_top_grid,
		data);

	m_top_grid->AutoSizeColumns();
	m_top_grid->ClearSelection();
	row = m_top_grid->GetNumberRows();
	int w, h, x, y;
	int s0 = m_top_grid->GetColLabelSize();
	int s1 = row > 0 ? m_top_grid->GetRowSize(0) : 0;
	m_top_grid->GetPosition(&x, &y);
	y += s0 + row * s1 + 50;
	GetSize(&w, &h);
	y = std::min(y, h / 2);
	m_splitter->SetSashPosition(y);
}

void MachineLearningPanel::UpdateTopListRow(
	const GridRowData& row_data)
{
	if (row_data.cells.empty())
		return;

	wxString name(row_data.cells[0].text);

	for (int row = 0; row < m_top_grid->GetNumberRows(); ++row)
	{
		if (m_top_grid->GetCellValue(row, 0) == name)
		{
			GridHelper::UpdateRow(
				m_top_grid,
				row,
				row_data);
			return;
		}
	}
}

void MachineLearningPanel::PopBotList(const GridData& data)
{
	int row = m_bot_grid->GetNumberRows();
	if (row)
		m_bot_grid->DeleteRows(0, row, true);

	GridHelper::Populate(
		m_bot_grid,
		data);

	EvenSizeBotGrid();
	m_bot_grid->ClearSelection();
	Layout();
}

void MachineLearningPanel::UpdateStartRecording(bool bval)
{
	if (bval)
	{
		m_start_rec_btn->SetLabel("Started");
		m_start_rec_btn->SetValue(true);
	}
	else
	{
		m_start_rec_btn->SetLabel("Start");
		m_start_rec_btn->SetValue(false);
	}
}

void MachineLearningPanel::EvenSizeBotGrid()
{
	int s0 = m_bot_grid->GetRowLabelSize();
	int w, h;
	m_bot_grid->GetSize(&w, &h);
	int colw = (w - s0) / 2;
	if (colw > 0)
	{
		m_bot_grid->SetColSize(0, colw);
		m_bot_grid->SetColMinimalWidth(0, colw);
		m_bot_grid->SetColSize(1, colw);
		m_bot_grid->SetColMinimalWidth(1, colw);
	}
}

void MachineLearningPanel::OnTopGridCellChanging(wxGridEvent& event)
{
	int c = event.GetCol();
	if (c != 0 && c != 2)
		event.Veto();
}

void MachineLearningPanel::OnBotGridCellChanging(wxGridEvent& event)
{
	event.Veto();
}

void MachineLearningPanel::OnTopGridCellChanged(wxGridEvent& event)
{
}

void MachineLearningPanel::OnBotGridCellChanged(wxGridEvent& event)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
MLCompGenPanel::MLCompGenPanel(wxWindow* parent) :
	MachineLearningPanel(parent)
{
	Create();
}

MLCompGenPanel::~MLCompGenPanel()
{
}

void MLCompGenPanel::UpdateAutoStart(bool bval)
{
	m_auto_start_check->SetValue(bval);
}

void MLCompGenPanel::OnNewTable(wxCommandEvent& event)
{
	m_top_grid->InsertRows(0);
}

void MLCompGenPanel::OnLoadTable(wxCommandEvent& event)
{
	wxArrayInt seli = m_top_grid->GetSelectedRows();
	if (seli.GetCount() > 0)
	{
		std::wstring name = m_top_grid->GetCellValue(seli[0], 0).ToStdWstring();
		auto agent = m_agent->As<MLCompGenPanelAgent>();
		if (agent)
		{
			agent->LoadTable(name);
			agent->UpdateDataToUI({ gstMlBotList });
		}
	}
}

void MLCompGenPanel::OnDelTable(wxCommandEvent& event)
{
	auto agent = m_agent->As<MLCompGenPanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstCompGenDelTable });
}

void MLCompGenPanel::OnDupTable(wxCommandEvent& event)
{
	auto agent = m_agent->As<MLCompGenPanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstCompGenDupTable });
}

void MLCompGenPanel::OnAutoLoad(wxCommandEvent& event)
{
	auto agent = m_agent->As<MLCompGenPanelAgent>();
	if (!agent)
		return;
	wxArrayInt seli = m_top_grid->GetSelectedRows();
	if (seli.GetCount() > 0)
	{
		std::wstring name = m_top_grid->GetCellValue(seli[0], 0).ToStdWstring();
		agent->SetTable(name);
	}
}

void MLCompGenPanel::OnAutoStartRec(wxCommandEvent& event)
{
	bool bval = m_auto_start_check->GetValue();
	auto agent = m_agent->As<MLCompGenPanelAgent>();
	if (agent)
		agent->SetAutoStart(bval);
}

void MLCompGenPanel::OnStartRec(wxCommandEvent& event)
{
	auto agent = m_agent->As<MLCompGenPanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstCompGenStartRec });
}

void MLCompGenPanel::OnDelRec(wxCommandEvent& event)
{
	auto agent = m_agent->As<MLCompGenPanelAgent>();
	if (agent)
		agent->DeleteRecord(
			GridHelper::GetSelection(m_bot_grid));
}

void MLCompGenPanel::OnApplyRec(wxCommandEvent& event)
{
	auto agent = m_agent->As<MLCompGenPanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstCompGenApplyRecord });
}

void MLCompGenPanel::OnBotGridAutoSize(wxGridSizeEvent& event)
{
	EvenSizeBotGrid();
}

void MLCompGenPanel::OnTopGridCellChanged(wxGridEvent& event)
{
	auto agent = m_agent->As<MLCompGenPanelAgent>();
	if (!agent)
		return;

	GridCellChanged info;
	info.col = event.GetCol();
	info.row = event.GetRow();
	info.old_value = event.GetString();
	info.new_value = m_top_grid->GetCellValue(info.row, info.col).ToStdWstring();
	info.index_value = m_top_grid->GetCellValue(info.row, 0).ToStdWstring();
	agent->UpdateCellChanged(info);
	if (info.col == 0)
		m_top_grid->ClearSelection();

	event.Skip();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
MLVolPropPanel::MLVolPropPanel(wxWindow* parent) :
	MachineLearningPanel(parent)
{
	Create();

	//add more options
	m_auto_apply_chk = new wxCheckBox(m_panel_bot, wxID_ANY, "Auto Apply",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_auto_apply_chk->Bind(wxEVT_CHECKBOX, &MLVolPropPanel::OnAutoApply, this);
	m_sizer2->Add(m_auto_apply_chk, 0);
	m_sizer2->Add(5, 5);
	m_panel_bot->Layout();
}

MLVolPropPanel::~MLVolPropPanel()
{
}

void MLVolPropPanel::UpdateAutoStart(bool bval)
{
	m_auto_start_check->SetValue(bval);
}

void MLVolPropPanel::OnNewTable(wxCommandEvent& event)
{
	m_top_grid->InsertRows(0);
}

void MLVolPropPanel::OnLoadTable(wxCommandEvent& event)
{
	wxArrayInt seli = m_top_grid->GetSelectedRows();
	if (seli.GetCount() > 0)
	{
		std::wstring name = m_top_grid->GetCellValue(seli[0], 0).ToStdWstring();
		auto agent = m_agent->As<MLVolPropPanelAgent>();
		if (agent)
		{
			agent->LoadTable(name);
			agent->UpdateDataToUI({ gstMlBotList });
		}
	}
}

void MLVolPropPanel::OnDelTable(wxCommandEvent& event)
{
	auto agent = m_agent->As<MLVolPropPanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstVolPropDelTable });
}

void MLVolPropPanel::OnDupTable(wxCommandEvent& event)
{
	auto agent = m_agent->As<MLVolPropPanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstVolPropDupTable });
}

void MLVolPropPanel::OnAutoLoad(wxCommandEvent& event)
{
	auto agent = m_agent->As<MLVolPropPanelAgent>();
	if (!agent)
		return;
	wxArrayInt seli = m_top_grid->GetSelectedRows();
	if (seli.GetCount() > 0)
	{
		std::wstring name = m_top_grid->GetCellValue(seli[0], 0).ToStdWstring();
		agent->SetTable(name);
	}
}

void MLVolPropPanel::OnAutoStartRec(wxCommandEvent& event)
{
	bool bval = m_auto_start_check->GetValue();
	auto agent = m_agent->As<MLVolPropPanelAgent>();
	if (agent)
		agent->SetAutoStart(bval);
}

void MLVolPropPanel::OnStartRec(wxCommandEvent& event)
{
	auto agent = m_agent->As<MLVolPropPanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstVolPropStartRec });
}

void MLVolPropPanel::OnDelRec(wxCommandEvent& event)
{
	auto agent = m_agent->As<MLVolPropPanelAgent>();
	if (agent)
		agent->DeleteRecord(
			GridHelper::GetSelection(m_bot_grid));
}

void MLVolPropPanel::OnApplyRec(wxCommandEvent& event)
{
	auto agent = m_agent->As<MLCompGenPanelAgent>();
	if (agent)
		agent->UpdateUIToData({ gstVolPropApplyRecord });
}

void MLVolPropPanel::OnBotGridAutoSize(wxGridSizeEvent& event)
{
	EvenSizeBotGrid();
}

void MLVolPropPanel::OnTopGridCellChanged(wxGridEvent& event)
{
	auto agent = m_agent->As<MLVolPropPanelAgent>();
	if (!agent)
		return;

	GridCellChanged info;
	info.col = event.GetCol();
	info.row = event.GetRow();
	info.old_value = event.GetString();
	info.new_value = m_top_grid->GetCellValue(info.row, info.col).ToStdWstring();
	info.index_value = m_top_grid->GetCellValue(info.row, 0).ToStdWstring();
	agent->UpdateCellChanged(info);
	if (info.col == 0)
		m_top_grid->ClearSelection();

	event.Skip();
}

void MLVolPropPanel::OnAutoApply(wxCommandEvent& event)
{
	bool bval = m_auto_apply_chk->GetValue();
	auto agent = m_agent->As<MLVolPropPanelAgent>();
	if (agent)
		agent->SetAutoApply(bval);
}

void MLVolPropPanel::UpdateAutoApply(bool bval)
{
	m_auto_apply_chk->SetValue(bval);
}

