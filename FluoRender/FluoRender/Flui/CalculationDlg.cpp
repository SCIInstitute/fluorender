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
#include <CalculationDlg.h>
#include <Global.h>
#include <MainFrame.h>
#include <RenderCanvas.h>
#include <CombineList.h>

CalculationDlg::CalculationDlg(MainFrame *frame)
	: PropPanel(frame, frame,
	wxDefaultPosition,
	frame->FromDIP(wxSize(500, 350)),
	0, "CalculationDlg")
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);
	SetDoubleBuffered(true);

	wxStaticText *st = 0;

	//operand A
	wxBoxSizer *sizer1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Operand A:",
		wxDefaultPosition, FromDIP(wxSize(75, 20)));
	m_calc_load_a_btn = new wxButton(this, wxID_ANY, "Load",
		wxDefaultPosition, FromDIP(wxSize(50, 20)));
	m_calc_a_text = new wxTextCtrl(this, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(200, 20)), wxTE_READONLY);
	m_calc_load_a_btn->Bind(wxEVT_BUTTON, &CalculationDlg::OnLoadA, this);
	sizer1->Add(st, 0, wxALIGN_CENTER);
	sizer1->Add(m_calc_load_a_btn, 0, wxALIGN_CENTER);
	sizer1->Add(m_calc_a_text, 1, wxEXPAND);
	//operand B
	wxBoxSizer *sizer2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Operand B:",
		wxDefaultPosition, FromDIP(wxSize(75, 20)));
	m_calc_load_b_btn = new wxButton(this, wxID_ANY, "Load",
		wxDefaultPosition, FromDIP(wxSize(50, 20)));
	m_calc_b_text = new wxTextCtrl(this, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(200, 20)), wxTE_READONLY);
	m_calc_load_b_btn->Bind(wxEVT_BUTTON, &CalculationDlg::OnLoadB, this);
	sizer2->Add(st, 0, wxALIGN_CENTER);
	sizer2->Add(m_calc_load_b_btn, 0, wxALIGN_CENTER);
	sizer2->Add(m_calc_b_text, 1, wxEXPAND);
	//single operators
	wxBoxSizer *sizer3 = new wxStaticBoxSizer(
		new wxStaticBox(this, wxID_ANY,
			"Single-valued Operators (Require only A)"), wxHORIZONTAL);
	//sizer3
	m_calc_fill_btn = new wxButton(this, wxID_ANY, "Consolidate Voxels",
		wxDefaultPosition, FromDIP(wxSize(50, 25)));
	m_calc_combine_btn = new wxButton(this, wxID_ANY, "Combine Group",
		wxDefaultPosition, FromDIP(wxSize(50, 25)));
	m_calc_fill_btn->Bind(wxEVT_BUTTON, &CalculationDlg::OnCalcFill, this);
	m_calc_combine_btn->Bind(wxEVT_BUTTON, &CalculationDlg::OnCalcCombine, this);
	sizer3->Add(m_calc_fill_btn, 1, wxEXPAND);
	sizer3->Add(m_calc_combine_btn, 1, wxEXPAND);
	//two operators
	wxBoxSizer *sizer4 = new wxStaticBoxSizer(
		new wxStaticBox(this, wxID_ANY,
			"Two-valued Operators (Require both A and B)"), wxHORIZONTAL);
	m_calc_sub_btn = new wxButton(this, wxID_ANY, "Subtract",
		wxDefaultPosition, FromDIP(wxSize(50, 25)));
	m_calc_add_btn = new wxButton(this, wxID_ANY, "Add",
		wxDefaultPosition, FromDIP(wxSize(50, 25)));
	m_calc_div_btn = new wxButton(this, wxID_ANY, "Divide",
		wxDefaultPosition, FromDIP(wxSize(50, 25)));
	m_calc_isc_btn = new wxButton(this, wxID_ANY, "Colocalize",
		wxDefaultPosition, FromDIP(wxSize(50, 25)));
	m_calc_sub_btn->Bind(wxEVT_BUTTON, &CalculationDlg::OnCalcSub, this);
	m_calc_add_btn->Bind(wxEVT_BUTTON, &CalculationDlg::OnCalcAdd, this);
	m_calc_div_btn->Bind(wxEVT_BUTTON, &CalculationDlg::OnCalcDiv, this);
	m_calc_isc_btn->Bind(wxEVT_BUTTON, &CalculationDlg::OnCalcIsc, this);
	sizer4->Add(m_calc_sub_btn, 1, wxEXPAND);
	sizer4->Add(m_calc_add_btn, 1, wxEXPAND);
	sizer4->Add(m_calc_div_btn, 1, wxEXPAND);
	sizer4->Add(m_calc_isc_btn, 1, wxEXPAND);

	//vertical sizer
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
	sizer_v->Add(10, 10);
	sizer_v->Add(sizer1, 0, wxEXPAND);
	sizer_v->Add(10, 10);
	sizer_v->Add(sizer2, 0, wxEXPAND);
	sizer_v->Add(10, 30);
	sizer_v->Add(sizer3, 0, wxEXPAND);
	sizer_v->Add(10, 30);
	sizer_v->Add(sizer4, 0, wxEXPAND);
	sizer_v->Add(10, 10);

	SetSizer(sizer_v);
	Layout();
	SetAutoLayout(true);
	SetScrollRate(10, 10);
}

