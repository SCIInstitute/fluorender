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
#include <TextureRenderer.h>
#include <Global.h>
#include <GlobalStates.h>
#include <MainSettings.h>
#include <TextureBrick.h>
#include <Texture.h>
#include <ShaderProgram.h>
#include <VertexArray.h>
#include <Color.h>
#include <Utils.h>
#include <Ray.h>
#include <DataManager.h>
#include <VolCache4D.h>
#include <algorithm>
#include <glm/gtc/type_ptr.hpp>
#include <compatibility.h>
#include <thread>

namespace flvr
{
	int TextureRenderer::active_view_ = -1;
	double TextureRenderer::mainmem_buf_size_ = 4000.0;
	double TextureRenderer::available_mainmem_buf_size_ = 0.0;
	std::vector<TexParam> TextureRenderer::tex_pool_;
	bool TextureRenderer::start_update_loop_ = false;
	bool TextureRenderer::done_update_loop_ = true;
	bool TextureRenderer::done_current_chan_ = true;
	int TextureRenderer::total_brick_num_ = 0;
	int TextureRenderer::cur_brick_num_ = 0;
	int TextureRenderer::cur_chan_brick_num_ = 0;
	bool TextureRenderer::clear_chan_buffer_ = true;
	bool TextureRenderer::save_final_buffer_ = true;
#ifdef _DEBUG
	bool TextureRenderer::maximize_uptime_ = false;//change to max time
#else
	bool TextureRenderer::maximize_uptime_ = false;//change to max time
#endif
	unsigned long TextureRenderer::st_time_ = 0;
	unsigned long TextureRenderer::cor_up_time_ = 100;
	unsigned long TextureRenderer::consumed_time_ = 0;
	bool TextureRenderer::interactive_ = false;
	int TextureRenderer::finished_bricks_ = 0;
	BrickQueue TextureRenderer::brick_queue_(5);
	int TextureRenderer::quota_bricks_ = 0;
	fluo::Point TextureRenderer::quota_center_;
	bool TextureRenderer::load_on_main_thread_ = false;
	std::vector<TextureRenderer::LoadedBrick> TextureRenderer::loadedbrks;
	int TextureRenderer::del_id = 0;
#ifdef _DARWIN
	CGLContextObj TextureRenderer::gl_context_ = 0;
#endif

	TextureRenderer::TextureRenderer()
		:
		mode_(RENDER_MODE_NONE),
		sample_rate_(1.0),
		num_slices_(0),
		imode_(false),
		cur_framebuffer_(0),
		tex_2d_mask_(0),
		tex_2d_weight1_(0),
		tex_2d_weight2_(0),
		tex_2d_dmap_(0),
		blend_num_bits_(32),
		va_slices_(0),
		va_wirefm_(0),
		cache_queue_(0),
		quota_bricks_chan_(0)
	{
		if (!ShaderProgram::init())
			return;
#ifdef _DARWIN
		CGLSetCurrentContext(TextureRenderer::gl_context_);
#endif
	}

	TextureRenderer::TextureRenderer(const TextureRenderer& copy)
		:
		tex_(copy.tex_),
		mode_(copy.mode_),
		sample_rate_(copy.sample_rate_),
		num_slices_(0),
		imode_(copy.imode_),
		tex_2d_mask_(0),
		tex_2d_weight1_(0),
		tex_2d_weight2_(0),
		tex_2d_dmap_(0),
		blend_num_bits_(copy.blend_num_bits_),
		va_slices_(0),
		va_wirefm_(0),
		cache_queue_(0),
		quota_bricks_chan_(0)
	{
		if (!ShaderProgram::init())
			return;
	}

	TextureRenderer::~TextureRenderer()
	{
		if (!ShaderProgram::init())
			return;

		clear_brick_buf();
		clear_tex_current();

		if (va_slices_)
			delete va_slices_;
		if (va_wirefm_)
			delete va_wirefm_;
	}

	//set the texture for rendering
	void TextureRenderer::set_texture(const std::shared_ptr<Texture>& tex)
	{
		auto old_tex = tex_.lock();
		if (old_tex != tex)
		{
			// new texture, flag existing tex id's for deletion.
			clear_tex_current();
			if (tex)
				tex->clear_undos();
			tex_ = tex;
		}
	}

	void TextureRenderer::reset_texture()
	{
		tex_.reset();
	}

	//set blending bits. b>8 means 32bit blending
	void TextureRenderer::set_blend_num_bits(int b)
	{
		blend_num_bits_ = b;
	}

	// Pool is static, however it is cleared each time
	// when a texture is deleted
	void TextureRenderer::clear_tex_pool()
	{
		for (unsigned int i = 0; i < tex_pool_.size(); i++)
		{
			// delete tex object.
			if (glIsTexture(tex_pool_[i].id))
			{
				glDeleteTextures(1, (GLuint*)&tex_pool_[i].id);
				tex_pool_[i].id = 0;
			}
		}
		tex_pool_.clear();
		glbin_settings.m_available_mem = glbin_settings.m_mem_limit;
	}

	void TextureRenderer::clear_tex_current()
	{
		auto tex = tex_.lock();
		if (!tex)
			return;

		std::vector<TextureBrick*>* bricks = tex->get_bricks();
		TextureBrick* brick = 0;
		for (size_t i = tex_pool_.size(); i > 0; --i)
		{
			for (size_t j = 0; j < bricks->size(); ++j)
			{
				brick = (*bricks)[j];
				if (tex_pool_[i-1].brick == brick/* &&
					(tex_pool_[i].comp == 0 ||
					tex_pool_[i].comp == brick->nmask() ||
					tex_pool_[i].comp == brick->nlabel())*/)
				{
					glbin_settings.m_available_mem +=
						tex_pool_[i-1].nx *
						tex_pool_[i-1].ny *
						tex_pool_[i-1].nz *
						tex_pool_[i-1].nb / 1.04e6;
					glDeleteTextures(1, (GLuint*)&tex_pool_[i-1].id);
					tex_pool_.erase(tex_pool_.begin() + i - 1);
					break;
				}
			}
		}
	}

