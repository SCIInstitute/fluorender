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
#include "MachineLearningDlg.h"
#include "VRenderFrame.h"
#include <Global.h>
#include <EntryHist.h>
#include <EntryParams.h>
#include <wx/stdpaths.h>
#include <format>

BEGIN_EVENT_TABLE(MachineLearningDlg, wxPanel)
END_EVENT_TABLE()

MachineLearningDlg::MachineLearningDlg(VRenderFrame *frame) :
	wxPanel(frame, wxID_ANY,
		wxDefaultPosition, wxSize(450, 750),
		0, "SettingDlg"),
	m_frame(frame)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);
	SetDoubleBuffered(true);

	//notebook
	wxNotebook *notebook = new wxNotebook(this, wxID_ANY);
	MLCompGenPanel* panel1 = new MLCompGenPanel(frame, notebook);
	notebook->AddPage(panel1, "Component Generator");

	//interface
	wxBoxSizer *sizerV = new wxBoxSizer(wxVERTICAL);
	sizerV->Add(notebook, 1, wxEXPAND);
	SetSizerAndFit(sizerV);
	Layout();

	//GetSettings();
}

MachineLearningDlg::~MachineLearningDlg()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
MachineLearningPanel::MachineLearningPanel(
	VRenderFrame* frame, wxWindow* parent) :
	wxPanel(parent, wxID_ANY,
		wxDefaultPosition, wxSize(450, 750),
		0, "MachineLearningPanel"),
	m_frame(frame),
	m_record(false)
{
	m_exepath = wxStandardPaths::Get().GetExecutablePath();
	m_exepath = wxPathOnly(m_exepath);
}

MachineLearningPanel::~MachineLearningPanel()
{
}

