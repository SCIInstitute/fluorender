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
#include <RenderView.h>
#include <ModalDlg.h>
#include <Directory.h>
#include <CompSelector.h>
#include <CompEditor.h>
#include <CompAnalyzer.h>
#include <MovieMaker.h>
#include <compatibility.h>

TrackDlgAgent::TrackDlgAgent(
	TrackDlg* dlg) :
	Agent(dlg)
{
	glbin_trackmap_proc.RegisterInfoOutFunc(
		std::bind(&TrackDlgAgent::WriteInfo, this, std::placeholders::_1));
}

void TrackDlgAgent::UpdateUI(const UpdateRequest& request)
{
	auto dlg = GetDialog();
	if (!dlg)
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
	if (update_all || request.HasValue(gstTrackFile))
	{
		//track file
		std::wstring str = trkg->get().GetPath();
		dlg->UpdateTrackFile(str);
	}

	if (update_all || request.HasValue(gstTrackIter))
	{
		ival = glbin_settings.m_track_iter;
		dlg->UpdateTrackIter(ival);
	}

	if (update_all || request.HasValue(gstTrackSize))
	{
		dval = glbin_settings.m_component_size;
		dlg->UpdateTrackSize(dval);
	}

	if (update_all || request.HasValue(gstTrackSimilarity))
	{
		dval = glbin_settings.m_similarity;
		dlg->UpdateTrackSimilarity(dval);
	}

	if (update_all || request.HasValue(gstTrackContactFactor))
	{
		dval = glbin_settings.m_contact_factor;
		dlg->UpdateTrackContactFactor(dval);
	}

	if (update_all || request.HasValue(gstTrackConsistent))
	{
		bval = glbin_settings.m_consistent_color;
		dlg->UpdateTrackConsistent(bval);
	}

	if (update_all || request.HasValue(gstTrackMerge))
	{
		bval = glbin_settings.m_try_merge;
		dlg->UpdateTrackMerge(bval);
	}

	if (update_all || request.HasValue(gstTrackSplit))
	{
		bval = glbin_settings.m_try_split;
		dlg->UpdateTrackSplit(bval);
	}

	//select page
	if (update_all || request.HasValue(gstTrackCompId))
	{
		unsigned long id;
		if (TryToULong(m_comp_id, id))
		{
			fluo::Color c(id, vd->GetShuffle());
			dlg->UpdateTrackCompId(m_comp_id, c);
		}
	}

	if (update_all || request.HasValue(gstTrackCellSize))
	{
		dval = glbin_settings.m_component_size;
		dlg->UpdateTrackCellSize(dval);
	}

	if (update_all || request.HasValue(gstTrackUncertainLow))
	{
		ival = trkg->get().GetUncertainLow();
		dlg->UpdateTrackUncertainLow(ival);
	}

	//modify page
	if (update_all || request.HasValue(gstTrackNewCompId))
	{
		unsigned long id;
		if (TryToULong(m_comp_id3, id))
		{
			fluo::Color c(id, vd->GetShuffle());
			dlg->UpdateTrackNewCompId(m_comp_id3, c);
		}
	}

	if (update_all || request.HasValue(gstTrackClusterNum))
	{
		ival = glbin_trackmap_proc.GetClusterNum();
		dlg->UpdateTrackClusterNum(ival);
	}

	//analysis page (empty)
	//lists
	if (update_all || request.HasValue(gstGhostNum))
	{
		ival = trkg->get().GetGhostNum();
		dlg->UpdateGhostNum(ival);
	}

	if (update_all || request.HasValue(gstGhostEnable))
	{
		bool bval1 = trkg->get().GetDrawTail();
		bool bval2 = trkg->get().GetDrawLead();
		dlg->UpdateGhostEnable(bval1, bval2);
	}

	if (update_all || request.HasValue(gstTrackList))
	{
		auto data = GetTrackViewData();
		dlg->UpdateTracks(data);
	}
}

