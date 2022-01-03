/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2021 Scientific Computing and Imaging Institute,
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

#include "VolumeFactory.hpp"
#include <VolumeGroup.hpp>
#include <Names.hpp>

using namespace fluo;

VolumeFactory::VolumeFactory()
{
	m_name = gstVolumeFactory;
	default_object_name_ = gstDefaultVolume;

	addRvalu(gstCurrent, (VolumeData*)(0));//current volume data

	setValueChangedFunction(gstDefaultFile,
		std::bind(&VolumeFactory::OnSetDefault,
		this, std::placeholders::_1));
}

VolumeFactory::~VolumeFactory()
{

}

void VolumeFactory::createDefault()
{
	if (!getDefault())
	{
		VolumeData* vd = new VolumeData();
		vd->setName(default_object_name_);
		objects_.push_front(vd);

		//add default values here
		//output (2d) adjustments
		vd->addValue(gstGammaR, double(1));
		vd->addValue(gstGammaG, double(1));
		vd->addValue(gstGammaB, double(1));
		vd->addValue(gstBrightnessR, double(1));
		vd->addValue(gstBrightnessG, double(1));
		vd->addValue(gstBrightnessB, double(1));
		vd->addValue(gstEqualizeR, double(0));
		vd->addValue(gstEqualizeG, double(0));
		vd->addValue(gstEqualizeB, double(0));
		vd->addValue(gstSyncR, bool(false));
		vd->addValue(gstSyncG, bool(false));
		vd->addValue(gstSyncB, bool(false));

		vd->addValue(gstBounds, BBox());
		//clipping planes
		vd->addValue(gstClipPlanes, PlaneSet(6));
		vd->addValue(gstClipBounds, BBox());
		//save clip values individually
		//actual clipping planes are calculated after either
		//clip values or rotations are changed
		vd->addValue(gstClipX1, double(0));
		vd->addValue(gstClipX2, double(1));
		vd->addValue(gstClipY1, double(0));
		vd->addValue(gstClipY2, double(1));
		vd->addValue(gstClipZ1, double(0));
		vd->addValue(gstClipZ2, double(1));
		//clip distance (normalized)
		vd->addValue(gstClipDistX, double(1));
		vd->addValue(gstClipDistY, double(1));
		vd->addValue(gstClipDistZ, double(1));
		//clip link
		vd->addValue(gstClipLinkX, bool(false));
		vd->addValue(gstClipLinkY, bool(false));
		vd->addValue(gstClipLinkZ, bool(false));
		//clip rotation
		vd->addValue(gstClipRotX, double(0));
		vd->addValue(gstClipRotY, double(0));
		vd->addValue(gstClipRotZ, double(0));
		//clip plane rendering
		vd->addValue(gstClipDisplay, bool(false));
		vd->addValue(gstClipHold, bool(false));
		vd->addValue(gstClipMask, long(-1));
		vd->addValue(gstClipRenderMode, long(PRMNormal));

		vd->addValue(gstDataPath, std::wstring());//path to original file
		vd->addValue(gstChannel, long(0));//channel index of the original file
		vd->addValue(gstTime, long(0));//time index of the original file

		//modes
		//blend mode
		vd->addValue(gstBlendMode, long(0));//0: ignore; 1: layered; 2: depth; 3: composite
		//mip
		vd->addValue(gstMipMode, long(0));//0-normal; 1-MIP
		vd->addValue(gstOverlayMode, long(0));//0-unset; 1-base layer; 2-white; 3-white mip
		vd->addValue(gstStreamMode, long(0));//0-normal; 1-MIP; 2-shading; 3-shadow, 4-mask
		vd->addValue(gstMaskMode, long(0));//0-normal, 1-render with mask, 2-render with mask excluded,
											//3-random color with label, 4-random color with label+mask
		vd->addValue(gstUseMaskThresh, bool(false));// use mask threshold
		vd->addValue(gstMaskThresh, double(0));//mask threshold
		vd->addValue(gstInvert, bool(false));//invert intensity values

		//volume properties
		vd->addValue(gstSoftThresh, double(0));//soft threshold
		vd->addValue(gstIntScale, double(1));//scaling factor for intensity values
		vd->addValue(gstGmScale, double(1));//scaling factor for gradient magnitude values
		vd->addValue(gstMaxInt, double(255));//max intensity value from integer reading
		vd->addValue(gstMaxScale, double(255));//max intensity based on bits
		vd->addValue(gstGamma3d, double(1));
		vd->addValue(gstExtractBoundary, double(0));//previously called gm thresh
		vd->addValue(gstSaturation, double(1));//previously called offset, low offset
		vd->addValue(gstLowThreshold, double(0));
		vd->addValue(gstHighThreshold, double(1));
		vd->addValue(gstAlpha, double(1));
		vd->addValue(gstAlphaPower, double(1));
		vd->addValue(gstAlphaEnable, bool(true));
		vd->addValue(gstMatAmb, double(1));//materials
		vd->addValue(gstMatDiff, double(1));
		vd->addValue(gstMatSpec, double(1));
		vd->addValue(gstMatShine, double(10));
		vd->addValue(gstNoiseRedct, bool(false));//noise reduction
		vd->addValue(gstShadingEnable, bool(false));//shading
		vd->addValue(gstLowShading, double(1));//low shading
		vd->addValue(gstHighShading, double(10));//highg shading
		vd->addValue(gstShadowEnable, bool(false));//shadow
		vd->addValue(gstShadowInt, double(1));
		vd->addValue(gstSampleRate, double(1));//sample rate
		//color
		vd->addValue(gstColor, Color(1.0));
		vd->addValue(gstHsv, HSVColor(Color(1.0)));
		vd->addValue(gstLuminance, double(1.0));
		vd->addValue(gstSecColor, Color(1.0));//secondary color
		vd->addValue(gstSecColorSet, bool(false));
		vd->addValue(gstRandomizeColor, bool(false));//set to change color
		//vd->addValue("shuffle", long(0));//comp id shuffle

		//resolution
		vd->addValue(gstResX, long(0));
		vd->addValue(gstResY, long(0));
		vd->addValue(gstResZ, long(0));
		vd->addValue(gstBaseResX, long(0));
		vd->addValue(gstBaseResY, long(0));
		vd->addValue(gstBaseResZ, long(0));
		//scaling
		vd->addValue(gstScaleX, double(1));
		vd->addValue(gstScaleY, double(1));
		vd->addValue(gstScaleZ, double(1));
		//spacing
		vd->addValue(gstSpcX, double(1));
		vd->addValue(gstSpcY, double(1));
		vd->addValue(gstSpcZ, double(3));
		vd->addValue(gstSpcFromFile, bool(false));//if spacing value are from original file, otherwise use default settings
		//added for multires data
		vd->addValue(gstBaseSpcX, double(1));
		vd->addValue(gstBaseSpcY, double(1));
		vd->addValue(gstBaseSpcZ, double(3));
		vd->addValue(gstSpcSclX, double(1));
		vd->addValue(gstSpcSclY, double(1));
		vd->addValue(gstSpcSclZ, double(1));

		//display control
		vd->addValue(gstBits, long(8));//bits
		vd->addValue(gstDisplay, bool(true));
		vd->addValue(gstDrawBounds, bool(false));
		vd->addValue(gstTestWire, bool(false));

		//color map settings
		vd->addValue(gstColormapEnable, bool(false));
		vd->addValue(gstColormapMode, long(0));
		vd->addValue(gstColormapType, long(0));
		vd->addValue(gstColormapLow, double(0));
		vd->addValue(gstColormapHigh, double(1));
		vd->addValue(gstColormapProj, long(0));
		vd->addValue(gstColormapInv, double(1.0));

		//texture ids will be removed later
		vd->addValue(gst2dMaskId, (unsigned long)(0));//2d mask texture for segmentation
		//2d weight map for segmentation
		vd->addValue(gst2dWeight1Id, (unsigned long)(0));//after tone mapping
		vd->addValue(gst2dWeight2Id, (unsigned long)(0));//before tone mapping
		vd->addValue(gst2dDmapId, (unsigned long)(0));//2d depth map texture for rendering shadows

		//compression
		vd->addValue(gstCompression, bool(false));

		//resize
		vd->addValue(gstResize, bool(false));
		vd->addValue(gstResizeX, long(0));
		vd->addValue(gstResizeY, long(0));
		vd->addValue(gstResizeZ, long(0));

		//brisk skipping
		vd->addValue(gstSkipBrick, bool(false));
		//valid brick number
		vd->addValue(gstBrickNum, long(0));

		//shown in legend
		vd->addValue(gstLegend, bool(true));

		//interpolate
		vd->addValue(gstInterpolate, bool(true));

		//label mode
		vd->addValue(gstLabelMode, long(1));

		//depth attenuation, also called fog previously
		vd->addValue(gstDepthAtten, bool(false));
		vd->addValue(gstDaInt, double(0.5));
		vd->addValue(gstDaStart, double(0));
		vd->addValue(gstDaEnd, double(1));

		//estimate threshold
		vd->addValue(gstEstimateThresh, double(0));

		//parameters not in original class but passed to renderer
		vd->addValue(gstViewport, Vector4i());//viewport
		vd->addValue(gstClearColor, Vector4f());//clear color
		vd->addValue(gstCurFramebuffer, (unsigned long)(0));//current framebuffer

		//multires level
		vd->addValue(gstMultires, bool(false));
		vd->addValue(gstLevel, long(0));
		vd->addValue(gstLevelNum, long(1));

		//tex transform
		vd->addValue(gstTexTransform, Transform());

		//selected on the ui
		vd->addValue(gstSelected, bool(false));

		//mask cleared
		vd->addValue(gstMaskClear, bool(false));

		//sync group
		vd->addValue(gstSyncGroup, bool(false));

		//duplicate
		vd->addValue(gstDuplicate, bool(false));
	}
}

