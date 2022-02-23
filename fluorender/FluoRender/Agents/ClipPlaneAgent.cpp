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

#include <ClipPlaneAgent.hpp>
#include <ClipPlanePanel.h>
#include <Global.hpp>
#include <AgentFactory.hpp>
#include <Renderview.hpp>
#include <RenderCanvas.h>
#include <SearchVisitor.hpp>
#include <RenderCanvasAgent.hpp>
#include <wx/valnum.h>
#include <png_resource.h>
#include <img/icons.h>

using namespace fluo;

ClipPlaneAgent::ClipPlaneAgent(ClipPlanePanel &panel) :
	InterfaceAgent(),
	panel_(panel)
{

}

void ClipPlaneAgent::setObject(Node* obj)
{
	InterfaceAgent::setObject(obj);
}

Node* ClipPlaneAgent::getObject()
{
	return dynamic_cast<Node*>(InterfaceAgent::getObject());
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

	//states
	bool bval;
	getValue(gstClipHold, bval);
	panel_.m_toolbar->ToggleTool(panel_.ID_HoldPlanesBtn, bval);
	long render_mode;
	getValue(gstClipRenderMode, render_mode);
	switch (render_mode)
	{
	case fluo::PRMNormal:
		panel_.m_toolbar->SetToolNormalBitmap(panel_.ID_PlaneModesBtn,
			wxGetBitmapFromMemory(clip_normal));
		break;
	case fluo::PRMFrame:
		panel_.m_toolbar->SetToolNormalBitmap(panel_.ID_PlaneModesBtn,
			wxGetBitmapFromMemory(clip_frame));
		break;
	case fluo::PRMLowTrans:
		panel_.m_toolbar->SetToolNormalBitmap(panel_.ID_PlaneModesBtn,
			wxGetBitmapFromMemory(clip_low));
		break;
	case fluo::PRMLowTransBack:
		panel_.m_toolbar->SetToolNormalBitmap(panel_.ID_PlaneModesBtn,
			wxGetBitmapFromMemory(clip_low_back));
		break;
	case fluo::PRMNormalBack:
		panel_.m_toolbar->SetToolNormalBitmap(panel_.ID_PlaneModesBtn,
			wxGetBitmapFromMemory(clip_normal_back));
		break;
	case fluo::PRMNone:
		panel_.m_toolbar->SetToolNormalBitmap(panel_.ID_PlaneModesBtn,
			wxGetBitmapFromMemory(clip_none));
		break;
	}

	//ranges
	long resx, resy, resz;
	bool result = getValue(gstResX, resx);
	if (!result)
	{
		panel_.DisableAll();
		return;
	}
	getValue(gstResY, resy);
	getValue(gstResZ, resz);
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

	double distx, disty, distz;
	getValue(gstClipDistX, distx);
	getValue(gstClipDistY, disty);
	getValue(gstClipDistZ, distz);
	panel_.m_yz_dist_text->ChangeValue(
		wxString::Format("%d", int(distx*resx + 0.5)));
	panel_.m_xz_dist_text->ChangeValue(
		wxString::Format("%d", int(disty*resy + 0.5)));
	panel_.m_xy_dist_text->ChangeValue(
		wxString::Format("%d", int(distz*resz + 0.5)));

	//clip values
	int ival = 0;
	wxString str;
	double x1, x2, y1, y2, z1, z2;
	getValue(gstClipX1, x1);
	getValue(gstClipX2, x2);
	getValue(gstClipY1, y1);
	getValue(gstClipY2, y2);
	getValue(gstClipZ1, z1);
	getValue(gstClipZ2, z2);
	double percent;
	int barsize;
	//x1
	ival = int(x1 * resx + 0.5);
	panel_.m_x1_clip_sldr->SetValue(ival);
	str = wxString::Format("%d", ival);
	panel_.m_x1_clip_text->ChangeValue(str);
	percent = (double)ival / (double)panel_.m_x1_clip_sldr->GetMax();
	//x2
	ival = int(x2 * resx + 0.5);
	panel_.m_x2_clip_sldr->SetValue(ival);
	str = wxString::Format("%d", ival);
	panel_.m_x2_clip_text->ChangeValue(str);
	//xbar
	barsize = (panel_.m_x1_clip_sldr->GetSize().GetHeight() - 20);
	panel_.m_xBar->SetPosition(wxPoint(20, 10 + percent * barsize));
	panel_.m_xBar->SetSize(wxSize(3, barsize*((double)
		(ival - panel_.m_x1_clip_sldr->GetValue()) /
		(double)panel_.m_x1_clip_sldr->GetMax())));
	//y1
	ival = int(y1 * resy + 0.5);
	panel_.m_y1_clip_sldr->SetValue(ival);
	str = wxString::Format("%d", ival);
	panel_.m_y1_clip_text->ChangeValue(str);
	percent = (double)ival / (double)panel_.m_y1_clip_sldr->GetMax();
	//y2
	ival = int(y2 * resy + 0.5);
	panel_.m_y2_clip_sldr->SetValue(ival);
	str = wxString::Format("%d", ival);
	panel_.m_y2_clip_text->ChangeValue(str);
	//ybar
	barsize = (panel_.m_y1_clip_sldr->GetSize().GetHeight() - 20);
	panel_.m_yBar->SetPosition(wxPoint(20, 10 + percent * barsize));
	panel_.m_yBar->SetSize(wxSize(3, barsize*((double)
		(ival - panel_.m_y1_clip_sldr->GetValue()) /
		(double)panel_.m_y1_clip_sldr->GetMax())));
	//z1
	ival = int(z1 * resz + 0.5);
	panel_.m_z1_clip_sldr->SetValue(ival);
	str = wxString::Format("%d", ival);
	panel_.m_z1_clip_text->ChangeValue(str);
	percent = (double)ival / (double)panel_.m_z1_clip_sldr->GetMax();
	//z2
	ival = int(z2 * resz + 0.5);
	panel_.m_z2_clip_sldr->SetValue(ival);
	str = wxString::Format("%d", ival);
	panel_.m_z2_clip_text->ChangeValue(str);
	//zbar
	barsize = (panel_.m_z1_clip_sldr->GetSize().GetHeight() - 20);
	panel_.m_zBar->SetPosition(wxPoint(20, 10 + percent * barsize));
	panel_.m_zBar->SetSize(wxSize(3, barsize*((double)
		(ival - panel_.m_z1_clip_sldr->GetValue()) /
		(double)panel_.m_z1_clip_sldr->GetMax())));

	//clip link
	bool clip_link_x, clip_link_y, clip_link_z;
	getValue(gstClipLinkX, clip_link_x);
	getValue(gstClipLinkY, clip_link_y);
	getValue(gstClipLinkZ, clip_link_z);
	panel_.m_link_x_tb->ToggleTool(
		ClipPlanePanel::ID_LinkXChk, clip_link_x);
	if (clip_link_x)
	{
		panel_.m_link_x_tb->SetToolNormalBitmap(
			ClipPlanePanel::ID_LinkXChk,
			wxGetBitmapFromMemory(link));
		panel_.m_x1_clip_sldr->SetPageSize(int(distx*resx + 0.5));
		panel_.m_x2_clip_sldr->SetPageSize(int(distx*resx + 0.5));
	}
	else
	{
		panel_.m_link_x_tb->SetToolNormalBitmap(
			ClipPlanePanel::ID_LinkXChk,
			wxGetBitmapFromMemory(unlink));
		panel_.m_x1_clip_sldr->SetPageSize(std::max(int(resx) / 20, 1));
		panel_.m_x2_clip_sldr->SetPageSize(std::max(int(resx) / 20, 1));
	}
	panel_.m_link_y_tb->ToggleTool(
		ClipPlanePanel::ID_LinkYChk, clip_link_y);
	if (clip_link_y)
	{
		panel_.m_link_y_tb->SetToolNormalBitmap(
			ClipPlanePanel::ID_LinkYChk,
			wxGetBitmapFromMemory(link));
		panel_.m_y1_clip_sldr->SetPageSize(int(disty*resy + 0.5));
		panel_.m_y2_clip_sldr->SetPageSize(int(disty*resy + 0.5));
	}
	else
	{
		panel_.m_link_y_tb->SetToolNormalBitmap(
			ClipPlanePanel::ID_LinkYChk,
			wxGetBitmapFromMemory(unlink));
		panel_.m_y1_clip_sldr->SetPageSize(std::max(int(resy) / 20, 1));
		panel_.m_y2_clip_sldr->SetPageSize(std::max(int(resy) / 20, 1));
	}
	panel_.m_link_z_tb->ToggleTool(
		ClipPlanePanel::ID_LinkZChk, clip_link_z);
	if (clip_link_z)
	{
		panel_.m_link_z_tb->SetToolNormalBitmap(
			ClipPlanePanel::ID_LinkZChk,
			wxGetBitmapFromMemory(link));
		panel_.m_z1_clip_sldr->SetPageSize(int(distz*resz + 0.5));
		panel_.m_z2_clip_sldr->SetPageSize(int(distz*resz + 0.5));
	}
	else
	{
		panel_.m_link_z_tb->SetToolNormalBitmap(
			ClipPlanePanel::ID_LinkZChk,
			wxGetBitmapFromMemory(unlink));
		panel_.m_z1_clip_sldr->SetPageSize(std::max(int(resz) / 20, 1));
		panel_.m_z2_clip_sldr->SetPageSize(std::max(int(resz) / 20, 1));
	}

	//clip rotations
	double clip_rot_x, clip_rot_y, clip_rot_z;
	getValue(gstClipRotX, clip_rot_x);
	getValue(gstClipRotY, clip_rot_y);
	getValue(gstClipRotZ, clip_rot_z);
	panel_.m_x_rot_sldr->SetThumbPosition(int(180.5 - clip_rot_x));
	panel_.m_x_rot_text->ChangeValue(wxString::Format("%.1f", clip_rot_x));
	panel_.m_y_rot_sldr->SetThumbPosition(int(180.5 - clip_rot_y));
	panel_.m_y_rot_text->ChangeValue(wxString::Format("%.1f", clip_rot_y));
	panel_.m_z_rot_sldr->SetThumbPosition(int(180.5 - clip_rot_z));
	panel_.m_z_rot_text->ChangeValue(wxString::Format("%.1f", clip_rot_z));
}

