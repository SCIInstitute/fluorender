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

#include <VolumeDefault.h>
#include <Global.h>
#include <Names.h>
#include <VolumeData.h>
#include <VolumeGroup.h>
#include <RenderView.h>
#include <Color.h>
#include <BaseTreeFile.h>
#include <TreeFileFactory.h>
#include <TextureRenderer.h>
#include <ShaderProgram.h>

VolumeDataDefault::VolumeDataDefault()
{
	//transfer function settings
	m_gamma_enable = true;
	m_gamma = 1.0;

	m_boundary_enable = true;
	m_boundary_low = 0.0;
	m_boundary_high = 0.5;
	m_boundary_max = 0.5;

	m_minmax_enable = true;
	m_lo_offset = 0.0;
	m_hi_offset = 1.0;

	m_thresh_enable = true;
	m_lo_thresh = 0.0;
	m_hi_thresh = 1.0;
	m_sw = 0.0;

	m_luminance_enable = true;
	m_luminance = 1.0;

	m_alpha_enable = true;
	m_alpha = 1.0;

	//shading
	m_shading_enable = false;
	m_shading_strength = 1.0;
	m_shading_shine = 1.0;

	//shadow
	m_shadow_enable = false;
	m_shadow_intensity = 0.0;

	m_sample_rate_enable = true;
	m_sample_rate = 2.0;

	m_spacing = fluo::Vector(1.0);

	//colormap mode
	m_colormap_disp = false;
	m_colormap_low_value = 0.0;
	m_colormap_hi_value = 1.0;
	m_colormap_inv = 1.0;
	m_colormap_type = 0;
	m_colormap_proj = flvr::ColormapProj::Intensity;

	m_noise_rd = true;
	m_interpolate = true;
	m_inverted = false;
	m_render_mode = flvr::RenderMode::Standard;
	m_transparent = false;

	//blend mode
	m_channel_mix_mode = ChannelMixMode::CompositeAdd;
	//legend
	m_legend = true;
	//mask mode
	m_main_mode = flvr::ColorMode::SingleColor;
	m_mask_mode = flvr::ColorMode::SingleColor;
}

VolumeDataDefault::~VolumeDataDefault()
{

}

void VolumeDataDefault::Read()
{
	std::shared_ptr<BaseTreeFile> f =
		glbin_tree_file_factory.getTreeFile(gstConfigFile);
	if (!f)
		return;

	int ival;

	if (f->Exists("/volume default"))
		f->SetPath("/volume default");

	f->Read(gstGammaEnable, &m_gamma_enable, true);
	f->Read(gstGamma3d, &m_gamma, 1.0);

	f->Read(gstBoundaryEnable, &m_boundary_enable, true);
	f->Read(gstBoundaryLow, &m_boundary_low, 0.0);
	f->Read(gstBoundaryHigh, &m_boundary_high, 0.5);
	f->Read(gstBoundaryMax, &m_boundary_max, 0.5);

	f->Read(gstMinMaxEnable, &m_minmax_enable, true);
	f->Read(gstLowOffset, &m_lo_offset, 0.0);
	f->Read(gstHighOffset, &m_hi_offset, 1.0);

	f->Read(gstThresholdEnable, &m_thresh_enable, true);
	f->Read(gstLowThreshold, &m_lo_thresh, 0.0);
	f->Read(gstHighThreshold, &m_hi_thresh, 1.0);
	f->Read("soft thresh", &m_sw, 0.0);

	f->Read(gstLuminanceEnable, &m_luminance_enable, true);
	f->Read(gstLuminance, &m_luminance, 1.0);

	f->Read(gstAlphaEnable, &m_alpha_enable, true);
	f->Read(gstAlpha, &m_alpha, 1.0);

	f->Read(gstShadingEnable, &m_shading_enable, false);
	f->Read(gstShadingStrength, &m_shading_strength, 1.0);
	f->Read(gstShadingShine, &m_shading_shine, 1.0);

	f->Read(gstShadowEnable, &m_shadow_enable, false);
	f->Read(gstShadowInt, &m_shadow_intensity, 0.0);

	f->Read(gstSampleRateEnable, &m_sample_rate_enable, true);
	f->Read(gstSampleRate, &m_sample_rate, 2.0);

	f->Read(gstSpacing, &m_spacing, fluo::Vector(1.0));

	f->Read(gstColormapDisp, &m_colormap_disp, false);
	f->Read(gstColormapLow, &m_colormap_low_value, 0.0);
	f->Read(gstColormapHigh, &m_colormap_hi_value, 1.0);
	f->Read(gstColormapInv, &m_colormap_inv, 1.0);
	f->Read(gstColormapType, &m_colormap_type, 0);
	f->Read(gstColormapProj, &ival, 0);
	m_colormap_proj = static_cast<flvr::ColormapProj>(ival);

	f->Read(gstNoiseRedct, &m_noise_rd, true);
	f->Read(gstInterpolate, &m_interpolate, true);
	f->Read(gstInvert, &m_inverted, false);
	f->Read(gstRenderMode, &ival, 1);
	m_render_mode = static_cast<flvr::RenderMode>(ival);
	f->Read(gstTransparent, &m_transparent, false);

	f->Read(gstChannelMixMode, &ival, 3);
	m_channel_mix_mode = static_cast<ChannelMixMode>(ival);
	f->Read(gstLegend, &m_legend, true);
	f->Read(gstMainMode, &ival, 1);
	m_main_mode = static_cast<flvr::ColorMode>(ival);
	f->Read(gstMaskMode, &ival, 1);
	m_mask_mode = static_cast<flvr::ColorMode>(ival);
}

