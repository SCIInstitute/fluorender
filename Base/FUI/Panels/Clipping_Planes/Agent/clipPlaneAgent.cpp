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

/*
#include <wx/valnum.h>
#include <png_resource.h>
#include <img/icons.h>
*/
#include <Panels/Clipping_Planes/clippingPlane.hpp>
#include <SearchVisitor.hpp>
#include <Global/Global.hpp>

//#include <Fui/RenderCanvasAgent.h> Will need to figure out how these work
//#include <VRenderGLView.h>

#include "clipPlaneAgent.hpp"

ClipPlaneAgent::ClipPlaneAgent(ClippingPlane &panel) :
	InterfaceAgent(),
	parentPanel(panel)
{

}

void ClipPlaneAgent::setObject(fluo::Node* obj)
{
  InterfaceAgent::setObject(obj);
}

fluo::Node* ClipPlaneAgent::getObject()
{
  return dynamic_cast<fluo::Node*>(InterfaceAgent::getObject());
}

void ClipPlaneAgent::UpdateAllSettings()
{
/*
  To be enabled/disabled later. As of right now all the panels are enabled.

	if (!getObject())
	{
		parentPanel.DisableAll();
		return;
	}
	else
		parentPanel.EnableAll();
*/
  //states
  bool bval = false;
  long render_mode = 0;
  
  getValue("clip hold", bval);
  //parentPanel.m_toolbar->ToggleTool(parentPanel.ID_HoldPlanesBtn, bval);
  getValue("clip render mode", render_mode);

/* This will be updated later on as I add more buttons
  switch (render_mode)
  {
  case FLTYPE::PRMNormal:
  	parentPanel.m_toolbar->SetToolNormalBitmap(parentPanel.ID_PlaneModesBtn,
  		wxGetBitmapFromMemory(clip_normal));
  	break;
  case FLTYPE::PRMFrame:
  	parentPanel.m_toolbar->SetToolNormalBitmap(parentPanel.ID_PlaneModesBtn,
  		wxGetBitmapFromMemory(clip_frame));
  	break;
  case FLTYPE::PRMLowTrans:
  	parentPanel.m_toolbar->SetToolNormalBitmap(parentPanel.ID_PlaneModesBtn,
  		wxGetBitmapFromMemory(clip_low));
  	break;
  case FLTYPE::PRMLowTransBack:
  	parentPanel.m_toolbar->SetToolNormalBitmap(parentPanel.ID_PlaneModesBtn,
  		wxGetBitmapFromMemory(clip_low_back));
  	break;
  case FLTYPE::PRMNormalBack:
  	parentPanel.m_toolbar->SetToolNormalBitmap(parentPanel.ID_PlaneModesBtn,
  		wxGetBitmapFromMemory(clip_normal_back));
  	break;
  case FLTYPE::PRMNone:
  	parentPanel.m_toolbar->SetToolNormalBitmap(parentPanel.ID_PlaneModesBtn,
  		wxGetBitmapFromMemory(clip_none));
  	break;
  }
*/ 
  //ranges
/* To be updated later as well
  bool result = getValue("res x", resx);
  if (!result)
  {
  	parentPanel.DisableAll();
  	return;
  }
*/
  long resx, resy, resz = 0;
  getValue("res y", resy);
  getValue("res z", resz);

/* I would rather let the sliders handle this. However, this will probably need to be implemented
  //slider range
  parentPanel.m_x1_clip_sldr->SetRange(0, resx);
  parentPanel.m_x2_clip_sldr->SetRange(0, resx);
  parentPanel.m_y1_clip_sldr->SetRange(0, resy);
  parentPanel.m_y2_clip_sldr->SetRange(0, resy);
  parentPanel.m_z1_clip_sldr->SetRange(0, resz);
  parentPanel.m_z2_clip_sldr->SetRange(0, resz);
  //text range
  wxIntegerValidator<int>* vald_i;
  if ((vald_i = (wxIntegerValidator<int>*)parentPanel.m_x1_clip_text->GetValidator()))
  	vald_i->SetRange(0, int(resx));
  if ((vald_i = (wxIntegerValidator<int>*)parentPanel.m_x2_clip_text->GetValidator()))
  	vald_i->SetRange(0, int(resx));
  if ((vald_i = (wxIntegerValidator<int>*)parentPanel.m_y1_clip_text->GetValidator()))
  	vald_i->SetRange(0, int(resy));
  if ((vald_i = (wxIntegerValidator<int>*)parentPanel.m_y2_clip_text->GetValidator()))
  	vald_i->SetRange(0, int(resy));
  if ((vald_i = (wxIntegerValidator<int>*)parentPanel.m_z1_clip_text->GetValidator()))
  	vald_i->SetRange(0, int(resz));
  if ((vald_i = (wxIntegerValidator<int>*)parentPanel.m_z2_clip_text->GetValidator()))
  	vald_i->SetRange(0, int(resz));
*/
 
  double distx, disty, distz = 0.0;
  getValue("clip dist x", distx);
  getValue("clip dist y", disty);
  getValue("clip dist z", distz);
/*
  parentPanel.m_yz_dist_text->ChangeValue(
  	wxString::Format("%d", int(distx*resx + 0.5)));
  parentPanel.m_xz_dist_text->ChangeValue(
  	wxString::Format("%d", int(disty*resy + 0.5)));
  parentPanel.m_xy_dist_text->ChangeValue(
  	wxString::Format("%d", int(distz*resz + 0.5)));
*/
  parentPanel.setClipClipValue(distx);
  parentPanel.setClipSlabValue(disty);
  parentPanel.setClipWidthValue(distz);


  //clip values
  int ival = 0;
  //wxString str;
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
  ival = static_cast<int>(x1 * resx + 0.5);
  parentPanel.setClipSalmonValue(ival);
/*
  parentPanel.m_x1_clip_sldr->SetValue(ival);
  str = wxString::Format("%d", ival);
  parentPanel.m_x1_clip_text->ChangeValue(str);
*/
  percent = static_cast<double>(ival) / static_cast<double>(parentPanel.getClipSalmonMaxVal());

  //x2
  ival = static_cast<int>(x2 * resx + 0.5);
  parentPanel.setClipMagentaValue(ival);
/*
  parentPanel.m_x2_clip_sldr->SetValue(ival);
  str = wxString::Format("%d", ival);
  parentPanel.m_x2_clip_text->ChangeValue(str);

  //xbar //will figure this out later
  barsize = (parentPanel.m_x1_clip_sldr->GetSize().GetHeight() - 20);
  parentPanel.m_xBar->SetPosition(wxPoint(20, 10 + percent * barsize));
  parentPanel.m_xBar->SetSize(wxSize(3, barsize*((double)
  	(ival - parentPanel.m_x1_clip_sldr->GetValue()) /
  	(double)parentPanel.m_x1_clip_sldr->GetMax())));
*/
  //y1
  ival = static_cast<int>(y1 * resy + 0.5);
  parentPanel.setClipGreenValue(ival);
/*
  parentPanel.m_y1_clip_sldr->SetValue(ival);
  str = wxString::Format("%d", ival);
  parentPanel.m_y1_clip_text->ChangeValue(str);
*/
  percent = static_cast<double>(ival) / static_cast<double>(parentPanel.getClipGreenMaxVal());

  //y2
  ival = static_cast<int>(y2 * resy + 0.5);
  parentPanel.setClipYellowValue(ival);
/*
  parentPanel.m_y2_clip_sldr->SetValue(ival);
  str = wxString::Format("%d", ival);
  parentPanel.m_y2_clip_text->ChangeValue(str);

  //ybar will figure this out later
  barsize = (parentPanel.m_y1_clip_sldr->GetSize().GetHeight() - 20);
  parentPanel.m_yBar->SetPosition(wxPoint(20, 10 + percent * barsize));
  parentPanel.m_yBar->SetSize(wxSize(3, barsize*((double)
  	(ival - parentPanel.m_y1_clip_sldr->GetValue()) /
  	(double)parentPanel.m_y1_clip_sldr->GetMax())));
*/
  //z1
  ival = static_cast<int>(z1 * resz + 0.5);
  parentPanel.setClipPurpleValue(ival);
/*
  parentPanel.m_z1_clip_sldr->SetValue(ival);
  str = wxString::Format("%d", ival);
  parentPanel.m_z1_clip_text->ChangeValue(str);
*/
  percent = static_cast<double>(ival) / static_cast<double>(parentPanel.getClipPurpleMaxVal());

  //z2
  ival = static_cast<int>(z2 * resz + 0.5);
  parentPanel.setClipTealValue(ival);
/*
  parentPanel.m_z2_clip_sldr->SetValue(ival);
  str = wxString::Format("%d", ival);
  parentPanel.m_z2_clip_text->ChangeValue(str);

  //zbar will figure out later
  barsize = (parentPanel.m_z1_clip_sldr->GetSize().GetHeight() - 20);
  parentPanel.m_zBar->SetPosition(wxPoint(20, 10 + percent * barsize));
  parentPanel.m_zBar->SetSize(wxSize(3, barsize*((double)
  	(ival - parentPanel.m_z1_clip_sldr->GetValue()) /
  	(double)parentPanel.m_z1_clip_sldr->GetMax())));
*/ 
  //clip link
  bool clip_link_x, clip_link_y, clip_link_z = false; 
  getValue("clip link x", clip_link_x);
  getValue("clip link y", clip_link_y);
  getValue("clip link z", clip_link_z);
  //parentPanel.m_link_x_tb->ToggleTool(ClipPlanePanel::ID_LinkXChk, clip_link_x);
  parentPanel.setXPlaneLockStatus(clip_link_x);
  parentPanel.setYPlaneLockStatus(clip_link_y);
  parentPanel.setZPlaneLockStatus(clip_link_z);
/*
  if (clip_link_x)
  {
  	//parentPanel.m_link_x_tb->SetToolNormalBitmap(
  		//ClipPlanePanel::ID_LinkXChk,
  		//wxGetBitmapFromMemory(link));
  	parentPanel.m_x1_clip_sldr->SetPageSize(int(distx*resx + 0.5));
  	parentPanel.m_x2_clip_sldr->SetPageSize(int(distx*resx + 0.5));
  }
  else
  {
  	parentPanel.m_link_x_tb->SetToolNormalBitmap(
  		ClipPlanePanel::ID_LinkXChk,
  		wxGetBitmapFromMemory(unlink));
  	parentPanel.m_x1_clip_sldr->SetPageSize(std::max(int(resx) / 20, 1));
  	parentPanel.m_x2_clip_sldr->SetPageSize(std::max(int(resx) / 20, 1));
  }
  parentPanel.m_link_y_tb->ToggleTool(
  	ClipPlanePanel::ID_LinkYChk, clip_link_y);
  if (clip_link_y)
  {
  	parentPanel.m_link_y_tb->SetToolNormalBitmap(
  		ClipPlanePanel::ID_LinkYChk,
  		wxGetBitmapFromMemory(link));
  	parentPanel.m_y1_clip_sldr->SetPageSize(int(disty*resy + 0.5));
  	parentPanel.m_y2_clip_sldr->SetPageSize(int(disty*resy + 0.5));
  }
  else
  {
  	parentPanel.m_link_y_tb->SetToolNormalBitmap(
  		ClipPlanePanel::ID_LinkYChk,
  		wxGetBitmapFromMemory(unlink));
  	parentPanel.m_y1_clip_sldr->SetPageSize(std::max(int(resy) / 20, 1));
  	parentPanel.m_y2_clip_sldr->SetPageSize(std::max(int(resy) / 20, 1));
  }
  parentPanel.m_link_z_tb->ToggleTool(
  	ClipPlanePanel::ID_LinkZChk, clip_link_z);
  if (clip_link_z)
  {
  	parentPanel.m_link_z_tb->SetToolNormalBitmap(
  		ClipPlanePanel::ID_LinkZChk,
  		wxGetBitmapFromMemory(link));
  	parentPanel.m_z1_clip_sldr->SetPageSize(int(distz*resz + 0.5));
  	parentPanel.m_z2_clip_sldr->SetPageSize(int(distz*resz + 0.5));
  }
  else
  {
  	parentPanel.m_link_z_tb->SetToolNormalBitmap(
  		ClipPlanePanel::ID_LinkZChk,
  		wxGetBitmapFromMemory(unlink));
  	parentPanel.m_z1_clip_sldr->SetPageSize(std::max(int(resz) / 20, 1));
  	parentPanel.m_z2_clip_sldr->SetPageSize(std::max(int(resz) / 20, 1));
  }
 */ 
  //clip rotations
  double clip_rot_x, clip_rot_y, clip_rot_z = 0.0;
  getValue("clip rot x", clip_rot_x);
  getValue("clip rot y", clip_rot_y);
  getValue("clip rot z", clip_rot_z);

  parentPanel.setClipXRotValue(clip_rot_x);
  parentPanel.setClipYRotValue(clip_rot_y);
  parentPanel.setClipZRotValue(clip_rot_z);

/*
  parentPanel.m_x_rot_sldr->SetThumbPosition(int(180.5 - clip_rot_x));
  parentPanel.m_x_rot_text->ChangeValue(wxString::Format("%.1f", clip_rot_x));
  parentPanel.m_y_rot_sldr->SetThumbPosition(int(180.5 - clip_rot_y));
  parentPanel.m_y_rot_text->ChangeValue(wxString::Format("%.1f", clip_rot_y));
  parentPanel.m_z_rot_sldr->SetThumbPosition(int(180.5 - clip_rot_z));
  parentPanel.m_z_rot_text->ChangeValue(wxString::Format("%.1f", clip_rot_z));
*/
}

