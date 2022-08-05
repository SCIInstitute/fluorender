/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2022 Scientific Computing and Imaging Institute,
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

#include <MeasureAgent.hpp>
//#include <MeasureDlg.h>
#include <DistCalculator.h>
#include <RulerHandler.h>
#include <RulerAlign.h>
#include <Ruler.h>
#include <CompAnalyzer.h>
#include <FLIVR/VertexArray.h>
#include <RulerRenderer.h>
#include <FLIVR/TextureRenderer.h>
#include <FLIVR/VolumeRenderer.h>
#include <sstream>
#include <fstream>
#include <iostream>
#include <compatibility.h>

using namespace fluo;

MeasureAgent::MeasureAgent(MeasureDlg &dlg) :
	InterfaceAgent(),
	dlg_(dlg)
{
	m_calculator = new flrd::DistCalculator();
}

MeasureAgent::~MeasureAgent()
{
	delete m_calculator;
}

void MeasureAgent::setupInputs()
{

}

void MeasureAgent::setObject(Renderview* obj)
{
	InterfaceAgent::setObject(obj);
}

Renderview* MeasureAgent::getObject()
{
	return dynamic_cast<Renderview*>(InterfaceAgent::getObject());
}

void MeasureAgent::UpdateFui(const ValueCollection &names)
{
	bool update_all = names.empty();

	if (update_all || FOUND_VALUE(gstNonObjectValues))
	{
		UpdateRulers();
	}

	if (update_all || FOUND_VALUE(gstInterMode))
	{
		//dlg_.m_toolbar1->ToggleTool(MeasureDlg::ID_LocatorBtn, false);
		//dlg_.m_toolbar1->ToggleTool(MeasureDlg::ID_ProbeBtn, false);
		//dlg_.m_toolbar1->ToggleTool(MeasureDlg::ID_ProtractorBtn, false);
		//dlg_.m_toolbar1->ToggleTool(MeasureDlg::ID_RulerBtn, false);
		//dlg_.m_toolbar1->ToggleTool(MeasureDlg::ID_RulerMPBtn, false);
		//dlg_.m_toolbar1->ToggleTool(MeasureDlg::ID_EllipseBtn, false);
		//dlg_.m_toolbar1->ToggleTool(MeasureDlg::ID_GrowBtn, false);
		//dlg_.m_toolbar1->ToggleTool(MeasureDlg::ID_PencilBtn, false);
		//dlg_.m_toolbar2->ToggleTool(MeasureDlg::ID_RulerEditBtn, false);
		//dlg_.m_toolbar2->ToggleTool(MeasureDlg::ID_RulerDelBtn, false);
		//dlg_.m_toolbar2->ToggleTool(MeasureDlg::ID_RulerMoveBtn, false);
		//dlg_.m_toolbar2->ToggleTool(MeasureDlg::ID_LockBtn, false);

		long int_mode;
		getValue(gstInterMode, int_mode);
/*		if (int_mode == 5 || int_mode == 7)
		{
			Renderview* view = getObject();
			if (!view) return;
			flrd::RulerHandler* rhandler = view->GetRulerHandler();
			if (!rhandler) return;

			int ruler_type = rhandler->GetType();
			if (ruler_type == 0)
				dlg_.m_toolbar1->ToggleTool(MeasureDlg::ID_RulerBtn, true);
			else if (ruler_type == 1)
				dlg_.m_toolbar1->ToggleTool(MeasureDlg::ID_RulerMPBtn, true);
			else if (ruler_type == 2)
				dlg_.m_toolbar1->ToggleTool(MeasureDlg::ID_LocatorBtn, true);
			else if (ruler_type == 3)
				dlg_.m_toolbar1->ToggleTool(MeasureDlg::ID_ProbeBtn, true);
			else if (ruler_type == 4)
				dlg_.m_toolbar1->ToggleTool(MeasureDlg::ID_ProtractorBtn, true);
			else if (ruler_type == 5)
				dlg_.m_toolbar1->ToggleTool(MeasureDlg::ID_EllipseBtn, true);
		}
		else if (int_mode == 6)
			dlg_.m_toolbar2->ToggleTool(MeasureDlg::ID_RulerEditBtn, true);
		else if (int_mode == 9)
			dlg_.m_toolbar2->ToggleTool(MeasureDlg::ID_RulerMoveBtn, true);
		else if (int_mode == 11)
			dlg_.m_toolbar2->ToggleTool(MeasureDlg::ID_LockBtn, true);
		else if (int_mode == 12)
			dlg_.m_toolbar1->ToggleTool(MeasureDlg::ID_GrowBtn, true);
		else if (int_mode == 13)
			dlg_.m_toolbar1->ToggleTool(MeasureDlg::ID_PencilBtn, true);
		else if (int_mode == 14)
			dlg_.m_toolbar2->ToggleTool(MeasureDlg::ID_RulerDelBtn, true);
*/	}

	if (update_all || FOUND_VALUE(gstPointVolumeMode))
	{
		long point_volume_mode;
		getValue(gstPointVolumeMode, point_volume_mode);
/*		switch (point_volume_mode)
		{
		case 0:
			dlg_.m_view_plane_rd->SetValue(true);
			dlg_.m_max_intensity_rd->SetValue(false);
			dlg_.m_acc_intensity_rd->SetValue(false);
			break;
		case 1:
			dlg_.m_view_plane_rd->SetValue(false);
			dlg_.m_max_intensity_rd->SetValue(true);
			dlg_.m_acc_intensity_rd->SetValue(false);
			break;
		case 2:
			dlg_.m_view_plane_rd->SetValue(false);
			dlg_.m_max_intensity_rd->SetValue(false);
			dlg_.m_acc_intensity_rd->SetValue(true);
			break;
		}
*/	}

	if (update_all || FOUND_VALUE(gstRulerUseTransf))
	{
		bool bval;
		getValue(gstRulerUseTransf, bval);
		//dlg_.m_use_transfer_chk->SetValue(bval);
	}
	if (update_all || FOUND_VALUE(gstRulerTransient))
	{
		bool bval;
		getValue(gstRulerTransient, bval);
		//dlg_.m_transient_chk->SetValue(bval);
	}
	//if (m_frame && m_frame->GetSettingDlg())
	//{
	//	//ruler exports df/f
	//	bool bval = m_frame->GetSettingDlg()->GetRulerDF_F();
	//	m_df_f_chk->SetValue(bval);
	//	m_rulerlist->m_ruler_df_f = bval;
	//	//relax
	//	m_relax_value_spin->SetValue(
	//		m_frame->GetSettingDlg()->GetRulerRelaxF1());
	//	m_auto_relax_btn->SetValue(
	//		m_frame->GetSettingDlg()->GetRulerAutoRelax());
	//	m_view->setValue(gstRulerRelax,
	//		m_frame->GetSettingDlg()->GetRulerAutoRelax());
	//	m_relax_data_cmb->Select(
	//		m_frame->GetSettingDlg()->GetRulerRelaxType());
	//}
}

