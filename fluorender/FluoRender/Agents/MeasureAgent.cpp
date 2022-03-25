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
#include <MeasureDlg.h>
#include <DistCalculator.h>
#include <RulerHandler.h>
#include <RulerAlign.h>
#include <Ruler.h>
#include <CompAnalyzer.h>
#include <FLIVR/VertexArray.h>
#include <RulerRenderer.h>
#include <FLIVR/TextureRenderer.h>
#include <FLIVR/VolumeRenderer.h>

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

void MeasureAgent::setObject(Renderview* obj)
{
	InterfaceAgent::setObject(obj);
}

Renderview* MeasureAgent::getObject()
{
	return dynamic_cast<Renderview*>(InterfaceAgent::getObject());
}

void MeasureAgent::UpdateAllSettings()
{
	Renderview* view = getObject();
	if (!view) return;
	flrd::RulerHandler* rhandler = view->GetRulerHandler();
	if (!rhandler) return;
	flrd::RulerList* rlist = view->GetRulerList();
	if (!rlist) return;

	UpdateRulers();

	dlg_.m_toolbar1->ToggleTool(MeasureDlg::ID_LocatorBtn, false);
	dlg_.m_toolbar1->ToggleTool(MeasureDlg::ID_ProbeBtn, false);
	dlg_.m_toolbar1->ToggleTool(MeasureDlg::ID_ProtractorBtn, false);
	dlg_.m_toolbar1->ToggleTool(MeasureDlg::ID_RulerBtn, false);
	dlg_.m_toolbar1->ToggleTool(MeasureDlg::ID_RulerMPBtn, false);
	dlg_.m_toolbar1->ToggleTool(MeasureDlg::ID_EllipseBtn, false);
	dlg_.m_toolbar1->ToggleTool(MeasureDlg::ID_GrowBtn, false);
	dlg_.m_toolbar1->ToggleTool(MeasureDlg::ID_PencilBtn, false);
	dlg_.m_toolbar2->ToggleTool(MeasureDlg::ID_RulerEditBtn, false);
	dlg_.m_toolbar2->ToggleTool(MeasureDlg::ID_RulerDelBtn, false);
	dlg_.m_toolbar2->ToggleTool(MeasureDlg::ID_RulerMoveBtn, false);
	dlg_.m_toolbar2->ToggleTool(MeasureDlg::ID_LockBtn, false);

	long int_mode;
	view->getValue(gstInterMode, int_mode);
	if (int_mode == 5 || int_mode == 7)
	{
		int ruler_type = m_rhdl->GetType();
		if (ruler_type == 0)
			m_toolbar1->ToggleTool(ID_RulerBtn, true);
		else if (ruler_type == 1)
			m_toolbar1->ToggleTool(ID_RulerMPBtn, true);
		else if (ruler_type == 2)
			m_toolbar1->ToggleTool(ID_LocatorBtn, true);
		else if (ruler_type == 3)
			m_toolbar1->ToggleTool(ID_ProbeBtn, true);
		else if (ruler_type == 4)
			m_toolbar1->ToggleTool(ID_ProtractorBtn, true);
		else if (ruler_type == 5)
			m_toolbar1->ToggleTool(ID_EllipseBtn, true);
	}
	else if (int_mode == 6)
		m_toolbar2->ToggleTool(ID_RulerEditBtn, true);
	else if (int_mode == 9)
		m_toolbar2->ToggleTool(ID_RulerMoveBtn, true);
	else if (int_mode == 11)
		m_toolbar2->ToggleTool(ID_LockBtn, true);
	else if (int_mode == 12)
		m_toolbar1->ToggleTool(ID_GrowBtn, true);
	else if (int_mode == 13)
		m_toolbar1->ToggleTool(ID_PencilBtn, true);
	else if (int_mode == 14)
		m_toolbar2->ToggleTool(ID_RulerDelBtn, true);

	long point_volume_mode;
	m_view->getValue(gstPointVolumeMode, point_volume_mode);
	switch (point_volume_mode)
	{
	case 0:
		m_view_plane_rd->SetValue(true);
		m_max_intensity_rd->SetValue(false);
		m_acc_intensity_rd->SetValue(false);
		break;
	case 1:
		m_view_plane_rd->SetValue(false);
		m_max_intensity_rd->SetValue(true);
		m_acc_intensity_rd->SetValue(false);
		break;
	case 2:
		m_view_plane_rd->SetValue(false);
		m_max_intensity_rd->SetValue(false);
		m_acc_intensity_rd->SetValue(true);
		break;
	}

	bool bval;
	m_view->getValue(gstRulerUseTransf, bval);
	m_use_transfer_chk->SetValue(bval);
	m_view->getValue(gstRulerTransient, bval);
	m_transient_chk->SetValue(bval);
	if (m_frame && m_frame->GetSettingDlg())
	{
		//ruler exports df/f
		bool bval = m_frame->GetSettingDlg()->GetRulerDF_F();
		m_df_f_chk->SetValue(bval);
		m_rulerlist->m_ruler_df_f = bval;
		//relax
		m_relax_value_spin->SetValue(
			m_frame->GetSettingDlg()->GetRulerRelaxF1());
		m_auto_relax_btn->SetValue(
			m_frame->GetSettingDlg()->GetRulerAutoRelax());
		m_view->setValue(gstRulerRelax,
			m_frame->GetSettingDlg()->GetRulerAutoRelax());
		m_relax_data_cmb->Select(
			m_frame->GetSettingDlg()->GetRulerRelaxType());
	}
}

