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
#include <Root.hpp>
#include <Renderview.hpp>
#include <RenderviewFactory.hpp>
#include <AgentFactory.hpp>
#include <Global.hpp>
#include <Timer.h>
#include <TextureRenderer.h>
#include <RulerHandler.h>
#include <RulerRenderer.h>
#include <VolumePoint.h>
#include <VolumeSelector.h>
#include <VolumeCalculator.h>
#include <ScriptProc.h>
#include <Debug.h>

#include <wx/stdpaths.h>
#include <wx/msw/private.h>
#include <png_resource.h>
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
	m_set_gl(false),
	//touch
	m_enable_touch(false),
	m_ptr_id1(-1),
	m_ptr_id2(-1),
	//flags for idle
	ks_v_mask(false),
	ks_ctrl_left(false),
	ks_ctrl_right(false),
	ks_ctrl_up(false),
	ks_ctrl_down(false),
	ks_d_spc_forward(false),
	ks_a_backward(false),
	ks_s_up(false),
	ks_w_down(false),
	ks_f_full(false),
	ks_l_link(false),
	ks_n_new(false),
	ks_c_clear(false),
	ks_m_svmask(false),
	ks_rtn_include(false),
	ks_bksl_exclude(false),
	ks_r_relax(false),
	ms_lb_grow(false)
{
	m_glRC = sharedContext;
	m_sharedRC = m_glRC ? true : false;

	fluo::Renderview* view = glbin_revf->build();
	view->setName(m_vrv->GetName().ToStdString());
	glbin_root->addChild(view);
	glbin_root->setRvalu(gstCurrentView, view);
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
	m_agent->holdoffObserverNotification();

	double dval = 1;
#ifdef _DARWIN
	dval = GetDPIScaleFactor();
#endif
	wxSize size = GetSize() * dval;
	m_vrv->UpdateScaleFactor(false);
	m_agent->setValue(gstSizeX, long(size.x));
	m_agent->setValue(gstSizeY, long(size.y));

	m_agent->resumeObserverNotificationAndUpdate();
	//m_agent->getObject()->RefreshGL(1);
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
#ifdef _WIN32
		//check touch
		HMODULE user32 = LoadLibrary(L"user32");
		GetPI = reinterpret_cast<decltype(GetPointerInfo)*>
			(GetProcAddress(user32, "GetPointerInfo"));
		if (GetPI != NULL)
			m_enable_touch = true;
		else
			m_enable_touch = false;
#endif

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
				view->Update(6);
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
				view->Update(7);
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
				view->Update(8);
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
					view->Update(9);
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
					view->Update(10);
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
					view->Update(11);
				}
			}
			else
			{
				if (view)
				{
					view->SetBrush(lval);
					view->Update(12);
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
			view->Update(13);

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
	std::string str;
	m_agent->getValue(gstSelectedMshName, str);
	if (!str.empty())
	{
		if (m_frame && m_frame->GetTree())
		{
			m_frame->GetTree()->SetFocus();
			m_frame->GetTree()->Select(m_vrv->GetName(), str);
		}
		m_agent->getObject()->Update(27);
	}
	else
	{
		if (m_frame && m_frame->GetCurSelType() == 3 &&
			m_frame->GetTree())
			m_frame->GetTree()->Select(m_vrv->GetName(), "");
	}
}

void RenderCanvas::PickVolume()
{
	int kmode = wxGetKeyState(WXK_CONTROL) ? 1 : 0;
	std::string str;
	m_agent->getValue(gstSelectedVolName, str);
	fluo::Point ip;
	m_agent->getValue(gstSelPointVolume, ip);
	if (!str.empty())
	{
		if (m_frame && m_frame->GetTree())
		{
			m_frame->GetTree()->SetFocus();
			m_frame->GetTree()->Select(m_vrv->GetName(), str);
		}
		//update label selection
		SetCompSelection(ip, kmode);
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
	m_agent->holdoffObserverNotification();

	bool refresh = false;
	bool start_loop = true;
	bool set_focus = false;
	bool retain_fb = false;
	bool pre_draw = false;

	//check memory swap status
	if (flvr::TextureRenderer::get_mem_swap() &&
		flvr::TextureRenderer::get_start_update_loop() &&
		!flvr::TextureRenderer::get_done_update_loop())
	{
		if (flvr::TextureRenderer::active_view_ == m_vrv->GetID())
		{
			refresh = true;
			start_loop = false;
		}
	}
	bool capture_rotat, capture_tsequ, capture_param, test_speed;
	m_agent->getValue(gstCaptureRot, capture_rotat);
	m_agent->getValue(gstCaptureTime, capture_tsequ);
	m_agent->getValue(gstCaptureParam, capture_param);
	m_agent->getValue(gstTestSpeed, test_speed);
	if (capture_rotat ||
		capture_tsequ ||
		capture_param ||
		test_speed)
	{
		refresh = true;
		if (flvr::TextureRenderer::get_mem_swap() &&
			flvr::TextureRenderer::get_done_update_loop())
			pre_draw = true;
	}

	bool openvr_enable;
	m_agent->getValue(gstOpenvrEnable, openvr_enable);
	if (openvr_enable)
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
			pre_draw = true;
	}

	//pin rotation center
	refresh = refresh || m_agent->getObject()->PinRotCtr();

	wxPoint mouse_pos = wxGetMousePosition();
	wxRect view_reg = GetScreenRect();

	wxWindow *window = wxWindow::FindFocus();
	if (window && view_reg.Contains(mouse_pos))
	{
		UpdateBrushState();

		//draw_mask
		if (wxGetKeyState(wxKeyCode('V')) &&
			ks_v_mask)
		{
			ks_v_mask = false;
			refresh = true;
			set_focus = true;
		}
		if (!wxGetKeyState(wxKeyCode('V')) &&
			!ks_v_mask)
		{
			ks_v_mask = true;
			refresh = true;
		}

		//move view
		//left
		if (!ks_ctrl_left &&
			wxGetKeyState(WXK_CONTROL) &&
			wxGetKeyState(WXK_LEFT))
		{
			ks_ctrl_left = true;
			double dx, dy, dz;
			m_agent->getValue(gstCamTransX, dx);
			m_agent->getValue(gstCamTransY, dy);
			m_agent->getValue(gstCamTransZ, dz);
			fluo::Vector head(-dx, -dy, -dz);
			head.normalize();
			m_agent->setValue(gstCamHead, head);
			fluo::Vector up;
			m_agent->getValue(gstCamUp, up);
			fluo::Vector side = fluo::Cross(up, head);
			double dr, dl;
			m_agent->getValue(gstOrthoLeft, dl);
			m_agent->getValue(gstOrthoRight, dr);
			fluo::Vector trans = -(side*(int(0.8*(dr - dl))));
			m_agent->getValue(gstObjTransX, dx);
			m_agent->getValue(gstObjTransY, dy);
			m_agent->getValue(gstObjTransZ, dz);
			dx += trans.x();
			dy += trans.y();
			dz += trans.z();
			m_agent->setValue(gstObjTransX, dx);
			m_agent->setValue(gstObjTransY, dy);
			m_agent->setValue(gstObjTransZ, dz);
			//if (m_persp) SetSortBricks();
			refresh = true;
			set_focus = true;
		}
		if (ks_ctrl_left &&
			(!wxGetKeyState(WXK_CONTROL) ||
				!wxGetKeyState(WXK_LEFT)))
			ks_ctrl_left = false;
		//right
		if (!ks_ctrl_right &&
			wxGetKeyState(WXK_CONTROL) &&
			wxGetKeyState(WXK_RIGHT))
		{
			ks_ctrl_right = true;
			double dx, dy, dz;
			m_agent->getValue(gstCamTransX, dx);
			m_agent->getValue(gstCamTransY, dy);
			m_agent->getValue(gstCamTransZ, dz);
			fluo::Vector head(-dx, -dy, -dz);
			head.normalize();
			m_agent->setValue(gstCamHead, head);
			fluo::Vector up;
			m_agent->getValue(gstCamUp, up);
			fluo::Vector side = fluo::Cross(up, head);
			double dr, dl;
			m_agent->getValue(gstOrthoLeft, dl);
			m_agent->getValue(gstOrthoRight, dr);
			fluo::Vector trans = side*(int(0.8*(dr - dl)));
			m_agent->getValue(gstObjTransX, dx);
			m_agent->getValue(gstObjTransY, dy);
			m_agent->getValue(gstObjTransZ, dz);
			dx += trans.x();
			dy += trans.y();
			dz += trans.z();
			m_agent->setValue(gstObjTransX, dx);
			m_agent->setValue(gstObjTransY, dy);
			m_agent->setValue(gstObjTransZ, dz);
			//if (m_persp) SetSortBricks();
			refresh = true;
			set_focus = true;
		}
		if (ks_ctrl_right &&
			(!wxGetKeyState(WXK_CONTROL) ||
				!wxGetKeyState(WXK_RIGHT)))
			ks_ctrl_right = false;
		//up
		if (!ks_ctrl_up &&
			wxGetKeyState(WXK_CONTROL) &&
			wxGetKeyState(WXK_UP))
		{
			ks_ctrl_up = true;
			double dx, dy, dz;
			m_agent->getValue(gstCamTransX, dx);
			m_agent->getValue(gstCamTransY, dy);
			m_agent->getValue(gstCamTransZ, dz);
			fluo::Vector head(-dx, -dy, -dz);
			head.normalize();
			m_agent->setValue(gstCamHead, head);
			fluo::Vector up;
			m_agent->getValue(gstCamUp, up);
			double dt, db;
			m_agent->getValue(gstOrthoTop, dt);
			m_agent->getValue(gstOrthoBottom, db);
			fluo::Vector trans = -up*(int(0.8*(dt - db)));
			m_agent->getValue(gstObjTransX, dx);
			m_agent->getValue(gstObjTransY, dy);
			m_agent->getValue(gstObjTransZ, dz);
			dx += trans.x();
			dy += trans.y();
			dz += trans.z();
			m_agent->setValue(gstObjTransX, dx);
			m_agent->setValue(gstObjTransY, dy);
			m_agent->setValue(gstObjTransZ, dz);
			//if (m_persp) SetSortBricks();
			refresh = true;
			set_focus = true;
		}
		if (ks_ctrl_up &&
			(!wxGetKeyState(WXK_CONTROL) ||
				!wxGetKeyState(WXK_UP)))
			ks_ctrl_up = false;
		//down
		if (!ks_ctrl_down &&
			wxGetKeyState(WXK_CONTROL) &&
			wxGetKeyState(WXK_DOWN))
		{
			ks_ctrl_down = true;
			double dx, dy, dz;
			m_agent->getValue(gstCamTransX, dx);
			m_agent->getValue(gstCamTransY, dy);
			m_agent->getValue(gstCamTransZ, dz);
			fluo::Vector head(-dx, -dy, -dz);
			head.normalize();
			m_agent->setValue(gstCamHead, head);
			fluo::Vector up;
			m_agent->getValue(gstCamUp, up);
			double dt, db;
			m_agent->getValue(gstOrthoTop, dt);
			m_agent->getValue(gstOrthoBottom, db);
			fluo::Vector trans = up*(int(0.8*(dt - db)));
			m_agent->getValue(gstObjTransX, dx);
			m_agent->getValue(gstObjTransY, dy);
			m_agent->getValue(gstObjTransZ, dz);
			dx += trans.x();
			dy += trans.y();
			dz += trans.z();
			m_agent->setValue(gstObjTransX, dx);
			m_agent->setValue(gstObjTransY, dy);
			m_agent->setValue(gstObjTransZ, dz);
			//if (m_persp) SetSortBricks();
			refresh = true;
			set_focus = true;
		}
		if (ks_ctrl_down &&
			(!wxGetKeyState(WXK_CONTROL) ||
				!wxGetKeyState(WXK_DOWN)))
			ks_ctrl_down = false;

		//move time sequence
		//forward
		if (!ks_d_spc_forward &&
			(wxGetKeyState(wxKeyCode('d')) ||
				wxGetKeyState(WXK_SPACE)))
		{
			ks_d_spc_forward = true;
			if (m_frame && m_frame->GetMovieView())
				m_frame->GetMovieView()->UpFrame();
			refresh = true;
			set_focus = true;
		}
		if (ks_d_spc_forward &&
			!wxGetKeyState(wxKeyCode('d')) &&
			!wxGetKeyState(WXK_SPACE))
			ks_d_spc_forward = false;
		//backforward
		if (!ks_a_backward &&
			wxGetKeyState(wxKeyCode('a')))
		{
			ks_a_backward = true;
			if (m_frame && m_frame->GetMovieView())
				m_frame->GetMovieView()->DownFrame();
			refresh = true;
			set_focus = true;
		}
		if (ks_a_backward &&
			!wxGetKeyState(wxKeyCode('a')))
			ks_a_backward = false;

		//move clip
		//up
		if (!ks_s_up &&
			wxGetKeyState(wxKeyCode('s')))
		{
			ks_s_up = true;
			if (m_frame && m_frame->GetClippingView())
				m_frame->GetClippingView()->MoveLinkedClippingPlanes(1);
			refresh = true;
			set_focus = true;
		}
		if (ks_s_up &&
			!wxGetKeyState(wxKeyCode('s')))
			ks_s_up = false;
		//down
		if (!ks_w_down &&
			wxGetKeyState(wxKeyCode('w')))
		{
			ks_w_down = true;
			if (m_frame && m_frame->GetClippingView())
				m_frame->GetClippingView()->MoveLinkedClippingPlanes(0);
			refresh = true;
			set_focus = true;
		}
		if (ks_w_down &&
			!wxGetKeyState(wxKeyCode('w')))
			ks_w_down = false;

		//cell full
		if (!ks_f_full &&
			wxGetKeyState(wxKeyCode('f')))
		{
			ks_f_full = true;
			if (m_frame && m_frame->GetComponentDlg())
				m_frame->GetComponentDlg()->SelectFullComp();
			if (m_frame && m_frame->GetTraceDlg())
				m_frame->GetTraceDlg()->CellUpdate();
			refresh = true;
			set_focus = true;
		}
		if (ks_f_full &&
			!wxGetKeyState(wxKeyCode('f')))
			ks_f_full = false;
		//cell link
		if (!ks_l_link &&
			wxGetKeyState(wxKeyCode('l')))
		{
			ks_l_link = true;
			if (m_frame && m_frame->GetTraceDlg())
				m_frame->GetTraceDlg()->CellLink(false);
			refresh = true;
			set_focus = true;
		}
		if (ks_l_link &&
			!wxGetKeyState(wxKeyCode('l')))
			ks_l_link = false;
		//new cell id
		if (!ks_n_new &&
			wxGetKeyState(wxKeyCode('n')))
		{
			ks_n_new = true;
			if (m_frame && m_frame->GetTraceDlg())
				m_frame->GetTraceDlg()->CellNewID(false);
			refresh = true;
			set_focus = true;
		}
		if (ks_n_new &&
			!wxGetKeyState(wxKeyCode('n')))
			ks_n_new = false;
		//clear
		if (wxGetKeyState(wxKeyCode('c')) &&
			!ks_c_clear)
		{
			if (m_frame && m_frame->GetTree())
				m_frame->GetTree()->BrushClear();
			if (m_frame && m_frame->GetTraceDlg())
				m_frame->GetTraceDlg()->CompClear();
			ks_c_clear = true;
			refresh = true;
			set_focus = true;
		}
		if (!wxGetKeyState(wxKeyCode('c')) &&
			ks_c_clear)
			ks_c_clear = false;
		//save all masks
		if (wxGetKeyState(wxKeyCode('m')) &&
			!ks_m_svmask)
		{
			if (m_frame && m_frame->GetList())
				m_frame->GetList()->SaveAllMasks();
			ks_m_svmask = true;
			set_focus = true;
		}
		if (!wxGetKeyState(wxKeyCode('m')) &&
			ks_m_svmask)
			ks_m_svmask = false;
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
			!ks_rtn_include)
		{
			if (m_frame && m_frame->GetComponentDlg())
				m_frame->GetComponentDlg()->IncludeComps();
			ks_rtn_include = true;
			refresh = true;
			set_focus = true;
		}
		if (!wxGetKeyState(WXK_RETURN) &&
			ks_rtn_include)
			ks_rtn_include = false;
		//comp exclude
		if (wxGetKeyState(wxKeyCode('\\')) &&
			!ks_bksl_exclude)
		{
			if (m_frame && m_frame->GetComponentDlg())
				m_frame->GetComponentDlg()->ExcludeComps();
			ks_bksl_exclude = true;
			refresh = true;
			set_focus = true;
		}
		if (!wxGetKeyState(wxKeyCode('\\')) &&
			ks_bksl_exclude)
			ks_bksl_exclude = false;
		//ruler relax
		if (wxGetKeyState(wxKeyCode('r')) &&
			!ks_r_relax)
		{
			if (m_frame && m_frame->GetMeasureDlg())
				m_frame->GetMeasureDlg()->Relax();
			ks_r_relax = true;
			refresh = true;
			set_focus = true;
		}
		if (!wxGetKeyState(wxKeyCode('r')) &&
			ks_r_relax)
			ks_r_relax = false;

		long int_mode;
		m_agent->getValue(gstInterMode, int_mode);
		//grow
		if ((int_mode == 10 ||
			int_mode == 12) &&
			wxGetMouseState().LeftIsDown() &&
			ms_lb_grow)
		{
			int sz = 5;
			if (m_frame && m_frame->GetSettingDlg())
				sz = m_frame->GetSettingDlg()->GetRulerSizeThresh();
			//event.RequestMore();
			m_agent->getObject()->Grow(sz);
			refresh = true;
			start_loop = true;
			//update
			if (m_frame)
			{
				bool bval;
				m_agent->getValue(gstPaintCount, bval);
				if (bval && m_frame->GetBrushToolDlg())
					m_frame->GetBrushToolDlg()->Update(0);
				m_agent->getValue(gstPaintColocalize, bval);
				if (bval && m_frame->GetColocalizationDlg())
					m_frame->GetColocalizationDlg()->Colocalize();
				if (int_mode == 12 && m_frame->GetMeasureDlg())
					m_frame->GetMeasureDlg()->GetSettings(m_agent->getObject());
			}
		}

		//forced refresh
		if (wxGetKeyState(WXK_F5))
		{
			SetFocus();
			if (m_frame && m_frame->GetStatusBar())
				m_frame->GetStatusBar()->PushStatusText("Forced Refresh");
			//wxSizeEvent e;
			//OnResize(e);

			//m_agent->setValue(gstClearBuffer, true);
			//m_agent->setValue(gstUpdating, true);
			//m_agent->setValue(gstPreDraw, pre_draw);
			//m_agent->setValue(gstRetainFb, retain_fb);
			//m_agent->getObject()->Update(14);

			if (m_frame && m_frame->GetStatusBar())
				m_frame->GetStatusBar()->PopStatusText();
			//return;
			refresh = true;
		}
	}

#ifdef _WIN32
	//update ortho rotation
	fluo::Quaternion q;
	m_agent->getValue(gstCamRotQ, q);
	int cmb_sel = 6;
	if (q.AlmostEqual(fluo::Quaternion(0, sqrt(2.0) / 2.0, 0, sqrt(2.0) / 2.0)))
		cmb_sel = 0;
	else if (q.AlmostEqual(fluo::Quaternion(0, -sqrt(2.0) / 2.0, 0, sqrt(2.0) / 2.0)))
		cmb_sel = 1;
	else if (q.AlmostEqual(fluo::Quaternion(sqrt(2.0) / 2.0, 0, 0, sqrt(2.0) / 2.0)))
		cmb_sel = 2;
	else if (q.AlmostEqual(fluo::Quaternion(-sqrt(2.0) / 2.0, 0, 0, sqrt(2.0) / 2.0)))
		cmb_sel = 3;
	else if (q.AlmostEqual(fluo::Quaternion(0, 0, 0, 1)))
		cmb_sel = 4;
	else if (q.AlmostEqual(fluo::Quaternion(0, -1, 0, 0)))
		cmb_sel = 5;
	else
		cmb_sel = 6;
	m_vrv->UpdateOrientCmb(cmb_sel);
#endif

#if defined(_WIN32) && defined(USE_XINPUT)
	//xinput controller
	refresh = refresh || m_agent->getObject()->UpdateController();
#endif

	if (set_focus)
		SetFocus();
	if (refresh)
	{
		m_agent->setValue(gstClearBuffer, true);
		m_agent->setValue(gstUpdating, true);
		m_agent->setValue(gstPreDraw, pre_draw);
		m_agent->setValue(gstRetainFb, retain_fb);
		//m_agent->getObject()->RefreshGL(15, start_loop);
		m_agent->resumeObserverNotificationAndUpdate();
	}
	else
		m_agent->resumeObserverNotification();
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
	bool bval;
	m_agent->getValue(gstBenchmark, bval);
	if (bval)
	{
		m_vrv->HideFullFrame();
		if (m_frame)
			m_frame->Close();
	}
	else if (GetParent() == m_vrv->GetFullFrame())
	{
		Reparent(m_vrv);
		m_vrv->AddCanvas(this);
		m_vrv->Layout();
		m_vrv->HideFullFrame();
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
		m_agent->getObject()->Update(40);
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

//draw
void RenderCanvas::OnDraw(wxPaintEvent& event)
{
	if (!m_glRC) return;
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
	SwapBuffers();
	//if (!m_refresh)
	//	m_retain_finalbuffer = true;
	//else
	//	m_refresh = false;
	//ForceDraw();
}

//void RenderCanvas::RemoveVolumeDataDup(const std::string &name)
//{
//	fluo::VolumeData* vd_main = 0;
//	for (int i = 0; i < GetLayerNum() && !vd_main; i++)
//	{
//		if (fluo::VolumeData* vd = dynamic_cast<fluo::VolumeData*>(GetLayer(i)))
//		{
//			if (name == vd->getName())
//			{
//				vd_main = vd;
//				m_vd_pop_dirty = true;
//			}
//		}
//		else if (fluo::VolumeGroup* group = dynamic_cast<fluo::VolumeGroup*>(GetLayer(i)))//group
//		{
//			for (int j = 0; j<group->getNumChildren(); j++)
//			{
//				fluo::VolumeData* vd = group->getChild(j)->asVolumeData();
//				if (vd && name == vd->getName())
//				{
//					vd_main = vd;
//					m_vd_pop_dirty = true;
//				}
//			}
//		}
//	}
//
//	if (!vd_main)
//		return;
//	
//	for (int i = 0; i < GetLayerNum();)
//	{
//		if (!GetLayer(i))
//		{
//			++i;
//			continue;
//		}
//		if (fluo::VolumeData* vd = dynamic_cast<fluo::VolumeData*>(GetLayer(i)))
//		{
//			bool del = false;
//			if (vd == vd_main)
//				del = true;
//			bool dup;
//			vd->getValue(gstDuplicate, dup);
//			if (dup)
//			{
//				//if (vd->GetDupData() == vd_main)
//				//	del = true;
//			}
//			if (del)
//			{
//				m_layer_list.erase(m_layer_list.begin() + i);
//				if (m_cur_vol == vd)
//					m_cur_vol = 0;
//			}
//			else
//				++i;
//		}
//		else if (fluo::VolumeGroup* group = dynamic_cast<fluo::VolumeGroup*>(GetLayer(i)))//group
//		{
//			for (int j = group->getNumChildren()-1; j >= 0; --j)
//			{
//				fluo::VolumeData* vd = group->getChild(j)->asVolumeData();
//				if (vd)
//				{
//					bool del = false;
//					bool dup;
//					vd->getValue(gstDuplicate, dup);
//					if (dup)
//					{
//						//if (vd->GetDupData() == vd_main)
//						//	del = true;
//					}
//					else
//					{
//						if (vd == vd_main)
//							del = true;
//					}
//					if (del)
//					{
//						group->removeChild(j);
//						if (m_cur_vol == vd)
//							m_cur_vol = 0;
//					}
//				}
//			}
//			++i;
//		}
//		else
//			++i;
//	}
//}
//
//void RenderCanvas::RemoveMeshData(const std::string &name)
//{
//	int i, j;
//
//	for (i = 0; i<GetLayerNum(); i++)
//	{
//		if (fluo::MeshData* md = dynamic_cast<fluo::MeshData*>(GetLayer(i)))
//		{
//			if (name == md->getName())
//			{
//				m_layer_list.erase(m_layer_list.begin() + i);
//				m_md_pop_dirty = true;
//				return;
//			}
//		}
//		else if (fluo::MeshGroup* group = dynamic_cast<fluo::MeshGroup*>(GetLayer(i)))//mesh group
//		{
//			for (j = 0; j<group->getNumChildren(); j++)
//			{
//				fluo::MeshData* md = group->getChild(j)->asMeshData();
//				if (md && name == md->getName())
//				{
//					group->removeChild(j);
//					m_md_pop_dirty = true;
//					return;
//				}
//			}
//		}
//	}
//}
//
//void RenderCanvas::RemoveAnnotations(const std::string &name)
//{
//	for (int i = 0; i<GetLayerNum(); i++)
//	{
//		if (fluo::Annotations* ann = dynamic_cast<fluo::Annotations*>(GetLayer(i)))
//		{
//			if (name == ann->getName())
//				m_layer_list.erase(m_layer_list.begin() + i);
//		}
//	}
//}
//
//void RenderCanvas::RemoveGroup(const std::string &name)
//{
//	int i, j;
//	for (i = 0; i<GetLayerNum(); i++)
//	{
//		if (fluo::VolumeGroup* group = dynamic_cast<fluo::VolumeGroup*>(GetLayer(i)))
//		{
//			if (name == group->getName())
//			{
//				for (j = group->getNumChildren() - 1; j >= 0; j--)
//				{
//					fluo::VolumeData* vd = group->getChild(j)->asVolumeData();
//					if (vd)
//					{
//						group->removeChild(j);
//						//if add back to view
//					}
//				}
//				m_layer_list.erase(m_layer_list.begin() + i);
//				//delete group;
//				m_vd_pop_dirty = true;
//			}
//		}
//		else if (fluo::MeshGroup* group = dynamic_cast<fluo::MeshGroup*>(GetLayer(i)))//mesh group
//		{
//			if (name == group->getName())
//			{
//				for (j = group->getNumChildren() - 1; j >= 0; j--)
//				{
//					fluo::MeshData* md = group->getChild(j)->asMeshData();
//					if (md)
//					{
//						group->removeChild(j);
//					}
//				}
//				m_layer_list.erase(m_layer_list.begin() + i);
//				//delete group;
//				m_md_pop_dirty = true;
//			}
//		}
//	}
//}
//
////isolate
//void RenderCanvas::Isolate(int type, const std::string &name)
//{
//	for (int i = 0; i < GetLayerNum(); i++)
//	{
//		fluo::Object* layer = GetLayer(i);
//		if (!layer) continue;
//		if (name == layer->getName())
//			layer->setValue(gstDisplay, true);
//		else
//			layer->setValue(gstDisplay, false);
//	}
//
//	m_vd_pop_dirty = true;
//	m_md_pop_dirty = true;
//}
//
////move layer of the same level within this view
////source is after the destination
//void RenderCanvas::MoveLayerinView(const std::string &src_name, const std::string &dst_name)
//{
//	int i, src_index;
//	fluo::Object* src = 0;
//	for (i = 0; i<GetLayerNum(); i++)
//	{
//		fluo::Object* layer = GetLayer(i);
//		if (layer && src_name == layer->getName())
//		{
//			src = layer;
//			src_index = i;
//			m_layer_list.erase(m_layer_list.begin() + i);
//			break;
//		}
//	}
//	if (!src)
//		return;
//	for (i = 0; i<GetLayerNum(); i++)
//	{
//		fluo::Object* layer = GetLayer(i);
//		if (layer && dst_name == layer->getName())
//		{
//			if (i >= src_index)
//				m_layer_list.insert(m_layer_list.begin() + i + 1, src);
//			else
//				m_layer_list.insert(m_layer_list.begin() + i, src);
//			break;
//		}
//	}
//	m_vd_pop_dirty = true;
//	m_md_pop_dirty = true;
//}
//
//void RenderCanvas::ShowAll()
//{
//	for (unsigned int i = 0; i < GetLayerNum(); ++i)
//	{
//		fluo::Object* layer = GetLayer(i);
//		if (layer)
//			layer->setValue(gstDisplay, true);
//	}
//	m_vd_pop_dirty = true;
//	m_md_pop_dirty = true;
//}
//
////move layer (volume) of the same level within the given group
////source is after the destination
//void RenderCanvas::MoveLayerinGroup(const std::string &group_name, const std::string &src_name, const std::string &dst_name)
//{
//	fluo::VolumeGroup* group = GetGroup(group_name);
//	if (!group)
//		return;
//
//	fluo::VolumeData* src_vd = 0;
//	int i, src_index;
//	for (i = 0; i<group->getNumChildren(); i++)
//	{
//		std::string name = group->getChild(i)->getName();
//		if (name == src_name)
//		{
//			src_index = i;
//			src_vd = group->getChild(i)->asVolumeData();
//			group->removeChild(i);
//			break;
//		}
//	}
//	if (!src_vd)
//		return;
//	for (i = 0; i<group->getNumChildren(); i++)
//	{
//		std::string name = group->getChild(i)->getName();
//		if (name == dst_name)
//		{
//			if (i >= src_index)
//				group->insertChild(i, src_vd);
//			else
//				group->insertChild(i - 1, src_vd);
//			break;
//		}
//	}
//	m_vd_pop_dirty = true;
//	m_md_pop_dirty = true;
//}
//
////move layer (volume) from the given group up one level to this view
////source is after the destination
//void RenderCanvas::MoveLayertoView(const std::string &group_name, const std::string &src_name, const std::string &dst_name)
//{
//	fluo::VolumeGroup* group = GetGroup(group_name);
//	if (!group)
//		return;
//
//	fluo::VolumeData* src_vd = 0;
//	int i;
//	for (i = 0; i<group->getNumChildren(); i++)
//	{
//		std::string name = group->getChild(i)->getName();
//		if (name == src_name)
//		{
//			src_vd = group->getChild(i)->asVolumeData();
//			group->removeChild(i);
//			break;
//		}
//	}
//	if (!src_vd)
//		return;
//	if (dst_name == "")
//	{
//		m_layer_list.push_back(src_vd);
//	}
//	else
//	{
//		for (i = 0; i<GetLayerNum(); i++)
//		{
//			std::string name = GetLayer(i)->getName();
//			if (name == dst_name)
//			{
//				m_layer_list.insert(m_layer_list.begin() + i + 1, src_vd);
//				break;
//			}
//		}
//	}
//	m_vd_pop_dirty = true;
//	m_md_pop_dirty = true;
//}
//
////move layer (volume) one level down to the given group
////source is after the destination
//void RenderCanvas::MoveLayertoGroup(const std::string &group_name, const std::string &src_name, const std::string &dst_name)
//{
//	fluo::VolumeData* src_vd = 0;
//	int i;
//
//	for (i = 0; i<GetLayerNum(); i++)
//	{
//		if (src_vd = dynamic_cast<fluo::VolumeData*>(GetLayer(i)))
//		{
//			//is volume data
//			if (src_name == src_vd->getName())
//			{
//				m_layer_list.erase(m_layer_list.begin() + i);
//				break;
//			}
//		}
//	}
//	fluo::VolumeGroup* group = GetGroup(group_name);
//	if (!group || !src_vd)
//		return;
//	if (group->getNumChildren() == 0 || dst_name == "")
//	{
//		group->insertChild(0, src_vd);
//	}
//	else
//	{
//		for (i = 0; i<group->getNumChildren(); i++)
//		{
//			std::string name = group->getChild(i)->getName();
//			if (name == dst_name)
//			{
//				group->insertChild(i, src_vd);
//				break;
//			}
//		}
//	}
//
//	//set the 2d adjustment settings of the volume the same as the group
//	fluo::ValueCollection names{
//		gstGammaR, gstGammaG, gstGammaB,
//		gstBrightnessR, gstBrightnessG, gstBrightnessB,
//		gstEqualizeR, gstEqualizeG, gstEqualizeB,
//		gstSyncR, gstSyncG, gstSyncB };
//	src_vd->propValues(names, group);
//
//	m_vd_pop_dirty = true;
//	m_md_pop_dirty = true;
//}
//
////move layer (volume from one group to another different group
////sourece is after the destination
//void RenderCanvas::MoveLayerfromtoGroup(
//	const std::string &src_group_name,
//	const std::string &dst_group_name,
//	const std::string &src_name,
//	const std::string &dst_name)
//{
//	fluo::VolumeGroup* src_group = GetGroup(src_group_name);
//	if (!src_group)
//		return;
//	int i;
//	fluo::VolumeData* src_vd = 0;
//	for (i = 0; i<src_group->getNumChildren(); i++)
//	{
//		std::string name = src_group->getChild(i)->getName();
//		if (name == src_name)
//		{
//			src_vd = src_group->getChild(i)->asVolumeData();
//			src_group->removeChild(i);
//			break;
//		}
//	}
//	fluo::VolumeGroup* dst_group = GetGroup(dst_group_name);
//	if (!dst_group || !src_vd)
//		return;
//	if (dst_group->getNumChildren() == 0 || dst_name == "")
//	{
//		dst_group->insertChild(0, src_vd);
//	}
//	else
//	{
//		for (i = 0; i<dst_group->getNumChildren(); i++)
//		{
//			std::string name = dst_group->getChild(i)->getName();
//			if (name == dst_name)
//			{
//				dst_group->insertChild(i, src_vd);
//				break;
//			}
//		}
//	}
//
//	//reset the sync of the source group
//	//src_group->ResetSync();
//
//	//set the 2d adjustment settings of the volume the same as the group
//	fluo::ValueCollection names{
//		gstGammaR, gstGammaG, gstGammaB,
//		gstBrightnessR, gstBrightnessG, gstBrightnessB,
//		gstEqualizeR, gstEqualizeG, gstEqualizeB,
//		gstSyncR, gstSyncG, gstSyncB };
//	src_vd->propValues(names, dst_group);
//
//	m_vd_pop_dirty = true;
//	m_md_pop_dirty = true;
//
//	if (m_frame)
//	{
//		AdjustView* adjust_view = m_frame->GetAdjustView();
//		if (adjust_view)
//		{
//			adjust_view->SetVolumeData(src_vd);
//			adjust_view->SetGroupLink(dst_group);
//			adjust_view->UpdateSync();
//		}
//	}
//}
//
////move mesh within a group
//void RenderCanvas::MoveMeshinGroup(const std::string &group_name,
//	const std::string &src_name, const std::string &dst_name)
//{
//	fluo::MeshGroup* group = GetMGroup(group_name);
//	if (!group)
//		return;
//
//	fluo::MeshData* src_md = 0;
//	int i, src_index;
//	for (i = 0; i<group->getNumChildren(); i++)
//	{
//		std::string name = group->getChild(i)->getName();
//		if (name == src_name)
//		{
//			src_index = i;
//			src_md = group->getChild(i)->asMeshData();
//			group->removeChild(i);
//			break;
//		}
//	}
//	if (!src_md)
//		return;
//	for (i = 0; i<group->getNumChildren(); i++)
//	{
//		std::string name = group->getChild(i)->getName();
//		if (name == dst_name)
//		{
//			if (i >= src_index)
//				group->insertChild(i, src_md);
//			else
//				group->insertChild(i - 1, src_md);
//			break;
//		}
//	}
//
//	m_vd_pop_dirty = true;
//	m_md_pop_dirty = true;
//}
//
////move mesh out of a group
//void RenderCanvas::MoveMeshtoView(const std::string &group_name,
//	const std::string &src_name, const std::string &dst_name)
//{
//	fluo::MeshGroup* group = GetMGroup(group_name);
//	if (!group)
//		return;
//
//	fluo::MeshData* src_md = 0;
//	int i;
//	for (i = 0; i<group->getNumChildren(); i++)
//	{
//		std::string name = group->getChild(i)->getName();
//		if (name == src_name)
//		{
//			src_md = group->getChild(i)->asMeshData();
//			group->removeChild(i);
//			break;
//		}
//	}
//	if (!src_md)
//		return;
//	if (dst_name == "")
//		m_layer_list.push_back(src_md);
//	else
//	{
//		for (i = 0; i<GetLayerNum(); i++)
//		{
//			std::string name = GetLayer(i)->getName();
//			if (name == dst_name)
//			{
//				m_layer_list.insert(m_layer_list.begin() + i + 1, src_md);
//				break;
//			}
//		}
//	}
//	m_vd_pop_dirty = true;
//	m_md_pop_dirty = true;
//}
//
////move mesh into a group
//void RenderCanvas::MoveMeshtoGroup(const std::string &group_name,
//	const std::string &src_name, const std::string &dst_name)
//{
//	fluo::MeshData* src_md = 0;
//	int i;
//
//	for (i = 0; i<GetLayerNum(); i++)
//	{
//		std::string name = GetLayer(i)->getName();
//		if (name != src_name) continue;
//		if (src_md = dynamic_cast<fluo::MeshData*>(GetLayer(i)))
//		{
//			m_layer_list.erase(m_layer_list.begin() + i);
//			break;
//		}
//	}
//	fluo::MeshGroup* group = GetMGroup(group_name);
//	if (!group || !src_md)
//		return;
//	if (group->getNumChildren() == 0 || dst_name == "")
//		group->insertChild(0, src_md);
//	else
//	{
//		for (i = 0; i<group->getNumChildren(); i++)
//		{
//			std::string name = group->getChild(i)->getName();
//			if (name == dst_name)
//			{
//				group->insertChild(i, src_md);
//				break;
//			}
//		}
//	}
//	m_vd_pop_dirty = true;
//	m_md_pop_dirty = true;
//}
//
////move mesh out of then into a group
//void RenderCanvas::MoveMeshfromtoGroup(
//	const std::string &src_group_name,
//	const std::string &dst_group_name,
//	const std::string &src_name,
//	const std::string &dst_name)
//{
//	fluo::MeshGroup* src_group = GetMGroup(src_group_name);
//	if (!src_group)
//		return;
//	int i;
//	fluo::MeshData* src_md = 0;
//	for (i = 0; i<src_group->getNumChildren(); i++)
//	{
//		std::string name = src_group->getChild(i)->getName();
//		if (name == src_name)
//		{
//			src_md = src_group->getChild(i)->asMeshData();
//			src_group->removeChild(i);
//			break;
//		}
//	}
//	fluo::MeshGroup* dst_group = GetMGroup(dst_group_name);
//	if (!dst_group || !src_md)
//		return;
//	if (dst_group->getNumChildren() == 0 || dst_name == "")
//		dst_group->insertChild(0, src_md);
//	else
//	{
//		for (i = 0; i<dst_group->getNumChildren(); i++)
//		{
//			std::string name = dst_group->getChild(i)->getName();
//			if (name == dst_name)
//			{
//				dst_group->insertChild(i, src_md);
//				break;
//			}
//		}
//	}
//	m_vd_pop_dirty = true;
//	m_md_pop_dirty = true;
//}
//
////layer control
//int RenderCanvas::GetGroupNum()
//{
//	int group_num = 0;
//
//	for (int i = 0; i<GetLayerNum(); i++)
//	{
//		fluo::VolumeGroup *group = dynamic_cast<fluo::VolumeGroup*>(GetLayer(i));
//		if (group)
//			group_num++;
//	}
//	return group_num;
//}
//
//int RenderCanvas::GetLayerNum()
//{
//	return m_layer_list.size();
//}
//
//fluo::Object* RenderCanvas::GetLayer(int index)
//{
//	if (index >= 0 && index<(int)m_layer_list.size())
//		return m_layer_list[index].get();
//	else
//		return nullptr;
//}
//
//std::string RenderCanvas::AddGroup(const std::string &str, const std::string &prev_group)
//{
//	fluo::VolumeGroup* group = glbin_volf->buildGroup();
//	if (group && str != "")
//		group->setName(str);
//
//	bool found_prev = false;
//	for (int i = 0; i<GetLayerNum(); i++)
//	{
//		if (fluo::VolumeGroup* group_temp = dynamic_cast<fluo::VolumeGroup*>(GetLayer(i)))
//		{
//			if (prev_group == group_temp->getName())
//			{
//				m_layer_list.insert(m_layer_list.begin() + i + 1, group);
//				found_prev = true;
//			}
//		}
//	}
//	if (!found_prev)
//		m_layer_list.push_back(group);
//
//	//set default settings
//	if (m_frame)
//	{
//		AdjustView* adjust_view = m_frame->GetAdjustView();
//		if (adjust_view && group)
//		{
//			fluo::Color gamma, brightness, hdr;
//			bool sync_r, sync_g, sync_b;
//			adjust_view->GetDefaults(gamma, brightness, hdr, sync_r, sync_g, sync_b);
//			group->setValue(gstGammaR, gamma.r());
//			group->setValue(gstGammaG, gamma.g());
//			group->setValue(gstGammaB, gamma.b());
//			group->setValue(gstBrightnessR, brightness.r());
//			group->setValue(gstBrightnessG, brightness.g());
//			group->setValue(gstBrightnessB, brightness.b());
//			group->setValue(gstEqualizeR, hdr.r());
//			group->setValue(gstEqualizeG, hdr.g());
//			group->setValue(gstEqualizeB, hdr.b());
//			group->setValue(gstSyncR, sync_r);
//			group->setValue(gstSyncG, sync_g);
//			group->setValue(gstSyncB, sync_b);
//		}
//	}
//
//	if (group)
//		return group->getName();
//	else
//		return "Group";
//}
//
//fluo::VolumeGroup* RenderCanvas::AddOrGetGroup()
//{
//	for (int i = 0; i < GetLayerNum(); i++)
//	{
//		if (fluo::VolumeGroup* group_temp = dynamic_cast<fluo::VolumeGroup*>(GetLayer(i)))
//		{
//			if (!group_temp->getNumChildren())
//				return group_temp;
//		}
//	}
//	//group not found
//	fluo::VolumeGroup* group = glbin_volf->buildGroup();
//	if (!group)
//		return 0;
//	//set default settings
//	if (m_frame)
//	{
//		AdjustView* adjust_view = m_frame->GetAdjustView();
//		if (adjust_view)
//		{
//			fluo::Color gamma, brightness, hdr;
//			bool sync_r, sync_g, sync_b;
//			adjust_view->GetDefaults(gamma, brightness, hdr, sync_r, sync_g, sync_b);
//			group->setValue(gstGammaR, gamma.r());
//			group->setValue(gstGammaG, gamma.g());
//			group->setValue(gstGammaB, gamma.b());
//			group->setValue(gstBrightnessR, brightness.r());
//			group->setValue(gstBrightnessG, brightness.g());
//			group->setValue(gstBrightnessB, brightness.b());
//			group->setValue(gstEqualizeR, hdr.r());
//			group->setValue(gstEqualizeG, hdr.g());
//			group->setValue(gstEqualizeB, hdr.b());
//			group->setValue(gstSyncR, sync_r);
//			group->setValue(gstSyncG, sync_g);
//			group->setValue(gstSyncB, sync_b);
//		}
//	}
//	m_layer_list.push_back(group);
//	return group;
//}
//
//std::string RenderCanvas::AddMGroup(const std::string &str)
//{
//	fluo::MeshGroup* group = glbin_mshf->buildGroup();
//	if (group && str != "")
//		group->setName(str);
//	m_layer_list.push_back(group);
//
//	if (group)
//		return group->getName();
//	else
//		return "";
//}
//
//fluo::MeshGroup* RenderCanvas::AddOrGetMGroup()
//{
//	for (int i = 0; i < GetLayerNum(); i++)
//	{
//		if (fluo::MeshGroup* group_temp = dynamic_cast<fluo::MeshGroup*>(GetLayer(i)))
//		{
//			if (group_temp && !group_temp->getNumChildren())
//				return group_temp;
//		}
//	}
//	//group not found
//	fluo::MeshGroup* group = glbin_mshf->buildGroup();
//	if (!group)
//		return 0;
//	m_layer_list.push_back(group);
//	return group;
//}
//
//fluo::MeshGroup* RenderCanvas::GetMGroup(const std::string &str)
//{
//	int i;
//
//	for (i = 0; i<GetLayerNum(); i++)
//	{
//		if (fluo::MeshGroup* group = dynamic_cast<fluo::MeshGroup*>(GetLayer(i)))
//		{
//			if (group && str == group->getName())
//				return group;
//		}
//	}
//	return 0;
//}

#ifdef _WIN32
WXLRESULT RenderCanvas::MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam)
{
	//handle wacom messages
	if (message == WT_PACKET)
	{
		PACKET pkt;
		if (gpWTPacket((HCTX)lParam, wParam, &pkt))
			m_agent->getObject()->SetPressure(pkt.pkNormalPressure, pkt.pkTangentPressure);
	}

	//map touch messages to mouse events
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
	bool interactive, geared_rot;
	m_agent->getValue(gstInteractive, interactive);
	m_agent->getValue(gstGearedEnable, geared_rot);
	if (interactive && !geared_rot)
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

	//set properties
	m_agent->setValue(gstMouseX, long(event.GetX()));
	m_agent->setValue(gstMouseY, long(event.GetY()));
	m_agent->setValue(gstMouseLeftDown, event.LeftDown());
	m_agent->setValue(gstMouseRightDown, event.RightDown());
	m_agent->setValue(gstMouseMiddleDown, event.MiddleDown());
	m_agent->setValue(gstMouseLeftUp, event.LeftUp());
	m_agent->setValue(gstMouseRightUp, event.RightUp());
	m_agent->setValue(gstMouseMiddleUp, event.MiddleUp());
	m_agent->setValue(gstMouseLeftHold, event.LeftIsDown());
	m_agent->setValue(gstMouseRightHold, event.RightIsDown());
	m_agent->setValue(gstMouseMiddleHold, event.MiddleIsDown());
	m_agent->setValue(gstMouseDrag, event.Dragging());
	m_agent->setValue(gstMouseWheel, long(event.GetWheelRotation()));
	m_agent->setValue(gstKbAltDown, event.AltDown());
	m_agent->setValue(gstKbCtrlDown, event.ControlDown());
	m_agent->getObject()->HandleMouse();
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

