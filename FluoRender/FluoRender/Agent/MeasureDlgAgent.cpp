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
#include <CurrentObjects.h>
#include <RenderView.h>
#include <RulerHandler.h>
#include <Ruler.h>
#include <RulerList.h>
#include <MainSettings.h>
#include <RulerAlign.h>

MeasureDlgAgent::MeasureDlgAgent(
	MeasureDlg* dlg) :
	Agent(dlg)
{

}

void MeasureDlgAgent::UpdateUI(const UpdateRequest& request)
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	bool update_all = request.values.empty();

	int ival;

	if (update_all || request.HasValue(gstFreehandToolState))
	{
		auto view = glbin_current.render_view.lock();
		InteractiveMode int_mode = view ? view->GetIntMode() : InteractiveMode::Disabled;
		flrd::RulerMode rul_mode = glbin_ruler_handler.GetRulerMode();
		bool redist_length = glbin_ruler_handler.GetRedistLength();
		dlg->UpdateFreehandToolState(int_mode, rul_mode, redist_length);
	}

	if (update_all || request.HasValue(gstRulerList))
	{
		dlg->UpdateRulerList();
	}

	if (request.HasValue(gstRulerListCur))
	{
		dlg->UpdateRulerListCur();
	}

	if (update_all || request.HasValue(gstRulerListDisp))
	{
		RulerListDisplayInfo info;
		auto list = glbin_current.GetRulerList();
		auto ruler_list = list->get();
		if (!ruler_list.IsEmpty())
		{
			for (size_t i = 0; i < ruler_list.size(); ++i)
			{
				auto ruler = ruler_list.GetRuler(i);
				info.visible.push_back(ruler && ruler->GetDisp());
			}
		}

		dlg->UpdateRulerListDisp(info);
	}

	if (update_all || request.HasValue(gstRulerListSel))
	{
		ival = glbin_ruler_handler.GetRulerIndex();
		dlg->UpdateRulerListSel(ival);
	}

	if (request.HasValue(gstRulerGroupSel))
	{
		dlg->UpdateGroupSel();
	}

	if (update_all || request.HasValue(gstRulerProfile))
	{
		dlg->UpdateProfile();
	}

	if (update_all || request.HasValue(gstRulerMethod))
	{
		ival = glbin_settings.m_point_volume_mode;
		dlg->UpdateRulerMethod(ival);
	}

	if (update_all || request.HasValue(gstRulerTransient))
	{
		auto ruler = glbin_current.GetRuler();
		if (ruler)
		{
			dlg->UpdateRulerTransient(ruler->GetTransient());
		}
	}

	if (update_all || request.HasValue(gstRulerUseTransf))
	{
		dlg->UpdateRulerUseTransf(glbin_settings.m_ruler_use_transf);
	}

	if (update_all || request.HasValue(gstRulerDisp))
	{
		auto ruler = glbin_current.GetRuler();
		bool bval0 = false;
		bool bval1 = false;
		bool bval2 = false;
		if (ruler)
		{
			if (ruler->GetDisp())
			{
				bval0 = ruler->GetDisplay(0);
				bval1 = ruler->GetDisplay(1);
				bval2 = ruler->GetDisplay(2);
			}
		}
		dlg->UpdateRulerDisp(bval0, bval1, bval2);
	}

	if (update_all || request.HasValue(gstRulerRelaxType))
	{
		dlg->UpdateRulerRelaxType(glbin_settings.m_ruler_relax_type);
	}

	if (update_all || request.HasValue(gstRulerF1))
	{
		dlg->UpdateRulerF1(glbin_settings.m_ruler_relax_f1);
	}

	if (update_all || request.HasValue(gstRulerInterpolation))
	{
		auto ruler = glbin_current.GetRuler();
		if (ruler)
		{
			ival = ruler->GetInterp();
			dlg->UpdateRulerInterpolation(ival);
		}
	}

	//align center
	if (update_all || request.HasValue(gstAlignCenter))
	{
		bool bval = glbin_aligner.GetAlignCenter();
		dlg->UpdateAlignCenter(bval);
	}
}

void MeasureDlgAgent::UpdateData(const UpdateRequest& request)
{

}

