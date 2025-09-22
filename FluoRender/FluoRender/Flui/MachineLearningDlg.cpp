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
#include <MachineLearningDlg.h>
#include <Global.h>
#include <Names.h>
#include <MainSettings.h>
#include <MainFrame.h>
#include <ComponentDlg.h>
#include <EntryHist.h>
#include <Table.h>
#include <TableHistParams.h>
#include <CompGenerator.h>
#include <CurrentObjects.h>
#include <VolumeData.h>
#include <VolumeGroup.h>
#include <format>
#include <filesystem>

MachineLearningDlg::MachineLearningDlg(MainFrame *frame) :
	TabbedPanel(frame, frame,
		wxDefaultPosition,
		frame->FromDIP(wxSize(500, 620)),
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
	MLVolPropPanel* panel1 = new MLVolPropPanel(frame, m_notebook);
	MLCompGenPanel* panel2 = new MLCompGenPanel(frame, m_notebook);
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

void MachineLearningDlg::FluoUpdate(const fluo::ValueCollection& vc)
{
	//update user interface
	if (FOUND_VALUE(gstNull))
		return;
	bool update_all = vc.empty();

	for (auto it : m_panels)
	{
		if (it)
			it->FluoUpdate(vc);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
MachineLearningPanel::MachineLearningPanel(
	MainFrame* frame, wxWindow* parent) :
	PropPanel(frame, parent,
		wxDefaultPosition,
		frame->FromDIP(wxSize(500, 620)),
		0, "MachineLearningPanel"),
	m_record(false)
{
	std::filesystem::path p = std::filesystem::current_path();
	m_exepath = p.wstring();
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

	m_panel_top = new wxPanel(m_splitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER);
	wxBoxSizer* sizerTop = new wxBoxSizer(wxVERTICAL);
	st = new wxStaticText(m_panel_top, wxID_ANY, m_top_grid_name,
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
	st = new wxStaticText(m_panel_bot, wxID_ANY, m_bot_grid_name,
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
	m_del_rec_btn = new wxButton(m_panel_bot, wxID_ANY, "Delete",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_del_rec_btn->Bind(wxEVT_BUTTON, &MachineLearningPanel::OnDelRec, this);
	m_apply_rec_btn = new wxButton(m_panel_bot, wxID_ANY, "Apply",
		wxDefaultPosition,wxDefaultSize, wxALIGN_LEFT);
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

void MachineLearningPanel::FluoUpdate(const fluo::ValueCollection& vc)
{
	//update user interface
	if (FOUND_VALUE(gstNull))
		return;
	bool update_all = vc.empty();

	if (update_all || FOUND_VALUE(gstMlTopList))
		PopTopList();
}

void MachineLearningPanel::PopTopList()
{
	int row = m_top_grid->GetNumberRows();
	if (row)
		m_top_grid->DeleteRows(0, row, true);

	std::filesystem::path p(m_exepath);
	p /= m_dir;
	std::wstring path = p.wstring();
	std::wstring name, ext, filename;
	int i = 0;
	for (const auto& entry : std::filesystem::directory_iterator(path))
	{
		ext = entry.path().extension().wstring();
		if (ext != m_ext)
			continue;
		filename = entry.path().wstring();
		flrd::Table table;
		table.open(filename, true);
		m_top_grid->InsertRows(i);
		name = table.getName();
		if (name.empty()) name = entry.path().stem().wstring();
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
	std::wstring name = table.getName();
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

bool MachineLearningPanel::MatchTableName(std::wstring& name)
{
	bool modified = false;
	std::filesystem::path p(m_exepath);
	p /= m_dir;
	std::wstring path = p.wstring();
	std::wstring stem, ext;
	while (true)
	{
		bool found_same = false;
		for (const auto& entry : std::filesystem::directory_iterator(path))
		{
			stem = entry.path().stem().wstring();
			ext = entry.path().extension().wstring();
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
MLCompGenPanel::MLCompGenPanel(
	MainFrame* frame, wxWindow* parent) :
	MachineLearningPanel(frame, parent)
{
	m_dir = L"Database";
	m_ext = L".cgtbl";
	m_top_grid_name = "Data Sets";
	m_bot_grid_name = "Machine Learning Records";
	Create();

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
		std::wstring name = table.getName();
		std::filesystem::path p(m_exepath);
		p /= m_dir;
		p /= name + m_ext;
		std::wstring filename = p.wstring();
		table.save(filename);
	}
}

void MLCompGenPanel::FluoUpdate(const fluo::ValueCollection& vc)
{
	MachineLearningPanel::FluoUpdate(vc);

	//update user interface
	if (FOUND_VALUE(gstNull))
		return;
	bool update_all = vc.empty();

	bool bval;

	if (update_all ||
		FOUND_VALUE(gstMlAutoStart) ||
		FOUND_VALUE(gstMlCgAutoStart))
	{
		bval = glbin_settings.m_cg_auto_start;
		m_auto_start_check->SetValue(bval);
	}

	if (update_all || FOUND_VALUE(gstMlAutoLoadTable))
		AutoLoadTable();
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
	std::wstring name;
	std::filesystem::path p(m_exepath);
	p /= m_dir;
	p /= "";
	std::wstring filename = p.wstring();
	for (size_t i = 0; i < count; ++i)
	{
		name = m_top_grid->GetCellValue(seli[i], 0).ToStdWstring();
		if (name == table.getName())
		{
			table.clear();
			UpdateBotList();
		}
		name = filename + name + m_ext;
		std::remove(ws2s(name).c_str());
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
	std::wstring name = new_table.getName();
	if (MatchTableName(name))
		new_table.setName(name);
	//save it
	std::filesystem::path p(m_exepath);
	p /= m_dir;
	p /= name + m_ext;
	std::wstring str = p.wstring();
	new_table.save(str);
	PopTopList();
}

void MLCompGenPanel::OnAutoLoad(wxCommandEvent& event)
{
	wxArrayInt seli = m_top_grid->GetSelectedRows();
	if (seli.GetCount() > 0)
	{
		std::wstring name = m_top_grid->GetCellValue(seli[0], 0).ToStdWstring();
		glbin_settings.m_cg_table = name;
	}
}

void MLCompGenPanel::OnAutoStartRec(wxCommandEvent& event)
{
	glbin_settings.m_cg_auto_start = m_auto_start_check->GetValue();
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

void MLCompGenPanel::OnApplyRec(wxCommandEvent& event)
{
	if (!m_frame)
		return;
	glbin_comp_generator.ApplyRecord();
}

void MLCompGenPanel::OnBotGridAutoSize(wxGridSizeEvent& event)
{
	EvenSizeBotGrid();
}

void MLCompGenPanel::OnTopGridCellChanged(wxGridEvent& event)
{
	int c = event.GetCol();
	int r = event.GetRow();
	std::wstring str0, str1;
	flrd::TableHistParams& table = glbin.get_cg_table();
	if (c == 0)
	{
		//name
		str0 = event.GetString();
		str1 = m_top_grid->GetCellValue(r, c).ToStdWstring();
		if (str0 == table.getName())
			table.setName(str1);
		flrd::TableHistParams temptbl;
		std::filesystem::path p(m_exepath);
		p /= m_dir;
		p /= "";
		std::wstring filename = p.wstring();
		temptbl.open(filename + str0 + m_ext);
		temptbl.setName(str1);
		temptbl.save(filename + str1 + m_ext);
		PopTopList();
		m_top_grid->ClearSelection();
	}
	else if (c == 2)
	{
		//notes
		str0 = m_top_grid->GetCellValue(r, 0).ToStdWstring();
		str1 = m_top_grid->GetCellValue(r, c).ToStdWstring();
		if (str0 == table.getName())
		{
			table.setNotes(str1);
		}
		else
		{
			flrd::TableHistParams temptbl;
			std::filesystem::path p(m_exepath);
			p /= m_dir;
			p /= str0 + m_ext;
			std::wstring filename = p.wstring();
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
	std::wstring name = table.getName();
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
		m_bot_table_name->SetLabelText(L"Table loaded:" + name);
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
#ifdef _WIN32
			for (size_t j = 0; j < data_in.size() - 1; ++j)
				str_in += std::format("{:.2f}", data_in[j]) + ", ";
			str_in += std::format("{:.2f}", data_in[len - 1]);
#else
            for (size_t j = 0; j < data_in.size() - 1; ++j)
                str_in += wxString::Format("%.2f", data_in[j]).ToStdString() + ", ";
            str_in += wxString::Format("%.2f", data_in[len - 1]).ToStdString();
#endif
		}
		m_bot_grid->SetCellValue(0, 0, str_in);

		str_out.clear();
		table.getOneOutput(i, data_out);
		len = data_out.size();
		if (len)
		{
#ifdef _WIN32
			for (size_t j = 0; j < data_out.size() - 1; ++j)
				str_out += std::format("{:.2f}", data_out[j]) + ", ";
			str_out += std::format("{:.2f}", data_out[len - 1]);
#else
			for (size_t j = 0; j < data_out.size() - 1; ++j)
				str_out += wxString::Format("%.2f", data_out[j]).ToStdString() + ", ";
			str_out += wxString::Format("%.2f", data_out[len - 1]).ToStdString();
#endif
		}
		m_bot_grid->SetCellValue(0, 1, str_out);
	}
	EvenSizeBotGrid();
	m_bot_grid->ClearSelection();
	Layout();
}

void MLCompGenPanel::AutoLoadTable()
{
	std::wstring name, str;
	int count = m_top_grid->GetNumberRows();
	bool found = false;
	name = glbin_settings.m_cg_table;
	for (int i = 0; i < count; ++i)
	{
		str = m_top_grid->GetCellValue(i, 0).ToStdWstring();
		if (str == name)
		{
			found = true;
			break;
		}
	}
	if (!found && count > 0)
		name = m_top_grid->GetCellValue(0, 0).ToStdWstring();
	LoadTable(name);
	UpdateBotList();

	if (glbin_settings.m_cg_auto_start)
	{
		wxCommandEvent e;
		OnStartRec(e);
	}
}

void MLCompGenPanel::LoadTable(const std::wstring& filename)
{
	std::filesystem::path p(m_exepath);
	p /= m_dir;
	p /= "";
	std::wstring str = p.wstring();
	flrd::TableHistParams& table = glbin.get_cg_table();
	//save existing table if modified
	if (table.getModified())
	{
		std::wstring name = table.getName();
		str += name + m_ext;
		table.save(str);
	}
	str += filename + m_ext;
	table.open(str);
}

void MLCompGenPanel::SaveTable(const std::wstring& filename)
{
	std::filesystem::path p(m_exepath);
	p /= m_dir;
	p /= filename + m_ext;
	std::wstring str = p.wstring();
	glbin.get_cg_table().save(str);
}

void MLCompGenPanel::SetAutoStart(bool bval)
{
	//MachineLearningPanel::SetAutoStart(bval);
	glbin_settings.m_cg_auto_start = bval;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
MLVolPropPanel::MLVolPropPanel(
	MainFrame* frame, wxWindow* parent) :
	MachineLearningPanel(frame, parent)
{
	m_dir = L"Database";
	m_ext = L".vptbl";
	m_top_grid_name = "Data Sets";
	m_bot_grid_name = "Machine Learning Records";
	Create();

	//add more options
	m_auto_apply_chk = new wxCheckBox(m_panel_bot, wxID_ANY, "Auto Apply",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_auto_apply_chk->Bind(wxEVT_CHECKBOX, &MLVolPropPanel::OnAutoApply, this);
	m_sizer2->Add(m_auto_apply_chk, 0);
	m_sizer2->Add(5, 5);
	m_panel_bot->Layout();

	flrd::TableHistParams& table = glbin.get_vp_table();
	table.setUpdateFunc(std::bind(
		&MLVolPropPanel::UpdateList, this, std::placeholders::_1));
}

MLVolPropPanel::~MLVolPropPanel()
{
	flrd::TableHistParams& table = glbin.get_vp_table();
	//save existing table if modified
	if (table.getModified())
	{
		std::wstring name = table.getName();
		std::filesystem::path p(m_exepath);
		p /= m_dir;
		p /= name + m_ext;
		std::wstring filename = p.wstring();
		table.save(filename);
	}
}

void MLVolPropPanel::FluoUpdate(const fluo::ValueCollection& vc)
{
	MachineLearningPanel::FluoUpdate(vc);

	//update user interface
	if (FOUND_VALUE(gstNull))
		return;
	bool update_all = vc.empty();

	bool bval;

	if (update_all ||
		FOUND_VALUE(gstMlAutoStart) ||
		FOUND_VALUE(gstMlVpAutoStart))
	{
		bval = glbin_settings.m_vp_auto_start;
		m_auto_start_check->SetValue(bval);
	}

	if (update_all || FOUND_VALUE(gstMlVpAutoApply))
	{
		bval = glbin_settings.m_vp_auto_apply;
		m_auto_apply_chk->SetValue(bval);
	}

	if (update_all || FOUND_VALUE(gstMlAutoLoadTable))
		AutoLoadTable();
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
		LoadTable(name);
		UpdateBotList();
	}
}

void MLVolPropPanel::OnDelTable(wxCommandEvent& event)
{
	wxArrayInt seli = m_top_grid->GetSelectedRows();
	size_t count = seli.GetCount();
	if (!count)
		return;

	flrd::TableHistParams& table = glbin.get_vp_table();
	std::wstring name;
	std::filesystem::path p(m_exepath);
	p /= m_dir;
	p /= "";
	std::wstring filename = p.wstring();
	for (size_t i = 0; i < count; ++i)
	{
		name = m_top_grid->GetCellValue(seli[i], 0).ToStdWstring();
		if (name == table.getName())
		{
			table.clear();
			UpdateBotList();
		}
		name = filename + name + m_ext;
		std::remove(ws2s(name).c_str());
	}
	PopTopList();
}

void MLVolPropPanel::OnDupTable(wxCommandEvent& event)
{
	flrd::TableHistParams& table = glbin.get_vp_table();
	if (table.getRecSize() == 0)
	{
		OnNewTable(event);
		return;
	}

	flrd::TableHistParams new_table(table);
	std::wstring name = new_table.getName();
	if (MatchTableName(name))
		new_table.setName(name);
	//save it
	std::filesystem::path p(m_exepath);
	p /= m_dir;
	p /= name + m_ext;
	std::wstring str = p.wstring();
	new_table.save(str);
	PopTopList();
}

void MLVolPropPanel::OnAutoLoad(wxCommandEvent& event)
{
	wxArrayInt seli = m_top_grid->GetSelectedRows();
	if (seli.GetCount() > 0)
		glbin_settings.m_vp_table = m_top_grid->GetCellValue(seli[0], 0).ToStdWstring();
}

void MLVolPropPanel::OnAutoStartRec(wxCommandEvent& event)
{
	glbin_settings.m_vp_auto_start = m_auto_start_check->GetValue();
}

void MLVolPropPanel::OnStartRec(wxCommandEvent& event)
{
	flrd::TableHistParams& table = glbin.get_vp_table();
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
		glbin.set_vp_table_enable(true);
	}
	else
	{
		m_start_rec_btn->SetLabel("Start");
		m_start_rec_btn->SetValue(false);
		glbin.set_vp_table_enable(false);
	}
}

void MLVolPropPanel::OnDelRec(wxCommandEvent& event)
{
	flrd::TableHistParams& table = glbin.get_vp_table();
	wxArrayInt seli = m_bot_grid->GetSelectedRows();
	std::vector<size_t> vi;
	size_t count = table.getRecSize();
	for (size_t i = 0; i < seli.GetCount(); ++i)
		vi.push_back(count - 1 - seli[i]);
	table.delRecords(vi);
}

void MLVolPropPanel::OnApplyRec(wxCommandEvent& event)
{
	auto vd = glbin_current.vol_data.lock();
	auto group = glbin_current.vol_group.lock();
	if (group && group->GetVolumeSyncProp())
		group->ApplyMlVolProp();
	else if (vd)
		vd->ApplyMlVolProp();
}

void MLVolPropPanel::OnBotGridAutoSize(wxGridSizeEvent& event)
{
	EvenSizeBotGrid();
}

void MLVolPropPanel::OnTopGridCellChanged(wxGridEvent& event)
{
	int c = event.GetCol();
	int r = event.GetRow();
	std::wstring str0, str1;
	flrd::TableHistParams& table = glbin.get_vp_table();
	if (c == 0)
	{
		//name
		str0 = event.GetString();
		str1 = m_top_grid->GetCellValue(r, c).ToStdWstring();
		if (str0 == table.getName())
			table.setName(str1);
		flrd::TableHistParams temptbl;
		std::filesystem::path p(m_exepath);
		p /= m_dir;
		p /= "";
		std::wstring filename = p.wstring();
		temptbl.open(filename + str0 + m_ext);
		temptbl.setName(str1);
		temptbl.save(filename + str1 + m_ext);
		PopTopList();
		m_top_grid->ClearSelection();
	}
	else if (c == 2)
	{
		//notes
		str0 = m_top_grid->GetCellValue(r, 0).ToStdWstring();
		str1 = m_top_grid->GetCellValue(r, c).ToStdWstring();
		if (str0 == table.getName())
		{
			table.setNotes(str1);
		}
		else
		{
			flrd::TableHistParams temptbl;
			std::filesystem::path p(m_exepath);
			p /= m_dir;
			p /= str0 + m_ext;
			std::wstring filename = p.wstring();
			temptbl.open(filename);
			temptbl.setNotes(str1);
			temptbl.save(filename);
		}
	}
	event.Skip();
}

void MLVolPropPanel::UpdateBotList()
{
	int row = m_bot_grid->GetNumberRows();
	if (row > 0)
		m_bot_grid->DeleteRows(0, row, true);

	flrd::TableHistParams& table = glbin.get_vp_table();
	std::wstring name = table.getName();
	if (name.empty())
	{
		m_bot_table_name->SetLabelText("No table loaded");
		m_start_prompt_text->Hide();
		m_record = false;
		m_start_rec_btn->SetValue(false);
		m_start_rec_btn->SetLabel("Start");
		glbin.set_vp_table_enable(false);
	}
	else
	{
		m_bot_table_name->SetLabelText(L"Table loaded:" + name);
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
#ifdef _WIN32
			for (size_t j = 0; j < data_in.size() - 1; ++j)
				str_in += std::format("{:.2f}", data_in[j]) + ", ";
			str_in += std::format("{:.2f}", data_in[len - 1]);
#else
			for (size_t j = 0; j < data_in.size() - 1; ++j)
				str_in += wxString::Format("%.2f", data_in[j]).ToStdString() + ", ";
			str_in += wxString::Format("%.2f", data_in[len - 1]).ToStdString();
#endif
		}
		m_bot_grid->SetCellValue(0, 0, str_in);

		str_out.clear();
		table.getOneOutput(i, data_out);
		len = data_out.size();
		if (len)
		{
#ifdef _WIN32
			for (size_t j = 0; j < data_out.size() - 1; ++j)
				str_out += std::format("{:.2f}", data_out[j]) + ", ";
			str_out += std::format("{:.2f}", data_out[len - 1]);
#else
			for (size_t j = 0; j < data_out.size() - 1; ++j)
				str_out += wxString::Format("%.2f", data_out[j]).ToStdString() + ", ";
			str_out += wxString::Format("%.2f", data_out[len - 1]).ToStdString();
#endif
		}
		m_bot_grid->SetCellValue(0, 1, str_out);
	}
	EvenSizeBotGrid();
	m_bot_grid->ClearSelection();
	Layout();
}

void MLVolPropPanel::AutoLoadTable()
{
	std::wstring name, str;
	int count = m_top_grid->GetNumberRows();
	bool found = false;
	name = glbin_settings.m_vp_table;
	for (int i = 0; i < count; ++i)
	{
		str = m_top_grid->GetCellValue(i, 0).ToStdWstring();
		if (str == name)
		{
			found = true;
			break;
		}
	}
	if (!found && count > 0)
		name = m_top_grid->GetCellValue(0, 0).ToStdWstring();
	LoadTable(name);
	UpdateBotList();

	if (glbin_settings.m_vp_auto_start)
	{
		wxCommandEvent e;
		OnStartRec(e);
	}
}

void MLVolPropPanel::LoadTable(const std::wstring& filename)
{
	std::filesystem::path p(m_exepath);
	p /= m_dir;
	p /= "";
	std::wstring str = p.wstring();
	flrd::TableHistParams& table = glbin.get_vp_table();
	std::wstring str2;
	//save existing table if modified
	if (table.getModified())
	{
		std::wstring name = table.getName();
		str2 = str + name + m_ext;
		table.save(str2);
	}
	str2 = str + filename + m_ext;
	table.open(str2);
}

void MLVolPropPanel::SaveTable(const std::wstring& filename)
{
	std::filesystem::path p(m_exepath);
	p /= m_dir;
	p /= filename + m_ext;
	std::wstring str = p.wstring();
	glbin.get_vp_table().save(str);
}

void MLVolPropPanel::SetAutoStart(bool bval)
{
	//MachineLearningPanel::SetAutoStart(bval);
	glbin_settings.m_vp_auto_start = bval;
}

void MLVolPropPanel::OnAutoApply(wxCommandEvent& event)
{
	glbin_settings.m_vp_auto_apply = m_auto_apply_chk->GetValue();
}
