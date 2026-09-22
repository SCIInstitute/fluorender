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

#include <MeshPropPanelAgent.h>
#include <MeshPropPanel.h>
#include <MeshData.h>
#include <MeshGroup.h>
#include <Global.h>
#include <Names.h>
#include <MainSettings.h>
#include <CurrentObjects.h>
#include <RenderView.h>
#include <Coordinator.h>

MeshPropPanelAgent::MeshPropPanelAgent(
	MeshPropPanel* panel,
	const std::shared_ptr<MeshData>& md) :
	Agent(panel),
	m_md(md)
{

}

MeshPropPanel* MeshPropPanelAgent::GetPanel() const
{
	return static_cast<MeshPropPanel*>(GetWindow());
}

void MeshPropPanelAgent::UpdateUI(const UpdateRequest& request)
{
	auto panel = GetPanel();
	if (!panel)
		return;
	auto md = GetData();
	if (!md)
		return;

	bool update_all = request.values.empty() || request.HasValue(gstMeshProps);

	fluo::Color cval;
	double dval;
	wxString str;
	bool bval;

	//outline
	if (update_all || request.HasValue(gstOutline))
	{
		bval = md->GetOutline();
		panel->UpdateOutline(bval);
	}

	//legend
	if (update_all || request.HasValue(gstLegend))
	{
		bval = md->GetLegend();
		panel->UpdateLegend(bval);
	}

	//color
	if (update_all || request.HasValue(gstMeshColor))
	{
		cval = md->GetDataColor();
		panel->UpdateMeshColor(cval);
	}

	//alpha
	if (update_all || request.HasValue(gstMeshAlpha))
	{
		bval = md->GetAlphaEnable();
		dval = md->GetAlpha();
		panel->UpdateMeshAlpha(bval, dval);
	}

	//shading
	if (update_all || request.HasValue(gstMeshShading))
	{
		bval = md->GetShading();
		double strength = md->GetShadingStrength();
		double shine = md->GetShadingShine();
		panel->UpdateMeshShading(bval, strength, shine);
	}

	//shadow
	if (update_all || request.HasValue(gstMeshShadow))
	{
		bval = md->GetShadowEnable();
		dval = md->GetShadowIntensity();
		panel->UpdateMeshShadow(bval, dval);
	}
	//dir
	if (update_all || request.HasValue(gstShadowDir))
	{
		bval = glbin_settings.m_shadow_dir;
		double dirx = glbin_settings.m_shadow_dir_x;
		double diry = glbin_settings.m_shadow_dir_y;
		if (dirx == 0.0 && diry == 0.0)
			dval = 0.0;
		else
			dval = r2d(atan2(glbin_settings.m_shadow_dir_y, glbin_settings.m_shadow_dir_x)) + 45.0;
		panel->UpdateShadowDir(bval, dval);
	}

	//scaling
	if (update_all || request.HasValue(gstMeshScale))
	{
		bval = md->GetScalingEnable();
		auto vval = md->GetScaling();
		dval = vval.x();
		panel->UpdateMeshScale(bval, dval);
	}
}

void MeshPropPanelAgent::UpdateData(const UpdateRequest& request)
{
	if (request.HasValue(gstMeshOutline))
	{
		SetOutline();
	}
	if (request.HasValue(gstMeshSyncGroup))
	{
		SetSyncGroup();
	}
	if (request.HasValue(gstLegend))
	{
		SetLegend();
	}
	if (request.HasValue(gstMeshResetDefault))
	{
		ResetDefault();
	}
	if (request.HasValue(gstMeshSaveDefault))
	{
		SaveDefault();
	}
	if (request.HasValue(gstMeshEnableShading))
	{
		SetEnableShading();
	}
	if (request.HasValue(gstMeshShading))
	{
		SetShading();
	}
	if (request.HasValue(gstMeshShine))
	{
		SetShine();
	}
	if (request.HasValue(gstMeshEnableAlpha))
	{
		SetEnableAlpha();
	}
	if (request.HasValue(gstMeshAlpha))
	{
		SetAlpha();
	}
	if (request.HasValue(gstMeshEnableScale))
	{
		SetEnableScale();
	}
	if (request.HasValue(gstMeshScale))
	{
		SetScale();
	}
	if (request.HasValue(gstMeshEnableShadow))
	{
		SetEnableShadow();
	}
	if (request.HasValue(gstMeshShadow))
	{
		SetShadow();
	}
	if (request.HasValue(gstEnableShadowDir))
	{
		SetEnableShadowDir();
	}
	if (request.HasValue(gstShadowDir))
	{
		SetShadowDir();
	}
}

