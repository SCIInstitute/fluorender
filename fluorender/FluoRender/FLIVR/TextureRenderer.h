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

#ifndef TextureRenderer_h
#define TextureRenderer_h

#include <nrrd.h>
#include "TextureBrick.h"
#include "Texture.h"

namespace FLIVR
{
   //a simple fixed-length fifo sequence
   class BrickQueue
   {
      public:
         BrickQueue(int limit):
            m_queue(0),
            m_limit(limit),
            m_pos(0)
      {
         if (m_limit >=0)
         {
            m_queue = new int[m_limit];
            memset(m_queue, 0, m_limit*sizeof(int));
         }
      }
         ~BrickQueue()
         {
            if (m_queue)
               delete []m_queue;
         }

         int GetLimit()
         {return m_limit;}
         int Push(int value)
         {
            if (m_queue)
            {
               m_queue[m_pos] = value;
               if (m_pos < m_limit-1)
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
            if (index>=0 && index<m_limit)
               return m_queue[m_pos+index<m_limit?m_pos+index:m_pos+index-m_limit];
            else
               return 0;
         }
         int GetLast()
         {
            return m_queue[m_pos==0?m_limit-1:m_pos-1];
         }

      private:
         int *m_queue;
         int m_limit;
         int m_pos;
   };

   class FragmentProgram;
   class VolShaderFactory;
   class SegShaderFactory;
   class VolCalShaderFactory;
   class VolKernelFactory;

   struct TexParam
   {
      int nx, ny, nz, nb;
      unsigned int id;
      TextureBrick *brick;
      int comp;
      GLenum textype;
      bool delayed_del;
      TexParam() :
         nx(0), ny(0), nz(0), nb(0),
         id(0), brick(0), comp(0),
         textype(GL_UNSIGNED_BYTE),
         delayed_del(false)
      {}
      TexParam(int c, int x,
            int y, int z,
            int b, GLenum f,
            unsigned int i) :
          nx(x), ny(y),
         nz(z), nb(b), id(i),
         brick(0), comp(c), textype(f),
         delayed_del(false)
      {}
   };

   class TextureRenderer
   {
      public:
         enum RenderMode
         {
            MODE_NONE,
            MODE_OVER,
            MODE_MIP,
            MODE_SLICE
         };

         TextureRenderer(Texture* tex);
         TextureRenderer(const TextureRenderer&);
         virtual ~TextureRenderer();

         //set the texture for rendering
         void set_texture(Texture* tex);

         //set blending bits. b>8 means 32bit blending
         void set_blend_num_bits(int b);

         //clear the opengl textures from the texture pool
         void clear_tex_pool();

         //resize the fbo texture
         void resize();

         //set the 2d texture mask for segmentation
         void set_2d_mask(GLuint id);
         //set 2d weight map for segmentation
         void set_2d_weight(GLuint weight1, GLuint weight2);

         //set the 2d texture depth map for rendering shadows
         void set_2d_dmap(GLuint id);

         // Tests the bounding box against the current MODELVIEW and
         // PROJECTION matrices to determine if it is within the viewport.
         // Returns true if it is visible.
         bool test_against_view(const BBox &bbox, bool use_ex=false);

		 //exposed load brick
		 GLint load_brick_cl(int c, vector<TextureBrick*> *b, int i);

         //memory swap
         static void set_mem_swap(bool val) {mem_swap_ = val;}
         static bool get_mem_swap() {return mem_swap_;}
         //use memory limit (otherwise get available memory from graphics card)
         static void set_use_mem_limit(bool val) {use_mem_limit_ = val;}
         static bool get_use_mem_limit() {return use_mem_limit_;}
         //memory limit
         static void set_mem_limit(double val) {mem_limit_ = val;}
         static double get_mem_limit() {return mem_limit_;}
         //available memory
         static void set_available_mem(double val) {available_mem_ = val;}
         static double get_available_mem() {return available_mem_;}
         //large data size
         static void set_large_data_size(double val) {large_data_size_ = val;}
         static double get_large_data_size() {return large_data_size_;}
         //force brick size
         static void set_force_brick_size(int val) {force_brick_size_ = val;}
         static int get_force_brick_size() {return force_brick_size_;}
         //callback checks if the swapping is done
         static void set_update_loop() {start_update_loop_ = true;
            done_update_loop_ = false;}
            static void reset_update_loop() {start_update_loop_ = false;}
            static bool get_done_update_loop() {return done_update_loop_;}
            static bool get_start_update_loop() {return start_update_loop_;}
            static bool get_done_current_chan() {return done_current_chan_;}
            static void reset_done_current_chan()
            {done_current_chan_ = false; cur_chan_brick_num_ = 0;
               save_final_buffer_ = true; clear_chan_buffer_ = true;}
               static int get_cur_chan_brick_num() {return cur_chan_brick_num_;}
               static void set_total_brick_num(int num) {total_brick_num_ = num; cur_brick_num_ = 0;}
               static int get_total_brick_num() {return total_brick_num_;}
               static void reset_clear_chan_buffer() {clear_chan_buffer_ = false;}
               static bool get_clear_chan_buffer() {return clear_chan_buffer_;}
               static void reset_save_final_buffer() {save_final_buffer_ = false;}
               static bool get_save_final_buffer() {return save_final_buffer_;}
               //set start time
               static void set_st_time(unsigned long time) {st_time_ = time;}
               static unsigned long get_st_time() {return st_time_;}
               static void set_up_time(unsigned long time) {up_time_ = time;}
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
               static int get_finished_bricks_max();
               static int get_est_bricks(int mode);
               static int get_queue_last() {return brick_queue_.GetLast();}
               //quota bricks in interactive mode
               static void set_quota_bricks(int quota) {quota_bricks_ = quota;}
               static int get_quota_bricks() {return quota_bricks_;}
               //current channel
               void set_quota_bricks_chan(int quota) {quota_bricks_chan_ = quota;}
               int get_quota_bricks_chan() {return quota_bricks_chan_;}
               //quota center
               static void set_qutoa_center(Point &point) {quota_center_ = point;}
               //update order
               static void set_update_order(int val) {update_order_ = val;}
               static int get_update_order() {return update_order_;}

