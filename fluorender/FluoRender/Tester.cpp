/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2014 Scientific Computing and Imaging Institute,
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
#include "Tester.h"
#include "VRenderFrame.h"

#include "Formats/tif_reader.h"

BEGIN_EVENT_TABLE(TesterDlg, wxDialog)
	//sliders
	EVT_COMMAND_SCROLL(ID_P1Slider, TesterDlg::OnP1Change)
	EVT_COMMAND_SCROLL(ID_P2Slider, TesterDlg::OnP2Change)
	EVT_COMMAND_SCROLL(ID_P3Slider, TesterDlg::OnP3Change)
	EVT_COMMAND_SCROLL(ID_P4Slider, TesterDlg::OnP4Change)
	//check boxes
	EVT_CHECKBOX(ID_P1Check, TesterDlg::OnP1Check)
	EVT_CHECKBOX(ID_P2Check, TesterDlg::OnP2Check)
	EVT_CHECKBOX(ID_P3Check, TesterDlg::OnP3Check)
	EVT_CHECKBOX(ID_P4Check, TesterDlg::OnP4Check)
	//all control
	EVT_CHECKBOX(ID_AllCheck, TesterDlg::OnAllCheck)
	//buttons
	EVT_BUTTON(ID_B1Btn, TesterDlg::OnB1)
END_EVENT_TABLE()

TesterDlg::TesterDlg(wxWindow *frame, wxWindow *parent)
: wxDialog(parent, wxID_ANY, wxString("Tester"),
		   wxDefaultPosition, wxSize(600, 600),
		   wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|
		   wxMAXIMIZE_BOX|wxMINIMIZE_BOX),
m_p1(1.0),
m_p2(0.0),
m_p3(0.0),
m_p4(0.0)//,
//m_frame(frame)
{
/*	wxStaticText *st;

	//p1
	wxBoxSizer *sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "resizing",
		wxDefaultPosition, wxSize(50, 20));
	m_p1_sldr = new wxSlider(this, ID_P1Slider, 250, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_p1_text = new wxTextCtrl(this, ID_P1Text, "1.0",
		wxDefaultPosition, wxSize(40, 20), wxTE_PROCESS_ENTER);
	m_p1_chck = new wxCheckBox(this, ID_P1Check, "",
		wxDefaultPosition, wxDefaultSize);
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	sizer_1->Add(m_p1_sldr, 1, wxEXPAND);
	sizer_1->Add(m_p1_text, 0, wxALIGN_CENTER);
	sizer_1->Add(m_p1_chck, 0, wxALIGN_CENTER);

	//p2
	wxBoxSizer *sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "min filter",
		wxDefaultPosition, wxSize(50, 20));
	m_p2_sldr = new wxSlider(this, ID_P2Slider, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_p2_text = new wxTextCtrl(this, ID_P2Text, "0",
		wxDefaultPosition, wxSize(40, 20), wxTE_PROCESS_ENTER);
	m_p2_chck = new wxCheckBox(this, ID_P2Check, "",
		wxDefaultPosition, wxDefaultSize);
	sizer_2->Add(st, 0, wxALIGN_CENTER);
	sizer_2->Add(m_p2_sldr, 1, wxEXPAND);
	sizer_2->Add(m_p2_text, 0, wxALIGN_CENTER);
	sizer_2->Add(m_p2_chck, 0, wxALIGN_CENTER);

	//p3
	wxBoxSizer *sizer_3 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "max filter",
		wxDefaultPosition, wxSize(50, 20));
	m_p3_sldr = new wxSlider(this, ID_P3Slider, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_p3_text = new wxTextCtrl(this, ID_P3Text, "0",
		wxDefaultPosition, wxSize(40, 20), wxTE_PROCESS_ENTER);
	m_p3_chck = new wxCheckBox(this, ID_P3Check, "",
		wxDefaultPosition, wxDefaultSize);
	sizer_3->Add(st, 0, wxALIGN_CENTER);
	sizer_3->Add(m_p3_sldr, 1, wxEXPAND);
	sizer_3->Add(m_p3_text, 0, wxALIGN_CENTER);
	sizer_3->Add(m_p3_chck, 0, wxALIGN_CENTER);

	//p4
	wxBoxSizer *sizer_4 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "shp filter",
		wxDefaultPosition, wxSize(50, 20));
	m_p4_sldr = new wxSlider(this, ID_P4Slider, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_p4_text = new wxTextCtrl(this, ID_P4Text, "0",
		wxDefaultPosition, wxSize(40, 20), wxTE_PROCESS_ENTER);
	m_p4_chck = new wxCheckBox(this, ID_P4Check, "",
		wxDefaultPosition, wxDefaultSize);
	sizer_4->Add(st, 0, wxALIGN_CENTER);
	sizer_4->Add(m_p4_sldr, 1, wxEXPAND);
	sizer_4->Add(m_p4_text, 0, wxALIGN_CENTER);
	sizer_4->Add(m_p4_chck, 0, wxALIGN_CENTER);

	//all control
	wxBoxSizer *sizer_a = new wxBoxSizer(wxHORIZONTAL);
	m_all_chk = new wxCheckBox(this, ID_AllCheck, "All",
		wxDefaultPosition, wxDefaultSize);
	sizer_a->AddStretchSpacer(1);
	sizer_a->Add(m_all_chk, 0, wxALIGN_CENTER);

	//sizer
	wxBoxSizer *sizer_v = new wxBoxSizer(wxVERTICAL);
	sizer_v->Add(sizer_1, 0, wxEXPAND);
	sizer_v->Add(sizer_2, 0, wxEXPAND);
	sizer_v->Add(sizer_3, 0, wxEXPAND);
	sizer_v->Add(sizer_4, 0, wxEXPAND);
	sizer_v->Add(sizer_a, 0, wxEXPAND);

	SetSizer(sizer_v);
*/	

	wxBoxSizer *sizer_v = new wxBoxSizer(wxVERTICAL);
	m_b1_btn = new wxButton(this, ID_B1Btn, "Click Me");
	sizer_v->Add(m_b1_btn, 0);

	SetSizer(sizer_v);
	Layout();

}

