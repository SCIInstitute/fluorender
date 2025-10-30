//
//  For more information, please see: http://software.sci.utah.edu
//
//  The MIT License
//
//  Copyright (c) 2025 Scientific Computing and Imaging Institute,
//  University of Utah.
//
//  
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//

#include <GL/glew.h>
#include <VolumeRenderer.h>
#include <Global.h>
#include <Names.h>
#include <MainSettings.h>
#include <Plane.h>
#include <Texture.h>
#include <Framebuffer.h>
#include <VolShader.h>
#include <ImgShader.h>
#include <SegShader.h>
#include <ShaderProgram.h>
#include <TextureBrick.h>
#include <VolCalShader.h>
#include <MovieMaker.h>
#include <VolCache4D.h>
#include <compatibility.h>
#include <fstream>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <Debug.h>

using namespace flvr;

VolumeRenderer::VolumeRenderer(
	const std::vector<fluo::Plane*>& planes)
	:TextureRenderer(),
	//scalar scaling factor
	scalar_scale_(1.0),
	//gm range
	gm_scale_(1.0),
	//transfer function
	gamma3d_(1.0),
	gm_low_(0.0),
	gm_high_(0.5),
	gm_max_(0.5),
	lo_offset_(0.0),
	hi_offset_(1.0),
	lo_thresh_(0.0),
	hi_thresh_(1.0),
	color_(fluo::Color(1.0, 1.0, 1.0)),
	mask_color_(fluo::Color(0.0, 1.0, 0.0)),
	mask_color_set_(false),
	mask_thresh_(0.0),
	alpha_(1.0),
	//shading
	shading_(false),
	ambient_(1.0),
	diffuse_(1.0),
	specular_(1.0),
	shine_(10.0),
	//colormap mode
	colormap_inv_(1.0),
	colormap_mode_(0),
	colormap_low_value_(0.0),
	colormap_hi_value_(1.0),
	colormap_(0),
	colormap_proj_(0),
	//shuffling
	shuffle_(0),
	//solid
	solid_(false),
	//interpolate
	interpolate_(true),
	//clipping planes
	planes_(planes),
	//depth peel
	depth_peel_(0),
	//segmentation
	ml_mode_(0),
	mask_(false),
	label_(false),
	//scale factor
	noise_red_(false),
	sfactor_(1.0),
	inv_(false),
	compression_(false),
	alpha_power_(1.0),
	zoom_(1.0),
	zoom_data_(1.0)
{
	//mode
	mode_ = RenderMode::RENDER_MODE_OVER;
	//done loop
	for (int i = 0; i < TEXTURE_RENDER_MODES; i++)
		done_loop_[i] = false;
}

VolumeRenderer::VolumeRenderer(const VolumeRenderer& copy)
	:TextureRenderer(copy),
	//scalar scale
	scalar_scale_(copy.scalar_scale_),
	//gm range
	gm_scale_(copy.gm_scale_),
	//transfer function
	gamma3d_(copy.gamma3d_),
	gm_low_(copy.gm_low_),
	gm_high_(copy.gm_high_),
	gm_max_(copy.gm_max_),
	lo_offset_(copy.lo_offset_),
	hi_offset_(copy.hi_offset_),
	lo_thresh_(copy.lo_thresh_),
	hi_thresh_(copy.hi_thresh_),
	color_(copy.color_),
	mask_color_(copy.mask_color_),
	mask_color_set_(copy.mask_color_set_),
	mask_thresh_(0.0),
	alpha_(copy.alpha_),
	//shading
	shading_(copy.shading_),
	ambient_(copy.ambient_),
	diffuse_(copy.diffuse_),
	specular_(copy.specular_),
	shine_(copy.shine_),
	//colormap mode
	colormap_inv_(copy.colormap_inv_),
	colormap_mode_(copy.colormap_mode_),
	colormap_low_value_(copy.colormap_low_value_),
	colormap_hi_value_(copy.colormap_hi_value_),
	colormap_(copy.colormap_),
	colormap_proj_(copy.colormap_proj_),
	//shuffling
	shuffle_(0),
	//solid
	solid_(copy.solid_),
	//interpolate
	interpolate_(copy.interpolate_),
	//depth peel
	depth_peel_(copy.depth_peel_),
	//segmentation
	ml_mode_(copy.ml_mode_),
	mask_(copy.mask_),
	label_(copy.label_),
	//scale factor
	noise_red_(copy.noise_red_),
	inv_(copy.inv_),
	compression_(copy.compression_),
	alpha_power_(copy.alpha_power_),
	sw_(0)
{
	//mode
	mode_ = copy.mode_;
	//clipping planes
	for (int i = 0; i < (int)copy.planes_.size(); i++)
	{
		fluo::Plane* plane = new fluo::Plane(*copy.planes_[i]);
		planes_.push_back(plane);
	}
	//done loop
	for (int i = 0; i < TEXTURE_RENDER_MODES; i++)
		done_loop_[i] = false;
}

VolumeRenderer::~VolumeRenderer()
{
	//release clipping planes
	for (int i = 0; i < (int)planes_.size(); i++)
	{
		if (planes_[i])
			delete planes_[i];
	}
	planes_.clear();
}

void VolumeRenderer::set_color(const fluo::Color& color)
{
	color_ = color;

	if (!mask_color_set_)
	{
		//generate opposite color for mask
		fluo::HSVColor hsv_color(color_);
		double h, s, v;
		if (hsv_color.sat() < 0.2)
			mask_color_ = fluo::Color(0.0, 1.0, 0.0);	//if low saturation, set to green
		else
		{
			double h0 = hsv_color.hue();
			h = h0 < 30.0 ? h0 - 180.0 : h0 < 90.0 ? h0 + 120.0 : h0 < 210.0 ? h0 - 120.0 : h0 - 180.0;
			s = 1.0;
			v = 1.0;
			mask_color_ = fluo::Color(fluo::HSVColor(h < 0.0 ? h + 360.0 : h, s, v));
		}
	}
}

void VolumeRenderer::set_mask_color(const fluo::Color& color, bool set)
{
	mask_color_ = color;
	mask_color_set_ = set;
}

//clipping planes
void VolumeRenderer::set_planes(std::vector<fluo::Plane*>* p)
{
	int i;
	if (!planes_.empty())
	{
		for (i = 0; i < (int)planes_.size(); i++)
		{
			if (planes_[i])
				delete planes_[i];
		}
		planes_.clear();
	}

	for (i = 0; i < (int)p->size(); i++)
	{
		fluo::Plane* plane = new fluo::Plane(*(*p)[i]);
		planes_.push_back(plane);
	}
}

