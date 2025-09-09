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

#include <GlobalStates.h>
#include <Global.h>
#include <MainSettings.h>
#include <DataManager.h>
#include <VolumeSelector.h>
#include <RenderView.h>
#include <Ruler.h>
#include <RulerHandler.h>

GlobalStates::GlobalStates()
{
}

GlobalStates::~GlobalStates()
{

}

bool GlobalStates::ClipDisplayChanged()
{
	bool bval = m_clip_display;
	m_clip_display =
		glbin_settings.m_clip_hold ||
		m_mouse_in_clip_plane_panel ||
		m_mouse_in_aov_slider;
	bval = bval != m_clip_display;
	return 	bval;
}

void GlobalStates::SetModal(bool bval)
{
	m_modal_shown = bval;
}

bool GlobalStates::ToggleBrushMode(flrd::SelectMode mode)
{
	bool result = false;
	auto view = glbin_current.render_view.lock();
	if (!view)
		return result;
	bool single_sel = mode == flrd::SelectMode::SingleSelect;
	bool grow_sel = mode == flrd::SelectMode::Grow;
	InteractiveMode int_mode = InteractiveMode::None;
	flrd::SelectMode sel_mode = glbin_vol_selector.GetSelectMode();
	flrd::RulerMode rul_mode = flrd::RulerMode::None;
	if (sel_mode == mode)
	{
		if (grow_sel &&
			view->GetIntMode() == InteractiveMode::GrowRuler)
		{
			//switch to grow brush if it was grow ruler
			//otherwise turn off
			int_mode = InteractiveMode::Grow;
		}
		else
		{
			int_mode = InteractiveMode::Viewport;
			sel_mode = flrd::SelectMode::None;
		}
		result = true;
	}
	else
	{
		if (single_sel)
			rul_mode = glbin_ruler_handler.GetRulerMode();//keep ruler mode unchanged if the brush is in single mode
		if (single_sel && rul_mode != flrd::RulerMode::None)
			int_mode = InteractiveMode::BrushRuler;
		else if (grow_sel)
			int_mode = InteractiveMode::Grow;
		else
			int_mode = InteractiveMode::BrushSelect;
		sel_mode = mode;
		result = false;
	}

	view->SetIntMode(int_mode);
	glbin_vol_selector.SetSelectMode(sel_mode);
	glbin_vol_selector.SetEstimateThreshold(false);
	glbin_ruler_handler.SetRulerMode(rul_mode);
	return result;
}

bool GlobalStates::ToggleRulerMode(flrd::RulerMode mode)
{
	bool result = false;
	auto view = glbin_current.render_view.lock();
	if (!view)
		return result;
	InteractiveMode int_mode = InteractiveMode::None;
	flrd::SelectMode sel_mode = glbin_vol_selector.GetSelectMode();
	flrd::RulerMode rul_mode = glbin_ruler_handler.GetRulerMode();
	bool poly_rul = rul_mode == flrd::RulerMode::Polyline;
	bool single_sel = sel_mode == flrd::SelectMode::SingleSelect;

	glbin_ruler_handler.FinishRuler();

	if (rul_mode == mode)
	{
		if (poly_rul &&
			view->GetIntMode() != InteractiveMode::Ruler)
		{
			if (single_sel)
			{
				int_mode = InteractiveMode::Viewport;
				rul_mode = flrd::RulerMode::None;
				sel_mode = flrd::SelectMode::None;
			}
			else
			{
				int_mode = InteractiveMode::Ruler;
				sel_mode = flrd::SelectMode::None;
				rul_mode = mode;
			}
		}
		else
		{
			int_mode = InteractiveMode::Viewport;
			rul_mode = flrd::RulerMode::None;
			sel_mode = flrd::SelectMode::None;
		}
		result = true;
	}
	else
	{
		if (single_sel)
			int_mode = InteractiveMode::BrushRuler;//keep select mode if it's single
		else
		{
			int_mode = InteractiveMode::Ruler;
			sel_mode = flrd::SelectMode::None;
		}
		rul_mode = mode;
		result = false;
	}

	view->SetIntMode(int_mode);
	glbin_vol_selector.SetSelectMode(sel_mode);
	glbin_ruler_handler.SetRulerMode(rul_mode);
	return result;
}

bool GlobalStates::ToggleIntMode(InteractiveMode mode)
{
	bool result = false;
	auto view = glbin_current.render_view.lock();
	if (!view)
		return result;
	InteractiveMode int_mode = view->GetIntMode();
	flrd::SelectMode sel_mode = flrd::SelectMode::None;
	flrd::RulerMode rul_mode = flrd::RulerMode::None;

	glbin_ruler_handler.FinishRuler();

	if (int_mode == mode)
	{
		int_mode = InteractiveMode::Viewport;
		rul_mode = flrd::RulerMode::None;
		result = true;
	}
	else
	{
		int_mode = mode;
		rul_mode = flrd::RulerMode::Polyline;
		if (int_mode == InteractiveMode::GrowRuler)
			sel_mode = flrd::SelectMode::Grow;
		result = false;
	}

	view->SetIntMode(int_mode);
	glbin_vol_selector.SetSelectMode(sel_mode);
	glbin_ruler_handler.SetRulerMode(rul_mode);
	return result;
}

bool GlobalStates::ToggleMagnet(bool redist)
{
	bool result = false;
	auto view = glbin_current.render_view.lock();
	if (!view)
		return result;
	InteractiveMode int_mode = view->GetIntMode();
	flrd::SelectMode sel_mode = flrd::SelectMode::None;
	flrd::RulerMode rul_mode = flrd::RulerMode::None;
	bool cur_redist = glbin_ruler_handler.GetRedistLength();

	glbin_ruler_handler.FinishRuler();

	if (int_mode == InteractiveMode::Magnet)
	{
		if (cur_redist == redist)
		{
			int_mode = InteractiveMode::Viewport;
			rul_mode = flrd::RulerMode::None;
			result = true;
		}
	}
	else
	{
		int_mode = InteractiveMode::Magnet;
		rul_mode = flrd::RulerMode::Polyline;
		result = false;
	}

	view->SetIntMode(int_mode);
	glbin_vol_selector.SetSelectMode(sel_mode);
	glbin_ruler_handler.SetRulerMode(rul_mode);
	glbin_ruler_handler.SetRedistLength(redist);
	return result;
}

bool GlobalStates::QueryShowBrush()
{
	flrd::SelectMode sel_mode = glbin_vol_selector.GetSelectMode();
	return sel_mode == flrd::SelectMode::SingleSelect ||
		sel_mode == flrd::SelectMode::Append ||
		sel_mode == flrd::SelectMode::Eraser ||
		sel_mode == flrd::SelectMode::Diffuse ||
		sel_mode == flrd::SelectMode::Segment ||
		sel_mode == flrd::SelectMode::Mesh;
}