MeasureDlg* MeasureDlgAgent::GetDialog() const
{
	return static_cast<MeasureDlg*>(GetWindow());
}

void MeasureDlgAgent::ToggleDisplay()
{
	std::set<int> sel;
	if (!m_ruler_list->GetCurrSelection(sel))
		return;
	glbin_ruler_handler.ToggleDisplay(sel);
	FluoRefresh(2, { gstRulerListDisp, gstRulerDisp }, { glbin_current.GetViewId() });
}

void MeasureDlgAgent::SetCurrentRuler()
{
	auto ruler = glbin_current.GetRuler();
	int focus = glbin_ruler_handler.GetEditingRuler();
	auto editing_ruler = glbin_ruler_handler.GetRuler(focus);
	if (ruler != editing_ruler)
		return;
	auto view = glbin_current.render_view.lock();
	if (!ruler || !view)
		return;
	ruler->SetName(m_ruler_list->m_name.ToStdWstring());
	if (ruler->GetRulerMode() == flrd::RulerMode::Locator)
	{
		ruler->SetWorkTime(view->m_tseq_cur_num);
		ruler->SetPoint(0, m_ruler_list->m_center);
	}
	if (m_ruler_list->m_color_set)
		ruler->SetColor(m_ruler_list->m_color);
	FluoRefresh(0, { gstRulerListCur },
		{ glbin_current.GetViewId() });
}

void MeasureDlgAgent::UpdateProfile()
{
	wxString str;
	for (int i = 0; i < m_ruler_list->GetItemCount(); ++i)
	{
		auto ruler = glbin_ruler_handler.GetRuler(i);
		if (!ruler)
			continue;

		double dval = ruler->GetProfileMaxValue();
		dval *= ruler->GetScalarScale();
		str = wxString::Format("%.0f", dval);
		m_ruler_list->SetText(i, IntCol, str);
	}
}

void MeasureDlgAgent::Locator()
{
	glbin_states.ToggleRulerMode(flrd::RulerMode::Locator);
	FluoRefresh(0, { gstFreehandToolState }, { -1 });
}

void MeasureDlgAgent::Probe()
{
	glbin_states.ToggleRulerMode(flrd::RulerMode::Probe);
	FluoRefresh(0, { gstFreehandToolState }, { -1 });
}

void MeasureDlgAgent::RulerLine()
{
	glbin_states.ToggleRulerMode(flrd::RulerMode::Line);
	FluoRefresh(0, { gstFreehandToolState }, { -1 });
}

void MeasureDlgAgent::Protractor()
{
	glbin_states.ToggleRulerMode(flrd::RulerMode::Protractor);
	FluoRefresh(0, { gstFreehandToolState }, { -1 });
}

void MeasureDlgAgent::Ellipse()
{
	glbin_states.ToggleRulerMode(flrd::RulerMode::Ellipse);
	FluoRefresh(0, { gstFreehandToolState }, { -1 });
}

void MeasureDlgAgent::RulerPolyline()
{
	glbin_states.ToggleRulerMode(flrd::RulerMode::Polyline);
	FluoRefresh(0, { gstFreehandToolState }, { -1 });
}

void MeasureDlgAgent::Pencil()
{
	glbin_states.ToggleIntMode(InteractiveMode::Pencil);
	FluoRefresh(0, { gstFreehandToolState }, { -1 });
}

void MeasureDlgAgent::Grow()
{
	bool bval = glbin_states.ToggleIntMode(InteractiveMode::GrowRuler);
	if (!bval)
	{
		//reset label volume
		auto vd = glbin_current.vol_data.lock();
		if (vd)
		{
			vd->GetVolumeRenderer().clear_tex_mask();
			vd->GetVolumeRenderer().clear_tex_label();
			vd->AddEmptyMask(0, true);
			vd->AddEmptyLabel(0, true);
		}
	}

	FluoRefresh(0, { gstFreehandToolState, gstBrushSize1, gstBrushSize2, gstBrushIter }, { -1 });
}

void MeasureDlgAgent::RulerMove()
{
	glbin_states.ToggleIntMode(InteractiveMode::MoveRuler);
	FluoRefresh(0, { gstFreehandToolState }, { -1 });
}

