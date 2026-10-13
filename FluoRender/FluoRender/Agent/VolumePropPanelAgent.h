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
#ifndef VolumePropPanelAgent_h
#define VolumePropPanelAgent_h

#include <Agent.h>
#include <Tokens.h>
#include <memory>

class VolumePropPanel;
class VolumeData;
class VolumeGroup;
class RenderView;

class VolumePropPanelAgent : public Agent
{
public:
	VolumePropPanelAgent(
		VolumePropPanel* panel,
		const std::shared_ptr<VolumeData>& vd,
		const std::shared_ptr<VolumeGroup>& group,
		const std::shared_ptr<RenderView>& view);

	virtual ~VolumePropPanelAgent() = default;

	VolumePropPanel* GetPanel() const;

	std::shared_ptr<VolumeData> GetData() const
	{
		return m_vd.lock();
	}

	void SetGroup(const std::shared_ptr<VolumeGroup>& group);
	void SetView(const std::shared_ptr<RenderView>& view);

	double GetBoundaryMax();

protected:
	std::span < const std::string_view>
		AcceptedValues() const override
	{
		return kAcceptedValues;
	}

	void UpdateUI(const UpdateRequest& request) override;

	void UpdateData(const UpdateRequest& request) override;

private:
	static constexpr std::string_view kAcceptedValues[] =
	{
		gstCurrentSelect,
	};

	std::weak_ptr<VolumeData> m_vd;
	std::weak_ptr<VolumeGroup> m_group;
	std::weak_ptr<RenderView> m_view;

	bool m_sync_group = false;
	double m_max_val = 255.0;

private:
	void UpdateGradient();

	void SetMinMax();
	void SyncMinMax();
	void SetDefaultMinMax();
	void SetMlMinMax();
	void EnableMinMax();
	void ToggleMinMax();

	void SetGamma();
	void SyncGamma();
	void SetDefaultGamma();
	void SetMlGamma();
	void EnableGamma();
	void ToggleGamma();

	void SetAlpha();
	void SyncAlpha();
	void SetDefaultAlpha();
	void SetMlAlpha();
	void EnableAlpha();
	void ToggleAlpha();

	void SetLuminance();
	void SyncLuminance();
	void SetDefaultLuminance();
	void SetMlLuminance();
	void EnableLuminance();
	void ToggleLuminance();

	void SetSampleRate();
	void SyncSampleRate();
	void SetDefaultSampleRate();
	void SetMlSampleRate();
	void EnableSampleRate();
	void ToggleSampleRate();

	void SetThreshold();
	void SyncThreshold();
	void SetDefaultThreshold();
	void SetMlThreshold();
	void EnableThreshold();
	void ToggleThreshold();

	void SetBoundary();
	void SyncBoundary();
	void SetDefaultBoundary();
	void SetMlBoundary();
	void EnableBoundary();
	void ToggleBoundary();

	void SetShadingStrength();
	void SetShadingShine();
	void SyncShading();
	void SetDefaultShading();
	void SetMlShading();
	void EnableShading();
	void ToggleShading();

	void SetShadowIntensity();
	void SyncShadowIntensity();
	void SetDefaultShadowIntensity();
	void SetMlShadowIntensity();
	void EnableShadowIntensity();
	void ToggleShadowIntensity();
	void SetShadowDirection();
	void EnableShadowDirection();

	void SetColormap();
	void SetColormapSlider();
	void SyncColormap();
	void SetDefaultColormap();
	void SetMlColormap();
	void EnableColormap();
	void ToggleColormap();

	void InvertColormap();
	void SetColormapType();
	void SetColormapProj();

	void SetMainColorMode();
	void SetMainColor();
	void SetAltColorMode();
	void SetAltColor();

	void SetSpacing();

	//update max value
	void UpdateMaxVal(double value);

	//enable/disable
	//1
	void EnableMinMax(bool);
	void EnableGamma(bool);
	void EnableAlpha(bool);
	void EnableLuminance(bool);
	void EnableSampleRate(bool);
	//2
	void EnableThreshold(bool);
	void EnableBoundary(bool);
	void EnableShading(bool);
	void EnableShadow(bool);
	void EnableShadowDir(bool);
	void EnableColormap(bool);
	//3
	void EnableMip(bool);
	void EnableTransparent(bool);

	//set values
	void SetMinMax(double, double);
	void SetGamma(double);
	void SetAlpha(double);
	void SetLuminance(double);
	void SetSampleRate(double);
	void SetThreshold(double, double);
	void SetBoundary(double, double);
	void SetShadingStrength(double);
	void SetShadingShine(double);
	void SetShadowInt(double);
	void SetShadowDir(double);
	void SetColormapVal(double, double);

	//sync values
	void SyncMinMax(double, double);
	void SyncGamma(double);
	void SyncAlpha(double);
	void SyncLuminance(double);
	void SyncSampleRate(double);
	void SyncThreshold(double, double);
	void SyncBoundary(double, double);
	void SyncShadingStrength(double);
	void SyncShadingShine(double);
	void SyncShadowInt(double);
	void SyncColormapVal(double, double);

	//options
	void SetMachineLearning();
	void SetTransparent();
	void SetMIP();
	void SetInvert();
	void SetOutline();
	void SetInterpolate();
	void SetNoiseReduction();
	void SetSyncGroup();
	void SetChannelMixDepth();
	void SetLegend();
	void SaveMl();
	void SaveDefault();
	void ResetDefault();

};

#endif // VolumePropPanelAgent_h