void ClipPlaneAgent::alignRenderViewRot()
{
	Node* obj = getObject();
	if (!obj)
		return;
	SearchVisitor visitor;
	visitor.setTraversalMode(NodeVisitor::TRAVERSE_PARENTS);
	visitor.matchClassName("Renderview");
	obj->accept(visitor);
	Renderview* rv = visitor.getRenderview();
	if (!rv)
		return;
	InterfaceAgent* rv_agent = glbin_agtf->findFirst(rv->getName());
	if (!rv_agent)
		return;
	RenderCanvasAgent* agent = dynamic_cast<RenderCanvasAgent*>(rv_agent);
	if (!agent)
		return;
	double rot_x, rot_y, rot_z;
	agent->getValue(gstCamRotX, rot_x);
	agent->getValue(gstCamRotY, rot_y);
	agent->getValue(gstCamRotZ, rot_z);
	//convert
	Quaternion q_cl;
	q_cl.FromEuler(rot_x, -rot_y, -rot_z);
	q_cl.ToEuler(rot_x, rot_y, rot_z);
	if (rot_x > 180.0) rot_x -= 360.0;
	if (rot_y > 180.0) rot_y -= 360.0;
	if (rot_z > 180.0) rot_z -= 360.0;
	setValue(gstClipRotX, rot_x);
	setValue(gstClipRotX, rot_y);
	updValue(gstClipRotX, rot_z);
}

