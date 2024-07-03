//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2024 Scientific Computing and Imaging Institute,
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

#ifndef VolumeRenderer_h
#define VolumeRenderer_h

#include <Color.h>
#include <Plane.h>
#include <Texture.h>
#include <TextureRenderer.h>
#include <ImgShader.h>
#include <LightFieldShader.h>

namespace flvr
{
	class MultiVolumeRenderer;

	class VolumeRenderer : public TextureRenderer
	{
	public:
		VolumeRenderer(Texture* tex,
			const vector<fluo::Plane*> &planes);
		VolumeRenderer(const VolumeRenderer&);
		virtual ~VolumeRenderer();

		//set viewport
		void set_viewport(GLint vp[4])
		{
			memcpy(vp_, vp, sizeof(GLint) * 4);
		}

		//mouse position in viewport
		void set_mouse_position(GLint mp[2])
		{
			memcpy(mp_, mp, sizeof(GLint) * 2);
		}
		//mouse direction
		void set_mouse_vec(fluo::Vector &mvec)
		{
			mvec_ = mvec;
		}
		//set scale/zoom factor for 2d
		void set_zoom(GLfloat sf, GLfloat sf121)
		{
			zoom_ = sf;
			zoom_data_ = sf / sf121;
		}

		//set clear color
		void set_clear_color(GLfloat clear_color[4])
		{
			memcpy(clear_color_, clear_color, sizeof(GLfloat) * 4);
		}

		//render mode
		void set_mode(RenderMode mode);
		//range and scale
		void set_scalar_scale(double scale);
		double get_scalar_scale();
		void set_gm_scale(double scale);
		//transfer function properties
		void set_gamma3d(double gamma);
		double get_gamma3d();
		void set_gm_thresh(double thresh);
		double get_gm_thresh();
		void set_offset(double offset);
		double get_offset();
		void set_lo_thresh(double thresh);
		double get_lo_thresh();
		void set_hi_thresh(double thresh);
		double get_hi_thresh();
		void set_soft_thresh(double val);
		double get_soft_thresh();
		void set_color(const fluo::Color& color);
		fluo::Color get_color();
		void set_mask_color(const fluo::Color& color, bool set=true);
		fluo::Color get_mask_color();
		bool get_mask_color_set() {return mask_color_set_; }
		void reset_mask_color_set() {mask_color_set_ = false;}
		void set_mask_thresh(double thresh);
		double get_mask_thresh();
		void set_alpha(double alpha);
		double get_alpha();

		//shading
		void set_shading(bool shading) { shading_ = shading; }
		bool get_shading() { return shading_; }
		void set_material(double amb, double diff, double spec, double shine)
		{ ambient_ = amb; diffuse_ = amb+0.7;
		diffuse_ = diffuse_>1.0?1.0:diffuse_;
		specular_ = spec; shine_ = shine; }

		//colormap mode
		void set_colormap_inv(double val)
		{colormap_inv_ = val;}
		void set_colormap_mode(int mode)
		{colormap_mode_ = mode;}
		void set_colormap_values(double low, double hi)
		{colormap_low_value_ = low; colormap_hi_value_ = hi;}
		void set_colormap(int value)
		{colormap_ = value;}
		void set_colormap_proj(int value)
		{colormap_proj_ = value;}

		//label color shuffling
		void set_shuffle(int val)
		{ shuffle_ = val; }
		int get_shuffle()
		{ return shuffle_; }

		//solid
		void set_solid(bool mode)
		{solid_ = mode;}
		bool get_solid()
		{return solid_;}

		//sampling rate
		void set_sampling_rate(double rate);
		double get_sampling_rate();
		double num_slices_to_rate(int slices);
		//slice number
		int get_slice_num();

		//interactive modes
		void set_interactive_rate(double irate);
		void set_interactive_mode(bool mode);
		void set_adaptive(bool b);

		//clipping planes
		void set_planes(vector<fluo::Plane*> *p);
		vector<fluo::Plane*> *get_planes();

		//interpolation
		bool get_interpolate();
		void set_interpolate(bool b);

		//depth peel
		void set_depth_peel(int dp) {depth_peel_ = dp;}
		int get_depth_peel() {return depth_peel_;}

		//draw
		void eval_ml_mode();
		//mode: 0-normal; 1-MIP; 2-shading; 3-shadow, 4-mask
		virtual void draw(bool draw_wireframe_p,
			bool adaptive,
			bool interactive_mode_p,
			bool orthographic_p = false,
			int mode = 0);
		void draw_wireframe(bool orthographic_p = false);
		//mode: 0-normal; 1-MIP; 2-shading; 3-shadow, 4-mask
		void draw_volume(bool adaptive,
			bool interactive_mode_p,
			bool orthographic_p = false,
			int mode = 0);
		//type: 0-initial; 1-diffusion-based growing; 2-masked filtering
		//paint_mode: 1-select; 2-append; 3-erase; 4-diffuse; 5-flood; 6-clear; 7-all;
		//			  11-posterize
		//hr_mode (hidden removal): 0-none; 1-ortho; 2-persp
		void draw_mask(int type, int paint_mode, int hr_mode,
			double ini_thresh, double gm_falloff, double scl_falloff,
			double scl_translate, double w2d, double bins, int order,
			bool ortho, bool estimate);
		//for multibrick, copy border to continue diffusion
		void copy_mask_border(GLint btex, TextureBrick *b, int order);

