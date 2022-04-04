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

#include <TrackAgent.hpp>
#include <TrackDlg.h>
#include <Tracks.h>

using namespace fluo;

TrackAgent::TrackAgent(TrackDlg &dlg) :
	InterfaceAgent(),
	dlg_(dlg)
{
}

void TrackAgent::setObject(Renderview* obj)
{
	InterfaceAgent::setObject(obj);
}

Renderview* TrackAgent::getObject()
{
	return dynamic_cast<Renderview*>(InterfaceAgent::getObject());
}

void TrackAgent::UpdateAllSettings()
{
	m_view = vrv;
	if (!m_view)
		return;

	m_view->getValue(gstCurrentFrame, m_cur_time);
	m_view->getValue(gstPreviousFrame, m_prv_time);

	flrd::Tracks* trace_group = m_view->GetTraceGroup();
	if (trace_group)
	{
		wxString str;
		//cell size filter
		str = m_cell_size_text->GetValue();
		unsigned long ival;
		str.ToULong(&ival);
		trace_group->SetCellSize(ival);

		str = trace_group->GetPath();
		if (str != "")
			m_load_trace_text->SetValue(str);
		else
			m_load_trace_text->SetValue("Track map created but not saved");
		UpdateList();

		int ghost_num = trace_group->GetGhostNum();
		m_ghost_num_text->ChangeValue(wxString::Format("%d", ghost_num));
		m_ghost_num_sldr->SetValue(ghost_num);
		m_ghost_show_tail_chk->SetValue(trace_group->GetDrawTail());
		m_ghost_show_lead_chk->SetValue(trace_group->GetDrawLead());
	}
	else
	{
		m_load_trace_text->SetValue("No Track map");
	}

	//settings for tracking
	if (m_frame && m_frame->GetSettingDlg())
	{
		m_iter_num =
			m_frame->GetSettingDlg()->GetTrackIter();
		m_size_thresh =
			m_frame->GetSettingDlg()->GetComponentSize();
		m_consistent_color =
			m_frame->GetSettingDlg()->GetConsistentColor();
		m_try_merge =
			m_frame->GetSettingDlg()->GetTryMerge();
		m_try_split =
			m_frame->GetSettingDlg()->GetTrySplit();
		m_similarity =
			m_frame->GetSettingDlg()->GetSimilarity();
		m_contact_factor =
			m_frame->GetSettingDlg()->GetContactFactor();
		//
		m_map_iter_spin->SetValue(m_iter_num);
		m_map_size_spin->SetValue(m_size_thresh);
		m_map_consistent_btn->SetValue(m_consistent_color);
		m_map_merge_btn->SetValue(m_try_merge);
		m_map_split_btn->SetValue(m_try_split);
		m_map_similar_spin->SetValue(m_similarity);
		m_map_contact_spin->SetValue(m_contact_factor);
	}
}

void TrackAgent::UpdateTraces()
{
	Renderview* view = getObject();
	if (!view) return;
	flrd::Tracks* traces = view->GetTraceGroup();
	if (!traces) return;
	int shuffle = 0;
	VolumeData* vd = view->GetCurrentVolume();
	if (vd) shuffle = vd->GetShuffle();

	dlg_.m_trace_list_curr->DeleteAllItems();

	flrd::CelpList sel_cells = traces->GetCellList();
	std::vector<flrd::Celp> cells;
	for (auto siter = sel_cells.begin();
		siter != sel_cells.end(); ++siter)
		cells.push_back(siter->second);

	if (cells.empty())
		return;
	else
		std::sort(cells.begin(), cells.end(),
			[](const flrd::Celp &c1, const flrd::Celp &c2) -> bool
	{
		unsigned int vid1 = c1->GetVertexId();
		unsigned int vid2 = c2->GetVertexId();
		if (vid1 == vid2)
			return c1->GetSizeUi() > c2->GetSizeUi();
		else
			return vid1 < vid2;
	});

	wxString gtype;
	unsigned int id;
	unsigned int vid;
	Color c;
	wxColor wxc;
	int size;
	Point center;
	bool prev, next;

	for (size_t i = 0; i < cells.size(); ++i)
	{
		id = cells[i]->Id();
		vid = cells[i]->GetVertexId();
		c = Color(id, shuffle);
		wxColor color(c.r() * 255, c.g() * 255, c.b() * 255);
		size = (int)(cells[i]->GetSizeUi());
		center = cells[i]->GetCenter();

		if (vid == 0)
			gtype = L"\u25ef";
		else
		{
			if (i == 0)
				prev = false;
			else
				prev = cells[i - 1]->GetVertexId() == vid;
			if (i == cells.size() - 1)
				next = false;
			else
				next = cells[i + 1]->GetVertexId() == vid;
			if (prev)
			{
				if (next)
					gtype = L"\u2502";
				else
					gtype = L"\u2514";
			}
			else
			{
				if (next)
					gtype = L"\u250c";
				else
					gtype = L"\u2500";
			}
		}

		dlg_.m_trace_list_curr->Append(gtype, id, color, size,
			center.x(), center.y(), center.z());
	}
}

void TrackAgent::UpdateList()
{
	if (!m_view)
		return;

	int shuffle = 0;
	fluo::VolumeData* vd = m_view->GetCurrentVolume();
	if (vd)
		shuffle = vd->GetShuffle();
	flrd::Tracks* trace_group = m_view->GetTraceGroup();
	if (trace_group)
	{
		int cur_time = trace_group->GetCurTime();
		int prv_time = trace_group->GetPrvTime();
		wxString item_gtype;
		wxString item_id;
		wxString item_size;
		wxString item_x;
		wxString item_y;
		wxString item_z;
		unsigned long id;
		double hue;
		long size;
		double x, y, z;
		//copy current to previous
		if (cur_time != prv_time)
		{
			m_trace_list_prev->DeleteAllItems();
			long item = -1;
			for (;;)
			{
				item = m_trace_list_curr->GetNextItem(item,
					wxLIST_NEXT_ALL,
					wxLIST_STATE_DONTCARE);
				if (item != -1)
				{
					item_gtype = m_trace_list_curr->GetText(item, 0);
					item_id = m_trace_list_curr->GetText(item, 1);
					item_size = m_trace_list_curr->GetText(item, 2);
					item_x = m_trace_list_curr->GetText(item, 3);
					item_y = m_trace_list_curr->GetText(item, 4);
					item_z = m_trace_list_curr->GetText(item, 5);
					item_id.ToULong(&id);
					fluo::Color c(id, shuffle);
					wxColor color(c.r() * 255, c.g() * 255, c.b() * 255);
					item_size.ToLong(&size);
					item_x.ToDouble(&x);
					item_y.ToDouble(&y);
					item_z.ToDouble(&z);
					m_trace_list_prev->Append(item_gtype, id, color, size, x, y, z);
				}
				else break;
			}

			if (cur_time >= 0) m_cur_time = cur_time;
			if (prv_time >= 0) m_prv_time = prv_time;

			//set tiem text
			wxString str;
			str = wxString::Format("\tCurrent T: %d", m_cur_time);
			m_cell_time_curr_st->SetLabel(str);
			if (m_prv_time != m_cur_time)
				m_cell_time_prev_st->SetLabel(
					wxString::Format("\tPrevious T: %d", m_prv_time));
			else
				m_cell_time_prev_st->SetLabel("\tPrevious T");
		}
	}
	UpdateTraces();
	dlg_.Layout();
}