void MachineLearningPanel::Create()
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);
	SetDoubleBuffered(true);
	wxStaticText* st = 0;

	wxBoxSizer *mainsizer = new wxBoxSizer(wxHORIZONTAL);
	wxSplitterWindow *splittermain = new wxSplitterWindow(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxSP_THIN_SASH | wxSP_BORDER | wxSP_LIVE_UPDATE);
	splittermain->SetMinimumPaneSize(160);
	mainsizer->Add(splittermain, 1, wxBOTTOM | wxLEFT | wxEXPAND, 5);

	wxPanel *panel_top = new wxPanel(splittermain, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER);
	wxBoxSizer* sizerTop = new wxBoxSizer(wxVERTICAL);
	st = new wxStaticText(panel_top, wxID_ANY, m_top_grid_name,
		wxDefaultPosition, wxDefaultSize);
	m_top_grid = new wxGrid(panel_top, m_top_grid_id);
	m_top_grid->CreateGrid(1, 5);
	m_top_grid->SetColLabelValue(0, "Name");
	m_top_grid->SetColLabelValue(1, "Records");
	m_top_grid->SetColLabelValue(2, "Notes");
	m_top_grid->SetColLabelValue(3, "Date modified");
	m_top_grid->SetColLabelValue(4, "Date created");
	m_top_grid->Fit();
	wxBoxSizer* sizer1 = new wxBoxSizer(wxHORIZONTAL);
	m_new_table_btn = new wxButton(panel_top, m_new_table_id, "New",
		wxDefaultPosition, wxSize(75, -1), wxALIGN_LEFT);
	m_load_table_btn = new wxButton(panel_top, m_load_table_id, "Load",
		wxDefaultPosition, wxSize(75, -1), wxALIGN_LEFT);
	m_del_table_btn = new wxButton(panel_top, m_del_table_id, "Delete",
		wxDefaultPosition, wxSize(75, -1), wxALIGN_LEFT);
	m_dup_table_btn = new wxButton(panel_top, m_dup_table_id, "Duplicate",
		wxDefaultPosition, wxSize(75, -1), wxALIGN_LEFT);
	m_new_table_btn->Connect(m_new_table_id, wxEVT_BUTTON,
		wxCommandEventHandler(MachineLearningPanel::OnNewTable), NULL, this);
	m_load_table_btn->Connect(m_load_table_id, wxEVT_BUTTON,
		wxCommandEventHandler(MachineLearningPanel::OnLoadTable), NULL, this);
	m_del_table_btn->Connect(m_del_table_id, wxEVT_BUTTON,
		wxCommandEventHandler(MachineLearningPanel::OnDelTable), NULL, this);
	m_dup_table_btn->Connect(m_dup_table_id, wxEVT_BUTTON,
		wxCommandEventHandler(MachineLearningPanel::OnDupTable), NULL, this);
	sizer1->Add(5, 5);
	sizer1->Add(m_new_table_btn, 1, wxEXPAND);
	sizer1->Add(m_load_table_btn, 1, wxEXPAND);
	sizer1->Add(m_del_table_btn, 1, wxEXPAND);
	sizer1->Add(m_dup_table_btn, 1, wxEXPAND);
	sizer1->Add(5, 5);
	//
	sizerTop->Add(10, 10);
	sizerTop->Add(st, 0, wxALIGN_CENTER);
	sizerTop->Add(10, 10);
	sizerTop->Add(sizer1, 0, wxEXPAND);
	sizerTop->Add(10, 10);
	sizerTop->Add(m_top_grid, 1, wxEXPAND);
	sizerTop->Add(10, 10);
	panel_top->SetSizer(sizerTop);

	wxPanel *panel_bot = new wxPanel(splittermain, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER);
	wxBoxSizer* sizerBot = new wxBoxSizer(wxVERTICAL);
	st = new wxStaticText(panel_bot, wxID_ANY, m_bot_grid_name,
		wxDefaultPosition, wxDefaultSize);
	m_bot_grid = new wxGrid(panel_bot, m_bot_grid_id);
	m_bot_grid->CreateGrid(1, 2);
	m_bot_grid->SetColLabelValue(0, "Input");
	m_bot_grid->SetColLabelValue(1, "Output");
	m_bot_grid->Fit();
	wxBoxSizer* sizer2 = new wxBoxSizer(wxHORIZONTAL);
	m_start_rec_btn = new wxToggleButton(panel_bot, m_start_rec_id, "Start",
		wxDefaultPosition, wxSize(75, -1), wxALIGN_LEFT);
	m_start_rec_btn->Connect(m_start_rec_id, wxEVT_TOGGLEBUTTON,
		wxCommandEventHandler(MachineLearningPanel::OnStartRec), NULL, this);
	if (m_record)
	{
		m_start_rec_btn->SetLabel("Started");
		m_start_rec_btn->SetValue(true);
	}
	else
	{
		m_start_rec_btn->SetLabel("Start");
		m_start_rec_btn->SetValue(false);
	}
	m_del_rec_btn = new wxButton(panel_bot, m_del_rec_id, "Delete",
		wxDefaultPosition, wxSize(75, -1), wxALIGN_LEFT);
	m_del_rec_btn->Connect(m_del_rec_id, wxEVT_BUTTON,
		wxCommandEventHandler(MachineLearningPanel::OnDelRec), NULL, this);
	sizer2->AddStretchSpacer(1);
	sizer2->Add(m_start_rec_btn, 0);
	sizer2->Add(m_del_rec_btn, 0);
	sizer2->Add(5, 5);
	//
	sizerBot->Add(10, 10);
	sizerBot->Add(st, 0, wxALIGN_CENTER);
	sizerBot->Add(10, 10);
	sizerBot->Add(sizer2, 0, wxEXPAND);
	sizerBot->Add(10, 10);
	sizerBot->Add(m_bot_grid, 1, wxEXPAND);
	sizerBot->Add(10, 10);
	panel_bot->SetSizer(sizerBot);

	splittermain->SetSashGravity(0.0);
	splittermain->SplitHorizontally(panel_top, panel_bot, 100);

	SetSizer(mainsizer);
	panel_top->Layout();
	panel_bot->Layout();
}

