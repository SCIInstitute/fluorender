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

#ifndef SLIVR_VolumeRenderer_h
#define SLIVR_VolumeRenderer_h

#include "Color.h"
#include "Plane.h"
#include "Texture.h"
#include "TextureRenderer.h"
#include "ImgShader.h"

namespace FLIVR
{
	class MultiVolumeRenderer;

	class VolumeRenderer : public TextureRenderer
	{
	public:
		VolumeRenderer(Texture* tex,
			const vector<Plane*> &planes,
			bool hiqual);
		VolumeRenderer(const VolumeRenderer&);
		virtual ~VolumeRenderer();

		//quality and data type
		//set by initialization
		bool get_hiqual() {return hiqual_;}

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
		void set_color(Color color);
		Color get_color();
		Color get_mask_color();
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
		void set_colormap_mode(int mode)
		{colormap_mode_ = mode;}
		void set_colormap_values(double low, double hi)
		{colormap_low_value_ = low; colormap_hi_value_ = hi;}

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
		void set_planes(vector<Plane*> *p);
		vector<Plane*> *get_planes();

		//interpolation
		bool get_interpolate();
		void set_interpolate(bool b);

		//depth peel
		void set_depth_peel(int dp) {depth_peel_ = dp;}
		int get_depth_peel() {return depth_peel_;}

		//draw
		virtual void draw(bool draw_wireframe_p, 
			bool interactive_mode_p, 
			bool orthographic_p = false,
			double zoom = 1.0, 
			int mode = 0);
		void draw_wireframe(bool orthographic_p = false);
		void draw_volume(bool interactive_mode_p,
			bool orthographic_p = false,
			double zoom = 1.0, 
			int mode = 0);
		//type: 0-initial; 1-diffusion-based growing; 2-masked filtering
		//paint_mode: 1-select; 2-append; 3-erase; 4-diffuse; 5-flood; 6-clear; 7-all;
		//			  11-posterize
		//hr_mode (hidden removal): 0-none; 1-ortho; 2-persp
		void draw_mask(int type, int paint_mode, int hr_mode,
			double ini_thresh, double gm_falloff, double scl_falloff,
			double scl_translate, double w2d, double bins, bool ortho);
		//generate the labeling assuming the mask is already generated
		//type: 0-initialization; 1-maximum intensity filtering
		//mode: 0-normal; 1-posterized
		void draw_label(int type, int mode, double thresh, double gm_falloff);

		//calculation
		void calculate(int type, VolumeRenderer* vr_a, VolumeRenderer* vr_b);

		//return
		void return_volume();//return the data volume
		void return_mask();//return the mask volume
		void return_label(); //return the label volume

		//mask and label
		int get_ml_mode() {return ml_mode_;}
		void set_ml_mode(int mode) {ml_mode_ = mode;}
		//bool get_mask() {return mask_;}
		//void set_mask(bool mask) {mask_ = mask;}
		//bool get_label() {return label_;}
		//void set_label(bool label) {label_ = label;}

		//set noise reduction
		void SetNoiseRed(bool nd) {noise_red_ = nd;}
		bool GetNoiseRed() {return noise_red_;}

		//soft threshold
		static void set_soft_threshold(double val) {sw_ = val;}

		//inversion
		void set_inversion(bool mode) {inv_ = mode;}
		bool get_inversion() {return inv_;}

		//compression
		void set_compression(bool compression) {compression_ = compression;}

		//done loop
		bool get_done_loop(int mode)
		{if (mode>=0 && mode<TEXTURE_RENDER_MODES) return done_loop_[mode]; else return false;}
		void set_done_loop(bool done)
		{for (int i=0; i<TEXTURE_RENDER_MODES; i++) done_loop_[i] = done;}

		friend class MultiVolumeRenderer;

	protected:
		double scalar_scale_;
		double gm_scale_;
		//transfer function properties
		double gamma3d_;
		double gm_thresh_;
		double offset_;
		double lo_thresh_;
		double hi_thresh_;
		Color color_;
		Color mask_color_;
		double mask_thresh_;
		double alpha_;
		//shading
		bool shading_;
		double ambient_, diffuse_, specular_, shine_;
		//colormap mode
		int colormap_mode_;//0-normal; 1-rainbow; 2-depth
		double colormap_low_value_;
		double colormap_hi_value_;
		//solid
		bool solid_;
		//interpolation
		bool interpolate_;
		//adaptive
		bool adaptive_;
		//planes
		vector<Plane *> planes_;
		//depth peel
		int depth_peel_;
		//hi quality
		bool hiqual_;
		//segmentation
		int ml_mode_;	//0-normal, 1-render with mask, 2-render with mask excluded,
						//3-random color with label, 4-random color with label+mask
		bool mask_;
		bool label_;
		//smooth filter
		static ImgShaderFactory m_img_shader_factory;

		//noise reduction
		bool noise_red_;
		//scale factor and filter sizes
		double sfactor_;
		double filter_size_min_;
		double filter_size_max_;
		double filter_size_shp_;

		//soft threshold
		static double sw_;

		//inversion
		bool inv_;

		//compression
		bool compression_;

		//done loop
		bool done_loop_[TEXTURE_RENDER_MODES];

		//calculating scaling factor, etc
		double CalcScaleFactor(double w, double h, double tex_w, double tex_h, double zoom);
		//calculate the filter sizes
		double CalcFilterSize(int type, 
			double w, double h, double tex_w, double tex_h, 
			double zoom, double sf);	//type - 1:min filter
										//		 2:max filter
										//		 3:sharpening
	};

} // End namespace FLIVR

#endif 
