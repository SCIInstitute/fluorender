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
#include "ColocalizationDlg.h"
#include "VRenderFrame.h"
#include "DataManager.h"
#include <Calculate/Compare.h>

BEGIN_EVENT_TABLE(ColocalizationDlg, wxPanel)
	EVT_BUTTON(ID_ColocalizeBtn, ColocalizationDlg::OnColocalizenBtn)
	EVT_CHECKBOX(ID_UseSelChk, ColocalizationDlg::OnUseSelChk)
	EVT_TOGGLEBUTTON(ID_AutoUpdateBtn, ColocalizationDlg::OnAutoUpdate)
	//settings
	EVT_RADIOBUTTON(ID_ProductRdb, ColocalizationDlg::OnMethodRdb)
	EVT_RADIOBUTTON(ID_MinValueRdb, ColocalizationDlg::OnMethodRdb)
	EVT_RADIOBUTTON(ID_LogicalAndRdb, ColocalizationDlg::OnMethodRdb)
	//format
	EVT_TOGGLEBUTTON(ID_IntWeightBtn, ColocalizationDlg::OnIntWeightBtn)
	EVT_TOGGLEBUTTON(ID_RatioBtn, ColocalizationDlg::OnRatioBtn)
	EVT_TOGGLEBUTTON(ID_PhysicalBtn, ColocalizationDlg::OnPhysicalBtn)
	EVT_TOGGLEBUTTON(ID_ColorMapBtn, ColocalizationDlg::OnColorMapBtn)
	//output
	EVT_CHECKBOX(ID_HistoryChk, ColocalizationDlg::OnHistoryChk)
	EVT_BUTTON(ID_ClearHistBtn, ColocalizationDlg::OnClearHistBtn)
	EVT_KEY_DOWN(ColocalizationDlg::OnKeyDown)
	EVT_GRID_SELECT_CELL(ColocalizationDlg::OnSelectCell)
	EVT_GRID_LABEL_LEFT_CLICK(ColocalizationDlg::OnGridLabelClick)
END_EVENT_TABLE()

ColocalizationDlg::ColocalizationDlg(wxWindow* frame,
	wxWindow* parent) :
wxPanel(parent, wxID_ANY,
wxDefaultPosition, wxSize(500, 500),
0, "ColocalizationDlg"),
m_frame(parent),
m_view(0),
m_group(0),
m_hold_history(false)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);
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

ColocalizationDlg::~ColocalizationDlg()
{
}