void MeasureAgent::UpdateRulers()
{
	Renderview* view = getObject();
	if (!view) return;
	flrd::RulerList* ruler_list = view->GetRulerList();
	if (!ruler_list) return;

	//dlg_.m_rulerlist->m_name_text->Hide();
	//dlg_.m_rulerlist->m_center_text->Hide();
	//dlg_.m_rulerlist->m_color_picker->Hide();

	std::vector<int> sel;
	//dlg_.m_rulerlist->GetCurrSelection(sel);

	std::vector<unsigned int> groups;
	int group_num = ruler_list->GetGroupNum(groups);
	std::vector<int> group_count(group_num, 0);

	//dlg_.m_rulerlist->DeleteAllItems();

	std::string points;
	fluo::Point p;
	int num_points;
	long cur_frame, sb_unit;
	view->getValue(gstCurrentFrame, cur_frame);
	view->getValue(gstScaleBarUnit, sb_unit);
	for (int i = 0; i < (int)ruler_list->size(); i++)
	{
		flrd::Ruler* ruler = (*ruler_list)[i];
		if (!ruler) continue;
		if (ruler->GetTimeDep() &&
			ruler->GetTime() != cur_frame)
			continue;

		std::wstring unit;
		switch (sb_unit)
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

		points = "";
		num_points = ruler->GetNumPoint();
		if (num_points > 0)
		{
			p = ruler->GetPoint(0)->GetPoint();
			points += "(" + std::to_string(p.x()) + ", " +  std::to_string(p.y()) + ", " + std::to_string(p.z()) + ")";
		}
		if (num_points > 1)
		{
			p = ruler->GetPoint(num_points - 1)->GetPoint();
			points += ", ";
			points += "(" + std::to_string(p.x()) + ", " + std::to_string(p.y()) + ", " + std::to_string(p.z()) + ")";
		}
		unsigned int group = ruler->Group();
		int count = 0;
		auto iter = std::find(groups.begin(), groups.end(), group);
		if (iter != groups.end())
		{
			int index = std::distance(groups.begin(), iter);
			count = ++group_count[index];
		}
		std::string color;
		if (ruler->GetUseColor())
			color = "RGB(" +
				std::to_string(int(ruler->GetColor().r() * 255)) + ", " +
				std::to_string(int(ruler->GetColor().g() * 255)) + ", " +
				std::to_string(int(ruler->GetColor().b() * 255)) + ")";
		else
			color = "N/A";
		std::string center;
		fluo::Point cp = ruler->GetCenter();
		center = "(" + std::to_string(cp.x()) + ", " + std::to_string(cp.y()) + ", " + std::to_string(cp.z()) + ")";
		std::string str = ruler->GetDelInfoValues(", ");
		//dlg_.m_rulerlist->Append(ruler->GetDisp(), ruler->Id(),
		//	ruler->GetName(), group, count,
		//	color, ruler->GetNumBranch(), ruler->GetLength(), unit,
		//	ruler->GetAngle(), center, ruler->GetTimeDep(),
		//	ruler->GetTime(), str, points);
	}

	//dlg_.m_rulerlist->AdjustSize();

	for (size_t i = 0; i < sel.size(); ++i)
	{
		int index = sel[i];
		if (0 > index || ruler_list->size() <= index)
			continue;
		//dlg_.m_rulerlist->SetItemState(index, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
	}

	flvr::TextureRenderer::vertex_array_manager_.set_dirty(flvr::VA_Rulers);
}

void MeasureAgent::Relax()
{
	std::vector<int> sel;
/*	if (dlg_.m_rulerlist->GetCurrSelection(sel))
	{
		for (size_t i = 0; i < sel.size(); ++i)
			Relax(sel[i]);
	}
*/}