void TrackDlgAgent::UpdateData(const UpdateRequest& request)
{
	if (request.HasValue(gstClearTrack))
		ClearTrack();
	if (request.HasValue(gstTrackFile))
		LoadTrackFile();
	if (request.HasValue(gstSaveTrackFile))
		SaveTrackFile();
	if (request.HasValue(gstSaveAsTrackFile))
		SaveAsTrackFile();
	if (request.HasValue(gstGenerateMap))
		GenerateMap();
	if (request.HasValue(gstRefineTime))
		RefineTime();
	if (request.HasValue(gstRefineAll))
		RefineAll();
	if (request.HasValue(gstTrackIter))
		SetMapIter();
	if (request.HasValue(gstTrackSize))
		SetMapSize();
	if (request.HasValue(gstTrackConsistent))
		SetMapConsistent();
	if (request.HasValue(gstTrackMerge))
		SetTryMerge();
	if (request.HasValue(gstTrackSplit))
		SetTrySplit();
	if (request.HasValue(gstTrackSimilarity))
		SetMapSimilarity();
	if (request.HasValue(gstTrackContactFactor))
		SetMapContact();
	if (request.HasValue(gstTrackCompId))
		SetCompId();
	if (request.HasValue(gstTrackClearCompId))
		ClearCompId();
	if (request.HasValue(gstCompFull))
		SetCompFull();
	if (request.HasValue(gstCompExclusive))
		SetCompExclusive();
	if (request.HasValue(gstCompAppend))
		SetCompAppend();
	if (request.HasValue(gstCompClear))
		SetCompClear();
	if (request.HasValue(gstShuffle))
		SetShuffle();

	if (request.HasValue(gstTrackCellSize))
		SetCellSize();
	if (request.HasValue(gstComputeUncertainty))
		ComputeUncertainty();
	if (request.HasValue(gstTrackUncertainLow))
		SetCompUncertaintyLow();
	if (request.HasValue(gstTrackCompId2))
		SetCompId2();
	if (request.HasValue(gstCellExclusiveLink))
		SetCellExclusiveLink();
	if (request.HasValue(gstCellLink))
		SetCellLink();
	if (request.HasValue(gstCellLinkAll))
		SetCellLinkAll();
	if (request.HasValue(gstCellIsolate))
		SetCellIsolate();
	if (request.HasValue(gstCellUnlink))
		SetCellUnlink();
	if (request.HasValue(gstTrackNewCompId))
		SetCellNewId();
	if (request.HasValue(gstTrackClearCompNewId))
		ClearCompNewId();
	if (request.HasValue(gstCreateCellNewId))
		CreateCellNewId();
	if (request.HasValue(gstCellAppendId))
		SetCellAppendId();
	if (request.HasValue(gstCellReplaceId))
		SetCellReplaceId();
	if (request.HasValue(gstCellCombineId))
		SetCellCombineId();
	if (request.HasValue(gstCellSeparateId))
		SetCellSeparateId();
	if (request.HasValue(gstCellSegment))
		SetCellSegment();
	if (request.HasValue(gstCellClusterNum))
		SetClusterNum();

	if (request.HasValue(gstTrackConvertRulers))
		ConvertRulers();
	if (request.HasValue(gstTrackConsistent))
		ConvertConsistent();
	if (request.HasValue(gstAnalyzeComps))
		AnalyzeComps();
	if (request.HasValue(gstAnalyzeLinks))
		AnalyzeLinks();
	if (request.HasValue(gstAnalyzeUncertainty))
		AnalyzeUncertainty();
	if (request.HasValue(gstAnalyzePaths))
		AnalyzePaths();
	if (request.HasValue(gstTrackSaveResult))
		SaveTrackResult();
	if (request.HasValue(gstCellPrev))
		SetCellPrev();
	if (request.HasValue(gstCellNext))
		SetCellNext();
	if (request.HasValue(gstGhostNum))
		SetGhostNum();
	if (request.HasValue(gstGhostShowTail))
		SetGhostShowTail();
	if (request.HasValue(gstGhostShowLead))
		SetGhostShowLead();

	if (request.HasValue(gstTrackListSel))
		SetListSelection();
	if (request.HasValue(gstTrackListDelete))
		DeleteSelection();
}

