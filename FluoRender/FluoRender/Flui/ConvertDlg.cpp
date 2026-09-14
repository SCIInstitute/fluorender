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
#include <ConvertDlg.h>
#include <ConvertDlgAgent.h>
#include <GridHelper.h>
#include <wxSingleSlider.h>
#include <wx/valnum.h>
#include <wx/clipbrd.h>
//resources
#include <png_resource.h>
#include <icons.h>

ConvertDlg::ConvertDlg(wxWindow *parent) :
	TabbedPanel(parent,
		wxDefaultPosition,
		parent->FromDIP(wxSize(500, 620)),
		0, "ConvertDlg"),
	m_hold_history(false)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);
	Freeze();
	SetDoubleBuffered(true);

	//notebook
	m_notebook = new wxAuiNotebook(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize,
		wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE |
		wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_TAB_EXTERNAL_MOVE | wxNO_BORDER);
	m_notebook->AddPage(CreateSettingPage(m_notebook), "Volume to Mesh", true);
	m_notebook->AddPage(CreateInfoPage(m_notebook), "Information");

	Bind(wxEVT_SIZE, &ConvertDlg::OnSize, this);

	//vertical sizer
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
	sizer_v->Add(m_notebook, 1, wxEXPAND | wxALL);

	SetSizer(sizer_v);
	Layout();
	SetAutoLayout(true);
	SetScrollRate(10, 10);
	Thaw();
}

ConvertDlg::~ConvertDlg()
{
}