void VolumeDataDefault::Save()
{
	std::shared_ptr<BaseTreeFile> f =
		glbin_tree_file_factory.getTreeFile(gstConfigFile);
	if (!f)
		return;

	f->SetPath("/volume default");

	f->Write(gstGammaEnable, m_gamma_enable);
	f->Write(gstGamma3d, m_gamma);

	f->Write(gstBoundaryEnable, m_boundary_enable);
	f->Write(gstBoundaryLow, m_boundary_low);
	f->Write(gstBoundaryHigh, m_boundary_high);
	f->Write(gstBoundaryMax, m_boundary_max);

	f->Write(gstMinMaxEnable, m_minmax_enable);
	f->Write(gstLowOffset, m_lo_offset);
	f->Write(gstHighOffset, m_hi_offset);

	f->Write(gstThresholdEnable, m_thresh_enable);
	f->Write(gstLowThreshold, m_lo_thresh);
	f->Write(gstHighThreshold, m_hi_thresh);
	f->Write("soft thresh", m_sw);

	f->Write(gstLuminanceEnable, m_luminance_enable);
	f->Write(gstLuminance, m_luminance);

	f->Write(gstAlphaEnable, m_alpha_enable);
	f->Write(gstAlpha, m_alpha);

	f->Write(gstShadingEnable, m_shading_enable);
	f->Write(gstShadingStrength, m_shading_strength);
	f->Write(gstShadingShine, m_shading_shine);

	f->Write(gstShadowEnable, m_shadow_enable);
	f->Write(gstShadowInt, m_shadow_intensity);

	f->Write(gstSampleRateEnable, m_sample_rate_enable);
	f->Write(gstSampleRate, m_sample_rate);

	f->Write(gstSpacing, m_spacing);

	f->Write(gstColormapDisp, m_colormap_disp);
	f->Write(gstColormapLow, m_colormap_low_value);
	f->Write(gstColormapHigh, m_colormap_hi_value);
	f->Write(gstColormapInv, m_colormap_inv);
	f->Write(gstColormapType, m_colormap_type);
	f->Write(gstColormapProj, static_cast<int>(m_colormap_proj));

	f->Write(gstNoiseRedct, m_noise_rd);
	f->Write(gstInterpolate, m_interpolate);
	f->Write(gstInvert, m_inverted);
	f->Write(gstRenderMode, static_cast<int>(m_render_mode));
	f->Write(gstTransparent, m_transparent);

	f->Write(gstChannelMixMode, static_cast<int>(m_channel_mix_mode));
	f->Write(gstLegend, m_legend);

	f->Write(gstMainMode, static_cast<int>(m_main_mode));
	f->Write(gstMaskMode, static_cast<int>(m_mask_mode));
}

