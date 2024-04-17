//
//  For more information, please see: http://software.sci.utah.edu
//
//  The MIT License
//
//  Copyright (c) 2004 Scientific Computing and Imaging Institute,
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

#include <Global.h>
#include <FLIVR/VolumeRenderer.h>
#include <FLIVR/VolShader.h>
#include <FLIVR/SegShader.h>
#include <FLIVR/VolCalShader.h>
#include <FLIVR/ShaderProgram.h>
#include <FLIVR/TextureBrick.h>
#include <FLIVR/KernelProgram.h>
#include <FLIVR/VolKernel.h>
#include <FLIVR/Framebuffer.h>
#include <FLIVR/VertexArray.h>
#include <utility.h>
#include <compatibility.h>
#include <fstream>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <Debug.h>

namespace flvr
{
	VolShaderFactory TextureRenderer::vol_shader_factory_;
	SegShaderFactory TextureRenderer::seg_shader_factory_;
	VolCalShaderFactory TextureRenderer::cal_shader_factory_;
	ImgShaderFactory TextureRenderer::img_shader_factory_;
	VolKernelFactory TextureRenderer::vol_kernel_factory_;
	double VolumeRenderer::sw_ = 0.0;

	VolumeRenderer::VolumeRenderer(Texture* tex,
		const vector<fluo::Plane*> &planes)
		:TextureRenderer(tex),
		//scalar scaling factor
		scalar_scale_(1.0),
		//gm range
		gm_scale_(1.0),
		//transfer function
		gamma3d_(1.0),
		gm_thresh_(0.0),
		offset_(1.0),
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
		//adaptive
		adaptive_(true),
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
		filter_size_min_(0.0),
		filter_size_max_(0.0),
		filter_size_shp_(0.0),
		inv_(false),
		compression_(false),
		alpha_power_(1.0),
		zoom_(1.0),
		zoom_data_(1.0)
	{
		//mode
		mode_ = MODE_OVER;
		//done loop
		for (int i=0; i<TEXTURE_RENDER_MODES; i++)
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
		gm_thresh_(copy.gm_thresh_),
		offset_(copy.offset_),
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
		//adaptive
		adaptive_(copy.adaptive_),
		//depth peel
		depth_peel_(copy.depth_peel_),
		//segmentation
		ml_mode_(copy.ml_mode_),
		mask_(copy.mask_),
		label_(copy.label_),
		//scale factor
		noise_red_(copy.noise_red_),
		sfactor_(1.0),
		filter_size_min_(0.0),
		filter_size_max_(0.0),
		filter_size_shp_(0.0),
		inv_(copy.inv_),
		compression_(copy.compression_),
		alpha_power_(copy.alpha_power_)
	{
		//mode
		mode_ = copy.mode_;
		//clipping planes
		for (int i=0; i<(int)copy.planes_.size(); i++)
		{
			fluo::Plane* plane = new fluo::Plane(*copy.planes_[i]);
			planes_.push_back(plane);
		}
		//done loop
		for (int i=0; i<TEXTURE_RENDER_MODES; i++)
			done_loop_[i] = false;
	}

	VolumeRenderer::~VolumeRenderer()
	{
		//release clipping planes
		for (int i=0; i<(int)planes_.size(); i++)
		{
			if (planes_[i])
				delete planes_[i];
		}
		planes_.clear();
	}

	//render mode
	void VolumeRenderer::set_mode(RenderMode mode)
	{
		mode_ = mode;
	}

	//range and scale
	void VolumeRenderer::set_scalar_scale(double scale)
	{
		scalar_scale_ = scale;
	}

	double VolumeRenderer::get_scalar_scale()
	{
		return scalar_scale_;
	}

	void VolumeRenderer::set_gm_scale(double scale)
	{
		gm_scale_ = scale;
	}

	//transfer function properties
	void VolumeRenderer::set_gamma3d(double gamma)
	{
		gamma3d_ = gamma;
	}

	double VolumeRenderer::get_gamma3d()
	{
		return gamma3d_;
	}

	void VolumeRenderer::set_gm_thresh(double thresh)
	{
		gm_thresh_ = thresh;
	}

	double VolumeRenderer::get_gm_thresh()
	{
		return gm_thresh_;
	}

	void VolumeRenderer::set_offset(double offset)
	{
		offset_ = offset;
	}

	double VolumeRenderer::get_offset()
	{
		return offset_;
	}

	void VolumeRenderer::set_lo_thresh(double thresh)
	{
		lo_thresh_ = thresh;
	}

	double VolumeRenderer::get_lo_thresh()
	{
		return lo_thresh_;
	}

	void VolumeRenderer::set_hi_thresh(double thresh)
	{
		hi_thresh_ = thresh;
	}

	double VolumeRenderer::get_hi_thresh()
	{
		return hi_thresh_;
	}

	void VolumeRenderer::set_color(fluo::Color color)
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
				h = h0<30.0?h0-180.0:h0<90.0?h0+120.0:h0<210.0?h0-120.0:h0-180.0;
				s = 1.0;
				v = 1.0;
				mask_color_ = fluo::Color(fluo::HSVColor(h<0.0?h+360.0:h, s, v));
			}
		}
	}

	fluo::Color VolumeRenderer::get_color()
	{
		return color_;
	}

	void VolumeRenderer::set_mask_color(fluo::Color color, bool set)
	{
		mask_color_ = color;
		mask_color_set_ = set;
	}

	fluo::Color VolumeRenderer::get_mask_color()
	{
		return mask_color_;
	}

	void VolumeRenderer::set_mask_thresh(double thresh)
	{
		mask_thresh_ = thresh;
	}

	double VolumeRenderer::get_mask_thresh()
	{
		return mask_thresh_;
	}

	void VolumeRenderer::set_alpha(double alpha)
	{
		alpha_ = alpha;
	}

	double VolumeRenderer::get_alpha()
	{
		return alpha_;
	}

	//sampling rate
	void VolumeRenderer::set_sampling_rate(double rate)
	{
		sampling_rate_ = rate;
		irate_ = rate / 2.0;
	}

	double VolumeRenderer::get_sampling_rate()
	{
		return sampling_rate_;
	}

	double VolumeRenderer::num_slices_to_rate(int num_slices)
	{
		const fluo::Vector diag = tex_->bbox()->diagonal();
		const fluo::Vector cell_diag(diag.x()/tex_->nx(),
			diag.y()/tex_->ny(),
			diag.z()/tex_->nz());
		const double dt = diag.length() / num_slices;
		const double rate = cell_diag.length() / dt;

		return rate;
	}

	int VolumeRenderer::get_slice_num()
	{
		return num_slices_;
	}

	//interactive modes
	void VolumeRenderer::set_interactive_rate(double rate)
	{
		irate_ = rate;
	}

	void VolumeRenderer::set_interactive_mode(bool mode)
	{
		imode_ = mode;
	}

	void VolumeRenderer::set_adaptive(bool b)
	{
		adaptive_ = b;
	}

	//clipping planes
	void VolumeRenderer::set_planes(vector<fluo::Plane*> *p)
	{
		int i;
		if (!planes_.empty())
		{
			for (i=0; i<(int)planes_.size(); i++)
			{
				if (planes_[i])
					delete planes_[i];
			}
			planes_.clear();
		}

		for (i=0; i<(int)p->size(); i++)
		{
			fluo::Plane *plane = new fluo::Plane(*(*p)[i]);
			planes_.push_back(plane);
		}
	}

	vector<fluo::Plane*>* VolumeRenderer::get_planes()
	{
		return &planes_;
	}
	
	//interpolation
	bool VolumeRenderer::get_interpolate() {return interpolate_; }

	void VolumeRenderer::set_interpolate(bool b) { interpolate_ = b;}

	//calculating scaling factor, etc
	//calculate scaling factor according to viewport and texture size
	double VolumeRenderer::CalcScaleFactor(double w, double h, double tex_w, double tex_h)
	{
		double cs;
		double vs;
		double sf = 1.0;
		if (w > h)
		{
			cs = fluo::Clamp(tex_h, 500.0, 2000.0);
			vs = h;
		}
		else
		{
			cs = fluo::Clamp(tex_w, 500.0, 2000.0);
			vs = w;
		}
		double p1 = 1.282e9/(cs*cs*cs)+1.522;
		double p2 = -8.494e7/(cs*cs*cs)+0.496;
		sf = cs*p1/(vs*zoom_)+p2;
		sf = fluo::Clamp(sf, 0.6, 2.0);
		return sf;
	}

	//calculate the filter sizes
	//calculate filter sizes according to viewport and texture size
	double VolumeRenderer::CalcFilterSize(int type,
		double w, double h, double tex_w, double tex_h,
		double sf)
	{
		//clamped texture size
		double cs;
		//viewport size
		double vs;
		//filter size
		double size = 0.0;
		if (w > h)
		{
			cs = fluo::Clamp(tex_h, 500.0, 1200.0);
			vs = h;
		}
		else
		{
			cs = fluo::Clamp(tex_w, 500.0, 1200.0);
			vs = w;
		}

		switch (type)
		{
		case 1:	//min filter
			{
				double p = 0.29633+(-2.18448e-4)*cs;
				size = (p*zoom_+0.24512)*sf;
				size = fluo::Clamp(size, 0.0, 2.0);
			}
			break;
		case 2:	//max filter
			{
				double p1 = 0.26051+(-1.90542e-4)*cs;
				double p2 = -0.29188+(2.45276e-4)*cs;
				p2 = std::min(p2, 0.0);
				size = (p1*zoom_+p2)*sf;
				size = fluo::Clamp(size, 0.0, 2.0);
			}
			break;
		case 3:	//sharpening filter
			{
				//double p = 0.012221;
				//size = p*zoom_;
				//size = Clamp(size, 0.0, 0.25);
				double sf11 = std::sqrt(tex_w*tex_w + tex_h*tex_h)/vs;
				size = zoom_ / sf11 / 10.0;
				size = size<1.0?0.0:size;
				size = fluo::Clamp(size, 0.0, 0.3);
			}
			break;
		case 4:	//blur filter
			{
				double sf11 = std::sqrt(tex_w*tex_w + tex_h*tex_h)/vs;
				size = zoom_ / sf11 / 2.0;
				size = size<1.0?0.5:size;
				size = fluo::Clamp(size, 0.1, 1.0);
			}
		}

		//adjusting for screen size
		//double af = vs/800.0;

		return size;
	}

	//darw
	void VolumeRenderer::eval_ml_mode()
	{
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
			if (tex_->nmask() == -1)
			{
				mask_ = false;
				ml_mode_ = 0;
			}
			else
				mask_ = true;
			label_ = false;
			break;
		case 2:
			if (tex_->nmask() == -1)
			{
				mask_ = false;
				ml_mode_ = 0;
			}
			else
				mask_ = true;
			label_ = false;
			break;
		case 3:
			if (tex_->nlabel() == -1)
			{
				label_ = false;
				ml_mode_ = 0;
			}
			else
				label_ = true;
			mask_ = false;
			break;
		case 4:
			if (tex_->nlabel() == -1)
			{
				if (tex_->nmask()>-1)
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

	void VolumeRenderer::draw(bool draw_wireframe_p, bool adaptive,
		bool interactive_mode_p, bool orthographic_p, int mode)
	{
		draw_volume(adaptive, interactive_mode_p, orthographic_p, mode);
		if(draw_wireframe_p)
			draw_wireframe(orthographic_p);
	}

	void VolumeRenderer::draw_volume(bool adaptive,
		bool interactive_mode_p,
		bool orthographic_p,
		int mode)
	{
		if (!tex_)
			return;

		fluo::Ray view_ray = compute_view();
		fluo::Ray snapview = compute_snapview(0.4);

		vector<TextureBrick*> *bricks = 0;
		tex_->set_matrices(m_mv_mat2, m_proj_mat);
		if (glbin_settings.m_mem_swap && interactive_)
			bricks = tex_->get_closest_bricks(
			quota_center_, quota_bricks_chan_, true,
			view_ray, orthographic_p);
		else
			bricks = tex_->get_sorted_bricks(view_ray, orthographic_p);
		if (!bricks || bricks->size() == 0)
			return;

		set_adaptive(adaptive);
		set_interactive_mode(interactive_mode_p);

		// Set sampling rate based on interaction
		double rate = imode_ && adaptive_ ? irate_ : sampling_rate_;
		fluo::Vector diag = tex_->bbox()->diagonal();
		fluo::Vector cell_diag(
			diag.x() / tex_->nx(),
			diag.y() / tex_->ny(),
			diag.z() / tex_->nz());
		double dt;
		if (rate > 0.0)
		{
			dt = cell_diag.length() / compute_rate_scale(snapview.direction()) / rate;
			num_slices_ = std::round(diag.length()/dt);
		}
		else
		{
			dt = 0.0;
			num_slices_ = 0;
		}

		vector<float> vertex;
		vector<uint32_t> index;
		vector<uint32_t> size;
		vertex.reserve(num_slices_*12);
		index.reserve(num_slices_*6);
		size.reserve(num_slices_*6);

		//--------------------------------------------------------------------------

		int cm_mode = mode == 4 ? 0 : colormap_mode_;
		bool use_fog = m_use_fog && cm_mode !=2;

		// set up blending
		glEnable(GL_BLEND);
		switch(mode_)
		{
		case MODE_OVER:
			glBlendEquation(GL_FUNC_ADD);
			if (glbin_settings.m_update_order == 0)
				glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			else if (glbin_settings.m_update_order == 1)
				glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE);
			break;
		case MODE_MIP:
			glBlendEquation(GL_MAX);
			glBlendFunc(GL_ONE, GL_ONE);
			break;
		default:
			break;
		}

		int w = vp_[2];
		int h = vp_[3];
		int w2 = w;
		int h2 = h;
		double sf;
		std::string bbufname;
		if (imode_ && adaptive_)
		{
			sf = fluo::Clamp(double(1.0 / zoom_data_), 0.1, 1.0);
			bbufname = "blend_int";
		}
		else if (noise_red_)
		{
			sf = CalcScaleFactor(w, h, tex_->nx(), tex_->ny());
			bbufname = "blend_nr";
		}
		else
		{
			sf = fluo::Clamp(double(1.0 / zoom_data_), 0.5, 2.0);
			bbufname = "blend_hi";
		}
		if (fabs(sf - sfactor_) > 0.05)
			sfactor_ = sf;
		else if (sf == 1.0 && sfactor_ != 1.0)
			sfactor_ = sf;
		w2 = int(w*sfactor_ + 0.5);
		h2 = int(h*sfactor_ + 0.5);

		Framebuffer* blend_buffer = 0;
		if(blend_num_bits_ > 8)
		{
			blend_buffer = framebuffer_manager_.framebuffer(
				FB_Render_RGBA, w2, h2, bbufname);
			if (!blend_buffer)
				return;
			blend_buffer->bind();
			blend_buffer->protect();

			glClearColor(clear_color_[0], clear_color_[1], clear_color_[2], clear_color_[3]);
			glClear(GL_COLOR_BUFFER_BIT);

			glViewport(vp_[0], vp_[1], w2, h2);
		}

		//disable depth test
		glDisable(GL_DEPTH_TEST);

		eval_ml_mode();

		//--------------------------------------------------------------------------
		// Set up shaders
		ShaderProgram* shader = 0;
		//create/bind
		bool grad = gm_thresh_ > 0.0 ||
			(cm_mode &&
			colormap_proj_>3);
		shader = vol_shader_factory_.shader(
			false, tex_->nc(),
			shading_, use_fog,
			depth_peel_, true,
			grad, ml_mode_, mode_ == TextureRenderer::MODE_MIP,
			cm_mode, colormap_, colormap_proj_,
			solid_, 1);
		if (shader)
		{
			if (!shader->valid())
				shader->create();
			shader->bind();
		}

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
		tex_->get_spacings(spcx, spcy, spcz);
		shader->setLocalParam(5,
			spcx==0.0?1.0:spcx,
			spcy==0.0?1.0:spcy,
			spcz==0.0?1.0:spcz, shuffle_);

		//transfer function
		shader->setLocalParam(2, inv_?-scalar_scale_:scalar_scale_, gm_scale_, lo_thresh_, hi_thresh_);
		shader->setLocalParam(3, 1.0/gamma3d_, gm_thresh_, offset_, sw_);
		if (mode_==TextureRenderer::MODE_MIP &&
			colormap_proj_)
			shader->setLocalParam(6, colormap_low_value_, colormap_hi_value_,
				colormap_hi_value_ - colormap_low_value_, colormap_inv_);
		else
		{
			switch (cm_mode)
			{
			case 0://normal
				if (mask_ && !label_)
					shader->setLocalParam(6, mask_color_.r(), mask_color_.g(), mask_color_.b(), mask_thresh_);
				else
					shader->setLocalParam(6, color_.r(), color_.g(), color_.b(), 0.0);
				break;
			case 1://colormap
				shader->setLocalParam(6, colormap_low_value_, colormap_hi_value_,
					colormap_hi_value_-colormap_low_value_, colormap_inv_);
				break;
			case 2://depth map
				shader->setLocalParam(6, color_.r(), color_.g(), color_.b(), 0.0);
				break;
			}
		}
		//color
		shader->setLocalParam(9, color_.r(), color_.g(), color_.b(), alpha_power_);

		//setup depth peeling
		if (depth_peel_ || cm_mode == 2)
			shader->setLocalParam(7, 1.0/double(w2), 1.0/double(h2), 0.0, 0.0);

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
		fluo::Transform *tform = tex_->transform();
		double mvmat[16];
		tform->get_trans(mvmat);
		m_mv_mat2 = glm::mat4(
			mvmat[0], mvmat[4], mvmat[8], mvmat[12],
			mvmat[1], mvmat[5], mvmat[9], mvmat[13],
			mvmat[2], mvmat[6], mvmat[10], mvmat[14],
			mvmat[3], mvmat[7], mvmat[11], mvmat[15]);
		m_mv_mat2 = m_mv_mat * m_mv_mat2;
		shader->setLocalParamMatrix(0, glm::value_ptr(m_proj_mat));
		shader->setLocalParamMatrix(1, glm::value_ptr(m_mv_mat2));
		//shader->setLocalParamMatrix(5, glm::value_ptr(m_tex_mat));
		
		if (cur_chan_brick_num_ == 0) rearrangeLoadedBrkVec();

		num_slices_ = 0;
		bool multibricks = bricks->size() > 1;
		bool drawn_once = false;
		for (unsigned int i=0; i < bricks->size(); i++)
		{
			//comment off when debug_ds
			if (glbin_settings.m_mem_swap && drawn_once)
			{
				unsigned long long rn_time = GET_TICK_COUNT();
				if (rn_time - st_time_ > get_up_time())
					break;
			}

			TextureBrick* b = (*bricks)[i];
			if (tex_->isBrxml() && !b->isLoaded())
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
				b->get_priority()>0) //nothing to draw
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

			if ((cm_mode ==1 ||
				mode_==TextureRenderer::MODE_MIP) &&
				colormap_proj_)
			{
				fluo::BBox bbox = b->dbox();
				float matrix[16];
				matrix[0] = float(bbox.Max().x()-bbox.Min().x());
				matrix[1] = 0.0f;
				matrix[2] = 0.0f;
				matrix[3] = 0.0f;
				matrix[4] = 0.0f;
				matrix[5] = float(bbox.Max().y()-bbox.Min().y());
				matrix[6] = 0.0f;
				matrix[7] = 0.0f;
				matrix[8] = 0.0f;
				matrix[9] = 0.0f;
				matrix[10] = float(bbox.Max().z()-bbox.Min().z());
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

			num_slices_ += vertex.size()/12;

			if (vertex.size() == 0)
			{
				if (glbin_settings.m_mem_swap &&
					start_update_loop_ &&
					!done_update_loop_)
				{
					if (!(*bricks)[i]->drawn(mode))
					{
						(*bricks)[i]->set_drawn(mode, true);
						cur_brick_num_++;
						cur_chan_brick_num_++;
					}
				}
			}
			else
			{
				GLint filter;
				if (interpolate_)
					filter = GL_LINEAR;
				else
					filter = GL_NEAREST;

				if (!load_brick(b, filter, compression_, 0, mode))
					continue;
				if (mask_)
					load_brick_mask(b, filter);
				if (label_)
					load_brick_label(b);
				shader->setLocalParam(4, 1.0 / b->nx(), 1.0 / b->ny(), 1.0 / b->nz(),
					mode_ == MODE_OVER ? 1.0 / rate : 1.0);

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
			}

			if (glbin_settings.m_mem_swap)
				finished_bricks_++;
			drawn_once = true;
		}

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

		//DBGPRINT(L"done_current_chan_:%d\tclear_chan_buffer_:%d\tsave_final_buffer_:%d\tdone_loop_[%d]:%d\tcur_chan_brick_num_:%d\tcur_brick_num_:%d\tfinished_bricks_:%d\tbrick_size:%d\ttotal_brick_num_:%d\n",
		//	done_current_chan_, clear_chan_buffer_, save_final_buffer_, mode, done_loop_[mode], cur_chan_brick_num_, cur_brick_num_, finished_bricks_, (*bricks).size(), total_brick_num_);

		//release depth texture for rendering shadows
		if (cm_mode == 2)
			release_texture(4, GL_TEXTURE_2D);

		// Release shader.
		if (shader && shader->valid())
			shader->release();

		//Release 3d texture
		release_texture(0, GL_TEXTURE_3D);
		if (mask_)
			release_texture((*bricks)[0]->nmask(), GL_TEXTURE_3D);
		if (label_)
			release_texture((*bricks)[0]->nlabel(), GL_TEXTURE_3D);

		//enable
		glEnable(GL_DEPTH_TEST);

		//output result
		if (blend_num_bits_ > 8)
		{
			//states
			GLboolean depth_test = glIsEnabled(GL_DEPTH_TEST);
			GLboolean cull_face = glIsEnabled(GL_CULL_FACE);
			glDisable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);

			ShaderProgram* img_shader = 0;

			Framebuffer* filter_buffer = 0;
			if (noise_red_ && cm_mode !=2)
			{
				//FILTERING/////////////////////////////////////////////////////////////////
				filter_buffer = framebuffer_manager_.framebuffer(
					FB_Render_RGBA, w2, h2);
				filter_buffer->bind();

				glClear(GL_COLOR_BUFFER_BIT);

				blend_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
				img_shader = 
					img_shader_factory_.shader(IMG_SHDR_FILTER_BLUR);
				if (img_shader)
				{
					if (!img_shader->valid())
					{
						img_shader->create();
					}
					img_shader->bind();
				}
				filter_size_min_ = CalcFilterSize(4, w, h, tex_->nx(), tex_->ny(), sfactor_);
				img_shader->setLocalParam(0, filter_size_min_/w2, filter_size_min_/h2, 1.0/w2, 1.0/h2);

				draw_view_quad();

				if (img_shader && img_shader->valid())
					img_shader->release();
			}

			//go back to normal
			glBindFramebuffer(GL_FRAMEBUFFER, cur_framebuffer_); 

			glViewport(vp_[0], vp_[1], vp_[2], vp_[3]);

			if (noise_red_ && cm_mode != 2)
				filter_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
			else
				blend_buffer->bind_texture(GL_COLOR_ATTACHMENT0);

			if (noise_red_ && cm_mode !=2)
				img_shader = 
					img_shader_factory_.shader(IMG_SHDR_FILTER_SHARPEN);
			else
				img_shader = 
				img_shader_factory_.shader(IMG_SHADER_TEXTURE_LOOKUP);

			if (img_shader)
			{
				if (!img_shader->valid())
					img_shader->create();
				img_shader->bind();
			}

			if (noise_red_ && cm_mode !=2)
			{
				filter_size_shp_ = CalcFilterSize(3, w, h, tex_->nx(), tex_->ny(), sfactor_);
				img_shader->setLocalParam(0, filter_size_shp_/w, filter_size_shp_/h, 0.0, 0.0);
			}

			draw_view_quad();

			if (img_shader && img_shader->valid())
				img_shader->release();

			if (depth_test) glEnable(GL_DEPTH_TEST);
			if (cull_face) glEnable(GL_CULL_FACE);

			if (blend_buffer)
				blend_buffer->unprotect();
		}

		// Reset the blend functions after MIP
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_BLEND);

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void VolumeRenderer::draw_wireframe(bool orthographic_p)
	{
		fluo::Ray view_ray = compute_view();
		fluo::Ray snapview = compute_snapview(0.4);

		glEnable(GL_DEPTH_TEST);
		vector<TextureBrick*> *bricks = tex_->get_sorted_bricks(view_ray, orthographic_p);

		double rate = imode_ && adaptive_ ? irate_ : sampling_rate_;
		fluo::Vector diag = tex_->bbox()->diagonal();
		fluo::Vector cell_diag(diag.x()/tex_->nx(),
			diag.y()/tex_->ny(),
			diag.z()/tex_->nz());
		double dt;
		int num_slices;
		if (rate > 0.0)
		{
			dt = cell_diag.length() / compute_rate_scale(snapview.direction()) / rate;
			num_slices = std::round(diag.length()/dt);
		}
		else
		{
			dt = 0.0;
			num_slices = 0;
		}

		vector<float> vertex;
		vector<uint32_t> index;
		vector<uint32_t> size;
		vertex.reserve(num_slices * 12);
		index.reserve(num_slices * 6);
		size.reserve(num_slices * 6);

		// Set up shaders
		ShaderProgram* shader = 0;
		//create/bind
		shader = vol_shader_factory_.shader(
			true, 0,
			false, false,
			0, false,
			false, 0, false,
			0, 0, 0,
			false, 1);
		if (shader)
		{
			if (!shader->valid())
				shader->create();
			shader->bind();
		}

		////////////////////////////////////////////////////////
		// render bricks
		// Set up transform
		fluo::Transform *tform = tex_->transform();
		double mvmat[16];
		tform->get_trans(mvmat);
		m_mv_mat2 = glm::mat4(
			mvmat[0], mvmat[4], mvmat[8], mvmat[12],
			mvmat[1], mvmat[5], mvmat[9], mvmat[13],
			mvmat[2], mvmat[6], mvmat[10], mvmat[14],
			mvmat[3], mvmat[7], mvmat[11], mvmat[15]);
		m_mv_mat2 = m_mv_mat * m_mv_mat2;
		shader->setLocalParamMatrix(0, glm::value_ptr(m_proj_mat));
		shader->setLocalParamMatrix(1, glm::value_ptr(m_mv_mat2));
		shader->setLocalParamMatrix(5, glm::value_ptr(m_tex_mat));
		shader->setLocalParam(0, color_.r(), color_.g(), color_.b(), 1.0);

		if (bricks)
		{
			bool multibricks = bricks->size() > 1;
			for (unsigned int i=0; i<bricks->size(); i++)
			{
				TextureBrick* b = (*bricks)[i];
				if (!test_against_view(b->bbox(), !orthographic_p)) continue;

				vertex.clear();
				index.clear();
				size.clear();

				// Scale out dt such that the slices are artificially further apart.
				b->compute_polygons(snapview, dt * 1, vertex, index, size, multibricks);
				draw_polygons_wireframe(vertex, index, size);
			}
		}

		// Release shader.
		if (shader && shader->valid())
			shader->release();
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
		if (estimate && type==0)
			est_thresh_ = 0.0;
		bool use_2d = tex_2d_weight1_&&
			tex_2d_weight2_;

		vector<TextureBrick*> *bricks = tex_->get_bricks();
		if (!bricks || bricks->size() == 0)
			return;

		//mask frame buffer object
		Framebuffer* fbo_mask =
			framebuffer_manager_.framebuffer(FB_3D_Int, 0, 0);
		if (fbo_mask)
			fbo_mask->bind();

		//--------------------------------------------------------------------------
		// Set up shaders
		//seg shader
		ShaderProgram* seg_shader = 0;
		double mvec_len = mvec_.length();

		switch (type)
		{
		case 0://initialize
			seg_shader = seg_shader_factory_.shader(
				SEG_SHDR_INITIALIZE, paint_mode, hr_mode,
				use_2d, true, depth_peel_,
				true, false);
			break;
		case 1://diffusion-based growing
			seg_shader = seg_shader_factory_.shader(
				SEG_SHDR_DB_GROW, paint_mode, hr_mode,
				use_2d, true, depth_peel_,
				true, mvec_len>0.5);
			break;
		}

		if (seg_shader)
		{
			if (!seg_shader->valid())
				seg_shader->create();
			seg_shader->bind();
		}

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
		tex_->get_spacings(spcx, spcy, spcz);
		seg_shader->setLocalParam(5, spcx, spcy, spcz, shuffle_);

		//transfer function
		seg_shader->setLocalParam(2, inv_?-scalar_scale_:scalar_scale_, gm_scale_, lo_thresh_, hi_thresh_);
		seg_shader->setLocalParam(3, 1.0/gamma3d_, gm_thresh_, offset_, sw_);
		seg_shader->setLocalParam(6, mp_[0], vp_[3]-mp_[1], vp_[2], vp_[3]);
		seg_shader->setLocalParam(9, mvec_.x(), mvec_.y(), mvec_.z(), 0);

		//setup depth peeling
		//if (depth_peel_)
		//	seg_shader->setLocalParam(7, 1.0/double(w2), 1.0/double(h2), 0.0, 0.0);

		//thresh1
		seg_shader->setLocalParam(7, ini_thresh, gm_falloff, scl_falloff, scl_translate);
		//w2d
		seg_shader->setLocalParam(8, w2d, bins, zoom_, zoom_data_);

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
		// Set up transform
		fluo::Transform *tform = tex_->transform();
		double mvmat[16];
		tform->get_trans(mvmat);
		m_mv_mat2 = glm::mat4(
			mvmat[0], mvmat[4], mvmat[8], mvmat[12],
			mvmat[1], mvmat[5], mvmat[9], mvmat[13],
			mvmat[2], mvmat[6], mvmat[10], mvmat[14],
			mvmat[3], mvmat[7], mvmat[11], mvmat[15]);
		m_mv_mat2 = m_mv_mat * m_mv_mat2;
		seg_shader->setLocalParamMatrix(0, glm::value_ptr(m_mv_mat2));
		seg_shader->setLocalParamMatrix(1, glm::value_ptr(m_proj_mat));
		if (hr_mode > 0)
		{
			glm::mat4 mv_inv = glm::inverse(m_mv_mat2);
			seg_shader->setLocalParamMatrix(3, glm::value_ptr(mv_inv));
		}

		glDisable(GL_DEPTH_TEST);

		//bind 2d mask texture
		bind_2d_mask();
		//bind 2d weight map
		if (use_2d) bind_2d_weight();

		int i;
		float matrix[16];
		unsigned int num = bricks->size();
		for (i = ((order == 2) ? (num - 1) : 0);
			(order == 2) ? (i >= 0) : (i < num);
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
			matrix[0] = float(bbox.Max().x()-bbox.Min().x());
			matrix[1] = 0.0f;
			matrix[2] = 0.0f;
			matrix[3] = 0.0f;
			matrix[4] = 0.0f;
			matrix[5] = float(bbox.Max().y()-bbox.Min().y());
			matrix[6] = 0.0f;
			matrix[7] = 0.0f;
			matrix[8] = 0.0f;
			matrix[9] = 0.0f;
			matrix[10] = float(bbox.Max().z()-bbox.Min().z());
			matrix[11] = 0.0f;
			matrix[12] = float(bbox.Min().x());
			matrix[13] = float(bbox.Min().y());
			matrix[14] = float(bbox.Min().z());
			matrix[15] = 1.0f;
			seg_shader->setLocalParamMatrix(2, matrix);

			//set viewport size the same as the texture
			glViewport(0, 0, b->nx(), b->ny());

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
			seg_shader->setLocalParam(4, 1.0/b->nx(), 1.0/b->ny(), 1.0/b->nz(),
				mode_==MODE_OVER?1.0/sampling_rate_:1.0);

			//draw each slice
			for (int z=0; z<b->nz(); z++)
			{
				fbo_mask->attach_texture(GL_COLOR_ATTACHMENT0, tex_id, z);
				draw_view_quad(double(z+0.5) / double(b->nz()));
			}

			if (num > 1 && type == 1 && order)
				copy_mask_border(mask_id, b, order);

			//test cl
			if (estimate && type == 0)
			{
				double temp = calc_hist_3d(vd_id, mask_id, b->nx(), b->ny(), b->nz());
				est_thresh_ = std::max(est_thresh_, temp);
			}

		}

		glViewport(vp_[0], vp_[1], vp_[2], vp_[3]);

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

		//unbind framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, cur_framebuffer_);

		//release seg shader
		if (seg_shader && seg_shader->valid())
			seg_shader->release();

		// Release texture
		release_texture(0, GL_TEXTURE_3D);

		// Reset the blend functions after MIP
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_BLEND);

		//enable depth test
		glEnable(GL_DEPTH_TEST);
	}

	//for multibrick, copy border to continue diffusion
	void VolumeRenderer::copy_mask_border(GLint btex, TextureBrick* b, int order)
	{
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
			nid = tex_->negxid(bid);
			if (nid != bid)
			{
				nb = tex_->get_brick(nid);
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
			nid = tex_->negyid(bid);
			if (nid != bid)
			{
				nb = tex_->get_brick(nid);
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
			nid = tex_->negzid(bid);
			if (nid != bid)
			{
				nb = tex_->get_brick(nid);
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
			nid = tex_->posxid(bid);
			if (nid != bid)
			{
				nb = tex_->get_brick(nid);
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
			nid = tex_->posyid(bid);
			if (nid != bid)
			{
				nb = tex_->get_brick(nid);
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
			nid = tex_->poszid(bid);
			if (nid != bid)
			{
				nb = tex_->get_brick(nid);
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

	double VolumeRenderer::calc_hist_3d(GLuint data_id, GLuint mask_id,
		size_t brick_x, size_t brick_y, size_t brick_z)
	{
		double result = 0.0;
		int kernel_index = -1;
		KernelProgram* kernel = vol_kernel_factory_.kernel(KERNEL_HIST_3D);
		if (kernel)
		{
			kernel_index = kernel->createKernel("hist_3d");
			kernel->setKernelArgBegin(kernel_index);
			kernel->setKernelArgTex3D(CL_MEM_READ_ONLY, data_id);
			kernel->setKernelArgTex3D(CL_MEM_READ_ONLY, mask_id);
			unsigned int hist_size = 64;
			if (tex_ && tex_->get_nrrd(0))
			{
				if (tex_->get_nrrd(0)->type == nrrdTypeUChar)
					hist_size = 64;
				else if (tex_->get_nrrd(0)->type == nrrdTypeUShort)
					hist_size = 1024;
			}
			float* hist = new float[hist_size]();
			kernel->setKernelArgBuf(CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR, hist_size*sizeof(float), hist);
			kernel->setKernelArgConst(sizeof(unsigned int), (void*)(&hist_size));
			size_t global_size[3] = {brick_x, brick_y, brick_z};
			size_t local_size[3] = {1, 1, 1};
			kernel->executeKernel(kernel_index, 3, global_size, local_size);
			kernel->readBuffer(hist_size * sizeof(float), (void*)hist, (void*)hist);
			//release buffer
			kernel->releaseMemObject(kernel_index, 0, 0, data_id);
			kernel->releaseMemObject(kernel_index, 1, 0, mask_id);
			kernel->releaseMemObject(kernel_index, 2, hist_size * sizeof(float), 0);
			//analyze hist
			int i;
			float sum = 0;
			for (i=0; i<hist_size; ++i)
				sum += hist[i];
			for (i=hist_size-1; i>0; --i)
			{
				if (hist[i] > sum/hist_size && hist[i] > hist[i-1])
				{
					result = double(i)/double(hist_size);
					break;
				}
			}
			delete[] hist;
		}
		return result;
	}

	//calculation
	void VolumeRenderer::calculate(int type, VolumeRenderer *vr_a, VolumeRenderer *vr_b)
	{
		//sync sorting
		fluo::Ray view_ray(fluo::Point(0.802,0.267,0.534), fluo::Vector(0.802,0.267,0.534));
		tex_->set_sort_bricks();
		vector<TextureBrick*> *bricks = tex_->get_sorted_bricks(view_ray);
		if (!bricks || bricks->size() == 0)
			return;
		vector<TextureBrick*> *bricks_a = 0;
		vector<TextureBrick*> *bricks_b = 0;

		bricks_a = vr_a->tex_->get_bricks();
		if (vr_a)
		{
			vr_a->tex_->set_sort_bricks();
			bricks_a = vr_a->tex_->get_sorted_bricks(view_ray);
		}
		if (vr_b)
		{
			vr_b->tex_->set_sort_bricks();
			bricks_b = vr_b->tex_->get_sorted_bricks(view_ray);
		}

		glActiveTexture(GL_TEXTURE0);
		//mask frame buffer object
		Framebuffer* fbo_calc =
			framebuffer_manager_.framebuffer(FB_3D_Int, 0, 0);
		if (fbo_calc)
			fbo_calc->bind();

		//--------------------------------------------------------------------------
		// Set up shaders
		//calculate shader
		ShaderProgram* cal_shader = cal_shader_factory_.shader(type);

		if (cal_shader)
		{
			if (!cal_shader->valid())
				cal_shader->create();
			cal_shader->bind();
		}

		//set uniforms
		if (type == 1 ||
			type == 2 ||
			type == 3 ||
			type == 8)
			cal_shader->setLocalParam(0, vr_a ? vr_a->get_scalar_scale() : 1.0,
				vr_b ? vr_b->get_scalar_scale() : 1.0,
				(vr_a&&vr_a->tex_&&vr_a->tex_->nmask() != -1) ? 1.0 : 0.0,
				(vr_b&&vr_b->tex_&&vr_b->tex_->nmask() != -1) ? 1.0 : 0.0);
		else if (type == 4 ||
			type == 5 ||
			type == 6 ||
			type == 7)
			cal_shader->setLocalParam(0, 1.0, 1.0, 0.0, 0.0);
		else
			cal_shader->setLocalParam(0, vr_a?vr_a->get_scalar_scale():1.0,
				vr_b?vr_b->get_scalar_scale():1.0,
				(vr_a&&vr_a->get_inversion())?-1.0:0.0,
				(vr_b&&vr_b->get_inversion())?-1.0:0.0);
		if (vr_a && (type==6 || type==7))
		{
			cal_shader->setLocalParam(2, inv_?-scalar_scale_:scalar_scale_, gm_scale_, lo_thresh_, hi_thresh_);
			cal_shader->setLocalParam(3, 1.0/gamma3d_, gm_thresh_, offset_, sw_);
		}

		for (unsigned int i=0; i < bricks->size(); i++)
		{
			TextureBrick* b = (*bricks)[i];

			//set viewport size the same as the texture
			glViewport(0, 0, b->nx(), b->ny());

			//load the texture
			GLuint tex_id = load_brick(b, GL_NEAREST);
			if (bricks_a) vr_a->load_brick((*bricks_a)[i], GL_NEAREST, false, 1);
			if (bricks_b) vr_b->load_brick((*bricks_b)[i], GL_NEAREST, false, 2);
			if ((type==5 || type==6 ||type==7) && bricks_a) vr_a->load_brick_mask((*bricks_a)[i]);
			if (type==8)
			{
				if (bricks_a)
					vr_a->load_brick_mask((*bricks_a)[i], GL_NEAREST, false, 3);
				if (bricks_b)
					vr_b->load_brick_mask((*bricks_b)[i], GL_NEAREST, false, 4);
			}
			//draw each slice
			for (int z=0; z<b->nz(); z++)
			{
				fbo_calc->attach_texture(GL_COLOR_ATTACHMENT0, tex_id, z);
				draw_view_quad(double(z+0.5) / double(b->nz()));
			}
		}

		//release 3d mask
		release_texture((*bricks)[0]->nmask(), GL_TEXTURE_3D);
		release_texture(4, GL_TEXTURE_3D);
		//release 3d label
		release_texture((*bricks)[0]->nlabel(), GL_TEXTURE_3D);

		//unbind framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, cur_framebuffer_);

		//release seg shader
		if (cal_shader && cal_shader->valid())
			cal_shader->release();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_3D, 0);
		//--------------------------------------------------------------------------

		// Reset the blend functions after MIP
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

		glDisable(GL_BLEND);
	}

	//return the data volume
	void VolumeRenderer::return_volume()
	{
		if (!tex_)
			return;
		vector<TextureBrick*> *bricks = tex_->get_bricks();
		if (!bricks || bricks->size() == 0)
			return;

		int c = 0;
		for (unsigned int i=0; i<bricks->size(); i++)
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
		if (!tex_)
			return;
		vector<TextureBrick*> *bricks = tex_->get_bricks();
		if (!bricks || bricks->size() == 0)
			return;

		int c = tex_->nmask();
		if (c<0 || c>=TEXTURE_MAX_COMPONENTS)
			return;

		int i;
		size_t num = bricks->size();
		for (i=((order==2)?(num-1):0);
			(order==2)?(i>=0):(i<num);
			i+=((order==2)?-1:1))
		{
			TextureBrick* b = (*bricks)[i];
			if (!b || !b->is_mask_valid())
				continue;

			load_brick_mask(b);
			glActiveTexture(GL_TEXTURE0+c);

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
		if (!tex_)
			return;
		vector<TextureBrick*> *bricks = tex_->get_bricks_id();
		if (!bricks || bricks->size() == 0)
			return;

		int c = tex_->nlabel();
		if (c<0 || c>=TEXTURE_MAX_COMPONENTS)
			return;

		for (unsigned int i=0; i<bricks->size(); i++)
		{
			TextureBrick *b = (*bricks)[i];
			load_brick_label(b);
			glActiveTexture(GL_TEXTURE0+c);

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

} // namespace flvr