void MachineLearningPanel::PopTopList()
{
	int row = m_top_grid->GetNumberRows();
	m_top_grid->DeleteRows(0, row, true);

	wxString loc = m_exepath + GETSLASH() +
		m_dir + GETSLASH() +
		"*" + m_ext;
	wxLogNull logNo;
	wxArrayString list;
	wxString file = wxFindFirstFile(loc);
	while (!file.empty())
	{
		file = wxFileNameFromPath(file);
		file = file.BeforeLast('.');
		list.Add(file);
		file = wxFindNextFile();
	}
	list.Sort();
	std::string filename, name;
	for (size_t i = 0; i < list.GetCount(); ++i)
	{
		filename = m_exepath + GETSLASH() +
			m_dir + GETSLASH() +
			list[i].ToStdString() + m_ext;
		flrd::Table table;
		table.open(filename, true);
		m_top_grid->InsertRows(i);
		name = table.getName();
		if (name.empty()) name = list[i].ToStdString();
		m_top_grid->SetCellValue(i, 0, name);
		m_top_grid->SetCellValue(i, 1, std::to_string(table.getRecNum()));
		m_top_grid->SetCellValue(i, 2, table.getNotes());
		char b[32];
		std::tm* ptm;
		ptm = std::localtime(table.getCreateTime());
		std::strftime(b, 32, "%m/%d/%Y %H:%M:%S", ptm);
		m_top_grid->SetCellValue(i, 4, std::string(b));
		ptm = std::localtime(table.getModifyTime());
		std::strftime(b, 32, "%m/%d/%Y %H:%M:%S", ptm);
		m_top_grid->SetCellValue(i, 3, std::string(b));
	}
	m_top_grid->AutoSizeColumns();
	m_top_grid->ClearSelection();
}

void MachineLearningPanel::UpdateList(int index)
{
	if (index & 1)
		this->UpdateTopList();
	if (index & 2)
		this->UpdateBotList();
}