void VolumeDataDefault::Set(VolumeData* vd)
{
	if (!vd)
		return;

	m_gamma_enable = vd->GetGammaEnable();
	m_gamma = vd->GetGamma();

	m_boundary_enable = vd->GetBoundaryEnable();
	m_boundary_low = vd->GetBoundaryLow();
	m_boundary_high = vd->GetBoundaryHigh();
	m_boundary_max = vd->GetBoundaryMax();

	m_minmax_enable = vd->GetMinMaxEnable();
	m_lo_offset = vd->GetLowOffset();
	m_hi_offset = vd->GetHighOffset();

	m_thresh_enable = vd->GetThreshEnable();
	m_lo_thresh = vd->GetLeftThresh();
	m_hi_thresh = vd->GetRightThresh();
	m_sw = vd->GetSoftThreshold();

	m_luminance_enable = vd->GetLuminanceEnable();
	m_luminance = vd->GetLuminance();

	m_alpha_enable = vd->GetAlphaEnable();
	m_alpha = vd->GetAlpha();

	m_shading_enable = vd->GetShadingEnable();
	m_shading_strength = vd->GetShadingStrength();
	m_shading_shine = vd->GetShadingShine();

	m_shadow_enable = vd->GetShadingEnable();
	m_shadow_intensity = vd->GetShadowIntensity();

	m_sample_rate_enable = vd->GetSampleRateEnable();
	m_sample_rate = vd->GetSampleRate();

	m_spacing = vd->GetSpacing();

	m_colormap_disp = vd->GetColormapDisp();
	m_colormap_low_value = vd->GetColormapLow();
	m_colormap_hi_value = vd->GetColormapHigh();
	m_colormap_inv = vd->GetColormapInv();
	m_colormap_type = vd->GetColormap();
	m_colormap_proj = vd->GetColormapProj();

	m_noise_rd = vd->GetNR();
	m_interpolate = vd->GetInterpolate();
	m_inverted = vd->GetInvert();
	m_render_mode = vd->GetRenderMode();
	m_transparent = vd->GetTransparent();

	m_channel_mix_mode = vd->GetChannelMixMode();
	m_legend = vd->GetLegend();
	m_main_mode = vd->GetMainColorMode();
	m_mask_mode = vd->GetMaskColorMode();
}

void VolumeDataDefault::Apply(VolumeData* vd)
{
	if (!vd)
		return;

	auto res = vd->GetResolution();

	vd->SetGammaEnable(m_gamma_enable);
	vd->SetGamma(m_gamma);

	vd->SetBoundaryEnable(m_boundary_enable);
	vd->SetBoundaryLow(m_boundary_low);
	vd->SetBoundaryHigh(m_boundary_high);
	vd->SetBoundaryMax(m_boundary_max);

	vd->SetMinMaxEnable(m_minmax_enable);
	vd->SetLowOffset(vd->GetMinValueScale());
	vd->SetHighOffset(m_hi_offset);

	vd->SetThreshEnable(m_thresh_enable);
	vd->SetLeftThresh(m_lo_thresh);
	vd->SetRightThresh(m_hi_thresh);
	vd->SetSoftThreshold(m_sw);

	vd->SetLuminanceEnable(m_luminance_enable);
	vd->SetLuminance(m_luminance);

	vd->SetAlphaEnable(m_alpha_enable);
	vd->SetAlpha(m_alpha);

	if (res.intz() > 1)
		vd->SetShadingEnable(m_shading_enable);
	else
		vd->SetShadingEnable(false);
	vd->SetShadingStrength(m_shading_strength);
	vd->SetShadingShine(m_shading_shine);

	if (res.intz() > 1)
		vd->SetShadowEnable(m_shadow_enable);
	else
		vd->SetShadingEnable(false);
	vd->SetShadowIntensity(m_shadow_intensity);

	vd->SetSampleRateEnable(m_sample_rate_enable);
	vd->SetSampleRate(m_sample_rate);

	if (!vd->GetSpcFromFile())
		vd->SetBaseSpacing(m_spacing);

	vd->SetColormapDisp(m_colormap_disp);
	vd->SetColormapValues(m_colormap_low_value, m_colormap_hi_value);
	vd->SetColormapInv(m_colormap_inv);
	vd->SetColormap(m_colormap_type);
	vd->SetColormapProj(m_colormap_proj);

	vd->SetNR(m_noise_rd);
	vd->SetInterpolate(m_interpolate);
	vd->SetInvert(m_inverted);
	vd->SetRenderMode(m_render_mode);
	vd->SetTransparent(m_transparent);

	vd->SetChannelMixMode(m_channel_mix_mode);
	vd->SetLegend(m_legend);
	vd->SetMainMaskMode(m_main_mode);
	vd->SetMaskMode(m_mask_mode);
}

