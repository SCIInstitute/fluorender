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

//#include "VolumeSampler.h"

#include "VolumeData.hpp"
#include "VolumeBaker.h"
#include "VolumeSampler.h"
#include "Global.hpp"
#include "VolumeFactory.hpp"
#include "Plane.h"
#include "Quaternion.h"
#include "VolumeRenderer.h"
#include "base_reader.h"
#include "oib_reader.h"
#include "nrrd_reader.h"
#include "tif_reader.h"
#include "nrrd_writer.h"
#include "tif_writer.h"
#include "msk_reader.h"
#include "msk_writer.h"
#include "lsm_reader.h"
#include "lbl_reader.h"
#include "pvxml_reader.h"
#include "brkxml_reader.h"
//#include "imageJ_reader.hpp"

#include <glm/gtc/matrix_transform.hpp>

using namespace fluo;

VolumeData::VolumeData() :
	m_vr(0),
	m_tex(0),
	m_reader(0),
	m_label_save(0)
{
}

VolumeData::VolumeData(const VolumeData& data, const CopyOp& copyop, bool copy_values):
	Node(data, copyop, false),
	m_vr(0),
	m_tex(0),
	m_reader(0),
	m_label_save(0)
{
	//volume renderer and texture
	//if (data.m_vr)
	//	m_vr = new flvr::VolumeRenderer(*data.m_vr);
	//else
	//	m_vr = new flvr::VolumeRenderer();
	//m_tex = data.m_tex;
	if (copy_values)
		copyValues(data, copyop);
}

VolumeData::~VolumeData()
{
	if (m_vr && referenceCount() == 1)//if it's the last one
		delete m_vr;
	if (m_tex && referenceCount() == 1)
		delete m_tex;
	if (m_label_save && referenceCount() == 1)
		delete[] m_label_save;
}

void VolumeData::Initialize()
{
	Event event;
	OnMipModeChanged(event);//modes
	OnViewportChanged(event);
	OnClearColorChanged(event);
	OnCurFramebufferChanged(event);
	OnCompressionChanged(event);
	OnInvertChanged(event);
	OnMaskModeChanged(event);
	OnNoiseRedctChanged(event);
	On2dDmapIdChanged(event);
	OnGamma3dChanged(event);
	OnExtractBoundaryChanged(event);
	OnSaturationChanged(event);
	OnLowThresholdChanged(event);
	OnHighThresholdChanged(event);
	OnColorChanged(event);
	OnSecColorChanged(event);
	OnSecColorSetChanged(event);
	//OnLuminanceChanged(event);
	OnAlphaChanged(event);
	OnAlphaEnableChanged(event);
	OnMaskThreshChanged(event);
	OnUseMaskThreshChanged(event);
	OnShadingEnableChanged(event);
	OnMaterialChanged(event);
	OnSampleRateChanged(event);
	OnColormapModeChanged(event);
	OnColormapValueChanged(event);
	OnColormapTypeChanged(event);
	OnColormapProjChanged(event);
	OnSpacingChanged(event);
	//OnBaseSpacingChanged(event);
	//OnSpacingScaleChanged(event);
	OnLevelChanged(event);
	OnDisplayChanged(event);
	OnInterpolateChanged(event);
	OnDepthAttenChanged(event);
	OnSkipBrickChanged(event);
	OnClipPlanesChanged(event);
	OnIntScaleChanged(event);
}

//void VolumeData::OnMipModeChanging(Event& event)
//{
//	//save mode
//	long mode;
//	getValue(gstMipMode, mode);
//	setValue("saved mode", mode);
//}

//MIP & normal modes
void VolumeData::OnMipModeChanged(Event& event)
{
	if (!m_vr)
		return;

	long mode;
	getValue(gstMipMode, mode);

	switch (mode)
	{
	case 0://normal
		m_vr->set_mode(flvr::TextureRenderer::MODE_OVER);
		break;
	case 1://MIP
		m_vr->set_mode(flvr::TextureRenderer::MODE_MIP);
		break;
	}
}

void VolumeData::OnOverlayModeChanged(Event& event)
{
	if (!m_vr)
		return;

	long mode;
	getValue(gstOverlayMode, mode);

	switch (mode)
	{
	case 0://restore
	{
		Color color;
		getValue(gstColor, color);
		m_vr->set_color(color);
		double alpha;
		getValue(gstAlpha, alpha);
		m_vr->set_alpha(alpha);
		bool shading_enable;
		getValue(gstShadingEnable, shading_enable);
		m_vr->set_shading(shading_enable);
		bool colormap_enable;
		getValue(gstColormapEnable, colormap_enable);
		m_vr->set_colormap_mode(colormap_enable?1:0);
		long mask_mode;
		getValue(gstMaskMode, mask_mode);
		m_vr->set_ml_mode(mask_mode);
		long mip_mode;
		getValue(gstMipMode, mip_mode);
		if (mip_mode == 1)
			m_vr->set_mode(flvr::TextureRenderer::MODE_MIP);
		else
			m_vr->set_mode(flvr::TextureRenderer::MODE_OVER);
		bool alpha_enable;
		getValue(gstAlphaEnable, alpha_enable);
		m_vr->set_solid(!alpha_enable);
	}
	break;
	case 1://for shadow
	{
		m_vr->set_mode(flvr::TextureRenderer::MODE_OVER);
		m_vr->set_shading(false);
		m_vr->set_solid(false);
		m_vr->set_colormap_mode(2);
		m_vr->set_ml_mode(0);
	}
	break;
	case 2://for shading
	{
		m_vr->set_mode(flvr::TextureRenderer::MODE_OVER);
		m_vr->set_shading(true);
		m_vr->set_solid(false);
		m_vr->set_alpha(1.0);
		m_vr->set_colormap_mode(0);
		m_vr->set_color(Color(1.0));
	}
	break;
	case 3://white mip
	{
		bool colormap_enable;
		getValue(gstColormapEnable, colormap_enable);
		if (colormap_enable)
		{
			m_vr->set_mode(flvr::TextureRenderer::MODE_MIP);
			m_vr->set_colormap_mode(0);
			m_vr->set_color(Color(1.0));
			//m_vr->set_alpha(1.0);
			m_vr->set_solid(true);
		}
	}
	}
}

void VolumeData::OnViewportChanged(Event& event)
{
	if (!m_vr)
		return;

	Vector4i vp;
	getValue(gstViewport, vp);

	m_vr->set_viewport(vp.get());
}

void VolumeData::OnClearColorChanged(Event& event)
{
	if (!m_vr)
		return;

	Vector4f clear_color;
	getValue(gstClearColor, clear_color);
	m_vr->set_clear_color(clear_color.get());
}

void VolumeData::OnCurFramebufferChanged(Event& event)
{
	if (!m_vr)
		return;

	unsigned long cur_framebuffer;
	getValue(gstCurFramebuffer, cur_framebuffer);
	m_vr->set_cur_framebuffer(cur_framebuffer);
}

void VolumeData::OnCompressionChanged(Event& event)
{
	if (!m_vr)
		return;

	bool compression;
	getValue(gstHardwareCompress, compression);
	m_vr->set_compression(compression);
}

void VolumeData::OnInvertChanged(Event& event)
{
	if (!m_vr)
		return;

	bool invert;
	getValue(gstInvert, invert);
	m_vr->set_inversion(invert);
}

void VolumeData::OnMaskModeChanged(Event& event)
{
	if (!m_vr)
		return;

	long mask_mode;
	getValue(gstMaskMode, mask_mode);
	m_vr->set_ml_mode(mask_mode);
}

void VolumeData::OnNoiseRedctChanged(Event& event)
{
	if (!m_vr)
		return;

	bool noise_redct;
	getValue(gstNoiseRedct, noise_redct);
	m_vr->SetNoiseRed(noise_redct);
}

void VolumeData::On2dDmapIdChanged(Event& event)
{
	if (!m_vr)
		return;

	unsigned long dmap_id;
	getValue(gst2dDmapId, dmap_id);
	m_vr->set_2d_dmap(dmap_id);
}

void VolumeData::OnGamma3dChanged(Event& event)
{
	if (!m_vr)
		return;

	double gamma_3d;
	getValue(gstGamma3d, gamma_3d);
	m_vr->set_gamma3d(gamma_3d);
}

void VolumeData::OnExtractBoundaryChanged(Event& event)
{
	if (!m_vr)
		return;

	double extract_boundary;
	getValue(gstExtractBoundary, extract_boundary);
	m_vr->set_gm_thresh(extract_boundary);
}

void VolumeData::OnSaturationChanged(Event& event)
{
	if (!m_vr)
		return;

	double saturation;
	getValue(gstSaturation, saturation);
	m_vr->set_offset(saturation);
}

void VolumeData::OnLowThresholdChanged(Event& event)
{
	if (!m_vr)
		return;

	double low_threshold;
	getValue(gstLowThreshold, low_threshold);
	m_vr->set_lo_thresh(low_threshold);
}

void VolumeData::OnHighThresholdChanged(Event& event)
{
	if (!m_vr)
		return;

	double high_threshold;
	getValue(gstHighThreshold, high_threshold);
	m_vr->set_hi_thresh(high_threshold);
}

void VolumeData::OnColorChanged(Event& event)
{
	if (!m_vr)
		return;
	Color color;
	getValue(gstColor, color);
	m_vr->set_color(color);
	HSVColor hsv(color);
	event.setNotifyFlags(Event::NOTIFY_SELF);
	updateValue(gstHsv, hsv, event);
	event.setNotifyFlags(Event::NOTIFY_SELF | Event::NOTIFY_AGENT);
	updateValue(gstLuminance, hsv.val(), event);
}

void VolumeData::OnSecColorChanged(Event& event)
{
	if (!m_vr)
		return;

	Color sec_color;
	getValue(gstSecColor, sec_color);
	m_vr->set_mask_color(sec_color);
	event.setNotifyFlags(Event::NOTIFY_SELF);
	updateValue(gstSecColorSet, bool(true), event);
}

void VolumeData::OnSecColorSetChanged(Event& event)
{
	if (!m_vr)
		return;

	bool sec_color_set;
	getValue(gstSecColorSet, sec_color_set);
	if (!sec_color_set)
		m_vr->reset_mask_color_set();
}

