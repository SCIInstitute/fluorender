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
#ifndef SettingDlgAgent_h
#define SettingDlgAgent_h

#include <Agent.h>
#include <Names.h>

class SettingDlg;
class SettingDlgAgent : public Agent
{
public:
	SettingDlgAgent(
		SettingDlg* dlg);

	virtual ~SettingDlgAgent() = default;

	SettingDlg* GetDialog() const;

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
		gstSaveProjectEnable,
		gstFontFile,
		gstSettingsFont,
		gstLineWidth,
		gstPaintHistory,
		gstPencilDist,
		gstMicroBlendEnable,
		gstPeelNum,
		gstPinThreshold,
		gstRotLink,
		gstGradBg,
		gstClearColorBg,
		gstMouseInt,
		gstStreamEnable,
		gstAutomate,
		gstHologramMode,
		gstFullscreenDisplay,
		gstDisplayColorDepth,
		gstWavelengthColors,
		gstMaxTextureSize,
		gstDeviceTree,
		gstSettingsJava,
		gstSaveProjectInc,
		gstRealtimeCompress,
		gstScriptBreakEnable,
		gstInverseSliders,
		gstMulFuncBtn,
		gstConfigFileType,
		gstYDir,
		gstXrApi,
		gstMvHmd,
		gstSbs,
		gstEyeDist,
		gstHoloIp,
		gstLgOffset,
		gstLgQuilt,
		gstLgCameraMode,
		gstMaxTextureSizeEnable,
		gstUpdateOrder,
		gstGraphicsMem,
		gstLargeDataSize,
		gstBrickSize,
		gstResponseTime,
		gstLodOffset,
		gstFontSize,
		gstTextColor,
		gstJavaJvm,
		gstJavaIJ,
		gstJavaBioformats,
		gstJavaJvmBrowse,
		gstJavaIJBrowse,
		gstJavaBioformatsBrowse,
		gstJavaEnable,
		gstJavaIJEnable,
		gstAutomate,
		gstResetSettings,
		gstRecommendedSettings
	};

	bool GetFontList(std::vector<std::string>& list) const;
	
	void SetProjectSave();
	void SetProjectSaveInc();
	void SetRealtimeCompress();
	void SetScriptBreak();
	void SetInverseSliders();
	void SetMulFuncBtn();
	void SetConfigFileType();
	void SetYDir();
	void SetInteractiveQuality();
	void SetPeelingLayers();
	void SetMicroBlend();
	void SetGradBg();
	void SetClearColorBg();
	void SetPinThreshold();
	void SetRotLink();
	void SetHologramMode();
	void SetXrApi();
	void SetMvHmd();
	void SetSbs();
	void SetEyeDist();
	void SetHoloIp();
	void SetLgOffset();
	void SetLgQuilt();
	void SetLgCameraMode();
	void SetDispId();
	void SetColorDepth();
	void SetWavelengthColor();
	void SetMaxTextureSizeEnable();
	void SetMaxTextureSize();
	void SetStreamEnable();
	void SetUpdateOrder();
	void SetGraphicsMem();
	void SetLargeData();
	void SetBrickSize();
	void SetResponseTime();
	void SetDetailLevelOffset();
	void SetFont();
	void SetFontSize();
	void SetTextColor();
	void SetLineWidth();
	void SetPaintHistDepth();
	void SetPencilDist();
	void SetJavaJvm();
	void SetJavaIJ();
	void SetJavaBioformats();
	void SetJavaJvmBrowse();
	void SetJavaIJBrowse();
	void SetJavaBioformatsBrowse();
	void SetJavaEnable();
	void SetJavaIJEnable();
	void SetDevice();
	void SetAutomation();
	void Reset();
	void SetRecommended();
};

#endif // SettingDlgAgent_h