void MeasureAgent::Relax(int idx)
{
	Renderview* view = getObject();
	if (!view) return;
	flrd::RulerList* ruler_list = view->GetRulerList();
	if (!ruler_list) return;
	if (idx < 0 || idx >= ruler_list->size())
		return;
	flrd::Ruler* ruler = ruler_list->at(idx);
	flrd::ComponentAnalyzer* analyzer = view->GetCompAnalyzer();
	if (!analyzer) return;
	flrd::CelpList* list = 0;
	list = analyzer->GetCelpList();
	if (list && list->empty())
		list = 0;
	bool bval;
	long lval;
	double dval;
	long relax_type;
	m_calculator->SetCelpList(list);
	m_calculator->SetRuler(ruler);
	getValue(gstRulerF1, dval);
	m_calculator->SetF1(dval);
	getValue(gstRulerInfr, dval);
	m_calculator->SetInfr(dval);
	m_calculator->SetVolume(view->GetCurrentVolume());
	getValue(gstRulerRelaxType, relax_type);
	getValue(gstRulerEdited, bval);
	getValue(gstRulerRelaxIter, lval);
	m_calculator->CenterRuler(relax_type, bval, lval);
	setValue(gstRulerEdited, false);
	//m_view->Update(39);
	//GetSettings(m_view);
}

void MeasureAgent::Project(int idx)
{
	Renderview* view = getObject();
	if (!view) return;
	flrd::RulerList* ruler_list = view->GetRulerList();
	if (!ruler_list) return;
	if (idx < 0 || idx >= ruler_list->size())
		return;
	flrd::Ruler* ruler = ruler_list->at(idx);
	flrd::ComponentAnalyzer* analyzer = view->GetCompAnalyzer();
	flrd::CelpList* list = 0;
	if (!analyzer)
		return;
	list = analyzer->GetCelpList();
	if (list->empty())
		return;

	m_calculator->SetCelpList(list);
	m_calculator->SetRuler(ruler);
	m_calculator->Project();

	std::vector<flrd::Celp> comps;
	for (auto it = list->begin();
		it != list->end(); ++it)
		comps.push_back(it->second);
	std::sort(comps.begin(), comps.end(),
		[](const flrd::Celp &a, const flrd::Celp &b) -> bool
	{
		fluo::Point pa = a->GetProjp();
		fluo::Point pb = b->GetProjp();
		if (pa.z() != pb.z()) return pa.z() < pb.z();
		else return pa.x() < pb.x();
	});

	//export
/*	wxFileDialog *fopendlg = new wxFileDialog(
		&dlg_, "Save Analysis Data", "", "",
		"Text file (*.txt)|*.txt",
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		wxString filename = fopendlg->GetPath();
		string str = filename.ToStdString();

		std::ofstream ofs;
		ofs.open(str, std::ofstream::out);

		for (auto it = comps.begin();
			it != comps.end(); ++it)
		{
			ofs << (*it)->Id() << "\t";
			fluo::Point p = (*it)->GetProjp();
			ofs << p.x() << "\t";
			ofs << p.y() << "\t";
			ofs << p.z() << "\n";
		}
		ofs.close();
	}
	if (fopendlg)
		delete fopendlg;
*/}

void MeasureAgent::Prune(int len)
{
	std::vector<int> sel;
/*	if (dlg_.m_rulerlist->GetCurrSelection(sel))
	{
		for (size_t i = 0; i < sel.size(); ++i)
			Prune(sel[i], len);
	}
*/	//if (m_view)
	//	m_view->Update(39);
	//GetSettings(m_view);
}

void MeasureAgent::Prune(int idx, int len)
{
	Renderview* view = getObject();
	if (!view) return;
	flrd::RulerHandler* handler = view->GetRulerHandler();
	if (!handler) return;
	handler->Prune(idx, len);
}

void MeasureAgent::AlignRuler()
{
	Renderview* view = getObject();
	if (!view) return;
	flrd::RulerList* ruler_list = view->GetRulerList();
	if (!ruler_list) return;
	flrd::RulerAlign* aligner = view->GetRulerAlign();
	if (!aligner) return;

	std::vector<int> sel;
	//if (!dlg_.m_rulerlist->GetCurrSelection(sel))
	//	return;
	flrd::Ruler* ruler = ruler_list->at(sel[0]);
	aligner->SetRuler(ruler);
	long lval;
	getValue(gstAlignAxisType, lval);
	aligner->AlignRuler(lval);
	bool bval;
	getValue(gstAlignCenter, bval);
	if (bval)
		AlignCenter(ruler, 0);
}

void MeasureAgent::AlignPca()
{
	Renderview* view = getObject();
	if (!view) return;
	flrd::RulerList* ruler_list = view->GetRulerList();
	if (!ruler_list) return;
	flrd::RulerAlign* aligner = view->GetRulerAlign();
	if (!aligner) return;

	aligner->SetRulerList(ruler_list);
	long lval;
	getValue(gstAlignAxisType, lval);
	aligner->AlignPca(lval);
	bool bval;
	getValue(gstAlignCenter, bval);
	if (bval)
		AlignCenter(0, ruler_list);
}

void MeasureAgent::AlignCenter(flrd::Ruler* ruler, flrd::RulerList* ruler_list)
{
	fluo::Point center;
	bool valid_center = false;
	if (ruler)
	{
		center = ruler->GetCenter();
		valid_center = true;
	}
	else if (ruler_list && !ruler_list->empty())
	{
		for (size_t i = 0; i < ruler_list->size(); ++i)
		{
			flrd::Ruler* r = (*ruler_list)[i];
			center += r->GetCenter();
		}
		center /= double(ruler_list->size());
		valid_center = true;
	}
	if (valid_center)
	{
		Renderview* view = getObject();
		double tx, ty, tz;
		view->getValue(gstObjCtrX, tx);
		view->getValue(gstObjCtrY, ty);
		view->getValue(gstObjCtrZ, tz);
		view->setValue(gstObjTransX, tx - center.x());
		view->setValue(gstObjTransY, center.y() - ty);
		view->setValue(gstObjTransZ, center.z() - tz);
	}
}