std::vector<fluo::Plane*>* VolumeRenderer::get_planes()
{
	return &planes_;
}

std::string VolumeRenderer::get_buffer_name()
{
	bool adaptive = get_adaptive();
	if (imode_ && adaptive)
	{
		return gstRBBlendInteractive;
	}
	else if (noise_red_)
	{
		return gstRBBlendDenoise;
	}
	else
	{
		return gstRBBlendQuality;
	}
}

Size2D VolumeRenderer::resize(const std::string& buf_name)
{
	Size2D out_size(0, 0);
	int w = viewport_[2];
	int h = viewport_[3];
	double sf = 1.0;
	if (buf_name == gstRBBlendInteractive)
	{
		sf = fluo::Clamp(double(1.0 / zoom_data_), 0.1, 1.0);
	}
	else if (buf_name == gstRBBlendDenoise)
	{
		sf = fluo::Clamp(double(1.0 / zoom_data_), 0.5, 3.5);
		//sf = std::min(double(1.0 / zoom_data_), 3.5);
	}
	else
	{
		sf = fluo::Clamp(double(1.0 / zoom_data_), 0.5, 2.0);
	}
	//if (std::fabs(sf - sfactor_) > 0.05)
	sfactor_ = sf;

	out_size = Size2D(
		int(std::round(w * sfactor_)),
		int(std::round(h * sfactor_)));
	return out_size;
}

//darw
void VolumeRenderer::eval_ml_mode()
{
	auto tex = tex_.lock();
	if (!tex)
		return;

	//reassess the mask/label mode
	//0-normal, 1-render with mask, 2-render with mask excluded
	//3-random color with label, 4-random color with label+mask
	switch (ml_mode_)
	{
	case 0:
		mask_ = false;
		label_ = false;
		break;
	case 1:
		if (tex->nmask() == -1)
		{
			mask_ = false;
			ml_mode_ = 0;
		}
		else
			mask_ = true;
		label_ = false;
		break;
	case 2:
		if (tex->nmask() == -1)
		{
			mask_ = false;
			ml_mode_ = 0;
		}
		else
			mask_ = true;
		label_ = false;
		break;
	case 3:
		if (tex->nlabel() == -1)
		{
			label_ = false;
			ml_mode_ = 0;
		}
		else
			label_ = true;
		mask_ = false;
		break;
	case 4:
		if (tex->nlabel() == -1)
		{
			if (tex->nmask() > -1)
			{
				mask_ = true;
				label_ = false;
				ml_mode_ = 1;
			}
			else
			{
				mask_ = label_ = false;
				ml_mode_ = 0;
			}
		}
		else
			mask_ = label_ = true;
		break;
	}
}

void VolumeRenderer::draw(bool draw_wireframe_p,
	bool interactive_mode_p, bool orthographic_p, int mode)
{
	draw_volume(interactive_mode_p, orthographic_p, mode);
	if (draw_wireframe_p)
		draw_wireframe(orthographic_p);
}