/* I'll have to figure this out once I get a renderview part of this
void ClipPlaneAgent::alignRenderViewRot()
{
	fluo::Node* obj = getObject();
	if (!obj)
		return;
	fluo::SearchVisitor visitor;
	visitor.setTraversalMode(fluo::NodeVisitor::TRAVERSE_PARENTS);
	visitor.matchClassName("RenderView");
	obj->accept(visitor);
	fluo::RenderView* rv = visitor.getRenderView();
	if (!rv)
		return;
	InterfaceAgent* rv_agent =
		fluo::Global::instance().getAgentFactory().findFirst(rv->getName());
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
*/
void ClipPlaneAgent::OnClipXChanged(fluo::Event& event)
{
  long res = 0;
  double x1, x2 = 0.0;
  int ival = 0;
  
  getValue("res x", res);
  getValue("clip x1", x1);
  getValue("clip x2", x2);
  
  //x1
  ival = static_cast<int>(x1 * res + 0.5);
  //parentPanel.m_x1_clip_sldr->SetValue(ival);
  //if (!parentPanel.m_x1_clip_text->HasFocus())
  	//parentPanel.m_x1_clip_text->ChangeValue(wxString::Format("%d", ival));
  
  parentPanel.setClipSalmonValue(ival);
  double percent = static_cast<double>(ival) / static_cast<double>(parentPanel.getClipSalmonMaxVal());

  //x2
  ival = static_cast<int>(x2 * res + 0.5);
  //parentPanel.m_x2_clip_sldr->SetValue(ival);
  //if (!parentPanel.m_x2_clip_text->HasFocus())
  	//parentPanel.m_x2_clip_text->ChangeValue(wxString::Format("%d", ival));
  parentPanel.setClipMagentaValue(ival);

  //xbar
/*  will figure this out later
  double barsize = (parentPanel.m_x1_clip_sldr->GetSize().GetHeight() - 20);
  parentPanel.m_xBar->SetPosition(wxPoint(20, 10 + percent * barsize));
  parentPanel.m_xBar->SetSize(wxSize(3, barsize*((double)
  	(ival - parentPanel.m_x1_clip_sldr->GetValue()) /
  	(double)parentPanel.m_x1_clip_sldr->GetMax())));
*/
}