void ClipPlaneAgent::OnClipXChanged(Event& event)
{
	long res;
	getValue(gstResX, res);
	double x1, x2;
	getValue(gstClipX1, x1);
	getValue(gstClipX2, x2);
	int ival;

	//x1
	ival = int(x1 * res + 0.5);
	panel_.m_x1_clip_sldr->SetValue(ival);
	if (!panel_.m_x1_clip_text->HasFocus())
		panel_.m_x1_clip_text->ChangeValue(wxString::Format("%d", ival));
	double percent = (double)ival / (double)panel_.m_x1_clip_sldr->GetMax();
	//x2
	ival = int(x2 * res + 0.5);
	panel_.m_x2_clip_sldr->SetValue(ival);
	if (!panel_.m_x2_clip_text->HasFocus())
		panel_.m_x2_clip_text->ChangeValue(wxString::Format("%d", ival));
	//bar
	int barsize = (panel_.m_x1_clip_sldr->GetSize().GetHeight() - 20);
	panel_.m_xBar->SetPosition(wxPoint(20, 10 + percent * barsize));
	panel_.m_xBar->SetSize(wxSize(3, barsize*((double)
		(ival - panel_.m_x1_clip_sldr->GetValue()) /
		(double)panel_.m_x1_clip_sldr->GetMax())));
}

