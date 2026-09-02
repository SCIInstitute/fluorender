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
#include <CurrentObjects.h>
#include <TrackGroup.h>
#include <TrackMap.h>
#include <MainSettings.h>
#include <VolumeData.h>
#include <Cell.h>
#include <compatibility.h>

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

	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;

	int ival;
	double dval;
	bool bval;

	//create page
	if (update_all || FOUND_VALUE(gstTrackFile))
	{
		//track file
		std::wstring str = trkg->get().GetPath();
		dlg->UpdateTrackFile(str);
	}

	if (update_all || FOUND_VALUE(gstTrackIter))
	{
		ival = glbin_settings.m_track_iter;
		dlg->UpdateTrackIter(ival);
	}

	if (update_all || FOUND_VALUE(gstTrackSize))
	{
		dval = glbin_settings.m_component_size;
		dlg->UpdateTrackSize(dval);
	}

	if (update_all || FOUND_VALUE(gstTrackSimilarity))
	{
		dval = glbin_settings.m_similarity;
		dlg->UpdateTrackSimilarity(dval);
	}

	if (update_all || FOUND_VALUE(gstTrackContactFactor))
	{
		dval = glbin_settings.m_contact_factor;
		dlg->UpdateTrackContactFactor(dval);
	}

	if (update_all || FOUND_VALUE(gstTrackConsistent))
	{
		bval = glbin_settings.m_consistent_color;
		dlg->UpdateTrackConsistent(bval);
	}

	if (update_all || FOUND_VALUE(gstTrackMerge))
	{
		bval = glbin_settings.m_try_merge;
		dlg->UpdateTrackMerge(bval);
	}

	if (update_all || FOUND_VALUE(gstTrackSplit))
	{
		bval = glbin_settings.m_try_split;
		dlg->UpdateTrackSplit(bval);
	}

	//select page
	if (update_all || FOUND_VALUE(gstTrackCompId))
	{
		unsigned long id;
		if (TryToULong(m_comp_id, id))
		{
			fluo::Color c(id, vd->GetShuffle());
			dlg->UpdateTrackCompId(m_comp_id, c);
		}
	}

	if (update_all || FOUND_VALUE(gstTrackCellSize))
	{
		dval = glbin_settings.m_component_size;
		dlg->UpdateTrackCellSize(dval);
	}

	if (update_all || FOUND_VALUE(gstTrackUncertainLow))
	{
		ival = trkg->get().GetUncertainLow();
		dlg->UpdateTrackUncertainLow(ival);
	}

	//modify page
	if (update_all || FOUND_VALUE(gstTrackNewCompId))
	{
		unsigned long id;
		if (TryToULong(m_comp_id3, id))
		{
			fluo::Color c(id, vd->GetShuffle());
			dlg->UpdateTrackNewCompId(m_comp_id3, c);
		}
	}

	if (update_all || FOUND_VALUE(gstTrackClusterNum))
	{
		ival = glbin_trackmap_proc.GetClusterNum();
		dlg->UpdateTrackClusterNum(ival);
	}

	//analysis page (empty)
	//lists
	if (update_all || FOUND_VALUE(gstGhostNum))
	{
		ival = trkg->get().GetGhostNum();
		dlg->UpdateGhostNum(ival);
	}

	if (update_all || FOUND_VALUE(gstGhostEnable))
	{
		bool bval1 = trkg->get().GetDrawTail();
		bool bval2 = trkg->get().GetDrawLead();
		dlg->UpdateGhostEnable(bval1, bval2);
	}

	if (update_all || FOUND_VALUE(gstTrackList))
	{
		auto data = GetTrackViewData();
		dlg->UpdateTracks(data);
	}
}

void TrackDlgAgent::UpdateData(const UpdateRequest& request)
{

}

TrackDlg* TrackDlgAgent::GetDialog() const
{
	return static_cast<TrackDlg*>(GetWindow());
}

std::vector<TrackItem> TrackDlgAgent::BuildTrackList(
	const flrd::CelpList& sel_cells,
	bool shuffle)
{
	std::vector<TrackItem> result;

	std::vector<flrd::Celp> cells;
	for (const auto& item : sel_cells)
		cells.push_back(item.second);

	if (cells.empty())
		return result;

	std::sort(cells.begin(), cells.end(),
		[](const flrd::Celp& c1,
			const flrd::Celp& c2)
			{
				unsigned int vid1 = c1->GetVertexId();
				unsigned int vid2 = c2->GetVertexId();

				if (vid1 == vid2)
					return c1->GetSizeUi() >
						   c2->GetSizeUi();

				return vid1 < vid2;
			});

	for (size_t i = 0; i < cells.size(); ++i)
	{
		TrackItem item;

		auto cell = cells[i];

		item.id = cell->Id();
		item.color = fluo::Color(item.id, shuffle);
		item.size = int(cell->GetSizeUi());

		auto center = cell->GetCenter();

		item.x = center.x();
		item.y = center.y();
		item.z = center.z();

		unsigned int vid =
			cell->GetVertexId();

		if (vid == 0)
		{
			item.glyph = L"\u25ef";
		}
		else
		{
			bool prev =
				i > 0 &&
				cells[i - 1]->GetVertexId() == vid;

			bool next =
				i + 1 < cells.size() &&
				cells[i + 1]->GetVertexId() == vid;

			if (prev)
				item.glyph =
				next ? L"\u2502" : L"\u2514";
			else
				item.glyph =
				next ? L"\u250c" : L"\u2500";
		}

		result.push_back(std::move(item));
	}

	return result;
}

TrackViewData TrackDlgAgent::GetTrackViewData()
{
	TrackViewData data;

	auto trkg = glbin_current.GetTrackGroup();
	if (!trkg)
		return data;

	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return data;

	bool shuffle = vd->GetShuffle();

	data.cur_time =
		trkg->get().GetCurTime();

	data.prv_time =
		trkg->get().GetPrvTime();

	// current selection
	data.current =
		BuildTrackList(
			trkg->get().GetCellList(),
			shuffle);

	// previous tracked cells
	data.previous =
		BuildTrackList(
			trkg->get().GetPrevCellList(),
			shuffle);

	return data;
}