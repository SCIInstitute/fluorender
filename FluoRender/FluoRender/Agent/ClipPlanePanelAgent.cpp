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
#include <Names.h>
#include <CurrentObjects.h>
#include <TreeLayer.h>
#include <RenderView.h>
#include <VolumeData.h>
#include <MeshData.h>
#include <MainSettings.h>
#include <ClippingBoxRenderer.h>
#include <png_resource.h>
#include <icons.h>

ClipPlanePanelAgent::ClipPlanePanelAgent(
	ClipPlanePanel* dlg) :
	Agent(dlg)
{

}

bool ClipPlanePanelAgent::Accept(
	const UpdateRequest& request) const
{
	return true;
}

void ClipPlanePanelAgent::Update(
	const UpdateRequest& request)
{
	if (request.dir == UpdateDir::DataToUI)
	{
		UpdateUI(request);
	}
	else if (request.dir == UpdateDir::UItoData)
	{
		UpdateData(request);
	}
}

void ClipPlanePanelAgent::UpdateUI(const UpdateRequest& request)
{
	auto panel = GetPanel();
	if (!panel)
		return;

	if (FOUND_VALUE(gstNull))
		return;
	bool update_all = request.values.empty() || FOUND_VALUE(gstCurrentSelect);

	auto obj = GetObject();
	if (!obj)
	{
		panel->EnableAll(false);
		return;
	}
	auto& cb = obj->GetClippingBox();
	auto fc = obj->GetColor();

	//mf button tips
	if (update_all || FOUND_VALUE(gstMultiFuncTips))
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
	if (update_all || FOUND_VALUE(gstClipLinkChan))
	{
		panel->ToggleClipLinkChan(glbin_settings.m_clip_link);
	}

	//hold clipping planes
	if (update_all || FOUND_VALUE(gstClipHold))
	{
		panel->ToggleClipHold(glbin_settings.m_clip_hold);
	}

	//modes
	if (update_all || FOUND_VALUE(gstClipPlaneMode))
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

	if (update_all || FOUND_VALUE(gstClipPlaneRanges))
	{
		//slider range
		auto bbox = cb.GetBBoxIndex();
		panel->UpdateClipPlaneRanges(bbox);
	}

	if (update_all || FOUND_VALUE(gstClipPlaneRangeColor))
	{
		panel->UpdateClipPlaneRangeColor(c);
	}

	//clip distance
	if (update_all || FOUND_VALUE(gstClipDist))
	{
		int dx = 1, dy = 1, dz = 1;
		dx = cb.GetLinkedDistIndex(fluo::ClipPlane::XNeg);
		dy = cb.GetLinkedDistIndex(fluo::ClipPlane::YNeg);
		dz = cb.GetLinkedDistIndex(fluo::ClipPlane::ZNeg);
		panel->UpdateClipDist(dx, dy, dz);
	}

	bool bval;
	double dval;
	int ival;
	wxString str;

	//x1
	if (update_all || FOUND_VALUE(gstClipX1))
	{
		dval = cb.GetClipIndex(fluo::ClipPlane::XNeg);
		panel->UpdateClipX1(dval);
	}
	//x2
	if (update_all || FOUND_VALUE(gstClipX2))
	{
		dval = cb.GetClipIndex(fluo::ClipPlane::XPos);
		panel->UpdateClipX2(dval);
	}
	//y1
	if (update_all || FOUND_VALUE(gstClipY1))
	{
		dval = cb.GetClipIndex(fluo::ClipPlane::YNeg);
		panel->UpdateClipY1(dval);
	}
	//y2
	if (update_all || FOUND_VALUE(gstClipY2))
	{
		dval = cb.GetClipIndex(fluo::ClipPlane::YPos);
		panel->UpdateClipY2(dval);
	}
	//z1
	if (update_all || FOUND_VALUE(gstClipZ1))
	{
		dval = cb.GetClipIndex(fluo::ClipPlane::ZNeg);
		panel->UpdateClipZ1(dval);
	}
	//z2
	if (update_all || FOUND_VALUE(gstClipZ2))
	{
		dval = cb.GetClipIndex(fluo::ClipPlane::ZPos);
		panel->UpdateClipZ2(dval);
	}

	//link
	if (update_all || FOUND_VALUE(gstClipLinkX))
	{
		panel->UpdateClipLinkX();
	}
	if (update_all || FOUND_VALUE(gstClipLinkY))
	{
		panel->UpdateClipLinkY();
	}
	if (update_all || FOUND_VALUE(gstClipLinkZ))
	{
		panel->UpdateClipLinkZ();
	}

	//rotations
	fluo::Vector rot = cb.GetEuler();
	//x
	if (update_all || FOUND_VALUE(gstClipRotX))
	{
		panel->UpdateClipRotX(rot.x());
	}
	//y
	if (update_all || FOUND_VALUE(gstClipRotY))
	{
		panel->UpdateClipRotY(rot.y());
	}
	//z
	if (update_all || FOUND_VALUE(gstClipRotZ))
	{
		panel->UpdateClipRotZ(rot.z());
	}

	panel->EnableAll(true);
}

void ClipPlanePanelAgent::UpdateData(const UpdateRequest& request)
{

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