void ClipPlaneAgent::OnClipYChanged(Event& event)
{
	long res;
	getValue(gstResY, res);
	double y1, y2;
	getValue(gstClipY1, y1);
	getValue(gstClipY2, y2);
	int ival;

	//y1
	ival = int(y1 * res + 0.5);
	panel_.m_y1_clip_sldr->SetValue(ival);
	if (!panel_.m_y1_clip_text->HasFocus())
		panel_.m_y1_clip_text->ChangeValue(wxString::Format("%d", ival));
	double percent = (double)ival / (double)panel_.m_y1_clip_sldr->GetMax();
	//y2
	ival = int(y2 * res + 0.5);
	panel_.m_y2_clip_sldr->SetValue(ival);
	if (!panel_.m_y2_clip_text->HasFocus())
		panel_.m_y2_clip_text->ChangeValue(wxString::Format("%d", ival));
	//ybar
	double barsize = (panel_.m_y1_clip_sldr->GetSize().GetHeight() - 20);
	panel_.m_yBar->SetPosition(wxPoint(20, 10 + percent * barsize));
	panel_.m_yBar->SetSize(wxSize(3, barsize*((double)
		(ival - panel_.m_y1_clip_sldr->GetValue()) /
		(double)panel_.m_y1_clip_sldr->GetMax())));
}

void ClipPlaneAgent::OnClipZChanged(Event& event)
{
	long res;
	getValue(gstResZ, res);
	double z1, z2;
	getValue(gstClipZ1, z1);
	getValue(gstClipZ2, z2);
	int ival;

	//z1
	ival = int(z1 * res + 0.5);
	panel_.m_z1_clip_sldr->SetValue(ival);
	if (!panel_.m_z1_clip_text->HasFocus())
		panel_.m_z1_clip_text->ChangeValue(wxString::Format("%d", ival));
	double percent = (double)ival / (double)panel_.m_z1_clip_sldr->GetMax();
	//z2
	ival = int(z2 * res + 0.5);
	panel_.m_z2_clip_sldr->SetValue(ival);
	if (!panel_.m_z2_clip_text->HasFocus())
		panel_.m_z2_clip_text->ChangeValue(wxString::Format("%d", ival));
	//zbar
	double barsize = (panel_.m_z1_clip_sldr->GetSize().GetHeight() - 20);
	panel_.m_zBar->SetPosition(wxPoint(20, 10 + percent * barsize));
	panel_.m_zBar->SetSize(wxSize(3, barsize*((double)
		(ival - panel_.m_z1_clip_sldr->GetValue()) /
		(double)panel_.m_z1_clip_sldr->GetMax())));
}

void ClipPlaneAgent::OnClipDistXChanged(Event& event)
{
	double clip_dist;
	getValue(gstClipDistX, clip_dist);
	long res;
	getValue(gstResX, res);
	bool set_dist = false;
	if (clip_dist > 1.0)
	{
		clip_dist = 1.0;
		set_dist = true;
	}
	if (clip_dist <= 0.0)
	{
		clip_dist = 1.0 / res;
		set_dist = true;
	}
	double x1, x2, center;
	getValue(gstClipX1, x1);
	getValue(gstClipX2, x2);
	center = (x1 + x2) / 2;
	x1 = center - clip_dist / 2;
	x1 = x1 < 0.0 ? 0.0 : x1;
	x1 = std::round(x1 * res) / res;
	x2 = x1 + clip_dist;
	if (x2 > 1.0)
	{
		x2 = 1.0;
		x1 = x2 - clip_dist;
	}

	if (set_dist) updValue(gstClipDistX, clip_dist, event);
	updValue(gstClipX1, x1, event);
	updValue(gstClipX2, x2, event);
	if (!panel_.m_yz_dist_text->HasFocus())
		panel_.m_yz_dist_text->ChangeValue(
			wxString::Format("%d", int(clip_dist * res + 0.5)));

	bool clip_link;
	getValue(gstClipLinkX, clip_link);
	if (clip_link)
	{
		panel_.m_x1_clip_sldr->SetPageSize(int(clip_dist*res + 0.5));
		panel_.m_x2_clip_sldr->SetPageSize(int(clip_dist*res + 0.5));
	}
}

