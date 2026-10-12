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

#include <VolumePropPanelAgent.h>
#include <VolumePropPanel.h>
#include <Global.h>
#include <Tokens.h>
#include <MainSettings.h>
#include <VolumeData.h>
#include <VolumeGroup.h>
#include <RenderView.h>
#include <ShaderProgram.h>
#include <Root.h>
#include <DataManager.h>

VolumePropPanelAgent::VolumePropPanelAgent(
	VolumePropPanel* panel,
	const std::shared_ptr<VolumeData>& vd,
	const std::shared_ptr<VolumeGroup>& group,
	const std::shared_ptr<RenderView>& view) :
	Agent(panel),
	m_vd(vd),
	m_group(group),
	m_view(view)
{
	if (group)
		m_sync_group = group->GetVolumeSyncProp();
}

VolumePropPanel* VolumePropPanelAgent::GetPanel() const
{
	return static_cast<VolumePropPanel*>(GetOwner());
}

void VolumePropPanelAgent::SetGroup(const std::shared_ptr<VolumeGroup>& group)
{
	m_group = group;
	if (group)
	{
		m_sync_group = group->GetVolumeSyncProp();
		UpdateDataToUI({ gstSyncGroup });
	}
}

void VolumePropPanelAgent::SetView(const std::shared_ptr<RenderView>& view)
{
	m_view = view;
}

double VolumePropPanelAgent::GetBoundaryMax()
{
	auto vd = m_vd.lock();
	if (vd)
		return vd->GetBoundaryMax();
	return 1.0;
}

void VolumePropPanelAgent::UpdateUI(const UpdateRequest& request)
{
	auto panel = GetPanel();
	if (!panel)
		return;
	auto vd = GetData();
	if (!vd)
		return;

	//std::chrono::time_point t = std::chrono::high_resolution_clock::now();

	double dval = 0.0;
	int ival = 0;
	bool bval;
	fluo::Color cval;

	//maximum value
	m_max_val = vd->GetMaxValue();
	m_max_val = std::max(255.0, m_max_val);

	bool update_all = request.values.empty() || request.HasValue(gstVolumeProps);
	bool update_tips = update_all || request.HasValue(gstMultiFuncTips);
	bool update_gamma = update_all || request.HasValue(gstGamma3d);
	bool update_boundary = update_all || request.HasValue(gstBoundary);
	bool update_minmax = update_all || request.HasValue(gstMinMax);
	bool update_threshold = update_all || request.HasValue(gstThreshold);
	bool update_color = update_all || request.HasValue(gstColor);
	bool update_alpha = update_all || request.HasValue(gstAlpha);
	bool update_luminance = update_all || request.HasValue(gstLuminance);
	bool update_shading = update_all || request.HasValue(gstShading);
	bool update_shadow = update_all || request.HasValue(gstShadow);
	bool update_sample = update_all || request.HasValue(gstSampleRate);
	bool update_colormap = update_all || request.HasValue(gstColormap);
	bool update_histogram = update_all || request.HasValue(gstUpdateHistogram);
	bool mf_enable = glbin_settings.m_mulfunc == 5;

	//DBGPRINT(L"update vol props, update_all=%d, vc_size=%d\n", update_all, vc.size());
	//mf button tips
	if (update_tips)
	{
		panel->UpdateMultiFuncTips(glbin_settings.m_mulfunc);
	}

	//volume properties
	//histogram
	if (update_histogram)
	{
		std::vector<unsigned char> hist_data;
		if (vd->GetHistogram(hist_data))
		{
			cval = vd->GetColor();
			panel->UpdateHistogram(cval, hist_data);
		}
	}
	//transfer function
	//gamma
	if (update_gamma)
	{
		dval = vd->GetGamma();
		bval = vd->GetGammaEnable();
		panel->UpdateGamma3d(bval, dval);
	}
	if (update_gamma || update_tips)
	{
		bval = vd->GetGammaEnable() || mf_enable;
		panel->UpdateGamma3dTips(bval);
	}
	//boundary
	if (update_boundary)
	{
		bval = vd->GetBoundaryEnable();
		double gmf = 1000 / vd->GetBoundaryMax();
		double low = vd->GetBoundaryLow();
		double hi = vd->GetBoundaryHigh();
		panel->UpdateBoundary(bval, low, hi, gmf);
	}
	if (update_boundary || update_tips)
	{
		bval = vd->GetBoundaryEnable() || mf_enable;
		panel->UpdateBoundaryTips(bval);
	}
	//minmax
	if (update_minmax || request.HasValue(gstTransparent))
	{
		dval = vd->GetLowOffset();
		int low = int(std::round(dval * m_max_val));
		int max;
		if (vd->GetAlphaPower() > 1.1)
			max = int(std::round(m_max_val * 2));
		else
			max = int(std::round(m_max_val));
		dval = vd->GetHighOffset();
		int hi = int(std::round(dval * m_max_val));
		bval = vd->GetMinMaxEnable();
		panel->UpdateMinMax(bval, low, hi, max);
	}
	if (update_minmax || update_tips)
	{
		bval = vd->GetMinMaxEnable() || mf_enable;
		panel->UpdateMinMaxTips(bval);
	}
	//threshold
	if (update_threshold)
	{
		dval = vd->GetLeftThresh();
		int max = int(std::round(m_max_val));
		int low = int(std::round(dval * m_max_val));
		dval = vd->GetRightThresh();
		int hi = int(std::round(dval * m_max_val));
		bval = vd->GetThreshEnable();
		panel->UpdateThreshold(bval, low, hi, max);
	}
	if (update_threshold || update_tips)
	{
		bval = vd->GetThreshEnable() || mf_enable;
		panel->UpdateThresholdTips(bval);
	}
	//alpha
	if (update_alpha)
	{
		dval = vd->GetAlpha();
		ival = int(std::round(dval * m_max_val));
		int max = int(std::round(m_max_val));
		bval = vd->GetAlphaEnable();
		panel->UpdateAlpha(bval, ival, max);
	}
	if (update_alpha || update_tips)
	{
		bval = vd->GetAlphaEnable() || mf_enable;
		panel->UpdateAlphaTips(bval);
	}
	//luminance
	if (update_luminance)
	{
		dval = vd->GetLuminance();
		bval = vd->GetLuminanceEnable();
		ival = int(std::round(dval * m_max_val));
		int max = int(std::round(m_max_val * 2));
		panel->UpdateLuminance(bval, ival, max);
	}
	if (update_luminance || update_tips)
	{
		bval = vd->GetLuminanceEnable() || mf_enable;
		panel->UpdateLuminanceTips(bval);
	}
	//shadings
	if (update_shading)
	{
		double strength = vd->GetShadingStrength();
		double shine = vd->GetShadingShine();
		bval = vd->GetShadingEnable();
		panel->UpdateShading(bval, strength, shine);
	}
	if (update_shading || update_tips)
	{
		bval = vd->GetShadingEnable() || mf_enable;
		panel->UpdateShadingTips(bval);
	}
	//shadow
	if (update_shadow)
	{
		bval = vd->GetShadowEnable();
		dval = vd->GetShadowIntensity();
		panel->UpdateShadow(bval, dval);
	}
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
	if (update_shadow || update_tips)
	{
		bval = vd->GetShadowEnable() || mf_enable;
		panel->UpdateShadowTips(bval);
	}
	//smaple rate
	if (update_sample)
	{
		bval = vd->GetSampleRateEnable();
		dval = vd->GetSampleRate();
		panel->UpdateSampleRate(bval, dval);
	}
	if (update_sample || update_tips)
	{
		bval = vd->GetSampleRateEnable() || mf_enable;
		panel->UpdateSampleRateTips(bval);
	}

	//spacings
	if (update_all || request.HasValue(gstSpacing))
	{
		auto spc = vd->GetBaseSpacing();
		panel->UpdateSpacing(spc);
	}

	//colormap
	if (update_colormap)
	{
		double low, high;
		vd->GetColormapValues(low, high);
		int ilow = int(std::round(low * m_max_val));
		int ihigh = int(std::round(high * m_max_val));
		int max = int(std::round(m_max_val));
		bval = vd->GetMainColorMode() == flvr::ColorMode::Colormap ||
			vd->GetMaskColorMode() == flvr::ColorMode::Colormap;
		panel->UpdateColormapValues(bval, ilow, ihigh, max);

		//text
		vd->GetColormapDispValues(low, high);
		double minv, maxv;
		vd->GetColormapRange(minv, maxv);
		bool int_validator = (maxv - minv) > 10.0;
		panel->UpdateColormapDispValues(low, high, int_validator);

		//colormap
		bval = vd->GetColormapInv() > 0.0 ? false : true;
		panel->UpdateColormapInv(bval);

		ival = vd->GetColormap();
		panel->UpdateColormapType(ival);

		flvr::ColormapProj colormap_proj = vd->GetColormapProj();
		ival = 0;
		if (flvr::ShaderParams::ValidColormapProj(colormap_proj))
			ival = static_cast<int>(colormap_proj) - 1;
		panel->UpdateColormapProj(ival);

		//show colormap on slider
		std::vector<unsigned char> colormap_data;
		if (vd->GetColormapData(colormap_data))
		{
			auto lc = vd->GetColorFromColormap(0, true);
			auto hc = vd->GetColorFromColormap(1, true);
			panel->UpdateColormapVis(colormap_data, lc, hc);
		}
	}
	if (update_colormap || update_tips)
	{
		bval = mf_enable ||
			vd->GetMainColorMode() == flvr::ColorMode::Colormap ||
			vd->GetMaskColorMode() == flvr::ColorMode::Colormap;
		panel->UpdateColormapTips(bval);
	}

	//color
	if (update_color)
	{
		auto main_color = vd->GetColor();
		auto alt_color = vd->GetMaskColor();
		panel->UpdateColor(main_color, alt_color);
	}

	//mask mode
	if (update_all || request.HasValue(gstMainMode))
	{
		auto main_mode = vd->GetMainColorMode();
		panel->UpdateMainMode(main_mode);
	}

	if (update_all || request.HasValue(gstMaskMode))
	{
		auto mask_mode = vd->GetMaskColorMode();
		panel->UpdateMaskMode(mask_mode);
	}

	//inversion
	if (update_all || request.HasValue(gstInvert))
	{
		bval = vd->GetInvert();
		panel->UpdateInvert(bval);
	}

	//MIP
	if (update_all || request.HasValue(gstRenderMode))
	{
		bval = vd->GetRenderMode() == flvr::RenderMode::Mip;
		panel->UpdateRenderMode(bval);
	}

	//transparency
	if (update_all || request.HasValue(gstTransparent))
	{
		bval = vd->GetAlphaPower() > 1.1;
		panel->UpdateTransparent(bval);
	}

	//legend
	if (update_all || request.HasValue(gstLegend))
	{
		bval = vd->GetLegend();
		panel->UpdateLegend(bval);
	}

	//outline
	if (update_all || request.HasValue(gstOutline))
	{
		bval = vd->GetOutline();
		panel->UpdateOutline(bval);
	}

	//interpolate
	if (update_all || request.HasValue(gstInterpolate))
	{
		bval = vd->GetInterpolate();
		panel->UpdateInterpolate(bval);
	}

	//sync group
	if (update_all || request.HasValue(gstSyncGroup))
	{
		auto group = m_group.lock();
		if (group)
			m_sync_group = group->GetVolumeSyncProp();
		panel->UpdateSyncGroup(m_sync_group);
	}

	//noise reduction
	if (update_all || request.HasValue(gstNoiseRedct))
	{
		bval = vd->GetNR();
		panel->UpdateNoiseRedct(bval);
	}

	//blend mode
	if (update_all || request.HasValue(gstChannelMixMode))
	{
		auto channel_mix_mode = vd->GetChannelMixMode();
		bval = channel_mix_mode == ChannelMixMode::Depth;
		panel->UpdateChannelMixMode(bval);
	}

	//std::chrono::duration<double> ts = std::chrono::duration_cast<std::chrono::duration<double>>(
	//	std::chrono::high_resolution_clock::now() - t);
	//DBGPRINT(L"update settings, time: %f\n", ts);
	//return;
}