void MeasureAgent::SelectGroup(unsigned int group)
{
/*	dlg_.m_rulerlist->ClearSelection();
	Renderview* view = getObject();
	if (!view) return;
	flrd::RulerList* ruler_list = view->GetRulerList();
	if (!ruler_list) return;
	for (int i = 0; i < (int)ruler_list->size(); i++)
	{
		flrd::Ruler* ruler = (*ruler_list)[i];
		if (!ruler) continue;
		if (ruler->Group() == group)
			dlg_.m_rulerlist->SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
	}
*/}

void MeasureAgent::DeleteSelection()
{
/*	Renderview* view = getObject();
	if (!view) return;
	flrd::RulerHandler* handler = view->GetRulerHandler();
	if (!handler) return;
	std::vector<int> sel;
	dlg_.m_rulerlist->GetCurrSelection(sel);
	handler->DeleteSelection(sel);
	UpdateRulers();
	//m_view->Update(39);
*/}

void MeasureAgent::DeleteAll(bool cur_time)
{
	Renderview* view = getObject();
	if (!view) return;
	flrd::RulerHandler* handler = view->GetRulerHandler();
	if (!handler) return;
	handler->DeleteAll(cur_time);
	UpdateRulers();
	//m_view->Update(39);
}

void MeasureAgent::Export(const std::string &filename)
{
/*	Renderview* view = getObject();
	if (!view) return;
	flrd::RulerList* ruler_list = view->GetRulerList();
	if (!ruler_list) return;

	std::ofstream os;
	OutputStreamOpen(os, filename);

	wxString str;
	wxString unit;
	int num_points;
	fluo::Point p;
	flrd::Ruler* ruler;
	long sb_unit;
	view->getValue(gstScaleBarUnit, sb_unit);
	switch (sb_unit)
	{
	case 0:
		unit = "nm";
		break;
	case 1:
	default:
		unit = L"\u03BCm";
		break;
	case 2:
		unit = "mm";
		break;
	}

	int ruler_num = ruler_list->size();
	std::vector<unsigned int> groups;
	std::vector<int> counts;
	int group_num = ruler_list->GetGroupNumAndCount(groups, counts);
	std::vector<int> group_count(group_num, 0);

	if (ruler_num > 1)
		os << "Ruler Count:\t" << ruler_num << "\n";
	if (group_num > 1)
	{
		//group count
		os << "Group Count:\t" << group_num << "\n";
		for (int i = 0; i < group_num; ++i)
		{
			os << "Group " << groups[i];
			if (i < group_num - 1)
				os << "\t";
			else
				os << "\n";
		}
		for (int i = 0; i < group_num; ++i)
		{
			os << counts[i];
			if (i < group_num - 1)
				os << "\t";
			else
				os << "\n";
		}
	}

	os << "Name\tGroup\tCount\tColor\tBranch\tLength(" << unit << ")\tAngle/Pitch(Deg)\tx1\ty1\tz1\txn\tyn\tzn\tTime\tv1\tv2\n";

	double f = 0.0;
	fluo::Color color;
	bool dfoverf;
	getValue(gstRulerDfoverf, dfoverf);
	for (size_t i = 0; i < ruler_list->size(); i++)
	{
		//for each ruler
		ruler = (*ruler_list)[i];
		if (!ruler) continue;

		os << ruler->GetName() << "\t";

		//group and count
		unsigned int group = ruler->Group();
		os << group << "\t";
		int count = 0;
		auto iter = std::find(groups.begin(), groups.end(), group);
		if (iter != groups.end())
		{
			int index = std::distance(groups.begin(), iter);
			count = ++group_count[index];
		}
		os << count << "\t";

		//color
		if (ruler->GetUseColor())
		{
			color = ruler->GetColor();
			str = wxString::Format("RGB(%d, %d, %d)",
				int(color.r() * 255), int(color.g() * 255), int(color.b() * 255));
		}
		else
			str = "N/A";
		os << str << "\t";

		//branch count
		str = wxString::Format("%d", ruler->GetNumBranch());
		os << str << "\t";
		//length
		str = wxString::Format("%.2f", ruler->GetLength());
		os << str << "\t";
		//angle
		str = wxString::Format("%.1f", ruler->GetAngle());
		os << str << "\t";

		str = "";
		//start and end points
		num_points = ruler->GetNumPoint();
		if (num_points > 0)
		{
			p = ruler->GetPoint(0)->GetPoint();
			str += wxString::Format("%.2f\t%.2f\t%.2f\t", p.x(), p.y(), p.z());
		}
		if (num_points > 1)
		{
			p = ruler->GetPoint(num_points - 1)->GetPoint();
			str += wxString::Format("%.2f\t%.2f\t%.2f\t", p.x(), p.y(), p.z());
		}
		else
			str += "\t\t\t";
		os << str;

		//time
		if (ruler->GetTimeDep())
			str = wxString::Format("%d", ruler->GetTime());
		else
			str = "N/A";
		os << str << "\t";

		//info values v1 v2
		os << ruler->GetInfoValues() << "\n";

		//export points
		if (ruler->GetNumPoint() > 2)
		{
			os << ruler->GetPosNames();
			os << ruler->GetPosValues();
		}

		//export profile
		std::vector<flrd::ProfileBin>* profile = ruler->GetProfile();
		if (profile && profile->size())
		{
			double sumd = 0.0;
			unsigned long long sumull = 0;
			os << ruler->GetInfoProfile() << "\n";
			for (size_t j = 0; j < profile->size(); ++j)
			{
				//for each profile
				int pixels = (*profile)[j].m_pixels;
				if (pixels <= 0)
					os << "0.0\t";
				else
				{
					os << (*profile)[j].m_accum / pixels << "\t";
					sumd += (*profile)[j].m_accum;
					sumull += pixels;
				}
			}
			if (dfoverf)
			{
				double avg = 0.0;
				if (sumull != 0)
					avg = sumd / double(sumull);
				if (i == 0)
				{
					f = avg;
					os << "\t" << f << "\t";
				}
				else
				{
					double df = avg - f;
					if (f == 0.0)
						os << "\t" << df << "\t";
					else
						os << "\t" << df / f << "\t";
				}
			}
			os << "\n";
		}
	}

	os.close();
*/}

