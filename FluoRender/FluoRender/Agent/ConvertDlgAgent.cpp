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
#include <Coordinator.h>
#include <MeshData.h>
#include <VolumeData.h>
#include <ConvVolMesh.h>
#include <MeshStat.h>
#include <CurrentObjects.h>
#include <RenderView.h>
#include <VolumeSelector.h>
#include <DataManager.h>
#include <ColorMesh.h>
#include <GridBuilder.h>

ConvertDlgAgent::ConvertDlgAgent(
	ConvertDlg* dlg) :
	Agent(dlg)
{

}

void ConvertDlgAgent::UpdateUI(const UpdateRequest& request)
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	bool update_all = request.values.empty();

	double dval;
	int ival;
	bool bval;

	if (update_all || request.HasValue(gstVolMeshThresh))
	{
		dval = glbin_conv_vol_mesh.GetIsoValue();
		dlg->UpdateVolMeshThresh(dval);
	}

	if (update_all || request.HasValue(gstVolMeshDownXY))
	{
		ival = glbin_conv_vol_mesh.GetDownsample();
		dlg->UpdateVolMeshDownXY(ival);
	}

	if (update_all || request.HasValue(gstVolMeshDownZ))
	{
		ival = glbin_conv_vol_mesh.GetDownsampleZ();
		dlg->UpdateVolMeshDownZ(ival);
	}

	if (update_all || request.HasValue(gstUseTransferFunc))
	{
		bval = glbin_conv_vol_mesh.GetUseTransfer();
		dlg->UpdateUseTransferFunc(bval);
	}

	if (update_all || request.HasValue(gstUseSelection))
	{
		bval = glbin_conv_vol_mesh.GetUseMask();
		dlg->UpdateUseSelection(bval);
	}

	if (update_all || request.HasValue(gstVolMeshSimplify))
	{
		//settings
		dval = glbin_conv_vol_mesh.GetSimplify();
		dlg->UpdateVolMeshSimplify(dval);
	}

	if (update_all || request.HasValue(gstVolMeshSmoothN))
	{
		//settings
		dval = glbin_conv_vol_mesh.GetSmoothStrength();
		dlg->UpdateVolMeshSmoothN(dval);
	}

	if (update_all || request.HasValue(gstVolMeshSmoothT))
	{
		//settings
		dval = glbin_conv_vol_mesh.GetSmoothScale();
		dlg->UpdateVolMeshSmoothT(dval);
	}

	if (request.HasValue(gstVolMeshInfo))
	{
		auto md = glbin_conv_vol_mesh.GetMeshData();
		if (md)
		{
			std::string titles =
				"Surface Area\t" \
				"Volume\t" \
				"Vertex Count\t" \
				"Triangle Count\t" \
				"Normal Count\n";
			std::wstring values;
			flrd::MeshStat stat(md.get());
			stat.Run();
			ConvertGridData data;
			data.area = stat.GetArea();
			data.volume = stat.GetVolume();
			data.vertex_count = stat.GetVertexNum();
			data.triangle_count = stat.GetTriangleNum();
			data.normal_count = stat.GetNormalNum();
			std::wstring unit_area, unit_vol;
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
			values += std::to_wstring(data.area) + unit_area + L"\t";
			values += std::to_wstring(data.volume) + unit_vol + L"\t";
			values += std::to_wstring(data.vertex_count) + L"\t";
			values += std::to_wstring(data.triangle_count) + L"\t";
			values += std::to_wstring(data.normal_count) + L"\n";
			auto griddata = GridBuilder::Build(titles, ws2s(values));
			dlg->UpdateGrid(griddata);
		}
	}

	bool brush_update = request.HasValue(gstBrushCountAutoUpdate);
	bool transf_update = request.HasValue(gstConvVolMeshUpdateTransf);
	if (request.HasValue(gstConvVolMeshUpdate) ||
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
	if (request.HasValue(gstMeshConvert))
		MeshConvert();
	if (request.HasValue(gstMeshUpdate))
		MeshUpdate();
	if (request.HasValue(gstMeshWeldVertices))
		MeshWeldVertices();
	if (request.HasValue(gstMeshColor))
		MeshColor();
	if (request.HasValue(gstMeshSimplify))
		MeshSimplify();
	if (request.HasValue(gstMeshSmooth))
		MeshSmooth();
}

ConvertDlg* ConvertDlgAgent::GetDialog() const
{
	return static_cast<ConvertDlg*>(GetWindow());
}

void ConvertDlgAgent::SetIsoValue(double dval)
{
	glbin_conv_vol_mesh.SetIsoValue(dval);

	NotifyViewUpdate({ gstConvVolMeshUpdate }, UpdateMode::SenderOnly);
}

