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
#include <Coordinator.h>
#include <GlobalStates.h>
#include <VolumeData.h>
#include <VolumeRenderer.h>
#include <ModalDlg.h>
#include <Project.h>
#include <DistCalculator.h>

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
		auto info = GetRulerListInfo();
		dlg->UpdateRulerList(info);
	}

	if (request.HasValue(gstRulerListCur))
	{
		auto info = GetCurrentRulerInfo();
		dlg->UpdateRulerListCur(info);
	}

	if (update_all || request.HasValue(gstRulerListDisp))
	{
		auto info = GetRulerListDisplayInfo();
		dlg->UpdateRulerListDisp(info);
	}

	if (update_all || request.HasValue(gstRulerListSel))
	{
		ival = glbin_ruler_handler.GetRulerIndex();
		dlg->UpdateRulerListSel(ival);
	}

	if (request.HasValue(gstRulerGroupSel))
	{
		auto info = GetGroupSelectionInfo();
		dlg->UpdateGroupSel(info);
	}

	if (update_all || request.HasValue(gstRulerProfile))
	{
		auto info = GetProfileInfo();
		dlg->UpdateProfile(info);
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
	if (request.HasValue(gstCurrentRuler))
	{
		SetSelectedRulers();
	}
	if (request.HasValue(gstRulerLocator))
	{
		Locator();
	}
	if (request.HasValue(gstRulerProbe))
	{
		Probe();
	}
	if (request.HasValue(gstRulerLine))
	{
		RulerLine();
	}
	if (request.HasValue(gstRulerProtractor))
	{
		Protractor();
	}
	if (request.HasValue(gstRulerEllipse))
	{
		Ellipse();
	}
	if (request.HasValue(gstRulerPolyline))
	{
		RulerPolyline();
	}
	if (request.HasValue(gstRulerPencil))
	{
		Pencil();
	}
	if (request.HasValue(gstRulerGrow))
	{
		Grow();
	}
	if (request.HasValue(gstRulerMove))
	{
		RulerMove();
	}
	if (request.HasValue(gstRulerMovePoint))
	{
		RulerMovePoint();
	}
	if (request.HasValue(gstRulerMagnet))
	{
		Magnet();
	}
	if (request.HasValue(gstRulerMovePencil))
	{
		RulerMovePencil();
	}
	if (request.HasValue(gstRulerFlip))
	{
		RulerFlip();
	}
	if (request.HasValue(gstRulerAvg))
	{
		RulerAvg();
	}
	if (request.HasValue(gstRulerLock))
	{
		Lock();
	}
	if (request.HasValue(gstRulerRelax))
	{
		Relax();
	}
	if (request.HasValue(gstRulerDeleteSelection))
	{
		DeleteSelection();
	}
	if (request.HasValue(gstRulerDeleteAll))
	{
		DeleteAll();
	}
	if (request.HasValue(gstRulerDeletePoint))
	{
		DeletePoint();
	}
	if (request.HasValue(gstRulerPrune))
	{
		Prune();
	}
	if (request.HasValue(gstRulerProfile))
	{
		Profile();
	}
	if (request.HasValue(gstRulerDistance))
	{
		Distance();
	}
	if (request.HasValue(gstRulerProject))
	{
		Project();
	}
	if (request.HasValue(gstRulerExport))
	{
		Export();
	}
	if (request.HasValue(gstRulerNewGroup))
	{
		NewGroup();
	}
	if (request.HasValue(gstRulerGroupRulers))
	{
		GroupRulers();
	}
	if (request.HasValue(gstRulerToggleGroupDisp))
	{
		ToggleGroupDisp();
	}
	if (request.HasValue(gstRulerDeleteKey))
	{
		DeleteKey();
	}
	if (request.HasValue(gstRulerDeleteAllKeys))
	{
		DeleteAllKeys();
	}
	if (request.HasValue(gstRulerToggleDisp))
	{
		ToggleDisplay();
	}
}

MeasureDlg* MeasureDlgAgent::GetDialog() const
{
	return static_cast<MeasureDlg*>(GetWindow());
}

namespace
{
	std::string PointToString(const fluo::Point& p)
	{
		std::ostringstream oss;

		oss << std::fixed << std::setprecision(2)
			<< "("
			<< p.x() << ", "
			<< p.y() << ", "
			<< p.z() << ")";

		return oss.str();
	}
}