void MeasureAgent::EndEdit(bool update)
{
/*	if (dlg_.m_rulerlist->m_name_text->IsShown())
	{
		dlg_.m_rulerlist->m_name_text->Hide();
		dlg_.m_rulerlist->m_center_text->Hide();
		dlg_.m_rulerlist->m_color_picker->Hide();
		dlg_.m_rulerlist->m_editing_item = -1;
		if (update) UpdateRulers();
	}
*/}

void MeasureAgent::SelectRuler(long i)
{
/*	flrd::Ruler* ruler = GetRuler(dlg_.m_rulerlist->GetItemData(i));
	if (!ruler || !ruler->GetDisp())
		return;

	wxRect rect;
	wxString str;
	//add frame text
	dlg_.m_rulerlist->GetSubItemRect(i, 0, rect);
	str = dlg_.m_rulerlist->GetText(i, 0);
	dlg_.m_rulerlist->m_name_text->SetPosition(rect.GetTopLeft());
	dlg_.m_rulerlist->m_name_text->SetSize(rect.GetSize());
	dlg_.m_rulerlist->m_name_text->SetValue(str);
	dlg_.m_rulerlist->m_name_text->Show();
	//add color picker
	dlg_.m_rulerlist->GetSubItemRect(i, 3, rect);
	dlg_.m_rulerlist->m_color_picker->SetPosition(rect.GetTopLeft());
	dlg_.m_rulerlist->m_color_picker->SetSize(rect.GetSize());
	if (ruler->GetRulerType() == 2)
	{
		//locator
		dlg_.m_rulerlist->GetSubItemRect(i, 7, rect);
		str = dlg_.m_rulerlist->GetText(i, 7);
		dlg_.m_rulerlist->m_center_text->SetPosition(rect.GetTopLeft());
		dlg_.m_rulerlist->m_center_text->SetSize(rect.GetSize());
		dlg_.m_rulerlist->m_center_text->SetValue(str);
		dlg_.m_rulerlist->m_center_text->Show();
	}
	if (ruler->GetUseColor())
	{
		fluo::Color color = ruler->GetColor();
		wxColor c(int(color.r()*255.0), int(color.g()*255.0), int(color.b()*255.0));
		dlg_.m_rulerlist->m_color_picker->SetColour(c);
	}
	dlg_.m_rulerlist->m_color_picker->Show();
*/}

Point MeasureAgent::GetRulerCenter(unsigned int i)
{
	flrd::Ruler* ruler = GetRuler(i);
	if (!ruler) return Point();
	return ruler->GetCenter();
}

flrd::Ruler* MeasureAgent::GetRuler(unsigned int i)
{
	Renderview* view = getObject();
	if (!view) return 0;
	return view->GetRuler(i);
}

void MeasureAgent::SetRulerName(unsigned int i, const std::string &name)
{
/*	flrd::Ruler* ruler = GetRuler(i);
	if (!ruler) return;
	ruler->SetName(name);
	wxString str(name);
	dlg_.m_rulerlist->SetText(dlg_.m_rulerlist->m_editing_item, 0, str);
	//m_view->Update(39);
*/}

void MeasureAgent::SetRulerCenter(unsigned int i, const Point &p)
{
	flrd::Ruler* ruler = GetRuler(i);
	if (!ruler || ruler->GetRulerType() != 2) return;
	if (!ruler->GetPoint(0)) return;
	ruler->GetPoint(0)->SetPoint(p);
/*	wxString str = wxString::Format("(%.2f, %.2f, %.2f)",
		p.x(), p.y(), p.z());
	dlg_.m_rulerlist->SetText(dlg_.m_rulerlist->m_editing_item, 7, str);
	dlg_.m_rulerlist->SetText(dlg_.m_rulerlist->m_editing_item, 9, str);
	//m_view->Update(39);
*/}

void MeasureAgent::SetRulerColor(unsigned int i, const Color &c)
{
	flrd::Ruler* ruler = GetRuler(i);
	if (!ruler) return;
	ruler->SetColor(c);
}

bool MeasureAgent::ToggleRulerDisp(unsigned int i)
{
	flrd::Ruler* ruler = GetRuler(i);
	if (!ruler) return false;
	ruler->ToggleDisp();
	return ruler->GetDisp();
}

void MeasureAgent::ToggleToolBtns(bool val)
{
/*	dlg_.m_toolbar1->ToggleTool(MeasureDlg::ID_LocatorBtn, val);
	dlg_.m_toolbar1->ToggleTool(MeasureDlg::ID_ProbeBtn, val);
	dlg_.m_toolbar1->ToggleTool(MeasureDlg::ID_ProtractorBtn, val);
	dlg_.m_toolbar1->ToggleTool(MeasureDlg::ID_RulerBtn, val);
	dlg_.m_toolbar1->ToggleTool(MeasureDlg::ID_RulerMPBtn, val);
	dlg_.m_toolbar1->ToggleTool(MeasureDlg::ID_EllipseBtn, val);
	dlg_.m_toolbar1->ToggleTool(MeasureDlg::ID_GrowBtn, val);
	dlg_.m_toolbar1->ToggleTool(MeasureDlg::ID_PencilBtn, val);
	dlg_.m_toolbar2->ToggleTool(MeasureDlg::ID_RulerEditBtn, val);
	dlg_.m_toolbar2->ToggleTool(MeasureDlg::ID_RulerDelBtn, val);
	dlg_.m_toolbar2->ToggleTool(MeasureDlg::ID_RulerMoveBtn, val);
	dlg_.m_toolbar2->ToggleTool(MeasureDlg::ID_LockBtn, val);
*/}

