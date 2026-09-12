/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2026 Scientific Computing and Imaging Institute,
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
#include <ColocalizationDlg.h>
#include <ColocalizationDlgAgent.h>
#include <GridHelper.h>

ColocalizationDlg::ColocalizationDlg(wxWindow* parent) :
	PropPanel(parent,
		wxDefaultPosition,
		parent->FromDIP(wxSize(500, 500)),
		0, "ColocalizationDlg"),
	m_hold_history(false)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);
	SetDoubleBuffered(true);

	wxStaticText* st = 0;

	//controls
	wxStaticBoxSizer* sizer1 = new wxStaticBoxSizer(
		wxVERTICAL, this, "Colocalization Settings");
	wxBoxSizer* sizer1_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Overlapping Calculation:",
		wxDefaultPosition, wxDefaultSize);
	m_product_rdb = new wxRadioButton(this, wxID_ANY, "Product",
		wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	m_min_value_rdb = new wxRadioButton(this, wxID_ANY, "Min Value",
		wxDefaultPosition, wxDefaultSize);
	m_logical_and_rdb = new wxRadioButton(this, wxID_ANY, "Threshold + Logical AND",
		wxDefaultPosition, wxDefaultSize);
	m_product_rdb->Bind(wxEVT_RADIOBUTTON, &ColocalizationDlg::OnMethodRdb, this);
	m_min_value_rdb->Bind(wxEVT_RADIOBUTTON, &ColocalizationDlg::OnMethodRdb, this);
	m_logical_and_rdb->Bind(wxEVT_RADIOBUTTON, &ColocalizationDlg::OnMethodRdb, this);
	sizer1_1->Add(10, 10);
	sizer1_1->Add(st, 0, wxALIGN_CENTER);
	sizer1_1->Add(10, 10);
	sizer1_1->Add(m_logical_and_rdb, 0, wxALIGN_CENTER);
	sizer1_1->Add(10, 10);
	sizer1_1->Add(m_min_value_rdb, 0, wxALIGN_CENTER);
	sizer1_1->Add(10, 10);
	sizer1_1->Add(m_product_rdb, 0, wxALIGN_CENTER);
	wxBoxSizer* sizer1_2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Output Format:",
		wxDefaultPosition, wxDefaultSize);
	m_int_weight_btn = new wxToggleButton(this, wxID_ANY, "Int. Weighted",
		wxDefaultPosition, FromDIP(wxSize(75, -1)));
	m_ratio_btn = new wxToggleButton(this, wxID_ANY, "Ratio (%)",
		wxDefaultPosition, FromDIP(wxSize(75, -1)));
	m_physical_btn = new wxToggleButton(this, wxID_ANY, "Physical Size",
		wxDefaultPosition, FromDIP(wxSize(75, -1)));
	m_colormap_btn = new wxToggleButton(this, wxID_ANY, "Color Map",
		wxDefaultPosition, FromDIP(wxSize(75, -1)));
	m_int_weight_btn->Bind(wxEVT_TOGGLEBUTTON, &ColocalizationDlg::OnIntWeightBtn, this);
	m_ratio_btn->Bind(wxEVT_TOGGLEBUTTON, &ColocalizationDlg::OnRatioBtn, this);
	m_physical_btn->Bind(wxEVT_TOGGLEBUTTON, &ColocalizationDlg::OnPhysicalBtn, this);
	m_colormap_btn->Bind(wxEVT_TOGGLEBUTTON, &ColocalizationDlg::OnColorMapBtn, this);
	sizer1_2->Add(10, 10);
	sizer1_2->Add(st, 0, wxALIGN_CENTER);
	sizer1_2->Add(10, 10);
	sizer1_2->Add(m_ratio_btn, 0, wxALIGN_CENTER);
	sizer1_2->Add(10, 10);
	sizer1_2->Add(m_int_weight_btn, 0, wxALIGN_CENTER);
	sizer1_2->Add(10, 10);
	sizer1_2->Add(m_physical_btn, 0, wxALIGN_CENTER);
	sizer1_2->Add(10, 10);
	sizer1_2->Add(m_colormap_btn, 0, wxALIGN_CENTER);
	wxBoxSizer* sizer1_3 = new wxBoxSizer(wxHORIZONTAL);
	m_use_sel_chk = new wxCheckBox(this, wxID_ANY, "Use Selection",
		wxDefaultPosition, wxDefaultSize);
	m_colocalize_btn = new wxButton(this, wxID_ANY, "Colocalize",
		wxDefaultPosition, FromDIP(wxSize(75, -1)));
	m_use_sel_chk->Bind(wxEVT_CHECKBOX, &ColocalizationDlg::OnUseSelChk, this);
	m_colocalize_btn->Bind(wxEVT_BUTTON, &ColocalizationDlg::OnColocalizenBtn, this);
	sizer1_3->AddStretchSpacer(1);
	sizer1_3->Add(m_use_sel_chk, 0, wxALIGN_CENTER);
	sizer1_3->Add(m_colocalize_btn, 0, wxALIGN_CENTER);
	sizer1->Add(10, 10);
	sizer1->Add(sizer1_1, 0, wxEXPAND);
	sizer1->Add(10, 10);
	sizer1->Add(sizer1_2, 0, wxEXPAND);
	sizer1->Add(10, 10);
	sizer1->Add(sizer1_3, 0, wxEXPAND);
	sizer1->Add(10, 10);

	//output
	wxStaticBoxSizer *sizer2 = new wxStaticBoxSizer(
		wxVERTICAL, this, "Output");
	wxBoxSizer *sizer2_1 = new wxBoxSizer(wxHORIZONTAL);
	m_history_chk = new wxCheckBox(this, wxID_ANY,
		"Hold History", wxDefaultPosition, FromDIP(wxSize(85, 20)), wxALIGN_LEFT);
	m_clear_hist_btn = new wxButton(this, wxID_ANY,
		"Clear History", wxDefaultPosition, FromDIP(wxSize(75, -1)));
	m_history_chk->Bind(wxEVT_CHECKBOX, &ColocalizationDlg::OnHistoryChk, this);
	m_clear_hist_btn->Bind(wxEVT_BUTTON, &ColocalizationDlg::OnClearHistBtn, this);
	sizer2_1->AddStretchSpacer(1);
	sizer2_1->Add(m_history_chk, 0, wxALIGN_CENTER);
	sizer2_1->Add(5, 5);
	sizer2_1->Add(m_clear_hist_btn, 0, wxALIGN_CENTER);
	//grid
	m_output_grid = new wxGrid(this, wxID_ANY);
	m_output_grid->CreateGrid(0, 1);
	//m_output_grid->Fit();
	m_output_grid->Bind(wxEVT_GRID_SELECT_CELL, &ColocalizationDlg::OnSelectCell, this);
	m_output_grid->Bind(wxEVT_GRID_LABEL_LEFT_CLICK, &ColocalizationDlg::OnGridLabelClick, this);
	sizer2->Add(5, 5);
	sizer2->Add(sizer2_1, 0, wxEXPAND);
	sizer2->Add(5, 5);
	sizer2->Add(m_output_grid, 1, wxEXPAND);
	sizer2->Add(5, 5);

	wxBoxSizer* sizerV = new wxBoxSizer(wxVERTICAL);
	sizerV->Add(10, 10);
	sizerV->Add(sizer1, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(sizer2, 1, wxEXPAND);
	sizerV->Add(10, 10);

	SetSizer(sizerV);
	Layout();

	Bind(wxEVT_KEY_DOWN, &ColocalizationDlg::OnKeyDown, this);
	Bind(wxEVT_SIZE, &ColocalizationDlg::OnSize, this);
}