void ClipPlaneAgent::OnClipYChanged(fluo::Event& event)
{
  long res = 0;
  double y1, y2 = 0.0;
  int ival = 0;
  
  getValue("res y", res);
  getValue("clip y1", y1);
  getValue("clip y2", y2);
  
  //y1
  ival = static_cast<int>(y1 * res + 0.5);
  //parentPanel.m_y1_clip_sldr->SetValue(ival);
  //if (!parentPanel.m_y1_clip_text->HasFocus())
  	//parentPanel.m_y1_clip_text->ChangeValue(wxString::Format("%d", ival));
  
  parentPanel.setClipGreenValue(ival);
  double percent = static_cast<double>(ival) / static_cast<double>(parentPanel.getClipGreenMaxVal());

  //y2
  ival = static_cast<int>(y2 * res + 0.5);
  //parentPanel.m_y2_clip_sldr->SetValue(ival);
  //if (!parentPanel.m_y2_clip_text->HasFocus())
  	//parentPanel.m_y2_clip_text->ChangeValue(wxString::Format("%d", ival));
  parentPanel.setClipYellowValue(ival);

  //ybar
/*  will figure this out later
  double barsize = (parentPanel.m_y1_clip_sldr->GetSize().GetHeight() - 20);
  parentPanel.m_yBar->SetPosition(wxPoint(20, 10 + percent * barsize));
  parentPanel.m_yBar->SetSize(wxSize(3, barsize*((double)
  	(ival - parentPanel.m_y1_clip_sldr->GetValue()) /
  	(double)parentPanel.m_y1_clip_sldr->GetMax())));
*/
}