void VolumePropPanelAgent::UpdateData(const UpdateRequest& request)
{
	if (request.HasValue(gstVolumeGradient))
		UpdateGradient();

	if (request.HasValue(gstMinMax))
		SetMinMax();
	if (request.HasValue(gstSyncMinMax))
		SyncMinMax();
	if (request.HasValue(gstSetDefaultMinMax))
		SetDefaultMinMax();
	if (request.HasValue(gstSetMlMinMax))
		SetMlMinMax();
	if (request.HasValue(gstEnableMinMax))
		EnableMinMax();
	if (request.HasValue(gstToggleMinMax))
		ToggleMinMax();

	if (request.HasValue(gstGamma3d))
		SetGamma();
	if (request.HasValue(gstSyncGamma3d))
		SyncGamma();
	if (request.HasValue(gstSetDefaultGamma3d))
		SetDefaultGamma();
	if (request.HasValue(gstSetMlGamma3d))
		SetMlGamma();
	if (request.HasValue(gstEnableGamma3d))
		EnableGamma();
	if (request.HasValue(gstToggleGamma3d))
		ToggleGamma();

	if (request.HasValue(gstAlpha))
		SetAlpha();
	if (request.HasValue(gstSyncAlpha))
		SyncAlpha();
	if (request.HasValue(gstSetDefaultAlpha))
		SetDefaultAlpha();
	if (request.HasValue(gstSetMlAlpha))
		SetMlAlpha();
	if (request.HasValue(gstEnableAlpha))
		EnableAlpha();
	if (request.HasValue(gstToggleAlpha))
		ToggleAlpha();

	if (request.HasValue(gstLuminance))
		SetLuminance();
	if (request.HasValue(gstSyncLuminance))
		SyncLuminance();
	if (request.HasValue(gstSetDefaultLuminance))
		SetDefaultLuminance();
	if (request.HasValue(gstSetMlLuminance))
		SetMlLuminance();
	if (request.HasValue(gstEnableLuminance))
		EnableLuminance();
	if (request.HasValue(gstToggleLuminance))
		ToggleLuminance();

	if (request.HasValue(gstSampleRate))
		SetSampleRate();
	if (request.HasValue(gstSyncSampleRate))
		SyncSampleRate();
	if (request.HasValue(gstSetDefaultSampleRate))
		SetDefaultSampleRate();
	if (request.HasValue(gstSetMlSampleRate))
		SetMlSampleRate();
	if (request.HasValue(gstEnableSampleRate))
		EnableSampleRate();
	if (request.HasValue(gstToggleSampleRate))
		ToggleSampleRate();

	if (request.HasValue(gstThreshold))
		SetThreshold();
	if (request.HasValue(gstSyncThreshold))
		SyncThreshold();
	if (request.HasValue(gstSetDefaultThreshold))
		SetDefaultThreshold();
	if (request.HasValue(gstSetMlThreshold))
		SetMlThreshold();
	if (request.HasValue(gstEnableThreshold))
		EnableThreshold();
	if (request.HasValue(gstToggleThreshold))
		ToggleThreshold();

	if (request.HasValue(gstBoundary))
		SetBoundary();
	if (request.HasValue(gstSyncBoundary))
		SyncBoundary();
	if (request.HasValue(gstSetDefaultBoundary))
		SetDefaultBoundary();
	if (request.HasValue(gstSetMlBoundary))
		SetMlBoundary();
	if (request.HasValue(gstEnableBoundary))
		EnableBoundary();
	if (request.HasValue(gstToggleBoundary))
		ToggleBoundary();

	if (request.HasValue(gstShadingStrength))
		SetShadingStrength();
	if (request.HasValue(gstShadingShine))
		SetShadingShine();
	if (request.HasValue(gstSyncShading))
		SyncShading();
	if (request.HasValue(gstSetDefaultShading))
		SetDefaultShading();
	if (request.HasValue(gstSetMlShading))
		SetMlShading();
	if (request.HasValue(gstEnableShading))
		EnableShading();
	if (request.HasValue(gstToggleShading))
		ToggleShading();

	if (request.HasValue(gstShadow))
		SetShadowIntensity();
	if (request.HasValue(gstSyncShadowIntensity))
		SyncShadowIntensity();
	if (request.HasValue(gstSetDefaultShadowIntensity))
		SetDefaultShadowIntensity();
	if (request.HasValue(gstSetMlShadowIntensity))
		SetMlShadowIntensity();
	if (request.HasValue(gstEnableShadow))
		EnableShadowIntensity();
	if (request.HasValue(gstToggleShadow))
		ToggleShadowIntensity();
	if (request.HasValue(gstShadowDir))
		SetShadowDirection();
	if (request.HasValue(gstEnableShadowDir))
		EnableShadowDirection();

	if (request.HasValue(gstColormap))
		SetColormap();
	if (request.HasValue(gstColormapSlider))
		SetColormapSlider();
	if (request.HasValue(gstSyncColormap))
		SyncColormap();
	if (request.HasValue(gstSetDefaultColormap))
		SetDefaultColormap();
	if (request.HasValue(gstSetMlColormap))
		SetMlColormap();
	if (request.HasValue(gstEnableColormap))
		EnableColormap();
	if (request.HasValue(gstToggleColormap))
		ToggleColormap();

	if (request.HasValue(gstInvertColormap))
		InvertColormap();
	if (request.HasValue(gstColormapType))
		SetColormapType();
	if (request.HasValue(gstColormapProj))
		SetColormapProj();

	if (request.HasValue(gstMainColorMode))
		SetMainColorMode();
	if (request.HasValue(gstMainColor))
		SetMainColor();
	if (request.HasValue(gstAltColorMode))
		SetAltColorMode();
	if (request.HasValue(gstAltColor))
		SetAltColor();

	if (request.HasValue(gstSetMlVolumeProp))
		SetMachineLearning();
	if (request.HasValue(gstTransparent))
		SetTransparent();
	if (request.HasValue(gstRenderMode))
		SetMIP();
	if (request.HasValue(gstInvert))
		SetInvert();
	if (request.HasValue(gstOutline))
		SetOutline();
	if (request.HasValue(gstInterpolate))
		SetInterpolate();
	if (request.HasValue(gstNoiseRedct))
		SetNoiseReduction();
	if (request.HasValue(gstSyncGroup))
		SetSyncGroup();
	if (request.HasValue(gstChannelMixMode))
		SetChannelMixDepth();
	if (request.HasValue(gstLegend))
		SetLegend();
	if (request.HasValue(gstResetDefault))
		ResetDefault();
	if (request.HasValue(gstSaveDefault))
		SaveDefault();
	if (request.HasValue(gstSpacing))
		SetSpacing();
}

