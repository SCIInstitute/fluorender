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

#include <ShaderProgram.h>
#include <KernelProgram.h>
#include <RenderCanvas.h>
#include <VRenderView.h>
#include <VRenderFrame.h>
#include <Renderview.hpp>
#include <RenderviewFactory.hpp>
#include <AgentFactory.hpp>
#include <Global.hpp>
#include <RulerHandler.h>
#include <RulerRenderer.h>
#include <VolumePoint.h>
#include <VolumeSelector.h>
#include <VolumeCalculator.h>
#include <ScriptProc.h>
#include <Debug.h>

#include <wx/stdpaths.h>
#include <wx/msw/private.h>
#include "png_resource.h"
#include "img/icons.h"

bool RenderCanvas::m_linked_rot = false;
RenderCanvas* RenderCanvas::m_master_linked_view = 0;

BEGIN_EVENT_TABLE(RenderCanvas, wxGLCanvas)
EVT_PAINT(RenderCanvas::OnDraw)
EVT_SIZE(RenderCanvas::OnResize)
EVT_MOUSE_EVENTS(RenderCanvas::OnMouse)
EVT_IDLE(RenderCanvas::OnIdle)
EVT_KEY_DOWN(RenderCanvas::OnKeyDown)
EVT_TIMER(ID_ftrigger, RenderCanvas::OnQuitFscreen)
EVT_CLOSE(RenderCanvas::OnClose)
END_EVENT_TABLE()

RenderCanvas::RenderCanvas(VRenderFrame* frame,
	VRenderView* parent,
	const wxGLAttributes& attriblist,
	wxGLContext* sharedContext,
	const wxPoint& pos,
	const wxSize& size,
	long style) :
	wxGLCanvas(parent, attriblist, wxID_ANY, pos, size, style),
	m_frame(frame),
	m_vrv(parent),
	//initializaion
	m_initialized(false),
	//timer for fullscreen
	m_fullscreen_trigger(this, ID_ftrigger),
	//set gl
	m_set_gl(false)
{
	m_glRC = sharedContext;
	m_sharedRC = m_glRC ? true : false;

	fluo::Renderview* view = glbin_revf->build();
	view->setName(m_vrv->GetName().ToStdString());
	m_agent = glbin_agtf->getOrAddRenderCanvasAgent(view->getName(), *this);
	m_agent->setObject(view);
	m_agent->setValue(gstHwnd, (unsigned long long)GetHWND());
	m_agent->setValue(gstHinstance, (unsigned long long)::wxGetInstance());

	bool bval = m_frame && m_frame->GetBenchmark();
	m_agent->setValue(gstBenchmark, bval);

	view->GetRulerHandler()->SetView(view);
	m_agent->getObject()->GetRulerHandler()->SetRulerList(
		m_agent->getObject()->GetRulerList());
	m_agent->getObject()->GetRulerRenderer()->SetView(view);
	m_agent->getObject()->GetRulerRenderer()->SetRulerList(
		m_agent->getObject()->GetRulerList());
	m_agent->getObject()->GetVolumePoint()->SetView(view);
	m_agent->getObject()->GetVolumeSelector()->SetView(view);
	m_agent->getObject()->GetVolumeCalculator()->SetRoot(glbin_root);
	m_agent->getObject()->GetVolumeCalculator()->SetView(view);
	m_agent->getObject()->GetVolumeCalculator()->SetVolumeSelector(
		m_agent->getObject()->GetVolumeSelector());
	m_agent->getObject()->GetScriptProc()->SetFrame(m_frame);
	m_agent->getObject()->GetScriptProc()->SetView(view);
}

RenderCanvas::~RenderCanvas()
{
	bool bval;
	m_agent->getValue(gstBenchmark, bval);
	if (bval)
	{
		unsigned long long msec, frames;
		double fps;
		m_agent->getValue(gstBmRuntime, msec);
		m_agent->getValue(gstBmFrames, frames);
		m_agent->getValue(gstBmFps, fps);
		long lx, ly;
		m_agent->getValue(gstSizeX, lx);
		m_agent->getValue(gstSizeY, ly);
		wxString string = wxString("FluoRender has finished benchmarking.\n") +
			wxString("Results:\n") +
			wxString("Render size: ") + wxString::Format("%d X %d\n", lx, ly) +
			wxString("Time: ") + wxString::Format("%llu msec\n", msec) +
			wxString("Frames: ") + wxString::Format("%llu\n", frames) +
			wxString("FPS: ") + wxString::Format("%.2f", fps);
		wxMessageDialog *diag = new wxMessageDialog(this, string, "Benchmark Results",
			wxOK | wxICON_INFORMATION);
		diag->ShowModal();
	}

	if (m_full_screen)
	{
		m_full_screen = false;
		m_vrv->SetCanvas(0);
		m_vrv->SetFullFrame(0);
		if (m_frame)
		{
			m_frame->ClearVrvList();
			m_frame->Close();
		}
	}

	if (!m_sharedRC)
		delete m_glRC;
}

void RenderCanvas::OnResize(wxSizeEvent& event)
{
	double dval = 1;
#ifdef _DARWIN
	dval = GetDPIScaleFactor();
#endif
	wxSize size = GetSize() * dval;
	m_agent->setValue(gstSizeX, long(size.x));
	m_agent->setValue(gstSizeY, long(size.y));
	m_vrv->UpdateScaleFactor(false);
	m_agent->getObject()->RefreshGL(1);
	//RefreshGL(1);
}

void RenderCanvas::Init()
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
		////glViewport(0, 0, (GLint)(GetSize().x), (GLint)(GetSize().y));
		//glEnable(GL_MULTISAMPLE);

		m_initialized = true;

		//glbin_timer->start();
	}
}

//change brush display
void RenderCanvas::ChangeBrushSize(int value)
{
	m_agent->getObject()->GetVolumeSelector()->ChangeBrushSize(
		value, wxGetKeyState(WXK_CONTROL));
	if (m_frame && m_frame->GetBrushToolDlg())
		m_frame->GetBrushToolDlg()->GetSettings(m_agent->getObject());
}

void RenderCanvas::UpdateBrushState()
{
	TreePanel* tree_panel = 0;
	BrushToolDlg* brush_dlg = 0;
	if (m_frame)
	{
		tree_panel = m_frame->GetTree();
		brush_dlg = m_frame->GetBrushToolDlg();
	}

	fluo::Renderview* view = m_agent->getObject();
	long int_mode;
	m_agent->getValue(gstInterMode, int_mode);
	if (int_mode != 2 && int_mode != 7)
	{
		if (wxGetKeyState(WXK_SHIFT))
		{
			if (tree_panel)
				tree_panel->SelectBrush(TreePanel::ID_BrushAppend);
			if (brush_dlg)
				brush_dlg->SelectBrush(BrushToolDlg::ID_BrushAppend);
			if (view)
			{
				view->SetBrush(2);
				view->RefreshGL(6);
			}
		}
		else if (wxGetKeyState(wxKeyCode('Z')))
		{
			if (tree_panel)
				tree_panel->SelectBrush(TreePanel::ID_BrushDiffuse);
			if (brush_dlg)
				brush_dlg->SelectBrush(BrushToolDlg::ID_BrushDiffuse);
			if (view)
			{
				view->SetBrush(4);
				view->RefreshGL(7);
			}
		}
		else if (wxGetKeyState(wxKeyCode('X')))
		{
			if (tree_panel)
				tree_panel->SelectBrush(TreePanel::ID_BrushDesel);
			if (brush_dlg)
				brush_dlg->SelectBrush(BrushToolDlg::ID_BrushDesel);
			if (view)
			{
				view->SetBrush(3);
				view->RefreshGL(8);
			}
		}
	}
	else
	{
		long lval;
		m_agent->getValue(gstBrushState, lval);
		if (lval)
		{
			if (wxGetKeyState(WXK_SHIFT))
			{
				m_agent->setValue(gstBrushState, long(0));
				if (tree_panel)
					tree_panel->SelectBrush(TreePanel::ID_BrushAppend);
				if (brush_dlg)
					brush_dlg->SelectBrush(BrushToolDlg::ID_BrushAppend);
				if (view)
				{
					view->SetBrush(2);
					view->RefreshGL(9);
				}
			}
			else if (wxGetKeyState(wxKeyCode('Z')))
			{
				m_agent->setValue(gstBrushState, long(0));
				if (tree_panel)
					tree_panel->SelectBrush(TreePanel::ID_BrushDiffuse);
				if (brush_dlg)
					brush_dlg->SelectBrush(BrushToolDlg::ID_BrushDiffuse);
				if (view)
				{
					view->SetBrush(4);
					view->RefreshGL(10);
				}
			}
			else if (wxGetKeyState(wxKeyCode('X')))
			{
				m_agent->setValue(gstBrushState, long(0));
				if (tree_panel)
					tree_panel->SelectBrush(TreePanel::ID_BrushDesel);
				if (brush_dlg)
					brush_dlg->SelectBrush(BrushToolDlg::ID_BrushDesel);
				if (view)
				{
					view->SetBrush(3);
					view->RefreshGL(11);
				}
			}
			else
			{
				if (view)
				{
					view->SetBrush(lval);
					view->RefreshGL(12);
				}
			}
		}
		else if (!wxGetKeyState(WXK_SHIFT) &&
			!wxGetKeyState(wxKeyCode('Z')) &&
			!wxGetKeyState(wxKeyCode('X')))
		{
			if (wxGetMouseState().LeftIsDown())
				view->Segment();
			long lval;
			m_agent->getValue(gstInterMode, lval);
			if (lval == 7)
				m_agent->setValue(gstInterMode, 5);
			else
				m_agent->setValue(gstInterMode, 1);
			m_agent->setValue(gstPaintDisplay, false);
			m_agent->setValue(gstDrawBrush, false);
			if (tree_panel)
				tree_panel->SelectBrush(0);
			if (brush_dlg)
				brush_dlg->SelectBrush(0);
			view->RefreshGL(13);

			if (m_prev_focus)
			{
				m_prev_focus->SetFocus();
				m_prev_focus = 0;
			}
		}
	}
}

void RenderCanvas::PickMesh()
{
	wxPoint mouse_pos = ScreenToClient(wxGetMousePosition());
	m_agent->setValue(gstMouseClientX, long(mouse_pos.x));
	m_agent->setValue(gstMouseClientY, long(mouse_pos.y));
	m_agent->getObject()->PickMesh();

	//if (choose >0 && choose <= (int)m_md_pop_list.size())
	//{
	//	fluo::MeshData* md = m_md_pop_list[choose - 1];
	//	if (md)
	//	{
	//		if (m_frame && m_frame->GetTree())
	//		{
	//			m_frame->GetTree()->SetFocus();
	//			m_frame->GetTree()->Select(m_vrv->GetName(), md->getName());
	//		}
	//		RefreshGL(27);
	//	}
	//}
	//else
	//{
	//	if (m_frame && m_frame->GetCurSelType() == 3 &&
	//		m_frame->GetTree())
	//		m_frame->GetTree()->Select(m_vrv->GetName(), "");
	//}
	//m_mv_mat = mv_temp;
}

void RenderCanvas::PickVolume()
{
	int kmode = wxGetKeyState(WXK_CONTROL) ? 1 : 0;
	//double dist = 0.0;
	//double min_dist = -1.0;
	//fluo::Point p, ip, pp;
	fluo::VolumeData* vd = 0;
	fluo::VolumeData* picked_vd = 0;
	//for (int i = 0; i<(int)m_vd_pop_list.size(); i++)
	//{
	//	vd = m_vd_pop_list[i];
	//	if (!vd) continue;
	//	long mode = 2;
	//	long mip_mode;
	//	vd->getValue(gstMipMode, mip_mode);
	//	if (mip_mode == 1) mode = 1;
	//	m_vp.SetVolumeData(vd);
	//	dist = m_vp.GetPointVolume(old_mouse_X, old_mouse_Y,
	//		mode, true, 0.5, p, ip);
	//	if (dist > 0.0)
	//	{
	//		if (min_dist < 0.0)
	//		{
	//			min_dist = dist;
	//			picked_vd = vd;
	//			pp = p;
	//		}
	//		else
	//		{
	//			if (m_persp)
	//			{
	//				if (dist < min_dist)
	//				{
	//					min_dist = dist;
	//					picked_vd = vd;
	//					pp = p;
	//				}
	//			}
	//			else
	//			{
	//				if (dist > min_dist)
	//				{
	//					min_dist = dist;
	//					picked_vd = vd;
	//					pp = p;
	//				}
	//			}
	//		}
	//	}
	//}

	if (picked_vd)
	{
		if (m_frame && m_frame->GetTree())
		{
			m_frame->GetTree()->SetFocus();
			m_frame->GetTree()->Select(m_vrv->GetName(), picked_vd->getName());
		}
		//update label selection
		SetCompSelection(ip, kmode);

		//if (m_pick_lock_center)
		//	m_lock_center = pp;
	}
}

