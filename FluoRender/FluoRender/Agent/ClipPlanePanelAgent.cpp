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

#include <ClipPlanePanelAgent.h>
#include <ClipPlanePanel.h>
#include <Global.h>
#include <GlobalStates.h>
#include <Names.h>
#include <CurrentObjects.h>
#include <Coordinator.h>
#include <TreeLayer.h>
#include <RenderView.h>
#include <VolumeData.h>
#include <MeshData.h>
#include <MainSettings.h>
#include <ClippingBoxRenderer.h>
#include <ClippingBox.h>
#include <RendererFactory.h>
#include <DataManager.h>
#include <png_resource.h>
#include <icons.h>

ClipPlanePanelAgent::ClipPlanePanelAgent(
	ClipPlanePanel* dlg) :
	Agent(dlg)
{

}

void ClipPlanePanelAgent::UpdateUI(const UpdateRequest& request)
{
	auto panel = GetPanel();
	if (!panel)
		return;

	bool update_all = request.values.empty() || request.HasValue(gstCurrentSelect);

	auto obj = GetObject();
	if (!obj)
	{
		panel->EnableAll(false);
		return;
	}
	auto& cb = obj->GetClippingBox();
	auto fc = obj->GetColor();

	//mf button tips
	if (update_all || request.HasValue(gstMultiFuncTips))
	{
		ClipPlaneToolTips tips;
		switch (glbin_settings.m_mulfunc)
		{
		case 0:
			tips.clip_x = "Synchronize the X clipping values for channels in render view";
			tips.clip_y = "Synchronize the Y clipping values for channels in render view";
			tips.clip_z = "Synchronize the Z clipping values for channels in render view";
			tips.rot_x = "Synchronize the X rotation values for channels in render view";
			tips.rot_y = "Synchronize the Y rotation values for channels in render view";
			tips.rot_z = "Synchronize the Z rotation values for channels in render view";
			break;
		case 1:
			tips.clip_x = "Move mouse cursor in render view and change X clipping using the mouse wheel";
			tips.clip_y = "Move mouse cursor in render view and change Y clipping using the mouse wheel";
			tips.clip_z = "Move mouse cursor in render view and change Z clipping using the mouse wheel";
			tips.rot_x = "Move mouse cursor in render view and change X rotation using the mouse wheel";
			tips.rot_y = "Move mouse cursor in render view and change Y rotation using the mouse wheel";
			tips.rot_z = "Move mouse cursor in render view and change Z rotation using the mouse wheel";
			break;
		case 2:
			tips.clip_x = "Reset X clipping values";
			tips.clip_y = "Reset Y clipping values";
			tips.clip_z = "Reset Z clipping values";
			tips.rot_x = "Reset X rotation values";
			tips.rot_y = "Reset Y rotation values";
			tips.rot_z = "Reset Z rotation values";
			break;
		case 4:
			tips.clip_x = "Undo X clipping value changes";
			tips.clip_y = "Undo Y clipping value changes";
			tips.clip_z = "Undo Z clipping value changes";
			tips.rot_x = "Undo X rotation value changes";
			tips.rot_y = "Undo Y rotation value changes";
			tips.rot_z = "Undo Z rotation value changes";
			break;
		case 3:
		case 5:
			tips.clip_x = "No function assigned";
			tips.clip_y = "No function assigned";
			tips.clip_z = "No function assigned";
			tips.rot_x = "No function assigned";
			tips.rot_y = "No function assigned";
			tips.rot_z = "No function assigned";
			break;
		}
		panel->UpdateToolTips(tips);
	}

	//link channels in view
	if (update_all || request.HasValue(gstClipLinkChan))
	{
		panel->ToggleClipLinkChan(glbin_settings.m_clip_link);
	}

	//hold clipping planes
	if (update_all || request.HasValue(gstClipHold))
	{
		panel->ToggleClipHold(glbin_settings.m_clip_hold);
	}

	//modes
	if (update_all || request.HasValue(gstClipPlaneMode))
	{
		auto mode = static_cast<flrd::ClippingRenderMode>(glbin_settings.m_clip_mode);
		wxBitmapBundle bitmap;
		switch (mode)
		{
		case flrd::ClippingRenderMode::Disabled:
			bitmap = wxGetBitmap(clip_none);
			break;
		case flrd::ClippingRenderMode::ColoredFront:
			bitmap = wxGetBitmap(clip_normal);
			break;
		case flrd::ClippingRenderMode::ColoredBack:
			bitmap = wxGetBitmap(clip_normal_back);
			break;
		case flrd::ClippingRenderMode::FrameAll:
			bitmap = wxGetBitmap(clip_frame6);
			break;
		case flrd::ClippingRenderMode::FrameFront:
			bitmap = wxGetBitmap(clip_frame3);
			break;
		case flrd::ClippingRenderMode::FrameBack:
			bitmap = wxGetBitmap(clip_frame_back);
			break;
		case flrd::ClippingRenderMode::TransFront:
			bitmap = wxGetBitmap(clip_low);
			break;
		case flrd::ClippingRenderMode::TransBack:
			bitmap = wxGetBitmap(clip_low_back);
			break;
		}
		panel->UpdateClipPlaneMode(bitmap);
	}

	wxColor c(fc.r() * 255, fc.g() * 255, fc.b() * 255);

	if (update_all || request.HasValue(gstClipPlaneRanges))
	{
		//slider range
		auto bbox = cb.GetBBoxIndex();
		panel->UpdateClipPlaneRanges(bbox);
	}

	if (update_all || request.HasValue(gstClipPlaneRangeColor))
	{
		panel->UpdateClipPlaneRangeColor(c);
	}

	//clip distance
	if (update_all || request.HasValue(gstClipDist))
	{
		int dx = 1, dy = 1, dz = 1;
		dx = cb.GetLinkedDistIndex(fluo::ClipPlane::XNeg);
		dy = cb.GetLinkedDistIndex(fluo::ClipPlane::YNeg);
		dz = cb.GetLinkedDistIndex(fluo::ClipPlane::ZNeg);
		panel->UpdateClipDist(dx, dy, dz);
	}

	double dval;

	//x1
	if (update_all || request.HasValue(gstClipX1))
	{
		dval = cb.GetClipIndex(fluo::ClipPlane::XNeg);
		panel->UpdateClipX1(dval);
	}
	//x2
	if (update_all || request.HasValue(gstClipX2))
	{
		dval = cb.GetClipIndex(fluo::ClipPlane::XPos);
		panel->UpdateClipX2(dval);
	}
	//y1
	if (update_all || request.HasValue(gstClipY1))
	{
		dval = cb.GetClipIndex(fluo::ClipPlane::YNeg);
		panel->UpdateClipY1(dval);
	}
	//y2
	if (update_all || request.HasValue(gstClipY2))
	{
		dval = cb.GetClipIndex(fluo::ClipPlane::YPos);
		panel->UpdateClipY2(dval);
	}
	//z1
	if (update_all || request.HasValue(gstClipZ1))
	{
		dval = cb.GetClipIndex(fluo::ClipPlane::ZNeg);
		panel->UpdateClipZ1(dval);
	}
	//z2
	if (update_all || request.HasValue(gstClipZ2))
	{
		dval = cb.GetClipIndex(fluo::ClipPlane::ZPos);
		panel->UpdateClipZ2(dval);
	}

	//link
	if (update_all || request.HasValue(gstClipLinkX))
	{
		panel->UpdateClipLinkX();
	}
	if (update_all || request.HasValue(gstClipLinkY))
	{
		panel->UpdateClipLinkY();
	}
	if (update_all || request.HasValue(gstClipLinkZ))
	{
		panel->UpdateClipLinkZ();
	}

	//rotations
	fluo::Vector rot = cb.GetEuler();
	//x
	if (update_all || request.HasValue(gstClipRotX))
	{
		panel->UpdateClipRotX(rot.x());
	}
	//y
	if (update_all || request.HasValue(gstClipRotY))
	{
		panel->UpdateClipRotY(rot.y());
	}
	//z
	if (update_all || request.HasValue(gstClipRotZ))
	{
		panel->UpdateClipRotZ(rot.z());
	}

	panel->EnableAll(true);
}