void VolumePropPanelAgent::UpdateGradient()
{
	auto vd = GetData();
	if (!vd)
		return;

	auto proj = vd->GetColormapProj();
	if (proj == flvr::ColormapProj::Radial ||
		proj == flvr::ColormapProj::Linear)
	{
		vd->UpdateGradient();
	}
}

void VolumePropPanelAgent::SetMinMax()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	auto [min, max] = panel->GetMinMax();
	if (double(max) > m_max_val)
		UpdateMaxVal(max);
	double val1 = min / m_max_val;
	double val2 = max / m_max_val;
	//set gamma value
	if (m_sync_group)
		SyncMinMax(val1, val2);
	else
		SetMinMax(val1, val2);
}

void VolumePropPanelAgent::SyncMinMax()
{
	auto vd = m_vd.lock();
	if (vd)
		SyncMinMax(vd->GetLowOffset(), vd->GetHighOffset());
}

void VolumePropPanelAgent::SetDefaultMinMax()
{
	double val1 = glbin_vol_def.m_lo_offset;
	double val2 = glbin_vol_def.m_hi_offset;
	if (m_sync_group)
		SyncMinMax(val1, val2);
	else
		SetMinMax(val1, val2);
}

void VolumePropPanelAgent::SetMlMinMax()
{
	auto vd = m_vd.lock();
	if (!vd)
		return;
	double val1 = vd->GetMlLowOffset();
	double val2 = vd->GetMlHighOffset();
	if (m_sync_group)
		SyncMinMax(val1, val2);
	else
		SetMinMax(val1, val2);
}

void VolumePropPanelAgent::EnableMinMax()
{
	auto panel = GetPanel();
	if (!panel)
		return;
	bool bval = panel->GetEnableMinMax();
	EnableMinMax(bval);
}

void VolumePropPanelAgent::ToggleMinMax()
{
	auto vd = m_vd.lock();
	if (!vd)
		return;
	bool bval = !vd->GetMinMaxEnable();
	EnableMinMax(bval);
}

void VolumePropPanelAgent::SetGamma()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	double val = panel->GetGamma();
	//set gamma value
	if (m_sync_group)
		SyncGamma(val);
	else
		SetGamma(val);
}

void VolumePropPanelAgent::SyncGamma()
{
	auto vd = m_vd.lock();
	if (vd)
		SyncGamma(vd->GetGamma());
}

void VolumePropPanelAgent::SetDefaultGamma()
{
	double val = glbin_vol_def.m_gamma;
	if (m_sync_group)
		SyncGamma(val);
	else
		SetGamma(val);
}

void VolumePropPanelAgent::SetMlGamma()
{
	auto vd = m_vd.lock();
	if (!vd)
		return;
	double val = vd->GetMlGamma();
	if (m_sync_group)
		SyncGamma(val);
	else
		SetGamma(val);
}

void VolumePropPanelAgent::EnableGamma()
{
	auto panel = GetPanel();
	if (!panel)
		return;
	bool bval = panel->GetEnableGamma();
	EnableGamma(bval);
}

void VolumePropPanelAgent::ToggleGamma()
{
	auto vd = m_vd.lock();
	if (!vd)
		return;
	bool bval = !vd->GetGammaEnable();
	EnableGamma(bval);
}

void VolumePropPanelAgent::SetAlpha()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	int ival = panel->GetAlpha();
	if (double(ival) > m_max_val)
		UpdateMaxVal(ival);
	double val = ival / m_max_val;

	if (m_sync_group)
		SyncAlpha(val);
	else
		SetAlpha(val);
}

void VolumePropPanelAgent::SyncAlpha()
{
	auto vd = m_vd.lock();
	if (vd)
		SyncAlpha(vd->GetAlpha());
}

void VolumePropPanelAgent::SetDefaultAlpha()
{
	double val = glbin_vol_def.m_alpha;
	if (m_sync_group)
		SyncAlpha(val);
	else
		SetAlpha(val);
}

void VolumePropPanelAgent::SetMlAlpha()
{
	auto vd = m_vd.lock();
	if (!vd)
		return;
	double val = vd->GetMlAlpha();
	if (m_sync_group)
		SyncAlpha(val);
	else
		SetAlpha(val);
}

void VolumePropPanelAgent::EnableAlpha()
{
	auto panel = GetPanel();
	if (!panel)
		return;
	bool bval = panel->GetEnableAlpha();
	EnableAlpha(bval);
}

void VolumePropPanelAgent::ToggleAlpha()
{
	auto vd = m_vd.lock();
	if (!vd)
		return;
	bool bval = !vd->GetAlphaEnable();
	EnableAlpha(bval);
}

void VolumePropPanelAgent::SetLuminance()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	int ival = panel->GetLuminance();
	if (double(ival) > m_max_val)
		UpdateMaxVal(ival);
	double val = ival / m_max_val;

	if (m_sync_group)
		SyncLuminance(val);
	else
		SetLuminance(val);
}

void VolumePropPanelAgent::SyncLuminance()
{
	auto vd = m_vd.lock();
	if (vd)
		SyncLuminance(vd->GetLuminance());
}

void VolumePropPanelAgent::SetDefaultLuminance()
{
	double val = glbin_vol_def.m_luminance;
	if (m_sync_group)
		SyncLuminance(val);
	else
		SetLuminance(val);
}

void VolumePropPanelAgent::SetMlLuminance()
{
	auto vd = m_vd.lock();
	if (!vd)
		return;
	double val = vd->GetMlLuminance();
	if (m_sync_group)
		SyncLuminance(val);
	else
		SetLuminance(val);
}

void VolumePropPanelAgent::EnableLuminance()
{
	auto panel = GetPanel();
	if (!panel)
		return;
	bool bval = panel->GetEnableLuminance();
	EnableLuminance(bval);
}

void VolumePropPanelAgent::ToggleLuminance()
{
	auto vd = m_vd.lock();
	if (!vd)
		return;
	bool bval = !vd->GetLuminanceEnable();
	EnableLuminance(bval);
}

void VolumePropPanelAgent::SetSampleRate()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	double dval = panel->GetSampleRate();

	if (m_sync_group)
		SyncSampleRate(dval);
	else
		SetSampleRate(dval);
}

void VolumePropPanelAgent::SyncSampleRate()
{
	auto vd = m_vd.lock();
	if (vd)
		SyncSampleRate(vd->GetSampleRate());
}

void VolumePropPanelAgent::SetDefaultSampleRate()
{
	double val = glbin_vol_def.m_sample_rate;
	if (m_sync_group)
		SyncSampleRate(val);
	else
		SetSampleRate(val);
}

void VolumePropPanelAgent::SetMlSampleRate()
{
	auto vd = m_vd.lock();
	if (!vd)
		return;
	double val = vd->GetMlSampleRate();
	if (m_sync_group)
		SyncSampleRate(val);
	else
		SetSampleRate(val);
}

void VolumePropPanelAgent::EnableSampleRate()
{
	auto panel = GetPanel();
	if (!panel)
		return;
	bool bval = panel->GetEnableSampleRate();
	EnableSampleRate(bval);
}

