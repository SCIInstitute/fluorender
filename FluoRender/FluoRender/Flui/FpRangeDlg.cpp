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
#include <FpRangeDlg.h>
#include <Global.h>
#include <Names.h>
#include <MainSettings.h>
#include <MainFrame.h>
#include <wx/valnum.h>

//BEGIN_EVENT_TABLE(FpRangeDlg, wxDialog)
////text boxes
//	EVT_TEXT(ID_MinValText, FpRangeDlg::OnMinText)
//	EVT_TEXT(ID_MaxValText, FpRangeDlg::OnMaxText)
//END_EVENT_TABLE()

FpRangeDlg::FpRangeDlg(MainFrame *frame)
: PropDialog(frame, frame,
	wxDefaultPosition,
	frame->FromDIP(wxSize(400, 200)),
	wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER |
	wxMAXIMIZE_BOX | wxMINIMIZE_BOX | wxSTAY_ON_TOP,
	wxString("Data Conversion"))
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
	m_min_text = new wxTextCtrl(this, wxID_ANY, "0.0",
		wxDefaultPosition, wxDefaultSize , wxTE_RIGHT, vald_fp);
	m_min_text->Bind(wxEVT_TEXT, &FpRangeDlg::OnMinText, this);
	sizer2->Add(10, 10);
	sizer2->Add(st, 0, wxALIGN_CENTER);
	sizer2->Add(10, 10);
	sizer2->Add(m_min_text, 0, wxALIGN_CENTER);
	st = new wxStaticText(this, 0, "Maximum:",
		wxDefaultPosition, wxDefaultSize);
	m_max_text = new wxTextCtrl(this, wxID_ANY, "1.0",
		wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, vald_fp);
	m_max_text->Bind(wxEVT_TEXT, &FpRangeDlg::OnMaxText, this);
	sizer2->Add(10, 10);
	sizer2->Add(st, 0, wxALIGN_CENTER);
	sizer2->Add(10, 10);
	sizer2->Add(m_max_text, 0, wxALIGN_CENTER);

	//buttons
	wxBoxSizer* sizer3 = new wxBoxSizer(wxHORIZONTAL);
	m_ok_btn = new wxButton(this, wxID_ANY, "Yes",
		wxDefaultPosition, wxDefaultSize);
	m_cancel_btn = new wxButton(this, wxID_ANY, "Use Default",
		wxDefaultPosition, wxDefaultSize);
	m_ok_btn->Bind(wxEVT_BUTTON, &FpRangeDlg::OnOkBtn, this);
	m_cancel_btn->Bind(wxEVT_BUTTON, &FpRangeDlg::OnCancelBtn, this);
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

	Bind(wxEVT_SHOW, &FpRangeDlg::OnShow, this);
}

FpRangeDlg::~FpRangeDlg()
{
}

void FpRangeDlg::FluoUpdate(const fluo::ValueCollection& vc)
{
	//update user interface
	if (FOUND_VALUE(gstNull))
		return;
	bool update_all = vc.empty();

	wxString str;

	if (update_all || FOUND_VALUE(gstFpRangeMin))
	{
		m_fp_min = glbin_settings.m_fp_min;
		str = wxString::Format("%.2f", m_fp_min);
		m_min_text->ChangeValue(str);
	}
	if (update_all || FOUND_VALUE(gstFpRangeMax))
	{
		m_fp_max = glbin_settings.m_fp_max;
		str = wxString::Format("%.2f", m_fp_max);
		m_max_text->ChangeValue(str);
	}
}

void FpRangeDlg::OnMinText(wxCommandEvent& event)
{
	wxString str = m_min_text->GetValue();
	double dval;
	if (str.ToDouble(&dval))
		m_fp_min = dval;
}

void FpRangeDlg::OnMaxText(wxCommandEvent& event)
{
	wxString str = m_max_text->GetValue();
	double dval;
	if (str.ToDouble(&dval))
		m_fp_max = dval;
}

void FpRangeDlg::OnShow(wxShowEvent& event)
{
	FluoUpdate({});
}

void FpRangeDlg::OnOkBtn(wxCommandEvent& event)
{
	glbin_settings.m_fp_min = m_fp_min;
	glbin_settings.m_fp_max = m_fp_max;
	Hide();
}

void FpRangeDlg::OnCancelBtn(wxCommandEvent& event)
{
	Hide();
}
