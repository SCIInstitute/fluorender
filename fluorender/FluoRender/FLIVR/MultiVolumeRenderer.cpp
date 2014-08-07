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

#include "MultiVolumeRenderer.h"
#include "VolShader.h"
#include "ShaderProgram.h"
#include "../compatibility.h"
#include <algorithm>

namespace FLIVR
{
#ifdef _WIN32
#undef min
#undef max
#endif

   double MultiVolumeRenderer::sw_ = 0.0;

   MultiVolumeRenderer::MultiVolumeRenderer()
      : mode_(TextureRenderer::MODE_OVER),
      depth_peel_(0),
      hiqual_(true),
      blend_num_bits_(32),
      blend_slices_(false),
      blend_framebuffer_resize_(false),
      blend_framebuffer_(0),
      blend_tex_id_(0),
      filter_buffer_resize_(false),
      filter_buffer_(0),
      filter_tex_id_(0),
      noise_red_(false),
      sfactor_(1.0),
      filter_size_min_(0.0),
      filter_size_max_(0.0),
      filter_size_shp_(0.0),
      imode_(false),
      adaptive_(true),
      irate_(1.0),
      sampling_rate_(1.0),
      num_slices_(0),
      colormap_mode_(0)
   {
   }

   MultiVolumeRenderer::MultiVolumeRenderer(MultiVolumeRenderer& copy)
      : mode_(copy.mode_),
      depth_peel_(copy.depth_peel_),
      hiqual_(copy.hiqual_),
      blend_num_bits_(copy.blend_num_bits_),
      blend_slices_(copy.blend_slices_),
      blend_framebuffer_resize_(false),
      blend_framebuffer_(0),
      blend_tex_id_(0),
      filter_buffer_resize_(false),
      filter_buffer_(0),
      filter_tex_id_(0),
      noise_red_(false),
      sfactor_(1.0),
      filter_size_min_(0.0),
      filter_size_max_(0.0),
      filter_size_shp_(0.0),
      imode_(copy.imode_),
      adaptive_(copy.adaptive_),
      irate_(copy.irate_),
      sampling_rate_(copy.sampling_rate_),
      num_slices_(0),
      colormap_mode_(copy.colormap_mode_)
   {
   }

   MultiVolumeRenderer::~MultiVolumeRenderer()
   {
   }

   //mode and sampling rate
   void MultiVolumeRenderer::set_mode(TextureRenderer::RenderMode mode)
   {
      mode_ = mode;
   }

   void MultiVolumeRenderer::set_sampling_rate(double rate)
   {
      sampling_rate_ = rate;
      //irate_ = rate>1.0 ? max(rate / 2.0, 1.0) : rate;
      irate_ = max(rate / 2.0, 0.1);
   }

   void MultiVolumeRenderer::set_interactive_rate(double rate)
   {
      irate_ = rate;
   }

   void MultiVolumeRenderer::set_interactive_mode(bool mode)
   {
      imode_ = mode;
   }

   void MultiVolumeRenderer::set_adaptive(bool b)
   {
      adaptive_ = b;
   }

   int MultiVolumeRenderer::get_slice_num()
   {
      return num_slices_;
   }

   //manages volume renderers for rendering
   void MultiVolumeRenderer::add_vr(VolumeRenderer* vr)
   {
      for (unsigned int i=0; i<vr_list_.size(); i++)
      {
         if (vr_list_[i] == vr)
            return;
      }

      vr_list_.push_back(vr);

      if (vr && vr->tex_)
      {
         bbox_.extend(*(vr->tex_->bbox()));
         res_ = Max(res_, vr->tex_->res());
      }
   }

   void MultiVolumeRenderer::clear_vr()
   {
      vr_list_.clear();
      bbox_.reset();
      res_ = Vector(0.0);
   }

   int MultiVolumeRenderer::get_vr_num()
   {
      return int(vr_list_.size());
   }

   //draw
   void MultiVolumeRenderer::draw(bool draw_wireframe_p,
         bool interactive_mode_p,
         bool orthographic_p,
         double zoom, bool intp)
   {
      draw_volume(interactive_mode_p, orthographic_p, zoom, intp);
      if(draw_wireframe_p)
         draw_wireframe(orthographic_p);
   }