void VolumeRenderer::draw_volume(
	bool interactive_mode_p,
	bool orthographic_p,
	int mode)
{
	auto tex = tex_.lock();
	if (!tex)
		return;

	fluo::Ray view_ray = compute_view();
	fluo::Ray snapview = compute_snapview(0.4);
	float zoom_data_clamp = zoom_data_;
	if (orthographic_p)
		zoom_data_clamp = std::clamp(static_cast<float>(zoom_data_clamp), 0.2f, 35.0f);
	else
		zoom_data_clamp = std::clamp(static_cast<float>(zoom_data_clamp), 1.0f, 10.0f);

	std::vector<TextureBrick*>* bricks = 0;
	tex->set_matrices(m_mv_tex_scl_mat, m_proj_mat);
	if (glbin_settings.m_mem_swap && interactive_)
		bricks = tex->get_closest_bricks(
			quota_center_, quota_bricks_chan_, true,
			view_ray, orthographic_p);
	else
		bricks = tex->get_sorted_bricks(view_ray, orthographic_p);
	if (!bricks || bricks->size() == 0)
		return;

	set_interactive_mode(interactive_mode_p);

	double rate = get_sample_rate();
	fluo::Vector diag = tex->bbox()->diagonal();
	fluo::Vector cell_diag(
		diag.x() / tex->nx(),
		diag.y() / tex->ny(),
		diag.z() / tex->nz());
	double dt;
	size_t est_slices;
	if (rate > 0.0)
	{
		dt = cell_diag.length() / compute_rate_scale(snapview.direction()) / rate;
		est_slices = static_cast<int>(std::round(diag.length() / dt));
	}
	else
	{
		dt = 0.0;
		est_slices = 0;
	}

	std::vector<float> vertex;
	std::vector<uint32_t> index;
	std::vector<uint32_t> size;
	vertex.reserve(est_slices * 12);
	index.reserve(est_slices * 6);
	size.reserve(est_slices * 6);

	//--------------------------------------------------------------------------

	int cm_mode = mode == 4 ? 0 : colormap_mode_;
	bool use_fog = m_use_fog && cm_mode != 2;

	int w = viewport_[2];
	int h = viewport_[3];
	std::string buf_name = get_buffer_name();
	Size2D new_size = resize(buf_name);
	int w2 = new_size.w();
	int h2 = new_size.h();

	auto cur_buffer = glbin_framebuffer_manager.current();
	auto blend_buffer = glbin_framebuffer_manager.framebuffer(
		flvr::FBRole::RenderFloat, w2, h2, buf_name);
	assert(blend_buffer);
	//set clear color and viewport size
	blend_buffer->set_clear_color({ clear_color_[0], clear_color_[1], clear_color_[2], clear_color_[3] });
	blend_buffer->set_viewport({ viewport_[0], viewport_[1], w2, h2 });
	// set up blending
	blend_buffer->set_blend_enabled(true);
	//blend: normal: (add, add)(b2f: (1, 1-a), f2b: (1-a, a)), mip: (max, max)(1, 1)
	switch (mode_)
	{
	case RenderMode::RENDER_MODE_OVER:
		blend_buffer->set_blend_equation(BlendEquation::Add, BlendEquation::Add);
		if (glbin_settings.m_update_order == 0)
			blend_buffer->set_blend_func(BlendFactor::One, BlendFactor::OneMinusSrcAlpha);
		else if (glbin_settings.m_update_order == 1)
			blend_buffer->set_blend_func(BlendFactor::OneMinusDstAlpha, BlendFactor::One);
		break;
	case RenderMode::RENDER_MODE_MIP:
		blend_buffer->set_blend_equation(BlendEquation::Max, BlendEquation::Max);
		blend_buffer->set_blend_func(BlendFactor::One, BlendFactor::One);
		break;
	default:
		break;
	}
	glbin_framebuffer_manager.bind(blend_buffer);
	blend_buffer->clear(true, false);

	eval_ml_mode();

	//--------------------------------------------------------------------------
	// Set up shaders
	std::shared_ptr<ShaderProgram> shader;
	//create/bind
	bool grad = gm_low_ > 0.0 ||
		gm_high_ < gm_max_ ||
		(cm_mode &&
			colormap_proj_ > 3);
	shader = glbin_shader_manager.shader(gstVolShader,
		ShaderParams::Volume(
			false, tex->nc(),
			shading_, use_fog,
			depth_peel_, true,
			grad, ml_mode_, mode_ == RenderMode::RENDER_MODE_MIP,
			cm_mode, colormap_, colormap_proj_,
			solid_, 1));
	assert(shader);
	shader->bind();

	//set uniforms
	//set up shading
	//set the light
	fluo::Vector light = view_ray.direction();
	light.safe_normalize();
	shader->setLocalParam(0, light.x(), light.y(), light.z(), alpha_);
	if (shading_)
		shader->setLocalParam(1, 2.0 - ambient_, diffuse_, specular_, shine_);
	else
		shader->setLocalParam(1, 2.0 - ambient_, 0.0, specular_, shine_);

	//spacings
	double spcx, spcy, spcz;
	tex->get_spacings(spcx, spcy, spcz);
	shader->setLocalParam(5,
		spcx == 0.0 ? 1.0 : spcx,
		spcy == 0.0 ? 1.0 : spcy,
		spcz == 0.0 ? 1.0 : spcz, shuffle_);

	//transfer function
	shader->setLocalParam(2, inv_ ? -scalar_scale_ : scalar_scale_, gm_scale_, lo_thresh_, hi_thresh_);
	shader->setLocalParam(3, 1.0 / gamma3d_, lo_offset_, hi_offset_, sw_);
	if (mode_ == RenderMode::RENDER_MODE_MIP &&
		colormap_proj_)
		shader->setLocalParam(6, colormap_low_value_, colormap_hi_value_,
			colormap_hi_value_ - colormap_low_value_, colormap_inv_);
	else
	{
		switch (cm_mode)
		{
		case 0://normal
			if (mask_ && !label_)
				shader->setLocalParam(6, mask_color_.r(), mask_color_.g(), mask_color_.b(), 0.0);
			else
				shader->setLocalParam(6, color_.r(), color_.g(), color_.b(), 0.0);
			break;
		case 1://colormap
			shader->setLocalParam(6, colormap_low_value_, colormap_hi_value_,
				colormap_hi_value_ - colormap_low_value_, colormap_inv_);
			break;
		case 2://depth map
			shader->setLocalParam(6, color_.r(), color_.g(), color_.b(), 0.0);
			break;
		}
	}
	//color
	shader->setLocalParam(9, color_.r(), color_.g(), color_.b(), alpha_power_);
	shader->setLocalParam(16, mask_color_.r(), mask_color_.g(), mask_color_.b(), mask_thresh_);
	//gm
	shader->setLocalParam(17, gm_low_, gm_high_, gm_max_, 0.0);

	if (colormap_proj_ == 4)
	{
		shader->setLocalParamUInt(0, static_cast<unsigned int>(glbin_moviemaker.GetSeqCurNum()));
		shader->setLocalParamUInt(1, static_cast<unsigned int>(glbin_moviemaker.GetSeqAllNum()));
	}

	//setup depth peeling
	if (depth_peel_ || cm_mode == 2)
		shader->setLocalParam(7, 1.0 / double(w2), 1.0 / double(h2), 0.0, 0.0);

	//fog
	if (m_use_fog)
		shader->setLocalParam(8, m_fog_intensity, m_fog_start, m_fog_end, 0.0);

	//set clipping planes
	double abcd[4];
	planes_[0]->get(abcd);
	shader->setLocalParam(10, abcd[0], abcd[1], abcd[2], abcd[3]);
	planes_[1]->get(abcd);
	shader->setLocalParam(11, abcd[0], abcd[1], abcd[2], abcd[3]);
	planes_[2]->get(abcd);
	shader->setLocalParam(12, abcd[0], abcd[1], abcd[2], abcd[3]);
	planes_[3]->get(abcd);
	shader->setLocalParam(13, abcd[0], abcd[1], abcd[2], abcd[3]);
	planes_[4]->get(abcd);
	shader->setLocalParam(14, abcd[0], abcd[1], abcd[2], abcd[3]);
	planes_[5]->get(abcd);
	shader->setLocalParam(15, abcd[0], abcd[1], abcd[2], abcd[3]);

	////////////////////////////////////////////////////////
	// render bricks
	// Set up transform
	shader->setLocalParamMatrix(0, glm::value_ptr(m_proj_mat));
	shader->setLocalParamMatrix(1, glm::value_ptr(m_mv_tex_scl_mat));

	if (cur_chan_brick_num_ == 0) rearrangeLoadedBrkVec();

	num_slices_ = 0;
	bool multibricks = bricks->size() > 1;
	bool drawn_once = false;
	//std::wstring wstr;
	for (unsigned int i = 0; i < bricks->size(); i++)
	{
		//comment off when debug_ds
		if (mode != 2 && mode != 3 && glbin_settings.m_mem_swap && drawn_once)
		{
			unsigned long long rn_time = GET_TICK_COUNT();
			if (rn_time - st_time_ > get_up_time())
				break;
		}

		TextureBrick* b = (*bricks)[i];
		if (tex->isBrxml() && !b->isLoaded())
		{
			if (!test_against_view(b->bbox(), !orthographic_p) ||
				b->get_priority() > 0)
				cur_chan_brick_num_++;
			continue;
		}

		if (glbin_settings.m_mem_swap && start_update_loop_ && !done_update_loop_)
		{
			if (b->drawn(mode))
				continue;
		}

		if (((!glbin_settings.m_mem_swap || !interactive_) &&
			!test_against_view(b->bbox(), !orthographic_p)) || // Clip against view
			b->get_priority() > 0) //nothing to draw
		{
			if (glbin_settings.m_mem_swap && start_update_loop_ && !done_update_loop_)
			{
				if (!b->drawn(mode))
				{
					b->set_drawn(mode, true);
					cur_chan_brick_num_++;
				}
			}
			continue;
		}

		if ((cm_mode == 1 ||
			mode_ == RenderMode::RENDER_MODE_MIP) &&
			colormap_proj_)
		{
			fluo::BBox bbox = b->dbox();
			float matrix[16];
			matrix[0] = float(bbox.Max().x() - bbox.Min().x());
			matrix[1] = 0.0f;
			matrix[2] = 0.0f;
			matrix[3] = 0.0f;
			matrix[4] = 0.0f;
			matrix[5] = float(bbox.Max().y() - bbox.Min().y());
			matrix[6] = 0.0f;
			matrix[7] = 0.0f;
			matrix[8] = 0.0f;
			matrix[9] = 0.0f;
			matrix[10] = float(bbox.Max().z() - bbox.Min().z());
			matrix[11] = 0.0f;
			matrix[12] = float(bbox.Min().x());
			matrix[13] = float(bbox.Min().y());
			matrix[14] = float(bbox.Min().z());
			matrix[15] = 1.0f;
			shader->setLocalParamMatrix(5, matrix);
		}

		vertex.clear();
		index.clear();
		size.clear();
		b->compute_polygons(snapview, dt, vertex, index, size, multibricks);

		num_slices_ += size.size();

		if (!vertex.empty())
		{
			GLint filter;
			if (interpolate_)
				filter = GL_LINEAR;
			else
				filter = GL_NEAREST;

			if (!load_brick(b, filter, compression_, 0, mode, 0))
				continue;
			if (colormap_proj_ >= 7)
				load_brick(b, filter, compression_, 10, mode, -1);
			if (mask_)
				load_brick_mask(b, filter);
			if (label_)
				load_brick_label(b);
			shader->setLocalParam(4, 1.0 / b->nx(), 1.0 / b->ny(), 1.0 / b->nz(),
				mode_ == RenderMode::RENDER_MODE_OVER ? 1.0 / rate : 1.0);

			//for brick transformation
			float matrix[16];
			fluo::BBox bbox = b->dbox();
			matrix[0] = float(bbox.Max().x() - bbox.Min().x());
			matrix[1] = 0.0f;
			matrix[2] = 0.0f;
			matrix[3] = 0.0f;
			matrix[4] = 0.0f;
			matrix[5] = float(bbox.Max().y() - bbox.Min().y());
			matrix[6] = 0.0f;
			matrix[7] = 0.0f;
			matrix[8] = 0.0f;
			matrix[9] = 0.0f;
			matrix[10] = float(bbox.Max().z() - bbox.Min().z());
			matrix[11] = 0.0f;
			matrix[12] = float(bbox.Min().x());
			matrix[13] = float(bbox.Min().y());
			matrix[14] = float(bbox.Min().z());
			matrix[15] = 1.0f;
			shader->setLocalParamMatrix(2, matrix);

			draw_polygons(vertex, index);

			if (glbin_settings.m_mem_swap && !b->drawn(mode))
			{
				b->set_drawn(mode, true);
				cur_brick_num_++;
				cur_chan_brick_num_++;
			}
		}

		if (glbin_settings.m_mem_swap)
			finished_bricks_++;
		drawn_once = true;

		//wstr += std::to_wstring(i) + L" ";
	}

	//wstr += L"\n";
	//DBGPRINT(wstr.c_str());
	//DBGPRINT(L"done_current_chan_:%d\tclear_chan_buffer_:%d\tsave_final_buffer_:%d\tdone_loop_[%d]:%d\tcur_chan_brick_num_:%d\tcur_brick_num_:%d\tfinished_bricks_:%d\tbrick_size:%d\ttotal_brick_num_:%d\n",
	//	done_current_chan_, clear_chan_buffer_, save_final_buffer_, mode, done_loop_[mode], cur_chan_brick_num_, cur_brick_num_, finished_bricks_, (*bricks).size(), total_brick_num_);

	if (glbin_settings.m_mem_swap &&
		cur_brick_num_ >= total_brick_num_)
	{
		done_update_loop_ = true;
		active_view_ = -1;
	}
	if (glbin_settings.m_mem_swap &&
		(size_t)cur_chan_brick_num_ >= (*bricks).size())
	{
		done_current_chan_ = true;
		clear_chan_buffer_ = true;
		save_final_buffer_ = true;
		cur_chan_brick_num_ = 0;
		done_loop_[mode] = true;
	}

	//release depth texture for rendering shadows
	if (cm_mode == 2)
		release_texture(4, GL_TEXTURE_2D);

	// Release shader.
	shader->unbind();

	//Release 3d texture
	release_texture(0, GL_TEXTURE_3D);
	if (colormap_proj_ >= 7)
		release_texture(10, GL_TEXTURE_3D);
	if (mask_)
		release_texture((*bricks)[0]->nmask(), GL_TEXTURE_3D);
	if (label_)
		release_texture((*bricks)[0]->nlabel(), GL_TEXTURE_3D);

	std::shared_ptr<ShaderProgram> img_shader;

	std::shared_ptr<Framebuffer> filter_buffer = 0;
	if (noise_red_ && cm_mode != 2)
	{
		//FILTERING/////////////////////////////////////////////////////////////////
		filter_buffer = glbin_framebuffer_manager.framebuffer(
			flvr::FBRole::RenderFloat, w, h, gstRBFilter);
		assert(filter_buffer);
		filter_buffer->set_viewport({ viewport_[0], viewport_[1], viewport_[2], viewport_[3] });
		glbin_framebuffer_manager.bind(filter_buffer);
		filter_buffer->clear(true, false);

		blend_buffer->bind_texture(AttachmentPoint::Color(0), 0);

		img_shader = glbin_shader_manager.shader(gstImgShader,
			ShaderParams::Img(IMG_SHDR_FILTER_LANCZOS_BICUBIC, 0));
		assert(img_shader);
		img_shader->bind();

		img_shader->setLocalParam(0, zoom_data_clamp / w, zoom_data_clamp / h, zoom_data_clamp, 0.0);

		draw_view_quad();

		img_shader->unbind();
		blend_buffer->unbind_texture(AttachmentPoint::Color(0));
	}

	//go back to normal
	assert(cur_buffer);
	//set viewport size
	cur_buffer->set_viewport({ viewport_[0], viewport_[1], viewport_[2], viewport_[3] });
	glbin_framebuffer_manager.bind(cur_buffer);//blend buffer

	if (noise_red_ && cm_mode != 2)
		filter_buffer->bind_texture(AttachmentPoint::Color(0), 0);
	else
		blend_buffer->bind_texture(AttachmentPoint::Color(0), 0);

	img_shader = glbin_shader_manager.shader(gstImgShader,
		ShaderParams::Img(IMG_SHADER_TEXTURE_LOOKUP, 0));
	assert(img_shader);
	img_shader->bind();

	draw_view_quad();

	img_shader->unbind();

	if (noise_red_ && cm_mode != 2)
		filter_buffer->unbind_texture(AttachmentPoint::Color(0));
	else
		blend_buffer->unbind_texture(AttachmentPoint::Color(0));
}