void VolumePropPanelAgent::ToggleSampleRate()
{
	auto vd = m_vd.lock();
	if (!vd)
		return;
	bool bval = !vd->GetSampleRateEnable();
	EnableSampleRate(bval);
}

void VolumePropPanelAgent::SetThreshold()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	auto [min, max] = panel->GetThreshold();
	if (double(max) > m_max_val)
		UpdateMaxVal(max);
	double val1 = min / m_max_val;
	double val2 = max / m_max_val;
	//set gamma value
	if (m_sync_group)
		SyncThreshold(val1, val2);
	else
		SetThreshold(val1, val2);
}

void VolumePropPanelAgent::SyncThreshold()
{
	auto vd = m_vd.lock();
	if (vd)
		SyncThreshold(vd->GetLeftThresh(), vd->GetRightThresh());
}

void VolumePropPanelAgent::SetDefaultThreshold()
{
	double val1 = glbin_vol_def.m_lo_thresh;
	double val2 = glbin_vol_def.m_hi_thresh;
	if (m_sync_group)
		SyncThreshold(val1, val2);
	else
		SetThreshold(val1, val2);
}

void VolumePropPanelAgent::SetMlThreshold()
{
	auto vd = m_vd.lock();
	if (!vd)
		return;
	double val1 = vd->GetMlLeftThresh();
	double val2 = vd->GetMlRightThresh();
	if (m_sync_group)
		SyncThreshold(val1, val2);
	else
		SetThreshold(val1, val2);
}

void VolumePropPanelAgent::EnableThreshold()
{
	auto panel = GetPanel();
	if (!panel)
		return;
	bool bval = panel->GetEnableThreshold();
	EnableThreshold(bval);
}

void VolumePropPanelAgent::ToggleThreshold()
{
	auto vd = m_vd.lock();
	if (!vd)
		return;
	bool bval = !vd->GetThreshEnable();
	EnableThreshold(bval);
}

void VolumePropPanelAgent::SetBoundary()
{
	auto panel = GetPanel();
	if (!panel)
		return;
	auto vd = m_vd.lock();
	if (!vd)
		return;

	auto [val1, val2] = panel->GetBoundary();
	double gmf = 1000 / vd->GetBoundaryMax();
	int low = std::round(val1 * gmf);
	int high = std::round(val2 * gmf);
	int low_save = low;
	int high_save = high;
	panel->TrySetBoundarySlider(low, high);
	if (low != low_save)
	{
		val1 = low / gmf;
	}
	if (high != high_save)
	{
		val2 = high / gmf;
	}
	//set gamma value
	if (m_sync_group)
		SyncBoundary(val1, val2);
	else
		SetBoundary(val1, val2);
}

void VolumePropPanelAgent::SyncBoundary()
{
	auto vd = m_vd.lock();
	if (vd)
		SyncBoundary(vd->GetBoundaryLow(), vd->GetBoundaryHigh());
}

void VolumePropPanelAgent::SetDefaultBoundary()
{
	double val1 = glbin_vol_def.m_boundary_low;
	double val2 = glbin_vol_def.m_boundary_high;
	if (m_sync_group)
		SyncBoundary(val1, val2);
	else
		SetBoundary(val1, val2);
}

void VolumePropPanelAgent::SetMlBoundary()
{
	auto vd = m_vd.lock();
	if (!vd)
		return;
	double val1 = vd->GetMlBoundaryLow();
	double val2 = vd->GetMlBoundaryHigh();
	if (m_sync_group)
		SyncBoundary(val1, val2);
	else
		SetBoundary(val1, val2);
}

void VolumePropPanelAgent::EnableBoundary()
{
	auto panel = GetPanel();
	if (!panel)
		return;
	bool bval = panel->GetEnableBoundary();
	EnableBoundary(bval);
}

void VolumePropPanelAgent::ToggleBoundary()
{
	auto vd = m_vd.lock();
	if (!vd)
		return;
	bool bval = !vd->GetThreshEnable();
	EnableBoundary(bval);
}

void VolumePropPanelAgent::SetShadingStrength()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	double val = panel->GetShadingStrength();
	if (m_sync_group)
		SyncShadingStrength(val);
	else
		SetShadingStrength(val);
}

void VolumePropPanelAgent::SetShadingShine()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	double val = panel->GetShadingShine();
	if (m_sync_group)
		SyncShadingShine(val);
	else
		SetShadingShine(val);
}

void VolumePropPanelAgent::SyncShading()
{
	auto vd = m_vd.lock();
	if (vd)
	{
		SyncShadingStrength(vd->GetShadingStrength());
		SyncShadingShine(vd->GetShadingShine());
	}
}

void VolumePropPanelAgent::SetDefaultShading()
{
	double strength = glbin_vol_def.m_shading_strength;
	double shine = glbin_vol_def.m_shading_shine;
	if (m_sync_group)
	{
		SyncShadingStrength(strength);
		SyncShadingShine(shine);
	}
	else
	{
		SetShadingStrength(strength);
		SetShadingShine(shine);
	}
}

void VolumePropPanelAgent::SetMlShading()
{
	auto vd = m_vd.lock();
	if (!vd)
		return;

	double strength = vd->GetMlShadingStrength();
	double shine = vd->GetMlShadingShine();
	if (m_sync_group)
	{
		SyncShadingStrength(strength);
		SyncShadingShine(shine);
	}
	else
	{
		SetShadingStrength(strength);
		SetShadingShine(shine);
	}
}

void VolumePropPanelAgent::EnableShading()
{
	auto panel = GetPanel();
	if (!panel)
		return;
	bool bval = panel->GetShadingEnable();
	EnableShading(bval);
}

void VolumePropPanelAgent::ToggleShading()
{
	auto vd = m_vd.lock();
	if (!vd)
		return;
	bool bval = !vd->GetShadingEnable();
	EnableShading(bval);
}

void VolumePropPanelAgent::SetShadowIntensity()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	double val = panel->GetShadowIntensity();
	if (m_sync_group)
		SyncShadowInt(val);
	else
		SetShadowInt(val);
}

void VolumePropPanelAgent::SyncShadowIntensity()
{
	auto vd = m_vd.lock();
	if (vd)
		SyncShadowInt(vd->GetShadowIntensity());
}

void VolumePropPanelAgent::SetDefaultShadowIntensity()
{
	double val = glbin_vol_def.m_shadow_intensity;
	if (m_sync_group)
		SyncShadowInt(val);
	else
		SetShadowInt(val);
}

void VolumePropPanelAgent::SetMlShadowIntensity()
{
	auto vd = m_vd.lock();
	if (!vd)
		return;

	double val = vd->GetMlShadowIntensity();
	if (m_sync_group)
		SyncShadowInt(val);
	else
		SetShadowInt(val);
}

void VolumePropPanelAgent::EnableShadowIntensity()
{
	auto panel = GetPanel();
	if (!panel)
		return;
	bool bval = panel->GetShadowEnable();
	EnableShadow(bval);
}

void VolumePropPanelAgent::ToggleShadowIntensity()
{
	auto vd = m_vd.lock();
	if (!vd)
		return;
	bool bval = !vd->GetShadowEnable();
	EnableShadow(bval);
}

void VolumePropPanelAgent::SetShadowDirection()
{
	auto panel = GetPanel();
	if (!panel)
		return;
	double val = panel->GetShadowDirection();
	SetShadowDir(val);
}

void VolumePropPanelAgent::EnableShadowDirection()
{
	auto panel = GetPanel();
	if (!panel)
		return;
	bool bval = panel->GetShadowDirEnable();
	EnableShadowDir(bval);
}

void VolumePropPanelAgent::SetColormap()
{
	auto panel = GetPanel();
	if (!panel)
		return;
	auto vd = m_vd.lock();
	if (!vd)
		return;

	auto [min, max] = panel->GetColormap();
	double dval1 = 0, dval2 = 0;
	double minv, maxv;
	vd->GetColormapRange(minv, maxv);
	dval1 = (min - minv) / (maxv - minv);
	dval2 = (max - minv) / (maxv - minv);

	if (m_sync_group)
		SyncColormapVal(dval1, dval2);
	else
		SetColormapVal(dval1, dval2);
}

void VolumePropPanelAgent::SetColormapSlider()
{
	auto panel = GetPanel();
	if (!panel)
		return;
	auto vd = m_vd.lock();
	if (!vd)
		return;

	auto [min, max] = panel->GetColormapSlider();
	double val1 = double(min) / m_max_val;
	double val2 = double(max) / m_max_val;
	double minv, maxv;
	vd->GetColormapRange(minv, maxv);
	bool int_validator = (maxv - minv) > 10.0;
	panel->UpdateColormapDispValues(val1, val2, int_validator);

	SetColormap();
}

