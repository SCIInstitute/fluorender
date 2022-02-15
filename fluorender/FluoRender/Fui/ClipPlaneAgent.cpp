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

#include <ClipPlaneAgent.hpp>
#include <ClipPlanePanel.h>
#include <SearchVisitor.hpp>
#include <RenderCanvasAgent.hpp>
#include <Global.hpp>
#include <RenderCanvas.h>
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
	getValue("clip hold", bval);
	panel_.m_toolbar->ToggleTool(panel_.ID_HoldPlanesBtn, bval);
	long render_mode;
	getValue("clip render mode", render_mode);
	switch (render_mode)
	{
	case FLTYPE::PRMNormal:
		panel_.m_toolbar->SetToolNormalBitmap(panel_.ID_PlaneModesBtn,
			wxGetBitmapFromMemory(clip_normal));
		break;
	case FLTYPE::PRMFrame:
		panel_.m_toolbar->SetToolNormalBitmap(panel_.ID_PlaneModesBtn,
			wxGetBitmapFromMemory(clip_frame));
		break;
	case FLTYPE::PRMLowTrans:
		panel_.m_toolbar->SetToolNormalBitmap(panel_.ID_PlaneModesBtn,
			wxGetBitmapFromMemory(clip_low));
		break;
	case FLTYPE::PRMLowTransBack:
		panel_.m_toolbar->SetToolNormalBitmap(panel_.ID_PlaneModesBtn,
			wxGetBitmapFromMemory(clip_low_back));
		break;
	case FLTYPE::PRMNormalBack:
		panel_.m_toolbar->SetToolNormalBitmap(panel_.ID_PlaneModesBtn,
			wxGetBitmapFromMemory(clip_normal_back));
		break;
	case FLTYPE::PRMNone:
		panel_.m_toolbar->SetToolNormalBitmap(panel_.ID_PlaneModesBtn,
			wxGetBitmapFromMemory(clip_none));
		break;
	}

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

	double distx, disty, distz;
	getValue("clip dist x", distx);
	getValue("clip dist y", disty);
	getValue("clip dist z", distz);
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
	getValue("clip x1", x1);
	getValue("clip x2", x2);
	getValue("clip y1", y1);
	getValue("clip y2", y2);
	getValue("clip z1", z1);
	getValue("clip z2", z2);
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
	getValue("clip link x", clip_link_x);
	getValue("clip link y", clip_link_y);
	getValue("clip link z", clip_link_z);
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
	getValue("clip rot x", clip_rot_x);
	getValue("clip rot y", clip_rot_y);
	getValue("clip rot z", clip_rot_z);
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
	visitor.matchClassName("RenderView");
	obj->accept(visitor);
	RenderView* rv = visitor.getRenderView();
	if (!rv)
		return;
	InterfaceAgent* rv_agent =
		Global::instance().getAgentFactory().findFirst(rv->getName());
	if (!rv_agent)
		return;
	RenderCanvasAgent* agent = dynamic_cast<RenderCanvasAgent*>(rv_agent);
	if (!agent)
		return;
	double rot_x, rot_y, rot_z;
	agent->getView().GetRotations(rot_x, rot_y, rot_z);
	//convert
	FLTYPE::Quaternion q_cl;
	q_cl.FromEuler(rot_x, -rot_y, -rot_z);
	q_cl.ToEuler(rot_x, rot_y, rot_z);
	if (rot_x > 180.0) rot_x -= 360.0;
	if (rot_y > 180.0) rot_y -= 360.0;
	if (rot_z > 180.0) rot_z -= 360.0;
	setValue("clip rot x", rot_x);
	setValue("clip rot y", rot_y);
	setValue("clip rot z", rot_z);
}

void ClipPlaneAgent::OnClipXChanged(Event& event)
{
	long res;
	getValue("res x", res);
	double x1, x2;
	getValue("clip x1", x1);
	getValue("clip x2", x2);
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
	getValue("res y", res);
	double y1, y2;
	getValue("clip y1", y1);
	getValue("clip y2", y2);
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
	getValue("res z", res);
	double z1, z2;
	getValue("clip z1", z1);
	getValue("clip z2", z2);
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
	getValue("clip dist x", clip_dist);
	long res;
	getValue("res x", res);
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
	getValue("clip x1", x1);
	getValue("clip x2", x2);
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

	if (set_dist) setValue("clip dist x", clip_dist, event);
	setValue("clip x1", x1, event);
	setValue("clip x2", x2, event);
	if (!panel_.m_yz_dist_text->HasFocus())
		panel_.m_yz_dist_text->ChangeValue(
			wxString::Format("%d", int(clip_dist * res + 0.5)));

	bool clip_link;
	getValue("clip link x", clip_link);
	if (clip_link)
	{
		panel_.m_x1_clip_sldr->SetPageSize(int(clip_dist*res + 0.5));
		panel_.m_x2_clip_sldr->SetPageSize(int(clip_dist*res + 0.5));
	}
}

