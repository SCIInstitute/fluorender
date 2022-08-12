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

#include "VolumePropAgent.hpp"
#include "AgentFactory.hpp"
//#include "VolumePropPanel.h"
#include "VolumeFactory.hpp"
#include "icons.h"

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

void VolumePropAgent::SaveDefault()
{
	fluo::ValueCollection names{
		fluo::VolumePropAgent::Gamma3d,
		fluo::VolumePropAgent::ExtractBoundary,
		fluo::VolumePropAgent::Saturation,
		fluo::VolumePropAgent::LowThreshold,
		fluo::VolumePropAgent::HighThreshold,
		fluo::VolumePropAgent::ShadowEnable,
		fluo::VolumePropAgent::ShadowInt,
		fluo::VolumePropAgent::Alpha,
		fluo::VolumePropAgent::AlphaEnable,
		fluo::VolumePropAgent::SampleRate,
		fluo::VolumePropAgent::ShadingEnable,
		fluo::VolumePropAgent::LowShading,
		fluo::VolumePropAgent::HighShading,
		fluo::VolumePropAgent::ColormapEnable,
		fluo::VolumePropAgent::ColormapMode,
		fluo::VolumePropAgent::ColormapType,
		fluo::VolumePropAgent::ColormapLow,
		fluo::VolumePropAgent::ColormapHigh,
		fluo::VolumePropAgent::ColormapProj,
		fluo::VolumePropAgent::Invert,
		fluo::VolumePropAgent::Interpolate,
		fluo::VolumePropAgent::MipMode,
		fluo::VolumePropAgent::NoiseRedct,
		fluo::VolumePropAgent::SpcX,
		fluo::VolumePropAgent::SpcY,
		fluo::VolumePropAgent::SpcZ
	};
	glbin_volf->propValuesToDefault(this, names);
	glbin_volf->writeDefault(names);
}

void VolumePropAgent::ResetDefault()
{
	fluo::ValueCollection names{
		fluo::VolumePropAgent::Gamma3d,
		fluo::VolumePropAgent::ExtractBoundary,
		fluo::VolumePropAgent::Saturation,
		fluo::VolumePropAgent::LowThreshold,
		fluo::VolumePropAgent::HighThreshold,
		fluo::VolumePropAgent::ShadowEnable,
		fluo::VolumePropAgent::ShadowInt,
		fluo::VolumePropAgent::Alpha,
		fluo::VolumePropAgent::AlphaEnable,
		fluo::VolumePropAgent::SampleRate,
		fluo::VolumePropAgent::ShadingEnable,
		fluo::VolumePropAgent::LowShading,
		fluo::VolumePropAgent::HighShading,
		fluo::VolumePropAgent::ColormapEnable,
		fluo::VolumePropAgent::ColormapMode,
		fluo::VolumePropAgent::ColormapType,
		fluo::VolumePropAgent::ColormapLow,
		fluo::VolumePropAgent::ColormapHigh,
		fluo::VolumePropAgent::ColormapProj,
		fluo::VolumePropAgent::Invert,
		fluo::VolumePropAgent::Interpolate,
		fluo::VolumePropAgent::MipMode,
		fluo::VolumePropAgent::NoiseRedct
	};
	glbin_volf->propValuesFromDefault(this, names);
}
