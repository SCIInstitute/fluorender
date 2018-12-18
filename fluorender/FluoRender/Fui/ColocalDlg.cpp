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
#include "ColocalDlg.h"
#include <VRenderFrame.h>
#include <Scenegraph/VolumeData.h>
#include <Global/Global.h>
#include <wx/valnum.h>

using namespace FUI;

BEGIN_EVENT_TABLE(ColocalDlg, wxPanel)
	EVT_COMMAND_SCROLL(ID_MinSizeSldr, ColocalDlg::OnMinSizeChange)
	EVT_TEXT(ID_MinSizeText, ColocalDlg::OnMinSizeText)
	EVT_COMMAND_SCROLL(ID_MaxSizeSldr, ColocalDlg::OnMaxSizeChange)
	EVT_TEXT(ID_MaxSizeText, ColocalDlg::OnMaxSizeText)
	EVT_CHECKBOX(ID_BrushSelectBothChk, ColocalDlg::OnSelectBothChk)
	EVT_BUTTON(ID_CalcColocalizationBtn, ColocalDlg::OnColocalizationBtn)
END_EVENT_TABLE()

ColocalDlg::ColocalDlg(
	wxWindow* parent) :
wxPanel(parent, wxID_ANY,
wxDefaultPosition, wxSize(400, 165),
0, "ColocalDlg")
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);

	m_agent =
		FL::Global::instance().getAgentFactory().
		getOrAddColocalAgent("ColocalDlg", *this);

	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;

	wxStaticText *st = 0;

	//operand A
	wxBoxSizer *sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Volume A:",
		wxDefaultPosition, wxSize(75, 20));
	m_calc_load_a_btn = new wxButton(this, ID_CalcLoadABtn, "Load",
		wxDefaultPosition,wxSize(75, 20));
	m_calc_a_text = new wxTextCtrl(this, ID_CalcAText, "",
		wxDefaultPosition, wxSize(200, 20));
	sizer_1->Add(10, 10);
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	sizer_1->Add(m_calc_load_a_btn, 0, wxALIGN_CENTER);
	sizer_1->Add(m_calc_a_text, 1, wxEXPAND);
	sizer_1->Add(10, 10);
	//operand B
	wxBoxSizer *sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Volume B:",
		wxDefaultPosition, wxSize(75, 20));
	m_calc_load_b_btn = new wxButton(this, ID_CalcLoadBBtn, "Load",
		wxDefaultPosition, wxSize(75, 20));
	m_calc_b_text = new wxTextCtrl(this, ID_CalcBText, "",
		wxDefaultPosition, wxSize(75, 20));
	sizer_2->Add(10, 10);
	sizer_2->Add(st, 0, wxALIGN_CENTER);
	sizer_2->Add(m_calc_load_b_btn, 0, wxALIGN_CENTER);
	sizer_2->Add(m_calc_b_text, 1, wxEXPAND);
	sizer_2->Add(10, 10);
	//min size
	wxBoxSizer *sizer_3 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Min Size:",
		wxDefaultPosition, wxSize(75, 20));
	m_min_size_sldr = new wxSlider(this, ID_MinSizeSldr, 10, 0, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_min_size_text = new wxTextCtrl(this, ID_MinSizeText, "10",
		wxDefaultPosition, wxSize(75, 20), 0, vald_int);
	sizer_3->Add(10, 10);
	sizer_3->Add(st, 0, wxALIGN_CENTER);
	sizer_3->Add(m_min_size_sldr, 1, wxEXPAND);
	sizer_3->Add(m_min_size_text, 0, wxALIGN_CENTER);
	sizer_3->Add(10, 10);
	//max size
	wxBoxSizer *sizer_4 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Max Size:",
		wxDefaultPosition, wxSize(75, 20));
	m_max_size_sldr = new wxSlider(this, ID_MaxSizeSldr, 101, 1, 101,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_max_size_text = new wxTextCtrl(this, ID_MaxSizeText, "Ignored",
		wxDefaultPosition, wxSize(75, 20), 0, vald_int);
	sizer_4->Add(10, 10);
	sizer_4->Add(st, 0, wxALIGN_CENTER);
	sizer_4->Add(m_max_size_sldr, 1, wxEXPAND);
	sizer_4->Add(m_max_size_text, 0, wxALIGN_CENTER);
	sizer_4->Add(10, 10);
	//select and calculate
	wxBoxSizer *sizer_5 = new wxBoxSizer(wxHORIZONTAL);
	m_select_both_chk = new wxCheckBox(this, ID_BrushSelectBothChk, "Select Both: ",
		wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_colocalization_btn = new wxButton(this, ID_CalcColocalizationBtn, "Colocalization",
		wxDefaultPosition, wxDefaultSize);
	sizer_5->Add(10, 10);
	sizer_5->Add(m_select_both_chk, 0, wxALIGN_CENTER);
	sizer_5->AddStretchSpacer(1);
	sizer_5->Add(m_colocalization_btn, 0, wxALIGN_CENTER);
	sizer_5->Add(10, 10);

	wxBoxSizer *sizerV = new wxBoxSizer(wxVERTICAL);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_1, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_2, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_3, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_4, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_5, 0, wxEXPAND);
	
	SetSizer(sizerV);
	Layout();
}

