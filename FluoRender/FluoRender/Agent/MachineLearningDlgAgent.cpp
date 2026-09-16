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

#include <MachineLearningDlgAgent.h>
#include <MachineLearningDlg.h>
#include <Global.h>
#include <Names.h>
#include <MainSettings.h>
#include <Table.h>
#include <TableHistParams.h>
#include <GridBuilder.h>
#include <CompGenerator.h>
#include <Directory.h>
#include <filesystem>

MachineLearningDlgAgent::MachineLearningDlgAgent(
	MachineLearningDlg* dlg) :
	Agent(dlg)
{

}

void MachineLearningDlgAgent::UpdateUI(const UpdateRequest& request)
{
	bool update_all = request.values.empty();

	//request panels to update
	UpdateRequest sub_request(request.values, this, request.mode, request.reason);
	Notify(sub_request);
}

void MachineLearningDlgAgent::UpdateData(const UpdateRequest& request)
{

}

MachineLearningDlg* MachineLearningDlgAgent::GetDialog() const
{
	return static_cast<MachineLearningDlg*>(GetWindow());
}

MachineLearningPanelAgent::MachineLearningPanelAgent(
	MachineLearningPanel* panel) :
	Agent(panel)
{
	std::filesystem::path p = GetUserSettingsRoot();
	m_exepath = p.wstring();
}

void MachineLearningPanelAgent::UpdateUI(const UpdateRequest& request)
{
	auto panel = GetPanel();
	if (!panel)
		return;

	bool update_all = request.values.empty();

	if (update_all || request.HasValue(gstMlTopList))
		UpdateTopListFromFile();
	if (update_all || request.HasValue(gstMlBotList))
		UpdateBotList();
}

void MachineLearningPanelAgent::UpdateData(const UpdateRequest& request)
{

}

MachineLearningPanel* MachineLearningPanelAgent::GetPanel() const
{
	return static_cast<MachineLearningPanel*>(GetWindow());
}

void MachineLearningPanelAgent::UpdateTopListFromFile()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	std::string titles =
		"Name\t"
		"Records\t"
		"Notes\t"
		"Date modified\t"
		"Date created\n";

	std::string values;

	std::filesystem::path p(m_exepath);
	p /= m_dir;

	if (!std::filesystem::exists(p) ||
		!std::filesystem::is_directory(p))
		return;

	for (const auto& entry : std::filesystem::directory_iterator(p))
	{
		auto ext = entry.path().extension().wstring();
		if (ext != m_ext)
			continue;

		flrd::Table table;
		table.open(entry.path().wstring(), true);

		std::wstring name = table.getName();
		if (name.empty())
			name = entry.path().stem().wstring();

		char b[32];
		std::tm* ptm;

		std::time_t modifyTime = table.getModifyTime();
		ptm = std::localtime(&modifyTime);
		std::strftime(b, sizeof(b), "%m/%d/%Y %H:%M:%S", ptm);
		std::string modify_str(b);

		std::time_t createTime = table.getCreateTime();
		ptm = std::localtime(&createTime);
		std::strftime(b, sizeof(b), "%m/%d/%Y %H:%M:%S", ptm);
		std::string create_str(b);

		values += ws2s(name) + "\t";
		values += std::to_string(table.getRecNum()) + "\t";
		values += ws2s(table.getNotes()) + "\t";
		values += modify_str + "\t";
		values += create_str + "\n";
	}

	auto griddata = GridBuilder::Build(titles, values);
	panel->PopTopList(griddata);
}

void MachineLearningPanelAgent::UpdateList(int index)
{
	if (index & 1)
		this->UpdateTopListByName();
	if (index & 2)
		this->UpdateBotList();
}

void MachineLearningPanelAgent::UpdateTopListByName()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	flrd::TableHistParams& table = glbin.get_cg_table();
	std::wstring name = table.getName();
	GridRowData row;
	row.cells.push_back({ ws2s(name) });
	row.cells.push_back({ std::to_string(table.getRecNum()) });
	row.cells.push_back({ ws2s(table.getNotes()) });
	char b[32];
	std::tm* ptm;
	std::time_t createTime = table.getCreateTime();
	ptm = std::localtime(&createTime);
	std::strftime(b, 32, "%m/%d/%Y %H:%M:%S", ptm);
	row.cells.push_back({ std::string(b) });
	std::time_t modifyTime = table.getModifyTime();
	ptm = std::localtime(&modifyTime);
	std::strftime(b, 32, "%m/%d/%Y %H:%M:%S", ptm);
	row.cells.push_back({ std::string(b) });

	panel->UpdateTopListRow(row);
}

