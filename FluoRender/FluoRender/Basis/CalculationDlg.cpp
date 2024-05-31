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
#include <Calculate/CombineList.h>

BEGIN_EVENT_TABLE(CalculationDlg, wxPanel)
	//calculations
	//operands
	EVT_BUTTON(ID_CalcLoadABtn, CalculationDlg::OnLoadA)
	EVT_BUTTON(ID_CalcLoadBBtn, CalculationDlg::OnLoadB)
	//operators
	EVT_BUTTON(ID_CalcSubBtn, CalculationDlg::OnCalcSub)
	EVT_BUTTON(ID_CalcAddBtn, CalculationDlg::OnCalcAdd)
	EVT_BUTTON(ID_CalcDivBtn, CalculationDlg::OnCalcDiv)
	EVT_BUTTON(ID_CalcIscBtn, CalculationDlg::OnCalcIsc)
	//one-operators
	EVT_BUTTON(ID_CalcFillBtn, CalculationDlg::OnCalcFill)
	EVT_BUTTON(ID_CalcCombineBtn, CalculationDlg::OnCalcCombine)
END_EVENT_TABLE()

CalculationDlg::CalculationDlg(MainFrame *frame)
	: wxPanel(frame, wxID_ANY,
	wxDefaultPosition,
	frame->FromDIP(wxSize(500, 350)),
	0, "CalculationDlg"),
	m_frame(frame),
	m_group(0)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);
	SetDoubleBuffered(true);

	wxStaticText *st = 0;

	//operand A
	wxBoxSizer *sizer1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Operand A:",
		wxDefaultPosition, FromDIP(wxSize(75, 20)));
	m_calc_load_a_btn = new wxButton(this, ID_CalcLoadABtn, "Load",
		wxDefaultPosition, FromDIP(wxSize(50, 20)));
	m_calc_a_text = new wxTextCtrl(this, ID_CalcAText, "",
		wxDefaultPosition, FromDIP(wxSize(200, 20)));
	sizer1->Add(st, 0, wxALIGN_CENTER);
	sizer1->Add(m_calc_load_a_btn, 0, wxALIGN_CENTER);
	sizer1->Add(m_calc_a_text, 1, wxEXPAND);
	//operand B
	wxBoxSizer *sizer2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Operand B:",
		wxDefaultPosition, FromDIP(wxSize(75, 20)));
	m_calc_load_b_btn = new wxButton(this, ID_CalcLoadBBtn, "Load",
		wxDefaultPosition, FromDIP(wxSize(50, 20)));
	m_calc_b_text = new wxTextCtrl(this, ID_CalcBText, "",
		wxDefaultPosition, FromDIP(wxSize(200, 20)));
	sizer2->Add(st, 0, wxALIGN_CENTER);
	sizer2->Add(m_calc_load_b_btn, 0, wxALIGN_CENTER);
	sizer2->Add(m_calc_b_text, 1, wxEXPAND);
	//single operators
	wxBoxSizer *sizer3 = new wxStaticBoxSizer(
		new wxStaticBox(this, wxID_ANY,
			"Single-valued Operators (Require only A)"), wxHORIZONTAL);
	//sizer3
	m_calc_fill_btn = new wxButton(this, ID_CalcFillBtn, "Consolidate Voxels",
		wxDefaultPosition, FromDIP(wxSize(50, 25)));
	m_calc_combine_btn = new wxButton(this, ID_CalcCombineBtn, "Combine Group",
		wxDefaultPosition, FromDIP(wxSize(50, 25)));
	sizer3->Add(m_calc_fill_btn, 1, wxEXPAND);
	sizer3->Add(m_calc_combine_btn, 1, wxEXPAND);
	//two operators
	wxBoxSizer *sizer4 = new wxStaticBoxSizer(
		new wxStaticBox(this, wxID_ANY,
			"Two-valued Operators (Require both A and B)"), wxHORIZONTAL);
	m_calc_sub_btn = new wxButton(this, ID_CalcSubBtn, "Subtract",
		wxDefaultPosition, FromDIP(wxSize(50, 25)));
	m_calc_add_btn = new wxButton(this, ID_CalcAddBtn, "Add",
		wxDefaultPosition, FromDIP(wxSize(50, 25)));
	m_calc_div_btn = new wxButton(this, ID_CalcDivBtn, "Divide",
		wxDefaultPosition, FromDIP(wxSize(50, 25)));
	m_calc_isc_btn = new wxButton(this, ID_CalcIscBtn, "Colocalize",
		wxDefaultPosition, FromDIP(wxSize(50, 25)));
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
}

CalculationDlg::~CalculationDlg()
{

}

void CalculationDlg::SetGroup(DataGroup* group)
{
	m_group = group;
}

