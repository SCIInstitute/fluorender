/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2022 Scientific Computing and Imaging Institute,
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
#include <RenderviewPanel.h>
#include <RenderFrame.h>
#include <Root.hpp>
#include <Renderview.hpp>
#include <RenderviewFactory.hpp>
#include <AgentFactory.hpp>
#include <Global.hpp>
#include <Timer.hpp>
#include <Input.hpp>
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

RenderCanvas::RenderCanvas(RenderFrame* frame,
	RenderviewPanel* parent,
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
	m_ptr_id2(-1)
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
	//bool bval;
	//m_agent->getValue(gstBenchmark, bval);
	//if (bval)
	//{
	//	unsigned long long msec, frames;
	//	double fps;
	//	m_agent->getValue(gstBmRuntime, msec);
	//	m_agent->getValue(gstBmFrames, frames);
	//	m_agent->getValue(gstBmFps, fps);
	//	long lx, ly;
	//	m_agent->getValue(gstSizeX, lx);
	//	m_agent->getValue(gstSizeY, ly);
	//	wxString string = wxString("FluoRender has finished benchmarking.\n") +
	//		wxString("Results:\n") +
	//		wxString("Render size: ") + wxString::Format("%d X %d\n", lx, ly) +
	//		wxString("Time: ") + wxString::Format("%llu msec\n", msec) +
	//		wxString("Frames: ") + wxString::Format("%llu\n", frames) +
	//		wxString("FPS: ") + wxString::Format("%.2f", fps);
	//	wxMessageDialog *diag = new wxMessageDialog(this, string, "Benchmark Results",
	//		wxOK | wxICON_INFORMATION);
	//	diag->ShowModal();
	//}

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
	//key states
	wxPoint mouse_pos = wxGetMousePosition();
	wxRect view_reg = GetScreenRect();
	wxWindow *window = wxWindow::FindFocus();
	bool mouse_in = window && view_reg.Contains(mouse_pos);
	m_agent->setValue(gstMouseIn, mouse_in);
	m_agent->setValue(gstRenderviewPanelId, long(m_vrv->GetID()));
	glbin_input->Update();
	m_agent->getObject()->HandleIdle();

	//full screen
	if (mouse_in && wxGetKeyState(WXK_ESCAPE))
	{
		m_fullscreen_trigger.Start(10);
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
//		OutAdjustPanel* adjust_view = m_frame->GetAdjustView();
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
//		OutAdjustPanel* adjust_view = m_frame->GetAdjustView();
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
//		OutAdjustPanel* adjust_view = m_frame->GetAdjustView();
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
	double dval = 1;
	long mx, my;
#ifdef _DARWIN
	dval = GetDPIScaleFactor();
#endif
	mx = long(event.GetX() * dval + 0.5);
	my = long(event.GetY() * dval + 0.5);
	m_agent->setValue(gstMouseX, mx);
	m_agent->setValue(gstMouseY, my);
	m_agent->setValue(gstMouseDrag, event.Dragging());
	m_agent->setValue(gstMouseWheel, long(event.GetWheelRotation()));
	glbin_input->Update();
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

