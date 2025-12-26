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
#include <ConvertDlg.h>
#include <Global.h>
#include <Names.h>
#include <MainFrame.h>
#include <CurrentObjects.h>
#include <DataManager.h>
#include <VolumeData.h>
#include <MeshData.h>
#include <RenderView.h>
#include <BaseConvVolMesh.h>
#include <ColorMesh.h>
#include <VolumeSelector.h>
#include <wxSingleSlider.h>
#include <wx/valnum.h>
#include <wx/clipbrd.h>

ConvertDlg::ConvertDlg(MainFrame *frame) :
	TabbedPanel(frame, frame,
	wxDefaultPosition,
	frame->FromDIP(wxSize(500, 620)),
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

	//button
	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	m_cnv_vol_mesh_convert_btn = new wxButton(page, wxID_ANY, "Convert",
		wxDefaultPosition, FromDIP(wxSize(-1, 23)));
	m_cnv_vol_mesh_update_btn = new wxButton(page, wxID_ANY, "Update",
		wxDefaultPosition, FromDIP(wxSize(-1, 23)));
	m_cnv_vol_mesh_weld_btn = new wxButton(page, wxID_ANY, "Weld",
		wxDefaultPosition, FromDIP(wxSize(-1, 23)));
	m_cnv_vol_mesh_color_btn = new wxButton(page, wxID_ANY, "Color",
		wxDefaultPosition, FromDIP(wxSize(-1, 23)));
	m_cnv_vol_mesh_simplify_btn = new wxButton(page, wxID_ANY, "Simplify",
		wxDefaultPosition, FromDIP(wxSize(-1, 23)));
	m_cnv_vol_mesh_smooth_btn = new wxButton(page, wxID_ANY, "Smooth",
		wxDefaultPosition, FromDIP(wxSize(-1, 23)));
	m_cnv_vol_mesh_convert_btn->Bind(wxEVT_BUTTON, &ConvertDlg::OnCnvVolMeshConvert, this);
	m_cnv_vol_mesh_update_btn->Bind(wxEVT_BUTTON, &ConvertDlg::OnCnvVolMeshUpdate, this);
	m_cnv_vol_mesh_weld_btn->Bind(wxEVT_BUTTON, &ConvertDlg::OnCnvVolMeshWeldVertices, this);
	m_cnv_vol_mesh_color_btn->Bind(wxEVT_BUTTON, &ConvertDlg::OnCnvVolMeshColor, this);
	m_cnv_vol_mesh_simplify_btn->Bind(wxEVT_BUTTON, &ConvertDlg::OnCnvVolMeshSimplify, this);
	m_cnv_vol_mesh_smooth_btn->Bind(wxEVT_BUTTON, &ConvertDlg::OnCnvVolMeshSmooth, this);
	sizer_1->Add(m_cnv_vol_mesh_convert_btn, 0, wxALIGN_CENTER);
	sizer_1->Add(m_cnv_vol_mesh_update_btn, 0, wxALIGN_CENTER);
	sizer_1->Add(m_cnv_vol_mesh_weld_btn, 0, wxALIGN_CENTER);
	sizer_1->Add(m_cnv_vol_mesh_color_btn, 0, wxALIGN_CENTER);
	sizer_1->Add(m_cnv_vol_mesh_simplify_btn, 0, wxALIGN_CENTER);
	sizer_1->Add(m_cnv_vol_mesh_smooth_btn, 0, wxALIGN_CENTER);

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
	st = new wxStaticText(page, 0, "Smooth N:",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_cnv_vol_mesh_smooth_n_sldr = new wxSingleSlider(page, wxID_ANY, 30, 0, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_cnv_vol_mesh_smooth_n_text = new wxTextCtrl(page, wxID_ANY, "0.30",
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
	st = new wxStaticText(page, 0, "Smooth T:",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_cnv_vol_mesh_smooth_t_sldr = new wxSingleSlider(page, wxID_ANY, 30, 0, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_cnv_vol_mesh_smooth_t_text = new wxTextCtrl(page, wxID_ANY, "0.30",
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
	sizerV->Add(sizer_1, 0, wxEXPAND);
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
	m_output_grid->CreateGrid(0, 4);
	m_output_grid->SetColLabelValue(0, "Surface Area");
	m_output_grid->SetColLabelValue(1, "Volume");
	m_output_grid->SetColLabelValue(2, "Vertex Count");
	m_output_grid->SetColLabelValue(3, "Triangle Count");
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

void ConvertDlg::FluoUpdate(const fluo::ValueCollection& vc)
{
	//update user interface
	if (FOUND_VALUE(gstNull))
		return;
	bool update_all = vc.empty();

	double dval;
	int ival;
	bool bval;

	if (update_all || FOUND_VALUE(gstVolMeshThresh))
	{
		dval = glbin_conv_vol_mesh->GetIsoValue();
		m_cnv_vol_mesh_thresh_sldr->ChangeValue(std::round(dval * 100.0));
		m_cnv_vol_mesh_thresh_text->ChangeValue(wxString::Format("%.2f", dval));
	}

	if (update_all || FOUND_VALUE(gstVolMeshDownXY))
	{
		ival = glbin_conv_vol_mesh->GetDownsample();
		m_cnv_vol_mesh_downsample_sldr->ChangeValue(ival);
		m_cnv_vol_mesh_downsample_text->ChangeValue(wxString::Format("%d", ival));
	}

	if (update_all || FOUND_VALUE(gstVolMeshDownZ))
	{
		ival = glbin_conv_vol_mesh->GetDownsampleZ();
		m_cnv_vol_mesh_downsample_z_sldr->ChangeValue(ival);
		m_cnv_vol_mesh_downsample_z_text->ChangeValue(wxString::Format("%d", ival));
	}

	if (update_all || FOUND_VALUE(gstUseTransferFunc))
	{
		bval = glbin_conv_vol_mesh->GetUseTransfer();
		m_cnv_vol_mesh_usetransf_chk->SetValue(bval);
	}

	if (update_all || FOUND_VALUE(gstUseSelection))
	{
		bval = glbin_conv_vol_mesh->GetUseMask();
		m_cnv_vol_mesh_selected_chk->SetValue(bval);
	}

	if (FOUND_VALUE(gstVolMeshInfo))
	{
		//wxString str = "The surface area of mesh object ";
		//auto md = glbin_current.mesh_data.lock();
		//if (md)
		//{
		//	str += md->GetName();
		//	str += " is ";
		//	str += glbin_conv_vol_mesh->GetInfo();
		//}
		//(*m_stat_text) << str << "\n";
	}

	bool brush_update = FOUND_VALUE(gstBrushCountAutoUpdate);
	bool transf_update = FOUND_VALUE(gstConvVolMeshUpdateTransf);
	if (FOUND_VALUE(gstConvVolMeshUpdate) ||
		transf_update ||
		brush_update)
	{
		auto mode = glbin_vol_selector.GetSelectMode();
		if (mode == flrd::SelectMode::Segment ||
			mode == flrd::SelectMode::Mesh)
			return;
		if (transf_update && !glbin_conv_vol_mesh->GetUseTransfer())
			return;
		if (brush_update && !glbin_conv_vol_mesh->GetUseMask())
			return;
		if (glbin_conv_vol_mesh->GetAutoUpdate() &&
			!glbin_conv_vol_mesh->IsBusy())
			glbin_conv_vol_mesh->Update(false);
	}
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
	double val;
	if (str.ToDouble(&val))
	{
		m_cnv_vol_mesh_thresh_sldr->ChangeValue(std::round(val * 100.0));
		glbin_conv_vol_mesh->SetIsoValue(val);
	}

	FluoRefresh(2, { gstConvVolMeshUpdate });
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
		glbin_conv_vol_mesh->SetDownsample(ival);
	}

	FluoRefresh(2, { gstConvVolMeshUpdate });
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
		glbin_conv_vol_mesh->SetDownsampleZ(ival);
	}

	FluoRefresh(2, { gstConvVolMeshUpdate });
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
	double val;
	if (str.ToDouble(&val))
	{
		m_cnv_vol_mesh_simplify_sldr->ChangeValue(std::round(val * 100.0));
		glbin_conv_vol_mesh->SetSimplify(val);
	}

	FluoRefresh(2, { gstConvVolMeshUpdate });
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
	double val;
	if (str.ToDouble(&val))
	{
		m_cnv_vol_mesh_smooth_n_sldr->ChangeValue(std::round(val * 100.0));
		glbin_conv_vol_mesh->SetSmoothN(val);
	}

	FluoRefresh(2, { gstConvVolMeshUpdate });
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
	double val;
	if (str.ToDouble(&val))
	{
		m_cnv_vol_mesh_smooth_t_sldr->ChangeValue(std::round(val * 100.0));
		glbin_conv_vol_mesh->SetSmoothN(val);
	}

	FluoRefresh(2, { gstConvVolMeshUpdate });
}

void ConvertDlg::OnCnvVolMeshUseTransfCheck(wxCommandEvent& event)
{
	bool bval = m_cnv_vol_mesh_usetransf_chk->GetValue();
	glbin_conv_vol_mesh->SetUseTransfer(bval);
	FluoRefresh(2, { gstConvVolMeshUpdate });
}

void ConvertDlg::OnCnvVolMeshUseSelCheck(wxCommandEvent& event)
{
	bool bval = m_cnv_vol_mesh_selected_chk->GetValue();
	glbin_conv_vol_mesh->SetUseMask(bval);
	FluoRefresh(2, { gstConvVolMeshUpdate });
}

void ConvertDlg::OnCnvVolMeshConvert(wxCommandEvent& event)
{
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;
	glbin_conv_vol_mesh->SetVolumeData(vd);
	glbin_conv_vol_mesh->Convert();
	auto md = glbin_conv_vol_mesh->GetMeshData();
	if (md)
	{
		glbin_data_manager.AddMeshData(md);
		auto view = glbin_current.render_view.lock();
		if (view)
			view->AddMeshData(md);
		//glbin_current.SetMeshData(md);
	}

	FluoRefresh(0, { gstBrushThreshold, gstCompThreshold, gstVolMeshThresh, gstVolMeshInfo, gstListCtrl, gstTreeCtrl },
		{ glbin_current.GetViewId() });
}

void ConvertDlg::OnCnvVolMeshUpdate(wxCommandEvent& event)
{
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;
	glbin_conv_vol_mesh->SetVolumeData(vd);
	glbin_conv_vol_mesh->Update(true);
	auto md = glbin_conv_vol_mesh->GetMeshData();
	if (md)
	{
		auto temp = glbin_data_manager.GetMeshData(md->GetName());
		if (!temp)
		{
			glbin_data_manager.AddMeshData(md);
			auto view = glbin_current.render_view.lock();
			if (view)
				view->AddMeshData(md);
			//glbin_current.SetMeshData(md);
		}
	}

	FluoRefresh(0, { gstVolMeshInfo, gstListCtrl, gstTreeCtrl },
		{ glbin_current.GetViewId() });
}

void ConvertDlg::OnCnvVolMeshWeldVertices(wxCommandEvent& event)
{
	//bool bval = m_cnv_vol_mesh_weld_chk->GetValue();
	//glbin_conv_vol_mesh->SetVertexMerge(bval);
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;
	glbin_conv_vol_mesh->MergeVertices(true);
	FluoRefresh(0, { gstNull },
		{ glbin_current.GetViewId() });
}

void ConvertDlg::OnCnvVolMeshColor(wxCommandEvent& event)
{
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;
	auto md = glbin_conv_vol_mesh->GetMeshData();
	if (!md)
		return;
	if (vd->GetLabel(false))
	{
		glbin_color_mesh.SetUseSel(true);
		glbin_color_mesh.SetUseComp(true);
	}
	else if (vd->GetMask(false))
	{
		glbin_color_mesh.SetUseSel(true);
		glbin_color_mesh.SetUseComp(false);
	}
	else
	{
		glbin_color_mesh.SetUseSel(false);
		glbin_color_mesh.SetUseComp(false);
	}
	glbin_color_mesh.SetVolumeData(vd);
	glbin_color_mesh.SetMeshData(md);
	glbin_color_mesh.Update();
	FluoRefresh(0, { gstNull },
		{ glbin_current.GetViewId() });
}

void ConvertDlg::OnCnvVolMeshSimplify(wxCommandEvent& event)
{
	glbin_conv_vol_mesh->Simplify();
	FluoRefresh(0, { gstNull },
		{ glbin_current.GetViewId() });
}

void ConvertDlg::OnCnvVolMeshSmooth(wxCommandEvent& event)
{
	glbin_conv_vol_mesh->Smooth();
	FluoRefresh(0, { gstNull },
		{ glbin_current.GetViewId() });
}

//output
void ConvertDlg::SetOutput(const ConvertGridData& data, const wxString& unit)
{
	if (m_output_grid->GetNumberRows() == 0 ||
		m_hold_history)
	{
		m_output_grid->InsertRows();
	}
	m_output_grid->SetCellValue(0, 0,
		wxString::Format("%f", data.area));
	m_output_grid->SetCellValue(0, 1,
		wxString::Format("%f", data.volume));
	m_output_grid->SetCellValue(0, 2,
		wxString::Format("%d", data.vertex_count));
	m_output_grid->SetCellValue(0, 3,
		wxString::Format("%d", data.triangle_count));
	//m_output_grid->Fit();
	//m_output_grid->AutoSizeColumns();
	m_output_grid->ClearSelection();
}

void ConvertDlg::OnUpdateBtn(wxCommandEvent& event)
{
	FluoUpdate({ gstBrushCountResult });
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
		else if (event.GetKeyCode() == wxKeyCode('V'))
			PasteData();
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

void ConvertDlg::CopyData()
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

void ConvertDlg::PasteData()
{
	/*	wxString copy_data;
		wxString cur_field;
		wxString cur_line;
		int i, k, k2;

		if (wxTheClipboard->Open())
		{
			if (wxTheClipboard->IsSupported(wxDF_TEXT))
			{
				wxTextDataObject data;
				wxTheClipboard->GetData(data);
				copy_data = data.GetText();
			}
			wxTheClipboard->Close();
		}

		i = m_output_grid->GetGridCursorRow();
		k = m_output_grid->GetGridCursorCol();
		k2 = k;

		do
		{
			cur_line = copy_data.BeforeFirst('\n');
			copy_data = copy_data.AfterFirst('\n');
			do
			{
				cur_field = cur_line.BeforeFirst('\t');
				cur_line = cur_line.AfterFirst('\t');
				m_output_grid->SetCellValue(i, k, cur_field);
				k++;
			} while (cur_line.IsEmpty() == false);
			i++;
			k = k2;
		} while (copy_data.IsEmpty() == false);
	*/
}