void ClipPlaneAgent::OnClipDistYChanged(Event& event)
{
	double clip_dist;
	getValue(gstClipDistY, clip_dist);
	long res;
	getValue(gstResY, res);
	bool set_dist = false;
	if (clip_dist > 1.0)
	{
		clip_dist = 1.0;
		set_dist = true;
	}
	if (clip_dist <= 0.0)
	{
		clip_dist = 1.0 / res;
		set_dist = true;
	}
	double y1, y2, center;
	getValue(gstClipY1, y1);
	getValue(gstClipY2, y2);
	center = (y1 + y2) / 2;
	y1 = center - clip_dist / 2;
	y1 = y1 < 0.0 ? 0.0 : y1;
	y1 = std::round(y1 * res) / res;
	y2 = y1 + clip_dist;
	if (y2 > 1.0)
	{
		y2 = 1.0;
		y1 = y2 - clip_dist;
	}

	if (set_dist) updValue(gstClipDistY, clip_dist, event);
	updValue(gstClipY1, y1, event);
	updValue(gstClipY2, y2, event);
	if (!panel_.m_xz_dist_text->HasFocus())
		panel_.m_xz_dist_text->ChangeValue(
			wxString::Format("%d", int(clip_dist * res + 0.5)));

	bool clip_link;
	getValue(gstClipLinkY, clip_link);
	if (clip_link)
	{
		panel_.m_y1_clip_sldr->SetPageSize(int(clip_dist*res + 0.5));
		panel_.m_y2_clip_sldr->SetPageSize(int(clip_dist*res + 0.5));
	}
}

void ClipPlaneAgent::OnClipDistZChanged(Event& event)
{
	double clip_dist;
	getValue(gstClipDistZ, clip_dist);
	long res;
	getValue(gstResZ, res);
	bool set_dist = false;
	if (clip_dist > 1.0)
	{
		clip_dist = 1.0;
		set_dist = true;
	}
	if (clip_dist <= 0.0)
	{
		clip_dist = 1.0 / res;
		set_dist = true;
	}
	double z1, z2, center;
	getValue(gstClipZ1, z1);
	getValue(gstClipZ2, z2);
	center = (z1 + z2) / 2;
	z1 = center - clip_dist / 2;
	z1 = z1 < 0.0 ? 0.0 : z1;
	z1 = std::round(z1 * res) / res;
	z2 = z1 + clip_dist;
	if (z2 > 1.0)
	{
		z2 = 1.0;
		z1 = z2 - clip_dist;
	}

	if (set_dist) updValue(gstClipDistZ, clip_dist, event);
	updValue(gstClipZ1, z1, event);
	updValue(gstClipZ2, z2, event);
	if (!panel_.m_xy_dist_text->HasFocus())
		panel_.m_xy_dist_text->ChangeValue(
			wxString::Format("%d", int(clip_dist * res + 0.5)));

	bool clip_link;
	getValue(gstClipLinkZ, clip_link);
	if (clip_link)
	{
		panel_.m_z1_clip_sldr->SetPageSize(int(clip_dist*res + 0.5));
		panel_.m_z2_clip_sldr->SetPageSize(int(clip_dist*res + 0.5));
	}
}

