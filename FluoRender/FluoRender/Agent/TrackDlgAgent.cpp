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

