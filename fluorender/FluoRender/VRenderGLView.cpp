/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2018 Scientific Computing and Imaging Institute,
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

#include "VRenderGLView.h"
#include "VRenderView.h"
#include "VRenderFrame.h"
#include <Global.h>
#include <Components/CompAnalyzer.h>
#include <Calculate/Count.h>
#include <Calculate/Histogram.h>
#include <Database/RecordHistParams.h>
#include <Distance/SegGrow.h>
#include <Distance/Cov.h>
#include <FLIVR/Framebuffer.h>
#include <FLIVR/VertexArray.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <wx/stdpaths.h>
#include <Debug.h>
#include <Timer.h>
#include "png_resource.h"
#include "img/icons.h"

bool VRenderGLView::m_linked_rot = false;
VRenderGLView* VRenderGLView::m_master_linked_view = 0;
bool VRenderGLView::m_keep_enlarge = false;
bool VRenderGLView::m_enlarge = false;
double VRenderGLView::m_enlarge_scale = 1.0;
#ifdef _WIN32
HCTX VRenderGLView::m_hTab = 0;
LOGCONTEXTA VRenderGLView::m_lc;
#endif

BEGIN_EVENT_TABLE(VRenderGLView, wxGLCanvas)
EVT_PAINT(VRenderGLView::OnDraw)
EVT_SIZE(VRenderGLView::OnResize)
EVT_MOUSE_EVENTS(VRenderGLView::OnMouse)
EVT_IDLE(VRenderGLView::OnIdle)
EVT_KEY_DOWN(VRenderGLView::OnKeyDown)
EVT_TIMER(ID_ftrigger, VRenderGLView::OnQuitFscreen)
EVT_CLOSE(VRenderGLView::OnClose)
END_EVENT_TABLE()

VRenderGLView::VRenderGLView(VRenderFrame* frame,
	VRenderView* parent,
	const wxGLAttributes& attriblist,
	wxGLContext* sharedContext,
	const wxPoint& pos,
	const wxSize& size,
	long style) :
	wxGLCanvas(parent, attriblist, wxID_ANY, pos, size, style),
	//public
	//set gl
	m_set_gl(false),
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
	m_test_speed(false),
	m_draw_clip(false),
	m_draw_legend(false),
	m_draw_colormap(false),
	m_mouse_focus(false),
	m_test_wiref(false),
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
	m_sb_x(0),
	m_sb_y(0),
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
	//mode in determining depth of volume
	m_point_volume_mode(0),
	//ruler use volume transfer function
	m_ruler_use_transf(false),
	//private
	m_frame(frame),
	m_vrv(parent),
	//populated lists of data
	m_vd_pop_dirty(true),
	m_md_pop_dirty(true),
	//traces
	m_trace_group(0),
	//multivolume
	m_mvr(0),
	//initializaion
	m_initialized(false),
	m_init_view(false),
	//bg color
	m_bg_color(0.0, 0.0, 0.0),
	m_bg_color_inv(1.0, 1.0, 1.0),
	m_grad_bg(false),
	//frustrum
	m_aov(15.0),
	m_near_clip(0.1),
	m_far_clip(100.0),
	//interpolation
	m_intp(true),
	//previous focus
	m_prev_focus(0),
	//interactive modes
	m_int_mode(1),
	m_force_clear(false),
	m_interactive(false),
	m_clear_buffer(false),
	m_adaptive(true),
	m_brush_state(0),
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
	m_draw_bounds(false),
	m_draw_all(true),
	m_draw_grid(false),
	m_draw_type(1),
	m_vol_method(VOL_METHOD_SEQ),
	m_peeling_layers(1),
	m_blend_slices(false),
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
	//movie frame properties
	m_frame_x(-1),
	m_frame_y(-1),
	m_frame_w(-1),
	m_frame_h(-1),
	//post image processing
	m_gamma(fluo::Color(1.0, 1.0, 1.0)),
	m_brightness(fluo::Color(1.135, 1.135, 1.135)),
	m_hdr(0.0, 0.0, 0.0),
	m_sync_r(false),
	m_sync_g(false),
	m_sync_b(false),
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
	//timer for fullscreen
	m_fullscreen_trigger(this, ID_ftrigger),
	//nodraw count
	m_nodraw_count(0),
	//pin rotation center
	m_auto_update_rot_center(true),
	m_pin_rot_center(false),
	m_rot_center_dirty(false),
	m_pin_pick_thresh(0.6),
	m_res_mode(1),
	m_enable_touch(false),
	m_ptr_id1(-1),
	m_ptr_id2(-1),
	m_full_screen(false),
	m_drawing(false),
	m_refresh(false),
	m_paint_count(false),
	m_paint_colocalize(false),
	m_ruler_autorelax(false),
	//vr settings
	m_enable_vr(false),
	m_use_openvr(false),
	m_vr_eye_offset(6.0),
	m_vr_eye_idx(0)
#if defined(_WIN32) && defined(USE_XINPUT)
	,
	m_controller(0)
#endif
{
	m_glRC = sharedContext;
	m_sharedRC = m_glRC ? true : false;

	m_sb_num = "50";
#ifdef _WIN32
	//tablet initialization
	if (m_selector.GetBrushUsePres())
	{
		if (!m_hTab && LoadWintab() &&
			gpWTInfoA(0, 0, NULL))
		{
			m_hTab = TabletInit((HWND)GetHWND(), (HINSTANCE)::wxGetInstance());
		}
		else
			m_selector.SetBrushUsePres(false);
	}

	//check touch
	HMODULE user32 = LoadLibrary(L"user32");
	GetPI = reinterpret_cast<decltype(GetPointerInfo)*>
		(GetProcAddress(user32, "GetPointerInfo"));
	if (GetPI != NULL)
		m_enable_touch = true;
	else
		m_enable_touch = false;

	//xbox controller
#ifdef USE_XINPUT
	m_controller = new XboxController(1);
#endif
#endif

	m_selector.LoadBrushSettings();

	m_timer = new Fltimer(10);
	if (m_frame && m_frame->GetBenchmark())
		m_benchmark = true;
	else
		m_benchmark = false;

	m_ruler_handler.SetView(this);
	m_ruler_handler.SetRulerList(&m_ruler_list);
	if (m_frame && m_frame->GetSettingDlg())
	{
		int type, kx, ky; double varth, gauth;
		m_frame->GetSettingDlg()->GetBgParams(type, kx, ky, varth, gauth);
		m_ruler_handler.SetBgParams(type, kx, ky, varth, gauth);
	}
	m_ruler_renderer.SetView(this);
	m_ruler_renderer.SetRulerList(&m_ruler_list);
	m_vp.SetView(this);
	m_selector.SetView(this);
	m_calculator.SetFrame(m_frame);
	m_calculator.SetView(this);
	m_calculator.SetVolumeSelector(&m_selector);
	m_scriptor.SetFrame(m_frame);
	m_scriptor.SetView(this);
}

#ifdef _WIN32
//tablet init
HCTX VRenderGLView::TabletInit(HWND hWnd, HINSTANCE hInst)
{
	HCTX hctx = NULL;
	UINT wDevice = 0;
	UINT wExtX = 0;
	UINT wExtY = 0;
	UINT wWTInfoRetVal = 0;
	AXIS TabletX = { 0 };
	AXIS TabletY = { 0 };
	AXIS TabletNPress = { 0 };
	AXIS TabletTPress = { 0 };

	// Set option to move system cursor before getting default system context.
	m_lc.lcOptions |= CXO_SYSTEM;

	// Open default system context so that we can get tablet data
	// in screen coordinates (not tablet coordinates).
	wWTInfoRetVal = gpWTInfoA(WTI_DEFSYSCTX, 0, &m_lc);
	WACOM_ASSERT(wWTInfoRetVal == sizeof(LOGCONTEXTA));

	WACOM_ASSERT(m_lc.lcOptions & CXO_SYSTEM);

	// modify the digitizing region
	sprintf(m_lc.lcName,
		"FluoRender Digitizing %llx",
		reinterpret_cast<unsigned long long>(hInst));

	// We process WT_PACKET (CXO_MESSAGES) messages.
	m_lc.lcOptions |= CXO_MESSAGES;

	// What data items we want to be included in the tablet packets
	m_lc.lcPktData = PACKETDATA;

	// Which packet items should show change in value since the last
	// packet (referred to as 'relative' data) and which items
	// should be 'absolute'.
	m_lc.lcPktMode = PACKETMODE;

	// This bitfield determines whether or not this context will receive
	// a packet when a value for each packet field changes.  This is not
	// supported by the Intuos Wintab.  Your context will always receive
	// packets, even if there has been no change in the data.
	m_lc.lcMoveMask = PACKETDATA;

	// Which buttons events will be handled by this context.  lcBtnMask
	// is a bitfield with one bit per button.
	m_lc.lcBtnUpMask = m_lc.lcBtnDnMask;

	// Set the entire tablet as active
	wWTInfoRetVal = gpWTInfoA(WTI_DEVICES + 0, DVC_X, &TabletX);
	WACOM_ASSERT(wWTInfoRetVal == sizeof(AXIS));

	wWTInfoRetVal = gpWTInfoA(WTI_DEVICES, DVC_Y, &TabletY);
	WACOM_ASSERT(wWTInfoRetVal == sizeof(AXIS));

	wWTInfoRetVal = gpWTInfoA(WTI_DEVICES, DVC_NPRESSURE, &TabletNPress);
	if (wWTInfoRetVal == sizeof(AXIS))
		m_selector.SetBrushPnMax(TabletNPress.axMax);
	else
		m_selector.SetBrushPnMax(1.0);

	wWTInfoRetVal = gpWTInfoA(WTI_DEVICES, DVC_TPRESSURE, &TabletTPress);
	if (wWTInfoRetVal == sizeof(AXIS))
		m_selector.SetBrushPtMax(TabletTPress.axMax);
	else
		m_selector.SetBrushPnMax(1.0);

/*	m_lc.lcInOrgX = 0;
	m_lc.lcInOrgY = 0;
	m_lc.lcInExtX = TabletX.axMax;
	m_lc.lcInExtY = TabletY.axMax;

	// Guarantee the output coordinate space to be in screen coordinates.  
	m_lc.lcOutOrgX = GetSystemMetrics(SM_XVIRTUALSCREEN);
	m_lc.lcOutOrgY = GetSystemMetrics(SM_YVIRTUALSCREEN);
	m_lc.lcOutExtX = GetSystemMetrics(SM_CXVIRTUALSCREEN); //SM_CXSCREEN );

														   // In Wintab, the tablet origin is lower left.  Move origin to upper left
														   // so that it coincides with screen origin.
	m_lc.lcOutExtY = -GetSystemMetrics(SM_CYVIRTUALSCREEN);	//SM_CYSCREEN );

															// Leave the system origin and extents as received:
															// lcSysOrgX, lcSysOrgY, lcSysExtX, lcSysExtY

															// open the region
															// The Wintab spec says we must open the context disabled if we are 
															// using cursor masks.  
*/	hctx = gpWTOpenA(hWnd, &m_lc, TRUE);

	WacomTrace("HCTX: %i\n", hctx);

	return hctx;
}

void VRenderGLView::InitOpenVR()
{
	//openvr initilization
	vr::EVRInitError vr_error;
	m_vr_system = vr::VR_Init(&vr_error, vr::VRApplication_Scene, 0);
	if (vr_error == vr::VRInitError_None &&
		vr::VRCompositor())
	{
		m_use_openvr = true;
		//get render size
		m_vr_system->GetRecommendedRenderTargetSize(&m_vr_size[0], &m_vr_size[1]);
		//get eye offset
		//vr::HmdMatrix34_t eye_mat;
		//eye_mat = m_vr_system->GetEyeToHeadTransform(vr::Eye_Left);
		//double eye_x = eye_mat.m[0][3];
		//double eye_y = eye_mat.m[1][3];
		//double eye_z = eye_mat.m[2][3];
		//m_vr_eye_offset = std::sqrt(eye_x*eye_x+eye_y*eye_y+eye_z*eye_z)*100.0;
	}//otherwise use default settings
}
#endif

VRenderGLView::~VRenderGLView()
{
	if (m_benchmark)
	{
		int msec = int(m_timer->total_time() * 1000.0);
		double fps = m_timer->total_fps();
		wxString string = wxString("FluoRender has finished benchmarking.\n") +
			wxString("Results:\n") +
			wxString("Render size: ") + wxString::Format("%d X %d\n", m_size.GetWidth(), m_size.GetHeight()) +
			wxString("Time: ") + wxString::Format("%d msec\n", msec) +
			wxString("Frames: ") + wxString::Format("%llu\n", m_timer->count()) +
			wxString("FPS: ") + wxString::Format("%.2f", fps);
		wxMessageDialog *diag = new wxMessageDialog(this, string, "Benchmark Results",
			wxOK | wxICON_INFORMATION);
		diag->ShowModal();
	}
	m_timer->stop();
	delete m_timer;

	m_selector.SaveBrushSettings();

#ifdef _WIN32
	//tablet
	if (m_hTab)
	{
		gpWTClose(m_hTab);
		m_hTab = 0;
		UnloadWintab();
	}
#endif

#ifdef _WIN32
	if (m_enable_vr && m_use_openvr)
	{
		//vr shutdown
		vr::VR_Shutdown();
		//UnloadVR();
	}
#endif

#if defined(_WIN32) && defined(USE_XINPUT)
	if (m_controller)
		delete m_controller;
#endif

	m_loader.StopAll();

	int i;
	//delete groups
	for (i = 0; i<(int)m_layer_list.size(); i++)
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

	//delete rulers
	for (i = 0; i<(int)m_ruler_list.size(); i++)
	{
		if (m_ruler_list[i])
			delete m_ruler_list[i];
	}

	if (m_trace_group)
		delete m_trace_group;

	if (m_full_screen)
	{
		m_full_screen = false;
		m_vrv->m_glview = 0;
		m_vrv->m_full_frame = 0;
		if (m_frame)
		{
			m_frame->ClearVrvList();
			m_frame->Close();
		}
	}

	if (m_mvr)
		delete m_mvr;

	if (!m_sharedRC)
		delete m_glRC;
}

void VRenderGLView::OnResize(wxSizeEvent& event)
{
	wxSize size = GetGLSize();
	if (m_size == size)
		return;
	else
		m_size = size;

	m_vrv->UpdateScaleFactor(false);

	RefreshGL(1);
}

void VRenderGLView::Init()
{
	if (!m_initialized)
	{
		flvr::ShaderProgram::init_shaders_supported();
		if (m_frame && m_frame->GetSettingDlg())
		{
			flvr::KernelProgram::set_platform_id(m_frame->
				GetSettingDlg()->GetCLPlatformID());
			flvr::KernelProgram::set_device_id(m_frame->
				GetSettingDlg()->GetCLDeviceID());
		}
		flvr::KernelProgram::init_kernels_supported();
#ifdef _DARWIN
		CGLContextObj ctx = CGLGetCurrentContext();
        if (ctx != flvr::TextureRenderer::gl_context_)
            flvr::TextureRenderer::gl_context_ = ctx;
#endif
		if (m_frame)
		{
			m_frame->SetTextureRendererSettings();
			m_frame->SetTextureUndos();
			m_frame->GetSettingDlg()->UpdateTextureSize();
		}
		//glViewport(0, 0, (GLint)(GetSize().x), (GLint)(GetSize().y));
		glEnable(GL_MULTISAMPLE);

		m_initialized = true;

		m_timer->start();
	}
}

void VRenderGLView::ClearAll()
{
	Clear();
	ClearVolList();
	ClearMeshList();
}