void VolumePropPanelAgent::SyncColormap()
{
	auto vd = m_vd.lock();
	if (vd)
	{
		double val1, val2;
		vd->GetColormapValues(val1, val2);
		SyncColormapVal(val1, val2);
	}
}

void VolumePropPanelAgent::SetDefaultColormap()
{
	double val1 = glbin_vol_def.m_colormap_low_value;
	double val2 = glbin_vol_def.m_colormap_hi_value;

	if (m_sync_group)
		SyncColormapVal(val1, val2);
	else
		SetColormapVal(val1, val2);
}

void VolumePropPanelAgent::SetMlColormap()
{
	auto vd = m_vd.lock();
	if (!vd)
		return;

	double val1 = vd->GetMlColormapLow();
	double val2 = vd->GetMlColormapHigh();

	if (m_sync_group)
		SyncColormapVal(val1, val2);
	else
		SetColormapVal(val1, val2);
}

void VolumePropPanelAgent::EnableColormap()
{
	auto panel = GetPanel();
	if (!panel)
		return;
	bool bval = panel->GetColormapEnable();
	EnableColormap(bval);
}

void VolumePropPanelAgent::ToggleColormap()
{
	auto vd = m_vd.lock();
	if (!vd)
		return;

	EnableColormap(
		vd->GetMainColorMode() ==
		flvr::ColorMode::SingleColor);
}

void VolumePropPanelAgent::InvertColormap()
{
	auto panel = GetPanel();
	if (!panel)
		return;
	bool bval = panel->GetColormapInv();

	auto view = m_view.lock();
	auto group = m_group.lock();
	auto vd = m_vd.lock();
	assert(view);
	assert(group);
	assert(vd);

	bool mip_enable = vd && vd->GetRenderMode() == flvr::RenderMode::Mip;
	bool sync_view_depth_mip = mip_enable && view && view->GetChannelMixMode() == ChannelMixMode::Depth;
	bool sync_group_depth_mip = mip_enable && vd && vd->GetChannelMixMode() == ChannelMixMode::Depth;

	if (sync_view_depth_mip)
		view->SetColormapInv(bval ? -1.0 : 1.0);
	else if (m_sync_group || sync_group_depth_mip)
		group->SetColormapInv(bval ? -1.0 : 1.0);
	else
		vd->SetColormapInv(bval ? -1.0 : 1.0);

	EnableColormap(true);
}

void VolumePropPanelAgent::SetColormapType()
{
	auto panel = GetPanel();
	if (!panel)
		return;
	int colormap = panel->GetColormapType();

	auto view = m_view.lock();
	auto group = m_group.lock();
	auto vd = m_vd.lock();
	assert(view);
	assert(group);
	assert(vd);
	bool mip_enable = vd->GetRenderMode() == flvr::RenderMode::Mip;
	bool sync_view_depth_mip = mip_enable && view->GetChannelMixMode() == ChannelMixMode::Depth;
	bool sync_group_depth_mip = mip_enable && vd->GetChannelMixMode() == ChannelMixMode::Depth;

	if (sync_view_depth_mip)
		view->SetColormap(colormap);
	else if (m_sync_group || sync_group_depth_mip)
		group->SetColormap(colormap);
	else
		vd->SetColormap(colormap);

	EnableColormap(true);
	if (colormap >= 5)
	{
		EnableTransparent(true);
	}
}

void VolumePropPanelAgent::SetColormapProj()
{
	auto panel = GetPanel();
	if (!panel)
		return;
	int ival = panel->GetColormapProj();

	flvr::ColormapProj colormap_proj =
		static_cast<flvr::ColormapProj>(ival);

	auto group = m_group.lock();
	auto vd = m_vd.lock();
	if (m_sync_group && group)
		group->SetColormapProj(colormap_proj);
	else if (vd)
		vd->SetColormapProj(colormap_proj);
	EnableColormap(true);
}

void VolumePropPanelAgent::SetMainColorMode()
{
	auto group = m_group.lock();
	auto vd = m_vd.lock();
	if (!vd)
		return;

	if (m_sync_group && group)
		group->IncMainMaskMode();
	else
		vd->IncMainMaskMode();

	NotifyViewUpdate({ gstMainMode, gstColormap });
}

void VolumePropPanelAgent::SetMainColor()
{
	auto panel = GetPanel();
	if (!panel)
		return;
	auto vd = m_vd.lock();
	if (!vd)
		return;

	fluo::Color color = panel->GetMainColor();

	vd->SetColor(color);
	NotifyViewUpdate({ gstColor, gstSecColor, gstTreeColors, gstClipPlaneRangeColor, gstUpdateSync, gstColormap, gstUpdateHistogram });
}

void VolumePropPanelAgent::SetAltColorMode()
{
	auto vd = m_vd.lock();
	if (!vd)
		return;

	auto group = m_group.lock();
	if (m_sync_group && group)
		group->IncMaskMode();
	else
		vd->IncMaskMode();

	NotifyViewUpdate({ gstMaskMode, gstColormap });
}

void VolumePropPanelAgent::SetAltColor()
{
	auto panel = GetPanel();
	if (!panel)
		return;
	auto vd = m_vd.lock();
	if (!vd)
		return;

	fluo::Color color = panel->GetAltColor();
	vd->SetMaskColor(color);
	NotifyViewUpdate({ gstSecColor, gstColormap });
}

void VolumePropPanelAgent::ApplyMl()
{
	auto group = m_group.lock();
	auto vd = m_vd.lock();
	if (m_sync_group && group)
		group->ApplyMlVolProp();
	else if (vd)
		vd->ApplyMlVolProp();
	NotifyViewUpdate({ gstVolumeProps, gstConvVolMeshUpdateTransf });
}

void VolumePropPanelAgent::SaveMl()
{
	auto vd = m_vd.lock();
	if (!vd)
		return;

	std::vector<float> val;
	val.push_back(float(vd->GetBoundaryLow()));
	val.push_back(float(vd->GetGamma()));
	val.push_back(float(vd->GetLowOffset()));
	val.push_back(float(vd->GetHighOffset()));
	val.push_back(float(vd->GetLeftThresh()));
	val.push_back(float(vd->GetRightThresh()));
	val.push_back(float(vd->GetShadingStrength()));
	val.push_back(float(vd->GetShadingShine()));
	val.push_back(float(vd->GetAlpha()));
	val.push_back(float(vd->GetSampleRate()));
	val.push_back(float(vd->GetLuminance()));
	val.push_back(float(vd->GetMainColorMode() == flvr::ColorMode::Colormap));
	val.push_back(float(vd->GetColormapInv()));
	val.push_back(float(vd->GetColormap()));
	val.push_back(float(vd->GetColormapProj()));
	val.push_back(float(vd->GetColormapLow()));
	val.push_back(float(vd->GetColormapHigh()));
	val.push_back(float(vd->GetAlphaEnable()));
	val.push_back(float(vd->GetShadingEnable()));
	val.push_back(float(vd->GetInterpolate()));
	val.push_back(float(vd->GetInvert()));
	val.push_back(float(vd->GetRenderMode() == flvr::RenderMode::Mip));
	val.push_back(float(vd->GetTransparent()));
	val.push_back(float(vd->GetNR()));
	val.push_back(float(vd->GetShadowEnable()));
	val.push_back(float(vd->GetShadowIntensity()));
	auto ep = flrd::Reshape::get_entry_params("vol_prop", val);

	//histogram
	flrd::Histogram histogram(vd);
	histogram.SetProgressFunc(glbin_data_manager.GetProgressFunc());
	histogram.SetUseMask(false);
	auto eh = histogram.GetEntryHist();

	if (eh)
	{
		//record
		auto rec = std::make_shared<flrd::RecordHistParams>();
		rec->setInput(eh);
		rec->setOutput(ep);
		//table
		glbin.get_vp_table().addRecord(rec);
	}
}

//enable/disable
void VolumePropPanelAgent::EnableGamma(bool bval)
{
	auto group = m_group.lock();
	auto vd = m_vd.lock();
	if (m_sync_group && group)
		group->SetGammaEnable(bval);
	else if (vd)
		vd->SetGammaEnable(bval);

	FluoRefresh(0, { gstGamma3d }, { glbin_current.GetViewId() });
}

void VolumePropPanelAgent::EnableMinMax(bool bval)
{
	auto group = m_group.lock();
	auto vd = m_vd.lock();
	if (m_sync_group && group)
		group->SetMinMaxEnable(bval);
	else if (vd)
		vd->SetMinMaxEnable(bval);

	FluoRefresh(0, { gstMinMax }, { glbin_current.GetViewId() });
}