ColocalizationDlg::~ColocalizationDlg()
{
}

void ColocalizationDlg::UpdateColocalMethod(int ival)
{
	m_product_rdb->SetValue(ival == 0);
	m_min_value_rdb->SetValue(ival == 1);
	m_logical_and_rdb->SetValue(ival == 2);
}

void ColocalizationDlg::UpdateIntWeighted(bool bval)
{
	m_int_weight_btn->SetValue(bval);
}

void ColocalizationDlg::UpdateGetRatio(bool bval)
{
	m_ratio_btn->SetValue(bval);
}

void ColocalizationDlg::UpdatePhysicalSize(bool bval)
{
	m_physical_btn->SetValue(bval);
}

void ColocalizationDlg::UpdateColocalColormap(bool bval)
{
	m_colormap_btn->SetValue(bval);
}

void ColocalizationDlg::UpdateUseSelection(bool bval)
{
	m_use_sel_chk->SetValue(bval);
}

void ColocalizationDlg::CopyData()
{
	auto text =
		GridHelper::CopySelection(
			m_output_grid);

	if (text.empty())
		return;

	if (wxTheClipboard->Open())
	{
		wxTheClipboard->SetData(
			new wxTextDataObject(text));

		wxTheClipboard->Close();
	}
}

void ColocalizationDlg::UpdateGrid(const GridData& data)
{
	GridPopulateOptions options;
	options.append_rows = false;
	options.remove_extra_rows = !m_hold_history;
	options.remove_extra_cols = !m_hold_history;

	GridHelper::Populate(
		m_output_grid,
		data,
		options);

	m_output_grid->ClearSelection();
}


