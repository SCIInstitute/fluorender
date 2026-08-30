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

#include <MeasureDlgAgent.h>
#include <MeasureDlg.h>
#include <Global.h>
#include <Names.h>

MeasureDlgAgent::MeasureDlgAgent(
	MeasureDlg* dlg) :
	Agent(dlg)
{

}

bool MeasureDlgAgent::Accept(
	const UpdateRequest& request) const
{
	return true;
}

void MeasureDlgAgent::Update(
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

void MeasureDlgAgent::UpdateUI(const UpdateRequest& request)
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	if (FOUND_VALUE(gstNull))
		return;

	bool update_all = request.values.empty();

	bool bval;
	int ival;

	if (update_all || FOUND_VALUE(gstFreehandToolState))
	{
		auto view = glbin_current.render_view.lock();
		InteractiveMode int_mode = view ? view->GetIntMode() : InteractiveMode::Disabled;
		flrd::RulerMode rul_mode = glbin_ruler_handler.GetRulerMode();
		//toolbar1
		m_toolbar1->ToggleTool(ID_RulerLocator, rul_mode == flrd::RulerMode::Locator);
		m_toolbar1->ToggleTool(ID_RulerProbe, rul_mode == flrd::RulerMode::Probe);
		m_toolbar1->ToggleTool(ID_RulerLine, rul_mode == flrd::RulerMode::Line);
		m_toolbar1->ToggleTool(ID_RulerAngle, rul_mode == flrd::RulerMode::Protractor);
		m_toolbar1->ToggleTool(ID_RulerEllipse, rul_mode == flrd::RulerMode::Ellipse);
		bval = rul_mode == flrd::RulerMode::Polyline &&
			(int_mode == InteractiveMode::Ruler ||
				int_mode == InteractiveMode::BrushRuler);
		m_toolbar1->ToggleTool(ID_RulerPolyline, bval);
		m_toolbar1->ToggleTool(ID_RulerPencil, int_mode == InteractiveMode::Pencil);
		m_toolbar1->ToggleTool(ID_RulerGrow, int_mode == InteractiveMode::GrowRuler);
		//toolbar2
		m_toolbar2->ToggleTool(ID_RulerMoveBtn, int_mode == InteractiveMode::MoveRuler);
		m_toolbar2->ToggleTool(ID_RulerMovePointBtn, int_mode == InteractiveMode::EditRulerPoint);
		bool bval2 = glbin_ruler_handler.GetRedistLength();
		m_toolbar2->ToggleTool(ID_MagnetBtn, int_mode == InteractiveMode::Magnet && !bval2);
		m_toolbar2->ToggleTool(ID_RulerMovePencilBtn, int_mode == InteractiveMode::Magnet && bval2);
		m_toolbar2->ToggleTool(ID_LockBtn, int_mode == InteractiveMode::RulerLockPoint);
		//toolbar3
		m_toolbar3->ToggleTool(ID_RulerDelBtn, int_mode == InteractiveMode::RulerDelPoint);
	}

	if (update_all || FOUND_VALUE(gstRulerList))
	{
		UpdateRulerList();
	}

	if (FOUND_VALUE(gstRulerListCur))
	{
		UpdateRulerListCur();
	}

	if (update_all || FOUND_VALUE(gstRulerListDisp))
	{
		wxColour c;
		for (int i = 0; i < m_ruler_list->GetItemCount(); ++i)
		{
			auto ruler = glbin_ruler_handler.GetRuler(i);
			if (!ruler)
				continue;
			bval = ruler->GetDisp();
			c = bval ? wxColour(255, 255, 255) : wxColour(200, 200, 200);
			m_ruler_list->SetItemBackgroundColour(i, c);
		}
	}

	if (update_all || FOUND_VALUE(gstRulerListSel))
	{
		ival = glbin_ruler_handler.GetRulerIndex();
		m_ruler_list->SelectItemSilently(ival);
	}

	if (FOUND_VALUE(gstRulerGroupSel))
	{
		UpdateGroupSel();
	}

	if (update_all || FOUND_VALUE(gstRulerProfile))
	{
		UpdateProfile();
	}

	if (update_all || FOUND_VALUE(gstRulerMethod))
	{
		ival = glbin_settings.m_point_volume_mode;
		m_view_plane_rd->SetValue(ival == 0);
		m_max_intensity_rd->SetValue(ival == 1);
		m_acc_intensity_rd->SetValue(ival == 2);
	}

	if (update_all || FOUND_VALUE(gstRulerTransient))
	{
		auto ruler = glbin_current.GetRuler();
		if (ruler)
		{
			bval = ruler->GetTransient();
			m_transient_chk->SetValue(bval);
		}
	}

	if (update_all || FOUND_VALUE(gstRulerUseTransf))
	{
		m_use_transfer_chk->SetValue(glbin_settings.m_ruler_use_transf);
	}

	if (update_all || FOUND_VALUE(gstRulerDisp))
	{
		auto ruler = glbin_current.GetRuler();
		if (ruler)
		{
			if (ruler->GetDisp())
			{
				bval = ruler->GetDisplay(0);
				m_disp_point_chk->SetValue(bval);
				bval = ruler->GetDisplay(1);
				m_disp_line_chk->SetValue(bval);
				bval = ruler->GetDisplay(2);
				m_disp_name_chk->SetValue(bval);
				m_disp_all_chk->SetValue(true);
			}
			else
			{
				m_disp_all_chk->SetValue(false);
				m_disp_point_chk->SetValue(false);
				m_disp_line_chk->SetValue(false);
				m_disp_name_chk->SetValue(false);
			}
		}
	}

	if (update_all || FOUND_VALUE(gstRulerRelaxType))
	{
		m_relax_data_cmb->Select(glbin_settings.m_ruler_relax_type);
	}

	if (update_all || FOUND_VALUE(gstRulerF1))
	{
		m_relax_value_spin->SetValue(glbin_settings.m_ruler_relax_f1);
	}

	if (update_all || FOUND_VALUE(gstRulerInterpolation))
	{
		auto ruler = glbin_current.GetRuler();
		if (ruler)
		{
			ival = ruler->GetInterp();
			m_interp_cmb->Select(ival);
		}
	}

	//align center
	if (update_all || FOUND_VALUE(gstAlignCenter))
	{
		bval = glbin_aligner.GetAlignCenter();
		m_align_center->SetValue(bval);
	}
}

void MeasureDlgAgent::UpdateData(const UpdateRequest& request)
{

}

MeasureDlg* MeasureDlgAgent::GetDialog() const
{
	return static_cast<MeasureDlg*>(GetWindow());
}

