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

	//set range
	wxFloatingPointValidator<double>* vald_fp;
	wxIntegerValidator<unsigned int>* vald_i;

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
		//if ((vald_fp = (wxFloatingPointValidator<double>*)m_shadow_text->GetValidator()))
		//	vald_fp->SetRange(0.0, 1.0);
		bval = vd->GetShadowEnable();
		dval = vd->GetShadowIntensity();
		str = wxString::Format("%.2f", dval);
		m_shadow_sldr->ChangeValue(std::round(dval * 100.0));
		m_shadow_text->ChangeValue(str);
		m_shadow_chk->SetValue(bval);
		if (m_shadow_sldr->IsEnabled() != bval)
		{
			m_shadow_sldr->Enable(bval);
			m_shadow_text->Enable(bval);
		}
	}
	if (update_all || FOUND_VALUE(gstShadowDir))
	{
		bool bval = glbin_settings.m_shadow_dir;
		m_shadow_dir_chk->ToggleTool(0, bval);
		m_shadow_dir_sldr->Enable(bval);
		m_shadow_dir_text->Enable(bval);
		double dirx = glbin_settings.m_shadow_dir_x;
		double diry = glbin_settings.m_shadow_dir_y;
		if (dirx == 0.0 && diry == 0.0)
			dval = 0.0;
		else
			dval = r2d(atan2(glbin_settings.m_shadow_dir_y, glbin_settings.m_shadow_dir_x)) + 45.0;
		m_shadow_dir_sldr->ChangeValue(std::round(dval));
		m_shadow_dir_text->ChangeValue(wxString::Format("%.0f", dval));
	}
	if (update_shadow || update_tips)
	{
		bval = vd->GetShadowEnable() || mf_enable;
		if (m_shadow_st->IsEnabled() != bval)
			m_shadow_st->Enable(bval);
	}
	//smaple rate
	if (update_sample)
	{
		if ((vald_fp = (wxFloatingPointValidator<double>*)m_sample_text->GetValidator()))
			vald_fp->SetRange(0.0, 100.0);
		bval = vd->GetSampleRateEnable();
		dval = vd->GetSampleRate();
		str = wxString::Format("%.1f", dval);
		m_sample_sldr->ChangeValue(dval * 10.0);
		m_sample_text->ChangeValue(str);
		m_sample_chk->SetValue(bval);
		if (m_sample_sldr->IsEnabled() != bval)
		{
			m_sample_sldr->Enable(bval);
			m_sample_text->Enable(bval);
		}
	}
	if (update_sample || update_tips)
	{
		bval = vd->GetSampleRateEnable() || mf_enable;
		if (m_sample_st->IsEnabled() != bval)
			m_sample_st->Enable(bval);
	}

	//spacings
	if (update_all || FOUND_VALUE(gstSpacing))
	{
		auto spc = vd->GetBaseSpacing();
		if ((vald_fp = (wxFloatingPointValidator<double>*)m_space_x_text->GetValidator()))
			vald_fp->SetMin(0.0);
		str = wxString::Format("%.3f", spc.x());
		m_space_x_text->ChangeValue(str);
		if ((vald_fp = (wxFloatingPointValidator<double>*)m_space_y_text->GetValidator()))
			vald_fp->SetMin(0.0);
		str = wxString::Format("%.3f", spc.y());
		m_space_y_text->ChangeValue(str);
		if ((vald_fp = (wxFloatingPointValidator<double>*)m_space_z_text->GetValidator()))
			vald_fp->SetMin(0.0);
		str = wxString::Format("%.3f", spc.z());
		m_space_z_text->ChangeValue(str);
	}

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

	if (update_colormap)
	{
		//slider
		m_colormap_sldr->SetRange(0, std::round(m_max_val));
		double low, high;
		vd->GetColormapValues(low, high);
		//low
		ival = std::round(low * m_max_val);
		m_colormap_sldr->ChangeLowValue(ival);
		//high
		ival = std::round(high * m_max_val);
		m_colormap_sldr->ChangeHighValue(ival);

		//text
		vd->GetColormapDispValues(low, high);
		double minv, maxv;
		vd->GetColormapRange(minv, maxv);
		bool int_validator = (maxv - minv) > 10.0;
		if (int_validator)
		{
			m_colormap_low_text->SetValidator(wxIntegerValidator<int>());
			str = wxString::Format("%.0f", low);
			m_colormap_low_text->ChangeValue(str);
			m_colormap_hi_text->SetValidator(wxIntegerValidator<int>());
			str = wxString::Format("%.0f", high);
			m_colormap_hi_text->ChangeValue(str);
		}
		else
		{
			m_colormap_low_text->SetValidator(wxFloatingPointValidator<double>());
			str = wxString::Format("%.3f", low);
			m_colormap_low_text->ChangeValue(str);
			m_colormap_hi_text->SetValidator(wxFloatingPointValidator<double>());
			str = wxString::Format("%.3f", high);
			m_colormap_hi_text->ChangeValue(str);
		}

		bval = m_colormap_sldr->GetLink();
		if (bval != m_colormap_link_tb->GetToolState(0))
		{
			m_colormap_link_tb->ToggleTool(0, bval);
			wxBitmapBundle bitmap;
			if (bval)
				bitmap = wxGetBitmap(link);
			else
				bitmap = wxGetBitmap(unlink);
			m_colormap_link_tb->SetToolNormalBitmap(0, bitmap);
		}
		//mode
		bval = vd->GetMainColorMode() == flvr::ColorMode::Colormap ||
			vd->GetMaskColorMode() == flvr::ColorMode::Colormap;
		m_colormap_chk->SetValue(bval);
		if (m_colormap_sldr->IsEnabled() != bval)
		{
			m_colormap_sldr->Enable(bval);
			m_colormap_low_text->Enable(bval);
			m_colormap_hi_text->Enable(bval);
			m_colormap_link_tb->Enable(bval);
		}
		//colormap
		bval = vd->GetColormapInv() > 0.0 ? false : true;
		if (bval != m_colormap_inv_btn->GetToolState(0))
		{
			m_colormap_inv_btn->ToggleTool(0, bval);
			if (bval)
				m_colormap_inv_btn->SetToolNormalBitmap(0,
					wxGetBitmap(invert));
			else
				m_colormap_inv_btn->SetToolNormalBitmap(0,
					wxGetBitmap(invert_off));
		}
		m_colormap_combo->SetSelection(vd->GetColormap());
		flvr::ColormapProj colormap_proj = vd->GetColormapProj();
		ival = 0;
		if (flvr::ShaderParams::ValidColormapProj(colormap_proj))
			ival = static_cast<int>(colormap_proj) - 1;
		m_colormap_proj_combo->SetSelection(ival);
		//show colormap on slider
		std::vector<unsigned char> colormap_data;
		if (vd->GetColormapData(colormap_data))
		{
			wxColor lc, hc;
			cval = vd->GetColorFromColormap(0, true);
			lc = wxColor((unsigned char)(cval.r() * 255 + 0.5),
				(unsigned char)(cval.g() * 255 + 0.5),
				(unsigned char)(cval.b() * 255 + 0.5));
			cval = vd->GetColorFromColormap(1, true);
			hc = wxColor((unsigned char)(cval.r() * 255 + 0.5),
				(unsigned char)(cval.g() * 255 + 0.5),
				(unsigned char)(cval.b() * 255 + 0.5));
			m_colormap_sldr->SetColors(lc, hc);
			m_colormap_sldr->SetMapData(colormap_data);
		}
	}
	if (update_colormap || update_tips)
	{
		bval = mf_enable ||
			vd->GetMainColorMode() == flvr::ColorMode::Colormap ||
			vd->GetMaskColorMode() == flvr::ColorMode::Colormap;
		if (m_colormap_st->IsEnabled() != bval)
			m_colormap_st->Enable(bval);
	}

	//color
	if (update_color)
	{
		cval = vd->GetColor();
		wxColor wxc((unsigned char)(cval.r() * 255 + 0.5),
			(unsigned char)(cval.g() * 255 + 0.5),
			(unsigned char)(cval.b() * 255 + 0.5));
		m_main_color_text->ChangeValue(wxString::Format("%d , %d , %d",
			wxc.Red(), wxc.Green(), wxc.Blue()));
		m_main_color_btn->SetValue(wxc);
		cval = vd->GetMaskColor();
		wxc = wxColor((unsigned char)(cval.r() * 255 + 0.5),
			(unsigned char)(cval.g() * 255 + 0.5),
			(unsigned char)(cval.b() * 255 + 0.5));
		m_alt_color_text->ChangeValue(wxString::Format("%d , %d , %d",
			wxc.Red(), wxc.Green(), wxc.Blue()));
		m_alt_color_btn->SetValue(wxc);
	}

	//mask mode
	if (update_all || FOUND_VALUE(gstMainMode))
	{
		auto main_mode = vd->GetMainColorMode();
		switch (main_mode)
		{
		case flvr::ColorMode::Disabled:
			m_main_color_mode_tb->SetToolNormalBitmap(0,
				wxGetBitmap(clip_none));
			break;
		case flvr::ColorMode::SingleColor:
			m_main_color_mode_tb->SetToolNormalBitmap(0,
				wxGetBitmap(palette));
			break;
		case flvr::ColorMode::Colormap:
			m_main_color_mode_tb->SetToolNormalBitmap(0,
				wxGetBitmap(colormap));
			break;
		case flvr::ColorMode::Component:
			m_main_color_mode_tb->SetToolNormalBitmap(0,
				wxGetBitmap(comp));
			break;
		}
	}

	if (update_all || FOUND_VALUE(gstMaskMode))
	{
		auto mask_mode = vd->GetMaskColorMode();
		switch (mask_mode)
		{
		case flvr::ColorMode::Disabled:
			m_alt_color_mode_tb->SetToolNormalBitmap(0,
				wxGetBitmap(clip_none));
			break;
		case flvr::ColorMode::SingleColor:
			m_alt_color_mode_tb->SetToolNormalBitmap(0,
				wxGetBitmap(palette));
			break;
		case flvr::ColorMode::Colormap:
			m_alt_color_mode_tb->SetToolNormalBitmap(0,
				wxGetBitmap(colormap));
			break;
		case flvr::ColorMode::Component:
			m_alt_color_mode_tb->SetToolNormalBitmap(0,
				wxGetBitmap(comp));
			break;
		}
	}

	//inversion
	if (update_all || FOUND_VALUE(gstInvert))
	{
		bool inv = vd->GetInvert();
		m_options_toolbar->ToggleTool(ID_InvChk, inv);
		if (inv)
			m_options_toolbar->SetToolNormalBitmap(ID_InvChk,
				wxGetBitmap(invert));
		else
			m_options_toolbar->SetToolNormalBitmap(ID_InvChk,
				wxGetBitmap(invert_off));
	}

	//MIP
	if (update_all || FOUND_VALUE(gstRenderMode))
	{
		bool mip = vd->GetRenderMode() == flvr::RenderMode::Mip;
		m_options_toolbar->ToggleTool(ID_MipChk, mip);
	}

	//transparency
	if (update_all || FOUND_VALUE(gstTransparent))
	{
		double alpha_power = vd->GetAlphaPower();
		if (alpha_power > 1.1)
		{
			m_options_toolbar->ToggleTool(ID_TranspChk, true);
			m_options_toolbar->SetToolNormalBitmap(ID_TranspChk,
				wxGetBitmap(transphi));
		}
		else
		{
			m_options_toolbar->ToggleTool(ID_TranspChk, false);
			m_options_toolbar->SetToolNormalBitmap(ID_TranspChk,
				wxGetBitmap(transplo));
		}
	}

	//legend
	if (update_all || FOUND_VALUE(gstLegend))
		m_options_toolbar->ToggleTool(ID_LegendChk, vd->GetLegend());

	//outline
	if (update_all || FOUND_VALUE(gstOutline))
	{
		bval = vd->GetOutline();
		m_options_toolbar->ToggleTool(ID_OutlineChk, bval);
		if (bval)
			m_options_toolbar->SetToolNormalBitmap(ID_OutlineChk,
				wxGetBitmap(outline));
		else
			m_options_toolbar->SetToolNormalBitmap(ID_OutlineChk,
				wxGetBitmap(outline_off));
	}

	//interpolate
	if (update_all || FOUND_VALUE(gstInterpolate))
	{
		bool interp = vd->GetInterpolate();
		m_options_toolbar->ToggleTool(ID_InterpolateChk, interp);
		if (interp)
			m_options_toolbar->SetToolNormalBitmap(ID_InterpolateChk,
				wxGetBitmap(interpolate));
		else
			m_options_toolbar->SetToolNormalBitmap(ID_InterpolateChk,
				wxGetBitmap(interpolate_off));
	}

	//sync group
	if (update_all || FOUND_VALUE(gstSyncGroup))
	{
		auto group = m_group.lock();
		if (group)
			m_sync_group = group->GetVolumeSyncProp();
		m_options_toolbar->ToggleTool(ID_SyncGroupChk, m_sync_group);
	}

	//noise reduction
	if (update_all || FOUND_VALUE(gstNoiseRedct))
	{
		bool nr = vd->GetNR();
		m_options_toolbar->ToggleTool(ID_NoiseReductChk, nr);
		if (nr)
			m_options_toolbar->SetToolNormalBitmap(ID_NoiseReductChk,
				wxGetBitmap(filter));
		else
			m_options_toolbar->SetToolNormalBitmap(ID_NoiseReductChk,
				wxGetBitmap(filter_off));
	}

	//blend mode
	if (update_all || FOUND_VALUE(gstChannelMixMode))
	{
		auto channel_mix_mode = vd->GetChannelMixMode();
		if (channel_mix_mode == ChannelMixMode::Depth)
		{
			m_options_toolbar->ToggleTool(ID_ChannelMixDepthChk, true);
			m_options_toolbar->SetToolNormalBitmap(ID_ChannelMixDepthChk, wxGetBitmap(depth));
		}
		else
		{
			m_options_toolbar->ToggleTool(ID_ChannelMixDepthChk, false);
			m_options_toolbar->SetToolNormalBitmap(ID_ChannelMixDepthChk, wxGetBitmap(depth_off));
		}
	}

	//std::chrono::duration<double> ts = std::chrono::duration_cast<std::chrono::duration<double>>(
	//	std::chrono::high_resolution_clock::now() - t);
	//DBGPRINT(L"update settings, time: %f\n", ts);
	//return;
}

void VolumePropPanelAgent::UpdateData(const UpdateRequest& request)
{

}
