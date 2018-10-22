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

#include <Fui/ClipPlaneAgent.h>
#include <Fui/ClipPlanePanel.h>
#include <wx/valnum.h>
#include <png_resource.h>
#include <img/icons.h>

using namespace FUI;

ClipPlaneAgent::ClipPlaneAgent(ClipPlanePanel &panel) :
	InterfaceAgent(),
	panel_(panel)
{

}

void ClipPlaneAgent::objectChanging(int notify_level, void* ptr, void* orig_node, const std::string &exp)
{
	//before change
	if (notify_level & FL::Value::NotifyLevel::NOTIFY_AGENT)
	{
		InterfaceAgent::objectChanging(notify_level, ptr, orig_node, exp);
	}
}

void ClipPlaneAgent::objectChanged(int notify_level, void* ptr, void* orig_node, const std::string &exp)
{
	//set values in ui
	if (notify_level & FL::Value::NotifyLevel::NOTIFY_AGENT)
	{
		InterfaceAgent::objectChanged(notify_level, ptr, orig_node, exp);
	}
}

void ClipPlaneAgent::setObject(FL::Node* obj)
{
	InterfaceAgent::setObject(obj);
}

FL::Node* ClipPlaneAgent::getObject()
{
	return dynamic_cast<FL::Node*>(InterfaceAgent::getObject());
}

void ClipPlaneAgent::UpdateAllSettings()
{
	if (!getObject())
	{
		panel_.DisableAll();
		return;
	}
	else
		panel_.EnableAll();

	//ranges
	long resx, resy, resz;
	bool result = getValue("res x", resx);
	if (!result)
	{
		panel_.DisableAll();
		return;
	}
	getValue("res y", resy);
	getValue("res z", resz);
	//slider range
	panel_.m_x1_clip_sldr->SetRange(0, resx);
	panel_.m_x2_clip_sldr->SetRange(0, resx);
	panel_.m_y1_clip_sldr->SetRange(0, resy);
	panel_.m_y2_clip_sldr->SetRange(0, resy);
	panel_.m_z1_clip_sldr->SetRange(0, resz);
	panel_.m_z2_clip_sldr->SetRange(0, resz);
	//text range
	wxIntegerValidator<int>* vald_i;
	if ((vald_i = (wxIntegerValidator<int>*)panel_.m_x1_clip_text->GetValidator()))
		vald_i->SetRange(0, int(resx));
	if ((vald_i = (wxIntegerValidator<int>*)panel_.m_x2_clip_text->GetValidator()))
		vald_i->SetRange(0, int(resx));
	if ((vald_i = (wxIntegerValidator<int>*)panel_.m_y1_clip_text->GetValidator()))
		vald_i->SetRange(0, int(resy));
	if ((vald_i = (wxIntegerValidator<int>*)panel_.m_y2_clip_text->GetValidator()))
		vald_i->SetRange(0, int(resy));
	if ((vald_i = (wxIntegerValidator<int>*)panel_.m_z1_clip_text->GetValidator()))
		vald_i->SetRange(0, int(resz));
	if ((vald_i = (wxIntegerValidator<int>*)panel_.m_z2_clip_text->GetValidator()))
		vald_i->SetRange(0, int(resz));

	long distx, disty, distz;
	getValue("clip dist x", distx);
	getValue("clip dist y", disty);
	getValue("clip dist z", distz);
	panel_.m_yz_dist_text->SetValue(
		wxString::Format("%d", distx));
	panel_.m_xz_dist_text->SetValue(
		wxString::Format("%d", disty));
	panel_.m_xy_dist_text->SetValue(
		wxString::Format("%d", distz));

	FLTYPE::PlaneSet planes;
	getValue("clip planes", planes);
	if (planes.GetSize() != 6)
		return;

	UpdatePlanes(planes, resx, resy, resz);

	//clip link
	bool clip_link_x, clip_link_y, clip_link_z;
	getValue("clip link x", clip_link_x);
	getValue("clip link y", clip_link_y);
	getValue("clip link z", clip_link_z);
	panel_.m_link_x_tb->ToggleTool(
		ClipPlanePanel::ID_LinkXChk, clip_link_x);
	if (clip_link_x)
		panel_.m_link_x_tb->SetToolNormalBitmap(
			ClipPlanePanel::ID_LinkXChk,
			wxGetBitmapFromMemory(link));
	else
		panel_.m_link_x_tb->SetToolNormalBitmap(
			ClipPlanePanel::ID_LinkXChk,
			wxGetBitmapFromMemory(unlink));
	panel_.m_link_y_tb->ToggleTool(
		ClipPlanePanel::ID_LinkYChk, clip_link_y);
	if (clip_link_y)
		panel_.m_link_y_tb->SetToolNormalBitmap(
			ClipPlanePanel::ID_LinkYChk,
			wxGetBitmapFromMemory(link));
	else
		panel_.m_link_y_tb->SetToolNormalBitmap(
			ClipPlanePanel::ID_LinkYChk,
			wxGetBitmapFromMemory(unlink));
	panel_.m_link_z_tb->ToggleTool(
		ClipPlanePanel::ID_LinkZChk, clip_link_z);
	if (clip_link_z)
		panel_.m_link_z_tb->SetToolNormalBitmap(
			ClipPlanePanel::ID_LinkZChk,
			wxGetBitmapFromMemory(link));
	else
		panel_.m_link_z_tb->SetToolNormalBitmap(
			ClipPlanePanel::ID_LinkZChk,
			wxGetBitmapFromMemory(unlink));
}