void ClipPlanePanelAgent::UpdateData(const UpdateRequest& request)
{
	if (request.HasValue(gstClipLinkChan))
		LinkChannels();
	if (request.HasValue(gstClipHold))
		HoldPlanes();
	if (request.HasValue(gstClipPlaneMode))
		SetPlaneMode();
	if (request.HasValue(gstClipSetZero))
		SetClipZero();
	if (request.HasValue(gstClipRotReset))
		RotReset();
	if (request.HasValue(gstClipRotResetX))
		RotResetAxis(0);
	if (request.HasValue(gstClipRotResetY))
		RotResetAxis(1);
	if (request.HasValue(gstClipRotResetZ))
		RotResetAxis(2);
	if (request.HasValue(gstVolumeSampleRate))
		SyncVolumeSampleRate();
}

ClipPlanePanel* ClipPlanePanelAgent::GetPanel() const
{
	return static_cast<ClipPlanePanel*>(GetWindow());
}

std::shared_ptr<TreeLayer> ClipPlanePanelAgent::GetObject()
{
	int type = glbin_current.GetType();
	if (type != 1 && type != 2 && type != 3)
	{
		return nullptr;
	}
	auto vd = glbin_current.vol_data.lock();
	auto md = glbin_current.mesh_data.lock();
	auto view = glbin_current.render_view.lock();

	switch (type)
	{
	case 1:	//render view
		return view;
	case 2:	//volume
		return vd;
	case 3:	//mesh
		return md;
	}

	return nullptr;
}