void MeasureAgent::Locator()
{
	Renderview* view = getObject();
	if (!view) return;
	flrd::RulerHandler* handler = view->GetRulerHandler();
	if (!handler) return;

	bool bval = false;
/*	if (dlg_.m_toolbar1->GetToolState(MeasureDlg::ID_RulerMPBtn))
		handler->FinishRuler();

	bool bval = dlg_.m_toolbar1->GetToolState(MeasureDlg::ID_LocatorBtn);
	ToggleToolBtns(false);
	dlg_.m_toolbar1->ToggleTool(MeasureDlg::ID_LocatorBtn, bval);
*/
	if (bval)
	{
		view->setValue(gstInterMode, long(5));
		handler->SetType(2);
	}
	else
	{
		view->setValue(gstInterMode, long(1));
	}
}

void MeasureAgent::Probe()
{
	Renderview* view = getObject();
	if (!view) return;
	flrd::RulerHandler* handler = view->GetRulerHandler();
	if (!handler) return;

	bool bval = false;
	/*	if (dlg_.m_toolbar1->GetToolState(MeasureDlg::ID_RulerMPBtn))
		handler->FinishRuler();

	bool bval = dlg_.m_toolbar1->GetToolState(MeasureDlg::ID_ProbeBtn);
	ToggleToolBtns(false);
	dlg_.m_toolbar1->ToggleTool(MeasureDlg::ID_ProbeBtn, bval);
*/
	if (bval)
	{
		view->setValue(gstInterMode, long(5));
		handler->SetType(3);
	}
	else
	{
		view->setValue(gstInterMode, long(1));
	}
}

void MeasureAgent::Protractor()
{
	Renderview* view = getObject();
	if (!view) return;
	flrd::RulerHandler* handler = view->GetRulerHandler();
	if (!handler) return;

	bool bval = false;
/*	if (dlg_.m_toolbar1->GetToolState(MeasureDlg::ID_RulerMPBtn))
		handler->FinishRuler();

	bool bval = dlg_.m_toolbar1->GetToolState(MeasureDlg::ID_ProtractorBtn);
	ToggleToolBtns(false);
	dlg_.m_toolbar1->ToggleTool(MeasureDlg::ID_ProtractorBtn, bval);
*/
	if (bval)
	{
		view->setValue(gstInterMode, long(5));
		handler->SetType(4);
	}
	else
	{
		view->setValue(gstInterMode, long(1));
	}
}

void MeasureAgent::Ruler()
{
	Renderview* view = getObject();
	if (!view) return;
	flrd::RulerHandler* handler = view->GetRulerHandler();
	if (!handler) return;

	bool bval = false;
/*	if (dlg_.m_toolbar1->GetToolState(MeasureDlg::ID_RulerMPBtn))
		handler->FinishRuler();

	bool bval = dlg_.m_toolbar1->GetToolState(MeasureDlg::ID_RulerBtn);
	ToggleToolBtns(false);
	dlg_.m_toolbar1->ToggleTool(MeasureDlg::ID_RulerBtn, bval);
*/
	if (bval)
	{
		view->setValue(gstInterMode, long(5));
		handler->SetType(0);
	}
	else
	{
		view->setValue(gstInterMode, long(1));
	}
}

void MeasureAgent::RulerMP()
{
	Renderview* view = getObject();
	if (!view) return;
	flrd::RulerHandler* handler = view->GetRulerHandler();
	if (!handler) return;

	bool bval = false;
/*	bool bval = dlg_.m_toolbar1->GetToolState(MeasureDlg::ID_RulerMPBtn);
	ToggleToolBtns(false);
	dlg_.m_toolbar1->ToggleTool(MeasureDlg::ID_RulerMPBtn, bval);
*/
	if (bval)
	{
		view->setValue(gstInterMode, long(5));
		handler->SetType(1);
	}
	else
	{
		view->setValue(gstInterMode, long(1));
		handler->FinishRuler();
	}
}

void MeasureAgent::Ellipse()
{
	Renderview* view = getObject();
	if (!view) return;
	flrd::RulerHandler* handler = view->GetRulerHandler();
	if (!handler) return;

	bool bval = false;
/*	if (dlg_.m_toolbar1->GetToolState(MeasureDlg::ID_RulerMPBtn))
		handler->FinishRuler();

	bool bval = dlg_.m_toolbar1->GetToolState(MeasureDlg::ID_EllipseBtn);
	ToggleToolBtns(false);
	dlg_.m_toolbar1->ToggleTool(MeasureDlg::ID_EllipseBtn, bval);
*/
	if (bval)
	{
		view->setValue(gstInterMode, long(5));
		handler->SetType(5);
	}
	else
	{
		view->setValue(gstInterMode, long(1));
	}
}

