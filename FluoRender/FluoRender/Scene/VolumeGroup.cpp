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
#include <VolumeGroup.h>
#include <VolumeData.h>

int VolumeGroup::m_num = 0;
VolumeGroup::VolumeGroup()
{
	type = 5;//group
	m_num++;
	m_name = L"Group " + std::to_wstring(m_num);
	m_disp = true;
	m_sync_volume_prop = false;

	m_gamma = 1.0;
	m_boundary_low = 0.0;
	m_boundary_high = 0.5;
	m_boundary_max = 0.5;
	m_lo_offset = 0.0;
	m_hi_offset = 1.0;
	m_lo_thresh = 0.0;
	m_hi_thresh = 1.0;
	m_sw = 0.0;
	m_luminance = 1.0;
	m_alpha = 1.0;
	m_mat_amb = 1.0;
	m_mat_diff = 1.0;
	m_mat_spec = 1.0;
	m_mat_shine = 10;
	m_shadow_intensity = 0.0;
	m_sample_rate = 2.0;
	m_colormap_low = 0.0;
	m_colormap_high = 1.0;
}

VolumeGroup::~VolumeGroup()
{
}

int VolumeGroup::GetBlendMode()
{
	if (!m_vd_list.empty())
		return m_vd_list[0]->GetBlendMode();
	else
		return 0;
}

//set gamma to all
void VolumeGroup::SetGammaAll(const fluo::Color &gamma)
{
	SetGammaColor(gamma);
	for (auto& it : m_vd_list)
	{
		if (it)
			it->SetGammaColor(gamma);
	}
}

//set brightness to all
void VolumeGroup::SetBrightnessAll(const fluo::Color &brightness)
{
	SetBrightness(brightness);
	for (auto& it : m_vd_list)
	{
		if (it)
			it->SetBrightness(brightness);
	}
}

//set Hdr to all
void VolumeGroup::SetHdrAll(const fluo::Color &hdr)
{
	SetHdr(hdr);
	for (auto& it : m_vd_list)
	{
		if (it)
			it->SetHdr(hdr);
	}
}

//set sync red to all
void VolumeGroup::SetSyncAll(int i, bool val)
{
	SetSync(i, val);
	for (auto& it : m_vd_list)
	{
		if (it)
			it->SetSync(i, val);
	}
}

void VolumeGroup::ResetSync()
{
	int cnt = 0;
	bool r_v = false;
	bool g_v = false;
	bool b_v = false;

	for (auto& it : m_vd_list)
	{
		if (it)
		{
			fluo::Color c = it->GetColor();
			bool r, g, b;
			r = g = b = false;
			cnt = 0;
			if (c.r()>0) {cnt++; r=true;}
			if (c.g()>0) {cnt++; g=true;}
			if (c.b()>0) {cnt++; b=true;}

			if (cnt > 1)
			{
				r_v = r_v||r;
				g_v = g_v||g;
				b_v = b_v||b;
			}
		}
	}

	SetSyncAll(0, r_v);
	SetSyncAll(1, g_v);
	SetSyncAll(2, b_v);
}

//volume properties
void VolumeGroup::SetGammaEnable(bool bval)
{
	for (auto& it : m_vd_list)
	{
		if (it)
			it->SetGammaEnable(bval);
	}
}

void VolumeGroup::SetGamma(double val, bool set_this)
{
	m_gamma = val;
	for (auto& it : m_vd_list)
	{
		if (it)
			it->SetGamma(val, set_this);
	}
}

double VolumeGroup::GetGamma()
{
	return m_gamma;
}

void VolumeGroup::SetBoundaryEnable(bool bval)
{
	for (auto& it : m_vd_list)
	{
		if (it)
			it->SetBoundaryEnable(bval);
	}
}

void VolumeGroup::SetBoundaryLow(double val, bool set_this)
{
	m_boundary_low = val;
	for (auto& it : m_vd_list)
	{
		if (it)
			it->SetBoundaryLow(val, set_this);
	}
}

double VolumeGroup::GetBoundaryLow()
{
	return m_boundary_low;
}