void ClipPlaneAgent::OnClipZChanged(fluo::Event& event)
{
  long res = 0;
  double z1, z2 = 0.0;
  int ival = 0;
  
  getValue("res z", res);
  getValue("clip z1", z1);
  getValue("clip z2", z2);
  
  //z1
  ival = static_cast<int>(z1 * res + 0.5);
  //parentPanel.m_z1_clip_sldr->SetValue(ival);
  //if (!parentPanel.m_z1_clip_text->HasFocus())
  	//parentPanel.m_z1_clip_text->ChangeValue(wxString::Format("%d", ival));
  
  parentPanel.setClipPurpleValue(ival);
  double percent = static_cast<double>(ival) / static_cast<double>(parentPanel.getClipPurpleMaxVal());

  //z2
  ival = static_cast<int>(z2 * res + 0.5);
  //parentPanel.m_z2_clip_sldr->SetValue(ival);
  //if (!parentPanel.m_z2_clip_text->HasFocus())
  	//parentPanel.m_z2_clip_text->ChangeValue(wxString::Format("%d", ival));
  parentPanel.setClipTealValue(ival);

  //zbar
/*  will figure this out later
  double barsize = (parentPanel.m_z1_clip_sldr->GetSize().GetHeight() - 20);
  parentPanel.m_zBar->SetPosition(wxPoint(20, 10 + percent * barsize));
  parentPanel.m_zBar->SetSize(wxSize(3, barsize*((double)
  	(ival - parentPanel.m_z1_clip_sldr->GetValue()) /
  	(double)parentPanel.m_z1_clip_sldr->GetMax())));
*/
}

