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
#include <Vector4i.h>
#include <Vector4f.h>
#include <glm/mat4x4.hpp>
#include <vector>
#include <memory>

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
	class MultiVolumeRenderer
	{
	public:
		MultiVolumeRenderer();
		MultiVolumeRenderer(MultiVolumeRenderer&);
		virtual ~MultiVolumeRenderer();

		//set viewport
		void set_viewport(const fluo::Vector4i& vp);

		//set clear color
		void set_clear_color(const fluo::Vector4f& cc)
		{
			clear_color_ = cc;
		}

		//mode and sampling rate
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

		//depth peeling
		int get_depth_peel() {return depth_peel_;}
		void set_depth_peel(int dp) {depth_peel_ = dp;}

		//blend slices
		bool get_blend_slices() {return micro_blend_;}
		void set_blend_slices(bool bs) {micro_blend_ = bs;}

		//set noise reduction
		void SetNoiseRed(bool nd) {noise_red_ = nd;}
		bool GetNoiseRed() {return noise_red_;}

		//set matrices
		//void set_matrices(glm::mat4 &mv_mat2, glm::mat4 &proj_mat, glm::mat4 &tex_mat);

	private:
		//viewport
		fluo::Vector4i viewport_;
		//clear color
		fluo::Vector4f clear_color_;

		//volume renderer list
		std::vector<VolumeRenderer*> vr_list_;

		//unified matrices
		//glm::mat4 mv_mat2_;
		//glm::mat4 proj_mat_;
		//glm::mat4 tex_mat_;

		//mode and quality control
		int depth_peel_;
		bool micro_blend_;

		//scale factor
		bool noise_red_;

		//bounding box of all volumes
		fluo::BBox bbox_;
		//total resolution
		fluo::Vector res_;

		//sample rate etc
		bool interactive_mode_;
		size_t num_slices_;

		//vertex array for rendering slices
		std::shared_ptr<VertexArray> va_slices_;
		void draw_polygons_vol(
			TextureBrick* b, double rate,
			std::vector<float>& vertex,
			std::vector<uint32_t>& index,
			std::vector<uint32_t>& size,
			fluo::Ray &view_ray,
			int bi, bool orthographic_p,
			int w, int h, bool intp,
			int quota_bricks_chan);

		//find out combined bricks in interactive mode
		std::vector<TextureBrick*> *get_combined_bricks(
			fluo::Point& center, fluo::Ray& view, bool is_orthographic = false);
	};

} // End namespace flvr

#endif 