void MeasureDlgAgent::RulerMovePoint()
{
	glbin_states.ToggleIntMode(InteractiveMode::EditRulerPoint);
	FluoRefresh(0, { gstFreehandToolState }, { -1 });
}

void MeasureDlgAgent::Magnet()
{
	glbin_states.ToggleMagnet(false);
	FluoRefresh(0, { gstFreehandToolState }, { -1 });
}

void MeasureDlgAgent::RulerMovePencil()
{
	glbin_states.ToggleMagnet(true);
	FluoRefresh(0, { gstFreehandToolState }, { -1 });
}

void MeasureDlgAgent::RulerFlip()
{
	std::set<int> sel;
	m_ruler_list->GetCurrSelection(sel);
	glbin_ruler_handler.Flip(sel);

	FluoRefresh(2, { gstRulerList },
		{ glbin_current.GetViewId() });
}

void MeasureDlgAgent::RulerAvg()
{
	std::set<int> sel;
	m_ruler_list->GetCurrSelection(sel);
	glbin_ruler_handler.AddAverage(sel);

	FluoRefresh(2, { gstRulerList },
		{ glbin_current.GetViewId() });
}

void MeasureDlgAgent::Lock()
{
	glbin_states.ToggleIntMode(InteractiveMode::RulerLockPoint);
	FluoRefresh(0, { gstFreehandToolState }, { -1 });
}

void MeasureDlgAgent::Relax()
{
	std::set<int> sel;
	m_ruler_list->GetCurrSelection(sel);
	glbin_ruler_handler.Relax(sel);

	FluoRefresh(2, { gstRulerList },
		{ glbin_current.GetViewId() });
}

void MeasureDlgAgent::DeleteSelection()
{
	std::set<int> sel;
	m_ruler_list->GetCurrSelection(sel);
	glbin_ruler_handler.DeleteSelection(sel);
	FluoRefresh(2, { gstRulerList, gstRulerListSel },
		{ glbin_current.GetViewId() });
}

void MeasureDlgAgent::DeleteAll()
{
	glbin_ruler_handler.DeleteAll(false);
	FluoRefresh(2, { gstRulerList, gstRulerListSel },
		{ glbin_current.GetViewId() });
}

void MeasureDlgAgent::DeletePoint()
{
	glbin_states.ToggleIntMode(InteractiveMode::RulerDelPoint);
	FluoRefresh(0, { gstFreehandToolState }, { -1 });
}

void MeasureDlgAgent::Prune()
{
	std::set<int> sel;
	m_ruler_list->GetCurrSelection(sel);
	glbin_ruler_handler.Prune(sel);

	FluoRefresh(2, { gstRulerList },
		{ glbin_current.GetViewId() });
}

void MeasureDlgAgent::Profile()
{
	std::set<int> sel;
	m_ruler_list->GetCurrSelection(sel);
	glbin_ruler_handler.Profile(sel);

	FluoUpdate({ gstRulerProfile });
}

void MeasureDlgAgent::Distance()
{
	ModalDlg fopendlg(
		this, "Save Analysis Data", "", "",
		"Text file (*.txt)|*.txt",
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	int rval = fopendlg.ShowModal();
	if (rval == wxID_OK)
	{
		wxString wxstr = fopendlg.GetPath();
		std::set<int> sel;
		m_ruler_list->GetCurrSelection(sel);
		glbin_ruler_handler.Distance(sel, wxstr.ToStdWstring());
	}
}

void MeasureDlgAgent::Project()
{
	ModalDlg fopendlg(
		this, "Save Analysis Data", "", "",
		"Text file (*.txt)|*.txt",
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	int rval = fopendlg.ShowModal();
	if (rval == wxID_OK)
	{
		wxString wxstr = fopendlg.GetPath();
		std::set<int> sel;
		m_ruler_list->GetCurrSelection(sel);
		glbin_ruler_handler.Project(sel, wxstr.ToStdWstring());
	}
}

void MeasureDlgAgent::Export()
{
	ModalDlg fopendlg(
		m_frame, "Export rulers", "", "",
		"Text file (*.txt)|*.txt",
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	int rval = fopendlg.ShowModal();

	if (rval == wxID_OK)
	{
		wxString filename = fopendlg.GetPath();
		glbin_project.ExportRulerList(filename.ToStdWstring());
	}
}