void MeasureAgent::UpdateRulers()
{
	m_name_text->Hide();
	m_center_text->Hide();
	m_color_picker->Hide();
	if (view)
		m_view = view;

	flrd::RulerList* ruler_list = m_view->GetRulerList();
	if (!ruler_list) return;

	std::vector<int> sel;
	GetCurrSelection(sel);

	std::vector<unsigned int> groups;
	int group_num = ruler_list->GetGroupNum(groups);
	std::vector<int> group_count(group_num, 0);

	DeleteAllItems();

	wxString points;
	fluo::Point p;
	int num_points;
	long cur_frame, sb_unit;
	m_view->getValue(gstCurrentFrame, cur_frame);
	m_view->getValue(gstScaleBarUnit, sb_unit);
	for (int i = 0; i < (int)ruler_list->size(); i++)
	{
		flrd::Ruler* ruler = (*ruler_list)[i];
		if (!ruler) continue;
		if (ruler->GetTimeDep() &&
			ruler->GetTime() != cur_frame)
			continue;

		wxString unit;
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

		points = "";
		num_points = ruler->GetNumPoint();
		if (num_points > 0)
		{
			p = ruler->GetPoint(0)->GetPoint();
			points += wxString::Format("(%.2f, %.2f, %.2f)", p.x(), p.y(), p.z());
		}
		if (num_points > 1)
		{
			p = ruler->GetPoint(num_points - 1)->GetPoint();
			points += ", ";
			points += wxString::Format("(%.2f, %.2f, %.2f)", p.x(), p.y(), p.z());
		}
		unsigned int group = ruler->Group();
		int count = 0;
		auto iter = std::find(groups.begin(), groups.end(), group);
		if (iter != groups.end())
		{
			int index = std::distance(groups.begin(), iter);
			count = ++group_count[index];
		}
		wxString color;
		if (ruler->GetUseColor())
			color = wxString::Format("RGB(%d, %d, %d)",
				int(ruler->GetColor().r() * 255),
				int(ruler->GetColor().g() * 255),
				int(ruler->GetColor().b() * 255));
		else
			color = "N/A";
		wxString center;
		fluo::Point cp = ruler->GetCenter();
		center = wxString::Format("(%.2f, %.2f, %.2f)",
			cp.x(), cp.y(), cp.z());
		wxString str = ruler->GetDelInfoValues(", ");
		Append(ruler->GetDisp(), ruler->Id(),
			ruler->GetName(), group, count,
			color, ruler->GetNumBranch(), ruler->GetLength(), unit,
			ruler->GetAngle(), center, ruler->GetTimeDep(),
			ruler->GetTime(), str, points);
	}

	AdjustSize();

	for (size_t i = 0; i < sel.size(); ++i)
	{
		int index = sel[i];
		if (0 > index || ruler_list->size() <= index)
			continue;
		SetItemState(index, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
	}

	flvr::TextureRenderer::vertex_array_manager_.set_dirty(flvr::VA_Rulers);
}

void MeasureAgent::Relax()
{
	std::vector<int> sel;
	if (m_rulerlist->GetCurrSelection(sel))
	{
		for (size_t i = 0; i < sel.size(); ++i)
			Relax(sel[i]);
	}
}

void MeasureAgent::Relax(int idx)
{
	if (!m_view)
		return;
	flrd::RulerList* ruler_list = m_view->GetRulerList();
	if (!ruler_list)
		return;
	if (idx < 0 || idx >= ruler_list->size())
		return;
	flrd::Ruler* ruler = ruler_list->at(idx);
	flrd::ComponentAnalyzer* analyzer = 0;
	analyzer = m_view->GetCompAnalyzer();
	if (!analyzer)
		return;
	flrd::CelpList* list = 0;
	list = analyzer->GetCelpList();
	if (list && list->empty())
		list = 0;
	double infr = 2.0;
	int type = 1;
	int iter = 10;
	if (m_frame && m_frame->GetSettingDlg())
	{
		iter = m_frame->GetSettingDlg()->GetRulerRelaxIter();
		infr = m_frame->GetSettingDlg()->GetRulerInfr();
		type = m_frame->GetSettingDlg()->GetRulerRelaxType();
	}

	m_calculator->SetF1(m_relax_value_spin->GetValue());
	m_calculator->SetInfr(infr);
	m_calculator->SetCelpList(list);
	m_calculator->SetRuler(ruler);
	m_calculator->SetVolume(m_view->GetCurrentVolume());
	m_calculator->CenterRuler(type, m_edited, iter);
	m_edited = false;
	m_view->Update(39);
	GetSettings(m_view);
}

void MeasureAgent::Project(int idx)
{
	if (!m_view || !m_frame)
		return;
	flrd::RulerList* ruler_list = m_view->GetRulerList();
	if (!ruler_list)
		return;
	if (idx < 0 || idx >= ruler_list->size())
		return;
	flrd::Ruler* ruler = ruler_list->at(idx);
	flrd::ComponentAnalyzer* analyzer =
		m_view->GetCompAnalyzer();
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
	wxFileDialog *fopendlg = new wxFileDialog(
		this, "Save Analysis Data", "", "",
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
}

void MeasureAgent::Prune(int len)
{
	std::vector<int> sel;
	if (m_rulerlist->GetCurrSelection(sel))
	{
		for (size_t i = 0; i < sel.size(); ++i)
			Prune(sel[i], len);
	}
	if (m_view)
		m_view->Update(39);
	GetSettings(m_view);
}

void MeasureAgent::Prune(int idx, int len)
{
	m_rhdl->Prune(idx, len);
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
		double tx, ty, tz;
		m_view->getValue(gstObjCtrX, tx);
		m_view->getValue(gstObjCtrY, ty);
		m_view->getValue(gstObjCtrZ, tz);
		m_view->setValue(gstObjTransX, tx - center.x());
		m_view->setValue(gstObjTransY, center.y() - ty);
		m_view->setValue(gstObjTransZ, center.z() - tz);
	}
}

void MeasureAgent::SelectGroup(unsigned int group)
{
	ClearSelection();
	if (!m_view)
		return;
	flrd::RulerList* ruler_list = m_view->GetRulerList();
	if (!ruler_list) return;
	for (int i = 0; i < (int)ruler_list->size(); i++)
	{
		flrd::Ruler* ruler = (*ruler_list)[i];
		if (!ruler) continue;
		if (ruler->Group() == group)
			SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
	}
}

void MeasureAgent::DeleteSelection()
{
	if (!m_rhdl)
		return;
	std::vector<int> sel;
	GetCurrSelection(sel);
	m_rhdl->DeleteSelection(sel);
	UpdateRulers();
	m_view->Update(39);
}

void MeasureAgent::DeleteAll(bool cur_time)
{
	if (!m_rhdl)
		return;
	m_rhdl->DeleteAll(cur_time);
	UpdateRulers();
	m_view->Update(39);
}

void MeasureAgent::Export(const std::string &filename)
{
	if (!m_view) return;
	flrd::RulerList* ruler_list = m_view->GetRulerList();
	if (ruler_list)
	{
		std::ofstream os;
		OutputStreamOpen(os, filename.ToStdString());

		wxString str;
		wxString unit;
		int num_points;
		fluo::Point p;
		flrd::Ruler* ruler;
		long sb_unit;
		m_view->getValue(gstScaleBarUnit, sb_unit);
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
			vector<flrd::ProfileBin>* profile = ruler->GetProfile();
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
				if (m_ruler_df_f)
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
	}
}

void MeasureAgent::EndEdit(bool update)
{
	if (m_name_text->IsShown())
	{
		m_name_text->Hide();
		m_center_text->Hide();
		m_color_picker->Hide();
		m_editing_item = -1;
		if (update) UpdateRulers();
	}
}

