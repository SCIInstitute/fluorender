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
#ifndef _VOLUME_GROUP_H_
#define _VOLUME_GROUP_H_

#include <TreeLayer.h>
#include <nrrd.h>
#include <vector>
#include <memory>

class VolumeData;
enum class ChannelMixMode : int;
namespace flvr
{
	enum class RenderMode : int;
	enum class ColorMode : int;
	enum class ColormapProj : int;
	enum class MaskMode : int;
}
class VolumeGroup : public TreeLayer
{
public:
	VolumeGroup();
	virtual ~VolumeGroup();

	//reset counter
	static void ResetID()
	{
		m_num = 0;
	}
	static void SetID(int id)
	{
		m_num = id;
	}
	static int GetID()
	{
		return m_num;
	}

	int GetVolumeNum()
	{
		return static_cast<int>(m_vd_list.size());
	}
	std::shared_ptr<VolumeData> GetVolumeData(int index)
	{
		if (index>=0 && index<(int)m_vd_list.size())
			return m_vd_list[index];
		else return nullptr;
	}
	void InsertVolumeData(int index, const std::shared_ptr<VolumeData>& vd)
	{
		if (!m_vd_list.empty())
		{
			if (index>-1 && index<(int)m_vd_list.size())
				m_vd_list.insert(m_vd_list.begin()+(index+1), vd);
			else if (index == -1)
				m_vd_list.insert(m_vd_list.begin()+0, vd);
		}
		else
		{
			m_vd_list.push_back(vd);
		}
	}
	void ReplaceVolumeData(int index, const std::shared_ptr<VolumeData>& vd)
	{
		if (index >= 0 && index<(int)m_vd_list.size())
			m_vd_list[index] = vd;
		ResetSync();
	}
	void RemoveVolumeData(int index)
	{
		if (index>=0 && index<(int)m_vd_list.size())
			m_vd_list.erase(m_vd_list.begin()+index);
		ResetSync();
	}

	//display functions
	void SetDisp(bool disp)
	{
		m_disp = disp;
	}
	void ToggleDisp()
	{
		m_disp = !m_disp;
	}
	bool GetDisp()
	{
		return m_disp;
	}

	//group blend mode
	void SetChannelMixMode(ChannelMixMode mode);
	ChannelMixMode GetChannelMixMode();

	//set gamma to all
	void SetGammaAll(const fluo::Color &gamma);
	//set brightness to all
	void SetBrightnessAll(const fluo::Color &brightness);
	//set hdr to all
	void SetHdrAll(const fluo::Color &hdr);
	//set sync to all
	void SetSyncAll(int i, bool val);
	//reset sync
	void ResetSync();

	//volume properties
	void SetGammaEnable(bool);
	void SetGamma(double val, bool set_this = true);
	double GetGamma();
	void SetBoundaryEnable(bool);
	void SetBoundaryLow(double, bool set_this = true);
	double GetBoundaryLow();
	void SetBoundaryHigh(double, bool set_this = true);
	double GetBoundaryHigh();
	void SetBoundaryMax(double val);
	double GetBoundaryMax();
	void SetMinMaxEnable(bool);
	void SetLowOffset(double, bool set_this = true);
	double GetLowOffset();
	void SetHighOffset(double, bool set_this = true);
	double GetHighOffset();
	void SetThreshEnable(bool);
	void SetLeftThresh(double, bool set_this = true);
	double GetLeftThresh();
	void SetRightThresh(double, bool set_this = true);
	double GetRightThresh();
	double GetSoftThreshold();
	void SetLuminanceEnable(bool);
	void SetLuminance(double, bool set_this = true);
	double GetLuminance();
	void SetAlphaEnable(bool);
	void SetAlpha(double, bool set_this = true);
	double GetAlpha();
	void SetShadingEnable(bool);
	void SetShadingStrength(double);
	void SetShadingShine(double);
	double GetShadingStrength();
	double GetShadingShine();
	void SetShadowEnable(bool);
	void SetShadowIntensity(double);
	double GetShadowIntensity();
	void SetSampleRateEnable(bool);
	void SetSampleRate(double, bool set_this = true);
	double GetSampleRate();

	void SetColorMode(flvr::ColorMode mode);
	void SetColormapDisp(bool disp);
	void SetColormapValues(double low, double high);
	double GetColormapLow();
	double GetColormapHigh();
	void SetColormapInv(double val);
	void SetColormap(int value);
	void SetColormapProj(flvr::ColormapProj value);
	void SetRenderMode(flvr::RenderMode mode);
	void SetAlphaPower(double val);
	void SetNR(bool val);
	void SetInterpolate(bool mode);
	void SetInvert(bool mode);
	void SetTransparent(bool val);

	//mask mode
	void SetMainMaskMode(flvr::MaskMode mode);
	void SetMaskMode(flvr::MaskMode mode);

	//use ml
	void ApplyMlVolProp();

	//sync prop
	void SetVolumeSyncProp(bool bVal)
	{
		m_sync_volume_prop = bVal;
	}
	bool GetVolumeSyncProp()
	{
		return m_sync_volume_prop;
	}

	//randomize color
	void RandomizeColor();

	void AddMask(Nrrd* mask, int op);//op: 0-replace; 1-union; 2-exclude; 3-intersect

	virtual void SetOutline(bool outline) override;

private:
	static int m_num;
	std::vector<std::shared_ptr<VolumeData>> m_vd_list;
	bool m_sync_volume_prop;

	bool m_disp;

	//synced values
	double m_gamma;
	double m_boundary_low;
	double m_boundary_high;
	double m_boundary_max;
	double m_lo_offset;
	double m_hi_offset;
	double m_lo_thresh;
	double m_hi_thresh;
	double m_shading_strength;
	double m_shading_shine;
	double m_sw;
	double m_luminance;
	double m_alpha;
	double m_shadow_intensity;
	double m_sample_rate;
	double m_colormap_low;
	double m_colormap_high;
};

#endif//_VOLUME_GROUP_H_