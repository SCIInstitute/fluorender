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

RenderViewPanelAgent::RenderViewPanelAgent(
	RenderViewPanel* panel) :
	Agent(panel)
{

}

bool RenderViewPanelAgent::Accept(
	const UpdateRequest& request) const
{
	return
		FOUND_VALUE(gstAnnotMemoText) ||
		FOUND_VALUE(gstAnnotMemoReadOnly);
}

void RenderViewPanelAgent::Update(
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

void RenderViewPanelAgent::UpdateUI(const UpdateRequest& request)
{
	auto panel = GetPanel();
	if (!panel)
		return;
	auto view = GetData();
	if (!view)
		return;

	if (FOUND_VALUE(gstNull))
		return;
	if (!view)
		return;

	int ival;
	bool bval;
	double dval;

	bool update_all = request.values.empty();
	bool update_pin_rot_ctr = FOUND_VALUE(gstPinRotCtr);

	//blend mode
	if (update_all || FOUND_VALUE(gstMixMethod))
	{
		ChannelMixMode mode = view->GetChannelMixMode();
		panel->UpdateMixMethod(mode);
	}

	//info
	if (update_all || FOUND_VALUE(gstDrawInfo))
	{
		bval = view->m_draw_info & 1;
		m_hud_tb->ToggleTool(ID_InfoChk, bval);
	}

	//cam center
	if (update_all || FOUND_VALUE(gstDrawCamCtr))
	{
		bval = view->m_draw_camctr;
		m_hud_tb->ToggleTool(ID_CamCtrChk, bval);
	}

	//legend
	if (update_all || FOUND_VALUE(gstDrawLegend))
	{
		bval = view->m_draw_legend;
		m_hud_tb->ToggleTool(ID_LegendChk, bval);
	}

	//colormap
	if (update_all || FOUND_VALUE(gstDrawColormap))
	{
		ival = view->m_colormap_disp;
		wxBitmapBundle colormap_bmp;
		switch (ival)
		{
		case 0:
		default:
			colormap_bmp = wxGetBitmap(colormap_off);
			break;
		case 1:
			colormap_bmp = wxGetBitmap(colormap);
			break;
		case 2:
			colormap_bmp = wxGetBitmap(colormap_text);
			break;
		}
		m_hud_tb->SetToolNormalBitmap(ID_Colormap, colormap_bmp);
	}

	//scale bar
	if (update_all || FOUND_VALUE(gstDrawScaleBar))
	{
		ival = view->m_scalebar_disp;
		switch (ival)
		{
		case 0:
		default:
			m_hud_tb->SetToolNormalBitmap(ID_ScaleBar,
				wxGetBitmap(scalebar));
			m_scale_text->Disable();
			m_scale_cmb->Disable();
			break;
		case 1:
			m_hud_tb->SetToolNormalBitmap(ID_ScaleBar,
				wxGetBitmap(scale_text_off));
			m_scale_text->Enable();
			m_scale_cmb->Disable();
			break;
		case 2:
			m_hud_tb->SetToolNormalBitmap(ID_ScaleBar,
				wxGetBitmap(scale_text));
			m_scale_text->Enable();
			m_scale_cmb->Enable();
			break;
		}
	}
	if (update_all || FOUND_VALUE(gstScaleBarUnit))
		m_scale_cmb->Select(view->m_sb_unit);

	//background
	if (update_all || FOUND_VALUE(gstBgColor))
	{
		fluo::Color c = view->GetBackgroundColor();
		wxColor wxc((unsigned char)(c.r() * 255 + 0.5),
			(unsigned char)(c.g() * 255 + 0.5),
			(unsigned char)(c.b() * 255 + 0.5));
		m_bg_color_picker->SetColour(wxc);
	}
	if (update_all || FOUND_VALUE(gstBgColorInv))
	{
		m_bg_inv_btn->ToggleTool(0, m_bg_color_inv);
		if (m_bg_color_inv)
			m_bg_inv_btn->SetToolNormalBitmap(0,
				wxGetBitmap(invert));
		else
			m_bg_inv_btn->SetToolNormalBitmap(0,
				wxGetBitmap(invert_off));
	}

	//angle of view
	if (update_all || FOUND_VALUE(gstAov))
	{
		ival = static_cast<int>(std::round(view->GetAov()));
		bval = view->GetPersp();
		m_aov_sldr->ChangeValue(bval ? ival : 10);
		m_aov_text->ChangeValue(bval ? std::to_string(ival) : "Ortho");
		if (bval)
			m_cam_op_tb->SetToolNormalBitmap(ID_OrthoPerspBtn,
				wxGetBitmap(persp));
		else
			m_cam_op_tb->SetToolNormalBitmap(ID_OrthoPerspBtn,
				wxGetBitmap(ortho));
	}

	//free fly
	if (update_all || FOUND_VALUE(gstCamMode))
	{
		ival = view->GetCamMode();
		switch (ival)
		{
		case 0:
			m_cam_op_tb->SetToolNormalBitmap(ID_CamModeBtn,
				wxGetBitmap(globe));
			break;
		case 1:
			m_cam_op_tb->SetToolNormalBitmap(ID_CamModeBtn,
				wxGetBitmap(flight));
			break;
		}
	}

	//stereo & holography
	if (update_all || FOUND_VALUE(gstHologramMode))
	{
		ival = glbin_settings.m_hologram_mode;
		m_full_screen_toolbar->ToggleTool(ID_VrChk, ival == 1);
		m_full_screen_toolbar->ToggleTool(ID_LookingGlassChk, ival == 2);
		if (ival != 2)
			view->ResetSize();
	}

	//depthe attenuation
	if (update_all || FOUND_VALUE(gstDepthAtten))
	{
		bval = view->GetFog();
		m_depth_atten_btn->ToggleTool(0, bval);
		if (bval)
			m_depth_atten_btn->SetToolNormalBitmap(0,
				wxGetBitmap(depth_atten));
		else
			m_depth_atten_btn->SetToolNormalBitmap(0,
				wxGetBitmap(no_depth_atten));
		m_depth_atten_factor_sldr->Enable(bval);
		m_depth_atten_factor_text->Enable(bval);
	}
	if (update_all || FOUND_VALUE(gstDaInt))
	{
		dval = view->GetFogIntensity();
		m_depth_atten_factor_sldr->ChangeValue(std::round(dval * 100));
		m_depth_atten_factor_text->ChangeValue(wxString::Format("%.2f", dval));
	}

	//center click
	if (update_all || FOUND_VALUE(gstFreehandToolState))
	{
		m_center_click_btn->ToggleTool(0, view->GetIntMode() == InteractiveMode::CenterClick);
	}

	//scale factor
	if (update_all || FOUND_VALUE(gstScaleFactor))
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
		m_scale_factor_sldr->ChangeValue(ival);
		m_scale_factor_text->ChangeValue(wxString::Format("%d", ival));
		m_scale_factor_text->Update();

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
	if (update_all || FOUND_VALUE(gstScaleMode))
	{
		switch (view->m_scale_mode)
		{
		case 0:
			m_scale_mode_btn->SetToolNormalBitmap(0,
				wxGetBitmap(zoom_view));
			m_scale_mode_btn->SetToolShortHelp(0,
				"View-based zoom ratio");
			m_scale_mode_btn->SetToolLongHelp(0,
				"View-based zoom ratio (View entire data set at 100%)");
			break;
		case 1:
			m_scale_mode_btn->SetToolNormalBitmap(0,
				wxGetBitmap(zoom_pixel));
			m_scale_mode_btn->SetToolShortHelp(0,
				"Pixel-based zoom ratio");
			m_scale_mode_btn->SetToolLongHelp(0,
				"Pixel-based zoom ratio (View 1 data pixel to 1 screen pixel at 100%)");
			break;
		case 2:
			m_scale_mode_btn->SetToolNormalBitmap(0,
				wxGetBitmap(zoom_data));
			m_scale_mode_btn->SetToolShortHelp(0,
				"Data-based zoom ratio");
			m_scale_mode_btn->SetToolLongHelp(0,
				"Data-based zoom ratio (View with consistent scale bar sizes)");
			break;
		}
	}
	//pin rotation center
	if (update_all || update_pin_rot_ctr)
	{
		bval = view->m_pin_rot_ctr;
		m_pin_btn->ToggleTool(0, bval);
		if (bval)
			m_pin_btn->SetToolNormalBitmap(0,
				wxGetBitmap(pin));
		else
			m_pin_btn->SetToolNormalBitmap(0,
				wxGetBitmap(pin_off));
	}

	//lock rot
	if (update_all || FOUND_VALUE(gstGearedEnable))
	{
		bval = view->GetRotLock();
		m_rot_btn->ToggleTool(ID_RotLockChk, bval);
		if (bval)
			m_rot_btn->SetToolNormalBitmap(ID_RotLockChk,
				wxGetBitmap(gear_45));
		else
			m_rot_btn->SetToolNormalBitmap(ID_RotLockChk,
				wxGetBitmap(gear_dark));
	}

	//slider type
	if (update_all || FOUND_VALUE(gstRotSliderMode))
	{
		m_slider_mode_btn->ToggleTool(0, m_rot_slider);
		if (m_rot_slider)
		{
			m_slider_mode_btn->SetToolNormalBitmap(0,
				wxGetBitmap(jog));
			if (m_x_rot_sldr->GetMode() != 1)
			{
				m_x_rot_sldr->SetMode(1);
				m_y_rot_sldr->SetMode(1);
				m_z_rot_sldr->SetMode(1);
			}
		}
		else
		{
			m_slider_mode_btn->SetToolNormalBitmap(0,
				wxGetBitmap(slider));
			if (m_x_rot_sldr->GetMode() != 0)
			{
				m_x_rot_sldr->SetMode(0);
				m_y_rot_sldr->SetMode(0);
				m_z_rot_sldr->SetMode(0);
			}
		}
	}

	//roatation
	if (update_all || FOUND_VALUE(gstCamRotation))
	{
		fluo::Vector rot = view->GetRotations();
		m_x_rot_sldr->ChangeValue(static_cast<int>(std::round(rot.x())));
		m_y_rot_sldr->ChangeValue(static_cast<int>(std::round(rot.y())));
		m_z_rot_sldr->ChangeValue(static_cast<int>(std::round(rot.z())));
		m_x_rot_text->ChangeValue(wxString::Format("%.1f", rot.x()));
		m_y_rot_text->ChangeValue(wxString::Format("%.1f", rot.y()));
		m_z_rot_text->ChangeValue(wxString::Format("%.1f", rot.z()));
		m_x_rot_text->Update();
		m_y_rot_text->Update();
		m_z_rot_text->Update();
		m_ortho_view_cmb->Select(view->GetOrientation());
	}
}

void RenderViewPanelAgent::UpdateData(const UpdateRequest& request)
{
	auto panel = GetPanel();
	if (!panel)
		return;
	auto ann = GetData();
	if (!ann)
		return;

	if (FOUND_VALUE(gstAnnotMemoText))
	{
		std::wstring str = panel->GetMemoText();
		ann->SetMemo(str);
	}
}