TesterDlg::~TesterDlg()
{
}

//sliders
void TesterDlg::OnP1Change(wxScrollEvent &event)
{
/*	int ival = event.GetPosition();
	double val = ival*4.0/1000.0;
	VolumeData* vd = ((VRenderFrame*)m_frame)->GetDataManager()->GetVolumeData(0);
	if (vd)
	{
		vd->GetVR()->sfactor_ = val;
		vd->GetVR()->resize();
		((VRenderFrame*)m_frame)->RefreshVRenderViews();
		m_p1 = val;
	}

	wxString str = wxString::Format("%f", val);
	m_p1_text->SetValue(str);
*/
}

void TesterDlg::OnP2Change(wxScrollEvent &event)
{
/*	int ival = event.GetPosition();
	double val = ival*4.0/1000.0;
	VolumeData* vd = ((VRenderFrame*)m_frame)->GetDataManager()->GetVolumeData(0);
	if (vd)
	{
		vd->GetVR()->filter_size_min_ = val;
		((VRenderFrame*)m_frame)->RefreshVRenderViews();
		m_p2 = val;//
	}

	wxString str = wxString::Format("%f", val);
	m_p2_text->SetValue(str);
*/
}

void TesterDlg::OnP3Change(wxScrollEvent &event)
{
/*	int ival = event.GetPosition();
	double val = ival*4.0/1000.0;
	VolumeData* vd = ((VRenderFrame*)m_frame)->GetDataManager()->GetVolumeData(0);
	if (vd)
	{
		vd->GetVR()->filter_size_max_ = val;
		((VRenderFrame*)m_frame)->RefreshVRenderViews();
		m_p3 = val;
	}

	wxString str = wxString::Format("%f", val);
	m_p3_text->SetValue(str);
*/
}