#define ADD_BEFORE_EVENT(obj, name, funct) \
	obj->setValueChangingFunction(name, std::bind(&VolumeData::funct, obj, std::placeholders::_1))

#define ADD_AFTER_EVENT(obj, name, funct) \
	obj->setValueChangedFunction(name, std::bind(&VolumeData::funct, obj, std::placeholders::_1))

void VolumeFactory::setEventHandler(VolumeData* vd)
{
	//handle before events
	//ADD_BEFORE_EVENT(vd, gstMipMode, OnMipModeChanging);

	//handle after events
	ADD_AFTER_EVENT(vd, gstMipMode, OnMipModeChanged);
	ADD_AFTER_EVENT(vd, gstOverlayMode, OnOverlayModeChanged);
	ADD_AFTER_EVENT(vd, gstViewport, OnViewportChanged);
	ADD_AFTER_EVENT(vd, gstClearColor, OnClearColorChanged);
	ADD_AFTER_EVENT(vd, gstCurFramebuffer, OnCurFramebufferChanged);
	ADD_AFTER_EVENT(vd, gstCompression, OnCompressionChanged);
	ADD_AFTER_EVENT(vd, gstInvert, OnInvertChanged);
	ADD_AFTER_EVENT(vd, gstMaskMode, OnMaskModeChanged);
	ADD_AFTER_EVENT(vd, gstNoiseRedct, OnNoiseRedctChanged);
	ADD_AFTER_EVENT(vd, gst2dDmapId, On2dDmapIdChanged);
	ADD_AFTER_EVENT(vd, gstGamma3d, OnGamma3dChanged);
	ADD_AFTER_EVENT(vd, gstExtractBoundary, OnExtractBoundaryChanged);
	ADD_AFTER_EVENT(vd, gstSaturation, OnSaturationChanged);
	ADD_AFTER_EVENT(vd, gstLowThreshold, OnLowThresholdChanged);
	ADD_AFTER_EVENT(vd, gstHighThreshold, OnHighThresholdChanged);
	ADD_AFTER_EVENT(vd, gstColor, OnColorChanged);
	ADD_AFTER_EVENT(vd, gstSecColor, OnSecColorChanged);
	ADD_AFTER_EVENT(vd, gstSecColorSet, OnSecColorSetChanged);
	ADD_AFTER_EVENT(vd, gstLuminance, OnLuminanceChanged);
	ADD_AFTER_EVENT(vd, gstAlpha, OnAlphaChanged);
	ADD_AFTER_EVENT(vd, gstAlphaPower, OnAlphaPowerChanged);
	ADD_AFTER_EVENT(vd, gstAlphaEnable, OnAlphaEnableChanged);
	ADD_AFTER_EVENT(vd, gstMaskThresh, OnMaskThreshChanged);
	ADD_AFTER_EVENT(vd, gstUseMaskThresh, OnUseMaskThreshChanged);
	ADD_AFTER_EVENT(vd, gstShadingEnable, OnShadingEnableChanged);
	ADD_AFTER_EVENT(vd, gstMatAmb, OnMaterialChanged);
	ADD_AFTER_EVENT(vd, gstLowShading, OnLowShadingChanged);
	ADD_AFTER_EVENT(vd, gstMatDiff, OnMaterialChanged);
	ADD_AFTER_EVENT(vd, gstMatSpec, OnMaterialChanged);
	ADD_AFTER_EVENT(vd, gstMatShine, OnMaterialChanged);
	ADD_AFTER_EVENT(vd, gstHighShading, OnHighShadingChanged);
	ADD_AFTER_EVENT(vd, gstSampleRate, OnSampleRateChanged);
	ADD_AFTER_EVENT(vd, gstColormapMode, OnColormapModeChanged);
	ADD_AFTER_EVENT(vd, gstColormapLow, OnColormapValueChanged);
	ADD_AFTER_EVENT(vd, gstColormapHigh, OnColormapValueChanged);
	ADD_AFTER_EVENT(vd, gstColormapType, OnColormapTypeChanged);
	ADD_AFTER_EVENT(vd, gstColormapProj, OnColormapProjChanged);
	ADD_AFTER_EVENT(vd, gstSpcX, OnSpacingChanged);
	ADD_AFTER_EVENT(vd, gstSpcY, OnSpacingChanged);
	ADD_AFTER_EVENT(vd, gstSpcZ, OnSpacingChanged);
	ADD_AFTER_EVENT(vd, gstBaseSpcX, OnBaseSpacingChanged);
	ADD_AFTER_EVENT(vd, gstBaseSpcY, OnBaseSpacingChanged);
	ADD_AFTER_EVENT(vd, gstBaseSpcZ, OnBaseSpacingChanged);
	ADD_AFTER_EVENT(vd, gstSpcSclX, OnSpacingScaleChanged);
	ADD_AFTER_EVENT(vd, gstSpcSclY, OnSpacingScaleChanged);
	ADD_AFTER_EVENT(vd, gstSpcSclZ, OnSpacingScaleChanged);
	ADD_AFTER_EVENT(vd, gstLevel, OnLevelChanged);
	ADD_AFTER_EVENT(vd, gstDisplay, OnDisplayChanged);
	ADD_AFTER_EVENT(vd, gstInterpolate, OnInterpolateChanged);
	ADD_AFTER_EVENT(vd, gstDepthAtten, OnDepthAttenChanged);
	ADD_AFTER_EVENT(vd, gstSkipBrick, OnSkipBrickChanged);
	ADD_AFTER_EVENT(vd, gstClipPlanes, OnClipPlanesChanged);
	ADD_AFTER_EVENT(vd, gstSyncR, OnSyncOutputChannels);
	ADD_AFTER_EVENT(vd, gstSyncG, OnSyncOutputChannels);
	ADD_AFTER_EVENT(vd, gstSyncB, OnSyncOutputChannels);
	ADD_AFTER_EVENT(vd, gstClipX1, OnClipX1Changed);
	ADD_AFTER_EVENT(vd, gstClipX2, OnClipX2Changed);
	ADD_AFTER_EVENT(vd, gstClipY1, OnClipY1Changed);
	ADD_AFTER_EVENT(vd, gstClipY2, OnClipY2Changed);
	ADD_AFTER_EVENT(vd, gstClipZ1, OnClipZ1Changed);
	ADD_AFTER_EVENT(vd, gstClipZ2, OnClipZ2Changed);
	ADD_AFTER_EVENT(vd, gstClipRotX, OnClipRot);
	ADD_AFTER_EVENT(vd, gstClipRotY, OnClipRot);
	ADD_AFTER_EVENT(vd, gstClipRotZ, OnClipRot);
	ADD_AFTER_EVENT(vd, gstRandomizeColor, OnRandomizeColor);
}

