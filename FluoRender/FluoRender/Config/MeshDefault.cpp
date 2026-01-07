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

#include <MeshDefault.h>
#include <Global.h>
#include <Names.h>
#include <MeshData.h>
#include <MeshGroup.h>
#include <BaseConvVolMesh.h>
#include <BaseTreeFile.h>
#include <TreeFileFactory.h>

MeshDefault::MeshDefault()
{
}

MeshDefault::~MeshDefault()
{
}

void MeshDefault::Read()
{
	std::shared_ptr<BaseTreeFile> f =
		glbin_tree_file_factory.getTreeFile(gstConfigFile);
	if (!f)
		return;

	if (f->Exists("/mesh default"))
		f->SetPath("/mesh default");

	f->Read(gstUseTransferFunc, &m_use_transfer, false);
	f->Read(gstUseSelection, &m_use_mask, true);
	f->Read(gstVolMeshThresh, &m_iso, 0.5);
	f->Read(gstVolMeshDownXY, &m_downsample, 1);
	f->Read(gstVolMeshDownZ, &m_downsample_z, 1);
	f->Read(gstVolMeshSimplify, &m_simplify, 0.1);
	f->Read(gstVolMeshSmoothN, &m_smooth_strength, 0.1);
	f->Read(gstVolMeshSmoothT, &m_smooth_scale, 0.1);

	f->Read(gstMeshOutline, &m_outline, false);
	f->Read(gstMeshLegend, &m_legend, false);
	f->Read(gstMeshAlphaEnable, &m_alpha_enable, true);
	f->Read(gstMeshAlpha, &m_alpha, 0.7);
	f->Read(gstMeshShading, &m_shading, true);
	f->Read(gstMeshShadingStrength, &m_shading_strength, 1.0);
	f->Read(gstMeshShadingShine, &m_shading_shine, 1.0);
	f->Read(gstMeshShadowEnable, &m_shadow_enable, true);
	f->Read(gstMeshShadowIntensity, &m_shadow_intensity, 0.6);
	f->Read(gstMeshScalingEnable, &m_scaling_enable, false);
	f->Read(gstMeshScale, &m_scale, fluo::Vector(1.0));
}

void MeshDefault::Save()
{
	std::shared_ptr<BaseTreeFile> f =
		glbin_tree_file_factory.getTreeFile(gstConfigFile);
	if (!f)
		return;

	f->SetPath("/mesh default");

	f->Write(gstUseTransferFunc, m_use_transfer);
	f->Write(gstUseSelection, m_use_mask);
	f->Write(gstVolMeshThresh, m_iso);
	f->Write(gstVolMeshDownXY, m_downsample);
	f->Write(gstVolMeshDownZ, m_downsample_z);
	f->Write(gstVolMeshSimplify, m_simplify);
	f->Write(gstVolMeshSmoothN, m_smooth_strength);
	f->Write(gstVolMeshSmoothT, m_smooth_scale);

	f->Write(gstMeshOutline, m_outline);
	f->Write(gstMeshLegend, m_legend);
	f->Write(gstMeshAlphaEnable, m_alpha_enable);
	f->Write(gstMeshAlpha, m_alpha);
	f->Write(gstMeshShading, m_shading);
	f->Write(gstMeshShadingStrength, m_shading_strength);
	f->Write(gstMeshShadingShine, m_shading_shine);
	f->Write(gstMeshShadowEnable, m_shadow_enable);
	f->Write(gstMeshShadowIntensity, m_shadow_intensity);
	f->Write(gstMeshScalingEnable, m_scaling_enable);
	f->Write(gstMeshScale, m_scale);
}

void MeshDefault::Set(MeshData* md)
{
	if (!md)
		return;

	m_outline = md->GetOutline();
	m_legend = md->GetLegend();
	m_alpha_enable = md->GetAlphaEnable();
	m_alpha = md->GetAlpha();
	m_shading = md->GetShading();
	m_shading_strength = md->GetShadingStrength();
	m_shading_shine = md->GetShadingShine();
	m_shadow_enable = md->GetShadowEnable();
	m_shadow_intensity = md->GetShadowIntensity();
	m_scaling_enable = md->GetScalingEnable();
	m_scale = md->GetScaling();
}

void MeshDefault::Set(flrd::BaseConvVolMesh* cvm)
{
	if (!cvm)
		return;

	m_use_transfer = cvm->GetUseTransfer();
	m_use_mask = cvm->GetUseMask();
	m_iso = cvm->GetIsoValue();
	m_downsample = cvm->GetDownsample();
	m_downsample_z = cvm->GetDownsampleZ();
	m_simplify = cvm->GetSimplify();
	m_smooth_strength = cvm->GetSmoothStrength();
	m_smooth_scale = cvm->GetSmoothScale();
}

void MeshDefault::Apply(MeshData* md)
{
	if (!md)
		return;

	md->SetOutline(m_outline);
	md->SetLegend(m_legend);
	md->SetAlphaEnable(m_alpha_enable);
	md->SetAlpha(m_alpha);
	md->SetShading(m_shading);
	md->SetShadingStrength(m_shading_strength);
	md->SetShadingShine(m_shading_shine);
	md->SetShadowEnable(m_shadow_enable);
	md->SetShadowIntensity(m_shadow_intensity);
	md->SetScalingEnable(m_scaling_enable);
	md->SetScaling(m_scale);
}

void MeshDefault::Copy(MeshData* m1, MeshData* m2)//m2 to m1
{
	if (!m1 || !m2)
		return;

	m1->SetOutline(m2->GetOutline());
	m1->SetLegend(m2->GetLegend());
	m1->SetAlphaEnable(m2->GetAlphaEnable());
	m1->SetAlpha(m2->GetAlpha());
	m1->SetShading(m2->GetShading());
	m1->SetShadingStrength(m2->GetShadingStrength());
	m1->SetShadingShine(m2->GetShadingShine());
	m1->SetShadowEnable(m2->GetShadowEnable());
	m1->SetShadowIntensity(m2->GetShadowIntensity());
	m1->SetScalingEnable(m2->GetScalingEnable());
	m1->SetScaling(m2->GetScaling());
}

void MeshDefault::Apply(MeshGroup* g)
{
	if (!g)
		return;

	g->SetOutline(m_outline);
	g->SetLegend(m_legend);
	g->SetAlphaEnable(m_alpha_enable);
	g->SetAlpha(m_alpha);
	g->SetShading(m_shading);
	g->SetShadingStrength(m_shading_strength);
	g->SetShadingShine(m_shading_shine);
	g->SetShadowEnable(m_shadow_enable);
	g->SetShadowIntensity(m_shadow_intensity);
	g->SetScalingEnable(m_scaling_enable);
	g->SetScaling(m_scale);
}

void MeshDefault::Apply(flrd::BaseConvVolMesh* cvm)
{
	if (!cvm)
		return;

	cvm->SetUseTransfer(m_use_transfer);
	cvm->SetUseMask(m_use_mask);
	cvm->SetIsoValue(m_iso);
	cvm->SetDownsample(m_downsample);
	cvm->SetDownsampleZ(m_downsample_z);
	cvm->SetSimplify(m_simplify);
	cvm->SetSmooth(m_smooth_strength, m_smooth_scale);
}