TrackDlg* TrackDlgAgent::GetDialog() const
{
	return static_cast<TrackDlg*>(GetOwner());
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

void TrackDlgAgent::WriteInfo(const std::wstring& str)
{
	auto dlg = GetDialog();
	if (dlg)
		dlg->UpdateStatText(str);
}

void TrackDlgAgent::ClearTrack()
{
	auto trkg = glbin_current.GetTrackGroup();
	if (trkg)
		trkg->get().Clear();
	UpdateDataToUI({ gstTrackFile });
}

void TrackDlgAgent::LoadTrackFile()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;

	ModalDlg fopendlg(
		dlg, "Choose a FluoRender track file",
		"", "", "*.track", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	int rval = fopendlg.ShowModal();
	if (rval == wxID_OK)
	{
		std::wstring filename = fopendlg.GetPath().ToStdWstring();
		view->LoadTrackGroup(filename);
		UpdateDataToUI({ gstTrackFile });
	}
}

void TrackDlgAgent::SaveTrackFile()
{
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;
	auto trkg = glbin_current.GetTrackGroup();
	if (!trkg)
		return;
	std::wstring str = trkg->get().GetPath();
	if (std::filesystem::exists(str))
		view->SaveTrackGroup(str);
	else
		SaveAsTrackFile();
}

void TrackDlgAgent::SaveAsTrackFile()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;
	ModalDlg fopendlg(
		dlg, "Save a FluoRender track file",
		"", "", "*.track", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	int rval = fopendlg.ShowModal();
	if (rval == wxID_OK)
	{
		std::wstring filename = fopendlg.GetPath().ToStdWstring();
		view->SaveTrackGroup(filename);
		UpdateDataToUI({ gstTrackFile });
	}
}

void TrackDlgAgent::GenerateMap()
{
	glbin_trackmap_proc.GenMap();
	//enable script
	glbin_settings.m_run_script = true;
	std::filesystem::path p = GetUserSettingsRoot();
	p = p / "Scripts" / "track_selected_results.txt";
	glbin_settings.m_script_file = p.wstring();
	NotifyDataToUI({ gstMovPlay, gstRunScript, gstScriptFile, gstScriptSelect });
}

void TrackDlgAgent::RefineTime()
{
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;

	glbin_trackmap_proc.RefineMap(view->m_tseq_cur_num);
}

void TrackDlgAgent::RefineAll()
{
	glbin_trackmap_proc.RefineMap();
}

void TrackDlgAgent::SetMapIter()
{
	auto dlg = GetDialog();
	if (dlg)
		glbin_settings.m_track_iter = dlg->GetMapIter();
}

void TrackDlgAgent::SetMapSize()
{
	auto dlg = GetDialog();
	if (dlg)
		glbin_settings.m_component_size = dlg->GetMapSize();
}

void TrackDlgAgent::SetMapConsistent()
{
	auto dlg = GetDialog();
	if (dlg)
		glbin_settings.m_consistent_color = dlg->GetMapConsistent();
}

void TrackDlgAgent::SetTryMerge()
{
	auto dlg = GetDialog();
	if (dlg)
		glbin_settings.m_try_merge = dlg->GetMapMerge();
}

void TrackDlgAgent::SetTrySplit()
{
	auto dlg = GetDialog();
	if (dlg)
		glbin_settings.m_try_split = dlg->GetMapSplit();
}

void TrackDlgAgent::SetMapSimilarity()
{
	auto dlg = GetDialog();
	if (dlg)
		glbin_settings.m_similarity = dlg->GetMapSimilarity();
}

void TrackDlgAgent::SetMapContact()
{
	auto dlg = GetDialog();
	if (dlg)
		glbin_settings.m_contact_factor = dlg->GetMapContact();
}

void TrackDlgAgent::SetCompId()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	m_comp_id = dlg->GetCompId();
	unsigned long ival;
	if (m_comp_id.empty())
		glbin_comp_selector.SetId(0, true);
	else if (TryToULong(m_comp_id, ival))
		glbin_comp_selector.SetId(ival, false);

	UpdateDataToUI({ gstTrackCompId });
}

void TrackDlgAgent::ClearCompId()
{
	m_comp_id.clear();
	glbin_comp_selector.SetId(0, true);
	UpdateDataToUI({ gstTrackCompId });
}

void TrackDlgAgent::SetCompFull()
{
	if (m_comp_id.empty())
	{
		glbin_comp_selector.CompFull();
		NotifyViewUpdate({ gstTrackList, gstSelUndoRedo });
	}
	else
		SetCompAppend();
}

void TrackDlgAgent::SetCompExclusive()
{
	glbin_comp_selector.Exclusive();
	auto view = glbin_current.render_view.lock();
	if (view)
		view->GetTraces(false);
	NotifyViewUpdate({ gstTrackList, gstSelUndoRedo });
}

void TrackDlgAgent::SetCompAppend()
{
	bool get_all = ToLower(m_comp_id) == "all" ? true : false;
	glbin_comp_selector.Select(get_all);
	auto view = glbin_current.render_view.lock();
	if (view)
		view->GetTraces(false);
	NotifyViewUpdate({ gstTrackList, gstSelUndoRedo });
}

void TrackDlgAgent::SetCompClear()
{
	glbin_comp_selector.Clear();
	auto view = glbin_current.render_view.lock();
	if (view)
		view->GetTraces(false);
	NotifyViewUpdate({ gstTrackList, gstSelUndoRedo });
}

void TrackDlgAgent::SetShuffle()
{
	//get current vd
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;

	vd->IncShuffle();
	NotifyViewUpdate({ gstNull });
}