		double calc_hist_3d(GLuint, GLuint, size_t, size_t, size_t);

		//calculation
		void calculate(int type, VolumeRenderer* vr_a, VolumeRenderer* vr_b);

		//return
		void return_volume();//return the data volume
		void return_mask(int order = 0);//return the mask volume
		void return_label(); //return the label volume

		//mask and label
		int get_ml_mode() {return ml_mode_;}
		void set_ml_mode(int mode) {ml_mode_ = mode;}

		//set noise reduction
		void SetNoiseRed(bool nd) {noise_red_ = nd;}
		bool GetNoiseRed() {return noise_red_;}

		//inversion
		void set_inversion(bool mode) {inv_ = mode;}
		bool get_inversion() {return inv_;}

		//alpha power
		void set_alpha_power(double val) { alpha_power_ = val; }
		double get_alpha_power() { return alpha_power_; }

		//compression
		void set_compression(bool compression) {compression_ = compression;}

		//done loop
		bool get_done_loop(int mode)
		{if (mode>=0 && mode<TEXTURE_RENDER_MODES) return done_loop_[mode]; else return false;}
		void set_done_loop(bool done)
		{for (int i=0; i<TEXTURE_RENDER_MODES; i++) done_loop_[i] = done;}

		//estimated threshold
		double get_estimated_thresh()
		{ return est_thresh_; }

		//set matrices
		void set_matrices(glm::mat4 &mv_mat, glm::mat4 &proj_mat, glm::mat4 &tex_mat)
		{ m_mv_mat = mv_mat; m_proj_mat = proj_mat; m_tex_mat = tex_mat; }

		//fog
		void set_fog(bool use_fog, double fog_intensity, double fog_start, double fog_end)
		{ m_use_fog = use_fog; m_fog_intensity = fog_intensity; m_fog_start = fog_start; m_fog_end = fog_end; }

		friend class MultiVolumeRenderer;

	protected:
		GLint vp_[4];//viewport
		GLint mp_[2];//mouse position in viewport
		fluo::Vector mvec_;//mouse direction vector for grow selection
		GLfloat zoom_;//zoom ratio for 2d processings
		GLfloat zoom_data_;//for point grow

		//clear color
		GLfloat clear_color_[4];

		double scalar_scale_;
		double gm_scale_;
		//transfer function properties
		double gamma3d_;
		double gm_thresh_;
		double offset_;
		double lo_thresh_;
		double hi_thresh_;
		double sw_;//soft threshold
		fluo::Color color_;
		fluo::Color mask_color_;
		double alpha_power_;
		bool mask_color_set_;
		double mask_thresh_;
		double alpha_;
		//shading
		bool shading_;
		double ambient_, diffuse_, specular_, shine_;
		//colormap mode
		double colormap_inv_;
		int colormap_mode_;//0-normal; 1-rainbow; 2-depth
		double colormap_low_value_;
		double colormap_hi_value_;
		int colormap_;
		int colormap_proj_;
		//solid
		bool solid_;
		//interpolation
		bool interpolate_;
		//adaptive
		bool adaptive_;
		//planes
		vector<fluo::Plane *> planes_;
		//depth peel
		int depth_peel_;
		//segmentation
		int ml_mode_;	//0-normal, 1-render with mask, 2-render with mask excluded,
						//3-random color with label, 4-random color with label+mask
		bool mask_;
		bool label_;
		int shuffle_;//for label color shuffling

		//noise reduction
		bool noise_red_;
		//scale factor and filter sizes
		double sfactor_;
		double filter_size_min_;
		double filter_size_max_;
		double filter_size_shp_;

		//inversion
		bool inv_;

		//compression
		bool compression_;

		//done loop
		bool done_loop_[TEXTURE_RENDER_MODES];

		//estimated threshold
		double est_thresh_;

		//fog
		bool m_use_fog;
		double m_fog_intensity;
		double m_fog_start;
		double m_fog_end;

		//calculating scaling factor, etc
		double CalcScaleFactor(double w, double h, double tex_w, double tex_h);
		//calculate the filter sizes
		double CalcFilterSize(int type, 
			double w, double h, double tex_w, double tex_h, 
			double sf);	//type - 1:min filter
						//		 2:max filter
						//		 3:sharpening

	};

} // End namespace flvr

#endif 
