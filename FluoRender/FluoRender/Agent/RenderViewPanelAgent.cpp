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

#include <RenderViewPanelAgent.h>
#include <RenderViewPanel.h>
#include <Global.h>
#include <Names.h>
#include <RenderView.h>
#include <MainSettings.h>
#include <VolumeData.h>
#include <Coordinator.h>
#include <ModalDlg.h>
#include <GlobalStates.h>
#include <Project.h>

int RenderViewPanelAgent::m_max_id = 1;

RenderViewPanelAgent::RenderViewPanelAgent(
	RenderViewPanel* panel,
	const std::shared_ptr<RenderView>& view) :
	Agent(panel),
	m_view(view),
	m_default_saved(false)
{
	m_id = m_max_id++;
}

RenderViewPanel* RenderViewPanelAgent::GetPanel() const
{
	return static_cast<RenderViewPanel*>(GetWindow());
}

void RenderViewPanelAgent::UpdateUI(const UpdateRequest& request)
{
	auto panel = GetPanel();
	if (!panel)
		return;
	auto view = GetView();
	if (!view)
		return;

	int ival;
	bool bval;
	double dval;

	bool update_all = request.values.empty();
	bool update_pin_rot_ctr = request.HasValue(gstPinRotCtr);

	//name
	if (update_all || request.HasValue(gstRenderViewName))
	{
		panel->SetName(wxString::Format("Render View:%d", m_id));
	}

	//blend mode
	if (update_all || request.HasValue(gstMixMethod))
	{
		ChannelMixMode mode = view->GetChannelMixMode();
		panel->UpdateMixMethod(mode);
	}

	//info
	if (update_all || request.HasValue(gstDrawInfo))
	{
		bval = view->m_draw_info & 1;
		panel->UpdateDrawInfo(bval);
	}

	//cam center
	if (update_all || request.HasValue(gstDrawCamCtr))
	{
		bval = view->m_draw_camctr;
		panel->UpdateDrawCamCtr(bval);
	}

	//legend
	if (update_all || request.HasValue(gstDrawLegend))
	{
		bval = view->m_draw_legend;
		panel->UpdateDrawLegend(bval);
	}

	//colormap
	if (update_all || request.HasValue(gstDrawColormap))
	{
		ival = view->m_colormap_disp;
		panel->UpdateDrawColormap(ival);
	}

	//scale bar
	if (update_all || request.HasValue(gstDrawScaleBar))
	{
		ival = view->m_scalebar_disp;
		panel->UpdateDrawScalebar(ival);
	}
	if (update_all || request.HasValue(gstScaleBarLen))
	{
		dval = view->m_sb_length;
		panel->UpdateScaleBarValue(dval);
	}
	if (update_all || request.HasValue(gstScaleBarUnit))
	{
		ival = view->m_sb_unit;
		panel->UpdateScaleBarUnit(ival);
	}

	//background
	if (update_all || request.HasValue(gstBgColor))
	{
		fluo::Color c = view->GetBackgroundColor();
		panel->UpdateBgColor(c);
	}
	if (update_all || request.HasValue(gstBgColorInv))
	{
		panel->UpdateBgColorInvert(m_bg_color_inv);
	}

	//angle of view
	if (update_all || request.HasValue(gstAov))
	{
		ival = static_cast<int>(std::round(view->GetAov()));
		bval = view->GetPersp();
		panel->UpdateAov(ival, bval);
	}

	//free fly
	if (update_all || request.HasValue(gstCamMode))
	{
		ival = view->GetCamMode();
		panel->UpdateCamMode(ival);
	}

	//stereo & holography
	if (update_all || request.HasValue(gstHologramMode))
	{
		ival = glbin_settings.m_hologram_mode;
		panel->UpdateHologramMode(ival);
		if (ival != 2)
			view->ResetSize();
	}

	//center click
	if (update_all || request.HasValue(gstFreehandToolState))
	{
		bval = view->GetIntMode() == InteractiveMode::CenterClick;
		panel->UpdateFreehandToolState(bval);
	}

	//depthe attenuation
	if (update_all || request.HasValue(gstDepthAtten))
	{
		bval = view->GetFog();
		panel->UpdateDepthAtten(bval);
	}
	if (update_all || request.HasValue(gstDaInt))
	{
		dval = view->GetFogIntensity();
		panel->UpdateDepthAttenFactor(dval);
	}

	//scale factor
	if (update_all || request.HasValue(gstScaleFactor))
	{
		double scale = view->m_scale_factor;
		switch (view->m_scale_mode)
		{
		case 0:
			break;
		case 1:
			scale /= view->Get121ScaleFactor();
			break;
		case 2:
		{
			auto vd = view->m_cur_vol.lock();
			if (!vd && !view->GetVolPopListEmpty())
				vd = view->GetVolPopList(0);
			if (!vd)
				break;
			auto spc = vd->GetSpacing(vd->GetLevel());
			if (spc.x() > 0.0)
				scale /= view->Get121ScaleFactor() * spc.x();
		}
		break;
		}

		ival = std::round(scale * 100);
		panel->UpdateScaleFactor(ival);

		//check if need update pin rot center
		m_pin_by_scale = scale > glbin_settings.m_pin_threshold;
		if (m_pin_by_user == 0)
		{
			bool pin_by_canvas = view->m_pin_rot_ctr;
			view->SetPinRotCenter(m_pin_by_scale, false);
			update_pin_rot_ctr = m_pin_by_scale != pin_by_canvas;
		}
	}
	//scale mode
	if (update_all || request.HasValue(gstScaleMode))
	{
		ival = view->m_scale_mode;
		panel->UpdateScaleMode(ival);
	}
	//pin rotation center
	if (update_all || update_pin_rot_ctr)
	{
		bval = view->m_pin_rot_ctr;
		panel->UpdatePinRotCenter(bval);
	}

	//lock rot
	if (update_all || request.HasValue(gstGearedEnable))
	{
		bval = view->GetRotLock();
		panel->UpdateGearedEnable(bval);
	}

	//slider type
	if (update_all || request.HasValue(gstRotSliderMode))
	{
		panel->UpdateRotSliderMode(m_rot_slider);
	}

	//roatation
	if (update_all || request.HasValue(gstCamRotation))
	{
		fluo::Vector rot = view->GetRotations();
		ival = view->GetOrientation();
		panel->UpdateCamRotation(rot, ival);
	}
}

