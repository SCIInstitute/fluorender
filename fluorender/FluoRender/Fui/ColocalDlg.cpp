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
#include <ColocalDlg.h>
#include <RenderFrame.h>
#include <Global.hpp>
#include <AgentFactory.hpp>
#include <Renderview.hpp>
#include <VolumeData.hpp>
#include <VolumeGroup.hpp>
#include <Calculate/Compare.h>

BEGIN_EVENT_TABLE(ColocalDlg, wxPanel)
	EVT_BUTTON(ID_ColocalizeBtn, ColocalDlg::OnColocalizenBtn)
	EVT_CHECKBOX(ID_UseSelChk, ColocalDlg::OnUseSelChk)
	EVT_TOGGLEBUTTON(ID_AutoUpdateBtn, ColocalDlg::OnAutoUpdate)
	//settings
	EVT_RADIOBUTTON(ID_ProductRdb, ColocalDlg::OnMethodRdb)
	EVT_RADIOBUTTON(ID_MinValueRdb, ColocalDlg::OnMethodRdb)
	EVT_RADIOBUTTON(ID_LogicalAndRdb, ColocalDlg::OnMethodRdb)
	//format
	EVT_TOGGLEBUTTON(ID_IntWeightBtn, ColocalDlg::OnIntWeightBtn)
	EVT_TOGGLEBUTTON(ID_RatioBtn, ColocalDlg::OnRatioBtn)
	EVT_TOGGLEBUTTON(ID_PhysicalBtn, ColocalDlg::OnPhysicalBtn)
	EVT_TOGGLEBUTTON(ID_ColorMapBtn, ColocalDlg::OnColorMapBtn)
	//output
	EVT_CHECKBOX(ID_HistoryChk, ColocalDlg::OnHistoryChk)
	EVT_BUTTON(ID_ClearHistBtn, ColocalDlg::OnClearHistBtn)
	EVT_KEY_DOWN(ColocalDlg::OnKeyDown)
	EVT_GRID_SELECT_CELL(ColocalDlg::OnSelectCell)
	EVT_GRID_LABEL_LEFT_CLICK(ColocalDlg::OnGridLabelClick)
END_EVENT_TABLE()