   void MultiVolumeRenderer::draw_volume(bool interactive_mode_p, bool orthographic_p, double zoom, bool intp)
   {
      if (get_vr_num()<=0 || !(vr_list_[0]))
         return;

      Ray view_ray = vr_list_[0]->compute_view();

      set_interactive_mode(adaptive_ && interactive_mode_p);

      // Set sampling rate based on interaction.
      double rate = imode_ ? irate_ : sampling_rate_;
      Vector diag = bbox_.diagonal();
      Vector cell_diag(diag.x()/res_.x(),
            diag.y()/res_.y(),
            diag.z()/res_.z());
      double dt = cell_diag.length()/
         vr_list_[0]->compute_rate_scale()/rate;
      num_slices_ = (int)(diag.length()/dt);

      vector<double> vertex;
      vector<double> texcoord;
      vector<int> size;
      vertex.reserve(num_slices_*6);
      texcoord.reserve(num_slices_*6);
      size.reserve(num_slices_*6);

      //--------------------------------------------------------------------------
      bool use_shading = vr_list_[0]->shading_;
      GLboolean use_fog = glIsEnabled(GL_FOG) && colormap_mode_!=2;
      GLfloat clear_color[4];
      glGetFloatv(GL_COLOR_CLEAR_VALUE, clear_color);
      GLint vp[4];
      glGetIntegerv(GL_VIEWPORT, vp);

      // set up blending
      glEnable(GL_BLEND);
      switch(mode_)
      {
      case TextureRenderer::MODE_OVER:
         glBlendEquation(GL_FUNC_ADD);
         if (TextureRenderer::get_update_order() == 0)
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
         else if (TextureRenderer::get_update_order() == 1)
            glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE);
         break;
      case TextureRenderer::MODE_MIP:
         glBlendEquation(GL_MAX);
         glBlendFunc(GL_ONE, GL_ONE);
         break;
      default:
         break;
      }

      // Cache this value to reset, in case another framebuffer is active,
      // as it is in the case of saving an image from the viewer.
      GLint cur_framebuffer_id;
      glGetIntegerv(GL_FRAMEBUFFER_BINDING, &cur_framebuffer_id);
      GLint cur_draw_buffer;
      glGetIntegerv(GL_DRAW_BUFFER, &cur_draw_buffer);
      GLint cur_read_buffer;
      glGetIntegerv(GL_READ_BUFFER, &cur_read_buffer);

      int w = vp[2];
      int h = vp[3];
      int w2 = w;
      int h2 = h;

      double sf = vr_list_[0]->CalcScaleFactor(w, h, res_.x(), res_.y(), zoom);
      if (fabs(sf-sfactor_)>0.05)
      {
         sfactor_ = sf;
         blend_framebuffer_resize_ = true;
         filter_buffer_resize_ = true;
         vr_list_[0]->blend_framebuffer_resize_ = true;
      }
      else if (sf==1.0 && sfactor_!=1.0)
      {
         sfactor_ = sf;
         blend_framebuffer_resize_ = true;
         filter_buffer_resize_ = true;
         vr_list_[0]->blend_framebuffer_resize_ = true;
      }

      w2 = int(w*sfactor_+0.5);
      h2 = int(h*sfactor_+0.5);

      if(blend_num_bits_ > 8)
      {
         if (!glIsFramebuffer(blend_framebuffer_))
         {
            glGenFramebuffers(1, &blend_framebuffer_);
            glGenTextures(1, &blend_tex_id_);

            glBindFramebuffer(GL_FRAMEBUFFER, blend_framebuffer_);

            // Initialize texture color renderbuffer
            glBindTexture(GL_TEXTURE_2D, blend_tex_id_);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w2, h2, 0,
                  GL_RGBA, GL_FLOAT, NULL);//GL_RGBA16F
            glFramebufferTexture2D(GL_FRAMEBUFFER,
                  GL_COLOR_ATTACHMENT0,
                  GL_TEXTURE_2D, blend_tex_id_, 0);
         }

         if (blend_framebuffer_resize_)
         {
            // resize texture color renderbuffer
            glBindTexture(GL_TEXTURE_2D, blend_tex_id_);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w2, h2, 0,
                  GL_RGBA, GL_FLOAT, NULL);//GL_RGBA16F