void RenderViewPanelAgent::UpdateData(const UpdateRequest& request)
{
	if (request.HasValue(gstMixMethod))
		SetChannelMixMode();
	if (request.HasValue(gstDrawInfo))
		SetInfo();
	if (request.HasValue(gstDrawCamCtr))
		SetDrawCamCtr();
	if (request.HasValue(gstDrawLegend))
		SetLegend();
	if (request.HasValue(gstDrawColormap))
		SetDrawColormap();
	if (request.HasValue(gstDrawScaleBar))
		SetDrawScalebar();
	if (request.HasValue(gstScaleBarLen))
		SetScaleText();
	if (request.HasValue(gstScaleBarUnit))
		SetScaleUnit();
	if (request.HasValue(gstBgColor))
		SetBgColor();
	if (request.HasValue(gstBgColorInv))
		SetBgColorInvert();
	if (request.HasValue(gstCapture))
		Capture();
}

int RenderViewPanelAgent::GetViewId()
{
	auto view = GetView();
	if (view)
		view->Id();
}

//reset counter
void RenderViewPanelAgent::ResetID()
{
	m_max_id = 1;
}

void RenderViewPanelAgent::SetChannelMixMode()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	auto mode = panel->GetChannelMixMethod();

	auto view = GetView();
	if (!view)
		return;

	fluo::ValueCollection vc;
	view->UpdateChannelMixMode(mode, vc);
	vc.insert(gstMixMethod);
	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate(vc, target);
}

void RenderViewPanelAgent::SetInfo()
{
	bool bval;
	auto panel = GetPanel();
	if (panel)
		bval = panel->GetInfo();

	auto view = GetView();
	if (!view)
		return;

	if (bval)
		view->m_draw_info |= 1;
	else
		view->m_draw_info &= ~1;

	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate({ gstDrawInfo }, target);
}

void RenderViewPanelAgent::SetDrawCamCtr()
{
	bool bval;
	auto panel = GetPanel();
	if (panel)
		bval = panel->GetCamCtr();

	auto view = GetView();
	if (!view)
		return;

	view->m_draw_camctr = bval;

	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate({ gstDrawCamCtr }, target);
}

void RenderViewPanelAgent::SetLegend()
{
	bool bval;
	auto panel = GetPanel();
	if (panel)
		bval = panel->GetLegend();

	auto view = GetView();
	if (!view)
		return;

	view->m_draw_legend = bval;

	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate({ gstDrawLegend }, target);
}