	void TextureRenderer::clear_tex_mask(bool skip)
	{
		auto tex = tex_.lock();
		if (!tex)
			return;

		std::vector<TextureBrick*>* bricks = tex->get_bricks();
		TextureBrick *brick = 0;
		TextureBrick *locbk = 0;
		for (size_t i = tex_pool_.size(); i > 0; --i)
		{
			brick = tex_pool_[i-1].brick;
			if (skip && !brick->is_mask_valid())
			{
				//brick->reset_skip_mask();
				continue;
			}
			for (size_t j = 0; j < bricks->size(); ++j)
			{
				locbk = (*bricks)[j];
				if (brick == locbk &&
					tex_pool_[i-1].comp == brick->nmask())
				{
					glbin_settings.m_available_mem +=
						tex_pool_[i-1].nx *
						tex_pool_[i-1].ny *
						tex_pool_[i-1].nz *
						tex_pool_[i-1].nb / 1.04e6;
					glDeleteTextures(1, (GLuint*)&tex_pool_[i-1].id);
					tex_pool_.erase(tex_pool_.begin() + i - 1);
					break;
				}
			}
		}
	}

	void TextureRenderer::clear_tex_label()
	{
		auto tex = tex_.lock();
		if (!tex)
			return;

		std::vector<TextureBrick*>* bricks = tex->get_bricks();
		TextureBrick *brick = 0;
		TextureBrick *locbk = 0;
		for (size_t i = tex_pool_.size(); i > 0; --i)
		{
			brick = tex_pool_[i-1].brick;
			if (!brick->is_mask_valid())
			{
				//brick->reset_skip_mask();
				continue;
			}
			for (size_t j = 0; j < bricks->size(); ++j)
			{
				locbk = (*bricks)[j];
				if (brick == locbk &&
					tex_pool_[i-1].comp == brick->nlabel())
				{
					glbin_settings.m_available_mem +=
						tex_pool_[i-1].nx *
						tex_pool_[i-1].ny *
						tex_pool_[i-1].nz *
						tex_pool_[i-1].nb / 1.04e6;
					glDeleteTextures(1, (GLuint*)&tex_pool_[i-1].id);
					tex_pool_.erase(tex_pool_.begin() + i - 1);
					break;
				}
			}
		}
	}

	//set the 2d texture mask for segmentation
	void TextureRenderer::set_2d_mask(GLuint id)
	{
		tex_2d_mask_ = id;
	}

	//set 2d weight map for segmentation
	void TextureRenderer::set_2d_weight(GLuint weight1, GLuint weight2)
	{
		tex_2d_weight1_ = weight1;
		tex_2d_weight2_ = weight2;
	}

	//set the 2d texture depth map for rendering shadows
	void TextureRenderer::set_2d_dmap(GLuint id)
	{
		tex_2d_dmap_ = id;
	}

	//timer
	unsigned long TextureRenderer::get_up_time()
	{
		if (maximize_uptime_)
			return -1;
		else
		{
			if (interactive_)
				return cor_up_time_;
			else
				return glbin_settings.m_up_time;
		}
	}

	//set corrected up time according to mouse speed
	void TextureRenderer::set_cor_up_time(int speed)
	{
		//cor_up_time_ = speed;
		if (speed < 5) speed = 5;
		if (speed > 20) speed = 20;
		cor_up_time_ = (unsigned long)(log10(100.0 / speed)* glbin_settings.m_up_time);
		cor_up_time_ = std::max(1, (int)cor_up_time_);
	}

	//number of bricks rendered before time is up
	void TextureRenderer::reset_finished_bricks()
	{
		//if (finished_bricks_ > 0 && consumed_time_ > 0)
		//{
		//	brick_queue_.Push(int(double(finished_bricks_)*double(up_time_) / double(consumed_time_)));
		//}
		finished_bricks_ = 0;
	}

	void TextureRenderer::push_quota_brick(int bricks)
	{
		brick_queue_.Push(bricks);
	}

	//estimate next brick number
	int TextureRenderer::get_est_bricks(int mode, int init)
	{
		double result = 0.0;
		if (mode == 0)
		{
			//mean
			double sum = 0.0;
			int count = 0;
			for (int i = 0; i < brick_queue_.GetLimit(); i++)
			{
				int quota = brick_queue_.Get(i);
				if (quota)
				{
					sum += quota;
					count++;
				}
			}
			result = (sum + init) / (count + 1);
		}
		else if (mode == 1)
		{
			//trend (weighted average)
			double sum = 0.0;
			double weights = 0.0;
			double w;
			for (int i = 0; i < brick_queue_.GetLimit(); i++)
			{
				w = (i + 1) * (i + 1);
				sum += brick_queue_.Get(i) * w;
				weights += w;
			}
			result = sum / weights;
		}
		else if (mode == 2)
		{
			//linear regression
			double sum_xy = 0.0;
			double sum_x = 0.0;
			double sum_y = 0.0;
			double sum_x2 = 0.0;
			double x, y;
			double n = brick_queue_.GetLimit();
			for (int i = 0; i < brick_queue_.GetLimit(); i++)
			{
				x = i;
				y = brick_queue_.Get(i);
				sum_xy += x * y;
				sum_x += x;
				sum_y += y;
				sum_x2 += x * x;
			}
			double beta = (sum_xy / n - sum_x*sum_y / n / n) / (sum_x2 / n - sum_x*sum_x / n / n);
			result = std::max(sum_y / n - beta*sum_x / n + beta*n, 1.0);
		}
		else if (mode == 3)
		{
			//most recently
			result = brick_queue_.GetLast();
		}
		else if (mode == 4)
		{
			//median
			int n0 = 0;
			double sum = 0.0;
			int n = brick_queue_.GetLimit();
			int *sorted_queue = new int[n]();
			for (int i = 0; i < n; i++)
			{
				sorted_queue[i] = brick_queue_.Get(i);
				if (sorted_queue[i] == 0)
					n0++;
				else
					sum += sorted_queue[i];
				for (int j = i; j > 0; j--)
				{
					if (sorted_queue[j] < sorted_queue[j - 1])
					{
						sorted_queue[j] = sorted_queue[j] + sorted_queue[j - 1];
						sorted_queue[j - 1] = sorted_queue[j] - sorted_queue[j - 1];
						sorted_queue[j] = sorted_queue[j] - sorted_queue[j - 1];
					}
					else
						break;
				}
			}
			if (n0 == 0)
				result = sorted_queue[n / 2];
			else if (n0 < n)
				result = sum / (n - n0);
			else
				result = 0.0;
			delete[]sorted_queue;
		}

		return int(result);
	}

