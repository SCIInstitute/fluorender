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
#ifndef _COUNTINGDLG_H_
#define _COUNTINGDLG_H_

#include <PropPanel.h>

class CountingDlg : public PropPanel
{
public:
	CountingDlg(MainFrame* frame);
	~CountingDlg();

	virtual void FluoUpdate(const fluo::ValueCollection& vc = {});
	void OutputSize();

private:
	//max volume value
	double m_max_value;

	//component analyzer
	wxCheckBox *m_ca_select_only_chk;
	wxTextCtrl *m_ca_min_text;
	wxTextCtrl *m_ca_max_text;
	wxCheckBox *m_ca_ignore_max_chk;
	wxButton *m_ca_analyze_btn;
	wxTextCtrl *m_ca_comps_text;
	wxTextCtrl *m_ca_volume_text;
	wxTextCtrl *m_ca_vol_unit_text;

private:
	//component analyzer
	void OnUseSelChk(wxCommandEvent& event);
	void OnMinText(wxCommandEvent& event);
	void OnMaxText(wxCommandEvent& event);
	void OnIgnoreMaxChk(wxCommandEvent& event);
	void OnAnalyzeBtn(wxCommandEvent& event);
};

#endif//_COUNTINGDLG_H_