void RenderCanvas::SetCompSelection(fluo::Point& p, int mode)
{
	//update selection
	if (m_frame && m_frame->GetComponentDlg())
	{
		std::set<unsigned long long> ids;
		m_frame->GetComponentDlg()->GetAnalyzer()->GetCompsPoint(p, ids);
		m_frame->GetComponentDlg()->SetCompSelection(ids, mode);
	}
}

void RenderCanvas::OnIdle(wxIdleEvent& event)
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
		double fps = 1.0 / glbin_timer->average();
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
		long mode = 2;
		long mip_mode;
		m_cur_vol->getValue(gstMipMode, mip_mode);
		if (mip_mode == 1) mode = 1;
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
			p = fluo::Point(m_obj_ctrx - p.x(),
				p.y() - m_obj_ctry,
				p.z() - m_obj_ctrz);
			obj_transx = p.x();
			obj_transy = p.y();
			obj_transz = p.z();
			double thresh = 10.0;
			double spcx, spcy, spcz;
			m_cur_vol->getValue(gstSpcX, spcx);
			m_cur_vol->getValue(gstSpcY, spcy);
			m_cur_vol->getValue(gstSpcZ, spcz);
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
	if (window && view_reg.Contains(mouse_pos))
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

void RenderCanvas::OnKeyDown(wxKeyEvent& event)
{
	event.Skip();
}

void RenderCanvas::OnQuitFscreen(wxTimerEvent& event)
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

void RenderCanvas::OnClose(wxCloseEvent &event)
{
	if (m_full_screen)
	{
		m_fullscreen_trigger.Start(10);
		event.Veto();
	}
}

//void RenderCanvas::Set3DRotCapture(double start_angle,
//	double end_angle,
//	double step,
//	int frames,
//	int rot_axis,
//	wxString &cap_file,
//	bool rewind,
//	int len)
//{
//	double rv[3];
//	GetRotations(rv[0], rv[1], rv[2]);
//
//	//remove the chance of the x/y/z angles being outside 360.
//	while (rv[0] > 360.)  rv[0] -= 360.;
//	while (rv[0] < -360.) rv[0] += 360.;
//	while (rv[1] > 360.)  rv[1] -= 360.;
//	while (rv[1] < -360.) rv[1] += 360.;
//	while (rv[2] > 360.)  rv[2] -= 360.;
//	while (rv[2] < -360.) rv[2] += 360.;
//	if (360. - std::abs(rv[0]) < 0.001) rv[0] = 0.;
//	if (360. - std::abs(rv[1]) < 0.001) rv[1] = 0.;
//	if (360. - std::abs(rv[2]) < 0.001) rv[2] = 0.;
//
//	m_step = step;
//	m_total_frames = frames;
//	m_cap_file = cap_file;
//	m_rewind = rewind;
//	m_movie_seq = 0;
//
//	m_rot_axis = rot_axis;
//	if (start_angle == 0.)
//	{
//		m_init_angle = rv[m_rot_axis];
//		m_end_angle = rv[m_rot_axis] + end_angle;
//	}
//	m_cur_angle = rv[m_rot_axis];
//	m_start_angle = rv[m_rot_axis];
//
//	m_capture = true;
//	m_capture_rotat = true;
//	m_capture_rotate_over = false;
//	m_stages = 0;
//}

//void RenderCanvas::Set3DBatCapture(wxString &cap_file, int begin_frame, int end_frame)
//{
//	m_cap_file = cap_file;
//	m_begin_frame = begin_frame;
//	m_end_frame = end_frame;
//	m_capture_bat = true;
//	m_capture = true;
//
//	if (!m_cap_file.IsEmpty() && m_total_frames>1)
//	{
//		wxString new_folder = wxPathOnly(m_cap_file)
//			+ GETSLASH() + m_bat_folder + "_folder";
//		MkDirW(new_folder.ToStdWstring());
//	}
//}

//void RenderCanvas::Set4DSeqCapture(wxString &cap_file, int begin_frame, int end_frame, bool rewind)
//{
//	m_cap_file = cap_file;
//	m_tseq_cur_num = begin_frame;
//	m_tseq_prv_num = begin_frame;
//	m_begin_frame = begin_frame;
//	m_end_frame = end_frame;
//	m_capture_tsequ = true;
//	m_capture = true;
//	m_movie_seq = begin_frame;
//	m_4d_rewind = rewind;
//}

//void RenderCanvas::SetParamCapture(wxString &cap_file, int begin_frame, int end_frame, bool rewind)
//{
//	m_cap_file = cap_file;
//	m_param_cur_num = begin_frame;
//	m_begin_frame = begin_frame;
//	m_end_frame = end_frame;
//	m_capture_param = true;
//	m_capture = true;
//	m_movie_seq = begin_frame;
//	m_4d_rewind = rewind;
//}

//void RenderCanvas::SetParams(double t)
//{
//	if (!m_vrv)
//		return;
//	if (!m_frame)
//		return;
//	ClippingView* clip_view = m_frame->GetClippingView();
//	Interpolator *interpolator = m_frame->GetInterpolator();
//	if (!interpolator)
//		return;
//	FlKeyCode keycode;
//	keycode.l0 = 1;
//	keycode.l0_name = m_vrv->GetName();
//
//	for (int i = 0; i<GetAllVolumeNum(); i++)
//	{
//		fluo::VolumeData* vd = GetAllVolumeData(i);
//		if (!vd) continue;
//
//		keycode.l1 = 2;
//		keycode.l1_name = vd->getName();
//
//		//display
//		keycode.l2 = 0;
//		keycode.l2_name = "display";
//		bool bval;
//		if (interpolator->GetBoolean(keycode, t, bval))
//			vd->setValue(gstDisplay, bval);
//
//		//clipping planes
//		vector<fluo::Plane*> *planes = vd->GetRenderer()->get_planes();
//		if (!planes) continue;
//		if (planes->size() != 6) continue;
//		fluo::Plane *plane = 0;
//		double val = 0;
//		//x1
//		plane = (*planes)[0];
//		keycode.l2 = 0;
//		keycode.l2_name = "x1_val";
//		if (interpolator->GetDouble(keycode, t, val))
//			plane->ChangePlane(fluo::Point(abs(val), 0.0, 0.0),
//				fluo::Vector(1.0, 0.0, 0.0));
//		//x2
//		plane = (*planes)[1];
//		keycode.l2 = 0;
//		keycode.l2_name = "x2_val";
//		if (interpolator->GetDouble(keycode, t, val))
//			plane->ChangePlane(fluo::Point(abs(val), 0.0, 0.0),
//				fluo::Vector(-1.0, 0.0, 0.0));
//		//y1
//		plane = (*planes)[2];
//		keycode.l2 = 0;
//		keycode.l2_name = "y1_val";
//		if (interpolator->GetDouble(keycode, t, val))
//			plane->ChangePlane(fluo::Point(0.0, abs(val), 0.0),
//				fluo::Vector(0.0, 1.0, 0.0));
//		//y2
//		plane = (*planes)[3];
//		keycode.l2 = 0;
//		keycode.l2_name = "y2_val";
//		if (interpolator->GetDouble(keycode, t, val))
//			plane->ChangePlane(fluo::Point(0.0, abs(val), 0.0),
//				fluo::Vector(0.0, -1.0, 0.0));
//		//z1
//		plane = (*planes)[4];
//		keycode.l2 = 0;
//		keycode.l2_name = "z1_val";
//		if (interpolator->GetDouble(keycode, t, val))
//			plane->ChangePlane(fluo::Point(0.0, 0.0, abs(val)),
//				fluo::Vector(0.0, 0.0, 1.0));
//		//z2
//		plane = (*planes)[5];
//		keycode.l2 = 0;
//		keycode.l2_name = "z2_val";
//		if (interpolator->GetDouble(keycode, t, val))
//			plane->ChangePlane(fluo::Point(0.0, 0.0, abs(val)),
//				fluo::Vector(0.0, 0.0, -1.0));
//		//t
//		double frame;
//		keycode.l2 = 0;
//		keycode.l2_name = "frame";
//		if (interpolator->GetDouble(keycode, t, frame))
//			UpdateVolumeData(int(frame + 0.5), vd);
//	}
//
//	bool bx, by, bz;
//	//for the view
//	keycode.l1 = 1;
//	keycode.l1_name = m_vrv->GetName();
//	//translation
//	double tx, ty, tz;
//	keycode.l2 = 0;
//	keycode.l2_name = "translation_x";
//	bx = interpolator->GetDouble(keycode, t, tx);
//	keycode.l2_name = "translation_y";
//	by = interpolator->GetDouble(keycode, t, ty);
//	keycode.l2_name = "translation_z";
//	bz = interpolator->GetDouble(keycode, t, tz);
//	if (bx && by && bz)
//		SetTranslations(tx, ty, tz);
//	//centers
//	keycode.l2_name = "center_x";
//	bx = interpolator->GetDouble(keycode, t, tx);
//	keycode.l2_name = "center_y";
//	by = interpolator->GetDouble(keycode, t, ty);
//	keycode.l2_name = "center_z";
//	bz = interpolator->GetDouble(keycode, t, tz);
//	if (bx && by && bz)
//		SetCenters(tx, ty, tz);
//	//obj translation
//	keycode.l2_name = "obj_trans_x";
//	bx = interpolator->GetDouble(keycode, t, tx);
//	keycode.l2_name = "obj_trans_y";
//	by = interpolator->GetDouble(keycode, t, ty);
//	keycode.l2_name = "obj_trans_z";
//	bz = interpolator->GetDouble(keycode, t, tz);
//	if (bx && by && bz)
//		SetObjTrans(tx, ty, tz);
//	//scale
//	double scale;
//	keycode.l2_name = "scale";
//	if (interpolator->GetDouble(keycode, t, scale))
//	{
//		m_scale_factor = scale;
//		m_vrv->UpdateScaleFactor(false);
//	}
//	//rotation
//	keycode.l2 = 0;
//	keycode.l2_name = "rotation";
//	fluo::Quaternion q;
//	if (interpolator->GetQuaternion(keycode, t, q))
//	{
//		m_q = q;
//		q *= -m_zq;
//		double rotx, roty, rotz;
//		q.ToEuler(rotx, roty, rotz);
//		SetRotations(rotx, roty, rotz, true);
//	}
//	//intermixing mode
//	keycode.l2_name = "volmethod";
//	int ival;
//	if (interpolator->GetInt(keycode, t, ival))
//		SetVolMethod(ival);
//	//perspective angle
//	keycode.l2_name = "aov";
//	double aov;
//	if (interpolator->GetDouble(keycode, t, aov))
//	{
//		if (aov <= 10)
//		{
//			SetPersp(false);
//			m_vrv->m_aov_text->ChangeValue("Ortho");
//			m_vrv->m_aov_sldr->SetValue(10);
//		}
//		else
//		{
//			SetPersp(true);
//			SetAov(aov);
//		}
//	}
//
//	if (m_frame && clip_view)
//		clip_view->SetVolumeData(m_frame->GetCurSelVol());
//	if (m_frame)
//	{
//		m_frame->UpdateTree(m_cur_vol ? m_cur_vol->getName() : "");
//		int index = interpolator->GetKeyIndexFromTime(t);
//		m_frame->GetRecorderDlg()->SetSelection(index);
//	}
//	SetVolPopDirty();
//}

//void RenderCanvas::ResetMovieAngle()
//{
//	double rv[3];
//	GetRotations(rv[0], rv[1], rv[2]);
//	rv[m_rot_axis] = m_init_angle;
//	SetRotations(rv[0], rv[1], rv[2]);
//
//	m_capture = false;
//	m_capture_rotat = false;
//
//	RefreshGL(16);
//}

