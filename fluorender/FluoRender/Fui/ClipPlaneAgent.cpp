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

	//clip rotations
	double clip_rot_x, clip_rot_y, clip_rot_z;
	getValue("clip rot x", clip_rot_x);
	getValue("clip rot y", clip_rot_y);
	getValue("clip rot z", clip_rot_z);
	panel_.m_x_rot_sldr->SetValue(int(clip_rot_x + 0.5));
	panel_.m_x_rot_text->ChangeValue(wxString::Format("%.1f", clip_rot_x));
	panel_.m_y_rot_sldr->SetValue(int(clip_rot_y + 0.5));
	panel_.m_y_rot_text->ChangeValue(wxString::Format("%.1f", clip_rot_y));
	panel_.m_z_rot_sldr->SetValue(int(clip_rot_z + 0.5));
	panel_.m_z_rot_text->ChangeValue(wxString::Format("%.1f", clip_rot_z));
}

void ClipPlaneAgent::OnClipXChanged()
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
	panel_.m_x1_clip_text->ChangeValue(wxString::Format("%d", ival));
	double percent = (double)ival / (double)panel_.m_x1_clip_sldr->GetMax();
	//x2
	ival = int(x2 * res + 0.5);
	panel_.m_x2_clip_sldr->SetValue(ival);
	panel_.m_x2_clip_text->ChangeValue(wxString::Format("%d", ival));
	//bar
	int barsize = (panel_.m_x1_clip_sldr->GetSize().GetHeight() - 20);
	panel_.m_xBar->SetPosition(wxPoint(20, 10 + percent * barsize));
	panel_.m_xBar->SetSize(wxSize(3, barsize*((double)
		(ival - panel_.m_x1_clip_sldr->GetValue()) /
		(double)panel_.m_x1_clip_sldr->GetMax())));
}

void ClipPlaneAgent::OnClipYChanged()
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
	panel_.m_y1_clip_text->ChangeValue(wxString::Format("%d", ival));
	double percent = (double)ival / (double)panel_.m_y1_clip_sldr->GetMax();
	//y2
	ival = int(y2 * res + 0.5);
	panel_.m_y2_clip_sldr->SetValue(ival);
	panel_.m_y2_clip_text->ChangeValue(wxString::Format("%d", ival));
	//ybar
	double barsize = (panel_.m_y1_clip_sldr->GetSize().GetHeight() - 20);
	panel_.m_yBar->SetPosition(wxPoint(20, 10 + percent * barsize));
	panel_.m_yBar->SetSize(wxSize(3, barsize*((double)
		(ival - panel_.m_y1_clip_sldr->GetValue()) /
		(double)panel_.m_y1_clip_sldr->GetMax())));
}

void ClipPlaneAgent::OnClipZChanged()
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
	panel_.m_z1_clip_text->ChangeValue(wxString::Format("%d", ival));
	double percent = (double)ival / (double)panel_.m_z1_clip_sldr->GetMax();
	//z2
	ival = int(z2 * res + 0.5);
	panel_.m_z2_clip_sldr->SetValue(ival);
	panel_.m_z2_clip_text->ChangeValue(wxString::Format("%d", ival));
	//zbar
	double barsize = (panel_.m_z1_clip_sldr->GetSize().GetHeight() - 20);
	panel_.m_zBar->SetPosition(wxPoint(20, 10 + percent * barsize));
	panel_.m_zBar->SetSize(wxSize(3, barsize*((double)
		(ival - panel_.m_z1_clip_sldr->GetValue()) /
		(double)panel_.m_z1_clip_sldr->GetMax())));
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

void ClipPlaneAgent::OnClipLinkXChanged()
{
	bool bval;
	getValue("clip link x", bval);
	panel_.m_link_x_tb->ToggleTool(
		ClipPlanePanel::ID_LinkXChk, bval);
}

void ClipPlaneAgent::OnClipLinkYChanged()
{
	bool bval;
	getValue("clip link y", bval);
	panel_.m_link_y_tb->ToggleTool(
		ClipPlanePanel::ID_LinkYChk, bval);
}

void ClipPlaneAgent::OnClipLinkZChanged()
{
	bool bval;
	getValue("clip link z", bval);
	panel_.m_link_z_tb->ToggleTool(
		ClipPlanePanel::ID_LinkZChk, bval);
}