void VolumeRenderer::draw_wireframe(bool orthographic_p)
{
	auto tex = tex_.lock();
	if (!tex)
		return;

	fluo::Ray view_ray = compute_view();
	fluo::Ray snapview = compute_snapview(0.4);

	std::vector<TextureBrick*>* bricks = tex->get_sorted_bricks(view_ray, orthographic_p);

	bool adaptive = get_adaptive();
	double rate = get_sample_rate();
	fluo::Vector diag = tex->bbox()->diagonal();
	fluo::Vector cell_diag(diag.x() / tex->nx(),
		diag.y() / tex->ny(),
		diag.z() / tex->nz());
	double dt;
	size_t est_slices;
	if (rate > 0.0)
	{
		dt = cell_diag.length() / compute_rate_scale(snapview.direction()) / rate;
		est_slices = static_cast<int>(std::round(diag.length() / dt));
	}
	else
	{
		dt = 0.0;
		est_slices = 0;
	}

	std::vector<float> vertex;
	std::vector<uint32_t> index;
	std::vector<uint32_t> size;
	vertex.reserve(est_slices * 12);
	index.reserve(est_slices * 6);
	size.reserve(est_slices * 6);

	// Set up shaders
	auto shader = glbin_shader_manager.shader(gstVolShader,
		ShaderParams::Volume(
			true, 0,
			false, false,
			0, false,
			false, 0, false,
			0, 0, 0,
			false, 1));
	if (shader)
		shader->bind();

	////////////////////////////////////////////////////////
	// render bricks
	shader->setLocalParamMatrix(0, glm::value_ptr(m_proj_mat));
	shader->setLocalParamMatrix(1, glm::value_ptr(m_mv_tex_scl_mat));
	shader->setLocalParam(0, color_.r(), color_.g(), color_.b(), 1.0);

	if (bricks)
	{
		bool multibricks = bricks->size() > 1;
		for (unsigned int i = 0; i < bricks->size(); i++)
		{
			TextureBrick* b = (*bricks)[i];
			if (!test_against_view(b->bbox(), !orthographic_p)) continue;

			vertex.clear();
			index.clear();
			size.clear();

			b->compute_polygons(snapview, dt, vertex, index, size, multibricks);
			draw_polygons_wireframe(vertex, index, size);
		}
	}

	// Release shader.
	if (shader)
		shader->unbind();
}