void VolumeData::OnLuminanceChanged(Event& event)
{
	if (!m_vr)
		return;

	double luminance;
	getValue(gstLuminance, luminance);
	HSVColor hsv;
	getValue(gstHsv, hsv);
	hsv.val(luminance);
	Color color(hsv);
	event.setNotifyFlags(Event::NOTIFY_SELF);
	updateValue(gstColor, color, event);
	m_vr->set_color(color);
}

void VolumeData::OnAlphaChanged(Event& event)
{
	if (!m_vr)
		return;

	double alpha;
	getValue(gstAlpha, alpha);
	m_vr->set_alpha(alpha);
}

void VolumeData::OnAlphaPowerChanged(Event& event)
{
	if (!m_vr)
		return;

	double dval;
	getValue(gstAlphaPower, dval);
	m_vr->set_alpha_power(dval);
}

void VolumeData::OnAlphaEnableChanged(Event& event)
{
	if (!m_vr)
		return;

	bool alpha_enable;
	getValue(gstAlphaEnable, alpha_enable);
	m_vr->set_solid(!alpha_enable);
	if (alpha_enable)
	{
		double alpha;
		getValue(gstAlpha, alpha);
		m_vr->set_alpha(alpha);
	}
	else
		m_vr->set_alpha(1.0);
}

void VolumeData::OnMaskThreshChanged(Event& event)
{
	if (!m_vr)
		return;

	double mask_thresh;
	getValue(gstMaskThresh, mask_thresh);
	m_vr->set_mask_thresh(mask_thresh);
}

void VolumeData::OnUseMaskThreshChanged(Event& event)
{
	if (!m_vr)
		return;

	bool use_mask_thresh;
	getValue(gstUseMaskThresh, use_mask_thresh);
	if (!use_mask_thresh)
		m_vr->set_mask_thresh(0.0);
}

void VolumeData::OnShadingEnableChanged(Event& event)
{
	if (!m_vr)
		return;

	bool shading_enable;
	getValue(gstShadingEnable, shading_enable);
	m_vr->set_shading(shading_enable);
}

void VolumeData::OnMaterialChanged(Event& event)
{
	if (!m_vr)
		return;

	double mat_amb, mat_diff, mat_spec, mat_shine;
	getValue(gstMatAmb, mat_amb);
	getValue(gstMatDiff, mat_diff);
	getValue(gstMatSpec, mat_spec);
	getValue(gstMatShine, mat_shine);
	m_vr->set_material(mat_amb, mat_diff, mat_spec, mat_shine);
}

void VolumeData::OnLowShadingChanged(Event& event)
{
	double low_shading;
	getValue(gstLowShading, low_shading);
	event.setNotifyFlags(Event::NOTIFY_SELF);
	updateValue(gstMatAmb, low_shading, event);
}

void VolumeData::OnHighShadingChanged(Event& event)
{
	double high_shading;
	getValue(gstHighShading, high_shading);
	event.setNotifyFlags(Event::NOTIFY_SELF);
	updateValue(gstMatShine, high_shading, event);
}

void VolumeData::OnSampleRateChanged(Event& event)
{
	if (!m_vr)
		return;

	double sample_rate;
	getValue(gstSampleRate, sample_rate);
	m_vr->set_sampling_rate(sample_rate);
}

void VolumeData::OnColormapModeChanged(Event& event)
{
	if (!m_vr)
		return;

	long colormap_mode;
	getValue(gstColormapMode, colormap_mode);
	Color color;
	getValue(gstColor, color);
	m_vr->set_colormap_mode(colormap_mode);
	m_vr->set_color(color);
}

void VolumeData::OnColormapValueChanged(Event& event)
{
	if (!m_vr)
		return;

	double colormap_low, colormap_high;
	getValue(gstColormapLow, colormap_low);
	getValue(gstColormapHigh, colormap_high);
	m_vr->set_colormap_values(colormap_low, colormap_high);
}

void VolumeData::OnColormapTypeChanged(Event& event)
{
	if (!m_vr)
		return;
	long colormap_type;
	getValue(gstColormapType, colormap_type);
	m_vr->set_colormap(colormap_type);
}

void VolumeData::OnColormapProjChanged(Event& event)
{
	if (!m_vr)
		return;

	long colormap_proj;
	getValue(gstColormapProj, colormap_proj);
	m_vr->set_colormap_proj(colormap_proj);
}

void VolumeData::OnSpacingChanged(Event& event)
{
	if (!m_tex)
		return;
	double spc_x, spc_y, spc_z;
	getValue(gstSpcX, spc_x);
	getValue(gstSpcY, spc_y);
	getValue(gstSpcZ, spc_z);
	m_tex->set_spacings(spc_x, spc_y, spc_z);
	BBox bbox;
	m_tex->get_bounds(bbox);
	event.setNotifyFlags(Event::NOTIFY_PARENT | Event::NOTIFY_AGENT);
	updateValue(gstBounds, bbox, event);

	//m_tex->get_base_spacings(spc_x, spc_y, spc_z);
	event.setNotifyFlags(Event::NOTIFY_SELF);
	updateValue(gstBaseSpcX, spc_x, event);
	updateValue(gstBaseSpcY, spc_y, event);
	updateValue(gstBaseSpcZ, spc_z, event);

	//tex transform
	Transform *temp = m_tex->transform();
    Event texTransform;
    texTransform.setNotifyFlags(Event::NOTIFY_NONE);
    updateValue(gstTexTransform, *temp, texTransform);
}

void VolumeData::OnBaseSpacingChanged(Event& event)
{
	if (!m_tex)
		return;
	double spc_x, spc_y, spc_z;
	getValue(gstBaseSpcX, spc_x);
	getValue(gstBaseSpcY, spc_y);
	getValue(gstBaseSpcZ, spc_z);
	m_tex->set_base_spacings(spc_x, spc_y, spc_z);

	long level;
	getValue(gstLevel, level);
	m_tex->get_spacings(spc_x, spc_y, spc_z, level);
	event.setNotifyFlags(Event::NOTIFY_SELF);
	updateValue(gstSpcX, spc_x, event);
	updateValue(gstSpcY, spc_y, event);
	updateValue(gstSpcZ, spc_z, event);

	//tex transform
	Transform *temp = m_tex->transform();
    Event texTransform;
    texTransform.setNotifyFlags(Event::NOTIFY_NONE);
    updateValue(gstTexTransform, *temp, texTransform);
}

void VolumeData::OnSpacingScaleChanged(Event& event)
{
	if (!m_tex)
		return;
	double spc_x, spc_y, spc_z;
	getValue(gstSpcSclX, spc_x);
	getValue(gstSpcSclY, spc_y);
	getValue(gstSpcSclZ, spc_z);
	m_tex->set_spacing_scales(spc_x, spc_y, spc_z);
}

void VolumeData::OnLevelChanged(Event& event)
{
	if (!m_tex)
		return;
	bool multires = false;
	getValue(gstMultires, multires);
	if (!multires)
		return;
	long level;
	getValue(gstLevel, level);
	m_tex->setLevel(level);
	BBox bbox;
	m_tex->get_bounds(bbox);
	event.setNotifyFlags(Event::NOTIFY_SELF);
	updateValue(gstBounds, bbox, event);

	//tex transform
	Transform *temp = m_tex->transform();
    Event texTransform;
    texTransform.setNotifyFlags(Event::NOTIFY_NONE);
    updateValue(gstTexTransform, *temp, texTransform);
}

void VolumeData::OnDisplayChanged(Event& event)
{
	if (!m_tex)
		return;
	m_tex->set_sort_bricks();
}

void VolumeData::OnInterpolateChanged(Event& event)
{
	if (!m_vr)
		return;

	bool interpolate;
	getValue(gstInterpolate, interpolate);
	m_vr->set_interpolate(interpolate);
}

void VolumeData::OnLabelModeChanged(Event& event)
{
	if (!m_vr)
		return;
}

void VolumeData::OnDepthAttenChanged(Event& event)
{
	if (!m_vr)
		return;

	bool depth_atten;
	double da_int, da_start, da_end;
	getValue(gstDepthAtten, depth_atten);
	getValue(gstDaInt, da_int);
	getValue(gstDaStart, da_start);
	getValue(gstDaEnd, da_end);
	m_vr->set_fog(depth_atten, da_int, da_start, da_end);
}

void VolumeData::OnSkipBrickChanged(Event& event)
{
	if (!m_tex)
		return;
	bool skip_brick;
	getValue(gstSkipBrick, skip_brick);
	m_tex->set_use_priority(skip_brick);
}

void VolumeData::OnClipPlanesChanged(Event& event)
{
	if (!m_vr)
		return;

	PlaneSet planeset;
	getValue(gstClipPlanes, planeset);

	//this needs to be made consistent in the future
	std::vector<Plane*> planelist(0);
	for (int i = 0; i < planeset.GetSize(); ++i)
	{
		Plane* plane = new Plane(planeset.Get(i));
		planelist.push_back(plane);
	}
	m_vr->set_planes(&planelist);

	//calculating planes
	//get six planes
	fluo::Plane* px1 = &(planeset[0]);
	fluo::Plane* px2 = &(planeset[1]);
	fluo::Plane* py1 = &(planeset[2]);
	fluo::Plane* py2 = &(planeset[3]);
	fluo::Plane* pz1 = &(planeset[4]);
	fluo::Plane* pz2 = &(planeset[5]);
	fluo::BBox bounds;
	getValue(gstBounds, bounds);

	fluo::Vector diff =
		bounds.Max() - bounds.Min();
	fluo::Point min = fluo::Point(
		bounds.Min().x() - diff.x()*px1->d(),
		bounds.Min().y() - diff.y()*py1->d(),
		bounds.Min().z() - diff.z()*pz1->d());
	fluo::Point max = fluo::Point(
		bounds.Min().x() + diff.x()*px2->d(),
		bounds.Min().y() + diff.y()*py2->d(),
		bounds.Min().z() + diff.z()*pz2->d());

	setValue(gstClipBounds, fluo::BBox(min, max));
}

void VolumeData::OnIntScaleChanged(Event& event)
{
	if (!m_vr)
		return;

	double int_scale;
	getValue(gstIntScale, int_scale);
	m_vr->set_scalar_scale(int_scale);
	m_vr->set_gm_scale(int_scale);
}

