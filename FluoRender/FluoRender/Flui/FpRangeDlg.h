﻿/*
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
#ifndef _FPRANGEDLG_H_
#define _FPRANGEDLG_H_

#include <PropPanel.h>

class FpRangeDlg : public PropDialog
{
public:
	FpRangeDlg(MainFrame* frame);
	~FpRangeDlg();

	virtual void FluoUpdate(const fluo::ValueCollection& vc = {});

private:
	double m_fp_min;
	double m_fp_max;

	//text boxes
	wxTextCtrl* m_min_text;
	wxTextCtrl* m_max_text;

	//buttons
	wxButton *m_ok_btn;
	wxButton* m_cancel_btn;

	//text boxes
	void OnMinText(wxCommandEvent& event);
	void OnMaxText(wxCommandEvent& event);
	void OnShow(wxShowEvent& event);
	void OnOkBtn(wxCommandEvent& event);
	void OnCancelBtn(wxCommandEvent& event);
};

#endif//_FPRANGEDLG_H_