//calculations
//operands
void CalculationDlg::OnLoadA(wxCommandEvent &event)
{
	m_view = 0;
	if (m_frame)
	{
		switch (glbin_current.GetType())
		{
		case 2://volume
			m_vol1 = glbin_current.vol_data;
			m_group = 0;
			if (m_vol1)
			{
				wxString str = m_vol1->GetName();
				m_calc_a_text->ChangeValue(str);
				for (int i = 0; i < m_frame->GetViewNum(); i++)
				{
					RenderCanvas* view = m_frame->GetRenderCanvas(i);
					if (view && view->GetVolumeData(str))
					{
						m_view = view;
						break;
					}
				}
			}
			break;
		case 5://volume group
			m_vol1 = 0;
			if (m_group)
			{
				wxString str = m_group->GetName();
				m_calc_a_text->ChangeValue(str);
				for (int i = 0; i < m_frame->GetViewNum(); i++)
				{
					RenderCanvas* view = m_frame->GetRenderCanvas(i);
					if (view && view->GetGroup(str))
					{
						m_view = view;
						break;
					}
				}
			}
			break;
		}
		if (!m_view)
			m_view = m_frame->GetRenderCanvas(0);
	}
}

void CalculationDlg::OnLoadB(wxCommandEvent &event)
{
	if (m_frame)
	{
		switch (glbin_current.GetType())
		{
		case 2://volume
			m_vol2 = glbin_current.vol_data;
			if (m_vol2)
				m_calc_b_text->ChangeValue(m_vol2->GetName());
			break;
		case 5://volume group
			if (m_group)
				m_calc_b_text->ChangeValue(m_group->GetName());
			break;
		}
	}
}

//operators
void CalculationDlg::OnCalcSub(wxCommandEvent &event)
{
	if (!m_frame || !m_view)
		return;
	if (!m_vol1 || !m_vol2)
		return;

	glbin_vol_calculator.SetVolumeA(m_vol1);
	glbin_vol_calculator.SetVolumeB(m_vol2);
	glbin_vol_calculator.CalculateGroup(1);
}

void CalculationDlg::OnCalcAdd(wxCommandEvent &event)
{
	if (!m_frame || !m_view)
		return;
	if (!m_vol1 || !m_vol2)
		return;

	glbin_vol_calculator.SetVolumeA(m_vol1);
	glbin_vol_calculator.SetVolumeB(m_vol2);
	glbin_vol_calculator.CalculateGroup(2);
}

void CalculationDlg::OnCalcDiv(wxCommandEvent &event)
{
	if (!m_frame || !m_view)
		return;
	if (!m_vol1 || !m_vol2)
		return;

	glbin_vol_calculator.SetVolumeA(m_vol1);
	glbin_vol_calculator.SetVolumeB(m_vol2);
	glbin_vol_calculator.CalculateGroup(3);
}

void CalculationDlg::OnCalcIsc(wxCommandEvent &event)
{
	if (!m_frame || !m_view)
		return;
	if (!m_vol1 || !m_vol2)
		return;

	glbin_vol_calculator.SetVolumeA(m_vol1);
	glbin_vol_calculator.SetVolumeB(m_vol2);
	glbin_vol_calculator.CalculateGroup(4);
}

//one-operators
void CalculationDlg::OnCalcFill(wxCommandEvent &event)
{
	if (!m_frame || !m_view)
		return;
	if (!m_vol1)
		return;

	glbin_vol_calculator.SetVolumeA(m_vol1);
	m_vol2 = 0;
	glbin_vol_calculator.SetVolumeB(0);
	m_calc_b_text->Clear();
	glbin_vol_calculator.CalculateGroup(9);
}

void CalculationDlg::OnCalcCombine(wxCommandEvent &event)
{
	if (!m_frame || !m_view)
		return;
	if (m_calc_a_text->GetValue() == "")
		return;
	if (!m_group)
		return;

	bool refresh = false;
	flrd::CombineList Op;
	wxString name = m_group->GetName() + "_combined";
	Op.SetName(name);
	std::list<VolumeData*> channs;
	for (int i = 0; i < m_group->GetVolumeNum(); ++i)
	{
		VolumeData* vd = m_group->GetVolumeData(i);
		if (!vd)
			continue;
		channs.push_back(vd);
	}
	Op.SetVolumes(channs);
	if (Op.Execute())
	{
		std::list<VolumeData*> results;
		Op.GetResults(results);
		if (results.empty())
			return;

		wxString group_name = "";
		DataGroup* group = 0;
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
					group_name = m_view->AddGroup("");
					group = m_view->GetGroup(group_name);
				}
				m_view->AddVolumeData(vd, group_name);
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
		//m_frame->UpdateList();
		//m_frame->UpdateTree(m_group->GetName());
		//m_view->RefreshGL(39);
		glbin_current.SetVolumeGroup(m_group);
		refresh = true;
	}

	if (refresh)
	{
		m_frame->UpdateProps({ gstListCtrl, gstTreeCtrl, gstCurrentSelect });
		m_frame->RefreshCanvases({ m_frame->GetRenderCanvas(m_view) });
	}
}