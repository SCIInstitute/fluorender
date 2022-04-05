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
#include <Global.hpp>
#include <AgentFactory.hpp>
#include <ComponentAgent.hpp>
#include <BrushToolAgent.hpp>
#include <VolumeData.hpp>
#include <Tracks.h>
#include <CompSelector.h>
#include <CompAnalyzer.h>
#include <TextureRenderer.h>
#include <VolumeRenderer.h>
#include <VertexArray.h>
#include <base_reader.h>
#include <lbl_reader.h>
#include <msk_writer.h>
#include <fstream>
#include <iostream>

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
	Renderview* view = getObject();
	if (!view) return;

	long cf, pf;
	view->getValue(gstCurrentFrame, cf);
	view->getValue(gstPreviousFrame, pf);

	flrd::Tracks* trace_group = view->GetTraceGroup();
	if (trace_group)
	{
		long lval;
		getValue(gstTrackCellSize, lval);
		trace_group->SetCellSize(lval);

		std::wstring str = trace_group->GetPath();
		setValue(gstTrackFile, str);
		UpdateList();

		lval = trace_group->GetGhostNum();
		setValue(gstGhostNum, lval);
		bool bval;
		bval = trace_group->GetDrawTail();
		setValue(gstGhostTailEnable, bval);
		bval = trace_group->GetDrawLead();
		setValue(gstGhostLeadEnable, bval);
	}
	else
	{
		dlg_.m_load_trace_text->SetValue("No Track map");
	}

	//settings for tracking
	//if (m_frame && m_frame->GetSettingDlg())
	//{
	//	m_iter_num =
	//		m_frame->GetSettingDlg()->GetTrackIter();
	//	m_size_thresh =
	//		m_frame->GetSettingDlg()->GetComponentSize();
	//	m_consistent_color =
	//		m_frame->GetSettingDlg()->GetConsistentColor();
	//	m_try_merge =
	//		m_frame->GetSettingDlg()->GetTryMerge();
	//	m_try_split =
	//		m_frame->GetSettingDlg()->GetTrySplit();
	//	m_similarity =
	//		m_frame->GetSettingDlg()->GetSimilarity();
	//	m_contact_factor =
	//		m_frame->GetSettingDlg()->GetContactFactor();
	//	//
	//	m_map_iter_spin->SetValue(m_iter_num);
	//	m_map_size_spin->SetValue(m_size_thresh);
	//	m_map_consistent_btn->SetValue(m_consistent_color);
	//	m_map_merge_btn->SetValue(m_try_merge);
	//	m_map_split_btn->SetValue(m_try_split);
	//	m_map_similar_spin->SetValue(m_similarity);
	//	m_map_contact_spin->SetValue(m_contact_factor);
	//}
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
	flrd::Tracks* trace_group = getObject()->GetTraceGroup();
	if (trace_group)
	{
		int cur_time = trace_group->GetCurTime();
		int prv_time = trace_group->GetPrvTime();
		//copy current to previous
		if (cur_time != prv_time)
		{
			MoveCurToPrv();
			//if (cur_time >= 0) m_cur_time = cur_time;
			//if (prv_time >= 0) m_prv_time = prv_time;

			//set tiem text
			wxString str;
			str = wxString::Format("\tCurrent T: %d", cur_time);
			dlg_.m_cell_time_curr_st->SetLabel(str);
			if (prv_time != cur_time)
				dlg_.m_cell_time_prev_st->SetLabel(
					wxString::Format("\tPrevious T: %d", prv_time));
			else
				dlg_.m_cell_time_prev_st->SetLabel("\tPrevious T");
		}
	}
	UpdateTraces();
	//dlg_.Layout();
}

void TrackAgent::MoveCurToPrv()
{
	dlg_.m_trace_list_prev->DeleteAllItems();

	int shuffle = glbin_agtf->getComponentAgent()->GetShuffle();
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
	long item = -1;
	for (;;)
	{
		item = dlg_.m_trace_list_curr->GetNextItem(item,
			wxLIST_NEXT_ALL,
			wxLIST_STATE_DONTCARE);
		if (item != -1)
		{
			item_gtype = dlg_.m_trace_list_curr->GetText(item, 0);
			item_id = dlg_.m_trace_list_curr->GetText(item, 1);
			item_size = dlg_.m_trace_list_curr->GetText(item, 2);
			item_x = dlg_.m_trace_list_curr->GetText(item, 3);
			item_y = dlg_.m_trace_list_curr->GetText(item, 4);
			item_z = dlg_.m_trace_list_curr->GetText(item, 5);
			item_id.ToULong(&id);
			Color c(id, shuffle);
			wxColor color(c.r() * 255, c.g() * 255, c.b() * 255);
			item_size.ToLong(&size);
			item_x.ToDouble(&x);
			item_y.ToDouble(&y);
			item_z.ToDouble(&z);
			dlg_.m_trace_list_prev->Append(item_gtype, id, color, size, x, y, z);
		}
		else break;
	}
}

void TrackAgent::AddLabel(long item, int type, flrd::CelpList &list)
{
	wxString str;
	unsigned long id;
	unsigned long size;
	double x, y, z;
	TraceListCtrl* ctrl = type == 0 ? dlg_.m_trace_list_curr : dlg_.m_trace_list_prev;

	str = ctrl->GetText(item, 1);
	str.ToULong(&id);
	str = ctrl->GetText(item, 2);
	str.ToULong(&size);
	str = ctrl->GetText(item, 3);
	str.ToDouble(&x);
	str = ctrl->GetText(item, 4);
	str.ToDouble(&y);
	str = ctrl->GetText(item, 5);
	str.ToDouble(&z);

	flrd::Celp cell(new flrd::Cell(id));
	cell->SetSizeUi(size);
	cell->SetSizeD(size);
	Point p(x, y, z);
	cell->SetCenter(p);
	list.insert(std::pair<unsigned int, flrd::Celp>
		(id, cell));
}