void VolumePropPanelAgent::EnableLuminance(bool bval)
{
	auto group = m_group.lock();
	auto vd = m_vd.lock();
	if (m_sync_group && group)
		group->SetLuminanceEnable(bval);
	else if (vd)
		vd->SetLuminanceEnable(bval);

	FluoRefresh(0, { gstColor }, { glbin_current.GetViewId() });
}

void VolumePropPanelAgent::EnableAlpha(bool bval)
{
	auto group = m_group.lock();
	auto vd = m_vd.lock();
	if (m_sync_group && group)
		group->SetAlphaEnable(bval);
	else if (vd)
		vd->SetAlphaEnable(bval);

	FluoRefresh(0, { gstAlpha }, { glbin_current.GetViewId() });
}

void VolumePropPanelAgent::EnableShading(bool bval)
{
	auto group = m_group.lock();
	auto vd = m_vd.lock();
	if (m_sync_group && group)
		group->SetShadingEnable(bval);
	else if (vd)
		vd->SetShadingEnable(bval);

	FluoRefresh(0, { gstShading }, { glbin_current.GetViewId() });
}

void VolumePropPanelAgent::EnableBoundary(bool bval)
{
	auto group = m_group.lock();
	auto vd = m_vd.lock();
	if (m_sync_group && group)
		group->SetBoundaryEnable(bval);
	else if (vd)
		vd->SetBoundaryEnable(bval);

	FluoRefresh(0, { gstBoundary }, { glbin_current.GetViewId() });
}

void VolumePropPanelAgent::EnableThresh(bool bval)
{
	auto group = m_group.lock();
	auto vd = m_vd.lock();
	if (m_sync_group && group)
		group->SetThreshEnable(bval);
	else if (vd)
		vd->SetThreshEnable(bval);

	FluoRefresh(0, { gstThreshold }, { glbin_current.GetViewId() });
}

void VolumePropPanelAgent::EnableShadow(bool bval)
{
	auto group = m_group.lock();
	auto vd = m_vd.lock();
	if (m_sync_group && group)
		group->SetShadowEnable(bval);
	else if (group && group->GetChannelMixMode() == ChannelMixMode::Depth)
		group->SetShadowEnable(bval);
	else if (vd)
		vd->SetShadowEnable(bval);

	FluoRefresh(0, { gstShadow }, { glbin_current.GetViewId() });
}

void VolumePropPanelAgent::EnableShadowDir(bool bval)
{
	if (glbin_settings.m_shadow_dir == bval)
		return;

	glbin_settings.m_shadow_dir = bval;
	if (bval)
	{
		wxString str;
		str = m_shadow_dir_text->GetValue();
		double deg;
		str.ToDouble(&deg);
		deg -= 45.0;
		glbin_settings.m_shadow_dir_x = cos(d2r(deg));
		glbin_settings.m_shadow_dir_y = sin(d2r(deg));
	}
	else
	{
		glbin_settings.m_shadow_dir_x = 0.0;
		glbin_settings.m_shadow_dir_y = 0.0;
	}
	FluoRefresh(0, { gstShadowDir }, { glbin_current.GetViewId() });
}

void VolumePropPanelAgent::EnableSampleRate(bool bval)
{
	auto group = m_group.lock();
	auto vd = m_vd.lock();
	if (m_sync_group && group)
		group->SetSampleRateEnable(bval);
	else if (vd)
		vd->SetSampleRateEnable(bval);

	FluoRefresh(0, { gstSampleRate }, { glbin_current.GetViewId() });
}

void VolumePropPanelAgent::EnableColormap(bool bval)
{
	auto view = m_view.lock();
	auto group = m_group.lock();
	auto vd = m_vd.lock();
	bool mip_enable = vd->GetRenderMode() == flvr::RenderMode::Mip;
	bool sync_view_depth_mip = mip_enable && view->GetChannelMixMode() == ChannelMixMode::Depth;
	bool sync_group_depth_mip = mip_enable && vd->GetChannelMixMode() == ChannelMixMode::Depth;
	if (sync_view_depth_mip)
	{
		view->SetMainMaskMode(bval ? flvr::ColorMode::Colormap : flvr::ColorMode::SingleColor);
		view->SetColormapDisp(bval);
	}
	else if (m_sync_group || sync_group_depth_mip)
	{
		group->SetMainMaskMode(bval ? flvr::ColorMode::Colormap : flvr::ColorMode::SingleColor);
		group->SetColormapDisp(bval);
	}
	else
	{
		vd->SetMainMaskMode(bval ? flvr::ColorMode::Colormap : flvr::ColorMode::SingleColor);
		vd->SetColormapDisp(bval);
	}

	FluoRefresh(0, { gstColormap, gstMainMode, gstMaskMode, gstUpdateSync }, { glbin_current.GetViewId() });
}

void VolumePropPanelAgent::EnableMip(bool bval)
{
	auto view = m_view.lock();
	auto group = m_group.lock();
	auto vd = m_vd.lock();
	bool depth_view = view->GetChannelMixMode() == ChannelMixMode::Depth;
	bool depth_group = vd->GetChannelMixMode() == ChannelMixMode::Depth;
	if (depth_view)
		view->SetRenderMode(bval ? flvr::RenderMode::Mip : flvr::RenderMode::Standard);
	else if (m_sync_group || depth_group)
		group->SetRenderMode(bval ? flvr::RenderMode::Mip : flvr::RenderMode::Standard);
	else
		vd->SetRenderMode(bval ? flvr::RenderMode::Mip : flvr::RenderMode::Standard);

	FluoRefresh(0, { gstRenderMode }, { glbin_current.GetViewId() });
}

void VolumePropPanelAgent::EnableTransparent(bool bval)
{
	auto group = m_group.lock();
	auto vd = m_vd.lock();
	if (m_sync_group && group)
		group->SetTransparent(bval);
	else if (vd)
		vd->SetTransparent(bval);

	FluoRefresh(0, { gstTransparent }, { glbin_current.GetViewId() });
}

//set values
void VolumePropPanelAgent::SetGamma(double val)
{
	auto vd = m_vd.lock();
	if (!vd)
		return;
	if (vd->GetGamma() == val)
		return;

	vd->SetGamma(val);
	NotifyViewUpdate({ gstGamma3d, gstConvVolMeshUpdateTransf });
}

void VolumePropPanelAgent::SetMinMax(double val1, double val2)
{
	auto vd = m_vd.lock();
	if (!vd)
		return;
	if (vd->GetLowOffset() == val1 &&
		vd->GetHighOffset() == val2)
		return;

	vd->SetLowOffset(val1);
	vd->SetHighOffset(val2);
	if (notify)
		FluoRefresh(0, { gstMinMax, gstConvVolMeshUpdateTransf }, { glbin_current.GetViewId() });
	else
		FluoRefresh(0, { gstConvVolMeshUpdateTransf }, { glbin_current.GetViewId() });
}

void VolumePropPanelAgent::SetLuminance(double val, bool notify)
{
	auto vd = m_vd.lock();
	if (!vd)
		return;
	if (vd->GetLuminance() == val)
		return;

	vd->SetLuminance(val);

	if (notify)
		FluoRefresh(0, { gstLuminance }, { glbin_current.GetViewId() });
	else
		FluoRefresh(0, { gstNull }, { glbin_current.GetViewId() });
}

void VolumePropPanelAgent::SetAlpha(double val, bool notify)
{
	auto vd = m_vd.lock();
	if (!vd)
		return;
	if (vd->GetAlpha() == val)
		return;

	vd->SetAlpha(val);
	if (notify)
		FluoRefresh(0, { gstAlpha }, { glbin_current.GetViewId() });
	else
		FluoRefresh(0, { gstNull }, { glbin_current.GetViewId() });
}

void VolumePropPanelAgent::SetShadingStrength(double val, bool notify)
{
	auto vd = m_vd.lock();
	if (!vd)
		return;
	if (vd->GetShadingStrength() == val)
		return;

	vd->SetShadingStrength(val);
	if (notify)
		FluoRefresh(0, { gstShading }, { glbin_current.GetViewId() });
	else
		FluoRefresh(0, { gstNull }, { glbin_current.GetViewId() });
}

void VolumePropPanelAgent::SetShadingShine(double val, bool notify)
{
	auto vd = m_vd.lock();
	if (!vd)
		return;
	if (vd->GetShadingShine() == val)
		return;

	vd->SetShadingShine(val);
	if (notify)
		FluoRefresh(0, { gstShading }, { glbin_current.GetViewId() });
	else
		FluoRefresh(0, { gstNull }, { glbin_current.GetViewId() });
}

