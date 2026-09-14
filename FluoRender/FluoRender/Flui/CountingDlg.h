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
#ifndef _COUNTINGDLG_H_
#define _COUNTINGDLG_H_

#include <PropPanel.h>
#include <GridData.h>
#include <wx/grid.h>

class CountingDlg : public TabbedPanel
{
public:
	CountingDlg(wxWindow* parent);
	~CountingDlg();

	//update
	void UpdateUseSelection(bool bval);
	void UpdateCountMinValue(int ival);
	void UpdateCountMaxValue(int ival);
	void UpdateCountUseMax(bool bval);

	//output
	void CopyData();
	void UpdateGrid(const GridData& data);

private:
	//output
	bool m_hold_history = false;

	//component analyzer
	wxCheckBox *m_ca_select_only_chk;
	wxTextCtrl *m_ca_min_text;
	wxTextCtrl *m_ca_max_text;
	wxCheckBox *m_ca_ignore_max_chk;
	wxButton *m_ca_analyze_btn;

	//output
	wxCheckBox* m_history_chk;
	wxButton* m_clear_hist_btn;
	wxGrid* m_output_grid;

private:
	wxWindow* CreateSettingPage(wxWindow* parent);
	wxWindow* CreateInfoPage(wxWindow* parent);

	//component analyzer
	void OnUseSelChk(wxCommandEvent& event);
	void OnMinText(wxCommandEvent& event);
	void OnMaxText(wxCommandEvent& event);
	void OnIgnoreMaxChk(wxCommandEvent& event);
	void OnAnalyzeBtn(wxCommandEvent& event);

	//output
	void OnHistoryChk(wxCommandEvent& event);
	void OnClearHistBtn(wxCommandEvent& event);
	void OnKeyDown(wxKeyEvent& event);
	void OnSelectCell(wxGridEvent& event);
	//resize
	void OnSize(wxSizeEvent& event);
};

#endif//_COUNTINGDLG_H_
