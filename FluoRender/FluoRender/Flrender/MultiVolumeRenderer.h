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

#ifndef MultiVolumeRenderer_h
#define MultiVolumeRenderer_h

#include <BBox.h>
#include <glm/mat4x4.hpp>
#include <vector>

namespace fluo
{
	class Ray;
}
namespace flvr
{
	class Framebuffer;
	class VertexArray;
	class VolumeRenderer;
	class TextureBrick;
	enum class RenderMode : int;
	class MultiVolumeRenderer
	{
	public:
		MultiVolumeRenderer();
		MultiVolumeRenderer(MultiVolumeRenderer&);
		virtual ~MultiVolumeRenderer();

		void set_cur_framebuffer(GLuint cur_framebuffer)
		{
			cur_framebuffer_ = cur_framebuffer;
		}

		//set viewport
		void set_viewport(GLint vp[4]);

		//set clear color
		void set_clear_color(GLfloat clear_color[4])
		{
			memcpy(clear_color_, clear_color, sizeof(GLfloat) * 4);
		}

		//mode and sampling rate
		void set_mode(const RenderMode& mode);
		void set_interactive_mode(bool mode);
		int get_slice_num();

		//manages volume renderers for rendering
		void add_vr(VolumeRenderer* vr);
		void clear_vr();
		int get_vr_num();

		void draw(bool draw_wireframe_p,
			bool interactive_mode_p, 
			bool orthographic_p,
			bool intp);

		void draw_wireframe(bool adaptive, bool orthographic_p);
		void draw_volume(bool adaptive, bool interactive_mode_p, bool orthographic_p, bool intp);

		double num_slices_to_rate(int slices);

		//depth peeling
		int get_depth_peel() {return depth_peel_;}
		void set_depth_peel(int dp) {depth_peel_ = dp;}

		//blend slices
		bool get_blend_slices() {return blend_slices_;}
		void set_blend_slices(bool bs) {blend_slices_ = bs;}

		//set noise reduction
		void SetNoiseRed(bool nd) {noise_red_ = nd;}
		bool GetNoiseRed() {return noise_red_;}

		//set matrices
		//void set_matrices(glm::mat4 &mv_mat2, glm::mat4 &proj_mat, glm::mat4 &tex_mat);

	private:
		//viewport
		GLint vp_[4];
		//clear color
		GLfloat clear_color_[4];

		//volume renderer list
		std::vector<VolumeRenderer*> vr_list_;

		//unified matrices
		//glm::mat4 mv_mat2_;
		//glm::mat4 proj_mat_;
		//glm::mat4 tex_mat_;

		//mode and quality control
		RenderMode mode_;
		int depth_peel_;
		int blend_num_bits_;
		bool blend_slices_;

		//saved framebuffer
		GLuint cur_framebuffer_;

		//scale factor
		bool noise_red_;

		//bounding box of all volumes
		fluo::BBox bbox_;
		//total resolution
		fluo::Vector res_;

		//sample rate etc
		bool imode_;
		int num_slices_;

		//light position
		fluo::Vector light_pos_;

		//vertex array for rendering slices
		VertexArray* va_slices_;
		void draw_polygons_vol(
			TextureBrick* b, double rate,
			std::vector<float>& vertex,
			std::vector<uint32_t>& index,
			std::vector<uint32_t>& size,
			fluo::Ray &view_ray,
			int bi, bool orthographic_p,
			int w, int h, bool intp,
			int quota_bricks_chan,
			Framebuffer* blend_buffer);

		//find out combined bricks in interactive mode
		std::vector<TextureBrick*> *get_combined_bricks(
			fluo::Point& center, fluo::Ray& view, bool is_orthographic = false);
	};

} // End namespace flvr

#endif 
