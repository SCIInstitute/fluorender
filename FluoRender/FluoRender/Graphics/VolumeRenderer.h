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

#ifndef VolumeRenderer_h
#define VolumeRenderer_h

#include <TextureRenderer.h>
#include <Color.h>
#include <Size.h>
#include <Vector4i.h>
#include <Vector4f.h>
#include <vector>

#ifndef TextureBrick_h
#define TEXTURE_RENDER_MODES	5
#endif

namespace fluo
{
	class Plane;
}
namespace flvr
{
	class MultiVolumeRenderer;
	class Texture;
	class RenderModeGuard;

	class VolumeRenderer : public TextureRenderer
	{
	public:
		VolumeRenderer(const std::vector<fluo::Plane*> &planes);
		VolumeRenderer(const VolumeRenderer&);
		virtual ~VolumeRenderer();

		//set viewport
		void set_viewport(const fluo::Vector4i& vp) { viewport_ = vp; }

		//mouse position in viewport
		void set_mouse_position(int mp[2]) { memcpy(mp_, mp, sizeof(int) * 2); }
		//mouse direction
		void set_mouse_vec(fluo::Vector &mvec) { mvec_ = mvec; }
		//set scale/zoom factor for 2d
		void set_zoom(float sf, float sf121) { zoom_ = sf; zoom_data_ = sf / sf121; }

		//set clear color
		void set_clear_color(const fluo::Vector4f& clear_color) { clear_color_ = clear_color; }

		//render mode
		void set_mode(RenderMode mode) { mode_ = mode; }
		//range and scale
		void set_scalar_scale(double val) { scalar_scale_ = val; }
		double get_scalar_scale() { return scalar_scale_; }
		void set_gm_scale(double val) { gm_scale_ = val; }
		//transfer function properties
		void set_gamma3d(double gamma) { gamma3d_ = gamma; }
		double get_gamma3d() { return gamma3d_; }
		void set_gm_low(double val) { gm_low_ = val; }
		double get_gm_low() { return gm_low_; }
		void set_gm_high(double val) { gm_high_ = val; }
		double get_gm_high() { return gm_high_; }
		void set_gm_max(double val) { gm_max_ = val; }
		double get_gm_max() { return gm_max_; }
		void set_lo_offset(double val) { lo_offset_ = val; }
		double get_lo_offset() { return lo_offset_; }
		void set_hi_offset(double val) { hi_offset_ = val; }
		double get_hi_offset() { return hi_offset_; }
		void set_lo_thresh(double thresh) { lo_thresh_ = thresh; }
		double get_lo_thresh() { return lo_thresh_; }
		void set_hi_thresh(double thresh) { hi_thresh_ = thresh; }
		double get_hi_thresh() { return hi_thresh_; }
		void set_soft_thresh(double val) { sw_ = val; }
		double get_soft_thresh() { return sw_; }
		void set_color(const fluo::Color& color);
		fluo::Color get_color() { return color_; }
		void set_mask_color(const fluo::Color& color, bool set=true);
		fluo::Color get_mask_color() { return mask_color_; }
		bool get_mask_color_set() {return mask_color_set_; }
		void reset_mask_color_set() {mask_color_set_ = false;}
		void set_mask_thresh(double thresh) { mask_thresh_ = thresh; }
		double get_mask_thresh() { return mask_thresh_; }
		void set_alpha(double alpha) { alpha_ = alpha; }
		double get_alpha() { return alpha_; }
		void set_luminance(double luminance) { luminance_ = luminance; }
		double get_luminance() { return luminance_; }

		//shading
		void set_shading(bool shading) { shading_ = shading; }
		bool get_shading() { return shading_; }
		void set_material(double amb, double diff, double spec, double shine) { ambient_ = amb; diffuse_ = amb+0.7; diffuse_ = diffuse_>1.0?1.0:diffuse_; specular_ = spec; shine_ = shine; }

		//colormap mode
		void set_colormap_inv(double val) { colormap_inv_ = val; }
		void set_colormap_mode(int mode) { colormap_mode_ = mode; }
		void set_colormap_values(double low, double hi) { colormap_low_value_ = low; colormap_hi_value_ = hi; }
		void set_colormap(int value) { colormap_ = value; }
		void set_colormap_proj(int value) { colormap_proj_ = value; }

		//label color shuffling
		void set_shuffle(int val) { shuffle_ = val; }
		int get_shuffle() { return shuffle_; }

		//solid
		void set_solid(bool mode) { solid_ = mode; }
		bool get_solid() { return solid_; }

		//slice number
		int get_slice_num() { return static_cast<int>(num_slices_); }

		//clipping planes
		void set_planes(std::vector<fluo::Plane*> *p);
		std::vector<fluo::Plane*> *get_planes();

		//interpolation
		bool get_interpolate() {return interpolate_; }
		void set_interpolate(bool b) { interpolate_ = b;}