void TrackDlgAgent::SetCellSize()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	int ival = dlg->GetCellSize();
	if (ival > 0)
	{
		glbin_comp_selector.SetUseMin(true);
		glbin_comp_selector.SetMinNum(ival);
		auto trkg = glbin_current.GetTrackGroup();
		if (trkg)
			trkg->get().SetCellSize(ival);
	}
	else
		glbin_comp_selector.SetUseMin(false);

	UpdateDataToUI({ gstTrackCellSize });
}

void TrackDlgAgent::ComputeUncertainty()
{
	glbin_trackmap_proc.GetCellsByUncertainty(false);
	NotifyViewUpdate({ gstTrackList, gstSelUndoRedo });
}

void TrackDlgAgent::SetCompUncertaintyLow()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	glbin_trackmap_proc.SetUncertainLow(dlg->GetCompUncertainLow());
	glbin_trackmap_proc.GetCellsByUncertainty(true);
	NotifyViewUpdate({ gstTrackUncertainLow, gstTrackList, gstSelUndoRedo });
}

void TrackDlgAgent::SetCompId2()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	m_comp_id = dlg->GetCompId2();
	unsigned long ival;
	if (m_comp_id.empty())
		glbin_comp_selector.SetId(0, true);
	else if (TryToULong(m_comp_id, ival))
		glbin_comp_selector.SetId(ival, false);

	UpdateDataToUI({ gstTrackCompId });
}

void TrackDlgAgent::SetCellExclusiveLink()
{
	glbin_trackmap_proc.LinkCells(true);
	NotifyViewUpdate({ gstNull });
}

void TrackDlgAgent::SetCellLink()
{
	glbin_trackmap_proc.LinkCells(false);
	NotifyViewUpdate({ gstNull });
}

void TrackDlgAgent::SetCellLinkAll()
{
	glbin_trackmap_proc.LinkAllCells();
	NotifyViewUpdate({ gstTrackList, gstSelUndoRedo });
}

void TrackDlgAgent::SetCellIsolate()
{
	glbin_trackmap_proc.IsolateCells();
	NotifyViewUpdate({ gstTrackList, gstSelUndoRedo });
}

void TrackDlgAgent::SetCellUnlink()
{
	glbin_trackmap_proc.UnlinkCells();
	NotifyViewUpdate({ gstTrackList, gstSelUndoRedo });
}

void TrackDlgAgent::SetCellNewId()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	m_comp_id3 = dlg->GetCellNewId();
	unsigned long ival;
	if (m_comp_id3.empty())
		glbin_comp_editor.SetId(0, true);
	else if (TryToULong(m_comp_id3, ival))
		glbin_comp_editor.SetId(ival, false);

	UpdateDataToUI({ gstTrackNewCompId });
}

void TrackDlgAgent::ClearCompNewId()
{
	m_comp_id3.clear();
	glbin_comp_editor.SetId(0, true);
	UpdateDataToUI({ gstTrackNewCompId });
}

void TrackDlgAgent::CreateCellNewId()
{
	glbin_comp_editor.NewId(false, true);
	NotifyViewUpdate({ gstTrackList, gstSelUndoRedo });
}

void TrackDlgAgent::SetCellAppendId()
{
	glbin_comp_editor.NewId(true, true);
	NotifyViewUpdate({ gstTrackList, gstSelUndoRedo });
}

void TrackDlgAgent::SetCellReplaceId()
{
	glbin_comp_editor.ReplaceList();
	NotifyViewUpdate({ gstTrackList, gstSelUndoRedo });
}

void TrackDlgAgent::SetCellCombineId()
{
	glbin_comp_editor.CombineList();
	NotifyViewUpdate({ gstTrackList, gstSelUndoRedo });
}

void TrackDlgAgent::SetCellSeparateId()
{
	glbin_trackmap_proc.DivideCells();
	NotifyViewUpdate({ gstTrackList, gstSelUndoRedo });
}

void TrackDlgAgent::SetCellSegment()
{
	glbin_trackmap_proc.SegmentCells();
	NotifyViewUpdate({ gstTrackList, gstSelUndoRedo });
}

void TrackDlgAgent::SetClusterNum()
{
	auto dlg = GetDialog();
	if (dlg)
		glbin_trackmap_proc.SetClusterNum(dlg->GetClusterNum());
}

void TrackDlgAgent::ConvertRulers()
{
	glbin_trackmap_proc.ConvertRulers();
	NotifyViewUpdate({ gstTrackList, gstSelUndoRedo, gstRulerList });
}