//void RenderCanvas::StopMovie()
//{
//	m_capture = false;
//	m_capture_rotat = false;
//	m_capture_tsequ = false;
//	m_capture_param = false;
//}

//void RenderCanvas::Get4DSeqRange(int &start_frame, int &end_frame)
//{
//	for (int i = 0; i<(int)m_vd_pop_list.size(); i++)
//	{
//		fluo::VolumeData* vd = m_vd_pop_list[i];
//		if (vd && vd->GetReader())
//		{
//			BaseReader* reader = vd->GetReader();
//
//			int vd_start_frame = 0;
//			int vd_end_frame = reader->GetTimeNum() - 1;
//			int vd_cur_frame = reader->GetCurTime();
//
//			if (i == 0)
//			{
//				//first dataset
//				start_frame = vd_start_frame;
//				end_frame = vd_end_frame;
//			}
//			else
//			{
//				//datasets after the first one
//				if (vd_end_frame > end_frame)
//					end_frame = vd_end_frame;
//			}
//		}
//	}
//}

//void RenderCanvas::UpdateVolumeData(int frame, fluo::VolumeData* vd)
//{
//	if (vd && vd->GetReader())
//	{
//		BaseReader* reader = vd->GetReader();
//		bool clear_pool = false;
//
//		long cur_time;
//		vd->getValue(gstTime, cur_time);
//		if (cur_time != frame)
//		{
//			flvr::Texture *tex = vd->GetTexture();
//			if (tex && tex->isBrxml())
//			{
//				BRKXMLReader *br = (BRKXMLReader *)reader;
//				br->SetCurTime(frame);
//				int curlv = tex->GetCurLevel();
//				for (int j = 0; j < br->GetLevelNum(); j++)
//				{
//					tex->setLevel(j);
//					if (vd->GetRenderer()) vd->GetRenderer()->clear_brick_buf();
//				}
//				tex->setLevel(curlv);
//				long chan;
//				vd->getValue(gstChannel, chan);
//				tex->set_FrameAndChannel(frame, chan);
//				vd->setValue(gstTime, long(reader->GetCurTime()));
//				//update rulers
//				if (m_frame && m_frame->GetMeasureDlg())
//					m_frame->GetMeasureDlg()->UpdateList();
//			}
//			else
//			{
//				double spcx, spcy, spcz;
//				vd->getValue(gstSpcX, spcx);
//				vd->getValue(gstSpcY, spcy);
//				vd->getValue(gstSpcZ, spcz);
//
//				long chan;
//				vd->getValue(gstChannel, chan);
//				Nrrd* data = reader->Convert(frame, chan, false);
//				if (!vd->ReplaceData(data, false))
//					return;
//
//				vd->setValue(gstTime, long(reader->GetCurTime()));
//				vd->setValue(gstSpcX, spcx);
//				vd->setValue(gstSpcY, spcy);
//				vd->setValue(gstSpcZ, spcz);
//
//				//update rulers
//				if (m_frame && m_frame->GetMeasureDlg())
//					m_frame->GetMeasureDlg()->UpdateList();
//
//				clear_pool = true;
//			}
//		}
//
//		if (clear_pool && vd->GetRenderer())
//			vd->GetRenderer()->clear_tex_pool();
//	}
//}

//void RenderCanvas::ReloadVolumeData(int frame)
//{
//	int i, j;
//	vector<BaseReader*> reader_list;
//	m_bat_folder = "";
//
//	for (i = 0; i < (int)m_vd_pop_list.size(); i++)
//	{
//		fluo::VolumeData* vd = m_vd_pop_list[i];
//		if (vd && vd->GetReader())
//		{
//			flvr::Texture *tex = vd->GetTexture();
//			BaseReader* reader = vd->GetReader();
//			if (tex && tex->isBrxml())
//			{
//				BRKXMLReader *br = (BRKXMLReader *)reader;
//				int curlv = tex->GetCurLevel();
//				for (j = 0; j < br->GetLevelNum(); j++)
//				{
//					tex->setLevel(j);
//					if (vd->GetRenderer()) vd->GetRenderer()->clear_brick_buf();
//				}
//				tex->setLevel(curlv);
//				long chan;
//				vd->getValue(gstChannel, chan);
//				tex->set_FrameAndChannel(0, chan);
//				vd->setValue(gstTime, long(reader->GetCurTime()));
//				wxString data_name = wxString(reader->GetDataName());
//				if (i > 0)
//					m_bat_folder += "_";
//				m_bat_folder += data_name;
//
//				int chan_num = 0;
//				if (data_name.Find("_1ch") != -1)
//					chan_num = 1;
//				else if (data_name.Find("_2ch") != -1)
//					chan_num = 2;
//				if (chan_num > 0 && chan >= chan_num)
//					vd->setValue(gstDisplay, false);
//				else
//					vd->setValue(gstDisplay, true);
//
//				if (reader->GetChanNum() > 1)
//					data_name += wxString::Format("_%d", chan + 1);
//				vd->setName(data_name.ToStdString());
//			}
//			else
//			{
//				bool found = false;
//				for (j = 0; j < (int)reader_list.size(); j++)
//				{
//					if (reader == reader_list[j])
//					{
//						found = true;
//						break;
//					}
//				}
//				if (!found)
//				{
//					reader->LoadOffset(frame);
//					reader_list.push_back(reader);
//				}
//
//				double spcx, spcy, spcz;
//				vd->getValue(gstSpcX, spcx);
//				vd->getValue(gstSpcY, spcy);
//				vd->getValue(gstSpcZ, spcz);
//				long chan;
//				vd->getValue(gstChannel, chan);
//				Nrrd* data = reader->Convert(0, chan, true);
//				if (vd->ReplaceData(data, true))
//					vd->setValue(gstDisplay, true);
//				else
//				{
//					vd->setValue(gstDisplay, false);
//					continue;
//				}
//
//				wxString data_name = wxString(reader->GetDataName());
//				if (i > 0)
//					m_bat_folder += "_";
//				m_bat_folder += data_name;
//
//				int chan_num = 0;
//				if (data_name.Find("_1ch") != -1)
//					chan_num = 1;
//				else if (data_name.Find("_2ch") != -1)
//					chan_num = 2;
//				if (chan_num > 0 && chan >= chan_num)
//					vd->setValue(gstDisplay, false);
//				else
//					vd->setValue(gstDisplay, true);
//
//				if (reader->GetChanNum() > 1)
//					data_name += wxString::Format("_%d", chan + 1);
//				vd->setName(data_name.ToStdString());
//				vd->setValue(gstDataPath, reader->GetPathName());
//				vd->setValue(gstTime, long(reader->GetCurTime()));
//				if (!reader->IsSpcInfoValid())
//				{
//					vd->setValue(gstSpcX, spcx);
//					vd->setValue(gstSpcY, spcy);
//					vd->setValue(gstSpcZ, spcz);
//				}
//				else
//				{
//					vd->setValue(gstSpcX, reader->GetXSpc());
//					vd->setValue(gstSpcY, reader->GetYSpc());
//					vd->setValue(gstSpcZ, reader->GetZSpc());
//				}
//				if (vd->GetRenderer())
//					vd->GetRenderer()->clear_tex_pool();
//			}
//		}
//	}
//
//	InitView(INIT_BOUNDS | INIT_CENTER);
//
//	if (m_frame)
//	{
//		m_frame->UpdateList();
//		m_frame->UpdateTree(
//			m_frame->GetCurSelVol() ?
//			m_frame->GetCurSelVol()->getName() :
//			"");
//	}
//}

void RenderCanvas::Set4DSeqFrame(int frame, int start_frame, int end_frame, bool rewind)
{
	//skip update if frame num unchanged
	bool update = m_tseq_cur_num == frame ? false : true;
	//compute frame number
	m_begin_frame = start_frame;
	m_end_frame = end_frame;
	m_total_frames = std::abs(end_frame - start_frame + 1);

	//save currently selected volume
	fluo::VolumeData* cur_vd_save = m_cur_vol;

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
		m_scriptor.Run4DScript(flrd::ScriptProc::TM_ALL_POST, m_script_file, rewind);

	//restore currently selected volume
	m_cur_vol = cur_vd_save;
	m_selector.SetVolume(m_cur_vol);
	m_calculator.SetVolumeA(m_cur_vol);

	RefreshGL(17);
}

