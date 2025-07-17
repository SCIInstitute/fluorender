/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2025 Scientific Computing and Imaging Institute,
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

#include <ViewDefault.h>
#include <Names.h>
#include <Global.h>
#include <RenderView.h>
#include <BaseTreeFile.h>
#include <TreeFileFactory.h>

ViewDefault::ViewDefault()
{
	m_vol_method = 1;
	m_bg_color = fluo::Color(0, 0, 0);
	m_draw_camctr = false;
	m_camctr_size = 2.0;
	m_draw_info = 250;
	m_draw_legend = false;
	m_colormap_disp = 0;
	m_scalebar_disp = 0;
	m_scalebar_len = 50;
	m_scalebar_text = L"50 \u03BCm";
	m_scalebar_num = L"50";
	m_scalebar_unit = 1;
	m_mouse_focus = false;
	m_persp = false;
	m_aov = 10.0;
	m_cam_mode = 0;
	m_center = fluo::Point(0);
	m_rot_lock = false;
	m_pin_rot_center = false;
	m_scale_mode = 0;
	m_scale_factor = 1.0;
	m_use_fog = false;
	m_fog_intensity = 0.0;

	m_rot = fluo::Vector(0);
	m_rot_slider = true;
}

ViewDefault::~ViewDefault()
{

}

void ViewDefault::Read()
{
	std::shared_ptr<BaseTreeFile> f =
		glbin_tree_file_factory.getTreeFile(gstConfigFile);
	if (!f)
		return;

	if (f->Exists("/view default"))
		f->SetPath("/view default");

	f->Read("vol method", &m_vol_method, 1);
	f->Read("bg color", &m_bg_color);
	f->Read("draw camctr", &m_draw_camctr, false);
	f->Read("camctr size", &m_camctr_size, 2.0);
	f->Read("draw info", &m_draw_info, 250);
	f->Read("draw legend", &m_draw_legend, false);
	f->Read("colormap disp", &m_colormap_disp, 0);
	f->Read("scalebar disp", &m_scalebar_disp, 0);
	f->Read("scalebar len", &m_scalebar_len, 50.0);
	f->Read("scalebar text", &m_scalebar_text, std::wstring(L"50 \u03BCm"));
	f->Read("scalebar num", &m_scalebar_num, std::wstring(L"50"));
	f->Read("scalebar unit", &m_scalebar_unit, 1);
	f->Read("mouse focus", &m_mouse_focus, false);
	f->Read("persp", &m_persp, false);
	f->Read("aov", &m_aov, 15.0);
	f->Read("cam mode", &m_cam_mode, 0);
	f->Read("center", &m_center);
	f->Read("rot lock", &m_rot_lock, false);
	f->Read("pin rot center", &m_pin_rot_center, false);
	f->Read("scale mode", &m_scale_mode, 0);
	f->Read("scale factor", &m_scale_factor, 1.0);
	f->Read("use fog", &m_use_fog, false);
	f->Read("fog intensity", &m_fog_intensity, 0.0);
	f->Read("rot", &m_rot);
	f->Read("rot slider", &m_rot_slider, true);
}

void ViewDefault::Save()
{
	std::shared_ptr<BaseTreeFile> f =
		glbin_tree_file_factory.getTreeFile(gstConfigFile);
	if (!f)
		return;

	f->SetPath("/view default");

	f->Write("vol method", m_vol_method);
	f->Write("bg color", m_bg_color);
	f->Write("draw camctr", m_draw_camctr);
	f->Write("camctr size", m_camctr_size);
	f->Write("draw info", m_draw_info);
	f->Write("draw legend", m_draw_legend);
	f->Write("colormap disp", m_colormap_disp);
	f->Write("scalebar disp", m_scalebar_disp);
	f->Write("scalebar len", m_scalebar_len);
	f->Write("scalebar text", m_scalebar_text);
	f->Write("scalebar num", m_scalebar_num);
	f->Write("scalebar unit", m_scalebar_unit);
	f->Write("mouse focus", m_mouse_focus);
	f->Write("persp", m_persp);
	f->Write("aov", m_aov);
	f->Write("cam mode", m_cam_mode);
	f->Write("center", m_center);
	f->Write("rot lock", m_rot_lock);
	f->Write("pin rot center", m_pin_rot_center);
	f->Write("scale mode", m_scale_mode);
	f->Write("scale factor", m_scale_factor);
	f->Write("use fog", m_use_fog);
	f->Write("fog intensity", m_fog_intensity);
	f->Write("rot", m_rot);
	f->Write("rot slider", m_rot_slider);
}

void ViewDefault::Set(RenderView* canvas)
{
	if (!canvas)
		return;

	m_vol_method = canvas->GetVolMethod();
	m_bg_color = canvas->GetBackgroundColor();
	m_draw_camctr = canvas->m_draw_camctr;
	m_camctr_size = canvas->m_camctr_size;
	m_draw_info = canvas->m_draw_info;
	m_draw_legend = canvas->m_draw_legend;
	m_colormap_disp = canvas->m_colormap_disp;
	m_mouse_focus = canvas->m_mouse_focus;
	m_scalebar_disp = canvas->m_scalebar_disp;
	m_scalebar_len = canvas->m_sb_length;
	m_scalebar_text = canvas->m_sb_text;
	m_scalebar_num = canvas->m_sb_num;
	m_scalebar_unit = canvas->m_sb_unit;
	m_persp = canvas->GetPersp();
	m_aov = canvas->GetAov();
	m_cam_mode = canvas->GetCamMode();
	m_center = canvas->GetCenters();
	m_rot_lock = canvas->GetRotLock();
	m_pin_rot_center = canvas->m_pin_rot_ctr;
	m_scale_mode = canvas->m_scale_mode;
	m_scale_factor = canvas->m_scale_factor;
	m_use_fog = canvas->GetFog();
	m_fog_intensity = canvas->GetFogIntensity();
}

void ViewDefault::Apply(RenderView* canvas)
{
	if (!canvas)
		return;

	canvas->SetVolMethod(m_vol_method);
	canvas->SetBackgroundColor(m_bg_color);
	canvas->m_draw_camctr = m_draw_camctr;
	canvas->m_camctr_size = m_camctr_size;
	canvas->m_draw_info = m_draw_info;
	canvas->m_draw_legend = m_draw_legend;
	canvas->m_colormap_disp = m_colormap_disp;
	canvas->m_scalebar_disp = m_scalebar_disp;
	canvas->m_sb_length = m_scalebar_len;
	canvas->m_sb_text = m_scalebar_text;
	canvas->m_sb_num = m_scalebar_num;
	canvas->m_sb_unit = m_scalebar_unit;
	canvas->m_mouse_focus = m_mouse_focus;
	canvas->SetPersp(m_persp);
	canvas->SetAov(m_aov);
	canvas->SetCamMode(m_cam_mode);
	canvas->SetCenters(m_center);
	canvas->SetRotLock(m_rot_lock);
	canvas->SetPinRotCenter(m_pin_rot_center, false);
	canvas->m_scale_mode = m_scale_mode;
	canvas->m_scale_factor = m_scale_factor;
	canvas->SetFog(m_use_fog);
	canvas->SetFogIntensity(m_fog_intensity);
}

