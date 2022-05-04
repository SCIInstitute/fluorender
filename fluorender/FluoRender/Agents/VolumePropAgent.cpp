/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2022 Scientific Computing and Imaging Institute,
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

#include <VolumePropAgent.hpp>
#include <VolumePropPanel.h>
#include <wx/valnum.h>
#include <png_resource.h>
#include <img/icons.h>

using namespace fluo;

VolumePropAgent::VolumePropAgent(VolumePropPanel &panel) :
	InterfaceAgent(),
	panel_(panel)
{
	setupInputs();
}

void VolumePropAgent::setupInputs()
{
	inputs_ = ValueCollection
	{
		//trasnfer function
		MaxInt,
		Gamma3d,
		ExtractBoundary,
		Saturation,
		LowThreshold,
		HighThreshold,
		Luminance,
		ShadowEnable,
		ShadowInt,
		AlphaEnable,
		Alpha,
		SampleRate,
		ShadingEnable,
		HighShading,
		LowShading,
		//modes
		AlphaPower,
		BlendMode,
		MipMode,
		Invert,
		LabelMode,
		Interpolate,
		SyncGroup,
		NoiseRedct,
		Legend,
		//voxel sizes
		SpcX,
		SpcY,
		SpcZ,
		//color
		Color,
		Hsv,
		SecColor,
		SecColorSet,
		//colormaps
		ColormapEnable,
		ColormapLow,
		ColormapHigh,
		ColormapMode,
		ColormapType,
		ColormapProj,
		ColormapInv
	};
}

void VolumePropAgent::setObject(VolumeData* obj)
{
	InterfaceAgent::setObject(obj);
}

VolumeData* VolumePropAgent::getObject()
{
	return dynamic_cast<VolumeData*>(InterfaceAgent::getObject());
}