void RenderCanvas::Get3DBatRange(int &start_frame, int &end_frame)
{
	m_bat_folder = "";

	for (int i = 0; i<(int)m_vd_pop_list.size(); i++)
	{
		fluo::VolumeData* vd = m_vd_pop_list[i];
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

void RenderCanvas::Set3DBatFrame(int frame, int start_frame, int end_frame, bool rewind)
{
	//skip update if frame num unchanged
	bool update = m_tseq_cur_num == frame ? false : true;
	//compute frame number
	m_begin_frame = start_frame;
	m_end_frame = end_frame;
	m_total_frames = std::abs(end_frame - start_frame + 1);

	//save currently selected volume
	fluo::VolumeData* cur_vd_save = m_cur_vol;

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
void RenderCanvas::PreDraw()
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
void RenderCanvas::ReadPixels(
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

void RenderCanvas::PostDraw()
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

		tsize_t linebytes = chann * w * (fp32?4:1);
		void *buf = NULL;
		buf = _TIFFmalloc(linebytes);
		TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(out, 0));
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

void RenderCanvas::ResetEnlarge()
{
	//skip if not done with loop
	if (flvr::TextureRenderer::get_mem_swap() &&
		flvr::TextureRenderer::get_start_update_loop() &&
		!flvr::TextureRenderer::get_done_update_loop())
		return;
	if (m_keep_enlarge)
		return;
	m_enlarge = false;
	flvr::TextRenderer::text_texture_manager_.SetSize(m_tsize);
	RefreshGL(19);
}

//draw
void RenderCanvas::OnDraw(wxPaintEvent& event)
{
#ifdef _WIN32
	if (!m_set_gl)
	{
		SetCurrent(*m_glRC);
		m_set_gl = true;
		if (m_frame)
		{
			for (int i = 0; i < m_frame->GetViewNum(); i++)
			{
				RenderCanvas* view = m_frame->GetView(i);
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
	wxPaintDC dc(this);
	Init();
	m_agent->getObject()->ForceDraw();
	//if (!m_refresh)
	//	m_retain_finalbuffer = true;
	//else
	//	m_refresh = false;
	//ForceDraw();
}

void RenderCanvas::ForceDraw()
{
//#ifdef _WIN32
//	if (!m_set_gl)
//	{
//		SetCurrent(*m_glRC);
//		m_set_gl = true;
//		if (m_frame)
//		{
//			for (int i = 0; i< m_frame->GetViewNum(); i++)
//			{
//				RenderCanvas* view = m_frame->GetView(i);
//				if (view && view != this)
//				{
//					view->m_set_gl = false;
//				}
//			}
//		}
//	}
//#endif
//#if defined(_DARWIN) || defined(__linux__)
//	SetCurrent(*m_glRC);
//#endif
//	Init();
//	wxPaintDC dc(this);

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

	glbin_timer->sample();
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
				RenderCanvas* view = m_frame->GetView(i);
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

void RenderCanvas::SetRadius(double r)
{
	m_radius = r;
}

void RenderCanvas::SetCenter()
{
	InitView(INIT_BOUNDS | INIT_CENTER | INIT_OBJ_TRANSL);

	fluo::VolumeData *vd = 0;
	if (m_cur_vol)
		vd = m_cur_vol;
	else if (m_vd_pop_list.size())
		vd = m_vd_pop_list[0];

	if (vd)
	{
		fluo::BBox bbox;
		vd->getValue(gstBounds, bbox);
		flvr::VolumeRenderer *vr = vd->GetRenderer();
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

double RenderCanvas::Get121ScaleFactor()
{
	double result = 1.0;

	int nx, ny;
	GetRenderSize(nx, ny);

	double spc_x = 1.0;
	double spc_y = 1.0;
	double spc_z = 1.0;
	fluo::VolumeData *vd = 0;
	if (m_cur_vol)
		vd = m_cur_vol;
	else if (m_vd_pop_list.size())
		vd = m_vd_pop_list[0];
	if (vd)
	{
		vd->getValue(gstSpcX, spc_x);
		vd->getValue(gstSpcY, spc_y);
		vd->getValue(gstSpcZ, spc_z);
		//vd->GetSpacings(spc_x, spc_y, spc_z, vd->GetLevel());
	}
	spc_y = spc_y<EPS ? 1.0 : spc_y;

	result = 2.0*m_radius / spc_y / double(ny);

	return result;
}

void RenderCanvas::SetScale121()
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
		fluo::VolumeData *vd = 0;
		if (m_cur_vol)
			vd = m_cur_vol;
		else if (m_vd_pop_list.size())
			vd = m_vd_pop_list[0];
		double spcx, spcy, spcz;
		if (vd)
		{
			vd->getValue(gstSpcX, spcx);
			vd->getValue(gstSpcY, spcy);
			vd->getValue(gstSpcZ, spcz);
			//vd->GetSpacings(spcx, spcy, spcz, vd->GetLevel());
		}
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

void RenderCanvas::SetPinRotCenter(bool pin)
{
	m_pin_rot_center = pin;
	if (pin)
		m_rot_center_dirty = true;
}

void RenderCanvas::SetPersp(bool persp)
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

void RenderCanvas::SetFree(bool free)
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

void RenderCanvas::SetAov(double aov)
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

void RenderCanvas::SetVolMethod(int method)
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

fluo::VolumeData* RenderCanvas::GetAllVolumeData(int index)
{
	int cnt = 0;
	int i, j;
	for (i = 0; i<GetLayerNum(); i++)
	{
		if (fluo::VolumeData* vd = dynamic_cast<fluo::VolumeData*>(GetLayer(i)))
		{
			if (cnt == index)
				return vd;
			cnt++;
		}
		else if (fluo::VolumeGroup* group = dynamic_cast<fluo::VolumeGroup*>(GetLayer(i)))//group
		{
			for (j = 0; j<group->getNumChildren(); j++)
			{
				if (cnt == index)
					return group->getChild(j)->asVolumeData();
				cnt++;
			}
		}
	}
	return nullptr;
}

fluo::VolumeData* RenderCanvas::GetDispVolumeData(int index)
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

fluo::MeshData* RenderCanvas::GetMeshData(int index)
{
	if (GetMeshNum() <= 0)
		return 0;

	PopMeshList();

	if (index >= 0 && index<(int)m_md_pop_list.size())
		return m_md_pop_list[index];
	else
		return 0;
}

fluo::VolumeData* RenderCanvas::GetVolumeData(const std::string &name)
{
	int i, j;

	for (i = 0; i<GetLayerNum(); i++)
	{
		if (fluo::VolumeData* vd = dynamic_cast<fluo::VolumeData*>(GetLayer(i)))
		{
			if (name == vd->getName())
				return vd;
		}
		else if (fluo::VolumeGroup* group = dynamic_cast<fluo::VolumeGroup*>(GetLayer(i)))//group
		{
			for (j = 0; j<group->getNumChildren(); j++)
			{
				fluo::VolumeData* vd = group->getChild(j)->asVolumeData();
				if (vd && name == vd->getName())
					return vd;
			}
		}
	}

	return nullptr;
}

fluo::MeshData* RenderCanvas::GetMeshData(const std::string &name)
{
	int i, j;

	for (i = 0; i< GetLayerNum(); i++)
	{
		if (fluo::MeshData* md = dynamic_cast<fluo::MeshData*>(GetLayer(i)))
		{
			if (name == md->getName())
				return md;
		}
		if (fluo::MeshGroup* group = dynamic_cast<fluo::MeshGroup*>(GetLayer(i)))//mesh group
		{
			for (j = 0; j<group->getNumChildren(); j++)
			{
				fluo::MeshData* md = group->getChild(j)->asMeshData();
				if (md && name == md->getName())
					return md;
			}
		}
	}
	return nullptr;
}

fluo::Annotations* RenderCanvas::GetAnnotations(const std::string &name)
{
	int i;

	for (i = 0; i<GetLayerNum(); i++)
	{
		if (fluo::Annotations* ann = dynamic_cast<fluo::Annotations*>(GetLayer(i)))
		{
			if (name == ann->getName())
				return ann;
		}
	}
	return nullptr;
}

fluo::VolumeGroup* RenderCanvas::GetGroup(const std::string &name)
{
	int i;

	for (i = 0; i< GetLayerNum(); i++)
	{
		if (fluo::VolumeGroup* group = dynamic_cast<fluo::VolumeGroup*>(GetLayer(i)))
		{
			if (name == group->getName())
				return group;
		}
	}
	return nullptr;
}

fluo::VolumeGroup* RenderCanvas::GetGroup(int index)
{
	int i;
	int count = 0;

	for (i = 0; i < GetLayerNum(); i++)
	{
		if (fluo::VolumeGroup* group = dynamic_cast<fluo::VolumeGroup*>(GetLayer(i)))
		{
			if (count == index)
				return group;
			count++;
		}
	}
	return nullptr;
}

fluo::VolumeGroup* RenderCanvas::GetGroup(fluo::VolumeData* vd)
{
	for (int i = 0; i < GetLayerNum(); i++)
	{
		if (fluo::VolumeGroup* group =
			dynamic_cast<fluo::VolumeGroup*>(GetLayer(i)))
		{
			for (int j = 0; j < group->getNumChildren(); j++)
			{
				fluo::VolumeData* tmp_vd = group->getChild(j)->asVolumeData();
				if (tmp_vd && tmp_vd == vd)
					return group;
			}
		}
	}
	return nullptr;
}

int RenderCanvas::GetAny()
{
	PopVolumeList();
	PopMeshList();
	return m_vd_pop_list.size() + m_md_pop_list.size();
}

int RenderCanvas::GetDispVolumeNum()
{
	//get the volume list m_vd_pop_list
	PopVolumeList();

	return m_vd_pop_list.size();
}

int RenderCanvas::GetAllVolumeNum()
{
	int num = 0;
	for (int i = 0; i<GetLayerNum(); i++)
	{
		if (fluo::VolumeData* vd = dynamic_cast<fluo::VolumeData*>(GetLayer(i)))
			num++;
		else if (fluo::VolumeGroup* group = dynamic_cast<fluo::VolumeGroup*>(GetLayer(i)))//group
			num += group->getNumChildren();
	}
	return num;
}

int RenderCanvas::GetMeshNum()
{
	PopMeshList();
	return m_md_pop_list.size();
}

fluo::VolumeGroup* RenderCanvas::AddVolumeData(fluo::VolumeData* vd, const std::string &group_name)
{
	//m_layer_list.push_back(vd);
	int i;
	fluo::VolumeGroup* group = 0;
	fluo::VolumeGroup* group_temp = 0;

	for (i = 0; i<GetLayerNum(); i++)
	{
		//layer is group
		if (group_temp = dynamic_cast<fluo::VolumeGroup*>(GetLayer(i)))
		{
			if (group_name == group_temp->getName())
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
		std::string group_name = AddGroup("");
		group = GetGroup(group_name);
		if (!group)
			return 0;
	}

	/*for (i=0; i<1; i++)
	{
	fluo::VolumeData* vol_data = group->GetVolumeData(i);
	if (vol_data)
	{
	double spcx, spcy, spcz;
	vol_data->GetSpacings(spcx, spcy, spcz);
	vd->SetSpacings(spcx, spcy, spcz);
	}
	}*/

	group->insertChild(group->getNumChildren() - 1, vd);

	if (group && vd)
	{
		//fluo::Color gamma = group->GetGamma();
		//vd->SetGamma(gamma);
		//fluo::Color brightness = group->GetBrightness();
		//vd->SetBrightness(brightness);
		//fluo::Color hdr = group->GetHdr();
		//vd->SetHdr(hdr);
		//bool sync_r = group->GetSyncR();
		//vd->SetSyncR(sync_r);
		//bool sync_g = group->GetSyncG();
		//vd->SetSyncG(sync_g);
		//bool sync_b = group->GetSyncB();
		//vd->SetSyncB(sync_b);

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

void RenderCanvas::AddMeshData(fluo::MeshData* md)
{
	m_layer_list.push_back(md);
	m_md_pop_dirty = true;
}

void RenderCanvas::AddAnnotations(fluo::Annotations* ann)
{
	m_layer_list.push_back(ann);
}

void RenderCanvas::ReplaceVolumeData(const std::string &name, fluo::VolumeData *dst)
{
	int i, j;

	bool found = false;
	fluo::VolumeGroup* group = 0;

	if (!m_frame) return;
	DataManager *dm = m_frame->GetDataManager();
	if (!dm) return;

	for (i = 0; i<GetLayerNum(); i++)
	{
		if (fluo::VolumeData* vd = dynamic_cast<fluo::VolumeData*>(GetLayer(i)))
		{
			if (name == vd->getName())
			{
				if (m_cur_vol == vd) m_cur_vol = dst;
				m_loader.RemoveBrickVD(vd);
				vd->GetRenderer()->clear_tex_current();
				m_layer_list[i] = dst;
				m_vd_pop_dirty = true;
				found = true;
				dm->RemoveVolumeData(name);
				break;
			}
		}
		else if (fluo::VolumeGroup* tmpgroup = dynamic_cast<fluo::VolumeGroup*>(GetLayer(i)))//group
		{
			for (j = 0; j<tmpgroup->getNumChildren(); j++)
			{
				fluo::VolumeData* vd = tmpgroup->getChild(j)->asVolumeData();
				if (vd && name == vd->getName())
				{
					if (m_cur_vol == vd) m_cur_vol = dst;
					m_loader.RemoveBrickVD(vd);
					vd->GetRenderer()->clear_tex_current();
					tmpgroup->replaceChild(j, dst);
					m_vd_pop_dirty = true;
					found = true;
					group = tmpgroup;
					dm->RemoveVolumeData(name);
					break;
				}
			}
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

void RenderCanvas::RemoveVolumeData(const std::string &name)
{
	for (int i = 0; i < GetLayerNum(); i++)
	{
		if (fluo::VolumeData* vd = dynamic_cast<fluo::VolumeData*>(GetLayer(i)))
		{
			if (name == vd->getName())
			{
				m_layer_list.erase(m_layer_list.begin()+i);
				if (m_cur_vol == vd)
					m_cur_vol = 0;
				m_vd_pop_dirty = true;
				return;
			}
		}
		else if (fluo::VolumeGroup* group = dynamic_cast<fluo::VolumeGroup*>(GetLayer(i)))//group
		{
			for (int j = 0; j < group->getNumChildren(); ++j)
			{
				fluo::VolumeData* vd = group->getChild(j)->asVolumeData();
				if (vd && name == vd->getName())
				{
					group->removeChild(j);
					if (m_cur_vol == vd)
						m_cur_vol = 0;
					m_vd_pop_dirty = true;
					return;
				}
			}
		}
	}
}

void RenderCanvas::RemoveVolumeDataDup(const std::string &name)
{
	fluo::VolumeData* vd_main = 0;
	for (int i = 0; i < GetLayerNum() && !vd_main; i++)
	{
		if (fluo::VolumeData* vd = dynamic_cast<fluo::VolumeData*>(GetLayer(i)))
		{
			if (name == vd->getName())
			{
				vd_main = vd;
				m_vd_pop_dirty = true;
			}
		}
		else if (fluo::VolumeGroup* group = dynamic_cast<fluo::VolumeGroup*>(GetLayer(i)))//group
		{
			for (int j = 0; j<group->getNumChildren(); j++)
			{
				fluo::VolumeData* vd = group->getChild(j)->asVolumeData();
				if (vd && name == vd->getName())
				{
					vd_main = vd;
					m_vd_pop_dirty = true;
				}
			}
		}
	}

	if (!vd_main)
		return;
	
	for (int i = 0; i < GetLayerNum();)
	{
		if (!GetLayer(i))
		{
			++i;
			continue;
		}
		if (fluo::VolumeData* vd = dynamic_cast<fluo::VolumeData*>(GetLayer(i)))
		{
			bool del = false;
			if (vd == vd_main)
				del = true;
			bool dup;
			vd->getValue(gstDuplicate, dup);
			if (dup)
			{
				//if (vd->GetDupData() == vd_main)
				//	del = true;
			}
			if (del)
			{
				m_layer_list.erase(m_layer_list.begin() + i);
				if (m_cur_vol == vd)
					m_cur_vol = 0;
			}
			else
				++i;
		}
		else if (fluo::VolumeGroup* group = dynamic_cast<fluo::VolumeGroup*>(GetLayer(i)))//group
		{
			for (int j = group->getNumChildren()-1; j >= 0; --j)
			{
				fluo::VolumeData* vd = group->getChild(j)->asVolumeData();
				if (vd)
				{
					bool del = false;
					bool dup;
					vd->getValue(gstDuplicate, dup);
					if (dup)
					{
						//if (vd->GetDupData() == vd_main)
						//	del = true;
					}
					else
					{
						if (vd == vd_main)
							del = true;
					}
					if (del)
					{
						group->removeChild(j);
						if (m_cur_vol == vd)
							m_cur_vol = 0;
					}
				}
			}
			++i;
		}
		else
			++i;
	}
}

void RenderCanvas::RemoveMeshData(const std::string &name)
{
	int i, j;

	for (i = 0; i<GetLayerNum(); i++)
	{
		if (fluo::MeshData* md = dynamic_cast<fluo::MeshData*>(GetLayer(i)))
		{
			if (name == md->getName())
			{
				m_layer_list.erase(m_layer_list.begin() + i);
				m_md_pop_dirty = true;
				return;
			}
		}
		else if (fluo::MeshGroup* group = dynamic_cast<fluo::MeshGroup*>(GetLayer(i)))//mesh group
		{
			for (j = 0; j<group->getNumChildren(); j++)
			{
				fluo::MeshData* md = group->getChild(j)->asMeshData();
				if (md && name == md->getName())
				{
					group->removeChild(j);
					m_md_pop_dirty = true;
					return;
				}
			}
		}
	}
}

void RenderCanvas::RemoveAnnotations(const std::string &name)
{
	for (int i = 0; i<GetLayerNum(); i++)
	{
		if (fluo::Annotations* ann = dynamic_cast<fluo::Annotations*>(GetLayer(i)))
		{
			if (name == ann->getName())
				m_layer_list.erase(m_layer_list.begin() + i);
		}
	}
}

void RenderCanvas::RemoveGroup(const std::string &name)
{
	int i, j;
	for (i = 0; i<GetLayerNum(); i++)
	{
		if (fluo::VolumeGroup* group = dynamic_cast<fluo::VolumeGroup*>(GetLayer(i)))
		{
			if (name == group->getName())
			{
				for (j = group->getNumChildren() - 1; j >= 0; j--)
				{
					fluo::VolumeData* vd = group->getChild(j)->asVolumeData();
					if (vd)
					{
						group->removeChild(j);
						//if add back to view
					}
				}
				m_layer_list.erase(m_layer_list.begin() + i);
				//delete group;
				m_vd_pop_dirty = true;
			}
		}
		else if (fluo::MeshGroup* group = dynamic_cast<fluo::MeshGroup*>(GetLayer(i)))//mesh group
		{
			if (name == group->getName())
			{
				for (j = group->getNumChildren() - 1; j >= 0; j--)
				{
					fluo::MeshData* md = group->getChild(j)->asMeshData();
					if (md)
					{
						group->removeChild(j);
					}
				}
				m_layer_list.erase(m_layer_list.begin() + i);
				//delete group;
				m_md_pop_dirty = true;
			}
		}
	}
}

//isolate
void RenderCanvas::Isolate(int type, const std::string &name)
{
	for (int i = 0; i < GetLayerNum(); i++)
	{
		fluo::Object* layer = GetLayer(i);
		if (!layer) continue;
		if (name == layer->getName())
			layer->setValue(gstDisplay, true);
		else
			layer->setValue(gstDisplay, false);
	}

	m_vd_pop_dirty = true;
	m_md_pop_dirty = true;
}

//move layer of the same level within this view
//source is after the destination
void RenderCanvas::MoveLayerinView(const std::string &src_name, const std::string &dst_name)
{
	int i, src_index;
	fluo::Object* src = 0;
	for (i = 0; i<GetLayerNum(); i++)
	{
		fluo::Object* layer = GetLayer(i);
		if (layer && src_name == layer->getName())
		{
			src = layer;
			src_index = i;
			m_layer_list.erase(m_layer_list.begin() + i);
			break;
		}
	}
	if (!src)
		return;
	for (i = 0; i<GetLayerNum(); i++)
	{
		fluo::Object* layer = GetLayer(i);
		if (layer && dst_name == layer->getName())
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

void RenderCanvas::ShowAll()
{
	for (unsigned int i = 0; i < GetLayerNum(); ++i)
	{
		fluo::Object* layer = GetLayer(i);
		if (layer)
			layer->setValue(gstDisplay, true);
	}
	m_vd_pop_dirty = true;
	m_md_pop_dirty = true;
}

//move layer (volume) of the same level within the given group
//source is after the destination
void RenderCanvas::MoveLayerinGroup(const std::string &group_name, const std::string &src_name, const std::string &dst_name)
{
	fluo::VolumeGroup* group = GetGroup(group_name);
	if (!group)
		return;

	fluo::VolumeData* src_vd = 0;
	int i, src_index;
	for (i = 0; i<group->getNumChildren(); i++)
	{
		std::string name = group->getChild(i)->getName();
		if (name == src_name)
		{
			src_index = i;
			src_vd = group->getChild(i)->asVolumeData();
			group->removeChild(i);
			break;
		}
	}
	if (!src_vd)
		return;
	for (i = 0; i<group->getNumChildren(); i++)
	{
		std::string name = group->getChild(i)->getName();
		if (name == dst_name)
		{
			if (i >= src_index)
				group->insertChild(i, src_vd);
			else
				group->insertChild(i - 1, src_vd);
			break;
		}
	}
	m_vd_pop_dirty = true;
	m_md_pop_dirty = true;
}

//move layer (volume) from the given group up one level to this view
//source is after the destination
void RenderCanvas::MoveLayertoView(const std::string &group_name, const std::string &src_name, const std::string &dst_name)
{
	fluo::VolumeGroup* group = GetGroup(group_name);
	if (!group)
		return;

	fluo::VolumeData* src_vd = 0;
	int i;
	for (i = 0; i<group->getNumChildren(); i++)
	{
		std::string name = group->getChild(i)->getName();
		if (name == src_name)
		{
			src_vd = group->getChild(i)->asVolumeData();
			group->removeChild(i);
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
		for (i = 0; i<GetLayerNum(); i++)
		{
			std::string name = GetLayer(i)->getName();
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
void RenderCanvas::MoveLayertoGroup(const std::string &group_name, const std::string &src_name, const std::string &dst_name)
{
	fluo::VolumeData* src_vd = 0;
	int i;

	for (i = 0; i<GetLayerNum(); i++)
	{
		if (src_vd = dynamic_cast<fluo::VolumeData*>(GetLayer(i)))
		{
			//is volume data
			if (src_name == src_vd->getName())
			{
				m_layer_list.erase(m_layer_list.begin() + i);
				break;
			}
		}
	}
	fluo::VolumeGroup* group = GetGroup(group_name);
	if (!group || !src_vd)
		return;
	if (group->getNumChildren() == 0 || dst_name == "")
	{
		group->insertChild(0, src_vd);
	}
	else
	{
		for (i = 0; i<group->getNumChildren(); i++)
		{
			std::string name = group->getChild(i)->getName();
			if (name == dst_name)
			{
				group->insertChild(i, src_vd);
				break;
			}
		}
	}

	//set the 2d adjustment settings of the volume the same as the group
	fluo::ValueCollection names{
		gstGammaR, gstGammaG, gstGammaB,
		gstBrightnessR, gstBrightnessG, gstBrightnessB,
		gstEqualizeR, gstEqualizeG, gstEqualizeB,
		gstSyncR, gstSyncG, gstSyncB };
	src_vd->propValues(names, group);

	m_vd_pop_dirty = true;
	m_md_pop_dirty = true;
}

//move layer (volume from one group to another different group
//sourece is after the destination
void RenderCanvas::MoveLayerfromtoGroup(
	const std::string &src_group_name,
	const std::string &dst_group_name,
	const std::string &src_name,
	const std::string &dst_name)
{
	fluo::VolumeGroup* src_group = GetGroup(src_group_name);
	if (!src_group)
		return;
	int i;
	fluo::VolumeData* src_vd = 0;
	for (i = 0; i<src_group->getNumChildren(); i++)
	{
		std::string name = src_group->getChild(i)->getName();
		if (name == src_name)
		{
			src_vd = src_group->getChild(i)->asVolumeData();
			src_group->removeChild(i);
			break;
		}
	}
	fluo::VolumeGroup* dst_group = GetGroup(dst_group_name);
	if (!dst_group || !src_vd)
		return;
	if (dst_group->getNumChildren() == 0 || dst_name == "")
	{
		dst_group->insertChild(0, src_vd);
	}
	else
	{
		for (i = 0; i<dst_group->getNumChildren(); i++)
		{
			std::string name = dst_group->getChild(i)->getName();
			if (name == dst_name)
			{
				dst_group->insertChild(i, src_vd);
				break;
			}
		}
	}

	//reset the sync of the source group
	//src_group->ResetSync();

	//set the 2d adjustment settings of the volume the same as the group
	fluo::ValueCollection names{
		gstGammaR, gstGammaG, gstGammaB,
		gstBrightnessR, gstBrightnessG, gstBrightnessB,
		gstEqualizeR, gstEqualizeG, gstEqualizeB,
		gstSyncR, gstSyncG, gstSyncB };
	src_vd->propValues(names, dst_group);

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
void RenderCanvas::MoveMeshinGroup(const std::string &group_name,
	const std::string &src_name, const std::string &dst_name)
{
	fluo::MeshGroup* group = GetMGroup(group_name);
	if (!group)
		return;

	fluo::MeshData* src_md = 0;
	int i, src_index;
	for (i = 0; i<group->getNumChildren(); i++)
	{
		std::string name = group->getChild(i)->getName();
		if (name == src_name)
		{
			src_index = i;
			src_md = group->getChild(i)->asMeshData();
			group->removeChild(i);
			break;
		}
	}
	if (!src_md)
		return;
	for (i = 0; i<group->getNumChildren(); i++)
	{
		std::string name = group->getChild(i)->getName();
		if (name == dst_name)
		{
			if (i >= src_index)
				group->insertChild(i, src_md);
			else
				group->insertChild(i - 1, src_md);
			break;
		}
	}

	m_vd_pop_dirty = true;
	m_md_pop_dirty = true;
}

//move mesh out of a group
void RenderCanvas::MoveMeshtoView(const std::string &group_name,
	const std::string &src_name, const std::string &dst_name)
{
	fluo::MeshGroup* group = GetMGroup(group_name);
	if (!group)
		return;

	fluo::MeshData* src_md = 0;
	int i;
	for (i = 0; i<group->getNumChildren(); i++)
	{
		std::string name = group->getChild(i)->getName();
		if (name == src_name)
		{
			src_md = group->getChild(i)->asMeshData();
			group->removeChild(i);
			break;
		}
	}
	if (!src_md)
		return;
	if (dst_name == "")
		m_layer_list.push_back(src_md);
	else
	{
		for (i = 0; i<GetLayerNum(); i++)
		{
			std::string name = GetLayer(i)->getName();
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
void RenderCanvas::MoveMeshtoGroup(const std::string &group_name,
	const std::string &src_name, const std::string &dst_name)
{
	fluo::MeshData* src_md = 0;
	int i;

	for (i = 0; i<GetLayerNum(); i++)
	{
		std::string name = GetLayer(i)->getName();
		if (name != src_name) continue;
		if (src_md = dynamic_cast<fluo::MeshData*>(GetLayer(i)))
		{
			m_layer_list.erase(m_layer_list.begin() + i);
			break;
		}
	}
	fluo::MeshGroup* group = GetMGroup(group_name);
	if (!group || !src_md)
		return;
	if (group->getNumChildren() == 0 || dst_name == "")
		group->insertChild(0, src_md);
	else
	{
		for (i = 0; i<group->getNumChildren(); i++)
		{
			std::string name = group->getChild(i)->getName();
			if (name == dst_name)
			{
				group->insertChild(i, src_md);
				break;
			}
		}
	}
	m_vd_pop_dirty = true;
	m_md_pop_dirty = true;
}

//move mesh out of then into a group
void RenderCanvas::MoveMeshfromtoGroup(
	const std::string &src_group_name,
	const std::string &dst_group_name,
	const std::string &src_name,
	const std::string &dst_name)
{
	fluo::MeshGroup* src_group = GetMGroup(src_group_name);
	if (!src_group)
		return;
	int i;
	fluo::MeshData* src_md = 0;
	for (i = 0; i<src_group->getNumChildren(); i++)
	{
		std::string name = src_group->getChild(i)->getName();
		if (name == src_name)
		{
			src_md = src_group->getChild(i)->asMeshData();
			src_group->removeChild(i);
			break;
		}
	}
	fluo::MeshGroup* dst_group = GetMGroup(dst_group_name);
	if (!dst_group || !src_md)
		return;
	if (dst_group->getNumChildren() == 0 || dst_name == "")
		dst_group->insertChild(0, src_md);
	else
	{
		for (i = 0; i<dst_group->getNumChildren(); i++)
		{
			std::string name = dst_group->getChild(i)->getName();
			if (name == dst_name)
			{
				dst_group->insertChild(i, src_md);
				break;
			}
		}
	}
	m_vd_pop_dirty = true;
	m_md_pop_dirty = true;
}

//layer control
int RenderCanvas::GetGroupNum()
{
	int group_num = 0;

	for (int i = 0; i<GetLayerNum(); i++)
	{
		fluo::VolumeGroup *group = dynamic_cast<fluo::VolumeGroup*>(GetLayer(i));
		if (group)
			group_num++;
	}
	return group_num;
}

int RenderCanvas::GetLayerNum()
{
	return m_layer_list.size();
}

fluo::Object* RenderCanvas::GetLayer(int index)
{
	if (index >= 0 && index<(int)m_layer_list.size())
		return m_layer_list[index].get();
	else
		return nullptr;
}

std::string RenderCanvas::AddGroup(const std::string &str, const std::string &prev_group)
{
	fluo::VolumeGroup* group = glbin_volf->buildGroup();
	if (group && str != "")
		group->setName(str);

	bool found_prev = false;
	for (int i = 0; i<GetLayerNum(); i++)
	{
		if (fluo::VolumeGroup* group_temp = dynamic_cast<fluo::VolumeGroup*>(GetLayer(i)))
		{
			if (prev_group == group_temp->getName())
			{
				m_layer_list.insert(m_layer_list.begin() + i + 1, group);
				found_prev = true;
			}
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
			group->setValue(gstGammaR, gamma.r());
			group->setValue(gstGammaG, gamma.g());
			group->setValue(gstGammaB, gamma.b());
			group->setValue(gstBrightnessR, brightness.r());
			group->setValue(gstBrightnessG, brightness.g());
			group->setValue(gstBrightnessB, brightness.b());
			group->setValue(gstEqualizeR, hdr.r());
			group->setValue(gstEqualizeG, hdr.g());
			group->setValue(gstEqualizeB, hdr.b());
			group->setValue(gstSyncR, sync_r);
			group->setValue(gstSyncG, sync_g);
			group->setValue(gstSyncB, sync_b);
		}
	}

	if (group)
		return group->getName();
	else
		return "Group";
}

fluo::VolumeGroup* RenderCanvas::AddOrGetGroup()
{
	for (int i = 0; i < GetLayerNum(); i++)
	{
		if (fluo::VolumeGroup* group_temp = dynamic_cast<fluo::VolumeGroup*>(GetLayer(i)))
		{
			if (!group_temp->getNumChildren())
				return group_temp;
		}
	}
	//group not found
	fluo::VolumeGroup* group = glbin_volf->buildGroup();
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
			group->setValue(gstGammaR, gamma.r());
			group->setValue(gstGammaG, gamma.g());
			group->setValue(gstGammaB, gamma.b());
			group->setValue(gstBrightnessR, brightness.r());
			group->setValue(gstBrightnessG, brightness.g());
			group->setValue(gstBrightnessB, brightness.b());
			group->setValue(gstEqualizeR, hdr.r());
			group->setValue(gstEqualizeG, hdr.g());
			group->setValue(gstEqualizeB, hdr.b());
			group->setValue(gstSyncR, sync_r);
			group->setValue(gstSyncG, sync_g);
			group->setValue(gstSyncB, sync_b);
		}
	}
	m_layer_list.push_back(group);
	return group;
}

std::string RenderCanvas::AddMGroup(const std::string &str)
{
	fluo::MeshGroup* group = glbin_mshf->buildGroup();
	if (group && str != "")
		group->setName(str);
	m_layer_list.push_back(group);

	if (group)
		return group->getName();
	else
		return "";
}

fluo::MeshGroup* RenderCanvas::AddOrGetMGroup()
{
	for (int i = 0; i < GetLayerNum(); i++)
	{
		if (fluo::MeshGroup* group_temp = dynamic_cast<fluo::MeshGroup*>(GetLayer(i)))
		{
			if (group_temp && !group_temp->getNumChildren())
				return group_temp;
		}
	}
	//group not found
	fluo::MeshGroup* group = glbin_mshf->buildGroup();
	if (!group)
		return 0;
	m_layer_list.push_back(group);
	return group;
}

fluo::MeshGroup* RenderCanvas::GetMGroup(const std::string &str)
{
	int i;

	for (i = 0; i<GetLayerNum(); i++)
	{
		if (fluo::MeshGroup* group = dynamic_cast<fluo::MeshGroup*>(GetLayer(i)))
		{
			if (group && str == group->getName())
				return group;
		}
	}
	return 0;
}

//init
void RenderCanvas::InitView(unsigned int type)
{
	int i;

	if (type&INIT_BOUNDS)
	{
		m_bounds.reset();
		PopVolumeList();
		PopMeshList();

		fluo::BBox bounds;
		for (i = 0; i < (int)m_vd_pop_list.size(); i++)
		{
			m_vd_pop_list[i]->getValue(gstBounds, bounds);
			m_bounds.extend(bounds);
		}
		for (i = 0; i < (int)m_md_pop_list.size(); i++)
		{
			fluo::BBox b;
			m_md_pop_list[i]->getValue(gstBounds, b);
			m_bounds.extend(b);
		}

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

void RenderCanvas::DrawBounds()
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

void RenderCanvas::DrawClippingPlanes(bool border, int face_winding)
{
	int i;
	bool link = false;
	PLANE_MODES plane_mode = kNormal;
	if (m_frame && m_frame->GetClippingView())
	{
		link = m_frame->GetClippingView()->GetChannLink();
		plane_mode = m_frame->GetClippingView()->GetPlaneMode();
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

	flvr::ShaderProgram* shader =
		flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHDR_DRAW_GEOMETRY);
	if (shader)
	{
		if (!shader->valid())
			shader->create();
		shader->bind();
	}

	for (i = 0; i<GetDispVolumeNum(); i++)
	{
		fluo::VolumeData* vd = GetDispVolumeData(i);
		if (!vd)
			continue;

		if (vd != m_cur_vol)
			continue;

		flvr::VolumeRenderer *vr = vd->GetRenderer();
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
				vd->getValue(gstColor, color);
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
		vd->getValue(gstScaleX, sclx);
		vd->getValue(gstScaleY, scly);
		vd->getValue(gstScaleZ, sclz);
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
					shader->setLocalParam(0, 1.0, 0.5, 0.5, plane_trans);
				else
					shader->setLocalParam(0, color.r(), color.g(), color.b(), plane_trans);
				va_clipp->draw_clip_plane(0, false);
			}
			if (border)
			{
				shader->setLocalParam(0, color.r(), color.g(), color.b(), plane_trans);
				va_clipp->draw_clip_plane(16, true);
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
				va_clipp->draw_clip_plane(32, false);
			}
			if (border)
			{
				shader->setLocalParam(0, color.r(), color.g(), color.b(), plane_trans);
				va_clipp->draw_clip_plane(48, true);
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
				va_clipp->draw_clip_plane(64, false);
			}
			if (border)
			{
				shader->setLocalParam(0, color.r(), color.g(), color.b(), plane_trans);
				va_clipp->draw_clip_plane(80, true);
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
				va_clipp->draw_clip_plane(96, false);
			}
			if (border)
			{
				shader->setLocalParam(0, color.r(), color.g(), color.b(), plane_trans);
				va_clipp->draw_clip_plane(112, true);
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
				va_clipp->draw_clip_plane(128, false);
			}
			if (border)
			{
				shader->setLocalParam(0, color.r(), color.g(), color.b(), plane_trans);
				va_clipp->draw_clip_plane(144, true);
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
				va_clipp->draw_clip_plane(160, false);
			}
			if (border)
			{
				shader->setLocalParam(0, color.r(), color.g(), color.b(), plane_trans);
				va_clipp->draw_clip_plane(176, true);
			}
		}
		va_clipp->draw_end();
	}

	if (shader && shader->valid())
		shader->release();

	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
}

void RenderCanvas::DrawGrid()
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

void RenderCanvas::DrawCamCtr()
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

void RenderCanvas::DrawFrame()
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

void RenderCanvas::DrawScaleBar()
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

void RenderCanvas::DrawLegend()
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

		if (!m_vd_pop_list[i]) continue;
		bool legend;
		m_vd_pop_list[i]->getValue(gstLegend, legend);
		if (legend)
		{
			wxstr = m_vd_pop_list[i]->getName();
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
		if (!m_md_pop_list[i]) continue;
		bool legend;
		m_md_pop_list[i]->getValue(gstLegend, legend);
		if (legend)
		{
			wxstr = m_md_pop_list[i]->getName();
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
		if (!m_vd_pop_list[i]) continue;
		bool legend;
		m_vd_pop_list[i]->getValue(gstLegend, legend);
		if (legend)
		{
			wxstr = m_vd_pop_list[i]->getName();
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
				wxstr == m_frame->GetCurSelVol()->getName())
				highlighted = true;
			fluo::Color color;
			m_vd_pop_list[i]->getValue(gstColor, color);
			DrawName(xpos + xoffset, ny - (lines - cur_line + 0.1)*font_height - yoffset,
				nx, ny, wxstr, color, font_height, highlighted);
		}
	}
	for (i = 0; i<(int)m_md_pop_list.size(); i++)
	{
		if (!m_md_pop_list[i]) continue;
		bool legend;
		m_md_pop_list[i]->getValue(gstLegend, legend);
		if (legend)
		{
			wxstr = m_md_pop_list[i]->getName();
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
			fluo::Color color;
			m_md_pop_list[i]->getValue(gstColor, color);
			bool highlighted = false;
			if (m_frame->GetCurSelType() == 3 &&
				m_frame->GetCurSelMesh() &&
				wxstr == m_frame->GetCurSelMesh()->getName())
				highlighted = true;
			DrawName(xpos + xoffset, ny - (lines - cur_line + 0.1)*font_height - yoffset,
				nx, ny, wxstr, color, font_height, highlighted);
		}
	}

	m_sb_height = (lines + 1)*font_height;
}

void RenderCanvas::DrawName(
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

void RenderCanvas::DrawGradBg()
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

void RenderCanvas::SetColormapColors(int colormap, fluo::Color &c, double inv)
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

void RenderCanvas::DrawColormap()
{
	double max_val = 255.0;
	bool enable_alpha = false;

	if (!m_cur_vol) return;
	long colormap_mode;
	m_cur_vol->getValue(gstColormapMode, colormap_mode);
	if (colormap_mode)
	{
		double low, high;
		m_cur_vol->getValue(gstColormapLow, low);
		m_cur_vol->getValue(gstColormapHigh, high);
		m_value_2 = low;
		m_value_6 = high;
		m_value_4 = (low + high) / 2.0;
		m_value_3 = (low + m_value_4) / 2.0;
		m_value_5 = (m_value_4 + high) / 2.0;
		m_cur_vol->getValue(gstMaxInt, max_val);
		m_cur_vol->getValue(gstAlphaEnable, enable_alpha);
		fluo::Color vd_color;
		m_cur_vol->getValue(gstColor, vd_color);
		bool colormap_inv;
		m_cur_vol->getValue(gstColormapInv, colormap_inv);
		SetColormapColors(colormap_mode, vd_color, colormap_inv);
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

void RenderCanvas::DrawInfo(int nx, int ny)
{
	float sx, sy;
	sx = 2.0 / nx;
	sy = 2.0 / ny;
	float px, py;
	float gapw = flvr::TextRenderer::text_texture_manager_.GetSize();
	float gaph = gapw * 2;

	double fps_ = 1.0 / glbin_timer->average();
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
			long resx, resy, resz;
			m_cur_vol->getValue(gstResX, resx);
			m_cur_vol->getValue(gstResY, resy);
			m_cur_vol->getValue(gstResZ, resz);
			double spcx, spcy, spcz;
			m_cur_vol->getValue(gstSpcX, spcx);
			m_cur_vol->getValue(gstSpcY, spcy);
			m_cur_vol->getValue(gstSpcZ, spcz);
			vector<fluo::Plane*> *planes = m_cur_vol->GetRenderer()->get_planes();
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
				fluo::VolumeData* vd = m_vd_pop_list[i];
				if (vd && vd->GetRenderer())
				{
					str = wxString::Format("SLICES_%d: %d", i + 1, vd->GetRenderer()->get_slice_num());
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

fluo::Quaternion RenderCanvas::Trackball(double dx, double dy)
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

fluo::Quaternion RenderCanvas::TrackballClip(int p1x, int p1y, int p2x, int p2y)
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

void RenderCanvas::UpdateClips()
{
	if (m_clip_mode == 1)
		m_q_cl.FromEuler(m_rotx, -m_roty, -m_rotz);

	vector<fluo::Plane*> *planes = 0;
	for (int i = 0; i < (int)m_vd_pop_list.size(); i++)
	{
		if (!m_vd_pop_list[i])
			continue;

		double spcx, spcy, spcz;
		long resx, resy, resz;
		m_vd_pop_list[i]->getValue(gstSpcX, spcx);
		m_vd_pop_list[i]->getValue(gstSpcY, spcy);
		m_vd_pop_list[i]->getValue(gstSpcZ, spcz);
		m_vd_pop_list[i]->getValue(gstResX, resx);
		m_vd_pop_list[i]->getValue(gstResY, resy);
		m_vd_pop_list[i]->getValue(gstResZ, resz);
		fluo::Vector scale;
		if (spcx > 0.0 && spcy > 0.0 && spcz > 0.0)
		{
			scale = fluo::Vector(1.0 / resx / spcx, 1.0 / resy / spcy, 1.0 / resz / spcz);
			scale.safe_normalize();
		}
		else
			scale = fluo::Vector(1.0, 1.0, 1.0);

		if (m_vd_pop_list[i]->GetRenderer())
			planes = m_vd_pop_list[i]->GetRenderer()->get_planes();
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

void RenderCanvas::Q2A()
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

void RenderCanvas::A2Q()
{
	//view changed, re-sort bricks
	//SetSortBricks();

	m_q.FromEuler(m_rotx, m_roty, m_rotz);
	m_q = m_q * m_zq;

	if (m_clip_mode)
		UpdateClips();
}

//sort bricks after view changes
void RenderCanvas::SetSortBricks()
{
	PopVolumeList();

	for (int i = 0; i<(int)m_vd_pop_list.size(); i++)
	{
		fluo::VolumeData* vd = m_vd_pop_list[i];
		if (vd && vd->GetTexture())
			vd->GetTexture()->set_sort_bricks();
	}
}

void RenderCanvas::SetClipMode(int mode)
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

void RenderCanvas::RestorePlanes()
{
	vector<fluo::Plane*> *planes = 0;
	for (int i = 0; i<(int)m_vd_pop_list.size(); i++)
	{
		if (!m_vd_pop_list[i])
			continue;

		planes = 0;
		if (m_vd_pop_list[i]->GetRenderer())
			planes = m_vd_pop_list[i]->GetRenderer()->get_planes();
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

void RenderCanvas::SetClippingPlaneRotations(double rotx, double roty, double rotz)
{
	m_rotx_cl = -rotx;
	m_roty_cl = roty;
	m_rotz_cl = rotz;

	m_q_cl.FromEuler(m_rotx_cl, m_roty_cl, m_rotz_cl);
	m_q_cl.Normalize();

	SetRotations(m_rotx, m_roty, m_rotz);
}

void RenderCanvas::GetClippingPlaneRotations(double &rotx, double &roty, double &rotz)
{
	rotx = -m_rotx_cl;
	roty = m_roty_cl;
	rotz = m_rotz_cl;
}

//interpolation
void RenderCanvas::SetIntp(bool mode)
{
	m_intp = mode;
}

bool RenderCanvas::GetIntp()
{
	return m_intp;
}

//start loop update
void RenderCanvas::StartLoopUpdate()
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
			fluo::VolumeData* vd = m_vd_pop_list[i];
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
					vd->GetRenderer()->m_mv_mat2 = glm::mat4(
						mvmat[0], mvmat[4], mvmat[8], mvmat[12],
						mvmat[1], mvmat[5], mvmat[9], mvmat[13],
						mvmat[2], mvmat[6], mvmat[10], mvmat[14],
						mvmat[3], mvmat[7], mvmat[11], mvmat[15]);
					vd->GetRenderer()->m_mv_mat2 = vd->GetRenderer()->m_mv_mat * vd->GetRenderer()->m_mv_mat2;

					fluo::Ray view_ray = vd->GetRenderer()->compute_view();
					vector<flvr::TextureBrick*> *bricks = 0;
					bricks = tex->get_sorted_bricks(view_ray, !m_persp);
					if (!bricks || bricks->size() == 0)
						continue;
					for (j = 0; j<bricks->size(); j++)
					{
						(*bricks)[j]->set_drawn(false);
						if ((*bricks)[j]->get_priority() > 0 ||
							!vd->GetRenderer()->test_against_view((*bricks)[j]->bbox(), m_persp))
						{
							(*bricks)[j]->set_disp(false);
							continue;
						}
						else
							(*bricks)[j]->set_disp(true);
						total_num++;
						num_chan++;
						long mip_mode;
						vd->getValue(gstMipMode, mip_mode);
						bool shading;
						vd->getValue(gstShadingEnable, shading);
						if (mip_mode == 1 && shading)
							total_num++;
						bool shadow;
						vd->getValue(gstShadowEnable, shadow);
						if (shadow)
							total_num++;
						//mask
						long label_mode;
						vd->getValue(gstLabelMode, label_mode);
						if (vd->GetTexture() &&
							vd->GetTexture()->nmask() != -1 &&
							(!label_mode ||
							(label_mode &&
							vd->GetTexture()->nlabel() == -1)))
							total_num++;
					}
				}
				vd->setValue(gstBrickNum, long(num_chan));
				if (vd->GetRenderer())
					vd->GetRenderer()->set_done_loop(false);
			}
		}

		vector<VolumeLoaderData> queues;
		if (m_vol_method == VOL_METHOD_MULTI)
		{
			vector<fluo::VolumeData*> list;
			for (i = 0; i<m_vd_pop_list.size(); i++)
			{
				fluo::VolumeData* vd = m_vd_pop_list[i];
				bool disp, multires;
				vd->getValue(gstDisplay, disp);
				vd->getValue(gstMultires, multires);
				if (!vd || !disp || !multires)
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
				fluo::VolumeData* vd = list[i];
				flvr::Texture* tex = vd->GetTexture();
				fluo::Ray view_ray = vd->GetRenderer()->compute_view();
				vector<flvr::TextureBrick*> *bricks = tex->get_sorted_bricks(view_ray, !m_persp);
				long mip_mode;
				vd->getValue(gstMipMode, mip_mode);
				int mode = mip_mode == 1 ? 1 : 0;
				bool shading;
				vd->getValue(gstShadingEnable, shading);
				bool shade = (mode == 1 && shading);
				bool shadow;
				vd->getValue(gstShadowEnable, shadow);
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
				std::sort(queues.begin(), queues.end(),
				[](const VolumeLoaderData b1, const VolumeLoaderData b2)
				{ return b2.brick->get_d() > b1.brick->get_d(); });
			else if (flvr::TextureRenderer::get_update_order() == 0)
				std::sort(queues.begin(), queues.end(),
				[](const VolumeLoaderData b1, const VolumeLoaderData b2)
				{ return b2.brick->get_d() < b1.brick->get_d(); });

			if (!tmp_shade.empty())
			{
				if (flvr::TextureRenderer::get_update_order() == 1)
					std::sort(tmp_shade.begin(), tmp_shade.end(),
					[](const VolumeLoaderData b1, const VolumeLoaderData b2)
					{ return b2.brick->get_d() > b1.brick->get_d(); });
				else if (flvr::TextureRenderer::get_update_order() == 0)
					std::sort(tmp_shade.begin(), tmp_shade.end(),
					[](const VolumeLoaderData b1, const VolumeLoaderData b2)
					{ return b2.brick->get_d() < b1.brick->get_d(); });
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
						fluo::Ray view_ray = list[i]->GetRenderer()->compute_view();
						list[i]->GetTexture()->set_sort_bricks();
						list[i]->GetTexture()->get_sorted_bricks(view_ray, !m_persp); //recalculate brick.d_
						list[i]->GetTexture()->set_sort_bricks();
					}
					flvr::TextureRenderer::set_update_order(order);
					std::sort(tmp_shadow.begin(), tmp_shadow.end(),
						[](const VolumeLoaderData b1, const VolumeLoaderData b2)
						{ return b2.brick->get_d() > b1.brick->get_d(); });
				}
				else if (flvr::TextureRenderer::get_update_order() == 0)
					std::sort(tmp_shadow.begin(), tmp_shadow.end(),
						[](const VolumeLoaderData b1, const VolumeLoaderData b2)
					{ return b2.brick->get_d() > b1.brick->get_d(); });
				queues.insert(queues.end(), tmp_shadow.begin(), tmp_shadow.end());
			}
		}
		else if (GetLayerNum() > 0)
		{
			for (i = GetLayerNum() - 1; i >= 0; i--)
			{
				if (fluo::VolumeData* vd = dynamic_cast<fluo::VolumeData*>(GetLayer(i)))
				{
					vector<VolumeLoaderData> tmp_shade;
					vector<VolumeLoaderData> tmp_shadow;
					bool disp, multires;
					vd->getValue(gstDisplay, disp);
					vd->getValue(gstMultires, multires);
					if (disp && multires)
					{
						flvr::Texture* tex = vd->GetTexture();
						if (!tex)
							continue;
						fluo::Ray view_ray = vd->GetRenderer()->compute_view();
						vector<flvr::TextureBrick*> *bricks = tex->get_sorted_bricks(view_ray, !m_persp);
						if (!bricks || bricks->size() == 0)
							continue;
						long mip_mode;
						vd->getValue(gstMipMode, mip_mode);
						int mode = mip_mode == 1 ? 1 : 0;
						bool shading;
						vd->getValue(gstShadingEnable, shading);
						bool shade = (mode == 1 && shading);
						bool shadow;
						vd->getValue(gstShadowEnable, shadow);
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
								fluo::Ray view_ray = vd->GetRenderer()->compute_view();
								tex->set_sort_bricks();
								tex->get_sorted_bricks(view_ray, !m_persp); //recalculate brick.d_
								tex->set_sort_bricks();
								flvr::TextureRenderer::set_update_order(order);
								std::sort(tmp_shadow.begin(), tmp_shadow.end(),
									[](const VolumeLoaderData b1, const VolumeLoaderData b2)
									{ return b2.brick->get_d() > b1.brick->get_d(); });
							}
							queues.insert(queues.end(), tmp_shadow.begin(), tmp_shadow.end());
						}
					}
				}
				else if (fluo::VolumeGroup* group = dynamic_cast<fluo::VolumeGroup*>(GetLayer(i)))//group
				{
					vector<fluo::VolumeData*> list;
					bool disp;
					group->getValue(gstDisplay, disp);
					if (!disp)
						continue;
					for (j = group->getNumChildren() - 1; j >= 0; j--)
					{
						fluo::VolumeData* vd = group->getChild(j)->asVolumeData();
						bool disp, multires;
						vd->getValue(gstDisplay, disp);
						vd->getValue(gstMultires, multires);
						if (!vd || !disp || !multires)
							continue;
						flvr::Texture* tex = vd->GetTexture();
						if (!tex)
							continue;
						fluo::Ray view_ray = vd->GetRenderer()->compute_view();
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
					long blend_mode;
					group->getValue(gstBlendMode, blend_mode);
					if (blend_mode == VOL_METHOD_MULTI)
					{
						for (k = 0; k < list.size(); k++)
						{
							fluo::VolumeData* vd = list[k];
							flvr::Texture* tex = vd->GetTexture();
							fluo::Ray view_ray = vd->GetRenderer()->compute_view();
							vector<flvr::TextureBrick*> *bricks = tex->get_sorted_bricks(view_ray, !m_persp);
							long mip_mode;
							vd->getValue(gstMipMode, mip_mode);
							int mode = mip_mode == 1 ? 1 : 0;
							bool shading;
							vd->getValue(gstShadingEnable, shading);
							bool shade = (mode == 1 && shading);
							bool shadow;
							vd->getValue(gstShadowEnable, shadow);
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
								std::sort(tmp_q.begin(), tmp_q.end(),
								[](const VolumeLoaderData b1, const VolumeLoaderData b2)
								{ return b2.brick->get_d() > b1.brick->get_d(); });
							else if (flvr::TextureRenderer::get_update_order() == 0)
								std::sort(tmp_q.begin(), tmp_q.end(),
								[](const VolumeLoaderData b1, const VolumeLoaderData b2)
								{ return b2.brick->get_d() < b1.brick->get_d(); });
							queues.insert(queues.end(), tmp_q.begin(), tmp_q.end());
						}
						if (!tmp_shade.empty())
						{
							if (flvr::TextureRenderer::get_update_order() == 1)
								std::sort(tmp_shade.begin(), tmp_shade.end(),
								[](const VolumeLoaderData b1, const VolumeLoaderData b2)
								{ return b2.brick->get_d() > b1.brick->get_d(); });
							else if (flvr::TextureRenderer::get_update_order() == 0)
								std::sort(tmp_shade.begin(), tmp_shade.end(),
								[](const VolumeLoaderData b1, const VolumeLoaderData b2)
								{ return b2.brick->get_d() < b1.brick->get_d(); });
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
									fluo::Ray view_ray = list[k]->GetRenderer()->compute_view();
									list[i]->GetTexture()->set_sort_bricks();
									list[i]->GetTexture()->get_sorted_bricks(view_ray, !m_persp); //recalculate brick.d_
									list[i]->GetTexture()->set_sort_bricks();
								}
								flvr::TextureRenderer::set_update_order(order);
								std::sort(tmp_shadow.begin(), tmp_shadow.end(),
									[](const VolumeLoaderData b1, const VolumeLoaderData b2)
									{ return b2.brick->get_d() > b1.brick->get_d(); });
							}
							else if (flvr::TextureRenderer::get_update_order() == 0)
								std::sort(tmp_shadow.begin(), tmp_shadow.end(),
									[](const VolumeLoaderData b1, const VolumeLoaderData b2)
									{ return b2.brick->get_d() > b1.brick->get_d(); });
							queues.insert(queues.end(), tmp_shadow.begin(), tmp_shadow.end());
						}
					}
					else
					{
						for (j = 0; j < list.size(); j++)
						{
							fluo::VolumeData* vd = list[j];
							flvr::Texture* tex = vd->GetTexture();
							fluo::Ray view_ray = vd->GetRenderer()->compute_view();
							vector<flvr::TextureBrick*> *bricks = tex->get_sorted_bricks(view_ray, !m_persp);
							long mip_mode;
							vd->getValue(gstMipMode, mip_mode);
							int mode = mip_mode == 1 ? 1 : 0;
							bool shading;
							vd->getValue(gstShadingEnable, shading);
							bool shade = (mode == 1 && shading);
							bool shadow;
							vd->getValue(gstShadowEnable, shadow);
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
									fluo::Ray view_ray = vd->GetRenderer()->compute_view();
									tex->set_sort_bricks();
									tex->get_sorted_bricks(view_ray, !m_persp); //recalculate brick.d_
									tex->set_sort_bricks();
									flvr::TextureRenderer::set_update_order(order);
									std::sort(tmp_shadow.begin(), tmp_shadow.end(),
										[](const VolumeLoaderData b1, const VolumeLoaderData b2)
										{ return b2.brick->get_d() > b1.brick->get_d(); });
								}
								queues.insert(queues.end(), tmp_shadow.begin(), tmp_shadow.end());
							}
						}
					}

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
void RenderCanvas::HaltLoopUpdate()
{
	if (flvr::TextureRenderer::get_mem_swap())
	{
		flvr::TextureRenderer::reset_update_loop();
	}
}

//new function to refresh
void RenderCanvas::RefreshGL(int debug_code,
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

void RenderCanvas::DrawRulers()
{
	if (m_ruler_list.empty())
		return;
	double width = 1.0;
	if (m_frame && m_frame->GetSettingDlg())
		width = m_frame->GetSettingDlg()->GetLineWidth();
	m_ruler_renderer.SetLineSize(width);
	m_ruler_renderer.Draw();
}

flrd::RulerList* RenderCanvas::GetRulerList()
{
	return &m_ruler_list;
}

flrd::Ruler* RenderCanvas::GetRuler(unsigned int id)
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
void RenderCanvas::DrawCells()
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

unsigned int RenderCanvas::DrawCellVerts(vector<float>& verts)
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

void RenderCanvas::GetCellPoints(fluo::BBox& box,
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
flrd::Tracks* RenderCanvas::GetTraceGroup()
{
	return m_trace_group;
}

void RenderCanvas::CreateTraceGroup()
{
	if (m_trace_group)
		delete m_trace_group;

	m_trace_group = new flrd::Tracks;
}

int RenderCanvas::LoadTraceGroup(wxString filename)
{
	if (m_trace_group)
		delete m_trace_group;

	m_trace_group = new flrd::Tracks;
	return m_trace_group->LoadData(filename.ToStdWstring());
}

int RenderCanvas::SaveTraceGroup(wxString filename)
{
	if (m_trace_group)
		return m_trace_group->SaveData(filename.ToStdWstring());
	else
		return 0;
}

void RenderCanvas::ExportTrace(wxString filename, unsigned int id)
{
	if (!m_trace_group)
		return;
}

void RenderCanvas::DrawTraces()
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
		m_cur_vol->getValue(gstSpcX, spcx);
		m_cur_vol->getValue(gstSpcY, spcy);
		m_cur_vol->getValue(gstSpcZ, spcz);
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

void RenderCanvas::GetTraces(bool update)
{
	if (!m_trace_group)
		return;

	int ii, jj, kk;
	long nx, ny, nz;
	//return current mask (into system memory)
	if (!m_cur_vol) return;
	m_cur_vol->GetRenderer()->return_mask();
	m_cur_vol->getValue(gstResX, nx);
	m_cur_vol->getValue(gstResY, ny);
	m_cur_vol->getValue(gstResZ, nz);
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
WXLRESULT RenderCanvas::MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam)
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

void RenderCanvas::OnMouse(wxMouseEvent& event)
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
		bool found_rp = false;
		if (m_int_mode == 6 ||
			m_int_mode == 9 ||
			m_int_mode == 11 ||
			m_int_mode == 14)
		{
			found_rp = m_ruler_handler.FindEditingRuler(
				event.GetX(), event.GetY());
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
			m_ruler_handler.AddRulerPoint(event.GetX(), event.GetY(), true);
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
				m_ruler_handler.AddRulerPoint(event.GetX(), event.GetY(), true);
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
				m_ruler_handler.AddRulerPoint(event.GetX(), event.GetY(), true);
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
			int(sqrt(double(old_mouse_X - event.GetX())*
				double(old_mouse_X - event.GetX()) +
				double(old_mouse_Y - event.GetY())*
				double(old_mouse_Y - event.GetY()))));

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
				abs(old_mouse_X - event.GetX()) +
				abs(old_mouse_Y - event.GetY())<200)
			{
				if (event.LeftIsDown() &&
					!event.ControlDown() &&
					m_int_mode != 10 &&
					m_int_mode != 12)
				{
					fluo::Quaternion q_delta = Trackball(
						event.GetX() - old_mouse_X, old_mouse_Y - event.GetY());
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
					long dx = event.GetX() - old_mouse_X;
					long dy = event.GetY() - old_mouse_Y;

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
				abs(old_mouse_X - event.GetX()) +
				abs(old_mouse_Y - event.GetY())<200)
			{
				if (event.LeftIsDown())
				{
					fluo::Quaternion q_delta = TrackballClip(old_mouse_X, event.GetY(), event.GetX(), old_mouse_Y);
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
					event.GetX(), event.GetY(), event.AltDown());
			else if (m_int_mode == 9)
				rval = m_ruler_handler.MoveRuler(
					event.GetX(), event.GetY());
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
			if (m_ruler_handler.GetMouseDist(event.GetX(), event.GetY(), 35))
			{
				//add one point to a ruler
				m_ruler_handler.AddRulerPoint(event.GetX(), event.GetY(), true);
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

void RenderCanvas::SetDraw(bool draw)
{
	m_draw_all = draw;
}

void RenderCanvas::ToggleDraw()
{
	m_draw_all = !m_draw_all;
}

bool RenderCanvas::GetDraw()
{
	return m_draw_all;
}

fluo::Color RenderCanvas::GetBackgroundColor()
{
	return m_bg_color;
}

fluo::Color RenderCanvas::GetTextColor()
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
		{
			fluo::Color color;
			m_cur_vol->getValue(gstSecColor, color);
			return color;
		}
		else
			return m_bg_color_inv;
	}
	return m_bg_color_inv;
}

void RenderCanvas::SetBackgroundColor(fluo::Color &color)
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

void RenderCanvas::SetFog(bool b)
{
	m_use_fog = b;
	if (m_vrv)
		m_vrv->m_left_toolbar->ToggleTool(VRenderView::ID_DepthAttenChk, b);
}

void RenderCanvas::SetRotations(double rotx, double roty, double rotz, bool ui_update)
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

void RenderCanvas::SetZeroRotations()
{
	m_zq = m_q;
}

void RenderCanvas::ResetZeroRotations(double &rotx, double &roty, double &rotz)
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

void RenderCanvas::GetFrame(int &x, int &y, int &w, int &h)
{
	x = m_frame_x;
	y = m_frame_y;
	w = m_frame_w;
	h = m_frame_h;
}

void RenderCanvas::CalcFrame()
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
		fluo::BBox bbox;
		m_cur_vol->getValue(gstBounds, bbox);
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

void RenderCanvas::DrawViewQuad()
{
	flvr::VertexArray* quad_va =
		flvr::TextureRenderer::vertex_array_manager_.vertex_array(flvr::VA_Norm_Square);
	if (quad_va)
		quad_va->draw();
}

void RenderCanvas::switchLevel(fluo::VolumeData *vd)
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
		long prev_lv;
		vd->getValue(gstLevelNum, prev_lv);
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
			vd->setValue(gstLevel, long(new_lv));
			vtex->set_sort_bricks();
		}
	}
}

#ifdef _WIN32
int RenderCanvas::GetPixelFormat(PIXELFORMATDESCRIPTOR *pfd)
{
	int pixelFormat = ::GetPixelFormat(m_hDC);
	if (pixelFormat == 0) return GetLastError();
	pixelFormat = DescribePixelFormat(m_hDC, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), pfd);
	if (pixelFormat == 0) return GetLastError();
	return pixelFormat;
}
#endif

void RenderCanvas::SetLockCenter(int type)
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

void RenderCanvas::SetLockCenterVol()
{
	if (!m_cur_vol)
		return;
	fluo::BBox box;
	m_cur_vol->getValue(gstClipBounds, box);
	m_lock_center = box.center();
}

void RenderCanvas::SetLockCenterRuler()
{
	if (!m_cur_ruler)
		return;
	m_lock_center = m_cur_ruler->GetCenter();
}

void RenderCanvas::SetLockCenterSel()
{
	if (!m_cur_vol)
		return;
	flrd::Cov cover(m_cur_vol);
	cover.Compute(1);
	m_lock_center = cover.GetCenter();
}