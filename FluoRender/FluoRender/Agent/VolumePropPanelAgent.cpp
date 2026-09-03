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
#include <Names.h>
#include <MainSettings.h>
#include <VolumeData.h>
#include <VolumeGroup.h>
#include <RenderView.h>
#include <ShaderProgram.h>

VolumePropPanelAgent::VolumePropPanelAgent(
	VolumePropPanel* panel) :
	Agent(panel)
{

}

bool VolumePropPanelAgent::Accept(
	const UpdateRequest& request) const
{
	return true;
}

void VolumePropPanelAgent::Update(
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

void VolumePropPanelAgent::UpdateUI(const UpdateRequest& request)
{
	auto panel = GetPanel();
	if (!panel)
		return;
	auto vd = GetData();
	if (!vd)
		return;

	if (FOUND_VALUE(gstNull))
		return;

	//std::chrono::time_point t = std::chrono::high_resolution_clock::now();

	double dval = 0.0;
	int ival = 0;
	bool bval;
	fluo::Color cval;

	//maximum value
	m_max_val = vd->GetMaxValue();
	m_max_val = std::max(255.0, m_max_val);

	bool update_all = request.values.empty() || FOUND_VALUE(gstVolumeProps);
	bool update_tips = update_all || FOUND_VALUE(gstMultiFuncTips);
	bool update_gamma = update_all || FOUND_VALUE(gstGamma3d);
	bool update_boundary = update_all || FOUND_VALUE(gstBoundary);
	bool update_minmax = update_all || FOUND_VALUE(gstMinMax);
	bool update_threshold = update_all || FOUND_VALUE(gstThreshold);
	bool update_color = update_all || FOUND_VALUE(gstColor);
	bool update_alpha = update_all || FOUND_VALUE(gstAlpha);
	bool update_luminance = update_all || FOUND_VALUE(gstLuminance);
	bool update_shading = update_all || FOUND_VALUE(gstShading);
	bool update_shadow = update_all || FOUND_VALUE(gstShadow);
	bool update_sample = update_all || FOUND_VALUE(gstSampleRate);
	bool update_colormap = update_all || FOUND_VALUE(gstColormap);
	bool update_histogram = update_all || FOUND_VALUE(gstUpdateHistogram);
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
	if (update_minmax || FOUND_VALUE(gstTransparent))
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
	if (update_all || FOUND_VALUE(gstSpacing))
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
	if (update_all || FOUND_VALUE(gstMainMode))
	{
		auto main_mode = vd->GetMainColorMode();
		panel->UpdateMainMode(main_mode);
	}

	if (update_all || FOUND_VALUE(gstMaskMode))
	{
		auto mask_mode = vd->GetMaskColorMode();
		panel->UpdateMaskMode(mask_mode);
	}

	//inversion
	if (update_all || FOUND_VALUE(gstInvert))
	{
		bval = vd->GetInvert();
		panel->UpdateInvert(bval);
	}

	//MIP
	if (update_all || FOUND_VALUE(gstRenderMode))
	{
		bval = vd->GetRenderMode() == flvr::RenderMode::Mip;
		panel->UpdateRenderMode(bval);
	}

	//transparency
	if (update_all || FOUND_VALUE(gstTransparent))
	{
		bval = vd->GetAlphaPower() > 1.1;
		panel->UpdateTransparent(bval);
	}

	//legend
	if (update_all || FOUND_VALUE(gstLegend))
	{
		bval = vd->GetLegend();
		panel->UpdateLegend(bval);
	}

	//outline
	if (update_all || FOUND_VALUE(gstOutline))
	{
		bval = vd->GetOutline();
		panel->UpdateOutline(bval);
	}

	//interpolate
	if (update_all || FOUND_VALUE(gstInterpolate))
	{
		bval = vd->GetInterpolate();
		panel->UpdateInterpolate(bval);
	}

	//sync group
	if (update_all || FOUND_VALUE(gstSyncGroup))
	{
		auto group = m_group.lock();
		if (group)
			m_sync_group = group->GetVolumeSyncProp();
		panel->UpdateSyncGroup(m_sync_group);
	}

	//noise reduction
	if (update_all || FOUND_VALUE(gstNoiseRedct))
	{
		bval = vd->GetNR();
		panel->UpdateNoiseRedct(bval);
	}

	//blend mode
	if (update_all || FOUND_VALUE(gstChannelMixMode))
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
	auto vd = GetData();
	if (!vd)
		return;

	if (FOUND_VALUE(gstNull))
		return;

	bool update_all = request.values.empty() || FOUND_VALUE(gstVolumeProps);
	bool update_colormap = update_all || FOUND_VALUE(gstColormap);
	
	//colormap
	if (update_colormap ||
		FOUND_VALUE(gstRulerList) ||
		FOUND_VALUE(gstRulerListCur))
	{
		auto proj = vd->GetColormapProj();
		if (proj == flvr::ColormapProj::Radial ||
			proj == flvr::ColormapProj::Linear)
		{
			vd->UpdateGradient();
		}
	}

}
