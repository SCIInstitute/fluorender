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

class VolumePropPanel;
namespace fluo
{
	class AgentFactory;
	class VolumePropAgent : public InterfaceAgent
	{
	public:
		//trasnfer function
		DEFINE_ATTR(MaxInt);						//Max intensity value for range settings
		DEFINE_ATTR(Gamma3d);						//Gamma
		DEFINE_ATTR(ExtractBoundary);				//Extract Boundary
		DEFINE_ATTR(Saturation);					//Saturation
		DEFINE_ATTR(LowThreshold);					//Threshold 1
		DEFINE_ATTR(HighThreshold);					//Threshold 2
		DEFINE_ATTR(Luminance);						//Luminance
		DEFINE_ATTR(ShadowEnable);					//Shadow (button)
		DEFINE_ATTR(ShadowInt);						//Shadow
		DEFINE_ATTR(AlphaEnable);					//Alpha (button)
		DEFINE_ATTR(Alpha);							//Alpha
		DEFINE_ATTR(SampleRate);					//Sample Rate
		DEFINE_ATTR(ShadingEnable);					//Shading (button)
		DEFINE_ATTR(HighShading);					//Shading 1
		DEFINE_ATTR(LowShading);					//Shading 2
		//modes
		DEFINE_ATTR(AlphaPower);					//High Transparency (1-normal; 2-high)
		DEFINE_ATTR(BlendMode);						//blend mode for group
		DEFINE_ATTR(MipMode);						//MIP
		DEFINE_ATTR(Invert);						//Invert
		DEFINE_ATTR(LabelMode);						//Show Components
		DEFINE_ATTR(Interpolate);					//Interpolation
		DEFINE_ATTR(SyncGroup);						//synchronize settings for group
		DEFINE_ATTR(NoiseRedct);					//Smoothing
		DEFINE_ATTR(Legend);						//Legend
		//voxel sizes
		DEFINE_ATTR(SpcX);							//space setting x
		DEFINE_ATTR(SpcY);							//space setting y
		DEFINE_ATTR(SpcZ);							//space setting z
		//color										
		DEFINE_ATTR(Color);							//Primary Color
		DEFINE_ATTR(Hsv);							//HSV of primary color
		DEFINE_ATTR(SecColor);						//Secondary Color
		DEFINE_ATTR(SecColorSet);					//secondary color set by user, otherwise the invert of primary
		//colormaps									
		DEFINE_ATTR(ColormapEnable);				//Colormap (button)
		DEFINE_ATTR(ColormapLow);					//Colormap 1
		DEFINE_ATTR(ColormapHigh);					//Colormap 2
		DEFINE_ATTR(ColormapMode);					//1-enabled; 0-disabled
		DEFINE_ATTR(ColormapType);					//Rainbow, etc
		DEFINE_ATTR(ColormapProj);					//Intensity, etc
		DEFINE_ATTR(ColormapInv);					//Invert (button)

		//other attributes
		DEFINE_ATTR(Bits);
		DEFINE_ATTR(IntScale);
		DEFINE_ATTR(BaseSpcX);
		DEFINE_ATTR(BaseSpcY);
		DEFINE_ATTR(BaseSpcZ);

		VolumePropAgent(VolumePropPanel &panel);

		virtual bool isSameKindAs(const Object* obj) const
		{
			return dynamic_cast<const VolumePropAgent*>(obj) != NULL;
		}

		virtual const char* className() const { return "VolumePropAgent"; }

		virtual void setObject(VolumeData* vd);
		virtual VolumeData* getObject();

		virtual void UpdateFui(const ValueCollection &names = {});

		virtual VolumePropAgent* asVolumePropAgent() { return this; }
		virtual const VolumePropAgent* asVolumePropAgent() const { return this; }

		friend class AgentFactory;

		void SaveDefault();
		void ResetDefault();

	protected:
		VolumePropPanel &panel_;

		virtual void setupInputs();

	private:
		//update functions
	};
}

#endif//_VOLUMEPROPAGENT_H_