void MeasureAgent::Grow()
{
	Renderview* view = getObject();
	if (!view) return;
	flrd::RulerHandler* handler = view->GetRulerHandler();
	if (!handler) return;

	handler->FinishRuler();

	bool bval = false;
/*	bool bval = dlg_.m_toolbar1->GetToolState(MeasureDlg::ID_GrowBtn);
	ToggleToolBtns(false);
	dlg_.m_toolbar1->ToggleTool(MeasureDlg::ID_GrowBtn, bval);
*/
	if (bval)
	{
		view->setValue(gstInterMode, long(12));
		handler->SetType(1);
		if (view->GetRulerRenderer())
			view->GetRulerRenderer()->SetDrawText(false);
		//reset label volume
		fluo::VolumeData* vd = view->GetCurrentVolume();
		if (vd)
		{
			vd->GetRenderer()->clear_tex_mask();
			vd->GetRenderer()->clear_tex_label();
			vd->AddEmptyMask(0, true);
			vd->AddEmptyLabel(0, true);
		}
	}
	else
	{
		view->setValue(gstInterMode, long(1));
		if (view->GetRulerRenderer())
			view->GetRulerRenderer()->SetDrawText(true);
	}
}

void MeasureAgent::Pencil()
{
	Renderview* view = getObject();
	if (!view) return;
	flrd::RulerHandler* handler = view->GetRulerHandler();
	if (!handler) return;

	handler->FinishRuler();

	bool bval = false;
/*	bool bval = dlg_.m_toolbar1->GetToolState(MeasureDlg::ID_PencilBtn);
	ToggleToolBtns(false);
	dlg_.m_toolbar1->ToggleTool(MeasureDlg::ID_PencilBtn, bval);
*/
	if (bval)
	{
		view->setValue(gstInterMode, long(13));
		handler->SetType(1);
		//if (m_view->m_glview->GetRulerRenderer())
		//	m_view->m_glview->GetRulerRenderer()->SetDrawText(false);
	}
	else
	{
		view->setValue(gstInterMode, long(1));
		//if (m_view->m_glview->GetRulerRenderer())
		//	m_view->m_glview->GetRulerRenderer()->SetDrawText(true);
	}
}

void MeasureAgent::RulerFlip()
{
	Renderview* view = getObject();
	if (!view) return;

	int count = 0;
	std::vector<int> sel;
	flrd::RulerList* ruler_list = view->GetRulerList();
/*	if (dlg_.m_rulerlist->GetCurrSelection(sel))
	{
		for (size_t i = 0; i < sel.size(); ++i)
		{
			int index = sel[i];
			if (0 > index || ruler_list->size() <= index)
				continue;
			flrd::Ruler* r = (*ruler_list)[index];
			if (r)
				r->Reverse();
			count++;
		}
	}
	else
	{
		for (size_t i = 0; i < ruler_list->size(); ++i)
		{
			flrd::Ruler* r = (*ruler_list)[i];
			if (r)
				r->Reverse();
			count++;
		}
	}
*/
	//if (count)
	//{
	//	m_view->Update(39);
	//	GetSettings(m_view);
	//}
	setValue(gstRulerEdited, true);
}

void MeasureAgent::RulerEdit()
{
	Renderview* view = getObject();
	if (!view) return;
	flrd::RulerHandler* handler = view->GetRulerHandler();
	if (!handler) return;

	bool bval = false;
/*	if (dlg_.m_toolbar1->GetToolState(MeasureDlg::ID_RulerMPBtn))
		handler->FinishRuler();

	bool bval = dlg_.m_toolbar1->GetToolState(MeasureDlg::ID_RulerEditBtn);
	ToggleToolBtns(false);
	dlg_.m_toolbar1->ToggleTool(MeasureDlg::ID_RulerEditBtn, bval);
*/
	if (bval)
		view->setValue(gstInterMode, long(6));
	else
		view->setValue(gstInterMode, long(1));

	//m_edited = true;
}

void MeasureAgent::RulerDelete()
{
	Renderview* view = getObject();
	if (!view) return;
	flrd::RulerHandler* handler = view->GetRulerHandler();
	if (!handler) return;

	bool bval = false;
/*	if (dlg_.m_toolbar1->GetToolState(MeasureDlg::ID_RulerMPBtn))
		handler->FinishRuler();

	bool bval = dlg_.m_toolbar1->GetToolState(MeasureDlg::ID_RulerDelBtn);
	ToggleToolBtns(false);
	dlg_.m_toolbar1->ToggleTool(MeasureDlg::ID_RulerDelBtn, bval);
*/
	if (bval)
		view->setValue(gstInterMode, long(14));
	else
		view->setValue(gstInterMode, long(1));

	//m_edited = true;
}

void MeasureAgent::RulerMove()
{
	Renderview* view = getObject();
	if (!view) return;
	flrd::RulerHandler* handler = view->GetRulerHandler();
	if (!handler) return;

	bool bval = false;
/*	if (dlg_.m_toolbar1->GetToolState(MeasureDlg::ID_RulerMPBtn))
		handler->FinishRuler();

	bool bval = dlg_.m_toolbar1->GetToolState(MeasureDlg::ID_RulerMoveBtn);
	ToggleToolBtns(false);
	dlg_.m_toolbar1->ToggleTool(MeasureDlg::ID_RulerMoveBtn, bval);
*/
	if (bval)
		view->setValue(gstInterMode, long(9));
	else
		view->setValue(gstInterMode, long(1));

	//m_edited = true;
}