ColocalDlg::ColocalDlg(RenderFrame* frame) :
	wxPanel(frame, wxID_ANY,
	wxDefaultPosition, wxSize(500, 500),
	0, "ColocalDlg")
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);

	m_agent = glbin_agtf->getOrAddColocalAgent(gstColocalAgent, *this);


	wxStaticText* st = 0;
	wxBoxSizer *sizerV = new wxBoxSizer(wxVERTICAL);

	//controls
	wxBoxSizer* sizer1 = new wxStaticBoxSizer(
		new wxStaticBox(this, wxID_ANY, "Colocalization Settings"), wxVERTICAL);
	wxBoxSizer* sizer1_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Overlapping Calculation:",
		wxDefaultPosition, wxDefaultSize);
	m_product_rdb = new wxRadioButton(this, ID_ProductRdb, "Product",
		wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	m_min_value_rdb = new wxRadioButton(this, ID_MinValueRdb, "Min Value",
		wxDefaultPosition, wxDefaultSize);
	m_logical_and_rdb = new wxRadioButton(this, ID_LogicalAndRdb, "Threshold + Logical AND",
		wxDefaultPosition, wxDefaultSize);
	m_product_rdb->SetValue(false);
	m_min_value_rdb->SetValue(false);
	m_logical_and_rdb->SetValue(true);
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
	m_int_weight_btn = new wxToggleButton(this, ID_IntWeightBtn, "Int. Weighted",
		wxDefaultPosition, wxSize(75, -1));
	m_ratio_btn = new wxToggleButton(this, ID_RatioBtn, "Ratio (%)",
		wxDefaultPosition, wxSize(75, -1));
	m_physical_btn = new wxToggleButton(this, ID_PhysicalBtn, "Physical Size",
		wxDefaultPosition, wxSize(75, -1));
	m_colormap_btn = new wxToggleButton(this, ID_ColorMapBtn, "Color Map",
		wxDefaultPosition, wxSize(75, -1));
	m_int_weight_btn->SetValue(false);
	m_ratio_btn->SetValue(false);
	m_physical_btn->SetValue(false);
	m_colormap_btn->SetValue(false);
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
	m_use_sel_chk = new wxCheckBox(this, ID_UseSelChk, "Use Selection",
		wxDefaultPosition, wxDefaultSize);
	m_colocalize_btn = new wxButton(this, ID_ColocalizeBtn, "Colocalize",
		wxDefaultPosition, wxSize(75, -1));
	m_auto_update_btn = new wxToggleButton(this, ID_AutoUpdateBtn, "Auto Update",
		wxDefaultPosition, wxSize(75, -1));
	sizer1_3->AddStretchSpacer(1);
	sizer1_3->Add(m_use_sel_chk, 0, wxALIGN_CENTER);
	sizer1_3->Add(m_colocalize_btn, 0, wxALIGN_CENTER);
	sizer1_3->Add(m_auto_update_btn, 0, wxALIGN_CENTER);
	sizer1->Add(10, 10);
	sizer1->Add(sizer1_1, 0, wxEXPAND);
	sizer1->Add(10, 10);
	sizer1->Add(sizer1_2, 0, wxEXPAND);
	sizer1->Add(10, 10);
	sizer1->Add(sizer1_3, 0, wxEXPAND);
	sizer1->Add(10, 10);

	//output
	wxBoxSizer *sizer2 = new wxStaticBoxSizer(
		new wxStaticBox(this, wxID_ANY, "Output"),
		wxVERTICAL);
	wxBoxSizer *sizer2_1 = new wxBoxSizer(wxHORIZONTAL);
	m_history_chk = new wxCheckBox(this, ID_HistoryChk,
		"Hold History", wxDefaultPosition, wxSize(85, 20), wxALIGN_LEFT);
	m_clear_hist_btn = new wxButton(this, ID_ClearHistBtn,
		"Clear History", wxDefaultPosition, wxSize(75, -1));
	sizer2_1->AddStretchSpacer(1);
	sizer2_1->Add(m_history_chk, 0, wxALIGN_CENTER);
	sizer2_1->Add(5, 5);
	sizer2_1->Add(m_clear_hist_btn, 0, wxALIGN_CENTER);
	//grid
	m_output_grid = new wxGrid(this, ID_OutputGrid);
	m_output_grid->CreateGrid(0, 1);
	m_output_grid->Fit();
	sizer2->Add(5, 5);
	sizer2->Add(sizer2_1, 0, wxEXPAND);
	sizer2->Add(5, 5);
	sizer2->Add(m_output_grid, 1, wxEXPAND);
	sizer2->Add(5, 5);

	sizerV->Add(10, 10);
	sizerV->Add(sizer1, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(sizer2, 1, wxEXPAND);
	sizerV->Add(10, 10);

	SetSizer(sizerV);
	Layout();

	GetSettings();
}

ColocalDlg::~ColocalDlg()
{
}

void ColocalDlg::GetSettings()
{
}

void ColocalDlg::CopyData()
{
	int i, k;
	wxString copy_data;
	bool something_in_this_line;

	copy_data.Clear();

	bool t = m_output_grid->IsSelection();

	for (i = 0; i < m_output_grid->GetNumberRows(); i++)
	{
		something_in_this_line = false;
		for (k = 0; k < m_output_grid->GetNumberCols(); k++)
		{
			if (m_output_grid->IsInSelection(i, k))
			{
				if (something_in_this_line == false)
				{  // first field in this line => may need a linefeed
					if (copy_data.IsEmpty() == false)
					{     // ... if it is not the very first field
						copy_data = copy_data + wxT("\n");  // next LINE
					}
					something_in_this_line = true;
				}
				else
				{
					// if not the first field in this line we need a field seperator (TAB)
					copy_data = copy_data + wxT("\t");  // next COLUMN
				}
				copy_data = copy_data + m_output_grid->GetCellValue(i, k);    // finally we need the field value :-)
			}
		}
	}

	if (wxTheClipboard->Open())
	{
		// This data objects are held by the clipboard,
		// so do not delete them in the app.
		wxTheClipboard->SetData(new wxTextDataObject(copy_data));
		wxTheClipboard->Close();
	}
}

void ColocalDlg::PasteData()
{
}

void ColocalDlg::StartTimer(std::string str)
{
	if (m_test_speed)
	{
		m_tps.push_back(std::chrono::high_resolution_clock::now());
	}
}

void ColocalDlg::StopTimer(std::string str)
{
	if (m_test_speed)
	{
		auto t0 = m_tps.back();
		m_tps.push_back(std::chrono::high_resolution_clock::now());
		std::chrono::duration<double> time_span =
			std::chrono::duration_cast<std::chrono::duration<double>>(
				m_tps.back() - t0);

		m_values += str + "\t";
		m_values += wxString::Format("%.4f", time_span.count());
		m_values += " sec.\n";
	}
}

void ColocalDlg::OnColocalizenBtn(wxCommandEvent &event)
{
	m_agent->Run();
}

void ColocalDlg::OnAutoUpdate(wxCommandEvent &event)
{
	bool bval = m_auto_update_btn->GetValue();
	m_agent->updValue(gstAutoUpdate, bval);
}

void ColocalDlg::OnUseSelChk(wxCommandEvent &event)
{
	bool bval = m_use_sel_chk->GetValue();
	m_agent->updValue(gstUseSelection, bval);
}

void ColocalDlg::OnMethodRdb(wxCommandEvent &event)
{
	long method = 0;
	if (m_product_rdb->GetValue())
		method = 0;
	else if (m_min_value_rdb->GetValue())
		method = 1;
	else if (m_logical_and_rdb->GetValue())
		method = 2;
	m_agent->updValue(gstColocalMethod, method);
}

//format
void ColocalDlg::OnIntWeightBtn(wxCommandEvent &event)
{
	bool bval = m_int_weight_btn->GetValue();
	m_agent->updValue(gstIntWeighted, bval);
}

void ColocalDlg::OnRatioBtn(wxCommandEvent &event)
{
	bool bval = m_ratio_btn->GetValue();
	m_agent->updValue(gstGetRatio, bval);
}

void ColocalDlg::OnPhysicalBtn(wxCommandEvent &event)
{
	bool bval = m_physical_btn->GetValue();
	m_agent->updValue(gstPhysSize, bval);
}

void ColocalDlg::OnColorMapBtn(wxCommandEvent &event)
{
	bool bval = m_colormap_btn->GetValue();
	m_agent->updValue(gstColormapEnable, bval);
}

void ColocalDlg::OnHistoryChk(wxCommandEvent& event)
{
	bool bval = m_history_chk->GetValue();
	m_agent->updValue(gstHoldHistory, bval);
}

void ColocalDlg::OnClearHistBtn(wxCommandEvent& event)
{
	m_output_grid->DeleteRows(0, m_output_grid->GetNumberRows());
}

void ColocalDlg::OnKeyDown(wxKeyEvent& event)
{
	if (wxGetKeyState(WXK_CONTROL))
	{
		if (event.GetKeyCode() == wxKeyCode('C'))
			CopyData();
		else if (event.GetKeyCode() == wxKeyCode('V'))
			PasteData();
	}
	event.Skip();
}

void ColocalDlg::OnSelectCell(wxGridEvent& event)
{
	int r = event.GetRow();
	int c = event.GetCol();
	m_output_grid->SelectBlock(r, c, r, c);
	event.Skip();
}

void ColocalDlg::OnGridLabelClick(wxGridEvent& event)
{
	m_output_grid->SetFocus();
	event.Skip();
}