//type: 0-initial; 1-diffusion-based growing; 2-masked filtering
//paint_mode: 1-select; 2-append; 3-erase; 4-diffuse; 5-flood; 6-clear; 7-all;
//			  11-posterize
//hr_mode (hidden removal): 0-none; 1-ortho; 2-persp
void VolumeRenderer::draw_mask(int type, int paint_mode, int hr_mode,
	double ini_thresh, double gm_falloff, double scl_falloff,
	double scl_translate, double w2d, double bins, int order,
	bool orthographic_p, bool estimate)
{
	auto tex = tex_.lock();
	if (!tex)
		return;

	if (estimate && type == 0)
		est_thresh_ = 0.0;
	bool use_2d = tex_2d_weight1_ &&
		tex_2d_weight2_;

	std::vector<TextureBrick*>* bricks = tex->get_bricks();
	if (!bricks || bricks->size() == 0)
		return;

	//mask frame buffer object
	auto fbo_mask =
		glbin_framebuffer_manager.framebuffer(flvr::FBRole::Volume, 0, 0, "");
	assert(fbo_mask);
	auto guard = glbin_framebuffer_manager.bind_scoped(fbo_mask);

	double rate = get_sample_rate();

	//--------------------------------------------------------------------------
	// Set up shaders
	//seg shader
	std::shared_ptr<ShaderProgram> seg_shader;
	double mvec_len = mvec_.length();

	switch (type)
	{
	case 0://initialize
		seg_shader = glbin_shader_manager.shader(gstSegShader,
			ShaderParams::Seg(
				SEG_SHDR_INITIALIZE, paint_mode, hr_mode,
				use_2d, true, depth_peel_,
				true, false));
		break;
	case 1://diffusion-based growing
		seg_shader = glbin_shader_manager.shader(gstSegShader,
			ShaderParams::Seg(
				SEG_SHDR_DB_GROW, paint_mode, hr_mode,
				use_2d, true, depth_peel_,
				true, mvec_len > 0.5));
		break;
	}

	assert(seg_shader);
	seg_shader->bind();

	//set uniforms
	//set up shading
	fluo::Vector light = compute_view().direction();
	light.safe_normalize();
	seg_shader->setLocalParam(0, light.x(), light.y(), light.z(), alpha_);
	if (shading_)
		seg_shader->setLocalParam(1, 2.0 - ambient_, diffuse_, specular_, shine_);
	else
		seg_shader->setLocalParam(1, 2.0 - ambient_, 0.0, specular_, shine_);

	//spacings
	double spcx, spcy, spcz;
	tex->get_spacings(spcx, spcy, spcz);
	seg_shader->setLocalParam(5, spcx, spcy, spcz, shuffle_);

	//transfer function
	seg_shader->setLocalParam(2, inv_ ? -scalar_scale_ : scalar_scale_, gm_scale_, lo_thresh_, hi_thresh_);
	seg_shader->setLocalParam(3, 1.0 / gamma3d_, lo_offset_, hi_offset_, sw_);
	seg_shader->setLocalParam(6, mp_[0], viewport_[3] - mp_[1], viewport_[2], viewport_[3]);
	seg_shader->setLocalParam(9, mvec_.x(), mvec_.y(), mvec_.z(), 0);

	//setup depth peeling
	//if (depth_peel_)
	//	seg_shader->setLocalParam(7, 1.0/double(w2), 1.0/double(h2), 0.0, 0.0);

	//thresh1
	seg_shader->setLocalParam(7, ini_thresh, gm_falloff, scl_falloff, scl_translate);
	//w2d
	seg_shader->setLocalParam(8, w2d, bins, zoom_, zoom_data_);
	//gm
	seg_shader->setLocalParam(17, gm_low_, gm_high_, gm_max_, 0.0);

	//set clipping planes
	double abcd[4];
	planes_[0]->get(abcd);
	seg_shader->setLocalParam(10, abcd[0], abcd[1], abcd[2], abcd[3]);
	planes_[1]->get(abcd);
	seg_shader->setLocalParam(11, abcd[0], abcd[1], abcd[2], abcd[3]);
	planes_[2]->get(abcd);
	seg_shader->setLocalParam(12, abcd[0], abcd[1], abcd[2], abcd[3]);
	planes_[3]->get(abcd);
	seg_shader->setLocalParam(13, abcd[0], abcd[1], abcd[2], abcd[3]);
	planes_[4]->get(abcd);
	seg_shader->setLocalParam(14, abcd[0], abcd[1], abcd[2], abcd[3]);
	planes_[5]->get(abcd);
	seg_shader->setLocalParam(15, abcd[0], abcd[1], abcd[2], abcd[3]);

	////////////////////////////////////////////////////////
	// render bricks
	seg_shader->setLocalParamMatrix(0, glm::value_ptr(m_mv_tex_scl_mat));
	seg_shader->setLocalParamMatrix(1, glm::value_ptr(m_proj_mat));
	if (hr_mode > 0)
	{
		glm::mat4 mv_inv = glm::inverse(m_mv_tex_scl_mat);
		seg_shader->setLocalParamMatrix(3, glm::value_ptr(mv_inv));
	}

	//bind 2d mask texture
	bind_2d_mask();
	//bind 2d weight map
	if (use_2d) bind_2d_weight();

	int i;
	float matrix[16];
	unsigned int num = static_cast<unsigned int>(bricks->size());
	for (i = ((order == 2) ? (num - 1) : 0);
		(order == 2) ? (i >= 0) : (static_cast<long long>(i) < static_cast<long long>(num));
		i += ((order == 2) ? -1 : 1))
	{
		TextureBrick* b = (*bricks)[i];
		//if (!test_against_view(b->bbox(), !orthographic_p))
		if (paint_mode == 5 ||
			paint_mode == 6 ||
			paint_mode == 7)
		{
		}
		else if (!b->is_mask_act())
		{
			continue;
		}

		fluo::BBox bbox = b->bbox();
		matrix[0] = float(bbox.Max().x() - bbox.Min().x());
		matrix[1] = 0.0f;
		matrix[2] = 0.0f;
		matrix[3] = 0.0f;
		matrix[4] = 0.0f;
		matrix[5] = float(bbox.Max().y() - bbox.Min().y());
		matrix[6] = 0.0f;
		matrix[7] = 0.0f;
		matrix[8] = 0.0f;
		matrix[9] = 0.0f;
		matrix[10] = float(bbox.Max().z() - bbox.Min().z());
		matrix[11] = 0.0f;
		matrix[12] = float(bbox.Min().x());
		matrix[13] = float(bbox.Min().y());
		matrix[14] = float(bbox.Min().z());
		matrix[15] = 1.0f;
		seg_shader->setLocalParamMatrix(2, matrix);

		//set viewport size the same as the texture
		fbo_mask->set_viewport({ 0, 0, b->nx(), b->ny() });
		fbo_mask->apply_state();

		//load the texture
		GLint tex_id = -1;
		GLint vd_id = load_brick(b, GL_NEAREST, compression_);
		GLint mask_id = load_brick_mask(b);
		switch (type)
		{
		case 0:
		case 1:
			tex_id = mask_id;
			break;
		case 2:
			tex_id = vd_id;
			break;
		}

		//size and sample rate
		seg_shader->setLocalParam(4, 1.0 / b->nx(), 1.0 / b->ny(), 1.0 / b->nz(),
			mode_ == RenderMode::RENDER_MODE_OVER ? 1.0 / rate : 1.0);

		//draw each slice
		for (int z = 0; z < b->nz(); z++)
		{
			fbo_mask->attach_texture(AttachmentPoint::Color(0), tex_id, z);
			draw_view_quad(double(z + 0.5) / double(b->nz()));
		}

		if (num > 1 && type == 1 && order)
			copy_mask_border(mask_id, b, order);

		//test cl
		//if (estimate && type == 0)
		//{
		//	double temp = calc_hist_3d(vd_id, mask_id, b->nx(), b->ny(), b->nz());
		//	est_thresh_ = std::max(est_thresh_, temp);
		//}

	}

	//release 2d mask
	release_texture(6, GL_TEXTURE_2D);
	//release 2d weight map
	if (use_2d)
	{
		release_texture(4, GL_TEXTURE_2D);
		release_texture(5, GL_TEXTURE_2D);
	}

	//release 3d mask
	release_texture((*bricks)[0]->nmask(), GL_TEXTURE_3D);

	//release seg shader
	if (seg_shader)
		seg_shader->unbind();

	// Release texture
	release_texture(0, GL_TEXTURE_3D);
}