            blend_framebuffer_resize_ = false;
         }

         glBindTexture(GL_TEXTURE_2D, 0);
         glDisable(GL_TEXTURE_2D);

         glBindFramebuffer(GL_DRAW_FRAMEBUFFER, blend_framebuffer_);

         glClearColor(clear_color[0], clear_color[1], clear_color[2], clear_color[3]);
         glClear(GL_COLOR_BUFFER_BIT);

         glViewport(vp[0], vp[1], w2, h2);
      }

      //disable depth buffer writing
      glDepthMask(GL_FALSE);

      //--------------------------------------------------------------------------
      // enable data texture unit 0
      glActiveTexture(GL_TEXTURE0);

      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
      glEnable(GL_TEXTURE_3D);

      //--------------------------------------------------------------------------
      // Set up shaders
      FragmentProgram* shader = 0;
      shader = VolumeRenderer::vol_shader_factory_.shader(
            vr_list_[0]->tex_->nc(),
            use_shading, use_fog!=0,
            depth_peel_, true,
            hiqual_, 0,
            colormap_mode_, false);
      if (shader)
      {
         if (!shader->valid())
            shader->create();
         shader->bind();
      }

      //setup depth peeling
      if (depth_peel_ || colormap_mode_ == 2)
         shader->setLocalParam(7, 1.0/double(w2), 1.0/double(h2), 0.0, 0.0);

      //--------------------------------------------------------------------------
      // render bricks
      // Set up transform
      Transform *tform = vr_list_[0]->tex_->transform();
      double mvmat[16];
      tform->get_trans(mvmat);
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      glMultMatrixd(mvmat);
      float matrix[16];

      int quota_bricks_chan = vr_list_[0]->get_quota_bricks_chan();
      vector<TextureBrick*> *bs = 0;
      FLIVR::Point pt = TextureRenderer::quota_center_;
      if (TextureRenderer::mem_swap_ &&
            TextureRenderer::interactive_)
         //bs = vr_list_[0]->tex_->get_closest_bricks(
         //TextureRenderer::quota_center_,
         //quota_bricks_chan, false,
         //view_ray, orthographic_p);
         bs = get_combined_bricks(
               pt, view_ray, orthographic_p);
      else
         bs = vr_list_[0]->tex_->get_sorted_bricks(
               view_ray, orthographic_p);

      if (bs)
      {
         for (unsigned int i=0; i < bs->size(); i++)
         {
            if (TextureRenderer::mem_swap_)
            {
               uint32_t rn_time = GET_TICK_COUNT();
               if (rn_time - TextureRenderer::st_time_ > TextureRenderer::get_up_time())
                  break;
            }

            TextureBrick* b = (*bs)[i];
            if (TextureRenderer::mem_swap_ &&
                  TextureRenderer::start_update_loop_ &&
                  !TextureRenderer::done_update_loop_)
            {
               if (b->drawn(0))
                  continue;
            }

            if (!vr_list_[0]->test_against_view(b->bbox()))// Clip against view
            {
               if (TextureRenderer::mem_swap_ &&
                     TextureRenderer::start_update_loop_ &&
                     !TextureRenderer::done_update_loop_)
               {
                  for (unsigned int j=0; j<vr_list_.size(); j++)
                  {
                     vector<TextureBrick*>* bs_tmp = 0;
                     if (TextureRenderer::interactive_)
                        //bs_tmp = vr_list_[j]->tex_->get_closest_bricks(
                        //TextureRenderer::quota_center_,
                        //quota_bricks_chan, false,
                        //view_ray, orthographic_p);
                        bs_tmp = vr_list_[j]->tex_->get_quota_bricks();
                     else
                        bs_tmp = vr_list_[j]->tex_->get_sorted_bricks(
                              view_ray, orthographic_p);
                     if (!(*bs_tmp)[i]->drawn(0))
                     {
                        (*bs_tmp)[i]->set_drawn(0, true);
                        //TextureRenderer::cur_brick_num_++;
                     }
                  }
               }
               continue;
            }
            vertex.clear();
            texcoord.clear();
            size.clear();
            b->compute_polygons(view_ray, dt, vertex, texcoord, size);
            if (vertex.size() == 0) { continue; }
            shader->setLocalParam(4, 1.0/b->nx(), 1.0/b->ny(), 1.0/b->nz(), 1.0/rate);

            //for brick transformation
            BBox bbox = b->bbox();
            matrix[0] = float(bbox.max().x()-bbox.min().x());
            matrix[1] = 0.0f;
            matrix[2] = 0.0f;
            matrix[3] = 0.0f;
            matrix[4] = 0.0f;
            matrix[5] = float(bbox.max().y()-bbox.min().y());
            matrix[6] = 0.0f;
            matrix[7] = 0.0f;
            matrix[8] = 0.0f;
            matrix[9] = 0.0f;
            matrix[10] = float(bbox.max().z()-bbox.min().z());
            matrix[11] = 0.0f;
            matrix[12] = float(bbox.min().x());
            matrix[13] = float(bbox.min().y());
            matrix[14] = float(bbox.min().z());
            matrix[15] = 1.0f;
            shader->setLocalParamMatrix(2, matrix);

            draw_polygons_vol(vertex, texcoord, size, use_fog!=0, view_ray,
                  shader, i, orthographic_p, w2, h2, intp, quota_bricks_chan);
         }
      }


		if (TextureRenderer::mem_swap_ &&
			TextureRenderer::cur_brick_num_ == TextureRenderer::total_brick_num_)
		{
			TextureRenderer::done_update_loop_ = true;
			TextureRenderer::clear_chan_buffer_ = true;
		}

      // Undo transform.
      glPopMatrix();

      //enable depth buffer writing
      glDepthMask(GL_TRUE);

      // Release shader.
      if (shader && shader->valid())
         shader->release();

      //release texture
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_3D, 0);
      glDisable(GL_TEXTURE_3D);

      //reset blending
      glBlendEquation(GL_FUNC_ADD);
      if (TextureRenderer::get_update_order() == 0)
         glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
      else if (TextureRenderer::get_update_order() == 1)
         glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE);
      glDisable(GL_BLEND);

      //output
      if (blend_num_bits_ > 8)
      {
         //states
         GLboolean depth_test = glIsEnabled(GL_DEPTH_TEST);
         GLboolean lighting = glIsEnabled(GL_LIGHTING);
         GLboolean cull_face = glIsEnabled(GL_CULL_FACE);
         glDisable(GL_DEPTH_TEST);
         glDisable(GL_LIGHTING);
         glDisable(GL_CULL_FACE);
         glActiveTexture(GL_TEXTURE0);
         glEnable(GL_TEXTURE_2D);

         //transformations
         glMatrixMode(GL_PROJECTION);
         glPushMatrix();
         glLoadIdentity();
         glMatrixMode(GL_MODELVIEW);
         glPushMatrix();
         glLoadIdentity();

         FragmentProgram* img_shader = 0;

         if (noise_red_ && colormap_mode_!=2)
         {
            //FILTERING/////////////////////////////////////////////////////////////////
            if (!glIsTexture(filter_tex_id_))
            {
               glGenTextures(1, &filter_tex_id_);
               glBindTexture(GL_TEXTURE_2D, filter_tex_id_);
               glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
               glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
               glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
               glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
               glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w2, h2, 0,
                     GL_RGBA, GL_FLOAT, NULL);//GL_RGBA16F
            }
            if (!glIsFramebuffer(filter_buffer_))
            {
               glGenFramebuffers(1, &filter_buffer_);
               glBindFramebuffer(GL_FRAMEBUFFER, filter_buffer_);
               glFramebufferTexture2D(GL_FRAMEBUFFER,
                     GL_COLOR_ATTACHMENT0,
                     GL_TEXTURE_2D, filter_tex_id_, 0);
            }
            if (filter_buffer_resize_)
            {
               glBindTexture(GL_TEXTURE_2D, filter_tex_id_);
               glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w2, h2, 0,
                     GL_RGBA, GL_FLOAT, NULL);//GL_RGBA16F
               filter_buffer_resize_ = false;
            }

            glBindFramebuffer(GL_FRAMEBUFFER, filter_buffer_);

            glBindTexture(GL_TEXTURE_2D, blend_tex_id_);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

            img_shader = vr_list_[0]->
               m_img_shader_factory.shader(IMG_SHDR_FILTER_BLUR);
            if (img_shader)
            {
               if (!img_shader->valid())
               {
                  img_shader->create();
               }
               img_shader->bind();
            }
            filter_size_min_ = vr_list_[0]->
					CalcFilterSize(4, w, h, res_.x(), res_.y(), zoom, sfactor_);
				img_shader->setLocalParam(0, filter_size_min_/w2, filter_size_min_/h2, 1.0/w2, 1.0/h2);
            glBegin(GL_QUADS);
            {
               glTexCoord2f(0.0, 0.0);
               glVertex3f(-1, -1, 0.0);
               glTexCoord2f(1.0, 0.0);
               glVertex3f(1, -1, 0.0);
               glTexCoord2f(1.0, 1.0);
               glVertex3f(1, 1, 0.0);
               glTexCoord2f(0.0, 1.0);
               glVertex3f(-1, 1, 0.0);
            }
            glEnd();
            if (img_shader && img_shader->valid())
               img_shader->release();
            ///////////////////////////////////////////////////////////////////////////
         }

         //go back to normal
         glBindFramebuffer(GL_FRAMEBUFFER, cur_framebuffer_id);
         glDrawBuffer(cur_draw_buffer);
         glReadBuffer(cur_read_buffer);

         glViewport(vp[0], vp[1], vp[2], vp[3]);


			if (noise_red_ && colormap_mode_!=2)
				glBindTexture(GL_TEXTURE_2D, filter_tex_id_);
			else
				glBindTexture(GL_TEXTURE_2D, blend_tex_id_);
         glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
         glEnable(GL_BLEND);
         if (TextureRenderer::get_update_order() == 0)
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
         else if (TextureRenderer::get_update_order() == 1)
            glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE);

         if (noise_red_ && colormap_mode_!=2)
         {
            img_shader = vr_list_[0]->
               m_img_shader_factory.shader(IMG_SHDR_FILTER_SHARPEN);
            if (img_shader)
            {
               if (!img_shader->valid())
               {
                  img_shader->create();
               }
               img_shader->bind();
            }
            filter_size_shp_ = vr_list_[0]->
               CalcFilterSize(3, w, h, res_.x(), res_.y(), zoom, sfactor_);
            img_shader->setLocalParam(0, filter_size_shp_/w, filter_size_shp_/h, 0.0, 0.0);
         }

         glBegin(GL_QUADS);
         {
            glTexCoord2f(0.0, 0.0);
            glVertex3f(-1, -1, 0.0);
            glTexCoord2f(1.0, 0.0);
            glVertex3f(1, -1, 0.0);
            glTexCoord2f(1.0, 1.0);
            glVertex3f(1, 1, 0.0);
            glTexCoord2f(0.0, 1.0);
            glVertex3f(-1, 1, 0.0);
         }
         glEnd();

         if (noise_red_ && colormap_mode_!=2)
         {
            if (img_shader && img_shader->valid())
               img_shader->release();
         }

         if (depth_test) glEnable(GL_DEPTH_TEST);
         if (lighting) glEnable(GL_LIGHTING);
         if (cull_face) glEnable(GL_CULL_FACE);

         glMatrixMode(GL_PROJECTION);
         glPopMatrix();
         glMatrixMode(GL_MODELVIEW);
         glPopMatrix();
         glBindTexture(GL_TEXTURE_2D, 0);
         glDisable(GL_TEXTURE_2D);
         glDisable(GL_BLEND);
      }
   }

   void MultiVolumeRenderer::draw_polygons_vol(
         vector<double>& vertex,
         vector<double>& texcoord,
         vector<int>& poly,
         bool fog,
         Ray &view_ray,
         FragmentProgram* shader,
         int bi, bool orthographic_p,
         int w, int h, bool intp,
         int quota_bricks_chan)
   {
      //check vr_list size
      if (vr_list_.size() <= 0)
         return;

      GLfloat clear_color[4];
      glGetFloatv(GL_COLOR_CLEAR_VALUE, clear_color);
      GLint vp[4];
      glGetIntegerv(GL_VIEWPORT, vp);

      double mvmat[16];
      if(fog)
      {
         glGetDoublev(GL_MODELVIEW_MATRIX, mvmat);
      }

      //save original buffer
      GLint cur_framebuffer_id;
      glGetIntegerv(GL_FRAMEBUFFER_BINDING, &cur_framebuffer_id);
      GLint cur_draw_buffer;
      glGetIntegerv(GL_DRAW_BUFFER, &cur_draw_buffer);
      GLint cur_read_buffer;
      glGetIntegerv(GL_READ_BUFFER, &cur_read_buffer);
      GLuint *blend_fbo = &(vr_list_[0]->blend_framebuffer_);
      GLuint *blend_tex = &(vr_list_[0]->blend_tex_id_);

      if (blend_slices_ && colormap_mode_!=2)
      {
         //check blend buffer
         if (!glIsFramebuffer(*blend_fbo))
         {
            glGenFramebuffers(1, blend_fbo);
            if (!glIsTexture(*blend_tex))
               glGenTextures(1, blend_tex);
            glBindFramebuffer(GL_FRAMEBUFFER, *blend_fbo);
            // Initialize texture color renderbuffer
            glBindTexture(GL_TEXTURE_2D, *blend_tex);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w, h, 0,
                  GL_RGBA, GL_FLOAT, NULL);//GL_RGBA16F
            glFramebufferTexture2D(GL_FRAMEBUFFER,
                  GL_COLOR_ATTACHMENT0,
                  GL_TEXTURE_2D, *blend_tex, 0);
         }
         if (vr_list_[0]->blend_framebuffer_resize_)
         {
            glBindTexture(GL_TEXTURE_2D, *blend_tex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w, h, 0,
                  GL_RGBA, GL_FLOAT, NULL);//GL_RGBA16F
            vr_list_[0]->blend_framebuffer_resize_ = false;
         }
      }

      for(unsigned int i=0, k=0; i<poly.size(); i++)
      {
         double mat[16];
         glGetDoublev(GL_MODELVIEW_MATRIX, mat);
         Transform mv;
         mv.set_trans(mat);

         if (blend_slices_ && colormap_mode_!=2)
         {
            //set blend buffer
            glBindFramebuffer(GL_FRAMEBUFFER, *blend_fbo);
            glClearColor(clear_color[0], clear_color[1], clear_color[2], clear_color[3]);
            glClear(GL_COLOR_BUFFER_BIT);
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);

            glUseProgram(shader->id());
            glEnable(GL_TEXTURE_3D);
            glDisable(GL_TEXTURE_2D);
         }

         //draw a single slice
         for (int tn=0 ; tn<(int)vr_list_.size() ; tn++)
         {
            // set shader parameters
            light_pos_ = view_ray.direction();
            light_pos_.safe_normalize();
            shader->setLocalParam(0, light_pos_.x(), light_pos_.y(), light_pos_.z(), vr_list_[tn]->alpha_);
            shader->setLocalParam(1, 2.0 - vr_list_[tn]->ambient_,
                  vr_list_[tn]->shading_?vr_list_[tn]->diffuse_:0.0,
                  vr_list_[tn]->specular_,
                  vr_list_[tn]->shine_);
            shader->setLocalParam(2, vr_list_[tn]->scalar_scale_,
                  vr_list_[tn]->gm_scale_,
                  vr_list_[tn]->lo_thresh_,
                  vr_list_[tn]->hi_thresh_);
            shader->setLocalParam(3, 1.0/vr_list_[tn]->gamma3d_,
                  vr_list_[tn]->gm_thresh_,
                  vr_list_[tn]->offset_,
                  sw_);
            double spcx, spcy, spcz;
            vr_list_[tn]->tex_->get_spacings(spcx, spcy, spcz);
            shader->setLocalParam(5, spcx, spcy, spcz, 1.0);
            //switch (vr_list_[tn]->colormap_mode_)
            //{
            //case 0://normal
            shader->setLocalParam(6, vr_list_[tn]->color_.r(),
                  vr_list_[tn]->color_.g(),
                  vr_list_[tn]->color_.b(), 0.0);
            //  break;
            //case 1://colormap
            //  shader->setLocalParam(6, vr_list_[tn]->colormap_low_value_,
            //    vr_list_[tn]->colormap_hi_value_,
            //    vr_list_[tn]->colormap_hi_value_-vr_list_[tn]->colormap_low_value_, 0.0);
            //  break;
            //}

            double abcd[4];
            vr_list_[tn]->planes_[0]->get(abcd);
            shader->setLocalParam(10, abcd[0], abcd[1], abcd[2], abcd[3]);
            vr_list_[tn]->planes_[1]->get(abcd);
            shader->setLocalParam(11, abcd[0], abcd[1], abcd[2], abcd[3]);
            vr_list_[tn]->planes_[2]->get(abcd);
            shader->setLocalParam(12, abcd[0], abcd[1], abcd[2], abcd[3]);
            vr_list_[tn]->planes_[3]->get(abcd);
            shader->setLocalParam(13, abcd[0], abcd[1], abcd[2], abcd[3]);
            vr_list_[tn]->planes_[4]->get(abcd);
            shader->setLocalParam(14, abcd[0], abcd[1], abcd[2], abcd[3]);
            vr_list_[tn]->planes_[5]->get(abcd);
            shader->setLocalParam(15, abcd[0], abcd[1], abcd[2], abcd[3]);

            //bind depth texture for rendering shadows
            if (colormap_mode_ == 2)
            {
               if (blend_num_bits_ > 8)
                  vr_list_[tn]->tex_2d_dmap_ = blend_tex_id_;
               vr_list_[tn]->bind_2d_dmap();
            }

            vector<TextureBrick*> *bs = 0;
            if (TextureRenderer::mem_swap_ &&
                  TextureRenderer::interactive_)
               //bs = vr_list_[tn]->tex_->get_closest_bricks(
               //TextureRenderer::quota_center_,
               //quota_bricks_chan, false,
               //view_ray, orthographic_p);
               bs = vr_list_[tn]->tex_->get_quota_bricks();
            else
               bs = vr_list_[tn]->tex_->get_sorted_bricks(
                     view_ray, orthographic_p);
            if (!bs) break;
            if (bi>=(int)bs->size()) break;

            if ((*bs)[bi]->get_priority()>0)
            {
               if (TextureRenderer::mem_swap_ &&
                     TextureRenderer::start_update_loop_ &&
                     !TextureRenderer::done_update_loop_)
               {
                  if (!(*bs)[bi]->drawn(0))
                     (*bs)[bi]->set_drawn(0, true);
               }
               continue;
            }

            GLint filter;
            if (intp)
               filter = GL_LINEAR;
            else
               filter = GL_NEAREST;
            vr_list_[tn]->load_brick(0, 0, bs, bi, filter, vr_list_[tn]->compression_);
            glBegin(GL_POLYGON);
            {
               for(int j=0; j<poly[i]; j++)
               {
                  double* t = &texcoord[(k+j)*3];
                  double* v = &vertex[(k+j)*3];
                  if (glMultiTexCoord3f)
                  {
                     glMultiTexCoord3d(GL_TEXTURE0, t[0], t[1], t[2]);
                     if(fog)
                     {
                        double vz = mvmat[2]*v[0] + mvmat[6]*v[1] + mvmat[10]*v[2] + mvmat[14];
                        glMultiTexCoord3d(GL_TEXTURE1, -vz, 0.0, 0.0);
                     }
                  }
                  glVertex3d(v[0], v[1], v[2]);
               }
            }
            glEnd();

            //release depth texture for rendering shadows
			//release depth texture for rendering shadows
			if (colormap_mode_ == 2)
				vr_list_[tn]->release_texture(4, GL_TEXTURE_2D);

			if (TextureRenderer::mem_swap_ && i==0)
				TextureRenderer::finished_bricks_++;
         }
         k += poly[i];

         if (blend_slices_ && colormap_mode_!=2)
         {
            glUseProgram(0);
            glActiveTexture(GL_TEXTURE0);
            glEnable(GL_TEXTURE_2D);
            glDisable(GL_TEXTURE_3D);

            //set buffer back
            glBindFramebuffer(GL_FRAMEBUFFER, cur_framebuffer_id);
            glDrawBuffer(cur_draw_buffer);
            glReadBuffer(cur_read_buffer);
            glBindTexture(GL_TEXTURE_2D, *blend_tex);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
            //transformations
            glMatrixMode(GL_PROJECTION);
            glPushMatrix();
            glLoadIdentity();
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();
            //blend
            if (TextureRenderer::get_update_order() == 0)
               glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            else if (TextureRenderer::get_update_order() == 1)
               glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE);
            //draw
            glBegin(GL_QUADS);
            {
               glTexCoord2d(0.0, 0.0);
               glVertex3d(-1, -1, 0.0);
               glTexCoord2d(1.0, 0.0);
               glVertex3d(1, -1, 0.0);
               glTexCoord2d(1.0, 1.0);
               glVertex3d(1, 1, 0.0);
               glTexCoord2d(0.0, 1.0);
               glVertex3d(-1, 1, 0.0);
            }
            glEnd();
            glMatrixMode(GL_PROJECTION);
            glPopMatrix();
            glMatrixMode(GL_MODELVIEW);
            glPopMatrix();
         }
      }

      //if (TextureRenderer::mem_swap_)
      //  TextureRenderer::finished_bricks_ += (int)vr_list_.size();
   }

   vector<TextureBrick*> *MultiVolumeRenderer::get_combined_bricks(
         Point& center, Ray& view, bool is_orthographic)
   {
      if (!vr_list_.size())
         return 0;

      if (!vr_list_[0]->tex_->get_sort_bricks())
         return vr_list_[0]->tex_->get_quota_bricks();

      size_t i, j, k;
      vector<TextureBrick*>* bs;
      vector<TextureBrick*>* bs0;
      vector<TextureBrick*>* result;
      Point brick_center;
      double d;

      for (i=0; i<vr_list_.size(); i++)
      {
         //sort each brick list based on distance to center
         bs = vr_list_[i]->tex_->get_bricks();
         for (j=0; j<bs->size(); j++)
         {
            brick_center = (*bs)[j]->bbox().center();
            d = (brick_center - center).length();
            (*bs)[j]->set_d(d);
         }
         std::sort((*bs).begin(), (*bs).end(), TextureBrick::sort_dsc);

         //assign indecis so that bricks can be selected later
         for (j=0; j<bs->size(); j++)
            (*bs)[j]->set_ind(j);
      }

      //generate quota brick list for vr0
      bs0 = vr_list_[0]->tex_->get_bricks();
      result = vr_list_[0]->tex_->get_quota_bricks();
      result->clear();
      int quota = 0;
      int count;
      TextureBrick* pb;
      size_t ind;
      bool found;
      for (i=0; i<vr_list_.size(); i++)
      {
         //insert nonduplicated bricks into result list
         bs = vr_list_[i]->tex_->get_bricks();
         quota = vr_list_[i]->get_quota_bricks_chan();
         //quota = quota/2+1;
         count = 0;
         for (j=0; j<bs->size(); j++)
         {
            pb = (*bs)[j];
            if (pb->get_priority()>0)
               continue;
            ind = pb->get_ind();
            found = false;
            for (k=0; k<result->size(); k++)
            {
               if (ind == (*result)[k]->get_ind())
               {
                  found = true;
                  break;
               }
            }
            if (!found)
            {
               result->push_back((*bs0)[ind]);
               count++;
               if (count == quota)
                  break;
            }
         }
      }
      //reorder result
      for (i = 0; i < result->size(); i++)
      {
         Point minp((*result)[i]->bbox().min());
         Point maxp((*result)[i]->bbox().max());
         Vector diag((*result)[i]->bbox().diagonal());
         minp += diag / 1000.;
         maxp -= diag / 1000.;
         Point corner[8];
         corner[0] = minp;
         corner[1] = Point(minp.x(), minp.y(), maxp.z());
         corner[2] = Point(minp.x(), maxp.y(), minp.z());
         corner[3] = Point(minp.x(), maxp.y(), maxp.z());
         corner[4] = Point(maxp.x(), minp.y(), minp.z());
         corner[5] = Point(maxp.x(), minp.y(), maxp.z());
         corner[6] = Point(maxp.x(), maxp.y(), minp.z());
         corner[7] = maxp;
         double d = 0.0;
         for (unsigned int c = 0; c < 8; c++)
         {
            double dd;
            if (is_orthographic)
            {
               // orthographic: sort bricks based on distance to the view plane
               dd = Dot(corner[c], view.direction());
            }
            else
            {
               // perspective: sort bricks based on distance to the eye point
               dd = (corner[c] - view.origin()).length();
            }
            if (c == 0 || dd < d) d = dd;
         }
         (*result)[i]->set_d(d);
      }
      if (TextureRenderer::get_update_order() == 0)
         std::sort((*result).begin(), (*result).end(), TextureBrick::sort_asc);
      else if (TextureRenderer::get_update_order() == 1)
         std::sort((*result).begin(), (*result).end(), TextureBrick::sort_dsc);
      vr_list_[0]->tex_->reset_sort_bricks();

      //duplicate result into other quota-bricks
      for (i=1; i<vr_list_.size(); i++)
      {
         bs0 = vr_list_[i]->tex_->get_bricks();
         bs = vr_list_[i]->tex_->get_quota_bricks();
         bs->clear();

         for (j=0; j<result->size(); j++)
         {
            ind = (*result)[j]->get_ind();
            bs->push_back((*bs0)[ind]);
         }
         vr_list_[i]->tex_->reset_sort_bricks();
      }

      return result;
   }

   void MultiVolumeRenderer::draw_wireframe(bool orthographic_p)
   {
      if (get_vr_num()<=0)
         return;

      Ray view_ray = vr_list_[0]->compute_view();

      // Set sampling rate based on interaction.
      double rate = imode_ ? irate_ : sampling_rate_;
      Vector diag = bbox_.diagonal();
      Vector cell_diag(diag.x()/res_.x(),
            diag.y()/res_.y(),
            diag.z()/res_.z());
      double dt = cell_diag.length()/
         vr_list_[0]->compute_rate_scale()/rate;
      num_slices_ = (int)(diag.length()/dt);

      vector<double> vertex;
      vector<double> texcoord;
      vector<int> size;
      vertex.reserve(num_slices_*6);
      texcoord.reserve(num_slices_*6);
      size.reserve(num_slices_*6);

      //--------------------------------------------------------------------------
      // render bricks
      // Set up transform
      Transform *tform = vr_list_[0]->tex_->transform();
      double mvmat[16];
      tform->get_trans(mvmat);
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      glMultMatrixd(mvmat);

      glEnable(GL_DEPTH_TEST);
      GLboolean lighting = glIsEnabled(GL_LIGHTING);
      glDisable(GL_LIGHTING);
      glDisable(GL_TEXTURE_3D);
      glDisable(GL_TEXTURE_2D);
      glDisable(GL_FOG);

      vector<TextureBrick*> *bs = vr_list_[0]->tex_->get_sorted_bricks(view_ray, orthographic_p);

      if (bs)
      {
         for (unsigned int i=0; i < bs->size(); i++)
         {
            glColor4d(0.8*(i+1.0)/bs->size(), 0.8*(i+1.0)/bs->size(), 0.8, 1.0);

            TextureBrick* b = (*bs)[i];
            if (!vr_list_[0]->test_against_view(b->bbox())) continue; // Clip against view.

            Point ptmin = b->bbox().min();
            Point ptmax = b->bbox().max();
            Point &pmin(ptmin);
            Point &pmax(ptmax);
            Point corner[8];
            corner[0] = pmin;
            corner[1] = Point(pmin.x(), pmin.y(), pmax.z());
            corner[2] = Point(pmin.x(), pmax.y(), pmin.z());
            corner[3] = Point(pmin.x(), pmax.y(), pmax.z());
            corner[4] = Point(pmax.x(), pmin.y(), pmin.z());
            corner[5] = Point(pmax.x(), pmin.y(), pmax.z());
            corner[6] = Point(pmax.x(), pmax.y(), pmin.z());
            corner[7] = pmax;

            glBegin(GL_LINES);
            {
               for(int i=0; i<4; i++) {
                  glVertex3d(corner[i].x(), corner[i].y(), corner[i].z());
                  glVertex3d(corner[i+4].x(), corner[i+4].y(), corner[i+4].z());
               }
            }
            glEnd();
            glBegin(GL_LINE_LOOP);
            {
               glVertex3d(corner[0].x(), corner[0].y(), corner[0].z());
               glVertex3d(corner[1].x(), corner[1].y(), corner[1].z());
               glVertex3d(corner[3].x(), corner[3].y(), corner[3].z());
               glVertex3d(corner[2].x(), corner[2].y(), corner[2].z());
            }
            glEnd();
            glBegin(GL_LINE_LOOP);
            {
               glVertex3d(corner[4].x(), corner[4].y(), corner[4].z());
               glVertex3d(corner[5].x(), corner[5].y(), corner[5].z());
               glVertex3d(corner[7].x(), corner[7].y(), corner[7].z());
               glVertex3d(corner[6].x(), corner[6].y(), corner[6].z());
            }
            glEnd();

            glColor4d(0.4, 0.4, 0.4, 1.0);

            vertex.clear();
            texcoord.clear();
            size.clear();

            // Scale out dt such that the slices are artificially further apart.
            b->compute_polygons(view_ray, dt * 10, vertex, texcoord, size);
            vr_list_[0]->draw_polygons_wireframe(vertex, texcoord, size, false);
         }
      }

      if(lighting) glEnable(GL_LIGHTING);
      glMatrixMode(GL_MODELVIEW);
      glPopMatrix();
   }

   double MultiVolumeRenderer::num_slices_to_rate(int num_slices)
   {
      if (!bbox_.valid())
         return 1.0;
      Vector diag = bbox_.diagonal();
      Vector cell_diag(diag.x()/*/tex_->nx()*/,
            diag.y()/*/tex_->ny()*/,
            diag.z()/*/tex_->nz()*/);
      double dt = diag.length() / num_slices;
      double rate = cell_diag.length() / dt;

      return rate;
   }

   void MultiVolumeRenderer::resize()
   {
      blend_framebuffer_resize_ = true;
   }

} // namespace FLIVR