RulerListInfo MeasureDlgAgent::GetRulerListInfo()
{
	RulerListInfo result;

	auto view = glbin_current.render_view.lock();
	auto ruler_list = glbin_current.GetRulerList();

	if (!view || !ruler_list)
		return result;

	auto groups = ruler_list->get().Groups();
	std::vector<int> group_count(groups.size(), 0);

	size_t t = view->m_frame_num_type == 1 ?
		view->m_param_cur_num :
		view->m_tseq_cur_num;

	std::wstring unit;

	switch (view->m_sb_unit)
	{
	case 0:
		unit = L"nm";
		break;

	case 1:
	default:
		unit = L"\u03BCm";
		break;

	case 2:
		unit = L"mm";
		break;
	}

	for (int i = 0;
		i < static_cast<int>(ruler_list->get().size());
		++i)
	{
		auto ruler = ruler_list->get().GetRuler(i);

		if (!ruler)
			continue;

		ruler->SetWorkTime(t);

		if (ruler->GetTransient() &&
			ruler->GetTransTime() != t)
		{
			continue;
		}

		std::string points;

		int num_points = ruler->GetNumPoint();

		if (num_points > 0)
		{
			points += PointToString(
				ruler->GetPoint(0));
		}

		if (num_points > 1)
		{
			points += ", ";
			points += PointToString(
				ruler->GetPoint(num_points - 1));
		}

		unsigned int group = ruler->Group();

		int count = 0;

		auto iter =
			std::find(groups.begin(),
				groups.end(),
				group);

		if (iter != groups.end())
		{
			size_t index =
				std::distance(groups.begin(), iter);

			count = ++group_count[index];
		}

		double dval =
			ruler->GetProfileMaxValue() *
			ruler->GetScalarScale();

		std::ostringstream intensity_ss;
		intensity_ss << std::fixed
			<< std::setprecision(0)
			<< dval;

		std::string intensity =
			intensity_ss.str();

		std::string color;

		if (ruler->GetUseColor())
		{
			std::ostringstream color_ss;

			color_ss
				<< "RGB("
				<< int(std::round(
					ruler->GetColor().r() * 255))
				<< ", "
				<< int(std::round(
					ruler->GetColor().g() * 255))
				<< ", "
				<< int(std::round(
					ruler->GetColor().b() * 255))
				<< ")";

			color = color_ss.str();
		}
		else
		{
			color = "N/A";
		}

		std::string center =
			PointToString(
				ruler->GetCenter());

		std::string voxels =
			ruler->GetDelInfoValues(", ");

		RulerListItemInfo item;

		item.disp = ruler->GetDisp();
		item.id = ruler->Id();
		item.transient = ruler->GetTransient();

		item.unit = ws2s(unit);
		item.name = ws2s(ruler->GetName());

		item.group = group;
		item.group_count = count;

		item.intensity = intensity;
		item.color = color;

		item.branches = ruler->GetNumBranch();
		item.length = ruler->GetLength();
		item.angle = ruler->GetAngle();

		item.center = center;
		item.trans_time = ruler->GetTransTime();

		item.points = points;
		item.voxels = voxels;

		result.items.push_back(std::move(item));
	}

	return result;
}

RulerCurrentInfo MeasureDlgAgent::GetCurrentRulerInfo()
{
	RulerCurrentInfo info;

	auto ruler = glbin_current.GetRuler();
	if (!ruler)
		return info;

	std::string color_text;
	auto color = ruler->GetColor();

	if (ruler->GetUseColor())
	{
		std::ostringstream color_ss;

		color_ss
			<< "RGB("
			<< int(std::round(
				color.r() * 255))
			<< ", "
			<< int(std::round(
				color.g() * 255))
			<< ", "
			<< int(std::round(
				color.b() * 255))
			<< ")";

		color_text = color_ss.str();
		info.color_set = true;
	}
	else
	{
		color_text = "N/A";
		info.color_set = false;
	}

	info.index = glbin_ruler_handler.GetRulerIndex();
	info.mode = ruler->GetRulerMode();
	info.name = ws2s(ruler->GetName());
	info.center = PointToString(ruler->GetCenter());
	info.color = color;
	info.color_text = color_text;

	return info;
}

RulerListDisplayInfo MeasureDlgAgent::GetRulerListDisplayInfo()
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
	return info;
}

