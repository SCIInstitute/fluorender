/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2018 Scientific Computing and Imaging Institute,
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

#include <Scenegraph/VolumeData.h>
#include <FLIVR/VolumeRenderer.h>
#include <Formats/base_reader.h>
#include <Formats/oib_reader.h>
#include <Formats/oif_reader.h>
#include <Formats/nrrd_reader.h>
#include <Formats/tif_reader.h>
#include <Formats/nrrd_writer.h>
#include <Formats/tif_writer.h>
#include <Formats/msk_reader.h>
#include <Formats/msk_writer.h>
#include <Formats/lsm_reader.h>
#include <Formats/lbl_reader.h>
#include <Formats/pvxml_reader.h>
#include <Formats/brkxml_reader.h>
#include <Formats/imageJ_reader.h>
#include <FLIVR/Plane.h>
//#include <Calculate/VolumeSampler.h>
#include <glm/gtc/matrix_transform.hpp>

using namespace FL;

VolumeData::VolumeData() :
	m_vr(0),
	m_tex(0),
	m_reader(0)
{
}

VolumeData::VolumeData(const VolumeData& data, const CopyOp& copyop, bool copy_values):
	Node(data, copyop, false),
	m_vr(0),
	m_tex(0),
	m_reader(0)
{
	//volume renderer and texture
	//if (data.m_vr)
	//	m_vr = new FLIVR::VolumeRenderer(*data.m_vr);
	//else
	//	m_vr = new FLIVR::VolumeRenderer();
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
//	getValue("mip mode", mode);
//	setValue("saved mode", mode);
//}

//MIP & normal modes
void VolumeData::OnMipModeChanged(Event& event)
{
	if (!m_vr)
		return;

	long mode;
	getValue("mip mode", mode);

	switch (mode)
	{
	case 0://normal
		m_vr->set_mode(FLIVR::TextureRenderer::MODE_OVER);
		break;
	case 1://MIP
		m_vr->set_mode(FLIVR::TextureRenderer::MODE_MIP);
		break;
	}
}

void VolumeData::OnOverlayModeChanged(Event& event)
{
	if (!m_vr)
		return;

	long mode;
	getValue("overlay mode", mode);

	switch (mode)
	{
	case 0://restore
	{
		FLTYPE::Color color;
		getValue("color", color);
		m_vr->set_color(color);
		double alpha;
		getValue("alpha", alpha);
		m_vr->set_alpha(alpha);
		bool shading_enable;
		getValue("shading enable", shading_enable);
		m_vr->set_shading(shading_enable);
		bool colormap_enable;
		getValue("colormap enable", colormap_enable);
		m_vr->set_colormap_mode(colormap_enable?1:0);
		long mask_mode;
		getValue("mask mode", mask_mode);
		m_vr->set_ml_mode(mask_mode);
		long mip_mode;
		getValue("mip mode", mip_mode);
		if (mip_mode == 1)
			m_vr->set_mode(FLIVR::TextureRenderer::MODE_MIP);
		else
			m_vr->set_mode(FLIVR::TextureRenderer::MODE_OVER);
		bool alpha_enable;
		getValue("alpha enable", alpha_enable);
		m_vr->set_solid(!alpha_enable);
	}
	break;
	case 1://for shadow
	{
		m_vr->set_mode(FLIVR::TextureRenderer::MODE_OVER);
		m_vr->set_shading(false);
		m_vr->set_solid(false);
		m_vr->set_colormap_mode(2);
		m_vr->set_ml_mode(0);
	}
	break;
	case 2://for shading
	{
		m_vr->set_mode(FLIVR::TextureRenderer::MODE_OVER);
		m_vr->set_shading(true);
		m_vr->set_solid(false);
		m_vr->set_alpha(1.0);
		m_vr->set_colormap_mode(0);
		m_vr->set_color(FLIVR::Color(1.0));
	}
	break;
	case 3://white mip
	{
		bool colormap_enable;
		getValue("colormap enable", colormap_enable);
		if (colormap_enable)
		{
			m_vr->set_mode(FLIVR::TextureRenderer::MODE_MIP);
			m_vr->set_colormap_mode(0);
			m_vr->set_color(FLIVR::Color(1.0));
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

	FLTYPE::GLint4 vp;
	getValue("viewport", vp);

	m_vr->set_viewport(vp.get());
}

void VolumeData::OnClearColorChanged(Event& event)
{
	if (!m_vr)
		return;

	FLTYPE::GLfloat4 clear_color;
	getValue("clear color", clear_color);
	m_vr->set_clear_color(clear_color.get());
}

void VolumeData::OnCurFramebufferChanged(Event& event)
{
	if (!m_vr)
		return;

	unsigned long cur_framebuffer;
	getValue("cur framebuffer", cur_framebuffer);
	m_vr->set_cur_framebuffer(cur_framebuffer);
}

void VolumeData::OnCompressionChanged(Event& event)
{
	if (!m_vr)
		return;

	bool compression;
	getValue("compression", compression);
	m_vr->set_compression(compression);
}

void VolumeData::OnInvertChanged(Event& event)
{
	if (!m_vr)
		return;

	bool invert;
	getValue("invert", invert);
	m_vr->set_inversion(invert);
}

void VolumeData::OnMaskModeChanged(Event& event)
{
	if (!m_vr)
		return;

	long mask_mode;
	getValue("mask mode", mask_mode);
	m_vr->set_ml_mode(mask_mode);
}

void VolumeData::OnNoiseRedctChanged(Event& event)
{
	if (!m_vr)
		return;

	bool noise_redct;
	getValue("noise redct", noise_redct);
	m_vr->SetNoiseRed(noise_redct);
}

void VolumeData::On2dDmapIdChanged(Event& event)
{
	if (!m_vr)
		return;

	unsigned long dmap_id;
	getValue("2d dmap id", dmap_id);
	m_vr->set_2d_dmap(dmap_id);
}

void VolumeData::OnGamma3dChanged(Event& event)
{
	if (!m_vr)
		return;

	double gamma_3d;
	getValue("gamma 3d", gamma_3d);
	m_vr->set_gamma3d(gamma_3d);
}

void VolumeData::OnExtractBoundaryChanged(Event& event)
{
	if (!m_vr)
		return;

	double extract_boundary;
	getValue("extract boundary", extract_boundary);
	m_vr->set_gm_thresh(extract_boundary);
}

void VolumeData::OnSaturationChanged(Event& event)
{
	if (!m_vr)
		return;

	double saturation;
	getValue("saturation", saturation);
	m_vr->set_offset(saturation);
}

void VolumeData::OnLowThresholdChanged(Event& event)
{
	if (!m_vr)
		return;

	double low_threshold;
	getValue("low threshold", low_threshold);
	m_vr->set_lo_thresh(low_threshold);
}

void VolumeData::OnHighThresholdChanged(Event& event)
{
	if (!m_vr)
		return;

	double high_threshold;
	getValue("high threshold", high_threshold);
	m_vr->set_hi_thresh(high_threshold);
}

void VolumeData::OnColorChanged(Event& event)
{
	if (!m_vr)
		return;
	FLTYPE::Color color;
	getValue("color", color);
	m_vr->set_color(color);
	FLTYPE::HSVColor hsv(color);
	event.setNotifyFlags(Event::NOTIFY_SELF);
	setValue("hsv", hsv, event);
	event.setNotifyFlags(Event::NOTIFY_SELF | Event::NOTIFY_AGENT);
	setValue("luminance", hsv.val(), event);
}

void VolumeData::OnSecColorChanged(Event& event)
{
	if (!m_vr)
		return;

	FLTYPE::Color sec_color;
	getValue("sec color", sec_color);
	m_vr->set_mask_color(sec_color);
	event.setNotifyFlags(Event::NOTIFY_SELF);
	setValue("sec color set", bool(true), event);
}

void VolumeData::OnSecColorSetChanged(Event& event)
{
	if (!m_vr)
		return;

	bool sec_color_set;
	getValue("sec color set", sec_color_set);
	if (!sec_color_set)
		m_vr->reset_mask_color_set();
}

void VolumeData::OnLuminanceChanged(Event& event)
{
	if (!m_vr)
		return;

	double luminance;
	getValue("luminance", luminance);
	FLTYPE::HSVColor hsv;
	getValue("hsv", hsv);
	hsv.val(luminance);
	FLTYPE::Color color(hsv);
	event.setNotifyFlags(Event::NOTIFY_SELF);
	setValue("color", color, event);
	m_vr->set_color(color);
}

void VolumeData::OnAlphaChanged(Event& event)
{
	if (!m_vr)
		return;

	double alpha;
	getValue("alpha", alpha);
	m_vr->set_alpha(alpha);
}

void VolumeData::OnAlphaEnableChanged(Event& event)
{
	if (!m_vr)
		return;

	bool alpha_enable;
	getValue("alpha enable", alpha_enable);
	m_vr->set_solid(!alpha_enable);
	if (alpha_enable)
	{
		double alpha;
		getValue("alpha", alpha);
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
	getValue("mask thresh", mask_thresh);
	m_vr->set_mask_thresh(mask_thresh);
}

void VolumeData::OnUseMaskThreshChanged(Event& event)
{
	if (!m_vr)
		return;

	bool use_mask_thresh;
	getValue("use mask thresh", use_mask_thresh);
	if (!use_mask_thresh)
		m_vr->set_mask_thresh(0.0);
}

void VolumeData::OnShadingEnableChanged(Event& event)
{
	if (!m_vr)
		return;

	bool shading_enable;
	getValue("shading enable", shading_enable);
	m_vr->set_shading(shading_enable);
}

void VolumeData::OnMaterialChanged(Event& event)
{
	if (!m_vr)
		return;

	double mat_amb, mat_diff, mat_spec, mat_shine;
	getValue("mat amb", mat_amb);
	getValue("mat diff", mat_diff);
	getValue("mat spec", mat_spec);
	getValue("mat shine", mat_shine);
	m_vr->set_material(mat_amb, mat_diff, mat_spec, mat_shine);
}

void VolumeData::OnLowShadingChanged(Event& event)
{
	double low_shading;
	getValue("low shading", low_shading);
	event.setNotifyFlags(Event::NOTIFY_SELF);
	setValue("mat amb", low_shading, event);
}

void VolumeData::OnHighShadingChanged(Event& event)
{
	double high_shading;
	getValue("high shading", high_shading);
	event.setNotifyFlags(Event::NOTIFY_SELF);
	setValue("mat shine", high_shading, event);
}

void VolumeData::OnSampleRateChanged(Event& event)
{
	if (!m_vr)
		return;

	double sample_rate;
	getValue("sample rate", sample_rate);
	m_vr->set_sampling_rate(sample_rate);
}

void VolumeData::OnColormapModeChanged(Event& event)
{
	if (!m_vr)
		return;

	long colormap_mode;
	getValue("colormap mode", colormap_mode);
	FLTYPE::Color color;
	getValue("color", color);
	m_vr->set_colormap_mode(colormap_mode);
	m_vr->set_color(color);
}

void VolumeData::OnColormapValueChanged(Event& event)
{
	if (!m_vr)
		return;

	double colormap_low, colormap_high;
	getValue("colormap low", colormap_low);
	getValue("colormap high", colormap_high);
	m_vr->set_colormap_values(colormap_low, colormap_high);
}

void VolumeData::OnColormapTypeChanged(Event& event)
{
	if (!m_vr)
		return;
	long colormap_type;
	getValue("colormap type", colormap_type);
	m_vr->set_colormap(colormap_type);
}

void VolumeData::OnColormapProjChanged(Event& event)
{
	if (!m_vr)
		return;

	long colormap_proj;
	getValue("colormap proj", colormap_proj);
	m_vr->set_colormap_proj(colormap_proj);
}

void VolumeData::OnSpacingChanged(Event& event)
{
	if (!m_tex)
		return;
	double spc_x, spc_y, spc_z;
	getValue("spc x", spc_x);
	getValue("spc y", spc_y);
	getValue("spc z", spc_z);
	m_tex->set_spacings(spc_x, spc_y, spc_z);
	FLIVR::BBox bbox;
	m_tex->get_bounds(bbox);
	FLTYPE::Point box_min(bbox.min().x(), bbox.min().y(), bbox.min().z());
	FLTYPE::Point box_max(bbox.max().x(), bbox.max().y(), bbox.max().z());
	FLTYPE::BBox bounds(box_min, box_max);
	event.setNotifyFlags(Event::NOTIFY_PARENT | Event::NOTIFY_AGENT);
	setValue("bounds", bounds, event);

	//m_tex->get_base_spacings(spc_x, spc_y, spc_z);
	event.setNotifyFlags(Event::NOTIFY_SELF);
	setValue("base spc x", spc_x, event);
	setValue("base spc y", spc_y, event);
	setValue("base spc z", spc_z, event);

	//tex transform
	FLIVR::Transform *temp = m_tex->transform();
	FLTYPE::Transform tform;
	double tformarray[16];
	temp->get(tformarray);
	tform.set(tformarray);
	setValue("tex transform", tform, Event(Event::NOTIFY_NONE));
}

void VolumeData::OnBaseSpacingChanged(Event& event)
{
	if (!m_tex)
		return;
	double spc_x, spc_y, spc_z;
	getValue("base spc x", spc_x);
	getValue("base spc y", spc_y);
	getValue("base spc z", spc_z);
	m_tex->set_base_spacings(spc_x, spc_y, spc_z);

	long level;
	getValue("level", level);
	m_tex->get_spacings(spc_x, spc_y, spc_z, level);
	event.setNotifyFlags(Event::NOTIFY_SELF);
	setValue("spc x", spc_x, event);
	setValue("spc y", spc_y, event);
	setValue("spc z", spc_z, event);

	//tex transform
	FLIVR::Transform *temp = m_tex->transform();
	FLTYPE::Transform tform;
	double tformarray[16];
	temp->get(tformarray);
	tform.set(tformarray);
	setValue("tex transform", tform, Event(Event::NOTIFY_NONE));
}

void VolumeData::OnSpacingScaleChanged(Event& event)
{
	if (!m_tex)
		return;
	double spc_x, spc_y, spc_z;
	getValue("spc scl x", spc_x);
	getValue("spc scl y", spc_y);
	getValue("spc scl z", spc_z);
	m_tex->set_spacing_scales(spc_x, spc_y, spc_z);
}

void VolumeData::OnLevelChanged(Event& event)
{
	if (!m_tex)
		return;
	bool multires = false;
	getValue("multires", multires);
	if (!multires)
		return;
	long level;
	getValue("level", level);
	m_tex->setLevel(level);
	FLIVR::BBox bbox;
	m_tex->get_bounds(bbox);
	FLTYPE::Point box_min(bbox.min().x(), bbox.min().y(), bbox.min().z());
	FLTYPE::Point box_max(bbox.max().x(), bbox.max().y(), bbox.max().z());
	FLTYPE::BBox bounds(box_min, box_max);
	event.setNotifyFlags(Event::NOTIFY_SELF);
	setValue("bounds", bounds, event);

	//tex transform
	FLIVR::Transform *temp = m_tex->transform();
	FLTYPE::Transform tform;
	double tformarray[16];
	temp->get(tformarray);
	tform.set(tformarray);
	setValue("tex transform", tform, Event(Event::NOTIFY_NONE));
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
	getValue("interpolate", interpolate);
	m_vr->set_interpolate(interpolate);
}

void VolumeData::OnDepthAttenChanged(Event& event)
{
	if (!m_vr)
		return;

	bool depth_atten;
	double da_int, da_start, da_end;
	getValue("depth atten", depth_atten);
	getValue("da int", da_int);
	getValue("da start", da_start);
	getValue("da end", da_end);
	m_vr->set_fog(depth_atten, da_int, da_start, da_end);
}

void VolumeData::OnSkipBrickChanged(Event& event)
{
	if (!m_tex)
		return;
	bool skip_brick;
	getValue("skip brick", skip_brick);
	m_tex->set_use_priority(skip_brick);
}

void VolumeData::OnClipPlanesChanged(Event& event)
{
	if (!m_vr)
		return;

	FLTYPE::PlaneSet planeset;
	getValue("clip planes", planeset);

	//this needs to be made consistent in the future
	std::vector<FLIVR::Plane*> planelist(0);
	for (int i = 0; i < planeset.GetSize(); ++i)
	{
		FLIVR::Plane* plane = new FLIVR::Plane(planeset.Get(i));
		planelist.push_back(plane);
	}
	m_vr->set_planes(&planelist);
}

void VolumeData::OnIntScaleChanged(Event& event)
{
	if (!m_vr)
		return;

	double int_scale;
	getValue("int scale", int_scale);
	m_vr->set_scalar_scale(int_scale);
	m_vr->set_gm_scale(int_scale);
}

void VolumeData::OnSyncOutputChannels(Event& event)
{
	ValueCollection ss_gamma = {
		"gamma r",
		"gamma g",
		"gamma b"
	};
	ValueCollection ss_brigt = {
		"brightness r",
		"brightness g",
		"brightness b"
	};
	ValueCollection ss_equal = {
		"equalize r",
		"equalize g",
		"equalize b"
	};
	bool sync_r, sync_g, sync_b;
	getValue("sync r", sync_r);
	getValue("sync g", sync_g);
	getValue("sync b", sync_b);

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
		ss_gamma.insert("gamma r");
		ss_brigt.insert("brightness r");
		ss_equal.insert("equalize r");
	}
	if (sync_g)
	{
		ss_gamma.insert("gamma g");
		ss_brigt.insert("brightness g");
		ss_equal.insert("equalize g");
	}
	if (sync_b)
	{
		ss_gamma.insert("gamma b");
		ss_brigt.insert("brightness b");
		ss_equal.insert("equalize b");
	}
	syncValues(ss_gamma);
	syncValues(ss_brigt);
	syncValues(ss_equal);
}

void VolumeData::OnClipX1Changed(Event& event)
{
	bool clip_link;
	getValue("clip link x", clip_link);
	double value1, value2;
	double clip_dist;
	if (clip_link)
	{
		getValue("clip dist x", clip_dist);
		getValue("clip x1", value1);
		value2 = value1 + clip_dist;
		if (value2 > 1.0)
		{
			setValue("clip x1", 1.0 - clip_dist, event);
			setValue("clip x2", 1.0, event);
		}
		else
			setValue("clip x2", value2, event);
	}
	else
	{
		getValue("clip x1", value1);
		getValue("clip x2", value2);
		if (value1 >= value2)
		{
			long resx;
			getValue("res x", resx);
			value1 = value2 - 1.0 / resx;
			setValue("clip x1", value1);
		}
		clip_dist = value2 - value1;
		setValue("clip dist x", clip_dist, event);
	}

	UpdateClippingPlanes(event);
}

void VolumeData::OnClipX2Changed(Event& event)
{
	bool clip_link;
	getValue("clip link x", clip_link);
	double value1, value2;
	double clip_dist;
	if (clip_link)
	{
		getValue("clip dist x", clip_dist);
		getValue("clip x2", value2);
		value1 = value2 - clip_dist;
		if (value1 < 0.0)
		{
			setValue("clip x1", 0.0, event);
			setValue("clip x2", clip_dist, event);
		}
		else
			setValue("clip x1", value1, event);
	}
	else
	{
		getValue("clip x1", value1);
		getValue("clip x2", value2);
		if (value1 >= value2)
		{
			long resx;
			getValue("res x", resx);
			value2 = value1 + 1.0 / resx;
			setValue("clip x2", value2);
		}
		clip_dist = value2 - value1;
		setValue("clip dist x", clip_dist, event);
	}

	UpdateClippingPlanes(event);
}

void VolumeData::OnClipY1Changed(Event& event)
{
	bool clip_link;
	getValue("clip link y", clip_link);
	double value1, value2;
	double clip_dist;
	if (clip_link)
	{
		getValue("clip dist y", clip_dist);
		getValue("clip y1", value1);
		value2 = value1 + clip_dist;
		if (value2 > 1.0)
		{
			setValue("clip y1", 1.0 - clip_dist, event);
			setValue("clip y2", 1.0, event);
		}
		else
			setValue("clip y2", value2, event);
	}
	else
	{
		getValue("clip y1", value1);
		getValue("clip y2", value2);
		if (value1 >= value2)
		{
			long resy;
			getValue("res y", resy);
			value1 = value2 - 1.0 / resy;
			setValue("clip y1", value1);
		}
		clip_dist = value2 - value1;
		setValue("clip dist y", clip_dist, event);
	}

	UpdateClippingPlanes(event);
}

void VolumeData::OnClipY2Changed(Event& event)
{
	bool clip_link;
	getValue("clip link y", clip_link);
	double value1, value2;
	double clip_dist;
	if (clip_link)
	{
		getValue("clip dist y", clip_dist);
		getValue("clip y2", value2);
		value1 = value2 - clip_dist;
		if (value1 < 0.0)
		{
			setValue("clip y1", 0.0, event);
			setValue("clip y2", clip_dist, event);
		}
		else
			setValue("clip y1", value1, event);
	}
	else
	{
		getValue("clip y1", value1);
		getValue("clip y2", value2);
		if (value1 >= value2)
		{
			long resy;
			getValue("res y", resy);
			value2 = value1 + 1.0 / resy;
			setValue("clip y2", value2);
		}
		clip_dist = value2 - value1;
		setValue("clip dist y", clip_dist, event);
	}

	UpdateClippingPlanes(event);
}

void VolumeData::OnClipZ1Changed(Event& event)
{
	bool clip_link;
	getValue("clip link z", clip_link);
	double value1, value2;
	double clip_dist;
	if (clip_link)
	{
		getValue("clip dist z", clip_dist);
		getValue("clip z1", value1);
		value2 = value1 + clip_dist;
		if (value2 > 1.0)
		{
			setValue("clip z1", 1.0 - clip_dist, event);
			setValue("clip z2", 1.0, event);
		}
		else
			setValue("clip z2", value2, event);
	}
	else
	{
		getValue("clip z1", value1);
		getValue("clip z2", value2);
		if (value1 >= value2)
		{
			long resz;
			getValue("res z", resz);
			value1 = value2 - 1.0 / resz;
			setValue("clip z1", value1);
		}
		clip_dist = value2 - value1;
		setValue("clip dist z", clip_dist, event);
	}

	UpdateClippingPlanes(event);
}

void VolumeData::OnClipZ2Changed(Event& event)
{
	bool clip_link;
	getValue("clip link z", clip_link);
	double value1, value2;
	double clip_dist;
	if (clip_link)
	{
		getValue("clip dist z", clip_dist);
		getValue("clip z2", value2);
		value1 = value2 - clip_dist;
		if (value1 < 0.0)
		{
			setValue("clip z1", 0.0, event);
			setValue("clip z2", clip_dist, event);
		}
		else
			setValue("clip z1", value1, event);
	}
	else
	{
		getValue("clip z1", value1);
		getValue("clip z2", value2);
		if (value1 >= value2)
		{
			long resz;
			getValue("res z", resz);
			value2 = value1 + 1.0 / resz;
			setValue("clip z2", value2);
		}
		clip_dist = value2 - value1;
		setValue("clip dist z", clip_dist, event);
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
	getValue("clip x1", clip_x1);
	getValue("clip x2", clip_x2);
	getValue("clip y1", clip_y1);
	getValue("clip y2", clip_y2);
	getValue("clip z1", clip_z1);
	getValue("clip z2", clip_z2);
	double clip_rot_x, clip_rot_y, clip_rot_z;
	getValue("clip rot x", clip_rot_x);
	getValue("clip rot y", clip_rot_y);
	getValue("clip rot z", clip_rot_z);
	double spc_x, spc_y, spc_z;
	long res_x, res_y, res_z;
	getValue("spc x", spc_x);
	getValue("spc y", spc_y);
	getValue("spc z", spc_z);
	getValue("res x", res_x);
	getValue("res y", res_y);
	getValue("res z", res_z);

	FLTYPE::Quaternion cl_quat;
	cl_quat.FromEuler(clip_rot_x, clip_rot_y, clip_rot_z);
	FLTYPE::Vector scale;
	if (spc_x > 0.0 && spc_y > 0.0 && spc_z > 0.0)
	{
		scale = FLTYPE::Vector(
			1.0 / res_x / spc_x,
			1.0 / res_y / spc_y,
			1.0 / res_z / spc_z);
		scale.safe_normalize();
	}
	else
		scale = FLTYPE::Vector(1.0);

	FLTYPE::PlaneSet planes(6);
	//clip first
	planes[0].ChangePlane(FLTYPE::Point(clip_x1, 0, 0), FLTYPE::Vector(1, 0, 0));
	planes[1].ChangePlane(FLTYPE::Point(clip_x2, 0, 0), FLTYPE::Vector(-1, 0, 0));
	planes[2].ChangePlane(FLTYPE::Point(0, clip_y1, 0), FLTYPE::Vector(0, 1, 0));
	planes[3].ChangePlane(FLTYPE::Point(0, clip_y2, 0), FLTYPE::Vector(0, -1, 0));
	planes[4].ChangePlane(FLTYPE::Point(0, 0, clip_z1), FLTYPE::Vector(0, 0, 1));
	planes[5].ChangePlane(FLTYPE::Point(0, 0, clip_z2), FLTYPE::Vector(0, 0, -1));
	//then rotate
	FLTYPE::Vector trans1(-0.5);
	FLTYPE::Vector trans2(0.5);
	planes.Translate(trans1);
	planes.Rotate(cl_quat);
	planes.Scale(scale);
	planes.Translate(trans2);
	//set and update to the renderer
	setValue("clip planes", planes, event);
}

//randomize color
void VolumeData::OnRandomizeColor(Event& event)
{
	double hue = (double)rand() / (RAND_MAX) * 360.0;
	FLTYPE::Color color(FLTYPE::HSVColor(hue, 1.0, 1.0));
	setValue("color", color, event);
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

	setValue("data path", path);
	setName(name);

	if (m_tex)
	{
		delete m_tex;
		m_tex = NULL;
	}

	Nrrd *nv = data;

	m_tex = new FLIVR::Texture();
	//bool skip_brick;
	//getValue("skip brick", skip_brick);
	//m_tex->set_use_priority(skip_brick);

	if (m_reader && m_reader->GetType() == READER_BRKXML_TYPE)
	{
		BRKXMLReader *breader = (BRKXMLReader*)m_reader;
		vector<FLIVR::Pyramid_Level> pyramid;
		vector<vector<vector<vector<FLIVR::FileLocInfo *>>>> fnames;
		int ftype = BRICK_FILE_TYPE_NONE;

		breader->build_pyramid(pyramid, fnames, 0, breader->GetCurChan());
		m_tex->SetCopyableLevel(breader->GetCopyableLevel());

		int lmnum = breader->GetLandmarkNum();
		for (int j = 0; j < lmnum; j++)
		{
			wstring name;
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

		FLTYPE::PlaneSet planeset;
		getValue("clip planes", planeset);
		//this needs to be made consistent in the future
		vector<FLIVR::Plane*> planelist(0);
		for (int i = 0; i < planeset.GetSize(); ++i)
		{
			FLIVR::Plane* plane = new FLIVR::Plane(planeset.Get(i));
			planelist.push_back(plane);
		}

		m_vr = new FLIVR::VolumeRenderer(m_tex, planelist, true);
		FLTYPE::BBox bounds;
		FLTYPE::Point pmax(data->axis[0].max, data->axis[1].max, data->axis[2].max);
		FLTYPE::Point pmin(data->axis[0].min, data->axis[1].min, data->axis[2].min);
		bounds.extend(pmin);
		bounds.extend(pmax);
		setValue("bounds", bounds);
		setValue("res x", long(nv->axis[0].size));
		setValue("res y", long(nv->axis[1].size));
		setValue("res z", long(nv->axis[2].size));
		Initialize();

		//double sample_rate;
		//getValue("sample rate", sample_rate);
		//m_vr->set_sampling_rate(sample_rate);
		//double mat_amb, mat_diff, mat_spec, mat_shine;
		//getValue("mat amb", mat_amb);
		//getValue("mat diff", mat_diff);
		//getValue("mat spec", mat_spec);
		//getValue("mat shine", mat_shine);
		//m_vr->set_material(mat_amb, mat_diff, mat_spec, mat_shine);
		//bool shading;
		//getValue("shading enable", shading);
		//m_vr->set_shading(shading);
		//double int_scale;
		//getValue("int scale", int_scale);
		//m_vr->set_scalar_scale(int_scale);
		//m_vr->set_gm_scale(int_scale);

		//OnMipModeChanged();


		//bool depth_atten;
		//double da_int, da_start, da_end;
		//getValue("depth atten", depth_atten);
		//getValue("da int", da_int);
		//getValue("da start", da_start);
		//getValue("da end", da_end);
		//m_vr->set_fog(false, da_int, da_start, da_end);

		//double high_threshold;
		//getValue("high threshold", high_threshold);
		//m_vr->set_hi_thresh(high_threshold);
		//
		//double spcx, spcy, spcz;
		//getValue("spc x", spcx);
		//getValue("spc y", spcy);
		//getValue("spc z", spcz);
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
		setValue("res x", long(nv->axis[0].size));
		setValue("res y", long(nv->axis[1].size));
		setValue("res z", long(nv->axis[2].size));

		if (m_tex)
			delete m_tex;
		m_tex = new FLIVR::Texture();
		bool skip_brick;
		getValue("skip brick", skip_brick);
		m_tex->set_use_priority(skip_brick);
		double max_int;
		getValue("max int", max_int);
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
	getValue("res x", resx);
	getValue("res y", resy);
	getValue("res z", resz);
	data->getValue("res x", resx2);
	data->getValue("res y", resy2);
	data->getValue("res z", resz2);
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
	ValueCollection names;
	names.insert("int scale");
	names.insert("gm scale");
	names.insert("max int");
	data->propValues(names, this);
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
		uint8 *val8 = new (std::nothrow) uint8[mem_size];
		if (!val8)
		{
			//wxMessageBox("Not enough memory. Please save project and restart.");
			return;
		}
		memset((void*)val8, 0, sizeof(uint8)*nx*ny*nz);
		nrrdWrap(nv, val8, nrrdTypeUChar, 3, (size_t)nx, (size_t)ny, (size_t)nz);
		setValue("bits", long(8));
	}
	else if (bits == 16)
	{
		unsigned long long mem_size = (unsigned long long)nx*
			(unsigned long long)ny*(unsigned long long)nz;
		uint16 *val16 = new (std::nothrow) uint16[mem_size];
		if (!val16)
		{
			//wxMessageBox("Not enough memory. Please save project and restart.");
			return;
		}
		memset((void*)val16, 0, sizeof(uint16)*nx*ny*nz);
		nrrdWrap(nv, val16, nrrdTypeUShort, 3, (size_t)nx, (size_t)ny, (size_t)nz);
		setValue("bits", long(16));
	}
	nrrdAxisInfoSet(nv, nrrdAxisInfoSpacing, spcx, spcy, spcz);
	nrrdAxisInfoSet(nv, nrrdAxisInfoMax, spcx*nx, spcy*ny, spcz*nz);
	nrrdAxisInfoSet(nv, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
	nrrdAxisInfoSet(nv, nrrdAxisInfoSize, (size_t)nx, (size_t)ny, (size_t)nz);

	//resolution
	setValue("res x", long(nv->axis[0].size));
	setValue("res y", long(nv->axis[1].size));
	setValue("res z", long(nv->axis[2].size));

	//bounding box
	FLTYPE::BBox bounds;
	FLTYPE::Point pmax(nv->axis[0].max, nv->axis[1].max, nv->axis[2].max);
	FLTYPE::Point pmin(nv->axis[0].min, nv->axis[1].min, nv->axis[2].min);
	bounds.extend(pmin);
	bounds.extend(pmax);
	setValue("bounds", bounds);

	//create texture
	m_tex = new FLIVR::Texture();
	m_tex->set_use_priority(false);
	m_tex->set_brick_size(brick_size);
	m_tex->build(nv, 0, 0, 256, 0, 0);
	m_tex->set_spacings(spcx, spcy, spcz);

	//clipping planes
	//this needs to be made consistent in the future
	FLTYPE::PlaneSet planeset;
	getValue("clip planes", planeset);
	vector<FLIVR::Plane*> planelist(0);
	for (int i = 0; i < planeset.GetSize(); ++i)
	{
		FLIVR::Plane* plane = new FLIVR::Plane(planeset.Get(0));
		planelist.push_back(plane);
	}

	//create volume renderer
	m_vr = new FLIVR::VolumeRenderer(m_tex, planelist, true);
	double sample_rate;
	getValue("sample rate", sample_rate);
	m_vr->set_sampling_rate(sample_rate);
	double mat_amb, mat_diff, mat_spec, mat_shine;
	getValue("mat amb", mat_amb);
	getValue("mat diff", mat_diff);
	getValue("mat spec", mat_spec);
	getValue("mat shine", mat_shine);
	m_vr->set_material(mat_amb, mat_diff, mat_spec, mat_shine);
	bool shading;
	getValue("shading enable", shading);
	m_vr->set_shading(shading);
	double int_scale;
	getValue("int scale", int_scale);
	m_vr->set_scalar_scale(int_scale);
	m_vr->set_gm_scale(int_scale);

	OnMipModeChanged(Event());
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
	getValue("res x", resx);
	getValue("res y", resy);
	getValue("res z", resz);
	if (resx != nx2 || resy != ny2 || resz != nz2)
	{
		double spcx, spcy, spcz;
		//GetSpacings(spcx, spcy, spcz);
		getValue("spc x", spcx);
		getValue("spc y", spcy);
		getValue("spc z", spcz);
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

void VolumeData::AddEmptyMask(int mode)
{
	if (!m_tex || !m_vr)
		return;

	Nrrd *nrrd_mask = 0;
	uint8 *val8 = 0;
	long resx, resy, resz;
	getValue("res x", resx);
	getValue("res y", resy);
	getValue("res z", resz);
	unsigned long long mem_size = (unsigned long long)resx*
		(unsigned long long)resy*(unsigned long long)resz;
	//prepare the texture bricks for the mask
	bool empty = m_tex->add_empty_mask();
	if (empty)
	{
		//add the nrrd data for mask
		nrrd_mask = nrrdNew();
		val8 = new (std::nothrow) uint8[mem_size];
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
		val8 = (uint8*)nrrd_mask->data;
	}

	if (mode == 0 || mode == 1)
	{
		if (val8)
			memset((void*)val8, mode ?
				255 : 0, mem_size * sizeof(uint8));
	}
	else if (mode == 2)
	{
		if (empty)
			memset((void*)val8, 0, mem_size * sizeof(uint8));
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
	getValue("res x", resx);
	getValue("res y", resy);
	getValue("res z", resz);
	if (resx != nx2 || resy != ny2 || resz != nz2)
	{
		double spcx, spcy, spcz;
		//GetSpacings(spcx, spcy, spcz);
		getValue("spc x", spcx);
		getValue("spc y", spcy);
		getValue("spc z", spcz);
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

void VolumeData::AddEmptyLabel(int mode)
{
	if (!m_tex || !m_vr)
		return;

	long resx, resy, resz;
	getValue("res x", resx);
	getValue("res y", resy);
	getValue("res z", resz);
	unsigned long long mem_size = (unsigned long long)resx*
		(unsigned long long)resy*(unsigned long long)resz;
	Nrrd *nrrd_label = 0;
	unsigned int *val32 = 0;
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
		nrrdWrap(nrrd_label, val32, nrrdTypeUInt, 3, (size_t)resx, (size_t)resy, (size_t)resz);
		nrrdAxisInfoSet(nrrd_label, nrrdAxisInfoSpacing, spcx, spcy, spcz);
		nrrdAxisInfoSet(nrrd_label, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
		nrrdAxisInfoSet(nrrd_label, nrrdAxisInfoMax, spcx*resx, spcy*resy, spcz*resz);
		nrrdAxisInfoSet(nrrd_label, nrrdAxisInfoSize, (size_t)resx, (size_t)resy, (size_t)resz);

		m_tex->set_nrrd(nrrd_label, m_tex->nlabel());
	}
	else
	{
		nrrd_label = m_tex->get_nrrd(m_tex->nlabel());
		val32 = (unsigned int*)nrrd_label->data;
	}

	//apply values
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
	getValue("res x", resx);
	getValue("res y", resy);
	getValue("res z", resz);
	unsigned long long mem_size = (unsigned long long)resx*
		(unsigned long long)resy*(unsigned long long)resz;
	for (unsigned long long index = 0; index < mem_size; ++index)
		if (data_label[index] == label)
			return true;
	return false;
}

double VolumeData::GetOriginalValue(int i, int j, int k, FLIVR::TextureBrick* b)
{
	void *data_data = 0;
	long bits;
	getValue("bits", bits);
	bool multires;
	getValue("multires", multires);
	double int_scale;
	getValue("int scale", int_scale);

	int64_t nx, ny, nz;
	if (multires)
	{
		if (!b || !b->isLoaded()) return 0.0;
		FLIVR::FileLocInfo *finfo = m_tex->GetFileName(b->getID());
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
		uint8 old_value = ((uint8*)(data_data))[index];
		return double(old_value) / 255.0;
	}
	else if (bits == 16)
	{
		uint64_t index = (nx)*(ny)*(kk)+(nx)*(jj)+(ii);
		uint16 old_value = ((uint16*)(data_data))[index];
		return double(old_value)*int_scale / 65535.0;
	}

	return 0.0;
}

double VolumeData::GetTransferValue(int i, int j, int k, FLIVR::TextureBrick* b)
{
	void *data_data = 0;
	long bits;
	getValue("bits", bits);
	bool multires;
	getValue("multires", multires);

	int64_t nx, ny, nz;
	if (multires)
	{
		if (!b || !b->isLoaded()) return 0.0;
		FLIVR::FileLocInfo *finfo = m_tex->GetFileName(b->getID());
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
	getValue("low threshold", lo_thresh);
	getValue("high threshold", hi_thresh);
	getValue("soft thresh", soft_thresh);
	getValue("gamma 3d", gamma3d);
	getValue("extract boundary", extract_boundary);
	getValue("saturation", saturation);
	getValue("alpha", alpha);
	getValue("int scale", int_scale);

	if (bits == 8)
	{
		uint64_t index = nx*ny*kk + nx*jj + ii;
		uint8 old_value = ((uint8*)(data_data))[index];
		double gm = 0.0;
		double new_value = double(old_value) / 255.0;
		if (m_vr->get_inversion())
			new_value = 1.0 - new_value;
		if (i>0 && i<nx - 1 &&
			j>0 && j<ny - 1 &&
			k>0 && k<nz - 1)
		{
			double v1 = ((uint8*)(data_data))[nx*ny*kk + nx*jj + ii - 1];
			double v2 = ((uint8*)(data_data))[nx*ny*kk + nx*jj + ii + 1];
			double v3 = ((uint8*)(data_data))[nx*ny*kk + nx*(jj - 1) + ii];
			double v4 = ((uint8*)(data_data))[nx*ny*kk + nx*(jj + 1) + ii];
			double v5 = ((uint8*)(data_data))[nx*ny*(kk - 1) + nx*jj + ii];
			double v6 = ((uint8*)(data_data))[nx*ny*(kk + 1) + nx*jj + ii];
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
				FLTYPE::Clamp(gm / extract_boundary, 0.0,
					1.0 + extract_boundary*10.0) : 1.0);
			new_value = pow(FLTYPE::Clamp(new_value / saturation,
				gamma<1.0 ? -(gamma - 1.0)*0.00001 : 0.0,
				gamma>1.0 ? 0.9999 : 1.0), gamma);
			new_value *= alpha;
		}
		return new_value;
	}
	else if (bits == 16)
	{
		uint64_t index = nx*ny*kk + nx*jj + ii;
		uint16 old_value = ((uint16*)(data_data))[index];
		double gm = 0.0;
		double new_value = double(old_value)*int_scale / 65535.0;
		if (m_vr->get_inversion())
			new_value = 1.0 - new_value;
		if (ii>0 && ii<nx - 1 &&
			jj>0 && jj<ny - 1 &&
			kk>0 && kk<nz - 1)
		{
			double v1 = ((uint8*)(data_data))[nx*ny*kk + nx*jj + ii - 1];
			double v2 = ((uint8*)(data_data))[nx*ny*kk + nx*jj + ii + 1];
			double v3 = ((uint8*)(data_data))[nx*ny*kk + nx*(jj - 1) + ii];
			double v4 = ((uint8*)(data_data))[nx*ny*kk + nx*(jj + 1) + ii];
			double v5 = ((uint8*)(data_data))[nx*ny*(kk - 1) + nx*jj + ii];
			double v6 = ((uint8*)(data_data))[nx*ny*(kk + 1) + nx*jj + ii];
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
				FLTYPE::Clamp(gm / extract_boundary, 0.0,
					1.0 + extract_boundary*10.0) : 1.0);
			new_value = pow(FLTYPE::Clamp(new_value / saturation,
				gamma<1.0 ? -(gamma - 1.0)*0.00001 : 0.0,
				gamma>1.0 ? 0.9999 : 1.0), gamma);
			new_value *= alpha;
		}
		return new_value;
	}

	return 0.0;
}

void VolumeData::SaveData(std::wstring &filename, int mode, bool bake, bool compress)
{
	if (!m_vr || !m_tex)
		return;

	Nrrd* data = 0;
	bool delete_data = false;

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

	double spcx, spcy, spcz;
	//GetSpacings(spcx, spcy, spcz);
	getValue("spc x", spcx);
	getValue("spc y", spcy);
	getValue("spc z", spcz);

	//save data
	data = m_tex->get_nrrd(0);
	if (data)
	{
		if (bake)
		{
			//wxProgressDialog *prg_diag = new wxProgressDialog(
			//	"FluoRender: Baking volume data...",
			//	"Baking volume data. Please wait.",
			//	100, 0, wxPD_SMOOTH | wxPD_ELAPSED_TIME | wxPD_AUTO_HIDE);

			//process the data
			int bits = data->type == nrrdTypeUShort ? 16 : 8;
			int nx = int(data->axis[0].size);
			int ny = int(data->axis[1].size);
			int nz = int(data->axis[2].size);

			//clipping planes
			//vector<Plane*> *planes = m_vr->get_planes();
			FLTYPE::PlaneSet planeset;
			getValue("clip planes", planeset);

			Nrrd* baked_data = nrrdNew();
			if (bits == 8)
			{
				unsigned long long mem_size = (unsigned long long)nx*
					(unsigned long long)ny*(unsigned long long)nz;
				uint8 *val8 = new (std::nothrow) uint8[mem_size];
				if (!val8)
				{
					//wxMessageBox("Not enough memory. Please save project and restart.");
					return;
				}
				//transfer function
				for (int i = 0; i<nx; i++)
				{
					//prg_diag->Update(95 * (i + 1) / nx);
					for (int j = 0; j<ny; j++)
						for (int k = 0; k<nz; k++)
						{
							int index = nx*ny*k + nx*j + i;
							bool clipped = false;
							FLTYPE::Point p(double(i) / double(nx),
								double(j) / double(ny),
								double(k) / double(nz));
							for (int pi = 0; pi < planeset.GetSize(); ++pi)
							{
								if (planeset[pi].eval_point(p) < 0.0)
								{
									val8[index] = 0;
									clipped = true;
								}
							}
							if (!clipped)
							{
								double new_value = GetTransferValue(i, j, k);
								val8[index] = uint8(new_value*255.0);
							}
						}
				}
				nrrdWrap(baked_data, val8, nrrdTypeUChar, 3, (size_t)nx, (size_t)ny, (size_t)nz);
			}
			else if (bits == 16)
			{
				unsigned long long mem_size = (unsigned long long)nx*
					(unsigned long long)ny*(unsigned long long)nz;
				uint16 *val16 = new (std::nothrow) uint16[mem_size];
				if (!val16)
				{
					//wxMessageBox("Not enough memory. Please save project and restart.");
					return;
				}
				//transfer function
				for (int i = 0; i<nx; i++)
				{
					//prg_diag->Update(95 * (i + 1) / nx);
					for (int j = 0; j<ny; j++)
						for (int k = 0; k<nz; k++)
						{
							int index = nx*ny*k + nx*j + i;
							bool clipped = false;
							FLTYPE::Point p(double(i) / double(nx),
								double(j) / double(ny),
								double(k) / double(nz));
							for (int pi = 0; pi < 6; ++pi)
							{
								if (planeset[pi].eval_point(p) < 0.0)
								{
									val16[index] = 0;
									clipped = true;
								}
							}
							if (!clipped)
							{
								double new_value = GetTransferValue(i, j, k);
								val16[index] = uint16(new_value*65535.0);
							}
						}
				}
				nrrdWrap(baked_data, val16, nrrdTypeUShort, 3, (size_t)nx, (size_t)ny, (size_t)nz);
			}
			nrrdAxisInfoSet(baked_data, nrrdAxisInfoSpacing, spcx, spcy, spcz);
			nrrdAxisInfoSet(baked_data, nrrdAxisInfoMax, spcx*nx, spcy*ny, spcz*nz);
			nrrdAxisInfoSet(baked_data, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
			nrrdAxisInfoSet(baked_data, nrrdAxisInfoSize, (size_t)nx, (size_t)ny, (size_t)nz);

			data = baked_data;
			delete_data = true;

			//prg_diag->Update(100);
			//delete prg_diag;
		}

		//consider doing processing outside of the VolumeData class
		//to reduce coupling
/*		bool resize;
		getValue("resize", resize);
		long resize_x, resize_y, resize_z;
		getValue("resize x", resize_x);
		getValue("resize y", resize_y);
		getValue("resize z", resize_z);
		if (resize)
		{
			FL::VolumeSampler sampler;
			sampler.SetVolume(data);
			sampler.SetSize(resize_x, resize_y, resize_z);
			sampler.SetSpacings(spcx, spcy, spcz);
			sampler.SetType(0);
			//sampler.SetFilterSize(2, 2, 0);
			sampler.Resize();
			Nrrd* temp = data;
			data = sampler.GetResult();
			if (data)
			{
				spcx = data->axis[0].spacing;
				spcy = data->axis[1].spacing;
				spcz = data->axis[2].spacing;
			}
			if (delete_data)
				nrrdNuke(temp);
		}*/

		writer->SetData(data);
		writer->SetSpacings(spcx, spcy, spcz);
		writer->SetCompression(compress);
		writer->Save(filename, mode);

		if (delete_data)
			nrrdNuke(data);
	}
	delete writer;

	setValue("data path", filename);
	SaveMask(false, 0, 0);
	SaveLabel(false, 0, 0);
}

void VolumeData::SaveMask(bool use_reader, long t, long c)
{
	if (!m_vr || !m_tex)
		return;

	if (t == -1)
		getValue("time", t);
	if (c == -1)
		getValue("channel", c);

	Nrrd* data = 0;
	bool delete_data = false;
	double spcx, spcy, spcz;
	//GetSpacings(spcx, spcy, spcz);
	getValue("spc x", spcx);
	getValue("spc y", spcy);
	getValue("spc z", spcz);

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
			getValue("resize", resize);
			long resize_x, resize_y, resize_z;
			getValue("resize x", resize_x);
			getValue("resize y", resize_y);
			getValue("resize z", resize_z);
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
			wstring filename, tex_path;
			getValue("data path", tex_path);
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
		getValue("time", t);
	if (c == -1)
		getValue("channel", c);

	Nrrd* data = 0;
	bool delete_data = false;
	double spcx, spcy, spcz;
	//GetSpacings(spcx, spcy, spcz);
	getValue("spc x", spcx);
	getValue("spc y", spcy);
	getValue("spc z", spcz);

	//save label
	if (m_tex->nlabel() != -1)
	{
		data = m_tex->get_nrrd(m_tex->nlabel());
		if (data)
		{
			//consider doing processing outside of the VolumeData class
			//to reduce coupling
/*			bool resize;
			getValue("resize", resize);
			long resize_x, resize_y, resize_z;
			getValue("resize x", resize_x);
			getValue("resize y", resize_y);
			getValue("resize z", resize_z);
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
			wstring filename, tex_path;
			getValue("data path", tex_path);
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
FLIVR::VolumeRenderer *VolumeData::GetRenderer()
{
	return m_vr;
}

//texture
FLIVR::Texture* VolumeData::GetTexture()
{
	return m_tex;
}

void VolumeData::SetTexture()
{
	if (m_vr)
		m_vr->reset_texture();
	m_tex = 0;
}

void VolumeData::SetMatrices(glm::mat4 &mv_mat,
	glm::mat4 &proj_mat, glm::mat4 &tex_mat)
{
	double sclx, scly, sclz;
	getValue("scale x", sclx);
	getValue("scale y", scly);
	getValue("scale z", sclz);
	glm::mat4 scale_mv = glm::scale(mv_mat, glm::vec3(sclx, scly, sclz));
	if (m_vr)
		m_vr->set_matrices(scale_mv, proj_mat, tex_mat);
}

//draw volume
void VolumeData::Draw(bool ortho, bool adaptive,
	bool interactive, double zoom, int stream_mode)
{
	bool test_wire;
	getValue("test wire", test_wire);
	bool draw_bounds;
	getValue("draw bounds", draw_bounds);

	if (m_vr)
	{
		m_vr->draw(test_wire, adaptive, interactive, ortho, zoom, stream_mode);
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
		getValue("2d mask id", mask_id);
		getValue("2d weight1 id", weight1);
		getValue("2d weight2 id", weight2);
		long brick_num;
		getValue("brick num", brick_num);

		m_vr->set_2d_mask(mask_id);
		m_vr->set_2d_weight(weight1, weight2);
		m_vr->draw_mask(type, paint_mode, hr_mode, ini_thresh, gm_falloff, scl_falloff, scl_translate, w2d, bins, ortho, estimate);
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
			setValue("estimate thresh", est_thresh, Event(Event::NOTIFY_SELF));
		}
	}
}

void VolumeData::DrawLabel(int type, int mode, double thresh, double gm_falloff)
{
	if (m_vr)
		m_vr->draw_label(type, mode, thresh, gm_falloff);
}

//calculation
void VolumeData::Calculate(int type, VolumeData *vd_a, VolumeData *vd_b)
{
	if (m_vr)
	{
		double hi_thresh;
		vd_a->getValue("high threshold", hi_thresh);
		if (type == 6 || type == 7)
			m_vr->set_hi_thresh(hi_thresh);
		m_vr->calculate(type,
			vd_a ? vd_a->GetRenderer() : 0, vd_b ? vd_b->GetRenderer() : 0);
		m_vr->return_volume();
	}
}

FLTYPE::Color VolumeData::GetColorFromColormap(double value)
{
	FLTYPE::Color color;
	long type;
	getValue("colormap type", type);
	double low, high;
	getValue("colormap low", low);
	getValue("colormap high", high);

	double v = (value - low) / (high - low);
	switch (type)
	{
	case 0:
	default:
		color.r(FLTYPE::Clamp(4.0*v - 2.0, 0.0, 1.0));
		color.g(FLTYPE::Clamp(v<0.5 ? 4.0*v : -4.0*v + 4.0, 0.0, 1.0));
		color.b(FLTYPE::Clamp(-4.0*v + 2.0, 0.0, 1.0));
		break;
	case 1:
		color.r(FLTYPE::Clamp(-4.0*v + 2.0, 0.0, 1.0));
		color.g(FLTYPE::Clamp(v<0.5 ? 4.0*v : -4.0*v + 4.0, 0.0, 1.0));
		color.b(FLTYPE::Clamp(4.0*v - 2.0, 0.0, 1.0));
		break;
	case 2:
		color.r(FLTYPE::Clamp(2.0*v, 0.0, 1.0));
		color.g(FLTYPE::Clamp(4.0*v - 2.0, 0.0, 1.0));
		color.b(FLTYPE::Clamp(4.0*v - 3.0, 0.0, 1.0));
		break;
	case 3:
		color.r(FLTYPE::Clamp(v, 0.0, 1.0));
		color.g(FLTYPE::Clamp(1.0 - v, 0.0, 1.0));
		color.b(1.0);
		break;
	case 4:
		color.r(FLTYPE::Clamp(v<0.5 ? v*0.9 + 0.25 : 0.7, 0.0, 1.0));
		color.g(FLTYPE::Clamp(v<0.5 ? v*0.8 + 0.3 : (1.0 - v)*1.4, 0.0, 1.0));
		color.b(FLTYPE::Clamp(v<0.5 ? v*(-0.1) + 0.75 : (1.0 - v)*1.1 + 0.15, 0.0, 1.0));
		break;
	case 5:
		color.r(FLTYPE::Clamp(v, 0.0, 1.0));
		color.g(FLTYPE::Clamp(v, 0.0, 1.0));
		color.b(FLTYPE::Clamp(v, 0.0, 1.0));
		break;
	case 6:
		color.r(1.0 - FLTYPE::Clamp(v, 0.0, 1.0));
		color.g(1.0 - FLTYPE::Clamp(v, 0.0, 1.0));
		color.b(1.0 - FLTYPE::Clamp(v, 0.0, 1.0));
		break;
	}
	return color;
}

void VolumeData::SetOrderedID(unsigned int* val)
{
	long resx, resy, resz;
	getValue("res x", resx);
	getValue("res y", resy);
	getValue("res z", resz);
	unsigned long long mem_size = (unsigned long long)resx*
		(unsigned long long)resy*(unsigned long long)resz;
	for (unsigned long long index = 0;
		index < mem_size; ++index)
		val[index] = index + 1;
}

void VolumeData::SetReverseID(unsigned int* val)
{
	long resx, resy, resz;
	getValue("res x", resx);
	getValue("res y", resy);
	getValue("res z", resz);
	unsigned long long mem_size = (unsigned long long)resx*
		(unsigned long long)resy*(unsigned long long)resz;
	for (unsigned long long index = 0;
		index < mem_size; ++index)
		val[index] = mem_size - index;
}

void VolumeData::SetShuffledID(unsigned int* val)
{
	long resx, resy, resz;
	getValue("res x", resx);
	getValue("res y", resy);
	getValue("res z", resz);

	unsigned int x, y, z;
	unsigned int res;
	unsigned int len = 0;
	unsigned int r = FLTYPE::Max(resx, FLTYPE::Max(resy, resz));
	while (r > 0)
	{
		r /= 2;
		len++;
	}
	for (int i = 0; i<resx; i++)
	for (int j = 0; j<resy; j++)
	for (int k = 0; k<resz; k++)
	{
		x = FLTYPE::reverse_bit(i, len);
		y = FLTYPE::reverse_bit(j, len);
		z = FLTYPE::reverse_bit(k, len);
		res = 0;
		for (unsigned int ii = 0; ii<len; ii++)
		{
			res |= (1 << ii & x) << (2 * ii);
			res |= (1 << ii & y) << (2 * ii + 1);
			res |= (1 << ii & z) << (2 * ii + 2);
		}
		unsigned int index = resx*resy*k + resx*j + i;
		val[index] = resx*resy*resz - res;
	}
}

