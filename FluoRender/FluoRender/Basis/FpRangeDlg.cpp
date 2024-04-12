/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2023 Scientific Computing and Imaging Institute,
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
#include "FpRangeDlg.h"
#include <MainFrame.h>
#include <wx/valnum.h>

BEGIN_EVENT_TABLE(FpRangeDlg, wxDialog)
//text boxes
	EVT_TEXT(ID_MinValText, FpRangeDlg::OnMinText)
	EVT_TEXT(ID_MaxValText, FpRangeDlg::OnMaxText)
END_EVENT_TABLE()

FpRangeDlg::FpRangeDlg(MainFrame *frame)
: wxDialog(frame, wxID_ANY, wxString("Data Conversion"),
	wxDefaultPosition,
	frame->FromDIP(wxSize(400, 200)),
	wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|
	wxMAXIMIZE_BOX|wxMINIMIZE_BOX| wxSTAY_ON_TOP),
m_min_val(0),
m_max_val(0),
m_frame(frame)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);
	SetDoubleBuffered(true);

	wxStaticText *st;
	//validator: floating point 1
	wxFloatingPointValidator<double> vald_fp(10);

	wxBoxSizer* sizer1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Converting floating-point data. Use the conversion range below?");
	sizer1->Add(10, 10);
	sizer1->Add(st, 0, wxALIGN_CENTER);

	//text controls
	wxBoxSizer *sizer2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Minimum:",
		wxDefaultPosition, wxDefaultSize);
	m_min_text = new wxTextCtrl(this, ID_MinValText, "0.0",
		wxDefaultPosition, wxDefaultSize , wxTE_RIGHT, vald_fp);
	sizer2->Add(10, 10);
	sizer2->Add(st, 0, wxALIGN_CENTER);
	sizer2->Add(10, 10);
	sizer2->Add(m_min_text, 0, wxALIGN_CENTER);
	st = new wxStaticText(this, 0, "Maximum:",
		wxDefaultPosition, wxDefaultSize);
	m_max_text = new wxTextCtrl(this, ID_MaxValText, "1.0",
		wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, vald_fp);
	sizer2->Add(10, 10);
	sizer2->Add(st, 0, wxALIGN_CENTER);
	sizer2->Add(10, 10);
	sizer2->Add(m_max_text, 0, wxALIGN_CENTER);

	//buttons
	wxBoxSizer* sizer3 = new wxBoxSizer(wxHORIZONTAL);
	m_ok_btn = new wxButton(this, wxID_OK, "Yes",
		wxDefaultPosition, wxDefaultSize);
	m_cancel_btn = new wxButton(this, wxID_CANCEL, "Use Default",
		wxDefaultPosition, wxDefaultSize);
	sizer3->AddStretchSpacer(1);
	sizer3->Add(m_ok_btn, 0, wxALIGN_CENTER);
	sizer3->Add(5, 5);
	sizer3->Add(m_cancel_btn, 0, wxALIGN_CENTER);
	sizer3->AddStretchSpacer(1);

	//all
	wxBoxSizer *sizer_v = new wxBoxSizer(wxVERTICAL);
	sizer_v->Add(10, 10);
	sizer_v->Add(sizer1, 0, wxEXPAND);
	sizer_v->Add(10, 10);
	sizer_v->Add(sizer2, 0, wxEXPAND);
	sizer_v->AddStretchSpacer(1);
	sizer_v->Add(sizer3, 0, wxEXPAND);
	sizer_v->Add(10, 10);

	SetSizer(sizer_v);
	Layout();

}

FpRangeDlg::~FpRangeDlg()
{
}

void FpRangeDlg::SetRange(double min_val, double max_val)
{
	m_min_val = min_val;
	m_max_val = max_val;
	wxString str = wxString::Format("%f", m_min_val);
	m_min_text->ChangeValue(str);
	str = wxString::Format("%f", m_max_val);
	m_max_text->ChangeValue(str);
}

void FpRangeDlg::OnMinText(wxCommandEvent& event)
{
	wxString str = m_min_text->GetValue();
	double dval;
	if (str.ToDouble(&dval))
		m_min_val = dval;
}

void FpRangeDlg::OnMaxText(wxCommandEvent& event)
{
	wxString str = m_max_text->GetValue();
	double dval;
	if (str.ToDouble(&dval))
		m_max_val = dval;
}