VolumeData* VolumeFactory::build(VolumeData* vd)
{
	unsigned int default_id = 0;
	return clone(default_id);
}

VolumeData* VolumeFactory::clone(VolumeData* vd)
{
	if (!vd)
		return 0;

	incCounter();

	VolumeData* new_vd = dynamic_cast<VolumeData*>(
		vd->clone(CopyOp::DEEP_COPY_ALL));
	if (!new_vd)
		return 0;
	new_vd->setId(global_id_);
	std::string name = "volume" + std::to_string(local_id_);
	new_vd->setName(name);

	objects_.push_front(new_vd);

	setRvalu(gstCurrent, new_vd);
	setEventHandler(new_vd);

	//notify observers
	Event event;
	event.init(Event::EVENT_NODE_ADDED,
		this, new_vd);
	notifyObservers(event);

	return new_vd;
}

VolumeData* VolumeFactory::clone(const unsigned int id)
{
	Object* object = find(id);
	//std::cout << object->getId() << " " << object->getName() << std::endl;
	if (object)
	{
		VolumeData* vd = dynamic_cast<VolumeData*>(object);
		if (vd)
			return clone(vd);
	}
	return 0;
}

VolumeGroup* VolumeFactory::buildGroup(VolumeData* vd)
{
	VolumeGroup* group;
	if (vd)
		group = new VolumeGroup(*vd, CopyOp::DEEP_COPY_ALL);
	else
		group = new VolumeGroup(*getDefault(), CopyOp::DEEP_COPY_ALL);
	if (group)
	{
		group->setName("Group");
		group->setValueChangedFunction(
			gstRandomizeColor, std::bind(
				&VolumeGroup::OnRandomizeColor,
				group, std::placeholders::_1));
	}
	return group;
}

void VolumeFactory::OnSetDefault(Event& event)
{
	if (!readDefault())
		createDefault();
}