void VolumeGroup::SetBoundaryHigh(double val, bool set_this)
{
	m_boundary_high = val;
	for (auto& it : m_vd_list)
	{
		if (it)
			it->SetBoundaryHigh(val, set_this);
	}
}

double VolumeGroup::GetBoundaryHigh()
{
	return m_boundary_high;
}

void VolumeGroup::SetBoundaryMax(double val)
{
	m_boundary_max = val;
	for (auto& it : m_vd_list)
	{
		if (it)
			it->SetBoundaryMax(val);
	}
}

double VolumeGroup::GetBoundaryMax()
{
	return m_boundary_max;
}

void VolumeGroup::SetMinMaxEnable(bool bval)
{
	for (auto& it : m_vd_list)
	{
		if (it)
			it->SetMinMaxEnable(bval);
	}
}

void VolumeGroup::SetLowOffset(double val, bool set_this)
{
	m_lo_offset = val;
	for (auto& it : m_vd_list)
	{
		if (it)
			it->SetLowOffset(val, set_this);
	}
}

double VolumeGroup::GetLowOffset()
{
	return m_lo_offset;
}

void VolumeGroup::SetHighOffset(double val, bool set_this)
{
	m_hi_offset = val;
	for (auto& it : m_vd_list)
	{
		if (it)
			it->SetHighOffset(val, set_this);
	}
}

double VolumeGroup::GetHighOffset()
{
	return m_hi_offset;
}

void VolumeGroup::SetThreshEnable(bool bval)
{
	for (auto& it : m_vd_list)
	{
		if (it)
			it->SetThreshEnable(bval);
	}
}

void VolumeGroup::SetLeftThresh(double val, bool set_this)
{
	m_lo_thresh = val;
	for (auto& it : m_vd_list)
	{
		if (it)
			it->SetLeftThresh(val, set_this);
	}
}

double VolumeGroup::GetLeftThresh()
{
	return m_lo_thresh;
}

void VolumeGroup::SetRightThresh(double val, bool set_this)
{
	m_hi_thresh = val;
	for (auto& it : m_vd_list)
	{
		if (it)
			it->SetRightThresh(val, set_this);
	}
}

double VolumeGroup::GetRightThresh()
{
	return m_hi_thresh;
}

double VolumeGroup::GetSoftThreshold()
{
	return m_sw;
}

void VolumeGroup::SetLuminanceEnable(bool bval)
{
	for (auto& it : m_vd_list)
	{
		if (it)
			it->SetLuminanceEnable(bval);
	}
}

void VolumeGroup::SetLuminance(double val, bool set_this)
{
	m_luminance = val;
	for (auto& it : m_vd_list)
	{
		if (it)
			it->SetLuminance(val, set_this);
	}
}

double VolumeGroup::GetLuminance()
{
	return m_luminance;
}

void VolumeGroup::SetAlphaEnable(bool mode)
{
	for (auto& it : m_vd_list)
	{
		if (it)
			it->SetAlphaEnable(mode);
	}
}

void VolumeGroup::SetAlpha(double val, bool set_this)
{
	m_alpha = val;
	for (auto& it : m_vd_list)
	{
		if (it)
			it->SetAlpha(val, set_this);
	}
}

double VolumeGroup::GetAlpha()
{
	return m_alpha;
}

void VolumeGroup::SetShadingEnable(bool shading)
{
	for (auto& it : m_vd_list)
	{
		if (it)
			it->SetShadingEnable(shading);
	}
}

void VolumeGroup::SetLowShading(double val)
{
	m_mat_amb = val;
	for (auto& it : m_vd_list)
	{
		if (it)
			it->SetLowShading(val);
	}
}

double VolumeGroup::GetLowShading()
{
	return m_mat_amb;
}

void VolumeGroup::SetHiShading(double val)
{
	m_mat_shine = val;
	for (auto& it : m_vd_list)
	{
		if (it)
			it->SetHiShading(val);
	}
}

double VolumeGroup::GetHiShading()
{
	return m_mat_shine;
}

void VolumeGroup::SetShadowEnable(bool bval)
{
	for (auto& it : m_vd_list)
	{
		if (it)
			it->SetShadowEnable(bval);
	}
}

void VolumeGroup::SetShadowIntensity(double val)
{
	m_shadow_intensity = val;
	for (auto& it : m_vd_list)
	{
		if (it)
			it->SetShadowIntensity(val);
	}
}