void VolumeData::OnSyncOutputChannels(Event& event)
{
	ValueCollection ss_gamma = {
		gstGammaR,
		gstGammaG,
		gstGammaB
	};
	ValueCollection ss_brigt = {
		gstBrightnessR,
		gstBrightnessG,
		gstBrightnessB
	};
	ValueCollection ss_equal = {
		gstEqualizeR,
		gstEqualizeG,
		gstEqualizeB
	};
	bool sync_r, sync_g, sync_b;
	getValue(gstSyncR, sync_r);
	getValue(gstSyncG, sync_g);
	getValue(gstSyncB, sync_b);

	//unsync all for the sake of simplicity
	unsyncValues(ss_gamma);
	unsyncValues(ss_brigt);
	unsyncValues(ss_equal);

	//then sync
	ss_gamma.clear();
	ss_brigt.clear();
	ss_equal.clear();
	if (sync_r)
	{
		ss_gamma.insert(gstGammaR);
		ss_brigt.insert(gstBrightnessR);
		ss_equal.insert(gstEqualizeR);
	}
	if (sync_g)
	{
		ss_gamma.insert(gstGammaG);
		ss_brigt.insert(gstBrightnessG);
		ss_equal.insert(gstEqualizeG);
	}
	if (sync_b)
	{
		ss_gamma.insert(gstGammaB);
		ss_brigt.insert(gstBrightnessB);
		ss_equal.insert(gstEqualizeB);
	}
	syncValues(ss_gamma);
	syncValues(ss_brigt);
	syncValues(ss_equal);
}

void VolumeData::OnClipX1Changed(Event& event)
{
	bool clip_link;
	getValue(gstClipLinkX, clip_link);
	double value1, value2;
	double clip_dist;
	if (clip_link)
	{
		getValue(gstClipDistX, clip_dist);
		getValue(gstClipX1, value1);
		value2 = value1 + clip_dist;
		if (value2 > 1.0)
		{
			updateValue(gstClipX1, 1.0 - clip_dist, event);
			updateValue(gstClipX2, 1.0, event);
		}
		else
			updateValue(gstClipX2, value2, event);
	}
	else
	{
		getValue(gstClipX1, value1);
		getValue(gstClipX2, value2);
		if (value1 >= value2)
		{
			long resx;
			getValue(gstResX, resx);
			value1 = value2 - 1.0 / resx;
			setValue(gstClipX1, value1);
		}
		clip_dist = value2 - value1;
		updateValue(gstClipDistX, clip_dist, event);
	}

	UpdateClippingPlanes(event);
}

void VolumeData::OnClipX2Changed(Event& event)
{
	bool clip_link;
	getValue(gstClipLinkX, clip_link);
	double value1, value2;
	double clip_dist;
	if (clip_link)
	{
		getValue(gstClipDistX, clip_dist);
		getValue(gstClipX2, value2);
		value1 = value2 - clip_dist;
		if (value1 < 0.0)
		{
			updateValue(gstClipX1, 0.0, event);
			updateValue(gstClipX2, clip_dist, event);
		}
		else
			updateValue(gstClipX1, value1, event);
	}
	else
	{
		getValue(gstClipX1, value1);
		getValue(gstClipX2, value2);
		if (value1 >= value2)
		{
			long resx;
			getValue(gstResX, resx);
			value2 = value1 + 1.0 / resx;
			setValue(gstClipX2, value2);
		}
		clip_dist = value2 - value1;
		updateValue(gstClipDistX, clip_dist, event);
	}

	UpdateClippingPlanes(event);
}

void VolumeData::OnClipY1Changed(Event& event)
{
	bool clip_link;
	getValue(gstClipLinkY, clip_link);
	double value1, value2;
	double clip_dist;
	if (clip_link)
	{
		getValue(gstClipDistY, clip_dist);
		getValue(gstClipY1, value1);
		value2 = value1 + clip_dist;
		if (value2 > 1.0)
		{
			updateValue(gstClipY1, 1.0 - clip_dist, event);
			updateValue(gstClipY2, 1.0, event);
		}
		else
			updateValue(gstClipY2, value2, event);
	}
	else
	{
		getValue(gstClipY1, value1);
		getValue(gstClipY2, value2);
		if (value1 >= value2)
		{
			long resy;
			getValue(gstResY, resy);
			value1 = value2 - 1.0 / resy;
			setValue(gstClipY1, value1);
		}
		clip_dist = value2 - value1;
		updateValue(gstClipDistY, clip_dist, event);
	}

	UpdateClippingPlanes(event);
}

void VolumeData::OnClipY2Changed(Event& event)
{
	bool clip_link;
	getValue(gstClipLinkY, clip_link);
	double value1, value2;
	double clip_dist;
	if (clip_link)
	{
		getValue(gstClipDistY, clip_dist);
		getValue(gstClipY2, value2);
		value1 = value2 - clip_dist;
		if (value1 < 0.0)
		{
			updateValue(gstClipY1, 0.0, event);
			updateValue(gstClipY2, clip_dist, event);
		}
		else
			updateValue(gstClipY1, value1, event);
	}
	else
	{
		getValue(gstClipY1, value1);
		getValue(gstClipY2, value2);
		if (value1 >= value2)
		{
			long resy;
			getValue(gstResY, resy);
			value2 = value1 + 1.0 / resy;
			setValue(gstClipY2, value2);
		}
		clip_dist = value2 - value1;
		updateValue(gstClipDistY, clip_dist, event);
	}

	UpdateClippingPlanes(event);
}

void VolumeData::OnClipZ1Changed(Event& event)
{
	bool clip_link;
	getValue(gstClipLinkZ, clip_link);
	double value1, value2;
	double clip_dist;
	if (clip_link)
	{
		getValue(gstClipDistZ, clip_dist);
		getValue(gstClipZ1, value1);
		value2 = value1 + clip_dist;
		if (value2 > 1.0)
		{
			updateValue(gstClipZ1, 1.0 - clip_dist, event);
			updateValue(gstClipZ2, 1.0, event);
		}
		else
			updateValue(gstClipZ2, value2, event);
	}
	else
	{
		getValue(gstClipZ1, value1);
		getValue(gstClipZ2, value2);
		if (value1 >= value2)
		{
			long resz;
			getValue(gstResZ, resz);
			value1 = value2 - 1.0 / resz;
			setValue(gstClipZ1, value1);
		}
		clip_dist = value2 - value1;
		updateValue(gstClipDistZ, clip_dist, event);
	}

	UpdateClippingPlanes(event);
}

void VolumeData::OnClipZ2Changed(Event& event)
{
	bool clip_link;
	getValue(gstClipLinkZ, clip_link);
	double value1, value2;
	double clip_dist;
	if (clip_link)
	{
		getValue(gstClipDistZ, clip_dist);
		getValue(gstClipZ2, value2);
		value1 = value2 - clip_dist;
		if (value1 < 0.0)
		{
			updateValue(gstClipZ1, 0.0, event);
			updateValue(gstClipZ2, clip_dist, event);
		}
		else
			updateValue(gstClipZ1, value1, event);
	}
	else
	{
		getValue(gstClipZ1, value1);
		getValue(gstClipZ2, value2);
		if (value1 >= value2)
		{
			long resz;
			getValue(gstResZ, resz);
			value2 = value1 + 1.0 / resz;
			setValue(gstClipZ2, value2);
		}
		clip_dist = value2 - value1;
		updateValue(gstClipDistZ, clip_dist, event);
	}

	UpdateClippingPlanes(event);
}

void VolumeData::OnClipRot(Event& event)
{
	UpdateClippingPlanes(event);
}

void VolumeData::UpdateClippingPlanes(Event& event)
{
	double clip_x1, clip_x2, clip_y1, clip_y2, clip_z1, clip_z2;
	getValue(gstClipX1, clip_x1);
	getValue(gstClipX2, clip_x2);
	getValue(gstClipY1, clip_y1);
	getValue(gstClipY2, clip_y2);
	getValue(gstClipZ1, clip_z1);
	getValue(gstClipZ2, clip_z2);
	double clip_rot_x, clip_rot_y, clip_rot_z;
	getValue(gstClipRotX, clip_rot_x);
	getValue(gstClipRotY, clip_rot_y);
	getValue(gstClipRotZ, clip_rot_z);
	double spc_x, spc_y, spc_z;
	long res_x, res_y, res_z;
	getValue(gstSpcX, spc_x);
	getValue(gstSpcY, spc_y);
	getValue(gstSpcZ, spc_z);
	getValue(gstResX, res_x);
	getValue(gstResY, res_y);
	getValue(gstResZ, res_z);

	Quaternion cl_quat;
	cl_quat.FromEuler(clip_rot_x, clip_rot_y, clip_rot_z);
	Vector scale;
	if (spc_x > 0.0 && spc_y > 0.0 && spc_z > 0.0)
	{
		scale = Vector(
			1.0 / res_x / spc_x,
			1.0 / res_y / spc_y,
			1.0 / res_z / spc_z);
		scale.safe_normalize();
	}
	else
		scale = Vector(1.0);

	PlaneSet planes(6);
	//clip first
	planes[0].ChangePlane(Point(clip_x1, 0, 0), Vector(1, 0, 0));
	planes[1].ChangePlane(Point(clip_x2, 0, 0), Vector(-1, 0, 0));
	planes[2].ChangePlane(Point(0, clip_y1, 0), Vector(0, 1, 0));
	planes[3].ChangePlane(Point(0, clip_y2, 0), Vector(0, -1, 0));
	planes[4].ChangePlane(Point(0, 0, clip_z1), Vector(0, 0, 1));
	planes[5].ChangePlane(Point(0, 0, clip_z2), Vector(0, 0, -1));
	//then rotate
	Vector trans1(-0.5);
	Vector trans2(0.5);
	planes.Translate(trans1);
	planes.Rotate(cl_quat);
	planes.Scale(scale);
	planes.Translate(trans2);
	//set and update to the renderer
	updateValue(gstClipPlanes, planes, event);
}