void MeshPropPanelAgent::SetColor(const fluo::Color& color)
{
	auto md = m_md.lock();
	if (!md)
		return;

	md->SetColor(color);
	NotifyViewUpdate({ gstMeshColor, gstTreeColors });
}

void MeshPropPanelAgent::SetOutline()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	bool bval = panel->GetOutline();
	auto group = m_group.lock();
	auto md = m_md.lock();
	if (m_sync_group && group)
		group->SetOutline(bval);
	else if (md)
		md->SetOutline(bval);

	auto view = glbin_current.render_view.lock();
	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate({ gstOutline }, target);
}

void MeshPropPanelAgent::SetSyncGroup()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	m_sync_group = panel->GetSyncGroup();

	auto group = m_group.lock();
	auto md = m_md.lock();
	if (m_sync_group && group && md)
	{
		//outline
		group->SetOutline(md->GetOutline());
		//alpha
		group->SetAlphaEnable(md->GetAlphaEnable());
		group->SetAlpha(md->GetAlpha());
		//shading
		group->SetShading(md->GetShading());
		group->SetShadingStrength(md->GetShadingStrength());
		group->SetShadingShine(md->GetShadingShine());
		//shadow
		group->SetShadowEnable(md->GetShadowEnable());
		group->SetShadowIntensity(md->GetShadowIntensity());
		//scaling
		group->SetScalingEnable(md->GetScalingEnable());
		group->SetScaling(md->GetScaling());
	}
}

void MeshPropPanelAgent::SetLegend()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	bool bval = panel->GetLegend();

	auto group = m_group.lock();
	auto md = m_md.lock();
	if (m_sync_group && group)
		group->SetLegend(bval);
	else if (md)
		md->SetLegend(bval);

	auto view = glbin_current.render_view.lock();
	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate({ gstLegend }, target);
}

void MeshPropPanelAgent::SaveDefault()
{
	auto md = m_md.lock();
	if (md)
	{
		glbin_mesh_def.Set(*md);
	}
}

void MeshPropPanelAgent::ResetDefault()
{
	auto md = m_md.lock();
	if (!md)
		return;

	glbin_mesh_def.Apply(*md);

	auto view = glbin_current.render_view.lock();
	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate({ gstMeshProps }, target);
}

void MeshPropPanelAgent::SetEnableShading()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	bool bval = panel->GetShading();

	auto group = m_group.lock();
	auto md = m_md.lock();
	if (m_sync_group && group)
	{
		group->SetShading(bval);
	}
	else if (md)
	{
		md->SetShading(bval);
	}
	NotifyViewUpdate({ gstMeshShading });
}

void MeshPropPanelAgent::SetShading()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	double dval = panel->GetShading();

	auto group = m_group.lock();
	auto md = m_md.lock();
	if (m_sync_group && group)
	{
		group->SetShadingStrength(dval);
	}
	else if (md)
	{
		md->SetShadingStrength(dval);
	}
	NotifyViewUpdate({ gstMeshShading });
}

void MeshPropPanelAgent::SetShine()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	double dval = panel->GetShine();

	auto group = m_group.lock();
	auto md = m_md.lock();
	if (m_sync_group && group)
	{
		group->SetShadingShine(dval);
	}
	else if (md)
	{
		md->SetShadingShine(dval);
	}
	NotifyViewUpdate({ gstMeshShading });
}

