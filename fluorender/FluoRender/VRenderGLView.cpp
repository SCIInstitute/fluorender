/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2014 Scientific Computing and Imaging Institute,
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

#include <boost/process.hpp>
#include "VRenderGLView.h"
#include "VRenderView.h"
#include "VRenderFrame.h"
#include <FLIVR/Framebuffer.h>
#include <FLIVR/VertexArray.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <wx/stdpaths.h>
#include "png_resource.h"
#include "img/icons.h"

ImgShaderFactory VRenderGLView::m_img_shader_factory;
bool VRenderGLView::m_linked_rot = false;
VRenderGLView* VRenderGLView::m_master_linked_view = 0;
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

VRenderGLView::VRenderGLView(wxWindow* frame,
	wxWindow* parent,
	wxWindowID id,
	const wxGLAttributes& attriblist,
	wxGLContext* sharedContext,
	const wxPoint& pos,
	const wxSize& size,
	long style) :
	wxGLCanvas(parent, attriblist, id, pos, size, style),
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
	m_scale_mode(true),
	//mode in determining depth of volume
	m_point_volume_mode(0),
	//ruler use volume transfer function
	m_ruler_use_transf(false),
	//ruler time dependent
	m_ruler_time_dep(true),
	//private
	m_frame(frame),
	m_vrv((VRenderView*)parent),
	//populated lists of data
	m_vd_pop_dirty(true),
	m_md_pop_dirty(true),
	//ruler type
	m_ruler_type(0),
	m_editing_ruler_point(0),
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
	//resizing
	m_resize(false),
	//brush tools
	m_draw_brush(false),
	m_paint_enable(false),
	m_paint_display(false),
	//paint buffer
	m_clear_paint(true),
	//pick buffer
	//m_fbo_pick(0),
	//m_tex_pick(0),
	//m_tex_pick_depth(0),
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
	//object translation
	m_obj_transx(0.0), m_obj_transy(0.0), m_obj_transz(0.0),
	m_obj_transx_saved(0.0), m_obj_transy_saved(0.0), m_obj_transz_saved(0.0),
	//object rotation
	m_obj_rotx(0.0), m_obj_roty(0.0), m_obj_rotz(0.0),
	m_rot_lock(false),
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
	m_rot_axis(0),
	m_movie_seq(0),
	m_rewind(false),
	m_fr_length(0),
	m_stages(0),
	m_4d_rewind(false),
	//movie frame properties
	m_frame_x(-1),
	m_frame_y(-1),
	m_frame_w(-1),
	m_frame_h(-1),
	//post image processing
	m_gamma(Color(1.0, 1.0, 1.0)),
	m_brightness(Color(1.0, 1.0, 1.0)),
	m_hdr(0.0, 0.0, 0.0),
	m_sync_r(false),
	m_sync_g(false),
	m_sync_b(false),
	//volume color map
	m_color_1(Color(0.0, 0.0, 1.0)),
	m_value_2(0.0),
	m_color_2(Color(0.0, 0.0, 1.0)),
	m_value_3(0.25),
	m_color_3(Color(0.0, 1.0, 1.0)),
	m_value_4(0.5),
	m_color_4(Color(0.0, 1.0, 0.0)),
	m_value_5(0.75),
	m_color_5(Color(1.0, 1.0, 0.0)),
	m_value_6(1.0),
	m_color_6(Color(1.0, 0.0, 0.0)),
	m_color_7(Color(1.0, 0.0, 0.0)),
	//paint brush presssure
	m_use_press(true),
	m_on_press(false),
	//paint stroke radius
	m_brush_radius1(10),
	m_brush_radius2(30),
	m_use_brush_radius2(true),
	//paint stroke spacing
	m_brush_spacing(0.1),
	//brush size relation
	m_brush_size_data(true),
	//clipping plane rotations
	m_rotx_cl(0), m_roty_cl(0), m_rotz_cl(0),
	m_pressure(0.0),
	m_press_peak(0.0),
	m_air_press(0.5),
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
	m_refresh(false)
{
	m_glRC = sharedContext;
	m_sharedRC = m_glRC ? true : false;

	m_sb_num = "50";
#ifdef _WIN32
	//tablet initialization
	if (m_use_press)
	{
		if (!m_hTab && LoadWintab() &&
			gpWTInfoA(0, 0, NULL))
		{
			m_hTab = TabletInit((HWND)GetHWND(), (HINSTANCE)::wxGetInstance());
		}
		else
			m_use_press = false;
	}

	//check touch
	HMODULE user32 = LoadLibrary(L"user32");
	GetPI = reinterpret_cast<decltype(GetPointerInfo)*>
		(GetProcAddress(user32, "GetPointerInfo"));
	if (GetPI != NULL)
		m_enable_touch = true;
	else
		m_enable_touch = false;
#endif
	LoadBrushSettings();

	m_timer = new nv::Timer(10);
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetBenchmark())
		m_benchmark = true;
	else
		m_benchmark = false;
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
	sprintf(m_lc.lcName, "FluoRender Digitizing %x", hInst);

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
		m_press_nmax = TabletNPress.axMax;
	else
		m_press_nmax = 1.0;

	wWTInfoRetVal = gpWTInfoA(WTI_DEVICES, DVC_TPRESSURE, &TabletTPress);
	if (wWTInfoRetVal == sizeof(AXIS))
		m_press_tmax = TabletTPress.axMax;
	else
		m_press_tmax = 1.0;

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

	SaveBrushSettings();

#ifdef _WIN32
	//tablet
	if (m_hTab)
	{
		gpWTClose(m_hTab);
		m_hTab = 0;
		UnloadWintab();
	}
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

	if (glIsBuffer(m_misc_vbo))
		glDeleteBuffers(1, &m_misc_vbo);
	if (glIsBuffer(m_misc_ibo))
		glDeleteBuffers(1, &m_misc_ibo);
	if (glIsVertexArray(m_misc_vao))
		glDeleteVertexArrays(1, &m_misc_vao);

	if (!m_sharedRC)
		delete m_glRC;

	if (m_trace_group)
		delete m_trace_group;

	if (m_full_screen)
	{
		m_full_screen = false;
		m_vrv->m_glview = 0;
		m_vrv->m_full_frame = 0;
		VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
		if (vr_frame)
		{
			vr_frame->ClearVrvList();
			vr_frame->Close();
		}
	}
}

void VRenderGLView::ResizeFramebuffers()
{
	m_resize = true;
}

void VRenderGLView::OnResize(wxSizeEvent& event)
{
	wxSize size = GetGLSize();
	if (m_size == size)
		return;
	else
		m_size = size;

	ResizeFramebuffers();

	m_vrv->UpdateScaleFactor(false);

	RefreshGL(1);
}

void VRenderGLView::Init()
{
	if (!m_initialized)
	{
		VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
		ShaderProgram::init_shaders_supported();
		if (vr_frame && vr_frame->GetSettingDlg())
			KernelProgram::set_device_id(vr_frame->
				GetSettingDlg()->GetCLDeviceID());
		KernelProgram::init_kernels_supported();
#ifdef _DARWIN
		CGLContextObj ctx = CGLGetCurrentContext();
		if (ctx != TextureRenderer::gl_context_)
			TextureRenderer::gl_context_ = ctx;
#endif
		if (vr_frame)
		{
			vr_frame->SetTextureRendererSettings();
			vr_frame->SetTextureUndos();
			vr_frame->GetSettingDlg()->UpdateTextureSize();
		}
		//glViewport(0, 0, (GLint)(GetSize().x), (GLint)(GetSize().y));
		glGenBuffers(1, &m_misc_vbo);
		glGenBuffers(1, &m_misc_ibo);
		glGenVertexArrays(1, &m_misc_vao);
		glEnable(GL_MULTISAMPLE);

		m_initialized = true;

		m_timer->start();
	}
}

void VRenderGLView::Clear()
{
	m_loader.RemoveAllLoadedBrick();
	TextureRenderer::clear_tex_pool();

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
}

