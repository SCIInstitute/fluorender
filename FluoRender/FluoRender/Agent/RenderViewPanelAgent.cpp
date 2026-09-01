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
		panel->UpdateDrawInfo(bval);
	}

	//cam center
	if (update_all || FOUND_VALUE(gstDrawCamCtr))
	{
		bval = view->m_draw_camctr;
		panel->UpdateDrawCamCtr(bval);
	}

	//legend
	if (update_all || FOUND_VALUE(gstDrawLegend))
	{
		bval = view->m_draw_legend;
		panel->UpdateDrawLegend(bval);
	}

	//colormap
	if (update_all || FOUND_VALUE(gstDrawColormap))
	{
		ival = view->m_colormap_disp;
		panel->UpdateDrawColormap(ival);
	}

	//scale bar
	if (update_all || FOUND_VALUE(gstDrawScaleBar))
	{
		ival = view->m_scalebar_disp;
		panel->UpdateDrawScalebar(ival);
	}
	if (update_all || FOUND_VALUE(gstScaleBarUnit))
	{
		ival = view->m_sb_unit;
		panel->UpdateScaleBarUnit(ival);
	}

	//background
	if (update_all || FOUND_VALUE(gstBgColor))
	{
		fluo::Color c = view->GetBackgroundColor();
		panel->UpdateBgColor(c);
	}
	if (update_all || FOUND_VALUE(gstBgColorInv))
	{
		panel->UpdateBgColorInvert(m_bg_color_inv);
	}

	//angle of view
	if (update_all || FOUND_VALUE(gstAov))
	{
		ival = static_cast<int>(std::round(view->GetAov()));
		bval = view->GetPersp();
		panel->UpdateAov(ival, bval);
	}

	//free fly
	if (update_all || FOUND_VALUE(gstCamMode))
	{
		ival = view->GetCamMode();
		panel->UpdateCamMode(ival);
	}

	//stereo & holography
	if (update_all || FOUND_VALUE(gstHologramMode))
	{
		ival = glbin_settings.m_hologram_mode;
		panel->UpdateHologramMode(ival);
		if (ival != 2)
			view->ResetSize();
	}

	//center click
	if (update_all || FOUND_VALUE(gstFreehandToolState))
	{
		bval = view->GetIntMode() == InteractiveMode::CenterClick;
		panel->UpdateFreehandToolState(bval);
	}

	//depthe attenuation
	if (update_all || FOUND_VALUE(gstDepthAtten))
	{
		bval = view->GetFog();
		panel->UpdateDepthAtten(bval);
	}
	if (update_all || FOUND_VALUE(gstDaInt))
	{
		dval = view->GetFogIntensity();
		panel->UpdateDepthAttenFactor(dval);
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
	if (update_all || FOUND_VALUE(gstScaleMode))
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
	if (update_all || FOUND_VALUE(gstGearedEnable))
	{
		bval = view->GetRotLock();
		panel->UpdateGearedEnable(bval);
	}

	//slider type
	if (update_all || FOUND_VALUE(gstRotSliderMode))
	{
		panel->UpdateRotSliderMode(m_rot_slider);
	}

	//roatation
	if (update_all || FOUND_VALUE(gstCamRotation))
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
	auto ann = GetData();
	if (!ann)
		return;

}