RulerGroupSelectionInfo MeasureDlgAgent::GetGroupSelectionInfo()
{
	RulerGroupSelectionInfo result;

	auto ruler_list =
		glbin_current.GetRulerList();

	if (!ruler_list)
		return result;

	size_t group =
		glbin_ruler_handler.GetGroup();

	for (size_t i = 0;
		i < ruler_list->get().size();
		++i)
	{
		auto ruler =
			ruler_list->get().GetRuler(i);

		if (!ruler)
			continue;

		if (ruler->Group() == group)
			result.selected_indices.push_back(
				static_cast<int>(i));
	}

	return result;
}

RulerProfileInfo MeasureDlgAgent::GetProfileInfo()
{
	RulerProfileInfo info;

	auto ruler_list = glbin_current.GetRulerList();
	for (int i = 0;
		i < static_cast<int>(ruler_list->get().size());
		++i)
	{
		auto ruler = ruler_list->get().GetRuler(i);
		if (!ruler)
			continue;

		double dval = ruler->GetProfileMaxValue();
		dval *= ruler->GetScalarScale();
		auto str = std::to_string(dval);
		info.profile.push_back(str);
	}

	return info;
}

void MeasureDlgAgent::ToggleDisplay()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;
	auto sel = dlg->GetCurrentSelection();
	glbin_ruler_handler.ToggleDisplay(sel);

	auto view = glbin_current.render_view.lock();
	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate({ gstRulerListDisp, gstRulerDisp }, target);
}

void MeasureDlgAgent::SetSelectedRulers()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;
	auto sel = dlg->GetCurrentSelection();
	glbin_ruler_handler.SetSelectedRulers(sel);
	int focus = dlg->GetFocusSelection();
	if (focus > -1)
		glbin_ruler_handler.SetEditingRuler(focus);

	auto view = glbin_current.render_view.lock();
	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate({ gstRulerTransient, gstRulerInterpolation, gstRulerDisp, gstColormap }, target);
}

void MeasureDlgAgent::SetCurrentRuler(const RulerCurrentInfo& info)
{
	auto ruler = glbin_current.GetRuler();
	int focus = glbin_ruler_handler.GetEditingRuler();
	auto editing_ruler = glbin_ruler_handler.GetRuler(focus);
	if (ruler != editing_ruler)
		return;
	auto view = glbin_current.render_view.lock();
	if (!ruler || !view)
		return;
	ruler->SetName(s2ws(info.name));
	if (ruler->GetRulerMode() == flrd::RulerMode::Locator)
	{
		ruler->SetWorkTime(view->m_tseq_cur_num);
		ruler->SetPoint(0, info.center);
	}
	if (info.color_set)
		ruler->SetColor(info.color);
	NotifyViewUpdate({ gstRulerListCur });
}

void MeasureDlgAgent::SetIntensityMethod(int ival)
{
	glbin_settings.m_point_volume_mode = ival;
}

void MeasureDlgAgent::SetTransient(bool bval)
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	auto sel = dlg->GetCurrentSelection();
	glbin_ruler_handler.SetTransient(bval, sel);

	auto view = glbin_current.render_view.lock();
	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate({ gstRulerList }, target);
}

void MeasureDlgAgent::SetUseTransfer(bool bval)
{
	glbin_settings.m_ruler_use_transf = bval;
}

void MeasureDlgAgent::SetDispPoint(bool bval)
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	auto sel = dlg->GetCurrentSelection();
	glbin_ruler_handler.SetDisplay(bval, sel, 0);

	auto view = glbin_current.render_view.lock();
	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate({ gstRulerList, gstRulerDisp }, target);
}

void MeasureDlgAgent::SetDispLine(bool bval)
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	auto sel = dlg->GetCurrentSelection();
	glbin_ruler_handler.SetDisplay(bval, sel, 1);

	auto view = glbin_current.render_view.lock();
	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate({ gstRulerList, gstRulerDisp }, target);
}

void MeasureDlgAgent::SetDispName(bool bval)
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	auto sel = dlg->GetCurrentSelection();
	glbin_ruler_handler.SetDisplay(bval, sel, 2);

	auto view = glbin_current.render_view.lock();
	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate({ gstRulerList, gstRulerDisp }, target);
}

void MeasureDlgAgent::SetDispAll(bool bval)
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	auto sel = dlg->GetCurrentSelection();
	glbin_ruler_handler.SetDisplay(bval, sel, 0);
	glbin_ruler_handler.SetDisplay(bval, sel, 1);
	glbin_ruler_handler.SetDisplay(bval, sel, 2);
	glbin_ruler_handler.SetDisplay(bval, sel);

	auto view = glbin_current.render_view.lock();
	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate({ gstRulerList, gstRulerDisp }, target);
}

