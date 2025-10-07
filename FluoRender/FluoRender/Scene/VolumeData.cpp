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
#include <VolumeRenderer.h>
#include <VolumeSampler.h>
#include <VolumeBaker.h>
#include <Count.h>
#include <VolCache4D.h>
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
	m_mode = 0;
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
	m_mat_amb = 1.0;
	m_mat_diff = 1.0;
	m_mat_spec = 1.0;
	m_mat_shine = 10;

	//shadow
	m_shadow_enable = false;
	m_shadow_intensity = 0.0;

	m_sample_rate_enable = true;
	m_sample_rate = 2.0;

	m_color = fluo::Color(1.0, 1.0, 1.0);
	m_wl_color = false;
	SetHSV();

	//noise reduction
	m_noise_rd = false;

	//colormap mode
	m_colormap_inv = 1.0;
	m_colormap_mode = 0;
	m_colormap_disp = false;
	m_colormap = 0;
	m_colormap_proj = 0;
	m_colormap_low_value = 0.0;
	m_colormap_hi_value = 1.0;
	UpdateColormapRange();

	//blend mode
	m_blend_mode = 0;

	m_saved_mode = 0;

	//resolution, scaling, spacing
	m_res_x = 0;	m_res_y = 0;	m_res_z = 0;
	m_sclx = 1.0;	m_scly = 1.0;	m_sclz = 1.0;
	m_spcx = 1.0;	m_spcy = 1.0;	m_spcz = 1.0;
	m_spc_from_file = false;

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
	//resize
	m_resize = false;
	m_rnx = 0;
	m_rny = 0;
	m_rnz = 0;

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

	//m_clip
	for (int i : { 0, 1, 2 })
		m_clip_dist[i] = 1;

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

	//volume renderer and texture
	m_tex = copy.m_tex;

	//current channel index
	m_chan = copy.m_chan;
	m_time = 0;

	//mdoes
	m_mode = copy.m_mode;
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
	m_mat_amb = copy.m_mat_amb;
	m_mat_diff = copy.m_mat_diff;
	m_mat_spec = copy.m_mat_spec;
	m_mat_shine = copy.m_mat_shine;

	//shadow
	m_shadow_enable = copy.m_shadow_enable;
	m_shadow_intensity = copy.m_shadow_intensity;

	m_sample_rate_enable = copy.m_sample_rate_enable;
	m_sample_rate = copy.m_sample_rate;

	m_color = copy.m_color;
	m_wl_color = copy.m_wl_color;
	SetHSV();

	//noise reduction
	m_noise_rd = copy.m_noise_rd;

	//colormap mode
	m_colormap_inv = copy.m_colormap_inv;
	m_colormap_mode = copy.m_colormap_mode;
	m_colormap_disp = copy.m_colormap_disp;
	m_colormap = copy.m_colormap;
	m_colormap_proj = copy.m_colormap_proj;
	m_colormap_low_value = copy.m_colormap_low_value;
	m_colormap_hi_value = copy.m_colormap_hi_value;
	UpdateColormapRange();

	//blend mode
	m_blend_mode = copy.m_blend_mode;

	m_saved_mode = copy.m_saved_mode;

	//resolution, scaling, spacing
	m_res_x = copy.m_res_x;	m_res_y = copy.m_res_y;	m_res_z = copy.m_res_z;
	m_sclx = copy.m_sclx;	m_scly = copy.m_scly;	m_sclz = copy.m_sclz;
	m_spcx = copy.m_spcx;	m_spcy = copy.m_spcy;	m_spcz = copy.m_spcz;
	m_spc_from_file = copy.m_spc_from_file;

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
	//resize
	m_resize = false;
	m_rnx = 0;
	m_rny = 0;
	m_rnz = 0;

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

	//m_clip
	for (int i : { 0, 1, 2 })
		m_clip_dist[i] = copy.m_clip_dist[i];

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
void VolumeData::SetViewport(GLint vp[4])
{
	if (m_vr)
		m_vr->set_viewport(vp);
}

//set clear color
void VolumeData::SetClearColor(GLfloat clear_color[4])
{
	if (m_vr)
		m_vr->set_clear_color(clear_color);
}

//set current framebuffer
void VolumeData::SetCurFramebuffer(GLuint cur_framebuffer)
{
	if (m_vr)
		m_vr->set_cur_framebuffer(cur_framebuffer);
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
	m_res_x = nv->axis[0].size;
	m_res_y = nv->axis[1].size;
	m_res_z = nv->axis[2].size;

	fluo::BBox bounds;
	fluo::Point pmax(data->axis[0].max, data->axis[1].max, data->axis[2].max);
	fluo::Point pmin(data->axis[0].min, data->axis[1].min, data->axis[2].min);
	bounds.extend(pmin);
	bounds.extend(pmax);
	m_bounds = bounds;

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
		if (!m_tex->build(nv, gm, 0, 256, 0, 0))
			return 0;
	}

	if (m_tex)
	{
		std::vector<fluo::Plane*> planelist(0);
		fluo::Plane* plane = 0;
		//x
		plane = new fluo::Plane(fluo::Point(0.0, 0.0, 0.0), fluo::Vector(1.0, 0.0, 0.0));
		planelist.push_back(plane);
		plane = new fluo::Plane(fluo::Point(1.0, 0.0, 0.0), fluo::Vector(-1.0, 0.0, 0.0));
		planelist.push_back(plane);
		//y
		plane = new fluo::Plane(fluo::Point(0.0, 0.0, 0.0), fluo::Vector(0.0, 1.0, 0.0));
		planelist.push_back(plane);
		plane = new fluo::Plane(fluo::Point(0.0, 1.0, 0.0), fluo::Vector(0.0, -1.0, 0.0));
		planelist.push_back(plane);
		//z
		plane = new fluo::Plane(fluo::Point(0.0, 0.0, 0.0), fluo::Vector(0.0, 0.0, 1.0));
		planelist.push_back(plane);
		plane = new fluo::Plane(fluo::Point(0.0, 0.0, 1.0), fluo::Vector(0.0, 0.0, -1.0));
		planelist.push_back(plane);

		m_vr = std::make_unique<flvr::VolumeRenderer>(planelist);
		m_vr->set_texture(m_tex);
		m_vr->set_sample_rate(m_sample_rate);
		m_vr->set_material(m_mat_amb, m_mat_diff, m_mat_spec, m_mat_shine);
		m_vr->set_shading(true);
		m_vr->set_scalar_scale(m_scalar_scale);
		m_vr->set_gm_scale(m_scalar_scale);

		SetMode(m_mode);
	}

	//clip distance
	m_clip_dist[0] = std::max(1, m_res_x / 20);
	m_clip_dist[1] = std::max(1, m_res_y / 20);
	m_clip_dist[2] = std::max(1, m_res_z / 20);

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
		Nrrd *gm = 0;
		m_res_x = nv->axis[0].size;
		m_res_y = nv->axis[1].size;
		m_res_z = nv->axis[2].size;

		m_tex = std::make_shared<flvr::Texture>();
		m_tex->set_use_priority(m_skip_brick);
		m_tex->build(nv, gm, m_min_value, m_max_value, 0, 0);
	}
	else
	{
		//set new
		m_tex->set_nrrd(data, 0);
	}

	if (m_vr)
		m_vr->set_texture(m_tex);

	m_bg_valid = false;

	//clip distance
	m_clip_dist[0] = std::max(1, m_res_x / 20);
	m_clip_dist[1] = std::max(1, m_res_y / 20);
	m_clip_dist[2] = std::max(1, m_res_z / 20);

	m_hist_dirty = true;

	ResetVolume();

	return 1;
}