void VolumePropAgent::UpdateFui(const ValueCollection &names)
{
	bool update_all = names.empty();
	wxString str;
	double dval = 0.0;
	int ival = 0;
	bool bval = false;
	long lval = 0;

	//set range
	wxFloatingPointValidator<double>* vald_fp;
	wxIntegerValidator<unsigned int>* vald_i;

	//maximum value
	if (update_all || FOUND_VALUE(MaxInt))
	{
		getValue(MaxInt, dval);
		dval = std::max(255.0, dval);
		panel_.m_max_val = dval;
	}

	//volume properties
	//transfer function
	//gamma
	if (update_all || FOUND_VALUE(Gamma3d))
	{
		if ((vald_fp = (wxFloatingPointValidator<double>*)panel_.m_gamma_text->GetValidator()))
			vald_fp->SetRange(0.0, 10.0);
		getValue(Gamma3d, dval);
		panel_.m_gamma_sldr->SetValue(int(dval*100.0 + 0.5));
		str = wxString::Format("%.2f", dval);
		panel_.m_gamma_text->ChangeValue(str);
	}
	//boundary
	if (update_all || FOUND_VALUE(ExtractBoundary))
	{
		if ((vald_fp = (wxFloatingPointValidator<double>*)panel_.m_boundary_text->GetValidator()))
			vald_fp->SetRange(0.0, 1.0);
		getValue(ExtractBoundary, dval);
		panel_.m_boundary_sldr->SetValue(int(dval*2000.0 + 0.5));
		str = wxString::Format("%.4f", dval);
		panel_.m_boundary_text->ChangeValue(str);
	}
	//contrast
	if (update_all || FOUND_VALUE(Saturation))
	{
		if ((vald_i = (wxIntegerValidator<unsigned int>*)panel_.m_saturation_text->GetValidator()))
			vald_i->SetMin(0);
		getValue(Saturation, dval);
		ival = int(dval*panel_.m_max_val + 0.5);
		panel_.m_saturation_sldr->SetRange(0, int(panel_.m_max_val));
		str = wxString::Format("%d", ival);
		panel_.m_saturation_sldr->SetValue(ival);
		panel_.m_saturation_text->ChangeValue(str);
	}
	//left threshold
	if (update_all || FOUND_VALUE(LowThreshold))
	{
		if ((vald_i = (wxIntegerValidator<unsigned int>*)panel_.m_left_thresh_text->GetValidator()))
			vald_i->SetMin(0);
		getValue(LowThreshold, dval);
		ival = int(dval*panel_.m_max_val + 0.5);
		panel_.m_left_thresh_sldr->SetRange(0, int(panel_.m_max_val));
		str = wxString::Format("%d", ival);
		panel_.m_left_thresh_sldr->SetValue(ival);
		panel_.m_left_thresh_text->ChangeValue(str);
	}
	//right threshold
	if (update_all || FOUND_VALUE(HighThreshold))
	{
		if ((vald_i = (wxIntegerValidator<unsigned int>*)panel_.m_right_thresh_text->GetValidator()))
			vald_i->SetMin(0);
		getValue(HighThreshold, dval);
		ival = int(dval*panel_.m_max_val + 0.5);
		panel_.m_right_thresh_sldr->SetRange(0, int(panel_.m_max_val));
		str = wxString::Format("%d", ival);
		panel_.m_right_thresh_sldr->SetValue(ival);
		panel_.m_right_thresh_text->ChangeValue(str);
	}
	//luminance
	if (update_all || FOUND_VALUE(Luminance))
	{
		if ((vald_i = (wxIntegerValidator<unsigned int>*)panel_.m_luminance_text->GetValidator()))
			vald_i->SetMin(0);
		getValue(Luminance, dval);
		ival = int(dval*panel_.m_max_val + 0.5);
		panel_.m_luminance_sldr->SetRange(0, int(panel_.m_max_val));
		str = wxString::Format("%d", ival);
		panel_.m_luminance_sldr->SetValue(ival);
		panel_.m_luminance_text->ChangeValue(str);
	}
	//color
	if (update_all || FOUND_VALUE(Color))
	{
		fluo::Color c;
		getValue(Color, c);
		wxColor wxc((unsigned char)(c.r() * 255 + 0.5),
			(unsigned char)(c.g() * 255 + 0.5),
			(unsigned char)(c.b() * 255 + 0.5));
		panel_.m_color_text->ChangeValue(wxString::Format("%d , %d , %d",
			wxc.Red(), wxc.Green(), wxc.Blue()));
		panel_.m_color_btn->SetColour(wxc);
	}
	if (update_all || FOUND_VALUE(SecColor))
	{
		fluo::Color c;
		getValue(SecColor, c);
		wxColor wxc = wxColor((unsigned char)(c.r() * 255 + 0.5),
			(unsigned char)(c.g() * 255 + 0.5),
			(unsigned char)(c.b() * 255 + 0.5));
		panel_.m_color2_text->ChangeValue(wxString::Format("%d , %d , %d",
			wxc.Red(), wxc.Green(), wxc.Blue()));
		panel_.m_color2_btn->SetColour(wxc);
	}
	//alpha
	if (update_all || FOUND_VALUE(Alpha))
	{
		if ((vald_i = (wxIntegerValidator<unsigned int>*)panel_.m_alpha_text->GetValidator()))
			vald_i->SetMin(0);
		getValue(Alpha, dval);
		ival = int(dval*panel_.m_max_val + 0.5);
		panel_.m_alpha_sldr->SetRange(0, int(panel_.m_max_val));
		str = wxString::Format("%d", ival);
		panel_.m_alpha_sldr->SetValue(ival);
		panel_.m_alpha_text->ChangeValue(str);
	}
	if (update_all || FOUND_VALUE(AlphaEnable))
	{
		bool alpha_enable;
		getValue(AlphaEnable, alpha_enable);
		panel_.m_alpha_tool->ToggleTool(VolumePropPanel::ID_AlphaChk, alpha_enable);
		if (alpha_enable)
			panel_.EnableAlpha();
		else
			panel_.DisableAlpha();
	}

	//shadings
	if (update_all || FOUND_VALUE(LowShading))
	{
		if ((vald_fp = (wxFloatingPointValidator<double>*)panel_.m_low_shading_text->GetValidator()))
			vald_fp->SetRange(0.0, 10.0);
		double amb;
		getValue(LowShading, amb);
		panel_.m_low_shading_sldr->SetValue(amb*100.0);
		str = wxString::Format("%.2f", amb);
		panel_.m_low_shading_text->ChangeValue(str);
	}
	if (update_all || FOUND_VALUE(HighShading))
	{
		if ((vald_fp = (wxFloatingPointValidator<double>*)panel_.m_hi_shading_text->GetValidator()))
			vald_fp->SetRange(0.0, 100.0);
		double shine;
		getValue(HighShading, shine);
		panel_.m_hi_shading_sldr->SetValue(shine*10.0);
		str = wxString::Format("%.2f", shine);
		panel_.m_hi_shading_text->ChangeValue(str);
	}
	if (update_all || FOUND_VALUE(ShadingEnable))
	{
		bool shading_enable;
		getValue(ShadingEnable, shading_enable);
		panel_.m_shade_tool->ToggleTool(VolumePropPanel::ID_ShadingEnableChk, shading_enable);
		if (shading_enable)
			panel_.EnableShading();
		else
			panel_.DisableShading();
	}

	//shadow
	if (update_all || FOUND_VALUE(ShadowEnable))
	{
		if ((vald_fp = (wxFloatingPointValidator<double>*)panel_.m_shadow_text->GetValidator()))
			vald_fp->SetRange(0.0, 1.0);
		bool shadow_enable;
		getValue(ShadowEnable, shadow_enable);
		panel_.m_shadow_tool->ToggleTool(VolumePropPanel::ID_ShadowChk, shadow_enable);
		if (shadow_enable)
			panel_.EnableShadow();
		else
			panel_.DisableShadow();
	}
	if (update_all || FOUND_VALUE(ShadowInt))
	{
		getValue(ShadowInt, dval);
		panel_.m_shadow_sldr->SetValue(int(dval*100.0 + 0.5));
		str = wxString::Format("%.2f", dval);
		panel_.m_shadow_text->ChangeValue(str);
	}

	//smaple rate
	if (update_all || FOUND_VALUE(SampleRate))
	{
		if ((vald_fp = (wxFloatingPointValidator<double>*)panel_.m_sample_text->GetValidator()))
			vald_fp->SetRange(0.0, 100.0);
		getValue(SampleRate, dval);
		panel_.m_sample_sldr->SetValue(dval*10.0);
		str = wxString::Format("%.1f", dval);
		panel_.m_sample_text->ChangeValue(str);
	}

	//spacings
	if (update_all || FOUND_VALUE(SpcX))
	{
		double spcx;
		getValue(SpcX, spcx);
		if ((vald_fp = (wxFloatingPointValidator<double>*)panel_.m_space_x_text->GetValidator()))
			vald_fp->SetMin(0.0);
		str = wxString::Format("%.3f", spcx);
		panel_.m_space_x_text->ChangeValue(str);
	}
	if (update_all || FOUND_VALUE(SpcY))
	{
		double spcy;
		getValue(SpcY, spcy);
		if ((vald_fp = (wxFloatingPointValidator<double>*)panel_.m_space_y_text->GetValidator()))
			vald_fp->SetMin(0.0);
		str = wxString::Format("%.3f", spcy);
		panel_.m_space_y_text->ChangeValue(str);
	}
	if (update_all || FOUND_VALUE(SpcZ))
	{
		double spcz;
		getValue(SpcZ, spcz);
		if ((vald_fp = (wxFloatingPointValidator<double>*)panel_.m_space_z_text->GetValidator()))
			vald_fp->SetMin(0.0);
		str = wxString::Format("%.3f", spcz);
		panel_.m_space_z_text->ChangeValue(str);
	}

	//legend
	if (update_all || FOUND_VALUE(Legend))
	{
		getValue(Legend, bval);
		panel_.m_options_toolbar->ToggleTool(VolumePropPanel::ID_LegendChk, bval);
	}

	//interpolate
	if (update_all || FOUND_VALUE(Interpolate))
	{
		getValue(Interpolate, bval);
		panel_.m_options_toolbar->ToggleTool(VolumePropPanel::ID_InterpolateChk, bval);
		if (bval)
			panel_.m_options_toolbar->SetToolNormalBitmap(VolumePropPanel::ID_InterpolateChk,
				wxGetBitmapFromMemory(interpolate));
		else
			panel_.m_options_toolbar->SetToolNormalBitmap(VolumePropPanel::ID_InterpolateChk,
				wxGetBitmapFromMemory(interpolate_off));
	}

	//sync group
	//if (m_group)
	//	m_sync_group = m_group->GetVolumeSyncProp();
	if (update_all || FOUND_VALUE(SyncGroup))
	{
		bool sync = testSyncParentValue(Gamma3d);
		panel_.m_options_toolbar->ToggleTool(VolumePropPanel::ID_SyncGroupChk, sync);
	}

	//colormap
	//double low, high;
	//vd->GetColormapValues(low, high);
	//low
	if (update_all || FOUND_VALUE(ColormapLow))
	{
		if ((vald_i = (wxIntegerValidator<unsigned int>*)panel_.m_colormap_low_value_text->GetValidator()))
			vald_i->SetMin(0);
		getValue(ColormapLow, dval);
		ival = int(dval*panel_.m_max_val + 0.5);
		panel_.m_colormap_low_value_sldr->SetRange(0, int(panel_.m_max_val));
		str = wxString::Format("%d", ival);
		panel_.m_colormap_low_value_sldr->SetValue(ival);
		panel_.m_colormap_low_value_text->ChangeValue(str);
	}
	//high
	if (update_all || FOUND_VALUE(ColormapHigh))
	{
		if ((vald_i = (wxIntegerValidator<unsigned int>*)panel_.m_colormap_high_value_text->GetValidator()))
			vald_i->SetMin(0);
		getValue(ColormapHigh, dval);
		ival = int(dval*panel_.m_max_val + 0.5);
		panel_.m_colormap_high_value_sldr->SetRange(0, int(panel_.m_max_val));
		str = wxString::Format("%d", ival);
		panel_.m_colormap_high_value_sldr->SetValue(ival);
		panel_.m_colormap_high_value_text->ChangeValue(str);
	}
	//colormap
	if (update_all || FOUND_VALUE(ColormapType))
	{
		getValue(ColormapType, lval);
		panel_.m_colormap_combo->SetSelection(lval);
	}
	if (update_all || FOUND_VALUE(ColormapProj))
	{
		getValue(ColormapProj, lval);
		panel_.m_colormap_combo2->SetSelection(lval);
	}
	//mode
	if (update_all || FOUND_VALUE(ColormapMode))
	{
		getValue(ColormapMode, lval);
		bool colormap_enable = lval == 1;
		panel_.m_colormap_tool->ToggleTool(VolumePropPanel::ID_ColormapEnableChk, colormap_enable);
		if (colormap_enable)
			panel_.EnableColormap();
		else
			panel_.DisableColormap();
	}

	//inversion
	if (update_all || FOUND_VALUE(Invert))
	{
		getValue(Invert, bval);
		panel_.m_options_toolbar->ToggleTool(VolumePropPanel::ID_InvChk, bval);
		if (bval)
			panel_.m_options_toolbar->SetToolNormalBitmap(VolumePropPanel::ID_InvChk,
				wxGetBitmapFromMemory(invert));
		else
			panel_.m_options_toolbar->SetToolNormalBitmap(VolumePropPanel::ID_InvChk,
				wxGetBitmapFromMemory(invert_off));
	}

	//MIP
	if (update_all || FOUND_VALUE(MipMode))
	{
		getValue(MipMode, lval);
		bool mip_enable = lval == 1;
		panel_.m_options_toolbar->ToggleTool(VolumePropPanel::ID_MipChk, mip_enable);
		if (mip_enable)
			panel_.m_threh_st->SetLabel("Shade Threshold : ");
		else
			panel_.m_threh_st->SetLabel("Threshold : ");
		if (mip_enable)
			panel_.EnableMip();
		else
			panel_.DisableMip();
	}

	//noise reduction
	if (update_all || FOUND_VALUE(NoiseRedct))
	{
		getValue(NoiseRedct, bval);
		panel_.m_options_toolbar->ToggleTool(VolumePropPanel::ID_NRChk, bval);
		if (bval)
			panel_.m_options_toolbar->SetToolNormalBitmap(VolumePropPanel::ID_NRChk,
				wxGetBitmapFromMemory(smooth));
		else
			panel_.m_options_toolbar->SetToolNormalBitmap(VolumePropPanel::ID_NRChk,
				wxGetBitmapFromMemory(smooth_off));
	}

	//blend mode
	if (update_all || FOUND_VALUE(BlendMode))
	{
		getValue(BlendMode, lval);
		if (lval == 2)
		{
			panel_.m_options_toolbar->ToggleTool(VolumePropPanel::ID_DepthChk, true);
			panel_.m_options_toolbar->SetToolNormalBitmap(VolumePropPanel::ID_DepthChk, wxGetBitmapFromMemory(depth));
		}
		else
		{
			panel_.m_options_toolbar->ToggleTool(VolumePropPanel::ID_DepthChk, false);
			panel_.m_options_toolbar->SetToolNormalBitmap(VolumePropPanel::ID_DepthChk, wxGetBitmapFromMemory(depth_off));
		}
	}

	//panel_.Layout();
}

void VolumePropAgent::OnLuminanceChanged(Event& event)
{
}

void VolumePropAgent::OnColorChanged(Event& event)
{
}