void ClipPlanePanelAgent::SetClipValue(fluo::ClipPlane i, int val, bool link)
{
	auto view = glbin_current.render_view.lock();
	auto obj = GetObject();
	if (!obj)
		return;
	obj->SetClipValue(i, val);
	int type = glbin_current.GetType();
	if (type == 1)
		view->SyncClippingBoxes(view->GetClippingBox());
	else if (glbin_settings.m_clip_link)
		view->SyncClippingBoxes(obj->GetClippingBox());

	int mask = -1;
	fluo::ValueCollection vc;
	switch (i)
	{
	case fluo::ClipPlane::XNeg:
		mask = link ? 3 : 1;
		vc.insert(gstClipX1);
		if (link)
			vc.insert(gstClipX2);
		break;
	case fluo::ClipPlane::XPos:
		mask = link ? 3 : 2;
		vc.insert(gstClipX2);
		if (link)
			vc.insert(gstClipX1);
		break;
	case fluo::ClipPlane::YNeg:
		mask = link ? 12 : 4;
		vc.insert(gstClipY1);
		if (link)
			vc.insert(gstClipY2);
		break;
	case fluo::ClipPlane::YPos:
		mask = link ? 12 : 8;
		vc.insert(gstClipY2);
		if (link)
			vc.insert(gstClipY1);
		break;
	case fluo::ClipPlane::ZNeg:
		mask = link ? 48 : 16;
		vc.insert(gstClipZ1);
		if (link)
			vc.insert(gstClipZ2);
		break;
	case fluo::ClipPlane::ZPos:
		mask = link ? 48 : 32;
		vc.insert(gstClipZ2);
		if (link)
			vc.insert(gstClipZ1);
		break;
	}

	SetPlaneMask(mask);
	vc.insert(gstConvVolMeshUpdateTransf);
	std::set<Agent*> target{ this };
	auto agent = glbin_coordinator.FindRenderCanvasAgent(view);
	target.insert(agent);
	NotifyViewUpdate(vc, target);
}