bool MachineLearningPanelAgent::MatchTableName(std::wstring& name)
{
	bool modified = false;
	std::filesystem::path p(m_exepath);
	p /= m_dir;
	std::wstring path = p.wstring();
	std::wstring stem, ext;
	if (!std::filesystem::exists(p) || !std::filesystem::is_directory(p))
		return modified;
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

MLCompGenPanelAgent::MLCompGenPanelAgent(
	MLCompGenPanel* panel) :
	MachineLearningPanelAgent(panel)
{
	m_dir = L"Database";
	m_ext = L".cgtbl";
	m_top_grid_name = "Data Sets";
	m_bot_grid_name = "Machine Learning Records";

	flrd::TableHistParams& table = glbin.get_cg_table();
	table.setUpdateFunc(std::bind(
		&MLCompGenPanelAgent::UpdateList, this, std::placeholders::_1));
}

MLCompGenPanelAgent::~MLCompGenPanelAgent()
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

void MLCompGenPanelAgent::UpdateUI(const UpdateRequest& request)
{
	auto panel = GetPanel();
	if (!panel)
		return;

	bool update_all = request.values.empty();

	bool bval;

	if (update_all ||
		request.HasValue(gstMlAutoStart) ||
		request.HasValue(gstMlCgAutoStart))
	{
		bval = glbin_settings.m_cg_auto_start;
		panel->SetAutoStart(bval);
	}

	if (update_all || request.HasValue(gstMlAutoLoadTable))
		panel->AutoLoadTable();
}

void MLCompGenPanelAgent::UpdateData(const UpdateRequest& request)
{
	if (request.HasValue(gstCompGenDelTable))
		DelTable();
	if (request.HasValue(gstCompGenDupTable))
		DupTable();
	if (request.HasValue(gstCompGenStartRec))
		StartRecording();
	if (request.HasValue(gstCompGenApplyRecord))
		ApplyRecord();
}

MLCompGenPanel* MLCompGenPanelAgent::GetPanel() const
{
	return static_cast<MLCompGenPanel*>(GetWindow());
}

void MLCompGenPanelAgent::SetTable(const std::wstring& name)
{
	glbin_settings.m_cg_table = name;
}

void MLCompGenPanelAgent::SetAutoStart(bool bval)
{
	glbin_settings.m_cg_auto_start = bval;
}

void MLCompGenPanelAgent::DeleteRecord(const GridSelection& sel)
{
	flrd::TableHistParams& table = glbin.get_cg_table();
	std::vector<size_t> vi;
	size_t count = table.getRecSize();
	for (auto i : sel.rows)
		vi.push_back(count - 1 - i);
	table.delRecords(vi);
}

void MLCompGenPanelAgent::UpdateCellChanged(const GridCellChanged& cell)
{
	int c = cell.col;
	int r = cell.row;
	std::wstring str0, str1;
	flrd::TableHistParams& table = glbin.get_cg_table();
	if (c == 0)
	{
		//name
		str0 = cell.old_value;
		str1 = cell.new_value;
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
		UpdateTopListFromFile();
	}
	else if (c == 2)
	{
		//notes
		str0 = cell.index_value;
		str1 = cell.new_value;
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
}

void MLCompGenPanelAgent::UpdateBotList()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	GridData data;
	flrd::TableHistParams& table = glbin.get_cg_table();

	for (int i = 0; i < table.getRecSize(); ++i)
	{
		GridRowData row;

		// input column
		{
			std::string str;
			auto values = table.getOneInput(i);

			for (size_t j = 0; j < values.size(); ++j)
			{
				if (j > 0)
					str += ", ";

				str += wxString::Format("%.2f", values[j]).ToStdString();
			}

			row.cells.push_back({ str });
		}

		// output column
		{
			std::string str;
			auto values = table.getOneOutput(i);

			for (size_t j = 0; j < values.size(); ++j)
			{
				if (j > 0)
					str += ", ";

				str += wxString::Format("%.2f", values[j]).ToStdString();
			}

			row.cells.push_back({ str });
		}

		data.rows.push_back(row);
	}

	panel->PopBotList(data);
}

void MLCompGenPanelAgent::DelTable()
{
	flrd::TableHistParams& table = glbin.get_cg_table();

	std::wstring name;
	std::filesystem::path p(m_exepath);
	p /= m_dir;
	p /= "";
	std::wstring filename = p.wstring();
	std::remove(ws2s(name).c_str());
	UpdateTopListFromFile();
	UpdateBotList();
}

void MLCompGenPanelAgent::DupTable()
{
	flrd::TableHistParams& table = glbin.get_cg_table();
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
	UpdateTopListFromFile();
}

void MLCompGenPanelAgent::StartRecording()
{
	flrd::TableHistParams& table = glbin.get_cg_table();
	if (table.getName().empty())
	{
		m_record = false;
	}
	else
	{
		m_record = !m_record;
	}
	glbin.set_cg_table_enable(m_record);
	auto panel = GetPanel();
	if (panel)
		panel->UpdateStartRecording(m_record);
}

void MLCompGenPanelAgent::ApplyRecord()
{
	glbin_comp_generator.ApplyRecord();
}

MLVolPropPanelAgent::MLVolPropPanelAgent(
	MLVolPropPanel* panel) :
	MachineLearningPanelAgent(panel)
{

}

void MLVolPropPanelAgent::UpdateUI(const UpdateRequest& request)
{
	auto panel = GetPanel();
	if (!panel)
		return;

	bool update_all = request.values.empty();

	bool bval;

	if (update_all ||
		request.HasValue(gstMlAutoStart) ||
		request.HasValue(gstMlVpAutoStart))
	{
		bval = glbin_settings.m_vp_auto_start;
		panel->UpdateAutoStart(bval);
	}

	if (update_all || request.HasValue(gstMlVpAutoApply))
	{
		bval = glbin_settings.m_vp_auto_apply;
		panel->UpdateAutoApply(bval);
	}

	if (update_all || request.HasValue(gstMlAutoLoadTable))
		panel->AutoLoadTable();
}

void MLVolPropPanelAgent::UpdateData(const UpdateRequest& request)
{

}

MLVolPropPanel* MLVolPropPanelAgent::GetPanel() const
{
	return static_cast<MLVolPropPanel*>(GetWindow());
}
