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

#ifndef TextureRenderer_h
#define TextureRenderer_h

#include <Point.h>
#include <stdint.h>
#include <glm/glm.hpp>
#include <memory>
#ifdef _DARWIN
#include <OpenGL/OpenGL.h>
#endif

#ifndef __glew_h__
typedef unsigned int GLenum;
typedef unsigned int GLuint;
typedef int GLint;
#endif // !__glew_h__
#ifndef GL_UNSIGNED_BYTE
#define GL_UNSIGNED_BYTE 0x1401 // The actual value of GL_UNSIGNED_BYTE
#endif
#ifndef GL_LINEAR
#define GL_LINEAR 0x2601
#endif
#ifndef GL_NEAREST
#define GL_NEAREST 0x2600
#endif

namespace fluo
{
	class BBox;
	class Ray;
}
namespace flvr
{
	class CacheQueue;
	class Texture;
	class TextureBrick;
	//a simple fixed-length fifo sequence
	class BrickQueue
	{
	public:
		BrickQueue(int limit) :
			m_queue(0),
			m_limit(limit),
			m_pos(0)
		{
			if (m_limit >= 0)
				m_queue = new int[m_limit]();
		}
		~BrickQueue()
		{
			if (m_queue)
				delete[]m_queue;
		}

		int GetLimit()
		{
			return m_limit;
		}
		int Push(int value)
		{
			if (m_queue)
			{
				m_queue[m_pos] = value;
				if (m_pos < m_limit - 1)
					m_pos++;
				else
					m_pos = 0;
				return 1;
			}
			else
				return 0;
		}
		int Get(int index)
		{
			if (index >= 0 && index < m_limit)
				return m_queue[m_pos + index < m_limit ? m_pos + index : m_pos + index - m_limit];
			else
				return 0;
		}
		int GetLast()
		{
			return m_queue[m_pos == 0 ? m_limit - 1 : m_pos - 1];
		}

	private:
		int *m_queue;
		int m_limit;
		int m_pos;
	};

	class ShaderProgram;
	class VolShaderFactory;
	class SegShaderFactory;
	class VolCalShaderFactory;
	class VolKernelFactory;
	class FramebufferManager;
	class ImgShaderFactory;
	class VertexArray;
	class VertexArrayManager;
	class LightFieldShaderFactory;

	struct TexParam
	{
		int nx, ny, nz, nb;
		unsigned int id;
		TextureBrick *brick;
		int comp;
		int time;
		GLenum textype;
		bool delayed_del;
		TexParam() :
			nx(0), ny(0), nz(0), nb(0),
			id(0), brick(0), comp(0), time(0),
			textype(GL_UNSIGNED_BYTE),
			delayed_del(false)
		{}
		TexParam(int c, int t,
			int x, int y, int z, int b,
			GLenum f,
			unsigned int i) :
			nx(x), ny(y), nz(z), nb(b),
			id(i), brick(0), comp(c), time(t),
			textype(f),
			delayed_del(false)
		{}

		bool Match(TextureBrick* bk,
			int c, int t, int x, int y, int z, int b, GLenum f)
		{
			return id != 0 &&
				brick == bk &&
				comp == c &&
				time == t &&
				nx == x &&
				ny == y &&
				nz == z &&
				nb == b &&
				textype == f;
		}
	};

#define PALETTE_W 256
#define PALETTE_H 256
#define PALETTE_SIZE (PALETTE_W*PALETTE_H)
#define PALETTE_ELEM_COMP 4

	enum class RenderMode : int
	{
		RENDER_MODE_NONE,
		RENDER_MODE_OVER,
		RENDER_MODE_MIP,
		RENDER_MODE_SLICE
	};

	class TextureRenderer
	{
	public:
		TextureRenderer();
		TextureRenderer(const TextureRenderer&);
		virtual ~TextureRenderer();

		void set_cur_framebuffer(GLuint cur_framebuffer)
		{ cur_framebuffer_ = cur_framebuffer; }

		//set the texture for rendering
		void set_texture(const std::shared_ptr<Texture>& tex);
		void reset_texture();

		//set blending bits. b>8 means 32bit blending
		void set_blend_num_bits(int b);