	fluo::Ray TextureRenderer::compute_view()
	{
		auto tex = tex_.lock();
		if (!tex)
			return fluo::Ray();

		fluo::Transform *field_trans = tex->transform();
		double mvmat[16] =
		{ m_mv_mat[0][0], m_mv_mat[0][1], m_mv_mat[0][2], m_mv_mat[0][3],
		 m_mv_mat[1][0], m_mv_mat[1][1], m_mv_mat[1][2], m_mv_mat[1][3],
		 m_mv_mat[2][0], m_mv_mat[2][1], m_mv_mat[2][2], m_mv_mat[2][3],
		 m_mv_mat[3][0], m_mv_mat[3][1], m_mv_mat[3][2], m_mv_mat[3][3] };

		// index space view direction
		fluo::Vector v = field_trans->project(fluo::Vector(-mvmat[2], -mvmat[6], -mvmat[10]));
		v.safe_normalize();
		fluo::Transform mv;
		mv.set_trans(mvmat);
		fluo::Point p = field_trans->unproject(mv.unproject(fluo::Point(0, 0, 0)));
		return fluo::Ray(p, v);
	}

	fluo::Ray TextureRenderer::compute_snapview(double snap)
	{
		auto tex = tex_.lock();
		if (!tex)
			return fluo::Ray();

		fluo::Transform *field_trans = tex->transform();
		double mvmat[16] =
		{ m_mv_mat[0][0], m_mv_mat[0][1], m_mv_mat[0][2], m_mv_mat[0][3],
		 m_mv_mat[1][0], m_mv_mat[1][1], m_mv_mat[1][2], m_mv_mat[1][3],
		 m_mv_mat[2][0], m_mv_mat[2][1], m_mv_mat[2][2], m_mv_mat[2][3],
		 m_mv_mat[3][0], m_mv_mat[3][1], m_mv_mat[3][2], m_mv_mat[3][3] };

		//snap
		fluo::Vector vd;
		if (snap > 0.0 && snap < 0.5)
		{
			double vdx = -mvmat[2];
			double vdy = -mvmat[6];
			double vdz = -mvmat[10];
			double vdx_abs = fabs(vdx);
			double vdy_abs = fabs(vdy);
			double vdz_abs = fabs(vdz);
			//if (vdx_abs < snap) vdx = 0.0;
			//if (vdy_abs < snap) vdy = 0.0;
			//if (vdz_abs < snap) vdz = 0.0;
			//transition
			if (vdx_abs < snap - 0.1) vdx = 0.0;
			else if (vdx_abs < snap) vdx = (vdx_abs - snap + 0.1)*snap*10.0*vdx / vdx_abs;
			if (vdy_abs < snap - 0.1) vdy = 0.0;
			else if (vdy_abs < snap) vdy = (vdy_abs - snap + 0.1)*snap*10.0*vdy / vdy_abs;
			if (vdz_abs < snap - 0.1) vdz = 0.0;
			else if (vdz_abs < snap) vdz = (vdz_abs - snap + 0.1)*snap*10.0*vdz / vdz_abs;
			vd = fluo::Vector(vdx, vdy, vdz);
			vd.safe_normalize();
		}
		else
			vd = fluo::Vector(-mvmat[2], -mvmat[6], -mvmat[10]);

		// index space view direction
		fluo::Vector v = field_trans->project(vd);
		v.safe_normalize();
		fluo::Transform mv;
		mv.set_trans(mvmat);
		fluo::Point p = field_trans->unproject(mv.unproject(fluo::Point(0, 0, 0)));
		return fluo::Ray(p, v);
	}

	double TextureRenderer::compute_rate_scale(const fluo::Vector& v)
	{
		auto tex = tex_.lock();
		if (!tex)
			return 0;

		double basen = std::min(tex->nx(), std::min(tex->ny(), tex->nz()));
		fluo::Vector n(double(tex->nx() / basen),
			double(tex->ny() / basen),
			double(tex->nz() / basen));

		double e = 0.0001;
		double a, b, c;
		double result;
		double v_len2 = v.length2();
		if (fabs(v.x()) >= e && fabs(v.y()) >= e && fabs(v.z()) >= e)
		{
			a = v_len2 / v.x();
			b = v_len2 / v.y();
			c = v_len2 / v.z();
			result = n.x()*n.y()*n.z()*sqrt(a*a*b*b + b*b*c*c + c*c*a*a) /
				sqrt(n.x()*n.x()*a*a*n.y()*n.y()*b*b +
					n.x()*n.x()*a*a*n.z()*n.z()*c*c +
					n.y()*n.y()*b*b*n.z()*n.z()*c*c);
		}
		else if (fabs(v.x()) < e && fabs(v.y()) >= e && fabs(v.z()) >= e)
		{
			b = v_len2 / v.y();
			c = v_len2 / v.z();
			result = n.y()*n.z()*sqrt(b*b + c*c) /
				sqrt(n.y()*n.y()*b*b + n.z()*n.z()*c*c);
		}
		else if (fabs(v.y()) < e && fabs(v.x()) >= e && fabs(v.z()) >= e)
		{
			a = v_len2 / v.x();
			c = v_len2 / v.z();
			result = n.x()*n.z()*sqrt(a*a + c*c) /
				sqrt(n.x()*n.x()*a*a + n.z()*n.z()*c*c);
		}
		else if (fabs(v.z()) < e && fabs(v.x()) >= e && fabs(v.y()) >= e)
		{
			a = v_len2 / v.x();
			b = v_len2 / v.y();
			result = n.x()*n.y()*sqrt(a*a + b*b) /
				sqrt(n.x()*n.x()*a*a + n.y()*n.y()*b*b);
		}
		else if (fabs(v.x()) >= e && fabs(v.y()) < e && fabs(v.z()) < e)
		{
			result = n.x();
		}
		else if (fabs(v.y()) >= e && fabs(v.x()) < e && fabs(v.z()) < e)
		{
			result = n.y();
		}
		else if (fabs(v.z()) >= e && fabs(v.x()) < e && fabs(v.y()) < e)
		{
			result = n.z();
		}
		else
			result = 1.0;

		return result;

	}