void ClipPlanePanelAgent::SetClipValues(fluo::ClipPlane i, int val1, int val2)
{
	auto view = glbin_current.render_view.lock();
	auto obj = GetObject();
	if (!obj)
		return;
	obj->SetClipValues(i, val1, val2);
	int type = glbin_current.GetType();
	if (type == 1)
		view->SyncClippingBoxes(view->GetClippingBox());
	else if (glbin_settings.m_clip_link)
		view->SyncClippingBoxes(obj->GetClippingBox());

	int mask = -1;
	switch (i)
	{
	case fluo::ClipPlane::XNeg:
	case fluo::ClipPlane::XPos:
		mask = 3;
		break;
	case fluo::ClipPlane::YNeg:
	case fluo::ClipPlane::YPos:
		mask = 12;
		break;
	case fluo::ClipPlane::ZNeg:
	case fluo::ClipPlane::ZPos:
		mask = 48;
		break;
	}
	SetPlaneMask(mask);

	fluo::ValueCollection vc{ gstClipX1, gstClipX2, gstClipY1, gstClipY2, gstClipZ1, gstClipZ2, gstConvVolMeshUpdateTransf };
	std::set<Agent*> target{ this };
	auto agent = glbin_coordinator.FindRenderCanvasAgent(view);
	target.insert(agent);
	NotifyViewUpdate(vc, target);
}

void ClipPlanePanelAgent::SetClipValues(const std::array<int, 6>& vals)
{
	auto view = glbin_current.render_view.lock();
	auto obj = GetObject();
	if (!obj)
		return;
	obj->SetClipValues(vals);
	int type = glbin_current.GetType();
	if (type == 1)
		view->SyncClippingBoxes(view->GetClippingBox());
	else if (glbin_settings.m_clip_link)
		view->SyncClippingBoxes(obj->GetClippingBox());
	SetPlaneMask(63);

	fluo::ValueCollection vc{ gstClipX1, gstClipX2, gstClipY1, gstClipY2, gstClipZ1, gstClipZ2, gstConvVolMeshUpdateTransf };
	std::set<Agent*> target{ this };
	auto agent = glbin_coordinator.FindRenderCanvasAgent(view);
	target.insert(agent);
	NotifyViewUpdate(vc, target);
}

void ClipPlanePanelAgent::ResetClipValues()
{
	auto view = glbin_current.render_view.lock();
	auto obj = GetObject();
	if (!obj)
		return;
	obj->ResetClipValues();
	int type = glbin_current.GetType();
	if (type == 1)
		view->SyncClippingBoxes(view->GetClippingBox());
	else if (glbin_settings.m_clip_link)
		view->ResetAllClipValues();
	SetPlaneMask(-1);

	//links
	if (auto panel = GetPanel())
	{
		panel->SetXLink(false);
		panel->SetYLink(false);
		panel->SetZLink(false);
	}

	fluo::ValueCollection vc{
		gstClipX1, gstClipX2, gstClipLinkX,
		gstClipY1, gstClipY2, gstClipLinkY,
		gstClipZ1, gstClipZ2, gstClipLinkZ,
		gstConvVolMeshUpdateTransf };
	std::set<Agent*> target{ this };
	auto agent = glbin_coordinator.FindRenderCanvasAgent(view);
	target.insert(agent);
	NotifyViewUpdate(vc, target);
}