void VolumeDataDefault::Copy(VolumeData* v1, VolumeData* v2)//v2 to v1
{
	if (!v1 || !v2)
		return;

	v1->SetGammaEnable(v2->GetGammaEnable());
	v1->SetGamma(v2->GetGamma());

	v1->SetBoundaryEnable(v2->GetBoundaryEnable());
	v1->SetBoundaryLow(v2->GetBoundaryLow());
	v1->SetBoundaryHigh(v2->GetBoundaryHigh());
	v1->SetBoundaryMax(v2->GetBoundaryMax());

	v1->SetMinMaxEnable(v2->GetMinMaxEnable());
	v1->SetLowOffset(v2->GetLowOffset());
	v1->SetHighOffset(v2->GetHighOffset());

	v1->SetThreshEnable(v2->GetThreshEnable());
	v1->SetLeftThresh(v2->GetLeftThresh());
	v1->SetRightThresh(v2->GetRightThresh());
	v1->SetSoftThreshold(v2->GetSoftThreshold());

	v1->SetLuminanceEnable(v2->GetLuminanceEnable());
	v1->SetLuminance(v2->GetLuminance());

	v1->SetAlphaEnable(v2->GetAlphaEnable());
	v1->SetAlpha(v2->GetAlpha());

	v1->SetShadingEnable(v2->GetShadingEnable());
	v1->SetShadingStrength(v2->GetShadingStrength());
	v1->SetShadingShine(v2->GetShadingShine());

	v1->SetShadowEnable(v2->GetShadowEnable());
	v1->SetShadowIntensity(v2->GetShadowIntensity());

	v1->SetSampleRateEnable(v2->GetSampleRateEnable());
	v1->SetSampleRate(v2->GetSampleRate());

	v1->SetBaseSpacing(v2->GetSpacing());

	v1->SetColormapDisp(v2->GetColormapDisp());
	v1->SetColormapValues(v2->GetColormapLow(), v2->GetColormapHigh());
	v1->SetColormapInv(v2->GetColormapInv());
	v1->SetColormap(v2->GetColormap());
	v1->SetColormapProj(v2->GetColormapProj());

	v1->SetNR(v2->GetNR());
	v1->SetInterpolate(v2->GetInterpolate());
	v1->SetInvert(v2->GetInvert());
	v1->SetRenderMode(v2->GetRenderMode());
	v1->SetTransparent(v2->GetTransparent());

	v1->SetChannelMixMode(v2->GetChannelMixMode());
	v1->SetLegend(v2->GetLegend());
	v1->SetMainMaskMode(v2->GetMainColorMode());
	v1->SetMaskMode(v2->GetMaskColorMode());

	v1->SetColor(v2->GetColor());

	v1->SetGammaColor(v2->GetGammaColor());
	v1->SetBrightness(v2->GetBrightness());
	v1->SetHdr(v2->GetHdr());
	for (int i : { 0, 1, 2})
		v1->SetSync(i, v2->GetSync(i));

	v1->SetScalarScale(v2->GetScalarScale());
	v1->SetGMScale(v2->GetGMScale());
	v1->SetMinMaxValue(v2->GetMinValue(), v2->GetMaxValue());
}

void VolumeDataDefault::Apply(VolumeGroup* g)
{
	if (!g)
		return;

	//int resx, resy, resz;
	//g->GetResolution(resx, resy, resz);

	g->SetGammaEnable(m_gamma_enable);
	g->SetGamma(m_gamma);

	g->SetBoundaryEnable(m_boundary_enable);
	g->SetBoundaryLow(m_boundary_low);
	g->SetBoundaryHigh(m_boundary_high);
	g->SetBoundaryMax(m_boundary_max);

	g->SetMinMaxEnable(m_minmax_enable);
	g->SetLowOffset(m_lo_offset);
	g->SetHighOffset(m_hi_offset);

	g->SetThreshEnable(m_thresh_enable);
	g->SetLeftThresh(m_lo_thresh);
	g->SetRightThresh(m_hi_thresh);
	//g->SetSoftThreshold(m_sw);

	g->SetLuminanceEnable(m_luminance_enable);
	g->SetLuminance(m_luminance);

	g->SetAlphaEnable(m_alpha_enable);
	g->SetAlpha(m_alpha);

	g->SetShadingEnable(m_shading_enable);
	g->SetShadingStrength(m_shading_strength);
	g->SetShadingShine(m_shading_shine);

	g->SetShadowEnable(m_shadow_enable);
	g->SetShadowIntensity(m_shadow_intensity);

	g->SetSampleRateEnable(m_sample_rate_enable);
	g->SetSampleRate(m_sample_rate);

	//if (!g->GetSpcFromFile())
	//	g->SetBaseSpacings(m_spcx, m_spcy, m_spcz);

	g->SetColormapDisp(m_colormap_disp);
	g->SetColormapValues(m_colormap_low_value, m_colormap_hi_value);
	g->SetColormapInv(m_colormap_inv);
	g->SetColormap(m_colormap_type);
	g->SetColormapProj(m_colormap_proj);

	g->SetNR(m_noise_rd);
	g->SetInterpolate(m_interpolate);
	g->SetInvert(m_inverted);
	g->SetRenderMode(m_render_mode);
	g->SetTransparent(m_transparent);

	g->SetChannelMixMode(m_channel_mix_mode);
	//g->SetLegend(m_legend);

	g->SetMainMaskMode(m_main_mode);
	g->SetMaskMode(m_mask_mode);
}