void ClipPlaneAgent::UpdatePlanes(FLTYPE::PlaneSet &planes,
	long resx, long resy, long resz)
{
	//set values in ui
	int ival = 0;
	wxString str;
	//x1
	ival = int(fabs(planes[0].d() * resx) + 0.499);
	panel_.m_x1_clip_sldr->SetValue(ival);
	double percent = (double)ival / (double)panel_.m_x1_clip_sldr->GetMax();
	int barsize = (panel_.m_x1_clip_sldr->GetSize().GetHeight() - 20);
	str = wxString::Format("%d", ival);
	panel_.m_x1_clip_text->ChangeValue(str);
	//x2
	ival = fabs(planes[1].d() * resx) + 0.499;
	panel_.m_x2_clip_sldr->SetValue(ival);
	panel_.m_xBar->SetPosition(wxPoint(20, 10 + percent * barsize));
	panel_.m_xBar->SetSize(wxSize(3, barsize*((double)
		(ival - panel_.m_x1_clip_sldr->GetValue()) / (double)panel_.m_x1_clip_sldr->GetMax())));
	str = wxString::Format("%d", ival);
	panel_.m_x2_clip_text->ChangeValue(str);
	//y1
	ival = fabs(planes[2].d() * resy) + 0.499;
	panel_.m_y1_clip_sldr->SetValue(ival);
	percent = (double)ival / (double)panel_.m_y1_clip_sldr->GetMax();
	barsize = (panel_.m_y1_clip_sldr->GetSize().GetHeight() - 20);
	str = wxString::Format("%d", ival);
	panel_.m_y1_clip_text->ChangeValue(str);
	//y2
	ival = fabs(planes[3].d() * resy) + 0.499;
	panel_.m_y2_clip_sldr->SetValue(ival);
	panel_.m_yBar->SetPosition(wxPoint(20, 10 + percent * barsize));
	panel_.m_yBar->SetSize(wxSize(3, barsize*((double)
		(ival - panel_.m_y1_clip_sldr->GetValue()) / (double)panel_.m_y1_clip_sldr->GetMax())));
	str = wxString::Format("%d", ival);
	panel_.m_y2_clip_text->ChangeValue(str);
	//z1
	ival = fabs(planes[4].d() * resz) + 0.499;
	panel_.m_z1_clip_sldr->SetValue(ival);
	percent = (double)ival / (double)panel_.m_z1_clip_sldr->GetMax();
	barsize = (panel_.m_z1_clip_sldr->GetSize().GetHeight() - 20);
	str = wxString::Format("%d", ival);
	panel_.m_z1_clip_text->ChangeValue(str);
	//z2
	ival = fabs(planes[5].d() * resz) + 0.499;
	panel_.m_zBar->SetPosition(wxPoint(20, 10 + percent * barsize));
	panel_.m_zBar->SetSize(wxSize(3, barsize*((double)
		(ival - panel_.m_z1_clip_sldr->GetValue()) / (double)panel_.m_z1_clip_sldr->GetMax())));
	panel_.m_z2_clip_sldr->SetValue(ival);
	str = wxString::Format("%d", ival);
	panel_.m_z2_clip_text->ChangeValue(str);
}

void ClipPlaneAgent::OnClipPlanesChanging()
{
	getValue("clip planes", old_planes_);
}

