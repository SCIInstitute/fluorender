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

void ClipPlaneAgent::objectChanging(void* ptr, void* orig_node, const std::string &exp)
{
	//before change
}

void ClipPlaneAgent::objectChanged(void* ptr, void* orig_node, const std::string &exp)
{
	//set values in ui
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
	//m_vd->GetClipDistance(distx, disty, distz);
	getValue("clip dist x", distx);
	getValue("clip dist y", disty);
	getValue("clip dist z", distz);
	if (distx == 0)
	{
		distx = resx / 20;
		distx = distx == 0 ? 1 : distx;
	}
	if (disty == 0)
	{
		disty = resy / 20;
		disty = disty == 0 ? 1 : disty;
	}
	if (distz == 0)
	{
		distz = resz / 20;
		distz = distz == 0 ? 1 : distz;
	}
	panel_.m_yz_dist_text->SetValue(
		wxString::Format("%d", distx));
	panel_.m_xz_dist_text->SetValue(
		wxString::Format("%d", disty));
	panel_.m_xy_dist_text->SetValue(
		wxString::Format("%d", distz));
	//m_vd->SetClipDistance(distx, disty, distz);
	setValue("clip dist x", distx);
	setValue("clip dist y", disty);
	setValue("clip dist z", distz);

	FLTYPE::PlaneSet planes;
	getValue("clip planes", planes);
	if (planes.GetSize() != 6)
		return;

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
	panel_.m_xBar->SetPosition(wxPoint(20, 10 + percent*barsize));
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
	panel_.m_yBar->SetPosition(wxPoint(20, 10 + percent*barsize));
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
	panel_.m_zBar->SetPosition(wxPoint(20, 10 + percent*barsize));
	panel_.m_zBar->SetSize(wxSize(3, barsize*((double)
		(ival - panel_.m_z1_clip_sldr->GetValue()) / (double)panel_.m_z1_clip_sldr->GetMax())));
	panel_.m_z2_clip_sldr->SetValue(ival);
	str = wxString::Format("%d", ival);
	panel_.m_z2_clip_text->ChangeValue(str);

}