void MeasureAgent::RulerAvg()
{
	Renderview* view = getObject();
	if (!view) return;

	fluo::Point avg;
	int count = 0;
	std::vector<int> sel;
	flrd::RulerList* ruler_list = view->GetRulerList();
/*	if (dlg_.m_rulerlist->GetCurrSelection(sel))
	{
		for (size_t i = 0; i < sel.size(); ++i)
		{
			int index = sel[i];
			if (0 > index || ruler_list->size() <= index)
				continue;
			flrd::Ruler* r = (*ruler_list)[index];
			avg += r->GetCenter();
			count++;
		}
	}
	else
	{
		for (size_t i = 0; i < ruler_list->size(); ++i)
		{
			flrd::Ruler* r = (*ruler_list)[i];
			if (r->GetDisp())
			{
				avg += r->GetCenter();
				count++;
			}
		}
	}
*/
	if (count)
	{
		avg /= double(count);
		flrd::Ruler* ruler = new flrd::Ruler();
		ruler->SetRulerType(2);
		ruler->SetName("Average");
		ruler->AddPoint(avg);
		bool bval;
		view->getValue(gstRulerTransient, bval);
		ruler->SetTimeDep(bval);
		long lval;
		view->getValue(gstCurrentFrame, lval);
		ruler->SetTime(lval);
		ruler_list->push_back(ruler);
		//m_view->Update(39);
		//GetSettings(m_view);
	}
}

void MeasureAgent::Profile()
{
	Renderview* view = getObject();
	if (!view) return;
	flrd::RulerHandler* handler = view->GetRulerHandler();
	if (!handler) return;

	handler->SetVolumeData(view->GetCurrentVolume());
	std::vector<int> sel;
/*	if (dlg_.m_rulerlist->GetCurrSelection(sel))
	{
		//export selected
		for (size_t i = 0; i < sel.size(); ++i)
			handler->Profile(sel[i]);
	}
	else
	{
		//export all
		flrd::RulerList* ruler_list = view->GetRulerList();
		for (size_t i = 0; i < ruler_list->size(); ++i)
		{
			if ((*ruler_list)[i]->GetDisp())
				handler->Profile(i);
		}
	}
*/}

void MeasureAgent::Distance(const std::string& filename)
{
	Renderview* view = getObject();
	if (!view) return;
	flrd::RulerHandler* handler = view->GetRulerHandler();
	if (!handler) return;
	flrd::ComponentAnalyzer* analyzer = view->GetCompAnalyzer();
	if (!analyzer) return;
	handler->SetCompAnalyzer(analyzer);

	std::vector<int> sel;
	std::string fi;
/*	if (dlg_.m_rulerlist->GetCurrSelection(sel))
	{
		//export selected
		for (size_t i = 0; i < sel.size(); ++i)
		{
			fi = filename + std::to_string(i) + ".txt";
			handler->Distance(sel[i], fi);
		}
	}
	else
	{
		flrd::RulerList* ruler_list = view->GetRulerList();
		for (size_t i = 0; i < ruler_list->size(); ++i)
		{
			if ((*ruler_list)[i]->GetDisp())
			{
				fi = filename + std::to_string(i) + ".txt";
				handler->Distance(i, fi);
			}
		}
	}
*/}

void MeasureAgent::Project()
{
	std::vector<int> sel;
/*	if (dlg_.m_rulerlist->GetCurrSelection(sel))
	{
		//export selected
		for (size_t i = 0; i < sel.size(); ++i)
			Project(sel[i]);
	}
*/}

void MeasureAgent::Lock()
{
	Renderview* view = getObject();
	if (!view) return;
	flrd::RulerHandler* handler = view->GetRulerHandler();
	if (!handler) return;

	bool bval = false;
/*	if (dlg_.m_toolbar1->GetToolState(MeasureDlg::ID_RulerMPBtn))
		handler->FinishRuler();

	bool bval = dlg_.m_toolbar1->GetToolState(MeasureDlg::ID_LockBtn);
	ToggleToolBtns(false);
	dlg_.m_toolbar1->ToggleTool(MeasureDlg::ID_LockBtn, bval);
*/
	if (bval)
		view->setValue(gstInterMode, long(11));
	else
		view->setValue(gstInterMode, long(1));
}

void MeasureAgent::NewGroup()
{
	Renderview* view = getObject();
	if (!view) return;
	flrd::RulerHandler* handler = view->GetRulerHandler();
	if (!handler) return;
	handler->NewGroup();
}

void MeasureAgent::ChangeGroup(unsigned int ival)
{
	Renderview* view = getObject();
	if (!view) return;
	flrd::RulerHandler* handler = view->GetRulerHandler();
	if (!handler) return;

	//update group
	std::vector<int> sel;
/*	if (!dlg_.m_rulerlist->GetCurrSelection(sel))
		return;
	for (size_t i = 0; i < sel.size(); ++i)
	{
		int index = sel[i];
		flrd::Ruler* ruler = view->GetRuler(
			dlg_.m_rulerlist->GetItemData(index));
		if (!ruler)
			continue;
		ruler->Group(ival);
	}
*/	UpdateRulers();
}

void MeasureAgent::ToggleGroupDisp(unsigned int ival)
{
	Renderview* view = getObject();
	if (!view) return;
	flrd::RulerList* ruler_list = view->GetRulerList();
	if (!ruler_list) return;
	bool disp;
	bool first = true;
	for (int i = 0; i < (int)ruler_list->size(); i++)
	{
		flrd::Ruler* ruler = (*ruler_list)[i];
		if (!ruler) continue;
		if (ruler->Group() == ival)
		{
			if (first)
			{
				first = false;
				disp = !ruler->GetDisp();
			}
			ruler->SetDisp(disp);
/*			if (disp)
				dlg_.m_rulerlist->SetItemBackgroundColour(i, wxColour(255, 255, 255));
			else
				dlg_.m_rulerlist->SetItemBackgroundColour(i, wxColour(200, 200, 200));
*/		}
	}
	//m_view->Update(39);
}

void MeasureAgent::OnRulerF1(Event& event)
{
	Renderview* view = getObject();
	if (!view) return;
	double dval;
	getValue(gstRulerF1, dval);
	m_calculator->SetF1(dval);
}