		//clear the opengl textures from the texture pool
		static void clear_tex_pool();
		void clear_tex_current();
		void clear_tex_mask(bool skip=true);
		void clear_tex_label();

		//set the 2d texture mask for segmentation
		void set_2d_mask(GLuint id);
		//set 2d weight map for segmentation
		void set_2d_weight(GLuint weight1, GLuint weight2);

		//set the 2d texture depth map for rendering shadows
		void set_2d_dmap(GLuint id);

		// Tests the bounding box against the current MODELVIEW and
		// PROJECTION matrices to determine if it is within the viewport.
		// Returns true if it is visible.
		bool test_against_view(const fluo::BBox &bbox, bool persp=false);

		//set cache queue
		void set_cache_queue(const std::shared_ptr<CacheQueue>& cq) { cache_queue_ = cq; }

		//interactive modes
		void set_interactive_mode(bool mode) { imode_ = mode; }
		bool get_adaptive();
		double get_data_size();//size in mb
		int get_size_type();//0:normal; 1:small; 2:large

		//sample rate
		void set_sample_rate(double rate) { sample_rate_ = rate; }
		double get_sample_rate();

		//main(cpu) memory limit
		static void set_mainmem_buf_size(double val) { mainmem_buf_size_ = val; }
		static double get_mainmem_buf_size() { return mainmem_buf_size_; }
		//callback checks if the swapping is done
		static void set_update_loop() {start_update_loop_ = true;
		done_update_loop_ = false;}
		static void reset_update_loop() {start_update_loop_ = false;}
		static bool get_done_update_loop() {return done_update_loop_;}
		static void set_done_update_loop() { done_update_loop_ = true; }
		static bool get_start_update_loop() {return start_update_loop_;}
		static bool get_done_current_chan() {return done_current_chan_;}
		static void reset_done_current_chan()
		{done_current_chan_ = false; cur_chan_brick_num_ = 0;
		save_final_buffer_ = true; clear_chan_buffer_ = true;}
		static int get_cur_chan_brick_num() {return cur_chan_brick_num_;}
		static void set_total_brick_num(int num) {total_brick_num_ = num; cur_brick_num_ = 0;}
		static int get_total_brick_num() {return total_brick_num_;}
		static void set_clear_chan_buffer() { clear_chan_buffer_ = true; }
		static void reset_clear_chan_buffer() {clear_chan_buffer_ = false;}
		static bool get_clear_chan_buffer() {return clear_chan_buffer_;}
		static void reset_save_final_buffer() {save_final_buffer_ = false;}
		static bool get_save_final_buffer() {return save_final_buffer_;}
		//set start time
		static void set_st_time(unsigned long time) {st_time_ = time;}
		static unsigned long get_st_time() {return st_time_;}
		static unsigned long get_up_time();
		static void set_consumed_time(unsigned long time) {consumed_time_ = time;}
		static unsigned long get_consumed_time() {return consumed_time_;}
		//set corrected up time according to mouse speed
		static void set_cor_up_time(int speed);
		static unsigned long get_cor_up_time() {return cor_up_time_;}
		//interactive mdoe
		static void set_interactive(bool mode) {interactive_ = mode;}
		static bool get_interactive() {return interactive_;}
		//number of bricks rendered before time is up
		static void reset_finished_bricks();
		static int get_finished_bricks() {return finished_bricks_;}
		static void push_quota_brick(int bricks);
		static int get_est_bricks(int mode, int init=0);
		static int get_queue_last() {return brick_queue_.GetLast();}
		//quota bricks in interactive mode
		static void set_quota_bricks(int quota) {quota_bricks_ = quota;}
		static int get_quota_bricks() {return quota_bricks_;}
		//current channel
		void set_quota_bricks_chan(int quota) {quota_bricks_chan_ = quota;}
		int get_quota_bricks_chan() {return quota_bricks_chan_;}
		//quota center
		static void set_qutoa_center(fluo::Point &point) {quota_center_ = point;}
		static fluo::Point& get_quota_center() { return quota_center_; }
		//load on main thread
		static void set_load_on_main_thread(bool val) { load_on_main_thread_ = val; }
		static bool get_load_on_main_thread() { return load_on_main_thread_; }