void RenderViewPanelAgent::SetDrawColormap()
{
	auto view = GetView();
	if (!view)
		return;

	int val = view->m_colormap_disp;
	val++;
	val = val % 3; // cycle through 0, 1, 2
	view->m_colormap_disp = val;

	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate({ gstDrawColormap }, target);
}

void RenderViewPanelAgent::SetDrawScalebar()
{
	auto view = GetView();
	if (!view)
		return;

	int val = view->m_scalebar_disp;
	val++;
	val = val % 3; // cycle through 0, 1, 2
	view->m_scalebar_disp = val;

	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate({ gstDrawScaleBar }, target);
}

void RenderViewPanelAgent::SetScaleText()
{
	auto panel = GetPanel();
	if (!panel)
		return;
	auto view = GetView();
	if (!view)
		return;

	double dval = panel->GetScalebarValue();
	std::wstring str, num_text, unit_text;
	num_text = std::to_wstring((int)dval);
	switch (view->m_sb_unit)
	{
	case 0:
		unit_text = L"nm";
		break;
	case 1:
	default:
		unit_text = L"\u03BCm";
		break;
	case 2:
		unit_text = L"mm";
		break;
	}
	str = num_text + L" " + unit_text;
	view->SetSBText(str);
	view->SetScaleBarLen(dval);
	view->m_sb_num = num_text;

	std::set<Agent*> target{};
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate({ gstNull }, target);
}

void RenderViewPanelAgent::SetScaleUnit()
{
	auto panel = GetPanel();
	if (!panel)
		return;
	auto view = GetView();
	if (!view)
		return;

	int ival = panel->GetScalebarUnit();
	view->m_sb_unit = ival;
	double dval = view->m_sb_length;
	std::wstring str, num_text, unit_text;
	num_text = std::to_wstring((int)dval);
	switch (ival)
	{
	case 0:
		unit_text = L"nm";
		break;
	case 1:
	default:
		unit_text = L"\u03BCm";
		break;
	case 2:
		unit_text = L"mm";
		break;
	}
	str = num_text + L" " + unit_text;
	view->SetSBText(str);
	view->SetScaleBarLen(dval);
	view->m_sb_num = num_text;

	std::set<Agent*> target{};
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate({ gstNull }, target);
}

void RenderViewPanelAgent::SetBgColor()
{
	auto panel = GetPanel();
	if (!panel)
		return;
	auto view = GetView();
	if (!view)
		return;

	auto color = panel->GetBgColor();
	view->SetBackgroundColor(color);

	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate({ gstBgColor }, target);
}

void RenderViewPanelAgent::SetBgColorInvert()
{
	auto view = GetView();
	if (!view)
		return;

	m_bg_color_inv = !m_bg_color_inv;
	fluo::Color c = view->GetBackgroundColor();
	c = fluo::Color(1.0, 1.0, 1.0) - c;
	view->SetBackgroundColor(c);

	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate({ gstBgColor, gstBgColorInv }, target);
}