void ClipPlaneAgent::OnClipPlanesChanged()
{
	FLTYPE::PlaneSet planes;
	getValue("clip planes", planes);
	if (planes.GetSize() != 6)
		return;

	long resx, resy, resz;
	getValue("res x", resx);
	getValue("res y", resy);
	getValue("res z", resz);
	bool clip_link_x, clip_link_y, clip_link_z;
	getValue("clip link x", clip_link_x);
	getValue("clip link y", clip_link_y);
	getValue("clip link z", clip_link_z);
	if (clip_link_x)
	{
		long clip_dist_x;
		getValue("clip dist x", clip_dist_x);
		double dist = double(clip_dist_x) / double(resx);
		double ox1 = fabs(old_planes_[0].d());
		double ox2 = fabs(old_planes_[1].d());
		double nx1 = fabs(planes[0].d());
		double nx2 = fabs(planes[1].d());
		if (ox1 != nx1)
		{
			//plane 0 changed, sync plane1
			nx2 = nx1 + dist;
			if (nx2 > 1.0)
				planes[0].ChangePlane(
					FLTYPE::Point(ox1, 0.0, 0.0),
					FLTYPE::Vector(1.0, 0.0, 0.0));
			else
				planes[1].ChangePlane(
					FLTYPE::Point(nx2, 0.0, 0.0),
					FLTYPE::Vector(-1.0, 0.0, 0.0));
		}
		else if (ox2 != nx2)
		{
			//plane 1 changed, sync plane 0
			nx1 = nx2 - dist;
			if (nx1 < 0.0)
				planes[1].ChangePlane(
					FLTYPE::Point(ox2, 0.0, 0.0),
					FLTYPE::Vector(-1.0, 0.0, 0.0));
			else
				planes[0].ChangePlane(
					FLTYPE::Point(nx1, 0.0, 0.0),
					FLTYPE::Vector(1.0, 0.0, 0.0));
		}
	}
	if (clip_link_y)
	{
		long clip_dist_y;
		getValue("clip dist y", clip_dist_y);
		double dist = double(clip_dist_y) / double(resy);
		double oy1 = fabs(old_planes_[2].d());
		double oy2 = fabs(old_planes_[3].d());
		double ny1 = fabs(planes[2].d());
		double ny2 = fabs(planes[3].d());
		if (oy1 != ny1)
		{
			//plane 2 changed, sync plane3
			ny2 = ny1 + dist;
			if (ny2 > 1.0)
				planes[2].ChangePlane(
					FLTYPE::Point(0.0, oy1, 0.0),
					FLTYPE::Vector(0.0, 1.0, 0.0));
			else
				planes[3].ChangePlane(
					FLTYPE::Point(0.0, ny2, 0.0),
					FLTYPE::Vector(0.0, -1.0, 0.0));
		}
		else if (oy2 != ny2)
		{
			//plane 3 changed, sync plane 2
			ny1 = ny2 - dist;
			if (ny1 < 0.0)
				planes[3].ChangePlane(
					FLTYPE::Point(0.0, oy2, 0.0),
					FLTYPE::Vector(0.0, -1.0, 0.0));
			else
				planes[2].ChangePlane(
					FLTYPE::Point(0.0, ny1, 0.0),
					FLTYPE::Vector(0.0, 1.0, 0.0));
		}
	}
	if (clip_link_z)
	{
		long clip_dist_z;
		getValue("clip dist z", clip_dist_z);
		double dist = double(clip_dist_z) / double(resz);
		double oz1 = fabs(old_planes_[4].d());
		double oz2 = fabs(old_planes_[5].d());
		double nz1 = fabs(planes[4].d());
		double nz2 = fabs(planes[5].d());
		if (oz1 != nz1)
		{
			//plane 4 changed, sync plane 5
			nz2 = nz1 + dist;
			if (nz2 > 1.0)
				planes[4].ChangePlane(
					FLTYPE::Point(0.0, 0.0, oz1),
					FLTYPE::Vector(0.0, 0.0, 1.0));
			else
				planes[5].ChangePlane(
					FLTYPE::Point(0.0, 0.0, nz2),
					FLTYPE::Vector(0.0, 0.0, -1.0));
		}
		else if (oz2 != nz2)
		{
			//plane 1 changed, sync plane 0
			nz1 = nz2 - dist;
			if (nz1 < 0.0)
				planes[5].ChangePlane(
					FLTYPE::Point(0.0, 0.0, oz2),
					FLTYPE::Vector(0.0, 0.0, -1.0));
			else
				planes[4].ChangePlane(
					FLTYPE::Point(0.0, 0.0, nz1),
					FLTYPE::Vector(0.0, 0.0, 1.0));
		}
	}
	if (clip_link_x || clip_link_y || clip_link_z)
		setValue("clip planes", planes);
	UpdatePlanes(planes, resx, resy, resz);
}

void ClipPlaneAgent::OnClipDistXChanged()
{
	long lval;
	getValue("clip dist x", lval);
	panel_.m_yz_dist_text->SetValue(
		wxString::Format("%d", lval));
}

void ClipPlaneAgent::OnClipDistYChanged()
{
	long lval;
	getValue("clip dist y", lval);
	panel_.m_xz_dist_text->SetValue(
		wxString::Format("%d", lval));
}

void ClipPlaneAgent::OnClipDistZChanged()
{
	long lval;
	getValue("clip dist z", lval);
	panel_.m_xy_dist_text->SetValue(
		wxString::Format("%d", lval));
}