void MeasureDlgAgent::SetRelaxData(int ival)
{
	glbin_settings.m_ruler_relax_type = ival;
}

void MeasureDlgAgent::SetRelaxValue(double dval)
{
	glbin_dist_calculator.SetF1(dval);
	glbin_settings.m_ruler_relax_f1 = dval;
}

void MeasureDlgAgent::SetGroup(unsigned int ival)
{
	glbin_ruler_handler.SetGroup(ival);
}

void MeasureDlgAgent::SetInterpolation(int ival)
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	auto sel = dlg->GetCurrentSelection();
	glbin_ruler_handler.SetInterp(ival, sel);

	auto view = glbin_current.render_view.lock();
	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate({ gstRulerList }, target);
}

void MeasureDlgAgent::SetAlignCenter(bool bval)
{
	glbin_aligner.SetAlignCenter(bval);

	auto view = glbin_current.render_view.lock();
	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate({ gstAlignCenter }, target);
}

void MeasureDlgAgent::AlignRuler(int ival)
{
	auto ruler = glbin_current.GetRuler();
	if (!ruler)
		return;

	glbin_aligner.SetRuler(ruler);
	glbin_aligner.SetAxisType(ival);
	glbin_aligner.AlignRuler();

	auto view = glbin_current.render_view.lock();
	std::set<Agent*> target;
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate({ gstNull }, target);
}

void MeasureDlgAgent::AlignPca(int ival)
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	auto sel = dlg->GetCurrentSelection();
	flrd::RulerList list;
	glbin_ruler_handler.GetRulerList(sel, list);
	glbin_aligner.SetRulerList(list);
	glbin_aligner.SetAxisType(ival);
	glbin_aligner.AlignPca(true);

	auto view = glbin_current.render_view.lock();
	std::set<Agent*> target;
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate({ gstNull }, target);
}

bool MeasureDlgAgent::GetSelectedRulerText(
	std::wstring& text) const
{
	auto ruler = glbin_current.GetRuler();
	if (!ruler)
		return false;

	auto cp = ruler->GetCenter();

	text = wxString::Format(
		"%.2f\t%.2f\t%.2f",
		cp.x(), cp.y(), cp.z()).ToStdWstring();

	return true;
}

void MeasureDlgAgent::Locator()
{
	glbin_states.ToggleRulerMode(flrd::RulerMode::Locator);
	NotifyViewUpdate({ gstFreehandToolState });
}

void MeasureDlgAgent::Probe()
{
	glbin_states.ToggleRulerMode(flrd::RulerMode::Probe);
	NotifyViewUpdate({ gstFreehandToolState });
}

void MeasureDlgAgent::RulerLine()
{
	glbin_states.ToggleRulerMode(flrd::RulerMode::Line);
	NotifyViewUpdate({ gstFreehandToolState });
}

void MeasureDlgAgent::Protractor()
{
	glbin_states.ToggleRulerMode(flrd::RulerMode::Protractor);
	NotifyViewUpdate({ gstFreehandToolState });
}

void MeasureDlgAgent::Ellipse()
{
	glbin_states.ToggleRulerMode(flrd::RulerMode::Ellipse);
	NotifyViewUpdate({ gstFreehandToolState });
}

void MeasureDlgAgent::RulerPolyline()
{
	glbin_states.ToggleRulerMode(flrd::RulerMode::Polyline);
	NotifyViewUpdate({ gstFreehandToolState });
}

void MeasureDlgAgent::Pencil()
{
	glbin_states.ToggleIntMode(InteractiveMode::Pencil);
	NotifyViewUpdate({ gstFreehandToolState });
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

	NotifyViewUpdate({ gstFreehandToolState, gstBrushSize1, gstBrushSize2, gstBrushIter });
}

void MeasureDlgAgent::RulerMove()
{
	glbin_states.ToggleIntMode(InteractiveMode::MoveRuler);
	NotifyViewUpdate({ gstFreehandToolState });
}

void MeasureDlgAgent::RulerMovePoint()
{
	glbin_states.ToggleIntMode(InteractiveMode::EditRulerPoint);
	NotifyViewUpdate({ gstFreehandToolState });
}

void MeasureDlgAgent::Magnet()
{
	glbin_states.ToggleMagnet(false);
	NotifyViewUpdate({ gstFreehandToolState });
}

void MeasureDlgAgent::RulerMovePencil()
{
	glbin_states.ToggleMagnet(true);
	NotifyViewUpdate({ gstFreehandToolState });
}

