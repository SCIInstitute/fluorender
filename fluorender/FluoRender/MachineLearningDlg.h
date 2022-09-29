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

protected:
	VRenderFrame* m_frame;
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
	wxToggleButton* m_start_rec_btn;
	wxWindowID m_start_rec_id;
	wxButton* m_del_rec_btn;
	wxWindowID m_del_rec_id;

	bool m_record;//state for recording
	wxString m_dir;//dir for searching tables
	wxString m_ext;//file extension for tables
	wxString m_exepath;//path to executable

protected:
	virtual void OnNewTable(wxCommandEvent& event) = 0;
	virtual void OnLoadTable(wxCommandEvent& event) = 0;
	virtual void OnDelTable(wxCommandEvent& event) = 0;
	virtual void OnDupTable(wxCommandEvent& event) = 0;
	//
	virtual void OnStartRec(wxCommandEvent& event) = 0;
	virtual void OnDelRec(wxCommandEvent& event) = 0;
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

	virtual void PopTopList();
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

private:
};
#endif//_MACHINELEARNINGDLG_H_