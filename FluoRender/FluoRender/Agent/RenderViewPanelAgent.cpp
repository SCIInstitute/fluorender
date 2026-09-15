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

RenderViewPanelAgent::RenderViewPanelAgent(
	RenderViewPanel* panel,
	const std::shared_ptr<RenderView>& view) :
	Agent(panel),
	m_view(view)
{

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
	auto panel = GetPanel();
	if (!panel)
		return;
	auto view = GetView();
	if (!view)
		return;

}
