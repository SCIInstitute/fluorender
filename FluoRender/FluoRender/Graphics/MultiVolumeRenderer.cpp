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
#include <MultiVolumeRenderer.h>
#include <Global.h>
#include <Names.h>
#include <MainSettings.h>
#include <VolumeRenderer.h>
#include <ShaderProgram.h>
#include <Texture.h>
#include <TextureBrick.h>
#include <Framebuffer.h>
#include <VertexArray.h>
#include <ImgShader.h>
#include <VolShader.h>
#include <Ray.h>
#include <Plane.h>
#include <compatibility.h>
#include <algorithm>
#include <array>
#include <glm/gtc/type_ptr.hpp>

using namespace flvr;

MultiVolumeRenderer::MultiVolumeRenderer()
	: depth_peel_(0),
	micro_blend_(false),
	noise_red_(false),
	interactive_mode_(false),
	num_slices_(0)
{
}

MultiVolumeRenderer::MultiVolumeRenderer(MultiVolumeRenderer& copy)
	: depth_peel_(copy.depth_peel_),
	micro_blend_(copy.micro_blend_),
	noise_red_(false),
	interactive_mode_(copy.interactive_mode_),
	num_slices_(0)
{
}

MultiVolumeRenderer::~MultiVolumeRenderer()
{
}

void MultiVolumeRenderer::set_viewport(const fluo::Vector4i& vp)
{
	viewport_ = vp;
	for (auto it : vr_list_)
	{
		if (it)
			it->set_viewport(vp);
	}
}

//mode and sampling rate
void MultiVolumeRenderer::set_interactive_mode(bool mode)
{
	interactive_mode_ = mode;
}

int MultiVolumeRenderer::get_slice_num()
{
	return static_cast<int>(num_slices_);
}

//manages volume renderers for rendering
void MultiVolumeRenderer::add_vr(VolumeRenderer* vr)
{
	if (!vr)
		return;

	for (unsigned int i = 0; i < vr_list_.size(); i++)
	{
		if (vr_list_[i] == vr)
			return;
	}

	vr_list_.push_back(vr);

	auto tex = vr->tex_.lock();
	if (tex)
	{
		bbox_.extend(*(tex->bbox()));
		res_ = Max(res_, tex->get_res());
	}
}

void MultiVolumeRenderer::clear_vr()
{
	vr_list_.clear();
	bbox_.reset();
	res_ = fluo::Vector(0.0);
}

int MultiVolumeRenderer::get_vr_num()
{
	return int(vr_list_.size());
}

//draw
void MultiVolumeRenderer::draw(bool draw_wireframe_p,
	bool interactive_mode_p,
	bool orthographic_p,
	bool intp)
{
	bool adaptive = false;
	int interactive_quality = glbin_settings.m_interactive_quality;
	for (size_t i = 0; i < vr_list_.size(); ++i)
	{
		if (vr_list_[i]->get_adaptive())
		{
			adaptive = true;
			break;
		}
	}
	draw_volume(adaptive, interactive_mode_p, orthographic_p, intp);
	if (draw_wireframe_p)
		draw_wireframe(adaptive, orthographic_p);
}

