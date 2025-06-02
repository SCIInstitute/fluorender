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
#ifndef _COLOCALIZATIONDLG_H_
#define _COLOCALIZATIONDLG_H_

#include <PropPanel.h>
#include <wx/tglbtn.h>
#include <wx/grid.h>
#include <wx/clipbrd.h>

class ColocalizationDlg : public PropPanel
{
public:
	ColocalizationDlg(MainFrame* frame);
	~ColocalizationDlg();

	virtual void FluoUpdate(const fluo::ValueCollection& vc = {});
	void SetOutput();

	void CopyData();
	void PasteData();

private:
	//output
	bool m_hold_history;
	wxString m_output_file;

	//interface
	//colocalization
	wxButton *m_colocalize_btn;
	wxCheckBox* m_use_sel_chk;
	//settings
	wxRadioButton* m_product_rdb;
	wxRadioButton* m_min_value_rdb;
	wxRadioButton* m_logical_and_rdb;
	//format
	wxToggleButton* m_int_weight_btn;
	wxToggleButton* m_ratio_btn;
	wxToggleButton* m_physical_btn;
	wxToggleButton* m_colormap_btn;

	//output
	wxCheckBox* m_history_chk;
	wxButton* m_clear_hist_btn;
	wxGrid *m_output_grid;

private:
	//calculate
	void OnColocalizenBtn(wxCommandEvent& event);
	void OnUseSelChk(wxCommandEvent& event);
	//settings
	void OnMethodRdb(wxCommandEvent& event);
	//format
	void OnIntWeightBtn(wxCommandEvent& event);
	void OnRatioBtn(wxCommandEvent& event);
	void OnPhysicalBtn(wxCommandEvent& event);
	void OnColorMapBtn(wxCommandEvent& event);

	//output
	void OnHistoryChk(wxCommandEvent& event);
	void OnClearHistBtn(wxCommandEvent& event);
	void OnKeyDown(wxKeyEvent& event);
	void OnSelectCell(wxGridEvent& event);
	void OnGridLabelClick(wxGridEvent& event);
	//resize
	void OnSize(wxSizeEvent& event);
};

#endif//_COLOCALIZATIONDLG_H_
