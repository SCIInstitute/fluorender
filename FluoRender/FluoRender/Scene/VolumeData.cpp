/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2025 Scientific Computing and Imaging Institute,
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
#include <VolumeData.h>
#include <Global.h>
#include <MainSettings.h>
#include <DataManager.h>
#include <RenderView.h>
#include <VolumeRenderer.h>
#include <VolumeSampler.h>
#include <VolumeBaker.h>
#include <Count.h>
#include <VolCache4D.h>
#include <ShaderProgram.h>
#include <Texture.h>
#include <TextureBrick.h>
#include <Plane.h>
#include <EntryParams.h>
#include <Histogram.h>
#include <TableHistParams.h>
#include <Reshape.h>
#include <base_vol_reader.h>
#include <base_vol_writer.h>
#include <brkxml_reader.h>
#include <tif_writer.h>
#include <nrrd_writer.h>
#include <msk_writer.h>
#include <compatibility.h>
#include <glm/gtc/matrix_transform.hpp>

VolumeData::VolumeData()
{
	type = 2;//volume

	//current channel index
	m_chan = -1;
	m_time = 0;

	//mdoes
	m_render_mode = flvr::RenderMode::Standard;
	//stream modes
	m_stream_mode = 0;

	//mask mode
	m_label_mode = 0;
	m_mask_mode = 0;
	m_use_mask_threshold = false;
	m_mask_clear = false;

	//volume properties
	m_scalar_scale = 1.0;
	m_gm_scale = 1.0;
	m_max_value = 255.0;
	m_min_value = 0.0;

	//transfer function settings
	m_gamma_enable = true;
	m_gamma = 1.0;

	m_boundary_enable = true;
	m_boundary_low = 0.0;
	m_boundary_high = 0.5;
	m_boundary_max = 0.5;

	m_minmax_enable = true;
	m_lo_offset = 0.0;
	m_hi_offset = 1.0;

	m_thresh_enable = true;
	m_lo_thresh = 0.0;
	m_hi_thresh = 1.0;
	m_sw = 0.0;

	m_luminance_enable = true;
	m_luminance = 1.0;

	m_alpha_enable = true;
	m_alpha = 1.0;

	//shading
	m_shading_enable = false;
	m_shading_strength = 1.0;
	m_shading_shine = 1.0;

	//shadow
	m_shadow_enable = false;
	m_shadow_intensity = 0.0;

	m_sample_rate_enable = true;
	m_sample_rate = 2.0;

	m_color = fluo::Color(1.0, 1.0, 1.0);
	m_wl_color = false;

	//noise reduction
	m_noise_rd = false;

	//colormap mode
	m_colormap_inv = 1.0;
	m_color_mode = flvr::ColorMode::SingleColor;
	m_colormap_disp = false;
	m_colormap = 0;
	m_colormap_proj = flvr::ColormapProj::Intensity;
	m_colormap_low_value = 0.0;
	m_colormap_hi_value = 1.0;
	UpdateColormapRange();

	//blend mode
	m_channel_mix_mode = ChannelMixMode::CompositeAdd;

	//resolution, scaling, spacing
	m_size = fluo::Vector(0.0);
	m_scaling = fluo::Vector(1.0);
	m_spacing = fluo::Vector(1.0);
	m_spc_from_file = false;
	m_resample = false;
	m_resampled_size = fluo::Vector(0.0);

	//display control
	m_disp = true;
	m_draw_bounds = false;
	m_test_wiref = glbin_settings.m_test_wiref;

	m_2d_mask = 0;
	m_2d_weight1 = 0;
	m_2d_weight2 = 0;
	m_2d_dmap = 0;

	//compression
	m_compression = false;

	//skip brick
	m_skip_brick = false;

	//legend
	m_legend = true;
	//interpolate
	m_interpolate = true;

	//valid brick number
	m_brick_num = 0;

	//save label
	m_label_save = 0;

	//background intensity
	m_bg_valid = false;
	m_bg_int = 0;

	//machine learning applied
	m_ml_comp_gen_applied = false;

	//transparent
	m_transparent = false;

	m_hist_dirty = true;

	m_mask_count_dirty = true;
	m_mask_sum = 0;
	m_mask_wsum = 0;
}

VolumeData::VolumeData(VolumeData &copy)
{
	if (copy.m_vr)
		m_vr = std::make_unique<flvr::VolumeRenderer>(*copy.m_vr);
	//layer properties
	type = 2;//volume
	SetName(copy.GetName());
	SetGammaColor(copy.GetGammaColor());
	SetBrightness(copy.GetBrightness());
	SetHdr(copy.GetHdr());
	for (int i : { 0, 1, 2}) m_sync[i] = copy.m_sync[i];

	//path and bounds
	m_tex_path = copy.m_tex_path;
	m_bounds = copy.m_bounds;
	m_clipping_box = copy.m_clipping_box;

	//volume renderer and texture
	m_tex = copy.m_tex;

	//current channel index
	m_chan = copy.m_chan;
	m_time = 0;

	//mdoes
	m_render_mode = copy.m_render_mode;
	//stream modes
	m_stream_mode = copy.m_stream_mode;

	//mask mode
	m_label_mode = copy.m_label_mode;
	m_mask_mode = copy.m_mask_mode;
	m_use_mask_threshold = false;
	m_mask_clear = true;

	//volume properties
	m_scalar_scale = copy.m_scalar_scale;
	m_gm_scale = copy.m_gm_scale;
	m_max_value = copy.m_max_value;
	m_min_value = copy.m_min_value;

	//transfer function settings
	m_gamma_enable = copy.m_gamma_enable;
	m_gamma = copy.m_gamma;

	m_boundary_enable = copy.m_boundary_enable;
	m_boundary_low = copy.m_boundary_low;
	m_boundary_high = copy.m_boundary_high;
	m_boundary_max = copy.m_boundary_max;

	m_minmax_enable = copy.m_minmax_enable;
	m_lo_offset = copy.m_lo_offset;
	m_hi_offset = copy.m_hi_offset;

	m_thresh_enable = copy.m_thresh_enable;
	m_lo_thresh = copy.m_lo_thresh;
	m_hi_thresh = copy.m_hi_thresh;
	m_sw = copy.m_sw;

	m_luminance_enable = copy.m_luminance_enable;
	m_luminance = copy.m_luminance;

	m_alpha_enable = copy.m_alpha_enable;
	m_alpha = copy.m_alpha;

	//shading
	m_shading_enable = copy.m_shading_enable;
	m_shading_strength = copy.m_shading_strength;
	m_shading_shine = copy.m_shading_shine;

	//shadow
	m_shadow_enable = copy.m_shadow_enable;
	m_shadow_intensity = copy.m_shadow_intensity;

	m_sample_rate_enable = copy.m_sample_rate_enable;
	m_sample_rate = copy.m_sample_rate;

	m_color = copy.m_color;
	m_wl_color = copy.m_wl_color;

	//noise reduction
	m_noise_rd = copy.m_noise_rd;

	//colormap mode
	m_colormap_inv = copy.m_colormap_inv;
	m_color_mode = copy.m_color_mode;
	m_colormap_disp = copy.m_colormap_disp;
	m_colormap = copy.m_colormap;
	m_colormap_proj = copy.m_colormap_proj;
	m_colormap_low_value = copy.m_colormap_low_value;
	m_colormap_hi_value = copy.m_colormap_hi_value;
	UpdateColormapRange();

	//blend mode
	m_channel_mix_mode = copy.m_channel_mix_mode;

	//resolution, scaling, spacing
	m_size = copy.m_size;
	m_scaling = copy.m_scaling;
	m_spacing = copy.m_spacing;
	m_spc_from_file = copy.m_spc_from_file;
	m_resample = copy.m_resample;
	m_resampled_size = copy.m_resampled_size;

	//display control
	m_disp = copy.m_disp;
	m_draw_bounds = copy.m_draw_bounds;
	m_test_wiref = copy.m_test_wiref;

	m_2d_mask = 0;
	m_2d_weight1 = 0;
	m_2d_weight2 = 0;
	m_2d_dmap = 0;

	//compression
	m_compression = copy.m_compression;

	//skip brick
	m_skip_brick = false;

	//legend
	m_legend = true;

	//interpolate
	m_interpolate = copy.m_interpolate;

	//valid brick number
	m_brick_num = 0;

	//save label
	m_label_save = 0;

	//background intensity
	m_bg_valid = false;
	m_bg_int = 0;

	//machine learning applied
	m_ml_comp_gen_applied = false;

	//transparent
	m_transparent = false;

	m_hist_dirty = true;

	m_mask_count_dirty = true;
	m_mask_sum = 0;
	m_mask_wsum = 0;
}

VolumeData::~VolumeData()
{
	if (m_label_save)
		delete[] m_label_save;
}

//set viewport
void VolumeData::SetViewport(const fluo::Vector4i& vp)
{
	if (m_vr)
		m_vr->set_viewport(vp);
}

//set clear color
void VolumeData::SetClearColor(const fluo::Vector4f& cc)
{
	if (m_vr)
		m_vr->set_clear_color(cc);
}

//data related
//compression
void VolumeData::SetCompression(bool compression)
{
	m_compression = compression;
	if (m_vr)
		m_vr->set_compression(compression);
}

bool VolumeData::GetCompression()
{
	return m_compression;
}

//skip brick
void VolumeData::SetSkipBrick(bool skip)
{
	m_skip_brick = skip;
}

bool VolumeData::GetSkipBrick()
{
	return m_skip_brick;
}

int VolumeData::Load(Nrrd* data, const std::wstring &name, const std::wstring &path)
{
	if (!data || data->dim!=3)
		return 0;

	m_tex_path = path;
	m_name = name;

	Nrrd *nv = data;
	Nrrd *gm = 0;
	m_size = fluo::Vector(
		nv->axis[0].size,
		nv->axis[1].size,
		nv->axis[2].size);

	m_bounds = fluo::BBox(fluo::Point(0.0), fluo::Point(m_size * m_spacing));
	m_clipping_box.SetBBoxes(m_bounds, fluo::BBox(fluo::Point(0.0), fluo::Point(m_size)));

	m_tex = std::make_shared<flvr::Texture>();
	m_tex->set_use_priority(m_skip_brick);
	auto reader = m_reader.lock();
	if (reader && reader->GetType() == READER_BRKXML_TYPE)
	{
		auto breader = std::dynamic_pointer_cast<BRKXMLReader>(reader);
		std::vector<flvr::Pyramid_Level> pyramid;
		std::vector<std::vector<std::vector<std::vector<flvr::FileLocInfo*>>>> fnames;
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
		if (!m_tex->build(nv, 0, 256))
			return 0;
	}

	if (m_tex)
	{
		m_vr = std::make_unique<flvr::VolumeRenderer>();
		m_vr->set_texture(m_tex);
		m_vr->set_sample_rate(m_sample_rate);
		m_vr->set_shading(true);
		m_vr->set_shading_strength(m_shading_strength);
		m_vr->set_shading_shine(m_shading_shine);
		m_vr->set_scalar_scale(m_scalar_scale);
		m_vr->set_gm_scale(m_scalar_scale);
		m_vr->set_mode(m_render_mode);
		m_vr->set_clipping_box(m_clipping_box);
	}

	//clip distance
	fluo::Vector clip_dist = fluo::Max(fluo::Vector(1.0), m_size / fluo::Vector(20.0));
	SetLinkedDist(fluo::ClipPlane::XNeg, clip_dist.intx());
	SetLinkedDist(fluo::ClipPlane::YNeg, clip_dist.inty());
	SetLinkedDist(fluo::ClipPlane::ZNeg, clip_dist.intz());

	m_bg_valid = false;
	m_hist_dirty = true;

	ResetVolume();

	return 1;
}

