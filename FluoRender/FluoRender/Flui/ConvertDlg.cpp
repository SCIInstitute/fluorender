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
#include <MeshData.h>
#include <RenderView.h>
#include <BaseConvVolMesh.h>
#include <VolumeSelector.h>
#include <wxSingleSlider.h>
#include <wx/valnum.h>

ConvertDlg::ConvertDlg(MainFrame *frame) :
	PropPanel(frame, frame,
	wxDefaultPosition,
	frame->FromDIP(wxSize(400, 300)),
	0, "ConvertDlg")	
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);
	SetDoubleBuffered(true);

	//validator: floating point 2
	wxFloatingPointValidator<double> vald_fp2(2);
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;

	wxStaticText *st = 0;

	//button
	wxBoxSizer *sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	m_cnv_vol_mesh_convert_btn = new wxButton(this, wxID_ANY, "Convert",
		wxDefaultPosition, FromDIP(wxSize(-1, 23)));
	m_cnv_vol_mesh_update_btn = new wxButton(this, wxID_ANY, "Update",
		wxDefaultPosition, FromDIP(wxSize(-1, 23)));
	m_cnv_vol_mesh_weld_btn = new wxButton(this, wxID_ANY, "Weld",
		wxDefaultPosition, FromDIP(wxSize(-1, 23)));
	m_cnv_vol_mesh_convert_btn->Bind(wxEVT_BUTTON, &ConvertDlg::OnCnvVolMeshConvert, this);
	m_cnv_vol_mesh_update_btn->Bind(wxEVT_BUTTON, &ConvertDlg::OnCnvVolMeshUpdate, this);
	m_cnv_vol_mesh_weld_btn->Bind(wxEVT_BUTTON, &ConvertDlg::OnCnvVolMeshWeldVertices, this);
	sizer_1->Add(m_cnv_vol_mesh_convert_btn, 0, wxALIGN_CENTER);
	sizer_1->Add(m_cnv_vol_mesh_update_btn, 0, wxALIGN_CENTER);
	sizer_1->Add(m_cnv_vol_mesh_weld_btn, 0, wxALIGN_CENTER);
	
	//sizer_2
	//convert from volume to mesh
	wxStaticBoxSizer *sizer_2 = new wxStaticBoxSizer(
		wxVERTICAL, this, "Volume to Polygon Mesh");
	//check options and convert button
	wxBoxSizer *sizer_21 = new wxBoxSizer(wxHORIZONTAL);
	m_cnv_vol_mesh_selected_chk = new wxCheckBox(this, wxID_ANY, "Paint-Selected Data Only",
		wxDefaultPosition, FromDIP(wxSize(-1, 23)));
	m_cnv_vol_mesh_usetransf_chk = new wxCheckBox(this, wxID_ANY, "Use Volume Properties",
		wxDefaultPosition, FromDIP(wxSize(-1, 23)));
	m_cnv_vol_mesh_selected_chk->Bind(wxEVT_CHECKBOX, &ConvertDlg::OnCnvVolMeshUseSelCheck, this);
	m_cnv_vol_mesh_usetransf_chk->Bind(wxEVT_CHECKBOX, &ConvertDlg::OnCnvVolMeshUseTransfCheck, this);
	sizer_21->Add(m_cnv_vol_mesh_selected_chk, 0, wxALIGN_CENTER);
	sizer_21->Add(m_cnv_vol_mesh_usetransf_chk, 0, wxALIGN_CENTER);
	//threshold slider and text
	wxBoxSizer *sizer_22 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Threshold:",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_cnv_vol_mesh_thresh_sldr = new wxSingleSlider(this, wxID_ANY, 30, 1, 99,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_cnv_vol_mesh_thresh_text = new wxTextCtrl(this, wxID_ANY, "0.30",
		wxDefaultPosition, FromDIP(wxSize(40, 23)), wxTE_RIGHT, vald_fp2);
	m_cnv_vol_mesh_thresh_sldr->Bind(wxEVT_SCROLL_CHANGED, &ConvertDlg::OnCnvVolMeshThreshChange, this);
	m_cnv_vol_mesh_thresh_text->Bind(wxEVT_TEXT, &ConvertDlg::OnCnvVolMeshThreshText, this);
	sizer_22->Add(st, 0, wxALIGN_CENTER);
	sizer_22->Add(10, 10);
	sizer_22->Add(m_cnv_vol_mesh_thresh_sldr, 1, wxEXPAND);
	sizer_22->Add(m_cnv_vol_mesh_thresh_text, 0, wxALIGN_CENTER);
	sizer_22->Add(15, 15);
	//downsampling slider and text
	wxBoxSizer *sizer_23 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Downsmp. XY:",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_cnv_vol_mesh_downsample_sldr = new wxSingleSlider(this, wxID_ANY, 2, 1, 10,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_cnv_vol_mesh_downsample_text = new wxTextCtrl(this, wxID_ANY, "2",
		wxDefaultPosition, FromDIP(wxSize(40, 23)), wxTE_RIGHT, vald_int);
	m_cnv_vol_mesh_downsample_sldr->Bind(wxEVT_SCROLL_CHANGED, &ConvertDlg::OnCnvVolMeshDownsampleChange, this);
	m_cnv_vol_mesh_downsample_text->Bind(wxEVT_TEXT, &ConvertDlg::OnCnvVolMeshDownsampleText, this);
	sizer_23->Add(st, 0, wxALIGN_CENTER);
	sizer_23->Add(10, 10);
	sizer_23->Add(m_cnv_vol_mesh_downsample_sldr, 1, wxEXPAND);
	sizer_23->Add(m_cnv_vol_mesh_downsample_text, 0, wxALIGN_CENTER);
	sizer_23->Add(15, 15);
	//downsampling in z slider and text
	wxBoxSizer *sizer_24 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Downsmp. Z:",
		wxDefaultPosition, FromDIP(wxSize(100, 23)));
	m_cnv_vol_mesh_downsample_z_sldr = new wxSingleSlider(this, wxID_ANY, 1, 1, 10,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_cnv_vol_mesh_downsample_z_text = new wxTextCtrl(this, wxID_ANY, "1",
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

	//all controls
	wxBoxSizer *sizerV = new wxBoxSizer(wxVERTICAL);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_1, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_2, 0, wxEXPAND);
	sizerV->Add(10, 10);

	SetSizer(sizerV);
	Layout();
}

ConvertDlg::~ConvertDlg()
{
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