void ClipPlaneAgent::OnClipLinkXChanged(Event& event)
{
	bool bval;
	getValue(gstClipLinkX, bval);
	panel_.m_link_x_tb->ToggleTool(
		ClipPlanePanel::ID_LinkXChk, bval);
	long res;
	getValue(gstResX, res);
	double clip_dist;
	getValue(gstClipDistX, clip_dist);
	if (bval)
	{
		panel_.m_link_x_tb->SetToolNormalBitmap(
			ClipPlanePanel::ID_LinkXChk,
			wxGetBitmapFromMemory(link));
		panel_.m_x1_clip_sldr->SetPageSize(int(clip_dist*res + 0.5));
		panel_.m_x2_clip_sldr->SetPageSize(int(clip_dist*res + 0.5));
	}
	else
	{
		panel_.m_link_x_tb->SetToolNormalBitmap(
			ClipPlanePanel::ID_LinkXChk,
			wxGetBitmapFromMemory(unlink));
		panel_.m_x1_clip_sldr->SetPageSize(std::max(int(res) / 20, 1));
		panel_.m_x2_clip_sldr->SetPageSize(std::max(int(res) / 20, 1));
	}
}

void ClipPlaneAgent::OnClipLinkYChanged(Event& event)
{
	bool bval;
	getValue(gstClipLinkY, bval);
	panel_.m_link_y_tb->ToggleTool(
		ClipPlanePanel::ID_LinkYChk, bval);
	long res;
	getValue(gstResY, res);
	double clip_dist;
	getValue(gstClipDistY, clip_dist);
	if (bval)
	{
		panel_.m_link_y_tb->SetToolNormalBitmap(
			ClipPlanePanel::ID_LinkYChk,
			wxGetBitmapFromMemory(link));
		panel_.m_y1_clip_sldr->SetPageSize(int(clip_dist*res + 0.5));
		panel_.m_y2_clip_sldr->SetPageSize(int(clip_dist*res + 0.5));
	}
	else
	{
		panel_.m_link_y_tb->SetToolNormalBitmap(
			ClipPlanePanel::ID_LinkYChk,
			wxGetBitmapFromMemory(unlink));
		panel_.m_y1_clip_sldr->SetPageSize(std::max(int(res) / 20, 1));
		panel_.m_y2_clip_sldr->SetPageSize(std::max(int(res) / 20, 1));
	}
}

void ClipPlaneAgent::OnClipLinkZChanged(Event& event)
{
	bool bval;
	getValue(gstClipLinkZ, bval);
	panel_.m_link_z_tb->ToggleTool(
		ClipPlanePanel::ID_LinkZChk, bval);
	long res;
	getValue(gstResZ, res);
	double clip_dist;
	getValue(gstClipDistZ, clip_dist);
	if (bval)
	{
		panel_.m_link_z_tb->SetToolNormalBitmap(
			ClipPlanePanel::ID_LinkZChk,
			wxGetBitmapFromMemory(link));
		panel_.m_z1_clip_sldr->SetPageSize(int(clip_dist*res + 0.5));
		panel_.m_z2_clip_sldr->SetPageSize(int(clip_dist*res + 0.5));
	}
	else
	{
		panel_.m_link_z_tb->SetToolNormalBitmap(
			ClipPlanePanel::ID_LinkZChk,
			wxGetBitmapFromMemory(unlink));
		panel_.m_z1_clip_sldr->SetPageSize(std::max(int(res) / 20, 1));
		panel_.m_z2_clip_sldr->SetPageSize(std::max(int(res) / 20, 1));
	}
}

void ClipPlaneAgent::OnClipRotXChanged(Event& event)
{
	double dval;
	getValue(gstClipRotX, dval);
	panel_.m_x_rot_sldr->SetThumbPosition(int(180.5 - dval));
	if (!panel_.m_x_rot_text->HasFocus())
		panel_.m_x_rot_text->ChangeValue(wxString::Format("%.1f", dval));
}

void ClipPlaneAgent::OnClipRotYChanged(Event& event)
{
	double dval;
	getValue(gstClipRotY, dval);
	panel_.m_y_rot_sldr->SetThumbPosition(int(180.5 - dval));
	if (!panel_.m_y_rot_text->HasFocus())
		panel_.m_y_rot_text->ChangeValue(wxString::Format("%.1f", dval));
}

void ClipPlaneAgent::OnClipRotZChanged(Event& event)
{
	double dval;
	getValue(gstClipRotZ, dval);
	panel_.m_z_rot_sldr->SetThumbPosition(int(180.5 - dval));
	if (!panel_.m_z_rot_text->HasFocus())
		panel_.m_z_rot_text->ChangeValue(wxString::Format("%.1f", dval));
}
