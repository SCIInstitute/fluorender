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
#include <Global.h>
#include <Names.h>
#include <MainSettings.h>

MeshPropPanelAgent::MeshPropPanelAgent(
	MeshPropPanel* panel) :
	Agent(panel)
{

}

bool MeshPropPanelAgent::Accept(
	const UpdateRequest& request) const
{
	return true;
}

void MeshPropPanelAgent::Update(
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

void MeshPropPanelAgent::UpdateUI(const UpdateRequest& request)
{
	auto panel = GetPanel();
	if (!panel)
		return;
	auto md = GetData();
	if (!md)
		return;

	if (FOUND_VALUE(gstNull))
		return;
	auto md = m_md.lock();
	if (!md)
		return;

	bool update_all = request.values.empty() || FOUND_VALUE(gstMeshProps);

	fluo::Color cval;
	double dval;
	wxString str;
	bool bval;

	//outline
	if (update_all || FOUND_VALUE(gstOutline))
	{
		bval = md->GetOutline();
		panel->UpdateOutline(bval);
	}

	//legend
	if (update_all || FOUND_VALUE(gstLegend))
	{
		bval = md->GetLegend();
		panel->UpdateLegend(bval);
	}

	//color
	if (update_all || FOUND_VALUE(gstMeshColor))
	{
		cval = md->GetDataColor();
		panel->UpdateMeshColor(cval);
	}

	//alpha
	if (update_all || FOUND_VALUE(gstMeshAlpha))
	{
		bval = md->GetAlphaEnable();
		dval = md->GetAlpha();
		panel->UpdateMeshAlpha(bval, dval);
	}

	//shading
	if (update_all || FOUND_VALUE(gstMeshShading))
	{
		bval = md->GetShading();
		double strength = md->GetShadingStrength();
		double shine = md->GetShadingShine();
		panel->UpdateMeshShading(bval, strength, shine);
	}

	//shadow
	if (update_all || FOUND_VALUE(gstMeshShadow))
	{
		bval = md->GetShadowEnable();
		dval = md->GetShadowIntensity();
		panel->UpdateMeshShadow(bval, dval);
	}
	//dir
	if (update_all || FOUND_VALUE(gstShadowDir))
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
	if (update_all || FOUND_VALUE(gstMeshScale))
	{
		bval = md->GetScalingEnable();
		auto vval = md->GetScaling();
		dval = vval.x();
		panel->UpdateMeshScale(bval, dval);
	}
}

void MeshPropPanelAgent::UpdateData(const UpdateRequest& request)
{

}