void MeasureDlgAgent::RulerFlip()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	auto sel = dlg->GetCurrentSelection();
	glbin_ruler_handler.Flip(sel);

	auto view = glbin_current.render_view.lock();
	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate({ gstRulerList }, target);
}

void MeasureDlgAgent::RulerAvg()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	auto sel = dlg->GetCurrentSelection();
	glbin_ruler_handler.AddAverage(sel);

	auto view = glbin_current.render_view.lock();
	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate({ gstRulerList }, target);
}

void MeasureDlgAgent::Lock()
{
	glbin_states.ToggleIntMode(InteractiveMode::RulerLockPoint);
	NotifyViewUpdate({ gstFreehandToolState });
}

void MeasureDlgAgent::Relax()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	auto sel = dlg->GetCurrentSelection();
	glbin_ruler_handler.Relax(sel);

	auto view = glbin_current.render_view.lock();
	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate({ gstRulerList }, target);
}

void MeasureDlgAgent::DeleteSelection()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	auto sel = dlg->GetCurrentSelection();
	glbin_ruler_handler.DeleteSelection(sel);

	auto view = glbin_current.render_view.lock();
	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate({ gstRulerList, gstRulerListSel }, target);
}

void MeasureDlgAgent::DeleteAll()
{
	glbin_ruler_handler.DeleteAll(false);

	auto view = glbin_current.render_view.lock();
	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate({ gstRulerList, gstRulerListSel }, target);
}

void MeasureDlgAgent::DeletePoint()
{
	glbin_states.ToggleIntMode(InteractiveMode::RulerDelPoint);
	NotifyViewUpdate({ gstFreehandToolState });
}

void MeasureDlgAgent::Prune()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	auto sel = dlg->GetCurrentSelection();
	glbin_ruler_handler.Prune(sel);

	auto view = glbin_current.render_view.lock();
	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate({ gstRulerList }, target);
}

void MeasureDlgAgent::Profile()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	auto sel = dlg->GetCurrentSelection();
	glbin_ruler_handler.Profile(sel);

	UpdateDataToUI({ gstRulerProfile });
}

void MeasureDlgAgent::Distance()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	ModalDlg fopendlg(
		dlg, "Save Analysis Data", "", "",
		"Text file (*.txt)|*.txt",
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	int rval = fopendlg.ShowModal();
	if (rval == wxID_OK)
	{
		wxString wxstr = fopendlg.GetPath();
		auto sel = dlg->GetCurrentSelection();
		glbin_ruler_handler.Distance(sel, wxstr.ToStdWstring());
	}
}

void MeasureDlgAgent::Project()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	ModalDlg fopendlg(
		dlg, "Save Analysis Data", "", "",
		"Text file (*.txt)|*.txt",
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	int rval = fopendlg.ShowModal();
	if (rval == wxID_OK)
	{
		wxString wxstr = fopendlg.GetPath();
		auto sel = dlg->GetCurrentSelection();
		glbin_ruler_handler.Project(sel, wxstr.ToStdWstring());
	}
}

void MeasureDlgAgent::Export()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	ModalDlg fopendlg(
		dlg, "Export rulers", "", "",
		"Text file (*.txt)|*.txt",
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	int rval = fopendlg.ShowModal();

	if (rval == wxID_OK)
	{
		wxString filename = fopendlg.GetPath();
		glbin_project.ExportRulerList(filename.ToStdWstring());
	}
}

void MeasureDlgAgent::NewGroup()
{
	glbin_ruler_handler.NewGroup();
}

void MeasureDlgAgent::GroupRulers()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	auto sel = dlg->GetCurrentSelection();
	glbin_ruler_handler.GroupRulers(sel);
	UpdateDataToUI({ gstRulerList });
}

void MeasureDlgAgent::ToggleGroupDisp()
{
	glbin_ruler_handler.ToggleGroupDisp();
	auto view = glbin_current.render_view.lock();
	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate({ gstRulerListDisp }, target);
}

void MeasureDlgAgent::DeleteKey()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	auto sel = dlg->GetCurrentSelection();
	glbin_ruler_handler.DeleteKey(sel);

	auto view = glbin_current.render_view.lock();
	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate({ gstRulerList }, target);
}

void MeasureDlgAgent::DeleteAllKeys()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	auto sel = dlg->GetCurrentSelection();
	glbin_ruler_handler.DeleteAllKeys(sel);

	auto view = glbin_current.render_view.lock();
	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate({ gstRulerList }, target);
}