void ClipPlaneAgent::OnClipDistXChanged(fluo::Event& event)
{
  double clip_dist = 0.0;
  long res = 0;
  bool set_dist = false;
  
  getValue("clip dist x", clip_dist);
  getValue("res x", res);
  
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
  
  double x1, x2, center = 0.0;
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
  
  if (set_dist) 
    setValue("clip dist x", clip_dist, event);
  setValue("clip x1", x1, event);
  setValue("clip x2", x2, event);
  
  //if (!parentPanel.m_yz_dist_text->HasFocus())
    //parentPanel.m_yz_dist_text->ChangeValue(
  		//wxString::Format("%d", int(clip_dist * res + 0.5)));

  parentPanel.setClipClipValue(static_cast<int>(clip_dist * res + 0.5));
  
  bool clip_link = false;
  getValue("clip link x", clip_link);
  if (clip_link)
  {
    //parentPanel.m_x1_clip_sldr->SetPageSize(int(clip_dist*res + 0.5));
    //parentPanel.m_x2_clip_sldr->SetPageSize(int(clip_dist*res + 0.5));
    parentPanel.setClipSalmonValue(static_cast<int>(clip_dist*res + 0.5));
    parentPanel.setClipMagentaValue(static_cast<int>(clip_dist*res + 0.5));
  }
}