double VolumeGroup::GetShadowIntensity()
{
	return m_shadow_intensity;
}

void VolumeGroup::SetSampleRateEnable(bool bval)
{
	for (auto& it : m_vd_list)
	{
		if (it)
			it->SetSampleRateEnable(bval);
	}
}

void VolumeGroup::SetSampleRate(double val, bool set_this)
{
	m_sample_rate = val;
	for (auto& it : m_vd_list)
	{
		if (it)
			it->SetSampleRate(val, set_this);
	}
}

double VolumeGroup::GetSampleRate()
{
	return m_sample_rate;
}

void VolumeGroup::SetColormapMode(int mode)
{
	for (auto& it : m_vd_list)
	{
		if (it)
			it->SetColormapMode(mode);
	}
}

void VolumeGroup::SetColormapDisp(bool disp)
{
	for (auto& it : m_vd_list)
	{
		if (it)
			it->SetColormapDisp(disp);
	}
}

void VolumeGroup::SetColormapValues(double low, double high)
{
	m_colormap_low = low;
	m_colormap_high = high;
	for (auto& it : m_vd_list)
	{
		if (it)
		{
			double l, h;
			it->GetColormapValues(l, h);
			it->SetColormapValues(low<0?l:low, high<0?h:high);
		}
	}
}

double VolumeGroup::GetColormapLow()
{
	return m_colormap_low;
}

double VolumeGroup::GetColormapHigh()
{
	return m_colormap_high;
}

void VolumeGroup::SetColormapInv(double val)
{
	for (auto& it : m_vd_list)
	{
		if (it)
			it->SetColormapInv(val);
	}
}

void VolumeGroup::SetColormap(int value)
{
	for (auto& it : m_vd_list)
	{
		if (it)
			it->SetColormap(value);
	}
}

void VolumeGroup::SetColormapProj(int value)
{
	for (auto& it : m_vd_list)
	{
		if (it)
			it->SetColormapProj(value);
	}
}

void VolumeGroup::SetRenderMode(flvr::RenderMode mode)
{
	for (auto& it : m_vd_list)
	{
		if (it)
			it->SetRenderMode(mode);
	}
}

void VolumeGroup::SetAlphaPower(double val)
{
	for (auto& it : m_vd_list)
	{
		if (it)
			it->SetAlphaPower(val);
	}
}

void VolumeGroup::SetLabelMode(int val)
{
	for (auto& it : m_vd_list)
	{
		if (it)
			it->SetLabelMode(val);
	}
}

void VolumeGroup::SetNR(bool val)
{
	for (auto& it : m_vd_list)
	{
		if (it)
			it->SetNR(val);
	}
}
//inversion
void VolumeGroup::SetInterpolate(bool mode)
{
	for (auto& it : m_vd_list)
	{
		if (it)
			it->SetInterpolate(mode);
	}
}

//inversion
void VolumeGroup::SetInvert(bool mode)
{
	for (auto& it : m_vd_list)
	{
		if (it)
			it->SetInvert(mode);
	}
}

void VolumeGroup::SetTransparent(bool val)
{
	for (auto& it : m_vd_list)
	{
		if (it)
			it->SetTransparent(val);
	}
}

//use ml
void VolumeGroup::ApplyMlVolProp()
{
	for (auto& it : m_vd_list)
	{
		if (it)
			it->ApplyMlVolProp();
	}
}

//blend mode
void VolumeGroup::SetBlendMode(int mode)
{
	for (auto& it : m_vd_list)
	{
		if (it)
			it->SetBlendMode(mode);
	}
}

//randomize color
void VolumeGroup::RandomizeColor()
{
	for (auto& it : m_vd_list)
	{
		if (it)
		{
			double hue = (double)std::rand()/(RAND_MAX) * 360.0;
			fluo::Color color(fluo::HSVColor(hue, 1.0, 1.0));
			it->SetColor(color);
		}
	}
}

void VolumeGroup::AddMask(Nrrd* mask, int op)
{
	for (auto& it : m_vd_list)
	{
		if (it)
			it->AddMask(mask, op);
	}
}