void ClipPlanePanelAgent::ResetClipValues(fluo::ClipPlane i)
{
	auto view = glbin_current.render_view.lock();
	auto obj = GetObject();
	if (!obj)
		return;
	obj->ResetClipValues(i);
	int type = glbin_current.GetType();
	if (type == 1)
		view->SyncClippingBoxes(view->GetClippingBox());
	else if (glbin_settings.m_clip_link)
		view->SyncClippingBoxes(obj->GetClippingBox());
	SetPlaneMask(-1);

	fluo::ValueCollection vc = { gstConvVolMeshUpdateTransf };
	auto panel = GetPanel();
	if (!panel)
		return;
	//links
	switch (i)
	{
	case fluo::ClipPlane::XNeg:
	case fluo::ClipPlane::XPos:
		panel->SetXLink(false);
		vc.insert({ gstClipX1, gstClipX2, gstClipLinkX });
		break;
	case fluo::ClipPlane::YNeg:
	case fluo::ClipPlane::YPos:
		panel->SetYLink(false);
		vc.insert({ gstClipY1, gstClipY2, gstClipLinkY });
		break;
	case fluo::ClipPlane::ZNeg:
	case fluo::ClipPlane::ZPos:
		panel->SetZLink(false);
		vc.insert({ gstClipZ1, gstClipZ2, gstClipLinkZ });
		break;
	}

	std::set<Agent*> target{ this };
	auto agent = glbin_coordinator.FindRenderCanvasAgent(view);
	target.insert(agent);
	NotifyViewUpdate(vc, target);
}

void ClipPlanePanelAgent::SyncClipValue(int i)
{
	auto panel = GetPanel();
	if (!panel)
		return;

	std::array<int, 2> val;
	switch (i)
	{
	case 0:
		val = panel->GetClipX();
		SetClipValues(fluo::ClipPlane::XNeg, val[0], val[1]);
		break;
	case 1:
		val = panel->GetClipY();
		SetClipValues(fluo::ClipPlane::YNeg, val[0], val[1]);
		break;
	case 2:
		val = panel->GetClipZ();
		SetClipValues(fluo::ClipPlane::ZNeg, val[0], val[1]);
		break;
	}
}

void ClipPlanePanelAgent::UpdateClipIdle(bool bval)
{
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;

	if (view->m_capture)
		return;

	glbin_states.m_mouse_in_clip_plane_panel = bval;
	if (glbin_states.ClipDisplayChanged())
	{
		SetPlaneMask(-1);
		NotifyViewUpdate({ gstNull },
			{ glbin_coordinator.FindRenderCanvasAgent(view) });
	}
}

void ClipPlanePanelAgent::SetLinkedDist(fluo::ClipPlane i, int val)
{
	auto layer = GetObject();
	if (!layer)
		return;
	layer->SetLinkedDist(i, val);
}

void ClipPlanePanelAgent::SetPlaneMask(int val)
{
	auto base = glbin_renderer_factory.getOrCreate(gstClippingBoxRenderer);
	auto renderer = std::dynamic_pointer_cast<flrd::ClippingBoxRenderer>(base);
	if (renderer)
	{
		auto settings = std::dynamic_pointer_cast<flrd::ClippingBoxSettings>(renderer->getSettings());
		settings->plane_mask = val;
	}
}

void ClipPlanePanelAgent::SetClipRotX(double val)
{
	auto view = glbin_current.render_view.lock();
	auto obj = GetObject();
	if (!obj)
		return;
	obj->SetClipRotation(0, val);
	int type = glbin_current.GetType();
	if (type == 1)
		view->SyncClippingBoxes(view->GetClippingBox());
	else if (glbin_settings.m_clip_link)
		view->SyncClippingBoxes(obj->GetClippingBox());
	fluo::ValueCollection vc{ gstClipRotX, gstConvVolMeshUpdateTransf };
	std::set<Agent*> target{ this };
	auto agent = glbin_coordinator.FindRenderCanvasAgent(glbin_current.render_view.lock());
	target.insert(agent);
	NotifyViewUpdate(vc, target);
}

