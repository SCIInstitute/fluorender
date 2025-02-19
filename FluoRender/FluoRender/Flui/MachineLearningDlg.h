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
#ifndef _MACHINELEARNINGDLG_H_
#define _MACHINELEARNINGDLG_H_

#include <PropPanel.h>
#include <wx/notebook.h>
#include <wx/grid.h>
#include <wx/splitter.h>
#include <wx/tglbtn.h>
#include <wx/checkbox.h>
#include <string>
#include <vector>

class MachineLearningPanel;
class MachineLearningDlg : public PropPanel
{
public:
	MachineLearningDlg(MainFrame* frame);
	~MachineLearningDlg();

	virtual void FluoUpdate(const fluo::ValueCollection& vc = {});

private:
	wxCheckBox* m_auto_start_all;
	std::vector<MachineLearningPanel*> m_panels;
	//MLCompGenPanel* m_panel1;
	//MLVolPropPanel* m_panel2;

private:
	void OnAutoStartAll(wxCommandEvent& event);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
class MachineLearningPanel : public PropPanel
{
public:
	MachineLearningPanel(MainFrame* frame,
		wxWindow* parent);
	~MachineLearningPanel();

	void Create();
	virtual void FluoUpdate(const fluo::ValueCollection& vc = {});
	virtual void PopTopList();
	virtual void UpdateList(int index);
	virtual void UpdateTopList();
	virtual void UpdateBotList() {};
	virtual void AutoLoadTable() = 0;
	virtual void LoadTable(const std::string& filename) = 0;
	virtual void SaveTable(const std::string& filename) = 0;
	virtual void SetAutoStart(bool bval) { m_auto_start_check->SetValue(bval); }

protected:
	wxSplitterWindow* m_splitter;
	//
	wxPanel* m_panel_top;
	wxGrid *m_top_grid;
	wxString m_top_grid_name;
	wxButton* m_new_table_btn;
	wxButton* m_load_table_btn;
	wxButton* m_del_table_btn;
	wxButton* m_dup_table_btn;
	wxButton* m_auto_load_btn;
	//
	wxPanel* m_panel_bot;
	wxGrid *m_bot_grid;
	wxString m_bot_grid_name;
	wxBoxSizer* m_sizer2;
	wxStaticText* m_bot_table_name;
	wxStaticText* m_start_prompt_text;
	wxCheckBox* m_auto_start_check;
	wxToggleButton* m_start_rec_btn;
	wxButton* m_del_rec_btn;
	wxButton* m_apply_rec_btn;

	bool m_record;//state for recording
	std::string m_dir;//dir for searching tables
	std::string m_ext;//file extension for tables
	std::string m_exepath;//path to executable

protected:
	virtual bool MatchTableName(std::string& name);
	virtual void EvenSizeBotGrid();
	//
	virtual void OnNewTable(wxCommandEvent& event) = 0;
	virtual void OnLoadTable(wxCommandEvent& event) = 0;
	virtual void OnDelTable(wxCommandEvent& event) = 0;
	virtual void OnDupTable(wxCommandEvent& event) = 0;
	virtual void OnAutoLoad(wxCommandEvent& event) = 0;
	//
	virtual void OnAutoStartRec(wxCommandEvent& event) = 0;
	virtual void OnStartRec(wxCommandEvent& event) = 0;
	virtual void OnDelRec(wxCommandEvent& event) = 0;
	virtual void OnApplyRec(wxCommandEvent& event) = 0;
	//grid size
	virtual void OnBotGridAutoSize(wxGridSizeEvent& event) = 0;
	//grid edit
	virtual void OnTopGridCellChanging(wxGridEvent& event);
	virtual void OnBotGridCellChanging(wxGridEvent& event);
	virtual void OnTopGridCellChanged(wxGridEvent& event);
	virtual void OnBotGridCellChanged(wxGridEvent& event);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
class MLCompGenPanel : public MachineLearningPanel
{
public:
	MLCompGenPanel(MainFrame* frame,
		wxWindow* parent);
	~MLCompGenPanel();

	virtual void FluoUpdate(const fluo::ValueCollection& vc = {});
	virtual void UpdateBotList();
	virtual void AutoLoadTable();
	virtual void LoadTable(const std::string& filename);
	virtual void SaveTable(const std::string& filename);
	virtual void SetAutoStart(bool bval);

protected:
	virtual void OnNewTable(wxCommandEvent& event);
	virtual void OnLoadTable(wxCommandEvent& event);
	virtual void OnDelTable(wxCommandEvent& event);
	virtual void OnDupTable(wxCommandEvent& event);
	virtual void OnAutoLoad(wxCommandEvent& event);
	//
	virtual void OnAutoStartRec(wxCommandEvent& event);
	virtual void OnStartRec(wxCommandEvent& event);
	virtual void OnDelRec(wxCommandEvent& event);
	virtual void OnApplyRec(wxCommandEvent& event);
	//grid size
	void OnBotGridAutoSize(wxGridSizeEvent& event);
	//grid cell edit
	void OnTopGridCellChanged(wxGridEvent& event);

private:
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
class MLVolPropPanel : public MachineLearningPanel
{
public:
	MLVolPropPanel(MainFrame* frame,
		wxWindow* parent);
	~MLVolPropPanel();

	virtual void FluoUpdate(const fluo::ValueCollection& vc = {});
	virtual void UpdateBotList();
	virtual void AutoLoadTable();
	virtual void LoadTable(const std::string& filename);
	virtual void SaveTable(const std::string& filename);
	virtual void SetAutoStart(bool bval);

protected:
	virtual void OnNewTable(wxCommandEvent& event);
	virtual void OnLoadTable(wxCommandEvent& event);
	virtual void OnDelTable(wxCommandEvent& event);
	virtual void OnDupTable(wxCommandEvent& event);
	virtual void OnAutoLoad(wxCommandEvent& event);
	//
	virtual void OnAutoStartRec(wxCommandEvent& event);
	virtual void OnStartRec(wxCommandEvent& event);
	virtual void OnDelRec(wxCommandEvent& event);
	virtual void OnApplyRec(wxCommandEvent& event);
	//grid size
	void OnBotGridAutoSize(wxGridSizeEvent& event);
	//grid cell edit
	void OnTopGridCellChanged(wxGridEvent& event);

private:
	wxCheckBox* m_auto_apply_chk;

private:
	void OnAutoApply(wxCommandEvent& event);
};
#endif//_MACHINELEARNINGDLG_H_