void MultiVolumeRenderer::draw_volume(bool adaptive, bool interactive_mode_p, bool orthographic_p, bool intp)
{
	if (get_vr_num() <= 0 || !(vr_list_[0]))
		return;

	fluo::Ray view_ray = vr_list_[0]->compute_view();
	fluo::Ray snapview = vr_list_[0]->compute_snapview(0.4);
	float zoom_data_clamp = vr_list_[0]->zoom_data_;
	if (orthographic_p)
		zoom_data_clamp = std::clamp(static_cast<float>(zoom_data_clamp), 0.2f, 35.0f);
	else
		zoom_data_clamp = std::clamp(static_cast<float>(zoom_data_clamp), 1.0f, 10.0f);

	set_interactive_mode(interactive_mode_p);

	// Set sampling rate based on interaction.
	double rate = vr_list_[0]->get_sample_rate();
	RenderMode render_mode = vr_list_[0]->render_mode_;
	fluo::Vector diag = bbox_.diagonal();
	fluo::Vector cell_diag(diag.x() / res_.x(),
		diag.y() / res_.y(),
		diag.z() / res_.z());
	double dt;
	size_t est_slices;
	if (rate > 0.0)
	{
		dt = cell_diag.length() /
			vr_list_[0]->compute_rate_scale(snapview.direction()) / rate;
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

	int w = viewport_[2];
	int h = viewport_[3];
	std::string buf_name = vr_list_[0]->get_buffer_name();
	Size2D new_size = vr_list_[0]->resize(buf_name);
	int w2 = new_size.w();
	int h2 = new_size.h();
	bool depth = false;
	for (size_t i = 0; i < vr_list_.size(); ++i)
		depth |= (vr_list_[i]->depth_ || vr_list_[i]->shading_ || vr_list_[i]->outline_);

	auto cur_buffer = glbin_framebuffer_manager.current();
	flvr::FBRole role = depth ? flvr::FBRole::RenderColorFxFilter : flvr::FBRole::RenderColorFilter;
	auto blend_buffer = glbin_framebuffer_manager.framebuffer(role, w2, h2, buf_name);
	assert(blend_buffer);
	//set up clear color and viewport size
	blend_buffer->set_clear_color({ clear_color_[0], clear_color_[1], clear_color_[2], clear_color_[3] });
	blend_buffer->set_viewport({ viewport_[0], viewport_[1], w2, h2 });
	// set up blending
	blend_buffer->set_blend_enabled_all(true);
	//normal: b2f:(1, 1-a), f2b:(1-a, 1), mip: max(1, 1)
	switch (render_mode)
	{
	case RenderMode::Standard:
	case RenderMode::Overlay:
		blend_buffer->set_blend_equation(0, flvr::BlendEquation::Add, flvr::BlendEquation::Add);
		if (glbin_settings.m_update_order == 0)
			blend_buffer->set_blend_func(0,
				flvr::BlendFactor::One, flvr::BlendFactor::OneMinusSrcAlpha,
				flvr::BlendFactor::One, flvr::BlendFactor::OneMinusSrcAlpha);
		else if (glbin_settings.m_update_order == 1)
			blend_buffer->set_blend_func(0,
				flvr::BlendFactor::OneMinusDstAlpha, flvr::BlendFactor::One,
				flvr::BlendFactor::OneMinusDstAlpha, flvr::BlendFactor::One);
		break;
	case RenderMode::Mip:
		blend_buffer->set_blend_equation(0, flvr::BlendEquation::Max, flvr::BlendEquation::Max);
		blend_buffer->set_blend_func(0,
			flvr::BlendFactor::One, flvr::BlendFactor::One,
			flvr::BlendFactor::One, flvr::BlendFactor::One);
		break;
	default:
		break;
	}
	if (depth)
	{
		blend_buffer->set_blend_equation(1, flvr::BlendEquation::Add, flvr::BlendEquation::Add);
		blend_buffer->set_blend_func(1,
			flvr::BlendFactor::One, flvr::BlendFactor::One,
			flvr::BlendFactor::One, flvr::BlendFactor::One);
	}
	glbin_framebuffer_manager.bind(blend_buffer);
	blend_buffer->clear_base(true, false);
	bool clear_depth = depth;
	if (glbin_settings.m_mem_swap &&
		TextureRenderer::start_update_loop_ &&
		!TextureRenderer::done_update_loop_)
		clear_depth = false;
	if (clear_depth)
		blend_buffer->clear_attachment(AttachmentPoint::Color(1), std::array<float, 2>{ 0.0f, 0.0f }.data());

	for (size_t i = 0; i < vr_list_.size(); ++i)
		vr_list_[i]->eval_ml_mode();

	int quota_bricks_chan = vr_list_[0]->get_quota_bricks_chan();
	std::vector<TextureBrick*>* bs = 0;
	fluo::Point pt = TextureRenderer::quota_center_;
	if (glbin_settings.m_mem_swap &&
		TextureRenderer::interactive_)
		//bs = vr_list_[0]->tex_->get_closest_bricks(
		//TextureRenderer::quota_center_,
		//quota_bricks_chan, false,
		//view_ray, orthographic_p);
		bs = get_combined_bricks(
			pt, view_ray, orthographic_p);
	else
	{
		auto tex = vr_list_[0]->tex_.lock();
		if (tex)
			bs = tex->get_sorted_bricks(
				view_ray, orthographic_p);
	}

	if (bs)
	{
		num_slices_ = 0;
		bool multibricks = bs->size() > 1;
		bool drawn_once = false;
		for (unsigned int i = 0; i < bs->size(); i++)
		{
			if (glbin_settings.m_mem_swap && drawn_once)
			{
				unsigned long long rn_time = GET_TICK_COUNT();
				if (rn_time - TextureRenderer::st_time_ > TextureRenderer::get_up_time())
					break;
			}

			TextureBrick* b = (*bs)[i];
			if (glbin_settings.m_mem_swap &&
				TextureRenderer::start_update_loop_ &&
				!TextureRenderer::done_update_loop_)
			{
				if (b->drawn(0))
					continue;
			}

			if (!vr_list_[0]->test_against_view(b->bbox(), !orthographic_p))// Clip against view
			{
				if (glbin_settings.m_mem_swap &&
					TextureRenderer::start_update_loop_ &&
					!TextureRenderer::done_update_loop_)
				{
					for (unsigned int j = 0; j < vr_list_.size(); j++)
					{
						std::vector<TextureBrick*>* bs_tmp = 0;
						auto tex = vr_list_[j]->tex_.lock();
						if (tex)
						{
							if (TextureRenderer::interactive_)
								//bs_tmp = tex->get_closest_bricks(
								//TextureRenderer::quota_center_,
								//quota_bricks_chan, false,
								//view_ray, orthographic_p);
								bs_tmp = tex->get_quota_bricks();
							else
								bs_tmp = tex->get_sorted_bricks(
									view_ray, orthographic_p);
						}
						if (!(*bs_tmp)[i]->drawn(0))
						{
							(*bs_tmp)[i]->set_drawn(0, true);
							TextureRenderer::cur_chan_brick_num_++;
						}
					}
				}
				continue;
			}
			vertex.clear();
			index.clear();
			size.clear();
			b->compute_polygons(snapview, dt, vertex, index, size, multibricks);

			num_slices_ += size.size();

			if (vertex.size() == 0) { continue; }

			draw_polygons_vol(b, rate, vertex, index, size, view_ray,
				i, orthographic_p, w2, h2, intp, quota_bricks_chan);

			int vrn = (int)(vr_list_.size());
			if (glbin_settings.m_mem_swap && !b->drawn(0))
			{
				b->set_drawn(0, true);
				TextureRenderer::cur_brick_num_ += vrn;
				TextureRenderer::cur_chan_brick_num_ += vrn;
			}
			if (glbin_settings.m_mem_swap)
				TextureRenderer::finished_bricks_ += vrn;
			drawn_once = true;
		}
	}

	if (glbin_settings.m_mem_swap &&
		TextureRenderer::cur_brick_num_ >= TextureRenderer::total_brick_num_)
	{
		TextureRenderer::done_update_loop_ = true;
		TextureRenderer::active_view_ = -1;
	}
	if (glbin_settings.m_mem_swap)
	{
		size_t num = 0;
		for (size_t i = 0; i < vr_list_.size(); ++i)
		{
			auto tex = vr_list_[i]->tex_.lock();
			if (tex)
				num += tex->get_bricks()->size();
		}
		if (TextureRenderer::cur_chan_brick_num_ >= static_cast<int>(num))
		{
			TextureRenderer::done_current_chan_ = true;
			TextureRenderer::clear_chan_buffer_ = true;
			TextureRenderer::cur_chan_brick_num_ = 0;
		}
	}

	std::shared_ptr<ShaderProgram> img_shader;

	std::shared_ptr<Framebuffer> filter_buffer;
	if (noise_red_ &&
		render_mode != RenderMode::Overlay)
	{
		//FILTERING
		filter_buffer = glbin_framebuffer_manager.framebuffer(
			flvr::FBRole::RenderColorFilter, w, h, gstRBFilter);
		assert(filter_buffer);
		//set viewport size
		filter_buffer->set_viewport({ viewport_[0], viewport_[1], viewport_[2], viewport_[3] });
		glbin_framebuffer_manager.bind(filter_buffer);
		filter_buffer->clear_base(true, false);

		blend_buffer->bind_texture(AttachmentPoint::Color(0), 0);

		img_shader = glbin_shader_manager.shader(gstImgShader,
			ShaderParams::Img(IMG_SHDR_FILTER_LANCZOS_BICUBIC, 0));
		assert(img_shader);
		img_shader->bind();

		img_shader->setLocalParam(0, zoom_data_clamp / w, zoom_data_clamp / h, zoom_data_clamp, 0.0);

		vr_list_[0]->draw_view_quad();

		img_shader->unbind();

		blend_buffer->unbind_texture(AttachmentPoint::Color(0));
	}

	//current
	assert(cur_buffer);
	cur_buffer->set_viewport({ viewport_[0], viewport_[1], viewport_[2], viewport_[3] });
	glbin_framebuffer_manager.bind(cur_buffer);

	if (noise_red_ &&
		render_mode != RenderMode::Overlay)
		filter_buffer->bind_texture(AttachmentPoint::Color(0), 0);
	else
		blend_buffer->bind_texture(AttachmentPoint::Color(0), 0);

	img_shader = glbin_shader_manager.shader(gstImgShader,
		ShaderParams::Img(IMG_SHDR_TEXTURE_LOOKUP, 0));
	assert(img_shader);
	img_shader->bind();

	vr_list_[0]->draw_view_quad();

	img_shader->unbind();

	if (noise_red_ &&
		render_mode != RenderMode::Overlay)
		filter_buffer->unbind_texture(AttachmentPoint::Color(0));
	else
		blend_buffer->unbind_texture(AttachmentPoint::Color(0));
}

void MultiVolumeRenderer::draw_polygons_vol(
	TextureBrick* b, double rate,
	std::vector<float>& vertex,
	std::vector<uint32_t>& index,
	std::vector<uint32_t>& size,
	fluo::Ray& view_ray,
	int bi, bool orthographic_p,
	int w, int h, bool intp,
	int quota_bricks_chan)
{
	//check vr_list size
	if (vr_list_.size() <= 0)
		return;

	std::shared_ptr<Framebuffer> micro_blend_buffer;
	std::shared_ptr<Framebuffer> cur_buffer;
	if (micro_blend_/* && colormap_mode_!=2*/)
	{
		cur_buffer = glbin_framebuffer_manager.current();
		micro_blend_buffer = glbin_framebuffer_manager.framebuffer(
			flvr::FBRole::RenderColor, w, h, gstRBMicroBlend);
		assert(micro_blend_buffer);
	}

	bool set_pointers = false;
	if (!va_slices_ || !va_slices_->valid())
	{
		va_slices_ = glbin_vertex_array_manager.vertex_array(true, true);
		set_pointers = true;
	}
	if (va_slices_)
	{
		va_slices_->buffer_data(
			VABufferType::VABuf_Coord, sizeof(float) * vertex.size(),
			&vertex[0], BufferUsage::StreamDraw);
		va_slices_->buffer_data(
			VABufferType::VABuf_Index, sizeof(uint32_t) * index.size(),
			&index[0], BufferUsage::StreamDraw);
		if (set_pointers)
		{
			va_slices_->attrib_pointer(
				0, 3, VertexAttribType::Float, false,
				6 * sizeof(float),
				(const void*)0);
			va_slices_->attrib_pointer(
				1, 3, VertexAttribType::Float, false,
				6 * sizeof(float),
				(const void*)12);
		}
		va_slices_->draw_begin();
	}

	size_t location = 0;
	size_t idx_num;
	double rate_factor = 1.0;
	RenderMode render_mode = vr_list_[0]->render_mode_;
	if (render_mode == RenderMode::Standard ||
		render_mode == RenderMode::Overlay)
		rate_factor = 1.0 / rate;
	fluo::Vector light = view_ray.direction();
	light.safe_normalize();

	for (size_t i = 0; i < size.size(); i++)
	{
		if (micro_blend_/*&& colormap_mode_!=2*/)
		{
			//set blend buffer
			//blend: (add, max)(1, 1)
			micro_blend_buffer->set_clear_color({ clear_color_[0], clear_color_[1], clear_color_[2], clear_color_[3] });
			micro_blend_buffer->set_blend_enabled_all(true);
			micro_blend_buffer->set_blend_equation_all(flvr::BlendEquation::Add, flvr::BlendEquation::Max);
			micro_blend_buffer->set_blend_func_all(flvr::BlendFactor::One, flvr::BlendFactor::One, flvr::BlendFactor::One, flvr::BlendFactor::One);
			glbin_framebuffer_manager.bind(micro_blend_buffer);
			micro_blend_buffer->clear_base(true, false);
		}

		if (va_slices_)
			va_slices_->draw_begin();

		//draw a single slice
		for (size_t tn = 0; tn < vr_list_.size(); ++tn)
		{
			bool use_fog = vr_list_[tn]->m_use_fog;

			// Set up shaders
			std::shared_ptr<ShaderProgram> shader;
			bool grad = vr_list_[tn]->gm_low_ > 0.0 ||
				vr_list_[tn]->gm_high_ < vr_list_[tn]->gm_max_ ||
				vr_list_[tn]->colormap_proj_ == ColormapProj::Gradient ||
				vr_list_[tn]->colormap_proj_ == ColormapProj::Normal;
			auto tex = vr_list_[tn]->tex_.lock();
			assert(tex);
			int nc = tex->nc();
			bool depth = vr_list_[tn]->depth_ || vr_list_[tn]->shading_ || vr_list_[tn]->outline_;
			shader = glbin_shader_manager.shader(gstVolShader,
				ShaderParams::Volume(
					false,
					nc,
					vr_list_[tn]->shading_, use_fog,
					vr_list_[tn]->depth_peel_, true,
					grad,
					vr_list_[tn]->main_mode_,
					vr_list_[tn]->mask_mode_,
					vr_list_[tn]->render_mode_,
					vr_list_[tn]->color_mode_,
					vr_list_[tn]->colormap_,
					vr_list_[tn]->colormap_proj_,
					vr_list_[tn]->solid_,
					1,
					depth));
			assert(shader);
			shader->bind();

			//setup depth peeling
			shader->setLocalParam(7, 1.0 / double(w), 1.0 / double(h), 0.0, 0.0);

			//fog
			shader->setLocalParam(8,
				vr_list_[tn]->m_fog_intensity,
				vr_list_[tn]->m_fog_start,
				vr_list_[tn]->m_fog_end, 0.0);

			// render bricks
			glm::mat4 mv_tex_scl_mat = vr_list_[tn]->get_mv_tex_scl_mat();
			glm::mat4 proj_mat = vr_list_[tn]->get_proj_mat();
			shader->setLocalParamMatrix(0, glm::value_ptr(proj_mat));
			shader->setLocalParamMatrix(1, glm::value_ptr(mv_tex_scl_mat));

			auto texel_b = fluo::Vector(1.0) / b->get_size();
			shader->setLocalParam(4, texel_b.x(), texel_b.y(), texel_b.z(), rate_factor);

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

			// set shader parameters
			shader->setLocalParam(0,
				light.x(), light.y(), light.z(), 0.0);
			shader->setLocalParam(1,
				vr_list_[tn]->shading_strength_, vr_list_[tn]->shading_shine_,
				glbin_settings.m_shadow_dir_x, glbin_settings.m_shadow_dir_y);
			//transfer function
			shader->setLocalParam(2,
				vr_list_[tn]->inv_ ?
				-vr_list_[tn]->scalar_scale_ :
				vr_list_[tn]->scalar_scale_,
				vr_list_[tn]->gm_scale_,
				vr_list_[tn]->lo_thresh_,
				vr_list_[tn]->hi_thresh_);
			shader->setLocalParam(3,
				1.0 / vr_list_[tn]->gamma3d_,
				vr_list_[tn]->lo_offset_,
				vr_list_[tn]->hi_offset_,
				vr_list_[tn]->sw_);

			//spacings
			auto spacing = tex->get_spacing();
			shader->setLocalParam(5, spacing.x(), spacing.y(), spacing.z(), vr_list_[tn]->shuffle_);

			//colormap
			shader->setLocalParam(6, vr_list_[tn]->colormap_low_value_,
				vr_list_[tn]->colormap_hi_value_,
				vr_list_[tn]->colormap_hi_value_ - vr_list_[tn]->colormap_low_value_,
				vr_list_[tn]->colormap_inv_);
			//color
			shader->setLocalParam(9, vr_list_[tn]->color_.r(),
				vr_list_[tn]->color_.g(), vr_list_[tn]->color_.b(),
				vr_list_[tn]->alpha_power_);
			shader->setLocalParam(16, vr_list_[tn]->mask_color_.r(),
				vr_list_[tn]->mask_color_.g(), vr_list_[tn]->mask_color_.b(),
				vr_list_[tn]->mask_thresh_);
			//gm
			shader->setLocalParam(17, vr_list_[tn]->gm_low_,
				vr_list_[tn]->gm_high_, vr_list_[tn]->gm_max_, 0.0);
			//alpha & luminance
			shader->setLocalParam(18, vr_list_[tn]->alpha_,
				vr_list_[tn]->alpha_power_, vr_list_[tn]->luminance_, 0.0);
			//fog color
			shader->setLocalParam(19,
				vr_list_[tn]->fog_color_.r(),
				vr_list_[tn]->fog_color_.g(),
				vr_list_[tn]->fog_color_.b(), 0.0);

			auto planes = vr_list_[tn]->clipping_box_.GetPlanesUnit();
			double abcd[4];
			planes[0].get(abcd);
			shader->setLocalParam(10, abcd[0], abcd[1], abcd[2], abcd[3]);
			planes[1].get(abcd);
			shader->setLocalParam(11, abcd[0], abcd[1], abcd[2], abcd[3]);
			planes[2].get(abcd);
			shader->setLocalParam(12, abcd[0], abcd[1], abcd[2], abcd[3]);
			planes[3].get(abcd);
			shader->setLocalParam(13, abcd[0], abcd[1], abcd[2], abcd[3]);
			planes[4].get(abcd);
			shader->setLocalParam(14, abcd[0], abcd[1], abcd[2], abcd[3]);
			planes[5].get(abcd);
			shader->setLocalParam(15, abcd[0], abcd[1], abcd[2], abcd[3]);

			//bind depth texture for rendering shadows
			//if (vr_list_[tn]->color_mode_ == ColorMode::Depth && blend_buffer)
			//	blend_buffer->bind_texture(AttachmentPoint::Color(0), 0);

			std::vector<TextureBrick*>* bs2 = 0;
			if (tex)
			{
				if (glbin_settings.m_mem_swap &&
					TextureRenderer::interactive_)
					//bs2 = tex->get_closest_bricks(
					//TextureRenderer::quota_center_,
					//quota_bricks_chan, false,
					//view_ray, orthographic_p);
					bs2 = tex->get_quota_bricks();
				else
					bs2 = tex->get_sorted_bricks(
						view_ray, orthographic_p);
			}
			if (!bs2) break;
			if (bi >= (int)bs2->size()) break;

			TextureBrick* b2 = (*bs2)[bi];
			//if (glbin_settings.m_mem_swap && !b2->drawn(0))
			//	b2->set_drawn(0, true);
			if (b2->get_priority() > 0)
				continue;

			GLint filter;
			if (intp)
				filter = GL_LINEAR;
			else
				filter = GL_NEAREST;
			vr_list_[tn]->load_brick(b2, filter, vr_list_[tn]->compression_);
			if (vr_list_[tn]->mask_)
				vr_list_[tn]->load_brick_mask(b2, filter);
			if (vr_list_[tn]->label_)
				vr_list_[tn]->load_brick_label(b2);

			idx_num = (size[i] - 2) * 3;
			if (va_slices_)
				va_slices_->draw_elements(
					PrimitiveType::Triangles, idx_num,
					IndexType::UnsignedInt,
					(const void*)location);

			//release depth texture for rendering shadows
			//if (vr_list_[tn]->color_mode_ == ColorMode::Depth)
			//	vr_list_[tn]->release_texture(4, GL_TEXTURE_2D);

			if (glbin_settings.m_mem_swap && i == 0)
				TextureRenderer::finished_bricks_++;

			//release
			if (vr_list_[tn]->mask_)
				vr_list_[tn]->release_texture(2, GL_TEXTURE_3D);
			if (vr_list_[tn]->label_)
				vr_list_[tn]->release_texture(3, GL_TEXTURE_3D);
			// Release shader.
			shader->unbind();
			//unbind depth texture for rendering shadows
			//if (vr_list_[tn]->color_mode_ == ColorMode::Depth && blend_buffer)
			//	blend_buffer->unbind_texture(AttachmentPoint::Color(0));
		}
		location += idx_num * 4;

		if (va_slices_)
			va_slices_->draw_end();

		if (micro_blend_)
		{
			//set buffer back
			assert(cur_buffer);
			glbin_framebuffer_manager.bind(cur_buffer);
			//draw
			auto img_shader = glbin_shader_manager.shader(gstImgShader,
				ShaderParams::Img(IMG_SHDR_TEXTURE_LOOKUP, 0));
			assert(img_shader);
			img_shader->bind();

			micro_blend_buffer->bind_texture(AttachmentPoint::Color(0), 0);

			vr_list_[0]->draw_view_quad();

			img_shader->unbind();
			micro_blend_buffer->unbind_texture(AttachmentPoint::Color(0));
		}
	}

	//if (TextureRenderer::mem_swap_)
	//  TextureRenderer::finished_bricks_ += (int)vr_list_.size();
}

std::vector<TextureBrick*>* MultiVolumeRenderer::get_combined_bricks(
	fluo::Point& center, fluo::Ray& view, bool is_orthographic)
{
	if (!vr_list_.size())
		return 0;

	auto tex0 = vr_list_[0]->tex_.lock();
	if (!tex0)
		return 0;
	if (!tex0->get_sort_bricks())
		return tex0->get_quota_bricks();

	size_t i, j, k;
	std::vector<TextureBrick*>* bs;
	std::vector<TextureBrick*>* bs0;
	std::vector<TextureBrick*>* result;
	fluo::Point brick_center;
	double d;

	for (i = 0; i < vr_list_.size(); i++)
	{
		auto tex = vr_list_[i]->tex_.lock();
		if (!tex)
			continue;
		//sort each brick list based on distance to center
		bs = tex->get_bricks();
		for (j = 0; j < bs->size(); j++)
		{
			brick_center = (*bs)[j]->bbox().center();
			d = (brick_center - center).length();
			(*bs)[j]->set_d(d);
		}
		std::sort((*bs).begin(), (*bs).end(), TextureBrick::sort_dsc);

		//assign indecis so that bricks can be selected later
		for (j = 0; j < bs->size(); j++)
			(*bs)[j]->set_ind(j);
	}

	//generate quota brick list for vr0
	bs0 = tex0->get_bricks();
	result = tex0->get_quota_bricks();
	result->clear();
	int quota = 0;
	int count;
	TextureBrick* pb;
	size_t ind;
	bool found;
	for (i = 0; i < vr_list_.size(); i++)
	{
		auto tex = vr_list_[i]->tex_.lock();
		if (!tex)
			continue;
		//insert nonduplicated bricks into result list
		bs = tex->get_bricks();
		quota = vr_list_[i]->get_quota_bricks_chan();
		//quota = quota/2+1;
		count = 0;
		for (j = 0; j < bs->size(); j++)
		{
			pb = (*bs)[j];
			if (pb->get_priority() > 0)
				continue;
			ind = pb->get_ind();
			found = false;
			for (k = 0; k < result->size(); k++)
			{
				if (ind == (*result)[k]->get_ind())
				{
					found = true;
					break;
				}
			}
			if (!found)
			{
				result->push_back((*bs0)[ind]);
				count++;
				if (count == quota)
					break;
			}
		}
	}
	//reorder result
	for (i = 0; i < result->size(); i++)
	{
		fluo::Point minp((*result)[i]->bbox().Min());
		fluo::Point maxp((*result)[i]->bbox().Max());
		fluo::Vector diag((*result)[i]->bbox().diagonal());
		minp += diag / 1000.;
		maxp -= diag / 1000.;
		fluo::Point corner[8];
		corner[0] = minp;
		corner[1] = fluo::Point(minp.x(), minp.y(), maxp.z());
		corner[2] = fluo::Point(minp.x(), maxp.y(), minp.z());
		corner[3] = fluo::Point(minp.x(), maxp.y(), maxp.z());
		corner[4] = fluo::Point(maxp.x(), minp.y(), minp.z());
		corner[5] = fluo::Point(maxp.x(), minp.y(), maxp.z());
		corner[6] = fluo::Point(maxp.x(), maxp.y(), minp.z());
		corner[7] = maxp;
		double d = 0.0;
		for (unsigned int c = 0; c < 8; c++)
		{
			double dd;
			if (is_orthographic)
			{
				// orthographic: sort bricks based on distance to the view plane
				dd = Dot(corner[c], view.direction());
			}
			else
			{
				// perspective: sort bricks based on distance to the eye point
				dd = (corner[c] - view.origin()).length();
			}
			if (c == 0 || dd < d) d = dd;
		}
		(*result)[i]->set_d(d);
	}
	if (glbin_settings.m_update_order == 0)
		std::sort((*result).begin(), (*result).end(), TextureBrick::sort_asc);
	else if (glbin_settings.m_update_order == 1)
		std::sort((*result).begin(), (*result).end(), TextureBrick::sort_dsc);
	tex0->reset_sort_bricks();

	//duplicate result into other quota-bricks
	for (i = 1; i < vr_list_.size(); i++)
	{
		auto tex = vr_list_[i]->tex_.lock();
		if (!tex)
			continue;
		bs0 = tex->get_bricks();
		bs = tex->get_quota_bricks();
		bs->clear();

		for (j = 0; j < result->size(); j++)
		{
			ind = (*result)[j]->get_ind();
			bs->push_back((*bs0)[ind]);
		}
		tex->reset_sort_bricks();
	}

	return result;
}

void MultiVolumeRenderer::draw_wireframe(bool adaptive, bool orthographic_p)
{
	if (get_vr_num() <= 0)
		return;

	fluo::Ray view_ray = vr_list_[0]->compute_view();
	fluo::Ray snapview = vr_list_[0]->compute_snapview(0.4);

	// Set sampling rate based on interaction.
	double rate = vr_list_[0]->get_sample_rate();
	fluo::Vector diag = bbox_.diagonal();
	fluo::Vector cell_diag(diag.x() / res_.x(),
		diag.y() / res_.y(),
		diag.z() / res_.z());
	double dt;
	size_t est_slices;
	if (rate > 0.0)
	{
		dt = cell_diag.length() /
			vr_list_[0]->compute_rate_scale(snapview.direction()) / rate;
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
			false, false,
			false,
			MaskMode::None,
			MaskMode::None,
			RenderMode::Standard,
			ColorMode::SingleColor,
			0, ColormapProj::Disabled,
			false, 1, false));
	assert(shader);
	shader->bind();

	//--------------------------------------------------------------------------
	// render bricks
	glm::mat4 mv_tex_scl_mat = vr_list_[0]->get_mv_tex_scl_mat();
	glm::mat4 proj_mat = vr_list_[0]->get_proj_mat();
	shader->setLocalParamMatrix(0, glm::value_ptr(proj_mat));
	shader->setLocalParamMatrix(1, glm::value_ptr(mv_tex_scl_mat));
	//shader->setLocalParamMatrix(5, glm::value_ptr(tex_mat_));
	shader->setLocalParam(0, vr_list_[0]->color_.r(), vr_list_[0]->color_.g(), vr_list_[0]->color_.b(), 1.0);

	std::vector<TextureBrick*>* bs;
	auto tex0 = vr_list_[0]->tex_.lock();
	if (tex0)
		bs = tex0->get_sorted_bricks(view_ray, orthographic_p);

	if (bs)
	{
		bool multibricks = bs->size() > 1;
		for (unsigned int i = 0; i < bs->size(); i++)
		{
			TextureBrick* b = (*bs)[i];
			if (!vr_list_[0]->test_against_view(b->bbox(), !orthographic_p)) continue; // Clip against view.

			vertex.clear();
			index.clear();
			size.clear();

			b->compute_polygons(snapview, dt, vertex, index, size, multibricks);
			vr_list_[0]->draw_polygons_wireframe(vertex, index, size);
		}
	}

	// Release shader.
	shader->unbind();
}