	bool TextureRenderer::test_against_view(const fluo::BBox &bbox, bool persp)
	{
		memcpy(mvmat_, glm::value_ptr(m_mv_tex_scl_mat), 16 * sizeof(float));
		memcpy(prmat_, glm::value_ptr(m_proj_mat), 16 * sizeof(float));

		fluo::Transform mv;
		fluo::Transform pr;
		mv.set_trans(mvmat_);
		pr.set_trans(prmat_);

		if (persp)
		{
			const fluo::Point p0_cam(0.0, 0.0, 0.0);
			fluo::Point p0, p0_obj;
			pr.unproject(p0_cam, p0);
			mv.unproject(p0, p0_obj);
			if (bbox.inside(p0_obj))
				return true;
		}

		bool overx = true;
		bool overy = true;
		bool overz = true;
		bool underx = true;
		bool undery = true;
		bool underz = true;
		for (int i = 0; i < 8; i++)
		{
			const fluo::Point pold(
				(i & 1) ? bbox.Min().x() : bbox.Max().x(),
				(i & 2) ? bbox.Min().y() : bbox.Max().y(),
				(i & 4) ? bbox.Min().z() : bbox.Max().z());
			const fluo::Point p = pr.project(mv.project(pold));
			overx = overx && (p.x() > 1.0);
			overy = overy && (p.y() > 1.0);
			overz = overz && (p.z() > 1.0);
			underx = underx && (p.x() < -1.0);
			undery = undery && (p.y() < -1.0);
			underz = underz && (p.z() < -1.0);
		}

		return !(overx || overy || overz || underx || undery || underz);
	}

	bool TextureRenderer::get_adaptive()
	{
		int interactive_quality = glbin_settings.m_interactive_quality;
		switch (interactive_quality)
		{
		case 0://disable
			return false;
		case 1://enaable
			return true;
		case 2://enable for large data
		{
			double data_size = get_data_size();
			if (data_size > glbin_settings.m_large_data_size ||
				data_size > glbin_settings.m_mem_limit)
				return true;
			else
				return false;
		}
		}
		return false;
	}

	double TextureRenderer::get_data_size()
	{
		auto tex = tex_.lock();
		if (!tex)
			return 0.0;
		return tex->nx() * tex->ny() * tex->nz() / 1.04e6;
	}

	int TextureRenderer::get_size_type()
	{
		double data_size = get_data_size();
		if (data_size < glbin_settings.m_small_data_size)
			return 1; //small
		else if (data_size < glbin_settings.m_large_data_size)
			return 0; //normal
		else
			return 2; //large
	}

	double TextureRenderer::get_sample_rate()
	{
		bool interactive = imode_ && get_adaptive();
		bool capture = glbin_states.m_capture;
		int size_type = get_size_type();
		double rate = sample_rate_;
		if (interactive)
			rate *= glbin_settings.m_int_scale;
		if (capture)
			rate *= glbin_settings.m_capture_scale;
		if (size_type == 1) //small
			rate *= glbin_settings.m_small_scale;
		else if (size_type == 2) //large
			rate *= glbin_settings.m_large_scale;
		return rate;
	}