//for multibrick, copy border to continue diffusion
void VolumeRenderer::copy_mask_border(GLint btex, TextureBrick* b, int order)
{
	auto tex = tex_.lock();
	if (!tex)
		return;

#ifdef _DARWIN
	return;
#endif

	if (!b || !btex || !order)
		return;

	TextureBrick* nb;//neighbor brick
	unsigned int nid;//neighbor id
	unsigned int bid = b->get_id();
	GLint nbtex;//tex name of neighbor
	int bnx, bny, bnz, nbnx, nbny, nbnz;
	bnx = b->nx(); bny = b->ny(); bnz = b->nz();
	if (order == 2)
	{
		//negx
		nid = tex->negxid(bid);
		if (nid != bid)
		{
			nb = tex->get_brick(nid);
			if (nb && nb->is_mask_act())
			{
				nbnx = nb->nx();
				nbtex = load_brick_mask(nb);
				glCopyImageSubData(
					btex, GL_TEXTURE_3D, 0,
					0, 0, 0,
					nbtex, GL_TEXTURE_3D, 0,
					nbnx - 1, 0, 0,
					1, bny, bnz);
			}
		}
		//negy
		nid = tex->negyid(bid);
		if (nid != bid)
		{
			nb = tex->get_brick(nid);
			if (nb && nb->is_mask_act())
			{
				nbny = nb->ny();
				nbtex = load_brick_mask(nb);
				glCopyImageSubData(
					btex, GL_TEXTURE_3D, 0,
					0, 0, 0,
					nbtex, GL_TEXTURE_3D, 0,
					0, nbny - 1, 0,
					bnx, 1, bnz);
			}
		}
		//negz
		nid = tex->negzid(bid);
		if (nid != bid)
		{
			nb = tex->get_brick(nid);
			if (nb && nb->is_mask_act())
			{
				nbnz = nb->nz();
				nbtex = load_brick_mask(nb);
				glCopyImageSubData(
					btex, GL_TEXTURE_3D, 0,
					0, 0, 0,
					nbtex, GL_TEXTURE_3D, 0,
					0, 0, nbnz - 1,
					bnx, bny, 1);
			}
		}
	}
	else
	{
		//posx
		nid = tex->posxid(bid);
		if (nid != bid)
		{
			nb = tex->get_brick(nid);
			if (nb && nb->is_mask_act())
			{
				nbtex = load_brick_mask(nb);
				glCopyImageSubData(
					btex, GL_TEXTURE_3D, 0,
					bnx - 1, 0, 0,
					nbtex, GL_TEXTURE_3D, 0,
					0, 0, 0,
					1, bny, bnz);
			}
		}
		//posy
		nid = tex->posyid(bid);
		if (nid != bid)
		{
			nb = tex->get_brick(nid);
			if (nb && nb->is_mask_act())
			{
				nbtex = load_brick_mask(nb);
				glCopyImageSubData(
					btex, GL_TEXTURE_3D, 0,
					0, bny - 1, 0,
					nbtex, GL_TEXTURE_3D, 0,
					0, 0, 0,
					bnx, 1, bnz);
			}
		}
		//posz
		nid = tex->poszid(bid);
		if (nid != bid)
		{
			nb = tex->get_brick(nid);
			if (nb && nb->is_mask_act())
			{
				nbtex = load_brick_mask(nb);
				glCopyImageSubData(
					btex, GL_TEXTURE_3D, 0,
					0, 0, bnz - 1,
					nbtex, GL_TEXTURE_3D, 0,
					0, 0, 0,
					bnx, bny, 1);
			}
		}
	}
}