void ColocalizationDlg::OnColocalizenBtn(wxCommandEvent& event)
{
	auto agent = m_agent->As<ColocalizationDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstColocalResult });
}

void ColocalizationDlg::OnUseSelChk(wxCommandEvent& event)
{
	bool bval = m_use_sel_chk->GetValue();
	auto agent = m_agent->As<ColocalizationDlgAgent>();
	if (agent)
		agent->SetUseSelection(bval);
}

void ColocalizationDlg::OnMethodRdb(wxCommandEvent& event)
{
	int ival = 0;
	if (m_product_rdb->GetValue())
		ival = 0;
	else if (m_min_value_rdb->GetValue())
		ival = 1;
	else if (m_logical_and_rdb->GetValue())
		ival = 2;
	auto agent = m_agent->As<ColocalizationDlgAgent>();
	if (agent)
		agent->SetMethod(ival);
}

//format
void ColocalizationDlg::OnIntWeightBtn(wxCommandEvent& event)
{
	bool bval = m_int_weight_btn->GetValue();
	auto agent = m_agent->As<ColocalizationDlgAgent>();
	if (agent)
		agent->SetInWeight(bval);
}

void ColocalizationDlg::OnRatioBtn(wxCommandEvent& event)
{
	bool bval = m_ratio_btn->GetValue();
	auto agent = m_agent->As<ColocalizationDlgAgent>();
	if (agent)
		agent->SetRatio(bval);
}

void ColocalizationDlg::OnPhysicalBtn(wxCommandEvent& event)
{
	bool bval = m_physical_btn->GetValue();
	auto agent = m_agent->As<ColocalizationDlgAgent>();
	if (agent)
		agent->SetPhysical(bval);
}

void ColocalizationDlg::OnColorMapBtn(wxCommandEvent& event)
{
	bool bval = m_colormap_btn->GetValue();
	auto agent = m_agent->As<ColocalizationDlgAgent>();
	if (agent)
		agent->SetColormap(bval);
}

void ColocalizationDlg::OnHistoryChk(wxCommandEvent& event)
{
	m_hold_history = m_history_chk->GetValue();
}

void ColocalizationDlg::OnClearHistBtn(wxCommandEvent& event)
{
	m_output_grid->DeleteRows(0, m_output_grid->GetNumberRows());
}

void ColocalizationDlg::OnKeyDown(wxKeyEvent& event)
{
	if (wxGetKeyState(WXK_CONTROL))
	{
		if (event.GetKeyCode() == wxKeyCode('C'))
			CopyData();
		//else if (event.GetKeyCode() == wxKeyCode('V'))
		//	PasteData();
	}
}

void ColocalizationDlg::OnSelectCell(wxGridEvent& event)
{
	int r = event.GetRow();
	int c = event.GetCol();
	m_output_grid->SelectBlock(r, c, r, c);
}

void ColocalizationDlg::OnGridLabelClick(wxGridEvent& event)
{
	m_output_grid->SetFocus();
}

void ColocalizationDlg::OnSize(wxSizeEvent& event)
{
	if (!m_output_grid)
		return;

	wxSize size = GetSize();
	wxPoint p1 = GetScreenPosition();
	wxPoint p2 = m_output_grid->GetScreenPosition();
	int height, margin;
	if (m_output_grid->GetNumberRows())
		height = m_output_grid->GetRowSize(0) * 8;
	else
		height = 80;
	margin = size.y + p1.y - p2.y - 20;
	if (margin > height)
		size.y = margin;
	else
		size.y = height;
	size.x -= 15;
	m_output_grid->SetMaxSize(size);
}