void VRenderGLView::Clear()
{
	m_loader.RemoveAllLoadedBrick();
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

void VRenderGLView::HandleProjection(int nx, int ny, bool vr)
{
	if (!m_free)
		m_distance = m_radius / tan(d2r(m_aov / 2.0)) / m_scale_factor;

	double aspect = (double)nx / (double)ny;
	m_ortho_left = -m_radius * aspect / m_scale_factor;
	m_ortho_right = -m_ortho_left;
	m_ortho_bottom = -m_radius / m_scale_factor;
	m_ortho_top = -m_ortho_bottom;

	if (vr && m_use_openvr)
	{
#ifdef _WIN32
		//get projection matrix
		vr::EVREye eye = m_vr_eye_idx ? vr::Eye_Right : vr::Eye_Left;
		auto proj_mat = m_vr_system->GetProjectionMatrix(eye, m_near_clip, m_far_clip);
		static int ti[] = { 0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15 };
		for (int i = 0; i < 16; ++i)
			glm::value_ptr(m_proj_mat)[i] =
			((float*)(proj_mat.m))[ti[i]];

		/*{//not used
			double aspect;
			aspect = (double)nx / (double)ny;
			double frustum_shift = (m_vr_eye_offset / 2.0) * m_near_clip / m_distance;
			m_ortho_top = std::tan(m_aov / 2.0) * m_near_clip;
			m_ortho_right = aspect * m_ortho_top + frustum_shift *
				(m_vr_eye_idx ? 1.0 : -1.0);
			m_ortho_left = -aspect * m_ortho_top + frustum_shift *
				(m_vr_eye_idx ? -1.0 : 1.0);
			m_ortho_bottom = -m_ortho_top;
			m_proj_mat = glm::frustum(
				m_ortho_left, m_ortho_right,
				m_ortho_bottom, m_ortho_top,
				m_near_clip, m_far_clip);
		}*/
#endif
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

void VRenderGLView::HandleCamera(bool vr)
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

	if (vr && m_enable_vr)
	{
		glm::vec3 offset((m_vr_eye_idx ? 1.0 : -1.0) * m_vr_eye_offset / 2.0, 0.0, 0.0);
		m_mv_mat = glm::lookAt(
			eye + offset,
			center + offset,
			up);
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

//depth buffer calculation
double VRenderGLView::CalcZ(double z)
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

void VRenderGLView::CalcFogRange()
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
		minz = numeric_limits<double>::max();
		maxz = -numeric_limits<double>::max();

		vector<fluo::Point> points;
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
		if (m_pin_rot_center)
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
void VRenderGLView::Draw()
{
	int nx, ny;
	GetRenderSize(nx, ny);

	// clear color and depth buffers
	glClearDepth(1.0);
	glClearColor(m_bg_color.r(), m_bg_color.g(), m_bg_color.b(), 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, (GLint)nx, (GLint)ny);

	//gradient background
	if (m_grad_bg)
		DrawGradBg();

	//projection
	HandleProjection(nx, ny, true);
	//Transformation
	HandleCamera(true);

	if (m_draw_all)
	{
		glm::mat4 mv_temp = m_mv_mat;
		m_mv_mat = GetDrawMat();

		if (m_use_fog)
			CalcFogRange();

		if (m_draw_grid)
			DrawGrid();

		if (m_draw_clip)
			DrawClippingPlanes(BACK_FACE);

		//setup
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//draw the volumes
		DrawVolumes();

		//draw the clipping planes
		if (m_draw_clip)
			DrawClippingPlanes(FRONT_FACE);

		if (m_draw_bounds)
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
void VRenderGLView::DrawDP()
{
	int i;
	int nx, ny;
	GetRenderSize(nx, ny);
	string name;
	flvr::Framebuffer* peel_buffer = 0;

	//clear
	//	glDrawBuffer(GL_BACK);
	glClearDepth(1.0);
	glClearColor(m_bg_color.r(), m_bg_color.g(), m_bg_color.b(), 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, (GLint)nx, (GLint)ny);

	//gradient background
	if (m_grad_bg)
		DrawGradBg();

	//projection
	HandleProjection(nx, ny, true);
	//Transformation
	HandleCamera(true);

	if (m_draw_all)
	{
		glm::mat4 mv_temp = m_mv_mat;
		m_mv_mat = GetDrawMat();

		bool use_fog_save = m_use_fog;
		if (m_use_fog)
			CalcFogRange();

		if (m_draw_grid)
			DrawGrid();

		if (m_draw_clip)
			DrawClippingPlanes(BACK_FACE);

		//setup
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		m_use_fog = false;

		//draw depth values of each layer into the buffers
		for (i = 0; i<m_peeling_layers; i++)
		{
			name = "peel buffer" + std::to_string(i);
			peel_buffer =
				flvr::TextureRenderer::framebuffer_manager_.framebuffer(
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
					flvr::TextureRenderer::framebuffer_manager_.framebuffer(name);
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
		for (i = m_peeling_layers; i >= 0; i--)
		{
			if (i == 0)
			{
				//draw volumes before the depth
				glActiveTexture(GL_TEXTURE15);
				name = "peel buffer" + std::to_string(0);
				peel_buffer =
					flvr::TextureRenderer::framebuffer_manager_.framebuffer(name);
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
				if (m_peeling_layers == 1)
				{
					//i == m_peeling_layers == 1
					glActiveTexture(GL_TEXTURE15);
					name = "peel buffer" + std::to_string(0);
					peel_buffer =
						flvr::TextureRenderer::framebuffer_manager_.framebuffer(name);
					if (peel_buffer)
						peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
					glActiveTexture(GL_TEXTURE0);
				}
				else if (m_peeling_layers == 2)
				{
					glActiveTexture(GL_TEXTURE14);
					name = "peel buffer" + std::to_string(0);
					peel_buffer =
						flvr::TextureRenderer::framebuffer_manager_.framebuffer(name);
					if (peel_buffer)
						peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
					glActiveTexture(GL_TEXTURE15);
					name = "peel buffer" + std::to_string(1);
					peel_buffer =
						flvr::TextureRenderer::framebuffer_manager_.framebuffer(name);
					if (peel_buffer)
						peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
					glActiveTexture(GL_TEXTURE0);
				}
				else if (m_peeling_layers > 2)
				{
					if (i == m_peeling_layers)
					{
						glActiveTexture(GL_TEXTURE14);
						name = "peel buffer" + std::to_string(i-2);
						peel_buffer =
							flvr::TextureRenderer::framebuffer_manager_.framebuffer(name);
						if (peel_buffer)
							peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
						glActiveTexture(GL_TEXTURE15);
						name = "peel buffer" + std::to_string(i-1);
						peel_buffer =
							flvr::TextureRenderer::framebuffer_manager_.framebuffer(name);
						if (peel_buffer)
							peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
						glActiveTexture(GL_TEXTURE0);
					}
					else if (i == 1)
					{
						glActiveTexture(GL_TEXTURE14);
						name = "peel buffer" + std::to_string(0);
						peel_buffer =
							flvr::TextureRenderer::framebuffer_manager_.framebuffer(name);
						if (peel_buffer)
							peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
						glActiveTexture(GL_TEXTURE15);
						name = "peel buffer" + std::to_string(1);
						peel_buffer =
							flvr::TextureRenderer::framebuffer_manager_.framebuffer(name);
						if (peel_buffer)
							peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
						glActiveTexture(GL_TEXTURE0);
					}
					else
					{
						glActiveTexture(GL_TEXTURE13);
						name = "peel buffer" + std::to_string(i-2);
						peel_buffer =
							flvr::TextureRenderer::framebuffer_manager_.framebuffer(name);
						if (peel_buffer)
							peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
						glActiveTexture(GL_TEXTURE14);
						name = "peel buffer" + std::to_string(i-1);
						peel_buffer =
							flvr::TextureRenderer::framebuffer_manager_.framebuffer(name);
						if (peel_buffer)
							peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
						glActiveTexture(GL_TEXTURE15);
						name = "peel buffer" + std::to_string(i);
						peel_buffer =
							flvr::TextureRenderer::framebuffer_manager_.framebuffer(name);
						if (peel_buffer)
							peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
						glActiveTexture(GL_TEXTURE0);
					}
				}

				//draw volumes
				if (m_peeling_layers == 1)
					//i == m_peeling_layers == 1
					DrawVolumes(5);//draw volume after 15
				else if (m_peeling_layers == 2)
				{
					if (i == 2)
						DrawVolumes(2);//draw volume after 15
					else if (i == 1)
						DrawVolumes(4);//draw volume after 14 and before 15
				}
				else if (m_peeling_layers > 2)
				{
					if (i == m_peeling_layers)
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

				if (m_peeling_layers == 1)
					//i == m_peeling_layers == 1
					DrawMeshes(5);//draw mesh at 15
				else if (m_peeling_layers == 2)
				{
					if (i == 2)
						DrawMeshes(2);//draw mesh after 14
					else if (i == 1)
						DrawMeshes(4);//draw mesh before 15
				}
				else if (m_peeling_layers > 2)
				{
					if (i == m_peeling_layers)
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

		if (m_draw_clip)
			DrawClippingPlanes(FRONT_FACE);

		if (m_draw_bounds)
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
void VRenderGLView::DrawMeshes(int peel)
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
void VRenderGLView::DrawVolumes(int peel)
{
	int finished_bricks = 0;
	if (flvr::TextureRenderer::get_mem_swap())
	{
		finished_bricks = flvr::TextureRenderer::get_finished_bricks();
		flvr::TextureRenderer::reset_finished_bricks();
	}

	PrepFinalBuffer();

	int nx, ny;
	GetRenderSize(nx, ny);

	flrd::RulerPoint *p0 = m_ruler_handler.GetPoint();

	//draw
	if (m_load_update ||
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

		if (m_frame &&
			m_frame->GetSettingDlg() &&
			m_frame->GetSettingDlg()->GetUpdateOrder() == 1)
		{
			if (m_interactive)
				ClearFinalBuffer();
			else if (m_clear_buffer)
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

		vector<VolumeData*> quota_vd_list;
		if (flvr::TextureRenderer::get_mem_swap())
		{
			//set start time for the texture renderer
			flvr::TextureRenderer::set_st_time(GET_TICK_COUNT());

			flvr::TextureRenderer::set_interactive(m_interactive);
			//if in interactive mode, do interactive bricking also
			if (m_interactive)
			{
				//calculate quota
				int total_bricks = flvr::TextureRenderer::get_total_brick_num();
				int quota_bricks = 1;// total_bricks / 2;
				int fin_bricks = finished_bricks;
				int last_bricks = flvr::TextureRenderer::
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
				////test
				//std::ofstream ofs("quota.txt", std::ios::out | std::ios::app);
				//std::string str;
				//str += std::to_string(quota_bricks) + "\t";
				//str += std::to_string(total_bricks) + "\t";
				//str += std::to_string(fin_bricks) + "\t";
				//str += std::to_string(adj_bricks) + "\t";
				//str += std::to_string(TextureRenderer::get_up_time()) + "\t";
				//str += std::to_string(up_time) + "\t";
				//str += std::to_string(consumed_time) + "\n";
				//ofs.write(str.c_str(), str.size());

				int quota_bricks_chan = 0;
				if (m_vd_pop_list.size() > 1)
				{
					//priority: 1-selected channel; 2-group contains selected channel; 3-linear distance to above
					//not considering mask for now
					vector<VolumeData*>::iterator cur_iter;
					cur_iter = find(m_vd_pop_list.begin(), m_vd_pop_list.end(), m_cur_vol);
					size_t cur_index = distance(m_vd_pop_list.begin(), cur_iter);
					int vd_index;
					if (cur_iter != m_vd_pop_list.end())
					{
						VolumeData* vd;
						vd = *cur_iter;
						quota_vd_list.push_back(vd);
						int count_bricks = vd->GetBrickNum();
						quota_bricks_chan = std::min(count_bricks, quota_bricks);
						vd->GetVR()->set_quota_bricks_chan(quota_bricks_chan);
						int count = 0;
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
							int brick_num = vd->GetBrickNum();
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
				m_vp.SetVolumeData(vd);
				fluo::Point p;
				if (m_vp.GetPointVolumeBox(nx / 2, ny / 2, false, p) > 0.0 ||
					(vd && m_vp.GetPointPlane(nx / 2, ny / 2, 0, false, p) > 0.0))
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
			if (flvr::TextureRenderer::get_mem_swap() &&
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
			vector<VolumeData*> list;
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
						if (flvr::TextureRenderer::get_mem_swap() &&
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
							if (flvr::TextureRenderer::get_mem_swap() &&
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

	if (flvr::TextureRenderer::get_mem_swap())
	{
		flvr::TextureRenderer::set_consumed_time(GET_TICK_COUNT() - flvr::TextureRenderer::get_st_time());
		if (flvr::TextureRenderer::get_start_update_loop() &&
			flvr::TextureRenderer::get_done_update_loop())
			flvr::TextureRenderer::reset_update_loop();
	}

	if (m_interactive)
	{
		//wxMouseState ms = wxGetMouseState();
		//if (ms.LeftIsDown() ||
		//	ms.MiddleIsDown() ||
		//	ms.RightIsDown())
		//	return;
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

void VRenderGLView::DrawAnnotations()
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
				string str;
				fluo::Point pos;
				wstring wstr;
				for (int j = 0; j<ann->GetTextNum(); ++j)
				{
					str = ann->GetTextText(j);
					wstr = s2ws(str);
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
						m_text_renderer.RenderText(
							wstr, text_color,
							px*sx, py*sy, sx, sy);
					}
				}
			}
		}
	}
}

//get populated mesh list
//stored in m_md_pop_list
void VRenderGLView::PopMeshList()
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

//get populated volume list
//stored in m_vd_pop_list
void VRenderGLView::PopVolumeList()
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

//organize layers in view
//put all volume data under view into last group of the view
//if no group in view
void VRenderGLView::OrganizeLayers()
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
			wxString name = vd->GetName();
			if (le_group)
			{
				RemoveVolumeData(name);
				le_group->InsertVolumeData(le_group->GetVolumeNum(), vd);
			}
			else
			{
				wxString group_name = AddGroup("");
				le_group = GetGroup(group_name);
				if (le_group)
				{
					RemoveVolumeData(name);
					le_group->InsertVolumeData(le_group->GetVolumeNum(), vd);
				}
			}

			if (m_frame)
			{
				AdjustView* adjust_view = m_frame->GetAdjustView();
				if (adjust_view)
				{
					adjust_view->SetGroupLink(le_group);
					adjust_view->UpdateSync();
				}
			}
		}
	}
}

void VRenderGLView::RandomizeColor()
{
	for (int i = 0; i<(int)m_layer_list.size(); i++)
	{
		if (m_layer_list[i])
			m_layer_list[i]->RandomizeColor();
	}
}

void VRenderGLView::ClearVolList()
{
	m_loader.RemoveAllLoadedBrick();
	flvr::TextureRenderer::clear_tex_pool();
	m_vd_pop_list.clear();
}

void VRenderGLView::ClearMeshList()
{
	m_md_pop_list.clear();
}

//interactive modes
int VRenderGLView::GetIntMode()
{
	return m_int_mode;
}

void VRenderGLView::SetIntMode(int mode)
{
	m_int_mode = mode;
	if (m_int_mode == 1)
	{
		m_brush_state = 0;
		m_draw_brush = false;
	}
	else if (m_int_mode == 10 ||
		m_int_mode == 12)
		SetPaintMode(9);
}

//set use 2d rendering results
void VRenderGLView::SetPaintUse2d(bool use2d)
{
	m_selector.SetPaintUse2d(use2d);
}

bool VRenderGLView::GetPaintUse2d()
{
	return m_selector.GetPaintUse2d();
}

//segmentation mdoe selection
void VRenderGLView::SetPaintMode(int mode)
{
	m_selector.SetMode(mode);
	m_brush_state = mode;
	m_selector.ChangeBrushSetsIndex();
}

int VRenderGLView::GetPaintMode()
{
	return m_selector.GetMode();
}

void VRenderGLView::DrawCircles(double cx, double cy,
	double r1, double r2, fluo::Color &color, glm::mat4 &matrix)
{
	flvr::ShaderProgram* shader =
		flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHDR_DRAW_GEOMETRY);
	if (shader)
	{
		if (!shader->valid())
			shader->create();
		shader->bind();
	}

	shader->setLocalParam(0, color.r(), color.g(), color.b(), 1.0);
	//apply translate first
	glm::mat4 mat0 = matrix * glm::translate(
			glm::mat4(), glm::vec3(cx, cy, 0.0));
	shader->setLocalParamMatrix(0, glm::value_ptr(mat0));

	flvr::VertexArray* va_circles =
		flvr::TextureRenderer::vertex_array_manager_.vertex_array(flvr::VA_Brush_Circles);
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
void VRenderGLView::DrawBrush()
{
	double pressure = m_selector.GetNormPress();

	wxPoint pos1(old_mouse_X, old_mouse_Y);
	if (GetMouseIn(pos1))
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
		if (m_selector.GetBrushSizeData())
		{
			proj_mat = glm::ortho(float(m_ortho_left), float(m_ortho_right),
				float(m_ortho_top), float(m_ortho_bottom));
			cx = m_ortho_left + pos1.x * (m_ortho_right - m_ortho_left) / nx;
			cy = m_ortho_bottom + pos1.y * (m_ortho_top - m_ortho_bottom) / ny;
		}
		else
		{
			proj_mat = glm::ortho(float(0), float(nx), float(0), float(ny));
			cx = pos1.x;
			cy = ny - pos1.y;
		}

		//attributes
		glDisable(GL_DEPTH_TEST);

		int mode = m_selector.GetMode();

		fluo::Color text_color = GetTextColor();

		double br1 = m_selector.GetBrushSize1();
		double br2 = m_selector.GetBrushSize2();

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

		float cx2 = pos1.x;
		float cy2 = ny - pos1.y;
		float px, py;
		px = cx2 - 7 - nx / 2.0;
		py = cy2 - 3 - ny / 2.0;
		wstring wstr;
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
		m_text_renderer.RenderText(wstr, text_color, px*sx, py*sy, sx, sy);

		glEnable(GL_DEPTH_TEST);

	}
}

//paint strokes on the paint fbo
void VRenderGLView::PaintStroke()
{
	int nx, ny;
	GetRenderSize(nx, ny);

	double pressure = m_selector.GetNormPress();

	//generate texture and buffer objects
	//painting fbo
	flvr::Framebuffer* paint_buffer =
		flvr::TextureRenderer::framebuffer_manager_.framebuffer(
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
			flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHDR_PAINT);
		if (paint_shader)
		{
			if (!paint_shader->valid())
				paint_shader->create();
			paint_shader->bind();
		}

		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendEquation(GL_MAX);

		double radius1 = m_selector.GetBrushSize1();
		double radius2 = m_selector.GetBrushSize2();
		double bspc = m_selector.GetBrushSpacing();
		bool bs_data = m_selector.GetBrushSizeData();
		double px = double(old_mouse_X - prv_mouse_X);
		double py = double(old_mouse_Y - prv_mouse_Y);
		double dist = sqrt(px*px + py*py);
		double step = radius1 * pressure * bspc;
		int repeat = int(dist / step + 0.5);
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
			switch (m_selector.GetMode())
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
void VRenderGLView::DisplayStroke()
{
	//painting texture
	flvr::Framebuffer* paint_buffer =
		flvr::TextureRenderer::framebuffer_manager_.framebuffer("paint brush");
	if (!paint_buffer)
		return;

	//draw the final buffer to the windows buffer
	glActiveTexture(GL_TEXTURE0);
	paint_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	flvr::ShaderProgram* img_shader =
		flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHADER_TEXTURE_LOOKUP);
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

//segment volumes in current view
void VRenderGLView::Segment()
{
	int mode = m_selector.GetMode();

	//add ml record
	if (glbin.get_cg_table_enable() &&
		glbin.get_cg_entry().getValid() &&
		m_cur_vol)
	{
		//histogram
		flrd::Histogram histogram(m_cur_vol);
		histogram.SetUseMask(true);
		flrd::EntryHist* eh = histogram.GetEntryHist();

		if (eh)
		{
			//record
			flrd::RecordHistParams* rec = new flrd::RecordHistParams();
			rec->setInput(eh);
			flrd::EntryParams* ep = new flrd::EntryParams(glbin.get_cg_entry());
			rec->setOutput(ep);

			//table
			glbin.get_cg_table().addRecord(rec);
		}
	}

	HandleCamera();
	if (mode == 9)
	{
		wxPoint mouse_pos = ScreenToClient(wxGetMousePosition());
		m_selector.Segment(mouse_pos.x, mouse_pos.y);
	}
	else
		m_selector.Segment();

	bool count = false;
	bool colocal = false;
	if (mode == 1 ||
		mode == 2 ||
		mode == 3 ||
		mode == 4 ||
		mode == 5 ||
		mode == 7 ||
		mode == 8 ||
		mode == 9)
	{
		if (m_paint_count)
			count = true;
		if (m_paint_colocalize)
			colocal = true;
	}

	//update
	if (m_frame)
	{
		if (m_frame->GetBrushToolDlg())
			m_frame->GetBrushToolDlg()->Update(count?0:1);
		if (colocal && m_frame->GetColocalizationDlg())
			m_frame->GetColocalizationDlg()->Colocalize();
	}
}

//change brush display
void VRenderGLView::ChangeBrushSize(int value)
{
	m_selector.ChangeBrushSize(value, wxGetKeyState(WXK_CONTROL));
	if (m_frame && m_frame->GetBrushToolDlg())
		m_frame->GetBrushToolDlg()->GetSettings(this);
}

//calculations
void VRenderGLView::SetVolumeA(VolumeData* vd)
{
	m_calculator.SetVolumeA(vd);
	m_selector.SetVolume(vd);
}

void VRenderGLView::SetVolumeB(VolumeData* vd)
{
	m_calculator.SetVolumeB(vd);
}

//draw out the framebuffer after composition
void VRenderGLView::PrepFinalBuffer()
{
	int nx, ny;
	GetRenderSize(nx, ny);

	//generate textures & buffer objects
	glActiveTexture(GL_TEXTURE0);
	//glEnable(GL_TEXTURE_2D);
	//frame buffer for final
	flvr::Framebuffer* final_buffer =
		flvr::TextureRenderer::framebuffer_manager_.framebuffer(
			flvr::FB_Render_RGBA, nx, ny, "final");
	if (final_buffer)
		final_buffer->protect();
}

void VRenderGLView::ClearFinalBuffer()
{
	flvr::Framebuffer* final_buffer =
		flvr::TextureRenderer::framebuffer_manager_.framebuffer(
		"final");
	if (final_buffer)
		final_buffer->bind();
	//clear color buffer to black for compositing
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
}

void VRenderGLView::DrawFinalBuffer()
{
	if (m_enlarge)
		return;

	//bind back the window frame buffer
	BindRenderBuffer();

	//draw the final buffer to the windows buffer
	glActiveTexture(GL_TEXTURE0);
	flvr::Framebuffer* final_buffer =
		flvr::TextureRenderer::framebuffer_manager_.framebuffer("final");
	if (final_buffer)
		final_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	//glBlendFunc(GL_ONE, GL_ONE);
	glDisable(GL_DEPTH_TEST);

	//2d adjustment
	flvr::ShaderProgram* img_shader =
		flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHDR_BLEND_BRIGHT_BACKGROUND_HDR);
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

	//get pixel value
	if (m_pin_rot_center &&
		m_rot_center_dirty &&
		!m_free)
	{
		unsigned char pixel[4];
		int nx, ny;
		GetRenderSize(nx, ny);
		glReadPixels(nx / 2, ny / 2, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
		m_pin_pick_thresh = 0.8 * double(pixel[3]) / 255.0;
	}
}

//vr buffers
void VRenderGLView::GetRenderSize(int &nx, int &ny)
{
	if (m_use_openvr)
	{
		nx = m_vr_size[0];
		ny = m_vr_size[1];
	}
	else
	{
		nx = GetGLSize().x;
		ny = GetGLSize().y;
		if (m_enable_vr)
			nx /= 2;
	}
}

void VRenderGLView::PrepVRBuffer()
{
	if (m_use_openvr)
	{
#ifdef _WIN32
		std::array<vr::TrackedDevicePose_t, vr::k_unMaxTrackedDeviceCount> tracked_device_poses;
		vr::VRCompositor()->WaitGetPoses(tracked_device_poses.data(), tracked_device_poses.size(), NULL, 0);
#endif
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
		flvr::TextureRenderer::framebuffer_manager_.framebuffer(
			flvr::FB_UChar_RGBA, nx, ny, vr_buf_name);
	if (vr_buffer)
		vr_buffer->protect();
}

void VRenderGLView::BindRenderBuffer()
{
	if (m_enable_vr)
	{
		std::string vr_buf_name;
		if (m_vr_eye_idx)
			vr_buf_name = "vr right";
		else
			vr_buf_name = "vr left";
		flvr::Framebuffer* vr_buffer =
			flvr::TextureRenderer::framebuffer_manager_.framebuffer(
				vr_buf_name);
		if (vr_buffer)
			vr_buffer->bind();
	}
	else
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void VRenderGLView::ClearVRBuffer()
{
	BindRenderBuffer();
	//clear color buffer to black for compositing
	glClearDepth(1.0);
	glClearColor(m_bg_color.r(), m_bg_color.g(), m_bg_color.b(), 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void VRenderGLView::DrawVRBuffer()
{
	int vr_x, vr_y, gl_x, gl_y;
	GetRenderSize(vr_x, vr_y);
	gl_x = GetGLSize().x;
	gl_y = GetGLSize().y;
	int vp_y = int((double)gl_x * vr_y / vr_x / 2.0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, gl_x, vp_y);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	flvr::ShaderProgram* img_shader =
		flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHADER_TEXTURE_LOOKUP);
	if (img_shader)
	{
		if (!img_shader->valid())
			img_shader->create();
		img_shader->bind();
	}
	//left eye
	flvr::Framebuffer* buffer =
		flvr::TextureRenderer::framebuffer_manager_.framebuffer(
			"vr left");
	if (buffer)
		buffer->bind_texture(GL_COLOR_ATTACHMENT0);
	flvr::VertexArray* quad_va =
		flvr::TextureRenderer::vertex_array_manager_.vertex_array(flvr::VA_Left_Square);
	if (quad_va)
		quad_va->draw();
	//openvr left eye
	if (m_use_openvr)
	{
#ifdef _WIN32
		vr::Texture_t left_eye = {};
		left_eye.handle = reinterpret_cast<void*>(
			(unsigned long long)(buffer->tex_id(GL_COLOR_ATTACHMENT0)));
		left_eye.eType = vr::TextureType_OpenGL;
		left_eye.eColorSpace = vr::ColorSpace_Gamma;
		vr::VRCompositor()->Submit(vr::Eye_Left, &left_eye, nullptr);
#endif
	}
	//right eye
	buffer =
		flvr::TextureRenderer::framebuffer_manager_.framebuffer(
			"vr right");
	if (buffer)
		buffer->bind_texture(GL_COLOR_ATTACHMENT0);
	quad_va =
		flvr::TextureRenderer::vertex_array_manager_.vertex_array(flvr::VA_Right_Square);
	if (quad_va)
		quad_va->draw();
	//openvr left eye
	if (m_use_openvr)
	{
#ifdef _WIN32
		vr::Texture_t right_eye = {};
		right_eye.handle = reinterpret_cast<void*>(
			(unsigned long long)(buffer->tex_id(GL_COLOR_ATTACHMENT0)));
		right_eye.eType = vr::TextureType_OpenGL;
		right_eye.eColorSpace = vr::ColorSpace_Gamma;
		vr::VRCompositor()->Submit(vr::Eye_Right, &right_eye, nullptr);
#endif
	}

	if (img_shader && img_shader->valid())
		img_shader->release();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

//Draw the volmues with compositing
//peel==true -- depth peeling
void VRenderGLView::DrawVolumesComp(vector<VolumeData*> &list, bool mask, int peel)
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
		flvr::TextureRenderer::framebuffer_manager_.framebuffer(
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

void VRenderGLView::DrawOVER(VolumeData* vd, bool mask, int peel)
{
	int nx, ny;
	GetRenderSize(nx, ny);
	GLint vp[4] = { 0, 0, (GLint)nx, (GLint)ny };
	GLfloat clear_color[4] = { 0, 0, 0, 0 };

	flvr::ShaderProgram* img_shader = 0;

	bool do_over = true;
	if (flvr::TextureRenderer::get_mem_swap() &&
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
		flvr::TextureRenderer::framebuffer_manager_.framebuffer("channel");
	if (do_over)
	{
		//before rendering this channel, save final buffer to temp buffer
		if (flvr::TextureRenderer::get_mem_swap() &&
			flvr::TextureRenderer::get_start_update_loop() &&
			flvr::TextureRenderer::get_save_final_buffer())
		{
			flvr::TextureRenderer::reset_save_final_buffer();

			//bind temporary framebuffer for comp in stream mode
			flvr::Framebuffer* temp_buffer =
				flvr::TextureRenderer::framebuffer_manager_.framebuffer(
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
				flvr::TextureRenderer::framebuffer_manager_.framebuffer("final");
			if (final_buffer)
				final_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
			glDisable(GL_BLEND);
			glDisable(GL_DEPTH_TEST);

			img_shader =
				flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHADER_TEXTURE_LOOKUP);
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

		if (!flvr::TextureRenderer::get_mem_swap() ||
			(flvr::TextureRenderer::get_mem_swap() &&
			flvr::TextureRenderer::get_clear_chan_buffer()))
		{
			glClearColor(0.0, 0.0, 0.0, 0.0);
			glClear(GL_COLOR_BUFFER_BIT);
			flvr::TextureRenderer::reset_clear_chan_buffer();
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
		vd->Draw(!m_persp, m_adaptive, m_interactive, m_scale_factor, Get121ScaleFactor());
	}

	if (vd->GetShadow())
	{
		vector<VolumeData*> list;
		list.push_back(vd);
		DrawOLShadows(list);
	}

	//bind fbo for final composition
	flvr::Framebuffer* final_buffer =
		flvr::TextureRenderer::framebuffer_manager_.framebuffer(
		"final");
	if (final_buffer)
		final_buffer->bind();

	if (flvr::TextureRenderer::get_mem_swap())
	{
		//restore temp buffer to final buffer
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
		flvr::Framebuffer* temp_buffer =
			flvr::TextureRenderer::framebuffer_manager_.framebuffer(
			"temporary");
		if (temp_buffer)
		{
			//temp buffer becomes unused after texture is bound
			//ok to unprotect
			temp_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
			temp_buffer->unprotect();
		}
		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);

		img_shader =
			flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHADER_TEXTURE_LOOKUP);
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
		flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHDR_BRIGHTNESS_CONTRAST_HDR);
	if (img_shader)
	{
		if (!img_shader->valid())
			img_shader->create();
		img_shader->bind();
	}
	fluo::Color gamma = vd->GetGamma();
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
	if (flvr::TextureRenderer::get_mem_swap() &&
		flvr::TextureRenderer::get_done_current_chan())
	{
		vector<flvr::TextureBrick*> *bricks =
			vd->GetTexture()->get_bricks();
		for (int i = 0; i < bricks->size(); i++)
			(*bricks)[i]->set_drawn(false);
	}
}

void VRenderGLView::DrawMIP(VolumeData* vd, int peel)
{
	int nx, ny;
	GetRenderSize(nx, ny);
	GLint vp[4] = { 0, 0, (GLint)nx, (GLint)ny };
	GLfloat clear_color[4] = { 0, 0, 0, 0 };

	bool do_mip = true;
	if (flvr::TextureRenderer::get_mem_swap() &&
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
	bool shadow = vd->GetShadow();
	int color_mode = vd->GetColormapMode();
	bool enable_alpha = vd->GetEnableAlpha();
	flvr::ShaderProgram* img_shader = 0;

	flvr::Framebuffer* chann_buffer =
		flvr::TextureRenderer::framebuffer_manager_.framebuffer("channel");
	flvr::Framebuffer* overlay_buffer = 0;
	if (do_mip)
	{
		//before rendering this channel, save final buffer to temp buffer
		if (flvr::TextureRenderer::get_mem_swap() &&
			flvr::TextureRenderer::get_start_update_loop() &&
			flvr::TextureRenderer::get_save_final_buffer())
		{
			flvr::TextureRenderer::reset_save_final_buffer();

			//bind temporary framebuffer for comp in stream mode
			flvr::Framebuffer* temp_buffer =
				flvr::TextureRenderer::framebuffer_manager_.framebuffer(
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
				flvr::TextureRenderer::framebuffer_manager_.framebuffer("final");
			if (final_buffer)
				final_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
			glDisable(GL_BLEND);
			glDisable(GL_DEPTH_TEST);

			img_shader =
				flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHADER_TEXTURE_LOOKUP);
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
			flvr::TextureRenderer::framebuffer_manager_.framebuffer(
				flvr::FB_Render_RGBA, nx, ny);
		if (overlay_buffer)
		{
			overlay_buffer->bind();
			overlay_buffer->protect();
			m_cur_framebuffer = overlay_buffer->id();
		}

		if (!flvr::TextureRenderer::get_mem_swap() ||
			(flvr::TextureRenderer::get_mem_swap() &&
			flvr::TextureRenderer::get_clear_chan_buffer()))
		{
			glClearColor(0.0, 0.0, 0.0, 0.0);
			glClear(GL_COLOR_BUFFER_BIT);
			flvr::TextureRenderer::reset_clear_chan_buffer();
		}

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
			vd->SetEnableAlpha(false);
		//draw
		vd->SetStreamMode(1);
		vd->SetMatrices(m_mv_mat, m_proj_mat, m_tex_mat);
		vd->SetViewport(vp);
		vd->SetClearColor(clear_color);
		vd->SetCurFramebuffer(m_cur_framebuffer);
		vd->Draw(!m_persp, m_adaptive, m_interactive, m_scale_factor, Get121ScaleFactor());
		//restore
		if (color_mode == 0)
			vd->SetColormapProj(saved_colormap_proj);
		if (color_mode == 1)
		{
			vd->RestoreMode();
			//restore alpha
			vd->SetEnableAlpha(enable_alpha);
		}

		//bind channel fbo for final composition
		if (chann_buffer)
			chann_buffer->bind();
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
		if (overlay_buffer)
		{
			//ok to unprotect
			overlay_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
			overlay_buffer->unprotect();
		}
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

		if (color_mode == 1)
		{
			//2d adjustment
			if (vd->GetColormapProj())
				img_shader = flvr::TextureRenderer::img_shader_factory_.shader(
					IMG_SHDR_GRADIENT_PROJ_MAP, vd->GetColormap());
			else
				img_shader = flvr::TextureRenderer::img_shader_factory_.shader(
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
				flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHADER_TEXTURE_LOOKUP);
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
		vector<VolumeData*> list;
		list.push_back(vd);
		DrawOLShadows(list);
	}

	//bind fbo for final composition
	flvr::Framebuffer* final_buffer =
		flvr::TextureRenderer::framebuffer_manager_.framebuffer(
		"final");
	if (final_buffer)
		final_buffer->bind();

	if (flvr::TextureRenderer::get_mem_swap())
	{
		//restore temp buffer to final buffer
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
		flvr::Framebuffer* temp_buffer =
			flvr::TextureRenderer::framebuffer_manager_.framebuffer(
				"temporary");
		if (temp_buffer)
		{
			//bind tex from temp buffer
			//it becomes unprotected afterwards
			temp_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
			temp_buffer->unprotect();
		}
		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);

		img_shader =
			flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHADER_TEXTURE_LOOKUP);
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
		flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHDR_BRIGHTNESS_CONTRAST_HDR);
	if (img_shader)
	{
		if (!img_shader->valid())
			img_shader->create();
		img_shader->bind();
	}
	fluo::Color gamma = vd->GetGamma();
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
	if (flvr::TextureRenderer::get_mem_swap() &&
		flvr::TextureRenderer::get_done_current_chan())
	{
		vector<flvr::TextureBrick*> *bricks =
			vd->GetTexture()->get_bricks();
		for (int i = 0; i < bricks->size(); i++)
			(*bricks)[i]->set_drawn(false);
	}
}

void VRenderGLView::DrawOLShading(VolumeData* vd)
{
	int nx, ny;
	GetRenderSize(nx, ny);

	if (flvr::TextureRenderer::get_mem_swap() &&
		flvr::TextureRenderer::get_start_update_loop() &&
		!flvr::TextureRenderer::get_done_update_loop())
	{
		unsigned int rn_time = GET_TICK_COUNT();
		if (rn_time - flvr::TextureRenderer::get_st_time() >
			flvr::TextureRenderer::get_up_time())
			return;
		if (vd->GetVR()->get_done_loop(2))
			return;
	}

	//shading pass
	flvr::Framebuffer* overlay_buffer =
		flvr::TextureRenderer::framebuffer_manager_.framebuffer(
		flvr::FB_Render_RGBA, nx, ny);
	if (overlay_buffer)
	{
		overlay_buffer->bind();
		overlay_buffer->protect();
	}
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	vd->GetVR()->set_shading(true);
	bool alpha = vd->GetEnableAlpha();
	vd->SetEnableAlpha(true);
	vd->SetMode(2);
	int colormode = vd->GetColormapMode();
	vd->SetStreamMode(2);
	vd->SetMatrices(m_mv_mat, m_proj_mat, m_tex_mat);
	vd->SetFog(m_use_fog, m_fog_intensity, m_fog_start, m_fog_end);
	vd->Draw(!m_persp, m_adaptive, m_interactive, m_scale_factor, Get121ScaleFactor());
	vd->RestoreMode();
	vd->SetColormapMode(colormode);
	vd->SetEnableAlpha(alpha);

	//bind fbo for final composition
	flvr::Framebuffer* chann_buffer =
		flvr::TextureRenderer::framebuffer_manager_.framebuffer("channel");
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
		flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHADER_TEXTURE_LOOKUP);
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

//get mesh shadow
bool VRenderGLView::GetMeshShadow(double &val)
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
				md->GetShadowParams(val);
				return md->GetShadow();
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
						md->GetShadowParams(val);
						return md->GetShadow();
					}
				}
			}
		}
	}
	val = 0.0;
	return false;
}

void VRenderGLView::DrawOLShadowsMesh(double darkness)
{
	int nx, ny;
	GetRenderSize(nx, ny);

	//shadow pass
	//bind the fbo
	flvr::Framebuffer* overlay_buffer =
		flvr::TextureRenderer::framebuffer_manager_.framebuffer(
		flvr::FB_Render_RGBA, nx, ny);
	if (overlay_buffer)
	{
		overlay_buffer->bind();
		overlay_buffer->protect();
	}
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	string name = "peel buffer" + std::to_string(0);
	flvr::Framebuffer* peel_buffer =
		flvr::TextureRenderer::framebuffer_manager_.framebuffer(name);
	if (peel_buffer)
		peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	//2d adjustment
	flvr::ShaderProgram* img_shader =
		flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHDR_DEPTH_TO_GRADIENT);
	if (img_shader)
	{
		if (!img_shader->valid())
		{
			img_shader->create();
		}
		img_shader->bind();
	}
	img_shader->setLocalParam(0, 1.0 / nx, 1.0 / ny, m_persp ? 2e10 : 1e6, 0.0);
	double dir_x = 0.0, dir_y = 0.0;
	if (m_frame && m_frame->GetSettingDlg())
		m_frame->GetSettingDlg()->GetShadowDir(dir_x, dir_y);
	img_shader->setLocalParam(1, dir_x, dir_y, 0.0, 0.0);
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
		flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHDR_GRADIENT_TO_SHADOW_MESH);
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
		flvr::TextureRenderer::framebuffer_manager_.framebuffer("final");
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

void VRenderGLView::DrawOLShadows(vector<VolumeData*> &vlist)
{
	if (vlist.empty())
		return;

	int nx, ny;
	GetRenderSize(nx, ny);
	GLint vp[4] = { 0, 0, (GLint)nx, (GLint)ny };
	GLfloat clear_color[4] = { 1, 1, 1, 1 };

	size_t i;
	bool has_shadow = false;
	vector<int> colormodes;
	vector<bool> shadings;
	vector<VolumeData*> list;
	//generate list
	for (i = 0; i<vlist.size(); i++)
	{
		VolumeData* vd = vlist[i];
		if (vd && vd->GetShadow())
		{
			colormodes.push_back(vd->GetColormapMode());
			shadings.push_back(vd->GetVR()->get_shading());
			list.push_back(vd);
			has_shadow = true;
		}
	}

	if (!has_shadow)
		return;

	if (flvr::TextureRenderer::get_mem_swap() &&
		flvr::TextureRenderer::get_start_update_loop() &&
		!flvr::TextureRenderer::get_done_update_loop())
	{
		unsigned int rn_time = GET_TICK_COUNT();
		if (rn_time - flvr::TextureRenderer::get_st_time() >
			flvr::TextureRenderer::get_up_time())
			return;
		if (list.size() == 1 && list[0]->GetShadow())
			if (list[0]->GetVR()->get_done_loop(3))
				return;
	}

	flvr::Framebuffer* overlay_buffer =
		flvr::TextureRenderer::framebuffer_manager_.framebuffer(
			flvr::FB_Render_RGBA, nx, ny);
	if (overlay_buffer)
	{
		overlay_buffer->bind();
		overlay_buffer->protect();
		m_cur_framebuffer = overlay_buffer->id();
	}

	if (!flvr::TextureRenderer::get_mem_swap() ||
		(flvr::TextureRenderer::get_mem_swap() &&
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
		vd->Draw(!m_persp, m_adaptive, m_interactive, m_scale_factor, Get121ScaleFactor());
		//restore
		vd->RestoreMode();
		vd->SetMaskMode(msk_mode);
		vd->SetColormapMode(colormode);
		vd->GetVR()->set_shading(shading);
		vd->GetShadowParams(shadow_darkness);
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
		m_mvr->draw(m_test_wiref, m_adaptive, m_interactive, !m_persp, m_intp);

		for (i = 0; i<list.size(); i++)
		{
			VolumeData* vd = list[i];
			vd->RestoreMode();
			vd->SetColormapMode(colormodes[i]);
			vd->GetVR()->set_shading(shadings[i]);
		}
		list[0]->GetShadowParams(shadow_darkness);
	}

	//
	if (!flvr::TextureRenderer::get_mem_swap() ||
		(flvr::TextureRenderer::get_mem_swap() &&
		flvr::TextureRenderer::get_clear_chan_buffer()))
	{
		//shadow pass
		//bind the fbo
		flvr::Framebuffer* temp_buffer =
			flvr::TextureRenderer::framebuffer_manager_.framebuffer(
				flvr::FB_Render_RGBA, nx, ny);
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
			flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHDR_DEPTH_TO_GRADIENT);
		if (img_shader)
		{
			if (!img_shader->valid())
				img_shader->create();
			img_shader->bind();
		}
		img_shader->setLocalParam(0, 1.0 / nx, 1.0 / ny, m_persp ? 2e10 : 1e6, 0.0);
		double dir_x = 0.0, dir_y = 0.0;
		if (m_frame && m_frame->GetSettingDlg())
			m_frame->GetSettingDlg()->GetShadowDir(dir_x, dir_y);
		img_shader->setLocalParam(1, dir_x, dir_y, 0.0, 0.0);
		//2d adjustment

		DrawViewQuad();

		if (img_shader && img_shader->valid())
			img_shader->release();

		//bind fbo for final composition
		flvr::Framebuffer* chann_buffer =
			flvr::TextureRenderer::framebuffer_manager_.framebuffer("channel");
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
			flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHDR_GRADIENT_TO_SHADOW);
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

//draw multi volumes with depth consideration
//peel==true -- depth peeling
void VRenderGLView::DrawVolumesMulti(vector<VolumeData*> &list, int peel)
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

	m_mvr->set_blend_slices(m_blend_slices);

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
		flvr::TextureRenderer::framebuffer_manager_.framebuffer(
			flvr::FB_Render_RGBA, nx, ny, "channel");
	//bind the fbo
	if (chann_buffer)
	{
		chann_buffer->protect();
		chann_buffer->bind();
		m_cur_framebuffer = chann_buffer->id();
	}
	if (!flvr::TextureRenderer::get_mem_swap() ||
		(flvr::TextureRenderer::get_mem_swap() &&
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
	m_mvr->draw(m_test_wiref, m_adaptive, m_interactive, !m_persp, m_intp);

	//draw shadows
	DrawOLShadows(list);

	//bind fbo for final composition
	flvr::Framebuffer* final_buffer =
		flvr::TextureRenderer::framebuffer_manager_.framebuffer(
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
		flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHDR_BRIGHTNESS_CONTRAST_HDR);
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
	gamma = vd->GetGamma();
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

void VRenderGLView::SetBrush(int mode)
{
	m_prev_focus = FindFocus();
	SetFocus();

	int ruler_type = m_ruler_handler.GetType();

	if (m_int_mode == 5 ||
		m_int_mode == 7)
	{
		m_int_mode = 7;
		if (ruler_type == 3)
			m_selector.SetMode(8);
		else
			m_selector.SetMode(1);
	}
	else if (m_int_mode == 8)
	{
		if (ruler_type == 3)
			m_selector.SetMode(8);
		else
			m_selector.SetMode(1);
	}
	else if (m_int_mode == 10)
	{
		m_selector.SetMode(9);
	}
	else
	{
		m_int_mode = 2;
		m_selector.SetMode(mode);
	}
	m_paint_display = true;
	m_draw_brush = true;
	m_selector.ChangeBrushSetsIndex();
}

void VRenderGLView::UpdateBrushState()
{
	TreePanel* tree_panel = 0;
	BrushToolDlg* brush_dlg = 0;
	if (m_frame)
	{
		tree_panel = m_frame->GetTree();
		brush_dlg = m_frame->GetBrushToolDlg();
	}

	if (m_int_mode != 2 && m_int_mode != 7)
	{
		if (wxGetKeyState(WXK_SHIFT))
		{
			SetBrush(2);
			if (tree_panel)
				tree_panel->SelectBrush(TreePanel::ID_BrushAppend);
			if (brush_dlg)
				brush_dlg->SelectBrush(BrushToolDlg::ID_BrushAppend);
			RefreshGL(6);
		}
		else if (wxGetKeyState(wxKeyCode('Z')))
		{
			SetBrush(4);
			if (tree_panel)
				tree_panel->SelectBrush(TreePanel::ID_BrushDiffuse);
			if (brush_dlg)
				brush_dlg->SelectBrush(BrushToolDlg::ID_BrushDiffuse);
			RefreshGL(7);
		}
		else if (wxGetKeyState(wxKeyCode('X')))
		{
			SetBrush(3);
			if (tree_panel)
				tree_panel->SelectBrush(TreePanel::ID_BrushDesel);
			if (brush_dlg)
				brush_dlg->SelectBrush(BrushToolDlg::ID_BrushDesel);
			RefreshGL(8);
		}
	}
	else
	{
		if (m_brush_state)
		{
			if (wxGetKeyState(WXK_SHIFT))
			{
				m_brush_state = 0;
				SetBrush(2);
				if (tree_panel)
					tree_panel->SelectBrush(TreePanel::ID_BrushAppend);
				if (brush_dlg)
					brush_dlg->SelectBrush(BrushToolDlg::ID_BrushAppend);
				RefreshGL(9);
			}
			else if (wxGetKeyState(wxKeyCode('Z')))
			{
				m_brush_state = 0;
				SetBrush(4);
				if (tree_panel)
					tree_panel->SelectBrush(TreePanel::ID_BrushDiffuse);
				if (brush_dlg)
					brush_dlg->SelectBrush(BrushToolDlg::ID_BrushDiffuse);
				RefreshGL(10);
			}
			else if (wxGetKeyState(wxKeyCode('X')))
			{
				m_brush_state = 0;
				SetBrush(3);
				if (tree_panel)
					tree_panel->SelectBrush(TreePanel::ID_BrushDesel);
				if (brush_dlg)
					brush_dlg->SelectBrush(BrushToolDlg::ID_BrushDesel);
				RefreshGL(11);
			}
			else
			{
				SetBrush(m_brush_state);
				RefreshGL(12);
			}
		}
		else if (!wxGetKeyState(WXK_SHIFT) &&
			!wxGetKeyState(wxKeyCode('Z')) &&
			!wxGetKeyState(wxKeyCode('X')))
		{
			if (wxGetMouseState().LeftIsDown())
				Segment();
			if (m_int_mode == 7)
				m_int_mode = 5;
			else
				m_int_mode = 1;
			m_paint_display = false;
			m_draw_brush = false;
			if (tree_panel)
				tree_panel->SelectBrush(0);
			if (brush_dlg)
				brush_dlg->SelectBrush(0);
			RefreshGL(13);

			if (m_prev_focus)
			{
				m_prev_focus->SetFocus();
				m_prev_focus = 0;
			}
		}
	}
}

//selection
void VRenderGLView::Pick()
{
	if (m_draw_all)
	{
		PickVolume();
		PickMesh();
	}
}

void VRenderGLView::PickMesh()
{
	int i;
	int nx = GetGLSize().x;
	int ny = GetGLSize().y;
	if (nx <= 0 || ny <= 0)
		return;
	wxPoint mouse_pos = ScreenToClient(wxGetMousePosition());
	if (mouse_pos.x<0 || mouse_pos.x >= nx ||
		mouse_pos.y <= 0 || mouse_pos.y>ny)
		return;

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
		flvr::TextureRenderer::framebuffer_manager_.framebuffer(
			flvr::FB_Pick_Int32_Float, nx, ny);
	if (pick_buffer)
		pick_buffer->bind();

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glScissor(mouse_pos.x, ny - mouse_pos.y, 1, 1);
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

	unsigned int choose = 0;
	if (pick_buffer)
		choose = pick_buffer->read_value(mouse_pos.x, ny - mouse_pos.y);
	glBindFramebuffer(GL_FRAMEBUFFER, m_cur_framebuffer);

	if (choose >0 && choose <= (int)m_md_pop_list.size())
	{
		MeshData* md = m_md_pop_list[choose - 1];
		if (md)
		{
			if (m_frame && m_frame->GetTree())
			{
				//m_frame->GetTree()->SetFocus();
				m_frame->GetTree()->Select(m_vrv->GetName(), md->GetName());
			}
			RefreshGL(27);
		}
	}
	else
	{
		if (m_frame && m_frame->GetCurSelType() == 3 &&
			m_frame->GetTree())
			m_frame->GetTree()->Select(m_vrv->GetName(), "");
	}
	m_mv_mat = mv_temp;
}

void VRenderGLView::PickVolume()
{
	int kmode = wxGetKeyState(WXK_CONTROL) ? 1 : 0;
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
		m_vp.SetVolumeData(vd);
		dist = m_vp.GetPointVolume(old_mouse_X, old_mouse_Y,
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

	if (picked_vd)
	{
		if (m_frame && m_frame->GetTree())
		{
			//m_frame->GetTree()->SetFocus();
			m_frame->GetTree()->Select(m_vrv->GetName(), picked_vd->GetName());
		}
		//update label selection
		SetCompSelection(ip, kmode);

		if (m_pick_lock_center)
			m_lock_center = pp;
	}
}

void VRenderGLView::SetCompSelection(fluo::Point& p, int mode)
{
	//update selection
	if (m_frame && m_frame->GetComponentDlg())
	{
		std::set<unsigned long long> ids;
		m_frame->GetComponentDlg()->GetAnalyzer()->GetCompsPoint(p, ids);
		m_frame->GetComponentDlg()->SetCompSelection(ids, mode);
	}
}

void VRenderGLView::OnIdle(wxIdleEvent& event)
{
	bool refresh = false;
	bool ref_stat = false;
	bool start_loop = true;
	bool set_focus = false;
	m_retain_finalbuffer = false;

	//check memory swap status
	if (flvr::TextureRenderer::get_mem_swap() &&
		flvr::TextureRenderer::get_start_update_loop() &&
		!flvr::TextureRenderer::get_done_update_loop())
	{
		if (flvr::TextureRenderer::active_view_ == m_vrv->m_id)
		{
			refresh = true;
			start_loop = false;
		}
	}

	if (m_capture_rotat ||
		m_capture_tsequ ||
		m_capture_param ||
		m_test_speed)
	{
		refresh = true;
		if (flvr::TextureRenderer::get_mem_swap() &&
			flvr::TextureRenderer::get_done_update_loop())
			m_pre_draw = true;
	}

	if (m_use_openvr)
	{
		event.RequestMore();
		refresh = true;
		//m_retain_finalbuffer = true;
	}

	if (m_frame && m_frame->GetBenchmark())
	{
		double fps = 1.0 / m_timer->average();
		wxString title = wxString(FLUORENDER_TITLE) +
			" " + wxString(VERSION_MAJOR_TAG) +
			"." + wxString(VERSION_MINOR_TAG) +
			" Benchmarking... FPS = " +
			wxString::Format("%.2f", fps);
		m_frame->SetTitle(title);

		refresh = true;
		if (flvr::TextureRenderer::get_mem_swap() &&
			flvr::TextureRenderer::get_done_update_loop())
			m_pre_draw = true;
	}

	//pin rotation center
	if (m_pin_rot_center && m_rot_center_dirty &&
		m_cur_vol && !m_free)
	{
		fluo::Point p, ip;
		int nx = GetGLSize().x;
		int ny = GetGLSize().y;
		int mode = 2;
		if (m_cur_vol->GetMode() == 1) mode = 1;
		m_vp.SetVolumeData(m_cur_vol);
		double dist = m_vp.GetPointVolume(nx / 2.0, ny / 2.0,
			mode, true, m_pin_pick_thresh, p, ip);
		if (dist <= 0.0)
			dist = m_vp.GetPointVolumeBox(
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
		m_rot_center_dirty = false;
		refresh = true;
	}

	wxPoint mouse_pos = wxGetMousePosition();
	wxRect view_reg = GetScreenRect();

	wxWindow *window = wxWindow::FindFocus();
	if (window == this && view_reg.Contains(mouse_pos))
	{
		UpdateBrushState();

		//draw_mask
		if (wxGetKeyState(wxKeyCode('V')) &&
			m_draw_mask)
		{
			m_draw_mask = false;
			refresh = true;
			set_focus = true;
		}
		if (!wxGetKeyState(wxKeyCode('V')) &&
			!m_draw_mask)
		{
			m_draw_mask = true;
			refresh = true;
		}

		//move view
		//left
		if (!m_move_left &&
			wxGetKeyState(WXK_CONTROL) &&
			wxGetKeyState(WXK_LEFT))
		{
			m_move_left = true;

			m_head = fluo::Vector(-m_transx, -m_transy, -m_transz);
			m_head.normalize();
			fluo::Vector side = fluo::Cross(m_up, m_head);
			fluo::Vector trans = -(side*(int(0.8*(m_ortho_right - m_ortho_left))));
			m_obj_transx += trans.x();
			m_obj_transy += trans.y();
			m_obj_transz += trans.z();
			//if (m_persp) SetSortBricks();
			refresh = true;
			set_focus = true;
		}
		if (m_move_left &&
			(!wxGetKeyState(WXK_CONTROL) ||
				!wxGetKeyState(WXK_LEFT)))
			m_move_left = false;
		//right
		if (!m_move_right &&
			wxGetKeyState(WXK_CONTROL) &&
			wxGetKeyState(WXK_RIGHT))
		{
			m_move_right = true;

			m_head = fluo::Vector(-m_transx, -m_transy, -m_transz);
			m_head.normalize();
			fluo::Vector side = fluo::Cross(m_up, m_head);
			fluo::Vector trans = side*(int(0.8*(m_ortho_right - m_ortho_left)));
			m_obj_transx += trans.x();
			m_obj_transy += trans.y();
			m_obj_transz += trans.z();
			//if (m_persp) SetSortBricks();
			refresh = true;
			set_focus = true;
		}
		if (m_move_right &&
			(!wxGetKeyState(WXK_CONTROL) ||
				!wxGetKeyState(WXK_RIGHT)))
			m_move_right = false;
		//up
		if (!m_move_up &&
			wxGetKeyState(WXK_CONTROL) &&
			wxGetKeyState(WXK_UP))
		{
			m_move_up = true;

			m_head = fluo::Vector(-m_transx, -m_transy, -m_transz);
			m_head.normalize();
			fluo::Vector trans = -m_up*(int(0.8*(m_ortho_top - m_ortho_bottom)));
			m_obj_transx += trans.x();
			m_obj_transy += trans.y();
			m_obj_transz += trans.z();
			//if (m_persp) SetSortBricks();
			refresh = true;
			set_focus = true;
		}
		if (m_move_up &&
			(!wxGetKeyState(WXK_CONTROL) ||
				!wxGetKeyState(WXK_UP)))
			m_move_up = false;
		//down
		if (!m_move_down &&
			wxGetKeyState(WXK_CONTROL) &&
			wxGetKeyState(WXK_DOWN))
		{
			m_move_down = true;

			m_head = fluo::Vector(-m_transx, -m_transy, -m_transz);
			m_head.normalize();
			fluo::Vector trans = m_up*(int(0.8*(m_ortho_top - m_ortho_bottom)));
			m_obj_transx += trans.x();
			m_obj_transy += trans.y();
			m_obj_transz += trans.z();
			//if (m_persp) SetSortBricks();
			refresh = true;
			set_focus = true;
		}
		if (m_move_down &&
			(!wxGetKeyState(WXK_CONTROL) ||
				!wxGetKeyState(WXK_DOWN)))
			m_move_down = false;

		//move time sequence
		//forward
		if (!m_tseq_forward &&
			(wxGetKeyState(wxKeyCode('d')) ||
				wxGetKeyState(WXK_SPACE)))
		{
			m_tseq_forward = true;
			if (m_frame && m_frame->GetMovieView())
				m_frame->GetMovieView()->UpFrame();
			refresh = true;
			set_focus = true;
		}
		if (m_tseq_forward &&
			!wxGetKeyState(wxKeyCode('d')) &&
			!wxGetKeyState(WXK_SPACE))
			m_tseq_forward = false;
		//backforward
		if (!m_tseq_backward &&
			wxGetKeyState(wxKeyCode('a')))
		{
			m_tseq_backward = true;
			if (m_frame && m_frame->GetMovieView())
				m_frame->GetMovieView()->DownFrame();
			refresh = true;
			set_focus = true;
		}
		if (m_tseq_backward &&
			!wxGetKeyState(wxKeyCode('a')))
			m_tseq_backward = false;

		//move clip
		//up
		if (!m_clip_up &&
			wxGetKeyState(wxKeyCode('s')))
		{
			m_clip_up = true;
			if (m_frame && m_frame->GetClippingView())
				m_frame->GetClippingView()->MoveLinkedClippingPlanes(1);
			refresh = true;
			set_focus = true;
		}
		if (m_clip_up &&
			!wxGetKeyState(wxKeyCode('s')))
			m_clip_up = false;
		//down
		if (!m_clip_down &&
			wxGetKeyState(wxKeyCode('w')))
		{
			m_clip_down = true;
			if (m_frame && m_frame->GetClippingView())
				m_frame->GetClippingView()->MoveLinkedClippingPlanes(0);
			refresh = true;
			set_focus = true;
		}
		if (m_clip_down &&
			!wxGetKeyState(wxKeyCode('w')))
			m_clip_down = false;

		//cell full
		if (!m_cell_full &&
			wxGetKeyState(wxKeyCode('f')))
		{
			m_cell_full = true;
			if (m_frame && m_frame->GetComponentDlg())
				m_frame->GetComponentDlg()->SelectFullComp();
			if (m_frame && m_frame->GetTraceDlg())
				m_frame->GetTraceDlg()->CellUpdate();
			refresh = true;
			set_focus = true;
		}
		if (m_cell_full &&
			!wxGetKeyState(wxKeyCode('f')))
			m_cell_full = false;
		//cell link
		if (!m_cell_link &&
			wxGetKeyState(wxKeyCode('l')))
		{
			m_cell_link = true;
			if (m_frame && m_frame->GetTraceDlg())
				m_frame->GetTraceDlg()->CellLink(false);
			refresh = true;
			set_focus = true;
		}
		if (m_cell_link &&
			!wxGetKeyState(wxKeyCode('l')))
			m_cell_link = false;
		//new cell id
		if (!m_cell_new_id &&
			wxGetKeyState(wxKeyCode('n')))
		{
			m_cell_new_id = true;
			if (m_frame && m_frame->GetTraceDlg())
				m_frame->GetTraceDlg()->CellNewID(false);
			refresh = true;
			set_focus = true;
		}
		if (m_cell_new_id &&
			!wxGetKeyState(wxKeyCode('n')))
			m_cell_new_id = false;
		//clear
		if (wxGetKeyState(wxKeyCode('c')) &&
			!m_clear_mask)
		{
			if (m_frame && m_frame->GetTree())
				m_frame->GetTree()->BrushClear();
			if (m_frame && m_frame->GetTraceDlg())
				m_frame->GetTraceDlg()->CompClear();
			m_clear_mask = true;
			refresh = true;
			set_focus = true;
		}
		if (!wxGetKeyState(wxKeyCode('c')) &&
			m_clear_mask)
			m_clear_mask = false;
		//save all masks
		if (wxGetKeyState(wxKeyCode('m')) &&
			!m_save_mask)
		{
			if (m_frame && m_frame->GetList())
				m_frame->GetList()->SaveAllMasks();
			m_save_mask = true;
			set_focus = true;
		}
		if (!wxGetKeyState(wxKeyCode('m')) &&
			m_save_mask)
			m_save_mask = false;
		//full screen
		if (wxGetKeyState(WXK_ESCAPE))
		{
			m_fullscreen_trigger.Start(10);
		}
		//brush size
		if (wxGetKeyState(wxKeyCode('[')))
		{
			ChangeBrushSize(-10);
			set_focus = true;
		}
		if (wxGetKeyState(wxKeyCode(']')))
		{
			ChangeBrushSize(10);
			set_focus = true;
		}
		//comp include
		if (wxGetKeyState(WXK_RETURN) &&
			!m_comp_include)
		{
			if (m_frame && m_frame->GetComponentDlg())
				m_frame->GetComponentDlg()->IncludeComps();
			m_comp_include = true;
			refresh = true;
			set_focus = true;
		}
		if (!wxGetKeyState(WXK_RETURN) &&
			m_comp_include)
			m_comp_include = false;
		//comp exclude
		if (wxGetKeyState(wxKeyCode('\\')) &&
			!m_comp_exclude)
		{
			if (m_frame && m_frame->GetComponentDlg())
				m_frame->GetComponentDlg()->ExcludeComps();
			m_comp_exclude = true;
			refresh = true;
			set_focus = true;
		}
		if (!wxGetKeyState(wxKeyCode('\\')) &&
			m_comp_exclude)
			m_comp_exclude = false;
		//ruler relax
		if (wxGetKeyState(wxKeyCode('r')) &&
			!m_ruler_relax)
		{
			if (m_frame && m_frame->GetMeasureDlg())
				m_frame->GetMeasureDlg()->Relax();
			m_ruler_relax = true;
			refresh = true;
			set_focus = true;
		}
		if (!wxGetKeyState(wxKeyCode('r')) &&
			m_ruler_relax)
			m_ruler_relax = false;

		//grow
		if ((m_int_mode == 10 ||
			m_int_mode == 12) &&
			wxGetMouseState().LeftIsDown() &&
			m_grow_on)
		{
			int sz = 5;
			if (m_frame && m_frame->GetSettingDlg())
				sz = m_frame->GetSettingDlg()->GetRulerSizeThresh();
			//event.RequestMore();
			m_selector.SetInitMask(2);
			Segment();
			m_selector.SetInitMask(3);
			if (m_int_mode == 12)
			{
				flrd::SegGrow sg(m_cur_vol);
				sg.SetRulerHandler(&m_ruler_handler);
				sg.SetIter(m_selector.GetIter()*3);
				sg.SetSizeThresh(sz);
				sg.Compute();
			}
			refresh = true;
			start_loop = true;
			//update
			if (m_frame)
			{
				if (m_paint_count && m_frame->GetBrushToolDlg())
					m_frame->GetBrushToolDlg()->Update(0);
				if (m_paint_colocalize && m_frame->GetColocalizationDlg())
					m_frame->GetColocalizationDlg()->Colocalize();
				if (m_int_mode == 12 && m_frame->GetMeasureDlg())
					m_frame->GetMeasureDlg()->GetSettings(this);
			}
		}

		//forced refresh
		if (wxGetKeyState(WXK_F5))
		{
			SetFocus();
			m_clear_buffer = true;
			m_updating = true;
			if (m_frame && m_frame->GetStatusBar())
				m_frame->GetStatusBar()->PushStatusText("Forced Refresh");
			wxSizeEvent e;
			OnResize(e);
			RefreshGL(14);
			if (m_frame && m_frame->GetStatusBar())
				m_frame->GetStatusBar()->PopStatusText();
			return;
		}
	}

#ifdef _WIN32
	//update ortho rotation
	if (!m_vrv->m_ortho_view_cmb->HasFocus())
	{
		if (m_q.AlmostEqual(fluo::Quaternion(0, sqrt(2.0) / 2.0, 0, sqrt(2.0) / 2.0)))
			m_vrv->m_ortho_view_cmb->Select(0);
		else if (m_q.AlmostEqual(fluo::Quaternion(0, -sqrt(2.0) / 2.0, 0, sqrt(2.0) / 2.0)))
			m_vrv->m_ortho_view_cmb->Select(1);
		else if (m_q.AlmostEqual(fluo::Quaternion(sqrt(2.0) / 2.0, 0, 0, sqrt(2.0) / 2.0)))
			m_vrv->m_ortho_view_cmb->Select(2);
		else if (m_q.AlmostEqual(fluo::Quaternion(-sqrt(2.0) / 2.0, 0, 0, sqrt(2.0) / 2.0)))
			m_vrv->m_ortho_view_cmb->Select(3);
		else if (m_q.AlmostEqual(fluo::Quaternion(0, 0, 0, 1)))
			m_vrv->m_ortho_view_cmb->Select(4);
		else if (m_q.AlmostEqual(fluo::Quaternion(0, -1, 0, 0)))
			m_vrv->m_ortho_view_cmb->Select(5);
		else
			m_vrv->m_ortho_view_cmb->Select(6);
	}
#endif

#if defined(_WIN32) && defined(USE_XINPUT)
	//xinput controller
	if (m_controller->IsConnected())
	{
		XINPUT_STATE xstate = m_controller->GetState();
		double dzone = 0.2;
		double sclr = 15.0;
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

		int nx = GetGLSize().x;
		int ny = GetGLSize().y;
		//horizontal move
		if (leftx != 0.0)
		{
			m_head = fluo::Vector(-m_transx, -m_transy, -m_transz);
			m_head.normalize();
			fluo::Vector side = fluo::Cross(m_up, m_head);
			fluo::Vector trans = side * (leftx*sclr*(m_ortho_right - m_ortho_left) / double(nx));
			m_obj_transx += trans.x();
			m_obj_transy += trans.y();
			m_obj_transz += trans.z();
			m_interactive = true;
			m_rot_center_dirty = true;
			refresh = true;
		}
		//zoom/dolly
		if (lefty != 0.0)
		{
			double delta = lefty * sclr / (double)ny;
			m_scale_factor += m_scale_factor * delta;
			m_vrv->UpdateScaleFactor(false);
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
			m_interactive = true;
			refresh = true;
		}
		//rotate
		if (rghtx != 0.0 || rghty != 0.0)
		{
			fluo::Quaternion q_delta = Trackball(rghtx*sclr, rghty*sclr);
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
			wxString str = wxString::Format("%.1f", m_rotx);
			m_vrv->m_x_rot_text->ChangeValue(str);
			str = wxString::Format("%.1f", m_roty);
			m_vrv->m_y_rot_text->ChangeValue(str);
			str = wxString::Format("%.1f", m_rotz);
			m_vrv->m_z_rot_text->ChangeValue(str);
			if (!m_vrv->m_rot_slider)
			{
				m_vrv->m_x_rot_sldr->SetThumbPosition(int(m_rotx));
				m_vrv->m_y_rot_sldr->SetThumbPosition(int(m_roty));
				m_vrv->m_z_rot_sldr->SetThumbPosition(int(m_rotz));
			}
			m_interactive = true;
			refresh = true;
		}
		//pan
		if (px != 0 || py != 0)
		{
			m_head = fluo::Vector(-m_transx, -m_transy, -m_transz);
			m_head.normalize();
			fluo::Vector side = fluo::Cross(m_up, m_head);
			fluo::Vector trans =
				side * (double(px)*(m_ortho_right - m_ortho_left) / double(nx)) +
				m_up * (double(py)*(m_ortho_top - m_ortho_bottom) / double(ny));
			m_obj_transx += trans.x();
			m_obj_transy += trans.y();
			m_obj_transz += trans.z();
			m_interactive = true;
			m_rot_center_dirty = true;
			refresh = true;
		}
	}
#endif

	if (set_focus)
		SetFocus();
	if (refresh)
	{
		m_clear_buffer = true;
		m_updating = true;
		RefreshGL(15, ref_stat, start_loop);
	}
}

void VRenderGLView::OnKeyDown(wxKeyEvent& event)
{
	event.Skip();
}

void VRenderGLView::OnQuitFscreen(wxTimerEvent& event)
{
	m_fullscreen_trigger.Stop();
	if (!m_frame || !m_vrv)
		return;

	m_full_screen = false;
	if (m_benchmark)
	{
		if (m_vrv->m_full_frame)
			m_vrv->m_full_frame->Hide();
		if (m_frame)
			m_frame->Close();
	}
	else if (GetParent() == m_vrv->m_full_frame)
	{
		Reparent(m_vrv);
		m_vrv->m_view_sizer->Add(this, 1, wxEXPAND);
		m_vrv->Layout();
		m_vrv->m_full_frame->Hide();
		if (m_frame)
		{
#ifdef _WIN32
			if (m_frame->GetSettingDlg() &&
				!m_frame->GetSettingDlg()->GetShowCursor())
				ShowCursor(true);
#endif
			m_frame->Iconize(false);
			m_frame->SetFocus();
			m_frame->Raise();
			m_frame->Show();
		}
		RefreshGL(40);
	}
}

void VRenderGLView::OnClose(wxCloseEvent &event)
{
	if (m_full_screen)
	{
		m_fullscreen_trigger.Start(10);
		event.Veto();
	}
}

void VRenderGLView::Set3DRotCapture(double start_angle,
	double end_angle,
	double step,
	int frames,
	int rot_axis,
	wxString &cap_file,
	bool rewind,
	int len)
{
	double rv[3];
	GetRotations(rv[0], rv[1], rv[2]);

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

void VRenderGLView::Set3DBatCapture(wxString &cap_file, int begin_frame, int end_frame)
{
	m_cap_file = cap_file;
	m_begin_frame = begin_frame;
	m_end_frame = end_frame;
	m_capture_bat = true;
	m_capture = true;

	if (!m_cap_file.IsEmpty() && m_total_frames>1)
	{
		wxString new_folder = wxPathOnly(m_cap_file)
			+ GETSLASH() + m_bat_folder + "_folder";
		MkDirW(new_folder.ToStdWstring());
	}
}

void VRenderGLView::Set4DSeqCapture(wxString &cap_file, int begin_frame, int end_frame, bool rewind)
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

void VRenderGLView::SetParamCapture(wxString &cap_file, int begin_frame, int end_frame, bool rewind)
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

void VRenderGLView::SetParams(double t)
{
	if (!m_vrv)
		return;
	if (!m_frame)
		return;
	ClippingView* clip_view = m_frame->GetClippingView();
	Interpolator *interpolator = m_frame->GetInterpolator();
	if (!interpolator)
		return;
	FlKeyCode keycode;
	keycode.l0 = 1;
	keycode.l0_name = m_vrv->GetName();

	for (int i = 0; i<GetAllVolumeNum(); i++)
	{
		VolumeData* vd = GetAllVolumeData(i);
		if (!vd) continue;

		keycode.l1 = 2;
		keycode.l1_name = vd->GetName();

		//display
		keycode.l2 = 0;
		keycode.l2_name = "display";
		bool bval;
		if (interpolator->GetBoolean(keycode, t, bval))
			vd->SetDisp(bval);

		//clipping planes
		vector<fluo::Plane*> *planes = vd->GetVR()->get_planes();
		if (!planes) continue;
		if (planes->size() != 6) continue;
		fluo::Plane *plane = 0;
		double val = 0;
		//x1
		plane = (*planes)[0];
		keycode.l2 = 0;
		keycode.l2_name = "x1_val";
		if (interpolator->GetDouble(keycode, t, val))
			plane->ChangePlane(fluo::Point(abs(val), 0.0, 0.0),
				fluo::Vector(1.0, 0.0, 0.0));
		//x2
		plane = (*planes)[1];
		keycode.l2 = 0;
		keycode.l2_name = "x2_val";
		if (interpolator->GetDouble(keycode, t, val))
			plane->ChangePlane(fluo::Point(abs(val), 0.0, 0.0),
				fluo::Vector(-1.0, 0.0, 0.0));
		//y1
		plane = (*planes)[2];
		keycode.l2 = 0;
		keycode.l2_name = "y1_val";
		if (interpolator->GetDouble(keycode, t, val))
			plane->ChangePlane(fluo::Point(0.0, abs(val), 0.0),
				fluo::Vector(0.0, 1.0, 0.0));
		//y2
		plane = (*planes)[3];
		keycode.l2 = 0;
		keycode.l2_name = "y2_val";
		if (interpolator->GetDouble(keycode, t, val))
			plane->ChangePlane(fluo::Point(0.0, abs(val), 0.0),
				fluo::Vector(0.0, -1.0, 0.0));
		//z1
		plane = (*planes)[4];
		keycode.l2 = 0;
		keycode.l2_name = "z1_val";
		if (interpolator->GetDouble(keycode, t, val))
			plane->ChangePlane(fluo::Point(0.0, 0.0, abs(val)),
				fluo::Vector(0.0, 0.0, 1.0));
		//z2
		plane = (*planes)[5];
		keycode.l2 = 0;
		keycode.l2_name = "z2_val";
		if (interpolator->GetDouble(keycode, t, val))
			plane->ChangePlane(fluo::Point(0.0, 0.0, abs(val)),
				fluo::Vector(0.0, 0.0, -1.0));
		//t
		double frame;
		keycode.l2 = 0;
		keycode.l2_name = "frame";
		if (interpolator->GetDouble(keycode, t, frame))
			UpdateVolumeData(int(frame + 0.5), vd);
		//primary color
		fluo::Color pc;
		keycode.l2 = 0;
		keycode.l2_name = "color";
		if (interpolator->GetColor(keycode, t, pc))
			vd->SetColor(pc);
	}

	bool bx, by, bz;
	//for the view
	keycode.l1 = 1;
	keycode.l1_name = m_vrv->GetName();
	//translation
	double tx, ty, tz;
	keycode.l2 = 0;
	keycode.l2_name = "translation_x";
	bx = interpolator->GetDouble(keycode, t, tx);
	keycode.l2_name = "translation_y";
	by = interpolator->GetDouble(keycode, t, ty);
	keycode.l2_name = "translation_z";
	bz = interpolator->GetDouble(keycode, t, tz);
	if (bx && by && bz)
		SetTranslations(tx, ty, tz);
	//centers
	keycode.l2_name = "center_x";
	bx = interpolator->GetDouble(keycode, t, tx);
	keycode.l2_name = "center_y";
	by = interpolator->GetDouble(keycode, t, ty);
	keycode.l2_name = "center_z";
	bz = interpolator->GetDouble(keycode, t, tz);
	if (bx && by && bz)
		SetCenters(tx, ty, tz);
	//obj translation
	keycode.l2_name = "obj_trans_x";
	bx = interpolator->GetDouble(keycode, t, tx);
	keycode.l2_name = "obj_trans_y";
	by = interpolator->GetDouble(keycode, t, ty);
	keycode.l2_name = "obj_trans_z";
	bz = interpolator->GetDouble(keycode, t, tz);
	if (bx && by && bz)
		SetObjTrans(tx, ty, tz);
	//scale
	double scale;
	keycode.l2_name = "scale";
	if (interpolator->GetDouble(keycode, t, scale))
	{
		m_scale_factor = scale;
		m_vrv->UpdateScaleFactor(false);
	}
	//rotation
	keycode.l2 = 0;
	keycode.l2_name = "rotation";
	fluo::Quaternion q;
	if (interpolator->GetQuaternion(keycode, t, q))
	{
		m_q = q;
		q *= -m_zq;
		double rotx, roty, rotz;
		q.ToEuler(rotx, roty, rotz);
		SetRotations(rotx, roty, rotz, true);
	}
	//intermixing mode
	keycode.l2_name = "volmethod";
	int ival;
	if (interpolator->GetInt(keycode, t, ival))
		SetVolMethod(ival);
	//perspective angle
	keycode.l2_name = "aov";
	double aov;
	if (interpolator->GetDouble(keycode, t, aov))
	{
		if (aov <= 10)
		{
			SetPersp(false);
			m_vrv->m_aov_text->ChangeValue("Ortho");
			m_vrv->m_aov_sldr->SetValue(10);
		}
		else
		{
			SetPersp(true);
			SetAov(aov);
		}
	}

	if (m_frame && clip_view)
		clip_view->SetVolumeData(m_frame->GetCurSelVol());
	if (m_frame)
	{
		m_frame->UpdateTree(m_cur_vol ? m_cur_vol->GetName() : wxString(""));
		int index = interpolator->GetKeyIndexFromTime(t);
		m_frame->GetRecorderDlg()->SetSelection(index);
	}
	SetVolPopDirty();
}

void VRenderGLView::ResetMovieAngle()
{
	double rv[3];
	GetRotations(rv[0], rv[1], rv[2]);
	rv[m_rot_axis] = m_init_angle;
	SetRotations(rv[0], rv[1], rv[2]);

	m_capture = false;
	m_capture_rotat = false;

	RefreshGL(16);
}

void VRenderGLView::StopMovie()
{
	m_capture = false;
	m_capture_rotat = false;
	m_capture_tsequ = false;
	m_capture_param = false;
}

void VRenderGLView::Get4DSeqRange(int &start_frame, int &end_frame)
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
}

void VRenderGLView::UpdateVolumeData(int frame, VolumeData* vd)
{
	if (vd && vd->GetReader())
	{
		BaseReader* reader = vd->GetReader();
		bool clear_pool = false;

		if (vd->GetCurTime() != frame)
		{
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
				//update rulers
				if (m_frame && m_frame->GetMeasureDlg())
					m_frame->GetMeasureDlg()->UpdateList();
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

				//update rulers
				if (m_frame && m_frame->GetMeasureDlg())
					m_frame->GetMeasureDlg()->UpdateList();

				clear_pool = true;
			}
		}

		if (clear_pool && vd->GetVR())
			vd->GetVR()->clear_tex_pool();
	}
}

void VRenderGLView::ReloadVolumeData(int frame)
{
	int i, j;
	vector<BaseReader*> reader_list;
	m_bat_folder = "";

	for (i = 0; i < (int)m_vd_pop_list.size(); i++)
	{
		VolumeData* vd = m_vd_pop_list[i];
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
				wxString data_name = wxString(reader->GetDataName());
				if (i > 0)
					m_bat_folder += "_";
				m_bat_folder += data_name;

				int chan_num = 0;
				if (data_name.Find("_1ch") != -1)
					chan_num = 1;
				else if (data_name.Find("_2ch") != -1)
					chan_num = 2;
				if (chan_num > 0 && vd->GetCurChannel() >= chan_num)
					vd->SetDisp(false);
				else
					vd->SetDisp(true);

				if (reader->GetChanNum() > 1)
					data_name += wxString::Format("_%d", vd->GetCurChannel() + 1);
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

				wxString data_name = wxString(reader->GetDataName());
				if (i > 0)
					m_bat_folder += "_";
				m_bat_folder += data_name;

				int chan_num = 0;
				if (data_name.Find("_1ch") != -1)
					chan_num = 1;
				else if (data_name.Find("_2ch") != -1)
					chan_num = 2;
				if (chan_num > 0 && vd->GetCurChannel() >= chan_num)
					vd->SetDisp(false);
				else
					vd->SetDisp(true);

				if (reader->GetChanNum() > 1)
					data_name += wxString::Format("_%d", vd->GetCurChannel() + 1);
				vd->SetName(data_name);
				vd->SetPath(wxString(reader->GetPathName()));
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

	if (m_frame)
	{
		m_frame->UpdateList();
		m_frame->UpdateTree(
			m_frame->GetCurSelVol() ?
			m_frame->GetCurSelVol()->GetName() :
			wxString(""));
	}
}

void VRenderGLView::Set4DSeqFrame(int frame, int start_frame, int end_frame, bool rewind)
{
	//skip update if frame num unchanged
	bool update = m_tseq_cur_num == frame ? false : true;
	//compute frame number
	m_begin_frame = start_frame;
	m_end_frame = end_frame;
	m_total_frames = std::abs(end_frame - start_frame + 1);

	//save currently selected volume
	m_cur_vol_save = m_cur_vol;

	//run pre-change script
	if (update && m_run_script)
		m_scriptor.Run4DScript(flrd::ScriptProc::TM_ALL_PRE, m_script_file, rewind);

	//change time frame
	m_tseq_prv_num = m_tseq_cur_num;
	m_tseq_cur_num = frame;

	if (update)
	for (auto i : m_vd_pop_list)
		UpdateVolumeData(frame, i);

	//run post-change script
	if (update && m_run_script)
		m_scriptor.Run4DScript(flrd::ScriptProc::TM_ALL_POST_REWIND, m_script_file, rewind);

	//restore currently selected volume
	m_cur_vol = m_cur_vol_save;
	m_selector.SetVolume(m_cur_vol);
	m_calculator.SetVolumeA(m_cur_vol);

	RefreshGL(17);
}

void VRenderGLView::Get3DBatRange(int &start_frame, int &end_frame)
{
	m_bat_folder = "";

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

			if (i > 0)
				m_bat_folder += "_";
			m_bat_folder += wxString(reader->GetDataName());

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
}

void VRenderGLView::Set3DBatFrame(int frame, int start_frame, int end_frame, bool rewind)
{
	//skip update if frame num unchanged
	bool update = m_tseq_cur_num == frame ? false : true;
	//compute frame number
	m_begin_frame = start_frame;
	m_end_frame = end_frame;
	m_total_frames = std::abs(end_frame - start_frame + 1);

	//save currently selected volume
	VolumeData* cur_vd_save = m_cur_vol;

	//run pre-change script
	if (update && m_run_script)
		m_scriptor.Run4DScript(flrd::ScriptProc::TM_ALL_PRE, m_script_file, rewind);

	//change time frame
	m_tseq_prv_num = m_tseq_cur_num;
	m_tseq_cur_num = frame;

	if (update)
		ReloadVolumeData(frame);

	//run post-change script
	if (update && m_run_script)
		m_scriptor.Run4DScript(flrd::ScriptProc::TM_ALL_POST, m_script_file, rewind);

	//restore currently selected volume
	m_cur_vol = cur_vd_save;
	m_selector.SetVolume(m_cur_vol);
	m_calculator.SetVolumeA(m_cur_vol);

	RefreshGL(18);
}

//pre-draw processings
void VRenderGLView::PreDraw()
{
	//skip if not done with loop
	if (flvr::TextureRenderer::get_mem_swap())
	{
		if (m_pre_draw)
			m_pre_draw = false;
		else
			return;
	}
}

//read pixels
void VRenderGLView::ReadPixels(
	int chann, bool fp32,
	int &x, int &y, int &w, int &h,
	void** image)
{
	if (m_draw_frame)
	{
		if (m_enlarge)
		{
			x = m_frame_x * m_enlarge_scale;
			y = m_frame_y * m_enlarge_scale;
			w = m_frame_w * m_enlarge_scale;
			h = m_frame_h * m_enlarge_scale;
		}
		else
		{
			x = m_frame_x;
			y = m_frame_y;
			w = m_frame_w;
			h = m_frame_h;
		}
	}
	else
	{
		x = 0;
		y = 0;
		w = GetGLSize().x;
		h = GetGLSize().y;
	}

	if (m_enlarge || fp32)
	{
		glActiveTexture(GL_TEXTURE0);
		flvr::Framebuffer* final_buffer =
			flvr::TextureRenderer::framebuffer_manager_.framebuffer(
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
			flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHDR_BLEND_BRIGHT_BACKGROUND_HDR);
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

void VRenderGLView::PostDraw()
{
	//skip if not done with loop
	if (flvr::TextureRenderer::get_mem_swap() &&
		flvr::TextureRenderer::get_start_update_loop() &&
		!flvr::TextureRenderer::get_done_update_loop())
		return;

	//output animations
	if (m_capture && !m_cap_file.IsEmpty())
	{
		//capture
		wxString outputfilename = m_cap_file;

		int chann = VRenderFrame::GetSaveAlpha() ? 4 : 3;
		bool fp32 = VRenderFrame::GetSaveFloat();
		float dpi = VRenderFrame::GetDpi();
		int x, y, w, h;
		void* image = 0;
		ReadPixels(chann, fp32, x, y, w, h, &image);

		string str_fn = outputfilename.ToStdString();
		TIFF *out = TIFFOpen(str_fn.c_str(), "wb");
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
		if (VRenderFrame::GetCompression())
			TIFFSetField(out, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
		//dpi
		TIFFSetField(out, TIFFTAG_XRESOLUTION, dpi);
		TIFFSetField(out, TIFFTAG_YRESOLUTION, dpi);
		TIFFSetField(out, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);

		tsize_t linebytes = chann * w * (fp32?4:1);
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

void VRenderGLView::ResetEnlarge()
{
	//skip if not done with loop
	if (flvr::TextureRenderer::get_mem_swap() &&
		flvr::TextureRenderer::get_start_update_loop() &&
		!flvr::TextureRenderer::get_done_update_loop())
		return;
	if (m_keep_enlarge)
		return;
	m_enlarge = false;
	flvr::TextRenderer::text_texture_manager_.SetEnlargeScale(1);
	RefreshGL(19);
}

//draw
void VRenderGLView::OnDraw(wxPaintEvent& event)
{
	if (!m_refresh)
		m_retain_finalbuffer = true;
	else
		m_refresh = false;
	ForceDraw();
}

void VRenderGLView::ForceDraw()
{
#ifdef _WIN32
	if (!m_set_gl)
	{
		SetCurrent(*m_glRC);
		m_set_gl = true;
		if (m_frame)
		{
			for (int i = 0; i< m_frame->GetViewNum(); i++)
			{
				VRenderGLView* view = m_frame->GetView(i);
				if (view && view != this)
				{
					view->m_set_gl = false;
				}
			}
		}
	}
#endif
#if defined(_DARWIN) || defined(__linux__)
	SetCurrent(*m_glRC);
#endif
	Init();
	wxPaintDC dc(this);

	if (m_resize)
		m_retain_finalbuffer = false;

	int nx, ny;
	GetRenderSize(nx, ny);

	PopMeshList();
	if (m_md_pop_list.size()>0)
		m_draw_type = 2;
	else
		m_draw_type = 1;

	m_drawing = true;
	PreDraw();

	if (m_enable_vr)
	{
		PrepVRBuffer();
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
		DrawInfo(nx, ny);

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


	if (flvr::TextureRenderer::get_invalidate_tex())
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
	if (m_enable_vr)
	{
		if (m_vr_eye_idx)
		{
			DrawVRBuffer();
			m_vr_eye_idx = 0;
			SwapBuffers();
		}
		else
		{
			m_vr_eye_idx = 1;
			RefreshGL(99);
		}
	}
	else
		SwapBuffers();

	m_timer->sample();
	m_drawing = false;

	DBGPRINT(L"buffer swapped\t%d\n", m_interactive);

	if (m_resize)
		m_resize = false;

	if (m_enlarge)
		ResetEnlarge();

	if (m_linked_rot)
	{
		if (!m_master_linked_view ||
			this != m_master_linked_view)
			return;

		if (m_frame)
		{
			for (int i = 0; i< m_frame->GetViewNum(); i++)
			{
				VRenderGLView* view = m_frame->GetView(i);
				if (view && view != this)
				{
					view->SetRotations(m_rotx, m_roty, m_rotz, true);
					view->RefreshGL(39);
				}
			}
		}

		m_master_linked_view = 0;
	}
}

void VRenderGLView::SetRadius(double r)
{
	m_radius = r;
}

void VRenderGLView::SetCenter()
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
		vector<fluo::Plane*> *planes = vr->get_planes();
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

double VRenderGLView::Get121ScaleFactor()
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

void VRenderGLView::SetScale121()
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
		double spcx, spcy, spcz;
		if (vd)
			vd->GetSpacings(spcx, spcy, spcz, vd->GetLevel());
		if (spcx > 0.0)
			value /= spcx;
		}
		break;
	}

	wxString str = wxString::Format("%.0f", value*100.0);
	m_vrv->m_scale_factor_sldr->SetValue(value * 100);
	m_vrv->m_scale_factor_text->ChangeValue(str);

	//SetSortBricks();

	RefreshGL(21);
}

void VRenderGLView::SetPinRotCenter(bool pin)
{
	m_pin_rot_center = pin;
	if (pin)
		m_rot_center_dirty = true;
}

void VRenderGLView::SetPersp(bool persp)
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
		m_vrv->UpdateScaleFactor(false);
		//wxString str = wxString::Format("%.0f", m_scale_factor*100.0);
		//m_vrv->m_scale_factor_sldr->SetValue(m_scale_factor*100);
		//m_vrv->m_scale_factor_text->ChangeValue(str);
		m_vrv->m_options_toolbar->ToggleTool(VRenderView::ID_FreeChk, false);

		SetRotations(m_rotx, m_roty, m_rotz);
	}
	//SetSortBricks();
}

void VRenderGLView::SetFree(bool free)
{
	m_free = free;
	if (m_vrv->m_options_toolbar->GetToolState(VRenderView::ID_FreeChk) != m_free)
		m_vrv->m_options_toolbar->ToggleTool(VRenderView::ID_FreeChk, m_free);
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
		m_vrv->m_aov_text->ChangeValue(wxString::Format("%d", int(m_aov)));
		m_vrv->m_aov_sldr->SetValue(m_aov);
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
		m_vrv->UpdateScaleFactor(false);
		//wxString str = wxString::Format("%.0f", m_scale_factor*100.0);
		//m_vrv->m_scale_factor_sldr->SetValue(m_scale_factor*100);
		//m_vrv->m_scale_factor_text->ChangeValue(str);

		SetRotations(m_rotx, m_roty, m_rotz);
	}
	//SetSortBricks();
}

void VRenderGLView::SetAov(double aov)
{
	//view has been changed, sort bricks
	//SetSortBricks();

	m_aov = aov;
	if (m_persp)
	{
		m_vrv->m_aov_text->ChangeValue(wxString::Format("%d", int(m_aov)));
		m_vrv->m_aov_sldr->SetValue(int(m_aov));
	}
}

void VRenderGLView::SetVolMethod(int method)
{
	//get the volume list m_vd_pop_list
	PopVolumeList();

	m_vol_method = method;

	//ui
	m_vrv->m_options_toolbar->ToggleTool(VRenderView::ID_VolumeSeqRd, false);
	m_vrv->m_options_toolbar->ToggleTool(VRenderView::ID_VolumeMultiRd, false);
	m_vrv->m_options_toolbar->ToggleTool(VRenderView::ID_VolumeCompRd, false);
	switch (m_vol_method)
	{
	case VOL_METHOD_SEQ:
		m_vrv->m_options_toolbar->ToggleTool(VRenderView::ID_VolumeSeqRd, true);
		m_vrv->m_options_toolbar->SetToolNormalBitmap(
			VRenderView::ID_VolumeSeqRd, wxGetBitmapFromMemory(layers));
		m_vrv->m_options_toolbar->SetToolNormalBitmap(
			VRenderView::ID_VolumeMultiRd, wxGetBitmapFromMemory(depth_off));
		m_vrv->m_options_toolbar->SetToolNormalBitmap(
			VRenderView::ID_VolumeCompRd, wxGetBitmapFromMemory(composite_off));
		break;
	case VOL_METHOD_MULTI:
		m_vrv->m_options_toolbar->ToggleTool(VRenderView::ID_VolumeMultiRd, true);
		m_vrv->m_options_toolbar->SetToolNormalBitmap(
			VRenderView::ID_VolumeSeqRd, wxGetBitmapFromMemory(layers_off));
		m_vrv->m_options_toolbar->SetToolNormalBitmap(
			VRenderView::ID_VolumeMultiRd, wxGetBitmapFromMemory(depth));
		m_vrv->m_options_toolbar->SetToolNormalBitmap(
			VRenderView::ID_VolumeCompRd, wxGetBitmapFromMemory(composite_off));
		break;
	case VOL_METHOD_COMP:
		m_vrv->m_options_toolbar->ToggleTool(VRenderView::ID_VolumeCompRd, true);
		m_vrv->m_options_toolbar->SetToolNormalBitmap(
			VRenderView::ID_VolumeSeqRd, wxGetBitmapFromMemory(layers_off));
		m_vrv->m_options_toolbar->SetToolNormalBitmap(
			VRenderView::ID_VolumeMultiRd, wxGetBitmapFromMemory(depth_off));
		m_vrv->m_options_toolbar->SetToolNormalBitmap(
			VRenderView::ID_VolumeCompRd, wxGetBitmapFromMemory(composite));
		break;
	}
}

VolumeData* VRenderGLView::GetAllVolumeData(int index)
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

VolumeData* VRenderGLView::GetDispVolumeData(int index)
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

MeshData* VRenderGLView::GetMeshData(int index)
{
	if (GetMeshNum() <= 0)
		return 0;

	PopMeshList();

	if (index >= 0 && index<(int)m_md_pop_list.size())
		return m_md_pop_list[index];
	else
		return 0;
}

VolumeData* VRenderGLView::GetVolumeData(wxString &name)
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

MeshData* VRenderGLView::GetMeshData(wxString &name)
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

Annotations* VRenderGLView::GetAnnotations(wxString &name)
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

DataGroup* VRenderGLView::GetGroup(wxString &name)
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

DataGroup* VRenderGLView::GetGroup(int index)
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

DataGroup* VRenderGLView::GetGroup(VolumeData* vd)
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

int VRenderGLView::GetAny()
{
	PopVolumeList();
	PopMeshList();
	return m_vd_pop_list.size() + m_md_pop_list.size();
}

int VRenderGLView::GetDispVolumeNum()
{
	//get the volume list m_vd_pop_list
	PopVolumeList();

	return m_vd_pop_list.size();
}

int VRenderGLView::GetAllVolumeNum()
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

int VRenderGLView::GetMeshNum()
{
	PopMeshList();
	return m_md_pop_list.size();
}

DataGroup* VRenderGLView::AddVolumeData(VolumeData* vd, wxString group_name)
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
		wxString group_name = AddGroup("");
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
		fluo::Color gamma = group->GetGamma();
		vd->SetGamma(gamma);
		fluo::Color brightness = group->GetBrightness();
		vd->SetBrightness(brightness);
		fluo::Color hdr = group->GetHdr();
		vd->SetHdr(hdr);
		bool sync_r = group->GetSyncR();
		vd->SetSyncR(sync_r);
		bool sync_g = group->GetSyncG();
		vd->SetSyncG(sync_g);
		bool sync_b = group->GetSyncB();
		vd->SetSyncB(sync_b);

		if (m_frame)
		{
			m_frame->GetAdjustView()->SetVolumeData(vd);
			m_frame->GetAdjustView()->SetGroupLink(group);
		}
	}

	m_vd_pop_dirty = true;

	if (m_frame)
	{
		AdjustView* adjust_view = m_frame->GetAdjustView();
		if (adjust_view)
		{
			adjust_view->SetGroupLink(group);
			adjust_view->UpdateSync();
		}
	}

	m_load_update = true;

	return group;
}

void VRenderGLView::AddMeshData(MeshData* md)
{
	m_layer_list.push_back(md);
	m_md_pop_dirty = true;
}

void VRenderGLView::AddAnnotations(Annotations* ann)
{
	m_layer_list.push_back(ann);
}

void VRenderGLView::ReplaceVolumeData(wxString &name, VolumeData *dst)
{
	int i, j;

	bool found = false;
	DataGroup* group = 0;

	if (!m_frame) return;
	DataManager *dm = m_frame->GetDataManager();
	if (!dm) return;

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
				m_loader.RemoveBrickVD(vd);
				vd->GetVR()->clear_tex_current();
				m_layer_list[i] = dst;
				m_vd_pop_dirty = true;
				found = true;
				dm->RemoveVolumeData(name);
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
					m_loader.RemoveBrickVD(vd);
					vd->GetVR()->clear_tex_current();
					tmpgroup->ReplaceVolumeData(j, dst);
					m_vd_pop_dirty = true;
					found = true;
					group = tmpgroup;
					dm->RemoveVolumeData(name);
					break;
				}
			}
		}
		break;
		}
	}

	if (found)
	{
		if (m_frame)
		{
			AdjustView* adjust_view = m_frame->GetAdjustView();
			if (adjust_view)
			{
				adjust_view->SetVolumeData(dst);
				if (!group) adjust_view->SetGroupLink(group);
				adjust_view->UpdateSync();
			}
		}
	}
}

void VRenderGLView::RemoveVolumeData(wxString &name)
{
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

void VRenderGLView::RemoveVolumeDataDup(wxString &name)
{
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

void VRenderGLView::RemoveMeshData(wxString &name)
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

void VRenderGLView::RemoveAnnotations(wxString &name)
{
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

void VRenderGLView::RemoveGroup(wxString &name)
{
	int i, j;
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
			{
				for (j = group->GetVolumeNum() - 1; j >= 0; j--)
				{
					VolumeData* vd = group->GetVolumeData(j);
					if (vd)
					{
						group->RemoveVolumeData(j);
						//if add back to view
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
				for (j = group->GetMeshNum() - 1; j >= 0; j--)
				{
					MeshData* md = group->GetMeshData(j);
					if (md)
					{
						group->RemoveMeshData(j);
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
void VRenderGLView::Isolate(int type, wxString name)
{
	for (int i = 0; i<(int)m_layer_list.size(); i++)
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

//move layer of the same level within this view
//source is after the destination
void VRenderGLView::MoveLayerinView(wxString &src_name, wxString &dst_name)
{
	int i, src_index;
	TreeLayer* src = 0;
	for (i = 0; i<(int)m_layer_list.size(); i++)
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
	for (i = 0; i<(int)m_layer_list.size(); i++)
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

void VRenderGLView::ShowAll()
{
	for (unsigned int i = 0; i<m_layer_list.size(); ++i)
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

//move layer (volume) of the same level within the given group
//source is after the destination
void VRenderGLView::MoveLayerinGroup(wxString &group_name, wxString &src_name, wxString &dst_name)
{
	DataGroup* group = GetGroup(group_name);
	if (!group)
		return;

	VolumeData* src_vd = 0;
	int i, src_index;
	for (i = 0; i<group->GetVolumeNum(); i++)
	{
		wxString name = group->GetVolumeData(i)->GetName();
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
		wxString name = group->GetVolumeData(i)->GetName();
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
void VRenderGLView::MoveLayertoView(wxString &group_name, wxString &src_name, wxString &dst_name)
{
	DataGroup* group = GetGroup(group_name);
	if (!group)
		return;

	VolumeData* src_vd = 0;
	int i;
	for (i = 0; i<group->GetVolumeNum(); i++)
	{
		wxString name = group->GetVolumeData(i)->GetName();
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
		for (i = 0; i<(int)m_layer_list.size(); i++)
		{
			wxString name = m_layer_list[i]->GetName();
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
void VRenderGLView::MoveLayertoGroup(wxString &group_name, wxString &src_name, wxString &dst_name)
{
	VolumeData* src_vd = 0;
	int i;

	for (i = 0; i<(int)m_layer_list.size(); i++)
	{
		wxString name = m_layer_list[i]->GetName();
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
	if (group->GetVolumeNum() == 0 || dst_name == "")
	{
		group->InsertVolumeData(0, src_vd);
	}
	else
	{
		for (i = 0; i<group->GetVolumeNum(); i++)
		{
			wxString name = group->GetVolumeData(i)->GetName();
			if (name == dst_name)
			{
				group->InsertVolumeData(i, src_vd);
				break;
			}
		}
	}

	//set the 2d adjustment settings of the volume the same as the group
	fluo::Color gamma = group->GetGamma();
	src_vd->SetGamma(gamma);
	fluo::Color brightness = group->GetBrightness();
	src_vd->SetBrightness(brightness);
	fluo::Color hdr = group->GetHdr();
	src_vd->SetHdr(hdr);
	bool sync_r = group->GetSyncR();
	src_vd->SetSyncR(sync_r);
	bool sync_g = group->GetSyncG();
	src_vd->SetSyncG(sync_g);
	bool sync_b = group->GetSyncB();
	src_vd->SetSyncB(sync_b);

	m_vd_pop_dirty = true;
	m_md_pop_dirty = true;
}

//move layer (volume from one group to another different group
//sourece is after the destination
void VRenderGLView::MoveLayerfromtoGroup(wxString &src_group_name, wxString &dst_group_name, wxString &src_name, wxString &dst_name)
{
	DataGroup* src_group = GetGroup(src_group_name);
	if (!src_group)
		return;
	int i;
	VolumeData* src_vd = 0;
	for (i = 0; i<src_group->GetVolumeNum(); i++)
	{
		wxString name = src_group->GetVolumeData(i)->GetName();
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
	if (dst_group->GetVolumeNum() == 0 || dst_name == "")
	{
		dst_group->InsertVolumeData(0, src_vd);
	}
	else
	{
		for (i = 0; i<dst_group->GetVolumeNum(); i++)
		{
			wxString name = dst_group->GetVolumeData(i)->GetName();
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
	fluo::Color gamma = dst_group->GetGamma();
	src_vd->SetGamma(gamma);
	fluo::Color brightness = dst_group->GetBrightness();
	src_vd->SetBrightness(brightness);
	fluo::Color hdr = dst_group->GetHdr();
	src_vd->SetHdr(hdr);
	bool sync_r = dst_group->GetSyncR();
	src_vd->SetSyncR(sync_r);
	bool sync_g = dst_group->GetSyncG();
	src_vd->SetSyncG(sync_g);
	bool sync_b = dst_group->GetSyncB();
	src_vd->SetSyncB(sync_b);

	m_vd_pop_dirty = true;
	m_md_pop_dirty = true;

	if (m_frame)
	{
		AdjustView* adjust_view = m_frame->GetAdjustView();
		if (adjust_view)
		{
			adjust_view->SetVolumeData(src_vd);
			adjust_view->SetGroupLink(dst_group);
			adjust_view->UpdateSync();
		}
	}
}

//move mesh within a group
void VRenderGLView::MoveMeshinGroup(wxString &group_name, wxString &src_name, wxString &dst_name)
{
	MeshGroup* group = GetMGroup(group_name);
	if (!group)
		return;

	MeshData* src_md = 0;
	int i, src_index;
	for (i = 0; i<group->GetMeshNum(); i++)
	{
		wxString name = group->GetMeshData(i)->GetName();
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
		wxString name = group->GetMeshData(i)->GetName();
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
void VRenderGLView::MoveMeshtoView(wxString &group_name, wxString &src_name, wxString &dst_name)
{
	MeshGroup* group = GetMGroup(group_name);
	if (!group)
		return;

	MeshData* src_md = 0;
	int i;
	for (i = 0; i<group->GetMeshNum(); i++)
	{
		wxString name = group->GetMeshData(i)->GetName();
		if (name == src_name)
		{
			src_md = group->GetMeshData(i);
			group->RemoveMeshData(i);
			break;
		}
	}
	if (!src_md)
		return;
	if (dst_name == "")
		m_layer_list.push_back(src_md);
	else
	{
		for (i = 0; i<(int)m_layer_list.size(); i++)
		{
			wxString name = m_layer_list[i]->GetName();
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
void VRenderGLView::MoveMeshtoGroup(wxString &group_name, wxString &src_name, wxString &dst_name)
{
	MeshData* src_md = 0;
	int i;

	for (i = 0; i<(int)m_layer_list.size(); i++)
	{
		wxString name = m_layer_list[i]->GetName();
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
		for (i = 0; i<group->GetMeshNum(); i++)
		{
			wxString name = group->GetMeshData(i)->GetName();
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
void VRenderGLView::MoveMeshfromtoGroup(wxString &src_group_name, wxString &dst_group_name, wxString &src_name, wxString &dst_name)
{
	MeshGroup* src_group = GetMGroup(src_group_name);
	if (!src_group)
		return;
	int i;
	MeshData* src_md = 0;
	for (i = 0; i<src_group->GetMeshNum(); i++)
	{
		wxString name = src_group->GetMeshData(i)->GetName();
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
			wxString name = dst_group->GetMeshData(i)->GetName();
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

//layer control
int VRenderGLView::GetGroupNum()
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

int VRenderGLView::GetLayerNum()
{
	return m_layer_list.size();
}

TreeLayer* VRenderGLView::GetLayer(int index)
{
	if (index >= 0 && index<(int)m_layer_list.size())
		return m_layer_list[index];
	else
		return 0;
}

wxString VRenderGLView::AddGroup(wxString str, wxString prev_group)
{
	DataGroup* group = new DataGroup();
	if (group && str != "")
		group->SetName(str);

	bool found_prev = false;
	for (int i = 0; i<(int)m_layer_list.size(); i++)
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
	if (m_frame)
	{
		AdjustView* adjust_view = m_frame->GetAdjustView();
		if (adjust_view && group)
		{
			fluo::Color gamma, brightness, hdr;
			bool sync_r, sync_g, sync_b;
			adjust_view->GetDefaults(gamma, brightness, hdr, sync_r, sync_g, sync_b);
			group->SetGamma(gamma);
			group->SetBrightness(brightness);
			group->SetHdr(hdr);
			group->SetSyncR(sync_r);
			group->SetSyncG(sync_g);
			group->SetSyncB(sync_b);
		}
	}

	if (group)
		return group->GetName();
	else
		return "";
}

DataGroup* VRenderGLView::AddOrGetGroup()
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
	if (m_frame)
	{
		AdjustView* adjust_view = m_frame->GetAdjustView();
		if (adjust_view)
		{
			fluo::Color gamma, brightness, hdr;
			bool sync_r, sync_g, sync_b;
			adjust_view->GetDefaults(gamma, brightness, hdr, sync_r, sync_g, sync_b);
			group->SetGamma(gamma);
			group->SetBrightness(brightness);
			group->SetHdr(hdr);
			group->SetSyncR(sync_r);
			group->SetSyncG(sync_g);
			group->SetSyncB(sync_b);
		}
	}
	m_layer_list.push_back(group);
	return group;
}

wxString VRenderGLView::AddMGroup(wxString str)
{
	MeshGroup* group = new MeshGroup();
	if (group && str != "")
		group->SetName(str);
	m_layer_list.push_back(group);

	if (group)
		return group->GetName();
	else
		return "";
}

MeshGroup* VRenderGLView::AddOrGetMGroup()
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

MeshGroup* VRenderGLView::GetMGroup(wxString str)
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

//init
void VRenderGLView::InitView(unsigned int type)
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
		if (!m_vrv->m_use_dft_settings)
			m_scale_factor = 1.0;
	}

	if (type&INIT_OBJ_TRANSL)
	{
		m_obj_transx = 0.0;
		m_obj_transy = 0.0;
		m_obj_transz = 0.0;
	}

	if (type&INIT_ROTATE || !m_init_view)
	{
		if (!m_vrv->m_use_dft_settings)
		{
			m_q = fluo::Quaternion(0, 0, 0, 1);
			m_q.ToEuler(m_rotx, m_roty, m_rotz);
		}
	}

	m_init_view = true;

}

void VRenderGLView::DrawBounds()
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	flvr::ShaderProgram* shader =
		flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHDR_DRAW_GEOMETRY);
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
		flvr::TextureRenderer::vertex_array_manager_.vertex_array(flvr::VA_Bound_Cube);
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

void VRenderGLView::DrawClippingPlanes(int face_winding)
{
	int i;
	bool link = false;
	PLANE_MODES plane_mode = kNormal;
	if (m_frame && m_frame->GetClippingView())
	{
		link = m_frame->GetClippingView()->GetChannLink();
		plane_mode = m_frame->GetClippingView()->GetPlaneMode();
	}
	double width = 1.0;
	if (m_frame && m_frame->GetSettingDlg())
		width = m_frame->GetSettingDlg()->GetLineWidth();

	if (plane_mode == kNone)
		return;

	bool draw_plane = plane_mode != kFrame6 && plane_mode != kFrame3;
	bool border = plane_mode == kFrame6 ||
		(m_clip_mask == -1 && face_winding == FRONT_FACE) ||
		m_clip_mask != -1;
	if (!border && plane_mode == kFrame3)
		return;

	if ((plane_mode == kLowTransBack ||
		plane_mode == kNormalBack) &&
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
		flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHDR_DRAW_GEOMETRY);
	if (shader1)
	{
		if (!shader1->valid())
			shader1->create();
		shader1->bind();
	}
	flvr::ShaderProgram* shader2 =
		flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHDR_DRAW_THICK_LINES_COLOR);
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

		vector<fluo::Plane*> *planes = vr->get_planes();
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
			plane_trans = plane_mode == kLowTrans ||
			plane_mode == kLowTransBack ? 0.1 : 0.3;

		if (face_winding == FRONT_FACE)
		{
			plane_trans = plane_mode == kLowTrans ||
				plane_mode == kLowTransBack ? 0.1 : 0.3;
		}

		if (plane_mode == kNormal ||
			plane_mode == kNormalBack)
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
			shader2->setLocalParam(0, GetSize().x, GetSize().y, width, 0.0);
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
			flvr::TextureRenderer::vertex_array_manager_.vertex_array(flvr::VA_Clip_Planes);
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
				if (plane_mode == kNormal ||
					plane_mode == kNormalBack)
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
				if (plane_mode == kNormal ||
					plane_mode == kNormalBack)
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
				if (plane_mode == kNormal ||
					plane_mode == kNormalBack)
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
				if (plane_mode == kNormal ||
					plane_mode == kNormalBack)
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
				if (plane_mode == kNormal ||
					plane_mode == kNormalBack)
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
				if (plane_mode == kNormal ||
					plane_mode == kNormalBack)
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

void VRenderGLView::DrawGrid()
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	flvr::ShaderProgram* shader =
		flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHDR_DRAW_GEOMETRY);
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
		flvr::TextureRenderer::vertex_array_manager_.vertex_array(flvr::VA_Grid);
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

void VRenderGLView::DrawCamCtr()
{
	flvr::VertexArray* va_jack =
		flvr::TextureRenderer::vertex_array_manager_.vertex_array(flvr::VA_Cam_Jack);
	if (!va_jack)
		return;
	double len;
	if (m_camctr_size > 0.0)
		len = m_distance*tan(d2r(m_aov / 2.0))*m_camctr_size / 10.0;
	else
		len = fabs(m_camctr_size);
	if (m_pin_rot_center)
		len /= 10.0;
	va_jack->set_param(0, len);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	flvr::ShaderProgram* shader =
		flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHDR_DRAW_GEOMETRY);
	if (shader)
	{
		if (!shader->valid())
			shader->create();
		shader->bind();
	}
	glm::mat4 matrix = m_proj_mat * m_mv_mat;
	shader->setLocalParamMatrix(0, glm::value_ptr(matrix));

	va_jack->draw_begin();
	shader->setLocalParam(0, 1.0, 0.0, 0.0, 1.0);
	va_jack->draw_cam_jack(0);
	shader->setLocalParam(0, 0.0, 1.0, 0.0, 1.0);
	va_jack->draw_cam_jack(1);
	shader->setLocalParam(0, 0.0, 0.0, 1.0, 1.0);
	va_jack->draw_cam_jack(2);
	va_jack->draw_end();

	if (shader && shader->valid())
		shader->release();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
}

void VRenderGLView::DrawFrame()
{
	int nx, ny;
	GetRenderSize(nx, ny);
	glm::mat4 proj_mat = glm::ortho(float(0), float(nx), float(0), float(ny));

	flvr::VertexArray* va_frame =
		flvr::TextureRenderer::vertex_array_manager_.vertex_array(flvr::VA_Crop_Frame);
	if (!va_frame)
		return;

	glDisable(GL_DEPTH_TEST);
	flvr::ShaderProgram* shader =
		flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHDR_DRAW_GEOMETRY);
	if (shader)
	{
		if (!shader->valid())
			shader->create();
		shader->bind();
	}
	shader->setLocalParam(0, 1.0, 1.0, 0.0, 1.0);
	shader->setLocalParamMatrix(0, glm::value_ptr(proj_mat));

	//draw frame
	vector<std::pair<unsigned int, double>> params;
	params.push_back(std::pair<unsigned int, double>(0, m_frame_x));
	params.push_back(std::pair<unsigned int, double>(1, m_frame_y));
	params.push_back(std::pair<unsigned int, double>(2, m_frame_w));
	params.push_back(std::pair<unsigned int, double>(3, m_frame_h));
	va_frame->set_param(params);
	va_frame->draw();

	if (shader && shader->valid())
		shader->release();
	glEnable(GL_DEPTH_TEST);
}

void VRenderGLView::DrawScaleBar()
{
	flvr::VertexArray* va_scale_bar =
		flvr::TextureRenderer::vertex_array_manager_.vertex_array(flvr::VA_Scale_Bar);
	if (!va_scale_bar)
		return;

	//if (m_draw_legend)
	//	offset = m_sb_height;
	int nx, ny;
	GetRenderSize(nx, ny);
	float sx, sy;
	sx = 2.0 / nx;
	sy = 2.0 / ny;
	float px, py, ph;
	glm::mat4 proj_mat = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f);
	double len = m_sb_length / (m_ortho_right - m_ortho_left);
	wstring wsb_text = m_sb_text.ToStdWstring();
	double textlen =
		m_text_renderer.RenderTextLen(wsb_text);
	fluo::Color text_color = GetTextColor();
	double font_height =
		flvr::TextRenderer::text_texture_manager_.GetSize() + 3.0;

	std::vector<std::pair<unsigned int, double>> params;
	if (m_draw_frame)
	{
		int framew = m_frame_w;
		int frameh = m_frame_h;
		int framex = m_frame_x;
		int framey = m_frame_y;
		if (m_enlarge)
		{
			framew *= m_enlarge_scale;
			frameh *= m_enlarge_scale;
			framex *= m_enlarge_scale;
			framey *= m_enlarge_scale;
		}
		px = (framex + framew - font_height + m_sb_x) / nx;
		py = (1.1 * font_height + framey + m_sb_y) / ny;
		ph = 5.0 / ny;
		if (m_enlarge)
			ph *= m_enlarge_scale;
		params.push_back(std::pair<unsigned int, double>(0, px));
		params.push_back(std::pair<unsigned int, double>(1, py));
		params.push_back(std::pair<unsigned int, double>(2, len));
		params.push_back(std::pair<unsigned int, double>(3, ph));

		if (m_disp_scale_bar_text)
		{
			px = px * nx - 0.5 * (len * nx + textlen + nx) + m_sb_x;
			py = py * ny + 0.5 * font_height - ny / 2.0 + m_sb_y;
			m_text_renderer.RenderText(
				wsb_text, text_color,
				px*sx, py*sy, sx, sy);
		}
	}
	else
	{
		px = (nx - font_height + m_sb_x) / nx;
		py = (1.1 * font_height + m_sb_y) / ny;
		ph = 5.0 / ny;
		if (m_enlarge)
			ph *= m_enlarge_scale;
		params.push_back(std::pair<unsigned int, double>(0, px));
		params.push_back(std::pair<unsigned int, double>(1, py));
		params.push_back(std::pair<unsigned int, double>(2, len));
		params.push_back(std::pair<unsigned int, double>(3, ph));

		if (m_disp_scale_bar_text)
		{
			px = px * nx - 0.5 * (len * nx + textlen + nx) + m_sb_x;
			py = (py - 0.5) * ny + 0.5 * font_height + m_sb_y;
			m_text_renderer.RenderText(
				wsb_text, text_color,
				px*sx, py*sy, sx, sy);
		}
	}

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	flvr::ShaderProgram* shader =
		flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHDR_DRAW_GEOMETRY);
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

void VRenderGLView::DrawLegend()
{
	if (!m_frame)
		return;

	double font_height =
		flvr::TextRenderer::text_texture_manager_.GetSize() + 3.0;

	int nx, ny;
	GetRenderSize(nx, ny);

	double xoffset = 10.0;
	double yoffset = 10.0;
	if (m_draw_frame)
	{
		xoffset = 10.0 + m_frame_x;
		yoffset = ny - m_frame_h - m_frame_y + 10.0;
	}

	wxString wxstr;
	wstring wstr;
	double length = 0.0;
	double name_len = 0.0;
	double gap_width = font_height*1.5;
	int lines = 0;
	int i;
	//first pass
	for (i = 0; i<(int)m_vd_pop_list.size(); i++)
	{
		if (m_vd_pop_list[i] && m_vd_pop_list[i]->GetLegend())
		{
			wxstr = m_vd_pop_list[i]->GetName();
			wstr = wxstr.ToStdWstring();
			name_len = m_text_renderer.RenderTextLen(wstr) + font_height;
			length += name_len;
			if (length < double(m_draw_frame ? m_frame_w : nx) - gap_width)
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
			wxstr = m_md_pop_list[i]->GetName();
			wstr = wxstr.ToStdWstring();
			name_len = m_text_renderer.RenderTextLen(wstr) + font_height;
			length += name_len;
			if (length < double(m_draw_frame ? m_frame_w : nx) - gap_width)
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
	double xpos;
	length = 0.0;
	for (i = 0; i<(int)m_vd_pop_list.size(); i++)
	{
		if (m_vd_pop_list[i] && m_vd_pop_list[i]->GetLegend())
		{
			wxstr = m_vd_pop_list[i]->GetName();
			xpos = length;
			wstr = wxstr.ToStdWstring();
			name_len = m_text_renderer.RenderTextLen(wstr) + font_height;
			length += name_len;
			if (length < double(m_draw_frame ? m_frame_w : nx) - gap_width)
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
			if (m_frame->GetCurSelType() == 2 &&
				m_frame->GetCurSelVol() &&
				m_frame->GetCurSelVol()->GetName() == wxstr)
				highlighted = true;
			DrawName(xpos + xoffset, ny - (lines - cur_line + 0.1)*font_height - yoffset,
				nx, ny, wxstr, m_vd_pop_list[i]->GetColor(),
				font_height, highlighted);
		}
	}
	for (i = 0; i<(int)m_md_pop_list.size(); i++)
	{
		if (m_md_pop_list[i] && m_md_pop_list[i]->GetLegend())
		{
			wxstr = m_md_pop_list[i]->GetName();
			xpos = length;
			wstr = wxstr.ToStdWstring();
			name_len = m_text_renderer.RenderTextLen(wstr) + font_height;
			length += name_len;
			if (length < double(m_draw_frame ? m_frame_w : nx) - gap_width)
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
			if (m_frame->GetCurSelType() == 3 &&
				m_frame->GetCurSelMesh() &&
				m_frame->GetCurSelMesh()->GetName() == wxstr)
				highlighted = true;
			DrawName(xpos + xoffset, ny - (lines - cur_line + 0.1)*font_height - yoffset,
				nx, ny, wxstr, c, font_height, highlighted);
		}
	}

	m_sb_height = (lines + 1)*font_height;
}

void VRenderGLView::DrawName(
	double x, double y, int nx, int ny,
	wxString name, fluo::Color color,
	double font_height,
	bool highlighted)
{
	flvr::VertexArray* va_legend_squares =
		flvr::TextureRenderer::vertex_array_manager_.vertex_array(flvr::VA_Legend_Squares);
	if (!va_legend_squares)
		return;

	wstring wstr;
	float sx, sy;
	sx = 2.0 / nx;
	sy = 2.0 / ny;
	glm::mat4 proj_mat = glm::ortho(0.0f, float(nx), 0.0f, float(ny));

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	flvr::ShaderProgram* shader =
		flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHDR_DRAW_GEOMETRY);
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

	float px1 = x + font_height - nx / 2;
	float py1 = ny / 2 - y + 0.25*font_height;
	wstr = name.ToStdWstring();
	m_text_renderer.RenderText(
		wstr, text_color,
		px1*sx, py1*sy, sx, sy);
	if (highlighted)
	{
		px1 -= 0.5;
		py1 += 0.5;
		m_text_renderer.RenderText(
			wstr, color,
			px1*sx, py1*sy, sx, sy);
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
}

void VRenderGLView::DrawGradBg()
{
	glm::mat4 proj_mat = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

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
				std::max(hsv_color1.val() - 0.5, 0.0)));
			color2 = fluo::Color(fluo::HSVColor(hsv_color1.hue(),
				hsv_color1.sat() * 0.3,
				std::max(hsv_color1.val() - 0.3, 0.0)));
		}
	}
	else
	{
		color1 = fluo::Color(fluo::HSVColor(hsv_color1.hue(),
			hsv_color1.sat() * 0.1,
			std::min(hsv_color1.val() + 0.7, 1.0)));
		color2 = fluo::Color(fluo::HSVColor(hsv_color1.hue(),
			hsv_color1.sat() * 0.3,
			std::min(hsv_color1.val() + 0.5, 1.0)));
	}

	vector<float> vertex;
	vertex.reserve(16 * 3);
	vertex.push_back(0.0); vertex.push_back(0.0); vertex.push_back(0.0);
	vertex.push_back(m_bg_color.r()); vertex.push_back(m_bg_color.g()); vertex.push_back(m_bg_color.b());
	vertex.push_back(1.0); vertex.push_back(0.0); vertex.push_back(0.0);
	vertex.push_back(m_bg_color.r()); vertex.push_back(m_bg_color.g()); vertex.push_back(m_bg_color.b());
	vertex.push_back(0.0); vertex.push_back(0.3); vertex.push_back(0.0);
	vertex.push_back(color1.r()); vertex.push_back(color1.g()); vertex.push_back(color1.b());
	vertex.push_back(1.0); vertex.push_back(0.3); vertex.push_back(0.0);
	vertex.push_back(color1.r()); vertex.push_back(color1.g()); vertex.push_back(color1.b());
	vertex.push_back(0.0); vertex.push_back(0.5); vertex.push_back(0.0);
	vertex.push_back(color2.r()); vertex.push_back(color2.g()); vertex.push_back(color2.b());
	vertex.push_back(1.0); vertex.push_back(0.5); vertex.push_back(0.0);
	vertex.push_back(color2.r()); vertex.push_back(color2.g()); vertex.push_back(color2.b());
	vertex.push_back(0.0); vertex.push_back(1.0); vertex.push_back(0.0);
	vertex.push_back(m_bg_color.r()); vertex.push_back(m_bg_color.g()); vertex.push_back(m_bg_color.b());
	vertex.push_back(1.0); vertex.push_back(1.0); vertex.push_back(0.0);
	vertex.push_back(m_bg_color.r()); vertex.push_back(m_bg_color.g()); vertex.push_back(m_bg_color.b());

	flvr::ShaderProgram* shader =
		flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHDR_DRAW_GEOMETRY_COLOR3);
	if (shader)
	{
		if (!shader->valid())
			shader->create();
		shader->bind();
	}
	shader->setLocalParamMatrix(0, glm::value_ptr(proj_mat));

	flvr::VertexArray* va_bkg =
		flvr::TextureRenderer::vertex_array_manager_.vertex_array(flvr::VA_Grad_Bkg);
	if (va_bkg)
	{
		va_bkg->set_param(vertex);
		va_bkg->draw();
	}

	if (shader && shader->valid())
		shader->release();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
}

void VRenderGLView::SetColormapColors(int colormap, fluo::Color &c, double inv)
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

void VRenderGLView::DrawColormap()
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
		enable_alpha = m_cur_vol->GetEnableAlpha();
		fluo::Color vd_color = m_cur_vol->GetColor();
		SetColormapColors(m_cur_vol->GetColormap(),
			vd_color, m_cur_vol->GetColormapInv());
	}
	else return;

	double offset = 0.0;
	if (m_draw_legend)
		offset = m_sb_height;

	int nx, ny;
	GetRenderSize(nx, ny);
	float sx, sy;
	sx = 2.0 / nx;
	sy = 2.0 / ny;

	glm::mat4 proj_mat = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f);

	vector<float> vertex;
	vertex.reserve(14 * 7);

	float px, py;
	//draw colormap
	if (m_draw_frame)
	{
		px = (0.01*m_frame_w + m_frame_x) / nx;
		py = (0.05*m_frame_w + m_frame_x) / nx;
		vertex.push_back(px); vertex.push_back((0.1*m_frame_h + m_frame_y + offset) / ny); vertex.push_back(0.0);
		vertex.push_back(m_color_1.r()); vertex.push_back(m_color_1.g()); vertex.push_back(m_color_1.b()); vertex.push_back(enable_alpha ? 0.0 : 1.0);
		vertex.push_back(py); vertex.push_back((0.1*m_frame_h + m_frame_y + offset) / ny); vertex.push_back(0.0);
		vertex.push_back(m_color_1.r()); vertex.push_back(m_color_1.g()); vertex.push_back(m_color_1.b()); vertex.push_back(enable_alpha ? 0.0 : 1.0);
		vertex.push_back(px); vertex.push_back(((0.1 + 0.4*m_value_2)*m_frame_h + m_frame_y + offset) / ny); vertex.push_back(0.0);
		vertex.push_back(m_color_2.r()); vertex.push_back(m_color_2.g()); vertex.push_back(m_color_2.b()); vertex.push_back(enable_alpha ? m_value_2 : 1.0);
		vertex.push_back(py); vertex.push_back(((0.1 + 0.4*m_value_2)*m_frame_h + m_frame_y + offset) / ny); vertex.push_back(0.0);
		vertex.push_back(m_color_2.r()); vertex.push_back(m_color_2.g()); vertex.push_back(m_color_2.b()); vertex.push_back(enable_alpha ? m_value_2 : 1.0);
		vertex.push_back(px); vertex.push_back(((0.1 + 0.4*m_value_3)*m_frame_h + m_frame_y + offset) / ny); vertex.push_back(0.0);
		vertex.push_back(m_color_3.r()); vertex.push_back(m_color_3.g()); vertex.push_back(m_color_3.b()); vertex.push_back(enable_alpha ? m_value_3 : 1.0);
		vertex.push_back(py); vertex.push_back(((0.1 + 0.4*m_value_3)*m_frame_h + m_frame_y + offset) / ny); vertex.push_back(0.0);
		vertex.push_back(m_color_3.r()); vertex.push_back(m_color_3.g()); vertex.push_back(m_color_3.b()); vertex.push_back(enable_alpha ? m_value_3 : 1.0);
		vertex.push_back(px); vertex.push_back(((0.1 + 0.4*m_value_4)*m_frame_h + m_frame_y + offset) / ny); vertex.push_back(0.0);
		vertex.push_back(m_color_4.r()); vertex.push_back(m_color_4.g()); vertex.push_back(m_color_4.b()); vertex.push_back(enable_alpha ? m_value_4 : 1.0);
		vertex.push_back(py); vertex.push_back(((0.1 + 0.4*m_value_4)*m_frame_h + m_frame_y + offset) / ny); vertex.push_back(0.0);
		vertex.push_back(m_color_4.r()); vertex.push_back(m_color_4.g()); vertex.push_back(m_color_4.b()); vertex.push_back(enable_alpha ? m_value_4 : 1.0);
		vertex.push_back(px); vertex.push_back(((0.1 + 0.4*m_value_5)*m_frame_h + m_frame_y + offset) / ny); vertex.push_back(0.0);
		vertex.push_back(m_color_5.r()); vertex.push_back(m_color_5.g()); vertex.push_back(m_color_5.b()); vertex.push_back(enable_alpha ? m_value_5 : 1.0);
		vertex.push_back(py); vertex.push_back(((0.1 + 0.4*m_value_5)*m_frame_h + m_frame_y + offset) / ny); vertex.push_back(0.0);
		vertex.push_back(m_color_5.r()); vertex.push_back(m_color_5.g()); vertex.push_back(m_color_5.b()); vertex.push_back(enable_alpha ? m_value_5 : 1.0);
		vertex.push_back(px); vertex.push_back(((0.1 + 0.4*m_value_6)*m_frame_h + m_frame_y + offset) / ny); vertex.push_back(0.0);
		vertex.push_back(m_color_6.r()); vertex.push_back(m_color_6.g()); vertex.push_back(m_color_6.b()); vertex.push_back(enable_alpha ? m_value_6 : 1.0);
		vertex.push_back(py); vertex.push_back(((0.1 + 0.4*m_value_6)*m_frame_h + m_frame_y + offset) / ny); vertex.push_back(0.0);
		vertex.push_back(m_color_6.r()); vertex.push_back(m_color_6.g()); vertex.push_back(m_color_6.b()); vertex.push_back(enable_alpha ? m_value_6 : 1.0);
		vertex.push_back(px); vertex.push_back((0.5*m_frame_h + m_frame_y + offset) / ny); vertex.push_back(0.0);
		vertex.push_back(m_color_7.r()); vertex.push_back(m_color_7.g()); vertex.push_back(m_color_7.b()); vertex.push_back(1.0);
		vertex.push_back(py); vertex.push_back((0.5*m_frame_h + m_frame_y + offset) / ny); vertex.push_back(0.0);
		vertex.push_back(m_color_7.r()); vertex.push_back(m_color_7.g()); vertex.push_back(m_color_7.b()); vertex.push_back(1.0);

		wxString str;
		wstring wstr;

		fluo::Color text_color = GetTextColor();

		//value 1
		px = 0.052*m_frame_w + m_frame_x - nx / 2.0;
		py = 0.1*m_frame_h + m_frame_y + offset - ny / 2.0;
		str = wxString::Format("%d", 0);
		wstr = str.ToStdWstring();
		m_text_renderer.RenderText(
			wstr, text_color,
			px*sx, py*sy, sx, sy);
		//value 2
		px = 0.052*m_frame_w + m_frame_x - nx / 2.0;
		py = (0.1 + 0.4*m_value_2)*m_frame_h + m_frame_y + offset - ny / 2.0;
		str = wxString::Format("%d", int(m_value_2*max_val));
		wstr = str.ToStdWstring();
		m_text_renderer.RenderText(
			wstr, text_color,
			px*sx, py*sy, sx, sy);
		//value 4
		px = 0.052*m_frame_w + m_frame_x - nx / 2.0;
		py = (0.1 + 0.4*m_value_4)*m_frame_h + m_frame_y + offset - ny / 2.0;
		str = wxString::Format("%d", int(m_value_4*max_val));
		wstr = str.ToStdWstring();
		m_text_renderer.RenderText(
			wstr, text_color,
			px*sx, py*sy, sx, sy);
		//value 6
		px = 0.052*m_frame_w + m_frame_x - nx / 2.0;
		py = (0.1 + 0.4*m_value_6)*m_frame_h + m_frame_y + offset - ny / 2.0;
		str = wxString::Format("%d", int(m_value_6*max_val));
		wstr = str.ToStdWstring();
		m_text_renderer.RenderText(
			wstr, text_color,
			px*sx, py*sy, sx, sy);
		//value 7
		px = 0.052*m_frame_w + m_frame_x - nx / 2.0;
		py = 0.5*m_frame_h + m_frame_y + offset - ny / 2.0;
		str = wxString::Format("%d", int(max_val));
		wstr = str.ToStdWstring();
		m_text_renderer.RenderText(
			wstr, text_color,
			px*sx, py*sy, sx, sy);
	}
	else
	{
		vertex.push_back(0.01); vertex.push_back(0.1 + offset / ny); vertex.push_back(0.0);
		vertex.push_back(m_color_1.r()); vertex.push_back(m_color_1.g()); vertex.push_back(m_color_1.b()); vertex.push_back(enable_alpha ? 0.0 : 1.0);
		vertex.push_back(0.05); vertex.push_back(0.1 + offset / ny); vertex.push_back(0.0);
		vertex.push_back(m_color_1.r()); vertex.push_back(m_color_1.g()); vertex.push_back(m_color_1.b()); vertex.push_back(enable_alpha ? 0.0 : 1.0);
		vertex.push_back(0.01); vertex.push_back(0.1 + 0.4*m_value_2 + offset / ny); vertex.push_back(0.0);
		vertex.push_back(m_color_2.r()); vertex.push_back(m_color_2.g()); vertex.push_back(m_color_2.b()); vertex.push_back(enable_alpha ? m_value_2 : 1.0);
		vertex.push_back(0.05); vertex.push_back(0.1 + 0.4*m_value_2 + offset / ny); vertex.push_back(0.0);
		vertex.push_back(m_color_2.r()); vertex.push_back(m_color_2.g()); vertex.push_back(m_color_2.b()); vertex.push_back(enable_alpha ? m_value_2 : 1.0);
		vertex.push_back(0.01); vertex.push_back(0.1 + 0.4*m_value_3 + offset / ny); vertex.push_back(0.0);
		vertex.push_back(m_color_3.r()); vertex.push_back(m_color_3.g()); vertex.push_back(m_color_3.b()); vertex.push_back(enable_alpha ? m_value_3 : 1.0);
		vertex.push_back(0.05); vertex.push_back(0.1 + 0.4*m_value_3 + offset / ny); vertex.push_back(0.0);
		vertex.push_back(m_color_3.r()); vertex.push_back(m_color_3.g()); vertex.push_back(m_color_3.b()); vertex.push_back(enable_alpha ? m_value_3 : 1.0);
		vertex.push_back(0.01); vertex.push_back(0.1 + 0.4*m_value_4 + offset / ny); vertex.push_back(0.0);
		vertex.push_back(m_color_4.r()); vertex.push_back(m_color_4.g()); vertex.push_back(m_color_4.b()); vertex.push_back(enable_alpha ? m_value_4 : 1.0);
		vertex.push_back(0.05); vertex.push_back(0.1 + 0.4*m_value_4 + offset / ny); vertex.push_back(0.0);
		vertex.push_back(m_color_4.r()); vertex.push_back(m_color_4.g()); vertex.push_back(m_color_4.b()); vertex.push_back(enable_alpha ? m_value_4 : 1.0);
		vertex.push_back(0.01); vertex.push_back(0.1 + 0.4*m_value_5 + offset / ny); vertex.push_back(0.0);
		vertex.push_back(m_color_5.r()); vertex.push_back(m_color_5.g()); vertex.push_back(m_color_5.b()); vertex.push_back(enable_alpha ? m_value_5 : 1.0);
		vertex.push_back(0.05); vertex.push_back(0.1 + 0.4*m_value_5 + offset / ny); vertex.push_back(0.0);
		vertex.push_back(m_color_5.r()); vertex.push_back(m_color_5.g()); vertex.push_back(m_color_5.b()); vertex.push_back(enable_alpha ? m_value_5 : 1.0);
		vertex.push_back(0.01); vertex.push_back(0.1 + 0.4*m_value_6 + offset / ny); vertex.push_back(0.0);
		vertex.push_back(m_color_6.r()); vertex.push_back(m_color_6.g()); vertex.push_back(m_color_6.b()); vertex.push_back(enable_alpha ? m_value_6 : 1.0);
		vertex.push_back(0.05); vertex.push_back(0.1 + 0.4*m_value_6 + offset / ny); vertex.push_back(0.0);
		vertex.push_back(m_color_6.r()); vertex.push_back(m_color_6.g()); vertex.push_back(m_color_6.b()); vertex.push_back(enable_alpha ? m_value_6 : 1.0);
		vertex.push_back(0.01); vertex.push_back(0.5 + offset / ny); vertex.push_back(0.0);
		vertex.push_back(m_color_7.r()); vertex.push_back(m_color_7.g()); vertex.push_back(m_color_7.b()); vertex.push_back(1.0);
		vertex.push_back(0.05); vertex.push_back(0.5 + offset / ny); vertex.push_back(0.0);
		vertex.push_back(m_color_7.r()); vertex.push_back(m_color_7.g()); vertex.push_back(m_color_7.b()); vertex.push_back(1.0);

		wxString str;
		wstring wstr;

		fluo::Color text_color = GetTextColor();

		//value 1
		px = 0.052*nx - nx / 2.0;
		py = ny / 2.0 - 0.9*ny + offset;
		str = wxString::Format("%d", 0);
		wstr = str.ToStdWstring();
		m_text_renderer.RenderText(
			wstr, text_color,
			px*sx, py*sy, sx, sy);
		//value 2
		px = 0.052*nx - nx / 2.0;
		py = ny / 2.0 - (0.9 - 0.4*m_value_2)*ny + offset;
		str = wxString::Format("%d", int(m_value_2*max_val));
		wstr = str.ToStdWstring();
		m_text_renderer.RenderText(
			wstr, text_color,
			px*sx, py*sy, sx, sy);
		//value 4
		px = 0.052*nx - nx / 2.0;
		py = ny / 2.0 - (0.9 - 0.4*m_value_4)*ny + offset;
		str = wxString::Format("%d", int(m_value_4*max_val));
		wstr = str.ToStdWstring();
		m_text_renderer.RenderText(
			wstr, text_color,
			px*sx, py*sy, sx, sy);
		//value 6
		px = 0.052*nx - nx / 2.0;
		py = ny / 2.0 - (0.9 - 0.4*m_value_6)*ny + offset;
		str = wxString::Format("%d", int(m_value_6*max_val));
		wstr = str.ToStdWstring();
		m_text_renderer.RenderText(
			wstr, text_color,
			px*sx, py*sy, sx, sy);
		//value 7
		px = 0.052*nx - nx / 2.0;
		py = ny / 2.0 - 0.5*ny + offset;
		str = wxString::Format("%d", int(max_val));
		wstr = str.ToStdWstring();
		m_text_renderer.RenderText(
			wstr, text_color,
			px*sx, py*sy, sx, sy);
	}

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	flvr::ShaderProgram* shader =
		flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHDR_DRAW_GEOMETRY_COLOR4);
	if (shader)
	{
		if (!shader->valid())
			shader->create();
		shader->bind();
	}
	shader->setLocalParamMatrix(0, glm::value_ptr(proj_mat));

	flvr::VertexArray* va_colormap =
		flvr::TextureRenderer::vertex_array_manager_.vertex_array(flvr::VA_Color_Map);
	if (va_colormap)
	{
		va_colormap->set_param(vertex);
		va_colormap->draw();
	}

	if (shader && shader->valid())
		shader->release();

	glEnable(GL_DEPTH_TEST);
}

void VRenderGLView::DrawInfo(int nx, int ny)
{
	float sx, sy;
	sx = 2.0 / nx;
	sy = 2.0 / ny;
	float px, py;
	float gapw = flvr::TextRenderer::text_texture_manager_.GetSize();
	float gaph = gapw * 2;

	double fps_ = 1.0 / m_timer->average();
	wxString str;
	fluo::Color text_color = GetTextColor();
	if (flvr::TextureRenderer::get_mem_swap())
	{
		if (m_selector.GetBrushUsePres())
			str = wxString::Format(
				"Int: %s, FPS: %.2f, Bricks: %d, Quota: %d, Time: %lu, Pressure: %.2f",
				m_interactive ? "Yes" : "No",
				fps_ >= 0.0&&fps_<300.0 ? fps_ : 0.0,
				flvr::TextureRenderer::get_finished_bricks(),
				flvr::TextureRenderer::get_quota_bricks(),
				flvr::TextureRenderer::get_cor_up_time(),
				m_selector.GetPressure());
		else
			str = wxString::Format(
				"Int: %s, FPS: %.2f, Bricks: %d, Quota: %d, Time: %lu",
				m_interactive ? "Yes" : "No",
				fps_ >= 0.0&&fps_<300.0 ? fps_ : 0.0,
				flvr::TextureRenderer::get_finished_bricks(),
				flvr::TextureRenderer::get_quota_bricks(),
				flvr::TextureRenderer::get_cor_up_time());
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
		if (m_selector.GetBrushUsePres())
			str = wxString::Format("FPS: %.2f, Pressure: %.2f",
				fps_ >= 0.0&&fps_<300.0 ? fps_ : 0.0, m_selector.GetPressure());
		else
			str = wxString::Format("FPS: %.2f",
				fps_ >= 0.0&&fps_<300.0 ? fps_ : 0.0);
	}
	wstring wstr_temp = str.ToStdWstring();
	px = gapw - nx / 2;
	py = ny / 2 - gaph / 2;
	m_text_renderer.RenderText(
		wstr_temp, text_color,
		px*sx, py*sy, sx, sy);

	if ((m_draw_info & INFO_T) &&
		(m_draw_info & INFO_X) &&
		(m_draw_info & INFO_Y) &&
		(m_draw_info & INFO_Z))
	{
		fluo::Point p;
		wxPoint mouse_pos = ScreenToClient(wxGetMousePosition());
		m_vp.SetVolumeData(m_cur_vol);
		if ((m_vp.GetPointVolumeBox(mouse_pos.x, mouse_pos.y, true, p )>0.0) ||
			m_vp.GetPointPlane(mouse_pos.x, mouse_pos.y, 0, true, p)>0.0)
		{
			str = wxString::Format("T: %d  X: %.2f  Y: %.2f  Z: %.2f",
				m_tseq_cur_num, p.x(), p.y(), p.z());
			wstr_temp = str.ToStdWstring();
			px = gapw - nx / 2;
			py = ny / 2 - gaph;
			m_text_renderer.RenderText(
				wstr_temp, text_color,
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
			vector<fluo::Plane*> *planes = m_cur_vol->GetVR()->get_planes();
			fluo::Plane* plane = (*planes)[4];
			double abcd[4];
			plane->get_copy(abcd);
			int val = fabs(abcd[3] * resz) + 0.499;

			str = wxString::Format("Z: %.2f ", val * spcz);
			str += L"\u03BCm";
			wstr_temp = str.ToStdWstring();
			if (m_draw_frame)
			{
				px = 0.01*m_frame_w + m_frame_x - nx / 2.0;
				py = 0.04*m_frame_h + m_frame_y - ny / 2.0;
			}
			else
			{
				px = 0.01*nx - nx / 2.0;
				py = 0.04*ny - ny / 2.0;
			}
			m_text_renderer.RenderText(
				wstr_temp, text_color,
				px*sx, py*sy, sx, sy);
		}
	}

	if (m_test_wiref)
	{
		if (m_vol_method == VOL_METHOD_MULTI && m_mvr)
		{
			str = wxString::Format("SLICES: %d", m_mvr->get_slice_num());
			wstr_temp = str.ToStdWstring();
			px = gapw - nx / 2;
			py = ny / 2 - gaph*1.5;
			m_text_renderer.RenderText(
				wstr_temp, text_color,
				px*sx, py*sy, sx, sy);
		}
		else
		{
			for (int i = 0; i<(int)m_vd_pop_list.size(); i++)
			{
				VolumeData* vd = m_vd_pop_list[i];
				if (vd && vd->GetVR())
				{
					str = wxString::Format("SLICES_%d: %d", i + 1, vd->GetVR()->get_slice_num());
					wstr_temp = str.ToStdWstring();
					px = gapw - nx / 2;
					py = ny / 2 - gaph*(3 + i) / 2;
					m_text_renderer.RenderText(
						wstr_temp, text_color,
						px*sx, py*sy, sx, sy);
				}
			}
		}
	}
}

fluo::Quaternion VRenderGLView::Trackball(double dx, double dy)
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
		phi = int(phi / 45.0) * 45.0;
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

fluo::Quaternion VRenderGLView::TrackballClip(int p1x, int p1y, int p2x, int p2y)
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

void VRenderGLView::UpdateClips()
{
	if (m_clip_mode == 1)
		m_q_cl.FromEuler(m_rotx, -m_roty, -m_rotz);

	vector<fluo::Plane*> *planes = 0;
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

void VRenderGLView::Q2A()
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

void VRenderGLView::A2Q()
{
	//view changed, re-sort bricks
	//SetSortBricks();

	m_q.FromEuler(m_rotx, m_roty, m_rotz);
	m_q = m_q * m_zq;

	if (m_clip_mode)
		UpdateClips();
}

//sort bricks after view changes
void VRenderGLView::SetSortBricks()
{
	PopVolumeList();

	for (int i = 0; i<(int)m_vd_pop_list.size(); i++)
	{
		VolumeData* vd = m_vd_pop_list[i];
		if (vd && vd->GetTexture())
			vd->GetTexture()->set_sort_bricks();
	}
}

void VRenderGLView::SetClipMode(int mode)
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
		SetRotations(m_rotx, m_roty, m_rotz);
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
		SetRotations(m_rotx, m_roty, m_rotz);
		break;
	}
}

void VRenderGLView::RestorePlanes()
{
	vector<fluo::Plane*> *planes = 0;
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

void VRenderGLView::SetClippingPlaneRotations(double rotx, double roty, double rotz)
{
	m_rotx_cl = -rotx;
	m_roty_cl = roty;
	m_rotz_cl = rotz;

	m_q_cl.FromEuler(m_rotx_cl, m_roty_cl, m_rotz_cl);
	m_q_cl.Normalize();

	SetRotations(m_rotx, m_roty, m_rotz);
}

void VRenderGLView::GetClippingPlaneRotations(double &rotx, double &roty, double &rotz)
{
	rotx = -m_rotx_cl;
	roty = m_roty_cl;
	rotz = m_rotz_cl;
}

//interpolation
void VRenderGLView::SetIntp(bool mode)
{
	m_intp = mode;
}

bool VRenderGLView::GetIntp()
{
	return m_intp;
}

//start loop update
void VRenderGLView::StartLoopUpdate()
{
	////this is for debug_ds, comment when done
	//if (TextureRenderer::get_mem_swap() &&
	//  TextureRenderer::get_start_update_loop() &&
	//  !TextureRenderer::get_done_update_loop())
	//  return;

	if (flvr::TextureRenderer::get_mem_swap())
	{
		if (flvr::TextureRenderer::active_view_ > 0 &&
			flvr::TextureRenderer::active_view_ != m_vrv->m_id)
			return;
		else
			flvr::TextureRenderer::active_view_ = m_vrv->m_id;

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
					vd->GetVR()->m_mv_mat2 = vd->GetVR()->m_mv_mat * vd->GetVR()->m_mv_mat2;

					fluo::Ray view_ray = vd->GetVR()->compute_view();
					vector<flvr::TextureBrick*> *bricks = 0;
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
							vd->GetShading())
							total_num++;
						if (vd->GetShadow())
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

		vector<VolumeLoaderData> queues;
		if (m_vol_method == VOL_METHOD_MULTI)
		{
			vector<VolumeData*> list;
			for (i = 0; i<m_vd_pop_list.size(); i++)
			{
				VolumeData* vd = m_vd_pop_list[i];
				if (!vd || !vd->GetDisp() || !vd->isBrxml())
					continue;
				flvr::Texture* tex = vd->GetTexture();
				if (!tex)
					continue;
				vector<flvr::TextureBrick*> *bricks = tex->get_bricks();
				if (!bricks || bricks->size() == 0)
					continue;
				list.push_back(vd);
			}

			vector<VolumeLoaderData> tmp_shade;
			vector<VolumeLoaderData> tmp_shadow;
			for (i = 0; i < list.size(); i++)
			{
				VolumeData* vd = list[i];
				flvr::Texture* tex = vd->GetTexture();
				fluo::Ray view_ray = vd->GetVR()->compute_view();
				vector<flvr::TextureBrick*> *bricks = tex->get_sorted_bricks(view_ray, !m_persp);
				int mode = vd->GetMode() == 1 ? 1 : 0;
				bool shade = (mode == 1 && vd->GetShading());
				bool shadow = vd->GetShadow();
				for (j = 0; j < bricks->size(); j++)
				{
					VolumeLoaderData d;
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
			if (flvr::TextureRenderer::get_update_order() == 1)
				std::sort(queues.begin(), queues.end(), VolumeLoader::sort_data_dsc);
			else if (flvr::TextureRenderer::get_update_order() == 0)
				std::sort(queues.begin(), queues.end(), VolumeLoader::sort_data_asc);

			if (!tmp_shade.empty())
			{
				if (flvr::TextureRenderer::get_update_order() == 1)
					std::sort(tmp_shade.begin(), tmp_shade.end(), VolumeLoader::sort_data_dsc);
				else if (flvr::TextureRenderer::get_update_order() == 0)
					std::sort(tmp_shade.begin(), tmp_shade.end(), VolumeLoader::sort_data_asc);
				queues.insert(queues.end(), tmp_shade.begin(), tmp_shade.end());
			}
			if (!tmp_shadow.empty())
			{
				if (flvr::TextureRenderer::get_update_order() == 1)
				{
					int order = flvr::TextureRenderer::get_update_order();
					flvr::TextureRenderer::set_update_order(0);
					for (i = 0; i < list.size(); i++)
					{
						fluo::Ray view_ray = list[i]->GetVR()->compute_view();
						list[i]->GetTexture()->set_sort_bricks();
						list[i]->GetTexture()->get_sorted_bricks(view_ray, !m_persp); //recalculate brick.d_
						list[i]->GetTexture()->set_sort_bricks();
					}
					flvr::TextureRenderer::set_update_order(order);
					std::sort(tmp_shadow.begin(), tmp_shadow.end(), VolumeLoader::sort_data_asc);
				}
				else if (flvr::TextureRenderer::get_update_order() == 0)
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
					vector<VolumeLoaderData> tmp_shade;
					vector<VolumeLoaderData> tmp_shadow;
					if (vd && vd->GetDisp() && vd->isBrxml())
					{
						flvr::Texture* tex = vd->GetTexture();
						if (!tex)
							continue;
						fluo::Ray view_ray = vd->GetVR()->compute_view();
						vector<flvr::TextureBrick*> *bricks = tex->get_sorted_bricks(view_ray, !m_persp);
						if (!bricks || bricks->size() == 0)
							continue;
						int mode = vd->GetMode() == 1 ? 1 : 0;
						bool shade = (mode == 1 && vd->GetShading());
						bool shadow = vd->GetShadow();
						for (j = 0; j<bricks->size(); j++)
						{
							VolumeLoaderData d;
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
							if (flvr::TextureRenderer::get_update_order() == 1)
							{
								int order = flvr::TextureRenderer::get_update_order();
								flvr::TextureRenderer::set_update_order(0);
								fluo::Ray view_ray = vd->GetVR()->compute_view();
								tex->set_sort_bricks();
								tex->get_sorted_bricks(view_ray, !m_persp); //recalculate brick.d_
								tex->set_sort_bricks();
								flvr::TextureRenderer::set_update_order(order);
								std::sort(tmp_shadow.begin(), tmp_shadow.end(), VolumeLoader::sort_data_asc);
							}
							queues.insert(queues.end(), tmp_shadow.begin(), tmp_shadow.end());
						}
					}
				}
				break;
				case 5://group
				{
					vector<VolumeData*> list;
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
						vector<flvr::TextureBrick*> *bricks = tex->get_sorted_bricks(view_ray, !m_persp);
						if (!bricks || bricks->size() == 0)
							continue;
						list.push_back(vd);
					}
					if (list.empty())
						continue;

					vector<VolumeLoaderData> tmp_q;
					vector<VolumeLoaderData> tmp_shade;
					vector<VolumeLoaderData> tmp_shadow;
					if (group->GetBlendMode() == VOL_METHOD_MULTI)
					{
						for (k = 0; k < list.size(); k++)
						{
							VolumeData* vd = list[k];
							flvr::Texture* tex = vd->GetTexture();
							fluo::Ray view_ray = vd->GetVR()->compute_view();
							vector<flvr::TextureBrick*> *bricks = tex->get_sorted_bricks(view_ray, !m_persp);
							int mode = vd->GetMode() == 1 ? 1 : 0;
							bool shade = (mode == 1 && vd->GetShading());
							bool shadow = vd->GetShadow();
							for (j = 0; j < bricks->size(); j++)
							{
								VolumeLoaderData d;
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
							if (flvr::TextureRenderer::get_update_order() == 1)
								std::sort(tmp_q.begin(), tmp_q.end(), VolumeLoader::sort_data_dsc);
							else if (flvr::TextureRenderer::get_update_order() == 0)
								std::sort(tmp_q.begin(), tmp_q.end(), VolumeLoader::sort_data_asc);
							queues.insert(queues.end(), tmp_q.begin(), tmp_q.end());
						}
						if (!tmp_shade.empty())
						{
							if (flvr::TextureRenderer::get_update_order() == 1)
								std::sort(tmp_shade.begin(), tmp_shade.end(), VolumeLoader::sort_data_dsc);
							else if (flvr::TextureRenderer::get_update_order() == 0)
								std::sort(tmp_shade.begin(), tmp_shade.end(), VolumeLoader::sort_data_asc);
							queues.insert(queues.end(), tmp_shade.begin(), tmp_shade.end());
						}
						if (!tmp_shadow.empty())
						{
							if (flvr::TextureRenderer::get_update_order() == 1)
							{
								int order = flvr::TextureRenderer::get_update_order();
								flvr::TextureRenderer::set_update_order(0);
								for (k = 0; k < list.size(); k++)
								{
									fluo::Ray view_ray = list[k]->GetVR()->compute_view();
									list[i]->GetTexture()->set_sort_bricks();
									list[i]->GetTexture()->get_sorted_bricks(view_ray, !m_persp); //recalculate brick.d_
									list[i]->GetTexture()->set_sort_bricks();
								}
								flvr::TextureRenderer::set_update_order(order);
								std::sort(tmp_shadow.begin(), tmp_shadow.end(), VolumeLoader::sort_data_asc);
							}
							else if (flvr::TextureRenderer::get_update_order() == 0)
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
							vector<flvr::TextureBrick*> *bricks = tex->get_sorted_bricks(view_ray, !m_persp);
							int mode = vd->GetMode() == 1 ? 1 : 0;
							bool shade = (mode == 1 && vd->GetShading());
							bool shadow = vd->GetShadow();
							for (k = 0; k<bricks->size(); k++)
							{
								VolumeLoaderData d;
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
								if (flvr::TextureRenderer::get_update_order() == 1)
								{
									int order = flvr::TextureRenderer::get_update_order();
									flvr::TextureRenderer::set_update_order(0);
									fluo::Ray view_ray = vd->GetVR()->compute_view();
									tex->set_sort_bricks();
									tex->get_sorted_bricks(view_ray, !m_persp); //recalculate brick.d_
									tex->set_sort_bricks();
									flvr::TextureRenderer::set_update_order(order);
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
			m_loader.Set(queues);
			m_loader.SetMemoryLimitByte((long long)flvr::TextureRenderer::mainmem_buf_size_ * 1024LL * 1024LL);
			flvr::TextureRenderer::set_load_on_main_thread(false);
			m_loader.Run();
		}

		if (total_num > 0)
		{
			flvr::TextureRenderer::set_update_loop();
			if (m_draw_type == 1)
				flvr::TextureRenderer::set_total_brick_num(total_num);
			else if (m_draw_type == 2)
				flvr::TextureRenderer::set_total_brick_num(total_num*(m_peeling_layers+1));
			flvr::TextureRenderer::reset_done_current_chan();
		}
	}
}

//halt loop update
void VRenderGLView::HaltLoopUpdate()
{
	if (flvr::TextureRenderer::get_mem_swap())
	{
		flvr::TextureRenderer::reset_update_loop();
	}
}

//new function to refresh
void VRenderGLView::RefreshGL(int debug_code,
	bool erase,
	bool start_loop)
{
	//m_force_clear = force_clear;
	//m_interactive = interactive;

	//for debugging refresh events
	DBGPRINT(L"%d\trefresh\t%d\t%d\n", m_vrv->m_id, debug_code, m_interactive);
	m_updating = true;
	if (start_loop)
		StartLoopUpdate();
	SetSortBricks();
	m_refresh = true;
	Refresh(erase);
}

void VRenderGLView::DrawRulers()
{
	if (m_ruler_list.empty())
		return;
	double width = 1.0;
	if (m_frame && m_frame->GetSettingDlg())
		width = m_frame->GetSettingDlg()->GetLineWidth();
	m_ruler_renderer.SetLineSize(width);
	m_ruler_renderer.Draw();
}

flrd::RulerList* VRenderGLView::GetRulerList()
{
	return &m_ruler_list;
}

flrd::Ruler* VRenderGLView::GetRuler(unsigned int id)
{
	m_cur_ruler = 0;
	for (size_t i = 0; i < m_ruler_list.size(); ++i)
	{
		if (m_ruler_list[i] && m_ruler_list[i]->Id() == id)
		{
			m_cur_ruler = m_ruler_list[i];
			break;
		}
	}
	return m_cur_ruler;
}

//draw highlighted comps
void VRenderGLView::DrawCells()
{
	if (m_cell_list.empty())
		return;
	double width = 1.0;
	if (m_frame && m_frame->GetSettingDlg())
		width = m_frame->GetSettingDlg()->GetLineWidth();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	flvr::ShaderProgram* shader =
		flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHDR_DRAW_THICK_LINES);
	if (shader)
	{
		if (!shader->valid())
			shader->create();
		shader->bind();
	}
	glm::mat4 matrix = glm::ortho(float(0),
		float(GetGLSize().x), float(0), float(GetGLSize().y));
	shader->setLocalParamMatrix(0, glm::value_ptr(matrix));
	shader->setLocalParam(0, GetSize().x, GetSize().y, width, 0.0);

	flvr::VertexArray* va_rulers =
		flvr::TextureRenderer::vertex_array_manager_.vertex_array(flvr::VA_Rulers);
	if (va_rulers)
	{
		vector<float> verts;
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

unsigned int VRenderGLView::DrawCellVerts(vector<float>& verts)
{
	float w = flvr::TextRenderer::text_texture_manager_.GetSize() / 4.0f;
	float px, py;

	fluo::Transform mv;
	fluo::Transform p;
	mv.set(glm::value_ptr(m_mv_mat));
	p.set(glm::value_ptr(m_proj_mat));

	//estimate
	int vert_num = m_cell_list.size();
	verts.reserve(vert_num * 8 * 3 * 2);

	unsigned int num = 0;
	fluo::Point p1, p2, p3, p4;
	fluo::Color c = GetTextColor();
	double sx, sy, sz;
	sx = m_cell_list.sx;
	sy = m_cell_list.sy;
	sz = m_cell_list.sz;
	for (auto it = m_cell_list.begin();
		it != m_cell_list.end(); ++it)
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

void VRenderGLView::GetCellPoints(fluo::BBox& box,
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

	int nx = GetGLSize().x;
	int ny = GetGLSize().y;

	p1 = fluo::Point((minx+1)*nx/2, (miny+1)*ny/2, 0.0);
	p2 = fluo::Point((maxx+1)*nx/2, (miny+1)*ny/2, 0.0);
	p3 = fluo::Point((maxx+1)*nx/2, (maxy+1)*ny/2, 0.0);
	p4 = fluo::Point((minx+1)*nx/2, (maxy+1)*ny/2, 0.0);
}

//traces
TraceGroup* VRenderGLView::GetTraceGroup()
{
	return m_trace_group;
}

void VRenderGLView::CreateTraceGroup()
{
	if (m_trace_group)
		delete m_trace_group;

	m_trace_group = new TraceGroup;
}

int VRenderGLView::LoadTraceGroup(wxString filename)
{
	if (m_trace_group)
		delete m_trace_group;

	m_trace_group = new TraceGroup;
	return m_trace_group->Load(filename);
}

int VRenderGLView::SaveTraceGroup(wxString filename)
{
	if (m_trace_group)
		return m_trace_group->Save(filename);
	else
		return 0;
}

void VRenderGLView::ExportTrace(wxString filename, unsigned int id)
{
	if (!m_trace_group)
		return;
}

void VRenderGLView::DrawTraces()
{
	if (m_cur_vol &&
		m_trace_group)
	{
		double width = 1.0;
		if (m_frame && m_frame->GetSettingDlg())
			width = m_frame->GetSettingDlg()->GetLineWidth();

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
			flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHDR_DRAW_THICK_LINES);
		if (shader)
		{
			if (!shader->valid())
				shader->create();
			shader->bind();
		}
		shader->setLocalParamMatrix(0, glm::value_ptr(matrix));
		shader->setLocalParam(0, GetSize().x, GetSize().y, width, 0.0);

		flvr::VertexArray* va_traces =
			flvr::TextureRenderer::vertex_array_manager_.vertex_array(flvr::VA_Traces);
		if (va_traces)
		{
			if (va_traces->get_dirty())
			{
				vector<float> verts;
				unsigned int num = m_trace_group->Draw(verts, m_cur_vol->GetShuffle());
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

void VRenderGLView::GetTraces(bool update)
{
	if (!m_trace_group)
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
				sel_labels.insert(pair<unsigned int, flrd::Celp>
					(label_value, cell));
			}
			else
			{
				label_iter->second->Inc(ii, jj, kk, 1.0f);
			}
		}
	}

	//create id list
	m_trace_group->SetCurTime(m_tseq_cur_num);
	m_trace_group->SetPrvTime(m_tseq_cur_num);
	m_trace_group->UpdateCellList(sel_labels);
	//m_trace_group->SetPrvTime(m_tseq_prv_num);

	//add traces to trace dialog
	if (update)
	{
		if (m_vrv && m_frame && m_frame->GetTraceDlg())
			m_frame->GetTraceDlg()->GetSettings(m_vrv->m_glview);
	}
}

#ifdef _WIN32
WXLRESULT VRenderGLView::MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam)
{
	if (m_selector.GetBrushUsePres())
	{
		if (message == WT_PACKET)
		{
			PACKET pkt;
			if (gpWTPacket((HCTX)lParam, wParam, &pkt))
				m_selector.SetPressure(pkt.pkNormalPressure, pkt.pkTangentPressure);
		}
	}

	if (m_enable_touch)
	{
		if (message == WM_POINTERDOWN)
		{
			POINTER_INFO pointerinfo = {};
			// get pointer info
			if (GetPI(GET_POINTERID_WPARAM(
				wParam), &pointerinfo))
			{
				int id = pointerinfo.pointerId;
				if (m_ptr_id1 < 0)
				{
					m_ptr_id1 = id;
					m_ptr1_x = pointerinfo.ptPixelLocation.x;
					m_ptr1_y = pointerinfo.ptPixelLocation.y;
					m_ptr1_x_save = m_ptr1_x;
					m_ptr1_y_save = m_ptr1_y;
				}
				else if (m_ptr_id2 < 0 &&
					m_ptr_id1 != id)
				{
					m_ptr_id2 = id;
					m_ptr2_x = pointerinfo.ptPixelLocation.x;
					m_ptr2_y = pointerinfo.ptPixelLocation.y;
					m_ptr2_x_save = m_ptr2_x;
					m_ptr2_y_save = m_ptr2_y;
				}
				if (m_ptr_id1 >= 0 &&
					m_ptr_id2 >= 0)
				{
					//simulate middle button
					wxMouseEvent me(wxEVT_MIDDLE_DOWN);
					int ptx = (m_ptr1_x + m_ptr2_x) / 2;
					int pty = (m_ptr1_y + m_ptr2_y) / 2;
					me.SetX(ptx);
					me.SetY(pty);
					ProcessWindowEvent(me);
				}
			}
		}
		else if (message == WM_POINTERUP)
		{
			POINTER_INFO pointerinfo = {};
			// get pointer info
			if (GetPI(GET_POINTERID_WPARAM(
				wParam), &pointerinfo))
			{
				int id = pointerinfo.pointerId;
				if (m_ptr_id1 == id)
					m_ptr_id1 = -1;
				else if (m_ptr_id2 == id)
					m_ptr_id2 = -1;
				if (m_ptr_id1 < 0 &&
					m_ptr_id2 < 0)
				{
					//simulate middle button
					wxMouseEvent me(wxEVT_MIDDLE_UP);
					ProcessWindowEvent(me);
				}
			}
		}
		else if (message == WM_POINTERUPDATE)
		{
			POINTER_INFO pointerinfo = {};
			// get pointer info
			if (GetPI(GET_POINTERID_WPARAM(
				wParam), &pointerinfo))
			{
				int id = pointerinfo.pointerId;
				if (m_ptr_id1 == id)
				{
					m_ptr1_x = pointerinfo.ptPixelLocation.x;
					m_ptr1_y = pointerinfo.ptPixelLocation.y;
				}
				else if (m_ptr_id2 == id)
				{
					m_ptr2_x = pointerinfo.ptPixelLocation.x;
					m_ptr2_y = pointerinfo.ptPixelLocation.y;
				}
			}

			if (m_ptr_id1 > 0 &&
				m_ptr_id2 > 0)
			{
				int dist1 = abs(m_ptr1_x_save - m_ptr2_x_save) +
					abs(m_ptr1_y_save - m_ptr2_y_save);
				int dist2 = abs(m_ptr1_x - m_ptr2_x) +
					abs(m_ptr1_y - m_ptr2_y);
				if (abs(dist1 - dist2) < 20)
				{
					wxMouseEvent me(wxEVT_MOTION);
					me.SetMiddleDown(true);
					int ptx = (m_ptr1_x + m_ptr2_x) / 2;
					int pty = (m_ptr1_y + m_ptr2_y) / 2;
					me.SetX(ptx);
					me.SetY(pty);
					ProcessWindowEvent(me);

				}
			}
		}
	}

	return wxWindow::MSWWindowProc(message, wParam, lParam);
}
#endif

void VRenderGLView::OnMouse(wxMouseEvent& event)
{
	if (m_interactive && !m_rot_lock)
		return;
	//if (m_drawing) return;
	wxWindow *window = wxWindow::FindFocus();
	//if (window &&
	//	(window->GetClassInfo()->
	//		IsKindOf(CLASSINFO(wxTextCtrl)) ||
	//		window->GetClassInfo()->
	//		IsKindOf(CLASSINFO(wxComboBox))) &&
	//		(event.LeftDown() ||
	//			event.RightDown() ||
	//			event.MiddleDown() ||
	//			event.LeftUp() ||
	//			event.MiddleUp() ||
	//			event.RightUp() ||
	//			event.Dragging() ||
	//			event.GetWheelRotation()))
	if (window &&
		(event.LeftDown() ||
		event.RightDown() ||
		event.MiddleDown() ||
		event.LeftUp() ||
		event.MiddleUp() ||
		event.RightUp() ||
		event.Dragging() ||
		event.GetWheelRotation()))
		SetFocus();

	m_interactive = false;

	m_paint_enable = false;
	m_retain_finalbuffer = false;
	int nx = GetGLSize().x;
	int ny = GetGLSize().y;
	fluo::Point mp = GetMousePos(event);

	//mouse button down operations
	m_ruler_handler.SetVolumeData(m_cur_vol);
	if (event.LeftDown())
	{
		bool found_rp = false;
		if (m_int_mode == 6 ||
			m_int_mode == 9 ||
			m_int_mode == 11 ||
			m_int_mode == 14)
		{
			found_rp = m_ruler_handler.FindEditingRuler(
				mp.x(), mp.y());
		}
		if (found_rp)
		{
			if (m_int_mode == 11)
			{
				flrd::RulerPoint *p = m_ruler_handler.GetPoint();
				if (p) p->ToggleLocked();
			}
			if (m_int_mode == 14)
				m_ruler_handler.DeletePoint();
			if (m_frame && m_frame->GetMeasureDlg())
				m_frame->GetMeasureDlg()->GetSettings(this);
			RefreshGL(41);
		}

		if (m_int_mode == 1 ||
			(m_int_mode == 5 &&
			event.AltDown()) ||
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
			m_selector.SetBrushPressPeak(0.0);
			RefreshGL(26);
		}

		if (m_int_mode == 10 ||
			m_int_mode == 12)
		{
			m_selector.ResetMousePos();
			m_selector.SetInitMask(1);
			Segment();
			m_selector.SetInitMask(3);
			if (m_int_mode == 12)
				m_cur_vol->AddEmptyLabel(0, false);
			m_force_clear = true;
			m_grow_on = true;
		}

		return;
	}
	if (event.RightDown())
	{
		old_mouse_X = mp.x();
		old_mouse_Y = mp.y();
		return;
	}
	if (event.MiddleDown())
	{
		old_mouse_X = mp.x();
		old_mouse_Y = mp.y();
		return;
	}

	//mouse button up operations
	if (event.LeftUp())
	{
		if (m_int_mode == 1)
		{
			//pick stuff
			if (m_pick)
			{
				Pick();
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
			Segment();
			m_int_mode = 4;
			m_force_clear = true;
			RefreshGL(27);
			return;
		}
		else if (m_int_mode == 5 &&
			!event.AltDown())
		{
			//add one point to a ruler
			m_ruler_handler.AddRulerPoint(mp.x(), mp.y(), true);
			if (m_frame && m_frame->GetMeasureDlg())
				m_frame->GetMeasureDlg()->GetSettings(this);
			RefreshGL(27);
			return;
		}
		else if (m_int_mode == 6 ||
			m_int_mode == 9 ||
			m_int_mode ==11)
		{
			m_ruler_handler.SetPoint(0);
		}
		else if (m_int_mode == 7)
		{
			//segment volume, calculate center, add ruler point
			m_paint_enable = true;
			Segment();
			if (m_ruler_handler.GetType() == 3)
				m_ruler_handler.AddRulerPoint(mp.x(), mp.y(), true);
			else
				m_ruler_handler.AddPaintRulerPoint();
			m_int_mode = 8;
			m_force_clear = true;
			RefreshGL(27);
			if (m_frame && m_frame->GetMeasureDlg())
				m_frame->GetMeasureDlg()->GetSettings(this);
			return;
		}
		else if (m_int_mode == 10 ||
			m_int_mode == 12)
		{
			m_grow_on = false;
			return;
		}
		else if (m_int_mode == 13)
		{
			if (m_frame && m_frame->GetMeasureDlg())
			{
				if (m_ruler_autorelax)
				{
					m_frame->GetMeasureDlg()->SetEdit();
					m_frame->GetMeasureDlg()->Relax(
						m_ruler_handler.GetRulerIndex());
				}
				m_frame->GetMeasureDlg()->GetSettings(this);
			}
			RefreshGL(29);
			return;
		}
	}
	if (event.MiddleUp())
	{
		//SetSortBricks();
		//RefreshGL(28);
		return;
	}
	if (event.RightUp())
	{
		if (m_int_mode == 1)
		{
			//RefreshGL(27);
			//return;
		}
		if (m_int_mode == 5 &&
			!event.AltDown())
		{
			if (m_ruler_handler.GetRulerFinished())
			{
				SetIntMode(1);
			}
			else
			{
				m_ruler_handler.AddRulerPoint(mp.x(), mp.y(), true);
				m_ruler_handler.FinishRuler();
			}
			if (m_frame && m_frame->GetMeasureDlg())
			{
				if (m_ruler_autorelax)
				{
					m_frame->GetMeasureDlg()->SetEdit();
					m_frame->GetMeasureDlg()->Relax(
						m_ruler_handler.GetRulerIndex());
				}
				m_frame->GetMeasureDlg()->GetSettings(this);
			}
			RefreshGL(29);
			return;
		}
		//SetSortBricks();
	}

	//mouse dragging
	if (event.Dragging())
	{
		flvr::TextureRenderer::set_cor_up_time(
			int(sqrt(double(old_mouse_X - mp.x())*
				double(old_mouse_X - mp.x()) +
				double(old_mouse_Y - mp.y())*
				double(old_mouse_Y - mp.y()))));

		flrd::RulerPoint *p0 = m_ruler_handler.GetPoint();
		bool hold_old = false;
		if (m_int_mode == 1 ||
			(m_int_mode == 5 &&
			event.AltDown()) ||
			((m_int_mode == 6 ||
			m_int_mode == 9 ||
			m_int_mode == 10 ||
			m_int_mode == 11 ||
			m_int_mode == 12 ||
			m_int_mode == 14) &&
			!p0))
		{
			//disable picking
			m_pick = false;

			if (old_mouse_X != -1 &&
				old_mouse_Y != -1 &&
				abs(old_mouse_X - mp.x()) +
				abs(old_mouse_Y - mp.y())<200)
			{
				if (event.LeftIsDown() &&
					!event.ControlDown() &&
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

					wxString str = wxString::Format("%.1f", m_rotx);
					m_vrv->m_x_rot_text->ChangeValue(str);
					str = wxString::Format("%.1f", m_roty);
					m_vrv->m_y_rot_text->ChangeValue(str);
					str = wxString::Format("%.1f", m_rotz);
					m_vrv->m_z_rot_text->ChangeValue(str);
					if (!m_vrv->m_rot_slider)
					{
						m_vrv->m_x_rot_sldr->SetThumbPosition(int(m_rotx));
						m_vrv->m_y_rot_sldr->SetThumbPosition(int(m_roty));
						m_vrv->m_z_rot_sldr->SetThumbPosition(int(m_rotz));
					}

					m_interactive = true;

					if (m_linked_rot)
						m_master_linked_view = this;

					if (!hold_old)
						RefreshGL(30);
				}
				if (event.MiddleIsDown() || (event.ControlDown() && event.LeftIsDown()))
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

					m_rot_center_dirty = true;

					//SetSortBricks();
					RefreshGL(31);
				}
				if (event.RightIsDown())
				{
					long dx = mp.x() - old_mouse_X;
					long dy = mp.y() - old_mouse_Y;

					double delta = abs(dx)>abs(dy) ?
						(double)dx / (double)nx :
						(double)-dy / (double)ny;
					m_scale_factor += m_scale_factor*delta;
					m_vrv->UpdateScaleFactor(false);
					//wxString str = wxString::Format("%.0f", m_scale_factor*100.0);
					//m_vrv->m_scale_factor_sldr->SetValue(m_scale_factor*100);
					//m_vrv->m_scale_factor_text->ChangeValue(str);

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
				if (event.LeftIsDown())
				{
					fluo::Quaternion q_delta = TrackballClip(old_mouse_X, mp.y(), mp.x(), old_mouse_Y);
					m_q_cl = q_delta * m_q_cl;
					m_q_cl.Normalize();
					SetRotations(m_rotx, m_roty, m_rotz);
					RefreshGL(34);
				}
			}
		}
		else if (m_int_mode == 6 || m_int_mode == 9)
		{
			bool rval = false;
			if (m_int_mode == 6)
				rval = m_ruler_handler.EditPoint(
					mp.x(), mp.y(), event.AltDown());
			else if (m_int_mode == 9)
				rval = m_ruler_handler.MoveRuler(
					mp.x(), mp.y());
			if (rval)
			{
				RefreshGL(35);
				if (m_frame && m_frame->GetMeasureDlg())
				{
					m_frame->GetMeasureDlg()->GetSettings(this);
					m_frame->GetMeasureDlg()->SetEdit();
				}
			}
		}
		else if (m_int_mode == 13)
		{
			if (m_ruler_handler.GetMouseDist(mp.x(), mp.y(), 35))
			{
				//add one point to a ruler
				m_ruler_handler.AddRulerPoint(mp.x(), mp.y(), true);
				if (m_frame && m_frame->GetMeasureDlg())
					m_frame->GetMeasureDlg()->GetSettings(this);
				RefreshGL(27);
			}
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
	int wheel = event.GetWheelRotation();
	if (wheel)  //if rotation
	{
		if (m_int_mode == 2 || m_int_mode == 7)
		{
			ChangeBrushSize(wheel);
		}
		else
		{
			m_interactive = true;
			m_rot_center_dirty = true;
			double value = wheel*m_scale_factor / 1000.0;
			if (m_scale_factor + value > 0.01)
				m_scale_factor += value;
			//if (m_scale_factor < 0.01)
			//	m_scale_factor = 0.01;
			m_vrv->UpdateScaleFactor(false);
			//wxString str = wxString::Format("%.0f", m_scale_factor*100.0);
			//m_vrv->m_scale_factor_sldr->SetValue(m_scale_factor*100);
			//m_vrv->m_scale_factor_text->ChangeValue(str);
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
		if (m_frame && m_frame->GetMovieView() &&
			m_frame->GetMovieView()->GetRunning())
			return;
		if (m_enable_vr)
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

void VRenderGLView::SetDraw(bool draw)
{
	m_draw_all = draw;
}

void VRenderGLView::ToggleDraw()
{
	m_draw_all = !m_draw_all;
}

bool VRenderGLView::GetDraw()
{
	return m_draw_all;
}

fluo::Color VRenderGLView::GetBackgroundColor()
{
	return m_bg_color;
}

fluo::Color VRenderGLView::GetTextColor()
{
	if (!m_frame || !m_frame->GetSettingDlg())
		return m_bg_color_inv;
	switch (m_frame->GetSettingDlg()->GetTextColor())
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

void VRenderGLView::SetBackgroundColor(fluo::Color &color)
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
	if (m_vrv)
	{
		wxColor c(int(color.r()*255.0), int(color.g()*255.0), int(color.b()*255.0));
		m_vrv->m_bg_color_picker->SetColour(c);
	}
}

void VRenderGLView::SetFog(bool b)
{
	m_use_fog = b;
	if (m_vrv)
		m_vrv->m_left_toolbar->ToggleTool(VRenderView::ID_DepthAttenChk, b);
}

void VRenderGLView::SetRotations(double rotx, double roty, double rotz, bool ui_update)
{
	m_rotx = rotx;
	m_roty = roty;
	m_rotz = rotz;

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

	if (ui_update)
	{
		wxString str = wxString::Format("%.1f", m_rotx);
		m_vrv->m_x_rot_text->ChangeValue(str);
		str = wxString::Format("%.1f", m_roty);
		m_vrv->m_y_rot_text->ChangeValue(str);
		str = wxString::Format("%.1f", m_rotz);
		m_vrv->m_z_rot_text->ChangeValue(str);
		if (m_vrv->m_rot_slider)
		{
			m_vrv->m_x_rot_sldr->SetThumbPosition(180);
			m_vrv->m_y_rot_sldr->SetThumbPosition(180);
			m_vrv->m_z_rot_sldr->SetThumbPosition(180);
		}
		else
		{
			m_vrv->m_x_rot_sldr->SetThumbPosition(int(m_rotx));
			m_vrv->m_y_rot_sldr->SetThumbPosition(int(m_roty));
			m_vrv->m_z_rot_sldr->SetThumbPosition(int(m_rotz));
		}
	}

	if (m_linked_rot)
	{
		if (!m_master_linked_view)
			m_master_linked_view = this;
	}
}

void VRenderGLView::SetZeroRotations()
{
	m_zq = m_q;
}

void VRenderGLView::ResetZeroRotations(double &rotx, double &roty, double &rotz)
{
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
}

void VRenderGLView::GetFrame(int &x, int &y, int &w, int &h)
{
	x = m_frame_x;
	y = m_frame_y;
	w = m_frame_w;
	h = m_frame_h;
}

void VRenderGLView::CalcFrame()
{
	int w, h;
	w = GetGLSize().x;
	h = GetGLSize().y;

	if (m_cur_vol)
	{
		//projection
		HandleProjection(w, h);
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
		vector<fluo::Point> points;
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

		m_frame_x = int((minx + 1.0)*w / 2.0 + 1.0);
		m_frame_y = int((miny + 1.0)*h / 2.0 + 1.0);
		m_frame_w = int((maxx - minx)*w / 2.0 - 1.5);
		m_frame_h = int((maxy - miny)*h / 2.0 - 1.5);

	}
	else
	{
		int size;
		if (w > h)
		{
			size = h;
			m_frame_x = int((w - h) / 2.0);
			m_frame_y = 0;
		}
		else
		{
			size = w;
			m_frame_x = 0;
			m_frame_y = int((h - w) / 2.0);
		}
		m_frame_w = m_frame_h = size;
	}
}

void VRenderGLView::DrawViewQuad()
{
	flvr::VertexArray* quad_va =
		flvr::TextureRenderer::vertex_array_manager_.vertex_array(flvr::VA_Norm_Square);
	if (quad_va)
		quad_va->draw();
}

void VRenderGLView::switchLevel(VolumeData *vd)
{
	if (!vd) return;

	int nx, ny;
	GetRenderSize(nx, ny);
	if (m_enlarge)
	{
		nx = int(nx * m_enlarge_scale);
		ny = int(ny * m_enlarge_scale);
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
			vector<double> sfs;
			vector<double> spx, spy, spz;
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
			if (m_frame && m_frame->GetSettingDlg())
				lv += m_frame->GetSettingDlg()->GetDetailLevelOffset();
			if (lv < 0) lv = 0;
			//if (m_interactive) lv += 1;
			if (lv >= lvnum) lv = lvnum - 1;
			new_lv = lv;
		}
		if (prev_lv != new_lv)
		{
			vector<flvr::TextureBrick*> *bricks = vtex->get_bricks();
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

#ifdef _WIN32
int VRenderGLView::GetPixelFormat(PIXELFORMATDESCRIPTOR *pfd) {
	int pixelFormat = ::GetPixelFormat(m_hDC);
	if (pixelFormat == 0) return GetLastError();
	pixelFormat = DescribePixelFormat(m_hDC, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), pfd);
	if (pixelFormat == 0) return GetLastError();
	return pixelFormat;
}
#endif

wxString VRenderGLView::GetOGLVersion() {
	return m_GLversion;
}

void VRenderGLView::SetLockCenter(int type)
{
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

void VRenderGLView::SetLockCenterVol()
{
	if (!m_cur_vol)
		return;
	fluo::BBox box = m_cur_vol->GetClippedBounds();
	m_lock_center = box.center();
}

void VRenderGLView::SetLockCenterRuler()
{
	if (!m_cur_ruler)
		return;
	m_lock_center = m_cur_ruler->GetCenter();
}

void VRenderGLView::SetLockCenterSel()
{
	if (!m_cur_vol)
		return;
	flrd::Cov cover(m_cur_vol);
	cover.Compute(1);
	m_lock_center = cover.GetCenter();
}