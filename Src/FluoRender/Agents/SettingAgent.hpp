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
#ifndef _SETTINGAGENT_H_
#define _SETTINGAGENT_H_

#include <InterfaceAgent.hpp>
#include <Root.hpp>

class SettingDlg;
namespace fluo
{
	class SettingAgent : public InterfaceAgent
	{
	public:
		//project page
		DEFINE_ATTR(SaveProjectEnable);				//Save Project
		DEFINE_ATTR(HardwareCompress);				//Compress in graphics memory
		DEFINE_ATTR(FontFile);						//Font
		DEFINE_ATTR(TextSize);						//Size
		DEFINE_ATTR(TextColorMode);					//Color: 0- contrast to bg; 1-same as bg; 2-volume sec color
		DEFINE_ATTR(LineWidth);						//Line Width
		DEFINE_ATTR(PaintHistory);					//Paint History
		//rendering page
		DEFINE_ATTR(MicroBlendEnable);				//Micro Blending
		DEFINE_ATTR(PeelNum);						//Mesh Transparency Quality
		DEFINE_ATTR(ShadowDirEnable);				//Enable direcrional shadow
		DEFINE_ATTR(ShadowDirX);					//convert to angle in deg
		DEFINE_ATTR(ShadowDirY);
		DEFINE_ATTR(PinThresh);						//ray casting threshold value
		DEFINE_ATTR(LinkedRot);						//link rotation to views
		DEFINE_ATTR(GradBg);						//Gradient Background
		DEFINE_ATTR(VrEnable);						//Enable stereo
		DEFINE_ATTR(VrEyeOffset);					//Eye distance
		//Performance
		DEFINE_ATTR(Adaptive);						//Variable Sample Rate
		DEFINE_ATTR(StreamEnable);					//Enable streaming
		DEFINE_ATTR(UpdateOrder);					//Update Order: front to back (1) or back to front (0)
		DEFINE_ATTR(GpuMemSize);					//Graphics Memory
		DEFINE_ATTR(LargeDataSize);					//Large Data Size
		DEFINE_ATTR(BrickSize);						//Brick Size
		DEFINE_ATTR(ResponseTime);					//Response Time
		DEFINE_ATTR(LodOffset);						//Detail Level Offset
		//Format
		DEFINE_ATTR(OverrideVoxSpc);				//Override Voxel Size
		DEFINE_ATTR(WaveColor1);					//Default Colors for Excitation Wavelengths 1
		DEFINE_ATTR(WaveColor2);					//Default Colors for Excitation Wavelengths 2
		DEFINE_ATTR(WaveColor3);					//Default Colors for Excitation Wavelengths 3
		DEFINE_ATTR(WaveColor4);					//Default Colors for Excitation Wavelengths 4
		DEFINE_ATTR(MaxTextureSizeEnable);			//Max Texture Size (check)
		DEFINE_ATTR(MaxTextureSize);				//Max Texture Size
		DEFINE_ATTR(ClPlatformId);					//OpenCL Devices (platform id)
		DEFINE_ATTR(ClDeviceId);					//OpenCL Devices (device id)
		//Java
		DEFINE_ATTR(ImagejMode);					//Package
		DEFINE_ATTR(JvmPath);						//Path 1
		DEFINE_ATTR(ImagejPath);					//Path 2
		DEFINE_ATTR(BioformatsPath);				//Path 3

		//settings used internally, not exposed in the dlg so far
		DEFINE_ATTR(CaptureAlpha);
		DEFINE_ATTR(CaptureFloat);
		DEFINE_ATTR(SkipBrick);
		DEFINE_ATTR(TestSpeed);
		DEFINE_ATTR(TestParam);
		DEFINE_ATTR(TestWiref);
		DEFINE_ATTR(TimeFileId);
		DEFINE_ATTR(CaptureCompress);
		DEFINE_ATTR(SoftThresh);
		DEFINE_ATTR(RunScript);
		DEFINE_ATTR(ScriptFile);
		DEFINE_ATTR(StayOnTop);
		DEFINE_ATTR(ShowCursor);
		DEFINE_ATTR(LastTool);
		DEFINE_ATTR(TrackIter);
		DEFINE_ATTR(CompSizeLimit);
		DEFINE_ATTR(CompConsistent);
		DEFINE_ATTR(TryMerge);
		DEFINE_ATTR(TrySplit);
		DEFINE_ATTR(ContactFactor);
		DEFINE_ATTR(Similarity);
		DEFINE_ATTR(PointVolumeMode);
		DEFINE_ATTR(RulerUseTransf);
		DEFINE_ATTR(RulerTransient);
		DEFINE_ATTR(RulerF1);
		DEFINE_ATTR(RulerInfr);
		DEFINE_ATTR(RulerDfoverf);
		DEFINE_ATTR(RulerRelaxIter);
		DEFINE_ATTR(RulerRelax);
		DEFINE_ATTR(RulerRelaxType);
		DEFINE_ATTR(RulerSizeThresh);
		DEFINE_ATTR(PvxmlFlipX);
		DEFINE_ATTR(PvxmlFlipY);
		DEFINE_ATTR(PvxmlSeqType);
		DEFINE_ATTR(ApiType);
		DEFINE_ATTR(OutputBitR);
		DEFINE_ATTR(OutputBitG);
		DEFINE_ATTR(OutputBitB);
		DEFINE_ATTR(OutputBitA);
		DEFINE_ATTR(OutputBitD);
		DEFINE_ATTR(OutputSamples);
		DEFINE_ATTR(GlVersionMajor);
		DEFINE_ATTR(GlVersionMinor);
		DEFINE_ATTR(GlProfileMask);
		DEFINE_ATTR(NoTexPack);
		DEFINE_ATTR(ClipPlaneMode);

		SettingAgent(SettingDlg &dlg);

		virtual bool isSameKindAs(const Object* obj) const
		{
			return dynamic_cast<const SettingAgent*>(obj) != NULL;
		}

		virtual const char* className() const { return "SettingAgent"; }

		virtual void setObject(Root* obj);
		virtual Root* getObject();

		virtual void UpdateFui(const ValueCollection &names = {});

		virtual SettingAgent* asSettingAgent() { return this; }
		virtual const SettingAgent* asSettingAgent() const { return this; }

		friend class AgentFactory;

		void ReadSettings();
		void SaveSettings();
		void UpdateDeviceTree();

	protected:
		SettingDlg &dlg_;

		virtual void setupInputs();

	private:
		void OnMaxTextureSizeEnable(Event& event);
		void OnMaxTextureSize(Event& event);
		void OnFontFile(Event& event);
	};
}

#endif//_SETTINGAGENT_H_