//calculation
void VolumeRenderer::calculate(int type, VolumeRenderer* vr_a, VolumeRenderer* vr_b)
{
	auto tex = tex_.lock();
	if (!tex)
		return;

	//sync sorting
	fluo::Ray view_ray(fluo::Point(0.802, 0.267, 0.534), fluo::Vector(0.802, 0.267, 0.534));
	tex->set_sort_bricks();
	std::vector<TextureBrick*>* bricks = tex->get_sorted_bricks(view_ray);
	if (!bricks || bricks->size() == 0)
		return;
	std::vector<TextureBrick*>* bricks_a = 0;
	std::vector<TextureBrick*>* bricks_b = 0;

	std::shared_ptr<Texture> tex_a, tex_b;
	if (vr_a)
	{
		tex_a = vr_a->tex_.lock();
		if (tex_a)
		{
			bricks_a = tex_a->get_bricks();
			tex_a->set_sort_bricks();
			bricks_a = tex_a->get_sorted_bricks(view_ray);
		}
	}
	if (vr_b)
	{
		tex_b = vr_b->tex_.lock();
		if (tex_b)
		{
			tex_b->set_sort_bricks();
			bricks_b = tex_b->get_sorted_bricks(view_ray);
		}
	}

	//mask frame buffer object
	auto fbo_calc =
		glbin_framebuffer_manager.framebuffer(flvr::FBRole::Volume, 0, 0, "");
	assert(fbo_calc);
	auto guard = glbin_framebuffer_manager.bind_scoped(fbo_calc);

	//--------------------------------------------------------------------------
	// Set up shaders
	//calculate shader
	auto cal_shader = glbin_shader_manager.shader(gstVolCalShader,
		ShaderParams::VolCal(type));
	assert(cal_shader);
	cal_shader->bind();

	//set uniforms
	if (type == 1 ||
		type == 2 ||
		type == 3 ||
		type == 8)
		cal_shader->setLocalParam(0, vr_a ? vr_a->get_scalar_scale() : 1.0,
			vr_b ? vr_b->get_scalar_scale() : 1.0,
			(vr_a && tex_a && tex_a->nmask() != -1) ? 1.0 : 0.0,
			(vr_b && tex_b && tex_b->nmask() != -1) ? 1.0 : 0.0);
	else if (type == 4 ||
		type == 5 ||
		type == 6 ||
		type == 7)
		cal_shader->setLocalParam(0, 1.0, 1.0, 0.0, 0.0);
	else
		cal_shader->setLocalParam(0, vr_a ? vr_a->get_scalar_scale() : 1.0,
			vr_b ? vr_b->get_scalar_scale() : 1.0,
			(vr_a && vr_a->get_inversion()) ? -1.0 : 0.0,
			(vr_b && vr_b->get_inversion()) ? -1.0 : 0.0);
	if (vr_a && (type == 6 || type == 7))
	{
		cal_shader->setLocalParam(2, inv_ ? -scalar_scale_ : scalar_scale_, gm_scale_, lo_thresh_, hi_thresh_);
		cal_shader->setLocalParam(3, 1.0 / gamma3d_, lo_offset_, hi_offset_, sw_);
		cal_shader->setLocalParam(17, gm_low_, gm_high_, gm_max_, 0.0);
	}

	for (unsigned int i = 0; i < bricks->size(); i++)
	{
		TextureBrick* b = (*bricks)[i];

		//set viewport size the same as the texture
		fbo_calc->set_viewport({ 0, 0, b->nx(), b->ny() });
		fbo_calc->apply_state();

		//load the texture
		GLuint tex_id = load_brick(b, GL_NEAREST);
		if (bricks_a) vr_a->load_brick((*bricks_a)[i], GL_NEAREST, false, 1);
		if (bricks_b) vr_b->load_brick((*bricks_b)[i], GL_NEAREST, false, 2);
		if ((type == 5 || type == 6 || type == 7) && bricks_a) vr_a->load_brick_mask((*bricks_a)[i]);
		if (type == 8)
		{
			if (bricks_a)
				vr_a->load_brick_mask((*bricks_a)[i], GL_NEAREST, false, 3);
			if (bricks_b)
				vr_b->load_brick_mask((*bricks_b)[i], GL_NEAREST, false, 4);
		}
		//draw each slice
		for (int z = 0; z < b->nz(); z++)
		{
			fbo_calc->attach_texture(AttachmentPoint::Color(0), tex_id, z);
			draw_view_quad(double(z + 0.5) / double(b->nz()));
		}
	}

	//release 3d mask
	release_texture((*bricks)[0]->nmask(), GL_TEXTURE_3D);
	release_texture(4, GL_TEXTURE_3D);
	//release 3d label
	release_texture((*bricks)[0]->nlabel(), GL_TEXTURE_3D);

	//release seg shader
	cal_shader->unbind();
}