wxWindow* ConvertDlg::CreateSettingPage(wxWindow* parent)
{
	wxScrolledWindow* page = new wxScrolledWindow(parent);

	//validator: floating point 2
	wxFloatingPointValidator<double> vald_fp2(2);
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;

	wxStaticText* st = 0;

	//toolbar
	m_toolbar = new wxToolBar(page, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxTB_FLAT | wxTB_TOP | wxTB_NODIVIDER | wxTB_TEXT);
	wxBitmapBundle bitmap;
	bitmap = wxGetBitmap(mesh_convert);
	m_toolbar->AddTool(
		ID_MeshConvert, "Convert", bitmap,
		"Convert volume data to mesh");
	m_toolbar->SetToolLongHelp(ID_MeshConvert, "Convert volume data to mesh");
	bitmap = wxGetBitmap(mesh_update);
	m_toolbar->AddTool(
		ID_MeshUpdate, "Update", bitmap,
		"Update mesh after setting change");
	m_toolbar->SetToolLongHelp(ID_MeshUpdate, "Update mesh after setting change");
	m_toolbar->AddSeparator();
	bitmap = wxGetBitmap(mesh_weld);
	m_toolbar->AddTool(
		ID_MeshWeldVertices, "Weld", bitmap,
		"Weld mesh vertices to remove duplications");
	m_toolbar->SetToolLongHelp(ID_MeshWeldVertices, "Weld mesh vertices to remove duplications");
	bitmap = wxGetBitmap(palette);
	m_toolbar->AddTool(
		ID_MeshColor, "Color", bitmap,
		"Transfer voxel colors to mesh");
	m_toolbar->SetToolLongHelp(ID_MeshColor, "Transfer voxel colors to mesh");
	m_toolbar->AddSeparator();
	bitmap = wxGetBitmap(mesh_simplify);
	m_toolbar->AddTool(
		ID_MeshSimplify, "Simplify", bitmap,
		"Simplify mesh by merging nearby vertices");
	m_toolbar->SetToolLongHelp(ID_MeshSimplify, "Simplify mesh by merging nearby vertices");
	bitmap = wxGetBitmap(mesh_smooth);
	m_toolbar->AddTool(
		ID_MeshSmooth, "Smooth", bitmap,
		"Move mesh vetices to reduce noise");
	m_toolbar->SetToolLongHelp(ID_MeshSmooth, "Move mesh vetices to reduce noise");
	m_toolbar->Bind(wxEVT_TOOL, &ConvertDlg::OnToolBar, this);
	m_toolbar->Realize();

	//sizer_2
	//convert from volume to mesh
	wxStaticBoxSizer* sizer_2 = new wxStaticBoxSizer(
		wxVERTICAL, page, "Mesh Generation");
	//check options and convert button
	wxBoxSizer* sizer_21 = new wxBoxSizer(wxHORIZONTAL);
	m_cnv_vol_mesh_selected_chk = new wxCheckBox(page, wxID_ANY, "Paint-Selected Data Only",
		wxDefaultPosition, FromDIP(wxSize(-1, 23)));
	m_cnv_vol_mesh_usetransf_chk = new wxCheckBox(page, wxID_ANY, "Use Volume Properties",
		wxDefaultPosition, FromDIP(wxSize(-1, 23)));
	m_cnv_vol_mesh_selected_chk->Bind(wxEVT_CHECKBOX, &ConvertDlg::OnCnvVolMeshUseSelCheck, this);
	m_cnv_vol_mesh_usetransf_chk->Bind(wxEVT_CHECKBOX, &ConvertDlg::OnCnvVolMeshUseTransfCheck, this);
	sizer_21->Add(m_cnv_vol_mesh_selected_chk, 0, wxALIGN_CENTER);
	sizer_21->Add(m_cnv_vol_mesh_usetransf_chk, 0, wxALIGN_CENTER);
	//threshold slider and text
	wxBoxSizer* sizer_22 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Threshold:",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_cnv_vol_mesh_thresh_sldr = new wxSingleSlider(page, wxID_ANY, 30, 1, 99,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_cnv_vol_mesh_thresh_text = new wxTextCtrl(page, wxID_ANY, "0.30",
		wxDefaultPosition, FromDIP(wxSize(40, 23)), wxTE_RIGHT, vald_fp2);
	m_cnv_vol_mesh_thresh_sldr->Bind(wxEVT_SCROLL_CHANGED, &ConvertDlg::OnCnvVolMeshThreshChange, this);
	m_cnv_vol_mesh_thresh_text->Bind(wxEVT_TEXT, &ConvertDlg::OnCnvVolMeshThreshText, this);
	sizer_22->Add(st, 0, wxALIGN_CENTER);
	sizer_22->Add(10, 10);
	sizer_22->Add(m_cnv_vol_mesh_thresh_sldr, 1, wxEXPAND);
	sizer_22->Add(m_cnv_vol_mesh_thresh_text, 0, wxALIGN_CENTER);
	sizer_22->Add(15, 15);
	//downsampling slider and text
	wxBoxSizer* sizer_23 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Downsmp. XY:",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_cnv_vol_mesh_downsample_sldr = new wxSingleSlider(page, wxID_ANY, 2, 1, 10,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_cnv_vol_mesh_downsample_text = new wxTextCtrl(page, wxID_ANY, "2",
		wxDefaultPosition, FromDIP(wxSize(40, 23)), wxTE_RIGHT, vald_int);
	m_cnv_vol_mesh_downsample_sldr->Bind(wxEVT_SCROLL_CHANGED, &ConvertDlg::OnCnvVolMeshDownsampleChange, this);
	m_cnv_vol_mesh_downsample_text->Bind(wxEVT_TEXT, &ConvertDlg::OnCnvVolMeshDownsampleText, this);
	sizer_23->Add(st, 0, wxALIGN_CENTER);
	sizer_23->Add(10, 10);
	sizer_23->Add(m_cnv_vol_mesh_downsample_sldr, 1, wxEXPAND);
	sizer_23->Add(m_cnv_vol_mesh_downsample_text, 0, wxALIGN_CENTER);
	sizer_23->Add(15, 15);
	//downsampling in z slider and text
	wxBoxSizer* sizer_24 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Downsmp. Z:",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_cnv_vol_mesh_downsample_z_sldr = new wxSingleSlider(page, wxID_ANY, 1, 1, 10,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_cnv_vol_mesh_downsample_z_text = new wxTextCtrl(page, wxID_ANY, "1",
		wxDefaultPosition, FromDIP(wxSize(40, 23)), wxTE_RIGHT, vald_int);
	m_cnv_vol_mesh_downsample_z_sldr->Bind(wxEVT_SCROLL_CHANGED, &ConvertDlg::OnCnvVolMeshDownsampleZChange, this);
	m_cnv_vol_mesh_downsample_z_text->Bind(wxEVT_TEXT, &ConvertDlg::OnCnvVolMeshDownsampleZText, this);
	sizer_24->Add(st, 0, wxALIGN_CENTER);
	sizer_24->Add(10, 10);
	sizer_24->Add(m_cnv_vol_mesh_downsample_z_sldr, 1, wxEXPAND);
	sizer_24->Add(m_cnv_vol_mesh_downsample_z_text, 0, wxALIGN_CENTER);
	sizer_24->Add(15, 15);

	//sizer_2
	sizer_2->Add(5, 5);
	sizer_2->Add(sizer_21, 0, wxEXPAND);
	sizer_2->Add(5, 5);
	sizer_2->Add(sizer_22, 0, wxEXPAND);
	sizer_2->Add(5, 5);
	sizer_2->Add(sizer_23, 0, wxEXPAND);
	sizer_2->Add(5, 5);
	sizer_2->Add(sizer_24, 0, wxEXPAND);
	sizer_2->Add(5, 5);

	//sizer_3
	//mesh processing
	wxStaticBoxSizer* sizer_3 = new wxStaticBoxSizer(
		wxVERTICAL, page, "Mesh Processing");
	//simplify
	wxBoxSizer* sizer_31 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Simplify:",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_cnv_vol_mesh_simplify_sldr = new wxSingleSlider(page, wxID_ANY, 30, 0, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_cnv_vol_mesh_simplify_text = new wxTextCtrl(page, wxID_ANY, "0.30",
		wxDefaultPosition, FromDIP(wxSize(40, 23)), wxTE_RIGHT, vald_fp2);
	m_cnv_vol_mesh_simplify_sldr->Bind(wxEVT_SCROLL_CHANGED, &ConvertDlg::OnCnvVolMeshSimplifyChange, this);
	m_cnv_vol_mesh_simplify_text->Bind(wxEVT_TEXT, &ConvertDlg::OnCnvVolMeshSimplifyText, this);
	sizer_31->Add(st, 0, wxALIGN_CENTER);
	sizer_31->Add(10, 10);
	sizer_31->Add(m_cnv_vol_mesh_simplify_sldr, 1, wxEXPAND);
	sizer_31->Add(m_cnv_vol_mesh_simplify_text, 0, wxALIGN_CENTER);
	sizer_31->Add(15, 15);
	//smooth n
	wxBoxSizer* sizer_32 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Smooth Strength:",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_cnv_vol_mesh_smooth_n_sldr = new wxSingleSlider(page, wxID_ANY, 10, 0, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_cnv_vol_mesh_smooth_n_text = new wxTextCtrl(page, wxID_ANY, "0.10",
		wxDefaultPosition, FromDIP(wxSize(40, 23)), wxTE_RIGHT, vald_fp2);
	m_cnv_vol_mesh_smooth_n_sldr->Bind(wxEVT_SCROLL_CHANGED, &ConvertDlg::OnCnvVolMeshSmoothNChange, this);
	m_cnv_vol_mesh_smooth_n_text->Bind(wxEVT_TEXT, &ConvertDlg::OnCnvVolMeshSmoothNText, this);
	sizer_32->Add(st, 0, wxALIGN_CENTER);
	sizer_32->Add(10, 10);
	sizer_32->Add(m_cnv_vol_mesh_smooth_n_sldr, 1, wxEXPAND);
	sizer_32->Add(m_cnv_vol_mesh_smooth_n_text, 0, wxALIGN_CENTER);
	sizer_32->Add(15, 15);
	//smooth t
	wxBoxSizer* sizer_33 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Smooth Scale:",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_cnv_vol_mesh_smooth_t_sldr = new wxSingleSlider(page, wxID_ANY, 10, 0, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_cnv_vol_mesh_smooth_t_text = new wxTextCtrl(page, wxID_ANY, "0.10",
		wxDefaultPosition, FromDIP(wxSize(40, 23)), wxTE_RIGHT, vald_fp2);
	m_cnv_vol_mesh_smooth_t_sldr->Bind(wxEVT_SCROLL_CHANGED, &ConvertDlg::OnCnvVolMeshSmoothTChange, this);
	m_cnv_vol_mesh_smooth_t_text->Bind(wxEVT_TEXT, &ConvertDlg::OnCnvVolMeshSmoothTText, this);
	sizer_33->Add(st, 0, wxALIGN_CENTER);
	sizer_33->Add(10, 10);
	sizer_33->Add(m_cnv_vol_mesh_smooth_t_sldr, 1, wxEXPAND);
	sizer_33->Add(m_cnv_vol_mesh_smooth_t_text, 0, wxALIGN_CENTER);
	sizer_33->Add(15, 15);
	//sizer_3
	sizer_3->Add(5, 5);
	sizer_3->Add(sizer_31, 0, wxEXPAND);
	sizer_3->Add(5, 5);
	sizer_3->Add(sizer_32, 0, wxEXPAND);
	sizer_3->Add(5, 5);
	sizer_3->Add(sizer_33, 0, wxEXPAND);
	sizer_3->Add(5, 5);

	//all controls
	wxBoxSizer* sizerV = new wxBoxSizer(wxVERTICAL);
	sizerV->Add(10, 10);
	sizerV->Add(m_toolbar, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_2, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_3, 0, wxEXPAND);
	sizerV->Add(10, 10);

	page->SetSizer(sizerV);
	page->SetAutoLayout(true);
	page->SetScrollRate(10, 10);
	return page;
}