void ClipPlanePanelAgent::SetClipRotY(double val)
{
	auto view = glbin_current.render_view.lock();
	auto obj = GetObject();
	if (!obj)
		return;
	obj->SetClipRotation(1, val);
	int type = glbin_current.GetType();
	if (type == 1)
		view->SyncClippingBoxes(view->GetClippingBox());
	else if (glbin_settings.m_clip_link)
		view->SyncClippingBoxes(obj->GetClippingBox());
	fluo::ValueCollection vc{ gstClipRotY, gstConvVolMeshUpdateTransf };
	std::set<Agent*> target{ this };
	auto agent = glbin_coordinator.FindRenderCanvasAgent(glbin_current.render_view.lock());
	target.insert(agent);
	NotifyViewUpdate(vc, target);
}

void ClipPlanePanelAgent::SetClipRotZ(double val)
{
	auto view = glbin_current.render_view.lock();
	auto obj = GetObject();
	if (!obj)
		return;
	obj->SetClipRotation(2, val);
	int type = glbin_current.GetType();
	if (type == 1)
		view->SyncClippingBoxes(view->GetClippingBox());
	else if (glbin_settings.m_clip_link)
		view->SyncClippingBoxes(obj->GetClippingBox());
	fluo::ValueCollection vc{ gstClipRotZ, gstConvVolMeshUpdateTransf };
	std::set<Agent*> target{ this };
	auto agent = glbin_coordinator.FindRenderCanvasAgent(glbin_current.render_view.lock());
	target.insert(agent);
	NotifyViewUpdate(vc, target);
}

void ClipPlanePanelAgent::SetClipDistX(bool use_val, int val)
{
	auto view = glbin_current.render_view.lock();
	auto obj = GetObject();
	if (!obj)
		return;

	obj->ResetClipValues();

	if (use_val)
		SetClipValues(fluo::ClipPlane::XNeg, val, val + 1);
	else
	{
		int dist = obj->GetLinkedDist(fluo::ClipPlane::XNeg);
		SetClipValues(fluo::ClipPlane::XNeg, 0, dist);
	}
}

void ClipPlanePanelAgent::SetClipDistY(bool use_val, int val)
{
	auto view = glbin_current.render_view.lock();
	auto obj = GetObject();
	if (!obj)
		return;

	obj->ResetClipValues();

	if (use_val)
		SetClipValues(fluo::ClipPlane::YNeg, val, val + 1);
	else
	{
		int dist = obj->GetLinkedDist(fluo::ClipPlane::YNeg);
		SetClipValues(fluo::ClipPlane::YNeg, 0, dist);
	}
}

void ClipPlanePanelAgent::SetClipDistZ(bool use_val, int val)
{
	auto view = glbin_current.render_view.lock();
	auto obj = GetObject();
	if (!obj)
		return;

	obj->ResetClipValues();

	if (use_val)
		SetClipValues(fluo::ClipPlane::ZNeg, val, val + 1);
	else
	{
		int dist = obj->GetLinkedDist(fluo::ClipPlane::ZNeg);
		SetClipValues(fluo::ClipPlane::ZNeg, 0, dist);
	}
}

void ClipPlanePanelAgent::LinkChannels()
{
	glbin_settings.m_clip_link = !glbin_settings.m_clip_link;

	auto panel = GetPanel();
	if (!panel)
		return;

	if (glbin_settings.m_clip_link)
	{
		auto val = panel->GetClipValues();
		SetClipValues(val);
	}
	UpdateUIToData({ gstClipLinkChan });
}

void ClipPlanePanelAgent::HoldPlanes()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	glbin_settings.m_clip_hold = panel->GetHoldPlanes();
	glbin_states.ClipDisplayChanged();
	SetPlaneMask(-1);

	std::set<Agent*> target{ this };
	auto agent = glbin_coordinator.FindRenderCanvasAgent(glbin_current.render_view.lock());
	target.insert(agent);
	NotifyViewUpdate({ gstClipHold }, target);
}

