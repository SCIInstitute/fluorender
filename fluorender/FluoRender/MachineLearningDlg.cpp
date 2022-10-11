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
#include <filesystem>

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
	m_splitter = new wxSplitterWindow(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize, wxSP_THIN_SASH | wxSP_BORDER | wxSP_LIVE_UPDATE);
	m_splitter->SetMinimumPaneSize(160);
	mainsizer->Add(m_splitter, 1, wxBOTTOM | wxLEFT | wxEXPAND, 5);

	wxPanel *panel_top = new wxPanel(m_splitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER);
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
	m_top_grid->Connect(m_top_grid_id, wxEVT_GRID_CELL_CHANGING,
		wxGridEventHandler(MachineLearningPanel::OnTopGridCellChanging), NULL, this);
	m_top_grid->Connect(m_top_grid_id, wxEVT_GRID_CELL_CHANGED,
		wxGridEventHandler(MachineLearningPanel::OnTopGridCellChanged), NULL, this);
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

	wxPanel *panel_bot = new wxPanel(m_splitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER);
	wxBoxSizer* sizerBot = new wxBoxSizer(wxVERTICAL);
	st = new wxStaticText(panel_bot, wxID_ANY, m_bot_grid_name,
		wxDefaultPosition, wxDefaultSize);
	m_bot_grid = new wxGrid(panel_bot, m_bot_grid_id);
	m_bot_grid->CreateGrid(1, 2);
	m_bot_grid->SetColLabelValue(0, "Input");
	m_bot_grid->SetColLabelValue(1, "Output");
	m_bot_grid->Connect(m_bot_grid_id, wxEVT_GRID_COL_AUTO_SIZE,
		wxGridSizeEventHandler(MachineLearningPanel::OnBotGridAutoSize), NULL, this);
	m_bot_grid->Connect(m_bot_grid_id, wxEVT_GRID_CELL_CHANGING,
		wxGridEventHandler(MachineLearningPanel::OnBotGridCellChanging), NULL, this);
	m_bot_grid->Connect(m_bot_grid_id, wxEVT_GRID_CELL_CHANGED,
		wxGridEventHandler(MachineLearningPanel::OnBotGridCellChanged), NULL, this);
	m_bot_grid->Fit();
	wxBoxSizer* sizer2 = new wxBoxSizer(wxHORIZONTAL);
	m_bot_table_name = new wxStaticText(panel_bot, wxID_ANY, "No table loaded");
	m_start_prompt_text = new wxStaticText(panel_bot, wxID_ANY, "Click Start to begin/stop learning");
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
	sizer2->Add(10, 10);
	sizer2->Add(m_bot_table_name, 0, wxALIGN_CENTER);
	sizer2->AddStretchSpacer(1);
	sizer2->Add(m_start_prompt_text, 0, wxALIGN_CENTER);
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

	m_splitter->SetSashGravity(0.0);
	m_splitter->SplitHorizontally(panel_top, panel_bot, 100);

	SetSizer(mainsizer);
	panel_top->Layout();
	panel_bot->Layout();
}

