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
#ifndef _MACHINELEARNINGDLG_H_
#define _MACHINELEARNINGDLG_H_

#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/grid.h>
#include <wx/splitter.h>
#include <wx/tglbtn.h>
#include <string>

class MainFrame;
class MLCompGenPanel;
class MLVolPropPanel;
class MachineLearningDlg : public wxPanel
{
public:
	enum
	{
		ID_AutoStartAll = ID_LEARNING
	};
	MachineLearningDlg(MainFrame* frame);
	~MachineLearningDlg();

private:
	MainFrame* m_frame;
	//
	wxCheckBox* m_auto_start_all;
	MLCompGenPanel* m_panel1;
	MLVolPropPanel* m_panel2;

private:
	void OnAutoStartAll(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
class MachineLearningPanel : public wxPanel
{
public:
	MachineLearningPanel(MainFrame* frame,
		wxWindow* parent);
	~MachineLearningPanel();

	void Create();
	virtual void PopTopList();
	virtual void UpdateList(int index);
	virtual void UpdateTopList();
	virtual void UpdateBotList() {};
	virtual void AutoLoadTable() = 0;
	virtual void LoadTable(const std::string& filename) = 0;
	virtual void SaveTable(const std::string& filename) = 0;
	virtual void SetAutoStart(bool bval) { m_auto_start_check->SetValue(bval); }

protected:
	MainFrame* m_frame;
	wxSplitterWindow* m_splitter;
	//
	wxPanel* m_panel_top;
	wxGrid *m_top_grid;
	wxString m_top_grid_name;
	wxWindowID m_top_grid_id;
	wxButton* m_new_table_btn;
	wxWindowID m_new_table_id;
	wxButton* m_load_table_btn;
	wxWindowID m_load_table_id;
	wxButton* m_del_table_btn;
	wxWindowID m_del_table_id;
	wxButton* m_dup_table_btn;
	wxWindowID m_dup_table_id;
	wxButton* m_auto_load_btn;
	wxWindowID m_auto_load_id;
	//
	wxPanel* m_panel_bot;
	wxGrid *m_bot_grid;
	wxString m_bot_grid_name;
	wxWindowID m_bot_grid_id;
	wxBoxSizer* m_sizer2;
	wxStaticText* m_bot_table_name;
	wxStaticText* m_start_prompt_text;
	wxCheckBox* m_auto_start_check;
	wxWindowID m_auto_start_id;
	wxToggleButton* m_start_rec_btn;
	wxWindowID m_start_rec_id;
	wxButton* m_del_rec_btn;
	wxWindowID m_del_rec_id;
	wxButton* m_apply_rec_btn;
	wxWindowID m_apply_rec_id;

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
	enum
	{
		ID_TopGrid = ID_LEARNING_COMP_GEN,
		ID_NewTableBtn,
		ID_LoadTableBtn,
		ID_DelTableBtn,
		ID_DupTableBtn,
		ID_AutoLoadBtn,
		ID_BotGrid,
		ID_AutoStartChk,
		ID_StartRecBtn,
		ID_DelRecBtn,
		ID_ApplyRecBtn
	};
	MLCompGenPanel(MainFrame* frame,
		wxWindow* parent);
	~MLCompGenPanel();

	virtual void UpdateBotList();
	virtual void AutoLoadTable();
	virtual void LoadTable(const std::string& filename);
	virtual void SaveTable(const std::string& filename);

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
	enum
	{
		ID_TopGrid = ID_LEARNING_VOL_PROP,
		ID_NewTableBtn,
		ID_LoadTableBtn,
		ID_DelTableBtn,
		ID_DupTableBtn,
		ID_AutoLoadBtn,
		ID_BotGrid,
		ID_AutoStartChk,
		ID_StartRecBtn,
		ID_DelRecBtn,
		ID_ApplyRecBtn,
		ID_AutoApplyChk
	};
	MLVolPropPanel(MainFrame* frame,
		wxWindow* parent);
	~MLVolPropPanel();

	virtual void UpdateBotList();
	virtual void AutoLoadTable();
	virtual void LoadTable(const std::string& filename);
	virtual void SaveTable(const std::string& filename);

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