ColocalDlg::~ColocalDlg()
{
}

void ColocalDlg::OnMinSizeChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	wxString str = wxString::Format("%d", ival);
	m_min_size_text->SetValue(str);
}

void ColocalDlg::OnMinSizeText(wxCommandEvent &event)
{
	wxString str = m_min_size_text->GetValue();
	long ival;
	str.ToLong(&ival);
	m_min_size_sldr->SetValue(ival);
}

void ColocalDlg::OnMaxSizeChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	wxString str = wxString::Format("%d", ival*100);
	if (ival == 101)
		str = "Ignored";
	m_max_size_text->SetValue(str);
}

void ColocalDlg::OnMaxSizeText(wxCommandEvent &event)
{
	wxString str = m_max_size_text->GetValue();
	long ival;
	if (str == "Ignored")
		ival = 10100;
	else
		str.ToLong(&ival);
	m_max_size_sldr->SetValue(ival/100);
}

void ColocalDlg::OnSelectBothChk(wxCommandEvent &event)
{
	bool select_both = m_select_both_chk->GetValue();

	//if (m_view)
	//{
	//	m_view->SetSelectBoth(select_both);
	//	if (select_both)
	//	{
	//		m_view->SetVolumeA(m_vol_a);
	//		m_view->SetVolumeB(m_vol_b);
	//	}
	//}
}

void ColocalDlg::OnColocalizationBtn(wxCommandEvent &event)
{

	m_agent->Run();
/*	if (!m_view || !m_vol_a || !m_vol_b)
		return;

	bool select = true;
	wxString str = m_min_size_text->GetValue();
	long ival;
	str.ToLong(&ival);
	double min_voxels = ival;
	str = m_max_size_text->GetValue();
	if (str == "Ignored")
		ival = -1;
	else
		str.ToLong(&ival);
	double max_voxels = ival;
	double thresh = 0.0;
	double falloff = 1.0;

	//save masks
	m_vol_a->GetRenderer()->return_mask();
	m_vol_b->GetRenderer()->return_mask();

	//volume a
	m_view->GetVolumeSelector()->SetVolume(m_vol_a);
	m_view->CompAnalysis(min_voxels, max_voxels, thresh, falloff, select, true, false);

	m_view->m_glview->ForceDraw();

	//volume b
	m_view->GetVolumeSelector()->SetVolume(m_vol_b);
	m_view->CompAnalysis(min_voxels, max_voxels, thresh, falloff, select, true, false);

	m_view->m_glview->ForceDraw();

	m_vol_a->setValue("display", false);
	m_vol_b->setValue("display", false);

	//volume c
	m_view->GetVolumeCalculator()->SetVolumeA(m_vol_a);
	m_view->GetVolumeCalculator()->SetVolumeB(m_vol_b);
	m_view->Calculate(8);
	FL::VolumeData* vd = m_view->GetVolumeCalculator()->GetResult();
	if (vd)
	{
		select = false;
		m_view->GetVolumeSelector()->SetVolume(vd);
		m_view->CompAnalysis(min_voxels, max_voxels, thresh, falloff, select, true, false);
	}
*/
}