void MachineLearningPanel::UpdateTopList()
{
	flrd::TableHistParams& table = glbin.get_cg_table();
	std::string name = table.getName();
	for (int i = 0; i < m_top_grid->GetNumberRows(); ++i)
	{
		if (m_top_grid->GetCellValue(i, 0) == wxString(name))
		{
			m_top_grid->SetCellValue(i, 1, std::to_string(table.getRecNum()));
			m_top_grid->SetCellValue(i, 2, table.getNotes());
			char b[32];
			std::tm* ptm;
			ptm = std::localtime(table.getCreateTime());
			std::strftime(b, 32, "%m/%d/%Y %H:%M:%S", ptm);
			m_top_grid->SetCellValue(i, 4, std::string(b));
			ptm = std::localtime(table.getModifyTime());
			std::strftime(b, 32, "%m/%d/%Y %H:%M:%S", ptm);
			m_top_grid->SetCellValue(i, 3, std::string(b));
			break;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
MLCompGenPanel::MLCompGenPanel(
	VRenderFrame* frame, wxWindow* parent) :
	MachineLearningPanel(frame, parent)
{
	m_top_grid_name = "Data Sets";
	m_top_grid_id = ID_TopGrid;
	m_new_table_id = ID_NewTableBtn;
	m_load_table_id = ID_LoadTableBtn;
	m_del_table_id = ID_DelTableBtn;
	m_dup_table_id = ID_DupTableBtn;
	m_bot_grid_name = "History Records";
	m_bot_grid_id = ID_BotGrid;
	m_start_rec_id = ID_StartRecBtn;
	m_del_rec_id = ID_DelRecBtn;
	Create();
	PopTopList();

	flrd::TableHistParams& table = glbin.get_cg_table();
	table.setUpdateFunc(std::bind(
		&MLCompGenPanel::UpdateList, this, std::placeholders::_1));
}

MLCompGenPanel::~MLCompGenPanel()
{
	flrd::TableHistParams& table = glbin.get_cg_table();
	//save existing table if modified
	if (table.getModified())
	{
		wxString str = m_exepath + GETSLASH() +
			m_dir + GETSLASH();
		std::string name = str.ToStdString();
		name += table.getName() + m_ext.ToStdString();
		table.save(name);
	}
}

void MLCompGenPanel::OnNewTable(wxCommandEvent& event)
{
	wxMessageBox("cg new tbl");
}

void MLCompGenPanel::OnLoadTable(wxCommandEvent& event)
{
	wxArrayInt seli = m_top_grid->GetSelectedRows();
	if (seli.GetCount() > 0)
	{
		wxString name = m_top_grid->GetCellValue(seli[0], 0);
		LoadTable(name.ToStdString());
		UpdateBotList();
	}
}

void MLCompGenPanel::OnDelTable(wxCommandEvent& event)
{
	wxMessageBox("cg del tbl");
}

void MLCompGenPanel::OnDupTable(wxCommandEvent& event)
{
	flrd::TableHistParams table(glbin.get_cg_table());
	//save it
	wxString str = m_exepath + GETSLASH() +
		m_dir + GETSLASH() +
		table.getName() + m_ext;
	table.save(str.ToStdString());
	PopTopList();
}

void MLCompGenPanel::OnStartRec(wxCommandEvent& event)
{
	m_record = !m_record;
	if (m_record)
	{
		m_start_rec_btn->SetLabel("Started");
		m_start_rec_btn->SetValue(true);
		glbin.set_cg_table_enable(true);
	}
	else
	{
		m_start_rec_btn->SetLabel("Start");
		m_start_rec_btn->SetValue(false);
		glbin.set_cg_table_enable(false);
	}
}

void MLCompGenPanel::OnDelRec(wxCommandEvent& event)
{
	flrd::TableHistParams& table = glbin.get_cg_table();
	wxArrayInt seli = m_bot_grid->GetSelectedRows();
	std::vector<size_t> vi;
	size_t count = table.getRecSize();
	for (size_t i = 0; i < seli.GetCount(); ++i)
		vi.push_back(count - 1 - seli[i]);
	table.delRecords(vi);
}

void MLCompGenPanel::PopTopList()
{
	m_dir = "Database";
	m_ext = ".cgtbl";
	MachineLearningPanel::PopTopList();
}

void MLCompGenPanel::UpdateBotList()
{
	int row = m_bot_grid->GetNumberRows();
	m_bot_grid->DeleteRows(0, row, true);

	flrd::TableHistParams& table = glbin.get_cg_table();
	std::string str_in, str_out;
	std::vector<float> data_in, data_out;
	for (int i = 0; i < table.getRecSize(); ++i)
	{
		m_bot_grid->InsertRows(0);

		str_in.clear();
		table.getOneInput(i, data_in);
		size_t len = data_in.size();
		if (len)
		{
			for (size_t j = 0; j < data_in.size() - 1; ++j)
				str_in += std::format("{:.2f}", data_in[j]) + ", ";
			str_in += std::format("{:.2f}", data_in[len - 1]);
		}
		m_bot_grid->SetCellValue(0, 0, str_in);

		str_out.clear();
		table.getOneOutput(i, data_out);
		len = data_out.size();
		if (len)
		{
			for (size_t j = 0; j < data_out.size() - 1; ++j)
				str_out += std::format("{:.2f}", data_out[j]) + ", ";
			str_out += std::format("{:.2f}", data_out[len - 1]);
		}
		m_bot_grid->SetCellValue(0, 1, str_out);
	}
	m_bot_grid->AutoSizeColumns();
	m_bot_grid->ClearSelection();
}

void MLCompGenPanel::LoadTable(const std::string& filename)
{
	wxString str = m_exepath + GETSLASH() +
		m_dir + GETSLASH();
	flrd::TableHistParams& table = glbin.get_cg_table();
	//save existing table if modified
	if (table.getModified())
	{
		std::string name = str.ToStdString();
		name += table.getName() + m_ext.ToStdString();
		table.save(name);
	}
	str += filename + m_ext;
	table.open(str.ToStdString());
}

void MLCompGenPanel::SaveTable(const std::string& filename)
{
	wxString str = m_exepath + GETSLASH() +
		m_dir + GETSLASH() +
		filename + m_ext;
	glbin.get_cg_table().save(str.ToStdString());
}