void VRenderGLView::HandleProjection(int nx, int ny)
{
	double aspect = (double)nx / (double)ny;
	if (!m_free)
		m_distance = m_radius / tan(d2r(m_aov / 2.0)) / m_scale_factor;
	if (aspect>1.0)
	{
		m_ortho_left = -m_radius*aspect / m_scale_factor;
		m_ortho_right = -m_ortho_left;
		m_ortho_bottom = -m_radius / m_scale_factor;
		m_ortho_top = -m_ortho_bottom;
	}
	else
	{
		m_ortho_left = -m_radius / m_scale_factor;
		m_ortho_right = -m_ortho_left;
		m_ortho_bottom = -m_radius / aspect / m_scale_factor;
		m_ortho_top = -m_ortho_bottom;
	}
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

void VRenderGLView::HandleCamera()
{
	Vector pos(m_transx, m_transy, m_transz);
	pos.normalize();
	if (m_free)
		pos *= 0.1;
	else
		pos *= m_distance;
	m_transx = pos.x();
	m_transy = pos.y();
	m_transz = pos.z();

	if (m_free)
		m_mv_mat = glm::lookAt(glm::vec3(m_transx + m_ctrx, m_transy + m_ctry, m_transz + m_ctrz),
			glm::vec3(m_ctrx, m_ctry, m_ctrz),
			glm::vec3(m_up.x(), m_up.y(), m_up.z()));
	else
		m_mv_mat = glm::lookAt(glm::vec3(m_transx, m_transy, m_transz),
			glm::vec3(0.0), glm::vec3(m_up.x(), m_up.y(), m_up.z()));
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
	BBox bbox;
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
		Transform mv;
		mv.set(glm::value_ptr(m_mv_mat));

		double minz, maxz;
		minz = numeric_limits<double>::max();
		maxz = -numeric_limits<double>::max();

		vector<Point> points;
		points.push_back(Point(bbox.min().x(), bbox.min().y(), bbox.min().z()));
		points.push_back(Point(bbox.min().x(), bbox.min().y(), bbox.max().z()));
		points.push_back(Point(bbox.min().x(), bbox.max().y(), bbox.min().z()));
		points.push_back(Point(bbox.min().x(), bbox.max().y(), bbox.max().z()));
		points.push_back(Point(bbox.max().x(), bbox.min().y(), bbox.min().z()));
		points.push_back(Point(bbox.max().x(), bbox.min().y(), bbox.max().z()));
		points.push_back(Point(bbox.max().x(), bbox.max().y(), bbox.min().z()));
		points.push_back(Point(bbox.max().x(), bbox.max().y(), bbox.max().z()));

		Point p;
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
	int nx = GetGLSize().x;
	int ny = GetGLSize().y;

	// clear color and depth buffers
	glClearDepth(1.0);
	glClearColor(m_bg_color.r(), m_bg_color.g(), m_bg_color.b(), 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, (GLint)nx, (GLint)ny);

	//gradient background
	if (m_grad_bg)
		DrawGradBg();

	//projection
	HandleProjection(nx, ny);
	//Transformation
	HandleCamera();

	if (m_draw_all)
	{
		glm::mat4 mv_temp = m_mv_mat;
		//translate object
		m_mv_mat = glm::translate(m_mv_mat, glm::vec3(m_obj_transx, m_obj_transy, m_obj_transz));
		//rotate object
		m_mv_mat = glm::rotate(m_mv_mat, float(glm::radians(m_obj_rotx)), glm::vec3(1.0, 0.0, 0.0));
		m_mv_mat = glm::rotate(m_mv_mat, float(glm::radians(m_obj_roty + 180.0)), glm::vec3(0.0, 1.0, 0.0));
		m_mv_mat = glm::rotate(m_mv_mat, float(glm::radians(m_obj_rotz + 180.0)), glm::vec3(0.0, 0.0, 1.0));
		//center object
		m_mv_mat = glm::translate(m_mv_mat, glm::vec3(-m_obj_ctrx, -m_obj_ctry, -m_obj_ctrz));

		if (m_use_fog)
			CalcFogRange();

		if (m_draw_grid)
			DrawGrid();

		if (m_draw_clip)
			DrawClippingPlanes(false, BACK_FACE);

		//setup
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//draw the volumes
		DrawVolumes();

		//draw the clipping planes
		if (m_draw_clip)
			DrawClippingPlanes(true, FRONT_FACE);

		if (m_draw_bounds)
			DrawBounds();

		if (m_draw_annotations)
			DrawAnnotations();

		if (m_draw_rulers)
			DrawRulers();

		//traces
		DrawTraces();

		m_mv_mat = mv_temp;
	}
}

//draw with depth peeling
void VRenderGLView::DrawDP()
{
	int i;
	int nx = GetGLSize().x;
	int ny = GetGLSize().y;
	string name;
	Framebuffer* peel_buffer = 0;

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
	HandleProjection(nx, ny);
	//Transformation
	HandleCamera();

	if (m_draw_all)
	{
		glm::mat4 mv_temp = m_mv_mat;
		//translate object
		m_mv_mat = glm::translate(m_mv_mat, glm::vec3(m_obj_transx, m_obj_transy, m_obj_transz));
		//rotate object
		m_mv_mat = glm::rotate(m_mv_mat, float(glm::radians(m_obj_rotx)), glm::vec3(1.0, 0.0, 0.0));
		m_mv_mat = glm::rotate(m_mv_mat, float(glm::radians(m_obj_roty + 180.0)), glm::vec3(0.0, 1.0, 0.0));
		m_mv_mat = glm::rotate(m_mv_mat, float(glm::radians(m_obj_rotz + 180.0)), glm::vec3(0.0, 0.0, 1.0));
		//center object
		m_mv_mat = glm::translate(m_mv_mat, glm::vec3(-m_obj_ctrx, -m_obj_ctry, -m_obj_ctrz));

		bool use_fog_save = m_use_fog;
		if (m_use_fog)
			CalcFogRange();

		if (m_draw_grid)
			DrawGrid();

		if (m_draw_clip)
			DrawClippingPlanes(true, BACK_FACE);

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
				TextureRenderer::framebuffer_manager_.framebuffer(
					FB_Depth_Float, nx, ny, name);
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
					TextureRenderer::framebuffer_manager_.framebuffer(name);
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
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

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
					TextureRenderer::framebuffer_manager_.framebuffer(name);
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
						TextureRenderer::framebuffer_manager_.framebuffer(name);
					if (peel_buffer)
						peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
					glActiveTexture(GL_TEXTURE0);
				}
				else if (m_peeling_layers == 2)
				{
					glActiveTexture(GL_TEXTURE14);
					name = "peel buffer" + std::to_string(0);
					peel_buffer =
						TextureRenderer::framebuffer_manager_.framebuffer(name);
					if (peel_buffer)
						peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
					glActiveTexture(GL_TEXTURE15);
					name = "peel buffer" + std::to_string(1);
					peel_buffer =
						TextureRenderer::framebuffer_manager_.framebuffer(name);
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
							TextureRenderer::framebuffer_manager_.framebuffer(name);
						if (peel_buffer)
							peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
						glActiveTexture(GL_TEXTURE15);
						name = "peel buffer" + std::to_string(i-1);
						peel_buffer =
							TextureRenderer::framebuffer_manager_.framebuffer(name);
						if (peel_buffer)
							peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
						glActiveTexture(GL_TEXTURE0);
					}
					else if (i == 1)
					{
						glActiveTexture(GL_TEXTURE14);
						name = "peel buffer" + std::to_string(0);
						peel_buffer =
							TextureRenderer::framebuffer_manager_.framebuffer(name);
						if (peel_buffer)
							peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
						glActiveTexture(GL_TEXTURE15);
						name = "peel buffer" + std::to_string(1);
						peel_buffer =
							TextureRenderer::framebuffer_manager_.framebuffer(name);
						if (peel_buffer)
							peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
						glActiveTexture(GL_TEXTURE0);
					}
					else
					{
						glActiveTexture(GL_TEXTURE13);
						name = "peel buffer" + std::to_string(i-2);
						peel_buffer =
							TextureRenderer::framebuffer_manager_.framebuffer(name);
						if (peel_buffer)
							peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
						glActiveTexture(GL_TEXTURE14);
						name = "peel buffer" + std::to_string(i-1);
						peel_buffer =
							TextureRenderer::framebuffer_manager_.framebuffer(name);
						if (peel_buffer)
							peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
						glActiveTexture(GL_TEXTURE15);
						name = "peel buffer" + std::to_string(i);
						peel_buffer =
							TextureRenderer::framebuffer_manager_.framebuffer(name);
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
			DrawClippingPlanes(false, FRONT_FACE);

		if (m_draw_bounds)
			DrawBounds();

		if (m_draw_annotations)
			DrawAnnotations();

		if (m_draw_rulers)
			DrawRulers();

		//traces
		DrawTraces();

		m_mv_mat = mv_temp;
	}
}

//draw meshes
//peel==true -- depth peeling
void VRenderGLView::DrawMeshes(int peel)
{
	int nx = GetGLSize().x;
	int ny = GetGLSize().y;
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
	if (TextureRenderer::get_mem_swap())
	{
		finished_bricks = TextureRenderer::get_finished_bricks();
		TextureRenderer::reset_finished_bricks();
	}

	PrepFinalBuffer();

	int nx = GetGLSize().x;
	int ny = GetGLSize().y;

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
			(m_int_mode == 6 &&
				!m_editing_ruler_point) ||
			m_int_mode == 8 ||
			m_force_clear)))
	{
		m_updating = false;
		m_force_clear = false;
		m_load_update = false;

		VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
		if (vr_frame &&
			vr_frame->GetSettingDlg() &&
			vr_frame->GetSettingDlg()->GetUpdateOrder() == 1)
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
		if (TextureRenderer::get_mem_swap())
		{
			//set start time for the texture renderer
			TextureRenderer::set_st_time(GET_TICK_COUNT());

			TextureRenderer::set_interactive(m_interactive);
			//if in interactive mode, do interactive bricking also
			if (m_interactive)
			{
				//calculate quota
				int total_bricks = TextureRenderer::get_total_brick_num();
				int quota_bricks = 1;// total_bricks / 2;
				int fin_bricks = finished_bricks;
				int last_bricks = TextureRenderer::
					get_est_bricks(3);
				int adj_bricks = 0;
				unsigned long up_time = TextureRenderer::get_cor_up_time();
				unsigned long consumed_time = TextureRenderer::get_consumed_time();
				if (consumed_time == 0)
					quota_bricks = total_bricks;
				else if (consumed_time / up_time > total_bricks)
					quota_bricks = 1;
				else
				{
					adj_bricks = Max(1, int(double(last_bricks) *
						double(up_time) / double(consumed_time)));
					quota_bricks = TextureRenderer::
						get_est_bricks(0, adj_bricks);
				}
				quota_bricks = Min(total_bricks, quota_bricks);
				TextureRenderer::set_quota_bricks(quota_bricks);
				TextureRenderer::push_quota_brick(quota_bricks);
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
						quota_bricks_chan = Min(count_bricks, quota_bricks);
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
				Point p;
				if (vd &&
					(GetPointVolumeBox(p, nx / 2, ny / 2, vd, false) > 0.0 ||
						GetPointPlane(p, nx / 2, ny / 2, 0, false) > 0.0))
				{
					int resx, resy, resz;
					double sclx, scly, sclz;
					double spcx, spcy, spcz;
					vd->GetResolution(resx, resy, resz);
					vd->GetScalings(sclx, scly, sclz);
					vd->GetSpacings(spcx, spcy, spcz);
					p = Point(p.x() / (resx*sclx*spcx),
						p.y() / (resy*scly*spcy),
						p.z() / (resz*sclz*spcz));
					TextureRenderer::set_qutoa_center(p);
				}
				else
					TextureRenderer::set_interactive(false);
			}
		}

		//handle intermixing modes
		if (m_vol_method == VOL_METHOD_MULTI)
		{
			if (TextureRenderer::get_mem_swap() &&
				TextureRenderer::get_interactive() &&
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
						if (TextureRenderer::get_mem_swap() &&
							TextureRenderer::get_interactive() &&
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
							if (TextureRenderer::get_mem_swap() &&
								TextureRenderer::get_interactive() &&
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

	if (TextureRenderer::get_mem_swap())
	{
		TextureRenderer::set_consumed_time(GET_TICK_COUNT() - TextureRenderer::get_st_time());
		if (TextureRenderer::get_start_update_loop() &&
			TextureRenderer::get_done_update_loop())
			TextureRenderer::reset_update_loop();
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

	if (TextureRenderer::get_mem_swap())
	{
		//if (finished_bricks == 0)
		//{
		//	if (m_nodraw_count == 100)
		//	{
		//		TextureRenderer::set_done_update_loop();
		//		m_nodraw_count = 0;
		//	}
		//	else
		//		m_nodraw_count++;
		//}
	}
}

void VRenderGLView::DrawAnnotations()
{
	if (!m_text_renderer)
		return;

	int nx, ny;
	nx = GetGLSize().x;
	ny = GetGLSize().y;
	float sx, sy;
	sx = 2.0 / nx;
	sy = 2.0 / ny;
	float px, py;

	Transform mv;
	Transform p;
	mv.set(glm::value_ptr(m_mv_mat));
	p.set(glm::value_ptr(m_proj_mat));

	Color text_color = GetTextColor();

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
				Point pos;
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
						m_text_renderer->RenderText(
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

			VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
			if (vr_frame)
			{
				AdjustView* adjust_view = vr_frame->GetAdjustView();
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
	TextureRenderer::clear_tex_pool();
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
	ChangeBrushSetsIndex();
}

int VRenderGLView::GetPaintMode()
{
	return m_selector.GetMode();
}

void VRenderGLView::DrawCircle(double cx, double cy,
	double radius, Color &color, glm::mat4 &matrix)
{
	int secs = 60;
	double deg = 0.0;

	vector<float> vertex;
	vertex.reserve(secs * 3);

	for (size_t i = 0; i<secs; ++i)
	{
		deg = i * 2 * PI / secs;
		vertex.push_back(radius*sin(deg));
		vertex.push_back(radius*cos(deg));
		vertex.push_back(0.0f);
	}

	ShaderProgram* shader =
		m_img_shader_factory.shader(IMG_SHDR_DRAW_GEOMETRY);
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

	glBindBuffer(GL_ARRAY_BUFFER, m_misc_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertex.size(), &vertex[0], GL_DYNAMIC_DRAW);
	glBindVertexArray(m_misc_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_misc_vbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const GLvoid*)0);
	glDrawArrays(GL_LINE_LOOP, 0, secs);
	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	if (shader && shader->valid())
		shader->release();
}

//draw the brush shape
void VRenderGLView::DrawBrush()
{
	double pressure = m_use_press && m_pressure > 0.0 ?
		1.0 + (m_pressure - 0.5)*0.4 : 1.0;
	pressure += m_air_press - 0.5;
	pressure = max(pressure, 0.0);

	wxPoint pos1(old_mouse_X, old_mouse_Y);
	wxRect reg = GetClientRect();
	if (reg.Contains(pos1))
	{
		int nx, ny;
		nx = GetGLSize().x;
		ny = GetGLSize().y;
		float sx, sy;
		sx = 2.0 / nx;
		sy = 2.0 / ny;

		//draw the circles
		//set up the matrices
		glm::mat4 proj_mat;
		double cx, cy;
		if (m_brush_size_data)
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

		Color text_color = GetTextColor();

		if (mode == 1 ||
			mode == 2 ||
			mode == 8)
			DrawCircle(cx, cy, m_brush_radius1*pressure,
				text_color, proj_mat);

		if (mode == 1 ||
			mode == 2 ||
			mode == 3 ||
			mode == 4)
			DrawCircle(cx, cy, m_brush_radius2*pressure,
				text_color, proj_mat);

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
		m_text_renderer->RenderText(wstr, text_color, px*sx, py*sy, sx, sy);

		glEnable(GL_DEPTH_TEST);

	}
}

//paint strokes on the paint fbo
void VRenderGLView::PaintStroke()
{
	int nx, ny;
	nx = GetGLSize().x;
	ny = GetGLSize().y;

	double pressure = m_use_press && m_pressure > 0.0 ?
		1.0 + (m_pressure - 0.5)*0.4 : 1.0;
	pressure += m_air_press - 0.5;
	pressure = max(pressure, 0.0);

	//generate texture and buffer objects
	//painting fbo
	Framebuffer* paint_buffer =
		TextureRenderer::framebuffer_manager_.framebuffer(
		FB_Render_RGBA, nx, ny, "paint brush");
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
		ShaderProgram* paint_shader =
			m_img_shader_factory.shader(IMG_SHDR_PAINT);
		if (paint_shader)
		{
			if (!paint_shader->valid())
				paint_shader->create();
			paint_shader->bind();
		}

		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendEquation(GL_MAX);

		double px = double(old_mouse_X - prv_mouse_X);
		double py = double(old_mouse_Y - prv_mouse_Y);
		double dist = sqrt(px*px + py*py);
		double step = m_brush_radius1*pressure*m_brush_spacing;
		int repeat = int(dist / step + 0.5);
		double spx = (double)prv_mouse_X;
		double spy = (double)prv_mouse_Y;
		if (repeat > 0)
		{
			px /= repeat;
			py /= repeat;
		}

		//set the width and height
		if (m_brush_size_data)
			paint_shader->setLocalParam(1, m_ortho_right - m_ortho_left,
				m_ortho_top - m_ortho_bottom, 0.0f, 0.0f);
		else
			paint_shader->setLocalParam(1, nx, ny, 0.0f, 0.0f);

		double x, y;
		double cx, cy;
		double radius1 = m_brush_radius1;
		double radius2 = m_brush_radius2;
		for (int i = 0; i <= repeat; i++)
		{
			x = spx + i*px;
			y = spy + i*py;
			if (m_brush_size_data)
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
				radius1 = m_brush_radius2;
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
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBlendEquation(GL_FUNC_ADD);
	RefreshGL(3);
}

//show the stroke buffer
void VRenderGLView::DisplayStroke()
{
	//painting texture
	Framebuffer* paint_buffer =
		TextureRenderer::framebuffer_manager_.framebuffer("paint brush");
	if (!paint_buffer)
		return;

	//draw the final buffer to the windows buffer
	glActiveTexture(GL_TEXTURE0);
	paint_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	ShaderProgram* img_shader =
		m_img_shader_factory.shader(IMG_SHADER_TEXTURE_LOOKUP);
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

//set 2d weights
void VRenderGLView::Set2dWeights()
{
	Framebuffer* final_buffer =
		TextureRenderer::framebuffer_manager_.framebuffer(
		"final");
	Framebuffer* chann_buffer =
		TextureRenderer::framebuffer_manager_.framebuffer(
		"channel");
	if (final_buffer && chann_buffer)
		m_selector.Set2DWeight(
			final_buffer->tex_id(GL_COLOR_ATTACHMENT0),
			chann_buffer->tex_id(GL_COLOR_ATTACHMENT0));
}

//segment volumes in current view
void VRenderGLView::Segment()
{
	int nx = GetGLSize().x;
	int ny = GetGLSize().y;
	GLint vp[4] = { 0, 0, (GLint)nx, (GLint)ny };
	GLfloat clear_color[4] = { 0, 0, 0, 0 };

	glViewport(0, 0, vp[2], vp[3]);
	HandleCamera();

	//translate object
	m_mv_mat = glm::translate(m_mv_mat, glm::vec3(m_obj_transx, m_obj_transy, m_obj_transz));
	//rotate object
	m_mv_mat = glm::rotate(m_mv_mat, float(glm::radians(m_obj_rotx)), glm::vec3(1.0, 0.0, 0.0));
	m_mv_mat = glm::rotate(m_mv_mat, float(glm::radians(m_obj_roty + 180.0)), glm::vec3(0.0, 1.0, 0.0));
	m_mv_mat = glm::rotate(m_mv_mat, float(glm::radians(m_obj_rotz + 180.0)), glm::vec3(0.0, 0.0, 1.0));
	//center object
	m_mv_mat = glm::translate(m_mv_mat, glm::vec3(-m_obj_ctrx, -m_obj_ctry, -m_obj_ctrz));

	Framebuffer* paint_buffer =
		TextureRenderer::framebuffer_manager_.framebuffer("paint brush");
	if (paint_buffer)
		m_selector.Set2DMask(paint_buffer->tex_id(GL_COLOR_ATTACHMENT0));
	Set2dWeights();
	//orthographic
	m_selector.SetOrthographic(!m_persp);

	//modulate threshold with pressure
	double ini_thresh_save, gm_falloff_save,
		scl_falloff_save, scl_translate_save;
	if (m_use_press && m_press_peak > 0.0)
	{
		//initial threshold
		/*ini_thresh_save = m_selector.GetBrushIniThresh();
		double ini_thresh = ini_thresh_save - m_press_peak + 0.5;
		if (ini_thresh < 0.0) ini_thresh = 0.0;
		m_selector.SetBrushIniThresh(ini_thresh);*/
		//gradient magnitude falloff
		gm_falloff_save = m_selector.GetBrushGmFalloff();
		double gm_falloff = gm_falloff_save + m_press_peak * 0.5;
		//if (gm_falloff < 0.0) gm_falloff = 0.0;
		m_selector.SetBrushGmFalloff(gm_falloff);
		//scalar falloff
		/*scl_falloff_save = m_selector.GetBrushSclFalloff();
		double scl_falloff = scl_falloff_save + (m_press_peak - 0.5) * 0.1;
		if (scl_falloff < 0.0) scl_falloff = 0.0;
		m_selector.SetBrushSclFalloff(scl_falloff);*/
		//scalar translate
		scl_translate_save = m_selector.GetBrushSclTranslate();
		double scl_translate = scl_translate_save - m_press_peak + 0.5;
		if (scl_translate < 0.0) scl_translate = 0.0;
		m_selector.SetBrushSclTranslate(scl_translate);
	}

	if (m_selector.GetSelectGroup())
	{
		VolumeData* vd = m_selector.GetVolume();
		DataGroup* group = 0;
		if (vd)
		{
			vd->SetViewport(vp);
			vd->SetClearColor(clear_color);
			for (int i = 0; i<GetLayerNum(); i++)
			{
				TreeLayer* layer = GetLayer(i);
				if (layer && layer->IsA() == 5)
				{
					DataGroup* tmp_group = (DataGroup*)layer;
					for (int j = 0; j<tmp_group->GetVolumeNum(); j++)
					{
						VolumeData* tmp_vd = tmp_group->GetVolumeData(j);
						if (tmp_vd && tmp_vd == vd)
						{
							group = tmp_group;
							break;
						}
					}
				}
				if (group)
					break;
			}
			//select the group
			if (group && group->GetVolumeNum()>1)
			{
				for (int i = 0; i<group->GetVolumeNum(); i++)
				{
					VolumeData* tmp_vd = group->GetVolumeData(i);
					if (tmp_vd && tmp_vd->GetDisp())
					{
						tmp_vd->SetMatrices(m_mv_mat, m_proj_mat, m_tex_mat);
						tmp_vd->SetViewport(vp);
						tmp_vd->SetClearColor(clear_color);
						m_selector.SetVolume(tmp_vd);
						m_selector.Select(m_brush_radius2 - m_brush_radius1);
					}
				}
				m_selector.SetVolume(vd);
			}
			else
				m_selector.Select(m_brush_radius2 - m_brush_radius1);
		}
	}
	else if (m_selector.GetSelectBoth())
	{
		VolumeData* vd = m_calculator.GetVolumeA();
		if (vd)
		{
			vd->SetMatrices(m_mv_mat, m_proj_mat, m_tex_mat);
			vd->SetViewport(vp);
			vd->SetClearColor(clear_color);
			m_selector.SetVolume(vd);
			m_selector.Select(m_brush_radius2 - m_brush_radius1);
		}
		vd = m_calculator.GetVolumeB();
		if (vd)
		{
			vd->SetMatrices(m_mv_mat, m_proj_mat, m_tex_mat);
			vd->SetViewport(vp);
			vd->SetClearColor(clear_color);
			m_selector.SetVolume(vd);
			m_selector.Select(m_brush_radius2 - m_brush_radius1);
		}
	}
	else
		m_selector.Select(m_brush_radius2 - m_brush_radius1);

	//restore
	if (m_use_press && m_press_peak > 0.0)
	{
		//m_selector.SetBrushIniThresh(ini_thresh_save);
		m_selector.SetBrushGmFalloff(gm_falloff_save);
		//m_selector.SetBrushSclFalloff(scl_falloff_save);
		m_selector.SetBrushSclTranslate(scl_translate_save);
	}

	//update
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetTraceDlg())
	{
		if (vr_frame->GetTraceDlg()->GetAutoID())
		{
			if (m_selector.GetMode() == 1 ||
				m_selector.GetMode() == 2 ||
				m_selector.GetMode() == 4)
				vr_frame->GetTraceDlg()->CellNewID(true);
			else if (m_selector.GetMode() == 3)
				vr_frame->GetTraceDlg()->CellEraseID();
		}
		if (vr_frame->GetTraceDlg()->GetManualAssist())
		{
			if (!vr_frame->GetTraceDlg()->GetAutoID())
				vr_frame->GetTraceDlg()->CellUpdate();
			vr_frame->GetTraceDlg()->CellLink(true);
		}
	}
	if (vr_frame && vr_frame->GetBrushToolDlg())
	{
		vr_frame->GetBrushToolDlg()->GetSettings(m_vrv);
	}
}

//label volumes in current view
void VRenderGLView::Label()
{
	int nx = GetGLSize().x;
	int ny = GetGLSize().y;
	GLint vp[4] = { 0, 0, (GLint)nx, (GLint)ny };
	GLfloat clear_color[4] = { 0, 0, 0, 0 };

	VolumeData* vd = m_selector.GetVolume();
	if (vd)
	{
		vd->SetViewport(vp);
		vd->SetClearColor(clear_color);
		m_selector.Label(0);
	}
}

//remove noise
int VRenderGLView::CompAnalysis(double min_voxels, double max_voxels,
	double thresh, double falloff, bool select, bool gen_ann, bool size_map)
{
	int return_val = 0;

	if (!select)
	{
		Framebuffer* paint_buffer =
			TextureRenderer::framebuffer_manager_.framebuffer("paint brush");
		if (paint_buffer)
			m_selector.Set2DMask(paint_buffer->tex_id(GL_COLOR_ATTACHMENT0));
		Set2dWeights();
		m_selector.SetSizeMap(size_map);
		return_val = m_selector.CompAnalysis(min_voxels, max_voxels, thresh, falloff, select, gen_ann);
	}
	else
	{
		m_selector.SetSizeMap(size_map);
		return_val = m_selector.CompAnalysis(min_voxels, max_voxels, thresh, falloff, select, gen_ann);
	}

	Annotations* ann = m_selector.GetAnnotations();
	if (ann)
	{
		VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
		if (vr_frame)
		{
			DataManager* mgr = vr_frame->GetDataManager();
			if (mgr)
				mgr->AddAnnotations(ann);
			vr_frame->UpdateList();
		}
	}

	return return_val;
}

void VRenderGLView::CompExport(int mode, bool select)
{
	switch (mode)
	{
	case 0://multi channels
		m_selector.CompExportMultiChann(select);
		break;
	case 1://random colors
	{
		wxString sel_name = m_selector.GetVolume()->GetName();
		VolumeData* vd_r = 0;
		VolumeData* vd_g = 0;
		VolumeData* vd_b = 0;
		VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
		if (vr_frame)
		{
			for (int i = 0; i<vr_frame->GetDataManager()->GetVolumeNum(); i++)
			{
				VolumeData* vd = vr_frame->GetDataManager()->GetVolumeData(i);
				wxString name = vd->GetName();
				if (name == sel_name + "_COMP1")
					vd_r = vd;
				else if (name == sel_name + "_COMP2")
					vd_g = vd;
				else if (name == sel_name + "_COMP3")
					vd_b = vd;
			}
		}
		m_selector.CompExportRandomColor(1, vd_r, vd_g, vd_b, select);
	}
	break;
	}
	vector<VolumeData*> *vol_list = m_selector.GetResultVols();
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
		wxString group_name = "";
		DataGroup* group = 0;
		for (int i = 0; i<(int)vol_list->size(); i++)
		{
			VolumeData* vd = (*vol_list)[i];
			if (vd)
			{
				vr_frame->GetDataManager()->AddVolumeData(vd);
				//vr_frame->GetDataManager()->SetVolumeDefault(vd);
				if (i == 0)
				{
					group_name = AddGroup("");
					group = GetGroup(group_name);
				}
				AddVolumeData(vd, group_name);
			}
		}
		if (group)
		{
			group->SetSyncRAll(true);
			group->SetSyncGAll(true);
			group->SetSyncBAll(true);
			VolumeData* vd = m_selector.GetVolume();
			if (vd)
			{
				FLIVR::Color col = vd->GetGamma();
				group->SetGammaAll(col);
				col = vd->GetBrightness();
				group->SetBrightnessAll(col);
				col = vd->GetHdr();
				group->SetHdrAll(col);
			}
		}
		vr_frame->UpdateList();
		vr_frame->UpdateTree(m_selector.GetVolume()->GetName());
		RefreshGL(4);
	}
}

void VRenderGLView::ShowAnnotations()
{
	Annotations* ann = m_selector.GetAnnotations();
	if (ann)
	{
		AddAnnotations(ann);
		VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
		if (vr_frame)
			vr_frame->UpdateTree(vr_frame->GetCurSelVol()->GetName());
	}
}

int VRenderGLView::NoiseAnalysis(double min_voxels, double max_voxels, double thresh)
{
	int return_val = 0;

	Framebuffer* paint_buffer =
		TextureRenderer::framebuffer_manager_.framebuffer("paint brush");
	if (paint_buffer)
		m_selector.Set2DMask(paint_buffer->tex_id(GL_COLOR_ATTACHMENT0));
	Set2dWeights();
	return_val = m_selector.NoiseAnalysis(min_voxels, max_voxels, 10.0, thresh);

	return return_val;
}

void VRenderGLView::NoiseRemoval(int iter, double thresh)
{
	VolumeData* vd = m_selector.GetVolume();
	if (!vd) return;

	wxString name = vd->GetName();
	if (name.Find("_NR") == wxNOT_FOUND)
	{
		m_selector.NoiseRemoval(iter, thresh, 1);
		vector<VolumeData*> *vol_list = m_selector.GetResultVols();
		VolumeData* vd_new = (*vol_list)[0];
		VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
		if (vd_new && vr_frame)
		{
			vr_frame->GetDataManager()->AddVolumeData(vd_new);
			wxString group_name = AddGroup("");
			AddVolumeData(vd_new, group_name);
			vd->SetDisp(false);
			vr_frame->UpdateList();
			vr_frame->UpdateTree(vd_new->GetName());
		}
	}
	else
		m_selector.NoiseRemoval(iter, thresh);
}

//brush properties
//load settings
void VRenderGLView::LoadBrushSettings()
{
	wxString expath = wxStandardPaths::Get().GetExecutablePath();
	expath = wxPathOnly(expath);
	wxString dft = expath + "/default_brush_settings.dft";
	wxFileInputStream is(dft);
	if (!is.IsOk())
		return;
	wxFileConfig fconfig(is);

	double val;
	int ival;
	bool bval;

	//brush properties
	if (fconfig.Read("brush_ini_thresh", &val))
		m_selector.SetBrushIniThresh(val);
	if (fconfig.Read("brush_gm_falloff", &val))
		m_selector.SetBrushGmFalloff(val);
	if (fconfig.Read("brush_scl_falloff", &val))
		m_selector.SetBrushSclFalloff(val);
	if (fconfig.Read("brush_scl_translate", &val))
	{
		m_selector.SetBrushSclTranslate(val);
		m_calculator.SetThreshold(val);
	}
	//auto thresh
	if (fconfig.Read("auto_thresh", &bval))
		m_selector.SetEstimateThreshold(bval);
	//edge detect
	if (fconfig.Read("edge_detect", &bval))
		m_selector.SetEdgeDetect(bval);
	//hidden removal
	if (fconfig.Read("hidden_removal", &bval))
		m_selector.SetHiddenRemoval(bval);
	//select group
	if (fconfig.Read("select_group", &bval))
		m_selector.SetSelectGroup(bval);
	//2d influence
	if (fconfig.Read("brush_2dinfl", &val))
		m_selector.SetW2d(val);
	//size 1
	if (fconfig.Read("brush_size1", &val) && val>0.0)
		m_brush_radius1 = val;
	//size 2 link
	if (fconfig.Read("use_brush_size2", &bval))
	{
		m_use_brush_radius2 = bval;
		m_selector.SetUseBrushSize2(bval);
	}
	//size 2
	if (fconfig.Read("brush_size2", &val) && val>0.0)
		m_brush_radius2 = val;
	//radius settings for individual brush types
	if (fconfig.Exists("/radius_settings"))
	{
		fconfig.SetPath("/radius_settings");
		int brush_num = fconfig.Read("num", 0l);
		if (m_brush_radius_sets.size() != brush_num)
			m_brush_radius_sets.resize(brush_num);
		wxString str;
		for (int i = 0; i < brush_num; ++i)
		{
			str = wxString::Format("/radius_settings/%d", i);
			if (!fconfig.Exists(str))
				continue;
			fconfig.SetPath(str);
			//type
			fconfig.Read("type", &(m_brush_radius_sets[i].type));
			//radius 1
			fconfig.Read("radius1", &(m_brush_radius_sets[i].radius1));
			//radius 2
			fconfig.Read("radius2", &(m_brush_radius_sets[i].radius2));
			//use radius 2
			fconfig.Read("use_radius2", &(m_brush_radius_sets[i].use_radius2));
		}
		fconfig.SetPath("/");
	}
	if (m_brush_radius_sets.size() == 0)
	{
		BrushRadiusSet radius_set;
		//select brush
		radius_set.type = 2;
		radius_set.radius1 = 10;
		radius_set.radius2 = 30;
		radius_set.use_radius2 = true;
		m_brush_radius_sets.push_back(radius_set);
		//erase
		radius_set.type = 3;
		m_brush_radius_sets.push_back(radius_set);
		//diffuse brush
		radius_set.type = 4;
		m_brush_radius_sets.push_back(radius_set);
		//solid brush
		radius_set.type = 8;
		radius_set.use_radius2 = false;
		m_brush_radius_sets.push_back(radius_set);
	}
	m_brush_sets_index = 0;
	//iterations
	if (fconfig.Read("brush_iters", &ival))
	{
		switch (ival)
		{
		case 1:
			m_selector.SetBrushIteration(BRUSH_TOOL_ITER_WEAK);
			break;
		case 2:
			m_selector.SetBrushIteration(BRUSH_TOOL_ITER_NORMAL);
			break;
		case 3:
			m_selector.SetBrushIteration(BRUSH_TOOL_ITER_STRONG);
			break;
		}
	}
	//brush size relation
	if (fconfig.Read("brush_size_data", &bval))
		m_brush_size_data = bval;
}

void VRenderGLView::SaveBrushSettings()
{
	wxString app_name = "FluoRender " +
		wxString::Format("%d.%.1f", VERSION_MAJOR, float(VERSION_MINOR));
	wxString vendor_name = "FluoRender";
	wxString local_name = "default_brush_settings.dft";
	wxFileConfig fconfig(app_name, vendor_name, local_name, "",
		wxCONFIG_USE_LOCAL_FILE);
	wxString str;
	//brush properties
	fconfig.Write("brush_ini_thresh",
		m_selector.GetBrushIniThresh());
	fconfig.Write("brush_gm_falloff",
		m_selector.GetBrushGmFalloff());
	fconfig.Write("brush_scl_falloff",
		m_selector.GetBrushSclFalloff());
	fconfig.Write("brush_scl_translate",
		m_selector.GetBrushSclTranslate());
	//auto thresh
	fconfig.Write("auto_thresh",
		m_selector.GetEstimateThreshold());
	//edge detect
	fconfig.Write("edge_detect",
		m_selector.GetEdgeDetect());
	//hidden removal
	fconfig.Write("hidden_removal",
		m_selector.GetHiddenRemoval());
	//select group
	fconfig.Write("select_group",
		m_selector.GetSelectGroup());
	//2d influence
	fconfig.Write("brush_2dinfl",
		m_selector.GetW2d());
	//size 1
	fconfig.Write("brush_size1", m_brush_radius1);
	//size2 link
	fconfig.Write("use_brush_size2", m_use_brush_radius2);
	//size 2
	fconfig.Write("brush_size2", m_brush_radius2);
	//radius settings for individual brush types
	fconfig.SetPath("/radius_settings");
	int brush_num = m_brush_radius_sets.size();
	fconfig.Write("num", brush_num);
	for (int i = 0; i < brush_num; ++i)
	{
		BrushRadiusSet radius_set = m_brush_radius_sets[i];
		str = wxString::Format("/radius_settings/%d", i);
		fconfig.SetPath(str);
		//type
		fconfig.Write("type", radius_set.type);
		//radius 1
		fconfig.Write("radius1", radius_set.radius1);
		//radius 2
		fconfig.Write("radius2", radius_set.radius2);
		//use radius 2
		fconfig.Write("use_radius2", radius_set.use_radius2);
	}
	fconfig.SetPath("/");
	//iterations
	fconfig.Write("brush_iters",
		m_selector.GetBrushIteration());
	//brush size relation
	fconfig.Write("brush_size_data", m_brush_size_data);

	wxString expath = wxStandardPaths::Get().GetExecutablePath();
	expath = wxPathOnly(expath);
	wxString dft = expath + "/default_brush_settings.dft";
	wxFileOutputStream os(dft);
	fconfig.Save(os);
}

void VRenderGLView::SetBrushUsePres(bool pres)
{
	m_use_press = pres;
}

bool VRenderGLView::GetBrushUsePres()
{
	return m_use_press;
}

void VRenderGLView::SetUseBrushSize2(bool val)
{
	m_use_brush_radius2 = val;
	m_selector.SetUseBrushSize2(val);
	if (!val)
		m_brush_radius2 = m_brush_radius1;
}

bool VRenderGLView::GetUseBrushSize2()
{
	return m_use_brush_radius2;
}

void VRenderGLView::SetBrushSize(double size1, double size2)
{
	if (size1 > 0.0)
		m_brush_radius1 = size1;
	if (size2 > 0.0)
		m_brush_radius2 = size2;
	if (!m_use_brush_radius2)
		m_brush_radius2 = m_brush_radius1;
}

double VRenderGLView::GetBrushSize1()
{
	return m_brush_radius1;
}

double VRenderGLView::GetBrushSize2()
{
	return m_brush_radius2;
}

void VRenderGLView::SetBrushSpacing(double spacing)
{
	if (spacing > 0.0)
		m_brush_spacing = spacing;
}

double VRenderGLView::GetBrushSpacing()
{
	return m_brush_spacing;
}

void VRenderGLView::SetBrushIteration(int num)
{
	m_selector.SetBrushIteration(num);
}

int VRenderGLView::GetBrushIteration()
{
	return m_selector.GetBrushIteration();
}

void VRenderGLView::SetBrushSizeData(bool val)
{
	m_brush_size_data = val;
}

bool VRenderGLView::GetBrushSizeData()
{
	return m_brush_size_data;
}

//brush translate
void VRenderGLView::SetBrushSclTranslate(double val)
{
	m_selector.SetBrushSclTranslate(val);
	m_calculator.SetThreshold(val);
}

double VRenderGLView::GetBrushSclTranslate()
{
	return m_selector.GetBrushSclTranslate();
}

//gm falloff
void VRenderGLView::SetBrushGmFalloff(double val)
{
	m_selector.SetBrushGmFalloff(val);
}

double VRenderGLView::GetBrushGmFalloff()
{
	return m_selector.GetBrushGmFalloff();
}

//change brush display
void VRenderGLView::ChangeBrushSize(int value)
{
	if (!value) return;

	if (m_use_brush_radius2 && m_selector.GetMode() != 8)
	{
		double delta = value * m_brush_radius2 / 2000.0;
		m_brush_radius2 += delta;
		m_brush_radius2 = max(1.0, m_brush_radius2);
		m_brush_radius1 = min(m_brush_radius2, m_brush_radius1);
	}
	else
	{
		double delta = value * m_brush_radius1 / 1000.0;
		m_brush_radius1 += delta;
		m_brush_radius1 = max(m_brush_radius1, 1.0);
		m_brush_radius2 = m_brush_radius1;
	}

	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetBrushToolDlg())
		vr_frame->GetBrushToolDlg()->GetSettings(m_vrv);
}

void VRenderGLView::SetW2d(double val)
{
	m_selector.SetW2d(val);
}

double VRenderGLView::GetW2d()
{
	return m_selector.GetW2d();
}

//edge detect
void VRenderGLView::SetEdgeDetect(bool value)
{
	m_selector.SetEdgeDetect(value);
}

bool VRenderGLView::GetEdgeDetect()
{
	return m_selector.GetEdgeDetect();
}

//hidden removal
void VRenderGLView::SetHiddenRemoval(bool value)
{
	m_selector.SetHiddenRemoval(value);
}

bool VRenderGLView::GetHiddenRemoval()
{
	return m_selector.GetHiddenRemoval();
}

//select group
void VRenderGLView::SetSelectGroup(bool value)
{
	m_selector.SetSelectGroup(value);
}

bool VRenderGLView::GetSelectGroup()
{
	return m_selector.GetSelectGroup();
}

//estimate threshold
void VRenderGLView::SetEstimateThresh(bool value)
{
	m_selector.SetEstimateThreshold(value);
}

bool VRenderGLView::GetEstimateThresh()
{
	return m_selector.GetEstimateThreshold();
}

//select both
void VRenderGLView::SetSelectBoth(bool value)
{
	m_selector.SetSelectBoth(value);
}

bool VRenderGLView::GetSelectBoth()
{
	return m_selector.GetSelectBoth();
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

void VRenderGLView::CalculateSingle(int type, wxString prev_group, bool add)
{
	m_calculator.Calculate(type);
	VolumeData* vd = m_calculator.GetResult();
	if (vd)
	{
		if (type == 1 ||
			type == 2 ||
			type == 3 ||
			type == 4 ||
			type == 5 ||
			type == 6 ||
			type == 8 ||
			type == 9)
		{
			VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
			if (vr_frame)
			{
				//copy 2d adjust & color
				VolumeData* vd_a = m_calculator.GetVolumeA();
				if (vd_a)
				{
					//clipping planes
					vector<Plane*> *planes = vd_a->GetVR() ? vd_a->GetVR()->get_planes() : 0;
					if (planes && vd->GetVR())
						vd->GetVR()->set_planes(planes);
					//transfer function
					vd->Set3DGamma(vd_a->Get3DGamma());
					vd->SetBoundary(vd_a->GetBoundary());
					vd->SetOffset(vd_a->GetOffset());
					vd->SetLeftThresh(vd_a->GetLeftThresh());
					vd->SetRightThresh(vd_a->GetRightThresh());
					FLIVR::Color col = vd_a->GetColor();
					vd->SetColor(col);
					vd->SetAlpha(vd_a->GetAlpha());
					//shading
					vd->SetShading(vd_a->GetShading());
					double amb, diff, spec, shine;
					vd_a->GetMaterial(amb, diff, spec, shine);
					vd->SetMaterial(amb, diff, spec, shine);
					//shadow
					vd->SetShadow(vd_a->GetShadow());
					double shadow;
					vd_a->GetShadowParams(shadow);
					vd->SetShadowParams(shadow);
					//sample rate
					vd->SetSampleRate(vd_a->GetSampleRate());
					//2d adjusts
					col = vd_a->GetGamma();
					vd->SetGamma(col);
					col = vd_a->GetBrightness();
					vd->SetBrightness(col);
					col = vd_a->GetHdr();
					vd->SetHdr(col);
					vd->SetSyncR(vd_a->GetSyncR());
					vd->SetSyncG(vd_a->GetSyncG());
					vd->SetSyncB(vd_a->GetSyncB());
				}

				if (add)
				{
					vr_frame->GetDataManager()->AddVolumeData(vd);
					//vr_frame->GetDataManager()->SetVolumeDefault(vd);
					AddVolumeData(vd, prev_group);

					if (type == 5 ||
						type == 6 ||
						type == 9)
					{
						if (vd_a)
							vd_a->SetDisp(false);
					}
					else if (type == 1 ||
						type == 2 ||
						type == 3 ||
						type == 4)
					{
						if (vd_a)
							vd_a->SetDisp(false);
						VolumeData* vd_b = m_calculator.GetVolumeB();
						if (vd_b)
							vd_b->SetDisp(false);
					}
					vr_frame->UpdateList();
					vr_frame->UpdateTree(vd->GetName());
				}
			}
		}
		else if (type == 7)
		{
			VolumeData* vd_a = m_calculator.GetVolumeA();
			if (vd_a)
			{
				vd_a->Replace(vd);
				delete vd;
				VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
				if (vr_frame)
					vr_frame->GetPropView()->SetVolumeData(vd_a);
			}
		}
		RefreshGL(5);
	}
}

void VRenderGLView::Calculate(int type, wxString prev_group, bool add)
{
	if (type == 5 ||
		type == 6 ||
		type == 7)
	{
		vector<VolumeData*> vd_list;
		if (m_selector.GetSelectGroup())
		{
			VolumeData* vd = m_calculator.GetVolumeA();
			DataGroup* group = 0;
			if (vd)
			{
				for (int i = 0; i<GetLayerNum(); i++)
				{
					TreeLayer* layer = GetLayer(i);
					if (layer && layer->IsA() == 5)
					{
						DataGroup* tmp_group = (DataGroup*)layer;
						for (int j = 0; j<tmp_group->GetVolumeNum(); j++)
						{
							VolumeData* tmp_vd = tmp_group->GetVolumeData(j);
							if (tmp_vd && tmp_vd == vd)
							{
								group = tmp_group;
								break;
							}
						}
					}
					if (group)
						break;
				}
			}
			if (group && group->GetVolumeNum()>1)
			{
				for (int i = 0; i<group->GetVolumeNum(); i++)
				{
					VolumeData* tmp_vd = group->GetVolumeData(i);
					if (tmp_vd && tmp_vd->GetDisp())
						vd_list.push_back(tmp_vd);
				}
				for (size_t i = 0; i<vd_list.size(); ++i)
				{
					m_calculator.SetVolumeA(vd_list[i]);
					CalculateSingle(type, prev_group, add);
				}
				m_calculator.SetVolumeA(vd);
			}
			else
				CalculateSingle(type, prev_group, add);
		}
		else
			CalculateSingle(type, prev_group, add);
	}
	else
		CalculateSingle(type, prev_group, add);
}

//draw out the framebuffer after composition
void VRenderGLView::PrepFinalBuffer()
{
	int nx, ny;
	nx = GetGLSize().x;
	ny = GetGLSize().y;

	//generate textures & buffer objects
	glActiveTexture(GL_TEXTURE0);
	//glEnable(GL_TEXTURE_2D);
	//frame buffer for final
	Framebuffer* final_buffer =
		TextureRenderer::framebuffer_manager_.framebuffer(
		FB_Render_RGBA, nx, ny, "final");
	if (final_buffer)
		final_buffer->protect();
}

void VRenderGLView::ClearFinalBuffer()
{
	Framebuffer* final_buffer =
		TextureRenderer::framebuffer_manager_.framebuffer(
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
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//draw the final buffer to the windows buffer
	glActiveTexture(GL_TEXTURE0);
	Framebuffer* final_buffer =
		TextureRenderer::framebuffer_manager_.framebuffer("final");
	if (final_buffer)
		final_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	//glBlendFunc(GL_ONE, GL_ONE);
	glDisable(GL_DEPTH_TEST);

	//2d adjustment
	ShaderProgram* img_shader =
		m_img_shader_factory.shader(IMG_SHDR_BLEND_BRIGHT_BACKGROUND_HDR);
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
		int nx = GetGLSize().x;
		int ny = GetGLSize().y;
		glReadPixels(nx / 2, ny / 2, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
		m_pin_pick_thresh = 0.8 * double(pixel[3]) / 255.0;
	}
}

//Draw the volmues with compositing
//peel==true -- depth peeling
void VRenderGLView::DrawVolumesComp(vector<VolumeData*> &list, bool mask, int peel)
{
	if (list.size() <= 0)
		return;

	int i;

	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;

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
	nx = GetGLSize().x;
	ny = GetGLSize().y;

	//generate textures & buffer objects
	//frame buffer for each volume
	Framebuffer* chann_buffer =
		TextureRenderer::framebuffer_manager_.framebuffer(
		FB_Render_RGBA, nx, ny, "channel");
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
			//when run script
			VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
			if (vr_frame &&
				vr_frame->GetSettingDlg() &&
				vr_frame->GetSettingDlg()->GetRunScript() &&
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
				//when run script
				VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
				if (vr_frame &&
					vr_frame->GetSettingDlg() &&
					vr_frame->GetSettingDlg()->GetRunScript() &&
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
	int nx = GetGLSize().x;
	int ny = GetGLSize().y;
	GLint vp[4] = { 0, 0, (GLint)nx, (GLint)ny };
	GLfloat clear_color[4] = { 0, 0, 0, 0 };

	ShaderProgram* img_shader = 0;

	bool do_over = true;
	if (TextureRenderer::get_mem_swap() &&
		TextureRenderer::get_start_update_loop() &&
		!TextureRenderer::get_done_update_loop())
	{
		unsigned int rn_time = GET_TICK_COUNT();
		if (rn_time - TextureRenderer::get_st_time() >
			TextureRenderer::get_up_time())
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

	Framebuffer* chann_buffer =
		TextureRenderer::framebuffer_manager_.framebuffer("channel");
	Framebuffer* temp_buffer = 0;
	if (do_over)
	{
		//before rendering this channel, save final buffer to temp buffer
		if (TextureRenderer::get_mem_swap() &&
			TextureRenderer::get_start_update_loop() &&
			TextureRenderer::get_save_final_buffer())
		{
			TextureRenderer::reset_save_final_buffer();

			//bind temporary framebuffer for comp in stream mode
			temp_buffer =
				TextureRenderer::framebuffer_manager_.framebuffer(
				FB_Render_RGBA, nx, ny);
			if (temp_buffer)
			{
				temp_buffer->bind();
				temp_buffer->protect();
			}
			glClearColor(0.0, 0.0, 0.0, 0.0);
			glClear(GL_COLOR_BUFFER_BIT);
			glActiveTexture(GL_TEXTURE0);
			Framebuffer* final_buffer =
				TextureRenderer::framebuffer_manager_.framebuffer("final");
			if (final_buffer)
				final_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
			glDisable(GL_BLEND);
			glDisable(GL_DEPTH_TEST);

			img_shader =
				m_img_shader_factory.shader(IMG_SHADER_TEXTURE_LOOKUP);
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

		if (!TextureRenderer::get_mem_swap() ||
			(TextureRenderer::get_mem_swap() &&
				TextureRenderer::get_clear_chan_buffer()))
		{
			glClearColor(0.0, 0.0, 0.0, 0.0);
			glClear(GL_COLOR_BUFFER_BIT);
			TextureRenderer::reset_clear_chan_buffer();
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
		vd->Draw(!m_persp, m_adaptive, m_interactive, m_scale_factor);
	}

	if (vd->GetShadow())
	{
		vector<VolumeData*> list;
		list.push_back(vd);
		DrawOLShadows(list);
	}

	//bind fbo for final composition
	Framebuffer* final_buffer =
		TextureRenderer::framebuffer_manager_.framebuffer(
		"final");
	if (final_buffer)
		final_buffer->bind();

	if (TextureRenderer::get_mem_swap())
	{
		//restore temp buffer to final buffer
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
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
			m_img_shader_factory.shader(IMG_SHADER_TEXTURE_LOOKUP);
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
		m_img_shader_factory.shader(IMG_SHDR_BRIGHTNESS_CONTRAST_HDR);
	if (img_shader)
	{
		if (!img_shader->valid())
			img_shader->create();
		img_shader->bind();
	}
	Color gamma = vd->GetGamma();
	Color brightness = vd->GetBrightness();
	img_shader->setLocalParam(0, gamma.r(), gamma.g(), gamma.b(), 1.0);
	img_shader->setLocalParam(1, brightness.r(), brightness.g(), brightness.b(), 1.0);
	Color hdr = vd->GetHdr();
	img_shader->setLocalParam(2, hdr.r(), hdr.g(), hdr.b(), 0.0);
	//2d adjustment

	DrawViewQuad();

	if (img_shader && img_shader->valid())
		img_shader->release();

	//if vd is duplicated
	if (TextureRenderer::get_mem_swap() &&
		TextureRenderer::get_done_current_chan())
	{
		vector<TextureBrick*> *bricks =
			vd->GetTexture()->get_bricks();
		for (int i = 0; i < bricks->size(); i++)
			(*bricks)[i]->set_drawn(false);
	}
}

void VRenderGLView::DrawMIP(VolumeData* vd, int peel)
{
	int nx = GetGLSize().x;
	int ny = GetGLSize().y;
	GLint vp[4] = { 0, 0, (GLint)nx, (GLint)ny };
	GLfloat clear_color[4] = { 0, 0, 0, 0 };

	bool do_mip = true;
	if (TextureRenderer::get_mem_swap() &&
		TextureRenderer::get_start_update_loop() &&
		!TextureRenderer::get_done_update_loop())
	{
		unsigned int rn_time = GET_TICK_COUNT();
		if (rn_time - TextureRenderer::get_st_time() >
			TextureRenderer::get_up_time())
			return;
		if (vd->GetVR()->get_done_loop(1))
			do_mip = false;
	}

	bool shading = vd->GetVR()->get_shading();
	bool shadow = vd->GetShadow();
	int color_mode = vd->GetColormapMode();
	bool enable_alpha = vd->GetEnableAlpha();
	ShaderProgram* img_shader = 0;

	Framebuffer* chann_buffer =
		TextureRenderer::framebuffer_manager_.framebuffer("channel");
	Framebuffer* temp_buffer = 0;
	Framebuffer* overlay_buffer = 0;
	if (do_mip)
	{
		//before rendering this channel, save final buffer to temp buffer
		if (TextureRenderer::get_mem_swap() &&
			TextureRenderer::get_start_update_loop() &&
			TextureRenderer::get_save_final_buffer())
		{
			TextureRenderer::reset_save_final_buffer();

			//bind temporary framebuffer for comp in stream mode
			temp_buffer =
				TextureRenderer::framebuffer_manager_.framebuffer(
				FB_Render_RGBA, nx, ny);
			if (temp_buffer)
			{
				temp_buffer->bind();
				temp_buffer->protect();
			}
			glClearColor(0.0, 0.0, 0.0, 0.0);
			glClear(GL_COLOR_BUFFER_BIT);
			glActiveTexture(GL_TEXTURE0);
			Framebuffer* final_buffer =
				TextureRenderer::framebuffer_manager_.framebuffer("final");
			if (final_buffer)
				final_buffer->bind_texture(GL_COLOR_ATTACHMENT0);
			glDisable(GL_BLEND);
			glDisable(GL_DEPTH_TEST);

			img_shader =
				m_img_shader_factory.shader(IMG_SHADER_TEXTURE_LOOKUP);
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
			TextureRenderer::framebuffer_manager_.framebuffer(
			FB_Render_RGBA, nx, ny);
		if (overlay_buffer)
		{
			overlay_buffer->bind();
			overlay_buffer->protect();
			m_cur_framebuffer = overlay_buffer->id();
		}

		if (!TextureRenderer::get_mem_swap() ||
			(TextureRenderer::get_mem_swap() &&
				TextureRenderer::get_clear_chan_buffer()))
		{
			glClearColor(0.0, 0.0, 0.0, 0.0);
			glClear(GL_COLOR_BUFFER_BIT);
			TextureRenderer::reset_clear_chan_buffer();
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
		vd->Draw(!m_persp, m_adaptive, m_interactive, m_scale_factor);
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
				img_shader = m_img_shader_factory.shader(
					IMG_SHDR_GRADIENT_PROJ_MAP, vd->GetColormap());
			else
				img_shader = m_img_shader_factory.shader(
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
			//2d adjustment
		}
		else
		{
			img_shader =
				m_img_shader_factory.shader(IMG_SHADER_TEXTURE_LOOKUP);
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
	Framebuffer* final_buffer =
		TextureRenderer::framebuffer_manager_.framebuffer(
		"final");
	if (final_buffer)
		final_buffer->bind();

	if (TextureRenderer::get_mem_swap())
	{
		//restore temp buffer to final buffer
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
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
			m_img_shader_factory.shader(IMG_SHADER_TEXTURE_LOOKUP);
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
		m_img_shader_factory.shader(IMG_SHDR_BRIGHTNESS_CONTRAST_HDR);
	if (img_shader)
	{
		if (!img_shader->valid())
			img_shader->create();
		img_shader->bind();
	}
	Color gamma = vd->GetGamma();
	Color brightness = vd->GetBrightness();
	img_shader->setLocalParam(0, gamma.r(), gamma.g(), gamma.b(), 1.0);
	img_shader->setLocalParam(1, brightness.r(), brightness.g(), brightness.b(), 1.0);
	Color hdr = vd->GetHdr();
	img_shader->setLocalParam(2, hdr.r(), hdr.g(), hdr.b(), 0.0);
	//2d adjustment

	DrawViewQuad();

	if (img_shader && img_shader->valid())
		img_shader->release();

	vd->GetVR()->set_shading(shading);
	vd->SetColormapMode(color_mode);

	//if vd is duplicated
	if (TextureRenderer::get_mem_swap() &&
		TextureRenderer::get_done_current_chan())
	{
		vector<TextureBrick*> *bricks =
			vd->GetTexture()->get_bricks();
		for (int i = 0; i < bricks->size(); i++)
			(*bricks)[i]->set_drawn(false);
	}
}

void VRenderGLView::DrawOLShading(VolumeData* vd)
{
	int nx = GetGLSize().x;
	int ny = GetGLSize().y;

	if (TextureRenderer::get_mem_swap() &&
		TextureRenderer::get_start_update_loop() &&
		!TextureRenderer::get_done_update_loop())
	{
		unsigned int rn_time = GET_TICK_COUNT();
		if (rn_time - TextureRenderer::get_st_time() >
			TextureRenderer::get_up_time())
			return;
		if (vd->GetVR()->get_done_loop(2))
			return;
	}

	//shading pass
	Framebuffer* overlay_buffer =
		TextureRenderer::framebuffer_manager_.framebuffer(
		FB_Render_RGBA, nx, ny);
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
	vd->Draw(!m_persp, m_adaptive, m_interactive, m_scale_factor);
	vd->RestoreMode();
	vd->SetColormapMode(colormode);
	vd->SetEnableAlpha(alpha);

	//bind fbo for final composition
	Framebuffer* chann_buffer =
		TextureRenderer::framebuffer_manager_.framebuffer("channel");
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

	ShaderProgram* img_shader =
		m_img_shader_factory.shader(IMG_SHADER_TEXTURE_LOOKUP);
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
	nx = GetGLSize().x;
	ny = GetGLSize().y;

	//shadow pass
	//bind the fbo
	Framebuffer* overlay_buffer =
		TextureRenderer::framebuffer_manager_.framebuffer(
		FB_Render_RGBA, nx, ny);
	if (overlay_buffer)
	{
		overlay_buffer->bind();
		overlay_buffer->protect();
	}
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	string name = "peel buffer" + std::to_string(0);
	Framebuffer* peel_buffer =
		TextureRenderer::framebuffer_manager_.framebuffer(name);
	if (peel_buffer)
		peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	//2d adjustment
	ShaderProgram* img_shader =
		m_img_shader_factory.shader(IMG_SHDR_DEPTH_TO_GRADIENT);
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
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetSettingDlg())
		vr_frame->GetSettingDlg()->GetShadowDir(dir_x, dir_y);
	img_shader->setLocalParam(1, dir_x, dir_y, 0.0, 0.0);
	//2d adjustment

	DrawViewQuad();

	if (img_shader && img_shader->valid())
		img_shader->release();

	//
	//bind fbo for final composition
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
		m_img_shader_factory.shader(IMG_SHDR_GRADIENT_TO_SHADOW_MESH);
	if (img_shader)
	{
		if (!img_shader->valid())
			img_shader->create();
		img_shader->bind();
	}
	img_shader->setLocalParam(0, 1.0 / nx, 1.0 / ny, max(m_scale_factor, 1.0), 0.0);
	img_shader->setLocalParam(1, darkness, 0.0, 0.0, 0.0);
	glActiveTexture(GL_TEXTURE1);
	if (peel_buffer)
		peel_buffer->bind_texture(GL_DEPTH_ATTACHMENT);
	glActiveTexture(GL_TEXTURE2);
	Framebuffer* final_buffer =
		TextureRenderer::framebuffer_manager_.framebuffer("final");
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

	int nx = GetGLSize().x;
	int ny = GetGLSize().y;
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

	if (TextureRenderer::get_mem_swap() &&
		TextureRenderer::get_start_update_loop() &&
		!TextureRenderer::get_done_update_loop())
	{
		unsigned int rn_time = GET_TICK_COUNT();
		if (rn_time - TextureRenderer::get_st_time() >
			TextureRenderer::get_up_time())
			return;
		if (list.size() == 1 && list[0]->GetShadow())
			if (list[0]->GetVR()->get_done_loop(3))
				return;
	}

	Framebuffer* overlay_buffer =
		TextureRenderer::framebuffer_manager_.framebuffer(
		FB_Render_RGBA, nx, ny);
	if (overlay_buffer)
	{
		overlay_buffer->bind();
		overlay_buffer->protect();
		m_cur_framebuffer = overlay_buffer->id();
	}

	if (!TextureRenderer::get_mem_swap() ||
		(TextureRenderer::get_mem_swap() &&
			TextureRenderer::get_clear_chan_buffer()))
	{
		glClearColor(1.0, 1.0, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		TextureRenderer::reset_clear_chan_buffer();
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
		vd->Draw(!m_persp, m_adaptive, m_interactive, m_scale_factor);
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
			VolumeRenderer* vr = list[i]->GetVR();
			if (vr)
			{
				list[i]->SetMatrices(m_mv_mat, m_proj_mat, m_tex_mat);
				list[i]->SetFog(m_use_fog, m_fog_intensity, m_fog_start, m_fog_end);
				m_mvr->add_vr(vr);
				m_mvr->set_sampling_rate(vr->get_sampling_rate());
				m_mvr->SetNoiseRed(vr->GetNoiseRed());
			}
		}
		m_mvr->set_colormap_mode(2);
		//draw
		m_mvr->set_viewport(vp);
		m_mvr->set_clear_color(clear_color);
		m_mvr->set_cur_framebuffer(m_cur_framebuffer);
		m_mvr->draw(m_test_wiref, m_adaptive, m_interactive, !m_persp, m_scale_factor, m_intp);
		//restore
		m_mvr->set_colormap_mode(0);
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
	if (!TextureRenderer::get_mem_swap() ||
		(TextureRenderer::get_mem_swap() &&
			TextureRenderer::get_clear_chan_buffer()))
	{
		//shadow pass
		//bind the fbo
		Framebuffer* temp_buffer =
			TextureRenderer::framebuffer_manager_.framebuffer(
			FB_Render_RGBA, nx, ny);
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
		ShaderProgram* img_shader =
			m_img_shader_factory.shader(IMG_SHDR_DEPTH_TO_GRADIENT);
		if (img_shader)
		{
			if (!img_shader->valid())
				img_shader->create();
			img_shader->bind();
		}
		img_shader->setLocalParam(0, 1.0 / nx, 1.0 / ny, m_persp ? 2e10 : 1e6, 0.0);
		double dir_x = 0.0, dir_y = 0.0;
		VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
		if (vr_frame && vr_frame->GetSettingDlg())
			vr_frame->GetSettingDlg()->GetShadowDir(dir_x, dir_y);
		img_shader->setLocalParam(1, dir_x, dir_y, 0.0, 0.0);
		//2d adjustment

		DrawViewQuad();

		if (img_shader && img_shader->valid())
			img_shader->release();

		//bind fbo for final composition
		Framebuffer* chann_buffer =
			TextureRenderer::framebuffer_manager_.framebuffer("channel");
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
			m_img_shader_factory.shader(IMG_SHDR_GRADIENT_TO_SHADOW);
		if (img_shader)
		{
			if (!img_shader->valid())
				img_shader->create();
			img_shader->bind();
		}
		img_shader->setLocalParam(0, 1.0 / nx, 1.0 / ny, max(m_scale_factor, 1.0), 0.0);
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
		m_mvr = new MultiVolumeRenderer();
	if (!m_mvr)
		return;

	int nx = GetGLSize().x;
	int ny = GetGLSize().y;
	GLint vp[4] = { 0, 0, (GLint)nx, (GLint)ny };
	GLfloat clear_color[4] = { 0, 0, 0, 0 };

	m_mvr->set_blend_slices(m_blend_slices);

	int i;
	m_mvr->clear_vr();
	for (i = 0; i<(int)list.size(); i++)
	{
		VolumeData* vd = list[i];
		if (vd && vd->GetDisp())
		{
			VolumeRenderer* vr = vd->GetVR();
			if (vr)
			{
				VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
				if (vr_frame &&
					vr_frame->GetSettingDlg() &&
					vr_frame->GetSettingDlg()->GetRunScript() &&
					vd->GetMask(false) &&
					vd->GetLabel(false))
					vd->SetMaskMode(4);
				vd->SetMatrices(m_mv_mat, m_proj_mat, m_tex_mat);
				vd->SetFog(m_use_fog, m_fog_intensity, m_fog_start, m_fog_end);
				m_mvr->add_vr(vr);
				m_mvr->set_sampling_rate(vr->get_sampling_rate());
				m_mvr->SetNoiseRed(vr->GetNoiseRed());
			}
		}
	}

	if (m_mvr->get_vr_num() <= 0)
		return;
	m_mvr->set_depth_peel(peel);
	m_mvr->set_colormap_mode_single();
	// Set up transform
	Transform *tform = m_vd_pop_list[0]->GetTexture()->transform();
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
	Framebuffer* chann_buffer =
		TextureRenderer::framebuffer_manager_.framebuffer(
		FB_Render_RGBA, nx, ny, "channel");
	//bind the fbo
	if (chann_buffer)
	{
		chann_buffer->protect();
		chann_buffer->bind();
		m_cur_framebuffer = chann_buffer->id();
	}
	if (!TextureRenderer::get_mem_swap() ||
		(TextureRenderer::get_mem_swap() &&
			TextureRenderer::get_clear_chan_buffer()))
	{
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT);
		TextureRenderer::reset_clear_chan_buffer();
	}

	//draw multiple volumes at the same time
	m_mvr->set_viewport(vp);
	m_mvr->set_clear_color(clear_color);
	m_mvr->set_cur_framebuffer(m_cur_framebuffer);
	m_mvr->draw(m_test_wiref, m_adaptive, m_interactive, !m_persp, m_scale_factor, m_intp);

	//draw shadows
	DrawOLShadows(list);

	//bind fbo for final composition
	Framebuffer* final_buffer =
		TextureRenderer::framebuffer_manager_.framebuffer(
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
	ShaderProgram* img_shader =
		m_img_shader_factory.shader(IMG_SHDR_BRIGHTNESS_CONTRAST_HDR);
	if (img_shader)
	{
		if (!img_shader->valid())
		{
			img_shader->create();
		}
		img_shader->bind();
	}
	Color gamma, brightness, hdr;
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
	bool autoid = false;
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetTraceDlg())
		autoid = vr_frame->GetTraceDlg()->GetAutoID();

	if (m_int_mode == 5 ||
		m_int_mode == 7)
	{
		m_int_mode = 7;
		if (m_ruler_type == 3)
			m_selector.SetMode(8);
		else
			m_selector.SetMode(1);
	}
	else if (m_int_mode == 8)
	{
		if (m_ruler_type == 3)
			m_selector.SetMode(8);
		else
			m_selector.SetMode(1);
	}
	else
	{
		m_int_mode = 2;
		if (autoid && mode == 2)
			m_selector.SetMode(1);
		else
			m_selector.SetMode(mode);
	}
	m_paint_display = true;
	m_draw_brush = true;
	ChangeBrushSetsIndex();
}

void VRenderGLView::UpdateBrushState()
{
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	TreePanel* tree_panel = 0;
	BrushToolDlg* brush_dlg = 0;
	if (vr_frame)
	{
		tree_panel = vr_frame->GetTree();
		brush_dlg = vr_frame->GetBrushToolDlg();
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

//brush sets
void VRenderGLView::ChangeBrushSetsIndex()
{
	int mode = m_selector.GetMode();
	if (mode == 1)
		mode = 2;
	for (int i = 0; i < m_brush_radius_sets.size(); ++i)
	{
		BrushRadiusSet radius_set = m_brush_radius_sets[i];
		if (radius_set.type == mode &&
			m_brush_sets_index != i)
		{
			//save previous
			m_brush_radius_sets[m_brush_sets_index].radius1 = m_brush_radius1;
			m_brush_radius_sets[m_brush_sets_index].radius2 = m_brush_radius2;
			m_brush_radius_sets[m_brush_sets_index].use_radius2 = m_use_brush_radius2;
			//get new
			m_brush_radius1 = radius_set.radius1;
			m_brush_radius2 = radius_set.radius2;
			m_use_brush_radius2 = radius_set.use_radius2;
			m_brush_sets_index = i;
			break;
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
	//translate object
	m_mv_mat = glm::translate(m_mv_mat, glm::vec3(m_obj_transx, m_obj_transy, m_obj_transz));
	//rotate object
	m_mv_mat = glm::rotate(m_mv_mat, float(glm::radians(m_obj_rotx)), glm::vec3(1.0, 0.0, 0.0));
	m_mv_mat = glm::rotate(m_mv_mat, float(glm::radians(m_obj_roty + 180.0)), glm::vec3(0.0, 1.0, 0.0));
	m_mv_mat = glm::rotate(m_mv_mat, float(glm::radians(m_obj_rotz + 180.0)), glm::vec3(0.0, 0.0, 1.0));
	//center object
	m_mv_mat = glm::translate(m_mv_mat, glm::vec3(-m_obj_ctrx, -m_obj_ctry, -m_obj_ctrz));

	//set up fbo
	m_cur_framebuffer = 0;
	//bind
	Framebuffer* pick_buffer =
		TextureRenderer::framebuffer_manager_.framebuffer(
			FB_Pick_Int32_Float, nx, ny);
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
			VRenderFrame* frame = (VRenderFrame*)m_frame;
			if (frame && frame->GetTree())
			{
				frame->GetTree()->SetFocus();
				frame->GetTree()->Select(m_vrv->GetName(), md->GetName());
			}
			RefreshGL(27);
		}
	}
	else
	{
		VRenderFrame* frame = (VRenderFrame*)m_frame;
		if (frame && frame->GetCurSelType() == 3 && frame->GetTree())
			frame->GetTree()->Select(m_vrv->GetName(), "");
	}
	m_mv_mat = mv_temp;
}

void VRenderGLView::PickVolume()
{
	double dist = 0.0;
	double min_dist = -1.0;
	Point p;
	VolumeData* vd = 0;
	VolumeData* picked_vd = 0;
	for (int i = 0; i<(int)m_vd_pop_list.size(); i++)
	{
		vd = m_vd_pop_list[i];
		if (!vd) continue;
		int mode = 2;
		if (vd->GetMode() == 1) mode = 1;
		dist = GetPointVolume(p, old_mouse_X, old_mouse_Y, vd, mode, true, 0.5);
		if (dist > 0.0)
		{
			if (min_dist < 0.0)
			{
				min_dist = dist;
				picked_vd = vd;
			}
			else
			{
				if (m_persp)
				{
					if (dist < min_dist)
					{
						min_dist = dist;
						picked_vd = vd;
					}
				}
				else
				{
					if (dist > min_dist)
					{
						min_dist = dist;
						picked_vd = vd;
					}
				}
			}
		}
	}

	if (picked_vd)
	{
		VRenderFrame* frame = (VRenderFrame*)m_frame;
		if (frame && frame->GetTree())
		{
			frame->GetTree()->SetFocus();
			frame->GetTree()->Select(m_vrv->GetName(), picked_vd->GetName());
		}
	}
}

void VRenderGLView::OnIdle(wxIdleEvent& event)
{
	bool refresh = false;
	bool ref_stat = false;
	bool start_loop = true;
	m_retain_finalbuffer = false;

	VRenderFrame* frame = (VRenderFrame*)m_frame;

	//check memory swap status
	if (TextureRenderer::get_mem_swap() &&
		TextureRenderer::get_start_update_loop() &&
		!TextureRenderer::get_done_update_loop())
	{
		if (TextureRenderer::active_view_ == m_vrv->m_id)
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
		if (TextureRenderer::get_mem_swap() &&
			TextureRenderer::get_done_update_loop())
			m_pre_draw = true;
	}

	if (frame && frame->GetBenchmark())
	{
		double fps = 1.0 / m_timer->average();
		wxString title = wxString(FLUORENDER_TITLE) +
			" " + wxString(VERSION_MAJOR_TAG) +
			"." + wxString(VERSION_MINOR_TAG) +
			" Benchmarking... FPS = " +
			wxString::Format("%.2f", fps);
		frame->SetTitle(title);

		refresh = true;
		if (TextureRenderer::get_mem_swap() &&
			TextureRenderer::get_done_update_loop())
			m_pre_draw = true;
	}

	//pin rotation center
	if (m_pin_rot_center && m_rot_center_dirty &&
		m_cur_vol && !m_free)
	{
		Point p;
		int nx = GetGLSize().x;
		int ny = GetGLSize().y;
		int mode = 2;
		if (m_cur_vol->GetMode() == 1) mode = 1;
		double dist = GetPointVolume(p,
			nx / 2.0, ny / 2.0,
			m_cur_vol, mode, true, m_pin_pick_thresh);
		if (dist <= 0.0)
			dist = GetPointVolumeBox(p,
				nx / 2.0, ny / 2.0,
				m_cur_vol, true);
		if (dist > 0.0)
		{
			m_pin_ctr = p;
			double obj_transx, obj_transy, obj_transz;
			p = Point(m_obj_ctrx - p.x(),
				p.y() - m_obj_ctry,
				p.z() - m_obj_ctrz);
			obj_transx = p.x();
			obj_transy = p.y();
			obj_transz = p.z();
			double thresh = 10.0;
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
	if (window && view_reg.Contains(mouse_pos))
	{
		UpdateBrushState();

		//draw_mask
		if (wxGetKeyState(wxKeyCode('V')) &&
			m_draw_mask)
		{
			m_draw_mask = false;
			refresh = true;
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

			m_head = Vector(-m_transx, -m_transy, -m_transz);
			m_head.normalize();
			Vector side = Cross(m_up, m_head);
			Vector trans = -(side*(int(0.8*(m_ortho_right - m_ortho_left))));
			m_obj_transx += trans.x();
			m_obj_transy += trans.y();
			m_obj_transz += trans.z();
			//if (m_persp) SetSortBricks();
			refresh = true;
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

			m_head = Vector(-m_transx, -m_transy, -m_transz);
			m_head.normalize();
			Vector side = Cross(m_up, m_head);
			Vector trans = side*(int(0.8*(m_ortho_right - m_ortho_left)));
			m_obj_transx += trans.x();
			m_obj_transy += trans.y();
			m_obj_transz += trans.z();
			//if (m_persp) SetSortBricks();
			refresh = true;
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

			m_head = Vector(-m_transx, -m_transy, -m_transz);
			m_head.normalize();
			Vector trans = -m_up*(int(0.8*(m_ortho_top - m_ortho_bottom)));
			m_obj_transx += trans.x();
			m_obj_transy += trans.y();
			m_obj_transz += trans.z();
			//if (m_persp) SetSortBricks();
			refresh = true;
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

			m_head = Vector(-m_transx, -m_transy, -m_transz);
			m_head.normalize();
			Vector trans = m_up*(int(0.8*(m_ortho_top - m_ortho_bottom)));
			m_obj_transx += trans.x();
			m_obj_transy += trans.y();
			m_obj_transz += trans.z();
			//if (m_persp) SetSortBricks();
			refresh = true;
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
			if (frame && frame->GetMovieView())
				frame->GetMovieView()->UpFrame();
			refresh = true;
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
			if (frame && frame->GetMovieView())
				frame->GetMovieView()->DownFrame();
			refresh = true;
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
			if (frame && frame->GetClippingView())
				frame->GetClippingView()->MoveLinkedClippingPlanes(1);
			refresh = true;
		}
		if (m_clip_up &&
			!wxGetKeyState(wxKeyCode('s')))
			m_clip_up = false;
		//down
		if (!m_clip_down &&
			wxGetKeyState(wxKeyCode('w')))
		{
			m_clip_down = true;
			if (frame && frame->GetClippingView())
				frame->GetClippingView()->MoveLinkedClippingPlanes(0);
			refresh = true;
		}
		if (m_clip_down &&
			!wxGetKeyState(wxKeyCode('w')))
			m_clip_down = false;

		//cell full
		if (!m_cell_full &&
			wxGetKeyState(wxKeyCode('f')))
		{
			m_cell_full = true;
			if (frame && frame->GetComponentDlg())
				frame->GetComponentDlg()->SelectFullComp();
			if (frame && frame->GetTraceDlg())
				frame->GetTraceDlg()->CellUpdate();
			refresh = true;
		}
		if (m_cell_full &&
			!wxGetKeyState(wxKeyCode('f')))
			m_cell_full = false;
		//cell link
		if (!m_cell_link &&
			wxGetKeyState(wxKeyCode('l')))
		{
			m_cell_link = true;
			if (frame && frame->GetTraceDlg())
				frame->GetTraceDlg()->CellLink(false);
			refresh = true;
		}
		if (m_cell_link &&
			!wxGetKeyState(wxKeyCode('l')))
			m_cell_link = false;
		//new cell id
		if (!m_cell_new_id &&
			wxGetKeyState(wxKeyCode('n')))
		{
			m_cell_new_id = true;
			if (frame && frame->GetTraceDlg())
				frame->GetTraceDlg()->CellNewID(false);
			refresh = true;
		}
		if (m_cell_new_id &&
			!wxGetKeyState(wxKeyCode('n')))
			m_cell_new_id = false;
		//clear
		if (wxGetKeyState(wxKeyCode('c')) &&
			!m_clear_mask)
		{
			if (frame && frame->GetTraceDlg())
				frame->GetTraceDlg()->CompClear();
			m_clear_mask = true;
			refresh = true;
		}
		if (!wxGetKeyState(wxKeyCode('c')) &&
			m_clear_mask)
			m_clear_mask = false;
		//save all masks
		if (wxGetKeyState(wxKeyCode('m')) &&
			!m_save_mask)
		{
			if (frame && frame->GetList())
				frame->GetList()->SaveAllMasks();
			m_save_mask = true;
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
		}
		if (wxGetKeyState(wxKeyCode(']')))
		{
			ChangeBrushSize(10);
		}

		//forced refresh
		if (wxGetKeyState(WXK_F5))
		{
			m_clear_buffer = true;
			m_updating = true;
			if (frame && frame->GetStatusBar())
				frame->GetStatusBar()->PushStatusText("Forced Refresh");
			wxSizeEvent e;
			OnResize(e);
			RefreshGL(14);
			if (frame && frame->GetStatusBar())
				frame->GetStatusBar()->PopStatusText();
			return;
		}
	}

#ifdef _WIN32
	//update ortho rotation
	if (!m_vrv->m_ortho_view_cmb->HasFocus())
	{
		if (m_q.AlmostEqual(Quaternion(0, sqrt(2.0) / 2.0, 0, sqrt(2.0) / 2.0)))
			m_vrv->m_ortho_view_cmb->Select(0);
		else if (m_q.AlmostEqual(Quaternion(0, -sqrt(2.0) / 2.0, 0, sqrt(2.0) / 2.0)))
			m_vrv->m_ortho_view_cmb->Select(1);
		else if (m_q.AlmostEqual(Quaternion(sqrt(2.0) / 2.0, 0, 0, sqrt(2.0) / 2.0)))
			m_vrv->m_ortho_view_cmb->Select(2);
		else if (m_q.AlmostEqual(Quaternion(-sqrt(2.0) / 2.0, 0, 0, sqrt(2.0) / 2.0)))
			m_vrv->m_ortho_view_cmb->Select(3);
		else if (m_q.AlmostEqual(Quaternion(0, 0, 0, 1)))
			m_vrv->m_ortho_view_cmb->Select(4);
		else if (m_q.AlmostEqual(Quaternion(0, -1, 0, 0)))
			m_vrv->m_ortho_view_cmb->Select(5);
		else
			m_vrv->m_ortho_view_cmb->Select(6);
	}
#endif

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
	VRenderFrame* frame = (VRenderFrame*)m_frame;
	if (!frame || !m_vrv)
		return;

	m_full_screen = false;
	if (m_benchmark)
	{
		if (m_vrv->m_full_frame)
			m_vrv->m_full_frame->Hide();
		if (frame)
			frame->Close();
	}
	else if (GetParent() == m_vrv->m_full_frame)
	{
		Reparent(m_vrv);
		m_vrv->m_view_sizer->Add(this, 1, wxEXPAND);
		m_vrv->Layout();
		m_vrv->m_full_frame->Hide();
		if (frame)
		{
#ifdef _WIN32
			if (frame->GetSettingDlg() &&
				!frame->GetSettingDlg()->GetShowCursor())
				ShowCursor(true);
#endif
			frame->Iconize(false);
			frame->SetFocus();
			frame->Raise();
			frame->Show();
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
	double x, y, z;
	GetRotations(x, y, z);

	//remove the chance of the x/y/z angles being outside 360.
	while (x > 360.)  x -= 360.;
	while (x < -360.) x += 360.;
	while (y > 360.)  y -= 360.;
	while (y < -360.) y += 360.;
	while (z > 360.)  z -= 360.;
	while (z < -360.) z += 360.;
	if (360. - std::abs(x) < 0.001) x = 0.;
	if (360. - std::abs(y) < 0.001) y = 0.;
	if (360. - std::abs(z) < 0.001) z = 0.;
	m_step = step;
	m_total_frames = frames;
	m_rot_axis = rot_axis;
	m_cap_file = cap_file;
	m_rewind = rewind;
	//m_fr_length = len;

	m_movie_seq = 0;
	switch (m_rot_axis)
	{
	case 1: //X
		if (start_angle == 0.) {
			m_init_angle = x;
			m_end_angle = x + end_angle;
		}
		m_cur_angle = x;
		m_start_angle = x;
		break;
	case 2: //Y
		if (start_angle == 0.) {
			m_init_angle = y;
			m_end_angle = y + end_angle;
		}
		m_cur_angle = y;
		m_start_angle = y;
		break;
	case 3: //Z
		if (start_angle == 0.) {
			m_init_angle = z;
			m_end_angle = z + end_angle;
		}
		m_cur_angle = z;
		m_start_angle = z;
		break;
	}
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
		wxFileName::Mkdir(new_folder, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
	}

	//wxString s_fr_length = wxString ::Format("%d", end_frame);
	//m_fr_length = (int)s_fr_length.length();
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
	VRenderFrame* vframe = (VRenderFrame*)m_frame;
	if (vframe && vframe->GetSettingDlg())
		m_run_script = vframe->GetSettingDlg()->GetRunScript();

	//wxString s_fr_length = wxString ::Format("%d", end_frame);
	//m_fr_length = (int)s_fr_length.length();
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

	//wxString s_fr_length = wxString ::Format("%d", end_frame);
	//m_fr_length = (int)s_fr_length.length();
}

void VRenderGLView::SetParams(double t)
{
	if (!m_vrv)
		return;
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (!vr_frame)
		return;
	ClippingView* clip_view = vr_frame->GetClippingView();
	Interpolator *interpolator = vr_frame->GetInterpolator();
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
		vector<Plane*> *planes = vd->GetVR()->get_planes();
		if (!planes) continue;
		if (planes->size() != 6) continue;
		Plane *plane = 0;
		double val = 0;
		//x1
		plane = (*planes)[0];
		keycode.l2 = 0;
		keycode.l2_name = "x1_val";
		if (interpolator->GetDouble(keycode, t, val))
			plane->ChangePlane(Point(abs(val), 0.0, 0.0),
				Vector(1.0, 0.0, 0.0));
		//x2
		plane = (*planes)[1];
		keycode.l2 = 0;
		keycode.l2_name = "x2_val";
		if (interpolator->GetDouble(keycode, t, val))
			plane->ChangePlane(Point(abs(val), 0.0, 0.0),
				Vector(-1.0, 0.0, 0.0));
		//y1
		plane = (*planes)[2];
		keycode.l2 = 0;
		keycode.l2_name = "y1_val";
		if (interpolator->GetDouble(keycode, t, val))
			plane->ChangePlane(Point(0.0, abs(val), 0.0),
				Vector(0.0, 1.0, 0.0));
		//y2
		plane = (*planes)[3];
		keycode.l2 = 0;
		keycode.l2_name = "y2_val";
		if (interpolator->GetDouble(keycode, t, val))
			plane->ChangePlane(Point(0.0, abs(val), 0.0),
				Vector(0.0, -1.0, 0.0));
		//z1
		plane = (*planes)[4];
		keycode.l2 = 0;
		keycode.l2_name = "z1_val";
		if (interpolator->GetDouble(keycode, t, val))
			plane->ChangePlane(Point(0.0, 0.0, abs(val)),
				Vector(0.0, 0.0, 1.0));
		//z2
		plane = (*planes)[5];
		keycode.l2 = 0;
		keycode.l2_name = "z2_val";
		if (interpolator->GetDouble(keycode, t, val))
			plane->ChangePlane(Point(0.0, 0.0, abs(val)),
				Vector(0.0, 0.0, -1.0));
		//t
		double frame;
		keycode.l2 = 0;
		keycode.l2_name = "frame";
		if (interpolator->GetDouble(keycode, t, frame))
			Set4DSeqFrameVd(int(frame + 0.5), false,
				vd, vr_frame);
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
	Quaternion q;
	if (interpolator->GetQuaternion(keycode, t, q))
	{
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

	if (clip_view)
		clip_view->SetVolumeData(vr_frame->GetCurSelVol());
	if (vr_frame)
	{
		vr_frame->UpdateTree(m_cur_vol ? m_cur_vol->GetName() : "");
		int index = interpolator->GetKeyIndexFromTime(t);
		vr_frame->GetRecorderDlg()->SetSelection(index);
	}
	SetVolPopDirty();
}

void VRenderGLView::ResetMovieAngle()
{
	double rotx, roty, rotz;
	GetRotations(rotx, roty, rotz);

	switch (m_rot_axis)
	{
	case 1:  //x
		SetRotations(m_init_angle, roty, rotz);
		break;
	case 2:  //y
		SetRotations(rotx, m_init_angle, rotz);
		break;
	case 3:  //z
		SetRotations(rotx, roty, m_init_angle);
		break;
	}
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

void VRenderGLView::Get4DSeqFrames(int &start_frame, int &end_frame, int &cur_frame)
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
				cur_frame = vd_cur_frame;
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

void VRenderGLView::Set4DSeqFrameVd(int frame, bool run_script,
	VolumeData* vd, VRenderFrame* vframe)
{
	if (vd && vd->GetReader())
	{
		BaseReader* reader = vd->GetReader();
		bool clear_pool = false;

		if (vd->GetCurTime() != frame)
		{
			Texture *tex = vd->GetTexture();
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
				if (vframe && vframe->GetMeasureDlg())
					vframe->GetMeasureDlg()->UpdateList();
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
				if (vframe && vframe->GetMeasureDlg())
					vframe->GetMeasureDlg()->UpdateList();

				//run script
				if (run_script)
					Run4DScript(m_script_file, vd);

				clear_pool = true;
			}
		}

		if (clear_pool && vd->GetVR())
			vd->GetVR()->clear_tex_pool();
	}
}

void VRenderGLView::Set4DSeqFrame(int frame, bool run_script)
{
	int start_frame, end_frame, cur_frame;
	Get4DSeqFrames(start_frame, end_frame, cur_frame);
	if (frame > end_frame)
		frame = end_frame;
	if (frame < start_frame)
		frame = start_frame;

	if (frame == start_frame)
		m_sf_script = true;
	else
		m_sf_script = false;

	m_tseq_prv_num = m_tseq_cur_num;
	m_tseq_cur_num = frame;
	VRenderFrame* vframe = (VRenderFrame*)m_frame;
	if (vframe && vframe->GetSettingDlg())
	{
		m_run_script = vframe->GetSettingDlg()->GetRunScript();
		m_script_file = vframe->GetSettingDlg()->GetScriptFile();
	}

	//save currently selected volume
	VolumeData* cur_vd_save = m_cur_vol;
	for (int i = 0; i<(int)m_vd_pop_list.size(); i++)
	{
		VolumeData* vd = m_vd_pop_list[i];
		Set4DSeqFrameVd(frame, run_script,
			vd, vframe);
	}

	//run script
	if (run_script)
		Run4DScript(m_script_file);

	//restore currently selected volume
	m_cur_vol = cur_vd_save;
	m_selector.SetVolume(m_cur_vol);
	m_calculator.SetVolumeA(m_cur_vol);

	RefreshGL(17);
}

void VRenderGLView::Get3DBatFrames(int &start_frame, int &end_frame, int &cur_frame)
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
				cur_frame = 0;
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
	cur_frame -= start_frame;
	end_frame -= start_frame;
	start_frame = 0;
}

void VRenderGLView::Set3DBatFrame(int offset)
{
	int i, j;
	vector<BaseReader*> reader_list;
	m_bat_folder = "";

	m_tseq_prv_num = m_tseq_cur_num;
	m_tseq_cur_num = offset;
	for (i = 0; i<(int)m_vd_pop_list.size(); i++)
	{
		VolumeData* vd = m_vd_pop_list[i];
		if (vd && vd->GetReader())
		{
			Texture *tex = vd->GetTexture();
			BaseReader* reader = vd->GetReader();
			if (tex && tex->isBrxml())
			{
				BRKXMLReader *br = (BRKXMLReader *)reader;
				int curlv = tex->GetCurLevel();
				for (int j = 0; j < br->GetLevelNum(); j++)
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
				if (chan_num>0 && vd->GetCurChannel() >= chan_num)
					vd->SetDisp(false);
				else
					vd->SetDisp(true);

				if (reader->GetChanNum() > 1)
					data_name += wxString::Format("_%d", vd->GetCurChannel() + 1);
				vd->SetName(data_name);
			}
			else
			{
				//if (reader->GetOffset() == offset) return;
				bool found = false;
				for (j = 0; j<(int)reader_list.size(); j++)
				{
					if (reader == reader_list[j])
					{
						found = true;
						break;
					}
				}
				if (!found)
				{
					reader->LoadOffset(offset);
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
				if (chan_num>0 && vd->GetCurChannel() >= chan_num)
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

	RefreshGL(18);

	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
		vr_frame->UpdateList();
		vr_frame->UpdateTree(
			vr_frame->GetCurSelVol() ?
			vr_frame->GetCurSelVol()->GetName() :
			"");
	}
}

//pre-draw processings
void VRenderGLView::PreDraw()
{
	//skip if not done with loop
	if (TextureRenderer::get_mem_swap())
	{
		if (m_pre_draw)
			m_pre_draw = false;
		else
			return;
	}
}

void VRenderGLView::PostDraw()
{
	//skip if not done with loop
	if (TextureRenderer::get_mem_swap() &&
		TextureRenderer::get_start_update_loop() &&
		!TextureRenderer::get_done_update_loop())
		return;

	//output animations
	if (m_capture && !m_cap_file.IsEmpty())
	{
		//capture
		wxString outputfilename = m_cap_file;
		int x, y, w, h;

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

		if (m_enlarge)
		{
			glActiveTexture(GL_TEXTURE0);
			Framebuffer* final_buffer =
				TextureRenderer::framebuffer_manager_.framebuffer(
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
			ShaderProgram* img_shader =
				m_img_shader_factory.shader(IMG_SHDR_BLEND_BRIGHT_BACKGROUND_HDR);
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

		int chann = VRenderFrame::GetSaveAlpha() ? 4 : 3;
		glPixelStorei(GL_PACK_ROW_LENGTH, w);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		unsigned char *image = new unsigned char[w*h*chann];
		glReadBuffer(GL_BACK);
		glReadPixels(x, y, w, h, chann == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, image);
		glPixelStorei(GL_PACK_ROW_LENGTH, 0);

		if (m_enlarge)
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

		string str_fn = outputfilename.ToStdString();
		TIFF *out = TIFFOpen(str_fn.c_str(), "wb");
		if (!out)
			return;
		TIFFSetField(out, TIFFTAG_IMAGEWIDTH, w);
		TIFFSetField(out, TIFFTAG_IMAGELENGTH, h);
		TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, chann);
		TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 8);
		TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
		TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
		TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
		if (VRenderFrame::GetCompression())
			TIFFSetField(out, TIFFTAG_COMPRESSION, COMPRESSION_LZW);

		tsize_t linebytes = chann * w;
		unsigned char *buf = NULL;
		buf = (unsigned char *)_TIFFmalloc(linebytes);
		TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(out, 0));
		for (uint32 row = 0; row < (uint32)h; row++)
		{
			memcpy(buf, &image[(h - row - 1)*linebytes], linebytes);// check the index here, and figure out why not using h*linebytes
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
	if (TextureRenderer::get_mem_swap() &&
		TextureRenderer::get_start_update_loop() &&
		!TextureRenderer::get_done_update_loop())
		return;
	m_enlarge = false;
	ResizeFramebuffers();
	RefreshGL(19);
}

//run 4d script
void VRenderGLView::Run4DScript(wxString &scriptname, VolumeData* vd)
{
	if (m_run_script)
	{
		m_selector.SetVolume(vd);
		m_calculator.SetVolumeA(vd);
		m_cur_vol = vd;
	}
	else
		return;

	if (!wxFileExists(scriptname))
	{
		std::wstring name = scriptname.ToStdWstring();
		name = GET_NAME(name);
		wxString exePath = wxStandardPaths::Get().GetExecutablePath();
		exePath = wxPathOnly(exePath);
		scriptname = exePath + "\\Scripts\\" + name;

		if (!wxFileExists(scriptname))
			return;
	}
	wxFileInputStream is(scriptname);
	if (!is.IsOk())
		return;
	wxFileConfig fconfig(is);

	int i;
	wxString str;

	//tasks
	if (fconfig.Exists("/tasks"))
	{
		fconfig.SetPath("/tasks");
		int tasknum = fconfig.Read("tasknum", 0l);
		for (i = 0; i<tasknum; i++)
		{
			str = wxString::Format("/tasks/task%d", i);
			if (fconfig.Exists(str))
			{
				fconfig.SetPath(str);
				fconfig.Read("type", &str, "");
				if (vd)
				{
					if (str == "noise_reduction")
						RunNoiseReduction(fconfig);
					else if (str == "selection_tracking")
						RunSelectionTracking(fconfig);
					else if (str == "sparse_tracking")
						RunSparseTracking(fconfig);
					else if (str == "random_colors")
						RunRandomColors(fconfig);
					else if (str == "separate_channels")
						RunSeparateChannels(fconfig);
					else if (str == "external_exe")
						RunExternalExe(fconfig);
					else if (str == "fetch_mask")
						RunFetchMask(fconfig);
					else if (str == "save_mask")
						RunSaveMask(fconfig);
					else if (str == "opencl")
						RunOpenCL(fconfig);
					else if (str == "comp_analysis")
						RunCompAnalysis(fconfig);
					else if (str == "generate_comp")
						RunGenerateComp(fconfig);
					else if (str == "ruler_profile")
						RunRulerProfile(fconfig);
				}
				else
				{
					if (str == "save_volume")
						RunSaveVolume(fconfig);
					else if (str == "calculation")
						RunCalculation(fconfig);
				}
			}
		}
	}
}

void VRenderGLView::RunNoiseReduction(wxFileConfig &fconfig)
{
	wxString str;
	wxString pathname;
	double thresh, size;
	fconfig.Read("threshold", &thresh, 0.0);
	fconfig.Read("voxelsize", &size, 0.0);
	int mode;
	fconfig.Read("format", &mode, 0);
	bool bake;
	fconfig.Read("bake", &bake, false);
	bool compression;
	fconfig.Read("compress", &compression, false);
	fconfig.Read("savepath", &pathname, "");
	str = wxPathOnly(pathname);
	if (!wxDirExists(str))
		wxFileName::Mkdir(str, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);

	if (wxDirExists(str))
	{
		CompAnalysis(0.0, size, thresh, 1.0, false, false, false);
		Calculate(6, "", false);
		VolumeData* vd = m_calculator.GetResult();
		if (vd)
		{
			str = pathname;
			//time
			int time_num = vd->GetReader()->GetTimeNum();
			wxString format = wxString::Format("%d", time_num);
			m_fr_length = format.Length();
			format = wxString::Format("_T%%0%dd", m_fr_length);
			str += wxString::Format(format, m_tseq_cur_num);
			//channel
			int chan_num = vd->GetReader()->GetChanNum();
			if (chan_num > 1)
			{
				format = wxString::Format("%d", chan_num);
				int ch_length = format.Length();
				format = wxString::Format("_CH%%0%dd", ch_length + 1);
				str += wxString::Format(format, vd->GetCurChannel() + 1);
			}
			str += ".tif";
			vd->Save(str, mode, bake, compression);
			delete vd;
		}
	}
}

//add traces to trace dialog
#define UPDATE_TRACE_DLG_AND_RETURN \
	{ \
		VRenderFrame* vr_frame = (VRenderFrame*)m_frame; \
		if (m_vrv && vr_frame && vr_frame->GetTraceDlg()) \
			vr_frame->GetTraceDlg()->GetSettings(m_vrv); \
		return; \
	}

void VRenderGLView::RunSelectionTracking(wxFileConfig &fconfig)
{
	//read the size threshold
	int slimit;
	fconfig.Read("size_limit", &slimit, 0);
	//current state:
	//new data has been loaded in system memory
	//old data is in graphics memory
	//old mask in system memory is obsolete
	//new mask is in graphics memory
	//old label is in system memory
	//no label in graphics memory
	//steps:
	int ii, jj, kk;
	int nx, ny, nz;
	//return current mask (into system memory)
	if (!m_cur_vol)
		UPDATE_TRACE_DLG_AND_RETURN

		m_cur_vol->GetVR()->return_mask();
	m_cur_vol->GetResolution(nx, ny, nz);
	//find labels in the old that are selected by the current mask
	Nrrd* mask_nrrd = m_cur_vol->GetMask(true);
	if (!mask_nrrd)
	{
		m_cur_vol->AddEmptyMask(0);
		mask_nrrd = m_cur_vol->GetMask(false);
	}
	Nrrd* label_nrrd = m_cur_vol->GetLabel(false);
	if (!label_nrrd)
		UPDATE_TRACE_DLG_AND_RETURN

		unsigned char* mask_data = (unsigned char*)(mask_nrrd->data);
	if (!mask_data)
		UPDATE_TRACE_DLG_AND_RETURN

		unsigned int* label_data = (unsigned int*)(label_nrrd->data);
	if (!label_data)
		UPDATE_TRACE_DLG_AND_RETURN

		FL::CellList sel_labels;
	FL::CellListIter label_iter;
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
						FL::pCell cell(new FL::Cell(label_value));
						cell->SetSizeUi(1);
						sel_labels.insert(pair<unsigned int, FL::pCell>
							(label_value, cell));
					}
					else
						label_iter->second->Inc();
				}
			}
	//clean label list according to the size limit
	label_iter = sel_labels.begin();
	while (label_iter != sel_labels.end())
	{
		if (label_iter->second->GetSizeUi() < (unsigned int)slimit)
			label_iter = sel_labels.erase(label_iter);
		else
			++label_iter;
	}
	if (m_trace_group &&
		m_trace_group->GetTrackMap()->GetFrameNum())
	{
		//create new id list
		m_trace_group->SetCurTime(m_tseq_cur_num);
		m_trace_group->SetPrvTime(m_tseq_prv_num);
		m_trace_group->UpdateCellList(sel_labels);
	}
	//load and replace the label
	BaseReader* reader = m_cur_vol->GetReader();
	if (!reader)
		UPDATE_TRACE_DLG_AND_RETURN

		LBLReader lbl_reader;
	wstring lblname = reader->GetCurLabelName(m_tseq_cur_num, m_cur_vol->GetCurChannel());
	lbl_reader.SetFile(lblname);
	Nrrd* label_nrrd_new = lbl_reader.Convert(m_tseq_cur_num, m_cur_vol->GetCurChannel(), true);
	if (!label_nrrd_new)
	{
		m_cur_vol->AddEmptyLabel();
		label_nrrd_new = m_cur_vol->GetLabel(false);
	}
	else
		m_cur_vol->LoadLabel(label_nrrd_new);
	label_data = (unsigned int*)(label_nrrd_new->data);
	if (!label_data)
		UPDATE_TRACE_DLG_AND_RETURN

		//update the mask according to the new label
		memset((void*)mask_data, 0, sizeof(uint8)*nx*ny*nz);
	for (ii = 0; ii<nx; ii++)
		for (jj = 0; jj<ny; jj++)
			for (kk = 0; kk<nz; kk++)
			{
				int index = nx*ny*kk + nx*jj + ii;
				unsigned int label_value = label_data[index];
				if (m_trace_group &&
					m_trace_group->GetTrackMap()->GetFrameNum())
				{
					if (m_trace_group->FindCell(label_value))
						mask_data[index] = 255;
				}
				else
				{
					label_iter = sel_labels.find(label_value);
					if (label_iter != sel_labels.end())
						mask_data[index] = 255;
				}
			}

	UPDATE_TRACE_DLG_AND_RETURN
}

void VRenderGLView::RunSparseTracking(wxFileConfig &fconfig)
{
	if (!m_trace_group)
		CreateTraceGroup();

	FL::pTrackMap track_map = m_trace_group->GetTrackMap();
	FL::TrackMapProcessor tm_processor(track_map);
	int resx, resy, resz;
	m_cur_vol->GetResolution(resx, resy, resz);
	double spcx, spcy, spcz;
	m_cur_vol->GetSpacings(spcx, spcy, spcz);
	tm_processor.SetBits(m_cur_vol->GetBits());
	tm_processor.SetScale(m_cur_vol->GetScalarScale());
	tm_processor.SetSizes(resx, resy, resz);
	tm_processor.SetSpacings(spcx, spcy, spcz);
	//tm_processor.SetSizeThresh(component_size);
	//tm_processor.SetContactThresh(contact_factor);
	//register file reading and deleteing functions
	tm_processor.RegisterCacheQueueFuncs(
		boost::bind(&VRenderGLView::ReadVolCache, this, _1),
		boost::bind(&VRenderGLView::DelVolCache, this, _1));

	tm_processor.TrackStencils(m_tseq_prv_num, m_tseq_cur_num);

	//add traces to trace dialog
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (m_vrv && vr_frame && vr_frame->GetTraceDlg())
		vr_frame->GetTraceDlg()->GetSettings(m_vrv);
}

void VRenderGLView::RunRandomColors(wxFileConfig &fconfig)
{
	int hmode;
	fconfig.Read("huemode", &hmode, 1);
	wxString str, pathname;
	fconfig.Read("savepath", &pathname, "");
	str = wxPathOnly(pathname);
	if (!wxDirExists(str))
		wxFileName::Mkdir(str, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);

	if (wxDirExists(str))
	{
		//current state: see selection_tracking
		//steps:
		//load and replace the label
		if (!m_cur_vol)
			return;
		VolumeData* vd = m_cur_vol;
		BaseReader* reader = vd->GetReader();
		if (!reader)
			return;
		LBLReader lbl_reader;
		wstring lblname = reader->GetCurLabelName(m_tseq_cur_num, m_cur_vol->GetCurChannel());
		lbl_reader.SetFile(lblname);
		Nrrd* label_nrrd_new = lbl_reader.Convert(m_tseq_cur_num, m_cur_vol->GetCurChannel(), true);
		if (!label_nrrd_new)
			return;
		vd->LoadLabel(label_nrrd_new);
		int time_num = reader->GetTimeNum();
		//generate RGB volumes
		m_selector.CompExportRandomColor(hmode, 0, 0, 0, false, false);
		vector<VolumeData*> *vol_list = m_selector.GetResultVols();
		for (int ii = 0; ii < (int)vol_list->size(); ii++)
		{
			vd = (*vol_list)[ii];
			if (!vd)
				break;

			//time
			wxString format = wxString::Format("%d", time_num);
			m_fr_length = format.Length();
			format = wxString::Format("_T%%0%dd", m_fr_length);
			str += wxString::Format(format, m_tseq_cur_num);
			//channel
			int chan_num = vd->GetReader()->GetChanNum();
			if (chan_num > 1)
			{
				format = wxString::Format("%d", chan_num);
				int ch_length = format.Length();
				format = wxString::Format("_CH%%0%dd", ch_length + 1);
				str += wxString::Format(format, vd->GetCurChannel() + 1);
			}
			//comp
			str += wxString::Format("_COMP%d", ii + 1) + ".tif";
			vd->Save(str);
			delete vd;
		}
	}
}

void VRenderGLView::RunSeparateChannels(wxFileConfig &fconfig)
{
	wxString str, pathname;
	int mode;
	fconfig.Read("format", &mode, 0);
	bool bake;
	fconfig.Read("bake", &bake, false);
	bool compression;
	fconfig.Read("compress", &compression, false);
	fconfig.Read("savepath", &pathname, "");
	str = wxPathOnly(pathname);
	if (!wxDirExists(str))
		wxFileName::Mkdir(str, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);

	if (wxDirExists(str))
	{
		if (!m_cur_vol)
			return;

		VolumeData* vd = m_cur_vol;

		str = pathname;
		//time
		int time_num = vd->GetReader()->GetTimeNum();
		wxString format = wxString::Format("%d", time_num);
		m_fr_length = format.Length();
		format = wxString::Format("_T%%0%dd", m_fr_length);
		str += wxString::Format(format, m_tseq_cur_num);
		//channel
		int chan_num = vd->GetReader()->GetChanNum();
		format = wxString::Format("%d", chan_num);
		int ch_length = format.Length();
		format = wxString::Format("_CH%%0%dd", ch_length + 1);
		str += wxString::Format(format, vd->GetCurChannel() + 1) + ".tif";
		vd->Save(str, mode, bake, compression);
	}
}

void VRenderGLView::RunExternalExe(wxFileConfig &fconfig)
{
#ifndef __linux__
	wxString pathname;
	fconfig.Read("exepath", &pathname);
	if (!wxFileExists(pathname))
		return;
	VolumeData* vd = m_cur_vol;
	if (!vd)
		return;
	BaseReader* reader = vd->GetReader();
	if (!reader)
		return;
	wxString data_name = reader->GetCurDataName(m_tseq_cur_num, vd->GetCurChannel());

	vector<string> args;
	args.push_back(pathname.ToStdString());
	args.push_back(data_name.ToStdString());
	boost::process::child c(pathname.ToStdString(),
		data_name.ToStdString());
	c.wait();
#endif
}

void VRenderGLView::RunFetchMask(wxFileConfig &fconfig)
{
	//load and replace the mask
	if (!m_cur_vol)
		return;
	BaseReader* reader = m_cur_vol->GetReader();
	if (!reader)
		return;
	MSKReader msk_reader;
	wstring mskname = reader->GetCurMaskName(m_tseq_cur_num, m_cur_vol->GetCurChannel());
	msk_reader.SetFile(mskname);
	Nrrd* mask_nrrd_new = msk_reader.Convert(m_tseq_cur_num, m_cur_vol->GetCurChannel(), true);
	if (mask_nrrd_new)
		m_cur_vol->LoadMask(mask_nrrd_new);

	//load and replace the label
	LBLReader lbl_reader;
	wstring lblname = reader->GetCurLabelName(m_tseq_cur_num, m_cur_vol->GetCurChannel());
	lbl_reader.SetFile(lblname);
	Nrrd* label_nrrd_new = lbl_reader.Convert(m_tseq_cur_num, m_cur_vol->GetCurChannel(), true);
	if (label_nrrd_new)
		m_cur_vol->LoadLabel(label_nrrd_new);
}

void VRenderGLView::RunSaveMask(wxFileConfig &fconfig)
{
	if (!m_cur_vol)
		return;
	int toffset;
	fconfig.Read("toffset", &toffset, 0);
	int time;
	if (toffset)
		time = m_tseq_cur_num;
	else
		time = m_tseq_prv_num;
	m_cur_vol->SaveMask(true, time, m_cur_vol->GetCurChannel());
	m_cur_vol->SaveLabel(true, time, m_cur_vol->GetCurChannel());
}

void VRenderGLView::RunSaveVolume(wxFileConfig &fconfig)
{
	int mode;
	fconfig.Read("format", &mode, 0);
	bool bake;
	fconfig.Read("bake", &bake, false);
	bool compression;
	fconfig.Read("compress", &compression, false);
	wxString str, pathname;
	fconfig.Read("savepath", &pathname, "");
	bool del_vol;
	fconfig.Read("delete", &del_vol, false);
	str = wxPathOnly(pathname);
	if (!wxDirExists(str))
		wxFileName::Mkdir(str, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
	if (wxDirExists(str))
	{
		VolumeData* vd = m_calculator.GetResult();
		if (!vd || !m_vd_pop_list[0])
			return;

		str = pathname;
		//time
		int time_num = m_vd_pop_list[0]->GetReader()->GetTimeNum();
		wxString format = wxString::Format("%d", time_num);
		m_fr_length = format.Length();
		format = wxString::Format("_T%%0%dd", m_fr_length);
		str += wxString::Format(format, m_tseq_cur_num);
		if (mode == 0 || mode == 1)
			str += ".tif";
		else if (mode == 2)
			str += ".nrrd";
		vd->Save(str, mode, bake, compression);
		if (del_vol)
			delete vd;
	}
}

void VRenderGLView::RunCalculation(wxFileConfig &fconfig)
{
	int vol_a_index;
	fconfig.Read("vol_a", &vol_a_index, 0);
	int vol_b_index;
	fconfig.Read("vol_b", &vol_b_index, 0);
	wxString sOperand;
	fconfig.Read("operand", &sOperand, "");

	//get volumes
	VolumeData* vol_a = 0;
	if (vol_a_index >= 0 && vol_a_index < (int)m_vd_pop_list.size())
		vol_a = m_vd_pop_list[vol_a_index];
	VolumeData* vol_b = 0;
	if (vol_b_index >= 0 && vol_b_index < (int)m_vd_pop_list.size())
		vol_b = m_vd_pop_list[vol_b_index];
	//calculate
	SetVolumeA(vol_a);
	SetVolumeB(vol_b);
	if (sOperand == "subtract")
		Calculate(1, "", false);
	else if (sOperand == "add")
		Calculate(2, "", false);
	else if (sOperand == "divide")
		Calculate(3, "", false);
	else if (sOperand == "colocate")
		Calculate(4, "", false);
	else if (sOperand == "fill")
		Calculate(9, "", false);
}

void VRenderGLView::RunOpenCL(wxFileConfig &fconfig)
{
	wxString str, clname, pathname;
	fconfig.Read("clpath", &clname, "");
	if (!wxFileExists(clname))
		return;
	fconfig.Read("savepath", &pathname, "");
	int mode;
	fconfig.Read("format", &mode, 0);
	bool bake;
	fconfig.Read("bake", &bake, false);
	bool compression;
	fconfig.Read("compress", &compression, false);

	str = wxPathOnly(pathname);
	if (!wxDirExists(str))
		wxFileName::Mkdir(str, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);

	if (wxDirExists(str))
	{
		if (!m_cur_vol)
			return;

		VolumeData* vd = m_cur_vol;

		m_cur_vol->GetVR()->clear_tex_current();
		m_kernel_executor.LoadCode(clname);
		m_kernel_executor.SetVolume(m_cur_vol);
		m_kernel_executor.SetDuplicate(true);
		bool result = m_kernel_executor.Execute();
		VolumeData* vd_r = m_kernel_executor.GetResult();
		if (!result || !vd_r)
			return;

		str = pathname;
		//time
		int time_num = vd->GetReader()->GetTimeNum();
		wxString format = wxString::Format("%d", time_num);
		m_fr_length = format.Length();
		format = wxString::Format("_T%%0%dd", m_fr_length);
		str += wxString::Format(format, m_tseq_cur_num);
		//channel
		int chan_num = vd->GetReader()->GetChanNum();
		if (chan_num > 1)
		{
			format = wxString::Format("%d", chan_num);
			int ch_length = format.Length();
			format = wxString::Format("_CH%%0%dd", ch_length + 1);
			str += wxString::Format(format, vd->GetCurChannel() + 1);
		}
		str += ".tif";
		vd_r->Save(str, mode, bake, compression);

		m_kernel_executor.DeleteResult();
	}
}

void VRenderGLView::RunCompAnalysis(wxFileConfig &fconfig)
{
	wxString str, pathname;
	fconfig.Read("savepath", &pathname);
	int verbose;
	fconfig.Read("verbose", &verbose, 0);
	bool consistent;
	fconfig.Read("consistent", &consistent, true);


	str = wxPathOnly(pathname);
	if (!wxDirExists(str))
		wxFileName::Mkdir(str, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);

	if (!m_cur_vol)
		return;

	FL::ComponentAnalyzer comp_analyzer(m_cur_vol);
	comp_analyzer.Analyze(true, consistent);
	string result_str;
	string comp_header = wxString::Format("%d", m_tseq_cur_num);
	comp_analyzer.OutputCompListStr(result_str, verbose, comp_header);

	//save append
	wxFile file(pathname, m_sf_script ? wxFile::write : wxFile::write_append);
	if (!file.IsOpened())
		return;
	if (m_sf_script && verbose == 0)
	{
		string header;
		comp_analyzer.OutputFormHeader(header);
		file.Write(wxString::Format("Time\t"));
		file.Write(header);
	}
	if (verbose == 1)
		file.Write(wxString::Format("Time point: %d\n", m_tseq_cur_num));
	file.Write(result_str);
	file.Close();
}

void VRenderGLView::RunGenerateComp(wxFileConfig &fconfig)
{
	int type;
	fconfig.Read("gentype", &type, 0);
	int mode;
	fconfig.Read("mode", &mode, 0);

	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetComponentDlg())
		vr_frame->GetComponentDlg()->GenerateComp(type, mode);
}

void VRenderGLView::RunRulerProfile(wxFileConfig &fconfig)
{
	if (m_ruler_list.empty())
		return;

	for (size_t i = 0; i<m_ruler_list.size(); ++i)
		RulerProfile(i);

	if (m_tseq_cur_num == 0 ||
		m_script_output.IsEmpty())
	{
		wxString path;
		if (m_cur_vol)
		{
			path = m_cur_vol->GetPath();
			path = wxPathOnly(path);
		}
		path += GETSLASH();
		path += "profiles_1.txt";

		while (wxFileExists(path))
		{
			int pos = path.Find('_', true);
			if (pos == wxNOT_FOUND)
			{
				path = path.SubString(0, path.Length() - 4);
				path += "_1.txt";
			}
			else
			{
				wxString digits;
				for (int i = pos+1; i < path.Length() - 1; ++i)
				{
					if (wxIsdigit(path[i]))
						digits += path[i];
					else
						break;
				}
				long num = 0;
				digits.ToLong(&num);
				path = path.SubString(0, pos);
				path += wxString::Format("%d.txt", num + 1);
			}
		}
		m_script_output = path;
	}

	//save append
	wxFile file(m_script_output, m_sf_script ? wxFile::write : wxFile::write_append);
	if (!file.IsOpened())
		return;

	//get df/f setting
	bool df_f = false;
	VRenderFrame* frame = (VRenderFrame*)m_frame;
	if (frame && frame->GetSettingDlg())
		df_f = frame->GetSettingDlg()->GetRulerDF_F();
	double f = 0.0;

	for (size_t i = 0; i<m_ruler_list.size(); ++i)
	{
		//for each ruler
		wxString str;
		Ruler* ruler = m_ruler_list[i];
		if (!ruler) continue;

		vector<ProfileBin>* profile = ruler->GetProfile();
		if (profile && profile->size())
		{
			double sumd = 0.0;
			unsigned long long sumull = 0;
			for (size_t j = 0; j<profile->size(); ++j)
			{
				//for each profile
				int pixels = (*profile)[j].m_pixels;
				if (pixels <= 0)
					str += "0.0\t";
				else
				{
					str += wxString::Format("%f\t", (*profile)[j].m_accum / pixels);
					sumd += (*profile)[j].m_accum;
					sumull += pixels;
				}
			}
			if (df_f)
			{
				double avg = 0.0;
				if (sumull != 0)
					avg = sumd / double(sumull);
				if (i == 0)
				{
					f = avg;
					str += wxString::Format("\t%f\t", f);
				}
				else
				{
					double df = avg - f;
					if (f == 0.0)
						str += wxString::Format("\t%f\t", df);
					else
						str += wxString::Format("\t%f\t", df / f);
				}
			}
		}
		str += "\t";
		file.Write(str);
	}
	file.Write("\n");
	file.Close();
}

//read/delete volume cache
void VRenderGLView::ReadVolCache(FL::VolCache& vol_cache)
{
	//get volume, readers
	if (!m_cur_vol)
		return;
	BaseReader* reader = m_cur_vol->GetReader();
	if (!reader)
		return;
	LBLReader lbl_reader;

	int chan = m_cur_vol->GetCurChannel();
	int frame = vol_cache.frame;

	Nrrd* data = reader->Convert(frame, chan, true);
	vol_cache.nrrd_data = data;
	vol_cache.data = data->data;
	wstring lblname = reader->GetCurLabelName(frame, chan);
	lbl_reader.SetFile(lblname);
	Nrrd* label = lbl_reader.Convert(frame, chan, true);
	if (!label)
	{
		int resx, resy, resz;
		m_cur_vol->GetResolution(resx, resy, resz);
		double spcx, spcy, spcz;
		m_cur_vol->GetSpacings(spcx, spcy, spcz);
		label = nrrdNew();
		unsigned long long mem_size = (unsigned long long)resx*
			(unsigned long long)resy*(unsigned long long)resz;
		unsigned int *val32 = new (std::nothrow) unsigned int[mem_size];
		memset(val32, 0, sizeof(unsigned int)*mem_size);
		nrrdWrap(label, val32, nrrdTypeUInt, 3, (size_t)resx, (size_t)resy, (size_t)resz);
		nrrdAxisInfoSet(label, nrrdAxisInfoSpacing, spcx, spcy, spcz);
		nrrdAxisInfoSet(label, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
		nrrdAxisInfoSet(label, nrrdAxisInfoMax, spcx*resx, spcy*resy, spcz*resz);
		nrrdAxisInfoSet(label, nrrdAxisInfoSize, (size_t)resx, (size_t)resy, (size_t)resz);
	}
	vol_cache.nrrd_label = label;
	vol_cache.label = label->data;
	if (data && label)
		vol_cache.valid = true;
}

void VRenderGLView::DelVolCache(FL::VolCache& vol_cache)
{
	if (!m_cur_vol)
		return;
	vol_cache.valid = false;
	if (vol_cache.data)
	{
		nrrdNuke((Nrrd*)vol_cache.nrrd_data);
		vol_cache.data = 0;
		vol_cache.nrrd_data = 0;
	}
	if (vol_cache.label)
	{
		int chan = m_cur_vol->GetCurChannel();
		int frame = vol_cache.frame;
		double spcx, spcy, spcz;
		m_cur_vol->GetSpacings(spcx, spcy, spcz);

		MSKWriter msk_writer;
		msk_writer.SetData((Nrrd*)(vol_cache.nrrd_label));
		msk_writer.SetSpacings(spcx, spcy, spcz);
		BaseReader* reader = m_cur_vol->GetReader();
		if (reader)
		{
			wstring filename;
			filename = reader->GetCurLabelName(frame, chan);
			msk_writer.Save(filename, 1);
		}

		nrrdNuke((Nrrd*)vol_cache.nrrd_label);
		vol_cache.label = 0;
		vol_cache.nrrd_label = 0;
	}
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
		VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
		if (vr_frame)
		{
			for (int i = 0; i<vr_frame->GetViewNum(); i++)
			{
				VRenderView* view = vr_frame->GetView(i);
				if (view && view->m_glview &&
					view->m_glview != this)
				{
					view->m_glview->m_set_gl = false;
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

	int nx = GetGLSize().x;
	int ny = GetGLSize().y;

	PopMeshList();
	if (m_md_pop_list.size()>0)
		m_draw_type = 2;
	else
		m_draw_type = 1;

	m_drawing = true;
	PreDraw();

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


	if (TextureRenderer::get_invalidate_tex())
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

	SwapBuffers();
	m_timer->sample();
	m_drawing = false;
#ifdef _DEBUG
	std::wostringstream os;
	os << "buffer swapped" << "\t" <<
		m_interactive << "\n";
	OutputDebugString(os.str().c_str());
#endif

	if (m_resize)
		m_resize = false;

	if (m_enlarge)
		ResetEnlarge();

	if (m_linked_rot)
	{
		if (!m_master_linked_view ||
			this != m_master_linked_view)
			return;

		VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
		if (vr_frame)
		{
			for (int i = 0; i<vr_frame->GetViewNum(); i++)
			{
				VRenderView* view = vr_frame->GetView(i);
				if (view && view->m_glview &&
					view->m_glview != this)
				{
					view->m_glview->SetRotations(m_rotx, m_roty, m_rotz, true);
					view->RefreshGL();
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
		BBox bbox = vd->GetBounds();
		VolumeRenderer *vr = vd->GetVR();
		if (!vr) return;
		vector<Plane*> *planes = vr->get_planes();
		if (planes->size() != 6) return;
		double x1, x2, y1, y2, z1, z2;
		double abcd[4];
		(*planes)[0]->get_copy(abcd);
		x1 = fabs(abcd[3])*bbox.max().x();
		(*planes)[1]->get_copy(abcd);
		x2 = fabs(abcd[3])*bbox.max().x();
		(*planes)[2]->get_copy(abcd);
		y1 = fabs(abcd[3])*bbox.max().y();
		(*planes)[3]->get_copy(abcd);
		y2 = fabs(abcd[3])*bbox.max().y();
		(*planes)[4]->get_copy(abcd);
		z1 = fabs(abcd[3])*bbox.max().z();
		(*planes)[5]->get_copy(abcd);
		z2 = fabs(abcd[3])*bbox.max().z();

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

	int nx = GetGLSize().x;
	int ny = GetGLSize().y;
	double aspect = (double)nx / (double)ny;

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
	spc_x = spc_x<EPS ? 1.0 : spc_x;
	spc_y = spc_y<EPS ? 1.0 : spc_y;

	if (aspect > 1.0)
		result = 2.0*m_radius / spc_y / double(ny);
	else
		result = 2.0*m_radius / spc_x / double(nx);

	return result;
}

void VRenderGLView::SetScale121()
{
	//SetCenter();

	m_scale_factor = Get121ScaleFactor();
	double value = 1.0;
	if (m_scale_mode)
		value = m_scale_factor;

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
		Vector pos(m_transx, m_transy, m_transz);
		Vector d = pos;
		d.normalize();
		Vector ctr;
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
		Color gamma = group->GetGamma();
		vd->SetGamma(gamma);
		Color brightness = group->GetBrightness();
		vd->SetBrightness(brightness);
		Color hdr = group->GetHdr();
		vd->SetHdr(hdr);
		bool sync_r = group->GetSyncR();
		vd->SetSyncR(sync_r);
		bool sync_g = group->GetSyncG();
		vd->SetSyncG(sync_g);
		bool sync_b = group->GetSyncB();
		vd->SetSyncB(sync_b);

		VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
		if (vr_frame)
		{
			vr_frame->GetAdjustView()->SetVolumeData(vd);
			vr_frame->GetAdjustView()->SetGroupLink(group);
		}
	}

	m_vd_pop_dirty = true;

	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (m_frame)
	{
		AdjustView* adjust_view = vr_frame->GetAdjustView();
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

	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (!vr_frame) return;
	DataManager *dm = vr_frame->GetDataManager();
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
		VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
		if (vr_frame)
		{
			AdjustView* adjust_view = vr_frame->GetAdjustView();
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
				if (m_cur_vol = vd)
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
					if (m_cur_vol = vd)
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
				if (m_cur_vol = vd)
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
						if (m_cur_vol = vd)
							m_cur_vol = 0;
					}
				}
			}
			++iter;
		}
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
	Color gamma = group->GetGamma();
	src_vd->SetGamma(gamma);
	Color brightness = group->GetBrightness();
	src_vd->SetBrightness(brightness);
	Color hdr = group->GetHdr();
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
	Color gamma = dst_group->GetGamma();
	src_vd->SetGamma(gamma);
	Color brightness = dst_group->GetBrightness();
	src_vd->SetBrightness(brightness);
	Color hdr = dst_group->GetHdr();
	src_vd->SetHdr(hdr);
	bool sync_r = dst_group->GetSyncR();
	src_vd->SetSyncR(sync_r);
	bool sync_g = dst_group->GetSyncG();
	src_vd->SetSyncG(sync_g);
	bool sync_b = dst_group->GetSyncB();
	src_vd->SetSyncB(sync_b);

	m_vd_pop_dirty = true;
	m_md_pop_dirty = true;

	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (m_frame)
	{
		AdjustView* adjust_view = vr_frame->GetAdjustView();
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
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
		AdjustView* adjust_view = vr_frame->GetAdjustView();
		if (adjust_view && group)
		{
			Color gamma, brightness, hdr;
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
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
		AdjustView* adjust_view = vr_frame->GetAdjustView();
		if (adjust_view)
		{
			Color gamma, brightness, hdr;
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
			Vector diag = m_bounds.diagonal();
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
			m_obj_ctrx = (m_bounds.min().x() + m_bounds.max().x()) / 2.0;
			m_obj_ctry = (m_bounds.min().y() + m_bounds.max().y()) / 2.0;
			m_obj_ctrz = (m_bounds.min().z() + m_bounds.max().z()) / 2.0;
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
			m_q = Quaternion(0, 0, 0, 1);
			m_q.ToEuler(m_rotx, m_roty, m_rotz);
		}
	}

	m_init_view = true;

}

void VRenderGLView::DrawBounds()
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	vector<float> vertex;
	vertex.reserve(16 * 3);

	vertex.push_back(m_bounds.min().x()); vertex.push_back(m_bounds.min().y()); vertex.push_back(m_bounds.min().z());
	vertex.push_back(m_bounds.max().x()); vertex.push_back(m_bounds.min().y()); vertex.push_back(m_bounds.min().z());
	vertex.push_back(m_bounds.max().x()); vertex.push_back(m_bounds.max().y()); vertex.push_back(m_bounds.min().z());
	vertex.push_back(m_bounds.min().x()); vertex.push_back(m_bounds.max().y()); vertex.push_back(m_bounds.min().z());

	vertex.push_back(m_bounds.min().x()); vertex.push_back(m_bounds.min().y()); vertex.push_back(m_bounds.max().z());
	vertex.push_back(m_bounds.max().x()); vertex.push_back(m_bounds.min().y()); vertex.push_back(m_bounds.max().z());
	vertex.push_back(m_bounds.max().x()); vertex.push_back(m_bounds.max().y()); vertex.push_back(m_bounds.max().z());
	vertex.push_back(m_bounds.min().x()); vertex.push_back(m_bounds.max().y()); vertex.push_back(m_bounds.max().z());

	vertex.push_back(m_bounds.min().x()); vertex.push_back(m_bounds.min().y()); vertex.push_back(m_bounds.min().z());
	vertex.push_back(m_bounds.min().x()); vertex.push_back(m_bounds.min().y()); vertex.push_back(m_bounds.max().z());
	vertex.push_back(m_bounds.max().x()); vertex.push_back(m_bounds.min().y()); vertex.push_back(m_bounds.min().z());
	vertex.push_back(m_bounds.max().x()); vertex.push_back(m_bounds.min().y()); vertex.push_back(m_bounds.max().z());
	vertex.push_back(m_bounds.max().x()); vertex.push_back(m_bounds.max().y()); vertex.push_back(m_bounds.min().z());
	vertex.push_back(m_bounds.max().x()); vertex.push_back(m_bounds.max().y()); vertex.push_back(m_bounds.max().z());
	vertex.push_back(m_bounds.min().x()); vertex.push_back(m_bounds.max().y()); vertex.push_back(m_bounds.min().z());
	vertex.push_back(m_bounds.min().x()); vertex.push_back(m_bounds.max().y()); vertex.push_back(m_bounds.max().z());

	ShaderProgram* shader =
		m_img_shader_factory.shader(IMG_SHDR_DRAW_GEOMETRY);
	if (shader)
	{
		if (!shader->valid())
			shader->create();
		shader->bind();
	}
	shader->setLocalParam(0, 1.0, 1.0, 1.0, 1.0);
	glm::mat4 matrix = m_proj_mat * m_mv_mat;
	shader->setLocalParamMatrix(0, glm::value_ptr(matrix));

	glBindBuffer(GL_ARRAY_BUFFER, m_misc_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertex.size(), &vertex[0], GL_DYNAMIC_DRAW);
	glBindVertexArray(m_misc_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_misc_vbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const GLvoid*)0);
	glDrawArrays(GL_LINE_LOOP, 0, 4);
	glDrawArrays(GL_LINE_LOOP, 4, 4);
	glDrawArrays(GL_LINES, 8, 8);
	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	if (shader && shader->valid())
		shader->release();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
}

void VRenderGLView::DrawClippingPlanes(bool border, int face_winding)
{
	int i;
	bool link = false;
	PLANE_MODES plane_mode = kNormal;
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetClippingView())
	{
		link = vr_frame->GetClippingView()->GetChannLink();
		plane_mode = vr_frame->GetClippingView()->GetPlaneMode();
	}

	if (plane_mode == kNone)
		return;

	bool draw_plane = plane_mode != kFrame;
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

	if (!border && plane_mode == kFrame)
		return;

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

	ShaderProgram* shader =
		m_img_shader_factory.shader(IMG_SHDR_DRAW_GEOMETRY);
	if (shader)
	{
		if (!shader->valid())
			shader->create();
		shader->bind();
	}

	for (i = 0; i<GetDispVolumeNum(); i++)
	{
		VolumeData* vd = GetDispVolumeData(i);
		if (!vd)
			continue;

		if (vd != m_cur_vol)
			continue;

		VolumeRenderer *vr = vd->GetVR();
		if (!vr)
			continue;

		vector<Plane*> *planes = vr->get_planes();
		if (planes->size() != 6)
			continue;

		//calculating planes
		//get six planes
		Plane* px1 = (*planes)[0];
		Plane* px2 = (*planes)[1];
		Plane* py1 = (*planes)[2];
		Plane* py2 = (*planes)[3];
		Plane* pz1 = (*planes)[4];
		Plane* pz2 = (*planes)[5];

		//calculate 4 lines
		Vector lv_x1z1, lv_x1z2, lv_x2z1, lv_x2z2;
		Point lp_x1z1, lp_x1z2, lp_x2z1, lp_x2z2;
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
		Point pp[8];
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

		//draw the six planes out of the eight points
		//get color
		Color color(1.0, 1.0, 1.0);
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
		Transform *tform = vd->GetTexture()->transform();
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
		shader->setLocalParamMatrix(0, glm::value_ptr(matrix));

		vector<float> vertex;
		vertex.reserve(8 * 3);
		vector<uint32_t> index;
		index.reserve(6 * 4 * 2);

		//vertices
		for (size_t pi = 0; pi<8; ++pi)
		{
			vertex.push_back(pp[pi].x());
			vertex.push_back(pp[pi].y());
			vertex.push_back(pp[pi].z());
		}
		//indices
		index.push_back(4); index.push_back(0); index.push_back(5); index.push_back(1);
		index.push_back(4); index.push_back(0); index.push_back(1); index.push_back(5);
		index.push_back(7); index.push_back(3); index.push_back(6); index.push_back(2);
		index.push_back(7); index.push_back(3); index.push_back(2); index.push_back(6);
		index.push_back(1); index.push_back(0); index.push_back(3); index.push_back(2);
		index.push_back(1); index.push_back(0); index.push_back(2); index.push_back(3);
		index.push_back(4); index.push_back(5); index.push_back(6); index.push_back(7);
		index.push_back(4); index.push_back(5); index.push_back(7); index.push_back(6);
		index.push_back(0); index.push_back(4); index.push_back(2); index.push_back(6);
		index.push_back(0); index.push_back(4); index.push_back(6); index.push_back(2);
		index.push_back(5); index.push_back(1); index.push_back(7); index.push_back(3);
		index.push_back(5); index.push_back(1); index.push_back(3); index.push_back(7);

		glBindBuffer(GL_ARRAY_BUFFER, m_misc_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertex.size(), &vertex[0], GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_misc_ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t)*index.size(), &index[0], GL_DYNAMIC_DRAW);

		glBindVertexArray(m_misc_vao);
		glBindBuffer(GL_ARRAY_BUFFER, m_misc_vbo);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const GLvoid*)0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_misc_ibo);

		//draw
		//x1 = (p4, p0, p1, p5)
		if (m_clip_mask & 1)
		{
			if (draw_plane)
			{
				if (plane_mode == kNormal ||
					plane_mode == kNormalBack)
					shader->setLocalParam(0, 1.0, 0.5, 0.5, plane_trans);
				else
					shader->setLocalParam(0, color.r(), color.g(), color.b(), plane_trans);
				glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, (const GLvoid*)0);
			}
			if (border)
			{
				shader->setLocalParam(0, color.r(), color.g(), color.b(), plane_trans);
				glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, (const GLvoid*)(4 * 4));
			}
		}
		//x2 = (p7, p3, p2, p6)
		if (m_clip_mask & 2)
		{
			if (draw_plane)
			{
				if (plane_mode == kNormal ||
					plane_mode == kNormalBack)
					shader->setLocalParam(0, 1.0, 0.5, 1.0, plane_trans);
				else
					shader->setLocalParam(0, color.r(), color.g(), color.b(), plane_trans);
				glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, (const GLvoid*)(8 * 4));
			}
			if (border)
			{
				shader->setLocalParam(0, color.r(), color.g(), color.b(), plane_trans);
				glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, (const GLvoid*)(12 * 4));
			}
		}
		//y1 = (p1, p0, p2, p3)
		if (m_clip_mask & 4)
		{
			if (draw_plane)
			{
				if (plane_mode == kNormal ||
					plane_mode == kNormalBack)
					shader->setLocalParam(0, 0.5, 1.0, 0.5, plane_trans);
				else
					shader->setLocalParam(0, color.r(), color.g(), color.b(), plane_trans);
				glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, (const GLvoid*)(16 * 4));
			}
			if (border)
			{
				shader->setLocalParam(0, color.r(), color.g(), color.b(), plane_trans);
				glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, (const GLvoid*)(20 * 4));
			}
		}
		//y2 = (p4, p5, p7, p6)
		if (m_clip_mask & 8)
		{
			if (draw_plane)
			{
				if (plane_mode == kNormal ||
					plane_mode == kNormalBack)
					shader->setLocalParam(0, 1.0, 1.0, 0.5, plane_trans);
				else
					shader->setLocalParam(0, color.r(), color.g(), color.b(), plane_trans);
				glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, (const GLvoid*)(24 * 4));
			}
			if (border)
			{
				shader->setLocalParam(0, color.r(), color.g(), color.b(), plane_trans);
				glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, (const GLvoid*)(28 * 4));
			}
		}
		//z1 = (p0, p4, p6, p2)
		if (m_clip_mask & 16)
		{
			if (draw_plane)
			{
				if (plane_mode == kNormal ||
					plane_mode == kNormalBack)
					shader->setLocalParam(0, 0.5, 0.5, 1.0, plane_trans);
				else
					shader->setLocalParam(0, color.r(), color.g(), color.b(), plane_trans);
				glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, (const GLvoid*)(32 * 4));
			}
			if (border)
			{
				shader->setLocalParam(0, color.r(), color.g(), color.b(), plane_trans);
				glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, (const GLvoid*)(36 * 4));
			}
		}
		//z2 = (p5, p1, p3, p7)
		if (m_clip_mask & 32)
		{
			if (draw_plane)
			{
				if (plane_mode == kNormal ||
					plane_mode == kNormalBack)
					shader->setLocalParam(0, 0.5, 1.0, 1.0, plane_trans);
				else
					shader->setLocalParam(0, color.r(), color.g(), color.b(), plane_trans);
				glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, (const GLvoid*)(40 * 4));
			}
			if (border)
			{
				shader->setLocalParam(0, color.r(), color.g(), color.b(), plane_trans);
				glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, (const GLvoid*)(44 * 4));
			}
		}

		glDisableVertexAttribArray(0);
		//unbind
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	if (shader && shader->valid())
		shader->release();

	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
}

void VRenderGLView::DrawGrid()
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	size_t grid_num = 5;
	size_t line_num = grid_num * 2 + 1;
	size_t i;
	vector<float> vertex;
	vertex.reserve(line_num * 4 * 3);

	double gap = m_distance / grid_num;
	for (i = 0; i<line_num; ++i)
	{
		vertex.push_back(float(-m_distance + gap*i));
		vertex.push_back(float(0.0));
		vertex.push_back(float(-m_distance));
		vertex.push_back(float(-m_distance + gap*i));
		vertex.push_back(float(0.0));
		vertex.push_back(float(m_distance));
	}
	for (i = 0; i<line_num; ++i)
	{
		vertex.push_back(float(-m_distance));
		vertex.push_back(float(0.0));
		vertex.push_back(float(-m_distance + gap*i));
		vertex.push_back(float(m_distance));
		vertex.push_back(float(0.0));
		vertex.push_back(float(-m_distance + gap*i));
	}

	ShaderProgram* shader =
		m_img_shader_factory.shader(IMG_SHDR_DRAW_GEOMETRY);
	if (shader)
	{
		if (!shader->valid())
			shader->create();
		shader->bind();
	}
	Color text_color = GetTextColor();
	shader->setLocalParam(0, text_color.r(), text_color.g(), text_color.b(), 1.0);
	glm::mat4 matrix = m_proj_mat * m_mv_mat;
	shader->setLocalParamMatrix(0, glm::value_ptr(matrix));

	glBindBuffer(GL_ARRAY_BUFFER, m_misc_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertex.size(), &vertex[0], GL_DYNAMIC_DRAW);
	glBindVertexArray(m_misc_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_misc_vbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const GLvoid*)0);
	glDrawArrays(GL_LINES, 0, line_num * 4);
	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	if (shader && shader->valid())
		shader->release();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
}

void VRenderGLView::DrawCamCtr()
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	double len;
	if (m_camctr_size > 0.0)
		len = m_distance*tan(d2r(m_aov / 2.0))*m_camctr_size / 10.0;
	else
		len = fabs(m_camctr_size);

	vector<float> vertex;
	vertex.reserve(6 * 3);

	vertex.push_back(0.0); vertex.push_back(0.0); vertex.push_back(0.0);
	vertex.push_back(len); vertex.push_back(0.0); vertex.push_back(0.0);
	vertex.push_back(0.0); vertex.push_back(0.0); vertex.push_back(0.0);
	vertex.push_back(0.0); vertex.push_back(len); vertex.push_back(0.0);
	vertex.push_back(0.0); vertex.push_back(0.0); vertex.push_back(0.0);
	vertex.push_back(0.0); vertex.push_back(0.0); vertex.push_back(len);

	ShaderProgram* shader =
		m_img_shader_factory.shader(IMG_SHDR_DRAW_GEOMETRY);
	if (shader)
	{
		if (!shader->valid())
			shader->create();
		shader->bind();
	}
	glm::mat4 matrix = m_proj_mat * m_mv_mat;
	shader->setLocalParamMatrix(0, glm::value_ptr(matrix));

	glBindBuffer(GL_ARRAY_BUFFER, m_misc_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertex.size(), &vertex[0], GL_DYNAMIC_DRAW);
	glBindVertexArray(m_misc_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_misc_vbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const GLvoid*)0);

	shader->setLocalParam(0, 1.0, 0.0, 0.0, 1.0);
	glDrawArrays(GL_LINES, 0, 2);
	shader->setLocalParam(0, 0.0, 1.0, 0.0, 1.0);
	glDrawArrays(GL_LINES, 2, 2);
	shader->setLocalParam(0, 0.0, 0.0, 1.0, 1.0);
	glDrawArrays(GL_LINES, 4, 2);

	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	if (shader && shader->valid())
		shader->release();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
}

void VRenderGLView::DrawFrame()
{
	int nx, ny;
	nx = GetGLSize().x;
	ny = GetGLSize().y;
	glm::mat4 proj_mat = glm::ortho(float(0), float(nx), float(0), float(ny));

	glDisable(GL_DEPTH_TEST);
	//GLfloat line_width = 1.0f;

	vector<float> vertex;
	vertex.reserve(4 * 3);

	vertex.push_back(m_frame_x - 1); vertex.push_back(m_frame_y - 1); vertex.push_back(0.0);
	vertex.push_back(m_frame_x + m_frame_w + 1); vertex.push_back(m_frame_y - 1); vertex.push_back(0.0);
	vertex.push_back(m_frame_x + m_frame_w + 1); vertex.push_back(m_frame_y + m_frame_h + 1); vertex.push_back(0.0);
	vertex.push_back(m_frame_x - 1); vertex.push_back(m_frame_y + m_frame_h + 1); vertex.push_back(0.0);

	ShaderProgram* shader =
		m_img_shader_factory.shader(IMG_SHDR_DRAW_GEOMETRY);
	if (shader)
	{
		if (!shader->valid())
			shader->create();
		shader->bind();
	}
	shader->setLocalParam(0, 1.0, 1.0, 0.0, 1.0);
	shader->setLocalParamMatrix(0, glm::value_ptr(proj_mat));

	//draw frame
	glBindBuffer(GL_ARRAY_BUFFER, m_misc_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertex.size(), &vertex[0], GL_DYNAMIC_DRAW);
	glBindVertexArray(m_misc_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_misc_vbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const GLvoid*)0);
	glDrawArrays(GL_LINE_LOOP, 0, 4);
	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	if (shader && shader->valid())
		shader->release();

	glEnable(GL_DEPTH_TEST);
}

void VRenderGLView::DrawScaleBar()
{
	double offset = 0.0;
	if (m_draw_legend)
		offset = m_sb_height;

	int nx = GetGLSize().x;
	int ny = GetGLSize().y;
	float sx, sy;
	sx = 2.0 / nx;
	sy = 2.0 / ny;
	float px, py, ph;

	glm::mat4 proj_mat = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	double len = m_sb_length / (m_ortho_right - m_ortho_left);
	wstring wsb_text = m_sb_text.ToStdWstring();
	double textlen = m_text_renderer ?
		m_text_renderer->RenderTextLen(wsb_text) : 0.0;
	vector<float> vertex;
	vertex.reserve(4 * 3);

	Color text_color = GetTextColor();

	if (m_draw_frame)
	{
		px = (0.95*m_frame_w + m_frame_x) / nx;
		py = (0.05*m_frame_h + m_frame_y + offset) / ny;
		ph = 5.0 / ny;
		vertex.push_back(px); vertex.push_back(py); vertex.push_back(0.0);
		vertex.push_back(px - len); vertex.push_back(py); vertex.push_back(0.0);
		vertex.push_back(px); vertex.push_back(py - ph); vertex.push_back(0.0);
		vertex.push_back(px - len); vertex.push_back(py - ph); vertex.push_back(0.0);

		if (m_disp_scale_bar_text)
		{
			px = 0.95*m_frame_w + m_frame_x - (len*nx + textlen + nx) / 2.0;
			py = ny / 2.0 - ny + 0.065*m_frame_h + m_frame_y + offset;
			if (m_text_renderer)
				m_text_renderer->RenderText(
					wsb_text, text_color,
					px*sx, py*sy, sx, sy);
		}
	}
	else
	{
		px = 0.95;
		py = 0.05 + offset / ny;
		ph = 5.0 / ny;
		vertex.push_back(px); vertex.push_back(py); vertex.push_back(0.0);
		vertex.push_back(px - len); vertex.push_back(py); vertex.push_back(0.0);
		vertex.push_back(px); vertex.push_back(py - ph); vertex.push_back(0.0);
		vertex.push_back(px - len); vertex.push_back(py - ph); vertex.push_back(0.0);

		if (m_disp_scale_bar_text)
		{
			px = 0.95*nx - (len*nx + textlen + nx) / 2.0;
			py = ny / 2.0 - 0.935*ny + offset;
			if (m_text_renderer)
				m_text_renderer->RenderText(
					wsb_text, text_color,
					px*sx, py*sy, sx, sy);
		}
	}

	ShaderProgram* shader =
		m_img_shader_factory.shader(IMG_SHDR_DRAW_GEOMETRY);
	if (shader)
	{
		if (!shader->valid())
			shader->create();
		shader->bind();
	}
	shader->setLocalParamMatrix(0, glm::value_ptr(proj_mat));

	glBindBuffer(GL_ARRAY_BUFFER, m_misc_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertex.size(), &vertex[0], GL_DYNAMIC_DRAW);
	glBindVertexArray(m_misc_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_misc_vbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const GLvoid*)0);

	shader->setLocalParam(0, text_color.r(), text_color.g(), text_color.b(), 1.0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	if (shader && shader->valid())
		shader->release();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
}

void VRenderGLView::DrawLegend()
{
	if (!m_text_renderer)
		return;
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (!vr_frame)
		return;

	double font_height = m_text_renderer->GetSize() + 3.0;

	int nx = GetGLSize().x;
	int ny = GetGLSize().y;

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
			name_len = m_text_renderer->RenderTextLen(wstr) + font_height;
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
			name_len = m_text_renderer->RenderTextLen(wstr) + font_height;
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
			name_len = m_text_renderer->RenderTextLen(wstr) + font_height;
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
			if (vr_frame->GetCurSelType() == 2 &&
				vr_frame->GetCurSelVol() &&
				vr_frame->GetCurSelVol()->GetName() == wxstr)
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
			name_len = m_text_renderer->RenderTextLen(wstr) + font_height;
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
			Color amb, diff, spec;
			double shine, alpha;
			m_md_pop_list[i]->GetMaterial(amb, diff, spec, shine, alpha);
			Color c(diff.r(), diff.g(), diff.b());
			bool highlighted = false;
			if (vr_frame->GetCurSelType() == 3 &&
				vr_frame->GetCurSelMesh() &&
				vr_frame->GetCurSelMesh()->GetName() == wxstr)
				highlighted = true;
			DrawName(xpos + xoffset, ny - (lines - cur_line + 0.1)*font_height - yoffset,
				nx, ny, wxstr, c, font_height, highlighted);
		}
	}

	m_sb_height = (lines + 1)*font_height;
}

void VRenderGLView::DrawName(
	double x, double y, int nx, int ny,
	wxString name, Color color,
	double font_height,
	bool highlighted)
{
	float sx, sy;
	sx = 2.0 / nx;
	sy = 2.0 / ny;

	wstring wstr;

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	glm::mat4 proj_mat = glm::ortho(0.0f, float(nx), 0.0f, float(ny));
	vector<float> vertex;
	vertex.reserve(8 * 3);

	float px1, py1, px2, py2;
	px1 = x + 0.2*font_height;
	py1 = ny - y + 0.2*font_height;
	px2 = x + 0.8*font_height;
	py2 = ny - y + 0.8*font_height;
	vertex.push_back(px1 - 1.0); vertex.push_back(py2 + 1.0); vertex.push_back(0.0);
	vertex.push_back(px2 + 1.0); vertex.push_back(py2 + 1.0); vertex.push_back(0.0);
	vertex.push_back(px1 - 1.0); vertex.push_back(py1 - 1.0); vertex.push_back(0.0);
	vertex.push_back(px2 + 1.0); vertex.push_back(py1 - 1.0); vertex.push_back(0.0);

	vertex.push_back(px1); vertex.push_back(py2); vertex.push_back(0.0);
	vertex.push_back(px2); vertex.push_back(py2); vertex.push_back(0.0);
	vertex.push_back(px1); vertex.push_back(py1); vertex.push_back(0.0);
	vertex.push_back(px2); vertex.push_back(py1); vertex.push_back(0.0);

	ShaderProgram* shader =
		m_img_shader_factory.shader(IMG_SHDR_DRAW_GEOMETRY);
	if (shader)
	{
		if (!shader->valid())
			shader->create();
		shader->bind();
	}
	shader->setLocalParamMatrix(0, glm::value_ptr(proj_mat));

	glBindBuffer(GL_ARRAY_BUFFER, m_misc_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertex.size(), &vertex[0], GL_DYNAMIC_DRAW);
	glBindVertexArray(m_misc_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_misc_vbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const GLvoid*)0);

	Color text_color = GetTextColor();
	shader->setLocalParam(0, text_color.r(), text_color.g(), text_color.b(), 1.0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	shader->setLocalParam(0, color.r(), color.g(), color.b(), 1.0);
	glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);

	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	if (shader && shader->valid())
		shader->release();

	px1 = x + font_height - nx / 2;
	py1 = ny / 2 - y + 0.25*font_height;
	wstr = name.ToStdWstring();
	m_text_renderer->RenderText(
		wstr, text_color,
		px1*sx, py1*sy, sx, sy);
	if (highlighted)
	{
		px1 -= 0.5;
		py1 += 0.5;
		m_text_renderer->RenderText(
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

	Color color1, color2;
	HSVColor hsv_color1(m_bg_color);
	if (hsv_color1.val() > 0.5)
	{
		if (hsv_color1.sat() > 0.3)
		{
			color1 = Color(HSVColor(hsv_color1.hue(),
				hsv_color1.sat() * 0.1,
				Min(hsv_color1.val() + 0.3, 1.0)));
			color2 = Color(HSVColor(hsv_color1.hue(),
				hsv_color1.sat() * 0.3,
				Min(hsv_color1.val() + 0.1, 1.0)));
		}
		else
		{
			color1 = Color(HSVColor(hsv_color1.hue(),
				hsv_color1.sat() * 0.1,
				Max(hsv_color1.val() - 0.5, 0.0)));
			color2 = Color(HSVColor(hsv_color1.hue(),
				hsv_color1.sat() * 0.3,
				Max(hsv_color1.val() - 0.3, 0.0)));
		}
	}
	else
	{
		color1 = Color(HSVColor(hsv_color1.hue(),
			hsv_color1.sat() * 0.1,
			Min(hsv_color1.val() + 0.7, 1.0)));
		color2 = Color(HSVColor(hsv_color1.hue(),
			hsv_color1.sat() * 0.3,
			Min(hsv_color1.val() + 0.5, 1.0)));
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

	ShaderProgram* shader =
		m_img_shader_factory.shader(IMG_SHDR_DRAW_GEOMETRY_COLOR3);
	if (shader)
	{
		if (!shader->valid())
			shader->create();
		shader->bind();
	}
	shader->setLocalParamMatrix(0, glm::value_ptr(proj_mat));

	glBindBuffer(GL_ARRAY_BUFFER, m_misc_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertex.size(), &vertex[0], GL_DYNAMIC_DRAW);
	glBindVertexArray(m_misc_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_misc_vbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (const GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (const GLvoid*)12);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 8);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	if (shader && shader->valid())
		shader->release();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
}

void VRenderGLView::SetColormapColors(int colormap)
{
	switch (colormap)
	{
	case 0://rainbow
		m_color_1 = Color(0.0, 0.0, 1.0);
		m_color_2 = Color(0.0, 0.0, 1.0);
		m_color_3 = Color(0.0, 1.0, 1.0);
		m_color_4 = Color(0.0, 1.0, 0.0);
		m_color_5 = Color(1.0, 1.0, 0.0);
		m_color_6 = Color(1.0, 0.0, 0.0);
		m_color_7 = Color(1.0, 0.0, 0.0);
		break;
	case 1://reverse rainbow
		m_color_1 = Color(1.0, 0.0, 0.0);
		m_color_2 = Color(1.0, 0.0, 0.0);
		m_color_3 = Color(1.0, 1.0, 0.0);
		m_color_4 = Color(0.0, 1.0, 0.0);
		m_color_5 = Color(0.0, 1.0, 1.0);
		m_color_6 = Color(0.0, 0.0, 1.0);
		m_color_7 = Color(0.0, 0.0, 1.0);
		break;
	case 2://hot
		m_color_1 = Color(0.0, 0.0, 0.0);
		m_color_2 = Color(0.0, 0.0, 0.0);
		m_color_3 = Color(0.5, 0.0, 0.0);
		m_color_4 = Color(1.0, 0.0, 0.0);
		m_color_5 = Color(1.0, 1.0, 0.0);
		m_color_6 = Color(1.0, 1.0, 1.0);
		m_color_7 = Color(1.0, 1.0, 1.0);
		break;
	case 3://cool
		m_color_1 = Color(0.0, 1.0, 1.0);
		m_color_2 = Color(0.0, 1.0, 1.0);
		m_color_3 = Color(0.25, 0.75, 1.0);
		m_color_4 = Color(0.5, 0.5, 1.0);
		m_color_5 = Color(0.75, 0.25, 1.0);
		m_color_6 = Color(1.0, 0.0, 1.0);
		m_color_7 = Color(1.0, 0.0, 1.0);
		break;
	case 4://blue-red
		m_color_1 = Color(0.25, 0.3, 0.75);
		m_color_2 = Color(0.25, 0.3, 0.75);
		m_color_3 = Color(0.475, 0.5, 0.725);
		m_color_4 = Color(0.7, 0.7, 0.7);
		m_color_5 = Color(0.7, 0.35, 0.425);
		m_color_6 = Color(0.7, 0.0, 0.15);
		m_color_7 = Color(0.7, 0.0, 0.15);
		break;
	case 5://monochrome
		m_color_1 = Color(0.0, 0.0, 0.0);
		m_color_2 = Color(0.0, 0.0, 0.0);
		m_color_3 = Color(0.25, 0.25, 0.25);
		m_color_4 = Color(0.5, 0.5, 0.5);
		m_color_5 = Color(0.75, 0.75, 0.75);
		m_color_6 = Color(1.0, 1.0, 1.0);
		m_color_7 = Color(1.0, 1.0, 1.0);
		break;
	case 6://reverse mono
		m_color_1 = Color(1.0, 1.0, 1.0);
		m_color_2 = Color(1.0, 1.0, 1.0);
		m_color_3 = Color(0.75, 0.75, 0.75);
		m_color_4 = Color(0.5, 0.5, 0.5);
		m_color_5 = Color(0.25, 0.25, 0.25);
		m_color_6 = Color(0.0, 0.0, 0.0);
		m_color_7 = Color(0.0, 0.0, 0.0);
		break;
	}
}

void VRenderGLView::DrawColormap()
{
	bool draw = false;

	int num = 0;
	int vd_index;
	double max_val = 255.0;
	bool enable_alpha = false;

	for (int i = 0; i<GetDispVolumeNum(); i++)
	{
		VolumeData* vd = GetDispVolumeData(i);
		if (vd && vd->GetColormapMode() && vd->GetDisp())
		{
			num++;
			vd_index = i;
		}
	}

	if (num == 0)
		return;
	else if (num == 1)
	{
		VolumeData* vd_view = GetDispVolumeData(vd_index);
		if (vd_view)
		{
			draw = true;
			double low, high;
			vd_view->GetColormapValues(low, high);
			m_value_2 = low;
			m_value_6 = high;
			m_value_4 = (low + high) / 2.0;
			m_value_3 = (low + m_value_4) / 2.0;
			m_value_5 = (m_value_4 + high) / 2.0;
			max_val = vd_view->GetMaxValue();
			enable_alpha = vd_view->GetEnableAlpha();
			SetColormapColors(vd_view->GetColormap());
		}
	}
	else if (num > 1)
	{
		VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
		if (vr_frame)
		{
			VolumeData* vd = vr_frame->GetCurSelVol();
			if (vd && vd->GetDisp())
			{
				wxString str = vd->GetName();
				VolumeData* vd_view = GetVolumeData(str);
				if (vd_view && vd_view->GetColormapDisp())
				{
					draw = true;
					double low, high;
					vd_view->GetColormapValues(low, high);
					m_value_2 = low;
					m_value_6 = high;
					m_value_4 = (low + high) / 2.0;
					m_value_3 = (low + m_value_4) / 2.0;
					m_value_5 = (m_value_4 + high) / 2.0;
					max_val = vd_view->GetMaxValue();
					enable_alpha = vd_view->GetEnableAlpha();
					SetColormapColors(vd_view->GetColormap());
				}
			}
		}
	}

	if (!draw)
		return;

	double offset = 0.0;
	if (m_draw_legend)
		offset = m_sb_height;

	int nx = GetGLSize().x;
	int ny = GetGLSize().y;
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

		if (m_text_renderer)
		{
			wxString str;
			wstring wstr;

			Color text_color = GetTextColor();

			//value 1
			px = 0.052*m_frame_w + m_frame_x - nx / 2.0;
			py = 0.1*m_frame_h + m_frame_y + offset - ny / 2.0;
			str = wxString::Format("%d", 0);
			wstr = str.ToStdWstring();
			m_text_renderer->RenderText(
				wstr, text_color,
				px*sx, py*sy, sx, sy);
			//value 2
			px = 0.052*m_frame_w + m_frame_x - nx / 2.0;
			py = (0.1 + 0.4*m_value_2)*m_frame_h + m_frame_y + offset - ny / 2.0;
			str = wxString::Format("%d", int(m_value_2*max_val));
			wstr = str.ToStdWstring();
			m_text_renderer->RenderText(
				wstr, text_color,
				px*sx, py*sy, sx, sy);
			//value 4
			px = 0.052*m_frame_w + m_frame_x - nx / 2.0;
			py = (0.1 + 0.4*m_value_4)*m_frame_h + m_frame_y + offset - ny / 2.0;
			str = wxString::Format("%d", int(m_value_4*max_val));
			wstr = str.ToStdWstring();
			m_text_renderer->RenderText(
				wstr, text_color,
				px*sx, py*sy, sx, sy);
			//value 6
			px = 0.052*m_frame_w + m_frame_x - nx / 2.0;
			py = (0.1 + 0.4*m_value_6)*m_frame_h + m_frame_y + offset - ny / 2.0;
			str = wxString::Format("%d", int(m_value_6*max_val));
			wstr = str.ToStdWstring();
			m_text_renderer->RenderText(
				wstr, text_color,
				px*sx, py*sy, sx, sy);
			//value 7
			px = 0.052*m_frame_w + m_frame_x - nx / 2.0;
			py = 0.5*m_frame_h + m_frame_y + offset - ny / 2.0;
			str = wxString::Format("%d", int(max_val));
			wstr = str.ToStdWstring();
			m_text_renderer->RenderText(
				wstr, text_color,
				px*sx, py*sy, sx, sy);
		}
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

		if (m_text_renderer)
		{
			wxString str;
			wstring wstr;

			Color text_color = GetTextColor();

			//value 1
			px = 0.052*nx - nx / 2.0;
			py = ny / 2.0 - 0.9*ny + offset;
			str = wxString::Format("%d", 0);
			wstr = str.ToStdWstring();
			m_text_renderer->RenderText(
				wstr, text_color,
				px*sx, py*sy, sx, sy);
			//value 2
			px = 0.052*nx - nx / 2.0;
			py = ny / 2.0 - (0.9 - 0.4*m_value_2)*ny + offset;
			str = wxString::Format("%d", int(m_value_2*max_val));
			wstr = str.ToStdWstring();
			m_text_renderer->RenderText(
				wstr, text_color,
				px*sx, py*sy, sx, sy);
			//value 4
			px = 0.052*nx - nx / 2.0;
			py = ny / 2.0 - (0.9 - 0.4*m_value_4)*ny + offset;
			str = wxString::Format("%d", int(m_value_4*max_val));
			wstr = str.ToStdWstring();
			m_text_renderer->RenderText(
				wstr, text_color,
				px*sx, py*sy, sx, sy);
			//value 6
			px = 0.052*nx - nx / 2.0;
			py = ny / 2.0 - (0.9 - 0.4*m_value_6)*ny + offset;
			str = wxString::Format("%d", int(m_value_6*max_val));
			wstr = str.ToStdWstring();
			m_text_renderer->RenderText(
				wstr, text_color,
				px*sx, py*sy, sx, sy);
			//value 7
			px = 0.052*nx - nx / 2.0;
			py = ny / 2.0 - 0.5*ny + offset;
			str = wxString::Format("%d", int(max_val));
			wstr = str.ToStdWstring();
			m_text_renderer->RenderText(
				wstr, text_color,
				px*sx, py*sy, sx, sy);
		}
	}

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	ShaderProgram* shader =
		m_img_shader_factory.shader(IMG_SHDR_DRAW_GEOMETRY_COLOR4);
	if (shader)
	{
		if (!shader->valid())
			shader->create();
		shader->bind();
	}
	shader->setLocalParamMatrix(0, glm::value_ptr(proj_mat));

	glBindBuffer(GL_ARRAY_BUFFER, m_misc_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertex.size(), &vertex[0], GL_DYNAMIC_DRAW);
	glBindVertexArray(m_misc_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_misc_vbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (const GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (const GLvoid*)12);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 14);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	if (shader && shader->valid())
		shader->release();

	glEnable(GL_DEPTH_TEST);
}

void VRenderGLView::DrawInfo(int nx, int ny)
{
	if (!m_text_renderer)
		return;

	float sx, sy;
	sx = 2.0 / nx;
	sy = 2.0 / ny;
	float px, py;
	float gapw = m_text_renderer->GetSize();
	float gaph = gapw * 2;

	double fps_ = 1.0 / m_timer->average();
	wxString str;
	Color text_color = GetTextColor();
	if (TextureRenderer::get_mem_swap())
	{
		if (m_use_press)
			str = wxString::Format(
				"Int: %s, FPS: %.2f, Bricks: %d, Quota: %d, Time: %lu, Pressure: %.2f",
				m_interactive ? "Yes" : "No",
				fps_ >= 0.0&&fps_<300.0 ? fps_ : 0.0,
				TextureRenderer::get_finished_bricks(),
				TextureRenderer::get_quota_bricks(),
				TextureRenderer::get_cor_up_time(),
				m_pressure);
		else
			str = wxString::Format(
				"Int: %s, FPS: %.2f, Bricks: %d, Quota: %d, Time: %lu",
				m_interactive ? "Yes" : "No",
				fps_ >= 0.0&&fps_<300.0 ? fps_ : 0.0,
				TextureRenderer::get_finished_bricks(),
				TextureRenderer::get_quota_bricks(),
				TextureRenderer::get_cor_up_time());
		////budget_test
		//if (m_interactive)
		//  tos <<
		//  TextureRenderer::get_quota_bricks()
		//  << "\t" <<
		//  TextureRenderer::get_finished_bricks()
		//  << "\t" <<
		//  TextureRenderer::get_queue_last()
		//  << "\t" <<
		//  int(TextureRenderer::get_finished_bricks()*
		//    TextureRenderer::get_up_time()/
		//    TextureRenderer::get_consumed_time())
		//  << "\n";
	}
	else
	{
		if (m_use_press)
			str = wxString::Format("FPS: %.2f, Pressure: %.2f",
				fps_ >= 0.0&&fps_<300.0 ? fps_ : 0.0, m_pressure);
		else
			str = wxString::Format("FPS: %.2f",
				fps_ >= 0.0&&fps_<300.0 ? fps_ : 0.0);
	}
	wstring wstr_temp = str.ToStdWstring();
	px = gapw - nx / 2;
	py = ny / 2 - gaph / 2;
	m_text_renderer->RenderText(
		wstr_temp, text_color,
		px*sx, py*sy, sx, sy);

	if ((m_draw_info & INFO_T) &&
		(m_draw_info & INFO_X) &&
		(m_draw_info & INFO_Y) &&
		(m_draw_info & INFO_Z))
	{
		Point p;
		wxPoint mouse_pos = ScreenToClient(wxGetMousePosition());
		if ((m_cur_vol && GetPointVolumeBox(p, mouse_pos.x, mouse_pos.y, m_cur_vol)>0.0) ||
			GetPointPlane(p, mouse_pos.x, mouse_pos.y)>0.0)
		{
			str = wxString::Format("T: %d  X: %.2f  Y: %.2f  Z: %.2f",
				m_tseq_cur_num, p.x(), p.y(), p.z());
			wstr_temp = str.ToStdWstring();
			px = gapw - nx / 2;
			py = ny / 2 - gaph;
			m_text_renderer->RenderText(
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
			vector<Plane*> *planes = m_cur_vol->GetVR()->get_planes();
			Plane* plane = (*planes)[4];
			double abcd[4];
			plane->get_copy(abcd);
			int val = fabs(abcd[3] * resz) + 0.499;

			str = wxString::Format("Z: %.2f µm", val * spcz);
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
			m_text_renderer->RenderText(
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
			m_text_renderer->RenderText(
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
					if (m_text_renderer)
						m_text_renderer->RenderText(
							wstr_temp, text_color,
							px*sx, py*sy, sx, sy);
				}
			}
		}
	}
}

Quaternion VRenderGLView::Trackball(int p1x, int p1y, int p2x, int p2y)
{
	Quaternion q;
	Vector a; /* Axis of rotation */
	double phi;  /* how much to rotate about axis */

	if (p1x == p2x && p1y == p2y)
	{
		/* Zero rotation */
		return q;
	}

	if (m_rot_lock)
	{
		if (abs(p2x - p1x)<50 &&
			abs(p2y - p1y)<50)
			return q;
	}

	a = Vector(p1y - p2y, p2x - p1x, 0.0);
	phi = a.length() / 3.0;
	a.normalize();
	Quaternion q_a(a);
	//rotate back to local
	Quaternion q_a2 = (-m_q) * q_a * m_q;
	a = Vector(q_a2.x, q_a2.y, q_a2.z);
	a.normalize();

	q = Quaternion(phi, a);
	q.Normalize();

	if (m_rot_lock)
	{
		double rotx, roty, rotz;
		q.ToEuler(rotx, roty, rotz);
		rotx = int(rotx / 45.0)*45.0;
		roty = int(roty / 45.0)*45.0;
		rotz = int(rotz / 45.0)*45.0;
		q.FromEuler(rotx, roty, rotz);
	}

	return q;
}

Quaternion VRenderGLView::TrackballClip(int p1x, int p1y, int p2x, int p2y)
{
	Quaternion q;
	Vector a; /* Axis of rotation */
	double phi;  /* how much to rotate about axis */

	if (p1x == p2x && p1y == p2y)
	{
		/* Zero rotation */
		return q;
	}

	a = Vector(p2y - p1y, p2x - p1x, 0.0);
	phi = a.length() / 3.0;
	a.normalize();
	Quaternion q_a(a);
	//rotate back to local
	Quaternion q2;
	q2.FromEuler(-m_rotx, m_roty, m_rotz);
	q_a = (q2)* q_a * (-q2);
	q_a = (m_q_cl)* q_a * (-m_q_cl);
	a = Vector(q_a.x, q_a.y, q_a.z);
	a.normalize();

	q = Quaternion(phi, a);
	q.Normalize();

	return q;
}

void VRenderGLView::Q2A()
{
	//view changed, re-sort bricks
	//SetSortBricks();

	m_q.ToEuler(m_rotx, m_roty, m_rotz);

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

	if (m_clip_mode)
	{
		if (m_clip_mode == 1)
			m_q_cl.FromEuler(m_rotx, -m_roty, -m_rotz);

		vector<Plane*> *planes = 0;
		for (int i = 0; i<(int)m_vd_pop_list.size(); i++)
		{
			if (!m_vd_pop_list[i])
				continue;

			double spcx, spcy, spcz;
			int resx, resy, resz;
			m_vd_pop_list[i]->GetSpacings(spcx, spcy, spcz);
			m_vd_pop_list[i]->GetResolution(resx, resy, resz);
			Vector scale;
			if (spcx>0.0 && spcy>0.0 && spcz>0.0)
			{
				scale = Vector(1.0 / resx / spcx, 1.0 / resy / spcy, 1.0 / resz / spcz);
				scale.safe_normalize();
			}
			else
				scale = Vector(1.0, 1.0, 1.0);

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

				Vector trans1(-0.5, -0.5, -0.5);
				Vector trans2(0.5, 0.5, 0.5);

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
}

void VRenderGLView::A2Q()
{
	//view changed, re-sort bricks
	//SetSortBricks();

	m_q.FromEuler(m_rotx, m_roty, m_rotz);

	if (m_clip_mode)
	{
		if (m_clip_mode == 1)
			m_q_cl.FromEuler(m_rotx, -m_roty, -m_rotz);

		for (int i = 0; i<(int)m_vd_pop_list.size(); i++)
		{
			if (!m_vd_pop_list[i])
				continue;

			vector<Plane*> *planes = 0;
			double spcx, spcy, spcz;
			int resx, resy, resz;
			m_vd_pop_list[i]->GetSpacings(spcx, spcy, spcz);
			m_vd_pop_list[i]->GetResolution(resx, resy, resz);
			Vector scale;
			if (spcx>0.0 && spcy>0.0 && spcz>0.0)
			{
				scale = Vector(1.0 / resx / spcx, 1.0 / resy / spcy, 1.0 / resz / spcz);
				scale.safe_normalize();
			}
			else
				scale = Vector(1.0, 1.0, 1.0);

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

				Vector trans1(-0.5, -0.5, -0.5);
				Vector trans2(0.5, 0.5, 0.5);

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
		m_q_cl_zero.FromEuler(m_rotx, -m_roty, -m_rotz);
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
	vector<Plane*> *planes = 0;
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

void VRenderGLView::Run4DScript()
{
	//save currently selected volume
	VolumeData* cur_vd_save = m_cur_vol;
	for (int i = 0; i < (int)m_vd_pop_list.size(); ++i)
	{
		VolumeData* vd = m_vd_pop_list[i];
		if (vd)
			Run4DScript(m_script_file, vd);
	}
	Run4DScript(m_script_file);
	//restore currently selected volume
	m_cur_vol = cur_vd_save;
	m_selector.SetVolume(m_cur_vol);
	m_calculator.SetVolumeA(m_cur_vol);
}

//start loop update
void VRenderGLView::StartLoopUpdate()
{
	////this is for debug_ds, comment when done
	//if (TextureRenderer::get_mem_swap() &&
	//  TextureRenderer::get_start_update_loop() &&
	//  !TextureRenderer::get_done_update_loop())
	//  return;

	if (TextureRenderer::get_mem_swap())
	{
		if (TextureRenderer::active_view_ > 0 &&
			TextureRenderer::active_view_ != m_vrv->m_id)
			return;
		else
			TextureRenderer::active_view_ = m_vrv->m_id;

		int nx = GetGLSize().x;
		int ny = GetGLSize().y;
		//projection
		HandleProjection(nx, ny);
		//Transformation
		HandleCamera();
		glm::mat4 mv_temp = m_mv_mat;
		//translate object
		m_mv_mat = glm::translate(m_mv_mat, glm::vec3(m_obj_transx, m_obj_transy, m_obj_transz));
		//rotate object
		m_mv_mat = glm::rotate(m_mv_mat, float(glm::radians(m_obj_rotx)), glm::vec3(1.0, 0.0, 0.0));
		m_mv_mat = glm::rotate(m_mv_mat, float(glm::radians(m_obj_roty + 180.0)), glm::vec3(0.0, 1.0, 0.0));
		m_mv_mat = glm::rotate(m_mv_mat, float(glm::radians(m_obj_rotz + 180.0)), glm::vec3(0.0, 0.0, 1.0));
		//center object
		m_mv_mat = glm::translate(m_mv_mat, glm::vec3(-m_obj_ctrx, -m_obj_ctry, -m_obj_ctrz));

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
				Texture* tex = vd->GetTexture();
				if (tex)
				{
					Transform *tform = tex->transform();
					double mvmat[16];
					tform->get_trans(mvmat);
					vd->GetVR()->m_mv_mat2 = glm::mat4(
						mvmat[0], mvmat[4], mvmat[8], mvmat[12],
						mvmat[1], mvmat[5], mvmat[9], mvmat[13],
						mvmat[2], mvmat[6], mvmat[10], mvmat[14],
						mvmat[3], mvmat[7], mvmat[11], mvmat[15]);
					vd->GetVR()->m_mv_mat2 = vd->GetVR()->m_mv_mat * vd->GetVR()->m_mv_mat2;

					Ray view_ray = vd->GetVR()->compute_view();
					vector<TextureBrick*> *bricks = 0;
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
							vd->GetTexture()->nlabel() == -1)
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
				Texture* tex = vd->GetTexture();
				if (!tex)
					continue;
				vector<TextureBrick*> *bricks = tex->get_bricks();
				if (!bricks || bricks->size() == 0)
					continue;
				list.push_back(vd);
			}

			vector<VolumeLoaderData> tmp_shade;
			vector<VolumeLoaderData> tmp_shadow;
			for (i = 0; i < list.size(); i++)
			{
				VolumeData* vd = list[i];
				Texture* tex = vd->GetTexture();
				Ray view_ray = vd->GetVR()->compute_view();
				vector<TextureBrick*> *bricks = tex->get_sorted_bricks(view_ray, !m_persp);
				int mode = vd->GetMode() == 1 ? 1 : 0;
				bool shade = (mode == 1 && vd->GetShading());
				bool shadow = vd->GetShadow();
				for (j = 0; j < bricks->size(); j++)
				{
					VolumeLoaderData d;
					TextureBrick* b = (*bricks)[j];
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
			if (TextureRenderer::get_update_order() == 1)
				std::sort(queues.begin(), queues.end(), VolumeLoader::sort_data_dsc);
			else if (TextureRenderer::get_update_order() == 0)
				std::sort(queues.begin(), queues.end(), VolumeLoader::sort_data_asc);

			if (!tmp_shade.empty())
			{
				if (TextureRenderer::get_update_order() == 1)
					std::sort(tmp_shade.begin(), tmp_shade.end(), VolumeLoader::sort_data_dsc);
				else if (TextureRenderer::get_update_order() == 0)
					std::sort(tmp_shade.begin(), tmp_shade.end(), VolumeLoader::sort_data_asc);
				queues.insert(queues.end(), tmp_shade.begin(), tmp_shade.end());
			}
			if (!tmp_shadow.empty())
			{
				if (TextureRenderer::get_update_order() == 1)
				{
					int order = TextureRenderer::get_update_order();
					TextureRenderer::set_update_order(0);
					for (i = 0; i < list.size(); i++)
					{
						Ray view_ray = list[i]->GetVR()->compute_view();
						list[i]->GetTexture()->set_sort_bricks();
						list[i]->GetTexture()->get_sorted_bricks(view_ray, !m_persp); //recalculate brick.d_
						list[i]->GetTexture()->set_sort_bricks();
					}
					TextureRenderer::set_update_order(order);
					std::sort(tmp_shadow.begin(), tmp_shadow.end(), VolumeLoader::sort_data_asc);
				}
				else if (TextureRenderer::get_update_order() == 0)
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
						Texture* tex = vd->GetTexture();
						if (!tex)
							continue;
						Ray view_ray = vd->GetVR()->compute_view();
						vector<TextureBrick*> *bricks = tex->get_sorted_bricks(view_ray, !m_persp);
						if (!bricks || bricks->size() == 0)
							continue;
						int mode = vd->GetMode() == 1 ? 1 : 0;
						bool shade = (mode == 1 && vd->GetShading());
						bool shadow = vd->GetShadow();
						for (j = 0; j<bricks->size(); j++)
						{
							VolumeLoaderData d;
							TextureBrick* b = (*bricks)[j];
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
							if (TextureRenderer::get_update_order() == 1)
							{
								int order = TextureRenderer::get_update_order();
								TextureRenderer::set_update_order(0);
								Ray view_ray = vd->GetVR()->compute_view();
								tex->set_sort_bricks();
								tex->get_sorted_bricks(view_ray, !m_persp); //recalculate brick.d_
								tex->set_sort_bricks();
								TextureRenderer::set_update_order(order);
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
						Texture* tex = vd->GetTexture();
						if (!tex)
							continue;
						Ray view_ray = vd->GetVR()->compute_view();
						vector<TextureBrick*> *bricks = tex->get_sorted_bricks(view_ray, !m_persp);
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
							Texture* tex = vd->GetTexture();
							Ray view_ray = vd->GetVR()->compute_view();
							vector<TextureBrick*> *bricks = tex->get_sorted_bricks(view_ray, !m_persp);
							int mode = vd->GetMode() == 1 ? 1 : 0;
							bool shade = (mode == 1 && vd->GetShading());
							bool shadow = vd->GetShadow();
							for (j = 0; j < bricks->size(); j++)
							{
								VolumeLoaderData d;
								TextureBrick* b = (*bricks)[j];
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
							if (TextureRenderer::get_update_order() == 1)
								std::sort(tmp_q.begin(), tmp_q.end(), VolumeLoader::sort_data_dsc);
							else if (TextureRenderer::get_update_order() == 0)
								std::sort(tmp_q.begin(), tmp_q.end(), VolumeLoader::sort_data_asc);
							queues.insert(queues.end(), tmp_q.begin(), tmp_q.end());
						}
						if (!tmp_shade.empty())
						{
							if (TextureRenderer::get_update_order() == 1)
								std::sort(tmp_shade.begin(), tmp_shade.end(), VolumeLoader::sort_data_dsc);
							else if (TextureRenderer::get_update_order() == 0)
								std::sort(tmp_shade.begin(), tmp_shade.end(), VolumeLoader::sort_data_asc);
							queues.insert(queues.end(), tmp_shade.begin(), tmp_shade.end());
						}
						if (!tmp_shadow.empty())
						{
							if (TextureRenderer::get_update_order() == 1)
							{
								int order = TextureRenderer::get_update_order();
								TextureRenderer::set_update_order(0);
								for (k = 0; k < list.size(); k++)
								{
									Ray view_ray = list[k]->GetVR()->compute_view();
									list[i]->GetTexture()->set_sort_bricks();
									list[i]->GetTexture()->get_sorted_bricks(view_ray, !m_persp); //recalculate brick.d_
									list[i]->GetTexture()->set_sort_bricks();
								}
								TextureRenderer::set_update_order(order);
								std::sort(tmp_shadow.begin(), tmp_shadow.end(), VolumeLoader::sort_data_asc);
							}
							else if (TextureRenderer::get_update_order() == 0)
								std::sort(tmp_shadow.begin(), tmp_shadow.end(), VolumeLoader::sort_data_asc);
							queues.insert(queues.end(), tmp_shadow.begin(), tmp_shadow.end());
						}
					}
					else
					{
						for (j = 0; j < list.size(); j++)
						{
							VolumeData* vd = list[j];
							Texture* tex = vd->GetTexture();
							Ray view_ray = vd->GetVR()->compute_view();
							vector<TextureBrick*> *bricks = tex->get_sorted_bricks(view_ray, !m_persp);
							int mode = vd->GetMode() == 1 ? 1 : 0;
							bool shade = (mode == 1 && vd->GetShading());
							bool shadow = vd->GetShadow();
							for (k = 0; k<bricks->size(); k++)
							{
								VolumeLoaderData d;
								TextureBrick* b = (*bricks)[k];
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
								if (TextureRenderer::get_update_order() == 1)
								{
									int order = TextureRenderer::get_update_order();
									TextureRenderer::set_update_order(0);
									Ray view_ray = vd->GetVR()->compute_view();
									tex->set_sort_bricks();
									tex->get_sorted_bricks(view_ray, !m_persp); //recalculate brick.d_
									tex->set_sort_bricks();
									TextureRenderer::set_update_order(order);
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
			m_loader.SetMemoryLimitByte((long long)TextureRenderer::mainmem_buf_size_ * 1024LL * 1024LL);
			TextureRenderer::set_load_on_main_thread(false);
			m_loader.Run();
		}

		if (total_num > 0)
		{
			TextureRenderer::set_update_loop();
			if (m_draw_type == 1)
				TextureRenderer::set_total_brick_num(total_num);
			else if (m_draw_type == 2)
				TextureRenderer::set_total_brick_num(total_num*(m_peeling_layers+1));
			TextureRenderer::reset_done_current_chan();
		}
	}
}

//halt loop update
void VRenderGLView::HaltLoopUpdate()
{
	if (TextureRenderer::get_mem_swap())
	{
		TextureRenderer::reset_update_loop();
	}
}

//new function to refresh
void VRenderGLView::RefreshGL(int debug_code, bool erase, bool start_loop)
{
	//for debugging refresh events
#ifdef _DEBUG
	std::wostringstream os;
	os << m_vrv->m_id << "\t" <<
		"refresh" << "\t" <<
		debug_code << "\t" <<
		m_interactive << "\n";
	OutputDebugString(os.str().c_str());
#endif
	m_updating = true;
	if (start_loop)
		StartLoopUpdate();
	SetSortBricks();
	m_refresh = true;
	Refresh(erase);
}

double VRenderGLView::GetPointVolume(Point& mp, double mx, double my,
	VolumeData* vd, int mode, bool use_transf, double thresh)
{
	if (!vd)
		return -1.0;
	Texture* tex = vd->GetTexture();
	if (!tex) return -1.0;
	Nrrd* nrrd = tex->get_nrrd(0);
	if (!nrrd) return -1.0;
	void* data = nrrd->data;
	if (!data && vd->GetAllBrickNum()<1) return -1.0;

	int nx = GetGLSize().x;
	int ny = GetGLSize().y;

	if (nx <= 0 || ny <= 0)
		return -1.0;

	//projection
	HandleProjection(nx, ny);
	//Transformation
	HandleCamera();
	glm::mat4 mv_temp;
	//translate object
	mv_temp = glm::translate(m_mv_mat, glm::vec3(m_obj_transx, m_obj_transy, m_obj_transz));
	//rotate object
	mv_temp = glm::rotate(mv_temp, float(glm::radians(m_obj_rotz + 180.0)), glm::vec3(0.0, 0.0, 1.0));
	mv_temp = glm::rotate(mv_temp, float(glm::radians(m_obj_roty + 180.0)), glm::vec3(0.0, 1.0, 0.0));
	mv_temp = glm::rotate(mv_temp, float(glm::radians(m_obj_rotx)), glm::vec3(1.0, 0.0, 0.0));
	//center object
	mv_temp = glm::translate(mv_temp, glm::vec3(-m_obj_ctrx, -m_obj_ctry, -m_obj_ctrz));

	Transform mv;
	Transform p;
	mv.set(glm::value_ptr(mv_temp));
	p.set(glm::value_ptr(m_proj_mat));

	double x, y;
	x = mx * 2.0 / double(nx) - 1.0;
	y = 1.0 - my * 2.0 / double(ny);
	p.invert();
	mv.invert();
	//transform mp1 and mp2 to object space
	Point mp1(x, y, 0.0);
	mp1 = p.transform(mp1);
	mp1 = mv.transform(mp1);
	Point mp2(x, y, 1.0);
	mp2 = p.transform(mp2);
	mp2 = mv.transform(mp2);

	//volume res
	int xx = -1;
	int yy = -1;
	int zz = -1;
	int tmp_xx, tmp_yy, tmp_zz;
	Point nmp;
	double spcx, spcy, spcz;
	vd->GetSpacings(spcx, spcy, spcz);
	int resx, resy, resz;
	vd->GetResolution(resx, resy, resz, vd->GetLevel());
	//volume bounding box
	BBox bbox = vd->GetBounds();
	Vector vv = mp2 - mp1;
	vv.normalize();
	Point hit;
	double max_int = 0.0;
	double alpha = 0.0;
	double value = 0.0;
	vector<Plane*> *planes = 0;
	double mspc = 1.0;
	if (vd->GetSampleRate() > 0.0)
		mspc = sqrt(spcx*spcx + spcy*spcy + spcz*spcz) / vd->GetSampleRate();
	if (vd->GetVR())
		planes = vd->GetVR()->get_planes();
	int counter = 0;//counter to determine if the ray casting has run
	if (bbox.intersect(mp1, vv, hit))
	{
		int brick_id = -1;
		TextureBrick* hit_brick = 0;
		unsigned long long vindex;
		int data_nx, data_ny, data_nz;
		if (vd->isBrxml())
		{
			data_nx = tex->nx();
			data_ny = tex->ny();
			data_nz = tex->nz();
		}

		while (true)
		{
			tmp_xx = int(hit.x() / spcx);
			tmp_yy = int(hit.y() / spcy);
			tmp_zz = int(hit.z() / spcz);
			if (mode == 1 &&
				tmp_xx == xx && tmp_yy == yy && tmp_zz == zz)
			{
				//same, skip
				hit += vv*mspc;
				continue;
			}
			else
			{
				xx = tmp_xx;
				yy = tmp_yy;
				zz = tmp_zz;
			}
			//out of bound, stop
			if (xx<0 || xx>resx ||
				yy<0 || yy>resy ||
				zz<0 || zz>resz)
				break;
			//normalize
			nmp.x(hit.x() / bbox.max().x());
			nmp.y(hit.y() / bbox.max().y());
			nmp.z(hit.z() / bbox.max().z());
			bool inside = true;
			if (planes)
			{
				for (int i = 0; i<6; i++)
					if ((*planes)[i] &&
						(*planes)[i]->eval_point(nmp)<0.0)
					{
						inside = false;
						break;
					}
			}
			if (inside)
			{
				xx = xx == resx ? resx - 1 : xx;
				yy = yy == resy ? resy - 1 : yy;
				zz = zz == resz ? resz - 1 : zz;

				//if it's multiresolution, get brick first
				if (vd->isBrxml())
				{
					vindex = (unsigned long long)data_nx*(unsigned long long)data_ny*
						(unsigned long long)zz + (unsigned long long)data_nx*
						(unsigned long long)yy + (unsigned long long)xx;
					int id = tex->get_brick_id(vindex);
					if (id != brick_id)
					{
						//update hit brick
						hit_brick = tex->get_brick(id);
						brick_id = id;
					}
					if (hit_brick)
					{
						//coords in brick
						int ii, jj, kk;
						ii = xx - hit_brick->ox();
						jj = yy - hit_brick->oy();
						kk = zz - hit_brick->oz();
						if (use_transf)
							value = vd->GetTransferedValue(ii, jj, kk, hit_brick);
						else
							value = vd->GetOriginalValue(ii, jj, kk, hit_brick);
					}
				}
				else
				{
					if (use_transf)
						value = vd->GetTransferedValue(xx, yy, zz);
					else
						value = vd->GetOriginalValue(xx, yy, zz);
				}

				if (mode == 1)
				{
					if (value > max_int)
					{
						mp = Point((xx + 0.5)*spcx, (yy + 0.5)*spcy, (zz + 0.5)*spcz);
						max_int = value;
						counter++;
					}
				}
				else if (mode == 2)
				{
					//accumulate
					if (value > 0.0)
					{
						alpha = 1.0 - pow(Clamp(1.0 - value, 0.0, 1.0), vd->GetSampleRate());
						max_int += alpha*(1.0 - max_int);
						mp = Point((xx + 0.5)*spcx, (yy + 0.5)*spcy, (zz + 0.5)*spcz);
						counter++;
					}
					if (max_int > thresh || max_int >= 1.0)
						break;
				}
			}
			hit += vv*mspc;
		}
	}
	else
		return -1.0;

	if (counter == 0)
		return -1.0;

	if (mode == 1)
	{
		if (max_int > 0.0)
			return (mp - mp1).length();
		else
			return -1.0;
	}
	else if (mode == 2)
	{
		if (max_int > thresh || max_int >= 1.0)
			return (mp - mp1).length();
		else
			return -1.0;
	}
	else
		return -1.0;
}

double VRenderGLView::GetPointVolumeBox(Point &mp, double mx, double my, VolumeData* vd, bool calc_mats)
{
	if (!vd)
		return -1.0;
	int nx = GetGLSize().x;
	int ny = GetGLSize().y;
	if (nx <= 0 || ny <= 0)
		return -1.0;
	vector<Plane*> *planes = vd->GetVR()->get_planes();
	if (planes->size() != 6)
		return -1.0;

	Transform mv;
	Transform p;
	glm::mat4 mv_temp = m_mv_mat;
	Transform *tform = vd->GetTexture()->transform();
	double mvmat[16];
	tform->get_trans(mvmat);

	if (calc_mats)
	{
		//projection
		HandleProjection(nx, ny);
		//Transformation
		HandleCamera();
		//translate object
		mv_temp = glm::translate(m_mv_mat, glm::vec3(m_obj_transx, m_obj_transy, m_obj_transz));
		//rotate object
		mv_temp = glm::rotate(mv_temp, float(glm::radians(m_obj_roty + 180.0)), glm::vec3(0.0, 1.0, 0.0));
		mv_temp = glm::rotate(mv_temp, float(glm::radians(m_obj_rotz + 180.0)), glm::vec3(0.0, 0.0, 1.0));
		mv_temp = glm::rotate(mv_temp, float(glm::radians(m_obj_rotx)), glm::vec3(1.0, 0.0, 0.0));
		//center object
		mv_temp = glm::translate(mv_temp, glm::vec3(-m_obj_ctrx, -m_obj_ctry, -m_obj_ctrz));
	}
	else
		mv_temp = m_mv_mat;

	glm::mat4 mv_mat2 = glm::mat4(
		mvmat[0], mvmat[4], mvmat[8], mvmat[12],
		mvmat[1], mvmat[5], mvmat[9], mvmat[13],
		mvmat[2], mvmat[6], mvmat[10], mvmat[14],
		mvmat[3], mvmat[7], mvmat[11], mvmat[15]);
	mv_temp = mv_temp * mv_mat2;

	mv.set(glm::value_ptr(mv_temp));
	p.set(glm::value_ptr(m_proj_mat));

	double x, y;
	x = mx * 2.0 / double(nx) - 1.0;
	y = 1.0 - my * 2.0 / double(ny);
	p.invert();
	mv.invert();
	//transform mp1 and mp2 to object space
	Point mp1(x, y, 0.0);
	mp1 = p.transform(mp1);
	mp1 = mv.transform(mp1);
	Point mp2(x, y, 1.0);
	mp2 = p.transform(mp2);
	mp2 = mv.transform(mp2);
	Vector ray_d = mp1 - mp2;
	ray_d.normalize();
	Ray ray(mp1, ray_d);
	double mint = -1.0;
	double t;
	//for each plane, calculate the intersection point
	Plane* plane = 0;
	Point pp;//a point on plane
	int i, j;
	bool pp_out;
	for (i = 0; i<6; i++)
	{
		plane = (*planes)[i];
		FLIVR::Vector vec = plane->normal();
		FLIVR::Point pnt = plane->get_point();
		if (ray.planeIntersectParameter(vec, pnt, t))
		{
			pp = ray.parameter(t);

			pp_out = false;
			//determine if the point is inside the box
			for (j = 0; j<6; j++)
			{
				if (j == i)
					continue;
				if ((*planes)[j]->eval_point(pp) < 0)
				{
					pp_out = true;
					break;
				}
			}

			if (!pp_out)
			{
				if (t > mint)
				{
					mp = pp;
					mint = t;
				}
			}
		}
	}

	mp = tform->transform(mp);

	return mint;
}

double VRenderGLView::GetPointVolumeBox2(Point &p1, Point &p2, double mx, double my, VolumeData* vd)
{
	if (!vd)
		return -1.0;
	int nx = GetGLSize().x;
	int ny = GetGLSize().y;
	if (nx <= 0 || ny <= 0)
		return -1.0;
	vector<Plane*> *planes = vd->GetVR()->get_planes();
	if (planes->size() != 6)
		return -1.0;

	//projection
	HandleProjection(nx, ny);
	//Transformation
	HandleCamera();
	glm::mat4 mv_temp;
	//translate object
	mv_temp = glm::translate(m_mv_mat, glm::vec3(m_obj_transx, m_obj_transy, m_obj_transz));
	//rotate object
	mv_temp = glm::rotate(mv_temp, float(glm::radians(m_obj_roty + 180.0)), glm::vec3(0.0, 1.0, 0.0));
	mv_temp = glm::rotate(mv_temp, float(glm::radians(m_obj_rotz + 180.0)), glm::vec3(0.0, 0.0, 1.0));
	mv_temp = glm::rotate(mv_temp, float(glm::radians(m_obj_rotx)), glm::vec3(1.0, 0.0, 0.0));
	//center object
	mv_temp = glm::translate(mv_temp, glm::vec3(-m_obj_ctrx, -m_obj_ctry, -m_obj_ctrz));
	Transform *tform = vd->GetTexture()->transform();
	double mvmat[16];
	tform->get_trans(mvmat);
	glm::mat4 mv_mat2 = glm::mat4(
		mvmat[0], mvmat[4], mvmat[8], mvmat[12],
		mvmat[1], mvmat[5], mvmat[9], mvmat[13],
		mvmat[2], mvmat[6], mvmat[10], mvmat[14],
		mvmat[3], mvmat[7], mvmat[11], mvmat[15]);
	mv_temp = mv_temp * mv_mat2;

	Transform mv;
	Transform p;
	mv.set(glm::value_ptr(mv_temp));
	p.set(glm::value_ptr(m_proj_mat));

	double x, y;
	x = mx * 2.0 / double(nx) - 1.0;
	y = 1.0 - my * 2.0 / double(ny);
	p.invert();
	mv.invert();
	//transform mp1 and mp2 to object space
	Point mp1(x, y, 0.0);
	mp1 = p.transform(mp1);
	mp1 = mv.transform(mp1);
	Point mp2(x, y, 1.0);
	mp2 = p.transform(mp2);
	mp2 = mv.transform(mp2);
	Vector ray_d = mp1 - mp2;
	ray_d.normalize();
	Ray ray(mp1, ray_d);
	double mint = -1.0;
	double maxt = std::numeric_limits<double>::max();
	double t;
	//for each plane, calculate the intersection point
	Plane* plane = 0;
	Point pp;//a point on plane
	int i, j;
	bool pp_out;
	for (i = 0; i<6; i++)
	{
		plane = (*planes)[i];
		FLIVR::Vector vec = plane->normal();
		FLIVR::Point pnt = plane->get_point();
		if (ray.planeIntersectParameter(vec, pnt, t))
		{
			pp = ray.parameter(t);

			pp_out = false;
			//determine if the point is inside the box
			for (j = 0; j<6; j++)
			{
				if (j == i)
					continue;
				if ((*planes)[j]->eval_point(pp) < 0)
				{
					pp_out = true;
					break;
				}
			}

			if (!pp_out)
			{
				if (t > mint)
				{
					p1 = pp;
					mint = t;
				}
				if (t < maxt)
				{
					p2 = pp;
					maxt = t;
				}
			}
		}
	}

	p1 = tform->transform(p1);
	p2 = tform->transform(p2);

	return mint;
}

double VRenderGLView::GetPointPlane(Point &mp, double mx, double my, Point* planep, bool calc_mats)
{
	int nx = GetGLSize().x;
	int ny = GetGLSize().y;

	if (nx <= 0 || ny <= 0)
		return -1.0;

	glm::mat4 mv_temp;

	if (calc_mats)
	{
		//projection
		HandleProjection(nx, ny);
		//Transformation
		HandleCamera();
		//translate object
		mv_temp = glm::translate(m_mv_mat, glm::vec3(m_obj_transx, m_obj_transy, m_obj_transz));
		//rotate object
		mv_temp = glm::rotate(mv_temp, float(glm::radians(m_obj_roty + 180.0)), glm::vec3(0.0, 1.0, 0.0));
		mv_temp = glm::rotate(mv_temp, float(glm::radians(m_obj_rotz + 180.0)), glm::vec3(0.0, 0.0, 1.0));
		mv_temp = glm::rotate(mv_temp, float(glm::radians(m_obj_rotx)), glm::vec3(1.0, 0.0, 0.0));
		//center object
		mv_temp = glm::translate(mv_temp, glm::vec3(-m_obj_ctrx, -m_obj_ctry, -m_obj_ctrz));
	}
	else
		mv_temp = m_mv_mat;

	Transform mv;
	Transform p;
	mv.set(glm::value_ptr(mv_temp));
	p.set(glm::value_ptr(m_proj_mat));

	Vector n(0.0, 0.0, 1.0);
	Point center(0.0, 0.0, -m_distance);
	if (planep)
	{
		center = *planep;
		center = mv.transform(center);
	}
	double x, y;
	x = mx * 2.0 / double(nx) - 1.0;
	y = 1.0 - my * 2.0 / double(ny);
	p.invert();
	mv.invert();
	//transform mp1 and mp2 to eye space
	Point mp1(x, y, 0.0);
	mp1 = p.transform(mp1);
	Point mp2(x, y, 1.0);
	mp2 = p.transform(mp2);
	FLIVR::Vector vec = mp2 - mp1;
	Ray ray(mp1, vec);
	double t = 0.0;
	if (ray.planeIntersectParameter(n, center, t))
		mp = ray.parameter(t);
	//transform mp to world space
	mp = mv.transform(mp);

	return (mp - mp1).length();
}

Point* VRenderGLView::GetEditingRulerPoint(double mx, double my)
{
	Point* point = 0;

	int nx = GetGLSize().x;
	int ny = GetGLSize().y;

	if (nx <= 0 || ny <= 0)
		return 0;

	double x, y;
	x = mx * 2.0 / double(nx) - 1.0;
	y = 1.0 - my * 2.0 / double(ny);
	double aspect = (double)nx / (double)ny;

	//projection
	HandleProjection(nx, ny);
	//Transformation
	HandleCamera();
	glm::mat4 mv_temp;
	//translate object
	mv_temp = glm::translate(m_mv_mat, glm::vec3(m_obj_transx, m_obj_transy, m_obj_transz));
	//rotate object
	mv_temp = glm::rotate(mv_temp, float(glm::radians(m_obj_roty + 180.0)), glm::vec3(0.0, 1.0, 0.0));
	mv_temp = glm::rotate(mv_temp, float(glm::radians(m_obj_rotz + 180.0)), glm::vec3(0.0, 0.0, 1.0));
	mv_temp = glm::rotate(mv_temp, float(glm::radians(m_obj_rotx)), glm::vec3(1.0, 0.0, 0.0));
	//center object
	mv_temp = glm::translate(mv_temp, glm::vec3(-m_obj_ctrx, -m_obj_ctry, -m_obj_ctrz));

	Transform mv;
	Transform p;
	mv.set(glm::value_ptr(mv_temp));
	p.set(glm::value_ptr(m_proj_mat));

	int i, j;
	Point ptemp;
	bool found = false;
	for (i = 0; i<(int)m_ruler_list.size() && !found; i++)
	{
		Ruler* ruler = m_ruler_list[i];
		if (!ruler) continue;

		for (j = 0; j<ruler->GetNumPoint(); j++)
		{
			point = ruler->GetPoint(j);
			if (!point) continue;
			ptemp = *point;
			ptemp = mv.transform(ptemp);
			ptemp = p.transform(ptemp);
			if ((m_persp && (ptemp.z() <= 0.0 || ptemp.z() >= 1.0)) ||
				(!m_persp && (ptemp.z() >= 0.0 || ptemp.z() <= -1.0)))
				continue;
			if (x<ptemp.x() + 0.01 / aspect &&
				x>ptemp.x() - 0.01 / aspect &&
				y<ptemp.y() + 0.01 &&
				y>ptemp.y() - 0.01)
			{
				found = true;
				break;
			}
		}
	}

	if (found)
		return point;
	else
		return 0;
}

int VRenderGLView::GetRulerType()
{
	return m_ruler_type;
}

void VRenderGLView::SetRulerType(int type)
{
	m_ruler_type = type;
}

void VRenderGLView::FinishRuler()
{
	size_t size = m_ruler_list.size();
	if (!size) return;
	if (m_ruler_list[size - 1] &&
		m_ruler_list[size - 1]->GetRulerType() == 1)
		m_ruler_list[size - 1]->SetFinished();
}

bool VRenderGLView::GetRulerFinished()
{
	size_t size = m_ruler_list.size();
	if (!size) return true;
	if (m_ruler_list[size - 1])
		return m_ruler_list[size - 1]->GetFinished();
	else
		return true;
}

void VRenderGLView::AddRulerPoint(int mx, int my)
{
	if (m_ruler_type == 3)
	{
		Point p1, p2;
		Ruler* ruler = new Ruler();
		ruler->SetRulerType(m_ruler_type);
		GetPointVolumeBox2(p1, p2, mx, my, m_cur_vol);
		ruler->AddPoint(p1);
		ruler->AddPoint(p2);
		ruler->SetTimeDep(m_ruler_time_dep);
		ruler->SetTime(m_tseq_cur_num);
		m_ruler_list.push_back(ruler);
		//store brush size in ruler
		if (m_brush_size_data)
			ruler->SetBrushSize(m_brush_radius1);
		else
			ruler->SetBrushSize(m_brush_radius1 / Get121ScaleFactor());
	}
	else
	{
		Point p;
		if (m_point_volume_mode)
		{
			double t = GetPointVolume(p, mx, my, m_cur_vol,
				m_point_volume_mode, m_ruler_use_transf);
			if (t <= 0.0)
			{
				t = GetPointPlane(p, mx, my);
				if (t <= 0.0)
					return;
			}
		}
		else
		{
			double t = GetPointPlane(p, mx, my);
			if (t <= 0.0)
				return;
		}

		bool new_ruler = true;
		if (m_ruler_list.size())
		{
			Ruler* ruler = m_ruler_list[m_ruler_list.size() - 1];
			if (ruler && !ruler->GetFinished())
			{
				ruler->AddPoint(p);
				new_ruler = false;
			}
		}
		if (new_ruler)
		{
			Ruler* ruler = new Ruler();
			ruler->SetRulerType(m_ruler_type);
			ruler->AddPoint(p);
			ruler->SetTimeDep(m_ruler_time_dep);
			ruler->SetTime(m_tseq_cur_num);
			m_ruler_list.push_back(ruler);
		}
	}

	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (m_vrv && vr_frame && vr_frame->GetMeasureDlg())
		vr_frame->GetMeasureDlg()->GetSettings(m_vrv);
}

void VRenderGLView::AddPaintRulerPoint()
{
	if (m_selector.ProcessSel(0.01))
	{
		wxString str;
		Point center;
		double size;
		m_selector.GetCenter(center);
		m_selector.GetSize(size);

		bool new_ruler = true;
		if (m_ruler_list.size())
		{
			Ruler* ruler = m_ruler_list[m_ruler_list.size() - 1];
			if (ruler && !ruler->GetFinished())
			{
				ruler->AddPoint(center);
				str = wxString::Format("\tv%d", ruler->GetNumPoint() - 1);
				ruler->AddInfoNames(str);
				str = wxString::Format("\t%.0f", size);
				ruler->AddInfoValues(str);
				new_ruler = false;
			}
		}
		if (new_ruler)
		{
			Ruler* ruler = new Ruler();
			ruler->SetRulerType(m_ruler_type);
			ruler->AddPoint(center);
			ruler->SetTimeDep(m_ruler_time_dep);
			ruler->SetTime(m_tseq_cur_num);
			str = "v0";
			ruler->AddInfoNames(str);
			str = wxString::Format("%.0f", size);
			ruler->AddInfoValues(str);
			m_ruler_list.push_back(ruler);
		}

		VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
		if (m_vrv && vr_frame && vr_frame->GetMeasureDlg())
			vr_frame->GetMeasureDlg()->GetSettings(m_vrv);
	}
}

void VRenderGLView::DrawRulers()
{
	if (!m_text_renderer)
		return;

	int nx = GetGLSize().x;
	int ny = GetGLSize().y;
	float sx, sy;
	sx = 2.0 / nx;
	sy = 2.0 / ny;
	float w = m_text_renderer->GetSize() / 4.0f;
	float px, py, p2x, p2y;

	vector<float> verts;
	int vert_num = 0;
	for (size_t i = 0; i<m_ruler_list.size(); ++i)
		if (m_ruler_list[i])
			vert_num += m_ruler_list[i]->GetNumPoint();
	verts.reserve(vert_num * 10 * 3);

	Transform mv;
	Transform p;
	mv.set(glm::value_ptr(m_mv_mat));
	p.set(glm::value_ptr(m_proj_mat));
	Point p1, p2;
	unsigned int num;
	vector<unsigned int> nums;
	Color color;
	Color text_color = GetTextColor();

	for (size_t i = 0; i<m_ruler_list.size(); i++)
	{
		Ruler* ruler = m_ruler_list[i];
		if (!ruler) continue;
		if (!ruler->GetTimeDep() ||
			(ruler->GetTimeDep() &&
				ruler->GetTime() == m_tseq_cur_num))
		{
			num = 0;
			if (ruler->GetUseColor())
				color = ruler->GetColor();
			else
				color = text_color;
			for (size_t j = 0; j<ruler->GetNumPoint(); ++j)
			{
				p2 = *(ruler->GetPoint(j));
				p2 = mv.transform(p2);
				p2 = p.transform(p2);
				if ((m_persp && (p2.z() <= 0.0 || p2.z() >= 1.0)) ||
					(!m_persp && (p2.z() >= 0.0 || p2.z() <= -1.0)))
					continue;
				px = (p2.x() + 1.0)*nx / 2.0;
				py = (p2.y() + 1.0)*ny / 2.0;
				verts.push_back(px - w); verts.push_back(py - w); verts.push_back(0.0);
				verts.push_back(px + w); verts.push_back(py - w); verts.push_back(0.0);
				verts.push_back(px + w); verts.push_back(py - w); verts.push_back(0.0);
				verts.push_back(px + w); verts.push_back(py + w); verts.push_back(0.0);
				verts.push_back(px + w); verts.push_back(py + w); verts.push_back(0.0);
				verts.push_back(px - w); verts.push_back(py + w); verts.push_back(0.0);
				verts.push_back(px - w); verts.push_back(py + w); verts.push_back(0.0);
				verts.push_back(px - w); verts.push_back(py - w); verts.push_back(0.0);
				num += 8;
				if (j + 1 == ruler->GetNumPoint())
				{
					p2x = p2.x()*nx / 2.0;
					p2y = p2.y()*ny / 2.0;
					m_text_renderer->RenderText(
						ruler->GetName().ToStdWstring(),
						color,
						(p2x + w)*sx, (p2y + w)*sy, sx, sy);
				}
				if (j > 0)
				{
					p1 = *(ruler->GetPoint(j - 1));
					p1 = mv.transform(p1);
					p1 = p.transform(p1);
					if ((m_persp && (p1.z() <= 0.0 || p1.z() >= 1.0)) ||
						(!m_persp && (p1.z() >= 0.0 || p1.z() <= -1.0)))
						continue;
					verts.push_back(px); verts.push_back(py); verts.push_back(0.0);
					px = (p1.x() + 1.0)*nx / 2.0;
					py = (p1.y() + 1.0)*ny / 2.0;
					verts.push_back(px); verts.push_back(py); verts.push_back(0.0);
					num += 2;
				}
			}
			if (ruler->GetRulerType() == 4 &&
				ruler->GetNumPoint() >= 3)
			{
				Point center = *(ruler->GetPoint(1));
				Vector v1 = *(ruler->GetPoint(0)) - center;
				Vector v2 = *(ruler->GetPoint(2)) - center;
				double len = Min(v1.length(), v2.length());
				if (len > w)
				{
					v1.normalize();
					v2.normalize();
					p1 = center + v1*w;
					p1 = mv.transform(p1);
					p1 = p.transform(p1);
					px = (p1.x() + 1.0)*nx / 2.0;
					py = (p1.y() + 1.0)*ny / 2.0;
					verts.push_back(px); verts.push_back(py); verts.push_back(0.0);
					p1 = center + v2*w;
					p1 = mv.transform(p1);
					p1 = p.transform(p1);
					px = (p1.x() + 1.0)*nx / 2.0;
					py = (p1.y() + 1.0)*ny / 2.0;
					verts.push_back(px); verts.push_back(py); verts.push_back(0.0);
					num += 2;
				}
			}
			nums.push_back(num);
		}
	}

	if (!verts.empty())
	{
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

		glm::mat4 matrix = glm::ortho(float(0), float(nx), float(0), float(ny));

		ShaderProgram* shader =
			m_img_shader_factory.shader(IMG_SHDR_DRAW_GEOMETRY);
		if (shader)
		{
			if (!shader->valid())
				shader->create();
			shader->bind();
		}
		shader->setLocalParamMatrix(0, glm::value_ptr(matrix));

		glBindBuffer(GL_ARRAY_BUFFER, m_misc_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*verts.size(), &verts[0], GL_DYNAMIC_DRAW);
		glBindVertexArray(m_misc_vao);
		glBindBuffer(GL_ARRAY_BUFFER, m_misc_vbo);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const GLvoid*)0);

		GLint pos = 0;
		size_t j = 0;
		for (size_t i = 0; i<m_ruler_list.size(); i++)
		{
			Ruler* ruler = m_ruler_list[i];
			if (!ruler) continue;
			if (!ruler->GetTimeDep() ||
				(ruler->GetTimeDep() &&
					ruler->GetTime() == m_tseq_cur_num))
			{
				num = 0;
				if (ruler->GetUseColor())
					color = ruler->GetColor();
				else
					color = text_color;
				shader->setLocalParam(0, color.r(), color.g(), color.b(), 1.0);
				glDrawArrays(GL_LINES, pos, (GLsizei)(nums[j++]));
				pos += nums[j - 1];
			}
		}

		glDisableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		if (shader && shader->valid())
			shader->release();

		glDisable(GL_LINE_SMOOTH);
	}
}

vector<Ruler*>* VRenderGLView::GetRulerList()
{
	return &m_ruler_list;
}

Ruler* VRenderGLView::GetRuler(unsigned int id)
{
	for (size_t i = 0; i < m_ruler_list.size(); ++i)
	{
		if (m_ruler_list[i] && m_ruler_list[i]->Id() == id)
			return m_ruler_list[i];
	}
	return 0;
}

int VRenderGLView::RulerProfile(int index)
{
	if (index < 0 ||
		index >= m_ruler_list.size() ||
		!m_cur_vol)
		return 0;

	Ruler* ruler = m_ruler_list[index];
	if (ruler->GetNumPoint() < 1)
		return 0;

	VolumeData* vd = m_cur_vol;
	double spcx, spcy, spcz;
	vd->GetSpacings(spcx, spcy, spcz);
	int nx, ny, nz;
	vd->GetResolution(nx, ny, nz);
	if (spcx <= 0.0 || spcy <= 0.0 || spcz <= 0.0 ||
		nx <= 0 || ny <= 0 || nz <= 0)
		return 0;
	//get data
	vd->GetVR()->return_mask();
	Texture* tex = vd->GetTexture();
	if (!tex) return 0;
	Nrrd* nrrd_data = tex->get_nrrd(0);
	if (!nrrd_data) return 0;
	void* data = nrrd_data->data;
	if (!data) return 0;
	//mask
	Nrrd* nrrd_mask = tex->get_nrrd(tex->nmask());
	void* mask = 0;
	if (nrrd_mask)
		mask = nrrd_mask->data;
	double scale = vd->GetScalarScale();

	if (ruler->GetRulerType() == 3 && mask)
	{
		if (ruler->GetNumPoint() < 1)
			return 0;
		Point p1, p2;
		p1 = *(ruler->GetPoint(0));
		p2 = *(ruler->GetPoint(1));
		//object space
		p1 = Point(p1.x() / spcx, p1.y() / spcy, p1.z() / spcz);
		p2 = Point(p2.x() / spcx, p2.y() / spcy, p2.z() / spcz);
		Vector dir = p2 - p1;
		double dist = dir.length();
		if (dist < EPS)
			return 0;
		dir.normalize();

		//bin number
		int bins = int(dist / 1 + 0.5);
		if (bins <= 0) return 0;
		double bin_dist = dist / bins;
		vector<ProfileBin>* profile = ruler->GetProfile();
		if (!profile) return 0;
		profile->clear();
		profile->reserve(size_t(bins));
		for (unsigned int b = 0; b<bins; ++b)
			profile->push_back(ProfileBin());

		double brush_radius = ruler->GetBrushSize()+1.0;

		int i, j, k;
		long long vol_index;
		//go through data
		for (i = 0; i<nx; ++i)
		for (j = 0; j<ny; ++j)
		for (k = 0; k<nz; ++k)
		{
			vol_index = (long long)nx*ny*k + nx*j + i;
			unsigned char mask_value = ((unsigned char*)mask)[vol_index];
			if (mask_value)
			{
				//find bin
				Point p(i, j, k);
				Vector pdir = p - p1;
				double proj = Dot(pdir, dir);
				int bin_num = int(proj / bin_dist);
				if (bin_num<0 || bin_num >= bins)
					continue;
				//make sure it's within the brush radius
				Point p_ruler = p1 + proj * dir;
				if ((p_ruler - p).length() > brush_radius)
					continue;

				double intensity = 0.0;
				if (nrrd_data->type == nrrdTypeUChar)
					intensity = double(((unsigned char*)data)[vol_index]) / 255.0;
				else if (nrrd_data->type == nrrdTypeUShort)
					intensity = double(((unsigned short*)data)[vol_index]) * scale / 65535.0;

				(*profile)[bin_num].m_pixels++;
				(*profile)[bin_num].m_accum += intensity;
			}
		}
	}
	else
	{
		//calculate length in object space
		double total_length = ruler->GetLengthObject(spcx, spcy, spcz);
		int bins = int(total_length);
		vector<ProfileBin>* profile = ruler->GetProfile();
		if (!profile) return 0;
		profile->clear();

		//sample data through ruler
		int i, j, k;
		long long vol_index;
		Point p;
		double intensity;
		if (bins == 0)
		{
			//allocate
			profile->reserve(size_t(1));
			profile->push_back(ProfileBin());

			p = *(ruler->GetPoint(0));
			//object space
			p = Point(p.x() / spcx, p.y() / spcy, p.z() / spcz);
			intensity = 0.0;
			i = int(p.x() + 0.5);
			j = int(p.y() + 0.5);
			k = int(p.z() + 0.5);
			if (i >= 0 && i <= nx && j >= 0 && j <= ny && k >= 0 && k <= nz)
			{
				if (i == nx) i = nx - 1;
				if (j == ny) j = ny - 1;
				if (k == nz) k = nz - 1;
				vol_index = (long long)nx*ny*k + nx*j + i;
				if (nrrd_data->type == nrrdTypeUChar)
					intensity = double(((unsigned char*)data)[vol_index]) / 255.0;
				else if (nrrd_data->type == nrrdTypeUShort)
					intensity = double(((unsigned short*)data)[vol_index]) * scale / 65535.0;
			}
			(*profile)[0].m_pixels++;
			(*profile)[0].m_accum += intensity;
		}
		else
		{
			//allocate
			profile->reserve(size_t(bins));
			for (unsigned int b = 0; b<bins; ++b)
				profile->push_back(ProfileBin());

			Point p1, p2;
			Vector dir;
			double dist;
			int total_dist = 0;
			for (unsigned int pn = 0; pn<ruler->GetNumPoint() - 1; ++pn)
			{
				p1 = *(ruler->GetPoint(pn));
				p2 = *(ruler->GetPoint(pn + 1));
				//object space
				p1 = Point(p1.x() / spcx, p1.y() / spcy, p1.z() / spcz);
				p2 = Point(p2.x() / spcx, p2.y() / spcy, p2.z() / spcz);
				dir = p2 - p1;
				dist = dir.length();
				dir.normalize();

				for (unsigned int dn = 0; dn<(unsigned int)(dist + 0.5); ++dn)
				{
					p = p1 + dir * double(dn);
					intensity = 0.0;
					i = int(p.x() + 0.5);
					j = int(p.y() + 0.5);
					k = int(p.z() + 0.5);
					if (i >= 0 && i <= nx && j >= 0 && j <= ny && k >= 0 && k <= nz)
					{
						if (i == nx) i = nx - 1;
						if (j == ny) j = ny - 1;
						if (k == nz) k = nz - 1;
						vol_index = (long long)nx*ny*k + nx*j + i;
						if (nrrd_data->type == nrrdTypeUChar)
							intensity = double(((unsigned char*)data)[vol_index]) / 255.0;
						else if (nrrd_data->type == nrrdTypeUShort)
							intensity = double(((unsigned short*)data)[vol_index]) * scale / 65535.0;
					}
					if (total_dist >= bins) break;
					(*profile)[total_dist].m_pixels++;
					(*profile)[total_dist].m_accum += intensity;
					total_dist++;
				}
			}
			if (total_dist < bins)
				profile->erase(profile->begin() + total_dist, profile->begin() + bins - 1);
		}
	}
	wxString str("Profile of volume ");
	str = str + vd->GetName();
	ruler->SetInfoProfile(str);
	return 1;
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
	if (m_cur_vol && m_trace_group && m_text_renderer)
	{
		vector<float> verts;
		unsigned int num = m_trace_group->Draw(verts);

		if (!verts.empty())
		{
			glEnable(GL_LINE_SMOOTH);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

			double spcx, spcy, spcz;
			m_cur_vol->GetSpacings(spcx, spcy, spcz);
			glm::mat4 matrix = glm::scale(m_mv_mat,
				glm::vec3(float(spcx), float(spcy), float(spcz)));
			matrix = m_proj_mat*matrix;

			ShaderProgram* shader =
				m_img_shader_factory.shader(IMG_SHDR_DRAW_GEOMETRY_COLOR3);
			if (shader)
			{
				if (!shader->valid())
					shader->create();
				shader->bind();
			}
			shader->setLocalParamMatrix(0, glm::value_ptr(matrix));

			glBindBuffer(GL_ARRAY_BUFFER, m_misc_vbo);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float)*verts.size(), &verts[0], GL_DYNAMIC_DRAW);
			glBindVertexArray(m_misc_vao);
			glBindBuffer(GL_ARRAY_BUFFER, m_misc_vbo);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (const GLvoid*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (const GLvoid*)12);

			glDrawArrays(GL_LINES, 0, num);

			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);

			if (shader && shader->valid())
				shader->release();

			glDisable(GL_LINE_SMOOTH);
		}
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
	Nrrd* label_nrrd = m_cur_vol->GetLabel(false);
	if (!label_nrrd) return;
	unsigned char* mask_data = (unsigned char*)(mask_nrrd->data);
	if (!mask_data) return;
	unsigned int* label_data = (unsigned int*)(label_nrrd->data);
	if (!label_data) return;
	FL::CellList sel_labels;
	FL::CellListIter label_iter;
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
						FL::pCell cell(new FL::Cell(label_value));
						cell->Inc(ii, jj, kk, 1.0f);
						sel_labels.insert(pair<unsigned int, FL::pCell>
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

	//add traces to trace dialog
	if (update)
	{
		VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
		if (m_vrv && vr_frame && vr_frame->GetTraceDlg())
			vr_frame->GetTraceDlg()->GetSettings(m_vrv);
	}
}

#ifdef _WIN32
WXLRESULT VRenderGLView::MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam)
{
	if (m_use_press)
	{
		if (message == WT_PACKET)
		{
			PACKET pkt;
			if (gpWTPacket((HCTX)lParam, wParam, &pkt))
			{
				//compute pressure, normalized to [0, 1]
				if (m_press_nmax > 1.0)
					m_pressure = pkt.pkNormalPressure / m_press_nmax;
				if (m_pressure > m_press_peak)
					m_press_peak = m_pressure;
				//wheel of air brush
				if (m_press_tmax > 1.0)
					m_air_press = pkt.pkTangentPressure / m_press_tmax;
			}
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
	if (window &&
		(window->GetClassInfo()->
			IsKindOf(CLASSINFO(wxTextCtrl)) ||
			window->GetClassInfo()->
			IsKindOf(CLASSINFO(wxComboBox))) &&
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

	//mouse button down operations
	if (event.LeftDown())
	{
		if (m_int_mode == 6)
			m_editing_ruler_point = GetEditingRulerPoint(event.GetX(), event.GetY());

		if (m_int_mode == 1 ||
			(m_int_mode == 5 &&
				event.AltDown()) ||
				(m_int_mode == 6 &&
					m_editing_ruler_point == 0))
		{
			old_mouse_X = event.GetX();
			old_mouse_Y = event.GetY();
			m_pick = true;
		}
		else if (m_int_mode == 2 || m_int_mode == 7)
		{
			old_mouse_X = event.GetX();
			old_mouse_Y = event.GetY();
			prv_mouse_X = old_mouse_X;
			prv_mouse_Y = old_mouse_Y;
			m_paint_enable = true;
			m_clear_paint = true;
			m_press_peak = 0.0;
			RefreshGL(26);
		}
		return;
	}
	if (event.RightDown())
	{
		old_mouse_X = event.GetX();
		old_mouse_Y = event.GetY();
		return;
	}
	if (event.MiddleDown())
	{
		old_mouse_X = event.GetX();
		old_mouse_Y = event.GetY();
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
				return;
			}
			else
			{
				//				RefreshGL(27);
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
			AddRulerPoint(event.GetX(), event.GetY());
			RefreshGL(27);
			return;
		}
		else if (m_int_mode == 6)
		{
			m_editing_ruler_point = 0;
		}
		else if (m_int_mode == 7)
		{
			//segment volume, calculate center, add ruler point
			m_paint_enable = true;
			Segment();
			if (m_ruler_type == 3)
				AddRulerPoint(event.GetX(), event.GetY());
			else
				AddPaintRulerPoint();
			m_int_mode = 8;
			m_force_clear = true;
			RefreshGL(27);
			return;
		}
	}
	if (event.MiddleUp())
	{
		//SetSortBricks();
		//		RefreshGL(28);
		return;
	}
	if (event.RightUp())
	{
		if (m_int_mode == 1)
		{
			//			RefreshGL(27);
			//			return;
		}
		if (m_int_mode == 5 &&
			!event.AltDown())
		{
			if (GetRulerFinished())
			{
				SetIntMode(1);
				VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
				if (m_vrv && vr_frame && vr_frame->GetMeasureDlg())
					vr_frame->GetMeasureDlg()->GetSettings(m_vrv);
			}
			else
			{
				AddRulerPoint(event.GetX(), event.GetY());
				FinishRuler();
			}
			RefreshGL(29);
			return;
		}
		//SetSortBricks();
	}

	//mouse dragging
	if (event.Dragging())
	{
		TextureRenderer::set_cor_up_time(
			int(sqrt(double(old_mouse_X - event.GetX())*
				double(old_mouse_X - event.GetX()) +
				double(old_mouse_Y - event.GetY())*
				double(old_mouse_Y - event.GetY()))));

		bool hold_old = false;
		if (m_int_mode == 1 ||
			(m_int_mode == 5 &&
				event.AltDown()) ||
				(m_int_mode == 6 &&
					m_editing_ruler_point == 0))
		{
			//disable picking
			m_pick = false;

			if (old_mouse_X != -1 &&
				old_mouse_Y != -1 &&
				abs(old_mouse_X - event.GetX()) +
				abs(old_mouse_Y - event.GetY())<200)
			{
				if (event.LeftIsDown() && !event.ControlDown())
				{
					Quaternion q_delta = Trackball(old_mouse_X, event.GetY(), event.GetX(), old_mouse_Y);
					if (m_rot_lock && q_delta.IsIdentity())
						hold_old = true;
					m_q *= q_delta;
					m_q.Normalize();
					Quaternion cam_pos(0.0, 0.0, m_distance, 0.0);
					Quaternion cam_pos2 = (-m_q) * cam_pos * m_q;
					m_transx = cam_pos2.x;
					m_transy = cam_pos2.y;
					m_transz = cam_pos2.z;

					Quaternion up(0.0, 1.0, 0.0, 0.0);
					Quaternion up2 = (-m_q) * up * m_q;
					m_up = Vector(up2.x, up2.y, up2.z);

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
					long dx = event.GetX() - old_mouse_X;
					long dy = event.GetY() - old_mouse_Y;

					m_head = Vector(-m_transx, -m_transy, -m_transz);
					m_head.normalize();
					Vector side = Cross(m_up, m_head);
					Vector trans = -(
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
					long dx = event.GetX() - old_mouse_X;
					long dy = event.GetY() - old_mouse_Y;

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
						Vector pos(m_transx, m_transy, m_transz);
						pos.normalize();
						Vector ctr(m_ctrx, m_ctry, m_ctrz);
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
				abs(old_mouse_X - event.GetX()) +
				abs(old_mouse_Y - event.GetY())<200)
			{
				if (event.LeftIsDown())
				{
					Quaternion q_delta = TrackballClip(old_mouse_X, event.GetY(), event.GetX(), old_mouse_Y);
					m_q_cl = q_delta * m_q_cl;
					m_q_cl.Normalize();
					SetRotations(m_rotx, m_roty, m_rotz);
					RefreshGL(34);
				}
			}
		}
		else if (m_int_mode == 6)
		{
			if (event.LeftIsDown() &&
				m_editing_ruler_point)
			{
				Point point;
				bool failed = false;
				if (m_point_volume_mode)
				{
					double t = GetPointVolume(point,
						event.GetX(), event.GetY(),
						m_cur_vol, m_point_volume_mode,
						m_ruler_use_transf);
					if (t <= 0.0)
						t = GetPointPlane(point,
							event.GetX(), event.GetY(),
							m_editing_ruler_point);
					if (t <= 0.0)
						failed = true;
				}
				else
				{
					double t = GetPointPlane(point,
						event.GetX(), event.GetY(),
						m_editing_ruler_point);
					if (t <= 0.0)
						failed = true;
				}
				if (!failed)
				{
					m_editing_ruler_point->x(point.x());
					m_editing_ruler_point->y(point.y());
					m_editing_ruler_point->z(point.z());
					RefreshGL(35);
					VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
					if (m_vrv && vr_frame && vr_frame->GetMeasureDlg())
						vr_frame->GetMeasureDlg()->GetSettings(m_vrv);
				}
			}
		}

		//update mouse position
		if (old_mouse_X >= 0 && old_mouse_Y >= 0)
		{
			prv_mouse_X = old_mouse_X;
			prv_mouse_Y = old_mouse_Y;
			if (!hold_old)
			{
				old_mouse_X = event.GetX();
				old_mouse_Y = event.GetY();
			}
		}
		else
		{
			old_mouse_X = event.GetX();
			old_mouse_Y = event.GetY();
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
		old_mouse_X = event.GetX();
		old_mouse_Y = event.GetY();
		RefreshGL(37);
		return;
	}

	if (m_draw_info & INFO_DISP)
	{
		VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
		if (vr_frame && vr_frame->GetMovieView() &&
			vr_frame->GetMovieView()->GetRunning())
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

Color VRenderGLView::GetBackgroundColor()
{
	return m_bg_color;
}

Color VRenderGLView::GetTextColor()
{
	VRenderFrame* frame = (VRenderFrame*)m_frame;
	if (!frame || !frame->GetSettingDlg())
		return m_bg_color_inv;
	switch (frame->GetSettingDlg()->GetTextColor())
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

void VRenderGLView::SetBackgroundColor(Color &color)
{
	m_bg_color = color;
	HSVColor bg_color(m_bg_color);
	double hue, sat, val;
	if (bg_color.val()>0.7 && bg_color.sat()>0.7)
	{
		hue = bg_color.hue() + 180.0;
		sat = 1.0;
		val = 1.0;
		m_bg_color_inv = Color(HSVColor(hue, sat, val));
	}
	else if (bg_color.val()>0.7)
	{
		m_bg_color_inv = Color(0.0, 0.0, 0.0);
	}
	else
	{
		m_bg_color_inv = Color(1.0, 1.0, 1.0);
	}
}

void VRenderGLView::SetGradBg(bool val)
{
	m_grad_bg = val;
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

	Quaternion cam_pos(0.0, 0.0, m_distance, 0.0);
	Quaternion cam_pos2 = (-m_q) * cam_pos * m_q;
	m_transx = cam_pos2.x;
	m_transy = cam_pos2.y;
	m_transz = cam_pos2.z;

	Quaternion up(0.0, 1.0, 0.0, 0.0);
	Quaternion up2 = (-m_q) * up * m_q;
	m_up = Vector(up2.x, up2.y, up2.z);

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

char* VRenderGLView::wxStringToChar(wxString input)
{
#if (wxUSE_UNICODE)
	size_t size = input.size() + 1;
	char *buffer = new char[size];//No need to multiply by 4, converting to 1 byte char only.
	memset(buffer, 0, size); //Good Practice, Can use buffer[0] = '&#65533;' also.
	wxEncodingConverter wxec;
	wxec.Init(wxFONTENCODING_ISO8859_1, wxFONTENCODING_ISO8859_1, wxCONVERT_SUBSTITUTE);
	wxec.Convert(input.mb_str(), buffer);
	return buffer; //To free this buffer memory is user responsibility.
#else
	return (char *)(input.c_str());
#endif
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

		glm::mat4 mv_temp;
		//translate object
		mv_temp = glm::translate(m_mv_mat, glm::vec3(m_obj_transx, m_obj_transy, m_obj_transz));
		//rotate object
		mv_temp = glm::rotate(mv_temp, float(glm::radians(m_obj_rotx)), glm::vec3(1.0, 0.0, 0.0));
		mv_temp = glm::rotate(mv_temp, float(glm::radians(m_obj_roty + 180.0)), glm::vec3(0.0, 1.0, 0.0));
		mv_temp = glm::rotate(mv_temp, float(glm::radians(m_obj_rotz + 180.0)), glm::vec3(0.0, 0.0, 1.0));
		//center object
		mv_temp = glm::translate(mv_temp, glm::vec3(-m_obj_ctrx, -m_obj_ctry, -m_obj_ctrz));

		Transform mv;
		Transform pr;
		mv.set(glm::value_ptr(mv_temp));
		pr.set(glm::value_ptr(m_proj_mat));

		double minx, maxx, miny, maxy;
		minx = 1.0;
		maxx = -1.0;
		miny = 1.0;
		maxy = -1.0;
		vector<Point> points;
		BBox bbox = m_cur_vol->GetBounds();
		points.push_back(Point(bbox.min().x(), bbox.min().y(), bbox.min().z()));
		points.push_back(Point(bbox.min().x(), bbox.min().y(), bbox.max().z()));
		points.push_back(Point(bbox.min().x(), bbox.max().y(), bbox.min().z()));
		points.push_back(Point(bbox.min().x(), bbox.max().y(), bbox.max().z()));
		points.push_back(Point(bbox.max().x(), bbox.min().y(), bbox.min().z()));
		points.push_back(Point(bbox.max().x(), bbox.min().y(), bbox.max().z()));
		points.push_back(Point(bbox.max().x(), bbox.max().y(), bbox.min().z()));
		points.push_back(Point(bbox.max().x(), bbox.max().y(), bbox.max().z()));

		Point p;
		for (unsigned int i = 0; i<points.size(); ++i)
		{
			p = mv.transform(points[i]);
			p = pr.transform(p);
			minx = p.x()<minx ? p.x() : minx;
			maxx = p.x()>maxx ? p.x() : maxx;
			miny = p.y()<miny ? p.y() : miny;
			maxy = p.y()>maxy ? p.y() : maxy;
		}

		minx = Clamp(minx, -1.0, 1.0);
		maxx = Clamp(maxx, -1.0, 1.0);
		miny = Clamp(miny, -1.0, 1.0);
		maxy = Clamp(maxy, -1.0, 1.0);

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
	VertexArray* quad_va =
		TextureRenderer::vertex_array_manager_.vertex_array(VA_Norm_Square);
	if (quad_va)
		quad_va->draw();
}

void VRenderGLView::switchLevel(VolumeData *vd)
{
	if (!vd) return;

	int nx, ny;
	nx = GetSize().x;
	ny = GetSize().y;
	if (m_enlarge)
	{
		nx = int(nx * m_enlarge_scale);
		ny = int(ny * m_enlarge_scale);
	}

	Texture *vtex = vd->GetTexture();
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

				double sf;
				if (aspect > 1.0)
					sf = 2.0*m_radius*res_scale / spc_y / double(ny);
				else
					sf = 2.0*m_radius*res_scale / spc_x / double(nx);
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
			VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
			if (vr_frame && vr_frame->GetSettingDlg())
				lv += vr_frame->GetSettingDlg()->GetDetailLevelOffset();
			if (lv < 0) lv = 0;
			//if (m_interactive) lv += 1;
			if (lv >= lvnum) lv = lvnum - 1;
			new_lv = lv;
		}
		if (prev_lv != new_lv)
		{
			vector<TextureBrick*> *bricks = vtex->get_bricks();
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