void VolumePropPanelAgent::SetBoundary(double val1, double val2, bool notify)
{
	auto vd = m_vd.lock();
	if (!vd)
		return;
	if (vd->GetBoundaryLow() == val1 &&
		vd->GetBoundaryHigh() == val2)
		return;

	vd->SetBoundaryLow(val1);
	vd->SetBoundaryHigh(val2);
	if (notify)
		FluoRefresh(0, { gstBoundary, gstConvVolMeshUpdateTransf }, { glbin_current.GetViewId() });
	else
		FluoRefresh(0, { gstConvVolMeshUpdateTransf }, { glbin_current.GetViewId() });
}

void VolumePropPanelAgent::SetThresh(double val1, double val2, bool notify)
{
	auto vd = m_vd.lock();
	if (!vd)
		return;
	if (vd->GetLeftThresh() == val1 &&
		vd->GetRightThresh() == val2)
		return;

	vd->SetLeftThresh(val1);
	vd->SetRightThresh(val2);
	fluo::ValueCollection vc = { gstBrushCountAutoUpdate, gstColocalAutoUpdate, gstConvVolMeshUpdateTransf };
	if (notify)
		vc.insert(gstThreshold);

	FluoRefresh(0, vc, { glbin_current.GetViewId() });
}

void VolumePropPanelAgent::SetShadowInt(double val, bool notify)
{
	auto vd = m_vd.lock();
	if (!vd)
		return;
	if (vd->GetShadowIntensity() == val)
		return;

	vd->SetShadowIntensity(val);
	if (notify)
		FluoRefresh(0, { gstShadow }, { glbin_current.GetViewId() });
	else
		FluoRefresh(0, { gstNull }, { glbin_current.GetViewId() });
}

void VolumePropPanelAgent::SetShadowDir(double dval, bool notify)
{
	dval -= 45.0;
	double dir_x = cos(d2r(dval));
	double dir_y = sin(d2r(dval));
	if (glbin_settings.m_shadow_dir_x == dir_x &&
		glbin_settings.m_shadow_dir_y == dir_y)
		return;

	glbin_settings.m_shadow_dir_x = dir_x;
	glbin_settings.m_shadow_dir_y = dir_y;
	if (notify)
		FluoRefresh(0, { gstShadowDir }, { glbin_current.GetViewId() });
	else
		FluoRefresh(0, { gstNull }, { glbin_current.GetViewId() });
}

void VolumePropPanelAgent::SetSampleRate(double val, bool notify)
{
	auto vd = m_vd.lock();
	if (!vd)
		return;
	if (vd->GetSampleRate() == val)
		return;

	vd->SetSampleRate(val);
	if (notify)
		FluoRefresh(0, { gstSampleRate }, { glbin_current.GetViewId() });
	else
		FluoRefresh(0, { gstNull }, { glbin_current.GetViewId() });
}

void VolumePropPanelAgent::SetColormapVal(double val1, double val2, bool notify)
{
	auto vd = m_vd.lock();
	if (!vd)
		return;
	if (vd->GetColormapLow() == val1 &&
		vd->GetColormapHigh() == val2)
		return;

	vd->SetColormapValues(val1, val2);
	if (notify)
		FluoRefresh(0, { gstColormap }, { glbin_current.GetViewId() });
	else
		FluoRefresh(0, { gstNull }, { glbin_current.GetViewId() });
}


//sync values
void VolumePropPanelAgent::SyncGamma(double val)
{
	auto group = m_group.lock();
	if (!group)
		return;
	if (group->GetGamma() == val)
		return;

	group->SetGamma(val);
	NotifyViewUpdate({ gstGamma3d, gstConvVolMeshUpdateTransf });
}

void VolumePropPanelAgent::SyncMinMax(double val1, double val2)
{
	auto group = m_group.lock();
	if (!group)
		return;
	if (group->GetLowOffset() == val1 &&
		group->GetHighOffset() == val2)
		return;

	group->SetLowOffset(val1);
	group->SetHighOffset(val2);
	FluoRefresh(1, { gstMinMax }, { glbin_current.GetViewId() });
}

void VolumePropPanelAgent::SyncLuminance(double val)
{
	auto group = m_group.lock();
	if (!group)
		return;
	if (group->GetLuminance() == val)
		return;

	group->SetLuminance(val);
	FluoRefresh(1, { gstColor, gstTreeColors, gstClipPlaneRangeColor }, { glbin_current.GetViewId() });
}

void VolumePropPanelAgent::SyncAlpha(double val)
{
	auto group = m_group.lock();
	if (!group)
		return;
	if (group->GetAlpha() == val)
		return;

	group->SetAlpha(val);
	FluoRefresh(1, { gstAlpha }, { glbin_current.GetViewId() });
}

void VolumePropPanelAgent::SyncShadingStrength(double val)
{
	auto group = m_group.lock();
	if (!group)
		return;
	if (group->GetShadingStrength() == val)
		return;

	group->SetShadingStrength(val);
	FluoRefresh(1, { gstShading }, { glbin_current.GetViewId() });
}

void VolumePropPanelAgent::SyncShadingShine(double val)
{
	auto group = m_group.lock();
	if (!group)
		return;
	if (group->GetShadingShine() == val)
		return;

	group->SetShadingShine(val);
	FluoRefresh(1, { gstShading }, { glbin_current.GetViewId() });
}

void VolumePropPanelAgent::SyncBoundary(double val1, double val2)
{
	auto group = m_group.lock();
	if (!group)
		return;
	if (group->GetBoundaryLow() == val1 &&
		group->GetBoundaryHigh() == val2)
		return;

	group->SetBoundaryLow(val1);
	group->SetBoundaryHigh(val2);
	FluoRefresh(1, { gstBoundary }, { glbin_current.GetViewId() });
}

void VolumePropPanelAgent::SyncThresh(double val1, double val2)
{
	auto group = m_group.lock();
	if (!group)
		return;
	if (group->GetLeftThresh() == val1 &&
		group->GetRightThresh() == val2)
		return;

	group->SetLeftThresh(val1);
	group->SetRightThresh(val2);
	FluoRefresh(1, { gstThreshold, gstBrushCountAutoUpdate, gstColocalAutoUpdate }, { glbin_current.GetViewId() });
}

void VolumePropPanelAgent::SyncShadowInt(double val)
{
	auto group = m_group.lock();
	if (!group)
		return;
	if (group->GetShadowIntensity() == val)
		return;

	group->SetShadowIntensity(val);
	FluoRefresh(1, { gstShadow }, { glbin_current.GetViewId() });
}

void VolumePropPanelAgent::SyncSampleRate(double val)
{
	auto view = m_view.lock();
	auto group = m_group.lock();

	bool depth_view = view->GetChannelMixMode() == ChannelMixMode::Depth;
	if (depth_view)
		view->SetSampleRate(val);
	else
		group->SetSampleRate(val);
	FluoRefresh(1, { gstSampleRate }, { glbin_current.GetViewId() });
}

void VolumePropPanelAgent::SyncColormapVal(double val1, double val2)
{
	auto group = m_group.lock();
	if (!group)
		return;
	if (group->GetColormapLow() == val1 &&
		group->GetColormapHigh() == val2)
		return;

	group->SetColormapValues(val1, val2);
	FluoRefresh(1, { gstColormap }, { glbin_current.GetViewId() });
}

//ml
void VolumePropPanelAgent::SetMachineLearning()
{
	ApplyMl();
	//settings not managed by ml
	//component display
	//int ival = glbin_vol_def.m_label_mode;
	//m_options_toolbar->ToggleTool(ID_CompChk, ival ? true : false);
	//if (m_sync_group && m_group)
	//	m_group->SetLabelMode(ival);
	//else
	//	m_vd->SetLabelMode(ival);
}

void VolumePropPanelAgent::SetTransparent()
{
	bool bval = m_options_toolbar->GetToolState(ID_TranspChk);
	EnableTransparent(bval);
}

void VolumePropPanelAgent::SetMIP()
{
	int val = m_options_toolbar->GetToolState(ID_MipChk) ? 1 : 0;
	EnableMip(val);
}

void VolumePropPanelAgent::SetInvert()
{
	bool inv = m_options_toolbar->GetToolState(ID_InvChk);

	auto group = m_group.lock();
	auto vd = m_vd.lock();
	if (m_sync_group && group)
		group->SetInvert(inv);
	else if (vd)
		vd->SetInvert(inv);

	FluoRefresh(0, { gstInvert, gstConvVolMeshUpdateTransf }, { glbin_current.GetViewId() });
}

