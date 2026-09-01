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

#include <TrackDlgAgent.h>
#include <TrackDlg.h>
#include <Global.h>
#include <Names.h>

TrackDlgAgent::TrackDlgAgent(
	TrackDlg* dlg) :
	Agent(dlg)
{

}

bool TrackDlgAgent::Accept(
	const UpdateRequest& request) const
{
	return true;
}

void TrackDlgAgent::Update(
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

void TrackDlgAgent::UpdateUI(const UpdateRequest& request)
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	if (FOUND_VALUE(gstNull))
		return;

	bool update_all = request.values.empty();

	auto trkg = glbin_current.GetTrackGroup();
	if (!trkg)
		return;

	wxString str;
	int ival;
	double dval;

	//create page
	if (update_all || FOUND_VALUE(gstTrackFile))
	{
		//track file
		str = trkg->get().GetPath();
		if (str.IsEmpty())
			m_load_trace_text->ChangeValue("No track map or track map not saved");
		else
			m_load_trace_text->ChangeValue(str);
	}

	if (update_all || FOUND_VALUE(gstTrackIter))
		m_map_iter_spin->SetValue(glbin_settings.m_track_iter);

	if (update_all || FOUND_VALUE(gstTrackSize))
		m_map_size_spin->SetValue(glbin_settings.m_component_size);

	if (update_all || FOUND_VALUE(gstTrackSimilarity))
		m_map_similar_spin->SetValue(glbin_settings.m_similarity);

	if (update_all || FOUND_VALUE(gstTrackContactFactor))
		m_map_contact_spin->SetValue(glbin_settings.m_contact_factor);

	if (update_all || FOUND_VALUE(gstTrackConsistent))
		m_map_consistent_btn->SetValue(glbin_settings.m_consistent_color);

	if (update_all || FOUND_VALUE(gstTrackMerge))
		m_map_merge_btn->SetValue(glbin_settings.m_try_merge);

	if (update_all || FOUND_VALUE(gstTrackSplit))
		m_map_split_btn->SetValue(glbin_settings.m_try_split);

	//select page
	if (update_all || FOUND_VALUE(gstTrackCompId))
	{
		m_comp_id_text->ChangeValue(m_comp_id);
		m_comp_id_text2->ChangeValue(m_comp_id);
		unsigned long id;
		wxColor color(255, 255, 255);
		if (m_comp_id.ToULong(&id))
		{
			if (!id)
				color = wxColor(24, 167, 181);
			else
			{
				auto vd = glbin_current.vol_data.lock();
				bool shuffle = vd ? vd->GetShuffle() : 0;
				fluo::Color c(id, shuffle);
				color = wxColor(c.r() * 255, c.g() * 255, c.b() * 255);
			}
		}
		m_comp_id_text->SetBackgroundColour(color);
		m_comp_id_text2->SetBackgroundColour(color);
	}

	if (update_all || FOUND_VALUE(gstTrackCellSize))
	{
		dval = glbin_settings.m_component_size;
		m_cell_size_sldr->ChangeValue(int(std::round(dval)));
		m_cell_size_text->ChangeValue(wxString::Format("%.0f", dval));
	}

	if (update_all || FOUND_VALUE(gstTrackUncertainLow))
	{
		ival = trkg->get().GetUncertainLow();
		m_comp_uncertain_low_sldr->ChangeValue(ival);
		m_cell_size_text->ChangeValue(wxString::Format("%d", ival));
	}

	//modify page
	if (update_all || FOUND_VALUE(gstTrackNewCompId))
	{
		m_cell_new_id_text->ChangeValue(m_comp_id3);
		unsigned long id;
		wxColor color(255, 255, 255);
		if (m_comp_id3.ToULong(&id))
		{
			if (!id)
				color = wxColor(24, 167, 181);
			else
			{
				auto vd = glbin_current.vol_data.lock();
				bool shuffle = vd ? vd->GetShuffle() : 0;
				fluo::Color c(id, shuffle);
				color = wxColor(c.r() * 255, c.g() * 255, c.b() * 255);
			}
		}
		m_cell_new_id_text->SetBackgroundColour(color);
	}

	if (update_all || FOUND_VALUE(gstTrackClusterNum))
	{
		ival = glbin_trackmap_proc.GetClusterNum();
		m_cell_segment_spin->SetValue(wxString::Format("%d", ival));
	}

	//analysis page (empty)
	//lists
	if (update_all || FOUND_VALUE(gstGhostNum))
	{
		ival = trkg->get().GetGhostNum();
		m_ghost_num_sldr->ChangeValue(ival);
		m_ghost_num_text->ChangeValue(wxString::Format("%d", ival));
	}

	if (update_all || FOUND_VALUE(gstGhostEnable))
	{
		m_ghost_show_tail_chk->SetValue(trkg->get().GetDrawTail());
		m_ghost_show_lead_chk->SetValue(trkg->get().GetDrawLead());
	}

	if (update_all || FOUND_VALUE(gstTrackList))
	{
		UpdateTrackList();
		UpdateTracks();
		//Layout();
	}
}

void TrackDlgAgent::UpdateData(const UpdateRequest& request)
{

}

TrackDlg* TrackDlgAgent::GetDialog() const
{
	return static_cast<TrackDlg*>(GetWindow());
}

