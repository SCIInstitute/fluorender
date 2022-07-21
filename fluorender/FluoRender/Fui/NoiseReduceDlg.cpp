/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2022 Scientific Computing and Imaging Institute,
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
#include <NoiseReduceDlg.h>
#include <RenderFrame.h>
#include <wx/valnum.h>

BEGIN_EVENT_TABLE(NoiseReduceDlg, wxPanel)
	EVT_COMMAND_SCROLL(ID_ThresholdSldr, NoiseReduceDlg::OnThresholdChange)
	EVT_TEXT(ID_ThresholdText, NoiseReduceDlg::OnThresholdText)
	EVT_COMMAND_SCROLL(ID_VoxelSldr, NoiseReduceDlg::OnVoxelChange)
	EVT_TEXT(ID_VoxelText, NoiseReduceDlg::OnVoxelText)
	EVT_BUTTON(ID_PreviewBtn, NoiseReduceDlg::OnPreviewBtn)
	EVT_BUTTON(ID_EraseBtn, NoiseReduceDlg::OnEraseBtn)
	EVT_CHECKBOX(ID_EnhanceSelChk, NoiseReduceDlg::OnEnhanceSelChk)
END_EVENT_TABLE()

NoiseReduceDlg::NoiseReduceDlg(RenderFrame *frame)
: wxPanel(frame, wxID_ANY,
	wxDefaultPosition, wxSize(400, 150),
	0, "NoiseReduceDlg"),
	m_agent(nullptr)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);

	SetDoubleBuffered(true);

	//validator: floating point 1
	wxFloatingPointValidator<double> vald_fp1(1);
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;

	wxStaticText *st = 0;

	//group1
	wxBoxSizer *sizer1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Threshold:",
		wxDefaultPosition, wxSize(75, 23));
	m_threshold_sldr = new wxSlider(this, ID_ThresholdSldr, 0, 0, 2550,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_threshold_text = new wxTextCtrl(this, ID_ThresholdText, "0.0",
		wxDefaultPosition, wxSize(40, 20), 0, vald_fp1);
	m_preview_btn = new wxButton(this, ID_PreviewBtn, "Preview",
		wxDefaultPosition, wxSize(70, 23));
	sizer1->Add(st, 0, wxALIGN_CENTER);
	sizer1->Add(m_threshold_sldr, 1, wxEXPAND);
	sizer1->Add(m_threshold_text, 0, wxALIGN_CENTER);
	sizer1->Add(5, 5);
	sizer1->Add(m_preview_btn, 0, wxALIGN_CENTER);

	//group2
	wxBoxSizer *sizer2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Voxel Size:",
		wxDefaultPosition, wxSize(75, 23));
	m_voxel_sldr = new wxSlider(this, ID_VoxelSldr, 1, 1, 500,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_voxel_text = new wxTextCtrl(this, ID_VoxelText, "1",
		wxDefaultPosition, wxSize(40, 20), 0, vald_int);
	m_erase_btn = new wxButton(this, ID_EraseBtn, "Erase",
		wxDefaultPosition, wxSize(70, 23));
	sizer2->Add(st, 0, wxALIGN_CENTER);
	sizer2->Add(m_voxel_sldr, 1, wxEXPAND);
	sizer2->Add(m_voxel_text, 0, wxALIGN_CENTER);
	sizer2->Add(5, 5);
	sizer2->Add(m_erase_btn, 0, wxALIGN_CENTER);

	//group3
	wxBoxSizer *sizer3 = new wxBoxSizer(wxHORIZONTAL);
	m_ca_select_only_chk = new wxCheckBox(this, ID_CASelectOnlyChk, "Selct. Only",
		wxDefaultPosition, wxSize(95, 20));
	sizer3->Add(m_ca_select_only_chk, 0, wxALIGN_CENTER);

	//group4
	wxBoxSizer *sizer4 = new wxBoxSizer(wxHORIZONTAL);
	m_enhance_sel_chk = new wxCheckBox(this, ID_EnhanceSelChk, "Enhance selection",
		wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	sizer4->Add(m_enhance_sel_chk, 0, wxALIGN_CENTER);

	//group5
	wxBoxSizer *sizer5 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0,
		"Check this option if selections are too dim. It is equivalent to\n"\
		"adjusting the Equalization values in the Output Adjustment\n panel.",
		wxDefaultPosition, wxDefaultSize);
	sizer5->Add(st, 0, wxALIGN_CENTER);

	//all controls
	wxBoxSizer *sizerV = new wxBoxSizer(wxVERTICAL);
	sizerV->Add(10, 10);
	sizerV->Add(sizer1, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(sizer2, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(sizer3, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(sizer4, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(sizer5, 0, wxEXPAND);

	SetSizer(sizerV);
	Layout();
}

NoiseReduceDlg::~NoiseReduceDlg()
{
}

void NoiseReduceDlg::OnSelectOnlyChk(wxCommandEvent &event)
{
	bool bval = m_ca_select_only_chk->GetValue();
	m_agent->setValue(gstUseSelection, bval);
}

//threshold
void NoiseReduceDlg::OnThresholdChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	wxString str = wxString::Format("%.1f", double (ival)/10.0);
	if (str != m_threshold_text->GetValue())
		m_threshold_text->SetValue(str);
}

void NoiseReduceDlg::OnThresholdText(wxCommandEvent &event)
{
	wxString str = m_threshold_text->GetValue();
	double val;
	str.ToDouble(&val);
	m_agent->setValue(gstThreshold, val);
}

//voxel size
void NoiseReduceDlg::OnVoxelChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	wxString str = wxString::Format("%d", ival);
	if (str != m_voxel_text->GetValue())
		m_voxel_text->SetValue(str);
}

void NoiseReduceDlg::OnVoxelText(wxCommandEvent &event)
{
	wxString str = m_voxel_text->GetValue();
	long lval;
	str.ToLong(&lval);
	m_agent->setValue(gstCompSizeLimit, lval);
}

void NoiseReduceDlg::OnPreviewBtn(wxCommandEvent &event)
{
	m_agent->Preview();
}

void NoiseReduceDlg::OnEraseBtn(wxCommandEvent &event)
{
	m_agent->BrushErase();
}

void NoiseReduceDlg::OnEnhanceSelChk(wxCommandEvent &event)
{
	bool bval = m_enhance_sel_chk->GetValue();
	m_agent->setValue(gstEnhance, bval);
}