//ID link controls
void TrackAgent::CellUpdate()
{
	Renderview* view = getObject();
	if (!view) return;
	long lval;
	getValue(gstGhostNum, lval);
	flrd::Tracks* trace_group = view->GetTraceGroup();
	if (trace_group)
		trace_group->SetGhostNum(lval);
	else
		view->CreateTraceGroup();

	view->GetTraces(false);
	//m_view->Update(39);
	//GetSettings(m_view);
	UpdateAllSettings();
}

void TrackAgent::CellLink(bool exclusive)
{
	Renderview* view = getObject();
	if (!view) return;
	flrd::Tracks* trace_group = view->GetTraceGroup();
	if (!trace_group)
	{
		view->CreateTraceGroup();
		trace_group = view->GetTraceGroup();
	}

	//get selections
	long item;
	//current T
	flrd::CelpList list_cur;
	//previous T
	flrd::CelpList list_prv;
	//current list
	item = -1;
	while (true)
	{
		item = dlg_.m_trace_list_curr->GetNextItem(
			item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (item == -1)
			break;
		else
			AddLabel(item, 0, list_cur);
	}
	if (list_cur.size() == 0)
	{
		item = -1;
		while (true)
		{
			item = dlg_.m_trace_list_curr->GetNextItem(
				item, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
			if (item == -1)
				break;
			else
				AddLabel(item, 0, list_cur);
		}
	}
	//previous list
	item = -1;
	while (true)
	{
		item = dlg_.m_trace_list_prev->GetNextItem(
			item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (item == -1)
			break;
		else
			AddLabel(item, 1, list_prv);
	}
	if (list_prv.size() == 0)
	{
		item = -1;
		while (true)
		{
			item = dlg_.m_trace_list_prev->GetNextItem(
				item, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
			if (item == -1)
				break;
			else
				AddLabel(item, 1, list_prv);
		}
	}
	if (list_cur.size() == 0 ||
		list_prv.size() == 0)
		return;

	//link them
	long cf, pf;
	getValue(gstCurrentFrame, cf);
	getValue(gstPreviousFrame, pf);
	trace_group->LinkCells(list_cur, list_prv, cf, pf, exclusive);

	//update view
	//m_view->Update(39);
}

void TrackAgent::CellLinkAll()
{
	Renderview* view = getObject();
	if (!view) return;
	flrd::Tracks *trace_group = view->GetTraceGroup();
	if (!trace_group) return;

	flrd::pTrackMap track_map = trace_group->GetTrackMap();
	flrd::TrackMapProcessor tm_processor(track_map);
	//register file reading and deleteing functions
	tm_processor.RegisterCacheQueueFuncs(
		std::bind(&TrackAgent::ReadVolCache, this, std::placeholders::_1),
		std::bind(&TrackAgent::DelVolCache, this, std::placeholders::_1));
	tm_processor.SetVolCacheSize(3);
	flrd::CelpList in = glbin_agtf->getComponentAgent()->GetInCells();
	flrd::CelpList out = glbin_agtf->getComponentAgent()->GetOutCells();
	long cf;
	getValue(gstCurrentFrame, cf);
	tm_processor.RelinkCells(in, out, cf);

	CellUpdate();
}

void TrackAgent::CellIsolate()
{
	Renderview* view = getObject();
	if (!view) return;
	flrd::Tracks *trace_group = view->GetTraceGroup();
	if (!trace_group) return;

	//get selections
	long item;
	//current T
	flrd::CelpList list_cur;

	//current list
	item = -1;
	while (true)
	{
		item = dlg_.m_trace_list_curr->GetNextItem(
			item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (item == -1)
			break;
		else
			AddLabel(item, 0, list_cur);
	}
	if (list_cur.size() == 0)
	{
		item = -1;
		while (true)
		{
			item = dlg_.m_trace_list_curr->GetNextItem(
				item, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
			if (item == -1)
				break;
			else
				AddLabel(item, 0, list_cur);
		}
	}

	if (list_cur.size() == 0)
		return;

	//isolate
	long cf;
	getValue(gstCurrentFrame, cf);
	trace_group->IsolateCells(list_cur, cf);

	//update view
	//m_view->Update(39);
}

void TrackAgent::CellUnlink()
{
	Renderview* view = getObject();
	if (!view) return;
	flrd::Tracks *trace_group = view->GetTraceGroup();
	if (!trace_group) return;

	//get selections
	long item;
	//current T
	flrd::CelpList list_cur;
	//previous T
	flrd::CelpList list_prv;
	//current list
	item = -1;
	while (true)
	{
		item = dlg_.m_trace_list_curr->GetNextItem(
			item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (item == -1)
			break;
		else
			AddLabel(item, 0, list_cur);
	}
	if (list_cur.size() == 0)
	{
		item = -1;
		while (true)
		{
			item = dlg_.m_trace_list_curr->GetNextItem(
				item, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
			if (item == -1)
				break;
			else
				AddLabel(item, 0, list_cur);
		}
	}
	//previous list
	item = -1;
	while (true)
	{
		item = dlg_.m_trace_list_prev->GetNextItem(
			item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (item == -1)
			break;
		else
			AddLabel(item, 1, list_prv);
	}
	if (list_prv.size() == 0)
	{
		item = -1;
		while (true)
		{
			item = dlg_.m_trace_list_prev->GetNextItem(
				item, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
			if (item == -1)
				break;
			else
				AddLabel(item, 1, list_prv);
		}
	}
	if (list_cur.size() == 0 ||
		list_prv.size() == 0)
		return;

	//unlink them
	long cf, pf;
	getValue(gstCurrentFrame, cf);
	getValue(gstPreviousFrame, pf);
	trace_group->UnlinkCells(list_cur, list_prv, cf, pf);

	//update view
	//m_view->Update(39);
}

void TrackAgent::CellSeparate()
{
	Renderview* view = getObject();
	if (!view) return;
	flrd::Tracks *trace_group = view->GetTraceGroup();
	if (!trace_group) return;

	//current T
	flrd::CelpList list_cur;
	//fill current list
	long item = -1;
	while (true)
	{
		item = dlg_.m_trace_list_curr->GetNextItem(
			item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (item == -1)
			break;
		else
			AddLabel(item, 0, list_cur);
	}
	if (list_cur.empty())
	{
		item = -1;
		while (true)
		{
			item = dlg_.m_trace_list_curr->GetNextItem(
				item, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
			if (item == -1)
				break;
			else
				AddLabel(item, 0, list_cur);
		}
	}
	if (list_cur.size() <= 1)
		//nothing to divide
		return;

	//modify graphs
	long cf;
	getValue(gstCurrentFrame, cf);
	trace_group->DivideCells(list_cur, cf);
}

void TrackAgent::CellSegment()
{
	long clnum;
	getValue(gstClusterNum, clnum);
	if (clnum < 1)
		return;
	else if (clnum == 1)
	{
		ComponentAgent* compagent = glbin_agtf->getComponentAgent();
		if (compagent) compagent->CompCombine();
		return;
	}
	Renderview* view = getObject();
	if (!view) return;

	//current T
	flrd::CelpList list_cur;
	//fill current list
	long item = -1;
	while (true)
	{
		item = dlg_.m_trace_list_curr->GetNextItem(
			item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (item == -1)
			break;
		else
			AddLabel(item, 0, list_cur);
	}
	if (list_cur.empty())
	{
		item = -1;
		while (true)
		{
			item = dlg_.m_trace_list_curr->GetNextItem(
				item, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
			if (item == -1)
				break;
			else
				AddLabel(item, 0, list_cur);
		}
	}
	if (list_cur.size() == 0)
		//nothing to segment
		return;

	//modify graphs
	VolumeData* vd = view->GetCurrentVolume();
	if (!vd) return;
	long resx, resy, resz;
	vd->getValue(gstResX, resx);
	vd->getValue(gstResY, resy);
	vd->getValue(gstResZ, resz);
	flvr::Texture* tex = vd->GetTexture();
	if (!tex) return;
	flrd::Tracks *trace_group = view->GetTraceGroup();
	if (!trace_group) return;

	flrd::pTrackMap track_map = trace_group->GetTrackMap();
	flrd::TrackMapProcessor tm_processor(track_map);
	long lval;
	vd->getValue(gstBits, lval);
	tm_processor.SetBits(lval);
	double dval;
	vd->getValue(gstIntScale, dval);
	tm_processor.SetScale(dval);
	tm_processor.SetSizes(resx, resy, resz);
	//register file reading and deleteing functions
	tm_processor.RegisterCacheQueueFuncs(
		std::bind(&TrackAgent::ReadVolCache, this, std::placeholders::_1),
		std::bind(&TrackAgent::DelVolCache, this, std::placeholders::_1));
	tm_processor.SetVolCacheSize(3);
	long cf, pf;
	getValue(gstCurrentFrame, cf);
	getValue(gstPreviousFrame, pf);
	tm_processor.SegmentCells(list_cur, cf, pf);

	//invalidate label mask in gpu
	vd->GetRenderer()->clear_tex_current();
	//m_view->RefreshGL();
	//update view
	//CellUpdate();
	RefineMap();
}

//Component tools
void TrackAgent::CompDelete()
{
	Renderview* view = getObject();
	if (!view) return;

	long item = -1;
	wxString str;
	unsigned long ival;
	std::vector<unsigned long long> ids;
	for (;;)
	{
		item = dlg_.m_trace_list_curr->GetNextItem(item,
			wxLIST_NEXT_ALL,
			wxLIST_STATE_DONTCARE);

		if (item == -1)
			break;
		else if (dlg_.m_trace_list_curr->
			GetItemState(item, wxLIST_STATE_SELECTED)
			== wxLIST_STATE_SELECTED)
			continue;
		else
		{
			str = dlg_.m_trace_list_curr->GetItemText(item, 1);
			if (str.ToULong(&ival) && ival)
				ids.push_back(ival);
		}
	}

	//get current vd
	VolumeData* vd = view->GetCurrentVolume();
	flrd::ComponentSelector comp_selector(vd);
	if (ids.size() == 1)
	{
		comp_selector.SetId(ids[0]);
		comp_selector.Delete();
	}
	else
		comp_selector.Delete(ids);

	//update view
	CellUpdate();

	//frame
	glbin_agtf->getBrushToolAgent()->UpdateUndoRedo();
}

//uncertainty filter
void TrackAgent::UncertainFilter(bool input)
{
	Renderview* view = getObject();
	if (!view) return;
	//trace group
	flrd::Tracks *trace_group = view->GetTraceGroup();
	if (!trace_group) return;
	if (!trace_group->GetTrackMap()->GetFrameNum()) return;

	flrd::CelpList list_in, list_out;
	//fill inlist
	if (input)
	{
		long item = -1;
		while (true)
		{
			item = dlg_.m_trace_list_curr->GetNextItem(
				item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
			if (item == -1)
				break;
			else
				AddLabel(item, 0, list_in);
		}
		if (list_in.size() == 0)
		{
			item = -1;
			while (true)
			{
				item = dlg_.m_trace_list_curr->GetNextItem(
					item, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
				if (item == -1)
					break;
				else
					AddLabel(item, 0, list_in);
			}
		}
		if (list_in.empty())
			return;
	}

	flrd::pTrackMap track_map = trace_group->GetTrackMap();
	flrd::TrackMapProcessor tm_processor(track_map);
	long lval;
	getValue(gstCompUncertainLow, lval);
	tm_processor.SetUncertainLow(lval);
	long cf;
	getValue(gstCurrentFrame, cf);
	tm_processor.GetCellsByUncertainty(list_in, list_out, cf);

	VolumeData* vd = view->GetCurrentVolume();
	flrd::ComponentSelector comp_selector(vd);
	comp_selector.SelectList(list_out);

	//update view
	CellUpdate();

	//frame
	glbin_agtf->getBrushToolAgent()->UpdateUndoRedo();
}

void TrackAgent::LinkAddedCells(flrd::CelpList &list)
{
	Renderview* view = getObject();
	if (!view) return;
	VolumeData* vd = view->GetCurrentVolume();
	if (!vd) return;
	flrd::Tracks *trace_group = view->GetTraceGroup();
	if (!trace_group) return;

	long resx, resy, resz;
	vd->getValue(gstResX, resx);
	vd->getValue(gstResY, resy);
	vd->getValue(gstResZ, resz);

	flrd::pTrackMap track_map = trace_group->GetTrackMap();
	flrd::TrackMapProcessor tm_processor(track_map);
	long lval;
	vd->getValue(gstBits, lval);
	tm_processor.SetBits(lval);
	double dval;
	vd->getValue(gstIntScale, dval);
	tm_processor.SetScale(dval);
	tm_processor.SetSizes(resx, resy, resz);
	//register file reading and deleteing functions
	tm_processor.RegisterCacheQueueFuncs(
		std::bind(&TrackAgent::ReadVolCache, this, std::placeholders::_1),
		std::bind(&TrackAgent::DelVolCache, this, std::placeholders::_1));
	tm_processor.SetVolCacheSize(3);
	long cf;
	getValue(gstCurrentFrame, cf);
	tm_processor.LinkAddedCells(list, cf, cf - 1);
	tm_processor.LinkAddedCells(list, cf, cf + 1);
	RefineMap(true, false);
}

void TrackAgent::ClearTrack()
{
	Renderview* view = getObject();
	if (!view) return;
	flrd::Tracks* trace_group = view->GetTraceGroup();
	if (!trace_group) return;

	trace_group->Clear();
	dlg_.m_load_trace_text->SetValue("No Track map");
}

void TrackAgent::SaveOutputResult(const std::string &filename)
{
	std::ofstream os;
	OutputStreamOpen(os, filename);

	wxString str;
	str = dlg_.m_stat_text->GetValue();

	os << str;

	os.close();
}

//auto tracking
void TrackAgent::GenMap()
{
	Renderview* view = getObject();
	if (!view) return;

	//get trace group
	view->CreateTraceGroup();
	flrd::Tracks *trace_group = view->GetTraceGroup();
	if (!trace_group) return;

	VolumeData* vd = view->GetCurrentVolume();
	if (!vd) return;
	BaseReader* reader = vd->GetReader();
	if (!reader) return;

	//start progress
	dlg_.m_stat_text->SetValue("Generating track map.\n");
	//wxGetApp().Yield();
	int frames = reader->GetTimeNum();

	bool bval;
	long lval;
	double dval;
	//get and set parameters
	flrd::pTrackMap track_map = trace_group->GetTrackMap();
	flrd::TrackMapProcessor tm_processor(track_map);
	long resx, resy, resz;
	vd->getValue(gstResX, resx);
	vd->getValue(gstResY, resy);
	vd->getValue(gstResZ, resz);
	double spcx, spcy, spcz;
	vd->getValue(gstSpcX, spcx);
	vd->getValue(gstSpcY, spcy);
	vd->getValue(gstSpcZ, spcz);
	vd->getValue(gstBits, lval);
	tm_processor.SetBits(lval);
	vd->getValue(gstIntScale, dval);
	tm_processor.SetScale(dval);
	tm_processor.SetSizes(resx, resy, resz);
	tm_processor.SetSpacings(spcx, spcy, spcz);
	getValue(gstCompSizeLimit, lval);
	tm_processor.SetSizeThresh(lval);
	getValue(gstContactFactor, dval);
	tm_processor.SetContactThresh(dval);
	getValue(gstSimilarity, dval);
	tm_processor.SetSimilarThresh(dval);
	//register file reading and deleteing functions
	tm_processor.RegisterCacheQueueFuncs(
		std::bind(&TrackAgent::ReadVolCache, this, std::placeholders::_1),
		std::bind(&TrackAgent::DelVolCache, this, std::placeholders::_1));
	tm_processor.SetVolCacheSize(4);
	//merge/split
	getValue(gstTryMerge, bval);
	tm_processor.SetMerge(bval);
	getValue(gstTrySplit, bval);
	tm_processor.SetSplit(bval);

	//start timing
	std::chrono::high_resolution_clock::time_point t1 =
		std::chrono::high_resolution_clock::now();
	//initialization
	for (int i = 0; i < frames; ++i)
	{
		tm_processor.InitializeFrame(i);
		(*dlg_.m_stat_text) << wxString::Format("Time point %d initialized.\n", i);
		//wxGetApp().Yield();

		if (i < 1)
			continue;

		//link maps 1 and 2
		tm_processor.LinkFrames(i - 1, i);
		(*dlg_.m_stat_text) << wxString::Format("Time point %d linked.\n", i);
		//wxGetApp().Yield();

		//check contacts and merge cells
		tm_processor.ResolveGraph(i - 1, i);
		tm_processor.ResolveGraph(i, i - 1);
		(*dlg_.m_stat_text) << wxString::Format("Time point %d merged.\n", i - 1);
		//wxGetApp().Yield();

		if (i < 2)
			continue;

		//further process
		tm_processor.ProcessFrames(i - 2, i - 1);
		tm_processor.ProcessFrames(i - 1, i - 2);
		(*dlg_.m_stat_text) << wxString::Format("Time point %d processed.\n", i - 1);
		//wxGetApp().Yield();
	}
	//last frame
	tm_processor.ProcessFrames(frames - 2, frames - 1);
	tm_processor.ProcessFrames(frames - 1, frames - 2);
	(*dlg_.m_stat_text) << wxString::Format("Time point %d processed.\n", frames - 1);
	//wxGetApp().Yield();

	//iterations
	getValue(gstTrackIter, lval);
	for (size_t iteri = 0; iteri < lval; ++iteri)
	{
		for (int i = 2; i <= frames; ++i)
		{
			//further process
			tm_processor.ProcessFrames(i - 2, i - 1);
			tm_processor.ProcessFrames(i - 1, i - 2);
			(*dlg_.m_stat_text) << wxString::Format("Time point %d processed.\n", i - 1);
			//wxGetApp().Yield();
		}
	}

	//consistent colors
	getValue(gstCompConsistent, bval);
	if (bval)
	{
		(*dlg_.m_stat_text) << wxString::Format("Set colors for frame 0\n");
		//wxGetApp().Yield();
		tm_processor.MakeConsistent(0);
		//remaining frames
		for (size_t fi = 1; fi < track_map->GetFrameNum(); ++fi)
		{
			(*dlg_.m_stat_text) << wxString::Format("Set colors for frame %d\n", int(fi));
			//wxGetApp().Yield();
			tm_processor.MakeConsistent(fi - 1, fi);
		}
	}

	std::chrono::high_resolution_clock::time_point t2 =
		std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> time_span =
		std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
	(*dlg_.m_stat_text) << wxString::Format("Wall clock time: %.4fs\n", time_span.count());

	UpdateAllSettings();
	//GetSettings(m_view);

	//CellUpdate();
}

void TrackAgent::RefineMap(bool cur_time, bool erase_v)
{
	Renderview* view = getObject();
	if (!view) return;
	//get trace group
	VolumeData* vd = view->GetCurrentVolume();
	if (!vd) return;
	flrd::Tracks *trace_group = view->GetTraceGroup();
	if (!trace_group) return;

	long cf;
	getValue(gstCurrentFrame, cf);
	if (cur_time)
		dlg_.m_stat_text->SetValue(wxString::Format(
			"Refining track map at time point %d.\n", cf));
	else
		dlg_.m_stat_text->SetValue("Refining track map for all time points.\n");
	//wxGetApp().Yield();

	//start progress
	bool clear_counters = false;
	flrd::pTrackMap track_map = trace_group->GetTrackMap();
	int start_frame, end_frame;
	if (cur_time)
		start_frame = end_frame = cf;
	else
	{
		start_frame = 0;
		end_frame = track_map->GetFrameNum() - 1;
		clear_counters = true;
	}

	bool bval;
	long lval;
	double dval;
	//get and set parameters
	flrd::TrackMapProcessor tm_processor(track_map);
	long resx, resy, resz;
	vd->getValue(gstResX, resx);
	vd->getValue(gstResY, resy);
	vd->getValue(gstResZ, resz);
	double spcx, spcy, spcz;
	vd->getValue(gstSpcX, spcx);
	vd->getValue(gstSpcY, spcy);
	vd->getValue(gstSpcZ, spcz);
	vd->getValue(gstBits, lval);
	tm_processor.SetBits(lval);
	vd->getValue(gstIntScale, dval);
	tm_processor.SetScale(dval);
	tm_processor.SetSizes(resx, resy, resz);
	tm_processor.SetSpacings(spcx, spcy, spcz);
	getValue(gstCompSizeLimit, lval);
	tm_processor.SetSizeThresh(lval);
	getValue(gstContactFactor, dval);
	tm_processor.SetContactThresh(dval);
	getValue(gstSimilarity, dval);
	tm_processor.SetSimilarThresh(dval);
	//register file reading and deleteing functions
	tm_processor.RegisterCacheQueueFuncs(
		std::bind(&TrackAgent::ReadVolCache, this, std::placeholders::_1),
		std::bind(&TrackAgent::DelVolCache, this, std::placeholders::_1));
	tm_processor.SetVolCacheSize(4);
	//merge/split
	getValue(gstTryMerge, bval);
	tm_processor.SetMerge(bval);
	getValue(gstTrySplit, bval);
	tm_processor.SetSplit(bval);

	std::chrono::high_resolution_clock::time_point t1 =
		std::chrono::high_resolution_clock::now();

	//not sure if counters need to be cleared for all refinement
	//if (clear_counters)
	//	tm_processor.ClearCounters();
	//iterations
	getValue(gstTrackIter, lval);
	for (size_t iteri = 0; iteri < lval; ++iteri)
	{
		for (int i = start_frame - 1; i <= end_frame; ++i)
		{
			//further process
			tm_processor.ProcessFrames(i, i + 1, erase_v);
			tm_processor.ProcessFrames(i + 1, i, erase_v);
			(*dlg_.m_stat_text) << wxString::Format("Time point %d processed.\n", i + 1);
			//wxGetApp().Yield();
		}
	}

	//consistent colors
	getValue(gstCompConsistent, bval);
	if (bval)
	{
		if (cur_time)
			tm_processor.MakeConsistent(cf - 1, cf);
		else
		{
			(*dlg_.m_stat_text) << wxString::Format("Set colors for frame 0\n");
			//wxGetApp().Yield();
			tm_processor.MakeConsistent(0);
			//remaining frames
			for (size_t fi = 1; fi < track_map->GetFrameNum(); ++fi)
			{
				(*dlg_.m_stat_text) << wxString::Format("Set colors for frame %d\n", int(fi));
				//wxGetApp().Yield();
				tm_processor.MakeConsistent(fi - 1, fi);
			}
		}
	}

	std::chrono::high_resolution_clock::time_point t2 =
		std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> time_span =
		std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
	(*dlg_.m_stat_text) << wxString::Format("Wall clock time: %.4fs\n", time_span.count());

	CellUpdate();
}

int TrackAgent::GetTrackFileExist(bool save)
{
	Renderview* view = getObject();
	if (!view) return 0;
	flrd::Tracks* trace_group = view->GetTraceGroup();
	if (!trace_group) return 0;
	std::wstring filename = trace_group->GetPath();
	if (wxFileExists(filename))
	{
		if (save)
		{
			view->SaveTraceGroup(filename);
			setValue(gstTrackFile, filename);
		}
		return 2;
	}
	else
		return 1;
}

void TrackAgent::LoadTrackFile(const std::wstring &file)
{
	Renderview* view = getObject();
	if (!view) return;
	if (view->LoadTraceGroup(file))
		setValue(gstTrackFile, file);
}

void TrackAgent::SaveTrackFile(const std::wstring &file)
{
	if (getObject()->SaveTraceGroup(file))
	{
		dlg_.m_load_trace_text->SetValue(file);
		setValue(gstTrackFile, file);
	}
}

void TrackAgent::ReadVolCache(flrd::VolCache& vol_cache)
{
	//get volume, readers
	Renderview* view = getObject();
	if (!view) return;
	VolumeData* vd = view->GetCurrentVolume();
	if (!vd) return;
	BaseReader* reader = vd->GetReader();
	if (!reader) return;
	LBLReader lbl_reader;

	long chan;
	vd->getValue(gstChannel, chan);
	int frame = vol_cache.frame;
	long lval;
	getValue(gstCurrentFrame, lval);
	if (frame == lval)
	{
		flvr::Texture* tex = vd->GetTexture();
		if (!tex)
			return;

		Nrrd* data = tex->get_nrrd(0);
		vol_cache.nrrd_data = data;
		vol_cache.data = data->data;
		Nrrd* label = tex->get_nrrd(tex->nlabel());
		vol_cache.nrrd_label = label;
		vol_cache.label = label->data;
		if (data && label)
			vol_cache.valid = true;
	}
	else
	{
		Nrrd* data = reader->Convert(frame, chan, true);
		if (!data)
			return;
		vol_cache.nrrd_data = data;
		vol_cache.data = data->data;
		wstring lblname = reader->GetCurLabelName(frame, chan);
		lbl_reader.SetFile(lblname);
		Nrrd* label = lbl_reader.Convert(frame, chan, true);
		if (!label)
			return;
		vol_cache.nrrd_label = label;
		vol_cache.label = label->data;
		if (data && label)
			vol_cache.valid = true;
	}
}

void TrackAgent::DelVolCache(flrd::VolCache& vol_cache)
{
	Renderview* view = getObject();
	if (!view) return;
	VolumeData* vd = view->GetCurrentVolume();
	if (!vd) return;
	BaseReader* reader = vd->GetReader();
	if (!reader) return;
	long chan;
	vd->getValue(gstChannel, chan);
	int frame = vol_cache.frame;

	if (vol_cache.valid && vol_cache.modified)
	{
		//save it first if modified
		//assume that only label is modified
		MSKWriter msk_writer;
		msk_writer.SetData((Nrrd*)vol_cache.nrrd_label);
		double spcx, spcy, spcz;
		vd->getValue(gstSpcX, spcx);
		vd->getValue(gstSpcY, spcy);
		vd->getValue(gstSpcZ, spcz);
		msk_writer.SetSpacings(spcx, spcy, spcz);
		wstring filename = reader->GetCurLabelName(frame, chan);
		msk_writer.Save(filename, 1);
	}

	vol_cache.valid = false;
	long lval;
	getValue(gstCurrentFrame, lval);
	if (frame != lval)
	{
		if (vol_cache.data)
			nrrdNuke((Nrrd*)vol_cache.nrrd_data);
		if (vol_cache.label)
			nrrdNuke((Nrrd*)vol_cache.nrrd_label);
	}
	vol_cache.data = 0;
	vol_cache.nrrd_data = 0;
	vol_cache.label = 0;
	vol_cache.nrrd_label = 0;
}

void TrackAgent::OnTrackCellSize(Event& event)
{
	flrd::Tracks* trace_group = getObject()->GetTraceGroup();
	if (!trace_group) return;

	long lval;
	getValue(gstTrackCellSize, lval);
	trace_group->SetCellSize(lval);
}

void TrackAgent::OnTrackFile(Event& event)
{
	std::wstring str;
	getValue(gstTrackFile, str);
	if (str != L"")
		dlg_.m_load_trace_text->SetValue(str);
	else
		dlg_.m_load_trace_text->SetValue("Track map created but not saved");
}

void TrackAgent::OnGhostNum(Event& event)
{
	flrd::Tracks* trace_group = getObject()->GetTraceGroup();
	if (!trace_group) return;

	long lval;
	getValue(gstGhostNum, lval);
	trace_group->SetGhostNum(lval);
	dlg_.m_ghost_num_text->ChangeValue(wxString::Format("%d", lval));
	dlg_.m_ghost_num_sldr->SetValue(lval);
	//m_view->Update(39);
}

void TrackAgent::OnGhostTailEnable(Event& event)
{
	flrd::Tracks* trace_group = getObject()->GetTraceGroup();
	if (!trace_group) return;

	bool bval;
	getValue(gstGhostTailEnable, bval);
	trace_group->SetDrawTail(bval);
	dlg_.m_ghost_show_tail_chk->SetValue(bval);
	//m_view->Update(39);
}

void TrackAgent::OnGhostLeadEnable(Event& event)
{
	flrd::Tracks* trace_group = getObject()->GetTraceGroup();
	if (!trace_group) return;

	bool bval;
	getValue(gstGhostLeadEnable, bval);
	trace_group->SetDrawLead(bval);
	dlg_.m_ghost_show_lead_chk->SetValue(bval);
	//m_view->Update(39);
}

void TrackAgent::OnCompUncertainLow(Event& event)
{
	flrd::Tracks* trace_group = getObject()->GetTraceGroup();
	if (!trace_group) return;

	long lval;
	getValue(gstCompUncertainLow, lval);
	trace_group->SetUncertainLow(lval);
	UncertainFilter(true);
}

void TrackAgent::ConvertToRulers()
{
	Renderview* view = getObject();
	if (!view) return;
	flrd::Tracks* trace_group = view->GetTraceGroup();
	if (!trace_group) return;
	VolumeData* vd = view->GetCurrentVolume();
	if (!vd) return;

	double spcx, spcy, spcz;
	vd->getValue(gstSpcX, spcx);
	vd->getValue(gstSpcY, spcy);
	vd->getValue(gstSpcZ, spcz);

	//get rulers
	flrd::RulerList rulers;
	trace_group->GetMappedRulers(rulers);
	for (auto iter = rulers.begin();
		iter != rulers.end(); ++iter)
	{
		(*iter)->Scale(spcx, spcy, spcz);
		view->GetRulerList()->push_back(*iter);
	}
	//m_view->Update(39);
	//if (m_frame && m_frame->GetMeasureDlg())
	//	m_frame->GetMeasureDlg()->GetSettings(m_view);
	flvr::TextureRenderer::vertex_array_manager_.set_dirty(flvr::VA_Rulers);
}

void TrackAgent::ConvertConsistent()
{
	Renderview* view = getObject();
	if (!view) return;
	VolumeData* vd = view->GetCurrentVolume();
	if (!vd) return;
	flrd::Tracks *trace_group = view->GetTraceGroup();
	if (!trace_group) return;

	dlg_.m_stat_text->SetValue("Generating consistent IDs in");
	//wxGetApp().Yield();

	flrd::pTrackMap track_map = trace_group->GetTrackMap();
	flrd::TrackMapProcessor tm_processor(track_map);
	long resx, resy, resz;
	double spcx, spcy, spcz;
	vd->getValue(gstResX, resx);
	vd->getValue(gstResY, resy);
	vd->getValue(gstResZ, resz);
	vd->getValue(gstSpcX, spcx);
	vd->getValue(gstSpcY, spcy);
	vd->getValue(gstSpcZ, spcz);
	long lval;
	vd->getValue(gstBits, lval);
	tm_processor.SetBits(lval);
	double dval;
	vd->getValue(gstIntScale, dval);
	tm_processor.SetScale(dval);
	tm_processor.SetSizes(resx, resy, resz);
	tm_processor.SetSpacings(spcx, spcy, spcz);
	tm_processor.RegisterCacheQueueFuncs(
		std::bind(&TrackAgent::ReadVolCache, this, std::placeholders::_1),
		std::bind(&TrackAgent::DelVolCache, this, std::placeholders::_1));
	tm_processor.SetVolCacheSize(2);

	(*dlg_.m_stat_text) << wxString::Format("Frame %d\n", 0);
	//wxGetApp().Yield();
	tm_processor.MakeConsistent(0);

	//remaining frames
	for (size_t fi = 1; fi < track_map->GetFrameNum(); ++fi)
	{
		(*dlg_.m_stat_text) << wxString::Format("Frame %d\n", int(fi));
		//wxGetApp().Yield();
		tm_processor.MakeConsistent(fi - 1, fi);
	}

	CellUpdate();
}

void TrackAgent::AnalyzeComp()
{
	Renderview* view = getObject();
	if (!view) return;
	VolumeData* vd = view->GetCurrentVolume();
	flrd::ComponentAnalyzer* analyzer = view->GetCompAnalyzer();
	analyzer->SetVolume(vd);
	analyzer->Analyze(true, true);
	string str;
	analyzer->OutputCompListStr(str, 1);
	dlg_.m_stat_text->SetValue(str);
}

void TrackAgent::AnalyzeLink()
{
	Renderview* view = getObject();
	if (!view) return;
	flrd::Tracks* trace_group = view->GetTraceGroup();
	if (!trace_group) return;
	size_t frames = trace_group->GetTrackMap()->GetFrameNum();
	if (frames == 0)
		dlg_.m_stat_text->SetValue("ERROR! Generate a track map first.\n");
	else
		dlg_.m_stat_text->SetValue(
			wxString::Format("Time point number: %d\n", int(frames)));

	(*dlg_.m_stat_text) << "Time\tIn Orphan\tOut Orphan\tIn Multi\tOut Multi\n";
	flrd::VertexList in_orphan_list;
	flrd::VertexList out_orphan_list;
	flrd::VertexList in_multi_list;
	flrd::VertexList out_multi_list;
	for (size_t fi = 0; fi < frames; ++fi)
	{
		trace_group->GetLinkLists(fi,
			in_orphan_list, out_orphan_list,
			in_multi_list, out_multi_list);
		(*dlg_.m_stat_text) << int(fi) << "\t" <<
			int(in_orphan_list.size()) << "\t" <<
			int(out_orphan_list.size()) << "\t" <<
			int(in_multi_list.size()) << "\t" <<
			int(out_multi_list.size()) << "\n";
	}
}

void TrackAgent::AnalyzeUncertainHist()
{
	Renderview* view = getObject();
	if (!view) return;
	//trace group
	flrd::Tracks *trace_group = view->GetTraceGroup();
	if (!trace_group) return;
	if (!trace_group->GetTrackMap()->GetFrameNum()) return;

	//fill inlist
	flrd::CelpList list_in;
	long item = -1;
	while (true)
	{
		item = dlg_.m_trace_list_curr->GetNextItem(
			item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (item == -1)
			break;
		else
			AddLabel(item, 0, list_in);
	}
	if (list_in.size() == 0)
	{
		item = -1;
		while (true)
		{
			item = dlg_.m_trace_list_curr->GetNextItem(
				item, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
			if (item == -1)
				break;
			else
				AddLabel(item, 0, list_in);
		}
	}

	dlg_.m_stat_text->SetValue("");

	long cf;
	getValue(gstCurrentFrame, cf);
	flrd::pTrackMap track_map = trace_group->GetTrackMap();
	flrd::TrackMapProcessor tm_processor(track_map);
	if (list_in.empty())
	{
		flrd::UncertainHist hist1, hist2;
		tm_processor.GetUncertainHist(hist1, hist2, cf);
		//header
		(*dlg_.m_stat_text) << "In\n";
		(*dlg_.m_stat_text) << "Level\t" << "Frequency\n";
		int count = 0;
		for (auto iter = hist1.begin();
			iter != hist1.end(); ++iter)
		{
			while (iter->second.level > count)
			{
				(*dlg_.m_stat_text) << count++ << "\t" << "0\n";
			}
			(*dlg_.m_stat_text) << int(iter->second.level) << "\t" <<
				int(iter->second.count) << "\n";
			count++;
		}

		//header
		(*dlg_.m_stat_text) << "\n";
		(*dlg_.m_stat_text) << "Out\n";
		(*dlg_.m_stat_text) << "Level\t" << "Frequency\n";
		count = 0;
		for (auto iter = hist2.begin();
			iter != hist2.end(); ++iter)
		{
			while (iter->second.level > count)
			{
				(*dlg_.m_stat_text) << count++ << "\t" << "0\n";
			}
			(*dlg_.m_stat_text) << int(iter->second.level) << "\t" <<
				int(iter->second.count) << "\n";
			count++;
		}
	}
	else
	{
		tm_processor.GetCellUncertainty(list_in, cf);
		//header
		(*dlg_.m_stat_text) << "ID\t" << "In\t" << "Out\n";
		for (auto iter = list_in.begin();
			iter != list_in.end(); ++iter)
		{
			wxString sid = wxString::Format("%u", iter->second->Id());
			(*dlg_.m_stat_text) << sid << "\t" <<
				int(iter->second->GetCount0()) << "\t" <<
				int(iter->second->GetCount1()) << "\n";
		}
	}
}

void TrackAgent::AnalyzePath()
{
	Renderview* view = getObject();
	if (!view) return;
	//trace group
	flrd::Tracks *trace_group = view->GetTraceGroup();
	if (!trace_group) return;
	if (!trace_group->GetTrackMap()->GetFrameNum()) return;

	//fill inlist
	flrd::CelpList list_in;
	long item = -1;
	while (true)
	{
		item = dlg_.m_trace_list_curr->GetNextItem(
			item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (item == -1)
			break;
		else
			AddLabel(item, 0, list_in);
	}
	if (list_in.size() == 0)
	{
		item = -1;
		while (true)
		{
			item = dlg_.m_trace_list_curr->GetNextItem(
				item, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
			if (item == -1)
				break;
			else
				AddLabel(item, 0, list_in);
		}
	}

	dlg_.m_stat_text->SetValue("");

	flrd::pTrackMap track_map = trace_group->GetTrackMap();
	flrd::TrackMapProcessor tm_processor(track_map);
	if (list_in.empty())
		return;

	dlg_.m_stat_text->SetValue("");
	std::ostream os(dlg_.m_stat_text);

	long cf;
	getValue(gstCurrentFrame, cf);
	if (cf > 0)
	{
		(*dlg_.m_stat_text) << "Paths of T" << cf << " to T" << cf - 1 << ":\n";
		flrd::PathList paths_prv;
		tm_processor.GetPaths(list_in, paths_prv, cf, cf - 1);
		for (size_t i = 0; i < paths_prv.size(); ++i)
			os << paths_prv[i];
	}
	if (cf < track_map->GetFrameNum() - 1)
	{
		(*dlg_.m_stat_text) << "Paths of T" << cf << " to T" << cf + 1 << ":\n";
		flrd::PathList paths_nxt;
		tm_processor.GetPaths(list_in, paths_nxt, cf, cf + 1);
		for (size_t i = 0; i < paths_nxt.size(); ++i)
			os << paths_nxt[i];
	}
}