int VolumeData::Replace(Nrrd* data, bool del_tex)
{
	if (!data || data->dim!=3)
		return 0;
	if (del_tex)
	{
		Nrrd *nv = data;
		m_size = fluo::Vector(
			nv->axis[0].size,
			nv->axis[1].size,
			nv->axis[2].size);

		m_tex = std::make_shared<flvr::Texture>();
		m_tex->set_use_priority(m_skip_brick);
		m_tex->build(nv, m_min_value, m_max_value);
	}
	else
	{
		//set new
		int bytes = GetBits() / 8;
		flvr::TexComp comp = { flvr::CompType::Data, bytes, data};
		m_tex->set_nrrd(flvr::CompType::Data, comp);
	}

	if (m_vr)
		m_vr->set_texture(m_tex);

	m_bg_valid = false;

	//clip distance
	fluo::Vector clip_dist = fluo::Max(fluo::Vector(1.0), m_size / fluo::Vector(20.0));
	SetLinkedDist(fluo::ClipPlane::XNeg, clip_dist.intx());
	SetLinkedDist(fluo::ClipPlane::YNeg, clip_dist.inty());
	SetLinkedDist(fluo::ClipPlane::ZNeg, clip_dist.intz());

	m_hist_dirty = true;

	ResetVolume();

	return 1;
}

int VolumeData::Replace(VolumeData* data)
{
	if (!data ||
		m_size != data->m_size)
		return 0;

	if (m_tex && m_vr)
	{
		m_vr->clear_tex_current();
		m_vr->reset_texture();
	}
	m_tex = data->m_tex;
	SetScalarScale(data->GetScalarScale());
	SetGMScale(data->GetGMScale());
	SetMinMaxValue(data->GetMinValue(), data->GetMaxValue());
	if (m_vr)
		m_vr->set_texture(m_tex);
	else
		return 0;

	m_bg_valid = false;
	m_hist_dirty = true;

	ResetVolume();

	return 1;
}