void MachineLearningPanel::PopTopList()
{
	int row = m_top_grid->GetNumberRows();
	if (row)
		m_top_grid->DeleteRows(0, row, true);

	std::string path = m_exepath;
	path += GETSLASH() + m_dir;
	std::string name, ext, filename;
	int i = 0;
	for (const auto& entry : std::filesystem::directory_iterator(path))
	{
		ext = entry.path().extension().string();
		if (ext != m_ext)
			continue;
		filename = entry.path().string();
		flrd::Table table;
		table.open(filename, true);
		m_top_grid->InsertRows(i);
		name = table.getName();
		if (name.empty()) name = entry.path().stem().string();
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
		i++;
	}
	m_top_grid->AutoSizeColumns();
	m_top_grid->ClearSelection();
	row = m_top_grid->GetNumberRows();
	int w, h, x, y;
	int s0 = m_top_grid->GetColLabelSize();
	int s1 = row > 0 ? m_top_grid->GetRowSize(0) : 0;
	m_top_grid->GetPosition(&x, &y);
	y += s0 + row * s1 + 50;
	GetSize(&w, &h);
	y = std::min(y, h/2);
	m_splitter->SetSashPosition(y);
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

bool MachineLearningPanel::MatchTableName(std::string& name)
{
	bool modified = false;
	std::string path = m_exepath;
	path += GETSLASH() + m_dir;
	std::string stem, ext;
	while (true)
	{
		bool found_same = false;
		for (const auto& entry : std::filesystem::directory_iterator(path))
		{
			stem = entry.path().stem().string();
			ext = entry.path().extension().string();
			if (ext == m_ext && stem == name)
			{
				found_same = true;
				INC_NUMBER(name);
				modified = true;
			}
		}
		if (!found_same)
			break;
	}
	return modified;
}

void MachineLearningPanel::EvenSizeBotGrid()
{
	int s0 = m_bot_grid->GetRowLabelSize();
	int w, h;
	m_bot_grid->GetSize(&w, &h);
	int colw = (w - s0) / 2;
	m_bot_grid->SetColSize(0, colw);
	m_bot_grid->SetColMinimalWidth(0, colw);
	m_bot_grid->SetColSize(1, colw);
	m_bot_grid->SetColMinimalWidth(1, colw);
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
MLCompGenPanel::MLCompGenPanel(
	VRenderFrame* frame, wxWindow* parent) :
	MachineLearningPanel(frame, parent)
{
	m_dir = "Database";
	m_ext = ".cgtbl";
	m_top_grid_name = "Data Sets";
	m_top_grid_id = ID_TopGrid;
	m_new_table_id = ID_NewTableBtn;
	m_load_table_id = ID_LoadTableBtn;
	m_del_table_id = ID_DelTableBtn;
	m_dup_table_id = ID_DupTableBtn;
	m_bot_grid_name = "Machine Learning Records";
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
		std::string name = table.getName();
		std::string filename = m_exepath;
		filename += GETSLASH() + m_dir + GETSLASH() + name + m_ext;
		table.save(filename);
	}
}

void MLCompGenPanel::OnNewTable(wxCommandEvent& event)
{
	//flrd::TableHistParams table;
	//std::string name = "New_table";
	//MatchTableName(name);
	//table.setName(name);
	//std::string filename = m_exepath;
	//filename += GETSLASH() + m_dir + GETSLASH() + name + m_ext;
	//table.save(filename);
	//PopTopList();
	m_top_grid->InsertRows(0);
}

void MLCompGenPanel::OnLoadTable(wxCommandEvent& event)
{
	wxArrayInt seli = m_top_grid->GetSelectedRows();
	if (seli.GetCount() > 0)
	{
		std::string name = m_top_grid->GetCellValue(seli[0], 0).ToStdString();
		LoadTable(name);
		UpdateBotList();
	}
}

void MLCompGenPanel::OnDelTable(wxCommandEvent& event)
{
	wxArrayInt seli = m_top_grid->GetSelectedRows();
	size_t count = seli.GetCount();
	if (!count)
		return;

	flrd::TableHistParams& table = glbin.get_cg_table();
	std::string name;
	std::string filename = m_exepath;
	filename += GETSLASH() + m_dir + GETSLASH();
	for (size_t i = 0; i < count; ++i)
	{
		name = m_top_grid->GetCellValue(seli[i], 0).ToStdString();
		if (name == table.getName())
		{
			table.clear();
			UpdateBotList();
		}
		name = filename + name + m_ext;
		std::remove(name.c_str());
	}
	PopTopList();
}

void MLCompGenPanel::OnDupTable(wxCommandEvent& event)
{
	flrd::TableHistParams& table = glbin.get_cg_table();
	if (table.getRecSize() == 0)
	{
		OnNewTable(event);
		return;
	}

	flrd::TableHistParams new_table(table);
	std::string name = new_table.getName();
	if (MatchTableName(name))
		new_table.setName(name);
	//save it
	std::string str = m_exepath;
	str += GETSLASH() + m_dir + GETSLASH() + name + m_ext;
	new_table.save(str);
	PopTopList();
}

void MLCompGenPanel::OnStartRec(wxCommandEvent& event)
{
	flrd::TableHistParams& table = glbin.get_cg_table();
	if (table.getName().empty())
	{
		m_record = false;
		m_start_rec_btn->SetValue(false);
		return;
	}

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

void MLCompGenPanel::OnBotGridAutoSize(wxGridSizeEvent& event)
{
	EvenSizeBotGrid();
}

void MLCompGenPanel::OnTopGridCellChanged(wxGridEvent& event)
{
	int c = event.GetCol();
	int r = event.GetRow();
	std::string str0, str1;
	flrd::TableHistParams& table = glbin.get_cg_table();
	if (c == 0)
	{
		//name
		str0 = event.GetString();
		str1 = m_top_grid->GetCellValue(r, c).ToStdString();
		if (str0 == table.getName())
			table.setName(str1);
		flrd::TableHistParams temptbl;
		std::string filename = m_exepath;
		filename += GETSLASH() + m_dir + GETSLASH();
		temptbl.open(filename + str0 + m_ext);
		temptbl.setName(str1);
		temptbl.save(filename + str1 + m_ext);
		PopTopList();
		m_top_grid->ClearSelection();
	}
	else if (c == 2)
	{
		//notes
		str0 = m_top_grid->GetCellValue(r, 0).ToStdString();
		str1 = m_top_grid->GetCellValue(r, c).ToStdString();
		if (str0 == table.getName())
		{
			table.setNotes(str1);
		}
		else
		{
			flrd::TableHistParams temptbl;
			std::string filename = m_exepath;
			filename += GETSLASH() + m_dir + GETSLASH() + str0 + m_ext;
			temptbl.open(filename);
			temptbl.setNotes(str1);
			temptbl.save(filename);
		}
	}
	event.Skip();
}

void MLCompGenPanel::UpdateBotList()
{
	int row = m_bot_grid->GetNumberRows();
	if (row > 0)
		m_bot_grid->DeleteRows(0, row, true);

	flrd::TableHistParams& table = glbin.get_cg_table();
	std::string name = table.getName();
	if (name.empty())
	{
		m_bot_table_name->SetLabelText("No table loaded");
		m_start_prompt_text->Hide();
		m_record = false;
		m_start_rec_btn->SetValue(false);
		m_start_rec_btn->SetLabel("Start");
		glbin.set_cg_table_enable(false);
	}
	else
	{
		m_bot_table_name->SetLabelText("Table loaded:" + name);
		m_start_prompt_text->Show();
	}
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
	EvenSizeBotGrid();
	m_bot_grid->ClearSelection();
	Layout();
}

void MLCompGenPanel::LoadTable(const std::string& filename)
{
	std::string str = m_exepath;
	str += GETSLASH() + m_dir + GETSLASH();
	flrd::TableHistParams& table = glbin.get_cg_table();
	//save existing table if modified
	if (table.getModified())
	{
		std::string name = table.getName();
		str += name + m_ext;
		table.save(str);
	}
	str += filename + m_ext;
	table.open(str);
}

void MLCompGenPanel::SaveTable(const std::string& filename)
{
	std::string str = m_exepath;
	str += GETSLASH() + m_dir + GETSLASH() + filename + m_ext;
	glbin.get_cg_table().save(str);
}