//return the data volume
void VolumeRenderer::return_volume()
{
	auto tex = tex_.lock();
	if (!tex)
		return;

	std::vector<TextureBrick*>* bricks = tex->get_bricks();
	if (!bricks || bricks->size() == 0)
		return;

	int c = 0;
	for (unsigned int i = 0; i < bricks->size(); i++)
	{
		TextureBrick* b = (*bricks)[i];
		load_brick(b, GL_NEAREST);
		int nb = b->nb(c);
		GLenum format;
		if (nb < 3)
			format = GL_RED;
		else
			format = GL_RGBA;

		// download texture data
		int sx = b->sx();
		int sy = b->sy();
		glPixelStorei(GL_PACK_ROW_LENGTH, sx);
		glPixelStorei(GL_PACK_IMAGE_HEIGHT, sy);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);

		GLenum type = b->tex_type(c);
		void* data = b->tex_data(c);
		glGetTexImage(GL_TEXTURE_3D, 0, format,
			type, data);

		glPixelStorei(GL_PACK_ROW_LENGTH, 0);
		glPixelStorei(GL_PACK_IMAGE_HEIGHT, 0);
		glPixelStorei(GL_PACK_ALIGNMENT, 4);
	}

	//release 3d texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, 0);
}

//return the mask volume
void VolumeRenderer::return_mask(int order)
{
	auto tex = tex_.lock();
	if (!tex)
		return;

	std::vector<TextureBrick*>* bricks = tex->get_bricks();
	if (!bricks || bricks->size() == 0)
		return;

	int c = tex->nmask();
	if (c < 0 || c >= TEXTURE_MAX_COMPONENTS)
		return;

	size_t i;
	size_t num = bricks->size();
	for (i = ((order == 2) ? (num - 1) : 0);
		(order == 2) ? (i >= 0) : (i < num);
		i += ((order == 2) ? -1 : 1))
	{
		TextureBrick* b = (*bricks)[i];
		if (!b || !b->is_mask_valid())
			continue;

		load_brick_mask(b);
		glActiveTexture(GL_TEXTURE0 + c);

		// download texture data
		int sx = b->sx();
		int sy = b->sy();
		glPixelStorei(GL_PACK_ROW_LENGTH, sx);
		glPixelStorei(GL_PACK_IMAGE_HEIGHT, sy);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);

		GLenum type = b->tex_type(c);
		void* data = b->tex_data(c);
		glGetTexImage(GL_TEXTURE_3D, 0, GL_RED,
			type, data);

		glPixelStorei(GL_PACK_ROW_LENGTH, 0);
		glPixelStorei(GL_PACK_IMAGE_HEIGHT, 0);
		glPixelStorei(GL_PACK_ALIGNMENT, 4);
	}

	//release mask texture
	release_texture(c, GL_TEXTURE_3D);
}

//return the label volume
void VolumeRenderer::return_label()
{
	auto tex = tex_.lock();
	if (!tex)
		return;

	std::vector<TextureBrick*>* bricks = tex->get_bricks_id();
	if (!bricks || bricks->size() == 0)
		return;

	int c = tex->nlabel();
	if (c < 0 || c >= TEXTURE_MAX_COMPONENTS)
		return;

	for (unsigned int i = 0; i < bricks->size(); i++)
	{
		TextureBrick* b = (*bricks)[i];
		load_brick_label(b);
		glActiveTexture(GL_TEXTURE0 + c);

		//download texture data
		glPixelStorei(GL_PACK_ROW_LENGTH, b->sx());
		glPixelStorei(GL_PACK_IMAGE_HEIGHT, b->sy());
		glPixelStorei(GL_PACK_ALIGNMENT, 4);

		glGetTexImage(GL_TEXTURE_3D, 0, GL_RED_INTEGER,
			b->tex_type(c), b->tex_data(c));

		glPixelStorei(GL_PACK_ROW_LENGTH, 0);
		glPixelStorei(GL_PACK_IMAGE_HEIGHT, 0);
		//glPixelStorei(GL_PACK_ALIGNMENT, 4);
	}

	//release label texture
	release_texture(c, GL_TEXTURE_3D);
}

void VolumeRenderer::set_matrices(glm::mat4& mv_mat, glm::mat4& proj_mat, glm::mat4& tex_mat)
{
	m_mv_mat = mv_mat;
	m_proj_mat = proj_mat;
	m_tex_mat = tex_mat;
	update_mv_tex_scl_mat();
}

void VolumeRenderer::update_mv_tex_scl_mat()
{
	auto tex = tex_.lock();
	if (!tex)
		return;
	fluo::Transform* tform = tex->transform();
	double mvmat[16];
	tform->get_trans(mvmat);
	m_mv_tex_scl_mat = glm::mat4(
		mvmat[0], mvmat[4], mvmat[8], mvmat[12],
		mvmat[1], mvmat[5], mvmat[9], mvmat[13],
		mvmat[2], mvmat[6], mvmat[10], mvmat[14],
		mvmat[3], mvmat[7], mvmat[11], mvmat[15]);
	m_mv_tex_scl_mat = m_mv_mat * m_mv_tex_scl_mat;
}