void ClipPlanePanelAgent::SetPlaneMode()
{
	int ival = glbin_settings.m_clip_mode;
	ival++;
	ival = ival > static_cast<int>(flrd::ClippingRenderMode::TransBack) ?
		static_cast<int>(flrd::ClippingRenderMode::Disabled) : ival;
	glbin_settings.m_clip_mode = ival;
	auto base = glbin_renderer_factory.getOrCreate(gstClippingBoxRenderer);
	auto renderer = std::dynamic_pointer_cast<flrd::ClippingBoxRenderer>(base);
	if (renderer)
	{
		auto settings = std::dynamic_pointer_cast<flrd::ClippingBoxSettings>(renderer->getSettings());
		settings->mode = static_cast<flrd::ClippingRenderMode>(ival);
	}
	std::set<Agent*> target{ this };
	auto agent = glbin_coordinator.FindRenderCanvasAgent(glbin_current.render_view.lock());
	target.insert(agent);
	NotifyViewUpdate({ gstClipPlaneMode }, target);
}

void ClipPlanePanelAgent::SetClipZero()
{
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;

	view->SetClipRotMode(1);
	fluo::ValueCollection vc{ gstClipRotX, gstClipRotY, gstClipRotZ, gstConvVolMeshUpdateTransf };
	std::set<Agent*> target{ this };
	auto agent = glbin_coordinator.FindRenderCanvasAgent(glbin_current.render_view.lock());
	target.insert(agent);
	NotifyViewUpdate(vc, target);
}

void ClipPlanePanelAgent::RotReset()
{
	auto view = glbin_current.render_view.lock();
	auto obj = GetObject();
	if (!obj)
		return;
	obj->SetClipRotation(fluo::Vector(0));
	int type = glbin_current.GetType();
	if (type == 1)
		view->SyncClippingBoxes(view->GetClippingBox());
	else if (glbin_settings.m_clip_link)
		view->SyncClippingBoxes(obj->GetClippingBox());
	fluo::ValueCollection vc{ gstClipRotX, gstClipRotY, gstClipRotZ, gstConvVolMeshUpdateTransf };
	std::set<Agent*> target{ this };
	auto agent = glbin_coordinator.FindRenderCanvasAgent(glbin_current.render_view.lock());
	target.insert(agent);
	NotifyViewUpdate(vc, target);
}

void ClipPlanePanelAgent::RotResetAxis(int i)
{
	auto view = glbin_current.render_view.lock();
	auto obj = GetObject();
	if (!obj)
		return;
	obj->SetClipRotation(i, 0.0);
	int type = glbin_current.GetType();
	if (type == 1)
		view->SyncClippingBoxes(view->GetClippingBox());
	else if (glbin_settings.m_clip_link)
		view->SyncClippingBoxes(obj->GetClippingBox());
	fluo::ValueCollection vc{ gstConvVolMeshUpdateTransf };
	switch (i)
	{
	case 0://x
		vc.insert(gstClipRotX);
		break;
	case 1://y
		vc.insert(gstClipRotY);
		break;
	case 2://z
		vc.insert(gstClipRotZ);
		break;
	}
	std::set<Agent*> target{ this };
	auto agent = glbin_coordinator.FindRenderCanvasAgent(glbin_current.render_view.lock());
	target.insert(agent);
	NotifyViewUpdate(vc, target);
}

void ClipPlanePanelAgent::SyncVolumeSampleRate()
{
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;

	//good rate
	if (vd->GetSampleRate() < 2.0)
		vd->SetSampleRate(2.0);
	if (glbin_settings.m_clip_link)
	{
		int i;
		for (i = 0; i < glbin_data_manager.GetVolumeNum(); i++)
		{
			auto vd = glbin_data_manager.GetVolumeData(i);
			if (!vd || vd == vd)
				continue;
			if (vd->GetSampleRate() < 2.0)
				vd->SetSampleRate(2.0);
		}
	}
}