void ColocalizationDlg::SetOutput(wxString &titles, wxString &values)
{
	wxString copy_data;
	wxString cur_field;
	wxString cur_line;
	int i, k;

	k = 0;
	cur_line = titles;
	do
	{
		cur_field = cur_line.BeforeFirst('\t');
		cur_line = cur_line.AfterFirst('\t');
		if (m_output_grid->GetNumberCols() <= k)
			m_output_grid->InsertCols(k);
		m_output_grid->SetColLabelValue(k, cur_field);
		++k;
	} while (cur_line.IsEmpty() == false);

	Color c;
	double val;
	wxColor color;
	VolumeData* vd = 0;
	if (m_colormap && m_view)
		vd = m_view->m_glview->m_cur_vol;
	bool colormap = m_colormap && vd && (m_cm_max - m_cm_min) > 0.0;

	i = 0;
	copy_data = values;
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
				c = vd->GetColorFromColormap((val - m_cm_min)/(m_cm_max - m_cm_min));
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

	m_output_grid->AutoSizeColumns(false);
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

void ColocalizationDlg::GetSettings()
{
	m_use_mask = false;
	m_auto_update = false;
	m_method = 2;
	m_int_weighted = false;
	m_get_ratio = false;
	m_physical_size = false;
	m_colormap = false;
}

//execute
void ColocalizationDlg::Colocalize()
{
	if (!m_group)
		return;

	int num = m_group->GetVolumeNum();
	if (num < 2)
		return;

	//spacings, assuming they are all same for channels
	double spcx, spcy, spcz;
	double spc;
	wxString unit;
	VolumeData* vd = m_group->GetVolumeData(0);
	if (!vd)
	{
		spc = spcx = spcy = spcz = 1.0;
	}
	else
	{
		vd->GetSpacings(spcx, spcy, spcz);
		spc = spcx * spcy * spcz;
	}
	if (m_view)
	{
		switch (m_view->m_glview->m_sb_unit)
		{
		case 0:
			unit = L"nm\u00B3";
			break;
		case 1:
		default:
			unit = L"\u03BCm\u00B3";
			break;
		case 2:
			unit = L"mm\u00B3";
			break;
		}
	}

	//result
	std::vector<std::vector<double>> rm;//result matrix
	rm.reserve(num);
	for (size_t i = 0; i < num; ++i)
	{
		rm.push_back(std::vector<double>());
		rm[i].reserve(num);
		for (size_t j = 0; j < num; ++j)
			rm[i].push_back(0);
	}

	//fill the matrix
	size_t x = 0, y = 0;
	for (int it1 = 0; it1 < num; ++it1)
	{
		y = x;
		for (int it2 = it1; it2 < num; ++it2)
		{
			VolumeData* vd1 = m_group->GetVolumeData(it1);
			VolumeData* vd2 = m_group->GetVolumeData(it2);
			if (!vd1 || !vd2 ||
				!vd1->GetDisp() ||
				!vd2->GetDisp())
				continue;

			FL::ChannelCompare compare(vd1, vd2);
			compare.SetUseMask(m_use_mask);
			compare.SetIntWeighted(m_int_weighted);
			switch (m_method)
			{
			case 0://dot product
				compare.Product();
				break;
			case 1://min value
				compare.MinValue();
				break;
			case 2://threshold
			{
				//get threshold values
				float th1, th2, th3, th4;
				th1 = (float)(vd1->GetLeftThresh());
				th2 = (float)(vd1->GetRightThresh());
				th3 = (float)(vd2->GetLeftThresh());
				th4 = (float)(vd2->GetRightThresh());
				compare.Threshold(th1, th2, th3, th4);
			}
			break;
			}
			rm[x][y] = compare.Result();
			rm[y][x] = compare.Result();
			y++;
		}
		x++;
	}

	wxString titles;
	wxString name;
	double v;
	ResetMinMax();
	for (size_t i = 0; i < num; ++i)
	{
		if (m_get_ratio)
			titles += wxString::Format("%d (%%)", int(i + 1));
		else
			titles += wxString::Format("%d", int(i + 1));
		VolumeData* vd = m_group->GetVolumeData(i);
		if (vd)
			name = vd->GetName();
		else
			name = "";
		titles += ": " + name;
		if (i < num - 1)
			titles += "\t";
		else
			titles += "\n";
	}
	wxString values;
	for (int it1 = 0; it1 < num; ++it1)
	for (int it2 = 0; it2 < num; ++it2)
	{
		if (m_get_ratio)
		{
			if (rm[it2][it2])
			{
				v = rm[it1][it2] * 100.0 / rm[it1][it1];
				SetMinMax(v);
				values += wxString::Format("%f", v);
			}
			else
			{
				SetMinMax(0.0);
				values += "0";
			}
		}
		else
		{
			if (m_physical_size)
			{
				v = rm[it1][it2] * spc;
				SetMinMax(v);
				values += wxString::Format("%f", v);
				values += unit;
			}
			else
			{
				v = rm[it1][it2];
				SetMinMax(v);
				if (m_int_weighted)
					values += wxString::Format("%f", v);
				else
					values += wxString::Format("%.0f", v);
			}
		}
		if (it2 < num - 1)
			values += "\t";
		else
			values += "\n";
	}
	SetOutput(titles, values);
}

void ColocalizationDlg::OnColocalizenBtn(wxCommandEvent &event)
{
	Colocalize();
}

void ColocalizationDlg::OnUseSelChk(wxCommandEvent &event)
{
	m_use_mask = m_use_sel_chk->GetValue();

	if (m_auto_update)
		Colocalize();
}

void ColocalizationDlg::OnAutoUpdate(wxCommandEvent &event)
{
	m_auto_update = m_auto_update_btn->GetValue();
	if (m_view)
		m_view->m_glview->m_paint_colocalize = m_auto_update;
}

void ColocalizationDlg::OnMethodRdb(wxCommandEvent &event)
{
	if (m_product_rdb->GetValue())
		m_method = 0;
	else if (m_min_value_rdb->GetValue())
		m_method = 1;
	else if (m_logical_and_rdb->GetValue())
		m_method = 2;

	if (m_auto_update)
		Colocalize();
}

//format
void ColocalizationDlg::OnIntWeightBtn(wxCommandEvent &event)
{
	m_int_weighted = m_int_weight_btn->GetValue();

	if (m_auto_update)
		Colocalize();
}

void ColocalizationDlg::OnRatioBtn(wxCommandEvent &event)
{
	m_get_ratio = m_ratio_btn->GetValue();

	if (m_auto_update)
		Colocalize();
}

void ColocalizationDlg::OnPhysicalBtn(wxCommandEvent &event)
{
	m_physical_size = m_physical_btn->GetValue();

	if (m_auto_update)
		Colocalize();
}

void ColocalizationDlg::OnColorMapBtn(wxCommandEvent &event)
{
	m_colormap = m_colormap_btn->GetValue();

	if (m_auto_update)
		Colocalize();
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
	event.Skip();
}

void ColocalizationDlg::OnSelectCell(wxGridEvent& event)
{
	int r = event.GetRow();
	int c = event.GetCol();
	m_output_grid->SelectBlock(r, c, r, c);
	event.Skip();
}

void ColocalizationDlg::OnGridLabelClick(wxGridEvent& event)
{
	m_output_grid->SetFocus();
	event.Skip();
}
