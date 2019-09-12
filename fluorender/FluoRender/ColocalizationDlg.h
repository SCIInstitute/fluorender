/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2018 Scientific Computing and Imaging Institute,
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
#ifndef _COLOCALIZATIONDLG_H_
#define _COLOCALIZATIONDLG_H_

#include <wx/wx.h>
#include <wx/tglbtn.h>
#include <wx/grid.h>
#include <wx/clipbrd.h>

class DataGroup;

class ColocalizationDlg : public wxPanel
{
public:
	enum
	{
		ID_ColocalizeBtn = ID_COLOCALIZE,
		ID_UseSelChk,
		ID_AutoUpdateBtn,
		//settings
		ID_ProductRdb,
		ID_MinValueRdb,
		ID_LogicalAndRdb,
		//formats
		ID_IntWeightedRdb,
		ID_VoxCountRdb,
		ID_IntWghtRatioRdb,
		ID_VoxCountRatioRdb,

		//output
		ID_HistoryChk,
		ID_ClearHistBtn,
		ID_OutputGrid
	};

	ColocalizationDlg(wxWindow* frame,
		wxWindow* parent);
	~ColocalizationDlg();

	void SetGroup(DataGroup* group)
	{
		m_group = group;
	}

	//output
	void SetOutput(wxString &titles, wxString &values);
	void CopyData();
	void PasteData();

	//settings
	void GetSettings();

private:
	wxWindow* m_frame;
	wxString m_output_file;

	//current view
	DataGroup *m_group;

	//use selection
	bool m_use_mask;
	//format
	bool m_voxel_count;
	bool m_get_ratio;
	//output
	bool m_hold_history;

	//interface
	//colocalization
	wxButton *m_colocalize_btn;
	wxCheckBox* m_use_sel_chk;
	wxToggleButton* m_auto_update_btn;
	//settings
	wxRadioButton* m_product_rdb;
	wxRadioButton* m_min_value_rdb;
	wxRadioButton* m_logical_and_rdb;
	//format
	wxRadioButton* m_int_weighted_rdb;
	wxRadioButton* m_vox_count_rdb;
	wxRadioButton* m_int_wght_ratio_rdb;
	wxRadioButton* m_vox_count_ratio_rdb;

	//output
	wxCheckBox* m_history_chk;
	wxButton* m_clear_hist_btn;
	wxGrid *m_output_grid;

private:
	//calculate
	void OnColocalizenBtn(wxCommandEvent &event);
	void OnUseSelChk(wxCommandEvent &event);
	void OnAutoUpdate(wxCommandEvent &event);
	//settings
	void OnMethodRdb(wxCommandEvent &event);
	//format
	void OnFormatRdb(wxCommandEvent &event);

	//output
	void OnHistoryChk(wxCommandEvent& event);
	void OnClearHistBtn(wxCommandEvent& event);
	void OnKeyDown(wxKeyEvent& event);
	void OnSelectCell(wxGridEvent& event);
	void OnGridLabelClick(wxGridEvent& event);

	DECLARE_EVENT_TABLE()
};

#endif//_COLOCALIZATIONDLG_H_
