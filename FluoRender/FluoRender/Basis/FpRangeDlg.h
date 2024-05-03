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
#ifndef _FPRANGEDLG_H_
#define _FPRANGEDLG_H_

#include <wx/wx.h>
#include <wx/dialog.h>

class MainFrame;
class FpRangeDlg : public wxDialog
{
	enum
	{
		ID_MinValText = ID_FP_RANGE,
		ID_MaxValText
	};

public:
	FpRangeDlg(MainFrame* frame);
	~FpRangeDlg();

	void SetRange(double min_val, double max_val);
	double GetMinValue() { return m_min_val; }
	double GetMaxValue() { return m_max_val; }

private:
	MainFrame* m_frame;

	//values
	double m_min_val;
	double m_max_val;

	//text boxes
	wxTextCtrl* m_min_text;
	wxTextCtrl* m_max_text;

	//buttons
	wxButton *m_ok_btn;
	wxButton* m_cancel_btn;

	//text boxes
	void OnMinText(wxCommandEvent& event);
	void OnMaxText(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()
};

#endif//_FPRANGEDLG_H_
