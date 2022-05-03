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

//project page
#define gstSaveProjectEnable "save project enable"	//Save Project
#define gstHardwareCompress "hardware compress"		//Compress in graphics memory
#define gstFontFile "font file"						//Font
#define gstTextSize "text size"						//Size
#define gstTextColorMode "text color mode"			//Color: 0- contrast to bg; 1-same as bg; 2-volume sec color
#define gstLineWidth "line width"					//Line Width
#define gstPaintHistory "paint history"				//Paint History
//rendering page
#define gstMicroBlendEnable "micro blend enable"	//Micro Blending
#define gstPeelNum "peel num"						//Mesh Transparency Quality
#define gstShadowDirEnable "shadow dir enable"		//Enable direcrional shadow
#define gstShadowDirX "shadow dir x"				//convert to angle in deg
#define gstShadowDirY "shadow dir y"
#define gstPinThresh "pin thresh"					//ray casting threshold value
#define gstLinkedRot "linked rot"					//link rotation to views
#define gstGradBg "gradient background"				//Gradient Background
#define gstVrEnable "vr enable"						//Enable stereo
#define gstVrEyeOffset "vr eye offset"				//Eye distance
//Performance
#define gstAdaptive "adaptive"						//Variable Sample Rate
#define gstStreamEnable "stream enable"				//Enable streaming
#define gstUpdateOrder "update order"				//Update Order: front to back (1) or back to front (0)
#define gstGpuMemSize "gpu mem size"				//Graphics Memory
#define gstLargeDataSize "large data size"			//Large Data Size
#define gstBrickSize "brick size"					//Brick Size
#define gstResponseTime "response time"				//Response Time
#define gstLodOffset "lod offset"					//Detail Level Offset
//Format
#define gstOverrideVoxSpc "override vox spc"		//Override Voxel Size
#define gstWaveColor1 "wave color1"					//Default Colors for Excitation Wavelengths 1
#define gstWaveColor2 "wave color2"					//Default Colors for Excitation Wavelengths 2
#define gstWaveColor3 "wave color3"					//Default Colors for Excitation Wavelengths 3
#define gstWaveColor4 "wave color4"					//Default Colors for Excitation Wavelengths 4
#define gstMaxTextureSizeEnable "max texture size enable"//Max Texture Size (check)
#define gstMaxTextureSize "max texture size"		//Max Texture Size
#define gstClPlatformId "cl platform id"			//OpenCL Devices (platform id)
#define gstClDeviceId "cl device id"				//OpenCL Devices (device id)
//Java
#define gstImagejMode "imagej mode"					//Package
#define gstJvmPath "jvm path"						//Path 1
#define gstImagejPath "imagej path"					//Path 2
#define gstBioformatsPath "bioformats path"			//Path 3

class SettingDlg;
namespace fluo
{
	class SettingAgent : public InterfaceAgent
	{
	public:
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

	private:
		void OnMaxTextureSizeEnable(Event& event);
		void OnMaxTextureSize(Event& event);
		void OnFontFile(Event& event);
	};
}

#endif//_SETTINGAGENT_H_