void ConvertDlgAgent::SetDownSample(int ival)
{
	glbin_conv_vol_mesh.SetDownsample(ival);
	
	NotifyViewUpdate({ gstConvVolMeshUpdate }, UpdateMode::SenderOnly);
}

void ConvertDlgAgent::SetDownSampleZ(int ival)
{
	glbin_conv_vol_mesh.SetDownsampleZ(ival);

	NotifyViewUpdate({ gstConvVolMeshUpdate }, UpdateMode::SenderOnly);
}

void ConvertDlgAgent::SetSimplify(double dval)
{
	glbin_conv_vol_mesh.SetSimplify(dval);
	//FluoRefresh(2, { gstVolMeshInfo });
}

void ConvertDlgAgent::SetSmoothStrength(double dval)
{
	glbin_conv_vol_mesh.SetSmoothStrength(dval);
	//FluoRefresh(2, { gstVolMeshInfo });
}

void ConvertDlgAgent::SetSmoothScale(double dval)
{
	glbin_conv_vol_mesh.SetSmoothScale(dval);
	//FluoRefresh(2, { gstVolMeshInfo });
}

void ConvertDlgAgent::SetUseTransf(bool bval)
{
	glbin_conv_vol_mesh.SetUseTransfer(bval);
	NotifyViewUpdate({ gstConvVolMeshUpdate }, UpdateMode::SenderOnly);
}

void ConvertDlgAgent::SetUseSelection(bool bval)
{
	glbin_conv_vol_mesh.SetUseMask(bval);
	NotifyViewUpdate({ gstConvVolMeshUpdate }, UpdateMode::SenderOnly);
}

void ConvertDlgAgent::MeshConvert()
{
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;
	glbin_conv_vol_mesh.SetVolumeData(vd);
	glbin_conv_vol_mesh.Convert();
	auto md = glbin_conv_vol_mesh.GetMeshData();
	if (md)
	{
		glbin_data_manager.AddMeshData(md);
		view->AddMeshData(md);
		//glbin_current.SetMeshData(md);
	}

	NotifyViewUpdate(
		{ gstVolMeshInfo, gstBrushThreshold, gstCompThreshold, gstVolMeshThresh, gstListCtrl, gstTreeCtrl },
		{ glbin_coordinator.FindRenderCanvasAgent(view) });
}

void ConvertDlgAgent::MeshUpdate()
{
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;
	glbin_conv_vol_mesh.SetVolumeData(vd);
	glbin_conv_vol_mesh.Update(true);
	auto md = glbin_conv_vol_mesh.GetMeshData();
	if (md)
	{
		auto temp = glbin_data_manager.GetMeshData(md->GetName());
		if (!temp)
		{
			glbin_data_manager.AddMeshData(md);
			view->AddMeshData(md);
			//glbin_current.SetMeshData(md);
		}
	}

	NotifyViewUpdate(
		{ gstVolMeshInfo, gstListCtrl, gstTreeCtrl },
		{ glbin_coordinator.FindRenderCanvasAgent(view) });
}

void ConvertDlgAgent::MeshWeldVertices()
{
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;
	glbin_conv_vol_mesh.MergeVertices(true);
	NotifyViewUpdate(
		{ gstVolMeshInfo },
		{ glbin_coordinator.FindRenderCanvasAgent(view) });
}

void ConvertDlgAgent::MeshColor()
{
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;
	auto md = glbin_conv_vol_mesh.GetMeshData();
	if (!md)
		return;
	auto view = glbin_current.render_view.lock();
	if (!view)
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
	NotifyViewUpdate(
		{ gstNull },
		{ glbin_coordinator.FindRenderCanvasAgent(view) });
}

void ConvertDlgAgent::MeshSimplify()
{
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;
	if (!glbin_conv_vol_mesh.GetMerged())
		glbin_conv_vol_mesh.MergeVertices(false);
	glbin_conv_vol_mesh.Simplify(true);
	NotifyViewUpdate(
		{ gstVolMeshInfo },
		{ glbin_coordinator.FindRenderCanvasAgent(view) });
}

void ConvertDlgAgent::MeshSmooth()
{
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;
	if (!glbin_conv_vol_mesh.GetMerged())
		glbin_conv_vol_mesh.MergeVertices(false);
	glbin_conv_vol_mesh.Smooth(true);
	NotifyViewUpdate(
		{ gstVolMeshInfo },
		{ glbin_coordinator.FindRenderCanvasAgent(view) });
}