void RenderViewPanelAgent::Capture()
{
	auto panel = GetPanel();
	if (!panel)
		return;
	auto view = GetView();
	if (!view)
		return;

	//reset enlargement
	view->SetEnlarge(false);
	view->SetEnlargeScale(1.0);

	CaptureOptions options;
	options.compress =
		glbin_settings.m_save_compress;
	options.saveAlpha =
		glbin_settings.m_save_alpha;
	options.saveFloat =
		glbin_settings.m_save_float;
	options.dpi =
		static_cast<int>(glbin_settings.m_dpi);
	options.enlarge =
		glbin_settings.m_dpi > 72;
	options.enlargeScale =
		glbin_settings.m_dpi / 72.0;
	options.embedFiles =
		glbin_settings.m_vrp_embed;

	CaptureHook hook(options);

	ModalDlg dlg(panel,
		"Save Captured Image", "", "output",
		"Tiff File (*.tif)|*.tif|"\
		"Tiff File (*.tiff)|*.tiff|"\
		"Png File (*.png)|*.png|"\
		"Jpeg File (*.jpg)|*.jpg|"\
		"Jpeg File (*.jpeg)|*.jpeg|"\
		"Jpeg2000 File (*.jp2)|*.jp2",
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	dlg.SetCustomizeHook(hook);

	int rval = dlg.ShowModal();
	if (rval == wxID_OK)
	{
		const CaptureOptions& selected_options =
			hook.GetOptions();

		view->m_cap_file = dlg.GetPath().ToStdWstring();
		view->m_capture = true;
		glbin_states.m_capture = true;
		std::set<Agent*> target{};
		target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
		NotifyViewUpdate({ gstNull }, target);

		if (glbin_settings.m_prj_save)
		{
			std::wstring new_folder = view->m_cap_file + L"_project";
			MkDirW(new_folder);
			std::filesystem::path p = new_folder;
			p /= dlg.GetFilename().ToStdString() + "_project.vrp";
			std::wstring prop_file = p.wstring();
			bool inc = std::filesystem::exists(prop_file) &&
				glbin_settings.m_prj_save_inc;
			glbin_project.Save(prop_file, inc);
		}
	}
}

void RenderViewPanelAgent::SetAov(double val, bool notify)
{
	if (val < 11)
	{
		view->SetPersp(false);
		if (view->GetAov() == 10)
			return;
		view->SetAov(10);
	}
	else if (val > 100)
	{
		view->SetPersp(true);
		if (view->GetAov() == 100)
			return;
		view->SetAov(100);
	}
	else
	{
		view->SetPersp(true);
		if (view->GetAov() == val)
			return;
		view->SetAov(val);
	}

	if (notify)
		FluoRefresh(2, { gstAov }, { GetViewId() });
	else
		FluoRefresh(2, { gstNull }, { GetViewId() });
}

void RenderViewPanelAgent::SetProjection()
{
	bool bval = view->GetPersp();
	if (bval)
	{
		SetAov(10, true);
	}
	else
	{
		SetAov(45, true);
	}
}

void RenderViewPanelAgent::SetCamMode()
{
	int ival = view->GetCamMode();
	ival = (ival + 1) % 2; // cycle through 0 and 1
	view->SetCamMode(ival);

	FluoRefresh(2, { gstCamMode }, { GetViewId() });
}

void RenderViewPanelAgent::SetStereography()
{
	int ival = glbin_settings.m_hologram_mode;
	glbin_settings.m_hologram_mode = ival == 1 ? 0 : 1;
	FluoRefresh(0, { gstHologramMode });
}

void RenderViewPanelAgent::SetHolography()
{
	int ival = glbin_settings.m_hologram_mode;
	glbin_settings.m_hologram_mode = ival == 2 ? 0 : 2;
	FluoRefresh(0, { gstHologramMode });
}

void RenderViewPanelAgent::SetFullScreen()
{
	m_enter_fscreen_trigger.Start(10);
}

void RenderViewPanelAgent::CloseFullScreen()
{
	if (m_canvas->GetParent() == m_full_frame)
		m_canvas->Close();
}

void RenderViewPanelAgent::SetDepthAttenEnable(bool val)
{
	view->SetFog(val);
	FluoRefresh(2, { gstDepthAtten }, { GetViewId() });
}

void RenderViewPanelAgent::SetDepthAtten(double val, bool notify)
{
	if (view->GetFogIntensity() == val)
		return;
	view->SetFogIntensity(val);
	if (notify)
		FluoRefresh(2, { gstDaInt }, { GetViewId() });
	else
		FluoRefresh(2, { gstNull }, { GetViewId() });
}

void RenderViewPanelAgent::SetCenter()
{
	view->SetCenter();
	FluoRefresh(2, { gstNull }, { GetViewId() });
}

void RenderViewPanelAgent::SetScale121()
{
	view->SetScale121();
	if (view->m_mouse_focus)
		m_canvas->SetFocus();
	FluoRefresh(2, { gstScaleFactor }, { GetViewId() });
}

void RenderViewPanelAgent::SetScaleFactor(double val)
{
	double factor = val;
	switch (view->m_scale_mode)
	{
	case 0:
		break;
	case 1:
		factor = val * view->Get121ScaleFactor();
		break;
	case 2:
	{
		auto vd = view->m_cur_vol.lock();
		if (!vd && !view->GetVolPopListEmpty())
			vd = view->GetVolPopList(0);
		if (vd)
		{
			auto spc = vd->GetSpacing(vd->GetLevel());
			if (spc.x() > 0.0)
				factor = val * view->Get121ScaleFactor() * spc.x();
		}
	}
	break;
	}
	if (view->m_scale_factor == factor)
		return;
	view->m_scale_factor = factor;
	FluoRefresh(2, { gstScaleFactor, gstPinRotCtr }, { GetViewId() });
}

void RenderViewPanelAgent::SetScaleMode(int val)
{
	view->m_scale_mode = val;
	FluoRefresh(2, { gstScaleMode, gstScaleFactor }, { GetViewId() });
}

void RenderViewPanelAgent::SetRotLock(bool val)
{
	view->SetRotLock(val);
	if (val)
	{
		fluo::Vector rot = view->GetRotations();
		rot = fluo::Vector(static_cast<int>(rot.x() / 45) * 45,
			static_cast<int>(rot.y() / 45) * 45,
			static_cast<int>(rot.z() / 45) * 45);
		SetRotations(rot, true);
	}
	FluoRefresh(2, { gstGearedEnable }, { GetViewId() });
}

void RenderViewPanelAgent::SetSliderType()
{
	m_rot_slider = !m_rot_slider;
	FluoRefresh(2, { gstRotSliderMode }, { GetViewId() });
}

void RenderViewPanelAgent::SetRotations(const fluo::Vector& val, bool notify)
{
	if (view->GetRotations() == val)
		return;
	view->SetRotations(val, false);
	if (notify)
		FluoRefresh(2, { gstCamRotation }, { GetViewId() });
	else
		FluoRefresh(2, { gstNull }, { GetViewId() });
}

void RenderViewPanelAgent::SetZeroRotations()
{
	fluo::Vector rot = view->GetRotations();
	if (rot.x() == 0.0 &&
		rot.y() == 0.0 &&
		rot.z() == 0.0)
	{
		//reset
		rot = view->ResetZeroRotations();
		view->SetRotations(rot, false);
	}
	else
	{
		view->SetZeroRotations();
		view->SetRotations(fluo::Vector(0), false);
	}
	FluoRefresh(2, { gstCamRotation }, { GetViewId() });
}

void RenderViewPanelAgent::SaveDefault(unsigned int mask)
{
	wxString str;
	wxColor cVal;

	//render modes
	if (mask & 0x1)
		glbin_view_def.m_channel_mix_mode = view->GetChannelMixMode();
	//background color
	if (mask & 0x2)
		glbin_view_def.m_bg_color = view->GetBackgroundColor();
	//camera center
	if (mask & 0x4)
		glbin_view_def.m_draw_camctr = view->m_draw_camctr;
	//camctr size
	if (mask & 0x8)
		glbin_view_def.m_camctr_size = view->m_camctr_size;
	//fps
	if (mask & 0x10)
		glbin_view_def.m_draw_info = view->m_draw_info;
	//selection
	if (mask & 0x20)
		glbin_view_def.m_draw_legend = view->m_draw_legend;
	//mouse focus
	if (mask & 0x40)
		glbin_view_def.m_mouse_focus = view->m_mouse_focus;
	//ortho/persp
	if (mask & 0x80)
	{
		glbin_view_def.m_persp = view->GetPersp();
		glbin_view_def.m_aov = view->GetAov();
		glbin_view_def.m_cam_mode = view->GetCamMode();
	}
	//rotations
	if (mask & 0x100)
	{
		glbin_view_def.m_rot = view->GetRotations();
		glbin_view_def.m_rot_lock = view->GetRotLock();
		glbin_view_def.m_rot_slider = m_rot_slider;
	}
	//depth atten
	if (mask & 0x200)
	{
		glbin_view_def.m_use_fog = view->GetFog();
		glbin_view_def.m_fog_intensity = view->GetFogIntensity();
	}
	//scale factor
	if (mask & 0x400)
	{
		glbin_view_def.m_pin_rot_center = view->m_pin_rot_ctr;
		glbin_view_def.m_scale_factor = view->m_scale_factor;
		glbin_view_def.m_scale_mode = view->m_scale_mode;
	}
	//camera center
	if (mask & 0x800)
		glbin_view_def.m_center = view->GetCenters();
	//colormap
	if (mask & 0x1000)
		glbin_view_def.m_colormap_disp = view->m_colormap_disp;
}

void RenderViewPanelAgent::LoadSettings()
{
	glbin_view_def.Apply(*view);
	m_rot_slider = glbin_view_def.m_rot_slider;

	FluoRefresh(2, {}, { GetViewId() });
}