void ClipPlaneAgent::OnClipDistYChanged(fluo::Event& event)
{
  double clip_dist = 0.0;
  long res = 0;
  bool set_dist = false;
  
  getValue("clip dist y", clip_dist);
  getValue("res y", res);
  
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
  
  double y1, y2, center = 0.0;
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
  
  if (set_dist) 
    setValue("clip dist y", clip_dist, event);
  setValue("clip y1", y1, event);
  setValue("clip y2", y2, event);
  
  //if (!parentPanel.m_xz_dist_text->HasFocus())
    //parentPanel.m_xz_dist_text->ChangeValue(
  		//wxString::Format("%d", int(clip_dist * res + 0.5)));

  parentPanel.setClipSlabValue(static_cast<int>(clip_dist * res + 0.5));
  
  bool clip_link = false;
  getValue("clip link y", clip_link);
  if (clip_link)
  {
    //parentPanel.m_y1_clip_sldr->SetPageSize(int(clip_dist*res + 0.5));
    //parentPanel.m_y2_clip_sldr->SetPageSize(int(clip_dist*res + 0.5));
    parentPanel.setClipGreenValue(static_cast<int>(clip_dist*res + 0.5));
    parentPanel.setClipYellowValue(static_cast<int>(clip_dist*res + 0.5));
  }
}

void ClipPlaneAgent::OnClipDistZChanged(fluo::Event& event)
{
  double clip_dist = 0.0;
  long res = 0;
  bool set_dist = false;
  
  getValue("clip dist z", clip_dist);
  getValue("res z", res);
  
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
  
  double z1, z2, center = 0.0;
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
  
  if (set_dist) 
    setValue("clip dist z", clip_dist, event);
  setValue("clip z1", z1, event);
  setValue("clip z2", z2, event);
  
  //if (!parentPanel.m_xy_dist_text->HasFocus())
    //parentPanel.m_xy_dist_text->ChangeValue(
  		//wxString::Format("%d", int(clip_dist * res + 0.5)));

  parentPanel.setClipWidthValue(static_cast<int>(clip_dist * res + 0.5));
  
  bool clip_link = false;
  getValue("clip link z", clip_link);
  if (clip_link)
  {
    //parentPanel.m_z1_clip_sldr->SetPageSize(int(clip_dist*res + 0.5));
    //parentPanel.m_z2_clip_sldr->SetPageSize(int(clip_dist*res + 0.5));
    parentPanel.setClipPurpleValue(static_cast<int>(clip_dist*res + 0.5));
    parentPanel.setClipTealValue(static_cast<int>(clip_dist*res + 0.5));
  }
}

void ClipPlaneAgent::OnClipLinkXChanged(fluo::Event& event)
{
  bool bval = false;
  long res = 0;
  double clip_dist = 0.0;
  getValue("clip link x", bval);
  //parentPanel.m_link_x_tb->ToggleTool(ClipPlanePanel::ID_LinkXChk, bval);
  parentPanel.setXPlaneLockStatus(bval);
  getValue("res x", res);
  getValue("clip dist x", clip_dist);
  
if (bval)
  {
    //parentPanel.m_link_x_tb->SetToolNormalBitmap(
    	//ClipPlanePanel::ID_LinkXChk,
    	//wxGetBitmapFromMemory(link));
    //parentPanel.m_x1_clip_sldr->SetPageSize(int(clip_dist*res + 0.5));
    //parentPanel.m_x2_clip_sldr->SetPageSize(int(clip_dist*res + 0.5));
    parentPanel.setClipSalmonValue(static_cast<int>(clip_dist*res + 0.5));
    parentPanel.setClipMagentaValue(static_cast<int>(clip_dist*res + 0.5));
  }
  else
  {
    //parentPanel.m_link_x_tb->SetToolNormalBitmap(
    	//ClipPlanePanel::ID_LinkXChk,
    	//wxGetBitmapFromMemory(unlink));
    //parentPanel.m_x1_clip_sldr->SetPageSize(std::max(int(res) / 20, 1));
    //parentPanel.m_x2_clip_sldr->SetPageSize(std::max(int(res) / 20, 1));
    parentPanel.setClipSalmonValue(std::max(static_cast<int>(res)/20,1));
    parentPanel.setClipMagentaValue(std::max(static_cast<int>(res)/20,1));
  }
}