void MeshPropPanelAgent::SetEnableAlpha()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	bool bval = panel->GetEnableAlpha();

	auto group = m_group.lock();
	auto md = m_md.lock();
	if (m_sync_group && group)
	{
		group->SetAlphaEnable(bval);
	}
	else if (md)
	{
		md->SetAlphaEnable(bval);
	}
	NotifyViewUpdate({ gstMeshAlpha });
}

void MeshPropPanelAgent::SetAlpha()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	double dval = panel->GetAlpha();

	auto group = m_group.lock();
	auto md = m_md.lock();
	if (m_sync_group && group)
	{
		group->SetAlpha(dval);
	}
	else if (md)
	{
		md->SetAlpha(dval);
	}
	NotifyViewUpdate({ gstMeshAlpha });
}

void MeshPropPanelAgent::SetEnableScale()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	bool bval = panel->GetEnableScale();

	auto group = m_group.lock();
	auto md = m_md.lock();
	if (m_sync_group && group)
	{
		group->SetScalingEnable(bval);
	}
	else if (md)
	{
		md->SetScalingEnable(bval);
	}
	NotifyViewUpdate({ gstMeshScale });
}

void MeshPropPanelAgent::SetScale()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	double dval = panel->GetScale();

	auto group = m_group.lock();
	auto md = m_md.lock();
	if (m_sync_group && group)
	{
		group->SetScaling(fluo::Vector(dval, dval, dval));
	}
	else if (md)
	{
		md->SetScaling(fluo::Vector(dval, dval, dval));
	}
	NotifyViewUpdate({ gstMeshScale });
}

void MeshPropPanelAgent::SetEnableShadow()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	bool bval = panel->GetEnableShadow();

	auto group = m_group.lock();
	auto md = m_md.lock();
	if (m_sync_group && group)
	{
		group->SetShadowEnable(bval);
	}
	else if (md)
	{
		md->SetShadowEnable(bval);
	}
	NotifyViewUpdate({ gstMeshShadow });
}

void MeshPropPanelAgent::SetShadow()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	double dval = panel->GetShadow();

	auto group = m_group.lock();
	auto md = m_md.lock();
	if (m_sync_group && group)
	{
		group->SetShadowIntensity(dval);
	}
	else if (md)
	{
		md->SetShadowIntensity(dval);
	}
	NotifyViewUpdate({ gstMeshShadow });
}

void MeshPropPanelAgent::SetEnableShadowDir()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	bool bval = panel->GetEnableShadowDir();
	double dval = panel->GetShadowDir();

	if (glbin_settings.m_shadow_dir == bval)
		return;

	glbin_settings.m_shadow_dir = bval;
	if (bval)
	{
		double deg = dval - 45.0;
		glbin_settings.m_shadow_dir_x = cos(d2r(deg));
		glbin_settings.m_shadow_dir_y = sin(d2r(deg));
	}
	else
	{
		glbin_settings.m_shadow_dir_x = 0.0;
		glbin_settings.m_shadow_dir_y = 0.0;
	}
	NotifyViewUpdate({ gstShadowDir });
}

void MeshPropPanelAgent::SetShadowDir()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	double dval = panel->GetShadowDir();

	double deg = dval - 45.0;
	double dir_x = cos(d2r(deg));
	double dir_y = sin(d2r(deg));
	if (glbin_settings.m_shadow_dir_x == dir_x &&
		glbin_settings.m_shadow_dir_y == dir_y)
		return;

	glbin_settings.m_shadow_dir_x = dir_x;
	glbin_settings.m_shadow_dir_y = dir_y;
	NotifyViewUpdate({ gstShadowDir });
	//else
	//	FluoRefresh(0, { gstNull }, { glbin_current.GetViewId() });
}
