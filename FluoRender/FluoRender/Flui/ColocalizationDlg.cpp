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
#include <ColocalizationDlg.h>
#include <Global.h>
#include <Names.h>
#include <ColocalDefault.h>
#include <MainFrame.h>
#include <StringConvert.h>
#include <Colocalize.h>
#include <DataManager.h>

ColocalizationDlg::ColocalizationDlg(MainFrame* frame) :
	PropPanel(frame, frame,
		wxDefaultPosition,
		frame->FromDIP(wxSize(500, 500)),
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

void ColocalizationDlg::FluoUpdate(const fluo::ValueCollection& vc)
{
	//update user interface
	if (FOUND_VALUE(gstNull))
		return;
	bool update_all = vc.empty();

	//settings
	if (update_all || FOUND_VALUE(gstColocalMethod))
	{
		m_product_rdb->SetValue(glbin_colocal_def.m_method == 0);
		m_min_value_rdb->SetValue(glbin_colocal_def.m_method == 1);
		m_logical_and_rdb->SetValue(glbin_colocal_def.m_method == 2);
	}

	if (update_all || FOUND_VALUE(gstIntWeighted))
	{
		m_int_weight_btn->SetValue(glbin_colocal_def.m_int_weighted);
	}

	if (update_all || FOUND_VALUE(gstGetRatio))
	{
		m_ratio_btn->SetValue(glbin_colocal_def.m_get_ratio);
	}

	if (update_all || FOUND_VALUE(gstPhysSize))
	{
		m_physical_btn->SetValue(glbin_colocal_def.m_physical_size);
	}

	if (update_all || FOUND_VALUE(gstColocalColormap))
	{
		m_colormap_btn->SetValue(glbin_colocal_def.m_colormap);
	}

	if (update_all || FOUND_VALUE(gstUseSelection))
	{
		m_use_sel_chk->SetValue(glbin_colocal_def.m_use_mask);
	}

	bool colocal_update = false;
	bool colocal_result = FOUND_VALUE(gstColocalResult);
	bool auto_update = FOUND_VALUE(gstColocalAutoUpdate);
	if (update_all || auto_update)
	{
		if (auto_update)
			colocal_update = glbin_colocalizer.GetAutoColocalize();
		else
			colocal_result = true;
	}
	if (colocal_result)
	{
		glbin_colocalizer.Compute();
		SetOutput();
	}
}

void ColocalizationDlg::SetOutput()
{
	wxString copy_data;
	wxString cur_field;
	wxString cur_line;
	int i, k;

	k = 0;
	cur_line = glbin_colocalizer.GetTitles();
	do
	{
		cur_field = cur_line.BeforeFirst('\t');
		cur_line = cur_line.AfterFirst('\t');
		if (m_output_grid->GetNumberCols() <= k)
			m_output_grid->InsertCols(k);
		m_output_grid->SetColLabelValue(k, cur_field);
		++k;
	} while (cur_line.IsEmpty() == false);

	fluo::Color c;
	double val;
	wxColor color;
	auto vd = glbin_current.vol_data.lock();
	auto group = glbin_current.vol_group.lock();
	if (!vd && group)
		vd = group->GetVolumeData(0);
	bool colormap = glbin_colocal_def.m_colormap && vd &&
		(glbin_colocal_def.m_cm_max - glbin_colocal_def.m_cm_min) > 0.0;

	i = 0;
	copy_data = glbin_colocalizer.GetValues();
	do
	{
		k = 0;
		cur_line = copy_data.BeforeFirst('\n');
		copy_data = copy_data.AfterFirst('\n');
		if (m_output_grid->GetNumberRows() <= i ||
			m_hold_history)
			m_output_grid->InsertRows(i);
		do
		{
			cur_field = cur_line.BeforeFirst('\t');
			cur_line = cur_line.AfterFirst('\t');
			m_output_grid->SetCellValue(i, k, cur_field);
			if (colormap && cur_field.ToDouble(&val))
			{
				c = vd->GetColorFromColormap((val - glbin_colocal_def.m_cm_min)/
					(glbin_colocal_def.m_cm_max - glbin_colocal_def.m_cm_min));
				color = wxColor(c.r() * 255, c.g() * 255, c.b() * 255);
				m_output_grid->SetCellBackgroundColour(i, k, color);
			}
			else
			{
				color = *wxWHITE;
				m_output_grid->SetCellBackgroundColour(i, k, color);
			}
			++k;
		} while (cur_line.IsEmpty() == false);
		++i;
	} while (copy_data.IsEmpty() == false);

	if (m_output_grid->GetNumberCols() > k)
		m_output_grid->DeleteCols(k,
			m_output_grid->GetNumberCols() - k);

	//m_output_grid->AutoSizeColumns(false);
	if (glbin_colocal_def.m_physical_size && !glbin_colocal_def.m_get_ratio)
		m_output_grid->SetDefaultColSize(100, true);
	else
		m_output_grid->SetDefaultColSize(70, true);

	//m_output_grid->Fit();
	m_output_grid->Layout();
	m_output_grid->ClearSelection();
	m_output_grid->AdjustScrollbars();
	m_output_grid->ForceRefresh();
}

void ColocalizationDlg::CopyData()
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

void ColocalizationDlg::PasteData()
{
}

void ColocalizationDlg::OnColocalizenBtn(wxCommandEvent& event)
{
	glbin_colocalizer.Compute();
	FluoUpdate({ gstColocalResult });
}

void ColocalizationDlg::OnUseSelChk(wxCommandEvent& event)
{
	glbin_colocal_def.m_use_mask = m_use_sel_chk->GetValue();
	FluoUpdate({ gstColocalAutoUpdate });
}

void ColocalizationDlg::OnMethodRdb(wxCommandEvent& event)
{
	if (m_product_rdb->GetValue())
		glbin_colocal_def.m_method = 0;
	else if (m_min_value_rdb->GetValue())
		glbin_colocal_def.m_method = 1;
	else if (m_logical_and_rdb->GetValue())
		glbin_colocal_def.m_method = 2;

	FluoUpdate({ gstColocalAutoUpdate });
}

//format
void ColocalizationDlg::OnIntWeightBtn(wxCommandEvent& event)
{
	glbin_colocal_def.m_int_weighted = m_int_weight_btn->GetValue();
	FluoUpdate({ gstColocalAutoUpdate });
}

void ColocalizationDlg::OnRatioBtn(wxCommandEvent& event)
{
	glbin_colocal_def.m_get_ratio = m_ratio_btn->GetValue();
	FluoUpdate({ gstColocalAutoUpdate });
}

void ColocalizationDlg::OnPhysicalBtn(wxCommandEvent& event)
{
	glbin_colocal_def.m_physical_size = m_physical_btn->GetValue();
	FluoUpdate({ gstColocalAutoUpdate });
}

void ColocalizationDlg::OnColorMapBtn(wxCommandEvent& event)
{
	glbin_colocal_def.m_colormap = m_colormap_btn->GetValue();
	FluoUpdate({ gstColocalAutoUpdate });
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
		else if (event.GetKeyCode() == wxKeyCode('V'))
			PasteData();
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