			   //kernel for calculation
			   static VolKernelFactory vol_kernel_factory_;

	  public:
               struct BrickDist
               {
                  unsigned int index;    //index of the brick in current tex pool
                  TextureBrick* brick;  //a brick
                  double dist;      //distance to another brick
               };
               Texture *tex_;
               RenderMode mode_;
               double sampling_rate_;
               int num_slices_;
               double irate_;
               bool imode_;

               //blend frame buffer for output
               bool blend_framebuffer_resize_;
               GLuint blend_framebuffer_;
               GLuint blend_tex_id_;
               //2nd buffer for multiple filtering
               bool filter_buffer_resize_;
               GLuint filter_buffer_;
               GLuint filter_tex_id_;

               //sahder for volume rendering
               static VolShaderFactory vol_shader_factory_;
               //shader for segmentation
               static SegShaderFactory seg_shader_factory_;
               //shader for calculation
               static VolCalShaderFactory cal_shader_factory_;

               //3d frame buffer object for mask
               GLuint fbo_mask_;
               //3d frame buffer object for label
               GLuint fbo_label_;
               //2d mask texture
               GLuint tex_2d_mask_;
               //2d weight map
               GLuint tex_2d_weight1_;  //after tone mapping
               GLuint tex_2d_weight2_;  //before tone mapping
               //2d depth map texture
               GLuint tex_2d_dmap_;

               int blend_num_bits_;
               bool clear_pool_;

               //memory management
               static bool mem_swap_;
               static bool use_mem_limit_;
               static double mem_limit_;
               static double available_mem_;
               static double large_data_size_;
               static int force_brick_size_;
               static vector<TexParam> tex_pool_;
               static bool start_update_loop_;
               static bool done_update_loop_;
               static bool done_current_chan_;
               static int total_brick_num_;
               static int cur_brick_num_;
               static int cur_chan_brick_num_;
               static bool clear_chan_buffer_;
               static bool save_final_buffer_;
               //timer for rendering
               static unsigned long st_time_;
               static unsigned long up_time_;
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
               static Point quota_center_;
               //update order
               static int update_order_;

               //for view testing
               double mvmat_[16];
               double prmat_[16];

               //compute view
               Ray compute_view();
			   Ray compute_snapview(double snap);
               double compute_rate_scale(Vector v);

               //brick distance sort
               static bool brick_sort(const BrickDist& bd1, const BrickDist& bd2);
               //check and swap memory
               void check_swap_memory(TextureBrick* brick, int c);
               //load texture bricks for drawing
               //unit:assigned unit, c:channel
               GLint load_brick(int unit, int c, vector<TextureBrick*> *b, int i, GLint filter=GL_LINEAR, bool compression=false, int mode=0);
               //load the texture for volume mask into texture pool
               GLint load_brick_mask(vector<TextureBrick*> *b, int i, GLint filter=GL_NEAREST, bool compression=false, int unit=0);
               //load the texture for volume labeling into texture pool
               GLint load_brick_label(vector<TextureBrick*> *b, int i);
               void release_texture(int unit, GLenum target);

               //draw slices of the volume
               void draw_slices(double d);

               //slices
               void draw_polygons(vector<double>& vertex, vector<double>& texcoord,
                     vector<int>& poly,
                     bool fog,
                     FragmentProgram *shader = 0);
               void draw_polygons_wireframe(vector<double>& vertex, vector<double>& texcoord,
                     vector<int>& poly,
                     bool fog);

               //bind 2d mask for segmentation
               void bind_2d_mask();
               //bind 2d weight map for segmentation
               void bind_2d_weight();
               //bind 2d depth map for rendering shadows
               void bind_2d_dmap();
   };
} // end namespace FLIVR

#endif // TextureRenderer_h