void TesterDlg::OnP4Change(wxScrollEvent &event)
{
/*	int ival = event.GetPosition();
	double val = ival*4.0/1000.0;
	VolumeData* vd = ((VRenderFrame*)m_frame)->GetDataManager()->GetVolumeData(0);
	if (vd)
	{
		vd->GetVR()->filter_size_shp_ = val;
		((VRenderFrame*)m_frame)->RefreshVRenderViews();
		m_p4 = val;
	}

	wxString str = wxString::Format("%f", val);
	m_p4_text->SetValue(str);
*/
}

void TesterDlg::OnP1Check(wxCommandEvent& event)
{
/*	double val = 0.0;
	if (m_p1_chck->GetValue())
		val = 1.0;
	else
		val = m_p1;

	VolumeData* vd = ((VRenderFrame*)m_frame)->GetDataManager()->GetVolumeData(0);
	if (vd)
	{
		vd->GetVR()->sfactor_ = val;
		vd->GetVR()->resize();
		((VRenderFrame*)m_frame)->RefreshVRenderViews();
	}

	wxString str = wxString::Format("%f", val);
	m_p1_text->SetValue(str);
*/
}

void TesterDlg::OnP2Check(wxCommandEvent& event)
{
/*	double val = 0.0;
	if (m_p2_chck->GetValue())
		val = 0.0;
	else
		val = m_p2;

	VolumeData* vd = ((VRenderFrame*)m_frame)->GetDataManager()->GetVolumeData(0);
	if (vd)
	{
		vd->GetVR()->filter_size_min_ = val;
		((VRenderFrame*)m_frame)->RefreshVRenderViews();
	}

	wxString str = wxString::Format("%f", val);
	m_p2_text->SetValue(str);
*/
}

void TesterDlg::OnP3Check(wxCommandEvent& event)
{
/*	double val = 0.0;
	if (m_p3_chck->GetValue())
		val = 0.0;
	else
		val = m_p3;

	VolumeData* vd = ((VRenderFrame*)m_frame)->GetDataManager()->GetVolumeData(0);
	if (vd)
	{
		vd->GetVR()->filter_size_max_ = val;
		((VRenderFrame*)m_frame)->RefreshVRenderViews();
	}

	wxString str = wxString::Format("%f", val);
	m_p3_text->SetValue(str);
*/
}

void TesterDlg::OnP4Check(wxCommandEvent& event)
{
/*	double val = 0.0;
	if (m_p4_chck->GetValue())
		val = 0.0;
	else
		val = m_p4;

	VolumeData* vd = ((VRenderFrame*)m_frame)->GetDataManager()->GetVolumeData(0);
	if (vd)
	{
		vd->GetVR()->filter_size_shp_ = val;
		((VRenderFrame*)m_frame)->RefreshVRenderViews();
	}

	wxString str = wxString::Format("%f", val);
	m_p4_text->SetValue(str);
*/
}

void TesterDlg::OnAllCheck(wxCommandEvent& event)
{
/*	double val1 = 0.0;
	double val2 = 0.0;
	double val3 = 0.0;
	double val4 = 0.0;
	if (m_all_chk->GetValue())
	{
		val1 = 1.0;
		val2 = 0.0;
		val3 = 0.0;
		val4 = 0.0;
	}
	else
	{
		val1 = m_p1;
		val2 = m_p2;
		val3 = m_p3;
		val4 = m_p4;
	}

	VolumeData* vd = ((VRenderFrame*)m_frame)->GetDataManager()->GetVolumeData(0);
	if (vd)
	{
		vd->GetVR()->sfactor_ = val1;
		vd->GetVR()->filter_size_min_ = val2;
		vd->GetVR()->filter_size_max_ = val3;
		vd->GetVR()->filter_size_shp_ = val4;
		vd->GetVR()->resize();
		((VRenderFrame*)m_frame)->RefreshVRenderViews();
	}
*/
}

void TesterDlg::OnB1(wxCommandEvent& event)
{
	//TIFReader reader;
	//reader.SetFile(string("D:\\DATA\\002\\isl1actinCy3Top3_actin.tif"));
	////reader.SetSliceSeq(true);
	//reader.Preprocess();
	////reader.SetBatch(true);
	//Nrrd* data = reader.Convert();
}
