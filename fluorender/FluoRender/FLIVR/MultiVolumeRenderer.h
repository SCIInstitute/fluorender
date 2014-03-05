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

#ifndef SLIVR_MultiVolumeRenderer_h
#define SLIVR_MultiVolumeRenderer_h

#include "GL/glew.h"
#include "BBox.h"
#include "Plane.h"
#include "Texture.h"
#include "VolumeRenderer.h"
#include <vector>

using namespace std;

namespace FLIVR
{
	class MultiVolumeRenderer
	{
	public:
		MultiVolumeRenderer();
		MultiVolumeRenderer(MultiVolumeRenderer&);
		virtual ~MultiVolumeRenderer();

		//mode and sampling rate
		void set_mode(TextureRenderer::RenderMode mode);
		void set_sampling_rate(double rate);
		void set_interactive_rate(double irate);
		void set_interactive_mode(bool mode);
		void set_adaptive(bool b);
		int get_slice_num();

		//manages volume renderers for rendering
		void add_vr(VolumeRenderer* vr);
		void clear_vr();
		int get_vr_num();

		void draw(bool draw_wireframe_p, 
			bool interactive_mode_p, 
			bool orthographic_p,
			double zoom, bool intp);

		void draw_wireframe(bool orthographic_p);
		void draw_volume(bool interactive_mode_p, bool orthographic_p, double zoom, bool intp);

		double num_slices_to_rate(int slices);

		//depth peeling
		int get_depth_peel() {return depth_peel_;}
		void set_depth_peel(int dp) {depth_peel_ = dp;}

		//blend slices
		bool get_blend_slices() {return blend_slices_;}
		void set_blend_slices(bool bs) {blend_slices_ = bs;}

		//resize fbo texture
		void resize();

		//set noise reduction
		void SetNoiseRed(bool nd) {noise_red_ = nd;}
		bool GetNoiseRed() {return noise_red_;}

		//colormap mode
		void set_colormap_mode(int mode) {colormap_mode_ = mode;}

		//soft threshold
		static void set_soft_threshold(double val) {sw_ = val;}

	private:
		//volume renderer list
		vector<VolumeRenderer*> vr_list_;

		//mode and quality control
		TextureRenderer::RenderMode mode_;
		int depth_peel_;
		bool hiqual_;
		int blend_num_bits_;
		bool blend_slices_;

		//blend frame buffers
		bool blend_framebuffer_resize_;
		GLuint blend_framebuffer_;
		GLuint blend_tex_id_;
		//2nd buffer for multiple filtering
		bool filter_buffer_resize_;
		GLuint filter_buffer_;
		GLuint filter_tex_id_;

		//scale factor
		bool noise_red_;
		double sfactor_;
		double filter_size_min_;
		double filter_size_max_;
		double filter_size_shp_;

		//bounding box of all volumes
		BBox bbox_;
		//total resolution
		Vector res_;

		//sample rate etc
		bool imode_;
		bool adaptive_;
		double irate_;
		double sampling_rate_;
		int num_slices_;

		//light position
		Vector light_pos_;

		//colormap mode
		int colormap_mode_;//0-normal; 1-rainbow; 2-depth

		//soft threshold
		static double sw_;

		void draw_polygons_vol(
			vector<double>& vertex,
			vector<double>& texcoord,
			vector<int>& poly,
			bool fog,
			Ray &view_ray,
			FragmentProgram *shader,
			int bi, bool orthographic_p,
			int w, int h, bool intp,
			int quota_bricks_chan);

		//find out combined bricks in interactive mode
		vector<TextureBrick*> *get_combined_bricks(
			Point& center, Ray& view, bool is_orthographic = false);
	};

} // End namespace FLIVR

#endif 
