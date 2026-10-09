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
#include <Names.h>
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
	void SetGamma();


	void InitViews(unsigned int type);

	void SetGroup(const std::shared_ptr<VolumeGroup>& group);
	void SetView(const std::shared_ptr<RenderView>& view);

	void ApplyMl();
	void SaveMl();

	bool SetSpacing();

	//update max value
	void UpdateMaxVal(double value);

	//enable/disable
	//1
	void EnableMinMax(bool);
	void EnableGamma(bool);
	void EnableAlpha(bool);
	void EnableLuminance(bool);
	void EnableSample(bool);
	//2
	void EnableThresh(bool);
	void EnableBoundary(bool);
	void EnableShading(bool);
	void EnableShadow(bool);
	void EnableShadowDir(bool);
	void EnableColormap(bool);
	//3
	void EnableMip(bool);
	void EnableTransparent(bool);

	//set values
	void SetMinMax(double, double, bool);
	void SetGamma(double);
	void SetAlpha(double, bool);
	void SetLuminance(double, bool);
	void SetSampleRate(double, bool);
	void SetThresh(double, double, bool);
	void SetBoundary(double, double, bool);
	void SetShadingStrength(double, bool);
	void SetShadingShine(double, bool);
	void SetShadowInt(double, bool);
	void SetShadowDir(double, bool);
	void SetColormapVal(double, double, bool);

	//sync values
	void SyncMinMax(double, double);
	void SyncGamma(double);
	void SyncAlpha(double);
	void SyncLuminance(double);
	void SyncSampleRate(double);
	void SyncThresh(double, double);
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
	void SaveDefault();
	void ResetDefault();

};

#endif // VolumePropPanelAgent_h