void ClipPlaneAgent::OnClipDistYChanged(Event& event)
{
	double clip_dist;
	getValue("clip dist y", clip_dist);
	long res;
	getValue("res y", res);
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
	getValue("clip y1", y1);
	getValue("clip y2", y2);
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

	if (set_dist) setValue("clip dist y", clip_dist, event);
	setValue("clip y1", y1, event);
	setValue("clip y2", y2, event);
	if (!panel_.m_xz_dist_text->HasFocus())
		panel_.m_xz_dist_text->ChangeValue(
			wxString::Format("%d", int(clip_dist * res + 0.5)));

	bool clip_link;
	getValue("clip link y", clip_link);
	if (clip_link)
	{
		panel_.m_y1_clip_sldr->SetPageSize(int(clip_dist*res + 0.5));
		panel_.m_y2_clip_sldr->SetPageSize(int(clip_dist*res + 0.5));
	}
}

void ClipPlaneAgent::OnClipDistZChanged(Event& event)
{
	double clip_dist;
	getValue("clip dist z", clip_dist);
	long res;
	getValue("res z", res);
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
	getValue("clip z1", z1);
	getValue("clip z2", z2);
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

	if (set_dist) setValue("clip dist z", clip_dist, event);
	setValue("clip z1", z1, event);
	setValue("clip z2", z2, event);
	if (!panel_.m_xy_dist_text->HasFocus())
		panel_.m_xy_dist_text->ChangeValue(
			wxString::Format("%d", int(clip_dist * res + 0.5)));

	bool clip_link;
	getValue("clip link z", clip_link);
	if (clip_link)
	{
		panel_.m_z1_clip_sldr->SetPageSize(int(clip_dist*res + 0.5));
		panel_.m_z2_clip_sldr->SetPageSize(int(clip_dist*res + 0.5));
	}
}

void ClipPlaneAgent::OnClipLinkXChanged(Event& event)
{
	bool bval;
	getValue("clip link x", bval);
	panel_.m_link_x_tb->ToggleTool(
		ClipPlanePanel::ID_LinkXChk, bval);
	long res;
	getValue("res x", res);
	double clip_dist;
	getValue("clip dist x", clip_dist);
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
	getValue("clip link y", bval);
	panel_.m_link_y_tb->ToggleTool(
		ClipPlanePanel::ID_LinkYChk, bval);
	long res;
	getValue("res y", res);
	double clip_dist;
	getValue("clip dist y", clip_dist);
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
	getValue("clip link z", bval);
	panel_.m_link_z_tb->ToggleTool(
		ClipPlanePanel::ID_LinkZChk, bval);
	long res;
	getValue("res z", res);
	double clip_dist;
	getValue("clip dist z", clip_dist);
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
	getValue("clip rot x", dval);
	panel_.m_x_rot_sldr->SetThumbPosition(int(180.5 - dval));
	if (!panel_.m_x_rot_text->HasFocus())
		panel_.m_x_rot_text->ChangeValue(wxString::Format("%.1f", dval));
}

void ClipPlaneAgent::OnClipRotYChanged(Event& event)
{
	double dval;
	getValue("clip rot y", dval);
	panel_.m_y_rot_sldr->SetThumbPosition(int(180.5 - dval));
	if (!panel_.m_y_rot_text->HasFocus())
		panel_.m_y_rot_text->ChangeValue(wxString::Format("%.1f", dval));
}

void ClipPlaneAgent::OnClipRotZChanged(Event& event)
{
	double dval;
	getValue("clip rot z", dval);
	panel_.m_z_rot_sldr->SetThumbPosition(int(180.5 - dval));
	if (!panel_.m_z_rot_text->HasFocus())
		panel_.m_z_rot_text->ChangeValue(wxString::Format("%.1f", dval));
}