void ClipPlaneAgent::OnClipLinkYChanged(fluo::Event& event)
{
  bool bval = false;
  long res = 0;
  double clip_dist = 0.0;
  getValue("clip link y", bval);
  //parentPanel.m_link_y_tb->ToggleTool(ClipPlanePanel::ID_LinkYChk, bval);
  parentPanel.setYPlaneLockStatus(bval);
  getValue("res y", res);
  getValue("clip dist y", clip_dist);
  
  if (bval)
  {
    //parentPanel.m_link_y_tb->SetToolNormalBitmap(
    	//ClipPlanePanel::ID_LinkYChk,
    	//wxGetBitmapFromMemory(link));
    //parentPanel.m_y1_clip_sldr->SetPageSize(int(clip_dist*res + 0.5));
    //parentPanel.m_y2_clip_sldr->SetPageSize(int(clip_dist*res + 0.5));
    parentPanel.setClipGreenValue(static_cast<int>(clip_dist*res + 0.5));
    parentPanel.setClipYellowValue(static_cast<int>(clip_dist*res + 0.5));
  }
  else
  {
    //parentPanel.m_link_y_tb->SetToolNormalBitmap(
    	//ClipPlanePanel::ID_LinkYChk,
    	//wxGetBitmapFromMemory(unlink));
    //parentPanel.m_y1_clip_sldr->SetPageSize(std::max(int(res) / 20, 1));
    //parentPanel.m_y2_clip_sldr->SetPageSize(std::max(int(res) / 20, 1));
    parentPanel.setClipGreenValue(std::max(static_cast<int>(res)/20,1));
    parentPanel.setClipYellowValue(std::max(static_cast<int>(res)/20,1));
  }
}

void ClipPlaneAgent::OnClipLinkZChanged(fluo::Event& event)
{
  bool bval = false;
  long res = 0;
  double clip_dist = 0.0;
  getValue("clip link z", bval);
  //parentPanel.m_link_z_tb->ToggleTool(ClipPlanePanel::ID_LinkZChk, bval);
  parentPanel.setZPlaneLockStatus(bval);
  getValue("res z", res);
  getValue("clip dist z", clip_dist);

  if (bval)
  {
    //parentPanel.m_link_z_tb->SetToolNormalBitmap( // have to ask Yong what this is.
    	//ClipPlanePanel::ID_LinkZChk,
    	//wxGetBitmapFromMemory(link));
    //parentPanel.m_z1_clip_sldr->SetPageSize(int(clip_dist*res + 0.5));
    //parentPanel.m_z2_clip_sldr->SetPageSize(int(clip_dist*res + 0.5));

    parentPanel.setClipPurpleValue(static_cast<int>(clip_dist*res + 0.5));
    parentPanel.setClipTealValue(static_cast<int>(clip_dist*res + 0.5));
  }
  else
  {
  	//parentPanel.m_link_z_tb->SetToolNormalBitmap(
  		//ClipPlanePanel::ID_LinkZChk,
  		//wxGetBitmapFromMemory(unlink));
  	//parentPanel.m_z1_clip_sldr->SetPageSize(std::max(int(res) / 20, 1));
  	//parentPanel.m_z2_clip_sldr->SetPageSize(std::max(int(res) / 20, 1));

    parentPanel.setClipPurpleValue(std::max(static_cast<int>(res)/20,1));
    parentPanel.setClipTealValue(std::max(static_cast<int>(res)/20,1));
  }
}

void ClipPlaneAgent::OnClipRotXChanged(fluo::Event& event)
{
  double dval = 0.0;
  getValue("clip rot x", dval);
  parentPanel.setClipXRotValue(dval);
}

void ClipPlaneAgent::OnClipRotYChanged(fluo::Event& event)
{
  double dval = 0.0;
  getValue("clip rot y", dval);
  parentPanel.setClipYRotValue(dval);
}

void ClipPlaneAgent::OnClipRotZChanged(fluo::Event& event)
{
  double dval = 0.0;
  getValue("clip rot z", dval);
  parentPanel.setClipZRotValue(dval);
}