//randomize color
void VolumeData::OnRandomizeColor(Event& event)
{
	double hue = (double)rand() / (RAND_MAX) * 360.0;
	Color color(HSVColor(hue, 1.0, 1.0));
	updateValue(gstColor, color, event);
}

//functions from old class
//reader
void VolumeData::SetReader(BaseReader* reader)
{
	m_reader = reader;
}

BaseReader* VolumeData::GetReader()
{
	return m_reader;
}

//load

int VolumeData::LoadData(Nrrd* data, const std::string &name, const std::wstring &path)
{
	if (!data || data->dim != 3)
		return 0;

	setValue(gstDataPath, path);
	setName(name);

	if (m_tex)
	{
		delete m_tex;
		m_tex = NULL;
	}

	Nrrd *nv = data;

	m_tex = new flvr::Texture();
	//bool skip_brick;
	//getValue(gstSkipBrick, skip_brick);
	//m_tex->set_use_priority(skip_brick);

	if (m_reader && m_reader->GetType() == READER_BRKXML_TYPE)
	{
		BRKXMLReader *breader = (BRKXMLReader*)m_reader;
		std::vector<flvr::Pyramid_Level> pyramid;
		std::vector<std::vector<std::vector<std::vector<flvr::FileLocInfo *>>>> fnames;
		int ftype = BRICK_FILE_TYPE_NONE;

		breader->build_pyramid(pyramid, fnames, 0, breader->GetCurChan());
		m_tex->SetCopyableLevel(breader->GetCopyableLevel());

		int lmnum = breader->GetLandmarkNum();
		for (int j = 0; j < lmnum; j++)
		{
			std::wstring name;
			VD_Landmark vlm;
			breader->GetLandmark(j, vlm.name, vlm.x, vlm.y, vlm.z, vlm.spcx, vlm.spcy, vlm.spcz);
			m_landmarks.push_back(vlm);
			breader->GetMetadataID(m_metadata_id);
		}
		if (!m_tex->buildPyramid(pyramid, fnames, breader->isURL())) return 0;
	}
	else
	{
		if (!m_tex->build(nv, 0, 0, 256, 0, 0))
			return 0;
	}

	if (m_tex)
	{
		if (m_vr)
			delete m_vr;

		PlaneSet planeset;
		getValue(gstClipPlanes, planeset);
		//this needs to be made consistent in the future
		std::vector<Plane*> planelist(0);
		for (int i = 0; i < planeset.GetSize(); ++i)
		{
			Plane* plane = new Plane(planeset.Get(i));
			planelist.push_back(plane);
		}

		m_vr = new flvr::VolumeRenderer(m_tex, planelist);
		BBox bounds;
		Point pmax(data->axis[0].max, data->axis[1].max, data->axis[2].max);
		Point pmin(data->axis[0].min, data->axis[1].min, data->axis[2].min);
		bounds.extend(pmin);
		bounds.extend(pmax);
		setValue(gstBounds, bounds);
		setValue(gstResX, long(nv->axis[0].size));
		setValue(gstResY, long(nv->axis[1].size));
		setValue(gstResZ, long(nv->axis[2].size));
		Initialize();

		//double sample_rate;
		//getValue(gstSampleRate, sample_rate);
		//m_vr->set_sampling_rate(sample_rate);
		//double mat_amb, mat_diff, mat_spec, mat_shine;
		//getValue(gstMatAmb, mat_amb);
		//getValue(gstMatDiff, mat_diff);
		//getValue(gstMatSpec, mat_spec);
		//getValue(gstMatShine, mat_shine);
		//m_vr->set_material(mat_amb, mat_diff, mat_spec, mat_shine);
		//bool shading;
		//getValue(gstShadingEnable, shading);
		//m_vr->set_shading(shading);
		//double int_scale;
		//getValue(gstIntScale, int_scale);
		//m_vr->set_scalar_scale(int_scale);
		//m_vr->set_gm_scale(int_scale);

		//OnMipModeChanged();


		//bool depth_atten;
		//double da_int, da_start, da_end;
		//getValue(gstDepthAtten, depth_atten);
		//getValue(gstDaInt, da_int);
		//getValue(gstDaStart, da_start);
		//getValue(gstDaEnd, da_end);
		//m_vr->set_fog(false, da_int, da_start, da_end);

		//double high_threshold;
		//getValue(gstHighThreshold, high_threshold);
		//m_vr->set_hi_thresh(high_threshold);
		//
		//double spcx, spcy, spcz;
		//getValue(gstSpcX, spcx);
		//getValue(gstSpcY, spcy);
		//getValue(gstSpcZ, spcz);
		//m_tex->set_spacings(spcx, spcy, spcz);
		//m_tex->set_base_spacings(spcx, spcy, spcz);
	}

	return 1;
}

int VolumeData::ReplaceData(Nrrd* data, bool del_tex)
{
	if (!data || data->dim != 3)
		return 0;

	if (del_tex)
	{
		Nrrd *nv = data;
		setValue(gstResX, long(nv->axis[0].size));
		setValue(gstResY, long(nv->axis[1].size));
		setValue(gstResZ, long(nv->axis[2].size));

		if (m_tex)
			delete m_tex;
		m_tex = new flvr::Texture();
		bool skip_brick;
		getValue(gstSkipBrick, skip_brick);
		m_tex->set_use_priority(skip_brick);
		double max_int;
		getValue(gstMaxInt, max_int);
		m_tex->build(nv, 0, 0, max_int, 0, 0);
	}
	else
	{
		//set new
		m_tex->set_nrrd(data, 0);
	}

	//clear pool
	if (m_vr)
		m_vr->set_texture(m_tex);
	else
		return 0;

	return 1;
}

int VolumeData::ReplaceData(VolumeData* data)
{
	if (!data)
		return 0;
	long resx, resy, resz, resx2, resy2, resz2;
	getValue(gstResX, resx);
	getValue(gstResY, resy);
	getValue(gstResZ, resz);
	data->getValue(gstResX, resx2);
	data->getValue(gstResY, resy2);
	data->getValue(gstResZ, resz2);
	if (resx != resx2 || resy != resy2 || resz != resz2)
		return 0;

	double spcx = 1.0, spcy = 1.0, spcz = 1.0;

	if (m_tex && m_vr)
	{
		m_tex->get_spacings(spcx, spcy, spcz);
		m_vr->clear_tex_current();
		delete m_tex;
		m_vr->reset_texture();
	}
	m_tex = data->GetTexture();
	data->SetTexture();
	ValueCollection names{ gstIntScale, gstGmScale, gstMaxInt };
	data->propagateValues(names, this);
	if (m_vr)
		m_vr->set_texture(m_tex);
	else
		return 0;
	return 1;
}

Nrrd* VolumeData::GetData(bool ret)
{
	if (m_vr && m_tex)
	{
		if (ret) m_vr->return_volume();
		return m_tex->get_nrrd(0);
	}

	return 0;
}