	GLint TextureRenderer::load_brick(TextureBrick* brick,
		GLint filter, bool compression, int unit, int mode, int toffset)
	{
		auto tex = tex_.lock();
		if (!tex)
			return 0;

		GLint result = -1;
		int c = 0;
		int tn = 0;
		void* raw_data = 0;
		//get raw data from cache
		if (cache_queue_)
		{
			VolCache4D* vol_cache = cache_queue_->get_offset(toffset);
			if (vol_cache)
			{
				raw_data = vol_cache->GetRawData();
				tn = static_cast<int>(vol_cache->GetTime());
			}
		}
		else
			raw_data = brick->tex_data(c);

		if (!tex->isBrxml() &&
			(!brick || !raw_data))
			return 0;
		if (c < 0 || c >= TEXTURE_MAX_COMPONENTS)
			return 0;
		if (brick->ntype(c) != TextureBrick::TYPE_INT)
			return 0;

		glActiveTexture(GL_TEXTURE0 + unit);

		int nb = brick->nb(c);
		unsigned int nx = brick->nx();
		unsigned int ny = brick->ny();
		unsigned int nz = brick->nz();
		GLenum textype = brick->tex_type(c);

		//! Try to find the existing texture in tex_pool_, for this brick.
		int idx = -1;
		for (unsigned int i = 0; i < tex_pool_.size() && idx < 0; i++)
		{
			if (tex_pool_[i].Match(brick, c, tn, nx, ny, nz, nb, textype))
			{
				//found!
				idx = i;
			}
		}

		if (idx != -1)
		{
			//! The texture object was located, bind it.
			// bind texture object
			glBindTexture(GL_TEXTURE_3D, tex_pool_[idx].id);
			result = tex_pool_[idx].id;
			// set interpolation method
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, filter);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, filter);
		}
		else //idx == -1
		{
			//see if it needs to free some memory
			if (glbin_settings.m_mem_swap)
				check_swap_memory(brick, c);

			// allocate new object
			unsigned int tex_id;
			glGenTextures(1, (GLuint*)&tex_id);

			// create new entry
			tex_pool_.push_back(TexParam(c, tn, nx, ny, nz, nb, textype, tex_id));
			idx = int(tex_pool_.size()) - 1;

			tex_pool_[idx].brick = brick;
			tex_pool_[idx].comp = c;
			// bind texture object
			glBindTexture(GL_TEXTURE_3D, tex_pool_[idx].id);
			result = tex_pool_[idx].id;
			// set border behavior
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			// set interpolation method
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, filter);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, filter);

			// download texture data
			if (ShaderProgram::no_tex_unpack_)
			{
				glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
				glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
			}
			else if (tex->isBrxml())
			{
				glPixelStorei(GL_UNPACK_ROW_LENGTH, nx);
				glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, ny);
			}
			else
			{
				glPixelStorei(GL_UNPACK_ROW_LENGTH, brick->sx());
				glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, brick->sy());
			}

			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

			if (ShaderProgram::shaders_supported())
			{
				GLenum format;
				GLint internal_format;
				if (nb < 3)
				{
					if (compression && GLEW_ARB_texture_compression_rgtc &&
						brick->ntype(c) == TextureBrick::TYPE_INT)
						internal_format = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
					else
						internal_format = (brick->tex_type(c) == GL_SHORT ||
							brick->tex_type(c) == GL_UNSIGNED_SHORT) ?
						GL_R16 : GL_R8;
					format = GL_RED;
				}
				else
				{
					if (compression && GLEW_ARB_texture_compression_rgtc &&
						brick->ntype(c) == TextureBrick::TYPE_INT)
						internal_format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
					else
						internal_format = (brick->tex_type(c) == GL_SHORT ||
							brick->tex_type(c) == GL_UNSIGNED_SHORT) ?
						GL_RGBA16UI : GL_RGBA8UI;
					format = GL_RGBA;
				}

				if (glTexImage3D)
				{
					if (tex->isBrxml())
					{
						if (load_on_main_thread_)
						{
							bool brkerror = false;
							bool lb_swapped = false;
							if (brick->isLoaded())
							{
								if (brick->get_id_in_loadedbrks() >= 0 && brick->get_id_in_loadedbrks() < loadedbrks.size())
								{
									loadedbrks[brick->get_id_in_loadedbrks()].swapped = true;
									lb_swapped = true;
								}
								else
									brkerror = true;
							}
							else if (mainmem_buf_size_ >= 1.0)
							{
								double bsize = brick->nx()*brick->ny()*brick->nz()*brick->nb(c) / 1.04e6;
								if (available_mainmem_buf_size_ - bsize < 0.0)
								{
									double free_mem_size = 0.0;
									while (free_mem_size < bsize && del_id < loadedbrks.size())
									{
										TextureBrick* b = loadedbrks[del_id].brk;
										if (!loadedbrks[del_id].swapped && b->isLoaded()) {
											b->freeBrkData();
											free_mem_size += b->nx() * b->ny() * b->nz() * b->nb(0) / 1.04e6;
										}
										del_id++;
									}
									available_mainmem_buf_size_ += free_mem_size;
								}
							}

							FileLocInfo *finfo = tex->GetFileName(brick->getID());
							void *texdata = brick->tex_data_brk(c, finfo);
							if (texdata)
							{
								glTexImage3D(GL_TEXTURE_3D, 0, internal_format, nx, ny, nz, 0, format, brick->tex_type(c), 0);
								glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, nx, ny, nz, format, brick->tex_type(c), texdata);
							}
							else
							{
								glDeleteTextures(1, (GLuint*)&tex_pool_[idx].id);
								tex_pool_.erase(tex_pool_.begin() + idx);
								brkerror = true;
								result = -1;
							}

							if (mainmem_buf_size_ == 0.0) brick->freeBrkData();
							else
							{
								if (brkerror)
								{
									brick->freeBrkData();

									double new_mem = brick->nx()*brick->ny()*brick->nz()*brick->nb(c) / 1.04e6;
									available_mainmem_buf_size_ += new_mem;
								}
								else
								{
									if (!lb_swapped)
									{
										double new_mem = brick->nx()*brick->ny()*brick->nz()*brick->nb(c) / 1.04e6;
										available_mainmem_buf_size_ -= new_mem;
									}

									LoadedBrick lb;
									lb.swapped = false;
									lb.size = brick->nx()*brick->ny()*brick->nz()*brick->nb(c) / 1.04e6;
									lb.brk = brick;
									lb.brk->set_id_in_loadedbrks(static_cast<int>(loadedbrks.size()));
									loadedbrks.push_back(lb);

								}

							}
						}
						else
						{
							if (brick->isLoaded())
							{
								bool brkerror = false;
								void *texdata = brick->tex_data_brk(c, NULL);
								if (texdata)
								{
									glTexImage3D(GL_TEXTURE_3D, 0, internal_format, nx, ny, nz, 0, format, brick->tex_type(c), 0);
									glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, nx, ny, nz, format, brick->tex_type(c), texdata);
								}
								else
								{
									glDeleteTextures(1, (GLuint*)&tex_pool_[idx].id);
									tex_pool_.erase(tex_pool_.begin() + idx);
									brkerror = true;
									result = -1;
								}
							}
							else if (!interactive_)
							{
								uint32_t rn_time;
								unsigned long elapsed;
								long t;
								do {
									rn_time = static_cast<uint32_t>(GET_TICK_COUNT());
									elapsed = rn_time - st_time_;
									t = glbin_settings.m_up_time - elapsed;
									if (t > 0) std::this_thread::sleep_for(std::chrono::milliseconds(t));
								} while (elapsed <= static_cast<unsigned long>(glbin_settings.m_up_time));

								if (brick->isLoaded())
								{
									bool brkerror = false;
									void *texdata = brick->tex_data_brk(c, NULL);
									if (texdata)
									{
										glTexImage3D(GL_TEXTURE_3D, 0, internal_format, nx, ny, nz, 0, format, brick->tex_type(c), 0);
										glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, nx, ny, nz, format, brick->tex_type(c), texdata);
									}
									else
									{
										glDeleteTextures(1, (GLuint*)&tex_pool_[idx].id);
										tex_pool_.erase(tex_pool_.begin() + idx);
										brkerror = true;
										result = -1;
									}
								}
								else
								{
									glDeleteTextures(1, (GLuint*)&tex_pool_[idx].id);
									tex_pool_.erase(tex_pool_.begin() + idx);
									result = -1;
								}
							}
							else
							{
								glDeleteTextures(1, (GLuint*)&tex_pool_[idx].id);
								tex_pool_.erase(tex_pool_.begin() + idx);
								result = -1;
							}
						}
					}
					else
					{
						glTexImage3D(GL_TEXTURE_3D, 0, internal_format, nx, ny, nz, 0, format,
							brick->tex_type(c), 0);

						load_texture(brick->tex_data(c, raw_data), nx, ny, nz, nb, brick->sx(), brick->sy(), brick->tex_type(c), format);
					}

					if (glbin_settings.m_mem_swap)
					{
						double new_mem = brick->nx()*brick->ny()*brick->nz()*brick->nb(c) / 1.04e6;
						glbin_settings.m_available_mem -= new_mem;
					}
				}
			}

			if (!ShaderProgram::no_tex_unpack_)
			{
				glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
				glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
			}

			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		}

		//if (glbin_settings.m_mem_swap &&
		//	start_update_loop_ &&
		//	!done_update_loop_)
		//{
		//	if (!brick->drawn(mode))
		//	{
		//		brick->set_drawn(mode, true);
		//		cur_brick_num_++;
		//		cur_chan_brick_num_++;
		//	}
		//}

		glActiveTexture(GL_TEXTURE0);

		return result;
	}

	//search for or create the mask texture in the texture pool
	GLint TextureRenderer::load_brick_mask(TextureBrick* brick, GLint filter, bool compression, int unit)
	{
		GLint result = -1;
		if (!brick)
			return result;
		int c = brick->nmask();
		if (c < 0)
			return result;
		int tn = 0;

		glActiveTexture(GL_TEXTURE0 + (unit > 0 ? unit : c));

		int nb = brick->nb(c);
		unsigned int nx = brick->nx();
		unsigned int ny = brick->ny();
		unsigned int nz = brick->nz();
		GLenum textype = brick->tex_type(c);

		//! Try to find the existing texture in tex_pool_, for this brick.
		int idx = -1;
		for (size_t i = 0; i < tex_pool_.size() && idx < 0; i++)
		{
			if (tex_pool_[i].Match(brick, c, tn, nx, ny, nz, nb, textype))
			{
				//found!
				idx = static_cast<int>(i);
			}
		}

		if (idx != -1)
		{
			//! The texture object was located, bind it.
			// bind texture object
			glBindTexture(GL_TEXTURE_3D, tex_pool_[idx].id);
			result = tex_pool_[idx].id;
			// set interpolation method
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, filter);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, filter);
		}
		else //idx == -1
		{
			////! try again to find the matching texture object
			unsigned int tex_id;
			glGenTextures(1, (GLuint*)&tex_id);

			tex_pool_.push_back(TexParam(c, tn, nx, ny, nz, nb, textype, tex_id));
			idx = int(tex_pool_.size()) - 1;

			tex_pool_[idx].brick = brick;
			tex_pool_[idx].comp = c;
			// bind texture object
			glBindTexture(GL_TEXTURE_3D, tex_pool_[idx].id);
			result = tex_pool_[idx].id;

			// set border behavior
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			// set interpolation method
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, filter);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, filter);

			// download texture data
			if (ShaderProgram::no_tex_unpack_)
			{
				glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
				glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
			}
			else
			{
				glPixelStorei(GL_UNPACK_ROW_LENGTH, brick->sx());
				glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, brick->sy());
			}

			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

			if (ShaderProgram::shaders_supported())
			{
				GLint internal_format = GL_R8;
				GLenum format = (nb == 1 ? GL_RED : GL_RGBA);
				if (glTexImage3D)
				{
					glTexImage3D(GL_TEXTURE_3D, 0, internal_format, nx, ny, nz, 0, format,
						brick->tex_type(c), 0);

					load_texture(brick->tex_data(c), nx, ny, nz, nb, brick->sx(), brick->sy(), brick->tex_type(c), format);
				}
			}

			if (!ShaderProgram::no_tex_unpack_)
			{
				glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
				glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
			}

			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		}

		glActiveTexture(GL_TEXTURE0);

		return result;
	}

	//search for or create the label texture in the texture pool
	GLint TextureRenderer::load_brick_label(TextureBrick* brick)
	{
		GLint result = -1;
		if (!brick)
			return result;

		int c = brick->nlabel();
		int tn = 0;

		glActiveTexture(GL_TEXTURE0 + c);

		int nb = brick->nb(c);
		int nx = brick->nx();
		int ny = brick->ny();
		int nz = brick->nz();
		GLenum textype = brick->tex_type(c);

		//! Try to find the existing texture in tex_pool_, for this brick.
		int idx = -1;
		for (unsigned int i = 0; i < tex_pool_.size() && idx < 0; i++)
		{
			if (tex_pool_[i].Match(brick, c, tn, nx, ny, nz, nb, textype))
			{
				//found!
				idx = i;
			}
		}

		if (idx != -1)
		{
			//! The texture object was located, bind it.
			// bind texture object
			glBindTexture(GL_TEXTURE_3D, tex_pool_[idx].id);
			result = tex_pool_[idx].id;
			// set interpolation method
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
		else //idx == -1
		{
			unsigned int tex_id;
			glGenTextures(1, (GLuint*)&tex_id);
			tex_pool_.push_back(TexParam(c, tn, nx, ny, nz, nb, textype, tex_id));
			idx = int(tex_pool_.size()) - 1;

			tex_pool_[idx].brick = brick;
			tex_pool_[idx].comp = c;
			// bind texture object
			glBindTexture(GL_TEXTURE_3D, tex_pool_[idx].id);
			result = tex_pool_[idx].id;

			// set border behavior
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			// set interpolation method
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			// download texture data
			if (ShaderProgram::no_tex_unpack_)
			{
				glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
				glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
			}
			else
			{
				glPixelStorei(GL_UNPACK_ROW_LENGTH, brick->sx());
				glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, brick->sy());
			}

			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

			if (ShaderProgram::shaders_supported())
			{
				GLenum format = GL_RED_INTEGER;
				GLint internal_format = GL_R32UI;
				if (glTexImage3D)
				{
					glTexImage3D(GL_TEXTURE_3D, 0, internal_format, nx, ny, nz, 0, format,
						brick->tex_type(c), NULL);

					load_texture(brick->tex_data(c), nx, ny, nz, nb, brick->sx(), brick->sy(), brick->tex_type(c), format);
				}
			}

			if (!ShaderProgram::no_tex_unpack_)
			{
				glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
				glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
			}
		}

		glActiveTexture(GL_TEXTURE0);

		return result;
	}

	bool TextureRenderer::brick_sort(const BrickDist& bd1, const BrickDist& bd2)
	{
		return bd1.dist > bd2.dist;
	}

	void TextureRenderer::check_swap_memory(TextureBrick* brick, int c)
	{
		unsigned int i;
		double new_mem = brick->nx()*brick->ny()*brick->nz()*brick->nb(c) / 1.04e6;

		if (glbin_settings.m_use_mem_limit)
		{
			if (glbin_settings.m_available_mem >= new_mem)
				return;
		}
		else
		{
			GLenum error = glGetError();
			GLint mem_info[4] = { 0, 0, 0, 0 };
			glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, mem_info);
			error = glGetError();
			if (error == GL_INVALID_ENUM)
			{
				glGetIntegerv(GL_TEXTURE_FREE_MEMORY_ATI, mem_info);
				error = glGetError();
				if (error == GL_INVALID_ENUM)
					return;
			}

			//available memory size in MB
			glbin_settings.m_available_mem = mem_info[0] / 1024.0;
			if (glbin_settings.m_available_mem >= new_mem)
				return;
		}

		std::vector<BrickDist> bd_list;
		BrickDist bd;
		//generate a list of bricks and their distances to the new brick
		for (i = 0; i < tex_pool_.size(); i++)
		{
			bd.index = i;
			bd.brick = tex_pool_[i].brick;
			//calculate the distance
			bd.dist = brick->bbox().distance(bd.brick->bbox());
			bd_list.push_back(bd);
		}

		//release bricks far away
		double est_avlb_mem = glbin_settings.m_available_mem;
		if (bd_list.size() > 0)
		{
			//sort from farthest to closest
			std::sort(bd_list.begin(), bd_list.end(), TextureRenderer::brick_sort);

			//remove
			for (i = 0; i < bd_list.size(); i++)
			{
				TextureBrick* btemp = bd_list[i].brick;
				tex_pool_[bd_list[i].index].delayed_del = true;
				double released_mem = btemp->nx()*btemp->ny()*btemp->nz()*btemp->nb(c) / 1.04e6;
				est_avlb_mem += released_mem;
				if (est_avlb_mem >= new_mem)
					break;
			}

			//delete from pool
			for (int j = int(tex_pool_.size() - 1); j >= 0; j--)
			{
				if (tex_pool_[j].delayed_del &&
					tex_pool_[j].id)
				{
					glDeleteTextures(1, (GLuint*)&tex_pool_[j].id);
					tex_pool_.erase(tex_pool_.begin() + j);
				}
			}

			if (glbin_settings.m_use_mem_limit)
				glbin_settings.m_available_mem = est_avlb_mem;
		}
	}

	void TextureRenderer::draw_view_quad(double d)
	{
		VertexArray* quad_va = 0;
		if (d == 0.0)
		{
			quad_va = glbin_vertex_array_manager.vertex_array(VA_Norm_Square);
			if (quad_va)
				quad_va->draw();
		}
		else
		{
			quad_va = glbin_vertex_array_manager.vertex_array(VA_Norm_Square_d);
			if (quad_va)
			{
				quad_va->set_param(0, d);
				quad_va->draw();
			}
		}
	}

	void TextureRenderer::draw_polygons(std::vector<float>& vertex,
		std::vector<uint32_t>& index)
	{
		if (vertex.empty() || index.empty())
			return;

		bool set_pointers = false;
		if (!va_slices_ || !va_slices_->valid())
		{
			va_slices_ = glbin_vertex_array_manager.vertex_array(true, true);
			set_pointers = true;
		}
		if (va_slices_)
		{
			va_slices_->buffer_data(
				VABuf_Coord, sizeof(float)*vertex.size(),
				&vertex[0], GL_STREAM_DRAW);
			va_slices_->buffer_data(
				VABuf_Index, sizeof(uint32_t)*index.size(),
				&index[0], GL_STREAM_DRAW);
			if (set_pointers)
			{
				va_slices_->attrib_pointer(
					0, 3, GL_FLOAT, GL_FALSE,
					6 * sizeof(float),
					(const GLvoid*)0);
				va_slices_->attrib_pointer(
					1, 3, GL_FLOAT, GL_FALSE,
					6 * sizeof(float),
					(const GLvoid*)12);
			}
			va_slices_->draw_begin();
			va_slices_->draw_elements(
				GL_TRIANGLES, static_cast<GLsizei>(index.size()),
				GL_UNSIGNED_INT, 0);
			va_slices_->draw_end();
		}
	}

	void TextureRenderer::draw_polygons_wireframe(std::vector<float>& vertex,
		std::vector<uint32_t>& index, std::vector<uint32_t>& size)
	{
		if (vertex.empty() || index.empty())
			return;

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		bool set_pointers = false;
		if (!va_wirefm_ || !va_wirefm_->valid())
		{
			va_wirefm_ = glbin_vertex_array_manager.vertex_array(true, true);
			set_pointers = true;
		}
		if (va_wirefm_)
		{
			va_wirefm_->buffer_data(
				VABuf_Coord, sizeof(float)*vertex.size(),
				&vertex[0], GL_STREAM_DRAW);
			va_wirefm_->buffer_data(
				VABuf_Index, sizeof(uint32_t)*index.size(),
				&index[0], GL_STREAM_DRAW);
			if (set_pointers)
			{
				va_wirefm_->attrib_pointer(
					0, 3, GL_FLOAT, GL_FALSE,
					6 * sizeof(float),
					(const GLvoid*)0);
				va_wirefm_->attrib_pointer(
					1, 3, GL_FLOAT, GL_FALSE,
					6 * sizeof(float),
					(const GLvoid*)12);
			}
			va_wirefm_->draw_begin();
			unsigned int idx_num;
			for (unsigned int i = 0, k = 0; i < size.size(); ++i)
			{
				idx_num = (size[i] - 2) * 3;
				va_wirefm_->draw_elements(
					GL_TRIANGLES, idx_num, GL_UNSIGNED_INT,
					reinterpret_cast<const GLvoid*>((long long)(k)));
				k += idx_num * 4;
			}
			va_wirefm_->draw_end();
		}

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	//bind 2d mask for segmentation
	void TextureRenderer::bind_2d_mask()
	{
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, tex_2d_mask_);
		glActiveTexture(GL_TEXTURE0);
	}

	//bind 2d weight map for segmentation
	void TextureRenderer::bind_2d_weight()
	{
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, tex_2d_weight1_);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, tex_2d_weight2_);
		glActiveTexture(GL_TEXTURE0);
	}

	//bind 2d depth map for rendering shadows
	void TextureRenderer::bind_2d_dmap()
	{
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, tex_2d_dmap_);
		glActiveTexture(GL_TEXTURE0);
	}

	void TextureRenderer::load_texture(void* tex_data,
		unsigned int nx,
		unsigned int ny,
		unsigned int nz,
		unsigned int nb,
		unsigned int sx,
		unsigned int sy,
		GLenum tex_type,
		GLenum format)
	{
		auto tex = tex_.lock();
		if (!tex)
			return;

		if (ShaderProgram::no_tex_unpack_)
		{
			if (tex->get_brick_num() > 1)
			{
				unsigned long long mem_size = (unsigned long long)nx*
					(unsigned long long)ny*(unsigned long long)nz*nb;
				unsigned char* temp = new unsigned char[mem_size];
				unsigned char* tempp = temp;
				unsigned char* tp = (unsigned char*)(tex_data);
				unsigned char* tp2;
				for (size_t k = 0; k < static_cast<size_t>(nz); ++k)
				{
					tp2 = tp;
					for (size_t j = 0; j < static_cast<size_t>(ny); ++j)
					{
						memcpy(tempp, tp2, nx*nb);
						tempp += nx*nb;
						tp2 += sx*nb;
					}
					tp += sx*sy*nb;
				}
				glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, nx, ny, nz, format,
					tex_type, (GLvoid*)temp);
				delete[]temp;
			}
			else
				glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, nx, ny, nz, format,
					tex_type, tex_data);
		}
		else
			glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, nx, ny, nz, format,
				tex_type, tex_data);

	}

	//release texture
	void TextureRenderer::release_texture(int unit, GLenum taget)
	{
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(taget, 0);
		glActiveTexture(GL_TEXTURE0);
	}

	void TextureRenderer::rearrangeLoadedBrkVec()
	{
		if (loadedbrks.empty()) return;
		std::vector<LoadedBrick>::iterator ite = loadedbrks.begin();
		while (ite != loadedbrks.end())
		{
			if (!ite->brk->isLoaded())
			{
				ite->brk->set_id_in_loadedbrks(-1);
				ite = loadedbrks.erase(ite);
			}
			else if (ite->swapped)
			{
				ite = loadedbrks.erase(ite);
			}
			else ite++;
		}
		for (int i = 0; i < loadedbrks.size(); i++) loadedbrks[i].brk->set_id_in_loadedbrks(i);
		del_id = 0;
	}

	//Texture
	void TextureRenderer::clear_brick_buf()
	{
		auto tex = tex_.lock();
		if (!tex)
			return;

		int cur_lv = tex->GetCurLevel();
		for (size_t lv = 0; static_cast<long long>(lv) < static_cast<long long>(tex->GetLevelNum()); lv++)
		{
			tex->setLevel(static_cast<int>(lv));
			std::vector<TextureBrick *> *bs = tex->get_bricks();
			for (size_t i = 0; i < bs->size(); i++)
			{
				if ((*bs)[i]->isLoaded()) {
					available_mainmem_buf_size_ += (*bs)[i]->nx() * (*bs)[i]->ny() * (*bs)[i]->nz() * (*bs)[i]->nb(0) / 1.04e6;
					(*bs)[i]->freeBrkData();
				}
			}
			rearrangeLoadedBrkVec();//tex_
		}
		tex->setLevel(cur_lv);
	}

} // namespace flvr