//volume data
void VolumeData::AddEmptyData(int bits,
	const fluo::Vector& res,
	const fluo::Vector& spc,
	int brick_size)
{
	if (bits!=8 && bits!=16)
		return;

	Nrrd *nv = nrrdNew();
	if (bits == 8)
	{
		unsigned long long mem_size = (unsigned long long)res.intx()*
			(unsigned long long)res.inty()*(unsigned long long)res.intz();
		uint8_t *val8 = new (std::nothrow) uint8_t[mem_size]();
		if (!val8)
		{
			//SetProgress("Not enough memory. Please save project and restart.");
			return;
		}
		nrrdWrap_va(nv, val8, nrrdTypeUChar, 3, (size_t)res.intx(), (size_t)res.inty(), (size_t)res.intz());
	}
	else if (bits == 16)
	{
		unsigned long long mem_size = (unsigned long long)res.intx()*
			(unsigned long long)res.inty() * (unsigned long long)res.intz();
		uint16_t *val16 = new (std::nothrow) uint16_t[mem_size]();
		if (!val16)
		{
			//SetProgress("Not enough memory. Please save project and restart.");
			return;
		}
		nrrdWrap_va(nv, val16, nrrdTypeUShort, 3, (size_t)res.intx(), (size_t)res.inty(), (size_t)res.intz());
	}
	nrrdAxisInfoSet_va(nv, nrrdAxisInfoSpacing, spc.x(), spc.y(), spc.z());
	auto max_size = res * spc;
	nrrdAxisInfoSet_va(nv, nrrdAxisInfoMax, max_size.x(), max_size.y(), max_size.z());
	nrrdAxisInfoSet_va(nv, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
	nrrdAxisInfoSet_va(nv, nrrdAxisInfoSize, (size_t)res.intx(), (size_t)res.inty(), (size_t)res.intz());

	//resolution
	m_size = fluo::Vector(
		nv->axis[0].size,
		nv->axis[1].size,
		nv->axis[2].size);

	//bounding box
	m_bounds = fluo::BBox(fluo::Point(0.0), fluo::Point(m_size * m_spacing));
	m_clipping_box.SetBBoxes(m_bounds, fluo::BBox(fluo::Point(0.0), fluo::Point(m_size)));

	//create texture
	m_tex = std::make_shared<flvr::Texture>();
	m_tex->set_use_priority(false);
	m_tex->set_brick_planned_size(brick_size);
	m_tex->build(nv, 0, 256, 0);
	m_tex->set_spacing(spc);

	//create volume renderer
	m_vr = std::make_unique<flvr::VolumeRenderer>();
	m_vr->set_texture(m_tex);
	m_vr->set_sample_rate(m_sample_rate);
	m_vr->set_shading(true);
	m_vr->set_shading_strength(m_shading_strength);
	m_vr->set_shading_shine(m_shading_shine);
	m_vr->set_scalar_scale(m_scalar_scale);
	m_vr->set_gm_scale(m_scalar_scale);
	m_vr->set_mode(m_render_mode);
	m_vr->set_clipping_box(m_clipping_box);

	//SetMode(m_mode);
	m_bg_valid = false;

	//clip distance
	fluo::Vector clip_dist = fluo::Max(fluo::Vector(1.0), m_size / fluo::Vector(20.0));
	SetLinkedDist(fluo::ClipPlane::XNeg, clip_dist.intx());
	SetLinkedDist(fluo::ClipPlane::YNeg, clip_dist.inty());
	SetLinkedDist(fluo::ClipPlane::ZNeg, clip_dist.intz());

	m_hist_dirty = true;

	ResetVolume();
}

//volume mask
void VolumeData::LoadMask(Nrrd* mask)
{
	if (!mask || !m_tex || !m_vr)
		return;

	//prepare the texture bricks for the mask
	m_tex->add_empty_mask();
	flvr::TexComp comp = { flvr::CompType::Mask, 1, mask };
	m_tex->set_nrrd(flvr::CompType::Mask, comp);

	fluo::Vector mask_size(
		mask->axis[0].size,
		mask->axis[1].size,
		mask->axis[2].size);
	if (m_size != mask_size)
	{
		flrd::VolumeSampler sampler;
		sampler.SetInput(shared_from_this());
		sampler.SetSize(m_size);
		sampler.SetFilter(0);
		//sampler.SetFilterSize(2, 2, 0);
		sampler.Resize(flrd::SDT_Mask, true);
	}
}

void VolumeData::AddEmptyMask(int mode, bool change)
{
	if (!m_tex || !m_vr)
		return;

	Nrrd *nrrd_mask = 0;
	uint8_t *val8 = 0;
	unsigned long long mem_size = (unsigned long long)m_size.intx()*
		(unsigned long long)m_size.inty()*(unsigned long long)m_size.intz();
	//prepare the texture bricks for the mask
	bool empty = m_tex->add_empty_mask();
	if (empty)
	{
		//add the nrrd data for mask
		nrrd_mask = nrrdNew();
		val8 = new (std::nothrow) uint8_t[mem_size];
		if (!val8)
		{
			//SetProgress("Not enough memory. Please save project and restart.");
			return;
		}
		auto spc = m_tex->get_spacing();
		nrrdWrap_va(nrrd_mask, val8, nrrdTypeUChar, 3, (size_t)m_size.intx(), (size_t)m_size.inty(), (size_t)m_size.intz());
		nrrdAxisInfoSet_va(nrrd_mask, nrrdAxisInfoSize, (size_t)m_size.intx(), (size_t)m_size.inty(), (size_t)m_size.intz());
		nrrdAxisInfoSet_va(nrrd_mask, nrrdAxisInfoSpacing, spc.x(), spc.y(), spc.z());
		nrrdAxisInfoSet_va(nrrd_mask, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
		auto max_size = spc * m_size;
		nrrdAxisInfoSet_va(nrrd_mask, nrrdAxisInfoMax, max_size.x(), max_size.y(), max_size.z());

		flvr::TexComp comp = { flvr::CompType::Mask, 1, nrrd_mask };
		m_tex->set_nrrd(comp.type, comp);
	}
	else
	{
		auto comp = m_tex->get_nrrd(flvr::CompType::Mask);
		val8 = (uint8_t*)comp.data->data;
	}

	if (empty || change)
	{
		if (mode == 0 || mode == 1)
		{
			if (val8)
				std::memset((void*)val8, mode ?
					255 : 0, mem_size * sizeof(uint8_t));
		}
	}
}

void VolumeData::AddMask(Nrrd* mask, int op)
{
	if (!mask || !mask->data || !m_tex || !m_vr)
		return;
	if (mask->dim != 3 ||
		mask->axis[0].size != m_size.intx() ||
		mask->axis[1].size != m_size.inty() ||
		mask->axis[2].size != m_size.intz())
		return;

	Nrrd *nrrd_mask = 0;
	uint8_t *val8 = 0;
	unsigned long long mem_size = (unsigned long long)m_size.intx()*
		(unsigned long long)m_size.inty()*(unsigned long long)m_size.intz();
	//prepare the texture bricks for the mask
	bool empty = m_tex->add_empty_mask();
	if (empty)
	{
		//add the nrrd data for mask
		nrrd_mask = nrrdNew();
		val8 = new (std::nothrow) uint8_t[mem_size];
		if (!val8)
		{
			//SetProgress("Not enough memory. Please save project and restart.");
			return;
		}
		auto spc = m_tex->get_spacing();
		nrrdWrap_va(nrrd_mask, val8, nrrdTypeUChar, 3, (size_t)m_size.intx(), (size_t)m_size.inty(), (size_t)m_size.intz());
		nrrdAxisInfoSet_va(nrrd_mask, nrrdAxisInfoSize, (size_t)m_size.intx(), (size_t)m_size.inty(), (size_t)m_size.intz());
		nrrdAxisInfoSet_va(nrrd_mask, nrrdAxisInfoSpacing, spc.x(), spc.y(), spc.z());
		nrrdAxisInfoSet_va(nrrd_mask, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
		auto max_size = spc * m_size;
		nrrdAxisInfoSet_va(nrrd_mask, nrrdAxisInfoMax, max_size.x(), max_size.y(), max_size.z());

		flvr::TexComp comp = { flvr::CompType::Mask, 1, nrrd_mask };
		m_tex->set_nrrd(comp.type, comp);
	}
	else
	{
		auto comp = m_tex->get_nrrd(flvr::CompType::Mask);
		val8 = (uint8_t*)comp.data->data;
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
	if (mask->dim != 3 ||
		mask->axis[0].size != m_size.intx() ||
		mask->axis[1].size != m_size.inty() ||
		mask->axis[2].size != m_size.intz())
		return;

	Nrrd *nrrd_mask = 0;
	uint8_t *val8 = 0;
	unsigned long long mem_size = (unsigned long long)m_size.intx()*
		(unsigned long long)m_size.inty()*(unsigned long long)m_size.intz();
	//prepare the texture bricks for the mask
	bool empty = m_tex->add_empty_mask();
	if (empty)
	{
		//add the nrrd data for mask
		nrrd_mask = nrrdNew();
		val8 = new (std::nothrow) uint8_t[mem_size];
		if (!val8)
		{
			//SetProgress("Not enough memory. Please save project and restart.");
			return;
		}
		auto spc = m_tex->get_spacing();
		nrrdWrap_va(nrrd_mask, val8, nrrdTypeUChar, 3, (size_t)m_size.intx(), (size_t)m_size.inty(), (size_t)m_size.intz());
		nrrdAxisInfoSet_va(nrrd_mask, nrrdAxisInfoSize, (size_t)m_size.intx(), (size_t)m_size.inty(), (size_t)m_size.intz());
		nrrdAxisInfoSet_va(nrrd_mask, nrrdAxisInfoSpacing, spc.x(), spc.y(), spc.z());
		nrrdAxisInfoSet_va(nrrd_mask, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
		auto max_size = spc * m_size;
		nrrdAxisInfoSet_va(nrrd_mask, nrrdAxisInfoMax, max_size.x(), max_size.y(), max_size.z());

		flvr::TexComp comp = { flvr::CompType::Mask, 1, nrrd_mask };
		m_tex->set_nrrd(comp.type, comp);
	}
	else
	{
		auto comp = m_tex->get_nrrd(flvr::CompType::Mask);
		val8 = (uint8_t*)comp.data->data;
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

//volume label
void VolumeData::LoadLabel(Nrrd* label)
{
	if (!label || !m_tex || !m_vr)
		return;

	m_tex->add_empty_label();
	flvr::TexComp comp = { flvr::CompType::Label, 4, label };
	m_tex->set_nrrd(flvr::CompType::Label, comp);

	fluo::Vector size2(
		label->axis[0].size,
		label->axis[1].size,
		label->axis[2].size);
	if (m_size != size2)
	{
		flrd::VolumeSampler sampler;
		sampler.SetInput(shared_from_this());
		sampler.SetSize(m_size);
		sampler.Resize(flrd::SDT_Label, true);
		//nrrdNuke(label);
		//label = sampler.GetResult();
	}
}

void VolumeData::SetOrderedID(unsigned int* val)
{
	int res_x, res_y, res_z;
	res_x = m_size.intx();
	res_y = m_size.inty();
	res_z = m_size.intz();
	for (int i=0; i<res_x; i++) for (int j=0; j<res_y; j++) for (int k=0; k<res_z; k++)
	{
		unsigned int index = res_y*res_z*i + res_z*j + k;
		val[index] = index+1;
	}
}

void VolumeData::SetReverseID(unsigned int* val)
{
	int res_x, res_y, res_z;
	res_x = m_size.intx();
	res_y = m_size.inty();
	res_z = m_size.intz();
	for (int i = 0; i < res_x; i++) for (int j = 0; j < res_y; j++) for (int k = 0; k < res_z; k++)
	{
		unsigned int index = res_y*res_z*i + res_z*j + k;
		val[index] = res_x*res_y*res_z - index;
	}
}

void VolumeData::SetShuffledID(unsigned int* val)
{
	int res_x, res_y, res_z;
	res_x = m_size.intx();
	res_y = m_size.inty();
	res_z = m_size.intz();
	unsigned int x, y, z;
	unsigned int res;
	unsigned int len = 0;
	unsigned int r = std::max(res_x, std::max(res_y, res_z));
	while (r > 0)
	{
		r /= 2;
		len++;
	}
	for (int i = 0; i < res_x; i++) for (int j = 0; j < res_y; j++) for (int k = 0; k < res_z; k++)
	{
		x = reverse_bit(i, len);
		y = reverse_bit(j, len);
		z = reverse_bit(k, len);
		res = 0;
		for (unsigned int ii=0; ii<len; ii++)
		{
			res |= (1<<ii & x)<<(2*ii);
			res |= (1<<ii & y)<<(2*ii+1);
			res |= (1<<ii & z)<<(2*ii+2);
		}
		unsigned int index = res_x*res_y*k + res_x*j + i;
		val[index] = res_x*res_y*res_z - res;
	}
}

void VolumeData::UpdateColormapRange()
{
	switch (m_colormap_proj)
	{
	case flvr::ColormapProj::Intensity://intensity
	default:
		m_colormap_min_value = m_min_value;
		m_colormap_max_value = m_max_value;
		break;
	case flvr::ColormapProj::ZValue://z-value
		m_colormap_min_value = 0;
		m_colormap_max_value = m_size.z() * m_spacing.z();
		break;
	case flvr::ColormapProj::YValue://y-value
		m_colormap_min_value = 0;
		m_colormap_max_value = m_size.y() * m_spacing.y();
		break;
	case flvr::ColormapProj::XValue://x-value
		m_colormap_min_value = 0;
		m_colormap_max_value = m_size.x() * m_spacing.x();
		break;
	case flvr::ColormapProj::TValue://t-value
		m_colormap_min_value = 0;
		if (auto reader = m_reader.lock())
			m_colormap_max_value = reader->GetTimeNum();
		break;
	case flvr::ColormapProj::Gradient://gradient magnitude
	case flvr::ColormapProj::Normal://gradient dir
		m_colormap_min_value = 0;
		m_colormap_max_value = 1;
		break;
	case flvr::ColormapProj::IntDelta://intensity delta
		m_colormap_min_value = -m_max_value;
		m_colormap_max_value = m_max_value;
		break;
	case flvr::ColormapProj::Speed://speed
		m_colormap_min_value = 0;
		m_colormap_max_value = 1;
		break;
	}
}

void VolumeData::AddEmptyLabel(int mode, bool change)
{
	if (!m_tex || !m_vr)
		return;

	Nrrd *nrrd_label = 0;
	unsigned int *val32 = 0;
	bool exist = false;
	//prepare the texture bricks for the labeling mask
	if (m_tex->add_empty_label())
	{
		//add the nrrd data for the labeling mask
		nrrd_label = nrrdNew();
		unsigned long long mem_size = (unsigned long long)m_size.intx() *
			(unsigned long long)m_size.inty() * (unsigned long long)m_size.intz();
		val32 = new (std::nothrow) unsigned int[mem_size];
		if (!val32)
		{
			//SetProgress("Not enough memory. Please save project and restart.");
			return;
		}

		auto spc = m_tex->get_spacing();
		nrrdWrap_va(nrrd_label, val32, nrrdTypeUInt, 3, (size_t)m_size.intx(), (size_t)m_size.inty(), (size_t)m_size.intz());
		nrrdAxisInfoSet_va(nrrd_label, nrrdAxisInfoSize, (size_t)m_size.intx(), (size_t)m_size.inty(), (size_t)m_size.intz());
		nrrdAxisInfoSet_va(nrrd_label, nrrdAxisInfoSpacing, spc.x(), spc.y(), spc.z());
		nrrdAxisInfoSet_va(nrrd_label, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
		auto max_size = spc * m_size;
		nrrdAxisInfoSet_va(nrrd_label, nrrdAxisInfoMax, max_size.x(), max_size.y(), max_size.z());

		flvr::TexComp comp = { flvr::CompType::Label, 1, nrrd_label };
		m_tex->set_nrrd(comp.type, comp);
	}
	else
	{
		auto comp = m_tex->get_nrrd(flvr::CompType::Label);
		val32 = (unsigned int*)comp.data->data;
		exist = true;
	}

	//apply values
	if (!exist || change)
	{
		switch (mode)
		{
		case 0://zeros
			std::memset(val32, 0, sizeof(unsigned int)*GetVoxelCount());
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

	auto comp = m_tex->get_nrrd(flvr::CompType::Label);
	if (!comp.data)
		return false;
	unsigned int* data_label = (unsigned int*)(comp.data->data);
	if (!data_label)
		return false;

	unsigned long long for_size = GetVoxelCount();
	for (unsigned long long index = 0; index < for_size; ++index)
		if (data_label[index] == label)
			return true;
	return false;
}

Nrrd* VolumeData::GetVolume(bool ret)
{
	if (m_vr && m_tex)
	{
		if (ret) m_vr->return_volume();
		return m_tex->get_nrrd(flvr::CompType::Data).data;
	}

	return 0;
}

Nrrd* VolumeData::GetMask(bool ret)
{
	if (m_vr && m_tex && m_tex->has_comp(flvr::CompType::Mask))
	{
		if (ret) m_vr->return_mask();
		return m_tex->get_nrrd(flvr::CompType::Mask).data;
	}

	return 0;
}

bool VolumeData::IsValidMask()
{
	if (!m_tex)
		return false;
	if (!m_tex->has_comp(flvr::CompType::Mask))
		return false;
	if (m_mask_count_dirty)
	{
		flrd::CountVoxels counter;
		counter.SetVolumeData(shared_from_this());
		counter.Count();
		m_mask_sum = counter.GetSum();
		m_mask_wsum = counter.GetWeightedSum();
		m_mask_count_dirty = false;
	}
	return m_mask_sum;
}

Nrrd* VolumeData::GetLabel(bool ret)
{
	if (m_vr && m_tex && m_tex->has_comp(flvr::CompType::Label))
	{
		if (ret) m_vr->return_label();
		return m_tex->get_nrrd(flvr::CompType::Label).data;
	}

	return 0;
}

double VolumeData::GetOriginalValue(const fluo::Point& p, flvr::TextureBrick* b)
{
	void *data_data = 0;
	int bits = 8;
	int64_t nx, ny, nz;

	if (isBrxml())
	{
		if (!b || !b->isLoaded()) return 0.0;
		flvr::FileLocInfo *finfo = m_tex->GetFileName(b->getID());
		data_data = b->tex_data_brk(flvr::CompType::Data, finfo);
		if (!data_data) return 0.0;
		bits = b->nb(flvr::CompType::Data) * 8;
		auto res_b = b->get_size();
		nx = res_b.intx();
		ny = res_b.inty();
		nz = res_b.intz();
	}
	else
	{
		auto comp = m_tex->get_nrrd(flvr::CompType::Data);
		if (!comp.data || !comp.data->data) return 0.0;
		data_data = comp.data->data;
		if (comp.data->type == nrrdTypeUChar)
			bits = 8;
		else if (comp.data->type == nrrdTypeUShort)
			bits = 16;
		nx = (int64_t)(comp.data->axis[0].size);
		ny = (int64_t)(comp.data->axis[1].size);
		nz = (int64_t)(comp.data->axis[2].size);
	}

	fluo::Vector pv(p);
	if (pv.any_l_zero() || pv.any_ge(m_size))
		return 0.0;
	uint64_t ii = p.intx(), jj = p.inty(), kk = p.intz();

	if (bits == 8)
	{
		uint64_t index = (nx)*(ny)*(kk) + (nx)*(jj) + (ii);
		uint8_t old_value = ((uint8_t*)(data_data))[index];
		return double(old_value)/255.0;
	}
	else if (bits == 16)
	{
		uint64_t index = (nx)*(ny)*(kk) + (nx)*(jj) + (ii);
		uint16_t old_value = ((uint16_t*)(data_data))[index];
		return double(old_value)*m_scalar_scale/65535.0;
	}

	return 0.0;
}

double VolumeData::GetMaskValue(const fluo::Point& p, flvr::TextureBrick* b)
{
	void *data_data = 0;
	int bits = 8;
	int64_t nx, ny, nz;

	if (isBrxml())
	{
		if (!b || !b->isLoaded()) return 0.0;
		flvr::FileLocInfo *finfo = m_tex->GetFileName(b->getID());
		data_data = b->tex_data_brk(flvr::CompType::Mask, finfo);
		if (!data_data) return 0.0;
		bits = b->nb(flvr::CompType::Mask) * 8;
		auto res_b = b->get_size();
		nx = res_b.intx();
		ny = res_b.inty();
		nz = res_b.intz();
	}
	else
	{
		auto comp = m_tex->get_nrrd(flvr::CompType::Mask);
		if (!comp.data || !comp.data->data) return 0.0;
		data_data = comp.data->data;
		if (comp.data->type == nrrdTypeUChar)
			bits = 8;
		else if (comp.data->type == nrrdTypeUShort)
			bits = 16;
		nx = (int64_t)(comp.data->axis[0].size);
		ny = (int64_t)(comp.data->axis[1].size);
		nz = (int64_t)(comp.data->axis[2].size);
	}

	fluo::Vector pv(p);
	if (pv.any_l_zero() || pv.any_ge(m_size))
		return 0.0;
	uint64_t ii = p.intx(), jj = p.inty(), kk = p.intz();

	if (bits == 8)
	{
		uint64_t index = (nx)*(ny)*(kk) + (nx)*(jj) + (ii);
		uint8_t old_value = ((uint8_t*)(data_data))[index];
		return double(old_value)/255.0;
	}
	else if (bits == 16)
	{
		uint64_t index = (nx)*(ny)*(kk) + (nx)*(jj) + (ii);
		uint16_t old_value = ((uint16_t*)(data_data))[index];
		return double(old_value)*m_scalar_scale/65535.0;
	}

	return 0.0;
}

double VolumeData::GetTransferedValue(const fluo::Point& p, flvr::TextureBrick* b)
{
	void *data_data = 0;
	int bits = 8;
	int64_t nx, ny, nz;

	if (isBrxml())
	{
		if (!b || !b->isLoaded()) return 0.0;
		flvr::FileLocInfo *finfo = m_tex->GetFileName(b->getID());
		data_data = b->tex_data_brk(flvr::CompType::Data, finfo);
		if (!data_data) return 0.0;
		bits = b->nb(flvr::CompType::Data) * 8;
		auto res_b = b->get_size();
		nx = res_b.intx();
		ny = res_b.inty();
		nz = res_b.intz();
	}
	else
	{
		auto comp = m_tex->get_nrrd(flvr::CompType::Data);
		if (!comp.data || !comp.data->data) return 0.0;
		data_data = comp.data->data;
		if (comp.data->type == nrrdTypeUChar)
			bits = 8;
		else if (comp.data->type == nrrdTypeUShort)
			bits = 16;
		nx = (int64_t)(comp.data->axis[0].size);
		ny = (int64_t)(comp.data->axis[1].size);
		nz = (int64_t)(comp.data->axis[2].size);
	}

	fluo::Vector pv(p);
	if (pv.any_l_zero() || pv.any_ge(m_size))
		return 0.0;
	uint64_t ii = p.intx(), jj = p.inty(), kk = p.intz();

	if (bits == 8)
	{
		uint64_t index = nx*ny*kk + nx*jj + ii;
		uint8_t old_value = ((uint8_t*)(data_data))[index];
		double gm = 0.0;
		double new_value = double(old_value)/255.0;
		if (m_vr->get_inversion())
			new_value = 1.0-new_value;
		if (ii > 0 && ii < nx - 1 &&
			jj>0 && jj < ny - 1 &&
			kk>0 && kk < nz - 1)
		{
			double v1 = ((uint8_t*)(data_data))[nx*ny*kk + nx*jj + ii-1];
			double v2 = ((uint8_t*)(data_data))[nx*ny*kk + nx*jj + ii+1];
			double v3 = ((uint8_t*)(data_data))[nx*ny*kk + nx*(jj-1) + ii];
			double v4 = ((uint8_t*)(data_data))[nx*ny*kk + nx*(jj+1) + ii];
			double v5 = ((uint8_t*)(data_data))[nx*ny*(kk-1) + nx*jj + ii];
			double v6 = ((uint8_t*)(data_data))[nx*ny*(kk+1) + nx*jj + ii];
			double normal_x, normal_y, normal_z;
			normal_x = (v2 - v1) / 255.0;
			normal_y = (v4 - v3) / 255.0;
			normal_z = (v6 - v5) / 255.0;
			gm = sqrt(normal_x*normal_x + normal_y*normal_y + normal_z*normal_z)*0.53;
		}
		if (new_value<m_lo_thresh-m_sw ||
			new_value>m_hi_thresh+m_sw)
			new_value = 0.0;
		else
		{
			double gamma = 1.0 / m_gamma;
			new_value = (new_value<m_lo_thresh?
				(m_sw-m_lo_thresh+new_value)/m_sw:
			(new_value>m_hi_thresh?
				(m_sw-new_value+m_hi_thresh)/m_sw:1.0))
				*new_value;
			double gmf = 5.0 * (gm - m_boundary_low) * (m_boundary_max - m_boundary_high) / m_boundary_max / (m_boundary_high - m_boundary_low);
			new_value *= gm < m_boundary_low ? gm / m_boundary_low : 1.0 + gmf * gmf;
			new_value = pow(fluo::Clamp((new_value-m_lo_offset)/(m_hi_offset-m_lo_offset),
				gamma<1.0?-(gamma-1.0)*0.00001:0.0, 1.0), gamma);
			new_value *= m_alpha;
		}
		return new_value;
	}
	else if (bits == 16)
	{
		uint64_t index = nx*ny*kk + nx*jj + ii;
		uint16_t old_value = ((uint16_t*)(data_data))[index];
		double gm = 0.0;
		double new_value = double(old_value)*m_scalar_scale/65535.0;
		if (m_vr->get_inversion())
			new_value = 1.0-new_value;
		if (ii>0 && ii<nx-1 &&
			jj>0 && jj<ny-1 &&
			kk>0 && kk<nz-1)
		{
			double v1 = ((uint8_t*)(data_data))[nx*ny*kk + nx*jj + ii-1];
			double v2 = ((uint8_t*)(data_data))[nx*ny*kk + nx*jj + ii+1];
			double v3 = ((uint8_t*)(data_data))[nx*ny*kk + nx*(jj-1) + ii];
			double v4 = ((uint8_t*)(data_data))[nx*ny*kk + nx*(jj+1) + ii];
			double v5 = ((uint8_t*)(data_data))[nx*ny*(kk-1) + nx*jj + ii];
			double v6 = ((uint8_t*)(data_data))[nx*ny*(kk+1) + nx*jj + ii];
			double normal_x, normal_y, normal_z;
			normal_x = (v2 - v1)*m_scalar_scale/65535.0;
			normal_y = (v4 - v3)*m_scalar_scale/65535.0;
			normal_z = (v6 - v5)*m_scalar_scale/65535.0;
			gm = sqrt(normal_x*normal_x + normal_y*normal_y + normal_z*normal_z)*0.53;
		}
		if (new_value<m_lo_thresh-m_sw ||
			new_value>m_hi_thresh+m_sw)
			new_value = 0.0;
		else
		{
			double gamma = 1.0 / m_gamma;
			new_value = (new_value<m_lo_thresh?
				(m_sw-m_lo_thresh+new_value)/m_sw:
			(new_value>m_hi_thresh?
				(m_sw-new_value+m_hi_thresh)/m_sw:1.0))
				*new_value;
			double gmf = 5.0 * (gm - m_boundary_low) * (m_boundary_max - m_boundary_high) / m_boundary_max / (m_boundary_high - m_boundary_low);
			new_value *= gm < m_boundary_low ? gm / m_boundary_low : 1.0 + gmf * gmf;
			new_value = pow(fluo::Clamp((new_value-m_lo_offset)/(m_hi_offset-m_lo_offset),
				gamma<1.0?-(gamma-1.0)*0.00001:0.0, 1.0), gamma);
			new_value *= m_alpha;
		}
		return new_value;
	}

	return 0.0;
}

//save
void VolumeData::Save(const std::wstring &filename, int mode,
	int mask, bool neg_mask,
	bool crop, int filter,
	bool bake, bool compress,
	const fluo::Point &c,//rotation center
	const fluo::Quaternion &q,//rotation
	const fluo::Vector &t,//translate
	bool fix_size)
{
	if (!m_vr || !m_tex)
		return;

	std::shared_ptr<VolumeData> temp;
	if (bake)
	{
		flrd::VolumeBaker baker;
		baker.SetInput(temp ? temp : shared_from_this());
		baker.Bake(temp ? true : false);
		temp = baker.GetResult();
	}

	if (m_resample || crop)
	{
		flrd::VolumeSampler sampler;
		sampler.SetInput(temp ? temp : shared_from_this());
		sampler.SetFixSize(fix_size);
		sampler.SetSize(m_resampled_size);
		sampler.SetFilter(filter);
		sampler.SetFilterSize(fluo::Vector(1.0));
		sampler.SetCrop(crop);
		sampler.SetCenter(c);
		sampler.SetClipRotation(q);
		sampler.SetTranslate(t);
		sampler.SetNegMask(neg_mask);
		bool replace = temp ? true : false;
		sampler.Resize(flrd::SDT_All, replace);
		if (!replace)
			temp = sampler.GetResult();
	}

	BaseVolWriter *writer = 0;
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
	flvr::TexComp comp;
	if (temp)
	{
		if (temp->m_tex)
			comp = temp->m_tex->get_nrrd(flvr::CompType::Data);
	}
	else
	{
		comp = m_tex->get_nrrd(flvr::CompType::Data);
	}
	if (comp.data)
	{
		auto spc = fluo::Vector(
			comp.data->axis[0].spacing,
			comp.data->axis[1].spacing,
			comp.data->axis[2].spacing);
		writer->SetData(comp.data);
		writer->SetSpacing(spc);
		writer->SetCompression(compress);
		writer->Save(filename, mode);
	}
	delete writer;

	if (m_resample || crop)
	{
		temp->SetPath(filename);
		if (mask & 1)
			temp->SaveMask(false, 0, 0);
		if (mask & 2)
			temp->SaveLabel(false, 0, 0);
	}
	else
	{
		if (mask & 1)
			SaveMask(false, 0, 0);
		if (mask & 2)
			SaveLabel(false, 0, 0);
	}

	m_tex_path = filename;
}

void VolumeData::SaveMask(bool use_reader, int t, int c)
{
	if (!m_vr || !m_tex)
		return;

	Nrrd* data = 0;
	auto spc = GetSpacing();

	//save mask
	data = GetMask(true);
	if (!data)
		return;

	MSKWriter msk_writer;
	msk_writer.SetData(data);
	msk_writer.SetSpacing(spc);
	std::wstring filename;
	if (use_reader)
	{
		if (auto reader = m_reader.lock())
			filename = reader->GetCurMaskName(t, c);
	}
	else
		filename = m_tex_path.substr(0, m_tex_path.find_last_of(L'.')) + L".msk";
	msk_writer.Save(filename, 0);
}

void VolumeData::SaveLabel(bool use_reader, int t, int c)
{
	if (!m_vr || !m_tex)
		return;

	Nrrd* data = 0;
	auto spc = GetSpacing();

	//save label
	data = GetLabel(true);
	if (!data)
		return;

	MSKWriter msk_writer;
	msk_writer.SetData(data);
	msk_writer.SetSpacing(spc);
	std::wstring filename;
	if (use_reader)
	{
		if (auto reader = m_reader.lock())
			filename = reader->GetCurLabelName(t, c);
	}
	else
		filename = m_tex_path.substr(0, m_tex_path.find_last_of(L'.')) + L".lbl";
	msk_writer.Save(filename, 1);
}

//bounding box
fluo::BBox VolumeData::GetBounds()
{
	return m_bounds;
}

fluo::BBox VolumeData::GetClippedBounds()
{
	return m_clipping_box.GetBBoxWorld();
}

//path
void VolumeData::SetPath(const std::wstring& path)
{
	m_tex_path = path;
}

std::wstring VolumeData::GetPath()
{
	return m_tex_path;
}

//multi-channel
void VolumeData::SetCurChannel(int chan)
{
	m_chan = chan;
}

int VolumeData::GetCurChannel()
{
	return m_chan;
}

//time sequence
void VolumeData::SetCurTime(int time)
{
	m_time = time;
	SetMlCompGenApplied(false);
}

int VolumeData::GetCurTime()
{
	return m_time;
}

//MIP & normal modes
void VolumeData::SetRenderMode(flvr::RenderMode mode)
{
	m_render_mode = mode;
	if (m_vr)
		m_vr->set_mode(m_render_mode);

	//if (!m_vr)
	//	return;

	//switch (mode)
	//{
	//case 0://normal
	//	m_vr->set_mode(flvr::RenderMode::Standard);
	//	m_vr->set_color(m_color);
	//	break;
	//case 1://MIP
	//	m_vr->set_mode(flvr::RenderMode::Mip);
	//	m_vr->set_color(m_color);
	//	break;
	//case 2://white shading
	//	m_vr->set_mode(flvr::RenderMode::Standard);
	//	m_vr->set_color_mode(0);
	//	m_vr->set_color(fluo::Color(1.0, 1.0, 1.0));
	//	break;
	//case 3://white mip
	//	m_vr->set_mode(flvr::RenderMode::Mip);
	//	m_vr->set_color_mode(0);
	//	m_vr->set_color(fluo::Color(1.0, 1.0, 1.0));
	//	break;
	//}
}

//transparency
void VolumeData::SetAlphaPower(double val)
{
	if (m_vr)
		m_vr->set_alpha_power(val);
}

double VolumeData::GetAlphaPower()
{
	if (m_vr)
		return m_vr->get_alpha_power();
	else
		return 1.0;
}

//inversion
void VolumeData::SetInvert(bool val)
{
	m_invert = val;
	if (m_vr)
		m_vr->set_inversion(val);
}

bool VolumeData::GetInvert()
{
	if (m_vr)
		return m_vr->get_inversion();
	else
		return false;
}

//mask mode
void VolumeData::SetMaskMode(int mode)
{
	m_mask_mode = mode;
	if (m_vr)
		m_vr->set_ml_mode(mode);
}

int VolumeData::GetMaskMode()
{
	return m_mask_mode;
}

//noise reduction
void VolumeData::SetNR(bool val)
{
	m_noise_rd = val;
	if (m_vr)
		m_vr->SetNoiseRed(val);
}

bool VolumeData::GetNR()
{
	return m_noise_rd;
}

//volumerenderer
flvr::VolumeRenderer *VolumeData::GetVR()
{
	return m_vr.get();
}

//texture
flvr::Texture* VolumeData::GetTexture()
{
	return m_tex.get();
}

void VolumeData::ResetVolume()
{
	flvr::CacheQueue* cache_queue = glbin_data_manager.GetCacheQueue(this);
	if (cache_queue)
		cache_queue->reset(m_time);
	m_ep.reset();
	m_hist_dirty = true;
	m_auto_threshold = -1;
}

void VolumeData::SetMatrices(glm::mat4 &mv_mat,
	glm::mat4 &proj_mat, glm::mat4 &tex_mat)
{
	glm::mat4 scale_mv =
		glm::scale(mv_mat,
			glm::vec3(m_scaling.x(), m_scaling.y(), m_scaling.z()));
	if (m_vr)
		m_vr->set_matrices(scale_mv, proj_mat, tex_mat);
}

//draw volume
void VolumeData::Draw(bool ortho, bool interactive, double zoom, double sf121)
{
	if (m_vr)
	{
		m_vr->set_zoom(zoom, sf121);
		m_vr->draw(m_test_wiref, interactive, ortho, m_stream_mode);
	}
	if (m_draw_bounds)
		DrawBounds();
}

void VolumeData::DrawBounds()
{
}

//draw mask (create the mask)
//type: 0-initial; 1-diffusion-based growing
//paint_mode: 1-select; 2-append; 3-erase; 4-diffuse; 5-flood; 6-clear, 7-all
//			  11-posterize
//hr_mode (hidden removal): 0-none; 1-ortho; 2-persp
void VolumeData::DrawMask(int type, int paint_mode, int hr_mode,
	double ini_thresh, double gm_falloff, double scl_falloff, double scl_translate,
	double w2d, double bins, int order, bool ortho, bool estimate)
{
	int pmode2 = paint_mode;
	pmode2 = (pmode2 == 10 || pmode2 == 11) ?
		2 : pmode2; //convert to append mode
	if (m_vr)
	{
		m_vr->set_2d_mask(m_2d_mask);
		m_vr->set_2d_weight(m_2d_weight1, m_2d_weight2);
		m_vr->draw_mask(type, pmode2, hr_mode,
			ini_thresh, gm_falloff,
			scl_falloff, scl_translate,
			w2d, bins, order,
			ortho, estimate);
		if (estimate)
			m_est_thresh = m_vr->get_estimated_thresh();
	}
}

//calculation
void VolumeData::Calculate(int type, VolumeData *vd_a, VolumeData *vd_b)
{
	if (m_vr)
	{
		if (type==6 || type==7)
			m_vr->set_hi_thresh(vd_a->GetRightThresh());
		m_vr->calculate(type, vd_a?vd_a->GetVR():0, vd_b?vd_b->GetVR():0);
		m_vr->return_volume();
	}
}

//set 2d mask for segmentation
void VolumeData::Set2dMask(unsigned int mask)
{
	m_2d_mask = mask;
}

//set 2d weight map for segmentation
void VolumeData::Set2DWeight(unsigned int weight1, unsigned int weight2)
{
	m_2d_weight1 = weight1;
	m_2d_weight2 = weight2;
}

//set 2d depth map for rendering shadows
void VolumeData::Set2dDmap(unsigned int dmap)
{
	m_2d_dmap = dmap;
	if (m_vr)
		m_vr->set_2d_dmap(m_2d_dmap);
}

//transfer function properties
void VolumeData::SetGammaEnable(bool bval)
{
	m_gamma_enable = bval;
	if (bval)
		SetGamma(m_gamma, false);
	else
		SetGamma(1.0, false);
}

bool VolumeData::GetGammaEnable()
{
	return m_gamma_enable;
}

void VolumeData::SetGamma(double val, bool set_this)
{
	if (set_this)
		m_gamma = val;
	if (m_vr)
		m_vr->set_gamma3d(val);
}

double VolumeData::GetGamma()
{
	return m_gamma;
}

double VolumeData::GetMlGamma()
{
	GetMlParams();

	if (m_ep && m_ep->getValid())
		return m_ep->getParam("gamma3d");
	else
		return glbin_vol_def.m_gamma;
}

void VolumeData::SetBoundaryEnable(bool bval)
{
	m_boundary_enable = bval;
	if (bval)
	{
		SetBoundaryLow(m_boundary_low, false);
		SetBoundaryHigh(m_boundary_high, false);
	}
	else
	{
		SetBoundaryLow(0.0, false);
		SetBoundaryHigh(m_boundary_max, false);
	}
}

bool VolumeData::GetBoundaryEnable()
{
	return m_boundary_enable;
}

void VolumeData::SetBoundaryLow(double val, bool set_this)
{
	if (set_this)
		m_boundary_low = val;
	if (m_vr)
		m_vr->set_gm_low(val);
}

double VolumeData::GetBoundaryLow()
{
	return m_boundary_low;
}

double VolumeData::GetMlBoundaryLow()
{
	GetMlParams();

	if (m_ep && m_ep->getValid())
		return m_ep->getParam("extract_boundary");
	else
		return glbin_vol_def.m_boundary_low;
}

void VolumeData::SetBoundaryHigh(double val, bool set_this)
{
	if (set_this)
		m_boundary_high = val;
	if (m_vr)
		m_vr->set_gm_high(val);
}

double VolumeData::GetBoundaryHigh()
{
	return m_boundary_high;
}

double VolumeData::GetMlBoundaryHigh()
{
	return glbin_vol_def.m_boundary_high;
}

void VolumeData::SetBoundaryMax(double val)
{
	m_boundary_max = val;
}

double VolumeData::GetBoundaryMax()
{
	return m_boundary_max;
}

void VolumeData::SetMinMaxEnable(bool bval)
{
	m_minmax_enable = bval;
	if (bval)
	{
		SetLowOffset(m_lo_offset, false);
		SetHighOffset(m_hi_offset, false);
	}
	else
	{
		SetLowOffset(m_min_value / m_max_value, false);
		SetHighOffset(1.0, false);
	}
}

bool VolumeData::GetMinMaxEnable()
{
	return m_minmax_enable;
}

void VolumeData::SetLowOffset(double val, bool set_this)
{
	if (set_this)
		m_lo_offset = val;
	if (m_vr)
		m_vr->set_lo_offset(val);
}

double VolumeData::GetLowOffset()
{
	return m_lo_offset;
}

double VolumeData::GetMlLowOffset()
{
	GetMlParams();

	if (m_ep && m_ep->getValid())
		return m_ep->getParam("low_offset");
	else
		return glbin_vol_def.m_lo_offset;
}

void VolumeData::SetHighOffset(double val, bool set_this)
{
	if (set_this)
		m_hi_offset = val;
	if (m_vr)
		m_vr->set_hi_offset(val);
}

double VolumeData::GetHighOffset()
{
	return m_hi_offset;
}

double VolumeData::GetMlHighOffset()
{
	GetMlParams();

	if (m_ep && m_ep->getValid())
		return m_ep->getParam("high_offset");
	else
		return glbin_vol_def.m_hi_offset;
}

void VolumeData::SetThreshEnable(bool bval)
{
	m_thresh_enable = bval;
	if (bval)
	{
		SetLeftThresh(m_lo_thresh, false);
		SetRightThresh(m_hi_thresh, false);
	}
	else
	{
		SetLeftThresh(0, false);
		SetRightThresh(1, false);
	}
}

bool VolumeData::GetThreshEnable()
{
	return m_thresh_enable;
}

void VolumeData::SetLeftThresh(double val, bool set_this)
{
	if (set_this)
		m_lo_thresh = val;
	if (m_vr)
		m_vr->set_lo_thresh(val);
}

double VolumeData::GetLeftThresh()
{
	return m_lo_thresh;
}

double VolumeData::GetMlLeftThresh()
{
	GetMlParams();

	if (m_ep && m_ep->getValid())
		return m_ep->getParam("low_threshold");
	else
		return glbin_vol_def.m_lo_thresh;
}

void VolumeData::SetRightThresh(double val, bool set_this)
{
	if (set_this)
		m_hi_thresh = val;
	if (m_vr)
		m_vr->set_hi_thresh(val);
}

double VolumeData::GetRightThresh()
{
	return m_hi_thresh;
}

void VolumeData::SetSoftThreshold(double val)
{
	m_sw = val;
	if (m_vr)
		m_vr->set_soft_thresh(val);
}

double VolumeData::GetSoftThreshold()
{
	return m_sw;
}

double VolumeData::GetMlRightThresh()
{
	GetMlParams();

	if (m_ep && m_ep->getValid())
		return m_ep->getParam("high_threshold");
	else
		return glbin_vol_def.m_hi_thresh;
}

void VolumeData::SetLuminanceEnable(bool bval)
{
	m_luminance_enable = bval;
	if (bval)
		SetLuminance(m_luminance, false);
	else
		SetLuminance(1.0, false);
}

bool VolumeData::GetLuminanceEnable()
{
	return m_luminance_enable;
}

void VolumeData::SetLuminance(double val, bool set_this)
{
	if (set_this)
		m_luminance = val;
	if (m_vr)
		m_vr->set_luminance(m_luminance);
}

double VolumeData::GetLuminance()
{
	return m_luminance;
}

double VolumeData::GetMlLuminance()
{
	GetMlParams();

	if (m_ep && m_ep->getValid())
		return m_ep->getParam("luminance");
	else
		return glbin_vol_def.m_luminance;
}

void VolumeData::SetAlphaEnable(bool bval)
{
	m_alpha_enable = bval;
	if (m_vr)
		m_vr->set_solid(!bval);
	if (bval)
		SetAlpha(m_alpha, false);
	else
	{
		SetAlpha(1.0, false);
	}
}

bool VolumeData::GetAlphaEnable()
{
	return m_alpha_enable;
}

void VolumeData::SetAlpha(double val, bool set_this)
{
	if (set_this)
		m_alpha = val;
	if (m_vr)
		m_vr->set_alpha(val);
}

double VolumeData::GetAlpha()
{
	return m_alpha;
}

double VolumeData::GetMlAlpha()
{
	GetMlParams();

	if (m_ep && m_ep->getValid())
		return m_ep->getParam("alpha");
	else
		return glbin_vol_def.m_alpha;
}

//shading
void VolumeData::SetShadingEnable(bool bVal)
{
	m_shading_enable = bVal;
	if (m_vr)
		m_vr->set_shading(bVal);
}

bool VolumeData::GetShadingEnable()
{
	return m_shading_enable;
}

void VolumeData::SetShadingStrength(double val)
{
	m_shading_strength = val;
	if (m_vr)
		m_vr->set_shading_strength(val);
}

double VolumeData::GetShadingStrength()
{
	return m_shading_strength;
}

double VolumeData::GetMlShadingStrength()
{
	GetMlParams();

	if (m_ep && m_ep->getValid())
		return m_ep->getParam("low_shading");
	else
		return glbin_vol_def.m_shading_strength;
}

void VolumeData::SetShadingShine(double val)
{
	m_shading_shine = val;
	if (m_vr)
		m_vr->set_shading_shine(val);
}

double VolumeData::GetShadingShine()
{
	return m_shading_shine;
}

double VolumeData::GetMlShadingShine()
{
	GetMlParams();

	if (m_ep && m_ep->getValid())
		return m_ep->getParam("high_shading");
	else
		return glbin_vol_def.m_shading_shine;
}

//shadow
void VolumeData::SetShadowEnable(bool bVal)
{
	m_shadow_enable = bVal;
	if (m_vr)
		m_vr->set_depth(bVal);
}

bool VolumeData::GetShadowEnable()
{
	return m_shadow_enable;
}

void VolumeData::SetShadowIntensity(double val)
{
	m_shadow_intensity = val;
}

double VolumeData::GetShadowIntensity()
{
	return m_shadow_intensity;
}

double VolumeData::GetMlShadowIntensity()
{
	GetMlParams();

	if (m_ep && m_ep->getValid())
		return m_ep->getParam("shadow_intensity");
	else
		return glbin_vol_def.m_shadow_intensity;
}

//sample rate
void VolumeData::SetSampleRateEnable(bool bval)
{
	m_sample_rate_enable = bval;
	if (bval)
		SetSampleRate(m_sample_rate, false);
	else
		SetSampleRate(2.0, false);
}

bool VolumeData::GetSampleRateEnable()
{
	return m_sample_rate_enable;
}

void VolumeData::SetSampleRate(double val, bool set_this)
{
	if (set_this)
		m_sample_rate = val;
	if (m_vr)
		m_vr->set_sample_rate(val);
}

double VolumeData::GetSampleRate()
{
	return m_sample_rate;
}

double VolumeData::GetMlSampleRate()
{
	GetMlParams();

	if (m_ep && m_ep->getValid())
		return m_ep->getParam("sample_rate");
	else
		return glbin_vol_def.m_sample_rate;
}

void VolumeData::SetColor(const fluo::Color &color, bool set_this)
{
	if (set_this)
		m_color = color;
	if (m_vr)
		m_vr->set_color(color);
}

fluo::Color VolumeData::GetColor()
{
	return m_color;
}

void VolumeData::SetWlColor(bool bval)
{
	m_wl_color = bval;
}

bool VolumeData::GetWlColor()
{
	return m_wl_color;
}

void VolumeData::SetMaskColor(const fluo::Color &color, bool set)
{
	if (m_vr)
		m_vr->set_mask_color(color, set);
}

fluo::Color VolumeData::GetMaskColor()
{
	fluo::Color result;
	if (m_vr)
		result = m_vr->get_mask_color();
	return result;
}

bool VolumeData::GetMaskColorSet()
{
	if (m_vr)
		return m_vr->get_mask_color_set();
	else
		return false;
}

void VolumeData::ResetMaskColorSet()
{
	if (m_vr)
		m_vr->reset_mask_color_set();
}

//mask threshold
void VolumeData::SetMaskThreshold(double thresh)
{
	if (m_use_mask_threshold && m_vr)
		m_vr->set_mask_thresh(thresh);
}

void VolumeData::SetUseMaskThreshold(bool mode)
{
	m_use_mask_threshold = mode;
	if (m_vr && !m_use_mask_threshold)
		m_vr->set_mask_thresh(0.0);
}

//colormap mode
void VolumeData::SetColorMode(flvr::ColorMode mode)
{
	m_color_mode = mode;
	if (m_vr)
	{
		m_vr->set_color_mode(m_color_mode);
		m_vr->set_color(m_color);
	}
}

flvr::ColorMode VolumeData::GetColorMode()
{
	return m_color_mode;
}

void VolumeData::SetColormapDisp(bool disp)
{
	m_colormap_disp = disp;
}

bool VolumeData::GetColormapDisp()
{
	return m_colormap_disp;
}

void VolumeData::SetColormapValues(double low, double high)
{
	m_colormap_low_value = low;
	m_colormap_hi_value = high;
	if (m_vr)
		m_vr->set_colormap_values(
		m_colormap_low_value, m_colormap_hi_value);
}

void VolumeData::SetColormapLow(double val)
{
	m_colormap_low_value = val;
	if (m_vr)
		m_vr->set_colormap_values(
			m_colormap_low_value, m_colormap_hi_value);
}

void VolumeData::SetColormapHigh(double val)
{
	m_colormap_hi_value = val;
	if (m_vr)
		m_vr->set_colormap_values(
			m_colormap_low_value, m_colormap_hi_value);
}

void VolumeData::GetColormapRange(double& v1, double& v2)
{
	v1 = m_colormap_min_value;
	v2 = m_colormap_max_value;
}

double VolumeData::GetColormapMin()
{
	return m_colormap_min_value;
}

double VolumeData::GetColormapMax()
{
	return m_colormap_max_value;
}

void VolumeData::GetColormapValues(double &low, double &high)
{
	low = m_colormap_low_value;
	high = m_colormap_hi_value;
}

void VolumeData::GetColormapDispValues(double& low, double& high)
{
	low = m_colormap_low_value * (m_colormap_max_value - m_colormap_min_value) + m_colormap_min_value;
	high = m_colormap_hi_value * (m_colormap_max_value - m_colormap_min_value) + m_colormap_min_value;
}

double VolumeData::GetColormapLow()
{
	return m_colormap_low_value;
}

double VolumeData::GetMlColormapLow()
{
	GetMlParams();

	if (m_ep && m_ep->getValid())
		return m_ep->getParam("colormap_low");
	else
		return glbin_vol_def.m_colormap_low_value;
}

double VolumeData::GetColormapHigh()
{
	return m_colormap_hi_value;
}

double VolumeData::GetMlColormapHigh()
{
	GetMlParams();

	if (m_ep->getValid())
		return m_ep->getParam("colormap_hi");
	else
		return glbin_vol_def.m_colormap_hi_value;
}

void VolumeData::SetColormapInv(double val)
{
	m_colormap_inv = val;
	if (m_vr)
		m_vr->set_colormap_inv(val);
}

double VolumeData::GetColormapInv()
{
	return m_colormap_inv;
}

void VolumeData::SetColormap(int value)
{
	m_colormap = value;
	if (m_vr)
		m_vr->set_colormap(m_colormap);
}

void VolumeData::SetColormapProj(flvr::ColormapProj value)
{
	m_colormap_proj = value;
	if (m_vr)
		m_vr->set_colormap_proj(m_colormap_proj);
	UpdateColormapRange();
}

int VolumeData::GetColormap()
{
	return m_colormap;
}

flvr::ColormapProj VolumeData::GetColormapProj()
{
	return m_colormap_proj;
}

fluo::Color VolumeData::GetColorFromColormap(double value, bool raw)
{
	fluo::Color rb;
	double v = value;
	if (!raw)
		v = (v - m_colormap_low_value) /
			(m_colormap_hi_value - m_colormap_low_value);
	double valu = fluo::Clamp(v, 0.0, 1.0);
	double inv = GetColormapInv();
	switch (m_colormap)
	{
	case 0://rainbow
	default:
		rb.r(fluo::Clamp((4.0*valu - 2.0)*inv, 0.0, 1.0));
		rb.g(fluo::Clamp(valu<0.5 ? 4.0*valu : -4.0*valu+4.0, 0.0, 1.0));
		rb.b(fluo::Clamp((2.0 - 4.0*valu)*inv, 0.0, 1.0));
		break;
	case 1://primary-secondary
	{
		fluo::Color mask_color = m_vr->get_mask_color();
		rb = (inv > 0.0 ? mask_color : m_color) * (1.0 - valu) + (inv > 0.0 ? m_color : mask_color) * valu;
	}
		break;
	case 2://hot
		rb.r(fluo::Clamp(inv*2.0*valu+(inv>0.0?0.0:2.0), 0.0, 1.0));
		rb.g(fluo::Clamp(inv*(4.0*valu - 2.0), 0.0, 1.0));
		rb.b(fluo::Clamp(inv*4.0*valu+(inv>0.0?-3.0:1.0), 0.0, 1.0));
		break;
	case 3://cool
		rb.r(fluo::Clamp(inv>0.0?valu:(1.0-valu), 0.0, 1.0));
		rb.g(fluo::Clamp(inv>0.0?(1.0-valu):valu, 0.0, 1.0));
		rb.b(1.0);
		break;
	case 4://diverging
		rb.r(fluo::Clamp(inv>0.0?(valu<0.5?valu*0.9+0.25:0.7):(valu<0.5?0.7:-0.9*valu+1.15), 0.0, 1.0));
		rb.g(fluo::Clamp(inv>0.0?(valu<0.5?valu*0.8+0.3:1.4-1.4*valu):(valu<0.5?1.4*valu:-0.8*valu+1.1), 0.0, 1.0));
		rb.b(fluo::Clamp(inv>0.0?(valu<0.5?-0.1*valu+0.75:-1.1*valu+1.25):(valu<0.5?1.1*valu+0.15:0.1*valu+0.65), 0.0, 1.0));
		break;
	case 5://monochrome
	{
		double cv = (inv > 0.0 ? 0.0 : 1.0) + inv * fluo::Clamp(valu, 0.0, 1.0);
		rb.r(cv);
		rb.g(cv);
		rb.b(cv);
	}
		break;
	case 6://high-key
	{
		fluo::Color w(1.0, 1.0, 1.0);
		rb = (inv > 0.0 ? w : m_color) * (1.0 - valu) + (inv > 0.0 ? m_color : w) * valu;
	}
		break;
	case 7://low-key
	{
		fluo::Color l = m_color * 0.1;
		rb = (inv > 0.0 ? m_color : l) * (1.0 - valu) + (inv > 0.0 ? l : m_color) * valu;
	}
		break;
	case 8://increased transp
	{
		fluo::Color l(0.0, 0.0, 0.0);
		rb = (inv > 0.0 ? l : m_color) * (1.0 - valu) + (inv > 0.0 ? m_color : l) * valu;
	}
		break;
	}
	return rb;
}

bool VolumeData::GetColormapData(std::vector<unsigned char>& data)
{
	data.resize(32 * 3, 0);
	for (int i = 0; i < 32; ++i)
	{
		fluo::Color c = GetColorFromColormap(double(i) / 31, true);
		data[i * 3] = static_cast<unsigned char>(std::round(c.r() * 255.0));
		data[i * 3 + 1] = static_cast<unsigned char>(std::round(c.g() * 255.0));
		data[i * 3 + 2] = static_cast<unsigned char>(std::round(c.b() * 255.0));
	}
	return true;
}

void VolumeData::ComputeHistogram(bool set_prog_func)
{
	int get_histogram = glbin_automate_def.m_histogram;
	if (get_histogram == 0)
		return;
	else if (get_histogram == 1)
		;
	else if (get_histogram == 2)
	{
		if (GetAllBrickNum() > 1)
			return;
	}

	if (m_hist_dirty)
	{
		int bins = 128;
		flrd::Histogram histogram(this);
		if (set_prog_func)
			histogram.SetProgressFunc(glbin_data_manager.GetProgressFunc());
		histogram.SetUseMask(false);
		histogram.SetBins(bins);
		histogram.Compute();
		m_hist = histogram.GetHistogram();
		if (m_hist.size())
			m_hist_dirty = false;
	}
}

bool VolumeData::GetHistogram(std::vector<unsigned char>& data)
{
	ComputeHistogram(true);
	int bins = static_cast<int>(m_hist.size() - 1);
	if (bins <= 0)
		return false;
	int win = 4;//half window size
	data.resize(bins * 3, 0);
	fluo::HSVColor hsv(m_color);
	fluo::Color bg;
	if (hsv.val() > 0.5)
		bg = fluo::Color(0.25);
	else
		bg = fluo::Color(0.75);
	fluo::Color c;
	double sum = m_hist[bins] - m_hist[0];
	if (sum == 0.0)
		sum = 1.0;
	std::vector<double> val(bins);
	for (int i = 0; i < bins; ++i)
		val[i] = m_hist[i] / sum;
	for (int i = 0; i < bins; ++i)
	{
		int start = std::max(0, i - win);
		int end = std::min(bins - 1, i + win);
		double localSum = 0.0;
		for (int j = start; j <= end; ++j)
			localSum += val[j];
		double p = localSum / (end - start + 1);
		p = p + 15.0 * (val[i] - p);
		p = std::clamp(std::pow(p, 0.1), 0.0, 1.0);
		c = m_color * p + bg * (1 - p);
		data[i * 3] = static_cast<unsigned char>(c.r() * 255.0);
		data[i * 3 + 1] = static_cast<unsigned char>(c.g() * 255.0);
		data[i * 3 + 2] = static_cast<unsigned char>(c.b() * 255.0);
	}

	return true;
}

double VolumeData::GetAutoThreshold()
{
	if (m_auto_threshold < 0.0)
	{
		ComputeHistogram(true);
		if (m_hist.empty())
			return m_auto_threshold;
		size_t bins = m_hist.size() - 1;
		if (bins > 1)
		{
			// Compute mean and variance
			double sum = 0.0, sum_sq = 0.0, total = 0.0;
			for (size_t i = 0; i < bins; ++i)
			{
				double val = static_cast<double>(i) / (bins - 1);
				double freq = m_hist[i];
				sum += val * freq;
				sum_sq += val * val * freq;
				total += freq;
			}
			if (total == 0.0)
				total = 1.0;

			double mean = sum / total;
			double var = sqrt((sum_sq - 2.0 * mean * sum + total * mean * mean) / total);

			// Determine threshold
			if (var < glbin_settings.m_varth)
				m_auto_threshold = mean; // Low variance: use mean
			else
				m_auto_threshold = mean + var * glbin_settings.m_gauth; // Otherwise, offset from mean
		}
		else
		{
			m_auto_threshold = 0.0;
		}
	}
	return m_auto_threshold;
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

//resolution  scaling and spacing
fluo::Vector VolumeData::GetResolution(int lv)
{
	if (lv >= 0 && isBrxml() && m_tex)
	{
		return m_tex->get_res();
	}
	else
	{
		return m_size;
	}
}

void VolumeData::SetScaling(const fluo::Vector& scaling)
{
	m_scaling = scaling;
}

fluo::Vector VolumeData::GetScaling()
{
	return m_scaling;
}

void VolumeData::SetSpacing(const fluo::Vector& spacing)
{
	m_spacing = spacing;
	if (m_tex)
		m_tex->set_spacing(spacing);
	m_bounds = fluo::BBox(fluo::Point(0.0), fluo::Point(m_size * m_spacing));
	m_clipping_box.UpdateBoxes(m_bounds, fluo::BBox(fluo::Point(0.0), fluo::Point(m_size)));
}

fluo::Vector VolumeData::GetSpacing(int lv)
{
	return m_tex->get_spacing(lv);
}

//brkxml
void VolumeData::SetBaseSpacing(const fluo::Vector& spacing)
{
	SetSpacing(spacing);
}

fluo::Vector VolumeData::GetBaseSpacing()
{
	return GetSpacing();
}

void VolumeData::SetSpacingScale(const fluo::Vector& scaling)
{
	if (m_tex)
	{
		m_tex->set_spacing_scale(scaling);
		m_bounds.reset();
		m_tex->get_bounds(m_bounds);
		m_clipping_box.UpdateBoxes(m_bounds, fluo::BBox(fluo::Point(0.0), fluo::Point(m_size)));
	}
}

fluo::Vector VolumeData::GetSpacingScale()
{
	return m_tex->get_spacing_scale();
}

void VolumeData::SetLevel(int lv)
{
	if (m_tex && isBrxml())
	{
		m_tex->setLevel(lv);
		m_bounds.reset();
		m_tex->get_bounds(m_bounds);
	}
}

int VolumeData::GetLevel()
{
	if (m_tex && isBrxml())
		return m_tex->GetCurLevel();
	else
		return -1;
}

int VolumeData::GetLevelNum()
{
	if (m_tex && isBrxml())
		return m_tex->GetLevelNum();
	else
		return -1;
}

//bits
int VolumeData::GetBits()
{
	if (!m_tex)
		return 0;
	auto comp = m_tex->get_nrrd(flvr::CompType::Data);
	if (!comp.data)
		return 0;
	if (comp.data->type == nrrdTypeUChar)
		return 8;
	else if (comp.data->type == nrrdTypeUShort)
		return 16;
	return 0;
}

//display controls
void VolumeData::SetDisp(bool disp)
{
	m_disp = disp;
	m_tex->set_sort_bricks();
}

bool VolumeData::GetDisp()
{
	return m_disp;
}

void VolumeData::ToggleDisp()
{
	m_disp = !m_disp;
	m_tex->set_sort_bricks();
}

//bounding box
void VolumeData::SetDrawBounds(bool draw)
{
	m_draw_bounds = draw;
}

bool VolumeData::GetDrawBounds()
{
	return m_draw_bounds;
}

void VolumeData::ToggleDrawBounds()
{
	m_draw_bounds = !m_draw_bounds;
}

//wireframe
void VolumeData::SetWireframe(bool val)
{
	m_test_wiref = val;
}

//transparent
void VolumeData::SetTransparent(bool val)
{
	m_transparent = val;
	if (val)
		SetAlphaPower(2);
	else
		SetAlphaPower(1);
}

bool VolumeData::GetTransparent()
{
	return m_transparent;
}

void VolumeData::SetScalarScale(double val)
{
	m_scalar_scale = val;
	if (m_vr)
		m_vr->set_scalar_scale(val);
}

void VolumeData::SetGMScale(double val)
{
	m_gm_scale = val;
	if (m_vr)
		m_vr->set_gm_scale(val);
}

double VolumeData::GetMinValueScale()
{
	if (m_min_value > 0.0 && m_max_value > m_min_value)
		return m_min_value / m_max_value;
	ComputeHistogram(false);
	int bins = static_cast<int>(m_hist.size() - 1);
	if (bins <= 0)
		return 0.0;
	for (int i = 0; i < bins; ++i)
	{
		if (m_hist[i])
			return double(i) / bins;
	}
	return 0.0;
}

void VolumeData::SetClippingBox(const fluo::ClippingBox& box)
{
	TreeLayer::SetClippingBox(box);
	m_vr->set_clipping_box(m_clipping_box);
}

void VolumeData::SetClipValue(fluo::ClipPlane i, int val)
{
	TreeLayer::SetClipValue(i, val);
	m_vr->set_clipping_box(m_clipping_box);
}

void VolumeData::SetClipValues(fluo::ClipPlane i, int val1, int val2)
{
	TreeLayer::SetClipValues(i, val1, val2);
	m_vr->set_clipping_box(m_clipping_box);
}

void VolumeData::SetClipValues(const std::array<int, 6>& vals)
{
	TreeLayer::SetClipValues(vals);
	m_vr->set_clipping_box(m_clipping_box);
}

void VolumeData::ResetClipValues()
{
	TreeLayer::ResetClipValues();
	m_vr->set_clipping_box(m_clipping_box);
}

void VolumeData::ResetClipValues(fluo::ClipPlane i)
{
	TreeLayer::ResetClipValues(i);
	m_vr->set_clipping_box(m_clipping_box);
}

void VolumeData::SetClipRotation(int i, double val)
{
	TreeLayer::SetClipRotation(i, val);
	m_vr->set_clipping_box(m_clipping_box);
}

void VolumeData::SetClipRotation(const fluo::Vector& euler)
{
	TreeLayer::SetClipRotation(euler);
	m_vr->set_clipping_box(m_clipping_box);
}

void VolumeData::SetClipRotation(const fluo::Quaternion& q)
{
	TreeLayer::SetClipRotation(q);
	m_vr->set_clipping_box(m_clipping_box);
}

void VolumeData::SetLink(fluo::ClipPlane i, bool link)
{
	TreeLayer::SetLink(i, link);
	m_vr->set_clipping_box(m_clipping_box);
}

void VolumeData::ResetLink()
{
	TreeLayer::ResetLink();
	m_vr->set_clipping_box(m_clipping_box);
}

void VolumeData::SetLinkedDist(fluo::ClipPlane i, int val)
{
	TreeLayer::SetLinkedDist(i, val);
	m_vr->set_clipping_box(m_clipping_box);
}

//randomize color
void VolumeData::RandomizeColor()
{
	double hue = (double)std::rand()/(RAND_MAX) * 360.0;
	fluo::Color color(fluo::HSVColor(hue, 1.0, 1.0));
	SetColor(color);
}

//shown in legend
void VolumeData::SetLegend(bool val)
{
	m_legend = val;
}

bool VolumeData::GetLegend()
{
	return m_legend;
}

//interpolate
void VolumeData::SetInterpolate(bool val)
{
	if (m_vr)
		m_vr->set_interpolate(val);
	m_interpolate = val;
}

bool VolumeData::GetInterpolate()
{
	return m_interpolate;
}

void VolumeData::SetFog(bool use_fog,
	double fog_intensity, double fog_start, double fog_end)
{
	if (m_vr)
		m_vr->set_fog(use_fog, fog_intensity, fog_start, fog_end);
}

int VolumeData::GetAllBrickNum()
{
	if (!m_tex)
		return 0;
	return m_tex->get_brick_list_size();
}

bool VolumeData::isBrxml()
{
	if (!m_tex) return false;

	return m_tex->isBrxml();
}

//save label
void VolumeData::PushLabel(bool ret)
{
	if (ret && m_vr)
		m_vr->return_label();
	if (!m_tex)
		return;
	if (!m_tex->has_comp(flvr::CompType::Label))
		return;

	auto comp = m_tex->get_nrrd(flvr::CompType::Label);
	if (!comp.data || !comp.data->data)
		return;
	unsigned long long size = GetVoxelCount();
	if (!m_label_save)
		m_label_save = new unsigned int[size];
	memcpy(m_label_save, comp.data->data, size * sizeof(unsigned int));
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
		auto comp = m_tex->get_nrrd(flvr::CompType::Label);
		if (!comp.data || !comp.data->data)
			return;
		int nx, ny, nz;
		unsigned long long size = GetVoxelCount();
		memcpy(comp.data->data, m_label_save, size * sizeof(unsigned int));
		m_vr->clear_tex_current();
	}
}

void VolumeData::GetMlParams()
{
	if (!m_ep || !m_ep->getValid())
	{
		flrd::Histogram histogram(this);
		histogram.SetProgressFunc(glbin_data_manager.GetProgressFunc());
		histogram.SetUseMask(false);
		flrd::EntryHist* eh = histogram.GetEntryHist();
		if (!eh)
			return;
		//get entry from table
		flrd::TableHistParams& table = glbin.get_vp_table();
		m_ep = std::make_unique<flrd::EntryParams>(*table.infer(eh));
		delete eh;
		flrd::Reshape::clear();
	}
}

void VolumeData::ApplyMlVolProp()
{
	GetMlParams();
	//get histogram
	if (m_ep && m_ep->getValid())
	{
		double dval, dval2;
		int ival;

		//minmax
		if (m_minmax_enable)
		{
			//low
			dval = std::max(0.0f, m_ep->getParam("low_offset"));
			SetLowOffset(dval);
			//high offset
			dval = std::max(0.0f, m_ep->getParam("high_offset"));
			SetHighOffset(dval);
		}
		//gamma
		if (m_gamma_enable)
		{
			dval = std::max(0.0f, m_ep->getParam("gamma3d"));
			SetGamma(dval);
		}
		//alpha
		dval = m_ep->getParam("alpha_enable");
		SetAlphaEnable(dval > 0.5);
		if (m_alpha_enable)
		{
			dval = std::max(0.0f, m_ep->getParam("alpha"));
			SetAlpha(dval);
		}
		//luminance
		if (m_luminance_enable)
		{
			dval = std::max(0.0f, m_ep->getParam("luminance"));
			SetLuminance(dval);
		}
		//sample rate
		if (m_sample_rate_enable)
		{
			dval = std::max(0.1f, m_ep->getParam("sample_rate"));
			SetSampleRate(dval);
		}
		//threshold
		if (m_thresh_enable)
		{
			//low thresholding
			dval = std::max(0.0f, m_ep->getParam("low_threshold"));
			SetLeftThresh(dval);
			//high thresholding
			dval = std::max(0.0f, m_ep->getParam("high_threshold"));
			SetRightThresh(dval);
		}
		//extract boundary
		if (m_boundary_enable)
		{
			dval = std::max(0.0f, m_ep->getParam("extract_boundary"));
			SetBoundaryLow(dval);
			SetBoundaryMax(m_boundary_max);
		}
		//enable shading
		dval = m_ep->getParam("shading_enable");
		SetShadingEnable(dval > 0.5);
		if (m_shading_enable)
		{
			//low shading
			dval = std::max(0.0f, m_ep->getParam("low_shading"));
			SetShadingStrength(dval);
			//high shading
			dval = std::max(0.0f, m_ep->getParam("high_shading"));
			SetShadingShine(dval);
		}
		//shadow
		dval = m_ep->getParam("shadow_enable");
		SetShadowEnable(dval > 0.5);
		if (m_shadow_enable)
		{
			//shadow intensity
			dval = std::max(0.0f, m_ep->getParam("shadow_intensity"));
			SetShadowIntensity(dval);
		}
		//colormap enable
		dval = m_ep->getParam("colormap_enable");
		SetColorMode(dval>0.5 ? flvr::ColorMode::Colormap : flvr::ColorMode::SingleColor);
		if (m_color_mode == flvr::ColorMode::Colormap)
		{
			//colormap inv
			dval = m_ep->getParam("colormap_inv");
			SetColormapInv(dval > 0.5 ? -1.0 : 1.0);
			//colormap type
			dval = m_ep->getParam("colormap_type");
			SetColormap(std::round(dval));
			//colormap projection
			dval = m_ep->getParam("colormap_proj");
			ival = static_cast<int>(std::round(dval));
			auto colormap_proj = static_cast<flvr::ColormapProj>(ival);
			SetColormapProj(colormap_proj);
			//colormap low value
			dval = std::max(0.0f, m_ep->getParam("colormap_low"));
			//colormap high value
			dval2 = std::max(0.0f, m_ep->getParam("colormap_hi"));
			SetColormapValues(dval, dval2);
		}
		//inversion
		dval = m_ep->getParam("invert_enable");
		SetInvert(dval > 0.5);
		//enable mip
		dval = m_ep->getParam("mip_enable");
		flvr::RenderMode mode = dval > 0.5 ? flvr::RenderMode::Mip : flvr::RenderMode::Standard;
		SetRenderMode(mode);
		//enable hi transp
		dval = m_ep->getParam("transparent_enable");
		SetAlphaPower(dval > 0.5 ? 2.0 : 1.0);
		//interpolation
		//dval = m_ep->getParam("interp_enable");
		//SetInterpolate(dval > 0.5);
		//noise reduction
		//dval = m_ep->getParam("denoise_enable");
		//SetNR(dval > 0.5);
	}
}

void VolumeData::SetMaskCount(unsigned int sum, float wsum)
{
	m_mask_sum = sum;
	m_mask_wsum = wsum;
	m_mask_count_dirty = false;
}

