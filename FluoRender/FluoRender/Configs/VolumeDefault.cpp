/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2024 Scientific Computing and Imaging Institute,
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
#include <DataManager.h>
#include <Names.h>

VolumeDataDefault::VolumeDataDefault()
{
	//transfer function settings
	m_gamma_enable = true;
	m_gamma = 1.0;

	m_boundary_enable = true;
	m_boundary = 0.0;

	m_saturation_enable = true;
	m_saturation = 1.0;

	m_thresh_enable = true;
	m_lo_thresh = 0.0;
	m_hi_thresh = 1.0;

	m_luminance_enable = true;
	m_luminance = 1.0;

	m_alpha_enable = true;
	m_alpha = 1.0;

	//shading
	m_shading_enable = false;
	m_low_shading = 1.0;
	m_high_shading = 10.0;

	//shadow
	m_shadow_enable = false;
	m_shadow_intensity = 0.0;

	m_sample_rate_enable = true;
	m_sample_rate = 2.0;

	//colormap mode
	m_colormap_inv = 1.0;
	m_colormap_mode = 0;
	m_colormap_disp = false;
	m_colormap_low_value = 0.0;
	m_colormap_hi_value = 1.0;
	m_colormap_type = 0;
	m_colormap_proj = 0;

	m_noise_rd = false;
	m_interpolate = true;
	m_inverted = false;

	//blend mode
	m_blend_mode = 0;

	//legend
	m_legend = true;

	//lable
	m_label_mode = 1;
}

VolumeDataDefault::~VolumeDataDefault()
{

}

void VolumeDataDefault::ReadDefault(wxFileConfig& f)
{
	double dval;
	int ival;
	bool bval;

	if (f.Exists("/volume default"))
		f.SetPath("/volume default");

	if (f.Read(gstGammaEnable, &bval))
		m_gamma_enable = bval;
	if (f.Read(gstGamma3d, &dval))
		m_gamma = dval;

	if (f.Read(gstBoundaryEnable, &bval))
		m_boundary_enable = bval;
	if (f.Read(gstBoundary, &dval))
		m_boundary = dval;

	if (f.Read(gstSaturationEnable, &bval))
		m_saturation_enable = bval;
	if (f.Read(gstSaturation, &dval))
		m_saturation = dval;

	if (f.Read(gstThresholdEnable, &bval))
		m_thresh_enable = bval;
	if (f.Read(gstLowThreshold, &dval))
		m_lo_thresh = dval;
	if (f.Read(gstHighThreshold, &dval))
		m_hi_thresh = dval;

	if (f.Read(gstLuminanceEnable, &bval))
		m_luminance_enable = bval;
	if (f.Read(gstLuminance, &dval))
		m_luminance = dval;

	if (f.Read(gstAlphaEnable, &bval))
		m_alpha_enable = bval;
	if (f.Read(gstAlpha, &dval))
		m_alpha = dval;

	if (f.Read(gstShadingEnable, &bval))
		m_shading_enable = bval;
	if (f.Read(gstLowShading, &dval))
		m_low_shading = dval;
	if (f.Read(gstHighShading, &dval))
		m_high_shading = dval;

	if (f.Read(gstShadowEnable, &bval))
		m_shadow_enable = bval;
	if (f.Read(gstShadowInt, &dval))
		m_shadow_intensity = dval;

	if (f.Read(gstSampleRateEnable, &bval))
		m_sample_rate_enable = bval;
	if (f.Read(gstSampleRate, &dval))
		m_sample_rate = dval;

	if (f.Read(gstSpcX, &dval))
		m_spcx = dval;
	if (f.Read(gstSpcY, &dval))
		m_spcy = dval;
	if (f.Read(gstSpcZ, &dval))
		m_spcz = dval;

	if (f.Read(gstColormapMode, &ival))
		m_colormap_mode = ival;
	if (f.Read(gstColormapDisp, &bval))
		m_colormap_disp = bval;
	if (f.Read(gstColormapLow, &dval))
		m_colormap_low_value = dval;
	if (f.Read(gstColormapHigh, &dval))
		m_colormap_hi_value = dval;
	if (f.Read(gstColormapInv, &bval))
		m_colormap_inv = bval;
	if (f.Read(gstColormapType, &ival))
		m_colormap_type = ival;
	if (f.Read(gstColormapProj, &ival))
		m_colormap_proj = ival;

	if (f.Read(gstNoiseRedct, &bval))
		m_noise_rd = bval;
	if (f.Read(gstInterpolate, &bval))
		m_interpolate = bval;
	if (f.Read(gstInvert, &bval))
		m_inverted = bval;
	if (f.Read(gstMipMode, &bval))
		m_mip_enable = bval;
	if (f.Read(gstTransparent, &bval))
		m_transparent = bval;
	if (f.Read(gstBlendMode, &ival))
		m_blend_mode = ival;
	if (f.Read(gstLegend, &bval))
		m_legend = ival;
	if (f.Read(gstLabelMode, &ival))
		m_label_mode = ival;
}

void VolumeDataDefault::SaveDefault(wxFileConfig& f)
{

}

void VolumeDataDefault::SetDefault(VolumeData* vd)
{
	if (!vd)
		return;

	m_gamma_enable = vd->GetGammaEnable();
	m_gamma = vd->GetGamma();

	m_boundary_enable = vd->GetBoundaryEnable();
	m_boundary = vd->GetBoundary();

	m_saturation_enable = vd->GetSaturationEnable();
	m_saturation = vd->GetSaturation();

	m_thresh_enable = vd->GetThreshEnable();
	m_lo_thresh = vd->GetLeftThresh();
	m_hi_thresh = vd->GetRightThresh();

	m_luminance_enable = vd->GetLuminanceEnable();
	m_luminance = vd->GetLuminance();

	m_alpha_enable = vd->GetAlphaEnable();
	m_alpha = vd->GetAlpha();


}

void VolumeDataDefault::ApplyDefault(VolumeData* vd)
{

}