void VolumeData::AddEmptyData(int bits,
	int nx, int ny, int nz,
	double spcx, double spcy, double spcz,
	int brick_size)
{
	if (bits != 8 && bits != 16)
		return;

	if (m_vr)
		delete m_vr;
	if (m_tex)
		delete m_tex;

	Nrrd *nv = nrrdNew();
	if (bits == 8)
	{
		unsigned long long mem_size = (unsigned long long)nx*
			(unsigned long long)ny*(unsigned long long)nz;
		uint8_t *val8 = new (std::nothrow) uint8_t[mem_size];
		if (!val8)
		{
			//wxMessageBox("Not enough memory. Please save project and restart.");
			return;
		}
		memset((void*)val8, 0, sizeof(uint8_t)*nx*ny*nz);
        //nrrdWrap(nv, val8, nrrdTypeUChar, 3, (size_t)nx, (size_t)ny, (size_t)nz); //may be deprecated
        nrrdWrap_va(nv, val8, nrrdTypeUChar, 3, (size_t)nx, (size_t)ny, (size_t)nz);
		setValue(gstBits, long(8));
	}
	else if (bits == 16)
	{
		unsigned long long mem_size = (unsigned long long)nx*
			(unsigned long long)ny*(unsigned long long)nz;
		uint16_t *val16 = new (std::nothrow) uint16_t[mem_size];
		if (!val16)
		{
			//wxMessageBox("Not enough memory. Please save project and restart.");
			return;
		}
		memset((void*)val16, 0, sizeof(uint16_t)*nx*ny*nz);
        //nrrdWrap(nv, val16, nrrdTypeUShort, 3, (size_t)nx, (size_t)ny, (size_t)nz); //may be deprecated
        nrrdWrap_va(nv, val16, nrrdTypeUShort, 3, (size_t)nx, (size_t)ny, (size_t)nz);
		setValue(gstBits, long(16));
    }
    
    /*
    nrrdAxisInfoSet(nv, nrrdAxisInfoSpacing, spcx, spcy, spcz);
	nrrdAxisInfoSet(nv, nrrdAxisInfoMax, spcx*nx, spcy*ny, spcz*nz);
	nrrdAxisInfoSet(nv, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
	nrrdAxisInfoSet(nv, nrrdAxisInfoSize, (size_t)nx, (size_t)ny, (size_t)nz);
    these may be deprecated
    */

    nrrdAxisInfoSet_va(nv, nrrdAxisInfoSpacing, spcx, spcy, spcz);
    nrrdAxisInfoSet_va(nv, nrrdAxisInfoMax, spcx*nx, spcy*ny, spcz*nz);
    nrrdAxisInfoSet_va(nv, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
    nrrdAxisInfoSet_va(nv, nrrdAxisInfoSize, (size_t)nx, (size_t)ny, (size_t)nz);

	//resolution
	setValue(gstResX, long(nv->axis[0].size));
	setValue(gstResY, long(nv->axis[1].size));
	setValue(gstResZ, long(nv->axis[2].size));

	//bounding box
	BBox bounds;
    Point pmax(nv->axis[0].max, nv->axis[1].max, nv->axis[2].max);
    Point pmin(nv->axis[0].min, nv->axis[1].min, nv->axis[2].min);
	bounds.extend(pmin);
	bounds.extend(pmax);
	setValue(gstBounds, bounds);

	//create texture
	m_tex = new flvr::Texture();
	m_tex->set_use_priority(false);
	m_tex->set_brick_size(brick_size);
	m_tex->build(nv, 0, 0, 256, 0, 0);
	m_tex->set_spacings(spcx, spcy, spcz);

	//clipping planes
	//this needs to be made consistent in the future
	PlaneSet planeset;
	getValue(gstClipPlanes, planeset);
	std::vector<Plane*> planelist(0);
	for (int i = 0; i < planeset.GetSize(); ++i)
	{
		Plane* plane = new Plane(planeset.Get(0));
		planelist.push_back(plane);
	}

	//create volume renderer
	m_vr = new flvr::VolumeRenderer(m_tex, planelist);
	double sample_rate;
	getValue(gstSampleRate, sample_rate);
	m_vr->set_sampling_rate(sample_rate);
	double mat_amb, mat_diff, mat_spec, mat_shine;
	getValue(gstMatAmb, mat_amb);
	getValue(gstMatDiff, mat_diff);
	getValue(gstMatSpec, mat_spec);
	getValue(gstMatShine, mat_shine);
	m_vr->set_material(mat_amb, mat_diff, mat_spec, mat_shine);
	bool shading;
	getValue(gstShadingEnable, shading);
	m_vr->set_shading(shading);
	double int_scale;
	getValue(gstIntScale, int_scale);
	m_vr->set_scalar_scale(int_scale);
	m_vr->set_gm_scale(int_scale);

    Event newEvent;

    OnMipModeChanged(newEvent);
}

void VolumeData::LoadMask(Nrrd* mask)
{
	if (!mask || !m_tex || !m_vr)
		return;

	//consider doing processing outside of the VolumeData class
	//to reduce coupling
/*	int nx2, ny2, nz2;
	nx2 = mask->axis[0].size;
	ny2 = mask->axis[1].size;
	nz2 = mask->axis[2].size;
	long resx, resy, resz;
	getValue(gstResX, resx);
	getValue(gstResY, resy);
	getValue(gstResZ, resz);
	if (resx != nx2 || resy != ny2 || resz != nz2)
	{
		double spcx, spcy, spcz;
		//GetSpacings(spcx, spcy, spcz);
		getValue(gstSpcX, spcx);
		getValue(gstSpcY, spcy);
		getValue(gstSpcZ, spcz);
		FL::VolumeSampler sampler;
		sampler.SetVolume(mask);
		sampler.SetSize(resx, resy, resz);
		sampler.SetSpacings(spcx, spcy, spcz);
		sampler.SetType(0);
		//sampler.SetFilterSize(2, 2, 0);
		sampler.Resize();
		nrrdNuke(mask);
		mask = sampler.GetResult();
	}*/

	//prepare the texture bricks for the mask
	m_tex->add_empty_mask();
	m_tex->set_nrrd(mask, m_tex->nmask());
}

Nrrd* VolumeData::GetMask(bool ret)
{
	if (m_vr && m_tex && m_tex->nmask() != -1)
	{
		if (ret) m_vr->return_mask();
		return m_tex->get_nrrd(m_tex->nmask());
	}

	return 0;
}

void VolumeData::AddEmptyMask(int mode, bool change)
{
	if (!m_tex || !m_vr)
		return;

	Nrrd *nrrd_mask = 0;
	uint8_t *val8 = 0;
	long resx, resy, resz;
	getValue(gstResX, resx);
	getValue(gstResY, resy);
	getValue(gstResZ, resz);
	unsigned long long mem_size = (unsigned long long)resx*
		(unsigned long long)resy*(unsigned long long)resz;
	//prepare the texture bricks for the mask
	bool empty = m_tex->add_empty_mask();
	if (empty)
	{
		//add the nrrd data for mask
		nrrd_mask = nrrdNew();
		val8 = new (std::nothrow) uint8_t[mem_size];
		if (!val8)
		{
			//wxMessageBox("Not enough memory. Please save project and restart.");
			return;
		}
		double spcx, spcy, spcz;
        m_tex->get_spacings(spcx, spcy, spcz);
        
        /*
		nrrdWrap(nrrd_mask, val8, nrrdTypeUChar, 3, (size_t)resx, (size_t)resy, (size_t)resz);
		nrrdAxisInfoSet(nrrd_mask, nrrdAxisInfoSize, (size_t)resx, (size_t)resy, (size_t)resz);
		nrrdAxisInfoSet(nrrd_mask, nrrdAxisInfoSpacing, spcx, spcy, spcz);
		nrrdAxisInfoSet(nrrd_mask, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
		nrrdAxisInfoSet(nrrd_mask, nrrdAxisInfoMax, spcx*resx, spcy*resy, spcz*resz);

        these may be deprecated
        */

        nrrdWrap_va(nrrd_mask, val8, nrrdTypeUChar, 3, (size_t)resx, (size_t)resy, (size_t)resz);
        nrrdAxisInfoSet_va(nrrd_mask, nrrdAxisInfoSize, (size_t)resx, (size_t)resy, (size_t)resz);
        nrrdAxisInfoSet_va(nrrd_mask, nrrdAxisInfoSpacing, spcx, spcy, spcz);
        nrrdAxisInfoSet_va(nrrd_mask, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
        nrrdAxisInfoSet_va(nrrd_mask, nrrdAxisInfoMax, spcx*resx, spcy*resy, spcz*resz);

		m_tex->set_nrrd(nrrd_mask, m_tex->nmask());
	}
	else
	{
		nrrd_mask = m_tex->get_nrrd(m_tex->nmask());
		val8 = (uint8_t*)nrrd_mask->data;
	}

	if (empty || change)
	{
		if (mode == 0 || mode == 1)
		{
			if (val8)
				memset((void*)val8, mode ?
					255 : 0, mem_size * sizeof(uint8_t));
		}
	}
}

void VolumeData::AddMask(Nrrd* mask, int op)
{
	if (!mask || !mask->data || !m_tex || !m_vr)
		return;
	long resx, resy, resz;
	getValue(gstResX, resx);
	getValue(gstResY, resy);
	getValue(gstResZ, resz);
	if (mask->dim != 3 ||
		mask->axis[0].size != resx ||
		mask->axis[1].size != resy ||
		mask->axis[2].size != resz)
		return;

	Nrrd *nrrd_mask = 0;
	uint8_t *val8 = 0;
	unsigned long long mem_size = (unsigned long long)resx*
		(unsigned long long)resy*(unsigned long long)resz;
	//prepare the texture bricks for the mask
	bool empty = m_tex->add_empty_mask();
	if (empty)
	{
		//add the nrrd data for mask
		nrrd_mask = nrrdNew();
		val8 = new (std::nothrow) uint8_t[mem_size];
		if (!val8)
		{
			//wxMessageBox("Not enough memory. Please save project and restart.");
			return;
		}
		double spcx, spcy, spcz;
		m_tex->get_spacings(spcx, spcy, spcz);
		nrrdWrap(nrrd_mask, val8, nrrdTypeUChar, 3, (size_t)resx, (size_t)resy, (size_t)resz);
		nrrdAxisInfoSet(nrrd_mask, nrrdAxisInfoSize, (size_t)resx, (size_t)resy, (size_t)resz);
		nrrdAxisInfoSet(nrrd_mask, nrrdAxisInfoSpacing, spcx, spcy, spcz);
		nrrdAxisInfoSet(nrrd_mask, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
		nrrdAxisInfoSet(nrrd_mask, nrrdAxisInfoMax, spcx*resx, spcy*resy, spcz*resz);

		m_tex->set_nrrd(nrrd_mask, m_tex->nmask());
	}
	else
	{
		nrrd_mask = m_tex->get_nrrd(m_tex->nmask());
		val8 = (uint8_t*)nrrd_mask->data;
	}

	if (val8)
	{
		if (op > 0 && !empty)
		{
			switch (op)
			{
			case 1://union
				for (unsigned long long index = 0;
					index < mem_size; ++index)
				{
					val8[index] = std::max(val8[index],
						((uint8_t*)(mask->data))[index]);
				}
				break;
			case 2://exclude
				for (unsigned long long index = 0;
					index < mem_size; ++index)
				{
					if (std::min(val8[index],
						((uint8_t*)(mask->data))[index]) > 0)
						val8[index] = 0;
				}
				break;
			case 3://intersect
				for (unsigned long long index = 0;
					index < mem_size; ++index)
				{
					val8[index] = std::min(val8[index],
						((uint8_t*)(mask->data))[index]);
				}
				break;
			}
		}
		else//replace
		{
			memcpy(val8, mask->data, mem_size * sizeof(uint8_t));
		}
		m_vr->clear_tex_mask(false);
	}
}

void VolumeData::AddMask16(Nrrd* mask, int op, double scale)
{
	if (!mask || !mask->data || !m_tex || !m_vr)
		return;
	long resx, resy, resz;
	getValue(gstResX, resx);
	getValue(gstResY, resy);
	getValue(gstResZ, resz);
	if (mask->dim != 3 ||
		mask->axis[0].size != resx ||
		mask->axis[1].size != resy ||
		mask->axis[2].size != resz)
		return;

	Nrrd *nrrd_mask = 0;
	uint8_t *val8 = 0;
	unsigned long long mem_size = (unsigned long long)resx*
		(unsigned long long)resy*(unsigned long long)resz;
	//prepare the texture bricks for the mask
	bool empty = m_tex->add_empty_mask();
	if (empty)
	{
		//add the nrrd data for mask
		nrrd_mask = nrrdNew();
		val8 = new (std::nothrow) uint8_t[mem_size];
		if (!val8)
		{
			//wxMessageBox("Not enough memory. Please save project and restart.");
			return;
		}
		double spcx, spcy, spcz;
		m_tex->get_spacings(spcx, spcy, spcz);
		nrrdWrap(nrrd_mask, val8, nrrdTypeUChar, 3, (size_t)resx, (size_t)resy, (size_t)resz);
		nrrdAxisInfoSet(nrrd_mask, nrrdAxisInfoSize, (size_t)resx, (size_t)resy, (size_t)resz);
		nrrdAxisInfoSet(nrrd_mask, nrrdAxisInfoSpacing, spcx, spcy, spcz);
		nrrdAxisInfoSet(nrrd_mask, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
		nrrdAxisInfoSet(nrrd_mask, nrrdAxisInfoMax, spcx*resx, spcy*resy, spcz*resz);

		m_tex->set_nrrd(nrrd_mask, m_tex->nmask());
	}
	else
	{
		nrrd_mask = m_tex->get_nrrd(m_tex->nmask());
		val8 = (uint8_t*)nrrd_mask->data;
	}

	if (val8)
	{
		if (op > 0 && !empty)
		{
			switch (op)
			{
			case 1://union
				for (unsigned long long index = 0;
					index < mem_size; ++index)
				{
					val8[index] = std::max(val8[index],
						uint8_t(scale*((uint16_t*)(mask->data))[index]));
				}
				break;
			case 2://exclude
				for (unsigned long long index = 0;
					index < mem_size; ++index)
				{
					if (std::min(val8[index],
						uint8_t(scale*((uint16_t*)(mask->data))[index])) > 0)
						val8[index] = 0;
				}
				break;
			case 3://intersect
				for (unsigned long long index = 0;
					index < mem_size; ++index)
				{
					val8[index] = std::min(val8[index],
						uint8_t(scale*((uint16_t*)(mask->data))[index]));
				}
				break;
			}
		}
		else//replace
		{
			for (unsigned long long index = 0;
				index < mem_size; ++index)
			{
				val8[index] = uint8_t(scale*((uint16_t*)(mask->data))[index]);
			}
		}
		m_vr->clear_tex_mask(false);
	}
}

void VolumeData::LoadLabel(Nrrd* label)
{
	if (!label || !m_tex || !m_vr)
		return;

	//consider doing processing outside of the VolumeData class
	//to reduce coupling
/*	int nx2, ny2, nz2;
	nx2 = label->axis[0].size;
	ny2 = label->axis[1].size;
	nz2 = label->axis[2].size;
	long resx, resy, resz;
	getValue(gstResX, resx);
	getValue(gstResY, resy);
	getValue(gstResZ, resz);
	if (resx != nx2 || resy != ny2 || resz != nz2)
	{
		double spcx, spcy, spcz;
		//GetSpacings(spcx, spcy, spcz);
		getValue(gstSpcX, spcx);
		getValue(gstSpcY, spcy);
		getValue(gstSpcZ, spcz);
		FL::VolumeSampler sampler;
		sampler.SetVolume(label);
		sampler.SetSize(resx, resy, resz);
		sampler.SetSpacings(spcx, spcy, spcz);
		sampler.Resize();
		nrrdNuke(label);
		label = sampler.GetResult();
	}*/

	m_tex->add_empty_label();
	m_tex->set_nrrd(label, m_tex->nlabel());
}

Nrrd* VolumeData::GetLabel(bool ret)
{
	if (m_vr && m_tex && m_tex->nlabel() != -1)
	{
		if (ret) m_vr->return_label();
		return m_tex->get_nrrd(m_tex->nlabel());
	}

	return 0;
}

void VolumeData::AddEmptyLabel(int mode, bool change)
{
	if (!m_tex || !m_vr)
		return;

	long resx, resy, resz;
	getValue(gstResX, resx);
	getValue(gstResY, resy);
	getValue(gstResZ, resz);
	unsigned long long mem_size = (unsigned long long)resx*
		(unsigned long long)resy*(unsigned long long)resz;
	Nrrd *nrrd_label = 0;
	unsigned int *val32 = 0;
	bool exist = false;
	//prepare the texture bricks for the labeling mask
	if (m_tex->add_empty_label())
	{
		//add the nrrd data for the labeling mask
		nrrd_label = nrrdNew();
		val32 = new (std::nothrow) unsigned int[mem_size];
		if (!val32)
		{
			//wxMessageBox("Not enough memory. Please save project and restart.");
			return;
		}

		double spcx, spcy, spcz;
		m_tex->get_spacings(spcx, spcy, spcz);
        /*
		nrrdWrap(nrrd_label, val32, nrrdTypeUInt, 3, (size_t)resx, (size_t)resy, (size_t)resz);
		nrrdAxisInfoSet(nrrd_label, nrrdAxisInfoSpacing, spcx, spcy, spcz);
		nrrdAxisInfoSet(nrrd_label, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
		nrrdAxisInfoSet(nrrd_label, nrrdAxisInfoMax, spcx*resx, spcy*resy, spcz*resz);
		nrrdAxisInfoSet(nrrd_label, nrrdAxisInfoSize, (size_t)resx, (size_t)resy, (size_t)resz);

        these may be deprecated
        */
        nrrdWrap_va(nrrd_label, val32, nrrdTypeUInt, 3, (size_t)resx, (size_t)resy, (size_t)resz);
        nrrdAxisInfoSet_va(nrrd_label, nrrdAxisInfoSpacing, spcx, spcy, spcz);
        nrrdAxisInfoSet_va(nrrd_label, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
        nrrdAxisInfoSet_va(nrrd_label, nrrdAxisInfoMax, spcx*resx, spcy*resy, spcz*resz);
        nrrdAxisInfoSet_va(nrrd_label, nrrdAxisInfoSize, (size_t)resx, (size_t)resy, (size_t)resz);

		m_tex->set_nrrd(nrrd_label, m_tex->nlabel());
	}
	else
	{
		nrrd_label = m_tex->get_nrrd(m_tex->nlabel());
		val32 = (unsigned int*)nrrd_label->data;
		exist = true;
	}

	//apply values
	if (!exist || change)
	{
		switch (mode)
		{
		case 0://zeros
			memset(val32, 0, mem_size);
			break;
		case 1://ordered
			SetOrderedID(val32);
			break;
		case 2://shuffled
			SetShuffledID(val32);
			break;
		}
	}
}

bool VolumeData::SearchLabel(unsigned int label)
{
	if (!m_tex)
		return false;

	Nrrd* nrrd_label = m_tex->get_nrrd(m_tex->nlabel());
	if (!nrrd_label)
		return false;
	unsigned int* data_label = (unsigned int*)(nrrd_label->data);
	if (!data_label)
		return false;

	long resx, resy, resz;
	getValue(gstResX, resx);
	getValue(gstResY, resy);
	getValue(gstResZ, resz);
	unsigned long long mem_size = (unsigned long long)resx*
		(unsigned long long)resy*(unsigned long long)resz;
	for (unsigned long long index = 0; index < mem_size; ++index)
		if (data_label[index] == label)
			return true;
	return false;
}

//save label
void VolumeData::PushLabel(bool ret)
{
	if (ret && m_vr)
		m_vr->return_label();
	if (!m_tex)
		return;
	if (m_tex->nlabel() == -1)
		return;

	Nrrd* data = m_tex->get_nrrd(m_tex->nlabel());
	if (!data || !data->data)
		return;
	long nx, ny, nz;
	getValue(gstResX, nx);
	getValue(gstResY, ny);
	getValue(gstResZ, nz);
	unsigned long long size = (unsigned long long)nx * ny * nz;
	if (!m_label_save)
		m_label_save = new unsigned int[size];
	memcpy(m_label_save, data->data, size * sizeof(unsigned int));
}

void VolumeData::PopLabel()
{
	delete[] m_label_save;
	m_label_save = 0;
}

void VolumeData::LoadLabel2()
{
	if (m_label_save && m_vr)
	{
		Nrrd* data = m_tex->get_nrrd(m_tex->nlabel());
		if (!data || !data->data)
			return;
		long nx, ny, nz;
		getValue(gstResX, nx);
		getValue(gstResY, ny);
		getValue(gstResZ, nz);
		unsigned long long size = (unsigned long long)nx * ny * nz;
		memcpy(data->data, m_label_save, size * sizeof(unsigned int));
		m_vr->clear_tex_current();
	}
}

double VolumeData::GetOriginalValue(int i, int j, int k, flvr::TextureBrick* b)
{
	void *data_data = 0;
	long bits;
	getValue(gstBits, bits);
	bool multires;
	getValue(gstMultires, multires);
	double int_scale;
	getValue(gstIntScale, int_scale);

	int64_t nx, ny, nz;
	if (multires)
	{
		if (!b || !b->isLoaded()) return 0.0;
		flvr::FileLocInfo *finfo = m_tex->GetFileName(b->getID());
		data_data = b->tex_data_brk(0, finfo);
		if (!data_data) return 0.0;
		//bits = b->nb(0) * 8;
		nx = b->nx();
		ny = b->ny();
		nz = b->nz();
	}
	else
	{
		Nrrd* data = 0;
		data = m_tex->get_nrrd(0);
		if (!data || !data->data) return 0.0;
		data_data = data->data;
		//if (data->type == nrrdTypeUChar)
		//	bits = 8;
		//else if (data->type == nrrdTypeUShort)
		//	bits = 16;
		nx = (int64_t)(data->axis[0].size);
		ny = (int64_t)(data->axis[1].size);
		nz = (int64_t)(data->axis[2].size);
	}

	if (i<0 || i >= nx || j<0 || j >= ny || k<0 || k >= nz)
		return 0.0;
	uint64_t ii = i, jj = j, kk = k;

	if (bits == 8)
	{
		uint64_t index = (nx)*(ny)*(kk)+(nx)*(jj)+(ii);
		uint8_t old_value = ((uint8_t*)(data_data))[index];
		return double(old_value) / 255.0;
	}
	else if (bits == 16)
	{
		uint64_t index = (nx)*(ny)*(kk)+(nx)*(jj)+(ii);
		uint16_t old_value = ((uint16_t*)(data_data))[index];
		return double(old_value)*int_scale / 65535.0;
	}

	return 0.0;
}

double VolumeData::GetTransferValue(int i, int j, int k, flvr::TextureBrick* b)
{
	void *data_data = 0;
	long bits;
	getValue(gstBits, bits);
	bool multires;
	getValue(gstMultires, multires);

	int64_t nx, ny, nz;
	if (multires)
	{
		if (!b || !b->isLoaded()) return 0.0;
		flvr::FileLocInfo *finfo = m_tex->GetFileName(b->getID());
		data_data = b->tex_data_brk(0, finfo);
		if (!data_data) return 0.0;
		//bits = b->nb(0) * 8;
		nx = b->nx();
		ny = b->ny();
		nz = b->nz();
	}
	else
	{
		Nrrd* data = 0;
		data = m_tex->get_nrrd(0);
		if (!data || !data->data) return 0.0;
		data_data = data->data;
		//if (data->type == nrrdTypeUChar)
		//	bits = 8;
		//else if (data->type == nrrdTypeUShort)
		//	bits = 16;
		nx = (int64_t)(data->axis[0].size);
		ny = (int64_t)(data->axis[1].size);
		nz = (int64_t)(data->axis[2].size);
	}

	if (i<0 || i >= nx || j<0 || j >= ny || k<0 || k >= nz)
		return 0.0;
	int64_t ii = i, jj = j, kk = k;

	//this will become inefficient, package transfer function in the future
	double lo_thresh, hi_thresh, soft_thresh, gamma3d, extract_boundary, saturation, alpha, int_scale;
	getValue(gstLowThreshold, lo_thresh);
	getValue(gstHighThreshold, hi_thresh);
	getValue(gstSoftThresh, soft_thresh);
	getValue(gstGamma3d, gamma3d);
	getValue(gstExtractBoundary, extract_boundary);
	getValue(gstSaturation, saturation);
	getValue(gstAlpha, alpha);
	getValue(gstIntScale, int_scale);

	if (bits == 8)
	{
		uint64_t index = nx*ny*kk + nx*jj + ii;
		uint8_t old_value = ((uint8_t*)(data_data))[index];
		double gm = 0.0;
		double new_value = double(old_value) / 255.0;
		if (m_vr->get_inversion())
			new_value = 1.0 - new_value;
		if (i>0 && i<nx - 1 &&
			j>0 && j<ny - 1 &&
			k>0 && k<nz - 1)
		{
			double v1 = ((uint8_t*)(data_data))[nx*ny*kk + nx*jj + ii - 1];
			double v2 = ((uint8_t*)(data_data))[nx*ny*kk + nx*jj + ii + 1];
			double v3 = ((uint8_t*)(data_data))[nx*ny*kk + nx*(jj - 1) + ii];
			double v4 = ((uint8_t*)(data_data))[nx*ny*kk + nx*(jj + 1) + ii];
			double v5 = ((uint8_t*)(data_data))[nx*ny*(kk - 1) + nx*jj + ii];
			double v6 = ((uint8_t*)(data_data))[nx*ny*(kk + 1) + nx*jj + ii];
			double normal_x, normal_y, normal_z;
			normal_x = (v2 - v1) / 255.0;
			normal_y = (v4 - v3) / 255.0;
			normal_z = (v6 - v5) / 255.0;
			gm = sqrt(normal_x*normal_x + normal_y*normal_y + normal_z*normal_z)*0.53;
		}
		if (new_value<lo_thresh - soft_thresh ||
			new_value>hi_thresh + soft_thresh)
			new_value = 0.0;
		else
		{
			double gamma = 1.0 / gamma3d;
			new_value = (new_value<lo_thresh ?
				(soft_thresh - lo_thresh + new_value) / soft_thresh :
				(new_value>hi_thresh ?
				(soft_thresh - new_value + hi_thresh) / soft_thresh : 1.0))
				*new_value;
			new_value *= (extract_boundary > 0.0 ?
				Clamp(gm / extract_boundary, 0.0,
					1.0 + extract_boundary*10.0) : 1.0);
			new_value = pow(Clamp(new_value / saturation,
				gamma<1.0 ? -(gamma - 1.0)*0.00001 : 0.0,
				gamma>1.0 ? 0.9999 : 1.0), gamma);
			new_value *= alpha;
		}
		return new_value;
	}
	else if (bits == 16)
	{
		uint64_t index = nx*ny*kk + nx*jj + ii;
		uint16_t old_value = ((uint16_t*)(data_data))[index];
		double gm = 0.0;
		double new_value = double(old_value)*int_scale / 65535.0;
		if (m_vr->get_inversion())
			new_value = 1.0 - new_value;
		if (ii>0 && ii<nx - 1 &&
			jj>0 && jj<ny - 1 &&
			kk>0 && kk<nz - 1)
		{
			double v1 = ((uint8_t*)(data_data))[nx*ny*kk + nx*jj + ii - 1];
			double v2 = ((uint8_t*)(data_data))[nx*ny*kk + nx*jj + ii + 1];
			double v3 = ((uint8_t*)(data_data))[nx*ny*kk + nx*(jj - 1) + ii];
			double v4 = ((uint8_t*)(data_data))[nx*ny*kk + nx*(jj + 1) + ii];
			double v5 = ((uint8_t*)(data_data))[nx*ny*(kk - 1) + nx*jj + ii];
			double v6 = ((uint8_t*)(data_data))[nx*ny*(kk + 1) + nx*jj + ii];
			double normal_x, normal_y, normal_z;
			normal_x = (v2 - v1)*int_scale / 65535.0;
			normal_y = (v4 - v3)*int_scale / 65535.0;
			normal_z = (v6 - v5)*int_scale / 65535.0;
			gm = sqrt(normal_x*normal_x + normal_y*normal_y + normal_z*normal_z)*0.53;
		}
		if (new_value<lo_thresh - soft_thresh ||
			new_value>hi_thresh + soft_thresh)
			new_value = 0.0;
		else
		{
			double gamma = 1.0 / gamma3d;
			new_value = (new_value<lo_thresh ?
				(soft_thresh - lo_thresh + new_value) / soft_thresh :
				(new_value>hi_thresh ?
				(soft_thresh - new_value + hi_thresh) / soft_thresh : 1.0))
				*new_value;
			new_value *= (extract_boundary > 0.0 ?
				Clamp(gm / extract_boundary, 0.0,
					1.0 + extract_boundary*10.0) : 1.0);
			new_value = pow(Clamp(new_value / saturation,
				gamma<1.0 ? -(gamma - 1.0)*0.00001 : 0.0,
				gamma>1.0 ? 0.9999 : 1.0), gamma);
			new_value *= alpha;
		}
		return new_value;
	}

	return 0.0;
}

void VolumeData::SaveData(const std::wstring &filename,
	int mode, bool crop, int filter, bool bake,
	bool compress, fluo::Quaternion &q)
{
	if (!m_vr || !m_tex)
		return;

	VolumeData* temp = 0;
	if (bake)
	{
		flrd::VolumeBaker baker;
		baker.SetInput(temp ? temp : this);
		baker.Bake(temp);
		temp = baker.GetResult();
	}

	bool resize;
	getValue(gstResize, resize);
	long rnx, rny, rnz;
	getValue(gstResizeX, rnx);
	getValue(gstResizeY, rny);
	getValue(gstResizeZ, rnz);
	if (resize || crop)
	{
		flrd::VolumeSampler sampler;
		sampler.SetInput(temp ? temp : this);
		sampler.SetSize(rnx, rny, rnz);
		sampler.SetFilter(filter);
		sampler.SetFilterSize(1, 1, 1);
		sampler.SetCrop(crop);
		sampler.SetClipRotation(q);
		sampler.Resize(flrd::SDT_All, temp);
		temp = sampler.GetResult();
	}

	BaseWriter *writer = 0;
	switch (mode)
	{
	case 0://multi-page tiff
		writer = new TIFWriter();
		break;
	case 1://single-page tiff sequence
		writer = new TIFWriter();
		break;
	case 2://nrrd
		writer = new NRRDWriter();
		break;
	}

	//save data
	Nrrd* data = 0;
	if (temp)
	{
		if (temp->GetTexture())
			data = temp->GetTexture()->get_nrrd(0);
	}
	else
	{
		data = m_tex->get_nrrd(0);
	}
	if (data)
	{
		double spcx, spcy, spcz;
		spcx = data->axis[0].spacing;
		spcy = data->axis[1].spacing;
		spcz = data->axis[2].spacing;
		writer->SetData(data);
		writer->SetSpacings(spcx, spcy, spcz);
		writer->SetCompression(compress);
		writer->Save(filename, mode);
	}
	delete writer;

	if (resize || crop)
	{
		temp->setValue(gstDataPath, filename);
		temp->SaveMask(false, 0, 0);
		temp->SaveLabel(false, 0, 0);
	}
	else
	{
		SaveMask(false, 0, 0);
		SaveLabel(false, 0, 0);
	}

	if (temp)
		glbin_volf->remove(temp);

	setValue(gstDataPath, filename);
}

void VolumeData::SaveMask(bool use_reader, long t, long c)
{
	if (!m_vr || !m_tex)
		return;

	if (t == -1)
		getValue(gstTime, t);
	if (c == -1)
		getValue(gstChannel, c);

	Nrrd* data = 0;
	bool delete_data = false;
	double spcx, spcy, spcz;
	//GetSpacings(spcx, spcy, spcz);
	getValue(gstSpcX, spcx);
	getValue(gstSpcY, spcy);
	getValue(gstSpcZ, spcz);

	//save mask
	if (m_tex->nmask() != -1)
	{
		m_vr->return_mask();
		data = m_tex->get_nrrd(m_tex->nmask());
		if (data)
		{
			//consider doing processing outside of the VolumeData class
            //to reduce coupling
            
/*			bool resize;
			getValue(gstResize, resize);
			long resize_x, resize_y, resize_z;
			getValue(gstResizeX, resize_x);
			getValue(gstResizeY, resize_y);
			getValue(gstResizeZ, resize_z);
			if (resize)
			{
				FL::VolumeSampler sampler;
				sampler.SetVolume(data);
				sampler.SetSize(resize_x, resize_y, resize_z);
				sampler.SetSpacings(spcx, spcy, spcz);
				sampler.SetType(0);
				//sampler.SetFilterSize(2, 2, 0);
				sampler.Resize();
				data = sampler.GetResult();
				if (data)
				{
					spcx = data->axis[0].spacing;
					spcy = data->axis[1].spacing;
					spcz = data->axis[2].spacing;
					delete_data = true;
				}
			}*/

			MSKWriter msk_writer;
			msk_writer.SetData(data);
			msk_writer.SetSpacings(spcx, spcy, spcz);
			std::wstring filename, tex_path;
			getValue(gstDataPath, tex_path);
			if (use_reader && m_reader)
				filename = m_reader->GetCurMaskName(t, c);
			else
				filename = tex_path.substr(0, tex_path.find_last_of('.')) + L".msk";
			msk_writer.Save(filename, 0);
			if (delete_data)
				nrrdNuke(data);
		}
	}
}

void VolumeData::SaveLabel(bool use_reader, long t, long c)
{
	if (!m_vr || !m_tex)
		return;

	if (t == -1)
		getValue(gstTime, t);
	if (c == -1)
		getValue(gstChannel, c);

	Nrrd* data = 0;
	bool delete_data = false;
	double spcx, spcy, spcz;
	//GetSpacings(spcx, spcy, spcz);
	getValue(gstSpcX, spcx);
	getValue(gstSpcY, spcy);
	getValue(gstSpcZ, spcz);

	//save label
	if (m_tex->nlabel() != -1)
	{
		data = m_tex->get_nrrd(m_tex->nlabel());
		if (data)
		{
			//consider doing processing outside of the VolumeData class
            //to reduce coupling
            
/*			bool resize;
			getValue(gstResize, resize);
			long resize_x, resize_y, resize_z;
			getValue(gstResizeX, resize_x);
			getValue(gstResizeY, resize_y);
			getValue(gstResizeZ, resize_z);
			if (resize)
			{
				FL::VolumeSampler sampler;
				sampler.SetVolume(data);
				sampler.SetSize(resize_x, resize_y, resize_z);
				sampler.SetSpacings(spcx, spcy, spcz);
				sampler.Resize();
				data = sampler.GetResult();
				if (data)
				{
					spcx = data->axis[0].spacing;
					spcy = data->axis[1].spacing;
					spcz = data->axis[2].spacing;
					delete_data = true;
				}
			}*/

			MSKWriter msk_writer;
			msk_writer.SetData(data);
			msk_writer.SetSpacings(spcx, spcy, spcz);
			std::wstring filename, tex_path;
			getValue(gstDataPath, tex_path);
			if (use_reader && m_reader)
				filename = m_reader->GetCurLabelName(t, c);
			else
				filename = tex_path.substr(0, tex_path.find_last_of('.')) + L".lbl";
			msk_writer.Save(filename, 1);
			if (delete_data)
				nrrdNuke(data);
		}
	}
}

//volumerenderer
flvr::VolumeRenderer *VolumeData::GetRenderer()
{
	return m_vr;
}

//texture
flvr::Texture* VolumeData::GetTexture()
{
	return m_tex;
}

void VolumeData::SetTexture()
{
	if (m_vr)
		m_vr->reset_texture();
	m_tex = 0;
}

int VolumeData::GetAllBrickNum()
{
	if (!m_tex)
		return 0;
	return m_tex->get_brick_num();
}

void VolumeData::SetMatrices(glm::mat4 &mv_mat,
	glm::mat4 &proj_mat, glm::mat4 &tex_mat)
{
	double sclx, scly, sclz;
	getValue(gstScaleX, sclx);
	getValue(gstScaleY, scly);
	getValue(gstScaleZ, sclz);
	glm::mat4 scale_mv = glm::scale(mv_mat, glm::vec3(sclx, scly, sclz));
	if (m_vr)
		m_vr->set_matrices(scale_mv, proj_mat, tex_mat);
}

//draw volume
void VolumeData::Draw(bool ortho, bool adaptive,
	bool interactive, double zoom, int stream_mode)
{
	bool test_wire;
	getValue(gstTestWiref, test_wire);
	bool draw_bounds;
	getValue(gstDrawBounds, draw_bounds);

	if (m_vr)
	{
		m_vr->draw(test_wire, adaptive, interactive, ortho, stream_mode);
	}
	if (draw_bounds)
		DrawBounds();
}

void VolumeData::DrawBounds()
{
}

void VolumeData::DrawMask(int type, int paint_mode, int hr_mode,
	double ini_thresh, double gm_falloff, double scl_falloff, double scl_translate,
	double w2d, double bins, int order, bool ortho, bool estimate)
{
	if (m_vr)
	{
		unsigned long mask_id, weight1, weight2;
		getValue(gst2dMaskId, mask_id);
		getValue(gst2dWeight1Id, weight1);
		getValue(gst2dWeight2Id, weight2);
		long brick_num;
		getValue(gstBrickNum, brick_num);

		m_vr->set_2d_mask(mask_id);
		m_vr->set_2d_weight(weight1, weight2);
		m_vr->draw_mask(type, paint_mode, hr_mode,
			ini_thresh, gm_falloff, scl_falloff,
			scl_translate, w2d, bins, order,
			ortho, estimate);
		if (brick_num > 1 &&
			(order == 1 || order == 2))
		{
			//invalidate mask
			m_vr->return_mask(order);
			m_vr->clear_tex_mask();
		}
		if (estimate)
		{
			double est_thresh = m_vr->get_estimated_thresh();
            Event estimateThreshEvent;
            estimateThreshEvent.setNotifyFlags(Event::NOTIFY_SELF);
            updateValue(gstEstimateThresh, est_thresh, estimateThreshEvent);
		}
	}
}

void VolumeData::DrawLabel(int type, int mode, double thresh, double gm_falloff)
{
/*	if (m_vr)
		m_vr->draw_label(type, mode, thresh, gm_falloff);*/
}

//calculation
void VolumeData::Calculate(int type, VolumeData *vd_a, VolumeData *vd_b)
{
	if (m_vr)
	{
		double hi_thresh;
		vd_a->getValue(gstHighThreshold, hi_thresh);
		if (type == 6 || type == 7)
			m_vr->set_hi_thresh(hi_thresh);
		m_vr->calculate(type,
			vd_a ? vd_a->GetRenderer() : 0, vd_b ? vd_b->GetRenderer() : 0);
		m_vr->return_volume();
	}
}

Color VolumeData::GetColorFromColormap(double value)
{
	Color color;
	long type;
	getValue(gstColormapType, type);
	double low, high;
	getValue(gstColormapLow, low);
	getValue(gstColormapHigh, high);

	double v = (value - low) / (high - low);
	switch (type)
	{
	case 0:
	default:
		color.r(Clamp(4.0*v - 2.0, 0.0, 1.0));
		color.g(Clamp(v<0.5 ? 4.0*v : -4.0*v + 4.0, 0.0, 1.0));
		color.b(Clamp(-4.0*v + 2.0, 0.0, 1.0));
		break;
	case 1:
		color.r(Clamp(-4.0*v + 2.0, 0.0, 1.0));
		color.g(Clamp(v<0.5 ? 4.0*v : -4.0*v + 4.0, 0.0, 1.0));
		color.b(Clamp(4.0*v - 2.0, 0.0, 1.0));
		break;
	case 2:
		color.r(Clamp(2.0*v, 0.0, 1.0));
		color.g(Clamp(4.0*v - 2.0, 0.0, 1.0));
		color.b(Clamp(4.0*v - 3.0, 0.0, 1.0));
		break;
	case 3:
		color.r(Clamp(v, 0.0, 1.0));
		color.g(Clamp(1.0 - v, 0.0, 1.0));
		color.b(1.0);
		break;
	case 4:
		color.r(Clamp(v<0.5 ? v*0.9 + 0.25 : 0.7, 0.0, 1.0));
		color.g(Clamp(v<0.5 ? v*0.8 + 0.3 : (1.0 - v)*1.4, 0.0, 1.0));
		color.b(Clamp(v<0.5 ? v*(-0.1) + 0.75 : (1.0 - v)*1.1 + 0.15, 0.0, 1.0));
		break;
	case 5:
		color.r(Clamp(v, 0.0, 1.0));
		color.g(Clamp(v, 0.0, 1.0));
		color.b(Clamp(v, 0.0, 1.0));
		break;
	case 6:
		color.r(1.0 - Clamp(v, 0.0, 1.0));
		color.g(1.0 - Clamp(v, 0.0, 1.0));
		color.b(1.0 - Clamp(v, 0.0, 1.0));
		break;
	}
	return color;
}

void VolumeData::SetShuffle(int val)
{
	if (m_vr)
		m_vr->set_shuffle(val);
}

int VolumeData::GetShuffle()
{
	if (m_vr)
		return m_vr->get_shuffle();
	else
		return 0;
}

void VolumeData::IncShuffle()
{
	if (!m_vr)
		return;
	int ival = m_vr->get_shuffle();
	++ival;
	ival = ival >= 5 ? 0 : ival;
	m_vr->set_shuffle(ival);
}

void VolumeData::SetOrderedID(unsigned int* val)
{
	long resx, resy, resz;
	getValue(gstResX, resx);
	getValue(gstResY, resy);
	getValue(gstResZ, resz);
	unsigned long long mem_size = (unsigned long long)resx*
		(unsigned long long)resy*(unsigned long long)resz;
	for (unsigned long long index = 0;
		index < mem_size; ++index)
		val[index] = index + 1;
}

void VolumeData::SetReverseID(unsigned int* val)
{
	long resx, resy, resz;
	getValue(gstResX, resx);
	getValue(gstResY, resy);
	getValue(gstResZ, resz);
	unsigned long long mem_size = (unsigned long long)resx*
		(unsigned long long)resy*(unsigned long long)resz;
	for (unsigned long long index = 0;
		index < mem_size; ++index)
		val[index] = mem_size - index;
}

void VolumeData::SetShuffledID(unsigned int* val)
{
/*	long resx, resy, resz;
	getValue(gstResX, resx);
	getValue(gstResY, resy);
	getValue(gstResZ, resz);

	unsigned int x, y, z;
	unsigned int res;
	unsigned int len = 0;
    unsigned int r = Max(resx, Max(resy, resz));
	while (r > 0)
	{
		r /= 2;
		len++;
	}
	for (int i = 0; i<resx; i++)
	for (int j = 0; j<resy; j++)
	for (int k = 0; k<resz; k++)
	{
		x = reverse_bit(i, len);
		y = reverse_bit(j, len);
		z = reverse_bit(k, len);
		res = 0;
		for (unsigned int ii = 0; ii<len; ii++)
		{
			res |= (1 << ii & x) << (2 * ii);
			res |= (1 << ii & y) << (2 * ii + 1);
			res |= (1 << ii & z) << (2 * ii + 2);
		}
		unsigned int index = resx*resy*k + resx*j + i;
		val[index] = resx*resy*resz - res;
	}*/
}