		//depth peel
		void set_depth_peel(int dp) {depth_peel_ = dp;}
		int get_depth_peel() {return depth_peel_;}

		//draw
		void eval_ml_mode();
		//mode: 0-normal; 1-MIP; 2-shading; 3-shadow, 4-mask
		virtual void draw(bool draw_wireframe_p,
			bool interactive_mode_p,
			bool orthographic_p = false,
			int mode = 0);
		void draw_wireframe(bool orthographic_p = false);
		//mode: 0-normal; 1-MIP; 2-shading; 3-shadow, 4-mask
		void draw_volume(
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

		//calculation
		void calculate(int type, VolumeRenderer* vr_a, VolumeRenderer* vr_b);

		//return
		void return_volume();//return the data volume
		void return_mask(int order = 0);//return the mask volume
		void return_label(); //return the label volume

		//mask and label
		int get_ml_mode() { return ml_mode_; }
		void set_ml_mode(int mode) { ml_mode_ = mode; }

		//set noise reduction
		void SetNoiseRed(bool nd) { noise_red_ = nd; }
		bool GetNoiseRed() { return noise_red_; }

		//inversion
		void set_inversion(bool mode) { inv_ = mode; }
		bool get_inversion() { return inv_; }

		//alpha power
		void set_alpha_power(double val) { alpha_power_ = val; }
		double get_alpha_power() { return alpha_power_; }

		//compression
		void set_compression(bool compression) { compression_ = compression; }

		//done loop
		bool get_done_loop(int mode) { if (mode>=0 && mode<TEXTURE_RENDER_MODES) return done_loop_[mode]; else return false; }
		void set_done_loop(bool done) { for (int i=0; i<TEXTURE_RENDER_MODES; i++) done_loop_[i] = done; }

		//estimated threshold
		double get_estimated_thresh() { return est_thresh_; }

		//set matrices
		void set_matrices(glm::mat4& mv_mat, glm::mat4& proj_mat, glm::mat4& tex_mat);
		void update_mv_tex_scl_mat();
		glm::mat4 get_mv_tex_scl_mat() { return m_mv_tex_scl_mat; }
		glm::mat4 get_proj_mat() { return m_proj_mat; }

		//fog
		void set_fog(bool use_fog, double fog_intensity, double fog_start, double fog_end) { m_use_fog = use_fog; m_fog_intensity = fog_intensity; m_fog_start = fog_start; m_fog_end = fog_end; }
		void set_fog(bool use_fog) { m_use_fog = use_fog; }

		friend class MultiVolumeRenderer;

	protected:
		fluo::Vector4i viewport_;//viewport
		int mp_[2];//mouse position in viewport
		fluo::Vector mvec_;//mouse direction vector for grow selection
		float zoom_;//zoom ratio for 2d processings
		float zoom_data_;//for point grow

		//clear color
		fluo::Vector4f clear_color_;

		double scalar_scale_;
		double gm_scale_;
		//transfer function properties
		double gamma3d_;
		double gm_low_;
		double gm_high_;
		double gm_max_;
		double lo_offset_;
		double hi_offset_;
		double lo_thresh_;
		double hi_thresh_;
		double sw_;//soft threshold
		fluo::Color color_;
		fluo::Color mask_color_;
		double alpha_power_;
		bool mask_color_set_;
		double mask_thresh_;
		double alpha_;
		double luminance_;
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
		//planes
		std::vector<fluo::Plane *> planes_;
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
		double sfactor_;

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

	private:
		std::string get_buffer_name();
		Size2D resize(const std::string& buf_name);

		friend class RenderModeGuard;
	};

	struct ScopedRenderMode
	{
		RenderMode mode;
		fluo::Color color;
		int colormap_mode;
		int colormap_proj;
		bool solid;
		double alpha;
		bool shading;
		int ml_mode;
	};

	class RenderModeGuard
	{
	public:
		RenderModeGuard(VolumeRenderer& renderer)
			: renderer_(renderer)
		{
			prev_mode_ = {
				renderer_.mode_,
				renderer_.color_,
				renderer_.colormap_mode_,
				renderer_.colormap_proj_,
				renderer_.solid_,
				renderer_.alpha_,
				renderer_.shading_,
				renderer_.ml_mode_
			};
		}

		~RenderModeGuard()
		{
			renderer_.mode_ = prev_mode_.mode;
			renderer_.color_ = prev_mode_.color;
			renderer_.colormap_mode_ = prev_mode_.colormap_mode;
			renderer_.colormap_proj_ = prev_mode_.colormap_proj;
			renderer_.solid_ = prev_mode_.solid;
			renderer_.alpha_ = prev_mode_.alpha;
			renderer_.shading_ = prev_mode_.shading;
			renderer_.ml_mode_ = prev_mode_.ml_mode;
		}
	private:
		VolumeRenderer& renderer_;
		ScopedRenderMode prev_mode_;
	};
} // End namespace flvr

#endif 
