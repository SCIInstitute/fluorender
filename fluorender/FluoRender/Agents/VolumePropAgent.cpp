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

#include <VolumePropAgent.hpp>
#include <VolumePropPanel.h>
#include <wx/valnum.h>
#include <png_resource.h>
#include <img/icons.h>

using namespace fluo;

VolumePropAgent::VolumePropAgent(VolumePropPanel &panel) :
	InterfaceAgent(),
	panel_(panel)
{
	setupInputs();
}

void VolumePropAgent::setupInputs()
{
	inputs_ = ValueCollection
	{
		//trasnfer function
		MaxInt,
		Gamma3d,
		ExtractBoundary,
		Saturation,
		LowThreshold,
		HighThreshold,
		Luminance,
		ShadowEnable,
		ShadowInt,
		AlphaEnable,
		Alpha,
		SampleRate,
		ShadingEnable,
		HighShading,
		LowShading,
		//modes
		AlphaPower,
		BlendMode,
		MipMode,
		Invert,
		LabelMode,
		Interpolate,
		SyncGroup,
		NoiseRedct,
		Legend,
		//voxel sizes
		SpcX,
		SpcY,
		SpcZ,
		//color
		Color,
		Hsv,
		SecColor,
		SecColorSet,
		//colormaps
		ColormapEnable,
		ColormapLow,
		ColormapHigh,
		ColormapMode,
		ColormapType,
		ColormapProj,
		ColormapInv
	};
}

void VolumePropAgent::setObject(VolumeData* obj)
{
	InterfaceAgent::setObject(obj);
}

VolumeData* VolumePropAgent::getObject()
{
	return dynamic_cast<VolumeData*>(InterfaceAgent::getObject());
}

void VolumePropAgent::UpdateFui(const ValueCollection &names)
{
	panel_.UpdateWindow(names);
}