void VolumePropPanelAgent::SetOutline()
{
	bool bval = m_options_toolbar->GetToolState(ID_OutlineChk);

	auto group = m_group.lock();
	auto vd = m_vd.lock();
	if (m_sync_group && group)
		group->SetOutline(bval);
	else if (vd)
		vd->SetOutline(bval);

	FluoRefresh(0, { gstOutline }, { glbin_current.GetViewId() });
}

//interpolation
void VolumePropPanelAgent::SetInterpolate()
{
	bool inv = m_options_toolbar->GetToolState(ID_InterpolateChk);

	auto view = m_view.lock();
	auto group = m_group.lock();
	auto vd = m_vd.lock();
	if (m_sync_group && group)
		group->SetInterpolate(inv);
	else if (vd)
		vd->SetInterpolate(inv);
	if (view)
		view->SetIntp(inv);

	FluoRefresh(0, { gstInterpolate }, { glbin_current.GetViewId() });
}

//noise reduction
void VolumePropPanelAgent::SetNoiseReduction()
{
	bool val = m_options_toolbar->GetToolState(ID_NoiseReductChk);

	auto view = m_view.lock();
	auto group = m_group.lock();
	auto vd = m_vd.lock();
	bool depth_view = view && view->GetChannelMixMode() == ChannelMixMode::Depth;
	bool depth_group = vd && vd->GetChannelMixMode() == ChannelMixMode::Depth;
	if (depth_view)
		view->SetNR(val);
	else if (m_sync_group || depth_group)
		group->SetNR(val);
	else
		vd->SetNR(val);

	FluoRefresh(0, { gstNoiseRedct }, { glbin_current.GetViewId() });
}

//sync within group
void VolumePropPanelAgent::SetSyncGroup()
{
	m_sync_group = m_options_toolbar->GetToolState(ID_SyncGroupChk);
	auto view = m_view.lock();
	auto group = m_group.lock();
	auto vd = m_vd.lock();
	if (group)
		group->SetVolumeSyncProp(m_sync_group);

	if (m_sync_group && group && vd && view)
	{
		//gamma
		group->SetGammaEnable(vd->GetGammaEnable());
		group->SetGamma(vd->GetGamma());
		//minmax
		group->SetMinMaxEnable(vd->GetMinMaxEnable());
		group->SetLowOffset(vd->GetLowOffset());
		group->SetHighOffset(vd->GetHighOffset());
		//alpha
		group->SetAlphaEnable(vd->GetAlphaEnable());
		group->SetAlpha(vd->GetAlpha());
		//shading
		group->SetShadingEnable(vd->GetShadingEnable());
		group->SetShadingStrength(vd->GetShadingStrength());
		//high shading
		group->SetShadingShine(vd->GetShadingShine());
		//boundary
		group->SetBoundaryEnable(vd->GetBoundaryEnable());
		group->SetBoundaryLow(vd->GetBoundaryLow());
		group->SetBoundaryHigh(vd->GetBoundaryHigh());
		//left threshold
		group->SetThreshEnable(vd->GetThreshEnable());
		group->SetLeftThresh(vd->GetLeftThresh());
		//right thresh
		group->SetRightThresh(vd->GetRightThresh());
		//shadow
		group->SetShadowEnable(vd->GetShadowEnable());
		group->SetShadowIntensity(vd->GetShadowIntensity());
		//sample rate
		group->SetSampleRateEnable(vd->GetSampleRateEnable());
		group->SetSampleRate(vd->GetSampleRate());
		//inversion
		group->SetInvert(vd->GetInvert());
		//interpolation
		group->SetInterpolate(vd->GetInterpolate());
		if (view)
			view->SetIntp(vd->GetInterpolate());
		//MIP
		group->SetRenderMode(vd->GetRenderMode());
		//transp
		group->SetAlphaPower(vd->GetAlphaPower());
		//noise reduction
		group->SetNR(vd->GetNR());
		//colormap values
		group->SetColormapValues(vd->GetColormapLow(), vd->GetColormapHigh());
		//colormap type
		group->SetColormap(vd->GetColormap());
		//colormap inv
		group->SetColormapInv(vd->GetColormapInv());
		//colormap proj
		group->SetColormapProj(vd->GetColormapProj());
		//color mode
		group->SetMainMaskMode(vd->GetMainColorMode());
		group->SetMaskMode(vd->GetMaskColorMode());
	}

	FluoRefresh(1, { gstVolumeProps }, { glbin_current.GetViewId() });
}

//depth mode
void VolumePropPanelAgent::SetChannelMixDepth()
{
	bool val = m_options_toolbar->GetToolState(ID_ChannelMixDepthChk);

	auto group = m_group.lock();
	auto vd = m_vd.lock();
	if (val)
	{
		if (group)
		{
			group->SetChannelMixMode(ChannelMixMode::Depth);
			if (vd)
			{
				group->SetNR(vd->GetNR());
				group->SetSampleRate(vd->GetSampleRate());
				group->SetShadowEnable(vd->GetShadowEnable());
				group->SetShadowIntensity(vd->GetShadowIntensity());
			}
		}
	}
	else
	{
		if (group)
			group->SetChannelMixMode(ChannelMixMode::CompositeAdd);
	}

	FluoRefresh(0, { gstChannelMixMode }, { glbin_current.GetViewId() });
}

//legend
void VolumePropPanelAgent::SetLegend()
{
	bool leg = m_options_toolbar->GetToolState(ID_LegendChk);

	auto vd = m_vd.lock();
	if (vd)
		vd->SetLegend(leg);

	FluoRefresh(1, { gstLegend }, { glbin_current.GetViewId() });
}

void VolumePropPanelAgent::SaveDefault()
{
	auto vd = m_vd.lock();
	bool use_ml = glbin.get_vp_table_enable() && vd;
	if (use_ml)
		SaveMl();
	else
		glbin_vol_def.Set(*vd);
}

void VolumePropPanelAgent::ResetDefault()
{
	auto vd = m_vd.lock();
	if (!vd)
		return;

	m_thresh_sldr->SetLink(false);
	m_colormap_sldr->SetLink(false);

	auto group = m_group.lock();
	if (m_sync_group && group)
		glbin_vol_def.Apply(*group);
	else
		glbin_vol_def.Apply(*vd);

	FluoRefresh(0, { gstVolumeProps, gstConvVolMeshUpdateTransf }, { glbin_current.GetViewId() });
}

void VolumePropPanelAgent::SetSpacing()
{
	if (!m_space_x_text || !m_space_y_text || !m_space_z_text)
		return false;

	wxString str, str_new;
	double spcx = 0.0;
	double spcy = 0.0;
	double spcz = 0.0;

	str = m_space_x_text->GetValue();
	str.ToDouble(&spcx);
	if (spcx <= 0.0)
		return false;

	str = m_space_y_text->GetValue();
	str.ToDouble(&spcy);
	if (spcy <= 0.0)
		return false;

	str = m_space_z_text->GetValue();
	str.ToDouble(&spcz);
	if (spcz <= 0.0)
		return;

	fluo::Vector spc(spcx, spcy, spcz);
	auto group = m_group.lock();
	auto vd = m_vd.lock();
	if (m_sync_group && group)
	{
		for (int i = 0; i < group->GetVolumeNum(); i++)
		{
			auto gvd = group->GetVolumeData(i);
			if (gvd &&
				gvd->GetSpacingSource() != SpacingSource::FromFile)
			{
				gvd->SetSpacingSource(SpacingSource::FromUser);
				gvd->SetSpacing(spc);
				gvd->SetBaseSpacing(spc);
			}
		}
	}
	else if (vd)
	{
		vd->SetSpacingSource(SpacingSource::FromUser);
		vd->SetSpacing(spc);
		vd->SetBaseSpacing(spc);
	}

	NotifyViewUpdate({ gstSpacing, gstInitViewBoundsCenter });
}

//update max value
void VolumePropPanelAgent::UpdateMaxVal(double value)
{
	auto vd = m_vd.lock();
	if (!vd) return;
	int bits = vd->GetBits();
	if (bits == 8)
		return;
	else if (bits > 8)
	{
		if (value < 255.0)
			value = 255.0;
		if (value > 65535.0)
			value = 65535.0;
	}
	m_max_val = value;
	vd->SetMinMaxValue(vd->GetMinValue(), m_max_val);
	vd->SetScalarScale(65535.0 / m_max_val);

	m_minmax_sldr->SetRange(0, std::round(m_max_val));
	m_thresh_sldr->SetRange(0, std::round(m_max_val));
	m_luminance_sldr->SetRange(0, std::round(m_max_val));
	m_alpha_sldr->SetRange(0, std::round(m_max_val));
	m_colormap_sldr->SetRange(0, std::round(m_max_val));
}

