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

#include <ConvertDlgAgent.h>
#include <ConvertDlg.h>
#include <Global.h>
#include <Names.h>

ConvertDlgAgent::ConvertDlgAgent(
	ConvertDlg* dlg) :
	Agent(dlg)
{

}

bool ConvertDlgAgent::Accept(
	const UpdateRequest& request) const
{
	return true;
}

void ConvertDlgAgent::Update(
	const UpdateRequest& request)
{
	if (request.dir == UpdateDir::DataToUI)
	{
		UpdateUI(request);
	}
	else if (request.dir == UpdateDir::UItoData)
	{
		UpdateData(request);
	}
}

void ConvertDlgAgent::UpdateUI(const UpdateRequest& request)
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	//update user interface
	if (FOUND_VALUE(gstNull))
		return;
	bool update_all = request.values.empty();

	double dval;
	int ival;
	bool bval;

	if (update_all || FOUND_VALUE(gstVolMeshThresh))
	{
		dval = glbin_conv_vol_mesh.GetIsoValue();
		m_cnv_vol_mesh_thresh_sldr->ChangeValue(std::round(dval * 100.0));
		m_cnv_vol_mesh_thresh_text->ChangeValue(wxString::Format("%.2f", dval));
	}

	if (update_all || FOUND_VALUE(gstVolMeshDownXY))
	{
		ival = glbin_conv_vol_mesh.GetDownsample();
		m_cnv_vol_mesh_downsample_sldr->ChangeValue(ival);
		m_cnv_vol_mesh_downsample_text->ChangeValue(wxString::Format("%d", ival));
	}

	if (update_all || FOUND_VALUE(gstVolMeshDownZ))
	{
		ival = glbin_conv_vol_mesh.GetDownsampleZ();
		m_cnv_vol_mesh_downsample_z_sldr->ChangeValue(ival);
		m_cnv_vol_mesh_downsample_z_text->ChangeValue(wxString::Format("%d", ival));
	}

	if (update_all || FOUND_VALUE(gstUseTransferFunc))
	{
		bval = glbin_conv_vol_mesh.GetUseTransfer();
		m_cnv_vol_mesh_usetransf_chk->SetValue(bval);
	}

	if (update_all || FOUND_VALUE(gstUseSelection))
	{
		bval = glbin_conv_vol_mesh.GetUseMask();
		m_cnv_vol_mesh_selected_chk->SetValue(bval);
	}

	if (update_all || FOUND_VALUE(gstVolMeshSimplify))
	{
		//settings
		dval = glbin_conv_vol_mesh.GetSimplify();
		m_cnv_vol_mesh_simplify_sldr->ChangeValue(std::round(dval * 100.0));
		m_cnv_vol_mesh_simplify_text->ChangeValue(wxString::Format("%.2f", dval));
	}

	if (update_all || FOUND_VALUE(gstVolMeshSmoothN))
	{
		//settings
		dval = glbin_conv_vol_mesh.GetSmoothStrength();
		m_cnv_vol_mesh_smooth_n_sldr->ChangeValue(std::round(dval * 100.0));
		m_cnv_vol_mesh_smooth_n_text->ChangeValue(wxString::Format("%.2f", dval));
	}

	if (update_all || FOUND_VALUE(gstVolMeshSmoothT))
	{
		//settings
		dval = glbin_conv_vol_mesh.GetSmoothScale();
		m_cnv_vol_mesh_smooth_t_sldr->ChangeValue(std::round(dval * 100.0));
		m_cnv_vol_mesh_smooth_t_text->ChangeValue(wxString::Format("%.2f", dval));
	}

	if (FOUND_VALUE(gstVolMeshInfo))
	{
		auto md = glbin_conv_vol_mesh.GetMeshData();
		if (md)
		{
			flrd::MeshStat stat(md.get());
			stat.Run();
			ConvertGridData data;
			data.area = stat.GetArea();
			data.volume = stat.GetVolume();
			data.vertex_count = stat.GetVertexNum();
			data.triangle_count = stat.GetTriangleNum();
			data.normal_count = stat.GetNormalNum();
			wxString unit_area, unit_vol;
			auto view = glbin_current.render_view.lock();
			if (view)
			{
				switch (view->m_sb_unit)
				{
				case 0:
					unit_area = L"nm\u00B2";
					unit_vol = L"nm\u00B3";
					break;
				case 1:
				default:
					unit_area = L"\u03BCm\u00B2";
					unit_vol = L"\u03BCm\u00B3";
					break;
				case 2:
					unit_area = L"mm\u00B2";
					unit_vol = L"mm\u00B3";
					break;
				}
			}
			SetOutput(data, unit_area, unit_vol);
		}
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
		if (transf_update && !glbin_conv_vol_mesh.GetUseTransfer())
			return;
		if (brush_update && !glbin_conv_vol_mesh.GetUseMask())
			return;
		if (glbin_conv_vol_mesh.GetAutoUpdate())
			glbin_conv_vol_mesh.Update(false);
	}
}

void ConvertDlgAgent::UpdateData(const UpdateRequest& request)
{

}

ConvertDlg* ConvertDlgAgent::GetDialog() const
{
	return static_cast<ConvertDlg*>(GetWindow());
}
