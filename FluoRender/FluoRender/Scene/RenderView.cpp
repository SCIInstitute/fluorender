﻿/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2025 Scientific Computing and Imaging Institute,
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

#include <GL/glew.h>
#include <RenderView.h>
#include <Global.h>
#include <Names.h>
#include <MainSettings.h>
#include <MainFrame.h>
#include <RenderViewPanel.h>
#include <ClipPlanePanel.h>
#include <ComponentDlg.h>
#include <MeasureDlg.h>
#include <RenderCanvas.h>
#include <BaseXrRenderer.h>
#include <LookingGlassRenderer.h>
#include <ShaderProgram.h>
#include <KernelProgram.h>
#include <Texture.h>
#include <TextureBrick.h>
#include <MultiVolumeRenderer.h>
#include <TextRenderer.h>
#include <StopWatch.hpp>
#include <VolumeLoader.h>
#include <VolumePropPanel.h>
#include <MovieMaker.h>
#include <Cov.h>
#include <FlKey.h>
#include <Interpolator.h>
#include <Ruler.h>
#include <RulerHandler.h>
#include <ScriptProc.h>
#include <VolumeCalculator.h>
#include <VolumeSelector.h>
#include <RulerRenderer.h>
#include <CompAnalyzer.h>
#include <CompSelector.h>
#include <CompEditor.h>
#include <ImgShader.h>
#include <Framebuffer.h>
#include <VertexArray.h>
#include <VolumePoint.h>
#include <SegGrow.h>
#include <VolumeRenderer.h>
#include <TrackMap.h>
#include <base_reader.h>
#include <brkxml_reader.h>
#include <GlobalStates.h>
#include <State.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <tiffio.h>
#if defined(_WIN32) && defined(USE_XINPUT)
#include <XInput/XboxController.h>
#endif

RenderView::RenderView() :
#if defined(_WIN32) && defined(USE_XINPUT)
	m_controller(0),
#endif
	//ruler
	m_cur_ruler(0),
	//capture modes
	m_capture(false),
	m_capture_rotat(false),
	m_capture_rotate_over(false),
	m_capture_tsequ(false),
	m_capture_bat(false),
	m_capture_param(false),
	//begin and end frame
	m_begin_frame(0),
	m_begin_play_frame(0),
	m_end_frame(0),
	//counters
	m_tseq_cur_num(0),
	m_tseq_prv_num(0),
	m_param_cur_num(0),
	m_frame_num_type(0),
	m_total_frames(0),
	//hud
	m_updating(true),
	m_draw_annotations(true),
	m_draw_camctr(false),
	m_camctr_size(2.0),
	m_draw_info(250),
	m_load_update(false),
	m_retain_finalbuffer(false),
	m_draw_frame(false),
	m_draw_legend(false),
	m_draw_colormap(false),
	m_mouse_focus(false),
	m_draw_rulers(true),
	//current volume
	m_cur_vol(0),
	//clipping settings
	m_clip_mask(-1),
	m_clip_mode(2),
	//scale bar
	m_disp_scale_bar(false),
	m_disp_scale_bar_text(false),
	m_sb_length(50),
	m_sb_unit(1),
	m_sb_height(0.0),
	//ortho size
	m_ortho_left(0.0),
	m_ortho_right(1.0),
	m_ortho_bottom(0.0),
	m_ortho_top(1.0),
	//scale factor
	m_scale_factor(1.0),
	m_scale_factor_saved(1.0),
	//scale mode
	m_scale_mode(0),
	//populated lists of data
	m_vd_pop_dirty(true),
	m_md_pop_dirty(true),
	//traces
	m_track_group(0),
	//multivolume
	m_mvr(0),
	//initializaion
	m_initialized(false),
	m_init_view(false),
	//bg color
	m_bg_color(0.0, 0.0, 0.0),
	m_bg_color_inv(1.0, 1.0, 1.0),
	//frustrum
	m_aov(15.0),
	m_near_clip(0.1),
	m_far_clip(1000.0),
	//interpolation
	m_intp(true),
	//interactive modes
	m_int_mode(1),
	m_crop_type(0),
	m_force_clear(false),
	m_interactive(false),
	m_clear_buffer(false),
	m_grow_on(false),
	//resizing
	m_resize(false),
	//brush tools
	m_draw_brush(false),
	m_paint_enable(false),
	m_paint_display(false),
	//paint buffer
	m_clear_paint(true),
	//camera controls
	m_persp(false),
	m_free(false),
	//camera distance
	m_distance(10.0),
	m_init_dist(10.0),
	//camera translation
	m_transx(0.0), m_transy(0.0), m_transz(0.0),
	m_transx_saved(0.0), m_transy_saved(0.0), m_transz_saved(0.0),
	//camera rotation
	m_rotx(0.0), m_roty(0.0), m_rotz(0.0),
	m_rotx_saved(0.0), m_roty_saved(0.0), m_rotz_saved(0.0),
	//camera center
	m_ctrx(0.0), m_ctry(0.0), m_ctrz(0.0),
	m_ctrx_saved(0.0), m_ctry_saved(0.0), m_ctrz_saved(0.0),
	//camera direction
	m_up(0.0, 1.0, 0.0),
	m_head(0.0, 0.0, -1.0),
	//object center
	m_obj_ctrx(0.0), m_obj_ctry(0.0), m_obj_ctrz(0.0),
	//object rotation
	m_obj_rotx(0.0), m_obj_roty(180.0), m_obj_rotz(180.0),
	//flag for using offset values
	m_offset(false),
	//object center offset
	m_obj_ctr_offx(0), m_obj_ctr_offy(0), m_obj_ctr_offz(0),
	//object rotation center offset
	m_obj_rot_ctr_offx(0), m_obj_rot_ctr_offy(0), m_obj_rot_ctr_offz(0),
	//object rotation offset
	m_obj_rot_offx(0), m_obj_rot_offy(0), m_obj_rot_offz(0),
	//object translation
	m_obj_transx(0.0), m_obj_transy(0.0), m_obj_transz(0.0),
	m_obj_transx_saved(0.0), m_obj_transy_saved(0.0), m_obj_transz_saved(0.0),
	m_rot_lock(false),
	//lock cam center
	m_lock_cam_object(false),
	m_pick_lock_center(false),
	//object bounding box
	m_radius(348.0),
	//mouse position
	old_mouse_X(-1), old_mouse_Y(-1),
	prv_mouse_X(-1), prv_mouse_Y(-1),
	//draw controls
	m_draw_all(true),
	m_draw_type(1),
	m_vol_method(VOL_METHOD_SEQ),
	//fog
	m_use_fog(true),
	m_fog_intensity(0.0),
	m_fog_start(0.0),
	m_fog_end(0.0),
	//movie properties
	m_init_angle(0.0),
	m_start_angle(0.0),
	m_end_angle(0.0),
	m_cur_angle(0.0),
	m_step(0.0),
	m_movie_seq(0),
	m_rewind(false),
	m_stages(0),
	m_4d_rewind(false),
	//post image processing
	m_gamma(fluo::Color(1.0, 1.0, 1.0)),
	m_brightness(fluo::Color(1.135, 1.135, 1.135)),
	m_hdr(0.0, 0.0, 0.0),
	m_sync(),
	//volume color map
	m_color_1(fluo::Color(0.0, 0.0, 1.0)),
	m_value_2(0.0),
	m_color_2(fluo::Color(0.0, 0.0, 1.0)),
	m_value_3(0.25),
	m_color_3(fluo::Color(0.0, 1.0, 1.0)),
	m_value_4(0.5),
	m_color_4(fluo::Color(0.0, 1.0, 0.0)),
	m_value_5(0.75),
	m_color_5(fluo::Color(1.0, 1.0, 0.0)),
	m_value_6(1.0),
	m_color_6(fluo::Color(1.0, 0.0, 0.0)),
	m_color_7(fluo::Color(1.0, 0.0, 0.0)),
	//clipping plane rotations
	m_rotx_cl(0), m_roty_cl(0), m_rotz_cl(0),
	//selection
	m_pick(false),
	m_draw_mask(true),
	m_clear_mask(false),
	m_save_mask(false),
	//move view
	m_move_left(false),
	m_move_right(false),
	m_move_up(false),
	m_move_down(false),
	//move time
	m_tseq_forward(false),
	m_tseq_backward(false),
	//move clip
	m_clip_up(false),
	m_clip_down(false),
	//full cell
	m_cell_full(false),
	//link cells
	m_cell_link(false),
	//new cell id
	m_cell_new_id(false),
	//comp include
	m_comp_include(false),
	//comp exclude
	m_comp_exclude(false),
	//nodraw count
	m_nodraw_count(0),
	//pin rotation center
	m_pin_rot_ctr(false),
	m_update_rot_ctr(false),
	m_pin_pick_thresh(0.6),
	m_res_mode(1),
	m_drawing(false),
	m_refresh(false),
	m_keep_enlarge(false),
	m_enlarge(false),
	m_enlarge_scale(1.0),
	//vr settings
	m_lg_initiated(false),
	m_use_openxr(false),
	m_vr_eye_idx(0),
	//size
	m_size(0, 0),
	m_gl_size(0, 0),
	m_dpi_factor(1.0),
	//handle and parents
	m_hWnd(0),
	m_render_view_panel(0),
	m_render_canvas(0)
{
	type = 1;

	m_ruler_list = std::make_unique<flrd::RulerList>();
	m_cell_list = std::make_unique<flrd::CelpList>();
	m_text_renderer = std::make_unique<flvr::TextRenderer>();

	//xbox controller
#if defined(_WIN32) && defined(USE_XINPUT)
	m_controller = new XboxController(1);
	if (m_controller)
	{
		m_control_connected = m_controller->IsConnected();
	}
#endif
}

RenderView::RenderView(RenderView& copy):
#if defined(_WIN32) && defined(USE_XINPUT)
	m_controller(0),
#endif
	//ruler
	m_cur_ruler(copy.m_cur_ruler),
	//capture modes
	m_capture(copy.m_capture),
	m_capture_rotat(copy.m_capture_rotat),
	m_capture_rotate_over(copy.m_capture_rotate_over),
	m_capture_tsequ(copy.m_capture_tsequ),
	m_capture_bat(copy.m_capture_bat),
	m_capture_param(copy.m_capture_param),
	//begin and end frame
	m_begin_frame(copy.m_begin_frame),
	m_begin_play_frame(copy.m_begin_play_frame),
	m_end_frame(copy.m_end_frame),
	//counters
	m_tseq_cur_num(copy.m_tseq_cur_num),
	m_tseq_prv_num(copy.m_tseq_prv_num),
	m_param_cur_num(copy.m_param_cur_num),
	m_frame_num_type(copy.m_frame_num_type),
	m_total_frames(copy.m_total_frames),
	//hud
	m_updating(copy.m_updating),
	m_draw_annotations(copy.m_draw_annotations),
	m_draw_camctr(copy.m_draw_camctr),
	m_camctr_size(copy.m_camctr_size),
	m_draw_info(copy.m_draw_info),
	m_load_update(copy.m_load_update),
	m_retain_finalbuffer(copy.m_retain_finalbuffer),
	m_draw_frame(copy.m_draw_frame),
	m_draw_legend(copy.m_draw_legend),
	m_draw_colormap(copy.m_draw_colormap),
	m_mouse_focus(copy.m_mouse_focus),
	m_draw_rulers(true), //copy m_draw_rulers,
	//current volume
	m_cur_vol(0), //copy m_cur_vol,
	//clipping settings
	m_clip_mask(-1), //copy m_clip_mask,
	m_clip_mode(2), //copy m_clip_mode,
	//scale bar
	m_disp_scale_bar(false), //copy m_disp_scale_bar,
	m_disp_scale_bar_text(false), //copy m_disp_scale_bar_text,
	m_sb_length(50), //copy m_sb_length,
	m_sb_unit(1), //copy m_sb_unit,
	m_sb_height(0.0),
	//ortho size
	m_ortho_left(0.0), //copy m_ortho_left,
	m_ortho_right(1.0), //copy m_ortho_right,
	m_ortho_bottom(0.0), //copy m_ortho_bottom,
	m_ortho_top(1.0), //copy m_ortho_top,
	//scale factor
	m_scale_factor(1.0), //copy m_scale_factor,
	m_scale_factor_saved(1.0),
	//scale mode
	m_scale_mode(0),
	//populated lists of data
	m_vd_pop_dirty(true),
	m_md_pop_dirty(true),
	//traces
	m_track_group(0),
	//multivolume
	m_mvr(0),
	//initializaion
	m_initialized(false),
	m_init_view(false),
	//bg color
	m_bg_color(copy.m_bg_color),
	m_bg_color_inv(copy.m_bg_color_inv),
	//frustrum
	m_aov(copy.m_aov),
	m_near_clip(copy.m_near_clip),
	m_far_clip(copy.m_far_clip),
	//interpolation
	m_intp(copy.m_intp),
	//interacrive modes
	m_int_mode(copy.m_int_mode),
	m_crop_type(copy.m_crop_type),
	m_force_clear(false),
	m_interactive(false),
	m_clear_buffer(false),
	m_grow_on(false),
	//resizing
	m_resize(false),
	//brush tools
	m_draw_brush(false),
	m_paint_enable(false),
	m_paint_display(false),
	//paint buffer
	m_clear_paint(true),
	//camera controls
	m_persp(false),
	m_free(false),
	//camera distance
	m_distance(copy.m_distance),
	m_init_dist(copy.m_init_dist),
	//camera translation
	m_transx(0.0), m_transy(0.0), m_transz(0.0),
	m_transx_saved(0.0), m_transy_saved(0.0), m_transz_saved(0.0),
	//camera rotation
	m_rotx(0.0), m_roty(0.0), m_rotz(0.0),
	m_rotx_saved(0.0), m_roty_saved(0.0), m_rotz_saved(0.0),
	//camera center
	m_ctrx(0.0), m_ctry(0.0), m_ctrz(0.0),
	m_ctrx_saved(0.0), m_ctry_saved(0.0), m_ctrz_saved(0.0),
	//camera direction
	m_up(0.0, 1.0, 0.0),
	m_head(0.0, 0.0, -1.0),
	//object center
	m_obj_ctrx(0.0), m_obj_ctry(0.0), m_obj_ctrz(0.0),
	//object rotation
	m_obj_rotx(0.0), m_obj_roty(180.0), m_obj_rotz(180.0),
	//flag for using offset values
	m_offset(false),
	//object center offset
	m_obj_ctr_offx(0), m_obj_ctr_offy(0), m_obj_ctr_offz(0),
	//object rotation center offset
	m_obj_rot_ctr_offx(0), m_obj_rot_ctr_offy(0), m_obj_rot_ctr_offz(0),
	//object rotation offset
	m_obj_rot_offx(0), m_obj_rot_offy(0), m_obj_rot_offz(0),
	//object translation
	m_obj_transx(0.0), m_obj_transy(0.0), m_obj_transz(0.0),
	m_obj_transx_saved(0.0), m_obj_transy_saved(0.0), m_obj_transz_saved(0.0),
	m_rot_lock(false),
	//lock cam center
	m_lock_cam_object(false),
	m_pick_lock_center(false),
	//object bounding box
	m_radius(348.0),
	//mouse position
	old_mouse_X(-1), old_mouse_Y(-1),
	prv_mouse_X(-1), prv_mouse_Y(-1),
	//draw controls
	m_draw_all(copy.m_draw_all),
	m_draw_type(copy.m_draw_type),
	m_vol_method(copy.m_vol_method),
	//fog
	m_use_fog(true),
	m_fog_intensity(0.0),
	m_fog_start(0.0),
	m_fog_end(0.0),
	//movie properties
	m_init_angle(0.0),
	m_start_angle(0.0),
	m_end_angle(0.0),
	m_cur_angle(0.0),
	m_step(0.0),
	m_movie_seq(0),
	m_rewind(false),
	m_stages(0),
	m_4d_rewind(false),
	//post image processing
	m_gamma(copy.m_gamma),
	m_brightness(copy.m_brightness),
	m_hdr(copy.m_hdr),
	m_sync(copy.m_sync),
	//volume color map
	m_color_1(fluo::Color(0.0, 0.0, 1.0)),
	m_value_2(0.0),
	m_color_2(fluo::Color(0.0, 0.0, 1.0)),
	m_value_3(0.25),
	m_color_3(fluo::Color(0.0, 1.0, 1.0)),
	m_value_4(0.5),
	m_color_4(fluo::Color(0.0, 1.0, 0.0)),
	m_value_5(0.75),
	m_color_5(fluo::Color(1.0, 1.0, 0.0)),
	m_value_6(1.0),
	m_color_6(fluo::Color(1.0, 0.0, 0.0)),
	m_color_7(fluo::Color(1.0, 0.0, 0.0)),
	//clipping plane rotations
	m_rotx_cl(0), m_roty_cl(0), m_rotz_cl(0),
	//selection
	m_pick(false),
	m_draw_mask(true),
	m_clear_mask(false),
	m_save_mask(false),
	//move view
	m_move_left(false),
	m_move_right(false),
	m_move_up(false),
	m_move_down(false),
	//move time
	m_tseq_forward(false),
	m_tseq_backward(false),
	//move clip
	m_clip_up(false),
	m_clip_down(false),
	//full cell
	m_cell_full(false),
	//link cells
	m_cell_link(false),
	//new cell id
	m_cell_new_id(false),
	//comp include
	m_comp_include(false),
	//comp exclude
	m_comp_exclude(false),
	//nodraw count
	m_nodraw_count(0),
	//pin rotation center
	m_pin_rot_ctr(false),
	m_update_rot_ctr(false),
	m_pin_pick_thresh(0.6),
	m_res_mode(1),
	m_drawing(false),
	m_refresh(false),
	m_keep_enlarge(false),
	m_enlarge(false),
	m_enlarge_scale(1.0),
	//vr settings
	m_lg_initiated(false),
	m_use_openxr(false),
	m_vr_eye_idx(0),
	//size
	m_size(copy.m_size),
	m_gl_size(copy.m_gl_size),
	m_dpi_factor(copy.m_dpi_factor),
	//handle and parents
	m_hWnd(0),
	m_render_view_panel(0),
	m_render_canvas(0)
{
	type = 1;

	m_ruler_list = std::make_unique<flrd::RulerList>();
	m_cell_list = std::make_unique<flrd::CelpList>();
	m_text_renderer = std::make_unique<flvr::TextRenderer>();

	//xbox controller
#if defined(_WIN32) && defined(USE_XINPUT)
	m_controller = new XboxController(1);
	if (m_controller)
	{
		m_control_connected = m_controller->IsConnected();
	}
#endif

}

RenderView::~RenderView()
{
	if (glbin_settings.m_hologram_mode)
	{
		if (glbin_settings.m_hologram_mode == 1 && m_use_openxr)
		{
			if (glbin_xr_renderer)
				glbin_xr_renderer->Close();
		}
		if (glbin_settings.m_hologram_mode == 2)
		{
			glbin_lg_renderer.Close();
		}
	}

	int i;
	//delete groups
	for (i = 0; i<(int)m_layer_list.size(); i++)
	{
		if (!m_layer_list[i])
			continue;
		if (m_layer_list[i]->IsA() == 2)
		{
			VolumeData* vd = (VolumeData*)m_layer_list[i];
			if (vd)
				glbin_current.mainframe->DeleteProps(2, vd->GetName());
		}
		if (m_layer_list[i]->IsA() == 3)
		{
			MeshData* md = (MeshData*)m_layer_list[i];
			if (md)
			{
				glbin_current.mainframe->DeleteProps(3, md->GetName());
				glbin_current.mainframe->DeleteProps(6, md->GetName());
			}
		}
		if (m_layer_list[i]->IsA() == 4)
		{
			Annotations* ad = (Annotations*)m_layer_list[i];
			if (ad)
				glbin_current.mainframe->DeleteProps(4, ad->GetName());
		}
		if (m_layer_list[i]->IsA() == 5)//group
		{
			DataGroup* group = (DataGroup*)m_layer_list[i];
			for (size_t j = 0; j < group->GetVolumeNum(); ++j)
			{
				VolumeData* vd = group->GetVolumeData(j);
				if (vd)
					glbin_current.mainframe->DeleteProps(2, vd->GetName());
			}
			delete group;
		}
		else if (m_layer_list[i]->IsA() == 6)//mesh group
		{
			MeshGroup* group = (MeshGroup*)m_layer_list[i];
			for (size_t j = 0; j < group->GetMeshNum(); ++j)
			{
				MeshData* md = group->GetMeshData(j);
				if (md)
					glbin_current.mainframe->DeleteProps(3, md->GetName());
			}
			delete group;
		}
	}

	//delete rulers
	m_ruler_list->DeleteRulers();

	if (m_track_group)
		delete m_track_group;

	if (m_mvr)
		delete m_mvr;

#if defined(_WIN32) && defined(USE_XINPUT)
	if (m_controller)
		delete m_controller;
#endif
}

//set render view panel
void RenderView::SetRenderViewPanel(RenderViewPanel* panel)
{
	m_render_view_panel = panel;
	if (!panel)
		return;
	m_id = panel->GetId();
	m_name = panel->GetName();
}

//set render canvas
void RenderView::SetRenderCanvas(RenderCanvas* canvas)
{
	m_render_canvas = canvas;
}

void RenderView::SetSize(int x, int y)
{
	m_size = Size2D(x, y) * m_dpi_factor;
	if (m_enlarge)
		m_gl_size = Size2D(static_cast<int>(std::round(m_size.w() * m_enlarge_scale)),
			static_cast<int>(std::round(m_size.h() * m_enlarge_scale)));
	else
		m_gl_size = m_size;
}

std::string RenderView::GetOGLVersion()
{
	return m_GLversion;
}

void RenderView::Init()
{
	if (!m_initialized)
	{
		flvr::ShaderProgram::init_shaders_supported();
		flvr::ShaderProgram::set_max_texture_size(glbin_settings.m_max_texture_size);
		flvr::ShaderProgram::set_no_tex_upack(glbin_settings.m_no_tex_pack);
		flvr::KernelProgram::set_platform_id(glbin_settings.m_cl_platform_id);
		flvr::KernelProgram::set_device_id(glbin_settings.m_cl_device_id);
		flvr::KernelProgram::init_kernels_supported();
#ifdef _DARWIN
		CGLContextObj ctx = CGLGetCurrentContext();
		if (ctx != flvr::TextureRenderer::gl_context_)
			flvr::TextureRenderer::gl_context_ = ctx;
#endif
		glbin_settings.GetMemorySettings();
		flvr::Texture::mask_undo_num_ = (size_t)(glbin_brush_def.m_paint_hist_depth);
		glEnable(GL_MULTISAMPLE);

		m_initialized = true;

		glbin.getStopWatch(gstStopWatch)->start();
	}
}

void RenderView::InitOpenXR()
{
	if (m_use_openxr)
		return;
#ifdef _WIN32
	HDC hdc = GetDC((HWND)m_hWnd);
	HGLRC hglrc = wglGetCurrentContext();
	if (glbin_xr_renderer)
		m_use_openxr = glbin_xr_renderer->Init(
			static_cast<void*>(hdc),
			static_cast<void*>(hglrc),
			0);
#elif defined(__APPLE__)
	/*CGLContextObj cglContext = CGLGetCurrentContext();
	// Define the pixel format attributes based on wxGLCanvas attributes
	NSOpenGLPixelFormatAttribute profile_ver = NSOpenGLProfileVersionLegacy;
	if (glbin_settings.m_gl_major_ver == 3)
		profile_ver = NSOpenGLProfileVersion3_2Core;
	else if (glbin_settings.gl_major_ver == 4)
		profile_ver = NSOpenGLProfileVersion4_1Core;
	NSOpenGLPixelFormatAttribute color_size =
		glbin_settings.m_red_bit +
		glbin_settings.m_green_bit +
		glbin_settings.m_blue_bit;
	NSOpenGLPixelFormatAttribute alpha_size = glbin_settings.m_alpha_bit;
	NSOpenGLPixelFormatAttribute depth_size = glbin_settings.m_depth_bit;
	NSOpenGLPixelFormatAttribute attributes[] = {
		NSOpenGLPFAOpenGLProfile, profile_ver,
		NSOpenGLPFAColorSize, color_size,
		NSOpenGLPFAAlphaSize, alpha_size,
		NSOpenGLPFADepthSize, depth_size,
		NSOpenGLPFADoubleBuffer,
		0
	};
	// Create the NSOpenGLPixelFormat object
	NSOpenGLPixelFormat* pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
	if (!pixelFormat)
		return;
	// Retrieve the CGLPixelFormatObj from the NSOpenGLPixelFormat
	CGLPixelFormatObj cglPixelFormat = [pixelFormat CGLPixelFormatObj];*/
	if (glbin_xr_renderer)
		m_use_openxr = glbin_xr_renderer->Init(
			0,
			0,
			0);
#elif defined(__linux__)
	Display* xDisplay = glXGetCurrentDisplay();
	GLXDrawable glxDrawable = glXGetCurrentDrawable();
	GLXContext glxContext = glXGetCurrentContext();
	if (glbin_xr_renderer)
		m_use_openxr = glbin_xr_renderer->Init(
			static_cast<void*>(xDisplay),
			static_cast<void*>(glxContext),
			static_cast<uint64_t>(glxDrawable));
#endif
	if (!m_use_openxr)
	{
		glbin_settings.m_xr_api = 0;
		//m_frame->UpdateProps({ gstHologramMode });
	}
}

void RenderView::InitLookingGlass()
{
	if (m_lg_initiated)
		return;
	bool bval = glbin_lg_renderer.Init();
	if (!bval)
	{
		glbin_settings.m_hologram_mode = 0;
		glbin_current.mainframe->UpdateProps({ gstHologramMode });
		return;
	}
	glbin_lg_renderer.SetDevIndex(glbin_settings.m_lg_dev_id);
	glbin_lg_renderer.Setup();
	glbin_lg_renderer.Clear();
	if (glbin_settings.m_lg_offset == 0)
		glbin_settings.m_lg_offset = glbin_lg_renderer.GetHalfCone();
	glbin_settings.m_disp_id = glbin_lg_renderer.GetDisplayId();
	glbin_current.mainframe->UpdateProps({ gstHologramMode, gstFullscreenDisplay });
	if (m_render_view_panel)
		m_render_view_panel->SetFullScreen();
	m_lg_initiated = true;
}

//init
void RenderView::InitView(unsigned int type)
{
	int i;

	if (type&INIT_BOUNDS)
	{
		m_bounds.reset();
		PopVolumeList();
		PopMeshList();

		for (i = 0; i<(int)m_vd_pop_list.size(); i++)
			m_bounds.extend(m_vd_pop_list[i]->GetBounds());
		for (i = 0; i<(int)m_md_pop_list.size(); i++)
			m_bounds.extend(m_md_pop_list[i]->GetBounds());

		if (m_bounds.valid())
		{
			fluo::Vector diag = m_bounds.diagonal();
			m_radius = sqrt(diag.x()*diag.x() + diag.y()*diag.y()) / 2.0;
			if (m_radius<0.1)
				m_radius = 348.0;
			m_near_clip = m_radius / 1000.0;
			m_far_clip = m_radius * 100.0;
			if (glbin_xr_renderer)
				glbin_xr_renderer->SetClips(m_near_clip, m_far_clip);
		}
	}

	if (type&INIT_CENTER)
	{
		if (m_bounds.valid())
		{
			m_obj_ctrx = (m_bounds.Min().x() + m_bounds.Max().x()) / 2.0;
			m_obj_ctry = (m_bounds.Min().y() + m_bounds.Max().y()) / 2.0;
			m_obj_ctrz = (m_bounds.Min().z() + m_bounds.Max().z()) / 2.0;
		}
	}

	if (type&INIT_TRANSL/*||!m_init_view*/)
	{
		m_distance = m_radius / tan(d2r(m_aov / 2.0));
		m_init_dist = m_distance;
		m_transx = 0.0;
		m_transy = 0.0;
		m_transz = m_distance;
	}

	if (type&INIT_OBJ_TRANSL)
	{
		m_obj_transx = 0.0;
		m_obj_transy = 0.0;
		m_obj_transz = 0.0;
	}

	if (type&INIT_ROTATE || !m_init_view)
	{
		m_q = fluo::Quaternion(0, 0, 0, 1);
		m_q.ToEuler(m_rotx, m_roty, m_rotz);
	}

	m_init_view = true;

}

void RenderView::ClearAll()
{
	Clear();
	ClearVolList();
	ClearMeshList();
	m_cur_vol = 0;
	m_cur_vol_save = 0;
	m_ruler_list->clear();
	m_cur_ruler = 0;
	if (m_track_group)
		m_track_group->Clear();
	m_cell_list->clear();
	InitView();
	SetClippingPlaneRotations(fluo::Vector(0.0));
}

void RenderView::Clear()
{
	glbin_vol_loader.RemoveAllLoadedBrick();
	flvr::TextureRenderer::clear_tex_pool();

	//delete groups
	for (int i = 0; i<(int)m_layer_list.size(); i++)
	{
		if (!m_layer_list[i])
			continue;
		if (m_layer_list[i]->IsA() == 5)//group
		{
			DataGroup* group = (DataGroup*)m_layer_list[i];
			delete group;
		}
		else if (m_layer_list[i]->IsA() == 6)//mesh group
		{
			MeshGroup* group = (MeshGroup*)m_layer_list[i];
			delete group;
		}
	}

	m_layer_list.clear();
	m_cur_vol = 0;
}

void RenderView::ClearVolList()
{
	glbin_vol_loader.RemoveAllLoadedBrick();
	flvr::TextureRenderer::clear_tex_pool();
	m_vd_pop_list.clear();
}

void RenderView::ClearMeshList()
{
	m_md_pop_list.clear();
}

int RenderView::GetAny()
{
	PopVolumeList();
	PopMeshList();
	return m_vd_pop_list.size() + m_md_pop_list.size();
}

int RenderView::GetDispVolumeNum()
{
	//get the volume list m_vd_pop_list
	PopVolumeList();

	return m_vd_pop_list.size();
}

int RenderView::GetAllVolumeNum()
{
	int num = 0;
	for (int i = 0; i<(int)m_layer_list.size(); i++)
	{
		if (!m_layer_list[i])
			continue;
		switch (m_layer_list[i]->IsA())
		{
		case 2:  //volume data
			num++;
			break;
		case 5:  //group
		{
			DataGroup* group = (DataGroup*)m_layer_list[i];
			num += group->GetVolumeNum();
		}
		break;
		}
	}
	return num;
}

int RenderView::GetMeshNum()
{
	PopMeshList();
	return m_md_pop_list.size();
}

int RenderView::GetGroupNum()
{
	int group_num = 0;

	for (int i = 0; i<(int)m_layer_list.size(); i++)
	{
		TreeLayer *layer = m_layer_list[i];
		if (layer && layer->IsA() == 5)
			group_num++;
	}
	return group_num;
}

int RenderView::GetLayerNum()
{
	return m_layer_list.size();
}

VolumeData* RenderView::GetAllVolumeData(int index)
{
	int cnt = 0;
	int i, j;
	for (i = 0; i<(int)m_layer_list.size(); i++)
	{
		if (!m_layer_list[i])
			continue;
		switch (m_layer_list[i]->IsA())
		{
		case 2:  //volume data
			if (cnt == index)
				return (VolumeData*)m_layer_list[i];
			cnt++;
			break;
		case 5:  //group
		{
			DataGroup* group = (DataGroup*)m_layer_list[i];
			if (!group)
				break;
			for (j = 0; j<group->GetVolumeNum(); j++)
			{
				if (cnt == index)
					return group->GetVolumeData(j);
				cnt++;
			}
		}
		break;
		}
	}
	return 0;
}

VolumeData* RenderView::GetDispVolumeData(int index)
{
	if (GetDispVolumeNum() <= 0)
		return 0;

	//get the volume list m_vd_pop_list
	PopVolumeList();

	if (index >= 0 && index<(int)m_vd_pop_list.size())
		return m_vd_pop_list[index];
	else
		return 0;
}

MeshData* RenderView::GetMeshData(int index)
{
	if (GetMeshNum() <= 0)
		return 0;

	PopMeshList();

	if (index >= 0 && index<(int)m_md_pop_list.size())
		return m_md_pop_list[index];
	else
		return 0;
}

TreeLayer* RenderView::GetLayer(int index)
{
	if (index >= 0 && index<(int)m_layer_list.size())
		return m_layer_list[index];
	else
		return 0;
}

VolumeData* RenderView::GetVolumeData(const std::wstring &name)
{
	int i, j;

	for (i = 0; i<(int)m_layer_list.size(); i++)
	{
		if (!m_layer_list[i])
			continue;
		switch (m_layer_list[i]->IsA())
		{
		case 2://volume data
		{
			VolumeData* vd = (VolumeData*)m_layer_list[i];
			if (vd && vd->GetName() == name)
				return vd;
		}
		break;
		case 5://group
		{
			DataGroup* group = (DataGroup*)m_layer_list[i];
			if (!group)
				break;
			for (j = 0; j<group->GetVolumeNum(); j++)
			{
				VolumeData* vd = group->GetVolumeData(j);
				if (vd && vd->GetName() == name)
					return vd;
			}
		}
		break;
		}
	}

	return 0;
}

MeshData* RenderView::GetMeshData(const std::wstring &name)
{
	int i, j;

	for (i = 0; i<(int)m_layer_list.size(); i++)
	{
		if (!m_layer_list[i])
			continue;
		switch (m_layer_list[i]->IsA())
		{
		case 3://mesh data
		{
			MeshData* md = (MeshData*)m_layer_list[i];
			if (md && md->GetName() == name)
				return md;
		}
		break;
		case 6://mesh group
		{
			MeshGroup* group = (MeshGroup*)m_layer_list[i];
			if (!group) continue;
			for (j = 0; j<group->GetMeshNum(); j++)
			{
				MeshData* md = group->GetMeshData(j);
				if (md && md->GetName() == name)
					return md;
			}
		}
		break;
		}
	}
	return 0;
}

Annotations* RenderView::GetAnnotations(const std::wstring &name)
{
	int i;

	for (i = 0; i<(int)m_layer_list.size(); i++)
	{
		if (!m_layer_list[i])
			continue;
		switch (m_layer_list[i]->IsA())
		{
		case 4://annotations
		{
			Annotations* ann = (Annotations*)m_layer_list[i];
			if (ann && ann->GetName() == name)
				return ann;
		}
		}
	}
	return 0;
}

DataGroup* RenderView::GetGroup(const std::wstring &name)
{
	int i;

	for (i = 0; i<(int)m_layer_list.size(); i++)
	{
		if (!m_layer_list[i])
			continue;
		switch (m_layer_list[i]->IsA())
		{
		case 5://group
		{
			DataGroup* group = (DataGroup*)m_layer_list[i];
			if (group && group->GetName() == name)
				return group;
		}
		}
	}
	return 0;
}

DataGroup* RenderView::GetGroup(int index)
{
	int i;
	int count = 0;

	for (i = 0; i < (int)m_layer_list.size(); i++)
	{
		if (!m_layer_list[i])
			continue;
		if (m_layer_list[i]->IsA() == 5)
		{
			if (count == index)
			{
				DataGroup* group = (DataGroup*)m_layer_list[i];
				return group;
			}
			count++;
		}
	}
	return 0;
}

DataGroup* RenderView::GetGroup(VolumeData* vd)
{
	for (int i = 0; i < GetLayerNum(); i++)
	{
		TreeLayer* layer = GetLayer(i);
		if (layer && layer->IsA() == 5)
		{
			DataGroup* group = (DataGroup*)layer;
			for (int j = 0; j < group->GetVolumeNum(); j++)
			{
				VolumeData* tmp_vd = group->GetVolumeData(j);
				if (tmp_vd && tmp_vd == vd)
					return group;
			}
		}
	}
	return 0;
}

MeshGroup* RenderView::GetMGroup(const std::wstring& str)
{
	int i;

	for (i = 0; i<(int)m_layer_list.size(); i++)
	{
		if (!m_layer_list[i])
			continue;
		switch (m_layer_list[i]->IsA())
		{
		case 6://mesh group
		{
			MeshGroup* group = (MeshGroup*)m_layer_list[i];
			if (group && group->GetName() == str)
				return group;
		}
		}
	}
	return 0;
}

DataGroup* RenderView::AddVolumeData(VolumeData* vd, const std::wstring& group_name)
{
	//m_layer_list.push_back(vd);
	int i;
	DataGroup* group = 0;
	DataGroup* group_temp = 0;

	for (i = 0; i<(int)m_layer_list.size(); i++)
	{
		TreeLayer* layer = m_layer_list[i];
		if (layer && layer->IsA() == 5)
		{
			//layer is group
			group_temp = (DataGroup*)layer;
			if (group_temp && group_temp->GetName() == group_name)
			{
				group = group_temp;
				break;
			}
		}
	}

	if (!group && group_temp)
		group = group_temp;

	if (!group)
	{
		std::wstring group_name = AddGroup(L"");
		group = GetGroup(group_name);
		if (!group)
			return 0;
	}

	/*for (i=0; i<1; i++)
	{
	VolumeData* vol_data = group->GetVolumeData(i);
	if (vol_data)
	{
	double spcx, spcy, spcz;
	vol_data->GetSpacings(spcx, spcy, spcz);
	vd->SetSpacings(spcx, spcy, spcz);
	}
	}*/

	group->InsertVolumeData(group->GetVolumeNum() - 1, vd);

	if (group && vd)
	{
		fluo::Color gamma = group->GetGammaColor();
		vd->SetGammaColor(gamma);
		fluo::Color brightness = group->GetBrightness();
		vd->SetBrightness(brightness);
		fluo::Color hdr = group->GetHdr();
		vd->SetHdr(hdr);
		for (int i : { 0, 1, 2})
			vd->SetSync(i, group->GetSync(i));
		glbin_current.vol_data = vd;
	}

	m_vd_pop_dirty = true;
	m_load_update = true;

	return group;
}

void RenderView::AddMeshData(MeshData* md)
{
	m_layer_list.push_back(md);
	m_md_pop_dirty = true;
}

void RenderView::AddAnnotations(Annotations* ann)
{
	m_layer_list.push_back(ann);
}

std::wstring RenderView::AddGroup(const std::wstring& str, const std::wstring& prev_group)
{
	DataGroup* group = new DataGroup();
	if (group && str != L"")
		group->SetName(str);

	bool found_prev = false;
	for (size_t i = 0; i<m_layer_list.size(); i++)
	{
		if (!m_layer_list[i])
			continue;
		switch (m_layer_list[i]->IsA())
		{
		case 5://group
		{
			DataGroup* group_temp = (DataGroup*)m_layer_list[i];
			if (group_temp && group_temp->GetName() == prev_group)
			{
				m_layer_list.insert(m_layer_list.begin() + i + 1, group);
				found_prev = true;
			}
		}
		break;
		}
	}
	if (!found_prev)
		m_layer_list.push_back(group);

	//set default settings
	if (group)
	{
		//fluo::Color gamma, brightness, hdr;
		//bool sync_r, sync_g, sync_b;
		//adjust_view->GetDefaults(gamma, brightness, hdr, sync_r, sync_g, sync_b);
		group->SetGammaColor(
			fluo::Color(glbin_outadj_def.m_gamma_r, glbin_outadj_def.m_gamma_g, glbin_outadj_def.m_gamma_b));
		group->SetBrightness(
			fluo::Color(glbin_outadj_def.m_brightness_r, glbin_outadj_def.m_brightness_g, glbin_outadj_def.m_brightness_b));
		group->SetHdr(
			fluo::Color(glbin_outadj_def.m_hdr_r, glbin_outadj_def.m_hdr_g, glbin_outadj_def.m_hdr_b));
		group->SetSync(0, glbin_outadj_def.m_sync_r);
		group->SetSync(1, glbin_outadj_def.m_sync_g);
		group->SetSync(2, glbin_outadj_def.m_sync_b);
	}

	if (group)
		return group->GetName();
	else
		return L"";
}

DataGroup* RenderView::AddOrGetGroup()
{
	for (int i = 0; i < (int)m_layer_list.size(); i++)
	{
		if (!m_layer_list[i])
			continue;
		switch (m_layer_list[i]->IsA())
		{
		case 5://group
		{
			DataGroup* group_temp = (DataGroup*)m_layer_list[i];
			if (group_temp && !group_temp->GetVolumeNum())
				return group_temp;
		}
		break;
		}
	}
	//group not found
	DataGroup* group = new DataGroup();
	if (!group)
		return 0;
	//set default settings
	group->SetGammaColor(
		fluo::Color(glbin_outadj_def.m_gamma_r, glbin_outadj_def.m_gamma_g, glbin_outadj_def.m_gamma_b));
	group->SetBrightness(
		fluo::Color(glbin_outadj_def.m_brightness_r, glbin_outadj_def.m_brightness_g, glbin_outadj_def.m_brightness_b));
	group->SetHdr(
		fluo::Color(glbin_outadj_def.m_hdr_r, glbin_outadj_def.m_hdr_g, glbin_outadj_def.m_hdr_b));
	group->SetSync(0, glbin_outadj_def.m_sync_r);
	group->SetSync(1, glbin_outadj_def.m_sync_g);
	group->SetSync(2, glbin_outadj_def.m_sync_b);
	m_layer_list.push_back(group);
	return group;
}

std::wstring RenderView::AddMGroup(const std::wstring& str)
{
	MeshGroup* group = new MeshGroup();
	if (group && str != L"")
		group->SetName(str);
	m_layer_list.push_back(group);

	if (group)
		return group->GetName();
	else
		return L"";
}

MeshGroup* RenderView::AddOrGetMGroup()
{
	for (int i = 0; i < (int)m_layer_list.size(); i++)
	{
		if (!m_layer_list[i])
			continue;
		switch (m_layer_list[i]->IsA())
		{
		case 6://group
		{
			MeshGroup* group_temp = (MeshGroup*)m_layer_list[i];
			if (group_temp && !group_temp->GetMeshNum())
				return group_temp;
		}
		break;
		}
	}
	//group not found
	MeshGroup* group = new MeshGroup();
	if (!group)
		return 0;
	m_layer_list.push_back(group);
	return group;
}

void RenderView::RemoveVolumeData(const std::wstring &name)
{
	//std::wstring str = GetName().ToStdWstring() + L":";
	glbin_current.mainframe->DeleteProps(2, name);

	for (auto iter = m_layer_list.begin();
		iter != m_layer_list.end(); ++iter)
	{
		if (!(*iter))
			continue;
		switch ((*iter)->IsA())
		{
		case 2://volume data
		{
			VolumeData* vd = (VolumeData*)(*iter);
			if (vd && vd->GetName() == name)
			{
				m_layer_list.erase(iter);
				if (m_cur_vol == vd)
					m_cur_vol = 0;
				m_vd_pop_dirty = true;
				return;
			}
		}
		break;
		case 5://group
		{
			DataGroup* group = (DataGroup*)(*iter);
			for (int j = 0; j < group->GetVolumeNum(); ++j)
			{
				VolumeData* vd = group->GetVolumeData(j);
				if (vd && vd->GetName() == name)
				{
					group->RemoveVolumeData(j);
					if (m_cur_vol == vd)
						m_cur_vol = 0;
					m_vd_pop_dirty = true;
					return;
				}
			}
		}
		}
	}
}

void RenderView::RemoveVolumeDataDup(const std::wstring &name)
{
	//std::wstring str = GetName().ToStdWstring() + L":";
	glbin_current.mainframe->DeleteProps(2, name);
	VolumeData* vd_main = 0;
	for (auto iter = m_layer_list.begin();
		iter != m_layer_list.end() && !vd_main;
		++iter)
	{
		if (!(*iter))
			continue;
		switch ((*iter)->IsA())
		{
		case 2://volume data
		{
			VolumeData* vd = (VolumeData*)(*iter);
			if (vd && vd->GetName() == name)
			{
				vd_main = vd;
				m_vd_pop_dirty = true;
			}
		}
		break;
		case 5://group
		{
			DataGroup* group = (DataGroup*)(*iter);
			for (int j = 0; j<group->GetVolumeNum(); j++)
			{
				VolumeData* vd = group->GetVolumeData(j);
				if (vd && vd->GetName() == name)
				{
					vd_main = vd;
					m_vd_pop_dirty = true;
				}
			}
		}
		break;
		}
	}

	if (!vd_main)
		return;

	glbin_current.SetVolumeData(0);
	
	for (auto iter = m_layer_list.begin();
		iter != m_layer_list.end();)
	{
		if (!(*iter))
		{
			++iter;
			continue;
		}
		switch ((*iter)->IsA())
		{
		case 2://volume data
		{
			VolumeData* vd = (VolumeData*)(*iter);
			bool del = false;
			if (vd)
			{
				if (vd == vd_main)
					del = true;
				if (vd->GetDup())
				{
					if (vd->GetDupData() == vd_main)
						del = true;
				}
			}
			if (del)
			{
				iter = m_layer_list.erase(iter);
				if (m_cur_vol == vd)
					m_cur_vol = 0;
			}
			else
				++iter;
		}
		break;
		case 5://group
		{
			DataGroup* group = (DataGroup*)(*iter);
			for (int j = group->GetVolumeNum()-1; j >= 0; --j)
			{
				VolumeData* vd = group->GetVolumeData(j);
				if (vd)
				{
					bool del = false;
					if (vd->GetDup())
					{
						if (vd->GetDupData() == vd_main)
							del = true;
					}
					else
					{
						if (vd == vd_main)
							del = true;
					}
					if (del)
					{
						group->RemoveVolumeData(j);
						if (m_cur_vol == vd)
							m_cur_vol = 0;
					}
				}
			}
			++iter;
		}
		break;
		default:
			++iter;
			break;
		}
	}
}

void RenderView::ReplaceVolumeData(const std::wstring &name, VolumeData *dst)
{
	int i, j;

	bool found = false;
	DataGroup* group = 0;

	for (i = 0; i<(int)m_layer_list.size(); i++)
	{
		if (!m_layer_list[i])
			continue;
		switch (m_layer_list[i]->IsA())
		{
		case 2://volume data
		{
			VolumeData* vd = (VolumeData*)m_layer_list[i];
			if (vd && vd->GetName() == name)
			{
				if (m_cur_vol == vd) m_cur_vol = dst;
				glbin_vol_loader.RemoveBrickVD(vd);
				vd->GetVR()->clear_tex_current();
				m_layer_list[i] = dst;
				m_vd_pop_dirty = true;
				found = true;
				glbin_data_manager.RemoveVolumeData(name);
				break;
			}
		}
		break;
		case 5://group
		{
			DataGroup* tmpgroup = (DataGroup*)m_layer_list[i];
			for (j = 0; j<tmpgroup->GetVolumeNum(); j++)
			{
				VolumeData* vd = tmpgroup->GetVolumeData(j);
				if (vd && vd->GetName() == name)
				{
					if (m_cur_vol == vd) m_cur_vol = dst;
					glbin_vol_loader.RemoveBrickVD(vd);
					vd->GetVR()->clear_tex_current();
					tmpgroup->ReplaceVolumeData(j, dst);
					m_vd_pop_dirty = true;
					found = true;
					group = tmpgroup;
					glbin_data_manager.RemoveVolumeData(name);
					break;
				}
			}
		}
		break;
		}
	}

	if (found)
	{
		glbin_current.vol_data = dst;
		VolumePropPanel* vprop_view = glbin_current.mainframe->FindVolumeProps(name);
		if (vprop_view)
			vprop_view->SetVolumeData(dst);
	}
}

void RenderView::RemoveMeshData(const std::wstring &name)
{
	//std::wstring str = GetName().ToStdWstring() + L":";
	glbin_current.mainframe->DeleteProps(3, name);

	int i, j;

	for (i = 0; i<(int)m_layer_list.size(); i++)
	{
		if (!m_layer_list[i])
			continue;
		switch (m_layer_list[i]->IsA())
		{
		case 3://mesh data
		{
			MeshData* md = (MeshData*)m_layer_list[i];
			if (md && md->GetName() == name)
			{
				m_layer_list.erase(m_layer_list.begin() + i);
				m_md_pop_dirty = true;
				return;
			}
		}
		break;
		case 6://mesh group
		{
			MeshGroup* group = (MeshGroup*)m_layer_list[i];
			if (!group) continue;
			for (j = 0; j<group->GetMeshNum(); j++)
			{
				MeshData* md = group->GetMeshData(j);
				if (md && md->GetName() == name)
				{
					group->RemoveMeshData(j);
					m_md_pop_dirty = true;
					return;
				}
			}
		}
		break;
		}
	}
}

void RenderView::RemoveAnnotations(const std::wstring &name)
{
	glbin_current.mainframe->DeleteProps(4, name);
	for (int i = 0; i<(int)m_layer_list.size(); i++)
	{
		if (!m_layer_list[i])
			continue;
		if (m_layer_list[i]->IsA() == 4)
		{
			Annotations* ann = (Annotations*)m_layer_list[i];
			if (ann && ann->GetName() == name)
			{
				m_layer_list.erase(m_layer_list.begin() + i);
			}
		}
	}
}

void RenderView::RemoveGroup(const std::wstring &name)
{
	for (size_t i = 0; i<m_layer_list.size(); i++)
	{
		if (!m_layer_list[i])
			continue;
		switch (m_layer_list[i]->IsA())
		{
		case 5://group
		{
			DataGroup* group = (DataGroup*)m_layer_list[i];
			if (group && group->GetName() == name)
			{
				for (int j = group->GetVolumeNum() - 1; j >= 0; j--)
				{
					VolumeData* vd = group->GetVolumeData(j);
					if (vd)
					{
						group->RemoveVolumeData(j);
						//if add back to view
						glbin_current.mainframe->DeleteProps(2, vd->GetName());
					}
				}
				m_layer_list.erase(m_layer_list.begin() + i);
				delete group;
				m_vd_pop_dirty = true;
			}
		}
		break;
		case 6://mesh group
		{
			MeshGroup* group = (MeshGroup*)m_layer_list[i];
			if (group && group->GetName() == name)
			{
				for (int j = group->GetMeshNum() - 1; j >= 0; j--)
				{
					MeshData* md = group->GetMeshData(j);
					if (md)
					{
						group->RemoveMeshData(j);
						glbin_current.mainframe->DeleteProps(3, md->GetName());
					}
				}
				m_layer_list.erase(m_layer_list.begin() + i);
				delete group;
				m_md_pop_dirty = true;
			}
		}
		break;
		}
	}
}

//isolate
void RenderView::Isolate(int type, const std::wstring& name)
{
	for (size_t i = 0; i<m_layer_list.size(); i++)
	{
		if (!m_layer_list[i]) continue;

		switch (m_layer_list[i]->IsA())
		{
		case 2://volume
		{
			VolumeData* vd = (VolumeData*)m_layer_list[i];
			if (vd)
			{
				if (type == 2 &&
					vd->GetName() == name)
					vd->SetDisp(true);
				else
					vd->SetDisp(false);
			}
		}
		break;
		case 3://mesh
		{
			MeshData* md = (MeshData*)m_layer_list[i];
			if (md)
			{
				if (type == 3 &&
					md->GetName() == name)
					md->SetDisp(false);
				else
					md->SetDisp(false);
			}
		}
		break;
		case 4://annotation
		{
			Annotations* ann = (Annotations*)m_layer_list[i];
			if (ann)
			{
				if (type == 4 &&
					ann->GetName() == name)
					ann->SetDisp(true);
				else
					ann->SetDisp(false);
			}
		}
		break;
		case 5://volume group
		{
			DataGroup* group = (DataGroup*)m_layer_list[i];
			if (group)
			{
				if (type == 5)
				{
					if (group->GetName() == name)
						group->SetDisp(true);
					else
						group->SetDisp(false);
				}
				else if (type == 6)
					group->SetDisp(false);
				else
				{
					for (int i = 0; i<(int)group->GetVolumeNum(); i++)
					{
						VolumeData* vd = group->GetVolumeData(i);
						if (vd)
						{
							if (type == 2 &&
								vd->GetName() == name)
								vd->SetDisp(true);
							else
								vd->SetDisp(false);
						}
					}
				}
			}
		}
		break;
		case 6://mesh group
		{
			MeshGroup* group = (MeshGroup*)m_layer_list[i];
			if (group)
			{
				if (type == 6)
				{
					if (group->GetName() == name)
						group->SetDisp(true);
					else
						group->SetDisp(false);
				}
				else if (type == 5)
					group->SetDisp(false);
				else
				{
					for (int i = 0; i < (int)group->GetMeshNum(); i++)
					{
						MeshData* md = group->GetMeshData(i);
						if (md)
						{
							if (type == 3 &&
								md->GetName() == name)
								md->SetDisp(true);
							else
								md->SetDisp(false);
						}
					}
				}
			}
		}
		break;
		}
	}
	m_vd_pop_dirty = true;
	m_md_pop_dirty = true;
}

void RenderView::ShowAll()
{
	for (size_t i = 0; i<m_layer_list.size(); ++i)
	{
		if (!m_layer_list[i]) continue;

		switch (m_layer_list[i]->IsA())
		{
		case 2://volume
		{
			VolumeData* vd = (VolumeData*)m_layer_list[i];
			if (vd)
				vd->SetDisp(true);
		}
		break;
		case 3://mesh
		{
			MeshData* md = (MeshData*)m_layer_list[i];
			if (md)
				md->SetDisp(true);
		}
		break;
		case 4://annotation
		{
			Annotations* ann = (Annotations*)m_layer_list[i];
			if (ann)
				ann->SetDisp(true);
		}
		break;
		case 5:
		{
			DataGroup* group = (DataGroup*)m_layer_list[i];
			if (group)
			{
				group->SetDisp(true);
				for (int j = 0; j<group->GetVolumeNum(); ++j)
				{
					VolumeData* vd = group->GetVolumeData(j);
					if (vd)
						vd->SetDisp(true);
				}
			}
		}
		break;
		case 6://mesh group
		{
			MeshGroup* group = (MeshGroup*)m_layer_list[i];
			if (group)
			{
				group->SetDisp(true);
				for (int j = 0; j<group->GetMeshNum(); ++j)
				{
					MeshData* md = group->GetMeshData(j);
					if (md)
						md->SetDisp(true);
				}
			}
		}
		break;
		}
	}
	m_vd_pop_dirty = true;
	m_md_pop_dirty = true;
}

//move layer of the same level within this view
//source is after the destination
void RenderView::MoveLayerinView(const std::wstring &src_name, const std::wstring &dst_name)
{
	size_t src_index;
	TreeLayer* src = 0;
	for (size_t i = 0; i<m_layer_list.size(); i++)
	{
		if (m_layer_list[i] && m_layer_list[i]->GetName() == src_name)
		{
			src = m_layer_list[i];
			src_index = i;
			m_layer_list.erase(m_layer_list.begin() + i);
			break;
		}
	}
	if (!src)
		return;
	for (size_t i = 0; i<m_layer_list.size(); i++)
	{
		if (m_layer_list[i] && m_layer_list[i]->GetName() == dst_name)
		{
			if (i >= src_index)
				m_layer_list.insert(m_layer_list.begin() + i + 1, src);
			else
				m_layer_list.insert(m_layer_list.begin() + i, src);
			break;
		}
	}
	m_vd_pop_dirty = true;
	m_md_pop_dirty = true;
}

//move layer (volume) of the same level within the given group
//source is after the destination
void RenderView::MoveLayerinGroup(const std::wstring &group_name, const std::wstring &src_name, const std::wstring &dst_name)
{
	DataGroup* group = GetGroup(group_name);
	if (!group)
		return;

	VolumeData* src_vd = 0;
	int i, src_index;
	for (i = 0; i<group->GetVolumeNum(); i++)
	{
		std::wstring name = group->GetVolumeData(i)->GetName();
		if (name == src_name)
		{
			src_index = i;
			src_vd = group->GetVolumeData(i);
			group->RemoveVolumeData(i);
			break;
		}
	}
	if (!src_vd)
		return;
	for (i = 0; i<group->GetVolumeNum(); i++)
	{
		std::wstring name = group->GetVolumeData(i)->GetName();
		if (name == dst_name)
		{
			if (i >= src_index)
				group->InsertVolumeData(i, src_vd);
			else
				group->InsertVolumeData(i - 1, src_vd);
			break;
		}
	}
	m_vd_pop_dirty = true;
	m_md_pop_dirty = true;
}

//move layer (volume) from the given group up one level to this view
//source is after the destination
void RenderView::MoveLayertoView(const std::wstring &group_name, const std::wstring &src_name, const std::wstring &dst_name)
{
	DataGroup* group = GetGroup(group_name);
	if (!group)
		return;

	VolumeData* src_vd = 0;
	for (int i = 0; i<group->GetVolumeNum(); i++)
	{
		std::wstring name = group->GetVolumeData(i)->GetName();
		if (name == src_name)
		{
			src_vd = group->GetVolumeData(i);
			group->RemoveVolumeData(i);
			break;
		}
	}
	if (!src_vd)
		return;
	if (dst_name == "")
	{
		m_layer_list.push_back(src_vd);
	}
	else
	{
		for (size_t i = 0; i<m_layer_list.size(); i++)
		{
			std::wstring name = m_layer_list[i]->GetName();
			if (name == dst_name)
			{
				m_layer_list.insert(m_layer_list.begin() + i + 1, src_vd);
				break;
			}
		}
	}
	m_vd_pop_dirty = true;
	m_md_pop_dirty = true;
}

//move layer (volume) one level down to the given group
//source is after the destination
void RenderView::MoveLayertoGroup(const std::wstring &group_name, const std::wstring &src_name, const std::wstring &dst_name)
{
	VolumeData* src_vd = 0;

	for (size_t i = 0; i<m_layer_list.size(); i++)
	{
		std::wstring name = m_layer_list[i]->GetName();
		if (name == src_name && m_layer_list[i]->IsA() == 2)//is volume data
		{
			src_vd = (VolumeData*)m_layer_list[i];
			m_layer_list.erase(m_layer_list.begin() + i);
			break;
		}
	}
	DataGroup* group = GetGroup(group_name);
	if (!group || !src_vd)
		return;
	if (group->GetVolumeNum() == 0 || dst_name == L"")
	{
		group->InsertVolumeData(0, src_vd);
	}
	else
	{
		for (int i = 0; i<group->GetVolumeNum(); i++)
		{
			std::wstring name = group->GetVolumeData(i)->GetName();
			if (name == dst_name)
			{
				group->InsertVolumeData(i, src_vd);
				break;
			}
		}
	}

	//set the 2d adjustment settings of the volume the same as the group
	fluo::Color gamma = group->GetGammaColor();
	src_vd->SetGammaColor(gamma);
	fluo::Color brightness = group->GetBrightness();
	src_vd->SetBrightness(brightness);
	fluo::Color hdr = group->GetHdr();
	src_vd->SetHdr(hdr);
	for (int i : { 0, 1, 2})
		src_vd->SetSync(i, group->GetSync(i));

	m_vd_pop_dirty = true;
	m_md_pop_dirty = true;
}

//move layer (volume from one group to another different group
//sourece is after the destination
void RenderView::MoveLayerfromtoGroup(const std::wstring &src_group_name, const std::wstring &dst_group_name, const std::wstring &src_name, const std::wstring &dst_name)
{
	DataGroup* src_group = GetGroup(src_group_name);
	if (!src_group)
		return;
	int i;
	VolumeData* src_vd = 0;
	for (i = 0; i<src_group->GetVolumeNum(); i++)
	{
		std::wstring name = src_group->GetVolumeData(i)->GetName();
		if (name == src_name)
		{
			src_vd = src_group->GetVolumeData(i);
			src_group->RemoveVolumeData(i);
			break;
		}
	}
	DataGroup* dst_group = GetGroup(dst_group_name);
	if (!dst_group || !src_vd)
		return;
	if (dst_group->GetVolumeNum() == 0 || dst_name == L"")
	{
		dst_group->InsertVolumeData(0, src_vd);
	}
	else
	{
		for (i = 0; i<dst_group->GetVolumeNum(); i++)
		{
			std::wstring name = dst_group->GetVolumeData(i)->GetName();
			if (name == dst_name)
			{
				dst_group->InsertVolumeData(i, src_vd);
				break;
			}
		}
	}

	//reset the sync of the source group
	//src_group->ResetSync();

	//set the 2d adjustment settings of the volume the same as the group
	fluo::Color gamma = dst_group->GetGammaColor();
	src_vd->SetGammaColor(gamma);
	fluo::Color brightness = dst_group->GetBrightness();
	src_vd->SetBrightness(brightness);
	fluo::Color hdr = dst_group->GetHdr();
	src_vd->SetHdr(hdr);
	for (int i : { 0, 1, 2})
		src_vd->SetSync(i, dst_group->GetSync(i));

	m_vd_pop_dirty = true;
	m_md_pop_dirty = true;

	glbin_current.vol_data = src_vd;
}

//move mesh within a group
void RenderView::MoveMeshinGroup(const std::wstring &group_name, const std::wstring &src_name, const std::wstring &dst_name)
{
	MeshGroup* group = GetMGroup(group_name);
	if (!group)
		return;

	MeshData* src_md = 0;
	int i, src_index;
	for (i = 0; i<group->GetMeshNum(); i++)
	{
		std::wstring name = group->GetMeshData(i)->GetName();
		if (name == src_name)
		{
			src_index = i;
			src_md = group->GetMeshData(i);
			group->RemoveMeshData(i);
			break;
		}
	}
	if (!src_md)
		return;
	for (i = 0; i<group->GetMeshNum(); i++)
	{
		std::wstring name = group->GetMeshData(i)->GetName();
		if (name == dst_name)
		{
			if (i >= src_index)
				group->InsertMeshData(i, src_md);
			else
				group->InsertMeshData(i - 1, src_md);
			break;
		}
	}

	m_vd_pop_dirty = true;
	m_md_pop_dirty = true;
}

//move mesh out of a group
void RenderView::MoveMeshtoView(const std::wstring &group_name, const std::wstring &src_name, const std::wstring &dst_name)
{
	MeshGroup* group = GetMGroup(group_name);
	if (!group)
		return;

	MeshData* src_md = 0;
	int i;
	for (i = 0; i<group->GetMeshNum(); i++)
	{
		std::wstring name = group->GetMeshData(i)->GetName();
		if (name == src_name)
		{
			src_md = group->GetMeshData(i);
			group->RemoveMeshData(i);
			break;
		}
	}
	if (!src_md)
		return;
	if (dst_name == L"")
		m_layer_list.push_back(src_md);
	else
	{
		for (i = 0; i<(int)m_layer_list.size(); i++)
		{
			std::wstring name = m_layer_list[i]->GetName();
			if (name == dst_name)
			{
				m_layer_list.insert(m_layer_list.begin() + i + 1, src_md);
				break;
			}
		}
	}
	m_vd_pop_dirty = true;
	m_md_pop_dirty = true;
}

//move mesh into a group
void RenderView::MoveMeshtoGroup(const std::wstring &group_name, const std::wstring &src_name, const std::wstring &dst_name)
{
	MeshData* src_md = 0;

	for (size_t i = 0; i<m_layer_list.size(); i++)
	{
		std::wstring name = m_layer_list[i]->GetName();
		if (name == src_name && m_layer_list[i]->IsA() == 3)
		{
			src_md = (MeshData*)m_layer_list[i];
			m_layer_list.erase(m_layer_list.begin() + i);
			break;
		}
	}
	MeshGroup* group = GetMGroup(group_name);
	if (!group || !src_md)
		return;
	if (group->GetMeshNum() == 0 || dst_name == "")
		group->InsertMeshData(0, src_md);
	else
	{
		for (int i = 0; i<group->GetMeshNum(); i++)
		{
			std::wstring name = group->GetMeshData(i)->GetName();
			if (name == dst_name)
			{
				group->InsertMeshData(i, src_md);
				break;
			}
		}
	}
	m_vd_pop_dirty = true;
	m_md_pop_dirty = true;
}

//move mesh out of then into a group
void RenderView::MoveMeshfromtoGroup(const std::wstring &src_group_name, const std::wstring &dst_group_name, const std::wstring &src_name, const std::wstring &dst_name)
{
	MeshGroup* src_group = GetMGroup(src_group_name);
	if (!src_group)
		return;
	int i;
	MeshData* src_md = 0;
	for (i = 0; i<src_group->GetMeshNum(); i++)
	{
		std::wstring name = src_group->GetMeshData(i)->GetName();
		if (name == src_name)
		{
			src_md = src_group->GetMeshData(i);
			src_group->RemoveMeshData(i);
			break;
		}
	}
	MeshGroup* dst_group = GetMGroup(dst_group_name);
	if (!dst_group || !src_md)
		return;
	if (dst_group->GetMeshNum() == 0 || dst_name == "")
		dst_group->InsertMeshData(0, src_md);
	else
	{
		for (i = 0; i<dst_group->GetMeshNum(); i++)
		{
			std::wstring name = dst_group->GetMeshData(i)->GetName();
			if (name == dst_name)
			{
				dst_group->InsertMeshData(i, src_md);
				break;
			}
		}
	}
	m_vd_pop_dirty = true;
	m_md_pop_dirty = true;
}

//get populated volume list
//stored in m_vd_pop_list
void RenderView::PopVolumeList()
{
	if (!m_vd_pop_dirty)
		return;

	int i, j;
	m_vd_pop_list.clear();

	for (i = 0; i<(int)m_layer_list.size(); i++)
	{
		if (!m_layer_list[i])
			continue;
		switch (m_layer_list[i]->IsA())
		{
		case 2://volume data
		{
			VolumeData* vd = (VolumeData*)m_layer_list[i];
			if (vd->GetDisp())
				m_vd_pop_list.push_back(vd);
		}
		break;
		case 5://group
		{
			DataGroup* group = (DataGroup*)m_layer_list[i];
			if (!group->GetDisp())
				continue;
			for (j = 0; j<group->GetVolumeNum(); j++)
			{
				if (group->GetVolumeData(j) && group->GetVolumeData(j)->GetDisp())
					m_vd_pop_list.push_back(group->GetVolumeData(j));
			}
		}
		break;
		}
	}
	m_vd_pop_dirty = false;
}

//get populated mesh list
//stored in m_md_pop_list
void RenderView::PopMeshList()
{
	if (!m_md_pop_dirty)
		return;

	int i, j;
	m_md_pop_list.clear();

	for (i = 0; i<(int)m_layer_list.size(); i++)
	{
		if (!m_layer_list[i])
			return;
		switch (m_layer_list[i]->IsA())
		{
		case 3://mesh data
		{
			MeshData* md = (MeshData*)m_layer_list[i];
			if (md->GetDisp())
				m_md_pop_list.push_back(md);
		}
		break;
		case 6://mesh group
		{
			MeshGroup* group = (MeshGroup*)m_layer_list[i];
			if (!group->GetDisp())
				continue;
			for (j = 0; j<group->GetMeshNum(); j++)
			{
				if (group->GetMeshData(j) &&
					group->GetMeshData(j)->GetDisp())
					m_md_pop_list.push_back(group->GetMeshData(j));
			}
		}
		break;
		}
	}
	m_md_pop_dirty = false;
}

//organize layers in view
//put all volume data under view into last group of the view
//if no group in view
void RenderView::OrganizeLayers()
{
	DataGroup* le_group = 0;
	int i;

	//find last empty group
	for (i = GetLayerNum() - 1; i >= 0; i--)
	{
		TreeLayer* layer = GetLayer(i);
		if (layer && layer->IsA() == 5)
		{
			//layer is group
			DataGroup* group = (DataGroup*)layer;
			if (group->GetVolumeNum() == 0)
			{
				le_group = group;
				break;
			}
		}
	}

	for (i = 0; i<GetLayerNum(); i++)
	{
		TreeLayer* layer = GetLayer(i);
		if (layer && layer->IsA() == 2)
		{
			//layer is volume
			VolumeData* vd = (VolumeData*)layer;
			std::wstring name = vd->GetName();
			if (le_group)
			{
				RemoveVolumeData(name);
				le_group->InsertVolumeData(le_group->GetVolumeNum(), vd);
			}
			else
			{
				std::wstring group_name = AddGroup(L"");
				le_group = GetGroup(group_name);
				if (le_group)
				{
					RemoveVolumeData(name);
					le_group->InsertVolumeData(le_group->GetVolumeNum(), vd);
				}
			}
		}
	}
}

void RenderView::RandomizeColor()
{
	for (int i = 0; i<(int)m_layer_list.size(); i++)
	{
		if (m_layer_list[i])
			m_layer_list[i]->RandomizeColor();
	}
}

void RenderView::HandleProjection(int nx, int ny, bool vr)
{
	if (ny == 0 || m_aov == 0 || m_scale_factor == 0)
		return;
	if (!m_free)
		m_distance = m_radius / tan(d2r(m_aov / 2.0)) / m_scale_factor;

	double aspect = (double)nx / (double)ny;
	m_ortho_left = -m_radius * aspect / m_scale_factor;
	m_ortho_right = -m_ortho_left;
	m_ortho_bottom = -m_radius / m_scale_factor;
	m_ortho_top = -m_ortho_bottom;

	if (vr && m_use_openxr)
	{
		//get projection matrix
		m_proj_mat = glbin_xr_renderer->GetProjectionMatrix(m_vr_eye_idx);
	}
	else
	{
		if (m_persp)
		{
			m_proj_mat = glm::perspective(glm::radians(m_aov), aspect, m_near_clip, m_far_clip);
		}
		else
		{
			m_proj_mat = glm::ortho(m_ortho_left, m_ortho_right, m_ortho_bottom, m_ortho_top,
				-m_far_clip / 100.0, m_far_clip);
		}
	}
}

void RenderView::HandleCamera(bool vr)
{
	fluo::Vector pos(m_transx, m_transy, m_transz);
	pos.normalize();
	if (m_free)
		pos *= 0.1;
	else
		pos *= m_distance;
	m_transx = pos.x();
	m_transy = pos.y();
	m_transz = pos.z();

	glm::vec3 eye(m_transx, m_transy, m_transz);
	glm::vec3 center(0.0);
	glm::vec3 up(m_up.x(), m_up.y(), m_up.z());

	if (m_free)
	{
		center = glm::vec3(m_ctrx, m_ctry, m_ctrz);
		eye += center;
	}

	if (vr)
	{
		if (m_use_openxr)
		{
			if (glbin_settings.m_mv_hmd)
			{
				//get tracking pose matrix
				glm::mat4 mv_hmd = glbin_xr_renderer->GetModelViewMatrix(m_vr_eye_idx);
				m_mv_mat = glm::lookAt(eye, center, up);
				m_mv_mat = mv_hmd * m_mv_mat;
			}
			else
			{
				m_mv_mat = glm::lookAt(eye, center, up);
			}
		}
		else if (glbin_settings.m_hologram_mode)
		{
			fluo::Vector side = GetSide();
			if (glbin_settings.m_hologram_mode == 1)
			{
				side *= (m_vr_eye_idx ? 1.0 : -1.0) * glbin_settings.m_eye_dist / 2.0;
				glm::vec3 offset(side.x(), side.y(), side.z());
				m_mv_mat = glm::lookAt(
					eye + offset,
					center + offset,
					up);
			}
			else if (glbin_settings.m_hologram_mode == 2)
			{
				double f = glbin_lg_renderer.GetOffset();
				//linear shift
				//side *= glbin_settings.m_lg_offset;
				//side *= f;
				//glm::vec3 offset(side.x(), side.y(), side.z());
				//m_mv_mat = glm::lookAt(
				//	eye + offset,
				//	center + offset,
				//	up);

				//turntable
				double ang = f * glbin_settings.m_lg_offset;//half angle
				glm::mat4 rot(1);
				rot = glm::rotate(rot, float(glm::radians(-ang)), up);
				glm::vec4 vv = glm::vec4(eye - center, 1);
				vv = rot * vv;
				glm::vec3 new_eye = center + glm::vec3(vv);
				glm::vec3 new_view = glm::vec3(vv);
				new_view = glm::normalize(new_view);
				glm::vec3 new_up = glm::cross(new_view, up);
				new_up = glm::cross(new_up, new_view);
				m_mv_mat = glm::lookAt(new_eye, center, new_up);
				eye = new_eye;
				up = new_up;
			}
		}
		else
		{
			m_mv_mat = glm::lookAt(eye, center, up);
		}
	}
	else
	{
		m_mv_mat = glm::lookAt(eye, center, up);
	}

	if (m_lock_cam_object)
	{
		//rotate first
		glm::vec3 v1(0, 0, -1);//initial cam direction
		glm::mat4 mv_mat = GetDrawMat();
		glm::vec4 lock_ctr(m_lock_center.x(), m_lock_center.y(), m_lock_center.z(), 1);
		lock_ctr = mv_mat * lock_ctr;
		glm::vec3 v2(lock_ctr);
		v2 = glm::normalize(v2);
		float c = glm::dot(v1, v2);
		if (std::abs(std::abs(c) - 1) < fluo::Epsilon())
			return;
		glm::vec3 v = glm::cross(v1, v2);
		glm::mat3 vx(
			0, -v.z, v.y,
			v.z, 0, -v.x,
			-v.y, v.x, 0);
		glm::mat3 vx2 = vx * vx;
		glm::mat3 rot3(1);
		rot3 += vx + vx2 / (1 + c);
		glm::mat4 rot4(rot3);
		m_mv_mat = rot4 * glm::lookAt(eye, center, up);
	}
}

//camera operations
fluo::Vector RenderView::GetTranslations()
{
	return fluo::Vector(m_transx, m_transy, m_transz);
}
void RenderView::SetTranslations(const fluo::Vector& val)
{
	m_transx = val.x(); m_transy = val.y(); m_transz = val.z();
	m_distance = sqrt(m_transx*m_transx + m_transy*m_transy + m_transz*m_transz);
}

fluo::Vector RenderView::GetRotations()
{
	return fluo::Vector(m_rotx, m_roty, m_rotz);
}

void RenderView::SetRotations(const fluo::Vector& val, bool notify)
{
	m_rotx = val.x();
	m_roty = val.y();
	m_rotz = val.z();

	if (m_roty>360.0)
		m_roty -= 360.0;
	if (m_roty<0.0)
		m_roty += 360.0;
	if (m_rotx>360.0)
		m_rotx -= 360.0;
	if (m_rotx<0.0)
		m_rotx += 360.0;
	if (m_rotz>360.0)
		m_rotz -= 360.0;
	if (m_rotz<0.0)
		m_rotz += 360.0;

	A2Q();

	fluo::Quaternion cam_pos(0.0, 0.0, m_distance, 0.0);
	fluo::Quaternion cam_pos2 = (-m_q) * cam_pos * m_q;
	m_transx = cam_pos2.x;
	m_transy = cam_pos2.y;
	m_transz = cam_pos2.z;

	fluo::Quaternion up(0.0, 1.0, 0.0, 0.0);
	fluo::Quaternion up2 = (-m_q) * up * m_q;
	m_up = fluo::Vector(up2.x, up2.y, up2.z);

	if (notify && m_render_view_panel)
		m_render_view_panel->FluoUpdate({ gstCamRotation });
}

int RenderView::GetOrientation()
{
	//update ortho rotation
	if (m_q.AlmostEqual(fluo::Quaternion(0, sqrt(2.0) / 2.0, 0, sqrt(2.0) / 2.0)))
		return 0;
	else if (m_q.AlmostEqual(fluo::Quaternion(0, -sqrt(2.0) / 2.0, 0, sqrt(2.0) / 2.0)))
		return 1;
	else if (m_q.AlmostEqual(fluo::Quaternion(sqrt(2.0) / 2.0, 0, 0, sqrt(2.0) / 2.0)))
		return 2;
	else if (m_q.AlmostEqual(fluo::Quaternion(-sqrt(2.0) / 2.0, 0, 0, sqrt(2.0) / 2.0)))
		return 3;
	else if (m_q.AlmostEqual(fluo::Quaternion(0, 0, 0, 1)))
		return 4;
	else if (m_q.AlmostEqual(fluo::Quaternion(0, -1, 0, 0)))
		return 5;
	else
		return 6;
}

fluo::Vector RenderView::ResetZeroRotations()
{
	double rotx, roty, rotz;
	m_zq = fluo::Quaternion();
	m_q.ToEuler(rotx, roty, rotz);
	if (roty > 360.0)
		roty -= 360.0;
	if (roty < 0.0)
		roty += 360.0;
	if (rotx > 360.0)
		rotx -= 360.0;
	if (rotx < 0.0)
		rotx += 360.0;
	if (rotz > 360.0)
		rotz -= 360.0;
	if (rotz < 0.0)
		rotz += 360.0;
	return fluo::Vector(rotx, roty, rotz);
}

fluo::Point RenderView::GetCenters()
{
	return fluo::Point(m_ctrx, m_ctry, m_ctrz);
}

void RenderView::SetCenters(const fluo::Point& val)
{
	m_ctrx = val.x(); m_ctry = val.y(); m_ctrz = val.z();
}

void RenderView::SetCenter()
{
	InitView(INIT_BOUNDS | INIT_CENTER | INIT_OBJ_TRANSL);

	VolumeData *vd = 0;
	if (m_cur_vol)
		vd = m_cur_vol;
	else if (m_vd_pop_list.size())
		vd = m_vd_pop_list[0];

	if (vd)
	{
		fluo::BBox bbox = vd->GetBounds();
		flvr::VolumeRenderer *vr = vd->GetVR();
		if (!vr) return;
		std::vector<fluo::Plane*> *planes = vr->get_planes();
		if (planes->size() != 6) return;
		double x1, x2, y1, y2, z1, z2;
		double abcd[4];
		(*planes)[0]->get_copy(abcd);
		x1 = fabs(abcd[3])*bbox.Max().x();
		(*planes)[1]->get_copy(abcd);
		x2 = fabs(abcd[3])*bbox.Max().x();
		(*planes)[2]->get_copy(abcd);
		y1 = fabs(abcd[3])*bbox.Max().y();
		(*planes)[3]->get_copy(abcd);
		y2 = fabs(abcd[3])*bbox.Max().y();
		(*planes)[4]->get_copy(abcd);
		z1 = fabs(abcd[3])*bbox.Max().z();
		(*planes)[5]->get_copy(abcd);
		z2 = fabs(abcd[3])*bbox.Max().z();

		m_obj_ctrx = (x1 + x2) / 2.0;
		m_obj_ctry = (y1 + y2) / 2.0;
		m_obj_ctrz = (z1 + z2) / 2.0;

		//SetSortBricks();

		RefreshGL(20);
	}
}

double RenderView::Get121ScaleFactor()
{
	double result = 1.0;

	int nx, ny;
	GetRenderSize(nx, ny);

	double spc_x = 1.0;
	double spc_y = 1.0;
	double spc_z = 1.0;
	VolumeData *vd = 0;
	if (m_cur_vol)
		vd = m_cur_vol;
	else if (m_vd_pop_list.size())
		vd = m_vd_pop_list[0];
	if (vd)
		vd->GetSpacings(spc_x, spc_y, spc_z, vd->GetLevel());
	spc_y = spc_y<EPS ? 1.0 : spc_y;

	result = 2.0*m_radius / spc_y / double(ny);

	return result;
}

void RenderView::SetScale121()
{
	m_scale_factor = Get121ScaleFactor();
	double value = 1.0;
	switch (m_scale_mode)
	{
	case 0:
		value = m_scale_factor;
		break;
	case 1:
		break;
	case 2:
		{
		VolumeData *vd = 0;
		if (m_cur_vol)
			vd = m_cur_vol;
		else if (m_vd_pop_list.size())
			vd = m_vd_pop_list[0];
		if (!vd)
			break;
		double spcx = 0, spcy = 0, spcz = 0;
		vd->GetSpacings(spcx, spcy, spcz, vd->GetLevel());
		if (spcx > 0.0)
			value /= spcx;
		}
		break;
	}

	//SetSortBricks();

	RefreshGL(21);

	if (m_render_view_panel)
		m_render_view_panel->FluoUpdate({ gstScaleFactor });
}

void RenderView::SetPinRotCenter(bool pin)
{
	m_pin_rot_ctr = pin;
	if (pin)
		m_update_rot_ctr = true;
}

//object operations
fluo::Point RenderView::GetObjCenters()
{
	return fluo::Point(m_obj_ctrx, m_obj_ctry, m_obj_ctrz);
}

void RenderView::SetObjCenters(const fluo::Point& val)
{
	m_obj_ctrx = val.x();
	m_obj_ctry = val.y();
	m_obj_ctrz = val.z();
}

fluo::Vector RenderView::GetObjRot()
{
	return fluo::Vector(m_obj_rotx, m_obj_roty, m_obj_rotz);
}

void RenderView::SetObjRot(const fluo::Vector& val)
{
	m_obj_rotx = val.x();
	m_obj_roty = val.y();
	m_obj_rotz = val.z();
}

void RenderView::SetOffset()
{
	if (m_obj_ctr_offx != 0.0 || m_obj_ctr_offy != 0.0 || m_obj_ctr_offz != 0.0 ||
		m_obj_rot_ctr_offx != 0.0 || m_obj_rot_ctr_offy != 0.0 || m_obj_rot_ctr_offz != 0.0 ||
		m_obj_rot_offx != 0.0 || m_obj_rot_offy != 0.0 || m_obj_rot_offz != 0.0)
		m_offset = true;
	else
		m_offset = false;
}

fluo::Vector RenderView::GetObjCtrOff()
{
	return fluo::Vector(m_obj_ctr_offx, m_obj_ctr_offy, m_obj_ctr_offz);
}

void RenderView::SetObjCtrOff(const fluo::Vector& val)
{
	m_obj_ctr_offx = val.x();
	m_obj_ctr_offy = val.y();
	m_obj_ctr_offz = val.z();
	SetOffset();
}

fluo::Vector RenderView::GetObjRotCtrOff()
{
	return fluo::Vector(m_obj_rot_ctr_offx, m_obj_rot_ctr_offy, m_obj_rot_ctr_offz);
}

void RenderView::SetObjRotCtrOff(const fluo::Vector& val)
{
	m_obj_rot_ctr_offx = val.x() == 0.0 ? m_obj_ctrx : val.x();
	m_obj_rot_ctr_offy = val.y() == 0.0 ? m_obj_ctry : val.y();
	m_obj_rot_ctr_offz = val.z() == 0.0 ? m_obj_ctrz : val.z();
	SetOffset();
}

fluo::Vector RenderView::GetObjRotOff()
{
	return fluo::Vector(m_obj_rot_offx, m_obj_rot_offy, m_obj_rot_offz);
}

void RenderView::SetObjRotOff(const fluo::Vector& val)
{
	m_obj_rot_offx = val.x();
	m_obj_rot_offy = val.y();
	m_obj_rot_offz = val.z();
	SetOffset();
}

void RenderView::SetOffsetTransform(const fluo::Transform &tf)
{
	m_offset_tf = tf;
}

fluo::Vector RenderView::GetObjTrans()
{
	return fluo::Vector(m_obj_transx, m_obj_transy, m_obj_transz);
}

void RenderView::SetObjTrans(const fluo::Vector& val)
{
	m_obj_transx = val.x();
	m_obj_transy = val.y();
	m_obj_transz = val.z();
}

void RenderView::SetLockCenter()
{
	int type = glbin_moviemaker.GetCamLockType();
	switch (type)
	{
	case 1:
	default:
		SetLockCenterVol();
		break;
	case 2:
		m_pick_lock_center = true;
		break;
	case 3:
		SetLockCenterRuler();
		break;
	case 4:
		SetLockCenterSel();
		break;
	}
}

void RenderView::SetLockCenterVol()
{
	if (!m_cur_vol)
		return;
	fluo::BBox box = m_cur_vol->GetClippedBounds();
	m_lock_center = box.center();
}

void RenderView::SetLockCenterRuler()
{
	if (!m_cur_ruler)
		return;
	m_lock_center = m_cur_ruler->GetCenter();
}

void RenderView::SetLockCenterSel()
{
	if (!m_cur_vol)
		return;
	flrd::Cov cover(m_cur_vol);
	cover.Compute(1);
	m_lock_center = cover.GetCenter();
}

void RenderView::SetPersp(bool persp)
{
	m_persp = persp;
	if (m_free && !m_persp)
	{
		m_free = false;

		//restore camera translation
		m_transx = m_transx_saved;
		m_transy = m_transy_saved;
		m_transz = m_transz_saved;
		m_ctrx = m_ctrx_saved;
		m_ctry = m_ctry_saved;
		m_ctrz = m_ctrz_saved;
		//restore camera rotation
		m_rotx = m_rotx_saved;
		m_roty = m_roty_saved;
		m_rotz = m_rotz_saved;
		//restore object translation
		m_obj_transx = m_obj_transx_saved;
		m_obj_transy = m_obj_transy_saved;
		m_obj_transz = m_obj_transz_saved;
		//restore scale factor
		m_scale_factor = m_scale_factor_saved;

		if (m_render_view_panel)
			m_render_view_panel->FluoUpdate({ gstScaleFactor, gstFree });
		SetRotations(fluo::Vector(m_rotx, m_roty, m_rotz), true);
	}

	//SetSortBricks();
}

void RenderView::SetFree(bool free)
{
	m_free = free;
	if (free)
	{
		m_persp = true;
		fluo::Vector pos(m_transx, m_transy, m_transz);
		fluo::Vector d = pos;
		d.normalize();
		fluo::Vector ctr;
		ctr = pos - 0.1*d;
		m_ctrx = ctr.x();
		m_ctry = ctr.y();
		m_ctrz = ctr.z();

		//save camera translation
		m_transx_saved = m_transx;
		m_transy_saved = m_transy;
		m_transz_saved = m_transz;
		m_ctrx_saved = m_ctrx;
		m_ctry_saved = m_ctry;
		m_ctrz_saved = m_ctrz;
		//save camera rotation
		m_rotx_saved = m_rotx;
		m_roty_saved = m_roty;
		m_rotz_saved = m_rotz;
		//save object translateion
		m_obj_transx_saved = m_obj_transx;
		m_obj_transy_saved = m_obj_transy;
		m_obj_transz_saved = m_obj_transz;
		//save scale factor
		m_scale_factor_saved = m_scale_factor;
	}
	else
	{
		m_ctrx = m_ctry = m_ctrz = 0.0;

		//restore camera translation
		m_transx = m_transx_saved;
		m_transy = m_transy_saved;
		m_transz = m_transz_saved;
		m_ctrx = m_ctrx_saved;
		m_ctry = m_ctry_saved;
		m_ctrz = m_ctrz_saved;
		//restore camera rotation
		m_rotx = m_rotx_saved;
		m_roty = m_roty_saved;
		m_rotz = m_rotz_saved;
		//restore object translation
		m_obj_transx = m_obj_transx_saved;
		m_obj_transy = m_obj_transy_saved;
		m_obj_transz = m_obj_transz_saved;
		//restore scale factor
		m_scale_factor = m_scale_factor_saved;
		if (m_render_view_panel)
			m_render_view_panel->FluoUpdate({ gstScaleFactor });

		SetRotations(fluo::Vector(m_rotx, m_roty, m_rotz), true);
	}
	//SetSortBricks();
}

fluo::Color RenderView::GetTextColor()
{
	switch (glbin_settings.m_text_color)
	{
	case 0://background inverted
		return m_bg_color_inv;
	case 1://background
		return m_bg_color;
	case 2://secondary color of current volume
		if (m_cur_vol)
			return m_cur_vol->GetMaskColor();
		else
			return m_bg_color_inv;
	}
	return m_bg_color_inv;
}

void RenderView::SetBackgroundColor(fluo::Color &color)
{
	m_bg_color = color;
	fluo::HSVColor bg_color(m_bg_color);
	double hue, sat, val;
	if (bg_color.val()>0.7 && bg_color.sat()>0.7)
	{
		hue = bg_color.hue() + 180.0;
		sat = 1.0;
		val = 1.0;
		m_bg_color_inv = fluo::Color(fluo::HSVColor(hue, sat, val));
	}
	else if (bg_color.val()>0.7)
	{
		m_bg_color_inv = fluo::Color(0.0, 0.0, 0.0);
	}
	else
	{
		m_bg_color_inv = fluo::Color(1.0, 1.0, 1.0);
	}
}

void RenderView::SetVolMethod(int method)
{
	//get the volume list m_vd_pop_list
	PopVolumeList();

	m_vol_method = method;
}

void RenderView::Set3DRotCapture(double start_angle,
	double end_angle,
	double step,
	int frames,
	int rot_axis,
	const std::wstring &cap_file,
	bool rewind,
	int len)
{
	fluo::Vector rv = GetRotations();

	//remove the chance of the x/y/z angles being outside 360.
	while (rv[0] > 360.)  rv[0] -= 360.;
	while (rv[0] < -360.) rv[0] += 360.;
	while (rv[1] > 360.)  rv[1] -= 360.;
	while (rv[1] < -360.) rv[1] += 360.;
	while (rv[2] > 360.)  rv[2] -= 360.;
	while (rv[2] < -360.) rv[2] += 360.;
	if (360. - std::abs(rv[0]) < 0.001) rv[0] = 0.;
	if (360. - std::abs(rv[1]) < 0.001) rv[1] = 0.;
	if (360. - std::abs(rv[2]) < 0.001) rv[2] = 0.;

	m_step = step;
	m_total_frames = frames;
	m_cap_file = cap_file;
	m_rewind = rewind;
	m_movie_seq = 0;

	m_rot_axis = rot_axis;
	if (start_angle == 0.)
	{
		m_init_angle = rv[m_rot_axis];
		m_end_angle = rv[m_rot_axis] + end_angle;
	}
	m_cur_angle = rv[m_rot_axis];
	m_start_angle = rv[m_rot_axis];

	m_capture = true;
	m_capture_rotat = true;
	m_capture_rotate_over = false;
	m_stages = 0;
}

void RenderView::Set4DSeqCapture(const std::wstring &cap_file, int begin_frame, int end_frame, bool rewind)
{
	m_cap_file = cap_file;
	m_tseq_cur_num = begin_frame;
	m_tseq_prv_num = begin_frame;
	m_begin_frame = begin_frame;
	m_end_frame = end_frame;
	m_capture_tsequ = true;
	m_capture = true;
	m_movie_seq = begin_frame;
	m_4d_rewind = rewind;
}

void RenderView::Set3DBatCapture(const std::wstring &cap_file, int begin_frame, int end_frame)
{
	m_cap_file = cap_file;
	m_begin_frame = begin_frame;
	m_end_frame = end_frame;
	m_capture_bat = true;
	m_capture = true;

	if (!m_cap_file.empty() && m_total_frames>1)
	{
		std::filesystem::path p(m_cap_file);
		p = p.parent_path();
		p /= m_bat_folder + L"_folder";
		std::wstring new_folder = p.wstring();
		MkDirW(new_folder);
	}
}

void RenderView::SetParamCapture(const std::wstring &cap_file, int begin_frame, int end_frame, bool rewind)
{
	m_cap_file = cap_file;
	m_param_cur_num = begin_frame;
	m_begin_frame = begin_frame;
	m_end_frame = end_frame;
	m_capture_param = true;
	m_capture = true;
	m_movie_seq = begin_frame;
	m_4d_rewind = rewind;
}

void RenderView::SetParams(double t)
{
	fluo::ValueCollection vc;
	m_frame_num_type = 1;
	m_param_cur_num = std::round(t);
	FlKeyCode keycode;
	keycode.l0 = 1;
	keycode.l0_name = ws2s(GetName());

	bool bval;
	double dval;
	int ival;
	fluo::Color cval;

	for (int i = 0; i<GetAllVolumeNum(); i++)
	{
		VolumeData* vd = GetAllVolumeData(i);
		if (!vd) continue;

		keycode.l1 = 2;
		keycode.l1_name = ws2s(vd->GetName());

		//display
		keycode.l2 = 0;
		keycode.l2_name = "display";
		if (glbin_interpolator.GetBoolean(keycode, t, bval))
			vd->SetDisp(bval);

		//clipping planes
		std::vector<fluo::Plane*> *planes = vd->GetVR()->get_planes();
		if (!planes) continue;
		if (planes->size() != 6) continue;
		fluo::Plane *plane = 0;
		double val = 0;
		//x1
		plane = (*planes)[0];
		keycode.l2_name = "x1_val";
		if (glbin_interpolator.GetDouble(keycode, t, val))
			plane->ChangePlane(fluo::Point(abs(val), 0.0, 0.0),
				fluo::Vector(1.0, 0.0, 0.0));
		vc.insert(gstClipX1);
		//x2
		plane = (*planes)[1];
		keycode.l2_name = "x2_val";
		if (glbin_interpolator.GetDouble(keycode, t, val))
			plane->ChangePlane(fluo::Point(abs(val), 0.0, 0.0),
				fluo::Vector(-1.0, 0.0, 0.0));
		vc.insert(gstClipX2);
		//y1
		plane = (*planes)[2];
		keycode.l2_name = "y1_val";
		if (glbin_interpolator.GetDouble(keycode, t, val))
			plane->ChangePlane(fluo::Point(0.0, abs(val), 0.0),
				fluo::Vector(0.0, 1.0, 0.0));
		vc.insert(gstClipY1);
		//y2
		plane = (*planes)[3];
		keycode.l2_name = "y2_val";
		if (glbin_interpolator.GetDouble(keycode, t, val))
			plane->ChangePlane(fluo::Point(0.0, abs(val), 0.0),
				fluo::Vector(0.0, -1.0, 0.0));
		vc.insert(gstClipY2);
		//z1
		plane = (*planes)[4];
		keycode.l2_name = "z1_val";
		if (glbin_interpolator.GetDouble(keycode, t, val))
			plane->ChangePlane(fluo::Point(0.0, 0.0, abs(val)),
				fluo::Vector(0.0, 0.0, 1.0));
		vc.insert(gstClipZ1);
		//z2
		plane = (*planes)[5];
		keycode.l2_name = "z2_val";
		if (glbin_interpolator.GetDouble(keycode, t, val))
			plane->ChangePlane(fluo::Point(0.0, 0.0, abs(val)),
				fluo::Vector(0.0, 0.0, -1.0));
		vc.insert(gstClipZ2);
		//t
		double frame;
		keycode.l2_name = "frame";
		if (glbin_interpolator.GetDouble(keycode, t, frame))
		{
			UpdateVolumeData(std::round(frame), vd);
			glbin_moviemaker.SetSeqCurNum(frame);
		}
		//primary color
		fluo::Color pc;
		keycode.l2_name = "color";
		if (glbin_interpolator.GetColor(keycode, t, pc))
			vd->SetColor(pc);
		vc.insert(gstColor);

		//volume properties
		//minmax
		keycode.l2_name = "minmax enable";
		if (glbin_interpolator.GetBoolean(keycode, t, bval))
			vd->SetMinMaxEnable(bval);
		keycode.l2_name = "low_offset";
		if (glbin_interpolator.GetDouble(keycode, t, dval))
			vd->SetLowOffset(dval);
		keycode.l2_name = "high_offset";
		if (glbin_interpolator.GetDouble(keycode, t, dval))
			vd->SetHighOffset(dval);
		vc.insert(gstMinMax);
		//gamma
		keycode.l2_name = "gamma3d enable";
		if (glbin_interpolator.GetBoolean(keycode, t, bval))
			vd->SetGammaEnable(bval);
		keycode.l2_name = "gamma3d";
		if (glbin_interpolator.GetDouble(keycode, t, dval))
			vd->SetGamma(dval);
		vc.insert(gstGamma3d);
		//alpha
		keycode.l2_name = "alpha enable";
		if (glbin_interpolator.GetBoolean(keycode, t, bval))
			vd->SetAlphaEnable(bval);
		keycode.l2_name = "alpha";
		if (glbin_interpolator.GetDouble(keycode, t, dval))
			vd->SetAlpha(dval);
		vc.insert(gstAlpha);
		//sample rate
		keycode.l2_name = "sample rate enable";
		if (glbin_interpolator.GetBoolean(keycode, t, bval))
			vd->SetSampleRateEnable(bval);
		keycode.l2_name = "sample rate";
		if (glbin_interpolator.GetDouble(keycode, t, dval))
			vd->SetSampleRate(dval);
		vc.insert(gstSampleRate);
		//threshold
		keycode.l2_name = "threshold enable";
		if (glbin_interpolator.GetBoolean(keycode, t, bval))
			vd->SetThreshEnable(bval);
		keycode.l2_name = "low threshold";
		if (glbin_interpolator.GetDouble(keycode, t, dval))
			vd->SetLeftThresh(dval);
		keycode.l2_name = "high threshold";
		if (glbin_interpolator.GetDouble(keycode, t, dval))
			vd->SetRightThresh(dval);
		vc.insert(gstThreshold);
		//boundary
		keycode.l2_name = "boundary enable";
		if (glbin_interpolator.GetBoolean(keycode, t, bval))
			vd->SetBoundaryEnable(bval);
		keycode.l2_name = "boundary";
		if (glbin_interpolator.GetDouble(keycode, t, dval))
			vd->SetBoundary(dval);
		vc.insert(gstBoundary);
		//shading
		keycode.l2_name = "shading enable";
		if (glbin_interpolator.GetBoolean(keycode, t, bval))
			vd->SetShadingEnable(bval);
		keycode.l2_name = "low shading";
		if (glbin_interpolator.GetDouble(keycode, t, dval))
			vd->SetLowShading(dval);
		keycode.l2_name = "high shading";
		if (glbin_interpolator.GetDouble(keycode, t, dval))
			vd->SetHiShading(dval);
		vc.insert(gstShading);
		//shadow
		keycode.l2_name = "shadow enable";
		if (glbin_interpolator.GetBoolean(keycode, t, bval))
			vd->SetShadowEnable(bval);
		keycode.l2_name = "shadow intensity";
		if (glbin_interpolator.GetDouble(keycode, t, dval))
			vd->SetShadowIntensity(dval);
		vc.insert(gstShadow);
		//colormap
		keycode.l2_name = "colormap enable";
		if (glbin_interpolator.GetBoolean(keycode, t, bval))
			vd->SetColormapMode(bval ? 1 : 0);
		keycode.l2_name = "colormap low";
		if (glbin_interpolator.GetDouble(keycode, t, dval))
			vd->SetColormapLow(dval);
		keycode.l2_name = "colormap high";
		if (glbin_interpolator.GetDouble(keycode, t, dval))
			vd->SetColormapHigh(dval);
		vc.insert(gstColormap);

		//output adj
		keycode.l2_name = "gamma";
		if (glbin_interpolator.GetColor(keycode, t, cval))
			vd->SetGammaColor(cval);
		keycode.l2_name = "brightness";
		if (glbin_interpolator.GetColor(keycode, t, cval))
			vd->SetBrightness(cval);
		keycode.l2_name = "hdr";
		if (glbin_interpolator.GetColor(keycode, t, cval))
			vd->SetHdr(cval);
	}

	bool bx, by, bz;
	//for the view
	keycode.l1 = 1;
	keycode.l1_name = ws2s(GetName());
	//translation
	double tx, ty, tz;
	keycode.l2 = 0;
	keycode.l2_name = "translation_x";
	bx = glbin_interpolator.GetDouble(keycode, t, tx);
	keycode.l2_name = "translation_y";
	by = glbin_interpolator.GetDouble(keycode, t, ty);
	keycode.l2_name = "translation_z";
	bz = glbin_interpolator.GetDouble(keycode, t, tz);
	if (bx && by && bz)
		SetTranslations(fluo::Vector(tx, ty, tz));
	//centers
	keycode.l2_name = "center_x";
	bx = glbin_interpolator.GetDouble(keycode, t, tx);
	keycode.l2_name = "center_y";
	by = glbin_interpolator.GetDouble(keycode, t, ty);
	keycode.l2_name = "center_z";
	bz = glbin_interpolator.GetDouble(keycode, t, tz);
	if (bx && by && bz)
		SetCenters(fluo::Point(tx, ty, tz));
	//obj translation
	keycode.l2_name = "obj_trans_x";
	bx = glbin_interpolator.GetDouble(keycode, t, tx);
	keycode.l2_name = "obj_trans_y";
	by = glbin_interpolator.GetDouble(keycode, t, ty);
	keycode.l2_name = "obj_trans_z";
	bz = glbin_interpolator.GetDouble(keycode, t, tz);
	if (bx && by && bz)
		SetObjTrans(fluo::Vector(tx, ty, tz));
	//scale
	double scale;
	keycode.l2_name = "scale";
	if (glbin_interpolator.GetDouble(keycode, t, scale))
	{
		m_scale_factor = scale;
		vc.insert(gstScaleFactor);
	}
	//rotation
	keycode.l2 = 0;
	keycode.l2_name = "rotation";
	fluo::Quaternion q;
	if (glbin_interpolator.GetQuaternion(keycode, t, q))
	{
		m_q = q;
		q *= -m_zq;
		double rotx, roty, rotz;
		q.ToEuler(rotx, roty, rotz);
		SetRotations(fluo::Vector(rotx, roty, rotz), true);
		vc.insert(gstCamRotation);
	}
	//intermixing mode
	keycode.l2_name = "volmethod";
	if (glbin_interpolator.GetInt(keycode, t, ival))
		SetVolMethod(ival);
	//perspective angle
	keycode.l2_name = "aov";
	double aov;
	if (glbin_interpolator.GetDouble(keycode, t, aov))
	{
		if (aov <= 10)
		{
			SetPersp(false);
		}
		else
		{
			SetPersp(true);
			SetAov(aov);
		}
		vc.insert(gstAov);
	}
	//shadow dir
	keycode.l2_name = "shadow dir enable";
	if (glbin_interpolator.GetBoolean(keycode, t, bval))
		glbin_settings.m_shadow_dir = bval;
	keycode.l2_name = "shadow dir x";
	if (glbin_interpolator.GetDouble(keycode, t, dval))
		glbin_settings.m_shadow_dir_x = dval;
	keycode.l2_name = "shadow dir y";
	if (glbin_interpolator.GetDouble(keycode, t, dval))
		glbin_settings.m_shadow_dir_y = dval;
	vc.insert(gstShadowDir);
	//output adj
	keycode.l2_name = "gamma";
	if (glbin_interpolator.GetColor(keycode, t, cval))
		SetGammaColor(cval);
	keycode.l2_name = "brightness";
	if (glbin_interpolator.GetColor(keycode, t, cval))
		SetBrightness(cval);
	keycode.l2_name = "hdr";
	if (glbin_interpolator.GetColor(keycode, t, cval))
		SetHdr(cval);
	vc.insert({ gstGammaR, gstGammaG, gstGammaB, gstBrightnessR, gstBrightnessG, gstBrightnessB, gstEqualizeR, gstEqualizeG, gstEqualizeB });

	//update ruler intensity values
	glbin_ruler_handler.ProfileAll();
	SetVolPopDirty();

	vc.insert(gstParamListSelect);
	vc.insert(gstTreeIcons);
	vc.insert(gstTreeColors);
	vc.insert(gstClipPlaneRangeColor);
	vc.insert(gstRulerList);

	glbin_current.mainframe->UpdateProps(vc);
}

void RenderView::ResetMovieAngle()
{
	fluo::Vector rv = GetRotations();
	rv[m_rot_axis] = m_init_angle;
	SetRotations(rv, true);

	m_capture = false;
	m_capture_rotat = false;

	RefreshGL(16);
}

void RenderView::StopMovie()
{
	m_capture = false;
	m_capture_rotat = false;
	m_capture_tsequ = false;
	m_capture_param = false;
}

void RenderView::Get4DSeqRange(int &start_frame, int &end_frame)
{
	for (int i = 0; i<(int)m_vd_pop_list.size(); i++)
	{
		VolumeData* vd = m_vd_pop_list[i];
		if (vd && vd->GetReader())
		{
			BaseReader* reader = vd->GetReader();

			int vd_start_frame = 0;
			int vd_end_frame = reader->GetTimeNum() - 1;
			int vd_cur_frame = reader->GetCurTime();

			if (i == 0)
			{
				//first dataset
				start_frame = vd_start_frame;
				end_frame = vd_end_frame;
			}
			else
			{
				//datasets after the first one
				if (vd_end_frame > end_frame)
					end_frame = vd_end_frame;
			}
		}
	}
	m_end_all_frame = end_frame;
}

void RenderView::Set4DSeqFrame(int frame, int start_frame, int end_frame, bool rewind)
{
	m_frame_num_type = 0;

	//skip update if frame num unchanged
	bool update = m_tseq_cur_num == frame ? false : true;
	//compute frame number
	m_begin_frame = start_frame;
	m_end_frame = end_frame;
	m_total_frames = std::abs(end_frame - start_frame + 1);

	//save currently selected volume
	m_cur_vol_save = m_cur_vol;

	//run pre-change script
	if (update && glbin_settings.m_run_script)
	{
		int r = glbin_script_proc.Run4DScript(flrd::ScriptProc::TM_ALL_PRE, rewind);
		if (r == 2)
		{
			glbin_moviemaker.Reset();
			return;
		}
	}

	//change time frame
	m_tseq_prv_num = m_tseq_cur_num;
	m_tseq_cur_num = frame;

	if (update)
	for (auto i : m_vd_pop_list)
		UpdateVolumeData(frame, i);

	//run post-change script
	if (update && glbin_settings.m_run_script)
		glbin_script_proc.Run4DScript(flrd::ScriptProc::TM_ALL_POST_REWIND, rewind);

	//restore currently selected volume
	m_cur_vol = m_cur_vol_save;
	glbin_vol_calculator.SetVolumeA(m_cur_vol);

	//update ruler intensity values
	glbin_ruler_handler.ProfileAll();

	//clear results if rewind
	if (rewind)
		glbin_script_proc.ClearResults();

	glbin_current.mainframe->UpdateProps({ gstRulerList });
}

void RenderView::UpdateVolumeData(int frame, VolumeData* vd)
{
	if (!vd)
		return;

	if (vd->GetCurTime() == frame)
		return;

	BaseReader* reader = vd->GetReader();
	if (!reader)
		return;

	bool clear_pool = false;

	flvr::Texture *tex = vd->GetTexture();
	if (tex && tex->isBrxml())
	{
		BRKXMLReader *br = (BRKXMLReader *)reader;
		br->SetCurTime(frame);
		int curlv = tex->GetCurLevel();
		for (int j = 0; j < br->GetLevelNum(); j++)
		{
			tex->setLevel(j);
			if (vd->GetVR()) vd->GetVR()->clear_brick_buf();
		}
		tex->setLevel(curlv);
		tex->set_FrameAndChannel(frame, vd->GetCurChannel());
		vd->SetCurTime(reader->GetCurTime());
	}
	else
	{
		double spcx, spcy, spcz;
		vd->GetSpacings(spcx, spcy, spcz);

		Nrrd* data = reader->Convert(frame, vd->GetCurChannel(), false);
		if (!vd->Replace(data, false))
			return;

		vd->SetCurTime(reader->GetCurTime());
		vd->SetSpacings(spcx, spcy, spcz);

		clear_pool = true;
	}

	glbin_current.mainframe->UpdateProps({ gstRulerList });

	if (clear_pool && vd->GetVR())
		vd->GetVR()->clear_tex_pool();
}

void RenderView::ReloadVolumeData(int frame)
{
	int i, j;
	std::vector<BaseReader*> reader_list;
	m_bat_folder = L"";

	for (i = 0; i < (int)m_vd_pop_list.size(); i++)
	{
		VolumeData* vd = m_vd_pop_list[i];
		if (vd)
			glbin_current.mainframe->DeleteProps(2, vd->GetName());
		if (vd && vd->GetReader())
		{
			flvr::Texture *tex = vd->GetTexture();
			BaseReader* reader = vd->GetReader();
			if (tex && tex->isBrxml())
			{
				BRKXMLReader *br = (BRKXMLReader *)reader;
				int curlv = tex->GetCurLevel();
				for (j = 0; j < br->GetLevelNum(); j++)
				{
					tex->setLevel(j);
					if (vd->GetVR()) vd->GetVR()->clear_brick_buf();
				}
				tex->setLevel(curlv);
				tex->set_FrameAndChannel(0, vd->GetCurChannel());
				vd->SetCurTime(reader->GetCurTime());
				std::wstring data_name = reader->GetDataName();
				if (i > 0)
					m_bat_folder += L"_";
				m_bat_folder += data_name;

				int chan_num = 0;
				if (data_name.find(L"_1ch") != std::wstring::npos)
					chan_num = 1;
				else if (data_name.find(L"_2ch") != std::wstring::npos)
					chan_num = 2;
				if (chan_num > 0 && vd->GetCurChannel() >= chan_num)
					vd->SetDisp(false);
				else
					vd->SetDisp(true);

				if (reader->GetChanNum() > 1)
					data_name += L"_" + std::to_wstring(vd->GetCurChannel() + 1);
				vd->SetName(data_name);
			}
			else
			{
				bool found = false;
				for (j = 0; j < (int)reader_list.size(); j++)
				{
					if (reader == reader_list[j])
					{
						found = true;
						break;
					}
				}
				if (!found)
				{
					reader->LoadOffset(frame);
					reader_list.push_back(reader);
				}

				double spcx, spcy, spcz;
				vd->GetSpacings(spcx, spcy, spcz);

				Nrrd* data = reader->Convert(0, vd->GetCurChannel(), true);
				if (vd->Replace(data, true))
					vd->SetDisp(true);
				else
				{
					vd->SetDisp(false);
					continue;
				}

				std::wstring data_name = reader->GetDataName();
				if (i > 0)
					m_bat_folder += L"_";
				m_bat_folder += data_name;

				int chan_num = 0;
				if (data_name.find(L"_1ch") != std::wstring::npos)
					chan_num = 1;
				else if (data_name.find(L"_2ch") != std::wstring::npos)
					chan_num = 2;
				if (chan_num > 0 && vd->GetCurChannel() >= chan_num)
					vd->SetDisp(false);
				else
					vd->SetDisp(true);

				if (reader->GetChanNum() > 1)
					data_name += L"_" + std::to_wstring(vd->GetCurChannel() + 1);
				vd->SetName(data_name);
				vd->SetPath(reader->GetPathName());
				vd->SetCurTime(reader->GetCurTime());
				if (!reader->IsSpcInfoValid())
					vd->SetSpacings(spcx, spcy, spcz);
				else
					vd->SetSpacings(reader->GetXSpc(), reader->GetYSpc(), reader->GetZSpc());
				if (vd->GetVR())
					vd->GetVR()->clear_tex_pool();
			}
		}
	}

	InitView(INIT_BOUNDS | INIT_CENTER);

	//if (m_frame)
	//{
	//	VolumeData* vd = m_frame->GetCurSelVol();
	//	if (vd)
	//		glbin.set_tree_selection(vd->GetName().ToStdString());
	//	else
	//glbin.set_tree_selection("");
	glbin_current.mainframe->UpdateProps({ gstListCtrl, gstTreeCtrl });
}

void RenderView::Get3DBatRange(int &start_frame, int &end_frame)
{
	m_bat_folder = L"";
	int cur_t = -1;

	for (int i = 0; i<(int)m_vd_pop_list.size(); i++)
	{
		VolumeData* vd = m_vd_pop_list[i];
		if (vd && vd->GetReader())
		{
			BaseReader* reader = vd->GetReader();
			reader->SetBatch(true);

			int vd_cur_frame = reader->GetCurBatch();
			int vd_start_frame = -vd_cur_frame;
			int vd_end_frame = reader->GetBatchNum() - 1 - vd_cur_frame;
			if (cur_t < 0)
				cur_t = vd_cur_frame;

			if (i > 0)
				m_bat_folder += L"_";
			m_bat_folder += reader->GetDataName();

			if (i == 0)
			{
				//first dataset
				start_frame = vd_start_frame;
				end_frame = vd_end_frame;
			}
			else
			{
				//datasets after the first one
				if (vd_start_frame < start_frame)
					start_frame = vd_start_frame;
				if (vd_end_frame > end_frame)
					end_frame = vd_end_frame;
			}
		}
	}
	end_frame -= start_frame;
	start_frame = 0;
	m_end_all_frame = end_frame;
	if (cur_t > -1)
		m_tseq_cur_num = cur_t;
}

void RenderView::Set3DBatFrame(int frame, int start_frame, int end_frame, bool rewind)
{
	m_frame_num_type = 0;

	//skip update if frame num unchanged
	bool update = m_tseq_cur_num == frame ? false : true;
	//compute frame number
	m_begin_frame = start_frame;
	m_end_frame = end_frame;
	m_total_frames = std::abs(end_frame - start_frame + 1);

	//save currently selected volume
	VolumeData* cur_vd_save = m_cur_vol;

	//run pre-change script
	if (update && glbin_settings.m_run_script)
	{
		int r = glbin_script_proc.Run4DScript(flrd::ScriptProc::TM_ALL_PRE, rewind);
		if (r == 2)
		{
			glbin_moviemaker.Reset();
			return;
		}
	}

	//change time frame
	m_tseq_prv_num = m_tseq_cur_num;
	m_tseq_cur_num = frame;

	if (update)
		ReloadVolumeData(frame);

	//run post-change script
	if (update && glbin_settings.m_run_script)
		glbin_script_proc.Run4DScript(flrd::ScriptProc::TM_ALL_POST, rewind);

	//restore currently selected volume
	m_cur_vol = cur_vd_save;
	glbin_vol_calculator.SetVolumeA(m_cur_vol);

	//update ruler intensity values
	glbin_ruler_handler.ProfileAll();
	glbin_current.mainframe->UpdateProps({ gstRulerList });
}

void RenderView::CalcFrame()
{
	//int w, h;
	//w = GetGLSize().w();
	//h = GetGLSize().h();

	if (m_cur_vol)
	{
		//projection
		HandleProjection(m_gl_size.w(), m_gl_size.h());
		//Transformation
		HandleCamera();

		glm::mat4 mv_temp = GetDrawMat();
		fluo::Transform mv;
		fluo::Transform pr;
		mv.set(glm::value_ptr(mv_temp));
		pr.set(glm::value_ptr(m_proj_mat));

		double minx, maxx, miny, maxy;
		minx = 1.0;
		maxx = -1.0;
		miny = 1.0;
		maxy = -1.0;
		std::vector<fluo::Point> points;
		fluo::BBox bbox = m_cur_vol->GetBounds();
		points.push_back(fluo::Point(bbox.Min().x(), bbox.Min().y(), bbox.Min().z()));
		points.push_back(fluo::Point(bbox.Min().x(), bbox.Min().y(), bbox.Max().z()));
		points.push_back(fluo::Point(bbox.Min().x(), bbox.Max().y(), bbox.Min().z()));
		points.push_back(fluo::Point(bbox.Min().x(), bbox.Max().y(), bbox.Max().z()));
		points.push_back(fluo::Point(bbox.Max().x(), bbox.Min().y(), bbox.Min().z()));
		points.push_back(fluo::Point(bbox.Max().x(), bbox.Min().y(), bbox.Max().z()));
		points.push_back(fluo::Point(bbox.Max().x(), bbox.Max().y(), bbox.Min().z()));
		points.push_back(fluo::Point(bbox.Max().x(), bbox.Max().y(), bbox.Max().z()));

		fluo::Point p;
		for (unsigned int i = 0; i<points.size(); ++i)
		{
			p = mv.transform(points[i]);
			p = pr.transform(p);
			minx = p.x()<minx ? p.x() : minx;
			maxx = p.x()>maxx ? p.x() : maxx;
			miny = p.y()<miny ? p.y() : miny;
			maxy = p.y()>maxy ? p.y() : maxy;
		}

		minx = fluo::Clamp(minx, -1.0, 1.0);
		maxx = fluo::Clamp(maxx, -1.0, 1.0);
		miny = fluo::Clamp(miny, -1.0, 1.0);
		maxy = fluo::Clamp(maxy, -1.0, 1.0);

		glbin_moviemaker.SetCropX(std::round((minx + 1.0)*m_size.w() / 2.0 + 1.0));
		glbin_moviemaker.SetCropY(std::round((miny + 1.0)*m_size.h() / 2.0 + 1.0));
		glbin_moviemaker.SetCropW(std::round((maxx - minx)*m_size.w() / 2.0 - 1.5));
		glbin_moviemaker.SetCropH(std::round((maxy - miny)*m_size.h() / 2.0 - 1.5));

	}
	else
	{
		int size;
		if (m_size.w() > m_size.h())
		{
			size = m_size.h();
			glbin_moviemaker.SetCropX(std::round((m_size.w() - m_size.h()) / 2.0));
			glbin_moviemaker.SetCropY(0);
		}
		else
		{
			size = m_size.w();
			glbin_moviemaker.SetCropX(0);
			glbin_moviemaker.SetCropY(std::round((m_size.h() - m_size.w()) / 2.0));
		}
		glbin_moviemaker.SetCropW(size);
		glbin_moviemaker.SetCropH(size);
	}
}

//interactive modes
void RenderView::SetIntMode(int mode)
{
	m_int_mode = mode;
	if (m_int_mode == 1)
	{
		//glbin_vol_selector.SetMode(0);
		m_draw_brush = false;
	}
	//else if (m_int_mode == 10 ||
	//	m_int_mode == 12)
	//	glbin_vol_selector.SetMode(9);
}

//set use 2d rendering results
void RenderView::SetPaintUse2d(bool use2d)
{
	glbin_vol_selector.SetPaintUse2d(use2d);
}

bool RenderView::GetPaintUse2d()
{
	return glbin_vol_selector.GetPaintUse2d();
}

//calculations
void RenderView::SetVolumeA(VolumeData* vd)
{
	glbin_vol_calculator.SetVolumeA(vd);
}

void RenderView::SetVolumeB(VolumeData* vd)
{
	glbin_vol_calculator.SetVolumeB(vd);
}

//change brush display
void RenderView::ChangeBrushSize(int value, bool ctrl)
{
	glbin_vol_selector.ChangeBrushSize(value, ctrl);
	if (m_render_view_panel)
		m_render_view_panel->FluoRefresh(0, { gstBrushSize1, gstBrushSize2 }, {-1});
}

void RenderView::SetClipMode(int mode)
{
	switch (mode)
	{
	case 0:
		m_clip_mode = 0;
		RestorePlanes();
		m_rotx_cl = 0;
		m_roty_cl = 0;
		m_rotz_cl = 0;
		break;
	case 1:
		m_clip_mode = 1;
		SetRotations(fluo::Vector(m_rotx, m_roty, m_rotz), true);
		break;
	case 2:
		m_clip_mode = 2;
		{
			double rx, ry, rz;
			m_q.ToEuler(rx, ry, rz);
			m_q_cl_zero.FromEuler(rx, -ry, -rz);
		}
		m_q_cl = m_q_cl_zero;
		m_q_cl.ToEuler(m_rotx_cl, m_roty_cl, m_rotz_cl);
		if (m_rotx_cl > 180.0) m_rotx_cl -= 360.0;
		if (m_roty_cl > 180.0) m_roty_cl -= 360.0;
		if (m_rotz_cl > 180.0) m_rotz_cl -= 360.0;
		SetRotations(fluo::Vector(m_rotx, m_roty, m_rotz), true);
		break;
	}
}

void RenderView::RestorePlanes()
{
	std::vector<fluo::Plane*> *planes = 0;
	for (int i = 0; i<(int)m_vd_pop_list.size(); i++)
	{
		if (!m_vd_pop_list[i])
			continue;

		planes = 0;
		if (m_vd_pop_list[i]->GetVR())
			planes = m_vd_pop_list[i]->GetVR()->get_planes();
		if (planes && planes->size() == 6)
		{
			(*planes)[0]->Restore();
			(*planes)[1]->Restore();
			(*planes)[2]->Restore();
			(*planes)[3]->Restore();
			(*planes)[4]->Restore();
			(*planes)[5]->Restore();
		}
	}
}

void RenderView::ClipRotate()
{
	m_q_cl.FromEuler(m_rotx_cl, m_roty_cl, m_rotz_cl);
	m_q_cl.Normalize();

	SetRotations(fluo::Vector(m_rotx, m_roty, m_rotz), true);
}

void RenderView::SetClippingPlaneRotations(const fluo::Vector& val)
{
	m_rotx_cl = -val.x();
	m_roty_cl = val.y();
	m_rotz_cl = val.z();
	ClipRotate();
}

fluo::Vector RenderView::GetClippingPlaneRotations()
{
	return fluo::Vector(
		m_rotx_cl == 0.0 ? m_rotx_cl : -m_rotx_cl,
		m_roty_cl,
		m_rotz_cl);
}

void RenderView::SetClipRotX(double val)
{
	m_rotx_cl = -val;
	ClipRotate();
}

void RenderView::SetClipRotY(double val)
{
	m_roty_cl = val;
	ClipRotate();
}

void RenderView::SetClipRotZ(double val)
{
	m_rotz_cl = val;
	ClipRotate();
}

void RenderView::SetClipValue(int mask, int val)
{
	for (int i = 0; i < GetAllVolumeNum(); ++i)
	{
		VolumeData* vd = GetAllVolumeData(i);
		if (!vd)
			continue;
		vd->SetClipValue(mask, val);
	}
}

void RenderView::SetClipValues(int mask, int val1, int val2)
{
	for (int i = 0; i < GetAllVolumeNum(); ++i)
	{
		VolumeData* vd = GetAllVolumeData(i);
		if (!vd)
			continue;
		vd->SetClipValues(mask, val1, val2);
	}
}

void RenderView::SetClipValues(const int val[6])
{
	for (int i = 0; i < GetAllVolumeNum(); ++i)
	{
		VolumeData* vd = GetAllVolumeData(i);
		if (!vd)
			continue;
		vd->SetClipValues(val);
	}
}

void RenderView::ResetClipValues()
{
	for (int i = 0; i < GetAllVolumeNum(); ++i)
	{
		VolumeData* vd = GetAllVolumeData(i);
		if (!vd)
			continue;
		vd->ResetClipValues();
	}
}

void RenderView::ResetClipValuesX()
{
	for (int i = 0; i < GetAllVolumeNum(); ++i)
	{
		VolumeData* vd = GetAllVolumeData(i);
		if (!vd)
			continue;
		vd->ResetClipValuesX();
	}
}

void RenderView::ResetClipValuesY()
{
	for (int i = 0; i < GetAllVolumeNum(); ++i)
	{
		VolumeData* vd = GetAllVolumeData(i);
		if (!vd)
			continue;
		vd->ResetClipValuesY();
	}
}

void RenderView::ResetClipValuesZ()
{
	for (int i = 0; i < GetAllVolumeNum(); ++i)
	{
		VolumeData* vd = GetAllVolumeData(i);
		if (!vd)
			continue;
		vd->ResetClipValuesZ();
	}
}

flvr::TextRenderer* RenderView::GetTextRenderer()
{
	return m_text_renderer.get();
}

bool RenderView::ForceDraw()
{
	if (!m_refresh)
		m_retain_finalbuffer = true;
	else
		m_refresh = false;

	bool swap = false;

	Init();

	switch (glbin_settings.m_hologram_mode)
	{
	case 1:
		InitOpenXR();
		break;
	case 2:
		InitLookingGlass();
		break;
	}

	if (m_resize)
		m_retain_finalbuffer = false;

	bool intactive = m_interactive;
	int nx, ny;
	GetRenderSize(nx, ny);

	PopMeshList();
	if (m_md_pop_list.size()>0)
		m_draw_type = 2;
	else
		m_draw_type = 1;

	m_drawing = true;
	PreDraw();

	if (glbin_settings.m_hologram_mode == 1)
	{
		PrepVRBuffer();
		BindRenderBuffer();
	}
	else if (glbin_settings.m_hologram_mode == 2)
	{
		BindRenderBuffer();
	}

	switch (m_draw_type)
	{
	case 1:  //draw volumes only
		Draw();
		break;
	case 2:  //draw volumes and meshes with depth peeling
		DrawDP();
		break;
	}

	if (m_draw_camctr)
		DrawCamCtr();

	if (m_draw_legend)
		DrawLegend();

	if (m_disp_scale_bar)
		DrawScaleBar();

	if (m_draw_colormap)
		DrawColormap();

	PostDraw();

	//draw frame after capture
	if (m_draw_frame)
		DrawFrame();

	//draw info
	if (m_draw_info & INFO_DISP)
		DrawInfo(nx, ny, intactive);

	if (m_int_mode == 2 ||
		m_int_mode == 7)  //painting mode
	{
		if (m_draw_brush)
		{
			DrawBrush();
		}
		if (m_paint_enable)
		{
			//paiting mode
			//for volume segmentation
			PaintStroke();
		}
		if (m_paint_enable && m_paint_display)
		{
			//show the paint strokes
			DisplayStroke();
		}
	}

	if (m_int_mode == 4)
		m_int_mode = 2;
	if (m_int_mode == 8)
		m_int_mode = 7;


	if (glbin_settings.m_invalidate_tex)
	{
//#ifdef _WIN32
//		for (int i = 0; i < m_dp_tex_list.size(); ++i)
//			glInvalidateTexImage(m_dp_tex_list[i], 0);
//		glInvalidateTexImage(m_tex_paint, 0);
//		glInvalidateTexImage(m_tex_final, 0);
//		glInvalidateTexImage(m_tex, 0);
//		glInvalidateTexImage(m_tex_temp, 0);
//		glInvalidateTexImage(m_tex_wt2, 0);
//		glInvalidateTexImage(m_tex_ol1, 0);
//		glInvalidateTexImage(m_tex_ol2, 0);
//		glInvalidateTexImage(m_tex_pick, 0);
//		glInvalidateTexImage(m_tex_pick_depth, 0);
//#endif
	}

	//swap
	if (glbin_settings.m_hologram_mode == 1)
	{
		if (m_vr_eye_idx)
		{
			DrawVRBuffer();
			m_vr_eye_idx = 0;
			swap = true;
		}
		else
		{
			m_vr_eye_idx = 1;
			RefreshGL(99);
		}
	}
	else if (glbin_settings.m_hologram_mode == 2)
	{
		glbin_lg_renderer.Draw();
			swap = true;
	}
	else
			swap = true;

	glbin.getStopWatch(gstStopWatch)->sample();
	m_drawing = false;

	//DBGPRINT(L"buffer swapped\t%d\n", m_interactive);

	if (m_resize)
		m_resize = false;

	if (m_enlarge)
		ResetEnlarge();

	//need to add a root node to global to access the entire scene structure
	// this will be done later
	if (glbin_linked_rot)
	{
		RenderView* master_view = glbin_master_linked_view;
		if (!master_view ||
			this != master_view)
			return swap;

		Root* root = glbin_data_manager.GetRoot();
		if (root)
		{
			for (int i = 0; i < root->GetViewNum(); i++)
			{
				RenderView* view = root->GetView(i);
				if (view && view != this)
				{
					view->SetRotations(fluo::Vector(m_rotx, m_roty, m_rotz), true);
					view->RefreshGL(39);
					if (view->m_render_canvas)
						view->m_render_canvas->Update();
				}
			}
		}
	}

	return swap;
}

//start loop update
void RenderView::StartLoopUpdate()
{
	////this is for debug_ds, comment when done
	//if (TextureRenderer::get_mem_swap() &&
	//  TextureRenderer::get_start_update_loop() &&
	//  !TextureRenderer::get_done_update_loop())
	//  return;

	if (glbin_settings.m_mem_swap)
	{
		if (flvr::TextureRenderer::active_view_ > 0 &&
			flvr::TextureRenderer::active_view_ != Id())
			return;
		else
			flvr::TextureRenderer::active_view_ = Id();

		int nx, ny;
		GetRenderSize(nx, ny);
		//projection
		HandleProjection(nx, ny, true);
		//Transformation
		HandleCamera(true);
		glm::mat4 mv_temp = m_mv_mat;
		m_mv_mat = GetDrawMat();

		PopVolumeList();
		int total_num = 0;
		int num_chan;
		//reset drawn status for all bricks
		int i, j, k;
		for (i = 0; i<m_vd_pop_list.size(); i++)
		{
			VolumeData* vd = m_vd_pop_list[i];
			if (vd)
			{
				switchLevel(vd);
				vd->SetMatrices(m_mv_mat, m_proj_mat, m_tex_mat);

				num_chan = 0;
				flvr::Texture* tex = vd->GetTexture();
				if (tex)
				{
					fluo::Transform *tform = tex->transform();
					double mvmat[16];
					tform->get_trans(mvmat);
					vd->GetVR()->m_mv_mat2 = glm::mat4(
						mvmat[0], mvmat[4], mvmat[8], mvmat[12],
						mvmat[1], mvmat[5], mvmat[9], mvmat[13],
						mvmat[2], mvmat[6], mvmat[10], mvmat[14],
						mvmat[3], mvmat[7], mvmat[11], mvmat[15]);
					vd->GetVR()->m_mv_mat2 = vd->GetVR()->m_mv_mat *
						vd->GetVR()->m_tex_mat * vd->GetVR()->m_mv_mat2;

					fluo::Ray view_ray = vd->GetVR()->compute_view();
					std::vector<flvr::TextureBrick*> *bricks = 0;
					bricks = tex->get_sorted_bricks(view_ray, !m_persp);
					if (!bricks || bricks->size() == 0)
						continue;
					for (j = 0; j<bricks->size(); j++)
					{
						(*bricks)[j]->set_drawn(false);
						if ((*bricks)[j]->get_priority() > 0 ||
							!vd->GetVR()->test_against_view((*bricks)[j]->bbox(), m_persp))
						{
							(*bricks)[j]->set_disp(false);
							continue;
						}
						else
							(*bricks)[j]->set_disp(true);
						total_num++;
						num_chan++;
						if (vd->GetMode() == 1 &&
							vd->GetShadingEnable())
							total_num++;
						if (vd->GetShadowEnable())
							total_num++;
						//mask
						if (vd->GetTexture() &&
							vd->GetTexture()->nmask() != -1 &&
							(!vd->GetLabelMode() ||
							(vd->GetLabelMode() &&
							vd->GetTexture()->nlabel() == -1)))
							total_num++;
					}
				}
				vd->SetBrickNum(num_chan);
				if (vd->GetVR())
					vd->GetVR()->set_done_loop(false);
			}
		}

		std::vector<VolumeLoaderData> queues;
		if (m_vol_method == VOL_METHOD_MULTI)
		{
			std::vector<VolumeData*> list;
			for (i = 0; i<m_vd_pop_list.size(); i++)
			{
				VolumeData* vd = m_vd_pop_list[i];
				if (!vd || !vd->GetDisp() || !vd->isBrxml())
					continue;
				flvr::Texture* tex = vd->GetTexture();
				if (!tex)
					continue;
				std::vector<flvr::TextureBrick*> *bricks = tex->get_bricks();
				if (!bricks || bricks->size() == 0)
					continue;
				list.push_back(vd);
			}

			std::vector<VolumeLoaderData> tmp_shade;
			std::vector<VolumeLoaderData> tmp_shadow;
			for (i = 0; i < list.size(); i++)
			{
				VolumeData* vd = list[i];
				flvr::Texture* tex = vd->GetTexture();
				fluo::Ray view_ray = vd->GetVR()->compute_view();
				std::vector<flvr::TextureBrick*> *bricks = tex->get_sorted_bricks(view_ray, !m_persp);
				int mode = vd->GetMode() == 1 ? 1 : 0;
				bool shade = (mode == 1 && vd->GetShadingEnable());
				bool shadow = vd->GetShadowEnable();
				for (j = 0; j < bricks->size(); j++)
				{
					VolumeLoaderData d = {};
					flvr::TextureBrick* b = (*bricks)[j];
					if (b->get_disp())
					{
						d.brick = b;
						d.finfo = tex->GetFileName(b->getID());
						d.vd = vd;
						if (!b->drawn(mode))
						{
							d.mode = mode;
							queues.push_back(d);
						}
						if (shade && !b->drawn(2))
						{
							d.mode = 2;
							tmp_shade.push_back(d);
						}
						if (shadow && !b->drawn(3))
						{
							d.mode = 3;
							tmp_shadow.push_back(d);
						}
					}
				}
			}
			if (glbin_settings.m_update_order == 1)
				std::sort(queues.begin(), queues.end(), VolumeLoader::sort_data_dsc);
			else if (glbin_settings.m_update_order == 0)
				std::sort(queues.begin(), queues.end(), VolumeLoader::sort_data_asc);

			if (!tmp_shade.empty())
			{
				if (glbin_settings.m_update_order == 1)
					std::sort(tmp_shade.begin(), tmp_shade.end(), VolumeLoader::sort_data_dsc);
				else if (glbin_settings.m_update_order == 0)
					std::sort(tmp_shade.begin(), tmp_shade.end(), VolumeLoader::sort_data_asc);
				queues.insert(queues.end(), tmp_shade.begin(), tmp_shade.end());
			}
			if (!tmp_shadow.empty())
			{
				if (glbin_settings.m_update_order == 1)
				{
					int order = glbin_settings.m_update_order;
					glbin_settings.m_update_order = 0;
					for (i = 0; i < list.size(); i++)
					{
						fluo::Ray view_ray = list[i]->GetVR()->compute_view();
						list[i]->GetTexture()->set_sort_bricks();
						list[i]->GetTexture()->get_sorted_bricks(view_ray, !m_persp); //recalculate brick.d_
						list[i]->GetTexture()->set_sort_bricks();
					}
					glbin_settings.m_update_order = order;
					std::sort(tmp_shadow.begin(), tmp_shadow.end(), VolumeLoader::sort_data_asc);
				}
				else if (glbin_settings.m_update_order == 0)
					std::sort(tmp_shadow.begin(), tmp_shadow.end(), VolumeLoader::sort_data_asc);
				queues.insert(queues.end(), tmp_shadow.begin(), tmp_shadow.end());
			}
		}
		else if (m_layer_list.size() > 0)
		{
			for (i = (int)m_layer_list.size() - 1; i >= 0; i--)
			{
				if (!m_layer_list[i])
					continue;
				switch (m_layer_list[i]->IsA())
				{
				case 2://volume data (this won't happen now)
				{
					VolumeData* vd = (VolumeData*)m_layer_list[i];
					std::vector<VolumeLoaderData> tmp_shade;
					std::vector<VolumeLoaderData> tmp_shadow;
					if (vd && vd->GetDisp() && vd->isBrxml())
					{
						flvr::Texture* tex = vd->GetTexture();
						if (!tex)
							continue;
						fluo::Ray view_ray = vd->GetVR()->compute_view();
						std::vector<flvr::TextureBrick*> *bricks = tex->get_sorted_bricks(view_ray, !m_persp);
						if (!bricks || bricks->size() == 0)
							continue;
						int mode = vd->GetMode() == 1 ? 1 : 0;
						bool shade = (mode == 1 && vd->GetShadingEnable());
						bool shadow = vd->GetShadowEnable();
						for (j = 0; j<bricks->size(); j++)
						{
							VolumeLoaderData d = {};
							flvr::TextureBrick* b = (*bricks)[j];
							if (b->get_disp())
							{
								d.brick = b;
								d.finfo = tex->GetFileName(b->getID());
								d.vd = vd;
								if (!b->drawn(mode))
								{
									d.mode = mode;
									queues.push_back(d);
								}
								if (shade && !b->drawn(2))
								{
									d.mode = 2;
									tmp_shade.push_back(d);
								}
								if (shadow && !b->drawn(3))
								{
									d.mode = 3;
									tmp_shadow.push_back(d);
								}
							}
						}
						if (!tmp_shade.empty()) queues.insert(queues.end(), tmp_shade.begin(), tmp_shade.end());
						if (!tmp_shadow.empty())
						{
							if (glbin_settings.m_update_order == 1)
							{
								int order = glbin_settings.m_update_order;
								glbin_settings.m_update_order = 0;
								fluo::Ray view_ray = vd->GetVR()->compute_view();
								tex->set_sort_bricks();
								tex->get_sorted_bricks(view_ray, !m_persp); //recalculate brick.d_
								tex->set_sort_bricks();
								glbin_settings.m_update_order = order;
								std::sort(tmp_shadow.begin(), tmp_shadow.end(), VolumeLoader::sort_data_asc);
							}
							queues.insert(queues.end(), tmp_shadow.begin(), tmp_shadow.end());
						}
					}
				}
				break;
				case 5://group
				{
					std::vector<VolumeData*> list;
					DataGroup* group = (DataGroup*)m_layer_list[i];
					if (!group->GetDisp())
						continue;
					for (j = group->GetVolumeNum() - 1; j >= 0; j--)
					{
						VolumeData* vd = group->GetVolumeData(j);
						if (!vd || !vd->GetDisp() || !vd->isBrxml())
							continue;
						flvr::Texture* tex = vd->GetTexture();
						if (!tex)
							continue;
						fluo::Ray view_ray = vd->GetVR()->compute_view();
						std::vector<flvr::TextureBrick*> *bricks = tex->get_sorted_bricks(view_ray, !m_persp);
						if (!bricks || bricks->size() == 0)
							continue;
						list.push_back(vd);
					}
					if (list.empty())
						continue;

					std::vector<VolumeLoaderData> tmp_q;
					std::vector<VolumeLoaderData> tmp_shade;
					std::vector<VolumeLoaderData> tmp_shadow;
					if (group->GetBlendMode() == VOL_METHOD_MULTI)
					{
						for (k = 0; k < list.size(); k++)
						{
							VolumeData* vd = list[k];
							flvr::Texture* tex = vd->GetTexture();
							fluo::Ray view_ray = vd->GetVR()->compute_view();
							std::vector<flvr::TextureBrick*> *bricks = tex->get_sorted_bricks(view_ray, !m_persp);
							int mode = vd->GetMode() == 1 ? 1 : 0;
							bool shade = (mode == 1 && vd->GetShadingEnable());
							bool shadow = vd->GetShadowEnable();
							for (j = 0; j < bricks->size(); j++)
							{
								VolumeLoaderData d = {};
								flvr::TextureBrick* b = (*bricks)[j];
								if (b->get_disp())
								{
									d.brick = b;
									d.finfo = tex->GetFileName(b->getID());
									d.vd = vd;
									if (!b->drawn(mode))
									{
										d.mode = mode;
										tmp_q.push_back(d);
									}
									if (shade && !b->drawn(2))
									{
										d.mode = 2;
										tmp_shade.push_back(d);
									}
									if (shadow && !b->drawn(3))
									{
										d.mode = 3;
										tmp_shadow.push_back(d);
									}
								}
							}
						}
						if (!tmp_q.empty())
						{
							if (glbin_settings.m_update_order == 1)
								std::sort(tmp_q.begin(), tmp_q.end(), VolumeLoader::sort_data_dsc);
							else if (glbin_settings.m_update_order == 0)
								std::sort(tmp_q.begin(), tmp_q.end(), VolumeLoader::sort_data_asc);
							queues.insert(queues.end(), tmp_q.begin(), tmp_q.end());
						}
						if (!tmp_shade.empty())
						{
							if (glbin_settings.m_update_order == 1)
								std::sort(tmp_shade.begin(), tmp_shade.end(), VolumeLoader::sort_data_dsc);
							else if (glbin_settings.m_update_order == 0)
								std::sort(tmp_shade.begin(), tmp_shade.end(), VolumeLoader::sort_data_asc);
							queues.insert(queues.end(), tmp_shade.begin(), tmp_shade.end());
						}
						if (!tmp_shadow.empty())
						{
							if (glbin_settings.m_update_order == 1)
							{
								int order = glbin_settings.m_update_order;
								glbin_settings.m_update_order = 0;
								for (k = 0; k < list.size(); k++)
								{
									fluo::Ray view_ray = list[k]->GetVR()->compute_view();
									list[i]->GetTexture()->set_sort_bricks();
									list[i]->GetTexture()->get_sorted_bricks(view_ray, !m_persp); //recalculate brick.d_
									list[i]->GetTexture()->set_sort_bricks();
								}
								glbin_settings.m_update_order = order;
								std::sort(tmp_shadow.begin(), tmp_shadow.end(), VolumeLoader::sort_data_asc);
							}
							else if (glbin_settings.m_update_order == 0)
								std::sort(tmp_shadow.begin(), tmp_shadow.end(), VolumeLoader::sort_data_asc);
							queues.insert(queues.end(), tmp_shadow.begin(), tmp_shadow.end());
						}
					}
					else
					{
						for (j = 0; j < list.size(); j++)
						{
							VolumeData* vd = list[j];
							flvr::Texture* tex = vd->GetTexture();
							fluo::Ray view_ray = vd->GetVR()->compute_view();
							std::vector<flvr::TextureBrick*> *bricks = tex->get_sorted_bricks(view_ray, !m_persp);
							int mode = vd->GetMode() == 1 ? 1 : 0;
							bool shade = (mode == 1 && vd->GetShadingEnable());
							bool shadow = vd->GetShadowEnable();
							for (k = 0; k<bricks->size(); k++)
							{
								VolumeLoaderData d = {};
								flvr::TextureBrick* b = (*bricks)[k];
								if (b->get_disp())
								{
									d.brick = b;
									d.finfo = tex->GetFileName(b->getID());
									d.vd = vd;
									if (!b->drawn(mode))
									{
										d.mode = mode;
										queues.push_back(d);
									}
									if (shade && !b->drawn(2))
									{
										d.mode = 2;
										tmp_shade.push_back(d);
									}
									if (shadow && !b->drawn(3))
									{
										d.mode = 3;
										tmp_shadow.push_back(d);
									}
								}
							}
							if (!tmp_shade.empty()) queues.insert(queues.end(), tmp_shade.begin(), tmp_shade.end());
							if (!tmp_shadow.empty())
							{
								if (glbin_settings.m_update_order == 1)
								{
									int order = glbin_settings.m_update_order;
									glbin_settings.m_update_order = 0;
									fluo::Ray view_ray = vd->GetVR()->compute_view();
									tex->set_sort_bricks();
									tex->get_sorted_bricks(view_ray, !m_persp); //recalculate brick.d_
									tex->set_sort_bricks();
									glbin_settings.m_update_order = order;
									std::sort(tmp_shadow.begin(), tmp_shadow.end(), VolumeLoader::sort_data_asc);
								}
								queues.insert(queues.end(), tmp_shadow.begin(), tmp_shadow.end());
							}
						}
					}

				}
				break;
				}
			}
		}

		if (queues.size() > 0 /*&& !m_interactive*/)
		{
			glbin_vol_loader.Set(queues);
			glbin_vol_loader.SetMemoryLimitByte((long long)flvr::TextureRenderer::mainmem_buf_size_ * 1024LL * 1024LL);
			flvr::TextureRenderer::set_load_on_main_thread(false);
			glbin_vol_loader.Run();
		}

		if (total_num > 0)
		{
			flvr::TextureRenderer::set_update_loop();
			if (m_draw_type == 1)
				flvr::TextureRenderer::set_total_brick_num(total_num);
			else if (m_draw_type == 2)
				flvr::TextureRenderer::set_total_brick_num(total_num*(glbin_settings.m_peeling_layers+1));
			flvr::TextureRenderer::reset_done_current_chan();
		}
	}
}

//halt loop update
void RenderView::HaltLoopUpdate()
{
	if (glbin_settings.m_mem_swap)
	{
		flvr::TextureRenderer::reset_update_loop();
	}
}

//new function to refresh
void RenderView::RefreshGL(int debug_code,
	bool erase,
	bool start_loop,
	bool lg_changed)
{
	//m_force_clear = force_clear;
	//m_interactive = interactive;

	//for debugging refresh events
	//DBGPRINT(L"%d\trefresh\t%d\t%d\n", m_render_view_panel->m_id, debug_code, m_interactive);
	//if (!m_interactive)
	//	m_frame->UpdateProps();

	m_updating = true;
	if (start_loop)
		StartLoopUpdate();
	SetSortBricks();
	m_refresh = true;
	glbin_lg_renderer.SetUpdating(lg_changed);
	if (m_render_canvas)
		m_render_canvas->Refresh(erase);
	//Update();
}

void RenderView::DrawRulers()
{
	if (m_ruler_list->empty())
		return;
	double width = glbin_settings.m_line_width;
	glbin_ruler_renderer.SetLineSize(width);
	glbin_ruler_renderer.SetView(this);
	glbin_ruler_renderer.SetRulerList(m_ruler_list.get());
	glbin_ruler_renderer.Draw();
}

flrd::RulerList* RenderView::GetRulerList()
{
	return m_ruler_list.get();
}

void RenderView::SetCurRuler(flrd::Ruler* ruler)
{
	m_cur_ruler = ruler;
}

flrd::Ruler* RenderView::GetCurRuler()
{
	return m_cur_ruler;
}

flrd::Ruler* RenderView::GetRuler(unsigned int id)
{
	m_cur_ruler = 0;
	for (size_t i = 0; i < m_ruler_list->size(); ++i)
	{
		if ((*m_ruler_list)[i] && (*m_ruler_list)[i]->Id() == id)
		{
			m_cur_ruler = (*m_ruler_list)[i];
			break;
		}
	}
	return m_cur_ruler;
}

//draw highlighted comps
void RenderView::DrawCells()
{
	if (m_cell_list->empty())
		return;
	double width = glbin_settings.m_line_width;

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	flvr::ShaderProgram* shader =
		glbin_img_shader_factory.shader(IMG_SHDR_DRAW_THICK_LINES);
	if (shader)
	{
		if (!shader->valid())
			shader->create();
		shader->bind();
	}
	glm::mat4 matrix = glm::ortho(float(0),
		float(m_gl_size.w()), float(0), float(m_gl_size.h()));
	shader->setLocalParamMatrix(0, glm::value_ptr(matrix));
	shader->setLocalParam(0, m_size.w(), m_size.h(), width, 0.0);

	flvr::VertexArray* va_rulers =
		glbin_vertex_array_manager.vertex_array(flvr::VA_Rulers);
	if (va_rulers)
	{
		std::vector<float> verts;
		unsigned int num = DrawCellVerts(verts);
		if (num)
		{
			va_rulers->buffer_data(flvr::VABuf_Coord,
				sizeof(float)*verts.size(),
				&verts[0], GL_STREAM_DRAW);
			va_rulers->set_param(0, num);
			va_rulers->draw();
		}
	}

	if (shader && shader->valid())
		shader->release();
}

unsigned int RenderView::DrawCellVerts(std::vector<float>& verts)
{
	float w = glbin_text_tex_manager.GetSize() / 4.0f;
	float px = 0, py = 0;

	fluo::Transform mv;
	fluo::Transform p;
	mv.set(glm::value_ptr(m_mv_mat));
	p.set(glm::value_ptr(m_proj_mat));

	//estimate
	size_t vert_num = m_cell_list->size();
	verts.reserve(vert_num * 48);

	unsigned int num = 0;
	fluo::Point p1, p2, p3, p4;
	fluo::Color c = GetTextColor();
	double sx, sy, sz;
	sx = m_cell_list->sx;
	sy = m_cell_list->sy;
	sz = m_cell_list->sz;
	for (auto it = m_cell_list->begin();
		it != m_cell_list->end(); ++it)
	{
		fluo::BBox box = it->second->GetBox(sx, sy, sz);
		GetCellPoints(box, p1, p2, p3, p4, mv, p);

		verts.push_back(p1.x()); verts.push_back(p1.y()); verts.push_back(0.0);
		verts.push_back(c.r()); verts.push_back(c.g()); verts.push_back(c.b());
		verts.push_back(p2.x()); verts.push_back(p2.y()); verts.push_back(0.0);
		verts.push_back(c.r()); verts.push_back(c.g()); verts.push_back(c.b());
		verts.push_back(p2.x()); verts.push_back(p2.y()); verts.push_back(0.0);
		verts.push_back(c.r()); verts.push_back(c.g()); verts.push_back(c.b());
		verts.push_back(p3.x()); verts.push_back(p3.y()); verts.push_back(0.0);
		verts.push_back(c.r()); verts.push_back(c.g()); verts.push_back(c.b());
		verts.push_back(p3.x()); verts.push_back(p3.y()); verts.push_back(0.0);
		verts.push_back(c.r()); verts.push_back(c.g()); verts.push_back(c.b());
		verts.push_back(p4.x()); verts.push_back(p4.y()); verts.push_back(0.0);
		verts.push_back(c.r()); verts.push_back(c.g()); verts.push_back(c.b());
		verts.push_back(p4.x()); verts.push_back(p4.y()); verts.push_back(0.0);
		verts.push_back(c.r()); verts.push_back(c.g()); verts.push_back(c.b());
		verts.push_back(p1.x()); verts.push_back(p1.y()); verts.push_back(0.0);
		verts.push_back(c.r()); verts.push_back(c.g()); verts.push_back(c.b());
		num += 8;
	}

	return num;
}

void RenderView::GetCellPoints(fluo::BBox& box,
	fluo::Point& p1, fluo::Point& p2, fluo::Point& p3, fluo::Point& p4,
	fluo::Transform& mv, fluo::Transform& p)
{
	//get 6 points of the jack of bbox
	fluo::Point pp[6];
	fluo::Point c = box.center();
	pp[0] = fluo::Point(box.Min().x(), c.y(), c.z());
	pp[1] = fluo::Point(box.Max().x(), c.y(), c.z());
	pp[2] = fluo::Point(c.x(), box.Min().y(), c.z());
	pp[3] = fluo::Point(c.x(), box.Max().y(), c.z());
	pp[4] = fluo::Point(c.x(), c.y(), box.Min().z());
	pp[5] = fluo::Point(c.x(), c.y(), box.Max().z());

	double minx = std::numeric_limits<double>::max();
	double maxx = -std::numeric_limits<double>::max();
	double miny = std::numeric_limits<double>::max();
	double maxy = -std::numeric_limits<double>::max();

	for (int i = 0; i < 6; ++i)
	{
		pp[i] = mv.transform(pp[i]);
		pp[i] = p.transform(pp[i]);
		minx = std::min(minx, pp[i].x());
		maxx = std::max(maxx, pp[i].x());
		miny = std::min(miny, pp[i].y());
		maxy = std::max(maxy, pp[i].y());
	}

	int nx = m_gl_size.w();
	int ny = m_gl_size.h();

	p1 = fluo::Point((minx+1)*nx/2, (miny+1)*ny/2, 0.0);
	p2 = fluo::Point((maxx+1)*nx/2, (miny+1)*ny/2, 0.0);
	p3 = fluo::Point((maxx+1)*nx/2, (maxy+1)*ny/2, 0.0);
	p4 = fluo::Point((minx+1)*nx/2, (maxy+1)*ny/2, 0.0);
}

//traces
int RenderView::GetTrackFileExist(bool save)
{
	if (!m_track_group)
		return 0;
	if (!m_track_group->GetTrackMap())
		return 0;
	if (!m_track_group->GetTrackMap()->GetFrameNum())
		return 0;
	std::wstring filename = m_track_group->GetPath();
	if (std::filesystem::exists(filename))
	{
		if (save)
			SaveTrackGroup(filename);
		return 2;
	}
	else
		return 1;
}

TrackGroup* RenderView::GetTrackGroup()
{
	return m_track_group;
}

std::wstring RenderView::GetTrackGroupFile()
{
	std::wstring str;
	if (m_track_group)
		str = m_track_group->GetPath();
	return str;
}

void RenderView::CreateTrackGroup()
{
	if (m_track_group)
		delete m_track_group;

	m_track_group = new TrackGroup;
}

int RenderView::LoadTrackGroup(const std::wstring& filename)
{
	if (m_track_group)
		delete m_track_group;

	m_track_group = new TrackGroup;
	return m_track_group->Load(filename);
}

int RenderView::SaveTrackGroup(const std::wstring& filename)
{
	if (m_track_group)
		return m_track_group->Save(filename);
	else
		return 0;
}

void RenderView::ExportTrackGroup(const std::wstring& filename, unsigned int id)
{
	if (!m_track_group)
		return;
}

void RenderView::DrawTraces()
{
	if (m_cur_vol &&
		m_track_group)
	{
		double width = glbin_settings.m_line_width;

		//glEnable(GL_LINE_SMOOTH);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

		double spcx, spcy, spcz;
		m_cur_vol->GetSpacings(spcx, spcy, spcz);
		glm::mat4 matrix = glm::scale(m_mv_mat,
			glm::vec3(float(spcx), float(spcy), float(spcz)));
		matrix = m_proj_mat*matrix;

		flvr::ShaderProgram* shader =
			glbin_img_shader_factory.shader(IMG_SHDR_DRAW_THICK_LINES);
		if (shader)
		{
			if (!shader->valid())
				shader->create();
			shader->bind();
		}
		shader->setLocalParamMatrix(0, glm::value_ptr(matrix));
		shader->setLocalParam(0, m_size.w(), m_size.h(), width, 0.0);

		flvr::VertexArray* va_traces =
			glbin_vertex_array_manager.vertex_array(flvr::VA_Traces);
		if (va_traces)
		{
			if (va_traces->get_dirty())
			{
				std::vector<float> verts;
				unsigned int num = m_track_group->Draw(verts, m_cur_vol->GetShuffle());
				if (num)
				{
					va_traces->buffer_data(flvr::VABuf_Coord,
						sizeof(float)*verts.size(),
						&verts[0], GL_STREAM_DRAW);
					va_traces->set_param(0, num);
					va_traces->draw();
				}
			}
			else
				va_traces->draw();
		}

		if (shader && shader->valid())
			shader->release();
		//glDisable(GL_LINE_SMOOTH);
	}
}

void RenderView::GetTraces(bool update)
{
	if (!m_track_group)
		return;

	int ii, jj, kk;
	int nx, ny, nz;
	//return current mask (into system memory)
	if (!m_cur_vol) return;
	m_cur_vol->GetVR()->return_mask();
	m_cur_vol->GetResolution(nx, ny, nz);
	//find labels in the old that are selected by the current mask
	Nrrd* mask_nrrd = m_cur_vol->GetMask(true);
	if (!mask_nrrd) return;
	Nrrd* label_nrrd = m_cur_vol->GetLabel(true);
	if (!label_nrrd) return;
	unsigned char* mask_data = (unsigned char*)(mask_nrrd->data);
	if (!mask_data) return;
	unsigned int* label_data = (unsigned int*)(label_nrrd->data);
	if (!label_data) return;
	flrd::CelpList sel_labels;
	flrd::CelpListIter label_iter;
	for (ii = 0; ii<nx; ii++)
	for (jj = 0; jj<ny; jj++)
	for (kk = 0; kk<nz; kk++)
	{
		int index = nx*ny*kk + nx*jj + ii;
		unsigned int label_value = label_data[index];
		if (mask_data[index] && label_value)
		{
			label_iter = sel_labels.find(label_value);
			if (label_iter == sel_labels.end())
			{
				flrd::Celp cell(new flrd::Cell(label_value));
				cell->Inc(ii, jj, kk, 1.0f);
				sel_labels.insert(std::pair<unsigned int, flrd::Celp>
					(label_value, cell));
			}
			else
			{
				label_iter->second->Inc(ii, jj, kk, 1.0f);
			}
		}
	}

	//create id list
	m_track_group->SetCurTime(m_tseq_cur_num);
	m_track_group->SetPrvTime(m_tseq_cur_num);
	m_track_group->UpdateCellList(sel_labels);
	//m_track_group->SetPrvTime(m_tseq_prv_num);

	//add traces to trace dialog
	//if (update)
	//{
	//	if (m_render_view_panel && m_frame && m_frame->GetTrackDlg())
	//		m_frame->GetTrackDlg()->GetSettings(m_render_view_panel->m_canvas);
	//}
}

void RenderView::SetEnlarge(bool value)
{
	if (m_enlarge && !value)
		glbin_text_tex_manager.SetEnlargeScale(1);
	m_enlarge = value;
	if (m_enlarge)
	{
		m_gl_size = Size2D(static_cast<int>(std::round(m_size.w() * m_enlarge_scale)),
			static_cast<int>(std::round(m_size.h() * m_enlarge_scale)));
	}
	else
		m_gl_size = m_size;
}

void RenderView::SetEnlargeScale(double value)
{
	m_enlarge_scale = value;
	if (m_enlarge)
	{
		glbin_text_tex_manager.SetEnlargeScale(m_enlarge_scale);
		m_gl_size = Size2D(static_cast<int>(std::round(m_size.w() * m_enlarge_scale)),
			static_cast<int>(std::round(m_size.h() * m_enlarge_scale)));
	}
	else
		m_gl_size = m_size;
}

//read pixels
void RenderView::ReadPixels(
	int chann, bool fp32,
	int &x, int &y, int &w, int &h,
	void** image)
{
	if (m_draw_frame)
	{
		if (m_enlarge)
		{
			x = glbin_moviemaker.GetCropX() * m_enlarge_scale;
			y = glbin_moviemaker.GetCropY() * m_enlarge_scale;
			w = glbin_moviemaker.GetCropW() * m_enlarge_scale;
			h = glbin_moviemaker.GetCropH() * m_enlarge_scale;
		}
		else
		{
			x = glbin_moviemaker.GetCropX();
			y = glbin_moviemaker.GetCropY();
			w = glbin_moviemaker.GetCropW();
			h = glbin_moviemaker.GetCropH();
		}
	}
	else
	{
		x = 0;
		y = 0;
		w = m_gl_size.w();
		h = m_gl_size.h();
	}

	if (m_enlarge || fp32)
	{
		glActiveTexture(GL_TEXTURE0);
		flvr::Framebuffer* final_buffer =
			glbin_framebuffer_manager.framebuffer(
				"final");
		if (final_buffer)
		{
			//draw the final buffer to itself
			final_buffer->bind();
			final_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
		}
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_DEPTH_TEST);

		//2d adjustment
		flvr::ShaderProgram* img_shader =
			glbin_img_shader_factory.shader(IMG_SHDR_BLEND_BRIGHT_BACKGROUND_HDR);
		if (img_shader)
		{
			if (!img_shader->valid())
				img_shader->create();
			img_shader->bind();
		}
		img_shader->setLocalParam(0, m_gamma.r(), m_gamma.g(), m_gamma.b(), 1.0);
		img_shader->setLocalParam(1, m_brightness.r(), m_brightness.g(), m_brightness.b(), 1.0);
		img_shader->setLocalParam(2, m_hdr.r(), m_hdr.g(), m_hdr.b(), 0.0);
		//2d adjustment

		DrawViewQuad();

		if (img_shader && img_shader->valid())
			img_shader->release();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glPixelStorei(GL_PACK_ROW_LENGTH, w);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	if (fp32)
		*image = new float[w*h*chann];
	else
		*image = new unsigned char[w*h*chann];
	glReadBuffer(GL_BACK);
	glReadPixels(x, y, w, h,
		chann == 3 ? GL_RGB : GL_RGBA,
		fp32 ? GL_FLOAT : GL_UNSIGNED_BYTE, *image);
	glPixelStorei(GL_PACK_ROW_LENGTH, 0);

	if (m_enlarge || fp32)
		BindRenderBuffer();
}

//set cell list
void RenderView::SetCellList(flrd::CelpList& list)
{
	*m_cell_list = list;
}

glm::mat4 RenderView::GetObjectMat()
{
	glm::mat4 obj_mat = m_mv_mat;
	//translate object
	obj_mat = glm::translate(obj_mat, glm::vec3(
		m_obj_transx,
		m_obj_transy,
		m_obj_transz));

	if (m_offset)
	{
		obj_mat = glm::translate(obj_mat, glm::vec3(
			m_obj_rot_ctr_offx - m_obj_ctrx,
			m_obj_ctry - m_obj_rot_ctr_offy,
			m_obj_rot_ctr_offz - m_obj_ctrz));
		//rotate object
		obj_mat = glm::rotate(obj_mat, float(glm::radians(m_obj_roty + m_obj_rot_offy)), glm::vec3(0.0, 1.0, 0.0));
		obj_mat = glm::rotate(obj_mat, float(glm::radians(m_obj_rotz + m_obj_rot_offz)), glm::vec3(0.0, 0.0, 1.0));
		obj_mat = glm::rotate(obj_mat, float(glm::radians(m_obj_rotx + m_obj_rot_offx)), glm::vec3(1.0, 0.0, 0.0));
		//center object
		obj_mat = glm::translate(obj_mat, glm::vec3(
			-m_obj_rot_ctr_offx - m_obj_ctr_offx,
			-m_obj_rot_ctr_offy - m_obj_ctr_offy,
			-m_obj_rot_ctr_offz - m_obj_ctr_offz));
	}
	else
	{
		//rotate object
		obj_mat = glm::rotate(obj_mat, float(glm::radians(m_obj_roty)), glm::vec3(0.0, 1.0, 0.0));
		obj_mat = glm::rotate(obj_mat, float(glm::radians(m_obj_rotz)), glm::vec3(0.0, 0.0, 1.0));
		obj_mat = glm::rotate(obj_mat, float(glm::radians(m_obj_rotx)), glm::vec3(1.0, 0.0, 0.0));
		//center object
		obj_mat = glm::translate(obj_mat, glm::vec3(
			-m_obj_ctrx,
			-m_obj_ctry,
			-m_obj_ctrz));
	}

	return obj_mat;
}

glm::mat4 RenderView::GetDrawMat()
{
	glm::mat4 drw_mat = m_mv_mat;
	//translate object
	drw_mat = glm::translate(drw_mat, glm::vec3(
		m_obj_transx,
		m_obj_transy,
		m_obj_transz));

	if (m_offset)
	{
		drw_mat = glm::translate(drw_mat, glm::vec3(
			m_obj_rot_ctr_offx - m_obj_ctrx,
			m_obj_ctry - m_obj_rot_ctr_offy,
			m_obj_rot_ctr_offz - m_obj_ctrz));
		//rotate object
		drw_mat = glm::rotate(drw_mat, float(glm::radians(m_obj_rotx + m_obj_rot_offx)), glm::vec3(1.0, 0.0, 0.0));
		drw_mat = glm::rotate(drw_mat, float(glm::radians(m_obj_roty + m_obj_rot_offy)), glm::vec3(0.0, 1.0, 0.0));
		drw_mat = glm::rotate(drw_mat, float(glm::radians(m_obj_rotz + m_obj_rot_offz)), glm::vec3(0.0, 0.0, 1.0));
		//center object
		drw_mat = glm::translate(drw_mat, glm::vec3(
			-m_obj_rot_ctr_offx - m_obj_ctr_offx,
			-m_obj_rot_ctr_offy - m_obj_ctr_offy,
			-m_obj_rot_ctr_offz - m_obj_ctr_offz));
	}
	else
	{
		//rotate object
		drw_mat = glm::rotate(drw_mat, float(glm::radians(m_obj_rotx)), glm::vec3(1.0, 0.0, 0.0));
		drw_mat = glm::rotate(drw_mat, float(glm::radians(m_obj_roty)), glm::vec3(0.0, 1.0, 0.0));
		drw_mat = glm::rotate(drw_mat, float(glm::radians(m_obj_rotz)), glm::vec3(0.0, 0.0, 1.0));
		//center object
		drw_mat = glm::translate(drw_mat, glm::vec3(
			-m_obj_ctrx,
			-m_obj_ctry,
			-m_obj_ctrz));
	}

	return drw_mat;
}

glm::mat4 RenderView::GetInvtMat()
{
	glm::mat4 inv_mat = m_mv_mat;
	//translate object
	inv_mat = glm::translate(inv_mat, glm::vec3(
		m_obj_transx,
		m_obj_transy,
		m_obj_transz));

	if (m_offset)
	{
		inv_mat = glm::translate(inv_mat, glm::vec3(
			m_obj_rot_ctr_offx - m_obj_ctrx,
			m_obj_ctry - m_obj_rot_ctr_offy,
			m_obj_rot_ctr_offz - m_obj_ctrz));
		//rotate object
		inv_mat = glm::rotate(inv_mat, float(glm::radians(m_obj_rotz + m_obj_rot_offz)), glm::vec3(0.0, 0.0, 1.0));
		inv_mat = glm::rotate(inv_mat, float(glm::radians(m_obj_roty + m_obj_rot_offy)), glm::vec3(0.0, 1.0, 0.0));
		inv_mat = glm::rotate(inv_mat, float(glm::radians(m_obj_rotx + m_obj_rot_offx)), glm::vec3(1.0, 0.0, 0.0));
		//center object
		inv_mat = glm::translate(inv_mat, glm::vec3(
			-m_obj_rot_ctr_offx - m_obj_ctr_offx,
			-m_obj_rot_ctr_offy - m_obj_ctr_offy,
			-m_obj_rot_ctr_offz - m_obj_ctr_offz));
	}
	else
	{
		//rotate object
		inv_mat = glm::rotate(inv_mat, float(glm::radians(m_obj_rotz)), glm::vec3(0.0, 0.0, 1.0));
		inv_mat = glm::rotate(inv_mat, float(glm::radians(m_obj_roty)), glm::vec3(0.0, 1.0, 0.0));
		inv_mat = glm::rotate(inv_mat, float(glm::radians(m_obj_rotx)), glm::vec3(1.0, 0.0, 0.0));
		//center object
		inv_mat = glm::translate(inv_mat, glm::vec3(
			-m_obj_ctrx,
			-m_obj_ctry,
			-m_obj_ctrz));
	}

	return inv_mat;
}

glm::mat4 RenderView::GetDrawWorldMat()
{
	glm::mat4 drw_mat = m_mv_mat;
	//translate object
	drw_mat = glm::translate(drw_mat, glm::vec3(
		m_obj_transx,
		m_obj_transy,
		m_obj_transz));

	//rotate object
	drw_mat = glm::rotate(drw_mat, float(glm::radians(m_obj_rotx)), glm::vec3(1.0, 0.0, 0.0));
	drw_mat = glm::rotate(drw_mat, float(glm::radians(m_obj_roty)), glm::vec3(0.0, 1.0, 0.0));
	drw_mat = glm::rotate(drw_mat, float(glm::radians(m_obj_rotz)), glm::vec3(0.0, 0.0, 1.0));
	//center object
	drw_mat = glm::translate(drw_mat, glm::vec3(
		-m_obj_ctrx,
		-m_obj_ctry,
		-m_obj_ctrz));

	return drw_mat;
}

fluo::Transform RenderView::GetInvOffsetMat()
{
	return m_offset_tf;
}

fluo::Vector RenderView::GetSide()
{
	m_head = fluo::Vector(-m_transx, -m_transy, -m_transz);
	m_head.normalize();
	fluo::Vector side = Cross(m_up, m_head);
	return side;
}

void RenderView::UpdateClips()
{
	if (m_clip_mode == 1)
		m_q_cl.FromEuler(m_rotx, -m_roty, -m_rotz);

	std::vector<fluo::Plane*> *planes = 0;
	for (int i = 0; i < (int)m_vd_pop_list.size(); i++)
	{
		if (!m_vd_pop_list[i])
			continue;

		double spcx, spcy, spcz;
		int resx, resy, resz;
		m_vd_pop_list[i]->GetSpacings(spcx, spcy, spcz);
		m_vd_pop_list[i]->GetResolution(resx, resy, resz);
		fluo::Vector scale;
		if (spcx > 0.0 && spcy > 0.0 && spcz > 0.0)
		{
			scale = fluo::Vector(1.0 / resx / spcx, 1.0 / resy / spcy, 1.0 / resz / spcz);
			scale.safe_normalize();
		}
		else
			scale = fluo::Vector(1.0, 1.0, 1.0);

		if (m_vd_pop_list[i]->GetVR())
			planes = m_vd_pop_list[i]->GetVR()->get_planes();
		if (planes && planes->size() == 6)
		{
			double x1, x2, y1, y2, z1, z2;
			double abcd[4];

			(*planes)[0]->get_copy(abcd);
			x1 = fabs(abcd[3]);
			(*planes)[1]->get_copy(abcd);
			x2 = fabs(abcd[3]);
			(*planes)[2]->get_copy(abcd);
			y1 = fabs(abcd[3]);
			(*planes)[3]->get_copy(abcd);
			y2 = fabs(abcd[3]);
			(*planes)[4]->get_copy(abcd);
			z1 = fabs(abcd[3]);
			(*planes)[5]->get_copy(abcd);
			z2 = fabs(abcd[3]);

			fluo::Vector trans1(-0.5, -0.5, -0.5);
			fluo::Vector trans2(0.5, 0.5, 0.5);

			(*planes)[0]->Restore();
			(*planes)[0]->Translate(trans1);
			(*planes)[0]->Rotate(m_q_cl);
			(*planes)[0]->Scale(scale);
			(*planes)[0]->Translate(trans2);

			(*planes)[1]->Restore();
			(*planes)[1]->Translate(trans1);
			(*planes)[1]->Rotate(m_q_cl);
			(*planes)[1]->Scale(scale);
			(*planes)[1]->Translate(trans2);

			(*planes)[2]->Restore();
			(*planes)[2]->Translate(trans1);
			(*planes)[2]->Rotate(m_q_cl);
			(*planes)[2]->Scale(scale);
			(*planes)[2]->Translate(trans2);

			(*planes)[3]->Restore();
			(*planes)[3]->Translate(trans1);
			(*planes)[3]->Rotate(m_q_cl);
			(*planes)[3]->Scale(scale);
			(*planes)[3]->Translate(trans2);

			(*planes)[4]->Restore();
			(*planes)[4]->Translate(trans1);
			(*planes)[4]->Rotate(m_q_cl);
			(*planes)[4]->Scale(scale);
			(*planes)[4]->Translate(trans2);

			(*planes)[5]->Restore();
			(*planes)[5]->Translate(trans1);
			(*planes)[5]->Rotate(m_q_cl);
			(*planes)[5]->Scale(scale);
			(*planes)[5]->Translate(trans2);
		}
	}
}

//vr buffers
void RenderView::GetRenderSize(int &nx, int &ny)
{
	if (m_use_openxr)
	{
		nx = glbin_xr_renderer->GetSize(0);
		ny = glbin_xr_renderer->GetSize(1);
	}
	else
	{
		nx = m_gl_size.w();
		ny = m_gl_size.h();
		if (glbin_settings.m_hologram_mode == 1 &&
			!glbin_settings.m_sbs)
			nx /= 2;
	}
}

void RenderView::DrawBounds()
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	flvr::ShaderProgram* shader =
		glbin_img_shader_factory.shader(IMG_SHDR_DRAW_GEOMETRY);
	if (shader)
	{
		if (!shader->valid())
			shader->create();
		shader->bind();
	}
	shader->setLocalParam(0, 1.0, 1.0, 1.0, 1.0);
	glm::mat4 matrix = m_proj_mat * m_mv_mat;
	shader->setLocalParamMatrix(0, glm::value_ptr(matrix));

	flvr::VertexArray* va_cube =
		glbin_vertex_array_manager.vertex_array(flvr::VA_Bound_Cube);
	if (va_cube)
	{
		va_cube->set_param(m_bounds);
		va_cube->draw();
	}

	if (shader && shader->valid())
		shader->release();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
}

void RenderView::DrawGrid()
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	flvr::ShaderProgram* shader =
		glbin_img_shader_factory.shader(IMG_SHDR_DRAW_GEOMETRY);
	if (shader)
	{
		if (!shader->valid())
			shader->create();
		shader->bind();
	}
	fluo::Color text_color = GetTextColor();
	shader->setLocalParam(0, text_color.r(), text_color.g(), text_color.b(), 1.0);
	glm::mat4 matrix = m_proj_mat * m_mv_mat;
	shader->setLocalParamMatrix(0, glm::value_ptr(matrix));

	flvr::VertexArray* va_grid =
		glbin_vertex_array_manager.vertex_array(flvr::VA_Grid);
	if (va_grid)
	{
		//set parameters
		std::vector<std::pair<unsigned int, double>> params;
		params.push_back(std::pair<unsigned int, double>(0, 5.0));
		params.push_back(std::pair<unsigned int, double>(1, m_distance));
		va_grid->set_param(params);
		va_grid->draw();
	}

	if (shader && shader->valid())
		shader->release();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
}

void RenderView::DrawCamCtr()
{
	flvr::VertexArray* va_jack = 0;
	if (m_pin_rot_ctr)
		va_jack = glbin_vertex_array_manager.vertex_array(flvr::VA_Cam_Center);
	else
		va_jack = glbin_vertex_array_manager.vertex_array(flvr::VA_Cam_Jack);

	if (!va_jack)
		return;
	float len;
	if (m_pin_rot_ctr)
	{
		len = m_scale_factor * 5;
	}
	else
	{
		if (m_camctr_size > 0.0)
			len = m_distance * tan(d2r(m_aov / 2.0)) * m_camctr_size / 10.0;
		else
			len = fabs(m_camctr_size);
	}
	va_jack->set_param(0, len);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	flvr::ShaderProgram* shader =
		glbin_img_shader_factory.shader(IMG_SHDR_DRAW_GEOMETRY);
	if (shader)
	{
		if (!shader->valid())
			shader->create();
		shader->bind();
	}

	glm::mat4 matrix;
	va_jack->draw_begin();
	if (m_pin_rot_ctr)
	{
		int nx, ny;
		GetRenderSize(nx, ny);
		matrix = glm::ortho(-nx / 2.0f, nx / 2.0f, -ny / 2.0f, ny / 2.0f);
		shader->setLocalParamMatrix(0, glm::value_ptr(matrix));
		fluo::Color text_color = GetTextColor();
		shader->setLocalParam(0, text_color.r(), text_color.g(), text_color.b(), 1.0);
		va_jack->draw_cam_center();
	}
	else
	{
		matrix = m_proj_mat * m_mv_mat;
		shader->setLocalParamMatrix(0, glm::value_ptr(matrix));
		shader->setLocalParam(0, 1.0, 0.0, 0.0, 1.0);
		va_jack->draw_cam_jack(0);
		shader->setLocalParam(0, 0.0, 1.0, 0.0, 1.0);
		va_jack->draw_cam_jack(1);
		shader->setLocalParam(0, 0.0, 0.0, 1.0, 1.0);
		va_jack->draw_cam_jack(2);
	}
	va_jack->draw_end();

	if (shader && shader->valid())
		shader->release();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
}

void RenderView::DrawScaleBar()
{
	flvr::VertexArray* va_scale_bar =
		glbin_vertex_array_manager.vertex_array(flvr::VA_Scale_Bar);
	if (!va_scale_bar)
		return;

	int nx, ny;
	GetRenderSize(nx, ny);
	float sb_x, sb_y, sb_w, sb_h;
	float sx, sy;
	sx = 2.0 / nx;
	sy = 2.0 / ny;
	float px, py, ph;
	glm::mat4 proj_mat = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f);
	float len = m_sb_length / (m_ortho_right - m_ortho_left);
	sb_w = len * nx;
	sb_h = glbin_settings.m_line_width * 3;
	float textlen = m_text_renderer->RenderTextLen(m_sb_text);
	fluo::Color text_color = GetTextColor();
	float font_height = 0;
	if (m_disp_scale_bar_text)
		font_height = glbin_text_tex_manager.GetSize() + 3.0;

	std::vector<std::pair<unsigned int, double>> params;
	if (m_draw_frame)
	{
		float framex = glbin_moviemaker.GetCropX();
		float framey = glbin_moviemaker.GetCropY();
		float framew = glbin_moviemaker.GetCropW();
		float frameh = glbin_moviemaker.GetCropH();
		if (m_enlarge)
		{
			framew *= m_enlarge_scale;
			frameh *= m_enlarge_scale;
			framex *= m_enlarge_scale;
			framey *= m_enlarge_scale;
		}
		switch (glbin_moviemaker.GetScalebarPos())
		{
		case 0:
			sb_x = framex + glbin_moviemaker.GetScalebarX() + sb_w;
			sb_y = framey + frameh - glbin_moviemaker.GetScalebarY() - 1.1 * font_height;
			break;
		case 1:
			sb_x = framex + framew - glbin_moviemaker.GetScalebarX();
			sb_y = framey + frameh - glbin_moviemaker.GetScalebarY() - 1.1 * font_height;
			break;
		case 2:
			sb_x = framex + glbin_moviemaker.GetScalebarX() + sb_w;
			sb_y = framey + glbin_moviemaker.GetScalebarY() + sb_h;
			break;
		case 3:
		default:
			sb_x = framex + framew - glbin_moviemaker.GetScalebarX();
			sb_y = framey + glbin_moviemaker.GetScalebarY() + sb_h;
			break;
		}
	}
	else
	{
		sb_x = nx - 20;
		sb_y = 20;
	}
	px = sb_x / nx;
	py = sb_y / ny;
	ph = sb_h / ny;
	if (m_enlarge)
		ph *= m_enlarge_scale;
	params.push_back(std::pair<unsigned int, double>(0, px));
	params.push_back(std::pair<unsigned int, double>(1, py));
	params.push_back(std::pair<unsigned int, double>(2, len));
	params.push_back(std::pair<unsigned int, double>(3, ph));

	if (m_disp_scale_bar_text)
	{
		px = sb_x - 0.5 * (sb_w + textlen + nx);
		py = sb_y + 0.5 * font_height - ny / 2.0;
		m_text_renderer->RenderText(
			m_sb_text, text_color,
			px * sx, py * sy, sx, sy);
	}

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	flvr::ShaderProgram* shader =
		glbin_img_shader_factory.shader(IMG_SHDR_DRAW_GEOMETRY);
	if (shader)
	{
		if (!shader->valid())
			shader->create();
		shader->bind();
	}
	shader->setLocalParamMatrix(0, glm::value_ptr(proj_mat));
	shader->setLocalParam(0, text_color.r(), text_color.g(), text_color.b(), 1.0);
	va_scale_bar->set_param(params);
	va_scale_bar->draw();

	if (shader && shader->valid())
		shader->release();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
}

void RenderView::DrawLegend()
{
	float font_height =
		glbin_text_tex_manager.GetSize() + 3.0;

	int nx, ny;
	GetRenderSize(nx, ny);

	float xoffset = glbin_moviemaker.GetScalebarX();
	float yoffset = glbin_moviemaker.GetScalebarY();
	if (m_draw_frame)
	{
		xoffset += glbin_moviemaker.GetCropX();
		yoffset += glbin_moviemaker.GetCropY();
		if (m_disp_scale_bar)
		if (glbin_moviemaker.GetScalebarPos() == 2 ||
			glbin_moviemaker.GetScalebarPos() == 3)
			yoffset += font_height * 1.5;
	}

	float length = 0;
	float name_len = 0;
	float gap_width = font_height * 1.5;
	float w = glbin_moviemaker.GetCropW();
	int i;
	int lines = 0;
	//first pass
	for (i = 0; i<(int)m_vd_pop_list.size(); i++)
	{
		if (m_vd_pop_list[i] && m_vd_pop_list[i]->GetLegend())
		{
			std::wstring vd_name = m_vd_pop_list[i]->GetName();
			name_len = m_text_renderer->RenderTextLen(vd_name) + font_height;
			length += name_len;
			if (length < float(m_draw_frame ? w : nx) - gap_width)
			{
				length += gap_width;
			}
			else
			{
				length = name_len + gap_width;
				lines++;
			}
		}
	}
	for (i = 0; i<(int)m_md_pop_list.size(); i++)
	{
		if (m_md_pop_list[i] && m_md_pop_list[i]->GetLegend())
		{
			std::wstring md_name = m_md_pop_list[i]->GetName();
			name_len = m_text_renderer->RenderTextLen(md_name) + font_height;
			length += name_len;
			if (length < float(m_draw_frame ? w : nx) - gap_width)
			{
				length += gap_width;
			}
			else
			{
				length = name_len + gap_width;
				lines++;
			}
		}
	}

	//second pass
	int cur_line = 0;
	float xpos, ypos;
	length = 0.0;
	for (i = 0; i < (int)m_vd_pop_list.size(); i++)
	{
		if (m_vd_pop_list[i] && m_vd_pop_list[i]->GetLegend())
		{
			std::wstring vd_name = m_vd_pop_list[i]->GetName();
			xpos = length;
			name_len = m_text_renderer->RenderTextLen(vd_name) + font_height;
			length += name_len;
			if (length < double(m_draw_frame ? w : nx) - gap_width)
			{
				length += gap_width;
			}
			else
			{
				length = name_len + gap_width;
				xpos = 0.0;
				cur_line++;
			}
			bool highlighted = false;
			if (glbin_current.GetType() == 2 &&
				glbin_current.vol_data &&
				glbin_current.vol_data->GetName() == vd_name)
				highlighted = true;
			xpos += xoffset;
			ypos = ny - (lines - cur_line + 0.1) * font_height - yoffset;
			DrawName(xpos, ypos, nx, ny, vd_name,
				m_vd_pop_list[i]->GetColor(),
				font_height, highlighted);
		}
	}
	for (i = 0; i<(int)m_md_pop_list.size(); i++)
	{
		if (m_md_pop_list[i] && m_md_pop_list[i]->GetLegend())
		{
			std::wstring md_name = m_md_pop_list[i]->GetName();
			xpos = length;
			name_len = m_text_renderer->RenderTextLen(md_name) + font_height;
			length += name_len;
			if (length < double(m_draw_frame ? w : nx) - gap_width)
			{
				length += gap_width;
			}
			else
			{
				length = name_len + gap_width;
				xpos = 0.0;
				cur_line++;
			}
			fluo::Color amb, diff, spec;
			double shine, alpha;
			m_md_pop_list[i]->GetMaterial(amb, diff, spec, shine, alpha);
			fluo::Color c(diff.r(), diff.g(), diff.b());
			bool highlighted = false;
			if (glbin_current.GetType() == 3 &&
				glbin_current.mesh_data &&
				glbin_current.mesh_data->GetName() == md_name)
				highlighted = true;
			xpos += xoffset;
			ypos = ny - (lines - cur_line + 0.1) * font_height - yoffset;
			DrawName(xpos, ypos, nx, ny, md_name,
				m_vd_pop_list[i]->GetColor(),
				font_height, highlighted);
		}
	}

	m_sb_height = (lines + 1)*font_height;
}

void RenderView::DrawName(
	double x, double y, int nx, int ny,
	const std::wstring& name, fluo::Color color,
	double font_height,
	bool highlighted)
{
	flvr::VertexArray* va_legend_squares =
		glbin_vertex_array_manager.vertex_array(flvr::VA_Legend_Squares);
	if (!va_legend_squares)
		return;

	float sx, sy;
	sx = 2.0 / nx;
	sy = 2.0 / ny;
	glm::mat4 proj_mat = glm::ortho(0.0f, float(nx), 0.0f, float(ny));

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	flvr::ShaderProgram* shader =
		glbin_img_shader_factory.shader(IMG_SHDR_DRAW_GEOMETRY);
	if (shader)
	{
		if (!shader->valid())
			shader->create();
		shader->bind();
	}
	shader->setLocalParamMatrix(0, glm::value_ptr(proj_mat));

	std::vector<std::pair<unsigned int, double>> params;
	params.push_back(std::pair<unsigned int, double>(0, x + 0.2*font_height));
	params.push_back(std::pair<unsigned int, double>(1, ny - y + 0.2*font_height));
	params.push_back(std::pair<unsigned int, double>(2, x + 0.8*font_height));
	params.push_back(std::pair<unsigned int, double>(3, ny - y + 0.8*font_height));
	va_legend_squares->set_param(params);
	va_legend_squares->draw_begin();
	fluo::Color text_color = GetTextColor();
	shader->setLocalParam(0, text_color.r(), text_color.g(), text_color.b(), 1.0);
	va_legend_squares->draw_legend_square(0);
	shader->setLocalParam(0, color.r(), color.g(), color.b(), 1.0);
	va_legend_squares->draw_legend_square(1);
	va_legend_squares->draw_end();

	if (shader && shader->valid())
		shader->release();

	float px1 = x + font_height - nx / 2.0;
	float py1 = ny / 2.0 - y + 0.25 * font_height;
	m_text_renderer->RenderText(
		name, text_color,
		px1*sx, py1*sy, sx, sy);
	if (highlighted)
	{
		px1 -= 0.5;
		py1 += 0.5;
		m_text_renderer->RenderText(
			name, color,
			px1*sx, py1*sy, sx, sy);
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
}

void RenderView::DrawFrame()
{
	int nx, ny;
	GetRenderSize(nx, ny);
	glm::mat4 proj_mat = glm::ortho(float(0), float(nx), float(0), float(ny));

	flvr::VertexArray* va_frame =
		glbin_vertex_array_manager.vertex_array(flvr::VA_Crop_Frame);
	if (!va_frame)
		return;

	glDisable(GL_DEPTH_TEST);
	flvr::ShaderProgram* shader =
		glbin_img_shader_factory.shader(IMG_SHDR_DRAW_GEOMETRY);
	if (shader)
	{
		if (!shader->valid())
			shader->create();
		shader->bind();
	}
	shader->setLocalParam(0, 1.0, 1.0, 0.0, 1.0);
	shader->setLocalParamMatrix(0, glm::value_ptr(proj_mat));

	//draw frame
	std::vector<std::pair<unsigned int, double>> params;
	params.push_back(std::pair<unsigned int, double>(0, glbin_moviemaker.GetCropX()));
	params.push_back(std::pair<unsigned int, double>(1, glbin_moviemaker.GetCropY()));
	params.push_back(std::pair<unsigned int, double>(2, glbin_moviemaker.GetCropW()));
	params.push_back(std::pair<unsigned int, double>(3, glbin_moviemaker.GetCropH()));
	va_frame->set_param(params);
	va_frame->draw();

	if (shader && shader->valid())
		shader->release();
	glEnable(GL_DEPTH_TEST);
}

void RenderView::DrawClippingPlanes(int face_winding)
{
	int i;
	bool link = glbin_settings.m_clip_link;
	int plane_mode = glbin_settings.m_clip_mode;
	double width = glbin_settings.m_line_width;

	if (plane_mode == cm_None)
		return;

	bool draw_plane = plane_mode != cm_Frame6 && plane_mode != cm_Frame3;
	bool border = plane_mode == cm_Frame6 ||
		(m_clip_mask == -1 && face_winding == FRONT_FACE) ||
		m_clip_mask != -1;
	if (!border && plane_mode == cm_Frame3)
		return;

	if ((plane_mode == cm_LowTransBack ||
		plane_mode == cm_NormalBack) &&
		m_clip_mask == -1)
	{
		glCullFace(GL_FRONT);
		if (face_winding == BACK_FACE)
			face_winding = FRONT_FACE;
		else
			draw_plane = false;
	}
	else
		glCullFace(GL_BACK);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (face_winding == FRONT_FACE)
	{
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);
	}
	else if (face_winding == BACK_FACE)
	{
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CW);
	}
	else if (face_winding == CULL_OFF)
		glDisable(GL_CULL_FACE);

	flvr::ShaderProgram* shader1 =
		glbin_img_shader_factory.shader(IMG_SHDR_DRAW_GEOMETRY);
	if (shader1)
	{
		if (!shader1->valid())
			shader1->create();
		shader1->bind();
	}
	flvr::ShaderProgram* shader2 =
		glbin_img_shader_factory.shader(IMG_SHDR_DRAW_THICK_LINES_COLOR);
	if (shader2)
	{
		if (!shader2->valid())
			shader2->create();
	}

	for (i = 0; i<GetDispVolumeNum(); i++)
	{
		VolumeData* vd = GetDispVolumeData(i);
		if (!vd)
			continue;

		if (vd != m_cur_vol)
			continue;

		flvr::VolumeRenderer *vr = vd->GetVR();
		if (!vr)
			continue;

		std::vector<fluo::Plane*> *planes = vr->get_planes();
		if (planes->size() != 6)
			continue;

		//calculating planes
		//get six planes
		fluo::Plane* px1 = (*planes)[0];
		fluo::Plane* px2 = (*planes)[1];
		fluo::Plane* py1 = (*planes)[2];
		fluo::Plane* py2 = (*planes)[3];
		fluo::Plane* pz1 = (*planes)[4];
		fluo::Plane* pz2 = (*planes)[5];

		//calculate 4 lines
		fluo::Vector lv_x1z1, lv_x1z2, lv_x2z1, lv_x2z2;
		fluo::Point lp_x1z1, lp_x1z2, lp_x2z1, lp_x2z2;
		//x1z1
		if (!px1->Intersect(*pz1, lp_x1z1, lv_x1z1))
			continue;
		//x1z2
		if (!px1->Intersect(*pz2, lp_x1z2, lv_x1z2))
			continue;
		//x2z1
		if (!px2->Intersect(*pz1, lp_x2z1, lv_x2z1))
			continue;
		//x2z2
		if (!px2->Intersect(*pz2, lp_x2z2, lv_x2z2))
			continue;

		//calculate 8 points
		fluo::Point pp[8];
		//p0 = l_x1z1 * py1
		if (!py1->Intersect(lp_x1z1, lv_x1z1, pp[0]))
			continue;
		//p1 = l_x1z2 * py1
		if (!py1->Intersect(lp_x1z2, lv_x1z2, pp[1]))
			continue;
		//p2 = l_x2z1 *py1
		if (!py1->Intersect(lp_x2z1, lv_x2z1, pp[2]))
			continue;
		//p3 = l_x2z2 * py1
		if (!py1->Intersect(lp_x2z2, lv_x2z2, pp[3]))
			continue;
		//p4 = l_x1z1 * py2
		if (!py2->Intersect(lp_x1z1, lv_x1z1, pp[4]))
			continue;
		//p5 = l_x1z2 * py2
		if (!py2->Intersect(lp_x1z2, lv_x1z2, pp[5]))
			continue;
		//p6 = l_x2z1 * py2
		if (!py2->Intersect(lp_x2z1, lv_x2z1, pp[6]))
			continue;
		//p7 = l_x2z2 * py2
		if (!py2->Intersect(lp_x2z2, lv_x2z2, pp[7]))
			continue;
		fluo::Vector plane_centers[6];
		if (m_persp)
		{
			//compute plane centers
			plane_centers[0] = (pp[0] + pp[1] + pp[4] + pp[5]) / 4;
			plane_centers[1] = (pp[2] + pp[3] + pp[6] + pp[7]) / 4;
			plane_centers[2] = (pp[0] + pp[1] + pp[2] + pp[3]) / 4;
			plane_centers[3] = (pp[4] + pp[5] + pp[6] + pp[7]) / 4;
			plane_centers[4] = (pp[0] + pp[2] + pp[4] + pp[6]) / 4;
			plane_centers[5] = (pp[1] + pp[3] + pp[5] + pp[7]) / 4;
		}

		//draw the six planes out of the eight points
		//get color
		fluo::Color color(1.0, 1.0, 1.0);
		double plane_trans = 0.0;
		if (face_winding == BACK_FACE &&
			(m_clip_mask == 3 ||
				m_clip_mask == 12 ||
				m_clip_mask == 48 ||
				m_clip_mask == 1 ||
				m_clip_mask == 2 ||
				m_clip_mask == 4 ||
				m_clip_mask == 8 ||
				m_clip_mask == 16 ||
				m_clip_mask == 32 ||
				m_clip_mask == 64)
			)
			plane_trans = plane_mode == cm_LowTrans ||
			plane_mode == cm_LowTransBack ? 0.1 : 0.3;

		if (face_winding == FRONT_FACE)
		{
			plane_trans = plane_mode == cm_LowTrans ||
				plane_mode == cm_LowTransBack ? 0.1 : 0.3;
		}

		if (plane_mode == cm_Normal ||
			plane_mode == cm_NormalBack)
		{
			if (!link)
				color = vd->GetColor();
		}
		else
			color = GetTextColor();

		//transform
		if (!vd->GetTexture())
			continue;
		fluo::Transform *tform = vd->GetTexture()->transform();
		if (!tform)
			continue;
		double mvmat[16];
		tform->get_trans(mvmat);
		double sclx, scly, sclz;
		vd->GetScalings(sclx, scly, sclz);
		glm::mat4 mv_mat = glm::scale(m_mv_mat,
			glm::vec3(float(sclx), float(scly), float(sclz)));
		glm::mat4 mv_mat2 = glm::mat4(
			mvmat[0], mvmat[4], mvmat[8], mvmat[12],
			mvmat[1], mvmat[5], mvmat[9], mvmat[13],
			mvmat[2], mvmat[6], mvmat[10], mvmat[14],
			mvmat[3], mvmat[7], mvmat[11], mvmat[15]);
		mv_mat = mv_mat * mv_mat2;
		glm::mat4 matrix = m_proj_mat * mv_mat;
		shader1->setLocalParamMatrix(0, glm::value_ptr(matrix));
		if (border)
		{
			shader2->bind();
			shader2->setLocalParamMatrix(0, glm::value_ptr(matrix));
			shader2->setLocalParam(0, m_size.w(), m_size.h(), width, 0.0);
			shader1->bind();
		}

		bool draw_plane_border[6] = {true, true, true, true, true, true};
		fluo::Vector view(0, 0, 1);
		fluo::Vector normal;
		fluo::Transform mv;
		double dotp;
		mv.set(glm::value_ptr(mv_mat));
		for (int pi = 0; pi < 6; ++pi)
		{
			normal = (*planes)[pi]->normal();
			if (m_persp)
			{
				//look at plane center from origin
				view = plane_centers[pi];
				mv.transform_inplace(view);
				normal = -normal;
			}
			mv.unproject_inplace(normal);
			dotp = fluo::Dot(normal, view);
			if (face_winding == FRONT_FACE)
			{
				if (dotp >= 0)
					draw_plane_border[pi] = false;
			}
			else
			{
				if (dotp < 0)
					draw_plane_border[pi] = false;
			}
		}

		flvr::VertexArray* va_clipp =
			glbin_vertex_array_manager.vertex_array(flvr::VA_Clip_Planes);
		if (!va_clipp)
			return;
		std::vector<fluo::Point> clip_points(pp, pp+8);
		va_clipp->set_param(clip_points);
		va_clipp->draw_begin();
		//draw
		//x1 = (p4, p0, p1, p5)
		if (m_clip_mask & 1)
		{
			if (draw_plane)
			{
				if (plane_mode == cm_Normal ||
					plane_mode == cm_NormalBack)
					shader1->setLocalParam(0, 1.0, 0.5, 0.5, plane_trans);
				else
					shader1->setLocalParam(0, color.r(), color.g(), color.b(), plane_trans);
				va_clipp->draw_clip_plane(0, false);
			}
			if (border && draw_plane_border[0])
			{
				glDisable(GL_CULL_FACE);
				shader2->bind();
				shader2->setLocalParam(1, color.r(), color.g(), color.b(), 0.0);
				va_clipp->draw_clip_plane(16, true);
				shader1->bind();
				if (face_winding != CULL_OFF)
					glEnable(GL_CULL_FACE);
			}
		}
		//x2 = (p7, p3, p2, p6)
		if (m_clip_mask & 2)
		{
			if (draw_plane)
			{
				if (plane_mode == cm_Normal ||
					plane_mode == cm_NormalBack)
					shader1->setLocalParam(0, 1.0, 0.5, 1.0, plane_trans);
				else
					shader1->setLocalParam(0, color.r(), color.g(), color.b(), plane_trans);
				va_clipp->draw_clip_plane(32, false);
			}
			if (border && draw_plane_border[1])
			{
				glDisable(GL_CULL_FACE);
				shader2->bind();
				shader2->setLocalParam(1, color.r(), color.g(), color.b(), 0.0);
				va_clipp->draw_clip_plane(48, true);
				shader1->bind();
				if (face_winding != CULL_OFF)
					glEnable(GL_CULL_FACE);
			}
		}
		//y1 = (p1, p0, p2, p3)
		if (m_clip_mask & 4)
		{
			if (draw_plane)
			{
				if (plane_mode == cm_Normal ||
					plane_mode == cm_NormalBack)
					shader1->setLocalParam(0, 0.5, 1.0, 0.5, plane_trans);
				else
					shader1->setLocalParam(0, color.r(), color.g(), color.b(), plane_trans);
				va_clipp->draw_clip_plane(64, false);
			}
			if (border && draw_plane_border[2])
			{
				glDisable(GL_CULL_FACE);
				shader2->bind();
				shader2->setLocalParam(1, color.r(), color.g(), color.b(), 0.0);
				va_clipp->draw_clip_plane(80, true);
				shader1->bind();
				if (face_winding != CULL_OFF)
					glEnable(GL_CULL_FACE);
			}
		}
		//y2 = (p4, p5, p7, p6)
		if (m_clip_mask & 8)
		{
			if (draw_plane)
			{
				if (plane_mode == cm_Normal ||
					plane_mode == cm_NormalBack)
					shader1->setLocalParam(0, 1.0, 1.0, 0.5, plane_trans);
				else
					shader1->setLocalParam(0, color.r(), color.g(), color.b(), plane_trans);
				va_clipp->draw_clip_plane(96, false);
			}
			if (border && draw_plane_border[3])
			{
				glDisable(GL_CULL_FACE);
				shader2->bind();
				shader2->setLocalParam(1, color.r(), color.g(), color.b(), 0.0);
				va_clipp->draw_clip_plane(112, true);
				shader1->bind();
				if (face_winding != CULL_OFF)
					glEnable(GL_CULL_FACE);
			}
		}
		//z1 = (p0, p4, p6, p2)
		if (m_clip_mask & 16)
		{
			if (draw_plane)
			{
				if (plane_mode == cm_Normal ||
					plane_mode == cm_NormalBack)
					shader1->setLocalParam(0, 0.5, 0.5, 1.0, plane_trans);
				else
					shader1->setLocalParam(0, color.r(), color.g(), color.b(), plane_trans);
				va_clipp->draw_clip_plane(128, false);
			}
			if (border && draw_plane_border[4])
			{
				glDisable(GL_CULL_FACE);
				shader2->bind();
				shader2->setLocalParam(1, color.r(), color.g(), color.b(), 0.0);
				va_clipp->draw_clip_plane(144, true);
				shader1->bind();
				if (face_winding != CULL_OFF)
					glEnable(GL_CULL_FACE);
			}
		}
		//z2 = (p5, p1, p3, p7)
		if (m_clip_mask & 32)
		{
			if (draw_plane)
			{
				if (plane_mode == cm_Normal ||
					plane_mode == cm_NormalBack)
					shader1->setLocalParam(0, 0.5, 1.0, 1.0, plane_trans);
				else
					shader1->setLocalParam(0, color.r(), color.g(), color.b(), plane_trans);
				va_clipp->draw_clip_plane(160, false);
			}
			if (border && draw_plane_border[5])
			{
				glDisable(GL_CULL_FACE);
				shader2->bind();
				shader2->setLocalParam(1, color.r(), color.g(), color.b(), 0.0);
				va_clipp->draw_clip_plane(176, true);
				shader1->bind();
				if (face_winding != CULL_OFF)
					glEnable(GL_CULL_FACE);
			}
		}
		va_clipp->draw_end();
	}

	if (shader1 && shader1->valid())
		shader1->release();

	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
}

void RenderView::SetColormapColors(int colormap, fluo::Color &c, double inv)
{
	switch (colormap)
	{
	case 0://rainbow
		if (inv > 0.0)
		{
			m_color_1 = fluo::Color(0.0, 0.0, 1.0);
			m_color_2 = fluo::Color(0.0, 0.0, 1.0);
			m_color_3 = fluo::Color(0.0, 1.0, 1.0);
			m_color_4 = fluo::Color(0.0, 1.0, 0.0);
			m_color_5 = fluo::Color(1.0, 1.0, 0.0);
			m_color_6 = fluo::Color(1.0, 0.0, 0.0);
			m_color_7 = fluo::Color(1.0, 0.0, 0.0);
		}
		else
		{
			m_color_1 = fluo::Color(1.0, 0.0, 0.0);
			m_color_2 = fluo::Color(1.0, 0.0, 0.0);
			m_color_3 = fluo::Color(1.0, 1.0, 0.0);
			m_color_4 = fluo::Color(0.0, 1.0, 0.0);
			m_color_5 = fluo::Color(0.0, 1.0, 1.0);
			m_color_6 = fluo::Color(0.0, 0.0, 1.0);
			m_color_7 = fluo::Color(0.0, 0.0, 1.0);
		}
		break;
	case 1://hot
		if (inv > 0.0)
		{
			m_color_1 = fluo::Color(0.0, 0.0, 0.0);
			m_color_2 = fluo::Color(0.0, 0.0, 0.0);
			m_color_3 = fluo::Color(0.5, 0.0, 0.0);
			m_color_4 = fluo::Color(1.0, 0.0, 0.0);
			m_color_5 = fluo::Color(1.0, 1.0, 0.0);
			m_color_6 = fluo::Color(1.0, 1.0, 1.0);
			m_color_7 = fluo::Color(1.0, 1.0, 1.0);
		}
		else
		{
			m_color_1 = fluo::Color(1.0, 1.0, 1.0);
			m_color_2 = fluo::Color(1.0, 1.0, 1.0);
			m_color_3 = fluo::Color(1.0, 1.0, 0.0);
			m_color_4 = fluo::Color(1.0, 0.0, 0.0);
			m_color_5 = fluo::Color(0.5, 0.0, 0.0);
			m_color_6 = fluo::Color(0.0, 0.0, 0.0);
			m_color_7 = fluo::Color(0.0, 0.0, 0.0);
		}
		break;
	case 2://cool
		if (inv > 0.0)
		{
			m_color_1 = fluo::Color(0.0, 1.0, 1.0);
			m_color_2 = fluo::Color(0.0, 1.0, 1.0);
			m_color_3 = fluo::Color(0.25, 0.75, 1.0);
			m_color_4 = fluo::Color(0.5, 0.5, 1.0);
			m_color_5 = fluo::Color(0.75, 0.25, 1.0);
			m_color_6 = fluo::Color(1.0, 0.0, 1.0);
			m_color_7 = fluo::Color(1.0, 0.0, 1.0);
		}
		else
		{
			m_color_1 = fluo::Color(1.0, 0.0, 1.0);
			m_color_2 = fluo::Color(1.0, 0.0, 1.0);
			m_color_3 = fluo::Color(0.75, 0.25, 1.0);
			m_color_4 = fluo::Color(0.5, 0.5, 1.0);
			m_color_5 = fluo::Color(0.25, 0.75, 1.0);
			m_color_6 = fluo::Color(0.0, 1.0, 1.0);
			m_color_7 = fluo::Color(0.0, 1.0, 1.0);
		}
		break;
	case 3://diverging
		if (inv > 0.0)
		{
			m_color_1 = fluo::Color(0.25, 0.3, 0.75);
			m_color_2 = fluo::Color(0.25, 0.3, 0.75);
			m_color_3 = fluo::Color(0.475, 0.5, 0.725);
			m_color_4 = fluo::Color(0.7, 0.7, 0.7);
			m_color_5 = fluo::Color(0.7, 0.35, 0.425);
			m_color_6 = fluo::Color(0.7, 0.0, 0.15);
			m_color_7 = fluo::Color(0.7, 0.0, 0.15);
		}
		else
		{
			m_color_1 = fluo::Color(0.7, 0.0, 0.15);
			m_color_2 = fluo::Color(0.7, 0.0, 0.15);
			m_color_3 = fluo::Color(0.7, 0.35, 0.425);
			m_color_4 = fluo::Color(0.7, 0.7, 0.7);
			m_color_5 = fluo::Color(0.475, 0.5, 0.725);
			m_color_6 = fluo::Color(0.25, 0.3, 0.75);
			m_color_7 = fluo::Color(0.25, 0.3, 0.75);
		}
		break;
	case 4://monochrome
		if (inv > 0.0)
		{
			m_color_1 = fluo::Color(0.0, 0.0, 0.0);
			m_color_2 = fluo::Color(0.0, 0.0, 0.0);
			m_color_3 = fluo::Color(0.25, 0.25, 0.25);
			m_color_4 = fluo::Color(0.5, 0.5, 0.5);
			m_color_5 = fluo::Color(0.75, 0.75, 0.75);
			m_color_6 = fluo::Color(1.0, 1.0, 1.0);
			m_color_7 = fluo::Color(1.0, 1.0, 1.0);
		}
		else
		{
			m_color_1 = fluo::Color(1.0, 1.0, 1.0);
			m_color_2 = fluo::Color(1.0, 1.0, 1.0);
			m_color_3 = fluo::Color(0.75, 0.75, 0.75);
			m_color_4 = fluo::Color(0.5, 0.5, 0.5);
			m_color_5 = fluo::Color(0.25, 0.25, 0.25);
			m_color_6 = fluo::Color(0.0, 0.0, 0.0);
			m_color_7 = fluo::Color(0.0, 0.0, 0.0);
		}
		break;
	case 5://high-key
		if (inv > 0.0)
		{
			m_color_1 = fluo::Color(1.0, 1.0, 1.0);
			m_color_2 = fluo::Color(1.0, 1.0, 1.0);
			m_color_3 = c * 0.25 + fluo::Color(1.0, 1.0, 1.0)*0.75;
			m_color_4 = (c + fluo::Color(1.0, 1.0, 1.0))*0.5;
			m_color_5 = c * 0.75 + fluo::Color(1.0, 1.0, 1.0)*0.25;
			m_color_6 = c;
			m_color_7 = c;
		}
		else
		{
			m_color_1 = c;
			m_color_2 = c;
			m_color_3 = c * 0.75 + fluo::Color(1.0, 1.0, 1.0)*0.25;
			m_color_4 = (c + fluo::Color(1.0, 1.0, 1.0))*0.5;
			m_color_5 = c * 0.25 + fluo::Color(1.0, 1.0, 1.0)*0.75;
			m_color_6 = fluo::Color(1.0, 1.0, 1.0);
			m_color_7 = fluo::Color(1.0, 1.0, 1.0);
		}
		break;
	case 6://low-key
		if (inv > 0.0)
		{
			m_color_1 = c;
			m_color_2 = c;
			m_color_3 = c * (0.025 + 0.75);
			m_color_4 = c * 0.55;
			m_color_5 = c * (0.075 + 0.25);
			m_color_6 = c * 0.1;
			m_color_7 = c * 0.1;
		}
		else
		{
			m_color_1 = c * 0.1;
			m_color_2 = c * 0.1;
			m_color_3 = c * (0.075 + 0.25);
			m_color_4 = c * 0.55;
			m_color_5 = c * (0.025 + 0.75);
			m_color_6 = c;
			m_color_7 = c;
		}
		break;
	case 7://high transparency
		if (inv > 0.0)
		{
			m_color_1 = fluo::Color(0.0, 0.0, 0.0);
			m_color_2 = fluo::Color(0.0, 0.0, 0.0);
			m_color_3 = c * 0.25 + fluo::Color(0.0, 0.0, 0.0) * 0.75;
			m_color_4 = c * 0.5 + fluo::Color(0.0, 0.0, 0.0) * 0.5;
			m_color_5 = c * 0.75 + fluo::Color(0.0, 0.0, 0.0) * 0.25;
			m_color_6 = c;
			m_color_7 = c;
		}
		else
		{
			m_color_1 = c;
			m_color_2 = c;
			m_color_3 = c * 0.75 + fluo::Color(0.0, 0.0, 0.0) * 0.25;
			m_color_4 = c * 0.5 + fluo::Color(0.0, 0.0, 0.0) * 0.5;
			m_color_5 = c * 0.25 + fluo::Color(0.0, 0.0, 0.0) * 0.75;
			m_color_6 = fluo::Color(0.0, 0.0, 0.0);
			m_color_7 = fluo::Color(0.0, 0.0, 0.0);
		}
		break;
	}
}

void RenderView::DrawColormap()
{
	double max_val = 255.0;
	bool enable_alpha = false;

	if (m_cur_vol &&
		m_cur_vol->GetColormapMode())
	{
		double low, high;
		m_cur_vol->GetColormapValues(low, high);
		m_value_2 = low;
		m_value_6 = high;
		m_value_4 = (low + high) / 2.0;
		m_value_3 = (low + m_value_4) / 2.0;
		m_value_5 = (m_value_4 + high) / 2.0;
		max_val = m_cur_vol->GetMaxValue();
		enable_alpha = m_cur_vol->GetAlphaEnable();
		fluo::Color vd_color = m_cur_vol->GetColor();
		SetColormapColors(m_cur_vol->GetColormap(),
			vd_color, m_cur_vol->GetColormapInv());
	}
	else return;

	float offset = 0;
	if (m_draw_legend)
		offset = glbin_text_tex_manager.GetSize() + 3.0;

	int nx, ny;
	GetRenderSize(nx, ny);
	float sx, sy;//normalized size
	sx = 2.0 / nx;
	sy = 2.0 / ny;
	float px, py, ph, pw;//normalized pos
	float cmx, cmy, cmw, cmh;//pixel pos and size
	cmw = 40;
	float txx, txy;//pixel pos of text

	std::wstring wstr;
	std::vector<float> vertex;
	vertex.reserve(98);
	//draw colormap
	if (m_draw_frame)
	{
		float framex = glbin_moviemaker.GetCropX();
		float framey = glbin_moviemaker.GetCropY();
		float framew = glbin_moviemaker.GetCropW();
		float frameh = glbin_moviemaker.GetCropH();
		if (m_enlarge)
		{
			framew *= m_enlarge_scale;
			frameh *= m_enlarge_scale;
			framex *= m_enlarge_scale;
			framey *= m_enlarge_scale;
		}
		wstr = std::to_wstring(88);
		float textlen =
			m_text_renderer->RenderTextLen(wstr);

		switch (glbin_moviemaker.GetScalebarPos())
		{
		case 0:
		case 2:
			cmx = framex + framew - glbin_moviemaker.GetScalebarX() - cmw;
			cmy = framey + 20 + glbin_moviemaker.GetScalebarY() + offset;
			txx = cmx - cmw;
			break;
		case 1:
		case 3:
		default:
			cmx = framex + glbin_moviemaker.GetScalebarX();
			cmy = framey + 20 + glbin_moviemaker.GetScalebarY() + offset;
			txx = cmx + cmw;
			break;
		}
		cmh = frameh > 300 ? 300 : frameh - 40 - glbin_moviemaker.GetScalebarY() - offset;
		cmh = std::max(cmh, 40.0f);
		txy = cmy;
	}
	else
	{
		cmx = 20;
		cmy = 20 + offset;
		cmh = std::min(ny, 300);
		txx = cmx + cmw;
		txy = cmy;
	}

	fluo::Color text_color = GetTextColor();

	px = txx - nx / 2.0;
	//value 1
	py = txy - ny / 2.0;
	wstr = L"0";
	m_text_renderer->RenderText(
		wstr, text_color,
		px * sx, py * sy, sx, sy);
	//value 2
	py = txy + cmh * m_value_2 - ny / 2.0;
	wstr = std::to_wstring(int(std::round(m_value_2 * max_val)));
	m_text_renderer->RenderText(
		wstr, text_color,
		px * sx, py * sy, sx, sy);
	//value 4
	py = txy + cmh * m_value_4 - ny / 2.0;
	wstr = std::to_wstring(int(std::round(m_value_4 * max_val)));
	m_text_renderer->RenderText(
		wstr, text_color,
		px * sx, py * sy, sx, sy);
	//value 6
	py = txy + cmh * m_value_6 - ny / 2.0;
	wstr = std::to_wstring(int(std::round(m_value_6 * max_val)));
	m_text_renderer->RenderText(
		wstr, text_color,
		px * sx, py * sy, sx, sy);
	//value 7
	py = txy + cmh - ny / 2.0;
	wstr = std::to_wstring(int(std::round(max_val)));
	m_text_renderer->RenderText(
		wstr, text_color,
		px * sx, py * sy, sx, sy);

	px = cmx / nx;
	py = cmy / ny;
	pw = cmw / nx;
	ph = cmh / ny;
	if (m_enlarge)
	{
		pw *= m_enlarge_scale;
		ph *= m_enlarge_scale;
	}
	vertex.push_back(px); vertex.push_back(py); vertex.push_back(0.0);
	vertex.push_back(m_color_1.r()); vertex.push_back(m_color_1.g()); vertex.push_back(m_color_1.b()); vertex.push_back(enable_alpha ? 0.0 : 1.0);
	vertex.push_back(px + pw); vertex.push_back(py); vertex.push_back(0.0);
	vertex.push_back(m_color_1.r()); vertex.push_back(m_color_1.g()); vertex.push_back(m_color_1.b()); vertex.push_back(enable_alpha ? 0.0 : 1.0);
	vertex.push_back(px); vertex.push_back(py + ph * m_value_2); vertex.push_back(0.0);
	vertex.push_back(m_color_2.r()); vertex.push_back(m_color_2.g()); vertex.push_back(m_color_2.b()); vertex.push_back(enable_alpha ? m_value_2 : 1.0);
	vertex.push_back(px + pw); vertex.push_back(py + ph * m_value_2); vertex.push_back(0.0);
	vertex.push_back(m_color_2.r()); vertex.push_back(m_color_2.g()); vertex.push_back(m_color_2.b()); vertex.push_back(enable_alpha ? m_value_2 : 1.0);
	vertex.push_back(px); vertex.push_back(py + ph * m_value_3); vertex.push_back(0.0);
	vertex.push_back(m_color_3.r()); vertex.push_back(m_color_3.g()); vertex.push_back(m_color_3.b()); vertex.push_back(enable_alpha ? m_value_3 : 1.0);
	vertex.push_back(px + pw); vertex.push_back(py + ph * m_value_3); vertex.push_back(0.0);
	vertex.push_back(m_color_3.r()); vertex.push_back(m_color_3.g()); vertex.push_back(m_color_3.b()); vertex.push_back(enable_alpha ? m_value_3 : 1.0);
	vertex.push_back(px); vertex.push_back(py + ph * m_value_4); vertex.push_back(0.0);
	vertex.push_back(m_color_4.r()); vertex.push_back(m_color_4.g()); vertex.push_back(m_color_4.b()); vertex.push_back(enable_alpha ? m_value_4 : 1.0);
	vertex.push_back(px + pw); vertex.push_back(py + ph * m_value_4); vertex.push_back(0.0);
	vertex.push_back(m_color_4.r()); vertex.push_back(m_color_4.g()); vertex.push_back(m_color_4.b()); vertex.push_back(enable_alpha ? m_value_4 : 1.0);
	vertex.push_back(px); vertex.push_back(py + ph * m_value_5); vertex.push_back(0.0);
	vertex.push_back(m_color_5.r()); vertex.push_back(m_color_5.g()); vertex.push_back(m_color_5.b()); vertex.push_back(enable_alpha ? m_value_5 : 1.0);
	vertex.push_back(px + pw); vertex.push_back(py + ph * m_value_5); vertex.push_back(0.0);
	vertex.push_back(m_color_5.r()); vertex.push_back(m_color_5.g()); vertex.push_back(m_color_5.b()); vertex.push_back(enable_alpha ? m_value_5 : 1.0);
	vertex.push_back(px); vertex.push_back(py + ph * m_value_6); vertex.push_back(0.0);
	vertex.push_back(m_color_6.r()); vertex.push_back(m_color_6.g()); vertex.push_back(m_color_6.b()); vertex.push_back(enable_alpha ? m_value_6 : 1.0);
	vertex.push_back(px + pw); vertex.push_back(py + ph * m_value_6); vertex.push_back(0.0);
	vertex.push_back(m_color_6.r()); vertex.push_back(m_color_6.g()); vertex.push_back(m_color_6.b()); vertex.push_back(enable_alpha ? m_value_6 : 1.0);
	vertex.push_back(px); vertex.push_back(py + ph); vertex.push_back(0.0);
	vertex.push_back(m_color_7.r()); vertex.push_back(m_color_7.g()); vertex.push_back(m_color_7.b()); vertex.push_back(1.0);
	vertex.push_back(px + pw); vertex.push_back(py + ph); vertex.push_back(0.0);
	vertex.push_back(m_color_7.r()); vertex.push_back(m_color_7.g()); vertex.push_back(m_color_7.b()); vertex.push_back(1.0);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	flvr::ShaderProgram* shader =
		glbin_img_shader_factory.shader(IMG_SHDR_DRAW_GEOMETRY_COLOR4);
	if (shader)
	{
		if (!shader->valid())
			shader->create();
		shader->bind();
	}
	glm::mat4 proj_mat = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f);
	shader->setLocalParamMatrix(0, glm::value_ptr(proj_mat));

	flvr::VertexArray* va_colormap =
		glbin_vertex_array_manager.vertex_array(flvr::VA_Color_Map);
	if (va_colormap)
	{
		va_colormap->set_param(vertex);
		va_colormap->draw();
	}

	if (shader && shader->valid())
		shader->release();

	glEnable(GL_DEPTH_TEST);
}

void RenderView::DrawGradBg()
{
	//define colors
	fluo::Color color1, color2;
	fluo::HSVColor hsv_color1(m_bg_color);
	if (hsv_color1.val() > 0.5)
	{
		if (hsv_color1.sat() > 0.3)
		{
			color1 = fluo::Color(fluo::HSVColor(hsv_color1.hue(),
				hsv_color1.sat() * 0.1,
				std::min(hsv_color1.val() + 0.3, 1.0)));
			color2 = fluo::Color(fluo::HSVColor(hsv_color1.hue(),
				hsv_color1.sat() * 0.3,
				std::min(hsv_color1.val() + 0.1, 1.0)));
		}
		else
		{
			color1 = fluo::Color(fluo::HSVColor(hsv_color1.hue(),
				hsv_color1.sat() * 0.1,
				std::max(hsv_color1.val() - 0.6, 0.0)));
			color2 = fluo::Color(fluo::HSVColor(hsv_color1.hue(),
				hsv_color1.sat() * 0.3,
				std::max(hsv_color1.val() - 0.5, 0.0)));
		}
	}
	else
	{
		color1 = fluo::Color(fluo::HSVColor(hsv_color1.hue(),
			hsv_color1.sat() * 0.1,
			std::min(hsv_color1.val() + 0.7, 1.0)));
		color2 = fluo::Color(fluo::HSVColor(hsv_color1.hue(),
			hsv_color1.sat() * 0.3,
			std::min(hsv_color1.val() + 0.2, 1.0)));
	}

	//compute horizon
	int nx, ny;
	GetRenderSize(nx, ny);
	double aspect = (double)nx / (double)ny;
	double oh = 1000;
	double ow = oh * aspect;
	glm::mat4 proj_mat = glm::ortho(-ow, ow, -oh, oh, 0.1, 1.0);
	glm::mat4 mv_mat = glm::inverse(m_mv_mat);
	mv_mat[3][0] = 0.0;
	mv_mat[3][1] = 0.0;
	mv_mat[3][2] = -4000.0;
	glm::mat4 trans_mat = proj_mat * mv_mat;
	trans_mat = glm::inverse(trans_mat);

	//ndc transformation
	auto compute_angle = [](const glm::vec2& tc, const glm::mat4& trans)
	{
		glm::vec2 ndc = 2.0f * tc - glm::vec2(1.0f);
		glm::vec4 p = glm::vec4(ndc, 0.0f, 1.0f);
		p = trans * p;
		p /= p.w;
		glm::vec3 dir = glm::vec3(p.x, p.y, p.z);
		dir = glm::normalize(dir);
		return glm::degrees(glm::asin(dir.y));
	};
	double v0, v1, v2, v3;//min angle, band 1, band 2, max value
	v0 = compute_angle(glm::vec2(0.0, 0.0), trans_mat);
	v1 = compute_angle(glm::vec2(1.0, 0.0), trans_mat);
	v2 = compute_angle(glm::vec2(1.0, 1.0), trans_mat);
	v3 = compute_angle(glm::vec2(0.0, 1.0), trans_mat);
	double mind = std::min({ v0, v1, v2, v3 });
	double maxd = std::max({ v0, v1, v2, v3 });
	v0 = mind;
	v1 = -8;
	v2 = -4;
	v3 = maxd;

	//set up shader
	flvr::ShaderProgram* shader =
		glbin_img_shader_factory.shader(IMG_SHDR_GRADIENT_BACKGROUND);
	if (shader)
	{
		if (!shader->valid())
			shader->create();
		shader->bind();
	}
	shader->setLocalParam(0, m_bg_color.r(), m_bg_color.g(), m_bg_color.b(), 1.0);
	shader->setLocalParam(1, color1.r(), color1.g(), color1.b(), 1.0);
	shader->setLocalParam(2, color2.r(), color2.g(), color2.b(), 1.0);
	shader->setLocalParam(3, v0, v1, v2, v3);
	shader->setLocalParamMatrix(0, glm::value_ptr(trans_mat));

	//draw
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	DrawViewQuad();

	//restore
	if (shader && shader->valid())
		shader->release();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
}

void RenderView::DrawInfo(int nx, int ny, bool intactive)
{
	float sx, sy;
	sx = 2.0 / nx;
	sy = 2.0 / ny;
	float px, py;
	float gapw = glbin_text_tex_manager.GetSize();
	float gaph = gapw * 2;

	double fps = 1.0 / glbin.getStopWatch(gstStopWatch)->average();
	fps = std::max(fps, 0.0);
	fluo::Color text_color = GetTextColor();
	std::wstring str;
	std::wostringstream tos;
	if (glbin_settings.m_mem_swap)
	{
		if (glbin_vol_selector.GetBrushUsePres())
			tos << L"FPS: " << std::fixed << std::setprecision(2) << fps
				<< L", Act: " << (intactive ? L"Yes" : L"No")
				<< L", Bricks: " << flvr::TextureRenderer::get_finished_bricks()
				<< L", Quota: " << flvr::TextureRenderer::get_quota_bricks()
				<< L", Time: " << flvr::TextureRenderer::get_cor_up_time()
				<< L", Pressure: " << std::fixed << std::setprecision(2) << glbin_vol_selector.GetPressure();
		else
			tos << L"FPS: " << std::fixed << std::setprecision(2) << fps
				<< L", Act: " << (intactive ? L"Yes" : L"No")
				<< L", Bricks: " << flvr::TextureRenderer::get_finished_bricks()
				<< L", Quota: " << flvr::TextureRenderer::get_quota_bricks()
				<< L", Time: " << flvr::TextureRenderer::get_cor_up_time();
		
		////budget_test
		//if (m_interactive)
		//  tos <<
		//  flvr::TextureRenderer::get_quota_bricks()
		//  << "\t" <<
		//  flvr::TextureRenderer::get_finished_bricks()
		//  << "\t" <<
		//  flvr::TextureRenderer::get_queue_last()
		//  << "\t" <<
		//  int(flvr::TextureRenderer::get_finished_bricks()*
		//    flvr::TextureRenderer::get_up_time()/
		//    flvr::TextureRenderer::get_consumed_time())
		//  << "\n";
	}
	else
	{
		if (glbin_vol_selector.GetBrushUsePres())
			tos << L"FPS: " << std::fixed << std::setprecision(2) << fps
				<< L", Pressure: " << std::fixed << std::setprecision(2) << glbin_vol_selector.GetPressure();
		else
			tos << L"FPS: " << std::fixed << std::setprecision(2) << fps;
	}
	str = tos.str();
	px = gapw - nx / 2.0;
	py = ny / 2.0 - gaph / 2.0;
	m_text_renderer->RenderText(
		str, text_color,
		px*sx, py*sy, sx, sy);

	tos.str(L"");
	tos.clear();

	if ((m_draw_info & INFO_T) &&
		(m_draw_info & INFO_X) &&
		(m_draw_info & INFO_Y) &&
		(m_draw_info & INFO_Z))
	{
		fluo::Point p;
		glbin_volume_point.SetVolumeData(m_cur_vol);
		if ((glbin_volume_point.GetPointVolumeBox(m_mouse_x, m_mouse_y, true, p )>0.0) ||
			glbin_volume_point.GetPointPlane(m_mouse_x, m_mouse_y, 0, true, p)>0.0)
		{
			tos << L"T: " << m_tseq_cur_num
				<< L", X: " << std::fixed << std::setprecision(2) << p.x()
				<< L", Y: " << std::fixed << std::setprecision(2) << p.y()
				<< L", Z: " << std::fixed << std::setprecision(2) << p.z();
			str = tos.str();
			px = gapw - nx / 2.0;
			py = ny / 2.0 - gaph;
			m_text_renderer->RenderText(
				str, text_color,
				px*sx, py*sy, sx, sy);
		}
	}
	else if (m_draw_info & INFO_Z)
	{
		if (m_cur_vol)
		{
			int resx, resy, resz;
			m_cur_vol->GetResolution(resx, resy, resz);
			double spcx, spcy, spcz;
			m_cur_vol->GetSpacings(spcx, spcy, spcz);
			std::vector<fluo::Plane*> *planes = m_cur_vol->GetVR()->get_planes();
			fluo::Plane* plane = (*planes)[4];
			double abcd[4];
			plane->get_copy(abcd);
			int val = fabs(abcd[3] * resz) + 0.499;

			tos << L"Z: " << std::fixed << std::setprecision(2) << val * spcz << L"\u03BCm";
			str = tos.str();
			if (m_draw_frame)
			{
				px = 0.01*glbin_moviemaker.GetCropW() + glbin_moviemaker.GetCropX() - nx / 2.0;
				py = 0.04*glbin_moviemaker.GetCropH() + glbin_moviemaker.GetCropY() - ny / 2.0;
			}
			else
			{
				px = 0.01*nx - nx / 2.0;
				py = 0.04*ny - ny / 2.0;
			}
			m_text_renderer->RenderText(
				str, text_color,
				px*sx, py*sy, sx, sy);
		}
	}

	if (glbin_settings.m_test_wiref)
	{
		if (m_vol_method == VOL_METHOD_MULTI && m_mvr)
		{
			str = L"SLICES: " + std::to_wstring(m_mvr->get_slice_num());
			px = gapw - nx / 2.0;
			py = ny / 2.0 - gaph*1.5;
			m_text_renderer->RenderText(
				str, text_color,
				px*sx, py*sy, sx, sy);
		}
		else
		{
			for (int i = 0; i<(int)m_vd_pop_list.size(); i++)
			{
				VolumeData* vd = m_vd_pop_list[i];
				if (vd && vd->GetVR())
				{
					str = L"SLICES_" + std::to_wstring(i + 1) + L": " + std::to_wstring(vd->GetVR()->get_slice_num());
					px = gapw - nx / 2.0;
					py = ny / 2.0 - gaph*(3 + i) / 2;
					m_text_renderer->RenderText(
						str, text_color,
						px*sx, py*sy, sx, sy);
				}
			}
		}
	}
}

//depth buffer calculation
double RenderView::CalcZ(double z)
{
	double result = 0.0;
	if (m_persp)
	{
		if (z != 0.0)
		{
			result = (m_far_clip + m_near_clip) / (m_far_clip - m_near_clip) / 2.0 +
				(-m_far_clip*m_near_clip) / (m_far_clip - m_near_clip) / z + 0.5;
		}
	}
	else
		result = (z - m_near_clip) / (m_far_clip - m_near_clip);
	return result;
}

void RenderView::CalcFogRange()
{
	fluo::BBox bbox;
	bool use_box = false;
	if (m_cur_vol)
	{
		bbox = m_cur_vol->GetClippedBounds();
		use_box = true;
	}
	else if (!m_md_pop_list.empty())
	{
		for (size_t i = 0; i<m_md_pop_list.size(); ++i)
		{
			if (m_md_pop_list[i]->GetDisp())
			{
				bbox.extend(m_md_pop_list[i]->GetBounds());
				use_box = true;
			}
		}
	}

	if (use_box)
	{
		fluo::Transform mv;
		mv.set(glm::value_ptr(m_mv_mat));

		double minz, maxz;
		minz = std::numeric_limits<double>::max();
		maxz = -std::numeric_limits<double>::max();

		std::vector<fluo::Point> points;
		points.push_back(fluo::Point(bbox.Min().x(), bbox.Min().y(), bbox.Min().z()));
		points.push_back(fluo::Point(bbox.Min().x(), bbox.Min().y(), bbox.Max().z()));
		points.push_back(fluo::Point(bbox.Min().x(), bbox.Max().y(), bbox.Min().z()));
		points.push_back(fluo::Point(bbox.Min().x(), bbox.Max().y(), bbox.Max().z()));
		points.push_back(fluo::Point(bbox.Max().x(), bbox.Min().y(), bbox.Min().z()));
		points.push_back(fluo::Point(bbox.Max().x(), bbox.Min().y(), bbox.Max().z()));
		points.push_back(fluo::Point(bbox.Max().x(), bbox.Max().y(), bbox.Min().z()));
		points.push_back(fluo::Point(bbox.Max().x(), bbox.Max().y(), bbox.Max().z()));

		fluo::Point p;
		for (size_t i = 0; i<points.size(); ++i)
		{
			p = mv.transform(points[i]);
			minz = p.z()<minz ? p.z() : minz;
			maxz = p.z()>maxz ? p.z() : maxz;
		}

		minz = fabs(minz);
		maxz = fabs(maxz);
		m_fog_start = minz<maxz ? minz : maxz;
		m_fog_end = maxz>minz ? maxz : minz;
		if (m_pin_rot_ctr)
		{
			p = -mv.transform(m_pin_ctr);
			if (p.z() > m_fog_start && p.z() < m_fog_end)
				m_fog_start = p.z();
		}
	}
	else
	{
		m_fog_start = m_distance - m_radius / 2.0;
		m_fog_start = m_fog_start<0.0 ? 0.0 : m_fog_start;
		m_fog_end = m_distance + m_radius / 4.0;
	}
}

//draw the volume data only
void RenderView::Draw()
{
	int nx, ny;
	GetRenderSize(nx, ny);

	// clear color and depth buffers
	glClearDepth(1.0);
	glClearColor(m_bg_color.r(), m_bg_color.g(), m_bg_color.b(), 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, (GLint)nx, (GLint)ny);

	//projection
	HandleProjection(nx, ny, true);
	//Transformation
	HandleCamera(true);

	//gradient background
	if (glbin_settings.m_grad_bg)
		DrawGradBg();

	if (m_draw_all)
	{
		glm::mat4 mv_temp = m_mv_mat;
		m_mv_mat = GetDrawMat();

		if (m_use_fog)
			CalcFogRange();

		if (glbin_settings.m_test_wiref)
			DrawGrid();

		if (glbin_states.m_clip_display)
			DrawClippingPlanes(BACK_FACE);

		//setup
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//draw the volumes
		DrawVolumes();

		//draw the clipping planes
		if (glbin_states.m_clip_display)
			DrawClippingPlanes(FRONT_FACE);

		if (glbin_settings.m_test_wiref)
			DrawBounds();

		if (m_draw_annotations)
			DrawAnnotations();

		DrawCells();

		//traces
		DrawTraces();

		m_mv_mat = mv_temp;

		//obj independent items
		m_mv_mat = GetDrawWorldMat();

		if (m_draw_rulers)
			DrawRulers();

		m_mv_mat = mv_temp;
	}
}

//draw with depth peeling
void RenderView::DrawDP()
{
	int i;
	int nx, ny;
	GetRenderSize(nx, ny);
	std::string name;
	flvr::Framebuffer* peel_buffer = 0;

	//clear
	//	glDrawBuffer(GL_BACK);
	glClearDepth(1.0);
	glClearColor(m_bg_color.r(), m_bg_color.g(), m_bg_color.b(), 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, (GLint)nx, (GLint)ny);

	//projection
	HandleProjection(nx, ny, true);
	//Transformation
	HandleCamera(true);

	//gradient background
	if (glbin_settings.m_grad_bg)
		DrawGradBg();

	if (m_draw_all)
	{
		glm::mat4 mv_temp = m_mv_mat;
		m_mv_mat = GetDrawMat();

		bool use_fog_save = m_use_fog;
		if (m_use_fog)
			CalcFogRange();

		if (glbin_settings.m_test_wiref)
			DrawGrid();

		if (glbin_states.m_clip_display)
			DrawClippingPlanes(BACK_FACE);

		//setup
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		m_use_fog = false;

		//draw depth values of each layer into the buffers
		for (i = 0; i<glbin_settings.m_peeling_layers; i++)
		{
			name = "peel buffer" + std::to_string(i);
			peel_buffer =
				glbin_framebuffer_manager.framebuffer(
					flvr::FB_Depth_Float, nx, ny, name);
			if (peel_buffer)
			{
				peel_buffer->bind();
				peel_buffer->protect();
			}

			glClearDepth(1.0);
			glClear(GL_DEPTH_BUFFER_BIT);

			if (i == 0)
			{
				DrawMeshes(0);
			}
			else
			{
				glActiveTexture(GL_TEXTURE15);
				name = "peel buffer" + std::to_string(i-1);
				peel_buffer =
					glbin_framebuffer_manager.framebuffer(name);
				if (peel_buffer)
					peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
				glActiveTexture(GL_TEXTURE0);
				DrawMeshes(1);
				glActiveTexture(GL_TEXTURE15);
				glBindTexture(GL_TEXTURE_2D, 0);
				glActiveTexture(GL_TEXTURE0);
			}
		}

		//bind back the framebuffer
		BindRenderBuffer();

		//restore fog
		m_use_fog = use_fog_save;

		//draw depth peeling
		for (i = glbin_settings.m_peeling_layers; i >= 0; i--)
		{
			if (i == 0)
			{
				//draw volumes before the depth
				glActiveTexture(GL_TEXTURE15);
				name = "peel buffer" + std::to_string(0);
				peel_buffer =
					glbin_framebuffer_manager.framebuffer(name);
				if (peel_buffer)
					peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
				glActiveTexture(GL_TEXTURE0);
				DrawVolumes(1);
				glActiveTexture(GL_TEXTURE15);
				glBindTexture(GL_TEXTURE_2D, 0);
				glActiveTexture(GL_TEXTURE0);
			}
			else
			{
				if (glbin_settings.m_peeling_layers == 1)
				{
					//i == glbin_settings.m_peeling_layers == 1
					glActiveTexture(GL_TEXTURE15);
					name = "peel buffer" + std::to_string(0);
					peel_buffer =
						glbin_framebuffer_manager.framebuffer(name);
					if (peel_buffer)
						peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
					glActiveTexture(GL_TEXTURE0);
				}
				else if (glbin_settings.m_peeling_layers == 2)
				{
					glActiveTexture(GL_TEXTURE14);
					name = "peel buffer" + std::to_string(0);
					peel_buffer =
						glbin_framebuffer_manager.framebuffer(name);
					if (peel_buffer)
						peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
					glActiveTexture(GL_TEXTURE15);
					name = "peel buffer" + std::to_string(1);
					peel_buffer =
						glbin_framebuffer_manager.framebuffer(name);
					if (peel_buffer)
						peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
					glActiveTexture(GL_TEXTURE0);
				}
				else if (glbin_settings.m_peeling_layers > 2)
				{
					if (i == glbin_settings.m_peeling_layers)
					{
						glActiveTexture(GL_TEXTURE14);
						name = "peel buffer" + std::to_string(i-2);
						peel_buffer =
							glbin_framebuffer_manager.framebuffer(name);
						if (peel_buffer)
							peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
						glActiveTexture(GL_TEXTURE15);
						name = "peel buffer" + std::to_string(i-1);
						peel_buffer =
							glbin_framebuffer_manager.framebuffer(name);
						if (peel_buffer)
							peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
						glActiveTexture(GL_TEXTURE0);
					}
					else if (i == 1)
					{
						glActiveTexture(GL_TEXTURE14);
						name = "peel buffer" + std::to_string(0);
						peel_buffer =
							glbin_framebuffer_manager.framebuffer(name);
						if (peel_buffer)
							peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
						glActiveTexture(GL_TEXTURE15);
						name = "peel buffer" + std::to_string(1);
						peel_buffer =
							glbin_framebuffer_manager.framebuffer(name);
						if (peel_buffer)
							peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
						glActiveTexture(GL_TEXTURE0);
					}
					else
					{
						glActiveTexture(GL_TEXTURE13);
						name = "peel buffer" + std::to_string(i-2);
						peel_buffer =
							glbin_framebuffer_manager.framebuffer(name);
						if (peel_buffer)
							peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
						glActiveTexture(GL_TEXTURE14);
						name = "peel buffer" + std::to_string(i-1);
						peel_buffer =
							glbin_framebuffer_manager.framebuffer(name);
						if (peel_buffer)
							peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
						glActiveTexture(GL_TEXTURE15);
						name = "peel buffer" + std::to_string(i);
						peel_buffer =
							glbin_framebuffer_manager.framebuffer(name);
						if (peel_buffer)
							peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
						glActiveTexture(GL_TEXTURE0);
					}
				}

				//draw volumes
				if (glbin_settings.m_peeling_layers == 1)
					//i == glbin_settings.m_peeling_layers == 1
					DrawVolumes(5);//draw volume after 15
				else if (glbin_settings.m_peeling_layers == 2)
				{
					if (i == 2)
						DrawVolumes(2);//draw volume after 15
					else if (i == 1)
						DrawVolumes(4);//draw volume after 14 and before 15
				}
				else if (glbin_settings.m_peeling_layers > 2)
				{
					if (i == glbin_settings.m_peeling_layers)
						DrawVolumes(2);//draw volume after 15
					else if (i == 1)
						DrawVolumes(4);//draw volume after 14 and before 15
					else
						DrawVolumes(3);//draw volume after 14 and before 15
				}

				//draw meshes
				glEnable(GL_DEPTH_TEST);
				glDepthFunc(GL_LEQUAL);
				glEnable(GL_BLEND);
				glBlendEquation(GL_FUNC_ADD);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

				if (glbin_settings.m_peeling_layers == 1)
					//i == glbin_settings.m_peeling_layers == 1
					DrawMeshes(5);//draw mesh at 15
				else if (glbin_settings.m_peeling_layers == 2)
				{
					if (i == 2)
						DrawMeshes(2);//draw mesh after 14
					else if (i == 1)
						DrawMeshes(4);//draw mesh before 15
				}
				else if (glbin_settings.m_peeling_layers > 2)
				{
					if (i == glbin_settings.m_peeling_layers)
						DrawMeshes(2);//draw mesh after 14
					else if (i == 1)
						DrawMeshes(4);//draw mesh before 15
					else
						DrawMeshes(3);//draw mesh after 13 and before 15
				}

				glActiveTexture(GL_TEXTURE13);
				glBindTexture(GL_TEXTURE_2D, 0);
				glActiveTexture(GL_TEXTURE14);
				glBindTexture(GL_TEXTURE_2D, 0);
				glActiveTexture(GL_TEXTURE15);
				glBindTexture(GL_TEXTURE_2D, 0);
				glActiveTexture(GL_TEXTURE0);

			}
		}

		double darkness;
		if (GetMeshShadow(darkness))
			DrawOLShadowsMesh(darkness);

		if (glbin_states.m_clip_display)
			DrawClippingPlanes(FRONT_FACE);

		if (glbin_settings.m_test_wiref)
			DrawBounds();

		if (m_draw_annotations)
			DrawAnnotations();

		if (m_draw_rulers)
			DrawRulers();

		DrawCells();

		//traces
		DrawTraces();

		m_mv_mat = mv_temp;
	}
}

//draw meshes
//peel==true -- depth peeling
void RenderView::DrawMeshes(int peel)
{
	int nx, ny;
	GetRenderSize(nx, ny);
	GLint vp[4] = { 0, 0, (GLint)nx, (GLint)ny };

	for (int i = 0; i<(int)m_layer_list.size(); i++)
	{
		if (!m_layer_list[i])
			continue;
		if (m_layer_list[i]->IsA() == 3)
		{
			MeshData* md = (MeshData*)m_layer_list[i];
			if (md && md->GetDisp())
			{
				md->SetMatrices(m_mv_mat, m_proj_mat);
				md->SetFog(m_use_fog, m_fog_intensity, m_fog_start, m_fog_end);
				md->SetViewport(vp);
				md->Draw(peel);
			}
		}
		else if (m_layer_list[i]->IsA() == 6)
		{
			MeshGroup* group = (MeshGroup*)m_layer_list[i];
			if (group && group->GetDisp())
			{
				for (int j = 0; j<(int)group->GetMeshNum(); j++)
				{
					MeshData* md = group->GetMeshData(j);
					if (md && md->GetDisp())
					{
						md->SetMatrices(m_mv_mat, m_proj_mat);
						md->SetFog(m_use_fog, m_fog_intensity, m_fog_start, m_fog_end);
						md->SetViewport(vp);
						md->Draw(peel);
					}
				}
			}
		}
	}
}

//draw volumes
//peel==true -- depth peeling
void RenderView::DrawVolumes(int peel)
{
	int finished_bricks = 0;
	if (glbin_settings.m_mem_swap)
	{
		finished_bricks = flvr::TextureRenderer::get_finished_bricks();
		flvr::TextureRenderer::reset_finished_bricks();
	}

	PrepFinalBuffer();

	int nx, ny;
	GetRenderSize(nx, ny);

	flrd::RulerPoint *p0 = glbin_ruler_handler.GetPoint();

	//draw
	if (m_load_update ||
		glbin_settings.m_hologram_mode == 2 ||
		(!m_retain_finalbuffer &&
			m_int_mode != 2 &&
			m_int_mode != 7 &&
			m_updating) ||
			(!m_retain_finalbuffer &&
		(m_int_mode == 1 ||
			m_int_mode == 3 ||
			m_int_mode == 4 ||
			m_int_mode == 5 ||
			((m_int_mode == 6 ||
			m_int_mode == 9) &&
				!p0) ||
			m_int_mode == 8 ||
			m_force_clear)))
	{
		m_updating = false;
		m_force_clear = false;
		m_load_update = false;

		if (glbin_settings.m_update_order == 1)
		{
			if (m_interactive)
				ClearFinalBuffer();
			else/* if (m_clear_buffer)*/
			{
				ClearFinalBuffer();
				m_clear_buffer = false;
			}
		}
		else
			ClearFinalBuffer();

		GLboolean bCull = glIsEnabled(GL_CULL_FACE);
		glDisable(GL_CULL_FACE);

		PopVolumeList();

		std::vector<VolumeData*> quota_vd_list;
		if (glbin_settings.m_mem_swap)
		{
			//set start time for the texture renderer
			flvr::TextureRenderer::set_st_time(GET_TICK_COUNT());

			flvr::TextureRenderer::set_interactive(m_interactive);
			//if in interactive mode, do interactive bricking also
			if (m_interactive)
			{
				//calculate quota
				unsigned long total_bricks = flvr::TextureRenderer::get_total_brick_num();
				unsigned long quota_bricks = 1;// total_bricks / 2;
				unsigned long last_bricks = flvr::TextureRenderer::
					get_est_bricks(3);
				int adj_bricks = 0;
				unsigned long up_time = flvr::TextureRenderer::get_cor_up_time();
				unsigned long consumed_time = flvr::TextureRenderer::get_consumed_time();
				if (consumed_time == 0)
					quota_bricks = total_bricks;
				else if (consumed_time / up_time > total_bricks)
					quota_bricks = 1;
				else
				{
					adj_bricks = std::max(1, int(double(last_bricks) *
						double(up_time) / double(consumed_time)));
					quota_bricks = flvr::TextureRenderer::
						get_est_bricks(0, adj_bricks);
				}
				quota_bricks = std::min(total_bricks, quota_bricks);
				flvr::TextureRenderer::set_quota_bricks(quota_bricks);
				flvr::TextureRenderer::push_quota_brick(quota_bricks);

				unsigned long quota_bricks_chan = 0;
				if (m_vd_pop_list.size() > 1)
				{
					//priority: 1-selected channel; 2-group contains selected channel; 3-linear distance to above
					//not considering mask for now
					std::vector<VolumeData*>::iterator cur_iter;
					cur_iter = find(m_vd_pop_list.begin(), m_vd_pop_list.end(), m_cur_vol);
					size_t cur_index = distance(m_vd_pop_list.begin(), cur_iter);
					unsigned long vd_index;
					if (cur_iter != m_vd_pop_list.end())
					{
						VolumeData* vd;
						vd = *cur_iter;
						quota_vd_list.push_back(vd);
						unsigned long count_bricks = vd->GetBrickNum();
						quota_bricks_chan = std::min(count_bricks, quota_bricks);
						vd->GetVR()->set_quota_bricks_chan(quota_bricks_chan);
						unsigned long count = 0;
						while (count_bricks < quota_bricks &&
							quota_vd_list.size() < m_vd_pop_list.size())
						{
							if (count % 2 == 0)
								vd_index = cur_index + count / 2 + 1;
							else
								vd_index = cur_index - count / 2 - 1;
							count++;
							if (vd_index < 0 ||
								(size_t)vd_index >= m_vd_pop_list.size())
								continue;
							vd = m_vd_pop_list[vd_index];
							unsigned long brick_num = vd->GetBrickNum();
							quota_vd_list.push_back(vd);
							if (count_bricks + brick_num > quota_bricks)
								quota_bricks_chan = quota_bricks - count_bricks;
							else
								quota_bricks_chan = brick_num;
							vd->GetVR()->set_quota_bricks_chan(quota_bricks_chan);
							count_bricks += quota_bricks_chan;
						}
					}
				}
				else if (m_vd_pop_list.size() == 1)
				{
					quota_bricks_chan = quota_bricks;
					VolumeData* vd = m_vd_pop_list[0];
					if (vd)
						vd->GetVR()->set_quota_bricks_chan(quota_bricks_chan);
				}

				//get and set center point
				VolumeData* vd = m_cur_vol;
				if (!vd)
					if (m_vd_pop_list.size())
						vd = m_vd_pop_list[0];
				fluo::Point p;
				if (glbin_volume_point.GetPointVolumeBox(nx / 2.0, ny / 2.0, false, p) > 0.0 ||
					(vd && glbin_volume_point.GetPointPlane(nx / 2.0, ny / 2.0, 0, false, p) > 0.0))
				{
					int resx, resy, resz;
					double sclx, scly, sclz;
					double spcx, spcy, spcz;
					vd->GetResolution(resx, resy, resz);
					vd->GetScalings(sclx, scly, sclz);
					vd->GetSpacings(spcx, spcy, spcz);
					p = fluo::Point(p.x() / (resx*sclx*spcx),
						p.y() / (resy*scly*spcy),
						p.z() / (resz*sclz*spcz));
					flvr::TextureRenderer::set_qutoa_center(p);
				}
				else
					flvr::TextureRenderer::set_interactive(false);
			}
		}

		//handle intermixing modes
		if (m_vol_method == VOL_METHOD_MULTI)
		{
			if (glbin_settings.m_mem_swap &&
				flvr::TextureRenderer::get_interactive() &&
				quota_vd_list.size() > 0)
				DrawVolumesMulti(quota_vd_list, peel);
			else
				DrawVolumesMulti(m_vd_pop_list, peel);
			//draw masks
			if (m_draw_mask)
				DrawVolumesComp(m_vd_pop_list, true, peel);
		}
		else
		{
			int i, j;
			std::vector<VolumeData*> list;
			for (i = (int)m_layer_list.size() - 1; i >= 0; i--)
			{
				if (!m_layer_list[i])
					continue;
				switch (m_layer_list[i]->IsA())
				{
				case 2://volume data (this won't happen now)
				{
					VolumeData* vd = (VolumeData*)m_layer_list[i];
					if (vd && vd->GetDisp())
					{
						if (glbin_settings.m_mem_swap &&
							flvr::TextureRenderer::get_interactive() &&
							quota_vd_list.size() > 0)
						{
							if (find(quota_vd_list.begin(),
								quota_vd_list.end(), vd) !=
								quota_vd_list.end())
								list.push_back(vd);
						}
						else
							list.push_back(vd);
					}
				}
				break;
				case 5://group
				{
					if (!list.empty())
					{
						DrawVolumesComp(list, false, peel);
						//draw masks
						if (m_draw_mask)
							DrawVolumesComp(list, true, peel);
						list.clear();
					}
					DataGroup* group = (DataGroup*)m_layer_list[i];
					if (!group->GetDisp())
						continue;
					for (j = group->GetVolumeNum() - 1; j >= 0; j--)
					{
						VolumeData* vd = group->GetVolumeData(j);
						if (vd && vd->GetDisp())
						{
							if (glbin_settings.m_mem_swap &&
								flvr::TextureRenderer::get_interactive() &&
								quota_vd_list.size() > 0)
							{
								if (find(quota_vd_list.begin(),
									quota_vd_list.end(), vd) !=
									quota_vd_list.end())
									list.push_back(vd);
							}
							else
								list.push_back(vd);
						}
					}
					if (!list.empty())
					{
						if (group->GetBlendMode() == VOL_METHOD_MULTI)
							DrawVolumesMulti(list, peel);
						else
							DrawVolumesComp(list, false, peel);
						//draw masks
						if (m_draw_mask)
							DrawVolumesComp(list, true, peel);
						list.clear();
					}
				}
				break;
				}
			}
		}

		if (bCull) glEnable(GL_CULL_FACE);
	}

	//final composition
	DrawFinalBuffer();

	if (glbin_settings.m_mem_swap)
	{
		flvr::TextureRenderer::set_consumed_time(GET_TICK_COUNT() - flvr::TextureRenderer::get_st_time());
		if (flvr::TextureRenderer::get_start_update_loop() &&
			flvr::TextureRenderer::get_done_update_loop())
			flvr::TextureRenderer::reset_update_loop();
	}

	if (m_interactive)
	{
		m_interactive = false;
		m_clear_buffer = true;
		RefreshGL(2);
	}

	//if (TextureRenderer::get_mem_swap())
	//{
	//	if (finished_bricks == 0)
	//	{
	//		if (m_nodraw_count == 100)
	//		{
	//			TextureRenderer::set_done_update_loop();
	//			m_nodraw_count = 0;
	//		}
	//		else
	//			m_nodraw_count++;
	//	}
	//}
}

void RenderView::DrawAnnotations()
{
	int nx, ny;
	GetRenderSize(nx, ny);
	float sx, sy;
	sx = 2.0 / nx;
	sy = 2.0 / ny;
	float px, py;

	fluo::Transform mv;
	fluo::Transform p;
	mv.set(glm::value_ptr(m_mv_mat));
	p.set(glm::value_ptr(m_proj_mat));

	fluo::Color text_color = GetTextColor();

	for (size_t i = 0; i<m_layer_list.size(); i++)
	{
		if (!m_layer_list[i])
			continue;
		if (m_layer_list[i]->IsA() == 4)
		{
			Annotations* ann = (Annotations*)m_layer_list[i];
			if (!ann) continue;
			if (ann->GetDisp())
			{
				fluo::Point pos;
				std::wstring wstr;
				for (int j = 0; j<ann->GetTextNum(); ++j)
				{
					wstr = ann->GetTextText(j);
					pos = ann->GetTextPos(j);
					if (!ann->InsideClippingPlanes(pos))
						continue;
					pos = ann->GetTextTransformedPos(j);
					pos = mv.transform(pos);
					pos = p.transform(pos);
					if (pos.x() >= -1.0 && pos.x() <= 1.0 &&
						pos.y() >= -1.0 && pos.y() <= 1.0)
					{
						if (m_persp && (pos.z() <= 0.0 || pos.z() >= 1.0))
							continue;
						if (!m_persp && (pos.z() >= 0.0 || pos.z() <= -1.0))
							continue;
						px = pos.x()*nx / 2.0;
						py = pos.y()*ny / 2.0;
						m_text_renderer->RenderText(
							wstr, text_color,
							px*sx, py*sy, sx, sy);
					}
				}
			}
		}
	}
}

void RenderView::BindRenderBuffer()
{
	if (glbin_settings.m_hologram_mode == 1)
	{
		std::string vr_buf_name;
		if (m_vr_eye_idx)
			vr_buf_name = "vr right";
		else
			vr_buf_name = "vr left";
		flvr::Framebuffer* vr_buffer =
			glbin_framebuffer_manager.framebuffer(
				vr_buf_name);
		if (vr_buffer)
			vr_buffer->bind();
	}
	else if (glbin_settings.m_hologram_mode == 2)
	{
		int nx, ny;
		GetRenderSize(nx, ny);
		glbin_lg_renderer.BindRenderBuffer(nx, ny);
		glClearDepth(1.0);
		glClearColor(m_bg_color.r(), m_bg_color.g(), m_bg_color.b(), 0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	else
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//draw out the framebuffer after composition
void RenderView::PrepFinalBuffer()
{
	int nx, ny;
	GetRenderSize(nx, ny);

	//generate textures & buffer objects
	glActiveTexture(GL_TEXTURE0);
	//glEnable(GL_TEXTURE_2D);
	//frame buffer for final
	flvr::Framebuffer* final_buffer =
		glbin_framebuffer_manager.framebuffer(
			flvr::FB_Render_RGBA, nx, ny, "final");
	if (final_buffer)
		final_buffer->protect();
}

void RenderView::ClearFinalBuffer()
{
	flvr::Framebuffer* final_buffer =
		glbin_framebuffer_manager.framebuffer(
		"final");
	if (final_buffer)
		final_buffer->bind();
	//clear color buffer to black for compositing
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
}

void RenderView::DrawFinalBuffer()
{
	if (m_enlarge)
		return;

	//bind back the window frame buffer
	BindRenderBuffer();

	//draw the final buffer to the windows buffer
	glActiveTexture(GL_TEXTURE0);
	flvr::Framebuffer* final_buffer =
		glbin_framebuffer_manager.framebuffer("final");
	if (final_buffer)
		final_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	//glBlendFunc(GL_ONE, GL_ONE);
	glDisable(GL_DEPTH_TEST);

	//2d adjustment
	flvr::ShaderProgram* img_shader =
		glbin_img_shader_factory.shader(IMG_SHDR_BLEND_BRIGHT_BACKGROUND_HDR);
	if (img_shader)
	{
		if (!img_shader->valid())
			img_shader->create();
		img_shader->bind();
	}
	img_shader->setLocalParam(0, m_gamma.r(), m_gamma.g(), m_gamma.b(), 1.0);
	img_shader->setLocalParam(1, m_brightness.r(), m_brightness.g(), m_brightness.b(), 1.0);
	img_shader->setLocalParam(2, m_hdr.r(), m_hdr.g(), m_hdr.b(), 0.0);
	//2d adjustment

	DrawViewQuad();

	if (img_shader && img_shader->valid())
		img_shader->release();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	//compute the threshold value for picking volume
	if (m_update_rot_ctr && !m_free)
	{
		unsigned char pixel[4];
		int nx, ny;
		GetRenderSize(nx, ny);
		glReadPixels(nx / 2, ny / 2, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
		m_pin_pick_thresh = 0.8 * double(pixel[3]) / 255.0;
	}
}

void RenderView::PrepVRBuffer()
{
	if (m_use_openxr)
	{
		if (m_vr_eye_idx == 0)
			glbin_xr_renderer->BeginFrame();
	}

	int nx, ny;
	GetRenderSize(nx, ny);

	//generate textures & buffer objects
	glActiveTexture(GL_TEXTURE0);
	//glEnable(GL_TEXTURE_2D);
	//frame buffer for one eye
	std::string vr_buf_name;
	if (m_vr_eye_idx)
		vr_buf_name = "vr right";
	else
		vr_buf_name = "vr left";

	flvr::Framebuffer* vr_buffer =
		glbin_framebuffer_manager.framebuffer(
			flvr::FB_UChar_RGBA, nx, ny, vr_buf_name);
	if (vr_buffer)
		vr_buffer->protect();
}

void RenderView::ClearVRBuffer()
{
	BindRenderBuffer();
	//clear color buffer to black for compositing
	glClearDepth(1.0);
	glClearColor(m_bg_color.r(), m_bg_color.g(), m_bg_color.b(), 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RenderView::DrawVRBuffer()
{
	int vr_x, vr_y, gl_x, gl_y;
	GetRenderSize(vr_x, vr_y);
	gl_x = m_gl_size.w();
	gl_y = m_gl_size.h();
	if (glbin_settings.m_sbs)
		vr_x /= 2;
	int vp_y = std::round((double)gl_x * vr_y / vr_x / 2.0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, gl_x, vp_y);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	//draw in fluorender
	flvr::ShaderProgram* img_shader =
		glbin_img_shader_factory.shader(IMG_SHADER_TEXTURE_LOOKUP);
	if (img_shader)
	{
		if (!img_shader->valid())
			img_shader->create();
		img_shader->bind();
	}
	//left eye
	flvr::Framebuffer* buffer_left =
		glbin_framebuffer_manager.framebuffer(
			"vr left");
	if (buffer_left)
		buffer_left->bind_texture(GL_COLOR_ATTACHMENT0);
	flvr::VertexArray* quad_va =
		glbin_vertex_array_manager.vertex_array(flvr::VA_Left_Square);
	if (quad_va)
		quad_va->draw();

	//right eye
	flvr::Framebuffer* buffer_right =
		glbin_framebuffer_manager.framebuffer(
			"vr right");
	if (buffer_right)
		buffer_right->bind_texture(GL_COLOR_ATTACHMENT0);
	quad_va =
		glbin_vertex_array_manager.vertex_array(flvr::VA_Right_Square);
	if (quad_va)
		quad_va->draw();

	if (img_shader && img_shader->valid())
		img_shader->release();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	//draw in headset
	std::vector<flvr::Framebuffer*> fbos;
	//left eye
	fbos.push_back(buffer_left);
	//right eye
	fbos.push_back(buffer_right);
	//openxr draw
	if (m_use_openxr)
	{
		glbin_xr_renderer->Draw(fbos);
		glbin_xr_renderer->EndFrame();
	}
}

//draw multi volumes with depth consideration
//peel==true -- depth peeling
void RenderView::DrawVolumesMulti(std::vector<VolumeData*> &list, int peel)
{
	if (list.empty())
		return;

	if (!m_mvr)
		m_mvr = new flvr::MultiVolumeRenderer();
	if (!m_mvr)
		return;

	int nx, ny;
	GetRenderSize(nx, ny);
	GLint vp[4] = { 0, 0, (GLint)nx, (GLint)ny };
	GLfloat clear_color[4] = { 0, 0, 0, 0 };
	GLfloat zoom = m_scale_factor;
	GLfloat sf121 = Get121ScaleFactor();

	m_mvr->set_blend_slices(glbin_settings.m_micro_blend);

	int i;
	m_mvr->clear_vr();
	for (i = 0; i<(int)list.size(); i++)
	{
		VolumeData* vd = list[i];
		if (vd && vd->GetDisp())
		{
			flvr::VolumeRenderer* vr = vd->GetVR();
			if (vr)
			{
				//drawlabel
				if (vd->GetLabelMode() &&
					vd->GetMask(false) &&
					vd->GetLabel(false))
					vd->SetMaskMode(4);
				vd->SetMatrices(m_mv_mat, m_proj_mat, m_tex_mat);
				vd->SetFog(m_use_fog, m_fog_intensity, m_fog_start, m_fog_end);
				vr->set_zoom(zoom, sf121);
				m_mvr->add_vr(vr);
				m_mvr->set_sampling_rate(vr->get_sampling_rate());
				m_mvr->SetNoiseRed(vr->GetNoiseRed());
			}
		}
	}

	if (m_mvr->get_vr_num() <= 0)
		return;
	m_mvr->set_depth_peel(peel);

	// Set up transform
	fluo::Transform *tform = m_vd_pop_list[0]->GetTexture()->transform();
	float mvmat[16];
	tform->get_trans(mvmat);
	glm::mat4 mv_mat2 = glm::mat4(
		mvmat[0], mvmat[4], mvmat[8], mvmat[12],
		mvmat[1], mvmat[5], mvmat[9], mvmat[13],
		mvmat[2], mvmat[6], mvmat[10], mvmat[14],
		mvmat[3], mvmat[7], mvmat[11], mvmat[15]);
	mv_mat2 = m_vd_pop_list[0]->GetVR()->m_mv_mat * mv_mat2;
	m_mvr->set_matrices(mv_mat2,
		m_vd_pop_list[0]->GetVR()->m_proj_mat,
		m_vd_pop_list[0]->GetVR()->m_tex_mat);

	//generate textures & buffer objects
	//frame buffer for each volume
	flvr::Framebuffer* chann_buffer =
		glbin_framebuffer_manager.framebuffer(
			flvr::FB_Render_RGBA, nx, ny, "channel");
	//bind the fbo
	if (chann_buffer)
	{
		chann_buffer->protect();
		chann_buffer->bind();
		m_cur_framebuffer = chann_buffer->id();
	}
	if (!glbin_settings.m_mem_swap ||
		(glbin_settings.m_mem_swap &&
			flvr::TextureRenderer::get_clear_chan_buffer()))
	{
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT);
		flvr::
			TextureRenderer::reset_clear_chan_buffer();
	}

	//draw multiple volumes at the same time
	m_mvr->set_viewport(vp);
	m_mvr->set_clear_color(clear_color);
	m_mvr->set_cur_framebuffer(m_cur_framebuffer);
	m_mvr->draw(glbin_settings.m_test_wiref, glbin_settings.m_mouse_int, m_interactive, !m_persp, m_intp);

	//draw shadows
	DrawOLShadows(list);

	//bind fbo for final composition
	flvr::Framebuffer* final_buffer =
		glbin_framebuffer_manager.framebuffer(
		"final");
	if (final_buffer)
		final_buffer->bind();
	glActiveTexture(GL_TEXTURE0);
	if (chann_buffer)
		chann_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
	//build mipmap
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glEnable(GL_BLEND);
	if (m_vol_method == VOL_METHOD_COMP)
		glBlendFunc(GL_ONE, GL_ONE);
	else
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	//2d adjustment
	flvr::ShaderProgram* img_shader =
		glbin_img_shader_factory.shader(IMG_SHDR_BRIGHTNESS_CONTRAST_HDR);
	if (img_shader)
	{
		if (!img_shader->valid())
		{
			img_shader->create();
		}
		img_shader->bind();
	}
	fluo::Color gamma, brightness, hdr;
	VolumeData* vd = list[0];
	gamma = vd->GetGammaColor();
	brightness = vd->GetBrightness();
	hdr = vd->GetHdr();
	img_shader->setLocalParam(0, gamma.r(), gamma.g(), gamma.b(), 1.0);
	img_shader->setLocalParam(1, brightness.r(), brightness.g(), brightness.b(), 1.0);
	img_shader->setLocalParam(2, hdr.r(), hdr.g(), hdr.b(), 0.0);
	//2d adjustment

	DrawViewQuad();

	if (img_shader && img_shader->valid())
		img_shader->release();
}

//Draw the volmues with compositing
//peel==true -- depth peeling
void RenderView::DrawVolumesComp(std::vector<VolumeData*> &list, bool mask, int peel)
{
	if (list.size() <= 0)
		return;

	int i;

	//count volumes with mask
	int cnt_mask = 0;
	for (i = 0; i<(int)list.size(); i++)
	{
		VolumeData* vd = list[i];
		if (!vd || !vd->GetDisp())
			continue;
		if (vd->GetTexture() && vd->GetTexture()->nmask() != -1)
			cnt_mask++;
	}

	if (mask && cnt_mask == 0)
		return;

	int nx, ny;
	GetRenderSize(nx, ny);

	//generate textures & buffer objects
	//frame buffer for each volume
	flvr::Framebuffer* chann_buffer =
		glbin_framebuffer_manager.framebuffer(
		flvr::FB_Render_RGBA, nx, ny, "channel");
	if (chann_buffer)
		chann_buffer->protect();

	//draw each volume to fbo
	for (i = 0; i<(int)list.size(); i++)
	{
		VolumeData* vd = list[i];
		if (!vd || !vd->GetDisp())
			continue;
		if (mask)
		{
			//drawlabel
			if (vd->GetLabelMode() &&
				vd->GetMask(false) &&
				vd->GetLabel(false))
				continue;

			if (vd->GetTexture() && vd->GetTexture()->nmask() != -1)
			{
				vd->SetMaskMode(1);
				int vol_method = m_vol_method;
				m_vol_method = VOL_METHOD_COMP;
				if (vd->GetMode() == 1)
					DrawMIP(vd, peel);
				else
					DrawOVER(vd, mask, peel);
				vd->SetMaskMode(0);
				m_vol_method = vol_method;
			}
		}
		else
		{
			if (vd->GetBlendMode() != 2)
			{
				//drawlabel
				if (vd->GetLabelMode() &&
					vd->GetMask(false) &&
					vd->GetLabel(false))
					vd->SetMaskMode(4);

				if (vd->GetMode() == 1)
					DrawMIP(vd, peel);
				else
					DrawOVER(vd, mask, peel);
			}
		}
	}
}

void RenderView::DrawMIP(VolumeData* vd, int peel)
{
	int nx, ny;
	GetRenderSize(nx, ny);
	GLint vp[4] = { 0, 0, (GLint)nx, (GLint)ny };
	GLfloat clear_color[4] = { 0, 0, 0, 0 };

	bool do_mip = true;
	if (glbin_settings.m_mem_swap &&
		flvr::TextureRenderer::get_start_update_loop() &&
		!flvr::TextureRenderer::get_done_update_loop())
	{
		unsigned int rn_time = GET_TICK_COUNT();
		if (rn_time - flvr::TextureRenderer::get_st_time() >
			flvr::TextureRenderer::get_up_time())
			return;
		if (vd->GetVR()->get_done_loop(1))
			do_mip = false;
	}

	bool shading = vd->GetVR()->get_shading();
	bool shadow = vd->GetShadowEnable();
	int color_mode = vd->GetColormapMode();
	bool enable_alpha = vd->GetAlphaEnable();
	flvr::ShaderProgram* img_shader = 0;

	flvr::Framebuffer* chann_buffer =
		glbin_framebuffer_manager.framebuffer("channel");
	flvr::Framebuffer* overlay_buffer = 0;
	if (do_mip)
	{
		//before rendering this channel, save final buffer to temp buffer
		if (glbin_settings.m_mem_swap &&
			flvr::TextureRenderer::get_start_update_loop() &&
			flvr::TextureRenderer::get_save_final_buffer())
		{
			flvr::TextureRenderer::reset_save_final_buffer();

			//bind temporary framebuffer for comp in stream mode
			flvr::Framebuffer* temp_buffer =
				glbin_framebuffer_manager.framebuffer(
				flvr::FB_Render_RGBA, nx, ny, "temporary");
			if (temp_buffer)
			{
				temp_buffer->bind();
				temp_buffer->protect();
			}
			glClearColor(0.0, 0.0, 0.0, 0.0);
			glClear(GL_COLOR_BUFFER_BIT);
			glActiveTexture(GL_TEXTURE0);
			flvr::Framebuffer* final_buffer =
				glbin_framebuffer_manager.framebuffer("final");
			if (final_buffer)
				final_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
			glDisable(GL_BLEND);
			glDisable(GL_DEPTH_TEST);

			img_shader =
				glbin_img_shader_factory.shader(IMG_SHADER_TEXTURE_LOOKUP);
			if (img_shader)
			{
				if (!img_shader->valid())
					img_shader->create();
				img_shader->bind();
			}
			DrawViewQuad();
			if (img_shader && img_shader->valid())
				img_shader->release();

			glBindTexture(GL_TEXTURE_2D, 0);
		}

		//bind the fbo
		overlay_buffer =
			glbin_framebuffer_manager.framebuffer(
				flvr::FB_Render_RGBA, nx, ny);
		if (overlay_buffer)
		{
			overlay_buffer->bind();
			overlay_buffer->protect();
			m_cur_framebuffer = overlay_buffer->id();
		}


		bool clear = !glbin_settings.m_mem_swap ||
			(glbin_settings.m_mem_swap && flvr::TextureRenderer::get_clear_chan_buffer());
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT);
		//flvr::TextureRenderer::reset_clear_chan_buffer();
		//DBGPRINT(L"overlay cleared\n");

		if (vd->GetVR())
			vd->GetVR()->set_depth_peel(peel);
		vd->GetVR()->set_shading(false);
		//turn off colormap proj
		int saved_colormap_proj = vd->GetColormapProj();
		if (color_mode == 0)
			vd->SetColormapProj(0);
		if (color_mode == 1)
		{
			vd->SetMode(3);
			vd->SetFog(false, m_fog_intensity, m_fog_start, m_fog_end);
		}
		else
		{
			vd->SetMode(1);
			vd->SetFog(m_use_fog, m_fog_intensity, m_fog_start, m_fog_end);
		}
		//turn off alpha
		if (color_mode == 1)
			vd->SetAlphaEnable(false);
		//draw
		vd->SetStreamMode(1);
		vd->SetMatrices(m_mv_mat, m_proj_mat, m_tex_mat);
		vd->SetViewport(vp);
		vd->SetClearColor(clear_color);
		vd->SetCurFramebuffer(m_cur_framebuffer);
		vd->Draw(!m_persp, glbin_settings.m_mouse_int, m_interactive, m_scale_factor, Get121ScaleFactor());
		//restore
		if (color_mode == 0)
			vd->SetColormapProj(saved_colormap_proj);
		if (color_mode == 1)
		{
			vd->RestoreMode();
			//restore alpha
			vd->SetAlphaEnable(enable_alpha);
		}

		//bind channel fbo for final composition
		if (chann_buffer)
			chann_buffer->bind();
		//clear = !glbin_settings.m_mem_swap ||
		//	(glbin_settings.m_mem_swap && flvr::TextureRenderer::get_clear_chan_buffer());
		if (clear)
		{
			glClearColor(0.0, 0.0, 0.0, 0.0);
			glClear(GL_COLOR_BUFFER_BIT);
			flvr::TextureRenderer::reset_clear_chan_buffer();
			//DBGPRINT(L"chan buffer cleared\n");
		}
		glActiveTexture(GL_TEXTURE0);
		if (overlay_buffer)
		{
			//ok to unprotect
			overlay_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
			overlay_buffer->unprotect();
		}
		glEnable(GL_BLEND);
		if (glbin_settings.m_mem_swap && !vd->GetVR()->get_done_loop(0))
		{
			glBlendEquation(GL_MAX);
			glBlendFunc(GL_ONE, GL_ONE);
		}
		else
		{
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		}

		if (color_mode == 1)
		{
			//2d adjustment
			if (vd->GetColormapProj())
				img_shader = glbin_img_shader_factory.shader(
					IMG_SHDR_GRADIENT_PROJ_MAP, vd->GetColormap());
			else
				img_shader = glbin_img_shader_factory.shader(
					IMG_SHDR_GRADIENT_MAP, vd->GetColormap());
			if (img_shader)
			{
				if (!img_shader->valid())
				{
					img_shader->create();
				}
				img_shader->bind();
			}
			double lo, hi;
			vd->GetColormapValues(lo, hi);
			img_shader->setLocalParam(
				0, lo, hi, hi - lo, enable_alpha ? 0.0 : 1.0);
			fluo::Color c = vd->GetColor();
			img_shader->setLocalParam(
				6, c.r(), c.g(), c.b(), vd->GetColormapInv());
			img_shader->setLocalParam(
				9, c.r(), c.g(), c.b(), 0.0);
			//2d adjustment
		}
		else
		{
			img_shader =
				glbin_img_shader_factory.shader(IMG_SHADER_TEXTURE_LOOKUP);
		}

		if (img_shader)
		{
			if (!img_shader->valid())
				img_shader->create();
			img_shader->bind();
		}
		DrawViewQuad();
		if (img_shader && img_shader->valid())
			img_shader->release();

		if (color_mode == 1 &&
			img_shader &&
			img_shader->valid())
		{
			img_shader->release();
		}
	}

	if (shading)
	{
		DrawOLShading(vd);
	}

	if (shadow)
	{
		std::vector<VolumeData*> list;
		list.push_back(vd);
		DrawOLShadows(list);
	}

	//bind fbo for final composition
	flvr::Framebuffer* final_buffer =
		glbin_framebuffer_manager.framebuffer(
		"final");
	if (final_buffer)
		final_buffer->bind();

	if (glbin_settings.m_mem_swap)
	{
		//restore temp buffer to final buffer
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
		flvr::Framebuffer* temp_buffer =
			glbin_framebuffer_manager.framebuffer(
				"temporary");
		if (temp_buffer)
		{
			//bind tex from temp buffer
			//it becomes unprotected afterwards
			temp_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
			//temp_buffer->unprotect();
		}
		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);

		img_shader =
			glbin_img_shader_factory.shader(IMG_SHADER_TEXTURE_LOOKUP);
		if (img_shader)
		{
			if (!img_shader->valid())
				img_shader->create();
			img_shader->bind();
		}
		DrawViewQuad();
		if (img_shader && img_shader->valid())
			img_shader->release();

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glActiveTexture(GL_TEXTURE0);
	if (chann_buffer)
		chann_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
	//build mipmap
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	if (m_vol_method == VOL_METHOD_COMP)
		glBlendFunc(GL_ONE, GL_ONE);
	else
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	//2d adjustment
	img_shader =
		glbin_img_shader_factory.shader(IMG_SHDR_BRIGHTNESS_CONTRAST_HDR);
	if (img_shader)
	{
		if (!img_shader->valid())
			img_shader->create();
		img_shader->bind();
	}
	fluo::Color gamma = vd->GetGammaColor();
	fluo::Color brightness = vd->GetBrightness();
	img_shader->setLocalParam(0, gamma.r(), gamma.g(), gamma.b(), 1.0);
	img_shader->setLocalParam(1, brightness.r(), brightness.g(), brightness.b(), 1.0);
	fluo::Color hdr = vd->GetHdr();
	img_shader->setLocalParam(2, hdr.r(), hdr.g(), hdr.b(), 0.0);
	//2d adjustment

	DrawViewQuad();

	if (img_shader && img_shader->valid())
		img_shader->release();

	vd->GetVR()->set_shading(shading);
	vd->SetColormapMode(color_mode);

	//if vd is duplicated
	//if (glbin_settings.m_mem_swap &&
	//	flvr::TextureRenderer::get_done_current_chan())
	//{
	//	vector<flvr::TextureBrick*> *bricks =
	//		vd->GetTexture()->get_bricks();
	//	for (int i = 0; i < bricks->size(); i++)
	//		(*bricks)[i]->set_drawn(false);
	//}
}

void RenderView::DrawOVER(VolumeData* vd, bool mask, int peel)
{
	int nx, ny;
	GetRenderSize(nx, ny);
	GLint vp[4] = { 0, 0, (GLint)nx, (GLint)ny };
	GLfloat clear_color[4] = { 0, 0, 0, 0 };

	flvr::ShaderProgram* img_shader = 0;

	bool do_over = true;
	if (glbin_settings.m_mem_swap &&
		flvr::TextureRenderer::get_start_update_loop() &&
		!flvr::TextureRenderer::get_done_update_loop())
	{
		unsigned int rn_time = GET_TICK_COUNT();
		if (rn_time - flvr::TextureRenderer::get_st_time() >
			flvr::TextureRenderer::get_up_time())
			return;
		if (mask)
		{
			if (vd->GetVR()->get_done_loop(4))
				do_over = false;
		}
		else
		{
			if (vd->GetVR()->get_done_loop(0))
				do_over = false;
		}
	}

	flvr::Framebuffer* chann_buffer =
		glbin_framebuffer_manager.framebuffer("channel");
	if (do_over)
	{
		//before rendering this channel, save final buffer to temp buffer
		if (glbin_settings.m_mem_swap &&
			flvr::TextureRenderer::get_start_update_loop() &&
			flvr::TextureRenderer::get_save_final_buffer())
		{
			flvr::TextureRenderer::reset_save_final_buffer();

			//bind temporary framebuffer for comp in stream mode
			flvr::Framebuffer* temp_buffer =
				glbin_framebuffer_manager.framebuffer(
					flvr::FB_Render_RGBA, nx, ny, "temporary");
			if (temp_buffer)
			{
				temp_buffer->bind();
				temp_buffer->protect();
			}
			glClearColor(0.0, 0.0, 0.0, 0.0);
			glClear(GL_COLOR_BUFFER_BIT);
			glActiveTexture(GL_TEXTURE0);
			flvr::Framebuffer* final_buffer =
				glbin_framebuffer_manager.framebuffer("final");
			if (final_buffer)
				final_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
			glDisable(GL_BLEND);
			glDisable(GL_DEPTH_TEST);

			img_shader =
				glbin_img_shader_factory.shader(IMG_SHADER_TEXTURE_LOOKUP);
			if (img_shader)
			{
				if (!img_shader->valid())
					img_shader->create();
				img_shader->bind();
			}
			DrawViewQuad();
			if (img_shader && img_shader->valid())
				img_shader->release();

			glBindTexture(GL_TEXTURE_2D, 0);
		}
		//bind the fbo
		if (chann_buffer)
		{
			chann_buffer->bind();
			m_cur_framebuffer = chann_buffer->id();
		}

		if (!glbin_settings.m_mem_swap ||
			(glbin_settings.m_mem_swap &&
			flvr::TextureRenderer::get_clear_chan_buffer()))
		{
			glClearColor(0.0, 0.0, 0.0, 0.0);
			glClear(GL_COLOR_BUFFER_BIT);
			flvr::TextureRenderer::reset_clear_chan_buffer();
			//DBGPRINT(L"chan buffer cleared\n");
		}

		if (vd->GetVR())
			vd->GetVR()->set_depth_peel(peel);
		if (mask)
			vd->SetStreamMode(4);
		else
			vd->SetStreamMode(0);
		vd->SetMatrices(m_mv_mat, m_proj_mat, m_tex_mat);
		vd->SetFog(m_use_fog, m_fog_intensity, m_fog_start, m_fog_end);
		vd->SetViewport(vp);
		vd->SetClearColor(clear_color);
		vd->SetCurFramebuffer(m_cur_framebuffer);
		vd->Draw(!m_persp, glbin_settings.m_mouse_int, m_interactive, m_scale_factor, Get121ScaleFactor());
	}

	if (vd->GetShadowEnable())
	{
		std::vector<VolumeData*> list;
		list.push_back(vd);
		DrawOLShadows(list);
	}

	//bind fbo for final composition
	flvr::Framebuffer* final_buffer =
		glbin_framebuffer_manager.framebuffer(
		"final");
	if (final_buffer)
		final_buffer->bind();

	if (glbin_settings.m_mem_swap)
	{
		//restore temp buffer to final buffer
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
		flvr::Framebuffer* temp_buffer =
			glbin_framebuffer_manager.framebuffer(
			"temporary");
		if (temp_buffer)
		{
			//temp buffer becomes unused after texture is bound
			//ok to unprotect
			temp_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
			//temp_buffer->unprotect();
		}
		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);

		img_shader =
			glbin_img_shader_factory.shader(IMG_SHADER_TEXTURE_LOOKUP);
		if (img_shader)
		{
			if (!img_shader->valid())
				img_shader->create();
			img_shader->bind();
		}
		DrawViewQuad();
		if (img_shader && img_shader->valid())
			img_shader->release();

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glActiveTexture(GL_TEXTURE0);
	if (chann_buffer)
		chann_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
	//build mipmap
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glEnable(GL_BLEND);
	if (m_vol_method == VOL_METHOD_COMP)
		glBlendFunc(GL_ONE, GL_ONE);
	else
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	//2d adjustment
	img_shader =
		glbin_img_shader_factory.shader(IMG_SHDR_BRIGHTNESS_CONTRAST_HDR);
	if (img_shader)
	{
		if (!img_shader->valid())
			img_shader->create();
		img_shader->bind();
	}
	fluo::Color gamma = vd->GetGammaColor();
	fluo::Color brightness = vd->GetBrightness();
	img_shader->setLocalParam(0, gamma.r(), gamma.g(), gamma.b(), 1.0);
	img_shader->setLocalParam(1, brightness.r(), brightness.g(), brightness.b(), 1.0);
	fluo::Color hdr = vd->GetHdr();
	img_shader->setLocalParam(2, hdr.r(), hdr.g(), hdr.b(), 0.0);
	//2d adjustment

	DrawViewQuad();

	if (img_shader && img_shader->valid())
		img_shader->release();

	//if vd is duplicated
	//if (glbin_settings.m_mem_swap &&
	//	flvr::TextureRenderer::get_done_current_chan())
	//{
	//	vector<flvr::TextureBrick*> *bricks =
	//		vd->GetTexture()->get_bricks();
	//	for (int i = 0; i < bricks->size(); i++)
	//		(*bricks)[i]->set_drawn(false);
	//}
}

void RenderView::DrawOLShading(VolumeData* vd)
{
	if (glbin_settings.m_mem_swap &&
		!flvr::TextureRenderer::get_done_current_chan())
		return;

	int nx, ny;
	GetRenderSize(nx, ny);
	//if (glbin_settings.m_mem_swap &&
	//	flvr::TextureRenderer::get_start_update_loop() &&
	//	!flvr::TextureRenderer::get_done_update_loop())
	//{
	//	unsigned int rn_time = GET_TICK_COUNT();
	//	if (rn_time - flvr::TextureRenderer::get_st_time() >
	//		flvr::TextureRenderer::get_up_time())
	//		return;
	//	if (vd->GetVR()->get_done_loop(2))
	//		return;
	//}

	//shading pass
	flvr::Framebuffer* overlay_buffer =
		glbin_framebuffer_manager.framebuffer(
		flvr::FB_Render_RGBA, nx, ny);
	if (overlay_buffer)
	{
		overlay_buffer->bind();
		overlay_buffer->protect();
	}
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	vd->GetVR()->set_shading(true);
	bool alpha = vd->GetAlphaEnable();
	vd->SetAlphaEnable(true);
	vd->SetMode(2);
	int colormode = vd->GetColormapMode();
	vd->SetStreamMode(2);
	vd->SetMatrices(m_mv_mat, m_proj_mat, m_tex_mat);
	vd->SetFog(m_use_fog, m_fog_intensity, m_fog_start, m_fog_end);
	vd->Draw(!m_persp, glbin_settings.m_mouse_int, m_interactive, m_scale_factor, Get121ScaleFactor());
	vd->RestoreMode();
	vd->SetColormapMode(colormode);
	vd->SetAlphaEnable(alpha);

	//bind fbo for final composition
	flvr::Framebuffer* chann_buffer =
		glbin_framebuffer_manager.framebuffer("channel");
	if (chann_buffer)
		chann_buffer->bind();
	glActiveTexture(GL_TEXTURE0);
	if (overlay_buffer)
	{
		//ok to unprotect
		overlay_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
		overlay_buffer->unprotect();
	}
	glEnable(GL_BLEND);
	glBlendFunc(GL_ZERO, GL_SRC_COLOR);
	//glBlendEquation(GL_MIN);
	glDisable(GL_DEPTH_TEST);

	flvr::ShaderProgram* img_shader =
		glbin_img_shader_factory.shader(IMG_SHADER_TEXTURE_LOOKUP);
	if (img_shader)
	{
		if (!img_shader->valid())
			img_shader->create();
		img_shader->bind();
	}
	DrawViewQuad();
	if (img_shader && img_shader->valid())
		img_shader->release();

	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}

void RenderView::DrawOLShadows(std::vector<VolumeData*> &vlist)
{
	if (vlist.empty())
		return;
	if (glbin_settings.m_mem_swap &&
		!flvr::TextureRenderer::get_done_current_chan())
		return;

	int nx, ny;
	GetRenderSize(nx, ny);
	GLint vp[4] = { 0, 0, (GLint)nx, (GLint)ny };
	GLfloat clear_color[4] = { 1, 1, 1, 1 };

	size_t i;
	bool has_shadow = false;
	std::vector<int> colormodes;
	std::vector<bool> shadings;
	std::vector<VolumeData*> list;
	//generate list
	for (i = 0; i<vlist.size(); i++)
	{
		VolumeData* vd = vlist[i];
		if (vd && vd->GetShadowEnable())
		{
			colormodes.push_back(vd->GetColormapMode());
			shadings.push_back(vd->GetVR()->get_shading());
			list.push_back(vd);
			has_shadow = true;
		}
	}

	if (!has_shadow)
		return;

	//if (glbin_settings.m_mem_swap &&
	//	flvr::TextureRenderer::get_start_update_loop() &&
	//	!flvr::TextureRenderer::get_done_update_loop())
	//{
	//	unsigned int rn_time = GET_TICK_COUNT();
	//	if (rn_time - flvr::TextureRenderer::get_st_time() >
	//		flvr::TextureRenderer::get_up_time())
	//		return;
	//	if (list.size() == 1 && list[0]->GetShadowEnable())
	//		if (list[0]->GetVR()->get_done_loop(3))
	//			return;
	//}

	flvr::Framebuffer* overlay_buffer =
		glbin_framebuffer_manager.framebuffer(
			flvr::FB_Render_RGBA, nx, ny);
	if (overlay_buffer)
	{
		overlay_buffer->bind();
		overlay_buffer->protect();
		m_cur_framebuffer = overlay_buffer->id();
	}

	if (!glbin_settings.m_mem_swap ||
		(glbin_settings.m_mem_swap &&
		flvr::TextureRenderer::get_clear_chan_buffer()))
	{
		glClearColor(1.0, 1.0, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		flvr::TextureRenderer::reset_clear_chan_buffer();
	}
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	double shadow_darkness = 0.0;

	if (list.empty())
		;
	else if (list.size() == 1)
	{
		VolumeData* vd = list[0];
		//save
		int colormode = vd->GetColormapMode();
		bool shading = vd->GetVR()->get_shading();
		//set to draw depth
		vd->GetVR()->set_shading(false);
		vd->SetMode(0);
		vd->SetColormapMode(2);
		if (overlay_buffer)
			vd->Set2dDmap(overlay_buffer->tex_id(GL_COLOR_ATTACHMENT0));
		int msk_mode = vd->GetMaskMode();
		vd->SetMaskMode(0);
		//draw
		vd->SetStreamMode(3);
		vd->SetMatrices(m_mv_mat, m_proj_mat, m_tex_mat);
		vd->SetFog(m_use_fog, m_fog_intensity, m_fog_start, m_fog_end);
		vd->SetViewport(vp);
		vd->SetClearColor(clear_color);
		vd->SetCurFramebuffer(m_cur_framebuffer);
		vd->Draw(!m_persp, glbin_settings.m_mouse_int, m_interactive, m_scale_factor, Get121ScaleFactor());
		//restore
		vd->RestoreMode();
		vd->SetMaskMode(msk_mode);
		vd->SetColormapMode(colormode);
		vd->GetVR()->set_shading(shading);
		shadow_darkness = vd->GetShadowIntensity();
	}
	else
	{
		m_mvr->clear_vr();
		for (i = 0; i<list.size(); i++)
		{
			VolumeData* vd = list[i];
			vd->GetVR()->set_shading(false);
			vd->SetMode(0);
			vd->SetColormapMode(2);
			if (overlay_buffer)
				vd->Set2dDmap(overlay_buffer->tex_id(GL_COLOR_ATTACHMENT0));
			flvr::VolumeRenderer* vr = list[i]->GetVR();
			if (vr)
			{
				list[i]->SetMatrices(m_mv_mat, m_proj_mat, m_tex_mat);
				list[i]->SetFog(m_use_fog, m_fog_intensity, m_fog_start, m_fog_end);
				m_mvr->add_vr(vr);
				m_mvr->set_sampling_rate(vr->get_sampling_rate());
				m_mvr->SetNoiseRed(vr->GetNoiseRed());
			}
		}
		//draw
		m_mvr->set_viewport(vp);
		m_mvr->set_clear_color(clear_color);
		m_mvr->set_cur_framebuffer(m_cur_framebuffer);
		m_mvr->draw(glbin_settings.m_test_wiref, glbin_settings.m_mouse_int, m_interactive, !m_persp, m_intp);

		for (i = 0; i<list.size(); i++)
		{
			VolumeData* vd = list[i];
			vd->RestoreMode();
			vd->SetColormapMode(colormodes[i]);
			vd->GetVR()->set_shading(shadings[i]);
		}
		shadow_darkness = list[0]->GetShadowIntensity();
	}

	//
	if (!glbin_settings.m_mem_swap ||
		(glbin_settings.m_mem_swap &&
		flvr::TextureRenderer::get_clear_chan_buffer()))
	{
		//shadow pass
		//bind the fbo
		flvr::Framebuffer* temp_buffer =
			glbin_framebuffer_manager.framebuffer(
				flvr::FB_Render_RGBA, nx, ny, "temp");
		if (temp_buffer)
		{
			temp_buffer->bind();
			temp_buffer->protect();
		}
		glClearColor(1.0, 1.0, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
		if (overlay_buffer)
		{
			//ok to unprotect
			overlay_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
			overlay_buffer->unprotect();
		}
		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);

		//2d adjustment
		flvr::ShaderProgram* img_shader =
			glbin_img_shader_factory.shader(IMG_SHDR_DEPTH_TO_GRADIENT);
		if (img_shader)
		{
			if (!img_shader->valid())
				img_shader->create();
			img_shader->bind();
		}
		img_shader->setLocalParam(0, 1.0 / nx, 1.0 / ny, m_persp ? 2e10 : 1e6, 0.0);
		img_shader->setLocalParam(1,
			glbin_settings.m_shadow_dir_x,
			glbin_settings.m_shadow_dir_y, 0.0, 0.0);

		//2d adjustment

		DrawViewQuad();

		if (img_shader && img_shader->valid())
			img_shader->release();

		//bind fbo for final composition
		flvr::Framebuffer* chann_buffer =
			glbin_framebuffer_manager.framebuffer("channel");
		if (chann_buffer)
			chann_buffer->bind();
		glActiveTexture(GL_TEXTURE0);
		if (temp_buffer)
		{
			temp_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
			temp_buffer->unprotect();
		}
		glEnable(GL_BLEND);
		glBlendFunc(GL_ZERO, GL_SRC_COLOR);
		glDisable(GL_DEPTH_TEST);

		//2d adjustment
		img_shader =
			glbin_img_shader_factory.shader(IMG_SHDR_GRADIENT_TO_SHADOW);
		if (img_shader)
		{
			if (!img_shader->valid())
				img_shader->create();
			img_shader->bind();
		}
		img_shader->setLocalParam(0, 1.0 / nx, 1.0 / ny, std::max(m_scale_factor, 1.0), 0.0);
		img_shader->setLocalParam(1, shadow_darkness, 0.0, 0.0, 0.0);
		glActiveTexture(GL_TEXTURE1);
		if (chann_buffer)
			chann_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
		//2d adjustment

		DrawViewQuad();

		if (img_shader && img_shader->valid())
			img_shader->release();
	}

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);

	glBlendEquation(GL_FUNC_ADD);
}

void RenderView::DrawOLShadowsMesh(double darkness)
{
	int nx, ny;
	GetRenderSize(nx, ny);

	//shadow pass
	//bind the fbo
	flvr::Framebuffer* overlay_buffer =
		glbin_framebuffer_manager.framebuffer(
		flvr::FB_Render_RGBA, nx, ny);
	if (overlay_buffer)
	{
		overlay_buffer->bind();
		overlay_buffer->protect();
	}
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	std::string name = "peel buffer" + std::to_string(0);
	flvr::Framebuffer* peel_buffer =
		glbin_framebuffer_manager.framebuffer(name);
	if (peel_buffer)
		peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	//2d adjustment
	flvr::ShaderProgram* img_shader =
		glbin_img_shader_factory.shader(IMG_SHDR_DEPTH_TO_GRADIENT);
	if (img_shader)
	{
		if (!img_shader->valid())
		{
			img_shader->create();
		}
		img_shader->bind();
	}
	img_shader->setLocalParam(0, 1.0 / nx, 1.0 / ny, m_persp ? 2e10 : 1e6, 0.0);
	img_shader->setLocalParam(1,
		glbin_settings.m_shadow_dir_x,
		glbin_settings.m_shadow_dir_y, 0.0, 0.0);
	//2d adjustment

	DrawViewQuad();

	if (img_shader && img_shader->valid())
		img_shader->release();

	//
	//bind fbo for final composition
	BindRenderBuffer();
	glActiveTexture(GL_TEXTURE0);
	if (overlay_buffer)
	{
		overlay_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
		overlay_buffer->unprotect();
	}
	glEnable(GL_BLEND);
	glBlendFunc(GL_ZERO, GL_SRC_COLOR);
	glDisable(GL_DEPTH_TEST);

	//2d adjustment
	img_shader =
		glbin_img_shader_factory.shader(IMG_SHDR_GRADIENT_TO_SHADOW_MESH);
	if (img_shader)
	{
		if (!img_shader->valid())
			img_shader->create();
		img_shader->bind();
	}
	img_shader->setLocalParam(0, 1.0 / nx, 1.0 / ny, std::max(m_scale_factor, 1.0), 0.0);
	img_shader->setLocalParam(1, darkness, 0.0, 0.0, 0.0);
	glActiveTexture(GL_TEXTURE1);
	if (peel_buffer)
		peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
	glActiveTexture(GL_TEXTURE2);
	flvr::Framebuffer* final_buffer =
		glbin_framebuffer_manager.framebuffer("final");
	if (final_buffer)
		final_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
	//2d adjustment

	DrawViewQuad();

	if (img_shader && img_shader->valid())
	{
		img_shader->release();
	}
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
}

//get mesh shadow
bool RenderView::GetMeshShadow(double &val)
{
	for (int i = 0; i<(int)m_layer_list.size(); i++)
	{
		if (!m_layer_list[i])
			continue;
		if (m_layer_list[i]->IsA() == 3)
		{
			MeshData* md = (MeshData*)m_layer_list[i];
			if (md && md->GetDisp())
			{
				val = md->GetShadowIntensity();
				return md->GetShadowEnable();
			}
		}
		else if (m_layer_list[i]->IsA() == 6)
		{
			MeshGroup* group = (MeshGroup*)m_layer_list[i];
			if (group && group->GetDisp())
			{
				for (int j = 0; j<(int)group->GetMeshNum(); j++)
				{
					MeshData* md = group->GetMeshData(j);
					if (md && md->GetDisp())
					{
						val = md->GetShadowIntensity();
						return md->GetShadowEnable();
					}
				}
			}
		}
	}
	val = 0.0;
	return false;
}

void RenderView::DrawCircles(double cx, double cy,
	double r1, double r2, fluo::Color &color, glm::mat4 &matrix)
{
	flvr::ShaderProgram* shader =
		glbin_img_shader_factory.shader(IMG_SHDR_DRAW_GEOMETRY);
	if (shader)
	{
		if (!shader->valid())
			shader->create();
		shader->bind();
	}

	shader->setLocalParam(0, color.r(), color.g(), color.b(), 1.0);
	//apply translate first
	glm::mat4 mat0 = matrix * glm::translate(
			glm::mat4(1), glm::vec3(cx, cy, 0.0));
	shader->setLocalParamMatrix(0, glm::value_ptr(mat0));

	flvr::VertexArray* va_circles =
		glbin_vertex_array_manager.vertex_array(flvr::VA_Brush_Circles);
	if (va_circles)
	{
		//set parameters
		std::vector<std::pair<unsigned int, double>> params;
		params.push_back(std::pair<unsigned int, double>(0, r1));
		params.push_back(std::pair<unsigned int, double>(1, r2));
		params.push_back(std::pair<unsigned int, double>(2, 60.0));
		va_circles->set_param(params);
		va_circles->draw();
	}

	if (shader && shader->valid())
		shader->release();
}

//draw the brush shape
void RenderView::DrawBrush()
{
	double pressure = glbin_vol_selector.GetNormPress();

	if (PointInView(old_mouse_X, old_mouse_Y))
	{
		int nx, ny;
		GetRenderSize(nx, ny);
		float sx, sy;
		sx = 2.0 / nx;
		sy = 2.0 / ny;

		//draw the circles
		//set up the matrices
		glm::mat4 proj_mat;
		double cx, cy;
		if (glbin_vol_selector.GetBrushSizeData())
		{
			proj_mat = glm::ortho(float(m_ortho_left), float(m_ortho_right),
				float(m_ortho_top), float(m_ortho_bottom));
			cx = m_ortho_left + old_mouse_X * (m_ortho_right - m_ortho_left) / nx;
			cy = m_ortho_bottom + old_mouse_Y * (m_ortho_top - m_ortho_bottom) / ny;
		}
		else
		{
			proj_mat = glm::ortho(float(0), float(nx), float(0), float(ny));
			cx = old_mouse_X;
			cy = ny - old_mouse_Y;
		}

		//attributes
		glDisable(GL_DEPTH_TEST);

		int mode = glbin_vol_selector.GetMode();

		fluo::Color text_color = GetTextColor();

		double br1 = glbin_vol_selector.GetBrushSize1();
		double br2 = glbin_vol_selector.GetBrushSize2();

		if (mode == 1 ||
			mode == 2)
			DrawCircles(cx, cy, br1*pressure,
				br2*pressure, text_color, proj_mat);
		else if (mode == 8)
			DrawCircles(cx, cy, br1*pressure,
				-1.0, text_color, proj_mat);
		else if (mode == 3 ||
			mode == 4)
			DrawCircles(cx, cy, -1.0,
				br2*pressure, text_color, proj_mat);

		float cx2 = old_mouse_X;
		float cy2 = ny - old_mouse_Y;
		float px, py;
		px = cx2 - 7 - nx / 2.0;
		py = cy2 - 3 - ny / 2.0;
		std::wstring wstr;
		switch (mode)
		{
		case 1:
			wstr = L"S";
			break;
		case 2:
			wstr = L"+";
			break;
		case 3:
			wstr = L"-";
			break;
		case 4:
			wstr = L"*";
			break;
		}
		m_text_renderer->RenderText(wstr, text_color, px*sx, py*sy, sx, sy);

		glEnable(GL_DEPTH_TEST);

	}
}

//paint strokes on the paint fbo
void RenderView::PaintStroke()
{
	int nx, ny;
	GetRenderSize(nx, ny);

	double pressure = glbin_vol_selector.GetNormPress();

	//generate texture and buffer objects
	//painting fbo
	flvr::Framebuffer* paint_buffer =
		glbin_framebuffer_manager.framebuffer(
			flvr::FB_Render_RGBA, nx, ny, "paint brush");
	if (!paint_buffer)
		return;
	paint_buffer->bind();
	paint_buffer->protect();
	//clear if asked so
	if (m_clear_paint)
	{
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT);
		m_clear_paint = false;
	}
	else
	{
		//paint shader
		flvr::ShaderProgram* paint_shader =
			glbin_img_shader_factory.shader(IMG_SHDR_PAINT);
		if (paint_shader)
		{
			if (!paint_shader->valid())
				paint_shader->create();
			paint_shader->bind();
		}

		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendEquation(GL_MAX);

		double radius1 = glbin_vol_selector.GetBrushSize1();
		double radius2 = glbin_vol_selector.GetBrushSize2();
		double bspc = glbin_vol_selector.GetBrushSpacing();
		bool bs_data = glbin_vol_selector.GetBrushSizeData();
		double px = double(old_mouse_X - prv_mouse_X);
		double py = double(old_mouse_Y - prv_mouse_Y);
		double dist = sqrt(px*px + py*py);
		double step = radius1 * pressure * bspc;
		int repeat = std::round(dist / step);
		double spx = (double)prv_mouse_X;
		double spy = (double)prv_mouse_Y;
		if (repeat > 0)
		{
			px /= repeat;
			py /= repeat;
		}

		//set the width and height
		if (bs_data)
			paint_shader->setLocalParam(1, m_ortho_right - m_ortho_left,
				m_ortho_top - m_ortho_bottom, 0.0f, 0.0f);
		else
			paint_shader->setLocalParam(1, nx, ny, 0.0f, 0.0f);

		double x, y;
		double cx, cy;
		for (int i = 0; i <= repeat; i++)
		{
			x = spx + i*px;
			y = spy + i*py;
			if (bs_data)
			{
				cx = x * (m_ortho_right - m_ortho_left) / nx;
				cy = (ny - y) * (m_ortho_top - m_ortho_bottom) / ny;
			}
			else
			{
				cx = x;
				cy = double(ny) - y;
			}
			switch (glbin_vol_selector.GetMode())
			{
			case 3:
				radius1 = radius2;
				break;
			case 4:
				radius1 = 0.0;
				break;
			case 8:
				radius2 = radius1;
				break;
			default:
				break;
			}
			//send uniforms to paint shader
			paint_shader->setLocalParam(0, cx, cy,
				radius1*pressure,
				radius2*pressure);
			//draw a square
			DrawViewQuad();
		}

		//release paint shader
		if (paint_shader && paint_shader->valid())
			paint_shader->release();
	}

	//bind back the window frame buffer
	BindRenderBuffer();
	glBlendEquation(GL_FUNC_ADD);
	//RefreshGL(3);
}

//show the stroke buffer
void RenderView::DisplayStroke()
{
	//painting texture
	flvr::Framebuffer* paint_buffer =
		glbin_framebuffer_manager.framebuffer("paint brush");
	if (!paint_buffer)
		return;

	//draw the final buffer to the windows buffer
	glActiveTexture(GL_TEXTURE0);
	paint_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	flvr::ShaderProgram* img_shader =
		glbin_img_shader_factory.shader(IMG_SHADER_TEXTURE_LOOKUP);
	if (img_shader)
	{
		if (!img_shader->valid())
			img_shader->create();
		img_shader->bind();
	}
	DrawViewQuad();
	if (img_shader && img_shader->valid())
		img_shader->release();

	glBindTexture(GL_TEXTURE_2D, 0);
	glEnable(GL_DEPTH_TEST);
}

fluo::Quaternion RenderView::Trackball(double dx, double dy)
{
	fluo::Quaternion q;
	fluo::Vector a; /* Axis of rotation */
	double phi;  /* how much to rotate about axis */

	if (dx == 0.0 && dy == 0.0)
	{
		/* Zero rotation */
		return q;
	}

	a = fluo::Vector(-dy, dx, 0.0);
	phi = a.length() / 3.0;
	a.normalize();

	if (m_rot_lock && phi < 45.0)
	{
		/* Zero rotation */
		return q;
	}

	//rotate back to local
	fluo::Quaternion aq(a);
	aq = (-m_q) * aq * m_q;
	if (m_rot_lock)
	{
		//rotate back to basis
		aq = (m_zq)* aq * (-m_zq);
		a = fluo::Vector(aq.x, aq.y, aq.z);
		a.normalize();
		//snap to closest basis component
		double maxv = std::max(std::fabs(a.x()),
			std::max(std::fabs(a.y()), std::fabs(a.z())));
		if (std::fabs(maxv - std::fabs(a.x())) < EPS)
			a = fluo::Vector(a.x() < 0?-1:1, 0, 0);
		else if (std::fabs(maxv - std::fabs(a.y())) < EPS)
			a = fluo::Vector(0, a.y() < 0?-1:1, 0);
		else if (std::fabs(maxv - std::fabs(a.z())) < EPS)
			a = fluo::Vector(0, 0, a.z() < 0?-1:1);
		aq = fluo::Quaternion(a);
		//rotate again to restore
		aq = (-m_zq) * aq * m_zq;
		a = fluo::Vector(aq.x, aq.y, aq.z);
		a.normalize();
		//snap to 45 deg
		phi = std::round(phi / 45.0) * 45.0;
	}
	else
	{
		a = fluo::Vector(aq.x, aq.y, aq.z);
		a.normalize();
	}

	q = fluo::Quaternion(phi, a);
	q.Normalize();

	return q;
}

fluo::Quaternion RenderView::TrackballClip(int p1x, int p1y, int p2x, int p2y)
{
	fluo::Quaternion q;
	fluo::Vector a; /* Axis of rotation */
	double phi;  /* how much to rotate about axis */

	if (p1x == p2x && p1y == p2y)
	{
		/* Zero rotation */
		return q;
	}

	a = fluo::Vector(p2y - p1y, p2x - p1x, 0.0);
	phi = a.length() / 3.0;
	a.normalize();
	fluo::Quaternion q_a(a);
	//rotate back to local
	fluo::Quaternion q2;
	q2.FromEuler(-m_rotx, m_roty, m_rotz);
	q_a = (q2)* q_a * (-q2);
	q_a = (m_q_cl)* q_a * (-m_q_cl);
	a = fluo::Vector(q_a.x, q_a.y, q_a.z);
	a.normalize();

	q = fluo::Quaternion(phi, a);
	q.Normalize();

	return q;
}

void RenderView::Q2A()
{
	//view changed, re-sort bricks
	//SetSortBricks();
	fluo::Quaternion q = m_q * (-m_zq);
	q.ToEuler(m_rotx, m_roty, m_rotz);

	if (m_roty > 360.0)
		m_roty -= 360.0;
	if (m_roty < 0.0)
		m_roty += 360.0;
	if (m_rotx > 360.0)
		m_rotx -= 360.0;
	if (m_rotx < 0.0)
		m_rotx += 360.0;
	if (m_rotz > 360.0)
		m_rotz -= 360.0;
	if (m_rotz < 0.0)
		m_rotz += 360.0;

	if (m_clip_mode)
		UpdateClips();
}

void RenderView::A2Q()
{
	//view changed, re-sort bricks
	//SetSortBricks();

	m_q.FromEuler(m_rotx, m_roty, m_rotz);
	m_q = m_q * m_zq;

	if (m_clip_mode)
		UpdateClips();
}

//sort bricks after view changes
void RenderView::SetSortBricks()
{
	PopVolumeList();

	for (int i = 0; i<(int)m_vd_pop_list.size(); i++)
	{
		VolumeData* vd = m_vd_pop_list[i];
		if (vd && vd->GetTexture())
			vd->GetTexture()->set_sort_bricks();
	}
}

//pre-draw processings
void RenderView::PreDraw()
{
	//skip if not done with loop
	if (glbin_settings.m_mem_swap)
	{
		if (m_pre_draw)
			m_pre_draw = false;
		else
			return;
	}
}

void RenderView::PostDraw()
{
	//skip if not done with loop
	if (glbin_settings.m_mem_swap &&
		flvr::TextureRenderer::get_start_update_loop() &&
		!flvr::TextureRenderer::get_done_update_loop())
		return;

	//output animations
	if (m_capture && !m_cap_file.empty())
	{
		//capture
		int chann = glbin_settings.m_save_alpha ? 4 : 3;
		bool fp32 = glbin_settings.m_save_float;
		float dpi = glbin_settings.m_dpi;
		int x, y, w, h;
		void* image = 0;
		ReadPixels(chann, fp32, x, y, w, h, &image);

		TIFF *out = TIFFOpenW(m_cap_file.c_str(), "wb");
		if (!out)
			return;
		TIFFSetField(out, TIFFTAG_IMAGEWIDTH, w);
		TIFFSetField(out, TIFFTAG_IMAGELENGTH, h);
		TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, chann);
		if (fp32)
		{
			TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 32);
			TIFFSetField(out, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
		}
		else
		{
			TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 8);
			//TIFFSetField(out, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
		}
		TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
		TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
		TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
		if (glbin_settings.m_save_compress)
			TIFFSetField(out, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
		//dpi
		TIFFSetField(out, TIFFTAG_XRESOLUTION, dpi);
		TIFFSetField(out, TIFFTAG_YRESOLUTION, dpi);
		TIFFSetField(out, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);

		tsize_t linebytes = tsize_t(chann) * w * (fp32?4:1);
		void *buf = NULL;
		buf = _TIFFmalloc(linebytes);
		//TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(out, 0));
		for (uint32 row = 0; row < (uint32)h; row++)
		{
			if (fp32)
			{
				float* line = ((float*)image) + (h - row - 1) * chann * w;
				memcpy(buf, line, linebytes);
			}
			else
			{// check the index here, and figure out why not using h*linebytes
				unsigned char* line = ((unsigned char*)image) + (h - row - 1) * chann * w;
				memcpy(buf, line, linebytes);
			}
			if (TIFFWriteScanline(out, buf, row, 0) < 0)
				break;
		}
		TIFFClose(out);
		if (buf)
			_TIFFfree(buf);
		if (image)
			delete[]image;

		m_capture = false;
	}
}

void RenderView::ResetEnlarge()
{
	//skip if not done with loop
	if (glbin_settings.m_mem_swap &&
		flvr::TextureRenderer::get_start_update_loop() &&
		!flvr::TextureRenderer::get_done_update_loop())
		return;
	if (m_keep_enlarge)
		return;
	m_enlarge = false;
	glbin_text_tex_manager.SetEnlargeScale(1);
	if (m_enlarge)
		m_gl_size = Size2D(static_cast<int>(std::round(m_size.w() * m_enlarge_scale)),
			static_cast<int>(std::round(m_size.h() * m_enlarge_scale)));
	else
		m_gl_size = m_size;

	RefreshGL(19);
}

void RenderView::SetBrush(int mode, IdleState& state)
{
	//need to add somewhere else
	//m_prev_focus = FindFocus();
	//SetFocus();
	state.m_set_cur_focus = true;

	int ruler_type = glbin_ruler_handler.GetType();

	if (m_int_mode == 5 ||
		m_int_mode == 7)
	{
		m_int_mode = 7;
		if (ruler_type == 3)
			glbin_vol_selector.SetMode(8);
		else
			glbin_vol_selector.SetMode(1);
	}
	else if (m_int_mode == 8)
	{
		if (ruler_type == 3)
			glbin_vol_selector.SetMode(8);
		else
			glbin_vol_selector.SetMode(1);
	}
	else if (m_int_mode == 10)
	{
		glbin_vol_selector.SetMode(9);
	}
	else
	{
		m_int_mode = 2;
		glbin_vol_selector.SetMode(mode);
	}
}

bool RenderView::UpdateBrushState(IdleState& state)
{
	bool refresh = false;

	if (state.m_key_paint)
	{
		glbin_states.m_brush_mode_toolbar = 0;
		glbin_states.m_brush_mode_shortcut = 2;
		m_paint_display = true;
		m_draw_brush = true;
		SetBrush(2, state);
		refresh = true;
	}
	else if (state.m_key_erase)
	{
		glbin_states.m_brush_mode_toolbar = 0;
		glbin_states.m_brush_mode_shortcut = 3;
		m_paint_display = true;
		m_draw_brush = true;
		SetBrush(3, state);
		refresh = true;
	}
	else if (state.m_key_diff)
	{
		glbin_states.m_brush_mode_toolbar = 0;
		glbin_states.m_brush_mode_shortcut = 4;
		m_paint_display = true;
		m_draw_brush = true;
		SetBrush(4, state);
		refresh = true;
	}
	else
	{
		if (glbin_states.m_brush_mode_toolbar)
		{
			m_paint_display = true;
			m_draw_brush = true;
		}
		else if (!state.m_key_paint &&
			!state.m_key_erase &&
			!state.m_key_diff &&
			glbin_states.m_brush_mode_shortcut)
		{
			if (state.m_mouse_left)
			{
				//wxPoint mps = ScreenToClient(wxGetMousePosition());
				glbin_vol_selector.Segment(true, true, m_mouse_x, m_mouse_y);
			}

			if (m_int_mode == 7)
				m_int_mode = 5;
			else
				m_int_mode = 1;

			glbin_states.m_brush_mode_toolbar = 0;
			glbin_states.m_brush_mode_shortcut = 0;
			m_paint_display = false;
			m_draw_brush = false;
			SetBrush(0, state);
			refresh = true;

			//need to move somewhere else
			//if (m_prev_focus)
			//{
			//	m_prev_focus->SetFocus();
			//	m_prev_focus = 0;
			//}
			state.m_set_previous_focus = true;
		}
	}

	return refresh;
}

//selection
void RenderView::Pick(BaseState& state)
{
	if (!m_draw_all)
		return;

	bool vol_sel = PickVolume(state);
	bool mesh_sel = PickMesh(state);
	fluo::ValueCollection vc = { gstCurrentSelect };
	if (vol_sel)
		vc.insert(gstCompListSelection);
	else if (!mesh_sel)
		glbin_current.SetMeshData(0);
	glbin_current.mainframe->UpdateProps(vc);
}

bool RenderView::PickMesh(BaseState& state)
{
	int i;
	int nx = m_gl_size.w();
	int ny = m_gl_size.h();
	if (nx <= 0 || ny <= 0)
		return false;
	if (m_mouse_x<0 || m_mouse_x >= nx ||
		m_mouse_y <= 0 || m_mouse_y>ny)
		return false;

	//projection
	HandleProjection(nx, ny);
	//Transformation
	HandleCamera();
	//obj
	glm::mat4 mv_temp = m_mv_mat;
	m_mv_mat = GetDrawMat();

	//set up fbo
	m_cur_framebuffer = 0;
	//bind
	flvr::Framebuffer* pick_buffer =
		glbin_framebuffer_manager.framebuffer(
			flvr::FB_Pick_Int32_Float, nx, ny);
	if (pick_buffer)
		pick_buffer->bind();

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glScissor(m_mouse_x, ny - m_mouse_y, 1, 1);
	glEnable(GL_SCISSOR_TEST);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	for (i = 0; i<(int)m_md_pop_list.size(); i++)
	{
		MeshData* md = m_md_pop_list[i];
		if (md)
		{
			md->SetMatrices(m_mv_mat, m_proj_mat);
			md->DrawInt(i + 1);
		}
	}
	glDisable(GL_SCISSOR_TEST);

	size_t choose = 0;
	if (pick_buffer)
		choose = pick_buffer->read_value(m_mouse_x, ny - m_mouse_y);
	glBindFramebuffer(GL_FRAMEBUFFER, m_cur_framebuffer);

	bool selected = false;
	if (choose >0 && choose <= (int)m_md_pop_list.size())
	{
		MeshData* md = m_md_pop_list[choose - 1];
		if (md)
		{
			glbin_current.SetMeshData(md);
			RefreshGL(27);
			selected = true;
		}
	}
	//else
	//{
	//	glbin_current.SetMeshData(0);
	//}
	m_mv_mat = mv_temp;

	return selected;
}

bool RenderView::PickVolume(BaseState& state)
{
	int kmode = state.m_key_ctrl ? 1 : 0;
	double dist = 0.0;
	double min_dist = -1.0;
	fluo::Point p, ip, pp;
	VolumeData* vd = 0;
	VolumeData* picked_vd = 0;
	for (int i = 0; i<(int)m_vd_pop_list.size(); i++)
	{
		vd = m_vd_pop_list[i];
		if (!vd) continue;
		int mode = 2;
		if (vd->GetMode() == 1) mode = 1;
		glbin_volume_point.SetVolumeData(vd);
		dist = glbin_volume_point.GetPointVolume(old_mouse_X, old_mouse_Y,
			mode, true, 0.5, p, ip);
		if (dist > 0.0)
		{
			if (min_dist < 0.0)
			{
				min_dist = dist;
				picked_vd = vd;
				pp = p;
			}
			else
			{
				if (m_persp)
				{
					if (dist < min_dist)
					{
						min_dist = dist;
						picked_vd = vd;
						pp = p;
					}
				}
				else
				{
					if (dist > min_dist)
					{
						min_dist = dist;
						picked_vd = vd;
						pp = p;
					}
				}
			}
		}
	}

	//update label selection
	SetCompSelection(ip, kmode);

	if (picked_vd)
	{
		glbin_current.SetVolumeData(picked_vd);

		if (m_pick_lock_center)
			m_lock_center = pp;
	}

	return picked_vd != 0;
}

void RenderView::SetCompSelection(fluo::Point& p, int mode)
{
	//update selection
	std::set<unsigned long long> ids;
	glbin_comp_analyzer.GetCompsPoint(p, ids);
	glbin_comp_selector.SetSelectedCompIds(ids, mode);
	//m_frame->UpdateProps({ gstCompListSelection });
}

void RenderView::DrawViewQuad()
{
	flvr::VertexArray* quad_va =
		glbin_vertex_array_manager.vertex_array(flvr::VA_Norm_Square);
	if (quad_va)
		quad_va->draw();
}

//find crop frame
int RenderView::HitCropFrame(fluo::Point& mp)
{
	int ny = m_gl_size.h();
	fluo::Point p(mp);
	p.y(ny - p.y());

	double x, y, w, h;
	x = glbin_moviemaker.GetCropX();
	y = glbin_moviemaker.GetCropY();
	w = glbin_moviemaker.GetCropW();
	h = glbin_moviemaker.GetCropH();
	double tol = 10;
	fluo::BBox box;
	fluo::Point p1, p2;
	//check corners
	//bottom left
	p1 = fluo::Point(x, y, 0);
	p1 = p1 - fluo::Vector(tol, tol, 1);
	p2 = p1 + 2 * fluo::Vector(tol, tol, 1);
	box = fluo::BBox(p1, p2);
	if (box.inside(p))
		return 1;
	//bottom right
	p1 = fluo::Point(x + w, y, 0);
	p1 = p1 - fluo::Vector(tol, tol, 1);
	p2 = p1 + 2 * fluo::Vector(tol, tol, 1);
	box = fluo::BBox(p1, p2);
	if (box.inside(p))
		return 2;
	//top left
	p1 = fluo::Point(x, y + h, 0);
	p1 = p1 - fluo::Vector(tol, tol, 1);
	p2 = p1 + 2 * fluo::Vector(tol, tol, 1);
	box = fluo::BBox(p1, p2);
	if (box.inside(p))
		return 3;
	//top right
	p1 = fluo::Point(x + w, y + h, 0);
	p1 = p1 - fluo::Vector(tol, tol, 1);
	p2 = p1 + 2 * fluo::Vector(tol, tol, 1);
	box = fluo::BBox(p1, p2);
	if (box.inside(p))
		return 4;
	//bottom
	p1 = fluo::Point(x + w / 2, y, 0);
	p1 = p1 - fluo::Vector(w / 2, tol, 1);
	p2 = p1 + 2 * fluo::Vector(w / 2, tol, 1);
	box = fluo::BBox(p1, p2);
	if (box.inside(p))
		return 5;
	//left
	p1 = fluo::Point(x, y + h / 2, 0);
	p1 = p1 - fluo::Vector(tol, h / 2, 1);
	p2 = p1 + 2 * fluo::Vector(tol, h / 2, 1);
	box = fluo::BBox(p1, p2);
	if (box.inside(p))
		return 6;
	//right
	p1 = fluo::Point(x + w, y + h / 2, 0);
	p1 = p1 - fluo::Vector(tol, h / 2, 1);
	p2 = p1 + 2 * fluo::Vector(tol, h / 2, 1);
	box = fluo::BBox(p1, p2);
	if (box.inside(p))
		return 7;
	//top
	p1 = fluo::Point(x + w / 2, y + h, 0);
	p1 = p1 - fluo::Vector(w / 2, tol, 1);
	p2 = p1 + 2 * fluo::Vector(w / 2, tol, 1);
	box = fluo::BBox(p1, p2);
	if (box.inside(p))
		return 8;

	return 0;
}

void RenderView::ChangeCropFrame(fluo::Point& mp)
{
	if (!m_crop_type)
		return;

	int ny = m_gl_size.h();
	fluo::Point p(mp);
	p.y(ny - p.y());

	double x, y, w, h;
	x = glbin_moviemaker.GetCropX();
	y = glbin_moviemaker.GetCropY();
	w = glbin_moviemaker.GetCropW();
	h = glbin_moviemaker.GetCropH();
	fluo::Point p1(x, y, 0);
	fluo::Point p2(x + w, y, 0);
	fluo::Point p3(x, y + h, 0);
	fluo::Point p4(x + w, y + h, 0);

	switch (m_crop_type)
	{
	case 1:
		//bottom left
		p1 = p;
		p2.y(p.y());
		p3.x(p.x());
		break;
	case 2:
		//bottom right
		p1.y(p.y());
		p2 = p;
		p4.x(p.x());
		break;
	case 3:
		//top left
		p1.x(p.x());
		p3 = p;
		p4.y(p.y());
		break;
	case 4:
		//top right
		p2.x(p.x());
		p3.y(p.y());
		p4 = p;
		break;
	case 5:
		//bottom
		p1.y(p.y());
		p2.y(p.y());
		break;
	case 6:
		//left
		p1.x(p.x());
		p3.x(p.x());
		break;
	case 7:
		//right
		p2.x(p.x());
		p4.x(p.x());
		break;
	case 8:
		//top
		p3.y(p.y());
		p4.y(p.y());
		break;
	}

	x = p1.x();
	y = p1.y();
	w = (p2 - p1).x();
	h = (p3 - p1).y();

	glbin_moviemaker.SetCropX(std::round(x));
	glbin_moviemaker.SetCropY(std::round(y));
	glbin_moviemaker.SetCropW(std::round(w));
	glbin_moviemaker.SetCropH(std::round(h));
}

void RenderView::switchLevel(VolumeData *vd)
{
	if (!vd) return;

	int nx, ny;
	GetRenderSize(nx, ny);
	if (m_enlarge)
	{
		nx = std::round(nx * m_enlarge_scale);
		ny = std::round(ny * m_enlarge_scale);
	}

	flvr::Texture *vtex = vd->GetTexture();
	if (vtex && vtex->isBrxml())
	{
		int prev_lv = vd->GetLevel();
		int new_lv = 0;

		if (m_res_mode > 0)
		{
			double res_scale = 1.0;
			switch (m_res_mode)
			{
			case 1:
				res_scale = 1;
				break;
			case 2:
				res_scale = 1.5;
				break;
			case 3:
				res_scale = 2.0;
				break;
			case 4:
				res_scale = 3.0;
				break;
			default:
				res_scale = 1.0;
			}
			std::vector<double> sfs;
			std::vector<double> spx, spy, spz;
			int lvnum = vtex->GetLevelNum();
			for (int i = 0; i < lvnum; i++)
			{
				double aspect = (double)nx / (double)ny;

				double spc_x;
				double spc_y;
				double spc_z;
				vtex->get_spacings(spc_x, spc_y, spc_z, i);
				spc_x = spc_x<EPS ? 1.0 : spc_x;
				spc_y = spc_y<EPS ? 1.0 : spc_y;

				spx.push_back(spc_x);
				spy.push_back(spc_y);
				spz.push_back(spc_z);

				double sf = 2.0*m_radius*res_scale / spc_y / double(ny);
				sfs.push_back(sf);
			}

			int lv = lvnum - 1;
			//if (!m_manip)
			{
				for (int i = lvnum - 1; i >= 0; i--)
				{
					if (m_scale_factor / 5 > (/*m_interactive ? sfs[i] * 16.0 :*/ sfs[i])) lv = i - 1;
				}
			}
			//apply offset
			lv += glbin_settings.m_detail_level_offset;
			if (lv < 0) lv = 0;
			//if (m_interactive) lv += 1;
			if (lv >= lvnum) lv = lvnum - 1;
			new_lv = lv;
		}
		if (prev_lv != new_lv)
		{
			std::vector<flvr::TextureBrick*> *bricks = vtex->get_bricks();
			if (bricks)
			{
				for (int i = 0; i < bricks->size(); i++)
					(*bricks)[i]->set_disp(false);
			}
			vd->SetLevel(new_lv);
			vtex->set_sort_bricks();
		}
	}
}

//controller interactions
void RenderView::ControllerMoveHorizontal(double dval, int nx, int ny)
{
	m_head = fluo::Vector(-m_transx, -m_transy, -m_transz);
	m_head.normalize();
	fluo::Vector side = fluo::Cross(m_up, m_head);
	fluo::Vector trans = side * (dval*(m_ortho_right - m_ortho_left) / double(nx));
	m_obj_transx += trans.x();
	m_obj_transy += trans.y();
	m_obj_transz += trans.z();
}

void RenderView::ControllerZoomDolly(double dval, int nx, int ny)
{
	double delta = dval / (double)ny;
	if (m_scale_factor < 1e5 && m_scale_factor > 1e-3)
		m_scale_factor += m_scale_factor * delta;
	if (m_free)
	{
		fluo::Vector pos(m_transx, m_transy, m_transz);
		pos.normalize();
		fluo::Vector ctr(m_ctrx, m_ctry, m_ctrz);
		ctr -= delta * pos * 1000;
		m_ctrx = ctr.x();
		m_ctry = ctr.y();
		m_ctrz = ctr.z();
	}
}

void RenderView::ControllerRotate(double dx, double dy, int nx, int ny)
{
	fluo::Quaternion q_delta = Trackball(dx, dy);
	m_q *= q_delta;
	m_q.Normalize();
	fluo::Quaternion cam_pos(0.0, 0.0, m_distance, 0.0);
	fluo::Quaternion cam_pos2 = (-m_q) * cam_pos * m_q;
	m_transx = cam_pos2.x;
	m_transy = cam_pos2.y;
	m_transz = cam_pos2.z;
	fluo::Quaternion up(0.0, 1.0, 0.0, 0.0);
	fluo::Quaternion up2 = (-m_q) * up * m_q;
	m_up = fluo::Vector(up2.x, up2.y, up2.z);
	m_q.ToEuler(m_rotx, m_roty, m_rotz);
	if (m_roty > 360.0)
		m_roty -= 360.0;
	if (m_roty < 0.0)
		m_roty += 360.0;
	if (m_rotx > 360.0)
		m_rotx -= 360.0;
	if (m_rotx < 0.0)
		m_rotx += 360.0;
	if (m_rotz > 360.0)
		m_rotz -= 360.0;
	if (m_rotz < 0.0)
		m_rotz += 360.0;
}

void RenderView::ControllerPan(double dx, double dy, int nx, int ny)
{
	m_head = fluo::Vector(-m_transx, -m_transy, -m_transz);
	m_head.normalize();
	fluo::Vector side = fluo::Cross(m_up, m_head);
	fluo::Vector trans =
		side * (dx * (m_ortho_right - m_ortho_left) / double(nx)) +
		m_up * (dy * (m_ortho_top - m_ortho_bottom) / double(ny));
	m_obj_transx += trans.x() * m_scale_factor;
	m_obj_transy += trans.y() * m_scale_factor;
	m_obj_transz += trans.z() * m_scale_factor;
}

void RenderView::GrabRotate(const glm::mat4& pose)
{
	// Extract the rotation matrix (upper-left 3x3 part of the 4x4 matrix)
	glm::mat3 rotationMatrix = glm::mat3(pose);
	// Convert rotation matrix to quaternion
	glm::quat q = glm::quat_cast(rotationMatrix);
	//glm::quat rotationQuat = glm::angleAxis(glm::radians(-50.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	//q = rotationQuat * q;

	m_q = fluo::Quaternion(q.x, q.y, q.z, q.w);
	m_q.Normalize();
	fluo::Quaternion cam_pos(0.0, 0.0, m_distance, 0.0);
	fluo::Quaternion cam_pos2 = (-m_q) * cam_pos * m_q;
	m_transx = cam_pos2.x;
	m_transy = cam_pos2.y;
	m_transz = cam_pos2.z;
	fluo::Quaternion up(0.0, 1.0, 0.0, 0.0);
	fluo::Quaternion up2 = (-m_q) * up * m_q;
	m_up = fluo::Vector(up2.x, up2.y, up2.z);
	m_q.ToEuler(m_rotx, m_roty, m_rotz);
	if (m_roty > 360.0)
		m_roty -= 360.0;
	if (m_roty < 0.0)
		m_roty += 360.0;
	if (m_rotx > 360.0)
		m_rotx -= 360.0;
	if (m_rotx < 0.0)
		m_rotx += 360.0;
	if (m_rotz > 360.0)
		m_rotz -= 360.0;
	if (m_rotz < 0.0)
		m_rotz += 360.0;
}

void RenderView::ProcessIdle(IdleState& state)
{
	state.m_start_loop = true;
	m_retain_finalbuffer = false;

	if (glbin_settings.m_hologram_mode == 2)
	{
		//make sure all views are drawn for the quilt
		if (glbin_lg_renderer.GetFinished())
			state.m_request_more = false;
		else
		{
			state.m_request_more = true;
			state.m_refresh = true;
			state.m_looking_glass_changed = false;
		}
	}

	//check memory swap status
	if (glbin_settings.m_mem_swap &&
		flvr::TextureRenderer::get_start_update_loop() &&
		!flvr::TextureRenderer::get_done_update_loop())
	{
		if (flvr::TextureRenderer::active_view_ == m_id)
		{
			state.m_refresh = true;
			state.m_start_loop = false;
			state.m_looking_glass_changed = true;
		}
	}
	else
	{
		if (state.m_movie_maker_render_canvas)
		{
			//DBGPRINT(L"lg finished: %d, cur view: %d\n",
			//	glbin_lg_renderer.GetFinished(),
			//	glbin_lg_renderer.GetCurView());
			if (glbin_lg_renderer.GetFinished())
			{
				state.m_refresh = glbin_moviemaker.Action();
				state.m_request_more = glbin_moviemaker.IsRunning();
				if (state.m_refresh)
				{
					state.m_looking_glass_changed = true;
					state.m_value_collection.insert(gstCamRotation);
					if (!glbin_moviemaker.IsRunning())
						state.m_value_collection.insert(gstMovPlay);
				}
			}
		}
	}

	if (m_capture_rotat ||
		m_capture_tsequ ||
		m_capture_param ||
		glbin_settings.m_test_speed)
	{
		state.m_refresh = true;
		state.m_looking_glass_changed = true;
		if (glbin_settings.m_mem_swap &&
			flvr::TextureRenderer::get_done_update_loop())
			m_pre_draw = true;
	}

	if (m_use_openxr)
	{
		state.m_request_more = true;
		state.m_refresh = true;
		state.m_looking_glass_changed = true;
		//m_retain_finalbuffer = true;
	}

	if (glbin_states.m_benchmark)
	{
		state.m_benchmark_fps = 1.0 / glbin.getStopWatch(gstStopWatch)->average();
		state.m_refresh = true;
		state.m_looking_glass_changed = true;
		if (glbin_settings.m_mem_swap &&
			flvr::TextureRenderer::get_done_update_loop())
			m_pre_draw = true;
	}

	//update pin rotation center
	if (m_update_rot_ctr && m_cur_vol && !m_free)
	{
		fluo::Point p, ip;
		int nx = GetGLSize().w();
		int ny = GetGLSize().h();
		int mode = 2;
		if (m_cur_vol->GetMode() == 1) mode = 1;
		glbin_volume_point.SetVolumeData(m_cur_vol);
		double dist = glbin_volume_point.GetPointVolume(nx / 2.0, ny / 2.0,
			mode, true, m_pin_pick_thresh, p, ip);
		if (dist <= 0.0)
			dist = glbin_volume_point.GetPointVolumeBox(
				nx / 2.0, ny / 2.0,
				true, p);
		if (dist > 0.0)
		{
			m_pin_ctr = p;
			double obj_transx, obj_transy, obj_transz;
			p = fluo::Point(m_obj_ctrx + m_obj_ctr_offx - p.x(),
				p.y() - m_obj_ctry - m_obj_ctr_offy,
				p.z() - m_obj_ctrz - m_obj_ctr_offz);
			obj_transx = p.x();
			obj_transy = p.y();
			obj_transz = p.z();
			double thresh = 10.0;
			double spcx, spcy, spcz;
			m_cur_vol->GetSpacings(spcx, spcy, spcz);
			thresh *= spcx;
			if (sqrt((m_obj_transx - obj_transx)*(m_obj_transx - obj_transx) +
				(m_obj_transy - obj_transy)*(m_obj_transy - obj_transy) +
				(m_obj_transz - obj_transz)*(m_obj_transz - obj_transz)) > thresh)
			{
				m_obj_transx = obj_transx;
				m_obj_transy = obj_transy;
				m_obj_transz = obj_transz;
			}
		}
		m_update_rot_ctr = false;
		state.m_refresh = true;
		state.m_looking_glass_changed = true;
	}

	if (state.m_mouse_over)
	{
		//forced refresh
		if (state.m_key_refresh)
		{
			state.m_set_focus = true;
			m_clear_buffer = true;
			m_updating = true;
			glbin_states.m_status_str = "Forced Refresh";
			glbin_current.mainframe->FluoUpdate({ gstMainStatusbarPush });
			//wxSizeEvent e;
			//OnResize(e);
			RefreshGL(14, false, true, true);
			glbin_current.mainframe->FluoUpdate({ gstMainStatusbarPop });
			return;
		}

		if (UpdateBrushState(state))
		{
			state.m_set_focus = true;
			state.m_refresh = true;
			state.m_value_collection.insert({ gstSelUndo, gstBrushState, gstBrushThreshold, gstBrushSize1, gstBrushSize2 });
			if (glbin_brush_def.m_update_size)
				state.m_value_collection.insert(gstBrushCountResult);
			if (glbin_brush_def.m_update_colocal)
				state.m_value_collection.insert(gstColocalResult);
		}

		//draw_mask
		if (state.m_key_mask && m_draw_mask)
		{
			m_draw_mask = false;
			state.m_refresh = true;
			state.m_looking_glass_changed = true;
			state.m_set_focus = true;
		}
		if (!state.m_key_mask && !m_draw_mask)
		{
			m_draw_mask = true;
			state.m_refresh = true;
			state.m_looking_glass_changed = true;
		}

		//move view
		//left
		if (!m_move_left &&
			state.m_key_ctrl &&
			state.m_key_left)
		{
			m_move_left = true;

			m_head = fluo::Vector(-m_transx, -m_transy, -m_transz);
			m_head.normalize();
			fluo::Vector side = fluo::Cross(m_up, m_head);
			fluo::Vector trans = -(side * (std::round(0.8 * (m_ortho_right - m_ortho_left))));
			m_obj_transx += trans.x();
			m_obj_transy += trans.y();
			m_obj_transz += trans.z();
			//if (m_persp) SetSortBricks();
			state.m_refresh = true;
			state.m_looking_glass_changed = true;
			state.m_set_focus = true;
		}
		if (m_move_left &&
			(!state.m_key_ctrl ||
				!state.m_key_left))
			m_move_left = false;
		//right
		if (!m_move_right &&
			state.m_key_ctrl &&
			state.m_key_right)
		{
			m_move_right = true;

			m_head = fluo::Vector(-m_transx, -m_transy, -m_transz);
			m_head.normalize();
			fluo::Vector side = fluo::Cross(m_up, m_head);
			fluo::Vector trans = side * (std::round(0.8 * (m_ortho_right - m_ortho_left)));
			m_obj_transx += trans.x();
			m_obj_transy += trans.y();
			m_obj_transz += trans.z();
			//if (m_persp) SetSortBricks();
			state.m_refresh = true;
			state.m_looking_glass_changed = true;
			state.m_set_focus = true;
		}
		if (m_move_right &&
			(!state.m_key_ctrl ||
				!state.m_key_right))
			m_move_right = false;
		//up
		if (!m_move_up &&
			state.m_key_ctrl &&
			state.m_key_up)
		{
			m_move_up = true;

			m_head = fluo::Vector(-m_transx, -m_transy, -m_transz);
			m_head.normalize();
			fluo::Vector trans = -m_up * (std::round(0.8 * (m_ortho_top - m_ortho_bottom)));
			m_obj_transx += trans.x();
			m_obj_transy += trans.y();
			m_obj_transz += trans.z();
			//if (m_persp) SetSortBricks();
			state.m_refresh = true;
			state.m_looking_glass_changed = true;
			state.m_set_focus = true;
		}
		if (m_move_up &&
			(!state.m_key_ctrl ||
				!state.m_key_up))
			m_move_up = false;
		//down
		if (!m_move_down &&
			state.m_key_ctrl &&
			state.m_key_down)
		{
			m_move_down = true;

			m_head = fluo::Vector(-m_transx, -m_transy, -m_transz);
			m_head.normalize();
			fluo::Vector trans = m_up * (std::round(0.8 * (m_ortho_top - m_ortho_bottom)));
			m_obj_transx += trans.x();
			m_obj_transy += trans.y();
			m_obj_transz += trans.z();
			//if (m_persp) SetSortBricks();
			state.m_refresh = true;
			state.m_looking_glass_changed = true;
			state.m_set_focus = true;
		}
		if (m_move_down &&
			(!state.m_key_ctrl ||
				!state.m_key_down))
			m_move_down = false;

		//move time sequence
		//forward
		if ((!m_tseq_forward && state.m_key_mov_forward) ||
			(!state.m_key_ctrl && state.m_key_mov_play))
		{
			m_tseq_forward = true;
			int frame = glbin_moviemaker.GetCurrentFrame();
			frame++;
			glbin_moviemaker.SetCurrentFrame(frame);
			state.m_refresh = true;
			state.m_looking_glass_changed = true;
			state.m_set_focus = true;
			state.m_value_collection.insert({ gstMovProgSlider, gstCurrentFrame, gstMovCurTime, gstMovSeqNum, gstTrackList });
		}
		if (m_tseq_forward && !state.m_key_mov_forward)
			m_tseq_forward = false;
		//backforward
		if ((!m_tseq_backward && state.m_key_mov_backward) ||
			(state.m_key_ctrl && state.m_key_mov_play))
		{
			m_tseq_backward = true;
			int frame = glbin_moviemaker.GetCurrentFrame();
			frame--;
			glbin_moviemaker.SetCurrentFrame(frame);
			state.m_refresh = true;
			state.m_looking_glass_changed = true;
			state.m_set_focus = true;
			state.m_value_collection.insert({ gstMovProgSlider, gstCurrentFrame, gstMovCurTime, gstMovSeqNum, gstTrackList });
		}
		if (m_tseq_backward && state.m_key_mov_backward)
			m_tseq_backward = false;

		//move clip
		//up
		if (!m_clip_up && state.m_key_clip_up)
		{
			m_clip_up = true;
			if (glbin_current.mainframe->GetClipPlanPanel())
				glbin_current.mainframe->GetClipPlanPanel()->MoveLinkedClippingPlanes(-1);
			state.m_refresh = true;
			state.m_looking_glass_changed = true;
			state.m_set_focus = true;
			state.m_value_collection.insert({ gstClipX1, gstClipX2, gstClipY1, gstClipY2, gstClipZ1, gstClipZ2 });
		}
		if (m_clip_up && !state.m_key_clip_up)
			m_clip_up = false;
		//down
		if (!m_clip_down && state.m_key_clip_down)
		{
			m_clip_down = true;
			if (glbin_current.mainframe->GetClipPlanPanel())
				glbin_current.mainframe->GetClipPlanPanel()->MoveLinkedClippingPlanes(1);
			state.m_refresh = true;
			state.m_looking_glass_changed = true;
			state.m_set_focus = true;
			state.m_value_collection.insert({ gstClipX1, gstClipX2, gstClipY1, gstClipY2, gstClipZ1, gstClipZ2 });
		}
		if (m_clip_down && !state.m_key_clip_down)
			m_clip_down = false;

		//cell full
		if (!m_cell_full && state.m_key_cell_full)
		{
			m_cell_full = true;
			glbin_comp_selector.SelectFullComp();
			state.m_refresh = true;
			state.m_looking_glass_changed = true;
			state.m_set_focus = true;
			state.m_value_collection.insert({ gstTrackList, gstSelUndoRedo });
		}
		if (m_cell_full && !state.m_key_cell_full)
			m_cell_full = false;
		//cell link
		if (!m_cell_link && state.m_key_cell_link)
		{
			m_cell_link = true;
			glbin_trackmap_proc.LinkCells(false);
			state.m_refresh = true;
			state.m_looking_glass_changed = true;
			state.m_set_focus = true;
			state.m_value_collection.insert({ gstTrackList, gstSelUndoRedo });
		}
		if (m_cell_link && !state.m_key_cell_link)
			m_cell_link = false;
		//new cell id
		if (!m_cell_new_id && state.m_key_cell_new_id)
		{
			m_cell_new_id = true;
			glbin_comp_editor.NewId(false, true);
			state.m_refresh = true;
			state.m_looking_glass_changed = true;
			state.m_set_focus = true;
			state.m_value_collection.insert({ gstTrackList, gstSelUndoRedo });
		}
		if (m_cell_new_id && !state.m_key_cell_new_id)
			m_cell_new_id = false;
		//clear
		if (!m_clear_mask && state.m_key_cell_clear)
		{
			glbin_vol_selector.Clear();
			glbin_comp_selector.Clear();
			GetTraces(false);
			m_clear_mask = true;
			state.m_refresh = true;
			state.m_looking_glass_changed = true;
			state.m_set_focus = true;
			state.m_value_collection.insert({ gstTrackList, gstSelUndoRedo });
		}
		if (m_clear_mask && !state.m_key_cell_clear)
			m_clear_mask = false;
		//save all masks
		if (!m_save_mask && state.m_key_save_mask)
		{
			VolumeData* vd = glbin_current.vol_data;
			if (vd)
			{
				vd->SaveMask(true, vd->GetCurTime(), vd->GetCurChannel());
				vd->SaveLabel(true, vd->GetCurTime(), vd->GetCurChannel());
			}
			m_save_mask = true;
			state.m_set_focus = true;
		}
		if (m_save_mask && !state.m_key_save_mask)
			m_save_mask = false;
		//full screen
		if (state.m_key_exit_fullscreen)
		{
			state.m_key_exit_fullscreen = true;
			state.m_fullscreen = false;
		}
		if (state.m_key_fullscreen)
		{
			state.m_key_exit_fullscreen = false;
			state.m_fullscreen = true;
		}
		//brush size
		if (state.m_key_brush_size_down)
		{
			ChangeBrushSize(-10, state.m_key_ctrl);
			state.m_set_focus = true;
		}
		if (state.m_key_brush_size_up)
		{
			ChangeBrushSize(10, state.m_key_ctrl);
			state.m_set_focus = true;
		}
		//comp include
		if (!m_comp_include && state.m_key_cell_include)
		{
			if (glbin_current.mainframe->GetComponentDlg())
				glbin_current.mainframe->GetComponentDlg()->IncludeComps();
			m_comp_include = true;
			state.m_refresh = true;
			state.m_looking_glass_changed = true;
			state.m_set_focus = true;
		}
		if (m_comp_include && !state.m_key_cell_include)
			m_comp_include = false;
		//comp exclude
		if (!m_comp_exclude && state.m_key_cell_exclude)
		{
			if (glbin_current.mainframe->GetComponentDlg())
				glbin_current.mainframe->GetComponentDlg()->ExcludeComps();
			m_comp_exclude = true;
			state.m_refresh = true;
			state.m_looking_glass_changed = true;
			state.m_set_focus = true;
		}
		if (m_comp_exclude && !state.m_key_cell_exclude)
			m_comp_exclude = false;
		//ruler relax
		if (!m_ruler_relax && state.m_key_ruler_relax)
		{
			if (glbin_current.mainframe->GetMeasureDlg())
				glbin_current.mainframe->GetMeasureDlg()->Relax();
			m_ruler_relax = true;
			state.m_refresh = true;
			state.m_looking_glass_changed = true;
			state.m_set_focus = true;
		}
		if (m_ruler_relax && !state.m_key_ruler_relax)
			m_ruler_relax = false;

		//grow
		if ((m_int_mode == 10 ||
			m_int_mode == 12) &&
			state.m_mouse_left &&
			m_grow_on)
		{
			int sz = glbin_settings.m_ruler_size_thresh;
			//event.RequestMore();
			glbin_vol_selector.SetInitMask(2);
			//mps = ScreenToClient(mps);
			glbin_vol_selector.Segment(false, true, m_mouse_x, m_mouse_y);
			glbin_vol_selector.SetInitMask(3);
			if (m_int_mode == 12)
			{
				glbin_seg_grow.SetVolumeData(m_cur_vol);
				glbin_seg_grow.SetIter(glbin_vol_selector.GetIter() * 3);
				glbin_seg_grow.SetSizeThresh(sz);
				glbin_seg_grow.Compute();
			}
			state.m_refresh = true;
			state.m_looking_glass_changed = true;
			state.m_start_loop = true;
			//update
			if (glbin_brush_def.m_update_size)
				state.m_value_collection.insert(gstBrushCountResult);
			if (glbin_brush_def.m_update_colocal)
				state.m_value_collection.insert(gstColocalResult);
			if (m_int_mode == 12)
				state.m_value_collection.insert(gstRulerList);
		}
	}

#if defined(_WIN32)
	//vr controller, similar to xinput
	if (m_use_openxr)
	{
		glbin_xr_renderer->GetControllerStates();
		float leftx, lefty, rightx, righty;
		leftx = glbin_xr_renderer->GetControllerLeftThumbstickX();
		lefty = glbin_xr_renderer->GetControllerLeftThumbstickY();

		int nx = GetGLSize().w();
		int ny = GetGLSize().h();
		//horizontal move
		if (leftx != 0.0)
		{
			state.m_request_more = true;
			ControllerMoveHorizontal(leftx, nx, ny);
			m_interactive = true;
			m_update_rot_ctr = true;
			state.m_refresh = true;
		}
		//zoom/dolly
		if (lefty != 0.0)
		{
			state.m_request_more = true;
			ControllerZoomDolly(lefty, nx, ny);
			m_interactive = true;
			state.m_refresh = true;
			state.m_value_collection.insert(gstScaleFactor);
		}

		if (glbin_xr_renderer->GetGrab())
		{
			state.m_request_more = true;
			glm::mat4 rot_mat = glbin_xr_renderer->GetGrabMatrix();
			GrabRotate(rot_mat);
			m_interactive = true;
			state.m_refresh = true;
			state.m_value_collection.insert(gstCamRotation);
		}
		else
		{
			rightx = glbin_xr_renderer->GetControllerRightThumbstickX();
			righty = glbin_xr_renderer->GetControllerRightThumbstickY();

			//rotate
			if (rightx != 0.0 || righty != 0.0)
			{
				state.m_request_more = true;
				ControllerRotate(rightx, righty, nx, ny);
				m_interactive = true;
				state.m_refresh = true;
				state.m_value_collection.insert(gstCamRotation);
			}
		}
	}
#endif

#if defined(_WIN32) && defined(USE_XINPUT)
	//xinput controller
	if (m_control_connected)
	{
		//DBGPRINT(L"Idle controller\n");
		XINPUT_STATE xstate = m_controller->GetState();
		double dzone = 0.2;
		double sclr = 20;
		double leftx = double(xstate.Gamepad.sThumbLX) / 32767.0;
		if (leftx > -dzone && leftx < dzone) leftx = 0.0;
		double lefty = double(xstate.Gamepad.sThumbLY) / 32767.0;
		if (lefty > -dzone && lefty < dzone) lefty = 0.0;
		double rghtx = double(xstate.Gamepad.sThumbRX) / 32767.0;
		if (rghtx > -dzone && rghtx < dzone) rghtx = 0.0;
		double rghty = double(xstate.Gamepad.sThumbRY) / 32767.0;
		if (rghty > -dzone && rghty < dzone) rghty = 0.0;
		int px = 0;
		int py = 0;
		int inc = 5;
		if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) py = -inc;
		if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) py = inc;
		if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) px = -inc;
		if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) px = inc;

		int nx = GetGLSize().w();
		int ny = GetGLSize().h();
		//horizontal move
		if (leftx != 0.0)
		{
			state.m_request_more = true;
			ControllerMoveHorizontal(leftx * sclr, nx, ny);
			m_interactive = true;
			m_update_rot_ctr = true;
			state.m_refresh = true;
			state.m_looking_glass_changed = true;
		}
		//zoom/dolly
		if (lefty != 0.0)
		{
			state.m_request_more = true;
			ControllerZoomDolly(lefty * sclr, nx, ny);
			m_interactive = true;
			state.m_refresh = true;
			state.m_looking_glass_changed = true;
			state.m_value_collection.insert(gstScaleFactor);
		}
		//rotate
		if (rghtx != 0.0 || rghty != 0.0)
		{
			state.m_request_more = true;
			ControllerRotate(rghtx * sclr, rghty * sclr, nx, ny);
			m_interactive = true;
			state.m_refresh = true;
			state.m_looking_glass_changed = true;
			state.m_value_collection.insert(gstCamRotation);
		}
		//pan
		if (px != 0 || py != 0)
		{
			state.m_request_more = true;
			ControllerPan(px, py, nx, ny);
			m_interactive = true;
			m_update_rot_ctr = true;
			state.m_refresh = true;
			state.m_looking_glass_changed = true;
		}
	}
#endif


	if (state.m_refresh)
	{
		m_clear_buffer = true;
		m_updating = true;
		RefreshGL(15, state.m_erase_background, state.m_start_loop, state.m_looking_glass_changed);
		if (state.m_value_collection.empty())
			state.m_value_collection.insert(gstNull);
	}
	else if (glbin_settings.m_inf_loop)
	{
		RefreshGL(0, false, true, true);
		state.m_request_more = true;
		return;
	}
}

void RenderView::ProcessMouse(MouseState& state)
{
	if (m_interactive && !m_rot_lock)
		return;

	m_interactive = false;
	m_paint_enable = false;
	m_retain_finalbuffer = false;
	int nx = m_gl_size.w();
	int ny = m_gl_size.h();
	fluo::Point mp(m_mouse_x, m_mouse_y, 0);
	mp *= m_dpi_factor;
	if (m_enlarge)
		mp *= m_enlarge_scale;

	//mouse button down operations
	//glbin_ruler_handler.SetVolumeData(m_cur_vol);
	if (state.m_mouse_left_down)
	{
		if (m_draw_frame)
		{
			m_crop_type = HitCropFrame(mp);
			if (m_crop_type)
			{
				m_int_mode = 16;
				return;
			}
		}

		state.m_reset_focus_slider = true;

		bool found_rp = false;
		if (m_int_mode == 6 ||
			m_int_mode == 9 ||
			m_int_mode == 11 ||
			m_int_mode == 14)
		{
			found_rp = glbin_ruler_handler.FindEditingRuler(
				mp.x(), mp.y());
		}
		if (found_rp)
		{
			if (m_int_mode == 11)
			{
				flrd::RulerPoint *p = glbin_ruler_handler.GetPoint();
				if (p) p->ToggleLocked();
			}
			if (m_int_mode == 14)
				glbin_ruler_handler.DeletePoint();
			RefreshGL(41);
			glbin_current.mainframe->UpdateProps({ gstRulerList });
		}

		if (m_int_mode == 1 ||
			((m_int_mode == 5 ||
			//m_int_mode == 13 ||
			m_int_mode == 15) &&
			state.m_key_alt) ||
			((m_int_mode == 6 ||
			m_int_mode == 9 ||
			m_int_mode == 11 ||
			m_int_mode == 14) &&
			!found_rp))
		{
			old_mouse_X = mp.x();
			old_mouse_Y = mp.y();
			m_pick = true;
		}
		else if (m_int_mode == 2 || m_int_mode == 7)
		{
			old_mouse_X = mp.x();
			old_mouse_Y = mp.y();
			prv_mouse_X = old_mouse_X;
			prv_mouse_Y = old_mouse_Y;
			m_paint_enable = true;
			m_clear_paint = true;
			glbin_vol_selector.SetBrushPressPeak(0.0);
			RefreshGL(26);
		}

		if (m_int_mode == 10 ||
			m_int_mode == 12)
		{
			glbin_vol_selector.ResetMousePos();
			glbin_vol_selector.SetInitMask(1);
			//wxPoint mps = ScreenToClient(wxGetMousePosition());
			glbin_vol_selector.Segment(true, true, m_mouse_x, m_mouse_y);
			glbin_vol_selector.SetInitMask(3);
			if (m_int_mode == 12)
				m_cur_vol->AddEmptyLabel(0, false);
			m_force_clear = true;
			m_grow_on = true;
		}

		if (m_int_mode == 13 &&
			!state.m_key_alt)
		{
			//add one point to a ruler
			glbin_ruler_handler.AddRulerPoint(mp.x(), mp.y(), 2);
			RefreshGL(26);
			glbin_current.mainframe->UpdateProps({ gstRulerList });
		}

		return;
	}
	if (state.m_mouse_right_down)
	{
		state.m_reset_focus_slider = true;
		old_mouse_X = mp.x();
		old_mouse_Y = mp.y();
		return;
	}
	if (state.m_mouse_middle_down)
	{
		state.m_reset_focus_slider = true;
		old_mouse_X = mp.x();
		old_mouse_Y = mp.y();
		return;
	}

	//mouse button up operations
	if (state.m_mouse_left_up)
	{
		if (m_int_mode == 1)
		{
			//pick stuff
			if (m_pick)
			{
				Pick(state);
				m_pick_lock_center = false;
				return;
			}
			else
			{
				//RefreshGL(27);
				return;
			}
		}
		else if (m_int_mode == 2)
		{
			//segment volumes
			m_paint_enable = true;
			//wxPoint mps = ScreenToClient(wxGetMousePosition());
			glbin_vol_selector.Segment(true, true, m_mouse_x, m_mouse_y);
			m_int_mode = 4;
			m_force_clear = true;
			RefreshGL(27);
			fluo::ValueCollection vc;
			vc.insert({ gstSelUndo, gstBrushThreshold });
			if (glbin_brush_def.m_update_size)
				vc.insert(gstBrushCountResult);
			if (glbin_brush_def.m_update_colocal)
				vc.insert(gstColocalResult);
			glbin_current.mainframe->UpdateProps(vc);
			return;
		}
		else if (m_int_mode == 5 &&
			!state.m_key_alt)
		{
			//add one point to a ruler
			glbin_ruler_handler.AddRulerPoint(mp.x(), mp.y(), 1);
			RefreshGL(27);
			glbin_current.mainframe->UpdateProps({ gstRulerList });
			return;
		}
		else if (m_int_mode == 9 ||
			m_int_mode == 11)
		{
			glbin_ruler_handler.SetPoint(0);
		}
		else if (m_int_mode == 7)
		{
			//segment volume, calculate center, add ruler point
			m_paint_enable = true;
			//wxPoint mps = ScreenToClient(wxGetMousePosition());
			glbin_vol_selector.Segment(true, true, m_mouse_x, m_mouse_y);
			if (glbin_ruler_handler.GetType() == 3)
				glbin_ruler_handler.AddRulerPoint(mp.x(), mp.y(), 0);
			else
				glbin_ruler_handler.AddPaintRulerPoint();
			m_int_mode = 8;
			m_force_clear = true;
			RefreshGL(27);
			fluo::ValueCollection vc;
			vc.insert({ gstRulerList, gstSelUndo, gstBrushThreshold });
			if (glbin_brush_def.m_update_size)
				vc.insert(gstBrushCountResult);
			if (glbin_brush_def.m_update_colocal)
				vc.insert(gstColocalResult);
			glbin_current.mainframe->UpdateProps(vc);
			return;
		}
		else if (m_int_mode == 10 ||
			m_int_mode == 12)
		{
			//glbin_vol_selector.PushMask();
			m_grow_on = false;
			return;
		}
		else if (m_int_mode == 13 &&
			!state.m_key_alt)
		{
			if (glbin_settings.m_ruler_auto_relax)
			{
				glbin_ruler_handler.SetEdited(true);
				glbin_ruler_handler.Relax();
			}
			RefreshGL(29);
			glbin_current.mainframe->UpdateProps({ gstRulerList });
			return;
		}
		else if ((m_int_mode == 6 ||
			m_int_mode == 15) &&
			!state.m_key_alt)
		{
			if (m_int_mode == 6)
			{
				glbin_ruler_handler.ClearMagStroke();
				glbin_ruler_handler.AddMagStrokePoint(mp.x(), mp.y());
			}
			else if (glbin_ruler_handler.MagStrokeEmpty())
				glbin_ruler_handler.AddMagStrokePoint(mp.x(), mp.y());
			glbin_ruler_handler.ApplyMagPoint();
			glbin_ruler_handler.ClearMagStroke();
			RefreshGL(29);
			glbin_current.mainframe->UpdateProps({ gstRulerList });
			return;
		}
		else if (m_int_mode == 16)
		{
			m_int_mode = 1;
			m_crop_type = 0;
		}
	}
	if (state.m_mouse_middle_up)
	{
		//SetSortBricks();
		//RefreshGL(28);
		return;
	}
	if (state.m_mouse_right_up)
	{
		if (m_int_mode == 1)
		{
			//RefreshGL(27);
			//return;
		}
		if (m_int_mode == 5 &&
			!state.m_key_alt)
		{
			if (glbin_ruler_handler.GetRulerFinished())
			{
				m_int_mode = 1;
				//SetIntMode(1);
				//glbin_ruler_handler.SetMode(0);
			}
			else
			{
				glbin_ruler_handler.AddRulerPoint(mp.x(), mp.y(), 1);
				glbin_ruler_handler.FinishRuler();
			}
			if (glbin_settings.m_ruler_auto_relax)
			{
				glbin_ruler_handler.SetEdited(true);
				glbin_ruler_handler.Relax();
			}
			RefreshGL(29);
			glbin_current.mainframe->UpdateProps({ gstRulerList });
			return;
		}
		//SetSortBricks();
	}

	//mouse dragging
	if (state.m_mouse_drag)
	{
		//crop
		if (m_int_mode == 16)
		{
			ChangeCropFrame(mp);
			RefreshGL(29);
			glbin_current.mainframe->UpdateProps({ gstCropValues });
			return;
		}

		flvr::TextureRenderer::set_cor_up_time(
			std::round(sqrt(double(old_mouse_X - mp.x())*
				double(old_mouse_X - mp.x()) +
				double(old_mouse_Y - mp.y())*
				double(old_mouse_Y - mp.y()))));

		flrd::RulerPoint *p0 = glbin_ruler_handler.GetPoint();
		bool hold_old = false;
		if (m_int_mode == 1 ||
			((m_int_mode == 5  ||
			m_int_mode == 13 ||
			m_int_mode == 15) &&
			state.m_key_alt) ||
			((m_int_mode == 9 ||
			m_int_mode == 10 ||
			m_int_mode == 11 ||
			m_int_mode == 12 ||
			m_int_mode == 14) &&
			!p0) ||
			((m_int_mode == 6 ||
			m_int_mode == 15 ||
			m_int_mode == 13) &&
			(state.m_key_ctrl ||
			state.m_mouse_middle_is_down ||
			state.m_mouse_right_is_down)))
		{
			//disable picking
			m_pick = false;

			if (old_mouse_X != -1 &&
				old_mouse_Y != -1 &&
				abs(old_mouse_X - mp.x()) +
				abs(old_mouse_Y - mp.y())<200)
			{
				if (state.m_mouse_left_is_down &&
					!state.m_key_ctrl &&
					m_int_mode != 10 &&
					m_int_mode != 12)
				{
					fluo::Quaternion q_delta = Trackball(
						mp.x() - old_mouse_X, old_mouse_Y - mp.y());
					if (m_rot_lock && q_delta.IsIdentity())
						hold_old = true;
					m_q *= q_delta;
					m_q.Normalize();
					fluo::Quaternion cam_pos(0.0, 0.0, m_distance, 0.0);
					fluo::Quaternion cam_pos2 = (-m_q) * cam_pos * m_q;
					m_transx = cam_pos2.x;
					m_transy = cam_pos2.y;
					m_transz = cam_pos2.z;

					fluo::Quaternion up(0.0, 1.0, 0.0, 0.0);
					fluo::Quaternion up2 = (-m_q) * up * m_q;
					m_up = fluo::Vector(up2.x, up2.y, up2.z);

					Q2A();

					m_interactive = true;

					if (glbin.get_linked_rot())
						glbin.set_master_linked_view(this);

					if (!hold_old)
						RefreshGL(30);
					//DBGPRINT(L"refresh requested\n");
					if (m_render_view_panel)
						m_render_view_panel->FluoUpdate({ gstCamRotation });
				}
				if (state.m_mouse_middle_is_down || (state.m_key_ctrl && state.m_mouse_left_is_down))
				{
					long dx = mp.x() - old_mouse_X;
					long dy = mp.y() - old_mouse_Y;

					m_head = fluo::Vector(-m_transx, -m_transy, -m_transz);
					m_head.normalize();
					fluo::Vector side = Cross(m_up, m_head);
					fluo::Vector trans = -(
						side*(double(dx)*(m_ortho_right - m_ortho_left) / double(nx)) +
						m_up*(double(dy)*(m_ortho_top - m_ortho_bottom) / double(ny)));
					m_obj_transx += trans.x();
					m_obj_transy += trans.y();
					m_obj_transz += trans.z();

					m_interactive = true;

					if (m_pin_rot_ctr)
						m_update_rot_ctr = true;

					//SetSortBricks();
					RefreshGL(31);
				}
				if (state.m_mouse_right_is_down)
				{
					long dx = mp.x() - old_mouse_X;
					long dy = mp.y() - old_mouse_Y;

					double delta = abs(dx)>abs(dy) ?
						(double)dx / (double)nx :
						(double)-dy / (double)ny;
					m_scale_factor += m_scale_factor*delta;

					if (m_free)
					{
						fluo::Vector pos(m_transx, m_transy, m_transz);
						pos.normalize();
						fluo::Vector ctr(m_ctrx, m_ctry, m_ctrz);
						ctr -= delta*pos * 1000;
						m_ctrx = ctr.x();
						m_ctry = ctr.y();
						m_ctrz = ctr.z();
					}

					m_interactive = true;

					//SetSortBricks();
					RefreshGL(32);

					if (m_render_view_panel)
						m_render_view_panel->FluoUpdate({ gstScaleFactor });
				}
			}
		}
		else if (m_int_mode == 2 || m_int_mode == 7)
		{
			m_paint_enable = true;
			RefreshGL(33);
		}
		else if (m_int_mode == 3)
		{
			if (old_mouse_X != -1 &&
				old_mouse_Y != -1 &&
				abs(old_mouse_X - mp.x()) +
				abs(old_mouse_Y - mp.y())<200)
			{
				if (state.m_mouse_left_is_down)
				{
					fluo::Quaternion q_delta = TrackballClip(old_mouse_X, mp.y(), mp.x(), old_mouse_Y);
					m_q_cl = q_delta * m_q_cl;
					m_q_cl.Normalize();
					SetRotations(fluo::Vector(m_rotx, m_roty, m_rotz), true);
					RefreshGL(34);
				}
			}
		}
		else if (m_int_mode == 6 || m_int_mode == 9)
		{
			bool rval = false;
			if (m_int_mode == 6)
				rval = glbin_ruler_handler.EditPoint(
					mp.x(), mp.y(), state.m_key_alt);
			else if (m_int_mode == 9)
				rval = glbin_ruler_handler.MoveRuler(
					mp.x(), mp.y());
			if (rval)
			{
				RefreshGL(35);
				glbin_current.mainframe->UpdateProps({ gstRulerList });
				glbin_ruler_handler.SetEdited(true);
			}
		}
		else if (m_int_mode == 13 && !state.m_key_alt)
		{
			double dist = glbin_settings.m_pencil_dist;
			if (glbin_ruler_handler.GetMouseDist(mp.x(), mp.y(), dist))
			{
				//add one point to a ruler
				glbin_ruler_handler.AddRulerPoint(mp.x(), mp.y(), 0);
				RefreshGL(27);
				glbin_current.mainframe->UpdateProps({ gstRulerList });
			}
		}
		else if (m_int_mode == 15 && !state.m_key_alt)
		{
			double dist = glbin_settings.m_pencil_dist;
			if (glbin_ruler_handler.GetMouseDist(mp.x(), mp.y(), dist))
				glbin_ruler_handler.AddMagStrokePoint(mp.x(), mp.y());
			glbin_ruler_handler.ApplyMagStroke();
			RefreshGL(27);
		}

		//update mouse position
		if (old_mouse_X >= 0 && old_mouse_Y >= 0)
		{
			prv_mouse_X = old_mouse_X;
			prv_mouse_Y = old_mouse_Y;
			if (!hold_old)
			{
				old_mouse_X = mp.x();
				old_mouse_Y = mp.y();
			}
		}
		else
		{
			old_mouse_X = mp.x();
			old_mouse_Y = mp.y();
			prv_mouse_X = old_mouse_X;
			prv_mouse_Y = old_mouse_Y;
		}

		return;
	}

	//wheel operations
	int wheel = state.m_mouse_wheel_rotate;
	if (wheel)  //if rotation
	{
		if (state.m_valid_focus_slider)
		{
			state.m_scroll_focus_slider = true;
		}
		else
		{
			if (m_int_mode == 2 || m_int_mode == 7)
			{
				ChangeBrushSize(wheel, state.m_key_ctrl);
			}
			else
			{
				m_interactive = true;
				if (m_pin_rot_ctr)
					m_update_rot_ctr = true;
				double value = wheel * m_scale_factor / 1000.0;
				if (m_scale_factor + value > 0.01)
					m_scale_factor += value;
				if (m_render_view_panel)
					m_render_view_panel->FluoUpdate({ gstScaleFactor });
			}
		}

		RefreshGL(36);
		return;
	}

	// draw the strokes into a framebuffer texture
	//not actually for displaying it
	if (m_draw_brush)
	{
		old_mouse_X = mp.x();
		old_mouse_Y = mp.y();
		RefreshGL(37);
		return;
	}

	if (m_draw_info & INFO_DISP)
	{
		if (glbin_moviemaker.IsRunning())
			return;
		if (glbin_settings.m_hologram_mode)
			return;

		m_retain_finalbuffer = true;
#ifdef _WIN32
		RefreshGL(38, false, false);
#else
		RefreshGL(38, false, true);
#endif
		return;
	}
}