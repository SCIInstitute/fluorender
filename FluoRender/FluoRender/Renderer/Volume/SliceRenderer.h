/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2026 Scientific Computing and Imaging Institute,
University of Utah.


Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#ifndef SliceRenderer_h
#define SliceRenderer_h

#include <memory>
#include <vector>

namespace flvr
{

	class VolumePyramid;
	class Brick;
	class BrickTextureManager;
	class CacheQueue;
	class VertexArray;

	class SliceRenderer
	{
	public:
		SliceRenderer();
		virtual ~SliceRenderer();

		//------------------------------------------------------------------------
		// volumes
		//------------------------------------------------------------------------

		void set_volume(
			const std::shared_ptr<VolumePyramid>& volume);

		void set_volumes(
			const std::vector<std::shared_ptr<VolumePyramid>>& volumes);

		void add_volume(
			const std::shared_ptr<VolumePyramid>& volume);

		void clear_volumes();

		size_t volume_count() const
		{
			return volumes_.size();
		}

		const std::vector<std::shared_ptr<VolumePyramid>>&
			volumes() const
		{
			return volumes_;
		}

		//------------------------------------------------------------------------
		// rendering
		//------------------------------------------------------------------------

		void set_mode(RenderMode mode)
		{
			render_mode_ = mode;
		}

		RenderMode mode() const
		{
			return render_mode_;
		}

		void set_sample_rate(double rate)
		{
			sample_rate_ = rate;
		}

		double sample_rate() const
		{
			return sample_rate_;
		}

		void set_interactive_mode(bool mode)
		{
			interactive_mode_ = mode;
		}

		bool interactive_mode() const
		{
			return interactive_mode_;
		}

		//------------------------------------------------------------------------
		// auxiliary textures
		//------------------------------------------------------------------------

		void set_2d_mask(GLuint id);
		void set_2d_weight(GLuint weight1, GLuint weight2);
		void set_2d_dmap(GLuint id);

		void bind_2d_mask();
		void bind_2d_weight();
		void bind_2d_dmap();

		//------------------------------------------------------------------------
		// cache / residency
		//------------------------------------------------------------------------

		void set_cache_queue(
			const std::shared_ptr<CacheQueue>& queue);

		void clear_current_textures();
		void clear_mask_textures(bool skip = true);
		void clear_label_textures();

		//------------------------------------------------------------------------
		// view
		//------------------------------------------------------------------------

		bool test_against_view(
			const fluo::BBox& bbox,
			bool perspective = false);

		fluo::Ray compute_view();
		fluo::Ray compute_snapview(double snap);

		//------------------------------------------------------------------------
		// rendering helpers
		//------------------------------------------------------------------------

		void draw_view_quad(double d = 0.0);

		void draw_polygons(
			std::vector<float>& vertex,
			std::vector<uint32_t>& index);

		void draw_polygons_wireframe(
			std::vector<float>& vertex,
			std::vector<uint32_t>& index,
			std::vector<uint32_t>& size);

	protected:
		//------------------------------------------------------------------------
		// brick / texture loading
		//------------------------------------------------------------------------

		void check_swap_memory(
			const std::shared_ptr<Brick>& brick,
			CompType component) const;

		GLint load_brick(
			const std::shared_ptr<Brick>& brick,
			GLint filter = GL_LINEAR,
			bool compression = false,
			int unit = 0,
			int mode = 0,
			int texture_offset = 0) const;

		GLint load_brick_mask(
			const std::shared_ptr<Brick>& brick,
			GLint filter = GL_NEAREST,
			bool compression = false,
			int unit = 0) const;

		GLint load_brick_label(
			const std::shared_ptr<Brick>& brick) const;

		void release_texture(
			int unit,
			GLenum target) const;

	protected:
		//--------------------------------------------------------------------
		// renderer state
		//--------------------------------------------------------------------

		std::vector<std::shared_ptr<VolumePyramid>> volumes_;

		RenderMode render_mode_ =
			RenderMode::RenderMode_Normal;

		double sample_rate_ = 1.0;

		size_t num_slices_ = 0;

		bool interactive_mode_ = false;

		//--------------------------------------------------------------------
		// auxiliary textures
		//--------------------------------------------------------------------

		GLuint tex_2d_mask_ = 0;

		GLuint tex_2d_weight1_ = 0;
		GLuint tex_2d_weight2_ = 0;

		GLuint tex_2d_dmap_ = 0;

		//--------------------------------------------------------------------
		// matrices
		//--------------------------------------------------------------------

		float mvmat_[16];
		float prmat_[16];

		glm::mat4 mv_mat_ = glm::mat4(1.0f);
		glm::mat4 mv_tex_scale_mat_ = glm::mat4(1.0f);
		glm::mat4 proj_mat_ = glm::mat4(1.0f);
		glm::mat4 tex_mat_ = glm::mat4(1.0f);

		//--------------------------------------------------------------------
		// geometry
		//--------------------------------------------------------------------

		std::shared_ptr<VertexArray> va_slices_;
		std::shared_ptr<VertexArray> va_wireframe_;

		//--------------------------------------------------------------------
		// cache
		//--------------------------------------------------------------------

		std::weak_ptr<CacheQueue> cache_queue_;

		//--------------------------------------------------------------------
		// texture residency
		//--------------------------------------------------------------------

		std::shared_ptr<BrickTextureManager>
			brick_texture_manager_;
	};

} // namespace flvr

#endif// SliceRenderer_h