int VolumeData::Replace(VolumeData* data)
{
	if (!data ||
		m_res_x!=data->m_res_x ||
		m_res_y!=data->m_res_y ||
		m_res_z!=data->m_res_z)
		return 0;

	double spcx = 1.0, spcy = 1.0, spcz = 1.0;

	if (m_tex && m_vr)
	{
		m_tex->get_spacings(spcx, spcy, spcz);
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
	int nx, int ny, int nz,
	double spcx, double spcy, double spcz,
	int brick_size)
{
	if (bits!=8 && bits!=16)
		return;

	Nrrd *nv = nrrdNew();
	if (bits == 8)
	{
		unsigned long long mem_size = (unsigned long long)nx*
			(unsigned long long)ny*(unsigned long long)nz;
		uint8_t *val8 = new (std::nothrow) uint8_t[mem_size]();
		if (!val8)
		{
			//SetProgress("Not enough memory. Please save project and restart.");
			return;
		}
		nrrdWrap_va(nv, val8, nrrdTypeUChar, 3, (size_t)nx, (size_t)ny, (size_t)nz);
	}
	else if (bits == 16)
	{
		unsigned long long mem_size = (unsigned long long)nx*
			(unsigned long long)ny*(unsigned long long)nz;
		uint16_t *val16 = new (std::nothrow) uint16_t[mem_size]();
		if (!val16)
		{
			//SetProgress("Not enough memory. Please save project and restart.");
			return;
		}
		nrrdWrap_va(nv, val16, nrrdTypeUShort, 3, (size_t)nx, (size_t)ny, (size_t)nz);
	}
	nrrdAxisInfoSet_va(nv, nrrdAxisInfoSpacing, spcx, spcy, spcz);
	nrrdAxisInfoSet_va(nv, nrrdAxisInfoMax, spcx*nx, spcy*ny, spcz*nz);
	nrrdAxisInfoSet_va(nv, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
	nrrdAxisInfoSet_va(nv, nrrdAxisInfoSize, (size_t)nx, (size_t)ny, (size_t)nz);

	//resolution
	m_res_x = nv->axis[0].size;
	m_res_y = nv->axis[1].size;
	m_res_z = nv->axis[2].size;

	//bounding box
	fluo::BBox bounds;
	fluo::Point pmax(nv->axis[0].max, nv->axis[1].max, nv->axis[2].max);
	fluo::Point pmin(nv->axis[0].min, nv->axis[1].min, nv->axis[2].min);
	bounds.extend(pmin);
	bounds.extend(pmax);
	m_bounds = bounds;

	//create texture
	m_tex = std::make_shared<flvr::Texture>();
	m_tex->set_use_priority(false);
	m_tex->set_brick_size(brick_size);
	m_tex->build(nv, 0, 0, 256, 0, 0);
	m_tex->set_spacings(spcx, spcy, spcz);

	//clipping planes
	std::vector<fluo::Plane*> planelist(0);
	fluo::Plane* plane = 0;
	//x
	plane = new fluo::Plane(fluo::Point(0.0, 0.0, 0.0), fluo::Vector(1.0, 0.0, 0.0));
	planelist.push_back(plane);
	plane = new fluo::Plane(fluo::Point(1.0, 0.0, 0.0), fluo::Vector(-1.0, 0.0, 0.0));
	planelist.push_back(plane);
	//y
	plane = new fluo::Plane(fluo::Point(0.0, 0.0, 0.0), fluo::Vector(0.0, 1.0, 0.0));
	planelist.push_back(plane);
	plane = new fluo::Plane(fluo::Point(0.0, 1.0, 0.0), fluo::Vector(0.0, -1.0, 0.0));
	planelist.push_back(plane);
	//z
	plane = new fluo::Plane(fluo::Point(0.0, 0.0, 0.0), fluo::Vector(0.0, 0.0, 1.0));
	planelist.push_back(plane);
	plane = new fluo::Plane(fluo::Point(0.0, 0.0, 1.0), fluo::Vector(0.0, 0.0, -1.0));
	planelist.push_back(plane);

	//create volume renderer
	m_vr = std::make_unique<flvr::VolumeRenderer>(planelist);
	m_vr->set_texture(m_tex);
	m_vr->set_sample_rate(m_sample_rate);
	m_vr->set_material(m_mat_amb, m_mat_diff, m_mat_spec, m_mat_shine);
	m_vr->set_shading(true);
	m_vr->set_scalar_scale(m_scalar_scale);
	m_vr->set_gm_scale(m_scalar_scale);

	SetMode(m_mode);
	m_bg_valid = false;

	//clip distance
	m_clip_dist[0] = std::max(1, m_res_x / 20);
	m_clip_dist[1] = std::max(1, m_res_y / 20);
	m_clip_dist[2] = std::max(1, m_res_z / 20);

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
	m_tex->set_nrrd(mask, m_tex->nmask());

	int nx2, ny2, nz2;
	nx2 = mask->axis[0].size;
	ny2 = mask->axis[1].size;
	nz2 = mask->axis[2].size;
	if (m_res_x != nx2 || m_res_y != ny2 || m_res_z != nz2)
	{
		flrd::VolumeSampler sampler;
		sampler.SetInput(shared_from_this());
		sampler.SetSize(m_res_x, m_res_y, m_res_z);
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
	unsigned long long mem_size = (unsigned long long)m_res_x*
		(unsigned long long)m_res_y*(unsigned long long)m_res_z;
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
		double spcx, spcy, spcz;
		m_tex->get_spacings(spcx, spcy, spcz);
		nrrdWrap_va(nrrd_mask, val8, nrrdTypeUChar, 3, (size_t)m_res_x, (size_t)m_res_y, (size_t)m_res_z);
		nrrdAxisInfoSet_va(nrrd_mask, nrrdAxisInfoSize, (size_t)m_res_x, (size_t)m_res_y, (size_t)m_res_z);
		nrrdAxisInfoSet_va(nrrd_mask, nrrdAxisInfoSpacing, spcx, spcy, spcz);
		nrrdAxisInfoSet_va(nrrd_mask, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
		nrrdAxisInfoSet_va(nrrd_mask, nrrdAxisInfoMax, spcx*m_res_x, spcy*m_res_y, spcz*m_res_z);

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
		mask->axis[0].size != m_res_x ||
		mask->axis[1].size != m_res_y ||
		mask->axis[2].size != m_res_z)
		return;

	Nrrd *nrrd_mask = 0;
	uint8_t *val8 = 0;
	unsigned long long mem_size = (unsigned long long)m_res_x*
		(unsigned long long)m_res_y*(unsigned long long)m_res_z;
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
		double spcx, spcy, spcz;
		m_tex->get_spacings(spcx, spcy, spcz);
		nrrdWrap_va(nrrd_mask, val8, nrrdTypeUChar, 3, (size_t)m_res_x, (size_t)m_res_y, (size_t)m_res_z);
		nrrdAxisInfoSet_va(nrrd_mask, nrrdAxisInfoSize, (size_t)m_res_x, (size_t)m_res_y, (size_t)m_res_z);
		nrrdAxisInfoSet_va(nrrd_mask, nrrdAxisInfoSpacing, spcx, spcy, spcz);
		nrrdAxisInfoSet_va(nrrd_mask, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
		nrrdAxisInfoSet_va(nrrd_mask, nrrdAxisInfoMax, spcx*m_res_x, spcy*m_res_y, spcz*m_res_z);

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
	if (mask->dim != 3 ||
		mask->axis[0].size != m_res_x ||
		mask->axis[1].size != m_res_y ||
		mask->axis[2].size != m_res_z)
		return;

	Nrrd *nrrd_mask = 0;
	uint8_t *val8 = 0;
	unsigned long long mem_size = (unsigned long long)m_res_x*
		(unsigned long long)m_res_y*(unsigned long long)m_res_z;
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
		double spcx, spcy, spcz;
		m_tex->get_spacings(spcx, spcy, spcz);
		nrrdWrap_va(nrrd_mask, val8, nrrdTypeUChar, 3, (size_t)m_res_x, (size_t)m_res_y, (size_t)m_res_z);
		nrrdAxisInfoSet_va(nrrd_mask, nrrdAxisInfoSize, (size_t)m_res_x, (size_t)m_res_y, (size_t)m_res_z);
		nrrdAxisInfoSet_va(nrrd_mask, nrrdAxisInfoSpacing, spcx, spcy, spcz);
		nrrdAxisInfoSet_va(nrrd_mask, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
		nrrdAxisInfoSet_va(nrrd_mask, nrrdAxisInfoMax, spcx*m_res_x, spcy*m_res_y, spcz*m_res_z);

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

//volume label
void VolumeData::LoadLabel(Nrrd* label)
{
	if (!label || !m_tex || !m_vr)
		return;

	m_tex->add_empty_label();
	m_tex->set_nrrd(label, m_tex->nlabel());

	int nx2, ny2, nz2;
	nx2 = label->axis[0].size;
	ny2 = label->axis[1].size;
	nz2 = label->axis[2].size;
	if (m_res_x != nx2 || m_res_y != ny2 || m_res_z != nz2)
	{
		flrd::VolumeSampler sampler;
		sampler.SetInput(shared_from_this());
		sampler.SetSize(m_res_x, m_res_y, m_res_z);
		sampler.Resize(flrd::SDT_Label, true);
		//nrrdNuke(label);
		//label = sampler.GetResult();
	}
}

void VolumeData::SetOrderedID(unsigned int* val)
{
	for (int i=0; i<m_res_x; i++)
		for (int j=0; j<m_res_y; j++)
			for (int k=0; k<m_res_z; k++)
			{
				unsigned int index = m_res_y*m_res_z*i + m_res_z*j + k;
				val[index] = index+1;
			}
}

void VolumeData::SetReverseID(unsigned int* val)
{
	for (int i=0; i<m_res_x; i++)
		for (int j=0; j<m_res_y; j++)
			for (int k=0; k<m_res_z; k++)
			{
				unsigned int index = m_res_y*m_res_z*i + m_res_z*j + k;
				val[index] = m_res_x*m_res_y*m_res_z - index;
			}
}

void VolumeData::SetShuffledID(unsigned int* val)
{
	unsigned int x, y, z;
	unsigned int res;
	unsigned int len = 0;
	unsigned int r = std::max(m_res_x, std::max(m_res_y, m_res_z));
	while (r > 0)
	{
		r /= 2;
		len++;
	}
	for (int i=0; i<m_res_x; i++)
		for (int j=0; j<m_res_y; j++)
			for (int k=0; k<m_res_z; k++)
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
				unsigned int index = m_res_x*m_res_y*k + m_res_x*j + i;
				val[index] = m_res_x*m_res_y*m_res_z - res;
			}
}

void VolumeData::UpdateColormapRange()
{
	switch (m_colormap_proj)
	{
	case 0://intensity
	default:
		m_colormap_min_value = m_min_value;
		m_colormap_max_value = m_max_value;
		break;
	case 1://z-value
		m_colormap_min_value = 0;
		m_colormap_max_value = m_res_z * m_spcz;
		break;
	case 2://y-value
		m_colormap_min_value = 0;
		m_colormap_max_value = m_res_y * m_spcy;
		break;
	case 3://x-value
		m_colormap_min_value = 0;
		m_colormap_max_value = m_res_x * m_spcx;
		break;
	case 4://t-value
		m_colormap_min_value = 0;
		if (auto reader = m_reader.lock())
			m_colormap_max_value = reader->GetTimeNum();
		break;
	case 5://gradient magnitude
	case 6://gradient dir
		m_colormap_min_value = 0;
		m_colormap_max_value = 1;
		break;
	case 7://intensity delta
		m_colormap_min_value = -m_max_value;
		m_colormap_max_value = m_max_value;
		break;
	case 8://speed
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
		unsigned long long mem_size = (unsigned long long)m_res_x*
			(unsigned long long)m_res_y*(unsigned long long)m_res_z;
		val32 = new (std::nothrow) unsigned int[mem_size];
		if (!val32)
		{
			//SetProgress("Not enough memory. Please save project and restart.");
			return;
		}

		double spcx, spcy, spcz;
		m_tex->get_spacings(spcx, spcy, spcz);
		nrrdWrap_va(nrrd_label, val32, nrrdTypeUInt, 3, (size_t)m_res_x, (size_t)m_res_y, (size_t)m_res_z);
		nrrdAxisInfoSet_va(nrrd_label, nrrdAxisInfoSpacing, spcx, spcy, spcz);
		nrrdAxisInfoSet_va(nrrd_label, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
		nrrdAxisInfoSet_va(nrrd_label, nrrdAxisInfoMax, spcx*m_res_x, spcy*m_res_y, spcz*m_res_z);
		nrrdAxisInfoSet_va(nrrd_label, nrrdAxisInfoSize, (size_t)m_res_x, (size_t)m_res_y, (size_t)m_res_z);

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
			std::memset(val32, 0, sizeof(unsigned int)*m_res_x*m_res_y*m_res_z);
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

	unsigned long long for_size = (unsigned long long)m_res_x *
		(unsigned long long)m_res_y * (unsigned long long)m_res_z;
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
		return m_tex->get_nrrd(0);
	}

	return 0;
}

Nrrd* VolumeData::GetMask(bool ret)
{
	if (m_vr && m_tex && m_tex->nmask()!=-1)
	{
		if (ret) m_vr->return_mask();
		return m_tex->get_nrrd(m_tex->nmask());
	}

	return 0;
}

bool VolumeData::IsValidMask()
{
	if (!m_tex)
		return false;
	if (m_tex->nmask() == -1)
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
	if (m_vr && m_tex && m_tex->nlabel() != -1)
	{
		if (ret) m_vr->return_label();
		return m_tex->get_nrrd(m_tex->nlabel());
	}

	return 0;
}

double VolumeData::GetOriginalValue(int i, int j, int k, flvr::TextureBrick* b)
{
	void *data_data = 0;
	int bits = 8;
	int64_t nx, ny, nz;

	if (isBrxml())
	{
		if (!b || !b->isLoaded()) return 0.0;
		flvr::FileLocInfo *finfo = m_tex->GetFileName(b->getID());
		data_data = b->tex_data_brk(0, finfo);
		if (!data_data) return 0.0;
		bits = b->nb(0) * 8;
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
		if (data->type == nrrdTypeUChar)
			bits = 8;
		else if (data->type == nrrdTypeUShort)
			bits = 16;
		nx = (int64_t)(data->axis[0].size);
		ny = (int64_t)(data->axis[1].size);
		nz = (int64_t)(data->axis[2].size);
	}

	if (i<0 || i>=nx || j<0 || j>=ny || k<0 || k>=nz)
		return 0.0;
	uint64_t ii = i, jj = j, kk = k;

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

double VolumeData::GetMaskValue(int i, int j, int k, flvr::TextureBrick* b)
{
	void *data_data = 0;
	int bits = 8;
	int64_t nx, ny, nz;
	int nmask = m_tex->nmask();

	if (isBrxml())
	{
		if (!b || !b->isLoaded()) return 0.0;
		flvr::FileLocInfo *finfo = m_tex->GetFileName(b->getID());
		data_data = b->tex_data_brk(nmask, finfo);
		if (!data_data) return 0.0;
		bits = b->nb(0) * 8;
		nx = b->nx();
		ny = b->ny();
		nz = b->nz();
	}
	else
	{
		Nrrd* data = 0;
		data = m_tex->get_nrrd(nmask);
		if (!data || !data->data) return 0.0;
		data_data = data->data;
		if (data->type == nrrdTypeUChar)
			bits = 8;
		else if (data->type == nrrdTypeUShort)
			bits = 16;
		nx = (int64_t)(data->axis[0].size);
		ny = (int64_t)(data->axis[1].size);
		nz = (int64_t)(data->axis[2].size);
	}

	if (i<0 || i>=nx || j<0 || j>=ny || k<0 || k>=nz)
		return 0.0;
	uint64_t ii = i, jj = j, kk = k;

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

double VolumeData::GetTransferedValue(int i, int j, int k, flvr::TextureBrick* b)
{
	void *data_data = 0;
	int bits = 8;
	int64_t nx, ny, nz;

	if (isBrxml())
	{
		if (!b || !b->isLoaded()) return 0.0;
		flvr::FileLocInfo *finfo = m_tex->GetFileName(b->getID());
		data_data = b->tex_data_brk(0, finfo);
		if (!data_data) return 0.0;
		bits = b->nb(0) * 8;
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
		if (data->type == nrrdTypeUChar)
			bits = 8;
		else if (data->type == nrrdTypeUShort)
			bits = 16;
		nx = (int64_t)(data->axis[0].size);
		ny = (int64_t)(data->axis[1].size);
		nz = (int64_t)(data->axis[2].size);
	}

	if (i<0 || i>=nx || j<0 || j>=ny || k<0 || k>=nz)
		return 0.0;
	int64_t ii = i, jj = j, kk = k;

	if (bits == 8)
	{
		uint64_t index = nx*ny*kk + nx*jj + ii;
		uint8_t old_value = ((uint8_t*)(data_data))[index];
		double gm = 0.0;
		double new_value = double(old_value)/255.0;
		if (m_vr->get_inversion())
			new_value = 1.0-new_value;
		if (i>0 && i<nx-1 &&
			j>0 && j<ny-1 &&
			k>0 && k<nz-1)
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

void VolumeData::SetResize(int resize, int nx, int ny, int nz)
{
	if (resize > -1)
		m_resize = resize > 0;
	if (nx > -1)
		m_rnx = nx;
	if (ny > -1)
		m_rny = ny;
	if (nz > -1)
		m_rnz = nz;
}

void VolumeData::GetResize(bool &resize, int &nx, int &ny, int &nz)
{
	resize = m_resize;
	nx = m_rnx;
	ny = m_rny;
	nz = m_rnz;
}

//save
void VolumeData::Save(const std::wstring &filename, int mode,
	int mask, bool neg_mask,
	bool crop, int filter,
	bool bake, bool compress,
	const fluo::Point &c,//rotation center
	const fluo::Quaternion &q,//rotation
	const fluo::Point &t,//translate
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

	if (m_resize || crop)
	{
		flrd::VolumeSampler sampler;
		sampler.SetInput(temp ? temp : shared_from_this());
		sampler.SetFixSize(fix_size);
		sampler.SetSize(m_rnx, m_rny, m_rnz);
		sampler.SetFilter(filter);
		sampler.SetFilterSize(1, 1, 1);
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

	if (m_resize || crop)
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
	double spcx, spcy, spcz;
	GetSpacings(spcx, spcy, spcz);

	//save mask
	data = GetMask(true);
	if (!data)
		return;

	MSKWriter msk_writer;
	msk_writer.SetData(data);
	msk_writer.SetSpacings(spcx, spcy, spcz);
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
	double spcx, spcy, spcz;
	GetSpacings(spcx, spcy, spcz);

	//save label
	data = GetLabel(true);
	if (!data)
		return;

	MSKWriter msk_writer;
	msk_writer.SetData(data);
	msk_writer.SetSpacings(spcx, spcy, spcz);
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
	std::vector<fluo::Plane*> *planes = m_vr->get_planes();
	if (planes->size() != 6)
		return m_bounds;

	//calculating planes
	//get six planes
	fluo::Plane* px1 = (*planes)[0];
	fluo::Plane* px2 = (*planes)[1];
	fluo::Plane* py1 = (*planes)[2];
	fluo::Plane* py2 = (*planes)[3];
	fluo::Plane* pz1 = (*planes)[4];
	fluo::Plane* pz2 = (*planes)[5];

	fluo::Vector diff =
		m_bounds.Max() - m_bounds.Min();
	fluo::Point min = fluo::Point(
		m_bounds.Min().x() - diff.x()*px1->d(),
		m_bounds.Min().y() - diff.y()*py1->d(),
		m_bounds.Min().z() - diff.z()*pz1->d());
	fluo::Point max = fluo::Point(
		m_bounds.Min().x() + diff.x()*px2->d(),
		m_bounds.Min().y() + diff.y()*py2->d(),
		m_bounds.Min().z() + diff.z()*pz2->d());

	return fluo::BBox(min, max);
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
void VolumeData::SetMode(int mode)
{
	if (!m_vr)
		return;

	m_saved_mode = m_mode;
	m_mode = mode;

	switch (mode)
	{
	case 0://normal
		m_vr->set_mode(flvr::RenderMode::RENDER_MODE_OVER);
		m_vr->set_color(m_color);
		break;
	case 1://MIP
		m_vr->set_mode(flvr::RenderMode::RENDER_MODE_MIP);
		m_vr->set_color(m_color);
		break;
	case 2://white shading
		m_vr->set_mode(flvr::RenderMode::RENDER_MODE_OVER);
		m_vr->set_colormap_mode(0);
		m_vr->set_color(fluo::Color(1.0, 1.0, 1.0));
		break;
	case 3://white mip
		m_vr->set_mode(flvr::RenderMode::RENDER_MODE_MIP);
		m_vr->set_colormap_mode(0);
		m_vr->set_color(fluo::Color(1.0, 1.0, 1.0));
		break;
	}
}

int VolumeData::GetMode()
{
	return m_mode;
}

void VolumeData::RestoreMode()
{
	SetMode(m_saved_mode);
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
	glm::mat4 scale_mv = glm::scale(mv_mat, glm::vec3(m_sclx, m_scly, m_sclz));
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
void VolumeData::Set2dMask(GLuint mask)
{
	m_2d_mask = mask;
}

//set 2d weight map for segmentation
void VolumeData::Set2DWeight(GLuint weight1, GLuint weight2)
{
	m_2d_weight1 = weight1;
	m_2d_weight2 = weight2;
}

//set 2d depth map for rendering shadows
void VolumeData::Set2dDmap(GLuint dmap)
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

fluo::Color VolumeData::SetLuminance(double val, bool set_this)
{
	if (set_this)
		m_luminance = val;
	double h, s, v;
	GetHSV(h, s, v);
	fluo::HSVColor hsv(h, s, val);
	m_color = fluo::Color(hsv);
	if (m_vr)
		m_vr->set_color(m_color);
	return m_color;
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

void VolumeData::SetMaterial(double amb, double diff, double spec, double shine)
{
	m_mat_amb = amb;
	m_mat_diff = diff;
	m_mat_spec = spec;
	m_mat_shine = shine;
	if (m_vr)
		m_vr->set_material(m_mat_amb, m_mat_diff, m_mat_spec, m_mat_shine);
}

void VolumeData::GetMaterial(double& amb, double& diff, double& spec, double& shine)
{
	amb = m_mat_amb;
	diff = m_mat_diff;
	spec = m_mat_spec;
	shine = m_mat_shine;
}

void VolumeData::SetLowShading(double val)
{
	double amb, diff, spec, shine;
	GetMaterial(amb, diff, spec, shine);
	SetMaterial(val, diff, spec, shine);
}

void VolumeData::SetHiShading(double val)
{
	double amb, diff, spec, shine;
	GetMaterial(amb, diff, spec, shine);
	SetMaterial(amb, diff, spec, val);
}

double VolumeData::GetLowShading()
{
	double amb, diff, spec, shine;
	GetMaterial(amb, diff, spec, shine);
	return amb;
}

double VolumeData::GetMlLowShading()
{
	GetMlParams();

	if (m_ep && m_ep->getValid())
		return m_ep->getParam("low_shading");
	else
		return glbin_vol_def.m_low_shading;
}

double VolumeData::GetHiShading()
{
	double amb, diff, spec, shine;
	GetMaterial(amb, diff, spec, shine);
	return shine;
}

double VolumeData::GetMlHiShading()
{
	GetMlParams();

	if (m_ep && m_ep->getValid())
		return m_ep->getParam("high_shading");
	else
		return glbin_vol_def.m_high_shading;
}

//shadow
void VolumeData::SetShadowEnable(bool bVal)
{
	m_shadow_enable = bVal;
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

void VolumeData::SetColor(const fluo::Color &color, bool update_hsv)
{
	m_color = color;
	if (update_hsv)
		SetHSV();
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

void VolumeData::SetHSV(double hue, double sat, double val)
{
	if (hue < 0 || sat < 0 || val < 0)
	{
		m_hsv = fluo::HSVColor(m_color);
	}
	else
	{
		m_hsv = fluo::HSVColor(hue, sat, val);
	}
	m_luminance = m_hsv.val();
}

void VolumeData::GetHSV(double &hue, double &sat, double &val)
{
	hue = m_hsv.hue();
	sat = m_hsv.sat();
	val = m_hsv.val();
}

void VolumeData::SetHSVColor(const fluo::HSVColor& hsv)
{
	m_hsv = hsv;
	m_luminance = m_hsv.val();
}

fluo::HSVColor VolumeData::GetHSVColor()
{
	return m_hsv;
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
void VolumeData::SetColormapMode(int mode)
{
	m_colormap_mode = mode;
	if (m_vr)
	{
		m_vr->set_colormap_mode(m_colormap_mode);
		m_vr->set_color(m_color);
	}
}

int VolumeData::GetColormapMode()
{
	return m_colormap_mode;
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

void VolumeData::SetColormapProj(int value)
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

int VolumeData::GetColormapProj()
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
void VolumeData::GetResolution(int &res_x, int &res_y, int &res_z, int lv)
{
	if (lv >= 0 && isBrxml() && m_tex)
	{
		res_x = m_tex->nx();
		res_y = m_tex->ny();
		res_z = m_tex->nz();
	}
	else
	{
		res_x = m_res_x;
		res_y = m_res_y;
		res_z = m_res_z;
	}
}

void VolumeData::SetScalings(double sclx, double scly, double sclz)
{
	m_sclx = sclx; m_scly = scly; m_sclz = sclz;
}

void VolumeData::GetScalings(double &sclx, double &scly, double &sclz)
{
	sclx = m_sclx; scly = m_scly; sclz = m_sclz;
}

fluo::Vector VolumeData::GetScalings()
{
	return fluo::Vector(m_sclx, m_scly, m_sclz);
}

void VolumeData::SetSpacings(double spcx, double spcy, double spcz)
{
	if (GetTexture())
	{
		GetTexture()->set_spacings(spcx, spcy, spcz);
		m_bounds.reset();
		GetTexture()->get_bounds(m_bounds);
	}
}

void VolumeData::GetSpacings(double &spcx, double &spcy, double & spcz, int lv)
{
	if (GetTexture())
		GetTexture()->get_spacings(spcx, spcy, spcz, lv);
}

fluo::Vector VolumeData::GetSpacings(int lv)
{
	double x, y, z;
	if (GetTexture())
	{
		GetTexture()->get_spacings(x, y, z, lv);
		return fluo::Vector(x, y, z);
	}
	return fluo::Vector(0);
}

void VolumeData::GetFileSpacings(double &spcx, double &spcy, double &spcz)
{
	spcx = m_spcx; spcy = m_spcy; spcz = m_spcz;
}

//brkxml
void VolumeData::SetBaseSpacings(double spcx, double spcy, double spcz)
{
	if (GetTexture())
	{
		GetTexture()->set_base_spacings(spcx, spcy, spcz);
		m_bounds.reset();
		GetTexture()->get_bounds(m_bounds);
	}
}

void VolumeData::GetBaseSpacings(double &spcx, double &spcy, double & spcz)
{
	if (GetTexture())
		GetTexture()->get_base_spacings(spcx, spcy, spcz);
}

fluo::Vector VolumeData::GetBaseSpacings()
{
	double x, y, z;
	if (GetTexture())
	{
		GetTexture()->get_base_spacings(x, y, z);
		return fluo::Vector(x, y, z);
	}
	return fluo::Vector(0);
}

void VolumeData::SetSpacingScales(double s_spcx, double s_spcy, double s_spcz)
{
	if (GetTexture())
	{
		GetTexture()->set_spacing_scales(s_spcx, s_spcy, s_spcz);
		m_bounds.reset();
		GetTexture()->get_bounds(m_bounds);
	}
}

void VolumeData::GetSpacingScales(double &s_spcx, double &s_spcy, double &s_spcz)
{
	if (GetTexture())
		GetTexture()->get_spacing_scales(s_spcx, s_spcy, s_spcz);
}

fluo::Vector VolumeData::GetSpacingScales()
{
	double x, y, z;
	if (GetTexture())
	{
		GetTexture()->get_spacing_scales(x, y, z);
		return fluo::Vector(x, y, z);
	}
	return fluo::Vector(0);
}

void VolumeData::SetLevel(int lv)
{
	if (GetTexture() && isBrxml())
	{
		GetTexture()->setLevel(lv);
		m_bounds.reset();
		GetTexture()->get_bounds(m_bounds);
	}
}

int VolumeData::GetLevel()
{
	if (GetTexture() && isBrxml())
		return GetTexture()->GetCurLevel();
	else
		return -1;
}

int VolumeData::GetLevelNum()
{
	if (GetTexture() && isBrxml())
		return GetTexture()->GetLevelNum();
	else
		return -1;
}

//bits
int VolumeData::GetBits()
{
	if (!m_tex)
		return 0;
	Nrrd* nrrd_data = m_tex->get_nrrd(0);
	if (!nrrd_data)
		return 0;
	if (nrrd_data->type == nrrdTypeUChar)
		return 8;
	else if (nrrd_data->type == nrrdTypeUShort)
		return 16;
	return 0;
}

//display controls
void VolumeData::SetDisp(bool disp)
{
	m_disp = disp;
	GetTexture()->set_sort_bricks();
}

bool VolumeData::GetDisp()
{
	return m_disp;
}

void VolumeData::ToggleDisp()
{
	m_disp = !m_disp;
	GetTexture()->set_sort_bricks();
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

//blend mode
void VolumeData::SetBlendMode(int mode)
{
	m_blend_mode = mode;
}

int VolumeData::GetBlendMode()
{
	return m_blend_mode;
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

//clip size
void VolumeData::GetClipValues(int &ox, int &oy, int &oz,
	int &nx, int &ny, int &nz)
{
	std::vector<fluo::Plane*> *planes = m_vr->get_planes();
	if (planes->size() != 6)
		return;

	int resx, resy, resz;
	GetResolution(resx, resy, resz);

	//calculating planes
	//get six planes
	fluo::Plane* px1 = (*planes)[0];
	fluo::Plane* px2 = (*planes)[1];
	fluo::Plane* py1 = (*planes)[2];
	fluo::Plane* py2 = (*planes)[3];
	fluo::Plane* pz1 = (*planes)[4];
	fluo::Plane* pz2 = (*planes)[5];

	ox = std::round(-resx*px1->d());
	oy = std::round(-resy*py1->d());
	oz = std::round(-resz*pz1->d());
	nx = std::round(resx*px2->d()) - ox;
	ny = std::round(resy*py2->d()) - oy;
	nz = std::round(resz*pz2->d()) - oz;
}

void VolumeData::SetClipValue(int i, int val)
{
	if (i < 0 || i > 5)
		return;
	std::vector<fluo::Plane*>* planes = 0;
	if (GetVR())
		planes = GetVR()->get_planes();
	if (!planes)
		return;
	if (planes->size() != 6)
		return;

	double clip = 0;
	fluo::Point p;
	fluo::Vector v;
	switch (i)
	{
	case 0://x1
		clip = (double)val / m_res_x;
		p = fluo::Point(clip, 0, 0);
		v = fluo::Vector(1, 0, 0);
		break;
	case 1://x2
		clip = (double)val / m_res_x;
		p = fluo::Point(clip, 0, 0);
		v = fluo::Vector(-1, 0, 0);
		break;
	case 2://y1
		clip = (double)val / m_res_y;
		p = fluo::Point(0, clip, 0);
		v = fluo::Vector(0, 1, 0);
		break;
	case 3://y2
		clip = (double)val / m_res_y;
		p = fluo::Point(0, clip, 0);
		v = fluo::Vector(0, -1, 0);
		break;
	case 4://z1
		clip = (double)val / m_res_z;
		p = fluo::Point(0, 0, clip);
		v = fluo::Vector(0, 0, 1);
		break;
	case 5://z2
		clip = (double)val / m_res_z;
		p = fluo::Point(0, 0, clip);
		v = fluo::Vector(0, 0, -1);
		break;
	}
	(*planes)[i]->ChangePlane(p, v);
}

void VolumeData::SetClipValues(int i, int val1, int val2)
{
	std::vector<fluo::Plane*>* planes = 0;
	if (GetVR())
		planes = GetVR()->get_planes();
	if (!planes)
		return;
	if (planes->size() != 6)
		return;

	double clip1 = 0, clip2 = 0;
	fluo::Point p1, p2;
	fluo::Vector v1, v2;
	switch (i)
	{
	case 1://x
	case 2:
	case 3:
		clip1 = (double)val1 / m_res_x;
		clip2 = (double)val2 / m_res_x;
		p1 = fluo::Point(clip1, 0, 0);
		v1 = fluo::Vector(1, 0, 0);
		p2 = fluo::Point(clip2, 0, 0);
		v2 = fluo::Vector(-1, 0, 0);
		(*planes)[0]->ChangePlane(p1, v1);
		(*planes)[1]->ChangePlane(p2, v2);
		break;
	case 4://y1
	case 8:
	case 12:
		clip1 = (double)val1 / m_res_y;
		clip2 = (double)val2 / m_res_y;
		p1 = fluo::Point(0, clip1, 0);
		v1 = fluo::Vector(0, 1, 0);
		p2 = fluo::Point(0, clip2, 0);
		v2 = fluo::Vector(0, -1, 0);
		(*planes)[2]->ChangePlane(p1, v1);
		(*planes)[3]->ChangePlane(p2, v2);
		break;
	case 16://z1
	case 32:
	case 48:
		clip1 = (double)val1 / m_res_z;
		clip2 = (double)val2 / m_res_z;
		p1 = fluo::Point(0, 0, clip1);
		v1 = fluo::Vector(0, 0, 1);
		p2 = fluo::Point(0, 0, clip2);
		v2 = fluo::Vector(0, 0, -1);
		(*planes)[4]->ChangePlane(p1, v1);
		(*planes)[5]->ChangePlane(p2, v2);
		break;
	}
}

void VolumeData::SetClipValues(const int val[6])
{
	std::vector<fluo::Plane*>* planes = 0;
	if (GetVR())
		planes = GetVR()->get_planes();
	if (!planes)
		return;
	if (planes->size() != 6)
		return;

	double clip;
	fluo::Point p;
	fluo::Vector v;
	for (int i : { 0, 1, 2, 3, 4, 5 })
	{
		switch (i)
		{
		case 0:
			clip = (double)val[i] / m_res_x;
			p = fluo::Point(clip, 0, 0);
			v = fluo::Vector(1, 0, 0);
			break;
		case 1:
			clip = (double)val[i] / m_res_x;
			p = fluo::Point(clip, 0, 0);
			v = fluo::Vector(-1, 0, 0);
			break;
		case 2:
			clip = (double)val[i] / m_res_y;
			p = fluo::Point(0, clip, 0);
			v = fluo::Vector(0, 1, 0);
			break;
		case 3:
			clip = (double)val[i] / m_res_y;
			p = fluo::Point(0, clip, 0);
			v = fluo::Vector(0, -1, 0);
			break;
		case 4:
			clip = (double)val[i] / m_res_z;
			p = fluo::Point(0, 0, clip);
			v = fluo::Vector(0, 0, 1);
			break;
		case 5:
			clip = (double)val[i] / m_res_z;
			p = fluo::Point(0, 0, clip);
			v = fluo::Vector(0, 0, -1);
			break;
		}
		(*planes)[i]->ChangePlane(p, v);
	}
}

void VolumeData::ResetClipValues()
{
	std::vector<fluo::Plane*>* planes = 0;
	if (GetVR())
		planes = GetVR()->get_planes();
	if (!planes)
		return;
	if (planes->size() != 6)
		return;

	fluo::Point p;
	fluo::Vector v;
	for (int i : { 0, 1, 2, 3, 4, 5 })
	{
		switch (i)
		{
		case 0:
			p = fluo::Point(0, 0, 0);
			v = fluo::Vector(1, 0, 0);
			break;
		case 1:
			p = fluo::Point(1, 0, 0);
			v = fluo::Vector(-1, 0, 0);
			break;
		case 2:
			p = fluo::Point(0, 0, 0);
			v = fluo::Vector(0, 1, 0);
			break;
		case 3:
			p = fluo::Point(0, 1, 0);
			v = fluo::Vector(0, -1, 0);
			break;
		case 4:
			p = fluo::Point(0, 0, 0);
			v = fluo::Vector(0, 0, 1);
			break;
		case 5:
			p = fluo::Point(0, 0, 1);
			v = fluo::Vector(0, 0, -1);
			break;
		}
		(*planes)[i]->ChangePlane(p, v);
	}
}

void VolumeData::ResetClipValuesX()
{
	std::vector<fluo::Plane*>* planes = 0;
	if (GetVR())
		planes = GetVR()->get_planes();
	if (!planes)
		return;
	if (planes->size() != 6)
		return;

	fluo::Point p;
	fluo::Vector v;
	p = fluo::Point(0, 0, 0);
	v = fluo::Vector(1, 0, 0);
	(*planes)[0]->ChangePlane(p, v);
	p = fluo::Point(1, 0, 0);
	v = fluo::Vector(-1, 0, 0);
	(*planes)[1]->ChangePlane(p, v);
}

void VolumeData::ResetClipValuesY()
{
	std::vector<fluo::Plane*>* planes = 0;
	if (GetVR())
		planes = GetVR()->get_planes();
	if (!planes)
		return;
	if (planes->size() != 6)
		return;

	fluo::Point p;
	fluo::Vector v;
	p = fluo::Point(0, 0, 0);
	v = fluo::Vector(0, 1, 0);
	(*planes)[2]->ChangePlane(p, v);
	p = fluo::Point(0, 1, 0);
	v = fluo::Vector(0, -1, 0);
	(*planes)[3]->ChangePlane(p, v);
}

void VolumeData::ResetClipValuesZ()
{
	std::vector<fluo::Plane*>* planes = 0;
	if (GetVR())
		planes = GetVR()->get_planes();
	if (!planes)
		return;
	if (planes->size() != 6)
		return;

	fluo::Point p;
	fluo::Vector v;
	p = fluo::Point(0, 0, 0);
	v = fluo::Vector(0, 0, 1);
	(*planes)[4]->ChangePlane(p, v);
	p = fluo::Point(0, 0, 1);
	v = fluo::Vector(0, 0, -1);
	(*planes)[5]->ChangePlane(p, v);
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
	return m_tex->get_brick_num();
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
	if (m_tex->nlabel() == -1)
		return;

	Nrrd* data = m_tex->get_nrrd(m_tex->nlabel());
	if (!data || ! data->data)
		return;
	int nx, ny, nz;
	GetResolution(nx, ny, nz);
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
		int nx, ny, nz;
		GetResolution(nx, ny, nz);
		unsigned long long size = (unsigned long long)nx * ny * nz;
		memcpy(data->data, m_label_save, size * sizeof(unsigned int));
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
			double h, s, v;
			GetHSV(h, s, v);
			fluo::HSVColor hsv(h, s, dval);
			fluo::Color color(hsv);
			ResetMaskColorSet();
			SetColor(color);
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
			//high shading
			dval2 = std::max(0.0f, m_ep->getParam("high_shading"));
			double amb, diff, spec, shine;
			GetMaterial(amb, diff, spec, shine);
			SetMaterial(dval, diff, spec, dval2);
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
		SetColormapMode(dval>0.5);
		if (m_colormap_mode > 0)
		{
			//colormap inv
			dval = m_ep->getParam("colormap_inv");
			SetColormapInv(dval > 0.5 ? -1.0 : 1.0);
			//colormap type
			dval = m_ep->getParam("colormap_type");
			SetColormap(std::round(dval));
			//colormap projection
			dval = m_ep->getParam("colormap_proj");
			SetColormapProj(std::round(dval));
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
		SetMode(std::round(dval));
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