	public:
		struct BrickDist
		{
			unsigned int index;    //index of the brick in current tex pool
			TextureBrick* brick;  //a brick
			double dist;      //distance to another brick
		};
		std::weak_ptr<Texture> tex_;
		RenderMode mode_;
		double sample_rate_;
		size_t num_slices_;
		bool imode_;

		//saved framebuffer
		GLuint cur_framebuffer_;

		//2d mask texture
		GLuint tex_2d_mask_;
		//2d weight map
		GLuint tex_2d_weight1_;  //after tone mapping
		GLuint tex_2d_weight2_;  //before tone mapping
		//2d depth map texture
		GLuint tex_2d_dmap_;

		int blend_num_bits_;

#ifdef _DARWIN
		static CGLContextObj gl_context_;
#endif
		//memory management
		static int active_view_;
		static double mainmem_buf_size_;
		static double available_mainmem_buf_size_;
		static std::vector<TexParam> tex_pool_;
		static bool start_update_loop_;
		static bool done_update_loop_;
		static bool done_current_chan_;
		static int total_brick_num_;
		static int cur_brick_num_;
		static int cur_chan_brick_num_;
		static bool clear_chan_buffer_;
		static bool save_final_buffer_;
		//timer for rendering
		static bool maximize_uptime_;
		static unsigned long st_time_;
		static unsigned long cor_up_time_;//mouse movement corrected up time
		static unsigned long consumed_time_;
		//interactive mode
		static bool interactive_;
		//number of rendered blocks before time is up
		static int finished_bricks_;
		static BrickQueue brick_queue_;
		//quota in interactive mode
		static int quota_bricks_;
		int quota_bricks_chan_;//for current channel
		//center point of the quota
		static fluo::Point quota_center_;
		//threading
		static bool load_on_main_thread_;
		struct LoadedBrick {
			bool swapped;
			TextureBrick *brk;
			double size;
		};
		static std::vector<LoadedBrick> loadedbrks;
		static int del_id;

		//for view testing
		float mvmat_[16];
		float prmat_[16];

		//opengl matrices
		glm::mat4 m_mv_mat = glm::mat4(1.0);
		glm::mat4 m_mv_tex_scl_mat = glm::mat4(1.0);//mv matrix with texture scaling
		glm::mat4 m_proj_mat = glm::mat4(1.0);
		glm::mat4 m_tex_mat = glm::mat4(1.0);

		//renderer manages vertex array objects
		VertexArray* va_slices_;
		VertexArray* va_wirefm_;

		//cache queue for getting neighbors of time
		std::weak_ptr<CacheQueue> cache_queue_;

		//compute view
		fluo::Ray compute_view();
		fluo::Ray compute_snapview(double snap);
		double compute_rate_scale(const fluo::Vector& v);

		//brick distance sort
		static bool brick_sort(const BrickDist& bd1, const BrickDist& bd2);
		//check and swap memory
		void check_swap_memory(TextureBrick* brick, int c);
		//load texture bricks for drawing
		//unit:assigned unit, c:channel
		GLint load_brick(TextureBrick* brick, GLint filter=GL_LINEAR, bool compression=false, int unit=0, int mode=0, int toffset = 0);
		//load the texture for volume mask into texture pool
		GLint load_brick_mask(TextureBrick* brick, GLint filter=GL_NEAREST, bool compression=false, int unit=0);
		//load the texture for volume labeling into texture pool
		GLint load_brick_label(TextureBrick* brick);
		void load_texture(void* tex_data, unsigned int nx, unsigned int ny, unsigned int nz, unsigned int nb, unsigned int sx, unsigned int sy, GLenum tex_type, GLenum format);
		void release_texture(int unit, GLenum target);

		//draw slices of the volume
		void draw_view_quad(double d=0.0);

		//slices
		void draw_polygons(std::vector<float>& vertex, 
			std::vector<uint32_t>& poly);
		void draw_polygons_wireframe(std::vector<float>& vertex,
			std::vector<uint32_t>& index,
			std::vector<uint32_t>& size);

		//bind 2d mask for segmentation
		void bind_2d_mask();
		//bind 2d weight map for segmentation
		void bind_2d_weight();
		//bind 2d depth map for rendering shadows
		void bind_2d_dmap();

		void rearrangeLoadedBrkVec();
		void clear_brick_buf();
	};
} // end namespace flvr

#endif // TextureRenderer_h
