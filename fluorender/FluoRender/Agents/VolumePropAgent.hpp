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
#ifndef _VOLUMEPROPAGENT_H_
#define _VOLUMEPROPAGENT_H_

#include <InterfaceAgent.hpp>
#include <VolumeData.hpp>

//trasnfer function
#define gstGamma3d "gamma 3d"					//Gamma
#define gstExtractBoundary "extract boundary"	//Extract Boundary
#define gstSaturation "saturation"				//Saturation
#define gstLowThreshold "low threshold"			//Threshold 1
#define gstHighThreshold "high threshold"		//Threshold 2
#define gstLuminance "luminance"				//Luminance
#define gstShadowEnable "shadow enable"			//Shadow (button)
#define gstShadowInt "shadow int"				//Shadow
#define gstAlphaEnable "alpha enable"			//Alpha (button)
#define gstAlpha "alpha"						//Alpha
#define gstSampleRate "sample rate"				//Sample Rate
#define gstShadingEnable "shding enable"		//Shading (button)
#define gstHighShading "high shading"			//Shading 1
#define gstLowShading "low shading"				//Shading 2
//modes
#define gstAlphaPower "alpha power"				//High Transparency (1-normal; 2-high)
#define gstMipMode "mip mode"					//MIP
#define gstInvert "invert"						//Invert
#define gstLabelMode "label mode"				//Show Components
#define gstInterpolate "interpolate"			//Interpolation
#define gstNoiseRedct "noise redct"				//Smoothing
#define gstLegend "legend"						//Legend
//voxel sizes
#define gstSpcX "spc x"
#define gstSpcY "spc y"
#define gstSpcZ "spc z"
//color
#define gstColor "color"						//Primary Color
#define gstHsv "hsv"							//HSV of primary color
#define gstSecColor "sec color"					//Secondary Color
#define gstSecColorSet "sec color set"			//secondary color set by user, otherwise the invert of primary
//colormaps
#define gstColormapEnable "colormap enable"		//Colormap (button)
#define gstColormapLow "colormap low"			//Colormap 1
#define gstColormapHigh "colormap high"			//Colormap 2
#define gstColormapMode "colormap mode"			//1-enabled; 0-disabled
#define gstColormapType "colormap type"			//Rainbow, etc
#define gstColormapProj "colormap proj"			//Intensity, etc
#define gstColormapInv "colormap inv"			//Invert (button)

class VolumePropPanel;
namespace fluo
{
	class AgentFactory;
	class VolumePropAgent : public InterfaceAgent
	{
	public:
		VolumePropAgent(VolumePropPanel &panel);

		virtual bool isSameKindAs(const Object* obj) const
		{
			return dynamic_cast<const VolumePropAgent*>(obj) != NULL;
		}

		virtual const char* className() const { return "VolumePropAgent"; }

		virtual void setObject(VolumeData* vd);
		virtual VolumeData* getObject();

		virtual void UpdateAllSettings();

		virtual VolumePropAgent* asVolumePropAgent() { return this; }
		virtual const VolumePropAgent* asVolumePropAgent() const { return this; }

		friend class AgentFactory;

	protected:
		VolumePropPanel &panel_;

	private:
		//update functions
		void OnLuminanceChanged(Event& event);
		void OnColorChanged(Event& event);
	};
}

#endif//_VOLUMEPROPAGENT_H_