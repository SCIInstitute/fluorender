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
#ifndef _MACHINELEARNINGDLG_H_
#define _MACHINELEARNINGDLG_H_

#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/grid.h>
#include <wx/splitter.h>
#include <wx/tglbtn.h>
#include <string>

class VRenderFrame;
class MachineLearningDlg : public wxPanel
{
public:
	MachineLearningDlg(VRenderFrame* frame);
	~MachineLearningDlg();

private:
	VRenderFrame* m_frame;

	DECLARE_EVENT_TABLE()
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
class MachineLearningPanel : public wxPanel
{
public:
	MachineLearningPanel(VRenderFrame* frame,
		wxWindow* parent);
	~MachineLearningPanel();

	void Create();
	virtual void PopTopList();
	virtual void UpdateList(int index);
	virtual void UpdateTopList();
	virtual void UpdateBotList() {};

protected:
	VRenderFrame* m_frame;
	wxSplitterWindow* m_splitter;
	//
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
	//
	wxGrid *m_bot_grid;
	wxString m_bot_grid_name;
	wxWindowID m_bot_grid_id;
	wxStaticText* m_bot_table_name;
	wxStaticText* m_start_prompt_text;
	wxToggleButton* m_start_rec_btn;
	wxWindowID m_start_rec_id;
	wxButton* m_del_rec_btn;
	wxWindowID m_del_rec_id;

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
	//
	virtual void OnStartRec(wxCommandEvent& event) = 0;
	virtual void OnDelRec(wxCommandEvent& event) = 0;
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
		ID_BotGrid,
		ID_StartRecBtn,
		ID_DelRecBtn
	};
	MLCompGenPanel(VRenderFrame* frame,
		wxWindow* parent);
	~MLCompGenPanel();

	virtual void UpdateBotList();
	void LoadTable(const std::string& filename);
	void SaveTable(const std::string& filename);

protected:
	virtual void OnNewTable(wxCommandEvent& event);
	virtual void OnLoadTable(wxCommandEvent& event);
	virtual void OnDelTable(wxCommandEvent& event);
	virtual void OnDupTable(wxCommandEvent& event);
	//
	virtual void OnStartRec(wxCommandEvent& event);
	virtual void OnDelRec(wxCommandEvent& event);
	//grid size
	void OnBotGridAutoSize(wxGridSizeEvent& event);
	//grid cell edit
	void OnTopGridCellChanged(wxGridEvent& event);

private:
};
#endif//_MACHINELEARNINGDLG_H_