void TrackDlgAgent::ConvertConsistent()
{
	glbin_trackmap_proc.ConvertConsistent();
	NotifyViewUpdate({ gstTrackList, gstSelUndoRedo });
}

void TrackDlgAgent::AnalyzeComps()
{
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;

	glbin_comp_analyzer.SetVolume(vd);
	glbin_comp_analyzer.Analyze();
	std::string str;
	glbin_comp_analyzer.OutputCompListStr(str, 1);
	auto dlg = GetDialog();
	if (dlg)
		dlg->UpdateStatText(s2ws(str));
}

void TrackDlgAgent::AnalyzeLinks()
{
	glbin_trackmap_proc.AnalyzeLink();
}

void TrackDlgAgent::AnalyzeUncertainty()
{
	auto dlg = GetDialog();
	if (dlg)
		dlg->UpdateStatText(L"");
	glbin_trackmap_proc.AnalyzeUncertainty();
}

void TrackDlgAgent::AnalyzePaths()
{
	auto dlg = GetDialog();
	if (dlg)
		dlg->UpdateStatText(L"");
	glbin_trackmap_proc.AnalyzePath();
}

void TrackDlgAgent::SaveTrackResult()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	ModalDlg fopendlg(
		dlg, "Save results", "", "",
		"Text file (*.txt)|*.txt",
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	int rval = fopendlg.ShowModal();
	if (rval == wxID_OK)
	{
		wxString filename = fopendlg.GetPath();
		std::ofstream os;
		OutputStreamOpen(os, filename.ToStdString());

		wxString str;
		str = dlg->GetStatText();

		os << str;

		os.close();
	}
}

void TrackDlgAgent::SetCellPrev()
{
	if (glbin_moviemaker.IsRunning())
		return;
	int frame = glbin_moviemaker.GetCurrentFrame();
	frame--;
	glbin_moviemaker.SetCurrentFrame(frame);
	fluo::ValueCollection vc = { gstMovCurTime, gstMovProgSlider, gstCurrentFrame, gstMovSeqNum, gstTrackList };
	NotifyViewUpdate(vc);
}

void TrackDlgAgent::SetCellNext()
{
	if (glbin_moviemaker.IsRunning())
		return;
	int frame = glbin_moviemaker.GetCurrentFrame();
	frame++;
	glbin_moviemaker.SetCurrentFrame(frame);
	fluo::ValueCollection vc = { gstMovCurTime, gstMovProgSlider, gstCurrentFrame, gstMovSeqNum, gstTrackList };
	NotifyViewUpdate(vc);
}

void TrackDlgAgent::SetGhostNum()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	int ival = dlg->GetGhostNum();
	auto trkg = glbin_current.GetTrackGroup();
	if (!trkg)
		return;

	trkg->get().SetGhostNum(ival);
	NotifyViewUpdate({ gstGhostNum });
}

void TrackDlgAgent::SetGhostShowTail()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	bool bval = dlg->GetGhostShowTail();

	auto trkg = glbin_current.GetTrackGroup();
	if (!trkg)
		return;

	trkg->get().SetDrawTail(bval);
	NotifyViewUpdate({ gstNull });
}

void TrackDlgAgent::SetGhostShowLead()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	bool bval = dlg->GetGhostShowLead();

	auto trkg = glbin_current.GetTrackGroup();
	if (!trkg)
		return;

	trkg->get().SetDrawLead(bval);
	NotifyViewUpdate({ gstNull });
}

void TrackDlgAgent::SetListSelection()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	int sel = dlg->GetActiveList();
	auto list = dlg->GetSelection();

	flrd::CelpList celp_list;
	for (auto& i : list)
	{
		flrd::Celp cell(new flrd::Cell(i.id));
		cell->SetSizeUi(i.size);
		cell->SetSizeD(i.size);
		fluo::Point p(i.x, i.y, i.z);
		cell->SetCenter(p);
		celp_list.insert(std::pair<unsigned int, flrd::Celp>
			(i.id, cell));
	}

	switch (sel)
	{
	case 0:
		glbin_trackmap_proc.SetListIn(celp_list);
		glbin_comp_editor.SetList(celp_list);
		glbin_comp_selector.SetList(celp_list);
		break;
	case 1:
		glbin_trackmap_proc.SetListOut(celp_list);
		break;
	}
}

void TrackDlgAgent::DeleteSelection()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	int sel = dlg->GetActiveList();
	if (sel == 0)
	{
		glbin_comp_selector.DeleteList();
		NotifyViewUpdate({ gstTrackList, gstSelUndoRedo });
	}
}