CalculationDlg::~CalculationDlg()
{

}

void CalculationDlg::FluoUpdate(const fluo::ValueCollection& vc)
{
	//update user interface
	if (FOUND_VALUE(gstNull))
		return;
	bool update_all = vc.empty();

	wxString str;
	if (update_all || FOUND_VALUE(gstVolumeA))
	{
		VolumeData* vd = glbin_vol_calculator.GetVolumeA();
		if (vd)
		{
			str = vd->GetName();
			m_calc_a_text->ChangeValue(str);
		}
	}

	if (update_all || FOUND_VALUE(gstVolumeB))
	{
		VolumeData* vd = glbin_vol_calculator.GetVolumeB();
		if (vd)
		{
			str = vd->GetName();
			m_calc_b_text->ChangeValue(str);
		}
	}
}

//calculations
//operands
void CalculationDlg::OnLoadA(wxCommandEvent& event)
{
	glbin_vol_calculator.SetVolumeA(
		glbin_current.vol_data
	);

	FluoUpdate({ gstVolumeA });
}

void CalculationDlg::OnLoadB(wxCommandEvent& event)
{
	glbin_vol_calculator.SetVolumeB(
		glbin_current.vol_data
	);

	FluoUpdate({ gstVolumeB });
}

//operators
void CalculationDlg::OnCalcSub(wxCommandEvent& event)
{
	glbin_vol_calculator.CalculateGroup(1);
}

void CalculationDlg::OnCalcAdd(wxCommandEvent& event)
{
	glbin_vol_calculator.CalculateGroup(2);
}

void CalculationDlg::OnCalcDiv(wxCommandEvent& event)
{
	glbin_vol_calculator.CalculateGroup(3);
}

void CalculationDlg::OnCalcIsc(wxCommandEvent& event)
{
	glbin_vol_calculator.CalculateGroup(4);
}

//one-operators
void CalculationDlg::OnCalcFill(wxCommandEvent& event)
{
	glbin_vol_calculator.SetVolumeB(0);
	glbin_vol_calculator.CalculateGroup(9);
	FluoUpdate({ gstVolumeB });
}

void CalculationDlg::OnCalcCombine(wxCommandEvent& event)
{
	DataGroup* group = glbin_current.vol_group;
	if (!group)
		return;
	RenderCanvas* canvas = glbin_current.canvas;
	if (!canvas)
		return;

	flrd::CombineList Op;
	wxString name = group->GetName() + "_combined";
	Op.SetName(name);
	std::list<VolumeData*> channs;
	for (int i = 0; i < group->GetVolumeNum(); ++i)
	{
		VolumeData* vd = group->GetVolumeData(i);
		if (!vd)
			continue;
		channs.push_back(vd);
	}
	if (channs.empty())
		return;

	Op.SetVolumes(channs);
	if (!Op.Execute())
		return;

	std::list<VolumeData*> results;
	Op.GetResults(results);
	if (results.empty())
		return;

	wxString group_name = "";
	group = 0;
	VolumeData* volume = 0;
	for (auto i = results.begin(); i != results.end(); ++i)
	{
		VolumeData* vd = *i;
		if (vd)
		{
			if (!volume) volume = vd;
			glbin_data_manager.AddVolumeData(vd);
			if (i == results.begin())
			{
				group_name = canvas->AddGroup("");
				group = canvas->GetGroup(group_name);
			}
			canvas->AddVolumeData(vd, group_name);
		}
	}
	if (group && volume)
	{
		fluo::Color col = volume->GetGammaColor();
		group->SetGammaAll(col);
		col = volume->GetBrightness();
		group->SetBrightnessAll(col);
		col = volume->GetHdr();
		group->SetHdrAll(col);
	}
	glbin_current.SetVolumeGroup(group);

	FluoRefresh(1, { gstVolumePropPanel, gstListCtrl, gstTreeCtrl, gstCurrentSelect, gstUpdateSync },
		{ m_frame->GetRenderCanvas(canvas) });
}