wxWindow* ConvertDlg::CreateInfoPage(wxWindow* parent)
{
	wxScrolledWindow* page = new wxScrolledWindow(parent);

	//output
	wxBoxSizer* sizer1 = new wxBoxSizer(wxHORIZONTAL);
	m_update_btn = new wxButton(page, wxID_ANY, "Update",
		wxDefaultPosition, wxDefaultSize);
	m_update_btn->Bind(wxEVT_BUTTON, &ConvertDlg::OnUpdateBtn, this);
	m_history_chk = new wxCheckBox(page, wxID_ANY,
		"Hold History", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_history_chk->Bind(wxEVT_CHECKBOX, &ConvertDlg::OnHistoryChk, this);
	m_clear_hist_btn = new wxButton(page, wxID_ANY,
		"Clear History", wxDefaultPosition, wxDefaultSize);
	m_clear_hist_btn->Bind(wxEVT_BUTTON, &ConvertDlg::OnClearHistBtn, this);
	sizer1->Add(m_update_btn, 0, wxALIGN_CENTER);
	sizer1->AddStretchSpacer(1);
	sizer1->Add(m_history_chk, 0, wxALIGN_CENTER);
	sizer1->Add(5, 5);
	sizer1->Add(m_clear_hist_btn, 0, wxALIGN_CENTER);
	//grid
	m_output_grid = new wxGrid(page, wxID_ANY);
	m_output_grid->CreateGrid(0, 5);
	m_output_grid->SetColLabelValue(0, "Surface Area");
	m_output_grid->SetColLabelValue(1, "Volume");
	m_output_grid->SetColLabelValue(2, "Vertex Count");
	m_output_grid->SetColLabelValue(3, "Triangle Count");
	m_output_grid->SetColLabelValue(4, "Normal Count");
	//m_output_grid->Fit();
	m_output_grid->Bind(wxEVT_GRID_SELECT_CELL, &ConvertDlg::OnSelectCell, this);
	m_output_grid->Bind(wxEVT_KEY_DOWN, &ConvertDlg::OnKeyDown, this);

	//sizer
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
	sizer_v->Add(5, 5);
	sizer_v->Add(sizer1, 0, wxEXPAND);
	sizer_v->Add(5, 5);
	sizer_v->Add(m_output_grid, 1, wxEXPAND);
	sizer_v->Add(5, 5);

	page->SetSizer(sizer_v);
	page->SetAutoLayout(true);
	page->SetScrollRate(10, 10);
	return page;
}

void ConvertDlg::UpdateVolMeshThresh(double dval)
{
	m_cnv_vol_mesh_thresh_sldr->ChangeValue(std::round(dval * 100.0));
	m_cnv_vol_mesh_thresh_text->ChangeValue(wxString::Format("%.2f", dval));
}

void ConvertDlg::UpdateVolMeshDownXY(int ival)
{
	m_cnv_vol_mesh_downsample_sldr->ChangeValue(ival);
	m_cnv_vol_mesh_downsample_text->ChangeValue(wxString::Format("%d", ival));
}

void ConvertDlg::UpdateVolMeshDownZ(int ival)
{
	m_cnv_vol_mesh_downsample_z_sldr->ChangeValue(ival);
	m_cnv_vol_mesh_downsample_z_text->ChangeValue(wxString::Format("%d", ival));
}

void ConvertDlg::UpdateUseTransferFunc(bool bval)
{
	m_cnv_vol_mesh_usetransf_chk->SetValue(bval);
}

void ConvertDlg::UpdateUseSelection(bool bval)
{
	m_cnv_vol_mesh_selected_chk->SetValue(bval);
}

void ConvertDlg::UpdateVolMeshSimplify(double dval)
{
	m_cnv_vol_mesh_simplify_sldr->ChangeValue(std::round(dval * 100.0));
	m_cnv_vol_mesh_simplify_text->ChangeValue(wxString::Format("%.2f", dval));
}

void ConvertDlg::UpdateVolMeshSmoothN(double dval)
{
	m_cnv_vol_mesh_smooth_n_sldr->ChangeValue(std::round(dval * 100.0));
	m_cnv_vol_mesh_smooth_n_text->ChangeValue(wxString::Format("%.2f", dval));
}

void ConvertDlg::UpdateVolMeshSmoothT(double dval)
{
	m_cnv_vol_mesh_smooth_t_sldr->ChangeValue(std::round(dval * 100.0));
	m_cnv_vol_mesh_smooth_t_text->ChangeValue(wxString::Format("%.2f", dval));
}

void ConvertDlg::CopyData()
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

void ConvertDlg::UpdateGrid(const GridData& data)
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

//threshold
void ConvertDlg::OnCnvVolMeshThreshChange(wxScrollEvent& event)
{
	int ival = m_cnv_vol_mesh_thresh_sldr->GetValue();
	double val = double(ival)/100.0;
	wxString str = wxString::Format("%.2f", val);
	if (str != m_cnv_vol_mesh_thresh_text->GetValue())
		m_cnv_vol_mesh_thresh_text->SetValue(str);
}

void ConvertDlg::OnCnvVolMeshThreshText(wxCommandEvent& event)
{
	wxString str = m_cnv_vol_mesh_thresh_text->GetValue();
	double dval;
	if (str.ToDouble(&dval))
	{
		m_cnv_vol_mesh_thresh_sldr->ChangeValue(std::round(dval * 100.0));
		auto agent = m_agent->As<ConvertDlgAgent>();
		if (agent)
			agent->SetIsoValue(dval);
	}
}

//downsampling
void ConvertDlg::OnCnvVolMeshDownsampleChange(wxScrollEvent& event)
{
	int ival = m_cnv_vol_mesh_downsample_sldr->GetValue();
	wxString str = wxString::Format("%d", ival);
	if (str != m_cnv_vol_mesh_downsample_text->GetValue())
		m_cnv_vol_mesh_downsample_text->SetValue(str);
}

void ConvertDlg::OnCnvVolMeshDownsampleText(wxCommandEvent& event)
{
	wxString str = m_cnv_vol_mesh_downsample_text->GetValue();
	long ival;
	if (str.ToLong(&ival))
	{
		m_cnv_vol_mesh_downsample_sldr->ChangeValue(ival);
		auto agent = m_agent->As<ConvertDlgAgent>();
		if (agent)
			agent->SetDownSample(ival);
	}
}

//downsampling Z
void ConvertDlg::OnCnvVolMeshDownsampleZChange(wxScrollEvent& event)
{
	int ival = m_cnv_vol_mesh_downsample_z_sldr->GetValue();
	wxString str = wxString::Format("%d", ival);
	if (str != m_cnv_vol_mesh_downsample_z_text->GetValue())
		m_cnv_vol_mesh_downsample_z_text->SetValue(str);
}

void ConvertDlg::OnCnvVolMeshDownsampleZText(wxCommandEvent& event)
{
	wxString str = m_cnv_vol_mesh_downsample_z_text->GetValue();
	long ival;
	if (str.ToLong(&ival))
	{
		m_cnv_vol_mesh_downsample_z_sldr->ChangeValue(ival);
		auto agent = m_agent->As<ConvertDlgAgent>();
		if (agent)
			agent->SetDownSampleZ(ival);
	}
}

void ConvertDlg::OnCnvVolMeshSimplifyChange(wxScrollEvent& event)
{
	int ival = m_cnv_vol_mesh_simplify_sldr->GetValue();
	double val = ival / 100.0;
	wxString str = wxString::Format("%.2f", val);
	if (str != m_cnv_vol_mesh_simplify_text->GetValue())
		m_cnv_vol_mesh_simplify_text->SetValue(str);
}

void ConvertDlg::OnCnvVolMeshSimplifyText(wxCommandEvent& event)
{
	wxString str = m_cnv_vol_mesh_simplify_text->GetValue();
	double dval;
	if (str.ToDouble(&dval))
	{
		m_cnv_vol_mesh_simplify_sldr->ChangeValue(std::round(dval * 100.0));
		auto agent = m_agent->As<ConvertDlgAgent>();
		if (agent)
			agent->SetSimplify(dval);
	}
}

void ConvertDlg::OnCnvVolMeshSmoothNChange(wxScrollEvent& event)
{
	int ival = m_cnv_vol_mesh_smooth_n_sldr->GetValue();
	double val = ival / 100.0;
	wxString str = wxString::Format("%.2f", val);
	if (str != m_cnv_vol_mesh_smooth_n_text->GetValue())
		m_cnv_vol_mesh_smooth_n_text->SetValue(str);
}

void ConvertDlg::OnCnvVolMeshSmoothNText(wxCommandEvent& event)
{
	wxString str = m_cnv_vol_mesh_smooth_n_text->GetValue();
	double dval;
	if (str.ToDouble(&dval))
	{
		m_cnv_vol_mesh_smooth_n_sldr->ChangeValue(std::round(dval * 100.0));
		auto agent = m_agent->As<ConvertDlgAgent>();
		if (agent)
			agent->SetSmoothStrength(dval);
	}
}

void ConvertDlg::OnCnvVolMeshSmoothTChange(wxScrollEvent& event)
{
	int ival = m_cnv_vol_mesh_smooth_t_sldr->GetValue();
	double val = ival / 100.0;
	wxString str = wxString::Format("%.2f", val);
	if (str != m_cnv_vol_mesh_smooth_t_text->GetValue())
		m_cnv_vol_mesh_smooth_t_text->SetValue(str);
}

void ConvertDlg::OnCnvVolMeshSmoothTText(wxCommandEvent& event)
{
	wxString str = m_cnv_vol_mesh_smooth_t_text->GetValue();
	double dval;
	if (str.ToDouble(&dval))
	{
		m_cnv_vol_mesh_smooth_t_sldr->ChangeValue(std::round(dval * 100.0));
		auto agent = m_agent->As<ConvertDlgAgent>();
		if (agent)
			agent->SetSmoothScale(dval);
	}
}

void ConvertDlg::OnCnvVolMeshUseTransfCheck(wxCommandEvent& event)
{
	bool bval = m_cnv_vol_mesh_usetransf_chk->GetValue();
	auto agent = m_agent->As<ConvertDlgAgent>();
	if (agent)
		agent->SetUseTransf(bval);
}

void ConvertDlg::OnCnvVolMeshUseSelCheck(wxCommandEvent& event)
{
	bool bval = m_cnv_vol_mesh_selected_chk->GetValue();
	auto agent = m_agent->As<ConvertDlgAgent>();
	if (agent)
		agent->SetUseSelection(bval);
}

void ConvertDlg::OnToolBar(wxCommandEvent& event)
{
	fluo::ValueCollection vc;
	int id = event.GetId();

	switch (id)
	{
	case ID_MeshConvert:
		vc.insert(gstMeshConvert);
		break;
	case ID_MeshUpdate:
		vc.insert(gstMeshUpdate);
		break;
	case ID_MeshWeldVertices:
		vc.insert(gstMeshWeldVertices);
		break;
	case ID_MeshColor:
		vc.insert(gstMeshColor);
		break;
	case ID_MeshSimplify:
		vc.insert(gstMeshSimplify);
		break;
	case ID_MeshSmooth:
		vc.insert(gstMeshSmooth);
		break;
	}

	auto agent = m_agent->As<ConvertDlgAgent>();
	if (agent)
		agent->UpdateUIToData(vc);
}

void ConvertDlg::OnUpdateBtn(wxCommandEvent& event)
{
	auto agent = m_agent->As<ConvertDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstVolMeshInfo });
}

void ConvertDlg::OnHistoryChk(wxCommandEvent& event)
{
	m_hold_history = m_history_chk->GetValue();
}

void ConvertDlg::OnClearHistBtn(wxCommandEvent& event)
{
	m_output_grid->DeleteRows(0, m_output_grid->GetNumberRows());
}

void ConvertDlg::OnKeyDown(wxKeyEvent& event)
{
	if (wxGetKeyState(WXK_CONTROL))
	{
		if (event.GetKeyCode() == wxKeyCode('C'))
			CopyData();
		//else if (event.GetKeyCode() == wxKeyCode('V'))
		//	PasteData();
	}
}

void ConvertDlg::OnSelectCell(wxGridEvent& event)
{
	int r = event.GetRow();
	int c = event.GetCol();
	m_output_grid->SelectBlock(r, c, r, c);
}

void ConvertDlg::OnSize(wxSizeEvent& event)
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

